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
 *
 ******************************************************************************/
#define _RTW_XMIT_C_

#include <drv_types.h>
#include <rtw_debug.h>

static uint8_t P802_1H_OUI[P80211_OUI_LEN] = { 0x00, 0x00, 0xf8 };
static uint8_t RFC1042_OUI[P80211_OUI_LEN] = { 0x00, 0x00, 0x00 };

static void _init_txservq(struct tx_servq *ptxservq)
{
	INIT_LIST_HEAD(&ptxservq->tx_pending);
	_rtw_init_queue(&ptxservq->sta_pending);
	ptxservq->qcnt = 0;
}


void	_rtw_init_sta_xmit_priv(struct sta_xmit_priv *psta_xmitpriv)
{
	memset((unsigned char *)psta_xmitpriv, 0, sizeof (struct sta_xmit_priv));

	spin_lock_init(&psta_xmitpriv->lock);

	/*
	 * for(i = 0 ; i < MAX_NUMBLKS; i++)
	 * 	_init_txservq(&(psta_xmitpriv->blk_q[i]));
	 */

	_init_txservq(&psta_xmitpriv->be_q);
	_init_txservq(&psta_xmitpriv->bk_q);
	_init_txservq(&psta_xmitpriv->vi_q);
	_init_txservq(&psta_xmitpriv->vo_q);
	INIT_LIST_HEAD(&psta_xmitpriv->legacy_dz);
	INIT_LIST_HEAD(&psta_xmitpriv->apsd);
}

int32_t	_rtw_init_xmit_priv(struct xmit_priv *pxmitpriv, struct rtl_priv *rtlpriv)
{
	int i;
	struct xmit_buf *pxmitbuf;
	struct xmit_frame *pxframe;
	sint	res = _SUCCESS;
	uint32_t	 max_xmit_extbuf_size = MAX_XMIT_EXTBUF_SZ;
	uint32_t	 num_xmit_extbuf = NR_XMIT_EXTBUFF;

	/*
	 * We don't need to memset rtlpriv->XXX to zero, because rtlpriv is allocated by rtw_zvmalloc().
	 * memset((unsigned char *)pxmitpriv, 0, sizeof(struct xmit_priv));
	 */

	spin_lock_init(&pxmitpriv->lock);
	spin_lock_init(&pxmitpriv->lock_sctx);
	sema_init(&pxmitpriv->xmit_sema, 0);
	sema_init(&pxmitpriv->terminate_xmitthread_sema, 0);

	/*
	Please insert all the queue initializaiton using _rtw_init_queue below
	*/

	pxmitpriv->rtlpriv = rtlpriv;

	/*
	 * for(i = 0 ; i < MAX_NUMBLKS; i++)
	 * 	_rtw_init_queue(&pxmitpriv->blk_strms[i]);
	 */

	_rtw_init_queue(&pxmitpriv->be_pending);
	_rtw_init_queue(&pxmitpriv->bk_pending);
	_rtw_init_queue(&pxmitpriv->vi_pending);
	_rtw_init_queue(&pxmitpriv->vo_pending);
	_rtw_init_queue(&pxmitpriv->bm_pending);

	/*
	 * _rtw_init_queue(&pxmitpriv->legacy_dz_queue);
	 * _rtw_init_queue(&pxmitpriv->apsd_queue);
	 */

	_rtw_init_queue(&pxmitpriv->free_xmit_queue);

	/*
	Please allocate memory with the sz = (struct xmit_frame) * NR_XMITFRAME,
	and initialize free_xmit_frame below.
	Please also apply  free_txobj to link_up all the xmit_frames...
	*/

	pxmitpriv->pallocated_frame_buf = rtw_zvmalloc(NR_XMITFRAME * sizeof(struct xmit_frame) + 4);

	if (pxmitpriv->pallocated_frame_buf == NULL) {
		pxmitpriv->pxmit_frame_buf = NULL;
		res = _FAIL;
		goto exit;
	}
	pxmitpriv->pxmit_frame_buf = (uint8_t *)N_BYTE_ALIGMENT((SIZE_PTR)(pxmitpriv->pallocated_frame_buf), 4);
	/*
	 * pxmitpriv->pxmit_frame_buf = pxmitpriv->pallocated_frame_buf + 4 -
	 * 						((SIZE_PTR) (pxmitpriv->pallocated_frame_buf) &3);
	 */

	pxframe = (struct xmit_frame *) pxmitpriv->pxmit_frame_buf;

	for (i = 0; i < NR_XMITFRAME; i++) {
		INIT_LIST_HEAD(&(pxframe->list));

		pxframe->rtlpriv = rtlpriv;
		pxframe->frame_tag = NULL_FRAMETAG;

		pxframe->pkt = NULL;

		pxframe->buf_addr = NULL;
		pxframe->pxmitbuf = NULL;

		list_add_tail(&(pxframe->list), &(pxmitpriv->free_xmit_queue.queue));

		pxframe++;
	}

	pxmitpriv->free_xmitframe_cnt = NR_XMITFRAME;

	pxmitpriv->frag_len = MAX_FRAG_THRESHOLD;


	/* init xmit_buf */
	_rtw_init_queue(&pxmitpriv->free_xmitbuf_queue);
	_rtw_init_queue(&pxmitpriv->pending_xmitbuf_queue);

	pxmitpriv->pallocated_xmitbuf = rtw_zvmalloc(NR_XMITBUFF * sizeof(struct xmit_buf) + 4);

	if (pxmitpriv->pallocated_xmitbuf == NULL) {
		res = _FAIL;
		goto exit;
	}

	pxmitpriv->pxmitbuf = (uint8_t *)N_BYTE_ALIGMENT((SIZE_PTR)(pxmitpriv->pallocated_xmitbuf), 4);
	/*
	 * pxmitpriv->pxmitbuf = pxmitpriv->pallocated_xmitbuf + 4 -
	 * 						((SIZE_PTR) (pxmitpriv->pallocated_xmitbuf) &3);
	 */

	pxmitbuf = (struct xmit_buf *)pxmitpriv->pxmitbuf;

	for (i = 0; i < NR_XMITBUFF; i++) {
		INIT_LIST_HEAD(&pxmitbuf->list);

		pxmitbuf->priv_data = NULL;
		pxmitbuf->rtlpriv = rtlpriv;
		pxmitbuf->buf_tag = XMITBUF_DATA;

		/* Tx buf allocation may fail sometimes, so sleep and retry. */
		res = rtw_os_xmit_resource_alloc(rtlpriv, pxmitbuf, (MAX_XMITBUF_SZ + XMITBUF_ALIGN_SZ), _TRUE);
		if (res == _FAIL) {
			msleep(10);
			res = rtw_os_xmit_resource_alloc(rtlpriv, pxmitbuf, (MAX_XMITBUF_SZ + XMITBUF_ALIGN_SZ), _TRUE);
			if (res == _FAIL) {
				goto exit;
			}
		}


		pxmitbuf->flags = XMIT_VO_QUEUE;

		list_add_tail(&pxmitbuf->list, &(pxmitpriv->free_xmitbuf_queue.queue));

		pxmitbuf++;

	}

	pxmitpriv->free_xmitbuf_cnt = NR_XMITBUFF;

	/* init xframe_ext queue,  the same count as extbuf  */
	_rtw_init_queue(&pxmitpriv->free_xframe_ext_queue);

	pxmitpriv->xframe_ext_alloc_addr = rtw_zvmalloc(num_xmit_extbuf * sizeof(struct xmit_frame) + 4);

	if (pxmitpriv->xframe_ext_alloc_addr  == NULL) {
		pxmitpriv->xframe_ext = NULL;
		res = _FAIL;
		goto exit;
	}
	pxmitpriv->xframe_ext = (uint8_t *)N_BYTE_ALIGMENT((SIZE_PTR)(pxmitpriv->xframe_ext_alloc_addr), 4);
	pxframe = (struct xmit_frame *)pxmitpriv->xframe_ext;

	for (i = 0; i < num_xmit_extbuf; i++) {
		INIT_LIST_HEAD(&(pxframe->list));

		pxframe->rtlpriv = rtlpriv;
		pxframe->frame_tag = NULL_FRAMETAG;

		pxframe->pkt = NULL;

		pxframe->buf_addr = NULL;
		pxframe->pxmitbuf = NULL;

		pxframe->ext_tag = 1;

		list_add_tail(&(pxframe->list), &(pxmitpriv->free_xframe_ext_queue.queue));

		pxframe++;
	}
	pxmitpriv->free_xframe_ext_cnt = num_xmit_extbuf;

	/* Init xmit extension buff */
	_rtw_init_queue(&pxmitpriv->free_xmit_extbuf_queue);

	pxmitpriv->pallocated_xmit_extbuf = rtw_zvmalloc(num_xmit_extbuf * sizeof(struct xmit_buf) + 4);

	if (pxmitpriv->pallocated_xmit_extbuf  == NULL) {
		res = _FAIL;
		goto exit;
	}

	pxmitpriv->pxmit_extbuf = (uint8_t *)N_BYTE_ALIGMENT((SIZE_PTR)(pxmitpriv->pallocated_xmit_extbuf), 4);

	pxmitbuf = (struct xmit_buf *)pxmitpriv->pxmit_extbuf;

	for (i = 0; i < num_xmit_extbuf; i++) {
		INIT_LIST_HEAD(&pxmitbuf->list);

		pxmitbuf->priv_data = NULL;
		pxmitbuf->rtlpriv = rtlpriv;
		pxmitbuf->buf_tag = XMITBUF_MGNT;

		res = rtw_os_xmit_resource_alloc(rtlpriv, pxmitbuf, max_xmit_extbuf_size + XMITBUF_ALIGN_SZ, _TRUE);
		if (res == _FAIL) {
			res = _FAIL;
			goto exit;
		}


		list_add_tail(&pxmitbuf->list, &(pxmitpriv->free_xmit_extbuf_queue.queue));
		pxmitbuf++;

	}

	pxmitpriv->free_xmit_extbuf_cnt = num_xmit_extbuf;


	pxmitbuf = &pxmitpriv->pcmd_xmitbuf;
	if (pxmitbuf) {
		INIT_LIST_HEAD(&pxmitbuf->list);

		pxmitbuf->priv_data = NULL;
		pxmitbuf->rtlpriv = rtlpriv;
		pxmitbuf->buf_tag = XMITBUF_CMD;

		res = rtw_os_xmit_resource_alloc(rtlpriv, pxmitbuf, 0, _TRUE);
		if (res == _FAIL) {
			res = _FAIL;
			goto exit;
		}
	}

	rtw_alloc_hwxmits(rtlpriv);
	rtw_init_hwxmits(pxmitpriv->hwxmits, pxmitpriv->hwxmit_entry);

	for (i = 0; i < 4; i++) {
		pxmitpriv->wmm_para_seq[i] = i;
	}

	pxmitpriv->txirp_cnt = 1;

	sema_init(&(pxmitpriv->tx_retevt), 0);

	/* per AC pending irp */
	pxmitpriv->beq_cnt = 0;
	pxmitpriv->bkq_cnt = 0;
	pxmitpriv->viq_cnt = 0;
	pxmitpriv->voq_cnt = 0;

	rtw_hal_init_xmit_priv(rtlpriv);

exit:

	return res;
}



void _rtw_free_xmit_priv (struct xmit_priv *pxmitpriv)
{
	int i;
	struct rtl_priv *rtlpriv = pxmitpriv->rtlpriv;
	struct xmit_frame *pxmitframe = (struct xmit_frame *) pxmitpriv->pxmit_frame_buf;
	struct xmit_buf *pxmitbuf = (struct xmit_buf *)pxmitpriv->pxmitbuf;
	uint32_t	 max_xmit_extbuf_size = MAX_XMIT_EXTBUF_SZ;
	uint32_t	 num_xmit_extbuf = NR_XMIT_EXTBUFF;

	rtw_hal_free_xmit_priv(rtlpriv);

	if (pxmitpriv->pxmit_frame_buf == NULL)
		goto out;

	for (i = 0; i < NR_XMITFRAME; i++) {
		rtw_os_xmit_complete(rtlpriv, pxmitframe);

		pxmitframe++;
	}

	for (i = 0; i < NR_XMITBUFF; i++) {
		rtw_os_xmit_resource_free(rtlpriv, pxmitbuf, (MAX_XMITBUF_SZ + XMITBUF_ALIGN_SZ), _TRUE);

		pxmitbuf++;
	}

	if (pxmitpriv->pallocated_frame_buf) {
		rtw_vmfree(pxmitpriv->pallocated_frame_buf);
	}


	if (pxmitpriv->pallocated_xmitbuf) {
		rtw_vmfree(pxmitpriv->pallocated_xmitbuf);
	}

	/* free xframe_ext queue,  the same count as extbuf  */
	pxmitframe = (struct xmit_frame *)pxmitpriv->xframe_ext;
	if (pxmitframe) {
		for (i = 0; i < num_xmit_extbuf; i++) {
			rtw_os_xmit_complete(rtlpriv, pxmitframe);
			pxmitframe++;
		}
	}
	if (pxmitpriv->xframe_ext_alloc_addr)
		rtw_vmfree(pxmitpriv->xframe_ext_alloc_addr);

	pxmitbuf = (struct xmit_buf *)pxmitpriv->pxmit_extbuf;

	for (i = 0; i < num_xmit_extbuf; i++) {
		rtw_os_xmit_resource_free(rtlpriv, pxmitbuf, (max_xmit_extbuf_size + XMITBUF_ALIGN_SZ), _TRUE);

		pxmitbuf++;
	}

	if (pxmitpriv->pallocated_xmit_extbuf) {
		rtw_vmfree(pxmitpriv->pallocated_xmit_extbuf);
	}

	pxmitbuf = &pxmitpriv->pcmd_xmitbuf;
	rtw_os_xmit_resource_free(rtlpriv, pxmitbuf, 0, _TRUE);

	rtw_free_hwxmits(rtlpriv);

out:
	;
}

static void update_attrib_vcs_info(struct rtl_priv *rtlpriv, struct xmit_frame *pxmitframe)
{
	struct rtl_hal *rtlhal = rtl_hal(rtlpriv);
	uint32_t	sz;
	struct pkt_attrib	*pattrib = &pxmitframe->attrib;
	/* struct sta_info	*psta = pattrib->psta; */
	struct mlme_ext_priv	*pmlmeext = &(rtlpriv->mlmeextpriv);
	struct mlme_ext_info	*pmlmeinfo = &(pmlmeext->mlmext_info);

/*
	if (pattrib->psta) {
		psta = pattrib->psta;
	} else {
		DBG_871X("%s, call rtw_get_stainfo()\n", __func__);
		psta=rtw_get_stainfo(&rtlpriv->stapriv ,&pattrib->ra[0] );
	}

	if (psta==NULL) {
		DBG_871X("%s, psta==NUL\n", __func__);
		return;
	}

	if (!(psta->state &_FW_LINKED)) {
		DBG_871X("%s, psta->state(0x%x) != _FW_LINKED\n", __func__, psta->state);
		return;
	}
*/

	if (pattrib->nr_frags != 1) {
		sz = rtlpriv->xmitpriv.frag_len;
	} else {	/* no frag */
		sz = pattrib->last_txcmdsz;
	}

	/*
	 *  (1) RTS_Threshold is compared to the MPDU, not MSDU.
	 *  (2) If there are more than one frag in  this MSDU, only the first frag uses protection frame.
	 * 		Other fragments are protected by previous fragment.
	 * 		So we only need to check the length of first fragment.
	 */
	if (pmlmeext->cur_wireless_mode < WIRELESS_11_24N) {
		if (sz > rtlpriv->registrypriv.rts_thresh) {
			pattrib->vcs_mode = RTS_CTS;
		} else {
			if (pattrib->rtsen)
				pattrib->vcs_mode = RTS_CTS;
			else if (pattrib->cts2self)
				pattrib->vcs_mode = CTS_TO_SELF;
			else
				pattrib->vcs_mode = NONE_VCS;
		}
	} else {
		while (_TRUE) {
			/* IOT action */
			if ((pmlmeinfo->assoc_AP_vendor == HT_IOT_PEER_ATHEROS)
			 && (pattrib->ampdu_en == _TRUE)
			 && (rtlpriv->securitypriv.dot11PrivacyAlgrthm == _AES_)) {
				pattrib->vcs_mode = CTS_TO_SELF;
				break;
			}

			/* check ERP protection */
			if (pattrib->rtsen || pattrib->cts2self) {
				if (pattrib->rtsen)
					pattrib->vcs_mode = RTS_CTS;
				else if (pattrib->cts2self)
					pattrib->vcs_mode = CTS_TO_SELF;

				break;
			}

			/* check HT op mode */
			if (pattrib->ht_en) {
				uint8_t HTOpMode = pmlmeinfo->HT_protection;
				if ((pmlmeext->cur_bwmode && (HTOpMode == 2 || HTOpMode == 3)) ||
				  (!pmlmeext->cur_bwmode && HTOpMode == 3)) {
					pattrib->vcs_mode = RTS_CTS;
					break;
				}
			}

			/* check rts */
			if (sz > rtlpriv->registrypriv.rts_thresh) {
				pattrib->vcs_mode = RTS_CTS;
				break;
			}

			/* to do list: check MIMO power save condition. */

			/* check AMPDU aggregation for TXOP */
#if 0			/* ULLI we maqy keep this here */			
			if ((pattrib->ampdu_en == _TRUE) && (!IS_HARDWARE_TYPE_JAGUAR(rtlhal))) {
				pattrib->vcs_mode = RTS_CTS;
				break;
			}
#endif
			pattrib->vcs_mode = NONE_VCS;
			break;
		}
	}
}

static void update_attrib_phy_info(struct pkt_attrib *pattrib, struct sta_info *psta)
{
	pattrib->rtsen = psta->rtsen;
	pattrib->cts2self = psta->cts2self;

	pattrib->mdata = 0;
	pattrib->eosp = 0;
	pattrib->triggered = 0;

	/* qos_en, ht_en, init rate, ,bw, ch_offset, sgi */
	pattrib->qos_en = psta->qos_option;

	pattrib->raid = psta->raid;
#ifdef CONFIG_80211N_HT
#ifdef CONFIG_80211AC_VHT
	if (psta->vhtpriv.vht_option) {
		pattrib->bwmode = psta->vhtpriv.bwmode;
		pattrib->sgi = psta->vhtpriv.sgi;

		if (TEST_FLAG(psta->vhtpriv.ldpc_cap, LDPC_VHT_ENABLE_TX))
			pattrib->ldpc = 1;

		if (TEST_FLAG(psta->vhtpriv.stbc_cap, STBC_VHT_ENABLE_TX))
			pattrib->stbc = 1;
	} else
#endif
	{
		pattrib->bwmode = psta->htpriv.bwmode;
		pattrib->sgi = psta->htpriv.sgi;
	}

	pattrib->ht_en = psta->htpriv.ht_option;
	pattrib->ch_offset = psta->htpriv.ch_offset;
	pattrib->ampdu_en = _FALSE;
#endif
	/*
	 * if(pattrib->ht_en && psta->htpriv.ampdu_enable) {
	 * 	if(psta->htpriv.agg_enable_bitmap & BIT(pattrib->priority))
	 * 		pattrib->ampdu_en = _TRUE;
	 * }
	 */

	pattrib->retry_ctrl = _FALSE;


}

static int32_t update_attrib_sec_info(struct rtl_priv *rtlpriv, struct pkt_attrib *pattrib, struct sta_info *psta)
{
	sint res = _SUCCESS;
	struct mlme_priv	*pmlmepriv = &rtlpriv->mlmepriv;
	struct security_priv *psecuritypriv = &rtlpriv->securitypriv;
	sint bmcast = IS_MCAST(pattrib->ra);

	memset(pattrib->dot118021x_UncstKey.skey,  0, 16);
	memset(pattrib->dot11tkiptxmickey.skey,  0, 16);

	if (psta->ieee8021x_blocked == _TRUE) {
		pattrib->encrypt = 0;

		if ((pattrib->ether_type != 0x888e)
		 && (check_fwstate(pmlmepriv, WIFI_MP_STATE) == _FALSE)) {
			res = _FAIL;
			goto exit;
		}
	} else {
		GET_ENCRY_ALGO(psecuritypriv, psta, pattrib->encrypt, bmcast);

		switch (psecuritypriv->dot11AuthAlgrthm) {
		case dot11AuthAlgrthm_Open:
		case dot11AuthAlgrthm_Shared:
		case dot11AuthAlgrthm_Auto:
			pattrib->key_idx = (uint8_t)psecuritypriv->dot11PrivacyKeyIndex;
			break;
		case dot11AuthAlgrthm_8021X:
			if (bmcast)
				pattrib->key_idx = (uint8_t)psecuritypriv->dot118021XGrpKeyid;
			else
				pattrib->key_idx = 0;
			break;
		default:
			pattrib->key_idx = 0;
			break;
		}

		/* For WPS 1.0 WEP, driver should not encrypt EAPOL Packet for WPS handshake. */
		if (((pattrib->encrypt == _WEP40_)
		 || (pattrib->encrypt == _WEP104_)) && (pattrib->ether_type == 0x888e))
			pattrib->encrypt = _NO_PRIVACY_;

	}

	switch (pattrib->encrypt) {
	case _WEP40_:
	case _WEP104_:
		pattrib->iv_len = 4;
		pattrib->icv_len = 4;
		WEP_IV(pattrib->iv, psta->dot11txpn, pattrib->key_idx);
		break;

	case _TKIP_:
		pattrib->iv_len = 8;
		pattrib->icv_len = 4;

		if (psecuritypriv->busetkipkey == _FAIL) {
			res = _FAIL;
			goto exit;
		}

		if (bmcast)
			TKIP_IV(pattrib->iv, psta->dot11txpn, pattrib->key_idx);
		else
			TKIP_IV(pattrib->iv, psta->dot11txpn, 0);


		memcpy(pattrib->dot11tkiptxmickey.skey, psta->dot11tkiptxmickey.skey, 16);

		break;

	case _AES_:
		pattrib->iv_len = 8;
		pattrib->icv_len = 8;

		if (bmcast)
			AES_IV(pattrib->iv, psta->dot11txpn, pattrib->key_idx);
		else
			AES_IV(pattrib->iv, psta->dot11txpn, 0);

		break;

	default:
		pattrib->iv_len = 0;
		pattrib->icv_len = 0;
		break;
	}

	if (pattrib->encrypt > 0)
		memcpy(pattrib->dot118021x_UncstKey.skey, psta->dot118021x_UncstKey.skey, 16);

exit:

	return res;

}

uint8_t	qos_acm(uint8_t acm_mask, uint8_t priority)
{
	uint8_t	change_priority = priority;

	switch (priority) {
	case 0:
	case 3:
		if (acm_mask & BIT(1))
			change_priority = 1;
		break;
	case 1:
	case 2:
		break;
	case 4:
	case 5:
		if (acm_mask & BIT(2))
			change_priority = 0;
		break;
	case 6:
	case 7:
		if (acm_mask & BIT(3))
			change_priority = 5;
		break;
	default:
		DBG_871X("qos_acm(): invalid pattrib->priority: %d!!!\n", priority);
		break;
	}

	return change_priority;
}

static void set_qos(struct pkt_file *ppktfile, struct pkt_attrib *pattrib)
{
	struct ethhdr etherhdr;
	struct iphdr ip_hdr;
	int32_t UserPriority = 0;


	_rtw_open_pktfile(ppktfile->pkt, ppktfile);
	_rtw_pktfile_read(ppktfile, (unsigned char *) &etherhdr, ETH_HLEN);

	/* get UserPriority from IP hdr */
	if (pattrib->ether_type == 0x0800) {
		_rtw_pktfile_read(ppktfile, (uint8_t *)&ip_hdr, sizeof(ip_hdr));
		/* UserPriority = (ntohs(ip_hdr.tos) >> 5) & 0x3; */
		UserPriority = ip_hdr.tos >> 5;
	} else if (pattrib->ether_type == 0x888e) {
		/*
		 *  "When priority processing of data frames is supported,
		 *  a STA's SME should send EAPOL-Key frames at the highest priority."
		 */
		UserPriority = 7;
	}

	pattrib->priority = UserPriority;
	pattrib->hdrlen = WLAN_HDR_A3_QOS_LEN;
	pattrib->subtype = WIFI_QOS_DATA_TYPE;
}

static int32_t update_attrib(struct rtl_priv *rtlpriv, struct sk_buff *pkt, struct pkt_attrib *pattrib)
{
	uint i;
	struct pkt_file pktfile;
	struct sta_info *psta = NULL;
	struct ethhdr etherhdr;

	sint bmcast;
	struct sta_priv		*pstapriv = &rtlpriv->stapriv;
	struct security_priv	*psecuritypriv = &rtlpriv->securitypriv;
	struct mlme_priv	*pmlmepriv = &rtlpriv->mlmepriv;
	struct qos_priv		*pqospriv = &pmlmepriv->qospriv;
	sint res = _SUCCESS;

	_rtw_open_pktfile(pkt, &pktfile);
	i = _rtw_pktfile_read(&pktfile, (uint8_t *)&etherhdr, ETH_HLEN);

	pattrib->ether_type = ntohs(etherhdr.h_proto);


	memcpy(pattrib->dst, &etherhdr.h_dest, ETH_ALEN);
	memcpy(pattrib->src, &etherhdr.h_source, ETH_ALEN);


	if ((check_fwstate(pmlmepriv, WIFI_ADHOC_STATE) == _TRUE) ||
		(check_fwstate(pmlmepriv, WIFI_ADHOC_MASTER_STATE) == _TRUE)) {
		memcpy(pattrib->ra, pattrib->dst, ETH_ALEN);
		memcpy(pattrib->ta, pattrib->src, ETH_ALEN);
	} else if (check_fwstate(pmlmepriv, WIFI_STATION_STATE)) {
		memcpy(pattrib->ra, get_bssid(pmlmepriv), ETH_ALEN);
		memcpy(pattrib->ta, pattrib->src, ETH_ALEN);
	} else if (check_fwstate(pmlmepriv, WIFI_AP_STATE)) {
		memcpy(pattrib->ra, pattrib->dst, ETH_ALEN);
		memcpy(pattrib->ta, get_bssid(pmlmepriv), ETH_ALEN);
	}

	pattrib->pktlen = pktfile.pkt_len;

	if (ETH_P_IP == pattrib->ether_type) {
		/*
		 *  The following is for DHCP and ARP packet, we use cck1M to tx these packets and let LPS awake some time
		 *  to prevent DHCP protocol fail
		 */
		uint8_t tmp[24];
		_rtw_pktfile_read(&pktfile, &tmp[0], 24);
		pattrib->dhcp_pkt = 0;
		if (pktfile.pkt_len > 282) {	/* MINIMUM_DHCP_PACKET_SIZE) { */
			if (ETH_P_IP == pattrib->ether_type) {
				/* IP header */
				if (((tmp[21] == 68) && (tmp[23] == 67)) ||
					((tmp[21] == 67) && (tmp[23] == 68))) {
					/* 68 : UDP BOOTP client */
					/* 67 : UDP BOOTP server */
					/*
					 * Use low rate to send DHCP packet.
					 * if(pMgntInfo->IOTAction & HT_IOT_ACT_WA_IOT_Broadcom) {
					 * 	tcb_desc->DataRate = MgntQuery_TxRateExcludeCCKRates(ieee);//0xc;//ofdm 6m
					 * 	tcb_desc->bTxDisableRateFallBack = false;
					 * } else
					 * 	pTcb->DataRate = rtlpriv->MgntInfo.LowestBasicRate;
					 * RTPRINT(FDM, WA_IOT, ("DHCP TranslateHeader(), pTcb->DataRate = 0x%x\n", pTcb->DataRate));
					 */
					pattrib->dhcp_pkt = 1;
				}
			}
		}
	} else if (0x888e == pattrib->ether_type) {
		DBG_871X_LEVEL(_drv_always_, "send eapol packet\n");
	}

#ifdef CONFIG_LPS
	/* If EAPOL , ARP , OR DHCP packet, driver must be in active mode. */
	if ((pattrib->ether_type == 0x0806) || (pattrib->ether_type == 0x888e) || (pattrib->dhcp_pkt == 1)) {
		rtw_lps_ctrl_wk_cmd(rtlpriv, LPS_CTRL_SPECIAL_PACKET, 1);
	}
#endif

	bmcast = IS_MCAST(pattrib->ra);

	/* get sta_info */
	if (bmcast) {
		psta = rtw_get_bcmc_stainfo(rtlpriv);
	} else {
		psta = rtw_get_stainfo(pstapriv, pattrib->ra);
		if (psta == NULL) {	/* if we cannot get psta => drrp the pkt */
			res = _FAIL;
			goto exit;
		} else if ((check_fwstate(pmlmepriv, WIFI_AP_STATE) == _TRUE) && (!(psta->state & _FW_LINKED))) {
			res = _FAIL;
			goto exit;
		}
	}

	if (psta == NULL) {	/* if we cannot get psta => drop the pkt */
		res = _FAIL;
		goto exit;
	}

	if (!(psta->state & _FW_LINKED))	{
		DBG_871X("%s, psta("MAC_FMT")->state(0x%x) != _FW_LINKED\n", __func__, MAC_ARG(psta->hwaddr), psta->state);
		return _FAIL;
	}



	/* TODO:spinlock_t */
	if (update_attrib_sec_info(rtlpriv, pattrib, psta) == _FAIL) {
		res = _FAIL;
		goto exit;
	}

	update_attrib_phy_info(pattrib, psta);

	pattrib->mac_id = psta->mac_id;
	/* DBG_8192C("%s ==> mac_id(%d)\n",__FUNCTION__,pattrib->mac_id ); */

	pattrib->psta = psta;
	/* TODO:_unlock */

	pattrib->pctrl = 0;

	pattrib->ack_policy = 0;
	/* get ether_hdr_len */
	pattrib->pkt_hdrlen = ETH_HLEN; /*(pattrib->ether_type == 0x8100) ? (14 + 4 ): 14; //vlan tag */

	pattrib->hdrlen = WLAN_HDR_A3_LEN;
	pattrib->subtype = WIFI_DATA_TYPE;
	pattrib->priority = 0;

	if (check_fwstate(pmlmepriv, WIFI_AP_STATE|WIFI_ADHOC_STATE|WIFI_ADHOC_MASTER_STATE)) {
		if (pattrib->qos_en)
			set_qos(&pktfile, pattrib);
	} else {
		if (pqospriv->qos_option) {
			set_qos(&pktfile, pattrib);

			if (pmlmepriv->acm_mask != 0) {
				pattrib->priority = qos_acm(pmlmepriv->acm_mask, pattrib->priority);
			}
		}
	}

	/* pattrib->priority = 5; //force to used VI queue, for testing */

	if (pattrib->encrypt &&
	    ((rtlpriv->securitypriv.sw_encrypt == _TRUE) || (psecuritypriv->hw_decrypted == _FALSE))) {
		pattrib->bswenc = _TRUE;
	} else {
		pattrib->bswenc = _FALSE;
	}

	rtw_set_tx_chksum_offload(pkt, pattrib);

exit:

	return res;
}

static int32_t xmitframe_addmic(struct rtl_priv *rtlpriv, struct xmit_frame *pxmitframe)
{
	sint curfragnum, length;
	uint8_t	*pframe, *payload, mic[8];
	struct	mic_data		micdata;
	/* struct	sta_info		*stainfo; */
	struct	qos_priv   *pqospriv = &(rtlpriv->mlmepriv.qospriv);
	struct	pkt_attrib	 *pattrib = &pxmitframe->attrib;
	struct 	security_priv	*psecuritypriv = &rtlpriv->securitypriv;
	struct	xmit_priv		*pxmitpriv = &rtlpriv->xmitpriv;
	uint8_t priority[4] = {0x0, 0x0, 0x0, 0x0};
	uint8_t hw_hdr_offset = 0;
	sint bmcst = IS_MCAST(pattrib->ra);

/*
	if(pattrib->psta)
	{
		stainfo = pattrib->psta;
	}
	else
	{
		DBG_871X("%s, call rtw_get_stainfo()\n", __func__);
		stainfo=rtw_get_stainfo(&rtlpriv->stapriv ,&pattrib->ra[0]);
	}

	if(stainfo==NULL)
	{
		DBG_871X("%s, psta==NUL\n", __func__);
		return _FAIL;
	}

	if(!(stainfo->state &_FW_LINKED))
	{
		DBG_871X("%s, psta->state(0x%x) != _FW_LINKED\n", __func__, stainfo->state);
		return _FAIL;
	}
*/

#ifdef CONFIG_USB_TX_AGGREGATION
	hw_hdr_offset = TXDESC_SIZE + (pxmitframe->pkt_offset * PACKET_OFFSET_SZ);;
#else
	hw_hdr_offset = TXDESC_OFFSET;
#endif

	if (pattrib->encrypt == _TKIP_) {	/* if(psecuritypriv->dot11PrivacyAlgrthm==_TKIP_PRIVACY_) */
		/*
		 * encode mic code
		 * if(stainfo!= NULL)
		 */
		{
			uint8_t null_key[16] = {
				0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
				0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0 };

			pframe = pxmitframe->buf_addr + hw_hdr_offset;

			if (bmcst) {
				if (_rtw_memcmp(psecuritypriv->dot118021XGrptxmickey[psecuritypriv->dot118021XGrpKeyid].skey, null_key, 16) == _TRUE) {
					/*
					 * DbgPrint("\nxmitframe_addmic:stainfo->dot11tkiptxmickey==0\n");
					 * msleep(10);
					 */
					return _FAIL;
				}
				/* start to calculate the mic code */
				rtw_secmicsetkey(&micdata, psecuritypriv->dot118021XGrptxmickey[psecuritypriv->dot118021XGrpKeyid].skey);
			} else 	{
				if (_rtw_memcmp(&pattrib->dot11tkiptxmickey.skey[0], null_key, 16) == _TRUE) {
					/*
					 * DbgPrint("\nxmitframe_addmic:stainfo->dot11tkiptxmickey==0\n");
					 * msleep(10);
					 */
					return _FAIL;
				}
				/* start to calculate the mic code */
				rtw_secmicsetkey(&micdata, &pattrib->dot11tkiptxmickey.skey[0]);
			}

			if (pframe[1] & 1) {	/* ToDS==1 */
				rtw_secmicappend(&micdata, &pframe[16], 6);  /* DA */
				if (pframe[1] & 2)  /* From Ds==1 */
					rtw_secmicappend(&micdata, &pframe[24], 6);
				else
					rtw_secmicappend(&micdata, &pframe[10], 6);
			} else {	/* ToDS==0 */
				rtw_secmicappend(&micdata, &pframe[4], 6);   /* DA */
				if (pframe[1]&2)  /* From Ds==1 */
					rtw_secmicappend(&micdata, &pframe[16], 6);
				else
					rtw_secmicappend(&micdata, &pframe[10], 6);

			}

			/* if(pqospriv->qos_option==1) */
			if (pattrib->qos_en)
				priority[0] = (uint8_t)pxmitframe->attrib.priority;


			rtw_secmicappend(&micdata, &priority[0], 4);

			payload = pframe;

			for (curfragnum = 0; curfragnum < pattrib->nr_frags; curfragnum++) {
				payload = (uint8_t *)RND4((SIZE_PTR)(payload));

				payload = payload+pattrib->hdrlen+pattrib->iv_len;
				if ((curfragnum+1) == pattrib->nr_frags) {
					length = pattrib->last_txcmdsz-pattrib->hdrlen-pattrib->iv_len-((pattrib->bswenc) ? pattrib->icv_len : 0);
					rtw_secmicappend(&micdata, payload, length);
					payload = payload + length;
				} else {
					length = pxmitpriv->frag_len-pattrib->hdrlen-pattrib->iv_len-((pattrib->bswenc) ? pattrib->icv_len : 0);
					rtw_secmicappend(&micdata, payload, length);
					payload = payload + length+pattrib->icv_len;
				}
			}
			rtw_secgetmic(&micdata, &(mic[0]));

			memcpy(payload, &(mic[0]), 8);
			pattrib->last_txcmdsz += 8;

			payload = payload-pattrib->last_txcmdsz + 8;
			for (curfragnum = 0; curfragnum < pattrib->last_txcmdsz; curfragnum = curfragnum + 8)
				;
			}
/*
			else{
				RT_TRACE(_module_rtl871x_xmit_c_,_drv_err_,("xmitframe_addmic: rtw_get_stainfo==NULL!!!\n"));
			}
*/
	}



	return _SUCCESS;
}

static int32_t xmitframe_swencrypt(struct rtl_priv *rtlpriv, struct xmit_frame *pxmitframe)
{
	struct	pkt_attrib	 *pattrib = &pxmitframe->attrib;
	/* struct 	security_priv	*psecuritypriv=&rtlpriv->securitypriv; */

	/* if((psecuritypriv->sw_encrypt)||(pattrib->bswenc)) */
	if (pattrib->bswenc) {
		/* DBG_871X("start xmitframe_swencrypt\n"); */
		switch (pattrib->encrypt) {
		case _WEP40_:
		case _WEP104_:
			rtw_wep_encrypt(rtlpriv, (uint8_t *)pxmitframe);
			break;
		case _TKIP_:
			rtw_tkip_encrypt(rtlpriv, (uint8_t *)pxmitframe);
			break;
		case _AES_:
			rtw_aes_encrypt(rtlpriv, (uint8_t *)pxmitframe);
			break;
		default:
				break;
		}

	} else {
		;
	}

	return _SUCCESS;
}

int32_t rtw_make_wlanhdr (struct rtl_priv *rtlpriv , uint8_t *hdr, struct pkt_attrib *pattrib)
{
	u16 *qc;

	struct rtw_ieee80211_hdr *pwlanhdr = (struct rtw_ieee80211_hdr *)hdr;
	struct mlme_priv *pmlmepriv = &rtlpriv->mlmepriv;
	struct qos_priv *pqospriv = &pmlmepriv->qospriv;
	uint8_t qos_option = _FALSE;

	sint res = _SUCCESS;
	u16 *fctrl = &pwlanhdr->frame_ctl;

	/* struct sta_info *psta; */

	/* sint bmcst = IS_MCAST(pattrib->ra); */



/*
	psta = rtw_get_stainfo(&rtlpriv->stapriv, pattrib->ra);
	if(pattrib->psta != psta)
	{
		DBG_871X("%s, pattrib->psta(%p) != psta(%p)\n", __func__, pattrib->psta, psta);
		return;
	}

	if(psta==NULL)
	{
		DBG_871X("%s, psta==NUL\n", __func__);
		return _FAIL;
	}

	if(!(psta->state &_FW_LINKED))
	{
		DBG_871X("%s, psta->state(0x%x) != _FW_LINKED\n", __func__, psta->state);
		return _FAIL;
	}
*/

	memset(hdr, 0, WLANHDR_OFFSET);

	SetFrameSubType(fctrl, pattrib->subtype);

	if (pattrib->subtype & WIFI_DATA_TYPE) {
		if ((check_fwstate(pmlmepriv,  WIFI_STATION_STATE) == _TRUE)) {
			/* to_ds = 1, fr_ds = 0; */
			{
				/* Data transfer to AP */
				SetToDs(fctrl);
				memcpy(pwlanhdr->addr1, get_bssid(pmlmepriv), ETH_ALEN);
				memcpy(pwlanhdr->addr2, pattrib->src, ETH_ALEN);
				memcpy(pwlanhdr->addr3, pattrib->dst, ETH_ALEN);
			}

			if (pqospriv->qos_option)
				qos_option = _TRUE;

		} else if ((check_fwstate(pmlmepriv,  WIFI_AP_STATE) == _TRUE)) {
			/* to_ds = 0, fr_ds = 1; */
			SetFrDs(fctrl);
			memcpy(pwlanhdr->addr1, pattrib->dst, ETH_ALEN);
			memcpy(pwlanhdr->addr2, get_bssid(pmlmepriv), ETH_ALEN);
			memcpy(pwlanhdr->addr3, pattrib->src, ETH_ALEN);

			if (pattrib->qos_en)
				qos_option = _TRUE;
		} else if ((check_fwstate(pmlmepriv, WIFI_ADHOC_STATE) == _TRUE)
		    || (check_fwstate(pmlmepriv, WIFI_ADHOC_MASTER_STATE) == _TRUE)) {
			memcpy(pwlanhdr->addr1, pattrib->dst, ETH_ALEN);
			memcpy(pwlanhdr->addr2, pattrib->src, ETH_ALEN);
			memcpy(pwlanhdr->addr3, get_bssid(pmlmepriv), ETH_ALEN);

			if (pattrib->qos_en)
				qos_option = _TRUE;
		} else {
			res = _FAIL;
			goto exit;
		}

		if (pattrib->mdata)
			SetMData(fctrl);

		if (pattrib->encrypt)
			SetPrivacy(fctrl);

		if (qos_option) {
			qc = (unsigned short *)(hdr + pattrib->hdrlen - 2);

			if (pattrib->priority)
				SetPriority(qc, pattrib->priority);

			SetEOSP(qc, pattrib->eosp);

			SetAckpolicy(qc, pattrib->ack_policy);
		}

		/* TODO: fill HT Control Field */

		/* Update Seq Num will be handled by f/w */
		{
			struct sta_info *psta;
			psta = rtw_get_stainfo(&rtlpriv->stapriv, pattrib->ra);
			if (pattrib->psta != psta) {
				DBG_871X("%s, pattrib->psta(%p) != psta(%p)\n", __func__, pattrib->psta, psta);
				return _FAIL;
			}

			if (psta == NULL) {
				DBG_871X("%s, psta==NUL\n", __func__);
				return _FAIL;
			}

			if (!(psta->state & _FW_LINKED)) {
				DBG_871X("%s, psta->state(0x%x) != _FW_LINKED\n", __func__, psta->state);
				return _FAIL;
			}

			/* if(psta) */
			{

				psta->sta_xmitpriv.txseq_tid[pattrib->priority]++;
				psta->sta_xmitpriv.txseq_tid[pattrib->priority] &= 0xFFF;

				pattrib->seqnum = psta->sta_xmitpriv.txseq_tid[pattrib->priority];

				SetSeqNum(hdr, pattrib->seqnum);

#ifdef CONFIG_80211N_HT
				/* check if enable ampdu */
				if (pattrib->ht_en && psta->htpriv.ampdu_enable) {
					if (psta->htpriv.agg_enable_bitmap & BIT(pattrib->priority))
					pattrib->ampdu_en = _TRUE;
				}

				/* re-check if enable ampdu by BA_starting_seqctrl */
				if (pattrib->ampdu_en == _TRUE) 	{
					u16 tx_seq;

					tx_seq = psta->BA_starting_seqctrl[pattrib->priority & 0x0f];

					/* check BA_starting_seqctrl */
					if (SN_LESS(pattrib->seqnum, tx_seq)) {
						/* DBG_871X("tx ampdu seqnum(%d) < tx_seq(%d)\n", pattrib->seqnum, tx_seq); */
						pattrib->ampdu_en = _FALSE;	/* AGG BK */
					} else if (SN_EQUAL(pattrib->seqnum, tx_seq)) {
						psta->BA_starting_seqctrl[pattrib->priority & 0x0f] = (tx_seq+1)&0xfff;

						pattrib->ampdu_en = _TRUE;	/* AGG EN */
					} else 	{
						/* DBG_871X("tx ampdu over run\n"); */
						psta->BA_starting_seqctrl[pattrib->priority & 0x0f] = (pattrib->seqnum+1)&0xfff;
						pattrib->ampdu_en = _TRUE;	/* AGG EN */
					}

				}
#endif

			}
		}

	} else {

	}

exit:

	return res;
}

int32_t rtw_txframes_pending(struct rtl_priv *rtlpriv)
{
	struct xmit_priv *pxmitpriv = &rtlpriv->xmitpriv;

	return ((_rtw_queue_empty(&pxmitpriv->be_pending) == _FALSE) ||
			 (_rtw_queue_empty(&pxmitpriv->bk_pending) == _FALSE) ||
			 (_rtw_queue_empty(&pxmitpriv->vi_pending) == _FALSE) ||
			 (_rtw_queue_empty(&pxmitpriv->vo_pending) == _FALSE));
}

int32_t rtw_txframes_sta_ac_pending(struct rtl_priv *rtlpriv, struct pkt_attrib *pattrib)
{
	struct sta_info *psta;
	struct tx_servq *ptxservq;
	int priority = pattrib->priority;
/*
	if(pattrib->psta)
	{
		psta = pattrib->psta;
	}
	else
	{
		DBG_871X("%s, call rtw_get_stainfo()\n", __func__);
		psta=rtw_get_stainfo(&rtlpriv->stapriv ,&pattrib->ra[0]);
	}
*/
	psta = rtw_get_stainfo(&rtlpriv->stapriv, pattrib->ra);
	if (pattrib->psta != psta) {
		DBG_871X("%s, pattrib->psta(%p) != psta(%p)\n", __func__, pattrib->psta, psta);
		return 0;
	}

	if (psta == NULL) {
		DBG_871X("%s, psta==NUL\n", __func__);
		return 0;
	}

	if (!(psta->state & _FW_LINKED)) {
		DBG_871X("%s, psta->state(0x%x) != _FW_LINKED\n", __func__, psta->state);
		return 0;
	}

	switch (priority) {
	case 1:
	case 2:
		ptxservq = &(psta->sta_xmitpriv.bk_q);
		break;
	case 4:
	case 5:
		ptxservq = &(psta->sta_xmitpriv.vi_q);
		break;
	case 6:
	case 7:
		ptxservq = &(psta->sta_xmitpriv.vo_q);
		break;
	case 0:
	case 3:
	default:
		ptxservq = &(psta->sta_xmitpriv.be_q);
	break;

	}

	return ptxservq->qcnt;
}

/*
 * Calculate wlan 802.11 packet MAX size from pkt_attrib
 * This function doesn't consider fragment case
 */
uint32_t	 rtw_calculate_wlan_pkt_size_by_attribue(struct pkt_attrib *pattrib)
{
	uint32_t	len = 0;

	len = pattrib->hdrlen + pattrib->iv_len;	/* WLAN Header and IV */
	len += SNAP_SIZE + sizeof(u16);		/* LLC */
	len += pattrib->pktlen;
	if (pattrib->encrypt == _TKIP_)
		len += 8;	/* MIC */
	len += ((pattrib->bswenc) ? pattrib->icv_len : 0);	/* ICV */

	return len;
}

/*

This sub-routine will perform all the following:

1. remove 802.3 header.
2. create wlan_header, based on the info in pxmitframe
3. append sta's iv/ext-iv
4. append LLC
5. move frag chunk from pframe to pxmitframe->mem
6. apply sw-encrypt, if necessary.

*/
int32_t rtw_xmitframe_coalesce(struct rtl_priv *rtlpriv, struct sk_buff *pkt, struct xmit_frame *pxmitframe)
{
	struct pkt_file pktfile;

	int32_t frg_inx, frg_len, mpdu_len, llc_sz, mem_sz;

	SIZE_PTR addr;

	uint8_t *pframe, *mem_start;
	uint8_t hw_hdr_offset;

	/* struct sta_info		*psta; */
	/* struct sta_priv		*pstapriv = &rtlpriv->stapriv; */
	/* struct mlme_priv	*pmlmepriv = &rtlpriv->mlmepriv; */
	struct xmit_priv	*pxmitpriv = &rtlpriv->xmitpriv;

	struct pkt_attrib	*pattrib = &pxmitframe->attrib;

	uint8_t *pbuf_start;

	int32_t bmcst = IS_MCAST(pattrib->ra);
	int32_t res = _SUCCESS;



/*
	if (pattrib->psta)
	{
		psta = pattrib->psta;
	} else
	{
		DBG_871X("%s, call rtw_get_stainfo()\n", __func__);
		psta = rtw_get_stainfo(&rtlpriv->stapriv, pattrib->ra);
	}

	if(psta==NULL)
	{

		DBG_871X("%s, psta==NUL\n", __func__);
		return _FAIL;
	}


	if(!(psta->state &_FW_LINKED))
	{
		DBG_871X("%s, psta->state(0x%x) != _FW_LINKED\n", __func__, psta->state);
		return _FAIL;
	}
*/
	if (pxmitframe->buf_addr == NULL) {
		DBG_8192C("==> %s buf_addr==NULL \n", __FUNCTION__);
		return _FAIL;
	}

	pbuf_start = pxmitframe->buf_addr;

#ifdef CONFIG_USB_TX_AGGREGATION
	hw_hdr_offset =  TXDESC_SIZE + (pxmitframe->pkt_offset * PACKET_OFFSET_SZ);
#else
	hw_hdr_offset = TXDESC_OFFSET;
#endif

	mem_start = pbuf_start +	hw_hdr_offset;

	if (rtw_make_wlanhdr(rtlpriv, mem_start, pattrib) == _FAIL) {
		DBG_8192C("rtw_xmitframe_coalesce: rtw_make_wlanhdr fail; drop pkt\n");
		res = _FAIL;
		goto exit;
	}

	_rtw_open_pktfile(pkt, &pktfile);
	_rtw_pktfile_read(&pktfile, NULL, pattrib->pkt_hdrlen);

	frg_inx = 0;
	frg_len = pxmitpriv->frag_len - 4;	/* 2346-4 = 2342 */

	while (1) {
		llc_sz = 0;

		mpdu_len = frg_len;

		pframe = mem_start;

		SetMFrag(mem_start);

		pframe += pattrib->hdrlen;
		mpdu_len -= pattrib->hdrlen;

		/* adding icv, if necessary... */
		if (pattrib->iv_len) {
/*
			//if (check_fwstate(pmlmepriv, WIFI_MP_STATE))
			//	psta = rtw_get_stainfo(pstapriv, get_bssid(pmlmepriv));
			//else
			//	psta = rtw_get_stainfo(pstapriv, pattrib->ra);

			if (psta != NULL)
			{
				switch(pattrib->encrypt)
				{
					case _WEP40_:
					case _WEP104_:
							WEP_IV(pattrib->iv, psta->dot11txpn, pattrib->key_idx);
						break;
					case _TKIP_:
						if(bmcst)
							TKIP_IV(pattrib->iv, psta->dot11txpn, pattrib->key_idx);
						else
							TKIP_IV(pattrib->iv, psta->dot11txpn, 0);
						break;
					case _AES_:
						if(bmcst)
							AES_IV(pattrib->iv, psta->dot11txpn, pattrib->key_idx);
						else
							AES_IV(pattrib->iv, psta->dot11txpn, 0);
						break;
				}
			}
*/
			memcpy(pframe, pattrib->iv, pattrib->iv_len);

			pframe += pattrib->iv_len;

			mpdu_len -= pattrib->iv_len;
		}

		if (frg_inx == 0) {
			llc_sz = rtw_put_snap(pframe, pattrib->ether_type);
			pframe += llc_sz;
			mpdu_len -= llc_sz;
		}

		if ((pattrib->icv_len > 0) && (pattrib->bswenc)) {
			mpdu_len -= pattrib->icv_len;
		}


		if (bmcst) {
			/* don't do fragment to broadcat/multicast packets */
			mem_sz = _rtw_pktfile_read(&pktfile, pframe, pattrib->pktlen);
		} else {
			mem_sz = _rtw_pktfile_read(&pktfile, pframe, mpdu_len);
		}

		pframe += mem_sz;

		if ((pattrib->icv_len > 0) && (pattrib->bswenc)) {
			memcpy(pframe, pattrib->icv, pattrib->icv_len);
			pframe += pattrib->icv_len;
		}

		frg_inx++;

		if (bmcst || (rtw_endofpktfile(&pktfile) == _TRUE)) {
			pattrib->nr_frags = frg_inx;

			pattrib->last_txcmdsz = pattrib->hdrlen + pattrib->iv_len + ((pattrib->nr_frags == 1) ? llc_sz : 0) +
					((pattrib->bswenc) ? pattrib->icv_len : 0) + mem_sz;

			ClearMFrag(mem_start);

			break;
		} else {
			;
		}

		addr = (SIZE_PTR)(pframe);

		mem_start = (unsigned char *)RND4(addr) + hw_hdr_offset;
		memcpy(mem_start, pbuf_start + hw_hdr_offset, pattrib->hdrlen);

	}

	if (xmitframe_addmic(rtlpriv, pxmitframe) == _FAIL) {
		DBG_8192C("xmitframe_addmic(rtlpriv, pxmitframe)==_FAIL\n");
		res = _FAIL;
		goto exit;
	}

	xmitframe_swencrypt(rtlpriv, pxmitframe);

	if (bmcst == _FALSE)
		update_attrib_vcs_info(rtlpriv, pxmitframe);
	else
		pattrib->vcs_mode = NONE_VCS;

exit:

	return res;
}

/* Logical Link Control(LLC) SubNetwork Attachment Point(SNAP) header
 * IEEE LLC/SNAP header contains 8 octets
 * First 3 octets comprise the LLC portion
 * SNAP portion, 5 octets, is divided into two fields:
 *	Organizationally Unique Identifier(OUI), 3 octets,
 *	type, defined by that organization, 2 octets.
 */
int32_t rtw_put_snap(uint8_t *data, u16 h_proto)
{
	struct ieee80211_snap_hdr *snap;
	uint8_t *oui;

	snap = (struct ieee80211_snap_hdr *)data;
	snap->dsap = 0xaa;
	snap->ssap = 0xaa;
	snap->ctrl = 0x03;

	if (h_proto == 0x8137 || h_proto == 0x80f3)
		oui = P802_1H_OUI;
	else
		oui = RFC1042_OUI;

	snap->oui[0] = oui[0];
	snap->oui[1] = oui[1];
	snap->oui[2] = oui[2];

	*(u16 *)(data + SNAP_SIZE) = htons(h_proto);

	return SNAP_SIZE + sizeof(u16);
}

void rtw_update_protection(struct rtl_priv *rtlpriv, uint8_t *ie, uint ie_len)
{

	uint	protection;
	uint8_t	*perp;
	sint	 erp_len;
	struct	xmit_priv *pxmitpriv = &rtlpriv->xmitpriv;
	struct	registry_priv *pregistrypriv = &rtlpriv->registrypriv;

	switch (pxmitpriv->vcs_setting) {
	case DISABLE_VCS:
		pxmitpriv->vcs = NONE_VCS;
		break;

	case ENABLE_VCS:
		break;

	case AUTO_VCS:
	default:
		perp = rtw_get_ie(ie, _ERPINFO_IE_, &erp_len, ie_len);
		if (perp == NULL) {
			pxmitpriv->vcs = NONE_VCS;
		} else {
			protection = (*(perp + 2)) & BIT(1);
			if (protection) {
				if (pregistrypriv->vcs_type == RTS_CTS)
					pxmitpriv->vcs = RTS_CTS;
				else
					pxmitpriv->vcs = CTS_TO_SELF;
			} else
				pxmitpriv->vcs = NONE_VCS;
		}

		break;

	}

}

void rtw_count_tx_stats(struct rtl_priv *rtlpriv, struct xmit_frame *pxmitframe, int sz)
{
	struct sta_info *psta = NULL;
	struct stainfo_stats *pstats = NULL;
	struct xmit_priv	*pxmitpriv = &rtlpriv->xmitpriv;
	struct mlme_priv	*pmlmepriv = &rtlpriv->mlmepriv;

	if ((pxmitframe->frame_tag&0x0f) == DATA_FRAMETAG) {
		pxmitpriv->tx_bytes += sz;
#if defined(CONFIG_USB_TX_AGGREGATION)
		pmlmepriv->LinkDetectInfo.NumTxOkInPeriod += pxmitframe->agg_num;
#else
		pmlmepriv->LinkDetectInfo.NumTxOkInPeriod++;
#endif

		psta = pxmitframe->attrib.psta;
		if (psta) {
			pstats = &psta->sta_stats;
#if defined(CONFIG_USB_TX_AGGREGATION)
			pstats->tx_pkts += pxmitframe->agg_num;
#else
			pstats->tx_pkts++;
#endif
			pstats->tx_bytes += sz;
		}
	}
}

struct xmit_buf *rtw_alloc_cmd_xmitbuf(struct xmit_priv *pxmitpriv, uint32_t	 buffsize)
{
	struct xmit_buf *pxmitbuf =  NULL;

	pxmitbuf = &pxmitpriv->pcmd_xmitbuf;

	if (pxmitbuf !=  NULL) {
		if (rtw_os_xmit_resource_alloc(pxmitpriv->rtlpriv, pxmitbuf, (buffsize + XMITBUF_ALIGN_SZ), _FALSE) == _FAIL) {
			return NULL;
		}

		pxmitbuf->alloc_sz = buffsize + XMITBUF_ALIGN_SZ;

		pxmitbuf->priv_data = NULL;


		if (pxmitbuf->sctx) {
			DBG_871X("%s pxmitbuf->sctx is not NULL\n", __func__);
			rtw_sctx_done_err(&pxmitbuf->sctx, RTW_SCTX_DONE_BUF_ALLOC);
		}
	} else {
		DBG_871X("%s fail, no xmitbuf available !!!\n", __func__);
	}

	return pxmitbuf;
}

int32_t	rtw_free_cmd_xmitbuf(struct xmit_priv *pxmitpriv)
{
	struct xmit_buf *pxmitbuf =  NULL;

	pxmitbuf = &pxmitpriv->pcmd_xmitbuf;
	if (pxmitbuf == NULL) {
		DBG_871X("%s fail, no xmitbuf available !!!\n", __func__);
		return _FAIL;
	} else {
		rtw_os_xmit_resource_free(pxmitbuf->rtlpriv, pxmitbuf, pxmitbuf->alloc_sz, _FALSE);
	}

	return _SUCCESS;
}

struct xmit_frame *rtw_alloc_cmdxmitframe(struct xmit_priv *pxmitpriv, uint32_t	 buffsize)
{
	struct xmit_frame		*pcmdframe;
	struct xmit_buf		*pxmitbuf;

	pcmdframe = rtw_alloc_xmitframe(pxmitpriv);
	if (pcmdframe == NULL) {
		DBG_871X("%s, alloc xmitframe fail\n", __FUNCTION__);
		return NULL;
	}

	pxmitbuf = rtw_alloc_cmd_xmitbuf(pxmitpriv, buffsize);
	if (pxmitbuf == NULL) {
		DBG_871X("%s, alloc xmitbuf fail\n", __FUNCTION__);
		rtw_free_xmitframe(pxmitpriv, pcmdframe);
		return NULL;
	}

	pcmdframe->frame_tag = MGNT_FRAMETAG;

	pcmdframe->pxmitbuf = pxmitbuf;

	pcmdframe->buf_addr = pxmitbuf->pbuf;

	pxmitbuf->priv_data = pcmdframe;

	return pcmdframe;

}

void	rtw_free_cmdxmitframe(struct xmit_priv *pxmitpriv, struct xmit_frame *pxmitframe)
{
	rtw_free_xmitframe(pxmitpriv, pxmitframe);
	rtw_free_cmd_xmitbuf(pxmitpriv);
}

struct xmit_buf *rtw_alloc_xmitbuf_ext(struct xmit_priv *pxmitpriv)
{
	unsigned long flags;
	struct xmit_buf *pxmitbuf =  NULL;
	struct list_head *plist, *phead;
	struct __queue *pfree_queue = &pxmitpriv->free_xmit_extbuf_queue;

	spin_lock_irqsave(&pfree_queue->lock, flags);

	if (_rtw_queue_empty(pfree_queue) == _TRUE) {
		pxmitbuf = NULL;
	} else {

		phead = get_list_head(pfree_queue);

		plist = get_next(phead);

		pxmitbuf = LIST_CONTAINOR(plist, struct xmit_buf, list);

		rtw_list_delete(&(pxmitbuf->list));
	}

	if (pxmitbuf !=  NULL) {
		pxmitpriv->free_xmit_extbuf_cnt--;


		pxmitbuf->priv_data = NULL;


		if (pxmitbuf->sctx) {
			DBG_871X("%s pxmitbuf->sctx is not NULL\n", __func__);
			rtw_sctx_done_err(&pxmitbuf->sctx, RTW_SCTX_DONE_BUF_ALLOC);
		}

	}

	spin_unlock_irqrestore(&pfree_queue->lock, flags);

	return pxmitbuf;
}

int32_t rtw_free_xmitbuf_ext(struct xmit_priv *pxmitpriv, struct xmit_buf *pxmitbuf)
{
	unsigned long flags;
	struct __queue *pfree_queue = &pxmitpriv->free_xmit_extbuf_queue;

	if (pxmitbuf == NULL) {
		return _FAIL;
	}

	spin_lock_irqsave(&pfree_queue->lock, flags);

	rtw_list_delete(&pxmitbuf->list);

	list_add_tail(&(pxmitbuf->list), get_list_head(pfree_queue));
	pxmitpriv->free_xmit_extbuf_cnt++;

	spin_unlock_irqrestore(&pfree_queue->lock, flags);

	return _SUCCESS;
}

struct xmit_buf *rtw_alloc_xmitbuf(struct xmit_priv *pxmitpriv)
{
	unsigned long flags;
	struct xmit_buf *pxmitbuf =  NULL;
	struct list_head *plist, *phead;
	struct __queue *pfree_xmitbuf_queue = &pxmitpriv->free_xmitbuf_queue;

	/* DBG_871X("+rtw_alloc_xmitbuf\n"); */

	spin_lock_irqsave(&pfree_xmitbuf_queue->lock, flags);

	if (_rtw_queue_empty(pfree_xmitbuf_queue) == _TRUE) {
		pxmitbuf = NULL;
	} else {

		phead = get_list_head(pfree_xmitbuf_queue);

		plist = get_next(phead);

		pxmitbuf = LIST_CONTAINOR(plist, struct xmit_buf, list);

		rtw_list_delete(&(pxmitbuf->list));
	}

	if (pxmitbuf !=  NULL) {
		pxmitpriv->free_xmitbuf_cnt--;
		/* DBG_871X("alloc, free_xmitbuf_cnt=%d\n", pxmitpriv->free_xmitbuf_cnt); */

		pxmitbuf->priv_data = NULL;


		if (pxmitbuf->sctx) {
			DBG_871X("%s pxmitbuf->sctx is not NULL\n", __func__);
			rtw_sctx_done_err(&pxmitbuf->sctx, RTW_SCTX_DONE_BUF_ALLOC);
		}
	}

	spin_unlock_irqrestore(&pfree_xmitbuf_queue->lock, flags);

	return pxmitbuf;
}

int32_t rtw_free_xmitbuf(struct xmit_priv *pxmitpriv, struct xmit_buf *pxmitbuf)
{
	unsigned long flags;
	struct __queue *pfree_xmitbuf_queue = &pxmitpriv->free_xmitbuf_queue;

	/* DBG_871X("+rtw_free_xmitbuf\n"); */

	if (pxmitbuf == NULL) {
		return _FAIL;
	}

	if (pxmitbuf->sctx) {
		DBG_871X("%s pxmitbuf->sctx is not NULL\n", __func__);
		rtw_sctx_done_err(&pxmitbuf->sctx, RTW_SCTX_DONE_BUF_FREE);
	}

	if (pxmitbuf->buf_tag == XMITBUF_CMD) {
		;
	} else if (pxmitbuf->buf_tag == XMITBUF_MGNT) {
		rtw_free_xmitbuf_ext(pxmitpriv, pxmitbuf);
	} else {
		spin_lock_irqsave(&pfree_xmitbuf_queue->lock, flags);

		rtw_list_delete(&pxmitbuf->list);

		list_add_tail(&(pxmitbuf->list), get_list_head(pfree_xmitbuf_queue));

		pxmitpriv->free_xmitbuf_cnt++;
		/* DBG_871X("FREE, free_xmitbuf_cnt=%d\n", pxmitpriv->free_xmitbuf_cnt); */
		spin_unlock_irqrestore(&pfree_xmitbuf_queue->lock, flags);
	}

	return _SUCCESS;
}

void rtw_init_xmitframe(struct xmit_frame *pxframe)
{
	if (pxframe !=  NULL) {		/* default value setting */
		pxframe->buf_addr = NULL;
		pxframe->pxmitbuf = NULL;

		memset(&pxframe->attrib, 0, sizeof(struct pkt_attrib));
		/* pxframe->attrib.psta = NULL; */

		pxframe->frame_tag = DATA_FRAMETAG;

		pxframe->pkt = NULL;
		pxframe->pkt_offset = 1;	/*default use pkt_offset to fill tx desc */

#ifdef CONFIG_USB_TX_AGGREGATION
		pxframe->agg_num = 1;
#endif
	}
}

/*
Calling context:
1. OS_TXENTRY
2. RXENTRY (rx_thread or RX_ISR/RX_CallBack)

If we turn on USE_RXTHREAD, then, no need for critical section.
Otherwise, we must use _enter/_exit critical to protect free_xmit_queue...

Must be very very cautious...

*/
struct xmit_frame *rtw_alloc_xmitframe(struct xmit_priv *pxmitpriv)	/* (struct __queue *pfree_xmit_queue) */
{
	/*
		Please remember to use all the osdep_service api,
		and lock/unlock or _enter/_exit critical to protect
		pfree_xmit_queue
	*/

	struct xmit_frame *pxframe = NULL;
	struct list_head *plist, *phead;
	struct __queue *pfree_xmit_queue = &pxmitpriv->free_xmit_queue;

	spin_lock_bh(&pfree_xmit_queue->lock);

	if (_rtw_queue_empty(pfree_xmit_queue) == _TRUE) {
		pxframe =  NULL;
	} else {
		phead = get_list_head(pfree_xmit_queue);

		plist = get_next(phead);

		pxframe = LIST_CONTAINOR(plist, struct xmit_frame, list);

		rtw_list_delete(&(pxframe->list));
		pxmitpriv->free_xmitframe_cnt--;
	}

	spin_unlock_bh(&pfree_xmit_queue->lock);

	rtw_init_xmitframe(pxframe);

	return pxframe;
}

struct xmit_frame *rtw_alloc_xmitframe_ext(struct xmit_priv *pxmitpriv)
{
	struct xmit_frame *pxframe = NULL;
	struct list_head *plist, *phead;
	struct __queue *queue = &pxmitpriv->free_xframe_ext_queue;

	spin_lock_bh(&queue->lock);

	if (_rtw_queue_empty(queue) == _TRUE) {
		pxframe =  NULL;
	} else {
		phead = get_list_head(queue);
		plist = get_next(phead);
		pxframe = LIST_CONTAINOR(plist, struct xmit_frame, list);

		rtw_list_delete(&(pxframe->list));
		pxmitpriv->free_xframe_ext_cnt--;
	}

	spin_unlock_bh(&queue->lock);

	rtw_init_xmitframe(pxframe);

	return pxframe;
}

struct xmit_frame *rtw_alloc_xmitframe_once(struct xmit_priv *pxmitpriv)
{
	struct xmit_frame *pxframe = NULL;
	uint8_t *alloc_addr;

	alloc_addr = rtw_zmalloc(sizeof(struct xmit_frame) + 4);

	if (alloc_addr == NULL)
		goto exit;

	pxframe = (struct xmit_frame *)N_BYTE_ALIGMENT((SIZE_PTR)(alloc_addr), 4);
	pxframe->alloc_addr = alloc_addr;

	pxframe->rtlpriv = pxmitpriv->rtlpriv;
	pxframe->frame_tag = NULL_FRAMETAG;

	pxframe->pkt = NULL;

	pxframe->buf_addr = NULL;
	pxframe->pxmitbuf = NULL;

	rtw_init_xmitframe(pxframe);

	DBG_871X("################## %s ##################\n", __func__);

exit:
	return pxframe;
}

int32_t rtw_free_xmitframe(struct xmit_priv *pxmitpriv, struct xmit_frame *pxmitframe)
{
	struct __queue *queue;
	struct rtl_priv *rtlpriv = pxmitpriv->rtlpriv;
	struct sk_buff *pndis_pkt = NULL;

	if (pxmitframe == NULL) {
		goto exit;
	}

	if (pxmitframe->pkt) {
		pndis_pkt = pxmitframe->pkt;
		pxmitframe->pkt = NULL;
	}

	if (pxmitframe->alloc_addr) {
		DBG_871X("################## %s with alloc_addr ##################\n", __func__);
		rtw_mfree(pxmitframe->alloc_addr);
		goto check_pkt_complete;
	}

	if (pxmitframe->ext_tag == 0)
		queue = &pxmitpriv->free_xmit_queue;
	else if (pxmitframe->ext_tag == 1)
		queue = &pxmitpriv->free_xframe_ext_queue;
	else {
	}

	spin_lock_bh(&queue->lock);

	rtw_list_delete(&pxmitframe->list);
	list_add_tail(&pxmitframe->list, get_list_head(queue));
	if (pxmitframe->ext_tag == 0) {
		pxmitpriv->free_xmitframe_cnt++;
	} else if (pxmitframe->ext_tag == 1) {
		pxmitpriv->free_xframe_ext_cnt++;
	} else {
		;
	}

	spin_unlock_bh(&queue->lock);

check_pkt_complete:

	if (pndis_pkt)
		rtw_os_pkt_complete(rtlpriv, pndis_pkt);

exit:

	return _SUCCESS;
}

void rtw_free_xmitframe_queue(struct xmit_priv *pxmitpriv, struct __queue *pframequeue)
{
	struct list_head	*plist, *phead;
	struct	xmit_frame 	*pxmitframe;

	spin_lock_bh(&(pframequeue->lock));

	phead = get_list_head(pframequeue);
	plist = get_next(phead);

	while (rtw_end_of_queue_search(phead, plist) == _FALSE) {

		pxmitframe = LIST_CONTAINOR(plist, struct xmit_frame, list);

		plist = get_next(plist);

		rtw_free_xmitframe(pxmitpriv, pxmitframe);

	}
	spin_unlock_bh(&(pframequeue->lock));

}


int32_t rtw_xmitframe_enqueue(struct rtl_priv *rtlpriv, struct xmit_frame *pxmitframe)
{
	if (rtw_xmit_classifier(rtlpriv, pxmitframe) == _FAIL) {
/*		pxmitframe->pkt = NULL; */
		return _FAIL;
	}

	return _SUCCESS;
}


static struct xmit_frame *dequeue_one_xmitframe(struct xmit_priv *pxmitpriv, struct hw_xmit *phwxmit, struct tx_servq *ptxservq, struct __queue *pframe_queue)
{
	struct list_head	*xmitframe_plist, *xmitframe_phead;
	struct	xmit_frame	*pxmitframe = NULL;

	xmitframe_phead = get_list_head(pframe_queue);
	xmitframe_plist = get_next(xmitframe_phead);

	while ((rtw_end_of_queue_search(xmitframe_phead, xmitframe_plist)) == _FALSE) {
		pxmitframe = LIST_CONTAINOR(xmitframe_plist, struct xmit_frame, list);

		xmitframe_plist = get_next(xmitframe_plist);

/*#ifdef RTK_DMP_PLATFORM
#ifdef CONFIG_USB_TX_AGGREGATION
		if((ptxservq->qcnt>0) && (ptxservq->qcnt<=2))
		{
			pxmitframe = NULL;

			tasklet_schedule(&pxmitpriv->xmit_tasklet);

			break;
		}
#endif
#endif*/
		rtw_list_delete(&pxmitframe->list);

		ptxservq->qcnt--;

		/* list_add_tail(&pxmitframe->list, &phwxmit->pending); */

		/* ptxservq->qcnt--; */

		break;

		/* ULLI : stupid break ?? */
		pxmitframe = NULL;

	}

	return pxmitframe;
}

struct xmit_frame *rtw_dequeue_xframe(struct xmit_priv *pxmitpriv, struct hw_xmit *phwxmit_i, sint entry)
{
	struct list_head *sta_plist, *sta_phead;
	struct hw_xmit *phwxmit;
	struct tx_servq *ptxservq = NULL;
	struct __queue *pframe_queue = NULL;
	struct xmit_frame *pxmitframe = NULL;
	struct rtl_priv *rtlpriv = pxmitpriv->rtlpriv;
	struct registry_priv	*pregpriv = &rtlpriv->registrypriv;
	int i, inx[4];

	inx[0] = 0; inx[1] = 1; inx[2] = 2; inx[3] = 3;

	spin_lock_bh(&pxmitpriv->lock);

	for (i = 0; i < entry; i++) {
		phwxmit = phwxmit_i + inx[i];

		/* spin_lock_irqsave(&phwxmit->sta_queue->lock, &irqL0); */

		sta_phead = get_list_head(phwxmit->sta_queue);
		sta_plist = get_next(sta_phead);

		while ((rtw_end_of_queue_search(sta_phead, sta_plist)) == _FALSE) {

			ptxservq = LIST_CONTAINOR(sta_plist, struct tx_servq, tx_pending);

			pframe_queue = &ptxservq->sta_pending;

			pxmitframe = dequeue_one_xmitframe(pxmitpriv, phwxmit, ptxservq, pframe_queue);

			if (pxmitframe) {
				phwxmit->accnt--;

				/* Remove sta node when there is no pending packets. */
				if (_rtw_queue_empty(pframe_queue))	/* must be done after get_next and before break */
					rtw_list_delete(&ptxservq->tx_pending);

				/* spin_unlock_irqrestore(&phwxmit->sta_queue->lock, &irqL0); */

				goto exit;
			}

			sta_plist = get_next(sta_plist);

		}

		/* spin_unlock_irqrestore(&phwxmit->sta_queue->lock, &irqL0); */

	}

exit:

	spin_unlock_bh(&pxmitpriv->lock);

	return pxmitframe;
}

#if 1
struct tx_servq *rtw_get_sta_pending(struct rtl_priv *rtlpriv, struct sta_info *psta, sint up, uint8_t *ac)
{
	struct tx_servq *ptxservq = NULL;

	switch (up) {
	case 1:
	case 2:
		ptxservq = &(psta->sta_xmitpriv.bk_q);
		*(ac) = 3;
		break;

	case 4:
	case 5:
		ptxservq = &(psta->sta_xmitpriv.vi_q);
		*(ac) = 1;
		break;

	case 6:
	case 7:
		ptxservq = &(psta->sta_xmitpriv.vo_q);
		*(ac) = 0;
		break;

	case 0:
	case 3:
	default:
		ptxservq = &(psta->sta_xmitpriv.be_q);
		*(ac) = 2;
	break;

	}

	return ptxservq;
}
#else

__inline static struct tx_servq *rtw_get_sta_pending
	(struct rtl_priv *rtlpriv, struct __queue **ppstapending, struct sta_info *psta, sint up)
{
	struct tx_servq *ptxservq;
	struct hw_xmit *phwxmits =  rtlpriv->xmitpriv.hwxmits;

#ifdef CONFIG_RTL8711

	if (IS_MCAST(psta->hwaddr)) {
		ptxservq = &(psta->sta_xmitpriv.be_q);	/* we will use be_q to queue bc/mc frames in BCMC_stainfo */
		*ppstapending = &rtlpriv->xmitpriv.bm_pending;
	} else
#endif
	{
		switch (up) {
		case 1:
		case 2:
			ptxservq = &(psta->sta_xmitpriv.bk_q);
			*ppstapending = &rtlpriv->xmitpriv.bk_pending;
			(phwxmits+3)->accnt++;
			break;

		case 4:
		case 5:
			ptxservq = &(psta->sta_xmitpriv.vi_q);
			*ppstapending = &rtlpriv->xmitpriv.vi_pending;
			(phwxmits+1)->accnt++;
			break;

		case 6:
		case 7:
			ptxservq = &(psta->sta_xmitpriv.vo_q);
			*ppstapending = &rtlpriv->xmitpriv.vo_pending;
			(phwxmits+0)->accnt++;
			break;

		case 0:
		case 3:
		default:
			ptxservq = &(psta->sta_xmitpriv.be_q);
			*ppstapending = &rtlpriv->xmitpriv.be_pending;
			(phwxmits+2)->accnt++;
		break;
		}

	}

	return ptxservq;
}
#endif

/*
 * Will enqueue pxmitframe to the proper queue,
 * and indicate it to xx_pending list.....
 */
int32_t rtw_xmit_classifier(struct rtl_priv *rtlpriv, struct xmit_frame *pxmitframe)
{
	/* _irqL irqL0; */
	uint8_t	ac_index;
	struct sta_info	*psta;
	struct tx_servq	*ptxservq;
	struct pkt_attrib	*pattrib = &pxmitframe->attrib;
	struct sta_priv	*pstapriv = &rtlpriv->stapriv;
	struct hw_xmit	*phwxmits =  rtlpriv->xmitpriv.hwxmits;
	sint res = _SUCCESS;

/*
	if (pattrib->psta) {
		psta = pattrib->psta;
	} else {
		DBG_871X("%s, call rtw_get_stainfo()\n", __func__);
		psta = rtw_get_stainfo(pstapriv, pattrib->ra);
	}
*/

	psta = rtw_get_stainfo(&rtlpriv->stapriv, pattrib->ra);
	if (pattrib->psta != psta) {
		DBG_871X("%s, pattrib->psta(%p) != psta(%p)\n", __func__, pattrib->psta, psta);
		return _FAIL;
	}

	if (psta == NULL) {
		res = _FAIL;
		DBG_8192C("rtw_xmit_classifier: psta == NULL\n");
		goto exit;
	}

	if (!(psta->state & _FW_LINKED)) {
		DBG_871X("%s, psta->state(0x%x) != _FW_LINKED\n", __func__, psta->state);
		return _FAIL;
	}

	ptxservq = rtw_get_sta_pending(rtlpriv, psta, pattrib->priority, (uint8_t *)(&ac_index));

	/* spin_lock_irqsave(&pstapending->lock, &irqL0); */

	if (list_empty(&ptxservq->tx_pending)) {
		list_add_tail(&ptxservq->tx_pending, get_list_head(phwxmits[ac_index].sta_queue));
	}

	/* spin_lock_irqsave(&ptxservq->sta_pending.lock, &irqL1); */

	list_add_tail(&pxmitframe->list, get_list_head(&ptxservq->sta_pending));
	ptxservq->qcnt++;
	phwxmits[ac_index].accnt++;

	/* spin_unlock_irqrestore(&ptxservq->sta_pending.lock, &irqL1); */

	/* spin_unlock_irqrestore(&pstapending->lock, &irqL0); */

exit:

	return res;
}

void rtw_alloc_hwxmits(struct rtl_priv *rtlpriv)
{
	struct hw_xmit *hwxmits;
	struct xmit_priv *pxmitpriv = &rtlpriv->xmitpriv;

	pxmitpriv->hwxmit_entry = HWXMIT_ENTRY;

	pxmitpriv->hwxmits = (struct hw_xmit *)rtw_zmalloc(sizeof (struct hw_xmit) * pxmitpriv->hwxmit_entry);

	hwxmits = pxmitpriv->hwxmits;

	if (pxmitpriv->hwxmit_entry == 5) {
		/* pxmitpriv->bmc_txqueue.head = 0; */
		/* hwxmits[0] .phwtxqueue = &pxmitpriv->bmc_txqueue; */
		hwxmits[0].sta_queue = &pxmitpriv->bm_pending;

		/* pxmitpriv->vo_txqueue.head = 0; */
		/* hwxmits[1] .phwtxqueue = &pxmitpriv->vo_txqueue; */
		hwxmits[1].sta_queue = &pxmitpriv->vo_pending;

		/* pxmitpriv->vi_txqueue.head = 0; */
		/* hwxmits[2] .phwtxqueue = &pxmitpriv->vi_txqueue; */
		hwxmits[2].sta_queue = &pxmitpriv->vi_pending;

		/* pxmitpriv->bk_txqueue.head = 0; */
		/* hwxmits[3] .phwtxqueue = &pxmitpriv->bk_txqueue; */
		hwxmits[3].sta_queue = &pxmitpriv->bk_pending;

		/* pxmitpriv->be_txqueue.head = 0; */
		/* hwxmits[4] .phwtxqueue = &pxmitpriv->be_txqueue; */
		hwxmits[4].sta_queue = &pxmitpriv->be_pending;

	} else if (pxmitpriv->hwxmit_entry == 4) {
		/* pxmitpriv->vo_txqueue.head = 0; */
		/* hwxmits[0] .phwtxqueue = &pxmitpriv->vo_txqueue; */
		hwxmits[0].sta_queue = &pxmitpriv->vo_pending;

		/* pxmitpriv->vi_txqueue.head = 0; */
		/* hwxmits[1] .phwtxqueue = &pxmitpriv->vi_txqueue; */
		hwxmits[1].sta_queue = &pxmitpriv->vi_pending;

		/* pxmitpriv->be_txqueue.head = 0; */
		/* hwxmits[2] .phwtxqueue = &pxmitpriv->be_txqueue; */
		hwxmits[2].sta_queue = &pxmitpriv->be_pending;

		/* pxmitpriv->bk_txqueue.head = 0; */
		/* hwxmits[3] .phwtxqueue = &pxmitpriv->bk_txqueue; */
		hwxmits[3].sta_queue = &pxmitpriv->bk_pending;
	} else {
		;
	}
}

void rtw_free_hwxmits(struct rtl_priv *rtlpriv)
{
	struct hw_xmit *hwxmits;
	struct xmit_priv *pxmitpriv = &rtlpriv->xmitpriv;

	hwxmits = pxmitpriv->hwxmits;
	if (hwxmits)
		rtw_mfree(hwxmits);
}

void rtw_init_hwxmits(struct hw_xmit *phwxmit, sint entry)
{
	sint i;
	for (i = 0; i < entry; i++, phwxmit++) {
		/*
		 * spin_lock_init(&phwxmit->xmit_lock);
		 * INIT_LIST_HEAD(&phwxmit->pending);
		 * phwxmit->txcmdcnt = 0;
		 */
		phwxmit->accnt = 0;
	}
}

uint32_t rtw_get_ff_hwaddr(struct xmit_frame *pxmitframe)
{
	uint32_t	 addr;
	struct pkt_attrib *pattrib = &pxmitframe->attrib;

	switch (pattrib->qsel) {
	case 0:
	case 3:
		addr = BE_QUEUE_INX;
		break;
	case 1:
	case 2:
		addr = BK_QUEUE_INX;
		break;
	case 4:
	case 5:
		addr = VI_QUEUE_INX;
		break;
	case 6:
	case 7:
		addr = VO_QUEUE_INX;
		break;
	case 0x10:
		addr = BCN_QUEUE_INX;
		break;
	case 0x11:	/* BC/MC in PS (HIQ) */
		addr = HIGH_QUEUE_INX;
		break;
	case 0x12:
	default:
		addr = MGT_QUEUE_INX;
		break;

	}

	return addr;

}

static void do_queue_select(struct rtl_priv *rtlpriv, struct pkt_attrib *pattrib)
{
	uint8_t qsel;

	qsel = pattrib->priority;

	pattrib->qsel = qsel;
}

/*
 * The main transmit(tx) entry
 *
 * Return
 *	1	enqueue
 *	0	success, hardware will handle this xmit frame(packet)
 *	<0	fail
 */
int32_t rtw_xmit(struct rtl_priv *rtlpriv, struct sk_buff **ppkt)
{
	static uint32_t	 start = 0;
	static uint32_t	 drop_cnt = 0;
#ifdef CONFIG_AP_MODE
#endif
	struct xmit_priv *pxmitpriv = &rtlpriv->xmitpriv;
	struct xmit_frame *pxmitframe = NULL;

	int32_t res;

	if (start == 0)
		start = jiffies;

	pxmitframe = rtw_alloc_xmitframe(pxmitpriv);

	if (rtw_get_passing_time_ms(start) > 2000) {
		if (drop_cnt)
			DBG_871X("DBG_TX_DROP_FRAME %s no more pxmitframe, drop_cnt:%u\n", __FUNCTION__, drop_cnt);
		start = jiffies;
		drop_cnt = 0;
	}

	if (pxmitframe == NULL) {
		drop_cnt++;
		return -1;
	}

	res = update_attrib(rtlpriv, *ppkt, &pxmitframe->attrib);

	if (res == _FAIL) {
		rtw_free_xmitframe(pxmitpriv, pxmitframe);
		return -1;
	}
	pxmitframe->pkt = *ppkt;

	rtw_hal_led_control(rtlpriv, LED_CTL_TX);

	do_queue_select(rtlpriv, &pxmitframe->attrib);

#ifdef CONFIG_AP_MODE
	spin_lock_bh(&pxmitpriv->lock);
	if (xmitframe_enqueue_for_sleeping_sta(rtlpriv, pxmitframe) == _TRUE) {
		spin_unlock_bh(&pxmitpriv->lock);
		return 1;
	}
	spin_unlock_bh(&pxmitpriv->lock);
#endif

	if (rtw_hal_xmit(rtlpriv, pxmitframe) == _FALSE)
		return 1;

	return 0;
}


#if defined(CONFIG_AP_MODE)

sint xmitframe_enqueue_for_sleeping_sta(struct rtl_priv *rtlpriv, struct xmit_frame *pxmitframe)
{
	sint ret = _FALSE;
	struct sta_info *psta = NULL;
	struct sta_priv *pstapriv = &rtlpriv->stapriv;
	struct pkt_attrib *pattrib = &pxmitframe->attrib;
	struct mlme_priv *pmlmepriv = &rtlpriv->mlmepriv;
	sint bmcst = IS_MCAST(pattrib->ra);

	if (check_fwstate(pmlmepriv, WIFI_AP_STATE) == _FALSE)
	    return ret;
/*
	if(pattrib->psta)
	{
		psta = pattrib->psta;
	}
	else
	{
		DBG_871X("%s, call rtw_get_stainfo()\n", __func__);
		psta=rtw_get_stainfo(pstapriv, pattrib->ra);
	}
*/
	psta = rtw_get_stainfo(&rtlpriv->stapriv, pattrib->ra);
	if (pattrib->psta != psta) {
		DBG_871X("%s, pattrib->psta(%p) != psta(%p)\n", __func__, pattrib->psta, psta);
		return _FALSE;
	}

	if (psta == NULL) {
		DBG_871X("%s, psta==NUL\n", __func__);
		return _FALSE;
	}

	if (!(psta->state & _FW_LINKED)) {
		DBG_871X("%s, psta->state(0x%x) != _FW_LINKED\n", __func__, psta->state);
		return _FALSE;
	}

	if (pattrib->triggered == 1) {
		/* DBG_871X("directly xmit pspoll_triggered packet\n"); */

		/* pattrib->triggered=0; */

		if (bmcst)
			pattrib->qsel = 0x11;	/* HIQ */


		return ret;
	}


	if (bmcst) {
		spin_lock_bh(&psta->sleep_q.lock);

		if (pstapriv->sta_dz_bitmap) {
			/* if anyone sta is in ps mode */

			/* pattrib->qsel = 0x11;//HIQ */

			rtw_list_delete(&pxmitframe->list);

			/* spin_lock_bh(&psta->sleep_q.lock, &irqL); */

			list_add_tail(&pxmitframe->list, get_list_head(&psta->sleep_q));

			psta->sleepq_len++;

			pstapriv->tim_bitmap |= BIT(0);
			pstapriv->sta_dz_bitmap |= BIT(0);

			/* DBG_871X("enqueue, sq_len=%d, tim=%x\n", psta->sleepq_len, pstapriv->tim_bitmap); */

			update_beacon(rtlpriv, _TIM_IE_, NULL, _FALSE);	/* tx bc/mc packets after upate bcn */

			/* spin_unlock_bh(&psta->sleep_q.lock, &irqL); */

			ret = _TRUE;

		}

		spin_unlock_bh(&psta->sleep_q.lock);

		return ret;

	}


	spin_lock_bh(&psta->sleep_q.lock);

	if (psta->state & WIFI_SLEEP_STATE) {
		uint8_t wmmps_ac = 0;

		if (pstapriv->sta_dz_bitmap&BIT(psta->aid)) {
			rtw_list_delete(&pxmitframe->list);

			/*spin_lock_bh(&psta->sleep_q.lock, &irqL); */

			list_add_tail(&pxmitframe->list, get_list_head(&psta->sleep_q));

			psta->sleepq_len++;

			switch (pattrib->priority) {
			case 1:
			case 2:
				wmmps_ac = psta->uapsd_bk&BIT(0);
				break;
			case 4:
			case 5:
				wmmps_ac = psta->uapsd_vi&BIT(0);
				break;
			case 6:
			case 7:
				wmmps_ac = psta->uapsd_vo&BIT(0);
				break;
			case 0:
			case 3:
			default:
				wmmps_ac = psta->uapsd_be&BIT(0);
				break;
			}

			if (wmmps_ac)
				psta->sleepq_ac_len++;

			if (((psta->has_legacy_ac) && (!wmmps_ac)) || ((!psta->has_legacy_ac) && (wmmps_ac))) {
				pstapriv->tim_bitmap |= BIT(psta->aid);

				/* DBG_871X("enqueue, sq_len=%d, tim=%x\n", psta->sleepq_len, pstapriv->tim_bitmap); */

				if (psta->sleepq_len == 1) {
					/* DBG_871X("sleepq_len==1, update BCNTIM\n"); */
					/* upate BCN for TIM IE */
					update_beacon(rtlpriv, _TIM_IE_, NULL, _FALSE);
				}
			}

			/*spin_unlock_bh(&psta->sleep_q.lock, &irqL); */

			/*
			 * if(psta->sleepq_len > (NR_XMITFRAME>>3)) {
			 *	wakeup_sta_to_xmit(rtlpriv, psta);
			 * }
			 */

			ret = _TRUE;

		}

	}

	spin_unlock_bh(&psta->sleep_q.lock);

	return ret;

}

static void dequeue_xmitframes_to_sleeping_queue(struct rtl_priv *rtlpriv, struct sta_info *psta, struct __queue *pframequeue)
{
	sint ret;
	struct list_head	*plist, *phead;
	uint8_t	ac_index;
	struct tx_servq	*ptxservq;
	struct pkt_attrib	*pattrib;
	struct xmit_frame 	*pxmitframe;
	struct hw_xmit *phwxmits =  rtlpriv->xmitpriv.hwxmits;

	phead = get_list_head(pframequeue);
	plist = get_next(phead);

	while (rtw_end_of_queue_search(phead, plist) == _FALSE) {
		pxmitframe = LIST_CONTAINOR(plist, struct xmit_frame, list);

		plist = get_next(plist);

		ret = xmitframe_enqueue_for_sleeping_sta(rtlpriv, pxmitframe);

		if (_TRUE == ret) {
			pattrib = &pxmitframe->attrib;

			ptxservq = rtw_get_sta_pending(rtlpriv, psta, pattrib->priority, (uint8_t *)(&ac_index));

			ptxservq->qcnt--;
			phwxmits[ac_index].accnt--;
		} else 	{
			/* DBG_871X("xmitframe_enqueue_for_sleeping_sta return _FALSE\n"); */
		}

	}

}

void stop_sta_xmit(struct rtl_priv *rtlpriv, struct sta_info *psta)
{
	struct sta_info *psta_bmc;
	struct sta_xmit_priv *pstaxmitpriv;
	struct sta_priv *pstapriv = &rtlpriv->stapriv;
	struct xmit_priv *pxmitpriv = &rtlpriv->xmitpriv;

	pstaxmitpriv = &psta->sta_xmitpriv;

	/* for BC/MC Frames */
	psta_bmc = rtw_get_bcmc_stainfo(rtlpriv);


	spin_lock_bh(&pxmitpriv->lock);

	psta->state |= WIFI_SLEEP_STATE;

	pstapriv->sta_dz_bitmap |= BIT(psta->aid);

	dequeue_xmitframes_to_sleeping_queue(rtlpriv, psta, &pstaxmitpriv->vo_q.sta_pending);
	rtw_list_delete(&(pstaxmitpriv->vo_q.tx_pending));


	dequeue_xmitframes_to_sleeping_queue(rtlpriv, psta, &pstaxmitpriv->vi_q.sta_pending);
	rtw_list_delete(&(pstaxmitpriv->vi_q.tx_pending));


	dequeue_xmitframes_to_sleeping_queue(rtlpriv, psta, &pstaxmitpriv->be_q.sta_pending);
	rtw_list_delete(&(pstaxmitpriv->be_q.tx_pending));


	dequeue_xmitframes_to_sleeping_queue(rtlpriv, psta, &pstaxmitpriv->bk_q.sta_pending);
	rtw_list_delete(&(pstaxmitpriv->bk_q.tx_pending));

	/* for BC/MC Frames */
	pstaxmitpriv = &psta_bmc->sta_xmitpriv;
	dequeue_xmitframes_to_sleeping_queue(rtlpriv, psta_bmc, &pstaxmitpriv->be_q.sta_pending);
	rtw_list_delete(&(pstaxmitpriv->be_q.tx_pending));

	spin_unlock_bh(&pxmitpriv->lock);


}

void wakeup_sta_to_xmit(struct rtl_priv *rtlpriv, struct sta_info *psta)
{
	uint8_t update_mask = 0, wmmps_ac = 0;
	struct sta_info *psta_bmc;
	struct list_head	*xmitframe_plist, *xmitframe_phead;
	struct xmit_frame *pxmitframe = NULL;
	struct sta_priv *pstapriv = &rtlpriv->stapriv;
	struct xmit_priv *pxmitpriv = &rtlpriv->xmitpriv;

	psta_bmc = rtw_get_bcmc_stainfo(rtlpriv);

	/* spin_lock_bh(&psta->sleep_q.lock, &irqL); */
	spin_lock_bh(&pxmitpriv->lock);

	xmitframe_phead = get_list_head(&psta->sleep_q);
	xmitframe_plist = get_next(xmitframe_phead);

	while ((rtw_end_of_queue_search(xmitframe_phead, xmitframe_plist)) == _FALSE) {
		pxmitframe = LIST_CONTAINOR(xmitframe_plist, struct xmit_frame, list);

		xmitframe_plist = get_next(xmitframe_plist);

		rtw_list_delete(&pxmitframe->list);

		switch (pxmitframe->attrib.priority) {
		case 1:
		case 2:
			wmmps_ac = psta->uapsd_bk&BIT(1);
			break;
		case 4:
		case 5:
			wmmps_ac = psta->uapsd_vi&BIT(1);
			break;
		case 6:
		case 7:
			wmmps_ac = psta->uapsd_vo&BIT(1);
			break;
		case 0:
		case 3:
		default:
			wmmps_ac = psta->uapsd_be&BIT(1);
			break;
		}

		psta->sleepq_len--;
		if (psta->sleepq_len > 0)
			pxmitframe->attrib.mdata = 1;
		else
			pxmitframe->attrib.mdata = 0;

		if (wmmps_ac) {
			psta->sleepq_ac_len--;
			if (psta->sleepq_ac_len > 0) {
				pxmitframe->attrib.mdata = 1;
				pxmitframe->attrib.eosp = 0;
			} else {
				pxmitframe->attrib.mdata = 0;
				pxmitframe->attrib.eosp = 1;
			}
		}

		pxmitframe->attrib.triggered = 1;

/*
		spin_unlock_bh(&psta->sleep_q.lock, &irqL);
		if(rtw_hal_xmit(rtlpriv, pxmitframe) == _TRUE)
		{
			rtw_os_xmit_complete(rtlpriv, pxmitframe);
		}
		spin_lock_bh(&psta->sleep_q.lock, &irqL);
*/
		rtw_hal_xmitframe_enqueue(rtlpriv, pxmitframe);


	}

	/* for BC/MC Frames */
	if (!psta_bmc)
		goto _exit;

	if ((pstapriv->sta_dz_bitmap & 0xfffe) == 0x0) {
		/* no any sta in ps mode */

		xmitframe_phead = get_list_head(&psta_bmc->sleep_q);
		xmitframe_plist = get_next(xmitframe_phead);

		while ((rtw_end_of_queue_search(xmitframe_phead, xmitframe_plist)) == _FALSE) {
			pxmitframe = LIST_CONTAINOR(xmitframe_plist, struct xmit_frame, list);

			xmitframe_plist = get_next(xmitframe_plist);

			rtw_list_delete(&pxmitframe->list);

			psta_bmc->sleepq_len--;
			if (psta_bmc->sleepq_len > 0)
				pxmitframe->attrib.mdata = 1;
			else
				pxmitframe->attrib.mdata = 0;


			pxmitframe->attrib.triggered = 1;
/*
			spin_unlock_bh(&psta_bmc->sleep_q.lock, &irqL);
			if(rtw_hal_xmit(rtlpriv, pxmitframe) == _TRUE)
			{
				rtw_os_xmit_complete(rtlpriv, pxmitframe);
			}
			spin_lock_bh(&psta_bmc->sleep_q.lock, &irqL);

*/
			rtw_hal_xmitframe_enqueue(rtlpriv, pxmitframe);

		}

		if (psta_bmc->sleepq_len == 0) {
			pstapriv->tim_bitmap &= ~BIT(0);
			pstapriv->sta_dz_bitmap &= ~BIT(0);

			/* DBG_871X("wakeup to xmit, qlen==0, update_BCNTIM, tim=%x\n", pstapriv->tim_bitmap); */
			/* upate BCN for TIM IE */
			/* update_BCNTIM(rtlpriv); */
			update_mask |= BIT(1);
		}

	}

	if (psta->sleepq_len == 0) {
		pstapriv->tim_bitmap &= ~BIT(psta->aid);

		/* DBG_871X("wakeup to xmit, qlen==0, update_BCNTIM, tim=%x\n", pstapriv->tim_bitmap); */
		/* upate BCN for TIM IE */
		/* update_BCNTIM(rtlpriv); */
		update_mask = BIT(0);

		if (psta->state & WIFI_SLEEP_STATE)
			psta->state ^= WIFI_SLEEP_STATE;

		if (psta->state & WIFI_STA_ALIVE_CHK_STATE) {
			psta->expire_to = pstapriv->expire_to;
			psta->state ^= WIFI_STA_ALIVE_CHK_STATE;
		}

		pstapriv->sta_dz_bitmap &= ~BIT(psta->aid);
	}

_exit:

	/*spin_unlock_bh(&psta_bmc->sleep_q.lock, &irqL); */
	spin_unlock_bh(&pxmitpriv->lock);

	if (update_mask) {
		/* update_BCNTIM(rtlpriv); */
		/* printk("%s => call update_beacon\n",__FUNCTION__); */
		update_beacon(rtlpriv, _TIM_IE_, NULL, _FALSE);
	}

}

void xmit_delivery_enabled_frames(struct rtl_priv *rtlpriv, struct sta_info *psta)
{
	uint8_t wmmps_ac = 0;
	struct list_head *xmitframe_plist, *xmitframe_phead;
	struct xmit_frame *pxmitframe = NULL;
	struct sta_priv *pstapriv = &rtlpriv->stapriv;
	struct xmit_priv *pxmitpriv = &rtlpriv->xmitpriv;

	/* spin_lock_bh(&psta->sleep_q.lock, &irqL); */
	spin_lock_bh(&pxmitpriv->lock);

	xmitframe_phead = get_list_head(&psta->sleep_q);
	xmitframe_plist = get_next(xmitframe_phead);

	while ((rtw_end_of_queue_search(xmitframe_phead, xmitframe_plist)) == _FALSE) {
		pxmitframe = LIST_CONTAINOR(xmitframe_plist, struct xmit_frame, list);

		xmitframe_plist = get_next(xmitframe_plist);

		switch (pxmitframe->attrib.priority) {
		case 1:
		case 2:
			wmmps_ac = psta->uapsd_bk&BIT(1);
			break;
		case 4:
		case 5:
			wmmps_ac = psta->uapsd_vi&BIT(1);
			break;
		case 6:
		case 7:
			wmmps_ac = psta->uapsd_vo&BIT(1);
			break;
		case 0:
		case 3:
		default:
			wmmps_ac = psta->uapsd_be&BIT(1);
			break;
		}

		if (!wmmps_ac)
			continue;

		rtw_list_delete(&pxmitframe->list);

		psta->sleepq_len--;
		psta->sleepq_ac_len--;

		if (psta->sleepq_ac_len > 0) {
			pxmitframe->attrib.mdata = 1;
			pxmitframe->attrib.eosp = 0;
		} else {
			pxmitframe->attrib.mdata = 0;
			pxmitframe->attrib.eosp = 1;
		}

		pxmitframe->attrib.triggered = 1;

/*
		if(rtw_hal_xmit(rtlpriv, pxmitframe) == _TRUE)
		{
			rtw_os_xmit_complete(rtlpriv, pxmitframe);
		}
*/
		rtw_hal_xmitframe_enqueue(rtlpriv, pxmitframe);

		if ((psta->sleepq_ac_len == 0) && (!psta->has_legacy_ac) && (wmmps_ac)) {
			pstapriv->tim_bitmap &= ~BIT(psta->aid);

			/* DBG_871X("wakeup to xmit, qlen==0, update_BCNTIM, tim=%x\n", pstapriv->tim_bitmap); */
			/* upate BCN for TIM IE */
			/* update_BCNTIM(rtlpriv); */
			update_beacon(rtlpriv, _TIM_IE_, NULL, _FALSE);
			/* update_mask = BIT(0); */
		}

	}

	/* spin_unlock_bh(&psta->sleep_q.lock, &irqL); */
	spin_unlock_bh(&pxmitpriv->lock);

}

#endif


void rtw_sctx_init(struct submit_ctx *sctx, int timeout_ms)
{
	sctx->timeout_ms = timeout_ms;
	sctx->submit_time = jiffies;
#ifdef PLATFORM_LINUX				/* TODO: add condition wating interface for other os */
	init_completion(&sctx->done);
#endif
	sctx->status = RTW_SCTX_SUBMITTED;
}

int rtw_sctx_wait(struct submit_ctx *sctx)
{
	int ret = _FAIL;
	unsigned long expire;
	int status = 0;

#ifdef PLATFORM_LINUX
	expire = sctx->timeout_ms ? msecs_to_jiffies(sctx->timeout_ms) : MAX_SCHEDULE_TIMEOUT;
	if (!wait_for_completion_timeout(&sctx->done, expire)) {
		/* timeout, do something?? */
		status = RTW_SCTX_DONE_TIMEOUT;
		DBG_871X("%s timeout\n", __func__);
	} else {
		status = sctx->status;
	}
#endif

	if (status == RTW_SCTX_DONE_SUCCESS) {
		ret = _SUCCESS;
	}

	return ret;
}

bool rtw_sctx_chk_waring_status(int status)
{
	switch (status) {
	case RTW_SCTX_DONE_UNKNOWN:
	case RTW_SCTX_DONE_BUF_ALLOC:
	case RTW_SCTX_DONE_BUF_FREE:

	case RTW_SCTX_DONE_DRV_STOP:
	case RTW_SCTX_DONE_DEV_REMOVE:
		return _TRUE;
	default:
		return _FALSE;
	}
}

void rtw_sctx_done_err(struct submit_ctx **sctx, int status)
{
	if (*sctx) {
		if (rtw_sctx_chk_waring_status(status))
			DBG_871X("%s status:%d\n", __func__, status);
		(*sctx)->status = status;
		#ifdef PLATFORM_LINUX
		complete(&((*sctx)->done));
		#endif
		*sctx = NULL;
	}
}

void rtw_sctx_done(struct submit_ctx **sctx)
{
	rtw_sctx_done_err(sctx, RTW_SCTX_DONE_SUCCESS);
}

