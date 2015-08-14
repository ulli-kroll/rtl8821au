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

#define _RTL8812A_DM_C_

#include <rtl8812a_hal.h>
#include <../rtl8821au/dm.h>


/*
 * Initialize GPIO setting registers
 */

static void dm_InitGPIOSetting(struct rtl_priv *rtlpriv)
{
	uint8_t	tmp1byte;

	tmp1byte = rtl_read_byte(rtlpriv, REG_GPIO_MUXCFG);
	tmp1byte &= (GPIOSEL_GPIO | ~GPIOSEL_ENBT);

	rtl_write_byte(rtlpriv, REG_GPIO_MUXCFG, tmp1byte);

}

/*
 * ============================================================
 * functions
 * ============================================================
 */

static void Init_ODM_ComInfo_8812(struct rtl_priv *rtlpriv)
{
	struct _rtw_hal *pHalData = GET_HAL_DATA(rtlpriv);
	struct _rtw_dm *pDM_Odm = &(pHalData->odmpriv);

	/*
	 * Init Value
	 */

	memset(pDM_Odm,0,sizeof(*pDM_Odm));

	pDM_Odm->rtlpriv = rtlpriv;


	/* 1 ============== End of BoardType ============== */


	/* if(pHalData->AntDivCfg)
	 *	pdmpriv->InitODMFlag |= ODM_BB_ANT_DIV;
	 */
}
static void Update_ODM_ComInfo_8812(struct rtl_priv *rtlpriv)
{
	struct rtl_mac *mac = &(rtlpriv->mac80211);

	struct mlme_ext_priv	*pmlmeext = &rtlpriv->mlmeextpriv;
	struct mlme_priv	*pmlmepriv = &rtlpriv->mlmepriv;
	struct pwrctrl_priv *pwrctrlpriv = &rtlpriv->pwrctrlpriv;
	struct _rtw_hal *pHalData = GET_HAL_DATA(rtlpriv);
	struct _rtw_dm *	pDM_Odm = &(pHalData->odmpriv);
	int i;

	ODM_CmnInfoHook(pDM_Odm,ODM_CMNINFO_WM_MODE,&(pmlmeext->cur_wireless_mode));
	ODM_CmnInfoHook(pDM_Odm,ODM_CMNINFO_FORCED_RATE,&(pHalData->ForcedDataRate));

	ODM_CmnInfoHook(pDM_Odm,ODM_CMNINFO_SEC_MODE,&(rtlpriv->securitypriv.dot11PrivacyAlgrthm));
	ODM_CmnInfoHook(pDM_Odm,ODM_CMNINFO_SCAN,&(pmlmepriv->bScanInProcess));
	ODM_CmnInfoHook(pDM_Odm,ODM_CMNINFO_POWER_SAVING,&(pwrctrlpriv->bpower_saving));

	for (i = 0; i < NUM_STA; i++) {
		pDM_Odm->pODM_StaInfo[i] = NULL;
		/* pDM_Odm->pODM_StaInfo[i] = NULL; */
	}
}

void rtl8812_InitHalDm(struct rtl_priv *rtlpriv)
{
	uint8_t	i;

	dm_InitGPIOSetting(rtlpriv);

	rtlpriv->dm.dm_type = DM_Type_ByDriver;
	rtlpriv->dm.dm_flag = 0;

	Update_ODM_ComInfo_8812(rtlpriv);
	ODM_DMInit(rtlpriv);

	rtlpriv->fix_rate = 0xFF;
}


void rtl8812_init_dm_priv(IN struct rtl_priv *rtlpriv)
{
	struct _rtw_hal *pHalData = GET_HAL_DATA(rtlpriv);
	struct dm_priv	*pdmpriv = &pHalData->dmpriv;
	struct _rtw_dm *		podmpriv = &pHalData->odmpriv;
	memset(pdmpriv, 0, sizeof(struct dm_priv));

	/* spin_lock_init(&(pHalData->odm_stainfo_lock)); */

	Init_ODM_ComInfo_8812(rtlpriv);
	rtlpriv->registrypriv.bEn_RFE = 0;
}

void rtl8812_deinit_dm_priv(IN struct rtl_priv *rtlpriv)
{
	struct _rtw_hal *pHalData = GET_HAL_DATA(rtlpriv);
	struct dm_priv	*pdmpriv = &pHalData->dmpriv;
	struct _rtw_dm *		podmpriv = &pHalData->odmpriv;
}
