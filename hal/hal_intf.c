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
#include <rtl8812a_hal.h>

uint rtw_hal_init(struct rtl_priv *rtlpriv)
{
	uint status = _SUCCESS;

	rtlpriv->hw_init_completed = false;

	status = rtlpriv->cfg->ops->hw_init(rtlpriv);

	if (status == _SUCCESS) {
		rtlpriv->hw_init_completed = true;
	} else {
		rtlpriv->hw_init_completed = false;
		RT_TRACE(rtlpriv, COMP_ERR, DBG_LOUD, "rtw_hal_init: hal__init fail\n");
	}

	return status;

}

uint rtw_hal_deinit(struct rtl_priv *rtlpriv)
{
	uint	status = _SUCCESS;

	status = rtlpriv->cfg->ops->hal_deinit(rtlpriv);

	if (status == _SUCCESS) {
		rtlpriv->hw_init_completed = false;
	} else {
		RT_TRACE(rtlpriv, COMP_ERR, DBG_LOUD, "\n rtw_hal_deinit: hal_init fail\n");
	}

	return status;
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

	return false;
}

int32_t	rtw_hal_xmit(struct rtl_priv *rtlpriv, struct xmit_frame *pxmitframe)
{
	if (rtlpriv->cfg->ops->hal_xmit)
		return rtlpriv->cfg->ops->hal_xmit(rtlpriv, pxmitframe);

	return false;
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

void rtw_hal_update_ra_mask(struct rtl_priv *rtlpriv, struct sta_info *psta, 
			    uint8_t rssi_level)
{
	struct mlme_priv *pmlmepriv;

	if (!psta)
		return;

	pmlmepriv = &(rtlpriv->mlmepriv);

	if (check_fwstate(pmlmepriv, WIFI_AP_STATE) == true) {
		add_RATid(rtlpriv, psta, rssi_level);
	} else {
		UpdateHalRAMask8812A(rtlpriv, psta->mac_id, rssi_level);
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

void rtw_hal_bcn_related_reg_setting(struct rtl_priv *rtlpriv)
{
	if (rtlpriv->cfg->ops->SetBeaconRelatedRegistersHandler)
		rtlpriv->cfg->ops->SetBeaconRelatedRegistersHandler(rtlpriv);
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

