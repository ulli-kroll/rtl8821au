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
//***** temporarily flag *******
#define CONFIG_SINGLE_IMG
#define CONFIG_ODM_REFRESH_RAMASK
#define CONFIG_PHY_SETTING_WITH_ODM
//for FPGA VERIFICATION config
#define RTL8188E_FPGA_TRUE_PHY_VERIFICATION 0

//***** temporarily flag *******
/*
 * Public  General Config
 */
#define AUTOCONF_INCLUDED
#define DRV_NAME "rtl8821au"

#define PLATFORM_LINUX	1


//#define CONFIG_IOCTL_CFG80211 1

/*
 * Internal  General Config
 */
//#define CONFIG_PWRCTRL

#define CONFIG_EMBEDDED_FWIMG	1

#define CONFIG_80211N_HT	1

#ifdef CONFIG_80211N_HT
	#define CONFIG_80211AC_VHT 1
#endif

#define CONFIG_RECV_REORDERING_CTRL	1

#define CONFIG_DFS	1

	#define CONFIG_IPS	0	/* ULLI: fix for powermode on low bitrate streaming */
	#ifdef CONFIG_IPS
	//#define CONFIG_IPS_LEVEL_2	1 //enable this to set default IPS mode to IPS_LEVEL_2
	#endif

	#define CONFIG_LPS	1

	//befor link
	//#define CONFIG_ANTENNA_DIVERSITY

	//after link
	#ifdef CONFIG_ANTENNA_DIVERSITY
	#define CONFIG_HW_ANTENNA_DIVERSITY
	#endif


#define CONFIG_AP_MODE	1
#ifdef CONFIG_AP_MODE
#endif



#define CONFIG_SKB_COPY	1//for amsdu

#define USB_INTERFERENCE_ISSUE // this should be checked in all usb interface

#define CONFIG_NEW_SIGNAL_STAT_PROCESS
#define RTW_NOTCH_FILTER 0 /* 0:Disable, 1:Enable, */

#define CONFIG_TX_MCAST2UNI	1	// Support IP multicast->unicast


/*
 * Interface  Related Config
 */

#ifndef CONFIG_MINIMAL_MEMORY_USAGE
	#define CONFIG_USB_TX_AGGREGATION	1
	#define CONFIG_USB_RX_AGGREGATION	1
#endif

#define CONFIG_PREALLOC_RECV_SKB	1
/*
 * CONFIG_USE_USB_BUFFER_ALLOC_XX uses Linux USB Buffer alloc API and is for Linux platform only now!
 */

#define CONFIG_USB_VENDOR_REQ_BUFFER_PREALLOC

#define CONFIG_USB_VENDOR_REQ_MUTEX
#define CONFIG_VENDOR_REQ_RETRY

/*
 * HAL  Related Config
 */
#define RTL8812A_RX_PACKET_INCLUDE_CRC	0

//#define CONFIG_ONLY_ONE_OUT_EP_TO_LOW	0

#define CONFIG_OUT_EP_WIFI_MODE	0

#define ENABLE_USB_DROP_INCORRECT_OUT	0

/* ULLI symbol used in usb_halinit.c */
#define RTL8192CU_ADHOC_WORKAROUND_SETTING	1

#define ENABLE_NEW_RFE_TYPE	0

/*
 * Platform  Related Config
 */





/*
 * Outsource  Related Config
 */

#define RATE_ADAPTIVE_SUPPORT 		0
#define POWER_TRAINING_ACTIVE			0


#ifdef CONFIG_USB_TX_AGGREGATION
#endif

#define	RTL8188E_EARLY_MODE_PKT_NUM_10	0

#define CONFIG_80211D

#define CONFIG_ATTEMPT_TO_FIX_AP_BEACON_ERROR

/*
 * Debug Related Config
 */
#define DBG	1

#define CONFIG_DEBUG /* DBG_871X, etc... */
//#define CONFIG_DEBUG_RTL871X /* RT_TRACE, RT_PRINT_DATA, _func_enter_, _func_exit_ */

#define CONFIG_PROC_DEBUG


//#define DBG_TX

//#define DBG_RX_SIGNAL_DISPLAY_SSID_MONITORED "jeff-ap"



//#define DBG_ROAMING_TEST

//#define DBG_HAL_INIT_PROFILING


