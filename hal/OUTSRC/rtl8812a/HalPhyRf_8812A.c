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

#include "../odm_precomp.h"



/*---------------------------Define Local Constant---------------------------*/
/*  2010/04/25 MH Define the max tx power tracking tx agc power. */
#define		ODM_TXPWRTRACK_MAX_IDX8812A		6

/*---------------------------Define Local Constant---------------------------*/




/*
 * 2011/07/26 MH Add an API for testing IQK fail case.
 * MP Already declare in odm.c
 */

#define BW_20M 	0
#define	BW_40M  1
#define	BW_80M	2

static void phy_LCCalibrate_8812A(struct rtl_dm *pDM_Odm, BOOLEAN	is2T)
{
	uint32_t	/*RF_Amode=0, RF_Bmode=0,*/ LC_Cal = 0, tmp = 0;

	/* Check continuous TX and Packet TX */
	uint32_t	reg0x914 = rtl_read_dword(pDM_Odm->Adapter, rSingleTone_ContTx_Jaguar);;

	/* Backup RF reg18. */
	LC_Cal = rtw_hal_read_rfreg(pDM_Odm->Adapter, RF90_PATH_A, RF_CHNLBW, bRFRegOffsetMask);

	if ((reg0x914 & 0x70000) != 0)	/* If contTx, disable all continuous TX. 0x914[18:16] */
		/*
		 *  <20121121, Kordan> A workaround: If we set 0x914[18:16] as zero, BB turns off ContTx
		 *  until another packet comes in. To avoid ContTx being turned off, we skip this step.
		 * ODM_Write4Byte(pDM_Odm, rSingleTone_ContTx_Jaguar, reg0x914 & (~0x70000));
		 */

		;
	else		/* If packet Tx-ing, pause Tx. */
		rtl_write_byte(pDM_Odm->Adapter, REG_TXPAUSE, 0xFF);


/*
	//3 1. Read original RF mode
	RF_Amode = rtw_hal_read_rfreg(pDM_Odm->Adapter, RF90_PATH_A, RF_AC, bRFRegOffsetMask);
	if(is2T)
		RF_Bmode = rtw_hal_read_rfreg(pDM_Odm->Adapter, RF90_PATH_B, RF_AC, bRFRegOffsetMask);


	//3 2. Set RF mode = standby mode
	rtw_hal_write_rfreg(pDM_Odm->Adapter, RF90_PATH_A, RF_AC, bRFRegOffsetMask, (RF_Amode&0x8FFFF)|0x10000);
	if(is2T)
		rtw_hal_write_rfreg(pDM_Odm->Adapter, RF90_PATH_B, RF_AC, bRFRegOffsetMask, (RF_Bmode&0x8FFFF)|0x10000);
*/

	/* Enter LCK mode */
	tmp = rtw_hal_read_rfreg(pDM_Odm->Adapter, RF90_PATH_A, RF_LCK, bRFRegOffsetMask);
	rtw_hal_write_rfreg(pDM_Odm->Adapter, RF90_PATH_A, RF_LCK, bRFRegOffsetMask, tmp | BIT14);

	/* 3 3. Read RF reg18 */
	LC_Cal = rtw_hal_read_rfreg(pDM_Odm->Adapter, RF90_PATH_A, RF_CHNLBW, bRFRegOffsetMask);

	/* 3 4. Set LC calibration begin bit15 */
	rtw_hal_write_rfreg(pDM_Odm->Adapter, RF90_PATH_A, RF_CHNLBW, bRFRegOffsetMask, LC_Cal|0x08000);

	/* Leave LCK mode */
	tmp = rtw_hal_read_rfreg(pDM_Odm->Adapter, RF90_PATH_A, RF_LCK, bRFRegOffsetMask);
	rtw_hal_write_rfreg(pDM_Odm->Adapter, RF90_PATH_A, RF_LCK, bRFRegOffsetMask, tmp & ~BIT14);

	mdelay(100);

	/* 3 Restore original situation */
	if ((reg0x914 & 70000) != 0) {	/* Deal with contisuous TX case, 0x914[18:16] */
		/*
		 * <20121121, Kordan> A workaround: If we set 0x914[18:16] as zero, BB turns off ContTx
		 * until another packet comes in. To avoid ContTx being turned off, we skip this step.
		 * ODM_Write4Byte(pDM_Odm, rSingleTone_ContTx_Jaguar, reg0x914);
		 */
		;
	} else {
		/* Deal with Packet TX case */
		rtl_write_byte(pDM_Odm->Adapter, REG_TXPAUSE, 0x00);
	}

	/* Recover channel number */
	rtw_hal_write_rfreg(pDM_Odm->Adapter, RF90_PATH_A, RF_CHNLBW, bRFRegOffsetMask, LC_Cal);

	/*
	rtw_hal_write_rfreg(pDM_Odm->Adapter, RF90_PATH_A, RF_AC, bRFRegOffsetMask, RF_Amode);
	if(is2T)
		rtw_hal_write_rfreg(pDM_Odm->Adapter, RF90_PATH_B, RF_AC, bRFRegOffsetMask, RF_Bmode);
		*/

}

#define		DP_BB_REG_NUM		7
#define		DP_RF_REG_NUM		1
#define		DP_RETRY_LIMIT		10
#define		DP_PATH_NUM		2
#define		DP_DPK_NUM		3
#define		DP_DPK_VALUE_NUM	2

void PHY_LCCalibrate_8812A(struct rtl_dm *pDM_Odm)
{
	BOOLEAN 		bStartContTx = FALSE, bSingleTone = FALSE, bCarrierSuppression = FALSE;

	struct rtl_priv *		pAdapter = pDM_Odm->Adapter;
	 struct _rtw_hal	*pHalData = GET_HAL_DATA(pAdapter);

	ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("===> PHY_LCCalibrate_8812A\n"));

	ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("<=== PHY_LCCalibrate_8812A\n"));

}
