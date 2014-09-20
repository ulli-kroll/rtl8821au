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


static VOID dm_CheckProtection(PADAPTER	Adapter)
{
#if 0
	PMGNT_INFO pMgntInfo = &(Adapter->MgntInfo);
	u1Byte CurRate, RateThreshold;

	if (pMgntInfo->pHTInfo->bCurBW40MHz)
		RateThreshold = MGN_MCS1;
	else
		RateThreshold = MGN_MCS3;

	if (Adapter->TxStats.CurrentInitTxRate <= RateThreshold) {
		pMgntInfo->bDmDisableProtect = TRUE;
		DbgPrint("Forced disable protect: %x\n", Adapter->TxStats.CurrentInitTxRate);
	} else {
		pMgntInfo->bDmDisableProtect = FALSE;
		DbgPrint("Enable protect: %x\n", Adapter->TxStats.CurrentInitTxRate);
	}
#endif
}

static VOID dm_CheckStatistics(PADAPTER	Adapter)
{
#if 0
	if (!Adapter->MgntInfo.bMediaConnect)
		return;

	/* 2008.12.10 tynli Add for getting Current_Tx_Rate_Reg flexibly. */
	rtw_hal_get_hwreg( Adapter, HW_VAR_INIT_TX_RATE, (pu1Byte)(&Adapter->TxStats.CurrentInitTxRate) );

	/*  Calculate current Tx Rate(Successful transmited!!) */

	/*  Calculate current Rx Rate(Successful received!!) */

	/* for tx tx retry count */
	rtw_hal_get_hwreg( Adapter, HW_VAR_RETRY_COUNT, (pu1Byte)(&Adapter->TxStats.NumTxRetryCount) );
#endif
}

static void dm_CheckPbcGPIO(_adapter *padapter)
{
	uint8_t	tmp1byte;
	uint8_t	bPbcPressed = _FALSE;

	if(!padapter->registrypriv.hw_wps_pbc)
		return;

#if defined(CONFIG_USB_HCI)
	if (IS_HARDWARE_TYPE_8812(padapter)) {
		tmp1byte = rtw_read8(padapter, GPIO_IO_SEL);
		tmp1byte |= (HAL_8192C_HW_GPIO_WPS_BIT);
		rtw_write8(padapter, GPIO_IO_SEL, tmp1byte);	/* enable GPIO[2] as output mode */

		tmp1byte &= ~(HAL_8192C_HW_GPIO_WPS_BIT);
		rtw_write8(padapter,  GPIO_IN, tmp1byte);	/* reset the floating voltage level */

		tmp1byte = rtw_read8(padapter, GPIO_IO_SEL);
		tmp1byte &= ~(HAL_8192C_HW_GPIO_WPS_BIT);
		rtw_write8(padapter, GPIO_IO_SEL, tmp1byte);	/* enable GPIO[2] as input mode */

		tmp1byte =rtw_read8(padapter, GPIO_IN);

		if (tmp1byte == 0xff)
			return ;

		if (tmp1byte&HAL_8192C_HW_GPIO_WPS_BIT) {
			bPbcPressed = _TRUE;
		}
	} else if (IS_HARDWARE_TYPE_8821(padapter)) {
		tmp1byte = rtw_read8(padapter, GPIO_IO_SEL_8811A);
		tmp1byte |= (BIT4);
		rtw_write8(padapter, GPIO_IO_SEL_8811A, tmp1byte);	/* enable GPIO[2] as output mode */

		tmp1byte &= ~(BIT4);
		rtw_write8(padapter,  GPIO_IN_8811A, tmp1byte);		/* reset the floating voltage level */

		tmp1byte = rtw_read8(padapter, GPIO_IO_SEL_8811A);
		tmp1byte &= ~(BIT4);
		rtw_write8(padapter, GPIO_IO_SEL_8811A, tmp1byte);	/* enable GPIO[2] as input mode */

		tmp1byte =rtw_read8(padapter, GPIO_IN_8811A);

		if (tmp1byte == 0xff)
			return ;

		if (tmp1byte&BIT4) {
			bPbcPressed = _TRUE;
		}
	}
#else

#endif
	if( _TRUE == bPbcPressed) {
		/*
		 * Here we only set bPbcPressed to true
		 * After trigger PBC, the variable will be set to false
		 */
		DBG_8192C("CheckPbcGPIO - PBC is pressed\n");

		rtw_request_wps_pbc_event(padapter);
	}
}

/*
 * Initialize GPIO setting registers
 */

static void dm_InitGPIOSetting(PADAPTER	Adapter)
{
	PHAL_DATA_TYPE		pHalData = GET_HAL_DATA(Adapter);

	uint8_t	tmp1byte;

	tmp1byte = rtw_read8(Adapter, REG_GPIO_MUXCFG);
	tmp1byte &= (GPIOSEL_GPIO | ~GPIOSEL_ENBT);

	rtw_write8(Adapter, REG_GPIO_MUXCFG, tmp1byte);

}

/* A mapping from HalData to ODM. */
ODM_BOARD_TYPE_E boardType(uint8_t InterfaceSel)
{
	ODM_BOARD_TYPE_E        board	= ODM_BOARD_DEFAULT;

#if defined(CONFIG_USB_HCI)
	INTERFACE_SELECT_USB    usb 	= (INTERFACE_SELECT_USB)InterfaceSel;
	switch (usb) {
	case INTF_SEL1_USB_High_Power:
		board |= ODM_BOARD_EXT_LNA;
		board |= ODM_BOARD_EXT_PA;
		break;
	case INTF_SEL2_MINICARD:
		board |= ODM_BOARD_MINICARD;
		break;
	case INTF_SEL4_USB_Combo:
		board |= ODM_BOARD_BT;
		break;
	case INTF_SEL5_USB_Combo_MF:
		board |= ODM_BOARD_BT;
		break;
	case INTF_SEL0_USB:
	case INTF_SEL3_USB_Solo:
	default:
		board = ODM_BOARD_DEFAULT;
		break;
	}

#endif
	/* DBG_871X("===> boardType(): (pHalData->InterfaceSel, pDM_Odm->BoardType) = (%d, %d)\n", InterfaceSel, board); */

	return board;
}

/*
 * ============================================================
 * functions
 * ============================================================
 */

static void Init_ODM_ComInfo_8812(PADAPTER Adapter)
{
	PHAL_DATA_TYPE	pHalData = GET_HAL_DATA(Adapter);
	EEPROM_EFUSE_PRIV	*pEEPROM = GET_EEPROM_EFUSE_PRIV(Adapter);
	struct dm_priv	*pdmpriv = &pHalData->dmpriv;
	PDM_ODM_T		pDM_Odm = &(pHalData->odmpriv);
	uint8_t	cut_ver,fab_ver;
	uint8_t	BoardType = ODM_BOARD_DEFAULT;

	/*
	 * Init Value
	 */

	memset(pDM_Odm,0,sizeof(pDM_Odm));

	pDM_Odm->Adapter = Adapter;

	ODM_CmnInfoInit(pDM_Odm,ODM_CMNINFO_PLATFORM,ODM_CE);

	if (Adapter->interface_type == RTW_GSPI)
		ODM_CmnInfoInit(pDM_Odm,ODM_CMNINFO_INTERFACE,ODM_ITRF_SDIO);
	else
		ODM_CmnInfoInit(pDM_Odm,ODM_CMNINFO_INTERFACE,Adapter->interface_type);


	if (IS_HARDWARE_TYPE_8812(Adapter))
		ODM_CmnInfoInit(pDM_Odm, ODM_CMNINFO_IC_TYPE, ODM_RTL8812);
	else if (IS_HARDWARE_TYPE_8821(Adapter))
		ODM_CmnInfoInit(pDM_Odm, ODM_CMNINFO_IC_TYPE, ODM_RTL8821);


	fab_ver = ODM_TSMC;
	if (IS_VENDOR_8812A_C_CUT(Adapter))
		cut_ver = ODM_CUT_C;
	else
		cut_ver = ODM_CUT_A;

	ODM_CmnInfoInit(pDM_Odm,ODM_CMNINFO_FAB_VER,fab_ver);
	ODM_CmnInfoInit(pDM_Odm,ODM_CMNINFO_CUT_VER,cut_ver);

	ODM_CmnInfoInit(pDM_Odm,	ODM_CMNINFO_MP_TEST_CHIP,IS_NORMAL_CHIP(pHalData->VersionID));

	/* 1 ======= BoardType: ODM_CMNINFO_BOARD_TYPE ======= */
#if defined(CONFIG_USB_HCI)
	if(pHalData->InterfaceSel == INTF_SEL1_USB_High_Power)
	{
		ODM_CmnInfoInit(pDM_Odm, ODM_CMNINFO_EXT_PA, 1);
		ODM_CmnInfoInit(pDM_Odm, ODM_CMNINFO_EXT_LNA, 1);
	}
	else
	{
		ODM_CmnInfoInit(pDM_Odm, ODM_CMNINFO_EXT_PA, pHalData->ExternalPA_2G);
		ODM_CmnInfoInit(pDM_Odm, ODM_CMNINFO_EXT_LNA, 0);
	}
#else
	/* PCIE no external PA now??? */
	ODM_CmnInfoInit(pDM_Odm, ODM_CMNINFO_EXT_PA, 0);
	ODM_CmnInfoInit(pDM_Odm, ODM_CMNINFO_EXT_LNA, 0);
#endif

	if (pHalData->ExternalLNA_2G != 0) {
		BoardType |= ODM_BOARD_EXT_LNA;
		ODM_CmnInfoInit(pDM_Odm, ODM_CMNINFO_EXT_LNA, 1);
	}
	if (pHalData->ExternalLNA_5G != 0) {
		BoardType |= ODM_BOARD_EXT_LNA_5G;
		ODM_CmnInfoInit(pDM_Odm, ODM_CMNINFO_5G_EXT_LNA, 1);
	}
	if (pHalData->ExternalPA_2G != 0) {
		BoardType |= ODM_BOARD_EXT_PA;
		ODM_CmnInfoInit(pDM_Odm, ODM_CMNINFO_EXT_PA, 1);
	}
	if (pHalData->ExternalPA_5G != 0) {
		BoardType |= ODM_BOARD_EXT_PA_5G;
		ODM_CmnInfoInit(pDM_Odm, ODM_CMNINFO_5G_EXT_PA, 1);
	}

	ODM_CmnInfoInit(pDM_Odm, ODM_CMNINFO_BOARD_TYPE, BoardType);

	/* 1 ============== End of BoardType ============== */

	ODM_CmnInfoInit(pDM_Odm, ODM_CMNINFO_RFE_TYPE, pHalData->RFEType);

	ODM_CmnInfoInit(pDM_Odm, ODM_CMNINFO_EXT_TRSW, 0);

	ODM_CmnInfoInit(pDM_Odm,ODM_CMNINFO_PATCH_ID,pEEPROM->CustomerID);
	/* 	ODM_CMNINFO_BINHCT_TEST only for MP Team */
	ODM_CmnInfoInit(pDM_Odm,ODM_CMNINFO_BWIFI_TEST,Adapter->registrypriv.wifi_spec);


	if(pHalData->rf_type == RF_1T1R){
		ODM_CmnInfoUpdate(pDM_Odm,ODM_CMNINFO_RF_TYPE,ODM_1T1R);
	}
	else if(pHalData->rf_type == RF_2T2R){
		ODM_CmnInfoUpdate(pDM_Odm,ODM_CMNINFO_RF_TYPE,ODM_2T2R);
	}
	else if(pHalData->rf_type == RF_1T2R){
		ODM_CmnInfoUpdate(pDM_Odm,ODM_CMNINFO_RF_TYPE,ODM_1T2R);
	}

	ODM_CmnInfoInit(pDM_Odm, ODM_CMNINFO_RFE_TYPE, pHalData->RFEType);

 	ODM_CmnInfoInit(pDM_Odm, ODM_CMNINFO_RF_ANTENNA_TYPE, pHalData->TRxAntDivType);

#ifdef CONFIG_DISABLE_ODM
	pdmpriv->InitODMFlag = 0;
#else
	pdmpriv->InitODMFlag =	ODM_RF_CALIBRATION		|
							ODM_RF_TX_PWR_TRACK	//|
							;
	/* if(pHalData->AntDivCfg)
	 *	pdmpriv->InitODMFlag |= ODM_BB_ANT_DIV;
	 */
#endif

	ODM_CmnInfoUpdate(pDM_Odm,ODM_CMNINFO_ABILITY,pdmpriv->InitODMFlag);

}
static void Update_ODM_ComInfo_8812(PADAPTER Adapter)
{
	struct mlme_ext_priv	*pmlmeext = &Adapter->mlmeextpriv;
	struct mlme_priv	*pmlmepriv = &Adapter->mlmepriv;
	struct pwrctrl_priv *pwrctrlpriv = &Adapter->pwrctrlpriv;
	PHAL_DATA_TYPE	pHalData = GET_HAL_DATA(Adapter);
	PDM_ODM_T		pDM_Odm = &(pHalData->odmpriv);
	struct dm_priv	*pdmpriv = &pHalData->dmpriv;
	int i;

#ifdef CONFIG_DISABLE_ODM
	pdmpriv->InitODMFlag = 0;
#else

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

#if (MP_DRIVER==1)
	if (Adapter->registrypriv.mp_mode == 1) {
		pdmpriv->InitODMFlag = 	ODM_RF_CALIBRATION	|
					ODM_RF_TX_PWR_TRACK;
		}
#endif

#endif
	ODM_CmnInfoUpdate(pDM_Odm,ODM_CMNINFO_ABILITY,pdmpriv->InitODMFlag);

	ODM_CmnInfoHook(pDM_Odm,ODM_CMNINFO_TX_UNI,&(Adapter->xmitpriv.tx_bytes));
	ODM_CmnInfoHook(pDM_Odm,ODM_CMNINFO_RX_UNI,&(Adapter->recvpriv.rx_bytes));
	ODM_CmnInfoHook(pDM_Odm,ODM_CMNINFO_WM_MODE,&(pmlmeext->cur_wireless_mode));
	ODM_CmnInfoHook(pDM_Odm,ODM_CMNINFO_BAND,&(pHalData->CurrentBandType));
	ODM_CmnInfoHook(pDM_Odm,ODM_CMNINFO_FORCED_RATE,&(pHalData->ForcedDataRate));

	ODM_CmnInfoHook(pDM_Odm,ODM_CMNINFO_SEC_CHNL_OFFSET,&(pHalData->nCur40MhzPrimeSC));
	ODM_CmnInfoHook(pDM_Odm,ODM_CMNINFO_SEC_MODE,&(Adapter->securitypriv.dot11PrivacyAlgrthm));
	ODM_CmnInfoHook(pDM_Odm,ODM_CMNINFO_BW,&(pHalData->CurrentChannelBW ));
	ODM_CmnInfoHook(pDM_Odm,ODM_CMNINFO_CHNL,&( pHalData->CurrentChannel));
	ODM_CmnInfoHook(pDM_Odm,ODM_CMNINFO_NET_CLOSED,&( Adapter->net_closed));
	ODM_CmnInfoHook(pDM_Odm,ODM_CMNINFO_MP_MODE,&(Adapter->registrypriv.mp_mode));
	/*
	 * ================= only for 8192D   =================
	 *
	 * pHalData->CurrentBandType92D
	 * ODM_CmnInfoHook(pDM_Odm,ODM_CMNINFO_DMSP_GET_VALUE,&(pDM_Odm->u1Byte_temp));
	 * ODM_CmnInfoHook(pDM_Odm,ODM_CMNINFO_BUDDY_ADAPTOR,&(pDM_Odm->PADAPTER_temp));
	 * ODM_CmnInfoHook(pDM_Odm,ODM_CMNINFO_DMSP_IS_MASTER,&(pDM_Odm->u1Byte_temp));
	 * ================= only for 8192D   =================
	 * driver havn't those variable now
	 * ODM_CmnInfoHook(pDM_Odm,ODM_CMNINFO_BT_OPERATION,&(pDM_Odm->u1Byte_temp));
	 * ODM_CmnInfoHook(pDM_Odm,ODM_CMNINFO_BT_DISABLE_EDCA,&(pDM_Odm->u1Byte_temp));
	 */

	ODM_CmnInfoHook(pDM_Odm,ODM_CMNINFO_SCAN,&(pmlmepriv->bScanInProcess));
	ODM_CmnInfoHook(pDM_Odm,ODM_CMNINFO_POWER_SAVING,&(pwrctrlpriv->bpower_saving));
	ODM_CmnInfoInit(pDM_Odm, ODM_CMNINFO_RF_ANTENNA_TYPE, pHalData->TRxAntDivType);

	for (i = 0; i < NUM_STA; i++) {
		/* pDM_Odm->pODM_StaInfo[i] = NULL; */
		ODM_CmnInfoPtrArrayHook(pDM_Odm, ODM_CMNINFO_STA_STATUS,i,NULL);
	}
}

void rtl8812_InitHalDm(PADAPTER	Adapter)
{
	PHAL_DATA_TYPE	pHalData = GET_HAL_DATA(Adapter);
	struct dm_priv	*pdmpriv = &pHalData->dmpriv;
	PDM_ODM_T		pDM_Odm = &(pHalData->odmpriv);
	uint8_t	i;

#ifdef CONFIG_USB_HCI
	dm_InitGPIOSetting(Adapter);
#endif

	pdmpriv->DM_Type = DM_Type_ByDriver;
	pdmpriv->DMFlag = DYNAMIC_FUNC_DISABLE;

	Update_ODM_ComInfo_8812(Adapter);
	ODM_DMInit(pDM_Odm);

	Adapter->fix_rate = 0xFF;
}


VOID rtl8812_HalDmWatchDog(PADAPTER Adapter)
{
	BOOLEAN		bFwCurrentInPSMode = _FALSE;
	BOOLEAN		bFwPSAwake = _TRUE;
	uint8_t hw_init_completed = _FALSE;
	PHAL_DATA_TYPE	pHalData = GET_HAL_DATA(Adapter);
	struct dm_priv	*pdmpriv = &pHalData->dmpriv;
	PDM_ODM_T		pDM_Odm = &(pHalData->odmpriv);

	hw_init_completed = Adapter->hw_init_completed;

	if (hw_init_completed == _FALSE)
		goto skip_dm;

#ifdef CONFIG_LPS
	{
		bFwCurrentInPSMode = Adapter->pwrctrlpriv.bFwCurrentInPSMode;
		rtw_hal_get_hwreg(Adapter, HW_VAR_FWLPS_RF_ON, (uint8_t *)(&bFwPSAwake));
	}
#endif

#ifdef CONFIG_P2P_PS
	/*
	 * Fw is under p2p powersaving mode, driver should stop dynamic mechanism.
	 * modifed by thomas. 2011.06.11.
	 */
	if(Adapter->wdinfo.p2p_ps_mode)
		bFwPSAwake = _FALSE;
#endif

	if ((hw_init_completed == _TRUE) && ((!bFwCurrentInPSMode) && bFwPSAwake)) {
		/*
		 * Calculate Tx/Rx statistics.
		 */
		dm_CheckStatistics(Adapter);

		/*
		 * Dynamically switch RTS/CTS protection.
		 *
		 * dm_CheckProtection(Adapter);
		 */


	}

	/* ODM */
	if (hw_init_completed == _TRUE) {
		uint8_t	bLinked=_FALSE;

#ifdef CONFIG_DISABLE_ODM
		pHalData->odmpriv.SupportAbility = 0;
#endif

		if(rtw_linked_check(Adapter))
			bLinked = _TRUE;


		ODM_CmnInfoUpdate(&pHalData->odmpriv ,ODM_CMNINFO_LINK, bLinked);
		ODM_DMWatchdog(&pHalData->odmpriv);

	}

skip_dm:

	/*
	 * Check GPIO to determine current RF on/off and Pbc status.
	 * Check Hardware Radio ON/OFF or not
	 */
	{
		/* temp removed */
		dm_CheckPbcGPIO(Adapter);
	}
	return;
}

void rtl8812_init_dm_priv(IN PADAPTER Adapter)
{
	PHAL_DATA_TYPE	pHalData = GET_HAL_DATA(Adapter);
	struct dm_priv	*pdmpriv = &pHalData->dmpriv;
	PDM_ODM_T 		podmpriv = &pHalData->odmpriv;
	memset(pdmpriv, 0, sizeof(struct dm_priv));

	/* _rtw_spinlock_init(&(pHalData->odm_stainfo_lock)); */

	Init_ODM_ComInfo_8812(Adapter);
#ifdef CONFIG_SW_ANTENNA_DIVERSITY
	/*
	 * _init_timer(&(pdmpriv->SwAntennaSwitchTimer),  Adapter->ndev , odm_SW_AntennaSwitchCallback, Adapter);
	 */
	ODM_InitAllTimers(podmpriv );
#endif
	ODM_InitDebugSetting(podmpriv);

	Adapter->registrypriv.RegEnableTxPowerLimit = 0;
	Adapter->registrypriv.RegPowerBase = 14;
	Adapter->registrypriv.RegTxPwrLimit = 0xFFFFFFFF;
	Adapter->registrypriv.TxBBSwing_2G = 0xFF;
	Adapter->registrypriv.TxBBSwing_5G = 0xFF;
	Adapter->registrypriv.bEn_RFE = 0;
	Adapter->registrypriv.RFE_Type = 64;
	pHalData->RegRFPathS1 = 0;
	pHalData->TxPwrInPercentage = TX_PWR_PERCENTAGE_3;
}

void rtl8812_deinit_dm_priv(IN PADAPTER Adapter)
{
	PHAL_DATA_TYPE	pHalData = GET_HAL_DATA(Adapter);
	struct dm_priv	*pdmpriv = &pHalData->dmpriv;
	PDM_ODM_T 		podmpriv = &pHalData->odmpriv;
	/* _rtw_spinlock_free(&pHalData->odm_stainfo_lock); */
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
void AntDivCompare8812(PADAPTER Adapter, WLAN_BSSID_EX *dst, WLAN_BSSID_EX *src)
{
	/* PADAPTER Adapter = pDM_Odm->Adapter ; */

	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(Adapter);
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
uint8_t AntDivBeforeLink8812(PADAPTER Adapter )
{

	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(Adapter);
	PDM_ODM_T 	pDM_Odm =&pHalData->odmpriv;
	SWAT_T		*pDM_SWAT_Table = &pDM_Odm->DM_SWAT_Table;
	struct mlme_priv	*pmlmepriv = &(Adapter->mlmepriv);

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
		 * PHY_SetBBReg(Adapter, rFPGA0_XA_RFInterfaceOE, 0x300, pDM_SWAT_Table->CurAntenna);
		*/
		rtw_antenna_select_cmd(Adapter, pDM_SWAT_Table->CurAntenna, _FALSE);
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

