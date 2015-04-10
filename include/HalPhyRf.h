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

 #ifndef __HAL_PHY_RF_H__
 #define __HAL_PHY_RF_H__

typedef enum _SPUR_CAL_METHOD {
	PLL_RESET,
	AFE_PHASE_SEL
} SPUR_CAL_METHOD;

typedef enum _PWRTRACK_CONTROL_METHOD {
	BBSWING,
	TXAGC,
	MIX_MODE
} PWRTRACK_METHOD;

typedef void 	(*FuncSetPwr)(struct _rtw_dm *,PWRTRACK_METHOD, u8, u8);
typedef void 	(*FuncIQK)(struct _rtw_dm *, u8, u8, u8);
typedef void 	(*FuncLCK)(struct _rtw_dm *);
typedef void  	(*FuncSwing)(struct _rtw_dm *, u8 **, u8 **, u8 **, u8 **);

typedef struct _TXPWRTRACK_CFG {
	u8 		SwingTableSize_CCK;
	u8 		SwingTableSize_OFDM;
	u8 		Threshold_IQK;
	u8 		AverageThermalNum;
	u8 		RfPathCount;
	uint32_t 		ThermalRegAddr;
	FuncSetPwr 	ODM_TxPwrTrackSetPwr;
	FuncIQK 	DoIQK;
	FuncLCK		PHY_LCCalibrate;
	FuncSwing	GetDeltaSwingTable;
} TXPWRTRACK_CFG, *PTXPWRTRACK_CFG;

void ConfigureTxpowerTrack(
	IN 	struct _rtw_dm *	pDM_Odm,
	OUT	PTXPWRTRACK_CFG	pConfig
	);


void
ODM_ClearTxPowerTrackingState(
	IN struct _rtw_dm *	pDM_Odm
	);

void
ODM_TXPowerTrackingCallback_ThermalMeter(
	IN struct rtl_priv *rtlpriv
	);



#define ODM_TARGET_CHNL_NUM_2G_5G	59

#endif	// #ifndef __HAL_PHY_RF_H__

