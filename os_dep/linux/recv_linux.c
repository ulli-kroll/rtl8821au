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
#define _RECV_OSDEP_C_

#include <drv_types.h>

int rtw_os_alloc_recvframe(_adapter *padapter, union recv_frame *precvframe, uint8_t *pdata, _pkt *pskb)
{
	int res = _SUCCESS;
	uint8_t	shift_sz = 0;
	u32	skb_len, alloc_sz;
	_pkt	 *pkt_copy = NULL;
	struct rx_pkt_attrib *pattrib = &precvframe->u.hdr.attrib;


	if(pdata == NULL)
	{
		precvframe->u.hdr.pkt = NULL;
		res = _FAIL;
		return res;
	}


	//	Modified by Albert 20101213
	//	For 8 bytes IP header alignment.
	shift_sz = pattrib->qos ? 6:0;//	Qos data, wireless lan header length is 26

	skb_len = pattrib->pkt_len;

	// for first fragment packet, driver need allocate 1536+drvinfo_sz+RXDESC_SIZE to defrag packet.
	// modify alloc_sz for recvive crc error packet by thomas 2011-06-02
	if((pattrib->mfrag == 1)&&(pattrib->frag_num == 0))
	{
		//alloc_sz = 1664;	//1664 is 128 alignment.
		alloc_sz = (skb_len <= 1650) ? 1664:(skb_len + 14);
	}
	else
	{
		alloc_sz = skb_len;
		//	6 is for IP header 8 bytes alignment in QoS packet case.
		//	8 is for skb->data 4 bytes alignment.
		alloc_sz += 14;
	}

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,18)) // http://www.mail-archive.com/netdev@vger.kernel.org/msg17214.html
	pkt_copy = dev_alloc_skb(alloc_sz);
#else
	pkt_copy = netdev_alloc_skb(padapter->ndev, alloc_sz);
#endif

	if(pkt_copy)
	{
		pkt_copy->dev = padapter->ndev;
		precvframe->u.hdr.pkt = pkt_copy;
		precvframe->u.hdr.rx_head = pkt_copy->data;
		precvframe->u.hdr.rx_end = pkt_copy->data + alloc_sz;
		skb_reserve(pkt_copy, 8 - ((SIZE_PTR)( pkt_copy->data) & 7 ));//force pkt_copy->data at 8-byte alignment address
		skb_reserve(pkt_copy, shift_sz);//force ip_hdr at 8-byte alignment address according to shift_sz.
		memcpy(pkt_copy->data, pdata, skb_len);
		precvframe->u.hdr.rx_data = precvframe->u.hdr.rx_tail = pkt_copy->data;
	}
	else
	{
		if((pattrib->mfrag == 1)&&(pattrib->frag_num == 0))
		{
			DBG_871X("%s: alloc_skb fail , drop frag frame \n", __FUNCTION__);
			//rtw_free_recvframe(precvframe, pfree_recv_queue);
			res = _FAIL;
			goto exit_rtw_os_recv_resource_alloc;
		}

		if(pskb == NULL)
		{
			res = _FAIL;
			goto exit_rtw_os_recv_resource_alloc;
		}

		precvframe->u.hdr.pkt = skb_clone(pskb, GFP_ATOMIC);
		if(precvframe->u.hdr.pkt)
		{
			precvframe->u.hdr.rx_head = precvframe->u.hdr.rx_data = precvframe->u.hdr.rx_tail = pdata;
			precvframe->u.hdr.rx_end =  pdata + alloc_sz;
		}
		else
		{
			DBG_871X("%s: skb_clone fail\n", __FUNCTION__);
			//rtw_free_recvframe(precvframe, pfree_recv_queue);
			//goto _exit_recvbuf2recvframe;
			res = _FAIL;
		}
	}

exit_rtw_os_recv_resource_alloc:

	return res;

}

void rtw_os_free_recvframe(union recv_frame *precvframe)
{
	if(precvframe->u.hdr.pkt)
	{
		dev_kfree_skb_any(precvframe->u.hdr.pkt);//free skb by driver

		precvframe->u.hdr.pkt = NULL;
	}
}

//init os related resource in struct recv_priv
int rtw_os_recv_resource_init(struct recv_priv *precvpriv, _adapter *padapter)
{
	int	res=_SUCCESS;

	return res;
}

//alloc os related resource in union recv_frame
int rtw_os_recv_resource_alloc(_adapter *padapter, union recv_frame *precvframe)
{
	int	res=_SUCCESS;

	precvframe->u.hdr.pkt_newalloc = precvframe->u.hdr.pkt = NULL;

	return res;
}

//free os related resource in union recv_frame
void rtw_os_recv_resource_free(struct recv_priv *precvpriv)
{

}

//alloc os related resource in struct recv_buf
int rtw_os_recvbuf_resource_alloc(_adapter *padapter, struct recv_buf *precvbuf)
{
	int res=_SUCCESS;

	struct dvobj_priv	*pdvobjpriv = adapter_to_dvobj(padapter);
	struct usb_device	*pusbd = pdvobjpriv->pusbdev;

	precvbuf->irp_pending = _FALSE;
	precvbuf->purb = usb_alloc_urb(0, GFP_KERNEL);
	if(precvbuf->purb == NULL){
		res = _FAIL;
	}

	precvbuf->pskb = NULL;

	precvbuf->reuse = _FALSE;

	precvbuf->pallocated_buf  = precvbuf->pbuf = NULL;

	precvbuf->pdata = precvbuf->phead = precvbuf->ptail = precvbuf->pend = NULL;

	precvbuf->transfer_len = 0;

	precvbuf->len = 0;

	return res;
}

//free os related resource in struct recv_buf
int rtw_os_recvbuf_resource_free(_adapter *padapter, struct recv_buf *precvbuf)
{
	int ret = _SUCCESS;

	if(precvbuf->purb)
	{
		//usb_kill_urb(precvbuf->purb);
		usb_free_urb(precvbuf->purb);
	}



	if(precvbuf->pskb)
		dev_kfree_skb_any(precvbuf->pskb);


	return ret;

}

_pkt *rtw_os_alloc_msdu_pkt(union recv_frame *prframe, uint16_t nSubframe_Length, uint8_t *pdata)
{
	uint16_t	eth_type;
	uint8_t	*data_ptr;
	_pkt *sub_skb;
	struct rx_pkt_attrib *pattrib;

	pattrib = &prframe->u.hdr.attrib;

#ifdef CONFIG_SKB_COPY
	sub_skb = dev_alloc_skb(nSubframe_Length + 12);
	if(sub_skb)
	{
		skb_reserve(sub_skb, 12);
		data_ptr = (uint8_t *)skb_put(sub_skb, nSubframe_Length);
		memcpy(data_ptr, (pdata + ETH_HLEN), nSubframe_Length);
	}
	else
#endif // CONFIG_SKB_COPY
	{
		sub_skb = skb_clone(prframe->u.hdr.pkt, GFP_ATOMIC);
		if(sub_skb)
		{
			sub_skb->data = pdata + ETH_HLEN;
			sub_skb->len = nSubframe_Length;
			skb_set_tail_pointer(sub_skb, nSubframe_Length);
		}
		else
		{
			DBG_871X("%s(): skb_clone() Fail!!!\n",__FUNCTION__);
			return NULL;
		}
	}

	eth_type = RTW_GET_BE16(&sub_skb->data[6]);

	if (sub_skb->len >= 8 &&
		((_rtw_memcmp(sub_skb->data, rtw_rfc1042_header, SNAP_SIZE) &&
		  eth_type != ETH_P_AARP && eth_type != ETH_P_IPX) ||
		 _rtw_memcmp(sub_skb->data, rtw_bridge_tunnel_header, SNAP_SIZE) )) {
		/* remove RFC1042 or Bridge-Tunnel encapsulation and replace EtherType */
		skb_pull(sub_skb, SNAP_SIZE);
		memcpy(skb_push(sub_skb, ETH_ALEN), pattrib->src, ETH_ALEN);
		memcpy(skb_push(sub_skb, ETH_ALEN), pattrib->dst, ETH_ALEN);
	} else {
		uint16_t len;
		/* Leave Ethernet header part of hdr and full payload */
		len = htons(sub_skb->len);
		memcpy(skb_push(sub_skb, 2), &len, 2);
		memcpy(skb_push(sub_skb, ETH_ALEN), pattrib->src, ETH_ALEN);
		memcpy(skb_push(sub_skb, ETH_ALEN), pattrib->dst, ETH_ALEN);
	}

	return sub_skb;
}

void rtw_os_recv_indicate_pkt(_adapter *padapter, _pkt *pkt, struct rx_pkt_attrib *pattrib)
{
	struct mlme_priv*pmlmepriv = &padapter->mlmepriv;

	/* Indicat the packets to upper layer */
	if (pkt) {
		if(check_fwstate(pmlmepriv, WIFI_AP_STATE) == _TRUE)
		{
		 	_pkt *pskb2=NULL;
		 	struct sta_info *psta = NULL;
		 	struct sta_priv *pstapriv = &padapter->stapriv;
			int bmcast = IS_MCAST(pattrib->dst);

			//DBG_871X("bmcast=%d\n", bmcast);

			if(_rtw_memcmp(pattrib->dst, myid(&padapter->eeprompriv), ETH_ALEN)==_FALSE)
			{
				//DBG_871X("not ap psta=%p, addr=%pM\n", psta, pattrib->dst);

				if(bmcast)
				{
					psta = rtw_get_bcmc_stainfo(padapter);
					pskb2 = skb_clone(pkt, GFP_ATOMIC);
				} else {
					psta = rtw_get_stainfo(pstapriv, pattrib->dst);
				}

				if(psta)
				{
					struct net_device *ndev= (struct net_device*)padapter->ndev;

					//DBG_871X("directly forwarding to the rtw_xmit_entry\n");

					//skb->ip_summed = CHECKSUM_NONE;
					pkt->dev = ndev;
#if (LINUX_VERSION_CODE>=KERNEL_VERSION(2,6,35))
					skb_set_queue_mapping(pkt, rtw_recv_select_queue(pkt));
#endif //LINUX_VERSION_CODE>=KERNEL_VERSION(2,6,35)

					rtw_xmit_entry(pkt, ndev);

					if(bmcast && (pskb2 != NULL) ) {
						pkt = pskb2;
					} else {
						return;
					}
				}
			}
			else// to APself
			{
				//DBG_871X("to APSelf\n");
			}
		}

		pkt->protocol = eth_type_trans(pkt, padapter->ndev);
		pkt->dev = padapter->ndev;

#ifdef CONFIG_TCP_CSUM_OFFLOAD_RX
		if ( (pattrib->tcpchk_valid == 1) && (pattrib->tcp_chkrpt == 1) ) {
			pkt->ip_summed = CHECKSUM_UNNECESSARY;
		} else {
			pkt->ip_summed = CHECKSUM_NONE;
		}
#else /* !CONFIG_TCP_CSUM_OFFLOAD_RX */
		pkt->ip_summed = CHECKSUM_NONE;
#endif //CONFIG_TCP_CSUM_OFFLOAD_RX

		netif_rx(pkt);
	}
}

void rtw_handle_tkip_mic_err(_adapter *padapter,uint8_t bgroup)
{
	union iwreq_data wrqu;
	struct iw_michaelmicfailure    ev;
	struct mlme_priv*              pmlmepriv  = &padapter->mlmepriv;
	struct security_priv	*psecuritypriv = &padapter->securitypriv;
	u32 cur_time = 0;

	if( psecuritypriv->last_mic_err_time == 0 )
	{
		psecuritypriv->last_mic_err_time = rtw_get_current_time();
	}
	else
	{
		cur_time = rtw_get_current_time();

		if( cur_time - psecuritypriv->last_mic_err_time < 60*HZ )
		{
			psecuritypriv->btkip_countermeasure = _TRUE;
			psecuritypriv->last_mic_err_time = 0;
			psecuritypriv->btkip_countermeasure_time = cur_time;
		}
		else
		{
			psecuritypriv->last_mic_err_time = rtw_get_current_time();
		}
	}

	memset( &ev, 0x00, sizeof( ev ) );
	if ( bgroup )
	{
	    ev.flags |= IW_MICFAILURE_GROUP;
	}
	else
	{
	    ev.flags |= IW_MICFAILURE_PAIRWISE;
	}

	ev.src_addr.sa_family = ARPHRD_ETHER;
	memcpy( ev.src_addr.sa_data, &pmlmepriv->assoc_bssid[ 0 ], ETH_ALEN );

	memset( &wrqu, 0x00, sizeof( wrqu ) );
	wrqu.data.length = sizeof( ev );

	wireless_send_event( padapter->ndev, IWEVMICHAELMICFAILURE, &wrqu, (char*) &ev );
}

void rtw_hostapd_mlme_rx(_adapter *padapter, union recv_frame *precv_frame)
{
#ifdef CONFIG_HOSTAPD_MLME
	_pkt *skb;
	struct hostapd_priv *phostapdpriv  = padapter->phostapdpriv;
	struct net_device *pmgnt_netdev = phostapdpriv->pmgnt_netdev;

	RT_TRACE(_module_recv_osdep_c_, _drv_info_, ("+rtw_hostapd_mlme_rx\n"));

	skb = precv_frame->u.hdr.pkt;

	if (skb == NULL)
		return;

	skb->data = precv_frame->u.hdr.rx_data;
	skb->tail = precv_frame->u.hdr.rx_tail;
	skb->len = precv_frame->u.hdr.len;

	//pskb_copy = skb_copy(skb, GFP_ATOMIC);
//	if(skb == NULL) goto _exit;

	skb->dev = pmgnt_netdev;
	skb->ip_summed = CHECKSUM_NONE;
	skb->pkt_type = PACKET_OTHERHOST;
	//skb->protocol = __constant_htons(0x0019); /*ETH_P_80211_RAW*/
	skb->protocol = __constant_htons(0x0003); /*ETH_P_80211_RAW*/

	//DBG_871X("(1)data=0x%x, head=0x%x, tail=0x%x, mac_header=0x%x, len=%d\n", skb->data, skb->head, skb->tail, skb->mac_header, skb->len);

	//skb->mac.raw = skb->data;
	skb_reset_mac_header(skb);

       //skb_pull(skb, 24);
       memset(skb->cb, 0, sizeof(skb->cb));

	netif_rx(skb);

	precv_frame->u.hdr.pkt = NULL; // set pointer to NULL before rtw_free_recvframe() if call netif_rx()
#endif
}


int rtw_recv_indicatepkt(_adapter *padapter, union recv_frame *precv_frame)
{
	struct recv_priv *precvpriv;
	_queue	*pfree_recv_queue;
	_pkt *skb;
	struct mlme_priv*pmlmepriv = &padapter->mlmepriv;
	struct rx_pkt_attrib *pattrib = &precv_frame->u.hdr.attrib;

_func_enter_;

	precvpriv = &(padapter->recvpriv);
	pfree_recv_queue = &(precvpriv->free_recv_queue);

#ifdef CONFIG_DRVEXT_MODULE
	if (drvext_rx_handler(padapter, precv_frame->u.hdr.rx_data, precv_frame->u.hdr.len) == _SUCCESS)
	{
		goto _recv_indicatepkt_drop;
	}
#endif

	skb = precv_frame->u.hdr.pkt;
	if(skb == NULL)
	{
		RT_TRACE(_module_recv_osdep_c_,_drv_err_,("rtw_recv_indicatepkt():skb==NULL something wrong!!!!\n"));
		goto _recv_indicatepkt_drop;
	}

	RT_TRACE(_module_recv_osdep_c_,_drv_info_,("rtw_recv_indicatepkt():skb != NULL !!!\n"));
	RT_TRACE(_module_recv_osdep_c_,_drv_info_,("rtw_recv_indicatepkt():precv_frame->u.hdr.rx_head=%p  precv_frame->hdr.rx_data=%p\n", precv_frame->u.hdr.rx_head, precv_frame->u.hdr.rx_data));
	RT_TRACE(_module_recv_osdep_c_,_drv_info_,("precv_frame->hdr.rx_tail=%p precv_frame->u.hdr.rx_end=%p precv_frame->hdr.len=%d \n", precv_frame->u.hdr.rx_tail, precv_frame->u.hdr.rx_end, precv_frame->u.hdr.len));

	skb->data = precv_frame->u.hdr.rx_data;

	skb_set_tail_pointer(skb, precv_frame->u.hdr.len);

	skb->len = precv_frame->u.hdr.len;

	RT_TRACE(_module_recv_osdep_c_,_drv_info_,("\n skb->head=%p skb->data=%p skb->tail=%p skb->end=%p skb->len=%d\n", skb->head, skb->data, skb->tail, skb->end, skb->len));


	rtw_os_recv_indicate_pkt(padapter, skb, pattrib);

_recv_indicatepkt_end:

	precv_frame->u.hdr.pkt = NULL; // pointers to NULL before rtw_free_recvframe()

	rtw_free_recvframe(precv_frame, pfree_recv_queue);

	RT_TRACE(_module_recv_osdep_c_,_drv_info_,("\n rtw_recv_indicatepkt :after netif_rx!!!!\n"));

_func_exit_;

        return _SUCCESS;

_recv_indicatepkt_drop:

	 //enqueue back to free_recv_queue
	 if(precv_frame)
		 rtw_free_recvframe(precv_frame, pfree_recv_queue);

	 return _FAIL;

_func_exit_;

}

void rtw_os_read_port(_adapter *padapter, struct recv_buf *precvbuf)
{
	struct recv_priv *precvpriv = &padapter->recvpriv;

	precvbuf->ref_cnt--;

	//free skb in recv_buf
	dev_kfree_skb_any(precvbuf->pskb);

	precvbuf->pskb = NULL;
	precvbuf->reuse = _FALSE;

	if(precvbuf->irp_pending == _FALSE)
	{
		rtw_read_port(padapter, precvpriv->ff_hwaddr, 0, (unsigned char *)precvbuf);
	}



}
void _rtw_reordering_ctrl_timeout_handler (void *FunctionContext);
void _rtw_reordering_ctrl_timeout_handler (void *FunctionContext)
{
	struct recv_reorder_ctrl *preorder_ctrl = (struct recv_reorder_ctrl *)FunctionContext;
	rtw_reordering_ctrl_timeout_handler(preorder_ctrl);
}

void rtw_init_recv_timer(struct recv_reorder_ctrl *preorder_ctrl)
{
	_adapter *padapter = preorder_ctrl->padapter;

	_init_timer(&(preorder_ctrl->reordering_ctrl_timer), padapter->ndev, _rtw_reordering_ctrl_timeout_handler, preorder_ctrl);

}

