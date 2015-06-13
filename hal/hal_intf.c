/******************************************************************************
 *
 * Copyright(c) 2007 - 2012 Realtek Corporation. All rights reserved.
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

#define _HAL_INTF_C_

#include <drv_types.h>
#include <rtw_debug.h>

void rtw_hal_chip_configure(struct rtl_priv *rtlpriv)
{
	if (rtlpriv->cfg->ops->intf_chip_configure)
		rtlpriv->cfg->ops->intf_chip_configure(rtlpriv);
}

void rtw_hal_read_chip_info(struct rtl_priv *rtlpriv)
{
	if (rtlpriv->cfg->ops->read_adapter_info)
		rtlpriv->cfg->ops->read_adapter_info(rtlpriv);
}

void rtw_hal_read_chip_version(struct rtl_priv *rtlpriv)
{
	if (rtlpriv->cfg->ops->read_chip_version)
		rtlpriv->cfg->ops->read_chip_version(rtlpriv);
}

void rtw_hal_def_value_init(struct rtl_priv *rtlpriv)
{
	if (rtlpriv->cfg->ops->init_default_value)
		rtlpriv->cfg->ops->init_default_value(rtlpriv);
}
void	rtw_hal_free_data(struct rtl_priv *rtlpriv)
{
	if (rtlpriv->cfg->ops->free_hal_data)
		rtlpriv->cfg->ops->free_hal_data(rtlpriv);
}
void	rtw_hal_dm_init(struct rtl_priv *rtlpriv)
{
	if (rtlpriv->cfg->ops->dm_init)
		rtlpriv->cfg->ops->dm_init(rtlpriv);
}
void rtw_hal_dm_deinit(struct rtl_priv *rtlpriv)
{
	/* cancel dm  timer */
	if (rtlpriv->cfg->ops->dm_deinit)
		rtlpriv->cfg->ops->dm_deinit(rtlpriv);
}
void rtw_hal_sw_led_init(struct rtl_priv *rtlpriv)
{
	if (rtlpriv->cfg->ops->init_sw_leds)
		rtlpriv->cfg->ops->init_sw_leds(rtlpriv);
}

void rtw_hal_sw_led_deinit(struct rtl_priv *rtlpriv)
{
	if (rtlpriv->cfg->ops->deinit_sw_leds)
		rtlpriv->cfg->ops->deinit_sw_leds(rtlpriv);
}

void rtw_hal_led_control(struct rtl_priv *rtlpriv, enum led_ctl_mode LedAction)
{
	rtlpriv->cfg->ops->led_control(rtlpriv, LedAction);
}


uint rtw_hal_init(struct rtl_priv *rtlpriv)
{
	uint status = _SUCCESS;

	rtlpriv->hw_init_completed = _FALSE;

	status = rtlpriv->cfg->ops->hal_init(rtlpriv);

	if (status == _SUCCESS) {
		rtlpriv->hw_init_completed = _TRUE;

		if (rtlpriv->registrypriv.notch_filter == 1)
			rtw_hal_notch_filter(rtlpriv, 1);

		rtw_hal_reset_security_engine(rtlpriv);

		rtw_hal_led_control(rtlpriv, LED_CTL_POWER_ON);
	} else {
		rtlpriv->hw_init_completed = _FALSE;
		DBG_871X("rtw_hal_init: hal__init fail\n");
	}

	return status;

}

uint rtw_hal_deinit(struct rtl_priv *rtlpriv)
{
	uint	status = _SUCCESS;

	status = rtlpriv->cfg->ops->hal_deinit(rtlpriv);

	if (status == _SUCCESS) {
		rtw_hal_led_control(rtlpriv, LED_CTL_POWER_OFF);
		rtlpriv->hw_init_completed = _FALSE;
	} else {
		DBG_871X("\n rtw_hal_deinit: hal_init fail\n");
	}

	return status;
}

void rtw_hal_set_hwreg(struct rtl_priv *rtlpriv, uint8_t variable, uint8_t *val)
{
	if (rtlpriv->cfg->ops->set_hw_reg)
		rtlpriv->cfg->ops->set_hw_reg(rtlpriv, variable, val);
}

void rtw_hal_get_hwreg(struct rtl_priv *rtlpriv, uint8_t variable, uint8_t *val)
{
	if (rtlpriv->cfg->ops->get_hw_reg)
		rtlpriv->cfg->ops->get_hw_reg(rtlpriv, variable, val);
}

uint8_t rtw_hal_set_def_var(struct rtl_priv *rtlpriv, HAL_DEF_VARIABLE eVariable, void *pValue)
{
	if (rtlpriv->cfg->ops->SetHalDefVarHandler)
		return rtlpriv->cfg->ops->SetHalDefVarHandler(rtlpriv, eVariable, pValue);
	return _FAIL;
}
uint8_t rtw_hal_get_def_var(struct rtl_priv *rtlpriv, HAL_DEF_VARIABLE eVariable, void *pValue)
{
	if (rtlpriv->cfg->ops->GetHalDefVarHandler)
		return rtlpriv->cfg->ops->GetHalDefVarHandler(rtlpriv, eVariable, pValue);
	return _FAIL;
}

void rtw_hal_set_odm_var(struct rtl_priv *rtlpriv, HAL_ODM_VARIABLE eVariable, void *pValue1, BOOLEAN bSet)
{
	if (rtlpriv->cfg->ops->SetHalODMVarHandler)
		rtlpriv->cfg->ops->SetHalODMVarHandler(rtlpriv, eVariable, pValue1, bSet);
}

void rtw_hal_enable_interrupt(struct rtl_priv *rtlpriv)
{
	if (rtlpriv->cfg->ops->enable_interrupt)
		rtlpriv->cfg->ops->enable_interrupt(rtlpriv);
	else
		DBG_871X("%s: cfg->ops.enable_interrupt is NULL!\n", __func__);

}
void rtw_hal_disable_interrupt(struct rtl_priv *rtlpriv)
{
	if (rtlpriv->cfg->ops->disable_interrupt)
		rtlpriv->cfg->ops->disable_interrupt(rtlpriv);
	else
		DBG_871X("%s: cfg->ops.disable_interrupt is NULL!\n", __func__);
}


uint32_t	rtw_hal_inirp_init(struct rtl_priv *rtlpriv)
{
	uint32_t rst = _FAIL;

	if (rtlpriv->cfg->ops->inirp_init)
		rst = rtlpriv->cfg->ops->inirp_init(rtlpriv);
	else
		DBG_871X(" %s cfg->ops.inirp_init is NULL!!!\n", __func__);

	return rst;
}

uint32_t	rtw_hal_inirp_deinit(struct rtl_priv *rtlpriv)
{
	if (rtlpriv->cfg->ops->inirp_deinit)
		return rtlpriv->cfg->ops->inirp_deinit(rtlpriv);

	return _FAIL;

}

int32_t	rtw_hal_xmitframe_enqueue(struct rtl_priv *rtlpriv, struct xmit_frame *pxmitframe)
{
	if (rtlpriv->cfg->ops->hal_xmitframe_enqueue)
		return rtlpriv->cfg->ops->hal_xmitframe_enqueue(rtlpriv, pxmitframe);

	return _FALSE;
}

int32_t	rtw_hal_xmit(struct rtl_priv *rtlpriv, struct xmit_frame *pxmitframe)
{
	if (rtlpriv->cfg->ops->hal_xmit)
		return rtlpriv->cfg->ops->hal_xmit(rtlpriv, pxmitframe);

	return _FALSE;
}

int32_t	rtw_hal_mgnt_xmit(struct rtl_priv *rtlpriv, struct xmit_frame *pmgntframe)
{
	int32_t ret = _FAIL;

	update_mgntframe_attrib_addr(rtlpriv, pmgntframe);

	if (rtlpriv->cfg->ops->mgnt_xmit)
		ret = rtlpriv->cfg->ops->mgnt_xmit(rtlpriv, pmgntframe);
	return ret;
}

int32_t	rtw_hal_init_xmit_priv(struct rtl_priv *rtlpriv)
{
	if (rtlpriv->cfg->ops->init_xmit_priv != NULL)
		return rtlpriv->cfg->ops->init_xmit_priv(rtlpriv);
	return _FAIL;
}

void	rtw_hal_free_xmit_priv(struct rtl_priv *rtlpriv)
{
	if (rtlpriv->cfg->ops->free_xmit_priv != NULL)
		rtlpriv->cfg->ops->free_xmit_priv(rtlpriv);
}

int32_t	rtw_hal_init_recv_priv(struct rtl_priv *rtlpriv)
{
	if (rtlpriv->cfg->ops->init_recv_priv)
		return rtlpriv->cfg->ops->init_recv_priv(rtlpriv);

	return _FAIL;
}

void	rtw_hal_free_recv_priv(struct rtl_priv *rtlpriv)
{

	if (rtlpriv->cfg->ops->free_recv_priv)
		rtlpriv->cfg->ops->free_recv_priv(rtlpriv);
}

void rtw_hal_update_ra_mask(struct sta_info *psta, uint8_t rssi_level)
{
	struct rtl_priv *rtlpriv;
	struct mlme_priv *pmlmepriv;

	if (!psta)
		return;

	rtlpriv = psta->rtlpriv;

	pmlmepriv = &(rtlpriv->mlmepriv);

	if (check_fwstate(pmlmepriv, WIFI_AP_STATE) == _TRUE) {
		add_RATid(rtlpriv, psta, rssi_level);
	} else {
		if (rtlpriv->cfg->ops->UpdateRAMaskHandler)
			rtlpriv->cfg->ops->UpdateRAMaskHandler(rtlpriv, psta->mac_id, rssi_level);
	}
}

void rtw_hal_add_ra_tid(struct rtl_priv *rtlpriv, uint32_t bitmap, u8 *arg, uint8_t rssi_level)
{
	if (rtlpriv->cfg->ops->Add_RateATid)
		rtlpriv->cfg->ops->Add_RateATid(rtlpriv, bitmap, arg, rssi_level);
}


u32 rtl_get_bbreg(struct rtl_priv *rtlpriv, uint32_t RegAddr, uint32_t BitMask)
{
	return rtlpriv->cfg->ops->get_bbreg(rtlpriv, RegAddr, BitMask);
}

void rtl_set_bbreg(struct rtl_priv *rtlpriv, uint32_t RegAddr, uint32_t BitMask, uint32_t Data)
{
	rtlpriv->cfg->ops->set_bbreg(rtlpriv, RegAddr, BitMask, Data);
}

uint32_t rtw_hal_read_rfreg(struct rtl_priv *rtlpriv, uint32_t eRFPath, uint32_t RegAddr, uint32_t BitMask)
{
	return rtlpriv->cfg->ops->get_rfreg(rtlpriv, eRFPath, RegAddr, BitMask);
}
void	rtw_hal_write_rfreg(struct rtl_priv *rtlpriv, uint32_t eRFPath, uint32_t RegAddr, uint32_t BitMask, uint32_t Data)
{
	rtlpriv->cfg->ops->set_rfreg(rtlpriv, eRFPath, RegAddr, BitMask, Data);
}

int32_t	rtw_hal_interrupt_handler(struct rtl_priv *rtlpriv)
{
	if (rtlpriv->cfg->ops->interrupt_handler)
		return rtlpriv->cfg->ops->interrupt_handler(rtlpriv);
	return _FAIL;
}

void	rtw_hal_set_bwmode(struct rtl_priv *rtlpriv, enum CHANNEL_WIDTH Bandwidth, uint8_t Offset)
{
	if (rtlpriv->cfg->ops->set_bwmode_handler)
		rtlpriv->cfg->ops->set_bwmode_handler(rtlpriv, Bandwidth, Offset);
}

void	rtw_hal_set_chan(struct rtl_priv *rtlpriv, uint8_t channel)
{
	if (rtlpriv->cfg->ops->set_channel_handler)
		rtlpriv->cfg->ops->set_channel_handler(rtlpriv, channel);
}

void	rtw_hal_set_chnl_bw(struct rtl_priv *rtlpriv, uint8_t channel, enum CHANNEL_WIDTH Bandwidth, uint8_t Offset40, uint8_t Offset80)
{
	if (rtlpriv->cfg->ops->set_chnl_bw_handler)
		rtlpriv->cfg->ops->set_chnl_bw_handler(rtlpriv, channel, Bandwidth, Offset40, Offset80);
}

void	rtw_hal_dm_watchdog(struct rtl_priv *rtlpriv)
{
	if (rtlpriv->cfg->ops->hal_dm_watchdog)
		rtlpriv->cfg->ops->hal_dm_watchdog(rtlpriv);
}

void rtw_hal_bcn_related_reg_setting(struct rtl_priv *rtlpriv)
{
	if (rtlpriv->cfg->ops->SetBeaconRelatedRegistersHandler)
		rtlpriv->cfg->ops->SetBeaconRelatedRegistersHandler(rtlpriv);
}


#ifdef CONFIG_ANTENNA_DIVERSITY
uint8_t	rtw_hal_antdiv_before_linked(struct rtl_priv *rtlpriv)
{
	if (rtlpriv->cfg->ops->AntDivBeforeLinkHandler)
		return rtlpriv->cfg->ops->AntDivBeforeLinkHandler(rtlpriv);
	return _FALSE;
}

void rtw_hal_antdiv_rssi_compared(struct rtl_priv *rtlpriv, WLAN_BSSID_EX *dst, WLAN_BSSID_EX *src)
{
	if (rtlpriv->cfg->ops->AntDivCompareHandler)
		rtlpriv->cfg->ops->AntDivCompareHandler(rtlpriv, dst, src);
}
#endif

void rtw_hal_notch_filter(struct rtl_priv *rtlpriv, bool enable)
{
	if (rtlpriv->cfg->ops->hal_notch_filter)
		rtlpriv->cfg->ops->hal_notch_filter(rtlpriv, enable);
}

void rtw_hal_reset_security_engine(struct rtl_priv *rtlpriv)
{
	if (rtlpriv->cfg->ops->hal_reset_security_engine)
		rtlpriv->cfg->ops->hal_reset_security_engine(rtlpriv);
}

int32_t rtw_hal_c2h_handler(struct rtl_priv *rtlpriv, struct c2h_evt_hdr *c2h_evt)
{
	int32_t ret = _FAIL;

	if (rtlpriv->cfg->ops->c2h_handler)
		ret = rtlpriv->cfg->ops->c2h_handler(rtlpriv, c2h_evt);
	return ret;
}

c2h_id_filter rtw_hal_c2h_id_filter_ccx(struct rtl_priv *rtlpriv)
{
	return rtlpriv->cfg->ops->c2h_id_filter_ccx;
}

