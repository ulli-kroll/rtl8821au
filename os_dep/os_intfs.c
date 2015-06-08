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
#define _OS_INTFS_C_

#include <drv_types.h>
#include <rtw_debug.h>


MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Realtek Wireless Lan Driver");
MODULE_AUTHOR("Realtek Semiconductor Corp.");
MODULE_VERSION("git. based on v4.2.2_7502.20130517");

/* module param defaults */
static int rtw_chip_version = 0x00;
static int rtw_rfintfs = HWPI;
static int rtw_lbkmode = 0;			/* RTL8712_AIR_TRX; */


static int rtw_network_mode = Ndis802_11IBSS;	/* Ndis802_11Infrastructure; infra, ad-hoc, auto */
					/* NDIS_802_11_SSID	ssid; */
static int rtw_channel = 1;/* ad-hoc support requirement */
static int rtw_wireless_mode = WIRELESS_MODE_MAX;
static int rtw_vrtl_carrier_sense = AUTO_VCS;
static int rtw_vcs_type = RTS_CTS;
static int rtw_rts_thresh = 2347;
static int rtw_frag_thresh = 2346;
static int rtw_preamble = PREAMBLE_LONG;	/* long, short, auto */
static int rtw_scan_mode = 1;		/* active, passive */
static int rtw_adhoc_tx_pwr = 1;
static int rtw_soft_ap = 0;
				/* int smart_ps = 1; */
#ifdef CONFIG_POWER_SAVING
static int rtw_power_mgnt = 1;
#ifdef CONFIG_IPS_LEVEL_2
static int rtw_ips_mode = IPS_LEVEL_2;
#else
static int rtw_ips_mode = IPS_NORMAL;
#endif
#else
static int rtw_power_mgnt = PS_MODE_ACTIVE;
static int rtw_ips_mode = IPS_NONE;
#endif

static int rtw_smart_ps = 2;

module_param(rtw_ips_mode, int, 0644);
MODULE_PARM_DESC(rtw_ips_mode, "The default IPS mode");

static int rtw_radio_enable = 1;
static int rtw_long_retry_lmt = 7;
static int rtw_short_retry_lmt = 7;
static int rtw_busy_thresh = 40;
/* int qos_enable = 0; */
static int rtw_ack_policy = NORMAL_ACK;

static int rtw_software_encrypt = 0;
static int rtw_software_decrypt = 0;

static int rtw_acm_method = 0;			/* 0:By SW 1:By HW. */

static int rtw_wmm_enable = 1;		/* default is set to enable the wmm. */
static int rtw_uapsd_enable = 0;
static int rtw_uapsd_max_sp = NO_LIMIT;
static int rtw_uapsd_acbk_en = 0;
static int rtw_uapsd_acbe_en = 0;
static int rtw_uapsd_acvi_en = 0;
static int rtw_uapsd_acvo_en = 0;

#ifdef CONFIG_80211N_HT
static int rtw_ht_enable = 1;

/*
 *  0: 20 MHz, 1: 40 MHz, 2: 80 MHz, 3: 160MHz, 4: 80+80MHz
 *  2.4G use bit 0 ~ 3, 5G use bit 4 ~ 7
 * 0x21 means enable 2.4G 40MHz & 5G 80MHz
 */
static int rtw_bw_mode = 0x21;
static int rtw_cbw40_enable = 3;	/* 0 :diable, bit(0): enable 2.4g, bit(1): enable 5g */
static int rtw_ampdu_enable = 1;	/* for enable tx_ampdu */
static int rtw_rx_stbc = 1;		/* 0: disable, bit(0):enable 2.4g, bit(1):enable 5g, default is set to enable 2.4GHZ for IOT issue with bufflao's AP at 5GHZ */

static int rtw_ampdu_amsdu = 0;/*  0: disabled, 1:enabled, 2:auto */
/*
 *  Short GI support Bit Map
 *  BIT0 - 20MHz, 0: support, 1: non-support
 *  BIT1 - 40MHz, 0: support, 1: non-support
 *  BIT2 - 80MHz, 0: support, 1: non-support
 *  BIT3 - 160MHz, 0: support, 1: non-support
 */
static int rtw_short_gi = 0xf;
#endif

#ifdef CONFIG_80211AC_VHT
static int rtw_vht_enable = 1;
static int rtw_ampdu_factor = 7;
static int rtw_vht_rate_sel = 0;
/*
 *
 *  BIT0: Enable VHT LDPC Rx,
 *  BIT1: Enable VHT LDPC Tx,
 *  BIT4: Enable HT LDPC Rx,
 *  BIT5: Enable HT LDPC Tx
 */
static int rtw_ldpc_cap = 0x33;
/*  BIT0: Enable VHT STBC Rx,
 *  BIT1: Enable VHT STBC Tx,
 *  BIT4: Enable HT STBC Rx,
 *  BIT5: Enable HT STBC Tx
 */
static int rtw_stbc_cap = 0x3;
/*
 * BIT0: Enable VHT Beamformer,
 * BIT1: Enable VHT Beamformee,
 * BIT4: Enable HT Beamformer,
 * BIT5: Enable HT Beamformee
 */
static int rtw_beamform_cap = 0;
#endif /* CONFIG_80211AC_VHT */

static int rtw_lowrate_two_xmit = 1;/* Use 2 path Tx to transmit MCS0~7 and legacy mode */

/* int rf_config = RF_1T2R;   1T2R */
static int rtw_rf_config = RF_MAX_TYPE;  /* auto */
static int rtw_low_power = 0;
static int rtw_channel_plan = RT_CHANNEL_DOMAIN_MAX;


static int rtw_AcceptAddbaReq = _TRUE;/* 0:Reject AP's Add BA req, 1:Accept AP's Add BA req. */

static int rtw_antdiv_cfg = 2; /* 0:OFF , 1:ON, 2:decide by Efuse config */
static int rtw_antdiv_type = 0 ; /* 0:decide by efuse  1: for 88EE, 1Tx and 1RxCG are diversity.(2 Ant with SPDT), 2:  for 88EE, 1Tx and 2Rx are diversity.( 2 Ant, Tx and RxCG are both on aux port, RxCS is on main port ), 3: for 88EE, 1Tx and 1RxCG are fixed.(1Ant, Tx and RxCG are both on aux port) */


static int rtw_enusbss = 0;	/* 0:disable,1:enable */

static int rtw_hwpdn_mode = 2;	/* 0:disable,1:enable,2: by EFUSE config */

static int rtw_hwpwrp_detect = 0; /* HW power  ping detect 0:disable , 1:enable */

/* ULLI: check var rtw_hw_wps_pbc */
static int rtw_hw_wps_pbc = 1;

#ifdef CONFIG_TX_MCAST2UNI
int __rtw_mc2u_disable = 0;
#endif

#ifdef CONFIG_80211D
static int rtw_80211d = 0;
#endif

#ifdef CONFIG_REGULATORY_CTRL
static int rtw_regulatory_id = 2;
#else
static int rtw_regulatory_id = 0xff; /*  Regulatory tab id, 0xff = follow efuse's setting */
#endif
module_param(rtw_regulatory_id, int, 0644);

char *__rtw_initmac = 0;  /* temp mac address if users want to use instead of the mac address in Efuse */



module_param(__rtw_initmac, charp, 0644);
module_param(rtw_channel_plan, int, 0644);
module_param(rtw_chip_version, int, 0644);
module_param(rtw_rfintfs, int, 0644);
module_param(rtw_lbkmode, int, 0644);
module_param(rtw_network_mode, int, 0644);
module_param(rtw_channel, int, 0644);
module_param(rtw_wmm_enable, int, 0644);
module_param(rtw_vrtl_carrier_sense, int, 0644);
module_param(rtw_vcs_type, int, 0644);
module_param(rtw_busy_thresh, int, 0644);

#ifdef CONFIG_80211N_HT
module_param(rtw_ht_enable, int, 0644);
module_param(rtw_bw_mode, int, 0644);
module_param(rtw_ampdu_enable, int, 0644);
module_param(rtw_rx_stbc, int, 0644);
module_param(rtw_ampdu_amsdu, int, 0644);
#endif
#ifdef CONFIG_80211AC_VHT
module_param(rtw_vht_enable, int, 0644);
#endif

module_param(rtw_lowrate_two_xmit, int, 0644);

module_param(rtw_rf_config, int, 0644);
module_param(rtw_power_mgnt, int, 0644);
module_param(rtw_smart_ps, int, 0644);
module_param(rtw_low_power, int, 0644);

module_param(rtw_antdiv_cfg, int, 0644);
module_param(rtw_antdiv_type, int, 0644);

module_param(rtw_enusbss, int, 0644);
module_param(rtw_hwpdn_mode, int, 0644);
module_param(rtw_hwpwrp_detect, int, 0644);

module_param(rtw_hw_wps_pbc, int, 0644);

#ifdef CONFIG_TX_MCAST2UNI
module_param(__rtw_mc2u_disable, int, 0644);
#endif

#ifdef CONFIG_80211D
module_param(rtw_80211d, int, 0644);
MODULE_PARM_DESC(rtw_80211d, "Enable 802.11d mechanism");
#endif


static uint rtw_notch_filter = RTW_NOTCH_FILTER;
module_param(rtw_notch_filter, uint, 0644);
MODULE_PARM_DESC(rtw_notch_filter, "0:Disable, 1:Enable, 2:Enable only for P2P");

int _netdev_open(struct net_device *ndev);

#ifdef CONFIG_PROC_DEBUG
#define RTL8192C_PROC_NAME "rtl819xC"
static char rtw_proc_name[IFNAMSIZ];
static struct proc_dir_entry *rtw_proc;
static int	rtw_proc_cnt;

#define RTW_PROC_NAME DRV_NAME

#if 0
void rtw_proc_init_one(struct net_device *ndev)
{
	struct proc_dir_entry *dir_dev = NULL;
	struct proc_dir_entry *entry = NULL;
	struct rtl_priv	*rtlpriv = rtl_priv(dev);
	uint8_t rf_type;

	if (rtw_proc == NULL) {
		memcpy(rtw_proc_name, RTW_PROC_NAME, sizeof(RTW_PROC_NAME));

		rtw_proc = create_proc_entry(rtw_proc_name, S_IFDIR, init_net.proc_net);
		if (rtw_proc == NULL) {
			DBG_871X(KERN_ERR "Unable to create rtw_proc directory\n");
			return;
		}

		entry = create_proc_read_entry("ver_info", S_IFREG | S_IRUGO, rtw_proc, proc_get_drv_version, dev);
		if (!entry) {
			DBG_871X("Unable to create_proc_read_entry!\n");
			return;
		}
	}



	if (rtlpriv->dir_dev == NULL) {
		rtlpriv->dir_dev = create_proc_entry(dev->name,
					  S_IFDIR | S_IRUGO | S_IXUGO,
					  rtw_proc);

		dir_dev = rtlpriv->dir_dev;

		if (dir_dev == NULL) {
			if (rtw_proc_cnt == 0) {
				if (rtw_proc) {
					remove_proc_entry(rtw_proc_name, init_net.proc_net);
					rtw_proc = NULL;
				}
			}

			DBG_871X("Unable to create dir_dev directory\n");
			return;
		}
	} else {
		return;
	}

	rtw_proc_cnt++;

	entry = create_proc_read_entry("write_reg", S_IFREG | S_IRUGO,
				   dir_dev, proc_get_write_reg, dev);
	if (!entry) {
		DBG_871X("Unable to create_proc_read_entry!\n");
		return;
	}
	entry->write_proc = proc_set_write_reg;

	entry = create_proc_read_entry("read_reg", S_IFREG | S_IRUGO,
				   dir_dev, proc_get_read_reg, dev);
	if (!entry) {
		DBG_871X("Unable to create_proc_read_entry!\n");
		return;
	}
	entry->write_proc = proc_set_read_reg;


	entry = create_proc_read_entry("fwstate", S_IFREG | S_IRUGO,
				   dir_dev, proc_get_fwstate, dev);
	if (!entry) {
		DBG_871X("Unable to create_proc_read_entry!\n");
		return;
	}


	entry = create_proc_read_entry("sec_info", S_IFREG | S_IRUGO,
				   dir_dev, proc_get_sec_info, dev);
	if (!entry) {
		DBG_871X("Unable to create_proc_read_entry!\n");
		return;
	}


	entry = create_proc_read_entry("mlmext_state", S_IFREG | S_IRUGO,
				   dir_dev, proc_get_mlmext_state, dev);
	if (!entry) {
		DBG_871X("Unable to create_proc_read_entry!\n");
		return;
	}


	entry = create_proc_read_entry("qos_option", S_IFREG | S_IRUGO,
				   dir_dev, proc_get_qos_option, dev);
	if (!entry) {
		DBG_871X("Unable to create_proc_read_entry!\n");
		return;
	}

	entry = create_proc_read_entry("ht_option", S_IFREG | S_IRUGO,
				   dir_dev, proc_get_ht_option, dev);
	if (!entry) {
		DBG_871X("Unable to create_proc_read_entry!\n");
		return;
	}

	entry = create_proc_read_entry("rf_info", S_IFREG | S_IRUGO,
				   dir_dev, proc_get_rf_info, dev);
	if (!entry) {
		DBG_871X("Unable to create_proc_read_entry!\n");
		return;
	}

	entry = create_proc_read_entry("ap_info", S_IFREG | S_IRUGO,
				   dir_dev, proc_get_ap_info, dev);
	if (!entry) {
		DBG_871X("Unable to create_proc_read_entry!\n");
		return;
	}

	entry = create_proc_read_entry("adapter_state", S_IFREG | S_IRUGO,
				   dir_dev, proc_getstruct rtl_priv_state, dev);
	if (!entry) {
		DBG_871X("Unable to create_proc_read_entry!\n");
		return;
	}

	entry = create_proc_read_entry("trx_info", S_IFREG | S_IRUGO,
				   dir_dev, proc_get_trx_info, dev);
	if (!entry) {
		DBG_871X("Unable to create_proc_read_entry!\n");
		return;
	}

	entry = create_proc_read_entry("mac_reg_dump1", S_IFREG | S_IRUGO,
				   dir_dev, proc_get_mac_reg_dump1, dev);
	if (!entry) {
		DBG_871X("Unable to create_proc_read_entry!\n");
		return;
	}

	entry = create_proc_read_entry("mac_reg_dump2", S_IFREG | S_IRUGO,
				   dir_dev, proc_get_mac_reg_dump2, dev);
	if (!entry) {
		DBG_871X("Unable to create_proc_read_entry!\n");
		return;
	}

	entry = create_proc_read_entry("mac_reg_dump3", S_IFREG | S_IRUGO,
				   dir_dev, proc_get_mac_reg_dump3, dev);
	if (!entry) {
		DBG_871X("Unable to create_proc_read_entry!\n");
		return;
	}

	entry = create_proc_read_entry("bb_reg_dump1", S_IFREG | S_IRUGO,
				   dir_dev, proc_get_bb_reg_dump1, dev);
	if (!entry) {
		DBG_871X("Unable to create_proc_read_entry!\n");
		return;
	}

	entry = create_proc_read_entry("bb_reg_dump2", S_IFREG | S_IRUGO,
				   dir_dev, proc_get_bb_reg_dump2, dev);
	if (!entry) {
		DBG_871X("Unable to create_proc_read_entry!\n");
		return;
	}

	entry = create_proc_read_entry("bb_reg_dump3", S_IFREG | S_IRUGO,
				   dir_dev, proc_get_bb_reg_dump3, dev);
	if (!entry) {
		DBG_871X("Unable to create_proc_read_entry!\n");
		return;
	}

	entry = create_proc_read_entry("rf_reg_dump1", S_IFREG | S_IRUGO,
				   dir_dev, proc_get_rf_reg_dump1, dev);
	if (!entry) {
		DBG_871X("Unable to create_proc_read_entry!\n");
		return;
	}

	entry = create_proc_read_entry("rf_reg_dump2", S_IFREG | S_IRUGO,
				   dir_dev, proc_get_rf_reg_dump2, dev);
	if (!entry) {
		DBG_871X("Unable to create_proc_read_entry!\n");
		return;
	}

	rtw_hal_get_hwreg(rtlpriv, HW_VAR_RF_TYPE, (uint8_t *)(&rf_type));
	if ((RF_1T2R == rf_type) || (RF_1T1R == rf_type)) {
		entry = create_proc_read_entry("rf_reg_dump3", S_IFREG | S_IRUGO,
					   dir_dev, proc_get_rf_reg_dump3, dev);
		if (!entry) {
			DBG_871X("Unable to create_proc_read_entry!\n");
			return;
		}

		entry = create_proc_read_entry("rf_reg_dump4", S_IFREG | S_IRUGO,
					   dir_dev, proc_get_rf_reg_dump4, dev);
		if (!entry) {
			DBG_871X("Unable to create_proc_read_entry!\n");
			return;
		}
	}

#ifdef CONFIG_AP_MODE

	entry = create_proc_read_entry("all_sta_info", S_IFREG | S_IRUGO,
				   dir_dev, proc_get_all_sta_info, dev);
	if (!entry) {
		DBG_871X("Unable to create_proc_read_entry!\n");
		return;
	}
#endif

	entry = create_proc_read_entry("rx_signal", S_IFREG | S_IRUGO,
				   dir_dev, proc_get_rx_signal, dev);
	if (!entry) {
		DBG_871X("Unable to create_proc_read_entry!\n");
		return;
	}
	entry->write_proc = proc_set_rx_signal;
#ifdef CONFIG_80211N_HT
	entry = create_proc_read_entry("ht_enable", S_IFREG | S_IRUGO,
				   dir_dev, proc_get_ht_enable, dev);
	if (!entry) {
		DBG_871X("Unable to create_proc_read_entry!\n");
		return;
	}
	entry->write_proc = proc_set_ht_enable;

	entry = create_proc_read_entry("bw_mode", S_IFREG | S_IRUGO,
				   dir_dev, proc_get_bw_mode, dev);
	if (!entry) {
		DBG_871X("Unable to create_proc_read_entry!\n");
		return;
	}
	entry->write_proc = proc_set_bw_mode;

	entry = create_proc_read_entry("ampdu_enable", S_IFREG | S_IRUGO,
				   dir_dev, proc_get_ampdu_enable, dev);
	if (!entry) {
		DBG_871X("Unable to create_proc_read_entry!\n");
		return;
	}
	entry->write_proc = proc_set_ampdu_enable;

	entry = create_proc_read_entry("rx_stbc", S_IFREG | S_IRUGO,
				   dir_dev, proc_get_rx_stbc, dev);
	if (!entry) {
		DBG_871X("Unable to create_proc_read_entry!\n");
		return;
	}
	entry->write_proc = proc_set_rx_stbc;
#endif

	entry = create_proc_read_entry("path_rssi", S_IFREG | S_IRUGO,
					dir_dev, proc_get_two_path_rssi, dev);

	entry = create_proc_read_entry("rssi_disp", S_IFREG | S_IRUGO,
				   dir_dev, proc_get_rssi_disp, dev);
	if (!entry) {
		DBG_871X("Unable to create_proc_read_entry!\n");
		return;
	}
	entry->write_proc = proc_set_rssi_disp;

#if defined(DBG_CONFIG_ERROR_DETECT)
	entry = create_proc_read_entry("sreset", S_IFREG | S_IRUGO,
				   dir_dev, proc_get_sreset, dev);
	if (!entry) {
		DBG_871X("Unable to create_proc_read_entry!\n");
		return;
	}
	entry->write_proc = proc_set_sreset;
#endif

}

void rtw_proc_remove_one(struct net_device *ndev)
{
	struct proc_dir_entry *dir_dev = NULL;
	struct rtl_priv	*rtlpriv = rtl_priv(dev);
	uint8_t rf_type;

	dir_dev = rtlpriv->dir_dev;
	rtlpriv->dir_dev = NULL;

	if (dir_dev) {

		remove_proc_entry("write_reg", dir_dev);
		remove_proc_entry("read_reg", dir_dev);
		remove_proc_entry("fwstate", dir_dev);
		remove_proc_entry("sec_info", dir_dev);
		remove_proc_entry("mlmext_state", dir_dev);
		remove_proc_entry("qos_option", dir_dev);
		remove_proc_entry("ht_option", dir_dev);
		remove_proc_entry("rf_info", dir_dev);
		remove_proc_entry("ap_info", dir_dev);
		remove_proc_entry("adapter_state", dir_dev);
		remove_proc_entry("trx_info", dir_dev);

		remove_proc_entry("mac_reg_dump1", dir_dev);
		remove_proc_entry("mac_reg_dump2", dir_dev);
		remove_proc_entry("mac_reg_dump3", dir_dev);
		remove_proc_entry("bb_reg_dump1", dir_dev);
		remove_proc_entry("bb_reg_dump2", dir_dev);
		remove_proc_entry("bb_reg_dump3", dir_dev);
		remove_proc_entry("rf_reg_dump1", dir_dev);
		remove_proc_entry("rf_reg_dump2", dir_dev);
		rtw_hal_get_hwreg(rtlpriv, HW_VAR_RF_TYPE, (uint8_t *)(&rf_type));
		if ((RF_1T2R == rf_type) || (RF_1T1R == rf_type)) {
			remove_proc_entry("rf_reg_dump3", dir_dev);
			remove_proc_entry("rf_reg_dump4", dir_dev);
		}
#ifdef CONFIG_AP_MODE
		remove_proc_entry("all_sta_info", dir_dev);
#endif
		remove_proc_entry("rx_signal", dir_dev);
#ifdef CONFIG_80211N_HT
		remove_proc_entry("bw_mode", dir_dev);

		remove_proc_entry("ht_enable", dir_dev);

		remove_proc_entry("ampdu_enable", dir_dev);

		remove_proc_entry("rx_stbc", dir_dev);
#endif
		remove_proc_entry("path_rssi", dir_dev);

		remove_proc_entry("rssi_disp", dir_dev);


#if defined(DBG_CONFIG_ERROR_DETECT)
		remove_proc_entry("sreset", dir_dev);
#endif

		remove_proc_entry(dev->name, rtw_proc);
		dir_dev = NULL;

	} else {
		return;
	}

	rtw_proc_cnt--;

	if (rtw_proc_cnt == 0) {
		if (rtw_proc) {
			remove_proc_entry("ver_info", rtw_proc);
			remove_proc_entry(rtw_proc_name, init_net.proc_net);
			rtw_proc = NULL;
		}
	}
}
#endif
#endif

uint loadparam(struct rtl_priv *rtlpriv, struct net_device *ndev)
{

	uint status = _SUCCESS;
	struct registry_priv  *registry_par = &rtlpriv->registrypriv;

	registry_par->chip_version = (uint8_t)rtw_chip_version;
	registry_par->rfintfs = (uint8_t)rtw_rfintfs;
	registry_par->lbkmode = (uint8_t)rtw_lbkmode;
	/* registry_par->hci = (uint8_t)hci; */
	registry_par->network_mode  = (uint8_t)rtw_network_mode;

	memcpy(registry_par->ssid.Ssid, "ANY", 3);
	registry_par->ssid.SsidLength = 3;

	registry_par->channel = (uint8_t)rtw_channel;
	registry_par->wireless_mode = (uint8_t)rtw_wireless_mode;

	if (IsSupported24G(registry_par->wireless_mode) && (!IsSupported5G(registry_par->wireless_mode))
		&& (registry_par->channel > 14)) {
		registry_par->channel = 1;
	} else if (IsSupported5G(registry_par->wireless_mode) && (!IsSupported24G(registry_par->wireless_mode))
		&& (registry_par->channel <= 14)) {
		registry_par->channel = 36;
	}

	registry_par->vrtl_carrier_sense = (uint8_t)rtw_vrtl_carrier_sense;
	registry_par->vcs_type = (uint8_t)rtw_vcs_type;
	registry_par->rts_thresh = (u16)rtw_rts_thresh;
	registry_par->frag_thresh = (u16)rtw_frag_thresh;
	registry_par->preamble = (uint8_t)rtw_preamble;
	registry_par->scan_mode = (uint8_t)rtw_scan_mode;
	registry_par->adhoc_tx_pwr = (uint8_t)rtw_adhoc_tx_pwr;
	registry_par->soft_ap =  (uint8_t)rtw_soft_ap;
	registry_par->smart_ps =  (uint8_t)rtw_smart_ps;
	registry_par->power_mgnt = (uint8_t)rtw_power_mgnt;
	registry_par->ips_mode = (uint8_t)rtw_ips_mode;
	registry_par->radio_enable = (uint8_t)rtw_radio_enable;
	registry_par->long_retry_lmt = (uint8_t)rtw_long_retry_lmt;
	registry_par->short_retry_lmt = (uint8_t)rtw_short_retry_lmt;
	registry_par->busy_thresh = (u16)rtw_busy_thresh;
	/* registry_par->qos_enable = (uint8_t)rtw_qos_enable; */
	registry_par->ack_policy = (uint8_t)rtw_ack_policy;
	registry_par->software_encrypt = (uint8_t)rtw_software_encrypt;
	registry_par->software_decrypt = (uint8_t)rtw_software_decrypt;

	registry_par->acm_method = (uint8_t)rtw_acm_method;

	 /* UAPSD */
	registry_par->wmm_enable = (uint8_t)rtw_wmm_enable;
	registry_par->uapsd_enable = (uint8_t)rtw_uapsd_enable;
	registry_par->uapsd_max_sp = (uint8_t)rtw_uapsd_max_sp;
	registry_par->uapsd_acbk_en = (uint8_t)rtw_uapsd_acbk_en;
	registry_par->uapsd_acbe_en = (uint8_t)rtw_uapsd_acbe_en;
	registry_par->uapsd_acvi_en = (uint8_t)rtw_uapsd_acvi_en;
	registry_par->uapsd_acvo_en = (uint8_t)rtw_uapsd_acvo_en;

#ifdef CONFIG_80211N_HT
	registry_par->ht_enable = (uint8_t)rtw_ht_enable;
	registry_par->bw_mode = (uint8_t)rtw_bw_mode;
	registry_par->ampdu_enable = (uint8_t)rtw_ampdu_enable;
	registry_par->rx_stbc = (uint8_t)rtw_rx_stbc;
	registry_par->ampdu_amsdu = (uint8_t)rtw_ampdu_amsdu;
	registry_par->short_gi = (uint8_t)rtw_short_gi;
#endif

#ifdef CONFIG_80211AC_VHT
	registry_par->vht_enable = (uint8_t)rtw_vht_enable;
	registry_par->ampdu_factor = (uint8_t)rtw_ampdu_factor;
	registry_par->vht_rate_sel = (uint8_t)rtw_vht_rate_sel;
	registry_par->ldpc_cap = (uint8_t)rtw_ldpc_cap;
	registry_par->stbc_cap = (uint8_t)rtw_stbc_cap;
	registry_par->beamform_cap = (uint8_t)rtw_beamform_cap;
#endif
	registry_par->lowrate_two_xmit = (uint8_t)rtw_lowrate_two_xmit;
	registry_par->rf_config = (uint8_t)rtw_rf_config;
	registry_par->low_power = (uint8_t)rtw_low_power;

	registry_par->channel_plan = (uint8_t)rtw_channel_plan;


	registry_par->bAcceptAddbaReq = (uint8_t)rtw_AcceptAddbaReq;

	registry_par->antdiv_cfg = (uint8_t)rtw_antdiv_cfg;
	registry_par->antdiv_type = (uint8_t)rtw_antdiv_type;

#ifdef CONFIG_AUTOSUSPEND
	registry_par->usbss_enable = (uint8_t)rtw_enusbss;	/* 0:disable,1:enable */
#endif

	registry_par->hw_wps_pbc = (uint8_t)rtw_hw_wps_pbc;

#ifdef CONFIG_80211D
	registry_par->enable80211d = (uint8_t)rtw_80211d;
#endif

	registry_par->notch_filter = (uint8_t)rtw_notch_filter;

	registry_par->regulatory_tid = (uint8_t)rtw_regulatory_id;




	return status;
}

int rtw_net_set_mac_address(struct net_device *ndev, void *p)
{
	struct rtl_priv *rtlpriv = rtl_priv(ndev);
	struct sockaddr *addr = p;

	if (rtlpriv->bup == _FALSE) {
		/*
		 * DBG_871X("r8711_net_set_mac_address(), MAC=%x:%x:%x:%x:%x:%x\n", addr->sa_data[0], addr->sa_data[1], addr->sa_data[2], addr->sa_data[3],
		 * addr->sa_data[4], addr->sa_data[5]);
		 */
		memcpy(rtlpriv->eeprompriv.mac_addr, addr->sa_data, ETH_ALEN);
		/*
		 * memcpy(ndev->dev_addr, addr->sa_data, ETH_ALEN);
		 * rtlpriv->bset_hwaddr = _TRUE;
		 */
	}

	return 0;
}

struct net_device_stats *rtw_net_get_stats(struct net_device *ndev)
{
	struct rtl_priv *rtlpriv = rtl_priv(ndev);
	struct xmit_priv *pxmitpriv = &(rtlpriv->xmitpriv);
	struct recv_priv *precvpriv = &(rtlpriv->recvpriv);

	rtlpriv->stats.tx_packets = pxmitpriv->tx_pkts;	/* pxmitpriv->tx_pkts++; */
	rtlpriv->stats.rx_packets = precvpriv->rx_pkts;	/* precvpriv->rx_pkts++; */
	rtlpriv->stats.tx_dropped = pxmitpriv->tx_drop;
	rtlpriv->stats.rx_dropped = precvpriv->rx_drop;
	rtlpriv->stats.tx_bytes = pxmitpriv->tx_bytes;
	rtlpriv->stats.rx_bytes = precvpriv->rx_bytes;

	return &rtlpriv->stats;
}

/*
 * AC to queue mapping
 *
 * AC_VO -> queue 0
 * AC_VI -> queue 1
 * AC_BE -> queue 2
 * AC_BK -> queue 3
 */
static const u16 rtw_1d_to_queue[8] = { 2, 3, 3, 2, 1, 1, 0, 0 };

/* Given a data frame determine the 802.1p/1d tag to use. */
unsigned int rtw_classify8021d(struct sk_buff *skb)
{
	unsigned int dscp;

	/* skb->priority values from 256->263 are magic values to
	 * directly indicate a specific 802.1d priority.  This is used
	 * to allow 802.1d priority to be passed directly in from VLAN
	 * tags, etc.
	 */
	if (skb->priority >= 256 && skb->priority <= 263)
		return skb->priority - 256;

	switch (skb->protocol) {
	case htons(ETH_P_IP):
		dscp = ip_hdr(skb)->tos & 0xfc;
		break;
	default:
		return 0;
	}

	return dscp >> 5;
}

u16 rtw_recv_select_queue(struct sk_buff *skb)
{
	struct iphdr *piphdr;
	unsigned int dscp;
	u16	eth_type;
	u32 priority;
	uint8_t *pdata = skb->data;

	memcpy(&eth_type, pdata+(ETH_ALEN<<1), 2);

	switch (eth_type) {
	case htons(ETH_P_IP):

		piphdr = (struct iphdr *)(pdata+ETH_HLEN);

		dscp = piphdr->tos & 0xfc;

		priority = dscp >> 5;

		break;
	default:
		priority = 0;
	}

	return rtw_1d_to_queue[priority];

}

u32 rtw_start_drv_threads(struct rtl_priv *rtlpriv)
{
	u32 _status = _SUCCESS;
	int _unused;

	{
		rtlpriv->cmdThread = kthread_run(rtw_cmd_thread, rtlpriv, "RTW_CMD_THREAD");
		if (IS_ERR(rtlpriv->cmdThread))
			_status = _FAIL;
		else
			_unused = down_interruptible(&rtlpriv->cmdpriv.terminate_cmdthread_sema); /* wait for cmd_thread to run */
	}
	return _status;

}

void rtw_stop_drv_threads(struct rtl_priv *rtlpriv)
{
	int _unused;
	
	/* Below is to termindate rtw_cmd_thread & event_thread... */
	up(&rtlpriv->cmdpriv.cmd_queue_sema);
	/* up(&rtlpriv->cmdpriv.cmd_done_sema); */
	if (rtlpriv->cmdThread) {
		_unused = down_interruptible(&rtlpriv->cmdpriv.terminate_cmdthread_sema);
	}
}

uint8_t rtw_init_default_value(struct rtl_priv *rtlpriv);
uint8_t rtw_init_default_value(struct rtl_priv *rtlpriv)
{
	uint8_t ret  = _SUCCESS;
	struct registry_priv *pregistrypriv = &rtlpriv->registrypriv;
	struct xmit_priv	*pxmitpriv = &rtlpriv->xmitpriv;
	struct mlme_priv *pmlmepriv = &rtlpriv->mlmepriv;
	struct security_priv *psecuritypriv = &rtlpriv->securitypriv;

	/* xmit_priv */
	pxmitpriv->vcs_setting = pregistrypriv->vrtl_carrier_sense;
	pxmitpriv->vcs = pregistrypriv->vcs_type;
	pxmitpriv->vcs_type = pregistrypriv->vcs_type;
	/* pxmitpriv->rts_thresh = pregistrypriv->rts_thresh; */
	pxmitpriv->frag_len = pregistrypriv->frag_thresh;

	/* recv_priv */

	/* mlme_priv */
	pmlmepriv->scan_interval = SCAN_INTERVAL;	/* 30*2 sec = 60sec */
	pmlmepriv->scan_mode = SCAN_ACTIVE;

	/*
	 * qos_priv
	 * pmlmepriv->qospriv.qos_option = pregistrypriv->wmm_enable;
	 */

	/* ht_priv */
#ifdef CONFIG_80211N_HT
	pmlmepriv->htpriv.ampdu_enable = _FALSE; /* set to disabled */
#endif

	/* security_priv */
	/* rtw_get_encrypt_decrypt_from_registrypriv(rtlpriv); */
	psecuritypriv->binstallGrpkey = _FAIL;
	psecuritypriv->sw_encrypt = pregistrypriv->software_encrypt;
	psecuritypriv->sw_decrypt = pregistrypriv->software_decrypt;

	psecuritypriv->dot11AuthAlgrthm = dot11AuthAlgrthm_Open; /* open system */
	psecuritypriv->dot11PrivacyAlgrthm = _NO_PRIVACY_;

	psecuritypriv->dot11PrivacyKeyIndex = 0;

	psecuritypriv->dot118021XGrpPrivacy = _NO_PRIVACY_;
	psecuritypriv->dot118021XGrpKeyid = 1;

	psecuritypriv->ndisauthtype = Ndis802_11AuthModeOpen;
	psecuritypriv->ndisencryptstatus = Ndis802_11WEPDisabled;


	/* pwrctrl_priv */


	/* registry_priv */
	rtw_init_registrypriv_dev_network(rtlpriv);
	rtw_update_registrypriv_dev_network(rtlpriv);


	/* hal_priv */
	rtw_hal_def_value_init(rtlpriv);

	/* misc. */
	rtlpriv->bReadPortCancel = _FALSE;
	rtlpriv->bWritePortCancel = _FALSE;
	rtlpriv->bLinkInfoDump = 0;
	rtlpriv->bNotifyChannelChange = 0;

	return ret;
}

uint8_t rtw_reset_drv_sw(struct rtl_priv *rtlpriv)
{
	uint8_t	ret8 = _SUCCESS;
	struct mlme_priv *pmlmepriv = &rtlpriv->mlmepriv;
	struct pwrctrl_priv *pwrctrlpriv = &rtlpriv->pwrctrlpriv;

	/* hal_priv */
	rtw_hal_def_value_init(rtlpriv);
	rtlpriv->bReadPortCancel = _FALSE;
	rtlpriv->bWritePortCancel = _FALSE;
	rtlpriv->bLinkInfoDump = 0;
	pmlmepriv->scan_interval = SCAN_INTERVAL;	/* 30*2 sec = 60sec */

	rtlpriv->xmitpriv.tx_pkts = 0;
	rtlpriv->recvpriv.rx_pkts = 0;

	pmlmepriv->LinkDetectInfo.bBusyTraffic = _FALSE;

	_clr_fwstate_(pmlmepriv, _FW_UNDER_SURVEY | _FW_UNDER_LINKING);

#ifdef CONFIG_AUTOSUSPEND
#endif

	pwrctrlpriv->pwr_state_check_cnts = 0;

	/* mlmeextpriv */
	rtlpriv->mlmeextpriv.sitesurvey_res.state = SCAN_DISABLE;

#ifdef CONFIG_NEW_SIGNAL_STAT_PROCESS
	rtw_set_signal_stat_timer(&rtlpriv->recvpriv);
#endif

	return ret8;
}


int rtl8821au_init_sw_vars(struct net_device *ndev)
{
	struct rtl_priv *rtlpriv = rtl_priv(ndev);

	uint8_t	ret8 = _SUCCESS;

	ret8 = rtw_init_default_value(rtlpriv);

	if ((rtw_init_cmd_priv(&rtlpriv->cmdpriv)) == _FAIL) {
		ret8 = _FAIL;
		goto exit;
	}

	rtlpriv->cmdpriv.rtlpriv = rtlpriv;

	if ((rtw_init_evt_priv(&rtlpriv->evtpriv)) == _FAIL) {
		ret8 = _FAIL;
		goto exit;
	}


	if (rtw_init_mlme_priv(rtlpriv) == _FAIL) {
		ret8 = _FAIL;
		goto exit;
	}

	if (init_mlme_ext_priv(rtlpriv) == _FAIL) {
		ret8 = _FAIL;
		goto exit;
	}

	if (_rtw_init_xmit_priv(&rtlpriv->xmitpriv, rtlpriv) == _FAIL) 	{
		DBG_871X("Can't _rtw_init_xmit_priv\n");
		ret8 = _FAIL;
		goto exit;
	}

	if (_rtw_init_recv_priv(&rtlpriv->recvpriv, rtlpriv) == _FAIL) {
		DBG_871X("Can't _rtw_init_recv_priv\n");
		ret8 = _FAIL;
		goto exit;
	}

	/*
	 * We don't need to memset rtlpriv->XXX to zero, because rtlpriv is allocated by rtw_zvmalloc().
	 * memset((unsigned char *)&rtlpriv->securitypriv, 0, sizeof (struct security_priv));
	 */

	/* _init_timer(&(rtlpriv->securitypriv.tkip_timer), rtlpriv->pifp, rtw_use_tkipkey_handler, rtlpriv); */

	if (_rtw_init_sta_priv(&rtlpriv->stapriv) == _FAIL) {
		DBG_871X("Can't _rtw_init_sta_priv\n");
		ret8 = _FAIL;
		goto exit;
	}

	rtlpriv->stapriv.rtlpriv = rtlpriv;
	rtlpriv->setband = GHZ24_50;
	rtw_init_bcmc_stainfo(rtlpriv);

	rtw_init_pwrctrl_priv(rtlpriv);

	/* memset((uint8_t *)&rtlpriv->qospriv, 0, sizeof (struct qos_priv));//move to mlme_priv */


	rtw_hal_dm_init(rtlpriv);
	rtw_hal_sw_led_init(rtlpriv);

exit:

	return ret8;

}

void rtw_cancel_all_timer(struct rtl_priv *rtlpriv)
{
	_cancel_timer_ex(&rtlpriv->mlmepriv.assoc_timer);

	/*
	 * _cancel_timer_ex(&rtlpriv->securitypriv.tkip_timer);
	 * RT_TRACE(_module_os_intfs_c_,_drv_info_,("rtw_cancel_all_timer:cancel tkip_timer! \n"));
	 */

	_cancel_timer_ex(&rtlpriv->mlmepriv.scan_to_timer);
	_cancel_timer_ex(&rtlpriv->mlmepriv.dynamic_chk_timer);

	/* cancel sw led timer */
	rtw_hal_sw_led_deinit(rtlpriv);

	_cancel_timer_ex(&rtlpriv->pwrctrlpriv.pwr_state_check_timer);


#ifdef CONFIG_NEW_SIGNAL_STAT_PROCESS
	_cancel_timer_ex(&rtlpriv->recvpriv.signal_stat_timer);
#endif
	/* cancel dm timer */
	rtw_hal_dm_deinit(rtlpriv);

}

uint8_t rtw_free_drv_sw(struct rtl_priv *rtlpriv)
{
	struct net_device *ndev = (struct net_device *)rtlpriv->ndev;

	/*
	 * we can call rtw_p2p_enable here, but:
	 * 1. rtw_p2p_enable may have IO operation
	 * 2. rtw_p2p_enable is bundled with wext interface
	 */

	free_mlme_ext_priv(&rtlpriv->mlmeextpriv);


	rtw_free_cmd_priv(&rtlpriv->cmdpriv);

	rtw_free_evt_priv(&rtlpriv->evtpriv);

	rtw_free_mlme_priv(&rtlpriv->mlmepriv);

	/* free_io_queue(rtlpriv); */

	_rtw_free_xmit_priv(&rtlpriv->xmitpriv);

	_rtw_free_sta_priv(&rtlpriv->stapriv); /* will free bcmc_stainfo here */

	_rtw_free_recv_priv(&rtlpriv->recvpriv);

	/* rtw_mfree(rtlpriv); */

	rtw_hal_free_data(rtlpriv);

	return _SUCCESS;

}

int _netdev_open(struct net_device *ndev)
{
	uint status;
	struct rtl_priv *rtlpriv =  rtl_priv(ndev);
	struct pwrctrl_priv *pwrctrlpriv = &rtlpriv->pwrctrlpriv;

	DBG_871X("+871x_drv - drv_open, bup=%d\n", rtlpriv->bup);

	if (pwrctrlpriv->ps_flag == _TRUE) {
		rtlpriv->net_closed = _FALSE;
		goto netdev_open_normal_process;
	}

	if (rtlpriv->bup == _FALSE) {
		rtlpriv->bDriverStopped = _FALSE;
		rtlpriv->bSurpriseRemoved = _FALSE;
		rtlpriv->bCardDisableWOHSM = _FALSE;

		status = rtw_hal_init(rtlpriv);
		if (status == _FAIL) {
			goto netdev_open_error;
		}

		DBG_871X("MAC Address = "MAC_FMT"\n", MAC_ARG(ndev->dev_addr));

		status = rtw_start_drv_threads(rtlpriv);
		if (status == _FAIL) {
			DBG_871X("Initialize driver software resource Failed!\n");
			goto netdev_open_error;
		}

		if (init_hw_mlme_ext(rtlpriv) == _FAIL) {
			DBG_871X("can't init mlme_ext_priv\n");
			goto netdev_open_error;
		}

		if (rtlpriv->intf_start) {
			rtlpriv->intf_start(rtlpriv);
		}
		rtw_hal_led_control(rtlpriv, LED_CTL_NO_LINK);

		rtlpriv->bup = _TRUE;
	}
	rtlpriv->net_closed = _FALSE;

	_set_timer(&rtlpriv->mlmepriv.dynamic_chk_timer, 2000);

	rtlpriv->pwrctrlpriv.bips_processing = _FALSE;
	rtw_set_pwr_state_check_timer(&rtlpriv->pwrctrlpriv);

	/*
	 * netif_carrier_on(ndev);//call this func when rtw_joinbss_event_callback return success
	 */
	if (!rtw_netif_queue_stopped(ndev))
		rtw_netif_start_queue(ndev);
	else
		rtw_netif_wake_queue(ndev);

netdev_open_normal_process:

	DBG_871X("-871x_drv - drv_open, bup=%d\n", rtlpriv->bup);

	return 0;

netdev_open_error:

	rtlpriv->bup = _FALSE;

	netif_carrier_off(ndev);
	rtw_netif_stop_queue(ndev);

	DBG_871X("-871x_drv - drv_open fail, bup=%d\n", rtlpriv->bup);

	return (-1);

}

int netdev_open(struct net_device *ndev)
{
	int ret;
	int _unused;
	
	struct rtl_priv *rtlpriv =  rtl_priv(ndev);

	/* ULLI: orignal driver doesn't use the return value */
	_unused = mutex_lock_interruptible(&(rtl_usbdev(rtlpriv)->hw_init_mutex));
	ret = _netdev_open(ndev);
	mutex_unlock(&(rtl_usbdev(rtlpriv)->hw_init_mutex));

	return ret;
}

#ifdef CONFIG_IPS
int  ips_netdrv_open(struct rtl_priv *rtlpriv)
{
	int status = _SUCCESS;
	rtlpriv->net_closed = _FALSE;

	DBG_871X("===> %s.........\n", __FUNCTION__);

	rtlpriv->bDriverStopped = _FALSE;
	rtlpriv->bSurpriseRemoved = _FALSE;
	rtlpriv->bCardDisableWOHSM = _FALSE;
	/* rtlpriv->bup = _TRUE; */

	status = rtw_hal_init(rtlpriv);
	if (status == _FAIL) {
		goto netdev_open_error;
	}

	if (rtlpriv->intf_start) {
		rtlpriv->intf_start(rtlpriv);
	}

	rtw_set_pwr_state_check_timer(&rtlpriv->pwrctrlpriv);
	_set_timer(&rtlpriv->mlmepriv.dynamic_chk_timer, 5000);

	 return _SUCCESS;

netdev_open_error:
	/* rtlpriv->bup = _FALSE; */
	DBG_871X("-ips_netdrv_open - drv_open failure, bup=%d\n", rtlpriv->bup);

	return _FAIL;
}


int rtw_ips_pwr_up(struct rtl_priv *rtlpriv)
{
	int result;
	u32 start_time = jiffies;

	DBG_871X("===>  rtw_ips_pwr_up..............\n");
	rtw_reset_drv_sw(rtlpriv);

	result = ips_netdrv_open(rtlpriv);

	rtw_hal_led_control(rtlpriv, LED_CTL_NO_LINK);

	DBG_871X("<===  rtw_ips_pwr_up.............. in %dms\n", rtw_get_passing_time_ms(start_time));
	return result;

}

void rtw_ips_pwr_down(struct rtl_priv *rtlpriv)
{
	u32 start_time = jiffies;

	DBG_871X("===> rtw_ips_pwr_down...................\n");

	rtlpriv->bCardDisableWOHSM = _TRUE;
	rtlpriv->net_closed = _TRUE;

	rtw_ips_dev_unload(rtlpriv);
	rtlpriv->bCardDisableWOHSM = _FALSE;
	DBG_871X("<=== rtw_ips_pwr_down..................... in %dms\n", rtw_get_passing_time_ms(start_time));
}
#endif
void rtw_ips_dev_unload(struct rtl_priv *rtlpriv)
{
	struct net_device *ndev = (struct net_device *) rtlpriv->ndev;
	struct xmit_priv	*pxmitpriv = &(rtlpriv->xmitpriv);

	DBG_871X("====> %s...\n", __FUNCTION__);

	rtw_hal_set_hwreg(rtlpriv, HW_VAR_FIFO_CLEARN_UP, 0);

	if (rtlpriv->intf_stop) {
		rtlpriv->intf_stop(rtlpriv);
	}

	/* s5. */
	if (rtlpriv->bSurpriseRemoved == _FALSE) {
		rtw_hal_deinit(rtlpriv);
	}

}

int pm_netdev_open(struct net_device *ndev, uint8_t bnormal)
{
	int status;

	if (_TRUE == bnormal)
		status = netdev_open(ndev);
#ifdef CONFIG_IPS
	else
		status =  (_SUCCESS == ips_netdrv_open(rtl_priv(ndev)))?(0):(-1);
#endif

	return status;
}

int netdev_close(struct net_device *ndev)
{
	struct rtl_priv *rtlpriv = rtl_priv(ndev);
	struct rtl_hal *rtlhal = rtl_hal(rtlpriv);	

	if (rtlpriv->pwrctrlpriv.bInternalAutoSuspend == _TRUE) {
		/*rtw_pwr_wakeup(rtlpriv); */
		if (rtlpriv->pwrctrlpriv.rf_pwrstate == rf_off)
			rtlpriv->pwrctrlpriv.ps_flag = _TRUE;
	}
	rtlpriv->net_closed = _TRUE;

/*	if (!rtlpriv->hw_init_completed)
	{
		DBG_871X("(1)871x_drv - drv_close, bup=%d, hw_init_completed=%d\n", rtlpriv->bup, rtlpriv->hw_init_completed);

		rtlpriv->bDriverStopped = _TRUE;

		rtw_dev_unload(rtlpriv);
	}
	else*/
	if (rtlpriv->pwrctrlpriv.rf_pwrstate == rf_on) {
		DBG_871X("(2)871x_drv - drv_close, bup=%d, hw_init_completed=%d\n", rtlpriv->bup, rtlpriv->hw_init_completed);

		/* s1. */
		if (ndev) {
			if (!rtw_netif_queue_stopped(ndev))
				rtw_netif_stop_queue(ndev);
		}

		/* s2. */
		LeaveAllPowerSaveMode(rtlpriv);
		rtw_disassoc_cmd(rtlpriv, 500, _FALSE);
		/* s2-2.  indicate disconnect to os */
		rtw_indicate_disconnect(rtlpriv);
		/* s2-3. */
		rtw_free_assoc_resources(rtlpriv, 1);
		/* s2-4. */
		rtw_free_network_queue(rtlpriv, _TRUE);
		/* Close LED */
		rtw_hal_led_control(rtlpriv, LED_CTL_POWER_OFF);
	}

	kfree(rtlhal->pfirmware);
	rtlhal->pfirmware = NULL;

	DBG_871X("-871x_drv - drv_close, bup=%d\n", rtlpriv->bup);

	return 0;

}

void rtw_ndev_destructor(struct net_device *ndev)
{
	DBG_871X(FUNC_NDEV_FMT"\n", FUNC_NDEV_ARG(ndev));

	free_netdev(ndev);
}

