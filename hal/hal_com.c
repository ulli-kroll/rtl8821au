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
#define _HAL_COM_C_

#include <drv_types.h>
#include <odm_precomp.h>

#define	EEPROM_CHANNEL_PLAN_BY_HW_MASK	0x80

/*
 * 	u8 hw_channel_plan,	channel plan from HW (efuse/eeprom)
 *	u8 sw_channel_plan,	channel plan from SW (registry/module param)
 *	u8 def_channel_plan,	channel plan used when the former two is invalid
 *	bool AutoLoadFail
 */

#define rtw_is_channel_plan_valid(chplan) (chplan<RT_CHANNEL_DOMAIN_MAX || \
					   chplan == RT_CHANNEL_DOMAIN_REALTEK_DEFINE)


u8 hal_com_get_channel_plan(struct rtl_priv *rtlpriv, u8 hw_channel_plan,	
			    u8 sw_channel_plan, u8 def_channel_plan,
			    bool AutoLoadFail)
{
	uint8_t swConfig;
	uint8_t chnlPlan;

	swConfig = _TRUE;
	if (!AutoLoadFail) {
		if (!rtw_is_channel_plan_valid(sw_channel_plan))
			swConfig = _FALSE;
		if (hw_channel_plan & EEPROM_CHANNEL_PLAN_BY_HW_MASK)
			swConfig = _FALSE;
	}

	if (swConfig == _TRUE)
		chnlPlan = sw_channel_plan;
	else
		chnlPlan = hw_channel_plan & (~EEPROM_CHANNEL_PLAN_BY_HW_MASK);

	if (!rtw_is_channel_plan_valid(chnlPlan))
		chnlPlan = def_channel_plan;

	return chnlPlan;
}

bool HAL_IsLegalChannel(struct rtl_priv *rtlpriv, uint32_t Channel)
{
	bool bLegalChannel = _TRUE;

	if (Channel > 14) {
		if(IsSupported5G(rtlpriv->registrypriv.wireless_mode) == _FALSE) {
			bLegalChannel = _FALSE;
			RT_TRACE(rtlpriv, COMP_ERR, DBG_LOUD, "Channel > 14 but wireless_mode do not support 5G\n");
		}
	} else if ((Channel <= 14) && (Channel >=1)){
		if(IsSupported24G(rtlpriv->registrypriv.wireless_mode) == _FALSE) {
			bLegalChannel = _FALSE;
			RT_TRACE(rtlpriv, COMP_ERR, DBG_LOUD, "(Channel <= 14) && (Channel >=1) but wireless_mode do not support 2.4G\n");
		}
	} else {
		bLegalChannel = _FALSE;
		RT_TRACE(rtlpriv, COMP_ERR, DBG_LOUD, "Channel is Invalid !!!\n");
	}

	return bLegalChannel;
}

void	HalSetBrateCfg(
	IN struct rtl_priv *	rtlpriv,
	IN uint8_t			*mBratesOS,
	OUT u16			*pBrateCfg)
{
	uint8_t	i, is_brate, brate;

	for(i=0;i<NDIS_802_11_LENGTH_RATES_EX;i++)
	{
		is_brate = mBratesOS[i] & IEEE80211_BASIC_RATE_MASK;
		brate = mBratesOS[i] & 0x7f;

		if( is_brate )
		{
			switch(brate)
			{
				case IEEE80211_CCK_RATE_1MB:	*pBrateCfg |= RATE_1M;	break;
				case IEEE80211_CCK_RATE_2MB:	*pBrateCfg |= RATE_2M;	break;
				case IEEE80211_CCK_RATE_5MB:	*pBrateCfg |= RATE_5_5M;break;
				case IEEE80211_CCK_RATE_11MB:	*pBrateCfg |= RATE_11M;	break;
				case IEEE80211_OFDM_RATE_6MB:	*pBrateCfg |= RATE_6M;	break;
				case IEEE80211_OFDM_RATE_9MB:	*pBrateCfg |= RATE_9M;	break;
				case IEEE80211_OFDM_RATE_12MB:	*pBrateCfg |= RATE_12M;	break;
				case IEEE80211_OFDM_RATE_18MB:	*pBrateCfg |= RATE_18M;	break;
				case IEEE80211_OFDM_RATE_24MB:	*pBrateCfg |= RATE_24M;	break;
				case IEEE80211_OFDM_RATE_36MB:	*pBrateCfg |= RATE_36M;	break;
				case IEEE80211_OFDM_RATE_48MB:	*pBrateCfg |= RATE_48M;	break;
				case IEEE80211_OFDM_RATE_54MB:	*pBrateCfg |= RATE_54M;	break;
			}
		}
	}
}

void hal_init_macaddr(struct rtl_priv *rtlpriv)
{
	rtlpriv->cfg->ops->set_hw_reg(rtlpriv, HW_VAR_ETHER_ADDR, rtlpriv->mac80211.mac_addr);
}

/*
* C2H event format:
* Field	 TRIGGER		CONTENT	   CMD_SEQ 	CMD_LEN		 CMD_ID
* BITS	 [127:120]	[119:16]      [15:8]		  [7:4]	 	   [3:0]
*/

void c2h_evt_clear(struct rtl_priv *rtlpriv)
{
	rtl_write_byte(rtlpriv, REG_C2HEVT_CLEAR, C2H_EVT_HOST_CLOSE);
}

int32_t c2h_evt_read(struct rtl_priv *rtlpriv, uint8_t *buf)
{
	int32_t ret = _FAIL;
	struct c2h_evt_hdr *c2h_evt;
	int i;
	uint8_t trigger;

	if (buf == NULL)
		goto exit;

	trigger = rtl_read_byte(rtlpriv, REG_C2HEVT_CLEAR);

	if (trigger == C2H_EVT_HOST_CLOSE) {
		goto exit; /* Not ready */
	} else if (trigger != C2H_EVT_FW_CLOSE) {
		goto clear_evt; /* Not a valid value */
	}

	c2h_evt = (struct c2h_evt_hdr *)buf;

	memset(c2h_evt, 0, 16);

	*buf = rtl_read_byte(rtlpriv, REG_C2HEVT_MSG_NORMAL);
	*(buf+1) = rtl_read_byte(rtlpriv, REG_C2HEVT_MSG_NORMAL + 1);

	RT_PRINT_DATA(rtlpriv, COMP_FW, DBG_DMESG, "c2h_evt_read(): ",
		&c2h_evt , sizeof(c2h_evt));

	if (0) {
		RT_TRACE(rtlpriv, COMP_FW, DBG_LOUD, "%s id:%u, len:%u, seq:%u, trigger:0x%02x\n", __func__
			, c2h_evt->id, c2h_evt->plen, c2h_evt->seq, trigger);
	}

	/* Read the content */
	for (i = 0; i < c2h_evt->plen; i++)
		c2h_evt->payload[i] = rtl_read_byte(rtlpriv, REG_C2HEVT_MSG_NORMAL + sizeof(*c2h_evt) + i);

	RT_PRINT_DATA(rtlpriv, COMP_FW, DBG_DMESG, "c2h_evt_read(): Command Content:\n",
		c2h_evt->payload, c2h_evt->plen);

	ret = _SUCCESS;

clear_evt:
	/*
	* Clear event to notify FW we have read the command.
	* If this field isn't clear, the FW won't update the next command message.
	*/
	c2h_evt_clear(rtlpriv);
exit:
	return ret;
}

uint8_t  rtw_hal_networktype_to_raid(struct rtl_priv *rtlpriv,unsigned char network_type)
{
	return networktype_to_raid_ex(rtlpriv,network_type);
}

uint8_t rtw_get_mgntframe_raid(struct rtl_priv *rtlpriv,unsigned char network_type)
{

	uint8_t raid;
	raid = (network_type & WIRELESS_11B) ?RATEID_IDX_B :RATEID_IDX_G;
	return raid;
}


