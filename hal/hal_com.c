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
#include <rtw_debug.h>
#include <odm_precomp.h>

#define	EEPROM_CHANNEL_PLAN_BY_HW_MASK	0x80

#undef RT_PRINT_DATA
static inline void RT_PRINT_DATA(int comp, int level, char *titlestring, 
			void *hexdata, int hexdatalen)
{
}


uint8_t	//return the final channel plan decision
hal_com_get_channel_plan(
	IN	struct rtl_priv *rtlpriv,
	IN	uint8_t			hw_channel_plan,	//channel plan from HW (efuse/eeprom)
	IN	uint8_t			sw_channel_plan,	//channel plan from SW (registry/module param)
	IN	uint8_t			def_channel_plan,	//channel plan used when the former two is invalid
	IN	BOOLEAN		AutoLoadFail
	)
{
	uint8_t swConfig;
	uint8_t chnlPlan;

	swConfig = _TRUE;
	if (!AutoLoadFail)
	{
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

BOOLEAN
HAL_IsLegalChannel(
	IN	struct rtl_priv *rtlpriv,
	IN	uint32_t			Channel
	)
{
	BOOLEAN bLegalChannel = _TRUE;

	if (Channel > 14) {
		if(IsSupported5G(rtlpriv->registrypriv.wireless_mode) == _FALSE) {
			bLegalChannel = _FALSE;
			DBG_871X("Channel > 14 but wireless_mode do not support 5G\n");
		}
	} else if ((Channel <= 14) && (Channel >=1)){
		if(IsSupported24G(rtlpriv->registrypriv.wireless_mode) == _FALSE) {
			bLegalChannel = _FALSE;
			DBG_871X("(Channel <= 14) && (Channel >=1) but wireless_mode do not support 2.4G\n");
		}
	} else {
		bLegalChannel = _FALSE;
		DBG_871X("Channel is Invalid !!!\n");
	}

	return bLegalChannel;
}

uint8_t	MRateToHwRate(uint8_t rate)
{
	uint8_t	ret = DESC_RATE1M;

	switch(rate)
	{
		// CCK and OFDM non-HT rates
	case IEEE80211_CCK_RATE_1MB:	ret = DESC_RATE1M;	break;
	case IEEE80211_CCK_RATE_2MB:	ret = DESC_RATE2M;	break;
	case IEEE80211_CCK_RATE_5MB:	ret = DESC_RATE5_5M;	break;
	case IEEE80211_CCK_RATE_11MB:	ret = DESC_RATE11M;	break;
	case IEEE80211_OFDM_RATE_6MB:	ret = DESC_RATE6M;	break;
	case IEEE80211_OFDM_RATE_9MB:	ret = DESC_RATE9M;	break;
	case IEEE80211_OFDM_RATE_12MB:	ret = DESC_RATE12M;	break;
	case IEEE80211_OFDM_RATE_18MB:	ret = DESC_RATE18M;	break;
	case IEEE80211_OFDM_RATE_24MB:	ret = DESC_RATE24M;	break;
	case IEEE80211_OFDM_RATE_36MB:	ret = DESC_RATE36M;	break;
	case IEEE80211_OFDM_RATE_48MB:	ret = DESC_RATE48M;	break;
	case IEEE80211_OFDM_RATE_54MB:	ret = DESC_RATE54M;	break;

		// HT rates since here
	//case MGN_MCS0:		ret = DESC_RATEMCS0;	break;
	//case MGN_MCS1:		ret = DESC_RATEMCS1;	break;
	//case MGN_MCS2:		ret = DESC_RATEMCS2;	break;
	//case MGN_MCS3:		ret = DESC_RATEMCS3;	break;
	//case MGN_MCS4:		ret = DESC_RATEMCS4;	break;
	//case MGN_MCS5:		ret = DESC_RATEMCS5;	break;
	//case MGN_MCS6:		ret = DESC_RATEMCS6;	break;
	//case MGN_MCS7:		ret = DESC_RATEMCS7;	break;

	default:		break;
	}

	return ret;
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

static void
_OneOutPipeMapping(
	IN	struct rtl_priv *rtlpriv
	)
{
	struct rtl_usb *pdvobjpriv = rtl_usbdev(rtlpriv);

	pdvobjpriv->Queue2Pipe[0] = pdvobjpriv->RtOutPipe[0];//VO
	pdvobjpriv->Queue2Pipe[1] = pdvobjpriv->RtOutPipe[0];//VI
	pdvobjpriv->Queue2Pipe[2] = pdvobjpriv->RtOutPipe[0];//BE
	pdvobjpriv->Queue2Pipe[3] = pdvobjpriv->RtOutPipe[0];//BK

	pdvobjpriv->Queue2Pipe[4] = pdvobjpriv->RtOutPipe[0];//BCN
	pdvobjpriv->Queue2Pipe[5] = pdvobjpriv->RtOutPipe[0];//MGT
	pdvobjpriv->Queue2Pipe[6] = pdvobjpriv->RtOutPipe[0];//HIGH
	pdvobjpriv->Queue2Pipe[7] = pdvobjpriv->RtOutPipe[0];//TXCMD
}

static void
_TwoOutPipeMapping(
	IN	struct rtl_priv *rtlpriv
	)
{
	struct rtl_usb	*pdvobjpriv = rtl_usbdev(rtlpriv);

	//BK, 	BE, 	VI, 	VO, 	BCN,	CMD,MGT,HIGH,HCCA
	//{  1, 	1, 	0, 	0, 	0, 	0, 	0, 	0, 		0	};
	//0:H, 1:N

	pdvobjpriv->Queue2Pipe[0] = pdvobjpriv->RtOutPipe[0];//VO
	pdvobjpriv->Queue2Pipe[1] = pdvobjpriv->RtOutPipe[0];//VI
	pdvobjpriv->Queue2Pipe[2] = pdvobjpriv->RtOutPipe[1];//BE
	pdvobjpriv->Queue2Pipe[3] = pdvobjpriv->RtOutPipe[1];//BK

	pdvobjpriv->Queue2Pipe[4] = pdvobjpriv->RtOutPipe[0];//BCN
	pdvobjpriv->Queue2Pipe[5] = pdvobjpriv->RtOutPipe[0];//MGT
	pdvobjpriv->Queue2Pipe[6] = pdvobjpriv->RtOutPipe[0];//HIGH
	pdvobjpriv->Queue2Pipe[7] = pdvobjpriv->RtOutPipe[0];//TXCMD
}

static void _ThreeOutPipeMapping(
	IN	struct rtl_priv *rtlpriv
	)
{
	struct rtl_usb	*pdvobjpriv = rtl_usbdev(rtlpriv);

		//	BK, 	BE, 	VI, 	VO, 	BCN,	CMD,MGT,HIGH,HCCA
		//{  2, 	2, 	1, 	0, 	0, 	0, 	0, 	0, 		0	};
		//0:H, 1:N, 2:L

	pdvobjpriv->Queue2Pipe[0] = pdvobjpriv->RtOutPipe[0];//VO
	pdvobjpriv->Queue2Pipe[1] = pdvobjpriv->RtOutPipe[1];//VI
	pdvobjpriv->Queue2Pipe[2] = pdvobjpriv->RtOutPipe[2];//BE
	pdvobjpriv->Queue2Pipe[3] = pdvobjpriv->RtOutPipe[2];//BK

	pdvobjpriv->Queue2Pipe[4] = pdvobjpriv->RtOutPipe[0];//BCN
	pdvobjpriv->Queue2Pipe[5] = pdvobjpriv->RtOutPipe[0];//MGT
	pdvobjpriv->Queue2Pipe[6] = pdvobjpriv->RtOutPipe[0];//HIGH
	pdvobjpriv->Queue2Pipe[7] = pdvobjpriv->RtOutPipe[0];//TXCMD

}
BOOLEAN
Hal_MappingOutPipe(
	IN	struct rtl_priv *rtlpriv,
	IN	uint8_t		NumOutPipe
	)
{
	struct registry_priv *pregistrypriv = &rtlpriv->registrypriv;

	BOOLEAN result = _TRUE;

	switch(NumOutPipe)
	{
		case 2:
			_TwoOutPipeMapping(rtlpriv);
			break;
		case 3:
		case 4:
			_ThreeOutPipeMapping(rtlpriv);
			break;
		case 1:
			_OneOutPipeMapping(rtlpriv);
			break;
		default:
			result = _FALSE;
			break;
	}

	return result;

}

void hal_init_macaddr(struct rtl_priv *rtlpriv)
{
	rtw_hal_set_hwreg(rtlpriv, HW_VAR_MAC_ADDR, rtlpriv->eeprompriv.mac_addr);
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

	RT_PRINT_DATA(_module_hal_init_c_, _drv_info_, "c2h_evt_read(): ",
		&c2h_evt , sizeof(c2h_evt));

	if (0) {
		DBG_871X("%s id:%u, len:%u, seq:%u, trigger:0x%02x\n", __func__
			, c2h_evt->id, c2h_evt->plen, c2h_evt->seq, trigger);
	}

	/* Read the content */
	for (i = 0; i < c2h_evt->plen; i++)
		c2h_evt->payload[i] = rtl_read_byte(rtlpriv, REG_C2HEVT_MSG_NORMAL + sizeof(*c2h_evt) + i);

	RT_PRINT_DATA(_module_hal_init_c_, _drv_info_, "c2h_evt_read(): Command Content:\n",
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


