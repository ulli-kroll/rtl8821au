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
#ifndef __HAL_INTF_H__
#define __HAL_INTF_H__


enum RTL871X_HCI_TYPE {
	RTW_USB 	= BIT1,
};

enum _CHIP_TYPE {
	NULL_CHIP_TYPE,
	RTL8812,
	RTL8821, //RTL8811
	MAX_CHIP_TYPE
};



typedef enum _HAL_DEF_VARIABLE{
	HAL_DEF_UNDERCORATEDSMOOTHEDPWDB,
	HAL_DEF_IS_SUPPORT_ANT_DIV,
	HAL_DEF_CURRENT_ANTENNA,
	HAL_DEF_RA_DECISION_RATE,
	HAL_DEF_RA_SGI,
	HAL_DEF_LDPC, 					// LDPC support
	HW_VAR_MAX_RX_AMPDU_FACTOR,
	HAL_DEF_ANT_DETECT,//to do for 8723a

}HAL_DEF_VARIABLE;

typedef enum _HAL_ODM_VARIABLE{
	HAL_ODM_STA_INFO,
}HAL_ODM_VARIABLE;

typedef enum _HAL_INTF_PS_FUNC{
	HAL_USB_SELECT_SUSPEND,
	HAL_MAX_ID,
}HAL_INTF_PS_FUNC;

typedef int32_t (*c2h_id_filter)(uint8_t id);


typedef	enum _RT_EEPROM_TYPE{
	EEPROM_93C46,
	EEPROM_93C56,
	EEPROM_BOOT_EFUSE,
}RT_EEPROM_TYPE,*PRT_EEPROM_TYPE;



#define RF_CHANGE_BY_INIT	0
#define RF_CHANGE_BY_IPS 	BIT28
#define RF_CHANGE_BY_PS 	BIT29
#define RF_CHANGE_BY_HW 	BIT30
#define RF_CHANGE_BY_SW 	BIT31


typedef struct eeprom_priv EEPROM_EFUSE_PRIV, *PEEPROM_EFUSE_PRIV;
#define GET_EEPROM_EFUSE_PRIV(rtlpriv) (&rtlpriv->eeprompriv)


void rtw_hal_def_value_init(struct rtl_priv *rtlpriv);

void	rtw_hal_free_data(struct rtl_priv *rtlpriv);

void rtw_hal_dm_init(struct rtl_priv *rtlpriv);
void rtw_hal_dm_deinit(struct rtl_priv *rtlpriv);
void rtw_hal_sw_led_init(struct rtl_priv *rtlpriv);
void rtw_hal_sw_led_deinit(struct rtl_priv *rtlpriv);
void rtw_hal_led_control(struct rtl_priv *rtlpriv, enum led_ctl_mode LedAction);

uint rtw_hal_init(struct rtl_priv *rtlpriv);
uint rtw_hal_deinit(struct rtl_priv *rtlpriv);
void rtw_hal_stop(struct rtl_priv *rtlpriv);
void rtw_hal_set_hwreg(struct rtl_priv *rtlpriv, uint8_t variable, uint8_t *val);
void rtw_hal_get_hwreg(struct rtl_priv *rtlpriv, uint8_t variable, uint8_t *val);

void rtw_hal_read_chip_info(struct rtl_priv *rtlpriv);

uint8_t rtw_hal_get_def_var(struct rtl_priv *rtlpriv, HAL_DEF_VARIABLE eVariable, void *pValue);

void	rtw_hal_get_odm_var(struct rtl_priv *rtlpriv, HAL_ODM_VARIABLE eVariable, void *pValue1,BOOLEAN bSet);

void rtw_hal_enable_interrupt(struct rtl_priv *rtlpriv);
void rtw_hal_disable_interrupt(struct rtl_priv *rtlpriv);

u32	rtw_hal_inirp_deinit(struct rtl_priv *rtlpriv);

int32_t	rtw_hal_xmitframe_enqueue(struct rtl_priv *rtlpriv, struct xmit_frame *pxmitframe);
int32_t	rtw_hal_xmit(struct rtl_priv *rtlpriv, struct xmit_frame *pxmitframe);
int32_t	rtw_hal_mgnt_xmit(struct rtl_priv *rtlpriv, struct xmit_frame *pmgntframe);

int32_t	rtw_hal_init_xmit_priv(struct rtl_priv *rtlpriv);
void	rtw_hal_free_xmit_priv(struct rtl_priv *rtlpriv);

int32_t	rtw_hal_init_recv_priv(struct rtl_priv *rtlpriv);
void	rtw_hal_free_recv_priv(struct rtl_priv *rtlpriv);

void rtw_hal_update_ra_mask(struct rtl_priv *rtlpriv, struct sta_info *psta, uint8_t rssi_level);
void	rtw_hal_add_ra_tid(struct rtl_priv *rtlpriv, u32 bitmap, u8* arg, uint8_t rssi_level);

void rtw_hal_bcn_related_reg_setting(struct rtl_priv *rtlpriv);

u32	rtl_get_bbreg(struct rtl_priv *rtlpriv, u32 RegAddr, u32 BitMask);
void	rtl_set_bbreg(struct rtl_priv *rtlpriv, u32 RegAddr, u32 BitMask, u32 Data);
u32	rtw_hal_read_rfreg(struct rtl_priv *rtlpriv, u32 eRFPath, u32 RegAddr, u32 BitMask);

int32_t	rtw_hal_interrupt_handler(struct rtl_priv *rtlpriv);

void	rtw_hal_set_bwmode(struct rtl_priv *rtlpriv, enum CHANNEL_WIDTH Bandwidth, uint8_t Offset);
void	rtw_hal_set_chan(struct rtl_priv *rtlpriv, uint8_t channel);
void	rtw_hal_set_chnl_bw(struct rtl_priv *rtlpriv, uint8_t channel, enum CHANNEL_WIDTH Bandwidth, uint8_t Offset40, uint8_t Offset80);
void	rtw_hal_dm_watchdog(struct rtl_priv *rtlpriv);

void rtw_hal_notch_filter(struct rtl_priv * rtlpriv, bool enable);
void rtw_hal_reset_security_engine(struct rtl_priv * rtlpriv);

int32_t rtw_hal_c2h_handler(struct rtl_priv *rtlpriv, struct c2h_evt_hdr *c2h_evt);
c2h_id_filter rtw_hal_c2h_id_filter_ccx(struct rtl_priv *rtlpriv);

#endif //__HAL_INTF_H__

