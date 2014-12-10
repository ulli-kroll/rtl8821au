#include "dm.h"
#include "phy.h"
#include "../hal/OUTSRC/odm_precomp.h"

u8 CCKSwingTable_Ch1_Ch13_New[CCK_TABLE_SIZE][8] = {
	{0x09, 0x08, 0x07, 0x06, 0x04, 0x03, 0x01, 0x01},	/*  0, -16.0dB */
	{0x09, 0x09, 0x08, 0x06, 0x05, 0x03, 0x01, 0x01},	/*  1, -15.5dB */
	{0x0a, 0x09, 0x08, 0x07, 0x05, 0x03, 0x02, 0x01},	/*  2, -15.0dB */
	{0x0a, 0x0a, 0x09, 0x07, 0x05, 0x03, 0x02, 0x01},	/*  3, -14.5dB */
	{0x0b, 0x0a, 0x09, 0x08, 0x06, 0x04, 0x02, 0x01},	/*  4, -14.0dB */
	{0x0b, 0x0b, 0x0a, 0x08, 0x06, 0x04, 0x02, 0x01},	/*  5, -13.5dB */
	{0x0c, 0x0c, 0x0a, 0x09, 0x06, 0x04, 0x02, 0x01},	/*  6, -13.0dB */
	{0x0d, 0x0c, 0x0b, 0x09, 0x07, 0x04, 0x02, 0x01},	/*  7, -12.5dB */
	{0x0d, 0x0d, 0x0c, 0x0a, 0x07, 0x05, 0x02, 0x01},	/*  8, -12.0dB */
	{0x0e, 0x0e, 0x0c, 0x0a, 0x08, 0x05, 0x02, 0x01},	/*  9, -11.5dB */
	{0x0f, 0x0f, 0x0d, 0x0b, 0x08, 0x05, 0x03, 0x01},	/* 10, -11.0dB */
	{0x10, 0x10, 0x0e, 0x0b, 0x08, 0x05, 0x03, 0x01},	/* 11, -10.5dB */
	{0x11, 0x11, 0x0f, 0x0c, 0x09, 0x06, 0x03, 0x01},	/* 12, -10.0dB */
	{0x12, 0x12, 0x0f, 0x0c, 0x09, 0x06, 0x03, 0x01},	/* 13, -9.5dB */
	{0x13, 0x13, 0x10, 0x0d, 0x0a, 0x06, 0x03, 0x01},	/* 14, -9.0dB */
	{0x14, 0x14, 0x11, 0x0e, 0x0b, 0x07, 0x03, 0x02},	/* 15, -8.5dB */
	{0x16, 0x15, 0x12, 0x0f, 0x0b, 0x07, 0x04, 0x01},	/* 16, -8.0dB */
	{0x17, 0x16, 0x13, 0x10, 0x0c, 0x08, 0x04, 0x02},	/* 17, -7.5dB */
	{0x18, 0x17, 0x15, 0x11, 0x0c, 0x08, 0x04, 0x02},	/* 18, -7.0dB */
	{0x1a, 0x19, 0x16, 0x12, 0x0d, 0x09, 0x04, 0x02},	/* 19, -6.5dB */
	{0x1b, 0x1a, 0x17, 0x13, 0x0e, 0x09, 0x04, 0x02},	/* 20, -6.0dB */
	{0x1d, 0x1c, 0x18, 0x14, 0x0f, 0x0a, 0x05, 0x02},	/* 21, -5.5dB */
	{0x1f, 0x1e, 0x1a, 0x15, 0x10, 0x0a, 0x05, 0x02},	/* 22, -5.0dB */
	{0x20, 0x20, 0x1b, 0x16, 0x11, 0x08, 0x05, 0x02},	/* 23, -4.5dB */
	{0x22, 0x21, 0x1d, 0x18, 0x11, 0x0b, 0x06, 0x02},	/* 24, -4.0dB */
	{0x24, 0x23, 0x1f, 0x19, 0x13, 0x0c, 0x06, 0x03},	/* 25, -3.5dB */
	{0x26, 0x25, 0x21, 0x1b, 0x14, 0x0d, 0x06, 0x03},	/* 26, -3.0dB */
	{0x28, 0x28, 0x22, 0x1c, 0x15, 0x0d, 0x07, 0x03},	/* 27, -2.5dB */
	{0x2b, 0x2a, 0x25, 0x1e, 0x16, 0x0e, 0x07, 0x03},	/* 28, -2.0dB */
	{0x2d, 0x2d, 0x27, 0x1f, 0x18, 0x0f, 0x08, 0x03},	/* 29, -1.5dB */
	{0x30, 0x2f, 0x29, 0x21, 0x19, 0x10, 0x08, 0x03},	/* 30, -1.0dB */
	{0x33, 0x32, 0x2b, 0x23, 0x1a, 0x11, 0x08, 0x04},	/* 31, -0.5dB */
	{0x36, 0x35, 0x2e, 0x25, 0x1c, 0x12, 0x09, 0x04} 	/* 32, +0dB */
};

u8 CCKSwingTable_Ch14_New[CCK_TABLE_SIZE][8] = {
	{0x09, 0x08, 0x07, 0x04, 0x00, 0x00, 0x00, 0x00},	/*  0, -16.0dB */
	{0x09, 0x09, 0x08, 0x05, 0x00, 0x00, 0x00, 0x00},	/*  1, -15.5dB */
	{0x0a, 0x09, 0x08, 0x05, 0x00, 0x00, 0x00, 0x00},	/*  2, -15.0dB */
	{0x0a, 0x0a, 0x09, 0x05, 0x00, 0x00, 0x00, 0x00},	/*  3, -14.5dB */
	{0x0b, 0x0a, 0x09, 0x05, 0x00, 0x00, 0x00, 0x00},	/*  4, -14.0dB */
	{0x0b, 0x0b, 0x0a, 0x06, 0x00, 0x00, 0x00, 0x00},	/*  5, -13.5dB */
	{0x0c, 0x0c, 0x0a, 0x06, 0x00, 0x00, 0x00, 0x00},	/*  6, -13.0dB */
	{0x0d, 0x0c, 0x0b, 0x06, 0x00, 0x00, 0x00, 0x00},	/*  7, -12.5dB */
	{0x0d, 0x0d, 0x0c, 0x07, 0x00, 0x00, 0x00, 0x00},	/*  8, -12.0dB */
	{0x0e, 0x0e, 0x0c, 0x07, 0x00, 0x00, 0x00, 0x00},	/*  9, -11.5dB */
	{0x0f, 0x0f, 0x0d, 0x08, 0x00, 0x00, 0x00, 0x00},	/* 10, -11.0dB */
	{0x10, 0x10, 0x0e, 0x08, 0x00, 0x00, 0x00, 0x00},	/* 11, -10.5dB */
	{0x11, 0x11, 0x0f, 0x09, 0x00, 0x00, 0x00, 0x00},	/* 12, -10.0dB */
	{0x12, 0x12, 0x0f, 0x09, 0x00, 0x00, 0x00, 0x00},	/* 13, -9.5dB */
	{0x13, 0x13, 0x10, 0x0a, 0x00, 0x00, 0x00, 0x00},	/* 14, -9.0dB */
	{0x14, 0x14, 0x11, 0x0a, 0x00, 0x00, 0x00, 0x00},	/* 15, -8.5dB */
	{0x16, 0x15, 0x12, 0x0b, 0x00, 0x00, 0x00, 0x00},	/* 16, -8.0dB */
	{0x17, 0x16, 0x13, 0x0b, 0x00, 0x00, 0x00, 0x00},	/* 17, -7.5dB */
	{0x18, 0x17, 0x15, 0x0c, 0x00, 0x00, 0x00, 0x00},	/* 18, -7.0dB */
	{0x1a, 0x19, 0x16, 0x0d, 0x00, 0x00, 0x00, 0x00},	/* 19, -6.5dB */
	{0x1b, 0x1a, 0x17, 0x0e, 0x00, 0x00, 0x00, 0x00},	/* 20, -6.0dB */
	{0x1d, 0x1c, 0x18, 0x0e, 0x00, 0x00, 0x00, 0x00},	/* 21, -5.5dB */
	{0x1f, 0x1e, 0x1a, 0x0f, 0x00, 0x00, 0x00, 0x00},	/* 22, -5.0dB */
	{0x20, 0x20, 0x1b, 0x10, 0x00, 0x00, 0x00, 0x00},	/* 23, -4.5dB */
	{0x22, 0x21, 0x1d, 0x11, 0x00, 0x00, 0x00, 0x00},	/* 24, -4.0dB */
	{0x24, 0x23, 0x1f, 0x12, 0x00, 0x00, 0x00, 0x00},	/* 25, -3.5dB */
	{0x26, 0x25, 0x21, 0x13, 0x00, 0x00, 0x00, 0x00},	/* 26, -3.0dB */
	{0x28, 0x28, 0x24, 0x14, 0x00, 0x00, 0x00, 0x00},	/* 27, -2.5dB */
	{0x2b, 0x2a, 0x25, 0x15, 0x00, 0x00, 0x00, 0x00},	/* 28, -2.0dB */
	{0x2d, 0x2d, 0x17, 0x17, 0x00, 0x00, 0x00, 0x00},	/* 29, -1.5dB */
	{0x30, 0x2f, 0x29, 0x18, 0x00, 0x00, 0x00, 0x00},	/* 30, -1.0dB */
	{0x33, 0x32, 0x2b, 0x19, 0x00, 0x00, 0x00, 0x00},	/* 31, -0.5dB */
	{0x36, 0x35, 0x2e, 0x1b, 0x00, 0x00, 0x00, 0x00} 	/* 32, +0dB */
};


u8	CCKSwingTable_Ch1_Ch13[CCK_TABLE_SIZE][8] = {
	{0x36, 0x35, 0x2e, 0x25, 0x1c, 0x12, 0x09, 0x04},	/* 0, +0dB */
	{0x33, 0x32, 0x2b, 0x23, 0x1a, 0x11, 0x08, 0x04},	/* 1, -0.5dB */
	{0x30, 0x2f, 0x29, 0x21, 0x19, 0x10, 0x08, 0x03},	/* 2, -1.0dB */
	{0x2d, 0x2d, 0x27, 0x1f, 0x18, 0x0f, 0x08, 0x03},	/* 3, -1.5dB */
	{0x2b, 0x2a, 0x25, 0x1e, 0x16, 0x0e, 0x07, 0x03},	/* 4, -2.0dB */
	{0x28, 0x28, 0x22, 0x1c, 0x15, 0x0d, 0x07, 0x03},	/* 5, -2.5dB */
	{0x26, 0x25, 0x21, 0x1b, 0x14, 0x0d, 0x06, 0x03},	/* 6, -3.0dB */
	{0x24, 0x23, 0x1f, 0x19, 0x13, 0x0c, 0x06, 0x03},	/* 7, -3.5dB */
	{0x22, 0x21, 0x1d, 0x18, 0x11, 0x0b, 0x06, 0x02},	/* 8, -4.0dB */
	{0x20, 0x20, 0x1b, 0x16, 0x11, 0x08, 0x05, 0x02},	/* 9, -4.5dB */
	{0x1f, 0x1e, 0x1a, 0x15, 0x10, 0x0a, 0x05, 0x02},	/* 10, -5.0dB */
	{0x1d, 0x1c, 0x18, 0x14, 0x0f, 0x0a, 0x05, 0x02},	/* 11, -5.5dB */
	{0x1b, 0x1a, 0x17, 0x13, 0x0e, 0x09, 0x04, 0x02},	/* 12, -6.0dB <== default */
	{0x1a, 0x19, 0x16, 0x12, 0x0d, 0x09, 0x04, 0x02},	/* 13, -6.5dB */
	{0x18, 0x17, 0x15, 0x11, 0x0c, 0x08, 0x04, 0x02},	/* 14, -7.0dB */
	{0x17, 0x16, 0x13, 0x10, 0x0c, 0x08, 0x04, 0x02},	/* 15, -7.5dB */
	{0x16, 0x15, 0x12, 0x0f, 0x0b, 0x07, 0x04, 0x01},	/* 16, -8.0dB */
	{0x14, 0x14, 0x11, 0x0e, 0x0b, 0x07, 0x03, 0x02},	/* 17, -8.5dB */
	{0x13, 0x13, 0x10, 0x0d, 0x0a, 0x06, 0x03, 0x01},	/* 18, -9.0dB */
	{0x12, 0x12, 0x0f, 0x0c, 0x09, 0x06, 0x03, 0x01},	/* 19, -9.5dB */
	{0x11, 0x11, 0x0f, 0x0c, 0x09, 0x06, 0x03, 0x01},	/* 20, -10.0dB */
	{0x10, 0x10, 0x0e, 0x0b, 0x08, 0x05, 0x03, 0x01},	/* 21, -10.5dB */
	{0x0f, 0x0f, 0x0d, 0x0b, 0x08, 0x05, 0x03, 0x01},	/* 22, -11.0dB */
	{0x0e, 0x0e, 0x0c, 0x0a, 0x08, 0x05, 0x02, 0x01},	/* 23, -11.5dB */
	{0x0d, 0x0d, 0x0c, 0x0a, 0x07, 0x05, 0x02, 0x01},	/* 24, -12.0dB */
	{0x0d, 0x0c, 0x0b, 0x09, 0x07, 0x04, 0x02, 0x01},	/* 25, -12.5dB */
	{0x0c, 0x0c, 0x0a, 0x09, 0x06, 0x04, 0x02, 0x01},	/* 26, -13.0dB */
	{0x0b, 0x0b, 0x0a, 0x08, 0x06, 0x04, 0x02, 0x01},	/* 27, -13.5dB */
	{0x0b, 0x0a, 0x09, 0x08, 0x06, 0x04, 0x02, 0x01},	/* 28, -14.0dB */
	{0x0a, 0x0a, 0x09, 0x07, 0x05, 0x03, 0x02, 0x01},	/* 29, -14.5dB */
	{0x0a, 0x09, 0x08, 0x07, 0x05, 0x03, 0x02, 0x01},	/* 30, -15.0dB */
	{0x09, 0x09, 0x08, 0x06, 0x05, 0x03, 0x01, 0x01},	/* 31, -15.5dB */
	{0x09, 0x08, 0x07, 0x06, 0x04, 0x03, 0x01, 0x01}	/* 32, -16.0dB */
};


u8	CCKSwingTable_Ch14[CCK_TABLE_SIZE][8] = {
	{0x36, 0x35, 0x2e, 0x1b, 0x00, 0x00, 0x00, 0x00},	/* 0, +0dB */
	{0x33, 0x32, 0x2b, 0x19, 0x00, 0x00, 0x00, 0x00},	/* 1, -0.5dB */
	{0x30, 0x2f, 0x29, 0x18, 0x00, 0x00, 0x00, 0x00},	/* 2, -1.0dB */
	{0x2d, 0x2d, 0x17, 0x17, 0x00, 0x00, 0x00, 0x00},	/* 3, -1.5dB */
	{0x2b, 0x2a, 0x25, 0x15, 0x00, 0x00, 0x00, 0x00},	/* 4, -2.0dB */
	{0x28, 0x28, 0x24, 0x14, 0x00, 0x00, 0x00, 0x00},	/* 5, -2.5dB */
	{0x26, 0x25, 0x21, 0x13, 0x00, 0x00, 0x00, 0x00},	/* 6, -3.0dB */
	{0x24, 0x23, 0x1f, 0x12, 0x00, 0x00, 0x00, 0x00},	/* 7, -3.5dB */
	{0x22, 0x21, 0x1d, 0x11, 0x00, 0x00, 0x00, 0x00},	/* 8, -4.0dB */
	{0x20, 0x20, 0x1b, 0x10, 0x00, 0x00, 0x00, 0x00},	/* 9, -4.5dB */
	{0x1f, 0x1e, 0x1a, 0x0f, 0x00, 0x00, 0x00, 0x00},	/* 10, -5.0dB */
	{0x1d, 0x1c, 0x18, 0x0e, 0x00, 0x00, 0x00, 0x00},	/* 11, -5.5dB */
	{0x1b, 0x1a, 0x17, 0x0e, 0x00, 0x00, 0x00, 0x00},	/* 12, -6.0dB  <== default */
	{0x1a, 0x19, 0x16, 0x0d, 0x00, 0x00, 0x00, 0x00},	/* 13, -6.5dB */
	{0x18, 0x17, 0x15, 0x0c, 0x00, 0x00, 0x00, 0x00},	/* 14, -7.0dB */
	{0x17, 0x16, 0x13, 0x0b, 0x00, 0x00, 0x00, 0x00},	/* 15, -7.5dB */
	{0x16, 0x15, 0x12, 0x0b, 0x00, 0x00, 0x00, 0x00},	/* 16, -8.0dB */
	{0x14, 0x14, 0x11, 0x0a, 0x00, 0x00, 0x00, 0x00},	/* 17, -8.5dB */
	{0x13, 0x13, 0x10, 0x0a, 0x00, 0x00, 0x00, 0x00},	/* 18, -9.0dB */
	{0x12, 0x12, 0x0f, 0x09, 0x00, 0x00, 0x00, 0x00},	/* 19, -9.5dB */
	{0x11, 0x11, 0x0f, 0x09, 0x00, 0x00, 0x00, 0x00},	/* 20, -10.0dB */
	{0x10, 0x10, 0x0e, 0x08, 0x00, 0x00, 0x00, 0x00},	/* 21, -10.5dB */
	{0x0f, 0x0f, 0x0d, 0x08, 0x00, 0x00, 0x00, 0x00},	/* 22, -11.0dB */
	{0x0e, 0x0e, 0x0c, 0x07, 0x00, 0x00, 0x00, 0x00},	/* 23, -11.5dB */
	{0x0d, 0x0d, 0x0c, 0x07, 0x00, 0x00, 0x00, 0x00},	/* 24, -12.0dB */
	{0x0d, 0x0c, 0x0b, 0x06, 0x00, 0x00, 0x00, 0x00},	/* 25, -12.5dB */
	{0x0c, 0x0c, 0x0a, 0x06, 0x00, 0x00, 0x00, 0x00},	/* 26, -13.0dB */
	{0x0b, 0x0b, 0x0a, 0x06, 0x00, 0x00, 0x00, 0x00},	/* 27, -13.5dB */
	{0x0b, 0x0a, 0x09, 0x05, 0x00, 0x00, 0x00, 0x00},	/* 28, -14.0dB */
	{0x0a, 0x0a, 0x09, 0x05, 0x00, 0x00, 0x00, 0x00},	/* 29, -14.5dB */
	{0x0a, 0x09, 0x08, 0x05, 0x00, 0x00, 0x00, 0x00},	/* 30, -15.0dB */
	{0x09, 0x09, 0x08, 0x05, 0x00, 0x00, 0x00, 0x00},	/* 31, -15.5dB */
	{0x09, 0x08, 0x07, 0x04, 0x00, 0x00, 0x00, 0x00}	/* 32, -16.0dB */
};

/*
 * 2011/09/21 MH Add to describe different team necessary resource allocate??
 */
 
static void odm_CmnInfoInit_Debug(struct rtl_dm *pDM_Odm)
{
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_COMMON, ODM_DBG_LOUD, ("odm_CmnInfoInit_Debug==>\n"));
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_COMMON, ODM_DBG_LOUD, ("SupportPlatform=%d\n", pDM_Odm->SupportPlatform));
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_COMMON, ODM_DBG_LOUD, ("SupportAbility=0x%x\n", pDM_Odm->SupportAbility));
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_COMMON, ODM_DBG_LOUD, ("SupportInterface=%d\n", pDM_Odm->SupportInterface));
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_COMMON, ODM_DBG_LOUD, ("SupportICType=0x%x\n", pDM_Odm->SupportICType));
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_COMMON, ODM_DBG_LOUD, ("CutVersion=%d\n", pDM_Odm->CutVersion));
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_COMMON, ODM_DBG_LOUD, ("FabVersion=%d\n", pDM_Odm->FabVersion));
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_COMMON, ODM_DBG_LOUD, ("RFType=%d\n", pDM_Odm->RFType));
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_COMMON, ODM_DBG_LOUD, ("BoardType=%d\n", pDM_Odm->BoardType));
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_COMMON, ODM_DBG_LOUD, ("ExtLNA=%d\n", pDM_Odm->ExtLNA));
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_COMMON, ODM_DBG_LOUD, ("ExtPA=%d\n", pDM_Odm->ExtPA));
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_COMMON, ODM_DBG_LOUD, ("ExtTRSW=%d\n", pDM_Odm->ExtTRSW));
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_COMMON, ODM_DBG_LOUD, ("PatchID=%d\n", pDM_Odm->PatchID));
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_COMMON, ODM_DBG_LOUD, ("bInHctTest=%d\n", pDM_Odm->bInHctTest));
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_COMMON, ODM_DBG_LOUD, ("bWIFITest=%d\n", pDM_Odm->bWIFITest));
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_COMMON, ODM_DBG_LOUD, ("bDualMacSmartConcurrent=%d\n", pDM_Odm->bDualMacSmartConcurrent));

}

static void odm_CommonInfoSelfInit(struct rtl_dm *pDM_Odm)
{
	pDM_Odm->bCckHighPower = (BOOLEAN) ODM_GetBBReg(pDM_Odm, ODM_REG(CCK_RPT_FORMAT, pDM_Odm), ODM_BIT(CCK_RPT_FORMAT, pDM_Odm));
	pDM_Odm->RFPathRxEnable = (u8) ODM_GetBBReg(pDM_Odm, ODM_REG(BB_RX_PATH, pDM_Odm), ODM_BIT(BB_RX_PATH, pDM_Odm));
#if (DM_ODM_SUPPORT_TYPE != ODM_CE)
	pDM_Odm->pbNet_closed = &pDM_Odm->BOOLEAN_temp;
#endif


	pDM_Odm->TxRate = 0xFF;
	ODM_InitDebugSetting(pDM_Odm);
}

static void odm_DIGInit(struct rtl_dm *pDM_Odm)
{
	pDIG_T	pDM_DigTable = &pDM_Odm->DM_DigTable;

	/* pDM_DigTable->Dig_Enable_Flag = TRUE; */
	/* pDM_DigTable->Dig_Ext_Port_Stage = DIG_EXT_PORT_STAGE_MAX; */
	pDM_DigTable->CurIGValue = (u8) ODM_GetBBReg(pDM_Odm, ODM_REG(IGI_A, pDM_Odm), ODM_BIT(IGI, pDM_Odm));
	/* pDM_DigTable->PreIGValue = 0x0; */
	/* pDM_DigTable->CurSTAConnectState = pDM_DigTable->PreSTAConnectState = DIG_STA_DISCONNECT; */
	/* pDM_DigTable->CurMultiSTAConnectState = DIG_MultiSTA_DISCONNECT; */
	pDM_DigTable->RssiLowThresh 	= DM_DIG_THRESH_LOW;
	pDM_DigTable->RssiHighThresh 	= DM_DIG_THRESH_HIGH;
	pDM_DigTable->FALowThresh	= DM_FALSEALARM_THRESH_LOW;
	pDM_DigTable->FAHighThresh	= DM_FALSEALARM_THRESH_HIGH;

	if (pDM_Odm->BoardType & (ODM_BOARD_EXT_PA|ODM_BOARD_EXT_LNA)) {
		pDM_DigTable->rx_gain_range_max = DM_DIG_MAX_NIC;
		pDM_DigTable->rx_gain_range_min = DM_DIG_MIN_NIC;
	} else {
		pDM_DigTable->rx_gain_range_max = DM_DIG_MAX_NIC;
		pDM_DigTable->rx_gain_range_min = DM_DIG_MIN_NIC;
	}
	pDM_DigTable->BackoffVal = DM_DIG_BACKOFF_DEFAULT;
	pDM_DigTable->BackoffVal_range_max = DM_DIG_BACKOFF_MAX;
	pDM_DigTable->BackoffVal_range_min = DM_DIG_BACKOFF_MIN;
	pDM_DigTable->PreCCK_CCAThres = 0xFF;
	pDM_DigTable->CurCCK_CCAThres = 0x83;
	pDM_DigTable->ForbiddenIGI = DM_DIG_MIN_NIC;
	pDM_DigTable->LargeFAHit = 0;
	pDM_DigTable->Recover_cnt = 0;
	pDM_DigTable->DIG_Dynamic_MIN_0 = DM_DIG_MIN_NIC;
	pDM_DigTable->DIG_Dynamic_MIN_1 = DM_DIG_MIN_NIC;
	pDM_DigTable->bMediaConnect_0 = FALSE;
	pDM_DigTable->bMediaConnect_1 = FALSE;

	/* To Initialize pDM_Odm->bDMInitialGainEnable == FALSE to avoid DIG error */
	pDM_Odm->bDMInitialGainEnable = TRUE;

	/* To Initi BT30 IGI */
	pDM_DigTable->BT30_CurIGI = 0x32;

}

static void odm_AdaptivityInit(struct rtl_dm *pDM_Odm)
{
	struct rtl_priv *pAdapter = pDM_Odm->Adapter;

	pDM_Odm->TH_H = 0xfa; 		/* -6dB */
	pDM_Odm->TH_L = 0xfd; 		/* -3dB */
	pDM_Odm->IGI_Base = 0x32;
	pDM_Odm->IGI_target = 0x1c;
	pDM_Odm->ForceEDCCA = 0;
	pDM_Odm->AdapEn_RSSI = 32;	/* 45; */
}

static void odm_RateAdaptiveMaskInit(struct rtl_dm *pDM_Odm)
{
	PODM_RATE_ADAPTIVE	pOdmRA = &pDM_Odm->RateAdaptive;

#if (DM_ODM_SUPPORT_TYPE == ODM_CE)
	pOdmRA->Type = DM_Type_ByDriver;
	if (pOdmRA->Type == DM_Type_ByDriver)
		pDM_Odm->bUseRAMask = _TRUE;
	else
		pDM_Odm->bUseRAMask = _FALSE;
#endif

	pOdmRA->RATRState = DM_RATR_STA_INIT;
	pOdmRA->LdpcThres = 35;
	pOdmRA->bUseLdpc = FALSE;
	pOdmRA->HighRSSIThresh = 50;
	pOdmRA->LowRSSIThresh = 20;
}

static void ODM_EdcaTurboInit(struct rtl_dm *pDM_Odm)
{

#if (DM_ODM_SUPPORT_TYPE == ODM_CE)
	struct rtl_priv *Adapter = pDM_Odm->Adapter;
	pDM_Odm->DM_EDCA_Table.bCurrentTurboEDCA = FALSE;
	pDM_Odm->DM_EDCA_Table.bIsCurRDLState = FALSE;
	Adapter->recvpriv.bIsAnyNonBEPkts = FALSE;

#endif
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_EDCA_TURBO, ODM_DBG_LOUD, ("Orginial VO PARAM: 0x%x\n", rtl_read_dword(pDM_Odm->Adapter, ODM_EDCA_VO_PARAM)));
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_EDCA_TURBO, ODM_DBG_LOUD, ("Orginial VI PARAM: 0x%x\n", rtl_read_dword(pDM_Odm->Adapter, ODM_EDCA_VI_PARAM)));
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_EDCA_TURBO, ODM_DBG_LOUD, ("Orginial BE PARAM: 0x%x\n", rtl_read_dword(pDM_Odm->Adapter, ODM_EDCA_BE_PARAM)));
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_EDCA_TURBO, ODM_DBG_LOUD, ("Orginial BK PARAM: 0x%x\n", rtl_read_dword(pDM_Odm->Adapter, ODM_EDCA_BK_PARAM)));


}

static u8 getSwingIndex(struct rtl_dm *pDM_Odm)
{
	struct rtl_priv *	Adapter = pDM_Odm->Adapter;
	 struct rtw_hal	*pHalData = GET_HAL_DATA(Adapter);
	u8 			i = 0;
	uint32_t 			bbSwing;
#if ((RTL8812A_SUPPORT == 1) || (RTL8821A_SUPPORT == 1))
	bbSwing = PHY_GetTxBBSwing_8812A(Adapter, pHalData->CurrentBandType, RF90_PATH_A);
#endif

	for (i = 0; i < TXSCALE_TABLE_SIZE; ++i)
		if (bbSwing == TxScalingTable_Jaguar[i])
			break;

	return i;
}



static void odm_TXPowerTrackingThermalMeterInit(struct rtl_dm *pDM_Odm)
{
	u8 		p = 0;
#if (DM_ODM_SUPPORT_TYPE == ODM_CE)
	struct rtl_priv *Adapter = pDM_Odm->Adapter;
	 struct rtw_hal	*pHalData = GET_HAL_DATA(Adapter);

#if ((RTL8812A_SUPPORT == 1) || (RTL8821A_SUPPORT == 1))
	pDM_Odm->RFCalibrateInfo.bTXPowerTracking = _TRUE;
	pDM_Odm->RFCalibrateInfo.TXPowercount = 0;
	pDM_Odm->RFCalibrateInfo.bTXPowerTrackingInit = _FALSE;
	/* #if	(MP_DRIVER != 1) */		/* for mp driver, turn off txpwrtracking as default */
	if (*(pDM_Odm->mp_mode) != 1)
		pDM_Odm->RFCalibrateInfo.TxPowerTrackControl = _TRUE;
	/* #endif//#if	(MP_DRIVER != 1) */
	MSG_8192C("pDM_Odm TxPowerTrackControl = %d\n", pDM_Odm->RFCalibrateInfo.TxPowerTrackControl);
#else
	{
		struct dm_priv	*pdmpriv = &pHalData->dmpriv;

		/* if (IS_HARDWARE_TYPE_8192C(pHalData)) */
		{
			pdmpriv->bTXPowerTracking = _TRUE;
			pdmpriv->TXPowercount = 0;
			pdmpriv->bTXPowerTrackingInit = _FALSE;
			/* #if	(MP_DRIVER != 1) */	/* for mp driver, turn off txpwrtracking as default */

			if (*(pDM_Odm->mp_mode) != 1)
				pdmpriv->TxPowerTrackControl = _TRUE;
			/* #endif//#if	(MP_DRIVER != 1) */
		}
		MSG_8192C("pdmpriv->TxPowerTrackControl = %d\n", pdmpriv->TxPowerTrackControl);

	}
#endif
#endif

	pDM_Odm->RFCalibrateInfo.TxPowerTrackControl = TRUE;
	pDM_Odm->RFCalibrateInfo.ThermalValue = pHalData->EEPROMThermalMeter;
	pDM_Odm->RFCalibrateInfo.ThermalValue_IQK = pHalData->EEPROMThermalMeter;
	pDM_Odm->RFCalibrateInfo.ThermalValue_LCK = pHalData->EEPROMThermalMeter;

	/* The index of "0 dB" in SwingTable. */
	{
		u8 defaultSwingIndex = getSwingIndex(pDM_Odm);


		pDM_Odm->DefaultOfdmIndex = (defaultSwingIndex == TXSCALE_TABLE_SIZE) ? 24 : defaultSwingIndex;
		pDM_Odm->DefaultCckIndex = 24;
	}

	pDM_Odm->BbSwingIdxCckBase = pDM_Odm->DefaultCckIndex;
	pDM_Odm->RFCalibrateInfo.CCK_index = pDM_Odm->DefaultCckIndex;

	for (p = RF90_PATH_A; p < MAX_RF_PATH; ++p) {
		pDM_Odm->BbSwingIdxOfdmBase[p] = pDM_Odm->DefaultOfdmIndex;
		pDM_Odm->RFCalibrateInfo.OFDM_index[p] = pDM_Odm->DefaultOfdmIndex;
		pDM_Odm->RFCalibrateInfo.DeltaPowerIndex[p] = 0;
		pDM_Odm->RFCalibrateInfo.DeltaPowerIndexLast[p] = 0;
		pDM_Odm->RFCalibrateInfo.PowerIndexOffset[p] = 0;
	}
}

static void odm_TXPowerTrackingInit(struct rtl_dm *pDM_Odm)
{
	odm_TXPowerTrackingThermalMeterInit(pDM_Odm);
}

void ODM_DMInit(struct rtl_dm *pDM_Odm)
{
	/* 2012.05.03 Luke: For all IC series */
	odm_CommonInfoSelfInit(pDM_Odm);
	odm_CmnInfoInit_Debug(pDM_Odm);
	odm_DIGInit(pDM_Odm);
	odm_AdaptivityInit(pDM_Odm);
	odm_RateAdaptiveMaskInit(pDM_Odm);

	ODM_EdcaTurboInit(pDM_Odm);

	odm_TXPowerTrackingInit(pDM_Odm);
}

/* From hal/OUTSRC/rtl8812a/HalPhyRf_8812A.c */

/*
 * ============================================================
 *  Tx Power Tracking
 * ============================================================
 */

static void DoIQK_8812A(struct rtl_dm *pDM_Odm, u8 DeltaThermalIndex,
	u8 	ThermalValue, u8 Threshold)
{
	struct rtl_priv *Adapter = pDM_Odm->Adapter;
	 struct rtw_hal	*pHalData = GET_HAL_DATA(Adapter);

	ODM_ResetIQKResult(pDM_Odm);

	pDM_Odm->RFCalibrateInfo.ThermalValue_IQK = ThermalValue;
	rtl8812au_phy_iq_calibrate(Adapter, FALSE);
}

/*-----------------------------------------------------------------------------
 * Function:	odm_TxPwrTrackSetPwr88E()
 *
 * Overview:	88E change all channel tx power accordign to flag.
 *				OFDM & CCK are all different.
 *
 * Input:		NONE
 *
 * Output:		NONE
 *
 * Return:		NONE
 *
 * Revised History:
 *	When		Who		Remark
 *	04/23/2012	MHC		Create Version 0.
 *
 *---------------------------------------------------------------------------*/

static void ODM_TxPwrTrackSetPwr8812A(struct rtl_dm *pDM_Odm, PWRTRACK_METHOD Method,
	u8 	RFPath, u8 	ChannelMappedIndex)
{
	uint32_t 	finalBbSwingIdx[2];

	struct rtl_priv *	Adapter = pDM_Odm->Adapter;
	struct rtw_hal *pHalData = GET_HAL_DATA(Adapter);

	u8 PwrTrackingLimit = 26; /* +1.0dB */
	u8 TxRate = 0xFF;
	s8 Final_OFDM_Swing_Index = 0;
	s8 Final_CCK_Swing_Index = 0;
	u8 i = 0;

	if (TxRate != 0xFF) {
		/* Ulli better with switch case, see in rtlwifi-lib */
		/* 2 CCK */
		if ((TxRate >= MGN_1M) && (TxRate <= MGN_11M))
			PwrTrackingLimit = 32;				/* +4dB */
		/* 2 OFDM */
		else if ((TxRate >= MGN_6M) && (TxRate <= MGN_48M))
			PwrTrackingLimit = 32;				/* +4dB */
		else if (TxRate == MGN_54M)
			PwrTrackingLimit = 30;				/* +3dB */

		/* ULLI 80211n */
		/* 2 HT */
		else if ((TxRate >= MGN_MCS0) && (TxRate <= MGN_MCS2))  /* QPSK/BPSK */
			PwrTrackingLimit = 34;				/* +5dB */
		else if ((TxRate >= MGN_MCS3) && (TxRate <= MGN_MCS4))  /* 16QAM */
			PwrTrackingLimit = 32;				/* +4dB */
		else if ((TxRate >= MGN_MCS5) && (TxRate <= MGN_MCS7))  /* 64QAM */
			PwrTrackingLimit = 30;				/* +3dB */

		else if ((TxRate >= MGN_MCS8) && (TxRate <= MGN_MCS10)) 	/* QPSK/BPSK */
			PwrTrackingLimit = 34; 				/* +5dB */
		else if ((TxRate >= MGN_MCS11) && (TxRate <= MGN_MCS12)) 	/* 16QAM */
			PwrTrackingLimit = 32; 				/* +4dB */
		else if ((TxRate >= MGN_MCS13) && (TxRate <= MGN_MCS15)) 	/* 64QAM */
			PwrTrackingLimit = 30; 				/* +3dB */

		/* Ulli 802.11ac */
		/* 2 VHT */
		else if ((TxRate >= MGN_VHT1SS_MCS0) && (TxRate <= MGN_VHT1SS_MCS2))    /* QPSK/BPSK */
			PwrTrackingLimit = 34;						/* +5dB */
		else if ((TxRate >= MGN_VHT1SS_MCS3) && (TxRate <= MGN_VHT1SS_MCS4))    /* 16QAM */
			PwrTrackingLimit = 32;						/* +4dB */
		else if ((TxRate >= MGN_VHT1SS_MCS5) && (TxRate <= MGN_VHT1SS_MCS6))    /* 64QAM */
			PwrTrackingLimit = 30;						/* +3dB */
		else if (TxRate == MGN_VHT1SS_MCS7)					/* 64QAM */
			PwrTrackingLimit = 28;						/* +2dB */
		else if (TxRate == MGN_VHT1SS_MCS8)					/* 256QAM */
			PwrTrackingLimit = 26;						/* +1dB */
		else if (TxRate == MGN_VHT1SS_MCS9)					/* 256QAM */
			PwrTrackingLimit = 24;						/* +0dB */

		else if ((TxRate >= MGN_VHT2SS_MCS0) && (TxRate <= MGN_VHT2SS_MCS2)) 	/* QPSK/BPSK */
			PwrTrackingLimit = 34; 						/* +5dB */
		else if ((TxRate >= MGN_VHT2SS_MCS3) && (TxRate <= MGN_VHT2SS_MCS4)) 	/* 16QAM */
			PwrTrackingLimit = 32; 						/* +4dB */
		else if ((TxRate >= MGN_VHT2SS_MCS5) && (TxRate <= MGN_VHT2SS_MCS6)) 	/* 64QAM */
			PwrTrackingLimit = 30; 						/* +3dB */
		else if (TxRate == MGN_VHT2SS_MCS7) 					/* 64QAM */
			PwrTrackingLimit = 28; 						/* +2dB */
		else if (TxRate == MGN_VHT2SS_MCS8) 					/* 256QAM */
			PwrTrackingLimit = 26; 						/* +1dB */
		else if (TxRate == MGN_VHT2SS_MCS9) 					/* 256QAM */
			PwrTrackingLimit = 24; 						/* +0dB */

		else
			PwrTrackingLimit = 24;
	}
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD, ("TxRate=0x%x, PwrTrackingLimit=%d\n", TxRate, PwrTrackingLimit));

	if (Method == BBSWING) {
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD, ("===>ODM_TxPwrTrackSetPwr8812A\n"));

		if (RFPath == RF90_PATH_A) {
			finalBbSwingIdx[RF90_PATH_A] = (pDM_Odm->RFCalibrateInfo.OFDM_index[RF90_PATH_A] > PwrTrackingLimit) ? PwrTrackingLimit : pDM_Odm->RFCalibrateInfo.OFDM_index[RF90_PATH_A];
			ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD, ("pDM_Odm->RFCalibrateInfo.OFDM_index[RF90_PATH_A]=%d, pDM_Odm->RealBbSwingIdx[RF90_PATH_A]=%d\n",
				pDM_Odm->RFCalibrateInfo.OFDM_index[RF90_PATH_A], finalBbSwingIdx[RF90_PATH_A]));

			ODM_SetBBReg(pDM_Odm, rA_TxScale_Jaguar, 0xFFE00000, TxScalingTable_Jaguar[finalBbSwingIdx[RF90_PATH_A]]);
		} else {
			finalBbSwingIdx[RF90_PATH_B] = (pDM_Odm->RFCalibrateInfo.OFDM_index[RF90_PATH_B] > PwrTrackingLimit) ? PwrTrackingLimit : pDM_Odm->RFCalibrateInfo.OFDM_index[RF90_PATH_B];
			ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD, ("pDM_Odm->RFCalibrateInfo.OFDM_index[RF90_PATH_B]=%d, pDM_Odm->RealBbSwingIdx[RF90_PATH_B]=%d\n",
				pDM_Odm->RFCalibrateInfo.OFDM_index[RF90_PATH_B], finalBbSwingIdx[RF90_PATH_B]));

			ODM_SetBBReg(pDM_Odm, rB_TxScale_Jaguar, 0xFFE00000, TxScalingTable_Jaguar[finalBbSwingIdx[RF90_PATH_B]]);
		}
	} else if (Method == MIX_MODE) {
			ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD, ("pDM_Odm->DefaultOfdmIndex=%d, pDM_Odm->Aboslute_OFDMSwingIdx[RFPath]=%d, RF_Path = %d\n",
				pDM_Odm->DefaultOfdmIndex, pDM_Odm->Aboslute_OFDMSwingIdx[RFPath], RFPath));


			Final_OFDM_Swing_Index = pDM_Odm->DefaultOfdmIndex + pDM_Odm->Aboslute_OFDMSwingIdx[RFPath];

			if (RFPath == RF90_PATH_A) {
				if (Final_OFDM_Swing_Index > PwrTrackingLimit) {    /* BBSwing higher then Limit */
					pDM_Odm->Remnant_CCKSwingIdx = Final_OFDM_Swing_Index - PwrTrackingLimit;            /*  CCK Follow the same compensate value as Path A */
					pDM_Odm->Remnant_OFDMSwingIdx[RFPath] = Final_OFDM_Swing_Index - PwrTrackingLimit;

					ODM_SetBBReg(pDM_Odm, rA_TxScale_Jaguar, 0xFFE00000, TxScalingTable_Jaguar[PwrTrackingLimit]);

					pDM_Odm->Modify_TxAGC_Flag_PathA = TRUE;

					/* et TxAGC Page C{}; */
					/* Adapter->HalFunc.SetTxPowerLevelHandler(Adapter, pHalData->CurrentChannel); */
					PHY_SetTxPowerLevel8812(Adapter, pHalData->CurrentChannel);

					ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD, ("******Path_A Over BBSwing Limit , PwrTrackingLimit = %d , Remnant TxAGC Value = %d \n", PwrTrackingLimit, pDM_Odm->Remnant_OFDMSwingIdx[RFPath]));
				} else if (Final_OFDM_Swing_Index < 0) {
					pDM_Odm->Remnant_CCKSwingIdx = Final_OFDM_Swing_Index;            /* CCK Follow the same compensate value as Path A */
					pDM_Odm->Remnant_OFDMSwingIdx[RFPath] = Final_OFDM_Swing_Index;

					ODM_SetBBReg(pDM_Odm, rA_TxScale_Jaguar, 0xFFE00000, TxScalingTable_Jaguar[0]);

					pDM_Odm->Modify_TxAGC_Flag_PathA = TRUE;

					/* Set TxAGC Page C{}; */
					/* Adapter->HalFunc.SetTxPowerLevelHandler(Adapter, pHalData->CurrentChannel);*/
					PHY_SetTxPowerLevel8812(Adapter, pHalData->CurrentChannel);

					ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD, ("******Path_A Lower then BBSwing lower bound  0 , Remnant TxAGC Value = %d \n", pDM_Odm->Remnant_OFDMSwingIdx[RFPath]));
				} else 	{
					ODM_SetBBReg(pDM_Odm, rA_TxScale_Jaguar, 0xFFE00000, TxScalingTable_Jaguar[Final_OFDM_Swing_Index]);

					ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD, ("******Path_A Compensate with BBSwing , Final_OFDM_Swing_Index = %d \n", Final_OFDM_Swing_Index));

					if (pDM_Odm->Modify_TxAGC_Flag_PathA) { /* If TxAGC has changed, reset TxAGC again */
						pDM_Odm->Remnant_CCKSwingIdx = 0;
						pDM_Odm->Remnant_OFDMSwingIdx[RFPath] = 0;

						/* Set TxAGC Page C{}; */
						/* Adapter->HalFunc.SetTxPowerLevelHandler(Adapter, pHalData->CurrentChannel); */
						PHY_SetTxPowerLevel8812(Adapter, pHalData->CurrentChannel);

						pDM_Odm->Modify_TxAGC_Flag_PathA = FALSE;

						ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD, ("******Path_A pDM_Odm->Modify_TxAGC_Flag = FALSE \n"));
					}
				}
			}

			if (RFPath == RF90_PATH_B) {
				if (Final_OFDM_Swing_Index > PwrTrackingLimit) {			/* BBSwing higher then Limit */
					pDM_Odm->Remnant_OFDMSwingIdx[RFPath] = Final_OFDM_Swing_Index - PwrTrackingLimit;

					ODM_SetBBReg(pDM_Odm, rB_TxScale_Jaguar, 0xFFE00000, TxScalingTable_Jaguar[PwrTrackingLimit]);

					pDM_Odm->Modify_TxAGC_Flag_PathB = TRUE;

					/* Set TxAGC Page E{}; */

					ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD, ("******Path_B Over BBSwing Limit , PwrTrackingLimit = %d , Remnant TxAGC Value = %d \n", PwrTrackingLimit, pDM_Odm->Remnant_OFDMSwingIdx[RFPath]));
				} else if (Final_OFDM_Swing_Index < 0) {
					pDM_Odm->Remnant_OFDMSwingIdx[RFPath] = Final_OFDM_Swing_Index;

					ODM_SetBBReg(pDM_Odm, rB_TxScale_Jaguar, 0xFFE00000, TxScalingTable_Jaguar[0]);

					pDM_Odm->Modify_TxAGC_Flag_PathB = TRUE;

					/* Set TxAGC Page E{}; */

					ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD, ("******Path_B Lower then BBSwing lower bound  0 , Remnant TxAGC Value = %d \n", pDM_Odm->Remnant_OFDMSwingIdx[RFPath]));
				} else {
					ODM_SetBBReg(pDM_Odm, rB_TxScale_Jaguar, 0xFFE00000, TxScalingTable_Jaguar[Final_OFDM_Swing_Index]);

					ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD, ("******Path_B Compensate with BBSwing , Final_OFDM_Swing_Index = %d \n", Final_OFDM_Swing_Index));

					if (pDM_Odm->Modify_TxAGC_Flag_PathB) {			/* If TxAGC has changed, reset TxAGC again */
						pDM_Odm->Remnant_CCKSwingIdx = 0;
						pDM_Odm->Remnant_OFDMSwingIdx[RFPath] = 0;

						/* Set TxAGC Page E{}; */

						pDM_Odm->Modify_TxAGC_Flag_PathB = FALSE;

						ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD, ("******Path_B pDM_Odm->Modify_TxAGC_Flag = FALSE \n"));
					}
				}
			}

	} else {
		return;
	}
}

/* START Copied from hal/OUTSRC/rtl8812a/HalHWImg8812A_RF.c */
/******************************************************************************
*                           TxPowerTrack_USB.TXT
******************************************************************************/

static u8 rtl8812ae_delta_swing_table_idx_5gb_n[][DELTA_SWINGIDX_SIZE] = {
	{0, 1, 1, 2, 2, 3, 4, 5, 6,  6,  7,  7,  8,  8,  9,  9,
	10, 10, 11, 11, 12, 12, 13, 13, 14, 14, 14, 14, 14, 14},
	{0, 1, 1, 2, 3, 3, 4, 5, 5,  6,  7,  7,  8,  8,  9,  9,
	10, 10, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11},
	{0, 1, 1, 2, 3, 3, 4, 5, 6,  7,  7,  8,  8,  9,  9, 10,
	11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11},
};

static u8 rtl8812ae_delta_swing_table_idx_5gb_p[][DELTA_SWINGIDX_SIZE] = {
	{0, 1, 1, 2, 2, 3, 3, 4, 5,  6,  7,  7,  8,  8,  9,  9, 
	10, 10, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11},
	{0, 1, 1, 2, 3, 3, 4, 5, 5,  6,  7,  7,  8,  8,  9,  9,
	10, 10, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11},
	{0, 1, 1, 2, 3, 3, 4, 5, 6,  7,  7,  8,  8,  9,  9, 10, 
	11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11},
};

static u8 rtl8812ae_delta_swing_table_idx_5ga_n[][DELTA_SWINGIDX_SIZE] = {
	{0, 1, 1, 2, 2, 3, 4, 5, 6,  6,  7,  7,  8,  8,  9,  9, 
	10, 10, 11, 11, 12, 12, 13, 13, 14, 15, 15, 15, 15, 15},
	{0, 1, 1, 2, 2, 3, 4, 5, 6,  6,  7,  7,  8,  8,  9,  9, 
	10, 10, 11, 11, 12, 12, 13, 13, 14, 15, 15, 15, 15, 15},
	{0, 1, 1, 2, 2, 3, 4, 5, 6,  6,  7,  7,  8,  8,  9,  9, 
	10, 10, 11, 11, 12, 12, 13, 13, 14, 15, 15, 15, 15, 15},
};

static u8 rtl8812ae_delta_swing_table_idx_5ga_p[][DELTA_SWINGIDX_SIZE] = {
	{0, 1, 1, 2, 2, 3, 4, 5, 6,  7,  7,  8,  8,  9, 10, 11, 
	11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11},
	{0, 1, 1, 2, 3, 3, 4, 5, 6,  7,  7,  8,  8,  9, 10, 11, 
	11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11},
	{0, 1, 1, 2, 3, 3, 4, 5, 6,  7,  7,  8,  8,  9, 10, 11, 
	11, 12, 12, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11},
};

static u8 rtl8812au_delta_swing_table_idx_24gb_n[] = {
	0, 1, 1, 2, 2, 2, 3, 3, 3,  4,  4,  5,  5,  5,  6,  6,  
	7,  7,  8,  8,  9,  9, 10, 10, 11, 11, 11, 11, 11, 11
};
	
static u8 rtl8812au_delta_swing_table_idx_24gb_p[] = {
	0, 1, 1, 2, 2, 2, 3, 3, 3,  4,  4,  4,  5,  5,  5,  6,  
	6,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7
};
	
static u8 rtl8812au_delta_swing_table_idx_24ga_n[] = {
	0, 1, 1, 2, 2, 2, 3, 3, 3,  4,  4,  4,  5,  5,  5,  6,  
	6,  6,  7,  7,  7,  8,  8,  9, 10, 10, 10, 10, 10, 10
};

static u8 rtl8812au_delta_swing_table_idx_24ga_p[] = {
	0, 1, 1, 2, 2, 2, 3, 3, 3,  4,  4,  4,  5,  5,  5,  6,  
	6,  6,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7
};

static u8 rtl8812au_delta_swing_table_idx_24gcckb_n[] = {
	0, 1, 1, 2, 2, 2, 3, 3, 3,  4,  4,  5,  5,  5,  6,  6,  
	7,  7,  8,  8,  9,  9, 10, 10, 11, 11, 11, 11, 11, 11
};

static u8 rtl8812au_delta_swing_table_idx_24gcckb_p[] = {
	0, 1, 1, 2, 2, 2, 3, 3, 3,  4,  4,  4,  5,  5,  5,  6,  
	6,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7
};

static u8 rtl8812au_delta_swing_table_idx_24gccka_n[] = {
	0, 1, 1, 2, 2, 2, 3, 3, 3,  4,  4,  4,  5,  5,  5,  6,  
	6,  6,  7,  7,  7,  8,  8,  9, 10, 10, 10, 10, 10, 10
};
	
static u8 rtl8812au_delta_swing_table_idx_24gccka_p[] = {
	0, 1, 1, 2, 2, 2, 3, 3, 3,  4,  4,  4,  5,  5,  5,  6,  
	6,  6,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7
};

/******************************************************************************
*                           TxPowerTrack_USB_RFE3.TXT
******************************************************************************/

static u8 rtl8812ae_delta_swing_table_idx_5gb_n_rfe3[][DELTA_SWINGIDX_SIZE] = {
	{0, 1, 1, 2, 3, 3, 4, 5, 5,  6,  7,  7,  8,  8,  9,  9, 
	10, 10, 11, 12, 13, 13, 14, 15, 16, 16, 17, 17, 18, 18},
	{0, 1, 1, 2, 2, 3, 3, 4, 4,  5,  6,  7,  8,  8,  9,  9, 
	10, 10, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11},
	{0, 1, 1, 2, 2, 3, 3, 4, 4,  5,  6,  6,  7,  7,  8,  9, 
	10, 10, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11},
};

static u8 rtl8812ae_delta_swing_table_idx_5gb_p_rfe3[][DELTA_SWINGIDX_SIZE] = {
	{0, 1, 1, 2, 3, 3, 4, 5, 6,  7,  7,  8,  9,  9, 10, 10, 
	11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11},
	{0, 1, 1, 2, 2, 3, 3, 4, 4,  5,  6,  7,  8,  8,  9,  9, 
	10, 10, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11},
	{0, 1, 1, 2, 2, 3, 3, 4, 4,  5,  6,  6,  7,  7,  8,  9, 
	10, 10, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11},
};

static u8 rtl8812ae_delta_swing_table_idx_5ga_n_rfe3[][DELTA_SWINGIDX_SIZE] = {
	{0, 1, 1, 2, 3, 3, 4, 5, 5,  6,  7,  7,  8,  8,  9,  9, 
	10, 10, 11, 11, 12, 13, 14, 15, 16, 16, 17, 17, 18, 18},
	{0, 1, 1, 2, 3, 3, 4, 4, 5,  6,  6,  7,  7,  8,  9,  9, 
	10, 10, 11, 11, 12, 12, 13, 13, 14, 15, 16, 16, 17, 17},
	{0, 1, 1, 2, 3, 3, 4, 4, 5,  6,  7,  7,  8,  8,  9,  9, 
	10, 10, 11, 12, 13, 13, 14, 14, 15, 15, 16, 17, 18, 18},
};

static u8 rtl8812ae_delta_swing_table_idx_5ga_p_rfe3[][DELTA_SWINGIDX_SIZE] = {
	{0, 1, 1, 2, 2, 3, 4, 5, 6,  7,  7,  8,  9,  9, 10, 10, 
	11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11},
	{0, 1, 2, 3, 4, 4, 5, 5, 6,  7,  7,  8,  9,  9, 10, 11, 
	11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11},
	{0, 1, 2, 3, 4, 4, 5, 5, 6,  7,  7,  8,  9,  9, 10, 11, 
	11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11},
};

static u8 rtl8812au_delta_swing_table_idx_24gb_n_rfe3[] = {
	0, 1, 1, 2, 2, 3, 3, 4, 4,  4,  5,  5,  6,  6,  6,  7,  
	7,  7,  8,  8,  9,  9, 10, 11, 12, 12, 13, 14, 15, 15
};

static u8 rtl8812au_delta_swing_table_idx_24gb_p_rfe3[] = {
	0, 1, 1, 2, 2, 2, 2, 3, 3,  3,  4,  4,  5,  5,  5,  6,  
	6,  7,  7,  8,  9, 10, 10, 10, 10, 11, 11, 11, 11, 11
};

static u8 rtl8812au_delta_swing_table_idx_24ga_n_rfe3[] = {
	0, 1, 1, 2, 2, 3, 4, 5, 6,  6,  6,  7,  7,  8,  8,  9, 
	10, 10, 11, 11, 12, 12, 13, 13, 13, 13, 14, 14, 15, 15
};

static u8 rtl8812au_delta_swing_table_idx_24ga_p_rfe3[] = {
	0, 0, 1, 1, 1, 2, 2, 3, 3,  4,  4,  4,  5,  5,  5,  6,  
	6,  7,  7,  8,  8,  9,  9, 10, 10, 11, 11, 11, 11, 11
};

static u8 rtl8812au_delta_swing_table_idx_24gcckb_n_rfe3[] = {
	0, 1, 1, 2, 2, 3, 3, 4, 4,  4,  5,  5,  6,  6,  6,  7,  
	7,  7,  8,  8,  9,  9, 10, 11, 12, 12, 13, 14, 15, 15
};

static u8 rtl8812au_delta_swing_table_idx_24gcckb_p_rfe3[] = {
	0, 1, 1, 2, 2, 2, 2, 3, 3,  3,  4,  4,  5,  5,  5,  6,  
	6,  7,  7,  8,  9, 10, 10, 10, 10, 11, 11, 11, 11, 11
};

static u8 rtl8812au_delta_swing_table_idx_24gccka_n_rfe3[] = {
	0, 1, 1, 2, 2, 3, 4, 5, 6,  6,  6,  7,  7,  8,  8,  9, 
	10, 10, 11, 11, 12, 12, 13, 13, 13, 13, 14, 14, 15, 15
};

static u8 rtl8812au_delta_swing_table_idx_24gccka_p_rfe3[] = {
	0, 0, 1, 1, 1, 2, 2, 3, 3,  4,  4,  4,  5,  5,  5,  6,  
	6,  7,  7,  8,  8,  9,  9, 10, 10, 11, 11, 11, 11, 11
};

/* END Copied from hal/OUTSRC/rtl8812a/HalHWImg8812A_RF.c */

static void rtl8812au_get_delta_swing_table(struct rtl_dm *pDM_Odm,
					    u8 **up_a, u8 **down_a,
					    u8 **up_b, u8 **down_b)
{
	struct rtl_priv *       Adapter = pDM_Odm->Adapter;
	PODM_RF_CAL_T  	pRFCalibrateInfo = &(pDM_Odm->RFCalibrateInfo);
	 struct rtw_hal  	*pHalData = GET_HAL_DATA(Adapter);
	u16	rate = *(pDM_Odm->pForcedDataRate);
	u8         	channel   		 = pHalData->CurrentChannel;

	if (pDM_Odm->RFEType == 3 && pDM_Odm->bIsMPChip) {
		if (1 <= channel && channel <= 14) {
			if (IS_CCK_RATE(rate)) {
				*up_a   = rtl8812au_delta_swing_table_idx_24gccka_p_rfe3;
				*down_a = rtl8812au_delta_swing_table_idx_24gccka_n_rfe3;
				*up_b   = rtl8812au_delta_swing_table_idx_24gcckb_p_rfe3;
				*down_b = rtl8812au_delta_swing_table_idx_24gcckb_n_rfe3;
			} else {
				*up_a   = rtl8812au_delta_swing_table_idx_24ga_p_rfe3;
				*down_a = rtl8812au_delta_swing_table_idx_24ga_n_rfe3;
				*up_b   = rtl8812au_delta_swing_table_idx_24gb_p_rfe3;
				*down_b = rtl8812au_delta_swing_table_idx_24gb_n_rfe3;
			}
		} else if (36 <= channel && channel <= 64) {
			*up_a   = rtl8812ae_delta_swing_table_idx_5ga_p_rfe3[0];
			*down_a = rtl8812ae_delta_swing_table_idx_5ga_n_rfe3[0];
			*up_b   = rtl8812ae_delta_swing_table_idx_5gb_p_rfe3[0];
			*down_b = rtl8812ae_delta_swing_table_idx_5gb_n_rfe3[0];
		} else if (100 <= channel && channel <= 140) {
			*up_a   = rtl8812ae_delta_swing_table_idx_5ga_p_rfe3[1];
			*down_a = rtl8812ae_delta_swing_table_idx_5ga_n_rfe3[1];
			*up_b   = rtl8812ae_delta_swing_table_idx_5gb_p_rfe3[1];
			*down_b = rtl8812ae_delta_swing_table_idx_5gb_n_rfe3[1];
		} else if (149 <= channel && channel <= 173) {
			*up_a   = rtl8812ae_delta_swing_table_idx_5ga_p_rfe3[2];
			*down_a = rtl8812ae_delta_swing_table_idx_5ga_n_rfe3[2];
			*up_b   = rtl8812ae_delta_swing_table_idx_5gb_p_rfe3[2];
			*down_b = rtl8812ae_delta_swing_table_idx_5gb_n_rfe3[2];
		} else {
			*up_a   = (u8 *)DeltaSwingTableIdx_2GA_P_8188E;
			*down_a = (u8 *)DeltaSwingTableIdx_2GA_N_8188E;
			*up_b   = (u8 *)DeltaSwingTableIdx_2GA_P_8188E;
			*down_b = (u8 *)DeltaSwingTableIdx_2GA_N_8188E;
		}
	} else {
		if (1 <= channel && channel <= 14) {
			if (IS_CCK_RATE(rate)) {
				*up_a   = rtl8812au_delta_swing_table_idx_24gccka_p;
				*down_a = rtl8812au_delta_swing_table_idx_24gccka_n;
				*up_b   = rtl8812au_delta_swing_table_idx_24gcckb_p;
				*down_b = rtl8812au_delta_swing_table_idx_24gcckb_n;
			} else {
				*up_a   = rtl8812au_delta_swing_table_idx_24ga_p;
				*down_a = rtl8812au_delta_swing_table_idx_24ga_n;
				*up_b   = rtl8812au_delta_swing_table_idx_24gb_p;
				*down_b = rtl8812au_delta_swing_table_idx_24gb_n;
			}
		} else if (36 <= channel && channel <= 64) {
			*up_a   = rtl8812ae_delta_swing_table_idx_5ga_p[0];
			*down_a = rtl8812ae_delta_swing_table_idx_5ga_n[0];
			*up_b   = rtl8812ae_delta_swing_table_idx_5gb_p[0];
			*down_b = rtl8812ae_delta_swing_table_idx_5gb_n[0];
		} else if (100 <= channel && channel <= 140) {
			*up_a   = rtl8812ae_delta_swing_table_idx_5ga_p[1];
			*down_a = rtl8812ae_delta_swing_table_idx_5ga_n[1];
			*up_b   = rtl8812ae_delta_swing_table_idx_5gb_p[1];
			*down_b = rtl8812ae_delta_swing_table_idx_5gb_n[1];
		} else if (149 <= channel && channel <= 173) {
			*up_a   = rtl8812ae_delta_swing_table_idx_5ga_p[2];
			*down_a = rtl8812ae_delta_swing_table_idx_5ga_n[2];
			*up_b   = rtl8812ae_delta_swing_table_idx_5gb_p[2];
			*down_b = rtl8812ae_delta_swing_table_idx_5gb_n[2];
		} else {
			*up_a   = (u8 *)DeltaSwingTableIdx_2GA_P_8188E;
			*down_a = (u8 *)DeltaSwingTableIdx_2GA_N_8188E;
			*up_b   = (u8 *)DeltaSwingTableIdx_2GA_P_8188E;
			*down_b = (u8 *)DeltaSwingTableIdx_2GA_N_8188E;
		}
	}

	return;
}

void ConfigureTxpowerTrack_8812A(PTXPWRTRACK_CFG pConfig)
{
	pConfig->SwingTableSize_CCK = TXSCALE_TABLE_SIZE;
	pConfig->SwingTableSize_OFDM = TXSCALE_TABLE_SIZE;
	pConfig->Threshold_IQK = IQK_THRESHOLD;
	pConfig->AverageThermalNum = AVG_THERMAL_NUM_8812A;
	pConfig->RfPathCount = MAX_PATH_NUM_8812A;
	pConfig->ThermalRegAddr = RF_T_METER_8812A;

	pConfig->ODM_TxPwrTrackSetPwr = ODM_TxPwrTrackSetPwr8812A;
	pConfig->DoIQK = DoIQK_8812A;
	pConfig->PHY_LCCalibrate = PHY_LCCalibrate_8812A;
	pConfig->GetDeltaSwingTable = rtl8812au_get_delta_swing_table;
}

/* From hal/OUTSRC/rtl8821a/HalPhyRf_8821A.c */

static void DoIQK_8821A(struct rtl_dm *pDM_Odm, u8 DeltaThermalIndex,
	u8 ThermalValue, u8 Threshold)
{
	struct rtl_priv *Adapter = pDM_Odm->Adapter;
	 struct rtw_hal *pHalData = GET_HAL_DATA(Adapter);

	ODM_ResetIQKResult(pDM_Odm);

	pDM_Odm->RFCalibrateInfo.ThermalValue_IQK = ThermalValue;
	rtl8821au_phy_iq_calibrate(Adapter, FALSE);
}


static void ODM_TxPwrTrackSetPwr8821A(struct rtl_dm *pDM_Odm, PWRTRACK_METHOD Method,
	u8 RFPath, u8 ChannelMappedIndex)
{
	struct rtl_priv *Adapter = pDM_Odm->Adapter;
	struct rtw_hal *pHalData = GET_HAL_DATA(Adapter);

	u8 PwrTrackingLimit = 26; /* +1.0dB */
	u8 TxRate = 0xFF;
	s8 Final_OFDM_Swing_Index = 0;
	s8 Final_CCK_Swing_Index = 0;
	u8 i = 0;
	uint32_t finalBbSwingIdx[1];

	ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD, ("===>ODM_TxPwrTrackSetPwr8821A\n"));

	if (TxRate != 0xFF) {
		/* 2 CCK */
		if ((TxRate >= MGN_1M) && (TxRate <= MGN_11M))
			PwrTrackingLimit = 32;				/* +4dB */
		/* 2 OFDM */
		else if ((TxRate >= MGN_6M) && (TxRate <= MGN_48M))
			PwrTrackingLimit = 32;				/* +4dB */
		else if (TxRate == MGN_54M)
			PwrTrackingLimit = 30;				/* +3dB */
		/* 2 HT */
		else if ((TxRate >= MGN_MCS0) && (TxRate <= MGN_MCS2))  /* QPSK/BPSK */
			PwrTrackingLimit = 34;				/* +5dB */
		else if ((TxRate >= MGN_MCS3) && (TxRate <= MGN_MCS4))  /* 16QAM */
			PwrTrackingLimit = 32;				/* +4dB */
		else if ((TxRate >= MGN_MCS5) && (TxRate <= MGN_MCS7))  /* 64QAM */
			PwrTrackingLimit = 30;				/* +3dB */
		/* 2 VHT */
		else if ((TxRate >= MGN_VHT1SS_MCS0) && (TxRate <= MGN_VHT1SS_MCS2))    /* QPSK/BPSK */
			PwrTrackingLimit = 34;						/* +5dB */
		else if ((TxRate >= MGN_VHT1SS_MCS3) && (TxRate <= MGN_VHT1SS_MCS4))    /* 16QAM */
			PwrTrackingLimit = 32;						/* +4dB */
		else if ((TxRate >= MGN_VHT1SS_MCS5) && (TxRate <= MGN_VHT1SS_MCS6))    /* 64QAM */
			PwrTrackingLimit = 30;						/* +3dB */
		else if (TxRate == MGN_VHT1SS_MCS7)					/* 64QAM */
			PwrTrackingLimit = 28;						/* +2dB */
		else if (TxRate == MGN_VHT1SS_MCS8)					/* 256QAM */
			PwrTrackingLimit = 26;						/* +1dB */
		else if (TxRate == MGN_VHT1SS_MCS9)					/* 256QAM */
			PwrTrackingLimit = 24;						/* +0dB */
		else
			PwrTrackingLimit = 24;
	}
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD, ("TxRate=0x%x, PwrTrackingLimit=%d\n", TxRate, PwrTrackingLimit));

	if (Method == BBSWING) {
		if (RFPath == RF90_PATH_A) {
			finalBbSwingIdx[RF90_PATH_A] = (pDM_Odm->RFCalibrateInfo.OFDM_index[RF90_PATH_A] > PwrTrackingLimit) ? PwrTrackingLimit : pDM_Odm->RFCalibrateInfo.OFDM_index[RF90_PATH_A];
			ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD, ("pDM_Odm->RFCalibrateInfo.OFDM_index[RF90_PATH_A]=%d, pDM_Odm->RealBbSwingIdx[RF90_PATH_A]=%d\n",
				pDM_Odm->RFCalibrateInfo.OFDM_index[RF90_PATH_A], finalBbSwingIdx[RF90_PATH_A]));

			ODM_SetBBReg(pDM_Odm, rA_TxScale_Jaguar, 0xFFE00000, TxScalingTable_Jaguar[finalBbSwingIdx[RF90_PATH_A]]);
		}
	} else if (Method == MIX_MODE) {
			ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD, ("pDM_Odm->DefaultOfdmIndex=%d, pDM_Odm->Aboslute_OFDMSwingIdx[RFPath]=%d, RF_Path = %d\n",
				pDM_Odm->DefaultOfdmIndex, pDM_Odm->Aboslute_OFDMSwingIdx[RFPath], RFPath));


			Final_OFDM_Swing_Index = pDM_Odm->DefaultOfdmIndex + pDM_Odm->Aboslute_OFDMSwingIdx[RFPath];

			if (RFPath == RF90_PATH_A) {
				if (Final_OFDM_Swing_Index > PwrTrackingLimit) {
					/* BBSwing higher then Limit */
					pDM_Odm->Remnant_CCKSwingIdx = Final_OFDM_Swing_Index - PwrTrackingLimit;            /* CCK Follow the same compensate value as Path A */
					pDM_Odm->Remnant_OFDMSwingIdx[RFPath] = Final_OFDM_Swing_Index - PwrTrackingLimit;

					ODM_SetBBReg(pDM_Odm, rA_TxScale_Jaguar, 0xFFE00000, TxScalingTable_Jaguar[PwrTrackingLimit]);

					pDM_Odm->Modify_TxAGC_Flag_PathA = TRUE;

					/* Set TxAGC Page C{}; */
					/* Adapter->HalFunc.SetTxPowerLevelHandler(Adapter, pHalData->CurrentChannel); */
					PHY_SetTxPowerLevel8812(Adapter, pHalData->CurrentChannel);

					ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD, ("******Path_A Over BBSwing Limit , PwrTrackingLimit = %d , Remnant TxAGC Value = %d \n", PwrTrackingLimit, pDM_Odm->Remnant_OFDMSwingIdx[RFPath]));
				} else if (Final_OFDM_Swing_Index < 0) {
					pDM_Odm->Remnant_CCKSwingIdx = Final_OFDM_Swing_Index;            /* CCK Follow the same compensate value as Path A */
					pDM_Odm->Remnant_OFDMSwingIdx[RFPath] = Final_OFDM_Swing_Index;

					ODM_SetBBReg(pDM_Odm, rA_TxScale_Jaguar, 0xFFE00000, TxScalingTable_Jaguar[0]);

					pDM_Odm->Modify_TxAGC_Flag_PathA = TRUE;

					/* Set TxAGC Page C{}; */
					/* Adapter->HalFunc.SetTxPowerLevelHandler(Adapter, pHalData->CurrentChannel); */
					PHY_SetTxPowerLevel8812(Adapter, pHalData->CurrentChannel);

					ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD, ("******Path_A Lower then BBSwing lower bound  0 , Remnant TxAGC Value = %d \n", pDM_Odm->Remnant_OFDMSwingIdx[RFPath]));
				} else {
					ODM_SetBBReg(pDM_Odm, rA_TxScale_Jaguar, 0xFFE00000, TxScalingTable_Jaguar[Final_OFDM_Swing_Index]);

					ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD, ("******Path_A Compensate with BBSwing , Final_OFDM_Swing_Index = %d \n", Final_OFDM_Swing_Index));

					if (pDM_Odm->Modify_TxAGC_Flag_PathA) {
						/* If TxAGC has changed, reset TxAGC again */
						pDM_Odm->Remnant_CCKSwingIdx = 0;
						pDM_Odm->Remnant_OFDMSwingIdx[RFPath] = 0;

						/* Set TxAGC Page C{}; */
						/* Adapter->HalFunc.SetTxPowerLevelHandler(Adapter, pHalData->CurrentChannel); */
						PHY_SetTxPowerLevel8812(Adapter, pHalData->CurrentChannel);

						pDM_Odm->Modify_TxAGC_Flag_PathA = FALSE;

						ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD, ("******Path_A pDM_Odm->Modify_TxAGC_Flag = FALSE \n"));
					}
				}
			}

	} else {
		return;
	}
}
/* START Copied from hal/OUTSRC/rtl8821a/HalHWImg8821A_RF.c */

/******************************************************************************
*                           TxPowerTrack_USB.TXT
******************************************************************************/

static u8 rtl8821au_delta_swing_table_idx_5gb_n[][DELTA_SWINGIDX_SIZE] = {
	{0, 1, 1, 2, 2, 3, 4, 5, 6,  6,  7,  7,  8,  8,  9,  9, 
	10, 10, 11, 11, 12, 12, 13, 13, 14, 14, 14, 14, 14, 14},
	{0, 1, 1, 2, 3, 3, 4, 5, 5,  6,  7,  7,  8,  8,  9,  9, 
	10, 10, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11},
	{0, 1, 1, 2, 3, 3, 4, 5, 6,  7,  7,  8,  8,  9,  9, 10, 
	11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11},
};

static u8 rtl8821au_delta_swing_table_idx_5gb_p[][DELTA_SWINGIDX_SIZE] = {
	{0, 1, 1, 2, 2, 3, 3, 4, 5,  6,  7,  7,  8,  8,  9,  9, 
	10, 10, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11},
	{0, 1, 1, 2, 3, 3, 4, 5, 5,  6,  7,  7,  8,  8,  9,  9, 
	10, 10, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11},
	{0, 1, 1, 2, 3, 3, 4, 5, 6,  7,  7,  8,  8,  9,  9, 10, 
	11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11},
};

static u8 rtl8821au_delta_swing_table_idx_5ga_n[][DELTA_SWINGIDX_SIZE] = {
	{0, 0, 0, 1, 2, 2, 3, 4, 5, 6,  6,  7,  7,  8,  8,  9,  
	9, 10, 10, 11, 11, 12, 12, 13, 13, 14, 15, 15, 15, 15},
	{0, 0, 0, 1, 2, 2, 3, 4, 5, 6,  6,  7,  7,  8,  8,  9,  
	9, 10, 10, 11, 11, 12, 12, 13, 13, 14, 15, 15, 15, 15},
	{0, 0, 0, 1, 2, 2, 3, 4, 5, 6,  6,  7,  7,  8,  8,  9,  
	9, 10, 10, 11, 11, 12, 12, 13, 13, 14, 15, 15, 15, 15},
};

static u8 rtl8821au_delta_swing_table_idx_5ga_p[][DELTA_SWINGIDX_SIZE] = {
	{1, 2, 3, 4, 5, 6, 7, 8, 9,  10, 11, 12,  13,  14, 15, 16, 
	16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16},
	{1, 2, 3, 4, 5, 6, 7, 8, 9,  10, 11, 12,  13,  14, 15, 16, 
	16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16},
	{1, 2, 3, 4, 5, 6, 7, 8, 9,  10, 11, 12,  13,  14, 15, 16, 
	16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16},
};

static u8 rtl8821au_delta_swing_table_idx_24gb_n[]    = {
	0, 1, 1, 2, 2, 2, 3, 3, 3,  4,  4,  5,  5,  5,  6,  6,  
	7,  7,  8,  8,  9,  9, 10, 10, 11, 11, 11, 11, 11, 11
};

static u8 rtl8821au_delta_swing_table_idx_24gb_p[]    = {
	0, 1, 1, 2, 2, 2, 3, 3, 3,  4,  4,  4,  5,  5,  5,  6,  
	6,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7
};

static u8 rtl8821au_delta_swing_table_idx_24ga_n[]    = {
	0, 1, 1, 2, 2, 2, 3, 3, 3,  4,  4,  4,  5,  5,  5,  6,  
	6,  6,  7,  7,  7,  8,  8,  9, 10, 10, 10, 10, 10, 10
};

static u8 rtl8821au_delta_swing_table_idx_24ga_p[]    = {
	0, 1, 1, 2, 2, 2, 3, 3, 3,  4,  4,  4,  5,  5,  5,  6,  
	6,  6,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7
};

static u8 rtl8821au_delta_swing_table_idx_24gcckb_n[] = {
	0, 1, 1, 2, 2, 2, 3, 3, 3,  4,  4,  5,  5,  5,  6,  6,  
	7,  7,  8,  8,  9,  9, 10, 10, 11, 11, 11, 11, 11, 11
};

static u8 rtl8821au_delta_swing_table_idx_24gcckb_p[] = {
	0, 1, 1, 2, 2, 2, 3, 3, 3,  4,  4,  4,  5,  5,  5,  6,  
	6,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7
};

static u8 rtl8821au_delta_swing_table_idx_24gccka_n[] = {
	0, 1, 1, 2, 2, 2, 3, 3, 3,  4,  4,  4,  5,  5,  5,  6,  
	6,  6,  7,  7,  7,  8,  8,  9, 10, 10, 10, 10, 10, 10
};

static u8 rtl8821au_delta_swing_table_idx_24gccka_p[] = {
	0, 1, 1, 2, 2, 2, 3, 3, 3,  4,  4,  4,  5,  5,  5,  6, 
	6,  6,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7
};


/* END Copied from hal/OUTSRC/rtl8821a/HalHWImg8821A_RF.c */

static void rtl8821au_get_delta_swing_table(struct rtl_dm *pDM_Odm,
	u8 **up_a, u8 **down_a,
	u8 **up_b, u8 **down_b)
{
	struct rtl_priv *       Adapter = pDM_Odm->Adapter;
	PODM_RF_CAL_T  	pRFCalibrateInfo = &(pDM_Odm->RFCalibrateInfo);
	 struct rtw_hal  	*pHalData = GET_HAL_DATA(Adapter);
	/* u16     rate = pMgntInfo->ForcedDataRate; */
	u16	rate = 0;
	u8         	channel   		 = pHalData->CurrentChannel;

	if (1 <= channel && channel <= 14) {
		if (IS_CCK_RATE(rate)) {
			*up_a   = rtl8821au_delta_swing_table_idx_24gccka_p;
			*down_a = rtl8821au_delta_swing_table_idx_24gccka_n;
			*up_b   = rtl8821au_delta_swing_table_idx_24gcckb_p;
			*down_b = rtl8821au_delta_swing_table_idx_24gcckb_n;
		} else {
			*up_a   = rtl8821au_delta_swing_table_idx_24ga_p;
			*down_a = rtl8821au_delta_swing_table_idx_24ga_n;
			*up_b   = rtl8821au_delta_swing_table_idx_24gb_p;
			*down_b = rtl8821au_delta_swing_table_idx_24gb_n;
		}
	} else if (36 <= channel && channel <= 64) {
		*up_a   = rtl8821au_delta_swing_table_idx_5ga_p[0];
		*down_a = rtl8821au_delta_swing_table_idx_5ga_n[0];
		*up_b   = rtl8821au_delta_swing_table_idx_5gb_p[0];
		*down_b = rtl8821au_delta_swing_table_idx_5gb_n[0];
	} else if (100 <= channel && channel <= 140) {
		*up_a   = rtl8821au_delta_swing_table_idx_5ga_p[1];
		*down_a = rtl8821au_delta_swing_table_idx_5ga_n[1];
		*up_b   = rtl8821au_delta_swing_table_idx_5gb_p[1];
		*down_b = rtl8821au_delta_swing_table_idx_5gb_n[1];
	} else if (149 <= channel && channel <= 173) {
		*up_a   = rtl8821au_delta_swing_table_idx_5ga_p[2];
		*down_a = rtl8821au_delta_swing_table_idx_5ga_n[2];
		*up_b   = rtl8821au_delta_swing_table_idx_5gb_p[2];
		*down_b = rtl8821au_delta_swing_table_idx_5gb_n[2];
	} else {
		*up_a   = (u8 *)DeltaSwingTableIdx_2GA_P_8188E;
		*down_a = (u8 *)DeltaSwingTableIdx_2GA_N_8188E;
		*up_b   = (u8 *)DeltaSwingTableIdx_2GA_P_8188E;
		*down_b = (u8 *)DeltaSwingTableIdx_2GA_N_8188E;
	}

	return;
}

static void PHY_LCCalibrate_8821A(struct rtl_dm *pDM_Odm)
{
	PHY_LCCalibrate_8812A(pDM_Odm);
}

void ConfigureTxpowerTrack_8821A(PTXPWRTRACK_CFG pConfig)
{
	pConfig->SwingTableSize_CCK = TXSCALE_TABLE_SIZE;
	pConfig->SwingTableSize_OFDM = TXSCALE_TABLE_SIZE;
	pConfig->Threshold_IQK = IQK_THRESHOLD;
	pConfig->AverageThermalNum = AVG_THERMAL_NUM_8812A;
	pConfig->RfPathCount = MAX_PATH_NUM_8821A;
	pConfig->ThermalRegAddr = RF_T_METER_8812A;

	pConfig->ODM_TxPwrTrackSetPwr = ODM_TxPwrTrackSetPwr8821A;
	pConfig->DoIQK = DoIQK_8821A;
	pConfig->PHY_LCCalibrate = PHY_LCCalibrate_8821A;
	pConfig->GetDeltaSwingTable = rtl8821au_get_delta_swing_table;
}
