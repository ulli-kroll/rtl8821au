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

uint rtw_hal_init(struct rtl_priv *rtlpriv);
uint rtw_hal_deinit(struct rtl_priv *rtlpriv);
void rtw_hal_stop(struct rtl_priv *rtlpriv);

void	rtw_hal_get_odm_var(struct rtl_priv *rtlpriv, HAL_ODM_VARIABLE eVariable, void *pValue1,bool bSet);

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

u32	rtl_get_bbreg(struct rtl_priv *rtlpriv, u32 RegAddr, u32 BitMask);
void	rtl_set_bbreg(struct rtl_priv *rtlpriv, u32 RegAddr, u32 BitMask, u32 Data);
u32	rtw_hal_read_rfreg(struct rtl_priv *rtlpriv, u32 eRFPath, u32 RegAddr, u32 BitMask);

int32_t	rtw_hal_interrupt_handler(struct rtl_priv *rtlpriv);

void	rtw_hal_set_bwmode(struct rtl_priv *rtlpriv, enum CHANNEL_WIDTH Bandwidth, uint8_t Offset);

int32_t rtw_hal_c2h_handler(struct rtl_priv *rtlpriv, struct c2h_evt_hdr *c2h_evt);
c2h_id_filter rtw_hal_c2h_id_filter_ccx(struct rtl_priv *rtlpriv);

#endif //__HAL_INTF_H__

