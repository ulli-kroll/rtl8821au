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
	RTW_PCIE	= BIT0,
	RTW_USB 	= BIT1,
	RTW_SDIO 	= BIT2,
};

enum _CHIP_TYPE {

	NULL_CHIP_TYPE,
	RTL8188C_8192C,
	RTL8723A,
	RTL8188E,
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
	HW_VAR_RF_TYPE,
	HW_VAR_DM_FLAG,
	HW_VAR_DM_FUNC_OP,
	HW_VAR_DM_FUNC_SET,
	HW_VAR_DM_FUNC_CLR,
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
	HW_VAR_SET_RPWM,
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
	HW_VAR_USB_MODE,
}HW_VARIABLES;

typedef enum _HAL_DEF_VARIABLE{
	HAL_DEF_UNDERCORATEDSMOOTHEDPWDB,
	HAL_DEF_IS_SUPPORT_ANT_DIV,
	HAL_DEF_CURRENT_ANTENNA,
	HAL_DEF_DRVINFO_SZ,
	HAL_DEF_MAX_RECVBUF_SZ,
	HAL_DEF_RX_PACKET_OFFSET,
	HAL_DEF_DBG_DUMP_RXPKT,//for dbg
	HAL_DEF_DBG_DM_FUNC,//for dbg
	HAL_DEF_RA_DECISION_RATE,
	HAL_DEF_RA_SGI,
	HAL_DEF_PT_PWR_STATUS,
	HAL_DEF_LDPC, 					// LDPC support
	HAL_DEF_TX_STBC, 				// TX STBC support
	HAL_DEF_RX_STBC, 				// RX STBC support
	HW_VAR_MAX_RX_AMPDU_FACTOR,
	HW_DEF_RA_INFO_DUMP,
	HAL_DEF_DBG_DUMP_TXPKT,
	HW_DEF_FA_CNT_DUMP,
	HW_DEF_ODM_DBG_FLAG,
	HAL_DEF_TX_PAGE_BOUNDARY,
	HAL_DEF_TX_PAGE_BOUNDARY_WOWLAN,
	HAL_DEF_ANT_DETECT,//to do for 8723a

}HAL_DEF_VARIABLE;

typedef enum _HAL_ODM_VARIABLE{
	HAL_ODM_STA_INFO,
	HAL_ODM_P2P_STATE,
	HAL_ODM_WIFI_DISPLAY_STATE,
}HAL_ODM_VARIABLE;

typedef enum _HAL_INTF_PS_FUNC{
	HAL_USB_SELECT_SUSPEND,
	HAL_MAX_ID,
}HAL_INTF_PS_FUNC;

typedef int32_t (*c2h_id_filter)(uint8_t id);

struct hal_ops {
	u32	(*hal_power_on)(struct _ADAPTER *padapter);
	u32	(*hal_init)(struct _ADAPTER *padapter);
	u32	(*hal_deinit)(struct _ADAPTER *padapter);

	void	(*free_hal_data)(struct _ADAPTER *padapter);

	u32	(*inirp_init)(struct _ADAPTER *padapter);
	u32	(*inirp_deinit)(struct _ADAPTER *padapter);

	int32_t	(*init_xmit_priv)(struct _ADAPTER *padapter);
	void	(*free_xmit_priv)(struct _ADAPTER *padapter);

	int32_t	(*init_recv_priv)(struct _ADAPTER *padapter);
	void	(*free_recv_priv)(struct _ADAPTER *padapter);

	void	(*InitSwLeds)(struct _ADAPTER *padapter);
	void	(*DeInitSwLeds)(struct _ADAPTER *padapter);

	void	(*dm_init)(struct _ADAPTER *padapter);
	void	(*dm_deinit)(struct _ADAPTER *padapter);
	void	(*read_chip_version)(struct _ADAPTER *padapter);

	void	(*init_default_value)(struct _ADAPTER *padapter);

	void	(*intf_chip_configure)(struct _ADAPTER *padapter);

	void	(*read_adapter_info)(struct _ADAPTER *padapter);

	void	(*enable_interrupt)(struct _ADAPTER *padapter);
	void	(*disable_interrupt)(struct _ADAPTER *padapter);
	int32_t	(*interrupt_handler)(struct _ADAPTER *padapter);

	void	(*set_bwmode_handler)(struct _ADAPTER *padapter, CHANNEL_WIDTH Bandwidth, uint8_t Offset);
	void	(*set_channel_handler)(struct _ADAPTER *padapter, uint8_t channel);
	void	(*set_chnl_bw_handler)(struct _ADAPTER *padapter, uint8_t channel, CHANNEL_WIDTH Bandwidth, uint8_t Offset40, uint8_t Offset80);

	void	(*hal_dm_watchdog)(struct _ADAPTER *padapter);

	void	(*SetHwRegHandler)(struct _ADAPTER *padapter, uint8_t	variable,u8* val);
	void	(*GetHwRegHandler)(struct _ADAPTER *padapter, uint8_t	variable,u8* val);

	uint8_t	(*GetHalDefVarHandler)(struct _ADAPTER *padapter, HAL_DEF_VARIABLE eVariable, PVOID pValue);
	uint8_t	(*SetHalDefVarHandler)(struct _ADAPTER *padapter, HAL_DEF_VARIABLE eVariable, PVOID pValue);

	void	(*GetHalODMVarHandler)(struct _ADAPTER *padapter, HAL_ODM_VARIABLE eVariable, PVOID pValue1,BOOLEAN bSet);
	void	(*SetHalODMVarHandler)(struct _ADAPTER *padapter, HAL_ODM_VARIABLE eVariable, PVOID pValue1,BOOLEAN bSet);

	void	(*UpdateRAMaskHandler)(struct _ADAPTER *padapter, u32 mac_id, uint8_t rssi_level);
	void	(*SetBeaconRelatedRegistersHandler)(struct _ADAPTER *padapter);

	void	(*Add_RateATid)(struct _ADAPTER *padapter, u32 bitmap, u8* arg, uint8_t rssi_level);
#ifdef CONFIG_ANTENNA_DIVERSITY
	uint8_t	(*AntDivBeforeLinkHandler)(struct _ADAPTER *padapter);
	void	(*AntDivCompareHandler)(struct _ADAPTER *padapter, WLAN_BSSID_EX *dst, WLAN_BSSID_EX *src);
#endif
	uint8_t	(*interface_ps_func)(struct _ADAPTER *padapter,HAL_INTF_PS_FUNC efunc_id, u8* val);

	int32_t	(*hal_xmit)(struct _ADAPTER *padapter, struct xmit_frame *pxmitframe);
	int32_t (*mgnt_xmit)(struct _ADAPTER *padapter, struct xmit_frame *pmgntframe);
	int32_t	(*hal_xmitframe_enqueue)(struct _ADAPTER *padapter, struct xmit_frame *pxmitframe);

	u32	(*read_bbreg)(struct _ADAPTER *padapter, u32 RegAddr, u32 BitMask);
	void	(*write_bbreg)(struct _ADAPTER *padapter, u32 RegAddr, u32 BitMask, u32 Data);
	u32	(*read_rfreg)(struct _ADAPTER *padapter, uint8_t eRFPath, u32 RegAddr, u32 BitMask);
	void	(*write_rfreg)(struct _ADAPTER *padapter, uint8_t eRFPath, u32 RegAddr, u32 BitMask, u32 Data);

	void (*EfusePowerSwitch)(struct _ADAPTER *padapter, uint8_t bWrite, uint8_t PwrState);
	void (*ReadEFuse)(struct _ADAPTER *padapter, uint8_t efuseType, u16 _offset, u16 _size_byte, uint8_t *pbuf);
	void (*EFUSEGetEfuseDefinition)(struct _ADAPTER *padapter, uint8_t efuseType, uint8_t type, void *pOut);
	u16	(*EfuseGetCurrentSize)(struct _ADAPTER *padapter, uint8_t efuseType);
	int 	(*Efuse_PgPacketRead)(struct _ADAPTER *padapter, uint8_t offset, uint8_t *data);
	int 	(*Efuse_PgPacketWrite)(struct _ADAPTER *padapter, uint8_t offset, uint8_t word_en, uint8_t *data);
	uint8_t	(*Efuse_WordEnableDataWrite)(struct _ADAPTER *padapter, u16 efuse_addr, uint8_t word_en, uint8_t *data);
	BOOLEAN	(*Efuse_PgPacketWrite_BT)(struct _ADAPTER *padapter, uint8_t offset, uint8_t word_en, uint8_t *data);

#ifdef DBG_CONFIG_ERROR_DETECT
	void (*sreset_init_value)(struct _ADAPTER *padapter);
	void (*sreset_reset_value)(struct _ADAPTER *padapter);
	void (*silentreset)(struct _ADAPTER *padapter);
	void (*sreset_xmit_status_check)(struct _ADAPTER *padapter);
	void (*sreset_linked_status_check) (struct _ADAPTER *padapter);
	uint8_t (*sreset_get_wifi_status)(struct _ADAPTER *padapter);
	bool (*sreset_inprogress)(struct _ADAPTER *padapter);
#endif
	void (*hal_notch_filter)(struct _ADAPTER * adapter, bool enable);
	void (*hal_reset_security_engine)(struct _ADAPTER * adapter);
	int32_t (*c2h_handler)(struct _ADAPTER *padapter, struct c2h_evt_hdr *c2h_evt);
	c2h_id_filter c2h_id_filter_ccx;
};

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

typedef enum _HARDWARE_TYPE{
//	NEW_GENERATION_IC
	HARDWARE_TYPE_RTL8812E,
	HARDWARE_TYPE_RTL8812AU,
	HARDWARE_TYPE_RTL8811AU,
	HARDWARE_TYPE_RTL8821E,
	HARDWARE_TYPE_RTL8821U,
	HARDWARE_TYPE_RTL8821S,

	HARDWARE_TYPE_MAX,
}HARDWARE_TYPE;

// RTL8812 Series
#define IS_HARDWARE_TYPE_8812E(_Adapter)		((_Adapter)->HardwareType==HARDWARE_TYPE_RTL8812E)
#define IS_HARDWARE_TYPE_8812AU(_Adapter)	((_Adapter)->HardwareType==HARDWARE_TYPE_RTL8812AU)
#define IS_HARDWARE_TYPE_8812(_Adapter)			\
(IS_HARDWARE_TYPE_8812E(_Adapter) || IS_HARDWARE_TYPE_8812AU(_Adapter))

// RTL8821 Series
#define IS_HARDWARE_TYPE_8821E(_Adapter)		((_Adapter)->HardwareType==HARDWARE_TYPE_RTL8821E)
#define IS_HARDWARE_TYPE_8811AU(_Adapter)		((_Adapter)->HardwareType==HARDWARE_TYPE_RTL8811AU)
#define IS_HARDWARE_TYPE_8821U(_Adapter)		((_Adapter)->HardwareType==HARDWARE_TYPE_RTL8821U ||\
	              								 (_Adapter)->HardwareType==HARDWARE_TYPE_RTL8811AU)
#define IS_HARDWARE_TYPE_8821S(_Adapter)		((_Adapter)->HardwareType==HARDWARE_TYPE_RTL8821S)
#define IS_HARDWARE_TYPE_8821(_Adapter)			\
(IS_HARDWARE_TYPE_8821E(_Adapter) || IS_HARDWARE_TYPE_8821U(_Adapter)|| IS_HARDWARE_TYPE_8821S(_Adapter))

#define IS_HARDWARE_TYPE_JAGUAR(_Adapter)		\
(IS_HARDWARE_TYPE_8812(_Adapter) || IS_HARDWARE_TYPE_8821(_Adapter))

typedef struct eeprom_priv EEPROM_EFUSE_PRIV, *PEEPROM_EFUSE_PRIV;
#define GET_EEPROM_EFUSE_PRIV(adapter) (&adapter->eeprompriv)
#define is_boot_from_eeprom(adapter) (adapter->eeprompriv.EepromOrEfuse)


void rtw_hal_def_value_init(struct _ADAPTER *padapter);

void	rtw_hal_free_data(struct _ADAPTER *padapter);

void rtw_hal_dm_init(struct _ADAPTER *padapter);
void rtw_hal_dm_deinit(struct _ADAPTER *padapter);
void rtw_hal_sw_led_init(struct _ADAPTER *padapter);
void rtw_hal_sw_led_deinit(struct _ADAPTER *padapter);

u32 rtw_hal_power_on(struct _ADAPTER *padapter);
uint rtw_hal_init(struct _ADAPTER *padapter);
uint rtw_hal_deinit(struct _ADAPTER *padapter);
void rtw_hal_stop(struct _ADAPTER *padapter);
void rtw_hal_set_hwreg(struct _ADAPTER *padapter, uint8_t variable, uint8_t *val);
void rtw_hal_get_hwreg(struct _ADAPTER *padapter, uint8_t variable, uint8_t *val);

void rtw_hal_chip_configure(struct _ADAPTER *padapter);
void rtw_hal_read_chip_info(struct _ADAPTER *padapter);
void rtw_hal_read_chip_version(struct _ADAPTER *padapter);

uint8_t rtw_hal_set_def_var(struct _ADAPTER *padapter, HAL_DEF_VARIABLE eVariable, PVOID pValue);
uint8_t rtw_hal_get_def_var(struct _ADAPTER *padapter, HAL_DEF_VARIABLE eVariable, PVOID pValue);

void rtw_hal_set_odm_var(struct _ADAPTER *padapter, HAL_ODM_VARIABLE eVariable, PVOID pValue1,BOOLEAN bSet);
void	rtw_hal_get_odm_var(struct _ADAPTER *padapter, HAL_ODM_VARIABLE eVariable, PVOID pValue1,BOOLEAN bSet);

void rtw_hal_enable_interrupt(struct _ADAPTER *padapter);
void rtw_hal_disable_interrupt(struct _ADAPTER *padapter);

u32	rtw_hal_inirp_init(struct _ADAPTER *padapter);
u32	rtw_hal_inirp_deinit(struct _ADAPTER *padapter);

uint8_t	rtw_hal_intf_ps_func(struct _ADAPTER *padapter,HAL_INTF_PS_FUNC efunc_id, u8* val);

int32_t	rtw_hal_xmitframe_enqueue(struct _ADAPTER *padapter, struct xmit_frame *pxmitframe);
int32_t	rtw_hal_xmit(struct _ADAPTER *padapter, struct xmit_frame *pxmitframe);
int32_t	rtw_hal_mgnt_xmit(struct _ADAPTER *padapter, struct xmit_frame *pmgntframe);

int32_t	rtw_hal_init_xmit_priv(struct _ADAPTER *padapter);
void	rtw_hal_free_xmit_priv(struct _ADAPTER *padapter);

int32_t	rtw_hal_init_recv_priv(struct _ADAPTER *padapter);
void	rtw_hal_free_recv_priv(struct _ADAPTER *padapter);

void rtw_hal_update_ra_mask(struct sta_info *psta, uint8_t rssi_level);
void	rtw_hal_add_ra_tid(struct _ADAPTER *padapter, u32 bitmap, u8* arg, uint8_t rssi_level);

void rtw_hal_bcn_related_reg_setting(struct _ADAPTER *padapter);

u32	rtw_hal_read_bbreg(struct _ADAPTER *padapter, u32 RegAddr, u32 BitMask);
void	rtw_hal_write_bbreg(struct _ADAPTER *padapter, u32 RegAddr, u32 BitMask, u32 Data);
u32	rtw_hal_read_rfreg(struct _ADAPTER *padapter, u32 eRFPath, u32 RegAddr, u32 BitMask);
void	rtw_hal_write_rfreg(struct _ADAPTER *padapter, u32 eRFPath, u32 RegAddr, u32 BitMask, u32 Data);

#define PHY_QueryBBReg(Adapter, RegAddr, BitMask) rtw_hal_read_bbreg((Adapter), (RegAddr), (BitMask))
#define PHY_SetBBReg(Adapter, RegAddr, BitMask, Data) rtw_hal_write_bbreg((Adapter), (RegAddr), (BitMask), (Data))
#define PHY_QueryRFReg(Adapter, eRFPath, RegAddr, BitMask) rtw_hal_read_rfreg((Adapter), (eRFPath), (RegAddr), (BitMask))
#define PHY_SetRFReg(Adapter, eRFPath, RegAddr, BitMask, Data) rtw_hal_write_rfreg((Adapter), (eRFPath), (RegAddr), (BitMask), (Data))

int32_t	rtw_hal_interrupt_handler(struct _ADAPTER *padapter);

void	rtw_hal_set_bwmode(struct _ADAPTER *padapter, CHANNEL_WIDTH Bandwidth, uint8_t Offset);
void	rtw_hal_set_chan(struct _ADAPTER *padapter, uint8_t channel);
void	rtw_hal_set_chnl_bw(struct _ADAPTER *padapter, uint8_t channel, CHANNEL_WIDTH Bandwidth, uint8_t Offset40, uint8_t Offset80);
void	rtw_hal_dm_watchdog(struct _ADAPTER *padapter);

#ifdef CONFIG_ANTENNA_DIVERSITY
uint8_t	rtw_hal_antdiv_before_linked(struct _ADAPTER *padapter);
void	rtw_hal_antdiv_rssi_compared(struct _ADAPTER *padapter, WLAN_BSSID_EX *dst, WLAN_BSSID_EX *src);
#endif


#ifdef DBG_CONFIG_ERROR_DETECT
void rtw_hal_sreset_init(struct _ADAPTER *padapter);
void rtw_hal_sreset_reset(struct _ADAPTER *padapter);
void rtw_hal_sreset_reset_value(struct _ADAPTER *padapter);
void rtw_hal_sreset_xmit_status_check(struct _ADAPTER *padapter);
void rtw_hal_sreset_linked_status_check (struct _ADAPTER *padapter);
uint8_t   rtw_hal_sreset_get_wifi_status(struct _ADAPTER *padapter);
bool rtw_hal_sreset_inprogress(struct _ADAPTER *padapter);
#endif

void rtw_hal_notch_filter(struct _ADAPTER * adapter, bool enable);
void rtw_hal_reset_security_engine(struct _ADAPTER * adapter);

int32_t rtw_hal_c2h_handler(struct _ADAPTER *adapter, struct c2h_evt_hdr *c2h_evt);
c2h_id_filter rtw_hal_c2h_id_filter_ccx(struct _ADAPTER *adapter);

#endif //__HAL_INTF_H__

