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
#ifndef __ODM_TYPES_H__
#define __ODM_TYPES_H__

//
// Define Different SW team support
//
#define	ODM_AP		 	0x01	//BIT0
#define	ODM_CE		 	0x04	//BIT2

#define	DM_ODM_SUPPORT_TYPE			ODM_CE

// Deifne HW endian support
#define	ODM_ENDIAN_BIG	0
#define	ODM_ENDIAN_LITTLE	1

#define 	RT_PCI_INTERFACE				1
#define 	RT_USB_INTERFACE				2
#define 	RT_SDIO_INTERFACE				3

typedef enum _HAL_STATUS{
	HAL_STATUS_SUCCESS,
	HAL_STATUS_FAILURE,
	/*RT_STATUS_PENDING,
	RT_STATUS_RESOURCE,
	RT_STATUS_INVALID_CONTEXT,
	RT_STATUS_INVALID_PARAMETER,
	RT_STATUS_NOT_SUPPORT,
	RT_STATUS_OS_API_FAILED,*/
}HAL_STATUS,*PHAL_STATUS;

#if (DM_ODM_SUPPORT_TYPE == ODM_CE)
typedef enum _RT_SPINLOCK_TYPE{
	RT_TEMP =1,
}RT_SPINLOCK_TYPE;
#endif


#if (DM_ODM_SUPPORT_TYPE == ODM_CE)
	#include <drv_types.h>

#if 0
	typedef u8					u1Byte, *pu1Byte;
	typedef s8					s1Byte,*ps1Byte;
	typedef s16					s2Byte,*ps2Byte;
	typedef s64					s8Byte,*ps8Byte;
#else
	#define u1Byte 		u8
	#define	pu1Byte 	u8*

	#define uint16_t 		u16
	#define	puint16_t 	u16*

	#define s1Byte 		s8
	#define	ps1Byte 	s8*

	#define s2Byte 		s16
	#define	ps2Byte 	s16*

	#define s8Byte 		s64
	#define	ps8Byte 	s64*

#endif
	#ifdef CONFIG_USB_HCI
		#define DEV_BUS_TYPE  	RT_USB_INTERFACE
	#endif


	#if defined(CONFIG_LITTLE_ENDIAN)
		#define	ODM_ENDIAN_TYPE			ODM_ENDIAN_LITTLE
	#elif defined (CONFIG_BIG_ENDIAN)
		#define	ODM_ENDIAN_TYPE			ODM_ENDIAN_BIG
	#endif

	typedef struct timer_list		RT_TIMER, *PRT_TIMER;
	typedef  void *				RT_TIMER_CALL_BACK;
	#define	STA_INFO_T			struct sta_info
	#define	PSTA_INFO_T		struct sta_info *



	#define TRUE 	_TRUE
	#define FALSE	_FALSE


	#define SET_TX_DESC_ANTSEL_A_88E(__pTxDesc, __Value) SET_BITS_TO_LE_4BYTE(__pTxDesc+8, 24, 1, __Value)
	#define SET_TX_DESC_ANTSEL_B_88E(__pTxDesc, __Value) SET_BITS_TO_LE_4BYTE(__pTxDesc+8, 25, 1, __Value)
	#define SET_TX_DESC_ANTSEL_C_88E(__pTxDesc, __Value) SET_BITS_TO_LE_4BYTE(__pTxDesc+28, 29, 1, __Value)

	//define useless flag to avoid compile warning
	#define	USE_WORKITEM 0
	#define 	FOR_BRAZIL_PRETEST 0
	#define	BT_30_SUPPORT			0
	#define   FPGA_TWO_MAC_VERIFICATION	0
#endif


#endif // __ODM_TYPES_H__

