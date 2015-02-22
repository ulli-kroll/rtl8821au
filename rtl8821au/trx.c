#include <rtl8812a_hal.h>
#include "trx.h"
#include "def.h"

static void rtl8812a_cal_txdesc_chksum(uint8_t *ptxdesc)
{
	u16 *usPtr;
	uint32_t count;
	uint32_t index;
	u16 checksum = 0;

	usPtr = (u16 *)ptxdesc;
	/*
	 * checksume is always calculated by first 32 bytes,
	 * and it doesn't depend on TX DESC length.
	 * Thomas,Lucas@SD4,20130515
	 */
	count = 16;

	/* Clear first */
	SET_TX_DESC_TX_DESC_CHECKSUM(ptxdesc, 0);

	for (index = 0; index < count; index++) {
		checksum = checksum ^ le16_to_cpu(*(usPtr + index));
	}

	SET_TX_DESC_TX_DESC_CHECKSUM(ptxdesc, checksum);
}

void rtl8821au_fill_fake_txdesc(struct rtl_priv *padapter, uint8_t *pDesc,
	uint32_t BufferLen, uint8_t IsPsPoll, uint8_t IsBTQosNull)
{
	struct mlme_ext_priv	*pmlmeext = &padapter->mlmeextpriv;

	/* Clear all status */
	memset(pDesc, 0, TXDESC_SIZE);

	SET_TX_DESC_FIRST_SEG(pDesc, 1);
	SET_TX_DESC_LAST_SEG(pDesc, 1);

	SET_TX_DESC_OFFSET(pDesc, TXDESC_SIZE);

	SET_TX_DESC_PKT_SIZE(pDesc, BufferLen);

	SET_TX_DESC_QUEUE_SEL(pDesc,  QSLT_MGNT);

	if (pmlmeext->cur_wireless_mode & WIRELESS_11B) {
		SET_TX_DESC_RATE_ID(pDesc, RATEID_IDX_B);
	} else {
		SET_TX_DESC_RATE_ID(pDesc, RATEID_IDX_G);
	}

	/*
	 * Set NAVUSEHDR to prevent Ps-poll AId filed to be changed to error vlaue by Hw.
	 */
	if (IsPsPoll) {
		SET_TX_DESC_NAV_USE_HDR(pDesc, 1);
	} else {
		SET_TX_DESC_HWSEQ_EN(pDesc, 1); // Hw set sequence number
	}

	if (IsBTQosNull) {
		SET_TX_DESC_BT_INT(pDesc, 1);
	}

	SET_TX_DESC_USE_RATE(pDesc, 1);
	SET_TX_DESC_OWN(pDesc, 1);

	SET_TX_DESC_TX_RATE(pDesc, MRateToHwRate(pmlmeext->tx_rate));

	// USB interface drop packet if the checksum of descriptor isn't correct.
	// Using this checksum can let hardware recovery from packet bulk out error (e.g. Cancel URC, Bulk out error.).
	rtl8812a_cal_txdesc_chksum(pDesc);
}

/* ULLI TX DESC */
/*
 * Description: In normal chip, we should send some packet to Hw which will be used by Fw
 * in FW LPS mode. The function is to fill the Tx descriptor of this packets, then
 * Fw can tell Hw to send these packet derectly.
 *
*/
static void rtl8812a_fill_txdesc_sectype(struct pkt_attrib *pattrib, uint8_t *ptxdesc)
{
	if ((pattrib->encrypt > 0) && !pattrib->bswenc) {
		switch (pattrib->encrypt) {
		/* SEC_TYPE : 0:NO_ENC,1:WEP40/TKIP,2:WAPI,3:AES */
		case _WEP40_:
		case _WEP104_:
		case _TKIP_:
		case _TKIP_WTMIC_:
			SET_TX_DESC_SEC_TYPE(ptxdesc, 0x1);
			break;
		case _AES_:
			SET_TX_DESC_SEC_TYPE(ptxdesc, 0x3);
			break;
		case _NO_PRIVACY_:
		default:
			SET_TX_DESC_SEC_TYPE(ptxdesc, 0x0);
			break;

		}

	}

}


static void rtl8812a_fill_txdesc_phy(struct rtl_priv *padapter, struct pkt_attrib *pattrib, uint8_t *ptxdesc)
{
	/* DBG_8192C("bwmode=%d, ch_off=%d\n", pattrib->bwmode, pattrib->ch_offset); */

	if (pattrib->ht_en) {
		/*  Set Bandwidth and sub-channel settings. */
		SET_TX_DESC_DATA_BW(ptxdesc, BWMapping_8812(padapter,pattrib));
		/* SET_TX_DESC_DATA_SC(ptxdesc, SCMapping_8812(padapter,pattrib)); */
	}
}

static void rtl8812a_fill_txdesc_vcs(struct rtl_priv *padapter, struct pkt_attrib *pattrib, uint8_t *ptxdesc)
{
	struct mlme_ext_priv	*pmlmeext = &padapter->mlmeextpriv;
	struct mlme_ext_info	*pmlmeinfo = &(pmlmeext->mlmext_info);

	//DBG_8192C("vcs_mode=%d\n", pattrib->vcs_mode);

	if (pattrib->vcs_mode) {
		switch(pattrib->vcs_mode) {
		case RTS_CTS:
			SET_TX_DESC_RTS_ENABLE(ptxdesc, 1);
			break;
		case CTS_TO_SELF:
			SET_TX_DESC_CTS2SELF(ptxdesc, 1);
			break;
		case NONE_VCS:
		default:
			break;
		}
		if (pmlmeinfo->preamble_mode == PREAMBLE_SHORT)
			SET_TX_DESC_RTS_SHORT(ptxdesc, 1);

		SET_TX_DESC_RTS_RATE(ptxdesc, 0x8);	/*RTS Rate=24M */

		SET_TX_DESC_RTS_RATE_FB_LIMIT(ptxdesc, 0xf);

		/*
		 * Enable HW RTS
		 * SET_TX_DESC_HW_RTS_ENABLE(ptxdesc, 1);
		 */
	}
}

static int32_t update_txdesc(struct xmit_frame *pxmitframe, uint8_t *pmem, int32_t sz , uint8_t bagg_pkt)
{
	int	pull = 0;
	uint	qsel;
	uint8_t data_rate, pwr_status, offset;
	struct rtl_priv *padapter = pxmitframe->padapter;
	struct rtl_hal *rtlhal = rtl_hal(padapter);
	struct mlme_priv	*pmlmepriv = &padapter->mlmepriv;
	struct pkt_attrib	*pattrib = &pxmitframe->attrib;
	 struct _rtw_hal	*pHalData = GET_HAL_DATA(padapter);
	struct dm_priv	*pdmpriv = &pHalData->dmpriv;
	uint8_t	*ptxdesc =  pmem;
	struct mlme_ext_priv	*pmlmeext = &padapter->mlmeextpriv;
	struct mlme_ext_info	*pmlmeinfo = &(pmlmeext->mlmext_info);
	sint	bmcst = IS_MCAST(pattrib->ra);

	if (padapter->registrypriv.mp_mode == 0) {
		if ((!bagg_pkt) && (rtw_usb_bulk_size_boundary(padapter, TXDESC_SIZE+sz) == _FALSE)) {
			ptxdesc = (pmem+PACKET_OFFSET_SZ);
			/* DBG_8192C("==> non-agg-pkt,shift pointer...\n"); */
			pull = 1;
		}
	}

	memset(ptxdesc, 0, TXDESC_SIZE);

	/* 4 offset 0 */
	SET_TX_DESC_FIRST_SEG(ptxdesc, 1);
	SET_TX_DESC_LAST_SEG(ptxdesc, 1);
	SET_TX_DESC_OWN(ptxdesc, 1);

	/* DBG_8192C("%s==> pkt_len=%d,bagg_pkt=%02x\n",__FUNCTION__,sz,bagg_pkt); */
	SET_TX_DESC_PKT_SIZE(ptxdesc, sz);

	offset = TXDESC_SIZE + OFFSET_SZ;

	/* DBG_8192C("%s==>offset(0x%02x)  \n",__FUNCTION__,offset); */
	SET_TX_DESC_OFFSET(ptxdesc, offset);

	if (bmcst) {
		SET_TX_DESC_BMC(ptxdesc, 1);
	}

	if (padapter->registrypriv.mp_mode == 0) {
		if (!bagg_pkt) {
			if ((pull) && (pxmitframe->pkt_offset > 0)) {
				pxmitframe->pkt_offset = pxmitframe->pkt_offset - 1;
			}
		}
	}

	/*
	 * DBG_8192C("%s, pkt_offset=0x%02x\n",__FUNCTION__,pxmitframe->pkt_offset);
	 * pkt_offset, unit:8 bytes padding
	 */
	if (pxmitframe->pkt_offset > 0) {
		SET_TX_DESC_PKT_OFFSET(ptxdesc, pxmitframe->pkt_offset);
	}

	SET_TX_DESC_MACID(ptxdesc, pattrib->mac_id);
	SET_TX_DESC_RATE_ID(ptxdesc, pattrib->raid);

	SET_TX_DESC_QUEUE_SEL(ptxdesc,  pattrib->qsel);

	/* offset 12 */

	if (!pattrib->qos_en) {
		SET_TX_DESC_HWSEQ_EN(ptxdesc, 1); /* Hw set sequence number */
	} else {
		SET_TX_DESC_SEQ(ptxdesc, pattrib->seqnum);
	}

	if ((pxmitframe->frame_tag&0x0f) == DATA_FRAMETAG) {
		/* DBG_8192C("pxmitframe->frame_tag == DATA_FRAMETAG\n"); */

		rtl8812a_fill_txdesc_sectype(pattrib, ptxdesc);

		/* offset 20 */
#ifdef CONFIG_USB_TX_AGGREGATION
		if (pxmitframe->agg_num > 1) {
			/* DBG_8192C("%s agg_num:%d\n",__FUNCTION__,pxmitframe->agg_num ); */
			SET_TX_DESC_USB_TXAGG_NUM(ptxdesc, pxmitframe->agg_num);
		}
#endif

		rtl8812a_fill_txdesc_vcs(padapter, pattrib, ptxdesc);

		if ((pattrib->ether_type != 0x888e) &&
		    (pattrib->ether_type != 0x0806) &&
		    (pattrib->ether_type != 0x88b4) &&
		    (pattrib->dhcp_pkt != 1)) {
			/* Non EAP & ARP & DHCP type data packet */

			if (pattrib->ampdu_en == _TRUE) {
				SET_TX_DESC_AGG_ENABLE(ptxdesc, 1);
				SET_TX_DESC_MAX_AGG_NUM(ptxdesc, 0x1f);
				/* Set A-MPDU aggregation. */
				SET_TX_DESC_AMPDU_DENSITY(ptxdesc, pHalData->AMPDUDensity);
			} else {
				SET_TX_DESC_AGG_BREAK(ptxdesc, 1);
			}

			rtl8812a_fill_txdesc_phy(padapter, pattrib, ptxdesc);

			/* DATA  Rate FB LMT */
			SET_TX_DESC_DATA_RATE_FB_LIMIT(ptxdesc, 0x1f);

			if (pHalData->fw_ractrl == _FALSE) {
				SET_TX_DESC_USE_RATE(ptxdesc, 1);

				if (pdmpriv->INIDATA_RATE[pattrib->mac_id] & BIT(7))
					SET_TX_DESC_DATA_SHORT(ptxdesc, 	1);

				SET_TX_DESC_TX_RATE(ptxdesc, (pdmpriv->INIDATA_RATE[pattrib->mac_id] & 0x7F));
			}

			if (padapter->fix_rate != 0xFF) { 	/* modify data rate by iwpriv */
				SET_TX_DESC_USE_RATE(ptxdesc, 1);
				if (padapter->fix_rate & BIT(7))
					SET_TX_DESC_DATA_SHORT(ptxdesc, 	1);

				SET_TX_DESC_TX_RATE(ptxdesc, (padapter->fix_rate & 0x7F));
			}

			if (pattrib->ldpc)
				SET_TX_DESC_DATA_LDPC(ptxdesc, 1);
			if (pattrib->stbc)
				SET_TX_DESC_DATA_STBC(ptxdesc, 1);
		} else {
			/*
			 *  EAP data packet and ARP packet and DHCP.
			 *  Use the 1M data rate to send the EAP/ARP packet.
			 *  This will maybe make the handshake smooth.
			 */

			SET_TX_DESC_USE_RATE(ptxdesc, 1);
			SET_TX_DESC_AGG_BREAK(ptxdesc, 1);

			/* HW will ignore this setting if the transmission rate is legacy OFDM. */
			if (pmlmeinfo->preamble_mode == PREAMBLE_SHORT) {
				SET_TX_DESC_DATA_SHORT(ptxdesc, 1);
			}

			SET_TX_DESC_TX_RATE(ptxdesc, MRateToHwRate(pmlmeext->tx_rate));
		}
	} else if ((pxmitframe->frame_tag&0x0f) == MGNT_FRAMETAG) {
		/* DBG_8192C("pxmitframe->frame_tag == MGNT_FRAMETAG\n"); */

		if (IS_HARDWARE_TYPE_8821(rtlhal))
			SET_TX_DESC_MBSSID_8821(ptxdesc, pattrib->mbssid);

		/* offset 20 */
		SET_TX_DESC_RETRY_LIMIT_ENABLE(ptxdesc, 1);

		if (pattrib->retry_ctrl == _TRUE) {
			SET_TX_DESC_DATA_RETRY_LIMIT(ptxdesc, 6);
		} else {
			SET_TX_DESC_DATA_RETRY_LIMIT(ptxdesc, 12);
		}

		SET_TX_DESC_USE_RATE(ptxdesc, 1);
		{
			SET_TX_DESC_TX_RATE(ptxdesc, MRateToHwRate(pmlmeext->tx_rate));
		}
	} else if ((pxmitframe->frame_tag&0x0f) == TXAGG_FRAMETAG) {
		DBG_8192C("pxmitframe->frame_tag == TXAGG_FRAMETAG\n");
	} else {
		DBG_8192C("pxmitframe->frame_tag = %d\n", pxmitframe->frame_tag);

		SET_TX_DESC_USE_RATE(ptxdesc, 1);
		SET_TX_DESC_TX_RATE(ptxdesc, MRateToHwRate(pmlmeext->tx_rate));
	}

	rtl8812a_cal_txdesc_chksum(ptxdesc);
	_dbg_dump_tx_info(padapter, pxmitframe->frame_tag, ptxdesc);
	return pull;
}




/* for non-agg data frame or  management frame */
static int32_t rtw_dump_xframe(struct rtl_priv *padapter, struct xmit_frame *pxmitframe)
{
	int32_t ret = _SUCCESS;
	int32_t inner_ret = _SUCCESS;
	int t, sz, w_sz, pull = 0;
	uint8_t *mem_addr;
	uint32_t ff_hwaddr;
	struct xmit_buf *pxmitbuf = pxmitframe->pxmitbuf;
	struct pkt_attrib *pattrib = &pxmitframe->attrib;
	struct xmit_priv *pxmitpriv = &padapter->xmitpriv;
	struct security_priv *psecuritypriv = &padapter->securitypriv;
#ifdef CONFIG_80211N_HT
	if ((pxmitframe->frame_tag == DATA_FRAMETAG) &&
	    (pxmitframe->attrib.ether_type != 0x0806) &&
	    (pxmitframe->attrib.ether_type != 0x888e) &&
	    (pxmitframe->attrib.ether_type != 0x88b4) &&
	    (pxmitframe->attrib.dhcp_pkt != 1)) {
		rtw_issue_addbareq_cmd(padapter, pxmitframe);
	}
#endif
	mem_addr = pxmitframe->buf_addr;

	for (t = 0; t < pattrib->nr_frags; t++) {
		if (inner_ret != _SUCCESS && ret == _SUCCESS)
			ret = _FAIL;

		if (t != (pattrib->nr_frags - 1)) {
			sz = pxmitpriv->frag_len;
			sz = sz - 4 - (psecuritypriv->sw_encrypt ? 0 : pattrib->icv_len);
		} else {
			/* no frag */
			sz = pattrib->last_txcmdsz;
		}

		pull = update_txdesc(pxmitframe, mem_addr, sz, _FALSE);

		if (pull) {
			mem_addr += PACKET_OFFSET_SZ; /* pull txdesc head */

			/* pxmitbuf ->pbuf = mem_addr; */
			pxmitframe->buf_addr = mem_addr;

			w_sz = sz + TXDESC_SIZE;
		} else 	{
			w_sz = sz + TXDESC_SIZE + PACKET_OFFSET_SZ;
		}

		ff_hwaddr = rtw_get_ff_hwaddr(pxmitframe);

		inner_ret = rtw_write_port(padapter, ff_hwaddr, w_sz, (unsigned char *)pxmitbuf);

		rtw_count_tx_stats(padapter, pxmitframe, sz);

		/* DBG_8192C("rtw_write_port, w_sz=%d, sz=%d, txdesc_sz=%d, tid=%d\n", w_sz, sz, w_sz-sz, pattrib->priority); */

		mem_addr += w_sz;

		mem_addr = (uint8_t *)RND4(((SIZE_PTR)(mem_addr)));

	}

	rtw_free_xmitframe(pxmitpriv, pxmitframe);

	if  (ret != _SUCCESS)
		rtw_sctx_done_err(&pxmitbuf->sctx, RTW_SCTX_DONE_UNKNOWN);

	return ret;
}

#ifdef CONFIG_USB_TX_AGGREGATION
static uint32_t xmitframe_need_length(struct xmit_frame *pxmitframe)
{
	struct pkt_attrib *pattrib = &pxmitframe->attrib;

	uint32_t	len = 0;

	/* no consider fragement */
	len = pattrib->hdrlen + pattrib->iv_len +
		SNAP_SIZE + sizeof(u16) +
		pattrib->pktlen +
		((pattrib->bswenc) ? pattrib->icv_len : 0);

	if (pattrib->encrypt == _TKIP_)
		len += 8;

	return len;
}

#define IDEA_CONDITION 1	/* check all packets before enqueue */
int32_t rtl8812au_xmitframe_complete(struct rtl_priv *padapter, struct xmit_priv *pxmitpriv, struct xmit_buf *pxmitbuf)
{
	 struct _rtw_hal	*pHalData = GET_HAL_DATA(padapter);
	struct xmit_frame *pxmitframe = NULL;
	struct xmit_frame *pfirstframe = NULL;

	/* aggregate variable */
	struct hw_xmit *phwxmit;
	struct sta_info *psta = NULL;
	struct tx_servq *ptxservq = NULL;

	_irqL irqL;
	struct list_head *xmitframe_plist = NULL, *xmitframe_phead = NULL;

	uint32_t	pbuf;		/* next pkt address */
	uint32_t	pbuf_tail;	/* last pkt tail */
	uint32_t	len;		/* packet length, except TXDESC_SIZE and PKT_OFFSET */

	uint32_t	bulkSize = pHalData->UsbBulkOutSize;
	uint8_t	descCount;
	uint32_t	bulkPtr;

	/* dump frame variable */
	uint32_t ff_hwaddr;

#ifndef IDEA_CONDITION
	int res = _SUCCESS;
#endif

	/* check xmitbuffer is ok */
	if (pxmitbuf == NULL) {
		pxmitbuf = rtw_alloc_xmitbuf(pxmitpriv);
		if (pxmitbuf == NULL) {
			/* DBG_871X("%s #1, connot alloc xmitbuf!!!! \n",__FUNCTION__); */
			return _FALSE;
		}
	}

/* DBG_8192C("%s ===================================== \n",__FUNCTION__); */
	/* 3 1. pick up first frame */
	do {
		rtw_free_xmitframe(pxmitpriv, pxmitframe);

		pxmitframe = rtw_dequeue_xframe(pxmitpriv, pxmitpriv->hwxmits, pxmitpriv->hwxmit_entry);
		if (pxmitframe == NULL) {
			/*
			 * no more xmit frame, release xmit buffer
			 * DBG_8192C("no more xmit frame ,return\n");
			 */
			rtw_free_xmitbuf(pxmitpriv, pxmitbuf);
			return _FALSE;
		}

#ifndef IDEA_CONDITION
		if (pxmitframe->frame_tag != DATA_FRAMETAG) {
			/* rtw_free_xmitframe(pxmitpriv, pxmitframe); */
			continue;
		}

		/* TID 0~15 */
		if ((pxmitframe->attrib.priority < 0) ||
		    (pxmitframe->attrib.priority > 15)) {
			/* rtw_free_xmitframe(pxmitpriv, pxmitframe); */
			continue;
		}
#endif
		/* DBG_8192C("==> pxmitframe->attrib.priority:%d\n",pxmitframe->attrib.priority); */
		pxmitframe->pxmitbuf = pxmitbuf;
		pxmitframe->buf_addr = pxmitbuf->pbuf;
		pxmitbuf->priv_data = pxmitframe;

		pxmitframe->agg_num = 1; 	/* alloc xmitframe should assign to 1. */
		pxmitframe->pkt_offset = 1; 	/* first frame of aggregation, reserve offset */

		if (rtw_xmitframe_coalesce(padapter, pxmitframe->pkt, pxmitframe) == _FALSE) {
			DBG_871X("%s coalesce 1st xmitframe failed \n", __FUNCTION__);
			continue;
		}


		/* always return ndis_packet after rtw_xmitframe_coalesce */
		rtw_os_xmit_complete(padapter, pxmitframe);

		break;
	} while (1);

	/* 3 2. aggregate same priority and same DA(AP or STA) frames */
	pfirstframe = pxmitframe;
	len = xmitframe_need_length(pfirstframe) + TXDESC_SIZE+(pfirstframe->pkt_offset*PACKET_OFFSET_SZ);
	pbuf_tail = len;
	pbuf = _RND8(pbuf_tail);

	/* check pkt amount in one bulk */
	descCount = 0;
	bulkPtr = bulkSize;
	if (pbuf < bulkPtr)
		descCount++;
	else {
		descCount = 0;
		bulkPtr = ((pbuf / bulkSize) + 1) * bulkSize; /* round to next bulkSize */
	}

	/* dequeue same priority packet from station tx queue */
	psta = pfirstframe->attrib.psta;
	switch (pfirstframe->attrib.priority) {
	case 1:
	case 2:
		ptxservq = &(psta->sta_xmitpriv.bk_q);
		phwxmit = pxmitpriv->hwxmits + 3;
		break;

	case 4:
	case 5:
		ptxservq = &(psta->sta_xmitpriv.vi_q);
		phwxmit = pxmitpriv->hwxmits + 1;
		break;

	case 6:
	case 7:
		ptxservq = &(psta->sta_xmitpriv.vo_q);
		phwxmit = pxmitpriv->hwxmits;
		break;

	case 0:
	case 3:
	default:
		ptxservq = &(psta->sta_xmitpriv.be_q);
		phwxmit = pxmitpriv->hwxmits + 2;
		break;
	}
/*
 * DBG_8192C("==> pkt_no=%d,pkt_len=%d,len=%d,RND8_LEN=%d,pkt_offset=0x%02x\n",
 * 	pxmitframe->agg_num,pxmitframe->attrib.last_txcmdsz,len,pbuf,pxmitframe->pkt_offset );
 */

	_enter_critical_bh(&pxmitpriv->lock, &irqL);

	xmitframe_phead = get_list_head(&ptxservq->sta_pending);
	xmitframe_plist = get_next(xmitframe_phead);

	while (rtw_end_of_queue_search(xmitframe_phead, xmitframe_plist) == _FALSE) {
		pxmitframe = LIST_CONTAINOR(xmitframe_plist, struct xmit_frame, list);
		xmitframe_plist = get_next(xmitframe_plist);

		pxmitframe->agg_num = 0; 	/* not first frame of aggregation */
		pxmitframe->pkt_offset = 0; 	/* not first frame of aggregation, no need to reserve offset */

		len = xmitframe_need_length(pxmitframe) + TXDESC_SIZE + (pxmitframe->pkt_offset*PACKET_OFFSET_SZ);

		if (_RND8(pbuf + len) > MAX_XMITBUF_SZ) {
		/* if (_RND8(pbuf + len) > (MAX_XMITBUF_SZ/2))//to do : for TX TP finial tune , Georgia 2012-0323 */

			/* DBG_8192C("%s....len> MAX_XMITBUF_SZ\n",__FUNCTION__); */
			pxmitframe->agg_num = 1;
			pxmitframe->pkt_offset = 1;
			break;
		}
		rtw_list_delete(&pxmitframe->list);
		ptxservq->qcnt--;
		phwxmit->accnt--;

#ifndef IDEA_CONDITION
		/*  suppose only data frames would be in queue */
		if (pxmitframe->frame_tag != DATA_FRAMETAG) {
			rtw_free_xmitframe(pxmitpriv, pxmitframe);
			continue;
		}

		/* TID 0~15 */
		if ((pxmitframe->attrib.priority < 0) ||
		    (pxmitframe->attrib.priority > 15)) {
			rtw_free_xmitframe(pxmitpriv, pxmitframe);
			continue;
		}
#endif

		/* pxmitframe->pxmitbuf = pxmitbuf; */
		pxmitframe->buf_addr = pxmitbuf->pbuf + pbuf;

		if (rtw_xmitframe_coalesce(padapter, pxmitframe->pkt, pxmitframe) == _FALSE) {
			DBG_871X("%s coalesce failed \n", __FUNCTION__);
			rtw_free_xmitframe(pxmitpriv, pxmitframe);
			continue;
		}

		/*
		 * DBG_8192C("==> pxmitframe->attrib.priority:%d\n",pxmitframe->attrib.priority);
		 * always return ndis_packet after rtw_xmitframe_coalesce
		 */
		rtw_os_xmit_complete(padapter, pxmitframe);

		/* (len - TXDESC_SIZE) == pxmitframe->attrib.last_txcmdsz */
		update_txdesc(pxmitframe, pxmitframe->buf_addr, pxmitframe->attrib.last_txcmdsz, _TRUE);

		/* don't need xmitframe any more */
		rtw_free_xmitframe(pxmitpriv, pxmitframe);

		/* handle pointer and stop condition */
		pbuf_tail = pbuf + len;
		pbuf = _RND8(pbuf_tail);


		pfirstframe->agg_num++;
		if (MAX_TX_AGG_PACKET_NUMBER == pfirstframe->agg_num)
			break;

		if (pbuf < bulkPtr) {
			descCount++;
			if (descCount == pHalData->UsbTxAggDescNum)
				break;
		} else {
			descCount = 0;
			bulkPtr = ((pbuf / bulkSize) + 1) * bulkSize;
		}
	}	/* end while( aggregate same priority and same DA(AP or STA) frames) */


	if (_rtw_queue_empty(&ptxservq->sta_pending) == _TRUE)
		rtw_list_delete(&ptxservq->tx_pending);

	_exit_critical_bh(&pxmitpriv->lock, &irqL);
#ifdef CONFIG_80211N_HT
	if ((pfirstframe->attrib.ether_type != 0x0806) &&
	    (pfirstframe->attrib.ether_type != 0x888e) &&
	    (pfirstframe->attrib.ether_type != 0x88b4) &&
	    (pfirstframe->attrib.dhcp_pkt != 1)) {
		rtw_issue_addbareq_cmd(padapter, pfirstframe);
	}
#endif
	/* 3 3. update first frame txdesc */
	if ((pbuf_tail % bulkSize) == 0) {
		/* remove pkt_offset */
		pbuf_tail -= PACKET_OFFSET_SZ;
		pfirstframe->buf_addr += PACKET_OFFSET_SZ;
		pfirstframe->pkt_offset--;
		/* DBG_8192C("$$$$$ buf size equal to USB block size $$$$$$\n"); */
	}

	update_txdesc(pfirstframe, pfirstframe->buf_addr, pfirstframe->attrib.last_txcmdsz, _TRUE);

	/* 3 4. write xmit buffer to USB FIFO */
	ff_hwaddr = rtw_get_ff_hwaddr(pfirstframe);
/* DBG_8192C("%s ===================================== write port,buf_size(%d) \n",__FUNCTION__,pbuf_tail); */
	/* xmit address == ((xmit_frame*)pxmitbuf->priv_data)->buf_addr */
	rtw_write_port(padapter, ff_hwaddr, pbuf_tail, (u8 *)pxmitbuf);


	/* 3 5. update statisitc */
	pbuf_tail -= (pfirstframe->agg_num * TXDESC_SIZE);
	pbuf_tail -= (pfirstframe->pkt_offset * PACKET_OFFSET_SZ);


	rtw_count_tx_stats(padapter, pfirstframe, pbuf_tail);

	rtw_free_xmitframe(pxmitpriv, pfirstframe);

	return _TRUE;
}

#else

int32_t rtl8812au_xmitframe_complete(struct rtl_priv *padapter, struct xmit_priv *pxmitpriv, struct xmit_buf *pxmitbuf)
{

	struct hw_xmit *phwxmits;
	sint hwentry;
	struct xmit_frame *pxmitframe = NULL;
	int res = _SUCCESS, xcnt = 0;

	phwxmits = pxmitpriv->hwxmits;
	hwentry = pxmitpriv->hwxmit_entry;

	if (pxmitbuf == NULL) {
		pxmitbuf = rtw_alloc_xmitbuf(pxmitpriv);
		if (!pxmitbuf) {
			return _FALSE;
		}
	}


	do {
		pxmitframe =  rtw_dequeue_xframe(pxmitpriv, phwxmits, hwentry);

		if (pxmitframe) {
			pxmitframe->pxmitbuf = pxmitbuf;

			pxmitframe->buf_addr = pxmitbuf->pbuf;

			pxmitbuf->priv_data = pxmitframe;

			if ((pxmitframe->frame_tag&0x0f) == DATA_FRAMETAG) {
				if (pxmitframe->attrib.priority <= 15) {
					/* TID0~15 */
					res = rtw_xmitframe_coalesce(padapter, pxmitframe->pkt, pxmitframe);
				}
				/* DBG_8192C("==> pxmitframe->attrib.priority:%d\n",pxmitframe->attrib.priority); */
				rtw_os_xmit_complete(padapter, pxmitframe);	/* always return ndis_packet after rtw_xmitframe_coalesce */
			}

			if (res == _SUCCESS) {
				rtw_dump_xframe(padapter, pxmitframe);
			} else {
				rtw_free_xmitbuf(pxmitpriv, pxmitbuf);
				rtw_free_xmitframe(pxmitpriv, pxmitframe);
			}

			xcnt++;

		} else 	{
			rtw_free_xmitbuf(pxmitpriv, pxmitbuf);
			return _FALSE;
		}

		break;

	} while (0 /*xcnt < (NR_XMITFRAME >> 3)*/);

	return _TRUE;

}
#endif



static int32_t xmitframe_direct(struct rtl_priv *padapter, struct xmit_frame *pxmitframe)
{
	int32_t res = _SUCCESS;
	/* DBG_8192C("==> %s \n",__FUNCTION__); */

	res = rtw_xmitframe_coalesce(padapter, pxmitframe->pkt, pxmitframe);
	if (res == _SUCCESS) {
		rtw_dump_xframe(padapter, pxmitframe);
	} else {
		DBG_8192C("==> %s xmitframe_coalsece failed\n", __FUNCTION__);
	}

	return res;
}

/*
 * Return
 *	_TRUE	dump packet directly
 *	_FALSE	enqueue packet
 */
static int32_t pre_xmitframe(struct rtl_priv *padapter, struct xmit_frame *pxmitframe)
{
	_irqL irqL;
	int32_t res;
	struct xmit_buf *pxmitbuf = NULL;
	struct xmit_priv *pxmitpriv = &padapter->xmitpriv;
	struct pkt_attrib *pattrib = &pxmitframe->attrib;
	struct mlme_priv *pmlmepriv = &padapter->mlmepriv;

	_enter_critical_bh(&pxmitpriv->lock, &irqL);

	/* DBG_8192C("==> %s \n",__FUNCTION__); */

	if (rtw_txframes_sta_ac_pending(padapter, pattrib) > 0) {
		/* DBG_8192C("enqueue AC(%d)\n",pattrib->priority); */
		goto enqueue;
	}


	if (check_fwstate(pmlmepriv, _FW_UNDER_SURVEY|_FW_UNDER_LINKING) == _TRUE)
		goto enqueue;

	pxmitbuf = rtw_alloc_xmitbuf(pxmitpriv);
	if (pxmitbuf == NULL)
		goto enqueue;

	_exit_critical_bh(&pxmitpriv->lock, &irqL);

	pxmitframe->pxmitbuf = pxmitbuf;
	pxmitframe->buf_addr = pxmitbuf->pbuf;
	pxmitbuf->priv_data = pxmitframe;

	if (xmitframe_direct(padapter, pxmitframe) != _SUCCESS) {
		rtw_free_xmitbuf(pxmitpriv, pxmitbuf);
		rtw_free_xmitframe(pxmitpriv, pxmitframe);
	}

	return _TRUE;

enqueue:
	res = rtw_xmitframe_enqueue(padapter, pxmitframe);
	_exit_critical_bh(&pxmitpriv->lock, &irqL);

	if (res != _SUCCESS) {
		rtw_free_xmitframe(pxmitpriv, pxmitframe);

		/* Trick, make the statistics correct */
		pxmitpriv->tx_pkts--;
		pxmitpriv->tx_drop++;
		return _TRUE;
	}

	return _FALSE;
}

int32_t rtl8812au_mgnt_xmit(struct rtl_priv *padapter, struct xmit_frame *pmgntframe)
{
	return rtw_dump_xframe(padapter, pmgntframe);
}

/*
 * Return
 *	_TRUE	dump packet directly ok
 *	_FALSE	temporary can't transmit packets to hardware
 */
int32_t rtl8812au_hal_xmit(struct rtl_priv *padapter, struct xmit_frame *pxmitframe)
{
	return pre_xmitframe(padapter, pxmitframe);
}

int32_t	 rtl8812au_hal_xmitframe_enqueue(struct rtl_priv *padapter, struct xmit_frame *pxmitframe)
{
	struct xmit_priv 	*pxmitpriv = &padapter->xmitpriv;
	int32_t err;

	err = rtw_xmitframe_enqueue(padapter, pxmitframe);
	if (err != _SUCCESS) {
		rtw_free_xmitframe(pxmitpriv, pxmitframe);

		/* Trick, make the statistics correct */
		pxmitpriv->tx_pkts--;
		pxmitpriv->tx_drop++;
	} else {
#ifdef PLATFORM_LINUX
		tasklet_hi_schedule(&pxmitpriv->xmit_tasklet);
#endif
	}

	return err;

}

void _dbg_dump_tx_info(struct rtl_priv	*padapter,int frame_tag, uint8_t *ptxdesc)
{
	uint8_t bDumpTxPkt;
	uint8_t bDumpTxDesc = _FALSE;
	rtw_hal_get_def_var(padapter, HAL_DEF_DBG_DUMP_TXPKT, &(bDumpTxPkt));

	if(bDumpTxPkt ==1){		/* dump txdesc for data frame */
		DBG_871X("dump tx_desc for data frame\n");
		if((frame_tag&0x0f) == DATA_FRAMETAG){
			bDumpTxDesc = _TRUE;
		}
	} else if(bDumpTxPkt ==2){	/* dump txdesc for mgnt frame */
		DBG_871X("dump tx_desc for mgnt frame\n");
		if((frame_tag&0x0f) == MGNT_FRAMETAG){
			bDumpTxDesc = _TRUE;
		}
	}
	else if(bDumpTxPkt ==3){	/* dump early info */
	}

	if(bDumpTxDesc){
		/*
		 * ptxdesc->txdw4 = cpu_to_le32(0x00001006);//RTS Rate=24M
		 * ptxdesc->txdw6 = 0x6666f800;
		 */
		DBG_8192C("=====================================\n");
		DBG_8192C("Offset00(0x%08x)\n",*((uint32_t *)(ptxdesc)));
		DBG_8192C("Offset04(0x%08x)\n",*((uint32_t *)(ptxdesc+4)));
		DBG_8192C("Offset08(0x%08x)\n",*((uint32_t *)(ptxdesc+8)));
		DBG_8192C("Offset12(0x%08x)\n",*((uint32_t *)(ptxdesc+12)));
		DBG_8192C("Offset16(0x%08x)\n",*((uint32_t *)(ptxdesc+16)));
		DBG_8192C("Offset20(0x%08x)\n",*((uint32_t *)(ptxdesc+20)));
		DBG_8192C("Offset24(0x%08x)\n",*((uint32_t *)(ptxdesc+24)));
		DBG_8192C("Offset28(0x%08x)\n",*((uint32_t *)(ptxdesc+28)));
		DBG_8192C("Offset32(0x%08x)\n",*((uint32_t *)(ptxdesc+32)));
		DBG_8192C("Offset36(0x%08x)\n",*((uint32_t *)(ptxdesc+36)));
		DBG_8192C("=====================================\n");
	}

}

/*
 * Description:
 *	Aggregation packets and send to hardware
 *
 * Return:
 *	0	Success
 *	-1	Hardware resource(TX FIFO) not ready
 *	-2	Software resource(xmitbuf) not ready
 */



u8 BWMapping_8812(struct rtl_priv *Adapter, struct pkt_attrib *pattrib)
{
	uint8_t	BWSettingOfDesc = 0;
	struct _rtw_hal *pHalData = GET_HAL_DATA(Adapter);

	/*
	 * DBG_871X("BWMapping pHalData->CurrentChannelBW %d, pattrib->bwmode %d \n",pHalData->CurrentChannelBW,pattrib->bwmode);
	 */

	if (Adapter->phy.current_chan_bw == CHANNEL_WIDTH_80) {
		if (pattrib->bwmode == CHANNEL_WIDTH_80)
			BWSettingOfDesc= 2;
		else if (pattrib->bwmode == CHANNEL_WIDTH_40)
			BWSettingOfDesc = 1;
		else
			BWSettingOfDesc = 0;
	} else if(Adapter->phy.current_chan_bw == CHANNEL_WIDTH_40) {
		if((pattrib->bwmode == CHANNEL_WIDTH_40) || (pattrib->bwmode == CHANNEL_WIDTH_80))
			BWSettingOfDesc = 1;
		else
			BWSettingOfDesc = 0;
	} else
		BWSettingOfDesc = 0;

	return BWSettingOfDesc;
}

u8 SCMapping_8812(struct rtl_priv *Adapter, struct pkt_attrib *pattrib)
{
	uint8_t	SCSettingOfDesc = 0;
	struct _rtw_hal *pHalData = GET_HAL_DATA(Adapter);

	/*
	 * DBG_871X("SCMapping: pHalData->CurrentChannelBW %d, pHalData->nCur80MhzPrimeSC %d, pHalData->nCur40MhzPrimeSC %d \n",pHalData->CurrentChannelBW,pHalData->nCur80MhzPrimeSC,pHalData->nCur40MhzPrimeSC);
	 */

	if (Adapter->phy.current_chan_bw == CHANNEL_WIDTH_80) {
		if(pattrib->bwmode == CHANNEL_WIDTH_80) {
			SCSettingOfDesc = VHT_DATA_SC_DONOT_CARE;
		} else if(pattrib->bwmode == CHANNEL_WIDTH_40) {
			if(pHalData->nCur80MhzPrimeSC == HAL_PRIME_CHNL_OFFSET_LOWER)
				SCSettingOfDesc = VHT_DATA_SC_40_LOWER_OF_80MHZ;
			else if(pHalData->nCur80MhzPrimeSC == HAL_PRIME_CHNL_OFFSET_UPPER)
				SCSettingOfDesc = VHT_DATA_SC_40_UPPER_OF_80MHZ;
			else
				DBG_871X("SCMapping: Not Correct Primary40MHz Setting \n");
		} else {
			if((pHalData->nCur40MhzPrimeSC == HAL_PRIME_CHNL_OFFSET_LOWER) && (pHalData->nCur80MhzPrimeSC == HAL_PRIME_CHNL_OFFSET_LOWER))
				SCSettingOfDesc = VHT_DATA_SC_20_LOWEST_OF_80MHZ;
			else if((pHalData->nCur40MhzPrimeSC == HAL_PRIME_CHNL_OFFSET_UPPER) && (pHalData->nCur80MhzPrimeSC == HAL_PRIME_CHNL_OFFSET_LOWER))
				SCSettingOfDesc = VHT_DATA_SC_20_LOWER_OF_80MHZ;
			else if((pHalData->nCur40MhzPrimeSC == HAL_PRIME_CHNL_OFFSET_LOWER) && (pHalData->nCur80MhzPrimeSC == HAL_PRIME_CHNL_OFFSET_UPPER))
				SCSettingOfDesc = VHT_DATA_SC_20_UPPER_OF_80MHZ;
			else if((pHalData->nCur40MhzPrimeSC == HAL_PRIME_CHNL_OFFSET_UPPER) && (pHalData->nCur80MhzPrimeSC == HAL_PRIME_CHNL_OFFSET_UPPER))
				SCSettingOfDesc = VHT_DATA_SC_20_UPPERST_OF_80MHZ;
			else
				DBG_871X("SCMapping: Not Correct Primary40MHz Setting \n");
		}
	} else if(Adapter->phy.current_chan_bw== CHANNEL_WIDTH_40) {
		/*
		 * DBG_871X("SCMapping: HT Case: pHalData->CurrentChannelBW %d, pHalData->nCur40MhzPrimeSC %d \n",pHalData->CurrentChannelBW,pHalData->nCur40MhzPrimeSC);
		 */

		if(pattrib->bwmode == CHANNEL_WIDTH_40) {
			SCSettingOfDesc = VHT_DATA_SC_DONOT_CARE;
		} else if(pattrib->bwmode == CHANNEL_WIDTH_20) {
			if(pHalData->nCur40MhzPrimeSC == HAL_PRIME_CHNL_OFFSET_UPPER) {
				SCSettingOfDesc = VHT_DATA_SC_20_UPPER_OF_80MHZ;
			} else if(pHalData->nCur40MhzPrimeSC == HAL_PRIME_CHNL_OFFSET_LOWER) {
				SCSettingOfDesc = VHT_DATA_SC_20_LOWER_OF_80MHZ;
			} else 		{
				SCSettingOfDesc = VHT_DATA_SC_DONOT_CARE;
			}

		}
	} else {
		SCSettingOfDesc = VHT_DATA_SC_DONOT_CARE;
	}

	return SCSettingOfDesc;
}



