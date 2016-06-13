#ifdef CONFIG_RTLWIFI

#include <../drivers/net/wireless/realtek/rtlwifi/wifi.h>

#else

#include <drv_types.h>
#include <rtl8812a_hal.h>
#include "rf.h"
#include "reg.h"
#include "phy.h"
#include "dm.h"

#endif

void rtl8821au_phy_rf6052_set_bandwidth(struct rtl_priv *rtlpriv, enum CHANNEL_WIDTH	Bandwidth)	/* 20M or 40M */
{
	switch (Bandwidth) {
	case CHANNEL_WIDTH_20:
		rtl_set_rfreg(rtlpriv, RF90_PATH_A, RF_CHNLBW_Jaguar, BIT(11)|BIT(10), 3);
		rtl_set_rfreg(rtlpriv, RF90_PATH_B, RF_CHNLBW_Jaguar, BIT(11)|BIT(10), 3);
		break;
	case CHANNEL_WIDTH_40:
		rtl_set_rfreg(rtlpriv, RF90_PATH_A, RF_CHNLBW_Jaguar, BIT(11)|BIT(10), 1);
		rtl_set_rfreg(rtlpriv, RF90_PATH_B, RF_CHNLBW_Jaguar, BIT(11)|BIT(10), 1);
		break;

	case CHANNEL_WIDTH_80:
		rtl_set_rfreg(rtlpriv, RF90_PATH_A, RF_CHNLBW_Jaguar, BIT(11)|BIT(10), 0);
		rtl_set_rfreg(rtlpriv, RF90_PATH_B, RF_CHNLBW_Jaguar, BIT(11)|BIT(10), 0);
		break;

	default:
		RT_TRACE(rtlpriv, COMP_ERR, DBG_LOUD,
			 "rtl8821au_phy_rf6052_set_bandwidth(): unknown Bandwidth: %#X\n", Bandwidth);
		break;
	}
}

static void _rtl8821au_write_ofdm_power_reg(struct rtl_priv *rtlpriv,
					    u8 index, u32 *pvalue)
{
	u16 regoffset_a[6] = {
		RTXAGC_A_OFDM18_OFDM6,
		RTXAGC_A_OFDM54_OFDM24,
		RTXAGC_A_MCS03_MCS00,
		RTXAGC_A_MCS07_MCS04,
		RTXAGC_A_MCS11_MCS08,
		RTXAGC_A_MCS15_MCS12
	};
	u16 regoffset_b[6] = {
		RTXAGC_B_OFDM18_OFDM6,
		RTXAGC_B_OFDM54_OFDM24,
		RTXAGC_B_MCS03_MCS00,
		RTXAGC_B_MCS07_MCS04,
		RTXAGC_B_MCS11_MCS08,
		RTXAGC_B_MCS15_MCS12
	};
	u8 i, rf, pwr_val[4];
	u32 writeval;
	u16 regoffset;

	for(rf = 0; rf < 2; rf++) {
		writeval = pvalue[rf];

		for(i = 0; i < RF_PATH_MAX_92C_88E; i++) {	/* ULLI BUG wrong path */
			pwr_val[i] =
				(u8) ((writeval & (0x7f << (i*8))) >> (i*8));
			if (pwr_val[i]  > RF6052_MAX_TX_PWR)
				pwr_val[i]  = RF6052_MAX_TX_PWR;
		}
		writeval = (pwr_val[3]<<24) |
			(pwr_val[2]<<16) |(pwr_val[1]<<8) |pwr_val[0];

		if(rf == 0)
			regoffset = regoffset_a[index];
		else
			regoffset = regoffset_b[index];

		rtl_set_bbreg(rtlpriv, regoffset, bMaskDWord, writeval);
		//RTPRINT(FPHY, PHY_TXPWR, ("Set 0x%x = %08x\n", RegOffset, writeVal));
	}
}

//
// powerbase0 for OFDM rates
// powerbase1 for HT MCS rates
//
void rtl8821au_phy_get_power_base(struct rtl_priv *rtlpriv,
					 u8 *ppowerlevel_ofdm,
					 u8 *ppowerlevel_bw20,
					 u8 *ppowerlevel_bw40, u8 channel,
					 u32 *ofdmbase, u32 *mcsbase)
{
	u32 powerbase0, powerbase1;
	u8 i, powerlevel[2];		/* Ulli : MAX RF Path for 8814AU -> 4 */

	for (i = 0; i < 2; i++) {
		powerbase0 = ppowerlevel_ofdm[i];

		powerbase0 = (powerbase0<<24) | (powerbase0<<16) |
				(powerbase0<<8) |powerbase0;
		*(ofdmbase+i) = powerbase0;
		//DBG_871X(" [OFDM power base index rf(%c) = 0x%x]\n", ((i==0)?'A':'B'), *(OfdmBase+i));
	}

	for (i = 0; i < rtlpriv->phy.num_total_rfpath; i++) {
		//Check HT20 to HT40 diff
		if(rtlpriv->phy.current_chan_bw == CHANNEL_WIDTH_20)
			powerlevel[i] = ppowerlevel_bw20[i];
		else
			powerlevel[i] = ppowerlevel_bw40[i];

		powerbase1 = powerlevel[i];
		powerbase1 = (powerbase1<<24) | (powerbase1<<16) |
				(powerbase1<<8) |powerbase1;
		*(mcsbase+i) = powerbase1;
		//DBG_871X(" [MCS power base index rf(%c) = 0x%x]\n", ((i==0)?'A':'B'), *(MCSBase+i));
	}
}

void get_txpower_writeval_by_regulatory(struct rtl_priv *rtlpriv,
					       u8 channel, u8 index,
					       u32 *powerbase0,
					       u32 *powerbase1,
					       u32 *p_outwriteval)
{
	struct rtl_phy *rtlphy = &(rtlpriv->phy);
	struct rtl_efuse *efuse = rtl_efuse(rtlpriv);
	u8 i, chnlgroup = 0, pwr_diff_limit[4], pwr_diff = 0;
	u32 writeval, rf;

	//
	// Index 0 & 1= legacy OFDM, 2-5=HT_MCS rate
	//

	for (rf = 0; rf < 2; rf++) {
		switch(efuse->eeprom_regulatory) {
		case 0:	// Realtek better performance
				// increase power diff defined by Realtek for large power
			chnlgroup = 0;
			//RTPRINT(FPHY, PHY_TXPWR, ("MCSTxPowerLevelOriginalOffset[%d][%d] = 0x%x\n",
			//	chnlGroup, index, pHalData->MCSTxPowerLevelOriginalOffset[chnlGroup][index+(rf?8:0)]));
			writeval =
				rtlphy->mcs_txpwrlevel_origoffset[chnlgroup][index+(rf?8:0)] +
					((index<2) ? powerbase0[rf] : powerbase1[rf]);
			//RTPRINT(FPHY, PHY_TXPWR, ("RTK better performance, writeVal(%c) = 0x%x\n", ((rf==0)?'A':'B'), writeVal));
			break;
		case 1:	// Realtek regulatory
				// increase power diff defined by Realtek for regulatory
			if(rtlphy->pwrgroup_cnt == 1)
				chnlgroup = 0;
			//if(rtlphy->pwrgroup_cnt >= MAX_PG_GROUP)
			if (channel < 3)		// Chanel 1-2
				chnlgroup = 0;
			else if (channel < 6)		// Channel 3-5
				chnlgroup = 1;
			else if(channel < 9)		// Channel 6-8
				chnlgroup = 2;
			else if(channel < 12)		// Channel 9-11
				chnlgroup = 3;
			else if(channel < 14)		// Channel 12-13
				chnlgroup = 4;
			else if(channel == 14)		// Channel 14
				chnlgroup = 5;

/*
			if(Channel <= 3)
				chnlGroup = 0;
			else if(Channel >= 4 && Channel <= 9)
				chnlGroup = 1;
			else if(Channel > 9)
				chnlGroup = 2;


			if(pHalData->CurrentChannelBW == CHANNEL_WIDTH_20)
				chnlGroup++;
			else
				chnlGroup+=4;
*/
			//RTPRINT(FPHY, PHY_TXPWR, ("MCSTxPowerLevelOriginalOffset[%d][%d] = 0x%x\n",
			//chnlGroup, index, pHalData->MCSTxPowerLevelOriginalOffset[chnlGroup][index+(rf?8:0)]));
			writeval = rtlphy->mcs_txpwrlevel_origoffset[chnlgroup][index+(rf?8:0)] +
					((index<2) ? powerbase0[rf] : powerbase1[rf]);
			//RTPRINT(FPHY, PHY_TXPWR, ("Realtek regulatory, 20MHz, writeVal(%c) = 0x%x\n", ((rf==0)?'A':'B'), writeVal));
			break;
		case 2:	// Better regulatory
				// don't increase any power diff
			writeval = ((index<2) ? powerbase0[rf] : powerbase1[rf]);
			//RTPRINT(FPHY, PHY_TXPWR, ("Better regulatory, writeVal(%c) = 0x%x\n", ((rf==0)?'A':'B'), writeVal));
			break;
		default:
			chnlgroup = 0;
			writeval = rtlphy->mcs_txpwrlevel_origoffset[chnlgroup][index+(rf?8:0)] +
					((index<2) ? powerbase0[rf] : powerbase1[rf]);
			//RTPRINT(FPHY, PHY_TXPWR, ("RTK better performance, writeVal rf(%c) = 0x%x\n", ((rf==0)?'A':'B'), writeVal));
			break;
		}

// 20100427 Joseph: Driver dynamic Tx power shall not affect Tx power. It shall be determined by power training mechanism.
// Currently, we cannot fully disable driver dynamic tx power mechanism because it is referenced by BT coexist mechanism.
// In the future, two mechanism shall be separated from each other and maintained independantly. Thanks for Lanhsin's reminder.
		//92d do not need this
		if (rtlpriv->dm.dynamic_txhighpower_lvl == TxHighPwrLevel_Level1)
			writeval = 0x14141414;
		else if (rtlpriv->dm.dynamic_txhighpower_lvl == TxHighPwrLevel_Level2)
			writeval = 0x00000000;

		// 20100628 Joseph: High power mode for BT-Coexist mechanism.
		// This mechanism is only applied when Driver-Highpower-Mechanism is OFF.
		if (rtlpriv->dm.dynamic_txhighpower_lvl == TxHighPwrLevel_BT1) {
			//RTPRINT(FBT, BT_TRACE, ("Tx Power (-6)\n"));
			writeval = writeval - 0x06060606;
		} else if(rtlpriv->dm.dynamic_txhighpower_lvl == TxHighPwrLevel_BT2) {
			//RTPRINT(FBT, BT_TRACE, ("Tx Power (-0)\n"));
			writeval = writeval ;
		}
		/*
		if(pMgntInfo->bDisableTXPowerByRate) {
		// add for  OID_RT_11N_TX_POWER_BY_RATE ,disable tx powre change by rate
			writeVal = 0x2c2c2c2c;
		}
		*/
		*(p_outwriteval+rf) = writeval;
	}
}

void rtl8821au_phy_rf6052_set_ofdm_txpower(struct rtl_priv *rtlpriv,
					   u8 *ppowerlevel_ofdm,
					   u8 *ppowerlevel_bw20,
					   u8 *ppowerlevel_bw40,
					   u8 channel)
{
	u32 writeval[2], powerbase0[2], powerbase1[2];
	u8 index;

	//DBG_871X("PHY_RF6052SetOFDMTxPower, channel(%d) \n", Channel);

	rtl8821au_phy_get_power_base(rtlpriv, ppowerlevel_ofdm, ppowerlevel_bw20,
			 ppowerlevel_bw40, channel, &powerbase0[0], &powerbase1[0]);

	for(index = 0; index < 6; index++) {
		get_txpower_writeval_by_regulatory(rtlpriv, channel, index,
			&powerbase0[0], &powerbase1[0], &writeval[0]);

		_rtl8821au_write_ofdm_power_reg(rtlpriv, index, &writeval[0]);
	}
}

static bool _rtl8821au_phy_rf6052_config_parafile(struct rtl_priv *rtlpriv)
{
	struct rtl_hal *rtlhal = rtl_hal(rtlpriv);
	u8 rfpath;
	bool rtstatus = true;

	for (rfpath = 0; rfpath < rtlpriv->phy.num_total_rfpath; rfpath++) {
		switch (rfpath) {
		case RF90_PATH_A :
			if (rtlhal->hw_type == HARDWARE_TYPE_RTL8812AU)
				rtstatus =
				  rtl8812au_phy_config_rf_with_headerfile(rtlpriv,
						(enum radio_path) rfpath);
			else
				rtstatus =
				  rtl8821au_phy_config_rf_with_headerfile(rtlpriv,
						(enum radio_path) rfpath);

			break;
		case RF90_PATH_B :
			if (rtlhal->hw_type == HARDWARE_TYPE_RTL8812AU)
				rtstatus =
				  rtl8812au_phy_config_rf_with_headerfile(rtlpriv,
						(enum radio_path) rfpath);
			else
				rtstatus =
				  rtl8821au_phy_config_rf_with_headerfile(rtlpriv,
						(enum radio_path) rfpath);

		case RF90_PATH_C :
			break;
		case RF90_PATH_D :
			break;

		}
		if (!rtstatus) {
			RT_TRACE(rtlpriv, COMP_INIT, DBG_TRACE,
				 "Radio[%d] Fail!!", rfpath);
			return false;
		}
	}

	/*put arrays in dm.c*/
	RT_TRACE(rtlpriv, COMP_INIT, DBG_TRACE, "\n");

	return rtstatus;
}

bool rtl8821au_phy_rf6052_config(struct rtl_priv *rtlpriv)
{
	/* Initialize general global value */
	if (rtlpriv->phy.rf_type == RF_1T1R)
		rtlpriv->phy.num_total_rfpath = 1;
	else
		rtlpriv->phy.num_total_rfpath = 2;

	/*
	 * Config BB and RF
	 */

	return _rtl8821au_phy_rf6052_config_parafile(rtlpriv);
}

/* currently noz used, as in rtl8821ae */

void rtl8821au_phy_rf6052_set_cck_txpower(struct rtl_priv *rtlpriv,
					  u8 *ppowerlevel)
{
	struct rtl_phy *rtlphy = &(rtlpriv->phy);
	struct rtl_efuse *efuse = rtl_efuse(rtlpriv);
	struct rtl_hal *rtlhal = rtl_hal(rtlpriv);
	struct mlme_ext_priv 	*pmlmeext = &rtlpriv->mlmeextpriv;
	u32 tx_agc[2] = {0, 0}, tmpval;			/* ULLI : change for 4 tx ?? */
	bool turbo_scanoff = false;
	u8 idx1, idx2;
	u8 *ptr;

	/* FOR CE ,must disable turbo scan */
	turbo_scanoff = true;

	if (pmlmeext->sitesurvey_res.state == SCAN_PROCESS) {
		tx_agc[RF90_PATH_A] = 0x3f3f3f3f;
		tx_agc[RF90_PATH_B] = 0x3f3f3f3f;

		turbo_scanoff = true;	/* disable turbo scan */

		if (turbo_scanoff) {
			for (idx1 = RF90_PATH_A; idx1 <= RF90_PATH_B; idx1++) {
				tx_agc[idx1] =
					ppowerlevel[idx1] | (ppowerlevel[idx1]<<8) |
					(ppowerlevel[idx1]<<16) | (ppowerlevel[idx1]<<24);
				/* 2010/10/18 MH For external PA module. We need to limit power index to be less than 0x20. */
				if (tx_agc[idx1] > 0x20 && rtlhal->external_pa_5g)
					tx_agc[idx1] = 0x20;
			}
		}
	} else {
/*
 * 20100427 Joseph: Driver dynamic Tx power shall not affect Tx power. It shall be determined by power training mechanism.
 * Currently, we cannot fully disable driver dynamic tx power mechanism because it is referenced by BT coexist mechanism.
 * In the future, two mechanism shall be separated from each other and maintained independantly. Thanks for Lanhsin's reminder.
 */
		if (rtlpriv->dm.dynamic_txhighpower_lvl == TxHighPwrLevel_Level1) {
			tx_agc[RF90_PATH_A] = 0x10101010;
			tx_agc[RF90_PATH_B] = 0x10101010;
		} else if (rtlpriv->dm.dynamic_txhighpower_lvl == TxHighPwrLevel_Level2) {
			tx_agc[RF90_PATH_A] = 0x00000000;
			tx_agc[RF90_PATH_B] = 0x00000000;
		} else {
			for (idx1 = RF90_PATH_A; idx1 <= RF90_PATH_B; idx1++) {
				tx_agc[idx1] =
					ppowerlevel[idx1] | (ppowerlevel[idx1]<<8) |
					(ppowerlevel[idx1]<<16) | (ppowerlevel[idx1]<<24);
			}

			if (efuse->eeprom_regulatory == 0) {
				tmpval = (rtlphy->mcs_txpwrlevel_origoffset[0][6]) +
						(rtlphy->mcs_txpwrlevel_origoffset[0][7]<<8);
				tx_agc[RF90_PATH_A] += tmpval;

				tmpval = (rtlphy->mcs_txpwrlevel_origoffset[0][14]) +
						(rtlphy->mcs_txpwrlevel_origoffset[0][15]<<24);
				tx_agc[RF90_PATH_B] += tmpval;
			}
		}
	}

	for (idx1 = RF90_PATH_A; idx1 <= RF90_PATH_B; idx1++) {
		ptr = (u8 *)(&(tx_agc[idx1]));
		for (idx2 = 0; idx2 < 4; idx2++) {
			if (*ptr > RF6052_MAX_TX_PWR)
				*ptr = RF6052_MAX_TX_PWR;
			ptr++;
		}
	}

	/* rf-A cck tx power */
	tmpval = tx_agc[RF90_PATH_A] & 0xff;
	rtl_set_bbreg(rtlpriv, RTXAGC_A_CCK11_CCK1, MASKBYTE1, tmpval);
	/* RT_DISP(FPHY, PHY_TXPWR, ("CCK PWR 1M (rf-A) = 0x%x (reg 0x%x)\n", tmpval, rTxAGC_A_CCK1_Mcs32)); */
	tmpval = tx_agc[RF90_PATH_A] >> 8;
	rtl_set_bbreg(rtlpriv, RTXAGC_A_CCK11_CCK1, 0xffffff00, tmpval);
	/* RT_DISP(FPHY, PHY_TXPWR, ("CCK PWR 2~11M (rf-A) = 0x%x (reg 0x%x)\n", tmpval, rTxAGC_B_CCK11_A_CCK2_11)); */

	/* rf-B cck tx power */
	tmpval = tx_agc[RF90_PATH_B] >> 24;
	rtl_set_bbreg(rtlpriv, RTXAGC_B_CCK11_CCK1, MASKBYTE0, tmpval);
	/* RT_DISP(FPHY, PHY_TXPWR, ("CCK PWR 11M (rf-B) = 0x%x (reg 0x%x)\n", tmpval, rTxAGC_B_CCK11_A_CCK2_11)); */
	tmpval = tx_agc[RF90_PATH_B] & 0x00ffffff;
	rtl_set_bbreg(rtlpriv, RTXAGC_B_CCK11_CCK1, 0xffffff00, tmpval);
	/* RT_DISP(FPHY, PHY_TXPWR, ("CCK PWR 1~5.5M (rf-B) = 0x%x (reg 0x%x)\n", tmpval, rTxAGC_B_CCK1_55_Mcs32)); */

}	/* PHY_RF6052SetCckTxPower */


