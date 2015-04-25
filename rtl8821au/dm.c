#include "dm.h"
#include "phy.h"
#include <odm_precomp.h>


#undef ODM_RT_TRACE
#define ODM_RT_TRACE(x, ...)	do {} while (0);

static void rtl8821au_dm_dig(struct rtl_priv *rtlpriuv);

static uint32_t edca_setting_UL[HT_IOT_PEER_MAX] =
/* UNKNOWN		REALTEK_90	REALTEK_92SE	BROADCOM		RALINK		ATHEROS		CISCO		MERU        MARVELL	92U_AP		SELF_AP(DownLink/Tx) */
{ 0x5e4322, 		0xa44f, 		0x5e4322,		0x5ea32b,  		0x5ea422, 	0x5ea322,	0x3ea430,	0x5ea42b, 0x5ea44f,	0x5e4322,	0x5e4322};


static uint32_t edca_setting_DL[HT_IOT_PEER_MAX] =
/* UNKNOWN		REALTEK_90	REALTEK_92SE	BROADCOM		RALINK		ATHEROS		CISCO		MERU,       MARVELL	92U_AP		SELF_AP(UpLink/Rx) */
{ 0xa44f, 		0x5ea44f, 	0x5e4322, 		0x5ea42b, 		0xa44f, 		0xa630, 		0x5ea630,	0x5ea42b, 0xa44f,		0xa42b,		0xa42b};

static uint32_t edca_setting_DL_GMode[HT_IOT_PEER_MAX] =
/* UNKNOWN		REALTEK_90	REALTEK_92SE	BROADCOM		RALINK		ATHEROS		CISCO		MERU,       MARVELL	92U_AP		SELF_AP */
{ 0x4322, 		0xa44f, 		0x5e4322,		0xa42b, 			0x5e4322, 	0x4322, 		0xa42b,		0x5ea42b, 0xa44f,		0x5e4322,	0x5ea42b};



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

static void rtl8821au_cm_common_info_self_update(struct _rtw_dm *pDM_Odm)
{
	pDM_Odm->bCckHighPower = (BOOLEAN) rtl_get_bbreg(pDM_Odm->rtlpriv, ODM_REG(CCK_RPT_FORMAT, pDM_Odm), ODM_BIT(CCK_RPT_FORMAT, pDM_Odm));
	pDM_Odm->RFPathRxEnable = (u8) rtl_get_bbreg(pDM_Odm->rtlpriv, ODM_REG(BB_RX_PATH, pDM_Odm), ODM_BIT(BB_RX_PATH, pDM_Odm));
	pDM_Odm->pbNet_closed = &pDM_Odm->BOOLEAN_temp;


	pDM_Odm->TxRate = 0xFF;
	ODM_InitDebugSetting(pDM_Odm);
}

/* Ulli : check function in rtlwifi/core.c for _rtl_dm_diginit() */

static void _rtl_dm_diginit(struct _rtw_dm *pDM_Odm)
{
	struct rtl_priv *rtlpriv = pDM_Odm->rtlpriv;

	struct rtl_hal	*rtlhal = rtl_hal(rtlpriv);
	struct dig_t *pDM_DigTable = &(rtlpriv->dm_digtable);

	/* pDM_DigTable->Dig_Enable_Flag = TRUE; */
	/* pDM_DigTable->Dig_Ext_Port_Stage = DIG_EXT_PORT_STAGE_MAX; */
	pDM_DigTable->cur_igvalue = (u8) rtl_get_bbreg(pDM_Odm->rtlpriv, ODM_REG(IGI_A, pDM_Odm), ODM_BIT(IGI, pDM_Odm));
	/* pDM_DigTable->PreIGValue = 0x0; */
	/* pDM_DigTable->CurSTAConnectState = pDM_DigTable->PreSTAConnectState = DIG_STA_DISCONNECT; */
	/* pDM_DigTable->CurMultiSTAConnectState = DIG_MultiSTA_DISCONNECT; */
	pDM_DigTable->rssi_lowthresh 	= DM_DIG_THRESH_LOW;
	pDM_DigTable->rssi_highthresh	= DM_DIG_THRESH_HIGH;
	pDM_DigTable->fa_lowthresh	= DM_FALSEALARM_THRESH_LOW;
	pDM_DigTable->fa_highthresh	= DM_FALSEALARM_THRESH_HIGH;

	if (rtlhal->board_type & (ODM_BOARD_EXT_PA|ODM_BOARD_EXT_LNA)) {
		pDM_DigTable->rx_gain_max = DM_DIG_MAX_NIC;
		pDM_DigTable->rx_gain_min = DM_DIG_MIN_NIC;
	} else {
		pDM_DigTable->rx_gain_max = DM_DIG_MAX_NIC;
		pDM_DigTable->rx_gain_min = DM_DIG_MIN_NIC;
	}
	pDM_DigTable->back_val = DM_DIG_BACKOFF_DEFAULT;
	pDM_DigTable->back_range_max = DM_DIG_BACKOFF_MAX;
	pDM_DigTable->back_range_min = DM_DIG_BACKOFF_MIN;
	pDM_DigTable->pre_cck_cca_thres = 0xFF;
	pDM_DigTable->cur_cck_cca_thres = 0x83;
	pDM_DigTable->forbidden_igi= DM_DIG_MIN_NIC;
	pDM_DigTable->large_fa_hit = 0;
	pDM_DigTable->recover_cnt = 0;
	pDM_DigTable->dig_min_0 = DM_DIG_MIN_NIC;
	pDM_DigTable->dig_min_1 = DM_DIG_MIN_NIC;
	pDM_DigTable->media_connect_0 = FALSE;
	pDM_DigTable->media_connect_1 = FALSE;

	/* To Initialize pDM_Odm->bDMInitialGainEnable == FALSE to avoid DIG error */
	pDM_Odm->bDMInitialGainEnable = TRUE;

	/* To Initi BT30 IGI */
	pDM_DigTable->bt30_cur_igi = 0x32;

}

static void odm_AdaptivityInit(struct rtl_priv *rtlpriv)
{
	struct _rtw_hal	*pHalData = GET_HAL_DATA(rtlpriv);
	struct _rtw_dm *pDM_Odm = &pHalData->odmpriv;	

	pDM_Odm->TH_H = 0xfa; 		/* -6dB */
	pDM_Odm->TH_L = 0xfd; 		/* -3dB */
	pDM_Odm->IGI_Base = 0x32;
	pDM_Odm->IGI_target = 0x1c;
	pDM_Odm->ForceEDCCA = 0;
	pDM_Odm->AdapEn_RSSI = 32;	/* 45; */
}

static void rtl8821au_dm_init_rate_adaptive_mask(struct rtl_priv *rtlpriv)
{
	struct _rtw_hal	*pHalData = GET_HAL_DATA(rtlpriv);
	struct _rtw_dm *pDM_Odm = &pHalData->odmpriv;	
	struct rate_adaptive *p_ra = &(rtlpriv->ra);
	PODM_RATE_ADAPTIVE	pOdmRA = &pDM_Odm->RateAdaptive;

	pOdmRA->Type = DM_Type_ByDriver;
	if (pOdmRA->Type == DM_Type_ByDriver)
		pDM_Odm->bUseRAMask = _TRUE;
	else
		pDM_Odm->bUseRAMask = _FALSE;

	pOdmRA->RATRState = DM_RATR_STA_INIT;
	pOdmRA->LdpcThres = 35;
	pOdmRA->bUseLdpc = FALSE;
	p_ra->high_rssi_thresh_for_ra = 50;
	pOdmRA->LowRSSIThresh = 20;
}

static void rtl8821au_dm_init_edca_turbo(struct rtl_priv *rtlpriv)
{
	rtlpriv->dm.current_turbo_edca = false;
	rtlpriv->dm.is_cur_rdlstate = false;
	rtlpriv->dm.is_any_nonbepkts = false;

	ODM_RT_TRACE(pDM_Odm, ODM_COMP_EDCA_TURBO, ODM_DBG_LOUD, ("Orginial VO PARAM: 0x%x\n", rtl_read_dword(pDM_Odm->rtlpriv, ODM_EDCA_VO_PARAM)));
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_EDCA_TURBO, ODM_DBG_LOUD, ("Orginial VI PARAM: 0x%x\n", rtl_read_dword(pDM_Odm->rtlpriv, ODM_EDCA_VI_PARAM)));
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_EDCA_TURBO, ODM_DBG_LOUD, ("Orginial BE PARAM: 0x%x\n", rtl_read_dword(pDM_Odm->rtlpriv, ODM_EDCA_BE_PARAM)));
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_EDCA_TURBO, ODM_DBG_LOUD, ("Orginial BK PARAM: 0x%x\n", rtl_read_dword(pDM_Odm->rtlpriv, ODM_EDCA_BK_PARAM)));


}

static u8 getSwingIndex(struct _rtw_dm *pDM_Odm)
{
	struct rtl_priv *	rtlpriv = pDM_Odm->rtlpriv;
	u8 			i = 0;
	uint32_t 			bbSwing;

	bbSwing = phy_get_tx_swing_8821au(rtlpriv, rtlpriv->rtlhal.current_bandtype, RF90_PATH_A);

	for (i = 0; i < TXSCALE_TABLE_SIZE; ++i)
		if (bbSwing == TxScalingTable_Jaguar[i])
			break;

	return i;
}


static void rtl8821au_dm_initialize_txpower_tracking_thermalmeter(struct _rtw_dm *pDM_Odm)
{
	u8 		p = 0;
	struct rtl_dm	*rtldm = rtl_dm(pDM_Odm->rtlpriv);
	struct rtl_priv *rtlpriv = pDM_Odm->rtlpriv;
	struct rtl_efuse *efuse = rtl_efuse(rtlpriv);

	rtldm->txpower_tracking = _TRUE;
	rtldm->txpowercount = 0;
	rtldm->txpower_trackinginit = _FALSE;
	/* #if	(MP_DRIVER != 1) */		/* for mp driver, turn off txpwrtracking as default */
	if (*(pDM_Odm->mp_mode) != 1)
		rtldm->txpower_track_control = _TRUE;
	/* #endif//#if	(MP_DRIVER != 1) */
	MSG_8192C("pDM_Odm TxPowerTrackControl = %d\n", rtldm->txpower_track_control);

	rtldm->txpower_track_control = TRUE;
	rtldm->thermalvalue = efuse->eeprom_thermalmeter;
	rtldm->thermalvalue_iqk = efuse->eeprom_thermalmeter;
	rtldm->thermalvalue_lck = efuse->eeprom_thermalmeter;

	/* The index of "0 dB" in SwingTable. */
	{
		u8 defaultSwingIndex = getSwingIndex(pDM_Odm);


		rtldm->default_ofdm_index = (defaultSwingIndex == TXSCALE_TABLE_SIZE) ? 24 : defaultSwingIndex;
		rtldm->default_cck_index = 24;
	}

	rtldm->swing_idx_cck_base = rtldm->default_cck_index;
	rtldm->cck_index = rtldm->default_cck_index;

	for (p = RF90_PATH_A; p < MAX_RF_PATH; ++p) {
		rtldm->swing_idx_ofdm_base[p] = rtldm->default_ofdm_index;
		rtldm->ofdm_index[p] = rtldm->default_ofdm_index;
		rtldm->delta_power_index[p] = 0;
		rtldm->delta_power_index_last[p] = 0;
		rtldm->power_index_offset[p] = 0;
	}
}

void ODM_DMInit(struct _rtw_dm *pDM_Odm)
{
	struct rtl_priv *rtlpriv = pDM_Odm->rtlpriv;
	
	/* 2012.05.03 Luke: For all IC series */
	rtl8821au_cm_common_info_self_update(pDM_Odm);
	/* Ulli : check function in rtlwifi/core.c for _rtl_dm_diginit() */	
	_rtl_dm_diginit(pDM_Odm);
	odm_AdaptivityInit(rtlpriv);
	rtl8821au_dm_init_rate_adaptive_mask(rtlpriv);

	rtl8821au_dm_init_edca_turbo(rtlpriv);

	rtl8821au_dm_initialize_txpower_tracking_thermalmeter(pDM_Odm);
}

/* From hal/OUTSRC/rtl8812a/HalPhyRf_8812A.c */

/*
 * ============================================================
 *  Tx Power Tracking
 * ============================================================
 */
/* From hal/OUTSRC/rtl8812a/HalPhyRf_8812A.c, caution function pointer */
void DoIQK_8812A(struct rtl_priv *rtlpriv, u8 DeltaThermalIndex,
	u8 	ThermalValue, u8 Threshold)
{
	struct rtl_dm	*rtldm = rtl_dm(rtlpriv);

	rtldm->thermalvalue_iqk = ThermalValue;
	rtl8812au_phy_iq_calibrate(rtlpriv, FALSE);
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

void ODM_TxPwrTrackSetPwr8812A(struct rtl_priv *rtlpriv, PWRTRACK_METHOD Method,
	u8 	RFPath, u8 	ChannelMappedIndex)
{
	uint32_t 	finalBbSwingIdx[2];

	struct rtl_dm	*rtldm = rtl_dm(rtlpriv);

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
#if 0	
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD, ("TxRate=0x%x, PwrTrackingLimit=%d\n", TxRate, PwrTrackingLimit));
#endif
	if (Method == BBSWING) {
#if 0		
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD, ("===>ODM_TxPwrTrackSetPwr8812A\n"));
#endif
		if (RFPath == RF90_PATH_A) {
			finalBbSwingIdx[RF90_PATH_A] = (rtldm->ofdm_index[RF90_PATH_A] > PwrTrackingLimit) ? PwrTrackingLimit : rtldm->ofdm_index[RF90_PATH_A];
#if 0			
			ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD, ("pDM_Odm->RFCalibrateInfo.OFDM_index[RF90_PATH_A]=%d, pDM_Odm->RealBbSwingIdx[RF90_PATH_A]=%d\n",
				rtldm->OFDM_index[RF90_PATH_A], finalBbSwingIdx[RF90_PATH_A]));
#endif
			rtl_set_bbreg(rtlpriv, rA_TxScale_Jaguar, 0xFFE00000, TxScalingTable_Jaguar[finalBbSwingIdx[RF90_PATH_A]]);
		} else {
			finalBbSwingIdx[RF90_PATH_B] = (rtldm->ofdm_index[RF90_PATH_B] > PwrTrackingLimit) ? PwrTrackingLimit : rtldm->ofdm_index[RF90_PATH_B];
#if 0			
			ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD, ("pDM_Odm->RFCalibrateInfo.OFDM_index[RF90_PATH_B]=%d, pDM_Odm->RealBbSwingIdx[RF90_PATH_B]=%d\n",
				rtldm->OFDM_index[RF90_PATH_B], finalBbSwingIdx[RF90_PATH_B]));
#endif
			rtl_set_bbreg(rtlpriv, rB_TxScale_Jaguar, 0xFFE00000, TxScalingTable_Jaguar[finalBbSwingIdx[RF90_PATH_B]]);
		}
	} else if (Method == MIX_MODE) {
#if 0		
			ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD, ("pDM_Odm->DefaultOfdmIndex=%d, pDM_Odm->Aboslute_OFDMSwingIdx[RFPath]=%d, RF_Path = %d\n",
				rtldm->DefaultOfdmIndex, rtldm->absolute_ofdm_swing_idx[RFPath], RFPath));
#endif

			Final_OFDM_Swing_Index = rtldm->default_ofdm_index + rtldm->absolute_ofdm_swing_idx[RFPath];

			if (RFPath == RF90_PATH_A) {
				if (Final_OFDM_Swing_Index > PwrTrackingLimit) {    /* BBSwing higher then Limit */
					rtldm->remnant_cck_idx = Final_OFDM_Swing_Index - PwrTrackingLimit;            /*  CCK Follow the same compensate value as Path A */
					rtldm->remnant_ofdm_swing_idx[RFPath] = Final_OFDM_Swing_Index - PwrTrackingLimit;

					rtl_set_bbreg(rtlpriv, rA_TxScale_Jaguar, 0xFFE00000, TxScalingTable_Jaguar[PwrTrackingLimit]);

					rtldm->modify_txagc_flag_path_a = TRUE;

					/* et TxAGC Page C{}; */
					/* rtlpriv->cfg->ops.SetTxPowerLevelHandler(rtlpriv, pHalData->CurrentChannel); */
					PHY_SetTxPowerLevel8812(rtlpriv, rtlpriv->phy.current_channel);
#if 0
					ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD, ("******Path_A Over BBSwing Limit , PwrTrackingLimit = %d , Remnant TxAGC Value = %d \n", PwrTrackingLimit, rtldm->Remnant_OFDMSwingIdx[RFPath]));
#endif				
				} else if (Final_OFDM_Swing_Index < 0) {
					rtldm->remnant_cck_idx = Final_OFDM_Swing_Index;            /* CCK Follow the same compensate value as Path A */
					rtldm->remnant_ofdm_swing_idx[RFPath] = Final_OFDM_Swing_Index;

					rtl_set_bbreg(rtlpriv, rA_TxScale_Jaguar, 0xFFE00000, TxScalingTable_Jaguar[0]);

					rtldm->modify_txagc_flag_path_a = TRUE;

					/* Set TxAGC Page C{}; */
					/* rtlpriv->cfg->ops.SetTxPowerLevelHandler(rtlpriv, pHalData->CurrentChannel);*/
					PHY_SetTxPowerLevel8812(rtlpriv, rtlpriv->phy.current_channel);
#if 0
					ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD, ("******Path_A Lower then BBSwing lower bound  0 , Remnant TxAGC Value = %d \n", rtldm->Remnant_OFDMSwingIdx[RFPath]));
#endif					
				} else 	{
					rtl_set_bbreg(rtlpriv, rA_TxScale_Jaguar, 0xFFE00000, TxScalingTable_Jaguar[Final_OFDM_Swing_Index]);
#if 0
					ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD, ("******Path_A Compensate with BBSwing , Final_OFDM_Swing_Index = %d \n", Final_OFDM_Swing_Index));
#endif
					if (rtldm->modify_txagc_flag_path_a) { /* If TxAGC has changed, reset TxAGC again */
						rtldm->remnant_cck_idx = 0;
						rtldm->remnant_ofdm_swing_idx[RFPath] = 0;

						/* Set TxAGC Page C{}; */
						/* rtlpriv->cfg->ops.SetTxPowerLevelHandler(rtlpriv, pHalData->CurrentChannel); */
						PHY_SetTxPowerLevel8812(rtlpriv, rtlpriv->phy.current_channel);

						rtldm->modify_txagc_flag_path_a = FALSE;
#if 0
						ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD, ("******Path_A pDM_Odm->Modify_TxAGC_Flag = FALSE \n"));
#endif						
					}
				}
			}

			if (RFPath == RF90_PATH_B) {
				if (Final_OFDM_Swing_Index > PwrTrackingLimit) {			/* BBSwing higher then Limit */
					rtldm->remnant_ofdm_swing_idx[RFPath] = Final_OFDM_Swing_Index - PwrTrackingLimit;

					rtl_set_bbreg(rtlpriv, rB_TxScale_Jaguar, 0xFFE00000, TxScalingTable_Jaguar[PwrTrackingLimit]);

					rtldm->modify_txagc_flag_path_b = TRUE;

					/* Set TxAGC Page E{}; */
#if 0
					ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD, ("******Path_B Over BBSwing Limit , PwrTrackingLimit = %d , Remnant TxAGC Value = %d \n", PwrTrackingLimit, rtldm->Remnant_OFDMSwingIdx[RFPath]));
#endif					
				} else if (Final_OFDM_Swing_Index < 0) {
					rtldm->remnant_ofdm_swing_idx[RFPath] = Final_OFDM_Swing_Index;

					rtl_set_bbreg(rtlpriv, rB_TxScale_Jaguar, 0xFFE00000, TxScalingTable_Jaguar[0]);

					rtldm->modify_txagc_flag_path_b = TRUE;

					/* Set TxAGC Page E{}; */
#if 0
					ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD, ("******Path_B Lower then BBSwing lower bound  0 , Remnant TxAGC Value = %d \n", rtldm->Remnant_OFDMSwingIdx[RFPath]));
#endif					
				} else {
					rtl_set_bbreg(rtlpriv, rB_TxScale_Jaguar, 0xFFE00000, TxScalingTable_Jaguar[Final_OFDM_Swing_Index]);
#if 0
					ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD, ("******Path_B Compensate with BBSwing , Final_OFDM_Swing_Index = %d \n", Final_OFDM_Swing_Index));
#endif
					if (rtldm->modify_txagc_flag_path_b) {			/* If TxAGC has changed, reset TxAGC again */
						rtldm->remnant_cck_idx = 0;
						rtldm->remnant_ofdm_swing_idx[RFPath] = 0;

						/* Set TxAGC Page E{}; */

						rtldm->modify_txagc_flag_path_b = FALSE;
#if 0
						ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD, ("******Path_B pDM_Odm->Modify_TxAGC_Flag = FALSE \n"));
#endif						
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

void rtl8812au_get_delta_swing_table(struct rtl_priv *rtlpriv,
					    u8 **up_a, u8 **down_a,
					    u8 **up_b, u8 **down_b)
{
	struct rtl_hal	*rtlhal = rtl_hal(rtlpriv);
	struct _rtw_hal	*pHalData = GET_HAL_DATA(rtlpriv);
	struct _rtw_dm *pDM_Odm = &pHalData->odmpriv;	
	
	u16	rate = *(pDM_Odm->pForcedDataRate);
	u8         	channel   		 = rtlpriv->phy.current_channel;

	if (rtlhal->rfe_type == 3 && pDM_Odm->bIsMPChip) {
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


/* From hal/OUTSRC/rtl8821a/HalPhyRf_8821A.c, caution function pointer */

void DoIQK_8821A(struct rtl_priv *rtlpriv, u8 DeltaThermalIndex,
	u8 ThermalValue, u8 Threshold)
{
	struct rtl_dm	*rtldm = rtl_dm(rtlpriv);

	rtldm->thermalvalue_iqk = ThermalValue;
	rtl8821au_phy_iq_calibrate(rtlpriv, FALSE);
}


void ODM_TxPwrTrackSetPwr8821A(struct rtl_priv *rtlpriv, PWRTRACK_METHOD Method,
	u8 RFPath, u8 ChannelMappedIndex)
{
	struct rtl_dm	*rtldm = rtl_dm(rtlpriv);

	u8 PwrTrackingLimit = 26; /* +1.0dB */
	u8 TxRate = 0xFF;
	s8 Final_OFDM_Swing_Index = 0;
	s8 Final_CCK_Swing_Index = 0;
	u8 i = 0;
	uint32_t finalBbSwingIdx[1];

#if 0
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD, ("===>ODM_TxPwrTrackSetPwr8821A\n"));
#endif
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
#if 0	
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD, ("TxRate=0x%x, PwrTrackingLimit=%d\n", TxRate, PwrTrackingLimit));
#endif
	if (Method == BBSWING) {
		if (RFPath == RF90_PATH_A) {
			finalBbSwingIdx[RF90_PATH_A] = (rtldm->ofdm_index[RF90_PATH_A] > PwrTrackingLimit) ? PwrTrackingLimit : rtldm->ofdm_index[RF90_PATH_A];
#if 0			
			ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD, ("pDM_Odm->RFCalibrateInfo.OFDM_index[RF90_PATH_A]=%d, pDM_Odm->RealBbSwingIdx[RF90_PATH_A]=%d\n",
				rtldm->OFDM_index[RF90_PATH_A], finalBbSwingIdx[RF90_PATH_A]));
#endif
			rtl_set_bbreg(rtlpriv, rA_TxScale_Jaguar, 0xFFE00000, TxScalingTable_Jaguar[finalBbSwingIdx[RF90_PATH_A]]);
		}
	} else if (Method == MIX_MODE) {
#if 0		
			ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD, ("pDM_Odm->DefaultOfdmIndex=%d, pDM_Odm->Aboslute_OFDMSwingIdx[RFPath]=%d, RF_Path = %d\n",
				rtldm->DefaultOfdmIndex, rtldm->Aboslute_OFDMSwingIdx[RFPath], RFPath));
#endif

			Final_OFDM_Swing_Index = rtldm->default_ofdm_index + rtldm->absolute_ofdm_swing_idx[RFPath];

			if (RFPath == RF90_PATH_A) {
				if (Final_OFDM_Swing_Index > PwrTrackingLimit) {
					/* BBSwing higher then Limit */
					rtldm->remnant_cck_idx = Final_OFDM_Swing_Index - PwrTrackingLimit;            /* CCK Follow the same compensate value as Path A */
					rtldm->remnant_ofdm_swing_idx[RFPath] = Final_OFDM_Swing_Index - PwrTrackingLimit;

					rtl_set_bbreg(rtlpriv, rA_TxScale_Jaguar, 0xFFE00000, TxScalingTable_Jaguar[PwrTrackingLimit]);

					rtldm->modify_txagc_flag_path_a = TRUE;

					/* Set TxAGC Page C{}; */
					/* rtlpriv->cfg->ops.SetTxPowerLevelHandler(rtlpriv, pHalData->CurrentChannel); */
					PHY_SetTxPowerLevel8812(rtlpriv, rtlpriv->phy.current_channel);
#if 0
					ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD, ("******Path_A Over BBSwing Limit , PwrTrackingLimit = %d , Remnant TxAGC Value = %d \n", PwrTrackingLimit, rtldm->Remnant_OFDMSwingIdx[RFPath]));
#endif					
				} else if (Final_OFDM_Swing_Index < 0) {
					rtldm->remnant_cck_idx = Final_OFDM_Swing_Index;            /* CCK Follow the same compensate value as Path A */
					rtldm->remnant_ofdm_swing_idx[RFPath] = Final_OFDM_Swing_Index;

					rtl_set_bbreg(rtlpriv, rA_TxScale_Jaguar, 0xFFE00000, TxScalingTable_Jaguar[0]);

					rtldm->modify_txagc_flag_path_a = TRUE;

					/* Set TxAGC Page C{}; */
					/* rtlpriv->cfg->ops.SetTxPowerLevelHandler(rtlpriv, pHalData->CurrentChannel); */
					PHY_SetTxPowerLevel8812(rtlpriv, rtlpriv->phy.current_channel);
#if 0
					ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD, ("******Path_A Lower then BBSwing lower bound  0 , Remnant TxAGC Value = %d \n", rtldm->Remnant_OFDMSwingIdx[RFPath]));
#endif					
				} else {
					rtl_set_bbreg(rtlpriv, rA_TxScale_Jaguar, 0xFFE00000, TxScalingTable_Jaguar[Final_OFDM_Swing_Index]);
#if 0
					ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD, ("******Path_A Compensate with BBSwing , Final_OFDM_Swing_Index = %d \n", Final_OFDM_Swing_Index));
#endif
					if (rtldm->modify_txagc_flag_path_a) {
						/* If TxAGC has changed, reset TxAGC again */
						rtldm->remnant_cck_idx = 0;
						rtldm->remnant_ofdm_swing_idx[RFPath] = 0;

						/* Set TxAGC Page C{}; */
						/* rtlpriv->cfg->ops.SetTxPowerLevelHandler(rtlpriv, pHalData->CurrentChannel); */
						PHY_SetTxPowerLevel8812(rtlpriv, rtlpriv->phy.current_channel);

						rtldm->modify_txagc_flag_path_a = FALSE;
#if 0
						ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD, ("******Path_A pDM_Odm->Modify_TxAGC_Flag = FALSE \n"));
#endif						
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

void rtl8821au_get_delta_swing_table(struct rtl_priv *rtlpriv,
	u8 **up_a, u8 **down_a,
	u8 **up_b, u8 **down_b)
{
	/* u16     rate = pMgntInfo->ForcedDataRate; */
	u16	rate = 0;
	u8         	channel   		 = rtlpriv->phy.current_channel;

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

static void rtl8812au_dm_txpower_tracking_callback_thermalmeter(struct rtl_priv *rtlpriv)
{
	struct rtl_hal *rtlhal = rtl_hal(rtlpriv);
	struct rtl_efuse *efuse = rtl_efuse(rtlpriv);
	struct rtl_dm	*rtldm = rtl_dm(rtlpriv);

	 struct _rtw_hal	*pHalData = GET_HAL_DATA(rtlpriv);
	struct _rtw_dm *pDM_Odm = &pHalData->odmpriv;

	u8	ThermalValue = 0, delta, delta_LCK, delta_IQK, p = 0, i = 0;
	u8	ThermalValue_AVG_count = 0;
	uint32_t	ThermalValue_AVG = 0;

	u8	OFDM_min_index = 0;  /* OFDM BB Swing should be less than +if (.0dB, which is required by Arthur */
	u8	Indexforchannel = 0; /* GetRightChnlPlaceforIQK(pHalData->CurrentChannel) */

	/* 4 1. The following TWO tables decide the final index of OFDM/CCK swing table. */
	u8 *up_a, *down_a, *up_b, *down_b;

	/* 4 2. Initilization ( 7 steps in total ) */

	rtl8812au_get_delta_swing_table(rtlpriv, (u8 **)&up_a, (u8 **)&down_a,
					 (u8 **)&up_b, (u8 **)&down_b);

#if 0		/* ULLI : only writing, no use */		
	rtldm->TXPowerTrackingCallbackCnt++; /* cosa add for debug */
#endif	
	rtldm->txpower_trackinginit = TRUE;

	ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD, 
		("===>ODM_TXPowerTrackingCallback_ThermalMeter, \n rtldm->BbSwingIdxCckBase: %d, rtldm->BbSwingIdxOfdmBase[A]: %d, rtldm->DefaultOfdmIndex: %d\n", 
		rtldm->swing_idx_cck_base, rtldm->swing_idx_ofdm_base[RF90_PATH_A], 
		rtldm->default_ofdm_index));

	ThermalValue = (u8)rtw_hal_read_rfreg(rtlpriv, RF90_PATH_A, RF_T_METER_8812A, 0xfc00);	/* 0x42: RF Reg[15:10] 88E */
	if (!rtldm->txpower_track_control
	 || efuse->eeprom_thermalmeter == 0
	 || efuse->eeprom_thermalmeter == 0xFF)
		return;

	/* 4 if (. Initialize ThermalValues of RFCalibrateInfo */

#if 0 	/* ULLI : var not used, check ?? */
	if (rtldm->bReloadtxpowerindex) {
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD, ("reload ofdm index for band switch\n"));
	}
#endif	

	/* 4 4. Calculate average thermal meter */

	rtldm->thermalvalue_avg[rtldm->thermalvalue_avg_index] = ThermalValue;
	rtldm->thermalvalue_avg_index++;
	if (rtldm->thermalvalue_avg_index == AVG_THERMAL_NUM_8812A)   /* Average times =  c.AverageThermalNum */
		rtldm->thermalvalue_avg_index = 0;

	for (i = 0; i < AVG_THERMAL_NUM_8812A; i++) {
		if (rtldm->thermalvalue_avg[i]) {
			ThermalValue_AVG += rtldm->thermalvalue_avg[i];
			ThermalValue_AVG_count++;
		}
	}

	if (ThermalValue_AVG_count) {               /* Calculate Average ThermalValue after average enough times */
		ThermalValue = (u8)(ThermalValue_AVG / ThermalValue_AVG_count);
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD, ("AVG Thermal Meter = 0x%X, EFUSE Thermal Base = 0x%X\n", ThermalValue, efuse->eeprom_thermalmeter));
	}

	/* 4 5. Calculate delta, delta_LCK, delta_IQK. */

	/* "delta" here is used to determine whether thermal value changes or not. */
	delta 	  = (ThermalValue > rtldm->thermalvalue)?(ThermalValue - rtldm->thermalvalue):(rtldm->thermalvalue - ThermalValue);
	delta_LCK = (ThermalValue > rtldm->thermalvalue_lck)?(ThermalValue - rtldm->thermalvalue_lck):(rtldm->thermalvalue_lck - ThermalValue);
	delta_IQK = (ThermalValue > rtldm->thermalvalue_iqk)?(ThermalValue - rtldm->thermalvalue_iqk):(rtldm->thermalvalue_iqk - ThermalValue);

	ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD, ("(delta, delta_LCK, delta_IQK) = (%d, %d, %d)\n", delta, delta_LCK, delta_IQK));

	/* 4 6. If necessary, do LCK. */

	if ((delta_LCK >= IQK_THRESHOLD)) {	/* Delta temperature is equal to or larger than 20 centigrade. */
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD, ("delta_LCK(%d) >= Threshold_IQK(%d)\n", delta_LCK, IQK_THRESHOLD));
		rtldm->thermalvalue_lck = ThermalValue;
		PHY_LCCalibrate_8812A(rtlpriv);
	}

	/* if ( 7. If necessary, move the index of swing table to adjust Tx power. */

	if (delta > 0 && rtldm->txpower_track_control) {
		/* "delta" here is used to record the absolute value of differrence. */
	    delta = ThermalValue > efuse->eeprom_thermalmeter?(ThermalValue - efuse->eeprom_thermalmeter):(efuse->eeprom_thermalmeter - ThermalValue);
		if (delta >= TXSCALE_TABLE_SIZE)
			delta = TXSCALE_TABLE_SIZE - 1;

		/* 4 7.1 The Final Power Index = BaseIndex + PowerIndexOffset */

		if (ThermalValue > efuse->eeprom_thermalmeter) {
			ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD, ("deltaSwingTableIdx_TUP_A[%d] = %d\n", delta, up_a[delta]));
			rtldm->delta_power_index_last[RF90_PATH_A] = rtldm->delta_power_index[RF90_PATH_A];
			rtldm->delta_power_index[RF90_PATH_A] = up_a[delta];

			rtldm->absolute_ofdm_swing_idx[RF90_PATH_A] =  up_a[delta];        /* Record delta swing for mix mode power tracking */

			ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD, ("******Temp is higher and rtldm->Aboslute_OFDMSwingIdx[RF90_PATH_A] = %d\n", rtldm->absolute_ofdm_swing_idx[RF90_PATH_A]));

			ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD, ("deltaSwingTableIdx_TUP_B[%d] = %d\n", delta, up_b[delta]));
			rtldm->delta_power_index_last[RF90_PATH_B] = rtldm->delta_power_index[RF90_PATH_B];
			rtldm->delta_power_index[RF90_PATH_B] = up_b[delta];

			rtldm->absolute_ofdm_swing_idx[RF90_PATH_B] =  up_a[delta];       /* Record delta swing for mix mode power tracking */

			ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD, ("******Temp is higher and rtldm->Aboslute_OFDMSwingIdx[RF90_PATH_B] = %d\n", rtldm->absolute_ofdm_swing_idx[RF90_PATH_B]));

		} else {
			ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD, ("deltaSwingTableIdx_TDOWN_A[%d] = %d\n", delta, down_a[delta]));

			rtldm->delta_power_index_last[RF90_PATH_A] = rtldm->delta_power_index[RF90_PATH_A];
			rtldm->delta_power_index[RF90_PATH_A] = -1 * down_a[delta];

			rtldm->absolute_ofdm_swing_idx[RF90_PATH_A] =  -1 * down_a[delta];        /* Record delta swing for mix mode power tracking */

			ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD, ("******Temp is lower and rtldm->Aboslute_OFDMSwingIdx[RF90_PATH_A] = %d\n", rtldm->absolute_ofdm_swing_idx[RF90_PATH_A]));

			ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD, ("deltaSwingTableIdx_TDOWN_B[%d] = %d\n", delta, down_b[delta]));

			rtldm->delta_power_index_last[RF90_PATH_B] = rtldm->delta_power_index[RF90_PATH_B];
			rtldm->delta_power_index[RF90_PATH_B] = -1 * down_b[delta];

			rtldm->absolute_ofdm_swing_idx[RF90_PATH_B] =  -1 * down_b[delta];       /* Record delta swing for mix mode power tracking */

			ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD, ("******Temp is lower and rtldm->Aboslute_OFDMSwingIdx[RF90_PATH_B] = %d\n", rtldm->absolute_ofdm_swing_idx[RF90_PATH_B]));
		}

	    for (p = RF90_PATH_A; p < MAX_PATH_NUM_8812A; p++) {
			ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD, ("\n\n================================ [Path-%c] Calculating PowerIndexOffset ================================\n", (p == RF90_PATH_A ? 'A' : 'B')));
			if (rtldm->delta_power_index[p] == rtldm->delta_power_index_last[p])         /* If Thermal value changes but lookup table value still the same */
				rtldm->power_index_offset[p] = 0;
			else
				rtldm->power_index_offset[p] = rtldm->delta_power_index[p] - rtldm->delta_power_index_last[p];      /* Power Index Diff between 2 times Power Tracking */

			ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD, ("[Path-%c] PowerIndexOffset(%d) = DeltaPowerIndex(%d) - DeltaPowerIndexLast(%d)\n", (p == RF90_PATH_A ? 'A' : 'B'), rtldm->power_index_offset[p], rtldm->delta_power_index[p],
			rtldm->delta_power_index_last[p]));

			rtldm->ofdm_index[p] = rtldm->swing_idx_ofdm_base[p] + rtldm->power_index_offset[p];
			rtldm->cck_index = rtldm->swing_idx_cck_base + rtldm->power_index_offset[p];

			rtldm->swing_idx_cck = rtldm->cck_index;
			rtldm->swing_idx_ofdm[p] = rtldm->ofdm_index[p];

	       /* *************Print BB Swing Base and Index Offset************* */

			ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD, ("The 'CCK' final index(%d) = BaseIndex(%d) + PowerIndexOffset(%d)\n",  rtldm->swing_idx_cck, rtldm->swing_idx_cck_base, rtldm->power_index_offset[p]));
			ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD, ("The 'OFDM' final index(%d) = BaseIndex[%c](%d) + PowerIndexOffset(%d)\n", rtldm->swing_idx_ofdm[p], (p == RF90_PATH_A ? 'A' : 'B'), rtldm->swing_idx_ofdm_base[p], rtldm->power_index_offset[p]));

		    /* 4 7.1 Handle boundary conditions of index. */

			if (rtldm->ofdm_index[p] > TXSCALE_TABLE_SIZE-1) {
				rtldm->ofdm_index[p] = TXSCALE_TABLE_SIZE-1;
			} else if (rtldm->ofdm_index[p] < OFDM_min_index) {
				rtldm->ofdm_index[p] = OFDM_min_index;
			}
		}
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD, ("\n\n========================================================================================================\n"));
		if (rtldm->cck_index > TXSCALE_TABLE_SIZE-1)
			rtldm->cck_index = TXSCALE_TABLE_SIZE-1;
	} else {
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD, ("The thermal meter is unchanged or TxPowerTracking OFF(%d): ThermalValue: %d , rtldm->RFCalibrateInfo.ThermalValue: %d\n", rtldm->txpower_track_control, ThermalValue, rtldm->thermalvalue));

		for (p = RF90_PATH_A; p < MAX_PATH_NUM_8812A; p++)
			rtldm->power_index_offset[p] = 0;
	}
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD, ("TxPowerTracking: [CCK] Swing Current Index: %d, Swing Base Index: %d\n", rtldm->cck_index, rtldm->swing_idx_cck_base));       /* Print Swing base & current */
	for (p = RF90_PATH_A; p < MAX_PATH_NUM_8812A; p++) {
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD, ("TxPowerTracking: [OFDM] Swing Current Index: %d, Swing Base Index[%c]: %d\n", rtldm->ofdm_index[p], (p == RF90_PATH_A ? 'A' : 'B'), rtldm->swing_idx_ofdm_base[p]));
	}

	if ((rtldm->power_index_offset[RF90_PATH_A] != 0 ||  rtldm->power_index_offset[RF90_PATH_B] != 0)
	 && rtldm->txpower_track_control) {
		/* 4 7.2 Configure the Swing Table to adjust Tx Power. */
#if 0		/* ULLI : only writing, no use */		
		rtldm->bTxPowerChanged = TRUE; /* Always TRUE after Tx Power is adjusted by power tracking. */
#endif		
		/*
		 *  2012/04/2if ( MH According to Luke's suggestion, we can not write BB digital
		 *  to increase TX power. Otherwise, EVM will be bad.
		 *
		 *  2012/04/25 MH Add for tx power tracking to set tx power in tx agc for 88E.
		 */
		if (ThermalValue > rtldm->thermalvalue) {
			ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD, ("Temperature Increasing(A): delta_pi: %d , delta_t: %d, Now_t: %d, EFUSE_t: %d, Last_t: %d\n", rtldm->power_index_offset[RF90_PATH_A], delta, ThermalValue, efuse->eeprom_thermalmeter, rtldm->thermalvalue));
			ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD, ("Temperature Increasing(B): delta_pi: %d , delta_t: %d, Now_t: %d, EFUSE_t: %d, Last_t: %d\n", rtldm->power_index_offset[RF90_PATH_B], delta, ThermalValue, efuse->eeprom_thermalmeter, rtldm->thermalvalue));

			} else if (ThermalValue < rtldm->thermalvalue) { /* Low temperature */
				ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD, ("Temperature Decreasing(A): delta_pi: %d , delta_t: %d, Now_t: %d, EFUSE_t: %d, Last_t: %d\n", rtldm->power_index_offset[RF90_PATH_A], delta, ThermalValue, efuse->eeprom_thermalmeter, rtldm->thermalvalue));
				ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD, ("Temperature Decreasing(B): delta_pi: %d , delta_t: %d, Now_t: %d, EFUSE_t: %d, Last_t: %d\n", rtldm->power_index_offset[RF90_PATH_B], delta, ThermalValue, efuse->eeprom_thermalmeter, rtldm->thermalvalue));

			}
			if (ThermalValue > efuse->eeprom_thermalmeter) {
				ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD, ("Temperature(%d) higher than PG value(%d)\n", ThermalValue, efuse->eeprom_thermalmeter));

				for (p = RF90_PATH_A; p < MAX_PATH_NUM_8812A; p++)
					ODM_TxPwrTrackSetPwr8812A(rtlpriv, BBSWING, p, Indexforchannel);
			} else {
				ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD, ("Temperature(%d) lower than PG value(%d)\n", ThermalValue, efuse->eeprom_thermalmeter));

				for (p = RF90_PATH_A; p < MAX_PATH_NUM_8812A; p++)
					ODM_TxPwrTrackSetPwr8812A(rtlpriv, BBSWING, p, Indexforchannel);
			}

			rtldm->swing_idx_cck_base = rtldm->swing_idx_cck;  	/* Record last time Power Tracking result as base. */
			for (p = RF90_PATH_A; p < MAX_PATH_NUM_8812A; p++)	/* ULLI huh ?? */
				rtldm->swing_idx_ofdm_base[p] = rtldm->swing_idx_ofdm[p];

			ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD,  ("rtldm->RFCalibrateInfo.ThermalValue = %d ThermalValue= %d\n", rtldm->thermalvalue, ThermalValue));

			rtldm->thermalvalue = ThermalValue;     /* Record last Power Tracking Thermal Value */

	}
	if ((delta_IQK >= IQK_THRESHOLD))	/* Delta temperature is equal to or larger than 20 centigrade (When threshold is 8). */
		DoIQK_8812A(rtlpriv, delta_IQK, ThermalValue, 8);

	ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD, ("<===ODM_TXPowerTrackingCallback_ThermalMeter\n"));

	rtldm->txpowercount = 0;
}


static void rtl8821au_dm_txpower_tracking_callback_thermalmeter(struct rtl_priv *rtlpriv)
{
	struct rtl_hal *rtlhal = rtl_hal(rtlpriv);
	struct rtl_efuse *efuse = rtl_efuse(rtlpriv);
	struct rtl_dm	*rtldm = rtl_dm(rtlpriv);

	 struct _rtw_hal	*pHalData = GET_HAL_DATA(rtlpriv);
	struct _rtw_dm *pDM_Odm = &pHalData->odmpriv;

	u8	ThermalValue = 0, delta, delta_LCK, delta_IQK, p = 0, i = 0;
	u8	ThermalValue_AVG_count = 0;
	uint32_t	ThermalValue_AVG = 0;

	u8	OFDM_min_index = 0;  /* OFDM BB Swing should be less than +if (.0dB, which is required by Arthur */
	u8	Indexforchannel = 0; /* GetRightChnlPlaceforIQK(pHalData->CurrentChannel) */

	/* 4 1. The following TWO tables decide the final index of OFDM/CCK swing table. */
	u8 *up_a, *down_a, *up_b, *down_b;

	/* 4 2. Initilization ( 7 steps in total ) */

	rtl8821au_get_delta_swing_table(rtlpriv, (u8 **)&up_a, (u8 **)&down_a,
					 (u8 **)&up_b, (u8 **)&down_b);

#if 0		/* ULLI : only writing, no use */		
	rtldm->TXPowerTrackingCallbackCnt++; /* cosa add for debug */
	rtldm->bTXPowerTrackingInit = TRUE;
#endif

	ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD, ("===>ODM_TXPowerTrackingCallback_ThermalMeter, \n rtldm->BbSwingIdxCckBase: %d, rtldm->BbSwingIdxOfdmBase[A]: %d, rtldm->DefaultOfdmIndex: %d\n", rtldm->swing_idx_cck_base, rtldm->swing_idx_ofdm_base[RF90_PATH_A], rtldm->default_ofdm_index));

	ThermalValue = (u8)rtw_hal_read_rfreg(pDM_Odm->rtlpriv, RF90_PATH_A, RF_T_METER_8812A, 0xfc00);	/* 0x42: RF Reg[15:10] 88E */
	if (!rtldm->txpower_track_control
	 || efuse->eeprom_thermalmeter == 0
	 || efuse->eeprom_thermalmeter == 0xFF)
		return;

	/* 4 if (. Initialize ThermalValues of RFCalibrateInfo */

#if 0 	/* ULLI : var not used, check ?? */
	if (rtldm->bReloadtxpowerindex) {
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD, ("reload ofdm index for band switch\n"));
	}
#endif	

	/* 4 4. Calculate average thermal meter */

	rtldm->thermalvalue_avg[rtldm->thermalvalue_avg_index] = ThermalValue;
	rtldm->thermalvalue_avg_index++;
	if (rtldm->thermalvalue_avg_index == AVG_THERMAL_NUM_8812A)   /* Average times =  c.AverageThermalNum */
		rtldm->thermalvalue_avg_index = 0;

	for (i = 0; i < AVG_THERMAL_NUM_8812A; i++) {
		if (rtldm->thermalvalue_avg[i]) {
			ThermalValue_AVG += rtldm->thermalvalue_avg[i];
			ThermalValue_AVG_count++;
		}
	}

	if (ThermalValue_AVG_count) {               /* Calculate Average ThermalValue after average enough times */
		ThermalValue = (u8)(ThermalValue_AVG / ThermalValue_AVG_count);
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD, ("AVG Thermal Meter = 0x%X, EFUSE Thermal Base = 0x%X\n", ThermalValue, efuse->eeprom_thermalmeter));
	}

	/* 4 5. Calculate delta, delta_LCK, delta_IQK. */

	/* "delta" here is used to determine whether thermal value changes or not. */
	delta 	  = (ThermalValue > rtldm->thermalvalue)?(ThermalValue - rtldm->thermalvalue):(rtldm->thermalvalue - ThermalValue);
	delta_LCK = (ThermalValue > rtldm->thermalvalue_lck)?(ThermalValue - rtldm->thermalvalue_lck):(rtldm->thermalvalue_lck - ThermalValue);
	delta_IQK = (ThermalValue > rtldm->thermalvalue_iqk)?(ThermalValue - rtldm->thermalvalue_iqk):(rtldm->thermalvalue_iqk - ThermalValue);

	ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD, ("(delta, delta_LCK, delta_IQK) = (%d, %d, %d)\n", delta, delta_LCK, delta_IQK));

	/* 4 6. If necessary, do LCK. */

	if ((delta_LCK >= IQK_THRESHOLD)) {	/* Delta temperature is equal to or larger than 20 centigrade. */
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD, ("delta_LCK(%d) >= Threshold_IQK(%d)\n", delta_LCK, IQK_THRESHOLD));
		rtldm->thermalvalue_lck = ThermalValue;
		PHY_LCCalibrate_8812A(rtlpriv);
	}

	/* if ( 7. If necessary, move the index of swing table to adjust Tx power. */

	if (delta > 0 && rtldm->txpower_track_control) {
		/* "delta" here is used to record the absolute value of differrence. */
	    delta = ThermalValue > efuse->eeprom_thermalmeter?(ThermalValue - efuse->eeprom_thermalmeter):(efuse->eeprom_thermalmeter - ThermalValue);
		if (delta >= TXSCALE_TABLE_SIZE)
			delta = TXSCALE_TABLE_SIZE - 1;

		/* 4 7.1 The Final Power Index = BaseIndex + PowerIndexOffset */

		if (ThermalValue > efuse->eeprom_thermalmeter) {
			ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD, ("deltaSwingTableIdx_TUP_A[%d] = %d\n", delta, up_a[delta]));
			rtldm->delta_power_index_last[RF90_PATH_A] = rtldm->delta_power_index[RF90_PATH_A];
			rtldm->delta_power_index[RF90_PATH_A] = up_a[delta];

			rtldm->absolute_ofdm_swing_idx[RF90_PATH_A] =  up_a[delta];        /* Record delta swing for mix mode power tracking */

			ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD, ("******Temp is higher and rtldm->Aboslute_OFDMSwingIdx[RF90_PATH_A] = %d\n", rtldm->absolute_ofdm_swing_idx[RF90_PATH_A]));
		} else {
			ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD, ("deltaSwingTableIdx_TDOWN_A[%d] = %d\n", delta, down_a[delta]));

			rtldm->delta_power_index_last[RF90_PATH_A] = rtldm->delta_power_index[RF90_PATH_A];
			rtldm->delta_power_index[RF90_PATH_A] = -1 * down_a[delta];

			rtldm->absolute_ofdm_swing_idx[RF90_PATH_A] =  -1 * down_a[delta];        /* Record delta swing for mix mode power tracking */

			ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD, ("******Temp is lower and rtldm->Aboslute_OFDMSwingIdx[RF90_PATH_A] = %d\n", rtldm->absolute_ofdm_swing_idx[RF90_PATH_A]));
		}

	    for (p = RF90_PATH_A; p < MAX_PATH_NUM_8821A; p++) {
			ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD, ("\n\n================================ [Path-%c] Calculating PowerIndexOffset ================================\n", (p == RF90_PATH_A ? 'A' : 'B')));
			if (rtldm->delta_power_index[p] == rtldm->delta_power_index_last[p])         /* If Thermal value changes but lookup table value still the same */
				rtldm->power_index_offset[p] = 0;
			else
				rtldm->power_index_offset[p] = rtldm->delta_power_index[p] - rtldm->delta_power_index_last[p];      /* Power Index Diff between 2 times Power Tracking */

			ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD, ("[Path-%c] PowerIndexOffset(%d) = DeltaPowerIndex(%d) - DeltaPowerIndexLast(%d)\n", (p == RF90_PATH_A ? 'A' : 'B'), rtldm->power_index_offset[p], rtldm->delta_power_index[p], rtldm->delta_power_index_last[p]));

			rtldm->ofdm_index[p] = rtldm->swing_idx_ofdm_base[p] + rtldm->power_index_offset[p];
			rtldm->cck_index = rtldm->swing_idx_cck_base + rtldm->power_index_offset[p];

			rtldm->swing_idx_cck = rtldm->cck_index;
			rtldm->swing_idx_ofdm[p] = rtldm->ofdm_index[p];

	       /* *************Print BB Swing Base and Index Offset************* */

			ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD, ("The 'CCK' final index(%d) = BaseIndex(%d) + PowerIndexOffset(%d)\n", rtldm->swing_idx_cck, rtldm->swing_idx_cck_base, rtldm->power_index_offset[p]));
			ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD, ("The 'OFDM' final index(%d) = BaseIndex[%c](%d) + PowerIndexOffset(%d)\n", rtldm->swing_idx_ofdm[p], (p == RF90_PATH_A ? 'A' : 'B'), rtldm->swing_idx_ofdm_base[p], rtldm->power_index_offset[p]));

		    /* 4 7.1 Handle boundary conditions of index. */

			if (rtldm->ofdm_index[p] > TXSCALE_TABLE_SIZE-1) {
				rtldm->ofdm_index[p] = TXSCALE_TABLE_SIZE-1;
			} else if (rtldm->ofdm_index[p] < OFDM_min_index) {
				rtldm->ofdm_index[p] = OFDM_min_index;
			}
		}
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD, ("\n\n========================================================================================================\n"));
		if (rtldm->cck_index > TXSCALE_TABLE_SIZE-1)
			rtldm->cck_index = TXSCALE_TABLE_SIZE-1;
	} else {
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD, ("The thermal meter is unchanged or TxPowerTracking OFF(%d): ThermalValue: %d , rtldm->RFCalibrateInfo.ThermalValue: %d\n", rtldm->txpower_track_control, ThermalValue, rtldm->thermalvalue));

		for (p = RF90_PATH_A; p < MAX_PATH_NUM_8821A; p++)
			rtldm->power_index_offset[p] = 0;
	}
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD, ("TxPowerTracking: [CCK] Swing Current Index: %d, Swing Base Index: %d\n", rtldm->cck_index, rtldm->swing_idx_cck_base));       /* Print Swing base & current */
	for (p = RF90_PATH_A; p < MAX_PATH_NUM_8821A; p++) {
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD, ("TxPowerTracking: [OFDM] Swing Current Index: %d, Swing Base Index[%c]: %d\n", rtldm->ofdm_index[p], (p == RF90_PATH_A ? 'A' : 'B'), rtldm->swing_idx_ofdm_base[p]));
	}

	if ((rtldm->power_index_offset[RF90_PATH_A] != 0 ||  rtldm->power_index_offset[RF90_PATH_B] != 0)
	 && rtldm->txpower_track_control) {
		/* 4 7.2 Configure the Swing Table to adjust Tx Power. */
#if 0		/* ULLI : only writing, no use */		
		rtldm->bTxPowerChanged = TRUE; /* Always TRUE after Tx Power is adjusted by power tracking. */
#endif		
		/*
		 *  2012/04/2if ( MH According to Luke's suggestion, we can not write BB digital
		 *  to increase TX power. Otherwise, EVM will be bad.
		 *
		 *  2012/04/25 MH Add for tx power tracking to set tx power in tx agc for 88E.
		 */
		if (ThermalValue > rtldm->thermalvalue) {
			ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD, ("Temperature Increasing(A): delta_pi: %d , delta_t: %d, Now_t: %d, EFUSE_t: %d, Last_t: %d\n", rtldm->power_index_offset[RF90_PATH_A], delta, ThermalValue, efuse->eeprom_thermalmeter, rtldm->thermalvalue));

			} else if (ThermalValue < rtldm->thermalvalue) { /* Low temperature */
				ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD, ("Temperature Decreasing(A): delta_pi: %d , delta_t: %d, Now_t: %d, EFUSE_t: %d, Last_t: %d\n", rtldm->power_index_offset[RF90_PATH_A], delta, ThermalValue, efuse->eeprom_thermalmeter, rtldm->thermalvalue));

			}
			if (ThermalValue > efuse->eeprom_thermalmeter) {
				ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD, ("Temperature(%d) higher than PG value(%d)\n", ThermalValue, efuse->eeprom_thermalmeter));

				ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD, ("**********Enter POWER Tracking MIX_MODE**********\n"));
				for (p = RF90_PATH_A; p < MAX_PATH_NUM_8821A; p++)
					ODM_TxPwrTrackSetPwr8821A(rtlpriv, MIX_MODE, p, Indexforchannel);
			} else {
				ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD, ("Temperature(%d) lower than PG value(%d)\n", ThermalValue, efuse->eeprom_thermalmeter));

				ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD, ("**********Enter POWER Tracking MIX_MODE**********\n"));
				for (p = RF90_PATH_A; p < MAX_PATH_NUM_8821A; p++)
					ODM_TxPwrTrackSetPwr8821A(rtlpriv, MIX_MODE, p, Indexforchannel);
			}

			rtldm->swing_idx_cck_base = rtldm->swing_idx_cck;  	/* Record last time Power Tracking result as base. */
			for (p = RF90_PATH_A; p < MAX_PATH_NUM_8821A; p++)
				rtldm->swing_idx_ofdm_base[p] = rtldm->swing_idx_ofdm[p];

			ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD,  ("rtldm->RFCalibrateInfo.ThermalValue = %d ThermalValue= %d\n", rtldm->thermalvalue, ThermalValue));

			rtldm->thermalvalue = ThermalValue;     /* Record last Power Tracking Thermal Value */

	}
	if ((delta_IQK >= IQK_THRESHOLD))	/* Delta temperature is equal to or larger than 20 centigrade (When threshold is 8). */
		DoIQK_8821A(rtlpriv, delta_IQK, ThermalValue, 8);

	ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD, ("<===ODM_TXPowerTrackingCallback_ThermalMeter\n"));

	rtldm->txpowercount = 0;
}

void rtl8821au_check_tx_power_tracking_thermalmeter(struct _rtw_dm *pDM_Odm)
{
	struct rtl_priv *rtlpriv = pDM_Odm->rtlpriv;
	struct rtl_hal *rtlhal = rtl_hal(rtlpriv);

	if (!(pDM_Odm->SupportAbility & ODM_RF_TX_PWR_TRACK)) {
		return;
	}

	if (!pDM_Odm->RFCalibrateInfo.TM_Trigger) {		/* at least delay 1 sec */
		/* pHalData->TxPowerCheckCnt++;	//cosa add for debug */
		rtw_hal_write_rfreg(pDM_Odm->rtlpriv, RF90_PATH_A, RF_T_METER_NEW, (BIT17 | BIT16), 0x03);

		/* DBG_871X("Trigger Thermal Meter!!\n"); */

		pDM_Odm->RFCalibrateInfo.TM_Trigger = 1;
		return;
	} else {
		/* DBG_871X("Schedule TxPowerTracking direct call!!\n"); */
		if (IS_HARDWARE_TYPE_8812AU(rtlhal))
			rtl8812au_dm_txpower_tracking_callback_thermalmeter(rtlpriv);
		if (IS_HARDWARE_TYPE_8821U(rtlhal))
			rtl8821au_dm_txpower_tracking_callback_thermalmeter(rtlpriv);
		pDM_Odm->RFCalibrateInfo.TM_Trigger = 0;
	}

}



/*
 * 3============================================================
 * 3 FASLE ALARM CHECK
 * 3============================================================
 */

static void rtl8821ae_dm_false_alarm_counter_statistics(struct rtl_priv *rtlpriv)
{
	struct _rtw_hal *pHalData = GET_HAL_DATA(rtlpriv);
	struct _rtw_dm *pDM_Odm = &pHalData->odmpriv;
	
	uint32_t ret_value;
	PFALSE_ALARM_STATISTICS FalseAlmCnt = &(pDM_Odm->FalseAlmCnt);

	if (!(pDM_Odm->SupportAbility & ODM_BB_FA_CNT))
		return;

	{
		uint32_t CCKenable;
		/* read OFDM FA counter */
		FalseAlmCnt->Cnt_Ofdm_fail = rtl_get_bbreg(pDM_Odm->rtlpriv, ODM_REG_OFDM_FA_11AC, bMaskLWord);
		FalseAlmCnt->Cnt_Cck_fail = rtl_get_bbreg(pDM_Odm->rtlpriv, ODM_REG_CCK_FA_11AC, bMaskLWord);

		CCKenable =  rtl_get_bbreg(rtlpriv, ODM_REG_BB_RX_PATH_11AC, BIT28);
		if (CCKenable)		/* if (*pDM_Odm->pBandType == ODM_BAND_2_4G) */
			FalseAlmCnt->Cnt_all = FalseAlmCnt->Cnt_Ofdm_fail + FalseAlmCnt->Cnt_Cck_fail;
		else
			FalseAlmCnt->Cnt_all = FalseAlmCnt->Cnt_Ofdm_fail;

		/* reset OFDM FA coutner */
		rtl_set_bbreg(rtlpriv, ODM_REG_OFDM_FA_RST_11AC, BIT17, 1);
		rtl_set_bbreg(rtlpriv, ODM_REG_OFDM_FA_RST_11AC, BIT17, 0);
		/* reset CCK FA counter */
		rtl_set_bbreg(rtlpriv, ODM_REG_CCK_FA_RST_11AC, BIT15, 0);
		rtl_set_bbreg(rtlpriv, ODM_REG_CCK_FA_RST_11AC, BIT15, 1);
	}
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_FA_CNT, ODM_DBG_LOUD, ("Cnt_Cck_fail=%d\n", FalseAlmCnt->Cnt_Cck_fail));
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_FA_CNT, ODM_DBG_LOUD, ("Cnt_Ofdm_fail=%d\n", FalseAlmCnt->Cnt_Ofdm_fail));
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_FA_CNT, ODM_DBG_LOUD, ("Total False Alarm=%d\n", FalseAlmCnt->Cnt_all));
}


/*
 * 3============================================================
 * 3 RSSI Monitor
 * 3============================================================
 */


static void FindMinimumRSSI(struct rtl_priv *rtlpriv)
{
	 struct _rtw_hal	*pHalData = GET_HAL_DATA(rtlpriv);
	struct dm_priv	*pdmpriv = &pHalData->dmpriv;
	struct _rtw_dm *	pDM_Odm = &(pHalData->odmpriv);

	/* 1 1.Determine the minimum RSSI */

	if ((pDM_Odm->bLinked != _TRUE) && (pdmpriv->EntryMinUndecoratedSmoothedPWDB == 0)) {
		pdmpriv->MinUndecoratedPWDBForDM = 0;
		/* ODM_RT_TRACE(pDM_Odm,COMP_BB_POWERSAVING, DBG_LOUD, ("Not connected to any \n")); */
	} else {
		pdmpriv->MinUndecoratedPWDBForDM = pdmpriv->EntryMinUndecoratedSmoothedPWDB;
	}

	/* DBG_8192C("%s=>MinUndecoratedPWDBForDM(%d)\n",__FUNCTION__,pdmpriv->MinUndecoratedPWDBForDM); */
	/* ODM_RT_TRACE(pDM_Odm,COMP_DIG, DBG_LOUD, ("MinUndecoratedPWDBForDM =%d\n",pHalData->MinUndecoratedPWDBForDM)); */
}

static void odm_RSSIMonitorCheckCE(struct _rtw_dm *pDM_Odm)
{
	struct rtl_priv *rtlpriv = pDM_Odm->rtlpriv;
	 struct _rtw_hal	*pHalData = GET_HAL_DATA(rtlpriv);
	struct dm_priv	*pdmpriv = &pHalData->dmpriv;
	int	i;
	int	tmpEntryMaxPWDB = 0, tmpEntryMinPWDB = 0xff;
	u8 	sta_cnt = 0;
	u8	UL_DL_STATE = 0;			/*  for 8812 use */
	uint32_t PWDB_rssi[NUM_STA] = { 0 };		/* [0~15]:MACID, [16~31]:PWDB_rssi */

	if (pDM_Odm->bLinked != _TRUE)
		return;

	if (1) {
		u64	curTxOkCnt = rtlpriv->xmitpriv.tx_bytes - rtlpriv->xmitpriv.last_tx_bytes;
		u64	curRxOkCnt = rtlpriv->recvpriv.rx_bytes - rtlpriv->recvpriv.last_rx_bytes;

		if (curRxOkCnt > (curTxOkCnt*6))
			UL_DL_STATE = 1;
		else
			UL_DL_STATE = 0;
	}


	/* if (check_fwstate(&rtlpriv->mlmepriv, WIFI_AP_STATE|WIFI_ADHOC_STATE|WIFI_ADHOC_MASTER_STATE) == _TRUE) */
	{
#if 1
		struct sta_info *psta;

		for (i = 0; i < ODM_ASSOCIATE_ENTRY_NUM; i++) {
			psta = pDM_Odm->pODM_StaInfo[i];
			if (IS_STA_VALID(psta)) {
				if (psta->rssi_stat.UndecoratedSmoothedPWDB < tmpEntryMinPWDB)
					tmpEntryMinPWDB = psta->rssi_stat.UndecoratedSmoothedPWDB;

				if (psta->rssi_stat.UndecoratedSmoothedPWDB > tmpEntryMaxPWDB)
					tmpEntryMaxPWDB = psta->rssi_stat.UndecoratedSmoothedPWDB;

				if (psta->rssi_stat.UndecoratedSmoothedPWDB != (-1)) {
					if (1)
						PWDB_rssi[sta_cnt++] = (((u8)(psta->mac_id&0xFF)) | ((psta->rssi_stat.UndecoratedSmoothedPWDB&0x7F)<<16));
					else
						PWDB_rssi[sta_cnt++] = (psta->mac_id | (psta->rssi_stat.UndecoratedSmoothedPWDB<<16));

				}
			}
		}
#else
		_irqL irqL;
		struct list_head	*plist, *phead;
		struct sta_info *psta;
		struct sta_priv *pstapriv = &rtlpriv->stapriv;
		u8 bcast_addr[ETH_ALEN] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};

		spin_lock_bh(&pstapriv->sta_hash_lock, &irqL);

		for (i = 0; i < NUM_STA; i++) {
			phead = &(pstapriv->sta_hash[i]);
			plist = get_next(phead);

			while ((rtw_end_of_queue_search(phead, plist)) == _FALSE) {
				psta = LIST_CONTAINOR(plist, struct sta_info, hash_list);

				plist = get_next(plist);

				if (_rtw_memcmp(psta->hwaddr, bcast_addr, ETH_ALEN)
				 || _rtw_memcmp(psta->hwaddr, myid(&rtlpriv->eeprompriv), ETH_ALEN))
					continue;

				if (psta->state & WIFI_ASOC_STATE) {
					if (psta->rssi_stat.UndecoratedSmoothedPWDB < tmpEntryMinPWDB)
						tmpEntryMinPWDB = psta->rssi_stat.UndecoratedSmoothedPWDB;

					if (psta->rssi_stat.UndecoratedSmoothedPWDB > tmpEntryMaxPWDB)
						tmpEntryMaxPWDB = psta->rssi_stat.UndecoratedSmoothedPWDB;

					if (psta->rssi_stat.UndecoratedSmoothedPWDB != (-1)) {
						/* printk("%s==> mac_id(%d),rssi(%d)\n",__FUNCTION__,psta->mac_id,psta->rssi_stat.UndecoratedSmoothedPWDB); */
						PWDB_rssi[sta_cnt++] = (psta->mac_id | (psta->rssi_stat.UndecoratedSmoothedPWDB<<16));
					}
				}

			}

		}

		spin_unlock_bh(&pstapriv->sta_hash_lock, &irqL);
#endif

		/* printk("%s==> sta_cnt(%d)\n",__FUNCTION__,sta_cnt); */

		for (i = 0; i < sta_cnt; i++) {
			if (PWDB_rssi[i] != (0)) {
				if (pHalData->fw_ractrl == _TRUE) {	/* Report every sta's RSSI to FW */
					PWDB_rssi[i] |= (UL_DL_STATE << 24);
					rtl8812_set_rssi_cmd(rtlpriv, (u8 *)(&PWDB_rssi[i]));
				} else {
				}
			}
		}
	}



	if (tmpEntryMaxPWDB != 0) {	/* If associated entry is found */
		pdmpriv->EntryMaxUndecoratedSmoothedPWDB = tmpEntryMaxPWDB;
	} else {
		pdmpriv->EntryMaxUndecoratedSmoothedPWDB = 0;
	}

	if (tmpEntryMinPWDB != 0xff) {	/* If associated entry is found */
		pdmpriv->EntryMinUndecoratedSmoothedPWDB = tmpEntryMinPWDB;
	} else {
		pdmpriv->EntryMinUndecoratedSmoothedPWDB = 0;
	}

	FindMinimumRSSI(rtlpriv);	/* get pdmpriv->MinUndecoratedPWDBForDM */

	ODM_CmnInfoUpdate(&pHalData->odmpriv, ODM_CMNINFO_RSSI_MIN, pdmpriv->MinUndecoratedPWDBForDM);
}

static void odm_RSSIMonitorCheck(struct _rtw_dm *pDM_Odm)
{
	/*
	 * For AP/ADSL use prtl8192cd_priv
	 * For CE/NIC use _ADAPTER
	 */

	if (!(pDM_Odm->SupportAbility & ODM_BB_RSSI_MONITOR))
		return;

	/*
	 * 2011/09/29 MH In HW integration first stage, we provide 4 different handle to operate
	 * at the same time. In the stage2/3, we need to prive universal interface and merge all
	 * HW dynamic mechanism.
	 */

	odm_RSSIMonitorCheckCE(pDM_Odm);
}

/*
 * ============================================================
 * EDCA Turbo
 * ============================================================
 */

static void rtl8821au_dm_check_edca_turbo(struct rtl_priv *rtlpriv)
{
	struct rtl_hal *rtlhal =&(rtlpriv->rtlhal);
	struct _rtw_hal *pHalData = GET_HAL_DATA(rtlpriv);
	struct dm_priv	*pdmpriv = &pHalData->dmpriv;
	struct _rtw_dm *	pDM_Odm = &(pHalData->odmpriv);

	uint32_t	EDCA_BE_UL = 0x5ea42b;	/* Parameter suggested by Scott  */	/* edca_setting_UL[pMgntInfo->IOTPeer]; */
	uint32_t	EDCA_BE_DL = 0x5ea42b;	/* Parameter suggested by Scott  */	/* edca_setting_DL[pMgntInfo->IOTPeer]; */
	uint32_t	IOTPeer = 0;
	u8		WirelessMode = 0xFF;	/* invalid value */
	uint32_t 	trafficIndex;
	uint32_t	edca_param;
	u64		cur_tx_bytes = 0;
	u64		cur_rx_bytes = 0;
	u8		bbtchange = _FALSE;
	struct xmit_priv		*pxmitpriv = &(rtlpriv->xmitpriv);
	struct recv_priv		*precvpriv = &(rtlpriv->recvpriv);
	struct registry_priv	*pregpriv = &rtlpriv->registrypriv;
	struct mlme_ext_priv	*pmlmeext = &(rtlpriv->mlmeextpriv);
	struct mlme_ext_info	*pmlmeinfo = &(pmlmeext->mlmext_info);

	/*
	 * For AP/ADSL use prtl8192cd_priv
	 * For CE/NIC use _ADAPTER
	 */

	/*
	 *
	 * 2011/09/29 MH In HW integration first stage, we provide 4 different handle to operate
	 * at the same time. In the stage2/3, we need to prive universal interface and merge all
	 * HW dynamic mechanism.
	 */

	if (!(pDM_Odm->SupportAbility & ODM_MAC_EDCA_TURBO))
		return;

	if ((pregpriv->wifi_spec == 1)) {	/*|| (pmlmeinfo->HT_enable == 0)) */
		goto dm_CheckEdcaTurbo_EXIT;
	}

	if (pDM_Odm->pWirelessMode != NULL)
		WirelessMode = *(pDM_Odm->pWirelessMode);

	IOTPeer = pmlmeinfo->assoc_AP_vendor;

	if (IOTPeer >=  HT_IOT_PEER_MAX) {
		goto dm_CheckEdcaTurbo_EXIT;
	}

	/* Check if the status needs to be changed. */
	if ((bbtchange) || (!rtlpriv->dm.is_any_nonbepkts)) {
		cur_tx_bytes = pxmitpriv->tx_bytes - pxmitpriv->last_tx_bytes;
		cur_rx_bytes = precvpriv->rx_bytes - precvpriv->last_rx_bytes;

		/* traffic, TX or RX */
		if ((IOTPeer == HT_IOT_PEER_RALINK) || (IOTPeer == HT_IOT_PEER_ATHEROS)) {
			if (cur_tx_bytes > (cur_rx_bytes << 2)) {
				/* Uplink TP is present. */
				trafficIndex = UP_LINK;
			} else {
				/* Balance TP is present. */
				trafficIndex = DOWN_LINK;
			}
		} else {
			if (cur_rx_bytes > (cur_tx_bytes << 2)) {
				/* Downlink TP is present. */
				trafficIndex = DOWN_LINK;
			} else {
				/* Balance TP is present. */
				trafficIndex = UP_LINK;
			}
		}

		if ((pDM_Odm->DM_EDCA_Table.prv_traffic_idx != trafficIndex) || (!rtlpriv->dm.current_turbo_edca)) {
			/* merge from 92s_92c_merge temp brunch v2445    20120215 */
			if ((IOTPeer == HT_IOT_PEER_CISCO)
			   && ((WirelessMode == ODM_WM_G) || (WirelessMode == (ODM_WM_B|ODM_WM_G)) || (WirelessMode == ODM_WM_A) || (WirelessMode == ODM_WM_B))) {
				EDCA_BE_DL = edca_setting_DL_GMode[IOTPeer];
			} else if ((IOTPeer == HT_IOT_PEER_AIRGO)
			       && ((WirelessMode == ODM_WM_G) || (WirelessMode == ODM_WM_A))) {
					EDCA_BE_DL = 0xa630;
			} else if (IOTPeer == HT_IOT_PEER_MARVELL) {
				EDCA_BE_DL = edca_setting_DL[IOTPeer];
				EDCA_BE_UL = edca_setting_UL[IOTPeer];
			} else if (IOTPeer == HT_IOT_PEER_ATHEROS) {
				/* Set DL EDCA for Atheros peer to 0x3ea42b. Suggested by SD3 Wilson for ASUS TP issue. */
				EDCA_BE_DL = edca_setting_DL[IOTPeer];
			}

			if ((IS_HARDWARE_TYPE_8811AU(rtlhal))) {		/* add 8812AU/8812AE */
				EDCA_BE_UL = 0x5ea42b;
				EDCA_BE_DL = 0x5ea42b;
				ODM_RT_TRACE(pDM_Odm, ODM_COMP_EDCA_TURBO, ODM_DBG_LOUD, ("8812A: EDCA_BE_UL=0x%x EDCA_BE_DL =0x%x", EDCA_BE_UL, EDCA_BE_DL));
			}

			if (trafficIndex == DOWN_LINK)
				edca_param = EDCA_BE_DL;
			else
				edca_param = EDCA_BE_UL;

			rtl_write_dword(rtlpriv, REG_EDCA_BE_PARAM, edca_param);

			pDM_Odm->DM_EDCA_Table.prv_traffic_idx = trafficIndex;
		}

		rtlpriv->dm.current_turbo_edca = true;
	} else {
		/*
		 * Turn Off EDCA turbo here.
		 * Restore original EDCA according to the declaration of AP.
		 */
		if (rtlpriv->dm.current_turbo_edca) {
			rtl_write_dword(rtlpriv, REG_EDCA_BE_PARAM, pHalData->AcParam_BE);
			rtlpriv->dm.current_turbo_edca = false;
		}
	}

dm_CheckEdcaTurbo_EXIT:
	/* Set variables for next time. */
	rtlpriv->dm.is_any_nonbepkts = false;
	pxmitpriv->last_tx_bytes = pxmitpriv->tx_bytes;
	precvpriv->last_rx_bytes = precvpriv->rx_bytes;
}

static void dm_CheckPbcGPIO(struct rtl_priv *rtlpriv)
{
	struct rtl_hal *rtlhal = rtl_hal(rtlpriv);
	uint8_t	tmp1byte;
	uint8_t	bPbcPressed = _FALSE;

	if(!rtlpriv->registrypriv.hw_wps_pbc)
		return;

	if (IS_HARDWARE_TYPE_8812(rtlhal)) {
		tmp1byte = rtl_read_byte(rtlpriv, GPIO_IO_SEL);
		tmp1byte |= (HAL_8192C_HW_GPIO_WPS_BIT);
		rtl_write_byte(rtlpriv, GPIO_IO_SEL, tmp1byte);	/* enable GPIO[2] as output mode */

		tmp1byte &= ~(HAL_8192C_HW_GPIO_WPS_BIT);
		rtl_write_byte(rtlpriv,  GPIO_IN, tmp1byte);	/* reset the floating voltage level */

		tmp1byte = rtl_read_byte(rtlpriv, GPIO_IO_SEL);
		tmp1byte &= ~(HAL_8192C_HW_GPIO_WPS_BIT);
		rtl_write_byte(rtlpriv, GPIO_IO_SEL, tmp1byte);	/* enable GPIO[2] as input mode */

		tmp1byte =rtl_read_byte(rtlpriv, GPIO_IN);

		if (tmp1byte == 0xff)
			return ;

		if (tmp1byte&HAL_8192C_HW_GPIO_WPS_BIT) {
			bPbcPressed = _TRUE;
		}
	} else if (IS_HARDWARE_TYPE_8821(rtlhal)) {
		tmp1byte = rtl_read_byte(rtlpriv, GPIO_IO_SEL_8811A);
		tmp1byte |= (BIT4);
		rtl_write_byte(rtlpriv, GPIO_IO_SEL_8811A, tmp1byte);	/* enable GPIO[2] as output mode */

		tmp1byte &= ~(BIT4);
		rtl_write_byte(rtlpriv,  GPIO_IN_8811A, tmp1byte);		/* reset the floating voltage level */

		tmp1byte = rtl_read_byte(rtlpriv, GPIO_IO_SEL_8811A);
		tmp1byte &= ~(BIT4);
		rtl_write_byte(rtlpriv, GPIO_IO_SEL_8811A, tmp1byte);	/* enable GPIO[2] as input mode */

		tmp1byte =rtl_read_byte(rtlpriv, GPIO_IN_8811A);

		if (tmp1byte == 0xff)
			return ;

		if (tmp1byte&BIT4) {
			bPbcPressed = _TRUE;
		}
	}
	if( _TRUE == bPbcPressed) {
		/*
		 * Here we only set bPbcPressed to true
		 * After trigger PBC, the variable will be set to false
		 */
		DBG_8192C("CheckPbcGPIO - PBC is pressed\n");
	}
}



void rtl8821au_dm_watchdog(struct rtl_priv *rtlpriv)
{
	struct rtl_hal	*rtlhal = rtl_hal(rtlpriv);
	struct dig_t *pDM_DigTable = &(rtlpriv->dm_digtable);

	BOOLEAN		bFwCurrentInPSMode = _FALSE;
	BOOLEAN		bFwPSAwake = _TRUE;
	uint8_t hw_init_completed = _FALSE;
	
	struct _rtw_hal *pHalData = GET_HAL_DATA(rtlpriv);
	struct dm_priv	*pdmpriv = &pHalData->dmpriv;
	struct _rtw_dm *	pDM_Odm = &(pHalData->odmpriv);

	hw_init_completed = rtlpriv->hw_init_completed;

	if (hw_init_completed == _FALSE)
		goto skip_dm;

#ifdef CONFIG_LPS
	{
		bFwCurrentInPSMode = rtlpriv->pwrctrlpriv.bFwCurrentInPSMode;
		rtw_hal_get_hwreg(rtlpriv, HW_VAR_FWLPS_RF_ON, (uint8_t *)(&bFwPSAwake));
	}
#endif
	/* ODM */
	if (hw_init_completed == _TRUE) {
		uint8_t	bLinked=_FALSE;

		if(rtw_linked_check(rtlpriv))
			bLinked = _TRUE;


		ODM_CmnInfoUpdate(&pHalData->odmpriv ,ODM_CMNINFO_LINK, bLinked);
				
				
		
		
		/*
		 * 2011/09/20 MH This is the entry pointer for all team to execute HW out source DM.
		 * You can not add any dummy function here, be care, you can only use DM structure
		 * to perform any new ODM_DM.
		 */
	
		odm_CommonInfoSelfUpdate(pDM_Odm);
		rtl8821ae_dm_false_alarm_counter_statistics(rtlpriv);
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_DIG, ODM_DBG_LOUD, ("odm_DIG(): RSSI=0x%x\n", pDM_Odm->RSSI_Min));
	
		odm_RSSIMonitorCheck(pDM_Odm);
	
		rtl8821au_dm_dig(rtlpriv);
	
		odm_Adaptivity(pDM_Odm, pDM_DigTable->cur_igvalue);
	
		odm_CCKPacketDetectionThresh(pDM_Odm);
	
		if (*(pDM_Odm->pbPowerSaving) == TRUE)
			return;
	
		odm_RefreshRateAdaptiveMask(pDM_Odm);
		rtl8821au_dm_check_edca_turbo(rtlpriv);
	
		rtl8821au_check_tx_power_tracking_thermalmeter(pDM_Odm);
	
		if (IS_HARDWARE_TYPE_8821U(rtlhal)) {
			if (pDM_Odm->bLinked) {
				if ((*pDM_Odm->pChannel != pDM_Odm->preChannel) && (!*pDM_Odm->pbScanInProcess)) {
					pDM_Odm->preChannel = *pDM_Odm->pChannel;
					pDM_Odm->LinkedInterval = 0;
				}
	
				if (pDM_Odm->LinkedInterval < 3)
					pDM_Odm->LinkedInterval++;
	
				if (pDM_Odm->LinkedInterval == 2) {
					struct rtl_priv *	rtlpriv = pDM_Odm->rtlpriv;
	
					/*
					 * mark out IQK flow to prevent tx stuck. by Maddest 20130306
					 * void rtl8821au_phy_iq_calibrate(rtlpriv, FALSE);
					 */
				}
			} else
				pDM_Odm->LinkedInterval = 0;
		}
		pDM_Odm->PhyDbgInfo.NumQryBeaconPkt = 0;
	
		odm_dtc(pDM_Odm);
	}

skip_dm:

	/*
	 * Check GPIO to determine current RF on/off and Pbc status.
	 * Check Hardware Radio ON/OFF or not
	 */
	{
		/* temp removed */
		dm_CheckPbcGPIO(rtlpriv);
	}
	return;
}

static void rtl8821au_dm_dig(struct rtl_priv *rtlpriv)
{
	struct rtl_hal *rtlhal = rtl_hal(rtlpriv);
	struct dig_t *pDM_DigTable = &(rtlpriv->dm_digtable);
	
	struct _rtw_hal *pHalData = GET_HAL_DATA(rtlpriv);
	struct dm_priv	*pdmpriv = &pHalData->dmpriv;
	struct _rtw_dm *	pDM_Odm = &(pHalData->odmpriv);
	
	PFALSE_ALARM_STATISTICS		pFalseAlmCnt = &pDM_Odm->FalseAlmCnt;
	pRXHP_T						pRX_HP_Table  = &pDM_Odm->DM_RXHP_Table;
	u8						DIG_Dynamic_MIN;
	u8						DIG_MaxOfMin;
	BOOLEAN						FirstConnect, FirstDisConnect;
	u8						dm_dig_max, dm_dig_min, offset;
	u8						CurrentIGI = pDM_DigTable->cur_igvalue;

	ODM_RT_TRACE(pDM_Odm, ODM_COMP_DIG, ODM_DBG_LOUD, ("odm_DIG()==>\n"));
	/* if (!(pDM_Odm->SupportAbility & (ODM_BB_DIG|ODM_BB_FA_CNT))) */
	if ((!(pDM_Odm->SupportAbility&ODM_BB_DIG)) || (!(pDM_Odm->SupportAbility&ODM_BB_FA_CNT))) {
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_DIG, ODM_DBG_LOUD, ("odm_DIG() Return: SupportAbility ODM_BB_DIG or ODM_BB_FA_CNT is disabled\n"));
		return;
	}

	if (*(pDM_Odm->pbScanInProcess)) {
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_DIG, ODM_DBG_LOUD, ("odm_DIG() Return: In Scan Progress \n"));
		return;
	}

	/* add by Neil Chen to avoid PSD is processing */
	DIG_Dynamic_MIN = pDM_DigTable->dig_min_0;
	FirstConnect = (pDM_Odm->bLinked) && (pDM_DigTable->media_connect_0 == FALSE);
	FirstDisConnect = (!pDM_Odm->bLinked) && (pDM_DigTable->media_connect_0 == TRUE);


	/* 1 Boundary Decision */
	dm_dig_max = DM_DIG_MAX_NIC;

	if (IS_HARDWARE_TYPE_8821U(rtlhal))
		dm_dig_min = DM_DIG_MIN_NIC;
	else
		dm_dig_min = 0x1C;

	DIG_MaxOfMin = DM_DIG_MAX_AP;

	if (pDM_Odm->bLinked) {
		{
			/* 2 Modify DIG upper bound */
			/* 2013.03.19 Luke: Modified upper bound for Netgear rental house test */
			if (IS_HARDWARE_TYPE_8821U(rtlhal))
				offset = 20;
			else
				offset = 10;

			if ((pDM_Odm->RSSI_Min + offset) > dm_dig_max)
				pDM_DigTable->rx_gain_max = dm_dig_max;
			else if ((pDM_Odm->RSSI_Min + offset) < dm_dig_min)
				pDM_DigTable->rx_gain_max = dm_dig_min;
			else
				pDM_DigTable->rx_gain_max = pDM_Odm->RSSI_Min + offset;


			/* 2 Modify DIG lower bound */
			/*
			if ((pFalseAlmCnt->Cnt_all > 500)&&(DIG_Dynamic_MIN < 0x25))
				DIG_Dynamic_MIN++;
			else if (((pFalseAlmCnt->Cnt_all < 500)||(pDM_Odm->RSSI_Min < 8))&&(DIG_Dynamic_MIN > dm_dig_min))
				DIG_Dynamic_MIN--;
			*/
			if (pDM_Odm->bOneEntryOnly) {
				if (pDM_Odm->RSSI_Min < dm_dig_min)
					DIG_Dynamic_MIN = dm_dig_min;
				else if (pDM_Odm->RSSI_Min > DIG_MaxOfMin)
					DIG_Dynamic_MIN = DIG_MaxOfMin;
				else
					DIG_Dynamic_MIN = pDM_Odm->RSSI_Min;
				ODM_RT_TRACE(pDM_Odm, ODM_COMP_DIG, ODM_DBG_LOUD, ("odm_DIG() : bOneEntryOnly=TRUE,  DIG_Dynamic_MIN=0x%x\n", DIG_Dynamic_MIN));
				ODM_RT_TRACE(pDM_Odm, ODM_COMP_DIG, ODM_DBG_LOUD, ("odm_DIG() : pDM_Odm->RSSI_Min=%d\n", pDM_Odm->RSSI_Min));
			} else {
				/* 1 Lower Bound for 88E AntDiv */
				DIG_Dynamic_MIN = dm_dig_min;
			}
		}
	} else {
		pDM_DigTable->rx_gain_max = dm_dig_max;
		DIG_Dynamic_MIN = dm_dig_min;
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_DIG, ODM_DBG_LOUD, ("odm_DIG() : No Link\n"));
	}

	/* 1 Modify DIG lower bound, deal with abnorally large false alarm */
	if (pFalseAlmCnt->Cnt_all > 10000) {
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_DIG, ODM_DBG_LOUD, ("dm_DIG(): Abnornally false alarm case. \n"));

		if (pDM_DigTable->large_fa_hit != 3)
			pDM_DigTable->large_fa_hit++;
		if (pDM_DigTable->forbidden_igi < CurrentIGI) {			/* if (pDM_DigTable->ForbiddenIGI < pDM_DigTable->CurIGValue) */
			pDM_DigTable->forbidden_igi = (u8)CurrentIGI;	/* pDM_DigTable->ForbiddenIGI = pDM_DigTable->CurIGValue; */
			pDM_DigTable->large_fa_hit = 1;
		}

		if (pDM_DigTable->large_fa_hit >= 3) {
			if ((pDM_DigTable->forbidden_igi+1) > pDM_DigTable->rx_gain_max)
				pDM_DigTable->rx_gain_min = pDM_DigTable->rx_gain_max;
			else
				pDM_DigTable->rx_gain_min = (pDM_DigTable->forbidden_igi + 1);
			pDM_DigTable->recover_cnt = 3600; 	/* 3600=2hr */
		}

	} else {
		/* Recovery mechanism for IGI lower bound */
		if (pDM_DigTable->recover_cnt != 0)
			pDM_DigTable->recover_cnt--;
		else {
			if (pDM_DigTable->large_fa_hit < 3) {
				if ((pDM_DigTable->forbidden_igi-1) < DIG_Dynamic_MIN) {		/* DM_DIG_MIN)  */
					pDM_DigTable->forbidden_igi = DIG_Dynamic_MIN;		/* DM_DIG_MIN; */
					pDM_DigTable->rx_gain_min = DIG_Dynamic_MIN;	/* DM_DIG_MIN; */
					ODM_RT_TRACE(pDM_Odm, ODM_COMP_DIG, ODM_DBG_LOUD, ("odm_DIG(): Normal Case: At Lower Bound\n"));
				} else {
					pDM_DigTable->forbidden_igi--;
					pDM_DigTable->rx_gain_min = (pDM_DigTable->forbidden_igi+ 1);
					ODM_RT_TRACE(pDM_Odm, ODM_COMP_DIG, ODM_DBG_LOUD, ("odm_DIG(): Normal Case: Approach Lower Bound\n"));
				}
			} else {
				pDM_DigTable->large_fa_hit = 0;
			}
		}
	}
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_DIG, ODM_DBG_LOUD, ("odm_DIG(): pDM_DigTable->LargeFAHit=%d\n", pDM_DigTable->large_fa_hit));

	if ((pDM_Odm->PhyDbgInfo.NumQryBeaconPkt < 10))
		pDM_DigTable->rx_gain_min = dm_dig_min;

	if (pDM_DigTable->rx_gain_min > pDM_DigTable->rx_gain_max)
		pDM_DigTable->rx_gain_min = pDM_DigTable->rx_gain_max;

	/* 1 Adjust initial gain by false alarm */
	if (pDM_Odm->bLinked) {
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_DIG, ODM_DBG_LOUD, ("odm_DIG(): DIG AfterLink\n"));
		if (FirstConnect) {
			if (pDM_Odm->RSSI_Min <= DIG_MaxOfMin)
			    CurrentIGI = pDM_Odm->RSSI_Min;
			else
			    CurrentIGI = DIG_MaxOfMin;
			ODM_RT_TRACE(pDM_Odm,	ODM_COMP_DIG, ODM_DBG_LOUD, ("DIG: First Connect\n"));
		} else 	{
			/* FA for Combo IC--NeilChen--2012--09--28 */
			if (pFalseAlmCnt->Cnt_all > DM_DIG_FA_TH2)
				CurrentIGI = CurrentIGI + 4;	/* pDM_DigTable->CurIGValue = pDM_DigTable->PreIGValue+2; */
			else if (pFalseAlmCnt->Cnt_all > DM_DIG_FA_TH1)
				CurrentIGI = CurrentIGI + 2;	/* pDM_DigTable->CurIGValue = pDM_DigTable->PreIGValue+1; */
			else if (pFalseAlmCnt->Cnt_all < DM_DIG_FA_TH0)
				CurrentIGI = CurrentIGI - 2;	/* pDM_DigTable->CurIGValue =pDM_DigTable->PreIGValue-1; */

			if ((pDM_Odm->PhyDbgInfo.NumQryBeaconPkt < 10)
			 && (pFalseAlmCnt->Cnt_all < DM_DIG_FA_TH1))
				CurrentIGI = pDM_DigTable->rx_gain_min;
		}
	} else {
		/* CurrentIGI = pDM_DigTable->rx_gain_range_min; */	/* pDM_DigTable->CurIGValue = pDM_DigTable->rx_gain_range_min */
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_DIG, ODM_DBG_LOUD, ("odm_DIG(): DIG BeforeLink\n"));
		if (FirstDisConnect) {
			CurrentIGI = pDM_DigTable->rx_gain_min;
			ODM_RT_TRACE(pDM_Odm, ODM_COMP_DIG, ODM_DBG_LOUD, ("odm_DIG(): First DisConnect \n"));
		} else {
			/* 2012.03.30 LukeLee: enable DIG before link but with very high thresholds */
			if (pFalseAlmCnt->Cnt_all > 10000)
				CurrentIGI = CurrentIGI + 4;
			else if (pFalseAlmCnt->Cnt_all > 8000)
				CurrentIGI = CurrentIGI + 2;
			else if (pFalseAlmCnt->Cnt_all < 500)
				CurrentIGI = CurrentIGI - 2;
			ODM_RT_TRACE(pDM_Odm, ODM_COMP_DIG, ODM_DBG_LOUD, ("odm_DIG(): England DIG \n"));
		}
	}
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_DIG, ODM_DBG_LOUD, ("odm_DIG(): DIG End Adjust IGI\n"));
	/* 1 Check initial gain by upper/lower bound */

	if (CurrentIGI > pDM_DigTable->rx_gain_max)
		CurrentIGI = pDM_DigTable->rx_gain_max;
	if (CurrentIGI < pDM_DigTable->rx_gain_min)
		CurrentIGI = pDM_DigTable->rx_gain_min;

	if (pDM_Odm->SupportAbility & ODM_BB_ADAPTIVITY) {
		if (CurrentIGI > (pDM_Odm->IGI_target + 4))
			CurrentIGI = (u8)pDM_Odm->IGI_target + 4;
	}

	ODM_RT_TRACE(pDM_Odm, ODM_COMP_DIG, ODM_DBG_LOUD, ("odm_DIG(): rx_gain_range_max=0x%x, rx_gain_range_min=0x%x\n",
		pDM_DigTable->rx_gain_max, pDM_DigTable->rx_gain_min));
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_DIG, ODM_DBG_LOUD, ("odm_DIG(): TotalFA=%d\n", pFalseAlmCnt->Cnt_all));
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_DIG, ODM_DBG_LOUD, ("odm_DIG(): CurIGValue=0x%x\n", CurrentIGI));

	/* 2 High power RSSI threshold */

	{		/* BT is not using */
		ODM_Write_DIG(pDM_Odm, CurrentIGI);	/* ODM_Write_DIG(pDM_Odm, pDM_DigTable->CurIGValue); */
		pDM_DigTable->media_connect_0 = pDM_Odm->bLinked;
		pDM_DigTable->dig_min_0 = DIG_Dynamic_MIN;
	}
}

