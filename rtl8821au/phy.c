#include <rtl8812a_hal.h>
#include "phy.h"
#include "table.h"
/*
 * 1. BB register R/W API
 */

#define READ_NEXT_PAIR(array_table, v1, v2, i) \
	do { \
		i += 2; \
		v1 = array_table[i]; \
		v2 = array_table[i+1]; \
	} while (0)


u32 rtl8821au_phy_query_bb_reg(struct rtl_priv *Adapter, uint32_t RegAddr, uint32_t BitMask)
{
	uint32_t ReturnValue = 0, OriginalValue, BitShift;

	/* DBG_871X("--->PHY_QueryBBReg8812(): RegAddr(%#x), BitMask(%#x)\n", RegAddr, BitMask); */


	OriginalValue = rtw_read32(Adapter, RegAddr);
	BitShift = PHY_CalculateBitShift(BitMask);
	ReturnValue = (OriginalValue & BitMask) >> BitShift;

	/* DBG_871X("BBR MASK=0x%x Addr[0x%x]=0x%x\n", BitMask, RegAddr, OriginalValue); */
	return ReturnValue;
}


void rtl8821au_phy_set_bb_reg(struct rtl_priv *Adapter, u32 RegAddr, u32 BitMask, u32 Data)
{
	uint32_t OriginalValue, BitShift;

	if (BitMask != bMaskDWord) {	/* if not "double word" write */
		OriginalValue = rtw_read32(Adapter, RegAddr);
		BitShift = PHY_CalculateBitShift(BitMask);
		Data = ((OriginalValue) & (~BitMask)) | (((Data << BitShift)) & BitMask);
	}

	rtw_write32(Adapter, RegAddr, Data);

	/* DBG_871X("BBW MASK=0x%x Addr[0x%x]=0x%x\n", BitMask, RegAddr, Data); */
}
/* ****************************************************************************** */
/*									*/
/*  from HalPhyRf_8812A.c						*/
/*									*/
/* ****************************************************************************** */

static void _rtl8812au_iqk_rx_fill_iqc(struct rtl_dm *pDM_Odm, ODM_RF_RADIO_PATH_E Path,
	unsigned int RX_X, unsigned int RX_Y)
{
	switch (Path) {
	case ODM_RF_PATH_A:
		ODM_SetBBReg(pDM_Odm, 0x82c, BIT(31), 0x0); /* [31] = 0 --> Page C */
		if (RX_X>>1 == 0x112 || RX_Y>>1 == 0x3ee) {
			ODM_SetBBReg(pDM_Odm, 0xc10, 0x000003ff, 0x100);
			ODM_SetBBReg(pDM_Odm, 0xc10, 0x03ff0000, 0);
			ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("RX_X = %x;;RX_Y = %x ====>fill to IQC\n", RX_X>>1&0x000003ff, RX_Y>>1&0x000003ff));
		} else {
			ODM_SetBBReg(pDM_Odm, 0xc10, 0x000003ff, RX_X>>1);
			ODM_SetBBReg(pDM_Odm, 0xc10, 0x03ff0000, RX_Y>>1);
			ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("RX_X = %x;;RX_Y = %x ====>fill to IQC\n", RX_X>>1&0x000003ff, RX_Y>>1&0x000003ff));
			ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("0xc10 = %x ====>fill to IQC\n", ODM_Read4Byte(pDM_Odm, 0xc10)));
		}
		break;
	case ODM_RF_PATH_B:
			ODM_SetBBReg(pDM_Odm, 0x82c, BIT(31), 0x0); /*  [31] = 0 --> Page C */
			if (RX_X>>1 == 0x112 || RX_Y>>1 == 0x3ee) {
				ODM_SetBBReg(pDM_Odm, 0xe10, 0x000003ff, 0x100);
				ODM_SetBBReg(pDM_Odm, 0xe10, 0x03ff0000, 0);
				ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("RX_X = %x;;RX_Y = %x ====>fill to IQC\n", RX_X>>1&0x000003ff, RX_Y>>1&0x000003ff));
			} else {
				ODM_SetBBReg(pDM_Odm, 0xe10, 0x000003ff, RX_X>>1);
				ODM_SetBBReg(pDM_Odm, 0xe10, 0x03ff0000, RX_Y>>1);
				ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("RX_X = %x;;RX_Y = %x====>fill to IQC\n ", RX_X>>1&0x000003ff, RX_Y>>1&0x000003ff));
				ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("0xe10 = %x====>fill to IQC\n", ODM_Read4Byte(pDM_Odm, 0xe10)));
			}
		break;
	default:
		break;
	};
}

static void _rtl8821au_iqk_rx_fill_iqc(struct rtl_dm *pDM_Odm, ODM_RF_RADIO_PATH_E Path,
	unsigned int RX_X, unsigned int RX_Y)
{
	switch (Path) {
	case ODM_RF_PATH_A:
		ODM_SetBBReg(pDM_Odm, 0x82c, BIT(31), 0x0); /* [31] = 0 --> Page C */
		ODM_SetBBReg(pDM_Odm, 0xc10, 0x000003ff, RX_X>>1);
		ODM_SetBBReg(pDM_Odm, 0xc10, 0x03ff0000, RX_Y>>1);
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("RX_X = %x;;RX_Y = %x ====>fill to IQC\n", RX_X>>1, RX_Y>>1));
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("0xc10 = %x ====>fill to IQC\n", ODM_Read4Byte(pDM_Odm, 0xc10)));
		break;
	default:
		break;
	};
}

static void _rtl8812au_iqk_tx_fill_iqc(struct rtl_dm *pDM_Odm, ODM_RF_RADIO_PATH_E  Path,
	unsigned int TX_X, unsigned int TX_Y)
{
	switch (Path) {
	case ODM_RF_PATH_A:
		ODM_SetBBReg(pDM_Odm, 0x82c, BIT(31), 0x1); /* [31] = 1 --> Page C1 */
		ODM_Write4Byte(pDM_Odm, 0xc90, 0x00000080);
		ODM_Write4Byte(pDM_Odm, 0xcc4, 0x20040000);
		ODM_Write4Byte(pDM_Odm, 0xcc8, 0x20000000);
		ODM_SetBBReg(pDM_Odm, 0xccc, 0x000007ff, TX_Y);
		ODM_SetBBReg(pDM_Odm, 0xcd4, 0x000007ff, TX_X);
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("TX_X = %x;;TX_Y = %x =====> fill to IQC\n", TX_X&0x000007ff, TX_Y&0x000007ff));
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("0xcd4 = %x;;0xccc = %x ====>fill to IQC\n", ODM_GetBBReg(pDM_Odm, 0xcd4, 0x000007ff), ODM_GetBBReg(pDM_Odm, 0xccc, 0x000007ff)));
		break;
	case ODM_RF_PATH_B:
		ODM_SetBBReg(pDM_Odm, 0x82c, BIT(31), 0x1); /* [31] = 1 --> Page C1 */
		ODM_Write4Byte(pDM_Odm, 0xe90, 0x00000080);
		ODM_Write4Byte(pDM_Odm, 0xec4, 0x20040000);
		ODM_Write4Byte(pDM_Odm, 0xec8, 0x20000000);
		ODM_SetBBReg(pDM_Odm, 0xecc, 0x000007ff, TX_Y);
		ODM_SetBBReg(pDM_Odm, 0xed4, 0x000007ff, TX_X);
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("TX_X = %x;;TX_Y = %x =====> fill to IQC\n", TX_X&0x000007ff, TX_Y&0x000007ff));
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("0xed4 = %x;;0xecc = %x ====>fill to IQC\n", ODM_GetBBReg(pDM_Odm, 0xed4, 0x000007ff), ODM_GetBBReg(pDM_Odm, 0xecc, 0x000007ff)));
		break;
	default:
		break;
	};
}

static void _rtl8821au_iqk_tx_fill_iqc(struct rtl_dm *pDM_Odm, ODM_RF_RADIO_PATH_E Path,
	unsigned int TX_X, unsigned int TX_Y)
{
	switch (Path) {
	case ODM_RF_PATH_A:
		ODM_SetBBReg(pDM_Odm, 0x82c, BIT(31), 0x1); /* [31] = 1 --> Page C1 */
		ODM_Write4Byte(pDM_Odm, 0xc90, 0x00000080);
		ODM_Write4Byte(pDM_Odm, 0xcc4, 0x20040000);
		ODM_Write4Byte(pDM_Odm, 0xcc8, 0x20000000);
		ODM_SetBBReg(pDM_Odm, 0xccc, 0x000007ff, TX_Y);
		ODM_SetBBReg(pDM_Odm, 0xcd4, 0x000007ff, TX_X);
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("TX_X = %x;;TX_Y = %x =====> fill to IQC\n", TX_X, TX_Y));
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("0xcd4 = %x;;0xccc = %x ====>fill to IQC\n", ODM_GetBBReg(pDM_Odm, 0xcd4, 0x000007ff), ODM_GetBBReg(pDM_Odm, 0xccc, 0x000007ff)));
		break;
	default:
		break;
	};
}

#define cal_num 3

/* ULLI this function needs a complete rewrite (or we cantake code form rtlwifi-lib */

static void _rtl8812au_iqk_tx(struct rtl_dm *pDM_Odm, ODM_RF_RADIO_PATH_E Path)
{
	uint32_t 	TX_fail, RX_fail, delay_count, IQK_ready, cal_retry, cal = 0, temp_reg65;
	int		TX_X = 0, TX_Y = 0, RX_X = 0, RX_Y = 0, TX_Average = 0, RX_Average = 0;
	int 		TX_X0[cal_num], TX_Y0[cal_num], RX_X0[cal_num], RX_Y0[cal_num];
	BOOLEAN 	TX0IQKOK = FALSE, RX0IQKOK = FALSE;
	int 		TX_X1[cal_num], TX_Y1[cal_num], RX_X1[cal_num], RX_Y1[cal_num];
	BOOLEAN  	TX1IQKOK = FALSE, RX1IQKOK = FALSE, VDF_enable = FALSE;
	int 			i, k, VDF_Y[3], VDF_X[3], Tx_dt[3], Rx_dt[3], ii, dx = 0, dy = 0, TX_finish = 0, RX_finish = 0, dt = 0;
	PODM_RF_CAL_T  pRFCalibrateInfo = &(pDM_Odm->RFCalibrateInfo);

	ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("BandWidth = %d, ExtPA5G = %d, ExtPA2G = %d\n", *pDM_Odm->pBandWidth, pDM_Odm->ExtPA5G, pDM_Odm->ExtPA));
	if (*pDM_Odm->pBandWidth == 2) {
		VDF_enable = TRUE;
	}
	VDF_enable = FALSE;
	temp_reg65 = ODM_GetRFReg(pDM_Odm, Path, 0x65, bMaskDWord);

	switch (Path) {
	case ODM_RF_PATH_A:
		ODM_SetBBReg(pDM_Odm, 0x82c, BIT(31), 0x0);	/* [31] = 0 --> Page C */
		/*  ========Path-A AFE all on======== */
		/* Port 0 DAC/ADC on */
		ODM_Write4Byte(pDM_Odm, 0xc60, 0x77777777);
		ODM_Write4Byte(pDM_Odm, 0xc64, 0x77777777);

		/* Port 1 DAC/ADC off */
		ODM_Write4Byte(pDM_Odm, 0xe60, 0x00000000);
		ODM_Write4Byte(pDM_Odm, 0xe64, 0x00000000);

		ODM_Write4Byte(pDM_Odm, 0xc68, 0x19791979);

		ODM_SetBBReg(pDM_Odm, 0xc00, 0xf, 0x4);		/* hardware 3-wire off */

		/* DAC/ADC sampling rate (160 MHz) */
		ODM_SetBBReg(pDM_Odm, 0xc5c, BIT(26)|BIT(25)|BIT(24), 0x7);
		ODM_SetBBReg(pDM_Odm, 0x8c4, BIT(30), 0x1);
		/* ODM_SetBBReg(pDM_Odm, 0xcb0, 0x00ff0000, 0x77); */
		/* ODM_SetBBReg(pDM_Odm, 0xcb4, 0x03000000, 0x0); */
		break;
	case ODM_RF_PATH_B:
		ODM_SetBBReg(pDM_Odm, 0x82c, BIT(31), 0x0); /* [31] = 0 --> Page C */
		/* ========Path-B AFE all on======== */
		/* Port 0 DAC/ADC off */
		ODM_Write4Byte(pDM_Odm, 0xc60, 0x00000000);
		ODM_Write4Byte(pDM_Odm, 0xc64, 0x00000000);

		/* Port 1 DAC/ADC on */
		ODM_Write4Byte(pDM_Odm, 0xe60, 0x77777777);
		ODM_Write4Byte(pDM_Odm, 0xe64, 0x77777777);

		ODM_Write4Byte(pDM_Odm, 0xe68, 0x19791979);

		ODM_SetBBReg(pDM_Odm, 0xe00, 0xf, 0x4);		/* hardware 3-wire off */

		/* DAC/ADC sampling rate (160 MHz) */
		ODM_SetBBReg(pDM_Odm, 0xe5c, BIT(26)|BIT(25)|BIT(24), 0x7);
		ODM_SetBBReg(pDM_Odm, 0x8c4, BIT(30), 0x1);
		/* ODM_SetBBReg(pDM_Odm, 0xeb0, 0x00ff0000, 0x77); */
		/* ODM_SetBBReg(pDM_Odm, 0xeb4, 0x03000000, 0x0); */
		break;
	default:
		break;
	}

	switch (Path) {
	case ODM_RF_PATH_A:
	    {
		/* ====== TX IQK ====== */
		ODM_SetBBReg(pDM_Odm, 0x82c, BIT(31), 0x0); /*  [31] = 0 --> Page C */
		ODM_SetRFReg(pDM_Odm, Path, 0xef, bRFRegOffsetMask, 0x80002);
		ODM_SetRFReg(pDM_Odm, Path, 0x30, bRFRegOffsetMask, 0x20000);
		ODM_SetRFReg(pDM_Odm, Path, 0x31, bRFRegOffsetMask, 0x3fffd);
		ODM_SetRFReg(pDM_Odm, Path, 0x32, bRFRegOffsetMask, 0xfe83f);
		ODM_SetRFReg(pDM_Odm, Path, 0x65, bRFRegOffsetMask, 0x931d5);
		ODM_SetRFReg(pDM_Odm, Path, 0x8f, bRFRegOffsetMask, 0x8a001);
		ODM_Write4Byte(pDM_Odm, 0x90c, 0x00008000);
		ODM_Write4Byte(pDM_Odm, 0xb00, 0x03000100);
		ODM_SetBBReg(pDM_Odm, 0xc94, BIT(0), 0x1);
		ODM_Write4Byte(pDM_Odm, 0x978, 0x29002000);	/* TX (X,Y) */
		ODM_Write4Byte(pDM_Odm, 0x97c, 0xa9002000);	/* RX (X,Y) */
		ODM_Write4Byte(pDM_Odm, 0x984, 0x00462910);	/* [0]:AGC_en, [15]:idac_K_Mask */

		ODM_SetBBReg(pDM_Odm, 0x82c, BIT(31), 0x1);	/* [31] = 1 --> Page C1 */

		if (pDM_Odm->ExtPA5G)
			ODM_Write4Byte(pDM_Odm, 0xc88, 0x821403f7);
		else
			ODM_Write4Byte(pDM_Odm, 0xc88, 0x821403f1);

		if (*pDM_Odm->pBandType)
			ODM_Write4Byte(pDM_Odm, 0xc8c, 0x68163e96);
		else {
			ODM_Write4Byte(pDM_Odm, 0xc8c, 0x28163e96);
			if (pDM_Odm->RFEType == 3) {
				if (pDM_Odm->ExtPA)
					ODM_Write4Byte(pDM_Odm, 0xc88, 0x821403e3);
				else
					ODM_Write4Byte(pDM_Odm, 0xc88, 0x821403f7);
			}

		}

		if (VDF_enable == 1) {
			ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("TXVDF Start\n"));
			for (k = 0; k <= 2; k++) {
				switch (k) {
				case 0:
					ODM_Write4Byte(pDM_Odm, 0xc80, 0x18008c38);	/* TX_Tone_idx[9:0], TxK_Mask[29] TX_Tone = 16 */
					ODM_Write4Byte(pDM_Odm, 0xc84, 0x38008c38);	/* RX_Tone_idx[9:0], RxK_Mask[29] */
					ODM_Write4Byte(pDM_Odm, 0x984, 0x00462910);	/* [0]:AGC_en, [15]:idac_K_Mask */
					ODM_SetBBReg(pDM_Odm, 0xce8, BIT(31), 0x0);
					break;
				case 1:
					ODM_SetBBReg(pDM_Odm, 0xc80, BIT(28), 0x0);
					ODM_SetBBReg(pDM_Odm, 0xc84, BIT(28), 0x0);
					ODM_Write4Byte(pDM_Odm, 0x984, 0x0046a910);	/* [0]:AGC_en, [15]:idac_K_Mask */
					break;
				case 2:
					ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("VDF_Y[1] = %x;;;VDF_Y[0] = %x\n", VDF_Y[1]>>21 & 0x00007ff, VDF_Y[0]>>21 & 0x00007ff));
					ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("VDF_X[1] = %x;;;VDF_X[0] = %x\n", VDF_X[1]>>21 & 0x00007ff, VDF_X[0]>>21 & 0x00007ff));
					Tx_dt[cal] = (VDF_Y[1]>>20)-(VDF_Y[0]>>20);
					ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("Tx_dt = %d\n", Tx_dt[cal]));
					Tx_dt[cal] = ((16*Tx_dt[cal])*10000/15708);
					Tx_dt[cal] = (Tx_dt[cal] >> 1) + (Tx_dt[cal] & BIT(0));
					ODM_Write4Byte(pDM_Odm, 0xc80, 0x18008c20);	/* TX_Tone_idx[9:0], TxK_Mask[29] TX_Tone = 16 */
					ODM_Write4Byte(pDM_Odm, 0xc84, 0x38008c20);	/* RX_Tone_idx[9:0], RxK_Mask[29] */
					ODM_SetBBReg(pDM_Odm, 0xce8, BIT(31), 0x1);
					ODM_SetBBReg(pDM_Odm, 0xce8, 0x3fff0000, Tx_dt[cal] & 0x00003fff);
					break;
				default:
					break;
				}
				ODM_Write4Byte(pDM_Odm, 0xcb8, 0x00100000);		/* cb8[20] ±N SI/PI ¨Ï¥ÎÅv¤Áµ¹ iqk_dpk module */
				cal_retry = 0;
				while (1) {
					/* one shot */
					ODM_Write4Byte(pDM_Odm, 0x980, 0xfa000000);
					ODM_Write4Byte(pDM_Odm, 0x980, 0xf8000000);

					mdelay(10); /* Delay 10ms */
					ODM_Write4Byte(pDM_Odm, 0xcb8, 0x00000000);
					delay_count = 0;
					while (1) {
						IQK_ready = ODM_GetBBReg(pDM_Odm, 0xd00, BIT(10));
						if ((IQK_ready) || (delay_count > 20)) {
							break;
						} else {
							mdelay(1);
							delay_count++;
						}
					}

					if (delay_count < 20) {							/* If 20ms No Result, then cal_retry++ */
						/* ============TXIQK Check============== */
						TX_fail = ODM_GetBBReg(pDM_Odm, 0xd00, BIT(12));

						if (~TX_fail) {
							ODM_Write4Byte(pDM_Odm, 0xcb8, 0x02000000);
							VDF_X[k] = ODM_GetBBReg(pDM_Odm, 0xd00, 0x07ff0000)<<21;
							ODM_Write4Byte(pDM_Odm, 0xcb8, 0x04000000);
							VDF_Y[k] = ODM_GetBBReg(pDM_Odm, 0xd00, 0x07ff0000)<<21;
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
			ODM_Write4Byte(pDM_Odm, 0xc80, 0x18008c10);	/* TX_Tone_idx[9:0], TxK_Mask[29] TX_Tone = 16 */
			ODM_Write4Byte(pDM_Odm, 0xc84, 0x38008c10);	/* RX_Tone_idx[9:0], RxK_Mask[29] */
			ODM_Write4Byte(pDM_Odm, 0xce8, 0x00000000);

			for (cal = 0; cal < cal_num; cal++) {
				cal_retry = 0;
				while (1) {
					/* one shot */
					ODM_Write4Byte(pDM_Odm, 0xcb8, 0x00100000);	/* cb8[20] ±N SI/PI ¨Ï¥ÎÅv¤Áµ¹ iqk_dpk module */
					ODM_Write4Byte(pDM_Odm, 0x980, 0xfa000000);
					ODM_Write4Byte(pDM_Odm, 0x980, 0xf8000000);

					mdelay(10); 				/* Delay 25ms */
					ODM_Write4Byte(pDM_Odm, 0xcb8, 0x00000000);
					delay_count = 0;
					while (1) {
						IQK_ready = ODM_GetBBReg(pDM_Odm, 0xd00, BIT(10));
						if ((IQK_ready) || (delay_count > 20)) {
							break;
						} else {
							mdelay(1);
							delay_count++;
						}
					}

					if (delay_count < 20) {				/* If 20ms No Result, then cal_retry++ */
						/* ============TXIQK Check============== */
						TX_fail = ODM_GetBBReg(pDM_Odm, 0xd00, BIT(12));

						if (~TX_fail) {
							ODM_Write4Byte(pDM_Odm, 0xcb8, 0x02000000);
							TX_X0[cal] = ODM_GetBBReg(pDM_Odm, 0xd00, 0x07ff0000)<<21;
							ODM_Write4Byte(pDM_Odm, 0xcb8, 0x04000000);
							TX_Y0[cal] = ODM_GetBBReg(pDM_Odm, 0xd00, 0x07ff0000)<<21;
							TX0IQKOK = TRUE;
							/*
							ODM_Write4Byte(pDM_Odm, 0xcb8, 0x01000000);
							reg1 = ODM_GetBBReg(pDM_Odm, 0xd00, 0xffffffff);
							ODM_Write4Byte(pDM_Odm, 0xcb8, 0x02000000);
							reg2 = ODM_GetBBReg(pDM_Odm, 0xd00, 0x0000001f);
							Image_Power = (reg2<<32)+reg1;
							DbgPrint("Before PW = %d\n", Image_Power);
							ODM_Write4Byte(pDM_Odm, 0xcb8, 0x03000000);
							reg1 = ODM_GetBBReg(pDM_Odm, 0xd00, 0xffffffff);
							ODM_Write4Byte(pDM_Odm, 0xcb8, 0x04000000);
							reg2 = ODM_GetBBReg(pDM_Odm, 0xd00, 0x0000001f);
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

		ODM_SetBBReg(pDM_Odm, 0x82c, BIT(31), 0x0); /* [31] = 0 --> Page C */
		ODM_SetRFReg(pDM_Odm, Path, 0x58, 0x7fe00, ODM_GetRFReg(pDM_Odm, Path, 0x8, 0xffc00)); /* Load LOK */
		ODM_SetBBReg(pDM_Odm, 0x82c, BIT(31), 0x1); /* [31] = 1 --> Page C1 */

		if (TX0IQKOK == FALSE)
			break;				/* TXK fail, Don't do RXK */

		if (VDF_enable == 1) {
			ODM_SetBBReg(pDM_Odm, 0xce8, BIT(31), 0x0);    /*  TX VDF Disable */
			ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("RXVDF Start\n"));

			/* ====== RX IQK ====== */
			ODM_SetBBReg(pDM_Odm, 0x82c, BIT(31), 0x0); /* [31] = 0 --> Page C */
			ODM_SetRFReg(pDM_Odm, Path, 0xef, bRFRegOffsetMask, 0x80000);
			ODM_SetRFReg(pDM_Odm, Path, 0x30, bRFRegOffsetMask, 0x30000);
			ODM_SetRFReg(pDM_Odm, Path, 0x31, bRFRegOffsetMask, 0x3f7ff);
			ODM_SetRFReg(pDM_Odm, Path, 0x32, bRFRegOffsetMask, 0xfe7bf);
			ODM_SetRFReg(pDM_Odm, Path, 0x8f, bRFRegOffsetMask, 0x88001);
			ODM_SetRFReg(pDM_Odm, Path, 0x65, bRFRegOffsetMask, 0x931d0);
			ODM_SetRFReg(pDM_Odm, Path, 0xef, bRFRegOffsetMask, 0x00000);
			ODM_SetBBReg(pDM_Odm, 0x978, BIT(31), 0x1);
			ODM_SetBBReg(pDM_Odm, 0x97c, BIT(31), 0x0);
			ODM_Write4Byte(pDM_Odm, 0x984, 0x0046a911);

			ODM_SetBBReg(pDM_Odm, 0x82c, BIT(31), 0x1); /* [31] = 1 --> Page C1 */
			ODM_Write4Byte(pDM_Odm, 0xc88, 0x02140119);
			ODM_Write4Byte(pDM_Odm, 0xc8c, 0x28161420);

			for (k = 0; k <= 2; k++) {
				ODM_SetBBReg(pDM_Odm, 0x82c, BIT(31), 0x0); 	/* [31] = 0 --> Page C */
				ODM_SetBBReg(pDM_Odm, 0x978, 0x03FF8000, (VDF_X[k])>>21&0x000007ff);
				ODM_SetBBReg(pDM_Odm, 0x978, 0x000007FF, (VDF_Y[k])>>21&0x000007ff);

				ODM_SetBBReg(pDM_Odm, 0x82c, BIT(31), 0x1); 	/* [31] = 1 --> Page C1 */
				switch (k) {
				case 0:
					ODM_Write4Byte(pDM_Odm, 0xc80, 0x38008c38);	/* TX_Tone_idx[9:0], TxK_Mask[29] TX_Tone = 16 */
					ODM_Write4Byte(pDM_Odm, 0xc84, 0x18008c38);	/* RX_Tone_idx[9:0], RxK_Mask[29] */
					ODM_SetBBReg(pDM_Odm, 0xce8, BIT(30), 0x0);
					break;
				case 1:
					ODM_Write4Byte(pDM_Odm, 0xc80, 0x28008c38);	/* TX_Tone_idx[9:0], TxK_Mask[29] TX_Tone = 16 */
					ODM_Write4Byte(pDM_Odm, 0xc84, 0x08008c38);	/* RX_Tone_idx[9:0], RxK_Mask[29] */
					break;
				case 2:
					ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("VDF_Y[1] = %x;;;VDF_Y[0] = %x\n", VDF_Y[1]>>21 & 0x00007ff, VDF_Y[0]>>21 & 0x00007ff));
					ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("VDF_X[1] = %x;;;VDF_X[0] = %x\n", VDF_X[1]>>21 & 0x00007ff, VDF_X[0]>>21 & 0x00007ff));
					Rx_dt[cal] = (VDF_Y[1]>>20)-(VDF_Y[0]>>20);
					ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("Rx_dt = %d\n", Rx_dt[cal]));
					Rx_dt[cal] = ((16*Rx_dt[cal])*10000/13823);
					Rx_dt[cal] = (Rx_dt[cal] >> 1) + (Rx_dt[cal] & BIT(0));
					ODM_Write4Byte(pDM_Odm, 0xc80, 0x38008c20);	/* TX_Tone_idx[9:0], TxK_Mask[29] TX_Tone = 16 */
					ODM_Write4Byte(pDM_Odm, 0xc84, 0x18008c20);	/* RX_Tone_idx[9:0], RxK_Mask[29] */
					ODM_SetBBReg(pDM_Odm, 0xce8, 0x00003fff, Rx_dt[cal] & 0x00003fff);
					break;
				default:
					break;
				}


				if (k == 2) {
					ODM_SetBBReg(pDM_Odm, 0xce8, BIT(30), 0x1);  /* RX VDF Enable */
				}
				ODM_Write4Byte(pDM_Odm, 0xcb8, 0x00100000);/* cb8[20] ±N SI/PI ¨Ï¥ÎÅv¤Áµ¹ iqk_dpk module */

				cal_retry = 0;
				while (1) {
					/* one shot */
					ODM_Write4Byte(pDM_Odm, 0x980, 0xfa000000);
					ODM_Write4Byte(pDM_Odm, 0x980, 0xf8000000);

					mdelay(10); /* Delay 10ms */
					ODM_Write4Byte(pDM_Odm, 0xcb8, 0x00000000);
					delay_count = 0;

					while (1) {
						IQK_ready = ODM_GetBBReg(pDM_Odm, 0xd00, BIT(10));
						if ((IQK_ready) || (delay_count > 20)) {
							break;
						} else {
							mdelay(1);
							delay_count++;
						}
					}

					if (delay_count < 20) {	/* If 20ms No Result, then cal_retry++ */
						/* ============RXIQK Check============== */
						RX_fail = ODM_GetBBReg(pDM_Odm, 0xd00, BIT(11));
						if (RX_fail == 0) {
							ODM_Write4Byte(pDM_Odm, 0xcb8, 0x06000000);
							VDF_X[k] = ODM_GetBBReg(pDM_Odm, 0xd00, 0x07ff0000)<<21;
							ODM_Write4Byte(pDM_Odm, 0xcb8, 0x08000000);
							VDF_Y[k] = ODM_GetBBReg(pDM_Odm, 0xd00, 0x07ff0000)<<21;
							RX0IQKOK = TRUE;
							break;
						} else {
							ODM_SetBBReg(pDM_Odm, 0xc10, 0x000003ff, 0x200>>1);
							ODM_SetBBReg(pDM_Odm, 0xc10, 0x03ff0000, 0x0>>1);
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
			ODM_SetBBReg(pDM_Odm, 0xce8, BIT(31), 0x1);    /* TX VDF Enable */
		} else {
			/* ====== RX IQK ====== */
			ODM_SetBBReg(pDM_Odm, 0x82c, BIT(31), 0x0); /* [31] = 0 --> Page C */
			/*  1. RX RF Setting */
			ODM_SetRFReg(pDM_Odm, Path, 0xef, bRFRegOffsetMask, 0x80000);
			ODM_SetRFReg(pDM_Odm, Path, 0x30, bRFRegOffsetMask, 0x30000);
			ODM_SetRFReg(pDM_Odm, Path, 0x31, bRFRegOffsetMask, 0x3f7ff);
			ODM_SetRFReg(pDM_Odm, Path, 0x32, bRFRegOffsetMask, 0xfe7bf);
			ODM_SetRFReg(pDM_Odm, Path, 0x8f, bRFRegOffsetMask, 0x88001);
			ODM_SetRFReg(pDM_Odm, Path, 0x65, bRFRegOffsetMask, 0x931d0);
			ODM_SetRFReg(pDM_Odm, Path, 0xef, bRFRegOffsetMask, 0x00000);

			ODM_SetBBReg(pDM_Odm, 0x978, BIT(31), 0x1);
			ODM_SetBBReg(pDM_Odm, 0x97c, BIT(31), 0x0);
			ODM_Write4Byte(pDM_Odm, 0x90c, 0x00008000);
			/* ODM_Write4Byte(pDM_Odm, 0x984, 0x0046a911); */
			ODM_Write4Byte(pDM_Odm, 0x984, 0x0046a891);

			ODM_SetBBReg(pDM_Odm, 0x82c, BIT(31), 0x1); 	/* [31] = 1 --> Page C1 */
			ODM_Write4Byte(pDM_Odm, 0xc80, 0x38008c10);	/* TX_Tone_idx[9:0], TxK_Mask[29] TX_Tone = 16 */
			ODM_Write4Byte(pDM_Odm, 0xc84, 0x18008c10);	/* RX_Tone_idx[9:0], RxK_Mask[29] */
			ODM_Write4Byte(pDM_Odm, 0xc88, 0x02140119);
			ODM_Write4Byte(pDM_Odm, 0xc8c, 0x28160d40);

			for (cal = 0; cal < cal_num; cal++) {
				ODM_SetBBReg(pDM_Odm, 0x82c, BIT(31), 0x0);		/* [31] = 0 --> Page C */
				ODM_SetBBReg(pDM_Odm, 0x978, 0x03FF8000, (TX_X0[cal])>>21&0x000007ff);
				ODM_SetBBReg(pDM_Odm, 0x978, 0x000007FF, (TX_Y0[cal])>>21&0x000007ff);
				ODM_SetBBReg(pDM_Odm, 0x82c, BIT(31), 0x1);		/* [31] = 1 --> Page C1 */
				cal_retry = 0;
				while (1) {
					/*  one shot */
					ODM_Write4Byte(pDM_Odm, 0xcb8, 0x00100000);/* cb8[20] ±N SI/PI ¨Ï¥ÎÅv¤Áµ¹ iqk_dpk module */
					ODM_Write4Byte(pDM_Odm, 0x980, 0xfa000000);
					ODM_Write4Byte(pDM_Odm, 0x980, 0xf8000000);

					mdelay(10);			/* Delay 10ms */
					ODM_Write4Byte(pDM_Odm, 0xcb8, 0x00000000);
					delay_count = 0;
					while (1) {
						IQK_ready = ODM_GetBBReg(pDM_Odm, 0xd00, BIT(10));
						if ((IQK_ready) || (delay_count > 20)) {
							break;
						} else {
							mdelay(1);
							delay_count++;
						}
					}

				if (delay_count < 20) {	/* If 20ms No Result, then cal_retry++ */
					/* ============RXIQK Check============== */
					RX_fail = ODM_GetBBReg(pDM_Odm, 0xd00, BIT(11));
					if (RX_fail == 0) {
						ODM_Write4Byte(pDM_Odm, 0xcb8, 0x06000000);
						RX_X0[cal] = ODM_GetBBReg(pDM_Odm, 0xd00, 0x07ff0000)<<21;
						ODM_Write4Byte(pDM_Odm, 0xcb8, 0x08000000);
						RX_Y0[cal] = ODM_GetBBReg(pDM_Odm, 0xd00, 0x07ff0000)<<21;
						RX0IQKOK = TRUE;
						/*
						ODM_Write4Byte(pDM_Odm, 0xcb8, 0x05000000);
						reg1 = ODM_GetBBReg(pDM_Odm, 0xd00, 0xffffffff);
						ODM_Write4Byte(pDM_Odm, 0xcb8, 0x06000000);
						reg2 = ODM_GetBBReg(pDM_Odm, 0xd00, 0x0000001f);
						DbgPrint("reg1 = %d, reg2 = %d", reg1, reg2);
						Image_Power = (reg2<<32)+reg1;
						DbgPrint("Before PW = %d\n", Image_Power);
						ODM_Write4Byte(pDM_Odm, 0xcb8, 0x07000000);
						reg1 = ODM_GetBBReg(pDM_Odm, 0xd00, 0xffffffff);
						ODM_Write4Byte(pDM_Odm, 0xcb8, 0x08000000);
						reg2 = ODM_GetBBReg(pDM_Odm, 0xd00, 0x0000001f);
						Image_Power = (reg2<<32)+reg1;
						DbgPrint("After PW = %d\n", Image_Power);
						*/

						break;
					} else {
						ODM_SetBBReg(pDM_Odm, 0xc10, 0x000003ff, 0x200>>1);
						ODM_SetBBReg(pDM_Odm, 0xc10, 0x03ff0000, 0x0>>1);
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
	case ODM_RF_PATH_B:
	    {
		/* Path-B TX/RX IQK */
		ODM_SetBBReg(pDM_Odm, 0x82c, BIT(31), 0x0);		/* [31] = 0 --> Page C */
		ODM_SetRFReg(pDM_Odm, Path, 0xef, bRFRegOffsetMask, 0x80002);
		ODM_SetRFReg(pDM_Odm, Path, 0x30, bRFRegOffsetMask, 0x20000);
		ODM_SetRFReg(pDM_Odm, Path, 0x31, bRFRegOffsetMask, 0x3fffd);
		ODM_SetRFReg(pDM_Odm, Path, 0x32, bRFRegOffsetMask, 0xfe83f);
		ODM_SetRFReg(pDM_Odm, Path, 0x65, bRFRegOffsetMask, 0x931d5);
		ODM_SetRFReg(pDM_Odm, Path, 0x8f, bRFRegOffsetMask, 0x8a001);
		ODM_Write4Byte(pDM_Odm, 0x90c, 0x00008000);
		ODM_Write4Byte(pDM_Odm, 0xb00, 0x03000100);
		ODM_SetBBReg(pDM_Odm, 0xe94, BIT(0), 0x1);
		ODM_Write4Byte(pDM_Odm, 0x978, 0x29002000);		/* TX (X,Y) */
		ODM_Write4Byte(pDM_Odm, 0x97c, 0xa9002000);		/* RX (X,Y) */
		ODM_Write4Byte(pDM_Odm, 0x984, 0x00462910);		/* [0]:AGC_en, [15]:idac_K_Mask */

		ODM_SetBBReg(pDM_Odm, 0x82c, BIT(31), 0x1);		/* [31] = 1 --> Page C1 */

		if (pDM_Odm->ExtPA5G)
			ODM_Write4Byte(pDM_Odm, 0xe88, 0x821403f7);
		else
			ODM_Write4Byte(pDM_Odm, 0xe88, 0x821403f1);

		if (*pDM_Odm->pBandType)
			ODM_Write4Byte(pDM_Odm, 0xe8c, 0x68163e96);
		else
			ODM_Write4Byte(pDM_Odm, 0xe8c, 0x28163e96);

		if (VDF_enable == 1) {
			for (k = 0; k <= 2; k++) {
				switch (k) {
				case 0:
					ODM_Write4Byte(pDM_Odm, 0xe80, 0x18008c38);	/* TX_Tone_idx[9:0], TxK_Mask[29] TX_Tone = 16 */
					ODM_Write4Byte(pDM_Odm, 0xe84, 0x38008c38);	/* RX_Tone_idx[9:0], RxK_Mask[29] */
					ODM_Write4Byte(pDM_Odm, 0x984, 0x00462910);
					ODM_SetBBReg(pDM_Odm, 0xee8, BIT(31), 0x0);
					break;
				case 1:
					ODM_SetBBReg(pDM_Odm, 0xe80, BIT(28), 0x0);
					ODM_SetBBReg(pDM_Odm, 0xe84, BIT(28), 0x0);
					ODM_Write4Byte(pDM_Odm, 0x984, 0x0046a910);
					ODM_SetBBReg(pDM_Odm, 0xee8, BIT(31), 0x0);
					break;
				case 2:
					ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("VDF_Y[1] = %x;;;VDF_Y[0] = %x\n", VDF_Y[1]>>21 & 0x00007ff, VDF_Y[0]>>21 & 0x00007ff));
					ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("VDF_X[1] = %x;;;VDF_X[0] = %x\n", VDF_X[1]>>21 & 0x00007ff, VDF_X[0]>>21 & 0x00007ff));
					Tx_dt[cal] = (VDF_Y[1]>>20)-(VDF_Y[0]>>20);
					ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("Tx_dt = %d\n", Tx_dt[cal]));
					Tx_dt[cal] = ((16*Tx_dt[cal])*10000/15708);
					Tx_dt[cal] = (Tx_dt[cal] >> 1) + (Tx_dt[cal] & BIT(0));
					ODM_Write4Byte(pDM_Odm, 0xe80, 0x18008c20);	/* TX_Tone_idx[9:0], TxK_Mask[29] TX_Tone = 16 */
					ODM_Write4Byte(pDM_Odm, 0xe84, 0x38008c20);	/* RX_Tone_idx[9:0], RxK_Mask[29] */
					ODM_SetBBReg(pDM_Odm, 0xee8, BIT(31), 0x1);
					ODM_SetBBReg(pDM_Odm, 0xee8, 0x3fff0000, Tx_dt[cal] & 0x00003fff);
					break;
				default:
					break;
				}


				ODM_Write4Byte(pDM_Odm, 0xeb8, 0x00100000);/* cb8[20] ±N SI/PI ¨Ï¥ÎÅv¤Áµ¹ iqk_dpk module */
				cal_retry = 0;
				while (1) {
					/*  one shot */
					ODM_Write4Byte(pDM_Odm, 0x980, 0xfa000000);
					ODM_Write4Byte(pDM_Odm, 0x980, 0xf8000000);

					mdelay(10); /* Delay 10ms */
					ODM_Write4Byte(pDM_Odm, 0xeb8, 0x00000000);
					delay_count = 0;
					while (1) {
						IQK_ready = ODM_GetBBReg(pDM_Odm, 0xd40, BIT(10));
						if ((IQK_ready) || (delay_count > 20)) {
							break;
						} else {
							mdelay(1);
							delay_count++;
						}
					}

					if (delay_count < 20) {		/* If 20ms No Result, then cal_retry++ */
						/* ============TXIQK Check============== */
						TX_fail = ODM_GetBBReg(pDM_Odm, 0xd40, BIT(12));

						if (~TX_fail) {
							ODM_Write4Byte(pDM_Odm, 0xeb8, 0x02000000);
							VDF_X[k] = ODM_GetBBReg(pDM_Odm, 0xd40, 0x07ff0000)<<21;
							ODM_Write4Byte(pDM_Odm, 0xeb8, 0x04000000);
							VDF_Y[k] = ODM_GetBBReg(pDM_Odm, 0xd40, 0x07ff0000)<<21;
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
			ODM_Write4Byte(pDM_Odm, 0xe80, 0x18008c10);	/* TX_Tone_idx[9:0], TxK_Mask[29] TX_Tone = 16 */
			ODM_Write4Byte(pDM_Odm, 0xe84, 0x38008c10);	/* RX_Tone_idx[9:0], RxK_Mask[29] */
			ODM_Write4Byte(pDM_Odm, 0xee8, 0x00000000);

			for (cal = 0; cal < cal_num; cal++) {
				cal_retry = 0;

				while (1) {
					/* one shot */
					ODM_Write4Byte(pDM_Odm, 0xeb8, 0x00100000);/*  cb8[20] ±N SI/PI ¨Ï¥ÎÅv¤Áµ¹ iqk_dpk module */
					ODM_Write4Byte(pDM_Odm, 0x980, 0xfa000000);
					ODM_Write4Byte(pDM_Odm, 0x980, 0xf8000000);

					mdelay(10); /* Delay 25ms */
					ODM_Write4Byte(pDM_Odm, 0xeb8, 0x00000000);
					delay_count = 0;
					while (1) {
						IQK_ready = ODM_GetBBReg(pDM_Odm, 0xd40, BIT(10));
						if ((IQK_ready) || (delay_count > 20)) {
							break;
						} else {
							mdelay(1);
							delay_count++;
						}
					}

					if (delay_count < 20) {							/* If 20ms No Result, then cal_retry++ */
						/* ============TXIQK Check============== */
						TX_fail = ODM_GetBBReg(pDM_Odm, 0xd40, BIT(12));
						if (~TX_fail) {
							ODM_Write4Byte(pDM_Odm, 0xeb8, 0x02000000);
							TX_X1[cal] = ODM_GetBBReg(pDM_Odm, 0xd40, 0x07ff0000)<<21;
							ODM_Write4Byte(pDM_Odm, 0xeb8, 0x04000000);
							TX_Y1[cal] = ODM_GetBBReg(pDM_Odm, 0xd40, 0x07ff0000)<<21;
							TX1IQKOK = TRUE;
							/*
							int			reg1 = 0, reg2 = 0, Image_Power = 0;
							ODM_Write4Byte(pDM_Odm, 0xeb8, 0x01000000);
							reg1 = ODM_GetBBReg(pDM_Odm, 0xd40, 0xffffffff);
							ODM_Write4Byte(pDM_Odm, 0xeb8, 0x02000000);
							reg2 = ODM_GetBBReg(pDM_Odm, 0xd40, 0x0000001f);
							Image_Power = (reg2<<32)+reg1;
							DbgPrint("Before PW = %d\n", Image_Power);
							ODM_Write4Byte(pDM_Odm, 0xeb8, 0x03000000);
							reg1 = ODM_GetBBReg(pDM_Odm, 0xd40, 0xffffffff);
							ODM_Write4Byte(pDM_Odm, 0xeb8, 0x04000000);
							reg2 = ODM_GetBBReg(pDM_Odm, 0xd40, 0x0000001f);
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

		ODM_SetBBReg(pDM_Odm, 0x82c, BIT(31), 0x0);		/* [31] = 0 --> Page C */
		ODM_SetRFReg(pDM_Odm, Path, 0x58, 0x7fe00, ODM_GetRFReg(pDM_Odm, Path, 0x8, 0xffc00));	/* Load LOK */
		ODM_SetBBReg(pDM_Odm, 0x82c, BIT(31), 0x1);		/* [31] = 1 --> Page C1 */

		if (TX1IQKOK == FALSE)
			break;				/* TXK fail, Don't do RXK */

		if (VDF_enable == 1) {
			ODM_SetBBReg(pDM_Odm, 0xee8, BIT(31), 0x0);    /* TX VDF Disable */
			ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("RXVDF Start\n"));

			/* ====== RX IQK ====== */
			ODM_SetBBReg(pDM_Odm, 0x82c, BIT(31), 0x0);		/* [31] = 0 --> Page C */
			ODM_SetRFReg(pDM_Odm, Path, 0xef, bRFRegOffsetMask, 0x80000);
			ODM_SetRFReg(pDM_Odm, Path, 0x30, bRFRegOffsetMask, 0x30000);
			ODM_SetRFReg(pDM_Odm, Path, 0x31, bRFRegOffsetMask, 0x3f7ff);
			ODM_SetRFReg(pDM_Odm, Path, 0x32, bRFRegOffsetMask, 0xfe7bf);
			ODM_SetRFReg(pDM_Odm, Path, 0x8f, bRFRegOffsetMask, 0x88001);
			ODM_SetRFReg(pDM_Odm, Path, 0x65, bRFRegOffsetMask, 0x931d0);
			ODM_SetRFReg(pDM_Odm, Path, 0xef, bRFRegOffsetMask, 0x00000);

			ODM_SetBBReg(pDM_Odm, 0x978, BIT(31), 0x1);
			ODM_SetBBReg(pDM_Odm, 0x97c, BIT(31), 0x0);
			ODM_Write4Byte(pDM_Odm, 0x984, 0x0046a911);
			ODM_SetBBReg(pDM_Odm, 0x82c, BIT(31), 0x1);		/* [31] = 1 --> Page C1 */
			ODM_Write4Byte(pDM_Odm, 0xe88, 0x02140119);
			ODM_Write4Byte(pDM_Odm, 0xe8c, 0x28161420);

			for (k = 0; k <= 2; k++) {
				ODM_SetBBReg(pDM_Odm, 0x82c, BIT(31), 0x0);		/* [31] = 0 --> Page C */
				ODM_SetBBReg(pDM_Odm, 0x978, 0x03FF8000, (VDF_X[k])>>21&0x000007ff);
				ODM_SetBBReg(pDM_Odm, 0x978, 0x000007FF, (VDF_Y[k])>>21&0x000007ff);
				ODM_SetBBReg(pDM_Odm, 0x82c, BIT(31), 0x1);		/* [31] = 1 --> Page C1 */

				switch (k) {
				case 0:
					ODM_Write4Byte(pDM_Odm, 0xe80, 0x38008c38);	/* TX_Tone_idx[9:0], TxK_Mask[29] TX_Tone = 16 */
					ODM_Write4Byte(pDM_Odm, 0xe84, 0x18008c38);	/* RX_Tone_idx[9:0], RxK_Mask[29] */
					ODM_SetBBReg(pDM_Odm, 0xee8, BIT(30), 0x0);
					break;
				case 1:
					ODM_Write4Byte(pDM_Odm, 0xe80, 0x28008c38);	/* TX_Tone_idx[9:0], TxK_Mask[29] TX_Tone = 16 */
					ODM_Write4Byte(pDM_Odm, 0xe84, 0x08008c38);	/* RX_Tone_idx[9:0], RxK_Mask[29] */
					ODM_SetBBReg(pDM_Odm, 0xee8, BIT(30), 0x0);
					break;
				case 2:
					ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("VDF_Y[1] = %x;;;VDF_Y[0] = %x\n", VDF_Y[1]>>21 & 0x00007ff, VDF_Y[0]>>21 & 0x00007ff));
					ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("VDF_X[1] = %x;;;VDF_X[0] = %x\n", VDF_X[1]>>21 & 0x00007ff, VDF_X[0]>>21 & 0x00007ff));
					Rx_dt[cal] = (VDF_Y[1]>>20)-(VDF_Y[0]>>20);
					ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("Rx_dt = %d\n", Rx_dt[cal]));
					Rx_dt[cal] = ((16*Rx_dt[cal])*10000/13823);
					Rx_dt[cal] = (Rx_dt[cal] >> 1) + (Rx_dt[cal] & BIT(0));
					ODM_Write4Byte(pDM_Odm, 0xe80, 0x38008c20);	/* TX_Tone_idx[9:0], TxK_Mask[29] TX_Tone = 16 */
					ODM_Write4Byte(pDM_Odm, 0xe84, 0x18008c20);	/* RX_Tone_idx[9:0], RxK_Mask[29] */
					ODM_SetBBReg(pDM_Odm, 0xee8, 0x00003fff, Rx_dt[cal] & 0x00003fff);
					break;
				default:
					break;
				}


				if (k == 2) {
					ODM_SetBBReg(pDM_Odm, 0xee8, BIT(30), 0x1);	/* RX VDF Enable */
				}

				ODM_Write4Byte(pDM_Odm, 0xeb8, 0x00100000);		/* cb8[20] ±N SI/PI ¨Ï¥ÎÅv¤Áµ¹ iqk_dpk module */

				cal_retry = 0;

				while (1) {
					/* one shot */
					ODM_Write4Byte(pDM_Odm, 0x980, 0xfa000000);
					ODM_Write4Byte(pDM_Odm, 0x980, 0xf8000000);

					mdelay(10); /* Delay 10ms */
					ODM_Write4Byte(pDM_Odm, 0xeb8, 0x00000000);
					delay_count = 0;
					while (1) {
						IQK_ready = ODM_GetBBReg(pDM_Odm, 0xd40, BIT(10));
						if ((IQK_ready) || (delay_count > 20)) {
							break;
						} else {
							mdelay(1);
							delay_count++;
						}
					}

					if (delay_count < 20) {	/* If 20ms No Result, then cal_retry++ */
						/* ============RXIQK Check============== */
						RX_fail = ODM_GetBBReg(pDM_Odm, 0xd40, BIT(11));
						if (RX_fail == 0) {
							ODM_Write4Byte(pDM_Odm, 0xeb8, 0x06000000);
							VDF_X[k] = ODM_GetBBReg(pDM_Odm, 0xd40, 0x07ff0000)<<21;
							ODM_Write4Byte(pDM_Odm, 0xeb8, 0x08000000);
							VDF_Y[k] = ODM_GetBBReg(pDM_Odm, 0xd40, 0x07ff0000)<<21;
							RX1IQKOK = TRUE;
							break;
						} else {
							ODM_SetBBReg(pDM_Odm, 0xe10, 0x000003ff, 0x200>>1);
							ODM_SetBBReg(pDM_Odm, 0xe10, 0x03ff0000, 0x0>>1);
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
			ODM_SetBBReg(pDM_Odm, 0xee8, BIT(31), 0x1);	/* TX VDF Enable */
		} else {
			/* ====== RX IQK ====== */
			ODM_SetBBReg(pDM_Odm, 0x82c, BIT(31), 0x0); /* [31] = 0 --> Page C */
			ODM_SetRFReg(pDM_Odm, Path, 0xef, bRFRegOffsetMask, 0x80000);
			ODM_SetRFReg(pDM_Odm, Path, 0x30, bRFRegOffsetMask, 0x30000);
			ODM_SetRFReg(pDM_Odm, Path, 0x31, bRFRegOffsetMask, 0x3f7ff);
			ODM_SetRFReg(pDM_Odm, Path, 0x32, bRFRegOffsetMask, 0xfe7bf);
			ODM_SetRFReg(pDM_Odm, Path, 0x8f, bRFRegOffsetMask, 0x88001);
			ODM_SetRFReg(pDM_Odm, Path, 0x65, bRFRegOffsetMask, 0x931d0);
			ODM_SetRFReg(pDM_Odm, Path, 0xef, bRFRegOffsetMask, 0x00000);

			ODM_SetBBReg(pDM_Odm, 0x978, BIT(31), 0x1);
			ODM_SetBBReg(pDM_Odm, 0x97c, BIT(31), 0x0);
			ODM_Write4Byte(pDM_Odm, 0x90c, 0x00008000);
			/* ODM_Write4Byte(pDM_Odm, 0x984, 0x0046a911); */
			ODM_Write4Byte(pDM_Odm, 0x984, 0x0046a891);

			ODM_SetBBReg(pDM_Odm, 0x82c, BIT(31), 0x1);	/* [31] = 1 --> Page C1 */
			ODM_Write4Byte(pDM_Odm, 0xe80, 0x38008c10);	/* TX_Tone_idx[9:0], TxK_Mask[29] TX_Tone = 16 */
			ODM_Write4Byte(pDM_Odm, 0xe84, 0x18008c10);	/* RX_Tone_idx[9:0], RxK_Mask[29] */
			ODM_Write4Byte(pDM_Odm, 0xe88, 0x02140119);
			ODM_Write4Byte(pDM_Odm, 0xe8c, 0x28161180);

			for (cal = 0; cal < cal_num; cal++) {
				ODM_SetBBReg(pDM_Odm, 0x82c, BIT(31), 0x0);		/* [31] = 0 --> Page C */
				ODM_SetBBReg(pDM_Odm, 0x978, 0x03FF8000, (TX_X1[cal])>>21&0x000007ff);
				ODM_SetBBReg(pDM_Odm, 0x978, 0x000007FF, (TX_Y1[cal])>>21&0x000007ff);
				ODM_SetBBReg(pDM_Odm, 0x82c, BIT(31), 0x1);		/* [31] = 1 --> Page C1 */
				cal_retry = 0;

				while (1) {
					/* one shot */
					ODM_Write4Byte(pDM_Odm, 0xeb8, 0x00100000);	/* cb8[20] ±N SI/PI ¨Ï¥ÎÅv¤Áµ¹ iqk_dpk module */
					ODM_Write4Byte(pDM_Odm, 0x980, 0xfa000000);
					ODM_Write4Byte(pDM_Odm, 0x980, 0xf8000000);

					mdelay(10);	/*Delay 10ms */
					ODM_Write4Byte(pDM_Odm, 0xeb8, 0x00000000);
					delay_count = 0;

					while (1) {
						IQK_ready = ODM_GetBBReg(pDM_Odm, 0xd40, BIT(10));
						if ((IQK_ready) || (delay_count > 20)) {
							break;
						} else {
							mdelay(1);
							delay_count++;
						}
					}

					if (delay_count < 20) {				/* If 20ms No Result, then cal_retry++ */
						/* ============RXIQK Check============== */
						RX_fail = ODM_GetBBReg(pDM_Odm, 0xd40, BIT(11));
						if (RX_fail == 0) {
							ODM_Write4Byte(pDM_Odm, 0xeb8, 0x06000000);
							RX_X1[cal] = ODM_GetBBReg(pDM_Odm, 0xd40, 0x07ff0000)<<21;
							ODM_Write4Byte(pDM_Odm, 0xeb8, 0x08000000);
							RX_Y1[cal] = ODM_GetBBReg(pDM_Odm, 0xd40, 0x07ff0000)<<21;
							RX1IQKOK = TRUE;
							/*
							ODM_Write4Byte(pDM_Odm, 0xeb8, 0x05000000);
							reg1 = ODM_GetBBReg(pDM_Odm, 0xd40, 0xffffffff);
							ODM_Write4Byte(pDM_Odm, 0xeb8, 0x06000000);
							reg2 = ODM_GetBBReg(pDM_Odm, 0xd40, 0x0000001f);
							DbgPrint("reg1 = %d, reg2 = %d", reg1, reg2);
							Image_Power = (reg2<<32)+reg1;
							DbgPrint("Before PW = %d\n", Image_Power);
							ODM_Write4Byte(pDM_Odm, 0xeb8, 0x07000000);
							reg1 = ODM_GetBBReg(pDM_Odm, 0xd40, 0xffffffff);
							ODM_Write4Byte(pDM_Odm, 0xeb8, 0x08000000);
							reg2 = ODM_GetBBReg(pDM_Odm, 0xd40, 0x0000001f);
							Image_Power = (reg2<<32)+reg1;
							DbgPrint("After PW = %d\n", Image_Power);
							*/
							break;
						} else {
							ODM_SetBBReg(pDM_Odm, 0xe10, 0x000003ff, 0x200>>1);
							ODM_SetBBReg(pDM_Odm, 0xe10, 0x03ff0000, 0x0>>1);
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
	case ODM_RF_PATH_A:
	    {
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("========Path_A =======\n"));
		if (TX_Average == 0) {
			_rtl8812au_iqk_tx_fill_iqc(pDM_Odm, Path, 0x200, 0x0);
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
						if (*pDM_Odm->pBandWidth == 2) {
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

		if (*pDM_Odm->pBandWidth == 2) {
			ODM_SetBBReg(pDM_Odm, 0x82c, BIT(31), 0x1);		/* [31] = 0 --> Page C */
			ODM_SetBBReg(pDM_Odm, 0xce8, 0x3fff0000, Tx_dt[0] & 0x00003fff);
			ODM_SetBBReg(pDM_Odm, 0x82c, BIT(31), 0x0);		/* [31] = 0 --> Page C */
		}

		if (TX_finish == 1) {
			_rtl8812au_iqk_tx_fill_iqc(pDM_Odm, Path, TX_X, TX_Y);
		} else {
			_rtl8812au_iqk_tx_fill_iqc(pDM_Odm, Path, 0x200, 0x0);
		}

		if (RX_Average == 0) {
			_rtl8812au_iqk_rx_fill_iqc(pDM_Odm, Path, 0x200, 0x0);
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
						if (*pDM_Odm->pBandWidth == 2) {
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

		if (*pDM_Odm->pBandWidth == 2) {
			ODM_SetBBReg(pDM_Odm, 0x82c, BIT(31), 0x1); /* [31] = 0 --> Page C */
			ODM_SetBBReg(pDM_Odm, 0xce8, 0x00003fff, Rx_dt[0] & 0x00003fff);
			ODM_SetBBReg(pDM_Odm, 0x82c, BIT(31), 0x0); /* [31] = 0 --> Page C */
		}

		if (RX_finish == 1) {
			_rtl8812au_iqk_rx_fill_iqc(pDM_Odm, Path, RX_X, RX_Y);
		} else {
			_rtl8812au_iqk_rx_fill_iqc(pDM_Odm, Path, 0x200, 0x0);
		}

		/* 
		 * ULLI check with rtl8821ae source, if we can remove this
		 * look as the commit 5856f7384c22bf6364ecb77635e95b4858567080
		 * from #if 1 
		 */ 
#if 1
		if (TX_finish && RX_finish) {
			pRFCalibrateInfo->bNeedIQK = FALSE;

			if (*pDM_Odm->pBandWidth == 2) {
				ODM_SetBBReg(pDM_Odm, 0x82c, BIT(31), 0x1); /* [31] = 0 --> Page C */
				ODM_SetBBReg(pDM_Odm, 0x82c, BIT(31), 0x0); /* [31] = 0 --> Page C */
			}
		}
#endif		

	    }
		break;
	case ODM_RF_PATH_B:
	    {
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("========Path_B =======\n"));
		if (TX_Average == 0) {
			_rtl8812au_iqk_tx_fill_iqc(pDM_Odm, Path, 0x200, 0x0);
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
						if (*pDM_Odm->pBandWidth == 2) {
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

		if (*pDM_Odm->pBandWidth == 2) {
			ODM_SetBBReg(pDM_Odm, 0x82c, BIT(31), 0x1); /* [31] = 0 --> Page C */
			ODM_SetBBReg(pDM_Odm, 0xee8, 0x3fff0000, Tx_dt[0] & 0x00003fff);
			ODM_SetBBReg(pDM_Odm, 0x82c, BIT(31), 0x0); /* [31] = 0 --> Page C */
		}
		if (TX_finish == 1) {
			_rtl8812au_iqk_tx_fill_iqc(pDM_Odm, Path, TX_X, TX_Y);
		} else {
			_rtl8812au_iqk_tx_fill_iqc(pDM_Odm, Path, 0x200, 0x0);
		}

		if (RX_Average == 0) {
			_rtl8812au_iqk_rx_fill_iqc(pDM_Odm, Path, 0x200, 0x0);
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
						if (*pDM_Odm->pBandWidth == 2) {
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

		if (*pDM_Odm->pBandWidth == 2) {
			ODM_SetBBReg(pDM_Odm, 0x82c, BIT(31), 0x1); /* [31] = 0 --> Page C */
			ODM_SetBBReg(pDM_Odm, 0xee8, 0x00003fff, Rx_dt[0] & 0x00003fff);
			ODM_SetBBReg(pDM_Odm, 0x82c, BIT(31), 0x0); /* [31] = 0 --> Page C */
		}

		if (RX_finish == 1) {
			_rtl8812au_iqk_rx_fill_iqc(pDM_Odm, Path, RX_X, RX_Y);
		} else{
			_rtl8812au_iqk_rx_fill_iqc(pDM_Odm, Path, 0x200, 0x0);
		}


		/* 
		 * ULLI check with rtl8821ae source, if we can remove this
		 * look as the commit 5856f7384c22bf6364ecb77635e95b4858567080
		 * from #if 1 
		 */ 
#if 1		 
		if (TX_finish && RX_finish) {
/* pRFCalibrateInfo->IQKMatrixRegSetting[chnlIdx].bIQKDone= TRUE; */
			pRFCalibrateInfo->bNeedIQK = FALSE;

			if (*pDM_Odm->pBandWidth == 2) {
				ODM_SetBBReg(pDM_Odm, 0x82c, BIT(31), 0x1); /* [31] = 0 --> Page C */
				ODM_SetBBReg(pDM_Odm, 0x82c, BIT(31), 0x0); /* [31] = 0 --> Page C */
			}
		}
#endif		

	    }
		break;
	default:
		break;
	}
}



static void _rtl8812au_iqk_backup_macbb(struct rtl_dm *pDM_Odm, uint32_t *MACBB_backup,
	uint32_t *Backup_MACBB_REG, uint32_t MACBB_NUM)
{
	uint32_t i;
	ODM_SetBBReg(pDM_Odm, 0x82c, BIT(31), 0x0); /* [31] = 0 --> Page C */
	/* save MACBB default value */
	for (i = 0; i < MACBB_NUM; i++) {
		MACBB_backup[i] = ODM_Read4Byte(pDM_Odm, Backup_MACBB_REG[i]);
	}

	ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("BackupMacBB Success!!!!\n"));
}

static void _rtl8821au_iqk_backup_macbb(struct rtl_priv *rtlpriv,
					u32 *macbb_backup,
					u32 *backup_macbb_reg, u32 mac_bb_num)
{
	u32 i;

	rtl_set_bbreg(rtlpriv, 0x82c, BIT(31), 0x0); /* [31] = 0 --> Page C */
	/* save MACBB default value */
	for (i = 0; i < mac_bb_num; i++) {
		macbb_backup[i] = rtw_read32(rtlpriv, backup_macbb_reg[i]);
	}

	/* ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("BackupMacBB Success!!!!\n")); */
}

static void _rtl8812au_iqk_backup_rf(struct rtl_dm *pDM_Odm,
	uint32_t *RFA_backup, uint32_t *RFB_backup,
	uint32_t *Backup_RF_REG, uint32_t RF_NUM)
{
	uint32_t i;
	ODM_SetBBReg(pDM_Odm, 0x82c, BIT(31), 0x0); /* [31] = 0 --> Page C */
	/* Save RF Parameters */
	for (i = 0; i < RF_NUM; i++) {
		RFA_backup[i] = ODM_GetRFReg(pDM_Odm, ODM_RF_PATH_A, Backup_RF_REG[i], bMaskDWord);
		RFB_backup[i] = ODM_GetRFReg(pDM_Odm, ODM_RF_PATH_B, Backup_RF_REG[i], bMaskDWord);
	}
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("BackupRF Success!!!!\n"));
}

static void _rtl8821au_iqk_backup_rf(struct rtl_priv *rtlpriv, u32 *rfa_backup,
				     u32 *rfb_backup, u32 *backup_rf_reg,
				     u32 rf_num)
{
	u32 i;

	rtl_set_bbreg(rtlpriv, 0x82c, BIT(31), 0x0); /* [31] = 0 --> Page C */
	/* Save RF Parameters */
	for (i = 0; i < rf_num; i++) {
		rfa_backup[i] = rtw_hal_read_rfreg(rtlpriv, ODM_RF_PATH_A, backup_rf_reg[i], bMaskDWord);
	}

	/* ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("BackupRF Success!!!!\n")); */
}

static void _rtl8812au_iqk_backup_afe(struct rtl_dm *pDM_Odm,
	uint32_t *AFE_backup, uint32_t *Backup_AFE_REG, uint32_t AFE_NUM)
{
	uint32_t i;
	ODM_SetBBReg(pDM_Odm, 0x82c, BIT(31), 0x0); /* [31] = 0 --> Page C */
	/* Save AFE Parameters */
	for (i = 0; i < AFE_NUM; i++) {
		AFE_backup[i] = ODM_Read4Byte(pDM_Odm, Backup_AFE_REG[i]);
	}
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("BackupAFE Success!!!!\n"));
}

static void _rtl8821au_iqk_backup_afe(struct rtl_priv *rtlpriv, u32 *afe_backup,
				      u32 *backup_afe_REG, u32 afe_num)
{
	u32  i;

	rtl_set_bbreg(rtlpriv, 0x82c, BIT(31), 0x0); /* [31] = 0 --> Page C */
	/* Save AFE Parameters */
	for (i = 0; i < afe_num; i++) {
		afe_backup[i] = rtw_read32(rtlpriv, backup_afe_REG[i]);
	}
	/* ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("BackupAFE Success!!!!\n")); */
}

static void _rtl8812au_iqk_restore_macbb(struct rtl_dm *pDM_Odm,
	uint32_t *MACBB_backup, uint32_t *Backup_MACBB_REG, uint32_t MACBB_NUM)
{
	uint32_t i;
	ODM_SetBBReg(pDM_Odm, 0x82c, BIT(31), 0x0);     /* [31] = 0 --> Page C */
	/* Reload MacBB Parameters */
	for (i = 0; i < MACBB_NUM; i++) {
		ODM_Write4Byte(pDM_Odm, Backup_MACBB_REG[i], MACBB_backup[i]);
	}
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("RestoreMacBB Success!!!!\n"));
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
		rtw_write32(rtlpriv, backup_macbb_reg[i], macbb_backup[i]);
	}
	/* ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("RestoreMacBB Success!!!!\n")); */
}

static void _rtl8812au_iqk_restore_rf(struct rtl_dm *pDM_Odm,
	ODM_RF_RADIO_PATH_E Path, uint32_t *Backup_RF_REG, uint32_t *RF_backup, uint32_t RF_REG_NUM)
{
	uint32_t i;

	ODM_SetBBReg(pDM_Odm, 0x82c, BIT(31), 0x0); /*  [31] = 0 --> Page C */
	for (i = 0; i < RF_REG_NUM; i++)
		ODM_SetRFReg(pDM_Odm, Path, Backup_RF_REG[i], bRFRegOffsetMask, RF_backup[i]);

	ODM_SetRFReg(pDM_Odm, Path, 0xef, bRFRegOffsetMask, 0x0);

	switch (Path) {
	case ODM_RF_PATH_A:
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("RestoreRF Path A Success!!!!\n"));
		break;
	case ODM_RF_PATH_B:
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("RestoreRF Path B Success!!!!\n"));
		break;
	default:
		break;
	}
}

static void _rtl8821au_iqk_restore_rf(struct rtl_priv *rtlpriv,
				      ODM_RF_RADIO_PATH_E Path,
				      u32 *backup_rf_reg,
				      u32 *rf_backup, u32 rf_reg_num)
{
	u32 i;

	rtl_set_bbreg(rtlpriv, 0x82c, BIT(31), 0x0); /*  [31] = 0 --> Page C */
	for (i = 0; i < rf_reg_num; i++)
		rtw_hal_write_rfreg(rtlpriv, Path, backup_rf_reg[i], bRFRegOffsetMask, rf_backup[i]);

	switch (Path) {
	case ODM_RF_PATH_A:
		/* ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("RestoreRF Path A Success!!!!\n")); */
		break;
	default:
		break;
	}
}

static void  _rtl8812au_iqk_restore_afe(struct rtl_dm *pDM_Odm, uint32_t *AFE_backup,
	uint32_t *Backup_AFE_REG, uint32_t AFE_NUM)
{
	uint32_t i;
	ODM_SetBBReg(pDM_Odm, 0x82c, BIT(31), 0x0); /* [31] = 0 --> Page C */
	/* Reload AFE Parameters */
	for (i = 0; i < AFE_NUM; i++) {
		ODM_Write4Byte(pDM_Odm, Backup_AFE_REG[i], AFE_backup[i]);
	}
	ODM_SetBBReg(pDM_Odm, 0x82c, BIT(31), 0x1); /*  [31] = 1 --> Page C1 */
	ODM_Write4Byte(pDM_Odm, 0xc80, 0x0);
	ODM_Write4Byte(pDM_Odm, 0xc84, 0x0);
	ODM_Write4Byte(pDM_Odm, 0xc88, 0x0);
	ODM_Write4Byte(pDM_Odm, 0xc8c, 0x3c000000);
	ODM_Write4Byte(pDM_Odm, 0xcb8, 0x0);
	ODM_Write4Byte(pDM_Odm, 0xe80, 0x0);
	ODM_Write4Byte(pDM_Odm, 0xe84, 0x0);
	ODM_Write4Byte(pDM_Odm, 0xe88, 0x0);
	ODM_Write4Byte(pDM_Odm, 0xe8c, 0x3c000000);
	ODM_Write4Byte(pDM_Odm, 0xeb8, 0x0);
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("RestoreAFE Success!!!!\n"));
}

static void _rtl8821au_iqk_restore_afe(struct rtl_priv *rtlpriv,
				       u32 *afe_backup, u32 *backup_afe_reg,
				       u32 afe_num)
{
	uint32_t i;
	rtl_set_bbreg(rtlpriv, 0x82c, BIT(31), 0x0); /* [31] = 0 --> Page C */
	/* Reload AFE Parameters */
	for (i = 0; i < afe_num; i++) {
		rtw_write32(rtlpriv, backup_afe_reg[i], afe_backup[i]);
	}
	rtl_set_bbreg(rtlpriv, 0x82c, BIT(31), 0x1); /* [31] = 1 --> Page C1 */
	rtw_write32(rtlpriv, 0xc80, 0x0);
	rtw_write32(rtlpriv, 0xc84, 0x0);
	rtw_write32(rtlpriv, 0xc88, 0x0);
	rtw_write32(rtlpriv, 0xc8c, 0x3c000000);
	rtw_write32(rtlpriv, 0xcb8, 0x0);
	/* ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("RestoreAFE Success!!!!\n")); */
}


static void _DPK_parareload(struct rtl_dm *pDM_Odm,
	uint32_t *MACBB_backup, uint32_t *Backup_MACBB_REG,
	uint32_t MACBB_NUM)
{
	uint32_t i;

	ODM_SetBBReg(pDM_Odm, 0x82c, BIT(31), 0x0);	/* [31] = 0 --> Page C */

	/* save MACBB default value */
	for (i = 0; i < MACBB_NUM; i++) {
		ODM_Write4Byte(pDM_Odm, Backup_MACBB_REG[i], MACBB_backup[i]);
	}
}


static void _DPK_parabackup(struct rtl_dm *pDM_Odm, uint32_t *MACBB_backup,
	uint32_t *Backup_MACBB_REG, uint32_t MACBB_NUM)
{
	uint32_t i;

	ODM_SetBBReg(pDM_Odm, 0x82c, BIT(31), 0x0);	/* [31] = 0 --> Page C */
	/* save MACBB default value */

	for (i = 0; i < MACBB_NUM; i++) {
		MACBB_backup[i] = ODM_Read4Byte(pDM_Odm, Backup_MACBB_REG[i]);
	}
}

static void _DPK_Globalparaset(struct rtl_dm *pDM_Odm)
{

	/* ************************************* */
	/* set MAC register */
	/* ************************************* */

	/* TX pause */
	ODM_Write4Byte(pDM_Odm, 0x520, 0x007f3F0F);

	/* ************************************* */
	/* set BB register */
	/* ************************************* */

	/* reg82c[31] = b'0, ¤Á´«¨ì page C */
	ODM_Write4Byte(pDM_Odm, 0x82c, 0x002083d5);

	/* test pin in/out control */
	ODM_Write4Byte(pDM_Odm, 0x970, 0x00000000);

	/* path A regcb8[3:0] = h'd, TRSW to TX */
	ODM_Write4Byte(pDM_Odm, 0xcb8, 0x0050824d);

	/* path B regeb8[3:0] = h'd, TRSW to TX */
	ODM_Write4Byte(pDM_Odm, 0xeb8, 0x0050824d);

	/* reg838[3:0] = h'c, CCA off */
	ODM_Write4Byte(pDM_Odm, 0x838, 0x06c8d24c);

	/* path A 3-wire off */
	ODM_Write4Byte(pDM_Odm, 0xc00, 0x00000004);

	/* path B 3-wire off */
	ODM_Write4Byte(pDM_Odm, 0xe00, 0x00000004);

	/* reg90c[15] = b'1, DAC fifo reset by CSWU */
	ODM_Write4Byte(pDM_Odm, 0x90c, 0x00008000);

	/* reset DPK circuit */
	ODM_Write4Byte(pDM_Odm, 0xb00, 0x03000100);

	/* path A regc94[0] = b'1 (r_gothrough_iqkdpk), ±N DPK ¤Á¶i normal path */
	ODM_Write4Byte(pDM_Odm, 0xc94, 0x01000001);

	/* path B rege94[0] = b'1 (r_gothrough_iqkdpk), ±N DPK ¤Á¶i normal path */
	ODM_Write4Byte(pDM_Odm, 0xe94, 0x01000001);

	/* ************************************* */
	/* set AFE register */
	/* ************************************* */

	/* path A */
	/* regc68 ¨ì regc84À³¸Ó¬O­n¸ò¥¿±` Tx mode ®Éªº³]©w¤@­P */

	ODM_Write4Byte(pDM_Odm, 0xc68, 0x19791979);
	ODM_Write4Byte(pDM_Odm, 0xc6c, 0x19791979);
	ODM_Write4Byte(pDM_Odm, 0xc70, 0x19791979);
	ODM_Write4Byte(pDM_Odm, 0xc74, 0x19791979);
	ODM_Write4Byte(pDM_Odm, 0xc78, 0x19791979);
	ODM_Write4Byte(pDM_Odm, 0xc7c, 0x19791979);
	ODM_Write4Byte(pDM_Odm, 0xc80, 0x19791979);
	ODM_Write4Byte(pDM_Odm, 0xc84, 0x19791979);

	/* force DAC/ADC power on */
	ODM_Write4Byte(pDM_Odm, 0xc60, 0x77777777);
	ODM_Write4Byte(pDM_Odm, 0xc64, 0x77777777);

	/* path B */
	/* rege68 ¨ì rege84À³¸Ó¬O­n¸ò¥¿±` Tx mode ®Éªº³]©w¤@­P */

	ODM_Write4Byte(pDM_Odm, 0xe68, 0x19791979);
	ODM_Write4Byte(pDM_Odm, 0xe6c, 0x19791979);
	ODM_Write4Byte(pDM_Odm, 0xe70, 0x19791979);
	ODM_Write4Byte(pDM_Odm, 0xe74, 0x19791979);
	ODM_Write4Byte(pDM_Odm, 0xe78, 0x19791979);
	ODM_Write4Byte(pDM_Odm, 0xe7c, 0x19791979);
	ODM_Write4Byte(pDM_Odm, 0xe80, 0x19791979);
	ODM_Write4Byte(pDM_Odm, 0xe84, 0x19791979);

	/* force DAC/ADC power on */
	ODM_Write4Byte(pDM_Odm, 0xe60, 0x77777777);
	ODM_Write4Byte(pDM_Odm, 0xe64, 0x77777777);

}


static void _DPK_GetGainLoss(struct rtl_dm *pDM_Odm, u8 path)
{
	uint32_t GL_I = 0, GL_Q = 0;
	uint32_t GL_I_tmp = 0, GL_Q_tmp = 0;

	uint32_t Power_GL;
	u16 Scaler[] = { 0x4000, 0x41db, 0x43c7, 0x45c3, 0x47cf,
			      0x49ec, 0x4c19, 0x4e46, 0x5093, 0x52f2,  /* 10 */
			      0x5560, 0x57cf, 0x5a7f, 0x5d0e, 0x5fbe
			};
	u8 sindex = 0;
	uint32_t pagesel = 0, regsel = 0;

	if (path == 0) { /* pathA */
		pagesel = 0;
		regsel = 0;
	} else {	/* pathB */
		pagesel = 0x200;
		regsel = 0x40;
	}

	ODM_Write4Byte(pDM_Odm, 0xc90+pagesel, 0x0601f0bf);
	ODM_Write4Byte(pDM_Odm, 0xcb8+pagesel, 0x0c000000);

	GL_I_tmp = ODM_GetBBReg(pDM_Odm, 0xd00+regsel, 0xffff0000);
	GL_Q_tmp = ODM_GetBBReg(pDM_Odm, 0xd00+regsel, 0x0000ffff);

	if (GL_I_tmp >= 0x8000)
		GL_I = (GL_I_tmp-0x8000+0x1);
	else
		GL_I = GL_I_tmp;

	if (GL_Q_tmp >= 0x8000)
		GL_Q = (GL_Q_tmp-0x8000+0x1);
	else
		GL_Q = GL_Q_tmp;

	Power_GL = ((GL_I*GL_I)+(GL_Q*GL_Q))/4;
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("Power_GL = 0x%x", Power_GL));

	if (Power_GL > 63676) {
		sindex = 0;
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("Gainloss = 0 dB\n"));
	} else if (63676 >= Power_GL && Power_GL > 60114) {
		sindex = 1;
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("Gainloss = 	0.25 dB\n"));
	} else if (60114 >= Power_GL && Power_GL > 56751) {
		sindex = 2;
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("Gainloss = 	0.5 dB\n"));
	} else if (56751 >= Power_GL && Power_GL > 53577) {
		sindex = 3;
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("Gainloss = 	0.75 dB\n"));
	} else if (53577 >= Power_GL && Power_GL > 49145) {
		sindex = 4;
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("Gainloss = 	1 dB\n"));
	} else if (49145 >= Power_GL && Power_GL > 47750) {
		sindex = 5;
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("Gainloss = 	1.25 dB\n"));
	} else if (47750 >= Power_GL && Power_GL > 45079) {
		sindex = 6;
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("Gainloss = 	1.5 dB\n"));
	} else if (45079 >= Power_GL && Power_GL > 42557) {
		sindex = 7;
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("Gainloss = 	1.75 dB\n"));
	} else if (42557 >= Power_GL && Power_GL > 40177) {
		sindex = 8;
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("Gainloss = 	2 dB\n"));
	} else if (40177 >= Power_GL && Power_GL > 37929) {
		sindex = 9;
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("Gainloss = 	2.25 dB\n"));
	} else if (37929 >= Power_GL && Power_GL > 35807) {
		sindex = 10;
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("Gainloss = 	2.5 dB\n"));
	} else if (35807 >= Power_GL && Power_GL > 33804) {
		sindex = 11;
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("Gainloss = 	2.75 dB\n"));
	} else if (33804 >= Power_GL && Power_GL > 31913) {
		sindex = 12;
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("Gainloss = 	3 dB\n"));
	} else if (31913 >= Power_GL && Power_GL > 30128) {
		sindex = 13;
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("Gainloss = 	3.25 dB\n"));
	} else if (30128 >= Power_GL) {
		sindex = 14;
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("Gainloss = 	3.5 dB\n"));
	}

	ODM_Write4Byte(pDM_Odm, 0xc98+pagesel, (Scaler[sindex] << 16) | Scaler[sindex]);
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("Set Gainloss reg 0xc98(0xe98)= 0x%x\n", ODM_Read4Byte(pDM_Odm, 0xc98+pagesel)));

}


static void _DPK_EnableDP(struct rtl_dm *pDM_Odm, u8 path, uint32_t TXindex)
{
	/* ************************************* */
	/* Enable DP */
	/* ************************************* */

	/* PWSF[6] = 0x40 = 0dB, set the address represented TXindex as 0dB */
	u8 PWSF[] = { 0xff, 0xca, 0xa1, 0x80, 0x65, 0x51, 0x40,  /* 6~0dB */
			  0x33, 0x28, 0x20, 0x19, 0x14, 0x10, 0x0d,  /* -1~-7dB */
			  0x0a, 0x08, 0x06, 0x05, 0x04, 0x03, 0x03,  /* -8~-14dB */
			  0x02, 0x02, 0x01, 0x01,  };
	u8 zeropoint;
	u8 pwsf1, pwsf2;
	u8 i;
	uint32_t pagesel = 0, regsel = 0;

	if (path == 0) {
		pagesel = 0;
		regsel = 0;
	} else {
		pagesel = 0x200;
		regsel = 0x40;
	}

	/* ========= */
	/* DPK setting	*/
	/* ========= */
	/* reg82c[31] = b'1, ¤Á´«¨ì page C1 */
	ODM_Write4Byte(pDM_Odm, 0x82c, 0x802083d5);


	ODM_Write4Byte(pDM_Odm, 0xc90+pagesel, 0x0000f098);
	ODM_Write4Byte(pDM_Odm, 0xc94+pagesel, 0x776c9f84);
	ODM_Write4Byte(pDM_Odm, 0xcc4+pagesel, 0x08840000);
	ODM_Write4Byte(pDM_Odm, 0xcc8+pagesel, 0x20000000);
	ODM_Write4Byte(pDM_Odm, 0xc8c+pagesel, 0x3c000000);


	/* ¼gPWSF table in 1st SRAM for PA = 11 use */
	ODM_Write4Byte(pDM_Odm, 0xc20+pagesel, 0x00000800);

	ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("Write PWSF table\n"));


	if  (TXindex == 0x1f)
		zeropoint = 0;
	else if (TXindex == 0x1e)
		zeropoint = 1;
	else if (TXindex == 0x1d)
		zeropoint = 2;
	else if (TXindex == 0x1c)
		zeropoint = 3;
	else if (TXindex == 0x1b)
		zeropoint = 4;
	else if (TXindex == 0x1a)
		zeropoint = 5;
	else if (TXindex == 0x19)
		zeropoint = 6;
	else
		zeropoint = 6;

	for (i = 0; i < 16; i++) {
		if ((6-zeropoint)+i*2 > 24)
			pwsf1 = 24;
		else
			pwsf1 = (6-zeropoint)+i*2;

		if ((6-zeropoint-1)+i*2 > 24)
			pwsf2 = 24;
		else
			pwsf2 = (6-zeropoint-1)+i*2;

		ODM_Write4Byte(pDM_Odm, 0xce4+pagesel, 0x00000001 | i<<1 | (PWSF[pwsf1]<<8) | (PWSF[pwsf2]<<16));
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("0x%x\n", ODM_Read4Byte(pDM_Odm, 0xce4+pagesel)));
		ODM_SetBBReg(pDM_Odm, 0xce4+pagesel, 0xff, 0x0);
	}

	ODM_Write4Byte(pDM_Odm, 0xce4+pagesel, 0x00000000);

	/* reg82c[31] = b'0, ¤Á´«¨ì page C */
	ODM_Write4Byte(pDM_Odm, 0x82c, 0x002083d5);

}

static void _rtl8812au_iqk_configure_mac(struct rtl_dm *pDM_Odm)
{
	/* ========MAC register setting======== */
	ODM_SetBBReg(pDM_Odm, 0x82c, BIT(31), 0x0); 	/* [31] = 0 --> Page C */
	ODM_Write1Byte(pDM_Odm, 0x522, 0x3f);
	ODM_SetBBReg(pDM_Odm, 0x550, BIT(11)|BIT(3), 0x0);
	ODM_SetBBReg(pDM_Odm, 0x808, BIT(28), 0x0);	/* CCK Off */
	ODM_Write1Byte(pDM_Odm, 0x808, 0x00);		/* RX ante off */
	ODM_SetBBReg(pDM_Odm, 0x838, 0xf, 0xc);		/* CCA off */
}

static void _rtl8821au_iqk_configure_mac(struct rtl_priv *rtlpriv)
{
	/* ========MAC register setting======== */
	rtl_set_bbreg(rtlpriv, 0x82c, BIT(31), 0x0); /* [31] = 0 --> Page C */
	rtw_write8(rtlpriv, 0x522, 0x3f);
	rtl_set_bbreg(rtlpriv, 0x550, BIT(3), 0x0);
	rtl_set_bbreg(rtlpriv, 0x551, BIT(3), 0x0);
	rtl_set_bbreg(rtlpriv, 0x808, BIT(28), 0x0);	/* CCK Off */
	rtw_write8(rtlpriv, 0x808, 0x00);		/* RX ante off */
	rtl_set_bbreg(rtlpriv, 0x838, 0xf, 0xc);		/* CCA off */
}


#undef MACBB_REG_NUM
#undef AFE_REG_NUM
#undef RF_REG_NUM 
#define MACBB_REG_NUM 10
#define AFE_REG_NUM 14
#define RF_REG_NUM 3


/* Maintained by BB James. */
static void _rtl8812au_phy_iq_calibrate(struct rtl_dm *pDM_Odm, u8 Channel)
{
	uint32_t MACBB_backup[MACBB_REG_NUM], AFE_backup[AFE_REG_NUM], RFA_backup[RF_REG_NUM], RFB_backup[RF_REG_NUM];
	uint32_t Backup_MACBB_REG[MACBB_REG_NUM] = { 0xb00, 0x520, 0x550, 0x808, 0x90c, 0xc00, 0xe00, 0x8c4, 0x838, 0x82c };
	uint32_t Backup_AFE_REG[AFE_REG_NUM] = { 0xc5c, 0xc60, 0xc64, 0xc68, 0xcb8, 0xcb0, 0xcb4,
						 0xe5c, 0xe60, 0xe64, 0xe68, 0xeb8, 0xeb0, 0xeb4 };
	uint32_t Backup_RF_REG[RF_REG_NUM] = { 0x65, 0x8f, 0x0 };
	u8 	chnlIdx = ODM_GetRightChnlPlaceforIQK(Channel);

	_rtl8812au_iqk_backup_macbb(pDM_Odm, MACBB_backup, Backup_MACBB_REG, MACBB_REG_NUM);
	_rtl8812au_iqk_backup_afe(pDM_Odm, AFE_backup, Backup_AFE_REG, AFE_REG_NUM);
	_rtl8812au_iqk_backup_rf(pDM_Odm, RFA_backup, RFB_backup, Backup_RF_REG, RF_REG_NUM);

	_rtl8812au_iqk_configure_mac(pDM_Odm);
	_rtl8812au_iqk_tx(pDM_Odm, ODM_RF_PATH_A);
	_rtl8812au_iqk_restore_rf(pDM_Odm, ODM_RF_PATH_A, Backup_RF_REG, RFA_backup, RF_REG_NUM);

	_rtl8812au_iqk_tx(pDM_Odm, ODM_RF_PATH_B);
	_rtl8812au_iqk_restore_rf(pDM_Odm, ODM_RF_PATH_B, Backup_RF_REG, RFB_backup, RF_REG_NUM);

	 _rtl8812au_iqk_restore_afe(pDM_Odm, AFE_backup, Backup_AFE_REG, AFE_REG_NUM);
	_rtl8812au_iqk_restore_macbb(pDM_Odm, MACBB_backup, Backup_MACBB_REG, MACBB_REG_NUM);
}



void rtl8812au_phy_iq_calibrate(struct rtl_priv *pAdapter, BOOLEAN bReCovery)
{
	 struct rtw_hal	*pHalData = GET_HAL_DATA(pAdapter);

	struct rtl_dm *	pDM_Odm = &pHalData->odmpriv;

	{
		/*
		 * if(pMgntInfo->RegIQKFWOffload)
		 * 	phy_IQCalibrate_By_FW_8812A(pAdapter);
		 * else
		 */
			_rtl8812au_phy_iq_calibrate(pDM_Odm, pHalData->CurrentChannel);
	}

}



/* ****************************************************************************** */
/*									*/
/*  from HalPhyRf_8821A.c						*/
/*									*/
/* ****************************************************************************** */







static void _DPK_pathABDPK(struct rtl_dm *pDM_Odm)
{
	uint32_t TXindex = 0;
	u8 path = 0;
	uint32_t pagesel = 0, regsel = 0;
	uint32_t i = 0, j = 0;

	for (path = 0; path < 2; path++) {	/* path A = 0; path B = 1; */
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("path %s DPK start!!!\n", (path == 0) ? "A" : "B"));

		if (path == 0) {
			pagesel = 0;
			regsel = 0;
		} else {
			pagesel = 0x200;
			regsel = 0x40;
		}

		/* ************************************** */
		/* find compress-2.5dB TX index */
		/* ************************************** */


		/* reg82c[31] = b'1, ¤Á´«¨ì page C1 */
		ODM_Write4Byte(pDM_Odm, 0x82c, 0x802083d5);

	/*
	 * regc20[15:13] = dB sel, §i¶D Gain Loss function ¥h´M§ä dB_sel ©Ò³]©wªºPA gain loss¥Ø¼Ð©Ò¹ïÀ³ªº Tx AGC ¬°¦ó.
	 * dB_sel = b'000 ' 1.0 dB PA gain loss
	 * dB_sel = b'001 ' 1.5 dB PA gain loss
	 * dB_sel = b'010 ' 2.0 dB PA gain loss
	 * dB_sel = b'011 ' 2.5 dB PA gain loss
	 * dB_sel = b'100 ' 3.0 dB PA gain loss
	 * dB_sel = b'101 ' 3.5 dB PA gain loss
	 * dB_sel = b'110 ' 4.0 dB PA gain loss
	 */
	ODM_Write4Byte(pDM_Odm, 0xc20+pagesel, 0x00006000);

	ODM_Write4Byte(pDM_Odm, 0xc90+pagesel, 0x0401e038);
	ODM_Write4Byte(pDM_Odm, 0xc94+pagesel, 0xf76c9f84);
	ODM_Write4Byte(pDM_Odm, 0xcc8+pagesel, 0x000c5599);
	ODM_Write4Byte(pDM_Odm, 0xcc4+pagesel, 0x148b0000);
	ODM_Write4Byte(pDM_Odm, 0xc8c+pagesel, 0x3c000000);

	/* tx_amp ' ¨M©w Ramp ¤¤¦U©¶ªiªº®¶´T¤j¤p */
	ODM_Write4Byte(pDM_Odm, 0xc98+pagesel, 0x41382e21);
	ODM_Write4Byte(pDM_Odm, 0xc9c+pagesel, 0x5b554f48);
	ODM_Write4Byte(pDM_Odm, 0xca0+pagesel, 0x6f6b6661);
	ODM_Write4Byte(pDM_Odm, 0xca4+pagesel, 0x817d7874);
	ODM_Write4Byte(pDM_Odm, 0xca8+pagesel, 0x908c8884);
	ODM_Write4Byte(pDM_Odm, 0xcac+pagesel, 0x9d9a9793);
	ODM_Write4Byte(pDM_Odm, 0xcb0+pagesel, 0xaaa7a4a1);
	ODM_Write4Byte(pDM_Odm, 0xcb4+pagesel, 0xb6b3b0ad);

	/* tx_inverse ' Ramp ¤¤¦U©¶ªipower ªº­Ë¼Æ, ¥H­pºâ¥X PA ªº gain report?? */
	ODM_Write4Byte(pDM_Odm, 0xc40+pagesel, 0x02ce03e9);
	ODM_Write4Byte(pDM_Odm, 0xc44+pagesel, 0x01fd0249);
	ODM_Write4Byte(pDM_Odm, 0xc48+pagesel, 0x01a101c9);
	ODM_Write4Byte(pDM_Odm, 0xc4c+pagesel, 0x016a0181);
	ODM_Write4Byte(pDM_Odm, 0xc50+pagesel, 0x01430155);
	ODM_Write4Byte(pDM_Odm, 0xc54+pagesel, 0x01270135);
	ODM_Write4Byte(pDM_Odm, 0xc58+pagesel, 0x0112011c);
	ODM_Write4Byte(pDM_Odm, 0xc5c+pagesel, 0x01000108);
	ODM_Write4Byte(pDM_Odm, 0xc60+pagesel, 0x00f100f8);
	ODM_Write4Byte(pDM_Odm, 0xc64+pagesel, 0x00e500eb);
	ODM_Write4Byte(pDM_Odm, 0xc68+pagesel, 0x00db00e0);
	ODM_Write4Byte(pDM_Odm, 0xc6c+pagesel, 0x00d100d5);
	ODM_Write4Byte(pDM_Odm, 0xc70+pagesel, 0x00c900cd);
	ODM_Write4Byte(pDM_Odm, 0xc74+pagesel, 0x00c200c5);
	ODM_Write4Byte(pDM_Odm, 0xc78+pagesel, 0x00bb00be);
	ODM_Write4Byte(pDM_Odm, 0xc7c+pagesel, 0x00b500b8);

	/* ============ */
	/* RF setting for DPK */
	/* ============ */

	/* pathA,pathB standby mode */
	ODM_SetRFReg(pDM_Odm, (ODM_RF_RADIO_PATH_E)0x0, 0x0, bRFRegOffsetMask, 0x10000);
	ODM_SetRFReg(pDM_Odm, (ODM_RF_RADIO_PATH_E)0x1, 0x0, bRFRegOffsetMask, 0x10000);

	/* 00[4:0] = Tx AGC, 00[9:5] = Rx AGC (BB), 00[12:10] = Rx AGC (LNA) */
	ODM_SetRFReg(pDM_Odm, (ODM_RF_RADIO_PATH_E)(0x0+path), 0x0, bRFRegOffsetMask, 0x50bff);

	/* 64[14:12] = loop back attenuation */
	ODM_SetRFReg(pDM_Odm, (ODM_RF_RADIO_PATH_E)(0x0+path), 0x64, bRFRegOffsetMask, 0x19aac);

	/* 8f[14:13] = PGA2 gain */
	ODM_SetRFReg(pDM_Odm, (ODM_RF_RADIO_PATH_E)(0x0+path), 0x8f, bRFRegOffsetMask, 0x8e001);


	/* one shot */
	ODM_Write4Byte(pDM_Odm, 0xcc8+pagesel, 0x800c5599);
	ODM_Write4Byte(pDM_Odm, 0xcc8+pagesel, 0x000c5599);


	/* delay 100 ms */
	mdelay(100);


	/* read back */
	ODM_Write4Byte(pDM_Odm, 0xc90+pagesel, 0x0109f018);
	ODM_Write4Byte(pDM_Odm, 0xcb8+pagesel, 0x09000000);
	/*
	 * ¥i¥H¦b d00[3:0] ¤¤Åª¦^, dB_sel ¤¤©Ò³]©wªº gain loss ·|¸¨¦b­þ¤@­Ó Tx AGC ³]©w
	 * Åª¦^d00[3:0] = h'1 ' Tx AGC = 15
	 * Åª¦^d00[3:0] = h'2 ' Tx AGC = 16
	 * Åª¦^d00[3:0] = h'3 ' Tx AGC = 17
	 * Åª¦^d00[3:0] = h'4 ' Tx AGC = 18
	 * Åª¦^d00[3:0] = h'5 ' Tx AGC = 19
	 * Åª¦^d00[3:0] = h'6 ' Tx AGC = 1a
	 * Åª¦^d00[3:0] = h'7 ' Tx AGC = 1b
	 * Åª¦^d00[3:0] = h'8 ' Tx AGC = 1c
	 * Åª¦^d00[3:0] = h'9 ' Tx AGC = 1d
	 * Åª¦^d00[3:0] = h'a ' Tx AGC = 1e
	 * Åª¦^d00[3:0] = h'b ' Tx AGC = 1f
	 */

	TXindex = ODM_GetBBReg(pDM_Odm, 0xd00+regsel, 0x0000000f);

	/* ************************************* */
	/* get LUT */
	/* ************************************* */

	ODM_Write4Byte(pDM_Odm, 0xc90+pagesel, 0x0001e038);
	ODM_Write4Byte(pDM_Odm, 0xc94+pagesel, 0xf76c9f84);
	ODM_Write4Byte(pDM_Odm, 0xcc8+pagesel, 0x400c5599);
	ODM_Write4Byte(pDM_Odm, 0xcc4+pagesel, 0x11930080);	/* 0xcc4[9:4]= DPk fail threshold */
	ODM_Write4Byte(pDM_Odm, 0xc8c+pagesel, 0x3c000000);


	/* tx_amp ' ¨M©w Ramp ¤¤¦U©¶ªiªº®¶´T¤j¤p */

	ODM_Write4Byte(pDM_Odm, 0xc98+pagesel, 0x41382e21);
	ODM_Write4Byte(pDM_Odm, 0xc9c+pagesel, 0x5b554f48);
	ODM_Write4Byte(pDM_Odm, 0xca0+pagesel, 0x6f6b6661);
	ODM_Write4Byte(pDM_Odm, 0xca4+pagesel, 0x817d7874);
	ODM_Write4Byte(pDM_Odm, 0xca8+pagesel, 0x908c8884);
	ODM_Write4Byte(pDM_Odm, 0xcac+pagesel, 0x9d9a9793);
	ODM_Write4Byte(pDM_Odm, 0xcb0+pagesel, 0xaaa7a4a1);
	ODM_Write4Byte(pDM_Odm, 0xcb4+pagesel, 0xb6b3b0ad);

	/* tx_inverse ' Ramp ¤¤¦U©¶ªipower ªº­Ë¼Æ, ¥H­pºâ¥X PA ªº gain */
	ODM_Write4Byte(pDM_Odm, 0xc40+pagesel, 0x02ce03e9);
	ODM_Write4Byte(pDM_Odm, 0xc44+pagesel, 0x01fd0249);
	ODM_Write4Byte(pDM_Odm, 0xc48+pagesel, 0x01a101c9);
	ODM_Write4Byte(pDM_Odm, 0xc4c+pagesel, 0x016a0181);
	ODM_Write4Byte(pDM_Odm, 0xc50+pagesel, 0x01430155);
	ODM_Write4Byte(pDM_Odm, 0xc54+pagesel, 0x01270135);
	ODM_Write4Byte(pDM_Odm, 0xc58+pagesel, 0x0112011c);
	ODM_Write4Byte(pDM_Odm, 0xc5c+pagesel, 0x01000108);
	ODM_Write4Byte(pDM_Odm, 0xc60+pagesel, 0x00f100f8);
	ODM_Write4Byte(pDM_Odm, 0xc64+pagesel, 0x00e500eb);
	ODM_Write4Byte(pDM_Odm, 0xc68+pagesel, 0x00db00e0);
	ODM_Write4Byte(pDM_Odm, 0xc6c+pagesel, 0x00d100d5);
	ODM_Write4Byte(pDM_Odm, 0xc70+pagesel, 0x00c900cd);
	ODM_Write4Byte(pDM_Odm, 0xc74+pagesel, 0x00c200c5);
	ODM_Write4Byte(pDM_Odm, 0xc78+pagesel, 0x00bb00be);
	ODM_Write4Byte(pDM_Odm, 0xc7c+pagesel, 0x00b500b8);

	/* fill BB TX index for the DPK reference */
	/* reg82c[31] =1b'0, ¤Á´«¨ì page C */
	ODM_Write4Byte(pDM_Odm, 0x82c, 0x002083d5);

	ODM_Write4Byte(pDM_Odm, 0xc20+pagesel, 0x3c3c3c3c);
	ODM_Write4Byte(pDM_Odm, 0xc24+pagesel, 0x3c3c3c3c);
	ODM_Write4Byte(pDM_Odm, 0xc28+pagesel, 0x3c3c3c3c);
	ODM_Write4Byte(pDM_Odm, 0xc2c+pagesel, 0x3c3c3c3c);
	ODM_Write4Byte(pDM_Odm, 0xc30+pagesel, 0x3c3c3c3c);
	ODM_Write4Byte(pDM_Odm, 0xc34+pagesel, 0x3c3c3c3c);
	ODM_Write4Byte(pDM_Odm, 0xc38+pagesel, 0x3c3c3c3c);
	ODM_Write4Byte(pDM_Odm, 0xc3c+pagesel, 0x3c3c3c3c);
	ODM_Write4Byte(pDM_Odm, 0xc40+pagesel, 0x3c3c3c3c);
	ODM_Write4Byte(pDM_Odm, 0xc44+pagesel, 0x3c3c3c3c);
	ODM_Write4Byte(pDM_Odm, 0xc48+pagesel, 0x3c3c3c3c);
	ODM_Write4Byte(pDM_Odm, 0xc4c+pagesel, 0x3c3c3c3c);

	/* reg82c[31] =1b'1, ¤Á´«¨ì page C1 #*/
	ODM_Write4Byte(pDM_Odm, 0x82c, 0x802083d5);

	/*
	 *  r_agc_boudary
	 *  PA gain = 11 ¹ïÀ³ tx_agc ±q1f ¨ì11  boundary = b'11111 ' PageC1 ªº bc0[4:0] = 11111
	 *  PA gain = 10 ¹ïÀ³ tx_agc ±q11 ¨ì11 ? boundary = b'10011 ' PageC1 ªº bc0[9:5] = 10001
	 *  PA gain = 01 ¹ïÀ³ tx_agc ±q10 ¨ì0e ? boundary = b'10000 ' PageC1 ªº bc0[14:10] = 10000
	 *  PA gain = 00 ¹ïÀ³ tx_agc ±q0d ¨ì00 ? boundary = b'01101 ' PageC1 ªº bc0[19:15] = 01101
	 */
	ODM_Write4Byte(pDM_Odm, 0xcbc+pagesel, 0x0006c23f);

	/* r_bnd, ¥t¥~4¶ô PWSF (power scaling factor) ªº boundary, ¦]¬°¥Ø«e¥u¦³¦b PA gain = 11 ®É¤~°µ¸ÉÀv, ©Ò¥H³]¦¨ h'fffff §Y¥i. */
	ODM_Write4Byte(pDM_Odm, 0xcb8+pagesel, 0x000fffff);

	/* ============ */
	/* RF setting for DPK */
	/* ============ */
	/* 00[4:0] = Tx AGC, 00[9:5] = Rx AGC (BB), 00[12:10] = Rx AGC (LNA) */
	/* ¦¹³B reg00[4:0] = h'1d, ¬O¥Ñ«e­± gain loss function ±o¨ìªºµ²ªG. */
	ODM_SetRFReg(pDM_Odm, (ODM_RF_RADIO_PATH_E)(0x0+path), 0x0, bRFRegOffsetMask, 0x517e0 | TXindex);
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("RF 0x0 = 0x%x\n", 0x517e0 | TXindex));

	/* one shot */
	ODM_Write4Byte(pDM_Odm, 0xcc8+pagesel, 0xc00c5599);
	ODM_Write4Byte(pDM_Odm, 0xcc8+pagesel, 0x400c5599);

	/* delay 100 ms */
	mdelay(100);

	/* read back dp_fail report */
	ODM_Write4Byte(pDM_Odm, 0xcb8+pagesel, 0x00000000);

	/* if d00[6] = 1, DPK fail */
	if (ODM_GetBBReg(pDM_Odm, 0xd00+regsel, BIT6)) 	{
		/* bypass DPK */
		ODM_Write4Byte(pDM_Odm, 0xcc4+pagesel, 0x28848000);
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("path %s DPK fail!!!!!!!!!!!!!!!!!!!!!\n", (path == 0) ? "A" : "B"));

		return;
	}

	ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("path %s DPK ok!!!!!!!!!!!!!!!!!!!!!\n", (path == 0) ? "A" : "B"));



	/* read LMS table -->debug message only */
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("read LMS\n"));

	for (i = 0; i < 8; i++) {
		ODM_Write4Byte(pDM_Odm, 0xc90+pagesel, 0x0601f0b8+i);
		for (j = 0; j < 4; j++) {
			ODM_Write4Byte(pDM_Odm, 0xcb8+pagesel, 0x09000000+(j<<24));
			ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("0x%x", ODM_Read4Byte(pDM_Odm, 0xd00+regsel)));
			}
		}


	/* ************************************* */
	/* get gain loss */
	/* ************************************* */

	_DPK_GetGainLoss(pDM_Odm, path);


	/* ************************************* */
	/* Enable DP */
	/* ************************************* */

	_DPK_EnableDP(pDM_Odm, path, TXindex);
	}


}



static void phy_DPCalibrate_8812A(struct rtl_dm *pDM_Odm)
{
	uint32_t backupRegAddrs[] = {
		0x970, 0xcb8, 0x838, 0xc00, 0x90c, 0xb00, 0xc94, 0x82c, 0x520, 0xc60, /* 10 */
		0xc64, 0xc68, 0xc6c, 0xc70, 0xc74, 0xc78, 0xc7c, 0xc80, 0xc84, 0xc50, /* 20 */
		0xc20, 0xc24, 0xc28, 0xc2c, 0xc30, 0xc34, 0xc38, 0xc3c, 0xc40, 0xc44, /* 30 */
		0xc48, 0xc4c, 0xe50, 0xe20, 0xe24, 0xe28, 0xe2c, 0xe30, 0xe34, 0xe38, /* 40 */
		0xe3c, 0xe40, 0xe44, 0xe48, 0xe4c, 0xeb8, 0xe00, 0xe94, 0xe60, 0xe64, /* 50 */
		0xe68, 0xe6c, 0xe70, 0xe74, 0xe78, 0xe7c, 0xe80, 0xe84
	};

	uint32_t backupRegData[sizeof(backupRegAddrs)/sizeof(uint32_t)];


	/* backup BB&MAC default value */

	_DPK_parabackup(pDM_Odm, backupRegAddrs, backupRegData, sizeof(backupRegAddrs)/sizeof(uint32_t));

	/* set global parameters */
	_DPK_Globalparaset(pDM_Odm);

	/* DPK */
	_DPK_pathABDPK(pDM_Odm);

	/* TH_DPK=thermalmeter */


	/* reload BB&MAC defaul value; */
	_DPK_parareload(pDM_Odm, backupRegAddrs, backupRegData, sizeof(backupRegAddrs)/sizeof(uint32_t));

}


void PHY_DPCalibrate_8812A(struct rtl_dm *pDM_Odm)
{
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("===> PHY_DPCalibrate_8812A\n"));
	phy_DPCalibrate_8812A(pDM_Odm);
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("<=== PHY_DPCalibrate_8812A\n"));
}

#define cal_num 3

static void _rtl8821au_iqk_tx(struct rtl_priv *rtlpriv, ODM_RF_RADIO_PATH_E Path)
{
	uint32_t TX_fail, RX_fail, delay_count, IQK_ready, cal_retry, cal = 0, temp_reg65;
	int 	TX_X = 0, TX_Y = 0, RX_X = 0, RX_Y = 0, TX_Average = 0, RX_Average = 0;
	int 	TX_X0[cal_num], TX_Y0[cal_num], TX_X0_RXK[cal_num], TX_Y0_RXK[cal_num], RX_X0[cal_num], RX_Y0[cal_num];
	BOOLEAN TX0IQKOK = FALSE, RX0IQKOK = FALSE;
	BOOLEAN VDF_enable = FALSE;
	int 	i, k, VDF_Y[3], VDF_X[3], Tx_dt[3], Rx_dt[3], ii, dx = 0, dy = 0, TX_finish = 0, RX_finish = 0;

	 struct rtw_hal *pHalData = GET_HAL_DATA(rtlpriv);
	struct rtl_dm *pDM_Odm = &pHalData->odmpriv;

	ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("BandWidth = %d\n", *pDM_Odm->pBandWidth));
	if (*pDM_Odm->pBandWidth == 2) {
		VDF_enable = TRUE;
	}

	while (cal < cal_num) {
		switch (Path) {
		case ODM_RF_PATH_A:
		    {
			temp_reg65 = rtw_hal_read_rfreg(rtlpriv, Path, 0x65, bMaskDWord);

			if (pDM_Odm->ExtPA) {
				rtl_set_bbreg(rtlpriv, 0x82c, BIT(31), 0x0); /* [31] = 0 --> Page C */
				rtw_hal_write_rfreg(rtlpriv, Path, 0x65, bRFRegOffsetMask, 0x931d5);
			}

			/* Path-A LOK */
			rtl_set_bbreg(rtlpriv, 0x82c, BIT(31), 0x0); /* [31] = 0 --> Page C */
			/* ========Path-A AFE all on======== */
			/* Port 0 DAC/ADC on */
			rtw_write32(rtlpriv, 0xc60, 0x77777777);
			rtw_write32(rtlpriv, 0xc64, 0x77777777);

			rtw_write32(rtlpriv, 0xc68, 0x19791979);
			rtw_write32(rtlpriv, 0xc6c, 0x19791979);
			rtw_write32(rtlpriv, 0xc70, 0x19791979);
			rtw_write32(rtlpriv, 0xc74, 0x19791979);
			rtw_write32(rtlpriv, 0xc78, 0x19791979);
			rtw_write32(rtlpriv, 0xc7c, 0x19791979);
			rtw_write32(rtlpriv, 0xc80, 0x19791979);
			rtw_write32(rtlpriv, 0xc84, 0x19791979);

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
			rtw_write32(rtlpriv, 0x90c, 0x00008000);
			rtw_write32(rtlpriv, 0xb00, 0x03000100);
			rtl_set_bbreg(rtlpriv, 0xc94, BIT(0), 0x1);
			rtw_write32(rtlpriv, 0x978, 0x29002000);	/* TX (X,Y) */
			rtw_write32(rtlpriv, 0x97c, 0xa9002000);	/* RX (X,Y) */
			rtw_write32(rtlpriv, 0x984, 0x00462910);	/* [0]:AGC_en, [15]:idac_K_Mask */

			rtl_set_bbreg(rtlpriv, 0x82c, BIT(31), 0x1); 	/* [31] = 1 --> Page C1 */

			if (pDM_Odm->ExtPA)
				rtw_write32(rtlpriv, 0xc88, 0x821403f7);
			else
				rtw_write32(rtlpriv, 0xc88, 0x821403f4);

			if (*pDM_Odm->pBandType)
				rtw_write32(rtlpriv, 0xc8c, 0x68163e96);
			else
				rtw_write32(rtlpriv, 0xc8c, 0x28163e96);

			rtw_write32(rtlpriv, 0xc80, 0x18008c10);/* TX_Tone_idx[9:0], TxK_Mask[29] TX_Tone = 16 */
			rtw_write32(rtlpriv, 0xc84, 0x38008c10);/* RX_Tone_idx[9:0], RxK_Mask[29] */
			rtw_write32(rtlpriv, 0xcb8, 0x00100000);/* cb8[20] ±N SI/PI ¨Ï¥ÎÅv¤Áµ¹ iqk_dpk module */
			rtw_write32(rtlpriv, 0x980, 0xfa000000);
			rtw_write32(rtlpriv, 0x980, 0xf8000000);

			mdelay(10); /* Delay 10ms */
			rtw_write32(rtlpriv, 0xcb8, 0x00000000);

			rtl_set_bbreg(rtlpriv, 0x82c, BIT(31), 0x0); /* [31] = 0 --> Page C */
			rtw_hal_write_rfreg(rtlpriv, Path, 0x58, 0x7fe00, rtw_hal_read_rfreg(rtlpriv, Path, 0x8, 0xffc00)); /* Load LOK */
			switch (*pDM_Odm->pBandWidth) {
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
			rtw_write32(rtlpriv, 0x90c, 0x00008000);
			rtw_write32(rtlpriv, 0xb00, 0x03000100);
			rtl_set_bbreg(rtlpriv, 0xc94, BIT(0), 0x1);
			rtw_write32(rtlpriv, 0x978, 0x29002000);/* TX (X,Y) */
			rtw_write32(rtlpriv, 0x97c, 0xa9002000);/* RX (X,Y) */
			rtw_write32(rtlpriv, 0x984, 0x0046a910);/* [0]:AGC_en, [15]:idac_K_Mask */

			rtl_set_bbreg(rtlpriv, 0x82c, BIT(31), 0x1); /* [31] = 1 --> Page C1 */

			if (pDM_Odm->ExtPA)
				rtw_write32(rtlpriv, 0xc88, 0x821403f7);
			else
				rtw_write32(rtlpriv, 0xc88, 0x821403f1);

			if (*pDM_Odm->pBandType)
				rtw_write32(rtlpriv, 0xc8c, 0x40163e96);
			else
				rtw_write32(rtlpriv, 0xc8c, 0x00163e96);

			if (VDF_enable == 1) {
				DbgPrint("VDF_enable\n");
				for (k = 0; k <= 2; k++) {
					switch (k) {
					case 0:
						rtw_write32(rtlpriv, 0xc80, 0x18008c38);/* TX_Tone_idx[9:0], TxK_Mask[29] TX_Tone = 16 */
						rtw_write32(rtlpriv, 0xc84, 0x38008c38);/* RX_Tone_idx[9:0], RxK_Mask[29] */
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
						rtw_write32(rtlpriv, 0xc80, 0x18008c20);/* TX_Tone_idx[9:0], TxK_Mask[29] TX_Tone = 16 */
						rtw_write32(rtlpriv, 0xc84, 0x38008c20);/* RX_Tone_idx[9:0], RxK_Mask[29] */
						rtl_set_bbreg(rtlpriv, 0xce8, BIT(31), 0x1);
						rtl_set_bbreg(rtlpriv, 0xce8, 0x3fff0000, Tx_dt[cal] & 0x00003fff);
						break;
					default:
						break;
					}
					rtw_write32(rtlpriv, 0xcb8, 0x00100000);	/* cb8[20] ±N SI/PI ¨Ï¥ÎÅv¤Áµ¹ iqk_dpk module */
					cal_retry = 0;
					while (1) {
						/* one shot */
						rtw_write32(rtlpriv, 0x980, 0xfa000000);
						rtw_write32(rtlpriv, 0x980, 0xf8000000);

						mdelay(10); 	/* Delay 10ms */
						rtw_write32(rtlpriv, 0xcb8, 0x00000000);
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
								rtw_write32(rtlpriv, 0xcb8, 0x02000000);
								VDF_X[k] = rtl_get_bbreg(rtlpriv, 0xd00, 0x07ff0000)<<21;
								rtw_write32(rtlpriv, 0xcb8, 0x04000000);
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
				rtw_write32(rtlpriv, 0xc80, 0x18008c10);	/* TX_Tone_idx[9:0], TxK_Mask[29] TX_Tone = 16 */
				rtw_write32(rtlpriv, 0xc84, 0x38008c10);	/* RX_Tone_idx[9:0], RxK_Mask[29]  */
				rtw_write32(rtlpriv, 0xcb8, 0x00100000);	/* cb8[20] ±N SI/PI ¨Ï¥ÎÅv¤Áµ¹ iqk_dpk module */
				cal_retry = 0;
				while (1) {
					/* one shot */
					rtw_write32(rtlpriv, 0x980, 0xfa000000);
					rtw_write32(rtlpriv, 0x980, 0xf8000000);

					mdelay(10); /*  Delay 10ms */
					rtw_write32(rtlpriv, 0xcb8, 0x00000000);
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
							rtw_write32(rtlpriv, 0xcb8, 0x02000000);
							TX_X0[cal] = rtl_get_bbreg(rtlpriv, 0xd00, 0x07ff0000)<<21;
							rtw_write32(rtlpriv, 0xcb8, 0x04000000);
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
					rtw_write32(rtlpriv, 0x978, 0x29002000);/* TX (X,Y) */
					rtw_write32(rtlpriv, 0x97c, 0xa9002000);/* RX (X,Y) */
					rtw_write32(rtlpriv, 0x984, 0x0046a910);/* [0]:AGC_en, [15]:idac_K_Mask */
					rtw_write32(rtlpriv, 0x90c, 0x00008000);
					rtw_write32(rtlpriv, 0xb00, 0x03000100);
					rtl_set_bbreg(rtlpriv, 0x82c, BIT(31), 0x1); /* [31] = 1 --> Page C1 */
					switch (k) {
					case 0:
						rtw_write32(rtlpriv, 0xc80, 0x18008c38);	/* TX_Tone_idx[9:0], TxK_Mask[29] TX_Tone = 16 */
						rtw_write32(rtlpriv, 0xc84, 0x38008c38);	/* RX_Tone_idx[9:0], RxK_Mask[29] */
						rtl_set_bbreg(rtlpriv, 0xce8, BIT(30), 0x0);
						break;
					case 1:
						rtw_write32(rtlpriv, 0xc80, 0x08008c38);	/* TX_Tone_idx[9:0], TxK_Mask[29] TX_Tone = 16 */
						rtw_write32(rtlpriv, 0xc84, 0x28008c38);	/* RX_Tone_idx[9:0], RxK_Mask[29] */
						rtl_set_bbreg(rtlpriv, 0xce8, BIT(30), 0x0);
						break;
					case 2:
						ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("VDF_Y[1] = %x;;;VDF_Y[0] = %x\n", VDF_Y[1]>>21 & 0x00007ff, VDF_Y[0]>>21 & 0x00007ff));
						ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("VDF_X[1] = %x;;;VDF_X[0] = %x\n", VDF_X[1]>>21 & 0x00007ff, VDF_X[0]>>21 & 0x00007ff));
						Rx_dt[cal] = (VDF_Y[1]>>20)-(VDF_Y[0]>>20);
						ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("Rx_dt = %d\n", Rx_dt[cal]));
						Rx_dt[cal] = ((16*Rx_dt[cal])*10000/13823);
						Rx_dt[cal] = (Rx_dt[cal] >> 1)+(Rx_dt[cal] & BIT(0));
						rtw_write32(rtlpriv, 0xc80, 0x18008c20);	/* TX_Tone_idx[9:0], TxK_Mask[29] TX_Tone = 16 */
						rtw_write32(rtlpriv, 0xc84, 0x38008c20);	/* RX_Tone_idx[9:0], RxK_Mask[29] */
						rtl_set_bbreg(rtlpriv, 0xce8, 0x00003fff, Rx_dt[cal] & 0x00003fff);
						break;
					default:
						break;
					}

					rtw_write32(rtlpriv, 0xc88, 0x821603e0);
					rtw_write32(rtlpriv, 0xc8c, 0x68163e96);
					rtw_write32(rtlpriv, 0xcb8, 0x00100000);	/* cb8[20] ±N SI/PI ¨Ï¥ÎÅv¤Áµ¹ iqk_dpk module */
					cal_retry = 0;
					while (1) {
						/* one shot */
						rtw_write32(rtlpriv, 0x980, 0xfa000000);
						rtw_write32(rtlpriv, 0x980, 0xf8000000);

						mdelay(10); 	/* Delay 10ms */
						rtw_write32(rtlpriv, 0xcb8, 0x00000000);
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
								rtw_write32(rtlpriv, 0xcb8, 0x02000000);
								TX_X0_RXK[cal] = rtl_get_bbreg(rtlpriv, 0xd00, 0x07ff0000)<<21;
								rtw_write32(rtlpriv, 0xcb8, 0x04000000);
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
					rtw_write32(rtlpriv, 0x90c, 0x00008000);
					rtw_write32(rtlpriv, 0x984, 0x0046a911);

					rtl_set_bbreg(rtlpriv, 0x82c, BIT(31), 0x1); /* [31] = 1 --> Page C1 */
					rtl_set_bbreg(rtlpriv, 0xc80, BIT(29), 0x1);
					rtl_set_bbreg(rtlpriv, 0xc84, BIT(29), 0x0);
					rtw_write32(rtlpriv, 0xc88, 0x02140119);
					rtw_write32(rtlpriv, 0xc8c, 0x28161420);

					if (k == 2) {
						rtl_set_bbreg(rtlpriv, 0xce8, BIT(30), 0x1);  /* RX VDF Enable */
					}
					rtw_write32(rtlpriv, 0xcb8, 0x00100000);	/* cb8[20] ±N SI/PI ¨Ï¥ÎÅv¤Áµ¹ iqk_dpk module */

					cal_retry = 0;
					while (1) {
						/* one shot */
						rtw_write32(rtlpriv, 0x980, 0xfa000000);
						rtw_write32(rtlpriv, 0x980, 0xf8000000);

						mdelay(10); /* Delay 10ms */
						rtw_write32(rtlpriv, 0xcb8, 0x00000000);
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
								rtw_write32(rtlpriv, 0xcb8, 0x06000000);
								VDF_X[k] = rtl_get_bbreg(rtlpriv, 0xd00, 0x07ff0000)<<21;
								rtw_write32(rtlpriv, 0xcb8, 0x08000000);
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
				rtw_write32(rtlpriv, 0x90c, 0x00008000);
				rtw_write32(rtlpriv, 0xb00, 0x03000100);
				rtw_write32(rtlpriv, 0x984, 0x0046a910);/* [0]:AGC_en, [15]:idac_K_Mask */

				rtl_set_bbreg(rtlpriv, 0x82c, BIT(31), 0x1);	/* [31] = 1 --> Page C1 */
				rtw_write32(rtlpriv, 0xc80, 0x18008c10);	/* TX_Tone_idx[9:0], TxK_Mask[29] TX_Tone = 16 */
				rtw_write32(rtlpriv, 0xc84, 0x38008c10);	/* RX_Tone_idx[9:0], RxK_Mask[29] */
				rtw_write32(rtlpriv, 0xc88, 0x821603e0);
				/* rtw_write32(rtlpriv, 0xc8c, 0x68163e96); */
				rtw_write32(rtlpriv, 0xcb8, 0x00100000);/* cb8[20] ±N SI/PI ¨Ï¥ÎÅv¤Áµ¹ iqk_dpk module */
				cal_retry = 0;
				while (1) {
					/* one shot */
					rtw_write32(rtlpriv, 0x980, 0xfa000000);
					rtw_write32(rtlpriv, 0x980, 0xf8000000);

					mdelay(10); 	/* Delay 10ms */
					rtw_write32(rtlpriv, 0xcb8, 0x00000000);
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
							rtw_write32(rtlpriv, 0xcb8, 0x02000000);
							TX_X0_RXK[cal] = rtl_get_bbreg(rtlpriv, 0xd00, 0x07ff0000)<<21;
							rtw_write32(rtlpriv, 0xcb8, 0x04000000);
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
				rtw_write32(rtlpriv, 0x90c, 0x00008000);
				rtw_write32(rtlpriv, 0x984, 0x0046a911);

				rtl_set_bbreg(rtlpriv, 0x82c, BIT(31), 0x1); 	/* [31] = 1 --> Page C1 */
				rtw_write32(rtlpriv, 0xc80, 0x38008c10);	/* TX_Tone_idx[9:0], TxK_Mask[29] TX_Tone = 16 */
				rtw_write32(rtlpriv, 0xc84, 0x18008c10);	/* RX_Tone_idx[9:0], RxK_Mask[29] */
				rtw_write32(rtlpriv, 0xc88, 0x02140119);
				rtw_write32(rtlpriv, 0xc8c, 0x28161440);
				rtw_write32(rtlpriv, 0xcb8, 0x00100000);	/* cb8[20] ±N SI/PI ¨Ï¥ÎÅv¤Áµ¹ iqk_dpk module */

				cal_retry = 0;
				while (1) {
					/* one shot */
					rtw_write32(rtlpriv, 0x980, 0xfa000000);
					rtw_write32(rtlpriv, 0x980, 0xf8000000);

					mdelay(10); /* Delay 10ms */
					rtw_write32(rtlpriv, 0xcb8, 0x00000000);
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
							rtw_write32(rtlpriv, 0xcb8, 0x06000000);
							RX_X0[cal] = rtl_get_bbreg(rtlpriv, 0xd00, 0x07ff0000)<<21;
							rtw_write32(rtlpriv, 0xcb8, 0x08000000);
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
	case ODM_RF_PATH_A:
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
			_rtl8821au_iqk_tx_fill_iqc(pDM_Odm, Path, TX_X, TX_Y);
		} else {
			_rtl8821au_iqk_tx_fill_iqc(pDM_Odm, Path, 0x200, 0x0);
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
			 _rtl8821au_iqk_rx_fill_iqc(pDM_Odm, Path, RX_X, RX_Y);
		} else {
			 _rtl8821au_iqk_rx_fill_iqc(pDM_Odm, Path, 0x200, 0x0);
		}
	    }
		break;
	default:
		break;
	}
}



#if 0

/* from linux-master */

static void _rtl8821ae_phy_iq_calibrate(struct ieee80211_hw *hw)
{
	u32	macbb_backup[MACBB_REG_NUM];
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
	u32	backup_rf_reg[RF_REG_NUM] = {0x65, 0x8f, 0x0};

	_rtl8821ae_iqk_backup_macbb(hw, macbb_backup, backup_macbb_reg,
				    MACBB_REG_NUM);
	_rtl8821ae_iqk_backup_afe(hw, afe_backup, backup_afe_reg, AFE_REG_NUM);
	_rtl8821ae_iqk_backup_rf(hw, rfa_backup, rfb_backup, backup_rf_reg,
				 RF_REG_NUM);

	_rtl8821ae_iqk_configure_mac(hw);
	_rtl8821ae_iqk_tx(hw, RF90_PATH_A);
	_rtl8821ae_iqk_restore_rf(hw, RF90_PATH_A, backup_rf_reg, rfa_backup,
				  RF_REG_NUM);

	_rtl8821ae_iqk_restore_afe(hw, afe_backup, backup_afe_reg, AFE_REG_NUM);
	_rtl8821ae_iqk_restore_macbb(hw, macbb_backup, backup_macbb_reg,
				     MACBB_REG_NUM);
}

#endif

#undef MACBB_REG_NUM
#undef AFE_REG_NUM
#undef RF_REG_NUM 
#define MACBB_REG_NUM 11
#define AFE_REG_NUM 12
#define RF_REG_NUM 3


static void _rtl8821au_phy_iq_calibrate(struct rtl_priv *pAdapter)
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

	 struct rtw_hal *pHalData = GET_HAL_DATA(pAdapter);
	struct rtl_dm *pDM_Odm = &pHalData->odmpriv;

	_rtl8821au_iqk_backup_macbb(pAdapter, macbb_backup, backup_macbb_reg,
				    MACBB_REG_NUM);
	_rtl8821au_iqk_backup_afe(pAdapter, afe_backup, backup_afe_reg, AFE_REG_NUM);
	_rtl8821au_iqk_backup_rf(pAdapter, rfa_backup, rfb_backup, backup_rf_reg,
				 RF_REG_NUM);

	_rtl8821au_iqk_configure_mac(pAdapter);
	_rtl8821au_iqk_tx(pAdapter, ODM_RF_PATH_A);
	_rtl8821au_iqk_restore_rf(pAdapter, ODM_RF_PATH_A, backup_rf_reg, rfa_backup,
				 RF_REG_NUM);

	_rtl8821au_iqk_restore_afe(pAdapter, afe_backup, backup_afe_reg, AFE_REG_NUM);
	_rtl8821au_iqk_restore_macbb(pAdapter, macbb_backup, backup_macbb_reg,
				     MACBB_REG_NUM);

	/* _IQK_Exit_8821A(pDM_Odm); */
	/* _IQK_TX_CheckResult_8821A */
}

/*
 * from linux-master
 * void rtl8821ae_phy_iq_calibrate(struct ieee80211_hw *hw, bool b_recovery);
 */

void rtl8821au_phy_iq_calibrate(struct rtl_priv *pAdapter, BOOLEAN bReCovery)
{
	_rtl8821au_phy_iq_calibrate(pAdapter);
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


static s8 phy_GetChannelGroup(BAND_TYPE Band, uint8_t Channel)
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

static u8 PHY_GetPowerLimitValue(struct rtl_priv *Adapter, uint32_t RegPwrTblSel,
	BAND_TYPE Band, CHANNEL_WIDTH Bandwidth, RF_PATH RfPath,
	uint8_t DataRate, uint8_t Channel)
{
	 struct rtw_hal	*pHalData = GET_HAL_DATA(Adapter);
	s16				band = -1, regulation = -1, bandwidth = -1,
					rfPath = -1, rateSection = -1, channelGroup = -1;
	uint8_t				powerLimit = MAX_POWER_INDEX;

	if ((Adapter->registrypriv.RegEnableTxPowerLimit == 0 && pHalData->EEPROMRegulatory != 1)
	   || pHalData->EEPROMRegulatory == 2)
		return MAX_POWER_INDEX;

	switch (RegPwrTblSel) {
	case 1:
		regulation = TXPWR_LMT_ETSI;
		break;
	case 2:
		regulation = TXPWR_LMT_MKK;
		break;
	case 3:
		regulation = TXPWR_LMT_FCC;
		break;
	default:
		regulation = TXPWR_LMT_FCC;
		break;
	}
	/* DBG_871X("pregistrypriv->RegPwrTblSel %d\n", RegPwrTblSel); */


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

	if (BAND_ON_2_4G  && rateSection > 3)
		DBG_871X("Wrong rate 0x%x: No VHT in 2.4G Band\n", DataRate);
	if (BAND_ON_5G  && rateSection == 0)
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
		powerLimit = pHalData->TxPwrLimit_2_4G[regulation]
			[bandwidth][rateSection][channelGroup][RfPath];
	else if (Band == BAND_ON_5G)
		powerLimit = pHalData->TxPwrLimit_5G[regulation]
			[bandwidth][rateSection][channelGroup][RfPath];
	else
		DBG_871X("No power limit table of the specified band\n");

#if 0
	/* combine 5G VHT & HT rate */
	/*  5G 20M and 40M HT and VHT can cross reference */
	/*if ( Band == BAND_ON_5G && powerLimit == MAX_POWER_INDEX ) {
		if ( bandwidth == 0 || bandwidth == 1 ) {
			if ( rateSection == 2 )
				powerLimit = pHalData->TxPwrLimit_5G[regulation]
										[bandwidth][4][channelGroup][RfPath];
			else if ( rateSection == 4 )
				powerLimit = pHalData->TxPwrLimit_5G[regulation]
										[bandwidth][2][channelGroup][RfPath];
			else if ( rateSection == 3 )
				powerLimit = pHalData->TxPwrLimit_5G[regulation]
										[bandwidth][5][channelGroup][RfPath];
			else if ( rateSection == 5 )
				powerLimit = pHalData->TxPwrLimit_5G[regulation]
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

u32 phy_GetTxPwrByRateOffset_8812(struct rtl_priv *pAdapter,  uint8_t Band,
	uint8_t	Rf_Path, uint8_t Rate_Section)
{
	 struct rtw_hal	*pHalData	= GET_HAL_DATA(pAdapter);
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

	tx_pwr_diff = (pHalData->TxPwrByRateOffset[Band][Rf_Path][Rate_Section] >> shift) & 0xff;

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
VOID phy_TxPwrAdjInPercentage(struct rtl_priv *Adapter, uint8_t *pTxPwrIdx)
{
	 struct rtw_hal	*pHalData = GET_HAL_DATA(Adapter);
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

u32 PHY_GetTxPowerIndex_8812A(struct rtl_priv *pAdapter, uint8_t RFPath,
	uint8_t	Rate, CHANNEL_WIDTH BandWidth, uint8_t Channel)
{
	struct rtw_hal *	pHalData = GET_HAL_DATA(pAdapter);
	struct rtl_dm *		pDM_Odm = &pHalData->odmpriv;
	uint8_t					i = 0;	/* default set to 1S */
	struct registry_priv	*pregistrypriv = &pAdapter->registrypriv;
	uint32_t					powerDiffByRate = 0;
	uint32_t					txPower = 0;
	uint8_t					chnlIdx = (Channel-1);
	BOOLEAN				bIn24G = _FALSE;

	/* DBG_871X("===> PHY_GetTxPowerIndex_8812A\n"); */

	if (HAL_IsLegalChannel(pAdapter, Channel) == _FALSE) {
		chnlIdx = 0;
		DBG_871X("Illegal channel!!\n");
	}

	bIn24G = phy_GetChnlIndex8812A(Channel, &chnlIdx);

	/* DBG_871X("[%s] Channel Index: %d\n", (bIn24G?"2.4G":"5G"), chnlIdx); */

	if (bIn24G) { /* 3 ============================== 2.4 G ============================== */
		if (IS_CCK_RATE(Rate)) {
			txPower = pHalData->Index24G_CCK_Base[RFPath][chnlIdx];
		} else if (MGN_6M <= Rate) {
			txPower = pHalData->Index24G_BW40_Base[RFPath][chnlIdx];
		} else {
			DBG_871X("===> mpt_ProQueryCaltxPower_Jaguar: INVALID Rate.\n");
		}

		/* DBG_871X("Base Tx power(RF-%c, Rate #%d, Channel Index %d) = 0x%X\n", ((RFPath==0)?'A':'B'), Rate, chnlIdx, txPower); */

		/* OFDM-1T */
		if (MGN_6M <= Rate && Rate <= MGN_54M && !IS_CCK_RATE(Rate)) {
			txPower += pHalData->OFDM_24G_Diff[RFPath][TX_1S];
			/* DBG_871X("+PowerDiff 2.4G (RF-%c): (OFDM-1T) = (%d)\n", ((RFPath==0)?'A':'B'), pHalData->OFDM_24G_Diff[RFPath][TX_1S]); */
		}

		if (BandWidth == CHANNEL_WIDTH_20) {	/* BW20-1S, BW20-2S */
			if ((MGN_MCS0 <= Rate && Rate <= MGN_MCS15) || (MGN_VHT2SS_MCS0 <= Rate && Rate <= MGN_VHT2SS_MCS9))
				txPower += pHalData->BW20_24G_Diff[RFPath][TX_1S];
			if ((MGN_MCS8 <= Rate && Rate <= MGN_MCS15) || (MGN_VHT2SS_MCS0 <= Rate && Rate <= MGN_VHT2SS_MCS9))
				txPower += pHalData->BW20_24G_Diff[RFPath][TX_2S];

			/*
			 * DBG_871X("+PowerDiff 2.4G (RF-%c): (BW20-1S, BW20-2S) = (%d, %d)\n", ((RFPath==0)?'A':'B'),
			 * 	pHalData->BW20_24G_Diff[RFPath][TX_1S], pHalData->BW20_24G_Diff[RFPath][TX_2S]);
			 */
		} else if (BandWidth == CHANNEL_WIDTH_40) {	/* BW40-1S, BW40-2S */
			if ((MGN_MCS0 <= Rate && Rate <= MGN_MCS15) || (MGN_VHT1SS_MCS0 <= Rate && Rate <= MGN_VHT2SS_MCS9))
				txPower += pHalData->BW40_24G_Diff[RFPath][TX_1S];
			if ((MGN_MCS8 <= Rate && Rate <= MGN_MCS15) || (MGN_VHT2SS_MCS0 <= Rate && Rate <= MGN_VHT2SS_MCS9))
				txPower += pHalData->BW40_24G_Diff[RFPath][TX_2S];

			/*
			 * DBG_871X("+PowerDiff 2.4G (RF-%c): (BW40-1S, BW40-2S) = (%d, %d)\n", ((RFPath==0)?'A':'B'),
			 *	pHalData->BW40_24G_Diff[RFPath][TX_1S], pHalData->BW40_24G_Diff[RFPath][TX_2S]);
			 */
		} else if (BandWidth == CHANNEL_WIDTH_80) {	/* Willis suggest adopt BW 40M power index while in BW 80 mode */
			if ((MGN_MCS0 <= Rate && Rate <= MGN_MCS15) || (MGN_VHT1SS_MCS0 <= Rate && Rate <= MGN_VHT2SS_MCS9))
				txPower += pHalData->BW40_24G_Diff[RFPath][TX_1S];
			if ((MGN_MCS8 <= Rate && Rate <= MGN_MCS15) || (MGN_VHT2SS_MCS0 <= Rate && Rate <= MGN_VHT2SS_MCS9))
				txPower += pHalData->BW40_24G_Diff[RFPath][TX_2S];

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
			txPower = pHalData->Index5G_BW40_Base[RFPath][chnlIdx];
		} else {
			DBG_871X("===> mpt_ProQueryCalTxPower_Jaguar: INVALID Rate.\n");
		}

		/* DBG_871X("Base Tx power(RF-%c, Rate #%d, Channel Index %d) = 0x%X\n", ((RFPath==0)?'A':'B'), Rate, chnlIdx, txPower); */

		/* OFDM-1T */
		if (MGN_6M <= Rate && Rate <= MGN_54M && !IS_CCK_RATE(Rate)) {
			txPower += pHalData->OFDM_5G_Diff[RFPath][TX_1S];
			/* DBG_871X("+PowerDiff 5G (RF-%c): (OFDM-1T) = (%d)\n", ((RFPath==0)?'A':'B'), pHalData->OFDM_5G_Diff[RFPath][TX_1S]); */
		}

		if (BandWidth == CHANNEL_WIDTH_20) {	/* BW20-1S, BW20-2S */
			if ((MGN_MCS0 <= Rate && Rate <= MGN_MCS15)  || (MGN_VHT1SS_MCS0 <= Rate && Rate <= MGN_VHT2SS_MCS9))
			    txPower += pHalData->BW20_5G_Diff[RFPath][TX_1S];
			if ((MGN_MCS8 <= Rate && Rate <= MGN_MCS15) || (MGN_VHT2SS_MCS0 <= Rate && Rate <= MGN_VHT2SS_MCS9))
			    txPower += pHalData->BW20_5G_Diff[RFPath][TX_2S];

			/*
			 * DBG_871X("+PowerDiff 5G (RF-%c): (BW20-1S, BW20-2S) = (%d, %d)\n", ((RFPath==0)?'A':'B'),
			 * 	pHalData->BW20_5G_Diff[RFPath][TX_1S], pHalData->BW20_5G_Diff[RFPath][TX_2S]);
			 */
		} else if (BandWidth == CHANNEL_WIDTH_40) {	/* BW40-1S, BW40-2S */
			if ((MGN_MCS0 <= Rate && Rate <= MGN_MCS15)  || (MGN_VHT1SS_MCS0 <= Rate && Rate <= MGN_VHT2SS_MCS9))
			    txPower += pHalData->BW40_5G_Diff[RFPath][TX_1S];
			if ((MGN_MCS8 <= Rate && Rate <= MGN_MCS15) || (MGN_VHT2SS_MCS0 <= Rate && Rate <= MGN_VHT2SS_MCS9))
			    txPower += pHalData->BW40_5G_Diff[RFPath][TX_2S];

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
				txPower = pHalData->Index5G_BW80_Base[RFPath][chnlIdx] + pHalData->BW80_5G_Diff[RFPath][TX_1S];
			if ((MGN_MCS8 <= Rate && Rate <= MGN_MCS15) || (MGN_VHT2SS_MCS0 <= Rate && Rate <= MGN_VHT2SS_MCS9))
				txPower = pHalData->Index5G_BW80_Base[RFPath][chnlIdx] + pHalData->BW80_5G_Diff[RFPath][TX_1S] + pHalData->BW80_5G_Diff[RFPath][TX_2S];

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
	if (pregistrypriv->RegPwrByRate == _FALSE && pHalData->EEPROMRegulatory != 2) {
		powerDiffByRate = phy_GetTxPwrByRateOffset_8812(pAdapter, (uint8_t)(!bIn24G), RFPath, Rate);

		if ((pregistrypriv->RegEnableTxPowerLimit == 1 && pHalData->EEPROMRegulatory != 2)
		||  pHalData->EEPROMRegulatory == 1) {
			uint8_t limit = 0;
			limit = PHY_GetPowerLimitValue(pAdapter, pregistrypriv->RegPwrTblSel, (uint8_t)(!bIn24G) ? BAND_ON_5G : BAND_ON_2_4G, BandWidth, (ODM_RF_RADIO_PATH_E)RFPath, Rate, Channel);

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
		if (adapter_to_dvobj(pAdapter)->usb_speed <= RTW_USB_SPEED_2 && IS_HARDWARE_TYPE_8812AU(pAdapter)) {
			powerDiffByRate = 0;
		}

		txPower += powerDiffByRate;
	}
	/*
	 * DBG_871X("BASE ON HT MCS7\n");
	 * DBG_871X("Final Tx Power(RF-%c, Channel: %d) = %d(0x%X)\n", ((RFPath==0)?'A':'B'), chnlIdx+1, txPower, txPower);
	 */

	if (pDM_Odm->Modify_TxAGC_Flag_PathA || pDM_Odm->Modify_TxAGC_Flag_PathB) {
		/* 20130424 Mimic whether path A or B has to modify TxAGC */

		/* DBG_871X("Before add Remanant_OFDMSwingIdx[rfpath %u] %d", txPower); */
		txPower += pDM_Odm->Remnant_OFDMSwingIdx[RFPath];
		/* DBG_871X("After add Remanant_OFDMSwingIdx[rfpath %u] %d => txPower %d", RFPath, pDM_Odm->Remnant_OFDMSwingIdx[RFPath], txPower); */
	}

	if (txPower > MAX_POWER_INDEX)
		txPower = MAX_POWER_INDEX;

	/*
	 * 2012/09/26 MH We need to take care high power device limiation to prevent destroy EXT_PA.
	 * This case had ever happened in CU/SU high power module. THe limitation = 0x20.
	 * But for 8812, we still not know the value.
	 */
	phy_TxPwrAdjInPercentage(pAdapter, (uint8_t *)&txPower);

	return txPower;
}

static VOID PHY_SetPowerLimitTableValue(struct rtl_dm *pDM_Odm,
	s8 *Regulation, s8 *Band, s8 *Bandwidth, s8 *RateSection,
	s8 *RfPath, s8 *Channel, s8 *PowerLimit)
{
	struct rtl_priv *Adapter = pDM_Odm->Adapter;
	 struct rtw_hal	*pHalData = GET_HAL_DATA(Adapter);
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
		pHalData->TxPwrLimit_2_4G[regulation][bandwidth][rateSection][channelGroup][ODM_RF_PATH_A] = powerLimit;
	} else if (eqNByte(Band, "5G", 2)) {
		DBG_871X("5G Band value : [regulation %d][bw %d][rate_section %d][chnl %d][val %d]\n",
			  regulation, bandwidth, rateSection, channel, powerLimit);
		channelGroup = phy_GetChannelGroup(BAND_ON_5G, channel);
		pHalData->TxPwrLimit_5G[regulation][bandwidth][rateSection][channelGroup][ODM_RF_PATH_A] = powerLimit;
	} else {
		DBG_871X("Cannot recognize the band info in %s\n", Band);
		return;
	}
}


static void odm_ConfigBB_TXPWR_LMT_8812A(struct rtl_dm *pDM_Odm,
	u8 *Regulation, u8 *Band, u8 *Bandwidth,
	u8 *RateSection, u8 *RfPath, u8 *Channel,
	u8 *PowerLimit)
{
#if (DM_ODM_SUPPORT_TYPE & (ODM_CE))
	PHY_SetPowerLimitTableValue(pDM_Odm, Regulation, Band,
		Bandwidth, RateSection, RfPath, Channel, PowerLimit);
#endif
}


static void ODM_ReadAndConfig_MP_8812A_TXPWR_LMT(struct rtl_dm *pDM_Odm)
{
	uint32_t i		= 0;
	uint32_t ArrayLen       = RTL8812AU_TXPWR_LMT_ARRAY_LEN;
	u8 **Array		= RTL8812AU_TXPWR_LMT;

	ODM_RT_TRACE(pDM_Odm, ODM_COMP_INIT, ODM_DBG_TRACE, ("===> ODM_ReadAndConfig_MP_8812A_TXPWR_LMT\n"));

	for (i = 0; i < ArrayLen; i += 7) {
		u8 *regulation = Array[i];
		u8 *band = Array[i+1];
		u8 *bandwidth = Array[i+2];
		u8 *rate = Array[i+3];
		u8 *rfPath = Array[i+4];
		u8 *chnl = Array[i+5];
		u8 *val = Array[i+6];

		odm_ConfigBB_TXPWR_LMT_8812A(pDM_Odm, regulation, band, bandwidth, rate, rfPath, chnl, val);
	}

}


static void odm_ConfigBB_TXPWR_LMT_8821A(struct rtl_dm *pDM_Odm,
	u8 *Regulation, u8 *Band, u8 *Bandwidth,
	u8 * RateSection, u8 *RfPath, u8 *Channel,
	u8 *PowerLimit
    )
{
#if (DM_ODM_SUPPORT_TYPE & (ODM_CE))
	PHY_SetPowerLimitTableValue(pDM_Odm, Regulation, Band,
		Bandwidth, RateSection, RfPath, Channel, PowerLimit);
#endif
}


static void ODM_ReadAndConfig_MP_8821A_TXPWR_LMT(struct rtl_dm *pDM_Odm)
{
	uint32_t i		= 0;
	uint32_t ArrayLen       = RTL8821AU_TXPWR_LMT_ARRAY_LEN;
	u8 **Array		= RTL8821AU_TXPWR_LMT;

	ODM_RT_TRACE(pDM_Odm, ODM_COMP_INIT, ODM_DBG_TRACE, ("===> ODM_ReadAndConfig_MP_8821A_TXPWR_LMT\n"));

	for (i = 0; i < ArrayLen; i += 7) {
		u8 *regulation = Array[i];
		u8 *band = Array[i+1];
		u8 *bandwidth = Array[i+2];
		u8 *rate = Array[i+3];
		u8 *rfPath = Array[i+4];
		u8 *chnl = Array[i+5];
		u8 *val = Array[i+6];

		odm_ConfigBB_TXPWR_LMT_8821A(pDM_Odm, regulation, band, bandwidth, rate, rfPath, chnl, val);
	}

}

HAL_STATUS _rtl8821au_phy_read_and_config_txpwr_lmt(struct rtl_dm *pDM_Odm)
{
	if (pDM_Odm->SupportICType == ODM_RTL8812)
		ODM_ReadAndConfig_MP_8812A_TXPWR_LMT(pDM_Odm);
	if (pDM_Odm->SupportICType == ODM_RTL8821)
		ODM_ReadAndConfig_MP_8821A_TXPWR_LMT(pDM_Odm);

	return HAL_STATUS_SUCCESS;
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

static void ODM_ReadAndConfig_MP_8812A_MAC_REG(struct rtl_dm *pDM_Odm)
{
	uint32_t     hex         = 0;
	uint32_t     i           = 0;
	u16     count       = 0;
	uint32_t    *ptr_array   = NULL;
	u8     platform    = pDM_Odm->SupportPlatform;
	u8     _interface   = pDM_Odm->SupportInterface;
	u8     board       = pDM_Odm->BoardType;
	uint32_t     ArrayLen    = RTL8812AUMAC_1T_ARRAYLEN;
	uint32_t    *Array       = RTL8812AU_MAC_REG_ARRAY;


	hex += board;
	hex += _interface << 8;
	hex += platform << 16;
	hex += 0xFF000000;
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_INIT, ODM_DBG_TRACE, ("===> ODM_ReadAndConfig_MP_8812A_MAC_REG, hex = 0x%X\n", hex));

	for (i = 0; i < ArrayLen; i += 2) {
		uint32_t v1 = Array[i];
		uint32_t v2 = Array[i+1];

		/* This (offset, data) pair meets the condition. */
		if (v1 < 0xCDCDCDCD) {
			ODM_Write1Byte(pDM_Odm, v1, (u8)v2);
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
						ODM_Write1Byte(pDM_Odm, v1, (u8)v2);
						READ_NEXT_PAIR(Array, v1, v2, i);
				}

				while (v2 != 0xDEAD && i < ArrayLen - 2) {
					READ_NEXT_PAIR(Array, v1, v2, i);
				}
			}
		}
	}

}

static void ODM_ReadAndConfig_MP_8821A_MAC_REG(struct rtl_dm * pDM_Odm)
{

	uint32_t     hex         = 0;
	uint32_t     i           = 0;
	u16     count       = 0;
	uint32_t    *ptr_array   = NULL;
	u8     platform    = pDM_Odm->SupportPlatform;
	u8     _interface   = pDM_Odm->SupportInterface;
	u8     board       = pDM_Odm->BoardType;
	uint32_t     ArrayLen    = RTL8821AUMAC_1T_ARRAYLEN;
	uint32_t    *Array       = RTL8821AU_MAC_REG_ARRAY;


	hex += board;
	hex += _interface << 8;
	hex += platform << 16;
	hex += 0xFF000000;
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_INIT, ODM_DBG_TRACE, ("===> ODM_ReadAndConfig_MP_8821A_MAC_REG, hex = 0x%X\n", hex));

	for (i = 0; i < ArrayLen; i += 2) {
		uint32_t v1 = Array[i];
		uint32_t v2 = Array[i+1];

		/* This (offset, data) pair meets the condition. */
		if (v1 < 0xCDCDCDC) {
			ODM_Write1Byte(pDM_Odm, v1, (u8)v2);
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
						ODM_Write1Byte(pDM_Odm, v1, (u8)v2);
						READ_NEXT_PAIR(Array, v1, v2, i);
				}

				while (v2 != 0xDEAD && i < ArrayLen - 2) {
					READ_NEXT_PAIR(Array, v1, v2, i);
				}

			}
		}
	}

}


HAL_STATUS _rtl8821au_phy_config_mac_with_headerfile(struct rtl_dm *pDM_Odm)
{
	u8 result = HAL_STATUS_SUCCESS;

	ODM_RT_TRACE(pDM_Odm, ODM_COMP_INIT, ODM_DBG_LOUD,
		("===>ODM_ConfigMACWithHeaderFile (%s)\n", (pDM_Odm->bIsMPChip) ? "MPChip" : "TestChip"));
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_INIT, ODM_DBG_LOUD,
		("pDM_Odm->SupportPlatform: 0x%X, pDM_Odm->SupportInterface: 0x%X, pDM_Odm->BoardType: 0x%X\n",
		pDM_Odm->SupportPlatform, pDM_Odm->SupportInterface, pDM_Odm->BoardType));

#if (RTL8812A_SUPPORT == 1)
	if (pDM_Odm->SupportICType == ODM_RTL8812) {
		ODM_ReadAndConfig_MP_8812A_MAC_REG(pDM_Odm);
	}
#endif
#if (RTL8821A_SUPPORT == 1)
	if (pDM_Odm->SupportICType == ODM_RTL8821) {
		ODM_ReadAndConfig_MP_8821A_MAC_REG(pDM_Odm);
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_INIT, ODM_DBG_LOUD, ("<===8821_ODM_ConfigMACwithHeaderFile\n"));
	}
#endif
	return result;
}


/* *****  */

/******************************************************************************
*                           RadioA.TXT
******************************************************************************/

static void _rtl8812au_config_rf_radio_a(struct rtl_dm *pDM_Odm, uint32_t Addr,
	uint32_t Data)
{
	uint32_t content = 0x1000;		/* RF_Content: radioa_txt */
	uint32_t maskforPhySet = (uint32_t)(content&0xE000);

	odm_ConfigRFReg_8812A(pDM_Odm, Addr, Data, ODM_RF_PATH_A, Addr|maskforPhySet);

	ODM_RT_TRACE(pDM_Odm, ODM_COMP_INIT, ODM_DBG_TRACE, ("===> ODM_ConfigRFWithHeaderFile: [RadioA] %08X %08X\n", Addr, Data));
}

static void _rtl8812au_config_rf_radio_b(struct rtl_dm *pDM_Odm, uint32_t Addr,
	uint32_t Data)
{
	uint32_t  content = 0x1001;		/* RF_Content: radiob_txt */
	uint32_t maskforPhySet = (uint32_t)(content&0xE000);

	odm_ConfigRFReg_8812A(pDM_Odm, Addr, Data, ODM_RF_PATH_B, Addr|maskforPhySet);

	ODM_RT_TRACE(pDM_Odm, ODM_COMP_INIT, ODM_DBG_TRACE, ("===> ODM_ConfigRFWithHeaderFile: [RadioB] %08X %08X\n", Addr, Data));

}



void rtl8812au_phy_config_rf_with_headerfile(struct rtl_dm *pDM_Odm,
				ODM_RF_RADIO_PATH_E eRFPath)
{
	int i;
	bool rtstatus = true;
	u32 *radioa_array_table_a, *radioa_array_table_b;
	u16 radioa_arraylen_a, radioa_arraylen_b;
	u32 v1 = 0, v2 = 0;

	uint32_t	hex         = 0;
	u16	count       = 0;
	uint32_t	*ptr_array   = NULL;
	u8		platform    = pDM_Odm->SupportPlatform;
	u8		_interface   = pDM_Odm->SupportInterface;
	u8		board       = pDM_Odm->BoardType;

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
	case ODM_RF_PATH_A:
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
				_rtl8812au_config_rf_radio_a(pDM_Odm, v1, v2);
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
						_rtl8812au_config_rf_radio_a(pDM_Odm, v1, v2);
						READ_NEXT_PAIR(radioa_array_table_a, v1, v2, i);
					}

					while (v2 != 0xDEAD && i < radioa_arraylen_a-2) {
						READ_NEXT_PAIR(radioa_array_table_a, v1, v2, i);
					}
				}
			}
		}
	case ODM_RF_PATH_B:
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
				_rtl8812au_config_rf_radio_b(pDM_Odm, v1, v2);
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
						_rtl8812au_config_rf_radio_b(pDM_Odm, v1, v2);
						READ_NEXT_PAIR(radioa_array_table_b, v1, v2, i);
					}

					while (v2 != 0xDEAD && i < radioa_arraylen_b-2) {
						READ_NEXT_PAIR(radioa_array_table_b, v1, v2, i);
					}
				}
			}
		}

	case ODM_RF_PATH_C:
		/*
		 * RT_TRACE(rtlpriv, COMP_ERR, DBG_EMERG,
		 * 	 "switch case not process\n");
		 */
		break;
	case ODM_RF_PATH_D:
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

static void _rtl8821au_config_rf_radio_a(struct rtl_dm *pDM_Odm, uint32_t Addr,
	uint32_t Data)
{
	uint32_t  content = 0x1000;		/* RF_Content: radioa_txt */
	uint32_t maskforPhySet = (uint32_t)(content&0xE000);

	odm_ConfigRFReg_8821A(pDM_Odm, Addr, Data, ODM_RF_PATH_A, Addr|maskforPhySet);

	ODM_RT_TRACE(pDM_Odm, ODM_COMP_INIT, ODM_DBG_TRACE, ("===> ODM_ConfigRFWithHeaderFile: [RadioA] %08X %08X\n", Addr, Data));
}


void ODM_ReadAndConfig_MP_8821A_RadioA(struct rtl_dm *pDM_Odm, ODM_RF_RADIO_PATH_E eRFPath)
{
	uint32_t	hex         = 0;
	uint32_t	i           = 0;
	u16	count       = 0;
	uint32_t	*ptr_array   = NULL;
	u8		platform    = pDM_Odm->SupportPlatform;
	u8		_interface   = pDM_Odm->SupportInterface;
	u8		board       = pDM_Odm->BoardType;
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
			_rtl8821au_config_rf_radio_a(pDM_Odm, v1, v2);
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
					_rtl8821au_config_rf_radio_a(pDM_Odm, v1, v2);
					READ_NEXT_PAIR(Array, v1, v2, i);
				}

				while (v2 != 0xDEAD && i < ArrayLen-2) {
					READ_NEXT_PAIR(Array, v1, v2, i);
				}
			}
		}
	}

}

