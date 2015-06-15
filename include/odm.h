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


#ifndef	__HALDMOUTSRC_H__
#define __HALDMOUTSRC_H__

//============================================================
// Definition
//============================================================
//
// 2011/09/22 MH Define all team supprt ability.
//

//
// 2011/09/22 MH Define for all teams. Please Define the constan in your precomp header.
//
//#define		DM_ODM_SUPPORT_AP			0
//#define		DM_ODM_SUPPORT_ADSL			0
//#define		DM_ODM_SUPPORT_CE			0
//#define		DM_ODM_SUPPORT_MP			1

//
// 2011/09/28 MH Define ODM SW team support flag.
//



//
// Antenna Switch Relative Definition.
//

//
// 20100503 Joseph:
// Add new function SwAntDivCheck8192C().
// This is the main function of Antenna diversity function before link.
// Mainly, it just retains last scan result and scan again.
// After that, it compares the scan result to see which one gets better RSSI.
// It selects antenna with better receiving power and returns better scan result.
//
#define	TP_MODE		0
#define	RSSI_MODE		1
#define	TRAFFIC_LOW	0
#define	TRAFFIC_HIGH	1


//============================================================
//3 Tx Power Tracking
//3============================================================
#define DPK_DELTA_MAPPING_NUM	13
#define index_mapping_HP_NUM		15
#define OFDM_TABLE_SIZE 			37
#define OFDM_TABLE_SIZE_92D 		43
#define TXSCALE_TABLE_SIZE 			37
#define DELTA_SWINGIDX_SIZE		30
#define BAND_NUM 					3

//============================================================
//3 PSD Handler
//3============================================================

#define	AFH_PSD		1	//0:normal PSD scan, 1: only do 20 pts PSD
#define	MODE_40M		0	//0:20M, 1:40M
#define	PSD_TH2		3
#define	PSD_CHMIN		20   // Minimum channel number for BT AFH
#define	SIR_STEP_SIZE	3
#define   Smooth_Size_1 	5
#define	Smooth_TH_1	3
#define   Smooth_Size_2 	10
#define	Smooth_TH_2	4
#define   Smooth_Size_3 	20
#define	Smooth_TH_3	4
#define   Smooth_Step_Size 5
#define	Adaptive_SIR	1
#define	PSD_SCAN_INTERVAL	700 //ms



//8723A High Power IGI Setting
#define		DM_DIG_HIGH_PWR_IGI_LOWER_BOUND	0x22
#define  		DM_DIG_Gmode_HIGH_PWR_IGI_LOWER_BOUND 0x28
#define		DM_DIG_HIGH_PWR_THRESHOLD	0x3a
#define		DM_DIG_LOW_PWR_THRESHOLD	0x14

//ANT Test
#define 		ANTTESTALL		0x00		//Ant A or B will be Testing
#define		ANTTESTA		0x01		//Ant A will be Testing
#define		ANTTESTB		0x02		//Ant B will be testing

// LPS define
#define DM_DIG_FA_TH0_LPS				4 //-> 4 in lps
#define DM_DIG_FA_TH1_LPS				15 //-> 15 lps
#define DM_DIG_FA_TH2_LPS				30 //-> 30 lps
#define RSSI_OFFSET_DIG					0x05;



//for 8723A Ant Definition--2012--06--07 due to different IC may be different ANT define
#define		MAIN_ANT		1		//Ant A or Ant Main
#define		AUX_ANT		2		//AntB or Ant Aux
#define		MAX_ANT		3		// 3 for AP using


//Antenna Diversity Type
#define	SW_ANTDIV	0
#define	HW_ANTDIV	1

//Antenna Diversty Control Type
#define	ODM_AUTO_ANT	0
#define	ODM_FIX_MAIN_ANT	1
#define	ODM_FIX_AUX_ANT	2

//============================================================
// structure and define
//============================================================

//
// 2011/09/20 MH Add for AP/ADSLpseudo DM structuer requirement.
// We need to remove to other position???
//



//#ifdef CONFIG_ANTENNA_DIVERSITY
// This indicates two different the steps.
// In SWAW_STEP_PEAK, driver needs to switch antenna and listen to the signal on the air.
// In SWAW_STEP_DETERMINE, driver just compares the signal captured in SWAW_STEP_PEAK
// with original RSSI to determine if it is necessary to switch antenna.
#define SWAW_STEP_PEAK		0
#define SWAW_STEP_DETERMINE	1

#define	TP_MODE		0
#define	RSSI_MODE		1
#define	TRAFFIC_LOW	0
#define	TRAFFIC_HIGH	1






#define IQK_MAC_REG_NUM		4
#define IQK_ADDA_REG_NUM		16
#define IQK_BB_REG_NUM_MAX	10


#define IQK_BB_REG_NUM			10
#define HP_THERMAL_NUM		8


#define		DM_Type_ByFW			0
#define		DM_Type_ByDriver		1

//
// Declare for common info
//
#define MAX_PATH_NUM_92CS			2
#define MAX_PATH_NUM_8188E		1
#define MAX_PATH_NUM_8192E		2
#define MAX_PATH_NUM_8723B		1
#define MAX_PATH_NUM_8812A		2
#define MAX_PATH_NUM_8821A		1

#define IQK_THRESHOLD				8

typedef struct _ODM_Phy_Status_Info_
{
	//
	// Be care, if you want to add any element please insert between
	// RxPWDBAll & SignalStrength.
	//
	u8		RxPWDBAll;

	u8		SignalQuality;	 		// in 0-100 index.
	s8		RxMIMOSignalQuality[4];	//per-path's EVM
	u8		RxMIMOEVMdbm[4]; 		//per-path's EVM dbm

	u8		RxMIMOSignalStrength[4];// in 0~100 index

	u16		Cfo_short[4]; 			// per-path's Cfo_short
	u16		Cfo_tail[4];			// per-path's Cfo_tail

	s8		RxPower;				// in dBm Translate from PWdB
	s8		RecvSignalPower;		// Real power in dBm for this packet, no beautification and aggregation. Keep this raw info to be used for the other procedures.
	u8		BTRxRSSIPercentage;
	u8		SignalStrength; 		// in 0-100 index.

	u8		RxPwr[4];				//per-path's pwdb
	u8		RxSNR[4];				//per-path's SNR
	u8		BandWidth;
	u8		btCoexPwrAdjust;
}ODM_PHY_INFO_T,*PODM_PHY_INFO_T;


typedef struct _ODM_Per_Pkt_Info_
{
	//u8		Rate;
	u8		DataRate;
	u8		StationID;
	BOOLEAN		bPacketMatchBSSID;
	BOOLEAN		bPacketToSelf;
	BOOLEAN		bPacketBeacon;
}ODM_PACKET_INFO_T,*PODM_PACKET_INFO_T;





typedef enum tag_Dynamic_ODM_Support_Ability_Type
{
	// BB Team
	ODM_DIG				= 0x00000001,
	ODM_HIGH_POWER		= 0x00000002,
	ODM_CCK_CCA_TH		= 0x00000004,
	ODM_FA_STATISTICS		= 0x00000008,
	ODM_RAMASK			= 0x00000010,
	ODM_RSSI_MONITOR		= 0x00000020,
	ODM_SW_ANTDIV		= 0x00000040,
	ODM_HW_ANTDIV		= 0x00000080,
	ODM_BB_PWRSV			= 0x00000100,
	ODM_2TPATHDIV			= 0x00000200,
	ODM_1TPATHDIV			= 0x00000400,
	ODM_PSD2AFH			= 0x00000800
}ODM_Ability_E;

//
// 2011/20/20 MH For MP driver RT_WLAN_STA =  STA_INFO_T
// Please declare below ODM relative info in your STA info structure.
//
#if 1
typedef		struct _ODM_STA_INFO{
	// Driver Write
	BOOLEAN		bUsed;				// record the sta status link or not?
	//u8		WirelessMode;		//
	u8		IOTPeer;			// Enum value.	HT_IOT_PEER_E

	// ODM Write
	//1 PHY_STATUS_INFO
	u8		RSSI_Path[4];		//
	u8		RSSI_Ave;
	u8		RXEVM[4];
	u8		RXSNR[4];

	// ODM Write
	//1 TX_INFO (may changed by IC)
	//TX_INFO_T		pTxInfo;				// Define in IC folder. Move lower layer.
#if 0
	u8		ANTSEL_A;			//in Jagar: 4bit; others: 2bit
	u8		ANTSEL_B;			//in Jagar: 4bit; others: 2bit
	u8		ANTSEL_C;			//only in Jagar: 4bit
	u8		ANTSEL_D;			//only in Jagar: 4bit
	u8		TX_ANTL;			//not in Jagar: 2bit
	u8		TX_ANT_HT;			//not in Jagar: 2bit
	u8		TX_ANT_CCK;			//not in Jagar: 2bit
	u8		TXAGC_A;			//not in Jagar: 4bit
	u8		TXAGC_B;			//not in Jagar: 4bit
	u8		TXPWR_OFFSET;		//only in Jagar: 3bit
	u8		TX_ANT;				//only in Jagar: 4bit for TX_ANTL/TX_ANTHT/TX_ANT_CCK
#endif

	//
	// 	Please use compile flag to disabe the strcutrue for other IC except 88E.
	//	Move To lower layer.
	//
	// ODM Write Wilson will handle this part(said by Luke.Lee)
	//TX_RPT_T		pTxRpt;				// Define in IC folder. Move lower layer.
#if 0
	//1 For 88E RA (don't redefine the naming)
	u8		rate_id;
	u8		rate_SGI;
	u8		rssi_sta_ra;
	u8		SGI_enable;
	u8		Decision_rate;
	u8		Pre_rate;
	u8		Active;

	// Driver write Wilson handle.
	//1 TX_RPT (don't redefine the naming)
	u16		RTY[4];				// ???
	u16		TOTAL;				// ???
	u16		DROP;				// ???
	//
	// Please use compile flag to disabe the strcutrue for other IC except 88E.
	//
#endif

}ODM_STA_INFO_T, *PODM_STA_INFO_T;
#endif

//
// 2011/10/20 MH Define Common info enum for all team.
//
typedef enum _ODM_Common_Info_Definition
{
//-------------REMOVED CASE-----------//
	//ODM_CMNINFO_CCK_HP,
	//ODM_CMNINFO_RFPATH_ENABLE,		// Define as ODM write???
	//ODM_CMNINFO_OP_MODE,				// ODM_OPERATION_MODE_E
//-------------REMOVED CASE-----------//

	//
	// Fixed value:
	//

	//-----------HOOK BEFORE REG INIT-----------//
	//-----------HOOK BEFORE REG INIT-----------//


	//
	// Dynamic value:
	//
//--------- POINTER REFERENCE-----------//
	ODM_CMNINFO_WM_MODE,				// ODM_WIRELESS_MODE_E
	ODM_CMNINFO_SEC_MODE,				// ODM_SECURITY_E
	ODM_CMNINFO_CHNL,
	ODM_CMNINFO_FORCED_RATE,

	ODM_CMNINFO_SCAN,
	ODM_CMNINFO_POWER_SAVING,
	ODM_CMNINFO_ANT_TEST,
//--------- POINTER REFERENCE-----------//

//------------CALL BY VALUE-------------//
//------------CALL BY VALUE-------------//

	//
	// Dynamic ptr array hook itms.
	//
	ODM_CMNINFO_PHY_STATUS,
	ODM_CMNINFO_MAC_STATUS,

	ODM_CMNINFO_MAX,


}ODM_CMNINFO_E;

typedef enum tag_ODM_Support_Interface_Definition
{
	ODM_ITRF_USB 	=	0x2,
}ODM_INTERFACE_E;

// ODM_CMNINFO_RF_TYPE
//
// For example 1T2R (A+AB = BIT0|BIT4|BIT5)
//
typedef enum tag_ODM_RF_Path_Bit_Definition
{
	ODM_RF_TX_A 	=	BIT0,
	ODM_RF_TX_B 	=	BIT1,
	ODM_RF_TX_C	=	BIT2,
	ODM_RF_TX_D	=	BIT3,
	ODM_RF_RX_A	=	BIT4,
	ODM_RF_RX_B	=	BIT5,
	ODM_RF_RX_C	=	BIT6,
	ODM_RF_RX_D	=	BIT7,
}ODM_RF_PATH_E;


typedef enum tag_ODM_RF_Type_Definition
{
	ODM_1T1R 	=	0,
	ODM_1T2R 	=	1,
	ODM_2T2R	=	2,
	ODM_2T3R	=	3,
	ODM_2T4R	=	4,
	ODM_3T3R	=	5,
	ODM_3T4R	=	6,
	ODM_4T4R	=	7,
}ODM_RF_TYPE_E;


//
// ODM Dynamic common info value definition
//

//typedef enum _MACPHY_MODE_8192D{
//	SINGLEMAC_SINGLEPHY,
//	DUALMAC_DUALPHY,
//	DUALMAC_SINGLEPHY,
//}MACPHY_MODE_8192D,*PMACPHY_MODE_8192D;
// Above is the original define in MP driver. Please use the same define. THX.
typedef enum tag_ODM_MAC_PHY_Mode_Definition
{
	ODM_SMSP	= 0,
	ODM_DMSP	= 1,
	ODM_DMDP	= 2,
}ODM_MAC_PHY_MODE_E;


typedef enum tag_BT_Coexist_Definition
{
	ODM_BT_BUSY 		= 1,
	ODM_BT_ON 			= 2,
	ODM_BT_OFF 		= 3,
	ODM_BT_NONE 		= 4,
}ODM_BT_COEXIST_E;

// ODM_CMNINFO_OP_MODE
typedef enum tag_Operation_Mode_Definition
{
	ODM_NO_LINK 		= BIT0,
	ODM_LINK 			= BIT1,
	ODM_SCAN 			= BIT2,
	ODM_POWERSAVE 	= BIT3,
	ODM_CLIENT_MODE	= BIT5,
	ODM_AD_HOC 		= BIT6,
	ODM_WIFI_DIRECT	= BIT7,
	ODM_WIFI_DISPLAY	= BIT8,
}ODM_OPERATION_MODE_E;

// ODM_CMNINFO_SEC_MODE
typedef enum tag_Security_Definition
{
	ODM_SEC_OPEN 			= 0,
	ODM_SEC_WEP40 		= 1,
	ODM_SEC_TKIP 			= 2,
	ODM_SEC_RESERVE 		= 3,
	ODM_SEC_AESCCMP 		= 4,
	ODM_SEC_WEP104 		= 5,
	ODM_WEP_WPA_MIXED    = 6, // WEP + WPA
}ODM_SECURITY_E;

// ODM_CMNINFO_BW
typedef enum tag_Bandwidth_Definition
{
	ODM_BW20M 		= 0,
	ODM_BW40M 		= 1,
	ODM_BW80M 		= 2,
	ODM_BW160M 		= 3,
	ODM_BW10M 		= 4,
}ODM_BW_E;

// ODM_CMNINFO_CHNL




typedef struct _ODM_RA_Info_
{
	u8 RateID;
	uint32_t RateMask;
	uint32_t RAUseRate;
	u8 RateSGI;
	u8 RssiStaRA;
	u8 PreRssiStaRA;
	u8 SGIEnable;
	u8 DecisionRate;
	u8 PreRate;
	u8 HighestRate;
	u8 LowestRate;
	uint32_t NscUp;
	uint32_t NscDown;
	u16 RTY[5];
	uint32_t TOTAL;
	u16 DROP;
	u8 Active;
	u16 RptTime;
	u8 RAWaitingCounter;
	u8 RAPendingCounter;
#if 1 //POWER_TRAINING_ACTIVE == 1 // For compile  pass only~!
	u8 PTActive;  // on or off
	u8 PTTryState;  // 0 trying state, 1 for decision state
	u8 PTStage;  // 0~6
	u8 PTStopCount; //Stop PT counter
	u8 PTPreRate;  // if rate change do PT
	u8 PTPreRssi; // if RSSI change 5% do PT
	u8 PTModeSS;  // decide whitch rate should do PT
	u8 RAstage;  // StageRA, decide how many times RA will be done between PT
	u8 PTSmoothFactor;
#endif
} ODM_RA_INFO_T,*PODM_RA_INFO_T;

typedef struct _IQK_MATRIX_REGS_SETTING{
	BOOLEAN 	bIQKDone;
	int32_t		Value[3][IQK_Matrix_REG_NUM];
	BOOLEAN 	bBWIqkResultSaved[3];
}IQK_MATRIX_REGS_SETTING,*PIQK_MATRIX_REGS_SETTING;

//
// ODM Dynamic common info value definition
//

typedef enum _FAT_STATE
{
	FAT_NORMAL_STATE			= 0,
	FAT_TRAINING_STATE 		= 1,
}FAT_STATE_E, *PFAT_STATE_E;

//
// Antenna detection information from single tone mechanism, added by Roger, 2012.11.27.
//


#if 1 //92c-series
#define ODM_RF_PATH_MAX 2
#else //jaguar - series
#define ODM_RF_PATH_MAX 4
#endif
 typedef enum _ODM_RF_CONTENT{
	odm_radioa_txt = 0x1000,
	odm_radiob_txt = 0x1001,
	odm_radioc_txt = 0x1002,
	odm_radiod_txt = 0x1003
} ODM_RF_CONTENT;

typedef enum _ODM_BB_Config_Type{
    CONFIG_BB_PHY_REG,
    CONFIG_BB_AGC_TAB,
    CONFIG_BB_AGC_TAB_2G,
    CONFIG_BB_AGC_TAB_5G,
    CONFIG_BB_PHY_REG_PG,
} ODM_BB_Config_Type, *PODM_BB_Config_Type;

typedef enum _ODM_RF_Config_Type{
	CONFIG_RF_RADIO,
} ODM_RF_Config_Type, *PODM_RF_Config_Type;

// Status code
typedef enum _RT_STATUS{
	RT_STATUS_SUCCESS,
	RT_STATUS_FAILURE,
	RT_STATUS_PENDING,
	RT_STATUS_RESOURCE,
	RT_STATUS_INVALID_CONTEXT,
	RT_STATUS_INVALID_PARAMETER,
	RT_STATUS_NOT_SUPPORT,
	RT_STATUS_OS_API_FAILED,
}RT_STATUS,*PRT_STATUS;

#ifdef REMOVE_PACK
#pragma pack()
#endif

//#include "odm_function.h"

//3===========================================================
//3 DIG
//3===========================================================

typedef enum tag_Dynamic_Init_Gain_Operation_Type_Definition
{
	DIG_TYPE_THRESH_HIGH	= 0,
	DIG_TYPE_THRESH_LOW	= 1,
	DIG_TYPE_BACKOFF		= 2,
	DIG_TYPE_RX_GAIN_MIN	= 3,
	DIG_TYPE_RX_GAIN_MAX	= 4,
	DIG_TYPE_ENABLE 		= 5,
	DIG_TYPE_DISABLE 		= 6,
	DIG_OP_TYPE_MAX
}DM_DIG_OP_E;
/*
typedef enum tag_CCK_Packet_Detection_Threshold_Type_Definition
{
	CCK_PD_STAGE_LowRssi = 0,
	CCK_PD_STAGE_HighRssi = 1,
	CCK_PD_STAGE_MAX = 3,
}DM_CCK_PDTH_E;

typedef enum tag_DIG_EXT_PORT_ALGO_Definition
{
	DIG_EXT_PORT_STAGE_0 = 0,
	DIG_EXT_PORT_STAGE_1 = 1,
	DIG_EXT_PORT_STAGE_2 = 2,
	DIG_EXT_PORT_STAGE_3 = 3,
	DIG_EXT_PORT_STAGE_MAX = 4,
}DM_DIG_EXT_PORT_ALG_E;

typedef enum tag_DIG_Connect_Definition
{
	DIG_STA_DISCONNECT = 0,
	DIG_STA_CONNECT = 1,
	DIG_STA_BEFORE_CONNECT = 2,
	DIG_MultiSTA_DISCONNECT = 3,
	DIG_MultiSTA_CONNECT = 4,
	DIG_CONNECT_MAX
}DM_DIG_CONNECT_E;


#define DM_MultiSTA_InitGainChangeNotify(Event) {DM_DigTable.CurMultiSTAConnectState = Event;}

#define DM_MultiSTA_InitGainChangeNotify_CONNECT(_ADAPTER)	\
	DM_MultiSTA_InitGainChangeNotify(DIG_MultiSTA_CONNECT)

#define DM_MultiSTA_InitGainChangeNotify_DISCONNECT(_ADAPTER)	\
	DM_MultiSTA_InitGainChangeNotify(DIG_MultiSTA_DISCONNECT)
*/
#define		DM_DIG_THRESH_HIGH			40
#define		DM_DIG_THRESH_LOW			35

#define		DM_FALSEALARM_THRESH_LOW	400
#define		DM_FALSEALARM_THRESH_HIGH	1000

#define		DM_DIG_MAX_NIC				0x3e
#define		DM_DIG_MIN_NIC				0x1e //0x22//0x1c

#define		DM_DIG_MAX_AP					0x32
#define		DM_DIG_MIN_AP					0x20

#define		DM_DIG_MAX_NIC_HP			0x46
#define		DM_DIG_MIN_NIC_HP			0x2e

#define		DM_DIG_MAX_AP_HP				0x42
#define		DM_DIG_MIN_AP_HP				0x30

//vivi 92c&92d has different definition, 20110504
//this is for 92c
#define		DM_DIG_FA_TH0				0x200//0x20
#define		DM_DIG_FA_TH1				0x300//0x100
#define		DM_DIG_FA_TH2				0x400//0x200
//this is for 92d
#define		DM_DIG_FA_TH0_92D			0x100
#define		DM_DIG_FA_TH1_92D			0x400
#define		DM_DIG_FA_TH2_92D			0x600

#define		DM_DIG_BACKOFF_MAX			12
#define		DM_DIG_BACKOFF_MIN			-4
#define		DM_DIG_BACKOFF_DEFAULT		10

//3===========================================================
//3 AGC RX High Power Mode
//3===========================================================
#define          LNA_Low_Gain_1                      0x64
#define          LNA_Low_Gain_2                      0x5A
#define          LNA_Low_Gain_3                      0x58

#define          FA_RXHP_TH1                           5000
#define          FA_RXHP_TH2                           1500
#define          FA_RXHP_TH3                             800
#define          FA_RXHP_TH4                             600
#define          FA_RXHP_TH5                             500

//3===========================================================
//3 EDCA
//3===========================================================

//3===========================================================
//3 Tx Power Tracking
//3===========================================================
#if 0 //mask this, since these have been defined in typdef.h, vivi
#define	OFDM_TABLE_SIZE 	37
#define	OFDM_TABLE_SIZE_92D 	43
#endif


//3===========================================================
//3 Rate Adaptive
//3===========================================================
#define		DM_RATR_STA_INIT			0
#define		DM_RATR_STA_HIGH			1
#define 		DM_RATR_STA_MIDDLE		2
#define 		DM_RATR_STA_LOW			3

//3===========================================================
//3 BB Power Save
//3===========================================================


//3===========================================================
//3 Dynamic ATC switch
//3===========================================================
#define		ATC_Status_Off				0x0			// enable
#define		ATC_Status_On				0x1			// disable
#define		CFO_Threshold_Xtal			10			// kHz
#define		CFO_Threshold_ATC			80			// kHz

typedef enum tag_1R_CCA_Type_Definition
{
	CCA_1R =0,
	CCA_2R = 1,
	CCA_MAX = 2,
}DM_1R_CCA_E;

typedef enum tag_RF_Type_Definition
{
	RF_Save =0,
	RF_Normal = 1,
	RF_MAX = 2,
}DM_RF_E;

//3===========================================================
//3 Antenna Diversity
//3===========================================================
typedef enum tag_SW_Antenna_Switch_Definition
{
	Antenna_A = 1,
	Antenna_B = 2,
	Antenna_MAX = 3,
}DM_SWAS_E;


// Maximal number of antenna detection mechanism needs to perform, added by Roger, 2011.12.28.
#define	MAX_ANTENNA_DETECTION_CNT	10

//
// Extern Global Variables.
//
extern  uint32_t TxScalingTable_Jaguar[TXSCALE_TABLE_SIZE];

// <20121018, Kordan> In case fail to read TxPowerTrack.txt, we use the table of 88E as the default table.

//
// check Sta pointer valid or not
//
#define IS_STA_VALID(pSta)		(pSta)
// 20100514 Joseph: Add definition for antenna switching test after link.
// This indicates two different the steps.
// In SWAW_STEP_PEAK, driver needs to switch antenna and listen to the signal on the air.
// In SWAW_STEP_DETERMINE, driver just compares the signal captured in SWAW_STEP_PEAK
// with original RSSI to determine if it is necessary to switch antenna.
#define SWAW_STEP_PEAK		0
#define SWAW_STEP_DETERMINE	1

void ODM_Write_DIG(IN	struct _rtw_dm *pDM_Odm, 	IN	u8	CurrentIGI);
void ODM_Write_CCK_CCA_Thres(struct rtl_priv *rtlpriv, u8 CurCCK_CCAThres);

void
ODM_SetAntenna(
	IN 	struct _rtw_dm *pDM_Odm,
	IN	u8		Antenna);


#define dm_CheckTXPowerTracking 	ODM_TXPowerTrackingCheck
void
ODM_TXPowerTrackingCheck(
	IN		struct _rtw_dm *	pDM_Odm
	);

BOOLEAN
ODM_RAStateCheck(
	IN		struct _rtw_dm *	pDM_Odm,
	IN		int32_t			RSSI,
	IN		BOOLEAN			bForceUpdate,
	OUT		u8 *		pRATRState
	);


uint32_t
GetPSDData(
	struct _rtw_dm *pDM_Odm,
	unsigned int 	point,
	u8 initial_gain_psd);



void
odm_DIGbyRSSI_LPS(
	IN		struct _rtw_dm *	pDM_Odm
	);

uint32_t ODM_Get_Rate_Bitmap(
	IN	struct _rtw_dm *pDM_Odm,
	IN	uint32_t		macid,
	IN	uint32_t 		ra_mask,
	IN	u8 		rssi_level);



void ODM_DMInit(struct rtl_priv *rtlpriv);

void
ODM_CmnInfoHook(
	IN		struct _rtw_dm *	pDM_Odm,
	IN		ODM_CMNINFO_E	CmnInfo,
	IN		void 	*pValue
	);

void
ODM_CmnInfoPtrArrayHook(
	IN		struct _rtw_dm *	pDM_Odm,
	IN		ODM_CMNINFO_E	CmnInfo,
	IN		u16			Index,
	IN		void *pValue
	);


void
ODM_InitAllTimers(
    IN struct _rtw_dm *pDM_Odm
    );

void
ODM_CancelAllTimers(
    IN struct _rtw_dm *   pDM_Odm
    );

void
ODM_ReleaseAllTimers(
    IN struct _rtw_dm *pDM_Odm
    );

void
ODM_AntselStatistics_88C(
	IN		struct _rtw_dm *	pDM_Odm,
	IN		u8			MacId,
	IN		uint32_t			PWDBAll,
	IN		BOOLEAN			isCCKrate
);


void
ODM_UpdateInitRate(
	IN	struct _rtw_dm *pDM_Odm,
	IN	u8		Rate
	);

#endif

