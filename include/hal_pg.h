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

#ifndef __HAL_PG_H__
#define __HAL_PG_H__

//====================================================
//			EEPROM/Efuse PG Offset for 8192 CE/CU
//====================================================
#define EEPROM_VID_92C							0x0A
#define EEPROM_PID_92C							0x0C
#define EEPROM_DID_92C							0x0C
#define EEPROM_SVID_92C						0x0E
#define EEPROM_SMID_92C						0x10
#define EEPROM_MAC_ADDR_92C					0x16

#define EEPROM_MAC_ADDR						0x16
#define EEPROM_TV_OPTION						0x50
#define EEPROM_SUBCUSTOMER_ID_92C			0x59
#define EEPROM_CCK_TX_PWR_INX					0x5A
#define EEPROM_HT40_1S_TX_PWR_INX			0x60
#define EEPROM_HT40_2S_TX_PWR_INX_DIFF		0x66
#define EEPROM_HT20_TX_PWR_INX_DIFF			0x69
#define EEPROM_OFDM_TX_PWR_INX_DIFF			0x6C
#define EEPROM_HT40_MAX_PWR_OFFSET			0x6F
#define EEPROM_HT20_MAX_PWR_OFFSET			0x72
#define EEPROM_CHANNEL_PLAN_92C 				0x75
#define EEPROM_TSSI_A							0x76
#define EEPROM_TSSI_B							0x77
#define EEPROM_THERMAL_METER_92C				0x78
#define EEPROM_RF_OPT1_92C					0x79
#define EEPROM_RF_OPT2_92C					0x7A
#define EEPROM_RF_OPT3_92C					0x7B
#define EEPROM_RF_OPT4_92C					0x7C
#define EEPROM_VERSION_92C						0x7E
#define EEPROM_CUSTOMER_ID_92C				0x7F

#define EEPROM_NORMAL_CHANNEL_PLAN			0x75
#define EEPROM_NORMAL_BoardType_92C			EEPROM_RF_OPT1_92C
#define BOARD_TYPE_NORMAL_MASK				0xE0
#define BOARD_TYPE_TEST_MASK					0xF
#define EEPROM_TYPE_ID							0x7E

// EEPROM address for Test chip
#define EEPROM_TEST_USB_OPT					0x0E

#define EEPROM_EASY_REPLACEMENT				0x50//BIT0 1 for build-in module, 0 for external dongle


//====================================================
//			EEPROM/Efuse Value Type
//====================================================
#define EETYPE_TX_PWR							0x0
//====================================================
//			EEPROM/Efuse Default Value
//====================================================
#define EEPROM_CID_DEFAULT_EXT				0xFF // Reserved for Realtek
#define EEPROM_CID_TOSHIBA						0x4
#define EEPROM_CID_CCX							0x10
#define EEPROM_CID_QMI							0x0D

#define EEPROM_CHANNEL_PLAN_FCC				0x0
#define EEPROM_CHANNEL_PLAN_IC				0x1
#define EEPROM_CHANNEL_PLAN_ETSI				0x2
#define EEPROM_CHANNEL_PLAN_SPAIN			0x3
#define EEPROM_CHANNEL_PLAN_FRANCE			0x4
#define EEPROM_CHANNEL_PLAN_MKK				0x5
#define EEPROM_CHANNEL_PLAN_MKK1				0x6
#define EEPROM_CHANNEL_PLAN_ISRAEL			0x7
#define EEPROM_CHANNEL_PLAN_TELEC			0x8
#define EEPROM_CHANNEL_PLAN_GLOBAL_DOMAIN	0x9
#define EEPROM_CHANNEL_PLAN_WORLD_WIDE_13	0xA
#define EEPROM_CHANNEL_PLAN_NCC_TAIWAN		0xB
#define EEPROM_CHANNEL_PLAN_CHIAN			0XC
#define EEPROM_CHANNEL_PLAN_SINGAPORE_INDIA_MEXICO  0XD
#define EEPROM_CHANNEL_PLAN_KOREA			0xE
#define EEPROM_CHANNEL_PLAN_TURKEY              	0xF
#define EEPROM_CHANNEL_PLAN_JAPAN                 	0x10
#define EEPROM_CHANNEL_PLAN_FCC_NO_DFS		0x11
#define EEPROM_CHANNEL_PLAN_JAPAN_NO_DFS	0x12
#define EEPROM_CHANNEL_PLAN_WORLD_WIDE_5G	0x13
#define EEPROM_CHANNEL_PLAN_TAIWAN_NO_DFS 	0x14

#define EEPROM_USB_OPTIONAL1					0xE
#define EEPROM_CHANNEL_PLAN_BY_HW_MASK		0x80


#define EEPROM_Default_TSSI						0x0
#define EEPROM_Default_BoardType				0x02
#define EEPROM_Default_ThermalMeter			0x12
#define EEPROM_Default_ThermalMeter_92SU		0x7
#define EEPROM_Default_ThermalMeter_88E		0x18

#define	EEPROM_Default_ThermalMeter_8192E			0x1A
#define	EEPROM_Default_ThermalMeter_8723B		0x18



#define EEPROM_Default_TxPowerLevel_92C		0x22
#define EEPROM_Default_TxPowerLevel_2G			0x2C
#define EEPROM_Default_TxPowerLevel_5G			0x22
#define EEPROM_Default_TxPowerLevel			0x22
#define EEPROM_Default_HT40_2SDiff				0x0
#define EEPROM_Default_HT20_Diff				2
#define EEPROM_Default_LegacyHTTxPowerDiff		0x3
#define EEPROM_Default_LegacyHTTxPowerDiff_92C	0x3
#define EEPROM_Default_LegacyHTTxPowerDiff_92D	0x4
#define EEPROM_Default_HT40_PwrMaxOffset		0
#define EEPROM_Default_HT20_PwrMaxOffset		0

#define EEPROM_Default_externalPA_C9		0x00
#define EEPROM_Default_externalPA_CC		0xFF
#define EEPROM_Default_internalPA_SP3T_C9	0xAA
#define EEPROM_Default_internalPA_SP3T_CC	0xAF
#define EEPROM_Default_internalPA_SPDT_C9	0xAA
#define EEPROM_Default_internalPA_SPDT_CC	0xFA



#define EEPROM_DEFAULT_TX_CALIBRATE_RATE	0x00

//
// For VHT series TX power by rate table.
// VHT TX power by rate off setArray =
// Band:-2G&5G = 0 / 1
// RF: at most 4*4 = ABCD=0/1/2/3
// CCK=0 OFDM=1/2 HT-MCS 0-15=3/4/56 VHT=7/8/9/10/11
//
#define TX_PWR_BY_RATE_NUM_BAND			2
#define TX_PWR_BY_RATE_NUM_RF			4
#define TX_PWR_BY_RATE_NUM_SECTION		12

//----------------------------------------------------------------------------
//       EEPROM/EFUSE data structure definition.
//----------------------------------------------------------------------------


//For 88E new structure

/*
2.4G:
{
{1,2},
{3,4,5},
{6,7,8},
{9,10,11},
{12,13},
{14}
}

5G:
{
{36,38,40},
{44,46,48},
{52,54,56},
{60,62,64},
{100,102,104},
{108,110,112},
{116,118,120},
{124,126,128},
{132,134,136},
{140,142,144},
{149,151,153},
{157,159,161},
{173,175,177},
}
*/

#endif
