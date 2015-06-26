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
#define _RTL8812A_HAL_INIT_C_
#include <rtl8812a_hal.h>
#include <rtw_debug.h>

/*
 *-------------------------------------------------------------------------
 *
 * LLT R/W/Init function
 *
 *-------------------------------------------------------------------------
 */

/*====================================================================================
 *
 * 20100209 Joseph:
 * This function is used only for 92C to set REG_BCN_CTRL(0x550) register.
 * We just reserve the value of the register in variable pHalData->RegBcnCtrlVal and then operate
 * the value of the register via atomic operation.
 * This prevents from race condition when setting this register.
 * The value of pHalData->RegBcnCtrlVal is initialized in HwConfigureRTL8192CE() function.
 *
 */

void rtl8821au_set_bcn_ctrl_reg(struct rtl_priv *rtlpriv, uint8_t SetBits, uint8_t ClearBits)
{
	struct rtl_usb *rtlusb = rtl_usbdev(rtlpriv);

	rtlusb->reg_bcn_ctrl_val |= SetBits;
	rtlusb->reg_bcn_ctrl_val &= ~ClearBits;

	rtl_write_byte(rtlpriv, REG_BCN_CTRL, (uint8_t)rtlusb->reg_bcn_ctrl_val);
}


void _8051Reset8812(struct rtl_priv *rtlpriv)
{
	struct rtl_hal *rtlhal = rtl_hal(rtlpriv);
	uint8_t u1bTmp, u1bTmp2;

	/* Reset MCU IO Wrapper- sugggest by SD1-Gimmy */
	if (IS_HARDWARE_TYPE_8812(rtlhal)) {
		u1bTmp2 = rtl_read_byte(rtlpriv, REG_RSV_CTRL+1);
		rtl_write_byte(rtlpriv, REG_RSV_CTRL + 1, u1bTmp2&(~BIT3));
	} else if (IS_HARDWARE_TYPE_8821(rtlhal)) {
		u1bTmp2 = rtl_read_byte(rtlpriv, REG_RSV_CTRL+1);
		rtl_write_byte(rtlpriv, REG_RSV_CTRL + 1, u1bTmp2&(~BIT0));
	}

	u1bTmp = rtl_read_byte(rtlpriv, REG_SYS_FUNC_EN+1);
	rtl_write_byte(rtlpriv, REG_SYS_FUNC_EN+1, u1bTmp&(~BIT2));

	/* Enable MCU IO Wrapper */
	if (IS_HARDWARE_TYPE_8812(rtlhal)) {
		u1bTmp2 = rtl_read_byte(rtlpriv, REG_RSV_CTRL+1);
		rtl_write_byte(rtlpriv, REG_RSV_CTRL + 1, u1bTmp2 | (BIT3));
	} else if (IS_HARDWARE_TYPE_8821(rtlhal)) {
		u1bTmp2 = rtl_read_byte(rtlpriv, REG_RSV_CTRL+1);
		rtl_write_byte(rtlpriv, REG_RSV_CTRL + 1, u1bTmp2 | (BIT0));
	}

	rtl_write_byte(rtlpriv, REG_SYS_FUNC_EN+1, u1bTmp|(BIT2));

	DBG_871X("=====> _8051Reset8812(): 8051 reset success .\n");
}


void InitializeFirmwareVars8812(struct rtl_priv *rtlpriv)
{
	struct _rtw_hal *pHalData = GET_HAL_DATA(rtlpriv);
	struct pwrctrl_priv *pwrpriv;
	pwrpriv = &rtlpriv->pwrctrlpriv;

	/* Init Fw LPS related. */
	rtlpriv->pwrctrlpriv.bFwCurrentInPSMode = _FALSE;
	/* Init H2C counter. by tynli. 2009.12.09. */
	pHalData->LastHMEBoxNum = 0;
}

void rtl8812_free_hal_data(struct rtl_priv *rtlpriv)
{
	if (rtlpriv->HalData) {
			rtw_mfree(rtlpriv->HalData);
		rtlpriv->HalData = NULL;
	}
}

/*
 * ===========================================================
 * 				Efuse related code
 * ===========================================================
 */
BOOLEAN Hal_GetChnlGroup8812A(uint8_t Channel, uint8_t *pGroup)
{
	BOOLEAN bIn24G = _TRUE;

	if (Channel <= 14) {
		bIn24G = _TRUE;

		if (1 <= Channel && Channel <= 2)
			*pGroup = 0;
		else if (3  <= Channel && Channel <= 5)
			*pGroup = 1;
		else if (6  <= Channel && Channel <= 8)
			*pGroup = 2;
		else if (9  <= Channel && Channel <= 11)
			*pGroup = 3;
		else if (12 <= Channel && Channel <= 14)
			*pGroup = 4;
		else {
			DBG_871X("==>mpt_GetChnlGroup8812A in 2.4 G, but Channel %d in Group not found \n", Channel);
		}
	} else {
		bIn24G = _FALSE;

		if      (36   <= Channel && Channel <=  42)
			*pGroup = 0;
		else if (44   <= Channel && Channel <=  48)
			*pGroup = 1;
		else if (50   <= Channel && Channel <=  58)
			*pGroup = 2;
		else if (60   <= Channel && Channel <=  64)
			*pGroup = 3;
		else if (100  <= Channel && Channel <= 106)
			*pGroup = 4;
		else if (108  <= Channel && Channel <= 114)
			*pGroup = 5;
		else if (116  <= Channel && Channel <= 122)
			*pGroup = 6;
		else if (124  <= Channel && Channel <= 130)
			*pGroup = 7;
		else if (132  <= Channel && Channel <= 138)
			*pGroup = 8;
		else if (140  <= Channel && Channel <= 144)
			*pGroup = 9;
		else if (149  <= Channel && Channel <= 155)
			*pGroup = 10;
		else if (157  <= Channel && Channel <= 161)
			*pGroup = 11;
		else if (165  <= Channel && Channel <= 171)
			*pGroup = 12;
		else if (173  <= Channel && Channel <= 177)
			*pGroup = 13;
		else {
			DBG_871X("==>mpt_GetChnlGroup8812A in 5G, but Channel %d in Group not found \n", Channel);
		}

	}
	/* DBG_871X("<==mpt_GetChnlGroup8812A,  (%s) Channel = %d, Group =%d,\n", (bIn24G) ? "2.4G" : "5G", Channel, *pGroup); */

	return bIn24G;
}

static void _rtl8821au_read_power_value_fromprom(struct rtl_priv *rtlpriv,
	struct txpower_info_2g *pwrinfo24g,
	struct txpower_info_5g *pwrinfo5g,
	u8 *hwinfo,
	bool autoload_fail)
{
	 struct _rtw_hal	*pHalData = GET_HAL_DATA(rtlpriv);
	uint32_t rfPath, eeAddr = EEPROM_TX_PWR_INX_8812, group, TxCount = 0;

	memset(pwrinfo24g, 0, sizeof(*pwrinfo24g));
	memset(pwrinfo5g, 0, sizeof(*pwrinfo5g));

	/* DBG_871X("hal_ReadPowerValueFromPROM8812A(): PROMContent[0x%x]=0x%x\n", (eeAddr+1), PROMContent[eeAddr+1]); */
	if (0xFF == hwinfo[eeAddr+1])  /* YJ,add,120316 */
		autoload_fail = true;

	if (autoload_fail) {
		DBG_871X("hal_ReadPowerValueFromPROM8812A(): Use Default value!\n");
		for (rfPath = 0 ; rfPath < MAX_RF_PATH ; rfPath++) {
			/*  2.4G default value */
			for (group = 0 ; group < MAX_CHNL_GROUP_24G; group++) {
				pwrinfo24g->index_cck_base[rfPath][group] = EEPROM_DEFAULT_24G_INDEX;
				pwrinfo24g->index_bw40_base[rfPath][group] = EEPROM_DEFAULT_24G_INDEX;
			}
			for (TxCount = 0; TxCount < MAX_TX_COUNT; TxCount++) {
				if (TxCount == 0) {
					pwrinfo24g->bw20_diff[rfPath][0] = EEPROM_DEFAULT_24G_HT20_DIFF;
					pwrinfo24g->ofdm_diff[rfPath][0] = EEPROM_DEFAULT_24G_OFDM_DIFF;
				} else {
					pwrinfo24g->bw20_diff[rfPath][TxCount] = EEPROM_DEFAULT_DIFF;
					pwrinfo24g->bw40_diff[rfPath][TxCount] = EEPROM_DEFAULT_DIFF;
					pwrinfo24g->cck_diff[rfPath][TxCount] =	EEPROM_DEFAULT_DIFF;
					pwrinfo24g->ofdm_diff[rfPath][TxCount] = EEPROM_DEFAULT_DIFF;
				}
			}

			/* 5G default value */
			for (group = 0 ; group < MAX_CHNL_GROUP_5G; group++)
				pwrinfo5g->index_bw40_base[rfPath][group] = EEPROM_DEFAULT_5G_INDEX;

			for (TxCount = 0; TxCount < MAX_TX_COUNT; TxCount++) {
				if (TxCount == 0) {
					pwrinfo5g->ofdm_diff[rfPath][0] = EEPROM_DEFAULT_5G_OFDM_DIFF;
					pwrinfo5g->bw20_diff[rfPath][0] = EEPROM_DEFAULT_5G_HT20_DIFF;
					pwrinfo5g->bw80_diff[rfPath][0] = EEPROM_DEFAULT_DIFF;
					pwrinfo5g->bw160_diff[rfPath][0] = EEPROM_DEFAULT_DIFF;
				} else {
					/* ULLI check for-loop in _rtl8821ae_read_power_fromprom() */
					pwrinfo5g->ofdm_diff[rfPath][TxCount] =	EEPROM_DEFAULT_DIFF;
					pwrinfo5g->bw20_diff[rfPath][TxCount] =	EEPROM_DEFAULT_DIFF;
					pwrinfo5g->bw40_diff[rfPath][TxCount] =	EEPROM_DEFAULT_DIFF;
					pwrinfo5g->bw80_diff[rfPath][TxCount] =	EEPROM_DEFAULT_DIFF;
					pwrinfo5g->bw160_diff[rfPath][TxCount] = EEPROM_DEFAULT_DIFF;

				}
			}

		}

		/* pHalData->bNOPG = _TRUE; */
		return;
	}

	pHalData->bTXPowerDataReadFromEEPORM = _TRUE;		/* YJ,move,120316 */

	for (rfPath = 0; rfPath < MAX_RF_PATH; rfPath++) {
		/*  2.4G default value */
		for (group = 0; group < MAX_CHNL_GROUP_24G; group++) {
			pwrinfo24g->index_cck_base[rfPath][group] = hwinfo[eeAddr++];
			if (pwrinfo24g->index_cck_base[rfPath][group] == 0xFF) {
				pwrinfo24g->index_cck_base[rfPath][group] = EEPROM_DEFAULT_24G_INDEX;
				/* pHalData->bNOPG = _TRUE; */
			}
			/*
			 * DBG_871X("8812-2G RF-%d-G-%d CCK-Addr-%x BASE=%x\n",
			 * rfPath, group, eeAddr-1, pwrInfo24G->IndexCCK_Base[rfPath][group]);
			 */
		}

		for (group = 0; group < MAX_CHNL_GROUP_24G-1; group++) {
			pwrinfo24g->index_bw40_base[rfPath][group] = hwinfo[eeAddr++];
			if (pwrinfo24g->index_bw40_base[rfPath][group] == 0xFF)
				pwrinfo24g->index_bw40_base[rfPath][group] = EEPROM_DEFAULT_24G_INDEX;
			/*
			 * DBG_871X("8812-2G RF-%d-G-%d BW40-Addr-%x BASE=%x\n",
			 * rfPath, group, eeAddr-1, pwrInfo24G->IndexBW40_Base[rfPath][group]);
			 */
		}

		for (TxCount = 0; TxCount < MAX_TX_COUNT; TxCount++) {
			if (TxCount == 0) {
				pwrinfo24g->bw40_diff[rfPath][TxCount] = 0;

				pwrinfo24g->bw20_diff[rfPath][TxCount] = (hwinfo[eeAddr] & 0xf0) >> 4;
				if (pwrinfo24g->bw20_diff[rfPath][TxCount] & BIT3)		/* 4bit sign number to 8 bit sign number */
					pwrinfo24g->bw20_diff[rfPath][TxCount] |= 0xF0;

				/*
				 * DBG_871X("8812-2G RF-%d-SS-%d BW20-Addr-%x DIFF=%d\n",
				 * rfPath, TxCount, eeAddr, pwrInfo24G->BW20_Diff[rfPath][TxCount]);
				 */

				pwrinfo24g->ofdm_diff[rfPath][TxCount] =	(hwinfo[eeAddr]&0x0f);
				if (pwrinfo24g->ofdm_diff[rfPath][TxCount] & BIT3)		/* 4bit sign number to 8 bit sign number */
					pwrinfo24g->ofdm_diff[rfPath][TxCount] |= 0xF0;

				/*
				 * DBG_871X("8812-2G RF-%d-SS-%d LGOD-Addr-%x DIFF=%d\n",
				 * rfPath, TxCount, eeAddr, pwrInfo24G->OFDM_Diff[rfPath][TxCount]);
				 */

				pwrinfo24g->cck_diff[rfPath][TxCount] = 0;
				eeAddr++;
			} else {
				pwrinfo24g->bw40_diff[rfPath][TxCount] =	(hwinfo[eeAddr]&0xf0)>>4;
				if (pwrinfo24g->bw40_diff[rfPath][TxCount] & BIT3)		/* 4bit sign number to 8 bit sign number */
					pwrinfo24g->bw40_diff[rfPath][TxCount] |= 0xF0;

				/*
				 * DBG_871X("8812-2G RF-%d-SS-%d BW40-Addr-%x DIFF=%d\n",
				 * rfPath, TxCount, eeAddr, pwrInfo24G->BW40_Diff[rfPath][TxCount]);
				 */

				pwrinfo24g->bw20_diff[rfPath][TxCount] =	(hwinfo[eeAddr]&0x0f);
				if (pwrinfo24g->bw20_diff[rfPath][TxCount] & BIT3)		/* 4bit sign number to 8 bit sign number */
					pwrinfo24g->bw20_diff[rfPath][TxCount] |= 0xF0;

				/*
				 * DBG_871X("8812-2G RF-%d-SS-%d BW20-Addr-%x DIFF=%d\n",
				 * rfPath, TxCount, eeAddr, pwrInfo24G->BW20_Diff[rfPath][TxCount]);
				 */

				eeAddr++;


				pwrinfo24g->ofdm_diff[rfPath][TxCount] =	(hwinfo[eeAddr]&0xf0)>>4;
				if (pwrinfo24g->ofdm_diff[rfPath][TxCount] & BIT3)		/* 4bit sign number to 8 bit sign number */
					pwrinfo24g->ofdm_diff[rfPath][TxCount] |= 0xF0;

				/*
				 * DBG_871X("8812-2G RF-%d-SS-%d LGOD-Addr-%x DIFF=%d\n",
				 * rfPath, TxCount, eeAddr, pwrInfo24G->BW20_Diff[rfPath][TxCount]);
				 */

				pwrinfo24g->cck_diff[rfPath][TxCount] =	(hwinfo[eeAddr]&0x0f);
				if (pwrinfo24g->cck_diff[rfPath][TxCount] & BIT3)		/* 4bit sign number to 8 bit sign number */
					pwrinfo24g->cck_diff[rfPath][TxCount] |= 0xF0;
				/*
				 * DBG_871X("8812-2G RF-%d-SS-%d CCK-Addr-%x DIFF=%d\n",
				 * rfPath, TxCount, eeAddr, pwrInfo24G->CCK_Diff[rfPath][TxCount]);
				 */

				eeAddr++;
			}
		}

		/* 5G default value */
		for (group = 0 ; group < MAX_CHNL_GROUP_5G; group++) {
			pwrinfo5g->index_bw40_base[rfPath][group] =		hwinfo[eeAddr++];
			if (pwrinfo5g->index_bw40_base[rfPath][group] == 0xFF)
				pwrinfo5g->index_bw40_base[rfPath][group] = EEPROM_DEFAULT_DIFF;

			/*
			 * DBG_871X("8812-5G RF-%d-G-%d BW40-Addr-%x BASE=%x\n",
			 * rfPath, TxCount, eeAddr-1, pwrInfo5G->IndexBW40_Base[rfPath][group]);
			 */
		}

		for (TxCount = 0; TxCount < MAX_TX_COUNT; TxCount++) {
			if (TxCount == 0) {
				pwrinfo5g->bw40_diff[rfPath][TxCount] = 0;
				pwrinfo5g->bw20_diff[rfPath][0] = (hwinfo[eeAddr] & 0xf0) >> 4;
				if (pwrinfo5g->bw20_diff[rfPath][TxCount] & BIT3)		/* 4bit sign number to 8 bit sign number */
					pwrinfo5g->bw20_diff[rfPath][TxCount] |= 0xF0;
				/*
				 * DBG_871X("8812-5G RF-%d-SS-%d BW20-Addr-%x DIFF=%d\n",
				 * rfPath, TxCount, eeAddr, pwrInfo5G->BW20_Diff[rfPath][TxCount]);
				 */
				pwrinfo5g->ofdm_diff[rfPath][0] = (hwinfo[eeAddr] & 0x0f);
				if (pwrinfo5g->ofdm_diff[rfPath][TxCount] & BIT3)		/* 4bit sign number to 8 bit sign number */
					pwrinfo5g->ofdm_diff[rfPath][TxCount] |= 0xF0;
				/*
				 * DBG_871X("8812-5G RF-%d-SS-%d LGOD-Addr-%x DIFF=%d\n",
				 * rfPath, TxCount, eeAddr, pwrInfo5G->OFDM_Diff[rfPath][TxCount]);
				 */

				eeAddr++;
			} else {
				pwrinfo5g->bw40_diff[rfPath][TxCount] = (hwinfo[eeAddr] & 0xf0) >> 4;
				if (pwrinfo5g->bw40_diff[rfPath][TxCount] & BIT3)		/* 4bit sign number to 8 bit sign number */
					pwrinfo5g->bw40_diff[rfPath][TxCount] |= 0xF0;
				/*
				 * DBG_871X("8812-5G RF-%d-SS-%d BW40-Addr-%x DIFF=%d\n",
				 * rfPath, TxCount, eeAddr, pwrInfo5G->BW40_Diff[rfPath][TxCount]);
				 */

				pwrinfo5g->bw20_diff[rfPath][TxCount] = (hwinfo[eeAddr] & 0x0f);
				if (pwrinfo5g->bw20_diff[rfPath][TxCount] & BIT3)		/* 4bit sign number to 8 bit sign number */
					pwrinfo5g->bw20_diff[rfPath][TxCount] |= 0xF0;
				/*
				 * DBG_871X("8812-5G RF-%d-SS-%d BW20-Addr-%x DIFF=%d\n",
				 * rfPath, TxCount, eeAddr, pwrInfo5G->BW20_Diff[rfPath][TxCount]);
				 */

				eeAddr++;
			}
		}


		pwrinfo5g->ofdm_diff[rfPath][1] =	(hwinfo[eeAddr] & 0xf0) >> 4;
		pwrinfo5g->ofdm_diff[rfPath][2] =	(hwinfo[eeAddr] & 0x0f);
		/*
		 * DBG_871X("8812-5G RF-%d-SS-%d LGOD-Addr-%x DIFF=%d\n",
		 * rfPath, 2, eeAddr, pwrInfo5G->OFDM_Diff[rfPath][2]);
		 */
		eeAddr++;

		pwrinfo5g->ofdm_diff[rfPath][3] =	(hwinfo[eeAddr] & 0x0f);
		/*
		 * DBG_871X("8812-5G RF-%d-SS-%d LGOD-Addr-%x DIFF=%d\n",
		 * rfPath, 3, eeAddr, pwrInfo5G->OFDM_Diff[rfPath][3]);
		 */
		eeAddr++;

		for (TxCount = 1; TxCount < MAX_TX_COUNT; TxCount++) {
			if (pwrinfo5g->ofdm_diff[rfPath][TxCount] & BIT3)		/* 4bit sign number to 8 bit sign number */
				pwrinfo5g->ofdm_diff[rfPath][TxCount] |= 0xF0;

			/*
			 * DBG_871X("8812-5G RF-%d-SS-%d LGOD-Addr-%x DIFF=%d\n",
			 * rfPath, TxCount, eeAddr, pwrInfo5G->OFDM_Diff[rfPath][TxCount]);
			 */
		}

		for (TxCount = 0; TxCount < MAX_TX_COUNT; TxCount++) {
			pwrinfo5g->bw80_diff[rfPath][TxCount] =	(hwinfo[eeAddr] & 0xf0) >> 4;
			if (pwrinfo5g->bw80_diff[rfPath][TxCount] & BIT3)		/* 4bit sign number to 8 bit sign number */
				pwrinfo5g->bw80_diff[rfPath][TxCount] |= 0xF0;
			/*
			 * DBG_871X("8812-5G RF-%d-SS-%d BW80-Addr-%x DIFF=%d\n",
			 * rfPath, TxCount, eeAddr, pwrInfo5G->BW80_Diff[rfPath][TxCount]);
			 */
			pwrinfo5g->bw160_diff[rfPath][TxCount] =	(hwinfo[eeAddr] & 0x0f);
			if (pwrinfo5g->bw160_diff[rfPath][TxCount] & BIT3)		/* 4bit sign number to 8 bit sign number */
				pwrinfo5g->bw160_diff[rfPath][TxCount] |= 0xF0;
			/*
			 * DBG_871X("8812-5G RF-%d-SS-%d BW160-Addr-%x DIFF=%d\n",
			 * rfPath, TxCount, eeAddr, pwrInfo5G->BW160_Diff[rfPath][TxCount]);
			 */
			eeAddr++;
		}
	}

}

void Hal_EfuseParseIDCode8812A(struct rtl_priv *rtlpriv, u8 *hwinfo)
{
	struct rtl_efuse *efuse = rtl_efuse(rtlpriv);
	u16			EEPROMId;

	/*  Checl 0x8129 again for making sure autoload status!! */
	EEPROMId = le16_to_cpu(*((u16 *)hwinfo));
	if (EEPROMId != RTL_EEPROM_ID) {
		DBG_8192C("EEPROM ID(%#x) is invalid!!\n", EEPROMId);
		efuse->autoload_failflag = _TRUE;
	} else {
		efuse->autoload_failflag = _FALSE;
	}

	DBG_8192C("EEPROM ID=0x%04x\n", EEPROMId);
}

void Hal_ReadPROMVersion8812A(struct rtl_priv *rtlpriv, u8 *hwinfo,
	bool autoload_fail)
{
	struct rtl_efuse *efuse = rtl_efuse(rtlpriv);

	if (autoload_fail) {
		efuse->eeprom_version = EEPROM_Default_Version;
	} else{
		efuse->eeprom_version = hwinfo[EEPROM_VERSION_8812];
		if (efuse->eeprom_version == 0xFF)
			efuse->eeprom_version = EEPROM_Default_Version;
	}
	/* DBG_871X("pHalData->eeprom_version is 0x%x\n", pHalData->eeprom_version); */
}

void Hal_ReadTxPowerInfo8812A(struct rtl_priv *rtlpriv, u8 *hwinfo,
	bool autoload_fail)
{
	struct rtl_efuse *efuse = rtl_efuse(rtlpriv);
	 struct _rtw_hal	*pHalData = GET_HAL_DATA(rtlpriv);
	struct txpower_info_2g pwrInfo24G;
	struct txpower_info_5g pwrInfo5G;
	uint8_t	rfPath, ch, group, TxCount;
	uint8_t	channel5G[CHANNEL_MAX_NUMBER_5G] = {
		 36,  38,  40,  42,  44,  46,  48,  50,  52,  54,  56,  58,
		 60,  62,  64, 100, 102, 104, 106, 108, 110, 112, 114, 116,
		118, 120, 122, 124, 126, 128, 130, 132, 134, 136, 138, 140,
		142, 144, 149, 151, 153, 155, 157, 159, 161, 163, 165, 167,
		168, 169, 171, 173, 175, 177 };

	uint8_t	channel5G_80M[CHANNEL_MAX_NUMBER_5G_80M] = {
		42, 58, 106, 122, 138, 155, 171};

	_rtl8821au_read_power_value_fromprom(rtlpriv, &pwrInfo24G, &pwrInfo5G, hwinfo, autoload_fail);

	/*
	 * if(!AutoLoadFail)
	 * 	pHalData->bTXPowerDataReadFromEEPORM = _TRUE;
	 */

	for (rfPath = 0; rfPath < MAX_RF_PATH; rfPath++) {
		for (ch = 0 ; ch < CHANNEL_MAX_NUMBER_2G; ch++) {
			Hal_GetChnlGroup8812A(ch+1, &group);

			if (ch == (CHANNEL_MAX_NUMBER_2G-1)) {
				efuse->txpwrlevel_cck[rfPath][ch] =
					pwrInfo24G.index_cck_base[rfPath][5];
				efuse->txpwrlevel_ht40_1s[rfPath][ch] =
					pwrInfo24G.index_bw40_base[rfPath][group];
			} else {
				efuse->txpwrlevel_cck[rfPath][ch] =
					pwrInfo24G.index_cck_base[rfPath][group];
				efuse->txpwrlevel_ht40_1s[rfPath][ch] =
					pwrInfo24G.index_bw40_base[rfPath][group];
			}

			/*
			 * DBG_871X("======= Path %d, ChannelIndex %d, Group %d=======\n",rfPath,ch, group);
			 * DBG_871X("Index24G_CCK_Base[%d][%d] = 0x%x\n",rfPath,ch ,pHalData->Index24G_CCK_Base[rfPath][ch]);
			 * DBG_871X("Index24G_BW40_Base[%d][%d] = 0x%x\n",rfPath,ch,pHalData->Index24G_BW40_Base[rfPath][ch]);
			 */
		}

		for (ch = 0 ; ch < CHANNEL_MAX_NUMBER_5G; ch++) {
			Hal_GetChnlGroup8812A(channel5G[ch], &group);

			efuse->txpwr_5g_bw40base[rfPath][ch] = pwrInfo5G.index_bw40_base[rfPath][group];

			/*
			 * DBG_871X("======= Path %d, ChannelIndex %d, Group %d=======\n",rfPath,ch, group);
			 * DBG_871X("Index5G_BW40_Base[%d][%d] = 0x%x\n",rfPath,ch,pHalData->Index5G_BW40_Base[rfPath][ch]);
			 */
		}
		for (ch = 0 ; ch < CHANNEL_MAX_NUMBER_5G_80M; ch++) {
			uint8_t	upper, lower;

			Hal_GetChnlGroup8812A(channel5G_80M[ch], &group);
			upper = pwrInfo5G.index_bw40_base[rfPath][group];
			lower = pwrInfo5G.index_bw40_base[rfPath][group+1];

			efuse->txpwr_5g_bw80base[rfPath][ch] = (upper + lower) / 2;

			/*
			 * DBG_871X("======= Path %d, ChannelIndex %d, Group %d=======\n",rfPath,ch, group);
			 * DBG_871X("Index5G_BW80_Base[%d][%d] = 0x%x\n",rfPath,ch,pHalData->Index5G_BW80_Base[rfPath][ch]);
			 */
		}

		for (TxCount = 0; TxCount < MAX_TX_COUNT; TxCount++) {
			efuse->txpwr_cckdiff[rfPath][TxCount]  = pwrInfo24G.cck_diff[rfPath][TxCount];
			efuse->txpwr_legacyhtdiff[rfPath][TxCount] = pwrInfo24G.ofdm_diff[rfPath][TxCount];
			efuse->txpwr_ht20diff[rfPath][TxCount] = pwrInfo24G.bw20_diff[rfPath][TxCount];
			efuse->txpwr_ht40diff[rfPath][TxCount] = pwrInfo24G.bw40_diff[rfPath][TxCount];

			efuse->txpwr_5g_ofdmdiff[rfPath][TxCount] = pwrInfo5G.ofdm_diff[rfPath][TxCount];
			efuse->txpwr_5g_bw20diff[rfPath][TxCount] = pwrInfo5G.bw20_diff[rfPath][TxCount];
			efuse->txpwr_5g_bw40diff[rfPath][TxCount] = pwrInfo5G.bw40_diff[rfPath][TxCount];
			efuse->txpwr_5g_bw80diff[rfPath][TxCount] = pwrInfo5G.bw80_diff[rfPath][TxCount];
/* #if DBG */
		}
	}


	/* 2010/10/19 MH Add Regulator recognize for CU. */
	if (!autoload_fail) {
		struct registry_priv  *registry_par = &rtlpriv->registrypriv;
		if (registry_par->regulatory_tid == 0xff) {

			if (hwinfo[EEPROM_RF_BOARD_OPTION_8812] == 0xFF)
				efuse->eeprom_regulatory = (EEPROM_DEFAULT_BOARD_OPTION&0x7);	/* bit0~2 */
			else
				efuse->eeprom_regulatory = (hwinfo[EEPROM_RF_BOARD_OPTION_8812]&0x7);	/* bit0~2 */
		} else{
			efuse->eeprom_regulatory = registry_par->regulatory_tid;
		}

		/* 2012/09/26 MH Add for TX power calibrate rate. */
		pHalData->TxPwrCalibrateRate = hwinfo[EEPROM_TX_PWR_CALIBRATE_RATE_8812];
	} else {
		efuse->eeprom_regulatory = 0;
		/* 2012/09/26 MH Add for TX power calibrate rate. */
		pHalData->TxPwrCalibrateRate = EEPROM_DEFAULT_TX_CALIBRATE_RATE;
	}
	DBG_871X("EEPROMRegulatory = 0x%x TxPwrCalibrateRate=0x%x\n", efuse->eeprom_regulatory, pHalData->TxPwrCalibrateRate);

}

void Hal_ReadBoardType8812A(struct rtl_priv *rtlpriv, u8 *hwinfo,
	bool autoload_fail)
{
	 struct _rtw_hal	*pHalData = GET_HAL_DATA(rtlpriv);

	if (!autoload_fail) {
		pHalData->InterfaceSel = (hwinfo[EEPROM_RF_BOARD_OPTION_8812]&0xE0)>>5;
		if (hwinfo[EEPROM_RF_BOARD_OPTION_8812] == 0xFF)
			pHalData->InterfaceSel = (EEPROM_DEFAULT_BOARD_OPTION&0xE0)>>5;
	} else {
		pHalData->InterfaceSel = 0;
	}
	DBG_871X("Board Type: 0x%2x\n", pHalData->InterfaceSel);

}

void Hal_ReadThermalMeter_8812A(struct rtl_priv *rtlpriv, uint8_t *PROMContent,
	BOOLEAN	AutoloadFail)
{
	struct rtl_efuse *efuse = rtl_efuse(rtlpriv);

	/* uint8_t	tempval; */

	/*
	 * ThermalMeter from EEPROM
	 */
	if (!AutoloadFail)
		efuse->eeprom_thermalmeter = PROMContent[EEPROM_THERMAL_METER_8812];
	else
		efuse->eeprom_thermalmeter = EEPROM_Default_ThermalMeter_8812;
	/* pHalData->EEPROMThermalMeter = (tempval&0x1f);	//[4:0] */

	if (efuse->eeprom_thermalmeter == 0xff || AutoloadFail) {
		efuse->apk_thermalmeterignore = _TRUE;
		efuse->eeprom_thermalmeter = 0xFF;
	}

	/* pHalData->ThermalMeter[0] = pHalData->EEPROMThermalMeter; */
	DBG_871X("ThermalMeter = 0x%x\n", efuse->eeprom_thermalmeter);
}

void Hal_ReadChannelPlan8812A(struct rtl_priv *rtlpriv, uint8_t *hwinfo,
	BOOLEAN	AutoLoadFail)
{
	rtlpriv->mlmepriv.ChannelPlan = hal_com_get_channel_plan(
		rtlpriv
		, hwinfo?hwinfo[EEPROM_ChannelPlan_8812]:0xFF
		, rtlpriv->registrypriv.channel_plan
		, RT_CHANNEL_DOMAIN_REALTEK_DEFINE
		, AutoLoadFail
	);

	DBG_871X("mlmepriv.ChannelPlan = 0x%02x\n", rtlpriv->mlmepriv.ChannelPlan);
}

void Hal_EfuseParseXtal_8812A(struct rtl_priv *rtlpriv, uint8_t *hwinfo,
	BOOLEAN	AutoLoadFail)
{
	struct rtl_efuse *rtlefuse = rtl_efuse(rtlpriv);

	if (!AutoLoadFail) {
		rtlefuse->crystalcap = hwinfo[EEPROM_XTAL_8812];
		if (rtlefuse->crystalcap == 0xFF)
			rtlefuse->crystalcap = EEPROM_Default_CrystalCap_8812;	 /* what value should 8812 set? */
	} else {
		rtlefuse->crystalcap = EEPROM_Default_CrystalCap_8812;
	}
	DBG_871X("CrystalCap: 0x%2x\n", rtlefuse->crystalcap);
}

void Hal_ReadAntennaDiversity8812A(IN struct rtl_priv *rtlpriv,
	uint8_t *PROMContent, BOOLEAN AutoLoadFail)
{
	 struct _rtw_hal	*pHalData = GET_HAL_DATA(rtlpriv);
	struct registry_priv	*registry_par = &rtlpriv->registrypriv;

	if (!AutoLoadFail) {
		/*  Antenna Diversity setting. */
		if (registry_par->antdiv_cfg == 2) {
			pHalData->AntDivCfg = (PROMContent[EEPROM_RF_BOARD_OPTION_8812]&0x18)>>3;
			if (PROMContent[EEPROM_RF_BOARD_OPTION_8812] == 0xFF)
				pHalData->AntDivCfg = (EEPROM_DEFAULT_BOARD_OPTION&0x18)>>3;;
		} else {
			pHalData->AntDivCfg = registry_par->antdiv_cfg;
		}
	} else {
		pHalData->AntDivCfg = 0;
	}

	DBG_871X("SWAS: bHwAntDiv = %x\n", pHalData->AntDivCfg);
}

void _rtl8812au_read_pa_type(struct rtl_priv *rtlpriv, uint8_t *PROMContent,
			BOOLEAN	AutoloadFail)
{
	struct rtl_hal *rtlhal = rtl_hal(rtlpriv);

	if (!AutoloadFail) {
		if (GetRegAmplifierType2G(rtlpriv) == 0) {
			/* AUTO */
			rtlhal->pa_type_2g = EF1Byte(*(uint8_t *)&PROMContent[EEPROM_PA_TYPE_8812AU]);
			rtlhal->lna_type_2g = EF1Byte(*(uint8_t *)&PROMContent[EEPROM_LNA_TYPE_2G_8812AU]);
			if (rtlhal->pa_type_2g == 0xFF && rtlhal->lna_type_2g == 0xFF) {
				rtlhal->pa_type_2g = 0;
				rtlhal->lna_type_2g = 0;
			}
			rtlhal->external_pa_2g = ((rtlhal->pa_type_2g & BIT5) && (rtlhal->pa_type_2g & BIT4)) ? 1 : 0;
			rtlhal->external_lna_2g = ((rtlhal->lna_type_2g & BIT7) && (rtlhal->lna_type_2g & BIT3)) ? 1 : 0;
		} else 	{
			rtlhal->external_pa_2g  = (GetRegAmplifierType2G(rtlpriv)&ODM_BOARD_EXT_PA)  ? 1 : 0;
			rtlhal->external_lna_2g = (GetRegAmplifierType2G(rtlpriv)&ODM_BOARD_EXT_LNA) ? 1 : 0;
		}

		if (GetRegAmplifierType5G(rtlpriv) == 0) {
			/* AUTO */
			rtlhal->pa_type_5g = EF1Byte(*(uint8_t *)&PROMContent[EEPROM_PA_TYPE_8812AU]);
			rtlhal->lna_type_5g = EF1Byte(*(uint8_t *)&PROMContent[EEPROM_LNA_TYPE_5G_8812AU]);
			if (rtlhal->pa_type_5g == 0xFF && rtlhal->lna_type_5g == 0xFF) {
				rtlhal->pa_type_5g = 0;
				rtlhal->lna_type_5g = 0;
			}
			rtlhal->external_pa_5g = ((rtlhal->pa_type_5g & BIT1) && (rtlhal->pa_type_5g & BIT0)) ? 1 : 0;
			rtlhal->external_lna_5g = ((rtlhal->lna_type_5g & BIT7) && (rtlhal->lna_type_5g & BIT3)) ? 1 : 0;
		} else {
			rtlhal->external_pa_5g  = (GetRegAmplifierType5G(rtlpriv)&ODM_BOARD_EXT_PA_5G)  ? 1 : 0;
			rtlhal->external_lna_5g = (GetRegAmplifierType5G(rtlpriv)&ODM_BOARD_EXT_LNA_5G) ? 1 : 0;
		}
	} else {
		rtlhal->external_pa_2g  = EEPROM_Default_PAType;
		rtlhal->external_pa_5g  = 0xFF;
		rtlhal->external_lna_2g = EEPROM_Default_LNAType;
		rtlhal->external_lna_5g = 0xFF;

		if (GetRegAmplifierType2G(rtlpriv) == 0) {
			/* AUTO */
			rtlhal->external_pa_2g  = 0;
			rtlhal->external_lna_2g = 0;
		} else {
			rtlhal->external_pa_2g  = (GetRegAmplifierType2G(rtlpriv)&ODM_BOARD_EXT_PA)  ? 1 : 0;
			rtlhal->external_lna_2g = (GetRegAmplifierType2G(rtlpriv)&ODM_BOARD_EXT_LNA) ? 1 : 0;
		}
		if (GetRegAmplifierType5G(rtlpriv) == 0) {
			/* AUTO */
			rtlhal->external_pa_5g  = 0;
			rtlhal->external_lna_5g = 0;
		} else 	{
			rtlhal->external_pa_5g  = (GetRegAmplifierType5G(rtlpriv)&ODM_BOARD_EXT_PA_5G)  ? 1 : 0;
			rtlhal->external_lna_5g = (GetRegAmplifierType5G(rtlpriv)&ODM_BOARD_EXT_LNA_5G) ? 1 : 0;
		}
	}
	DBG_871X("pHalData->PAType_2G is 0x%x, pHalData->ExternalPA_2G = %d\n", rtlhal->pa_type_2g, rtlhal->external_pa_2g);
	DBG_871X("pHalData->PAType_5G is 0x%x, pHalData->ExternalPA_5G = %d\n", rtlhal->pa_type_5g, rtlhal->external_pa_5g);
	DBG_871X("pHalData->LNAType_2G is 0x%x, pHalData->ExternalLNA_2G = %d\n", rtlhal->lna_type_2g, rtlhal->external_lna_2g);
	DBG_871X("pHalData->LNAType_5G is 0x%x, pHalData->ExternalLNA_5G = %d\n", rtlhal->lna_type_5g, rtlhal->external_lna_5g);
}

void _rtl8821au_read_pa_type(struct rtl_priv *rtlpriv, uint8_t *PROMContent,
	BOOLEAN	 AutoloadFail)
{
	struct rtl_hal *rtlhal = rtl_hal(rtlpriv);

	if (!AutoloadFail) {
		if (GetRegAmplifierType2G(rtlpriv) == 0) {
			/* AUTO */

			rtlhal->pa_type_2g = EF1Byte(*(uint8_t *) &PROMContent[EEPROM_PA_TYPE_8812AU]);
			rtlhal->lna_type_2g = EF1Byte(*(uint8_t *) &PROMContent[EEPROM_LNA_TYPE_2G_8812AU]);
			if (rtlhal->pa_type_2g == 0xFF && rtlhal->lna_type_2g == 0xFF) {
				rtlhal->pa_type_2g = 0;
				rtlhal->lna_type_2g = 0;
			}
			rtlhal->external_pa_2g = (rtlhal->pa_type_2g & BIT4) ? 1 : 0;
			rtlhal->external_lna_2g = (rtlhal->lna_type_2g & BIT3) ? 1 : 0;
		} else {
			rtlhal->external_pa_2g  = (GetRegAmplifierType2G(rtlpriv)&ODM_BOARD_EXT_PA)  ? 1 : 0;
			rtlhal->external_lna_2g = (GetRegAmplifierType2G(rtlpriv)&ODM_BOARD_EXT_LNA) ? 1 : 0;
		}

		if (GetRegAmplifierType5G(rtlpriv) == 0) {
			/* AUTO */
			rtlhal->pa_type_5g = EF1Byte(*(uint8_t *) &PROMContent[EEPROM_PA_TYPE_8812AU]);
			rtlhal->lna_type_5g = EF1Byte(*(uint8_t *) &PROMContent[EEPROM_LNA_TYPE_5G_8812AU]);
			if (rtlhal->pa_type_5g == 0xFF && rtlhal->lna_type_5g == 0xFF) {
				rtlhal->pa_type_5g = 0;
				rtlhal->lna_type_5g = 0;
			}
			rtlhal->external_pa_5g = (rtlhal->pa_type_5g & BIT0) ? 1 : 0;
			rtlhal->external_lna_5g = (rtlhal->lna_type_5g & BIT3) ? 1 : 0;
		} else {
			rtlhal->external_pa_5g  = (GetRegAmplifierType5G(rtlpriv)&ODM_BOARD_EXT_PA_5G)  ? 1 : 0;
			rtlhal->external_lna_5g = (GetRegAmplifierType5G(rtlpriv)&ODM_BOARD_EXT_LNA_5G) ? 1 : 0;
		}
	} else {
		rtlhal->external_pa_2g  = EEPROM_Default_PAType;
		rtlhal->external_pa_5g  = 0xFF;
		rtlhal->external_lna_2g = EEPROM_Default_LNAType;
		rtlhal->external_lna_5g = 0xFF;

		if (GetRegAmplifierType2G(rtlpriv) == 0) {
			/* AUTO */
			rtlhal->external_pa_2g  = 0;
			rtlhal->external_lna_2g = 0;
		} else {
			rtlhal->external_pa_2g  = (GetRegAmplifierType2G(rtlpriv)&ODM_BOARD_EXT_PA)  ? 1 : 0;
			rtlhal->external_lna_2g = (GetRegAmplifierType2G(rtlpriv)&ODM_BOARD_EXT_LNA) ? 1 : 0;
		}
		if (GetRegAmplifierType5G(rtlpriv) == 0) {
			/* AUTO */
			rtlhal->external_pa_5g  = 0;
			rtlhal->external_lna_5g = 0;
		} else {
			rtlhal->external_pa_5g  = (GetRegAmplifierType5G(rtlpriv)&ODM_BOARD_EXT_PA_5G)  ? 1 : 0;
			rtlhal->external_lna_5g = (GetRegAmplifierType5G(rtlpriv)&ODM_BOARD_EXT_LNA_5G) ? 1 : 0;
		}
	}
	DBG_871X("pHalData->PAType_2G is 0x%x, pHalData->ExternalPA_2G = %d\n", rtlhal->pa_type_2g, rtlhal->external_pa_2g);
	DBG_871X("pHalData->PAType_5G is 0x%x, pHalData->ExternalPA_5G = %d\n", rtlhal->pa_type_5g, rtlhal->external_pa_5g);
	DBG_871X("pHalData->LNAType_2G is 0x%x, pHalData->ExternalLNA_2G = %d\n", rtlhal->lna_type_2g, rtlhal->external_lna_2g);
	DBG_871X("pHalData->LNAType_5G is 0x%x, pHalData->ExternalLNA_5G = %d\n", rtlhal->lna_type_5g, rtlhal->external_lna_5g);
}


/*
 * 2013/04/15 MH Add 8812AU- VL/VS/VN for different board type.
 */
void
hal_ReadUsbType_8812AU(struct rtl_priv *rtlpriv, uint8_t *PROMContent,
	BOOLEAN AutoloadFail)
{
	/* if (IS_HARDWARE_TYPE_8812AU(rtlpriv) && rtlpriv->UsbModeMechanism.RegForcedUsbMode == 5) */
	{
		uint8_t	reg_tmp, i, j, antenna = 0, wmode = 0;
		/* Read anenna type from EFUSE 1019/1018 */
		for (i = 0; i < 2; i++) {
			/* Check efuse address 1019 */
			/* Check efuse address 1018 */
			efuse_OneByteRead(rtlpriv, 1019-i, &reg_tmp);

			for (j = 0; j < 2; j++) {
				/* CHeck bit 7-5 */
				/* Check bit 3-1 */
				antenna = ((reg_tmp&0xee) >> (5-(j*4)));
				if (antenna == 0)
					continue;
				else {
					break;
				}
			}
		}

		/* Read anenna type from EFUSE 1021/1020 */
		for (i = 0; i < 2; i++) {
			/* Check efuse address 1019 */
			/* Check efuse address 1018 */
			efuse_OneByteRead(rtlpriv, 1021-i, &reg_tmp);

			for (j = 0; j < 2; j++) {
				/* CHeck bit 3-2 */
				/* Check bit 1-0 */
				wmode = ((reg_tmp&0x0f) >> (2-(j*2)));
				if (wmode)
					continue;
				else {
					break;
				}
			}
		}

		/* Ulli Antenna Mode */
		/* Antenna == 1 WMODE = 3 RTL8812AU-VL 11AC + USB2.0 Mode */
		if (antenna == 1) {
			/* Config 8812AU as 1*1 mode AC mode. */
			rtlpriv->phy.rf_type = RF_1T1R;
			/* UsbModeSwitch_SetUsbModeMechOn(rtlpriv, FALSE); */
			/* pHalData->EFUSEHidden = EFUSE_HIDDEN_812AU_VL; */
			DBG_871X("%s(): EFUSE_HIDDEN_812AU_VL\n", __FUNCTION__);
		} else if (antenna == 2) {
			if (wmode == 3) {
				if (PROMContent[EEPROM_USB_MODE_8812] == 0x2) {
					/*
					 * RTL8812AU Normal Mode. No further action.
					 * pHalData->EFUSEHidden = EFUSE_HIDDEN_812AU;
					 */
					DBG_871X("%s(): EFUSE_HIDDEN_812AU\n", __FUNCTION__);
				} else {
					/*
					 * Antenna == 2 WMODE = 3 RTL8812AU-VS 11AC + USB2.0 Mode
					 * Driver will not support USB automatic switch
					 * UsbModeSwitch_SetUsbModeMechOn(rtlpriv, FALSE);
					 * pHalData->EFUSEHidden = EFUSE_HIDDEN_812AU_VS;
					 */
					DBG_871X("%s(): EFUSE_HIDDEN_812AU_VS\n", __FUNCTION__);
				}
			} else
				if (wmode == 2) {
				/*
				 * Antenna == 2 WMODE = 2 RTL8812AU-VN 11N only + USB2.0 Mode
				 * UsbModeSwitch_SetUsbModeMechOn(rtlpriv, FALSE);
				 * pHalData->EFUSEHidden = EFUSE_HIDDEN_812AU_VN;
				 */
				DBG_871X("%s(): EFUSE_HIDDEN_812AU_VN\n", __FUNCTION__);
			}
		}
	}
}

/*
 * ULLI need to refactoring for rtlwifi-lib
 * static bool _rt8812au_phy_set_rf_power_state(struct ieee80211_hw *hw,
 * 					    enum rf_pwrstate rfpwr_state)
 *
 */

void ReadRFType8812A(struct rtl_priv *rtlpriv)
{
	/*
	 * if (pHalData->rf_type == RF_1T1R){
	 *	pHalData->bRFPathRxEnable[0] = _TRUE;
	 * }
	 * else {	// Default unknow type is 2T2r
	 *	pHalData->bRFPathRxEnable[0] = pHalData->bRFPathRxEnable[1] = _TRUE;
	 *}
	 */

	if (IsSupported24G(rtlpriv->registrypriv.wireless_mode) &&
		IsSupported5G(rtlpriv->registrypriv.wireless_mode))
		rtlpriv->rtlhal.bandset = BAND_ON_BOTH;
	else if (IsSupported5G(rtlpriv->registrypriv.wireless_mode))
		rtlpriv->rtlhal.bandset = BAND_ON_5G;
	else
		rtlpriv->rtlhal.bandset = BAND_ON_2_4G;

	/*
	 * if(rtlpriv->bInHctTest)
	 * 	pHalData->BandSet = BAND_ON_2_4G;
	 */
}

void rtl8812_SetHalODMVar(struct rtl_priv *rtlpriv, HAL_ODM_VARIABLE eVariable,
	void *pValue1, BOOLEAN bSet)
{
	 struct _rtw_hal	*pHalData = GET_HAL_DATA(rtlpriv);
	struct _rtw_dm *podmpriv = &pHalData->odmpriv;
	/* _irqL irqL; */
	switch (eVariable) {
	case HAL_ODM_STA_INFO:
		{
			struct sta_info *psta = (struct sta_info *)pValue1;
			if (bSet) {
				DBG_8192C("### Set STA_(%d) info\n", psta->mac_id);
				podmpriv->pODM_StaInfo[psta->mac_id] = psta;
#if (RATE_ADAPTIVE_SUPPORT == 1)
				ODM_RAInfo_Init(podmpriv, psta->mac_id);
#endif
			} else {
				DBG_8192C("### Clean STA_(%d) info\n", psta->mac_id);
				/* spin_lock_bh(&pHalData->odm_stainfo_lock, &irqL); */
				podmpriv->pODM_StaInfo[psta->mac_id] = NULL;

				/* spin_unlock_bh(&pHalData->odm_stainfo_lock, &irqL); */
			}
		}
		break;
	default:
		break;
	}
}

void hal_notch_filter_8812(struct rtl_priv *rtlpriv, bool enable)
{
	if (enable) {
		DBG_871X("Enable notch filter\n");
		/* rtl_write_byte(rtlpriv, rOFDM0_RxDSP+1, rtl_read_byte(rtlpriv, rOFDM0_RxDSP+1) | BIT1); */
	} else {
		DBG_871X("Disable notch filter\n");
		/* rtl_write_byte(rtlpriv, rOFDM0_RxDSP+1, rtl_read_byte(rtlpriv, rOFDM0_RxDSP+1) & ~BIT1); */
	}
}

void UpdateHalRAMask8812A(struct rtl_priv *rtlpriv, uint32_t mac_id, uint8_t rssi_level)
{
	/* volatile unsigned int result; */
	uint8_t	init_rate = 0;
	uint8_t	networkType, raid;
	uint32_t	mask, rate_bitmap;
	uint8_t	shortGIrate = _FALSE;
	int	supportRateNum = 0;
	uint8_t	arg[4] = {0};
	struct sta_info	*psta;
	 struct _rtw_hal	*pHalData = GET_HAL_DATA(rtlpriv);
	/* struct dm_priv	*pdmpriv = &pHalData->dmpriv; */
	struct mlme_ext_priv	*pmlmeext = &rtlpriv->mlmeextpriv;
	struct mlme_ext_info	*pmlmeinfo = &(pmlmeext->mlmext_info);
	WLAN_BSSID_EX 		*cur_network = &(pmlmeinfo->network);

	if (mac_id >= NUM_STA) {	/* CAM_SIZE */
		return;
	}

	psta = pmlmeinfo->FW_sta_info[mac_id].psta;
	if (psta == NULL) {
		return;
	}

	switch (mac_id) {
	case 0:
		/* for infra mode */
		supportRateNum = rtw_get_rateset_len(cur_network->SupportedRates);
		networkType = judge_network_type(rtlpriv, cur_network->SupportedRates, supportRateNum);
		/* pmlmeext->cur_wireless_mode = networkType; */
		/* raid = networktype_to_raid(networkType); */
		raid = rtw_hal_networktype_to_raid(rtlpriv, networkType);

		mask = update_supported_rate(cur_network->SupportedRates, supportRateNum);
#ifdef CONFIG_80211AC_VHT
		if (pmlmeinfo->VHT_enable) {
			mask |= (rtw_vht_rate_to_bitmap(psta->vhtpriv.vht_mcs_map) << 12);
			shortGIrate = psta->vhtpriv.sgi;
		} else
#endif
			{
				mask |= (pmlmeinfo->HT_enable) ? update_MCS_rate(&(pmlmeinfo->HT_caps)) : 0;
				if (support_short_GI(rtlpriv, &(pmlmeinfo->HT_caps)))
					shortGIrate = _TRUE;
			}

		break;

	case 1:
		/* for broadcast/multicast */
		supportRateNum = rtw_get_rateset_len(pmlmeinfo->FW_sta_info[mac_id].SupportedRates);
		if (pmlmeext->cur_wireless_mode & WIRELESS_11B)
			networkType = WIRELESS_11B;
		else
			networkType = WIRELESS_11G;

		/* raid = networktype_to_raid(networkType); */
		raid = rtw_hal_networktype_to_raid(rtlpriv, networkType);
		mask = update_basic_rate(cur_network->SupportedRates, supportRateNum);

		break;

	default:
		/*for each sta in IBSS */
		supportRateNum = rtw_get_rateset_len(pmlmeinfo->FW_sta_info[mac_id].SupportedRates);
		networkType = judge_network_type(rtlpriv, pmlmeinfo->FW_sta_info[mac_id].SupportedRates, supportRateNum) & 0xf;
		/*
		 * pmlmeext->cur_wireless_mode = networkType;
		 * raid = networktype_to_raid(networkType);
		 */
		raid = rtw_hal_networktype_to_raid(rtlpriv, networkType);
		mask = update_supported_rate(cur_network->SupportedRates, supportRateNum);

		/* todo: support HT in IBSS */

		break;
	}

	/* mask &=0x0fffffff; */
	rate_bitmap = 0xffffffff;
#ifdef	CONFIG_ODM_REFRESH_RAMASK
	{
		rate_bitmap = ODM_Get_Rate_Bitmap(&pHalData->odmpriv, mac_id, mask, rssi_level);
		printk("%s => mac_id:%d, networkType:0x%02x, mask:0x%08x\n\t ==> rssi_level:%d, rate_bitmap:0x%08x\n",
			__FUNCTION__, mac_id, networkType, mask, rssi_level, rate_bitmap);
	}
#endif

	mask &= rate_bitmap;
	init_rate = get_highest_rate_idx(mask)&0x3f;

	/*
	 * arg[0] = macid
	 * arg[1] = raid
	 * arg[2] = shortGIrate
	 * arg[3] = init_rate
	 */

	arg[0] = mac_id;
	arg[1] = raid;
	arg[2] = shortGIrate;
	arg[3] = init_rate;

	rtl8812_set_raid_cmd(rtlpriv, mask, arg);

	/* set ra_id */
	psta->raid = raid;
	psta->init_rate = init_rate;
}

/*
 *	Description:
 *		Query setting of specified variable.
 */
uint8_t rtl8821au_get_hal_def_var(struct rtl_priv *rtlpriv, HAL_DEF_VARIABLE variable, void *pval)
{
	struct _rtw_hal *pHalData;
	uint8_t bResult;
	struct rtl_hal *rtlhal = rtl_hal(rtlpriv);

	pHalData = GET_HAL_DATA(rtlpriv);

	bResult = _SUCCESS;

	switch (variable) {
	case HAL_DEF_UNDERCORATEDSMOOTHEDPWDB:
		{
			struct mlme_priv *pmlmepriv;
			struct sta_priv *pstapriv;
			struct sta_info *psta;

			pmlmepriv = &rtlpriv->mlmepriv;
			pstapriv = &rtlpriv->stapriv;
			psta = rtw_get_stainfo(pstapriv, pmlmepriv->cur_network.network.MacAddress);
			if (psta) {
				*((int *) pval) = psta->rssi_stat.UndecoratedSmoothedPWDB;
			}
		}
		break;

#ifdef CONFIG_ANTENNA_DIVERSITY
	case HAL_DEF_IS_SUPPORT_ANT_DIV:
		*((uint8_t *)pval) = (pHalData->AntDivCfg == 0) ? _FALSE : _TRUE;
		break;
#endif

#ifdef CONFIG_ANTENNA_DIVERSITY
	case HAL_DEF_CURRENT_ANTENNA:
		*((uint8_t *)pval) = pHalData->CurAntenna;
		break;
#endif

	case HAL_DEF_DRVINFO_SZ:
		*((u32 *)pval) = DRVINFO_SZ;
		break;

	case HAL_DEF_MAX_RECVBUF_SZ:
		*((u32 *)pval) = MAX_RECVBUF_SZ;
		break;

	case HAL_DEF_RX_PACKET_OFFSET:
		*((u32 *)pval) = RXDESC_SIZE + DRVINFO_SZ;
		break;

#if (RATE_ADAPTIVE_SUPPORT == 1)
	case HAL_DEF_RA_DECISION_RATE:
		{
			uint8_t MacID = *(uint8_t *)pval;
			*((uint8_t *)pval) = ODM_RA_GetDecisionRate_8812A(&pHalData->odmpriv, MacID);
		}
		break;

	case HAL_DEF_RA_SGI:
		{
			uint8_t MacID = *(uint8_t *)pval;
			*((uint8_t *)pval) = ODM_RA_GetShortGI_8812A(&pHalData->odmpriv, MacID);
		}
		break;
#endif

	case HW_VAR_MAX_RX_AMPDU_FACTOR:
		*((u32 *)pval) = MAX_AMPDU_FACTOR_64K;
		break;

	case HAL_DEF_LDPC:
		if (IS_VENDOR_8812A_C_CUT(rtlhal->version))
			*(uint8_t *)pval = _TRUE;
		else if (IS_HARDWARE_TYPE_8821(rtlhal))
			*(uint8_t *)pval = _FALSE;
		else
			*(uint8_t *)pval = _FALSE;
		break;

	case HAL_DEF_TX_STBC:
		if (rtlpriv->phy.rf_type == RF_2T2R)
			*(uint8_t *)pval = 1;
		else
			*(uint8_t *)pval = 0;
		break;

	case HAL_DEF_RX_STBC:
		*(uint8_t *)pval = 1;
		break;

	case HAL_DEF_TX_PAGE_BOUNDARY:
		if (IS_HARDWARE_TYPE_8812(rtlhal))
			*(uint8_t *)pval = TX_PAGE_BOUNDARY_8812;
		else
			*(uint8_t *)pval = TX_PAGE_BOUNDARY_8821;
		break;

	case HAL_DEF_TX_PAGE_BOUNDARY_WOWLAN:
		*(uint8_t *)pval = TX_PAGE_BOUNDARY_WOWLAN_8812;
		break;

	default:
		DBG_8192C("%s: [ERROR] HAL_DEF_VARIABLE(%d) not defined!\n", __FUNCTION__, variable);
		bResult = _FAIL;
		break;
	}

	return bResult;
}

