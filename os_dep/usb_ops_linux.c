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
#include <rtw_debug.h>
#include <../rtl8821au/trx.h>


static int32_t pre_recv_entry(struct recv_frame *precvframe, uint8_t *pphy_status)
{
	int32_t ret = _SUCCESS;

	return ret;
}

static int recvbuf2recvframe(struct rtl_priv *rtlpriv, struct sk_buff *pskb)
{
	uint8_t	*pbuf;
	uint8_t	pkt_cnt = 0;
	uint32_t	pkt_offset;
	int32_t	transfer_len;
	uint8_t				*pphy_status = NULL;
	struct recv_frame	*precvframe = NULL;
	struct rx_pkt_attrib	*pattrib = NULL;
	 struct _rtw_hal	*pHalData = GET_HAL_DATA(rtlpriv);
	struct recv_priv	*precvpriv = &rtlpriv->recvpriv;
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

		rtl8812_query_rx_desc_status(rtlpriv, &precvframe->attrib, precvframe, pbuf);

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

		if (rtw_os_alloc_recvframe(rtlpriv, precvframe,
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
				/* C2HPacketHandler_8812A(rtlpriv,precvframe->u.hdr.rx_data,pattrib->pkt_len); */
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
	struct rtl_priv		*rtlpriv = (struct rtl_priv *)priv;
	struct recv_priv	*precvpriv = &rtlpriv->recvpriv;

	while (NULL != (pskb = skb_dequeue(&precvpriv->rx_skb_queue))) {
		if ((rtlpriv->bDriverStopped == _TRUE)
		|| (rtlpriv->bSurpriseRemoved == _TRUE)) {
			DBG_8192C("recv_tasklet => bDriverStopped or bSurpriseRemoved \n");
			dev_kfree_skb_any(pskb);
			break;
		}

		recvbuf2recvframe(rtlpriv, pskb);

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
	struct rtl_priv *rtlpriv = (struct rtl_priv *) priv;
	struct xmit_priv *pxmitpriv = &rtlpriv->xmitpriv;

	if (check_fwstate(&rtlpriv->mlmepriv, _FW_UNDER_SURVEY) == _TRUE)
		return;

	while (1) {
		if ((rtlpriv->bDriverStopped == _TRUE)
		 || (rtlpriv->bSurpriseRemoved == _TRUE)
		 || (rtlpriv->bWritePortCancel == _TRUE)) {
			DBG_8192C("xmit_tasklet => bDriverStopped or bSurpriseRemoved or bWritePortCancel\n");
			break;
		}

		ret = rtl8812au_xmitframe_complete(rtlpriv, pxmitpriv, NULL);

		if (ret == _FALSE)
			break;

	}

}

struct zero_bulkout_context{
	void *pbuf;
	void *purb;
	void *pirp;
	void *rtlpriv;
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

static u32 usb_bulkout_zero(struct rtl_priv *rtlpriv, u32 addr)
{
	int pipe, status, len;
	u32 ret;
	unsigned char *pbuf;
	struct zero_bulkout_context *pcontext;
	PURB	purb = NULL;
	struct rtl_usb *pdvobj = rtl_usbdev(rtlpriv);
	struct usb_device *pusbd = pdvobj->udev;

	/* DBG_871X("%s\n", __func__); */


	if((rtlpriv->bDriverStopped) || (rtlpriv->bSurpriseRemoved) ||(rtlpriv->pwrctrlpriv.pnp_bstop_trx)) {
		return _FAIL;
	}

	pcontext = (struct zero_bulkout_context *)rtw_zmalloc(sizeof(struct zero_bulkout_context));

	pbuf = (unsigned char *)rtw_zmalloc(sizeof(int));
    	purb = usb_alloc_urb(0, GFP_ATOMIC);

	len = 0;
	pcontext->pbuf = pbuf;
	pcontext->purb = purb;
	pcontext->pirp = NULL;
	pcontext->rtlpriv = rtlpriv;

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

void usb_read_port_cancel(struct rtl_priv *rtlpriv)
{
	int i;
	struct recv_buf *precvbuf;
	precvbuf = (struct recv_buf *)rtlpriv->recvpriv.precv_buf;

	DBG_871X("%s\n", __func__);

	rtlpriv->bReadPortCancel = _TRUE;

	for (i=0; i < NR_RECVBUFF ; i++) {

		precvbuf->reuse = _TRUE;
		if (precvbuf->purb)	 {
			/* DBG_8192C("usb_read_port_cancel : usb_kill_urb \n"); */
			usb_kill_urb(precvbuf->purb);
		}
		precvbuf++;
	}
}
