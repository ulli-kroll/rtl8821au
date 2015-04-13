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
#ifndef __RTL8812A_DM_H__
#define __RTL8812A_DM_H__

void rtl8812_init_dm_priv(IN struct rtl_priv *rtlpriv);
void rtl8812_deinit_dm_priv(IN struct rtl_priv *rtlpriv);
void rtl8812_InitHalDm(IN struct rtl_priv *rtlpriv);

//void rtl8192c_dm_CheckTXPowerTracking(IN struct rtl_priv *rtlpriv);

//void rtl8192c_dm_RF_Saving(IN struct rtl_priv *rtlpriv, IN uint8_t bForceInNormal);

#ifdef CONFIG_ANTENNA_DIVERSITY
void	AntDivCompare8812(struct rtl_priv *rtlpriv, WLAN_BSSID_EX *dst, WLAN_BSSID_EX *src);
uint8_t AntDivBeforeLink8812(struct rtl_priv *rtlpriv );
#endif
#endif

