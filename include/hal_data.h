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
#ifndef __HAL_DATA_H__
#define __HAL_DATA_H__

#if 1//def  CONFIG_SINGLE_IMG

#include "../hal/OUTSRC/odm_precomp.h"

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

#define MAX_BASE_NUM_IN_PHY_REG_PG_2_4G			4 //  CCK:1,OFDM:2, HT:2
#define MAX_BASE_NUM_IN_PHY_REG_PG_5G			5 // OFDM:1, HT:2, VHT:2


//###### duplicate code,will move to ODM #########
#define IQK_MAC_REG_NUM		4
#define IQK_ADDA_REG_NUM		16

#define IQK_BB_REG_NUM			10
#define IQK_BB_REG_NUM_92C	9
#define IQK_BB_REG_NUM_92D	10
#define IQK_BB_REG_NUM_test	6

#define IQK_Matrix_Settings_NUM_92D	1+24+21

#define HP_THERMAL_NUM		8
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

struct dm_priv
{
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


struct rtw_hal {
	HAL_VERSION			VersionID;
	RT_MULTI_FUNC		MultiFunc; // For multi-function consideration.
	RT_POLARITY_CTL		PolarityCtl; // For Wifi PDn Polarity control.
	RT_REGULATOR_MODE	RegulatorMode; // switching regulator or LDO

	u16	FirmwareVersion;
	u16	FirmwareVersionRev;
	u16	FirmwareSubVersion;
	u16	FirmwareSignature;

	//current WIFI_PHY values
	WIRELESS_MODE		CurrentWirelessMode;
	CHANNEL_WIDTH	CurrentChannelBW;
	BAND_TYPE			CurrentBandType;	//0:2.4G, 1:5G
	BAND_TYPE			BandSet;
	uint8_t	CurrentChannel;
	uint8_t	CurrentCenterFrequencyIndex1;
	uint8_t	nCur40MhzPrimeSC;// Control channel sub-carrier
	uint8_t	nCur80MhzPrimeSC;   //used for primary 40MHz of 80MHz mode

	u16	CustomerID;
	u16	BasicRateSet;
	u16 ForcedDataRate;// Force Data Rate. 0: Auto, 0x02: 1M ~ 0x6C: 54M.
	u32	ReceiveConfig;

	//rf_ctrl
	uint8_t	rf_chip;
	uint8_t	rf_type;
	uint8_t	NumTotalRFPath;

	uint8_t	InterfaceSel;
	uint8_t	framesync;
	u32	framesyncC34;
	uint8_t	framesyncMonitor;
	uint8_t	DefaultInitialGain[4];
	//
	// EEPROM setting.
	//
	u16	EEPROMVID;
	u16	EEPROMSVID;
	u16	EEPROMPID;
	u16	EEPROMSDID;

	uint8_t	EEPROMCustomerID;
	uint8_t	EEPROMSubCustomerID;
	uint8_t	EEPROMVersion;
	uint8_t	EEPROMRegulatory;
	uint8_t	EEPROMThermalMeter;
	uint8_t	EEPROMBluetoothCoexist;
	uint8_t	EEPROMBluetoothType;
	uint8_t	EEPROMBluetoothAntNum;
	uint8_t	EEPROMBluetoothAntIsolation;
	uint8_t	EEPROMBluetoothRadioShared;
	uint8_t	bTXPowerDataReadFromEEPORM;
	uint8_t	bAPKThermalMeterIgnore;

	BOOLEAN 		EepromOrEfuse;
	uint8_t				EfuseUsedPercentage;
	u16				EfuseUsedBytes;
	//uint8_t				EfuseMap[2][HWSET_MAX_SIZE_JAGUAR];

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
	uint8_t	Index5G_BW40_Base[MAX_RF_PATH][CHANNEL_MAX_NUMBER];
	uint8_t	Index5G_BW80_Base[MAX_RF_PATH][CHANNEL_MAX_NUMBER_5G_80M];
	s8	OFDM_5G_Diff[MAX_RF_PATH][MAX_TX_COUNT];
	s8	BW20_5G_Diff[MAX_RF_PATH][MAX_TX_COUNT];
	s8	BW40_5G_Diff[MAX_RF_PATH][MAX_TX_COUNT];
	s8	BW80_5G_Diff[MAX_RF_PATH][MAX_TX_COUNT];

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
	uint8_t	TxPwrLimit_2_4G[MAX_REGULATION_NUM]
						[MAX_2_4G_BANDWITH_NUM]
	                                [MAX_2_4G_RATE_SECTION_NUM]
	                                [MAX_2_4G_CHANNEL_NUM]
						[MAX_RF_PATH_NUM];

	// Power Limit Table for 5G
	uint8_t	TxPwrLimit_5G[MAX_REGULATION_NUM]
						[MAX_5G_BANDWITH_NUM]
						[MAX_5G_RATE_SECTION_NUM]
						[MAX_5G_CHANNEL_NUM]
						[MAX_RF_PATH_NUM];


	// Store the original power by rate value of the base of each rate section of rf path A & B
	uint8_t	TxPwrByRateBase2_4G[MAX_RF_PATH_NUM_IN_POWER_LIMIT_TABLE]
						[MAX_BASE_NUM_IN_PHY_REG_PG_2_4G];
	uint8_t	TxPwrByRateBase5G[MAX_RF_PATH_NUM_IN_POWER_LIMIT_TABLE]
						[MAX_BASE_NUM_IN_PHY_REG_PG_5G];

	// For power group
	uint8_t	PwrGroupHT20[RF_PATH_MAX_92C_88E][CHANNEL_MAX_NUMBER];
	uint8_t	PwrGroupHT40[RF_PATH_MAX_92C_88E][CHANNEL_MAX_NUMBER];




	uint8_t	PGMaxGroup;
	uint8_t	LegacyHTTxPowerDiff;// Legacy to HT rate power diff
	// The current Tx Power Level
	uint8_t	CurrentCckTxPwrIdx;
	uint8_t	CurrentOfdm24GTxPwrIdx;
	uint8_t	CurrentBW2024GTxPwrIdx;
	uint8_t	CurrentBW4024GTxPwrIdx;

	// Read/write are allow for following hardware information variables
	uint8_t	pwrGroupCnt;
	u32	MCSTxPowerLevelOriginalOffset[MAX_PG_GROUP][16];
	u32	CCKTxPowerLevelOriginalOffset;

	uint8_t	CrystalCap;
	u32	AntennaTxPath;					// Antenna path Tx
	u32	AntennaRxPath;					// Antenna path Rx

	uint8_t	PAType_2G;
	uint8_t	PAType_5G;
	uint8_t	LNAType_2G;
	uint8_t	LNAType_5G;
	uint8_t	ExternalPA_2G;
	uint8_t	ExternalLNA_2G;
	uint8_t	ExternalPA_5G;
	uint8_t	ExternalLNA_5G;
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
	DM_ODM_T 		odmpriv;

#ifdef DBG_CONFIG_ERROR_DETECT
	struct sreset_priv srestpriv;
#endif


};

#define GET_HAL_DATA(__pAdapter)	(( struct rtw_hal *)((__pAdapter)->HalData))
#define GET_HAL_RFPATH_NUM(__pAdapter) ((( struct rtw_hal *)((__pAdapter)->HalData))->NumTotalRFPath )
#define RT_GetInterfaceSelection(_Adapter) 	(GET_HAL_DATA(_Adapter)->InterfaceSel)
#endif


#endif //__HAL_DATA_H__

