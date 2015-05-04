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

#include <odm_types.h>


#define		TEST_FALG___		1


//2 Config Flags and Structs - defined by each ODM Type



//2 Hardware Parameter Files




//2 OutSrc Header Files

#include <odm.h>
#include <odm_HWConfig.h>
#include <odm_debug.h>
#include <odm_RegDefine11AC.h>

	//#include "hal_com.h"
	#include <HalPhyRf.h>

		#include <HalPhyRf_8812A.h>//for IQK,LCK,Power-tracking
		#include <rtl8812a_hal.h>
		#include <HalPhyRf_8821A.h>//for IQK,LCK,Power-tracking
		#include <HalPhyRf_8812A.h>//for IQK,LCK,Power-tracking
		#include <rtl8812a_hal.h>


#include <odm_interface.h>
#include <odm_reg.h>

#include <HalHWImg8812A_BB.h>
#include <HalHWImg8812A_FW.h>


#include <HalHWImg8821A_BB.h>
#include <HalHWImg8821A_FW.h>
#include <odm_RegConfig8821A.h>

#endif	// __ODM_PRECOMP_H__

