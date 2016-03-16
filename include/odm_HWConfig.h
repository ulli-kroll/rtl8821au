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


#ifndef	__HALHWOUTSRC_H__
#define __HALHWOUTSRC_H__

#define AGC_DIFF_CONFIG_MP(ic, band) (ODM_ReadAndConfig_MP_##ic##_AGC_TAB_DIFF(pDM_Odm, Array_MP_##ic##_AGC_TAB_DIFF_##band, \
                                                                              sizeof(Array_MP_##ic##_AGC_TAB_DIFF_##band)/sizeof(uint32_t)))
#define AGC_DIFF_CONFIG_TC(ic, band) (ODM_ReadAndConfig_TC_##ic##_AGC_TAB_DIFF(pDM_Odm, Array_TC_##ic##_AGC_TAB_DIFF_##band, \
                                                                              sizeof(Array_TC_##ic##_AGC_TAB_DIFF_##band)/sizeof(uint32_t)))

#define AGC_DIFF_CONFIG(ic, band) do {\
                                            if (pDM_Odm->bIsMPChip)\
                                    		    AGC_DIFF_CONFIG_MP(ic,band);\
                                            else\
                                                AGC_DIFF_CONFIG_TC(ic,band);\
                                    } while(0)


//============================================================
// structure and define
//============================================================

typedef struct _Phy_Rx_AGC_Info
{
	#if (ODM_ENDIAN_TYPE == ODM_ENDIAN_LITTLE)
		u8	gain:7,trsw:1;
	#else
		u8	trsw:1,gain:7;
	#endif
} PHY_RX_AGC_INFO_T,*pPHY_RX_AGC_INFO_T;


void ODM_ConfigBBWithHeaderFile(struct _rtw_dm *pDM_Odm, ODM_BB_Config_Type ConfigType);



#endif

