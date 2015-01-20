#ifndef __WIFI_H__
#define __WIFI_H__

#include <linux/types.h>

enum radio_path {
	RF90_PATH_A = 0,
	RF90_PATH_B = 1,
	RF90_PATH_C = 2,
	RF90_PATH_D = 3,
};

struct rtl_hal_ops;


/* Here will go the new Halinterface data */
#define CHANNEL_MAX_NUMBER			14+24+21	// 14 is the max channel number
#define CHANNEL_MAX_NUMBER_2G		14
#define CHANNEL_MAX_NUMBER_5G		54			// Please refer to "phy_GetChnlGroup8812A" and "Hal_ReadTxPowerInfo8812A"
#define CHANNEL_MAX_NUMBER_5G_80M	7
#define CHANNEL_GROUP_MAX				3+9	// ch1~3, ch4~9, ch10~14 total three groups
#define MAX_PG_GROUP					13

#define MAX_REGULATION_NUM						3
#define MAX_RF_PATH_NUM_IN_POWER_LIMIT_TABLE	4
#define MAX_2_4G_BANDWITH_NUM					2
#define MAX_2_4G_RATE_SECTION_NUM				3
#define MAX_2_4G_CHANNEL_NUM						5 // adopt channel group instead of individual channel
#define MAX_5G_BANDWITH_NUM						4
#define MAX_5G_RATE_SECTION_NUM					4
#define MAX_5G_CHANNEL_NUM						14 // adopt channel group instead of individual channel

/*
 * ULLI : big remark
 * ULLI : in rtl8821ae there is no use of external_pa_2g or
 * ULLI : external_pa_5g in the phy.c file.
 * ULLI : But we need to configure the external_pa* here, because
 * ULLI : it is used here in the original source, and we don't want to 
 * ULLI : drop this.
 * ULLI :
 * ULLI : Thus we do transmit some values use in rtl_dm (now _rtw_dm) and
 * ULLI : dm_priv into appreciate struct's (rtl_phy, rtl_hal, rtl_efuse)
 * ULLI : and use them, to select features.
 */


struct rtl_efuse {
	//
	// EEPROM setting.
	//
	u16	eeprom_vid;
	u16	eeprom_svid;
	u16	EEPROMPID;
	u16	EEPROMSDID;

	uint8_t	EEPROMCustomerID;
	uint8_t	EEPROMSubCustomerID;
	uint8_t	eeprom_version;
	uint8_t	EEPROMRegulatory;
	uint8_t	EEPROMThermalMeter;
	uint8_t	EEPROMBluetoothCoexist;
	uint8_t	EEPROMBluetoothType;
	uint8_t	EEPROMBluetoothAntNum;
	uint8_t	EEPROMBluetoothAntIsolation;
	uint8_t	EEPROMBluetoothRadioShared;
	
	// For power group
	/* ULLI both vars are only read ??? */
	uint8_t	pwrgroup_ht20[RF_PATH_MAX_92C_88E][CHANNEL_MAX_NUMBER];
	uint8_t	pwrgroup_ht40[RF_PATH_MAX_92C_88E][CHANNEL_MAX_NUMBER];
	
	//---------------------------------------------------------------------------------//
	//3 [2.4G]
	uint8_t	Index24G_CCK_Base[MAX_RF_PATH][CHANNEL_MAX_NUMBER];
	uint8_t	Index24G_BW40_Base[MAX_RF_PATH][CHANNEL_MAX_NUMBER];
	//If only one tx, only BW20 and OFDM are used.
	s8	CCK_24G_Diff[MAX_RF_PATH][MAX_TX_COUNT];
	s8	OFDM_24G_Diff[MAX_RF_PATH][MAX_TX_COUNT];
	s8	BW20_24G_Diff[MAX_RF_PATH][MAX_TX_COUNT];
	s8	BW40_24G_Diff[MAX_RF_PATH][MAX_TX_COUNT];
	//3 [5G]
	u8	txpwr_5g_bw40base[MAX_RF_PATH][CHANNEL_MAX_NUMBER];
	u8	txpwr_5g_bw80base[MAX_RF_PATH][CHANNEL_MAX_NUMBER_5G_80M];
	s8	txpwr_5g_ofdmdiff[MAX_RF_PATH][MAX_TX_COUNT];
	s8	txpwr_5g_bw20diff[MAX_RF_PATH][MAX_TX_COUNT];
	s8	txpwr_5g_bw40diff[MAX_RF_PATH][MAX_TX_COUNT];
	s8	txpwr_5g_bw80diff[MAX_RF_PATH][MAX_TX_COUNT];
	
};

struct rtl_hal {
	u16	hw_type;

	u8 pa_type_2g;
	u8 pa_type_5g;
	u8 lna_type_2g;
	u8 lna_type_5g;
	u8 external_pa_2g;
	u8 external_lna_2g;
	u8 external_pa_5g;
	u8 external_lna_5g;
	

	/*firmware */
	u32 fwsize;
	u8 *pfirmware;
	u16 fw_version;
	u16 fw_subversion;
	bool h2c_setinprogress;
	u8 last_hmeboxnum;
	bool fw_ready;
};

struct rtl_phy {
	u8	rf_type;
	
	u8	current_chan_bw;
	
	u8	current_channel;

	// The current Tx Power Level 
	/* ULLI vars currently not used */
	uint8_t	cur_cck_txpwridx;
	uint8_t	cur_ofdm2g_txpwridx;
	uint8_t	cur_bw20_txpwridx;
	uint8_t	cur_bw40_txpwridx;

	uint8_t	txpwr_limit_2_4g[MAX_REGULATION_NUM]
				[MAX_2_4G_BANDWITH_NUM]
	                        [MAX_2_4G_RATE_SECTION_NUM]
	                        [MAX_2_4G_CHANNEL_NUM]
				[MAX_RF_PATH_NUM];

	// Power Limit Table for 5G
	uint8_t	txpwr_limit_5g[MAX_REGULATION_NUM]
				[MAX_5G_BANDWITH_NUM]
				[MAX_5G_RATE_SECTION_NUM]
				[MAX_5G_CHANNEL_NUM]
				[MAX_RF_PATH_NUM];
};


struct rtl_dm {
};

#define rtl_hal(rtlpriv)	(&((rtlpriv)->rtlhal))
#define rtl_efuse(rtlpriv)	(&((rtlpriv)->efuse))
#define rtl_phy(rtlpriv)	(&((rtlpriv)->phy))
#define rtl_dm(rtlpriv)		(&((rtlpriv)->dm))

struct rtl_priv {
	struct net_device *ndev;

	struct rtl_hal rtlhal;		/* Caution new Hal data */
	struct rtl_phy phy;
	struct rtl_efuse efuse;
	struct rtl_dm dm;		/* Caution new dm data */
	
	
	struct _rtw_hal *HalData;

	
	
	
	int	DriverState;// for disable driver using module, use dongle to replace module.
	int	pid[3];//process id from UI, 0:wps, 1:hostapd, 2:dhcpcd
	int	bDongle;//build-in module or external dongle
	u16 	chip_type;
	u16	interface_type;//USB,SDIO,SPI,PCI

	struct dvobj_priv *dvobj;
	struct	mlme_priv mlmepriv;
	struct	mlme_ext_priv mlmeextpriv;
	struct	cmd_priv	cmdpriv;
	struct	evt_priv	evtpriv;
	//struct	io_queue	*pio_queue;
	struct 	io_priv	iopriv;
	struct	xmit_priv	xmitpriv;
	struct	recv_priv	recvpriv;
	struct	sta_priv	stapriv;
	struct	security_priv	securitypriv;
	struct	registry_priv	registrypriv;
	struct	pwrctrl_priv	pwrctrlpriv;
	struct 	eeprom_priv eeprompriv;
	struct	led_priv	ledpriv;

#ifdef CONFIG_AP_MODE
	struct	hostapd_priv	*phostapdpriv;
#endif

	u32	setband;
	struct rtl_hal_ops	*HalFunc;

	int32_t	bDriverStopped;
	int32_t	bSurpriseRemoved;
	int32_t  bCardDisableWOHSM;

	u32	IsrContent;
	u32	ImrContent;

	uint8_t	EepromAddressSize;
	uint8_t	hw_init_completed;
	uint8_t	bDriverIsGoingToUnload;
	uint8_t	init_adpt_in_progress;
	uint8_t	bHaltInProgress;

	_thread_hdl_ cmdThread;
	_thread_hdl_ evtThread;
	_thread_hdl_ xmitThread;
	_thread_hdl_ recvThread;

#ifndef PLATFORM_LINUX
	NDIS_STATUS (*dvobj_init)(struct dvobj_priv *dvobj);
	void (*dvobj_deinit)(struct dvobj_priv *dvobj);
#endif

	void (*intf_start)(struct rtl_priv * adapter);
	void (*intf_stop)(struct rtl_priv * adapter);


#ifdef PLATFORM_LINUX
	int bup;
	struct net_device_stats stats;
	struct iw_statistics iwstats;
	struct proc_dir_entry *dir_dev;// for proc directory

#endif //end of PLATFORM_LINUX

	int net_closed;

	uint8_t bFWReady;
	uint8_t bBTFWReady;
	uint8_t bReadPortCancel;
	uint8_t bWritePortCancel;
	uint8_t bLinkInfoDump;
	//	Added by Albert 2012/10/26
	//	The driver will show up the desired channel number when this flag is 1.
	uint8_t bNotifyChannelChange;
#ifdef CONFIG_AUTOSUSPEND
	uint8_t	bDisableAutosuspend;
#endif
        uint8_t    fix_rate;

	unsigned char     in_cta_test;

};




struct rtl_hal_ops {
	/*
	 * New HAL functions with struct net_device  as first param
	 * this can be (hopefully)switched to struct ieee80211_hw
	 */
	int (*init_sw_vars) (struct net_device *ndev);
	void (*deinit_sw_vars) (struct net_device *ndev);

	void	(*init_sw_leds)(struct rtl_priv *rtlpriv);
	
	void	(*set_hw_reg)(struct rtl_priv *rtlpriv, u8 variable,u8 *val);
	void	(*get_hw_reg)(struct rtl_priv *rtlpriv, u8 variable,u8 *val);
/*
 * 	ULLI from original rtlwifi-lib in wifi.h 
 * 
 * 	void (*fill_fake_txdesc) (struct ieee80211_hw *hw, u8 *pDesc,
 *				  u32 buffer_len, bool bIsPsPoll);
*/	
	
	void	(*fill_fake_txdesc) (struct rtl_priv *rtlpriv, u8 *pDesc,
				     u32 BufferLen, u8 IsPsPoll, u8 IsBTQosNull);
	
	u32	(*get_bbreg)(struct rtl_priv *rtlpriv, u32 RegAddr, u32 BitMask);
	void	(*set_bbreg)(struct rtl_priv *rtlpriv, u32 RegAddr, u32 BitMask, u32 Data);
	u32	(*get_rfreg)(struct rtl_priv *rtlpriv, u32 eRFPath, u32 RegAddr, u32 BitMask);
	void	(*set_rfreg)(struct rtl_priv *rtlpriv, u32 eRFPath, u32 RegAddr, u32 BitMask, u32 Data);

	/* Old HAL functions */

	u32	(*hal_init)(struct rtl_priv *rtlpriv);
	u32	(*hal_deinit)(struct rtl_priv *rtlpriv);

	void	(*free_hal_data)(struct rtl_priv *rtlpriv);

	u32	(*inirp_init)(struct rtl_priv *rtlpriv);
	u32	(*inirp_deinit)(struct rtl_priv *rtlpriv);

	int32_t	(*init_xmit_priv)(struct rtl_priv *rtlpriv);
	void	(*free_xmit_priv)(struct rtl_priv *rtlpriv);

	int32_t	(*init_recv_priv)(struct rtl_priv *rtlpriv);
	void	(*free_recv_priv)(struct rtl_priv *rtlpriv);

	void	(*DeInitSwLeds)(struct rtl_priv *rtlpriv);

	void	(*dm_init)(struct rtl_priv *rtlpriv);
	void	(*dm_deinit)(struct rtl_priv *rtlpriv);
	void	(*read_chip_version)(struct rtl_priv *rtlpriv);

	void	(*init_default_value)(struct rtl_priv *rtlpriv);

	void	(*intf_chip_configure)(struct rtl_priv *rtlpriv);

	void	(*read_adapter_info)(struct rtl_priv *rtlpriv);

	void	(*enable_interrupt)(struct rtl_priv *rtlpriv);
	void	(*disable_interrupt)(struct rtl_priv *rtlpriv);
	int32_t	(*interrupt_handler)(struct rtl_priv *rtlpriv);

	void	(*set_bwmode_handler)(struct rtl_priv *rtlpriv, enum CHANNEL_WIDTH Bandwidth, uint8_t Offset);
	void	(*set_channel_handler)(struct rtl_priv *rtlpriv, uint8_t channel);
	void	(*set_chnl_bw_handler)(struct rtl_priv *rtlpriv, uint8_t channel, enum CHANNEL_WIDTH Bandwidth, uint8_t Offset40, uint8_t Offset80);

	void	(*hal_dm_watchdog)(struct rtl_priv *rtlpriv);

	uint8_t	(*GetHalDefVarHandler)(struct rtl_priv *rtlpriv, HAL_DEF_VARIABLE eVariable, PVOID pValue);
	uint8_t	(*SetHalDefVarHandler)(struct rtl_priv *rtlpriv, HAL_DEF_VARIABLE eVariable, PVOID pValue);

	void	(*GetHalODMVarHandler)(struct rtl_priv *rtlpriv, HAL_ODM_VARIABLE eVariable, PVOID pValue1,BOOLEAN bSet);
	void	(*SetHalODMVarHandler)(struct rtl_priv *rtlpriv, HAL_ODM_VARIABLE eVariable, PVOID pValue1,BOOLEAN bSet);

	void	(*UpdateRAMaskHandler)(struct rtl_priv *rtlpriv, u32 mac_id, uint8_t rssi_level);
	void	(*SetBeaconRelatedRegistersHandler)(struct rtl_priv *rtlpriv);

	void	(*Add_RateATid)(struct rtl_priv *rtlpriv, u32 bitmap, u8* arg, uint8_t rssi_level);
#ifdef CONFIG_ANTENNA_DIVERSITY
	uint8_t	(*AntDivBeforeLinkHandler)(struct rtl_priv *rtlpriv);
	void	(*AntDivCompareHandler)(struct rtl_priv *rtlpriv, WLAN_BSSID_EX *dst, WLAN_BSSID_EX *src);
#endif
	uint8_t	(*interface_ps_func)(struct rtl_priv *rtlpriv,HAL_INTF_PS_FUNC efunc_id, u8* val);

	int32_t	(*hal_xmit)(struct rtl_priv *rtlpriv, struct xmit_frame *pxmitframe);
	int32_t (*mgnt_xmit)(struct rtl_priv *rtlpriv, struct xmit_frame *pmgntframe);
	int32_t	(*hal_xmitframe_enqueue)(struct rtl_priv *rtlpriv, struct xmit_frame *pxmitframe);

	void (*EfusePowerSwitch)(struct rtl_priv *rtlpriv, uint8_t bWrite, uint8_t PwrState);
	void (*ReadEFuse)(struct rtl_priv *rtlpriv, uint8_t efuseType, u16 _offset, u16 _size_byte, uint8_t *pbuf);
	void (*EFUSEGetEfuseDefinition)(struct rtl_priv *rtlpriv, uint8_t efuseType, uint8_t type, void *pOut);
	u16	(*EfuseGetCurrentSize)(struct rtl_priv *rtlpriv, uint8_t efuseType);
	int 	(*Efuse_PgPacketRead)(struct rtl_priv *rtlpriv, uint8_t offset, uint8_t *data);
	int 	(*Efuse_PgPacketWrite)(struct rtl_priv *rtlpriv, uint8_t offset, uint8_t word_en, uint8_t *data);
	uint8_t	(*Efuse_WordEnableDataWrite)(struct rtl_priv *rtlpriv, u16 efuse_addr, uint8_t word_en, uint8_t *data);
	BOOLEAN	(*Efuse_PgPacketWrite_BT)(struct rtl_priv *rtlpriv, uint8_t offset, uint8_t word_en, uint8_t *data);

#ifdef DBG_CONFIG_ERROR_DETECT
	void (*sreset_init_value)(struct rtl_priv *rtlpriv);
	void (*sreset_reset_value)(struct rtl_priv *rtlpriv);
	void (*silentreset)(struct rtl_priv *rtlpriv);
	void (*sreset_xmit_status_check)(struct rtl_priv *rtlpriv);
	void (*sreset_linked_status_check) (struct rtl_priv *rtlpriv);
	uint8_t (*sreset_get_wifi_status)(struct rtl_priv *rtlpriv);
	bool (*sreset_inprogress)(struct rtl_priv *rtlpriv);
#endif
	void (*hal_notch_filter)(struct rtl_priv * adapter, bool enable);
	void (*hal_reset_security_engine)(struct rtl_priv * adapter);
	int32_t (*c2h_handler)(struct rtl_priv *rtlpriv, struct c2h_evt_hdr *c2h_evt);
	c2h_id_filter c2h_id_filter_ccx;
};


static void rtw_hal_fill_fake_txdesc (struct rtl_priv *rtlpriv, u8 *pDesc,
	u32 BufferLen, u8 IsPsPoll, u8 IsBTQosNull)
{
	rtlpriv->HalFunc->fill_fake_txdesc(rtlpriv, pDesc, BufferLen, IsPsPoll, IsBTQosNull);
}




/*Only for transition between old (RTW) and new (rtlwifi-lib) API */

uint32_t rtl8812au_hal_init(struct rtl_priv *Adapter);
uint32_t rtl8812au_hal_deinit(struct rtl_priv *Adapter);
unsigned int rtl8812au_inirp_init(struct rtl_priv *Adapter);
unsigned int rtl8812au_inirp_deinit(struct rtl_priv *Adapter);
void rtl8812au_init_default_value(struct rtl_priv *rtlpriv);
void rtl8812au_interface_configure(struct rtl_priv *rtlpriv);
void ReadAdapterInfo8812AU(struct rtl_priv *Adapter);
u8 GetHalDefVar8812AUsb(struct rtl_priv *Adapter, HAL_DEF_VARIABLE eVariable, PVOID pValue);
u8 SetHalDefVar8812AUsb(struct rtl_priv *Adapter, HAL_DEF_VARIABLE eVariable, PVOID pValue);
uint8_t rtl8812au_ps_func(struct rtl_priv *Adapter, HAL_INTF_PS_FUNC efunc_id, uint8_t *val);

static u8 rtl_read_byte(struct rtl_priv *adapter, u32 addr)
{
	uint8_t val;
	struct io_priv *pio_priv = &adapter->iopriv;
	struct intf_hdl *pintfhdl = &pio_priv->intf;

	val = pintfhdl->io_ops._read8(pintfhdl, addr);

	return val;
}

static u16 rtl_read_word(struct rtl_priv *adapter, u32 addr)
{
	u16 val;
	struct io_priv *pio_priv = &adapter->iopriv;
	struct intf_hdl	*pintfhdl = &pio_priv->intf;

	val = pintfhdl->io_ops._read16(pintfhdl, addr);

	return le16_to_cpu(val);
}

static u32 rtl_read_dword(struct rtl_priv *adapter, u32 addr)
{
	uint32_t val;
	struct io_priv *pio_priv = &adapter->iopriv;
	struct intf_hdl	*pintfhdl = &pio_priv->intf;

	val = pintfhdl->io_ops._read32(pintfhdl, addr);

	return le32_to_cpu(val);
}

static int rtl_write_byte(struct rtl_priv *adapter, u32 addr, u8 val)
{
	struct io_priv *pio_priv = &adapter->iopriv;
	struct	intf_hdl *pintfhdl = &(pio_priv->intf);
	int ret;

	ret = pintfhdl->io_ops._write8(pintfhdl, addr, val);

	return RTW_STATUS_CODE(ret);
}

static int rtl_write_word(struct rtl_priv *adapter, u32 addr, u16 val)
{
	struct io_priv *pio_priv = &adapter->iopriv;
	struct	intf_hdl *pintfhdl = &pio_priv->intf;
	int ret;

	val = cpu_to_le16(val);

	ret = pintfhdl->io_ops._write16(pintfhdl, addr, val);

	return RTW_STATUS_CODE(ret);
}

static int rtl_write_dword(struct rtl_priv *adapter, u32 addr, u32 val)
{
	struct io_priv *pio_priv = &adapter->iopriv;
	struct intf_hdl	*pintfhdl = &pio_priv->intf;
	int ret;

	val = cpu_to_le32(val);
	ret = pintfhdl->io_ops._write32(pintfhdl, addr, val);

	return RTW_STATUS_CODE(ret);
}

enum hardware_type {
	HARDWARE_TYPE_RTL8812E,
	HARDWARE_TYPE_RTL8812AU,
	HARDWARE_TYPE_RTL8811AU,
	HARDWARE_TYPE_RTL8821E,
	HARDWARE_TYPE_RTL8821U,
	HARDWARE_TYPE_RTL8821S,

	HARDWARE_TYPE_MAX,
};

// RTL8812 Series
#define IS_HARDWARE_TYPE_8812E(rtlhal)	\
	(rtlhal->hw_type == HARDWARE_TYPE_RTL8812E)
#define IS_HARDWARE_TYPE_8812AU(rtlhal)	\
	(rtlhal->hw_type == HARDWARE_TYPE_RTL8812AU)
#define IS_HARDWARE_TYPE_8812(rtlhal)	\
	(IS_HARDWARE_TYPE_8812E(rtlhal) || IS_HARDWARE_TYPE_8812AU(rtlhal))

// RTL8821 Series
#define IS_HARDWARE_TYPE_8821E(rtlhal)	\
	(rtlhal->hw_type == HARDWARE_TYPE_RTL8821E)
#define IS_HARDWARE_TYPE_8811AU(rtlhal)	\
	(rtlhal->hw_type == HARDWARE_TYPE_RTL8811AU)
#define IS_HARDWARE_TYPE_8821U(rtlhal)	\
	((rtlhal->hw_type == HARDWARE_TYPE_RTL8821U) || \
       	(rtlhal->hw_type == HARDWARE_TYPE_RTL8811AU))
#define IS_HARDWARE_TYPE_8821S(rtlhal)	\
	(rtlhal->hw_type == HARDWARE_TYPE_RTL8821S)
#define IS_HARDWARE_TYPE_8821(rtlhal)		\
	(IS_HARDWARE_TYPE_8821E(rtlhal) || \
	 IS_HARDWARE_TYPE_8821U(rtlhal) || \
	 IS_HARDWARE_TYPE_8821S(rtlhal))
	 
#define IS_HARDWARE_TYPE_JAGUAR(rtlhal)	\
	(IS_HARDWARE_TYPE_8812(rtlhal) || \
	 IS_HARDWARE_TYPE_8821(rtlhal))

//###### duplicate code,will move to ODM #########
#define IQK_MAC_REG_NUM		4
#define IQK_ADDA_REG_NUM		16

#define IQK_BB_REG_NUM			10
#define IQK_BB_REG_NUM_92C	9
#define IQK_BB_REG_NUM_92D	10
#define IQK_BB_REG_NUM_test	6

#define IQK_Matrix_Settings_NUM_92D	1+24+21

#define HP_THERMAL_NUM		8

struct dm_priv {
	uint8_t	DM_Type;
	uint8_t	DMFlag;
	uint8_t	InitDMFlag;
	//uint8_t   RSVD_1;

	u32	InitODMFlag;
	//* Upper and Lower Signal threshold for Rate Adaptive*/
	int	UndecoratedSmoothedPWDB;
	int	UndecoratedSmoothedCCK;
	int	EntryMinUndecoratedSmoothedPWDB;
	int	EntryMaxUndecoratedSmoothedPWDB;
	int	MinUndecoratedPWDBForDM;
	int	LastMinUndecoratedPWDBForDM;

	int32_t	UndecoratedSmoothedBeacon;

//###### duplicate code,will move to ODM #########
	//for High Power
	uint8_t 	bDynamicTxPowerEnable;
	uint8_t 	LastDTPLvl;
	uint8_t	DynamicTxHighPowerLvl;//Add by Jacken Tx Power Control for Near/Far Range 2008/03/06

	//for tx power tracking
	uint8_t	bTXPowerTracking;
	uint8_t	TXPowercount;
	uint8_t	bTXPowerTrackingInit;
	uint8_t	TxPowerTrackControl;	//for mp mode, turn off txpwrtracking as default
	uint8_t	TM_Trigger;

	uint8_t	ThermalMeter[2];				// ThermalMeter, index 0 for RFIC0, and 1 for RFIC1
	uint8_t	ThermalValue;
	uint8_t	ThermalValue_LCK;
	uint8_t	ThermalValue_IQK;
	uint8_t	ThermalValue_DPK;
	uint8_t	bRfPiEnable;
	//uint8_t   RSVD_2;

	//for APK
	u32	APKoutput[2][2];	//path A/B; output1_1a/output1_2a
	uint8_t	bAPKdone;
	uint8_t	bAPKThermalMeterIgnore;
	uint8_t	bDPdone;
	uint8_t	bDPPathAOK;
	uint8_t	bDPPathBOK;
	//uint8_t   RSVD_3;
	//uint8_t   RSVD_4;
	//uint8_t   RSVD_5;

	//for IQK
	u32	ADDA_backup[IQK_ADDA_REG_NUM];
	u32	IQK_MAC_backup[IQK_MAC_REG_NUM];
	u32	IQK_BB_backup_recover[9];
	u32	IQK_BB_backup[IQK_BB_REG_NUM];

	uint8_t	PowerIndex_backup[6];
	uint8_t	OFDM_index[2];

	uint8_t	bCCKinCH14;
	uint8_t	CCK_index;
	uint8_t	bDoneTxpower;
	uint8_t	CCK_index_HP;

	uint8_t	OFDM_index_HP[2];
	uint8_t	ThermalValue_HP[HP_THERMAL_NUM];
	uint8_t	ThermalValue_HP_index;
	//uint8_t   RSVD_6;

	//for TxPwrTracking2
	int32_t	RegE94;
	int32_t  RegE9C;
	int32_t	RegEB4;
	int32_t	RegEBC;

	u32	TXPowerTrackingCallbackCnt;	//cosa add for debug

	u32	prv_traffic_idx; // edca turbo
//###### duplicate code,will move to ODM #########

	// Add for Reading Initial Data Rate SEL Register 0x484 during watchdog. Using for fill tx desc. 2011.3.21 by Thomas
	uint8_t	INIDATA_RATE[32];
};

//
// <Roger_Notes> For RTL8723 WiFi/BT/GPS multi-function configuration. 2010.10.06.
//
typedef enum _RT_MULTI_FUNC{
	RT_MULTI_FUNC_NONE	= 0x00,
	RT_MULTI_FUNC_WIFI 	= 0x01,
	RT_MULTI_FUNC_BT 		= 0x02,
	RT_MULTI_FUNC_GPS 	= 0x04,
}RT_MULTI_FUNC,*PRT_MULTI_FUNC;
//
// <Roger_Notes> For RTL8723 WiFi PDn/GPIO polarity control configuration. 2010.10.08.
//
typedef enum _RT_POLARITY_CTL {
	RT_POLARITY_LOW_ACT 	= 0,
	RT_POLARITY_HIGH_ACT 	= 1,
} RT_POLARITY_CTL, *PRT_POLARITY_CTL;

// For RTL8723 regulator mode. by tynli. 2011.01.14.
typedef enum _RT_REGULATOR_MODE {
	RT_SWITCHING_REGULATOR 	= 0,
	RT_LDO_REGULATOR 			= 1,
} RT_REGULATOR_MODE, *PRT_REGULATOR_MODE;

//
// Interface type.
//
typedef	enum _INTERFACE_SELECT_PCIE{
	INTF_SEL0_SOLO_MINICARD			= 0,		// WiFi solo-mCard
	INTF_SEL1_BT_COMBO_MINICARD		= 1,		// WiFi+BT combo-mCard
	INTF_SEL2_PCIe						= 2,		// PCIe Card
} INTERFACE_SELECT_PCIE, *PINTERFACE_SELECT_PCIE;


typedef	enum _INTERFACE_SELECT_USB{
	INTF_SEL0_USB 				= 0,		// USB
	INTF_SEL1_USB_High_Power  	= 1,		// USB with high power PA
	INTF_SEL2_MINICARD		  	= 2,		// Minicard
	INTF_SEL3_USB_Solo 		= 3,		// USB solo-Slim module
	INTF_SEL4_USB_Combo		= 4,		// USB Combo-Slim module
	INTF_SEL5_USB_Combo_MF	= 5,		// USB WiFi+BT Multi-Function Combo, i.e., Proprietary layout(AS-VAU) which is the same as SDIO card
} INTERFACE_SELECT_USB, *PINTERFACE_SELECT_USB;

typedef enum _RT_AMPDU_BRUST_MODE{
	RT_AMPDU_BRUST_NONE 		= 0,
	RT_AMPDU_BRUST_92D 		= 1,
	RT_AMPDU_BRUST_88E 		= 2,
	RT_AMPDU_BRUST_8812_4 	= 3,
	RT_AMPDU_BRUST_8812_8 	= 4,
	RT_AMPDU_BRUST_8812_12 	= 5,
	RT_AMPDU_BRUST_8812_15	= 6,
	RT_AMPDU_BRUST_8723B	 	= 7,
}RT_AMPDU_BRUST,*PRT_AMPDU_BRUST_MODE;

#define MAX_BASE_NUM_IN_PHY_REG_PG_2_4G			4 //  CCK:1,OFDM:2, HT:2
#define MAX_BASE_NUM_IN_PHY_REG_PG_5G			5 // OFDM:1, HT:2, VHT:2


//###### duplicate code,will move to ODM #########


#ifdef CONFIG_USB_RX_AGGREGATION
typedef enum _USB_RX_AGG_MODE{
	USB_RX_AGG_DISABLE,
	USB_RX_AGG_DMA,
	USB_RX_AGG_USB,
	USB_RX_AGG_MIX
}USB_RX_AGG_MODE;

//#define MAX_RX_DMA_BUFFER_SIZE	10240		// 10K for 8192C RX DMA buffer

#endif

typedef struct _EDCA_TURBO_
{
	BOOLEAN bCurrentTurboEDCA;
	BOOLEAN bIsCurRDLState;

	uint32_t	prv_traffic_idx; // edca turbo

}EDCA_T,*pEDCA_T;


typedef struct _SW_Antenna_Switch_
{
	u8		try_flag;
	int32_t		PreRSSI;
	u8		CurAntenna;
	u8		PreAntenna;
	u8		RSSI_Trying;
	u8		TestMode;
	u8		bTriggerAntennaSwitch;
	u8		SelectAntennaMap;
	u8		RSSI_target;

	// Before link Antenna Switch check
	u8		SWAS_NoLink_State;
	uint32_t		SWAS_NoLink_BK_Reg860;
	BOOLEAN		ANTA_ON;	//To indicate Ant A is or not
	BOOLEAN		ANTB_ON;	//To indicate Ant B is on or not

	int32_t		RSSI_sum_A;
	int32_t		RSSI_sum_B;
	int32_t		RSSI_cnt_A;
	int32_t		RSSI_cnt_B;

	uint64_t		lastTxOkCnt;
	uint64_t		lastRxOkCnt;
	uint64_t 		TXByteCnt_A;
	uint64_t 		TXByteCnt_B;
	uint64_t 		RXByteCnt_A;
	uint64_t 		RXByteCnt_B;
	u8 		TrafficLoad;
	struct timer_list 	SwAntennaSwitchTimer;
/* CE Platform use
#ifdef CONFIG_SW_ANTENNA_DIVERSITY
	_timer SwAntennaSwitchTimer;
	uint64_t lastTxOkCnt;
	uint64_t lastRxOkCnt;
	uint64_t TXByteCnt_A;
	uint64_t TXByteCnt_B;
	uint64_t RXByteCnt_A;
	uint64_t RXByteCnt_B;
	u8 DoubleComfirm;
	u8 TrafficLoad;
	//SW Antenna Switch


#endif
*/
#ifdef CONFIG_HW_ANTENNA_DIVERSITY
	//Hybrid Antenna Diversity
	uint32_t		CCK_Ant1_Cnt[ASSOCIATE_ENTRY_NUM+1];
	uint32_t		CCK_Ant2_Cnt[ASSOCIATE_ENTRY_NUM+1];
	uint32_t		OFDM_Ant1_Cnt[ASSOCIATE_ENTRY_NUM+1];
	uint32_t		OFDM_Ant2_Cnt[ASSOCIATE_ENTRY_NUM+1];
	uint32_t		RSSI_Ant1_Sum[ASSOCIATE_ENTRY_NUM+1];
	uint32_t		RSSI_Ant2_Sum[ASSOCIATE_ENTRY_NUM+1];
	u8		TxAnt[ASSOCIATE_ENTRY_NUM+1];
	u8		TargetSTA;
	u8		antsel;
	u8		RxIdleAnt;

#endif

}SWAT_T, *pSWAT_T;
//#endif

typedef enum _BASEBAND_CONFIG_PHY_REG_PG_VALUE_TYPE{
	PHY_REG_PG_RELATIVE_VALUE = 0,
	PHY_REG_PG_EXACT_VALUE = 1
} PHY_REG_PG_TYPE;
#define AVG_THERMAL_NUM		8
#define IQK_Matrix_REG_NUM		8
#define IQK_Matrix_Settings_NUM	14+24+21 // Channels_2_4G_NUM + Channels_5G_20M_NUM + Channels_5G


typedef struct ODM_RF_Calibration_Structure
{
	//for tx power tracking

	uint32_t	RegA24; // for TempCCK
	int32_t	RegE94;
	int32_t 	RegE9C;
	int32_t	RegEB4;
	int32_t	RegEBC;

	u8  	TXPowercount;
	BOOLEAN bTXPowerTrackingInit;
	BOOLEAN bTXPowerTracking;
	u8  	TxPowerTrackControl; //for mp mode, turn off txpwrtracking as default
	u8  	TM_Trigger;
    	u8  	InternalPA5G[2];	//pathA / pathB

	u8  	ThermalMeter[2];    // ThermalMeter, index 0 for RFIC0, and 1 for RFIC1
	u8  	ThermalValue;
	u8  	ThermalValue_LCK;
	u8  	ThermalValue_IQK;
	u8	ThermalValue_DPK;
	u8	ThermalValue_AVG[AVG_THERMAL_NUM];
	u8	ThermalValue_AVG_index;
	u8	ThermalValue_RxGain;
	u8	ThermalValue_Crystal;
	u8	ThermalValue_DPKstore;
	u8	ThermalValue_DPKtrack;
	BOOLEAN	TxPowerTrackingInProgress;
	BOOLEAN	bDPKenable;

	BOOLEAN	bReloadtxpowerindex;
	u8 	bRfPiEnable;
	uint32_t 	TXPowerTrackingCallbackCnt; //cosa add for debug


	//------------------------- Tx power Tracking -------------------------//
	u8 	bCCKinCH14;
	u8 	CCK_index;
	u8 	OFDM_index[MAX_RF_PATH];
	s8	PowerIndexOffset[MAX_RF_PATH];
	s8	DeltaPowerIndex[MAX_RF_PATH];
	s8	DeltaPowerIndexLast[MAX_RF_PATH];
	BOOLEAN bTxPowerChanged;

	u8 	ThermalValue_HP[HP_THERMAL_NUM];
	u8 	ThermalValue_HP_index;
	BOOLEAN	bNeedIQK;
	u8	Delta_IQK;
	u8	Delta_LCK;
	s8  BBSwingDiff2G, BBSwingDiff5G; // Unit: dB

	//--------------------------------------------------------------------//

	//for IQK
	uint32_t 	RegC04;
	uint32_t 	Reg874;
	uint32_t 	RegC08;
	uint32_t 	RegB68;
	uint32_t 	RegB6C;
	uint32_t 	Reg870;
	uint32_t 	Reg860;
	uint32_t 	Reg864;

	BOOLEAN	bIQKInitialized;
	BOOLEAN bLCKInProgress;
	BOOLEAN	bAntennaDetected;
	uint32_t	ADDA_backup[IQK_ADDA_REG_NUM];
	uint32_t	IQK_MAC_backup[IQK_MAC_REG_NUM];
	uint32_t	IQK_BB_backup_recover[9];
	uint32_t	IQK_BB_backup[IQK_BB_REG_NUM];

	//for APK
	uint32_t 	APKoutput[2][2]; //path A/B; output1_1a/output1_2a
	u8 	bAPKdone;
	u8 	bAPKThermalMeterIgnore;
	u8 	bDPdone;
	u8 	bDPPathAOK;
	u8 	bDPPathBOK;

	uint32_t 	TxIQC_8723B[2][3][2]; // { {S0: 0xc94, 0xc80, 0xc4c} , {S1: 0xc9c, 0xc88, 0xc4c}}
	uint32_t 	RxIQC_8723B[2][2][2]; // { {S0: 0xc14, 0xca0} , {S1: 0xc1c, 0xc78}}

}ODM_RF_CAL_T,*PODM_RF_CAL_T;
typedef struct _ODM_RATE_ADAPTIVE
{
	u8				Type;				// DM_Type_ByFW/DM_Type_ByDriver
	u8				LdpcThres;			// if RSSI > LdpcThres => switch from LPDC to BCC
	BOOLEAN				bUseLdpc;
	BOOLEAN				bLowerRtsRate;
	u8				HighRSSIThresh;		// if RSSI > HighRSSIThresh	=> RATRState is DM_RATR_STA_HIGH
	u8				LowRSSIThresh;		// if RSSI <= LowRSSIThresh	=> RATRState is DM_RATR_STA_LOW
	u8				RATRState;			// Current RSSI level, DM_RATR_STA_HIGH/DM_RATR_STA_MIDDLE/DM_RATR_STA_LOW

} ODM_RATE_ADAPTIVE, *PODM_RATE_ADAPTIVE;
typedef struct _ODM_Phy_Dbg_Info_
{
	//ODM Write,debug info
	s8		RxSNRdB[4];
	uint64_t		NumQryPhyStatus;
	uint64_t		NumQryPhyStatusCCK;
	uint64_t		NumQryPhyStatusOFDM;
	u8		NumQryBeaconPkt;
	//Others
	int32_t		RxEVM[4];

}ODM_PHY_DBG_INFO_T;

typedef struct _ODM_Mac_Status_Info_
{
	u8	test;

}ODM_MAC_INFO;

typedef struct _FALSE_ALARM_STATISTICS{
	uint32_t	Cnt_Parity_Fail;
	uint32_t	Cnt_Rate_Illegal;
	uint32_t	Cnt_Crc8_fail;
	uint32_t	Cnt_Mcs_fail;
	uint32_t	Cnt_Ofdm_fail;
	uint32_t	Cnt_Cck_fail;
	uint32_t	Cnt_all;
	uint32_t	Cnt_Fast_Fsync;
	uint32_t	Cnt_SB_Search_fail;
	uint32_t	Cnt_OFDM_CCA;
	uint32_t	Cnt_CCK_CCA;
	uint32_t	Cnt_CCA_all;
	uint32_t	Cnt_BW_USC;	//Gary
	uint32_t	Cnt_BW_LSC;	//Gary
}FALSE_ALARM_STATISTICS, *PFALSE_ALARM_STATISTICS;
typedef struct _Dynamic_Power_Saving_
{
	u8		PreCCAState;
	u8		CurCCAState;

	u8		PreRFState;
	u8		CurRFState;

	int		    Rssi_val_min;

	u8		initialize;
	uint32_t		Reg874,RegC70,Reg85C,RegA74;

}PS_T,*pPS_T;
typedef struct _Dynamic_Primary_CCA{
	u8		PriCCA_flag;
	u8		intf_flag;
	u8		intf_type;
	u8		DupRTS_flag;
	u8		Monitor_flag;
	u8		CH_offset;
	u8  		MF_state;
}Pri_CCA_T, *pPri_CCA_T;

typedef struct _RX_High_Power_
{
	u8		RXHP_flag;
	u8		PSD_func_trigger;
	u8		PSD_bitmap_RXHP[80];
	u8		Pre_IGI;
	u8		Cur_IGI;
	u8		Pre_pw_th;
	u8		Cur_pw_th;
	BOOLEAN		First_time_enter;
	BOOLEAN		RXHP_enable;
	u8		TP_Mode;
	struct timer_list	PSDTimer;

}RXHP_T, *pRXHP_T;


typedef struct _Dynamic_Initial_Gain_Threshold_
{
	u8		Dig_Enable_Flag;
	u8		Dig_Ext_Port_Stage;

	int		   	RssiLowThresh;
	int		    	RssiHighThresh;

	uint32_t		FALowThresh;
	uint32_t		FAHighThresh;

	u8		CurSTAConnectState;
	u8		PreSTAConnectState;
	u8		CurMultiSTAConnectState;

	u8		PreIGValue;
	u8		CurIGValue;
	u8		BT30_CurIGI;
	u8		BackupIGValue;

	s8		BackoffVal;
	s8		BackoffVal_range_max;
	s8		BackoffVal_range_min;
	u8		rx_gain_range_max;
	u8		rx_gain_range_min;
	u8		Rssi_val_min;

	u8		PreCCK_CCAThres;
	u8		CurCCK_CCAThres;
	u8		PreCCKPDState;
	u8		CurCCKPDState;

	u8		LargeFAHit;
	u8		ForbiddenIGI;
	uint32_t		Recover_cnt;

	u8		DIG_Dynamic_MIN_0;
	u8		DIG_Dynamic_MIN_1;
	BOOLEAN		bMediaConnect_0;
	BOOLEAN		bMediaConnect_1;

	uint32_t		AntDiv_RSSI_max;
	uint32_t		RSSI_max;
}DIG_T,*pDIG_T;



#define ASSOCIATE_ENTRY_NUM					32 // Max size of AsocEntry[].
#define	ODM_ASSOCIATE_ENTRY_NUM				ASSOCIATE_ENTRY_NUM


typedef struct _FAST_ANTENNA_TRAINNING_
{
	u8	Bssid[6];
	u8	antsel_rx_keep_0;
	u8	antsel_rx_keep_1;
	u8	antsel_rx_keep_2;
	uint32_t	antSumRSSI[7];
	uint32_t	antRSSIcnt[7];
	uint32_t	antAveRSSI[7];
	u8	FAT_State;
	uint32_t	TrainIdx;
	u8	antsel_a[ODM_ASSOCIATE_ENTRY_NUM];
	u8	antsel_b[ODM_ASSOCIATE_ENTRY_NUM];
	u8	antsel_c[ODM_ASSOCIATE_ENTRY_NUM];
	uint32_t	MainAnt_Sum[ODM_ASSOCIATE_ENTRY_NUM];
	uint32_t	AuxAnt_Sum[ODM_ASSOCIATE_ENTRY_NUM];
	uint32_t	MainAnt_Cnt[ODM_ASSOCIATE_ENTRY_NUM];
	uint32_t	AuxAnt_Cnt[ODM_ASSOCIATE_ENTRY_NUM];
	u8	RxIdleAnt;
	BOOLEAN		bBecomeLinked;

}FAT_T,*pFAT_T;

typedef struct _ODM_PATH_DIVERSITY_
{
	u8	RespTxPath;
	u8	PathSel[ODM_ASSOCIATE_ENTRY_NUM];
	uint32_t	PathA_Sum[ODM_ASSOCIATE_ENTRY_NUM];
	uint32_t	PathB_Sum[ODM_ASSOCIATE_ENTRY_NUM];
	uint32_t	PathA_Cnt[ODM_ASSOCIATE_ENTRY_NUM];
	uint32_t	PathB_Cnt[ODM_ASSOCIATE_ENTRY_NUM];
}PATHDIV_T, *pPATHDIV_T;

typedef struct _ANT_DETECTED_INFO{
	BOOLEAN			bAntDetected;
	uint32_t			dBForAntA;
	uint32_t			dBForAntB;
	uint32_t			dBForAntO;
}ANT_DETECTED_INFO, *PANT_DETECTED_INFO;



//
// 2011/09/22 MH Copy from SD4 defined structure. We use to support PHY DM integration.
//
struct _rtw_dm {
	//struct timer_list 	FastAntTrainingTimer;
	//
	//	Add for different team use temporarily
	//
	struct rtl_priv *	Adapter;		// For CE/NIC team
	// WHen you use Adapter or priv pointer, you must make sure the pointer is ready.
	BOOLEAN			odm_ready;

	PHY_REG_PG_TYPE		PhyRegPgValueType;

	uint64_t			DebugComponents;
	uint32_t			DebugLevel;

	uint64_t			NumQryPhyStatusAll; 	//CCK + OFDM
	uint64_t			LastNumQryPhyStatusAll;
	uint64_t			RxPWDBAve;
	uint64_t			RxPWDBAve_final;
	BOOLEAN			MPDIG_2G; 		//off MPDIG
	u8			Times_2G;

//------ ODM HANDLE, DRIVER NEEDS NOT TO HOOK------//
	BOOLEAN			bCckHighPower;
	u8			RFPathRxEnable;		// ODM_CMNINFO_RFPATH_ENABLE
	u8			ControlChannel;
//------ ODM HANDLE, DRIVER NEEDS NOT TO HOOK------//

//--------REMOVED COMMON INFO----------//
	//u8				PseudoMacPhyMode;
	//BOOLEAN			*BTCoexist;
	//BOOLEAN			PseudoBtCoexist;
	//u8				OPMode;
	//BOOLEAN			bAPMode;
	//BOOLEAN			bClientMode;
	//BOOLEAN			bAdHocMode;
	//BOOLEAN			bSlaveOfDMSP;
//--------REMOVED COMMON INFO----------//


//1  COMMON INFORMATION

	//
	// Init Value
	//
//-----------HOOK BEFORE REG INIT-----------//
	// ODM Support Ability DIG/RATR/TX_PWR_TRACK/ ¡K¡K = 1/2/3/¡K
	uint32_t			SupportAbility;
	// ODM PCIE/USB/SDIO = 1/2/3
	u8			SupportInterface;
	// ODM composite or independent. Bit oriented/ 92C+92D+ .... or any other type = 1/2/3/...
	uint32_t			SupportICType;
	// Cut Version TestChip/A-cut/B-cut... = 0/1/2/3/...
	u8			CutVersion;
	// Fab Version TSMC/UMC = 0/1
	u8			FabVersion;
	// RF Type 4T4R/3T3R/2T2R/1T2R/1T1R/...
	u8			RFType;
	u8			RFEType;
	// Board Type Normal/HighPower/MiniCard/SLIM/Combo/... = 0/1/2/3/4/...
	u8			BoardType;
	// with external LNA  NO/Yes = 0/1
	u8			ExtLNA;
	u8			ExtLNA5G;
	// with external PA  NO/Yes = 0/1
	u8			ExtPA;
	u8			ExtPA5G;
	// with external TRSW  NO/Yes = 0/1
	u8			ExtTRSW;
	u8			PatchID; //Customer ID
	BOOLEAN			bInHctTest;
	BOOLEAN			bWIFITest;

	BOOLEAN			bDualMacSmartConcurrent;
	uint32_t			BK_SupportAbility;
	u8			AntDivType;
//-----------HOOK BEFORE REG INIT-----------//

	//
	// Dynamic Value
	//
//--------- POINTER REFERENCE-----------//

	u8			u8_temp;
	BOOLEAN			BOOLEAN_temp;

	// MAC PHY Mode SMSP/DMSP/DMDP = 0/1/2
	u8			*pMacPhyMode;
	// Wireless mode B/G/A/N = BIT0/BIT1/BIT2/BIT3
	u8			*pWirelessMode; //ODM_WIRELESS_MODE_E
	// Frequence band 2.4G/5G = 0/1
	u8			*pBandType;
	// Secondary channel offset don't_care/below/above = 0/1/2
	u8			*pSecChOffset;
	// Security mode Open/WEP/AES/TKIP = 0/1/2/3
	u8			*pSecurity;
	// BW info 20M/40M/80M = 0/1/2
	u8			*pBandWidth;
 	// Central channel location Ch1/Ch2/....
	u8			*pChannel;	//central channel number
	// Common info for 92D DMSP

	BOOLEAN			*pbGetValueFromOtherMac;
	BOOLEAN			*pbMasterOfDMSP; //MAC0: master, MAC1: slave
	// Common info for Status
	BOOLEAN			*pbScanInProcess;
	BOOLEAN			*pbPowerSaving;
	// CCA Path 2-path/path-A/path-B = 0/1/2; using ODM_CCA_PATH_E.
	u8			*pOnePathCCA;
	//pMgntInfo->AntennaTest
	u8			*pAntennaTest;
	BOOLEAN			*pbNet_closed;
	u8			*mp_mode;
	//u8			*pAidMap;
//--------- POINTER REFERENCE-----------//
	u16			*pForcedDataRate;
//------------CALL BY VALUE-------------//
	BOOLEAN			bLinkInProcess;
	BOOLEAN			bWIFI_Direct;
	BOOLEAN			bWIFI_Display;
	BOOLEAN			bLinked;
	u8			RSSI_Min;
	u8          InterfaceIndex; // Add for 92D  dual MAC: 0--Mac0 1--Mac1
	BOOLEAN         bIsMPChip;
	BOOLEAN			bOneEntryOnly;
	// Common info for BTDM
	BOOLEAN			bBtDisabled;			// BT is disabled
	BOOLEAN			bBtConnectProcess;	// BT HS is under connection progress.
	u8			btHsRssi;				// BT HS mode wifi rssi value.
	BOOLEAN			bBtHsOperation;		// BT HS mode is under progress
	BOOLEAN			bBtDisableEdcaTurbo;	// Under some condition, don't enable the EDCA Turbo
	BOOLEAN			bBtLimitedDig;   		// BT is busy.
//------------CALL BY VALUE-------------//
	u8			RSSI_A;
	u8			RSSI_B;
	uint64_t			RSSI_TRSW;
	uint64_t			RSSI_TRSW_H;
	uint64_t			RSSI_TRSW_L;
	uint64_t			RSSI_TRSW_iso;

	u8			RxRate;
	BOOLEAN			StopDIG;
	u8			TxRate;
	u8			LinkedInterval;
	u8			preChannel;
	u8			AntType;
	uint32_t			TxagcOffsetValueA;
	BOOLEAN			IsTxagcOffsetPositiveA;
	uint32_t			TxagcOffsetValueB;
	BOOLEAN			IsTxagcOffsetPositiveB;
	uint64_t			lastTxOkCnt;
	uint64_t			lastRxOkCnt;
	uint32_t			BbSwingOffsetA;
	BOOLEAN			IsBbSwingOffsetPositiveA;
	uint32_t			BbSwingOffsetB;
	BOOLEAN			IsBbSwingOffsetPositiveB;
	uint32_t			TH_H;
	uint32_t			TH_L;
	uint32_t			IGI_Base;
	uint32_t			IGI_target;
	BOOLEAN			ForceEDCCA;
	u8			AdapEn_RSSI;
	BOOLEAN			bForceThresh;
	uint32_t			Force_TH_H;
	uint32_t			Force_TH_L;

	//2 Define STA info.
	// _ODM_STA_INFO
	// 2012/01/12 MH For MP, we need to reduce one array pointer for default port.??
	struct sta_info *pODM_StaInfo[ODM_ASSOCIATE_ENTRY_NUM];

#if (RATE_ADAPTIVE_SUPPORT == 1)
	u16 			CurrminRptTime;
	ODM_RA_INFO_T   RAInfo[ODM_ASSOCIATE_ENTRY_NUM]; //See HalMacID support
#endif
	//
	// 2012/02/14 MH Add to share 88E ra with other SW team.
	// We need to colelct all support abilit to a proper area.
	//
	BOOLEAN				RaSupport88E;

	// Define ...........

	// Latest packet phy info (ODM write)
	ODM_PHY_DBG_INFO_T	 PhyDbgInfo;
	//PHY_INFO_88E		PhyInfo;

	// Latest packet phy info (ODM write)
	ODM_MAC_INFO		*pMacInfo;
	//MAC_INFO_88E		MacInfo;

	// Different Team independt structure??

	//
	//TX_RTP_CMN		TX_retrpo;
	//TX_RTP_88E		TX_retrpo;
	//TX_RTP_8195		TX_retrpo;

	//
	//ODM Structure
	//
	FAT_T		DM_FatTable;
	DIG_T		DM_DigTable;
	PS_T		DM_PSTable;
	Pri_CCA_T	DM_PriCCA;
	RXHP_T		DM_RXHP_Table;
	FALSE_ALARM_STATISTICS	FalseAlmCnt;
	FALSE_ALARM_STATISTICS	FlaseAlmCntBuddyAdapter;
	//#ifdef CONFIG_ANTENNA_DIVERSITY
	SWAT_T		DM_SWAT_Table;
	BOOLEAN		RSSI_test;
	//#endif


	EDCA_T		DM_EDCA_Table;
	uint32_t		WMMEDCA_BE;
	PATHDIV_T	DM_PathDiv;
	// Copy from SD4 structure
	//
	// ==================================================
	//

	//common
	//u8		DM_Type;
	//u8    PSD_Report_RXHP[80];   // Add By Gary
	//u8    PSD_func_flag;               // Add By Gary
	//for DIG
	//u8		bDMInitialGainEnable;
	//u8		binitialized; // for dm_initial_gain_Multi_STA use.
	//for Antenna diversity
	//u8	AntDivCfg;// 0:OFF , 1:ON, 2:by efuse
	//struct sta_info *RSSI_target;

	BOOLEAN			*pbDriverStopped;
	BOOLEAN			*pbDriverIsGoingToPnpSetPowerSleep;
	BOOLEAN			*pinit_adpt_in_progress;

	//PSD
	BOOLEAN			bUserAssignLevel;
	struct timer_list 		PSDTimer;
	u8			RSSI_BT;			//come from BT
	BOOLEAN			bPSDinProcess;
	BOOLEAN			bPSDactive;
	BOOLEAN			bDMInitialGainEnable;

	//MPT DIG
	struct timer_list 		MPT_DIGTimer;

	//for rate adaptive, in fact,  88c/92c fw will handle this
	u8			bUseRAMask;

	ODM_RATE_ADAPTIVE	RateAdaptive;

	ANT_DETECTED_INFO	AntDetectedInfo; // Antenna detected information for RSSI tool

	ODM_RF_CAL_T	RFCalibrateInfo;

	//
	// TX power tracking
	//
	u8			BbSwingIdxOfdm[MAX_RF_PATH];
	u8			BbSwingIdxOfdmCurrent;
	u8			BbSwingIdxOfdmBase[MAX_RF_PATH];
	BOOLEAN			BbSwingFlagOfdm;
	u8			BbSwingIdxCck;
	u8			BbSwingIdxCckCurrent;
	u8			BbSwingIdxCckBase;
	u8			DefaultOfdmIndex;
	u8			DefaultCckIndex;
	BOOLEAN			BbSwingFlagCck;

	s8			Aboslute_OFDMSwingIdx[MAX_RF_PATH];
	s8			Remnant_OFDMSwingIdx[MAX_RF_PATH];
	s8			Remnant_CCKSwingIdx;
	s8			Modify_TxAGC_Value;       //Remnat compensate value at TxAGC
	BOOLEAN			Modify_TxAGC_Flag_PathA;
	BOOLEAN			Modify_TxAGC_Flag_PathB;

	//
	// Dynamic ATC switch
	//
	BOOLEAN			bATCStatus;
	BOOLEAN			largeCFOHit;
	BOOLEAN			bIsfreeze;
	int				CFO_tail[2];
	int				CFO_ave_pre;
	int				CrystalCap;
	u8			CFOThreshold;
	uint32_t			packetCount;
	uint32_t			packetCount_pre;

	//
	// ODM system resource.
	//

	// ODM relative time.
	struct timer_list 				PathDivSwitchTimer;
	//2011.09.27 add for Path Diversity
	struct timer_list				CCKPathDiversityTimer;
	struct timer_list 	FastAntTrainingTimer;

	// ODM relative workitem.

};

struct _rtw_hal {
	HAL_VERSION			VersionID;
	RT_MULTI_FUNC		MultiFunc; // For multi-function consideration.
	RT_POLARITY_CTL		PolarityCtl; // For Wifi PDn Polarity control.
	RT_REGULATOR_MODE	RegulatorMode; // switching regulator or LDO

	//current WIFI_PHY values
	WIRELESS_MODE		CurrentWirelessMode;
	BAND_TYPE			CurrentBandType;	//0:2.4G, 1:5G
	BAND_TYPE			BandSet;
	uint8_t	CurrentCenterFrequencyIndex1;
	uint8_t	nCur40MhzPrimeSC;// Control channel sub-carrier
	uint8_t	nCur80MhzPrimeSC;   //used for primary 40MHz of 80MHz mode

	u16	CustomerID;
	u16	BasicRateSet;
	u16 ForcedDataRate;// Force Data Rate. 0: Auto, 0x02: 1M ~ 0x6C: 54M.
	u32	ReceiveConfig;

	//rf_ctrl
	uint8_t	rf_chip;
	uint8_t	NumTotalRFPath;

	uint8_t	InterfaceSel;
	uint8_t	framesync;
	u32	framesyncC34;
	uint8_t	framesyncMonitor;
	uint8_t	DefaultInitialGain[4];

	uint8_t	bTXPowerDataReadFromEEPORM;
	uint8_t	bAPKThermalMeterIgnore;

	BOOLEAN 		EepromOrEfuse;
	uint8_t				EfuseUsedPercentage;
	u16				EfuseUsedBytes;
	//uint8_t				EfuseMap[2][HWSET_MAX_SIZE_JAGUAR];


	uint8_t	Regulation2_4G;
	uint8_t	Regulation5G;

	uint8_t	TxPwrInPercentage;

	uint8_t	TxPwrCalibrateRate;
	//
	// TX power by rate table at most 4RF path.
	// The register is
	//
	// VHT TX power by rate off setArray =
	// Band:-2G&5G = 0 / 1
	// RF: at most 4*4 = ABCD=0/1/2/3
	// CCK=0 OFDM=1/2 HT-MCS 0-15=3/4/56 VHT=7/8/9/10/11
	//
	uint8_t	TxPwrByRateTable;
	uint8_t	TxPwrByRateBand;
	u32	TxPwrByRateOffset[TX_PWR_BY_RATE_NUM_BAND]
						[TX_PWR_BY_RATE_NUM_RF]
						[TX_PWR_BY_RATE_NUM_SECTION];
	//---------------------------------------------------------------------------------//

	//2 Power Limit Table
	uint8_t	TxPwrLevelCck[RF_PATH_MAX_92C_88E][CHANNEL_MAX_NUMBER];
	uint8_t	TxPwrLevelHT40_1S[RF_PATH_MAX_92C_88E][CHANNEL_MAX_NUMBER];	// For HT 40MHZ pwr
	uint8_t	TxPwrLevelHT40_2S[RF_PATH_MAX_92C_88E][CHANNEL_MAX_NUMBER];	// For HT 40MHZ pwr
	uint8_t	TxPwrHt20Diff[RF_PATH_MAX_92C_88E][CHANNEL_MAX_NUMBER];// HT 20<->40 Pwr diff
	uint8_t	TxPwrLegacyHtDiff[RF_PATH_MAX_92C_88E][CHANNEL_MAX_NUMBER];// For HT<->legacy pwr diff

	// Power Limit Table for 2.4G

	// Store the original power by rate value of the base of each rate section of rf path A & B
	uint8_t	TxPwrByRateBase2_4G[MAX_RF_PATH_NUM_IN_POWER_LIMIT_TABLE]
						[MAX_BASE_NUM_IN_PHY_REG_PG_2_4G];
	uint8_t	TxPwrByRateBase5G[MAX_RF_PATH_NUM_IN_POWER_LIMIT_TABLE]
						[MAX_BASE_NUM_IN_PHY_REG_PG_5G];




	uint8_t	PGMaxGroup;
	uint8_t	LegacyHTTxPowerDiff;// Legacy to HT rate power diff

	// Read/write are allow for following hardware information variables
	uint8_t	pwrGroupCnt;
	u32	MCSTxPowerLevelOriginalOffset[MAX_PG_GROUP][16];
	u32	CCKTxPowerLevelOriginalOffset;

	uint8_t	CrystalCap;
	u32	AntennaTxPath;					// Antenna path Tx
	u32	AntennaRxPath;					// Antenna path Rx

	uint8_t	RFEType;
	uint8_t	BoardType;
	uint8_t	ExternalPA;
	uint8_t	bIQKInitialized;
	BOOLEAN		bLCKInProgress;

	BOOLEAN		bSwChnl;
	BOOLEAN		bSetChnlBW;
	BOOLEAN		bChnlBWInitialzed;
	BOOLEAN		bNeedIQK;

	uint8_t	bLedOpenDrain; // Support Open-drain arrangement for controlling the LED. Added by Roger, 2009.10.16.
	uint8_t	TxPowerTrackControl; //for mp mode, turn off txpwrtracking as default
	uint8_t	b1x1RecvCombine;	// for 1T1R receive combining

	u32	AcParam_BE; //Original parameter for BE, use for EDCA turbo.

	BB_REGISTER_DEFINITION_T	PHYRegDef[4];	//Radio A/B/C/D

	u32	RfRegChnlVal[2];

	//RDG enable
	BOOLEAN	 bRDGEnable;

	//for host message to fw
	uint8_t	LastHMEBoxNum;

	uint8_t	fw_ractrl;
	uint8_t	RegTxPause;
	// Beacon function related global variable.
	uint8_t	RegBcnCtrlVal;
	uint8_t	RegFwHwTxQCtrl;
	uint8_t	RegReg542;
	uint8_t	RegCR_1;
	uint8_t	Reg837;
	uint8_t	RegRFPathS1;
	u16	RegRRSR;

	uint8_t	CurAntenna;
	uint8_t	AntDivCfg;
	uint8_t	TRxAntDivType;

	uint8_t	bDumpRxPkt;//for debug
	uint8_t	bDumpTxPkt;//for debug
	uint8_t	FwRsvdPageStartOffset; //2010.06.23. Added by tynli. Reserve page start offset except beacon in TxQ.

	// 2010/08/09 MH Add CU power down mode.
	BOOLEAN		pwrdown;

	// Add for dual MAC  0--Mac0 1--Mac1
	u32	interfaceIndex;

	uint8_t	OutEpQueueSel;
	uint8_t	OutEpNumber;

	// 2010/12/10 MH Add for USB aggreation mode dynamic shceme.
	BOOLEAN		UsbRxHighSpeedMode;

	// 2010/11/22 MH Add for slim combo debug mode selective.
	// This is used for fix the drawback of CU TSMC-A/UMC-A cut. HW auto suspend ability. Close BT clock.
	BOOLEAN		SlimComboDbg;

	uint8_t	AMPDUDensity;

	// Auto FSM to Turn On, include clock, isolation, power control for MAC only
	uint8_t	bMacPwrCtrlOn;

	RT_AMPDU_BRUST		AMPDUBurstMode; //92C maybe not use, but for compile successfully


	u32	UsbBulkOutSize;
	BOOLEAN		bSupportUSB3;

	// Interrupt relatd register information.
	u32	IntArray[3];//HISR0,HISR1,HSISR
	u32	IntrMask[3];
	uint8_t	C2hArray[16];
	#ifdef CONFIG_USB_TX_AGGREGATION
	uint8_t	UsbTxAggMode;
	uint8_t	UsbTxAggDescNum;
	#endif // CONFIG_USB_TX_AGGREGATION

	#ifdef CONFIG_USB_RX_AGGREGATION
	u16	HwRxPageSize;				// Hardware setting
	u32	MaxUsbRxAggBlock;

	USB_RX_AGG_MODE	UsbRxAggMode;
	uint8_t	UsbRxAggBlockCount;			// USB Block count. Block size is 512-byte in hight speed and 64-byte in full speed
	uint8_t	UsbRxAggBlockTimeout;
	uint8_t	UsbRxAggPageCount;			// 8192C DMA page count
	uint8_t	UsbRxAggPageTimeout;

	uint8_t	RegAcUsbDmaSize;
	uint8_t	RegAcUsbDmaTime;
	#endif//CONFIG_USB_RX_AGGREGATION



	struct dm_priv	dmpriv;
	struct _rtw_dm odmpriv;

#ifdef DBG_CONFIG_ERROR_DETECT
	struct sreset_priv srestpriv;
#endif


};

static inline struct _rtw_hal *GET_HAL_DATA(struct rtl_priv *priv)
{
	return priv->HalData;
}
#define GET_HAL_RFPATH_NUM(rtlpriv) ((( struct _rtw_hal *)((rtlpriv)->HalData))->NumTotalRFPath )
#define RT_GetInterfaceSelection(_Adapter) 	(GET_HAL_DATA(_Adapter)->InterfaceSel)


#endif
