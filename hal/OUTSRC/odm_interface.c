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
VOID
ODM_AllocateMemory(
	IN 	PDM_ODM_T	pDM_Odm,
	OUT	PVOID		*pPtr,
	IN	uint32_t		length
	)
{
#if (DM_ODM_SUPPORT_TYPE & ODM_CE )
	*pPtr = rtw_zvmalloc(length);
#endif
}

// length could be ignored, used to detect memory leakage.
VOID
ODM_FreeMemory(
	IN 	PDM_ODM_T	pDM_Odm,
	OUT	PVOID		pPtr,
	IN	uint32_t		length
	)
{
#if (DM_ODM_SUPPORT_TYPE & ODM_CE )
	/* ULLI check usage of length */
	rtw_vmfree(pPtr);
#endif
}

int32_t ODM_CompareMemory(
	IN 	PDM_ODM_T	pDM_Odm,
	IN	PVOID           pBuf1,
      IN	PVOID           pBuf2,
      IN	uint32_t          length
       )
{
#if (DM_ODM_SUPPORT_TYPE & ODM_CE )
	return _rtw_memcmp(pBuf1,pBuf2,length);
#endif
}

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

VOID
ODM_SetTimer(
	IN 	PDM_ODM_T		pDM_Odm,
	IN	PRT_TIMER 		pTimer,
	IN	uint32_t 			msDelay
	)
{
#if(DM_ODM_SUPPORT_TYPE & ODM_CE)
	_set_timer(pTimer,msDelay ); //ms
#endif

}

VOID
ODM_InitializeTimer(
	IN 	PDM_ODM_T			pDM_Odm,
	IN	PRT_TIMER 			pTimer,
	IN	RT_TIMER_CALL_BACK	CallBackFunc,
	IN	PVOID				pContext,
	IN	const char*			szID
	)
{
#if(DM_ODM_SUPPORT_TYPE & ODM_CE)
	PADAPTER Adapter = pDM_Odm->Adapter;
	_init_timer(pTimer,Adapter->ndev,CallBackFunc,pDM_Odm);
#endif
}


VOID
ODM_CancelTimer(
	IN 	PDM_ODM_T		pDM_Odm,
	IN	PRT_TIMER		pTimer
	)
{
#if(DM_ODM_SUPPORT_TYPE & ODM_CE)
	_cancel_timer_ex(pTimer);
#endif
}


VOID
ODM_ReleaseTimer(
	IN 	PDM_ODM_T		pDM_Odm,
	IN	PRT_TIMER		pTimer
	)
{
#if(DM_ODM_SUPPORT_TYPE & ODM_CE)

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





