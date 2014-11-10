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

#include <drv_types.h>
#include <hal_data.h>
#include <rtw_sreset.h>

void sreset_init_value(struct rtl_priv *padapter)
{
#if defined(DBG_CONFIG_ERROR_DETECT)
	 struct rtw_hal *pHalData = GET_HAL_DATA(padapter);
	struct sreset_priv *psrtpriv = &pHalData->srestpriv;

	/* ULLI: No mutex_drestroy found for this mutex */
	mutex_init(&psrtpriv->silentreset_mutex);
	psrtpriv->silent_reset_inprogress = _FALSE;
	psrtpriv->Wifi_Error_Status = WIFI_STATUS_SUCCESS;
	psrtpriv->last_tx_time =0;
	psrtpriv->last_tx_complete_time =0;
#endif
}
void sreset_reset_value(struct rtl_priv *padapter)
{
#if defined(DBG_CONFIG_ERROR_DETECT)
	 struct rtw_hal *pHalData = GET_HAL_DATA(padapter);
	struct sreset_priv *psrtpriv = &pHalData->srestpriv;

	psrtpriv->silent_reset_inprogress = _FALSE;
	psrtpriv->Wifi_Error_Status = WIFI_STATUS_SUCCESS;
	psrtpriv->last_tx_time =0;
	psrtpriv->last_tx_complete_time =0;
#endif
}

uint8_t sreset_get_wifi_status(struct rtl_priv *padapter)
{
#if defined(DBG_CONFIG_ERROR_DETECT)
	 struct rtw_hal *pHalData = GET_HAL_DATA(padapter);
	struct sreset_priv *psrtpriv = &pHalData->srestpriv;

	uint8_t status = WIFI_STATUS_SUCCESS;
	uint32_t val32 = 0;
	_irqL irqL;

	if(psrtpriv->silent_reset_inprogress == _TRUE)
		return status;

	val32 =rtw_read32(padapter,REG_TXDMA_STATUS);

	if(val32==0xeaeaeaea)
		psrtpriv->Wifi_Error_Status = WIFI_IF_NOT_EXIST;
	else if(val32!=0){
		DBG_8192C("txdmastatu(%x)\n",val32);
		psrtpriv->Wifi_Error_Status = WIFI_MAC_TXDMA_ERROR;
	}

	if(WIFI_STATUS_SUCCESS !=psrtpriv->Wifi_Error_Status) {
		DBG_8192C("==>%s error_status(0x%x) \n",__FUNCTION__,psrtpriv->Wifi_Error_Status);
		status = (psrtpriv->Wifi_Error_Status &( ~(USB_READ_PORT_FAIL|USB_WRITE_PORT_FAIL)));
	}

	DBG_8192C("==> %s wifi_status(0x%x)\n",__FUNCTION__,status);

	/* status restore */
	psrtpriv->Wifi_Error_Status = WIFI_STATUS_SUCCESS;

	return status;
#else
	return WIFI_STATUS_SUCCESS;
#endif
}

void sreset_set_wifi_error_status(struct rtl_priv *padapter, uint32_t	 status)
{
#if defined(DBG_CONFIG_ERROR_DETECT)
	 struct rtw_hal	*pHalData = GET_HAL_DATA(padapter);
	pHalData->srestpriv.Wifi_Error_Status = status;
#endif
}

void sreset_set_trigger_point(struct rtl_priv *padapter, int32_t tgp)
{
#if defined(DBG_CONFIG_ERROR_DETECT)
	 struct rtw_hal	*pHalData = GET_HAL_DATA(padapter);
	pHalData->srestpriv.dbg_trigger_point = tgp;
#endif
}

bool sreset_inprogress(struct rtl_priv *padapter)
{
	return _FALSE;
}

void sreset_restore_security_station(struct rtl_priv *padapter)
{
	uint8_t EntryId = 0;
	struct mlme_priv *mlmepriv = &padapter->mlmepriv;
	struct sta_priv * pstapriv = &padapter->stapriv;
	struct sta_info *psta;
	struct security_priv* psecuritypriv=&(padapter->securitypriv);
	struct mlme_ext_info	*pmlmeinfo = &padapter->mlmeextpriv.mlmext_info;

	{
		uint8_t val8;

		if (pmlmeinfo->auth_algo == dot11AuthAlgrthm_8021X) {
			val8 = 0xcc;
		} else {
			val8 = 0xcf;
		}
		rtw_hal_set_hwreg(padapter, HW_VAR_SEC_CFG, (uint8_t *)(&val8));
	}

	if((padapter->securitypriv.dot11PrivacyAlgrthm == _TKIP_) ||
		(padapter->securitypriv.dot11PrivacyAlgrthm == _AES_))
	{
		psta = rtw_get_stainfo(pstapriv, get_bssid(mlmepriv));
		if (psta == NULL) {
			//DEBUG_ERR( ("Set wpa_set_encryption: Obtain Sta_info fail \n"));
		}
		else
		{
			//pairwise key
			rtw_setstakey_cmd(padapter, (unsigned char *)psta, _TRUE);
			//group key
			rtw_set_key(padapter,&padapter->securitypriv,padapter->securitypriv.dot118021XGrpKeyid, 0);
		}
	}
}

void sreset_restore_network_station(struct rtl_priv *padapter)
{
	struct mlme_priv *mlmepriv = &padapter->mlmepriv;
	struct mlme_ext_priv	*pmlmeext = &padapter->mlmeextpriv;
	struct mlme_ext_info	*pmlmeinfo = &(pmlmeext->mlmext_info);

	rtw_setopmode_cmd(padapter, Ndis802_11Infrastructure);

	{
		uint8_t threshold;
		/*
		 * TH=1 => means that invalidate usb rx aggregation
		 * TH=0 => means that validate usb rx aggregation, use init value.
		 */
		if(mlmepriv->htpriv.ht_option) {
			if(padapter->registrypriv.wifi_spec==1)
				threshold = 1;
			else
				threshold = 0;
			rtw_hal_set_hwreg(padapter, HW_VAR_RXDMA_AGG_PG_TH, (uint8_t *)(&threshold));
		} else {
			threshold = 1;
			rtw_hal_set_hwreg(padapter, HW_VAR_RXDMA_AGG_PG_TH, (uint8_t *)(&threshold));
		}
	}

	set_channel_bwmode(padapter, pmlmeext->cur_channel, pmlmeext->cur_ch_offset, pmlmeext->cur_bwmode);

	/*
	 * disable dynamic functions, such as high power, DIG
	 * Switch_DM_Func(padapter, DYNAMIC_FUNC_DISABLE, _FALSE);
	 */

	rtw_hal_set_hwreg(padapter, HW_VAR_BSSID, pmlmeinfo->network.MacAddress);

	{
		uint8_t	join_type = 0;
		rtw_hal_set_hwreg(padapter, HW_VAR_MLME_JOIN, (uint8_t *)(&join_type));
	}

	Set_MSR(padapter, (pmlmeinfo->state & 0x3));

	mlmeext_joinbss_event_callback(padapter, 1);
	/* restore Sequence No. */
	rtw_write8(padapter,0x4dc,padapter->xmitpriv.nqos_ssn);

	sreset_restore_security_station(padapter);
}

void sreset_restore_network_status(struct rtl_priv *padapter)
{
	struct mlme_priv *mlmepriv = &padapter->mlmepriv;
	struct mlme_ext_priv	*pmlmeext = &padapter->mlmeextpriv;
	struct mlme_ext_info	*pmlmeinfo = &(pmlmeext->mlmext_info);

	if (check_fwstate(mlmepriv, WIFI_STATION_STATE)) {
		DBG_871X(FUNC_ADPT_FMT" fwstate:0x%08x - WIFI_STATION_STATE\n", FUNC_ADPT_ARG(padapter), get_fwstate(mlmepriv));
		sreset_restore_network_station(padapter);
	} else if (check_fwstate(mlmepriv, WIFI_AP_STATE)) {
		DBG_871X(FUNC_ADPT_FMT" fwstate:0x%08x - WIFI_AP_STATE\n", FUNC_ADPT_ARG(padapter), get_fwstate(mlmepriv));
		rtw_ap_restore_network(padapter);
	} else if (check_fwstate(mlmepriv, WIFI_ADHOC_STATE)) {
		DBG_871X(FUNC_ADPT_FMT" fwstate:0x%08x - WIFI_ADHOC_STATE\n", FUNC_ADPT_ARG(padapter), get_fwstate(mlmepriv));
	} else {
		DBG_871X(FUNC_ADPT_FMT" fwstate:0x%08x - ???\n", FUNC_ADPT_ARG(padapter), get_fwstate(mlmepriv));
	}
}

void sreset_stop_adapter(struct rtl_priv *padapter)
{
	struct mlme_priv	*pmlmepriv = &(padapter->mlmepriv);
	struct xmit_priv	*pxmitpriv = &padapter->xmitpriv;

	if (padapter == NULL)
		return;

	DBG_871X(FUNC_ADPT_FMT"\n", FUNC_ADPT_ARG(padapter));

	if (!rtw_netif_queue_stopped(padapter->ndev))
		rtw_netif_stop_queue(padapter->ndev);

	rtw_cancel_all_timer(padapter);

	/* TODO: OS and HCI independent */
	tasklet_kill(&pxmitpriv->xmit_tasklet);

	if (check_fwstate(pmlmepriv, _FW_UNDER_SURVEY))
		rtw_scan_abort(padapter);

	if (check_fwstate(pmlmepriv, _FW_UNDER_LINKING))
		_rtw_join_timeout_handler(padapter);

}

void sreset_start_adapter(struct rtl_priv *padapter)
{
	struct mlme_priv	*pmlmepriv = &(padapter->mlmepriv);
	struct xmit_priv	*pxmitpriv = &padapter->xmitpriv;

	if (padapter == NULL)
		return;

	DBG_871X(FUNC_ADPT_FMT"\n", FUNC_ADPT_ARG(padapter));

	if (check_fwstate(pmlmepriv, _FW_LINKED)) {
		sreset_restore_network_status(padapter);
	}

	/* TODO: OS and HCI independent */
	tasklet_hi_schedule(&pxmitpriv->xmit_tasklet);

	_set_timer(&padapter->mlmepriv.dynamic_chk_timer, 2000);

	if (rtw_netif_queue_stopped(padapter->ndev))
		rtw_netif_wake_queue(padapter->ndev);

}

void sreset_reset(struct rtl_priv *padapter)
{
}

