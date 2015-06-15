#ifndef __WIFI_H__
#define __WIFI_H__

#include <linux/types.h>
#include "usb.h"

enum intf_type {
	INTF_PCI = 0,
	INTF_USB = 1,
};

enum radio_path {
	RF90_PATH_A = 0,
	RF90_PATH_B = 1,
	RF90_PATH_C = 2,
	RF90_PATH_D = 3,
};
enum rtl_desc92_rate {
	DESC_RATE1M = 0x00,
	DESC_RATE2M = 0x01,
	DESC_RATE5_5M = 0x02,
	DESC_RATE11M = 0x03,

	DESC_RATE6M = 0x04,
	DESC_RATE9M = 0x05,
	DESC_RATE12M = 0x06,
	DESC_RATE18M = 0x07,
	DESC_RATE24M = 0x08,
	DESC_RATE36M = 0x09,
	DESC_RATE48M = 0x0a,
	DESC_RATE54M = 0x0b,

	DESC_RATEMCS0 = 0x0c,
	DESC_RATEMCS1 = 0x0d,
	DESC_RATEMCS2 = 0x0e,
	DESC_RATEMCS3 = 0x0f,
	DESC_RATEMCS4 = 0x10,
	DESC_RATEMCS5 = 0x11,
	DESC_RATEMCS6 = 0x12,
	DESC_RATEMCS7 = 0x13,
	DESC_RATEMCS8 = 0x14,
	DESC_RATEMCS9 = 0x15,
	DESC_RATEMCS10 = 0x16,
	DESC_RATEMCS11 = 0x17,
	DESC_RATEMCS12 = 0x18,
	DESC_RATEMCS13 = 0x19,
	DESC_RATEMCS14 = 0x1a,
	DESC_RATEMCS15 = 0x1b,
	DESC_RATEMCS15_SG = 0x1c,
	DESC_RATEMCS32 = 0x20,

	DESC_RATEVHT1SS_MCS0 = 0x2c,
	DESC_RATEVHT1SS_MCS1 = 0x2d,
	DESC_RATEVHT1SS_MCS2 = 0x2e,
	DESC_RATEVHT1SS_MCS3 = 0x2f,
	DESC_RATEVHT1SS_MCS4 = 0x30,
	DESC_RATEVHT1SS_MCS5 = 0x31,
	DESC_RATEVHT1SS_MCS6 = 0x32,
	DESC_RATEVHT1SS_MCS7 = 0x33,
	DESC_RATEVHT1SS_MCS8 = 0x34,
	DESC_RATEVHT1SS_MCS9 = 0x35,
	DESC_RATEVHT2SS_MCS0 = 0x36,
	DESC_RATEVHT2SS_MCS1 = 0x37,
	DESC_RATEVHT2SS_MCS2 = 0x38,
	DESC_RATEVHT2SS_MCS3 = 0x39,
	DESC_RATEVHT2SS_MCS4 = 0x3a,
	DESC_RATEVHT2SS_MCS5 = 0x3b,
	DESC_RATEVHT2SS_MCS6 = 0x3c,
	DESC_RATEVHT2SS_MCS7 = 0x3d,
	DESC_RATEVHT2SS_MCS8 = 0x3e,
	DESC_RATEVHT2SS_MCS9 = 0x3f,
};

struct rtl_hal_ops;

#define CHANNEL_MAX_NUMBER	(14 + 24 + 21)	/* 14 is the max channel no */
#define CHANNEL_MAX_NUMBER_2G		14
#define CHANNEL_MAX_NUMBER_5G		54 /* Please refer to
					    *"phy_GetChnlGroup8812A" and
					    * "Hal_ReadTxPowerInfo8812A"
					    */
#define CHANNEL_MAX_NUMBER_5G_80M	7
#define CHANNEL_GROUP_MAX	(3 + 9)	/*  ch1~3, 4~9, 10~14 = three groups */
#define CHANNEL_MAX_NUMBER_5G		54 /* Please refer to
					    *"phy_GetChnlGroup8812A" and
					    * "Hal_ReadTxPowerInfo8812A"
					    */
#define CHANNEL_MAX_NUMBER_5G_80M	7
#define MAX_PG_GROUP			13
#define	CHANNEL_GROUP_MAX_2G		3
#define	CHANNEL_GROUP_IDX_5GL		3
#define	CHANNEL_GROUP_IDX_5GM		6
#define	CHANNEL_GROUP_IDX_5GH		9
#define	CHANNEL_GROUP_MAX_5G		9
#define CHANNEL_MAX_NUMBER_2G		14
#define AVG_THERMAL_NUM			8
#define AVG_THERMAL_NUM_88E		4
#define AVG_THERMAL_NUM_8723BE		4
#define MAX_TID_COUNT			9






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
#define	MAX_RF_PATH				4
#define 	RF_PATH_MAX				MAX_RF_PATH
#define	MAX_CHNL_GROUP_24G		6
#define	MAX_CHNL_GROUP_5G		14

//It must always set to 4, otherwise read efuse table secquence will be wrong.
#define 	MAX_TX_COUNT				4

struct txpower_info_2g {
	u8 index_cck_base[MAX_RF_PATH][MAX_CHNL_GROUP_24G];
	u8 index_bw40_base[MAX_RF_PATH][MAX_CHNL_GROUP_24G];
	/*If only one tx, only BW20 and OFDM are used.*/
	u8 cck_diff[MAX_RF_PATH][MAX_TX_COUNT];
	u8 ofdm_diff[MAX_RF_PATH][MAX_TX_COUNT];
	u8 bw20_diff[MAX_RF_PATH][MAX_TX_COUNT];
	u8 bw40_diff[MAX_RF_PATH][MAX_TX_COUNT];
	u8 bw80_diff[MAX_RF_PATH][MAX_TX_COUNT];
	u8 bw160_diff[MAX_RF_PATH][MAX_TX_COUNT];
};

struct txpower_info_5g {
	u8 index_bw40_base[MAX_RF_PATH][MAX_CHNL_GROUP_5G];
	/*If only one tx, only BW20, OFDM, BW80 and BW160 are used.*/
	u8 ofdm_diff[MAX_RF_PATH][MAX_TX_COUNT];
	u8 bw20_diff[MAX_RF_PATH][MAX_TX_COUNT];
	u8 bw40_diff[MAX_RF_PATH][MAX_TX_COUNT];
	u8 bw80_diff[MAX_RF_PATH][MAX_TX_COUNT];
	u8 bw160_diff[MAX_RF_PATH][MAX_TX_COUNT];
};




#define	EFUSE_MAX_LOGICAL_SIZE			512

struct rtl_efuse {	bool autoLoad_ok;
	bool bootfromefuse;
	u16 max_physical_size;

	u8 efuse_map[2][EFUSE_MAX_LOGICAL_SIZE];
	u16 efuse_usedbytes;
	u8 efuse_usedpercentage;
#ifdef EFUSE_REPG_WORKAROUND
	bool efuse_re_pg_sec1flag;
	u8 efuse_re_pg_data[8];
#endif

	u8 autoload_failflag;
	u8 autoload_status;

	short epromtype;
	u16 eeprom_vid;
	u16 eeprom_did;
	u16 eeprom_svid;
	u16 eeprom_smid;
	u8 eeprom_oemid;
	u16 eeprom_channelplan;
	u8 eeprom_version;
	u8 board_type;
	u8 external_pa;

	u8 dev_addr[6];
	u8 wowlan_enable;
	u8 antenna_div_cfg;
	u8 antenna_div_type;

	bool txpwr_fromeprom;
	u8 eeprom_crystalcap;
	u8 eeprom_tssi[2];
	u8 eeprom_tssi_5g[3][2]; /* for 5GL/5GM/5GH band. */
	u8 eeprom_pwrlimit_ht20[CHANNEL_GROUP_MAX];
	u8 eeprom_pwrlimit_ht40[CHANNEL_GROUP_MAX];
	u8 eeprom_chnlarea_txpwr_cck[MAX_RF_PATH][CHANNEL_GROUP_MAX_2G];
	u8 eeprom_chnlarea_txpwr_ht40_1s[MAX_RF_PATH][CHANNEL_GROUP_MAX];
	u8 eprom_chnl_txpwr_ht40_2sdf[MAX_RF_PATH][CHANNEL_GROUP_MAX];

	u8 internal_pa_5g[2];	/* pathA / pathB */
	u8 eeprom_c9;
	u8 eeprom_cc;

	/*For power group */
	u8 eeprom_pwrgroup[2][3];
	u8 pwrgroup_ht20[2][CHANNEL_MAX_NUMBER];
	u8 pwrgroup_ht40[2][CHANNEL_MAX_NUMBER];

	u8 txpwrlevel_cck[MAX_RF_PATH][CHANNEL_MAX_NUMBER_2G];
	/*For HT 40MHZ pwr */
	u8 txpwrlevel_ht40_1s[MAX_RF_PATH][CHANNEL_MAX_NUMBER];
	/*For HT 40MHZ pwr */
	u8 txpwrlevel_ht40_2s[MAX_RF_PATH][CHANNEL_MAX_NUMBER];

	/*--------------------------------------------------------*
	 * 8192CE\8192SE\8192DE\8723AE use the following 4 arrays,
	 * other ICs (8188EE\8723BE\8192EE\8812AE...)
	 * define new arrays in Windows code.
	 * BUT, in linux code, we use the same array for all ICs.
	 *
	 * The Correspondance relation between two arrays is:
	 * txpwr_cckdiff[][] == CCK_24G_Diff[][]
	 * txpwr_ht20diff[][] == BW20_24G_Diff[][]
	 * txpwr_ht40diff[][] == BW40_24G_Diff[][]
	 * txpwr_legacyhtdiff[][] == OFDM_24G_Diff[][]
	 *
	 * Sizes of these arrays are decided by the larger ones.
	 */
	char txpwr_cckdiff[MAX_RF_PATH][CHANNEL_MAX_NUMBER];
	char txpwr_ht20diff[MAX_RF_PATH][CHANNEL_MAX_NUMBER];
	char txpwr_ht40diff[MAX_RF_PATH][CHANNEL_MAX_NUMBER];
	char txpwr_legacyhtdiff[MAX_RF_PATH][CHANNEL_MAX_NUMBER];

	u8 txpwr_5g_bw40base[MAX_RF_PATH][CHANNEL_MAX_NUMBER];
	u8 txpwr_5g_bw80base[MAX_RF_PATH][CHANNEL_MAX_NUMBER_5G_80M];
	char txpwr_5g_ofdmdiff[MAX_RF_PATH][MAX_TX_COUNT];
	char txpwr_5g_bw20diff[MAX_RF_PATH][MAX_TX_COUNT];
	char txpwr_5g_bw40diff[MAX_RF_PATH][MAX_TX_COUNT];
	char txpwr_5g_bw80diff[MAX_RF_PATH][MAX_TX_COUNT];

	u8 txpwr_safetyflag;			/* Band edge enable flag */
	u16 eeprom_txpowerdiff;
	u8 legacy_httxpowerdiff;	/* Legacy to HT rate power diff */
	u8 antenna_txpwdiff[3];

	u8 eeprom_regulatory;
	u8 eeprom_thermalmeter;
	u8 thermalmeter[2]; /*ThermalMeter, index 0 for RFIC0, 1 for RFIC1 */
	u16 tssi_13dbm;
	u8 crystalcap;		/* CrystalCap. */
	u8 delta_iqk;
	u8 delta_lck;

	u8 legacy_ht_txpowerdiff;	/*Legacy to HT rate power diff */
	bool apk_thermalmeterignore;

	bool b1x1_recvcombine;
	bool b1ss_support;

	/*channel plan */
	u8 channel_plan;

};

typedef enum tag_Board_Definition
{
    ODM_BOARD_DEFAULT  	= 0, 	  // The DEFAULT case.
    ODM_BOARD_MINICARD  = BIT(0), // 0 = non-mini card, 1= mini card.
    ODM_BOARD_SLIM      = BIT(1), // 0 = non-slim card, 1 = slim card
    ODM_BOARD_BT        = BIT(2), // 0 = without BT card, 1 = with BT
    ODM_BOARD_EXT_PA    = BIT(3), // 0 = no 2G ext-PA, 1 = existing 2G ext-PA
    ODM_BOARD_EXT_LNA   = BIT(4), // 0 = no 2G ext-LNA, 1 = existing 2G ext-LNA
    ODM_BOARD_EXT_TRSW  = BIT(5), // 0 = no ext-TRSW, 1 = existing ext-TRSW
    ODM_BOARD_EXT_PA_5G    = BIT(6), // 0 = no 5G ext-PA, 1 = existing 5G ext-PA
    ODM_BOARD_EXT_LNA_5G   = BIT(7), // 0 = no 5G ext-LNA, 1 = existing 5G ext-LNA
}ODM_BOARD_TYPE_E;

enum band_type {
	BAND_ON_2_4G = 0,
	BAND_ON_5G,
	BAND_ON_BOTH,
	BANDMAX
};

/*mlme related.*/
enum wireless_mode {
	WIRELESS_MODE_UNKNOWN = 0x00,
	WIRELESS_MODE_A = 0x01,
	WIRELESS_MODE_B = 0x02,
	WIRELESS_MODE_G = 0x04,
	WIRELESS_MODE_AUTO = 0x08,
	WIRELESS_MODE_N_24G = 0x10,
	WIRELESS_MODE_N_5G = 0x20,
	WIRELESS_MODE_AC_5G = 0x40,
	WIRELESS_MODE_AC_24G  = 0x80,
	WIRELESS_MODE_AC_ONLY = 0x100,
#if 0	/* ULLI : we can't enable this,but this is not used in rtlwifi */
	WIRELESS_MODE_MAX = 0x800
#endif
};

struct rtl_hal {
#if 0
	struct ieee80211_hw *hw;
#endif
	bool driver_is_goingto_unload;
	bool up_first_time;
	bool first_init;
	bool being_init_adapter;
	bool bbrf_ready;
	bool mac_func_enable;
	bool pre_edcca_enable;
#if 0
	struct bt_coexist_8723 hal_coex_8723;
#endif

	enum intf_type interface;
	u16 hw_type;		/*92c or 92d or 92s and so on */
	u8 ic_class;
	u8 oem_id;
	u32 version;		/*version of chip */
	u8 state;		/*stop 0, start 1 */
	u8 board_type;
	u8 external_pa;

	u8 pa_mode;
	u8 pa_type_2g;
	u8 pa_type_5g;
	u8 lna_type_2g;
	u8 lna_type_5g;
	u8 external_pa_2g;
	u8 external_lna_2g;
	u8 external_pa_5g;
	u8 external_lna_5g;
	u8 rfe_type;

	/*firmware */
	u32 fwsize;
	u8 *pfirmware;
	u16 fw_version;
	u16 fw_subversion;
	bool h2c_setinprogress;
	u8 last_hmeboxnum;
	bool fw_ready;
	/*Reserve page start offset except beacon in TxQ. */
	u8 fw_rsvdpage_startoffset;
	u8 h2c_txcmd_seq;
	u8 current_ra_rate;

	/* FW Cmd IO related */
	u16 fwcmd_iomap;
	u32 fwcmd_ioparam;
	bool set_fwcmd_inprogress;
	u8 current_fwcmd_io;

#if 0
	struct p2p_ps_offload_t p2p_ps_offload;
#endif
	bool fw_clk_change_in_progress;
	bool allow_sw_to_change_hwclc;
	u8 fw_ps_state;
	/**/
	bool driver_going2unload;

	/*AMPDU init min space*/
	u8 minspace_cfg;	/*For Min spacing configurations */

	/* Dual mac */
#if 0
	enum macphy_mode macphymode;
#endif
	enum band_type current_bandtype;	/* 0:2.4G, 1:5G */
#if 0
	enum band_type current_bandtypebackup;
#endif
	enum band_type bandset;

	/* dual MAC 0--Mac0 1--Mac1 */
	u32 interfaceindex;
	/* just for DualMac S3S4 */
	u8 macphyctl_reg;
	bool earlymode_enable;
	u8 max_earlymode_num;
	/* Dual mac*/
	bool during_mac0init_radiob;
	bool during_mac1init_radioa;
	bool reloadtxpowerindex;
	/* True if IMR or IQK  have done
	for 2.4G in scan progress */
	bool load_imrandiqk_setting_for2g;

	bool disable_amsdu_8k;
	bool master_of_dmsp;
	bool slave_of_dmsp;

	u16 rx_tag;/*for 92ee*/
	u8 rts_en;

	/*for wowlan*/
	bool wow_enable;
	bool enter_pnp_sleep;
	bool wake_from_pnp_sleep;
	bool wow_enabled;
	__kernel_time_t last_suspend_sec;
	u32 wowlan_fwsize;
	u8 *wowlan_firmware;

	u8 hw_rof_enable; /*Enable GPIO[9] as WL RF HW PDn source*/

	bool real_wow_v2_enable;
	bool re_init_llt_table;
};


#define MAX_TX_COUNT			4
#define MAX_RATE_SECTION_NUM		6
#define MAX_5G_BANDWITH_NUM		4
#define	MAX_RF_PATH			4
#define	MAX_CHNL_GROUP_24G		6
#define	MAX_CHNL_GROUP_5G		14

#define TX_PWR_BY_RATE_NUM_BAND		2
#define TX_PWR_BY_RATE_NUM_RF		4
#define TX_PWR_BY_RATE_NUM_SECTION	12
#define MAX_BASE_NUM_IN_PHY_REG_PG_24G  6
#define MAX_BASE_NUM_IN_PHY_REG_PG_5G	5


//###### duplicate code,will move to ODM #########
#define IQK_MAC_REG_NUM		4
#define IQK_ADDA_REG_NUM		16

#define IQK_BB_REG_NUM			10
#define IQK_BB_REG_NUM_92C	9
#define IQK_BB_REG_NUM_92D	10
#define IQK_BB_REG_NUM_test	6


#define IQK_MATRIX_REG_NUM	8
#define IQK_MATRIX_SETTINGS_NUM	(1 + 24 + 21)

enum rt_polarity_ctl {
	RT_POLARITY_LOW_ACT = 0,
	RT_POLARITY_HIGH_ACT = 1,
};

struct iqk_matrix_regs {
	bool iqk_done;
	long value[1][IQK_MATRIX_REG_NUM];
};


struct phy_parameters {
	u16 length;
	u32 *pdata;
};

enum hw_param_tab_index {
	PHY_REG_2T,
	PHY_REG_1T,
	PHY_REG_PG,
	RADIOA_2T,
	RADIOB_2T,
	RADIOA_1T,
	RADIOB_1T,
	MAC_REG,
	AGCTAB_2T,
	AGCTAB_1T,
	MAX_TAB
};

enum io_type {
	IO_CMD_PAUSE_DM_BY_SCAN = 0,
	IO_CMD_PAUSE_BAND0_DM_BY_SCAN = 0,
	IO_CMD_PAUSE_BAND1_DM_BY_SCAN = 1,
	IO_CMD_RESUME_DM_BY_SCAN = 2,
};

struct bb_reg_def {
	u32 rfintfs;
	u32 rfintfi;
	u32 rfintfo;
	u32 rfintfe;
	u32 rf3wire_offset;
	u32 rflssi_select;
	u32 rftxgain_stage;
	u32 rfhssi_para1;
	u32 rfhssi_para2;
	u32 rfsw_ctrl;
	u32 rfagc_control1;
	u32 rfagc_control2;
	u32 rfrxiq_imbal;
	u32 rfrx_afe;
	u32 rftxiq_imbal;
	u32 rftx_afe;
	u32 rf_rb;		/* rflssi_readback */
	u32 rf_rbpi;		/* rflssi_readbackpi */
};

struct init_gain {
	u8 xaagccore1;
	u8 xbagccore1;
	u8 xcagccore1;
	u8 xdagccore1;
	u8 cca;

};

struct rtl_phy {
	struct bb_reg_def phyreg_def[4];	/*Radio A/B/C/D */
	struct init_gain initgain_backup;
	enum io_type current_io_type;

	u8 rf_mode;
	u8 rf_type;
	u8 current_chan_bw;
	u8 set_bwmode_inprogress;
	u8 sw_chnl_inprogress;
	u8 sw_chnl_stage;
	u8 sw_chnl_step;
	u8 current_channel;
	u8 h2c_box_num;
	u8 set_io_inprogress;
	u8 lck_inprogress;

	/* record for power tracking */
	s32 reg_e94;
	s32 reg_e9c;
	s32 reg_ea4;
	s32 reg_eac;
	s32 reg_eb4;
	s32 reg_ebc;
	s32 reg_ec4;
	s32 reg_ecc;
	u8 rfpienable;
	u8 reserve_0;
	u16 reserve_1;
	u32 reg_c04, reg_c08, reg_874;
	u32 adda_backup[16];
	u32 iqk_mac_backup[IQK_MAC_REG_NUM];
	u32 iqk_bb_backup[10];
	bool iqk_initialized;

	bool rfpath_rx_enable[MAX_RF_PATH];
	u8 reg_837;
	/* Dual mac */
	bool need_iqk;
	struct iqk_matrix_regs iqk_matrix[IQK_MATRIX_SETTINGS_NUM];

	bool rfpi_enable;
	bool iqk_in_progress;

	u8 pwrgroup_cnt;
	u8 cck_high_power;
	/* this is for 88E & 8723A */
	u32 mcs_txpwrlevel_origoffset[MAX_PG_GROUP][16];
	/* MAX_PG_GROUP groups of pwr diff by rates */
	u32 mcs_offset[MAX_PG_GROUP][16];
	
	u32 tx_power_by_rate_offset[TX_PWR_BY_RATE_NUM_BAND]
				   [TX_PWR_BY_RATE_NUM_RF]
				   [TX_PWR_BY_RATE_NUM_SECTION];
	
#if 0 	/* ULLI disabled to get (in any case) compiler error */
	u32 tx_power_by_rate_offset[TX_PWR_BY_RATE_NUM_BAND]
				   [TX_PWR_BY_RATE_NUM_RF]
				   [TX_PWR_BY_RATE_NUM_RF]
				   [TX_PWR_BY_RATE_NUM_SECTION];
	u8 txpwr_by_rate_base_24g[TX_PWR_BY_RATE_NUM_RF]
				 [TX_PWR_BY_RATE_NUM_RF]
				 [MAX_BASE_NUM_IN_PHY_REG_PG_24G];
	u8 txpwr_by_rate_base_5g[TX_PWR_BY_RATE_NUM_RF]
				[TX_PWR_BY_RATE_NUM_RF]
				[MAX_BASE_NUM_IN_PHY_REG_PG_5G];
#endif
	u8 default_initialgain[4];

	/* the current Tx power level */
	u8 cur_cck_txpwridx;
	u8 cur_ofdm24g_txpwridx;
	u8 cur_bw20_txpwridx;
	u8 cur_bw40_txpwridx;

	char txpwr_limit_2_4g[MAX_REGULATION_NUM]
			     [MAX_2_4G_BANDWITH_NUM]
			     [MAX_RATE_SECTION_NUM]
			     [CHANNEL_MAX_NUMBER_2G]
			     [MAX_RF_PATH_NUM];
	char txpwr_limit_5g[MAX_REGULATION_NUM]
			   [MAX_5G_BANDWITH_NUM]
			   [MAX_RATE_SECTION_NUM]
			   [CHANNEL_MAX_NUMBER_5G]
			   [MAX_RF_PATH_NUM];

	u32 rfreg_chnlval[2];
	bool apk_done;
	u32 reg_rf3c[2];	/* pathA / pathB  */

	u32 backup_rf_0x1a;/*92ee*/
	/* bfsync */
	u8 framesync;
	u32 framesync_c34;

	u8 num_total_rfpath;
	struct phy_parameters hwparam_tables[MAX_TAB];
	u16 rf_pathmap;

	u8 hw_rof_enable; /*Enable GPIO[9] as WL RF HW PDn source*/
	enum rt_polarity_ctl polarity_ctl;


	/* ULLI : Border from old (struct rtw_hal) */

#define MAX_BASE_NUM_IN_PHY_REG_PG_2_4G			4 //  CCK:1,OFDM:2, HT:2
#define MAX_BASE_NUM_IN_PHY_REG_PG_5G			5 // OFDM:1, HT:2, VHT:2

	/* ULLI: neded to convert this */

	//---------------------------------------------------------------------------------//

	// Power Limit Table for 2.4G

	// Store the original power by rate value of the base of each rate section of rf path A & B
	uint8_t	 txpwr_by_rate_base_24g[MAX_RF_PATH_NUM_IN_POWER_LIMIT_TABLE]
						[MAX_BASE_NUM_IN_PHY_REG_PG_2_4G];
	uint8_t	txpwr_by_rate_base_5g[MAX_RF_PATH_NUM_IN_POWER_LIMIT_TABLE]
						[MAX_BASE_NUM_IN_PHY_REG_PG_5G];


};


#define AVG_THERMAL_NUM		8
#define IQK_Matrix_REG_NUM		8
#define IQK_Matrix_Settings_NUM	14+24+21 // Channels_2_4G_NUM + Channels_5G_20M_NUM + Channels_5G

#define ASSOCIATE_ENTRY_NUM					32 // Max size of AsocEntry[].
#define	ODM_ASSOCIATE_ENTRY_NUM				ASSOCIATE_ENTRY_NUM

struct fast_ant_training {
	u8	bssid[6];
	u8	antsel_rx_keep_0;
	u8	antsel_rx_keep_1;
	u8	antsel_rx_keep_2;
	u32	ant_sum[7];
	u32	ant_cnt[7];
	u32	ant_ave[7];
	u8	fat_state;
	u32	train_idx;
	u8	antsel_a[ASSOCIATE_ENTRY_NUM];
	u8	antsel_b[ASSOCIATE_ENTRY_NUM];
	u8	antsel_c[ASSOCIATE_ENTRY_NUM];
	u32	main_ant_sum[ASSOCIATE_ENTRY_NUM];
	u32	aux_ant_sum[ASSOCIATE_ENTRY_NUM];
	u32	main_ant_cnt[ASSOCIATE_ENTRY_NUM];
	u32	aux_ant_cnt[ASSOCIATE_ENTRY_NUM];
	u8	rx_idle_ant;
	bool	becomelinked;
};


struct dm_phy_dbg_info {
	char rx_snrdb[4];
	u64 num_qry_phy_status;
	u64 num_qry_phy_status_cck;
	u64 num_qry_phy_status_ofdm;
	u16 num_qry_beacon_pkt;
	u16 num_non_be_pkt;
	s32 rx_evm[4];
};

#define DEL_SW_IDX_SZ		30
#define BAND_NUM			3

struct rtl_dm {
	/*PHY status for Dynamic Management */
	long entry_min_undec_sm_pwdb;
	long undec_sm_cck;
	long undec_sm_pwdb;	/*out dm */
	long entry_max_undec_sm_pwdb;
	s32 ofdm_pkt_cnt;
	bool dm_initialgain_enable;
	bool dynamic_txpower_enable;
	bool current_turbo_edca;
	bool is_any_nonbepkts;	/*out dm */
	bool is_cur_rdlstate;
	bool txpower_trackinginit;
	bool disable_framebursting;
	bool cck_inch14;
	bool txpower_tracking;
	bool useramask;
	bool rfpath_rxenable[4];
	bool inform_fw_driverctrldm;
	bool current_mrc_switch;
	u8 txpowercount;
	u8 powerindex_backup[6];

	u8 thermalvalue_rxgain;
	u8 thermalvalue_iqk;
	u8 thermalvalue_lck;
	u8 thermalvalue;
	u8 last_dtp_lvl;
	u8 thermalvalue_avg[AVG_THERMAL_NUM];
	u8 thermalvalue_avg_index;
	bool done_txpower;
	u8 dynamic_txhighpower_lvl;	/*Tx high power level */
	u8 dm_flag;		/*Indicate each dynamic mechanism's status. */
	u8 dm_flag_tmp;
	u8 dm_type;
	u8 dm_rssi_sel;
	u8 txpower_track_control;
	bool interrupt_migration;
	bool disable_tx_int;
	char ofdm_index[MAX_RF_PATH];
	u8 default_ofdm_index;
	u8 default_cck_index;
	char cck_index;
	char delta_power_index[MAX_RF_PATH];
	char delta_power_index_last[MAX_RF_PATH];
	char power_index_offset[MAX_RF_PATH];
	char absolute_ofdm_swing_idx[MAX_RF_PATH];
	char remnant_ofdm_swing_idx[MAX_RF_PATH];
	char remnant_cck_idx;
	bool modify_txagc_flag_path_a;
	bool modify_txagc_flag_path_b;

	bool one_entry_only;

	struct dm_phy_dbg_info dbginfo;

	/* Dynamic ATC switch */
	bool atc_status;
	bool large_cfo_hit;
	bool is_freeze;
	int cfo_tail[2];
	int cfo_ave_pre;
	int crystal_cap;
	u8 cfo_threshold;
	u32 packet_count;
	u32 packet_count_pre;
	u8 tx_rate;

	/*88e tx power tracking*/
	u8	swing_idx_ofdm[MAX_RF_PATH];
	u8	swing_idx_ofdm_cur;
	u8	swing_idx_ofdm_base[MAX_RF_PATH];
	bool	swing_flag_ofdm;
	u8	swing_idx_cck;
	u8	swing_idx_cck_cur;
	u8	swing_idx_cck_base;
	bool	swing_flag_cck;

	char	swing_diff_2g;
	char	swing_diff_5g;

	u8 delta_swing_table_idx_24gccka_p[DEL_SW_IDX_SZ];
	u8 delta_swing_table_idx_24gccka_n[DEL_SW_IDX_SZ];
	u8 delta_swing_table_idx_24gcckb_p[DEL_SW_IDX_SZ];
	u8 delta_swing_table_idx_24gcckb_n[DEL_SW_IDX_SZ];
	u8 delta_swing_table_idx_24ga_p[DEL_SW_IDX_SZ];
	u8 delta_swing_table_idx_24ga_n[DEL_SW_IDX_SZ];
	u8 delta_swing_table_idx_24gb_p[DEL_SW_IDX_SZ];
	u8 delta_swing_table_idx_24gb_n[DEL_SW_IDX_SZ];
	u8 delta_swing_table_idx_5ga_p[BAND_NUM][DEL_SW_IDX_SZ];
	u8 delta_swing_table_idx_5ga_n[BAND_NUM][DEL_SW_IDX_SZ];
	u8 delta_swing_table_idx_5gb_p[BAND_NUM][DEL_SW_IDX_SZ];
	u8 delta_swing_table_idx_5gb_n[BAND_NUM][DEL_SW_IDX_SZ];
	u8 delta_swing_table_idx_24ga_p_8188e[DEL_SW_IDX_SZ];
	u8 delta_swing_table_idx_24ga_n_8188e[DEL_SW_IDX_SZ];

	/* DMSP */
	bool supp_phymode_switch;

	/* DulMac */
	struct fast_ant_training fat_table;

	u8	resp_tx_path;
	u8	path_sel;
	u32	patha_sum;
	u32	pathb_sum;
	u32	patha_cnt;
	u32	pathb_cnt;

	u8 pre_channel;
	u8 *p_channel;
	u8 linked_interval;

	u64 last_tx_ok_cnt;
	u64 last_rx_ok_cnt;
};

#define rtl_hal(rtlpriv)	(&((rtlpriv)->rtlhal))
#define rtl_mac(rtlpriv)	(&((rtlpriv)->mac80211))
#define rtl_efuse(rtlpriv)	(&((rtlpriv)->efuse))
#define rtl_phy(rtlpriv)	(&((rtlpriv)->phy))
#define rtl_dm(rtlpriv)		(&((rtlpriv)->dm))

struct rtl_hal_cfg {
	u8 bar_id;
	bool write_readback;
	char *name;
	char *fw_name;
	char *alt_fw_name;
	struct rtl_hal_ops *ops;
#if 0 		/* ULLI currently not defined */
	struct rtl_mod_params *mod_params;
	struct rtl_hal_usbint_cfg *usb_interface_cfg;

	/*this map used for some registers or vars
	   defined int HAL but used in MAIN */
	u32 maps[RTL_VAR_MAP_MAX];
#endif
};

struct rtl_priv;
struct rtl_io {
	struct device *dev;
	struct mutex bb_mutex;

	/*PCI MEM map */
	unsigned long pci_mem_end;	/*shared mem end        */
	unsigned long pci_mem_start;	/*shared mem start */

	/*PCI IO map */
	unsigned long pci_base_addr;	/*device I/O address */

	void (*write8_async) (struct rtl_priv *rtlpriv, u32 addr, u8 val);
	void (*write16_async) (struct rtl_priv *rtlpriv, u32 addr, u16 val);
	void (*write32_async) (struct rtl_priv *rtlpriv, u32 addr, u32 val);
	void (*writeN_sync) (struct rtl_priv *rtlpriv, u32 addr, void *buf,
			     u16 len);

	u8(*read8_sync) (struct rtl_priv *rtlpriv, u32 addr);
	u16(*read16_sync) (struct rtl_priv *rtlpriv, u32 addr);
	u32(*read32_sync) (struct rtl_priv *rtlpriv, u32 addr);
};

struct dig_t {
	u32 rssi_lowthresh;
	u32 rssi_highthresh;
	u32 fa_lowthresh;
	u32 fa_highthresh;
	long last_min_undec_pwdb_for_dm;
	long rssi_highpower_lowthresh;
	long rssi_highpower_highthresh;
	u32 recover_cnt;
	u32 pre_igvalue;
	u32 cur_igvalue;
	long rssi_val;
	u8 dig_enable_flag;
	u8 dig_ext_port_stage;
	u8 dig_algorithm;
	u8 dig_twoport_algorithm;
	u8 dig_dbgmode;
	u8 dig_slgorithm_switch;
	u8 cursta_cstate;
	u8 presta_cstate;
	u8 curmultista_cstate;
	u8 stop_dig;
	char back_val;
	char back_range_max;
	char back_range_min;
	u8 rx_gain_max;
	u8 rx_gain_min;
	u8 min_undec_pwdb_for_dm;
	u8 rssi_val_min;
	u8 pre_cck_cca_thres;
	u8 cur_cck_cca_thres;
	u8 pre_cck_pd_state;
	u8 cur_cck_pd_state;
	u8 pre_cck_fa_state;
	u8 cur_cck_fa_state;
	u8 pre_ccastate;
	u8 cur_ccasate;
	u8 large_fa_hit;
	u8 forbidden_igi;
	u8 dig_state;
	u8 dig_highpwrstate;
	u8 cur_sta_cstate;
	u8 pre_sta_cstate;
	u8 cur_ap_cstate;
	u8 pre_ap_cstate;
	u8 cur_pd_thstate;
	u8 pre_pd_thstate;
	u8 cur_cs_ratiostate;
	u8 pre_cs_ratiostate;
	u8 backoff_enable_flag;
	char backoffval_range_max;
	char backoffval_range_min;
	u8 dig_min_0;
	u8 dig_min_1;
	u8 bt30_cur_igi;
	bool media_connect_0;
	bool media_connect_1;

	u32 antdiv_rssi_max;
	u32 rssi_max;

/* ULLI : not in rtlwifi */

	u8 BackupIGValue;
};

struct rate_adaptive {
	u8 rate_adaptive_disabled;
	u8 ratr_state;
	u16 reserve;

	u32 high_rssi_thresh_for_ra;
	u32 high2low_rssi_thresh_for_ra;
	u8 low2high_rssi_thresh_for_ra40m;
	u32 low_rssi_thresh_for_ra40m;
	u8 low2high_rssi_thresh_for_ra20m;
	u32 low_rssi_thresh_for_ra20m;
	u32 upper_rssi_threshold_ratr;
	u32 middleupper_rssi_threshold_ratr;
	u32 middle_rssi_threshold_ratr;
	u32 middlelow_rssi_threshold_ratr;
	u32 low_rssi_threshold_ratr;
	u32 ultralow_rssi_threshold_ratr;
	u32 low_rssi_threshold_ratr_40m;
	u32 low_rssi_threshold_ratr_20m;
	u8 ping_rssi_enable;
	u32 ping_rssi_ratr;
	u32 ping_rssi_thresh_for_ra;
	u32 last_ratr;
	u8 pre_ratr_state;
	u8 ldpc_thres;
	bool use_ldpc;
	bool lower_rts_rate;
	bool is_special_data;
};

enum rtl_link_state {
	MAC80211_NOLINK = 0,
	MAC80211_LINKING = 1,
	MAC80211_LINKED = 2,
	MAC80211_LINKED_SCANNING = 3,
};

struct rtl_mac {
	u8 mac_addr[ETH_ALEN];
	u8 mac80211_registered;
	u8 beacon_enabled;

	u32 tx_ss_num;
	u32 rx_ss_num;

#if 0	/* ULLI : Currently we are using wireless ext */
	struct ieee80211_supported_band bands[IEEE80211_NUM_BANDS];
	struct ieee80211_hw *hw;
	struct ieee80211_vif *vif;
	enum nl80211_iftype opmode;
#endif

	/*Probe Beacon management */
#if 0	/* Ulli : currently we are using wireless-ext */
	struct rtl_tid_data tids[MAX_TID_COUNT];
#endif
	enum rtl_link_state link_state;
	int n_channels;
	int n_bitrates;

	bool offchan_delay;
	u8 p2p;	/*using p2p role*/
	bool p2p_in_use;

	/*filters */
	u32 rx_conf;
	u16 rx_mgt_filter;
	u16 rx_ctrl_filter;
	u16 rx_data_filter;

	bool act_scanning;
	u8 cnt_after_linked;
	bool skip_scan;

	/* early mode */
	/* skb wait queue */
	struct sk_buff_head skb_waitq[MAX_TID_COUNT];

	u8 ht_stbc_cap;
	u8 ht_cur_stbc;

	/*vht support*/
	u8 vht_enable;
	u8 bw_80;
	u8 vht_cur_ldpc;
	u8 vht_cur_stbc;
	u8 vht_stbc_cap;
	u8 vht_ldpc_cap;

	/*RDG*/
	bool rdg_en;

	/*AP*/
	u8 bssid[ETH_ALEN] __aligned(2);
	u32 vendor;
	u8 mcs[16];	/* 16 bytes mcs for HT rates. */
	u32 basic_rates; /* b/g rates */
	u8 ht_enable;
	u8 sgi_40;
	u8 sgi_20;
	u8 bw_40;
	u16 mode;		/* wireless mode */
	u8 slot_time;
	u8 short_preamble;
	u8 use_cts_protect;
	u8 cur_40_prime_sc;
	u8 cur_40_prime_sc_bk;
	u8 cur_80_prime_sc;
	u64 tsf;
	u8 retry_short;
	u8 retry_long;
	u16 assoc_id;
	bool hiddenssid;

	/*IBSS*/
	int beacon_interval;

	/*AMPDU*/
	u8 min_space_cfg;	/*For Min spacing configurations */
	u8 max_mss_density;
	u8 current_ampdu_factor;
	u8 current_ampdu_density;

	/*QOS & EDCA */
#if 0	/* Ulli : currently we are using wireless-ext */
	struct ieee80211_tx_queue_params edca_param[RTL_MAC80211_NUM_QUEUE];
	struct rtl_qos_parameters ac[AC_MAX];
#endif

	/* counters */
	u64 last_txok_cnt;
	u64 last_rxok_cnt;
	u32 last_bt_edca_ul;
	u32 last_bt_edca_dl;
};

struct false_alarm_statistics {
	u32 cnt_parity_fail;
	u32 cnt_rate_illegal;
	u32 cnt_crc8_fail;
	u32 cnt_mcs_fail;
	u32 cnt_fast_fsync_fail;
	u32 cnt_sb_search_fail;
	u32 cnt_ofdm_fail;
	u32 cnt_cck_fail;
	u32 cnt_all;
	u32 cnt_ofdm_cca;
	u32 cnt_cck_cca;
	u32 cnt_cca_all;
	u32 cnt_bw_usc;
	u32 cnt_bw_lsc;
};

struct rtl_priv {
	struct net_device *ndev;

	struct rtl_hal rtlhal;		/* Caution new Hal data */
	struct rtl_phy phy;
	struct rtl_efuse efuse;
	struct rtl_dm dm;		/* Caution new dm data */
	struct rtl_hal_cfg *cfg;
	struct rtl_io io;
	struct dig_t dm_digtable;
	struct rate_adaptive ra;
	struct rtl_mac mac80211;

	struct false_alarm_statistics falsealm_cnt;
	struct rtl_usb_priv priv;



	struct _rtw_hal *HalData;


	/* Border */

	struct HAL_VERSION VersionID;


	int	DriverState;// for disable driver using module, use dongle to replace module.
	int	bDongle;//build-in module or external dongle
	u16 	chip_type;

	struct	mlme_priv mlmepriv;
	struct	mlme_ext_priv mlmeextpriv;
	struct	cmd_priv	cmdpriv;
	struct	evt_priv	evtpriv;
	//struct	io_queue	*pio_queue;
	struct	xmit_priv	xmitpriv;
	struct	recv_priv	recvpriv;
	struct	sta_priv	stapriv;
	struct	security_priv	securitypriv;
	struct	registry_priv	registrypriv;
	struct	pwrctrl_priv	pwrctrlpriv;
	struct 	eeprom_priv eeprompriv;

#ifdef CONFIG_AP_MODE
	struct	hostapd_priv	*phostapdpriv;
#endif

	u32	setband;

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

	void *cmdThread;
	void *evtThread;
	void *xmitThread;
	void *recvThread;

#ifndef PLATFORM_LINUX
	NDIS_STATUS (*dvobj_init)(struct rtl_usb *dvobj);
	void (*dvobj_deinit)(struct rtl_usb *dvobj);
#endif

	void (*intf_start)(struct rtl_priv * rtlpriv);
	void (*intf_stop)(struct rtl_priv * rtlpriv);


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


struct rtl_stats {
	u8 psaddr[ETH_ALEN];
	u32 mac_time[2];
	s8 rssi;
	u8 signal;
	u8 noise;
	u8 rate;		/* hw desc rate */
	u8 received_channel;
	u8 control;
	u8 mask;
	u8 freq;
	u16 len;
	u64 tsf;
	u32 beacon_time;
	u8 nic_type;
	u16 length;
	u8 signalquality;	/*in 0-100 index. */
	/*
	 * Real power in dBm for this packet,
	 * no beautification and aggregation.
	 * */
	s32 recvsignalpower;
	s8 rxpower;		/*in dBm Translate from PWdB */
	u8 signalstrength;	/*in 0-100 index. */
	u16 hwerror:1;
	u16 crc:1;
	u16 icv:1;
	u16 shortpreamble:1;
	u16 antenna:1;
	u16 decrypted:1;
	u16 wakeup:1;
	u32 timestamp_low;
	u32 timestamp_high;
	bool shift;

	u8 rx_drvinfo_size;
	u8 rx_bufshift;
	bool isampdu;
	bool isfirst_ampdu;
	bool rx_is40Mhzpacket;
	u8 rx_packet_bw;
	u32 rx_pwdb_all;
	u8 rx_mimo_signalstrength[4];	/*in 0~100 index */
	s8 rx_mimo_signalquality[4];
	u8 rx_mimo_evm_dbm[4];
	u16 cfo_short[4];		/* per-path's Cfo_short */
	u16 cfo_tail[4];

	s8 rx_mimo_sig_qual[4];
	u8 rx_pwr[4]; /* per-path's pwdb */
	u8 rx_snr[4]; /* per-path's SNR */
	u8 bandwidth;
	u8 bt_coex_pwr_adjust;
	bool packet_matchbssid;
	bool is_cck;
	bool is_ht;
	bool packet_toself;
	bool packet_beacon;	/*for rssi */
	char cck_adc_pwdb[4];	/*for rx path selection */

	bool is_vht;
	bool is_short_gi;
	u8 vht_nss;

	u8 packet_report_type;

	u32 macid;
	u8 wake_match;
	u32 bt_rx_rssi_percentage;
	u32 macid_valid_entry[2];
};



struct rtl_hal_ops {
	/*
	 * New HAL functions with struct net_device  as first param
	 * this can be (hopefully)switched to struct ieee80211_hw
	 */
	int (*init_sw_vars) (struct net_device *ndev);
	void (*deinit_sw_vars) (struct net_device *ndev);

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

	void	(*init_sw_leds)(struct rtl_priv *rtlpriv);
	void	(*deinit_sw_leds)(struct rtl_priv *rtlpriv);
	void	(*led_control) (struct rtl_priv *rtlpriv, enum led_ctl_mode ledcation);


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

	uint8_t	(*GetHalDefVarHandler)(struct rtl_priv *rtlpriv, HAL_DEF_VARIABLE eVariable, void *pValue);
	uint8_t	(*SetHalDefVarHandler)(struct rtl_priv *rtlpriv, HAL_DEF_VARIABLE eVariable, void *pValue);

	void	(*SetHalODMVarHandler)(struct rtl_priv *rtlpriv, HAL_ODM_VARIABLE eVariable, void *pValue1,BOOLEAN bSet);

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

	void (*hal_notch_filter)(struct rtl_priv * rtlpriv, bool enable);
	void (*hal_reset_security_engine)(struct rtl_priv * rtlpriv);
	int32_t (*c2h_handler)(struct rtl_priv *rtlpriv, struct c2h_evt_hdr *c2h_evt);
	c2h_id_filter c2h_id_filter_ccx;
};


static void rtw_hal_fill_fake_txdesc (struct rtl_priv *rtlpriv, u8 *pDesc,
	u32 BufferLen, u8 IsPsPoll, u8 IsBTQosNull)
{
	rtlpriv->cfg->ops->fill_fake_txdesc(rtlpriv, pDesc, BufferLen, IsPsPoll, IsBTQosNull);
}




/*Only for transition between old (RTW) and new (rtlwifi-lib) API */

uint32_t rtl8812au_hal_init(struct rtl_priv *rtlpriv);
uint32_t rtl8812au_hal_deinit(struct rtl_priv *rtlpriv);
unsigned int rtl8812au_inirp_init(struct rtl_priv *rtlpriv);
unsigned int rtl8812au_inirp_deinit(struct rtl_priv *rtlpriv);
void rtl8812au_init_default_value(struct rtl_priv *rtlpriv);
void rtl8812au_interface_configure(struct rtl_priv *rtlpriv);
void _rtl8821au_read_adapter_info(struct rtl_priv *rtlpriv);

enum hardware_type {
	HARDWARE_TYPE_RTL8812AU,
	HARDWARE_TYPE_RTL8811AU,
	HARDWARE_TYPE_RTL8821U,
	HARDWARE_TYPE_RTL8821S,

	HARDWARE_TYPE_MAX,
};

// RTL8812 Series
#define IS_HARDWARE_TYPE_8812AU(rtlhal)	\
	(rtlhal->hw_type == HARDWARE_TYPE_RTL8812AU)
#define IS_HARDWARE_TYPE_8812(rtlhal)	\
	(IS_HARDWARE_TYPE_8812AU(rtlhal))

// RTL8821 Series
#define IS_HARDWARE_TYPE_8811AU(rtlhal)	\
	(rtlhal->hw_type == HARDWARE_TYPE_RTL8811AU)
#define IS_HARDWARE_TYPE_8821U(rtlhal)	\
	((rtlhal->hw_type == HARDWARE_TYPE_RTL8821U) || \
       	(rtlhal->hw_type == HARDWARE_TYPE_RTL8811AU))
#define IS_HARDWARE_TYPE_8821S(rtlhal)	\
	(rtlhal->hw_type == HARDWARE_TYPE_RTL8821S)
#define IS_HARDWARE_TYPE_8821(rtlhal)		\
	(IS_HARDWARE_TYPE_8821U(rtlhal) || \
	 IS_HARDWARE_TYPE_8821S(rtlhal))

static inline u8 get_rf_type(struct rtl_phy *rtlphy)
{
	return rtlphy->rf_type;
}

/* Not fully compatible */
#define rtl_usbdev(rtlpriv) (&(rtlpriv->priv.dev))


static inline u8 rtl_read_byte(struct rtl_priv *rtlpriv, u32 addr)
{
	return rtlpriv->io.read8_sync(rtlpriv, addr);
}

static inline u16 rtl_read_word(struct rtl_priv *rtlpriv, u32 addr)
{
	return rtlpriv->io.read16_sync(rtlpriv, addr);
}

static inline u32 rtl_read_dword(struct rtl_priv *rtlpriv, u32 addr)
{
	return rtlpriv->io.read32_sync(rtlpriv, addr);
}

static inline void rtl_write_byte(struct rtl_priv *rtlpriv, u32 addr, u8 val8)
{
	rtlpriv->io.write8_async(rtlpriv, addr, val8);
}

static inline void rtl_write_word(struct rtl_priv *rtlpriv, u32 addr, u16 val16)
{
	rtlpriv->io.write16_async(rtlpriv, addr, val16);
}

static inline void rtl_write_dword(struct rtl_priv *rtlpriv,
				   u32 addr, u32 val32)
{
	rtlpriv->io.write32_async(rtlpriv, addr, val32);
}

static inline void rtl_writeN(struct rtl_priv *rtlpriv,
				   u32 addr, void *data, u16 len)
{
	rtlpriv->io.writeN_sync(rtlpriv, addr, data, len);
}



/* ULLI : Hope this is an border, for old code  */





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
	int	LastMinUndecoratedPWDBForDM;

	int32_t	UndecoratedSmoothedBeacon;

//###### duplicate code,will move to ODM #########
	//for High Power
	uint8_t 	bDynamicTxPowerEnable;
	uint8_t 	LastDTPLvl;

	//for tx power tracking
	uint8_t	bTXPowerTracking;
	uint8_t	TXPowercount;
	uint8_t	TxPowerTrackControl;	//for mp mode, turn off txpwrtracking as default
	uint8_t	TM_Trigger;

	//uint8_t   RSVD_2;


	uint8_t	PowerIndex_backup[6];
	uint8_t	OFDM_index[2];

	uint8_t	bCCKinCH14;
	uint8_t	bDoneTxpower;

	uint8_t	OFDM_index_HP[2];
	//uint8_t   RSVD_6;

	u32	prv_traffic_idx; // edca turbo
//###### duplicate code,will move to ODM #########

	// Add for Reading Initial Data Rate SEL Register 0x484 during watchdog. Using for fill tx desc. 2011.3.21 by Thomas
	uint8_t	INIDATA_RATE[32];
};

//
// <Roger_Notes> For RTL8723 WiFi PDn/GPIO polarity control configuration. 2010.10.08.
//

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
	uint32_t	prv_traffic_idx; // edca turbo
}EDCA_T,*pEDCA_T;


//#endif

typedef enum _BASEBAND_CONFIG_PHY_REG_PG_VALUE_TYPE{
	PHY_REG_PG_RELATIVE_VALUE = 0,
	PHY_REG_PG_EXACT_VALUE = 1
} PHY_REG_PG_TYPE;


struct ODM_RF_Calibration_Structure
{
	//for tx power tracking

	u8  	TM_Trigger;
	BOOLEAN	TxPowerTrackingInProgress;
	//--------------------------------------------------------------------//


	BOOLEAN bLCKInProgress;
	BOOLEAN	bAntennaDetected;
};

typedef struct _ODM_RATE_ADAPTIVE
{
	BOOLEAN				bLowerRtsRate;

} ODM_RATE_ADAPTIVE, *PODM_RATE_ADAPTIVE;
typedef struct _ODM_Phy_Dbg_Info_
{
	//ODM Write,debug info
	u8		NumQryBeaconPkt;
	//Others

}ODM_PHY_DBG_INFO_T;

typedef struct _ODM_Mac_Status_Info_
{
	u8	test;

}ODM_MAC_INFO;

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

}RXHP_T, *pRXHP_T;

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
	//
	//	Add for different team use temporarily
	//
	struct rtl_priv *	rtlpriv;		// For CE/NIC team
	// WHen you use rtlpriv or priv pointer, you must make sure the pointer is ready.
	BOOLEAN			odm_ready;

	PHY_REG_PG_TYPE		PhyRegPgValueType;

	uint64_t			NumQryPhyStatusAll; 	//CCK + OFDM
	uint64_t			LastNumQryPhyStatusAll;
	uint64_t			RxPWDBAve;
	uint64_t			RxPWDBAve_final;
	BOOLEAN			MPDIG_2G; 		//off MPDIG
	u8			Times_2G;

//------ ODM HANDLE, DRIVER NEEDS NOT TO HOOK------//
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
	// ODM PCIE/USB/SDIO = 1/2/3
	u8			SupportInterface;
	// Cut Version TestChip/A-cut/B-cut... = 0/1/2/3/...
	u8			CutVersion;

	// with external TRSW  NO/Yes = 0/1
	u8			PatchID; //Customer ID

//-----------HOOK BEFORE REG INIT-----------//

	//
	// Dynamic Value
	//
//--------- POINTER REFERENCE-----------//

	u8			u8_temp;

	// Wireless mode B/G/A/N = BIT0/BIT1/BIT2/BIT3
	u8			*pWirelessMode; //ODM_WIRELESS_MODE_E
	// Security mode Open/WEP/AES/TKIP = 0/1/2/3
	u8			*pSecurity;
 	// Central channel location Ch1/Ch2/....
	u8			*pChannel;	//central channel number
	// Common info for 92D DMSP

	// Common info for Status
	BOOLEAN			*pbPowerSaving;
	//pMgntInfo->AntennaTest
	u8			*pAntennaTest;
	//u8			*pAidMap;
//--------- POINTER REFERENCE-----------//
	u16			*pForcedDataRate;
//------------CALL BY VALUE-------------//
	u8          InterfaceIndex; // Add for 92D  dual MAC: 0--Mac0 1--Mac1
	BOOLEAN			bOneEntryOnly;
	// Common info for BTDM
//------------CALL BY VALUE-------------//
	u8			RSSI_A;
	u8			RSSI_B;
	uint64_t			RSSI_TRSW;
	uint64_t			RSSI_TRSW_H;
	uint64_t			RSSI_TRSW_L;
	uint64_t			RSSI_TRSW_iso;

	u8			RxRate;
	BOOLEAN			StopDIG;
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
	PS_T		DM_PSTable;
	Pri_CCA_T	DM_PriCCA;
	RXHP_T		DM_RXHP_Table;
	//#ifdef CONFIG_ANTENNA_DIVERSITY
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
	//u8		binitialized; // for dm_initial_gain_Multi_STA use.
	//for Antenna diversity
	//u8	AntDivCfg;// 0:OFF , 1:ON, 2:by efuse
	//struct sta_info *RSSI_target;

	//PSD
	BOOLEAN			bUserAssignLevel;
	u8			RSSI_BT;			//come from BT
	BOOLEAN			bPSDinProcess;
	BOOLEAN			bPSDactive;

	ODM_RATE_ADAPTIVE	RateAdaptive;

	ANT_DETECTED_INFO	AntDetectedInfo; // Antenna detected information for RSSI tool

	struct ODM_RF_Calibration_Structure RFCalibrateInfo;

	//
	// TX power tracking
	//

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

	// ODM relative workitem.

	/* Vars moved out of
	 * typedef struct ODM_RF_Calibration_Structure {
	 * }ODM_RF_CAL_T,*PODM_RF_CAL_T;
	 */
};

struct _rtw_hal {
	enum rt_polarity_ctl		PolarityCtl; // For Wifi PDn Polarity control.
	RT_REGULATOR_MODE	RegulatorMode; // switching regulator or LDO

	//current WIFI_PHY values
	enum wireless_mode CurrentWirelessMode;
	uint8_t	CurrentCenterFrequencyIndex1;

	u16	CustomerID;
	u16	BasicRateSet;
	u16 ForcedDataRate;// Force Data Rate. 0: Auto, 0x02: 1M ~ 0x6C: 54M.
	u32	ReceiveConfig;

	//rf_ctrl
	uint8_t	rf_chip;

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



	uint8_t	PGMaxGroup;
	uint8_t	LegacyHTTxPowerDiff;// Legacy to HT rate power diff

	// Read/write are allow for following hardware information variables
	u32	CCKTxPowerLevelOriginalOffset;

	u32	AntennaTxPath;					// Antenna path Tx
	u32	AntennaRxPath;					// Antenna path Rx

	uint8_t	BoardType;
	uint8_t	ExternalPA;
	uint8_t	bIQKInitialized;
	BOOLEAN		bLCKInProgress;

	BOOLEAN		bChnlBWInitialzed;

	uint8_t	bLedOpenDrain; // Support Open-drain arrangement for controlling the LED. Added by Roger, 2009.10.16.
	uint8_t	TxPowerTrackControl; //for mp mode, turn off txpwrtracking as default
	uint8_t	b1x1RecvCombine;	// for 1T1R receive combining

	u32	AcParam_BE; //Original parameter for BE, use for EDCA turbo.

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
	u16	RegRRSR;

	uint8_t	CurAntenna;
	uint8_t	AntDivCfg;

	uint8_t	FwRsvdPageStartOffset; //2010.06.23. Added by tynli. Reserve page start offset except beacon in TxQ.

	// 2010/08/09 MH Add CU power down mode.
	BOOLEAN		pwrdown;

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
};

static inline struct _rtw_hal *GET_HAL_DATA(struct rtl_priv *priv)
{
	return priv->HalData;
}
#define RT_GetInterfaceSelection(_Adapter) 	(GET_HAL_DATA(_Adapter)->InterfaceSel)


#endif
