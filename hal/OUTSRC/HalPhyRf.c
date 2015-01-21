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

#include "odm_precomp.h"
#include <../rtl8821au/dm.h>

#define CALCULATE_SWINGTALBE_OFFSET(_offset, _direction, _size, _deltaThermal) \
	do { \
	for (_offset = 0; _offset < _size; _offset++) { \
		if (_deltaThermal < thermalThreshold[_direction][_offset]) { \
			if (_offset != 0) \
				_offset--; \
			break; \
		} \
	} \
	if (_offset >= _size) \
		_offset = _size-1; \
	} while (0)

static void ConfigureTxpowerTrack_8812A(PTXPWRTRACK_CFG pConfig)
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
	pConfig->GetDeltaSwingTable = rtl8812au_get_delta_swing_table;
}

static void ConfigureTxpowerTrack_8821A(PTXPWRTRACK_CFG pConfig)
{
	pConfig->SwingTableSize_CCK = TXSCALE_TABLE_SIZE;
	pConfig->SwingTableSize_OFDM = TXSCALE_TABLE_SIZE;
	pConfig->Threshold_IQK = IQK_THRESHOLD;
	pConfig->AverageThermalNum = AVG_THERMAL_NUM_8812A;
	pConfig->RfPathCount = MAX_PATH_NUM_8821A;
	pConfig->ThermalRegAddr = RF_T_METER_8812A;

	pConfig->ODM_TxPwrTrackSetPwr = ODM_TxPwrTrackSetPwr8821A;
	pConfig->DoIQK = DoIQK_8821A;
	pConfig->PHY_LCCalibrate = PHY_LCCalibrate_8812A;
	pConfig->GetDeltaSwingTable = rtl8821au_get_delta_swing_table;
}

void ConfigureTxpowerTrack(struct _rtw_dm *pDM_Odm, PTXPWRTRACK_CFG pConfig)
{
	if (pDM_Odm->SupportICType == ODM_RTL8821)
		ConfigureTxpowerTrack_8821A(pConfig);
	if (pDM_Odm->SupportICType == ODM_RTL8812)
		ConfigureTxpowerTrack_8812A(pConfig);
}

/*
 * ======================================================================
 *  <2012111if (, Kordan> This function should be called when TxAGC changed.
 *  Otherwise the previous compensation is gone, because we record the
 *  delta of temperature between two TxPowerTracking watch dogs.
 *
 *  NOTE: If Tx BB swing or Tx scaling is varified during run-time, still
 *        need to call this function.
 * ======================================================================
 */
VOID ODM_ClearTxPowerTrackingState(struct _rtw_dm *pDM_Odm)
{
	struct rtl_efuse *efuse = rtl_efuse(pDM_Odm->Adapter);
	struct rtl_dm	*rtldm = rtl_dm(pDM_Odm->Adapter);
	struct _rtw_hal *pHalData = GET_HAL_DATA(pDM_Odm->Adapter);
	u8 p = 0;

	rtldm->BbSwingIdxCckBase = rtldm->DefaultCckIndex;
	rtldm->BbSwingIdxCck = rtldm->DefaultCckIndex;
	rtldm->CCK_index = 0;

	for (p = RF90_PATH_A; p < MAX_RF_PATH; ++p) {
		rtldm->BbSwingIdxOfdmBase[p] = rtldm->DefaultOfdmIndex;
		rtldm->BbSwingIdxOfdm[p] = rtldm->DefaultOfdmIndex;
		rtldm->OFDM_index[p] = rtldm->DefaultOfdmIndex;

		rtldm->PowerIndexOffset[p] = 0;
		rtldm->DeltaPowerIndex[p] = 0;
		rtldm->DeltaPowerIndexLast[p] = 0;
		rtldm->PowerIndexOffset[p] = 0;

		rtldm->Aboslute_OFDMSwingIdx[p] = 0;    /* Initial Mix mode power tracking */
		rtldm->Remnant_OFDMSwingIdx[p] = 0;
	}

	rtldm->Modify_TxAGC_Flag_PathA = FALSE;       /* Initial at Modify Tx Scaling Mode */
	rtldm->Modify_TxAGC_Flag_PathB = FALSE;       /* Initial at Modify Tx Scaling Mode */
	rtldm->Remnant_CCKSwingIdx = 0;
	rtldm->thermalvalue = efuse->EEPROMThermalMeter;
	rtldm->thermalvalue_iqk = efuse->EEPROMThermalMeter;
	rtldm->thermalvalue_lck = efuse->EEPROMThermalMeter;
}

VOID ODM_TXPowerTrackingCallback_ThermalMeter(struct rtl_priv *Adapter)
{
	struct rtl_efuse *efuse = rtl_efuse(Adapter);
	struct rtl_dm	*rtldm = rtl_dm(Adapter);
	
	 struct _rtw_hal	*pHalData = GET_HAL_DATA(Adapter);
	struct _rtw_dm *pDM_Odm = &pHalData->odmpriv;

	u8	ThermalValue = 0, delta, delta_LCK, delta_IQK, p = 0, i = 0;
	u8	ThermalValue_AVG_count = 0;
	uint32_t	ThermalValue_AVG = 0;

	u8	OFDM_min_index = 0;  /* OFDM BB Swing should be less than +if (.0dB, which is required by Arthur */
	u8	Indexforchannel = 0; /* GetRightChnlPlaceforIQK(pHalData->CurrentChannel) */

	TXPWRTRACK_CFG 	c;


	/* 4 1. The following TWO tables decide the final index of OFDM/CCK swing table. */
	u8 *up_a, *down_a, *up_b, *down_b;

	/* 4 2. Initilization ( 7 steps in total ) */

	ConfigureTxpowerTrack(pDM_Odm, &c);

	(*c.GetDeltaSwingTable)(pDM_Odm, (u8 **)&up_a, (u8 **)&down_a,
					 (u8 **)&up_b, (u8 **)&down_b);

	rtldm->TXPowerTrackingCallbackCnt++; /* cosa add for debug */
	rtldm->bTXPowerTrackingInit = TRUE;

	ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD, ("===>ODM_TXPowerTrackingCallback_ThermalMeter, \n rtldm->BbSwingIdxCckBase: %d, rtldm->BbSwingIdxOfdmBase[A]: %d, rtldm->DefaultOfdmIndex: %d\n", rtldm->BbSwingIdxCckBase, rtldm->BbSwingIdxOfdmBase[RF90_PATH_A], rtldm->DefaultOfdmIndex));

	ThermalValue = (u8)rtw_hal_read_rfreg(pDM_Odm->Adapter, RF90_PATH_A, c.ThermalRegAddr, 0xfc00);	/* 0x42: RF Reg[15:10] 88E */
	if (!rtldm->TxPowerTrackControl
	 || efuse->EEPROMThermalMeter == 0
	 || efuse->EEPROMThermalMeter == 0xFF)
		return;

	/* 4 if (. Initialize ThermalValues of RFCalibrateInfo */

	if (rtldm->bReloadtxpowerindex) {
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD, ("reload ofdm index for band switch\n"));
	}

	/* 4 4. Calculate average thermal meter */

	rtldm->thermalvalue_avg[rtldm->thermalvalue_avg_index] = ThermalValue;
	rtldm->thermalvalue_avg_index++;
	if (rtldm->thermalvalue_avg_index == c.AverageThermalNum)   /* Average times =  c.AverageThermalNum */
		rtldm->thermalvalue_avg_index = 0;

	for (i = 0; i < c.AverageThermalNum; i++) {
		if (rtldm->thermalvalue_avg[i]) {
			ThermalValue_AVG += rtldm->thermalvalue_avg[i];
			ThermalValue_AVG_count++;
		}
	}

	if (ThermalValue_AVG_count) {               /* Calculate Average ThermalValue after average enough times */
		ThermalValue = (u8)(ThermalValue_AVG / ThermalValue_AVG_count);
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD, ("AVG Thermal Meter = 0x%X, EFUSE Thermal Base = 0x%X\n", ThermalValue, efuse->EEPROMThermalMeter));
	}

	/* 4 5. Calculate delta, delta_LCK, delta_IQK. */

	/* "delta" here is used to determine whether thermal value changes or not. */
	delta 	  = (ThermalValue > rtldm->thermalvalue)?(ThermalValue - rtldm->thermalvalue):(rtldm->thermalvalue - ThermalValue);
	delta_LCK = (ThermalValue > rtldm->thermalvalue_lck)?(ThermalValue - rtldm->thermalvalue_lck):(rtldm->thermalvalue_lck - ThermalValue);
	delta_IQK = (ThermalValue > rtldm->thermalvalue_iqk)?(ThermalValue - rtldm->thermalvalue_iqk):(rtldm->thermalvalue_iqk - ThermalValue);

	ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD, ("(delta, delta_LCK, delta_IQK) = (%d, %d, %d)\n", delta, delta_LCK, delta_IQK));

	/* 4 6. If necessary, do LCK. */

	if ((delta_LCK >= c.Threshold_IQK)) {	/* Delta temperature is equal to or larger than 20 centigrade. */
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD, ("delta_LCK(%d) >= Threshold_IQK(%d)\n", delta_LCK, c.Threshold_IQK));
		rtldm->thermalvalue_lck = ThermalValue;
		if (c.PHY_LCCalibrate)
			(*c.PHY_LCCalibrate)(pDM_Odm);
	}

	/* if ( 7. If necessary, move the index of swing table to adjust Tx power. */

	if (delta > 0 && rtldm->TxPowerTrackControl) {
		/* "delta" here is used to record the absolute value of differrence. */
	    delta = ThermalValue > efuse->EEPROMThermalMeter?(ThermalValue - efuse->EEPROMThermalMeter):(efuse->EEPROMThermalMeter - ThermalValue);
		if (delta >= TXSCALE_TABLE_SIZE)
			delta = TXSCALE_TABLE_SIZE - 1;

		/* 4 7.1 The Final Power Index = BaseIndex + PowerIndexOffset */

		if (ThermalValue > efuse->EEPROMThermalMeter) {
			ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD, ("deltaSwingTableIdx_TUP_A[%d] = %d\n", delta, up_a[delta]));
			rtldm->DeltaPowerIndexLast[RF90_PATH_A] = rtldm->DeltaPowerIndex[RF90_PATH_A];
			rtldm->DeltaPowerIndex[RF90_PATH_A] = up_a[delta];

			rtldm->Aboslute_OFDMSwingIdx[RF90_PATH_A] =  up_a[delta];        /* Record delta swing for mix mode power tracking */

			ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD, ("******Temp is higher and rtldm->Aboslute_OFDMSwingIdx[RF90_PATH_A] = %d\n", rtldm->Aboslute_OFDMSwingIdx[RF90_PATH_A]));

			if (c.RfPathCount > 1) {
				ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD, ("deltaSwingTableIdx_TUP_B[%d] = %d\n", delta, up_b[delta]));
				rtldm->DeltaPowerIndexLast[RF90_PATH_B] = rtldm->DeltaPowerIndex[RF90_PATH_B];
				rtldm->DeltaPowerIndex[RF90_PATH_B] = up_b[delta];

				rtldm->Aboslute_OFDMSwingIdx[RF90_PATH_B] =  up_a[delta];       /* Record delta swing for mix mode power tracking */

				ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD, ("******Temp is higher and rtldm->Aboslute_OFDMSwingIdx[RF90_PATH_B] = %d\n", rtldm->Aboslute_OFDMSwingIdx[RF90_PATH_B]));
			}

		} else {
			ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD, ("deltaSwingTableIdx_TDOWN_A[%d] = %d\n", delta, down_a[delta]));

			rtldm->DeltaPowerIndexLast[RF90_PATH_A] = rtldm->DeltaPowerIndex[RF90_PATH_A];
			rtldm->DeltaPowerIndex[RF90_PATH_A] = -1 * down_a[delta];

			rtldm->Aboslute_OFDMSwingIdx[RF90_PATH_A] =  -1 * down_a[delta];        /* Record delta swing for mix mode power tracking */

			ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD, ("******Temp is lower and rtldm->Aboslute_OFDMSwingIdx[RF90_PATH_A] = %d\n", rtldm->Aboslute_OFDMSwingIdx[RF90_PATH_A]));

			if (c.RfPathCount > 1) {
				ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD, ("deltaSwingTableIdx_TDOWN_B[%d] = %d\n", delta, down_b[delta]));

				rtldm->DeltaPowerIndexLast[RF90_PATH_B] = rtldm->DeltaPowerIndex[RF90_PATH_B];
				rtldm->DeltaPowerIndex[RF90_PATH_B] = -1 * down_b[delta];

				rtldm->Aboslute_OFDMSwingIdx[RF90_PATH_B] =  -1 * down_b[delta];       /* Record delta swing for mix mode power tracking */

				ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD, ("******Temp is lower and rtldm->Aboslute_OFDMSwingIdx[RF90_PATH_B] = %d\n", rtldm->Aboslute_OFDMSwingIdx[RF90_PATH_B]));
			}
		}

	    for (p = RF90_PATH_A; p < c.RfPathCount; p++) {
			ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD, ("\n\n================================ [Path-%c] Calculating PowerIndexOffset ================================\n", (p == RF90_PATH_A ? 'A' : 'B')));
			if (rtldm->DeltaPowerIndex[p] == rtldm->DeltaPowerIndexLast[p])         /* If Thermal value changes but lookup table value still the same */
				rtldm->PowerIndexOffset[p] = 0;
			else
				rtldm->PowerIndexOffset[p] = rtldm->DeltaPowerIndex[p] - rtldm->DeltaPowerIndexLast[p];      /* Power Index Diff between 2 times Power Tracking */

			ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD, ("[Path-%c] PowerIndexOffset(%d) = DeltaPowerIndex(%d) - DeltaPowerIndexLast(%d)\n", (p == RF90_PATH_A ? 'A' : 'B'), rtldm->PowerIndexOffset[p], rtldm->DeltaPowerIndex[p],
			rtldm->DeltaPowerIndexLast[p]));

			rtldm->OFDM_index[p] = rtldm->BbSwingIdxOfdmBase[p] + rtldm->PowerIndexOffset[p];
			rtldm->CCK_index = rtldm->BbSwingIdxCckBase + rtldm->PowerIndexOffset[p];

			rtldm->BbSwingIdxCck = rtldm->CCK_index;
			rtldm->BbSwingIdxOfdm[p] = rtldm->OFDM_index[p];

	       /* *************Print BB Swing Base and Index Offset************* */

			ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD, ("The 'CCK' final index(%d) = BaseIndex(%d) + PowerIndexOffset(%d)\n", rtldm->BbSwingIdxCck, rtldm->BbSwingIdxCckBase, rtldm->PowerIndexOffset[p]));
			ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD, ("The 'OFDM' final index(%d) = BaseIndex[%c](%d) + PowerIndexOffset(%d)\n", rtldm->BbSwingIdxOfdm[p], (p == RF90_PATH_A ? 'A' : 'B'), rtldm->BbSwingIdxOfdmBase[p], rtldm->PowerIndexOffset[p]));

		    /* 4 7.1 Handle boundary conditions of index. */

			if (rtldm->OFDM_index[p] > c.SwingTableSize_OFDM-1) {
				rtldm->OFDM_index[p] = c.SwingTableSize_OFDM-1;
			} else if (rtldm->OFDM_index[p] < OFDM_min_index) {
				rtldm->OFDM_index[p] = OFDM_min_index;
			}
		}
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD, ("\n\n========================================================================================================\n"));
		if (rtldm->CCK_index > c.SwingTableSize_CCK-1)
			rtldm->CCK_index = c.SwingTableSize_CCK-1;
		/*
		 * else if (rtldm->RFCalibrateInfo.CCK_index < 0)
		 * 	rtldm->RFCalibrateInfo.CCK_index = 0;
		 */
	} else {
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD, ("The thermal meter is unchanged or TxPowerTracking OFF(%d): ThermalValue: %d , rtldm->RFCalibrateInfo.ThermalValue: %d\n", rtldm->TxPowerTrackControl, ThermalValue, rtldm->thermalvalue));

		for (p = RF90_PATH_A; p < c.RfPathCount; p++)
			rtldm->PowerIndexOffset[p] = 0;
	}
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD, ("TxPowerTracking: [CCK] Swing Current Index: %d, Swing Base Index: %d\n", rtldm->CCK_index, rtldm->BbSwingIdxCckBase));       /* Print Swing base & current */
	for (p = RF90_PATH_A; p < c.RfPathCount; p++) {
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD, ("TxPowerTracking: [OFDM] Swing Current Index: %d, Swing Base Index[%c]: %d\n", rtldm->OFDM_index[p], (p == RF90_PATH_A ? 'A' : 'B'), rtldm->BbSwingIdxOfdmBase[p]));
	}

	if ((rtldm->PowerIndexOffset[RF90_PATH_A] != 0 ||  rtldm->PowerIndexOffset[RF90_PATH_B] != 0)
	 && rtldm->TxPowerTrackControl) {
		/* 4 7.2 Configure the Swing Table to adjust Tx Power. */
		rtldm->bTxPowerChanged = TRUE; /* Always TRUE after Tx Power is adjusted by power tracking. */
		/*
		 *  2012/04/2if ( MH According to Luke's suggestion, we can not write BB digital
		 *  to increase TX power. Otherwise, EVM will be bad.
		 *
		 *  2012/04/25 MH Add for tx power tracking to set tx power in tx agc for 88E.
		 */
		if (ThermalValue > rtldm->thermalvalue) {
			ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD, ("Temperature Increasing(A): delta_pi: %d , delta_t: %d, Now_t: %d, EFUSE_t: %d, Last_t: %d\n", rtldm->PowerIndexOffset[RF90_PATH_A], delta, ThermalValue, efuse->EEPROMThermalMeter, rtldm->thermalvalue));
			if (c.RfPathCount > 1)
				ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD, ("Temperature Increasing(B): delta_pi: %d , delta_t: %d, Now_t: %d, EFUSE_t: %d, Last_t: %d\n", rtldm->PowerIndexOffset[RF90_PATH_B], delta, ThermalValue, efuse->EEPROMThermalMeter, rtldm->thermalvalue));

			} else if (ThermalValue < rtldm->thermalvalue) { /* Low temperature */
				ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD, ("Temperature Decreasing(A): delta_pi: %d , delta_t: %d, Now_t: %d, EFUSE_t: %d, Last_t: %d\n", rtldm->PowerIndexOffset[RF90_PATH_A], delta, ThermalValue, efuse->EEPROMThermalMeter, rtldm->thermalvalue));

				if (c.RfPathCount > 1)
					ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD, ("Temperature Decreasing(B): delta_pi: %d , delta_t: %d, Now_t: %d, EFUSE_t: %d, Last_t: %d\n", rtldm->PowerIndexOffset[RF90_PATH_B], delta, ThermalValue, efuse->EEPROMThermalMeter, rtldm->thermalvalue));

			}
			if (ThermalValue > efuse->EEPROMThermalMeter) {
				ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD, ("Temperature(%d) higher than PG value(%d)\n", ThermalValue, efuse->EEPROMThermalMeter));

				if (pDM_Odm->SupportICType == ODM_RTL8821) {
					ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD, ("**********Enter POWER Tracking MIX_MODE**********\n"));
					for (p = RF90_PATH_A; p < c.RfPathCount; p++)
						(*c.ODM_TxPwrTrackSetPwr)(pDM_Odm, MIX_MODE, p, Indexforchannel);
				} else {
					for (p = RF90_PATH_A; p < c.RfPathCount; p++)
						(*c.ODM_TxPwrTrackSetPwr)(pDM_Odm, BBSWING, p, Indexforchannel);
				}
			} else {
				ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD, ("Temperature(%d) lower than PG value(%d)\n", ThermalValue, efuse->EEPROMThermalMeter));

				if (pDM_Odm->SupportICType == ODM_RTL8821) {
					ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD, ("**********Enter POWER Tracking MIX_MODE**********\n"));
					for (p = RF90_PATH_A; p < c.RfPathCount; p++)
						(*c.ODM_TxPwrTrackSetPwr)(pDM_Odm, MIX_MODE, p, Indexforchannel);
				} else {
					for (p = RF90_PATH_A; p < c.RfPathCount; p++)
						(*c.ODM_TxPwrTrackSetPwr)(pDM_Odm, BBSWING, p, Indexforchannel);
				}
			}

			rtldm->BbSwingIdxCckBase = rtldm->BbSwingIdxCck;  	/* Record last time Power Tracking result as base. */
			for (p = RF90_PATH_A; p < c.RfPathCount; p++)
				rtldm->BbSwingIdxOfdmBase[p] = rtldm->BbSwingIdxOfdm[p];

			ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD,  ("rtldm->RFCalibrateInfo.ThermalValue = %d ThermalValue= %d\n", rtldm->thermalvalue, ThermalValue));

			rtldm->thermalvalue = ThermalValue;     /* Record last Power Tracking Thermal Value */

	}
	if ((delta_IQK >= c.Threshold_IQK))	/* Delta temperature is equal to or larger than 20 centigrade (When threshold is 8). */
		(*c.DoIQK)(pDM_Odm, delta_IQK, ThermalValue, 8);

	ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD, ("<===ODM_TXPowerTrackingCallback_ThermalMeter\n"));

	rtldm->TXPowercount = 0;
}




/*
 * if (============================================================
 * if ( IQ Calibration
 * if (============================================================
 */


