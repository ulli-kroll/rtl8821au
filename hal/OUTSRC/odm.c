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

/*
 * ============================================================
 * include files
 * ============================================================
 */

#include "odm_precomp.h"
#include <../rtl8821au/reg.h>


const u16 dB_Invert_Table[8][12] = {
	{	1,		1,		1,		2,		2,		2,		2,		3,		3,		3,		4,		4},
	{	4,		5,		6,		6,		7,		8,		9,		10,		11,		13,		14,		16},
	{	18,		20,		22,		25,		28,		32,		35,		40,		45,		50,		56,		63},
	{	71,		79,		89,		100,	112,	126,	141,	158,	178,	200,	224,	251},
	{	282,	316,	355,	398,	447,	501,	562,	631,	708,	794,	891,	1000},
	{	1122,	1259,	1413,	1585,	1778,	1995,	2239,	2512,	2818,	3162,	3548,	3981},
	{	4467,	5012,	5623,	6310,	7079,	7943,	8913,	10000,	11220,	12589,	14125,	15849},
	{	17783,	19953,	22387,	25119,	28184,	31623,	35481,	39811,	44668,	50119,	56234,	65535}
	};

/*
 * 20100515 Joseph: Add global variable to keep temporary scan list for antenna switching test.
 * u8			tmpNumBssDesc;
 * RT_WLAN_BSS	tmpbssDesc[MAX_BSS_DESC];
 */

static uint32_t edca_setting_UL[HT_IOT_PEER_MAX] =
/* UNKNOWN		REALTEK_90	REALTEK_92SE	BROADCOM		RALINK		ATHEROS		CISCO		MERU        MARVELL	92U_AP		SELF_AP(DownLink/Tx) */
{ 0x5e4322, 		0xa44f, 		0x5e4322,		0x5ea32b,  		0x5ea422, 	0x5ea322,	0x3ea430,	0x5ea42b, 0x5ea44f,	0x5e4322,	0x5e4322};


static uint32_t edca_setting_DL[HT_IOT_PEER_MAX] =
/* UNKNOWN		REALTEK_90	REALTEK_92SE	BROADCOM		RALINK		ATHEROS		CISCO		MERU,       MARVELL	92U_AP		SELF_AP(UpLink/Rx) */
{ 0xa44f, 		0x5ea44f, 	0x5e4322, 		0x5ea42b, 		0xa44f, 		0xa630, 		0x5ea630,	0x5ea42b, 0xa44f,		0xa42b,		0xa42b};

static uint32_t edca_setting_DL_GMode[HT_IOT_PEER_MAX] =
/* UNKNOWN		REALTEK_90	REALTEK_92SE	BROADCOM		RALINK		ATHEROS		CISCO		MERU,       MARVELL	92U_AP		SELF_AP */
{ 0x4322, 		0xa44f, 		0x5e4322,		0xa42b, 			0x5e4322, 	0x4322, 		0xa42b,		0x5ea42b, 0xa44f,		0x5e4322,	0x5ea42b};


/*
 * ============================================================
 * EDCA Paramter for AP/ADSL   by Mingzhi 2011-11-22
 * ============================================================
 */

/*
 * ============================================================
 *  Global var
 * ============================================================
 */


uint32_t TxScalingTable_Jaguar[TXSCALE_TABLE_SIZE] = {
	0x081, /* 0,  -12.0dB */
	0x088, /* 1,  -11.5dB */
	0x090, /* 2,  -11.0dB */
	0x099, /* 3,  -10.5dB */
	0x0A2, /* 4,  -10.0dB */
	0x0AC, /* 5,  -9.5dB */
	0x0B6, /* 6,  -9.0dB */
	0x0C0, /* 7,  -8.5dB */
	0x0CC, /* 8,  -8.0dB */
	0x0D8, /* 9,  -7.5dB */
	0x0E5, /* 10, -7.0dB */
	0x0F2, /* 11, -6.5dB */
	0x101, /* 12, -6.0dB */
	0x110, /* 13, -5.5dB */
	0x120, /* 14, -5.0dB */
	0x131, /* 15, -4.5dB */
	0x143, /* 16, -4.0dB */
	0x156, /* 17, -3.5dB */
	0x16A, /* 18, -3.0dB */
	0x180, /* 19, -2.5dB */
	0x197, /* 20, -2.0dB */
	0x1AF, /* 21, -1.5dB */
	0x1C8, /* 22, -1.0dB */
	0x1E3, /* 23, -0.5dB */
	0x200, /* 24, +0  dB */
	0x21E, /* 25, +0.5dB */
	0x23E, /* 26, +1.0dB */
	0x261, /* 27, +1.5dB */
	0x285, /* 28, +2.0dB */
	0x2AB, /* 29, +2.5dB */
	0x2D3, /* 30, +3.0dB */
	0x2FE, /* 31, +3.5dB */
	0x32B, /* 32, +4.0dB */
	0x35C, /* 33, +4.5dB */
	0x38E, /* 34, +5.0dB */
	0x3C4, /* 35, +5.5dB */
	0x3FE  /* 36, +6.0dB */
};

#ifdef AP_BUILD_WORKAROUND

unsigned int TxPwrTrk_OFDM_SwingTbl[TxPwrTrk_OFDM_SwingTbl_Len] = {
	/*  +6.0dB */ 0x7f8001fe,
	/*  +5.5dB */ 0x788001e2,
	/*  +5.0dB */ 0x71c001c7,
	/*  +4.5dB */ 0x6b8001ae,
	/*  +4.0dB */ 0x65400195,
	/*  +3.5dB */ 0x5fc0017f,
	/*  +3.0dB */ 0x5a400169,
	/*  +2.5dB */ 0x55400155,
	/*  +2.0dB */ 0x50800142,
	/*  +1.5dB */ 0x4c000130,
	/*  +1.0dB */ 0x47c0011f,
	/*  +0.5dB */ 0x43c0010f,
	/*   0.0dB */ 0x40000100,
	/*  -0.5dB */ 0x3c8000f2,
	/*  -1.0dB */ 0x390000e4,
	/*  -1.5dB */ 0x35c000d7,
	/*  -2.0dB */ 0x32c000cb,
	/*  -2.5dB */ 0x300000c0,
	/*  -3.0dB */ 0x2d4000b5,
	/*  -3.5dB */ 0x2ac000ab,
	/*  -4.0dB */ 0x288000a2,
	/*  -4.5dB */ 0x26000098,
	/*  -5.0dB */ 0x24000090,
	/*  -5.5dB */ 0x22000088,
	/*  -6.0dB */ 0x20000080,
	/*  -6.5dB */ 0x1a00006c,
	/*  -7.0dB */ 0x1c800072,
	/*  -7.5dB */ 0x18000060,
	/*  -8.0dB */ 0x19800066,
	/*  -8.5dB */ 0x15800056,
	/*  -9.0dB */ 0x26c0005b,
	/*  -9.5dB */ 0x14400051,
	/* -10.0dB */ 0x24400051,
	/* -10.5dB */ 0x1300004c,
	/* -11.0dB */ 0x12000048,
	/* -11.5dB */ 0x11000044,
	/* -12.0dB */ 0x10000040
};
#endif

/*
 * ============================================================
 * Local Function predefine.
 * ============================================================
 */

/* START------------COMMON INFO RELATED--------------- */
void odm_CommonInfoSelfInit(struct _rtw_dm *pDM_Odm);
void odm_CommonInfoSelfUpdate(struct _rtw_dm * pDM_Odm);
/*
void odm_FindMinimumRSSI(struct _rtw_dm *pDM_Odm);
void odm_IsLinked(struct _rtw_dm *pDM_Odm);
*/
/* END------------COMMON INFO RELATED--------------- */

/* START---------------DIG--------------------------- */
void odm_FalseAlarmCounterStatistics(struct _rtw_dm *pDM_Odm);
void odm_DIGInit(struct _rtw_dm *pDM_Odm);
void odm_DIG(struct _rtw_dm *pDM_Odm);
void odm_CCKPacketDetectionThresh(struct _rtw_dm *pDM_Odm);
void odm_AdaptivityInit(struct _rtw_dm *pDM_Odm);
void odm_Adaptivity(struct _rtw_dm *pDM_Odm, u8 IGI);
/* END---------------DIG--------------------------- */

/* START-------BB POWER SAVE----------------------- */
void odm_1R_CCA(struct _rtw_dm *pDM_Odm);
/* END---------BB POWER SAVE----------------------- */

/* START-----------------PSD----------------------- */
/* END-------------------PSD----------------------- */

void odm_RefreshRateAdaptiveMaskCE(struct _rtw_dm *pDM_Odm);
void odm_RSSIMonitorCheckCE(struct _rtw_dm *pDM_Odm);
void odm_RSSIMonitorCheck(struct _rtw_dm *pDM_Odm);

void odm_SwAntDivChkAntSwitchCallback(void *FunctionContext);

void odm_RefreshRateAdaptiveMask(struct _rtw_dm *pDM_Odm);
void ODM_TXPowerTrackingCheck(struct _rtw_dm *pDM_Odm);
void odm_RateAdaptiveMaskInit(struct _rtw_dm *pDM_Odm);
void odm_TXPowerTrackingThermalMeterInit(struct _rtw_dm *pDM_Odm);
void odm_TXPowerTrackingInit(struct _rtw_dm *pDM_Odm);
void odm_TXPowerTrackingCheckCE(struct _rtw_dm *pDM_Odm);
void odm_EdcaTurboCheck(struct _rtw_dm *pDM_Odm);
void ODM_EdcaTurboInit(struct _rtw_dm *pDM_Odm);

void odm_EdcaTurboCheckCE(struct _rtw_dm *pDM_Odm);



#define	RxDefaultAnt1		0x65a9
#define	RxDefaultAnt2		0x569a

/*
 * ============================================================
 * Export Interface
 * ============================================================
 */


/*
 * 2011/09/20 MH This is the entry pointer for all team to execute HW out source DM.
 * You can not add any dummy function here, be care, you can only use DM structure
 * to perform any new ODM_DM.
 */
void ODM_DMWatchdog(struct _rtw_dm *pDM_Odm)
{
	odm_CommonInfoSelfUpdate(pDM_Odm);
	odm_FalseAlarmCounterStatistics(pDM_Odm);
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_DIG, ODM_DBG_LOUD, ("odm_DIG(): RSSI=0x%x\n", pDM_Odm->RSSI_Min));

	odm_RSSIMonitorCheck(pDM_Odm);

	odm_DIG(pDM_Odm);

	pDIG_T	pDM_DigTable = &pDM_Odm->DM_DigTable;
	odm_Adaptivity(pDM_Odm, pDM_DigTable->CurIGValue);

	odm_CCKPacketDetectionThresh(pDM_Odm);

	if (*(pDM_Odm->pbPowerSaving) == TRUE)
		return;

	odm_RefreshRateAdaptiveMask(pDM_Odm);
	odm_EdcaTurboCheck(pDM_Odm);

	if (pDM_Odm->SupportICType & (ODM_RTL8812|ODM_RTL8821)) {
		/* if (pDM_Odm->SupportICType & ODM_RTL8812) */
		ODM_TXPowerTrackingCheck(pDM_Odm);
	}
	if (pDM_Odm->SupportICType & ODM_RTL8821) {
		if (pDM_Odm->bLinked) {
			if ((*pDM_Odm->pChannel != pDM_Odm->preChannel) && (!*pDM_Odm->pbScanInProcess)) {
				pDM_Odm->preChannel = *pDM_Odm->pChannel;
				pDM_Odm->LinkedInterval = 0;
			}

			if (pDM_Odm->LinkedInterval < 3)
				pDM_Odm->LinkedInterval++;

			if (pDM_Odm->LinkedInterval == 2) {
				struct rtl_priv *	rtlpriv = pDM_Odm->Adapter;

				/*
				 * mark out IQK flow to prevent tx stuck. by Maddest 20130306
				 * void rtl8821au_phy_iq_calibrate(rtlpriv, FALSE);
				 */
			}
		} else
			pDM_Odm->LinkedInterval = 0;
	}
	pDM_Odm->PhyDbgInfo.NumQryBeaconPkt = 0;

	odm_dtc(pDM_Odm);
}


/*
 * Init /.. Fixed HW value. Only init time.
 */

void ODM_CmnInfoInit(struct _rtw_dm *pDM_Odm, ODM_CMNINFO_E	CmnInfo, uint32_t Value)
{
	struct rtl_hal *rtlhal = rtl_hal(pDM_Odm->Adapter);
	
	/* ODM_RT_TRACE(pDM_Odm,); */

	/*
	 * This section is used for init value
	 */
	switch (CmnInfo) {
	/*
	 * Fixed ODM value.
	 */
	case	ODM_CMNINFO_ABILITY:
		pDM_Odm->SupportAbility = (uint32_t)Value;
		break;

	case	ODM_CMNINFO_INTERFACE:
		pDM_Odm->SupportInterface = (u8)Value;
		break;

	case	ODM_CMNINFO_MP_TEST_CHIP:
		pDM_Odm->bIsMPChip = (u8)Value;
		break;

	case	ODM_CMNINFO_IC_TYPE:
		pDM_Odm->SupportICType = Value;
		break;

	case	ODM_CMNINFO_CUT_VER:
		pDM_Odm->CutVersion = (u8)Value;
		break;

	case	ODM_CMNINFO_FAB_VER:
		pDM_Odm->FabVersion = (u8)Value;
		break;

	case    ODM_CMNINFO_RF_ANTENNA_TYPE:
		pDM_Odm->AntDivType = (u8)Value;
		break;

	case	ODM_CMNINFO_BOARD_TYPE:
		rtlhal->board_type = (u8)Value;
		break;

	case	ODM_CMNINFO_EXT_TRSW:
		pDM_Odm->ExtTRSW = (u8)Value;
		break;
	case 	ODM_CMNINFO_PATCH_ID:
		pDM_Odm->PatchID = (u8)Value;
		break;
	case 	ODM_CMNINFO_BINHCT_TEST:
		pDM_Odm->bInHctTest = (BOOLEAN)Value;
		break;
	case 	ODM_CMNINFO_BWIFI_TEST:
		pDM_Odm->bWIFITest = (BOOLEAN)Value;
		break;

	case	ODM_CMNINFO_SMART_CONCURRENT:
		pDM_Odm->bDualMacSmartConcurrent = (BOOLEAN) Value;
		break;

	/* To remove the compiler warning, must add an empty default statement to handle the other values. */
	default:
		/* do nothing */
		break;
	}
}


void ODM_CmnInfoHook(struct _rtw_dm *pDM_Odm, ODM_CMNINFO_E	CmnInfo, PVOID pValue)
{
	/*
	 * Hook call by reference pointer.
	 */
	switch (CmnInfo) {
	/*
	 * Dynamic call by reference pointer.
	 */
	case	ODM_CMNINFO_MAC_PHY_MODE:
		pDM_Odm->pMacPhyMode = (u8 *)pValue;
		break;

	case	ODM_CMNINFO_WM_MODE:
		pDM_Odm->pWirelessMode = (u8 *)pValue;
		break;

	case	ODM_CMNINFO_BAND:
		pDM_Odm->pBandType = (u8 *)pValue;
		break;

	case	ODM_CMNINFO_SEC_CHNL_OFFSET:
		pDM_Odm->pSecChOffset = (u8 *)pValue;
		break;

	case	ODM_CMNINFO_SEC_MODE:
		pDM_Odm->pSecurity = (u8 *)pValue;
		break;

	case	ODM_CMNINFO_CHNL:
		pDM_Odm->pChannel = (u8 *)pValue;
		break;

	case	ODM_CMNINFO_DMSP_GET_VALUE:
		pDM_Odm->pbGetValueFromOtherMac = (BOOLEAN *)pValue;
		break;

	case	ODM_CMNINFO_DMSP_IS_MASTER:
		pDM_Odm->pbMasterOfDMSP = (BOOLEAN *)pValue;
		break;

	case	ODM_CMNINFO_SCAN:
		pDM_Odm->pbScanInProcess = (BOOLEAN *)pValue;
		break;

	case	ODM_CMNINFO_POWER_SAVING:
		pDM_Odm->pbPowerSaving = (BOOLEAN *)pValue;
		break;

	case	ODM_CMNINFO_ONE_PATH_CCA:
		pDM_Odm->pOnePathCCA = (u8 *)pValue;
		break;

	case	ODM_CMNINFO_DRV_STOP:
		pDM_Odm->pbDriverStopped =  (BOOLEAN *)pValue;
		break;

	case	ODM_CMNINFO_PNP_IN:
		pDM_Odm->pbDriverIsGoingToPnpSetPowerSleep =  (BOOLEAN *)pValue;
		break;

	case	ODM_CMNINFO_INIT_ON:
		pDM_Odm->pinit_adpt_in_progress =  (BOOLEAN *)pValue;
		break;

	case	ODM_CMNINFO_ANT_TEST:
		pDM_Odm->pAntennaTest =  (u8 *)pValue;
		break;

	case	ODM_CMNINFO_NET_CLOSED:
		pDM_Odm->pbNet_closed = (BOOLEAN *)pValue;
		break;

	case 	ODM_CMNINFO_FORCED_RATE:
		pDM_Odm->pForcedDataRate = (u16 *)pValue;
		break;

	case	ODM_CMNINFO_MP_MODE:
		pDM_Odm->mp_mode = (u8 *)pValue;
		break;
	/* To remove the compiler warning, must add an empty default statement to handle the other values. */
	default:
		/* do nothing */
		break;
		break;

	}

}


void ODM_CmnInfoPtrArrayHook(struct _rtw_dm *pDM_Odm, ODM_CMNINFO_E CmnInfo,
	u16 Index, PVOID pValue)
{
	/*
	 * Hook call by reference pointer.
	 */

	switch (CmnInfo) {
		/*
		 * Dynamic call by reference pointer.
		 */
	case	ODM_CMNINFO_STA_STATUS:
		pDM_Odm->pODM_StaInfo[Index] = (struct sta_info *)pValue;
		break;
	/* To remove the compiler warning, must add an empty default statement to handle the other values. */
	default:
		/* do nothing */
		break;
	}

}


/*
 * Update Band/CHannel/.. The values are dynamic but non-per-packet.
 */
void ODM_CmnInfoUpdate(struct _rtw_dm *pDM_Odm, uint32_t CmnInfo, uint64_t Value)
{
	/*
	 * This init variable may be changed in run time.
	 */
	switch	(CmnInfo) {
	case	ODM_CMNINFO_ABILITY:
		pDM_Odm->SupportAbility = (uint32_t)Value;
		break;

	case	ODM_CMNINFO_WIFI_DIRECT:
		pDM_Odm->bWIFI_Direct = (BOOLEAN)Value;
		break;

	case	ODM_CMNINFO_WIFI_DISPLAY:
		pDM_Odm->bWIFI_Display = (BOOLEAN)Value;
		break;

	case	ODM_CMNINFO_LINK:
		pDM_Odm->bLinked = (BOOLEAN)Value;
		break;

	case	ODM_CMNINFO_RSSI_MIN:
		pDM_Odm->RSSI_Min = (u8)Value;
		break;

	case	ODM_CMNINFO_DBG_COMP:
		pDM_Odm->DebugComponents = Value;
		break;

	case	ODM_CMNINFO_DBG_LEVEL:
		pDM_Odm->DebugLevel = (uint32_t)Value;
		break;
	case	ODM_CMNINFO_RA_THRESHOLD_HIGH:
		pDM_Odm->RateAdaptive.HighRSSIThresh = (u8)Value;
		break;

	case	ODM_CMNINFO_RA_THRESHOLD_LOW:
		pDM_Odm->RateAdaptive.LowRSSIThresh = (u8)Value;
		break;

	default:
		/* do nothing */
		break;
	}


}


void odm_CommonInfoSelfUpdate(struct _rtw_dm * pDM_Odm)
{
	struct rtl_priv *rtlpriv = pDM_Odm->Adapter;
	u8	EntryCnt = 0;
	u8	i;
	struct sta_info *pEntry;

	if (rtlpriv->phy.current_chan_bw == ODM_BW40M) {
		if (*(pDM_Odm->pSecChOffset) == 1)
			pDM_Odm->ControlChannel = *(pDM_Odm->pChannel)-2;
		else if (*(pDM_Odm->pSecChOffset) == 2)
			pDM_Odm->ControlChannel = *(pDM_Odm->pChannel)+2;
	} else
		pDM_Odm->ControlChannel = *(pDM_Odm->pChannel);

	for (i = 0; i < ODM_ASSOCIATE_ENTRY_NUM; i++) {
		pEntry = pDM_Odm->pODM_StaInfo[i];
		if (IS_STA_VALID(pEntry))
			EntryCnt++;
	}

	if (EntryCnt == 1)
		pDM_Odm->bOneEntryOnly = TRUE;
	else
		pDM_Odm->bOneEntryOnly = FALSE;
}




/*
 * 3============================================================
 * 3 DIG
 * 3============================================================
 */

/*-----------------------------------------------------------------------------
 * Function:	odm_DIGInit()
 *
 * Overview:	Set DIG scheme init value.
 *
 * Input:		NONE
 *
 * Output:		NONE
 *
 * Return:		NONE
 *
 * Revised History:
 *	When		Who		Remark
 *
 *---------------------------------------------------------------------------*/
void ODM_ChangeDynamicInitGainThresh(struct _rtw_dm *pDM_Odm, uint32_t DM_Type, uint32_t DM_Value)
{
	pDIG_T	pDM_DigTable = &pDM_Odm->DM_DigTable;

	/* ULLI better switch/case ?? */
	if (DM_Type == DIG_TYPE_THRESH_HIGH) {
		pDM_DigTable->RssiHighThresh = DM_Value;
	} else if (DM_Type == DIG_TYPE_THRESH_LOW) {
		pDM_DigTable->RssiLowThresh = DM_Value;
	} else if (DM_Type == DIG_TYPE_ENABLE) {
		pDM_DigTable->Dig_Enable_Flag	= TRUE;
	} else if (DM_Type == DIG_TYPE_DISABLE) {
		pDM_DigTable->Dig_Enable_Flag = FALSE;
	} else if (DM_Type == DIG_TYPE_BACKOFF) {
		if (DM_Value > 30)
			DM_Value = 30;
		pDM_DigTable->BackoffVal = (u8)DM_Value;
	} else if (DM_Type == DIG_TYPE_RX_GAIN_MIN) {
		if (DM_Value == 0)
			DM_Value = 0x1;
		pDM_DigTable->rx_gain_range_min = (u8)DM_Value;
	} else if (DM_Type == DIG_TYPE_RX_GAIN_MAX) {
		if (DM_Value > 0x50)
			DM_Value = 0x50;
		pDM_DigTable->rx_gain_range_max = (u8)DM_Value;
	}
}	/* DM_ChangeDynamicInitGainThresh */

int getIGIForDiff(int value_IGI)
{
	#define ONERCCA_LOW_TH		0x30
	#define ONERCCA_LOW_DIFF	8

	if (value_IGI < ONERCCA_LOW_TH) {
		if ((ONERCCA_LOW_TH - value_IGI) < ONERCCA_LOW_DIFF)
			return ONERCCA_LOW_TH;
		else
			return value_IGI + ONERCCA_LOW_DIFF;
	} else {
		return value_IGI;
	}
}




void odm_Adaptivity(struct _rtw_dm *pDM_Odm, u8	IGI)
{
	struct rtl_priv *rtlpriv = pDM_Odm->Adapter;

	int32_t TH_H_dmc, TH_L_dmc;
	int32_t TH_H, TH_L, Diff, IGI_target;
	uint32_t value32;
	BOOLEAN EDCCA_State;

	if (!(pDM_Odm->SupportAbility & ODM_BB_ADAPTIVITY)) {
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_DIG, ODM_DBG_LOUD, ("Go to odm_DynamicEDCCA() \n"));
		/*  Add by Neil Chen to enable edcca to MP Platform */
		return;
	}
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_DIG, ODM_DBG_LOUD, ("odm_Adaptivity() =====> \n"));

	if (pDM_Odm->bForceThresh) {
		pDM_Odm->TH_H = pDM_Odm->Force_TH_H;
		pDM_Odm->TH_L = pDM_Odm->Force_TH_L;
	} else {
		if (*pDM_Odm->pBandType == BAND_ON_5G) {
			pDM_Odm->TH_H = 0xf4;	/* 0xf8; */
			pDM_Odm->TH_L = 0xf7;	/* 0xfb; */
		} else {
			pDM_Odm->TH_H = 0xf4;	/* 0xfa; */
			pDM_Odm->TH_L = 0xf7;	/* 0xfd; */
		}
	}

	ODM_RT_TRACE(pDM_Odm, ODM_COMP_DIG, ODM_DBG_LOUD, ("pDM_Odm->ForceEDCCA=%d, IGI_Base=0x%x, TH_H=0x%x, TH_L=0x%x, AdapEn_RSSI = %d\n",
	pDM_Odm->ForceEDCCA, pDM_Odm->IGI_Base, pDM_Odm->TH_H, pDM_Odm->TH_L, pDM_Odm->AdapEn_RSSI));

	rtl_set_bbreg(pDM_Odm->Adapter, 0x800, BIT10, 0);		/* ADC_mask enable */

	if (!pDM_Odm->bLinked) {
		return;
	}

	if (!pDM_Odm->ForceEDCCA) {
		if (pDM_Odm->RSSI_Min > pDM_Odm->AdapEn_RSSI)
			EDCCA_State = 1;
		else if (pDM_Odm->RSSI_Min < (pDM_Odm->AdapEn_RSSI - 5))
			EDCCA_State = 0;
	} else
		EDCCA_State = 1;
	/*
	 * if ((pDM_Odm->SupportICType & ODM_IC_11AC_SERIES) && (*pDM_Odm->pBandType == BAND_ON_5G))
	 * 	IGI_target = pDM_Odm->IGI_Base;
	 * else
	 */

	if (rtlpriv->phy.current_chan_bw == ODM_BW20M)	/*CHANNEL_WIDTH_20 */
		IGI_target = pDM_Odm->IGI_Base;
	else if (rtlpriv->phy.current_chan_bw == ODM_BW40M)
		IGI_target = pDM_Odm->IGI_Base + 2;
	else if (rtlpriv->phy.current_chan_bw == ODM_BW80M)
		IGI_target = pDM_Odm->IGI_Base + 6;
	else
		IGI_target = pDM_Odm->IGI_Base;


	pDM_Odm->IGI_target = IGI_target;

	if (pDM_Odm->TH_H & BIT7)
		TH_H = pDM_Odm->TH_H | 0xFFFFFF00;
	else
		TH_H = pDM_Odm->TH_H;
	if (pDM_Odm->TH_L & BIT7)
		TH_L = pDM_Odm->TH_L | 0xFFFFFF00;
	else
		TH_L = pDM_Odm->TH_L;

	ODM_RT_TRACE(pDM_Odm, ODM_COMP_DIG, ODM_DBG_LOUD, ("BandWidth=%s, IGI_target=0x%x, EDCCA_State=%d\n",
		(rtlpriv->phy.current_chan_bw == ODM_BW80M) ? "80M" : ((rtlpriv->phy.current_chan_bw == ODM_BW40M) ? "40M" : "20M"), IGI_target, EDCCA_State));

	if (EDCCA_State == 1) {
		if (IGI < IGI_target) {
			Diff = IGI_target - (int32_t)IGI;
			TH_H_dmc = TH_H + Diff;
			if (TH_H_dmc > 10)
				TH_H_dmc = 10;
			TH_L_dmc = TH_L + Diff;
			if (TH_L_dmc > 10)
				TH_L_dmc = 10;
		} else 	{
			Diff = (int32_t)IGI - IGI_target;
			TH_H_dmc = TH_H - Diff;
			TH_L_dmc = TH_L - Diff;
		}
		TH_H_dmc = (TH_H_dmc & 0xFF);
		TH_L_dmc = (TH_L_dmc & 0xFF);
	} else {
		TH_H_dmc = 0x7f;
		TH_L_dmc = 0x7f;
	}
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_DIG, ODM_DBG_LOUD, ("IGI=0x%x, TH_H_dmc=0x%x, TH_L_dmc=0x%x\n",
		IGI, TH_H_dmc, TH_L_dmc));

	rtl_set_bbreg(pDM_Odm->Adapter, rFPGA0_XB_LSSIReadBack, 0xFFFF, (TH_H_dmc<<8) | TH_L_dmc);
}


void ODM_Write_DIG(struct _rtw_dm *pDM_Odm, u8 CurrentIGI)
{
	pDIG_T	pDM_DigTable = &pDM_Odm->DM_DigTable;

	if (pDM_Odm->StopDIG) {
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_DIG, ODM_DBG_LOUD, ("Stop Writing IGI\n"));
		return;
	}

	ODM_RT_TRACE(pDM_Odm, ODM_COMP_DIG, ODM_DBG_LOUD, ("ODM_REG(IGI_A,pDM_Odm)=0x%x, ODM_BIT(IGI,pDM_Odm)=0x%x \n",
		ODM_REG(IGI_A, pDM_Odm), ODM_BIT(IGI, pDM_Odm)));

	if (pDM_DigTable->CurIGValue != CurrentIGI) {	/*if (pDM_DigTable->PreIGValue != CurrentIGI) */
		rtl_set_bbreg(pDM_Odm->Adapter, ODM_REG(IGI_A, pDM_Odm), ODM_BIT(IGI, pDM_Odm), CurrentIGI);
		if (pDM_Odm->Adapter->phy.rf_type != ODM_1T1R)
			rtl_set_bbreg(pDM_Odm->Adapter, ODM_REG(IGI_B, pDM_Odm), ODM_BIT(IGI, pDM_Odm), CurrentIGI);

		ODM_RT_TRACE(pDM_Odm, ODM_COMP_DIG, ODM_DBG_LOUD, ("CurrentIGI(0x%02x). \n", CurrentIGI));
		/* pDM_DigTable->PreIGValue = pDM_DigTable->CurIGValue; */
		pDM_DigTable->CurIGValue = CurrentIGI;
	}
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_DIG, ODM_DBG_LOUD, ("ODM_Write_DIG():CurrentIGI=0x%x \n", CurrentIGI));

}

void odm_DIGbyRSSI_LPS(struct _rtw_dm *pDM_Odm)
{
	/* struct rtl_priv *rtlpriv =pDM_Odm->Adapter; */
	/* pDIG_T	pDM_DigTable = &pDM_Odm->DM_DigTable; */
	PFALSE_ALARM_STATISTICS		pFalseAlmCnt = &pDM_Odm->FalseAlmCnt;

	u8	RSSI_Lower = DM_DIG_MIN_NIC;   	/* 0x1E or 0x1C */
	u8	CurrentIGI = pDM_Odm->RSSI_Min;

	CurrentIGI = CurrentIGI+RSSI_OFFSET_DIG;

	/* ODM_RT_TRACE(pDM_Odm,ODM_COMP_DIG_LPS, ODM_DBG_LOUD, ("odm_DIG()==>\n")); */

	/* Using FW PS mode to make IGI */

	ODM_RT_TRACE(pDM_Odm, ODM_COMP_DIG, ODM_DBG_LOUD, ("---Neil---odm_DIG is in LPS mode\n"));
	/* Adjust by  FA in LPS MODE */
	if (pFalseAlmCnt->Cnt_all > DM_DIG_FA_TH2_LPS)
		CurrentIGI = CurrentIGI+2;
	else if (pFalseAlmCnt->Cnt_all > DM_DIG_FA_TH1_LPS)
		CurrentIGI = CurrentIGI+1;
	else if (pFalseAlmCnt->Cnt_all < DM_DIG_FA_TH0_LPS)
		CurrentIGI = CurrentIGI-1;

	/* Lower bound checking */

	/* RSSI Lower bound check */
	if ((pDM_Odm->RSSI_Min-10) > DM_DIG_MIN_NIC)
		RSSI_Lower = (pDM_Odm->RSSI_Min-10);
	else
		RSSI_Lower = DM_DIG_MIN_NIC;

	/* Upper and Lower Bound checking */
	 if (CurrentIGI > DM_DIG_MAX_NIC)
		CurrentIGI = DM_DIG_MAX_NIC;
	 else if (CurrentIGI < RSSI_Lower)
		CurrentIGI = RSSI_Lower;

	ODM_Write_DIG(pDM_Odm, CurrentIGI);/* ODM_Write_DIG(pDM_Odm, pDM_DigTable->CurIGValue); */

}


void odm_DIG(struct _rtw_dm *pDM_Odm)
{
	pDIG_T						pDM_DigTable = &pDM_Odm->DM_DigTable;
	PFALSE_ALARM_STATISTICS		pFalseAlmCnt = &pDM_Odm->FalseAlmCnt;
	pRXHP_T						pRX_HP_Table  = &pDM_Odm->DM_RXHP_Table;
	u8						DIG_Dynamic_MIN;
	u8						DIG_MaxOfMin;
	BOOLEAN						FirstConnect, FirstDisConnect;
	u8						dm_dig_max, dm_dig_min, offset;
	u8						CurrentIGI = pDM_DigTable->CurIGValue;

#ifdef CONFIG_SPECIAL_SETTING_FOR_FUNAI_TV
	if ((pDM_Odm->bLinked) && (pDM_Odm->Adapter->registrypriv.force_igi != 0)) {
		printk("pDM_Odm->RSSI_Min=%d \n", pDM_Odm->RSSI_Min);
		ODM_Write_DIG(pDM_Odm, pDM_Odm->Adapter->registrypriv.force_igi);
		return;
	}
#endif

	ODM_RT_TRACE(pDM_Odm, ODM_COMP_DIG, ODM_DBG_LOUD, ("odm_DIG()==>\n"));
	/* if (!(pDM_Odm->SupportAbility & (ODM_BB_DIG|ODM_BB_FA_CNT))) */
	if ((!(pDM_Odm->SupportAbility&ODM_BB_DIG)) || (!(pDM_Odm->SupportAbility&ODM_BB_FA_CNT))) {
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_DIG, ODM_DBG_LOUD, ("odm_DIG() Return: SupportAbility ODM_BB_DIG or ODM_BB_FA_CNT is disabled\n"));
		return;
	}

	if (*(pDM_Odm->pbScanInProcess)) {
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_DIG, ODM_DBG_LOUD, ("odm_DIG() Return: In Scan Progress \n"));
		return;
	}

	/* add by Neil Chen to avoid PSD is processing */
	DIG_Dynamic_MIN = pDM_DigTable->DIG_Dynamic_MIN_0;
	FirstConnect = (pDM_Odm->bLinked) && (pDM_DigTable->bMediaConnect_0 == FALSE);
	FirstDisConnect = (!pDM_Odm->bLinked) && (pDM_DigTable->bMediaConnect_0 == TRUE);


	/* 1 Boundary Decision */
	dm_dig_max = DM_DIG_MAX_NIC;

	if (pDM_Odm->SupportICType != ODM_RTL8821)
		dm_dig_min = DM_DIG_MIN_NIC;
	else
		dm_dig_min = 0x1C;

	DIG_MaxOfMin = DM_DIG_MAX_AP;

	if (pDM_Odm->bLinked) {
		{
			/* 2 Modify DIG upper bound */
			/* 2013.03.19 Luke: Modified upper bound for Netgear rental house test */
			if (pDM_Odm->SupportICType != ODM_RTL8821)
				offset = 20;
			else
				offset = 10;

			if ((pDM_Odm->RSSI_Min + offset) > dm_dig_max)
				pDM_DigTable->rx_gain_range_max = dm_dig_max;
			else if ((pDM_Odm->RSSI_Min + offset) < dm_dig_min)
				pDM_DigTable->rx_gain_range_max = dm_dig_min;
			else
				pDM_DigTable->rx_gain_range_max = pDM_Odm->RSSI_Min + offset;


			/* 2 Modify DIG lower bound */
			/*
			if ((pFalseAlmCnt->Cnt_all > 500)&&(DIG_Dynamic_MIN < 0x25))
				DIG_Dynamic_MIN++;
			else if (((pFalseAlmCnt->Cnt_all < 500)||(pDM_Odm->RSSI_Min < 8))&&(DIG_Dynamic_MIN > dm_dig_min))
				DIG_Dynamic_MIN--;
			*/
			if (pDM_Odm->bOneEntryOnly) {
				if (pDM_Odm->RSSI_Min < dm_dig_min)
					DIG_Dynamic_MIN = dm_dig_min;
				else if (pDM_Odm->RSSI_Min > DIG_MaxOfMin)
					DIG_Dynamic_MIN = DIG_MaxOfMin;
				else
					DIG_Dynamic_MIN = pDM_Odm->RSSI_Min;
				ODM_RT_TRACE(pDM_Odm, ODM_COMP_DIG, ODM_DBG_LOUD, ("odm_DIG() : bOneEntryOnly=TRUE,  DIG_Dynamic_MIN=0x%x\n", DIG_Dynamic_MIN));
				ODM_RT_TRACE(pDM_Odm, ODM_COMP_DIG, ODM_DBG_LOUD, ("odm_DIG() : pDM_Odm->RSSI_Min=%d\n", pDM_Odm->RSSI_Min));
			} else {
				/* 1 Lower Bound for 88E AntDiv */
				DIG_Dynamic_MIN = dm_dig_min;
			}
		}
	} else {
		pDM_DigTable->rx_gain_range_max = dm_dig_max;
		DIG_Dynamic_MIN = dm_dig_min;
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_DIG, ODM_DBG_LOUD, ("odm_DIG() : No Link\n"));
	}

	/* 1 Modify DIG lower bound, deal with abnorally large false alarm */
	if (pFalseAlmCnt->Cnt_all > 10000) {
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_DIG, ODM_DBG_LOUD, ("dm_DIG(): Abnornally false alarm case. \n"));

		if (pDM_DigTable->LargeFAHit != 3)
			pDM_DigTable->LargeFAHit++;
		if (pDM_DigTable->ForbiddenIGI < CurrentIGI) {			/* if (pDM_DigTable->ForbiddenIGI < pDM_DigTable->CurIGValue) */
			pDM_DigTable->ForbiddenIGI = (u8)CurrentIGI;	/* pDM_DigTable->ForbiddenIGI = pDM_DigTable->CurIGValue; */
			pDM_DigTable->LargeFAHit = 1;
		}

		if (pDM_DigTable->LargeFAHit >= 3) {
			if ((pDM_DigTable->ForbiddenIGI+1) > pDM_DigTable->rx_gain_range_max)
				pDM_DigTable->rx_gain_range_min = pDM_DigTable->rx_gain_range_max;
			else
				pDM_DigTable->rx_gain_range_min = (pDM_DigTable->ForbiddenIGI + 1);
			pDM_DigTable->Recover_cnt = 3600; 	/* 3600=2hr */
		}

	} else {
		/* Recovery mechanism for IGI lower bound */
		if (pDM_DigTable->Recover_cnt != 0)
			pDM_DigTable->Recover_cnt--;
		else {
			if (pDM_DigTable->LargeFAHit < 3) {
				if ((pDM_DigTable->ForbiddenIGI-1) < DIG_Dynamic_MIN) {		/* DM_DIG_MIN)  */
					pDM_DigTable->ForbiddenIGI = DIG_Dynamic_MIN;		/* DM_DIG_MIN; */
					pDM_DigTable->rx_gain_range_min = DIG_Dynamic_MIN;	/* DM_DIG_MIN; */
					ODM_RT_TRACE(pDM_Odm, ODM_COMP_DIG, ODM_DBG_LOUD, ("odm_DIG(): Normal Case: At Lower Bound\n"));
				} else {
					pDM_DigTable->ForbiddenIGI--;
					pDM_DigTable->rx_gain_range_min = (pDM_DigTable->ForbiddenIGI + 1);
					ODM_RT_TRACE(pDM_Odm, ODM_COMP_DIG, ODM_DBG_LOUD, ("odm_DIG(): Normal Case: Approach Lower Bound\n"));
				}
			} else {
				pDM_DigTable->LargeFAHit = 0;
			}
		}
	}
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_DIG, ODM_DBG_LOUD, ("odm_DIG(): pDM_DigTable->LargeFAHit=%d\n", pDM_DigTable->LargeFAHit));

	if ((pDM_Odm->PhyDbgInfo.NumQryBeaconPkt < 10))
		pDM_DigTable->rx_gain_range_min = dm_dig_min;

	if (pDM_DigTable->rx_gain_range_min > pDM_DigTable->rx_gain_range_max)
		pDM_DigTable->rx_gain_range_min = pDM_DigTable->rx_gain_range_max;

	/* 1 Adjust initial gain by false alarm */
	if (pDM_Odm->bLinked) {
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_DIG, ODM_DBG_LOUD, ("odm_DIG(): DIG AfterLink\n"));
		if (FirstConnect) {
			if (pDM_Odm->RSSI_Min <= DIG_MaxOfMin)
			    CurrentIGI = pDM_Odm->RSSI_Min;
			else
			    CurrentIGI = DIG_MaxOfMin;
			ODM_RT_TRACE(pDM_Odm,	ODM_COMP_DIG, ODM_DBG_LOUD, ("DIG: First Connect\n"));
		} else 	{
			/* FA for Combo IC--NeilChen--2012--09--28 */
			if (pFalseAlmCnt->Cnt_all > DM_DIG_FA_TH2)
				CurrentIGI = CurrentIGI + 4;	/* pDM_DigTable->CurIGValue = pDM_DigTable->PreIGValue+2; */
			else if (pFalseAlmCnt->Cnt_all > DM_DIG_FA_TH1)
				CurrentIGI = CurrentIGI + 2;	/* pDM_DigTable->CurIGValue = pDM_DigTable->PreIGValue+1; */
			else if (pFalseAlmCnt->Cnt_all < DM_DIG_FA_TH0)
				CurrentIGI = CurrentIGI - 2;	/* pDM_DigTable->CurIGValue =pDM_DigTable->PreIGValue-1; */

			if ((pDM_Odm->PhyDbgInfo.NumQryBeaconPkt < 10)
			 && (pFalseAlmCnt->Cnt_all < DM_DIG_FA_TH1))
				CurrentIGI = pDM_DigTable->rx_gain_range_min;
		}
	} else {
		/* CurrentIGI = pDM_DigTable->rx_gain_range_min; */	/* pDM_DigTable->CurIGValue = pDM_DigTable->rx_gain_range_min */
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_DIG, ODM_DBG_LOUD, ("odm_DIG(): DIG BeforeLink\n"));
		if (FirstDisConnect) {
			CurrentIGI = pDM_DigTable->rx_gain_range_min;
			ODM_RT_TRACE(pDM_Odm, ODM_COMP_DIG, ODM_DBG_LOUD, ("odm_DIG(): First DisConnect \n"));
		} else {
			/* 2012.03.30 LukeLee: enable DIG before link but with very high thresholds */
			if (pFalseAlmCnt->Cnt_all > 10000)
				CurrentIGI = CurrentIGI + 4;
			else if (pFalseAlmCnt->Cnt_all > 8000)
				CurrentIGI = CurrentIGI + 2;
			else if (pFalseAlmCnt->Cnt_all < 500)
				CurrentIGI = CurrentIGI - 2;
			ODM_RT_TRACE(pDM_Odm, ODM_COMP_DIG, ODM_DBG_LOUD, ("odm_DIG(): England DIG \n"));
		}
	}
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_DIG, ODM_DBG_LOUD, ("odm_DIG(): DIG End Adjust IGI\n"));
	/* 1 Check initial gain by upper/lower bound */

	if (CurrentIGI > pDM_DigTable->rx_gain_range_max)
		CurrentIGI = pDM_DigTable->rx_gain_range_max;
	if (CurrentIGI < pDM_DigTable->rx_gain_range_min)
		CurrentIGI = pDM_DigTable->rx_gain_range_min;

	if (pDM_Odm->SupportAbility & ODM_BB_ADAPTIVITY) {
		if (CurrentIGI > (pDM_Odm->IGI_target + 4))
			CurrentIGI = (u8)pDM_Odm->IGI_target + 4;
	}

	ODM_RT_TRACE(pDM_Odm, ODM_COMP_DIG, ODM_DBG_LOUD, ("odm_DIG(): rx_gain_range_max=0x%x, rx_gain_range_min=0x%x\n",
		pDM_DigTable->rx_gain_range_max, pDM_DigTable->rx_gain_range_min));
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_DIG, ODM_DBG_LOUD, ("odm_DIG(): TotalFA=%d\n", pFalseAlmCnt->Cnt_all));
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_DIG, ODM_DBG_LOUD, ("odm_DIG(): CurIGValue=0x%x\n", CurrentIGI));

	/* 2 High power RSSI threshold */

	{		/* BT is not using */
		ODM_Write_DIG(pDM_Odm, CurrentIGI);	/* ODM_Write_DIG(pDM_Odm, pDM_DigTable->CurIGValue); */
		pDM_DigTable->bMediaConnect_0 = pDM_Odm->bLinked;
		pDM_DigTable->DIG_Dynamic_MIN_0 = DIG_Dynamic_MIN;
	}
}

/*
 * 3============================================================
 * 3 FASLE ALARM CHECK
 * 3============================================================
 */

void odm_FalseAlarmCounterStatistics(struct _rtw_dm *pDM_Odm)
{
	uint32_t ret_value;
	PFALSE_ALARM_STATISTICS FalseAlmCnt = &(pDM_Odm->FalseAlmCnt);

	if (!(pDM_Odm->SupportAbility & ODM_BB_FA_CNT))
		return;

	{
		uint32_t CCKenable;
		/* read OFDM FA counter */
		FalseAlmCnt->Cnt_Ofdm_fail = rtl_get_bbreg(pDM_Odm->Adapter, ODM_REG_OFDM_FA_11AC, bMaskLWord);
		FalseAlmCnt->Cnt_Cck_fail = rtl_get_bbreg(pDM_Odm->Adapter, ODM_REG_CCK_FA_11AC, bMaskLWord);

		CCKenable =  rtl_get_bbreg(pDM_Odm->Adapter, ODM_REG_BB_RX_PATH_11AC, BIT28);
		if (CCKenable)		/* if (*pDM_Odm->pBandType == ODM_BAND_2_4G) */
			FalseAlmCnt->Cnt_all = FalseAlmCnt->Cnt_Ofdm_fail + FalseAlmCnt->Cnt_Cck_fail;
		else
			FalseAlmCnt->Cnt_all = FalseAlmCnt->Cnt_Ofdm_fail;

		/* reset OFDM FA coutner */
		rtl_set_bbreg(pDM_Odm->Adapter, ODM_REG_OFDM_FA_RST_11AC, BIT17, 1);
		rtl_set_bbreg(pDM_Odm->Adapter, ODM_REG_OFDM_FA_RST_11AC, BIT17, 0);
		/* reset CCK FA counter */
		rtl_set_bbreg(pDM_Odm->Adapter, ODM_REG_CCK_FA_RST_11AC, BIT15, 0);
		rtl_set_bbreg(pDM_Odm->Adapter, ODM_REG_CCK_FA_RST_11AC, BIT15, 1);
	}
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_FA_CNT, ODM_DBG_LOUD, ("Cnt_Cck_fail=%d\n", FalseAlmCnt->Cnt_Cck_fail));
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_FA_CNT, ODM_DBG_LOUD, ("Cnt_Ofdm_fail=%d\n", FalseAlmCnt->Cnt_Ofdm_fail));
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_FA_CNT, ODM_DBG_LOUD, ("Total False Alarm=%d\n", FalseAlmCnt->Cnt_all));
}

/*
 * 3============================================================
 * 3 CCK Packet Detect Threshold
 * 3============================================================
 */

void odm_CCKPacketDetectionThresh(struct _rtw_dm *pDM_Odm)
{
	struct rtl_hal *rtlhal = rtl_hal(pDM_Odm->Adapter);
	
	u8	CurCCK_CCAThres;
	PFALSE_ALARM_STATISTICS FalseAlmCnt = &(pDM_Odm->FalseAlmCnt);

	if (!(pDM_Odm->SupportAbility & (ODM_BB_CCK_PD|ODM_BB_FA_CNT)))
		return;

	if (rtlhal->external_lna_2g)
		return;

	if (pDM_Odm->bLinked) {
		if (pDM_Odm->RSSI_Min > 25)
			CurCCK_CCAThres = 0xcd;
		else if ((pDM_Odm->RSSI_Min <= 25) && (pDM_Odm->RSSI_Min > 10))
			CurCCK_CCAThres = 0x83;
		else {
			if (FalseAlmCnt->Cnt_Cck_fail > 1000)
				CurCCK_CCAThres = 0x83;
			else
				CurCCK_CCAThres = 0x40;
		}
	} else {
		if (FalseAlmCnt->Cnt_Cck_fail > 1000)
			CurCCK_CCAThres = 0x83;
		else
			CurCCK_CCAThres = 0x40;
	}

		ODM_Write_CCK_CCA_Thres(pDM_Odm, CurCCK_CCAThres);
}

void ODM_Write_CCK_CCA_Thres(struct _rtw_dm *pDM_Odm, u8	 CurCCK_CCAThres)
{
	pDIG_T	pDM_DigTable = &pDM_Odm->DM_DigTable;

	if (pDM_DigTable->CurCCK_CCAThres != CurCCK_CCAThres) {	/* modify by Guo.Mingzhi 2012-01-03 */
		rtl_write_byte(pDM_Odm->Adapter, ODM_REG(CCK_CCA, pDM_Odm), CurCCK_CCAThres);
	}
	pDM_DigTable->PreCCK_CCAThres = pDM_DigTable->CurCCK_CCAThres;
	pDM_DigTable->CurCCK_CCAThres = CurCCK_CCAThres;

}

void odm_1R_CCA(struct _rtw_dm *pDM_Odm)
{
	pPS_T	pDM_PSTable = &pDM_Odm->DM_PSTable;

	if (pDM_Odm->RSSI_Min != 0xFF) {
		if (pDM_PSTable->PreCCAState == CCA_2R) {
			if (pDM_Odm->RSSI_Min >= 35)
				pDM_PSTable->CurCCAState = CCA_1R;
			else
				pDM_PSTable->CurCCAState = CCA_2R;

		} else {
			if (pDM_Odm->RSSI_Min <= 30)
				pDM_PSTable->CurCCAState = CCA_2R;
			else
				pDM_PSTable->CurCCAState = CCA_1R;
		}
	} else {
		pDM_PSTable->CurCCAState = CCA_MAX;
	}

	if (pDM_PSTable->PreCCAState != pDM_PSTable->CurCCAState) {
		if (pDM_PSTable->CurCCAState == CCA_1R) {
			if (pDM_Odm->Adapter->phy.rf_type == ODM_2T2R) {
				rtl_set_bbreg(pDM_Odm->Adapter, 0xc04  , MASKBYTE0, 0x13);
				/* rtl_set_bbreg(rtlpriv, 0xe70, MASKBYTE3, 0x20); */
			} else {
				rtl_set_bbreg(pDM_Odm->Adapter, 0xc04  , MASKBYTE0, 0x23);
				/* rtl_set_bbreg(rtlpriv, 0xe70, 0x7fc00000, 0x10c); */ /* Set RegE70[30:22] = 9b'100001100 */
			}
		} else {
			rtl_set_bbreg(pDM_Odm->Adapter, 0xc04  , MASKBYTE0, 0x33);
			/* rtl_set_bbreg(rtlpriv,0xe70, MASKBYTE3, 0x63); */
		}
		pDM_PSTable->PreCCAState = pDM_PSTable->CurCCAState;
	}
	/* ODM_RT_TRACE(pDM_Odm,	COMP_BB_POWERSAVING, DBG_LOUD, ("CCAStage = %s\n",(pDM_PSTable->CurCCAState==0)?"1RCCA":"2RCCA")); */
}

/*
 * 3============================================================
 * 3 RATR MASK
 * 3============================================================
 * 3============================================================
 * 3 Rate Adaptive
 * 3============================================================
 */



uint32_t ODM_Get_Rate_Bitmap(struct _rtw_dm *pDM_Odm, uint32_t macid,
	uint32_t ra_mask, u8 rssi_level)
{
	struct rtl_priv *rtlpriv = pDM_Odm->Adapter;

	struct sta_info *pEntry;
	uint32_t 	rate_bitmap = 0;
	u8 	WirelessMode;
	/* u8 	WirelessMode =*(pDM_Odm->pWirelessMode); */

	pEntry = pDM_Odm->pODM_StaInfo[macid];
	if (!IS_STA_VALID(pEntry))
		return ra_mask;

	WirelessMode = pEntry->wireless_mode;

	switch (WirelessMode) {
	case ODM_WM_B:
		if (ra_mask & 0x0000000c)		/* 11M or 5.5M enable */
			rate_bitmap = 0x0000000d;
		else
			rate_bitmap = 0x0000000f;
		break;

	case (ODM_WM_G):
	case (ODM_WM_A):
		if (rssi_level == DM_RATR_STA_HIGH)
			rate_bitmap = 0x00000f00;
		else
			rate_bitmap = 0x00000ff0;
		break;

	case (ODM_WM_B|ODM_WM_G):
		if (rssi_level == DM_RATR_STA_HIGH)
			rate_bitmap = 0x00000f00;
		else if (rssi_level == DM_RATR_STA_MIDDLE)
			rate_bitmap = 0x00000ff0;
		else
			rate_bitmap = 0x00000ff5;
		break;

	case (ODM_WM_B|ODM_WM_G|ODM_WM_N24G):
	case (ODM_WM_B|ODM_WM_N24G):
	case (ODM_WM_A|ODM_WM_N5G):
		if (pDM_Odm->Adapter->phy.rf_type == ODM_1T2R || pDM_Odm->Adapter->phy.rf_type == ODM_1T1R) {
			if (rssi_level == DM_RATR_STA_HIGH) {
				rate_bitmap = 0x000f0000;
			} else if (rssi_level == DM_RATR_STA_MIDDLE) {
				rate_bitmap = 0x000ff000;
			} else {
				if (rtlpriv->phy.current_chan_bw == ODM_BW40M)
					rate_bitmap = 0x000ff015;
				else
					rate_bitmap = 0x000ff005;
			}
		} else {
			if (rssi_level == DM_RATR_STA_HIGH) {
				rate_bitmap = 0x0f8f0000;
			} else if (rssi_level == DM_RATR_STA_MIDDLE) {
				rate_bitmap = 0x0f8ff000;
			} else {
				if (rtlpriv->phy.current_chan_bw == ODM_BW40M)
					rate_bitmap = 0x0f8ff015;
				else
					rate_bitmap = 0x0f8ff005;
			}
		}
		break;
	case (ODM_WM_AC|ODM_WM_A):
	case (ODM_WM_AC|ODM_WM_G):
		if (pDM_Odm->Adapter->phy.rf_type == RF_1T1R) {
			if ((pDM_Odm->SupportICType == ODM_RTL8821) ||
				(pDM_Odm->SupportICType == ODM_RTL8812 && pDM_Odm->bIsMPChip)) {
				if ((pDM_Odm->SupportICType == ODM_RTL8821)
					&& (pDM_Odm->SupportInterface ==  ODM_ITRF_USB)
					&& (*(pDM_Odm->pChannel) >= 149)) {
					if (rssi_level == 1)				/* add by Gary for ac-series */
						rate_bitmap = 0x001f8000;
					else if (rssi_level == 2)
						rate_bitmap = 0x001ff000;
					else
						rate_bitmap = 0x001ff010;
				} else 	{
					if (rssi_level == 1)				/* add by Gary for ac-series */
						rate_bitmap = 0x003f8000;
					else if (rssi_level == 2)
						rate_bitmap = 0x003ff000;
					else
						rate_bitmap = 0x003ff010;
				}
			} else {
				rate_bitmap = 0x000ff010;
		       }
		} else {
			if (pDM_Odm->bIsMPChip) {
				if (rssi_level == 1)			/* add by Gary for ac-series */
					rate_bitmap = 0xfe3f8000;       /* VHT 2SS MCS3~9 */
				else if (rssi_level == 2)
					rate_bitmap = 0xfffff000;       /* VHT 2SS MCS0~9 */
				else
					rate_bitmap = 0xfffff010;       /* All */
			} else
				rate_bitmap = 0x3fcff010;
		}
		break;

	default:
		if (pDM_Odm->Adapter->phy.rf_type == RF_1T2R)
			rate_bitmap = 0x000fffff;
		else
			rate_bitmap = 0x0fffffff;
		break;

	}

	/* printk("%s ==> rssi_level:0x%02x, WirelessMode:0x%02x, rate_bitmap:0x%08x \n",__FUNCTION__,rssi_level,WirelessMode,rate_bitmap); */
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_RA_MASK, ODM_DBG_LOUD, (" ==> rssi_level:0x%02x, WirelessMode:0x%02x, rate_bitmap:0x%08x \n", rssi_level, WirelessMode, rate_bitmap));

	return (ra_mask&rate_bitmap);

}

/*-----------------------------------------------------------------------------
 * Function:	odm_RefreshRateAdaptiveMask()
 *
 * Overview:	Update rate table mask according to rssi
 *
 * Input:		NONE
 *
 * Output:		NONE
 *
 * Return:		NONE
 *
 * Revised History:
 *	When		Who		Remark
 *	05/27/2009	hpfan	Create Version 0.
 *
 *---------------------------------------------------------------------------*/
void odm_RefreshRateAdaptiveMask(struct _rtw_dm *pDM_Odm)
{

	ODM_RT_TRACE(pDM_Odm, ODM_COMP_RA_MASK, ODM_DBG_TRACE, ("odm_RefreshRateAdaptiveMask()---------->\n"));
	if (!(pDM_Odm->SupportAbility & ODM_BB_RA_MASK)) {
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_RA_MASK, ODM_DBG_TRACE, ("odm_RefreshRateAdaptiveMask(): Return cos not supported\n"));
		return;
	}
	/*
	 * 2011/09/29 MH In HW integration first stage, we provide 4 different handle to operate
	 * at the same time. In the stage2/3, we need to prive universal interface and merge all
	 * HW dynamic mechanism.
	 */

	odm_RefreshRateAdaptiveMaskCE(pDM_Odm);
}

void odm_RefreshRateAdaptiveMaskCE(struct _rtw_dm *pDM_Odm)
{
	u8	i;
	struct rtl_priv *rtlpriv	=  pDM_Odm->Adapter;
	PODM_RATE_ADAPTIVE	pRA = &pDM_Odm->RateAdaptive;

	if (rtlpriv->bDriverStopped) {
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_RA_MASK, ODM_DBG_TRACE, ("<---- odm_RefreshRateAdaptiveMask(): driver is going to unload\n"));
		return;
	}

	if (!pDM_Odm->bUseRAMask) {
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_RA_MASK, ODM_DBG_LOUD, ("<---- odm_RefreshRateAdaptiveMask(): driver does not control rate adaptive mask\n"));
		return;
	}

	/* printk("==> %s \n",__FUNCTION__); */

	for (i = 0; i < ODM_ASSOCIATE_ENTRY_NUM; i++) {
		struct sta_info *pstat = pDM_Odm->pODM_StaInfo[i];
		if (IS_STA_VALID(pstat)) {
			if ((pDM_Odm->SupportICType == ODM_RTL8812)
			 || (pDM_Odm->SupportICType == ODM_RTL8821)) {
				if (pstat->rssi_stat.UndecoratedSmoothedPWDB < pRA->LdpcThres) {
					pRA->bUseLdpc = TRUE;
					pRA->bLowerRtsRate = TRUE;
					Set_RA_LDPC_8812(pstat, TRUE);
					/* DbgPrint("RSSI=%d, bUseLdpc = TRUE\n", pHalData->UndecoratedSmoothedPWDB); */
				} else if (pstat->rssi_stat.UndecoratedSmoothedPWDB > (pRA->LdpcThres-5)) {
					pRA->bUseLdpc = FALSE;
					pRA->bLowerRtsRate = FALSE;
					Set_RA_LDPC_8812(pstat, FALSE);
					/* DbgPrint("RSSI=%d, bUseLdpc = FALSE\n", pHalData->UndecoratedSmoothedPWDB); */
				}
			}

			if (TRUE == ODM_RAStateCheck(pDM_Odm, pstat->rssi_stat.UndecoratedSmoothedPWDB, FALSE , &pstat->rssi_level)) {
				ODM_RT_TRACE(pDM_Odm, ODM_COMP_RA_MASK, ODM_DBG_LOUD, ("RSSI:%d, RSSI_LEVEL:%d\n", pstat->rssi_stat.UndecoratedSmoothedPWDB, pstat->rssi_level));
				/* printk("RSSI:%d, RSSI_LEVEL:%d\n", pstat->rssi_stat.UndecoratedSmoothedPWDB, pstat->rssi_level); */
				rtw_hal_update_ra_mask(pstat, pstat->rssi_level);
			}
		}
	}

}

/*
 * Return Value: BOOLEAN
 * - TRUE: RATRState is changed.
 */
BOOLEAN ODM_RAStateCheck(struct _rtw_dm *pDM_Odm, int32_t RSSI,
	BOOLEAN	 bForceUpdate, u8 *pRATRState)
{
	PODM_RATE_ADAPTIVE pRA = &pDM_Odm->RateAdaptive;
	const u8 GoUpGap = 5;
	u8 HighRSSIThreshForRA = pRA->HighRSSIThresh;
	u8 LowRSSIThreshForRA = pRA->LowRSSIThresh;
	u8 RATRState;

	/*
	 * Threshold Adjustment:
	 * when RSSI state trends to go up one or two levels, make sure RSSI is high enough.
	 * Here GoUpGap is added to solve the boundary's level alternation issue.
	 */
	switch (*pRATRState) {
	case DM_RATR_STA_INIT:
	case DM_RATR_STA_HIGH:
		break;
	case DM_RATR_STA_MIDDLE:
		HighRSSIThreshForRA += GoUpGap;
		break;
	case DM_RATR_STA_LOW:
		HighRSSIThreshForRA += GoUpGap;
		LowRSSIThreshForRA += GoUpGap;
		break;

	default:
		ODM_RT_ASSERT(pDM_Odm, FALSE, ("wrong rssi level setting %d !", *pRATRState));
		break;
	}

	/* Decide RATRState by RSSI. */
	if (RSSI > HighRSSIThreshForRA)
		RATRState = DM_RATR_STA_HIGH;
	else if (RSSI > LowRSSIThreshForRA)
		RATRState = DM_RATR_STA_MIDDLE;
	else
		RATRState = DM_RATR_STA_LOW;
	/* printk("==>%s,RATRState:0x%02x ,RSSI:%d \n",__FUNCTION__,RATRState,RSSI); */

	if (*pRATRState != RATRState || bForceUpdate) {
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_RA_MASK, ODM_DBG_LOUD, ("RSSI Level %d -> %d\n", *pRATRState, RATRState));
		*pRATRState = RATRState;
		return TRUE;
	}

	return FALSE;
}


/* ============================================================ */

/*
 * 3============================================================
 * 3 Dynamic Tx Power
 * 3============================================================
 */

/*
 * 3============================================================
 * 3 RSSI Monitor
 * 3============================================================
 */

void odm_RSSIMonitorCheck(struct _rtw_dm *pDM_Odm)
{
	/*
	 * For AP/ADSL use prtl8192cd_priv
	 * For CE/NIC use _ADAPTER
	 */

	if (!(pDM_Odm->SupportAbility & ODM_BB_RSSI_MONITOR))
		return;

	/*
	 * 2011/09/29 MH In HW integration first stage, we provide 4 different handle to operate
	 * at the same time. In the stage2/3, we need to prive universal interface and merge all
	 * HW dynamic mechanism.
	 */

	odm_RSSIMonitorCheckCE(pDM_Odm);
}

/*
 * sherry move from DUSC to here 20110517
 */
static VOID
FindMinimumRSSI_Dmsp(
	IN	struct rtl_priv *rtlpriv
)
{
}

static void FindMinimumRSSI(struct rtl_priv *rtlpriv)
{
	 struct _rtw_hal	*pHalData = GET_HAL_DATA(rtlpriv);
	struct dm_priv	*pdmpriv = &pHalData->dmpriv;
	struct _rtw_dm *	pDM_Odm = &(pHalData->odmpriv);

	/* 1 1.Determine the minimum RSSI */

	if ((pDM_Odm->bLinked != _TRUE) && (pdmpriv->EntryMinUndecoratedSmoothedPWDB == 0)) {
		pdmpriv->MinUndecoratedPWDBForDM = 0;
		/* ODM_RT_TRACE(pDM_Odm,COMP_BB_POWERSAVING, DBG_LOUD, ("Not connected to any \n")); */
	} else {
		pdmpriv->MinUndecoratedPWDBForDM = pdmpriv->EntryMinUndecoratedSmoothedPWDB;
	}

	/* DBG_8192C("%s=>MinUndecoratedPWDBForDM(%d)\n",__FUNCTION__,pdmpriv->MinUndecoratedPWDBForDM); */
	/* ODM_RT_TRACE(pDM_Odm,COMP_DIG, DBG_LOUD, ("MinUndecoratedPWDBForDM =%d\n",pHalData->MinUndecoratedPWDBForDM)); */
}

void odm_RSSIMonitorCheckCE(struct _rtw_dm *pDM_Odm)
{
	struct rtl_priv *Adapter = pDM_Odm->Adapter;
	 struct _rtw_hal	*pHalData = GET_HAL_DATA(Adapter);
	struct dm_priv	*pdmpriv = &pHalData->dmpriv;
	int	i;
	int	tmpEntryMaxPWDB = 0, tmpEntryMinPWDB = 0xff;
	u8 	sta_cnt = 0;
	u8	UL_DL_STATE = 0;			/*  for 8812 use */
	uint32_t PWDB_rssi[NUM_STA] = { 0 };		/* [0~15]:MACID, [16~31]:PWDB_rssi */

	if (pDM_Odm->bLinked != _TRUE)
		return;

	if ((pDM_Odm->SupportICType == ODM_RTL8812) || (pDM_Odm->SupportICType == ODM_RTL8821)) {
		u64	curTxOkCnt = Adapter->xmitpriv.tx_bytes - Adapter->xmitpriv.last_tx_bytes;
		u64	curRxOkCnt = Adapter->recvpriv.rx_bytes - Adapter->recvpriv.last_rx_bytes;

		if (curRxOkCnt > (curTxOkCnt*6))
			UL_DL_STATE = 1;
		else
			UL_DL_STATE = 0;
	}


	/* if (check_fwstate(&Adapter->mlmepriv, WIFI_AP_STATE|WIFI_ADHOC_STATE|WIFI_ADHOC_MASTER_STATE) == _TRUE) */
	{
#if 1
		struct sta_info *psta;

		for (i = 0; i < ODM_ASSOCIATE_ENTRY_NUM; i++) {
			psta = pDM_Odm->pODM_StaInfo[i];
			if (IS_STA_VALID(psta)) {
				if (psta->rssi_stat.UndecoratedSmoothedPWDB < tmpEntryMinPWDB)
					tmpEntryMinPWDB = psta->rssi_stat.UndecoratedSmoothedPWDB;

				if (psta->rssi_stat.UndecoratedSmoothedPWDB > tmpEntryMaxPWDB)
					tmpEntryMaxPWDB = psta->rssi_stat.UndecoratedSmoothedPWDB;

				if (psta->rssi_stat.UndecoratedSmoothedPWDB != (-1)) {
					if ((pDM_Odm->SupportICType == ODM_RTL8812) || (pDM_Odm->SupportICType == ODM_RTL8821))
						PWDB_rssi[sta_cnt++] = (((u8)(psta->mac_id&0xFF)) | ((psta->rssi_stat.UndecoratedSmoothedPWDB&0x7F)<<16));
					else
						PWDB_rssi[sta_cnt++] = (psta->mac_id | (psta->rssi_stat.UndecoratedSmoothedPWDB<<16));

				}
			}
		}
#else
		_irqL irqL;
		struct list_head	*plist, *phead;
		struct sta_info *psta;
		struct sta_priv *pstapriv = &Adapter->stapriv;
		u8 bcast_addr[ETH_ALEN] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};

		_enter_critical_bh(&pstapriv->sta_hash_lock, &irqL);

		for (i = 0; i < NUM_STA; i++) {
			phead = &(pstapriv->sta_hash[i]);
			plist = get_next(phead);

			while ((rtw_end_of_queue_search(phead, plist)) == _FALSE) {
				psta = LIST_CONTAINOR(plist, struct sta_info, hash_list);

				plist = get_next(plist);

				if (_rtw_memcmp(psta->hwaddr, bcast_addr, ETH_ALEN)
				 || _rtw_memcmp(psta->hwaddr, myid(&Adapter->eeprompriv), ETH_ALEN))
					continue;

				if (psta->state & WIFI_ASOC_STATE) {
					if (psta->rssi_stat.UndecoratedSmoothedPWDB < tmpEntryMinPWDB)
						tmpEntryMinPWDB = psta->rssi_stat.UndecoratedSmoothedPWDB;

					if (psta->rssi_stat.UndecoratedSmoothedPWDB > tmpEntryMaxPWDB)
						tmpEntryMaxPWDB = psta->rssi_stat.UndecoratedSmoothedPWDB;

					if (psta->rssi_stat.UndecoratedSmoothedPWDB != (-1)) {
						/* printk("%s==> mac_id(%d),rssi(%d)\n",__FUNCTION__,psta->mac_id,psta->rssi_stat.UndecoratedSmoothedPWDB); */
						PWDB_rssi[sta_cnt++] = (psta->mac_id | (psta->rssi_stat.UndecoratedSmoothedPWDB<<16));
					}
				}

			}

		}

		_exit_critical_bh(&pstapriv->sta_hash_lock, &irqL);
#endif

		/* printk("%s==> sta_cnt(%d)\n",__FUNCTION__,sta_cnt); */

		for (i = 0; i < sta_cnt; i++) {
			if (PWDB_rssi[i] != (0)) {
				if (pHalData->fw_ractrl == _TRUE) {	/* Report every sta's RSSI to FW */
					if ((pDM_Odm->SupportICType == ODM_RTL8812) || (pDM_Odm->SupportICType == ODM_RTL8821)) {
						PWDB_rssi[i] |= (UL_DL_STATE << 24);
						rtl8812_set_rssi_cmd(Adapter, (u8 *)(&PWDB_rssi[i]));
					}
				} else {
				}
			}
		}
	}



	if (tmpEntryMaxPWDB != 0) {	/* If associated entry is found */
		pdmpriv->EntryMaxUndecoratedSmoothedPWDB = tmpEntryMaxPWDB;
	} else {
		pdmpriv->EntryMaxUndecoratedSmoothedPWDB = 0;
	}

	if (tmpEntryMinPWDB != 0xff) {	/* If associated entry is found */
		pdmpriv->EntryMinUndecoratedSmoothedPWDB = tmpEntryMinPWDB;
	} else {
		pdmpriv->EntryMinUndecoratedSmoothedPWDB = 0;
	}

	FindMinimumRSSI(Adapter);	/* get pdmpriv->MinUndecoratedPWDBForDM */

	ODM_CmnInfoUpdate(&pHalData->odmpriv, ODM_CMNINFO_RSSI_MIN, pdmpriv->MinUndecoratedPWDBForDM);
}

/* #endif */

/*
 * 3============================================================
 * 3 Tx Power Tracking
 * 3============================================================
 */


void ODM_TXPowerTrackingCheck(struct _rtw_dm *pDM_Odm)
{
	/*
	 * For AP/ADSL use prtl8192cd_priv
	 * For CE/NIC use _ADAPTER
	 */

	/*
	 * if (!(pDM_Odm->SupportAbility & ODM_RF_TX_PWR_TRACK))
	 * 	return;
	 */

	/*
	 * 2011/09/29 MH In HW integration first stage, we provide 4 different handle to operate
	 * at the same time. In the stage2/3, we need to prive universal interface and merge all
	 * HW dynamic mechanism.
	 */

	odm_TXPowerTrackingCheckCE(pDM_Odm);
}

void odm_TXPowerTrackingCheckCE(struct _rtw_dm *pDM_Odm)
{
	struct rtl_priv *Adapter = pDM_Odm->Adapter;
	struct rtl_hal *rtlhal = rtl_hal(Adapter);

	if (!(pDM_Odm->SupportAbility & ODM_RF_TX_PWR_TRACK)) {
		return;
	}

	if (!pDM_Odm->RFCalibrateInfo.TM_Trigger) {		/* at least delay 1 sec */
		/* pHalData->TxPowerCheckCnt++;	//cosa add for debug */
		if (IS_HARDWARE_TYPE_JAGUAR(rtlhal))
			rtw_hal_write_rfreg(pDM_Odm->Adapter, RF90_PATH_A, RF_T_METER_NEW, (BIT17 | BIT16), 0x03);
		else
			rtw_hal_write_rfreg(pDM_Odm->Adapter, RF90_PATH_A, RF_T_METER_OLD, bRFRegOffsetMask, 0x60);

		/* DBG_871X("Trigger Thermal Meter!!\n"); */

		pDM_Odm->RFCalibrateInfo.TM_Trigger = 1;
		return;
	} else {
		/* DBG_871X("Schedule TxPowerTracking direct call!!\n"); */
		ODM_TXPowerTrackingCallback_ThermalMeter(Adapter);
		pDM_Odm->RFCalibrateInfo.TM_Trigger = 0;
	}

}


/*
 * antenna mapping info
 *  1: right-side antenna
 * 2/0: left-side antenna
 * PDM_SWAT_Table->CCK_Ant1_Cnt /OFDM_Ant1_Cnt:  for right-side antenna:   Ant:1    RxDefaultAnt1
 * PDM_SWAT_Table->CCK_Ant2_Cnt /OFDM_Ant2_Cnt:  for left-side antenna:     Ant:0    RxDefaultAnt2
 * We select left antenna as default antenna in initial process, modify it as needed
 */

/*
 * 3============================================================
 * 3 SW Antenna Diversity
 * 3============================================================
 */
#if (defined(CONFIG_SW_ANTENNA_DIVERSITY))

/*
 * 20100514 Luke/Joseph:
 * Callback function for 500ms antenna test trying.
 */

#else

void ODM_SwAntDivChkPerPktRssi(struct _rtw_dm *pDM_Odm, u8 StationID,
	PODM_PHY_INFO_T pPhyInfo)
{
}

void ODM_SwAntDivResetBeforeLink(struct _rtw_dm *pDM_Odm)
{
}

void ODM_SwAntDivRestAfterLink(IN struct _rtw_dm *pDM_Odm)
{
}

void odm_SwAntDivChkAntSwitchCallback(void *FunctionContext)
{
}

#endif

/*
 * 3============================================================
 * 3 SW Antenna Diversity
 * 3============================================================
 */

/*
 * ============================================================
 * EDCA Turbo
 * ============================================================
 */

void odm_EdcaTurboCheck(struct _rtw_dm *pDM_Odm)
{
	/*
	 * For AP/ADSL use prtl8192cd_priv
	 * For CE/NIC use _ADAPTER
	 */

	/*
	 *
	 * 2011/09/29 MH In HW integration first stage, we provide 4 different handle to operate
	 * at the same time. In the stage2/3, we need to prive universal interface and merge all
	 * HW dynamic mechanism.
	 */

	if (!(pDM_Odm->SupportAbility & ODM_MAC_EDCA_TURBO))
		return;

	odm_EdcaTurboCheckCE(pDM_Odm);
}



VOID odm_EdcaTurboCheckCE(struct _rtw_dm *pDM_Odm)
{

	struct rtl_priv *Adapter = pDM_Odm->Adapter;
	uint32_t	EDCA_BE_UL = 0x5ea42b;	/* Parameter suggested by Scott  */	/* edca_setting_UL[pMgntInfo->IOTPeer]; */
	uint32_t	EDCA_BE_DL = 0x5ea42b;	/* Parameter suggested by Scott  */	/* edca_setting_DL[pMgntInfo->IOTPeer]; */
	uint32_t	ICType = pDM_Odm->SupportICType;
	uint32_t	IOTPeer = 0;
	u8		WirelessMode = 0xFF;	/* invalid value */
	uint32_t 	trafficIndex;
	uint32_t	edca_param;
	u64		cur_tx_bytes = 0;
	u64		cur_rx_bytes = 0;
	u8		bbtchange = _FALSE;
	 struct _rtw_hal		*pHalData = GET_HAL_DATA(Adapter);
	struct xmit_priv		*pxmitpriv = &(Adapter->xmitpriv);
	struct recv_priv		*precvpriv = &(Adapter->recvpriv);
	struct registry_priv	*pregpriv = &Adapter->registrypriv;
	struct mlme_ext_priv	*pmlmeext = &(Adapter->mlmeextpriv);
	struct mlme_ext_info	*pmlmeinfo = &(pmlmeext->mlmext_info);


	if ((pregpriv->wifi_spec == 1)) {	/*|| (pmlmeinfo->HT_enable == 0)) */
		goto dm_CheckEdcaTurbo_EXIT;
	}

	if (pDM_Odm->pWirelessMode != NULL)
		WirelessMode = *(pDM_Odm->pWirelessMode);

	IOTPeer = pmlmeinfo->assoc_AP_vendor;

	if (IOTPeer >=  HT_IOT_PEER_MAX) {
		goto dm_CheckEdcaTurbo_EXIT;
	}

	/* Check if the status needs to be changed. */
	if ((bbtchange) || (!precvpriv->bIsAnyNonBEPkts)) {
		cur_tx_bytes = pxmitpriv->tx_bytes - pxmitpriv->last_tx_bytes;
		cur_rx_bytes = precvpriv->rx_bytes - precvpriv->last_rx_bytes;

		/* traffic, TX or RX */
		if ((IOTPeer == HT_IOT_PEER_RALINK) || (IOTPeer == HT_IOT_PEER_ATHEROS)) {
			if (cur_tx_bytes > (cur_rx_bytes << 2)) {
				/* Uplink TP is present. */
				trafficIndex = UP_LINK;
			} else {
				/* Balance TP is present. */
				trafficIndex = DOWN_LINK;
			}
		} else {
			if (cur_rx_bytes > (cur_tx_bytes << 2)) {
				/* Downlink TP is present. */
				trafficIndex = DOWN_LINK;
			} else {
				/* Balance TP is present. */
				trafficIndex = UP_LINK;
			}
		}

		if ((pDM_Odm->DM_EDCA_Table.prv_traffic_idx != trafficIndex) || (!pDM_Odm->DM_EDCA_Table.bCurrentTurboEDCA)) {
			/* merge from 92s_92c_merge temp brunch v2445    20120215 */
			if ((IOTPeer == HT_IOT_PEER_CISCO)
			   && ((WirelessMode == ODM_WM_G) || (WirelessMode == (ODM_WM_B|ODM_WM_G)) || (WirelessMode == ODM_WM_A) || (WirelessMode == ODM_WM_B))) {
				EDCA_BE_DL = edca_setting_DL_GMode[IOTPeer];
			} else if ((IOTPeer == HT_IOT_PEER_AIRGO)
			       && ((WirelessMode == ODM_WM_G) || (WirelessMode == ODM_WM_A))) {
					EDCA_BE_DL = 0xa630;
			} else if (IOTPeer == HT_IOT_PEER_MARVELL) {
				EDCA_BE_DL = edca_setting_DL[IOTPeer];
				EDCA_BE_UL = edca_setting_UL[IOTPeer];
			} else if (IOTPeer == HT_IOT_PEER_ATHEROS) {
				/* Set DL EDCA for Atheros peer to 0x3ea42b. Suggested by SD3 Wilson for ASUS TP issue. */
				EDCA_BE_DL = edca_setting_DL[IOTPeer];
			}

			if ((ICType == ODM_RTL8812)) {		/* add 8812AU/8812AE */
				EDCA_BE_UL = 0x5ea42b;
				EDCA_BE_DL = 0x5ea42b;
				ODM_RT_TRACE(pDM_Odm, ODM_COMP_EDCA_TURBO, ODM_DBG_LOUD, ("8812A: EDCA_BE_UL=0x%x EDCA_BE_DL =0x%x", EDCA_BE_UL, EDCA_BE_DL));
			}

			if (trafficIndex == DOWN_LINK)
				edca_param = EDCA_BE_DL;
			else
				edca_param = EDCA_BE_UL;

			rtl_write_dword(Adapter, REG_EDCA_BE_PARAM, edca_param);

			pDM_Odm->DM_EDCA_Table.prv_traffic_idx = trafficIndex;
		}

		pDM_Odm->DM_EDCA_Table.bCurrentTurboEDCA = _TRUE;
	} else {
		/*
		 * Turn Off EDCA turbo here.
		 * Restore original EDCA according to the declaration of AP.
		 */
		if (pDM_Odm->DM_EDCA_Table.bCurrentTurboEDCA) {
			rtl_write_dword(Adapter, REG_EDCA_BE_PARAM, pHalData->AcParam_BE);
			pDM_Odm->DM_EDCA_Table.bCurrentTurboEDCA = _FALSE;
		}
	}

dm_CheckEdcaTurbo_EXIT:
	/* Set variables for next time. */
	precvpriv->bIsAnyNonBEPkts = _FALSE;
	pxmitpriv->last_tx_bytes = pxmitpriv->tx_bytes;
	precvpriv->last_rx_bytes = precvpriv->rx_bytes;
}



/*
 * need to ODM CE Platform
 * move to here for ANT detection mechanism using
 */

/*
 * LukeLee:
 * PSD function will be moved to FW in future IC, but now is only implemented in MP platform
 * So PSD function will not be incorporated to common ODM
 */

/*
 * 2011/12/02 MH Copy from MP oursrc for temporarily test.
 */

/* Justin: According to the current RRSI to adjust Response Frame TX power, 2012/11/05 */
void odm_dtc(struct _rtw_dm *pDM_Odm)
{
#ifdef CONFIG_DM_RESP_TXAGC
	#define DTC_BASE            35	/* RSSI higher than this value, start to decade TX power */
	#define DTC_DWN_BASE       (DTC_BASE-5)	/* RSSI lower than this value, start to increase TX power */

	/* RSSI vs TX power step mapping: decade TX power */
	static const u8 dtc_table_down[] = {
		DTC_BASE,
		(DTC_BASE+5),
		(DTC_BASE+10),
		(DTC_BASE+15),
		(DTC_BASE+20),
		(DTC_BASE+25)
	};

	/* RSSI vs TX power step mapping: increase TX power */
	static const u8 dtc_table_up[] = {
		DTC_DWN_BASE,
		(DTC_DWN_BASE-5),
		(DTC_DWN_BASE-10),
		(DTC_DWN_BASE-15),
		(DTC_DWN_BASE-15),
		(DTC_DWN_BASE-20),
		(DTC_DWN_BASE-20),
		(DTC_DWN_BASE-25),
		(DTC_DWN_BASE-25),
		(DTC_DWN_BASE-30),
		(DTC_DWN_BASE-35)
	};

	u8 i;
	u8 dtc_steps = 0;
	u8 sign;
	u8 resp_txagc = 0;

	if (DTC_BASE < pDM_Odm->RSSI_Min) {
		/* need to decade the CTS TX power */
		sign = 1;
		for (i = 0; i < ARRAY_SIZE(dtc_table_down); i++) {
			if ((dtc_table_down[i] >= pDM_Odm->RSSI_Min) || (dtc_steps >= 6))
				break;
			else
				dtc_steps++;
		}
	}
	else {
		sign = 0;
		dtc_steps = 0;
	}

	resp_txagc = dtc_steps | (sign << 4);
	resp_txagc = resp_txagc | (resp_txagc << 5);
	ODM_Write1Byte(pDM_Odm, 0x06d9, resp_txagc);

	DBG_871X("%s RSSI_Min:%u, set RESP_TXAGC to %s %u\n",
		__func__, pDM_Odm->RSSI_Min, sign?"minus":"plus", dtc_steps);
#endif
}


