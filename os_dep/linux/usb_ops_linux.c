/******************************************************************************
 *
 * Copyright(c) 2007 - 2012 Realtek Corporation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110, USA
 *
 *******************************************************************************/
#define _USB_OPS_LINUX_C_

#include <drv_types.h>
#include <hal_data.h>
#include <../rtl8821au/trx.h>


static uint8_t usb_read8(struct intf_hdl *pintfhdl, uint32_t addr)
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

	usbctrl_vendorreq(pintfhdl, request, wvalue, index, &data, len, requesttype);

	return data;

}

static u16 usb_read16(struct intf_hdl *pintfhdl, uint32_t addr)
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

	usbctrl_vendorreq(pintfhdl, request, wvalue, index, &data, len, requesttype);

	return data;

}

static uint32_t usb_read32(struct intf_hdl *pintfhdl, uint32_t addr)
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

	usbctrl_vendorreq(pintfhdl, request, wvalue, index, &data, len, requesttype);


	return data;

}

static int usb_write8(struct intf_hdl *pintfhdl, uint32_t addr, uint8_t val)
{
	uint8_t request;
	uint8_t requesttype;
	u16 wvalue;
	u16 index;
	u16 len;
	uint8_t data;
	int ret;

	request = 0x05;
	requesttype = 0x00;		/* write_out */
	index = 0;			/* n/a */

	wvalue = (u16)(addr&0x0000ffff);
	len = 1;

	data = val;

	 ret = usbctrl_vendorreq(pintfhdl, request, wvalue, index, &data, len, requesttype);

	return ret;

}

static int usb_write16(struct intf_hdl *pintfhdl, uint32_t addr, u16 val)
{
	uint8_t request;
	uint8_t requesttype;
	u16 wvalue;
	u16 index;
	u16 len;
	u16 data;
	int ret;

	request = 0x05;
	requesttype = 0x00;	/* write_out */
	index = 0;		/* n/a */

	wvalue = (u16)(addr&0x0000ffff);
	len = 2;

	data = val;

	ret = usbctrl_vendorreq(pintfhdl, request, wvalue, index, &data, len, requesttype);

	return ret;

}

static int usb_write32(struct intf_hdl *pintfhdl, uint32_t addr, uint32_t val)
{
	uint8_t request;
	uint8_t requesttype;
	u16 wvalue;
	u16 index;
	u16 len;
	uint32_t data;
	int ret;

	request = 0x05;
	requesttype = 0x00;	/* write_out */
	index = 0;		/* n/a */

	wvalue = (u16)(addr&0x0000ffff);
	len = 4;
	data = val;

	ret = usbctrl_vendorreq(pintfhdl, request, wvalue, index, &data, len, requesttype);

	return ret;

}

static int usb_writeN(struct intf_hdl *pintfhdl, uint32_t addr, uint32_t length, uint8_t *pdata)
{
	uint8_t request;
	uint8_t requesttype;
	u16 wvalue;
	u16 index;
	u16 len;
	uint8_t buf[VENDOR_CMD_MAX_DATA_LEN] = {0};
	int ret;

	request = 0x05;
	requesttype = 0x00;	/* write_out */
	index = 0;		/* n/a */

	wvalue = (u16)(addr&0x0000ffff);
	len = length;
	memcpy(buf, pdata, len);

	ret = usbctrl_vendorreq(pintfhdl, request, wvalue, index, buf, len, requesttype);

	_func_exit_;

	return ret;

}


static int32_t pre_recv_entry(struct recv_frame *precvframe, uint8_t *pphy_status)
{
	int32_t ret = _SUCCESS;

	return ret;
}

static int recvbuf2recvframe(struct rtl_priv *padapter, struct sk_buff *pskb)
{
	uint8_t	*pbuf;
	uint8_t	pkt_cnt = 0;
	uint32_t	pkt_offset;
	int32_t	transfer_len;
	uint8_t				*pphy_status = NULL;
	struct recv_frame	*precvframe = NULL;
	struct rx_pkt_attrib	*pattrib = NULL;
	 struct _rtw_hal	*pHalData = GET_HAL_DATA(padapter);
	struct recv_priv	*precvpriv = &padapter->recvpriv;
	struct __queue			*pfree_recv_queue = &precvpriv->free_recv_queue;

	transfer_len = (int32_t)pskb->len;
	pbuf = pskb->data;

#ifdef CONFIG_USB_RX_AGGREGATION
	pkt_cnt = GET_RX_STATUS_DESC_USB_AGG_PKTNUM(pbuf);
#endif

	do {
		precvframe = rtw_alloc_recvframe(pfree_recv_queue);
		if (precvframe == NULL) {
			DBG_8192C("%s()-%d: rtw_alloc_recvframe() failed! RX Drop!\n", __FUNCTION__, __LINE__);
			goto _exit_recvbuf2recvframe;
		}

		INIT_LIST_HEAD(&precvframe->list);
		precvframe->precvbuf = NULL;	/* can't access the precvbuf for new arch. */
		precvframe->len = 0;

		rtl8812_query_rx_desc_status(precvframe, pbuf);

		pattrib = &precvframe->attrib;

		if ((pattrib->crc_err) || (pattrib->icv_err)) {
			DBG_8192C("%s: RX Warning! crc_err=%d icv_err=%d, skip!\n", __FUNCTION__, pattrib->crc_err, pattrib->icv_err);

			rtw_free_recvframe(precvframe, pfree_recv_queue);
			goto _exit_recvbuf2recvframe;
		}

		pkt_offset = RXDESC_SIZE + pattrib->drvinfo_sz + pattrib->shift_sz + pattrib->pkt_len;

		if ((pattrib->pkt_len <= 0) || (pkt_offset > transfer_len)) {
			DBG_8192C("%s()-%d: RX Warning!,pkt_len<=0 or pkt_offset> transfer_len \n", __FUNCTION__, __LINE__);
			rtw_free_recvframe(precvframe, pfree_recv_queue);
			goto _exit_recvbuf2recvframe;
		}

		if (rtw_os_alloc_recvframe(padapter, precvframe,
		    (pbuf+pattrib->shift_sz + pattrib->drvinfo_sz + RXDESC_SIZE),
		     pskb) == _FAIL) {
			rtw_free_recvframe(precvframe, pfree_recv_queue);

			goto _exit_recvbuf2recvframe;
		}

		recvframe_put(precvframe, pattrib->pkt_len);
		/* recvframe_pull(precvframe, drvinfo_sz + RXDESC_SIZE); */

		if (pattrib->pkt_rpt_type == NORMAL_RX) {
			/* Normal rx packet */
			if (pattrib->physt)
				pphy_status = (pbuf + RXDESC_OFFSET);

			if (pattrib->physt && pphy_status)
				rtl8812_query_rx_phy_status(precvframe, pphy_status);

			if (rtw_recv_entry(precvframe) != _SUCCESS) {
				;
			}

		} else {
			/* pkt_rpt_type == TX_REPORT1-CCX, TX_REPORT2-TX RTP,HIS_REPORT-USB HISR RTP */

			if (pattrib->pkt_rpt_type == C2H_PACKET) {
				/* DBG_8192C("rx C2H_PACKET \n"); */
				/* C2HPacketHandler_8812A(padapter,precvframe->u.hdr.rx_data,pattrib->pkt_len); */
			}
			/* enqueue recvframe to txrtp queue */
			else if (pattrib->pkt_rpt_type == TX_REPORT1) {
				DBG_8192C("rx CCX \n");
			}
			else if (pattrib->pkt_rpt_type == TX_REPORT2) {
				/* DBG_8192C("rx TX RPT \n"); */
			}
			/*else if(pattrib->pkt_rpt_type == HIS_REPORT)
			{
				//DBG_8192C("%s , rx USB HISR \n",__FUNCTION__);
			}*/
			rtw_free_recvframe(precvframe, pfree_recv_queue);

		}

#ifdef CONFIG_USB_RX_AGGREGATION
		/*  jaguar 8-byte alignment */
		pkt_offset = (u16)_RND8(pkt_offset);
		pkt_cnt--;
		pbuf += pkt_offset;
#endif
		transfer_len -= pkt_offset;
		precvframe = NULL;

	} while (transfer_len > 0);

_exit_recvbuf2recvframe:

	return _SUCCESS;
}

void rtl8812au_recv_tasklet(void *priv)
{
	struct sk_buff 		*pskb;
	struct rtl_priv		*padapter = (struct rtl_priv *)priv;
	struct recv_priv	*precvpriv = &padapter->recvpriv;

	while (NULL != (pskb = skb_dequeue(&precvpriv->rx_skb_queue))) {
		if ((padapter->bDriverStopped == _TRUE)
		|| (padapter->bSurpriseRemoved == _TRUE)) {
			DBG_8192C("recv_tasklet => bDriverStopped or bSurpriseRemoved \n");
			dev_kfree_skb_any(pskb);
			break;
		}

		recvbuf2recvframe(padapter, pskb);

#ifdef CONFIG_PREALLOC_RECV_SKB

		skb_reset_tail_pointer(pskb);
		pskb->len = 0;
		skb_queue_tail(&precvpriv->free_recv_skb_queue, pskb);
#else
		dev_kfree_skb_any(pskb);
#endif

	}

}


static void usb_read_port_complete(struct urb *purb, struct pt_regs *regs)
{
	_irqL irqL;
	uint isevt, *pbuf;
	struct recv_buf	*precvbuf = (struct recv_buf *) purb->context;
	struct rtl_priv 		*padapter = (struct rtl_priv *) precvbuf->adapter;
	struct recv_priv	*precvpriv = &padapter->recvpriv;

	/*
	 * _enter_critical(&precvpriv->lock, &irqL);
	 * precvbuf->irp_pending=_FALSE;
	 * precvpriv->rx_pending_cnt --;
	 * _exit_critical(&precvpriv->lock, &irqL);
	 */

	precvpriv->rx_pending_cnt--;

	/*
	 * if(precvpriv->rx_pending_cnt== 0) {
	 * 	RT_TRACE(_module_hci_ops_os_c_,_drv_err_,("usb_read_port_complete: rx_pending_cnt== 0, set allrxreturnevt!\n"));
	 * 	up(&precvpriv->allrxreturnevt);
	 * }
	 */

	if (padapter->bSurpriseRemoved || padapter->bDriverStopped
	|| padapter->bReadPortCancel) {
#ifdef CONFIG_PREALLOC_RECV_SKB
		precvbuf->reuse = _TRUE;
#else
		if (precvbuf->pskb) {
			DBG_8192C("==> free skb(%p)\n", precvbuf->pskb);
			dev_kfree_skb_any(precvbuf->pskb);
		}
#endif
		DBG_8192C("%s() RX Warning! bDriverStopped(%d) OR bSurpriseRemoved(%d) bReadPortCancel(%d)\n",
		__FUNCTION__, padapter->bDriverStopped, padapter->bSurpriseRemoved, padapter->bReadPortCancel);
		goto exit;
	}

	if (purb->status == 0) {
		/* SUCCESS */
		if ((purb->actual_length > MAX_RECVBUF_SZ)
		 || (purb->actual_length < RXDESC_SIZE)) {
			precvbuf->reuse = _TRUE;
			rtw_read_port(padapter, precvpriv->ff_hwaddr, 0, (unsigned char *)precvbuf);
			DBG_8192C("%s()-%d: RX Warning!\n", __FUNCTION__, __LINE__);
		} else {
			rtw_reset_continual_urb_error(adapter_to_dvobj(padapter));

			precvbuf->transfer_len = purb->actual_length;
			skb_put(precvbuf->pskb, purb->actual_length);
			skb_queue_tail(&precvpriv->rx_skb_queue, precvbuf->pskb);

			if (skb_queue_len(&precvpriv->rx_skb_queue) <= 1)
				tasklet_schedule(&precvpriv->recv_tasklet);

			precvbuf->pskb = NULL;
			precvbuf->reuse = _FALSE;
			rtw_read_port(padapter, precvpriv->ff_hwaddr, 0, (unsigned char *)precvbuf);
		}
	} else {
		DBG_8192C("###=> usb_read_port_complete => urb status(%d)\n", purb->status);

		if (rtw_inc_and_chk_continual_urb_error(adapter_to_dvobj(padapter)) == _TRUE) {
			padapter->bSurpriseRemoved = _TRUE;
		}

		switch (purb->status) {
		case -EINVAL:
		case -EPIPE:
		case -ENODEV:
		case -ESHUTDOWN:
			/* padapter->bSurpriseRemoved=_TRUE; */
		case -ENOENT:
			padapter->bDriverStopped = _TRUE;
			break;
		case -EPROTO:
		case -EILSEQ:
		case -ETIME:
		case -ECOMM:
		case -EOVERFLOW:
			precvbuf->reuse = _TRUE;
			rtw_read_port(padapter, precvpriv->ff_hwaddr, 0, (unsigned char *)precvbuf);
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

static uint32_t usb_read_port(struct intf_hdl *pintfhdl, uint32_t addr, uint32_t cnt, uint8_t *rmem)
{
	_irqL irqL;
	int err;
	unsigned int pipe;
	SIZE_PTR tmpaddr = 0;
	SIZE_PTR alignment = 0;
	uint32_t ret = _SUCCESS;
	PURB purb = NULL;
	struct recv_buf	*precvbuf = (struct recv_buf *) rmem;
	struct rtl_priv		*adapter = pintfhdl->padapter;
	struct dvobj_priv	*pdvobj = adapter_to_dvobj(adapter);
	struct recv_priv	*precvpriv = &adapter->recvpriv;
	struct usb_device	*pusbd = pdvobj->pusbdev;

	if (adapter->bDriverStopped || adapter->bSurpriseRemoved
	 || adapter->pwrctrlpriv.pnp_bstop_trx) {
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
		rtl8812au_init_recvbuf(adapter, precvbuf);

		/* re-assign for linux based on skb */
		if ((precvbuf->reuse == _FALSE) || (precvbuf->pskb == NULL)) {
			/* precvbuf->pskb = alloc_skb(MAX_RECVBUF_SZ, GFP_ATOMIC);//don't use this after v2.6.25 */
			precvbuf->pskb = netdev_alloc_skb(adapter->ndev, MAX_RECVBUF_SZ + RECVBUFF_ALIGN_SZ);
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
		 * _enter_critical(&precvpriv->lock, &irqL);
		 * precvpriv->rx_pending_cnt++;
		 * precvbuf->irp_pending = _TRUE;
		 * _exit_critical(&precvpriv->lock, &irqL);
		 */
		precvpriv->rx_pending_cnt++;

		purb = precvbuf->purb;

		/* translate DMA FIFO addr to pipehandle */
		pipe = ffaddr2pipehdl(pdvobj, addr);

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

void rtl8812au_xmit_tasklet(void *priv)
{
	int ret = _FALSE;
	struct rtl_priv *padapter = (struct rtl_priv *) priv;
	struct xmit_priv *pxmitpriv = &padapter->xmitpriv;

	if (check_fwstate(&padapter->mlmepriv, _FW_UNDER_SURVEY) == _TRUE)
		return;

	while (1) {
		if ((padapter->bDriverStopped == _TRUE)
		 || (padapter->bSurpriseRemoved == _TRUE)
		 || (padapter->bWritePortCancel == _TRUE)) {
			DBG_8192C("xmit_tasklet => bDriverStopped or bSurpriseRemoved or bWritePortCancel\n");
			break;
		}

		ret = rtl8812au_xmitframe_complete(padapter, pxmitpriv, NULL);

		if (ret == _FALSE)
			break;

	}

}

/* ULLI USB interface ops */

void rtl8812au_set_intf_ops(struct rtl_io *pops)
{
	memset((uint8_t *)pops, 0, sizeof(struct rtl_io));

	pops->_read8 = &usb_read8;
	pops->_read16 = &usb_read16;
	pops->_read32 = &usb_read32;
	pops->_read_port = &usb_read_port;

	pops->_write8 = &usb_write8;
	pops->_write16 = &usb_write16;
	pops->_write32 = &usb_write32;
	pops->_writeN = &usb_writeN;

	pops->_write_port = &usb_write_port;

	pops->_read_port_cancel = &usb_read_port_cancel;
	pops->_write_port_cancel = &usb_write_port_cancel;
}

void rtl8812au_set_hw_type(struct rtl_priv *padapter)
{
	if (padapter->chip_type == RTL8812) {
		padapter->rtlhal.hw_type = HARDWARE_TYPE_RTL8812AU;
		DBG_871X("CHIP TYPE: RTL8812\n");
	} else if (padapter->chip_type == RTL8821) {
		/* padapter->HardwareType = HARDWARE_TYPE_RTL8811AU; */
		padapter->rtlhal.hw_type = HARDWARE_TYPE_RTL8821U;
		DBG_871X("CHIP TYPE: RTL8811AU or RTL8821U\n");
	}
}



int usbctrl_vendorreq(struct intf_hdl *pintfhdl, uint8_t request, u16 value, u16 index, void *pdata, u16 len, uint8_t requesttype)
{
	struct rtl_priv	*padapter = pintfhdl->padapter;
	struct dvobj_priv  *pdvobjpriv = adapter_to_dvobj(padapter);
	struct usb_device *udev=pdvobjpriv->pusbdev;
	int _unused;

	unsigned int pipe;
	int status = 0;
	u32 tmp_buflen=0;
	uint8_t reqtype;
	uint8_t *pIo_buf;
	int vendorreq_times = 0;

	uint8_t tmp_buf[MAX_USB_IO_CTL_SIZE];

	/* DBG_871X("%s %s:%d\n",__FUNCTION__, current->comm, current->pid); */

	if((padapter->bSurpriseRemoved) ||(padapter->pwrctrlpriv.pnp_bstop_trx)){
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

		status = rtw_usb_control_msg(udev, pipe, request, reqtype, value, index, pIo_buf, len, RTW_USB_CONTROL_MSG_TIMEOUT);

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
					padapter->bSurpriseRemoved = _TRUE;
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
				padapter->bSurpriseRemoved = _TRUE;
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

unsigned int ffaddr2pipehdl(struct dvobj_priv *pdvobj, u32 addr)
{
	unsigned int pipe=0, ep_num=0;
	struct usb_device *pusbd = pdvobj->pusbdev;

	if (addr == RECV_BULK_IN_ADDR) {
		pipe=usb_rcvbulkpipe(pusbd, pdvobj->RtInPipe[0]);

	} else if (addr == RECV_INT_IN_ADDR) {
		pipe=usb_rcvbulkpipe(pusbd, pdvobj->RtInPipe[1]);

	} else if (addr < HW_QUEUE_ENTRY) {
		ep_num = pdvobj->Queue2Pipe[addr];
		pipe = usb_sndbulkpipe(pusbd, ep_num);
	}

	return pipe;
}

struct zero_bulkout_context{
	void *pbuf;
	void *purb;
	void *pirp;
	void *padapter;
};

static void usb_bulkout_zero_complete(struct urb *purb, struct pt_regs *regs)
{
	struct zero_bulkout_context *pcontext = (struct zero_bulkout_context *)purb->context;

	if(pcontext) {
		if(pcontext->pbuf) {
			rtw_mfree(pcontext->pbuf);
		}

		if(pcontext->purb && (pcontext->purb==purb)) {
			usb_free_urb(pcontext->purb);
		}

		rtw_mfree(pcontext);
	}


}

static u32 usb_bulkout_zero(struct intf_hdl *pintfhdl, u32 addr)
{
	int pipe, status, len;
	u32 ret;
	unsigned char *pbuf;
	struct zero_bulkout_context *pcontext;
	PURB	purb = NULL;
	struct rtl_priv *padapter = (struct rtl_priv *)pintfhdl->padapter;
	struct dvobj_priv *pdvobj = adapter_to_dvobj(padapter);
	struct usb_device *pusbd = pdvobj->pusbdev;

	/* DBG_871X("%s\n", __func__); */


	if((padapter->bDriverStopped) || (padapter->bSurpriseRemoved) ||(padapter->pwrctrlpriv.pnp_bstop_trx)) {
		return _FAIL;
	}

	pcontext = (struct zero_bulkout_context *)rtw_zmalloc(sizeof(struct zero_bulkout_context));

	pbuf = (unsigned char *)rtw_zmalloc(sizeof(int));
    	purb = usb_alloc_urb(0, GFP_ATOMIC);

	len = 0;
	pcontext->pbuf = pbuf;
	pcontext->purb = purb;
	pcontext->pirp = NULL;
	pcontext->padapter = padapter;

	/*
	 * translate DMA FIFO addr to pipehandle
	 * pipe = ffaddr2pipehdl(pdvobj, addr);
	 */

	usb_fill_bulk_urb(purb, pusbd, pipe,
       				pbuf,
              			len,
              			usb_bulkout_zero_complete,
              			pcontext);	/* context is pcontext */

	status = usb_submit_urb(purb, GFP_ATOMIC);

	if (!status) {
		ret= _SUCCESS;
	} else {
		ret= _FAIL;
	}


	return _SUCCESS;

}

void usb_read_port_cancel(struct intf_hdl *pintfhdl)
{
	int i;
	struct recv_buf *precvbuf;
	struct rtl_priv	*padapter = pintfhdl->padapter;
	precvbuf = (struct recv_buf *)padapter->recvpriv.precv_buf;

	DBG_871X("%s\n", __func__);

	padapter->bReadPortCancel = _TRUE;

	for (i=0; i < NR_RECVBUFF ; i++) {

		precvbuf->reuse = _TRUE;
		if (precvbuf->purb)	 {
			/* DBG_8192C("usb_read_port_cancel : usb_kill_urb \n"); */
			usb_kill_urb(precvbuf->purb);
		}
		precvbuf++;
	}
}

static void usb_write_port_complete(struct urb *purb, struct pt_regs *regs)
{
	_irqL irqL;
	int i;
	struct xmit_buf *pxmitbuf = (struct xmit_buf *)purb->context;
	/* struct xmit_frame *pxmitframe = (struct xmit_frame *)pxmitbuf->priv_data; */
	/* struct rtl_priv			*padapter = pxmitframe->padapter; */
	struct rtl_priv	*padapter = pxmitbuf->padapter;
	struct xmit_priv	*pxmitpriv = &padapter->xmitpriv;
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
			rtw_chk_hi_queue_cmd(padapter);
#endif
			break;
		default:
			break;
	}


/*
	_enter_critical(&pxmitpriv->lock, &irqL);

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

	_exit_critical(&pxmitpriv->lock, &irqL);


	if(pxmitpriv->txirp_cnt==0)
	{
		RT_TRACE(_module_hci_ops_os_c_,_drv_err_,("usb_write_port_complete: txirp_cnt== 0, set allrxreturnevt!\n"));
		_rtw_up_sema(&(pxmitpriv->tx_retevt));
	}
*/
        /* rtw_free_xmitframe(pxmitpriv, pxmitframe); */

	if (padapter->bSurpriseRemoved || padapter->bDriverStopped ||padapter->bWritePortCancel) {
		DBG_8192C("%s(): TX Warning! bDriverStopped(%d) OR bSurpriseRemoved(%d) bWritePortCancel(%d) pxmitbuf->buf_tag(%x) \n",
		__FUNCTION__,padapter->bDriverStopped, padapter->bSurpriseRemoved,padapter->bReadPortCancel,pxmitbuf->buf_tag);

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
			padapter->bDriverStopped=_TRUE;

			goto check_completion;
		}
		else
		{
			padapter->bSurpriseRemoved=_TRUE;
			DBG_8192C("bSurpriseRemoved=TRUE\n");
			/* rtl8192cu_trigger_gpio_0(padapter); */

			goto check_completion;
		}
	}

check_completion:
	_enter_critical(&pxmitpriv->lock_sctx, &irqL);
	rtw_sctx_done_err(&pxmitbuf->sctx,
		purb->status ? RTW_SCTX_DONE_WRITE_PORT_ERR : RTW_SCTX_DONE_SUCCESS);
	_exit_critical(&pxmitpriv->lock_sctx, &irqL);

	rtw_free_xmitbuf(pxmitpriv, pxmitbuf);

	/* if(rtw_txframes_pending(padapter)) */
	{
		tasklet_hi_schedule(&pxmitpriv->xmit_tasklet);
	}
}

u32 usb_write_port(struct intf_hdl *pintfhdl, u32 addr, u32 cnt, uint8_t *wmem)
{
	_irqL irqL;
	unsigned int pipe;
	int status;
	u32 ret = _FAIL, bwritezero = _FALSE;
	PURB	purb = NULL;
	struct rtl_priv *padapter = (struct rtl_priv *)pintfhdl->padapter;
	struct dvobj_priv	*pdvobj = adapter_to_dvobj(padapter);
	struct xmit_priv	*pxmitpriv = &padapter->xmitpriv;
	struct xmit_buf *pxmitbuf = (struct xmit_buf *)wmem;
	struct xmit_frame *pxmitframe = (struct xmit_frame *)pxmitbuf->priv_data;
	struct usb_device *pusbd = pdvobj->pusbdev;
	struct pkt_attrib *pattrib = &pxmitframe->attrib;

	if ((padapter->bDriverStopped) || (padapter->bSurpriseRemoved) ||(padapter->pwrctrlpriv.pnp_bstop_trx)) {
		#ifdef DBG_TX
		DBG_871X(" DBG_TX %s:%d bDriverStopped%d, bSurpriseRemoved:%d, pnp_bstop_trx:%d\n",__FUNCTION__, __LINE__
			,padapter->bDriverStopped, padapter->bSurpriseRemoved, padapter->pwrctrlpriv.pnp_bstop_trx );
		#endif
		rtw_sctx_done_err(&pxmitbuf->sctx, RTW_SCTX_DONE_TX_DENY);
		goto exit;
	}

	_enter_critical(&pxmitpriv->lock, &irqL);

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

	_exit_critical(&pxmitpriv->lock, &irqL);

	purb	= pxmitbuf->pxmit_urb[0];

	/* translate DMA FIFO addr to pipehandle */
	pipe = ffaddr2pipehdl(pdvobj, addr);

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
			padapter->bDriverStopped=_TRUE;
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

void usb_write_port_cancel(struct intf_hdl *pintfhdl)
{
	int i, j;
	struct rtl_priv	*padapter = pintfhdl->padapter;
	struct xmit_buf *pxmitbuf = (struct xmit_buf *)padapter->xmitpriv.pxmitbuf;

	DBG_871X("%s \n", __func__);

	padapter->bWritePortCancel = _TRUE;

	for (i=0; i<NR_XMITBUFF; i++) {
		for (j=0; j<8; j++) {
			if (pxmitbuf->pxmit_urb[j]) {
				usb_kill_urb(pxmitbuf->pxmit_urb[j]);
			}
		}
		pxmitbuf++;
	}

	pxmitbuf = (struct xmit_buf*)padapter->xmitpriv.pxmit_extbuf;
	for (i = 0; i < NR_XMIT_EXTBUFF; i++) {
		for (j=0; j<8; j++) {
			if(pxmitbuf->pxmit_urb[j]) {
				usb_kill_urb(pxmitbuf->pxmit_urb[j]);
			}
		}
		pxmitbuf++;
	}
}

