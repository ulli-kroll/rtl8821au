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
#ifndef __INC_HAL8812PHYCFG_H__
#define __INC_HAL8812PHYCFG_H__


/*--------------------------Define Parameters-------------------------------*/
#define LOOP_LIMIT				5
#define MAX_STALL_TIME			50		//us
#define AntennaDiversityValue	0x80	//(Adapter->bSoftwareAntennaDiversity ? 0x00:0x80)
#define MAX_TXPWR_IDX_NMODE_92S	63
#define Reset_Cnt_Limit			3


#define MAX_AGGR_NUM	0x07


/*--------------------------Define Parameters-------------------------------*/

/*------------------------------Define structure----------------------------*/


/* BB/RF related */

/*------------------------------Define structure----------------------------*/


/*------------------------Export global variable----------------------------*/
/*------------------------Export global variable----------------------------*/


/*------------------------Export Marco Definition---------------------------*/
/*------------------------Export Marco Definition---------------------------*/


/*--------------------------Exported Function prototype---------------------*/
//
// BB and RF register read/write
//
u32	PHY_QueryRFReg8812(	IN	struct rtl_priv *Adapter,
								IN	uint8_t			eRFPath,
								IN	u32			RegAddr,
								IN	u32			BitMask	);
void	PHY_SetRFReg8812(	IN	struct rtl_priv *	Adapter,
								IN	uint8_t			eRFPath,
								IN	u32			RegAddr,
								IN	u32			BitMask,
								IN	u32			Data	);

//
// Initialization related function
//
/* MAC/BB/RF HAL config */
int	PHY_MACConfig8812(IN struct rtl_priv *Adapter	);
int	PHY_BBConfig8812(IN struct rtl_priv *Adapter	);
void	PHY_BB8812_Config_1T(IN struct rtl_priv *Adapter );
int	PHY_RFConfig8812(IN struct rtl_priv *Adapter	);

VOID
PHY_InitPowerLimitTable(
	IN	struct rtl_dm *pDM_Odm
	);

VOID
PHY_ConvertPowerLimitToPowerIndex(
	IN	struct rtl_priv *		Adapter
	);

/* RF config */

int32_t
PHY_SwitchWirelessBand8812(
	IN struct rtl_priv *	Adapter,
	IN uint8_t			Band
);

//
// BB TX Power R/W
//
void	PHY_GetTxPowerLevel8812(	IN struct rtl_priv *Adapter, OUT u32*	powerlevel	);
void	PHY_SetTxPowerLevel8812(	IN struct rtl_priv *Adapter, IN uint8_t	Channel	);
BOOLEAN	PHY_UpdateTxPowerDbm8812( IN struct rtl_priv *Adapter, IN int	powerInDbm	);
u32 PHY_GetTxPowerIndex_8812A(
	IN	struct rtl_priv *		pAdapter,
	IN	uint8_t					RFPath,
	IN	uint8_t					Rate,
	IN	CHANNEL_WIDTH		BandWidth,
	IN	uint8_t					Channel
	);

u32 PHY_GetTxBBSwing_8812A(
	IN	struct rtl_priv *Adapter,
	IN	BAND_TYPE 	Band,
	IN	uint8_t			RFPath
	);

VOID
PHY_SetTxPowerIndex_8812A(
	IN	struct rtl_priv *		Adapter,
	IN	uint32_t				PowerIndex,
	IN	u8				RFPath,
	IN	u8				Rate
	);
//
// Switch bandwidth for 8192S
//
VOID
PHY_SetBWMode8812(
	IN	struct rtl_priv *		pAdapter,
	IN	CHANNEL_WIDTH		Bandwidth,
	IN	uint8_t					Offset
);

//
// channel switch related funciton
//
VOID
PHY_SwChnl8812(
	IN	struct rtl_priv *Adapter,
	IN	uint8_t			channel
);


VOID
PHY_SetSwChnlBWMode8812(
	IN	struct rtl_priv *		Adapter,
	IN	uint8_t					channel,
	IN	CHANNEL_WIDTH		Bandwidth,
	IN	uint8_t					Offset40,
	IN	uint8_t					Offset80
);

//
// BB/MAC/RF other monitor API
//

VOID
storePwrIndexDiffRateOffset(
	IN	struct rtl_priv *Adapter,
	IN	u32		RegAddr,
	IN	u32		BitMask,
	IN	u32		Data
	);

/*--------------------------Exported Function prototype---------------------*/
#endif	// __INC_HAL8192CPHYCFG_H

