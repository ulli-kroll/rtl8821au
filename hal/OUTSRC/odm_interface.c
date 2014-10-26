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

//============================================================
// include files
//============================================================

#include "odm_precomp.h"
//
// ODM IO Relative API.
//


//
// ODM Memory relative API.
//

// length could be ignored, used to detect memory leakage.
//
// ODM Timer relative API.
//
VOID
ODM_StallExecution(
	IN	uint32_t	usDelay
	)
{
#if(DM_ODM_SUPPORT_TYPE & ODM_CE)
	udelay(usDelay);
#endif
}

VOID
ODM_sleep_us(IN uint32_t	us)
{
#if(DM_ODM_SUPPORT_TYPE & ODM_CE)
	rtw_usleep_os(us);
#endif
}

//
// ODM FW relative API.
//
uint32_t
ODM_FillH2CCmd(
	IN	pu1Byte		pH2CBuffer,
	IN	uint32_t		H2CBufferLen,
	IN	uint32_t		CmdNum,
	IN	uint32_t		*pElementID,
	IN	uint32_t		*pCmdLen,
	IN	pu1Byte*		pCmbBuffer,
	IN	pu1Byte		CmdStartSeq
	)
{
#if(DM_ODM_SUPPORT_TYPE & ODM_CE)

#endif

	return	TRUE;
}





