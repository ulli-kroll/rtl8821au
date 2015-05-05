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

//#include "Mp_Precomp.h"
#include <odm_precomp.h>
#include <../rtl8821au/table.h>

#undef ODM_RT_TRACE
#define ODM_RT_TRACE(x, ...)	do {} while (0);

static BOOLEAN CheckCondition(const uint32_t  Condition, const uint32_t  Hex)
{
	uint32_t _board     = (Hex & 0x000000FF);
	uint32_t _interface = (Hex & 0x0000FF00) >> 8;
	uint32_t _platform  = (Hex & 0x00FF0000) >> 16;
	uint32_t cond = Condition;

	if (Condition == 0xCDCDCDCD )
		return TRUE;

	cond = Condition & 0x000000FF;
	if ((_board != cond) && (cond != 0xFF))
		return FALSE;

	cond = Condition & 0x0000FF00;
	cond = cond >> 8;
	if (((_interface & cond) == 0) && (cond != 0x07))
		return FALSE;

	cond = Condition & 0x00FF0000;
	cond = cond >> 16;
	if (((_platform & cond) == 0) && (cond != 0x0F))
		return FALSE;

	return TRUE;
}


/******************************************************************************
*                           AGC_TAB.TXT
******************************************************************************/


/******************************************************************************
*                           PHY_REG.TXT
******************************************************************************/
/******************************************************************************
*                           PHY_REG_PG.TXT
******************************************************************************/



