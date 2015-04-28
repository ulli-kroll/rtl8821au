#include <rtl8812a_hal.h>
#include "phy.h"
#include "table.h"
#include "rf.h"
#include "reg.h"
/*
 * 1. BB register R/W API
 */

#undef ODM_RT_TRACE
#define ODM_RT_TRACE(x, ...)	do {} while (0);

#define READ_NEXT_PAIR(array_table, v1, v2, i) \
	do { \
		i += 2; \
		v1 = array_table[i]; \
		v2 = array_table[i+1]; \
	} while (0)


u32 rtl8821au_phy_query_bb_reg(struct rtl_priv *rtlpriv, uint32_t RegAddr, uint32_t BitMask)
{
	uint32_t ReturnValue = 0, OriginalValue, BitShift;

	/* DBG_871X("--->PHY_QueryBBReg8812(): RegAddr(%#x), BitMask(%#x)\n", RegAddr, BitMask); */


	OriginalValue = rtl_read_dword(rtlpriv, RegAddr);
	BitShift = PHY_CalculateBitShift(BitMask);
	ReturnValue = (OriginalValue & BitMask) >> BitShift;

	/* DBG_871X("BBR MASK=0x%x Addr[0x%x]=0x%x\n", BitMask, RegAddr, OriginalValue); */
	return ReturnValue;
}


void rtl8821au_phy_set_bb_reg(struct rtl_priv *rtlpriv, u32 RegAddr, u32 BitMask, u32 Data)
{
	uint32_t OriginalValue, BitShift;

	if (BitMask != bMaskDWord) {	/* if not "double word" write */
		OriginalValue = rtl_read_dword(rtlpriv, RegAddr);
		BitShift = PHY_CalculateBitShift(BitMask);
		Data = ((OriginalValue) & (~BitMask)) | (((Data << BitShift)) & BitMask);
	}

	rtl_write_dword(rtlpriv, RegAddr, Data);

	/* DBG_871X("BBW MASK=0x%x Addr[0x%x]=0x%x\n", BitMask, RegAddr, Data); */
}

static u32 phy_RFSerialRead(struct rtl_priv *rtlpriv, uint8_t eRFPath,
	uint32_t Offset)
{
	struct rtl_hal *rtlhal = rtl_hal(rtlpriv);
	struct bb_reg_def *pphyreg = &(rtlpriv->phy.phyreg_def[eRFPath]);
	
	uint32_t			retValue = 0;
	BOOLEAN				bIsPIMode = _FALSE;


	/*
	 * 2009/06/17 MH We can not execute IO for power save or other accident mode.
	 * if(RT_CANNOT_IO(rtlpriv)) {
	 * 	RT_DISP(FPHY, PHY_RFR, ("phy_RFSerialRead return all one\n"));
	 * 	return	0xFFFFFFFF;
	 * }
	 */

	/* <20120809, Kordan> CCA OFF(when entering), asked by James to avoid reading the wrong value. */
	/* <20120828, Kordan> Toggling CCA would affect RF 0x0, skip it! */
	if (Offset != 0x0 &&  !(IS_VENDOR_8812A_C_CUT(rtlpriv) || IS_HARDWARE_TYPE_8821(rtlhal)))
		rtl_set_bbreg(rtlpriv, rCCAonSec_Jaguar, 0x8, 1);

	Offset &= 0xff;

	if (eRFPath == RF90_PATH_A)
		bIsPIMode = (BOOLEAN)rtl_get_bbreg(rtlpriv, 0xC00, 0x4);
	else if (eRFPath == RF90_PATH_B)
		bIsPIMode = (BOOLEAN)rtl_get_bbreg(rtlpriv, 0xE00, 0x4);

	if (IS_VENDOR_8812A_TEST_CHIP(rtlpriv))
		rtl_set_bbreg(rtlpriv, pphyreg->rfhssi_para2, bMaskDWord, 0);

	rtl_set_bbreg(rtlpriv, pphyreg->rfhssi_para2, bHSSIRead_addr_Jaguar, Offset);

	if (IS_VENDOR_8812A_TEST_CHIP(rtlpriv) )
		rtl_set_bbreg(rtlpriv, pphyreg->rfhssi_para2, bMaskDWord, Offset|BIT8);

	if (IS_VENDOR_8812A_C_CUT(rtlpriv) || IS_HARDWARE_TYPE_8821(rtlhal))
		udelay(20);

	if (bIsPIMode) {
		retValue = rtl_get_bbreg(rtlpriv, pphyreg->rf_rbpi, rRead_data_Jaguar);
		/* DBG_871X("[PI mode] RFR-%d Addr[0x%x]=0x%x\n", eRFPath, pPhyReg->rfLSSIReadBackPi, retValue); */
	} else {
		retValue = rtl_get_bbreg(rtlpriv, pphyreg->rf_rb, rRead_data_Jaguar);
		/* DBG_871X("[SI mode] RFR-%d Addr[0x%x]=0x%x\n", eRFPath, pPhyReg->rfLSSIReadBack, retValue); */
	}

	/* <20120809, Kordan> CCA ON(when exiting), asked by James to avoid reading the wrong value. */
	/* <20120828, Kordan> Toggling CCA would affect RF 0x0, skip it! */
	if (Offset != 0x0 &&  ! (IS_VENDOR_8812A_C_CUT(rtlpriv) || IS_HARDWARE_TYPE_8821(rtlhal)))
		rtl_set_bbreg(rtlpriv, rCCAonSec_Jaguar, 0x8, 0);

	return retValue;
}

static void phy_RFSerialWrite(struct rtl_priv *rtlpriv, uint8_t eRFPath,
	uint32_t Offset, uint32_t Data)
{
	struct bb_reg_def *pphyreg = &(rtlpriv->phy.phyreg_def[eRFPath]);
	uint32_t		DataAndAddr = 0;
	struct _rtw_hal		*pHalData = GET_HAL_DATA(rtlpriv);

	/*
	 * 2009/06/17 MH We can not execute IO for power save or other accident mode.
	 * if(RT_CANNOT_IO(rtlpriv)) {
	 * 	RTPRINT(FPHY, PHY_RFW, ("phy_RFSerialWrite stop\n"));
	 * 	return;
	 * }
	 */

	Offset &= 0xff;

	// Shadow Update
	//PHY_RFShadowWrite(rtlpriv, eRFPath, Offset, Data);

	// Put write addr in [27:20]  and write data in [19:00]
	DataAndAddr = ((Offset<<20) | (Data&0x000fffff)) & 0x0fffffff;

	/*
	 * 3 <Note> This is a workaround for 8812A test chips.
	 * <20120427, Kordan> MAC first moves lower 16 bits and then upper 16 bits of a 32-bit data.
	 * BaseBand doesn't know the two actions is actually only one action to access 32-bit data,
	 * so that the lower 16 bits is overwritten by the upper 16 bits. (Asked by ynlin.)
	 * (Unfortunately, the protection mechanism has not been implemented in 8812A yet.)
	 * 2012/10/26 MH Revise V3236 Lanhsin check in, if we do not enable the function
	 * for 8821, then it can not scan.
	 */
	if ((!pHalData->bSupportUSB3) && (IS_TEST_CHIP(pHalData->VersionID))) {	/* USB 2.0 or older */
		/* if (IS_VENDOR_8812A_TEST_CHIP(rtlpriv) || IS_HARDWARE_TYPE_8821(rtlpriv) is) */
		{
			rtl_write_dword(rtlpriv, 0x1EC, DataAndAddr);
			if (eRFPath == RF90_PATH_A)
				rtl_write_dword(rtlpriv, 0x1E8, 0x4000F000|0xC90);
			else
				rtl_write_dword(rtlpriv, 0x1E8, 0x4000F000|0xE90);
		}
	} else {
		/* USB 3.0 */
		/* Write Operation */
		/* TODO: Dynamically determine whether using PI or SI to write RF registers. */
		rtl_set_bbreg(rtlpriv, pphyreg->rf3wire_offset, bMaskDWord, DataAndAddr);
		/* DBG_871X("RFW-%d Addr[0x%x]=0x%x\n", eRFPath, pPhyReg->rf3wireOffset, DataAndAddr); */
	}

}


void rtl8821au_phy_set_rf_reg(struct rtl_priv *rtlpriv, u32 eRFPath, u32 RegAddr, 
	u32 BitMask, u32 Data)
{
	if (BitMask == 0)
		return;

	/* RF data is 20 bits only */
	if (BitMask != bLSSIWrite_data_Jaguar) {
		uint32_t	Original_Value, BitShift;
		Original_Value = phy_RFSerialRead(rtlpriv, eRFPath, RegAddr);
		BitShift =  PHY_CalculateBitShift(BitMask);
		Data = ((Original_Value) & (~BitMask)) | (Data<< BitShift);
	}

	phy_RFSerialWrite(rtlpriv, eRFPath, RegAddr, Data);

}

u32 rtl8821au_phy_query_rf_reg(struct rtl_priv *rtlpriv, u32 eRFPath, u32 RegAddr, 
	u32 BitMask)
{
	u32 Original_Value, Readback_Value, BitShift;

	Original_Value = phy_RFSerialRead(rtlpriv, eRFPath, RegAddr);

	BitShift =  PHY_CalculateBitShift(BitMask);
	Readback_Value = (Original_Value & BitMask) >> BitShift;

	return (Readback_Value);
}

/* ****************************************************************************** */
/*									*/
/*  from HalPhyRf_8812A.c						*/
/*									*/
/* ****************************************************************************** */

static void _rtl8812au_iqk_rx_fill_iqc(struct rtl_priv *rtlpriv, enum radio_path Path,
	unsigned int RX_X, unsigned int RX_Y)
{
	switch (Path) {
	case RF90_PATH_A:
		rtl_set_bbreg(rtlpriv, 0x82c, BIT(31), 0x0); /* [31] = 0 --> Page C */
		if (RX_X>>1 == 0x112 || RX_Y>>1 == 0x3ee) {
			rtl_set_bbreg(rtlpriv, 0xc10, 0x000003ff, 0x100);
			rtl_set_bbreg(rtlpriv, 0xc10, 0x03ff0000, 0);
			/* ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("RX_X = %x;;RX_Y = %x ====>fill to IQC\n", RX_X>>1&0x000003ff, RX_Y>>1&0x000003ff)); */
		} else {
			rtl_set_bbreg(rtlpriv, 0xc10, 0x000003ff, RX_X>>1);
			rtl_set_bbreg(rtlpriv, 0xc10, 0x03ff0000, RX_Y>>1);
			/* ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("RX_X = %x;;RX_Y = %x ====>fill to IQC\n", RX_X>>1&0x000003ff, RX_Y>>1&0x000003ff)); */
			/* ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("0xc10 = %x ====>fill to IQC\n", ODM_Read4Byte(pDM_Odm, 0xc10))); */
		}
		break;
	case RF90_PATH_B:
			rtl_set_bbreg(rtlpriv, 0x82c, BIT(31), 0x0); /*  [31] = 0 --> Page C */
			if (RX_X>>1 == 0x112 || RX_Y>>1 == 0x3ee) {
				rtl_set_bbreg(rtlpriv, 0xe10, 0x000003ff, 0x100);
				rtl_set_bbreg(rtlpriv, 0xe10, 0x03ff0000, 0);
				/* ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("RX_X = %x;;RX_Y = %x ====>fill to IQC\n", RX_X>>1&0x000003ff, RX_Y>>1&0x000003ff)); */
			} else {
				rtl_set_bbreg(rtlpriv, 0xe10, 0x000003ff, RX_X>>1);
				rtl_set_bbreg(rtlpriv, 0xe10, 0x03ff0000, RX_Y>>1);
				/* ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("RX_X = %x;;RX_Y = %x====>fill to IQC\n ", RX_X>>1&0x000003ff, RX_Y>>1&0x000003ff)); */
				/* ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("0xe10 = %x====>fill to IQC\n", ODM_Read4Byte(pDM_Odm, 0xe10))); */
			}
		break;
	default:
		break;
	};
}

static void _rtl8821au_iqk_rx_fill_iqc(struct rtl_priv *rtlpriv, enum radio_path Path,
	unsigned int RX_X, unsigned int RX_Y)
{
	switch (Path) {
	case RF90_PATH_A:
		rtl_set_bbreg(rtlpriv, 0x82c, BIT(31), 0x0); /* [31] = 0 --> Page C */
		rtl_set_bbreg(rtlpriv, 0xc10, 0x000003ff, RX_X>>1);
		rtl_set_bbreg(rtlpriv, 0xc10, 0x03ff0000, RX_Y>>1);
		/* ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("RX_X = %x;;RX_Y = %x ====>fill to IQC\n", RX_X>>1, RX_Y>>1)); */
		/* ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("0xc10 = %x ====>fill to IQC\n", ODM_Read4Byte(pDM_Odm, 0xc10))); */
		break;
	default:
		break;
	};
}

static void _rtl8812au_iqk_tx_fill_iqc(struct rtl_priv *rtlpriv, enum radio_path  Path,
	unsigned int TX_X, unsigned int TX_Y)
{
	switch (Path) {
	case RF90_PATH_A:
		rtl_set_bbreg(rtlpriv, 0x82c, BIT(31), 0x1); /* [31] = 1 --> Page C1 */
		rtl_write_dword(rtlpriv, 0xc90, 0x00000080);
		rtl_write_dword(rtlpriv, 0xcc4, 0x20040000);
		rtl_write_dword(rtlpriv, 0xcc8, 0x20000000);
		rtl_set_bbreg(rtlpriv, 0xccc, 0x000007ff, TX_Y);
		rtl_set_bbreg(rtlpriv, 0xcd4, 0x000007ff, TX_X);
		/* ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("TX_X = %x;;TX_Y = %x =====> fill to IQC\n", TX_X&0x000007ff, TX_Y&0x000007ff)); */
		/* ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("0xcd4 = %x;;0xccc = %x ====>fill to IQC\n", rtl_get_bbreg(pDM_Odm->rtlpriv, 0xcd4, 0x000007ff), rtl_get_bbreg(pDM_Odm->rtlpriv, 0xccc, 0x000007ff))); */
		break;
	case RF90_PATH_B:
		rtl_set_bbreg(rtlpriv, 0x82c, BIT(31), 0x1); /* [31] = 1 --> Page C1 */
		rtl_write_dword(rtlpriv, 0xe90, 0x00000080);
		rtl_write_dword(rtlpriv, 0xec4, 0x20040000);
		rtl_write_dword(rtlpriv, 0xec8, 0x20000000);
		rtl_set_bbreg(rtlpriv, 0xecc, 0x000007ff, TX_Y);
		rtl_set_bbreg(rtlpriv, 0xed4, 0x000007ff, TX_X);
		/* ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("TX_X = %x;;TX_Y = %x =====> fill to IQC\n", TX_X&0x000007ff, TX_Y&0x000007ff)); */
		/* ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("0xed4 = %x;;0xecc = %x ====>fill to IQC\n", rtl_get_bbreg(pDM_Odm->rtlpriv, 0xed4, 0x000007ff), rtl_get_bbreg(pDM_Odm->rtlpriv, 0xecc, 0x000007ff))); */
		break;
	default:
		break;
	};
}

static void _rtl8821au_iqk_tx_fill_iqc(struct rtl_priv *rtlpriv, enum radio_path Path,
	unsigned int TX_X, unsigned int TX_Y)
{
	switch (Path) {
	case RF90_PATH_A:
		rtl_set_bbreg(rtlpriv, 0x82c, BIT(31), 0x1); /* [31] = 1 --> Page C1 */
		rtl_write_dword(rtlpriv, 0xc90, 0x00000080);
		rtl_write_dword(rtlpriv, 0xcc4, 0x20040000);
		rtl_write_dword(rtlpriv, 0xcc8, 0x20000000);
		rtl_set_bbreg(rtlpriv, 0xccc, 0x000007ff, TX_Y);
		rtl_set_bbreg(rtlpriv, 0xcd4, 0x000007ff, TX_X);
		/* ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("TX_X = %x;;TX_Y = %x =====> fill to IQC\n", TX_X, TX_Y)); */
		/* ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("0xcd4 = %x;;0xccc = %x ====>fill to IQC\n", rtl_get_bbreg(pDM_Odm->rtlpriv, 0xcd4, 0x000007ff), rtl_get_bbreg(pDM_Odm->rtlpriv, 0xccc, 0x000007ff))); */
		break;
	default:
		break;
	};
}

#define cal_num 3

/* ULLI this function needs a complete rewrite (or we cantake code form rtlwifi-lib */

static void _rtl8812au_iqk_tx(struct rtl_priv *rtlpriv, enum radio_path Path)
{
	struct rtl_hal	*rtlhal = rtl_hal(rtlpriv);

	uint32_t 	TX_fail, RX_fail, delay_count, IQK_ready, cal_retry, cal = 0, temp_reg65;
	int		TX_X = 0, TX_Y = 0, RX_X = 0, RX_Y = 0, TX_Average = 0, RX_Average = 0;
	int 		TX_X0[cal_num], TX_Y0[cal_num], RX_X0[cal_num], RX_Y0[cal_num];
	BOOLEAN 	TX0IQKOK = FALSE, RX0IQKOK = FALSE;
	int 		TX_X1[cal_num], TX_Y1[cal_num], RX_X1[cal_num], RX_Y1[cal_num];
	BOOLEAN  	TX1IQKOK = FALSE, RX1IQKOK = FALSE, VDF_enable = FALSE;
	int 			i, k, VDF_Y[3], VDF_X[3], Tx_dt[3], Rx_dt[3], ii, dx = 0, dy = 0, TX_finish = 0, RX_finish = 0, dt = 0;

	ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("BandWidth = %d, ExtPA5G = %d, ExtPA2G = %d\n", rtlpriv->phy.current_chan_bw, rtlhal->external_pa_5g, rtlhal->external_pa_2g));
	if (rtlpriv->phy.current_chan_bw == 2) {
		VDF_enable = TRUE;
	}
	VDF_enable = FALSE;
	temp_reg65 = rtw_hal_read_rfreg(rtlpriv, Path, 0x65, bMaskDWord);

	switch (Path) {
	case RF90_PATH_A:
		rtl_set_bbreg(rtlpriv, 0x82c, BIT(31), 0x0);	/* [31] = 0 --> Page C */
		/*  ========Path-A AFE all on======== */
		/* Port 0 DAC/ADC on */
		rtl_write_dword(rtlpriv, 0xc60, 0x77777777);
		rtl_write_dword(rtlpriv, 0xc64, 0x77777777);

		/* Port 1 DAC/ADC off */
		rtl_write_dword(rtlpriv, 0xe60, 0x00000000);
		rtl_write_dword(rtlpriv, 0xe64, 0x00000000);

		rtl_write_dword(rtlpriv, 0xc68, 0x19791979);

		rtl_set_bbreg(rtlpriv, 0xc00, 0xf, 0x4);		/* hardware 3-wire off */

		/* DAC/ADC sampling rate (160 MHz) */
		rtl_set_bbreg(rtlpriv, 0xc5c, BIT(26)|BIT(25)|BIT(24), 0x7);
		rtl_set_bbreg(rtlpriv, 0x8c4, BIT(30), 0x1);
		/* rtl_set_bbreg(rtlpriv, 0xcb0, 0x00ff0000, 0x77); */
		/* rtl_set_bbreg(rtlpriv, 0xcb4, 0x03000000, 0x0); */
		break;
	case RF90_PATH_B:
		rtl_set_bbreg(rtlpriv, 0x82c, BIT(31), 0x0); /* [31] = 0 --> Page C */
		/* ========Path-B AFE all on======== */
		/* Port 0 DAC/ADC off */
		rtl_write_dword(rtlpriv, 0xc60, 0x00000000);
		rtl_write_dword(rtlpriv, 0xc64, 0x00000000);

		/* Port 1 DAC/ADC on */
		rtl_write_dword(rtlpriv, 0xe60, 0x77777777);
		rtl_write_dword(rtlpriv, 0xe64, 0x77777777);

		rtl_write_dword(rtlpriv, 0xe68, 0x19791979);

		rtl_set_bbreg(rtlpriv, 0xe00, 0xf, 0x4);		/* hardware 3-wire off */

		/* DAC/ADC sampling rate (160 MHz) */
		rtl_set_bbreg(rtlpriv, 0xe5c, BIT(26)|BIT(25)|BIT(24), 0x7);
		rtl_set_bbreg(rtlpriv, 0x8c4, BIT(30), 0x1);
		/* rtl_set_bbreg(rtlpriv, 0xeb0, 0x00ff0000, 0x77); */
		/* rtl_set_bbreg(rtlpriv, 0xeb4, 0x03000000, 0x0); */
		break;
	default:
		break;
	}

	switch (Path) {
	case RF90_PATH_A:
	    {
		/* ====== TX IQK ====== */
		rtl_set_bbreg(rtlpriv, 0x82c, BIT(31), 0x0); /*  [31] = 0 --> Page C */
		rtw_hal_write_rfreg(rtlpriv, Path, 0xef, bRFRegOffsetMask, 0x80002);
		rtw_hal_write_rfreg(rtlpriv, Path, 0x30, bRFRegOffsetMask, 0x20000);
		rtw_hal_write_rfreg(rtlpriv, Path, 0x31, bRFRegOffsetMask, 0x3fffd);
		rtw_hal_write_rfreg(rtlpriv, Path, 0x32, bRFRegOffsetMask, 0xfe83f);
		rtw_hal_write_rfreg(rtlpriv, Path, 0x65, bRFRegOffsetMask, 0x931d5);
		rtw_hal_write_rfreg(rtlpriv, Path, 0x8f, bRFRegOffsetMask, 0x8a001);
		rtl_write_dword(rtlpriv, 0x90c, 0x00008000);
		rtl_write_dword(rtlpriv, 0xb00, 0x03000100);
		rtl_set_bbreg(rtlpriv, 0xc94, BIT(0), 0x1);
		rtl_write_dword(rtlpriv, 0x978, 0x29002000);	/* TX (X,Y) */
		rtl_write_dword(rtlpriv, 0x97c, 0xa9002000);	/* RX (X,Y) */
		rtl_write_dword(rtlpriv, 0x984, 0x00462910);	/* [0]:AGC_en, [15]:idac_K_Mask */

		rtl_set_bbreg(rtlpriv, 0x82c, BIT(31), 0x1);	/* [31] = 1 --> Page C1 */

		if (rtlhal->external_pa_5g)
			rtl_write_dword(rtlpriv, 0xc88, 0x821403f7);
		else
			rtl_write_dword(rtlpriv, 0xc88, 0x821403f1);

		if (rtlhal->current_bandtype)
			rtl_write_dword(rtlpriv, 0xc8c, 0x68163e96);
		else {
			rtl_write_dword(rtlpriv, 0xc8c, 0x28163e96);
			if (rtlhal->rfe_type == 3) {
				if (rtlhal->external_pa_2g)
					rtl_write_dword(rtlpriv, 0xc88, 0x821403e3);
				else
					rtl_write_dword(rtlpriv, 0xc88, 0x821403f7);
			}

		}

		if (VDF_enable == 1) {
			ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("TXVDF Start\n"));
			for (k = 0; k <= 2; k++) {
				switch (k) {
				case 0:
					rtl_write_dword(rtlpriv, 0xc80, 0x18008c38);	/* TX_Tone_idx[9:0], TxK_Mask[29] TX_Tone = 16 */
					rtl_write_dword(rtlpriv, 0xc84, 0x38008c38);	/* RX_Tone_idx[9:0], RxK_Mask[29] */
					rtl_write_dword(rtlpriv, 0x984, 0x00462910);	/* [0]:AGC_en, [15]:idac_K_Mask */
					rtl_set_bbreg(rtlpriv, 0xce8, BIT(31), 0x0);
					break;
				case 1:
					rtl_set_bbreg(rtlpriv, 0xc80, BIT(28), 0x0);
					rtl_set_bbreg(rtlpriv, 0xc84, BIT(28), 0x0);
					rtl_write_dword(rtlpriv, 0x984, 0x0046a910);	/* [0]:AGC_en, [15]:idac_K_Mask */
					break;
				case 2:
					ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("VDF_Y[1] = %x;;;VDF_Y[0] = %x\n", VDF_Y[1]>>21 & 0x00007ff, VDF_Y[0]>>21 & 0x00007ff));
					ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("VDF_X[1] = %x;;;VDF_X[0] = %x\n", VDF_X[1]>>21 & 0x00007ff, VDF_X[0]>>21 & 0x00007ff));
					Tx_dt[cal] = (VDF_Y[1]>>20)-(VDF_Y[0]>>20);
					ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("Tx_dt = %d\n", Tx_dt[cal]));
					Tx_dt[cal] = ((16*Tx_dt[cal])*10000/15708);
					Tx_dt[cal] = (Tx_dt[cal] >> 1) + (Tx_dt[cal] & BIT(0));
					rtl_write_dword(rtlpriv, 0xc80, 0x18008c20);	/* TX_Tone_idx[9:0], TxK_Mask[29] TX_Tone = 16 */
					rtl_write_dword(rtlpriv, 0xc84, 0x38008c20);	/* RX_Tone_idx[9:0], RxK_Mask[29] */
					rtl_set_bbreg(rtlpriv, 0xce8, BIT(31), 0x1);
					rtl_set_bbreg(rtlpriv, 0xce8, 0x3fff0000, Tx_dt[cal] & 0x00003fff);
					break;
				default:
					break;
				}
				rtl_write_dword(rtlpriv, 0xcb8, 0x00100000);		/* cb8[20] ±N SI/PI ¨Ï¥ÎÅv¤Áµ¹ iqk_dpk module */
				cal_retry = 0;
				while (1) {
					/* one shot */
					rtl_write_dword(rtlpriv, 0x980, 0xfa000000);
					rtl_write_dword(rtlpriv, 0x980, 0xf8000000);

					mdelay(10); /* Delay 10ms */
					rtl_write_dword(rtlpriv, 0xcb8, 0x00000000);
					delay_count = 0;
					while (1) {
						IQK_ready = rtl_get_bbreg(rtlpriv, 0xd00, BIT(10));
						if ((IQK_ready) || (delay_count > 20)) {
							break;
						} else {
							mdelay(1);
							delay_count++;
						}
					}

					if (delay_count < 20) {							/* If 20ms No Result, then cal_retry++ */
						/* ============TXIQK Check============== */
						TX_fail = rtl_get_bbreg(rtlpriv, 0xd00, BIT(12));

						if (~TX_fail) {
							rtl_write_dword(rtlpriv, 0xcb8, 0x02000000);
							VDF_X[k] = rtl_get_bbreg(rtlpriv, 0xd00, 0x07ff0000)<<21;
							rtl_write_dword(rtlpriv, 0xcb8, 0x04000000);
							VDF_Y[k] = rtl_get_bbreg(rtlpriv, 0xd00, 0x07ff0000)<<21;
							TX0IQKOK = TRUE;
							break;
						} else {
							TX0IQKOK = FALSE;
							cal_retry++;
							if (cal_retry == 10) {
								break;
							}
						}
					} else {
						TX0IQKOK = FALSE;
						cal_retry++;
						if (cal_retry == 10) {
							break;
						}
					}
				}
			}
			ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("TXA_VDF_cal_retry = %d\n", cal_retry));
			TX_X0[cal] = VDF_X[k-1] ;
			TX_Y0[cal] = VDF_Y[k-1];
		} else {
			rtl_write_dword(rtlpriv, 0xc80, 0x18008c10);	/* TX_Tone_idx[9:0], TxK_Mask[29] TX_Tone = 16 */
			rtl_write_dword(rtlpriv, 0xc84, 0x38008c10);	/* RX_Tone_idx[9:0], RxK_Mask[29] */
			rtl_write_dword(rtlpriv, 0xce8, 0x00000000);

			for (cal = 0; cal < cal_num; cal++) {
				cal_retry = 0;
				while (1) {
					/* one shot */
					rtl_write_dword(rtlpriv, 0xcb8, 0x00100000);	/* cb8[20] ±N SI/PI ¨Ï¥ÎÅv¤Áµ¹ iqk_dpk module */
					rtl_write_dword(rtlpriv, 0x980, 0xfa000000);
					rtl_write_dword(rtlpriv, 0x980, 0xf8000000);

					mdelay(10); 				/* Delay 25ms */
					rtl_write_dword(rtlpriv, 0xcb8, 0x00000000);
					delay_count = 0;
					while (1) {
						IQK_ready = rtl_get_bbreg(rtlpriv, 0xd00, BIT(10));
						if ((IQK_ready) || (delay_count > 20)) {
							break;
						} else {
							mdelay(1);
							delay_count++;
						}
					}

					if (delay_count < 20) {				/* If 20ms No Result, then cal_retry++ */
						/* ============TXIQK Check============== */
						TX_fail = rtl_get_bbreg(rtlpriv, 0xd00, BIT(12));

						if (~TX_fail) {
							rtl_write_dword(rtlpriv, 0xcb8, 0x02000000);
							TX_X0[cal] = rtl_get_bbreg(rtlpriv, 0xd00, 0x07ff0000)<<21;
							rtl_write_dword(rtlpriv, 0xcb8, 0x04000000);
							TX_Y0[cal] = rtl_get_bbreg(rtlpriv, 0xd00, 0x07ff0000)<<21;
							TX0IQKOK = TRUE;
							/*
							rtl_write_dword(rtlpriv, 0xcb8, 0x01000000);
							reg1 = rtl_get_bbreg(rtlpriv, 0xd00, 0xffffffff);
							rtl_write_dword(rtlpriv, 0xcb8, 0x02000000);
							reg2 = rtl_get_bbreg(rtlpriv, 0xd00, 0x0000001f);
							Image_Power = (reg2<<32)+reg1;
							DbgPrint("Before PW = %d\n", Image_Power);
							rtl_write_dword(rtlpriv, 0xcb8, 0x03000000);
							reg1 = rtl_get_bbreg(rtlpriv, 0xd00, 0xffffffff);
							rtl_write_dword(rtlpriv, 0xcb8, 0x04000000);
							reg2 = rtl_get_bbreg(rtlpriv, 0xd00, 0x0000001f);
							Image_Power = (reg2<<32)+reg1;
							DbgPrint("After PW = %d\n", Image_Power);
							*/
							break;
						} else {
							TX0IQKOK = FALSE;
							cal_retry++;
							if (cal_retry == 10) {
								break;
							}
						}
					} else {
						TX0IQKOK = FALSE;
						cal_retry++;
						if (cal_retry == 10)
							break;
					}
				}
				ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("TXA_cal_retry = %d\n", cal_retry));
				if (TX0IQKOK)
					TX_Average++;
			}
		}

		rtl_set_bbreg(rtlpriv, 0x82c, BIT(31), 0x0); /* [31] = 0 --> Page C */
		rtw_hal_write_rfreg(rtlpriv, Path, 0x58, 0x7fe00, rtw_hal_read_rfreg(rtlpriv, Path, 0x8, 0xffc00)); /* Load LOK */
		rtl_set_bbreg(rtlpriv, 0x82c, BIT(31), 0x1); /* [31] = 1 --> Page C1 */

		if (TX0IQKOK == FALSE)
			break;				/* TXK fail, Don't do RXK */

		if (VDF_enable == 1) {
			rtl_set_bbreg(rtlpriv, 0xce8, BIT(31), 0x0);    /*  TX VDF Disable */
			ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("RXVDF Start\n"));

			/* ====== RX IQK ====== */
			rtl_set_bbreg(rtlpriv, 0x82c, BIT(31), 0x0); /* [31] = 0 --> Page C */
			rtw_hal_write_rfreg(rtlpriv, Path, 0xef, bRFRegOffsetMask, 0x80000);
			rtw_hal_write_rfreg(rtlpriv, Path, 0x30, bRFRegOffsetMask, 0x30000);
			rtw_hal_write_rfreg(rtlpriv, Path, 0x31, bRFRegOffsetMask, 0x3f7ff);
			rtw_hal_write_rfreg(rtlpriv, Path, 0x32, bRFRegOffsetMask, 0xfe7bf);
			rtw_hal_write_rfreg(rtlpriv, Path, 0x8f, bRFRegOffsetMask, 0x88001);
			rtw_hal_write_rfreg(rtlpriv, Path, 0x65, bRFRegOffsetMask, 0x931d0);
			rtw_hal_write_rfreg(rtlpriv, Path, 0xef, bRFRegOffsetMask, 0x00000);
			rtl_set_bbreg(rtlpriv, 0x978, BIT(31), 0x1);
			rtl_set_bbreg(rtlpriv, 0x97c, BIT(31), 0x0);
			rtl_write_dword(rtlpriv, 0x984, 0x0046a911);

			rtl_set_bbreg(rtlpriv, 0x82c, BIT(31), 0x1); /* [31] = 1 --> Page C1 */
			rtl_write_dword(rtlpriv, 0xc88, 0x02140119);
			rtl_write_dword(rtlpriv, 0xc8c, 0x28161420);

			for (k = 0; k <= 2; k++) {
				rtl_set_bbreg(rtlpriv, 0x82c, BIT(31), 0x0); 	/* [31] = 0 --> Page C */
				rtl_set_bbreg(rtlpriv, 0x978, 0x03FF8000, (VDF_X[k])>>21&0x000007ff);
				rtl_set_bbreg(rtlpriv, 0x978, 0x000007FF, (VDF_Y[k])>>21&0x000007ff);

				rtl_set_bbreg(rtlpriv, 0x82c, BIT(31), 0x1); 	/* [31] = 1 --> Page C1 */
				switch (k) {
				case 0:
					rtl_write_dword(rtlpriv, 0xc80, 0x38008c38);	/* TX_Tone_idx[9:0], TxK_Mask[29] TX_Tone = 16 */
					rtl_write_dword(rtlpriv, 0xc84, 0x18008c38);	/* RX_Tone_idx[9:0], RxK_Mask[29] */
					rtl_set_bbreg(rtlpriv, 0xce8, BIT(30), 0x0);
					break;
				case 1:
					rtl_write_dword(rtlpriv, 0xc80, 0x28008c38);	/* TX_Tone_idx[9:0], TxK_Mask[29] TX_Tone = 16 */
					rtl_write_dword(rtlpriv, 0xc84, 0x08008c38);	/* RX_Tone_idx[9:0], RxK_Mask[29] */
					break;
				case 2:
					ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("VDF_Y[1] = %x;;;VDF_Y[0] = %x\n", VDF_Y[1]>>21 & 0x00007ff, VDF_Y[0]>>21 & 0x00007ff));
					ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("VDF_X[1] = %x;;;VDF_X[0] = %x\n", VDF_X[1]>>21 & 0x00007ff, VDF_X[0]>>21 & 0x00007ff));
					Rx_dt[cal] = (VDF_Y[1]>>20)-(VDF_Y[0]>>20);
					ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("Rx_dt = %d\n", Rx_dt[cal]));
					Rx_dt[cal] = ((16*Rx_dt[cal])*10000/13823);
					Rx_dt[cal] = (Rx_dt[cal] >> 1) + (Rx_dt[cal] & BIT(0));
					rtl_write_dword(rtlpriv, 0xc80, 0x38008c20);	/* TX_Tone_idx[9:0], TxK_Mask[29] TX_Tone = 16 */
					rtl_write_dword(rtlpriv, 0xc84, 0x18008c20);	/* RX_Tone_idx[9:0], RxK_Mask[29] */
					rtl_set_bbreg(rtlpriv, 0xce8, 0x00003fff, Rx_dt[cal] & 0x00003fff);
					break;
				default:
					break;
				}


				if (k == 2) {
					rtl_set_bbreg(rtlpriv, 0xce8, BIT(30), 0x1);  /* RX VDF Enable */
				}
				rtl_write_dword(rtlpriv, 0xcb8, 0x00100000);/* cb8[20] ±N SI/PI ¨Ï¥ÎÅv¤Áµ¹ iqk_dpk module */

				cal_retry = 0;
				while (1) {
					/* one shot */
					rtl_write_dword(rtlpriv, 0x980, 0xfa000000);
					rtl_write_dword(rtlpriv, 0x980, 0xf8000000);

					mdelay(10); /* Delay 10ms */
					rtl_write_dword(rtlpriv, 0xcb8, 0x00000000);
					delay_count = 0;

					while (1) {
						IQK_ready = rtl_get_bbreg(rtlpriv, 0xd00, BIT(10));
						if ((IQK_ready) || (delay_count > 20)) {
							break;
						} else {
							mdelay(1);
							delay_count++;
						}
					}

					if (delay_count < 20) {	/* If 20ms No Result, then cal_retry++ */
						/* ============RXIQK Check============== */
						RX_fail = rtl_get_bbreg(rtlpriv, 0xd00, BIT(11));
						if (RX_fail == 0) {
							rtl_write_dword(rtlpriv, 0xcb8, 0x06000000);
							VDF_X[k] = rtl_get_bbreg(rtlpriv, 0xd00, 0x07ff0000)<<21;
							rtl_write_dword(rtlpriv, 0xcb8, 0x08000000);
							VDF_Y[k] = rtl_get_bbreg(rtlpriv, 0xd00, 0x07ff0000)<<21;
							RX0IQKOK = TRUE;
							break;
						} else {
							rtl_set_bbreg(rtlpriv, 0xc10, 0x000003ff, 0x200>>1);
							rtl_set_bbreg(rtlpriv, 0xc10, 0x03ff0000, 0x0>>1);
							RX0IQKOK = FALSE;
							cal_retry++;
							if (cal_retry == 10)
								break;
						}
					} else {
						RX0IQKOK = FALSE;
						cal_retry++;
						if (cal_retry == 10)
							break;
					}
				}
			}
			ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("RXA_VDF_cal_retry = %d\n", cal_retry));
			RX_X0[cal] = VDF_X[k-1] ;
			RX_Y0[cal] = VDF_Y[k-1];
			rtl_set_bbreg(rtlpriv, 0xce8, BIT(31), 0x1);    /* TX VDF Enable */
		} else {
			/* ====== RX IQK ====== */
			rtl_set_bbreg(rtlpriv, 0x82c, BIT(31), 0x0); /* [31] = 0 --> Page C */
			/*  1. RX RF Setting */
			rtw_hal_write_rfreg(rtlpriv, Path, 0xef, bRFRegOffsetMask, 0x80000);
			rtw_hal_write_rfreg(rtlpriv, Path, 0x30, bRFRegOffsetMask, 0x30000);
			rtw_hal_write_rfreg(rtlpriv, Path, 0x31, bRFRegOffsetMask, 0x3f7ff);
			rtw_hal_write_rfreg(rtlpriv, Path, 0x32, bRFRegOffsetMask, 0xfe7bf);
			rtw_hal_write_rfreg(rtlpriv, Path, 0x8f, bRFRegOffsetMask, 0x88001);
			rtw_hal_write_rfreg(rtlpriv, Path, 0x65, bRFRegOffsetMask, 0x931d0);
			rtw_hal_write_rfreg(rtlpriv, Path, 0xef, bRFRegOffsetMask, 0x00000);

			rtl_set_bbreg(rtlpriv, 0x978, BIT(31), 0x1);
			rtl_set_bbreg(rtlpriv, 0x97c, BIT(31), 0x0);
			rtl_write_dword(rtlpriv, 0x90c, 0x00008000);
			/* rtl_write_dword(rtlpriv, 0x984, 0x0046a911); */
			rtl_write_dword(rtlpriv, 0x984, 0x0046a891);

			rtl_set_bbreg(rtlpriv, 0x82c, BIT(31), 0x1); 	/* [31] = 1 --> Page C1 */
			rtl_write_dword(rtlpriv, 0xc80, 0x38008c10);	/* TX_Tone_idx[9:0], TxK_Mask[29] TX_Tone = 16 */
			rtl_write_dword(rtlpriv, 0xc84, 0x18008c10);	/* RX_Tone_idx[9:0], RxK_Mask[29] */
			rtl_write_dword(rtlpriv, 0xc88, 0x02140119);
			rtl_write_dword(rtlpriv, 0xc8c, 0x28160d40);

			for (cal = 0; cal < cal_num; cal++) {
				rtl_set_bbreg(rtlpriv, 0x82c, BIT(31), 0x0);		/* [31] = 0 --> Page C */
				rtl_set_bbreg(rtlpriv, 0x978, 0x03FF8000, (TX_X0[cal])>>21&0x000007ff);
				rtl_set_bbreg(rtlpriv, 0x978, 0x000007FF, (TX_Y0[cal])>>21&0x000007ff);
				rtl_set_bbreg(rtlpriv, 0x82c, BIT(31), 0x1);		/* [31] = 1 --> Page C1 */
				cal_retry = 0;
				while (1) {
					/*  one shot */
					rtl_write_dword(rtlpriv, 0xcb8, 0x00100000);/* cb8[20] ±N SI/PI ¨Ï¥ÎÅv¤Áµ¹ iqk_dpk module */
					rtl_write_dword(rtlpriv, 0x980, 0xfa000000);
					rtl_write_dword(rtlpriv, 0x980, 0xf8000000);

					mdelay(10);			/* Delay 10ms */
					rtl_write_dword(rtlpriv, 0xcb8, 0x00000000);
					delay_count = 0;
					while (1) {
						IQK_ready = rtl_get_bbreg(rtlpriv, 0xd00, BIT(10));
						if ((IQK_ready) || (delay_count > 20)) {
							break;
						} else {
							mdelay(1);
							delay_count++;
						}
					}

				if (delay_count < 20) {	/* If 20ms No Result, then cal_retry++ */
					/* ============RXIQK Check============== */
					RX_fail = rtl_get_bbreg(rtlpriv, 0xd00, BIT(11));
					if (RX_fail == 0) {
						rtl_write_dword(rtlpriv, 0xcb8, 0x06000000);
						RX_X0[cal] = rtl_get_bbreg(rtlpriv, 0xd00, 0x07ff0000)<<21;
						rtl_write_dword(rtlpriv, 0xcb8, 0x08000000);
						RX_Y0[cal] = rtl_get_bbreg(rtlpriv, 0xd00, 0x07ff0000)<<21;
						RX0IQKOK = TRUE;
						/*
						rtl_write_dword(rtlpriv, 0xcb8, 0x05000000);
						reg1 = rtl_get_bbreg(rtlpriv, 0xd00, 0xffffffff);
						rtl_write_dword(rtlpriv, 0xcb8, 0x06000000);
						reg2 = rtl_get_bbreg(rtlpriv, 0xd00, 0x0000001f);
						DbgPrint("reg1 = %d, reg2 = %d", reg1, reg2);
						Image_Power = (reg2<<32)+reg1;
						DbgPrint("Before PW = %d\n", Image_Power);
						rtl_write_dword(rtlpriv, 0xcb8, 0x07000000);
						reg1 = rtl_get_bbreg(rtlpriv, 0xd00, 0xffffffff);
						rtl_write_dword(rtlpriv, 0xcb8, 0x08000000);
						reg2 = rtl_get_bbreg(rtlpriv, 0xd00, 0x0000001f);
						Image_Power = (reg2<<32)+reg1;
						DbgPrint("After PW = %d\n", Image_Power);
						*/

						break;
					} else {
						rtl_set_bbreg(rtlpriv, 0xc10, 0x000003ff, 0x200>>1);
						rtl_set_bbreg(rtlpriv, 0xc10, 0x03ff0000, 0x0>>1);
						RX0IQKOK = FALSE;
						cal_retry++;
						if (cal_retry == 10)
						break;
					}
				} else {
					RX0IQKOK = FALSE;
					cal_retry++;
					if (cal_retry == 10)
						break;
					}
				}
			ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("RXA_cal_retry = %d\n", cal_retry));
			if (RX0IQKOK)
				RX_Average++;
			}
		}
	    }
		break; /* MARK */
	case RF90_PATH_B:
	    {
		/* Path-B TX/RX IQK */
		rtl_set_bbreg(rtlpriv, 0x82c, BIT(31), 0x0);		/* [31] = 0 --> Page C */
		rtw_hal_write_rfreg(rtlpriv, Path, 0xef, bRFRegOffsetMask, 0x80002);
		rtw_hal_write_rfreg(rtlpriv, Path, 0x30, bRFRegOffsetMask, 0x20000);
		rtw_hal_write_rfreg(rtlpriv, Path, 0x31, bRFRegOffsetMask, 0x3fffd);
		rtw_hal_write_rfreg(rtlpriv, Path, 0x32, bRFRegOffsetMask, 0xfe83f);
		rtw_hal_write_rfreg(rtlpriv, Path, 0x65, bRFRegOffsetMask, 0x931d5);
		rtw_hal_write_rfreg(rtlpriv, Path, 0x8f, bRFRegOffsetMask, 0x8a001);
		rtl_write_dword(rtlpriv, 0x90c, 0x00008000);
		rtl_write_dword(rtlpriv, 0xb00, 0x03000100);
		rtl_set_bbreg(rtlpriv, 0xe94, BIT(0), 0x1);
		rtl_write_dword(rtlpriv, 0x978, 0x29002000);		/* TX (X,Y) */
		rtl_write_dword(rtlpriv, 0x97c, 0xa9002000);		/* RX (X,Y) */
		rtl_write_dword(rtlpriv, 0x984, 0x00462910);		/* [0]:AGC_en, [15]:idac_K_Mask */

		rtl_set_bbreg(rtlpriv, 0x82c, BIT(31), 0x1);		/* [31] = 1 --> Page C1 */

		if (rtlhal->external_pa_5g)
			rtl_write_dword(rtlpriv, 0xe88, 0x821403f7);
		else
			rtl_write_dword(rtlpriv, 0xe88, 0x821403f1);

		if (rtlhal->current_bandtype)
			rtl_write_dword(rtlpriv, 0xe8c, 0x68163e96);
		else
			rtl_write_dword(rtlpriv, 0xe8c, 0x28163e96);

		if (VDF_enable == 1) {
			for (k = 0; k <= 2; k++) {
				switch (k) {
				case 0:
					rtl_write_dword(rtlpriv, 0xe80, 0x18008c38);	/* TX_Tone_idx[9:0], TxK_Mask[29] TX_Tone = 16 */
					rtl_write_dword(rtlpriv, 0xe84, 0x38008c38);	/* RX_Tone_idx[9:0], RxK_Mask[29] */
					rtl_write_dword(rtlpriv, 0x984, 0x00462910);
					rtl_set_bbreg(rtlpriv, 0xee8, BIT(31), 0x0);
					break;
				case 1:
					rtl_set_bbreg(rtlpriv, 0xe80, BIT(28), 0x0);
					rtl_set_bbreg(rtlpriv, 0xe84, BIT(28), 0x0);
					rtl_write_dword(rtlpriv, 0x984, 0x0046a910);
					rtl_set_bbreg(rtlpriv, 0xee8, BIT(31), 0x0);
					break;
				case 2:
					ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("VDF_Y[1] = %x;;;VDF_Y[0] = %x\n", VDF_Y[1]>>21 & 0x00007ff, VDF_Y[0]>>21 & 0x00007ff));
					ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("VDF_X[1] = %x;;;VDF_X[0] = %x\n", VDF_X[1]>>21 & 0x00007ff, VDF_X[0]>>21 & 0x00007ff));
					Tx_dt[cal] = (VDF_Y[1]>>20)-(VDF_Y[0]>>20);
					ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("Tx_dt = %d\n", Tx_dt[cal]));
					Tx_dt[cal] = ((16*Tx_dt[cal])*10000/15708);
					Tx_dt[cal] = (Tx_dt[cal] >> 1) + (Tx_dt[cal] & BIT(0));
					rtl_write_dword(rtlpriv, 0xe80, 0x18008c20);	/* TX_Tone_idx[9:0], TxK_Mask[29] TX_Tone = 16 */
					rtl_write_dword(rtlpriv, 0xe84, 0x38008c20);	/* RX_Tone_idx[9:0], RxK_Mask[29] */
					rtl_set_bbreg(rtlpriv, 0xee8, BIT(31), 0x1);
					rtl_set_bbreg(rtlpriv, 0xee8, 0x3fff0000, Tx_dt[cal] & 0x00003fff);
					break;
				default:
					break;
				}


				rtl_write_dword(rtlpriv, 0xeb8, 0x00100000);/* cb8[20] ±N SI/PI ¨Ï¥ÎÅv¤Áµ¹ iqk_dpk module */
				cal_retry = 0;
				while (1) {
					/*  one shot */
					rtl_write_dword(rtlpriv, 0x980, 0xfa000000);
					rtl_write_dword(rtlpriv, 0x980, 0xf8000000);

					mdelay(10); /* Delay 10ms */
					rtl_write_dword(rtlpriv, 0xeb8, 0x00000000);
					delay_count = 0;
					while (1) {
						IQK_ready = rtl_get_bbreg(rtlpriv, 0xd40, BIT(10));
						if ((IQK_ready) || (delay_count > 20)) {
							break;
						} else {
							mdelay(1);
							delay_count++;
						}
					}

					if (delay_count < 20) {		/* If 20ms No Result, then cal_retry++ */
						/* ============TXIQK Check============== */
						TX_fail = rtl_get_bbreg(rtlpriv, 0xd40, BIT(12));

						if (~TX_fail) {
							rtl_write_dword(rtlpriv, 0xeb8, 0x02000000);
							VDF_X[k] = rtl_get_bbreg(rtlpriv, 0xd40, 0x07ff0000)<<21;
							rtl_write_dword(rtlpriv, 0xeb8, 0x04000000);
							VDF_Y[k] = rtl_get_bbreg(rtlpriv, 0xd40, 0x07ff0000)<<21;
							TX1IQKOK = TRUE;
							break;
						} else {
							TX1IQKOK = FALSE;
							cal_retry++;
							if (cal_retry == 10) {
								break;
							}
						}
					} else {
						TX1IQKOK = FALSE;
						cal_retry++;
						if (cal_retry == 10) {
							break;
						}
					}
				}
			}
			ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("TXB_VDF_cal_retry = %d\n", cal_retry));
			TX_X1[cal] = VDF_X[k-1] ;
			TX_Y1[cal] = VDF_Y[k-1];
		} else {
			rtl_write_dword(rtlpriv, 0xe80, 0x18008c10);	/* TX_Tone_idx[9:0], TxK_Mask[29] TX_Tone = 16 */
			rtl_write_dword(rtlpriv, 0xe84, 0x38008c10);	/* RX_Tone_idx[9:0], RxK_Mask[29] */
			rtl_write_dword(rtlpriv, 0xee8, 0x00000000);

			for (cal = 0; cal < cal_num; cal++) {
				cal_retry = 0;

				while (1) {
					/* one shot */
					rtl_write_dword(rtlpriv, 0xeb8, 0x00100000);/*  cb8[20] ±N SI/PI ¨Ï¥ÎÅv¤Áµ¹ iqk_dpk module */
					rtl_write_dword(rtlpriv, 0x980, 0xfa000000);
					rtl_write_dword(rtlpriv, 0x980, 0xf8000000);

					mdelay(10); /* Delay 25ms */
					rtl_write_dword(rtlpriv, 0xeb8, 0x00000000);
					delay_count = 0;
					while (1) {
						IQK_ready = rtl_get_bbreg(rtlpriv, 0xd40, BIT(10));
						if ((IQK_ready) || (delay_count > 20)) {
							break;
						} else {
							mdelay(1);
							delay_count++;
						}
					}

					if (delay_count < 20) {							/* If 20ms No Result, then cal_retry++ */
						/* ============TXIQK Check============== */
						TX_fail = rtl_get_bbreg(rtlpriv, 0xd40, BIT(12));
						if (~TX_fail) {
							rtl_write_dword(rtlpriv, 0xeb8, 0x02000000);
							TX_X1[cal] = rtl_get_bbreg(rtlpriv, 0xd40, 0x07ff0000)<<21;
							rtl_write_dword(rtlpriv, 0xeb8, 0x04000000);
							TX_Y1[cal] = rtl_get_bbreg(rtlpriv, 0xd40, 0x07ff0000)<<21;
							TX1IQKOK = TRUE;
							/*
							int			reg1 = 0, reg2 = 0, Image_Power = 0;
							rtl_write_dword(rtlpriv, 0xeb8, 0x01000000);
							reg1 = rtl_get_bbreg(rtlpriv, 0xd40, 0xffffffff);
							rtl_write_dword(rtlpriv, 0xeb8, 0x02000000);
							reg2 = rtl_get_bbreg(rtlpriv, 0xd40, 0x0000001f);
							Image_Power = (reg2<<32)+reg1;
							DbgPrint("Before PW = %d\n", Image_Power);
							rtl_write_dword(rtlpriv, 0xeb8, 0x03000000);
							reg1 = rtl_get_bbreg(rtlpriv, 0xd40, 0xffffffff);
							rtl_write_dword(rtlpriv, 0xeb8, 0x04000000);
							reg2 = rtl_get_bbreg(rtlpriv, 0xd40, 0x0000001f);
							Image_Power = (reg2<<32)+reg1;
							DbgPrint("After PW = %d\n", Image_Power);
							*/
							break;
						} else {
							TX1IQKOK = FALSE;
							cal_retry++;
							if (cal_retry == 10) {
								break;
							}
						}
					}  else {
						TX1IQKOK = FALSE;
						cal_retry++;
						if (cal_retry == 10) {
							break;
						}
					}
				}
				ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("TXB_cal_retry = %d\n", cal_retry));
				if (TX1IQKOK)
					TX_Average++;
			}
		}

		rtl_set_bbreg(rtlpriv, 0x82c, BIT(31), 0x0);		/* [31] = 0 --> Page C */
		rtw_hal_write_rfreg(rtlpriv, Path, 0x58, 0x7fe00, rtw_hal_read_rfreg(rtlpriv, Path, 0x8, 0xffc00));	/* Load LOK */
		rtl_set_bbreg(rtlpriv, 0x82c, BIT(31), 0x1);		/* [31] = 1 --> Page C1 */

		if (TX1IQKOK == FALSE)
			break;				/* TXK fail, Don't do RXK */

		if (VDF_enable == 1) {
			rtl_set_bbreg(rtlpriv, 0xee8, BIT(31), 0x0);    /* TX VDF Disable */
			ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("RXVDF Start\n"));

			/* ====== RX IQK ====== */
			rtl_set_bbreg(rtlpriv, 0x82c, BIT(31), 0x0);		/* [31] = 0 --> Page C */
			rtw_hal_write_rfreg(rtlpriv, Path, 0xef, bRFRegOffsetMask, 0x80000);
			rtw_hal_write_rfreg(rtlpriv, Path, 0x30, bRFRegOffsetMask, 0x30000);
			rtw_hal_write_rfreg(rtlpriv, Path, 0x31, bRFRegOffsetMask, 0x3f7ff);
			rtw_hal_write_rfreg(rtlpriv, Path, 0x32, bRFRegOffsetMask, 0xfe7bf);
			rtw_hal_write_rfreg(rtlpriv, Path, 0x8f, bRFRegOffsetMask, 0x88001);
			rtw_hal_write_rfreg(rtlpriv, Path, 0x65, bRFRegOffsetMask, 0x931d0);
			rtw_hal_write_rfreg(rtlpriv, Path, 0xef, bRFRegOffsetMask, 0x00000);

			rtl_set_bbreg(rtlpriv, 0x978, BIT(31), 0x1);
			rtl_set_bbreg(rtlpriv, 0x97c, BIT(31), 0x0);
			rtl_write_dword(rtlpriv, 0x984, 0x0046a911);
			rtl_set_bbreg(rtlpriv, 0x82c, BIT(31), 0x1);		/* [31] = 1 --> Page C1 */
			rtl_write_dword(rtlpriv, 0xe88, 0x02140119);
			rtl_write_dword(rtlpriv, 0xe8c, 0x28161420);

			for (k = 0; k <= 2; k++) {
				rtl_set_bbreg(rtlpriv, 0x82c, BIT(31), 0x0);		/* [31] = 0 --> Page C */
				rtl_set_bbreg(rtlpriv, 0x978, 0x03FF8000, (VDF_X[k])>>21&0x000007ff);
				rtl_set_bbreg(rtlpriv, 0x978, 0x000007FF, (VDF_Y[k])>>21&0x000007ff);
				rtl_set_bbreg(rtlpriv, 0x82c, BIT(31), 0x1);		/* [31] = 1 --> Page C1 */

				switch (k) {
				case 0:
					rtl_write_dword(rtlpriv, 0xe80, 0x38008c38);	/* TX_Tone_idx[9:0], TxK_Mask[29] TX_Tone = 16 */
					rtl_write_dword(rtlpriv, 0xe84, 0x18008c38);	/* RX_Tone_idx[9:0], RxK_Mask[29] */
					rtl_set_bbreg(rtlpriv, 0xee8, BIT(30), 0x0);
					break;
				case 1:
					rtl_write_dword(rtlpriv, 0xe80, 0x28008c38);	/* TX_Tone_idx[9:0], TxK_Mask[29] TX_Tone = 16 */
					rtl_write_dword(rtlpriv, 0xe84, 0x08008c38);	/* RX_Tone_idx[9:0], RxK_Mask[29] */
					rtl_set_bbreg(rtlpriv, 0xee8, BIT(30), 0x0);
					break;
				case 2:
					ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("VDF_Y[1] = %x;;;VDF_Y[0] = %x\n", VDF_Y[1]>>21 & 0x00007ff, VDF_Y[0]>>21 & 0x00007ff));
					ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("VDF_X[1] = %x;;;VDF_X[0] = %x\n", VDF_X[1]>>21 & 0x00007ff, VDF_X[0]>>21 & 0x00007ff));
					Rx_dt[cal] = (VDF_Y[1]>>20)-(VDF_Y[0]>>20);
					ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("Rx_dt = %d\n", Rx_dt[cal]));
					Rx_dt[cal] = ((16*Rx_dt[cal])*10000/13823);
					Rx_dt[cal] = (Rx_dt[cal] >> 1) + (Rx_dt[cal] & BIT(0));
					rtl_write_dword(rtlpriv, 0xe80, 0x38008c20);	/* TX_Tone_idx[9:0], TxK_Mask[29] TX_Tone = 16 */
					rtl_write_dword(rtlpriv, 0xe84, 0x18008c20);	/* RX_Tone_idx[9:0], RxK_Mask[29] */
					rtl_set_bbreg(rtlpriv, 0xee8, 0x00003fff, Rx_dt[cal] & 0x00003fff);
					break;
				default:
					break;
				}


				if (k == 2) {
					rtl_set_bbreg(rtlpriv, 0xee8, BIT(30), 0x1);	/* RX VDF Enable */
				}

				rtl_write_dword(rtlpriv, 0xeb8, 0x00100000);		/* cb8[20] ±N SI/PI ¨Ï¥ÎÅv¤Áµ¹ iqk_dpk module */

				cal_retry = 0;

				while (1) {
					/* one shot */
					rtl_write_dword(rtlpriv, 0x980, 0xfa000000);
					rtl_write_dword(rtlpriv, 0x980, 0xf8000000);

					mdelay(10); /* Delay 10ms */
					rtl_write_dword(rtlpriv, 0xeb8, 0x00000000);
					delay_count = 0;
					while (1) {
						IQK_ready = rtl_get_bbreg(rtlpriv, 0xd40, BIT(10));
						if ((IQK_ready) || (delay_count > 20)) {
							break;
						} else {
							mdelay(1);
							delay_count++;
						}
					}

					if (delay_count < 20) {	/* If 20ms No Result, then cal_retry++ */
						/* ============RXIQK Check============== */
						RX_fail = rtl_get_bbreg(rtlpriv, 0xd40, BIT(11));
						if (RX_fail == 0) {
							rtl_write_dword(rtlpriv, 0xeb8, 0x06000000);
							VDF_X[k] = rtl_get_bbreg(rtlpriv, 0xd40, 0x07ff0000)<<21;
							rtl_write_dword(rtlpriv, 0xeb8, 0x08000000);
							VDF_Y[k] = rtl_get_bbreg(rtlpriv, 0xd40, 0x07ff0000)<<21;
							RX1IQKOK = TRUE;
							break;
						} else {
							rtl_set_bbreg(rtlpriv, 0xe10, 0x000003ff, 0x200>>1);
							rtl_set_bbreg(rtlpriv, 0xe10, 0x03ff0000, 0x0>>1);
							RX1IQKOK = FALSE;
							cal_retry++;
							if (cal_retry == 10)
								break;

						}
					} else {
						RX1IQKOK = FALSE;
						cal_retry++;
						if (cal_retry == 10)
							break;
					}
				}
			}


			ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("RXB_VDF_cal_retry = %d\n", cal_retry));
			RX_X1[cal] = VDF_X[k-1] ;
			RX_Y1[cal] = VDF_Y[k-1];
			rtl_set_bbreg(rtlpriv, 0xee8, BIT(31), 0x1);	/* TX VDF Enable */
		} else {
			/* ====== RX IQK ====== */
			rtl_set_bbreg(rtlpriv, 0x82c, BIT(31), 0x0); /* [31] = 0 --> Page C */
			rtw_hal_write_rfreg(rtlpriv, Path, 0xef, bRFRegOffsetMask, 0x80000);
			rtw_hal_write_rfreg(rtlpriv, Path, 0x30, bRFRegOffsetMask, 0x30000);
			rtw_hal_write_rfreg(rtlpriv, Path, 0x31, bRFRegOffsetMask, 0x3f7ff);
			rtw_hal_write_rfreg(rtlpriv, Path, 0x32, bRFRegOffsetMask, 0xfe7bf);
			rtw_hal_write_rfreg(rtlpriv, Path, 0x8f, bRFRegOffsetMask, 0x88001);
			rtw_hal_write_rfreg(rtlpriv, Path, 0x65, bRFRegOffsetMask, 0x931d0);
			rtw_hal_write_rfreg(rtlpriv, Path, 0xef, bRFRegOffsetMask, 0x00000);

			rtl_set_bbreg(rtlpriv, 0x978, BIT(31), 0x1);
			rtl_set_bbreg(rtlpriv, 0x97c, BIT(31), 0x0);
			rtl_write_dword(rtlpriv, 0x90c, 0x00008000);
			/* rtl_write_dword(rtlpriv, 0x984, 0x0046a911); */
			rtl_write_dword(rtlpriv, 0x984, 0x0046a891);

			rtl_set_bbreg(rtlpriv, 0x82c, BIT(31), 0x1);	/* [31] = 1 --> Page C1 */
			rtl_write_dword(rtlpriv, 0xe80, 0x38008c10);	/* TX_Tone_idx[9:0], TxK_Mask[29] TX_Tone = 16 */
			rtl_write_dword(rtlpriv, 0xe84, 0x18008c10);	/* RX_Tone_idx[9:0], RxK_Mask[29] */
			rtl_write_dword(rtlpriv, 0xe88, 0x02140119);
			rtl_write_dword(rtlpriv, 0xe8c, 0x28161180);

			for (cal = 0; cal < cal_num; cal++) {
				rtl_set_bbreg(rtlpriv, 0x82c, BIT(31), 0x0);		/* [31] = 0 --> Page C */
				rtl_set_bbreg(rtlpriv, 0x978, 0x03FF8000, (TX_X1[cal])>>21&0x000007ff);
				rtl_set_bbreg(rtlpriv, 0x978, 0x000007FF, (TX_Y1[cal])>>21&0x000007ff);
				rtl_set_bbreg(rtlpriv, 0x82c, BIT(31), 0x1);		/* [31] = 1 --> Page C1 */
				cal_retry = 0;

				while (1) {
					/* one shot */
					rtl_write_dword(rtlpriv, 0xeb8, 0x00100000);	/* cb8[20] ±N SI/PI ¨Ï¥ÎÅv¤Áµ¹ iqk_dpk module */
					rtl_write_dword(rtlpriv, 0x980, 0xfa000000);
					rtl_write_dword(rtlpriv, 0x980, 0xf8000000);

					mdelay(10);	/*Delay 10ms */
					rtl_write_dword(rtlpriv, 0xeb8, 0x00000000);
					delay_count = 0;

					while (1) {
						IQK_ready = rtl_get_bbreg(rtlpriv, 0xd40, BIT(10));
						if ((IQK_ready) || (delay_count > 20)) {
							break;
						} else {
							mdelay(1);
							delay_count++;
						}
					}

					if (delay_count < 20) {				/* If 20ms No Result, then cal_retry++ */
						/* ============RXIQK Check============== */
						RX_fail = rtl_get_bbreg(rtlpriv, 0xd40, BIT(11));
						if (RX_fail == 0) {
							rtl_write_dword(rtlpriv, 0xeb8, 0x06000000);
							RX_X1[cal] = rtl_get_bbreg(rtlpriv, 0xd40, 0x07ff0000)<<21;
							rtl_write_dword(rtlpriv, 0xeb8, 0x08000000);
							RX_Y1[cal] = rtl_get_bbreg(rtlpriv, 0xd40, 0x07ff0000)<<21;
							RX1IQKOK = TRUE;
							/*
							rtl_write_dword(rtlpriv, 0xeb8, 0x05000000);
							reg1 = rtl_get_bbreg(rtlpriv, 0xd40, 0xffffffff);
							rtl_write_dword(rtlpriv, 0xeb8, 0x06000000);
							reg2 = rtl_get_bbreg(rtlpriv, 0xd40, 0x0000001f);
							DbgPrint("reg1 = %d, reg2 = %d", reg1, reg2);
							Image_Power = (reg2<<32)+reg1;
							DbgPrint("Before PW = %d\n", Image_Power);
							rtl_write_dword(rtlpriv, 0xeb8, 0x07000000);
							reg1 = rtl_get_bbreg(rtlpriv, 0xd40, 0xffffffff);
							rtl_write_dword(rtlpriv, 0xeb8, 0x08000000);
							reg2 = rtl_get_bbreg(rtlpriv, 0xd40, 0x0000001f);
							Image_Power = (reg2<<32)+reg1;
							DbgPrint("After PW = %d\n", Image_Power);
							*/
							break;
						} else {
							rtl_set_bbreg(rtlpriv, 0xe10, 0x000003ff, 0x200>>1);
							rtl_set_bbreg(rtlpriv, 0xe10, 0x03ff0000, 0x0>>1);
							RX1IQKOK = FALSE;
							cal_retry++;
							if (cal_retry == 10)
								break;
						}
					} else {
						RX1IQKOK = FALSE;
						cal_retry++;
						if (cal_retry == 10)
							break;
					}

				}

			ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("RXB_cal_retry = %d\n", cal_retry));
			if (RX1IQKOK)
				RX_Average++;
			}
		}
	    }
		break;
	default:
		break;
	}

	/* FillIQK Result */
	switch (Path) {
	case RF90_PATH_A:
	    {
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("========Path_A =======\n"));
		if (TX_Average == 0) {
			_rtl8812au_iqk_tx_fill_iqc(rtlpriv, Path, 0x200, 0x0);
			break;
		}
		for (i = 0; i < TX_Average; i++) {
			ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("TX_X0[%d] = %x ;; TX_Y0[%d] = %x\n", i, (TX_X0[i])>>21&0x000007ff, i, (TX_Y0[i])>>21&0x000007ff));
		}

		for (i = 0; i < TX_Average; i++) {
			for (ii = i+1; ii < TX_Average; ii++) {
				dx = (TX_X0[i]>>21) - (TX_X0[ii]>>21);
				if (dx < 4 && dx > -4) {
					dy = (TX_Y0[i]>>21) - (TX_Y0[ii]>>21);
					if (dy < 4 && dy > -4) {
						TX_X = ((TX_X0[i]>>21) + (TX_X0[ii]>>21))/2;
						TX_Y = ((TX_Y0[i]>>21) + (TX_Y0[ii]>>21))/2;
						if (rtlpriv->phy.current_chan_bw == 2) {
							Tx_dt[0] = (Tx_dt[i] + Tx_dt[ii])/2;
						}
						TX_finish = 1;
						break;
					}
				}
			}
			if (TX_finish == 1)
				break;
		}

		if (rtlpriv->phy.current_chan_bw == 2) {
			rtl_set_bbreg(rtlpriv, 0x82c, BIT(31), 0x1);		/* [31] = 0 --> Page C */
			rtl_set_bbreg(rtlpriv, 0xce8, 0x3fff0000, Tx_dt[0] & 0x00003fff);
			rtl_set_bbreg(rtlpriv, 0x82c, BIT(31), 0x0);		/* [31] = 0 --> Page C */
		}

		if (TX_finish == 1) {
			_rtl8812au_iqk_tx_fill_iqc(rtlpriv, Path, TX_X, TX_Y);
		} else {
			_rtl8812au_iqk_tx_fill_iqc(rtlpriv, Path, 0x200, 0x0);
		}

		if (RX_Average == 0) {
			_rtl8812au_iqk_rx_fill_iqc(rtlpriv, Path, 0x200, 0x0);
			break;
		}

		for (i = 0; i < RX_Average; i++) {
			ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("RX_X0[%d] = %x ;; RX_Y0[%d] = %x\n", i, (RX_X0[i])>>21&0x000007ff, i, (RX_Y0[i])>>21&0x000007ff));
		}

		for (i = 0; i < RX_Average; i++) {
			for (ii = i+1; ii < RX_Average; ii++) {
				dx = (RX_X0[i]>>21) - (RX_X0[ii]>>21);
				if (dx < 4 && dx > -4) {
					dy = (RX_Y0[i]>>21) - (RX_Y0[ii]>>21);
					if (dy < 4 && dy > -4) {
						RX_X = ((RX_X0[i]>>21) + (RX_X0[ii]>>21))/2;
						RX_Y = ((RX_Y0[i]>>21) + (RX_Y0[ii]>>21))/2;
						if (rtlpriv->phy.current_chan_bw == 2) {
							Rx_dt[0] = (Rx_dt[i] + Rx_dt[ii])/2;
						}
						RX_finish = 1;
						break;
					}
				}
			}
			if (RX_finish == 1)
				break;
		}

		if (rtlpriv->phy.current_chan_bw == 2) {
			rtl_set_bbreg(rtlpriv, 0x82c, BIT(31), 0x1); /* [31] = 0 --> Page C */
			rtl_set_bbreg(rtlpriv, 0xce8, 0x00003fff, Rx_dt[0] & 0x00003fff);
			rtl_set_bbreg(rtlpriv, 0x82c, BIT(31), 0x0); /* [31] = 0 --> Page C */
		}

		if (RX_finish == 1) {
			_rtl8812au_iqk_rx_fill_iqc(rtlpriv, Path, RX_X, RX_Y);
		} else {
			_rtl8812au_iqk_rx_fill_iqc(rtlpriv, Path, 0x200, 0x0);
		}

		/* 
		 * ULLI check with rtl8821ae source, if we can remove this
		 * look as the commit 5856f7384c22bf6364ecb77635e95b4858567080
		 * from #if 1 
		 */ 
#if 1
		if (TX_finish && RX_finish) {
			rtlpriv->phy.need_iqk = false;

			if (rtlpriv->phy.current_chan_bw == 2) {
				rtl_set_bbreg(rtlpriv, 0x82c, BIT(31), 0x1); /* [31] = 0 --> Page C */
				rtl_set_bbreg(rtlpriv, 0x82c, BIT(31), 0x0); /* [31] = 0 --> Page C */
			}
		}
#endif		

	    }
		break;
	case RF90_PATH_B:
	    {
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("========Path_B =======\n"));
		if (TX_Average == 0) {
			_rtl8812au_iqk_tx_fill_iqc(rtlpriv, Path, 0x200, 0x0);
			break;
		}

		for (i = 0; i < TX_Average; i++) {
			ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("TX_X1[%d] = %x ;; TX_Y1[%d] = %x\n", i, (TX_X1[i])>>21&0x000007ff, i, (TX_Y1[i])>>21&0x000007ff));
		}

		for (i = 0; i < TX_Average; i++) {
			for (ii = i+1; ii < TX_Average; ii++) {
				dx = (TX_X1[i]>>21) - (TX_X1[ii]>>21);
				if (dx < 4 && dx > -4) {
					dy = (TX_Y1[i]>>21) - (TX_Y1[ii]>>21);
					if (dy < 4 && dy > -4) {
						TX_X = ((TX_X1[i]>>21) + (TX_X1[ii]>>21))/2;
						TX_Y = ((TX_Y1[i]>>21) + (TX_Y1[ii]>>21))/2;
						if (rtlpriv->phy.current_chan_bw == 2) {
							Tx_dt[0] = (Tx_dt[i] + Tx_dt[ii])/2;
						}
						TX_finish = 1;
						break;
					}
				}
			}
			if (TX_finish == 1)
				break;
		}

		if (rtlpriv->phy.current_chan_bw == 2) {
			rtl_set_bbreg(rtlpriv, 0x82c, BIT(31), 0x1); /* [31] = 0 --> Page C */
			rtl_set_bbreg(rtlpriv, 0xee8, 0x3fff0000, Tx_dt[0] & 0x00003fff);
			rtl_set_bbreg(rtlpriv, 0x82c, BIT(31), 0x0); /* [31] = 0 --> Page C */
		}
		if (TX_finish == 1) {
			_rtl8812au_iqk_tx_fill_iqc(rtlpriv, Path, TX_X, TX_Y);
		} else {
			_rtl8812au_iqk_tx_fill_iqc(rtlpriv, Path, 0x200, 0x0);
		}

		if (RX_Average == 0) {
			_rtl8812au_iqk_rx_fill_iqc(rtlpriv, Path, 0x200, 0x0);
			break;
		}

		for (i = 0; i < RX_Average; i++) {
			ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("RX_X1[%d] = %x ;; RX_Y1[%d] = %x\n", i, (RX_X1[i])>>21&0x000007ff, i, (RX_Y1[i])>>21&0x000007ff));
		}

		for (i = 0; i < RX_Average; i++) {
			for (ii = i+1; ii < RX_Average; ii++) {
				dx = (RX_X1[i]>>21) - (RX_X1[ii]>>21);
				if (dx < 4 && dx > -4) {
					dy = (RX_Y1[i]>>21) - (RX_Y1[ii]>>21);
					if (dy < 4 && dy > -4) {
						RX_X = ((RX_X1[i]>>21) + (RX_X1[ii]>>21))/2;
						RX_Y = ((RX_Y1[i]>>21) + (RX_Y1[ii]>>21))/2;
						if (rtlpriv->phy.current_chan_bw == 2) {
							Rx_dt[0] = (Rx_dt[i] + Rx_dt[ii])/2;
						}
						RX_finish = 1;
						break;
					}
				}
			}
			if (RX_finish == 1)
				break;
		}

		if (rtlpriv->phy.current_chan_bw == 2) {
			rtl_set_bbreg(rtlpriv, 0x82c, BIT(31), 0x1); /* [31] = 0 --> Page C */
			rtl_set_bbreg(rtlpriv, 0xee8, 0x00003fff, Rx_dt[0] & 0x00003fff);
			rtl_set_bbreg(rtlpriv, 0x82c, BIT(31), 0x0); /* [31] = 0 --> Page C */
		}

		if (RX_finish == 1) {
			_rtl8812au_iqk_rx_fill_iqc(rtlpriv, Path, RX_X, RX_Y);
		} else{
			_rtl8812au_iqk_rx_fill_iqc(rtlpriv, Path, 0x200, 0x0);
		}


		/* 
		 * ULLI check with rtl8821ae source, if we can remove this
		 * look as the commit 5856f7384c22bf6364ecb77635e95b4858567080
		 * from #if 1 
		 */ 
#if 1		 
		if (TX_finish && RX_finish) {
/* pRFCalibrateInfo->IQKMatrixRegSetting[chnlIdx].bIQKDone= TRUE; */
			rtlpriv->phy.need_iqk = false;

			if (rtlpriv->phy.current_chan_bw == 2) {
				rtl_set_bbreg(rtlpriv, 0x82c, BIT(31), 0x1); /* [31] = 0 --> Page C */
				rtl_set_bbreg(rtlpriv, 0x82c, BIT(31), 0x0); /* [31] = 0 --> Page C */
			}
		}
#endif		

	    }
		break;
	default:
		break;
	}
}



static void _rtl8812au_iqk_backup_macbb(struct rtl_priv *rtlpriv, 
					u32 *macbb_backup,
					u32 *backup_macbb_reg, u32 mac_bb_num)
{
	u32 i;
	
	rtl_set_bbreg(rtlpriv, 0x82c, BIT(31), 0x0); /* [31] = 0 --> Page C */
	/* save MACBB default value */
	for (i = 0; i < mac_bb_num; i++) {
		macbb_backup[i] = rtl_read_dword(rtlpriv, backup_macbb_reg[i]);
	}

	/* ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("BackupMacBB Success!!!!\n")); */
}

static void _rtl8821au_iqk_backup_macbb(struct rtl_priv *rtlpriv,
					u32 *macbb_backup,
					u32 *backup_macbb_reg, u32 mac_bb_num)
{
	u32 i;

	rtl_set_bbreg(rtlpriv, 0x82c, BIT(31), 0x0); /* [31] = 0 --> Page C */
	/* save MACBB default value */
	for (i = 0; i < mac_bb_num; i++) {
		macbb_backup[i] = rtl_read_dword(rtlpriv, backup_macbb_reg[i]);
	}

	/* ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("BackupMacBB Success!!!!\n")); */
}

static void _rtl8812au_iqk_backup_rf(struct rtl_priv *rtlpriv,
	uint32_t *RFA_backup, uint32_t *RFB_backup,
	uint32_t *Backup_RF_REG, uint32_t RF_NUM)
{
	uint32_t i;
	
	rtl_set_bbreg(rtlpriv, 0x82c, BIT(31), 0x0); /* [31] = 0 --> Page C */
	/* Save RF Parameters */
	for (i = 0; i < RF_NUM; i++) {
		RFA_backup[i] = rtw_hal_read_rfreg(rtlpriv, RF90_PATH_A, Backup_RF_REG[i], bMaskDWord);
		RFB_backup[i] = rtw_hal_read_rfreg(rtlpriv, RF90_PATH_B, Backup_RF_REG[i], bMaskDWord);
	}
	/* ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("BackupRF Success!!!!\n")); */
}

static void _rtl8821au_iqk_backup_rf(struct rtl_priv *rtlpriv, u32 *rfa_backup,
				     u32 *rfb_backup, u32 *backup_rf_reg,
				     u32 rf_num)
{
	u32 i;

	rtl_set_bbreg(rtlpriv, 0x82c, BIT(31), 0x0); /* [31] = 0 --> Page C */
	/* Save RF Parameters */
	for (i = 0; i < rf_num; i++) {
		rfa_backup[i] = rtw_hal_read_rfreg(rtlpriv, RF90_PATH_A, backup_rf_reg[i], bMaskDWord);
	}

	/* ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("BackupRF Success!!!!\n")); */
}

static void _rtl8812au_iqk_backup_afe(struct rtl_priv *rtlpriv,
	uint32_t *AFE_backup, uint32_t *Backup_AFE_REG, uint32_t AFE_NUM)
{
	uint32_t i;
	
	rtl_set_bbreg(rtlpriv, 0x82c, BIT(31), 0x0); /* [31] = 0 --> Page C */
	/* Save AFE Parameters */
	for (i = 0; i < AFE_NUM; i++) {
		AFE_backup[i] = rtl_read_dword(rtlpriv, Backup_AFE_REG[i]);
	}
	/* ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("BackupAFE Success!!!!\n")); */
}

static void _rtl8821au_iqk_backup_afe(struct rtl_priv *rtlpriv, u32 *afe_backup,
				      u32 *backup_afe_REG, u32 afe_num)
{
	u32  i;

	rtl_set_bbreg(rtlpriv, 0x82c, BIT(31), 0x0); /* [31] = 0 --> Page C */
	/* Save AFE Parameters */
	for (i = 0; i < afe_num; i++) {
		afe_backup[i] = rtl_read_dword(rtlpriv, backup_afe_REG[i]);
	}
	/* ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("BackupAFE Success!!!!\n")); */
}

static void _rtl8812au_iqk_restore_macbb(struct rtl_priv *rtlpriv,
	uint32_t *MACBB_backup, uint32_t *Backup_MACBB_REG, uint32_t MACBB_NUM)
{
	uint32_t i;
	rtl_set_bbreg(rtlpriv, 0x82c, BIT(31), 0x0);     /* [31] = 0 --> Page C */
	/* Reload MacBB Parameters */
	for (i = 0; i < MACBB_NUM; i++) {
		rtl_write_dword(rtlpriv, Backup_MACBB_REG[i], MACBB_backup[i]);
	}
	/* ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("RestoreMacBB Success!!!!\n")); */
}

static void _rtl8821au_iqk_restore_macbb(struct rtl_priv *rtlpriv,
					 u32 *macbb_backup,
					 u32 *backup_macbb_reg,
					 u32 macbb_num)
{
	u32 i;

	rtl_set_bbreg(rtlpriv, 0x82c, BIT(31), 0x0);     /* [31] = 0 --> Page C */
	/* Reload MacBB Parameters */
	for (i = 0; i < macbb_num; i++) {
		rtl_write_dword(rtlpriv, backup_macbb_reg[i], macbb_backup[i]);
	}
	/* ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("RestoreMacBB Success!!!!\n")); */
}

static void _rtl8812au_iqk_restore_rf(struct rtl_priv *rtlpriv,
	enum radio_path Path, uint32_t *Backup_RF_REG, uint32_t *RF_backup, uint32_t RF_REG_NUM)
{
	uint32_t i;

	rtl_set_bbreg(rtlpriv, 0x82c, BIT(31), 0x0); /*  [31] = 0 --> Page C */
	for (i = 0; i < RF_REG_NUM; i++)
		rtw_hal_write_rfreg(rtlpriv, Path, Backup_RF_REG[i], bRFRegOffsetMask, RF_backup[i]);

	rtw_hal_write_rfreg(rtlpriv, Path, 0xef, bRFRegOffsetMask, 0x0);

	switch (Path) {
	case RF90_PATH_A:
		/* ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("RestoreRF Path A Success!!!!\n")); */
		break;
	case RF90_PATH_B:
		/* ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("RestoreRF Path B Success!!!!\n")); */
		break;
	default:
		break;
	}
}

static void _rtl8821au_iqk_restore_rf(struct rtl_priv *rtlpriv,
				      enum radio_path Path,
				      u32 *backup_rf_reg,
				      u32 *rf_backup, u32 rf_reg_num)
{
	u32 i;

	rtl_set_bbreg(rtlpriv, 0x82c, BIT(31), 0x0); /*  [31] = 0 --> Page C */
	for (i = 0; i < rf_reg_num; i++)
		rtw_hal_write_rfreg(rtlpriv, Path, backup_rf_reg[i], bRFRegOffsetMask, rf_backup[i]);

	switch (Path) {
	case RF90_PATH_A:
		/* ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("RestoreRF Path A Success!!!!\n")); */
		break;
	default:
		break;
	}
}

static void _rtl8812au_iqk_restore_afe(struct rtl_priv *rtlpriv, uint32_t *AFE_backup,
	uint32_t *Backup_AFE_REG, uint32_t AFE_NUM)
{
	uint32_t i;
	
	rtl_set_bbreg(rtlpriv, 0x82c, BIT(31), 0x0); /* [31] = 0 --> Page C */
	/* Reload AFE Parameters */
	for (i = 0; i < AFE_NUM; i++) {
		rtl_write_dword(rtlpriv, Backup_AFE_REG[i], AFE_backup[i]);
	}
	rtl_set_bbreg(rtlpriv, 0x82c, BIT(31), 0x1); /*  [31] = 1 --> Page C1 */
	rtl_write_dword(rtlpriv, 0xc80, 0x0);
	rtl_write_dword(rtlpriv, 0xc84, 0x0);
	rtl_write_dword(rtlpriv, 0xc88, 0x0);
	rtl_write_dword(rtlpriv, 0xc8c, 0x3c000000);
	rtl_write_dword(rtlpriv, 0xcb8, 0x0);
	rtl_write_dword(rtlpriv, 0xe80, 0x0);
	rtl_write_dword(rtlpriv, 0xe84, 0x0);
	rtl_write_dword(rtlpriv, 0xe88, 0x0);
	rtl_write_dword(rtlpriv, 0xe8c, 0x3c000000);
	rtl_write_dword(rtlpriv, 0xeb8, 0x0);
	/* cODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("RestoreAFE Success!!!!\n")); */
}

static void _rtl8821au_iqk_restore_afe(struct rtl_priv *rtlpriv,
				       u32 *afe_backup, u32 *backup_afe_reg,
				       u32 afe_num)
{
	uint32_t i;
	rtl_set_bbreg(rtlpriv, 0x82c, BIT(31), 0x0); /* [31] = 0 --> Page C */
	/* Reload AFE Parameters */
	for (i = 0; i < afe_num; i++) {
		rtl_write_dword(rtlpriv, backup_afe_reg[i], afe_backup[i]);
	}
	rtl_set_bbreg(rtlpriv, 0x82c, BIT(31), 0x1); /* [31] = 1 --> Page C1 */
	rtl_write_dword(rtlpriv, 0xc80, 0x0);
	rtl_write_dword(rtlpriv, 0xc84, 0x0);
	rtl_write_dword(rtlpriv, 0xc88, 0x0);
	rtl_write_dword(rtlpriv, 0xc8c, 0x3c000000);
	rtl_write_dword(rtlpriv, 0xcb8, 0x0);
	/* ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("RestoreAFE Success!!!!\n")); */
}


static void _rtl8812au_iqk_configure_mac(struct rtl_priv *rtlpriv)
{
	/* ========MAC register setting======== */
	rtl_set_bbreg(rtlpriv, 0x82c, BIT(31), 0x0); 	/* [31] = 0 --> Page C */
	rtl_write_byte(rtlpriv, 0x522, 0x3f);
	rtl_set_bbreg(rtlpriv, 0x550, BIT(11)|BIT(3), 0x0);
	rtl_set_bbreg(rtlpriv, 0x808, BIT(28), 0x0);	/* CCK Off */
	rtl_write_byte(rtlpriv, 0x808, 0x00);		/* RX ante off */
	rtl_set_bbreg(rtlpriv, 0x838, 0xf, 0xc);		/* CCA off */
}

static void _rtl8821au_iqk_configure_mac(struct rtl_priv *rtlpriv)
{
	/* ========MAC register setting======== */
	rtl_set_bbreg(rtlpriv, 0x82c, BIT(31), 0x0); /* [31] = 0 --> Page C */
	rtl_write_byte(rtlpriv, 0x522, 0x3f);
	rtl_set_bbreg(rtlpriv, 0x550, BIT(3), 0x0);
	rtl_set_bbreg(rtlpriv, 0x551, BIT(3), 0x0);
	rtl_set_bbreg(rtlpriv, 0x808, BIT(28), 0x0);	/* CCK Off */
	rtl_write_byte(rtlpriv, 0x808, 0x00);		/* RX ante off */
	rtl_set_bbreg(rtlpriv, 0x838, 0xf, 0xc);		/* CCA off */
}


#undef MACBB_REG_NUM
#undef AFE_REG_NUM
#undef RF_REG_NUM 
#define MACBB_REG_NUM 10
#define AFE_REG_NUM 14
#define RF_REG_NUM 3


/* Maintained by BB James. */
static void _rtl8812au_phy_iq_calibrate(struct rtl_priv *rtlpriv)
{
	uint32_t MACBB_backup[MACBB_REG_NUM], AFE_backup[AFE_REG_NUM], RFA_backup[RF_REG_NUM], RFB_backup[RF_REG_NUM];
	uint32_t Backup_MACBB_REG[MACBB_REG_NUM] = { 0xb00, 0x520, 0x550, 0x808, 0x90c, 0xc00, 0xe00, 0x8c4, 0x838, 0x82c };
	uint32_t Backup_AFE_REG[AFE_REG_NUM] = { 0xc5c, 0xc60, 0xc64, 0xc68, 0xcb8, 0xcb0, 0xcb4,
						 0xe5c, 0xe60, 0xe64, 0xe68, 0xeb8, 0xeb0, 0xeb4 };
	uint32_t Backup_RF_REG[RF_REG_NUM] = { 0x65, 0x8f, 0x0 };


	_rtl8812au_iqk_backup_macbb(rtlpriv, MACBB_backup, Backup_MACBB_REG, MACBB_REG_NUM);
	_rtl8812au_iqk_backup_afe(rtlpriv, AFE_backup, Backup_AFE_REG, AFE_REG_NUM);
	_rtl8812au_iqk_backup_rf(rtlpriv, RFA_backup, RFB_backup, Backup_RF_REG, RF_REG_NUM);

	_rtl8812au_iqk_configure_mac(rtlpriv);
	_rtl8812au_iqk_tx(rtlpriv, RF90_PATH_A);
	_rtl8812au_iqk_restore_rf(rtlpriv, RF90_PATH_A, Backup_RF_REG, RFA_backup, RF_REG_NUM);

	_rtl8812au_iqk_tx(rtlpriv, RF90_PATH_B);
	_rtl8812au_iqk_restore_rf(rtlpriv, RF90_PATH_B, Backup_RF_REG, RFB_backup, RF_REG_NUM);

	_rtl8812au_iqk_restore_afe(rtlpriv, AFE_backup, Backup_AFE_REG, AFE_REG_NUM);
	_rtl8812au_iqk_restore_macbb(rtlpriv, MACBB_backup, Backup_MACBB_REG, MACBB_REG_NUM);
}



void rtl8812au_phy_iq_calibrate(struct rtl_priv *rtlpriv, BOOLEAN bReCovery)
{
	_rtl8812au_phy_iq_calibrate(rtlpriv);
}



/* ****************************************************************************** */
/*									*/
/*  from HalPhyRf_8821A.c						*/
/*									*/
/* ****************************************************************************** */








#define cal_num 3

static void _rtl8821au_iqk_tx(struct rtl_priv *rtlpriv, enum radio_path Path)
{
	struct rtl_hal	*rtlhal = rtl_hal(rtlpriv);

	uint32_t TX_fail, RX_fail, delay_count, IQK_ready, cal_retry, cal = 0, temp_reg65;
	int 	TX_X = 0, TX_Y = 0, RX_X = 0, RX_Y = 0, TX_Average = 0, RX_Average = 0;
	int 	TX_X0[cal_num], TX_Y0[cal_num], TX_X0_RXK[cal_num], TX_Y0_RXK[cal_num], RX_X0[cal_num], RX_Y0[cal_num];
	BOOLEAN TX0IQKOK = FALSE, RX0IQKOK = FALSE;
	BOOLEAN VDF_enable = FALSE;
	int 	i, k, VDF_Y[3], VDF_X[3], Tx_dt[3], Rx_dt[3], ii, dx = 0, dy = 0, TX_finish = 0, RX_finish = 0;

	struct _rtw_hal *pHalData = GET_HAL_DATA(rtlpriv);
	struct _rtw_dm *pDM_Odm = &pHalData->odmpriv;

	ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("BandWidth = %d\n", rtlpriv->phy.current_chan_bw));
	if (rtlpriv->phy.current_chan_bw == 2) {
		VDF_enable = TRUE;
	}

	while (cal < cal_num) {
		switch (Path) {
		case RF90_PATH_A:
		    {
			temp_reg65 = rtw_hal_read_rfreg(rtlpriv, Path, 0x65, bMaskDWord);

			if (rtlhal->external_pa_2g) {
				rtl_set_bbreg(rtlpriv, 0x82c, BIT(31), 0x0); /* [31] = 0 --> Page C */
				rtw_hal_write_rfreg(rtlpriv, Path, 0x65, bRFRegOffsetMask, 0x931d5);
			}

			/* Path-A LOK */
			rtl_set_bbreg(rtlpriv, 0x82c, BIT(31), 0x0); /* [31] = 0 --> Page C */
			/* ========Path-A AFE all on======== */
			/* Port 0 DAC/ADC on */
			rtl_write_dword(rtlpriv, 0xc60, 0x77777777);
			rtl_write_dword(rtlpriv, 0xc64, 0x77777777);

			rtl_write_dword(rtlpriv, 0xc68, 0x19791979);
			rtl_write_dword(rtlpriv, 0xc6c, 0x19791979);
			rtl_write_dword(rtlpriv, 0xc70, 0x19791979);
			rtl_write_dword(rtlpriv, 0xc74, 0x19791979);
			rtl_write_dword(rtlpriv, 0xc78, 0x19791979);
			rtl_write_dword(rtlpriv, 0xc7c, 0x19791979);
			rtl_write_dword(rtlpriv, 0xc80, 0x19791979);
			rtl_write_dword(rtlpriv, 0xc84, 0x19791979);

			rtl_set_bbreg(rtlpriv, 0xc00, 0xf, 0x4);	/* 	hardware 3-wire off */

			/* LOK Setting */
			/* ====== LOK ====== */
			/* 1. DAC/ADC sampling rate (160 MHz) */
			rtl_set_bbreg(rtlpriv, 0xc5c, BIT(26)|BIT(25)|BIT(24), 0x7);

			/* 2. LoK RF Setting (at BW = 20M) */
			rtw_hal_write_rfreg(rtlpriv, Path, 0xef, bRFRegOffsetMask, 0x80002);
			rtw_hal_write_rfreg(rtlpriv, Path, 0x18, 0x00c00, 0x3);     /* BW 20M */
			rtw_hal_write_rfreg(rtlpriv, Path, 0x30, bRFRegOffsetMask, 0x20000);
			rtw_hal_write_rfreg(rtlpriv, Path, 0x31, bRFRegOffsetMask, 0x0003f);
			rtw_hal_write_rfreg(rtlpriv, Path, 0x32, bRFRegOffsetMask, 0xf3fc3);
			rtw_hal_write_rfreg(rtlpriv, Path, 0x65, bRFRegOffsetMask, 0x931d5);
			rtw_hal_write_rfreg(rtlpriv, Path, 0x8f, bRFRegOffsetMask, 0x8a001);
			rtl_set_bbreg(rtlpriv, 0xcb8, 0xf, 0xd);
			rtl_write_dword(rtlpriv, 0x90c, 0x00008000);
			rtl_write_dword(rtlpriv, 0xb00, 0x03000100);
			rtl_set_bbreg(rtlpriv, 0xc94, BIT(0), 0x1);
			rtl_write_dword(rtlpriv, 0x978, 0x29002000);	/* TX (X,Y) */
			rtl_write_dword(rtlpriv, 0x97c, 0xa9002000);	/* RX (X,Y) */
			rtl_write_dword(rtlpriv, 0x984, 0x00462910);	/* [0]:AGC_en, [15]:idac_K_Mask */

			rtl_set_bbreg(rtlpriv, 0x82c, BIT(31), 0x1); 	/* [31] = 1 --> Page C1 */

			if (rtlhal->external_pa_2g)
				rtl_write_dword(rtlpriv, 0xc88, 0x821403f7);
			else
				rtl_write_dword(rtlpriv, 0xc88, 0x821403f4);

			if (rtlhal->current_bandtype)
				rtl_write_dword(rtlpriv, 0xc8c, 0x68163e96);
			else
				rtl_write_dword(rtlpriv, 0xc8c, 0x28163e96);

			rtl_write_dword(rtlpriv, 0xc80, 0x18008c10);/* TX_Tone_idx[9:0], TxK_Mask[29] TX_Tone = 16 */
			rtl_write_dword(rtlpriv, 0xc84, 0x38008c10);/* RX_Tone_idx[9:0], RxK_Mask[29] */
			rtl_write_dword(rtlpriv, 0xcb8, 0x00100000);/* cb8[20] ±N SI/PI ¨Ï¥ÎÅv¤Áµ¹ iqk_dpk module */
			rtl_write_dword(rtlpriv, 0x980, 0xfa000000);
			rtl_write_dword(rtlpriv, 0x980, 0xf8000000);

			mdelay(10); /* Delay 10ms */
			rtl_write_dword(rtlpriv, 0xcb8, 0x00000000);

			rtl_set_bbreg(rtlpriv, 0x82c, BIT(31), 0x0); /* [31] = 0 --> Page C */
			rtw_hal_write_rfreg(rtlpriv, Path, 0x58, 0x7fe00, rtw_hal_read_rfreg(rtlpriv, Path, 0x8, 0xffc00)); /* Load LOK */
			switch (rtlpriv->phy.current_chan_bw) {
			case 1:
				rtw_hal_write_rfreg(rtlpriv, Path, 0x18, 0x00c00, 0x1);
				break;
			case 2:
				rtw_hal_write_rfreg(rtlpriv, Path, 0x18, 0x00c00, 0x0);
				break;
			default:
				break;
			}
			rtl_set_bbreg(rtlpriv, 0x82c, BIT(31), 0x1); /* [31] = 1 --> Page C1 */

			/* 3. TX RF Setting */
			rtl_set_bbreg(rtlpriv, 0x82c, BIT(31), 0x0); /* [31] = 0 --> Page C */
			rtw_hal_write_rfreg(rtlpriv, Path, 0xef, bRFRegOffsetMask, 0x80000);
			rtw_hal_write_rfreg(rtlpriv, Path, 0x30, bRFRegOffsetMask, 0x20000);
			rtw_hal_write_rfreg(rtlpriv, Path, 0x31, bRFRegOffsetMask, 0x0003f);
			rtw_hal_write_rfreg(rtlpriv, Path, 0x32, bRFRegOffsetMask, 0xf3fc3);
			rtw_hal_write_rfreg(rtlpriv, Path, 0x65, bRFRegOffsetMask, 0x931d5);
			rtw_hal_write_rfreg(rtlpriv, Path, 0x8f, bRFRegOffsetMask, 0x8a001);
			rtw_hal_write_rfreg(rtlpriv, Path, 0xef, bRFRegOffsetMask, 0x00000);
			rtl_set_bbreg(rtlpriv, 0xcb8, 0xf, 0xd);
			rtl_write_dword(rtlpriv, 0x90c, 0x00008000);
			rtl_write_dword(rtlpriv, 0xb00, 0x03000100);
			rtl_set_bbreg(rtlpriv, 0xc94, BIT(0), 0x1);
			rtl_write_dword(rtlpriv, 0x978, 0x29002000);/* TX (X,Y) */
			rtl_write_dword(rtlpriv, 0x97c, 0xa9002000);/* RX (X,Y) */
			rtl_write_dword(rtlpriv, 0x984, 0x0046a910);/* [0]:AGC_en, [15]:idac_K_Mask */

			rtl_set_bbreg(rtlpriv, 0x82c, BIT(31), 0x1); /* [31] = 1 --> Page C1 */

			if (rtlhal->external_pa_2g)
				rtl_write_dword(rtlpriv, 0xc88, 0x821403f7);
			else
				rtl_write_dword(rtlpriv, 0xc88, 0x821403f1);

			if (rtlhal->current_bandtype)
				rtl_write_dword(rtlpriv, 0xc8c, 0x40163e96);
			else
				rtl_write_dword(rtlpriv, 0xc8c, 0x00163e96);

			if (VDF_enable == 1) {
				DbgPrint("VDF_enable\n");
				for (k = 0; k <= 2; k++) {
					switch (k) {
					case 0:
						rtl_write_dword(rtlpriv, 0xc80, 0x18008c38);/* TX_Tone_idx[9:0], TxK_Mask[29] TX_Tone = 16 */
						rtl_write_dword(rtlpriv, 0xc84, 0x38008c38);/* RX_Tone_idx[9:0], RxK_Mask[29] */
						rtl_set_bbreg(rtlpriv, 0xce8, BIT(31), 0x0);
						break;
					case 1:
						rtl_set_bbreg(rtlpriv, 0xc80, BIT(28), 0x0);
						rtl_set_bbreg(rtlpriv, 0xc84, BIT(28), 0x0);
						rtl_set_bbreg(rtlpriv, 0xce8, BIT(31), 0x0);
						break;
					case 2:
						ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("VDF_Y[1] = %x;;;VDF_Y[0] = %x\n", VDF_Y[1]>>21 & 0x00007ff, VDF_Y[0]>>21 & 0x00007ff));
						ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("VDF_X[1] = %x;;;VDF_X[0] = %x\n", VDF_X[1]>>21 & 0x00007ff, VDF_X[0]>>21 & 0x00007ff));
						Tx_dt[cal] = (VDF_Y[1]>>20)-(VDF_Y[0]>>20);
						Tx_dt[cal] = ((16*Tx_dt[cal])*10000/15708);
						Tx_dt[cal] = (Tx_dt[cal] >> 1)+(Tx_dt[cal] & BIT(0));
						rtl_write_dword(rtlpriv, 0xc80, 0x18008c20);/* TX_Tone_idx[9:0], TxK_Mask[29] TX_Tone = 16 */
						rtl_write_dword(rtlpriv, 0xc84, 0x38008c20);/* RX_Tone_idx[9:0], RxK_Mask[29] */
						rtl_set_bbreg(rtlpriv, 0xce8, BIT(31), 0x1);
						rtl_set_bbreg(rtlpriv, 0xce8, 0x3fff0000, Tx_dt[cal] & 0x00003fff);
						break;
					default:
						break;
					}
					rtl_write_dword(rtlpriv, 0xcb8, 0x00100000);	/* cb8[20] ±N SI/PI ¨Ï¥ÎÅv¤Áµ¹ iqk_dpk module */
					cal_retry = 0;
					while (1) {
						/* one shot */
						rtl_write_dword(rtlpriv, 0x980, 0xfa000000);
						rtl_write_dword(rtlpriv, 0x980, 0xf8000000);

						mdelay(10); 	/* Delay 10ms */
						rtl_write_dword(rtlpriv, 0xcb8, 0x00000000);
						delay_count = 0;
						while (1) {
							IQK_ready = rtl_get_bbreg(rtlpriv, 0xd00, BIT(10));
							if ((~IQK_ready) || (delay_count > 20)) {
								break;
							} else {
								mdelay(1);
								delay_count++;
							}
						}

						if (delay_count < 20) {			/* If 20ms No Result, then cal_retry++ */
						/* ============TXIQK Check============== */
							TX_fail = rtl_get_bbreg(rtlpriv, 0xd00, BIT(12));

							if (~TX_fail) {
								rtl_write_dword(rtlpriv, 0xcb8, 0x02000000);
								VDF_X[k] = rtl_get_bbreg(rtlpriv, 0xd00, 0x07ff0000)<<21;
								rtl_write_dword(rtlpriv, 0xcb8, 0x04000000);
								VDF_Y[k] = rtl_get_bbreg(rtlpriv, 0xd00, 0x07ff0000)<<21;
								TX0IQKOK = TRUE;
								break;
							} else {
								rtl_set_bbreg(rtlpriv, 0xccc, 0x000007ff, 0x0);
								rtl_set_bbreg(rtlpriv, 0xcd4, 0x000007ff, 0x200);
								TX0IQKOK = FALSE;
								cal_retry++;
								if (cal_retry == 10) {
									break;
								}
							}
						} else {
							TX0IQKOK = FALSE;
							cal_retry++;
							if (cal_retry == 10) {
								break;
							}
						}
					}
				}

				if (k == 3) {
					TX_X0[cal] = VDF_X[k-1] ;
					TX_Y0[cal] = VDF_Y[k-1];
				}
			} else {
				rtl_write_dword(rtlpriv, 0xc80, 0x18008c10);	/* TX_Tone_idx[9:0], TxK_Mask[29] TX_Tone = 16 */
				rtl_write_dword(rtlpriv, 0xc84, 0x38008c10);	/* RX_Tone_idx[9:0], RxK_Mask[29]  */
				rtl_write_dword(rtlpriv, 0xcb8, 0x00100000);	/* cb8[20] ±N SI/PI ¨Ï¥ÎÅv¤Áµ¹ iqk_dpk module */
				cal_retry = 0;
				while (1) {
					/* one shot */
					rtl_write_dword(rtlpriv, 0x980, 0xfa000000);
					rtl_write_dword(rtlpriv, 0x980, 0xf8000000);

					mdelay(10); /*  Delay 10ms */
					rtl_write_dword(rtlpriv, 0xcb8, 0x00000000);
					delay_count = 0;
					while (1) {
						IQK_ready = rtl_get_bbreg(rtlpriv, 0xd00, BIT(10));
						if ((~IQK_ready) || (delay_count > 20)) {
							break;
						} else {
							mdelay(1);
							delay_count++;
						}
					}

					if (delay_count < 20) {		/* If 20ms No Result, then cal_retry++ */
						/* ============TXIQK Check============== */
						TX_fail = rtl_get_bbreg(rtlpriv, 0xd00, BIT(12));
						if (~TX_fail) {
							rtl_write_dword(rtlpriv, 0xcb8, 0x02000000);
							TX_X0[cal] = rtl_get_bbreg(rtlpriv, 0xd00, 0x07ff0000)<<21;
							rtl_write_dword(rtlpriv, 0xcb8, 0x04000000);
							TX_Y0[cal] = rtl_get_bbreg(rtlpriv, 0xd00, 0x07ff0000)<<21;
							TX0IQKOK = TRUE;
							break;
						} else {
							rtl_set_bbreg(rtlpriv, 0xccc, 0x000007ff, 0x0);
							rtl_set_bbreg(rtlpriv, 0xcd4, 0x000007ff, 0x200);
							TX0IQKOK = FALSE;
							cal_retry++;
							if (cal_retry == 10) {
								break;
							}
						}
					} else {
						TX0IQKOK = FALSE;
						cal_retry++;
						if (cal_retry == 10)
							break;
					}
				}
			}

			if (TX0IQKOK == FALSE)
				break;					/* TXK fail, Don't do RXK */

			if (VDF_enable == 1) {
				rtl_set_bbreg(rtlpriv, 0xce8, BIT(31), 0x0);    /* TX VDF Disable */
				ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("RXVDF Start\n"));
				for (k = 0; k <= 2; k++) {
					/* ====== RX mode TXK (RXK Step 1) ====== */
					rtl_set_bbreg(rtlpriv, 0x82c, BIT(31), 0x0); /* [31] = 0 --> Page C */
					/*  1. TX RF Setting */
					rtw_hal_write_rfreg(rtlpriv, Path, 0xef, bRFRegOffsetMask, 0x80000);
					rtw_hal_write_rfreg(rtlpriv, Path, 0x30, bRFRegOffsetMask, 0x30000);
					rtw_hal_write_rfreg(rtlpriv, Path, 0x31, bRFRegOffsetMask, 0x00029);
					rtw_hal_write_rfreg(rtlpriv, Path, 0x32, bRFRegOffsetMask, 0xd7ffb);
					rtw_hal_write_rfreg(rtlpriv, Path, 0x65, bRFRegOffsetMask, temp_reg65);
					rtw_hal_write_rfreg(rtlpriv, Path, 0x8f, bRFRegOffsetMask, 0x8a001);
					rtw_hal_write_rfreg(rtlpriv, Path, 0xef, bRFRegOffsetMask, 0x00000);

					rtl_set_bbreg(rtlpriv, 0xcb8, 0xf, 0xd);
					rtl_write_dword(rtlpriv, 0x978, 0x29002000);/* TX (X,Y) */
					rtl_write_dword(rtlpriv, 0x97c, 0xa9002000);/* RX (X,Y) */
					rtl_write_dword(rtlpriv, 0x984, 0x0046a910);/* [0]:AGC_en, [15]:idac_K_Mask */
					rtl_write_dword(rtlpriv, 0x90c, 0x00008000);
					rtl_write_dword(rtlpriv, 0xb00, 0x03000100);
					rtl_set_bbreg(rtlpriv, 0x82c, BIT(31), 0x1); /* [31] = 1 --> Page C1 */
					switch (k) {
					case 0:
						rtl_write_dword(rtlpriv, 0xc80, 0x18008c38);	/* TX_Tone_idx[9:0], TxK_Mask[29] TX_Tone = 16 */
						rtl_write_dword(rtlpriv, 0xc84, 0x38008c38);	/* RX_Tone_idx[9:0], RxK_Mask[29] */
						rtl_set_bbreg(rtlpriv, 0xce8, BIT(30), 0x0);
						break;
					case 1:
						rtl_write_dword(rtlpriv, 0xc80, 0x08008c38);	/* TX_Tone_idx[9:0], TxK_Mask[29] TX_Tone = 16 */
						rtl_write_dword(rtlpriv, 0xc84, 0x28008c38);	/* RX_Tone_idx[9:0], RxK_Mask[29] */
						rtl_set_bbreg(rtlpriv, 0xce8, BIT(30), 0x0);
						break;
					case 2:
						ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("VDF_Y[1] = %x;;;VDF_Y[0] = %x\n", VDF_Y[1]>>21 & 0x00007ff, VDF_Y[0]>>21 & 0x00007ff));
						ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("VDF_X[1] = %x;;;VDF_X[0] = %x\n", VDF_X[1]>>21 & 0x00007ff, VDF_X[0]>>21 & 0x00007ff));
						Rx_dt[cal] = (VDF_Y[1]>>20)-(VDF_Y[0]>>20);
						ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("Rx_dt = %d\n", Rx_dt[cal]));
						Rx_dt[cal] = ((16*Rx_dt[cal])*10000/13823);
						Rx_dt[cal] = (Rx_dt[cal] >> 1)+(Rx_dt[cal] & BIT(0));
						rtl_write_dword(rtlpriv, 0xc80, 0x18008c20);	/* TX_Tone_idx[9:0], TxK_Mask[29] TX_Tone = 16 */
						rtl_write_dword(rtlpriv, 0xc84, 0x38008c20);	/* RX_Tone_idx[9:0], RxK_Mask[29] */
						rtl_set_bbreg(rtlpriv, 0xce8, 0x00003fff, Rx_dt[cal] & 0x00003fff);
						break;
					default:
						break;
					}

					rtl_write_dword(rtlpriv, 0xc88, 0x821603e0);
					rtl_write_dword(rtlpriv, 0xc8c, 0x68163e96);
					rtl_write_dword(rtlpriv, 0xcb8, 0x00100000);	/* cb8[20] ±N SI/PI ¨Ï¥ÎÅv¤Áµ¹ iqk_dpk module */
					cal_retry = 0;
					while (1) {
						/* one shot */
						rtl_write_dword(rtlpriv, 0x980, 0xfa000000);
						rtl_write_dword(rtlpriv, 0x980, 0xf8000000);

						mdelay(10); 	/* Delay 10ms */
						rtl_write_dword(rtlpriv, 0xcb8, 0x00000000);
						delay_count = 0;
						while (1) {
							IQK_ready = rtl_get_bbreg(rtlpriv, 0xd00, BIT(10));
							if ((~IQK_ready) || (delay_count > 20)) {
								break;
							} else {
								mdelay(1);
								delay_count++;
							}
						}

						if (delay_count < 20) {		/* If 20ms No Result, then cal_retry++ */
							/* ============TXIQK Check============== */
							TX_fail = rtl_get_bbreg(rtlpriv, 0xd00, BIT(12));

							if (~TX_fail) {
								rtl_write_dword(rtlpriv, 0xcb8, 0x02000000);
								TX_X0_RXK[cal] = rtl_get_bbreg(rtlpriv, 0xd00, 0x07ff0000)<<21;
								rtl_write_dword(rtlpriv, 0xcb8, 0x04000000);
								TX_Y0_RXK[cal] = rtl_get_bbreg(rtlpriv, 0xd00, 0x07ff0000)<<21;
								TX0IQKOK = TRUE;
								break;
							} else {
								TX0IQKOK = FALSE;
								cal_retry++;
								if (cal_retry == 10)
									break;
							}
						} else {
							TX0IQKOK = FALSE;
							cal_retry++;
							if (cal_retry == 10)
								break;
						}
					}

					if (TX0IQKOK == FALSE) {   /* If RX mode TXK fail, then take TXK Result */
						TX_X0_RXK[cal] = TX_X0[cal];
						TX_Y0_RXK[cal] = TX_Y0[cal];
						TX0IQKOK = TRUE;
						ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("RXK Step 1 fail\n"));
					}

					/* ====== RX IQK ====== */
					rtl_set_bbreg(rtlpriv, 0x82c, BIT(31), 0x0); /* [31] = 0 --> Page C */
					/* 1. RX RF Setting */
					rtw_hal_write_rfreg(rtlpriv, Path, 0xef, bRFRegOffsetMask, 0x80000);
					rtw_hal_write_rfreg(rtlpriv, Path, 0x30, bRFRegOffsetMask, 0x30000);
					rtw_hal_write_rfreg(rtlpriv, Path, 0x31, bRFRegOffsetMask, 0x0002f);
					rtw_hal_write_rfreg(rtlpriv, Path, 0x32, bRFRegOffsetMask, 0xfffbb);
					rtw_hal_write_rfreg(rtlpriv, Path, 0x8f, bRFRegOffsetMask, 0x88001);
					rtw_hal_write_rfreg(rtlpriv, Path, 0x65, bRFRegOffsetMask, 0x931d8);
					rtw_hal_write_rfreg(rtlpriv, Path, 0xef, bRFRegOffsetMask, 0x00000);

					rtl_set_bbreg(rtlpriv, 0x978, 0x03FF8000, (TX_X0_RXK[cal])>>21&0x000007ff);
					rtl_set_bbreg(rtlpriv, 0x978, 0x000007FF, (TX_Y0_RXK[cal])>>21&0x000007ff);
					rtl_set_bbreg(rtlpriv, 0x978, BIT(31), 0x1);
					rtl_set_bbreg(rtlpriv, 0x97c, BIT(31), 0x0);
					rtl_set_bbreg(rtlpriv, 0xcb8, 0xF, 0xe);
					rtl_write_dword(rtlpriv, 0x90c, 0x00008000);
					rtl_write_dword(rtlpriv, 0x984, 0x0046a911);

					rtl_set_bbreg(rtlpriv, 0x82c, BIT(31), 0x1); /* [31] = 1 --> Page C1 */
					rtl_set_bbreg(rtlpriv, 0xc80, BIT(29), 0x1);
					rtl_set_bbreg(rtlpriv, 0xc84, BIT(29), 0x0);
					rtl_write_dword(rtlpriv, 0xc88, 0x02140119);
					rtl_write_dword(rtlpriv, 0xc8c, 0x28161420);

					if (k == 2) {
						rtl_set_bbreg(rtlpriv, 0xce8, BIT(30), 0x1);  /* RX VDF Enable */
					}
					rtl_write_dword(rtlpriv, 0xcb8, 0x00100000);	/* cb8[20] ±N SI/PI ¨Ï¥ÎÅv¤Áµ¹ iqk_dpk module */

					cal_retry = 0;
					while (1) {
						/* one shot */
						rtl_write_dword(rtlpriv, 0x980, 0xfa000000);
						rtl_write_dword(rtlpriv, 0x980, 0xf8000000);

						mdelay(10); /* Delay 10ms */
						rtl_write_dword(rtlpriv, 0xcb8, 0x00000000);
						delay_count = 0;
						while (1) {
							IQK_ready = rtl_get_bbreg(rtlpriv, 0xd00, BIT(10));
							if ((~IQK_ready) || (delay_count > 20)) {
								break;
							} else {
								mdelay(1);
								delay_count++;
							}
						}

						if (delay_count < 20) {	/* If 20ms No Result, then cal_retry++ */
							/* ============RXIQK Check============== */
							RX_fail = rtl_get_bbreg(rtlpriv, 0xd00, BIT(11));
							if (RX_fail == 0) {
								rtl_write_dword(rtlpriv, 0xcb8, 0x06000000);
								VDF_X[k] = rtl_get_bbreg(rtlpriv, 0xd00, 0x07ff0000)<<21;
								rtl_write_dword(rtlpriv, 0xcb8, 0x08000000);
								VDF_Y[k] = rtl_get_bbreg(rtlpriv, 0xd00, 0x07ff0000)<<21;
								RX0IQKOK = TRUE;
								break;
							} else {
								rtl_set_bbreg(rtlpriv, 0xc10, 0x000003ff, 0x200>>1);
								rtl_set_bbreg(rtlpriv, 0xc10, 0x03ff0000, 0x0>>1);
								RX0IQKOK = FALSE;
								cal_retry++;
								if (cal_retry == 10)
									break;
							}
						} else {
							RX0IQKOK = FALSE;
							cal_retry++;
							if (cal_retry == 10)
								break;
						}
					}
				}

				if (k == 3) {
					RX_X0[cal] = VDF_X[k-1] ;
					RX_Y0[cal] = VDF_Y[k-1];
				}

				rtl_set_bbreg(rtlpriv, 0xce8, BIT(31), 0x1);    /* TX VDF Enable */
			} else {
				/* ====== RX mode TXK (RXK Step 1) ====== */
				rtl_set_bbreg(rtlpriv, 0x82c, BIT(31), 0x0); /* [31] = 0 --> Page C */
				/* 1. TX RF Setting */
				rtw_hal_write_rfreg(rtlpriv, Path, 0xef, bRFRegOffsetMask, 0x80000);
				rtw_hal_write_rfreg(rtlpriv, Path, 0x30, bRFRegOffsetMask, 0x30000);
				rtw_hal_write_rfreg(rtlpriv, Path, 0x31, bRFRegOffsetMask, 0x00029);
				rtw_hal_write_rfreg(rtlpriv, Path, 0x32, bRFRegOffsetMask, 0xd7ffb);
				rtw_hal_write_rfreg(rtlpriv, Path, 0x65, bRFRegOffsetMask, temp_reg65);
				rtw_hal_write_rfreg(rtlpriv, Path, 0x8f, bRFRegOffsetMask, 0x8a001);
				rtw_hal_write_rfreg(rtlpriv, Path, 0xef, bRFRegOffsetMask, 0x00000);
				rtl_write_dword(rtlpriv, 0x90c, 0x00008000);
				rtl_write_dword(rtlpriv, 0xb00, 0x03000100);
				rtl_write_dword(rtlpriv, 0x984, 0x0046a910);/* [0]:AGC_en, [15]:idac_K_Mask */

				rtl_set_bbreg(rtlpriv, 0x82c, BIT(31), 0x1);	/* [31] = 1 --> Page C1 */
				rtl_write_dword(rtlpriv, 0xc80, 0x18008c10);	/* TX_Tone_idx[9:0], TxK_Mask[29] TX_Tone = 16 */
				rtl_write_dword(rtlpriv, 0xc84, 0x38008c10);	/* RX_Tone_idx[9:0], RxK_Mask[29] */
				rtl_write_dword(rtlpriv, 0xc88, 0x821603e0);
				/* rtl_write_dword(rtlpriv, 0xc8c, 0x68163e96); */
				rtl_write_dword(rtlpriv, 0xcb8, 0x00100000);/* cb8[20] ±N SI/PI ¨Ï¥ÎÅv¤Áµ¹ iqk_dpk module */
				cal_retry = 0;
				while (1) {
					/* one shot */
					rtl_write_dword(rtlpriv, 0x980, 0xfa000000);
					rtl_write_dword(rtlpriv, 0x980, 0xf8000000);

					mdelay(10); 	/* Delay 10ms */
					rtl_write_dword(rtlpriv, 0xcb8, 0x00000000);
					delay_count = 0;
					while (1) {
						IQK_ready = rtl_get_bbreg(rtlpriv, 0xd00, BIT(10));
						if ((~IQK_ready) || (delay_count > 20)) {
							break;
						} else {
							mdelay(1);
							delay_count++;
						}
					}

					if (delay_count < 20) {							/* If 20ms No Result, then cal_retry++ */
						/* ============TXIQK Check============== */
						TX_fail = rtl_get_bbreg(rtlpriv, 0xd00, BIT(12));

						if (~TX_fail) {
							rtl_write_dword(rtlpriv, 0xcb8, 0x02000000);
							TX_X0_RXK[cal] = rtl_get_bbreg(rtlpriv, 0xd00, 0x07ff0000)<<21;
							rtl_write_dword(rtlpriv, 0xcb8, 0x04000000);
							TX_Y0_RXK[cal] = rtl_get_bbreg(rtlpriv, 0xd00, 0x07ff0000)<<21;
							TX0IQKOK = TRUE;
							break;
						} else {
							TX0IQKOK = FALSE;
							cal_retry++;
							if (cal_retry == 10)
								break;
						}
					} else {
						TX0IQKOK = FALSE;
						cal_retry++;
						if (cal_retry == 10)
							break;
					}
				}

				if (TX0IQKOK == FALSE) {	/* If RX mode TXK fail, then take TXK Result */
					TX_X0_RXK[cal] = TX_X0[cal];
					TX_Y0_RXK[cal] = TX_Y0[cal];
					TX0IQKOK = TRUE;
					ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("1"));
				}

				/* ====== RX IQK ====== */
				rtl_set_bbreg(rtlpriv, 0x82c, BIT(31), 0x0); /* [31] = 0 --> Page C */
				/* 1. RX RF Setting */
				rtw_hal_write_rfreg(rtlpriv, Path, 0xef, bRFRegOffsetMask, 0x80000);
				rtw_hal_write_rfreg(rtlpriv, Path, 0x30, bRFRegOffsetMask, 0x30000);
				rtw_hal_write_rfreg(rtlpriv, Path, 0x31, bRFRegOffsetMask, 0x0002f);
				rtw_hal_write_rfreg(rtlpriv, Path, 0x32, bRFRegOffsetMask, 0xfffbb);
				rtw_hal_write_rfreg(rtlpriv, Path, 0x8f, bRFRegOffsetMask, 0x88001);
				rtw_hal_write_rfreg(rtlpriv, Path, 0x65, bRFRegOffsetMask, 0x931d8);
				rtw_hal_write_rfreg(rtlpriv, Path, 0xef, bRFRegOffsetMask, 0x00000);

				rtl_set_bbreg(rtlpriv, 0x978, 0x03FF8000, (TX_X0_RXK[cal])>>21&0x000007ff);
				rtl_set_bbreg(rtlpriv, 0x978, 0x000007FF, (TX_Y0_RXK[cal])>>21&0x000007ff);
				rtl_set_bbreg(rtlpriv, 0x978, BIT(31), 0x1);
				rtl_set_bbreg(rtlpriv, 0x97c, BIT(31), 0x0);
				rtl_set_bbreg(rtlpriv, 0xcb8, 0xF, 0xe);
				rtl_write_dword(rtlpriv, 0x90c, 0x00008000);
				rtl_write_dword(rtlpriv, 0x984, 0x0046a911);

				rtl_set_bbreg(rtlpriv, 0x82c, BIT(31), 0x1); 	/* [31] = 1 --> Page C1 */
				rtl_write_dword(rtlpriv, 0xc80, 0x38008c10);	/* TX_Tone_idx[9:0], TxK_Mask[29] TX_Tone = 16 */
				rtl_write_dword(rtlpriv, 0xc84, 0x18008c10);	/* RX_Tone_idx[9:0], RxK_Mask[29] */
				rtl_write_dword(rtlpriv, 0xc88, 0x02140119);
				rtl_write_dword(rtlpriv, 0xc8c, 0x28161440);
				rtl_write_dword(rtlpriv, 0xcb8, 0x00100000);	/* cb8[20] ±N SI/PI ¨Ï¥ÎÅv¤Áµ¹ iqk_dpk module */

				cal_retry = 0;
				while (1) {
					/* one shot */
					rtl_write_dword(rtlpriv, 0x980, 0xfa000000);
					rtl_write_dword(rtlpriv, 0x980, 0xf8000000);

					mdelay(10); /* Delay 10ms */
					rtl_write_dword(rtlpriv, 0xcb8, 0x00000000);
					delay_count = 0;
					while (1) {
						IQK_ready = rtl_get_bbreg(rtlpriv, 0xd00, BIT(10));
						if ((~IQK_ready) || (delay_count > 20)) {
							break;
						} else {
							mdelay(1);
							delay_count++;
						}
					}

					if (delay_count < 20) {	/* If 20ms No Result, then cal_retry++ */
						/* ============RXIQK Check============== */
						RX_fail = rtl_get_bbreg(rtlpriv, 0xd00, BIT(11));
						if (RX_fail == 0) {
							rtl_write_dword(rtlpriv, 0xcb8, 0x06000000);
							RX_X0[cal] = rtl_get_bbreg(rtlpriv, 0xd00, 0x07ff0000)<<21;
							rtl_write_dword(rtlpriv, 0xcb8, 0x08000000);
							RX_Y0[cal] = rtl_get_bbreg(rtlpriv, 0xd00, 0x07ff0000)<<21;
							RX0IQKOK = TRUE;
							break;
						} else {
							rtl_set_bbreg(rtlpriv, 0xc10, 0x000003ff, 0x200>>1);
							rtl_set_bbreg(rtlpriv, 0xc10, 0x03ff0000, 0x0>>1);
							RX0IQKOK = FALSE;
							cal_retry++;
							if (cal_retry == 10)
								break;

						}
					} else {
						RX0IQKOK = FALSE;
						cal_retry++;
						if (cal_retry == 10)
							break;
					}
				}
			}
			if (TX0IQKOK)
				TX_Average++;
			if (RX0IQKOK)
				RX_Average++;
			rtl_set_bbreg(rtlpriv, 0x82c, BIT(31), 0x0); 	/* [31] = 0 --> Page C */
			rtw_hal_write_rfreg(rtlpriv, Path, 0x65, bRFRegOffsetMask, temp_reg65);
		    }
			break;
		default:
			break;
		}
		cal++;
	}
	/* FillIQK Result */
	switch (Path) {
	case RF90_PATH_A:
	    {
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("========Path_A =======\n"));
		if (TX_Average == 0)
			break;

		for (i = 0; i < TX_Average; i++) {
			ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, (" TX_X0_RXK[%d] = %x ;; TX_Y0_RXK[%d] = %x\n", i, (TX_X0_RXK[i])>>21&0x000007ff, i, (TX_Y0_RXK[i])>>21&0x000007ff));
			ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("TX_X0[%d] = %x ;; TX_Y0[%d] = %x\n", i, (TX_X0[i])>>21&0x000007ff, i, (TX_Y0[i])>>21&0x000007ff));
		}

		for (i = 0; i < TX_Average; i++) {
			for (ii = i+1; ii < TX_Average; ii++) {
				dx = (TX_X0[i]>>21) - (TX_X0[ii]>>21);
				if (dx < 3 && dx > -3) {
					dy = (TX_Y0[i]>>21) - (TX_Y0[ii]>>21);
					if (dy < 3 && dy > -3) {
						TX_X = ((TX_X0[i]>>21) + (TX_X0[ii]>>21))/2;
						TX_Y = ((TX_Y0[i]>>21) + (TX_Y0[ii]>>21))/2;
						TX_finish = 1;
						break;
					}
				}
			}
			if (TX_finish == 1)
				break;
		}

		if (TX_finish == 1) {
			_rtl8821au_iqk_tx_fill_iqc(rtlpriv, Path, TX_X, TX_Y);
		} else {
			_rtl8821au_iqk_tx_fill_iqc(rtlpriv, Path, 0x200, 0x0);
		}

		if (RX_Average == 0)
			break;

		for (i = 0; i < RX_Average; i++) {
			ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("RX_X0[%d] = %x ;; RX_Y0[%d] = %x\n", i, (RX_X0[i])>>21&0x000007ff, i, (RX_Y0[i])>>21&0x000007ff));
		}

		for (i = 0; i < RX_Average; i++) {
			for (ii = i+1; ii < RX_Average; ii++) {
				dx = (RX_X0[i]>>21) - (RX_X0[ii]>>21);
				if (dx < 3 && dx > -3) {
					dy = (RX_Y0[i]>>21) - (RX_Y0[ii]>>21);
					if (dy < 3 && dy > -3) {
						RX_X = ((RX_X0[i]>>21) + (RX_X0[ii]>>21))/2;
						RX_Y = ((RX_Y0[i]>>21) + (RX_Y0[ii]>>21))/2;
						RX_finish = 1;
						break;
					}
				}
			}
			if (RX_finish == 1)
				break;
		}

		if (RX_finish == 1) {
			 _rtl8821au_iqk_rx_fill_iqc(rtlpriv, Path, RX_X, RX_Y);
		} else {
			 _rtl8821au_iqk_rx_fill_iqc(rtlpriv, Path, 0x200, 0x0);
		}
	    }
		break;
	default:
		break;
	}
}

#undef MACBB_REG_NUM
#undef AFE_REG_NUM
#undef RF_REG_NUM 
#define MACBB_REG_NUM 11
#define AFE_REG_NUM 12
#define RF_REG_NUM 3


static void _rtl8821au_phy_iq_calibrate(struct rtl_priv *rtlpriv)
{
	u32 macbb_backup[MACBB_REG_NUM];
	u32 afe_backup[AFE_REG_NUM];
	u32 rfa_backup[RF_REG_NUM];
	u32 rfb_backup[RF_REG_NUM];
	u32 backup_macbb_reg[MACBB_REG_NUM] = {
		0xb00, 0x520, 0x550, 0x808, 0x90c, 0xc00, 0xc50,
		0xe00, 0xe50, 0x838, 0x82c
	};
	u32 backup_afe_reg[AFE_REG_NUM] = {
		0xc5c, 0xc60, 0xc64, 0xc68, 0xc6c, 0xc70, 0xc74,
		0xc78, 0xc7c, 0xc80, 0xc84, 0xcb8
	};
	u32 backup_rf_reg[RF_REG_NUM] = { 0x65, 0x8f, 0x0 };

	 struct _rtw_hal *pHalData = GET_HAL_DATA(rtlpriv);
	struct _rtw_dm *pDM_Odm = &pHalData->odmpriv;

	_rtl8821au_iqk_backup_macbb(rtlpriv, macbb_backup, backup_macbb_reg,
				    MACBB_REG_NUM);
	_rtl8821au_iqk_backup_afe(rtlpriv, afe_backup, backup_afe_reg, AFE_REG_NUM);
	_rtl8821au_iqk_backup_rf(rtlpriv, rfa_backup, rfb_backup, backup_rf_reg,
				 RF_REG_NUM);

	_rtl8821au_iqk_configure_mac(rtlpriv);
	_rtl8821au_iqk_tx(rtlpriv, RF90_PATH_A);
	_rtl8821au_iqk_restore_rf(rtlpriv, RF90_PATH_A, backup_rf_reg, rfa_backup,
				 RF_REG_NUM);

	_rtl8821au_iqk_restore_afe(rtlpriv, afe_backup, backup_afe_reg, AFE_REG_NUM);
	_rtl8821au_iqk_restore_macbb(rtlpriv, macbb_backup, backup_macbb_reg,
				     MACBB_REG_NUM);

	/* _IQK_Exit_8821A(pDM_Odm); */
	/* _IQK_TX_CheckResult_8821A */
}

void rtl8821au_phy_iq_calibrate(struct rtl_priv *rtlpriv, BOOLEAN bReCovery)
{
	_rtl8821au_phy_iq_calibrate(rtlpriv);
}


/* ********************************************************** */

/* OLD functions need complete ? rewrite */
BOOLEAN Getu8IntegerFromStringInDecimal(s8 *Str, uint8_t *pInt)
{
	u16 i = 0;
	*pInt = 0;

	while (Str[i] != '\0') {
		if (Str[i] >= '0' && Str[i] <= '9') {
			*pInt *= 10;
			*pInt += (Str[i] - '0');
		} else {
			return _FALSE;
		}
		++i;
	}

	return _TRUE;
}


static BOOLEAN eqNByte(uint8_t *str1, uint8_t *str2, uint32_t num)
{
	if (num == 0)
		return _FALSE;

	while (num > 0) {
		num--;
		if (str1[num] != str2[num])
			return _FALSE;
	}

	return _TRUE;
}


static s8 phy_GetChannelGroup(enum band_type Band, uint8_t Channel)
{
	s8 channelGroup = -1;

	if (Channel <= 14 && Band == BAND_ON_2_4G) {
		if (1 <= Channel && Channel <= 2)
			channelGroup = 0;
		else if (3  <= Channel && Channel <= 5)
			channelGroup = 1;
		else if (6  <= Channel && Channel <= 8)
			channelGroup = 2;
		else if (9  <= Channel && Channel <= 11)
			channelGroup = 3;
		else if (12 <= Channel && Channel <= 14)
			channelGroup = 4;
		else {
			DBG_871X("==> phy_GetChannelGroup() in 2.4 G, but Channel %d in Group not found \n", Channel);
			channelGroup = -1;
		}
	} else if (Band == BAND_ON_5G) {
		if (36 <= Channel && Channel <=  42)
			channelGroup = 0;
		else if (44 <= Channel && Channel <= 48)
			channelGroup = 1;
		else if (50 <= Channel && Channel <= 58)
			channelGroup = 2;
		else if (60 <= Channel && Channel <= 64)
			channelGroup = 3;
		else if (100 <= Channel && Channel <= 106)
			channelGroup = 4;
		else if (108 <= Channel && Channel <= 114)
			channelGroup = 5;
		else if (116 <= Channel && Channel <= 122)
			channelGroup = 6;
		else if (124 <= Channel && Channel <= 130)
			channelGroup = 7;
		else if (132 <= Channel && Channel <= 138)
			channelGroup = 8;
		else if (140 <= Channel && Channel <= 144)
			channelGroup = 9;
		else if (149 <= Channel && Channel <= 155)
			channelGroup = 10;
		else if (157 <= Channel && Channel <= 161)
			channelGroup = 11;
		else if (165 <= Channel && Channel <= 171)
			channelGroup = 12;
		else if (173 <= Channel && Channel <= 177)
			channelGroup = 13;
		else {
			DBG_871X("==>phy_GetChannelGroup() in 5G, but Channel %d in Group not found \n", Channel);
			channelGroup = -1;
		}
	} else {
		DBG_871X("==>phy_GetChannelGroup() in unsupported band %d\n", Band);
		channelGroup = -1;
	}

	return channelGroup;
}

static u8 _rtl8821au_phy_get_txpower_limit(struct rtl_priv *rtlpriv,
					   enum band_type Band,
					   enum CHANNEL_WIDTH Bandwidth,
					   enum radio_path RfPath,
					   uint8_t DataRate, uint8_t Channel)
{
	struct rtl_phy *rtlphy = rtl_phy(rtlpriv);
	struct rtl_efuse *efuse = rtl_efuse(rtlpriv);

	short band = -1, regulation = -1, bandwidth = -1,
		rfPath = -1, rateSection = -1, channelGroup = -1;
	uint8_t	powerLimit = MAX_POWER_INDEX;

	if ((rtlpriv->registrypriv.RegEnableTxPowerLimit == 0 && efuse->eeprom_regulatory != 1)
	   || efuse->eeprom_regulatory == 2)
		return MAX_POWER_INDEX;

	regulation = TXPWR_LMT_FCC;

	if (Band == BAND_ON_2_4G)
		band = 0;
	else if (Band == BAND_ON_5G)
		band = 1;

	if (Bandwidth == CHANNEL_WIDTH_20)
		bandwidth = 0;
	else if (Bandwidth == CHANNEL_WIDTH_40)
		bandwidth = 1;
	else if (Bandwidth == CHANNEL_WIDTH_80)
		bandwidth = 2;
	else if (Bandwidth == CHANNEL_WIDTH_160)
		bandwidth = 3;

	switch (DataRate) {
	case MGN_1M:
	case MGN_2M:
	case MGN_5_5M:
	case MGN_11M:
		rateSection = 0;
		break;

	case MGN_6M:
	case MGN_9M:
	case MGN_12M:
	case MGN_18M:
	case MGN_24M:
	case MGN_36M:
	case MGN_48M:
	case MGN_54M:
		rateSection = 1;
		break;

	case MGN_MCS0:
	case MGN_MCS1:
	case MGN_MCS2:
	case MGN_MCS3:
	case MGN_MCS4:
	case MGN_MCS5:
	case MGN_MCS6:
	case MGN_MCS7:
		rateSection = 2;
		break;

	case MGN_MCS8:
	case MGN_MCS9:
	case MGN_MCS10:
	case MGN_MCS11:
	case MGN_MCS12:
	case MGN_MCS13:
	case MGN_MCS14:
	case MGN_MCS15:
		rateSection = 3;
		break;

	case MGN_VHT1SS_MCS0:
	case MGN_VHT1SS_MCS1:
	case MGN_VHT1SS_MCS2:
	case MGN_VHT1SS_MCS3:
	case MGN_VHT1SS_MCS4:
	case MGN_VHT1SS_MCS5:
	case MGN_VHT1SS_MCS6:
	case MGN_VHT1SS_MCS7:
	case MGN_VHT1SS_MCS8:
	case MGN_VHT1SS_MCS9:
		rateSection = 4;
		break;

	case MGN_VHT2SS_MCS0:
	case MGN_VHT2SS_MCS1:
	case MGN_VHT2SS_MCS2:
	case MGN_VHT2SS_MCS3:
	case MGN_VHT2SS_MCS4:
	case MGN_VHT2SS_MCS5:
	case MGN_VHT2SS_MCS6:
	case MGN_VHT2SS_MCS7:
	case MGN_VHT2SS_MCS8:
	case MGN_VHT2SS_MCS9:
		rateSection = 5;
		break;

	default:
		DBG_871X("Wrong rate 0x%x\n", DataRate);
		break;
	}

	if (Band == BAND_ON_2_4G  && rateSection > 3)
		DBG_871X("Wrong rate 0x%x: No VHT in 2.4G Band\n", DataRate);
	if (Band == BAND_ON_5G  && rateSection == 0)
		DBG_871X("Wrong rate 0x%x: No CCK in 5G Band\n", DataRate);

	/*
	 * workaround for wrong index combination to obtain tx power limit,
	 * OFDM only exists in BW 20M
	 */
	if (rateSection == 1)
		bandwidth = 0;

	/*
	 * workaround for wrong indxe combination to obtain tx power limit,
	 * HT on 80M will reference to HT on 40M
	 */
	if ((rateSection == 2 || rateSection == 3) && Band == BAND_ON_5G && bandwidth == 2) {
		bandwidth = 1;
	}

	if (Band == BAND_ON_2_4G)
		channelGroup = phy_GetChannelGroup(BAND_ON_2_4G, Channel);
	else if (Band == BAND_ON_5G)
		channelGroup = phy_GetChannelGroup(BAND_ON_5G, Channel);
	else if (Band == BAND_ON_BOTH) {
		/* BAND_ON_BOTH don't care temporarily */
	}

	if (band == -1 || regulation == -1 || bandwidth == -1 ||
	     rateSection == -1 || channelGroup == -1) 	{
		DBG_871X("Wrong index value to access power limit table \
			  [band %d][regulation %d][bandwidth %d][rf_path %d][rate_section %d][chnlGroup %d]\n",
			  band, regulation, bandwidth, RfPath, rateSection, channelGroup);

		return 0xFF;
	}

	if (Band == BAND_ON_2_4G)
		powerLimit = rtlphy->txpwr_limit_2_4g[regulation]
			[bandwidth][rateSection][channelGroup][RfPath];
	else if (Band == BAND_ON_5G)
		powerLimit = rtlphy->txpwr_limit_5g[regulation]
			[bandwidth][rateSection][channelGroup][RfPath];
	else
		DBG_871X("No power limit table of the specified band\n");

#if 0
	/* combine 5G VHT & HT rate */
	/*  5G 20M and 40M HT and VHT can cross reference */
	/*if ( Band == BAND_ON_5G && powerLimit == MAX_POWER_INDEX ) {
		if ( bandwidth == 0 || bandwidth == 1 ) {
			if ( rateSection == 2 )
				powerLimit = pHalData->txpwr_limit_5g[regulation]
										[bandwidth][4][channelGroup][RfPath];
			else if ( rateSection == 4 )
				powerLimit = pHalData->txpwr_limit_5g[regulation]
										[bandwidth][2][channelGroup][RfPath];
			else if ( rateSection == 3 )
				powerLimit = pHalData->txpwr_limit_5g[regulation]
										[bandwidth][5][channelGroup][RfPath];
			else if ( rateSection == 5 )
				powerLimit = pHalData->txpwr_limit_5g[regulation]
										[bandwidth][3][channelGroup][RfPath];
		}
	}*/

	/*
	 * DBG_871X("TxPwrLmt[Regulation %d][Band %d][BW %d][RFPath %d][Rate 0x%x][Chnl %d] = %d\n",
	 * regulation, pHalData->CurrentBandType, Bandwidth, RfPath, DataRate, Channel, powerLimit);
	 */
#endif
	return powerLimit;
}

BOOLEAN phy_GetChnlIndex8812A(uint8_t Channel, uint8_t *ChannelIdx)
{
	uint8_t channel5G[CHANNEL_MAX_NUMBER_5G] = {
		36, 38, 40, 42, 44, 46, 48, 50, 52, 54, 56, 58, 60, 62, 64,
		100, 102, 104, 106, 108, 110, 112, 114, 116, 118, 120,
		122, 124, 126, 128, 130, 132, 134, 136, 138, 140, 142,
		144, 149, 151, 153, 155, 157, 159, 161, 163, 165, 167,
		168, 169, 171, 173, 175, 177
		};
	uint8_t	i = 0;
	BOOLEAN bIn24G = _TRUE;

	if (Channel <= 14) {
		bIn24G = _TRUE;
		*ChannelIdx = Channel - 1;
	} else {
		bIn24G = _FALSE;

		for (i = 0; i < sizeof(channel5G)/sizeof(uint8_t); ++i) {
			if (channel5G[i] == Channel) {
				*ChannelIdx = i;
				return bIn24G;
			}
		}
	}
	return bIn24G;

}

/*
 * For VHT series, we will use a new TX pwr by rate array to meet new spec.
 */

u32 phy_GetTxPwrByRateOffset_8812(struct rtl_priv *rtlpriv,  uint8_t Band,
	uint8_t	Rf_Path, uint8_t Rate_Section)
{
	struct rtl_phy *rtlphy = &(rtlpriv->phy);
	uint8_t			shift = 0, original_rate = Rate_Section;
	uint32_t			tx_pwr_diff = 0;

	/*
	 * For VHT series TX power by rate table.
	 * VHT TX power by rate off setArray =
	 * Band:-2G&5G = 0 / 1
	 * RF: at most 4*4 = ABCD=0/1/2/3
	 * CCK=0 				11/5.5/2/1
	 * OFDM=1/2 			18/12/9/6     54/48/36/24
	 * HT=3/4/5/6 			MCS0-3 MCS4-7 MCS8-11 MCS12-15
	 * VHT=7/8/9/10/11		1SSMCS0-3 1SSMCS4-7 2SSMCS1/0/1SSMCS/9/8 2SSMCS2-5
	 *
	 * #define		TX_PWR_BY_RATE_NUM_BAND			2
	 * #define		TX_PWR_BY_RATE_NUM_RF			4
	 * #define		TX_PWR_BY_RATE_NUM_SECTION		12
	 */

	switch (Rate_Section) {
	case MGN_1M:
	case MGN_2M:
	case MGN_5_5M:
	case MGN_11M:
		Rate_Section = 0;
		break;

	case MGN_6M:
	case MGN_9M:
	case MGN_12M:
	case MGN_18M:
		Rate_Section = 1;
		break;

	case MGN_24M:
	case MGN_36M:
	case MGN_48M:
	case MGN_54M:
		Rate_Section = 2;
		break;

	case MGN_MCS0:
	case MGN_MCS1:
	case MGN_MCS2:
	case MGN_MCS3:
		Rate_Section = 3;
		break;

	case MGN_MCS4:
	case MGN_MCS5:
	case MGN_MCS6:
	case MGN_MCS7:
		Rate_Section = 4;
		break;

	case MGN_MCS8:
	case MGN_MCS9:
	case MGN_MCS10:
	case MGN_MCS11:
		Rate_Section = 5;
		break;

	case MGN_MCS12:
	case MGN_MCS13:
	case MGN_MCS14:
	case MGN_MCS15:
		Rate_Section = 6;
		break;

	case MGN_VHT1SS_MCS0:
	case MGN_VHT1SS_MCS1:
	case MGN_VHT1SS_MCS2:
	case MGN_VHT1SS_MCS3:
		Rate_Section = 7;
		break;

	case MGN_VHT1SS_MCS4:
	case MGN_VHT1SS_MCS5:
	case MGN_VHT1SS_MCS6:
	case MGN_VHT1SS_MCS7:
		Rate_Section = 8;
		break;

	case MGN_VHT1SS_MCS8:
	case MGN_VHT1SS_MCS9:
	case MGN_VHT2SS_MCS0:
	case MGN_VHT2SS_MCS1:
		Rate_Section = 9;
		break;

	case MGN_VHT2SS_MCS2:
	case MGN_VHT2SS_MCS3:
	case MGN_VHT2SS_MCS4:
	case MGN_VHT2SS_MCS5:
		Rate_Section = 10;
		break;

	case MGN_VHT2SS_MCS6:
	case MGN_VHT2SS_MCS7:
	case MGN_VHT2SS_MCS8:
	case MGN_VHT2SS_MCS9:
		Rate_Section = 11;
		break;

	default:
		DBG_871X("Rate_Section is Illegal\n");
		break;
	}

	switch (original_rate) {
	case MGN_1M:
		shift = 0;
		break;
	case MGN_2M:
		shift = 8;
		break;
	case MGN_5_5M:
		shift = 16;
		break;
	case MGN_11M:
		shift = 24;
		break;
	case MGN_6M:
		shift = 0;
		break;
	case MGN_9M:
		shift = 8;
		break;
	case MGN_12M:
		shift = 16;
		break;
	case MGN_18M:
		shift = 24;
		break;
	case MGN_24M:
		shift = 0;
		break;
	case MGN_36M:
		shift = 8;
		break;
	case MGN_48M:
		shift = 16;
		break;
	case MGN_54M:
		shift = 24;
		break;

	case MGN_MCS0:
		shift = 0;
		break;
	case MGN_MCS1:
		shift = 8;
		break;
	case MGN_MCS2:
		shift = 16;
		break;
	case MGN_MCS3:
		shift = 24;
		break;

	case MGN_MCS4:
		shift = 0;
		break;
	case MGN_MCS5:
		shift = 8;
		break;
	case MGN_MCS6:
		shift = 16;
		break;
	case MGN_MCS7:
		shift = 24;
		break;

	case MGN_MCS8:
		shift = 0;
		break;
	case MGN_MCS9:
		shift = 8;
		break;
	case MGN_MCS10:
		shift = 16;
		break;
	case MGN_MCS11:
		shift = 24;
		break;

	case MGN_MCS12:
		shift = 0;
		break;
	case MGN_MCS13:
		shift = 8;
		break;
	case MGN_MCS14:
		shift = 16;
		break;
	case MGN_MCS15:
		shift = 24;
		break;

	case MGN_VHT1SS_MCS0:
		shift = 0;
		break;
	case MGN_VHT1SS_MCS1:
		shift = 8;
		break;
	case MGN_VHT1SS_MCS2:
		shift = 16;
		break;
	case MGN_VHT1SS_MCS3:
		shift = 24;
		break;

	case MGN_VHT1SS_MCS4:
		shift = 0;
		break;
	case MGN_VHT1SS_MCS5:
		shift = 8;
		break;
	case MGN_VHT1SS_MCS6:
		shift = 16;
		break;
	case MGN_VHT1SS_MCS7:
		shift = 24;
		break;

	case MGN_VHT1SS_MCS8:
		shift = 0;
		break;
	case MGN_VHT1SS_MCS9:
		shift = 8;
		break;
	case MGN_VHT2SS_MCS0:
		shift = 16;
		break;
	case MGN_VHT2SS_MCS1:
		shift = 24;
		break;

	case MGN_VHT2SS_MCS2:
		shift = 0;
		break;
	case MGN_VHT2SS_MCS3:
		shift = 8;
		break;
	case MGN_VHT2SS_MCS4:
		shift = 16;
		break;
	case MGN_VHT2SS_MCS5:
		shift = 24;
		break;

	case MGN_VHT2SS_MCS6:
		shift = 0;
		break;
	case MGN_VHT2SS_MCS7:
		shift = 8;
		break;
	case MGN_VHT2SS_MCS8:
		shift = 16;
		break;
	case MGN_VHT2SS_MCS9:
		shift = 24;
		break;

	default:
		DBG_871X("Rate_Section is Illegal\n");
		break;
	}

	/* Willis suggest to adopt 5G VHT power by rate for 2.4G */
	if (Band == BAND_ON_2_4G && (Rate_Section >= 7 && Rate_Section <= 11))
		Band = BAND_ON_5G;

	tx_pwr_diff = (rtlphy->TxPwrByRateOffset[Band][Rf_Path][Rate_Section] >> shift) & 0xff;

	/*
	 * DBG_871X("TxPwrByRateOffset-BAND(%d)-RF(%d)-RAS(%d)=%x tx_pwr_diff=%d shift=%d\n",
	 * Band, Rf_Path, Rate_Section, pHalData->TxPwrByRateOffset[Band][Rf_Path][Rate_Section], tx_pwr_diff, shift);
	 */

	return	tx_pwr_diff;

}

/*
 * Description:
 * 	Subtract number of TxPwr index from different advance settings.
 *
 * 	2010.03.09, added by Roger.
 */
void phy_TxPwrAdjInPercentage(struct rtl_priv *rtlpriv, uint8_t *pTxPwrIdx)
{
	 struct _rtw_hal	*pHalData = GET_HAL_DATA(rtlpriv);
	uint8_t	TxPwrInPercentage = 0;

	/* Retrieve default TxPwr index settings from registry. */
	TxPwrInPercentage = pHalData->TxPwrInPercentage;

	if (*pTxPwrIdx > RF6052_MAX_TX_PWR)
		*pTxPwrIdx = RF6052_MAX_TX_PWR;

	/*
	 * <Roger_Notes> NEC Spec: dB = 10*log(X/Y), X: target value, Y: default value.
	 * For example: TxPower 50%, 10*log(50/100)=(nearly)-3dB
	 * 2010.07.26.
	 */

	if (TxPwrInPercentage & TX_PWR_PERCENTAGE_0) {	/* 12.5% , -9dB */
		*pTxPwrIdx -= 18;
	} else if (TxPwrInPercentage & TX_PWR_PERCENTAGE_1) {	/* 25%, -6dB */
		*pTxPwrIdx -= 12;
	} else if (TxPwrInPercentage & TX_PWR_PERCENTAGE_2) {	/* 50%, -3dB */
		*pTxPwrIdx -= 6;
	}

	if (*pTxPwrIdx > RF6052_MAX_TX_PWR)	/* Avoid underflow condition. */
		*pTxPwrIdx = RF6052_MAX_TX_PWR;
}

u32 PHY_GetTxPowerIndex_8812A(struct rtl_priv *rtlpriv, uint8_t RFPath,
	uint8_t	Rate, enum CHANNEL_WIDTH BandWidth, uint8_t Channel)
{
	struct rtl_dm *rtldm = rtl_dm(rtlpriv);
	struct rtl_efuse *efuse = rtl_efuse(rtlpriv);
	struct rtl_hal *rtlhal = rtl_hal(rtlpriv);
	uint8_t					i = 0;	/* default set to 1S */
	struct registry_priv	*pregistrypriv = &rtlpriv->registrypriv;
	uint32_t					powerDiffByRate = 0;
	uint32_t					txPower = 0;
	uint8_t					chnlIdx = (Channel-1);
	BOOLEAN				bIn24G = _FALSE;

	/* DBG_871X("===> PHY_GetTxPowerIndex_8812A\n"); */

	if (HAL_IsLegalChannel(rtlpriv, Channel) == _FALSE) {
		chnlIdx = 0;
		DBG_871X("Illegal channel!!\n");
	}

	bIn24G = phy_GetChnlIndex8812A(Channel, &chnlIdx);

	/* DBG_871X("[%s] Channel Index: %d\n", (bIn24G?"2.4G":"5G"), chnlIdx); */

	if (bIn24G) { /* 3 ============================== 2.4 G ============================== */
		if (IS_CCK_RATE(Rate)) {
			txPower = efuse->txpwrlevel_cck[RFPath][chnlIdx];
		} else if (MGN_6M <= Rate) {
			txPower = efuse->txpwrlevel_ht40_1s[RFPath][chnlIdx];
		} else {
			DBG_871X("===> mpt_ProQueryCaltxPower_Jaguar: INVALID Rate.\n");
		}

		/* DBG_871X("Base Tx power(RF-%c, Rate #%d, Channel Index %d) = 0x%X\n", ((RFPath==0)?'A':'B'), Rate, chnlIdx, txPower); */

		/* OFDM-1T */
		if (MGN_6M <= Rate && Rate <= MGN_54M && !IS_CCK_RATE(Rate)) {
			txPower += efuse->txpwr_legacyhtdiff[RFPath][TX_1S];
			/* DBG_871X("+PowerDiff 2.4G (RF-%c): (OFDM-1T) = (%d)\n", ((RFPath==0)?'A':'B'), pHalData->OFDM_24G_Diff[RFPath][TX_1S]); */
		}

		if (BandWidth == CHANNEL_WIDTH_20) {	/* BW20-1S, BW20-2S */
			if ((MGN_MCS0 <= Rate && Rate <= MGN_MCS15) || (MGN_VHT2SS_MCS0 <= Rate && Rate <= MGN_VHT2SS_MCS9))
				txPower += efuse->txpwr_ht20diff[RFPath][TX_1S];
			if ((MGN_MCS8 <= Rate && Rate <= MGN_MCS15) || (MGN_VHT2SS_MCS0 <= Rate && Rate <= MGN_VHT2SS_MCS9))
				txPower += efuse->txpwr_ht20diff[RFPath][TX_2S];

			/*
			 * DBG_871X("+PowerDiff 2.4G (RF-%c): (BW20-1S, BW20-2S) = (%d, %d)\n", ((RFPath==0)?'A':'B'),
			 * 	pHalData->BW20_24G_Diff[RFPath][TX_1S], pHalData->BW20_24G_Diff[RFPath][TX_2S]);
			 */
		} else if (BandWidth == CHANNEL_WIDTH_40) {	/* BW40-1S, BW40-2S */
			if ((MGN_MCS0 <= Rate && Rate <= MGN_MCS15) || (MGN_VHT1SS_MCS0 <= Rate && Rate <= MGN_VHT2SS_MCS9))
				txPower += efuse->txpwr_ht40diff[RFPath][TX_1S];
			if ((MGN_MCS8 <= Rate && Rate <= MGN_MCS15) || (MGN_VHT2SS_MCS0 <= Rate && Rate <= MGN_VHT2SS_MCS9))
				txPower += efuse->txpwr_ht40diff[RFPath][TX_2S];

			/*
			 * DBG_871X("+PowerDiff 2.4G (RF-%c): (BW40-1S, BW40-2S) = (%d, %d)\n", ((RFPath==0)?'A':'B'),
			 *	pHalData->BW40_24G_Diff[RFPath][TX_1S], pHalData->BW40_24G_Diff[RFPath][TX_2S]);
			 */
		} else if (BandWidth == CHANNEL_WIDTH_80) {	/* Willis suggest adopt BW 40M power index while in BW 80 mode */
			if ((MGN_MCS0 <= Rate && Rate <= MGN_MCS15) || (MGN_VHT1SS_MCS0 <= Rate && Rate <= MGN_VHT2SS_MCS9))
				txPower += efuse->txpwr_ht40diff[RFPath][TX_1S];
			if ((MGN_MCS8 <= Rate && Rate <= MGN_MCS15) || (MGN_VHT2SS_MCS0 <= Rate && Rate <= MGN_VHT2SS_MCS9))
				txPower += efuse->txpwr_ht40diff[RFPath][TX_2S];

			/*
			 * DBG_871X("+PowerDiff 2.4G (RF-%c): (BW40-1S, BW40-2S) = (%d, %d) P.S. Current is in BW 80MHz\n", ((RFPath==0)?'A':'B'),
			 * 	pHalData->BW40_24G_Diff[RFPath][TX_1S], pHalData->BW40_24G_Diff[RFPath][TX_2S]);
			 */
		}

		/*
		 *  2012/09/26 MH Accordng to BB team's opinion, there might 40M VHT mode in the future.?
		 *  We need to judge VHT mode by what?
		 */
	} else {	/* 3 ============================== 5 G ============================== */
		if (MGN_6M <= Rate) {
			txPower = efuse->txpwr_5g_bw40base[RFPath][chnlIdx];
		} else {
			DBG_871X("===> mpt_ProQueryCalTxPower_Jaguar: INVALID Rate.\n");
		}

		/* DBG_871X("Base Tx power(RF-%c, Rate #%d, Channel Index %d) = 0x%X\n", ((RFPath==0)?'A':'B'), Rate, chnlIdx, txPower); */

		/* OFDM-1T */
		if (MGN_6M <= Rate && Rate <= MGN_54M && !IS_CCK_RATE(Rate)) {
			txPower += efuse->txpwr_5g_ofdmdiff[RFPath][TX_1S];
			/* DBG_871X("+PowerDiff 5G (RF-%c): (OFDM-1T) = (%d)\n", ((RFPath==0)?'A':'B'), pHalData->OFDM_5G_Diff[RFPath][TX_1S]); */
		}

		if (BandWidth == CHANNEL_WIDTH_20) {	/* BW20-1S, BW20-2S */
			if ((MGN_MCS0 <= Rate && Rate <= MGN_MCS15)  || (MGN_VHT1SS_MCS0 <= Rate && Rate <= MGN_VHT2SS_MCS9))
			    txPower += efuse->txpwr_5g_bw20diff[RFPath][TX_1S];
			if ((MGN_MCS8 <= Rate && Rate <= MGN_MCS15) || (MGN_VHT2SS_MCS0 <= Rate && Rate <= MGN_VHT2SS_MCS9))
			    txPower += efuse->txpwr_5g_bw20diff[RFPath][TX_2S];

			/*
			 * DBG_871X("+PowerDiff 5G (RF-%c): (BW20-1S, BW20-2S) = (%d, %d)\n", ((RFPath==0)?'A':'B'),
			 * 	pHalData->BW20_5G_Diff[RFPath][TX_1S], pHalData->BW20_5G_Diff[RFPath][TX_2S]);
			 */
		} else if (BandWidth == CHANNEL_WIDTH_40) {	/* BW40-1S, BW40-2S */
			if ((MGN_MCS0 <= Rate && Rate <= MGN_MCS15)  || (MGN_VHT1SS_MCS0 <= Rate && Rate <= MGN_VHT2SS_MCS9))
			    txPower += efuse->txpwr_5g_bw40diff[RFPath][TX_1S];
			if ((MGN_MCS8 <= Rate && Rate <= MGN_MCS15) || (MGN_VHT2SS_MCS0 <= Rate && Rate <= MGN_VHT2SS_MCS9))
			    txPower += efuse->txpwr_5g_bw40diff[RFPath][TX_2S];

			/*
			 * DBG_871X("+PowerDiff 5G(RF-%c): (BW40-1S, BW40-2S) = (%d, %d)\n", ((RFPath==0)?'A':'B'),
			 * 	pHalData->BW40_5G_Diff[RFPath][TX_1S], pHalData->BW40_5G_Diff[RFPath][TX_2S]);
			 */
		} else if (BandWidth == CHANNEL_WIDTH_80) {	/* BW80-1S, BW80-2S */
			/* <20121220, Kordan> Get the index of array "Index5G_BW80_Base". */
			uint8_t	channel5G_80M[CHANNEL_MAX_NUMBER_5G_80M] = {42, 58, 106, 122, 138, 155, 171};
			for (i = 0; i < sizeof(channel5G_80M)/sizeof(uint8_t); ++i)
				if (channel5G_80M[i] == Channel)
					chnlIdx = i;

			if ((MGN_MCS0 <= Rate && Rate <= MGN_MCS15)  || (MGN_VHT1SS_MCS0 <= Rate && Rate <= MGN_VHT2SS_MCS9))
				txPower = efuse->txpwr_5g_bw80diff[RFPath][chnlIdx] + efuse->txpwr_5g_bw80diff[RFPath][TX_1S];
			if ((MGN_MCS8 <= Rate && Rate <= MGN_MCS15) || (MGN_VHT2SS_MCS0 <= Rate && Rate <= MGN_VHT2SS_MCS9))
				txPower = efuse->txpwr_5g_bw80diff[RFPath][chnlIdx] + efuse->txpwr_5g_bw80diff[RFPath][TX_1S] + efuse->txpwr_5g_bw80diff[RFPath][TX_2S];

			/*
			 * DBG_871X("+PowerDiff 5G(RF-%c): (BW80-1S, BW80-2S) = (%d, %d)\n", ((RFPath==0)?'A':'B'),
			 * 	pHalData->BW80_5G_Diff[RFPath][TX_1S], pHalData->BW80_5G_Diff[RFPath][TX_2S]);
			 */
		}
	}

	/*
	 * Band:-2G&5G = 0 / 1
	 * Becasue in the functionwe use the bIn24G = 1=2.4G. Then we need to convert the value.
	 * RF: at most 4*4 = ABCD=0/1/2/3
	 * CCK=0 				11/5.5/2/1
	 * OFDM=1/2 			18/12/9/6     54/48/36/24
	 * HT=3/4/5/6 			MCS0-3 MCS4-7 MCS8-11 MCS12-15
	 * VHT=7/8/9/10/11		1SSMCS0-3 1SSMCS4-7 2SSMCS1/0/1SSMCS/9/8 2SSMCS2-5
	 */
	if (pregistrypriv->RegPwrByRate == _FALSE && efuse->eeprom_regulatory != 2) {
		powerDiffByRate = phy_GetTxPwrByRateOffset_8812(rtlpriv, (uint8_t)(!bIn24G), RFPath, Rate);

		if ((pregistrypriv->RegEnableTxPowerLimit == 1 && efuse->eeprom_regulatory != 2)
		||  efuse->eeprom_regulatory == 1) {
			uint8_t limit = 0;
			limit = _rtl8821au_phy_get_txpower_limit(rtlpriv, (uint8_t)(!bIn24G) ? BAND_ON_5G : BAND_ON_2_4G, BandWidth, (enum radio_path)RFPath, Rate, Channel);

			if (Rate == MGN_VHT1SS_MCS8 || Rate == MGN_VHT1SS_MCS9  ||
			    Rate == MGN_VHT2SS_MCS8 || Rate == MGN_VHT2SS_MCS9) {
				if (limit < 0) {
					if (powerDiffByRate < -limit)
						powerDiffByRate = -limit;
				}
			} else {
				if (limit < 0)
					powerDiffByRate = limit;
				else
					powerDiffByRate = powerDiffByRate > limit ? limit : powerDiffByRate;
			}
			/* DBG_871X("Maximum power by rate %d, final power by rate %d\n", limit, powerDiffByRate ); */
		}
	}

	/* DBG_871X("Rate-%x txPower=%x +PowerDiffByRate(RF-%c) = %d\n", Rate, txPower, ((RFPath==0)?'A':'B'), powerDiffByRate); */

	/*  We need to reduce power index for VHT MCS 8 & 9. */
	if (Rate == MGN_VHT1SS_MCS8 || Rate == MGN_VHT1SS_MCS9 ||
	    Rate == MGN_VHT2SS_MCS8 || Rate == MGN_VHT2SS_MCS9) {
		txPower -= powerDiffByRate;
	} else {
		/*
		 * 2013/01/29 MH For preventing VHT rate of 8812AU to be used in USB 2.0 mode
		 * and the current will be more than 500mA and card disappear. We need to limit
		 * TX power with any power by rate for VHT in U2.
		 * 2013/01/30 MH According to power current test compare with BCM AC NIC, we
		 * decide to use host hub = 2.0 mode to enable tx power limit behavior.
		 */
		if (rtl_usbdev(rtlpriv)->usb_speed <= RTW_USB_SPEED_2 && IS_HARDWARE_TYPE_8812AU(rtlhal)) {
			powerDiffByRate = 0;
		}

		txPower += powerDiffByRate;
	}
	/*
	 * DBG_871X("BASE ON HT MCS7\n");
	 * DBG_871X("Final Tx Power(RF-%c, Channel: %d) = %d(0x%X)\n", ((RFPath==0)?'A':'B'), chnlIdx+1, txPower, txPower);
	 */

	if (rtldm->modify_txagc_flag_path_a || rtldm->modify_txagc_flag_path_b) {
		/* 20130424 Mimic whether path A or B has to modify TxAGC */

		/* DBG_871X("Before add Remanant_OFDMSwingIdx[rfpath %u] %d", txPower); */
		txPower += rtldm->remnant_ofdm_swing_idx[RFPath];
		/* DBG_871X("After add Remanant_OFDMSwingIdx[rfpath %u] %d => txPower %d", RFPath, pDM_Odm->Remnant_OFDMSwingIdx[RFPath], txPower); */
	}

	if (txPower > MAX_POWER_INDEX)
		txPower = MAX_POWER_INDEX;

	/*
	 * 2012/09/26 MH We need to take care high power device limiation to prevent destroy EXT_PA.
	 * This case had ever happened in CU/SU high power module. THe limitation = 0x20.
	 * But for 8812, we still not know the value.
	 */
	phy_TxPwrAdjInPercentage(rtlpriv, (uint8_t *)&txPower);

	return txPower;
}

static void PHY_SetPowerLimitTableValue(struct rtl_priv *rtlpriv,
	s8 *Regulation, s8 *Band, s8 *Bandwidth, s8 *RateSection,
	s8 *RfPath, s8 *Channel, s8 *PowerLimit)
{
	struct rtl_phy *rtlphy = rtl_phy(rtlpriv);
	uint8_t		regulation = 0, bandwidth = 0, rateSection = 0,
			channel, powerLimit, channelGroup;

	DBG_871X("Index of power limit table \
		  [band %s][regulation %s][bw %s][rate section %s][rf path %s][chnl %s][val %s]\n",
		  Band, Regulation, Bandwidth, RateSection, RfPath, Channel, PowerLimit);

	if (!Getu8IntegerFromStringInDecimal(Channel, &channel) ||
		 !Getu8IntegerFromStringInDecimal(PowerLimit, &powerLimit)) {
		DBG_871X("Illegal index of power limit table [chnl %s][val %s]\n", Channel, PowerLimit);
	}

	powerLimit = powerLimit > MAX_POWER_INDEX ? MAX_POWER_INDEX : powerLimit;

	if (eqNByte(Regulation, "FCC", 3))
		regulation = 0;
	else if (eqNByte(Regulation, "MKK", 3))
		regulation = 1;
	else if (eqNByte(Regulation, "ETSI", 4))
		regulation = 2;

	if (eqNByte(RateSection, "CCK", 3))
		rateSection = 0;
	else if (eqNByte(RateSection, "OFDM", 4))
		rateSection = 1;
	else if (eqNByte(RateSection, "HT", 2) && eqNByte(RfPath, "1T", 2))
		rateSection = 2;
	else if (eqNByte(RateSection, "HT", 2) && eqNByte(RfPath, "2T", 2))
		rateSection = 3;
	else if (eqNByte(RateSection, "VHT", 3) && eqNByte(RfPath, "1T", 2))
		rateSection = 4;
	else if (eqNByte(RateSection, "VHT", 3) && eqNByte(RfPath, "2T", 2))
		rateSection = 5;


	if (eqNByte(Bandwidth, "20M", 3))
		bandwidth = 0;
	else if (eqNByte(Bandwidth, "40M", 3))
		bandwidth = 1;
	else if (eqNByte(Bandwidth, "80M", 3))
		bandwidth = 2;
	else if (eqNByte(Bandwidth, "160M", 4))
		bandwidth = 3;

	if (eqNByte(Band, "2.4G", 4)) {
		DBG_871X("2.4G Band value : [regulation %d][bw %d][rate_section %d][chnl %d][val %d]\n",
			regulation, bandwidth, rateSection, channel, powerLimit);
		channelGroup = phy_GetChannelGroup(BAND_ON_2_4G, channel);
		rtlphy->txpwr_limit_2_4g[regulation][bandwidth][rateSection][channelGroup][RF90_PATH_A] = powerLimit;
	} else if (eqNByte(Band, "5G", 2)) {
		DBG_871X("5G Band value : [regulation %d][bw %d][rate_section %d][chnl %d][val %d]\n",
			  regulation, bandwidth, rateSection, channel, powerLimit);
		channelGroup = phy_GetChannelGroup(BAND_ON_5G, channel);
		rtlphy->txpwr_limit_5g[regulation][bandwidth][rateSection][channelGroup][RF90_PATH_A] = powerLimit;
	} else {
		DBG_871X("Cannot recognize the band info in %s\n", Band);
		return;
	}
}


static void odm_ConfigBB_TXPWR_LMT_8812A(struct rtl_priv *rtlpriv,
	u8 *Regulation, u8 *Band, u8 *Bandwidth,
	u8 *RateSection, u8 *RfPath, u8 *Channel,
	u8 *PowerLimit)
{
	PHY_SetPowerLimitTableValue(rtlpriv, Regulation, Band,
		Bandwidth, RateSection, RfPath, Channel, PowerLimit);
}


static void ODM_ReadAndConfig_MP_8812A_TXPWR_LMT(struct rtl_priv *rtlpriv)
{
	uint32_t i		= 0;
	uint32_t ArrayLen       = RTL8812AU_TXPWR_LMT_ARRAY_LEN;
	u8 **Array		= RTL8812AU_TXPWR_LMT;

#if 0
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_INIT, ODM_DBG_TRACE, ("===> ODM_ReadAndConfig_MP_8812A_TXPWR_LMT\n"));
#endif
	for (i = 0; i < ArrayLen; i += 7) {
		u8 *regulation = Array[i];
		u8 *band = Array[i+1];
		u8 *bandwidth = Array[i+2];
		u8 *rate = Array[i+3];
		u8 *rfPath = Array[i+4];
		u8 *chnl = Array[i+5];
		u8 *val = Array[i+6];

		odm_ConfigBB_TXPWR_LMT_8812A(rtlpriv, regulation, band, bandwidth, rate, rfPath, chnl, val);
	}

}


static void odm_ConfigBB_TXPWR_LMT_8821A(struct rtl_priv *rtlpriv,
	u8 *Regulation, u8 *Band, u8 *Bandwidth,
	u8 * RateSection, u8 *RfPath, u8 *Channel,
	u8 *PowerLimit
    )
{
	PHY_SetPowerLimitTableValue(rtlpriv, Regulation, Band,
		Bandwidth, RateSection, RfPath, Channel, PowerLimit);
}


static void ODM_ReadAndConfig_MP_8821A_TXPWR_LMT(struct rtl_priv *rtlpriv)
{
	uint32_t i		= 0;
	uint32_t ArrayLen       = RTL8821AU_TXPWR_LMT_ARRAY_LEN;
	u8 **Array		= RTL8821AU_TXPWR_LMT;

#if 0
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_INIT, ODM_DBG_TRACE, ("===> ODM_ReadAndConfig_MP_8821A_TXPWR_LMT\n"));
#endif
	for (i = 0; i < ArrayLen; i += 7) {
		u8 *regulation = Array[i];
		u8 *band = Array[i+1];
		u8 *bandwidth = Array[i+2];
		u8 *rate = Array[i+3];
		u8 *rfPath = Array[i+4];
		u8 *chnl = Array[i+5];
		u8 *val = Array[i+6];

		odm_ConfigBB_TXPWR_LMT_8821A(rtlpriv, regulation, band, bandwidth, rate, rfPath, chnl, val);
	}

}

void _rtl8821au_phy_read_and_config_txpwr_lmt(struct rtl_priv *rtlpriv)
{
	struct rtl_hal	*rtlhal = rtl_hal(rtlpriv);

	if (IS_HARDWARE_TYPE_8812AU(rtlhal))
		ODM_ReadAndConfig_MP_8812A_TXPWR_LMT(rtlpriv);
	if (IS_HARDWARE_TYPE_8821U(rtlhal))
		ODM_ReadAndConfig_MP_8821A_TXPWR_LMT(rtlpriv);
}



static BOOLEAN CheckCondition(const uint32_t  Condition, const uint32_t  Hex)
{
	uint32_t _board     = (Hex & 0x000000FF);
	uint32_t _interface = (Hex & 0x0000FF00) >> 8;
	uint32_t _platform  = (Hex & 0x00FF0000) >> 16;
	uint32_t cond = Condition;

	if (Condition == 0xCDCDCDCD)
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

static void ODM_ReadAndConfig_MP_8812A_MAC_REG(struct rtl_priv *rtlpriv)
{
	struct rtl_hal	*rtlhal = rtl_hal(rtlpriv);
	
	uint32_t     hex         = 0;
	uint32_t     i           = 0;
	u16     count       = 0;
	uint32_t    *ptr_array   = NULL;
	
	/* ULLI : fixed values ?? */
	u8  platform = ODM_CE;
	u8 _interface = RTW_USB;
	u8 board = rtlhal->board_type;

	uint32_t     ArrayLen    = RTL8812AUMAC_1T_ARRAYLEN;
	uint32_t    *Array       = RTL8812AU_MAC_REG_ARRAY;


	hex += board;
	hex += _interface << 8;
	hex += platform << 16;
	hex += 0xFF000000;
#if 0
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_INIT, ODM_DBG_TRACE, ("===> ODM_ReadAndConfig_MP_8812A_MAC_REG, hex = 0x%X\n", hex));
#endif
	for (i = 0; i < ArrayLen; i += 2) {
		uint32_t v1 = Array[i];
		uint32_t v2 = Array[i+1];

		/* This (offset, data) pair meets the condition. */
		if (v1 < 0xCDCDCDCD) {
			rtl_write_byte(rtlpriv, v1, (u8)v2);
			continue;
		} else {
			/* This line is the start line of branch. */
			if (!CheckCondition(Array[i], hex)) {
				/* Discard the following (offset, data) pairs. */
				READ_NEXT_PAIR(Array, v1, v2, i);
				while (v2 != 0xDEAD &&
					v2 != 0xCDEF &&
					v2 != 0xCDCD && i < ArrayLen - 2) {
						READ_NEXT_PAIR(Array, v1, v2, i);
				}
				i -= 2; /* prevent from for-loop += 2 */
			} else { /* Configure matched pairs and skip to end of if-else. */
				READ_NEXT_PAIR(Array, v1, v2, i);
				while (v2 != 0xDEAD &&
					v2 != 0xCDEF &&
					v2 != 0xCDCD && i < ArrayLen - 2) {
						rtl_write_byte(rtlpriv, v1, (u8)v2);
						READ_NEXT_PAIR(Array, v1, v2, i);
				}

				while (v2 != 0xDEAD && i < ArrayLen - 2) {
					READ_NEXT_PAIR(Array, v1, v2, i);
				}
			}
		}
	}

}

static void ODM_ReadAndConfig_MP_8821A_MAC_REG(struct rtl_priv *rtlpriv)
{
	struct rtl_hal	*rtlhal = rtl_hal(rtlpriv);

	uint32_t     hex         = 0;
	uint32_t     i           = 0;
	u16     count       = 0;
	uint32_t    *ptr_array   = NULL;
	
	/* ULLI : fixed values ?? */
	u8  platform = ODM_CE;
	u8 _interface = RTW_USB;
	u8 board = rtlhal->board_type;
	
	uint32_t     ArrayLen    = RTL8821AUMAC_1T_ARRAYLEN;
	uint32_t    *Array       = RTL8821AU_MAC_REG_ARRAY;

	hex += board;
	hex += _interface << 8;
	hex += platform << 16;
	hex += 0xFF000000;
#if 0
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_INIT, ODM_DBG_TRACE, ("===> ODM_ReadAndConfig_MP_8821A_MAC_REG, hex = 0x%X\n", hex));
#endif
	for (i = 0; i < ArrayLen; i += 2) {
		uint32_t v1 = Array[i];
		uint32_t v2 = Array[i+1];

		/* This (offset, data) pair meets the condition. */
		if (v1 < 0xCDCDCDC) {
			rtl_write_byte(rtlpriv, v1, (u8)v2);
			continue;
		} else {
			/* This line is the start line of branch. */
			if (!CheckCondition(Array[i], hex)) {
				/* Discard the following (offset, data) pairs. */
				READ_NEXT_PAIR(Array, v1, v2, i);
				while (v2 != 0xDEAD &&
					v2 != 0xCDEF &&
					v2 != 0xCDCD && i < ArrayLen - 2) {
						READ_NEXT_PAIR(Array, v1, v2, i);
				}
				i -= 2; /* prevent from for-loop += 2 */
			} else {
				/* Configure matched pairs and skip to end of if-else. */
				READ_NEXT_PAIR(Array, v1, v2, i);
				while (v2 != 0xDEAD &&
					v2 != 0xCDEF &&
					v2 != 0xCDCD && i < ArrayLen - 2) {
						rtl_write_byte(rtlpriv, v1, (u8)v2);
						READ_NEXT_PAIR(Array, v1, v2, i);
				}

				while (v2 != 0xDEAD && i < ArrayLen - 2) {
					READ_NEXT_PAIR(Array, v1, v2, i);
				}

			}
		}
	}

}


void _rtl8821au_phy_config_mac_with_headerfile(struct rtl_priv *rtlpriv)
{
	struct rtl_hal	*rtlhal = rtl_hal(rtlpriv);

#if 0
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_INIT, ODM_DBG_LOUD,
		("===>ODM_ConfigMACWithHeaderFile (%s)\n", (pDM_Odm->bIsMPChip) ? "MPChip" : "TestChip"));
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_INIT, ODM_DBG_LOUD,
		("pDM_Odm->SupportInterface: 0x%X, pDM_Odm->BoardType: 0x%X\n",
		rtlhal->SupportInterface, rtlhal->board_type));
#endif
	if (IS_HARDWARE_TYPE_8812AU(rtlhal))
		ODM_ReadAndConfig_MP_8812A_MAC_REG(rtlpriv);
	
	if (IS_HARDWARE_TYPE_8821U(rtlhal))
		ODM_ReadAndConfig_MP_8821A_MAC_REG(rtlpriv);
}


/* *****  */

/******************************************************************************
*                           RadioA.TXT
******************************************************************************/
static void odm_ConfigRFReg_8812A(struct rtl_priv *rtlpriv, uint32_t Addr,
	uint32_t Data, enum radio_path path, uint32_t RegAddr)
{
	if (Addr == 0xfe || Addr == 0xffe) {
		msleep(50);
	} else {
		rtw_hal_write_rfreg(rtlpriv, path, RegAddr, bRFRegOffsetMask, Data);
		/* Add 1us delay between BB/RF register setting. */
		udelay(1);
	}
}

static void _rtl8812au_config_rf_radio_a(struct rtl_priv *rtlpriv, uint32_t Addr,
	uint32_t Data)
{
	uint32_t content = 0x1000;		/* RF_Content: radioa_txt */
	uint32_t maskforPhySet = (uint32_t)(content&0xE000);

	odm_ConfigRFReg_8812A(rtlpriv, Addr, Data, RF90_PATH_A, Addr|maskforPhySet);

	/* ODM_RT_TRACE(pDM_Odm, ODM_COMP_INIT, ODM_DBG_TRACE, ("===> ODM_ConfigRFWithHeaderFile: [RadioA] %08X %08X\n", Addr, Data)); */
}

static void _rtl8812au_config_rf_radio_b(struct rtl_priv *rtlpriv, uint32_t Addr,
	uint32_t Data)
{
	uint32_t  content = 0x1001;		/* RF_Content: radiob_txt */
	uint32_t maskforPhySet = (uint32_t)(content&0xE000);

	odm_ConfigRFReg_8812A(rtlpriv, Addr, Data, RF90_PATH_B, Addr|maskforPhySet);

	/* ODM_RT_TRACE(pDM_Odm, ODM_COMP_INIT, ODM_DBG_TRACE, ("===> ODM_ConfigRFWithHeaderFile: [RadioB] %08X %08X\n", Addr, Data)); */
}



void rtl8812au_phy_config_rf_with_headerfile(struct rtl_priv *rtlpriv,
				enum radio_path eRFPath)
{
	struct rtl_hal	*rtlhal = rtl_hal(rtlpriv);
	
//	struct _rtw_hal  *pHalData = GET_HAL_DATA(rtlpriv);
//	struct _rtw_dm * pDM_Odm = &pHalData->odmpriv;
	
	int i;
	bool rtstatus = true;
	u32 *radioa_array_table_a, *radioa_array_table_b;
	u16 radioa_arraylen_a, radioa_arraylen_b;
	u32 v1 = 0, v2 = 0;

	uint32_t	hex         = 0;
	u16	count       = 0;
	uint32_t	*ptr_array   = NULL;

	/* ULLI : fixed values ?? */
	u8  platform = ODM_CE;
	u8 _interface = RTW_USB;
	u8 board = rtlhal->board_type;
	
	radioa_arraylen_a = RTL8812AU_RADIOA_1TARRAYLEN;
	radioa_array_table_a = RTL8812AU_RADIOA_ARRAY;
	radioa_arraylen_b = RTL8812AU_RADIOB_1TARRAYLEN;
	radioa_array_table_b = RTL8812AU_RADIOB_ARRAY;
	/*

	RT_TRACE(rtlpriv, COMP_INIT, DBG_LOUD,
		 "Radio_A:RTL8821AE_RADIOA_ARRAY %d\n", radioa_arraylen_a);
	RT_TRACE(rtlpriv, COMP_INIT, DBG_LOUD, "Radio No %x\n", rfpath);
	*/
	rtstatus = true;
	switch (eRFPath) {
	case RF90_PATH_A:
		hex += board;
		hex += _interface << 8;
		hex += platform << 16;
		hex += 0xFF000000;
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_INIT, ODM_DBG_TRACE, ("===> ODM_ReadAndConfig_MP_8821A_RadioA, hex = 0x%X\n", hex));

		for (i = 0; i < radioa_arraylen_a; i += 2) {
			uint32_t v1 = radioa_array_table_a[i];
			uint32_t v2 = radioa_array_table_a[i+1];

			/* This (offset, data) pair meets the condition. */
			if (v1 < 0xCDCDCDCD) {
				_rtl8812au_config_rf_radio_a(rtlpriv, v1, v2);
				continue;
			} else {
				/* This line is the start line of branch. */
				if (!CheckCondition(radioa_array_table_a[i], hex)) {
					/* Discard the following (offset, data) pairs. */
					READ_NEXT_PAIR(radioa_array_table_a, v1, v2, i);
					while (v2 != 0xDEAD &&
					    v2 != 0xCDEF &&
					    v2 != 0xCDCD && i < radioa_arraylen_a-2) {
						READ_NEXT_PAIR(radioa_array_table_a, v1, v2, i);
					}
					i -= 2; /* prevent from for-loop += 2 */
				} else {
					/* Configure matched pairs and skip to end of if-else. */
					READ_NEXT_PAIR(radioa_array_table_a, v1, v2, i);
					while (v2 != 0xDEAD &&
					    v2 != 0xCDEF &&
					    v2 != 0xCDCD && i < radioa_arraylen_a-2) {
						_rtl8812au_config_rf_radio_a(rtlpriv, v1, v2);
						READ_NEXT_PAIR(radioa_array_table_a, v1, v2, i);
					}

					while (v2 != 0xDEAD && i < radioa_arraylen_a-2) {
						READ_NEXT_PAIR(radioa_array_table_a, v1, v2, i);
					}
				}
			}
		}
		break;
	case RF90_PATH_B:
		hex += board;
		hex += _interface << 8;
		hex += platform << 16;
		hex += 0xFF000000;
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_INIT, ODM_DBG_TRACE, ("===> ODM_ReadAndConfig_MP_8812A_RadioB, hex = 0x%X\n", hex));

		for (i = 0; i < radioa_arraylen_b; i += 2) {
			uint32_t v1 = radioa_array_table_b[i];
			uint32_t v2 = radioa_array_table_b[i+1];

			/* This (offset, data) pair meets the condition. */
			if (v1 < 0xCDCDCDCD) {
				_rtl8812au_config_rf_radio_b(rtlpriv, v1, v2);
				continue;
			} else {
				/* This line is the start line of branch. */
				if (!CheckCondition(radioa_array_table_b[i], hex)) {
					/* Discard the following (offset, data) pairs. */
					READ_NEXT_PAIR(radioa_array_table_b, v1, v2, i);
					while (v2 != 0xDEAD &&
					    v2 != 0xCDEF &&
					    v2 != 0xCDCD && i < radioa_arraylen_b-2) {
						READ_NEXT_PAIR(radioa_array_table_b, v1, v2, i);
					}
					i -= 2; /* prevent from for-loop += 2 */
				} else {
					/* Configure matched pairs and skip to end of if-else. */
					READ_NEXT_PAIR(radioa_array_table_b, v1, v2, i);
					while (v2 != 0xDEAD &&
					    v2 != 0xCDEF &&
					    v2 != 0xCDCD && i < radioa_arraylen_b-2) {
						_rtl8812au_config_rf_radio_b(rtlpriv, v1, v2);
						READ_NEXT_PAIR(radioa_array_table_b, v1, v2, i);
					}

					while (v2 != 0xDEAD && i < radioa_arraylen_b-2) {
						READ_NEXT_PAIR(radioa_array_table_b, v1, v2, i);
					}
				}
			}
		}

	case RF90_PATH_C:
		/*
		 * RT_TRACE(rtlpriv, COMP_ERR, DBG_EMERG,
		 * 	 "switch case not process\n");
		 */
		break;
	case RF90_PATH_D:
		/*
		 * RT_TRACE(rtlpriv, COMP_ERR, DBG_EMERG,
		 * 	 "switch case not process\n");
		 */
		break;
	}
}


/******************************************************************************
*                           RadioA.TXT
******************************************************************************/
static void odm_ConfigRFReg_8821A(struct rtl_priv *rtlpriv, uint32_t Addr,
	uint32_t Data, enum radio_path path, uint32_t RegAddr)
{
	if (Addr == 0xfe || Addr == 0xffe) {
		msleep(50);
	} else if (Addr == 0xfd) {
		mdelay(5);
	} else if (Addr == 0xfc) {
		mdelay(1);
	} else if (Addr == 0xfb) {
		udelay(50);
	} else if (Addr == 0xfa) {
		udelay(5);
	} else if (Addr == 0xf9) {
		udelay(1);
	} else {
		rtw_hal_write_rfreg(rtlpriv, path, RegAddr, bRFRegOffsetMask, Data);
		/* Add 1us delay between BB/RF register setting. */
		udelay(1);
	}
}



static void _rtl8821au_config_rf_radio_a(struct rtl_priv *rtlpriv, uint32_t Addr,
	uint32_t Data)
{
	uint32_t  content = 0x1000;		/* RF_Content: radioa_txt */
	uint32_t maskforPhySet = (uint32_t)(content&0xE000);

	odm_ConfigRFReg_8821A(rtlpriv, Addr, Data, RF90_PATH_A, Addr|maskforPhySet);

	/* ODM_RT_TRACE(pDM_Odm, ODM_COMP_INIT, ODM_DBG_TRACE, ("===> ODM_ConfigRFWithHeaderFile: [RadioA] %08X %08X\n", Addr, Data)); */
}


void rtl8821au_phy_config_rf_with_headerfile(struct rtl_priv *rtlpriv, enum radio_path eRFPath)
{
	struct rtl_hal	*rtlhal = rtl_hal(rtlpriv);

	uint32_t	hex         = 0;
	uint32_t	i           = 0;
	u16	count       = 0;
	uint32_t	*ptr_array   = NULL;
	
	/* ULLI : fixed values ?? */
	u8  platform = ODM_CE;
	u8 _interface = RTW_USB;
	u8 board = rtlhal->board_type;

	uint32_t	ArrayLen    =  RTL8821AU_RADIOA_1TARRAYLEN;
	uint32_t	*Array       = RTL8821AU_RADIOA_ARRAY;


	hex += board;
	hex += _interface << 8;
	hex += platform << 16;
	hex += 0xFF000000;
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_INIT, ODM_DBG_TRACE, ("===> ODM_ReadAndConfig_MP_8821A_RadioA, hex = 0x%X\n", hex));

	for (i = 0; i < ArrayLen; i += 2) {
		uint32_t v1 = Array[i];
		uint32_t v2 = Array[i+1];

		/* This (offset, data) pair meets the condition. */
		if (v1 < 0xCDCDCDCD) {
			_rtl8821au_config_rf_radio_a(rtlpriv, v1, v2);
			continue;
		} else {
			/* This line is the start line of branch. */
			if (!CheckCondition(Array[i], hex)) {
				/* Discard the following (offset, data) pairs. */
				READ_NEXT_PAIR(Array, v1, v2, i);
				while (v2 != 0xDEAD &&
				    v2 != 0xCDEF &&
				    v2 != 0xCDCD && i < ArrayLen-2) {
					READ_NEXT_PAIR(Array, v1, v2, i);
				}
				i -= 2; /* prevent from for-loop += 2 */
			} else {
				/* Configure matched pairs and skip to end of if-else. */
				READ_NEXT_PAIR(Array, v1, v2, i);
				while (v2 != 0xDEAD &&
				    v2 != 0xCDEF &&
				    v2 != 0xCDCD && i < ArrayLen-2) {
					_rtl8821au_config_rf_radio_a(rtlpriv, v1, v2);
					READ_NEXT_PAIR(Array, v1, v2, i);
				}

				while (v2 != 0xDEAD && i < ArrayLen-2) {
					READ_NEXT_PAIR(Array, v1, v2, i);
				}
			}
		}
	}

}

static void _rtl8821au_phy_set_reg_bw(struct rtl_priv *rtlpriv, enum CHANNEL_WIDTH bw)
{
	u16 reg_rf_mode_bw, tmp = 0;

	reg_rf_mode_bw = rtl_read_word(rtlpriv, REG_WMAC_TRXPTCL_CTL);
	switch (bw) {
	case CHANNEL_WIDTH_20:
		rtl_write_word(rtlpriv, REG_WMAC_TRXPTCL_CTL, reg_rf_mode_bw & 0xFE7F);
		break;
	case CHANNEL_WIDTH_40:
		tmp = reg_rf_mode_bw | BIT(7);
		rtl_write_word(rtlpriv, REG_WMAC_TRXPTCL_CTL, tmp & 0xFEFF);
		break;
	case CHANNEL_WIDTH_80:
		tmp = reg_rf_mode_bw | BIT(8);
		rtl_write_word(rtlpriv, REG_WMAC_TRXPTCL_CTL, tmp & 0xFF7F);
		break;
	default:
		DBG_871X("phy_PostSetBWMode8812():	unknown Bandwidth: %#X\n", bw);
		break;
	}
}

void rtl8812au_fixspur(struct rtl_priv *rtlpriv, enum CHANNEL_WIDTH Bandwidth,
	u8 Channel)
{
	struct rtl_hal *rtlhal = rtl_hal(rtlpriv);
	
	/* C cut Item12 ADC FIFO CLOCK */
	if(IS_VENDOR_8812A_C_CUT(rtlpriv)) {
		if(Bandwidth == CHANNEL_WIDTH_40 && Channel == 11)
			rtl_set_bbreg(rtlpriv, rRFMOD_Jaguar, 0xC00, 0x3);		/* 0x8AC[11:10] = 2'b11 */
		else
			rtl_set_bbreg(rtlpriv, rRFMOD_Jaguar, 0xC00, 0x2);		/* 0x8AC[11:10] = 2'b10 */

		/*
		 *  <20120914, Kordan> A workarould to resolve 2480Mhz spur by setting ADC clock as 160M. (Asked by Binson) 
		 */
		if (Bandwidth == CHANNEL_WIDTH_20 && (Channel == 13 || Channel == 14)) {
			rtl_set_bbreg(rtlpriv, rRFMOD_Jaguar, 0x300, 0x3);  		/* 0x8AC[9:8] = 2'b11 */
			rtl_set_bbreg(rtlpriv, rADC_Buf_Clk_Jaguar, BIT30, 1);  	/* 0x8C4[30] = 1 */
		} else if (Bandwidth == CHANNEL_WIDTH_40 && Channel == 11) {
			rtl_set_bbreg(rtlpriv, rADC_Buf_Clk_Jaguar, BIT30, 1);  	/* 0x8C4[30] = 1 */
		} else if (Bandwidth != CHANNEL_WIDTH_80) {
			rtl_set_bbreg(rtlpriv, rRFMOD_Jaguar, 0x300, 0x2);  		/* 0x8AC[9:8] = 2'b10 */
			rtl_set_bbreg(rtlpriv, rADC_Buf_Clk_Jaguar, BIT30, 0);  	/* 0x8C4[30] = 0 */

		}
	} else if (IS_HARDWARE_TYPE_8812(rtlhal)) {
		/* <20120914, Kordan> A workarould to resolve 2480Mhz spur by setting ADC clock as 160M. (Asked by Binson) */
		if (Bandwidth == CHANNEL_WIDTH_20 && (Channel == 13 || Channel == 14))
			rtl_set_bbreg(rtlpriv, rRFMOD_Jaguar, 0x300, 0x3);  /* 0x8AC[9:8] = 11 */
		else if (Channel <= 14) /* 2.4G only */
			rtl_set_bbreg(rtlpriv, rRFMOD_Jaguar, 0x300, 0x2);  /* 0x8AC[9:8] = 10 */
	}

}
static u8 _rtl8821au_phy_get_secondary_chnl(struct rtl_priv *rtlpriv)
{
	struct rtl_mac *mac = &(rtlpriv->mac80211);
	
	uint8_t					SCSettingOf40 = 0, SCSettingOf20 = 0;
	struct _rtw_hal *	pHalData = GET_HAL_DATA(rtlpriv);

	/*
	 * DBG_871X("SCMapping: VHT Case: pHalData->CurrentChannelBW %d, pHalData->nCur80MhzPrimeSC %d, pHalData->nCur40MhzPrimeSC %d \n",pHalData->CurrentChannelBW,pHalData->nCur80MhzPrimeSC,pHalData->nCur40MhzPrimeSC);
	 */
	if(rtlpriv->phy.current_chan_bw== CHANNEL_WIDTH_80) {
		if(mac->cur_80_prime_sc == HAL_PRIME_CHNL_OFFSET_LOWER)
			SCSettingOf40 = VHT_DATA_SC_40_LOWER_OF_80MHZ;
		else if(mac->cur_80_prime_sc == HAL_PRIME_CHNL_OFFSET_UPPER)
			SCSettingOf40 = VHT_DATA_SC_40_UPPER_OF_80MHZ;
		else
			DBG_871X("SCMapping: Not Correct Primary40MHz Setting \n");

		if((mac->cur_40_prime_sc == HAL_PRIME_CHNL_OFFSET_LOWER) && (mac->cur_80_prime_sc == HAL_PRIME_CHNL_OFFSET_LOWER))
			SCSettingOf20 = VHT_DATA_SC_20_LOWEST_OF_80MHZ;
		else if((mac->cur_40_prime_sc == HAL_PRIME_CHNL_OFFSET_UPPER) && (mac->cur_80_prime_sc == HAL_PRIME_CHNL_OFFSET_LOWER))
			SCSettingOf20 = VHT_DATA_SC_20_LOWER_OF_80MHZ;
		else if((mac->cur_40_prime_sc == HAL_PRIME_CHNL_OFFSET_LOWER) && (mac->cur_80_prime_sc == HAL_PRIME_CHNL_OFFSET_UPPER))
			SCSettingOf20 = VHT_DATA_SC_20_UPPER_OF_80MHZ;
		else if((mac->cur_40_prime_sc == HAL_PRIME_CHNL_OFFSET_UPPER) && (mac->cur_80_prime_sc == HAL_PRIME_CHNL_OFFSET_UPPER))
			SCSettingOf20 = VHT_DATA_SC_20_UPPERST_OF_80MHZ;
		else
			DBG_871X("SCMapping: Not Correct Primary40MHz Setting \n");
	} else if(rtlpriv->phy.current_chan_bw == CHANNEL_WIDTH_40) {
		/*
		 * DBG_871X("SCMapping: VHT Case: pHalData->CurrentChannelBW %d, pHalData->nCur40MhzPrimeSC %d \n",pHalData->CurrentChannelBW,pHalData->nCur40MhzPrimeSC);
		 */

		if(mac->cur_40_prime_sc == HAL_PRIME_CHNL_OFFSET_UPPER)
			SCSettingOf20 = VHT_DATA_SC_20_UPPER_OF_80MHZ;
		else if(mac->cur_40_prime_sc == HAL_PRIME_CHNL_OFFSET_LOWER)
			SCSettingOf20 = VHT_DATA_SC_20_LOWER_OF_80MHZ;
		else
			DBG_871X("SCMapping: Not Correct Primary40MHz Setting \n");
	}

	/*
	 * DBG_871X("SCMapping: SC Value %x \n", ( (SCSettingOf40 << 4) | SCSettingOf20));
	 */
	return  ( (SCSettingOf40 << 4) | SCSettingOf20);
}


void rtl8821au_phy_set_bw_mode_callback(struct rtl_priv *rtlpriv)
{
	uint8_t			SubChnlNum = 0;
	uint8_t			L1pkVal = 0;
	 struct _rtw_hal	*pHalData = GET_HAL_DATA(rtlpriv);

	/* 3 Set Reg668 Reg440 BW */
	_rtl8821au_phy_set_reg_bw(rtlpriv, rtlpriv->phy.current_chan_bw);

	/* 3 Set Reg483 */
	SubChnlNum = _rtl8821au_phy_get_secondary_chnl(rtlpriv);
	rtl_write_byte(rtlpriv, REG_DATA_SC_8812, SubChnlNum);

	/* DBG_871X("[BW:CHNL], phy_PostSetBwMode8812(), set BW=%s !!\n", GLBwSrc[pHalData->CurrentChannelBW]); */

	/* 3 Set Reg848 Reg864 Reg8AC Reg8C4 RegA00 */
	switch (rtlpriv->phy.current_chan_bw) {
	case CHANNEL_WIDTH_20:
		rtl_set_bbreg(rtlpriv, rRFMOD_Jaguar, 0x003003C3, 0x00300200); /* 0x8ac[21,20,9:6,1,0]=8'b11100000 */
		rtl_set_bbreg(rtlpriv, rADC_Buf_Clk_Jaguar, BIT30, 0);			// 0x8c4[30] = 1'b0

		rtl_set_bbreg(rtlpriv, rFPGA0_XB_RFInterfaceOE, 0x001C0000, 4);	/* 0x864[20:18] = 3'b4 */

		if(rtlpriv->phy.rf_type == RF_2T2R)
			rtl_set_bbreg(rtlpriv, rL1PeakTH_Jaguar, 0x03C00000, 7);	/* 2R 0x848[25:22] = 0x7 */
		else
			rtl_set_bbreg(rtlpriv, rL1PeakTH_Jaguar, 0x03C00000, 8);	/* 1R 0x848[25:22] = 0x8 */

		break;

	case CHANNEL_WIDTH_40:
		rtl_set_bbreg(rtlpriv, rRFMOD_Jaguar, 0x003003C3, 0x00300201);	/* 0x8ac[21,20,9:6,1,0]=8'b11100000 */
		rtl_set_bbreg(rtlpriv, rADC_Buf_Clk_Jaguar, BIT30, 0);		/* 0x8c4[30] = 1'b0 */
		rtl_set_bbreg(rtlpriv, rRFMOD_Jaguar, 0x3C, SubChnlNum);
		rtl_set_bbreg(rtlpriv, rCCAonSec_Jaguar, 0xf0000000, SubChnlNum);

		rtl_set_bbreg(rtlpriv, rFPGA0_XB_RFInterfaceOE, 0x001C0000, 2);	/* 0x864[20:18] = 3'b2 */

		if(pHalData->Reg837 & BIT2)
			L1pkVal = 6;
		else {
			if(rtlpriv->phy.rf_type == RF_2T2R)
				L1pkVal = 7;
			else
				L1pkVal = 8;
		}

		rtl_set_bbreg(rtlpriv, rL1PeakTH_Jaguar, 0x03C00000, L1pkVal);	/* 0x848[25:22] = 0x6 */

		if(SubChnlNum == VHT_DATA_SC_20_UPPER_OF_80MHZ)
			rtl_set_bbreg(rtlpriv, rCCK_System_Jaguar, bCCK_System_Jaguar, 1);
		else
			rtl_set_bbreg(rtlpriv, rCCK_System_Jaguar, bCCK_System_Jaguar, 0);
		break;

	case CHANNEL_WIDTH_80:
		rtl_set_bbreg(rtlpriv, rRFMOD_Jaguar, 0x003003C3, 0x00300202);	/* 0x8ac[21,20,9:6,1,0]=8'b11100010 */
		rtl_set_bbreg(rtlpriv, rADC_Buf_Clk_Jaguar, BIT30, 1);		/* 0x8c4[30] = 1 */
		rtl_set_bbreg(rtlpriv, rRFMOD_Jaguar, 0x3C, SubChnlNum);
		rtl_set_bbreg(rtlpriv, rCCAonSec_Jaguar, 0xf0000000, SubChnlNum);

		rtl_set_bbreg(rtlpriv, rFPGA0_XB_RFInterfaceOE, 0x001C0000, 2);	/* 0x864[20:18] = 3'b2 */

		if(pHalData->Reg837 & BIT2)
			L1pkVal = 5;
		else {
			if(rtlpriv->phy.rf_type == RF_2T2R)
				L1pkVal = 6;
			else
				L1pkVal = 7;
		}
		rtl_set_bbreg(rtlpriv, rL1PeakTH_Jaguar, 0x03C00000, L1pkVal);	/* 0x848[25:22] = 0x5 */

		break;

	default:
		DBG_871X("phy_PostSetBWMode8812():	unknown Bandwidth: %#X\n",rtlpriv->phy.current_chan_bw);
		break;
	}

	/* <20121109, Kordan> A workaround for 8812A only. */
	rtl8812au_fixspur(rtlpriv, rtlpriv->phy.current_chan_bw, rtlpriv->phy.current_channel);

	/*
	 * DBG_871X("phy_PostSetBwMode8812(): Reg483: %x\n", rtl_read_byte(rtlpriv, 0x483));
	 * DBG_871X("phy_PostSetBwMode8812(): Reg668: %x\n", rtl_read_dword(rtlpriv, 0x668));
	 * DBG_871X("phy_PostSetBwMode8812(): Reg8AC: %x\n", rtl_get_bbreg(rtlpriv, rRFMOD_Jaguar, 0xffffffff));
	 */

	/* 3 Set RF related register */
	rtl8821au_phy_rf6052_set_bandwidth(rtlpriv, rtlpriv->phy.current_chan_bw);
}

uint32_t phy_get_tx_swing_8821au(struct rtl_priv *rtlpriv, enum band_type Band,
	uint8_t	RFPath)
{
	struct rtl_dm *rtldm = &(rtlpriv->dm);
	struct rtl_hal *rtlhal = rtl_hal(rtlpriv);
	EEPROM_EFUSE_PRIV	*pEEPROM = GET_EEPROM_EFUSE_PRIV(rtlpriv);
	s8	bbSwing_2G = -1 * GetRegTxBBSwing_2G(rtlpriv);
	s8	bbSwing_5G = -1 * GetRegTxBBSwing_5G(rtlpriv);
	uint32_t	out = 0x200;
	const s8	AUTO = -1;


	if (pEEPROM->bautoload_fail_flag) {
		if (Band == BAND_ON_2_4G) {
			rtldm->swing_diff_2g = bbSwing_2G;
			if      (bbSwing_2G == 0)
				out = 0x200; /*  0 dB */
		        else if (bbSwing_2G == -3)
				out = 0x16A; /* -3 dB */
		        else if (bbSwing_2G == -6)
				out = 0x101; /* -6 dB */
		        else if (bbSwing_2G == -9)
				out = 0x0B6; /* -9 dB */
		        else {
				if (rtlhal->external_pa_2g) {
					rtldm->swing_diff_2g = -3;
					out = 0x16A;
				} else  {
					rtldm->swing_diff_2g = 0;
					out = 0x200;
				}
			}
		} else if (Band == BAND_ON_5G) {
			rtldm->swing_diff_5g = bbSwing_5G;
			if      (bbSwing_5G == 0)
				out = 0x200; /*  0 dB */
			else if (bbSwing_5G == -3)
				out = 0x16A; /* -3 dB */
			else if (bbSwing_5G == -6)
				out = 0x101; /* -6 dB */
			else if (bbSwing_5G == -9)
				out = 0x0B6; /* -9 dB */
			else {
				if (rtlhal->external_pa_5g) {
					rtldm->swing_diff_5g = -3;
					out = 0x16A;
				} else  {
					rtldm->swing_diff_5g = 0;
					out = 0x200;
				}
			}
		} else  {
			rtldm->swing_diff_2g = -3;
			rtldm->swing_diff_5g = -3;
			out = 0x16A; /* -3 dB */
		}
	} else {
		uint32_t swing = 0, swingA = 0, swingB = 0;

		if (Band == BAND_ON_2_4G) {
			if (GetRegTxBBSwing_2G(rtlpriv) == AUTO) {
				EFUSE_ShadowRead(rtlpriv, 1, EEPROM_TX_BBSWING_2G_8812, (uint32_t *)&swing);
				swing = (swing == 0xFF) ? 0x00 : swing;
			} else if (bbSwing_2G ==  0)
				swing = 0x00; /*  0 dB */
			else if (bbSwing_2G == -3)
				swing = 0x05; /* -3 dB */
			else if (bbSwing_2G == -6)
				swing = 0x0A; // -6 dB */
			else if (bbSwing_2G == -9)
				swing = 0xFF; // -9 dB */
			else swing = 0x00;
		} else {
			if (GetRegTxBBSwing_5G(rtlpriv) == AUTO) {
				EFUSE_ShadowRead(rtlpriv, 1, EEPROM_TX_BBSWING_5G_8812, (uint32_t *)&swing);
				swing = (swing == 0xFF) ? 0x00 : swing;
			} else if (bbSwing_5G ==  0)
				swing = 0x00; /*  0 dB */
			else if (bbSwing_5G == -3)
				swing = 0x05; /* -3 dB */
			else if (bbSwing_5G == -6)
				swing = 0x0A; /* -6 dB */
			else if (bbSwing_5G == -9)
				swing = 0xFF; /* -9 dB */
			else swing = 0x00;
		}

		swingA = (swing & 0x3) >> 0; /* 0xC6/C7[1:0] */
		swingB = (swing & 0xC) >> 2; /* 0xC6/C7[3:2] */

		/* DBG_871X("===> PHY_GetTxBBSwing_8812A, swingA: 0x%X, swingB: 0x%X\n", swingA, swingB); */

		/* 3 Path-A */
		if (swingA == 0x00) {
			if (Band == BAND_ON_2_4G)
				rtldm->swing_diff_2g = 0;
			else
				rtldm->swing_diff_5g = 0;
			out = 0x200; /* 0 dB */
		} else if (swingA == 0x01) {
			if (Band == BAND_ON_2_4G)
				rtldm->swing_diff_2g = -3;
			else
				rtldm->swing_diff_5g = -3;
			out = 0x16A; /*  -3 dB */
		} else if (swingA == 0x10) {
			if (Band == BAND_ON_2_4G)
				rtldm->swing_diff_2g = -6;
			else
				rtldm->swing_diff_5g = -6;
			out = 0x101; /* -6 dB */
		} else if (swingA == 0x11) {
			if (Band == BAND_ON_2_4G)
				rtldm->swing_diff_2g = -9;
			else
				rtldm->swing_diff_5g = -9;
			out = 0x0B6; /* -9 dB */
		}

		/* 3 Path-B */
		if (swingB == 0x00) {
			if (Band == BAND_ON_2_4G)
				rtldm->swing_diff_2g = 0;
			else
				rtldm->swing_diff_5g = 0;
			out = 0x200; /* 0 dB */
		} else if (swingB == 0x01) {
			if (Band == BAND_ON_2_4G)
				rtldm->swing_diff_2g = -3;
			else
				rtldm->swing_diff_5g = -3;
			out = 0x16A; /* -3 dB */
		} else if (swingB == 0x10) {
			if (Band == BAND_ON_2_4G)
				rtldm->swing_diff_2g = -6;
			else
				rtldm->swing_diff_5g = -6;
			out = 0x101; /* -6 dB */
		} else if (swingB == 0x11) {
			if (Band == BAND_ON_2_4G)
				rtldm->swing_diff_2g = -9;
			else
				rtldm->swing_diff_5g = -9;
			out = 0x0B6; /* -9 dB */
		}
	}

	/* DBG_871X("<=== PHY_GetTxBBSwing_8812A, out = 0x%X\n", out); */

	return out;
}


/* 
 * ULLI : in rtlwifi check is needed for swithing band 
 * ULLI : is in rtl8821au_phy_switch_wirelessband() 
 * */

BOOLEAN phy_SwBand8812(struct rtl_priv *rtlpriv, uint8_t channelToSW)
{
	uint8_t			u1Btmp;
	BOOLEAN		ret_value = _TRUE;
	uint8_t			Band = BAND_ON_5G, BandToSW;

	u1Btmp = rtl_read_byte(rtlpriv, REG_CCK_CHECK_8812);
	if(u1Btmp & BIT7)
		Band = BAND_ON_5G;
	else
		Band = BAND_ON_2_4G;

	/* Use current channel to judge Band Type and switch Band if need. */
	if(channelToSW > 14) {
		BandToSW = BAND_ON_5G;
	} else {
		BandToSW = BAND_ON_2_4G;
	}

	if(BandToSW != Band)
		rtl8821au_phy_switch_wirelessband(rtlpriv,BandToSW);

	return ret_value;
}
static void phy_SetRFEReg8812(struct rtl_priv *rtlpriv,uint8_t Band)
{
	struct rtl_hal *rtlhal = rtl_hal(rtlpriv);
	u8			u1tmp = 0;

	if(Band == BAND_ON_2_4G) {
		switch(rtlhal->rfe_type){
		case 0: case 1: case 2:
			rtl_set_bbreg(rtlpriv, rA_RFE_Pinmux_Jaguar,bMaskDWord, 0x77777777);
			rtl_set_bbreg(rtlpriv, rB_RFE_Pinmux_Jaguar,bMaskDWord, 0x77777777);
			rtl_set_bbreg(rtlpriv, rA_RFE_Inv_Jaguar,bMask_RFEInv_Jaguar, 0x000);
			rtl_set_bbreg(rtlpriv, rB_RFE_Inv_Jaguar,bMask_RFEInv_Jaguar, 0x000);
			break;
		case 3:
			rtl_set_bbreg(rtlpriv, rA_RFE_Pinmux_Jaguar,bMaskDWord, 0x54337770);
			rtl_set_bbreg(rtlpriv, rB_RFE_Pinmux_Jaguar,bMaskDWord, 0x54337770);
			rtl_set_bbreg(rtlpriv, rA_RFE_Inv_Jaguar,bMask_RFEInv_Jaguar, 0x010);
			rtl_set_bbreg(rtlpriv, rB_RFE_Inv_Jaguar,bMask_RFEInv_Jaguar, 0x010);
			rtl_set_bbreg(rtlpriv, r_ANTSEL_SW_Jaguar,0x00000303, 0x1);
			break;
		case 4:
			rtl_set_bbreg(rtlpriv, rA_RFE_Pinmux_Jaguar,bMaskDWord, 0x77777777);
			rtl_set_bbreg(rtlpriv, rB_RFE_Pinmux_Jaguar,bMaskDWord, 0x77777777);
			rtl_set_bbreg(rtlpriv, rA_RFE_Inv_Jaguar,bMask_RFEInv_Jaguar, 0x001);
			rtl_set_bbreg(rtlpriv, rB_RFE_Inv_Jaguar,bMask_RFEInv_Jaguar, 0x001);
			break;
		case 5:
			/* if(BT_IsBtExist(rtlpriv)) */
			{
				/* rtl_write_word(rtlpriv, rA_RFE_Pinmux_Jaguar, 0x7777); */
				rtl_write_byte(rtlpriv, rA_RFE_Pinmux_Jaguar+2, 0x77);
			}
			/* else */
				/* rtl_set_bbreg(rtlpriv, rA_RFE_Pinmux_Jaguar,bMaskDWord, 0x77777777); */

			rtl_set_bbreg(rtlpriv, rB_RFE_Pinmux_Jaguar,bMaskDWord, 0x77777777);

			/* if(BT_IsBtExist(rtlpriv)) */
			{
				/* 
				 * u1tmp = rtl_read_byte(rtlpriv, rA_RFE_Inv_Jaguar+2);
				 * rtl_write_byte(rtlpriv, rA_RFE_Inv_Jaguar+2,  (u1tmp &0x0f));
				 */
				u1tmp = rtl_read_byte(rtlpriv, rA_RFE_Inv_Jaguar+3);
				rtl_write_byte(rtlpriv, rA_RFE_Inv_Jaguar+3,  (u1tmp &= ~BIT0));
			}
			/* else */
				/* rtl_set_bbreg(rtlpriv, rA_RFE_Inv_Jaguar, bMask_RFEInv_Jaguar, 0x000); */

			rtl_set_bbreg(rtlpriv, rB_RFE_Inv_Jaguar, bMask_RFEInv_Jaguar, 0x000);
			break;
		default:
			break;
		}
	} else {
		switch(rtlhal->rfe_type){
		case 0:
			rtl_set_bbreg(rtlpriv, rA_RFE_Pinmux_Jaguar,bMaskDWord, 0x77337717);
			rtl_set_bbreg(rtlpriv, rB_RFE_Pinmux_Jaguar,bMaskDWord, 0x77337717);
			rtl_set_bbreg(rtlpriv, rA_RFE_Inv_Jaguar,bMask_RFEInv_Jaguar, 0x010);
			rtl_set_bbreg(rtlpriv, rB_RFE_Inv_Jaguar,bMask_RFEInv_Jaguar, 0x010);
			break;
		case 1:
			rtl_set_bbreg(rtlpriv, rA_RFE_Pinmux_Jaguar,bMaskDWord, 0x77337717);
			rtl_set_bbreg(rtlpriv, rB_RFE_Pinmux_Jaguar,bMaskDWord, 0x77337717);
			rtl_set_bbreg(rtlpriv, rA_RFE_Inv_Jaguar,bMask_RFEInv_Jaguar, 0x000);
			rtl_set_bbreg(rtlpriv, rB_RFE_Inv_Jaguar,bMask_RFEInv_Jaguar, 0x000);
			break;
		case 2: case 4:
			rtl_set_bbreg(rtlpriv, rA_RFE_Pinmux_Jaguar,bMaskDWord, 0x77337777);
			rtl_set_bbreg(rtlpriv, rB_RFE_Pinmux_Jaguar,bMaskDWord, 0x77337777);
			rtl_set_bbreg(rtlpriv, rA_RFE_Inv_Jaguar,bMask_RFEInv_Jaguar, 0x010);
			rtl_set_bbreg(rtlpriv, rB_RFE_Inv_Jaguar,bMask_RFEInv_Jaguar, 0x010);
			break;
		case 3:
			rtl_set_bbreg(rtlpriv, rA_RFE_Pinmux_Jaguar,bMaskDWord, 0x54337717);
			rtl_set_bbreg(rtlpriv, rB_RFE_Pinmux_Jaguar,bMaskDWord, 0x54337717);
			rtl_set_bbreg(rtlpriv, rA_RFE_Inv_Jaguar,bMask_RFEInv_Jaguar, 0x010);
			rtl_set_bbreg(rtlpriv, rB_RFE_Inv_Jaguar,bMask_RFEInv_Jaguar, 0x010);
			rtl_set_bbreg(rtlpriv, r_ANTSEL_SW_Jaguar,0x00000303, 0x1);
			break;
		case 5:
			//if(BT_IsBtExist(rtlpriv))
			{
				//rtl_write_word(rtlpriv, rA_RFE_Pinmux_Jaguar, 0x7777);
				if(rtlhal->external_pa_5g)
					rtl_write_byte(rtlpriv, rA_RFE_Pinmux_Jaguar+2, 0x33);
				else
					rtl_write_byte(rtlpriv, rA_RFE_Pinmux_Jaguar+2, 0x73);
			}
#if 0
			else
			{
				if (rtlhal->external_pa_5g)
					rtl_set_bbreg(rtlpriv, rA_RFE_Pinmux_Jaguar,bMaskDWord, 0x77337777);
				else
					rtl_set_bbreg(rtlpriv, rA_RFE_Pinmux_Jaguar,bMaskDWord, 0x77737777);
			}
#endif

			if (rtlhal->external_pa_5g)
				rtl_set_bbreg(rtlpriv, rB_RFE_Pinmux_Jaguar,bMaskDWord, 0x77337777);
			else
				rtl_set_bbreg(rtlpriv, rB_RFE_Pinmux_Jaguar,bMaskDWord, 0x77737777);

			/* if(BT_IsBtExist(rtlpriv)) */
			{
				/*
				 * u1tmp = rtl_read_byte(rtlpriv, rA_RFE_Inv_Jaguar+2);
				 * rtl_write_byte(rtlpriv, rA_RFE_Inv_Jaguar+2,  (u1tmp &0x0f));
				 */
				u1tmp = rtl_read_byte(rtlpriv, rA_RFE_Inv_Jaguar+3);
				rtl_write_byte(rtlpriv, rA_RFE_Inv_Jaguar+3,  (u1tmp |= BIT0));
			}
			/* else */
				/* rtl_set_bbreg(rtlpriv, rA_RFE_Inv_Jaguar, bMask_RFEInv_Jaguar, 0x010); */

			rtl_set_bbreg(rtlpriv, rB_RFE_Inv_Jaguar, bMask_RFEInv_Jaguar, 0x010);
			break;
		default:
			break;
		}
	}
}

void rtl8821au_phy_switch_wirelessband(struct rtl_priv *rtlpriv, u8 Band)
{
	struct rtl_hal *rtlhal = rtl_hal(rtlpriv);
	struct _rtw_hal	*pHalData = GET_HAL_DATA(rtlpriv);
	uint8_t				currentBand = rtlhal->current_bandtype;

	/* DBG_871X("==>rtl8821au_phy_switch_wirelessband() %s\n", ((Band==0)?"2.4G":"5G")); */

	rtlhal->current_bandtype =(enum band_type)Band;

	if(Band == BAND_ON_2_4G) {	/* 2.4G band */

		/* STOP Tx/Rx */
		rtl_set_bbreg(rtlpriv, rOFDMCCKEN_Jaguar, bOFDMEN_Jaguar|bCCKEN_Jaguar, 0x00);

		if (IS_HARDWARE_TYPE_8821(rtlhal)) {
			/* Turn off RF PA and LNA */
			rtl_set_bbreg(rtlpriv, rA_RFE_Pinmux_Jaguar, 0xF000, 0x7);	/* 0xCB0[15:12] = 0x7 (LNA_On) */
			rtl_set_bbreg(rtlpriv, rA_RFE_Pinmux_Jaguar, 0xF0, 0x7);	/* 0xCB0[7:4] = 0x7 (PAPE_A) */
		}

		/* AGC table select */
		if(IS_VENDOR_8821A_MP_CHIP(rtlpriv))
			rtl_set_bbreg(rtlpriv, rA_TxScale_Jaguar, 0xF00, 0); // 0xC1C[11:8] = 0
		else
			rtl_set_bbreg(rtlpriv, rAGC_table_Jaguar, 0x3, 0);

		if(IS_VENDOR_8812A_TEST_CHIP(rtlpriv)) {
			/* r_select_5G for path_A/B */
			rtl_set_bbreg(rtlpriv, rA_RFE_Jaguar, BIT12, 0x0);
			rtl_set_bbreg(rtlpriv, rB_RFE_Jaguar, BIT12, 0x0);

			/* LANON (5G uses external LNA) */
			rtl_set_bbreg(rtlpriv, rA_RFE_Jaguar, BIT15, 0x1);
			rtl_set_bbreg(rtlpriv, rB_RFE_Jaguar, BIT15, 0x1);
		} else if(IS_VENDOR_8812A_MP_CHIP(rtlpriv)) {
			if(GetRegbENRFEType(rtlpriv))
				phy_SetRFEReg8812(rtlpriv, Band);
			else {
				/* PAPE_A (bypass RFE module in 2G) */
				rtl_set_bbreg(rtlpriv, rA_RFE_Pinmux_Jaguar, 0x000000F0, 0x7);
				rtl_set_bbreg(rtlpriv, rB_RFE_Pinmux_Jaguar, 0x000000F0, 0x7);

				/* PAPE_G (bypass RFE module in 5G) */
				if (rtlhal->external_pa_2g) {
					rtl_set_bbreg(rtlpriv, rA_RFE_Pinmux_Jaguar, 0x0000000F, 0x0);
					rtl_set_bbreg(rtlpriv, rB_RFE_Pinmux_Jaguar, 0x0000000F, 0x0);
				} else {
					rtl_set_bbreg(rtlpriv, rA_RFE_Pinmux_Jaguar, 0x0000000F, 0x7);
					rtl_set_bbreg(rtlpriv, rB_RFE_Pinmux_Jaguar, 0x0000000F, 0x7);
				}

				/* TRSW bypass RFE moudle in 2G */
				if (rtlhal->external_lna_2g) {
					rtl_set_bbreg(rtlpriv, rA_RFE_Pinmux_Jaguar, MASKBYTE2, 0x54);
					rtl_set_bbreg(rtlpriv, rB_RFE_Pinmux_Jaguar, MASKBYTE2, 0x54);
				} else {
					rtl_set_bbreg(rtlpriv, rA_RFE_Pinmux_Jaguar, MASKBYTE2, 0x77);
					rtl_set_bbreg(rtlpriv, rB_RFE_Pinmux_Jaguar, MASKBYTE2, 0x77);
				}
			}
		}

		update_tx_basic_rate(rtlpriv, WIRELESS_11BG);

		/* cck_enable */
		rtl_set_bbreg(rtlpriv, rOFDMCCKEN_Jaguar, bOFDMEN_Jaguar|bCCKEN_Jaguar, 0x3);

		/* SYN Setting */
		if(IS_VENDOR_8812A_TEST_CHIP(rtlpriv)) 	{
			rtw_hal_write_rfreg(rtlpriv, RF90_PATH_A, 0xEF, bLSSIWrite_data_Jaguar, 0x40000);
			rtw_hal_write_rfreg(rtlpriv, RF90_PATH_A, 0x3E, bLSSIWrite_data_Jaguar, 0x00000);
			rtw_hal_write_rfreg(rtlpriv, RF90_PATH_A, 0x3F, bLSSIWrite_data_Jaguar, 0x0001c);
			rtw_hal_write_rfreg(rtlpriv, RF90_PATH_A, 0xEF, bLSSIWrite_data_Jaguar, 0x00000);
			rtw_hal_write_rfreg(rtlpriv, RF90_PATH_A, 0xB5, bLSSIWrite_data_Jaguar, 0x16BFF);
		}

		/* CCK_CHECK_en */
		rtl_write_byte(rtlpriv, REG_CCK_CHECK_8812, 0x0);
	} else {		/* 5G band */
		u16	count = 0, reg41A = 0;

		if (IS_HARDWARE_TYPE_8821(rtlhal)) {
			rtl_set_bbreg(rtlpriv, rA_RFE_Pinmux_Jaguar, 0xF000, 0x5);	/* 0xCB0[15:12] = 0x5 (LNA_On) */
			rtl_set_bbreg(rtlpriv, rA_RFE_Pinmux_Jaguar, 0xF0, 0x4);	/* 0xCB0[7:4] = 0x4 (PAPE_A) */
		}

		/* CCK_CHECK_en */
		rtl_write_byte(rtlpriv, REG_CCK_CHECK_8812, 0x80);

		count = 0;
		reg41A = rtl_read_word(rtlpriv, REG_TXPKT_EMPTY);
		/* DBG_871X("Reg41A value %d", reg41A); */
		reg41A &= 0x30;
		while((reg41A!= 0x30) && (count < 50)) {
			udelay(50);
			/* DBG_871X("Delay 50us \n"); */

			reg41A = rtl_read_word(rtlpriv, REG_TXPKT_EMPTY);
			reg41A &= 0x30;
			count++;
			/* DBG_871X("Reg41A value %d", reg41A); */
		}
		if(count != 0)
			DBG_871X("rtl8821au_phy_switch_wirelessband(): Switch to 5G Band. Count = %d reg41A=0x%x\n", count, reg41A);

		/* STOP Tx/Rx */
		rtl_set_bbreg(rtlpriv, rOFDMCCKEN_Jaguar, bOFDMEN_Jaguar|bCCKEN_Jaguar, 0x00);

		/* AGC table select */
		if (IS_VENDOR_8821A_MP_CHIP(rtlpriv))
			rtl_set_bbreg(rtlpriv, rA_TxScale_Jaguar, 0xF00, 1); /* 0xC1C[11:8] = 1 */
		else
			rtl_set_bbreg(rtlpriv, rAGC_table_Jaguar, 0x3, 1);

		if(IS_VENDOR_8812A_TEST_CHIP(rtlpriv)) 	{
			/* r_select_5G for path_A/B */
			rtl_set_bbreg(rtlpriv, rA_RFE_Jaguar, BIT12, 0x1);
			rtl_set_bbreg(rtlpriv, rB_RFE_Jaguar, BIT12, 0x1);

			/* LANON (5G uses external LNA) */
			rtl_set_bbreg(rtlpriv, rA_RFE_Jaguar, BIT15, 0x0);
			rtl_set_bbreg(rtlpriv, rB_RFE_Jaguar, BIT15, 0x0);
		} else if(IS_VENDOR_8812A_MP_CHIP(rtlpriv)) {
			if(GetRegbENRFEType(rtlpriv))
				phy_SetRFEReg8812(rtlpriv, Band);
			else {
				/* PAPE_A (bypass RFE module in 2G) */
				if (rtlhal->external_pa_5g) {
					rtl_set_bbreg(rtlpriv, rA_RFE_Pinmux_Jaguar, 0x000000F0, 0x1);
					rtl_set_bbreg(rtlpriv, rB_RFE_Pinmux_Jaguar, 0x000000F0, 0x1);
				} else {
					rtl_set_bbreg(rtlpriv, rA_RFE_Pinmux_Jaguar, 0x000000F0, 0x0);
					rtl_set_bbreg(rtlpriv, rB_RFE_Pinmux_Jaguar, 0x000000F0, 0x0);
				}

				/* PAPE_G (bypass RFE module in 5G) */
				rtl_set_bbreg(rtlpriv, rA_RFE_Pinmux_Jaguar, 0x0000000F, 0x7);
				rtl_set_bbreg(rtlpriv, rB_RFE_Pinmux_Jaguar, 0x0000000F, 0x7);

				/* TRSW bypass RFE moudle in 2G */
				if (rtlhal->external_lna_5g) {
					rtl_set_bbreg(rtlpriv, rA_RFE_Pinmux_Jaguar, MASKBYTE2, 0x54);
					rtl_set_bbreg(rtlpriv, rB_RFE_Pinmux_Jaguar, MASKBYTE2, 0x54);
				} else {
					rtl_set_bbreg(rtlpriv, rA_RFE_Pinmux_Jaguar, MASKBYTE2, 0x77);
					rtl_set_bbreg(rtlpriv, rB_RFE_Pinmux_Jaguar, MASKBYTE2, 0x77);
				}
			}
		}

		/*
		 * avoid using cck rate in 5G band
		 * Set RRSR rate table.
		 */
		update_tx_basic_rate(rtlpriv, WIRELESS_11A);

		/* cck_enable */
		rtl_set_bbreg(rtlpriv, rOFDMCCKEN_Jaguar, bOFDMEN_Jaguar|bCCKEN_Jaguar, 0x2);

		/* SYN Setting */
		if(IS_VENDOR_8812A_TEST_CHIP(rtlpriv)) 	{
			rtw_hal_write_rfreg(rtlpriv, RF90_PATH_A, 0xEF, bLSSIWrite_data_Jaguar, 0x40000);
			rtw_hal_write_rfreg(rtlpriv, RF90_PATH_A, 0x3E, bLSSIWrite_data_Jaguar, 0x00000);
			rtw_hal_write_rfreg(rtlpriv, RF90_PATH_A, 0x3F, bLSSIWrite_data_Jaguar, 0x00017);
			rtw_hal_write_rfreg(rtlpriv, RF90_PATH_A, 0xEF, bLSSIWrite_data_Jaguar, 0x00000);
			rtw_hal_write_rfreg(rtlpriv, RF90_PATH_A, 0xB5, bLSSIWrite_data_Jaguar, 0x04BFF);
		}

		/* DBG_871X("==>rtl8821au_phy_switch_wirelessband() BAND_ON_5G settings OFDM index 0x%x\n", pHalData->OFDM_index[RF90_PATH_A]); */
	}

	/* <20120903, Kordan> Tx BB swing setting for RL6286, asked by Ynlin. */
	if (IS_NORMAL_CHIP(pHalData->VersionID) || IS_HARDWARE_TYPE_8821(rtlhal)) {
		s8	BBDiffBetweenBand = 0;
		struct rtl_dm	*rtldm = rtl_dm(rtlpriv);
		struct _rtw_hal	*pHalData = GET_HAL_DATA(rtlpriv);
	 	struct _rtw_dm *	pDM_Odm = &pHalData->odmpriv;

		rtl_set_bbreg(rtlpriv, rA_TxScale_Jaguar, 0xFFE00000,
					 phy_get_tx_swing_8821au(rtlpriv, (enum band_type)Band, RF90_PATH_A)); // 0xC1C[31:21]
		rtl_set_bbreg(rtlpriv, rB_TxScale_Jaguar, 0xFFE00000,
					 phy_get_tx_swing_8821au(rtlpriv, (enum band_type)Band, RF90_PATH_B)); // 0xE1C[31:21]

		/*
		 *  <20121005, Kordan> When TxPowerTrack is ON, we should take care of the change of BB swing.
		 *  That is, reset all info to trigger Tx power tracking.
		 */
		{
			if (Band != currentBand) {
				BBDiffBetweenBand = (rtldm->swing_diff_2g - rtldm->swing_diff_5g);
				BBDiffBetweenBand = (Band == BAND_ON_2_4G) ? BBDiffBetweenBand : (-1 * BBDiffBetweenBand);
				rtldm->default_ofdm_index += BBDiffBetweenBand*2;
			}

			ODM_ClearTxPowerTrackingState(pDM_Odm);
		}
	}

	/* DBG_871X("<==rtl8821au_phy_switch_wirelessband():Switch Band OK.\n"); */
}

static void _rtl8821au_phy_set_txpower_index(struct rtl_priv *rtlpriv, uint32_t power_index,
	u8 path, u8 rate)
{
	struct rtl_hal *rtlhal = rtl_hal(rtlpriv);
	struct _rtw_hal *pHalData = GET_HAL_DATA(rtlpriv);
	BOOLEAN		Direction = FALSE;
	uint32_t	TxagcOffset = 0;

	/*
	 *  <20120928, Kordan> A workaround in 8812A/8821A testchip, to fix the bug of odd Tx power indexes.
	 */
	if ((power_index % 2 == 1) && IS_TEST_CHIP(pHalData->VersionID))
		power_index -= 1;

	/* 2013.01.18 LukeLee: Modify TXAGC by dcmd_Dynamic_Ctrl() */
	if (path == RF90_PATH_A) {
		Direction = pHalData->odmpriv.IsTxagcOffsetPositiveA;
		TxagcOffset = pHalData->odmpriv.TxagcOffsetValueA;
	} else if (path == RF90_PATH_B) {
		Direction = pHalData->odmpriv.IsTxagcOffsetPositiveB;
		TxagcOffset = pHalData->odmpriv.TxagcOffsetValueB;
	}
	
	if (Direction == FALSE) {
		if(power_index > TxagcOffset)
			power_index -= TxagcOffset;
		else
			power_index = 0;
	} else {
		power_index += TxagcOffset;
		if (power_index > 0x3F)
			power_index = 0x3F;
	}

	/* ULLI check register names as in rtlwifi-lib */

	if (path == RF90_PATH_A) {
		switch (rate) {
		case MGN_1M:    
			rtl_set_bbreg(rtlpriv, RTXAGC_A_CCK11_CCK1, MASKBYTE0, power_index);
			break;
		case MGN_2M:    
			rtl_set_bbreg(rtlpriv, RTXAGC_A_CCK11_CCK1, MASKBYTE1, power_index);
			break;
		case MGN_5_5M:  
			rtl_set_bbreg(rtlpriv, RTXAGC_A_CCK11_CCK1, MASKBYTE2, power_index);
			break;
		case MGN_11M:   
			rtl_set_bbreg(rtlpriv, RTXAGC_A_CCK11_CCK1, MASKBYTE3, power_index);
			break;

		case MGN_6M:    
			rtl_set_bbreg(rtlpriv, RTXAGC_A_OFDM18_OFDM6, MASKBYTE0, power_index);
			break;
		case MGN_9M:    
			rtl_set_bbreg(rtlpriv, RTXAGC_A_OFDM18_OFDM6, MASKBYTE1, power_index);
			break;
		case MGN_12M:   
			rtl_set_bbreg(rtlpriv, RTXAGC_A_OFDM18_OFDM6, MASKBYTE2, power_index);
			break;
		case MGN_18M:   
			rtl_set_bbreg(rtlpriv, RTXAGC_A_OFDM18_OFDM6, MASKBYTE3, power_index);
			break;
		
		case MGN_24M:   
			rtl_set_bbreg(rtlpriv, RTXAGC_A_OFDM54_OFDM24, MASKBYTE0, power_index);
			break;
		case MGN_36M:   
			rtl_set_bbreg(rtlpriv, RTXAGC_A_OFDM54_OFDM24, MASKBYTE1, power_index);
			break;
		case MGN_48M:   
			rtl_set_bbreg(rtlpriv, RTXAGC_A_OFDM54_OFDM24, MASKBYTE2, power_index);
			break;
		case MGN_54M:   
			rtl_set_bbreg(rtlpriv, RTXAGC_A_OFDM54_OFDM24, MASKBYTE3, power_index);
			break;
		
		case MGN_MCS0:  
			rtl_set_bbreg(rtlpriv, RTXAGC_A_MCS03_MCS00, MASKBYTE0, power_index);
			break;
		case MGN_MCS1:  
			rtl_set_bbreg(rtlpriv, RTXAGC_A_MCS03_MCS00, MASKBYTE1, power_index);
			break;
		case MGN_MCS2:  
			rtl_set_bbreg(rtlpriv, RTXAGC_A_MCS03_MCS00, MASKBYTE2, power_index);
			break;
		case MGN_MCS3:  
			rtl_set_bbreg(rtlpriv, RTXAGC_A_MCS03_MCS00, MASKBYTE3, power_index);
			break;
		
		case MGN_MCS4:  
			rtl_set_bbreg(rtlpriv, RTXAGC_A_MCS07_MCS04, MASKBYTE0, power_index);
			break;
		case MGN_MCS5:  
			rtl_set_bbreg(rtlpriv, RTXAGC_A_MCS07_MCS04, MASKBYTE1, power_index);
			break;
		case MGN_MCS6:  
			rtl_set_bbreg(rtlpriv, RTXAGC_A_MCS07_MCS04, MASKBYTE2, power_index);
			break;
		case MGN_MCS7:  
			rtl_set_bbreg(rtlpriv, RTXAGC_A_MCS07_MCS04, MASKBYTE3, power_index);
			break;
		
		case MGN_MCS8:  
			rtl_set_bbreg(rtlpriv, RTXAGC_A_MCS11_MCS08, MASKBYTE0, power_index);
			break;
		case MGN_MCS9:  
			rtl_set_bbreg(rtlpriv, RTXAGC_A_MCS11_MCS08, MASKBYTE1, power_index);
			break;
		case MGN_MCS10: 
			rtl_set_bbreg(rtlpriv, RTXAGC_A_MCS11_MCS08, MASKBYTE2, power_index);
			break;
		case MGN_MCS11: 
			rtl_set_bbreg(rtlpriv, RTXAGC_A_MCS11_MCS08, MASKBYTE3, power_index);
			break;
		
		case MGN_MCS12: 
			rtl_set_bbreg(rtlpriv, RTXAGC_A_MCS15_MCS12, MASKBYTE0, power_index);
			break;
		case MGN_MCS13: 
			rtl_set_bbreg(rtlpriv, RTXAGC_A_MCS15_MCS12, MASKBYTE1, power_index);
			break;
		case MGN_MCS14: 
			rtl_set_bbreg(rtlpriv, RTXAGC_A_MCS15_MCS12, MASKBYTE2, power_index);
			break;
		case MGN_MCS15: 
			rtl_set_bbreg(rtlpriv, RTXAGC_A_MCS15_MCS12, MASKBYTE3, power_index);
			break;
		
		case MGN_VHT1SS_MCS0: 
			rtl_set_bbreg(rtlpriv, RTXAGC_A_NSS1INDEX3_NSS1INDEX0, MASKBYTE0, power_index);
			break;
		case MGN_VHT1SS_MCS1: 
			rtl_set_bbreg(rtlpriv, RTXAGC_A_NSS1INDEX3_NSS1INDEX0, MASKBYTE1, power_index);
			break;
		case MGN_VHT1SS_MCS2: 
			rtl_set_bbreg(rtlpriv, RTXAGC_A_NSS1INDEX3_NSS1INDEX0, MASKBYTE2, power_index);
			break;
		case MGN_VHT1SS_MCS3: 
			rtl_set_bbreg(rtlpriv, RTXAGC_A_NSS1INDEX3_NSS1INDEX0, MASKBYTE3, power_index);
			break;
		
		case MGN_VHT1SS_MCS4: 
			rtl_set_bbreg(rtlpriv, RTXAGC_A_NSS1INDEX7_NSS1INDEX4, MASKBYTE0, power_index);
			break;
		case MGN_VHT1SS_MCS5: 
			rtl_set_bbreg(rtlpriv, RTXAGC_A_NSS1INDEX7_NSS1INDEX4, MASKBYTE1, power_index);
			break;
		case MGN_VHT1SS_MCS6: 
			rtl_set_bbreg(rtlpriv, RTXAGC_A_NSS1INDEX7_NSS1INDEX4, MASKBYTE2, power_index);
			break;
		case MGN_VHT1SS_MCS7: 
			rtl_set_bbreg(rtlpriv, RTXAGC_A_NSS1INDEX7_NSS1INDEX4, MASKBYTE3, power_index);
			break;
		
		case MGN_VHT1SS_MCS8: 
			rtl_set_bbreg(rtlpriv, RTXAGC_A_NSS2INDEX1_NSS1INDEX8, MASKBYTE0, power_index);
			break;
		case MGN_VHT1SS_MCS9: 
			rtl_set_bbreg(rtlpriv, RTXAGC_A_NSS2INDEX1_NSS1INDEX8, MASKBYTE1, power_index);
			break;
		case MGN_VHT2SS_MCS0: 
			rtl_set_bbreg(rtlpriv, RTXAGC_A_NSS2INDEX1_NSS1INDEX8, MASKBYTE2, power_index);
			break;
		case MGN_VHT2SS_MCS1: 
			rtl_set_bbreg(rtlpriv, RTXAGC_A_NSS2INDEX1_NSS1INDEX8, MASKBYTE3, power_index);
			break;
		
		case MGN_VHT2SS_MCS2: 
			rtl_set_bbreg(rtlpriv, RTXAGC_A_NSS2INDEX5_NSS2INDEX2, MASKBYTE0, power_index);
			break;
		case MGN_VHT2SS_MCS3: 
			rtl_set_bbreg(rtlpriv, RTXAGC_A_NSS2INDEX5_NSS2INDEX2, MASKBYTE1, power_index);
			break;
		case MGN_VHT2SS_MCS4: 
			rtl_set_bbreg(rtlpriv, RTXAGC_A_NSS2INDEX5_NSS2INDEX2, MASKBYTE2, power_index);
			break;
		case MGN_VHT2SS_MCS5: 
			rtl_set_bbreg(rtlpriv, RTXAGC_A_NSS2INDEX5_NSS2INDEX2, MASKBYTE3, power_index);
			break;
		
		case MGN_VHT2SS_MCS6: 
			rtl_set_bbreg(rtlpriv, RTXAGC_A_NSS2INDEX9_NSS2INDEX6, MASKBYTE0, power_index);
			break;
		case MGN_VHT2SS_MCS7: 
			rtl_set_bbreg(rtlpriv, RTXAGC_A_NSS2INDEX9_NSS2INDEX6, MASKBYTE1, power_index);
			break;
		case MGN_VHT2SS_MCS8: 
			rtl_set_bbreg(rtlpriv, RTXAGC_A_NSS2INDEX9_NSS2INDEX6, MASKBYTE2, power_index);
			break;
		case MGN_VHT2SS_MCS9: 
			rtl_set_bbreg(rtlpriv, RTXAGC_A_NSS2INDEX9_NSS2INDEX6, MASKBYTE3, power_index);
			break;
	
		default:
			DBG_871X("Invalid Rate!!\n");
			break;
		}
	} else if (path == RF90_PATH_B) {
		switch (rate) {
		case MGN_1M:    
			rtl_set_bbreg(rtlpriv, RTXAGC_B_CCK11_CCK1, MASKBYTE0, power_index); 
			break;
		case MGN_2M:    
			rtl_set_bbreg(rtlpriv, RTXAGC_B_CCK11_CCK1, MASKBYTE1, power_index); 
			break;
		case MGN_5_5M:  
			rtl_set_bbreg(rtlpriv, RTXAGC_B_CCK11_CCK1, MASKBYTE2, power_index); 
			break;
		case MGN_11M:   
			rtl_set_bbreg(rtlpriv, RTXAGC_B_CCK11_CCK1, MASKBYTE3, power_index); 
			break;
		
		case MGN_6M:    
			rtl_set_bbreg(rtlpriv, RTXAGC_B_OFDM18_OFDM6, MASKBYTE0, power_index);
			break;
		case MGN_9M:    
			rtl_set_bbreg(rtlpriv, RTXAGC_B_OFDM18_OFDM6, MASKBYTE1, power_index);
			break;
		case MGN_12M:   
			rtl_set_bbreg(rtlpriv, RTXAGC_B_OFDM18_OFDM6, MASKBYTE2, power_index);
			break;
		case MGN_18M:   
			rtl_set_bbreg(rtlpriv, RTXAGC_B_OFDM18_OFDM6, MASKBYTE3, power_index);
			break;
		
		case MGN_24M:   
			rtl_set_bbreg(rtlpriv, RTXAGC_B_OFDM54_OFDM24, MASKBYTE0, power_index); 
			break;
		case MGN_36M:   
			rtl_set_bbreg(rtlpriv, RTXAGC_B_OFDM54_OFDM24, MASKBYTE1, power_index); 
			break;
		case MGN_48M:   
			rtl_set_bbreg(rtlpriv, RTXAGC_B_OFDM54_OFDM24, MASKBYTE2, power_index); 
			break;
		case MGN_54M:   
			rtl_set_bbreg(rtlpriv, RTXAGC_B_OFDM54_OFDM24, MASKBYTE3, power_index); 
			break;
		
		case MGN_MCS0:  
			rtl_set_bbreg(rtlpriv, RTXAGC_B_MCS03_MCS00, MASKBYTE0, power_index);
			break;
		case MGN_MCS1:  
			rtl_set_bbreg(rtlpriv, RTXAGC_B_MCS03_MCS00, MASKBYTE1, power_index);
			break;
		case MGN_MCS2:  
			rtl_set_bbreg(rtlpriv, RTXAGC_B_MCS03_MCS00, MASKBYTE2, power_index);
			break;
		case MGN_MCS3:  
			rtl_set_bbreg(rtlpriv, RTXAGC_B_MCS03_MCS00, MASKBYTE3, power_index);
			break;
		
		case MGN_MCS4:  
			rtl_set_bbreg(rtlpriv, RTXAGC_B_MCS07_MCS04, MASKBYTE0, power_index);
			break;
		case MGN_MCS5:  
			rtl_set_bbreg(rtlpriv, RTXAGC_B_MCS07_MCS04, MASKBYTE1, power_index);
			break;
		case MGN_MCS6:  
			rtl_set_bbreg(rtlpriv, RTXAGC_B_MCS07_MCS04, MASKBYTE2, power_index);
			break;
		case MGN_MCS7:  
			rtl_set_bbreg(rtlpriv, RTXAGC_B_MCS07_MCS04, MASKBYTE3, power_index);
			break;
		
		case MGN_MCS8:  
			rtl_set_bbreg(rtlpriv, RTXAGC_B_MCS11_MCS08, MASKBYTE0, power_index);
			break;
		case MGN_MCS9:  
			rtl_set_bbreg(rtlpriv, RTXAGC_B_MCS11_MCS08, MASKBYTE1, power_index);
			break;
		case MGN_MCS10: 
			rtl_set_bbreg(rtlpriv, RTXAGC_B_MCS11_MCS08, MASKBYTE2, power_index);
			break;
		case MGN_MCS11: 
			rtl_set_bbreg(rtlpriv, RTXAGC_B_MCS11_MCS08, MASKBYTE3, power_index);
			break;
		
		case MGN_MCS12: 
			rtl_set_bbreg(rtlpriv, RTXAGC_B_MCS15_MCS12, MASKBYTE0, power_index);
			break;
		case MGN_MCS13: 
			rtl_set_bbreg(rtlpriv, RTXAGC_B_MCS15_MCS12, MASKBYTE1, power_index);
			break;
		case MGN_MCS14: 
			rtl_set_bbreg(rtlpriv, RTXAGC_B_MCS15_MCS12, MASKBYTE2, power_index);
			break;
		case MGN_MCS15:
			rtl_set_bbreg(rtlpriv, RTXAGC_B_MCS15_MCS12, MASKBYTE3, power_index);
			break;
		
		case MGN_VHT1SS_MCS0: 
			rtl_set_bbreg(rtlpriv, RTXAGC_B_NSS1INDEX3_NSS1INDEX0, MASKBYTE0, power_index); 
			break;
		case MGN_VHT1SS_MCS1: 
			rtl_set_bbreg(rtlpriv, RTXAGC_B_NSS1INDEX3_NSS1INDEX0, MASKBYTE1, power_index); 
			break;
		case MGN_VHT1SS_MCS2: 
			rtl_set_bbreg(rtlpriv, RTXAGC_B_NSS1INDEX3_NSS1INDEX0, MASKBYTE2, power_index); 
			break;
		case MGN_VHT1SS_MCS3: 
			rtl_set_bbreg(rtlpriv, RTXAGC_B_NSS1INDEX3_NSS1INDEX0, MASKBYTE3, power_index); 
			break;
		
		case MGN_VHT1SS_MCS4: 
			rtl_set_bbreg(rtlpriv, RTXAGC_B_NSS1INDEX7_NSS1INDEX4, MASKBYTE0, power_index);
			break;
		case MGN_VHT1SS_MCS5: 
			rtl_set_bbreg(rtlpriv, RTXAGC_B_NSS1INDEX7_NSS1INDEX4, MASKBYTE1, power_index);
			break;
		case MGN_VHT1SS_MCS6: 
			rtl_set_bbreg(rtlpriv, RTXAGC_B_NSS1INDEX7_NSS1INDEX4, MASKBYTE2, power_index);
			break;
		case MGN_VHT1SS_MCS7: 
			rtl_set_bbreg(rtlpriv, RTXAGC_B_NSS1INDEX7_NSS1INDEX4, MASKBYTE3, power_index);
			break;
		
		case MGN_VHT1SS_MCS8: 
			rtl_set_bbreg(rtlpriv, RTXAGC_B_NSS2INDEX1_NSS1INDEX8, MASKBYTE0, power_index);
			break;
		case MGN_VHT1SS_MCS9: 
			rtl_set_bbreg(rtlpriv, RTXAGC_B_NSS2INDEX1_NSS1INDEX8, MASKBYTE1, power_index);
			break;
		case MGN_VHT2SS_MCS0: 
			rtl_set_bbreg(rtlpriv, RTXAGC_B_NSS2INDEX1_NSS1INDEX8, MASKBYTE2, power_index);
			break;
		case MGN_VHT2SS_MCS1: 
			rtl_set_bbreg(rtlpriv, RTXAGC_B_NSS2INDEX1_NSS1INDEX8, MASKBYTE3, power_index);
			break;
		
		case MGN_VHT2SS_MCS2: 
			rtl_set_bbreg(rtlpriv, RTXAGC_B_NSS2INDEX5_NSS2INDEX2, MASKBYTE0, power_index); 
			break;
		case MGN_VHT2SS_MCS3: 
			rtl_set_bbreg(rtlpriv, RTXAGC_B_NSS2INDEX5_NSS2INDEX2, MASKBYTE1, power_index); 
			break;
		case MGN_VHT2SS_MCS4: 
			rtl_set_bbreg(rtlpriv, RTXAGC_B_NSS2INDEX5_NSS2INDEX2, MASKBYTE2, power_index); 
			break;
		case MGN_VHT2SS_MCS5: 
			rtl_set_bbreg(rtlpriv, RTXAGC_B_NSS2INDEX5_NSS2INDEX2, MASKBYTE3, power_index); 
			break;
		
		case MGN_VHT2SS_MCS6: 
			rtl_set_bbreg(rtlpriv, RTXAGC_B_NSS2INDEX9_NSS2INDEX6, MASKBYTE0, power_index);
			break;
		case MGN_VHT2SS_MCS7: 
			rtl_set_bbreg(rtlpriv, RTXAGC_B_NSS2INDEX9_NSS2INDEX6, MASKBYTE1, power_index);
			break;
		case MGN_VHT2SS_MCS8: 
			rtl_set_bbreg(rtlpriv, RTXAGC_B_NSS2INDEX9_NSS2INDEX6, MASKBYTE2, power_index);
			break;
		case MGN_VHT2SS_MCS9: 
			rtl_set_bbreg(rtlpriv, RTXAGC_B_NSS2INDEX9_NSS2INDEX6, MASKBYTE3, power_index);
			break;
		
		    default:
		         DBG_871X("Invalid Rate!!\n");
		         break;
		}
	}
	else
	{
		DBG_871X("Invalid RFPath!!\n");
	}
}

static void _rtl8821au_phy_set_txpower_level_by_path(struct rtl_priv *rtlpriv, uint8_t RFPath,
	enum CHANNEL_WIDTH BandWidth, uint8_t Channel, uint8_t *Rates,
	uint8_t	RateArraySize)
{
	uint32_t power_index = 0;
	int	i = 0;

	for (i = 0; i < RateArraySize; ++i) {
		power_index = PHY_GetTxPowerIndex_8812A(rtlpriv, RFPath, Rates[i], BandWidth, Channel);
		_rtl8821au_phy_set_txpower_index(rtlpriv, power_index, RFPath, Rates[i]);
	}

}
static void _rtl8821au_phy_txpower_training_by_path(struct rtl_priv *rtlpriv, 
	enum CHANNEL_WIDTH BandWidth, uint8_t Channel, uint8_t RfPath)
{
	uint8_t	i;
	uint32_t	PowerLevel, writeData, writeOffset;

	if(RfPath >=  rtlpriv->phy.num_total_rfpath)
		return;

	writeData = 0;

	if (RfPath == RF90_PATH_A) {
		PowerLevel = PHY_GetTxPowerIndex_8812A(rtlpriv, RF90_PATH_A, MGN_MCS7, BandWidth, Channel);
		writeOffset =  rA_TxPwrTraing_Jaguar;
	} else {
		PowerLevel = PHY_GetTxPowerIndex_8812A(rtlpriv, RF90_PATH_B, MGN_MCS7, BandWidth, Channel);
		writeOffset =  rB_TxPwrTraing_Jaguar;
	}

	for (i = 0; i < 3; i++) {
		if(i == 0)
			PowerLevel = PowerLevel - 10;
		else if(i == 1)
			PowerLevel = PowerLevel - 8;
		else
			PowerLevel = PowerLevel - 6;
			
		writeData |= (((PowerLevel > 2)?(PowerLevel):2) << (i * 8));
	}

	rtl_set_bbreg(rtlpriv, writeOffset, 0xffffff, writeData);
}


static void rtl8821au_phy_set_txpower_level_by_path(struct rtl_priv *rtlpriv, 
	uint8_t	channel, uint8_t path)
{
	uint8_t	cckRates[]   = {MGN_1M, MGN_2M, MGN_5_5M, MGN_11M};
	uint8_t	ofdmRates[]  = {MGN_6M, MGN_9M, MGN_12M, MGN_18M, MGN_24M, MGN_36M, MGN_48M, MGN_54M};
	uint8_t	htRates1T[]  = {MGN_MCS0, MGN_MCS1, MGN_MCS2, MGN_MCS3, MGN_MCS4, MGN_MCS5, MGN_MCS6, MGN_MCS7};
	uint8_t	htRates2T[]  = {MGN_MCS8, MGN_MCS9, MGN_MCS10, MGN_MCS11, MGN_MCS12, MGN_MCS13, MGN_MCS14, MGN_MCS15};
	uint8_t	vhtRates1T[] = {MGN_VHT1SS_MCS0, MGN_VHT1SS_MCS1, MGN_VHT1SS_MCS2, MGN_VHT1SS_MCS3, MGN_VHT1SS_MCS4,
				MGN_VHT1SS_MCS5, MGN_VHT1SS_MCS6, MGN_VHT1SS_MCS7, MGN_VHT1SS_MCS8, MGN_VHT1SS_MCS9};
	uint8_t	vhtRates2T[] = {MGN_VHT2SS_MCS0, MGN_VHT2SS_MCS1, MGN_VHT2SS_MCS2, MGN_VHT2SS_MCS3, MGN_VHT2SS_MCS4,
				MGN_VHT2SS_MCS5, MGN_VHT2SS_MCS6, MGN_VHT2SS_MCS7, MGN_VHT2SS_MCS8, MGN_VHT2SS_MCS9};

	//DBG_871X("==>PHY_SetTxPowerLevelByPath8812()\n");

	//if(pMgntInfo->RegNByteAccess == 0)
	if(rtlpriv->rtlhal.current_bandtype == BAND_ON_2_4G)
		_rtl8821au_phy_set_txpower_level_by_path(rtlpriv, path, rtlpriv->phy.current_chan_bw, channel,
								  cckRates, sizeof(cckRates)/sizeof(u8));

	_rtl8821au_phy_set_txpower_level_by_path(rtlpriv, path, rtlpriv->phy.current_chan_bw, channel,
								  ofdmRates, sizeof(ofdmRates)/sizeof(u8));
	_rtl8821au_phy_set_txpower_level_by_path(rtlpriv, path, rtlpriv->phy.current_chan_bw, channel,
								  htRates1T, sizeof(htRates1T)/sizeof(u8));
	_rtl8821au_phy_set_txpower_level_by_path(rtlpriv, path, rtlpriv->phy.current_chan_bw, channel,
							  	  vhtRates1T, sizeof(vhtRates1T)/sizeof(u8));

	if ( rtlpriv->phy.num_total_rfpath >= 2) {
		_rtl8821au_phy_set_txpower_level_by_path(rtlpriv, path, rtlpriv->phy.current_chan_bw, channel,
							  htRates2T, sizeof(htRates2T)/sizeof(u8));
		_rtl8821au_phy_set_txpower_level_by_path(rtlpriv, path, rtlpriv->phy.current_chan_bw, channel,
							  vhtRates2T, sizeof(vhtRates2T)/sizeof(u8));
	}

	_rtl8821au_phy_txpower_training_by_path(rtlpriv, rtlpriv->phy.current_chan_bw, channel, path);

	/* DBG_871X("<==PHY_SetTxPowerLevelByPath8812()\n"); */
}



void PHY_SetTxPowerLevel8812(struct rtl_priv *rtlpriv, uint8_t	Channel)
{
	uint8_t	path = 0;

	/* DBG_871X("==>PHY_SetTxPowerLevel8812()\n"); */

	for (path = RF90_PATH_A; path < rtlpriv->phy.num_total_rfpath; ++path) {
		rtl8821au_phy_set_txpower_level_by_path(rtlpriv, Channel, path);
	}

	/* DBG_871X("<==PHY_SetTxPowerLevel8812()\n"); */
}
