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
 *  include files
 * ============================================================
 */


#include "odm_precomp.h"
#include "../rtl8821au/phy.h"

void ODM_ConfigBBWithHeaderFile(struct _rtw_dm *pDM_Odm, ODM_BB_Config_Type ConfigType)
{
	struct rtl_priv *rtlpriv = pDM_Odm->rtlpriv;
	struct rtl_hal	*rtlhal = rtl_hal(rtlpriv);

	ODM_RT_TRACE(pDM_Odm, ODM_COMP_INIT, ODM_DBG_LOUD,
		("===>ODM_ConfigBBWithHeaderFile (%s)\n", (pDM_Odm->bIsMPChip) ? "MPChip" : "TestChip"));
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_INIT, ODM_DBG_LOUD,
		("pDM_Odm->SupportInterface: 0x%X, pDM_Odm->BoardType: 0x%X\n",
		rtlhal->SupportInterface, rtlhal->board_type));

	if (IS_HARDWARE_TYPE_8812AU(rtlhal)) {
		switch (ConfigType) {
		case CONFIG_BB_PHY_REG:
			ODM_ReadAndConfig_MP_8812A_PHY_REG(pDM_Odm);
			break;

		case CONFIG_BB_AGC_TAB:
			ODM_ReadAndConfig_MP_8812A_AGC_TAB(pDM_Odm);
			break;

		case CONFIG_BB_PHY_REG_PG:
			if (rtlhal->rfe_type == 3 && pDM_Odm->bIsMPChip)
				ODM_ReadAndConfig_MP_8812A_PHY_REG_PG_ASUS(pDM_Odm);
			else
				ODM_ReadAndConfig_MP_8812A_PHY_REG_PG(pDM_Odm);
			break;

		default:
			;
		}
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_INIT, ODM_DBG_LOUD, (" ===> phy_ConfigBBWithHeaderFile() phy:Rtl8812AGCTABArray\n"));
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_INIT, ODM_DBG_LOUD, (" ===> phy_ConfigBBWithHeaderFile() agc:Rtl8812PHY_REGArray\n"));
	}

	if (IS_HARDWARE_TYPE_8821U(rtlhal)) {
		switch (ConfigType) {
		case CONFIG_BB_PHY_REG:
			ODM_ReadAndConfig_MP_8821A_PHY_REG(pDM_Odm);
			break;

		case CONFIG_BB_AGC_TAB:
			ODM_ReadAndConfig_MP_8821A_AGC_TAB(pDM_Odm);
			break;

		case CONFIG_BB_PHY_REG_PG:
			ODM_ReadAndConfig_MP_8821A_PHY_REG_PG(pDM_Odm);
			break;

		default:
			;
		}
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_INIT, ODM_DBG_LOUD, (" ===> phy_ConfigBBWithHeaderFile() phy:Rtl8821AGCTABArray\n"));
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_INIT, ODM_DBG_LOUD, (" ===> phy_ConfigBBWithHeaderFile() agc:Rtl8821PHY_REGArray\n"));
	}
}

