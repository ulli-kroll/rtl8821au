/******************************************************************************
 *
 * Copyright(c) 2007 - 2011 Realtek Corporation. All rights reserved.
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
 *
 ******************************************************************************/
#define _HCI_OPS_OS_C_

#include <rtl8812a_hal.h>

static uint8_t usb_read8(struct intf_hdl *pintfhdl, uint32_t addr)
{
	uint8_t request;
	uint8_t requesttype;
	uint16_t wvalue;
	uint16_t index;
	uint16_t len;
	uint8_t data = 0;

	request = 0x05;
	requesttype = 0x01;	/* read_in */
	index = 0;		/* n/a */

	wvalue = (uint16_t) (addr&0x0000ffff);
	len = 1;

	usbctrl_vendorreq(pintfhdl, request, wvalue, index, &data, len, requesttype);

	return data;

}

static uint16_t usb_read16(struct intf_hdl *pintfhdl, uint32_t addr)
{
	uint8_t request;
	uint8_t requesttype;
	uint16_t wvalue;
	uint16_t index;
	uint16_t len;
	uint16_t data = 0;

	request = 0x05;
	requesttype = 0x01;	/* read_in */
	index = 0;		/* n/a */

	wvalue = (uint16_t)(addr&0x0000ffff);
	len = 2;

	usbctrl_vendorreq(pintfhdl, request, wvalue, index, &data, len, requesttype);

	return data;

}

static uint32_t usb_read32(struct intf_hdl *pintfhdl, uint32_t addr)
{
	uint8_t request;
	uint8_t requesttype;
	uint16_t wvalue;
	uint16_t index;
	uint16_t len;
	uint32_t data = 0;

	request = 0x05;
	requesttype = 0x01;	/* read_in */
	index = 0;		/* n/a */

	wvalue = (uint16_t)(addr&0x0000ffff);
	len = 4;

	usbctrl_vendorreq(pintfhdl, request, wvalue, index, &data, len, requesttype);


	return data;

}

static int usb_write8(struct intf_hdl *pintfhdl, uint32_t addr, uint8_t val)
{
	uint8_t request;
	uint8_t requesttype;
	uint16_t wvalue;
	uint16_t index;
	uint16_t len;
	uint8_t data;
	int ret;

	request = 0x05;
	requesttype = 0x00;		/* write_out */
	index = 0;			/* n/a */

	wvalue = (uint16_t)(addr&0x0000ffff);
	len = 1;

	data = val;

	 ret = usbctrl_vendorreq(pintfhdl, request, wvalue, index, &data, len, requesttype);

	return ret;

}

static int usb_write16(struct intf_hdl *pintfhdl, uint32_t addr, uint16_t val)
{
	uint8_t request;
	uint8_t requesttype;
	uint16_t wvalue;
	uint16_t index;
	uint16_t len;
	uint16_t data;
	int ret;

	request = 0x05;
	requesttype = 0x00;	/* write_out */
	index = 0;		/* n/a */

	wvalue = (uint16_t)(addr&0x0000ffff);
	len = 2;

	data = val;

	ret = usbctrl_vendorreq(pintfhdl, request, wvalue, index, &data, len, requesttype);

	return ret;

}

static int usb_write32(struct intf_hdl *pintfhdl, uint32_t addr, uint32_t val)
{
	uint8_t request;
	uint8_t requesttype;
	uint16_t wvalue;
	uint16_t index;
	uint16_t len;
	uint32_t data;
	int ret;

	request = 0x05;
	requesttype = 0x00;	/* write_out */
	index = 0;		/* n/a */

	wvalue = (uint16_t)(addr&0x0000ffff);
	len = 4;
	data = val;

	ret = usbctrl_vendorreq(pintfhdl, request, wvalue, index, &data, len, requesttype);

	return ret;

}

static int usb_writeN(struct intf_hdl *pintfhdl, uint32_t addr, uint32_t length, uint8_t *pdata)
{
	uint8_t request;
	uint8_t requesttype;
	uint16_t wvalue;
	uint16_t index;
	uint16_t len;
	uint8_t buf[VENDOR_CMD_MAX_DATA_LEN] = {0};
	int ret;

	request = 0x05;
	requesttype = 0x00;	/* write_out */
	index = 0;		/* n/a */

	wvalue = (uint16_t)(addr&0x0000ffff);
	len = length;
	memcpy(buf, pdata, len);

	ret = usbctrl_vendorreq(pintfhdl, request, wvalue, index, buf, len, requesttype);

	_func_exit_;

	return ret;

}

#ifdef CONFIG_SUPPORT_USB_INT
void interrupt_handler_8812au(_adapter *padapter, uint16_t pkt_len, uint8_t *pbuf)
{
	HAL_DATA_TYPE *pHalData = GET_HAL_DATA(padapter);
	struct reportpwrstate_parm pwr_rpt;

	if (pkt_len != INTERRUPT_MSG_FORMAT_LEN) {
		DBG_8192C("%s Invalid interrupt content length (%d)!\n", __FUNCTION__, pkt_len);
		return ;
	}

	/* HISR */
	memcpy(&(pHalData->IntArray[0]), &(pbuf[USB_INTR_CONTENT_HISR_OFFSET]), 4);
	memcpy(&(pHalData->IntArray[1]), &(pbuf[USB_INTR_CONTENT_HISRE_OFFSET]), 4);

#ifdef CONFIG_INTERRUPT_BASED_TXBCN

	{
		struct mlme_priv *pmlmepriv = &padapter->mlmepriv;

		if (check_fwstate(pmlmepriv, WIFI_AP_STATE)) {
			/* send_beacon(padapter); */
			if (pmlmepriv->update_bcn == _TRUE) {
				/* tx_beacon_hdl(padapter, NULL); */
				set_tx_beacon_cmd(padapter);
			}
		}
	}
#endif




#ifdef DBG_CONFIG_ERROR_DETECT_INT
	if (pHalData->IntArray[1]  & IMR_TXERR_88E)
		DBG_871X("===> %s Tx Error Flag Interrupt Status \n", __FUNCTION__);
	if (pHalData->IntArray[1]  & IMR_RXERR_88E)
		DBG_871X("===> %s Rx Error Flag INT Status \n", __FUNCTION__);
	if (pHalData->IntArray[1]  & IMR_TXFOVW_88E)
		DBG_871X("===> %s Transmit FIFO Overflow \n", __FUNCTION__);
	if (pHalData->IntArray[1]  & IMR_RXFOVW_88E)
		DBG_871X("===> %s Receive FIFO Overflow \n", __FUNCTION__);
#endif

	/* C2H Event */
	if (pbuf[0] != 0) {
		memcpy(&(pHalData->C2hArray[0]), &(pbuf[USB_INTR_CONTENT_C2H_OFFSET]), 16);
		/* rtw_c2h_wk_cmd(padapter); to do.. */
	}

}
#endif

static int32_t pre_recv_entry(union recv_frame *precvframe, uint8_t *pphy_status)
{
	int32_t ret = _SUCCESS;

	return ret;
}

static int recvbuf2recvframe(_adapter *padapter, _pkt *pskb)
{
	uint8_t	*pbuf;
	uint8_t	pkt_cnt = 0;
	uint32_t	pkt_offset;
	int32_t	transfer_len;
	uint8_t				*pphy_status = NULL;
	union recv_frame	*precvframe = NULL;
	struct rx_pkt_attrib	*pattrib = NULL;
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(padapter);
	struct recv_priv	*precvpriv = &padapter->recvpriv;
	_queue			*pfree_recv_queue = &precvpriv->free_recv_queue;

	transfer_len = (int32_t)pskb->len;
	pbuf = pskb->data;

#ifdef CONFIG_USB_RX_AGGREGATION
	pkt_cnt = GET_RX_STATUS_DESC_USB_AGG_PKTNUM_8812(pbuf);
#endif

	do {
		precvframe = rtw_alloc_recvframe(pfree_recv_queue);
		if (precvframe == NULL) {
			RT_TRACE(_module_rtl871x_recv_c_, _drv_err_, ("recvbuf2recvframe: precvframe==NULL\n"));
			DBG_8192C("%s()-%d: rtw_alloc_recvframe() failed! RX Drop!\n", __FUNCTION__, __LINE__);
			goto _exit_recvbuf2recvframe;
		}

		INIT_LIST_HEAD(&precvframe->u.hdr.list);
		precvframe->u.hdr.precvbuf = NULL;	/* can't access the precvbuf for new arch. */
		precvframe->u.hdr.len = 0;

		rtl8812_query_rx_desc_status(precvframe, pbuf);

		pattrib = &precvframe->u.hdr.attrib;

		if ((pattrib->crc_err) || (pattrib->icv_err)) {
			DBG_8192C("%s: RX Warning! crc_err=%d icv_err=%d, skip!\n", __FUNCTION__, pattrib->crc_err, pattrib->icv_err);

			rtw_free_recvframe(precvframe, pfree_recv_queue);
			goto _exit_recvbuf2recvframe;
		}

		pkt_offset = RXDESC_SIZE + pattrib->drvinfo_sz + pattrib->shift_sz + pattrib->pkt_len;

		if ((pattrib->pkt_len <= 0) || (pkt_offset > transfer_len)) {
			RT_TRACE(_module_rtl871x_recv_c_, _drv_info_, ("recvbuf2recvframe: pkt_len<=0\n"));
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
				RT_TRACE(_module_rtl871x_recv_c_, _drv_err_,
					("recvbuf2recvframe: rtw_recv_entry(precvframe) != _SUCCESS\n"));
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
				#ifdef CONFIG_SUPPORT_USB_INT
				interrupt_handler_8812au(padapter,pattrib->pkt_len,precvframe->u.hdr.rx_data);
				#endif
			}*/
			rtw_free_recvframe(precvframe, pfree_recv_queue);

		}

#ifdef CONFIG_USB_RX_AGGREGATION
		/*  jaguar 8-byte alignment */
		pkt_offset = (uint16_t)_RND8(pkt_offset);
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
	_pkt			*pskb;
	_adapter		*padapter = (_adapter *)priv;
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
	_adapter 		*padapter = (_adapter *) precvbuf->adapter;
	struct recv_priv	*precvpriv = &padapter->recvpriv;

	RT_TRACE(_module_hci_ops_os_c_, _drv_err_, ("usb_read_port_complete!!!\n"));

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
		RT_TRACE(_module_hci_ops_os_c_, _drv_err_, ("usb_read_port_complete:bDriverStopped(%d) OR bSurpriseRemoved(%d)\n", padapter->bDriverStopped, padapter->bSurpriseRemoved));

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
			RT_TRACE(_module_hci_ops_os_c_, _drv_err_, ("usb_read_port_complete: (purb->actual_length > MAX_RECVBUF_SZ) || (purb->actual_length < RXDESC_SIZE)\n"));
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
		RT_TRACE(_module_hci_ops_os_c_, _drv_err_, ("usb_read_port_complete : purb->status(%d) != 0 \n", purb->status));

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
			RT_TRACE(_module_hci_ops_os_c_, _drv_err_, ("usb_read_port_complete:bSurpriseRemoved=TRUE\n"));
		case -ENOENT:
			padapter->bDriverStopped = _TRUE;
			RT_TRACE(_module_hci_ops_os_c_, _drv_err_, ("usb_read_port_complete:bDriverStopped=TRUE\n"));
			break;
		case -EPROTO:
		case -EILSEQ:
		case -ETIME:
		case -ECOMM:
		case -EOVERFLOW:
#ifdef DBG_CONFIG_ERROR_DETECT
			{
				HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(padapter);
				pHalData->srestpriv.Wifi_Error_Status = USB_READ_PORT_FAIL;
			}
#endif
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
	_adapter		*adapter = pintfhdl->padapter;
	struct dvobj_priv	*pdvobj = adapter_to_dvobj(adapter);
	struct recv_priv	*precvpriv = &adapter->recvpriv;
	struct usb_device	*pusbd = pdvobj->pusbdev;

	if (adapter->bDriverStopped || adapter->bSurpriseRemoved
	 || adapter->pwrctrlpriv.pnp_bstop_trx) {
		RT_TRACE(_module_hci_ops_os_c_, _drv_err_, ("usb_read_port:( padapter->bDriverStopped ||padapter->bSurpriseRemoved ||adapter->pwrctrlpriv.pnp_bstop_trx)!!!\n"));
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
				RT_TRACE(_module_hci_ops_os_c_, _drv_err_, ("init_recvbuf(): alloc_skb fail!\n"));
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
			RT_TRACE(_module_hci_ops_os_c_, _drv_err_, ("cannot submit rx in-token(err=0x%.8x), URB_STATUS =0x%.8x", err, purb->status));
			DBG_8192C("cannot submit rx in-token(err = 0x%08x),urb_status = %d\n", err, purb->status);
			ret = _FAIL;
		}
	} else {
		RT_TRACE(_module_hci_ops_os_c_, _drv_err_, ("usb_read_port:precvbuf ==NULL\n"));
		ret = _FAIL;
	}

	return ret;
}

void rtl8812au_xmit_tasklet(void *priv)
{
	int ret = _FALSE;
	_adapter *padapter = (_adapter *) priv;
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

void rtl8812au_set_intf_ops(struct _io_ops	*pops)
{
	memset((uint8_t *)pops, 0, sizeof(struct _io_ops));

	pops->_read8 = &usb_read8;
	pops->_read16 = &usb_read16;
	pops->_read32 = &usb_read32;
	pops->_read_mem = &usb_read_mem;
	pops->_read_port = &usb_read_port;

	pops->_write8 = &usb_write8;
	pops->_write16 = &usb_write16;
	pops->_write32 = &usb_write32;
	pops->_writeN = &usb_writeN;

	pops->_write_mem = &usb_write_mem;
	pops->_write_port = &usb_write_port;

	pops->_read_port_cancel = &usb_read_port_cancel;
	pops->_write_port_cancel = &usb_write_port_cancel;
}

void rtl8812au_set_hw_type(_adapter *padapter)
{
	if (padapter->chip_type == RTL8812) {
		padapter->HardwareType = HARDWARE_TYPE_RTL8812AU;
		DBG_871X("CHIP TYPE: RTL8812\n");
	} else if (padapter->chip_type == RTL8821) {
		/* padapter->HardwareType = HARDWARE_TYPE_RTL8811AU; */
		padapter->HardwareType = HARDWARE_TYPE_RTL8821U;
		DBG_871X("CHIP TYPE: RTL8811AU or RTL8821U\n");
	}
}

