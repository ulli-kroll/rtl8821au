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
#define _HCI_HAL_INIT_C_

#include <rtl8812a_hal.h>
#include <../rtl8821au/phy.h>
#include <../rtl8821au/reg.h>
#include <../rtl8821au/trx.h>
#include <../rtl8821au/hw.h>
#include <../rtl8821au/fw.h>
#include <rtw_debug.h>
#include <hal_intf.h>

static void _dbg_dump_macreg(struct rtl_priv *rtlpriv)
{
	uint32_t offset = 0;
	uint32_t val32 = 0;
	uint32_t index = 0;

	for (index = 0; index < 64; index++) {
		offset = index*4;
		val32 = rtl_read_dword(rtlpriv, offset);
		DBG_8192C("offset : 0x%02x ,val:0x%08x\n", offset, val32);
	}
}

static void _InitBurstPktLen(IN struct rtl_priv *rtlpriv)
{
	struct rtl_usb	*rtlusb = rtl_usbdev(rtlpriv);
	struct rtl_hal *rtlhal = rtl_hal(rtlpriv);
	u8 speedvalue, provalue, temp;
 	struct _rtw_hal	*pHalData = GET_HAL_DATA(rtlpriv);

	/*
	 * rtl_write_word(rtlpriv, REG_TRXDMA_CTRL_8195, 0xf5b0);
	 * rtl_write_word(rtlpriv, REG_TRXDMA_CTRL_8812, 0xf5b4);
	 */
	rtl_write_byte(rtlpriv, 0xf050, 0x01);		/* usb3 rx interval */
	rtl_write_word(rtlpriv, REG_RXDMA_STATUS, 0x7400);	/* burset lenght=4, set 0x3400 for burset length=2 */
	rtl_write_byte(rtlpriv, 0x289, 0xf5);		/* for rxdma control */
	/* rtl_write_byte(rtlpriv, 0x3a, 0x46); */

	/*  0x456 = 0x70, sugguested by Zhilin */
	rtl_write_byte(rtlpriv, REG_AMPDU_MAX_TIME_8812, 0x70);

	rtl_write_dword(rtlpriv, 0x458, 0xffffffff);
	rtl_write_byte(rtlpriv, REG_USTIME_TSF, 0x50);
	rtl_write_byte(rtlpriv, REG_USTIME_EDCA, 0x50);

	if (IS_HARDWARE_TYPE_8821U(rtlhal))
		speedvalue = BIT7;
	else
		speedvalue = rtl_read_byte(rtlpriv, 0xff); /* check device operation speed: SS 0xff bit7 */

	if (speedvalue & BIT7) {		/* USB2/1.1 Mode */
		temp = rtl_read_byte(rtlpriv, 0xfe17);
		if (((temp >> 4) & 0x03) == 0) {
			rtlusb->max_bulk_out_size = USB_HIGH_SPEED_BULK_SIZE;
			provalue = rtl_read_byte(rtlpriv, REG_RXDMA_PRO_8812);
			rtl_write_byte(rtlpriv, REG_RXDMA_PRO_8812, ((provalue|BIT(4))&(~BIT(5)))); /* set burst pkt len=512B */
			rtl_write_word(rtlpriv, REG_RXDMA_PRO_8812, 0x1e);
		} else {
			rtlusb->max_bulk_out_size = USB_FULL_SPEED_BULK_SIZE;
			provalue = rtl_read_byte(rtlpriv, REG_RXDMA_PRO_8812);
			rtl_write_byte(rtlpriv, REG_RXDMA_PRO_8812, ((provalue|BIT(5))&(~BIT(4)))); /* set burst pkt len=64B */
		}

		rtl_write_word(rtlpriv, REG_RXDMA_AGG_PG_TH, 0x2005); /* dmc agg th 20K */

		/*
		 * rtl_write_byte(rtlpriv, 0x10c, 0xb4);
		 * hal_UphyUpdate8812AU(rtlpriv);
		 */

		pHalData->bSupportUSB3 = _FALSE;
	} else {		/* USB3 Mode */
		rtlusb->max_bulk_out_size = USB_SUPER_SPEED_BULK_SIZE;
		provalue = rtl_read_byte(rtlpriv, REG_RXDMA_PRO_8812);
		rtl_write_byte(rtlpriv, REG_RXDMA_PRO_8812, provalue&(~(BIT5|BIT4))); /* set burst pkt len=1k */
		rtl_write_word(rtlpriv, REG_RXDMA_PRO_8812, 0x0e);
		pHalData->bSupportUSB3 = _TRUE;

		/*  set Reg 0xf008[3:4] to 2'00 to disable U1/U2 Mode to avoid 2.5G spur in USB3.0. added by page, 20120712 */
		rtl_write_byte(rtlpriv, 0xf008, rtl_read_byte(rtlpriv, 0xf008)&0xE7);
	}

#ifdef CONFIG_USB_TX_AGGREGATION
	/* rtl_write_byte(rtlpriv, REG_TDECTRL_8195, 0x30); */
#else
	rtl_write_byte(rtlpriv, REG_TDECTRL, 0x10);
#endif

	temp = rtl_read_byte(rtlpriv, REG_SYS_FUNC_EN);
	rtl_write_byte(rtlpriv, REG_SYS_FUNC_EN, temp&(~BIT(10))); 	/* reset 8051 */

	rtl_write_byte(rtlpriv, REG_HT_SINGLE_AMPDU_8812, rtl_read_byte(rtlpriv, REG_HT_SINGLE_AMPDU_8812)|BIT(7));	/* enable single pkt ampdu */
	rtl_write_byte(rtlpriv, REG_RX_PKT_LIMIT, 0x18);		/* for VHT packet length 11K */

	rtl_write_byte(rtlpriv, REG_PIFS, 0x00);

	/* Suggention by SD1 Jong and Pisa, by Maddest 20130107. */
	if (IS_HARDWARE_TYPE_8821U(rtlhal)) {
		rtl_write_word(rtlpriv, REG_MAX_AGGR_NUM, 0x0a0a);
		rtl_write_byte(rtlpriv, REG_FWHW_TXQ_CTRL, 0x80);
		rtl_write_byte(rtlpriv, REG_AMPDU_MAX_TIME_8812, 0x5e);
		rtl_write_dword(rtlpriv, REG_FAST_EDCA_CTRL, 0x03087777);
	} else {
		rtl_write_byte(rtlpriv, REG_MAX_AGGR_NUM, 0x1f);
		rtl_write_byte(rtlpriv, REG_FWHW_TXQ_CTRL, rtl_read_byte(rtlpriv, REG_FWHW_TXQ_CTRL)&(~BIT(7)));
	}

	if (pHalData->AMPDUBurstMode)
		rtl_write_byte(rtlpriv, REG_AMPDU_BURST_MODE_8812, 0x5F);

	rtl_write_byte(rtlpriv, 0x1c, rtl_read_byte(rtlpriv, 0x1c) | BIT(5) | BIT(6));  /* to prevent mac is reseted by bus. 20111208, by Page */

	/* ARFB table 9 for 11ac 5G 2SS */
	rtl_write_dword(rtlpriv, REG_ARFR0, 0x00000010);
	if (IS_NORMAL_CHIP(rtlhal->version))
		rtl_write_dword(rtlpriv, REG_ARFR0+4, 0xfffff000);
	else
		rtl_write_dword(rtlpriv, REG_ARFR0+4, 0x3e0ff000);

	/* ARFB table 10 for 11ac 5G 1SS */
	rtl_write_dword(rtlpriv, REG_ARFR1, 0x00000010);
	if (IS_VENDOR_8812A_TEST_CHIP(rtlhal->version))
		rtl_write_dword(rtlpriv, REG_ARFR1_8812+4, 0x000ff000);
	else
		rtl_write_dword(rtlpriv, REG_ARFR1_8812+4, 0x003ff000);

}

static uint32_t _InitPowerOn8812AU(struct rtl_priv *rtlpriv)
{
	u16	u2btmp = 0;
	uint8_t	u1btmp = 0;
	struct rtl_hal *rtlhal = rtl_hal(rtlpriv);

	if (IS_VENDOR_8821A_MP_CHIP(rtlhal->version)) {
		/* HW Power on sequence */
		if (!HalPwrSeqCmdParsing(rtlpriv, PWR_CUT_A_MSK, PWR_FAB_ALL_MSK, PWR_INTF_USB_MSK, Rtl8821A_NIC_ENABLE_FLOW)) {
			DBG_871X(KERN_ERR "%s: run power on flow fail\n", __func__);
			return _FAIL;
		}
	} else if (IS_HARDWARE_TYPE_8821U(rtlhal)) {
		if (!HalPwrSeqCmdParsing(rtlpriv, PWR_CUT_TESTCHIP_MSK, PWR_FAB_ALL_MSK, PWR_INTF_USB_MSK, Rtl8821A_NIC_ENABLE_FLOW)) {
			DBG_871X(KERN_ERR "%s: run power on flow fail\n", __func__);
			return _FAIL;
		}
	} else {
		if (!HalPwrSeqCmdParsing(rtlpriv, PWR_CUT_ALL_MSK, PWR_FAB_ALL_MSK, PWR_INTF_USB_MSK, Rtl8812_NIC_ENABLE_FLOW)) {
			DBG_871X(KERN_ERR "%s: run power on flow fail\n", __func__);
			return _FAIL;
		}
	}

	/*
	 *  Enable MAC DMA/WMAC/SCHEDULE/SEC block
	 * Set CR bit10 to enable 32k calibration. Suggested by SD1 Gimmy. Added by tynli. 2011.08.31.
	 */
	rtl_write_word(rtlpriv, REG_CR, 0x00); 	/* suggseted by zhouzhou, by page, 20111230 */
	u2btmp = rtl_read_word(rtlpriv, REG_CR);
	u2btmp |= (HCI_TXDMA_EN | HCI_RXDMA_EN | TXDMA_EN | RXDMA_EN
				| PROTOCOL_EN | SCHEDULE_EN | ENSEC | CALTMR_EN);
	rtl_write_word(rtlpriv, REG_CR, u2btmp);

	/*
	 * Need remove below furture, suggest by Jackie.
	 * if 0xF0[24] =1 (LDO), need to set the 0x7C[6] to 1.
	 */
	if (IS_HARDWARE_TYPE_8821U(rtlhal)) {
		u1btmp = rtl_read_byte(rtlpriv, REG_SYS_CFG+3);
		if (u1btmp & BIT0) { 	/* LDO mode. */
			u1btmp = rtl_read_byte(rtlpriv, 0x7c);
			/* ULLI unknown register */
			rtl_write_byte(rtlpriv, 0x7c, u1btmp | BIT6);
		}
	}

	return _SUCCESS;
}





/*
---------------------------------------------------------------

	MAC init functions

---------------------------------------------------------------
*/

/* Shall USB interface init this? */
static void rtl8821au_enable_interrupt(struct rtl_priv *rtlpriv)
{
	struct rtl_usb *rtlusb = rtl_usbdev(rtlpriv);

	/* HIMR */
	rtl_write_dword(rtlpriv, REG_HIMR0_8812, rtlusb->irq_mask[0]&0xFFFFFFFF);
	rtl_write_dword(rtlpriv, REG_HIMR1_8812, rtlusb->irq_mask[1]&0xFFFFFFFF);
}

static void _InitQueueReservedPage_8821AUsb(struct rtl_priv *rtlpriv)
{
	struct _rtw_hal *pHalData = GET_HAL_DATA(rtlpriv);
	struct registry_priv *pregistrypriv = &rtlpriv->registrypriv;
	uint32_t numHQ = 0;
	uint32_t numLQ = 0;
	uint32_t numNQ = 0;
	uint32_t numPubQ = 0;
	uint32_t value32;
	uint8_t	 value8;

	numPubQ = NORMAL_PAGE_NUM_PUBQ_8821;

	if (pHalData->OutEpQueueSel & TX_SELE_HQ)
		numHQ = NORMAL_PAGE_NUM_HPQ_8821;

	if (pHalData->OutEpQueueSel & TX_SELE_LQ)
		numLQ = NORMAL_PAGE_NUM_LPQ_8821;

	/* NOTE: This step shall be proceed before writting REG_RQPN. */
	if (pHalData->OutEpQueueSel & TX_SELE_NQ)
		numNQ = NORMAL_PAGE_NUM_NPQ_8821;


	value8 = (u8)_NPQ(numNQ);
	rtl_write_byte(rtlpriv, REG_RQPN_NPQ, value8);

	/* TX DMA */
	value32 = _HPQ(numHQ) | _LPQ(numLQ) | _PUBQ(numPubQ) | LD_RQPN;
	rtl_write_dword(rtlpriv, REG_RQPN, value32);
}

static void _InitQueueReservedPage_8812AUsb(struct rtl_priv *rtlpriv)
{
	struct _rtw_hal *pHalData = GET_HAL_DATA(rtlpriv);
	struct registry_priv	*pregistrypriv = &rtlpriv->registrypriv;
	uint32_t numHQ		= 0;
	uint32_t numLQ		= 0;
	uint32_t numNQ		= 0;
	uint32_t numPubQ	= 0;
	uint32_t value32;
	uint8_t	value8;

	numPubQ = NORMAL_PAGE_NUM_PUBQ_8812;

	if (pHalData->OutEpQueueSel & TX_SELE_HQ)
		numHQ = NORMAL_PAGE_NUM_HPQ_8812;

	if (pHalData->OutEpQueueSel & TX_SELE_LQ)
		numLQ = NORMAL_PAGE_NUM_LPQ_8812;

	/* NOTE: This step shall be proceed before writting REG_RQPN. */
	if (pHalData->OutEpQueueSel & TX_SELE_NQ)
		numNQ = NORMAL_PAGE_NUM_NPQ_8812;

	value8 = (u8)_NPQ(numNQ);
	rtl_write_byte(rtlpriv, REG_RQPN_NPQ, value8);

	/* TX DMA */
	value32 = _HPQ(numHQ) | _LPQ(numLQ) | _PUBQ(numPubQ) | LD_RQPN;
	rtl_write_dword(rtlpriv, REG_RQPN, value32);
}

static void _InitID_8812A(IN  struct rtl_priv *rtlpriv)
{
	hal_init_macaddr(rtlpriv);	/* set mac_address */
}

static void _InitTxBufferBoundary_8821AUsb(struct rtl_priv *rtlpriv)
{
	uint8_t	txpktbuf_bndy;

	txpktbuf_bndy = TX_PAGE_BOUNDARY_8821;

	rtl_write_byte(rtlpriv, REG_BCNQ_BDNY, txpktbuf_bndy);
	rtl_write_byte(rtlpriv, REG_MGQ_BDNY, txpktbuf_bndy);
	rtl_write_byte(rtlpriv, REG_WMAC_LBK_BF_HD, txpktbuf_bndy);
	rtl_write_byte(rtlpriv, REG_TRXFF_BNDY, txpktbuf_bndy);
	rtl_write_byte(rtlpriv, REG_TDECTRL+1, txpktbuf_bndy);
}

static void _InitTxBufferBoundary_8812AUsb(struct rtl_priv *rtlpriv)
{
	uint8_t	txpktbuf_bndy;

	txpktbuf_bndy = TX_PAGE_BOUNDARY_8812;

	rtl_write_byte(rtlpriv, REG_BCNQ_BDNY, txpktbuf_bndy);
	rtl_write_byte(rtlpriv, REG_MGQ_BDNY, txpktbuf_bndy);
	rtl_write_byte(rtlpriv, REG_WMAC_LBK_BF_HD, txpktbuf_bndy);
	rtl_write_byte(rtlpriv, REG_TRXFF_BNDY, txpktbuf_bndy);
	rtl_write_byte(rtlpriv, REG_TDECTRL+1, txpktbuf_bndy);
}

static void _InitPageBoundary_8812AUsb(struct rtl_priv *rtlpriv)
{
	struct rtl_hal *rtlhal = rtl_hal(rtlpriv);
	/*
	 * u16 			rxff_bndy;
	 * u16			Offset;
	 * BOOLEAN			bSupportRemoteWakeUp;
	 */

	/*
	 * rtlpriv->cfg->ops.GetHalDefVarHandler(rtlpriv, HAL_DEF_WOWLAN , &bSupportRemoteWakeUp);
	 * RX Page Boundary
	 * srand(static_cast<unsigned int>(time(NULL)) );
	 */

	/*
	 * Offset = MAX_RX_DMA_BUFFER_SIZE_8812/256;
	 * rxff_bndy = (Offset*256)-1;
	 */

	if (IS_HARDWARE_TYPE_8812(rtlhal))
		rtl_write_word(rtlpriv, (REG_TRXFF_BNDY + 2), MAX_RX_DMA_BUFFER_SIZE_8812-1);
	else
		rtl_write_word(rtlpriv, (REG_TRXFF_BNDY + 2), MAX_RX_DMA_BUFFER_SIZE_8821-1);

}


static void _InitNormalChipRegPriority_8812AUsb(struct rtl_priv *rtlpriv,
	u16 beQ, u16 bkQ, u16 viQ,
	u16 voQ, u16 mgtQ, u16 hiQ)
{
	u16 value16 = (rtl_read_word(rtlpriv, REG_TRXDMA_CTRL) & 0x7);

	value16 |= _TXDMA_BEQ_MAP(beQ) 	| _TXDMA_BKQ_MAP(bkQ) |
		   _TXDMA_VIQ_MAP(viQ) 	| _TXDMA_VOQ_MAP(voQ) |
		   _TXDMA_MGQ_MAP(mgtQ) | _TXDMA_HIQ_MAP(hiQ);

	rtl_write_word(rtlpriv, REG_TRXDMA_CTRL, value16);
}

static void _InitNormalChipTwoOutEpPriority_8812AUsb(struct rtl_priv *rtlpriv)
{
	 struct _rtw_hal	*pHalData	= GET_HAL_DATA(rtlpriv);
	struct registry_priv *pregistrypriv = &rtlpriv->registrypriv;
	u16	beQ, bkQ, viQ, voQ, mgtQ, hiQ;

	u16	valueHi = 0;
	u16	valueLow = 0;

	switch (pHalData->OutEpQueueSel) {
	case (TX_SELE_HQ | TX_SELE_LQ):
		valueHi = QUEUE_HIGH;
		valueLow = QUEUE_LOW;
		break;
	case (TX_SELE_NQ | TX_SELE_LQ):
		valueHi = QUEUE_NORMAL;
		valueLow = QUEUE_LOW;
		break;
	case (TX_SELE_HQ | TX_SELE_NQ):
		valueHi = QUEUE_HIGH;
		valueLow = QUEUE_NORMAL;
		break;
	default:
		valueHi = QUEUE_HIGH;
		valueLow = QUEUE_NORMAL;
		break;
	}

		beQ	= valueLow;
		bkQ	= valueLow;
		viQ	= valueHi;
		voQ	= valueHi;
		mgtQ	= valueHi;
		hiQ	= valueHi;

	_InitNormalChipRegPriority_8812AUsb(rtlpriv, beQ, bkQ, viQ, voQ, mgtQ, hiQ);

}

static void _InitNormalChipThreeOutEpPriority_8812AUsb(struct rtl_priv *rtlpriv)
{
	struct registry_priv *pregistrypriv = &rtlpriv->registrypriv;
	u16	beQ, bkQ, viQ, voQ, mgtQ, hiQ;

		beQ	= QUEUE_LOW;
		bkQ	= QUEUE_LOW;
		viQ	= QUEUE_NORMAL;
		voQ	= QUEUE_HIGH;
		mgtQ 	= QUEUE_HIGH;
		hiQ	= QUEUE_HIGH;

	_InitNormalChipRegPriority_8812AUsb(rtlpriv, beQ, bkQ, viQ, voQ, mgtQ, hiQ);
}

static void _InitQueuePriority_8812AUsb(struct rtl_priv *rtlpriv)
{
	 struct _rtw_hal	*pHalData	= GET_HAL_DATA(rtlpriv);

	switch (pHalData->OutEpNumber) {
	case 2:
		_InitNormalChipTwoOutEpPriority_8812AUsb(rtlpriv);
		break;
	case 3:
	case 4:
		_InitNormalChipThreeOutEpPriority_8812AUsb(rtlpriv);
		break;
	default:
		DBG_871X("_InitQueuePriority_8812AUsb(): Shall not reach here!\n");
		break;
	}
}



static void _InitHardwareDropIncorrectBulkOut_8812A(struct rtl_priv *rtlpriv)
{
	uint32_t value32 = rtl_read_dword(rtlpriv, REG_TXDMA_OFFSET_CHK);
	value32 |= DROP_DATA_EN;
	rtl_write_dword(rtlpriv, REG_TXDMA_OFFSET_CHK, value32);
}

static void _InitNetworkType_8812A(struct rtl_priv *rtlpriv)
{
	uint32_t	value32;

	value32 = rtl_read_dword(rtlpriv, REG_CR);
	/*  TODO: use the other function to set network type */
	value32 = (value32 & ~MASK_NETTYPE) | _NETTYPE(NT_LINK_AP);

	rtl_write_dword(rtlpriv, REG_CR, value32);
}

static void _InitTransferPageSize_8812AUsb(struct rtl_priv *rtlpriv)
{
	uint8_t	value8;
	value8 = _PSTX(PBP_512);

	rtl_write_byte(rtlpriv, REG_PBP, value8);
}

static void _InitDriverInfoSize_8812A(struct rtl_priv *rtlpriv, uint8_t	drvInfoSize)
{
	rtl_write_byte(rtlpriv, REG_RX_DRVINFO_SZ, drvInfoSize);
}

static void _InitWMACSetting_8812A(struct rtl_priv *rtlpriv)
{
	struct rtl_hal *rtlhal = rtl_hal(rtlpriv);
	/* uint32_t			value32; */
	/* u16			value16; */
	 struct _rtw_hal	*pHalData = GET_HAL_DATA(rtlpriv);

	/*
	 * pHalData->ReceiveConfig = AAP | APM | AM | AB | APP_ICV | ADF | AMF | APP_FCS | HTC_LOC_CTRL | APP_MIC | APP_PHYSTS;
	 */
	pHalData->ReceiveConfig = RCR_APM | RCR_AM | RCR_AB | RCR_CBSSID_DATA
				| RCR_CBSSID_BCN | RCR_APP_ICV | RCR_AMF
				| RCR_HTC_LOC_CTRL | RCR_APP_MIC
				| RCR_APP_PHYST_RXFF;

#if (1 == RTL8812A_RX_PACKET_INCLUDE_CRC)
	pHalData->ReceiveConfig |= ACRC32;
#endif

	pHalData->ReceiveConfig |= FORCEACK;

	/*
	 *  some REG_RCR will be modified later by phy_ConfigMACWithHeaderFile()
	 */
	rtl_write_dword(rtlpriv, REG_RCR, pHalData->ReceiveConfig);

	/* Accept all multicast address */
	rtl_write_dword(rtlpriv, REG_MAR, 0xFFFFFFFF);
	rtl_write_dword(rtlpriv, REG_MAR + 4, 0xFFFFFFFF);


	/*
	 *  Accept all data frames
	 * value16 = 0xFFFF;
	 * rtl_write_word(rtlpriv, REG_RXFLTMAP2, value16);
	 */

	/*
	 * 2010.09.08 hpfan
	 * Since ADF is removed from RCR, ps-poll will not be indicate to driver,
	 * RxFilterMap should mask ps-poll to gurantee AP mode can rx ps-poll.
	 * value16 = 0x400;
	 * rtl_write_word(rtlpriv, REG_RXFLTMAP1, value16);
	 */

	/*
	 *  Accept all management frames
	 * value16 = 0xFFFF;
	 * rtl_write_word(rtlpriv, REG_RXFLTMAP0, value16);
	 */

	/*
	 * enable RX_SHIFT bits
	 * rtl_write_byte(rtlpriv, REG_TRXDMA_CTRL, rtl_read_byte(rtlpriv, REG_TRXDMA_CTRL)|BIT(1));
	 */

}

static void _InitAdaptiveCtrl_8812AUsb(IN struct rtl_priv *rtlpriv)
{
	u16	value16;
	uint32_t	value32;

	/* Response Rate Set */
	value32 = rtl_read_dword(rtlpriv, REG_RRSR);
	value32 &= ~RATE_BITMAP_ALL;

	if (rtlpriv->registrypriv.wireless_mode & WIRELESS_11B)
		value32 |= RATE_RRSR_CCK_ONLY_1M;
	else
		value32 |= RATE_RRSR_WITHOUT_CCK;

	value32 |= RATE_RRSR_CCK_ONLY_1M;
	rtl_write_dword(rtlpriv, REG_RRSR, value32);

	/*
	 * CF-END Threshold
	 * m_spIoBase->rtl_write_byte(REG_CFEND_TH, 0x1);
	 */

	/* SIFS (used in NAV) */
	value16 = _SPEC_SIFS_CCK(0x10) | _SPEC_SIFS_OFDM(0x10);
	rtl_write_word(rtlpriv, REG_SPEC_SIFS, value16);

	/* Retry Limit */
	value16 = _LRL(0x30) | _SRL(0x30);
	rtl_write_word(rtlpriv, REG_RL, value16);

}

static void _InitEDCA_8812AUsb(struct rtl_priv *rtlpriv)
{
	/* Set Spec SIFS (used in NAV) */
	rtl_write_word(rtlpriv, REG_SPEC_SIFS, 0x100a);
	rtl_write_word(rtlpriv, REG_MAC_SPEC_SIFS, 0x100a);

	/* Set SIFS for CCK */
	rtl_write_word(rtlpriv, REG_SIFS_CTX, 0x100a);

	/* Set SIFS for OFDM */
	rtl_write_word(rtlpriv, REG_SIFS_TRX, 0x100a);

	/* TXOP */
	rtl_write_dword(rtlpriv, REG_EDCA_BE_PARAM, 0x005EA42B);
	rtl_write_dword(rtlpriv, REG_EDCA_BK_PARAM, 0x0000A44F);
	rtl_write_dword(rtlpriv, REG_EDCA_VI_PARAM, 0x005EA324);
	rtl_write_dword(rtlpriv, REG_EDCA_VO_PARAM, 0x002FA226);

	/* 0x50 for 80MHz clock */
	rtl_write_byte(rtlpriv, REG_USTIME_TSF, 0x50);
	rtl_write_byte(rtlpriv, REG_USTIME_EDCA, 0x50);
}


static void _InitBeaconMaxError_8812A(struct rtl_priv *rtlpriv, BOOLEAN	InfraMode)
{
	/* ULLI: looks here is some hacking done, wrong nams ?? */
#ifdef RTL8192CU_ADHOC_WORKAROUND_SETTING
	rtl_write_byte(rtlpriv, REG_BCN_MAX_ERR, 0xFF);
#else
	/* rtl_write_byte(rtlpriv, REG_BCN_MAX_ERR, (InfraMode ? 0xFF : 0x10)); */
#endif
}

static void _InitRDGSetting_8812A(struct rtl_priv *rtlpriv)
{
	rtl_write_byte(rtlpriv, REG_RD_CTRL, 0xFF);
	rtl_write_word(rtlpriv, REG_RD_NAV_NXT, 0x200);
	rtl_write_byte(rtlpriv, REG_RD_RESP_PKT_TH, 0x05);
}

static void _InitRxSetting_8812AU(struct rtl_priv *rtlpriv)
{
	rtl_write_dword(rtlpriv, REG_MACID, 0x87654321);
	/* ULLI unknown register */
	rtl_write_dword(rtlpriv, 0x0700, 0x87654321);
}

static void _InitRetryFunction_8812A(IN  struct rtl_priv *rtlpriv)
{
	uint8_t	value8;

	value8 = rtl_read_byte(rtlpriv, REG_FWHW_TXQ_CTRL);
	value8 |= EN_AMPDU_RTY_NEW;
	rtl_write_byte(rtlpriv, REG_FWHW_TXQ_CTRL, value8);

	/*
	 * Set ACK timeout
	 * rtl_write_byte(rtlpriv, REG_ACKTO, 0x40);  //masked by page for BCM IOT issue temporally
	 */
	rtl_write_byte(rtlpriv, REG_ACKTO, 0x80);
}

/*-----------------------------------------------------------------------------
 * Function:	usb_AggSettingTxUpdate()
 *
 * Overview:	Seperate TX/RX parameters update independent for TP detection and
 *			dynamic TX/RX aggreagtion parameters update.
 *
 * Input:			_ADAPTER
 *
 * Output/Return:	NONE
 *
 * Revised History:
 *	When		Who		Remark
 *	12/10/2010	MHC		Seperate to smaller function.
 *
 *---------------------------------------------------------------------------*/
static void usb_AggSettingTxUpdate_8812A(struct rtl_priv *rtlpriv)
{
#ifdef CONFIG_USB_TX_AGGREGATION
	 struct _rtw_hal	*pHalData = GET_HAL_DATA(rtlpriv);
	uint32_t			value32;

	if (pHalData->UsbTxAggMode) {
		value32 = rtl_read_dword(rtlpriv, REG_TDECTRL);
		value32 = value32 & ~(BLK_DESC_NUM_MASK << BLK_DESC_NUM_SHIFT);
		value32 |= ((pHalData->UsbTxAggDescNum & BLK_DESC_NUM_MASK) << BLK_DESC_NUM_SHIFT);

		rtl_write_dword(rtlpriv, REG_TDECTRL, value32);
	}

#endif
}


/*-----------------------------------------------------------------------------
 * Function:	usb_AggSettingRxUpdate()
 *
 * Overview:	Seperate TX/RX parameters update independent for TP detection and
 *			dynamic TX/RX aggreagtion parameters update.
 *
 * Input:			_ADAPTER
 *
 * Output/Return:	NONE
 *
 * Revised History:
 *	When		Who		Remark
 *	12/10/2010	MHC		Seperate to smaller function.
 *
 *---------------------------------------------------------------------------*/
static void usb_AggSettingRxUpdate_8812A(struct rtl_priv *rtlpriv)
{
#ifdef CONFIG_USB_RX_AGGREGATION
	 struct _rtw_hal	*pHalData = GET_HAL_DATA(rtlpriv);
	uint8_t			valueDMA;
	uint8_t			valueUSB;

	valueDMA = rtl_read_byte(rtlpriv, REG_TRXDMA_CTRL);

	switch (pHalData->UsbRxAggMode) {
	case USB_RX_AGG_DMA:
		valueDMA |= RXDMA_AGG_EN;

		/* rtl_write_byte(rtlpriv, REG_RXDMA_AGG_PG_TH, 0x05); //dma agg mode, 20k
		 *
		 * 2012/10/26 MH For TX throught start rate temp fix.
		 */
		{
			u16			temp;

			/* ULLI DMA on USB Device WTF ??? */
			/* Adjust DMA page and thresh. */
			temp = pHalData->RegAcUsbDmaSize | (pHalData->RegAcUsbDmaTime<<8);
			rtl_write_word(rtlpriv, REG_RXDMA_AGG_PG_TH, temp);
		}
		break;
	case USB_RX_AGG_USB:
	case USB_RX_AGG_MIX:
	case USB_RX_AGG_DISABLE:
	default:
		/* TODO: */
		break;
	}

	rtl_write_byte(rtlpriv, REG_TRXDMA_CTRL, valueDMA);
#endif
}

static void init_UsbAggregationSetting_8812A(struct rtl_priv *rtlpriv)
{
	 struct _rtw_hal	*pHalData = GET_HAL_DATA(rtlpriv);

	/* Tx aggregation setting */
	usb_AggSettingTxUpdate_8812A(rtlpriv);

	/* Rx aggregation setting */
	usb_AggSettingRxUpdate_8812A(rtlpriv);

	/* 201/12/10 MH Add for USB agg mode dynamic switch. */
	pHalData->UsbRxHighSpeedMode = _FALSE;
}

/* Set CCK and OFDM Block "ON" */
static void _BBTurnOnBlock(struct rtl_priv *rtlpriv)
{
	rtl_set_bbreg(rtlpriv, RFPGA0_RFMOD, bCCKEn, 0x1);
	rtl_set_bbreg(rtlpriv, RFPGA0_RFMOD, bOFDMEn, 0x1);
}

enum {
	Antenna_Lfet = 1,
	Antenna_Right = 2,
};

static void _InitAntenna_Selection_8812A(struct rtl_priv *rtlpriv)
{
	 struct _rtw_hal	*pHalData	= GET_HAL_DATA(rtlpriv);

	if (pHalData->AntDivCfg == 0)
		return;
	DBG_8192C("==>  %s ....\n", __FUNCTION__);

	rtl_write_byte(rtlpriv, REG_LEDCFG2, 0x82);

	rtl_set_bbreg(rtlpriv, RFPGA0_XAB_RFPARAMETER, BIT13, 0x01);

	if (rtl_get_bbreg(rtlpriv, rFPGA0_XA_RFInterfaceOE, 0x300) == MAIN_ANT)
		pHalData->CurAntenna = MAIN_ANT;
	else
		pHalData->CurAntenna = AUX_ANT;
	DBG_8192C("%s,Cur_ant:(%x)%s\n", __FUNCTION__, pHalData->CurAntenna, (pHalData->CurAntenna == MAIN_ANT) ? "MAIN_ANT" : "AUX_ANT");


}

rt_rf_power_state RfOnOffDetect(struct rtl_priv *rtlpriv)
{
	uint8_t	val8;
	rt_rf_power_state rfpowerstate = rf_off;

	if (rtlpriv->pwrctrlpriv.bHWPowerdown) {
		val8 = rtl_read_byte(rtlpriv, REG_HSISR);
		DBG_8192C("pwrdown, 0x5c(BIT7)=%02x\n", val8);
		rfpowerstate = (val8 & BIT7) ? rf_off : rf_on;
	} else { /* rf on/off */
		rtl_write_byte(rtlpriv, REG_MAC_PINMUX_CFG, rtl_read_byte(rtlpriv, REG_MAC_PINMUX_CFG)&~(BIT3));
		val8 = rtl_read_byte(rtlpriv, REG_GPIO_IO_SEL);
		DBG_8192C("GPIO_IN=%02x\n", val8);
		rfpowerstate = (val8 & BIT3) ? rf_on : rf_off;
	}
	return rfpowerstate;
}

uint32_t rtl8812au_hal_init(struct rtl_priv *rtlpriv)
{
	struct rtl_phy *rtlphy = &(rtlpriv->phy);
	struct rtl_hal *rtlhal = rtl_hal(rtlpriv);
	uint8_t	value8 = 0, u1bRegCR;
	u16  value16;
	uint8_t	txpktbuf_bndy;
	uint32_t	status = _SUCCESS;
	 struct _rtw_hal *pHalData = GET_HAL_DATA(rtlpriv);
	struct pwrctrl_priv *pwrctrlpriv = &rtlpriv->pwrctrlpriv;
	struct registry_priv *pregistrypriv = &rtlpriv->registrypriv;
	rt_rf_power_state eRfPowerStateToSet;
	uint32_t init_start_time = jiffies;

	DBG_871X(" ULLI: Call rtl8812au_hal_init in usb_halinit.c\n");

	if (rtlpriv->pwrctrlpriv.bkeepfwalive) {
		if (rtlphy->iqk_initialized) {
			/* PHY_IQCalibrate_8812A(rtlpriv,_TRUE); */
		} else {
			/* PHY_IQCalibrate_8812A(rtlpriv,_FALSE); */
			rtlphy->iqk_initialized = _TRUE;
		}

		/*
		 * ODM_TXPowerTrackingCheck(&pHalData->odmpriv );
		 * rtl8812au_phy_lc_calibrate(rtlpriv);
		 */

		goto exit;
	}

	/* Check if MAC has already power on. by tynli. 2011.05.27. */
	value8 = rtl_read_byte(rtlpriv, REG_SYS_CLKR+1);
	u1bRegCR = rtl_read_byte(rtlpriv, REG_CR);
	DBG_871X(" power-on :REG_SYS_CLKR 0x09=0x%02x. REG_CR 0x100=0x%02x.\n", value8, u1bRegCR);
	if ((value8&BIT3)  && (u1bRegCR != 0 && u1bRegCR != 0xEA)) {
		/* pHalData->bMACFuncEnable = TRUE; */
		DBG_871X(" MAC has already power on.\n");
	} else {
		/*
		 * pHalData->bMACFuncEnable = FALSE;
		 * Set FwPSState to ALL_ON mode to prevent from the I/O be return because of 32k
		 * state which is set before sleep under wowlan mode. 2012.01.04. by tynli.
		 * pHalData->FwPSState = FW_PS_STATE_ALL_ON_88E;
		 */
		DBG_871X(" MAC has not been powered on yet.\n");
	}

	/*
	 * 2012/11/13 MH Revise for U2/U3 switch we can not update RF-A/B reset.
	 * After discuss with BB team YN, reset after MAC power on to prevent RF
	 * R/W error. Is it a right method?
	 */
	if (!IS_HARDWARE_TYPE_8821(rtlhal)) {
		rtl_write_byte(rtlpriv, REG_RF_CTRL, 5);
		rtl_write_byte(rtlpriv, REG_RF_CTRL, 7);
		rtl_write_byte(rtlpriv, REG_RF_B_CTRL_8812, 5);
		rtl_write_byte(rtlpriv, REG_RF_B_CTRL_8812, 7);
	}

	status = _InitPowerOn8812AU(rtlpriv);
	if (status == _FAIL) {
		goto exit;
	}

		if (IS_HARDWARE_TYPE_8812(rtlhal))
			txpktbuf_bndy = TX_PAGE_BOUNDARY_8812;
		else
			txpktbuf_bndy = TX_PAGE_BOUNDARY_8821;

	status =  _rtl8821au_llt_table_init(rtlpriv, txpktbuf_bndy);
	if (status == _FAIL) {
		goto exit;
	}

#if ENABLE_USB_DROP_INCORRECT_OUT
	_InitHardwareDropIncorrectBulkOut_8812A(rtlpriv);
#endif

	if (pHalData->bRDGEnable)
		_InitRDGSetting_8812A(rtlpriv);

	status = rtl8821au_download_fw(rtlpriv, _FALSE);
	if (status != _SUCCESS) {
		DBG_871X("%s: Download Firmware failed!!\n", __FUNCTION__);
		rtlhal->fw_ready = false;
		pHalData->fw_ractrl = _FALSE;
		/* return status; */
	} else {
		DBG_871X("%s: Download Firmware Success!!\n", __FUNCTION__);
		rtlhal->fw_ready = true;
		pHalData->fw_ractrl = _TRUE;
	}

	InitializeFirmwareVars8812(rtlpriv);

	if (pwrctrlpriv->reg_rfoff == _TRUE)
		pwrctrlpriv->rf_pwrstate = rf_off;

	/*
	 * 2010/08/09 MH We need to check if we need to turnon or off RF after detecting
	 * HW GPIO pin. Before PHY_RFConfig8192C.
	 * HalDetectPwrDownMode(rtlpriv);
	 * 2010/08/26 MH If Efuse does not support sective suspend then disable the function.
	 * HalDetectSelectiveSuspendMode(rtlpriv);
	 */

	/*
	 * Save target channel
	 * <Roger_Notes> Current Channel will be updated again later.
	 */
	rtlpriv->phy.current_channel = 0;	/* set 0 to trigger switch correct channel */

	 _rtl8821au_phy_config_mac_with_headerfile(rtlpriv);

	if (IS_HARDWARE_TYPE_8812(rtlhal)) {
		_InitQueueReservedPage_8812AUsb(rtlpriv);
		_InitTxBufferBoundary_8812AUsb(rtlpriv);
	} else if (IS_HARDWARE_TYPE_8821(rtlhal)) {
		_InitQueueReservedPage_8821AUsb(rtlpriv);
		_InitTxBufferBoundary_8821AUsb(rtlpriv);
	}

	_InitQueuePriority_8812AUsb(rtlpriv);
	_InitPageBoundary_8812AUsb(rtlpriv);

	if (IS_HARDWARE_TYPE_8812(rtlhal))
		_InitTransferPageSize_8812AUsb(rtlpriv);

	/* Get Rx PHY status in order to report RSSI and others. */
	_InitDriverInfoSize_8812A(rtlpriv, DRVINFO_SZ);

	rtl8821au_enable_interrupt(rtlpriv);
	_InitID_8812A(rtlpriv);			/* set mac_address */
	_InitNetworkType_8812A(rtlpriv);	/* set msr */
	_InitWMACSetting_8812A(rtlpriv);
	_InitAdaptiveCtrl_8812AUsb(rtlpriv);
	_InitEDCA_8812AUsb(rtlpriv);

	_InitRetryFunction_8812A(rtlpriv);
	init_UsbAggregationSetting_8812A(rtlpriv);
	rtl8821au_init_beacon_parameters(rtlpriv);
	_InitBeaconMaxError_8812A(rtlpriv, _TRUE);

	_InitBurstPktLen(rtlpriv);  /* added by page. 20110919 */

	/*
	 * Init CR MACTXEN, MACRXEN after setting RxFF boundary REG_TRXFF_BNDY to patch
	 * Hw bug which Hw initials RxFF boundry size to a value which is larger than the real Rx buffer size in 88E.
	 * 2011.08.05. by tynli.
	 */
	value8 = rtl_read_byte(rtlpriv, REG_CR);
	rtl_write_byte(rtlpriv, REG_CR, (value8|MACTXEN|MACRXEN));

#if defined(CONFIG_TX_MCAST2UNI)

#ifdef CONFIG_TX_MCAST2UNI
	rtl_write_word(rtlpriv, REG_PKT_VO_VI_LIFE_TIME, 0x0400);	/* unit: 256us. 256ms */
	rtl_write_word(rtlpriv, REG_PKT_BE_BK_LIFE_TIME, 0x0400);	/* unit: 256us. 256ms */
#else
	rtl_write_word(rtlpriv, REG_PKT_VO_VI_LIFE_TIME, 0x3000);	/* unit: 256us. 3s */
	rtl_write_word(rtlpriv, REG_PKT_BE_BK_LIFE_TIME, 0x3000);	/* unit: 256us. 3s */
#endif
#endif

	/*
	 * d. Initialize BB related configurations.
	 */


	status = PHY_BBConfig8812(rtlpriv);
	if (status == _FAIL)
		goto exit;

	/*
	 * 92CU use 3-wire to r/w RF
	 * pHalData->Rf_Mode = RF_OP_By_SW_3wire;
	 */

	status = PHY_RFConfig8812(rtlpriv);
	if (status == _FAIL)
		goto exit;

	if (rtlpriv->phy.rf_type == RF_1T1R && IS_HARDWARE_TYPE_8812AU(rtlhal))
		_rtl8812au_bb8812_config_1t(rtlpriv);

	if (rtlpriv->registrypriv.channel <= 14)
		rtl8821au_phy_switch_wirelessband(rtlpriv, BAND_ON_2_4G);
	else
		rtl8821au_phy_switch_wirelessband(rtlpriv, BAND_ON_5G);

	rtw_hal_set_chnl_bw(rtlpriv, rtlpriv->registrypriv.channel,
		CHANNEL_WIDTH_20, HAL_PRIME_CHNL_OFFSET_DONT_CARE, HAL_PRIME_CHNL_OFFSET_DONT_CARE);

	invalidate_cam_all(rtlpriv);

	_InitAntenna_Selection_8812A(rtlpriv);

	/*
	 * HW SEQ CTRL
	 * set 0x0 to 0xFF by tynli. Default enable HW SEQ NUM.
	 */
	rtl_write_byte(rtlpriv, REG_HWSEQ_CTRL, 0xFF);

	/*
	 * Disable BAR, suggested by Scott
	 * 2010.04.09 add by hpfan
	 */
	rtl_write_dword(rtlpriv, REG_BAR_MODE_CTRL, 0x0201ffff);

	/* Nav limit , suggest by scott */
	rtl_write_byte(rtlpriv, 0x652, 0x0);


	rtl8812_InitHalDm(rtlpriv);

	/*
	 * 2010/08/11 MH Merge from 8192SE for Minicard init. We need to confirm current radio status
	 * and then decide to enable RF or not.!!!??? For Selective suspend mode. We may not
	 * call initstruct rtl_priv. May cause some problem??
	 *
	 * Fix the bug that Hw/Sw radio off before S3/S4, the RF off action will not be executed
	 * in MgntActSet_RF_State() after wake up, because the value of pHalData->eRFPowerState
	 * is the same as eRfOff, we should change it to eRfOn after we config RF parameters.
	 * Added by tynli. 2010.03.30.
	 */
	pwrctrlpriv->rf_pwrstate = rf_on;

	/*
	 * 0x4c6[3] 1: RTS BW = Data BW
	 * 0: RTS BW depends on CCA / secondary CCA result.
	 */
	rtl_write_byte(rtlpriv, REG_QUEUE_CTRL, rtl_read_byte(rtlpriv, REG_QUEUE_CTRL)&0xF7);

	/* enable Tx report. */
	rtl_write_byte(rtlpriv,  REG_FWHW_TXQ_CTRL+1, 0x0F);

	/* Suggested by SD1 pisa. Added by tynli. 2011.10.21. */
	rtl_write_byte(rtlpriv, REG_EARLY_MODE_CONTROL_8812+3, 0x01);/* Pretx_en, for WEP/TKIP SEC */

	/* tynli_test_tx_report. */
	rtl_write_word(rtlpriv, REG_TX_RPT_TIME, 0x3DF0);

	/* Reset USB mode switch setting */
	rtl_write_byte(rtlpriv, REG_SDIO_CTRL_8812, 0x0);
	rtl_write_byte(rtlpriv, REG_ACLK_MON, 0x0);

	/*
	 * RT_TRACE(COMP_INIT, DBG_TRACE, ("InitializeAdapter8188EUsb() <====\n"));
	 */

	/* 2010/08/26 MH Merge from 8192CE. */
	if (pwrctrlpriv->rf_pwrstate == rf_on) {
		if (IS_HARDWARE_TYPE_8812AU(rtlhal)) {
			rtlphy->need_iqk = true;
			if (rtlphy->iqk_initialized)
				rtl8812au_phy_iq_calibrate(rtlpriv, _TRUE);
			else {
				rtl8812au_phy_iq_calibrate(rtlpriv, _FALSE);
				rtlphy->iqk_initialized = _TRUE;
			}
		}


		/* ODM_TXPowerTrackingCheck(&pHalData->odmpriv ); */


		/* rtl8812au_phy_lc_calibrate(rtlpriv); */
	}

	/* HAL_INIT_PROFILE_TAG(HAL_INIT_STAGES_INIT_PABIAS);
	 * _InitPABias(rtlpriv);
	 */

	/*
	 *  2010/08/23 MH According to Alfred's suggestion, we need to to prevent HW enter
	 *  suspend mode automatically.
	 * HwSuspendModeEnable92Cu(rtlpriv, _FALSE);
	 */


	rtl_write_byte(rtlpriv, REG_USB_HRPWM, 0);

	/* misc */
	{
		/* ULLI reading MAC address again ?? */
		int i;
		uint8_t mac_addr[6];
		for (i = 0; i < 6; i++) {
			mac_addr[i] = rtl_read_byte(rtlpriv, REG_MACID+i);
		}

		DBG_8192C("MAC Address from REG_MACID = "MAC_FMT"\n", MAC_ARG(mac_addr));
	}

exit:

	DBG_871X("%s in %dms\n", __FUNCTION__, rtw_get_passing_time_ms(init_start_time));

	return status;
}

static void _rtl8821au_poweroff_adapter(struct rtl_priv *rtlpriv)
{
	struct rtl_hal *rtlhal = rtl_hal(rtlpriv);
	uint8_t	u1bTmp;
	uint8_t 	val8;
	u16	val16;
	uint32_t	val32;

	/* DBG_871X("CardDisableRTL8188EU\n"); */

	/* Stop Tx Report Timer. 0x4EC[Bit1]=b'0 */
	u1bTmp = rtl_read_byte(rtlpriv, REG_TX_RPT_CTRL);
	rtl_write_byte(rtlpriv, REG_TX_RPT_CTRL, val8&(~BIT1));

	/* stop rx */
	rtl_write_byte(rtlpriv, REG_CR, 0x0);

	/* Run LPS WL RFOFF flow */
	if (IS_HARDWARE_TYPE_8821U(rtlhal))
		HalPwrSeqCmdParsing(rtlpriv, PWR_CUT_ALL_MSK, PWR_FAB_ALL_MSK, PWR_INTF_USB_MSK, Rtl8821A_NIC_LPS_ENTER_FLOW);
	else
		HalPwrSeqCmdParsing(rtlpriv, PWR_CUT_ALL_MSK, PWR_FAB_ALL_MSK, PWR_INTF_USB_MSK, Rtl8812_NIC_LPS_ENTER_FLOW);

	if ((rtl_read_byte(rtlpriv, REG_MCUFWDL)&RAM_DL_SEL) && rtlhal->fw_ready) {
		  /* 8051 RAM code */
		rtl8821au_firmware_selfreset(rtlpriv);
	}

	/* Reset MCU. Suggested by Filen. 2011.01.26. by tynli. */
	u1bTmp = rtl_read_byte(rtlpriv, REG_SYS_FUNC_EN+1);
	rtl_write_byte(rtlpriv, REG_SYS_FUNC_EN+1, (u1bTmp&(~BIT2)));

	/* MCUFWDL 0x80[1:0]=0
	 * reset MCU ready status
	 */
	rtl_write_byte(rtlpriv, REG_MCUFWDL, 0x00);

	/* Card disable power action flow */
	if (IS_HARDWARE_TYPE_8821U(rtlhal))
		HalPwrSeqCmdParsing(rtlpriv, PWR_CUT_ALL_MSK, PWR_FAB_ALL_MSK, PWR_INTF_USB_MSK, Rtl8821A_NIC_DISABLE_FLOW);
	else
		HalPwrSeqCmdParsing(rtlpriv, PWR_CUT_ALL_MSK, PWR_FAB_ALL_MSK, PWR_INTF_USB_MSK, Rtl8812_NIC_DISABLE_FLOW);
}

static void rtl8812au_hw_power_down(struct rtl_priv *rtlpriv)
{
	/*
	 *  2010/-8/09 MH For power down module, we need to enable register block contrl reg at 0x1c.
	 * Then enable power down control bit of register 0x04 BIT4 and BIT15 as 1.
	 */

	/* Enable register area 0x0-0xc. */
	rtl_write_byte(rtlpriv, REG_RSV_CTRL, 0x0);
	rtl_write_word(rtlpriv, REG_APS_FSMCO, 0x8812);
}

uint32_t rtl8812au_hal_deinit(struct rtl_priv *rtlpriv)
{

	struct _rtw_hal	*pHalData = GET_HAL_DATA(rtlpriv);
	DBG_8192C("==> %s \n", __FUNCTION__);

	rtl_write_word(rtlpriv, REG_GPIO_MUXCFG, rtl_read_word(rtlpriv, REG_GPIO_MUXCFG)&(~BIT12));

	if (pHalData->bSupportUSB3 == _TRUE) {
		/*
		 * set Reg 0xf008[3:4] to 2'11 to eable U1/U2 Mode in USB3.0. added by page, 20120712
		 */
		rtl_write_byte(rtlpriv, 0xf008, rtl_read_byte(rtlpriv, 0xf008)|0x18);
	}

	rtl_write_dword(rtlpriv, REG_HISR0_8812, 0xFFFFFFFF);
	rtl_write_dword(rtlpriv, REG_HISR1_8812, 0xFFFFFFFF);
	rtl_write_dword(rtlpriv, REG_HIMR0_8812, IMR_DISABLED_8812);
	rtl_write_dword(rtlpriv, REG_HIMR1_8812, IMR_DISABLED_8812);

	if (rtlpriv->hw_init_completed == _TRUE) {
		_rtl8821au_poweroff_adapter(rtlpriv);

		if ((rtlpriv->pwrctrlpriv.bHWPwrPindetect) && (rtlpriv->pwrctrlpriv.bHWPowerdown))
			rtl8812au_hw_power_down(rtlpriv);
	}
	return _SUCCESS;
}

unsigned int rtl8812au_inirp_deinit(struct rtl_priv *rtlpriv)
{
	usb_read_port_cancel(rtlpriv);
	return _SUCCESS;
}

/*
 * -------------------------------------------------------------------
 *
 * 	EEPROM/EFUSE Content Parsing
 *
 * -------------------------------------------------------------------
 */

void UpdateInterruptMask8812AU(struct rtl_priv *rtlpriv, uint8_t bHIMR0, uint32_t AddMSR, uint32_t RemoveMSR)
{
	struct rtl_usb *rtlusb = rtl_usbdev(rtlpriv);

	uint32_t *himr;

	if (bHIMR0)
		himr = &(rtlusb->irq_mask[0]);
	else
		himr = &(rtlusb->irq_mask[1]);

	if (AddMSR)
		*himr |= AddMSR;

	if (RemoveMSR)
		*himr &= (~RemoveMSR);

	if (bHIMR0)
		rtl_write_dword(rtlpriv, REG_HIMR0_8812, *himr);
	else
		rtl_write_dword(rtlpriv, REG_HIMR1_8812, *himr);

}

void _update_response_rate(struct rtl_priv *rtlpriv, unsigned int mask)
{
	uint8_t	RateIndex = 0;
	/* Set RRSR rate table. */
	rtl_write_byte(rtlpriv, REG_RRSR, mask&0xff);
	rtl_write_byte(rtlpriv, REG_RRSR+1, (mask>>8)&0xff);

	/* Set RTS initial rate */
	while (mask > 0x1) {
		mask = (mask >> 1);
		RateIndex++;
	}
	rtl_write_byte(rtlpriv, REG_INIRTS_RATE_SEL, RateIndex);
}

void rtl8812au_init_default_value(struct rtl_priv *rtlpriv)
{
	struct rtl_usb *rtlusb = rtl_usbdev(rtlpriv);
	struct rtl_phy *rtlphy = &(rtlpriv->phy);
	struct rtl_hal *rtlhal = rtl_hal(rtlpriv);
	struct _rtw_hal *pHalData;
	struct pwrctrl_priv *pwrctrlpriv;
	struct dm_priv *pdmpriv;
	uint8_t i;

	pHalData = GET_HAL_DATA(rtlpriv);
	pwrctrlpriv = &rtlpriv->pwrctrlpriv;
	pdmpriv = &pHalData->dmpriv;


	/* init default value */
	pHalData->fw_ractrl = _FALSE;
	if (!pwrctrlpriv->bkeepfwalive)
		rtlhal->last_hmeboxnum = 0;

	/* init dm default value */
	pHalData->bChnlBWInitialzed = _FALSE;
	rtlphy->iqk_initialized = _FALSE;
	pHalData->odmpriv.RFCalibrateInfo.TM_Trigger = 0;/* for IQK */
	rtlphy->pwrgroup_cnt = 0;

	rtlusb->irq_mask[0]	= (u32)(	\
/*
					IMR_ROK 		|
					IMR_RDU		|
					IMR_VODOK		|
					IMR_VIDOK		|
					IMR_BEDOK		|
					IMR_BKDOK		|
					IMR_MGNTDOK		|
					IMR_HIGHDOK		|
					IMR_CPWM		|
					IMR_CPWM2		|
					IMR_C2HCMD		|
					IMR_HISR1_IND_INT	|
					IMR_ATIMEND		|
					IMR_BCNDMAINT_E	|
					IMR_HSISR_IND_ON_INT	|
					IMR_BCNDOK0		|
					IMR_BCNDMAINT0	|
					IMR_TSF_BIT32_TOGGLE	|
					IMR_TXBCN0OK	|
					IMR_TXBCN0ERR	|
					IMR_GTINT3		|
					IMR_GTINT4		|
					IMR_TXCCK		|
 */
					0);

	rtlusb->irq_mask[1] 	= (u32)(	\
/*
					IMR_RXFOVW		|
					IMR_TXFOVW		|
					IMR_RXERR		|
					IMR_TXERR		|
					IMR_ATIMEND_E	|
					IMR_BCNDOK1		|
					IMR_BCNDOK2		|
					IMR_BCNDOK3		|
					IMR_BCNDOK4		|
					IMR_BCNDOK5		|
					IMR_BCNDOK6		|
					IMR_BCNDOK7		|
					IMR_BCNDMAINT1	|
					IMR_BCNDMAINT2	|
					IMR_BCNDMAINT3	|
					IMR_BCNDMAINT4	|
					IMR_BCNDMAINT5	|
					IMR_BCNDMAINT6	|
					IMR_BCNDMAINT7	|
 */
					0);
}

