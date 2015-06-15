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
#include <rtw_debug.h>

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

static void PHY_ConvertPowerLimitToPowerIndex(struct rtl_priv *rtlpriv);
static void PHY_InitPowerLimitTable(struct rtl_priv *rtlpriv);

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


/*
 * 2012/10/18
 */
static void PHY_StorePwrByRateIndexVhtSeries(struct rtl_priv *rtlpriv,
	uint32_t RegAddr, uint32_t BitMask, uint32_t Data)
{
	struct rtl_phy *rtlphy = &(rtlpriv->phy);
	struct _rtw_hal	*pHalData = GET_HAL_DATA(rtlpriv);
	uint8_t			rf_path, rate_section;

	/*
	 * For VHT series TX power by rate table.
	 * VHT TX power by rate off setArray =
	 * Band:-2G&5G = 0 / 1
	 * RF: at most 4*4 = ABCD=0/1/2/3
	 * CCK=0 				11/5.5/2/1
	 * OFDM=1/2 			18/12/9/6     54/48/36/24
	 * HT=3/4/56 			MCS0-3 MCS4-7 MCS8-11 MCS12-15
	 * VHT=7/8/9/10/11		1SSMCS0-3 1SSMCS4-7 2SSMCS1/0/1SSMCS/9/8 2SSMCS2-5
	 *
	 * #define		TX_PWR_BY_RATE_NUM_BAND			2
	 * #define		TX_PWR_BY_RATE_NUM_RF			4
	 * #define		TX_PWR_BY_RATE_NUM_SECTION		12
	 */

	/*
	 *  1. Judge TX power by rate array band type.
	 */

	/* if(RegAddr == rTxAGC_A_CCK11_CCK1_JAguar || RegAddr == rTxAGC_B_CCK11_CCK1_JAguar) */
	if ((RegAddr & 0xFFF) == 0xC20) {
		pHalData->TxPwrByRateTable++;	/* Record that it is the first data to record. */
		pHalData->TxPwrByRateBand = 0;
	}

	if ((RegAddr & 0xFFF) == 0xe20) {
		pHalData->TxPwrByRateTable++;	/* The value should be 2 now.*/
	}

	if ((RegAddr & 0xFFF) == 0xC24 && pHalData->TxPwrByRateTable != 1) {
		pHalData->TxPwrByRateTable++;	/* The value should be 3 bow. */
		pHalData->TxPwrByRateBand = 1;
	}

	/*
	 * 2. Judge TX power by rate array RF type
	 */
	if ((RegAddr & 0xF00) == 0xC00) {
		rf_path = 0;
	} else if ((RegAddr & 0xF00) == 0xE00) {
		rf_path = 1;
	}

	/*
	 * 3. Judge TX power by rate array rate section
	 */
	if (rf_path == 0) {
		rate_section = (uint8_t)((RegAddr&0xFFF)-0xC20)/4;
	} else if (rf_path == 1) {
		rate_section = (uint8_t)((RegAddr&0xFFF)-0xE20)/4;
	}

	rtlphy->tx_power_by_rate_offset[pHalData->TxPwrByRateBand][rf_path][rate_section] = Data;

	/*
	 * DBG_871X("VHT tx_power_by_rate_offset Addr-%x==>BAND/RF/SEC=%d/%d/%d = %08x\n",
	 * 	RegAddr, pHalData->TxPwrByRateBand, rf_path, rate_section, Data);
	 */

}

static void phy_ChangePGDataFromExactToRelativeValue(u32* pData, uint8_t Start,
	uint8_t End, uint8_t BaseValue)
{
	s8	i = 0;
	uint8_t	TempValue = 0;
	uint32_t	TempData = 0;

	/* BaseValue = ( BaseValue & 0xf ) + ( ( BaseValue >> 4 ) & 0xf ) * 10; */
	/* RT_TRACE(COMP_INIT, DBG_LOUD, ("Corrected BaseValue %u\n", BaseValue ) ); */

	for (i = 3; i >= 0; --i) {
		if (i >= Start && i <= End) {
			/* Get the exact value */
			TempValue = (uint8_t) (*pData >> (i * 8) ) & 0xF;
			TempValue += (( uint8_t) (( *pData >> (i * 8 + 4)) & 0xF)) * 10;

			/* Change the value to a relative value */
			TempValue = (TempValue > BaseValue) ? TempValue - BaseValue : BaseValue - TempValue;
		} else {
			TempValue = (uint8_t) (*pData >> (i * 8)) & 0xFF;
		}

		TempData <<= 8;
		TempData |= TempValue;
	}

	*pData = TempData;
}

static void phy_PreprocessVHTPGDataFromExactToRelativeValue(struct rtl_priv *rtlpriv,
	uint32_t RegAddr, uint32_t BitMask, u32 *pData)
{
	struct rtl_phy *rtlphy = &(rtlpriv->phy);
	struct _rtw_hal	*pHalData = GET_HAL_DATA(rtlpriv);
	uint8_t		rf_path, rate_section, BaseValue = 0;
	/*
	 * For VHT series TX power by rate table.
	 * VHT TX power by rate off setArray =
	 * Band:-2G&5G = 0 / 1
	 * RF: at most 4*4 = ABCD=0/1/2/3
	 * CCK=0 				11/5.5/2/1
	 * OFDM=1/2 			18/12/9/6     54/48/36/24
	 * HT=3/4/56 			MCS0-3 MCS4-7 MCS8-11 MCS12-15
	 * VHT=7/8/9/10/11		1SSMCS0-3 1SSMCS4-7 2SSMCS1/0/1SSMCS/9/8 2SSMCS2-5
	 *
	 * #define		TX_PWR_BY_RATE_NUM_BAND			2
	 * #define		TX_PWR_BY_RATE_NUM_RF			4
	 * #define		TX_PWR_BY_RATE_NUM_SECTION		12
	 *
	 * Judge TX power by rate array RF type
	 */

	if (( RegAddr & 0xF00) == 0xC00) {
		rf_path = 0;
	} else if (( RegAddr & 0xF00) == 0xE00) {
		rf_path = 1;
	}

	/*
	 * Judge TX power by rate array rate section
	 */

	if (rf_path == 0) {
		rate_section = (u8) (( RegAddr & 0xFFF) - 0xC20) / 4;
	} else if (rf_path == 1) {
		rate_section = (uint8_t) (( RegAddr & 0xFFF) - 0xE20) / 4;
	}

	switch (RegAddr) {
	case 0xC20:
	case 0xE20:
		/*
		 * RT_TRACE(COMP_INIT, DBG_LOUD, ("RegAddr %x\n", RegAddr ));
		 * RT_TRACE(COMP_INIT, DBG_LOUD, ("pHalData->tx_power_by_rate_offset[%d][%d][%d] = 0x%x, before changing to relative\n",
		 * 	pHalData->TxPwrByRateBand, rf_path, rate_section, *pData ));
		 */
		BaseValue = ((uint8_t) (*pData >> 28) & 0xF) * 10 + ((uint8_t) (*pData >> 24) & 0xF);
		phy_ChangePGDataFromExactToRelativeValue( pData, 0, 3, BaseValue );
		/*
		 * RT_TRACE(COMP_INIT, DBG_LOUD, ("pHalData->tx_power_by_rate_offset[%d][%d][%d] = 0x%x, after changing to relative\n",
		 * 		pHalData->TxPwrByRateBand, rf_path, rate_section, *pData ));
		 */
		break;

	case 0xC28:
	case 0xE28:
	case 0xC30:
	case 0xE30:
	case 0xC38:
	case 0xE38:
		/*
		 * RT_TRACE(COMP_INIT, DBG_LOUD, ("RegAddr %x\n", RegAddr ));
		 * RT_TRACE(COMP_INIT, DBG_LOUD, ("pHalData->tx_power_by_rate_offset[%d][%d][%d] = 0x%x, before changing to relative\n",
		 * 	pHalData->TxPwrByRateBand, rf_path, rate_section, *pData ));
		 * RT_TRACE(COMP_INIT, DBG_LOUD, ("pHalData->tx_power_by_rate_offset[%d][%d][%d] = 0x%x, before changing to relative\n",
		 * 	pHalData->TxPwrByRateBand, rf_path, rate_section - 1, pHalData->tx_power_by_rate_offset[pHalData->TxPwrByRateBand][rf_path][rate_section - 1] ));
		 */
		BaseValue = ((uint8_t) (*pData >> 28) & 0xF) * 10 + ((uint8_t) (*pData >> 24) & 0xF);
		phy_ChangePGDataFromExactToRelativeValue(pData, 0, 3, BaseValue);
		phy_ChangePGDataFromExactToRelativeValue(
			&(rtlphy->tx_power_by_rate_offset[pHalData->TxPwrByRateBand][rf_path][rate_section - 1]), 0, 3, BaseValue);

		/*
		 * RT_TRACE(COMP_INIT, DBG_LOUD, ("pHalData->tx_power_by_rate_offset[%d][%d][%d] = 0x%x, after changing to relative\n",
		 * 	pHalData->TxPwrByRateBand, rf_path, rate_section, *pData ));
		 * RT_TRACE(COMP_INIT, DBG_LOUD, ("pHalData->tx_power_by_rate_offset[%d][%d][%d] = 0x%x, after changing to relative\n",
		 * 	pHalData->TxPwrByRateBand, rf_path, rate_section - 1, pHalData->tx_power_by_rate_offset[pHalData->TxPwrByRateBand][rf_path][rate_section - 1] ));
		 */
		break;

	case 0xC44:
	case 0xE44:
		/*
		 * RT_TRACE(COMP_INIT, DBG_LOUD, ("RegAddr %x\n", RegAddr ));
		 * RT_TRACE(COMP_INIT, DBG_LOUD, ("pHalData->tx_power_by_rate_offset[%d][%d][%d] = 0x%x, before changing to relative\n",
		 * 	pHalData->TxPwrByRateBand, rf_path, rate_section, *pData ));
		 * RT_TRACE(COMP_INIT, DBG_LOUD, ("pHalData->tx_power_by_rate_offset[%d][%d][%d] = 0x%x, before changing to relative\n",
		 * 	pHalData->TxPwrByRateBand, rf_path, rate_section - 1, pHalData->tx_power_by_rate_offset[pHalData->TxPwrByRateBand][rf_path][rate_section - 1] ));
		 * RT_TRACE(COMP_INIT, DBG_LOUD, ("pHalData->tx_power_by_rate_offset[%d][%d][%d] = 0x%x, before changing to relative\n",
		 * 	pHalData->TxPwrByRateBand, rf_path, rate_section - 2, pHalData->tx_power_by_rate_offset[pHalData->TxPwrByRateBand][rf_path][rate_section - 2] ));
		 */
		BaseValue = ((uint8_t) (rtlphy->tx_power_by_rate_offset[pHalData->TxPwrByRateBand][rf_path][rate_section - 1] >> 28) & 0xF) * 10 +
					((uint8_t) (rtlphy->tx_power_by_rate_offset[pHalData->TxPwrByRateBand][rf_path][rate_section - 1] >> 24 ) & 0xF);

		phy_ChangePGDataFromExactToRelativeValue(pData, 0, 1, BaseValue);
		phy_ChangePGDataFromExactToRelativeValue(
			&( rtlphy->tx_power_by_rate_offset[pHalData->TxPwrByRateBand][rf_path][rate_section - 1]), 0, 3, BaseValue);
		phy_ChangePGDataFromExactToRelativeValue(
			&( rtlphy->tx_power_by_rate_offset[pHalData->TxPwrByRateBand][rf_path][rate_section - 2]), 0, 3, BaseValue);

		/*
		 * RT_TRACE(COMP_INIT, DBG_LOUD, ("pHalData->tx_power_by_rate_offset[%d][%d][%d] = 0x%x, after changing to relative\n",
		 * 	pHalData->TxPwrByRateBand, rf_path, rate_section, *pData ));
		 * RT_TRACE(COMP_INIT, DBG_LOUD, ("pHalData->tx_power_by_rate_offset[%d][%d][%d] = 0x%x, after changing to relative\n",
		 * 	pHalData->TxPwrByRateBand, rf_path, rate_section - 1, pHalData->tx_power_by_rate_offset[pHalData->TxPwrByRateBand][rf_path][rate_section - 1] ));
		 * RT_TRACE(COMP_INIT, DBG_LOUD, ("pHalData->tx_power_by_rate_offset[%d][%d][%d] = 0x%x, after changing to relative\n",
		 * 	pHalData->TxPwrByRateBand, rf_path, rate_section - 2, pHalData->tx_power_by_rate_offset[pHalData->TxPwrByRateBand][rf_path][rate_section - 2] ));
		 */
		break;

	case 0xC4C:
	case 0xE4C:
		/*
		 * RT_TRACE(COMP_INIT, DBG_LOUD, ("RegAddr %x\n", RegAddr ));
		 * RT_TRACE(COMP_INIT, DBG_LOUD, ("pHalData->tx_power_by_rate_offset[%d][%d][%d] = 0x%x, before changing to relative\n",
		 * 	pHalData->TxPwrByRateBand, rf_path, rate_section, *pData ));
		 * RT_TRACE(COMP_INIT, DBG_LOUD, ("pHalData->tx_power_by_rate_offset[%d][%d][%d] = 0x%x, before changing to relative\n",
		 * 	pHalData->TxPwrByRateBand, rf_path, rate_section - 1, pHalData->tx_power_by_rate_offset[pHalData->TxPwrByRateBand][rf_path][rate_section - 1] ));
		 * RT_TRACE(COMP_INIT, DBG_LOUD, ("pHalData->tx_power_by_rate_offset[%d][%d][%d] = 0x%x, before changing to relative\n",
		 * 	pHalData->TxPwrByRateBand, rf_path, rate_section - 2, pHalData->tx_power_by_rate_offset[pHalData->TxPwrByRateBand][rf_path][rate_section - 2] ));
		 */

		BaseValue = ( ( uint8_t ) ( *pData >> 12 ) & 0xF ) *10 + ( ( uint8_t ) ( *pData >> 8 ) & 0xF );
		phy_ChangePGDataFromExactToRelativeValue( pData, 0, 3, BaseValue );
		phy_ChangePGDataFromExactToRelativeValue(
			&(rtlphy->tx_power_by_rate_offset[pHalData->TxPwrByRateBand][rf_path][rate_section - 1]), 0, 3, BaseValue);
		phy_ChangePGDataFromExactToRelativeValue(
			&(rtlphy->tx_power_by_rate_offset[pHalData->TxPwrByRateBand][rf_path][rate_section - 2]), 2, 3, BaseValue);
		/*
		 * RT_TRACE(COMP_INIT, DBG_LOUD, ("pHalData->tx_power_by_rate_offset[%d][%d][%d] = 0x%x, after changing to relative\n",
		 * 	pHalData->TxPwrByRateBand, rf_path, rate_section, *pData ));
		 * RT_TRACE(COMP_INIT, DBG_LOUD, ("pHalData->tx_power_by_rate_offset[%d][%d][%d] = 0x%x, after changing to relative\n",
		 * 	pHalData->TxPwrByRateBand, rf_path, rate_section - 1, pHalData->tx_power_by_rate_offset[pHalData->TxPwrByRateBand][rf_path][rate_section - 1] ));
		 * RT_TRACE(COMP_INIT, DBG_LOUD, ("pHalData->tx_power_by_rate_offset[%d][%d][%d] = 0x%x, after changing to relative\n",
		 * 	pHalData->TxPwrByRateBand, rf_path, rate_section - 2, pHalData->tx_power_by_rate_offset[pHalData->TxPwrByRateBand][rf_path][rate_section - 2] ));
		 */
		break;
	}
}

static void phy_PreprocessPGDataFromExactToRelativeValue(struct rtl_priv *rtlpriv,
	uint32_t RegAddr, uint32_t BitMask, uint32_t *pData)
{
	struct rtl_phy *rtlphy = &(rtlpriv->phy);
	struct rtl_hal *rtlhal = rtl_hal(rtlpriv);
	u8			BaseValue = 0;

	if (RegAddr == rTxAGC_A_Rate54_24) {
		/*
		 * DBG_871X("RegAddr %x\n", RegAddr );
		 * DBG_871X("pHalData->MCSTxPowerLevelOriginalOffset[%d][1] = 0x%x, before changing to relative\n",
		 * 	pHalData->pwrGroupCnt, *pData );
		 * DBG_871X("pHalData->MCSTxPowerLevelOriginalOffset[%d][0] = 0x%x, before changing to relative\n",
		 * 	pHalData->pwrGroupCnt, pHalData->MCSTxPowerLevelOriginalOffset[pHalData->pwrGroupCnt][0] );
		 */
		BaseValue = ((uint8_t) (*pData >> 28) & 0xF) * 10 + ((uint8_t) (*pData >> 24) & 0xF );
		/* DBG_871X("BaseValue = %d\n", BaseValue ); */
		phy_ChangePGDataFromExactToRelativeValue( pData, 0, 3, BaseValue );
		phy_ChangePGDataFromExactToRelativeValue(
			&(rtlphy->mcs_txpwrlevel_origoffset[rtlphy->pwrgroup_cnt][0] ), 0, 3, BaseValue);

		/*
		 * DBG_871X("pHalData->MCSTxPowerLevelOriginalOffset[%d][1] = 0x%x, after changing to relative\n",
		 * 	pHalData->pwrGroupCnt, *pData );
		 * DBG_871X("pHalData->MCSTxPowerLevelOriginalOffset[%d][0] = 0x%x, after changing to relative\n",
		 * 	pHalData->pwrGroupCnt, pHalData->MCSTxPowerLevelOriginalOffset[pHalData->pwrGroupCnt][0] );
		 */
	}

	if (RegAddr == rTxAGC_A_CCK1_Mcs32) {
		rtlphy->mcs_txpwrlevel_origoffset[rtlphy->pwrgroup_cnt][6] = *pData;
		/*
		 * DBG_871X("MCSTxPowerLevelOriginalOffset[%d][6] = 0x%x\n", pHalData->pwrGroupCnt,
		 * 	pHalData->MCSTxPowerLevelOriginalOffset[pHalData->pwrGroupCnt][6]);
		 */
	}

	if (RegAddr == rTxAGC_B_CCK11_A_CCK2_11 && BitMask == 0xffffff00) {
		rtlphy->mcs_txpwrlevel_origoffset[rtlphy->pwrgroup_cnt][7] = *pData;
		/*
		 * DBG_871X("MCSTxPowerLevelOriginalOffset[%d][7] = 0x%x\n", pHalData->pwrGroupCnt,
		 * 	pHalData->MCSTxPowerLevelOriginalOffset[pHalData->pwrGroupCnt][7]);
		 */
	}

	if (RegAddr == rTxAGC_A_Mcs07_Mcs04) {
		/*
		 * DBG_871X("RegAddr %x\n", RegAddr );
		 * DBG_871X("pHalData->MCSTxPowerLevelOriginalOffset[%d][3] = 0x%x, before changing to relative\n",
		 * 	pHalData->pwrGroupCnt, *pData );
		 * DBG_871X("pHalData->MCSTxPowerLevelOriginalOffset[%d][2] = 0x%x, before changing to relative\n",
		 * 	pHalData->pwrGroupCnt, pHalData->MCSTxPowerLevelOriginalOffset[pHalData->pwrGroupCnt][2] );
		 */
		BaseValue = ((uint8_t) (*pData >> 28) & 0xF) *10 + ((uint8_t) (*pData >> 24) & 0xF);

		/* DBG_871X("BaseValue = %d\n", BaseValue ); */
		phy_ChangePGDataFromExactToRelativeValue(pData, 0, 3, BaseValue);
		phy_ChangePGDataFromExactToRelativeValue(
			&(rtlphy->mcs_txpwrlevel_origoffset[rtlphy->pwrgroup_cnt][2]), 0, 3, BaseValue);
		/*
		 * DBG_871X("pHalData->MCSTxPowerLevelOriginalOffset[%d][3] = 0x%x, after changing to relative\n",
		 * 	pHalData->pwrGroupCnt, *pData );
		 * DBG_871X("pHalData->MCSTxPowerLevelOriginalOffset[%d][2] = 0x%x, after changing to relative\n",
		 * 	pHalData->pwrGroupCnt, pHalData->MCSTxPowerLevelOriginalOffset[pHalData->pwrGroupCnt][2] );
		 */
	}

	if (RegAddr == rTxAGC_A_Mcs11_Mcs08) {
		rtlphy->mcs_txpwrlevel_origoffset[rtlphy->pwrgroup_cnt][4] = *pData;
		/* DBG_871X("MCSTxPowerLevelOriginalOffset[%d][4] = 0x%x\n", pHalData->pwrGroupCnt,
		 * 	pHalData->MCSTxPowerLevelOriginalOffset[pHalData->pwrGroupCnt][4]);
		 */
	}

	if (RegAddr == rTxAGC_A_Mcs15_Mcs12) {
		/* DBG_871X("RegAddr %x\n", RegAddr );
		 * DBG_871X("pHalData->MCSTxPowerLevelOriginalOffset[%d][5] = 0x%x, before changing to relative\n",
		 * 	pHalData->pwrGroupCnt, *pData );
		 * DBG_871X("pHalData->MCSTxPowerLevelOriginalOffset[%d][4] = 0x%x, before changing to relative\n",
		 * 	pHalData->pwrGroupCnt, pHalData->MCSTxPowerLevelOriginalOffset[pHalData->pwrGroupCnt][4] );
		 */
		BaseValue = ((uint8_t) (*pData >> 28) & 0xF) * 10 + ((uint8_t) (*pData >> 24) & 0xF);
		/* DBG_871X("BaseValue = %d\n", BaseValue ); */
		phy_ChangePGDataFromExactToRelativeValue(pData, 0, 3, BaseValue);
		phy_ChangePGDataFromExactToRelativeValue(
			&(rtlphy->mcs_txpwrlevel_origoffset[rtlphy->pwrgroup_cnt][4]), 0, 3, BaseValue);
		/*
		 * DBG_871X("pHalData->MCSTxPowerLevelOriginalOffset[%d][5] = 0x%x, after changing to relative\n",
		 * 	pHalData->pwrGroupCnt, *pData );
		 * DBG_871X("pHalData->MCSTxPowerLevelOriginalOffset[%d][4] = 0x%x, after changing to relative\n",
		 * 	pHalData->pwrGroupCnt, pHalData->MCSTxPowerLevelOriginalOffset[pHalData->pwrGroupCnt][4] );
		 */
	}

	if (RegAddr == rTxAGC_B_Rate54_24) {
		/*
		 * DBG_871X("RegAddr %x\n", RegAddr );
		 * DBG_871X("pHalData->MCSTxPowerLevelOriginalOffset[%d][9] = 0x%x, before changing to relative\n",
		 * 	pHalData->pwrGroupCnt, *pData );
		 * DBG_871X("pHalData->MCSTxPowerLevelOriginalOffset[%d][8] = 0x%x, before changing to relative\n",
		 * 	pHalData->pwrGroupCnt, pHalData->MCSTxPowerLevelOriginalOffset[pHalData->pwrGroupCnt][8] );
		 */
		BaseValue = ((uint8_t) (*pData >> 28) & 0xF) * 10 + ((uint8_t) (*pData >> 24) & 0xF);
		/* DBG_871X("BaseValue = %d\n", BaseValue ); */
		phy_ChangePGDataFromExactToRelativeValue(pData, 0, 3, BaseValue);
		phy_ChangePGDataFromExactToRelativeValue(
				&(rtlphy->mcs_txpwrlevel_origoffset[rtlphy->pwrgroup_cnt][8]), 0, 3, BaseValue);
		/*
		 * DBG_871X("pHalData->MCSTxPowerLevelOriginalOffset[%d][9] = 0x%x, after changing to relative\n",
		 * 	pHalData->pwrGroupCnt, *pData );
		 * DBG_871X("pHalData->MCSTxPowerLevelOriginalOffset[%d][8] = 0x%x, after changing to relative\n",
		 * 	pHalData->pwrGroupCnt, pHalData->MCSTxPowerLevelOriginalOffset[pHalData->pwrGroupCnt][8] );
		 */

	}

	if (RegAddr == rTxAGC_B_CCK1_55_Mcs32) {
		rtlphy->mcs_txpwrlevel_origoffset[rtlphy->pwrgroup_cnt][14] = *pData;
		/*
		 * DBG_871X("MCSTxPowerLevelOriginalOffset[%d][14] = 0x%x\n", pHalData->pwrGroupCnt,
		 * 	pHalData->MCSTxPowerLevelOriginalOffset[pHalData->pwrGroupCnt][14]);
		 */
	}

	if (RegAddr == rTxAGC_B_CCK11_A_CCK2_11 && BitMask == 0x000000ff) {
		rtlphy->mcs_txpwrlevel_origoffset[rtlphy->pwrgroup_cnt][15] = *pData;
		/*
		 * DBG_871X("MCSTxPowerLevelOriginalOffset[%d][15] = 0x%x\n", pHalData->pwrGroupCnt,
		 * 	pHalData->MCSTxPowerLevelOriginalOffset[pHalData->pwrGroupCnt][15]);
		 */
	}

	if (RegAddr == rTxAGC_B_Mcs07_Mcs04) {
		/*
		 * DBG_871X("RegAddr %x\n", RegAddr );
		 * DBG_871X("pHalData->MCSTxPowerLevelOriginalOffset[%d][11] = 0x%x, before changing to relative\n",
		 * 	pHalData->pwrGroupCnt, *pData );
		 * DBG_871X("pHalData->MCSTxPowerLevelOriginalOffset[%d][10] = 0x%x, before changing to relative\n",
		 * 	pHalData->pwrGroupCnt, pHalData->MCSTxPowerLevelOriginalOffset[pHalData->pwrGroupCnt][10] );
		 */
		BaseValue = ((uint8_t) (*pData >> 28) & 0xF) * 10 + ((uint8_t) (*pData >> 24) & 0xF);
		/* DBG_871X("BaseValue = %d\n", BaseValue ); */
		phy_ChangePGDataFromExactToRelativeValue(pData, 0, 3, BaseValue);
		phy_ChangePGDataFromExactToRelativeValue(
				&(rtlphy->mcs_txpwrlevel_origoffset[rtlphy->pwrgroup_cnt][10] ), 0, 3, BaseValue);
		/*
		 * DBG_871X("pHalData->MCSTxPowerLevelOriginalOffset[%d][11] = 0x%x, after changing to relative\n",
		 * 	pHalData->pwrGroupCnt, *pData );
		 * DBG_871X("pHalData->MCSTxPowerLevelOriginalOffset[%d][10] = 0x%x, after changing to relative\n",
		 * 	pHalData->pwrGroupCnt, pHalData->MCSTxPowerLevelOriginalOffset[pHalData->pwrGroupCnt][10] );
		 */
	}

	if (RegAddr == rTxAGC_B_Mcs15_Mcs12) {
		/*
		 * DBG_871X("RegAddr %x\n", RegAddr );
		 * DBG_871X("pHalData->MCSTxPowerLevelOriginalOffset[%d][13] = 0x%x, before changing to relative\n",
		 * 	pHalData->pwrGroupCnt, *pData );
		 * DBG_871X("pHalData->MCSTxPowerLevelOriginalOffset[%d][12] = 0x%x, before changing to relative\n",
		 * 	pHalData->pwrGroupCnt, pHalData->MCSTxPowerLevelOriginalOffset[pHalData->pwrGroupCnt][12] );
		 */
		BaseValue = ( ( uint8_t ) ( *pData >> 28 ) & 0xF ) *10 + ( ( uint8_t ) ( *pData >> 24 ) & 0xF );
		/* DBG_871X("BaseValue = %d\n", BaseValue ); */
		phy_ChangePGDataFromExactToRelativeValue(pData, 0, 3, BaseValue);
		phy_ChangePGDataFromExactToRelativeValue(
				&(rtlphy->mcs_txpwrlevel_origoffset[rtlphy->pwrgroup_cnt][12]), 0, 3, BaseValue);

		/*
		 * DBG_871X("pHalData->MCSTxPowerLevelOriginalOffset[%d][13] = 0x%x, after changing to relative\n",
		 * 	pHalData->pwrGroupCnt, *pData );
		 * DBG_871X("pHalData->MCSTxPowerLevelOriginalOffset[%d][12] = 0x%x, after changing to relative\n",
		 * 	pHalData->pwrGroupCnt, pHalData->MCSTxPowerLevelOriginalOffset[pHalData->pwrGroupCnt][12] );
		 */
	}



	/*
	 *  1. Judge TX power by rate array band type.
	 */
	/* if(RegAddr == rTxAGC_A_CCK11_CCK1_JAguar || RegAddr == rTxAGC_B_CCK11_CCK1_JAguar) */

	if (IS_HARDWARE_TYPE_8812(rtlhal) || IS_HARDWARE_TYPE_8821(rtlhal)) {
		phy_PreprocessVHTPGDataFromExactToRelativeValue(rtlpriv, RegAddr,
			BitMask, pData);
	}

}

static void phy_StorePwrByRateIndexBase(struct rtl_priv *rtlpriv, uint32_t RegAddr,
	uint32_t Data)
{
	struct rtl_phy *rtlphy = &(rtlpriv->phy);
	struct _rtw_hal	*pHalData = GET_HAL_DATA(rtlpriv);
	uint8_t		Base = 0;


	if (pHalData->TxPwrByRateTable == 1 && pHalData->TxPwrByRateBand == 0) {	/* 2.4G */
		if (pHalData->odmpriv.PhyRegPgValueType == PHY_REG_PG_EXACT_VALUE) {
			Base = (((uint8_t) (Data >> 28) & 0xF) * 10 +
					((uint8_t) (Data >> 24) & 0xF));

			switch(RegAddr) {
			case 0xC20:
				rtlphy->txpwr_by_rate_base_24g[RF90_PATH_A][0] = Base;
				/*
				 * RT_DISP(FPHY, PHY_TXPWR, ("2.4G power by rate of CCK (RF path A) = %d\n",
				 * 	pHalData->txpwr_by_rate_base_24g[RF90_PATH_A][0] ) );
				 */
				break;
			case 0xC28:
				rtlphy->txpwr_by_rate_base_24g[RF90_PATH_A][1] = Base;
				/*
				 * RT_DISP(FPHY, PHY_TXPWR, ("2.4G power by rate of OFDM 54M (RF path A) = %d\n",
				 * 	pHalData->txpwr_by_rate_base_24g[RF90_PATH_A][1] ) );
				 */
				break;
			case 0xC30:
				rtlphy->txpwr_by_rate_base_24g[RF90_PATH_A][2] = Base;
				/*
				 * RT_DISP(FPHY, PHY_TXPWR, ("2.4G power by rate of MCS7 (RF path A) = %d\n",
				 * 	pHalData->txpwr_by_rate_base_24g[RF90_PATH_A][2] ) );
				 */
				break;
			case 0xC38:
				rtlphy->txpwr_by_rate_base_24g[RF90_PATH_A][3] = Base;
				/*
				 * RT_DISP(FPHY, PHY_TXPWR, ("2.4G power by rate of MCS15 (RF path A) = %d\n",
				 * 	pHalData->txpwr_by_rate_base_24g[RF90_PATH_A][3] ) );
				 */
				break;
			default:
				break;
			};
		} else {
			Base = (uint8_t) (Data >> 24);
			switch(RegAddr) {
			case 0xC20:
				rtlphy->txpwr_by_rate_base_24g[RF90_PATH_A][0] = Base;
				/*
				 * RT_DISP(FPHY, PHY_TXPWR, ("2.4G power by rate of CCK (RF path A) = %d\n",
				 * 	pHalData->txpwr_by_rate_base_24g[RF90_PATH_A][0] ) );
				 */
				break;
			case 0xC28:
				rtlphy->txpwr_by_rate_base_24g[RF90_PATH_A][1] = Base;
				/*
				 * RT_DISP(FPHY, PHY_TXPWR, ("2.4G power by rate of OFDM 54M (RF path A) = %d\n",
				 * 	pHalData->txpwr_by_rate_base_24g[RF90_PATH_A][1] ) );
				 */
				break;
			case 0xC30:
				rtlphy->txpwr_by_rate_base_24g[RF90_PATH_A][2] = Base;
				/*
				 * RT_DISP(FPHY, PHY_TXPWR, ("2.4G power by rate of MCS7 (RF path A) = %d\n",
				 * 	pHalData->txpwr_by_rate_base_24g[RF90_PATH_A][2] ) );
				 */
				break;
			case 0xC38:
				rtlphy->txpwr_by_rate_base_24g[RF90_PATH_A][3] = Base;
				/*
				 * RT_DISP(FPHY, PHY_TXPWR, ("2.4G power by rate of MCS15 (RF path A) = %d\n",
				 * 	pHalData->txpwr_by_rate_base_24g[RF90_PATH_A][3] ) );
				 */
				break;
			default:
				break;
			};
		}
	} else if (pHalData->TxPwrByRateTable == 3 && pHalData->TxPwrByRateBand == 1) {	/* 5G */
		if (pHalData->odmpriv.PhyRegPgValueType == PHY_REG_PG_EXACT_VALUE) {
			Base = (((uint8_t) (Data >> 28) & 0xF) * 10 +
					((uint8_t) (Data >> 24) & 0xF));

			switch(RegAddr) {
			case 0xC28:
				rtlphy->txpwr_by_rate_base_5g[RF90_PATH_A][0] = Base;
				/*
				 * RT_DISP(FPHY, PHY_TXPWR, ("5G power by rate of OFDM 54M (RF path A) = %d\n",
				 * 	pHalData->txpwr_by_rate_base_5g[RF90_PATH_A][0] ) );
				 */
				break;
			case 0xC30:
				rtlphy->txpwr_by_rate_base_5g[RF90_PATH_A][1] = Base;
				/*
				 * RT_DISP(FPHY, PHY_TXPWR, ("5G power by rate of MCS7 (RF path A) = %d\n",
				 * 	pHalData->txpwr_by_rate_base_5g[RF90_PATH_A][1] ) );
				 */
				break;
			case 0xC38:
				rtlphy->txpwr_by_rate_base_5g[RF90_PATH_A][2] = Base;
				/*
				 * RT_DISP(FPHY, PHY_TXPWR, ("5G power by rate of MCS15 (RF path A) = %d\n",
				 * 	pHalData->txpwr_by_rate_base_5g[RF90_PATH_A][2] ) );
				 */
				break;
			case 0xC40:
				rtlphy->txpwr_by_rate_base_5g[RF90_PATH_A][3] = Base;
				/*
				 * RT_DISP(FPHY, PHY_TXPWR, ("5G power by rate of 1SS MCS7 (RF path A) = %d\n",
				 * 	pHalData->txpwr_by_rate_base_5g[RF90_PATH_A][3] ) );
				 */
				break;
			case 0xC4C:
				rtlphy->txpwr_by_rate_base_5g[RF90_PATH_A][4] =
					( uint8_t ) ( ( Data >> 12 ) & 0xF ) * 10 +
					( uint8_t ) ( ( Data >> 8 ) & 0xF );
				/*
				 * RT_DISP(FPHY, PHY_TXPWR, ("5G power by rate of 2SS MCS7 (RF path A) = %d\n",
				 * 	pHalData->txpwr_by_rate_base_5g[RF90_PATH_A][4] ) );
				 */
				break;
			case 0xE28:
				rtlphy->txpwr_by_rate_base_5g[RF90_PATH_B][0] = Base;
				/*
				 * RT_DISP(FPHY, PHY_TXPWR, ("5G power by rate of OFDM 54M (RF path B) = %d\n",
				 *	pHalData->txpwr_by_rate_base_5g[RF90_PATH_B][0] ) );
				 */
				break;
			case 0xE30:
				rtlphy->txpwr_by_rate_base_5g[RF90_PATH_B][1] = Base;
				/*
				 * RT_DISP(FPHY, PHY_TXPWR, ("5G power by rate of MCS7 (RF path B) = %d\n",
				 * 	pHalData->txpwr_by_rate_base_5g[RF90_PATH_B][1] ) );
				 */
				break;
			case 0xE38:
				rtlphy->txpwr_by_rate_base_5g[RF90_PATH_B][2] = Base;
				/*
				 * RT_DISP(FPHY, PHY_TXPWR, ("5G power by rate of MCS15 (RF path B) = %d\n",
				 * 	pHalData->txpwr_by_rate_base_5g[RF90_PATH_B][2] ) );
				 */
				break;
			case 0xE40:
				rtlphy->txpwr_by_rate_base_5g[RF90_PATH_B][3] = Base;
				/*
				 * RT_DISP(FPHY, PHY_TXPWR, ("5G power by rate of 1SS MCS7 (RF path B) = %d\n",
				 * 	pHalData->txpwr_by_rate_base_5g[RF90_PATH_B][3] ) );
				 */
				break;
			case 0xE4C:
				rtlphy->txpwr_by_rate_base_5g[RF90_PATH_B][4] =
					( uint8_t ) ( ( Data >> 12 ) & 0xF ) * 10 +
					( uint8_t ) ( ( Data >> 8 ) & 0xF );
				/*
				 * RT_DISP(FPHY, PHY_TXPWR, ("5G power by rate of 2SS MCS7 (RF path B) = %d\n",
				 * 	pHalData->txpwr_by_rate_base_5g[RF90_PATH_B][4] ) );
				 */
				break;
			default:
				break;
			};

		} else {
			Base = (uint8_t) (Data >> 24);
			switch(RegAddr) {
			case 0xC28:
				rtlphy->txpwr_by_rate_base_5g[RF90_PATH_A][0]  = Base;
				/*
				 * RT_DISP(FPHY, PHY_TXPWR, ("5G power by rate of OFDM 54M (RF path A) = %d\n",
				 * 	pHalData->txpwr_by_rate_base_5g[RF90_PATH_A][0] ) );
				 */
				break;
			case 0xC30:
				rtlphy->txpwr_by_rate_base_5g[RF90_PATH_A][1]  = Base;
				/*
				 * RT_DISP(FPHY, PHY_TXPWR, ("5G power by rate of MCS7 (RF path A) = %d\n",
				 * 	pHalData->txpwr_by_rate_base_5g[RF90_PATH_A][1] ) );
				 */
				break;
			case 0xC38:
				rtlphy->txpwr_by_rate_base_5g[RF90_PATH_A][2]  = Base;
				/*
				 * RT_DISP(FPHY, PHY_TXPWR, ("5G power by rate of MCS15 (RF path A) = %d\n",
				 * 	pHalData->txpwr_by_rate_base_5g[RF90_PATH_A][2] ) );
				 */
				break;
			case 0xC40:
				rtlphy->txpwr_by_rate_base_5g[RF90_PATH_A][3] = Base;
				/*
				 * RT_DISP(FPHY, PHY_TXPWR, ("5G power by rate of 1SS MCS7 (RF path A) = %d\n",
				 * 	pHalData->txpwr_by_rate_base_5g[RF90_PATH_A][3] ) );
				 */
				break;
			case 0xC4C:
				rtlphy->txpwr_by_rate_base_5g[RF90_PATH_A][4] = ( uint8_t ) ( ( Data >> 8 ) & 0xFF );
				/*
				 * RT_DISP(FPHY, PHY_TXPWR, ("5G power by rate of 2SS MCS7 (RF path A) = %d\n",
				 * 	pHalData->txpwr_by_rate_base_5g[RF90_PATH_A][4] ) );
				 */
				break;
			case 0xE28:
				rtlphy->txpwr_by_rate_base_5g[RF90_PATH_B][0] = Base;
				/*
				 * RT_DISP(FPHY, PHY_TXPWR, ("5G power by rate of OFDM 54M (RF path B) = %d\n",
				 * 	pHalData->txpwr_by_rate_base_5g[RF90_PATH_B][0] ) );
				 */
				break;
			case 0xE30:
				rtlphy->txpwr_by_rate_base_5g[RF90_PATH_B][1] = Base;
				/*
				 * RT_DISP(FPHY, PHY_TXPWR, ("5G power by rate of MCS7 (RF path B) = %d\n",
				 * 	pHalData->txpwr_by_rate_base_5g[RF90_PATH_B][1] ) );
				 */
				break;
			case 0xE38:
				rtlphy->txpwr_by_rate_base_5g[RF90_PATH_B][2]  = Base;
				/*
				 * RT_DISP(FPHY, PHY_TXPWR, ("5G power by rate of MCS15 (RF path B) = %d\n",
				 * 	pHalData->txpwr_by_rate_base_5g[RF90_PATH_B][2] ) );
				 */
				break;
			case 0xE40:
				rtlphy->txpwr_by_rate_base_5g[RF90_PATH_B][3] = Base;
				/*
				 * RT_DISP(FPHY, PHY_TXPWR, ("5G power by rate of 1SS MCS7 (RF path B) = %d\n",
				 * 	pHalData->txpwr_by_rate_base_5g[RF90_PATH_B][3] ) );
				 */
				break;
			case 0xE4C:
				rtlphy->txpwr_by_rate_base_5g[RF90_PATH_B][4] = ( uint8_t ) ( ( Data >> 8 ) & 0xFF );
				/*
				 * RT_DISP(FPHY, PHY_TXPWR, ("5G power by rate of 2SS MCS7 (RF path B) = %d\n",
				 * 	pHalData->txpwr_by_rate_base_5g[RF90_PATH_B][4] ) );
				 */
				break;
			default:
				break;
			};
		}
	} else if(pHalData->TxPwrByRateTable == 2 && pHalData->TxPwrByRateBand == 0) {	/* 2.4G */
		if (pHalData->odmpriv.PhyRegPgValueType == PHY_REG_PG_EXACT_VALUE) {
			Base = (((uint8_t) (Data >> 28) & 0xF) * 10 +
					((uint8_t) (Data >> 24) & 0xF));

			switch(RegAddr) {
			case 0xE20:
				rtlphy->txpwr_by_rate_base_24g[RF90_PATH_B][0] = Base;
				/*
				 * RT_DISP(FPHY, PHY_TXPWR, ("2.4G power by rate of CCK (RF path B) = %d\n",
				 * 	pHalData->txpwr_by_rate_base_24g[RF90_PATH_B][0] ) );
				 */
				break;
			case 0xE28:
				rtlphy->txpwr_by_rate_base_24g[RF90_PATH_B][1] = Base;
				/*
				 * RT_DISP(FPHY, PHY_TXPWR, ("2.4G power by rate of OFDM 54M (RF path B) = %d\n",
				 * 	pHalData->txpwr_by_rate_base_24g[RF90_PATH_B][1] ) );
				 */
				break;
			case 0xE30:
				rtlphy->txpwr_by_rate_base_24g[RF90_PATH_B][2] = Base;
				/*
				 * RT_DISP(FPHY, PHY_TXPWR, ("2.4G power by rate of MCS7 (RF path B) = %d\n",
				 * 	pHalData->txpwr_by_rate_base_24g[RF90_PATH_B][2] ) );
				 */
				break;
			case 0xE38:
				rtlphy->txpwr_by_rate_base_24g[RF90_PATH_B][3] = Base;
				/*
				 * RT_DISP(FPHY, PHY_TXPWR, ("2.4G power by rate of MCS15 (RF path B) = %d\n",
				 * 	pHalData->txpwr_by_rate_base_24g[RF90_PATH_B][3] ) );
				 */
				break;
			default:
				break;
			};

		} else {
			Base = (uint8_t) (Data >> 24);
			switch(RegAddr) {
			case 0xC20:
				rtlphy->txpwr_by_rate_base_24g[RF90_PATH_B][0] = Base;
				/*
				 * RT_DISP(FPHY, PHY_TXPWR, ("2.4G power by rate of CCK (RF path B) = %d\n",
				 * 	pHalData->txpwr_by_rate_base_24g[RF90_PATH_B][0] ) );
				 */
				break;
			case 0xC28:
				rtlphy->txpwr_by_rate_base_24g[RF90_PATH_B][1] = Base;
				/*
				 * RT_DISP(FPHY, PHY_TXPWR, ("2.4G power by rate of OFDM 54M (RF path B) = %d\n",
				 * 	pHalData->txpwr_by_rate_base_24g[RF90_PATH_B][1] ) );
				 */
				break;
			case 0xC30:
				rtlphy->txpwr_by_rate_base_24g[RF90_PATH_B][2] = Base;
				/*
				 * RT_DISP(FPHY, PHY_TXPWR, ("2.4G power by rate of MCS7 (RF path B) = %d\n",
				 * 	pHalData->txpwr_by_rate_base_24g[RF90_PATH_B][2] ) );
				 */
				break;
			case 0xC38:
				rtlphy->txpwr_by_rate_base_24g[RF90_PATH_B][3] = Base;
				/*
				 * RT_DISP(FPHY, PHY_TXPWR, ("2.4G  power by rate of MCS15 (RF path B) = %d\n",
				 * 	pHalData->txpwr_by_rate_base_24g[RF90_PATH_B][3] ) );
				 */
				break;
			default:
				break;
			};
		}
	}

	/* -------------- following code is for 88E ---------------- */

	if (pHalData->odmpriv.PhyRegPgValueType == PHY_REG_PG_EXACT_VALUE) {
		Base =  (uint8_t) ((Data >> 28) & 0xF) * 10 +
				(uint8_t) ((Data >> 24) & 0xF);
	} else {
		Base =  (uint8_t) ((Data >> 24) & 0xFF);
	}

	switch (RegAddr) {
	case rTxAGC_A_Rate54_24:
		rtlphy->txpwr_by_rate_base_24g[RF90_PATH_A][1] = Base;
		rtlphy->txpwr_by_rate_base_24g[RF90_PATH_B][1] = Base;
		/*
		 * RT_DISP(FPHY, PHY_TXPWR, ("2.4G power by rate of OFDM 54M (RF path A) = %d\n", pHalData->txpwr_by_rate_base_24g[RF90_PATH_A][1]));
		 */
		break;
	case rTxAGC_A_Mcs07_Mcs04:
		rtlphy->txpwr_by_rate_base_24g[RF90_PATH_A][2] = Base;
		rtlphy->txpwr_by_rate_base_24g[RF90_PATH_B][2] = Base;
		/*
		 * RT_DISP(FPHY, PHY_TXPWR, ("2.4G power by rate of MCS7 (RF path A) = %d\n", pHalData->txpwr_by_rate_base_24g[RF90_PATH_A][2]));
		 */
		break;
	case rTxAGC_A_Mcs15_Mcs12:
		rtlphy->txpwr_by_rate_base_24g[RF90_PATH_A][3] = Base;
		rtlphy->txpwr_by_rate_base_24g[RF90_PATH_B][3] = Base;
		/*
		 * RT_DISP(FPHY, PHY_TXPWR, ("2.4G power by rate of MCS15 (RF path A) = %d\n", pHalData->txpwr_by_rate_base_24g[RF90_PATH_A][3]));
		 */
		break;
	default:
		break;

	};
}

/* Ulli called in odm_RegConfig8812A.c and odm_RegConfig8821A.c */

void storePwrIndexDiffRateOffset(struct rtl_priv *rtlpriv, uint32_t RegAddr,
	uint32_t BitMask, uint32_t Data)
{
	struct rtl_phy *rtlphy = &(rtlpriv->phy);
	struct rtl_hal *rtlhal = rtl_hal(rtlpriv);
	struct _rtw_hal	*pHalData = GET_HAL_DATA(rtlpriv);
	uint32_t	tmpData = Data;

	/*
	 * If the pHalData->DM_OutSrc.PhyRegPgValueType == 1, which means that the data in PHY_REG_PG data are
	 * exact value, we must change them into relative values
	 */
	if (pHalData->odmpriv.PhyRegPgValueType == PHY_REG_PG_EXACT_VALUE) {
		/* DBG_871X("PhyRegPgValueType = PHY_REG_PG_EXACT_VALUE\n"); */
		phy_PreprocessPGDataFromExactToRelativeValue( rtlpriv, RegAddr, BitMask, &Data );
		/* DBG_871X("Data = 0x%x, tmpData = 0x%x\n", Data, tmpData ); */
	}

	/*
	 * 2012/09/26 MH Add for VHT series. The power by rate table is diffeent as before.
	 * 2012/10/24 MH Add description for the old tx power by rate method is only used
	 * for 11 n series. T
	 */

	if (IS_HARDWARE_TYPE_8812(rtlhal) || IS_HARDWARE_TYPE_8821(rtlhal)) {
		PHY_StorePwrByRateIndexVhtSeries(rtlpriv, RegAddr, BitMask, Data);
	}

	/* Awk add to stroe the base power by rate value */
	phy_StorePwrByRateIndexBase(rtlpriv, RegAddr, tmpData );

	if (RegAddr == rTxAGC_A_Rate18_06) {
		rtlphy->mcs_txpwrlevel_origoffset[rtlphy->pwrgroup_cnt][0] = Data;
		/*
		 * RT_TRACE(COMP_INIT, DBG_TRACE, ("MCSTxPowerLevelOriginalOffset[%d][0] = 0x%lx\n", pHalData->pwrGroupCnt,
		 * 	pHalData->MCSTxPowerLevelOriginalOffset[pHalData->pwrGroupCnt][0]));
		 */
	}
	if (RegAddr == rTxAGC_A_Rate54_24) {
		rtlphy->mcs_txpwrlevel_origoffset[rtlphy->pwrgroup_cnt][1] = Data;
		/*
		 * RT_TRACE(COMP_INIT, DBG_TRACE, ("MCSTxPowerLevelOriginalOffset[%d][1] = 0x%lx\n", pHalData->pwrGroupCnt,
		 * 	pHalData->MCSTxPowerLevelOriginalOffset[pHalData->pwrGroupCnt][1]));
		 */
	}
	if(RegAddr == rTxAGC_A_CCK1_Mcs32) {
		rtlphy->mcs_txpwrlevel_origoffset[rtlphy->pwrgroup_cnt][6] = Data;
		/*
		 * RT_TRACE(COMP_INIT, DBG_TRACE, ("MCSTxPowerLevelOriginalOffset[%d][6] = 0x%lx\n", pHalData->pwrGroupCnt,
		 * 	pHalData->MCSTxPowerLevelOriginalOffset[pHalData->pwrGroupCnt][6]));
		 */
	}
	if (RegAddr == rTxAGC_B_CCK11_A_CCK2_11 && BitMask == 0xffffff00) {
		rtlphy->mcs_txpwrlevel_origoffset[rtlphy->pwrgroup_cnt][7] = Data;
		/*
		 * RT_TRACE(COMP_INIT, DBG_TRACE, ("MCSTxPowerLevelOriginalOffset[%d][7] = 0x%lx\n", pHalData->pwrGroupCnt,
		 * 	pHalData->MCSTxPowerLevelOriginalOffset[pHalData->pwrGroupCnt][7]));
		 */
	}
	if (RegAddr == rTxAGC_A_Mcs03_Mcs00) {
		rtlphy->mcs_txpwrlevel_origoffset[rtlphy->pwrgroup_cnt][2] = Data;
		/*
		 * RT_TRACE(COMP_INIT, DBG_TRACE, ("MCSTxPowerLevelOriginalOffset[%d][2] = 0x%lx\n", pHalData->pwrGroupCnt,
		 * 	pHalData->MCSTxPowerLevelOriginalOffset[pHalData->pwrGroupCnt][2]));
		 */
	}
	if (RegAddr == rTxAGC_A_Mcs07_Mcs04) {
		rtlphy->mcs_txpwrlevel_origoffset[rtlphy->pwrgroup_cnt][3] = Data;
		/*
		 * RT_TRACE(COMP_INIT, DBG_TRACE, ("MCSTxPowerLevelOriginalOffset[%d][3] = 0x%lx\n", pHalData->pwrGroupCnt,
		 * 	pHalData->MCSTxPowerLevelOriginalOffset[pHalData->pwrGroupCnt][3]));
		 */
	}
	if (RegAddr == rTxAGC_A_Mcs11_Mcs08) {
		rtlphy->mcs_txpwrlevel_origoffset[rtlphy->pwrgroup_cnt][4] = Data;
		/*
		 * RT_TRACE(COMP_INIT, DBG_TRACE, ("MCSTxPowerLevelOriginalOffset[%d][4] = 0x%lx\n", pHalData->pwrGroupCnt,
		 * 	pHalData->MCSTxPowerLevelOriginalOffset[pHalData->pwrGroupCnt][4]));
		 */
	}
	if (RegAddr == rTxAGC_A_Mcs15_Mcs12) {
		rtlphy->mcs_txpwrlevel_origoffset[rtlphy->pwrgroup_cnt][5] = Data;
		/*
		 * RT_TRACE(COMP_INIT, DBG_TRACE, ("MCSTxPowerLevelOriginalOffset[%d][5] = 0x%lx\n", pHalData->pwrGroupCnt,
		 * 	pHalData->MCSTxPowerLevelOriginalOffset[pHalData->pwrGroupCnt][5]));
		 */
		if(rtlpriv->phy.rf_type== RF_1T1R) {
			rtlphy->pwrgroup_cnt++;
			/* RT_TRACE(COMP_INIT, DBG_TRACE, ("pwrGroupCnt = %d\n", pHalData->pwrGroupCnt)); */
		}
	}
	if (RegAddr == rTxAGC_B_Rate18_06) {
		rtlphy->mcs_txpwrlevel_origoffset[rtlphy->pwrgroup_cnt][8] = Data;
		/*
		 * RT_TRACE(COMP_INIT, DBG_TRACE, ("MCSTxPowerLevelOriginalOffset[%d][8] = 0x%lx\n", pHalData->pwrGroupCnt,
		 * 	pHalData->MCSTxPowerLevelOriginalOffset[pHalData->pwrGroupCnt][8]));
		 */
	}
	if (RegAddr == rTxAGC_B_Rate54_24) {
		rtlphy->mcs_txpwrlevel_origoffset[rtlphy->pwrgroup_cnt][9] = Data;
		/*
		 * RT_TRACE(COMP_INIT, DBG_TRACE, ("MCSTxPowerLevelOriginalOffset[%d][9] = 0x%lx\n", pHalData->pwrGroupCnt,
		 *	pHalData->MCSTxPowerLevelOriginalOffset[pHalData->pwrGroupCnt][9]));
		 */
	}
	if (RegAddr == rTxAGC_B_CCK1_55_Mcs32) {
		rtlphy->mcs_txpwrlevel_origoffset[rtlphy->pwrgroup_cnt][14] = Data;
		/*
		 * RT_TRACE(COMP_INIT, DBG_TRACE, ("MCSTxPowerLevelOriginalOffset[%d][14] = 0x%lx\n", pHalData->pwrGroupCnt,
		 * 	pHalData->MCSTxPowerLevelOriginalOffset[pHalData->pwrGroupCnt][14]));
		 */
	}
	if (RegAddr == rTxAGC_B_CCK11_A_CCK2_11 && BitMask == 0x000000ff) {
		rtlphy->mcs_txpwrlevel_origoffset[rtlphy->pwrgroup_cnt][15] = Data;
		/*
		 * RT_TRACE(COMP_INIT, DBG_TRACE, ("MCSTxPowerLevelOriginalOffset[%d][15] = 0x%lx\n", pHalData->pwrGroupCnt,
		 * 	pHalData->MCSTxPowerLevelOriginalOffset[pHalData->pwrGroupCnt][15]));
		 */
	}
	if (RegAddr == rTxAGC_B_Mcs03_Mcs00) {
		rtlphy->mcs_txpwrlevel_origoffset[rtlphy->pwrgroup_cnt][10] = Data;
		/*
		 * RT_TRACE(COMP_INIT, DBG_TRACE, ("MCSTxPowerLevelOriginalOffset[%d][10] = 0x%lx\n", pHalData->pwrGroupCnt,
		 * 	pHalData->MCSTxPowerLevelOriginalOffset[pHalData->pwrGroupCnt][10]));
		 */
	}
	if (RegAddr == rTxAGC_B_Mcs07_Mcs04) {
		rtlphy->mcs_txpwrlevel_origoffset[rtlphy->pwrgroup_cnt][11] = Data;
		/*
		 * RT_TRACE(COMP_INIT, DBG_TRACE, ("MCSTxPowerLevelOriginalOffset[%d][11] = 0x%lx\n", pHalData->pwrGroupCnt,
		 * 	pHalData->MCSTxPowerLevelOriginalOffset[pHalData->pwrGroupCnt][11]));
		 */
	}
	if (RegAddr == rTxAGC_B_Mcs11_Mcs08) {
		rtlphy->mcs_txpwrlevel_origoffset[rtlphy->pwrgroup_cnt][12] = Data;
		/*
		 * RT_TRACE(COMP_INIT, DBG_TRACE, ("MCSTxPowerLevelOriginalOffset[%d][12] = 0x%lx\n", pHalData->pwrGroupCnt,
		 * 	pHalData->MCSTxPowerLevelOriginalOffset[pHalData->pwrGroupCnt][12]));
		 */
	}
	if (RegAddr == rTxAGC_B_Mcs15_Mcs12) {
		rtlphy->mcs_txpwrlevel_origoffset[rtlphy->pwrgroup_cnt][13] = Data;
		/*
		 * RT_TRACE(COMP_INIT, DBG_TRACE, ("MCSTxPowerLevelOriginalOffset[%d][13] = 0x%lx\n", pHalData->pwrGroupCnt,
		 * 	pHalData->MCSTxPowerLevelOriginalOffset[pHalData->pwrGroupCnt][13]));
		 */
		if(rtlpriv->phy.rf_type != RF_1T1R)
			rtlphy->pwrgroup_cnt++;
	}
}

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
		if ((power_index[i] % 2 == 1) &&  !IS_NORMAL_CHIP(rtlpriv->VersionID))
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

void rtl8821au_phy_set_bw_mode_callback(struct rtl_priv *rtlpriv);
void rtl8812au_fixspur(struct rtl_priv *rtlpriv, enum CHANNEL_WIDTH Bandwidth,
	u8 Channel);

static void rtl8821au_phy_sw_chnl_callback(struct rtl_priv *rtlpriv)
{
	struct rtl_hal *rtlhal = rtl_hal(rtlpriv);

	uint8_t	eRFPath = 0;
	struct _rtw_hal	*pHalData = GET_HAL_DATA(rtlpriv);
	uint8_t	channelToSW = rtlpriv->phy.current_channel;

	if(phy_SwBand8812(rtlpriv, channelToSW) == _FALSE) {
		DBG_871X("error Chnl %d !\n", channelToSW);
	}

	/* DBG_871X("[BW:CHNL], phy_SwChnl8812(), switch to channel %d !!\n", channelToSW); */

	/* fc_area */
	if (36 <= channelToSW && channelToSW <= 48)
		rtl_set_bbreg(rtlpriv, rFc_area_Jaguar, 0x1ffe0000, 0x494);
	else if (50 <= channelToSW && channelToSW <= 64)
		rtl_set_bbreg(rtlpriv, rFc_area_Jaguar, 0x1ffe0000, 0x453);
	else if (100 <= channelToSW && channelToSW <= 116)
		rtl_set_bbreg(rtlpriv, rFc_area_Jaguar, 0x1ffe0000, 0x452);
	else if (118 <= channelToSW)
		rtl_set_bbreg(rtlpriv, rFc_area_Jaguar, 0x1ffe0000, 0x412);
	else
		rtl_set_bbreg(rtlpriv, rFc_area_Jaguar, 0x1ffe0000, 0x96a);

	for (eRFPath = 0; eRFPath <  rtlpriv->phy.num_total_rfpath; eRFPath++) {
		/* [2.4G] LC Tank */
		if (IS_VENDOR_8812A_TEST_CHIP(rtlpriv)) {
			if (1 <= channelToSW && channelToSW <= 7)
				rtw_hal_write_rfreg(rtlpriv, eRFPath, RF_TxLCTank_Jaguar, bLSSIWrite_data_Jaguar, 0x0017e);
			else if (8 <= channelToSW && channelToSW <= 14)
				rtw_hal_write_rfreg(rtlpriv, eRFPath, RF_TxLCTank_Jaguar, bLSSIWrite_data_Jaguar, 0x0013e);
		}

		/* RF_MOD_AG */
		if (36 <= channelToSW && channelToSW <= 64)
			rtw_hal_write_rfreg(rtlpriv, eRFPath, RF_CHNLBW_Jaguar, BIT18|BIT17|BIT16|BIT9|BIT8, 0x101); //5'b00101);
		else if (100 <= channelToSW && channelToSW <= 140)
			rtw_hal_write_rfreg(rtlpriv, eRFPath, RF_CHNLBW_Jaguar, BIT18|BIT17|BIT16|BIT9|BIT8, 0x301); //5'b01101);
		else if (140 < channelToSW)
			rtw_hal_write_rfreg(rtlpriv, eRFPath, RF_CHNLBW_Jaguar, BIT18|BIT17|BIT16|BIT9|BIT8, 0x501); //5'b10101);
		else
			rtw_hal_write_rfreg(rtlpriv, eRFPath, RF_CHNLBW_Jaguar, BIT18|BIT17|BIT16|BIT9|BIT8, 0x000); //5'b00000);

		/* <20121109, Kordan> A workaround for 8812A only. */
		rtl8812au_fixspur(rtlpriv, rtlpriv->phy.current_chan_bw, channelToSW);

		rtw_hal_write_rfreg(rtlpriv, eRFPath, RF_CHNLBW_Jaguar, MASKBYTE0, channelToSW);

		/* <20130104, Kordan> APK for MP chip is done on initialization from folder. */
		if (IS_HARDWARE_TYPE_8811AU(rtlhal) && ( !IS_NORMAL_CHIP(rtlpriv->VersionID)) && channelToSW > 14 ) {
			/* <20121116, Kordan> For better result of APK. Asked by AlexWang. */
			if (36 <= channelToSW && channelToSW <= 64)
				rtw_hal_write_rfreg(rtlpriv, eRFPath, RF_APK_Jaguar, bRFRegOffsetMask, 0x710E7);
			else if (100 <= channelToSW && channelToSW <= 140)
				rtw_hal_write_rfreg(rtlpriv, eRFPath, RF_APK_Jaguar, bRFRegOffsetMask, 0x716E9);
			else
				rtw_hal_write_rfreg(rtlpriv, eRFPath, RF_APK_Jaguar, bRFRegOffsetMask, 0x714E9);
		} else if ((IS_HARDWARE_TYPE_8821S(rtlhal))
			      && channelToSW > 14) {
			/* <20130111, Kordan> For better result of APK. Asked by Willson. */
			if (36 <= channelToSW && channelToSW <= 64)
				rtw_hal_write_rfreg(rtlpriv, eRFPath, RF_APK_Jaguar, bRFRegOffsetMask, 0x714E9);
			else if (100 <= channelToSW && channelToSW <= 140)
				rtw_hal_write_rfreg(rtlpriv, eRFPath, RF_APK_Jaguar, bRFRegOffsetMask, 0x110E9);
			else
				rtw_hal_write_rfreg(rtlpriv, eRFPath, RF_APK_Jaguar, bRFRegOffsetMask, 0x714E9);
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
	uint8_t			tmpCenterFrequencyIndex1 =pHalData->CurrentCenterFrequencyIndex1;
	struct mlme_ext_priv	*pmlmeext = &rtlpriv->mlmeextpriv;

	BOOLEAN bSwChnl = _FALSE, bSetChnlBW = _FALSE;


	/* DBG_871X("=> PHY_HandleSwChnlAndSetBW8812: bSwitchChannel %d, bSetBandWidth %d \n",bSwitchChannel,bSetBandWidth); */

	/* check is swchnl or setbw */
	if(!bSwitchChannel && !bSetBandWidth) {
		DBG_871X("PHY_HandleSwChnlAndSetBW8812:  not switch channel and not set bandwidth \n");
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
			(pHalData->CurrentCenterFrequencyIndex1!= CenterFrequencyIndex1)) {

			bSetChnlBW = _TRUE;
		}
	}

	if(!bSetChnlBW && !bSwChnl) {
		/* DBG_871X("<= PHY_HandleSwChnlAndSetBW8812: bSwChnl %d, bSetChnlBW %d \n",bSwChnl,bSetChnlBW); */
		return;
	}


	if(bSwChnl) {
		rtlpriv->phy.current_channel = ChannelNum;
		pHalData->CurrentCenterFrequencyIndex1 = ChannelNum;
	}


	if(bSetChnlBW) {
		rtlpriv->phy.current_chan_bw = ChnlWidth;
		mac->cur_40_prime_sc = ChnlOffsetOf40MHz;
		mac->cur_80_prime_sc = ChnlOffsetOf80MHz;

		pHalData->CurrentCenterFrequencyIndex1 = CenterFrequencyIndex1;
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
			rtl8821au_phy_set_bw_mode_callback(rtlpriv);
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
			pHalData->CurrentCenterFrequencyIndex1 = tmpChannel;
		}
		if(bSetChnlBW) {
			rtlpriv->phy.current_chan_bw = tmpBW;
			mac->cur_40_prime_sc = tmpnCur40MhzPrimeSC;
			mac->cur_80_prime_sc = tmpnCur80MhzPrimeSC;
			pHalData->CurrentCenterFrequencyIndex1 = tmpCenterFrequencyIndex1;
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


