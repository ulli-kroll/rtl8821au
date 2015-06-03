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


typedef enum _HW_VARIABLES{
	HW_VAR_MEDIA_STATUS,
	HW_VAR_MEDIA_STATUS1,
	HW_VAR_SET_OPMODE,
	HW_VAR_MAC_ADDR,
	HW_VAR_BSSID,
	HW_VAR_INIT_RTS_RATE,
	HW_VAR_BASIC_RATE,
	HW_VAR_TXPAUSE,
	HW_VAR_BCN_FUNC,
	HW_VAR_CORRECT_TSF,
	HW_VAR_CHECK_BSSID,
	HW_VAR_MLME_DISCONNECT,
	HW_VAR_MLME_SITESURVEY,
	HW_VAR_MLME_JOIN,
	HW_VAR_ON_RCR_AM,
       HW_VAR_OFF_RCR_AM,
	HW_VAR_BEACON_INTERVAL,
	HW_VAR_SLOT_TIME,
	HW_VAR_RESP_SIFS,
	HW_VAR_ACK_PREAMBLE,
	HW_VAR_SEC_CFG,
	HW_VAR_BCN_VALID,
	HW_VAR_CAM_EMPTY_ENTRY,
	HW_VAR_CAM_INVALID_ALL,
	HW_VAR_CAM_WRITE,
	HW_VAR_CAM_READ,
	HW_VAR_AC_PARAM_VO,
	HW_VAR_AC_PARAM_VI,
	HW_VAR_AC_PARAM_BE,
	HW_VAR_AC_PARAM_BK,
	HW_VAR_ACM_CTRL,
	HW_VAR_AMPDU_MIN_SPACE,
	HW_VAR_AMPDU_FACTOR,
	HW_VAR_RXDMA_AGG_PG_TH,
	HW_VAR_H2C_FW_PWRMODE,
	HW_VAR_H2C_PS_TUNE_PARAM,
	HW_VAR_H2C_FW_JOINBSSRPT,
	HW_VAR_FWLPS_RF_ON,
	HW_VAR_H2C_FW_P2P_PS_OFFLOAD,
	HW_VAR_TDLS_WRCR,
	HW_VAR_TDLS_INIT_CH_SEN,
	HW_VAR_TDLS_RS_RCR,
	HW_VAR_TDLS_DONE_CH_SEN,
	HW_VAR_INITIAL_GAIN,
	HW_VAR_TRIGGER_GPIO_0,
	HW_VAR_BT_SET_COEXIST,
	HW_VAR_BT_ISSUE_DELBA,
	HW_VAR_CURRENT_ANTENNA,
	HW_VAR_ANTENNA_DIVERSITY_LINK,
	HW_VAR_ANTENNA_DIVERSITY_SELECT,
	HW_VAR_SWITCH_EPHY_WoWLAN,
	HW_VAR_EFUSE_USAGE,
	HW_VAR_EFUSE_BYTES,
	HW_VAR_EFUSE_BT_USAGE,
	HW_VAR_EFUSE_BT_BYTES,
	HW_VAR_FIFO_CLEARN_UP,
	HW_VAR_CHECK_TXBUF,
	HW_VAR_APFM_ON_MAC, //Auto FSM to Turn On, include clock, isolation, power control for MAC only
	// The valid upper nav range for the HW updating, if the true value is larger than the upper range, the HW won't update it.
	// Unit in microsecond. 0 means disable this function.
	HW_VAR_NAV_UPPER,
	HW_VAR_C2H_HANDLE,
	HW_VAR_RPT_TIMER_SETTING,
	HW_VAR_TX_RPT_MAX_MACID,
	HW_VAR_H2C_MEDIA_STATUS_RPT,
	HW_VAR_CHK_HI_QUEUE_EMPTY,
	HW_VAR_DL_BCN_SEL,
	HW_VAR_AMPDU_MAX_TIME,
	HW_VAR_WIRELESS_MODE,
}HW_VARIABLES;

typedef enum _HAL_DEF_VARIABLE{
	HAL_DEF_UNDERCORATEDSMOOTHEDPWDB,
	HAL_DEF_IS_SUPPORT_ANT_DIV,
	HAL_DEF_CURRENT_ANTENNA,
	HAL_DEF_DRVINFO_SZ,
	HAL_DEF_MAX_RECVBUF_SZ,
	HAL_DEF_RX_PACKET_OFFSET,
	HAL_DEF_RA_DECISION_RATE,
	HAL_DEF_RA_SGI,
	HAL_DEF_PT_PWR_STATUS,
	HAL_DEF_LDPC, 					// LDPC support
	HAL_DEF_TX_STBC, 				// TX STBC support
	HAL_DEF_RX_STBC, 				// RX STBC support
	HW_VAR_MAX_RX_AMPDU_FACTOR,
	HW_DEF_RA_INFO_DUMP,
	HW_DEF_FA_CNT_DUMP,
	HW_DEF_ODM_DBG_FLAG,
	HAL_DEF_TX_PAGE_BOUNDARY,
	HAL_DEF_TX_PAGE_BOUNDARY_WOWLAN,
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
#define is_boot_from_eeprom(rtlpriv) (rtlpriv->eeprompriv.EepromOrEfuse)


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

void rtw_hal_chip_configure(struct rtl_priv *rtlpriv);
void rtw_hal_read_chip_info(struct rtl_priv *rtlpriv);
void rtw_hal_read_chip_version(struct rtl_priv *rtlpriv);

uint8_t rtw_hal_set_def_var(struct rtl_priv *rtlpriv, HAL_DEF_VARIABLE eVariable, void *pValue);
uint8_t rtw_hal_get_def_var(struct rtl_priv *rtlpriv, HAL_DEF_VARIABLE eVariable, void *pValue);

void rtw_hal_set_odm_var(struct rtl_priv *rtlpriv, HAL_ODM_VARIABLE eVariable, void *pValue1,BOOLEAN bSet);
void	rtw_hal_get_odm_var(struct rtl_priv *rtlpriv, HAL_ODM_VARIABLE eVariable, void *pValue1,BOOLEAN bSet);

void rtw_hal_enable_interrupt(struct rtl_priv *rtlpriv);
void rtw_hal_disable_interrupt(struct rtl_priv *rtlpriv);

u32	rtw_hal_inirp_init(struct rtl_priv *rtlpriv);
u32	rtw_hal_inirp_deinit(struct rtl_priv *rtlpriv);

int32_t	rtw_hal_xmitframe_enqueue(struct rtl_priv *rtlpriv, struct xmit_frame *pxmitframe);
int32_t	rtw_hal_xmit(struct rtl_priv *rtlpriv, struct xmit_frame *pxmitframe);
int32_t	rtw_hal_mgnt_xmit(struct rtl_priv *rtlpriv, struct xmit_frame *pmgntframe);

int32_t	rtw_hal_init_xmit_priv(struct rtl_priv *rtlpriv);
void	rtw_hal_free_xmit_priv(struct rtl_priv *rtlpriv);

int32_t	rtw_hal_init_recv_priv(struct rtl_priv *rtlpriv);
void	rtw_hal_free_recv_priv(struct rtl_priv *rtlpriv);

void rtw_hal_update_ra_mask(struct sta_info *psta, uint8_t rssi_level);
void	rtw_hal_add_ra_tid(struct rtl_priv *rtlpriv, u32 bitmap, u8* arg, uint8_t rssi_level);

void rtw_hal_bcn_related_reg_setting(struct rtl_priv *rtlpriv);

u32	rtl_get_bbreg(struct rtl_priv *rtlpriv, u32 RegAddr, u32 BitMask);
void	rtl_set_bbreg(struct rtl_priv *rtlpriv, u32 RegAddr, u32 BitMask, u32 Data);
u32	rtw_hal_read_rfreg(struct rtl_priv *rtlpriv, u32 eRFPath, u32 RegAddr, u32 BitMask);
void	rtw_hal_write_rfreg(struct rtl_priv *rtlpriv, u32 eRFPath, u32 RegAddr, u32 BitMask, u32 Data);

int32_t	rtw_hal_interrupt_handler(struct rtl_priv *rtlpriv);

void	rtw_hal_set_bwmode(struct rtl_priv *rtlpriv, enum CHANNEL_WIDTH Bandwidth, uint8_t Offset);
void	rtw_hal_set_chan(struct rtl_priv *rtlpriv, uint8_t channel);
void	rtw_hal_set_chnl_bw(struct rtl_priv *rtlpriv, uint8_t channel, enum CHANNEL_WIDTH Bandwidth, uint8_t Offset40, uint8_t Offset80);
void	rtw_hal_dm_watchdog(struct rtl_priv *rtlpriv);

#ifdef CONFIG_ANTENNA_DIVERSITY
uint8_t	rtw_hal_antdiv_before_linked(struct rtl_priv *rtlpriv);
void	rtw_hal_antdiv_rssi_compared(struct rtl_priv *rtlpriv, WLAN_BSSID_EX *dst, WLAN_BSSID_EX *src);
#endif

void rtw_hal_notch_filter(struct rtl_priv * rtlpriv, bool enable);
void rtw_hal_reset_security_engine(struct rtl_priv * rtlpriv);

int32_t rtw_hal_c2h_handler(struct rtl_priv *rtlpriv, struct c2h_evt_hdr *c2h_evt);
c2h_id_filter rtw_hal_c2h_id_filter_ccx(struct rtl_priv *rtlpriv);

#endif //__HAL_INTF_H__

