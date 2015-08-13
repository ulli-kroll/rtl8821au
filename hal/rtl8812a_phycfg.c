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
#define _RTL8812A_PHYCFG_C_

#include <rtl8812a_hal.h>
#include <../rtl8821au/phy.h>
#include <../rtl8821au/dm.h>
#include <../rtl8821au/reg.h>
#include <../rtl8821au/rf.h>
#include <../wifi.h>

const char *const GLBwSrc[]={
	"CHANNEL_WIDTH_20",
	"CHANNEL_WIDTH_40",
	"CHANNEL_WIDTH_80",
	"CHANNEL_WIDTH_160",
	"CHANNEL_WIDTH_80_80"
};
#define		ENABLE_POWER_BY_RATE		1
#define		POWERINDEX_ARRAY_SIZE		48 //= cckRatesSize + ofdmRatesSize + htRates1TSize + htRates2TSize + vhtRates1TSize + vhtRates1TSize;

/* ---------------------Define local function prototype----------------------- */

/* ----------------------------Function Body---------------------------------- */

/*
 * 2. RF register R/W API
 */
/*
 * 3. Initial MAC/BB/RF config by reading MAC/BB/RF txt.
 */

void _rtl8812au_bb8812_config_1t(struct rtl_priv *rtlpriv)
{
	/* BB OFDM RX Path_A */
	rtl_set_bbreg(rtlpriv, rRxPath_Jaguar, bRxPath_Jaguar, 0x11);
	/* BB OFDM TX Path_A */
	rtl_set_bbreg(rtlpriv, rTxPath_Jaguar, bMaskLWord, 0x1111);
	/* BB CCK R/Rx Path_A */
	rtl_set_bbreg(rtlpriv, rCCK_RX_Jaguar, bCCK_RX_Jaguar, 0x0);
	/* MCS support */
	rtl_set_bbreg(rtlpriv, 0x8bc, 0xc0000060, 0x4);
	/* RF Path_B HSSI OFF */
	rtl_set_bbreg(rtlpriv, 0xe00, 0xf, 0x4);
	/* RF Path_B Power Down */
	rtl_set_bbreg(rtlpriv, 0xe90, bMaskDWord, 0);
	/* ADDA Path_B OFF */
	rtl_set_bbreg(rtlpriv, 0xe60, bMaskDWord, 0);
	rtl_set_bbreg(rtlpriv, 0xe64, bMaskDWord, 0);
}


int PHY_RFConfig8812(struct rtl_priv *rtlpriv)
{
	int rtStatus = _SUCCESS;

	if (rtlpriv->bSurpriseRemoved)
		return _FAIL;

	rtStatus = rtl8821au_phy_rf6052_config(rtlpriv);

	return rtStatus;
}


/* Ulli called in odm_RegConfig8812A.c and odm_RegConfig8821A.c */

/**************************************************************************************************************
 *   Description:
 *       The low-level interface to get the FINAL Tx Power Index , called  by both MP and Normal Driver.
 *
 *                                                                                    <20120830, Kordan>
 **************************************************************************************************************/

/**************************************************************************************************************
 *   Description:
 *       The low-level interface to set TxAGC , called by both MP and Normal Driver.
 *
 *                                                                                    <20120830, Kordan>
 **************************************************************************************************************/


static void PHY_GetTxPowerIndexByRateArray_8812A(struct rtl_priv *rtlpriv,
	uint8_t RFPath, enum CHANNEL_WIDTH BandWidth,
	uint8_t Channel, uint8_t *Rate, uint8_t *power_index,
	uint8_t	ArraySize)
{
	struct rtl_hal *rtlhal = rtl_hal(rtlpriv);
	struct _rtw_hal *pHalData = GET_HAL_DATA(rtlpriv);
	uint8_t i;
	for (i = 0; i < ArraySize; i++) {
		power_index[i] = _rtl8821au_get_txpower_index(rtlpriv, RFPath, Rate[i], BandWidth, Channel);
		if ((power_index[i] % 2 == 1) &&  !IS_NORMAL_CHIP(rtlhal->version))
			power_index[i] -= 1;
	}

}


/*
 * create new definition of PHY_SetTxPowerLevel8812 by YP.
 * Page revised on 20121106
 * the new way to set tx power by rate, NByte access, here N byte shall be 4 byte(DWord) or NByte(N>4) access. by page/YP, 20121106
 */

/* ULLI called in HalPhyRf8812A.c and HalPhyRf21A.c */

/* ULLI used in rtl8821au/dm.c */




/* <20130207, Kordan> The variales initialized here are used in odm_LNAPowerControl(). */
static void phy_InitRssiTRSW(struct rtl_priv *rtlpriv)
{
	struct rtl_hal	*rtlhal = rtl_hal(rtlpriv);

	struct _rtw_hal	*pHalData = GET_HAL_DATA(rtlpriv);
	struct _rtw_dm *	pDM_Odm = &pHalData->odmpriv;
	uint8_t 			channel = rtlpriv->phy.current_channel;

	if (rtlhal->rfe_type == 3){
		if (channel <= 14) {
			pDM_Odm->RSSI_TRSW_H    = 70;		/* Unit: percentage(%) */
			pDM_Odm->RSSI_TRSW_iso  = 25;
		} else if (36 <= channel && channel <= 64) {
			pDM_Odm->RSSI_TRSW_H   = 70;
			pDM_Odm->RSSI_TRSW_iso = 25;
		} else if (100 <= channel && channel <= 144) {
			pDM_Odm->RSSI_TRSW_H   = 80;
			pDM_Odm->RSSI_TRSW_iso = 35;
		} else if (149 <= channel) {
			pDM_Odm->RSSI_TRSW_H   = 75;
			pDM_Odm->RSSI_TRSW_iso = 30;
		}

		pDM_Odm->RSSI_TRSW_L = pDM_Odm->RSSI_TRSW_H - pDM_Odm->RSSI_TRSW_iso - 10;
	}
}

/*
 * Prototypes needed here, because functions are moved to rtl8821au/phy.c
 */

void rtl8812au_fixspur(struct rtl_priv *rtlpriv, enum CHANNEL_WIDTH Bandwidth,
	u8 Channel);

static void rtl8821au_phy_sw_chnl_callback(struct rtl_priv *rtlpriv)
{
	struct rtl_hal *rtlhal = rtl_hal(rtlpriv);

	uint8_t	path = 0;
	uint8_t	channel = rtlpriv->phy.current_channel;
	u32 data;

	if(phy_SwBand8812(rtlpriv, channel) == _FALSE)
		RT_TRACE(rtlpriv, COMP_ERR, DBG_LOUD, "error Chnl %d !\n", channel);

	/* DBG_871X("[BW:CHNL], phy_SwChnl8812(), switch to channel %d !!\n", channel); */

	/* fc_area */
	if (36 <= channel && channel <= 48)
		data = 0x494;
	else if (50 <= channel && channel <= 64)
		data = 0x453;
	else if (100 <= channel && channel <= 116)
		data = 0x452;
	else if (118 <= channel)
		data = 0x412;
	else
		data = 0x96a;

	rtl_set_bbreg(rtlpriv, rFc_area_Jaguar, 0x1ffe0000, data);

	for (path = 0; path <  rtlpriv->phy.num_total_rfpath; path++) {
		/* [2.4G] LC Tank */
		if (IS_VENDOR_8812A_TEST_CHIP(rtlhal->version)) {
			if (1 <= channel && channel <= 7)
				rtl_set_rfreg(rtlpriv, path, RF_TxLCTank_Jaguar, bLSSIWrite_data_Jaguar, 0x0017e);
			else if (8 <= channel && channel <= 14)
				rtl_set_rfreg(rtlpriv, path, RF_TxLCTank_Jaguar, bLSSIWrite_data_Jaguar, 0x0013e);
		}

		/* RF_MOD_AG */
		if (36 <= channel && channel <= 64)
			data = 0x101; //5'b00101);
		else if (100 <= channel && channel <= 140)
			data = 0x301; //5'b01101);
		else if (140 < channel)
			data = 0x501; //5'b10101);
		else
			data = 0x000; //5'b00000);

		rtl_set_rfreg(rtlpriv, path, RF_CHNLBW_Jaguar,
			      BIT18|BIT17|BIT16|BIT9|BIT8, data);

		/* <20121109, Kordan> A workaround for 8812A only. */
		rtl8812au_fixspur(rtlpriv, rtlpriv->phy.current_chan_bw, channel);

		rtl_set_rfreg(rtlpriv, path, RF_CHNLBW_Jaguar, MASKBYTE0, channel);

		/* <20130104, Kordan> APK for MP chip is done on initialization from folder. */
		if (IS_HARDWARE_TYPE_8811AU(rtlhal) &&
		    (!IS_NORMAL_CHIP(rtlhal->version)) && channel > 14 ) {
			/* <20121116, Kordan> For better result of APK. Asked by AlexWang. */
			if (36 <= channel && channel <= 64)
				rtl_set_rfreg(rtlpriv, path, RF_APK_Jaguar, bRFRegOffsetMask, 0x710E7);
			else if (100 <= channel && channel <= 140)
				rtl_set_rfreg(rtlpriv, path, RF_APK_Jaguar, bRFRegOffsetMask, 0x716E9);
			else
				rtl_set_rfreg(rtlpriv, path, RF_APK_Jaguar, bRFRegOffsetMask, 0x714E9);
		} else if ((IS_HARDWARE_TYPE_8821S(rtlhal)) && channel > 14) {
			/* <20130111, Kordan> For better result of APK. Asked by Willson. */
			if (36 <= channel && channel <= 64)
				rtl_set_rfreg(rtlpriv, path, RF_APK_Jaguar, bRFRegOffsetMask, 0x714E9);
			else if (100 <= channel && channel <= 140)
				rtl_set_rfreg(rtlpriv, path, RF_APK_Jaguar, bRFRegOffsetMask, 0x110E9);
			else
				rtl_set_rfreg(rtlpriv, path, RF_APK_Jaguar, bRFRegOffsetMask, 0x714E9);
		}
	}
}


static void PHY_HandleSwChnlAndSetBW8812(struct rtl_priv *rtlpriv,
	BOOLEAN	bSwitchChannel, BOOLEAN	bSetBandWidth,
	uint8_t	ChannelNum, enum CHANNEL_WIDTH ChnlWidth,
	uint8_t	ChnlOffsetOf40MHz, uint8_t ChnlOffsetOf80MHz,
	uint8_t	CenterFrequencyIndex1
)
{
	struct rtl_mac	*mac = &(rtlpriv->mac80211);
	struct _rtw_hal *	pHalData = GET_HAL_DATA(rtlpriv);
	uint8_t			tmpChannel = rtlpriv->phy.current_channel;
	enum CHANNEL_WIDTH	tmpBW= rtlpriv->phy.current_chan_bw;
	uint8_t			tmpnCur40MhzPrimeSC = mac->cur_40_prime_sc;
	uint8_t			tmpnCur80MhzPrimeSC = mac->cur_80_prime_sc;
	uint8_t			tmpCenterFrequencyIndex1 = rtlpriv->phy.current_channel;
	struct mlme_ext_priv	*pmlmeext = &rtlpriv->mlmeextpriv;

	BOOLEAN bSwChnl = _FALSE, bSetChnlBW = _FALSE;


	/* DBG_871X("=> PHY_HandleSwChnlAndSetBW8812: bSwitchChannel %d, bSetBandWidth %d \n",bSwitchChannel,bSetBandWidth); */

	/* check is swchnl or setbw */
	if(!bSwitchChannel && !bSetBandWidth) {
		RT_TRACE(rtlpriv, COMP_ERR, DBG_LOUD, "PHY_HandleSwChnlAndSetBW8812:  not switch channel and not set bandwidth \n");
		return;
	}

	/* skip change for channel or bandwidth is the same */
	if(bSwitchChannel) {
		if(rtlpriv->phy.current_channel != ChannelNum) {
			if (HAL_IsLegalChannel(rtlpriv, ChannelNum))
				bSwChnl = _TRUE;
			else
				return;
		}
	}

	if(bSetBandWidth) {
		if(pHalData->bChnlBWInitialzed == _FALSE) {
			pHalData->bChnlBWInitialzed = _TRUE;
			bSetChnlBW = _TRUE;
		} else if((rtlpriv->phy.current_chan_bw != ChnlWidth) ||
			(mac->cur_40_prime_sc != ChnlOffsetOf40MHz) ||
			(mac->cur_80_prime_sc != ChnlOffsetOf80MHz) ||
			(rtlpriv->phy.current_channel != CenterFrequencyIndex1)) {

			bSetChnlBW = _TRUE;
		}
	}

	if(!bSetChnlBW && !bSwChnl) {
		/* DBG_871X("<= PHY_HandleSwChnlAndSetBW8812: bSwChnl %d, bSetChnlBW %d \n",bSwChnl,bSetChnlBW); */
		return;
	}


	if(bSwChnl) {
		rtlpriv->phy.current_channel = ChannelNum;
	}


	if(bSetChnlBW) {
		rtlpriv->phy.current_chan_bw = ChnlWidth;
		mac->cur_40_prime_sc = ChnlOffsetOf40MHz;
		mac->cur_80_prime_sc = ChnlOffsetOf80MHz;
	}

	/* Switch workitem or set timer to do switch channel or setbandwidth operation */
	if((!rtlpriv->bDriverStopped) && (!rtlpriv->bSurpriseRemoved)) {
		struct rtl_hal *rtlhal = rtl_hal(rtlpriv);

		/* DBG_871X("phy_SwChnlAndSetBwMode8812(): bSwChnl %d, bSetChnlBW %d \n", bSwChnl, bSetChnlBW); */

		if ((rtlpriv->bDriverStopped) || (rtlpriv->bSurpriseRemoved)) {
			return;
		}

		if (bSwChnl) {
			rtl8821au_phy_sw_chnl_callback(rtlpriv);
			bSwChnl = _FALSE;
		}

		if (bSetChnlBW) {
			rtlpriv->cfg->ops->phy_set_bw_mode_callback(rtlpriv);
			bSetChnlBW = _FALSE;
		}

		rtl8821au_dm_clean_txpower_tracking_state(rtlpriv);
		PHY_SetTxPowerLevel8812(rtlpriv, rtlpriv->phy.current_channel);

		if (IS_HARDWARE_TYPE_8812(rtlhal))
			phy_InitRssiTRSW(rtlpriv);

		if ((rtlpriv->phy.need_iqk = false == true)) {
			if(IS_HARDWARE_TYPE_8812(rtlhal))
				rtl8812au_phy_iq_calibrate(rtlpriv, _FALSE);
			else if(IS_HARDWARE_TYPE_8821(rtlhal))
				rtl8821au_phy_iq_calibrate(rtlpriv, _FALSE);

			rtlpriv->phy.need_iqk = false;
		}
	} else {
		if(bSwChnl) {
			rtlpriv->phy.current_channel = tmpChannel;
		}
		if(bSetChnlBW) {
			rtlpriv->phy.current_chan_bw = tmpBW;
			mac->cur_40_prime_sc = tmpnCur40MhzPrimeSC;
			mac->cur_80_prime_sc = tmpnCur80MhzPrimeSC;
		}
	}

	/*
	 * DBG_871X("Channel %d ChannelBW %d ",pHalData->CurrentChannel, pHalData->CurrentChannelBW);
	 * DBG_871X("40MhzPrimeSC %d 80MhzPrimeSC %d ",pHalData->nCur40MhzPrimeSC, pHalData->nCur80MhzPrimeSC);
	 * DBG_871X("CenterFrequencyIndex1 %d \n",pHalData->CurrentCenterFrequencyIndex1);
	 */

	/*
	 * DBG_871X("<= PHY_HandleSwChnlAndSetBW8812: bSwChnl %d, bSetChnlBW %d \n",bSwChnl,bSetChnlBW);
	 */

}

void PHY_SetBWMode8812(struct rtl_priv *rtlpriv,
	enum CHANNEL_WIDTH	Bandwidth,	/* 20M or 40M */
	uint8_t	Offset)		/* Upper, Lower, or Don't care */
{
	struct _rtw_hal *	pHalData = GET_HAL_DATA(rtlpriv);

	/* DBG_871X("%s()===>\n",__FUNCTION__); */

	PHY_HandleSwChnlAndSetBW8812(rtlpriv, _FALSE, _TRUE, rtlpriv->phy.current_channel, Bandwidth, Offset, Offset, rtlpriv->phy.current_channel);

	//DBG_871X("<==%s()\n",__FUNCTION__);
}

void PHY_SwChnl8812(struct rtl_priv *rtlpriv, uint8_t channel)
{
	/* DBG_871X("%s()===>\n",__FUNCTION__); */

	PHY_HandleSwChnlAndSetBW8812(rtlpriv, _TRUE, _FALSE, channel, 0, 0, 0, channel);

	/* DBG_871X("<==%s()\n",__FUNCTION__); */
}

void PHY_SetSwChnlBWMode8812(struct rtl_priv *rtlpriv, uint8_t channel,
	enum CHANNEL_WIDTH Bandwidth, uint8_t Offset40, uint8_t Offset80)
{
	/* DBG_871X("%s()===>\n",__FUNCTION__); */

	PHY_HandleSwChnlAndSetBW8812(rtlpriv, _TRUE, _TRUE, channel, Bandwidth, Offset40, Offset80, channel);

	/* DBG_871X("<==%s()\n",__FUNCTION__); */
}


