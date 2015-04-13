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
void ODM_ClearTxPowerTrackingState(struct _rtw_dm *pDM_Odm)
{
	struct rtl_efuse *efuse = rtl_efuse(pDM_Odm->rtlpriv);
	struct rtl_dm	*rtldm = rtl_dm(pDM_Odm->rtlpriv);
	struct _rtw_hal *pHalData = GET_HAL_DATA(pDM_Odm->rtlpriv);
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



/*
 * if (============================================================
 * if ( IQ Calibration
 * if (============================================================
 */


