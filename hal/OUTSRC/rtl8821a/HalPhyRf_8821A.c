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
#include <../rtl8821au/phy.h>

/*---------------------------Define Local Constant---------------------------*/
/* 2010/04/25 MH Define the max tx power tracking tx agc power. */
#define		ODM_TXPWRTRACK_MAX_IDX8821A		6

/*---------------------------Define Local Constant---------------------------*/


/*
 * 3 ============================================================
//3 Tx Power Tracking
//3 ============================================================
*/

static void PHY_LCCalibrate_8821A(PDM_ODM_T pDM_Odm);

static void DoIQK_8821A(PDM_ODM_T pDM_Odm, u1Byte DeltaThermalIndex,
	u1Byte ThermalValue, u1Byte Threshold)
{
	struct rtl_priv *Adapter = pDM_Odm->Adapter;
	 struct rtw_hal *pHalData = GET_HAL_DATA(Adapter);

	ODM_ResetIQKResult(pDM_Odm);

	pDM_Odm->RFCalibrateInfo.ThermalValue_IQK = ThermalValue;
	rtl8821au_phy_iq_calibrate(Adapter, FALSE);
}


static void ODM_TxPwrTrackSetPwr8821A(PDM_ODM_T pDM_Odm, PWRTRACK_METHOD Method,
	u1Byte RFPath, u1Byte ChannelMappedIndex)
{
	struct rtl_priv *Adapter = pDM_Odm->Adapter;
	struct rtw_hal *pHalData = GET_HAL_DATA(Adapter);

	u1Byte PwrTrackingLimit = 26; /* +1.0dB */
	u1Byte TxRate = 0xFF;
	s1Byte Final_OFDM_Swing_Index = 0;
	s1Byte Final_CCK_Swing_Index = 0;
	u1Byte i = 0;
	uint32_t finalBbSwingIdx[1];

	ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD, ("===>ODM_TxPwrTrackSetPwr8821A\n"));

	if (TxRate != 0xFF) {
		/* 2 CCK */
		if ((TxRate >= MGN_1M) && (TxRate <= MGN_11M))
			PwrTrackingLimit = 32;				/* +4dB */
		/* 2 OFDM */
		else if ((TxRate >= MGN_6M) && (TxRate <= MGN_48M))
			PwrTrackingLimit = 32;				/* +4dB */
		else if (TxRate == MGN_54M)
			PwrTrackingLimit = 30;				/* +3dB */
		/* 2 HT */
		else if ((TxRate >= MGN_MCS0) && (TxRate <= MGN_MCS2))  /* QPSK/BPSK */
			PwrTrackingLimit = 34;				/* +5dB */
		else if ((TxRate >= MGN_MCS3) && (TxRate <= MGN_MCS4))  /* 16QAM */
			PwrTrackingLimit = 32;				/* +4dB */
		else if ((TxRate >= MGN_MCS5) && (TxRate <= MGN_MCS7))  /* 64QAM */
			PwrTrackingLimit = 30;				/* +3dB */
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
		else
			PwrTrackingLimit = 24;
	}
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD, ("TxRate=0x%x, PwrTrackingLimit=%d\n", TxRate, PwrTrackingLimit));

	if (Method == BBSWING) {
		if (RFPath == ODM_RF_PATH_A) {
			finalBbSwingIdx[ODM_RF_PATH_A] = (pDM_Odm->RFCalibrateInfo.OFDM_index[ODM_RF_PATH_A] > PwrTrackingLimit) ? PwrTrackingLimit : pDM_Odm->RFCalibrateInfo.OFDM_index[ODM_RF_PATH_A];
			ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD, ("pDM_Odm->RFCalibrateInfo.OFDM_index[ODM_RF_PATH_A]=%d, pDM_Odm->RealBbSwingIdx[ODM_RF_PATH_A]=%d\n",
				pDM_Odm->RFCalibrateInfo.OFDM_index[ODM_RF_PATH_A], finalBbSwingIdx[ODM_RF_PATH_A]));

			ODM_SetBBReg(pDM_Odm, rA_TxScale_Jaguar, 0xFFE00000, TxScalingTable_Jaguar[finalBbSwingIdx[ODM_RF_PATH_A]]);
		}
	} else if (Method == MIX_MODE) {
			ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD, ("pDM_Odm->DefaultOfdmIndex=%d, pDM_Odm->Aboslute_OFDMSwingIdx[RFPath]=%d, RF_Path = %d\n",
				pDM_Odm->DefaultOfdmIndex, pDM_Odm->Aboslute_OFDMSwingIdx[RFPath], RFPath));


			Final_OFDM_Swing_Index = pDM_Odm->DefaultOfdmIndex + pDM_Odm->Aboslute_OFDMSwingIdx[RFPath];

			if (RFPath == ODM_RF_PATH_A) {
				if (Final_OFDM_Swing_Index > PwrTrackingLimit) {
					/* BBSwing higher then Limit */
					pDM_Odm->Remnant_CCKSwingIdx = Final_OFDM_Swing_Index - PwrTrackingLimit;            /* CCK Follow the same compensate value as Path A */
					pDM_Odm->Remnant_OFDMSwingIdx[RFPath] = Final_OFDM_Swing_Index - PwrTrackingLimit;

					ODM_SetBBReg(pDM_Odm, rA_TxScale_Jaguar, 0xFFE00000, TxScalingTable_Jaguar[PwrTrackingLimit]);

					pDM_Odm->Modify_TxAGC_Flag_PathA = TRUE;

					/* Set TxAGC Page C{}; */
					/* Adapter->HalFunc.SetTxPowerLevelHandler(Adapter, pHalData->CurrentChannel); */
					PHY_SetTxPowerLevel8812(Adapter, pHalData->CurrentChannel);

					ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD, ("******Path_A Over BBSwing Limit , PwrTrackingLimit = %d , Remnant TxAGC Value = %d \n", PwrTrackingLimit, pDM_Odm->Remnant_OFDMSwingIdx[RFPath]));
				} else if (Final_OFDM_Swing_Index < 0) {
					pDM_Odm->Remnant_CCKSwingIdx = Final_OFDM_Swing_Index;            /* CCK Follow the same compensate value as Path A */
					pDM_Odm->Remnant_OFDMSwingIdx[RFPath] = Final_OFDM_Swing_Index;

					ODM_SetBBReg(pDM_Odm, rA_TxScale_Jaguar, 0xFFE00000, TxScalingTable_Jaguar[0]);

					pDM_Odm->Modify_TxAGC_Flag_PathA = TRUE;

					/* Set TxAGC Page C{}; */
					/* Adapter->HalFunc.SetTxPowerLevelHandler(Adapter, pHalData->CurrentChannel); */
					PHY_SetTxPowerLevel8812(Adapter, pHalData->CurrentChannel);

					ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD, ("******Path_A Lower then BBSwing lower bound  0 , Remnant TxAGC Value = %d \n", pDM_Odm->Remnant_OFDMSwingIdx[RFPath]));
				} else {
					ODM_SetBBReg(pDM_Odm, rA_TxScale_Jaguar, 0xFFE00000, TxScalingTable_Jaguar[Final_OFDM_Swing_Index]);

					ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD, ("******Path_A Compensate with BBSwing , Final_OFDM_Swing_Index = %d \n", Final_OFDM_Swing_Index));

					if (pDM_Odm->Modify_TxAGC_Flag_PathA) {
						/* If TxAGC has changed, reset TxAGC again */
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

	} else {
		return;
	}
}

static void GetDeltaSwingTable_8821A(PDM_ODM_T pDM_Odm,
	pu1Byte *TemperatureUP_A, pu1Byte *TemperatureDOWN_A,
	pu1Byte *TemperatureUP_B, pu1Byte *TemperatureDOWN_B)
{
	struct rtl_priv *       Adapter = pDM_Odm->Adapter;
	PODM_RF_CAL_T  	pRFCalibrateInfo = &(pDM_Odm->RFCalibrateInfo);
	 struct rtw_hal  	*pHalData = GET_HAL_DATA(Adapter);
	/* uint16_t     rate = pMgntInfo->ForcedDataRate; */
	uint16_t	rate = 0;
	u1Byte         	channel   		 = pHalData->CurrentChannel;

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
		*TemperatureUP_A   = (pu1Byte)DeltaSwingTableIdx_2GA_P_8188E;
		*TemperatureDOWN_A = (pu1Byte)DeltaSwingTableIdx_2GA_N_8188E;
		*TemperatureUP_B   = (pu1Byte)DeltaSwingTableIdx_2GA_P_8188E;
		*TemperatureDOWN_B = (pu1Byte)DeltaSwingTableIdx_2GA_N_8188E;
	}

	return;
}

void ConfigureTxpowerTrack_8821A(PTXPWRTRACK_CFG pConfig)
{
	pConfig->SwingTableSize_CCK = TXSCALE_TABLE_SIZE;
	pConfig->SwingTableSize_OFDM = TXSCALE_TABLE_SIZE;
	pConfig->Threshold_IQK = IQK_THRESHOLD;
	pConfig->AverageThermalNum = AVG_THERMAL_NUM_8812A;
	pConfig->RfPathCount = MAX_PATH_NUM_8821A;
	pConfig->ThermalRegAddr = RF_T_METER_8812A;

	pConfig->ODM_TxPwrTrackSetPwr = ODM_TxPwrTrackSetPwr8821A;
	pConfig->DoIQK = DoIQK_8821A;
	pConfig->PHY_LCCalibrate = PHY_LCCalibrate_8821A;
	pConfig->GetDeltaSwingTable = GetDeltaSwingTable_8821A;
}

/* 1 7.	IQK */
#define MAX_TOLERANCE		5
#define IQK_DELAY_TIME		1		/* ms */




void phy_IQCalibrate_By_FW_8821A(struct rtl_priv *pAdapter)
{
	 struct rtw_hal	*pHalData = GET_HAL_DATA(pAdapter);
	u1Byte			IQKcmd[3] = {pHalData->CurrentChannel, 0x0, 0x0};
	u1Byte			Buf1 = 0x0;
	u1Byte			Buf2 = 0x0;

	/* Byte 2, Bit 4 ~ Bit 5 : BandType */
	if (pHalData->CurrentBandType)
		Buf1 = 0x2<<4;
	else
		Buf1 = 0x1<<4;

	/* Byte 2, Bit 0 ~ Bit 3 : Bandwidth */
	if (pHalData->CurrentChannelBW == CHANNEL_WIDTH_20)
		Buf2 = 0x1;
	else if (pHalData->CurrentChannelBW == CHANNEL_WIDTH_40)
		Buf2 = 0x1<<1;
	else if (pHalData->CurrentChannelBW == CHANNEL_WIDTH_80)
		Buf2 = 0x1<<2;
	else
		Buf2 = 0x1<<3;

	IQKcmd[1] = Buf1 | Buf2;
	IQKcmd[2] = pHalData->ExternalPA_5G | pHalData->ExternalLNA_5G<<1;


	/* RT_TRACE(COMP_MP, DBG_LOUD, ("== IQK Start ==\n")); */

	/* FillH2CCmd_8812(pAdapter, 0x45, 3, IQKcmd); */

}



#define		DP_BB_REG_NUM		7
#define		DP_RF_REG_NUM		1
#define		DP_RETRY_LIMIT		10
#define		DP_PATH_NUM		2
#define		DP_DPK_NUM		3
#define		DP_DPK_VALUE_NUM	2



static void PHY_LCCalibrate_8821A(PDM_ODM_T pDM_Odm)
{
	PHY_LCCalibrate_8812A(pDM_Odm);
}


