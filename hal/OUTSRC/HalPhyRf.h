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

typedef VOID 	(*FuncSetPwr)(struct rtl_dm *,PWRTRACK_METHOD, u8, u8);
typedef VOID 	(*FuncIQK)(struct rtl_dm *, u8, u8, u8);
typedef VOID 	(*FuncLCK)(struct rtl_dm *);
typedef VOID  	(*FuncSwing)(struct rtl_dm *, u8 **, u8 **, u8 **, u8 **);

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
	IN 	struct rtl_dm *	pDM_Odm,
	OUT	PTXPWRTRACK_CFG	pConfig
	);


VOID
ODM_ClearTxPowerTrackingState(
	IN struct rtl_dm *	pDM_Odm
	);

VOID
ODM_TXPowerTrackingCallback_ThermalMeter(
	IN struct rtl_priv *Adapter
	);



#define ODM_TARGET_CHNL_NUM_2G_5G	59


VOID
ODM_ResetIQKResult(
	IN struct rtl_dm *pDM_Odm
);
u8
ODM_GetRightChnlPlaceforIQK(
    IN u8 chnl
);


#endif	// #ifndef __HAL_PHY_RF_H__

