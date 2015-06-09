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

struct rtl_priv;


#ifdef CONFIG_80211N_HT
#include <rtw_ht.h>
#endif

#ifdef CONFIG_80211AC_VHT
#include <rtw_vht.h>
#endif


#include <rtw_rf.h>
#include <rtw_cmd.h>
#include <cmd_osdep.h>
#include <xmit_osdep.h>
#include <rtw_recv.h>
#include <rtw_security.h>
#include <rtw_xmit.h>
#include <recv_osdep.h>
#include <rtw_efuse.h>
#include <../rtl8821au/led.h>			/* Cureently here needed */
#include <hal_intf.h>
#include <hal_com.h>
#include <rtw_qos.h>
#include <rtw_pwrctrl.h>
#include <rtw_mlme.h>
#include <mlme_osdep.h>
#include <rtw_ioctl.h>
#include <rtw_ioctl_set.h>
#include <osdep_intf.h>
#include <rtw_eeprom.h>
#include <sta_info.h>
#include <rtw_event.h>
#include <rtw_mlme_ext.h>
#include <rtw_ap.h>
#include <rtw_efuse.h>

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
	uint8_t	RegPowerBase;
	uint8_t	AmplifierType_2G;
	uint8_t	AmplifierType_5G;
	uint8_t	bEn_RFE;
	uint8_t	RFE_Type;

};


//For registry parameters
#define RGTRY_OFT(field) ((u32)FIELD_OFFSET(struct registry_priv,field))
#define RGTRY_SZ(field)   sizeof(((struct registry_priv*) 0)->field)
#define BSSID_OFT(field) ((u32)FIELD_OFFSET(WLAN_BSSID_EX,field))
#define BSSID_SZ(field)   sizeof(((PWLAN_BSSID_EX) 0)->field)

#define MAX_CONTINUAL_URB_ERR 4


#define GET_IFACE_NUMS(rtlpriv) (((struct rtl_priv *)rtlpriv)->dvobj->iface_nums)

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

#include "../wifi.h"



int rtw_handle_dualmac(struct rtl_priv *rtlpriv, bool init);

__inline static uint8_t *myid(struct eeprom_priv *peepriv)
{
	return (peepriv->mac_addr);
}

// HCI Related header file
#include <usb_ops.h>


#endif //__DRV_TYPES_H__

