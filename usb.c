#include <drv_types.h>
#include <rtw_debug.h>
#include <rtl8812a_recv.h>
#include <../rtl8821au/trx.h>

/*
* Increase and check if the continual_urb_error of this @param dvobjprive is larger than MAX_CONTINUAL_URB_ERR
* @return _TRUE:
* @return _FALSE:
*/
static inline int rtw_inc_and_chk_continual_urb_error(struct rtl_usb *dvobj)
{
	int ret = _FALSE;
	int value;
	if( (value=atomic_inc_return(&dvobj->continual_urb_error)) > MAX_CONTINUAL_URB_ERR) {
		DBG_871X("[dvobj:%p][ERROR] continual_urb_error:%d > %d\n", dvobj, value, MAX_CONTINUAL_URB_ERR);
		ret = _TRUE;
	} else {
		//DBG_871X("[dvobj:%p] continual_urb_error:%d\n", dvobj, value);
	}
	return ret;
}

extern int pm_netdev_open(struct net_device *ndev,uint8_t bnormal);
int rtw_resume_process(struct rtl_priv *rtlpriv);

static int usbctrl_vendorreq(struct rtl_priv *rtlpriv, uint8_t request, u16 value, u16 index, void *pdata, u16 len, uint8_t requesttype)
{
	struct rtl_usb  *pdvobjpriv = rtl_usbdev(rtlpriv);
	struct usb_device *udev=pdvobjpriv->udev;
	int _unused;

	unsigned int pipe;
	int status = 0;
	u32 tmp_buflen=0;
	uint8_t reqtype;
	uint8_t *pIo_buf;
	int vendorreq_times = 0;

	uint8_t tmp_buf[MAX_USB_IO_CTL_SIZE];

	/* DBG_871X("%s %s:%d\n",__FUNCTION__, current->comm, current->pid); */

	if((rtlpriv->bSurpriseRemoved) ||(rtlpriv->pwrctrlpriv.pnp_bstop_trx)){
		status = -EPERM;
		goto exit;
	}

	if(len>MAX_VENDOR_REQ_CMD_SIZE){
		DBG_8192C( "[%s] Buffer len error ,vendor request failed\n", __FUNCTION__ );
		status = -EINVAL;
		goto exit;
	}

#ifdef CONFIG_USB_VENDOR_REQ_MUTEX
	_unused = mutex_lock_interruptible(&pdvobjpriv->usb_vendor_req_mutex);
#endif


	/* Acquire IO memory for vendorreq */
#ifdef CONFIG_USB_VENDOR_REQ_BUFFER_PREALLOC
	pIo_buf = pdvobjpriv->usb_vendor_req_buf;
#else
	tmp_buflen = MAX_USB_IO_CTL_SIZE;

	/*
	 * Added by Albert 2010/02/09
	 * For mstar platform, mstar suggests the address for USB IO should be 16 bytes alignment.
	 * Trying to fix it here.
	 */
	pIo_buf = (tmp_buf==NULL)?NULL:tmp_buf + ALIGNMENT_UNIT -((SIZE_PTR)(tmp_buf) & 0x0f );
#endif

	if ( pIo_buf== NULL) {
		DBG_8192C( "[%s] pIo_buf == NULL \n", __FUNCTION__ );
		status = -ENOMEM;
		goto release_mutex;
	}

	while (++vendorreq_times<= MAX_USBCTRL_VENDORREQ_TIMES) {
		memset(pIo_buf, 0, len);

		if (requesttype == 0x01) {
			pipe = usb_rcvctrlpipe(udev, 0);	/* read_in */
			reqtype =  REALTEK_USB_VENQT_READ;
		} else {
			pipe = usb_sndctrlpipe(udev, 0);	/* write_out */
			reqtype =  REALTEK_USB_VENQT_WRITE;
			memcpy( pIo_buf, pdata, len);
		}

		status = usb_control_msg(udev, pipe, request, reqtype, value, index, pIo_buf, len, RTW_USB_CONTROL_MSG_TIMEOUT);

		if (status == len) {   // Success this control transfer. */
			rtw_reset_continual_urb_error(pdvobjpriv);
			if (requesttype == 0x01) {
				/* For Control read transfer, we have to copy the read data from pIo_buf to pdata. */
				memcpy(pdata, pIo_buf,  len);
			}
		} else {
			/* error cases */
			DBG_8192C("reg 0x%x, usb %s %u fail, status:%d value=0x%x, vendorreq_times:%d\n"
				, value,(requesttype == 0x01)?"read":"write" , len, status, *(u32*)pdata, vendorreq_times);

			if (status < 0) {
				if(status == (-ESHUTDOWN) || status == -ENODEV) {
					rtlpriv->bSurpriseRemoved = _TRUE;
				} else {
					;
				}
			} else {
				/* status != len && status >= 0 */

				if(status > 0) {
					if ( requesttype == 0x01 ) {
						/* For Control read transfer, we have to copy the read data from pIo_buf to pdata. */
						memcpy( pdata, pIo_buf,  len );
					}
				}
			}

			if(rtw_inc_and_chk_continual_urb_error(pdvobjpriv) == _TRUE ){
				rtlpriv->bSurpriseRemoved = _TRUE;
				break;
			}

		}

		/* firmware download is checksumed, don't retry */
		if( (value >= FW_START_ADDRESS ) || status == len )
			break;

	}

	/* release IO memory used by vendorreq */

release_mutex:
#ifdef CONFIG_USB_VENDOR_REQ_MUTEX
	mutex_unlock(&pdvobjpriv->usb_vendor_req_mutex);
#endif
exit:
	return status;

}



static uint8_t usb_read8(struct rtl_priv *rtlpriv, uint32_t addr)
{
	uint8_t request;
	uint8_t requesttype;
	u16 wvalue;
	u16 index;
	u16 len;
	uint8_t data = 0;

	request = 0x05;
	requesttype = 0x01;	/* read_in */
	index = 0;		/* n/a */

	wvalue = (u16) (addr&0x0000ffff);
	len = 1;

	usbctrl_vendorreq(rtlpriv, request, wvalue, index, &data, len, requesttype);

	return data;

}

static u16 usb_read16(struct rtl_priv *rtlpriv, uint32_t addr)
{
	uint8_t request;
	uint8_t requesttype;
	u16 wvalue;
	u16 index;
	u16 len;
	u16 data = 0;

	request = 0x05;
	requesttype = 0x01;	/* read_in */
	index = 0;		/* n/a */

	wvalue = (u16)(addr&0x0000ffff);
	len = 2;

	usbctrl_vendorreq(rtlpriv, request, wvalue, index, &data, len, requesttype);

	return data;

}

static uint32_t usb_read32(struct rtl_priv *rtlpriv, uint32_t addr)
{
	uint8_t request;
	uint8_t requesttype;
	u16 wvalue;
	u16 index;
	u16 len;
	uint32_t data = 0;

	request = 0x05;
	requesttype = 0x01;	/* read_in */
	index = 0;		/* n/a */

	wvalue = (u16)(addr&0x0000ffff);
	len = 4;

	usbctrl_vendorreq(rtlpriv, request, wvalue, index, &data, len, requesttype);


	return data;

}

static void usb_write8(struct rtl_priv *rtlpriv, uint32_t addr, uint8_t val)
{
	uint8_t request;
	uint8_t requesttype;
	u16 wvalue;
	u16 index;
	u16 len;
	uint8_t data;

	request = 0x05;
	requesttype = 0x00;		/* write_out */
	index = 0;			/* n/a */

	wvalue = (u16)(addr&0x0000ffff);
	len = 1;

	data = val;

	usbctrl_vendorreq(rtlpriv, request, wvalue, index, &data, len, requesttype);
}

static void usb_write16(struct rtl_priv *rtlpriv, uint32_t addr, u16 val)
{
	uint8_t request;
	uint8_t requesttype;
	u16 wvalue;
	u16 index;
	u16 len;
	u16 data;

	request = 0x05;
	requesttype = 0x00;	/* write_out */
	index = 0;		/* n/a */

	wvalue = (u16)(addr&0x0000ffff);
	len = 2;

	data = val;

	usbctrl_vendorreq(rtlpriv, request, wvalue, index, &data, len, requesttype);
}

static void usb_write32(struct rtl_priv *rtlpriv, uint32_t addr, uint32_t val)
{
	uint8_t request;
	uint8_t requesttype;
	u16 wvalue;
	u16 index;
	u16 len;
	uint32_t data;

	request = 0x05;
	requesttype = 0x00;	/* write_out */
	index = 0;		/* n/a */

	wvalue = (u16)(addr&0x0000ffff);
	len = 4;
	data = val;

	usbctrl_vendorreq(rtlpriv, request, wvalue, index, &data, len, requesttype);
}

static void usb_writeN(struct rtl_priv *rtlpriv, uint32_t addr, void *pdata, u16 length)
{
	uint8_t request;
	uint8_t requesttype;
	u16 wvalue;
	u16 index;
	u16 len;
	uint8_t buf[VENDOR_CMD_MAX_DATA_LEN] = {0};

	request = 0x05;
	requesttype = 0x00;	/* write_out */
	index = 0;		/* n/a */

	wvalue = (u16)(addr&0x0000ffff);
	len = length;
	memcpy(buf, pdata, len);

	usbctrl_vendorreq(rtlpriv, request, wvalue, index, buf, len, requesttype);
}

static void _rtl_usb_io_handler_init(struct device *dev,
				     struct rtl_priv *rtlpriv)
{
	rtlpriv->io.dev = dev;
	mutex_init(&rtlpriv->io.bb_mutex);
	rtlpriv->io.write8_async	= usb_write8;
	rtlpriv->io.write16_async	= usb_write16;
	rtlpriv->io.write32_async	= usb_write32;
	rtlpriv->io.read8_sync		= usb_read8;
	rtlpriv->io.read16_sync		= usb_read16;
	rtlpriv->io.read32_sync		= usb_read32;
	rtlpriv->io.writeN_sync		= usb_writeN;
}

static void _rtl_usb_io_handler_release(struct rtl_priv *rtlpriv)
{
	mutex_destroy(&rtlpriv->io.bb_mutex);
}


static void usb_write_port_complete(struct urb *purb, struct pt_regs *regs)
{
	unsigned long flags;
	int i;
	struct xmit_buf *pxmitbuf = (struct xmit_buf *)purb->context;
	/* struct xmit_frame *pxmitframe = (struct xmit_frame *)pxmitbuf->priv_data; */
	/* struct rtl_priv			*rtlpriv = pxmitframe->rtlpriv; */
	struct rtl_priv	*rtlpriv = pxmitbuf->rtlpriv;
	struct xmit_priv	*pxmitpriv = &rtlpriv->xmitpriv;
	/* struct pkt_attrib *pattrib = &pxmitframe->attrib; */

	switch(pxmitbuf->flags) {
		case VO_QUEUE_INX:
			pxmitpriv->voq_cnt--;
			break;
		case VI_QUEUE_INX:
			pxmitpriv->viq_cnt--;
			break;
		case BE_QUEUE_INX:
			pxmitpriv->beq_cnt--;
			break;
		case BK_QUEUE_INX:
			pxmitpriv->bkq_cnt--;
			break;
		case HIGH_QUEUE_INX:
#ifdef CONFIG_AP_MODE
			rtw_chk_hi_queue_cmd(rtlpriv);
#endif
			break;
		default:
			break;
	}


/*
	spin_lock_irqsave(&pxmitpriv->lock, &irqL);

	pxmitpriv->txirp_cnt--;

	switch(pattrib->priority)
	{
		case 1:
		case 2:
			pxmitpriv->bkq_cnt--;
			//DBG_8192C("pxmitpriv->bkq_cnt=%d\n", pxmitpriv->bkq_cnt);
			break;
		case 4:
		case 5:
			pxmitpriv->viq_cnt--;
			//DBG_8192C("pxmitpriv->viq_cnt=%d\n", pxmitpriv->viq_cnt);
			break;
		case 6:
		case 7:
			pxmitpriv->voq_cnt--;
			//DBG_8192C("pxmitpriv->voq_cnt=%d\n", pxmitpriv->voq_cnt);
			break;
		case 0:
		case 3:
		default:
			pxmitpriv->beq_cnt--;
			//DBG_8192C("pxmitpriv->beq_cnt=%d\n", pxmitpriv->beq_cnt);
			break;

	}

	spin_unlock_irqrestore(&pxmitpriv->lock, &irqL);


	if(pxmitpriv->txirp_cnt==0)
	{
		RT_TRACE(_module_hci_ops_os_c_,_drv_err_,("usb_write_port_complete: txirp_cnt== 0, set allrxreturnevt!\n"));
		_rtw_up_sema(&(pxmitpriv->tx_retevt));
	}
*/
        /* rtw_free_xmitframe(pxmitpriv, pxmitframe); */

	if (rtlpriv->bSurpriseRemoved || rtlpriv->bDriverStopped ||rtlpriv->bWritePortCancel) {
		DBG_8192C("%s(): TX Warning! bDriverStopped(%d) OR bSurpriseRemoved(%d) bWritePortCancel(%d) pxmitbuf->buf_tag(%x) \n",
		__FUNCTION__,rtlpriv->bDriverStopped, rtlpriv->bSurpriseRemoved,rtlpriv->bReadPortCancel,pxmitbuf->buf_tag);

		goto check_completion;
	}


	if (purb->status==0) {

	} else {
		DBG_871X("###=> urb_write_port_complete status(%d)\n",purb->status);
		if((purb->status==-EPIPE)||(purb->status==-EPROTO)) {
			/*
			 * usb_clear_halt(pusbdev, purb->pipe);
			 * msleep(10);
			 */
		} else if (purb->status == -EINPROGRESS) {
			goto check_completion;

		} else if (purb->status == -ENOENT) {
			DBG_871X("%s: -ENOENT\n", __func__);
			goto check_completion;

		} else if (purb->status == -ECONNRESET) {
			DBG_871X("%s: -ECONNRESET\n", __func__);
			goto check_completion;

		} else if (purb->status == -ESHUTDOWN) {
			rtlpriv->bDriverStopped=_TRUE;

			goto check_completion;
		}
		else
		{
			rtlpriv->bSurpriseRemoved=_TRUE;
			DBG_8192C("bSurpriseRemoved=TRUE\n");
			/* rtl8192cu_trigger_gpio_0(rtlpriv); */

			goto check_completion;
		}
	}

check_completion:
	spin_lock_irqsave(&pxmitpriv->lock_sctx, flags);
	rtw_sctx_done_err(&pxmitbuf->sctx,
		purb->status ? RTW_SCTX_DONE_WRITE_PORT_ERR : RTW_SCTX_DONE_SUCCESS);
	spin_unlock_irqrestore(&pxmitpriv->lock_sctx, flags);

	rtw_free_xmitbuf(pxmitpriv, pxmitbuf);

	/* if(rtw_txframes_pending(rtlpriv)) */
	{
		tasklet_hi_schedule(&pxmitpriv->xmit_tasklet);
	}
}



u32 usb_write_port(struct rtl_priv *rtlpriv, u32 addr, u32 cnt, struct xmit_buf *pxmitbuf)
{
	unsigned long flags;
	unsigned int pipe;
	int status;
	u32 ret = _FAIL, bwritezero = _FALSE;
	PURB	purb = NULL;
	struct rtl_usb	*rtlusb = rtl_usbdev(rtlpriv);
	struct xmit_priv *pxmitpriv = &rtlpriv->xmitpriv;
	struct xmit_frame *pxmitframe = (struct xmit_frame *)pxmitbuf->priv_data;
	struct usb_device *pusbd = rtlusb->udev;
	struct pkt_attrib *pattrib = &pxmitframe->attrib;

	if ((rtlpriv->bDriverStopped) || (rtlpriv->bSurpriseRemoved) ||(rtlpriv->pwrctrlpriv.pnp_bstop_trx)) {
		#ifdef DBG_TX
		DBG_871X(" DBG_TX %s:%d bDriverStopped%d, bSurpriseRemoved:%d, pnp_bstop_trx:%d\n",__FUNCTION__, __LINE__
			,rtlpriv->bDriverStopped, rtlpriv->bSurpriseRemoved, rtlpriv->pwrctrlpriv.pnp_bstop_trx );
		#endif
		rtw_sctx_done_err(&pxmitbuf->sctx, RTW_SCTX_DONE_TX_DENY);
		goto exit;
	}

	spin_lock_irqsave(&pxmitpriv->lock, flags);

	switch(addr) {
		case VO_QUEUE_INX:
			pxmitpriv->voq_cnt++;
			pxmitbuf->flags = VO_QUEUE_INX;
			break;
		case VI_QUEUE_INX:
			pxmitpriv->viq_cnt++;
			pxmitbuf->flags = VI_QUEUE_INX;
			break;
		case BE_QUEUE_INX:
			pxmitpriv->beq_cnt++;
			pxmitbuf->flags = BE_QUEUE_INX;
			break;
		case BK_QUEUE_INX:
			pxmitpriv->bkq_cnt++;
			pxmitbuf->flags = BK_QUEUE_INX;
			break;
		case HIGH_QUEUE_INX:
			pxmitbuf->flags = HIGH_QUEUE_INX;
			break;
		default:
			pxmitbuf->flags = MGT_QUEUE_INX;
			break;
	}

	spin_unlock_irqrestore(&pxmitpriv->lock, flags);

	purb	= pxmitbuf->pxmit_urb[0];

	/* translate DMA FIFO addr to pipehandle */
	pipe = usb_sndbulkpipe(pusbd, rtlusb->Queue2Pipe[addr]);	

#ifdef CONFIG_REDUCE_USB_TX_INT
	if ((pxmitpriv->free_xmitbuf_cnt%NR_XMITBUFF == 0) ||
	   (pxmitbuf->buf_tag > XMITBUF_DATA)) {
		purb->transfer_flags  &=  (~URB_NO_INTERRUPT);
	} else {
		purb->transfer_flags  |=  URB_NO_INTERRUPT;
		/* DBG_8192C("URB_NO_INTERRUPT "); */
	}
#endif

	usb_fill_bulk_urb(purb, pusbd, pipe,
       				pxmitframe->buf_addr, 	/* = pxmitbuf->pbuf */
              			cnt,
              			usb_write_port_complete,
              			pxmitbuf);		/* context is pxmitbuf */
#if 0
	if (bwritezero) {
            purb->transfer_flags |= URB_ZERO_PACKET;
        }
#endif

	status = usb_submit_urb(purb, GFP_ATOMIC);
	if (!status) {
	} else {
		rtw_sctx_done_err(&pxmitbuf->sctx, RTW_SCTX_DONE_WRITE_PORT_ERR);
		DBG_871X("usb_write_port, status=%d\n", status);
		switch (status) {
		case -ENODEV:
			rtlpriv->bDriverStopped=_TRUE;
			break;
		default:
			break;
		}
		goto exit;
	}

	ret= _SUCCESS;

/*
 *  Commented by Albert 2009/10/13
 *  We add the URB_ZERO_PACKET flag to urb so that the host will send the zero packet automatically.
 */
/*
	if(bwritezero == _TRUE)
	{
		usb_bulkout_zero(pintfhdl, addr);
	}
*/

exit:
	if (ret != _SUCCESS)
		rtw_free_xmitbuf(pxmitpriv, pxmitbuf);

	return ret;
}

void usb_write_port_cancel(struct rtl_priv *rtlpriv)
{
	int i, j;
	struct xmit_buf *pxmitbuf = (struct xmit_buf *)rtlpriv->xmitpriv.pxmitbuf;

	DBG_871X("%s \n", __func__);

	rtlpriv->bWritePortCancel = _TRUE;

	for (i=0; i<NR_XMITBUFF; i++) {
		for (j=0; j<8; j++) {
			if (pxmitbuf->pxmit_urb[j]) {
				usb_kill_urb(pxmitbuf->pxmit_urb[j]);
			}
		}
		pxmitbuf++;
	}

	pxmitbuf = (struct xmit_buf*)rtlpriv->xmitpriv.pxmit_extbuf;
	for (i = 0; i < NR_XMIT_EXTBUFF; i++) {
		for (j=0; j<8; j++) {
			if(pxmitbuf->pxmit_urb[j]) {
				usb_kill_urb(pxmitbuf->pxmit_urb[j]);
			}
		}
		pxmitbuf++;
	}
}



static void usb_read_port_complete(struct urb *purb, struct pt_regs *regs)
{
	uint isevt, *pbuf;
	struct recv_buf	*precvbuf = (struct recv_buf *) purb->context;
	struct rtl_priv 		*rtlpriv = (struct rtl_priv *) precvbuf->rtlpriv;
	struct recv_priv	*precvpriv = &rtlpriv->recvpriv;

	/*
	 * spin_lock_irqsave(&precvpriv->lock, &irqL);
	 * precvbuf->irp_pending=_FALSE;
	 * precvpriv->rx_pending_cnt --;
	 * spin_unlock_irqrestore(&precvpriv->lock, &irqL);
	 */

	precvpriv->rx_pending_cnt--;

	/*
	 * if(precvpriv->rx_pending_cnt== 0) {
	 * 	RT_TRACE(_module_hci_ops_os_c_,_drv_err_,("usb_read_port_complete: rx_pending_cnt== 0, set allrxreturnevt!\n"));
	 * 	up(&precvpriv->allrxreturnevt);
	 * }
	 */

	if (rtlpriv->bSurpriseRemoved || rtlpriv->bDriverStopped
	|| rtlpriv->bReadPortCancel) {
#ifdef CONFIG_PREALLOC_RECV_SKB
		precvbuf->reuse = _TRUE;
#else
		if (precvbuf->pskb) {
			DBG_8192C("==> free skb(%p)\n", precvbuf->pskb);
			dev_kfree_skb_any(precvbuf->pskb);
		}
#endif
		DBG_8192C("%s() RX Warning! bDriverStopped(%d) OR bSurpriseRemoved(%d) bReadPortCancel(%d)\n",
		__FUNCTION__, rtlpriv->bDriverStopped, rtlpriv->bSurpriseRemoved, rtlpriv->bReadPortCancel);
		goto exit;
	}

	if (purb->status == 0) {
		/* SUCCESS */
		if ((purb->actual_length > MAX_RECVBUF_SZ)
		 || (purb->actual_length < RXDESC_SIZE)) {
			precvbuf->reuse = _TRUE;
			usb_read_port(rtlpriv, 0, (unsigned char *)precvbuf);
			DBG_8192C("%s()-%d: RX Warning!\n", __FUNCTION__, __LINE__);
		} else {
			rtw_reset_continual_urb_error(rtl_usbdev(rtlpriv));

			precvbuf->transfer_len = purb->actual_length;
			skb_put(precvbuf->pskb, purb->actual_length);
			skb_queue_tail(&precvpriv->rx_skb_queue, precvbuf->pskb);

			if (skb_queue_len(&precvpriv->rx_skb_queue) <= 1)
				tasklet_schedule(&precvpriv->recv_tasklet);

			precvbuf->pskb = NULL;
			precvbuf->reuse = _FALSE;
			usb_read_port(rtlpriv, 0, (unsigned char *)precvbuf);
		}
	} else {
		DBG_8192C("###=> usb_read_port_complete => urb status(%d)\n", purb->status);

		if (rtw_inc_and_chk_continual_urb_error(rtl_usbdev(rtlpriv)) == _TRUE) {
			rtlpriv->bSurpriseRemoved = _TRUE;
		}

		switch (purb->status) {
		case -EINVAL:
		case -EPIPE:
		case -ENODEV:
		case -ESHUTDOWN:
			/* rtlpriv->bSurpriseRemoved=_TRUE; */
		case -ENOENT:
			rtlpriv->bDriverStopped = _TRUE;
			break;
		case -EPROTO:
		case -EILSEQ:
		case -ETIME:
		case -ECOMM:
		case -EOVERFLOW:
			precvbuf->reuse = _TRUE;
			usb_read_port(rtlpriv, 0, (unsigned char *)precvbuf);
			break;
		case -EINPROGRESS:
			DBG_8192C("ERROR: URB IS IN PROGRESS!/n");
			break;
		default:
			break;
		}

	}

exit:
	;
}

uint32_t usb_read_port(struct rtl_priv *rtlpriv, uint32_t cnt, uint8_t *rmem)
{
	int err;
	unsigned int pipe;
	SIZE_PTR tmpaddr = 0;
	SIZE_PTR alignment = 0;
	uint32_t ret = _SUCCESS;
	PURB purb = NULL;
	struct recv_buf	*precvbuf = (struct recv_buf *) rmem;
	struct rtl_usb	*rtlusb = rtl_usbdev(rtlpriv);
	struct recv_priv	*precvpriv = &rtlpriv->recvpriv;
	struct usb_device	*pusbd = rtlusb->udev;
	uint32_t addr = RECV_BULK_IN_ADDR;

	if (rtlpriv->bDriverStopped || rtlpriv->bSurpriseRemoved
	 || rtlpriv->pwrctrlpriv.pnp_bstop_trx) {
		return _FAIL;
	}

#ifdef CONFIG_PREALLOC_RECV_SKB
	if ((precvbuf->reuse == _FALSE) || (precvbuf->pskb == NULL)) {
		precvbuf->pskb = skb_dequeue(&precvpriv->free_recv_skb_queue);
		if (precvbuf->pskb != NULL) {
			precvbuf->reuse = _TRUE;
		}
	}
#endif

	if (precvbuf != NULL) {
		rtl8812au_init_recvbuf(rtlpriv, precvbuf);

		/* re-assign for linux based on skb */
		if ((precvbuf->reuse == _FALSE) || (precvbuf->pskb == NULL)) {
			/* precvbuf->pskb = alloc_skb(MAX_RECVBUF_SZ, GFP_ATOMIC);//don't use this after v2.6.25 */
			precvbuf->pskb = netdev_alloc_skb(rtlpriv->ndev, MAX_RECVBUF_SZ + RECVBUFF_ALIGN_SZ);
			if (precvbuf->pskb == NULL) {
				DBG_8192C("#### usb_read_port() alloc_skb fail!#####\n");
				return _FAIL;
			}

			tmpaddr = (SIZE_PTR)precvbuf->pskb->data;
			alignment = tmpaddr & (RECVBUFF_ALIGN_SZ-1);
			skb_reserve(precvbuf->pskb, (RECVBUFF_ALIGN_SZ - alignment));

			precvbuf->phead = precvbuf->pskb->head;
			precvbuf->pdata = precvbuf->pskb->data;
			precvbuf->ptail = skb_tail_pointer(precvbuf->pskb);
			precvbuf->pend = skb_end_pointer(precvbuf->pskb);
			precvbuf->pbuf = precvbuf->pskb->data;
		} else {
			/* reuse skb */
			precvbuf->phead = precvbuf->pskb->head;
			precvbuf->pdata = precvbuf->pskb->data;
			precvbuf->ptail = skb_tail_pointer(precvbuf->pskb);
			precvbuf->pend = skb_end_pointer(precvbuf->pskb);
			precvbuf->pbuf = precvbuf->pskb->data;

			precvbuf->reuse = _FALSE;
		}

		/*
		 * spin_lock_irqsave(&precvpriv->lock, &irqL);
		 * precvpriv->rx_pending_cnt++;
		 * precvbuf->irp_pending = _TRUE;
		 * spin_unlock_irqrestore(&precvpriv->lock, &irqL);
		 */
		precvpriv->rx_pending_cnt++;

		purb = precvbuf->purb;

		/* translate DMA FIFO addr to pipehandle */
		pipe = usb_rcvbulkpipe(pusbd, rtlusb->RtInPipe[0]);

		usb_fill_bulk_urb(purb, pusbd, pipe,
						precvbuf->pbuf,
						MAX_RECVBUF_SZ,
						usb_read_port_complete,
						precvbuf);	/* context is precvbuf */

		err = usb_submit_urb(purb, GFP_ATOMIC);
		if ((err) && (err != (-EPERM))) {
			DBG_8192C("cannot submit rx in-token(err = 0x%08x),urb_status = %d\n", err, purb->status);
			ret = _FAIL;
		}
	} else {
		ret = _FAIL;
	}

	return ret;
}


static inline int RT_usb_endpoint_dir_in(const struct usb_endpoint_descriptor *epd)
{
	return ((epd->bEndpointAddress & USB_ENDPOINT_DIR_MASK) == USB_DIR_IN);
}

static inline int RT_usb_endpoint_dir_out(const struct usb_endpoint_descriptor *epd)
{
	return ((epd->bEndpointAddress & USB_ENDPOINT_DIR_MASK) == USB_DIR_OUT);
}

static inline int RT_usb_endpoint_xfer_int(const struct usb_endpoint_descriptor *epd)
{
	return ((epd->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK) == USB_ENDPOINT_XFER_INT);
}

static inline int RT_usb_endpoint_xfer_bulk(const struct usb_endpoint_descriptor *epd)
{
 	return ((epd->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK) == USB_ENDPOINT_XFER_BULK);
}

static inline int RT_usb_endpoint_is_bulk_in(const struct usb_endpoint_descriptor *epd)
{
	return (RT_usb_endpoint_xfer_bulk(epd) && RT_usb_endpoint_dir_in(epd));
}

static inline int RT_usb_endpoint_is_bulk_out(const struct usb_endpoint_descriptor *epd)
{
	return (RT_usb_endpoint_xfer_bulk(epd) && RT_usb_endpoint_dir_out(epd));
}

static inline int RT_usb_endpoint_num(const struct usb_endpoint_descriptor *epd)
{
	return epd->bEndpointAddress & USB_ENDPOINT_NUMBER_MASK;
}

static uint8_t rtw_init_intf_priv(struct rtl_usb *dvobj)
{
	uint8_t rst = _SUCCESS;

#ifdef CONFIG_USB_VENDOR_REQ_MUTEX
	mutex_init(&dvobj->usb_vendor_req_mutex);
#endif


#ifdef CONFIG_USB_VENDOR_REQ_BUFFER_PREALLOC
	dvobj->usb_alloc_vendor_req_buf = rtw_zmalloc(MAX_USB_IO_CTL_SIZE);
	if (dvobj->usb_alloc_vendor_req_buf == NULL) {
		DBG_871X("alloc usb_vendor_req_buf failed... /n");
		rst = _FAIL;
		goto exit;
	}
	dvobj->usb_vendor_req_buf  =
		(uint8_t *)N_BYTE_ALIGMENT((SIZE_PTR)(dvobj->usb_alloc_vendor_req_buf ), ALIGNMENT_UNIT);
exit:
#endif

	return rst;

}

static uint8_t rtw_deinit_intf_priv(struct rtl_usb *dvobj)
{
	uint8_t rst = _SUCCESS;

#ifdef CONFIG_USB_VENDOR_REQ_BUFFER_PREALLOC
	if(dvobj->usb_vendor_req_buf)
		rtw_mfree(dvobj->usb_alloc_vendor_req_buf);
#endif

#ifdef CONFIG_USB_VENDOR_REQ_MUTEX
	mutex_destroy(&dvobj->usb_vendor_req_mutex);
#endif

	return rst;
}

static struct rtl_usb *usb_dvobj_init(struct usb_interface *usb_intf, struct rtl_usb *pdvobjpriv)
{
	int	i;
	uint8_t	val8;
	int	status = _FAIL;
	struct usb_device_descriptor 	*pdev_desc;
	struct usb_host_config		*phost_conf;
	struct usb_config_descriptor	*pconf_desc;
	struct usb_host_interface	*phost_iface;
	struct usb_interface_descriptor	*piface_desc;
	struct usb_host_endpoint	*phost_endp;
	struct usb_endpoint_descriptor	*pendp_desc;
	struct usb_device			*pusbd;

	mutex_init(&pdvobjpriv->hw_init_mutex);
	mutex_init(&pdvobjpriv->h2c_fwcmd_mutex);
	mutex_init(&pdvobjpriv->setch_mutex);
	mutex_init(&pdvobjpriv->setbw_mutex);

	spin_lock_init(&pdvobjpriv->lock);

	pdvobjpriv->macid[1] = _TRUE; 	/* macid=1 for bc/mc stainfo */


	pdvobjpriv->intf = usb_intf ;
	pusbd = pdvobjpriv->udev = interface_to_usbdev(usb_intf);
	usb_set_intfdata(usb_intf, pdvobjpriv);

	pdvobjpriv->RtNumInPipes = 0;
	pdvobjpriv->RtNumOutPipes = 0;

	/*
	 * rtlpriv->EepromAddressSize = 6;
	 * pdvobjpriv->nr_endpoint = 6;
	 */

	pdev_desc = &pusbd->descriptor;


	phost_conf = pusbd->actconfig;
	pconf_desc = &phost_conf->desc;


	/*
	 * DBG_871X("\n****** num of altsetting = (%d) ******\n", pusb_interface->num_altsetting);
	 */


	phost_iface = &usb_intf->altsetting[0];
	piface_desc = &phost_iface->desc;


	pdvobjpriv->nr_endpoint = piface_desc->bNumEndpoints;

	/* DBG_871X("\ndump usb_endpoint_descriptor:\n"); */

	for (i = 0; i < pdvobjpriv->nr_endpoint; i++) {
		phost_endp = phost_iface->endpoint + i;
		if (phost_endp) 		{
			pendp_desc = &phost_endp->desc;

			DBG_871X("\nusb_endpoint_descriptor(%d):\n", i);
			DBG_871X("bLength=%x\n",pendp_desc->bLength);
			DBG_871X("bDescriptorType=%x\n",pendp_desc->bDescriptorType);
			DBG_871X("bEndpointAddress=%x\n",pendp_desc->bEndpointAddress);
			/* DBG_871X("bmAttributes=%x\n",pendp_desc->bmAttributes); */
			DBG_871X("wMaxPacketSize=%d\n",le16_to_cpu(pendp_desc->wMaxPacketSize));
			DBG_871X("bInterval=%x\n",pendp_desc->bInterval);
			/* DBG_871X("bRefresh=%x\n",pendp_desc->bRefresh); */
			/* DBG_871X("bSynchAddress=%x\n",pendp_desc->bSynchAddress); */

			if (RT_usb_endpoint_is_bulk_in(pendp_desc)) {
				DBG_871X("RT_usb_endpoint_is_bulk_in = %x\n", RT_usb_endpoint_num(pendp_desc));
				pdvobjpriv->RtInPipe[pdvobjpriv->RtNumInPipes] = RT_usb_endpoint_num(pendp_desc);
				pdvobjpriv->RtNumInPipes++;
			} else if (RT_usb_endpoint_is_bulk_out(pendp_desc)) {
				DBG_871X("RT_usb_endpoint_is_bulk_out = %x\n", RT_usb_endpoint_num(pendp_desc));
				pdvobjpriv->RtOutPipe[pdvobjpriv->RtNumOutPipes] = RT_usb_endpoint_num(pendp_desc);
				pdvobjpriv->RtNumOutPipes++;
			}
			pdvobjpriv->ep_num[i] = RT_usb_endpoint_num(pendp_desc);
		}
	}

	DBG_871X("nr_endpoint=%d, in_num=%d, out_num=%d\n\n", pdvobjpriv->nr_endpoint, pdvobjpriv->RtNumInPipes, pdvobjpriv->RtNumOutPipes);

	switch(pusbd->speed) {
	case USB_SPEED_LOW:
		DBG_871X("USB_SPEED_LOW\n");
		pdvobjpriv->usb_speed = RTW_USB_SPEED_1_1;
		break;
	case USB_SPEED_FULL:
		DBG_871X("USB_SPEED_FULL\n");
		pdvobjpriv->usb_speed = RTW_USB_SPEED_1_1;
		break;
	case USB_SPEED_HIGH:
		DBG_871X("USB_SPEED_HIGH\n");
		pdvobjpriv->usb_speed = RTW_USB_SPEED_2;
		break;
	case USB_SPEED_SUPER:
		DBG_871X("USB_SPEED_SUPER\n");
		pdvobjpriv->usb_speed = RTW_USB_SPEED_3;
		break;
	default:
		DBG_871X("USB_SPEED_UNKNOWN(%x)\n",pusbd->speed);
		pdvobjpriv->usb_speed = RTW_USB_SPEED_UNKNOWN;
		break;
	}

	if (pdvobjpriv->usb_speed == RTW_USB_SPEED_UNKNOWN) {
		DBG_871X("UNKNOWN USB SPEED MODE, ERROR !!!\n");
		goto free_dvobj;
	}

	if (rtw_init_intf_priv(pdvobjpriv) == _FAIL) {
		goto free_dvobj;
	}

	/* .3 misc */
	rtw_reset_continual_urb_error(pdvobjpriv);

	usb_get_dev(pusbd);

	status = _SUCCESS;

free_dvobj:
	if (status != _SUCCESS && pdvobjpriv) {
		usb_set_intfdata(usb_intf, NULL);
		mutex_destroy(&pdvobjpriv->hw_init_mutex);
		mutex_destroy(&pdvobjpriv->h2c_fwcmd_mutex);
		mutex_destroy(&pdvobjpriv->setch_mutex);
		mutex_destroy(&pdvobjpriv->setbw_mutex);
		pdvobjpriv = NULL;
	}
exit:
	return pdvobjpriv;
}

void usb_dvobj_deinit(struct usb_interface *usb_intf)
{
	struct rtl_usb *dvobj = usb_get_intfdata(usb_intf);

	usb_set_intfdata(usb_intf, NULL);
	if (dvobj) {
		if (interface_to_usbdev(usb_intf)->state != USB_STATE_NOTATTACHED) {
			/*
			 * If we didn't unplug usb dongle and remove/insert modlue, driver fails on sitesurvey for the first time when device is up .
			 * Reset usb port for sitesurvey fail issue. 2009.8.13, by Thomas
			 */
			DBG_871X("usb attached..., try to reset usb device\n");
			usb_reset_device(interface_to_usbdev(usb_intf));
		}
		rtw_deinit_intf_priv(dvobj);
		mutex_destroy(&dvobj->hw_init_mutex);
		mutex_destroy(&dvobj->h2c_fwcmd_mutex);
		mutex_destroy(&dvobj->setch_mutex);
		mutex_destroy(&dvobj->setbw_mutex);
	}

	/* DBG_871X("%s %d\n", __func__, atomic_read(&usb_intf->dev.kobj.kref.refcount)); */
	usb_put_dev(interface_to_usbdev(usb_intf));


}


static void rtw_decide_chip_type_by_usb_info(struct rtl_priv *rtlpriv, const struct usb_device_id *pdid)
{
	rtlpriv->chip_type = pdid->driver_info;

	if (rtlpriv->chip_type == RTL8812) {
		rtlpriv->rtlhal.hw_type = HARDWARE_TYPE_RTL8812AU;
		DBG_871X("CHIP TYPE: RTL8812\n");
	} else if (rtlpriv->chip_type == RTL8821) {
		/* rtlpriv->HardwareType = HARDWARE_TYPE_RTL8811AU; */
		rtlpriv->rtlhal.hw_type = HARDWARE_TYPE_RTL8821U;
		DBG_871X("CHIP TYPE: RTL8811AU or RTL8821U\n");
	}
}


static void usb_intf_start(struct rtl_priv *rtlpriv)
{
	rtw_hal_inirp_init(rtlpriv);
}

static void usb_intf_stop(struct rtl_priv *rtlpriv)
{
	/* disabel_hw_interrupt */
	if (rtlpriv->bSurpriseRemoved == _FALSE) {
		/* device still exists, so driver can do i/o operation */
		/* TODO: */
	}

	/* cancel in irp */
	rtw_hal_inirp_deinit(rtlpriv);

	/* cancel out irp */
	usb_write_port_cancel(rtlpriv);

	/* todo:cancel other irps */

}

static void rtw_dev_unload(struct rtl_priv *rtlpriv)
{
	struct net_device *ndev= (struct net_device*)rtlpriv->ndev;
	uint8_t val8;

	if (rtlpriv->bup == _TRUE) {
		DBG_871X("===> rtw_dev_unload\n");

		rtlpriv->bDriverStopped = _TRUE;
		/* s3. */
		if(rtlpriv->intf_stop)
			rtlpriv->intf_stop(rtlpriv);

		/* s4. */
		if(!rtlpriv->pwrctrlpriv.bInternalAutoSuspend )
		rtw_stop_drv_threads(rtlpriv);


		/* s5. */
		if(rtlpriv->bSurpriseRemoved == _FALSE) {
			rtw_hal_deinit(rtlpriv);
			rtlpriv->bSurpriseRemoved = _TRUE;
		}

		rtlpriv->bup = _FALSE;
	} else {
		;
	}

	DBG_871X("<=== rtw_dev_unload\n");
}


static int rtl8821au_suspend(struct usb_interface *pusb_intf, pm_message_t message)
{
	struct rtl_usb *dvobj = usb_get_intfdata(pusb_intf);
	struct rtl_priv *rtlpriv = dvobj->rtlpriv;
	struct net_device *ndev = rtlpriv->ndev;
	struct mlme_priv *pmlmepriv = &rtlpriv->mlmepriv;
	struct pwrctrl_priv *pwrpriv = &rtlpriv->pwrctrlpriv;
	struct usb_device *usb_dev = interface_to_usbdev(pusb_intf);

	int ret = 0;
	u32 start_time = jiffies;

	DBG_871X("==> %s (%s:%d)\n",__FUNCTION__, current->comm, current->pid);

	if((!rtlpriv->bup) || (rtlpriv->bDriverStopped)||(rtlpriv->bSurpriseRemoved)) {
		DBG_871X("rtlpriv->bup=%d bDriverStopped=%d bSurpriseRemoved = %d\n",
			rtlpriv->bup, rtlpriv->bDriverStopped,rtlpriv->bSurpriseRemoved);
		goto exit;
	}

	if(pwrpriv->bInternalAutoSuspend )
	{
	}
	pwrpriv->bInSuspend = _TRUE;
	rtw_cancel_all_timer(rtlpriv);
	LeaveAllPowerSaveMode(rtlpriv);

	down(&pwrpriv->lock);
	/*
	 * rtlpriv->net_closed = _TRUE;
	 * s1.
	 */
	if (ndev) {
		netif_carrier_off(ndev);
		rtw_netif_stop_queue(ndev);
	}

	/* s2. */
	rtw_disassoc_cmd(rtlpriv, 0, _FALSE);

	/* s2-2.  indicate disconnect to os */
	rtw_indicate_disconnect(rtlpriv);
	/* s2-3. */
	rtw_free_assoc_resources(rtlpriv, 1);
#ifdef CONFIG_AUTOSUSPEND
	if(!pwrpriv->bInternalAutoSuspend )
#endif
	/* s2-4. */
	rtw_free_network_queue(rtlpriv, _TRUE);

	rtw_dev_unload(rtlpriv);
#ifdef CONFIG_AUTOSUSPEND
	pwrpriv->rf_pwrstate = rf_off;
	pwrpriv->bips_processing = _FALSE;
#endif
	up(&pwrpriv->lock);

	if (check_fwstate(pmlmepriv, _FW_UNDER_SURVEY))
		rtw_indicate_scan_done(rtlpriv, 1);

	if (check_fwstate(pmlmepriv, _FW_UNDER_LINKING))
		rtw_indicate_disconnect(rtlpriv);

exit:
	DBG_871X("<===  %s return %d.............. in %dms\n", __FUNCTION__
		, ret, rtw_get_passing_time_ms(start_time));

	return ret;
}

static int rtl8821au_resume(struct usb_interface *pusb_intf)
{
	struct rtl_usb *dvobj = usb_get_intfdata(pusb_intf);
	struct rtl_priv *rtlpriv = dvobj->rtlpriv;

	return rtw_resume_process(rtlpriv);
}

int rtw_resume_process(struct rtl_priv *rtlpriv)
{
	struct net_device *ndev;
	struct pwrctrl_priv *pwrpriv;
	int ret = -1;
	u32 start_time = jiffies;

	DBG_871X("==> %s (%s:%d)\n",__FUNCTION__, current->comm, current->pid);

	if(rtlpriv) {
		ndev= rtlpriv->ndev;
		pwrpriv = &rtlpriv->pwrctrlpriv;
	} else
		goto exit;

	down(&pwrpriv->lock);
	rtw_reset_drv_sw(rtlpriv);
	pwrpriv->bkeepfwalive = _FALSE;

	DBG_871X("bkeepfwalive(%x)\n",pwrpriv->bkeepfwalive);
	if(pm_netdev_open(ndev,_TRUE) != 0){
		up(&pwrpriv->lock);
		goto exit;
	}

	netif_device_attach(ndev);
	netif_carrier_on(ndev);

#ifdef CONFIG_AUTOSUSPEND
	if (pwrpriv->bInternalAutoSuspend) {
		pwrpriv->bInternalAutoSuspend = _FALSE;
		pwrpriv->brfoffbyhw = _FALSE;
		DBG_871X("enc_algorithm(%x),wepkeymask(%x)\n",
			rtlpriv->securitypriv.dot11PrivacyAlgrthm,pwrpriv->wepkeymask);
		if  ((_WEP40_ == rtlpriv->securitypriv.dot11PrivacyAlgrthm) ||
			(_WEP104_ == rtlpriv->securitypriv.dot11PrivacyAlgrthm)) {
			sint keyid;

			for (keyid = 0; keyid < 4; keyid++) {
				if (pwrpriv->wepkeymask & BIT(keyid)) {
					if (keyid == rtlpriv->securitypriv.dot11PrivacyKeyIndex)
						rtw_set_key(rtlpriv,&rtlpriv->securitypriv, keyid, 1);
					else
						rtw_set_key(rtlpriv,&rtlpriv->securitypriv, keyid, 0);
				}
			}
		}
	}
#endif
	up(&pwrpriv->lock);

	ret = 0;
exit:
	pwrpriv->bInSuspend = _FALSE;
	DBG_871X("<===  %s return %d.............. in %dms\n", __FUNCTION__
		, ret, rtw_get_passing_time_ms(start_time));

	return ret;
}

#ifdef CONFIG_AUTOSUSPEND
void autosuspend_enter(struct rtl_priv* rtlpriv)
{
	struct pwrctrl_priv *pwrpriv = &rtlpriv->pwrctrlpriv;
	struct rtl_usb *dvobj = rtl_usbdev(rtlpriv);

	DBG_871X("==>autosuspend_enter...........\n");

	pwrpriv->bInternalAutoSuspend = _TRUE;
	pwrpriv->bips_processing = _TRUE;

	if (rf_off == pwrpriv->change_rfpwrstate) {
		usb_enable_autosuspend(dvobj->pusbdev);

			usb_autopm_put_interface(dvobj->pusbintf);
	}
	DBG_871X("...pm_usage_cnt(%d).....\n", atomic_read(&(dvobj->pusbintf->pm_usage_cnt)));

}

int autoresume_enter(struct rtl_priv* rtlpriv)
{
	int result = _SUCCESS;
	struct pwrctrl_priv *pwrpriv = &rtlpriv->pwrctrlpriv;
	struct security_priv* psecuritypriv=&(rtlpriv->securitypriv);
	struct mlme_ext_priv	*pmlmeext = &rtlpriv->mlmeextpriv;
	struct mlme_ext_info	*pmlmeinfo = &(pmlmeext->mlmext_info);
	struct rtl_usb *dvobj = rtl_usbdev(rtlpriv);

	DBG_871X("====> autoresume_enter \n");

	if (rf_off == pwrpriv->rf_pwrstate) {
		pwrpriv->ps_flag = _FALSE;
			if (usb_autopm_get_interface(dvobj->pusbintf) < 0) {
				DBG_871X( "can't get autopm: %d\n", result);
				result = _FAIL;
				goto error_exit;
			}

		DBG_871X("...pm_usage_cnt(%d).....\n", atomic_read(&(dvobj->pusbintf->pm_usage_cnt)));
	}
	DBG_871X("<==== autoresume_enter \n");
error_exit:

	return result;
}
#endif

/*
 * ULLI messy but needed
 */

int netdev_open(struct net_device *ndev);
int netdev_close(struct net_device *ndev);
int rtw_net_set_mac_address(struct net_device *ndev, void *p);
struct net_device_stats *rtw_net_get_stats(struct net_device *ndev);
uint loadparam(struct rtl_priv *rtlpriv, struct net_device *ndev);

static const struct net_device_ops rtw_netdev_ops = {
	.ndo_open = netdev_open,
	.ndo_stop = netdev_close,
	.ndo_start_xmit = rtw_xmit_entry,
	.ndo_set_mac_address = rtw_net_set_mac_address,
	.ndo_get_stats = rtw_net_get_stats,
	.ndo_do_ioctl = rtw_ioctl,
};


/*
 * drv_init() - a device potentially for us
 *
 * notes: drv_init() is called when the bus driver has located a card for us to support.
 *        We accept the new device by returning 0.
*/

struct rtl_priv  *rtw_sw_export = NULL;

static char *ifname = "wlan%d";

static int rtw_init_netdev_name(struct net_device *ndev, const char *ifname)
{
	if (dev_alloc_name(ndev, ifname) < 0)
		/*
		 * RT_TRACE(_module_os_intfs_c_, _drv_err_, ("dev_alloc_name, fail!\n"));
		 */

	netif_carrier_off(ndev);
	return 0;
}

int rtw_usb_probe(struct usb_interface *pusb_intf, const struct usb_device_id *pdid, 
	struct rtl_hal_cfg *rtl_hal_cfg)
{
	struct rtl_usb *rtlusb;
	struct usb_device *udev;
	struct rtl_priv *rtlpriv = NULL;
	struct net_device *ndev = NULL;
	int status = _FAIL;

	ndev = alloc_etherdev_mq(sizeof(*rtlpriv), 4);
	if (!ndev)
		goto exit;

	rtlpriv = netdev_priv(ndev);
	rtlusb = rtl_usbdev(rtlpriv);
	usb_dvobj_init(pusb_intf, rtlusb);

	rtlpriv = netdev_priv(ndev);
	rtlpriv->ndev = ndev;

	rtlusb->rtlpriv = rtlpriv;
	udev = rtlusb->udev;

	/* ULLI: Init IO handler */
	_rtl_usb_io_handler_init(&udev->dev, rtlpriv);

	rtlpriv->bDriverStopped=_TRUE;

	/* step 1-1., decide the chip_type via driver_info */
	rtlpriv->rtlhal.interface = INTF_USB;
	rtw_decide_chip_type_by_usb_info(rtlpriv, pdid);

	/* ndev->init = NULL; */

	DBG_871X("register rtw_netdev_ops to netdev_ops\n");
	ndev->netdev_ops = &rtw_netdev_ops;

#ifdef CONFIG_TCP_CSUM_OFFLOAD_TX
	ndev->features |= NETIF_F_IP_CSUM;
#endif
	/* ndev->tx_timeout = NULL; */
	ndev->watchdog_timeo = HZ*3; /* 3 second timeout */
#ifdef CONFIG_WIRELESS_EXT
	ndev->wireless_handlers = (struct iw_handler_def *)&rtw_handlers_def;
#endif


	/* step 2. */
	loadparam(rtlpriv, ndev);

	SET_NETDEV_DEV(ndev, dvobj_to_dev(rtlusb));
	rtlpriv = rtl_priv(ndev);

	/* step 2. hook cfg->ops, allocate HalData */
	/* hal_set_hal_ops(rtlpriv); */
	rtlpriv->HalData = rtw_zmalloc(sizeof( struct _rtw_hal));
	if (rtlpriv->HalData == NULL) {
		DBG_8192C("cant not alloc memory for HAL DATA \n");
	}

	rtlpriv->cfg = rtl_hal_cfg;

	rtlpriv->intf_start=&usb_intf_start;
	rtlpriv->intf_stop=&usb_intf_stop;

	/* step read_chip_version */
	rtw_hal_read_chip_version(rtlpriv);

	/* step usb endpoint mapping */
	rtw_hal_chip_configure(rtlpriv);

	/* step read efuse/eeprom data and get mac_addr */
	rtw_hal_read_chip_info(rtlpriv);

	/* step 5. */
	if(rtl8821au_init_sw_vars(ndev) ==_FAIL) {
		goto free_hal_data;
	}

#ifdef CONFIG_PM
	if (rtlpriv->pwrctrlpriv.bSupportRemoteWakeup) {
		rtlusb->udev->do_remote_wakeup=1;
		pusb_intf->needs_remote_wakeup = 1;
		device_init_wakeup(&pusb_intf->dev, 1);
		DBG_871X("\n  rtlpriv->pwrctrlpriv.bSupportRemoteWakeup~~~~~~\n");
		DBG_871X("\n  rtlpriv->pwrctrlpriv.bSupportRemoteWakeup~~~[%d]~~~\n",device_may_wakeup(&pusb_intf->dev));
	}
#endif

#ifdef CONFIG_AUTOSUSPEND
	if (rtlpriv->registrypriv.power_mgnt != PS_MODE_ACTIVE) {
		if(rtlpriv->registrypriv.usbss_enable ){ 	/* autosuspend (2s delay) */
			dvobj->pusbdev->dev.power.autosuspend_delay = 0 * HZ;//15 * HZ; idle-delay time

			usb_enable_autosuspend(dvobj->pusbdev);

			/* usb_autopm_get_interface(rtl_usbdev(rtlpriv)->pusbintf );//init pm_usage_cnt ,let it start from 1 */

			DBG_871X("%s...pm_usage_cnt(%d).....\n",__FUNCTION__,atomic_read(&(dvobj->pusbintf ->pm_usage_cnt)));
		}
	}
#endif
	/* 2012-07-11 Move here to prevent the 8723AS-VAU BT auto suspend influence */
	if (usb_autopm_get_interface(pusb_intf) < 0) {
		DBG_871X( "can't get autopm: \n");
	}

	/*  set mac addr */
	rtw_init_netdev_name(ndev, ifname);
	rtw_macaddr_cfg(rtlpriv->eeprompriv.mac_addr);

	DBG_871X("bDriverStopped:%d, bSurpriseRemoved:%d, bup:%d, hw_init_completed:%d\n"
		, rtlpriv->bDriverStopped
		, rtlpriv->bSurpriseRemoved
		, rtlpriv->bup
		, rtlpriv->hw_init_completed
	);

	status = _SUCCESS;

	netif_carrier_off(rtlpriv->ndev);
	/* rtw_netif_stop_queue(ndev); */

/*
 * 	ULLI : for support older kernel < 3.14,
 *	ether_addr_copy(ndev->dev_addr, rtlpriv->eeprompriv.mac_addr);
 */
	memcpy(ndev->dev_addr,  rtlpriv->eeprompriv.mac_addr, ETH_ALEN);

	/* Tell the network stack we exist */
	if (register_netdev(rtlpriv->ndev) != 0) {
		DBG_871X(FUNC_NDEV_FMT "Failed!\n", FUNC_NDEV_ARG(ndev));
		status = _FAIL;
		goto free_hal_data;
	}


	return 0;
	
free_dvobj:
	usb_dvobj_deinit(pusb_intf);
	
free_hal_data:
	if (status != _SUCCESS && rtlpriv->HalData)
		rtw_mfree(rtlpriv->HalData);

free_adapter:
	if (status != _SUCCESS) {
		if (ndev)
			free_netdev(ndev);

		rtlpriv = NULL;
	}
exit:
	return -ENODEV;
}

void rtw_usb_if1_deinit(struct rtl_priv *rtlpriv)
{
	struct net_device *ndev = rtlpriv->ndev;
	struct mlme_priv *pmlmepriv= &rtlpriv->mlmepriv;

	if(check_fwstate(pmlmepriv, _FW_LINKED))
		rtw_disassoc_cmd(rtlpriv, 0, _FALSE);


#ifdef CONFIG_AP_MODE
	free_mlme_ap_info(rtlpriv);
#endif

	if (rtlpriv->DriverState != DRIVER_DISAPPEAR) {
		if (ndev) {
			unregister_netdev(ndev); /* will call netdev_close() */
			rtw_proc_remove_one(ndev);
		}
	}

	rtw_cancel_all_timer(rtlpriv);
	rtw_dev_unload(rtlpriv);

	DBG_871X("+r871xu_dev_remove, hw_init_completed=%d\n", rtlpriv->hw_init_completed);

	rtw_free_drv_sw(rtlpriv);

	if(ndev)
		free_netdev(ndev);
}


static void dump_usb_interface(struct usb_interface *usb_intf)
{
	int	i;
	uint8_t	val8;

	struct usb_device				*udev = interface_to_usbdev(usb_intf);
	struct usb_device_descriptor 	*dev_desc = &udev->descriptor;

	struct usb_host_config			*act_conf = udev->actconfig;
	struct usb_config_descriptor	*act_conf_desc = &act_conf->desc;

	struct usb_host_interface		*host_iface;
	struct usb_interface_descriptor	*iface_desc;
	struct usb_host_endpoint		*host_endp;
	struct usb_endpoint_descriptor	*endp_desc;

#if 1 /* The usb device this usb interface belongs to */
	DBG_871X("usb_interface:%p, usb_device:%p(num:%d, path:%s), usb_device_descriptor:%p\n", usb_intf, udev, udev->devnum, udev->devpath, dev_desc);
	DBG_871X("bLength:%u\n", dev_desc->bLength);
	DBG_871X("bDescriptorType:0x%02x\n", dev_desc->bDescriptorType);
	DBG_871X("bcdUSB:0x%04x\n", le16_to_cpu(dev_desc->bcdUSB));
	DBG_871X("bDeviceClass:0x%02x\n", dev_desc->bDeviceClass);
	DBG_871X("bDeviceSubClass:0x%02x\n", dev_desc->bDeviceSubClass);
	DBG_871X("bDeviceProtocol:0x%02x\n", dev_desc->bDeviceProtocol);
	DBG_871X("bMaxPacketSize0:%u\n", dev_desc->bMaxPacketSize0);
	DBG_871X("idVendor:0x%04x\n", le16_to_cpu(dev_desc->idVendor));
	DBG_871X("idProduct:0x%04x\n", le16_to_cpu(dev_desc->idProduct));
	DBG_871X("bcdDevice:0x%04x\n", le16_to_cpu(dev_desc->bcdDevice));
	DBG_871X("iManufacturer:0x02%x\n", dev_desc->iManufacturer);
	DBG_871X("iProduct:0x%02x\n", dev_desc->iProduct);
	DBG_871X("iSerialNumber:0x%02x\n", dev_desc->iSerialNumber);
	DBG_871X("bNumConfigurations:%u\n", dev_desc->bNumConfigurations);
#endif


#if 1 /* The acting usb_config_descriptor */
	DBG_871X("\nact_conf_desc:%p\n", act_conf_desc);
	DBG_871X("bLength:%u\n", act_conf_desc->bLength);
	DBG_871X("bDescriptorType:0x%02x\n", act_conf_desc->bDescriptorType);
	DBG_871X("wTotalLength:%u\n", le16_to_cpu(act_conf_desc->wTotalLength));
	DBG_871X("bNumInterfaces:%u\n", act_conf_desc->bNumInterfaces);
	DBG_871X("bConfigurationValue:0x%02x\n", act_conf_desc->bConfigurationValue);
	DBG_871X("iConfiguration:0x%02x\n", act_conf_desc->iConfiguration);
	DBG_871X("bmAttributes:0x%02x\n", act_conf_desc->bmAttributes);
	DBG_871X("bMaxPower=%u\n", act_conf_desc->bMaxPower);
#endif


	DBG_871X("****** num of altsetting = (%d) ******/\n", usb_intf->num_altsetting);
	/* Get he host side alternate setting (the current alternate setting) for this interface*/
	host_iface = usb_intf->cur_altsetting;
	iface_desc = &host_iface->desc;

#if 1 /* The current alternate setting*/
	DBG_871X("\nusb_interface_descriptor:%p:\n", iface_desc);
	DBG_871X("bLength:%u\n", iface_desc->bLength);
	DBG_871X("bDescriptorType:0x%02x\n", iface_desc->bDescriptorType);
	DBG_871X("bInterfaceNumber:0x%02x\n", iface_desc->bInterfaceNumber);
	DBG_871X("bAlternateSetting=%x\n", iface_desc->bAlternateSetting);
	DBG_871X("bNumEndpoints=%x\n", iface_desc->bNumEndpoints);
	DBG_871X("bInterfaceClass=%x\n", iface_desc->bInterfaceClass);
	DBG_871X("bInterfaceSubClass=%x\n", iface_desc->bInterfaceSubClass);
	DBG_871X("bInterfaceProtocol=%x\n", iface_desc->bInterfaceProtocol);
	DBG_871X("iInterface=%x\n", iface_desc->iInterface);
#endif


#if 1
	/* DBG_871X("\ndump usb_endpoint_descriptor:\n"); */

	for (i = 0; i < iface_desc->bNumEndpoints; i++) {
		host_endp = host_iface->endpoint + i;
		if (host_endp) {
			endp_desc = &host_endp->desc;

			DBG_871X("\nusb_endpoint_descriptor(%d):\n", i);
			DBG_871X("bLength=%x\n",endp_desc->bLength);
			DBG_871X("bDescriptorType=%x\n",endp_desc->bDescriptorType);
			DBG_871X("bEndpointAddress=%x\n",endp_desc->bEndpointAddress);
			DBG_871X("bmAttributes=%x\n",endp_desc->bmAttributes);
			DBG_871X("wMaxPacketSize=%x\n",endp_desc->wMaxPacketSize);
			DBG_871X("wMaxPacketSize=%x\n",le16_to_cpu(endp_desc->wMaxPacketSize));
			DBG_871X("bInterval=%x\n",endp_desc->bInterval);
			/* DBG_871X("bRefresh=%x\n",pendp_desc->bRefresh); */
			/* DBG_871X("bSynchAddress=%x\n",pendp_desc->bSynchAddress); */

			if (RT_usb_endpoint_is_bulk_in(endp_desc)) {
				DBG_871X("RT_usb_endpoint_is_bulk_in = %x\n", RT_usb_endpoint_num(endp_desc));
				/* pdvobjpriv->RtNumInPipes++; */
			} else if (RT_usb_endpoint_is_bulk_out(endp_desc)) {
				DBG_871X("RT_usb_endpoint_is_bulk_out = %x\n", RT_usb_endpoint_num(endp_desc));
				/* pdvobjpriv->RtNumOutPipes++; */
			}
			/* pdvobjpriv->ep_num[i] = RT_usb_endpoint_num(pendp_desc); */
		}
	}

	/*
	 * DBG_871X("nr_endpoint=%d, in_num=%d, out_num=%d\n\n", pdvobjpriv->nr_endpoint, pdvobjpriv->RtNumInPipes, pdvobjpriv->RtNumOutPipes);
	 */
#endif

	if (udev->speed == USB_SPEED_HIGH)
		DBG_871X("USB_SPEED_HIGH\n");
	else
		DBG_871X("NON USB_SPEED_HIGH\n");

}

/*
 * dev_remove() - our device is being removed
*/
/*
 * rmmod module & unplug(SurpriseRemoved) will call r871xu_dev_remove() => how to recognize both
 */
void rtw_usb_disconnect(struct usb_interface *pusb_intf)
{
	struct rtl_usb *dvobj = usb_get_intfdata(pusb_intf);
	struct rtl_priv *rtlpriv = dvobj->rtlpriv;
	struct net_device *ndev = rtlpriv->ndev;
	struct mlme_priv *pmlmepriv= &rtlpriv->mlmepriv;

	DBG_871X("+rtw_dev_remove\n");


	rtw_pm_set_ips(rtlpriv, IPS_NONE);
	rtw_pm_set_lps(rtlpriv, PS_MODE_ACTIVE);

	LeaveAllPowerSaveMode(rtlpriv);

	rtw_usb_if1_deinit(rtlpriv);

	usb_dvobj_deinit(pusb_intf);

	DBG_871X("-r871xu_dev_remove, done\n");

	return;
}

