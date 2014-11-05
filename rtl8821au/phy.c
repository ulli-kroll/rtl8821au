#include <rtl8812a_hal.h>
#include "phy.h"
//
// 1. BB register R/W API
//

u32 rtl8821au_phy_query_bb_reg(struct rtl_priv *Adapter, uint32_t RegAddr, uint32_t BitMask)
{
	uint32_t ReturnValue = 0, OriginalValue, BitShift;

#if (DISABLE_BB_RF == 1)
	return 0;
#endif

	//DBG_871X("--->PHY_QueryBBReg8812(): RegAddr(%#x), BitMask(%#x)\n", RegAddr, BitMask);


	OriginalValue = rtw_read32(Adapter, RegAddr);
	BitShift = PHY_CalculateBitShift(BitMask);
	ReturnValue = (OriginalValue & BitMask) >> BitShift;

	//DBG_871X("BBR MASK=0x%x Addr[0x%x]=0x%x\n", BitMask, RegAddr, OriginalValue);
	return (ReturnValue);
}


void rtl8821au_phy_set_bb_reg(struct rtl_priv *Adapter, u32 RegAddr, u32 BitMask, u32 Data)
{
	uint32_t OriginalValue, BitShift;

#if (DISABLE_BB_RF == 1)
	return;
#endif

	if(BitMask!= bMaskDWord)
	{//if not "double word" write
		OriginalValue = rtw_read32(Adapter, RegAddr);
		BitShift = PHY_CalculateBitShift(BitMask);
		Data = ((OriginalValue) & (~BitMask)) |( ((Data << BitShift)) & BitMask);
	}

	rtw_write32(Adapter, RegAddr, Data);

	//DBG_871X("BBW MASK=0x%x Addr[0x%x]=0x%x\n", BitMask, RegAddr, Data);
}

/* ****************************************************************************** */
/*									*/
/*  from HalPhyRf_8821A.c						*/
/*									*/
/* ****************************************************************************** */

static void  _rtl8821au_iqk_rx_fill_iqc(PDM_ODM_T pDM_Odm, ODM_RF_RADIO_PATH_E Path,
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

static void _rtl8821au_iqk_tx_fill_iqc(PDM_ODM_T pDM_Odm, ODM_RF_RADIO_PATH_E Path,
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




static void _IQK_ConfigureMAC_8821A(PDM_ODM_T pDM_Odm)
{
	/* ========MAC register setting======== */
	ODM_SetBBReg(pDM_Odm, 0x82c, BIT(31), 0x0); /* [31] = 0 --> Page C */
	ODM_Write1Byte(pDM_Odm, 0x522, 0x3f);
	ODM_SetBBReg(pDM_Odm, 0x550, BIT(3), 0x0);
	ODM_SetBBReg(pDM_Odm, 0x551, BIT(3), 0x0);
	ODM_SetBBReg(pDM_Odm, 0x808, BIT(28), 0x0);	/* CCK Off */
	ODM_Write1Byte(pDM_Odm, 0x808, 0x00);		/* RX ante off */
	ODM_SetBBReg(pDM_Odm, 0x838, 0xf, 0xc);		/* CCA off */
}

#define cal_num 3

static void _IQK_Tx_8821A(PDM_ODM_T pDM_Odm, ODM_RF_RADIO_PATH_E Path)
{
	uint32_t TX_fail, RX_fail, delay_count, IQK_ready, cal_retry, cal = 0, temp_reg65;
	int 	TX_X = 0, TX_Y = 0, RX_X = 0, RX_Y = 0, TX_Average = 0, RX_Average = 0;
	int 	TX_X0[cal_num], TX_Y0[cal_num], TX_X0_RXK[cal_num], TX_Y0_RXK[cal_num], RX_X0[cal_num], RX_Y0[cal_num];
	BOOLEAN TX0IQKOK = FALSE, RX0IQKOK = FALSE;
	BOOLEAN VDF_enable = FALSE;
	int 	i, k, VDF_Y[3], VDF_X[3], Tx_dt[3], Rx_dt[3], ii, dx = 0, dy = 0, TX_finish = 0, RX_finish = 0;


	ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("BandWidth = %d\n", *pDM_Odm->pBandWidth));
	if (*pDM_Odm->pBandWidth == 2) {
		VDF_enable = TRUE;
	}

	while (cal < cal_num) {
		switch (Path) {
		case ODM_RF_PATH_A:
		    {
			temp_reg65 = ODM_GetRFReg(pDM_Odm, Path, 0x65, bMaskDWord);

			if (pDM_Odm->ExtPA) {
				ODM_SetBBReg(pDM_Odm, 0x82c, BIT(31), 0x0); /* [31] = 0 --> Page C */
				ODM_SetRFReg(pDM_Odm, Path, 0x65, bRFRegOffsetMask, 0x931d5);
			}

			/* Path-A LOK */
			ODM_SetBBReg(pDM_Odm, 0x82c, BIT(31), 0x0); /* [31] = 0 --> Page C */
			/* ========Path-A AFE all on======== */
			/* Port 0 DAC/ADC on */
			ODM_Write4Byte(pDM_Odm, 0xc60, 0x77777777);
			ODM_Write4Byte(pDM_Odm, 0xc64, 0x77777777);

			ODM_Write4Byte(pDM_Odm, 0xc68, 0x19791979);
			ODM_Write4Byte(pDM_Odm, 0xc6c, 0x19791979);
			ODM_Write4Byte(pDM_Odm, 0xc70, 0x19791979);
			ODM_Write4Byte(pDM_Odm, 0xc74, 0x19791979);
			ODM_Write4Byte(pDM_Odm, 0xc78, 0x19791979);
			ODM_Write4Byte(pDM_Odm, 0xc7c, 0x19791979);
			ODM_Write4Byte(pDM_Odm, 0xc80, 0x19791979);
			ODM_Write4Byte(pDM_Odm, 0xc84, 0x19791979);

			ODM_SetBBReg(pDM_Odm, 0xc00, 0xf, 0x4);	/* 	hardware 3-wire off */

			/* LOK Setting */
			/* ====== LOK ====== */
			/* 1. DAC/ADC sampling rate (160 MHz) */
			ODM_SetBBReg(pDM_Odm, 0xc5c, BIT(26)|BIT(25)|BIT(24), 0x7);

			/* 2. LoK RF Setting (at BW = 20M) */
			ODM_SetRFReg(pDM_Odm, Path, 0xef, bRFRegOffsetMask, 0x80002);
			ODM_SetRFReg(pDM_Odm, Path, 0x18, 0x00c00, 0x3);     /* BW 20M */
			ODM_SetRFReg(pDM_Odm, Path, 0x30, bRFRegOffsetMask, 0x20000);
			ODM_SetRFReg(pDM_Odm, Path, 0x31, bRFRegOffsetMask, 0x0003f);
			ODM_SetRFReg(pDM_Odm, Path, 0x32, bRFRegOffsetMask, 0xf3fc3);
			ODM_SetRFReg(pDM_Odm, Path, 0x65, bRFRegOffsetMask, 0x931d5);
			ODM_SetRFReg(pDM_Odm, Path, 0x8f, bRFRegOffsetMask, 0x8a001);
			ODM_SetBBReg(pDM_Odm, 0xcb8, 0xf, 0xd);
			ODM_Write4Byte(pDM_Odm, 0x90c, 0x00008000);
			ODM_Write4Byte(pDM_Odm, 0xb00, 0x03000100);
			ODM_SetBBReg(pDM_Odm, 0xc94, BIT(0), 0x1);
			ODM_Write4Byte(pDM_Odm, 0x978, 0x29002000);	/* TX (X,Y) */
			ODM_Write4Byte(pDM_Odm, 0x97c, 0xa9002000);	/* RX (X,Y) */
			ODM_Write4Byte(pDM_Odm, 0x984, 0x00462910);	/* [0]:AGC_en, [15]:idac_K_Mask */

			ODM_SetBBReg(pDM_Odm, 0x82c, BIT(31), 0x1); 	/* [31] = 1 --> Page C1 */

			if (pDM_Odm->ExtPA)
				ODM_Write4Byte(pDM_Odm, 0xc88, 0x821403f7);
			else
				ODM_Write4Byte(pDM_Odm, 0xc88, 0x821403f4);

			if (*pDM_Odm->pBandType)
				ODM_Write4Byte(pDM_Odm, 0xc8c, 0x68163e96);
			else
				ODM_Write4Byte(pDM_Odm, 0xc8c, 0x28163e96);

			ODM_Write4Byte(pDM_Odm, 0xc80, 0x18008c10);/* TX_Tone_idx[9:0], TxK_Mask[29] TX_Tone = 16 */
			ODM_Write4Byte(pDM_Odm, 0xc84, 0x38008c10);/* RX_Tone_idx[9:0], RxK_Mask[29] */
			ODM_Write4Byte(pDM_Odm, 0xcb8, 0x00100000);/* cb8[20] ±N SI/PI ¨Ï¥ÎÅv¤Áµ¹ iqk_dpk module */
			ODM_Write4Byte(pDM_Odm, 0x980, 0xfa000000);
			ODM_Write4Byte(pDM_Odm, 0x980, 0xf8000000);

			mdelay(10); /* Delay 10ms */
			ODM_Write4Byte(pDM_Odm, 0xcb8, 0x00000000);

			ODM_SetBBReg(pDM_Odm, 0x82c, BIT(31), 0x0); /* [31] = 0 --> Page C */
			ODM_SetRFReg(pDM_Odm, Path, 0x58, 0x7fe00, ODM_GetRFReg(pDM_Odm, Path, 0x8, 0xffc00)); /* Load LOK */
			switch (*pDM_Odm->pBandWidth) {
			case 1:
				ODM_SetRFReg(pDM_Odm, Path, 0x18, 0x00c00, 0x1);
				break;
			case 2:
				ODM_SetRFReg(pDM_Odm, Path, 0x18, 0x00c00, 0x0);
				break;
			default:
				break;
			}
			ODM_SetBBReg(pDM_Odm, 0x82c, BIT(31), 0x1); /* [31] = 1 --> Page C1 */

			/* 3. TX RF Setting */
			ODM_SetBBReg(pDM_Odm, 0x82c, BIT(31), 0x0); /* [31] = 0 --> Page C */
			ODM_SetRFReg(pDM_Odm, Path, 0xef, bRFRegOffsetMask, 0x80000);
			ODM_SetRFReg(pDM_Odm, Path, 0x30, bRFRegOffsetMask, 0x20000);
			ODM_SetRFReg(pDM_Odm, Path, 0x31, bRFRegOffsetMask, 0x0003f);
			ODM_SetRFReg(pDM_Odm, Path, 0x32, bRFRegOffsetMask, 0xf3fc3);
			ODM_SetRFReg(pDM_Odm, Path, 0x65, bRFRegOffsetMask, 0x931d5);
			ODM_SetRFReg(pDM_Odm, Path, 0x8f, bRFRegOffsetMask, 0x8a001);
			ODM_SetRFReg(pDM_Odm, Path, 0xef, bRFRegOffsetMask, 0x00000);
			ODM_SetBBReg(pDM_Odm, 0xcb8, 0xf, 0xd);
			ODM_Write4Byte(pDM_Odm, 0x90c, 0x00008000);
			ODM_Write4Byte(pDM_Odm, 0xb00, 0x03000100);
			ODM_SetBBReg(pDM_Odm, 0xc94, BIT(0), 0x1);
			ODM_Write4Byte(pDM_Odm, 0x978, 0x29002000);/* TX (X,Y) */
			ODM_Write4Byte(pDM_Odm, 0x97c, 0xa9002000);/* RX (X,Y) */
			ODM_Write4Byte(pDM_Odm, 0x984, 0x0046a910);/* [0]:AGC_en, [15]:idac_K_Mask */

			ODM_SetBBReg(pDM_Odm, 0x82c, BIT(31), 0x1); /* [31] = 1 --> Page C1 */

			if (pDM_Odm->ExtPA)
				ODM_Write4Byte(pDM_Odm, 0xc88, 0x821403f7);
			else
				ODM_Write4Byte(pDM_Odm, 0xc88, 0x821403f1);

			if (*pDM_Odm->pBandType)
				ODM_Write4Byte(pDM_Odm, 0xc8c, 0x40163e96);
			else
				ODM_Write4Byte(pDM_Odm, 0xc8c, 0x00163e96);

			if (VDF_enable == 1) {
				DbgPrint("VDF_enable\n");
				for (k = 0; k <= 2; k++) {
					switch (k) {
					case 0:
						ODM_Write4Byte(pDM_Odm, 0xc80, 0x18008c38);/* TX_Tone_idx[9:0], TxK_Mask[29] TX_Tone = 16 */
						ODM_Write4Byte(pDM_Odm, 0xc84, 0x38008c38);/* RX_Tone_idx[9:0], RxK_Mask[29] */
						ODM_SetBBReg(pDM_Odm, 0xce8, BIT(31), 0x0);
						break;
					case 1:
						ODM_SetBBReg(pDM_Odm, 0xc80, BIT(28), 0x0);
						ODM_SetBBReg(pDM_Odm, 0xc84, BIT(28), 0x0);
						ODM_SetBBReg(pDM_Odm, 0xce8, BIT(31), 0x0);
						break;
					case 2:
						ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("VDF_Y[1] = %x;;;VDF_Y[0] = %x\n", VDF_Y[1]>>21 & 0x00007ff, VDF_Y[0]>>21 & 0x00007ff));
						ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("VDF_X[1] = %x;;;VDF_X[0] = %x\n", VDF_X[1]>>21 & 0x00007ff, VDF_X[0]>>21 & 0x00007ff));
						Tx_dt[cal] = (VDF_Y[1]>>20)-(VDF_Y[0]>>20);
						Tx_dt[cal] = ((16*Tx_dt[cal])*10000/15708);
						Tx_dt[cal] = (Tx_dt[cal] >> 1)+(Tx_dt[cal] & BIT(0));
						ODM_Write4Byte(pDM_Odm, 0xc80, 0x18008c20);/* TX_Tone_idx[9:0], TxK_Mask[29] TX_Tone = 16 */
						ODM_Write4Byte(pDM_Odm, 0xc84, 0x38008c20);/* RX_Tone_idx[9:0], RxK_Mask[29] */
						ODM_SetBBReg(pDM_Odm, 0xce8, BIT(31), 0x1);
						ODM_SetBBReg(pDM_Odm, 0xce8, 0x3fff0000, Tx_dt[cal] & 0x00003fff);
						break;
					default:
						break;
					}
					ODM_Write4Byte(pDM_Odm, 0xcb8, 0x00100000);	/* cb8[20] ±N SI/PI ¨Ï¥ÎÅv¤Áµ¹ iqk_dpk module */
					cal_retry = 0;
					while (1) {
						/* one shot */
						ODM_Write4Byte(pDM_Odm, 0x980, 0xfa000000);
						ODM_Write4Byte(pDM_Odm, 0x980, 0xf8000000);

						mdelay(10); 	/* Delay 10ms */
						ODM_Write4Byte(pDM_Odm, 0xcb8, 0x00000000);
						delay_count = 0;
						while (1) {
							IQK_ready = ODM_GetBBReg(pDM_Odm, 0xd00, BIT(10));
							if ((~IQK_ready) || (delay_count > 20)) {
								break;
							} else {
								mdelay(1);
								delay_count++;
							}
						}

						if (delay_count < 20) {			/* If 20ms No Result, then cal_retry++ */
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
								ODM_SetBBReg(pDM_Odm, 0xccc, 0x000007ff, 0x0);
								ODM_SetBBReg(pDM_Odm, 0xcd4, 0x000007ff, 0x200);
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
				ODM_Write4Byte(pDM_Odm, 0xc80, 0x18008c10);	/* TX_Tone_idx[9:0], TxK_Mask[29] TX_Tone = 16 */
				ODM_Write4Byte(pDM_Odm, 0xc84, 0x38008c10);	/* RX_Tone_idx[9:0], RxK_Mask[29]  */
				ODM_Write4Byte(pDM_Odm, 0xcb8, 0x00100000);	/* cb8[20] ±N SI/PI ¨Ï¥ÎÅv¤Áµ¹ iqk_dpk module */
				cal_retry = 0;
				while (1) {
					/* one shot */
					ODM_Write4Byte(pDM_Odm, 0x980, 0xfa000000);
					ODM_Write4Byte(pDM_Odm, 0x980, 0xf8000000);

					mdelay(10); /*  Delay 10ms */
					ODM_Write4Byte(pDM_Odm, 0xcb8, 0x00000000);
					delay_count = 0;
					while (1) {
						IQK_ready = ODM_GetBBReg(pDM_Odm, 0xd00, BIT(10));
						if ((~IQK_ready) || (delay_count > 20)) {
							break;
						} else {
							mdelay(1);
							delay_count++;
						}
					}

					if (delay_count < 20) {		/* If 20ms No Result, then cal_retry++ */
						/* ============TXIQK Check============== */
						TX_fail = ODM_GetBBReg(pDM_Odm, 0xd00, BIT(12));
						if (~TX_fail) {
							ODM_Write4Byte(pDM_Odm, 0xcb8, 0x02000000);
							TX_X0[cal] = ODM_GetBBReg(pDM_Odm, 0xd00, 0x07ff0000)<<21;
							ODM_Write4Byte(pDM_Odm, 0xcb8, 0x04000000);
							TX_Y0[cal] = ODM_GetBBReg(pDM_Odm, 0xd00, 0x07ff0000)<<21;
							TX0IQKOK = TRUE;
							break;
						} else {
							ODM_SetBBReg(pDM_Odm, 0xccc, 0x000007ff, 0x0);
							ODM_SetBBReg(pDM_Odm, 0xcd4, 0x000007ff, 0x200);
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
				ODM_SetBBReg(pDM_Odm, 0xce8, BIT(31), 0x0);    /* TX VDF Disable */
				ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("RXVDF Start\n"));
				for (k = 0; k <= 2; k++) {
					/* ====== RX mode TXK (RXK Step 1) ====== */
					ODM_SetBBReg(pDM_Odm, 0x82c, BIT(31), 0x0); /* [31] = 0 --> Page C */
					/*  1. TX RF Setting */
					ODM_SetRFReg(pDM_Odm, Path, 0xef, bRFRegOffsetMask, 0x80000);
					ODM_SetRFReg(pDM_Odm, Path, 0x30, bRFRegOffsetMask, 0x30000);
					ODM_SetRFReg(pDM_Odm, Path, 0x31, bRFRegOffsetMask, 0x00029);
					ODM_SetRFReg(pDM_Odm, Path, 0x32, bRFRegOffsetMask, 0xd7ffb);
					ODM_SetRFReg(pDM_Odm, Path, 0x65, bRFRegOffsetMask, temp_reg65);
					ODM_SetRFReg(pDM_Odm, Path, 0x8f, bRFRegOffsetMask, 0x8a001);
					ODM_SetRFReg(pDM_Odm, Path, 0xef, bRFRegOffsetMask, 0x00000);

					ODM_SetBBReg(pDM_Odm, 0xcb8, 0xf, 0xd);
					ODM_Write4Byte(pDM_Odm, 0x978, 0x29002000);/* TX (X,Y) */
					ODM_Write4Byte(pDM_Odm, 0x97c, 0xa9002000);/* RX (X,Y) */
					ODM_Write4Byte(pDM_Odm, 0x984, 0x0046a910);/* [0]:AGC_en, [15]:idac_K_Mask */
					ODM_Write4Byte(pDM_Odm, 0x90c, 0x00008000);
					ODM_Write4Byte(pDM_Odm, 0xb00, 0x03000100);
					ODM_SetBBReg(pDM_Odm, 0x82c, BIT(31), 0x1); /* [31] = 1 --> Page C1 */
					switch (k) {
					case 0:
						ODM_Write4Byte(pDM_Odm, 0xc80, 0x18008c38);	/* TX_Tone_idx[9:0], TxK_Mask[29] TX_Tone = 16 */
						ODM_Write4Byte(pDM_Odm, 0xc84, 0x38008c38);	/* RX_Tone_idx[9:0], RxK_Mask[29] */
						ODM_SetBBReg(pDM_Odm, 0xce8, BIT(30), 0x0);
						break;
					case 1:
						ODM_Write4Byte(pDM_Odm, 0xc80, 0x08008c38);	/* TX_Tone_idx[9:0], TxK_Mask[29] TX_Tone = 16 */
						ODM_Write4Byte(pDM_Odm, 0xc84, 0x28008c38);	/* RX_Tone_idx[9:0], RxK_Mask[29] */
						ODM_SetBBReg(pDM_Odm, 0xce8, BIT(30), 0x0);
						break;
					case 2:
						ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("VDF_Y[1] = %x;;;VDF_Y[0] = %x\n", VDF_Y[1]>>21 & 0x00007ff, VDF_Y[0]>>21 & 0x00007ff));
						ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("VDF_X[1] = %x;;;VDF_X[0] = %x\n", VDF_X[1]>>21 & 0x00007ff, VDF_X[0]>>21 & 0x00007ff));
						Rx_dt[cal] = (VDF_Y[1]>>20)-(VDF_Y[0]>>20);
						ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("Rx_dt = %d\n", Rx_dt[cal]));
						Rx_dt[cal] = ((16*Rx_dt[cal])*10000/13823);
						Rx_dt[cal] = (Rx_dt[cal] >> 1)+(Rx_dt[cal] & BIT(0));
						ODM_Write4Byte(pDM_Odm, 0xc80, 0x18008c20);	/* TX_Tone_idx[9:0], TxK_Mask[29] TX_Tone = 16 */
						ODM_Write4Byte(pDM_Odm, 0xc84, 0x38008c20);	/* RX_Tone_idx[9:0], RxK_Mask[29] */
						ODM_SetBBReg(pDM_Odm, 0xce8, 0x00003fff, Rx_dt[cal] & 0x00003fff);
						break;
					default:
						break;
					}

					ODM_Write4Byte(pDM_Odm, 0xc88, 0x821603e0);
					ODM_Write4Byte(pDM_Odm, 0xc8c, 0x68163e96);
					ODM_Write4Byte(pDM_Odm, 0xcb8, 0x00100000);	/* cb8[20] ±N SI/PI ¨Ï¥ÎÅv¤Áµ¹ iqk_dpk module */
					cal_retry = 0;
					while (1) {
						/* one shot */
						ODM_Write4Byte(pDM_Odm, 0x980, 0xfa000000);
						ODM_Write4Byte(pDM_Odm, 0x980, 0xf8000000);

						mdelay(10); 	/* Delay 10ms */
						ODM_Write4Byte(pDM_Odm, 0xcb8, 0x00000000);
						delay_count = 0;
						while (1) {
							IQK_ready = ODM_GetBBReg(pDM_Odm, 0xd00, BIT(10));
							if ((~IQK_ready) || (delay_count > 20)) {
								break;
							} else {
								mdelay(1);
								delay_count++;
							}
						}

						if (delay_count < 20) {		/* If 20ms No Result, then cal_retry++ */
							/* ============TXIQK Check============== */
							TX_fail = ODM_GetBBReg(pDM_Odm, 0xd00, BIT(12));

							if (~TX_fail) {
								ODM_Write4Byte(pDM_Odm, 0xcb8, 0x02000000);
								TX_X0_RXK[cal] = ODM_GetBBReg(pDM_Odm, 0xd00, 0x07ff0000)<<21;
								ODM_Write4Byte(pDM_Odm, 0xcb8, 0x04000000);
								TX_Y0_RXK[cal] = ODM_GetBBReg(pDM_Odm, 0xd00, 0x07ff0000)<<21;
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
					ODM_SetBBReg(pDM_Odm, 0x82c, BIT(31), 0x0); /* [31] = 0 --> Page C */
					/* 1. RX RF Setting */
					ODM_SetRFReg(pDM_Odm, Path, 0xef, bRFRegOffsetMask, 0x80000);
					ODM_SetRFReg(pDM_Odm, Path, 0x30, bRFRegOffsetMask, 0x30000);
					ODM_SetRFReg(pDM_Odm, Path, 0x31, bRFRegOffsetMask, 0x0002f);
					ODM_SetRFReg(pDM_Odm, Path, 0x32, bRFRegOffsetMask, 0xfffbb);
					ODM_SetRFReg(pDM_Odm, Path, 0x8f, bRFRegOffsetMask, 0x88001);
					ODM_SetRFReg(pDM_Odm, Path, 0x65, bRFRegOffsetMask, 0x931d8);
					ODM_SetRFReg(pDM_Odm, Path, 0xef, bRFRegOffsetMask, 0x00000);

					ODM_SetBBReg(pDM_Odm, 0x978, 0x03FF8000, (TX_X0_RXK[cal])>>21&0x000007ff);
					ODM_SetBBReg(pDM_Odm, 0x978, 0x000007FF, (TX_Y0_RXK[cal])>>21&0x000007ff);
					ODM_SetBBReg(pDM_Odm, 0x978, BIT(31), 0x1);
					ODM_SetBBReg(pDM_Odm, 0x97c, BIT(31), 0x0);
					ODM_SetBBReg(pDM_Odm, 0xcb8, 0xF, 0xe);
					ODM_Write4Byte(pDM_Odm, 0x90c, 0x00008000);
					ODM_Write4Byte(pDM_Odm, 0x984, 0x0046a911);

					ODM_SetBBReg(pDM_Odm, 0x82c, BIT(31), 0x1); /* [31] = 1 --> Page C1 */
					ODM_SetBBReg(pDM_Odm, 0xc80, BIT(29), 0x1);
					ODM_SetBBReg(pDM_Odm, 0xc84, BIT(29), 0x0);
					ODM_Write4Byte(pDM_Odm, 0xc88, 0x02140119);
					ODM_Write4Byte(pDM_Odm, 0xc8c, 0x28161420);

					if (k == 2) {
						ODM_SetBBReg(pDM_Odm, 0xce8, BIT(30), 0x1);  /* RX VDF Enable */
					}
					ODM_Write4Byte(pDM_Odm, 0xcb8, 0x00100000);	/* cb8[20] ±N SI/PI ¨Ï¥ÎÅv¤Áµ¹ iqk_dpk module */

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
							if ((~IQK_ready) || (delay_count > 20)) {
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

				if (k == 3) {
					RX_X0[cal] = VDF_X[k-1] ;
					RX_Y0[cal] = VDF_Y[k-1];
				}

				ODM_SetBBReg(pDM_Odm, 0xce8, BIT(31), 0x1);    /* TX VDF Enable */
			} else {
				/* ====== RX mode TXK (RXK Step 1) ====== */
				ODM_SetBBReg(pDM_Odm, 0x82c, BIT(31), 0x0); /* [31] = 0 --> Page C */
				/* 1. TX RF Setting */
				ODM_SetRFReg(pDM_Odm, Path, 0xef, bRFRegOffsetMask, 0x80000);
				ODM_SetRFReg(pDM_Odm, Path, 0x30, bRFRegOffsetMask, 0x30000);
				ODM_SetRFReg(pDM_Odm, Path, 0x31, bRFRegOffsetMask, 0x00029);
				ODM_SetRFReg(pDM_Odm, Path, 0x32, bRFRegOffsetMask, 0xd7ffb);
				ODM_SetRFReg(pDM_Odm, Path, 0x65, bRFRegOffsetMask, temp_reg65);
				ODM_SetRFReg(pDM_Odm, Path, 0x8f, bRFRegOffsetMask, 0x8a001);
				ODM_SetRFReg(pDM_Odm, Path, 0xef, bRFRegOffsetMask, 0x00000);
				ODM_Write4Byte(pDM_Odm, 0x90c, 0x00008000);
				ODM_Write4Byte(pDM_Odm, 0xb00, 0x03000100);
				ODM_Write4Byte(pDM_Odm, 0x984, 0x0046a910);/* [0]:AGC_en, [15]:idac_K_Mask */

				ODM_SetBBReg(pDM_Odm, 0x82c, BIT(31), 0x1);	/* [31] = 1 --> Page C1 */
				ODM_Write4Byte(pDM_Odm, 0xc80, 0x18008c10);	/* TX_Tone_idx[9:0], TxK_Mask[29] TX_Tone = 16 */
				ODM_Write4Byte(pDM_Odm, 0xc84, 0x38008c10);	/* RX_Tone_idx[9:0], RxK_Mask[29] */
				ODM_Write4Byte(pDM_Odm, 0xc88, 0x821603e0);
				/* ODM_Write4Byte(pDM_Odm, 0xc8c, 0x68163e96); */
				ODM_Write4Byte(pDM_Odm, 0xcb8, 0x00100000);/* cb8[20] ±N SI/PI ¨Ï¥ÎÅv¤Áµ¹ iqk_dpk module */
				cal_retry = 0;
				while (1) {
					/* one shot */
					ODM_Write4Byte(pDM_Odm, 0x980, 0xfa000000);
					ODM_Write4Byte(pDM_Odm, 0x980, 0xf8000000);

					mdelay(10); 	/* Delay 10ms */
					ODM_Write4Byte(pDM_Odm, 0xcb8, 0x00000000);
					delay_count = 0;
					while (1) {
						IQK_ready = ODM_GetBBReg(pDM_Odm, 0xd00, BIT(10));
						if ((~IQK_ready) || (delay_count > 20)) {
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
							TX_X0_RXK[cal] = ODM_GetBBReg(pDM_Odm, 0xd00, 0x07ff0000)<<21;
							ODM_Write4Byte(pDM_Odm, 0xcb8, 0x04000000);
							TX_Y0_RXK[cal] = ODM_GetBBReg(pDM_Odm, 0xd00, 0x07ff0000)<<21;
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
				ODM_SetBBReg(pDM_Odm, 0x82c, BIT(31), 0x0); /* [31] = 0 --> Page C */
				/* 1. RX RF Setting */
				ODM_SetRFReg(pDM_Odm, Path, 0xef, bRFRegOffsetMask, 0x80000);
				ODM_SetRFReg(pDM_Odm, Path, 0x30, bRFRegOffsetMask, 0x30000);
				ODM_SetRFReg(pDM_Odm, Path, 0x31, bRFRegOffsetMask, 0x0002f);
				ODM_SetRFReg(pDM_Odm, Path, 0x32, bRFRegOffsetMask, 0xfffbb);
				ODM_SetRFReg(pDM_Odm, Path, 0x8f, bRFRegOffsetMask, 0x88001);
				ODM_SetRFReg(pDM_Odm, Path, 0x65, bRFRegOffsetMask, 0x931d8);
				ODM_SetRFReg(pDM_Odm, Path, 0xef, bRFRegOffsetMask, 0x00000);

				ODM_SetBBReg(pDM_Odm, 0x978, 0x03FF8000, (TX_X0_RXK[cal])>>21&0x000007ff);
				ODM_SetBBReg(pDM_Odm, 0x978, 0x000007FF, (TX_Y0_RXK[cal])>>21&0x000007ff);
				ODM_SetBBReg(pDM_Odm, 0x978, BIT(31), 0x1);
				ODM_SetBBReg(pDM_Odm, 0x97c, BIT(31), 0x0);
				ODM_SetBBReg(pDM_Odm, 0xcb8, 0xF, 0xe);
				ODM_Write4Byte(pDM_Odm, 0x90c, 0x00008000);
				ODM_Write4Byte(pDM_Odm, 0x984, 0x0046a911);

				ODM_SetBBReg(pDM_Odm, 0x82c, BIT(31), 0x1); 	/* [31] = 1 --> Page C1 */
				ODM_Write4Byte(pDM_Odm, 0xc80, 0x38008c10);	/* TX_Tone_idx[9:0], TxK_Mask[29] TX_Tone = 16 */
				ODM_Write4Byte(pDM_Odm, 0xc84, 0x18008c10);	/* RX_Tone_idx[9:0], RxK_Mask[29] */
				ODM_Write4Byte(pDM_Odm, 0xc88, 0x02140119);
				ODM_Write4Byte(pDM_Odm, 0xc8c, 0x28161440);
				ODM_Write4Byte(pDM_Odm, 0xcb8, 0x00100000);	/* cb8[20] ±N SI/PI ¨Ï¥ÎÅv¤Áµ¹ iqk_dpk module */

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
						if ((~IQK_ready) || (delay_count > 20)) {
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
			if (TX0IQKOK)
				TX_Average++;
			if (RX0IQKOK)
				RX_Average++;
			ODM_SetBBReg(pDM_Odm, 0x82c, BIT(31), 0x0); 	/* [31] = 0 --> Page C */
			ODM_SetRFReg(pDM_Odm, Path, 0x65, bRFRegOffsetMask, temp_reg65);
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



static void _IQK_BackupMacBB_8821A(PDM_ODM_T pDM_Odm, uint32_t *MACBB_backup,
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

static void _IQK_BackupRF_8821A(PDM_ODM_T pDM_Odm,
	uint32_t *RFA_backup, uint32_t *RFB_backup,
	uint32_t *Backup_RF_REG, uint32_t RF_NUM)
{
	uint32_t i;

	ODM_SetBBReg(pDM_Odm, 0x82c, BIT(31), 0x0); /* [31] = 0 --> Page C */
	/* Save RF Parameters */
	for (i = 0; i < RF_NUM; i++) {
		RFA_backup[i] = ODM_GetRFReg(pDM_Odm, ODM_RF_PATH_A, Backup_RF_REG[i], bMaskDWord);
	}
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("BackupRF Success!!!!\n"));
}

static void _IQK_BackupAFE_8821A(PDM_ODM_T pDM_Odm,
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

static void _IQK_RestoreMacBB_8821A(PDM_ODM_T pDM_Odm,
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

static void _IQK_RestoreRF_8821A(PDM_ODM_T pDM_Odm,
	ODM_RF_RADIO_PATH_E  Path,uint32_t *Backup_RF_REG, uint32_t *RF_backup, uint32_t RF_REG_NUM)
{
	uint32_t i;

	ODM_SetBBReg(pDM_Odm, 0x82c, BIT(31), 0x0); /*  [31] = 0 --> Page C */
	for (i = 0; i < RF_REG_NUM; i++)
		ODM_SetRFReg(pDM_Odm, Path, Backup_RF_REG[i], bRFRegOffsetMask, RF_backup[i]);

	switch (Path) {
	case ODM_RF_PATH_A:
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("RestoreRF Path A Success!!!!\n"));
		break;
	default:
		break;
	}
}

static void _IQK_RestoreAFE_8821A(PDM_ODM_T pDM_Odm, uint32_t *AFE_backup,
	uint32_t *Backup_AFE_REG, uint32_t AFE_NUM)
{
	uint32_t i;
	ODM_SetBBReg(pDM_Odm, 0x82c, BIT(31), 0x0); /* [31] = 0 --> Page C */
	/* Reload AFE Parameters */
	for (i = 0; i < AFE_NUM; i++) {
		ODM_Write4Byte(pDM_Odm, Backup_AFE_REG[i], AFE_backup[i]);
	}
	ODM_SetBBReg(pDM_Odm, 0x82c, BIT(31), 0x1); /* [31] = 1 --> Page C1 */
	ODM_Write4Byte(pDM_Odm, 0xc80, 0x0);
	ODM_Write4Byte(pDM_Odm, 0xc84, 0x0);
	ODM_Write4Byte(pDM_Odm, 0xc88, 0x0);
	ODM_Write4Byte(pDM_Odm, 0xc8c, 0x3c000000);
	ODM_Write4Byte(pDM_Odm, 0xcb8, 0x0);
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("RestoreAFE Success!!!!\n"));
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

#define MACBB_REG_NUM 11
#define AFE_REG_NUM 12
#define RF_REG_NUM 3


static void phy_IQCalibrate_8821A(PDM_ODM_T pDM_Odm)
{
	uint32_t	MACBB_backup[MACBB_REG_NUM], AFE_backup[AFE_REG_NUM], RFA_backup[RF_REG_NUM], RFB_backup[RF_REG_NUM];
	uint32_t 	Backup_MACBB_REG[MACBB_REG_NUM] = {0xb00, 0x520, 0x550, 0x808, 0x90c, 0xc00, 0xc50, 0xe00, 0xe50, 0x838, 0x82c};
	uint32_t 	Backup_AFE_REG[AFE_REG_NUM] = {0xc5c, 0xc60, 0xc64, 0xc68, 0xc6c, 0xc70, 0xc74, 0xc78, 0xc7c, 0xc80, 0xc84, 0xcb8};
	uint32_t 	Backup_RF_REG[RF_REG_NUM] = {0x65, 0x8f, 0x0};

	_IQK_BackupMacBB_8821A(pDM_Odm, MACBB_backup, Backup_MACBB_REG, MACBB_REG_NUM);
	_IQK_BackupAFE_8821A(pDM_Odm, AFE_backup, Backup_AFE_REG, AFE_REG_NUM);
	_IQK_BackupRF_8821A(pDM_Odm, RFA_backup, RFB_backup, Backup_RF_REG, RF_REG_NUM);

	_IQK_ConfigureMAC_8821A(pDM_Odm);
	_IQK_Tx_8821A(pDM_Odm, ODM_RF_PATH_A);
	_IQK_RestoreRF_8821A(pDM_Odm, ODM_RF_PATH_A, Backup_RF_REG, RFA_backup, RF_REG_NUM);

	_IQK_RestoreAFE_8821A(pDM_Odm, AFE_backup, Backup_AFE_REG, AFE_REG_NUM);
	_IQK_RestoreMacBB_8821A(pDM_Odm, MACBB_backup, Backup_MACBB_REG, MACBB_REG_NUM);

	/* _IQK_Exit_8821A(pDM_Odm); */
	/* _IQK_TX_CheckResult_8821A */
}

void PHY_IQCalibrate_8821A(struct rtl_priv *pAdapter, BOOLEAN bReCovery)
{
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(pAdapter);

	PDM_ODM_T		pDM_Odm = &pHalData->odmpriv;

	{
		/*
		 * if(pMgntInfo->RegIQKFWOffload)
		 * 	phy_IQCalibrate_By_FW_8821A(pAdapter);
		 * else
		 */
			phy_IQCalibrate_8821A(pDM_Odm);
	}
}

