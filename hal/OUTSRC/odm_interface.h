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


#ifndef	__ODM_INTERFACE_H__
#define __ODM_INTERFACE_H__




#define _reg_11AC(_name)		ODM_REG_##_name##_11AC
#define _bit_11AC(_name)		ODM_BIT_##_name##_11AC

#if 1 //TODO: enable it if we need to support run-time to differentiate between 92C_SERIES and JAGUAR_SERIES.
#define _cat(_name, _ic_type, _func)	_func##_11AC(_name)
#endif

// _name: name of register or bit.
// Example: "ODM_REG(R_A_AGC_CORE1, pDM_Odm)"
//        gets "ODM_R_A_AGC_CORE1" or "ODM_R_A_AGC_CORE1_8192C", depends on SupportICType.
#define ODM_REG(_name, _pDM_Odm)	_cat(_name, _pDM_Odm->SupportICType, _reg)
#define ODM_BIT(_name, _pDM_Odm)	_cat(_name, _pDM_Odm->SupportICType, _bit)

typedef enum _ODM_H2C_CMD
{
	ODM_H2C_RSSI_REPORT = 0,
	ODM_H2C_PSD_RESULT=1,
	ODM_H2C_PathDiv = 2,
	ODM_MAX_H2CCMD
}ODM_H2C_CMD;


//
// 2012/02/17 MH For non-MP compile pass only. Linux does not support workitem.
// Suggest HW team to use thread instead of workitem. Windows also support the feature.
//
typedef  void *PRT_WORK_ITEM ;
typedef  void RT_WORKITEM_HANDLE,*PRT_WORKITEM_HANDLE;
typedef VOID (*RT_WORKITEM_CALL_BACK)(PVOID pContext);


static u1Byte ODM_Read1Byte(PDM_ODM_T pDM_Odm, uint32_t RegAddr)
{
	return rtw_read8(pDM_Odm->Adapter,RegAddr);
}

static uint16_t ODM_Read2Byte(PDM_ODM_T pDM_Odm, uint32_t RegAddr)
{
	return rtw_read16(pDM_Odm->Adapter,RegAddr);
}

static uint32_t ODM_Read4Byte(PDM_ODM_T pDM_Odm, uint32_t RegAddr)
{
	return rtw_read32(pDM_Odm->Adapter,RegAddr);
}

static void ODM_Write1Byte(PDM_ODM_T pDM_Odm, uint32_t RegAddr, u1Byte Data)
{
	rtw_write8(pDM_Odm->Adapter,RegAddr, Data);
}

static void ODM_Write2Byte(PDM_ODM_T pDM_Odm, uint32_t RegAddr, uint16_t Data)
{
	rtw_write16(pDM_Odm->Adapter,RegAddr, Data);
}

static void ODM_Write4Byte(PDM_ODM_T pDM_Odm, uint32_t RegAddr, uint32_t Data)
{
	rtw_write32(pDM_Odm->Adapter,RegAddr, Data);
}


static void ODM_SetMACReg(PDM_ODM_T pDM_Odm, uint32_t RegAddr,
	uint32_t BitMask, uint32_t Data)
{
	rtl_set_bbreg(pDM_Odm->Adapter, RegAddr, BitMask, Data);
}


static uint32_t ODM_GetMACReg(PDM_ODM_T pDM_Odm, uint32_t RegAddr,
	uint32_t BitMask)
{
	return rtl_get_bbreg(pDM_Odm->Adapter, RegAddr, BitMask);
}


static void ODM_SetBBReg(PDM_ODM_T pDM_Odm, uint32_t RegAddr,
	uint32_t BitMask, uint32_t Data)
{
	struct rtl_priv *	Adapter = pDM_Odm->Adapter;
	rtl_set_bbreg(pDM_Odm->Adapter, RegAddr, BitMask, Data);
}

static uint32_t ODM_GetBBReg(PDM_ODM_T pDM_Odm, uint32_t RegAddr,
	uint32_t BitMask)
{
	return rtl_get_bbreg(pDM_Odm->Adapter, RegAddr, BitMask);
}

static void ODM_SetRFReg(PDM_ODM_T pDM_Odm, ODM_RF_RADIO_PATH_E eRFPath,
	uint32_t RegAddr, uint32_t BitMask, uint32_t Data)
{
	PHY_SetRFReg(pDM_Odm->Adapter, eRFPath, RegAddr, BitMask, Data);
}

static uint32_t ODM_GetRFReg(PDM_ODM_T pDM_Odm, ODM_RF_RADIO_PATH_E eRFPath,
	uint32_t RegAddr, uint32_t BitMask)
{
	return PHY_QueryRFReg(pDM_Odm->Adapter, eRFPath, RegAddr, BitMask);
}

#endif	// __ODM_INTERFACE_H__

