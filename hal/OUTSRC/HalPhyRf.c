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
	struct _rtw_hal *pHalData = GET_HAL_DATA(pDM_Odm->Adapter);
	u8 p = 0;

	pDM_Odm->BbSwingIdxCckBase = pDM_Odm->DefaultCckIndex;
	pDM_Odm->BbSwingIdxCck = pDM_Odm->DefaultCckIndex;
	pDM_Odm->RFCalibrateInfo.CCK_index = 0;

	for (p = RF90_PATH_A; p < MAX_RF_PATH; ++p) {
		pDM_Odm->BbSwingIdxOfdmBase[p] = pDM_Odm->DefaultOfdmIndex;
		pDM_Odm->BbSwingIdxOfdm[p] = pDM_Odm->DefaultOfdmIndex;
		pDM_Odm->RFCalibrateInfo.OFDM_index[p] = pDM_Odm->DefaultOfdmIndex;

		pDM_Odm->RFCalibrateInfo.PowerIndexOffset[p] = 0;
		pDM_Odm->RFCalibrateInfo.DeltaPowerIndex[p] = 0;
		pDM_Odm->RFCalibrateInfo.DeltaPowerIndexLast[p] = 0;
		pDM_Odm->RFCalibrateInfo.PowerIndexOffset[p] = 0;

		pDM_Odm->Aboslute_OFDMSwingIdx[p] = 0;    /* Initial Mix mode power tracking */
		pDM_Odm->Remnant_OFDMSwingIdx[p] = 0;
	}

	pDM_Odm->Modify_TxAGC_Flag_PathA = FALSE;       /* Initial at Modify Tx Scaling Mode */
	pDM_Odm->Modify_TxAGC_Flag_PathB = FALSE;       /* Initial at Modify Tx Scaling Mode */
	pDM_Odm->Remnant_CCKSwingIdx = 0;
	pDM_Odm->RFCalibrateInfo.ThermalValue = efuse->EEPROMThermalMeter;
	pDM_Odm->RFCalibrateInfo.ThermalValue_IQK = efuse->EEPROMThermalMeter;
	pDM_Odm->RFCalibrateInfo.ThermalValue_LCK = efuse->EEPROMThermalMeter;
}

VOID ODM_TXPowerTrackingCallback_ThermalMeter(struct rtl_priv *Adapter)
{
	struct rtl_efuse *efuse = rtl_efuse(Adapter);
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

	pDM_Odm->RFCalibrateInfo.TXPowerTrackingCallbackCnt++; /* cosa add for debug */
	pDM_Odm->RFCalibrateInfo.bTXPowerTrackingInit = TRUE;

	ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD, ("===>ODM_TXPowerTrackingCallback_ThermalMeter, \n pDM_Odm->BbSwingIdxCckBase: %d, pDM_Odm->BbSwingIdxOfdmBase[A]: %d, pDM_Odm->DefaultOfdmIndex: %d\n", pDM_Odm->BbSwingIdxCckBase, pDM_Odm->BbSwingIdxOfdmBase[RF90_PATH_A], pDM_Odm->DefaultOfdmIndex));

	ThermalValue = (u8)rtw_hal_read_rfreg(pDM_Odm->Adapter, RF90_PATH_A, c.ThermalRegAddr, 0xfc00);	/* 0x42: RF Reg[15:10] 88E */
	if (!pDM_Odm->RFCalibrateInfo.TxPowerTrackControl
	 || efuse->EEPROMThermalMeter == 0
	 || efuse->EEPROMThermalMeter == 0xFF)
		return;

	/* 4 if (. Initialize ThermalValues of RFCalibrateInfo */

	if (pDM_Odm->RFCalibrateInfo.bReloadtxpowerindex) {
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD, ("reload ofdm index for band switch\n"));
	}

	/* 4 4. Calculate average thermal meter */

	pDM_Odm->RFCalibrateInfo.ThermalValue_AVG[pDM_Odm->RFCalibrateInfo.ThermalValue_AVG_index] = ThermalValue;
	pDM_Odm->RFCalibrateInfo.ThermalValue_AVG_index++;
	if (pDM_Odm->RFCalibrateInfo.ThermalValue_AVG_index == c.AverageThermalNum)   /* Average times =  c.AverageThermalNum */
		pDM_Odm->RFCalibrateInfo.ThermalValue_AVG_index = 0;

	for (i = 0; i < c.AverageThermalNum; i++) {
		if (pDM_Odm->RFCalibrateInfo.ThermalValue_AVG[i]) {
			ThermalValue_AVG += pDM_Odm->RFCalibrateInfo.ThermalValue_AVG[i];
			ThermalValue_AVG_count++;
		}
	}

	if (ThermalValue_AVG_count) {               /* Calculate Average ThermalValue after average enough times */
		ThermalValue = (u8)(ThermalValue_AVG / ThermalValue_AVG_count);
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD, ("AVG Thermal Meter = 0x%X, EFUSE Thermal Base = 0x%X\n", ThermalValue, efuse->EEPROMThermalMeter));
	}

	/* 4 5. Calculate delta, delta_LCK, delta_IQK. */

	/* "delta" here is used to determine whether thermal value changes or not. */
	delta 	  = (ThermalValue > pDM_Odm->RFCalibrateInfo.ThermalValue)?(ThermalValue - pDM_Odm->RFCalibrateInfo.ThermalValue):(pDM_Odm->RFCalibrateInfo.ThermalValue - ThermalValue);
	delta_LCK = (ThermalValue > pDM_Odm->RFCalibrateInfo.ThermalValue_LCK)?(ThermalValue - pDM_Odm->RFCalibrateInfo.ThermalValue_LCK):(pDM_Odm->RFCalibrateInfo.ThermalValue_LCK - ThermalValue);
	delta_IQK = (ThermalValue > pDM_Odm->RFCalibrateInfo.ThermalValue_IQK)?(ThermalValue - pDM_Odm->RFCalibrateInfo.ThermalValue_IQK):(pDM_Odm->RFCalibrateInfo.ThermalValue_IQK - ThermalValue);

	ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD, ("(delta, delta_LCK, delta_IQK) = (%d, %d, %d)\n", delta, delta_LCK, delta_IQK));

	/* 4 6. If necessary, do LCK. */

	if ((delta_LCK >= c.Threshold_IQK)) {	/* Delta temperature is equal to or larger than 20 centigrade. */
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD, ("delta_LCK(%d) >= Threshold_IQK(%d)\n", delta_LCK, c.Threshold_IQK));
		pDM_Odm->RFCalibrateInfo.ThermalValue_LCK = ThermalValue;
		if (c.PHY_LCCalibrate)
			(*c.PHY_LCCalibrate)(pDM_Odm);
	}

	/* if ( 7. If necessary, move the index of swing table to adjust Tx power. */

	if (delta > 0 && pDM_Odm->RFCalibrateInfo.TxPowerTrackControl) {
		/* "delta" here is used to record the absolute value of differrence. */
	    delta = ThermalValue > efuse->EEPROMThermalMeter?(ThermalValue - efuse->EEPROMThermalMeter):(efuse->EEPROMThermalMeter - ThermalValue);
		if (delta >= TXSCALE_TABLE_SIZE)
			delta = TXSCALE_TABLE_SIZE - 1;

		/* 4 7.1 The Final Power Index = BaseIndex + PowerIndexOffset */

		if (ThermalValue > efuse->EEPROMThermalMeter) {
			ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD, ("deltaSwingTableIdx_TUP_A[%d] = %d\n", delta, up_a[delta]));
			pDM_Odm->RFCalibrateInfo.DeltaPowerIndexLast[RF90_PATH_A] = pDM_Odm->RFCalibrateInfo.DeltaPowerIndex[RF90_PATH_A];
			pDM_Odm->RFCalibrateInfo.DeltaPowerIndex[RF90_PATH_A] = up_a[delta];

			pDM_Odm->Aboslute_OFDMSwingIdx[RF90_PATH_A] =  up_a[delta];        /* Record delta swing for mix mode power tracking */

			ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD, ("******Temp is higher and pDM_Odm->Aboslute_OFDMSwingIdx[RF90_PATH_A] = %d\n", pDM_Odm->Aboslute_OFDMSwingIdx[RF90_PATH_A]));

			if (c.RfPathCount > 1) {
				ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD, ("deltaSwingTableIdx_TUP_B[%d] = %d\n", delta, up_b[delta]));
				pDM_Odm->RFCalibrateInfo.DeltaPowerIndexLast[RF90_PATH_B] = pDM_Odm->RFCalibrateInfo.DeltaPowerIndex[RF90_PATH_B];
				pDM_Odm->RFCalibrateInfo.DeltaPowerIndex[RF90_PATH_B] = up_b[delta];

				pDM_Odm->Aboslute_OFDMSwingIdx[RF90_PATH_B] =  up_a[delta];       /* Record delta swing for mix mode power tracking */

				ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD, ("******Temp is higher and pDM_Odm->Aboslute_OFDMSwingIdx[RF90_PATH_B] = %d\n", pDM_Odm->Aboslute_OFDMSwingIdx[RF90_PATH_B]));
			}

		} else {
			ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD, ("deltaSwingTableIdx_TDOWN_A[%d] = %d\n", delta, down_a[delta]));

			pDM_Odm->RFCalibrateInfo.DeltaPowerIndexLast[RF90_PATH_A] = pDM_Odm->RFCalibrateInfo.DeltaPowerIndex[RF90_PATH_A];
			pDM_Odm->RFCalibrateInfo.DeltaPowerIndex[RF90_PATH_A] = -1 * down_a[delta];

			pDM_Odm->Aboslute_OFDMSwingIdx[RF90_PATH_A] =  -1 * down_a[delta];        /* Record delta swing for mix mode power tracking */

			ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD, ("******Temp is lower and pDM_Odm->Aboslute_OFDMSwingIdx[RF90_PATH_A] = %d\n", pDM_Odm->Aboslute_OFDMSwingIdx[RF90_PATH_A]));

			if (c.RfPathCount > 1) {
				ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD, ("deltaSwingTableIdx_TDOWN_B[%d] = %d\n", delta, down_b[delta]));

				pDM_Odm->RFCalibrateInfo.DeltaPowerIndexLast[RF90_PATH_B] = pDM_Odm->RFCalibrateInfo.DeltaPowerIndex[RF90_PATH_B];
				pDM_Odm->RFCalibrateInfo.DeltaPowerIndex[RF90_PATH_B] = -1 * down_b[delta];

				pDM_Odm->Aboslute_OFDMSwingIdx[RF90_PATH_B] =  -1 * down_b[delta];       /* Record delta swing for mix mode power tracking */

				ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD, ("******Temp is lower and pDM_Odm->Aboslute_OFDMSwingIdx[RF90_PATH_B] = %d\n", pDM_Odm->Aboslute_OFDMSwingIdx[RF90_PATH_B]));
			}
		}

	    for (p = RF90_PATH_A; p < c.RfPathCount; p++) {
			ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD, ("\n\n================================ [Path-%c] Calculating PowerIndexOffset ================================\n", (p == RF90_PATH_A ? 'A' : 'B')));
			if (pDM_Odm->RFCalibrateInfo.DeltaPowerIndex[p] == pDM_Odm->RFCalibrateInfo.DeltaPowerIndexLast[p])         /* If Thermal value changes but lookup table value still the same */
				pDM_Odm->RFCalibrateInfo.PowerIndexOffset[p] = 0;
			else
				pDM_Odm->RFCalibrateInfo.PowerIndexOffset[p] = pDM_Odm->RFCalibrateInfo.DeltaPowerIndex[p] - pDM_Odm->RFCalibrateInfo.DeltaPowerIndexLast[p];      /* Power Index Diff between 2 times Power Tracking */

			ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD, ("[Path-%c] PowerIndexOffset(%d) = DeltaPowerIndex(%d) - DeltaPowerIndexLast(%d)\n", (p == RF90_PATH_A ? 'A' : 'B'), pDM_Odm->RFCalibrateInfo.PowerIndexOffset[p], pDM_Odm->RFCalibrateInfo.DeltaPowerIndex[p],
			pDM_Odm->RFCalibrateInfo.DeltaPowerIndexLast[p]));

			pDM_Odm->RFCalibrateInfo.OFDM_index[p] = pDM_Odm->BbSwingIdxOfdmBase[p] + pDM_Odm->RFCalibrateInfo.PowerIndexOffset[p];
			pDM_Odm->RFCalibrateInfo.CCK_index = pDM_Odm->BbSwingIdxCckBase + pDM_Odm->RFCalibrateInfo.PowerIndexOffset[p];

			pDM_Odm->BbSwingIdxCck = pDM_Odm->RFCalibrateInfo.CCK_index;
			pDM_Odm->BbSwingIdxOfdm[p] = pDM_Odm->RFCalibrateInfo.OFDM_index[p];

	       /* *************Print BB Swing Base and Index Offset************* */

			ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD, ("The 'CCK' final index(%d) = BaseIndex(%d) + PowerIndexOffset(%d)\n", pDM_Odm->BbSwingIdxCck, pDM_Odm->BbSwingIdxCckBase, pDM_Odm->RFCalibrateInfo.PowerIndexOffset[p]));
			ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD, ("The 'OFDM' final index(%d) = BaseIndex[%c](%d) + PowerIndexOffset(%d)\n", pDM_Odm->BbSwingIdxOfdm[p], (p == RF90_PATH_A ? 'A' : 'B'), pDM_Odm->BbSwingIdxOfdmBase[p], pDM_Odm->RFCalibrateInfo.PowerIndexOffset[p]));

		    /* 4 7.1 Handle boundary conditions of index. */

			if (pDM_Odm->RFCalibrateInfo.OFDM_index[p] > c.SwingTableSize_OFDM-1) {
				pDM_Odm->RFCalibrateInfo.OFDM_index[p] = c.SwingTableSize_OFDM-1;
			} else if (pDM_Odm->RFCalibrateInfo.OFDM_index[p] < OFDM_min_index) {
				pDM_Odm->RFCalibrateInfo.OFDM_index[p] = OFDM_min_index;
			}
		}
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD, ("\n\n========================================================================================================\n"));
		if (pDM_Odm->RFCalibrateInfo.CCK_index > c.SwingTableSize_CCK-1)
			pDM_Odm->RFCalibrateInfo.CCK_index = c.SwingTableSize_CCK-1;
		/*
		 * else if (pDM_Odm->RFCalibrateInfo.CCK_index < 0)
		 * 	pDM_Odm->RFCalibrateInfo.CCK_index = 0;
		 */
	} else {
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD, ("The thermal meter is unchanged or TxPowerTracking OFF(%d): ThermalValue: %d , pDM_Odm->RFCalibrateInfo.ThermalValue: %d\n", pDM_Odm->RFCalibrateInfo.TxPowerTrackControl, ThermalValue, pDM_Odm->RFCalibrateInfo.ThermalValue));

		for (p = RF90_PATH_A; p < c.RfPathCount; p++)
			pDM_Odm->RFCalibrateInfo.PowerIndexOffset[p] = 0;
	}
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD, ("TxPowerTracking: [CCK] Swing Current Index: %d, Swing Base Index: %d\n", pDM_Odm->RFCalibrateInfo.CCK_index, pDM_Odm->BbSwingIdxCckBase));       /* Print Swing base & current */
	for (p = RF90_PATH_A; p < c.RfPathCount; p++) {
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD, ("TxPowerTracking: [OFDM] Swing Current Index: %d, Swing Base Index[%c]: %d\n", pDM_Odm->RFCalibrateInfo.OFDM_index[p], (p == RF90_PATH_A ? 'A' : 'B'), pDM_Odm->BbSwingIdxOfdmBase[p]));
	}

	if ((pDM_Odm->RFCalibrateInfo.PowerIndexOffset[RF90_PATH_A] != 0 ||  pDM_Odm->RFCalibrateInfo.PowerIndexOffset[RF90_PATH_B] != 0)
	 && pDM_Odm->RFCalibrateInfo.TxPowerTrackControl) {
		/* 4 7.2 Configure the Swing Table to adjust Tx Power. */
		pDM_Odm->RFCalibrateInfo.bTxPowerChanged = TRUE; /* Always TRUE after Tx Power is adjusted by power tracking. */
		/*
		 *  2012/04/2if ( MH According to Luke's suggestion, we can not write BB digital
		 *  to increase TX power. Otherwise, EVM will be bad.
		 *
		 *  2012/04/25 MH Add for tx power tracking to set tx power in tx agc for 88E.
		 */
		if (ThermalValue > pDM_Odm->RFCalibrateInfo.ThermalValue) {
			ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD, ("Temperature Increasing(A): delta_pi: %d , delta_t: %d, Now_t: %d, EFUSE_t: %d, Last_t: %d\n", pDM_Odm->RFCalibrateInfo.PowerIndexOffset[RF90_PATH_A], delta, ThermalValue, efuse->EEPROMThermalMeter, pDM_Odm->RFCalibrateInfo.ThermalValue));
			if (c.RfPathCount > 1)
				ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD, ("Temperature Increasing(B): delta_pi: %d , delta_t: %d, Now_t: %d, EFUSE_t: %d, Last_t: %d\n", pDM_Odm->RFCalibrateInfo.PowerIndexOffset[RF90_PATH_B], delta, ThermalValue, efuse->EEPROMThermalMeter, pDM_Odm->RFCalibrateInfo.ThermalValue));

			} else if (ThermalValue < pDM_Odm->RFCalibrateInfo.ThermalValue) { /* Low temperature */
				ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD, ("Temperature Decreasing(A): delta_pi: %d , delta_t: %d, Now_t: %d, EFUSE_t: %d, Last_t: %d\n", pDM_Odm->RFCalibrateInfo.PowerIndexOffset[RF90_PATH_A], delta, ThermalValue, efuse->EEPROMThermalMeter, pDM_Odm->RFCalibrateInfo.ThermalValue));

				if (c.RfPathCount > 1)
					ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD, ("Temperature Decreasing(B): delta_pi: %d , delta_t: %d, Now_t: %d, EFUSE_t: %d, Last_t: %d\n", pDM_Odm->RFCalibrateInfo.PowerIndexOffset[RF90_PATH_B], delta, ThermalValue, efuse->EEPROMThermalMeter, pDM_Odm->RFCalibrateInfo.ThermalValue));

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

			pDM_Odm->BbSwingIdxCckBase = pDM_Odm->BbSwingIdxCck;  	/* Record last time Power Tracking result as base. */
			for (p = RF90_PATH_A; p < c.RfPathCount; p++)
				pDM_Odm->BbSwingIdxOfdmBase[p] = pDM_Odm->BbSwingIdxOfdm[p];

			ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD,  ("pDM_Odm->RFCalibrateInfo.ThermalValue = %d ThermalValue= %d\n", pDM_Odm->RFCalibrateInfo.ThermalValue, ThermalValue));

			pDM_Odm->RFCalibrateInfo.ThermalValue = ThermalValue;     /* Record last Power Tracking Thermal Value */

	}
	if ((delta_IQK >= c.Threshold_IQK))	/* Delta temperature is equal to or larger than 20 centigrade (When threshold is 8). */
		(*c.DoIQK)(pDM_Odm, delta_IQK, ThermalValue, 8);

	ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD, ("<===ODM_TXPowerTrackingCallback_ThermalMeter\n"));

	pDM_Odm->RFCalibrateInfo.TXPowercount = 0;
}




/*
 * if (============================================================
 * if ( IQ Calibration
 * if (============================================================
 */


