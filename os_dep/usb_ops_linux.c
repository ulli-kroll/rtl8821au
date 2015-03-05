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
	struct rtl_usb  *pdvobjpriv = rtl_usbdev(padapter);
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
	struct rtl_usb *pdvobj = rtl_usbdev(padapter);
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
