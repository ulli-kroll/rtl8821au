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
#define _HAL_PHY_C_

#include <drv_types.h>

/*
 * ================================================================================
 *	Constant.
 * ================================================================================
 * 2008/11/20 MH For Debug only, RF
 */
static RF_SHADOW_T RF_Shadow[RF6052_MAX_PATH][RF6052_MAX_REG];

/**
* Function:	PHY_CalculateBitShift
*
* OverView:	Get shifted position of the BitMask
*
* Input:
*			uint32_t		BitMask,
*
* Output:	none
* Return:		uint32_t		Return the shift bit bit position of the mask
*/
u32 PHY_CalculateBitShift(uint32_t BitMask)
{
	uint32_t i;

	for (i = 0; i <= 31; i++) {
		if (((BitMask >> i) & 0x1) == 1)
			break;
	}

	return i;
}

