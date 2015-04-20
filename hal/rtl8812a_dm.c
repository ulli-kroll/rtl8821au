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

#define _RTL8812A_DM_C_

#include <rtl8812a_hal.h>
#include <../rtl8821au/dm.h>


/*
 * Initialize GPIO setting registers
 */

static void dm_InitGPIOSetting(struct rtl_priv *rtlpriv)
{
	struct _rtw_hal *	pHalData = GET_HAL_DATA(rtlpriv);

	uint8_t	tmp1byte;

	tmp1byte = rtl_read_byte(rtlpriv, REG_GPIO_MUXCFG);
	tmp1byte &= (GPIOSEL_GPIO | ~GPIOSEL_ENBT);

	rtl_write_byte(rtlpriv, REG_GPIO_MUXCFG, tmp1byte);

}

/*
 * ============================================================
 * functions
 * ============================================================
 */

static void Init_ODM_ComInfo_8812(struct rtl_priv *rtlpriv)
{
	struct rtl_hal *rtlhal = rtl_hal(rtlpriv);
	struct _rtw_hal *pHalData = GET_HAL_DATA(rtlpriv);
	EEPROM_EFUSE_PRIV	*pEEPROM = GET_EEPROM_EFUSE_PRIV(rtlpriv);
	struct dm_priv	*pdmpriv = &pHalData->dmpriv;
	struct _rtw_dm *	pDM_Odm = &(pHalData->odmpriv);
	uint8_t	cut_ver,fab_ver;
	uint8_t	BoardType = ODM_BOARD_DEFAULT;

	/*
	 * Init Value
	 */

	memset(pDM_Odm,0,sizeof(*pDM_Odm));

	pDM_Odm->rtlpriv = rtlpriv;

	if (IS_HARDWARE_TYPE_8812(rtlhal))
		ODM_CmnInfoInit(pDM_Odm, ODM_CMNINFO_IC_TYPE, ODM_RTL8812);
	else if (IS_HARDWARE_TYPE_8821(rtlhal))
		ODM_CmnInfoInit(pDM_Odm, ODM_CMNINFO_IC_TYPE, ODM_RTL8821);


	fab_ver = ODM_TSMC;
	if (IS_VENDOR_8812A_C_CUT(rtlpriv))
		cut_ver = ODM_CUT_C;
	else
		cut_ver = ODM_CUT_A;

	ODM_CmnInfoInit(pDM_Odm,ODM_CMNINFO_FAB_VER,fab_ver);
	ODM_CmnInfoInit(pDM_Odm,ODM_CMNINFO_CUT_VER,cut_ver);

	ODM_CmnInfoInit(pDM_Odm,	ODM_CMNINFO_MP_TEST_CHIP,IS_NORMAL_CHIP(pHalData->VersionID));

	/* 1 ======= BoardType: ODM_CMNINFO_BOARD_TYPE ======= */
	if(pHalData->InterfaceSel == INTF_SEL1_USB_High_Power)
	{
		rtlhal->external_pa_2g = 1;
		rtlhal->external_lna_2g = 1;
	}
	else
	{
		rtlhal->external_lna_2g = 0;
	}

	if (rtlhal->external_lna_2g != 0) {
		BoardType |= ODM_BOARD_EXT_LNA;
	}
	if (rtlhal->external_lna_5g != 0) {
		BoardType |= ODM_BOARD_EXT_LNA_5G;
	}
	if (rtlhal->external_pa_2g != 0) {
		BoardType |= ODM_BOARD_EXT_PA;
	}
	if (rtlhal->external_pa_5g != 0) {
		BoardType |= ODM_BOARD_EXT_PA_5G;
	}

	ODM_CmnInfoInit(pDM_Odm, ODM_CMNINFO_BOARD_TYPE, BoardType);

	/* 1 ============== End of BoardType ============== */

	ODM_CmnInfoInit(pDM_Odm, ODM_CMNINFO_EXT_TRSW, 0);

	ODM_CmnInfoInit(pDM_Odm,ODM_CMNINFO_PATCH_ID,pEEPROM->CustomerID);
	/* 	ODM_CMNINFO_BINHCT_TEST only for MP Team */
	ODM_CmnInfoInit(pDM_Odm,ODM_CMNINFO_BWIFI_TEST,rtlpriv->registrypriv.wifi_spec);

 	ODM_CmnInfoInit(pDM_Odm, ODM_CMNINFO_RF_ANTENNA_TYPE, pHalData->TRxAntDivType);

	pdmpriv->InitODMFlag =	ODM_RF_CALIBRATION		|
							ODM_RF_TX_PWR_TRACK	//|
							;
	/* if(pHalData->AntDivCfg)
	 *	pdmpriv->InitODMFlag |= ODM_BB_ANT_DIV;
	 */

	ODM_CmnInfoUpdate(pDM_Odm,ODM_CMNINFO_ABILITY,pdmpriv->InitODMFlag);

}
static void Update_ODM_ComInfo_8812(struct rtl_priv *rtlpriv)
{
	struct mlme_ext_priv	*pmlmeext = &rtlpriv->mlmeextpriv;
	struct mlme_priv	*pmlmepriv = &rtlpriv->mlmepriv;
	struct pwrctrl_priv *pwrctrlpriv = &rtlpriv->pwrctrlpriv;
	struct _rtw_hal *pHalData = GET_HAL_DATA(rtlpriv);
	struct _rtw_dm *	pDM_Odm = &(pHalData->odmpriv);
	struct dm_priv	*pdmpriv = &pHalData->dmpriv;
	int i;

	pdmpriv->InitODMFlag =	ODM_BB_DIG |
#ifdef	CONFIG_ODM_REFRESH_RAMASK
				ODM_BB_RA_MASK		|
#endif
				ODM_BB_FA_CNT			|
				ODM_BB_RSSI_MONITOR	|
				ODM_RF_TX_PWR_TRACK	|	/* For RF */
				ODM_MAC_EDCA_TURBO
			;
	if (pHalData->AntDivCfg)
		pdmpriv->InitODMFlag |= ODM_BB_ANT_DIV;

	ODM_CmnInfoUpdate(pDM_Odm,ODM_CMNINFO_ABILITY,pdmpriv->InitODMFlag);

	ODM_CmnInfoHook(pDM_Odm,ODM_CMNINFO_WM_MODE,&(pmlmeext->cur_wireless_mode));
	ODM_CmnInfoHook(pDM_Odm,ODM_CMNINFO_BAND,&(pHalData->CurrentBandType));
	ODM_CmnInfoHook(pDM_Odm,ODM_CMNINFO_FORCED_RATE,&(pHalData->ForcedDataRate));

	ODM_CmnInfoHook(pDM_Odm,ODM_CMNINFO_SEC_CHNL_OFFSET,&(pHalData->nCur40MhzPrimeSC));
	ODM_CmnInfoHook(pDM_Odm,ODM_CMNINFO_SEC_MODE,&(rtlpriv->securitypriv.dot11PrivacyAlgrthm));
	ODM_CmnInfoHook(pDM_Odm,ODM_CMNINFO_CHNL,&( rtlpriv->phy.current_channel));
	ODM_CmnInfoHook(pDM_Odm,ODM_CMNINFO_NET_CLOSED,&( rtlpriv->net_closed));
	ODM_CmnInfoHook(pDM_Odm,ODM_CMNINFO_MP_MODE,&(rtlpriv->registrypriv.mp_mode));
	ODM_CmnInfoHook(pDM_Odm,ODM_CMNINFO_SCAN,&(pmlmepriv->bScanInProcess));
	ODM_CmnInfoHook(pDM_Odm,ODM_CMNINFO_POWER_SAVING,&(pwrctrlpriv->bpower_saving));
	ODM_CmnInfoInit(pDM_Odm, ODM_CMNINFO_RF_ANTENNA_TYPE, pHalData->TRxAntDivType);

	for (i = 0; i < NUM_STA; i++) {
		/* pDM_Odm->pODM_StaInfo[i] = NULL; */
		ODM_CmnInfoPtrArrayHook(pDM_Odm, ODM_CMNINFO_STA_STATUS,i,NULL);
	}
}

void rtl8812_InitHalDm(struct rtl_priv *rtlpriv)
{
	struct _rtw_hal *pHalData = GET_HAL_DATA(rtlpriv);
	struct dm_priv	*pdmpriv = &pHalData->dmpriv;
	struct _rtw_dm *	pDM_Odm = &(pHalData->odmpriv);
	uint8_t	i;

	dm_InitGPIOSetting(rtlpriv);

	pdmpriv->DM_Type = DM_Type_ByDriver;
	pdmpriv->DMFlag = DYNAMIC_FUNC_DISABLE;

	Update_ODM_ComInfo_8812(rtlpriv);
	ODM_DMInit(pDM_Odm);

	rtlpriv->fix_rate = 0xFF;
}


void rtl8812_init_dm_priv(IN struct rtl_priv *rtlpriv)
{
	struct _rtw_hal *pHalData = GET_HAL_DATA(rtlpriv);
	struct dm_priv	*pdmpriv = &pHalData->dmpriv;
	struct _rtw_dm *		podmpriv = &pHalData->odmpriv;
	memset(pdmpriv, 0, sizeof(struct dm_priv));

	/* spin_lock_init(&(pHalData->odm_stainfo_lock)); */

	Init_ODM_ComInfo_8812(rtlpriv);
#ifdef CONFIG_SW_ANTENNA_DIVERSITY
	/*
	 * _init_timer(&(pdmpriv->SwAntennaSwitchTimer),  rtlpriv->ndev , odm_SW_AntennaSwitchCallback, rtlpriv);
	 */
	ODM_InitAllTimers(podmpriv );
#endif
	ODM_InitDebugSetting(podmpriv);

	rtlpriv->registrypriv.RegEnableTxPowerLimit = 0;
	rtlpriv->registrypriv.RegPowerBase = 14;
	rtlpriv->registrypriv.RegTxPwrLimit = 0xFFFFFFFF;
	rtlpriv->registrypriv.TxBBSwing_2G = 0xFF;
	rtlpriv->registrypriv.TxBBSwing_5G = 0xFF;
	rtlpriv->registrypriv.bEn_RFE = 0;
	rtlpriv->registrypriv.RFE_Type = 64;
	pHalData->RegRFPathS1 = 0;
	pHalData->TxPwrInPercentage = TX_PWR_PERCENTAGE_3;
}

void rtl8812_deinit_dm_priv(IN struct rtl_priv *rtlpriv)
{
	struct _rtw_hal *pHalData = GET_HAL_DATA(rtlpriv);
	struct dm_priv	*pdmpriv = &pHalData->dmpriv;
	struct _rtw_dm *		podmpriv = &pHalData->odmpriv;
#ifdef CONFIG_SW_ANTENNA_DIVERSITY
	/* _cancel_timer_ex(&pdmpriv->SwAntennaSwitchTimer); */
	ODM_CancelAllTimers(podmpriv);
#endif
}


#ifdef CONFIG_ANTENNA_DIVERSITY
/*
 *  Add new function to reset the state of antenna diversity before link.
 *
 * Compare RSSI for deciding antenna
 */
void AntDivCompare8812(struct rtl_priv *rtlpriv, WLAN_BSSID_EX *dst, WLAN_BSSID_EX *src)
{
	/* struct rtl_priv *rtlpriv = pDM_Odm->rtlpriv ; */

	 struct _rtw_hal	*pHalData = GET_HAL_DATA(rtlpriv);
	if (0 != pHalData->AntDivCfg ) {
		/*
		 * DBG_8192C("update_network=> orgRSSI(%d)(%d),newRSSI(%d)(%d)\n",dst->Rssi,query_rx_pwr_percentage(dst->Rssi),
		 * 	src->Rssi,query_rx_pwr_percentage(src->Rssi));
		 * select optimum_antenna for before linked =>For antenna diversity
		 */
		if(dst->Rssi >=  src->Rssi )//keep org parameter
		{
			src->Rssi = dst->Rssi;
			src->PhyInfo.Optimum_antenna = dst->PhyInfo.Optimum_antenna;
		}
	}
}

/* Add new function to reset the state of antenna diversity before link. */
uint8_t AntDivBeforeLink8812(struct rtl_priv *rtlpriv )
{

	 struct _rtw_hal	*pHalData = GET_HAL_DATA(rtlpriv);
	struct _rtw_dm *	pDM_Odm =&pHalData->odmpriv;
	SWAT_T		*pDM_SWAT_Table = &pDM_Odm->DM_SWAT_Table;
	struct mlme_priv	*pmlmepriv = &(rtlpriv->mlmepriv);

	/* Condition that does not need to use antenna diversity. */
	if (pHalData->AntDivCfg==0) {
		/* DBG_8192C("odm_AntDivBeforeLink8192C(): No AntDiv Mechanism.\n"); */
		return _FALSE;
	}

	if (check_fwstate(pmlmepriv, _FW_LINKED) == _TRUE) {
		return _FALSE;
	}


	if (pDM_SWAT_Table->SWAS_NoLink_State == 0){
		/* switch channel */
		pDM_SWAT_Table->SWAS_NoLink_State = 1;
		pDM_SWAT_Table->CurAntenna = (pDM_SWAT_Table->CurAntenna==MAIN_ANT)?AUX_ANT:MAIN_ANT;

		/*
		 * rtl_set_bbreg(rtlpriv, rFPGA0_XA_RFInterfaceOE, 0x300, pDM_SWAT_Table->CurAntenna);
		*/
		rtw_antenna_select_cmd(rtlpriv, pDM_SWAT_Table->CurAntenna, _FALSE);
		/*
		 * DBG_8192C("%s change antenna to ANT_( %s ).....\n",__FUNCTION__, (pDM_SWAT_Table->CurAntenna==MAIN_ANT)?"MAIN":"AUX");
		 */
		return _TRUE;
	} else {
		pDM_SWAT_Table->SWAS_NoLink_State = 0;
		return _FALSE;
	}

}
#endif

