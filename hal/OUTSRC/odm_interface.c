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

u1Byte
ODM_Read1Byte(
	IN 	PDM_ODM_T		pDM_Odm,
	IN	uint32_t			RegAddr
	)
{
#if(DM_ODM_SUPPORT_TYPE & ODM_CE)
	PADAPTER		Adapter = pDM_Odm->Adapter;
	return rtw_read8(Adapter,RegAddr);
#endif

}


uint16_t
ODM_Read2Byte(
	IN 	PDM_ODM_T		pDM_Odm,
	IN	uint32_t			RegAddr
	)
{
#if(DM_ODM_SUPPORT_TYPE & ODM_CE)
	PADAPTER		Adapter = pDM_Odm->Adapter;
	return rtw_read16(Adapter,RegAddr);
#endif

}


uint32_t
ODM_Read4Byte(
	IN 	PDM_ODM_T		pDM_Odm,
	IN	uint32_t			RegAddr
	)
{
#if(DM_ODM_SUPPORT_TYPE & ODM_CE)
	PADAPTER		Adapter = pDM_Odm->Adapter;
	return rtw_read32(Adapter,RegAddr);
#endif

}


VOID
ODM_Write1Byte(
	IN 	PDM_ODM_T		pDM_Odm,
	IN	uint32_t			RegAddr,
	IN	u1Byte			Data
	)
{
#if(DM_ODM_SUPPORT_TYPE & ODM_CE)
	PADAPTER		Adapter = pDM_Odm->Adapter;
	rtw_write8(Adapter,RegAddr, Data);
#endif

}


VOID
ODM_Write2Byte(
	IN 	PDM_ODM_T		pDM_Odm,
	IN	uint32_t			RegAddr,
	IN	uint16_t			Data
	)
{
#if(DM_ODM_SUPPORT_TYPE & ODM_CE)
	PADAPTER		Adapter = pDM_Odm->Adapter;
	rtw_write16(Adapter,RegAddr, Data);
#endif

}


VOID
ODM_Write4Byte(
	IN 	PDM_ODM_T		pDM_Odm,
	IN	uint32_t			RegAddr,
	IN	uint32_t			Data
	)
{
#if(DM_ODM_SUPPORT_TYPE & ODM_CE)
	PADAPTER		Adapter = pDM_Odm->Adapter;
	rtw_write32(Adapter,RegAddr, Data);
#endif
}


VOID
ODM_SetMACReg(
	IN 	PDM_ODM_T	pDM_Odm,
	IN	uint32_t		RegAddr,
	IN	uint32_t		BitMask,
	IN	uint32_t		Data
	)
{
#if(DM_ODM_SUPPORT_TYPE & (ODM_CE))
	PADAPTER		Adapter = pDM_Odm->Adapter;
	PHY_SetBBReg(Adapter, RegAddr, BitMask, Data);
#endif
}


uint32_t
ODM_GetMACReg(
	IN 	PDM_ODM_T	pDM_Odm,
	IN	uint32_t		RegAddr,
	IN	uint32_t		BitMask
	)
{
#if(DM_ODM_SUPPORT_TYPE & (ODM_CE))
	PADAPTER		Adapter = pDM_Odm->Adapter;
	return PHY_QueryBBReg(Adapter, RegAddr, BitMask);
#endif
}


VOID
ODM_SetBBReg(
	IN 	PDM_ODM_T	pDM_Odm,
	IN	uint32_t		RegAddr,
	IN	uint32_t		BitMask,
	IN	uint32_t		Data
	)
{
#if(DM_ODM_SUPPORT_TYPE & (ODM_CE))
	PADAPTER		Adapter = pDM_Odm->Adapter;
	PHY_SetBBReg(Adapter, RegAddr, BitMask, Data);
#endif
}


uint32_t
ODM_GetBBReg(
	IN 	PDM_ODM_T	pDM_Odm,
	IN	uint32_t		RegAddr,
	IN	uint32_t		BitMask
	)
{
#if(DM_ODM_SUPPORT_TYPE & (ODM_CE))
	PADAPTER		Adapter = pDM_Odm->Adapter;
	return PHY_QueryBBReg(Adapter, RegAddr, BitMask);
#endif
}


VOID
ODM_SetRFReg(
	IN 	PDM_ODM_T			pDM_Odm,
	IN	ODM_RF_RADIO_PATH_E	eRFPath,
	IN	uint32_t				RegAddr,
	IN	uint32_t				BitMask,
	IN	uint32_t				Data
	)
{
#if(DM_ODM_SUPPORT_TYPE & (ODM_CE))
	PADAPTER		Adapter = pDM_Odm->Adapter;
	PHY_SetRFReg(Adapter, eRFPath, RegAddr, BitMask, Data);
#endif
}


uint32_t
ODM_GetRFReg(
	IN 	PDM_ODM_T			pDM_Odm,
	IN	ODM_RF_RADIO_PATH_E	eRFPath,
	IN	uint32_t				RegAddr,
	IN	uint32_t				BitMask
	)
{
#if(DM_ODM_SUPPORT_TYPE & (ODM_CE))
	PADAPTER		Adapter = pDM_Odm->Adapter;
	return PHY_QueryRFReg(Adapter, eRFPath, RegAddr, BitMask);
#endif
}




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
// ODM MISC relative API.
//
VOID
ODM_ReleaseSpinLock(
	IN 	PDM_ODM_T			pDM_Odm,
	IN	RT_SPINLOCK_TYPE	type
	)
{
#if (DM_ODM_SUPPORT_TYPE & ODM_CE )

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





