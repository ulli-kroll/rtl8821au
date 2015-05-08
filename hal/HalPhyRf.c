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
void rtl8821au_dm_clean_txpower_tracking_state(struct rtl_priv *rtlpriv)
{
	struct rtl_efuse *efuse = rtl_efuse(rtlpriv);
	struct rtl_dm	*rtldm = rtl_dm(rtlpriv);
	u8 p = 0;

	rtldm->swing_idx_cck_base = rtldm->default_cck_index;
	rtldm->swing_idx_cck = rtldm->default_cck_index;
	rtldm->cck_index;

	for (p = RF90_PATH_A; p < MAX_RF_PATH; ++p) {
		rtldm->swing_idx_ofdm_base[p] = rtldm->default_ofdm_index;
		rtldm->swing_idx_ofdm[p] = rtldm->default_ofdm_index;
		rtldm->ofdm_index[p] = rtldm->default_ofdm_index;

		rtldm->power_index_offset[p] = 0;
		rtldm->delta_power_index[p] = 0;
		rtldm->delta_power_index_last[p] = 0;
		rtldm->power_index_offset[p] = 0;	/* ULLI Huh */

		rtldm->absolute_ofdm_swing_idx[p] = 0;    /* Initial Mix mode power tracking */
		rtldm->remnant_ofdm_swing_idx[p] = 0;
	}

	rtldm->modify_txagc_flag_path_a = FALSE;       /* Initial at Modify Tx Scaling Mode */
	rtldm->modify_txagc_flag_path_b = FALSE;       /* Initial at Modify Tx Scaling Mode */
	rtldm->remnant_cck_idx = 0;
	rtldm->thermalvalue = efuse->eeprom_thermalmeter;
	rtldm->thermalvalue_iqk = efuse->eeprom_thermalmeter;
	rtldm->thermalvalue_lck = efuse->eeprom_thermalmeter;
}



/*
 * if (============================================================
 * if ( IQ Calibration
 * if (============================================================
 */


