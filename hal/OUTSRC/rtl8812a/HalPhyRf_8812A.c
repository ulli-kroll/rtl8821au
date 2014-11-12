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

#include "../odm_precomp.h"



/*---------------------------Define Local Constant---------------------------*/
/*  2010/04/25 MH Define the max tx power tracking tx agc power. */
#define		ODM_TXPWRTRACK_MAX_IDX8812A		6

/*---------------------------Define Local Constant---------------------------*/


/*
 * ============================================================
 *  Tx Power Tracking
 * ============================================================
 */


static void DoIQK_8812A(struct rtl_dm *pDM_Odm, u8 DeltaThermalIndex,
	u8 	ThermalValue, u8 Threshold)
{
	struct rtl_priv *Adapter = pDM_Odm->Adapter;
	 struct rtw_hal	*pHalData = GET_HAL_DATA(Adapter);

	ODM_ResetIQKResult(pDM_Odm);

	pDM_Odm->RFCalibrateInfo.ThermalValue_IQK = ThermalValue;
	rtl8812au_phy_iq_calibrate(Adapter, FALSE);
}

/*-----------------------------------------------------------------------------
 * Function:	odm_TxPwrTrackSetPwr88E()
 *
 * Overview:	88E change all channel tx power accordign to flag.
 *				OFDM & CCK are all different.
 *
 * Input:		NONE
 *
 * Output:		NONE
 *
 * Return:		NONE
 *
 * Revised History:
 *	When		Who		Remark
 *	04/23/2012	MHC		Create Version 0.
 *
 *---------------------------------------------------------------------------*/
static void ODM_TxPwrTrackSetPwr8812A(struct rtl_dm *pDM_Odm, PWRTRACK_METHOD Method,
	u8 	RFPath, u8 	ChannelMappedIndex)
{
	uint32_t 	finalBbSwingIdx[2];

	struct rtl_priv *	Adapter = pDM_Odm->Adapter;
	struct rtw_hal *pHalData = GET_HAL_DATA(Adapter);

	u8 PwrTrackingLimit = 26; /* +1.0dB */
	u8 TxRate = 0xFF;
	s8 Final_OFDM_Swing_Index = 0;
	s8 Final_CCK_Swing_Index = 0;
	u8 i = 0;

	if (TxRate != 0xFF) {
		/* Ulli better with switch case, see in rtlwifi-lib */
		/* 2 CCK */
		if ((TxRate >= MGN_1M) && (TxRate <= MGN_11M))
			PwrTrackingLimit = 32;				/* +4dB */
		/* 2 OFDM */
		else if ((TxRate >= MGN_6M) && (TxRate <= MGN_48M))
			PwrTrackingLimit = 32;				/* +4dB */
		else if (TxRate == MGN_54M)
			PwrTrackingLimit = 30;				/* +3dB */

		/* ULLI 80211n */
		/* 2 HT */
		else if ((TxRate >= MGN_MCS0) && (TxRate <= MGN_MCS2))  /* QPSK/BPSK */
			PwrTrackingLimit = 34;				/* +5dB */
		else if ((TxRate >= MGN_MCS3) && (TxRate <= MGN_MCS4))  /* 16QAM */
			PwrTrackingLimit = 32;				/* +4dB */
		else if ((TxRate >= MGN_MCS5) && (TxRate <= MGN_MCS7))  /* 64QAM */
			PwrTrackingLimit = 30;				/* +3dB */

		else if ((TxRate >= MGN_MCS8) && (TxRate <= MGN_MCS10)) 	/* QPSK/BPSK */
			PwrTrackingLimit = 34; 				/* +5dB */
		else if ((TxRate >= MGN_MCS11) && (TxRate <= MGN_MCS12)) 	/* 16QAM */
			PwrTrackingLimit = 32; 				/* +4dB */
		else if ((TxRate >= MGN_MCS13) && (TxRate <= MGN_MCS15)) 	/* 64QAM */
			PwrTrackingLimit = 30; 				/* +3dB */

		/* Ulli 802.11ac */
		/* 2 VHT */
		else if ((TxRate >= MGN_VHT1SS_MCS0) && (TxRate <= MGN_VHT1SS_MCS2))    /* QPSK/BPSK */
			PwrTrackingLimit = 34;						/* +5dB */
		else if ((TxRate >= MGN_VHT1SS_MCS3) && (TxRate <= MGN_VHT1SS_MCS4))    /* 16QAM */
			PwrTrackingLimit = 32;						/* +4dB */
		else if ((TxRate >= MGN_VHT1SS_MCS5) && (TxRate <= MGN_VHT1SS_MCS6))    /* 64QAM */
			PwrTrackingLimit = 30;						/* +3dB */
		else if (TxRate == MGN_VHT1SS_MCS7)					/* 64QAM */
			PwrTrackingLimit = 28;						/* +2dB */
		else if (TxRate == MGN_VHT1SS_MCS8)					/* 256QAM */
			PwrTrackingLimit = 26;						/* +1dB */
		else if (TxRate == MGN_VHT1SS_MCS9)					/* 256QAM */
			PwrTrackingLimit = 24;						/* +0dB */

		else if ((TxRate >= MGN_VHT2SS_MCS0) && (TxRate <= MGN_VHT2SS_MCS2)) 	/* QPSK/BPSK */
			PwrTrackingLimit = 34; 						/* +5dB */
		else if ((TxRate >= MGN_VHT2SS_MCS3) && (TxRate <= MGN_VHT2SS_MCS4)) 	/* 16QAM */
			PwrTrackingLimit = 32; 						/* +4dB */
		else if ((TxRate >= MGN_VHT2SS_MCS5) && (TxRate <= MGN_VHT2SS_MCS6)) 	/* 64QAM */
			PwrTrackingLimit = 30; 						/* +3dB */
		else if (TxRate == MGN_VHT2SS_MCS7) 					/* 64QAM */
			PwrTrackingLimit = 28; 						/* +2dB */
		else if (TxRate == MGN_VHT2SS_MCS8) 					/* 256QAM */
			PwrTrackingLimit = 26; 						/* +1dB */
		else if (TxRate == MGN_VHT2SS_MCS9) 					/* 256QAM */
			PwrTrackingLimit = 24; 						/* +0dB */

		else
			PwrTrackingLimit = 24;
	}
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD, ("TxRate=0x%x, PwrTrackingLimit=%d\n", TxRate, PwrTrackingLimit));

	if (Method == BBSWING) {
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD, ("===>ODM_TxPwrTrackSetPwr8812A\n"));

		if (RFPath == ODM_RF_PATH_A) {
			finalBbSwingIdx[ODM_RF_PATH_A] = (pDM_Odm->RFCalibrateInfo.OFDM_index[ODM_RF_PATH_A] > PwrTrackingLimit) ? PwrTrackingLimit : pDM_Odm->RFCalibrateInfo.OFDM_index[ODM_RF_PATH_A];
			ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD, ("pDM_Odm->RFCalibrateInfo.OFDM_index[ODM_RF_PATH_A]=%d, pDM_Odm->RealBbSwingIdx[ODM_RF_PATH_A]=%d\n",
				pDM_Odm->RFCalibrateInfo.OFDM_index[ODM_RF_PATH_A], finalBbSwingIdx[ODM_RF_PATH_A]));

			ODM_SetBBReg(pDM_Odm, rA_TxScale_Jaguar, 0xFFE00000, TxScalingTable_Jaguar[finalBbSwingIdx[ODM_RF_PATH_A]]);
		} else {
			finalBbSwingIdx[ODM_RF_PATH_B] = (pDM_Odm->RFCalibrateInfo.OFDM_index[ODM_RF_PATH_B] > PwrTrackingLimit) ? PwrTrackingLimit : pDM_Odm->RFCalibrateInfo.OFDM_index[ODM_RF_PATH_B];
			ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD, ("pDM_Odm->RFCalibrateInfo.OFDM_index[ODM_RF_PATH_B]=%d, pDM_Odm->RealBbSwingIdx[ODM_RF_PATH_B]=%d\n",
				pDM_Odm->RFCalibrateInfo.OFDM_index[ODM_RF_PATH_B], finalBbSwingIdx[ODM_RF_PATH_B]));

			ODM_SetBBReg(pDM_Odm, rB_TxScale_Jaguar, 0xFFE00000, TxScalingTable_Jaguar[finalBbSwingIdx[ODM_RF_PATH_B]]);
		}
	} else if (Method == MIX_MODE) {
			ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD, ("pDM_Odm->DefaultOfdmIndex=%d, pDM_Odm->Aboslute_OFDMSwingIdx[RFPath]=%d, RF_Path = %d\n",
				pDM_Odm->DefaultOfdmIndex, pDM_Odm->Aboslute_OFDMSwingIdx[RFPath], RFPath));


			Final_OFDM_Swing_Index = pDM_Odm->DefaultOfdmIndex + pDM_Odm->Aboslute_OFDMSwingIdx[RFPath];

			if (RFPath == ODM_RF_PATH_A) {
				if (Final_OFDM_Swing_Index > PwrTrackingLimit) {    /* BBSwing higher then Limit */
					pDM_Odm->Remnant_CCKSwingIdx = Final_OFDM_Swing_Index - PwrTrackingLimit;            /*  CCK Follow the same compensate value as Path A */
					pDM_Odm->Remnant_OFDMSwingIdx[RFPath] = Final_OFDM_Swing_Index - PwrTrackingLimit;

					ODM_SetBBReg(pDM_Odm, rA_TxScale_Jaguar, 0xFFE00000, TxScalingTable_Jaguar[PwrTrackingLimit]);

					pDM_Odm->Modify_TxAGC_Flag_PathA = TRUE;

					/* et TxAGC Page C{}; */
					/* Adapter->HalFunc.SetTxPowerLevelHandler(Adapter, pHalData->CurrentChannel); */
					PHY_SetTxPowerLevel8812(Adapter, pHalData->CurrentChannel);

					ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD, ("******Path_A Over BBSwing Limit , PwrTrackingLimit = %d , Remnant TxAGC Value = %d \n", PwrTrackingLimit, pDM_Odm->Remnant_OFDMSwingIdx[RFPath]));
				} else if (Final_OFDM_Swing_Index < 0) {
					pDM_Odm->Remnant_CCKSwingIdx = Final_OFDM_Swing_Index;            /* CCK Follow the same compensate value as Path A */
					pDM_Odm->Remnant_OFDMSwingIdx[RFPath] = Final_OFDM_Swing_Index;

					ODM_SetBBReg(pDM_Odm, rA_TxScale_Jaguar, 0xFFE00000, TxScalingTable_Jaguar[0]);

					pDM_Odm->Modify_TxAGC_Flag_PathA = TRUE;

					/* Set TxAGC Page C{}; */
					/* Adapter->HalFunc.SetTxPowerLevelHandler(Adapter, pHalData->CurrentChannel);*/
					PHY_SetTxPowerLevel8812(Adapter, pHalData->CurrentChannel);

					ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD, ("******Path_A Lower then BBSwing lower bound  0 , Remnant TxAGC Value = %d \n", pDM_Odm->Remnant_OFDMSwingIdx[RFPath]));
				} else 	{
					ODM_SetBBReg(pDM_Odm, rA_TxScale_Jaguar, 0xFFE00000, TxScalingTable_Jaguar[Final_OFDM_Swing_Index]);

					ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD, ("******Path_A Compensate with BBSwing , Final_OFDM_Swing_Index = %d \n", Final_OFDM_Swing_Index));

					if (pDM_Odm->Modify_TxAGC_Flag_PathA) { /* If TxAGC has changed, reset TxAGC again */
						pDM_Odm->Remnant_CCKSwingIdx = 0;
						pDM_Odm->Remnant_OFDMSwingIdx[RFPath] = 0;

						/* Set TxAGC Page C{}; */
						/* Adapter->HalFunc.SetTxPowerLevelHandler(Adapter, pHalData->CurrentChannel); */
						PHY_SetTxPowerLevel8812(Adapter, pHalData->CurrentChannel);

						pDM_Odm->Modify_TxAGC_Flag_PathA = FALSE;

						ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD, ("******Path_A pDM_Odm->Modify_TxAGC_Flag = FALSE \n"));
					}
				}
			}

			if (RFPath == ODM_RF_PATH_B) {
				if (Final_OFDM_Swing_Index > PwrTrackingLimit) {			/* BBSwing higher then Limit */
					pDM_Odm->Remnant_OFDMSwingIdx[RFPath] = Final_OFDM_Swing_Index - PwrTrackingLimit;

					ODM_SetBBReg(pDM_Odm, rB_TxScale_Jaguar, 0xFFE00000, TxScalingTable_Jaguar[PwrTrackingLimit]);

					pDM_Odm->Modify_TxAGC_Flag_PathB = TRUE;

					/* Set TxAGC Page E{}; */

					ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD, ("******Path_B Over BBSwing Limit , PwrTrackingLimit = %d , Remnant TxAGC Value = %d \n", PwrTrackingLimit, pDM_Odm->Remnant_OFDMSwingIdx[RFPath]));
				} else if (Final_OFDM_Swing_Index < 0) {
					pDM_Odm->Remnant_OFDMSwingIdx[RFPath] = Final_OFDM_Swing_Index;

					ODM_SetBBReg(pDM_Odm, rB_TxScale_Jaguar, 0xFFE00000, TxScalingTable_Jaguar[0]);

					pDM_Odm->Modify_TxAGC_Flag_PathB = TRUE;

					/* Set TxAGC Page E{}; */

					ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD, ("******Path_B Lower then BBSwing lower bound  0 , Remnant TxAGC Value = %d \n", pDM_Odm->Remnant_OFDMSwingIdx[RFPath]));
				} else {
					ODM_SetBBReg(pDM_Odm, rB_TxScale_Jaguar, 0xFFE00000, TxScalingTable_Jaguar[Final_OFDM_Swing_Index]);

					ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD, ("******Path_B Compensate with BBSwing , Final_OFDM_Swing_Index = %d \n", Final_OFDM_Swing_Index));

					if (pDM_Odm->Modify_TxAGC_Flag_PathB) {			/* If TxAGC has changed, reset TxAGC again */
						pDM_Odm->Remnant_CCKSwingIdx = 0;
						pDM_Odm->Remnant_OFDMSwingIdx[RFPath] = 0;

						/* Set TxAGC Page E{}; */

						pDM_Odm->Modify_TxAGC_Flag_PathB = FALSE;

						ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD, ("******Path_B pDM_Odm->Modify_TxAGC_Flag = FALSE \n"));
					}
				}
			}

	} else {
		return;
	}
}

static void GetDeltaSwingTable_8812A(struct rtl_dm *pDM_Odm,
	u8 **TemperatureUP_A, u8 **TemperatureDOWN_A,
	u8 **TemperatureUP_B, u8 **TemperatureDOWN_B)
{
	struct rtl_priv *       Adapter = pDM_Odm->Adapter;
	PODM_RF_CAL_T  	pRFCalibrateInfo = &(pDM_Odm->RFCalibrateInfo);
	 struct rtw_hal  	*pHalData = GET_HAL_DATA(Adapter);
	u16	rate = *(pDM_Odm->pForcedDataRate);
	u8         	channel   		 = pHalData->CurrentChannel;

	if (1 <= channel && channel <= 14) {
		if (IS_CCK_RATE(rate)) {
			*TemperatureUP_A   = pRFCalibrateInfo->DeltaSwingTableIdx_2GCCKA_P;
			*TemperatureDOWN_A = pRFCalibrateInfo->DeltaSwingTableIdx_2GCCKA_N;
			*TemperatureUP_B   = pRFCalibrateInfo->DeltaSwingTableIdx_2GCCKB_P;
			*TemperatureDOWN_B = pRFCalibrateInfo->DeltaSwingTableIdx_2GCCKB_N;
		} else {
			*TemperatureUP_A   = pRFCalibrateInfo->DeltaSwingTableIdx_2GA_P;
			*TemperatureDOWN_A = pRFCalibrateInfo->DeltaSwingTableIdx_2GA_N;
			*TemperatureUP_B   = pRFCalibrateInfo->DeltaSwingTableIdx_2GB_P;
			*TemperatureDOWN_B = pRFCalibrateInfo->DeltaSwingTableIdx_2GB_N;
		}
	} else if (36 <= channel && channel <= 64) {
		*TemperatureUP_A   = pRFCalibrateInfo->DeltaSwingTableIdx_5GA_P[0];
		*TemperatureDOWN_A = pRFCalibrateInfo->DeltaSwingTableIdx_5GA_N[0];
		*TemperatureUP_B   = pRFCalibrateInfo->DeltaSwingTableIdx_5GB_P[0];
		*TemperatureDOWN_B = pRFCalibrateInfo->DeltaSwingTableIdx_5GB_N[0];
	} else if (100 <= channel && channel <= 140) {
		*TemperatureUP_A   = pRFCalibrateInfo->DeltaSwingTableIdx_5GA_P[1];
		*TemperatureDOWN_A = pRFCalibrateInfo->DeltaSwingTableIdx_5GA_N[1];
		*TemperatureUP_B   = pRFCalibrateInfo->DeltaSwingTableIdx_5GB_P[1];
		*TemperatureDOWN_B = pRFCalibrateInfo->DeltaSwingTableIdx_5GB_N[1];
	} else if (149 <= channel && channel <= 173) {
		*TemperatureUP_A   = pRFCalibrateInfo->DeltaSwingTableIdx_5GA_P[2];
		*TemperatureDOWN_A = pRFCalibrateInfo->DeltaSwingTableIdx_5GA_N[2];
		*TemperatureUP_B   = pRFCalibrateInfo->DeltaSwingTableIdx_5GB_P[2];
		*TemperatureDOWN_B = pRFCalibrateInfo->DeltaSwingTableIdx_5GB_N[2];
	} else {
		*TemperatureUP_A   = (u8 *)DeltaSwingTableIdx_2GA_P_8188E;
		*TemperatureDOWN_A = (u8 *)DeltaSwingTableIdx_2GA_N_8188E;
		*TemperatureUP_B   = (u8 *)DeltaSwingTableIdx_2GA_P_8188E;
		*TemperatureDOWN_B = (u8 *)DeltaSwingTableIdx_2GA_N_8188E;
	}

	return;
}

void ConfigureTxpowerTrack_8812A(PTXPWRTRACK_CFG pConfig)
{
	pConfig->SwingTableSize_CCK = TXSCALE_TABLE_SIZE;
	pConfig->SwingTableSize_OFDM = TXSCALE_TABLE_SIZE;
	pConfig->Threshold_IQK = IQK_THRESHOLD;
	pConfig->AverageThermalNum = AVG_THERMAL_NUM_8812A;
	pConfig->RfPathCount = MAX_PATH_NUM_8812A;
	pConfig->ThermalRegAddr = RF_T_METER_8812A;

	pConfig->ODM_TxPwrTrackSetPwr = ODM_TxPwrTrackSetPwr8812A;
	pConfig->DoIQK = DoIQK_8812A;
	pConfig->PHY_LCCalibrate = PHY_LCCalibrate_8812A;
	pConfig->GetDeltaSwingTable = GetDeltaSwingTable_8812A;
}


/*
 * 2011/07/26 MH Add an API for testing IQK fail case.
 * MP Already declare in odm.c
 */

#define BW_20M 	0
#define	BW_40M  1
#define	BW_80M	2

static void phy_LCCalibrate_8812A(struct rtl_dm *pDM_Odm, BOOLEAN	is2T)
{
	uint32_t	/*RF_Amode=0, RF_Bmode=0,*/ LC_Cal = 0, tmp = 0;

	/* Check continuous TX and Packet TX */
	uint32_t	reg0x914 = ODM_Read4Byte(pDM_Odm, rSingleTone_ContTx_Jaguar);;

	/* Backup RF reg18. */
	LC_Cal = ODM_GetRFReg(pDM_Odm, ODM_RF_PATH_A, RF_CHNLBW, bRFRegOffsetMask);

	if ((reg0x914 & 0x70000) != 0)	/* If contTx, disable all continuous TX. 0x914[18:16] */
		/*
		 *  <20121121, Kordan> A workaround: If we set 0x914[18:16] as zero, BB turns off ContTx
		 *  until another packet comes in. To avoid ContTx being turned off, we skip this step.
		 * ODM_Write4Byte(pDM_Odm, rSingleTone_ContTx_Jaguar, reg0x914 & (~0x70000));
		 */

		;
	else		/* If packet Tx-ing, pause Tx. */
		ODM_Write1Byte(pDM_Odm, REG_TXPAUSE, 0xFF);


/*
	//3 1. Read original RF mode
	RF_Amode = ODM_GetRFReg(pDM_Odm, ODM_RF_PATH_A, RF_AC, bRFRegOffsetMask);
	if(is2T)
		RF_Bmode = ODM_GetRFReg(pDM_Odm, ODM_RF_PATH_B, RF_AC, bRFRegOffsetMask);


	//3 2. Set RF mode = standby mode
	ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_A, RF_AC, bRFRegOffsetMask, (RF_Amode&0x8FFFF)|0x10000);
	if(is2T)
		ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_B, RF_AC, bRFRegOffsetMask, (RF_Bmode&0x8FFFF)|0x10000);
*/

	/* Enter LCK mode */
	tmp = ODM_GetRFReg(pDM_Odm, ODM_RF_PATH_A, RF_LCK, bRFRegOffsetMask);
	ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_A, RF_LCK, bRFRegOffsetMask, tmp | BIT14);

	/* 3 3. Read RF reg18 */
	LC_Cal = ODM_GetRFReg(pDM_Odm, ODM_RF_PATH_A, RF_CHNLBW, bRFRegOffsetMask);

	/* 3 4. Set LC calibration begin bit15 */
	ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_A, RF_CHNLBW, bRFRegOffsetMask, LC_Cal|0x08000);

	/* Leave LCK mode */
	tmp = ODM_GetRFReg(pDM_Odm, ODM_RF_PATH_A, RF_LCK, bRFRegOffsetMask);
	ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_A, RF_LCK, bRFRegOffsetMask, tmp & ~BIT14);

	mdelay(100);

	/* 3 Restore original situation */
	if ((reg0x914 & 70000) != 0) {	/* Deal with contisuous TX case, 0x914[18:16] */
		/*
		 * <20121121, Kordan> A workaround: If we set 0x914[18:16] as zero, BB turns off ContTx
		 * until another packet comes in. To avoid ContTx being turned off, we skip this step.
		 * ODM_Write4Byte(pDM_Odm, rSingleTone_ContTx_Jaguar, reg0x914);
		 */
		;
	} else {
		/* Deal with Packet TX case */
		ODM_Write1Byte(pDM_Odm, REG_TXPAUSE, 0x00);
	}

	/* Recover channel number */
	ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_A, RF_CHNLBW, bRFRegOffsetMask, LC_Cal);

	/*
	ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_A, RF_AC, bRFRegOffsetMask, RF_Amode);
	if(is2T)
		ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_B, RF_AC, bRFRegOffsetMask, RF_Bmode);
		*/

}

#define		DP_BB_REG_NUM		7
#define		DP_RF_REG_NUM		1
#define		DP_RETRY_LIMIT		10
#define		DP_PATH_NUM		2
#define		DP_DPK_NUM		3
#define		DP_DPK_VALUE_NUM	2

void PHY_LCCalibrate_8812A(struct rtl_dm *pDM_Odm)
{
	BOOLEAN 		bStartContTx = FALSE, bSingleTone = FALSE, bCarrierSuppression = FALSE;

	struct rtl_priv *		pAdapter = pDM_Odm->Adapter;
	 struct rtw_hal	*pHalData = GET_HAL_DATA(pAdapter);

	ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("===> PHY_LCCalibrate_8812A\n"));

	ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("<=== PHY_LCCalibrate_8812A\n"));

}
