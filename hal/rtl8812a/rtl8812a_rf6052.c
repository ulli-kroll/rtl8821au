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
#define _RTL8812A_RF6052_C_

//#include <drv_types.h>
#include <rtl8812a_hal.h>


/*-----------------------------------------------------------------------------
 * Function:    PHY_RF6052SetBandwidth()
 *
 * Overview:    This function is called by SetBWModeCallback8190Pci() only
 *
 * Input:       struct rtl_priv *			Adapter
 *			WIRELESS_BANDWIDTH_E	Bandwidth	//20M or 40M
 *
 * Output:      NONE
 *
 * Return:      NONE
 *
 * Note:		For RF type 0222D
 *---------------------------------------------------------------------------*/
VOID
PHY_RF6052SetBandwidth8812(
	IN	struct rtl_priv *			Adapter,
	IN	CHANNEL_WIDTH		Bandwidth)	//20M or 40M
{
	 struct rtw_hal	*pHalData = GET_HAL_DATA(Adapter);

	switch(Bandwidth)
	{
		case CHANNEL_WIDTH_20:
			//DBG_871X("PHY_RF6052SetBandwidth8812(), set 20MHz, pHalData->RfRegChnlVal[0] = 0x%x \n", pHalData->RfRegChnlVal[0]);
			rtw_hal_write_rfreg(Adapter, RF_PATH_A, RF_CHNLBW_Jaguar, BIT11|BIT10, 3);
			rtw_hal_write_rfreg(Adapter, RF_PATH_B, RF_CHNLBW_Jaguar, BIT11|BIT10, 3);
		break;

		case CHANNEL_WIDTH_40:
			//DBG_871X("PHY_RF6052SetBandwidth8812(), set 40MHz, pHalData->RfRegChnlVal[0] = 0x%x \n", pHalData->RfRegChnlVal[0]);
			rtw_hal_write_rfreg(Adapter, RF_PATH_A, RF_CHNLBW_Jaguar, BIT11|BIT10, 1);
			rtw_hal_write_rfreg(Adapter, RF_PATH_B, RF_CHNLBW_Jaguar, BIT11|BIT10, 1);
		break;

		case CHANNEL_WIDTH_80:
			//DBG_871X("PHY_RF6052SetBandwidth8812(), set 80MHz, pHalData->RfRegChnlVal[0] = 0x%x \n", pHalData->RfRegChnlVal[0]);
			rtw_hal_write_rfreg(Adapter, RF_PATH_A, RF_CHNLBW_Jaguar, BIT11|BIT10, 0);
			rtw_hal_write_rfreg(Adapter, RF_PATH_B, RF_CHNLBW_Jaguar, BIT11|BIT10, 0);
		break;

		default:
			DBG_871X("PHY_RF6052SetBandwidth8812(): unknown Bandwidth: %#X\n",Bandwidth );
			break;
	}
}


VOID
PHY_RF6052SetCckTxPower8812(
	IN	struct rtl_priv *Adapter,
	IN	uint8_t *			pPowerlevel)
{
	 struct rtw_hal		*pHalData = GET_HAL_DATA(Adapter);
	struct dm_priv		*pdmpriv = &pHalData->dmpriv;
	struct mlme_ext_priv 	*pmlmeext = &Adapter->mlmeextpriv;
	uint32_t			TxAGC[2]={0, 0}, tmpval=0;
	BOOLEAN		TurboScanOff = _FALSE;
	uint8_t			idx1, idx2;
	uint8_t *			ptr;

	//FOR CE ,must disable turbo scan
	TurboScanOff = _TRUE;

	if(pmlmeext->sitesurvey_res.state == SCAN_PROCESS)
	{
		TxAGC[RF_PATH_A] = 0x3f3f3f3f;
		TxAGC[RF_PATH_B] = 0x3f3f3f3f;

		TurboScanOff = _TRUE;//disable turbo scan

		if(TurboScanOff)
		{
			for(idx1=RF_PATH_A; idx1<=RF_PATH_B; idx1++)
			{
				TxAGC[idx1] =
					pPowerlevel[idx1] | (pPowerlevel[idx1]<<8) |
					(pPowerlevel[idx1]<<16) | (pPowerlevel[idx1]<<24);
				// 2010/10/18 MH For external PA module. We need to limit power index to be less than 0x20.
				if (TxAGC[idx1] > 0x20 && pHalData->ExternalPA_5G)
					TxAGC[idx1] = 0x20;
			}
		}
	}
	else
	{
// 20100427 Joseph: Driver dynamic Tx power shall not affect Tx power. It shall be determined by power training mechanism.
// Currently, we cannot fully disable driver dynamic tx power mechanism because it is referenced by BT coexist mechanism.
// In the future, two mechanism shall be separated from each other and maintained independantly. Thanks for Lanhsin's reminder.
		if(pdmpriv->DynamicTxHighPowerLvl == TxHighPwrLevel_Level1)
		{
			TxAGC[RF_PATH_A] = 0x10101010;
			TxAGC[RF_PATH_B] = 0x10101010;
		}
		else if(pdmpriv->DynamicTxHighPowerLvl == TxHighPwrLevel_Level2)
		{
			TxAGC[RF_PATH_A] = 0x00000000;
			TxAGC[RF_PATH_B] = 0x00000000;
		}
		else
		{
			for(idx1=RF_PATH_A; idx1<=RF_PATH_B; idx1++)
			{
				TxAGC[idx1] =
					pPowerlevel[idx1] | (pPowerlevel[idx1]<<8) |
					(pPowerlevel[idx1]<<16) | (pPowerlevel[idx1]<<24);
			}

			if(pHalData->EEPROMRegulatory==0)
			{
				tmpval = (pHalData->MCSTxPowerLevelOriginalOffset[0][6]) +
						(pHalData->MCSTxPowerLevelOriginalOffset[0][7]<<8);
				TxAGC[RF_PATH_A] += tmpval;

				tmpval = (pHalData->MCSTxPowerLevelOriginalOffset[0][14]) +
						(pHalData->MCSTxPowerLevelOriginalOffset[0][15]<<24);
				TxAGC[RF_PATH_B] += tmpval;
			}
		}
	}

	for(idx1=RF_PATH_A; idx1<=RF_PATH_B; idx1++)
	{
		ptr = (uint8_t *)(&(TxAGC[idx1]));
		for(idx2=0; idx2<4; idx2++)
		{
			if(*ptr > RF6052_MAX_TX_PWR)
				*ptr = RF6052_MAX_TX_PWR;
			ptr++;
		}
	}

	// rf-A cck tx power
	tmpval = TxAGC[RF_PATH_A]&0xff;
	rtl_set_bbreg(Adapter, rTxAGC_A_CCK11_CCK1_JAguar, bMaskByte1, tmpval);
	//RT_DISP(FPHY, PHY_TXPWR, ("CCK PWR 1M (rf-A) = 0x%x (reg 0x%x)\n", tmpval, rTxAGC_A_CCK1_Mcs32));
	tmpval = TxAGC[RF_PATH_A]>>8;
	rtl_set_bbreg(Adapter, rTxAGC_A_CCK11_CCK1_JAguar, 0xffffff00, tmpval);
	//RT_DISP(FPHY, PHY_TXPWR, ("CCK PWR 2~11M (rf-A) = 0x%x (reg 0x%x)\n", tmpval, rTxAGC_B_CCK11_A_CCK2_11));

	// rf-B cck tx power
	tmpval = TxAGC[RF_PATH_B]>>24;
	rtl_set_bbreg(Adapter, rTxAGC_B_CCK11_CCK1_JAguar, bMaskByte0, tmpval);
	//RT_DISP(FPHY, PHY_TXPWR, ("CCK PWR 11M (rf-B) = 0x%x (reg 0x%x)\n", tmpval, rTxAGC_B_CCK11_A_CCK2_11));
	tmpval = TxAGC[RF_PATH_B]&0x00ffffff;
	rtl_set_bbreg(Adapter, rTxAGC_B_CCK11_CCK1_JAguar, 0xffffff00, tmpval);
	//RT_DISP(FPHY, PHY_TXPWR, ("CCK PWR 1~5.5M (rf-B) = 0x%x (reg 0x%x)\n", tmpval, rTxAGC_B_CCK1_55_Mcs32));

}	/* PHY_RF6052SetCckTxPower */


static int
phy_RF6052_Config_ParaFile_8812(
	IN	struct rtl_priv *	Adapter
	)
{
	uint8_t					eRFPath;
	int					rtStatus = _SUCCESS;
	 struct rtw_hal		*pHalData = GET_HAL_DATA(Adapter);
	static char			sz8812RadioAFile[] = RTL8812_PHY_RADIO_A;
	static char			sz8812RadioBFile[] = RTL8812_PHY_RADIO_B;
	static char 			sz8812TxPwrTrack[] = RTL8812_TXPWR_TRACK;
	static char			sz8821RadioAFile[] = RTL8821_PHY_RADIO_A;
	static char			sz8821RadioBFile[] = RTL8821_PHY_RADIO_B;
	static char 			sz8821TxPwrTrack[] = RTL8821_TXPWR_TRACK;
	char					*pszRadioAFile = NULL, *pszRadioBFile = NULL, *pszTxPwrTrack = NULL;


	if(IS_HARDWARE_TYPE_8812(Adapter))
	{
		pszRadioAFile = sz8812RadioAFile;
		pszRadioBFile = sz8812RadioBFile;
		pszTxPwrTrack = sz8812TxPwrTrack;
	}
	else
	{
		pszRadioAFile = sz8821RadioAFile;
		pszRadioBFile = sz8821RadioBFile;
		pszTxPwrTrack = sz8821TxPwrTrack;
	}


	//3//-----------------------------------------------------------------
	//3// <2> Initialize RF
	//3//-----------------------------------------------------------------
	//for(eRFPath = RF_PATH_A; eRFPath <pHalData->NumTotalRFPath; eRFPath++)
	for(eRFPath = 0; eRFPath <pHalData->NumTotalRFPath; eRFPath++)
	{
		/*----Initialize RF fom connfiguration file----*/
		switch(eRFPath)
		{
		case RF_PATH_A:
#ifdef CONFIG_EMBEDDED_FWIMG
			if(HAL_STATUS_FAILURE ==ODM_ConfigRFWithHeaderFile(&pHalData->odmpriv,CONFIG_RF_RADIO, (ODM_RF_RADIO_PATH_E)eRFPath))
				rtStatus= _FAIL;
#else
			rtStatus = PHY_ConfigRFWithParaFile(Adapter, pszRadioAFile, eRFPath);
#endif//#ifdef CONFIG_EMBEDDED_FWIMG
			break;
		case RF_PATH_B:
#ifdef CONFIG_EMBEDDED_FWIMG
			if(HAL_STATUS_FAILURE ==ODM_ConfigRFWithHeaderFile(&pHalData->odmpriv,CONFIG_RF_RADIO, (ODM_RF_RADIO_PATH_E)eRFPath))
				rtStatus= _FAIL;
#else
			rtStatus =PHY_ConfigRFWithParaFile(Adapter, pszRadioBFile, eRFPath);
#endif
			break;
		default:
			break;
		}

		if(rtStatus != _SUCCESS){
			DBG_871X("%s():Radio[%d] Fail!!", __FUNCTION__, eRFPath);
			goto phy_RF6052_Config_ParaFile_Fail;
		}

	}

	//3 -----------------------------------------------------------------
	//3 Configuration of Tx Power Tracking
	//3 -----------------------------------------------------------------

#ifdef CONFIG_EMBEDDED_FWIMG
	ODM_ConfigRFWithTxPwrTrackHeaderFile(&pHalData->odmpriv);
#else
	PHY_ConfigRFWithTxPwrTrackParaFile(Adapter, pszTxPwrTrack);
#endif

	//RT_TRACE(COMP_INIT, DBG_LOUD, ("<---phy_RF6052_Config_ParaFile_8812()\n"));

phy_RF6052_Config_ParaFile_Fail:
	return rtStatus;
}


int
PHY_RF6052_Config_8812(
	IN	struct rtl_priv *	Adapter)
{
	 struct rtw_hal		*pHalData = GET_HAL_DATA(Adapter);
	int					rtStatus = _SUCCESS;

	// Initialize general global value
	if(pHalData->rf_type == RF_1T1R)
		pHalData->NumTotalRFPath = 1;
	else
		pHalData->NumTotalRFPath = 2;

	//
	// Config BB and RF
	//
	rtStatus = phy_RF6052_Config_ParaFile_8812(Adapter);

	return rtStatus;

}


/* End of HalRf6052.c */

