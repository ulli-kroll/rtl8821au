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

#include <odm_precomp.h>



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

#define		DP_BB_REG_NUM		7
#define		DP_RF_REG_NUM		1
#define		DP_RETRY_LIMIT		10
#define		DP_PATH_NUM		2
#define		DP_DPK_NUM		3
#define		DP_DPK_VALUE_NUM	2
