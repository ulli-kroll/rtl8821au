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
/*-------------------------------------------------------------------------------

	For type defines and data structure defines

--------------------------------------------------------------------------------*/


#ifndef __DRV_TYPES_H__
#define __DRV_TYPES_H__

#include <drv_conf.h>
#include <basic_types.h>
#include <osdep_service.h>
#include <wlan_bssdef.h>
#include <wifi.h>
#include <ieee80211.h>

struct _ADAPTER;

#include <rtw_debug.h>

#ifdef CONFIG_80211N_HT
#include <rtw_ht.h>
#endif

#ifdef CONFIG_80211AC_VHT
#include <rtw_vht.h>
#endif


#include <rtw_rf.h>
#include <rtw_cmd.h>
#include <cmd_osdep.h>
#include <rtw_security.h>
#include <rtw_xmit.h>
#include <xmit_osdep.h>
#include <rtw_recv.h>
#include <recv_osdep.h>
#include <rtw_efuse.h>
#include <rtw_sreset.h>
#include <hal_intf.h>
#include <hal_com.h>
#include <hal_com_led.h>
#include <rtw_qos.h>
#include <rtw_pwrctrl.h>
#include <rtw_mlme.h>
#include <mlme_osdep.h>
#include <rtw_io.h>
#include <rtw_ioctl.h>
#include <rtw_ioctl_set.h>
#include <osdep_intf.h>
#include <rtw_eeprom.h>
#include <sta_info.h>
#include <rtw_event.h>
#include <rtw_mlme_ext.h>
#include <rtw_ap.h>
#include <rtw_efuse.h>
#include <rtw_version.h>

#include <ip.h>
#include <if_ether.h>
#include <ethernet.h>
#include <circ_buf.h>

#define SPEC_DEV_ID_NONE BIT(0)
#define SPEC_DEV_ID_DISABLE_HT BIT(1)
#define SPEC_DEV_ID_ENABLE_PS BIT(2)
#define SPEC_DEV_ID_RF_CONFIG_1T1R BIT(3)
#define SPEC_DEV_ID_RF_CONFIG_2T2R BIT(4)
#define SPEC_DEV_ID_ASSIGN_IFNAME BIT(5)

struct specific_device_id{

	u32		flags;

	u16		idVendor;
	u16		idProduct;

};

struct registry_priv
{
	uint8_t	chip_version;
	uint8_t	rfintfs;
	uint8_t	lbkmode;
	uint8_t	hci;
	NDIS_802_11_SSID	ssid;
	uint8_t	network_mode;	//infra, ad-hoc, auto
	uint8_t	channel;//ad-hoc support requirement
	uint8_t	wireless_mode;//A, B, G, auto
	uint8_t 	scan_mode;//active, passive
	uint8_t	radio_enable;
	uint8_t	preamble;//long, short, auto
	uint8_t	vrtl_carrier_sense;//Enable, Disable, Auto
	uint8_t	vcs_type;//RTS/CTS, CTS-to-self
	u16	rts_thresh;
	u16  frag_thresh;
	uint8_t	adhoc_tx_pwr;
	uint8_t	soft_ap;
	uint8_t	power_mgnt;
	uint8_t	ips_mode;
	uint8_t	smart_ps;
	uint8_t	long_retry_lmt;
	uint8_t	short_retry_lmt;
	u16	busy_thresh;
	uint8_t	ack_policy;
	uint8_t	mp_mode;
	uint8_t	software_encrypt;
	uint8_t	software_decrypt;
	uint8_t	acm_method;
	  //UAPSD
	uint8_t	wmm_enable;
	uint8_t	uapsd_enable;
	uint8_t	uapsd_max_sp;
	uint8_t	uapsd_acbk_en;
	uint8_t	uapsd_acbe_en;
	uint8_t	uapsd_acvi_en;
	uint8_t	uapsd_acvo_en;

	WLAN_BSSID_EX    dev_network;

#ifdef CONFIG_80211N_HT
	uint8_t	ht_enable;
	// 0: 20 MHz, 1: 40 MHz, 2: 80 MHz, 3: 160MHz
	// 2.4G use bit 0 ~ 3, 5G use bit 4 ~ 7
	// 0x21 means enable 2.4G 40MHz & 5G 80MHz
	uint8_t	bw_mode;
	uint8_t	ampdu_enable;//for tx
	uint8_t 	rx_stbc;
	uint8_t	ampdu_amsdu;//A-MPDU Supports A-MSDU is permitted
	// Short GI support Bit Map
	// BIT0 - 20MHz, 1: support, 0: non-support
	// BIT1 - 40MHz, 1: support, 0: non-support
	// BIT2 - 80MHz, 1: support, 0: non-support
	// BIT3 - 160MHz, 1: support, 0: non-support
	uint8_t	short_gi;
#endif //CONFIG_80211N_HT

#ifdef CONFIG_80211AC_VHT
	uint8_t	vht_enable;
	uint8_t	ampdu_factor;
	uint8_t	vht_rate_sel;
	// BIT0: Enable VHT LDPC Rx, BIT1: Enable VHT LDPC Tx, BIT4: Enable HT LDPC Rx, BIT5: Enable HT LDPC Tx
	uint8_t	ldpc_cap;
	// BIT0: Enable VHT STBC Rx, BIT1: Enable VHT STBC Tx, BIT4: Enable HT STBC Rx, BIT5: Enable HT STBC Tx
	uint8_t	stbc_cap;
	// BIT0: Enable VHT Beamformer, BIT1: Enable VHT Beamformee, BIT4: Enable HT Beamformer, BIT5: Enable HT Beamformee
	uint8_t	beamform_cap;
#endif //CONFIG_80211AC_VHT

	uint8_t	lowrate_two_xmit;

	uint8_t	rf_config ;
	uint8_t	low_power ;

	uint8_t	wifi_spec;// !turbo_mode

	uint8_t	channel_plan;
	BOOLEAN	bAcceptAddbaReq;

	uint8_t	antdiv_cfg;
	uint8_t	antdiv_type;

	uint8_t	usbss_enable;//0:disable,1:enable
	uint8_t	hwpdn_mode;//0:disable,1:enable,2:decide by EFUSE config
	uint8_t	hwpwrp_detect;//0:disable,1:enable

	uint8_t	hw_wps_pbc;//0:disable,1:enable
#ifdef CONFIG_80211D
	uint8_t enable80211d;
#endif

	uint8_t ifname[16];
	uint8_t if2name[16];

	uint8_t notch_filter;

	uint8_t regulatory_tid;

	//define for tx power adjust
	u32	RegTxPwrLimit;
	uint8_t	RegEnableTxPowerLimit;
	uint8_t	RegPowerBase;
	uint8_t	RegPwrTblSel;
	uint8_t	RegPwrByRate;
	s8	TxBBSwing_2G;
	s8	TxBBSwing_5G;
	uint8_t	AmplifierType_2G;
	uint8_t	AmplifierType_5G;
	uint8_t	bEn_RFE;
	uint8_t	RFE_Type;

};


//For registry parameters
#define RGTRY_OFT(field) ((ULONG)FIELD_OFFSET(struct registry_priv,field))
#define RGTRY_SZ(field)   sizeof(((struct registry_priv*) 0)->field)
#define BSSID_OFT(field) ((ULONG)FIELD_OFFSET(WLAN_BSSID_EX,field))
#define BSSID_SZ(field)   sizeof(((PWLAN_BSSID_EX) 0)->field)

#define MAX_CONTINUAL_URB_ERR 4


#define GET_IFACE_NUMS(padapter) (((struct _ADAPTER *)padapter)->dvobj->iface_nums)

struct dvobj_priv {
        struct _ADAPTER *padapter;

	//for local/global synchronization
	//
	spinlock_t	lock;
	int macid[NUM_STA];

	struct mutex hw_init_mutex;
	struct mutex h2c_fwcmd_mutex;
	struct mutex setch_mutex;
	struct mutex setbw_mutex;

	unsigned char	oper_channel; //saved channel info when call set_channel_bw
	unsigned char	oper_bwmode;
	unsigned char	oper_ch_offset;//PRIME_CHNL_OFFSET

	//For 92D, DMDP have 2 interface.
	uint8_t	InterfaceNumber;
	uint8_t	NumInterfaces;

	//In /Out Pipe information
	int	RtInPipe[2];
	int	RtOutPipe[3];
	uint8_t	Queue2Pipe[HW_QUEUE_ENTRY];//for out pipe mapping

	uint8_t	irq_alloc;

/*-------- below is for SDIO INTERFACE --------*/

#ifdef INTF_DATA
	INTF_DATA intf_data;
#endif

/*-------- below is for USB INTERFACE --------*/


	uint8_t	usb_speed; // 1.1, 2.0 or 3.0
	uint8_t	nr_endpoint;
	uint8_t	RtNumInPipes;
	uint8_t	RtNumOutPipes;
	int	ep_num[6]; //endpoint number

	int	RegUsbSS;

	struct  semaphore	usb_suspend_sema;

#ifdef CONFIG_USB_VENDOR_REQ_MUTEX
	struct mutex usb_vendor_req_mutex;
#endif

#ifdef CONFIG_USB_VENDOR_REQ_BUFFER_PREALLOC
	uint8_t * usb_alloc_vendor_req_buf;
	uint8_t * usb_vendor_req_buf;
#endif

#ifdef PLATFORM_LINUX
	struct usb_interface *pusbintf;
	struct usb_device *pusbdev;
#endif//PLATFORM_LINUX

	ATOMIC_T continual_urb_error;

/*-------- below is for PCIE INTERFACE --------*/

};

#ifdef PLATFORM_LINUX
static struct device *dvobj_to_dev(struct dvobj_priv *dvobj)
{
	/* todo: get interface type from dvobj and the return the dev accordingly */
#ifdef RTW_DVOBJ_CHIP_HW_TYPE
#endif

	return &dvobj->pusbintf->dev;
}
#endif

enum _IFACE_TYPE {
	IFACE_PORT0, //mapping to port0 for C/D series chips
	IFACE_PORT1, //mapping to port1 for C/D series chip
	MAX_IFACE_PORT,
};

enum _ADAPTER_TYPE {
	PRIMARY_ADAPTER,
	SECONDARY_ADAPTER,
	MAX_ADAPTER = 0xFF,
};

typedef enum _DRIVER_STATE{
	DRIVER_NORMAL = 0,
	DRIVER_DISAPPEAR = 1,
	DRIVER_REPLACE_DONGLE = 2,
}DRIVER_STATE;


struct _ADAPTER{
	int	DriverState;// for disable driver using module, use dongle to replace module.
	int	pid[3];//process id from UI, 0:wps, 1:hostapd, 2:dhcpcd
	int	bDongle;//build-in module or external dongle
	u16 	chip_type;
	u16	HardwareType;
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
	PVOID			HalData;
	struct hal_ops	*HalFunc;

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

	void (*intf_start)(struct _ADAPTER * adapter);
	void (*intf_stop)(struct _ADAPTER * adapter);


#ifdef PLATFORM_LINUX
	_nic_hdl ndev;

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

#define adapter_to_dvobj(adapter) (adapter->dvobj)

int rtw_handle_dualmac(struct _ADAPTER *adapter, bool init);

__inline static uint8_t *myid(struct eeprom_priv *peepriv)
{
	return (peepriv->mac_addr);
}

// HCI Related header file
#include <usb_osintf.h>
#include <usb_ops.h>
#include <usb_hal.h>





#endif //__DRV_TYPES_H__

