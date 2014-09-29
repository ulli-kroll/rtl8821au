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

#ifndef	__ODM_PRECOMP_H__
#define __ODM_PRECOMP_H__

#include "odm_types.h"


#define		TEST_FALG___		1


//2 Config Flags and Structs - defined by each ODM Type

#if (DM_ODM_SUPPORT_TYPE ==ODM_CE)
	//#include <drv_conf.h>
	//#include <basic_types.h>
	//#include <osdep_service.h>
	//#include <drv_types.h>
	//#include <rtw_byteorder.h>
	//#include <hal_intf.h>
#define BEAMFORMING_SUPPORT 0
#endif


//2 Hardware Parameter Files


#if (DM_ODM_SUPPORT_TYPE == ODM_CE)
	#if(RTL8192DU_SUPPORT ==1)
	#include "rtl8192d/Hal8192DUFWImg_CE.h"
	#include "rtl8192d/Hal8192DUPHYImg_CE.h"
	#include "rtl8192d/Hal8192DUMACImg_CE.h"
	#endif

#endif


//2 OutSrc Header Files

#include "odm.h"
#include "odm_HWConfig.h"
#include "odm_debug.h"
#include "odm_RegDefine11AC.h"
#include "odm_RegDefine11N.h"


#if (DM_ODM_SUPPORT_TYPE == ODM_CE)
	//#include "hal_com.h"
	#include "HalPhyRf.h"

	#if (RTL8192D_SUPPORT==1)
	#include  "rtl8192d/HalDMOutSrc8192D_CE.h"
	#include  "rtl8192d_hal.h"
	#endif

	#if (RTL8812A_SUPPORT==1)
		#include "rtl8812a/HalPhyRf_8812A.h"//for IQK,LCK,Power-tracking
		#include "rtl8812a_hal.h"
	#endif

	#if (RTL8821A_SUPPORT==1)
		#include "rtl8821a/HalPhyRf_8821A.h"//for IQK,LCK,Power-tracking
		#include "rtl8812a/HalPhyRf_8812A.h"//for IQK,LCK,Power-tracking
		#include "rtl8812a_hal.h"
	#endif

#endif

#include "odm_interface.h"
#include "odm_reg.h"

#if (RTL8192D_SUPPORT==1)
#include "rtl8192d/odm_RTL8192D.h"
#endif




#if (RTL8812A_SUPPORT==1)
#include "rtl8812a/HalHWImg8812A_MAC.h"
#include "rtl8812a/HalHWImg8812A_RF.h"
#include "rtl8812a/HalHWImg8812A_BB.h"
#include "rtl8812a/HalHWImg8812A_FW.h"
#include "rtl8812a/odm_RegConfig8812A.h"
#if (TEST_CHIP_SUPPORT == 1)
#include "rtl8812a/HalHWImg8812A_TestChip_MAC.h"
#include "rtl8812a/HalHWImg8812A_TestChip_RF.h"
#include "rtl8812a/HalHWImg8812A_TestChip_BB.h"
#endif
#endif


#if (RTL8821A_SUPPORT==1)
#include "rtl8821a/HalHWImg8821A_MAC.h"
#include "rtl8821a/HalHWImg8821A_RF.h"
#include "rtl8821a/HalHWImg8821A_BB.h"
#include "rtl8821a/HalHWImg8821A_FW.h"
#include "rtl8821a/odm_RegConfig8821A.h"
#if (TEST_CHIP_SUPPORT == 1)
#include "rtl8821a/HalHWImg8821A_TestChip_MAC.h"
#include "rtl8821a/HalHWImg8821A_TestChip_RF.h"
#include "rtl8821a/HalHWImg8821A_TestChip_BB.h"
#endif
#endif

#endif	// __ODM_PRECOMP_H__

