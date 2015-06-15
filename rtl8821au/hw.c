#include "hw.h"

void rtl8821au_init_beacon_parameters(struct rtl_priv *rtlpriv)
{
	 struct _rtw_hal	*pHalData = GET_HAL_DATA(rtlpriv);

	rtl_write_word(rtlpriv, REG_BCN_CTRL, 0x1010);

	/* TODO: Remove these magic number */
	rtl_write_word(rtlpriv, REG_TBTT_PROHIBIT, 0x6404);		/* ms */
	rtl_write_byte(rtlpriv, REG_DRVERLYINT, DRIVER_EARLY_INT_TIME_8812);	/* 5ms */
	rtl_write_byte(rtlpriv, REG_BCNDMATIM, BCN_DMA_ATIME_INT_TIME_8812); 	/* 2ms */

	/*
	 *  Suggested by designer timchen. Change beacon AIFS to the largest number
	 *  beacause test chip does not contension before sending beacon. by tynli. 2009.11.03
	 */
	rtl_write_word(rtlpriv, REG_BCNTCFG, 0x660F);

	pHalData->RegBcnCtrlVal = rtl_read_byte(rtlpriv, REG_BCN_CTRL);
	pHalData->RegTxPause = rtl_read_byte(rtlpriv, REG_TXPAUSE);
	pHalData->RegFwHwTxQCtrl = rtl_read_byte(rtlpriv, REG_FWHW_TXQ_CTRL+2);
	pHalData->RegReg542 = rtl_read_byte(rtlpriv, REG_TBTT_PROHIBIT+2);
	pHalData->RegCR_1 = rtl_read_byte(rtlpriv, REG_CR+1);
}

static void _rtl8821au_stop_tx_beacon(struct rtl_priv *rtlpriv)
{
	 struct _rtw_hal *pHalData = GET_HAL_DATA(rtlpriv);

	rtl_write_byte(rtlpriv, REG_FWHW_TXQ_CTRL+2, (pHalData->RegFwHwTxQCtrl) & (~BIT6));
	pHalData->RegFwHwTxQCtrl &= (~BIT6);
	rtl_write_byte(rtlpriv, REG_TBTT_PROHIBIT+1, 0x64);
	pHalData->RegReg542 &= ~(BIT0);
	rtl_write_byte(rtlpriv, REG_TBTT_PROHIBIT+2, pHalData->RegReg542);

	 /* todo: CheckFwRsvdPageContent(rtlpriv);  // 2010.06.23. Added by tynli. */
}

static void  _rtl8821au_resume_tx_beacon(struct rtl_priv *rtlpriv)
{
	 struct _rtw_hal *pHalData = GET_HAL_DATA(rtlpriv);

	/*
	 * 2010.03.01. Marked by tynli. No need to call workitem beacause we record the value
	 * which should be read from register to a global variable.
	 */

	rtl_write_byte(rtlpriv, REG_FWHW_TXQ_CTRL+2, (pHalData->RegFwHwTxQCtrl) | BIT6);
	pHalData->RegFwHwTxQCtrl |= BIT6;
	rtl_write_byte(rtlpriv, REG_TBTT_PROHIBIT+1, 0xff);
	pHalData->RegReg542 |= BIT0;
	rtl_write_byte(rtlpriv, REG_TBTT_PROHIBIT+2, pHalData->RegReg542);
}

static void _BeaconFunctionEnable(struct rtl_priv *rtlpriv, BOOLEAN Enable,
	BOOLEAN	Linked)
{
	rtl_write_byte(rtlpriv, REG_BCN_CTRL, (BIT4 | BIT3 | BIT1));
	/*
	 * SetBcnCtrlReg(rtlpriv, (BIT4 | BIT3 | BIT1), 0x00);
	 * RT_TRACE(COMP_BEACON, DBG_LOUD, ("_BeaconFunctionEnable 0x550 0x%x\n", rtl_read_byte(rtlpriv, 0x550)));
	 */

	rtl_write_byte(rtlpriv, REG_RD_CTRL+1, 0x6F);
}

void rtl8821au_set_beacon_related_registers(struct rtl_priv *rtlpriv)
{
	uint32_t	value32;
	 struct _rtw_hal	*pHalData = GET_HAL_DATA(rtlpriv);
	struct mlme_ext_priv	*pmlmeext = &(rtlpriv->mlmeextpriv);
	struct mlme_ext_info	*pmlmeinfo = &(pmlmeext->mlmext_info);
	uint32_t bcn_ctrl_reg 			= REG_BCN_CTRL;
	/* reset TSF, enable update TSF, correcting TSF On Beacon */

	/*
	 * REG_BCN_INTERVAL
	 * REG_BCNDMATIM
	 * REG_ATIMWND
	 * REG_TBTT_PROHIBIT
	 * REG_DRVERLYINT
	 * REG_BCN_MAX_ERR
	 * REG_BCNTCFG //(0x510)
	 * REG_DUAL_TSF_RST
	 * REG_BCN_CTRL //(0x550)
	 */

	/* BCN interval */
	rtl_write_word(rtlpriv, REG_BCN_INTERVAL, pmlmeinfo->bcn_interval);
	rtl_write_byte(rtlpriv, REG_ATIMWND, 0x02);	/* 2ms */

	rtl8821au_init_beacon_parameters(rtlpriv);

	rtl_write_byte(rtlpriv, REG_SLOT, 0x09);

	value32 = rtl_read_dword(rtlpriv, REG_TCR);
	value32 &= ~TSFRST;
	rtl_write_dword(rtlpriv,  REG_TCR, value32);

	value32 |= TSFRST;
	rtl_write_dword(rtlpriv, REG_TCR, value32);

	/* NOTE: Fix test chip's bug (about contention windows's randomness) */
	rtl_write_byte(rtlpriv,  REG_RXTSF_OFFSET_CCK, 0x50);
	rtl_write_byte(rtlpriv, REG_RXTSF_OFFSET_OFDM, 0x50);

	_BeaconFunctionEnable(rtlpriv, _TRUE, _TRUE);

	_rtl8821au_resume_tx_beacon(rtlpriv);

	/* rtl_write_byte(rtlpriv, 0x422, rtl_read_byte(rtlpriv, 0x422)|BIT(6)); */

	/* rtl_write_byte(rtlpriv, 0x541, 0xff); */

	/* rtl_write_byte(rtlpriv, 0x542, rtl_read_byte(rtlpriv, 0x541)|BIT(0)); */

	rtl_write_byte(rtlpriv, bcn_ctrl_reg, rtl_read_byte(rtlpriv, bcn_ctrl_reg)|BIT(1));

}

static void hw_var_set_opmode(struct rtl_priv *rtlpriv, uint8_t variable, uint8_t *val)
{
	struct rtl_hal *rtlhal = rtl_hal(rtlpriv);
	uint8_t	val8;
	uint8_t	mode = *((uint8_t *)val);

	{
		/* disable Port0 TSF update */
		rtl_write_byte(rtlpriv, REG_BCN_CTRL, rtl_read_byte(rtlpriv, REG_BCN_CTRL)|DIS_TSF_UDT);

		/*  set net_type */
		val8 = rtl_read_byte(rtlpriv, MSR)&0x0c;
		val8 |= mode;
		rtl_write_byte(rtlpriv, MSR, val8);

		dev_info(&(rtlpriv->ndev->dev), "%s()-%d mode = %d\n", __FUNCTION__, __LINE__, mode);

		if ((mode == _HW_STATE_STATION_) || (mode == _HW_STATE_NOLINK_)) {
			_rtl8821au_stop_tx_beacon(rtlpriv);

			rtl_write_byte(rtlpriv, REG_BCN_CTRL, 0x19);		/* disable atim wnd */
			/* rtl_write_byte(rtlpriv,REG_BCN_CTRL, 0x18); */
		} else if ((mode == _HW_STATE_ADHOC_) /*|| (mode == _HW_STATE_AP_)*/ ) {
			_rtl8821au_resume_tx_beacon(rtlpriv);
			rtl_write_byte(rtlpriv, REG_BCN_CTRL, 0x1a);
		} else if (mode == _HW_STATE_AP_) {
			_rtl8821au_resume_tx_beacon(rtlpriv);

			rtl_write_byte(rtlpriv, REG_BCN_CTRL, 0x12);

			/* Set RCR */
			rtl_write_dword(rtlpriv, REG_RCR, 0x7000208e);	/* CBSSID_DATA must set to 0,reject ICV_ERR packet */
			/* enable to rx data frame */
			rtl_write_word(rtlpriv, REG_RXFLTMAP2, 0xFFFF);
			/* enable to rx ps-poll */
			rtl_write_word(rtlpriv, REG_RXFLTMAP1, 0x0400);

			/* Beacon Control related register for first time */
			rtl_write_byte(rtlpriv, REG_BCNDMATIM, 0x02); /* 2ms */

			/* rtl_write_byte(rtlpriv, REG_BCN_MAX_ERR, 0xFF); */
			rtl_write_byte(rtlpriv, REG_ATIMWND, 0x0a); 	/* 10ms */
			rtl_write_word(rtlpriv, REG_BCNTCFG, 0x00);
			rtl_write_word(rtlpriv, REG_TBTT_PROHIBIT, 0xff04);
			rtl_write_word(rtlpriv, REG_TSFTR_SYN_OFFSET, 0x7fff);	/* +32767 (~32ms) */

			/* reset TSF */
			rtl_write_byte(rtlpriv, REG_DUAL_TSF_RST, BIT(0));

			/*
			 * enable BCN0 Function for if1
			 * don't enable update TSF0 for if1 (due to TSF update when beacon/probe rsp are received)
			 */
			rtl_write_byte(rtlpriv, REG_BCN_CTRL, (DIS_TSF_UDT|EN_BCN_FUNCTION | EN_TXBCN_RPT|DIS_BCNQ_SUB));

			if (IS_HARDWARE_TYPE_8821(rtlhal)) {
				/*  select BCN on port 0 */
				rtl_write_byte(rtlpriv, REG_CCK_CHECK_8812,	rtl_read_byte(rtlpriv, REG_CCK_CHECK_8812)&(~BIT(5)));
			}


			/* dis BCN1 ATIM  WND if if2 is station */
			rtl_write_byte(rtlpriv, REG_BCN_CTRL_1, rtl_read_byte(rtlpriv, REG_BCN_CTRL_1)|DIS_ATIM);
		}
	}

}

static void hw_var_set_bcn_func(struct rtl_priv *rtlpriv, uint8_t variable, uint8_t *val)
{
	if (*((uint8_t *) val))
		rtl_write_byte(rtlpriv, REG_BCN_CTRL, (EN_BCN_FUNCTION | EN_TXBCN_RPT));
	else {
		u8 tmp;
		
		tmp = rtl_read_byte(rtlpriv, REG_BCN_CTRL);
		tmp &= (~(EN_BCN_FUNCTION | EN_TXBCN_RPT));
		rtl_write_byte(rtlpriv, REG_BCN_CTRL, tmp);
	}
}

static void hw_var_set_mlme_sitesurvey(struct rtl_priv *rtlpriv, uint8_t variable, uint8_t *val)
{
	uint32_t value_rcr, rcr_clear_bit, reg_bcn_ctl;
	u16 value_rxfltmap2;
	 struct _rtw_hal *pHalData = GET_HAL_DATA(rtlpriv);
	struct mlme_priv *pmlmepriv = &(rtlpriv->mlmepriv);

		reg_bcn_ctl = REG_BCN_CTRL;

	rcr_clear_bit = RCR_CBSSID_BCN;

	/* config RCR to receive different BSSID & not to receive data frame */
	value_rxfltmap2 = 0;

	if ((check_fwstate(pmlmepriv, WIFI_AP_STATE) == _TRUE)) {
		rcr_clear_bit = RCR_CBSSID_BCN;
	}

	value_rcr = rtl_read_dword(rtlpriv, REG_RCR);

	if (*((uint8_t *) val)) {
		/* under sitesurvey */

		value_rcr &= ~(rcr_clear_bit);
		rtl_write_dword(rtlpriv, REG_RCR, value_rcr);

		rtl_write_word(rtlpriv, REG_RXFLTMAP2, value_rxfltmap2);

		if (check_fwstate(pmlmepriv, WIFI_STATION_STATE | WIFI_ADHOC_STATE | WIFI_ADHOC_MASTER_STATE)) {
			/* disable update TSF */
			rtl_write_byte(rtlpriv, reg_bcn_ctl, rtl_read_byte(rtlpriv, reg_bcn_ctl)|DIS_TSF_UDT);
		}

		/* Save orignal RRSR setting. */
		pHalData->RegRRSR = rtl_read_word(rtlpriv, REG_RRSR);

	} else {
		/* sitesurvey done */

		if (check_fwstate(pmlmepriv, (_FW_LINKED | WIFI_AP_STATE))) {
			/* enable to rx data frame */
			rtl_write_word(rtlpriv, REG_RXFLTMAP2, 0xFFFF);
		}

		if (check_fwstate(pmlmepriv, WIFI_STATION_STATE | WIFI_ADHOC_STATE | WIFI_ADHOC_MASTER_STATE)) {
			/* enable update TSF */
			rtl_write_byte(rtlpriv, reg_bcn_ctl, rtl_read_byte(rtlpriv, reg_bcn_ctl)&(~(DIS_TSF_UDT)));
		}

		value_rcr |= rcr_clear_bit;
		rtl_write_dword(rtlpriv, REG_RCR, value_rcr);

		/* Restore orignal RRSR setting. */
		rtl_write_word(rtlpriv, REG_RRSR, pHalData->RegRRSR);

	}
}

static void Hal_PatchwithJaguar_8812(struct rtl_priv *rtlpriv, RT_MEDIA_STATUS	MediaStatus)
{
	struct mlme_ext_priv	*pmlmeext = &(rtlpriv->mlmeextpriv);
	struct mlme_ext_info	*pmlmeinfo = &(pmlmeext->mlmext_info);

	if ((MediaStatus == RT_MEDIA_CONNECT)
	  && (pmlmeinfo->assoc_AP_vendor == HT_IOT_PEER_REALTEK_JAGUAR_BCUTAP)) {
		rtl_write_byte(rtlpriv, rVhtlen_Use_Lsig_Jaguar, 0x1);
		rtl_write_byte(rtlpriv, REG_TCR+3, BIT2);
	} else {
		rtl_write_byte(rtlpriv, rVhtlen_Use_Lsig_Jaguar, 0x3F);
		rtl_write_byte(rtlpriv, REG_TCR+3, BIT0|BIT1|BIT2);
	}


	if ((MediaStatus == RT_MEDIA_CONNECT)
	   && ((pmlmeinfo->assoc_AP_vendor == HT_IOT_PEER_REALTEK_JAGUAR_BCUTAP)
	      || (pmlmeinfo->assoc_AP_vendor == HT_IOT_PEER_REALTEK_JAGUAR_CCUTAP))) {
		rtlpriv->phy.reg_837 |= BIT2;
		rtl_write_byte(rtlpriv, rBWIndication_Jaguar+3, rtlpriv->phy.reg_837);
	} else {
		rtlpriv->phy.reg_837 &= (~BIT2);
		rtl_write_byte(rtlpriv, rBWIndication_Jaguar+3, rtlpriv->phy.reg_837);
	}
}

void rtl8821au_set_hw_reg(struct rtl_priv *rtlpriv, u8 variable, u8 *pval)
{
	struct rtl_phy *rtlphy = &(rtlpriv->phy);
	struct rtl_hal *rtlhal = rtl_hal(rtlpriv);
	struct _rtw_hal *pHalData;
	struct dm_priv *pdmpriv;
	struct _rtw_dm *podmpriv;
	uint8_t val8;
	u16 val16;
	uint32_t val32;
	u8 idx;

	pHalData = GET_HAL_DATA(rtlpriv);
	pdmpriv = &pHalData->dmpriv;
	podmpriv = &pHalData->odmpriv;

	switch (variable) {
	case HW_VAR_RXDMA_AGG_PG_TH:
#ifdef CONFIG_USB_RX_AGGREGATION
		{
			/*uint8_t	threshold = *((uint8_t *)val);
			if ( threshold == 0)
			{
				threshold = pHalData->UsbRxAggPageCount;
			}
			rtl_write_byte(rtlpriv, REG_RXDMA_AGG_PG_TH, threshold);*/
		}
#endif
		break;
	case HW_VAR_MEDIA_STATUS:
		val8 = rtl_read_byte(rtlpriv, MSR) & 0x0c;
		val8 |= *pval;
		rtl_write_byte(rtlpriv, MSR, val8);
		break;

	case HW_VAR_MEDIA_STATUS1:
		val8 = rtl_read_byte(rtlpriv, MSR) & 0x03;
		val8 |= *pval << 2;
		rtl_write_byte(rtlpriv, MSR, val8);
		break;

	case HW_VAR_SET_OPMODE:
		hw_var_set_opmode(rtlpriv, variable, pval);
		break;

	case HW_VAR_MAC_ADDR:
		for (idx = 0 ; idx < 6; idx++) {
			rtl_write_byte(rtlpriv, (REG_MACID + idx), pval[idx]);
		}
		break;

	case HW_VAR_BSSID:
		for (idx = 0 ; idx < 6; idx++) {
			rtl_write_byte(rtlpriv, (REG_BSSID + idx), pval[idx]);
		}
		break;

	case HW_VAR_BASIC_RATE:
		{
			u16 BrateCfg = 0;
			uint8_t RateIndex = 0;

			/*
			 * 2007.01.16, by Emily
			 * Select RRSR (in Legacy-OFDM and CCK)
			 * For 8190, we select only 24M, 12M, 6M, 11M, 5.5M, 2M, and 1M from the Basic rate.
			 * We do not use other rates.
			 */
			HalSetBrateCfg(rtlpriv, pval, &BrateCfg);

			if (rtlhal->current_bandtype == BAND_ON_2_4G) {
				/*
				 * CCK 2M ACK should be disabled for some BCM and Atheros AP IOT
				 * because CCK 2M has poor TXEVM
				 * CCK 5.5M & 11M ACK should be enabled for better performance
				 */
				pHalData->BasicRateSet = BrateCfg = (BrateCfg | 0xd) & 0x15d;
				BrateCfg |= 0x01; /* default enable 1M ACK rate */
			} else { /* 5G */
				pHalData->BasicRateSet &= 0xFF0;
				BrateCfg |= 0x10; /* default enable 6M ACK rate */
			}
			/*
			 * DBG_8192C("HW_VAR_BASIC_RATE: BrateCfg(%#x)\n", BrateCfg);
			 */

			/* Set RRSR rate table. */
			rtl_write_byte(rtlpriv, REG_RRSR, BrateCfg&0xff);
			rtl_write_byte(rtlpriv, REG_RRSR+1, (BrateCfg>>8)&0xff);
			rtl_write_byte(rtlpriv, REG_RRSR+2, rtl_read_byte(rtlpriv, REG_RRSR+2)&0xf0);
		}
		break;

	case HW_VAR_TXPAUSE:
		rtl_write_byte(rtlpriv, REG_TXPAUSE, *pval);
		break;

	case HW_VAR_BCN_FUNC:
		hw_var_set_bcn_func(rtlpriv, variable, pval);
		break;

	case HW_VAR_CORRECT_TSF:
		{
			u64	tsf;
			struct mlme_ext_priv	*pmlmeext = &rtlpriv->mlmeextpriv;
			struct mlme_ext_info	*pmlmeinfo = &(pmlmeext->mlmext_info);

			/*
			 * tsf = pmlmeext->TSFValue - ((u32)pmlmeext->TSFValue % (pmlmeinfo->bcn_interval*1024)) -1024; //us
			 */
			tsf = pmlmeext->TSFValue - rtw_modular64(pmlmeext->TSFValue, (pmlmeinfo->bcn_interval*1024)) - 1024; /* us */

			if (((pmlmeinfo->state&0x03) == WIFI_FW_ADHOC_STATE)
			   || ((pmlmeinfo->state&0x03) == WIFI_FW_AP_STATE)) {
				/*
				 * pHalData->RegTxPause |= STOP_BCNQ;BIT(6)
				 * rtl_write_byte(rtlpriv, REG_TXPAUSE, (rtl_read_byte(rtlpriv, REG_TXPAUSE)|BIT(6)));
				 */
				_rtl8821au_stop_tx_beacon(rtlpriv);
			}

			/* disable related TSF function */
			rtl_write_byte(rtlpriv, REG_BCN_CTRL, rtl_read_byte(rtlpriv, REG_BCN_CTRL)&(~BIT(3)));

			rtl_write_dword(rtlpriv, REG_TSFTR, tsf);
			rtl_write_dword(rtlpriv, REG_TSFTR+4, tsf>>32);

			/* enable related TSF function */
			rtl_write_byte(rtlpriv, REG_BCN_CTRL, rtl_read_byte(rtlpriv, REG_BCN_CTRL)|BIT(3));


			if (((pmlmeinfo->state&0x03) == WIFI_FW_ADHOC_STATE)
			   || ((pmlmeinfo->state&0x03) == WIFI_FW_AP_STATE)) {
				/*
				 * pHalData->RegTxPause  &= (~STOP_BCNQ);
				 * rtl_write_byte(rtlpriv, REG_TXPAUSE, (rtl_read_byte(rtlpriv, REG_TXPAUSE)&(~BIT(6))));
				 */
				_rtl8821au_resume_tx_beacon(rtlpriv);
			}
		}
		break;

	case HW_VAR_CHECK_BSSID:
		val32 = rtl_read_dword(rtlpriv, REG_RCR);
		if (*pval)
			val32 |= RCR_CBSSID_DATA|RCR_CBSSID_BCN;
		else
			val32 &= ~(RCR_CBSSID_DATA|RCR_CBSSID_BCN);
		rtl_write_dword(rtlpriv, REG_RCR, val32);
		break;

	case HW_VAR_MLME_DISCONNECT:
		{
			/* Set RCR to not to receive data frame when NO LINK state
			 * val32 = rtl_read_dword(rtlpriv, REG_RCR);
			 * val32 &= ~RCR_ADF;
			 * rtl_write_dword(rtlpriv, REG_RCR, val32);
			 */

			 /* reject all data frames */
			rtl_write_word(rtlpriv, REG_RXFLTMAP2, 0x00);

			/* reset TSF */
			val8 = BIT(0) | BIT(1);
			rtl_write_byte(rtlpriv, REG_DUAL_TSF_RST, val8);

			/* disable update TSF */
			val8 = rtl_read_byte(rtlpriv, REG_BCN_CTRL);
			val8 |= BIT(4);
			rtl_write_byte(rtlpriv, REG_BCN_CTRL, val8);
		}
		break;

	case HW_VAR_MLME_SITESURVEY:
		hw_var_set_mlme_sitesurvey(rtlpriv, variable,  pval);

		break;

	case HW_VAR_MLME_JOIN:
		{
			uint8_t RetryLimit = 0x30;
			uint8_t type = *(uint8_t *)pval;
			struct mlme_priv *pmlmepriv = &rtlpriv->mlmepriv;
			EEPROM_EFUSE_PRIV *pEEPROM = GET_EEPROM_EFUSE_PRIV(rtlpriv);

			if (type == 0) { 	/* prepare to join  */
				/*
				 * enable to rx data frame.Accept all data frame
				 * rtl_write_dword(rtlpriv, REG_RCR, rtl_read_dword(rtlpriv, REG_RCR)|RCR_ADF);
				 */
				rtl_write_word(rtlpriv, REG_RXFLTMAP2, 0xFFFF);

				val32 = rtl_read_dword(rtlpriv, REG_RCR);
				if (rtlpriv->in_cta_test)
					val32 &= ~(RCR_CBSSID_DATA | RCR_CBSSID_BCN);/* | RCR_ADF */
				else
					val32 |= RCR_CBSSID_DATA|RCR_CBSSID_BCN;
				rtl_write_dword(rtlpriv, REG_RCR, val32);

				if (check_fwstate(pmlmepriv, WIFI_STATION_STATE) == _TRUE) {
					/* ULLI removed
					 * RetryLimit = (pEEPROM->CustomerID == RT_CID_CCX) ? 7 : 48;
					 */

					RetryLimit = 48;
				} else { /* Ad-hoc Mode */
					RetryLimit = 0x7;
				}

				rtlphy->need_iqk = true;
			} else if (type == 1) { /* joinbss_event call back when join res < 0  */

				rtl_write_word(rtlpriv, REG_RXFLTMAP2, 0x00);
			} else if (type == 2) { /* sta add event call back */
				/* enable update TSF */
				val8 = rtl_read_byte(rtlpriv, REG_BCN_CTRL);
				val8 &= ~BIT(4);
				rtl_write_byte(rtlpriv, REG_BCN_CTRL, val8);

				if (check_fwstate(pmlmepriv, WIFI_ADHOC_STATE|WIFI_ADHOC_MASTER_STATE)) {
					RetryLimit = 0x7;
				}
			}

			val16 = RetryLimit << RETRY_LIMIT_SHORT_SHIFT | RetryLimit << RETRY_LIMIT_LONG_SHIFT;
			rtl_write_word(rtlpriv, REG_RL, val16);
		}

		break;

	case HW_VAR_ON_RCR_AM:
		val32 = rtl_read_dword(rtlpriv, REG_RCR);
		val32 |= RCR_AM;
		rtl_write_dword(rtlpriv, REG_RCR, val32);
		dev_info(&(rtlpriv->ndev->dev), "%s, %d, RCR= %x\n", __FUNCTION__, __LINE__, rtl_read_dword(rtlpriv, REG_RCR));
		break;

	case HW_VAR_OFF_RCR_AM:
		val32 = rtl_read_dword(rtlpriv, REG_RCR);
		val32 &= ~RCR_AM;
		rtl_write_dword(rtlpriv, REG_RCR, val32);
		dev_info(&(rtlpriv->ndev->dev), "%s, %d, RCR= %x\n", __FUNCTION__, __LINE__, rtl_read_dword(rtlpriv, REG_RCR));
		break;

	case HW_VAR_BEACON_INTERVAL:
		rtl_write_word(rtlpriv, REG_BCN_INTERVAL, *(u16 *)pval);
		break;

	case HW_VAR_SLOT_TIME:
		rtl_write_byte(rtlpriv, REG_SLOT, *pval);
		break;

	case HW_VAR_RESP_SIFS:
		/*
		 * SIFS_Timer = 0x0a0a0808;
		 * RESP_SIFS for CCK
		 */
		rtl_write_byte(rtlpriv, REG_RESP_SIFS_CCK, pval[0]); 	/* SIFS_T2T_CCK (0x08) */
		rtl_write_byte(rtlpriv, REG_RESP_SIFS_CCK+1, pval[1]); 	/* SIFS_R2T_CCK(0x08) */
		/*  RESP_SIFS for OFDM */
		rtl_write_byte(rtlpriv, REG_RESP_SIFS_OFDM, pval[2]); 	/* SIFS_T2T_OFDM (0x0a) */
		rtl_write_byte(rtlpriv, REG_RESP_SIFS_OFDM+1, pval[3]); 	/* SIFS_R2T_OFDM(0x0a) */
		break;

	case HW_VAR_ACK_PREAMBLE:
		{
			uint8_t bShortPreamble = *pval;

			/*  Joseph marked out for Netgear 3500 TKIP channel 7 issue.(Temporarily) */
			val8 = (rtlpriv->mac80211.cur_40_prime_sc) << 5;
			if (bShortPreamble)
				val8 |= 0x80;
			rtl_write_byte(rtlpriv, REG_RRSR+2, val8);
		}
		break;

	case HW_VAR_SEC_CFG:
		val8 = *pval;
		rtl_write_byte(rtlpriv, REG_SECCFG, val8);
		break;

	case HW_VAR_CAM_EMPTY_ENTRY:
		{
			uint8_t ucIndex = *pval;
			uint8_t i;
			uint32_t	ulCommand = 0;
			uint32_t	ulContent = 0;
			uint32_t	ulEncAlgo = CAM_AES;

			for (i = 0; i < CAM_CONTENT_COUNT; i++) {
				/* filled id in CAM config 2 byte */
				if (i == 0) {
					ulContent |= (ucIndex & 0x03) | ((u16)(ulEncAlgo)<<2);
					/* ulContent |= CAM_VALID; */
				} else 	{
					ulContent = 0;
				}
				/*  polling bit, and No Write enable, and address */
				ulCommand = CAM_CONTENT_COUNT*ucIndex+i;
				ulCommand = ulCommand | CAM_POLLINIG | CAM_WRITE;
				/* write content 0 is equall to mark invalid */
				rtl_write_dword(rtlpriv, WCAMI, ulContent);  /* delay_ms(40); */
				rtl_write_dword(rtlpriv, RWCAM, ulCommand);  /* delay_ms(40); */
			}
		}
		break;

	case HW_VAR_CAM_INVALID_ALL:
		val32 = BIT(31) | BIT(30);
		rtl_write_dword(rtlpriv, RWCAM, val32);
		break;

	case HW_VAR_CAM_WRITE:
		{
			uint32_t cmd;
			uint32_t *cam_val = (u32 *)pval;

			rtl_write_dword(rtlpriv, WCAMI, cam_val[0]);

			cmd = CAM_POLLINIG | CAM_WRITE | cam_val[1];
			rtl_write_dword(rtlpriv, RWCAM, cmd);
		}
		break;

	case HW_VAR_CAM_READ:
		break;

	case HW_VAR_AC_PARAM_VO:
		rtl_write_dword(rtlpriv, REG_EDCA_VO_PARAM, *(u32 *)pval);
		break;

	case HW_VAR_AC_PARAM_VI:
		rtl_write_dword(rtlpriv, REG_EDCA_VI_PARAM, *(u32 *)pval);
		break;

	case HW_VAR_AC_PARAM_BE:
		pHalData->AcParam_BE = *(u32 *)pval;
		rtl_write_dword(rtlpriv, REG_EDCA_BE_PARAM, *(u32 *)pval);
		break;

	case HW_VAR_AC_PARAM_BK:
		rtl_write_dword(rtlpriv, REG_EDCA_BK_PARAM, *(u32 *)pval);
		break;

	case HW_VAR_ACM_CTRL:
		{
			uint8_t acm_ctrl;
			uint8_t AcmCtrl;

			acm_ctrl = *(uint8_t *)pval;
			AcmCtrl = rtl_read_byte(rtlpriv, REG_ACMHWCTRL);

			if (acm_ctrl > 1)
				AcmCtrl = AcmCtrl | 0x1;

			if (acm_ctrl & BIT(3))
				AcmCtrl |= AcmHw_VoqEn;
			else
				AcmCtrl &= (~AcmHw_VoqEn);

			if (acm_ctrl & BIT(2))
				AcmCtrl |= AcmHw_ViqEn;
			else
				AcmCtrl &= (~AcmHw_ViqEn);

			if (acm_ctrl & BIT(1))
				AcmCtrl |= AcmHw_BeqEn;
			else
				AcmCtrl &= (~AcmHw_BeqEn);

			dev_info(&(rtlpriv->ndev->dev), "[HW_VAR_ACM_CTRL] Write 0x%X\n", AcmCtrl);
			rtl_write_byte(rtlpriv, REG_ACMHWCTRL, AcmCtrl);
		}
		break;

	case HW_VAR_AMPDU_MIN_SPACE:
		pHalData->AMPDUDensity = *(uint8_t *)pval;
		break;

	case HW_VAR_AMPDU_FACTOR:
		{
			uint32_t	AMPDULen = *(uint8_t *)pval;

			if (IS_HARDWARE_TYPE_8812(rtlhal)) {
				if (AMPDULen < VHT_AGG_SIZE_128K)
					AMPDULen = (0x2000 << *(uint8_t *)pval) - 1;
				else
					AMPDULen = 0x1ffff;
			} else if (IS_HARDWARE_TYPE_8821(rtlhal)) {
				if (AMPDULen < HT_AGG_SIZE_64K)
					AMPDULen = (0x2000 << *(uint8_t *)pval) - 1;
				else
					AMPDULen = 0xffff;
			}
			AMPDULen |= BIT(31);
			rtl_write_dword(rtlpriv, REG_AMPDU_MAX_LENGTH_8812, AMPDULen);
		}
		break;
	case HW_VAR_H2C_FW_PWRMODE:
		{
			uint8_t psmode = *pval;

			/*
			 * Forece leave RF low power mode for 1T1R to prevent conficting setting in Fw power
			 * saving sequence. 2010.06.07. Added by tynli. Suggested by SD3 yschang.
			 */
			rtl8812au_set_fw_pwrmode_cmd(rtlpriv, psmode);
		}
		break;

	case HW_VAR_H2C_FW_JOINBSSRPT:
		rtl8812_set_FwJoinBssReport_cmd(rtlpriv, *pval);
		break;

	case HW_VAR_INITIAL_GAIN:	/* ULLI not in rtlwifi */
		{
			struct dig_t *dm_digtable = &(rtlpriv->dm_digtable);
			uint32_t rx_gain = *(u32 *)pval;

			if (rx_gain == 0xff) {		/* restore rx gain */
				ODM_Write_DIG(podmpriv, dm_digtable->BackupIGValue);
			} else {
				dm_digtable->BackupIGValue = dm_digtable->cur_igvalue;
				ODM_Write_DIG(podmpriv, rx_gain);
			}
		}
		break;


#if (RATE_ADAPTIVE_SUPPORT == 1)
	case HW_VAR_RPT_TIMER_SETTING:
		{
			val16 = *(u16 *)pval;
			ODM_RA_Set_TxRPT_Time(podmpriv, val16);
		}
		break;
#endif

#ifdef CONFIG_SW_ANTENNA_DIVERSITY
	case HW_VAR_ANTENNA_DIVERSITY_LINK:
		/* SwAntDivRestAfterLink8192C(rtlpriv); */
		ODM_SwAntDivRestAfterLink(podmpriv);
		break;

	case HW_VAR_ANTENNA_DIVERSITY_SELECT:
		{
			uint8_t Optimum_antenna = *pval;
			uint8_t 	Ant;

			/*
			 * switch antenna to Optimum_antenna
			 * DBG_8192C("==> HW_VAR_ANTENNA_DIVERSITY_SELECT , Ant_(%s)\n",(Optimum_antenna==2)?"A":"B");
			 */
			if (pHalData->CurAntenna != Optimum_antenna) {
				Ant = (Optimum_antenna == 2) ? MAIN_ANT : AUX_ANT;
				ODM_UpdateRxIdleAnt_88E(podmpriv, Ant);

				pHalData->CurAntenna = Optimum_antenna;
				/*
				 * DBG_8192C("==> HW_VAR_ANTENNA_DIVERSITY_SELECT , Ant_(%s)\n",(Optimum_antenna==2)?"A":"B");
				 */
			}
		}
		break;
#endif

	case HW_VAR_EFUSE_USAGE:
		pHalData->EfuseUsedPercentage = *pval;
		break;

	case HW_VAR_EFUSE_BYTES:
		pHalData->EfuseUsedBytes = *(u16 *)pval;
		break;
	case HW_VAR_FIFO_CLEARN_UP:
		{
			struct pwrctrl_priv *pwrpriv;
			uint8_t trycnt = 100;

			pwrpriv = &rtlpriv->pwrctrlpriv;

			/* pause tx */
			rtl_write_byte(rtlpriv, REG_TXPAUSE, 0xff);

			/* keep sn */
			rtlpriv->xmitpriv.nqos_ssn = rtl_read_word(rtlpriv, REG_NQOS_SEQ);

			if (pwrpriv->bkeepfwalive != _TRUE) {
				/* RX DMA stop */
				val32 = rtl_read_dword(rtlpriv, REG_RXPKT_NUM);
				val32 |= RW_RELEASE_EN;
				rtl_write_dword(rtlpriv, REG_RXPKT_NUM, val32);
				do {
					val32 = rtl_read_dword(rtlpriv, REG_RXPKT_NUM);
					val32 &= RXDMA_IDLE;
					if (!val32)
						break;
				} while (trycnt--);
				if (trycnt == 0) {
					dev_info(&(rtlpriv->ndev->dev), "[HW_VAR_FIFO_CLEARN_UP] Stop RX DMA failed......\n");
				}

				/* RQPN Load 0 */
				rtl_write_word(rtlpriv, REG_RQPN_NPQ, 0x0);
				rtl_write_dword(rtlpriv, REG_RQPN, 0x80000000);
				mdelay(10);
			}
		}
		break;


#if (RATE_ADAPTIVE_SUPPORT == 1)
	case HW_VAR_TX_RPT_MAX_MACID:
		{
			uint8_t maxMacid = *pval;
			dev_info(&(rtlpriv->ndev->dev), "### MacID(%d),Set Max Tx RPT MID(%d)\n", maxMacid, maxMacid+1);
			rtl_write_byte(rtlpriv, REG_TX_RPT_CTRL+1, maxMacid+1);
		}
		break;
#endif

	case HW_VAR_H2C_MEDIA_STATUS_RPT:
		{
			struct mlme_priv *pmlmepriv = &rtlpriv->mlmepriv;
			RT_MEDIA_STATUS	mstatus = *(u16 *)pval & 0xFF;

			rtl8812_set_FwMediaStatus_cmd(rtlpriv, *(u16 *)pval);

			if (check_fwstate(pmlmepriv, WIFI_STATION_STATE))
				Hal_PatchwithJaguar_8812(rtlpriv, mstatus);
		}
		break;

	case HW_VAR_APFM_ON_MAC:
		pHalData->bMacPwrCtrlOn = *pval;
		dev_info(&(rtlpriv->ndev->dev), "%s: bMacPwrCtrlOn=%d\n", __FUNCTION__, pHalData->bMacPwrCtrlOn);
		break;

	case HW_VAR_NAV_UPPER:
		{
			uint32_t usNavUpper = *((u32 *)pval);

			if (usNavUpper > HAL_NAV_UPPER_UNIT * 0xFF) {
				dev_info(&(rtlpriv->ndev->dev), "%s: [HW_VAR_NAV_UPPER] set value(0x%08X us) is larger than (%d * 0xFF)!\n",
					__FUNCTION__, usNavUpper, HAL_NAV_UPPER_UNIT);
				break;
			}

			/*
			 *  The value of ((usNavUpper + HAL_NAV_UPPER_UNIT - 1) / HAL_NAV_UPPER_UNIT)
			 * is getting the upper integer.
			 */
			usNavUpper = (usNavUpper + HAL_NAV_UPPER_UNIT - 1) / HAL_NAV_UPPER_UNIT;
			rtl_write_byte(rtlpriv, REG_NAV_UPPER, (uint8_t)usNavUpper);
		}
		break;

	case HW_VAR_BCN_VALID:
		{
			/*
			 * BCN_VALID, BIT16 of REG_TDECTRL = BIT0 of REG_TDECTRL+2, write 1 to clear, Clear by sw
			 */
			val8 = rtl_read_byte(rtlpriv, REG_TDECTRL+2);
			val8 |= BIT(0);
			rtl_write_byte(rtlpriv, REG_TDECTRL+2, val8);
		}
		break;

	case HW_VAR_DL_BCN_SEL:
		{
			/* SW_BCN_SEL - Port0 */
			val8 = rtl_read_byte(rtlpriv, REG_TDECTRL1_8812+2);
			val8 &= ~BIT(4);
			rtl_write_byte(rtlpriv, REG_TDECTRL1_8812+2, val8);
		}
		break;

	case HW_VAR_WIRELESS_MODE:
		{
			uint8_t	R2T_SIFS = 0, SIFS_Timer = 0;
			uint8_t	wireless_mode = *pval;

			if ((wireless_mode == WIRELESS_11BG) || (wireless_mode == WIRELESS_11G))
				SIFS_Timer = 0xa;
			else
				SIFS_Timer = 0xe;

			/* SIFS for OFDM Data ACK */
			rtl_write_byte(rtlpriv, REG_SIFS_CTX+1, SIFS_Timer);
			/* SIFS for OFDM consecutive tx like CTS data! */
			rtl_write_byte(rtlpriv, REG_SIFS_TRX+1, SIFS_Timer);

			rtl_write_byte(rtlpriv, REG_SPEC_SIFS+1, SIFS_Timer);
			rtl_write_byte(rtlpriv, REG_MAC_SPEC_SIFS+1, SIFS_Timer);

			/* 20100719 Joseph: Revise SIFS setting due to Hardware register definition change. */
			rtl_write_byte(rtlpriv, REG_RESP_SIFS_OFDM+1, SIFS_Timer);
			rtl_write_byte(rtlpriv, REG_RESP_SIFS_OFDM, SIFS_Timer);

			/*
			 * Adjust R2T SIFS for IOT issue. Add by hpfan 2013.01.25
			 * Set R2T SIFS to 0x0a for Atheros IOT. Add by hpfan 2013.02.22
			 *
			 * Mac has 10 us delay so use 0xa value is enough.
			 */
			R2T_SIFS = 0xa;

			rtl_write_byte(rtlpriv, REG_RESP_SIFS_OFDM+1, R2T_SIFS);
		}
		break;

	default:
		dev_info(&(rtlpriv->ndev->dev), "%s: [WARNNING] variable(%d) not defined!\n", __FUNCTION__, variable);
		break;
	}
}


void rtl8821au_get_hw_reg(struct rtl_priv *rtlpriv, u8 variable,u8 *pval)
{
	struct _rtw_hal *pHalData;
	struct _rtw_dm *podmpriv;
	uint8_t val8;
	u16 val16;
	uint32_t val32;

	pHalData = GET_HAL_DATA(rtlpriv);
	podmpriv = &pHalData->odmpriv;

	switch (variable) {
	case HW_VAR_BASIC_RATE:
		*(u16 *)pval = pHalData->BasicRateSet;
		break;

	case HW_VAR_TXPAUSE:
		*pval = rtl_read_byte(rtlpriv, REG_TXPAUSE);
		break;

	case HW_VAR_BCN_VALID:
		{
			/* BCN_VALID, BIT16 of REG_TDECTRL = BIT0 of REG_TDECTRL+2 */
			val8 = rtl_read_byte(rtlpriv, REG_TDECTRL+2);
			*pval = (BIT(0) & val8) ? _TRUE:_FALSE;
		}
		break;

	case HW_VAR_FWLPS_RF_ON:
		/* When we halt NIC, we should check if FW LPS is leave. */
		if (rtlpriv->pwrctrlpriv.rf_pwrstate == rf_off) {
			/*
			 *  If it is in HW/SW Radio OFF or IPS state, we do not check Fw LPS Leave,
			 *  because Fw is unload.
			 */
			*pval = _TRUE;
		} else {
			uint32_t valRCR;
			valRCR = rtl_read_dword(rtlpriv, REG_RCR);
			valRCR &= 0x00070000;
			if (valRCR)
				*pval = _FALSE;
			else
				*pval = _TRUE;
		}

		break;

#ifdef CONFIG_ANTENNA_DIVERSITY
	case HW_VAR_CURRENT_ANTENNA:
		*pval = pHalData->CurAntenna;
		break;
#endif
	case HW_VAR_EFUSE_BYTES: /*  To get EFUE total used bytes, added by Roger, 2008.12.22. */
		*(u16 *)pval = pHalData->EfuseUsedBytes;
		break;

	case HW_VAR_APFM_ON_MAC:
		*pval = pHalData->bMacPwrCtrlOn;
		break;

	case HW_VAR_CHK_HI_QUEUE_EMPTY:
		val16 = rtl_read_word(rtlpriv, REG_TXPKT_EMPTY);
		*pval = (val16 & BIT(10)) ? _TRUE:_FALSE;
		break;

	default:
		dev_info(&(rtlpriv->ndev->dev), "%s: [WARNNING] variable(%d) not defined!\n", __FUNCTION__, variable);
		break;
	}
}

/*
 * These functions sets the media type register
 * AP, STA, ADHOC
 */

void Set_MSR(struct rtl_priv *rtlpriv, uint8_t type)
{
	rtw_hal_set_hwreg(rtlpriv, HW_VAR_MEDIA_STATUS, (uint8_t *)(&type));	
}

void rtl8821au_read_chip_version(struct rtl_priv *rtlpriv)
{
	uint32_t	value32;
	struct HAL_VERSION ChipVersion;
	struct _rtw_hal *pHalData;
	struct rtl_hal *rtlhal = rtl_hal(rtlpriv);

	pHalData = GET_HAL_DATA(rtlpriv);

	value32 = rtl_read_dword(rtlpriv, REG_SYS_CFG);
	dev_info(&(rtlpriv->ndev->dev), "%s SYS_CFG(0x%X)=0x%08x \n", __FUNCTION__, REG_SYS_CFG, value32);

	if (IS_HARDWARE_TYPE_8812(rtlhal))
		ChipVersion.ICType = CHIP_8812;
	else
		ChipVersion.ICType = CHIP_8821;

	ChipVersion.ChipType = ((value32 & RTL_ID) ? 0 : NORMAL_CHIP);

	if (rtlpriv->registrypriv.rf_config == RF_MAX_TYPE) {
		if (IS_HARDWARE_TYPE_8812(rtlhal))
			ChipVersion.RFType = RF_TYPE_2T2R;	/* RF_2T2R; */
		else
			ChipVersion.RFType = RF_TYPE_1T1R;	/*RF_1T1R; */
	}

	if (IS_HARDWARE_TYPE_8812(rtlhal))
		ChipVersion.VendorType = ((value32 & VENDOR_ID) ? CHIP_VENDOR_UMC : 0);
	else {
		uint32_t vendor;

		vendor = (value32 & EXT_VENDOR_ID) >> EXT_VENDOR_ID_SHIFT;
		switch (vendor) {
		case 2:
			vendor = CHIP_VENDOR_UMC;
			break;
		}
		ChipVersion.VendorType = vendor;
	}
	ChipVersion.CUTVersion = (value32 & CHIP_VER_RTL_MASK)>>CHIP_VER_RTL_SHIFT; /* IC version (CUT) */
	if (IS_HARDWARE_TYPE_8812(rtlhal))
		ChipVersion.CUTVersion += 1;
	ChipVersion.CUTVersion <<= 12;		/* ULLI shift for CUT */

	/* value32 = rtl_read_dword(rtlpriv, REG_GPIO_OUTSTS); */
	ChipVersion.ROMVer = 0;	/* ROM code version. */

	/* For multi-function consideration. Added by Roger, 2010.10.06. */
	value32 = rtl_read_dword(rtlpriv, REG_MULTI_FUNC_CTRL);
	pHalData->PolarityCtl = ((value32 & WL_HWPDN_SL) ? RT_POLARITY_HIGH_ACT : RT_POLARITY_LOW_ACT);

	memcpy(&(rtlpriv->VersionID), &ChipVersion, sizeof(struct HAL_VERSION));

	if (IS_1T2R(ChipVersion)) {
		rtlpriv->phy.rf_type = RF_1T2R;
		 rtlpriv->phy.num_total_rfpath = 2;
		dev_info(&(rtlpriv->ndev->dev), "==> RF_Type : 1T2R\n");
	} else if (IS_2T2R(ChipVersion)) {
		rtlpriv->phy.rf_type = RF_2T2R;
		 rtlpriv->phy.num_total_rfpath = 2;
		dev_info(&(rtlpriv->ndev->dev), "==> RF_Type : 2T2R\n");
	} else {
		rtlpriv->phy.rf_type = RF_1T1R;
		 rtlpriv->phy.num_total_rfpath = 1;
		dev_info(&(rtlpriv->ndev->dev), "==> RF_Type 1T1R\n");
	}

	dev_info(&(rtlpriv->ndev->dev), "RF_Type is %x!!\n", rtlpriv->phy.rf_type);
}


static int32_t _rtl8821au_llt_write(struct rtl_priv *rtlpriv, uint32_t address, uint32_t data)
{
	bool status = true;
	int32_t	count = 0;
	uint32_t value = _LLT_INIT_ADDR(address) | _LLT_INIT_DATA(data) | 
			 _LLT_OP(_LLT_WRITE_ACCESS);

	rtl_write_dword(rtlpriv, REG_LLT_INIT, value);

	/* polling */
	do {
		value = rtl_read_dword(rtlpriv, REG_LLT_INIT);
		if (_LLT_NO_ACTIVE == _LLT_OP_VALUE(value)) {
			break;
		}

		if (count > POLLING_LLT_THRESHOLD) {
			status = false;
			break;
		}
	} while (count++);

	return status;
}

int32_t  _rtl8821au_llt_table_init(struct rtl_priv *rtlpriv, uint8_t txpktbuf_bndy)
{
	bool status;
	uint32_t i;
	uint32_t Last_Entry_Of_TxPktBuf = LAST_ENTRY_OF_TX_PKT_BUFFER_8812;

	for (i = 0; i < (txpktbuf_bndy - 1); i++) {
		status = _rtl8821au_llt_write(rtlpriv, i, i + 1);
		if (!status)
			return status;
	}

	/* end of list */
	status = _rtl8821au_llt_write(rtlpriv, (txpktbuf_bndy - 1), 0xFF);
	if (_SUCCESS != status) {
		return status;
	}

	/*
	 * Make the other pages as ring buffer
	 * This ring buffer is used as beacon buffer if we config this MAC as two MAC transfer.
	 * Otherwise used as local loopback buffer.
	 */
	for (i = txpktbuf_bndy; i < Last_Entry_Of_TxPktBuf; i++) {
		status = _rtl8821au_llt_write(rtlpriv, i, (i + 1));
		if (!status)
			return status;
	}

	/*  Let last entry point to the start entry of ring buffer */
	status = _rtl8821au_llt_write(rtlpriv, Last_Entry_Of_TxPktBuf, txpktbuf_bndy);
	if (!status)
		return status;

	return true;
}
