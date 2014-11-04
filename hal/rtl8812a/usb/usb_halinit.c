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

static void _dbg_dump_macreg(struct rtl_priv *padapter)
{
	uint32_t offset = 0;
	uint32_t val32 = 0;
	uint32_t index = 0;

	for (index = 0; index < 64; index++) {
		offset = index*4;
		val32 = rtw_read32(padapter, offset);
		DBG_8192C("offset : 0x%02x ,val:0x%08x\n", offset, val32);
	}
}

static VOID _ConfigChipOutEP_8812(struct rtl_priv *pAdapter, uint8_t NumOutPipe)
{
	HAL_DATA_TYPE *pHalData = GET_HAL_DATA(pAdapter);

	pHalData->OutEpQueueSel = 0;
	pHalData->OutEpNumber = 0;

	switch (NumOutPipe) {
	case 	4:
		pHalData->OutEpQueueSel = TX_SELE_HQ | TX_SELE_LQ | TX_SELE_NQ;
		pHalData->OutEpNumber = 4;
		break;
	case 	3:
		pHalData->OutEpQueueSel = TX_SELE_HQ | TX_SELE_LQ | TX_SELE_NQ;
		pHalData->OutEpNumber = 3;
		break;
	case 	2:
		pHalData->OutEpQueueSel = TX_SELE_HQ | TX_SELE_NQ;
		pHalData->OutEpNumber = 2;
		break;
	case 	1:
		pHalData->OutEpQueueSel = TX_SELE_HQ;
		pHalData->OutEpNumber = 1;
		break;
	default:
		break;

	}
	DBG_871X("%s OutEpQueueSel(0x%02x), OutEpNumber(%d) \n", __FUNCTION__, pHalData->OutEpQueueSel, pHalData->OutEpNumber);

}

static BOOLEAN HalUsbSetQueuePipeMapping8812AUsb(struct rtl_priv *pAdapter,
	uint8_t	NumInPipe, uint8_t NumOutPipe)
{
	HAL_DATA_TYPE	*pHalData	= GET_HAL_DATA(pAdapter);
	BOOLEAN		result		= _FALSE;

	_ConfigChipOutEP_8812(pAdapter, NumOutPipe);

	/* Normal chip with one IN and one OUT doesn't have interrupt IN EP. */
	if (1 == pHalData->OutEpNumber) {
		if (1 != NumInPipe) {
			return result;
		}
	}

	/*
	 * All config other than above support one Bulk IN and one Interrupt IN.
	 * if (2 != NumInPipe){
	 * 	return result;
	 * }
	 */

	result = Hal_MappingOutPipe(pAdapter, NumOutPipe);

	return result;

}

void rtl8812au_interface_configure(struct rtl_priv *padapter)
{
	HAL_DATA_TYPE	*pHalData	= GET_HAL_DATA(padapter);
	struct dvobj_priv	*pdvobjpriv = adapter_to_dvobj(padapter);

	if (IS_SUPER_SPEED_USB(padapter)) {
		pHalData->UsbBulkOutSize = USB_SUPER_SPEED_BULK_SIZE;	/* 1024 bytes */
	} else if (IS_HIGH_SPEED_USB(padapter)) {
		pHalData->UsbBulkOutSize = USB_HIGH_SPEED_BULK_SIZE;	/* 512 bytes */
	} else {
		pHalData->UsbBulkOutSize = USB_FULL_SPEED_BULK_SIZE; 	/*64 bytes */
	}

	pHalData->interfaceIndex = pdvobjpriv->InterfaceNumber;

#ifdef CONFIG_USB_TX_AGGREGATION
	pHalData->UsbTxAggMode		= 1;
	pHalData->UsbTxAggDescNum	= 6;	/* only 4 bits */

	if (IS_HARDWARE_TYPE_8812AU(padapter))	/* page added for Jaguar */
		pHalData->UsbTxAggDescNum = 3;
#endif

#ifdef CONFIG_USB_RX_AGGREGATION
	pHalData->UsbRxAggMode		= USB_RX_AGG_DMA;	/* USB_RX_AGG_DMA; */
	pHalData->UsbRxAggBlockCount	= 8; 			/* unit : 512b */
	pHalData->UsbRxAggBlockTimeout	= 0x6;
	pHalData->UsbRxAggPageCount	= 16; 			/* uint :128 b //0x0A;	// 10 = MAX_RX_DMA_BUFFER_SIZE/2/pHalData->UsbBulkOutSize */
	pHalData->UsbRxAggPageTimeout = 0x6; 			/* 6, absolute time = 34ms/(2^6) */

	pHalData->RegAcUsbDmaSize = 4;
	pHalData->RegAcUsbDmaTime = 8;
#endif

	HalUsbSetQueuePipeMapping8812AUsb(padapter,
				pdvobjpriv->RtNumInPipes, pdvobjpriv->RtNumOutPipes);

}

static VOID _InitBurstPktLen(IN struct rtl_priv *Adapter)
{
	u1Byte speedvalue, provalue, temp;
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(Adapter);

	/*
	 * rtw_write16(Adapter, REG_TRXDMA_CTRL_8195, 0xf5b0);
	 * rtw_write16(Adapter, REG_TRXDMA_CTRL_8812, 0xf5b4);
	 */
	rtw_write8(Adapter, 0xf050, 0x01);		/* usb3 rx interval */
	rtw_write16(Adapter, REG_RXDMA_STATUS, 0x7400);	/* burset lenght=4, set 0x3400 for burset length=2 */
	rtw_write8(Adapter, 0x289, 0xf5);		/* for rxdma control */
	/* rtw_write8(Adapter, 0x3a, 0x46); */

	/*  0x456 = 0x70, sugguested by Zhilin */
	rtw_write8(Adapter, REG_AMPDU_MAX_TIME_8812, 0x70);

	rtw_write32(Adapter, 0x458, 0xffffffff);
	rtw_write8(Adapter, REG_USTIME_TSF, 0x50);
	rtw_write8(Adapter, REG_USTIME_EDCA, 0x50);

	if (IS_HARDWARE_TYPE_8821U(Adapter))
		speedvalue = BIT7;
	else
		speedvalue = rtw_read8(Adapter, 0xff); /* check device operation speed: SS 0xff bit7 */

	if (speedvalue & BIT7) {		/* USB2/1.1 Mode */
		temp = rtw_read8(Adapter, 0xfe17);
		if (((temp >> 4) & 0x03) == 0) {
			pHalData->UsbBulkOutSize = USB_HIGH_SPEED_BULK_SIZE;
			provalue = rtw_read8(Adapter, REG_RXDMA_PRO_8812);
			rtw_write8(Adapter, REG_RXDMA_PRO_8812, ((provalue|BIT(4))&(~BIT(5)))); /* set burst pkt len=512B */
			rtw_write16(Adapter, REG_RXDMA_PRO_8812, 0x1e);
		} else {
			pHalData->UsbBulkOutSize = 64;
			provalue = rtw_read8(Adapter, REG_RXDMA_PRO_8812);
			rtw_write8(Adapter, REG_RXDMA_PRO_8812, ((provalue|BIT(5))&(~BIT(4)))); /* set burst pkt len=64B */
		}

		rtw_write16(Adapter, REG_RXDMA_AGG_PG_TH, 0x2005); /* dmc agg th 20K */

		/*
		 * rtw_write8(Adapter, 0x10c, 0xb4);
		 * hal_UphyUpdate8812AU(Adapter);
		 */

		pHalData->bSupportUSB3 = _FALSE;
	} else {		/* USB3 Mode */
		pHalData->UsbBulkOutSize = USB_SUPER_SPEED_BULK_SIZE;
		provalue = rtw_read8(Adapter, REG_RXDMA_PRO_8812);
		rtw_write8(Adapter, REG_RXDMA_PRO_8812, provalue&(~(BIT5|BIT4))); /* set burst pkt len=1k */
		rtw_write16(Adapter, REG_RXDMA_PRO_8812, 0x0e);
		pHalData->bSupportUSB3 = _TRUE;

		/*  set Reg 0xf008[3:4] to 2'00 to disable U1/U2 Mode to avoid 2.5G spur in USB3.0. added by page, 20120712 */
		rtw_write8(Adapter, 0xf008, rtw_read8(Adapter, 0xf008)&0xE7);
	}

#ifdef CONFIG_USB_TX_AGGREGATION
	/* rtw_write8(Adapter, REG_TDECTRL_8195, 0x30); */
#else
	rtw_write8(Adapter, REG_TDECTRL, 0x10);
#endif

	temp = rtw_read8(Adapter, REG_SYS_FUNC_EN);
	rtw_write8(Adapter, REG_SYS_FUNC_EN, temp&(~BIT(10))); 	/* reset 8051 */

	rtw_write8(Adapter, REG_HT_SINGLE_AMPDU_8812, rtw_read8(Adapter, REG_HT_SINGLE_AMPDU_8812)|BIT(7));	/* enable single pkt ampdu */
	rtw_write8(Adapter, REG_RX_PKT_LIMIT, 0x18);		/* for VHT packet length 11K */

	rtw_write8(Adapter, REG_PIFS, 0x00);

	/* Suggention by SD1 Jong and Pisa, by Maddest 20130107. */
	if (IS_HARDWARE_TYPE_8821U(Adapter) && (Adapter->registrypriv.wifi_spec == _FALSE)) {
		rtw_write16(Adapter, REG_MAX_AGGR_NUM, 0x0a0a);
		rtw_write8(Adapter, REG_FWHW_TXQ_CTRL, 0x80);
		rtw_write8(Adapter, REG_AMPDU_MAX_TIME_8812, 0x5e);
		rtw_write32(Adapter, REG_FAST_EDCA_CTRL, 0x03087777);
	} else {
		rtw_write8(Adapter, REG_MAX_AGGR_NUM, 0x1f);
		rtw_write8(Adapter, REG_FWHW_TXQ_CTRL, rtw_read8(Adapter, REG_FWHW_TXQ_CTRL)&(~BIT(7)));
	}

	if (pHalData->AMPDUBurstMode) {
		rtw_write8(Adapter, REG_AMPDU_BURST_MODE_8812, 0x5F);
	}

	rtw_write8(Adapter, 0x1c, rtw_read8(Adapter, 0x1c) | BIT(5) | BIT(6));  /* to prevent mac is reseted by bus. 20111208, by Page */

	/* ARFB table 9 for 11ac 5G 2SS */
	rtw_write32(Adapter, REG_ARFR0, 0x00000010);
	if (IS_NORMAL_CHIP(pHalData->VersionID))
		rtw_write32(Adapter, REG_ARFR0+4, 0xfffff000);
	else
		rtw_write32(Adapter, REG_ARFR0+4, 0x3e0ff000);

	/* ARFB table 10 for 11ac 5G 1SS */
	rtw_write32(Adapter, REG_ARFR1, 0x00000010);
	if (IS_VENDOR_8812A_TEST_CHIP(Adapter))
		rtw_write32(Adapter, REG_ARFR1_8812+4, 0x000ff000);
	else
		rtw_write32(Adapter, REG_ARFR1_8812+4, 0x003ff000);

}

static uint32_t _InitPowerOn8812AU(struct rtl_priv *padapter)
{
	uint16_t	u2btmp = 0;
	uint8_t	u1btmp = 0;

	if (IS_VENDOR_8821A_MP_CHIP(padapter)) {
		/* HW Power on sequence */
		if (!HalPwrSeqCmdParsing(padapter, PWR_CUT_A_MSK, PWR_FAB_ALL_MSK, PWR_INTF_USB_MSK, Rtl8821A_NIC_ENABLE_FLOW)) {
			DBG_871X(KERN_ERR "%s: run power on flow fail\n", __func__);
			return _FAIL;
		}
	} else if (IS_HARDWARE_TYPE_8821U(padapter)) {
		if (!HalPwrSeqCmdParsing(padapter, PWR_CUT_TESTCHIP_MSK, PWR_FAB_ALL_MSK, PWR_INTF_USB_MSK, Rtl8821A_NIC_ENABLE_FLOW)) {
			DBG_871X(KERN_ERR "%s: run power on flow fail\n", __func__);
			return _FAIL;
		}
	} else {
		if (!HalPwrSeqCmdParsing(padapter, PWR_CUT_ALL_MSK, PWR_FAB_ALL_MSK, PWR_INTF_USB_MSK, Rtl8812_NIC_ENABLE_FLOW)) {
			DBG_871X(KERN_ERR "%s: run power on flow fail\n", __func__);
			return _FAIL;
		}
	}

	/*
	 *  Enable MAC DMA/WMAC/SCHEDULE/SEC block
	 * Set CR bit10 to enable 32k calibration. Suggested by SD1 Gimmy. Added by tynli. 2011.08.31.
	 */
	rtw_write16(padapter, REG_CR, 0x00); 	/* suggseted by zhouzhou, by page, 20111230 */
	u2btmp = rtw_read16(padapter, REG_CR);
	u2btmp |= (HCI_TXDMA_EN | HCI_RXDMA_EN | TXDMA_EN | RXDMA_EN
				| PROTOCOL_EN | SCHEDULE_EN | ENSEC | CALTMR_EN);
	rtw_write16(padapter, REG_CR, u2btmp);

	/*
	 * Need remove below furture, suggest by Jackie.
	 * if 0xF0[24] =1 (LDO), need to set the 0x7C[6] to 1.
	 */
	if (IS_HARDWARE_TYPE_8821U(padapter)) {
		u1btmp = rtw_read8(padapter, REG_SYS_CFG+3);
		if (u1btmp & BIT0) { 	/* LDO mode. */
			u1btmp = rtw_read8(padapter, 0x7c);
			/* ULLI unknown register */
			rtw_write8(padapter, 0x7c, u1btmp | BIT6);
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
static VOID _InitInterrupt_8812AU(struct rtl_priv *Adapter)
{
	HAL_DATA_TYPE *pHalData = GET_HAL_DATA(Adapter);

	/* HIMR */
	rtw_write32(Adapter, REG_HIMR0_8812, pHalData->IntrMask[0]&0xFFFFFFFF);
	rtw_write32(Adapter, REG_HIMR1_8812, pHalData->IntrMask[1]&0xFFFFFFFF);
}

static VOID _InitQueueReservedPage_8821AUsb(struct rtl_priv *Adapter)
{
	HAL_DATA_TYPE *pHalData = GET_HAL_DATA(Adapter);
	struct registry_priv *pregistrypriv = &Adapter->registrypriv;
	uint32_t numHQ = 0;
	uint32_t numLQ = 0;
	uint32_t numNQ = 0;
	uint32_t numPubQ = 0;
	uint32_t value32;
	uint8_t	 value8;
	BOOLEAN	bWiFiConfig = pregistrypriv->wifi_spec;

	if (!bWiFiConfig) {
		numPubQ = NORMAL_PAGE_NUM_PUBQ_8821;

		if (pHalData->OutEpQueueSel & TX_SELE_HQ) {
			numHQ = NORMAL_PAGE_NUM_HPQ_8821;
		}

		if (pHalData->OutEpQueueSel & TX_SELE_LQ) {
			numLQ = NORMAL_PAGE_NUM_LPQ_8821;
		}

		/* NOTE: This step shall be proceed before writting REG_RQPN. */
		if (pHalData->OutEpQueueSel & TX_SELE_NQ) {
			numNQ = NORMAL_PAGE_NUM_NPQ_8821;
		}
	} else { /* WMM */
		numPubQ = WMM_NORMAL_PAGE_NUM_PUBQ_8821;

		if (pHalData->OutEpQueueSel & TX_SELE_HQ) {
			numHQ = WMM_NORMAL_PAGE_NUM_HPQ_8821;
		}

		if (pHalData->OutEpQueueSel & TX_SELE_LQ) {
			numLQ = WMM_NORMAL_PAGE_NUM_LPQ_8821;
		}

		/* NOTE: This step shall be proceed before writting REG_RQPN. */
		if (pHalData->OutEpQueueSel & TX_SELE_NQ) {
			numNQ = WMM_NORMAL_PAGE_NUM_NPQ_8821;
		}
	}

	value8 = (u8)_NPQ(numNQ);
	rtw_write8(Adapter, REG_RQPN_NPQ, value8);

	/* TX DMA */
	value32 = _HPQ(numHQ) | _LPQ(numLQ) | _PUBQ(numPubQ) | LD_RQPN;
	rtw_write32(Adapter, REG_RQPN, value32);
}

static VOID _InitQueueReservedPage_8812AUsb(struct rtl_priv *Adapter)
{
	HAL_DATA_TYPE *pHalData = GET_HAL_DATA(Adapter);
	struct registry_priv	*pregistrypriv = &Adapter->registrypriv;
	uint32_t numHQ		= 0;
	uint32_t numLQ		= 0;
	uint32_t numNQ		= 0;
	uint32_t numPubQ	= 0;
	uint32_t value32;
	uint8_t	value8;
	BOOLEAN	bWiFiConfig	= pregistrypriv->wifi_spec;

	if (!bWiFiConfig) {
		numPubQ = NORMAL_PAGE_NUM_PUBQ_8812;

		if (pHalData->OutEpQueueSel & TX_SELE_HQ) {
			numHQ = NORMAL_PAGE_NUM_HPQ_8812;
		}

		if (pHalData->OutEpQueueSel & TX_SELE_LQ) {
			numLQ = NORMAL_PAGE_NUM_LPQ_8812;
		}

		/* NOTE: This step shall be proceed before writting REG_RQPN. */
		if (pHalData->OutEpQueueSel & TX_SELE_NQ) {
			numNQ = NORMAL_PAGE_NUM_NPQ_8812;
		}
	} else { /* WMM */
		numPubQ = WMM_NORMAL_PAGE_NUM_PUBQ_8812;

		if (pHalData->OutEpQueueSel & TX_SELE_HQ) {
			numHQ = WMM_NORMAL_PAGE_NUM_HPQ_8812;
		}

		if (pHalData->OutEpQueueSel & TX_SELE_LQ) {
			numLQ = WMM_NORMAL_PAGE_NUM_LPQ_8812;
		}

		/* NOTE: This step shall be proceed before writting REG_RQPN. */
		if (pHalData->OutEpQueueSel & TX_SELE_NQ) {
			numNQ = WMM_NORMAL_PAGE_NUM_NPQ_8812;
		}
	}

	value8 = (u8)_NPQ(numNQ);
	rtw_write8(Adapter, REG_RQPN_NPQ, value8);

	/* TX DMA */
	value32 = _HPQ(numHQ) | _LPQ(numLQ) | _PUBQ(numPubQ) | LD_RQPN;
	rtw_write32(Adapter, REG_RQPN, value32);
}

static void _InitID_8812A(IN  struct rtl_priv *Adapter)
{
	hal_init_macaddr(Adapter);	/* set mac_address */
}

static VOID _InitTxBufferBoundary_8821AUsb(struct rtl_priv *Adapter)
{
	struct registry_priv *pregistrypriv = &Adapter->registrypriv;
	uint8_t	txpktbuf_bndy;

	if (!pregistrypriv->wifi_spec) {
		txpktbuf_bndy = TX_PAGE_BOUNDARY_8821;
	} else {	/* for WMM */
		txpktbuf_bndy = WMM_NORMAL_TX_PAGE_BOUNDARY_8821;
	}

	rtw_write8(Adapter, REG_BCNQ_BDNY, txpktbuf_bndy);
	rtw_write8(Adapter, REG_MGQ_BDNY, txpktbuf_bndy);
	rtw_write8(Adapter, REG_WMAC_LBK_BF_HD, txpktbuf_bndy);
	rtw_write8(Adapter, REG_TRXFF_BNDY, txpktbuf_bndy);
	rtw_write8(Adapter, REG_TDECTRL+1, txpktbuf_bndy);
}

static VOID _InitTxBufferBoundary_8812AUsb(struct rtl_priv *Adapter)
{
	struct registry_priv *pregistrypriv = &Adapter->registrypriv;
	uint8_t	txpktbuf_bndy;

	if (!pregistrypriv->wifi_spec) {
		txpktbuf_bndy = TX_PAGE_BOUNDARY_8812;
	} else {	/* for WMM */
		txpktbuf_bndy = WMM_NORMAL_TX_PAGE_BOUNDARY_8812;
	}

	rtw_write8(Adapter, REG_BCNQ_BDNY, txpktbuf_bndy);
	rtw_write8(Adapter, REG_MGQ_BDNY, txpktbuf_bndy);
	rtw_write8(Adapter, REG_WMAC_LBK_BF_HD, txpktbuf_bndy);
	rtw_write8(Adapter, REG_TRXFF_BNDY, txpktbuf_bndy);
	rtw_write8(Adapter, REG_TDECTRL+1, txpktbuf_bndy);

}

static VOID _InitPageBoundary_8812AUsb(struct rtl_priv *Adapter)
{
	/*
	 * uint16_t 			rxff_bndy;
	 * uint16_t			Offset;
	 * BOOLEAN			bSupportRemoteWakeUp;
	 */

	/*
	 * Adapter->HalFunc.GetHalDefVarHandler(Adapter, HAL_DEF_WOWLAN , &bSupportRemoteWakeUp);
	 * RX Page Boundary
	 * srand(static_cast<unsigned int>(time(NULL)) );
	 */

	/*
	 * Offset = MAX_RX_DMA_BUFFER_SIZE_8812/256;
	 * rxff_bndy = (Offset*256)-1;
	 */

	if (IS_HARDWARE_TYPE_8812(Adapter))
		rtw_write16(Adapter, (REG_TRXFF_BNDY + 2), MAX_RX_DMA_BUFFER_SIZE_8812-1);
	else
		rtw_write16(Adapter, (REG_TRXFF_BNDY + 2), MAX_RX_DMA_BUFFER_SIZE_8821-1);

}


static VOID _InitNormalChipRegPriority_8812AUsb(struct rtl_priv *Adapter,
	uint16_t beQ, uint16_t bkQ, uint16_t viQ,
	uint16_t voQ, uint16_t mgtQ, uint16_t hiQ)
{
	uint16_t value16 = (rtw_read16(Adapter, REG_TRXDMA_CTRL) & 0x7);

	value16 |= _TXDMA_BEQ_MAP(beQ) 	| _TXDMA_BKQ_MAP(bkQ) |
		   _TXDMA_VIQ_MAP(viQ) 	| _TXDMA_VOQ_MAP(voQ) |
		   _TXDMA_MGQ_MAP(mgtQ) | _TXDMA_HIQ_MAP(hiQ);

	rtw_write16(Adapter, REG_TRXDMA_CTRL, value16);
}

static VOID _InitNormalChipTwoOutEpPriority_8812AUsb(struct rtl_priv *Adapter)
{
	HAL_DATA_TYPE	*pHalData	= GET_HAL_DATA(Adapter);
	struct registry_priv *pregistrypriv = &Adapter->registrypriv;
	uint16_t	beQ, bkQ, viQ, voQ, mgtQ, hiQ;

	uint16_t	valueHi = 0;
	uint16_t	valueLow = 0;

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

	if (!pregistrypriv->wifi_spec) {
		beQ	= valueLow;
		bkQ	= valueLow;
		viQ	= valueHi;
		voQ	= valueHi;
		mgtQ	= valueHi;
		hiQ	= valueHi;
	} else{	/* for WMM ,CONFIG_OUT_EP_WIFI_MODE */
		beQ	= valueLow;
		bkQ	= valueHi;
		viQ	= valueHi;
		voQ	= valueLow;
		mgtQ	= valueHi;
		hiQ	= valueHi;
	}

	_InitNormalChipRegPriority_8812AUsb(Adapter, beQ, bkQ, viQ, voQ, mgtQ, hiQ);

}

static VOID _InitNormalChipThreeOutEpPriority_8812AUsb(struct rtl_priv *Adapter)
{
	struct registry_priv *pregistrypriv = &Adapter->registrypriv;
	uint16_t	beQ, bkQ, viQ, voQ, mgtQ, hiQ;

	if (!pregistrypriv->wifi_spec) {	/* typical setting */
		beQ	= QUEUE_LOW;
		bkQ	= QUEUE_LOW;
		viQ	= QUEUE_NORMAL;
		voQ	= QUEUE_HIGH;
		mgtQ 	= QUEUE_HIGH;
		hiQ	= QUEUE_HIGH;
	} else {	/* for WMM */
		beQ	= QUEUE_LOW;
		bkQ	= QUEUE_NORMAL;
		viQ	= QUEUE_NORMAL;
		voQ	= QUEUE_HIGH;
		mgtQ 	= QUEUE_HIGH;
		hiQ	= QUEUE_HIGH;
	}
	_InitNormalChipRegPriority_8812AUsb(Adapter, beQ, bkQ, viQ, voQ, mgtQ, hiQ);
}

static VOID _InitQueuePriority_8812AUsb(struct rtl_priv *Adapter)
{
	HAL_DATA_TYPE	*pHalData	= GET_HAL_DATA(Adapter);

	switch (pHalData->OutEpNumber) {
	case 2:
		_InitNormalChipTwoOutEpPriority_8812AUsb(Adapter);
		break;
	case 3:
	case 4:
		_InitNormalChipThreeOutEpPriority_8812AUsb(Adapter);
		break;
	default:
		DBG_871X("_InitQueuePriority_8812AUsb(): Shall not reach here!\n");
		break;
	}
}



static VOID _InitHardwareDropIncorrectBulkOut_8812A(struct rtl_priv *Adapter)
{
	uint32_t value32 = rtw_read32(Adapter, REG_TXDMA_OFFSET_CHK);
	value32 |= DROP_DATA_EN;
	rtw_write32(Adapter, REG_TXDMA_OFFSET_CHK, value32);
}

static VOID _InitNetworkType_8812A(struct rtl_priv *Adapter)
{
	uint32_t	value32;

	value32 = rtw_read32(Adapter, REG_CR);
	/*  TODO: use the other function to set network type */
	value32 = (value32 & ~MASK_NETTYPE) | _NETTYPE(NT_LINK_AP);

	rtw_write32(Adapter, REG_CR, value32);
}

static VOID _InitTransferPageSize_8812AUsb(struct rtl_priv *Adapter)
{
	uint8_t	value8;
	value8 = _PSTX(PBP_512);

	PlatformEFIOWrite1Byte(Adapter, REG_PBP, value8);
}

static VOID _InitDriverInfoSize_8812A(struct rtl_priv *Adapter, uint8_t	drvInfoSize)
{
	rtw_write8(Adapter, REG_RX_DRVINFO_SZ, drvInfoSize);
}

static VOID _InitWMACSetting_8812A(struct rtl_priv *Adapter)
{
	/* uint32_t			value32; */
	/* uint16_t			value16; */
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(Adapter);

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

	if (IS_HARDWARE_TYPE_8812AU(Adapter) || IS_HARDWARE_TYPE_8821U(Adapter))
		pHalData->ReceiveConfig |= FORCEACK;

	/*
	 *  some REG_RCR will be modified later by phy_ConfigMACWithHeaderFile()
	 */
	rtw_write32(Adapter, REG_RCR, pHalData->ReceiveConfig);

	/* Accept all multicast address */
	rtw_write32(Adapter, REG_MAR, 0xFFFFFFFF);
	rtw_write32(Adapter, REG_MAR + 4, 0xFFFFFFFF);


	/*
	 *  Accept all data frames
	 * value16 = 0xFFFF;
	 * rtw_write16(Adapter, REG_RXFLTMAP2, value16);
	 */

	/*
	 * 2010.09.08 hpfan
	 * Since ADF is removed from RCR, ps-poll will not be indicate to driver,
	 * RxFilterMap should mask ps-poll to gurantee AP mode can rx ps-poll.
	 * value16 = 0x400;
	 * rtw_write16(Adapter, REG_RXFLTMAP1, value16);
	 */

	/*
	 *  Accept all management frames
	 * value16 = 0xFFFF;
	 * rtw_write16(Adapter, REG_RXFLTMAP0, value16);
	 */

	/*
	 * enable RX_SHIFT bits
	 * rtw_write8(Adapter, REG_TRXDMA_CTRL, rtw_read8(Adapter, REG_TRXDMA_CTRL)|BIT(1));
	 */

}

static VOID _InitAdaptiveCtrl_8812AUsb(IN struct rtl_priv *Adapter)
{
	uint16_t	value16;
	uint32_t	value32;

	/* Response Rate Set */
	value32 = rtw_read32(Adapter, REG_RRSR);
	value32 &= ~RATE_BITMAP_ALL;

	if (Adapter->registrypriv.wireless_mode & WIRELESS_11B)
		value32 |= RATE_RRSR_CCK_ONLY_1M;
	else
		value32 |= RATE_RRSR_WITHOUT_CCK;

	value32 |= RATE_RRSR_CCK_ONLY_1M;
	rtw_write32(Adapter, REG_RRSR, value32);

	/*
	 * CF-END Threshold
	 * m_spIoBase->rtw_write8(REG_CFEND_TH, 0x1);
	 */

	/* SIFS (used in NAV) */
	value16 = _SPEC_SIFS_CCK(0x10) | _SPEC_SIFS_OFDM(0x10);
	rtw_write16(Adapter, REG_SPEC_SIFS, value16);

	/* Retry Limit */
	value16 = _LRL(0x30) | _SRL(0x30);
	rtw_write16(Adapter, REG_RL, value16);

}

static VOID _InitEDCA_8812AUsb(struct rtl_priv *Adapter)
{
	/* Set Spec SIFS (used in NAV) */
	rtw_write16(Adapter, REG_SPEC_SIFS, 0x100a);
	rtw_write16(Adapter, REG_MAC_SPEC_SIFS, 0x100a);

	/* Set SIFS for CCK */
	rtw_write16(Adapter, REG_SIFS_CTX, 0x100a);

	/* Set SIFS for OFDM */
	rtw_write16(Adapter, REG_SIFS_TRX, 0x100a);

	/* TXOP */
	rtw_write32(Adapter, REG_EDCA_BE_PARAM, 0x005EA42B);
	rtw_write32(Adapter, REG_EDCA_BK_PARAM, 0x0000A44F);
	rtw_write32(Adapter, REG_EDCA_VI_PARAM, 0x005EA324);
	rtw_write32(Adapter, REG_EDCA_VO_PARAM, 0x002FA226);

	/* 0x50 for 80MHz clock */
	rtw_write8(Adapter, REG_USTIME_TSF, 0x50);
	rtw_write8(Adapter, REG_USTIME_EDCA, 0x50);
}


static VOID _InitBeaconMaxError_8812A(struct rtl_priv *Adapter, BOOLEAN	InfraMode)
{
	/* ULLI: looks here is some hacking done, wrong nams ?? */
#ifdef RTL8192CU_ADHOC_WORKAROUND_SETTING
	rtw_write8(Adapter, REG_BCN_MAX_ERR, 0xFF);
#else
	/* rtw_write8(Adapter, REG_BCN_MAX_ERR, (InfraMode ? 0xFF : 0x10)); */
#endif
}


static void _InitHWLed(struct rtl_priv *Adapter)
{
	struct led_priv *pledpriv = &(Adapter->ledpriv);

	if (pledpriv->LedStrategy != HW_LED)
		return;

/*
 * HW led control
 * to do ....
 * must consider cases of antenna diversity/ commbo card/solo card/mini card
 */

}

static VOID _InitRDGSetting_8812A(struct rtl_priv *Adapter)
{
	rtw_write8(Adapter, REG_RD_CTRL, 0xFF);
	rtw_write16(Adapter, REG_RD_NAV_NXT, 0x200);
	rtw_write8(Adapter, REG_RD_RESP_PKT_TH, 0x05);
}

static VOID _InitRxSetting_8812AU(struct rtl_priv *Adapter)
{
	rtw_write32(Adapter, REG_MACID, 0x87654321);
	/* ULLI unknown register */
	rtw_write32(Adapter, 0x0700, 0x87654321);
}

static VOID _InitRetryFunction_8812A(IN  struct rtl_priv *Adapter)
{
	uint8_t	value8;

	value8 = rtw_read8(Adapter, REG_FWHW_TXQ_CTRL);
	value8 |= EN_AMPDU_RTY_NEW;
	rtw_write8(Adapter, REG_FWHW_TXQ_CTRL, value8);

	/*
	 * Set ACK timeout
	 * rtw_write8(Adapter, REG_ACKTO, 0x40);  //masked by page for BCM IOT issue temporally
	 */
	rtw_write8(Adapter, REG_ACKTO, 0x80);
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
static VOID usb_AggSettingTxUpdate_8812A(struct rtl_priv *Adapter)
{
#ifdef CONFIG_USB_TX_AGGREGATION
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(Adapter);
	uint32_t			value32;

	if (Adapter->registrypriv.wifi_spec)
		pHalData->UsbTxAggMode = _FALSE;

	if (pHalData->UsbTxAggMode) {
		value32 = rtw_read32(Adapter, REG_TDECTRL);
		value32 = value32 & ~(BLK_DESC_NUM_MASK << BLK_DESC_NUM_SHIFT);
		value32 |= ((pHalData->UsbTxAggDescNum & BLK_DESC_NUM_MASK) << BLK_DESC_NUM_SHIFT);

		rtw_write32(Adapter, REG_TDECTRL, value32);
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
static VOID usb_AggSettingRxUpdate_8812A(struct rtl_priv *Adapter)
{
#ifdef CONFIG_USB_RX_AGGREGATION
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(Adapter);
	uint8_t			valueDMA;
	uint8_t			valueUSB;

	valueDMA = rtw_read8(Adapter, REG_TRXDMA_CTRL);

	switch (pHalData->UsbRxAggMode) {
	case USB_RX_AGG_DMA:
		valueDMA |= RXDMA_AGG_EN;

		/* rtw_write8(Adapter, REG_RXDMA_AGG_PG_TH, 0x05); //dma agg mode, 20k
		 *
		 * 2012/10/26 MH For TX throught start rate temp fix.
		 */
		{
			uint16_t			temp;

			/* Adjust DMA page and thresh. */
			temp = pHalData->RegAcUsbDmaSize | (pHalData->RegAcUsbDmaTime<<8);
			rtw_write16(Adapter, REG_RXDMA_AGG_PG_TH, temp);
		}
		break;
	case USB_RX_AGG_USB:
	case USB_RX_AGG_MIX:
	case USB_RX_AGG_DISABLE:
	default:
		/* TODO: */
		break;
	}

	rtw_write8(Adapter, REG_TRXDMA_CTRL, valueDMA);
#endif
}

static VOID init_UsbAggregationSetting_8812A(struct rtl_priv *Adapter)
{
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(Adapter);

	/* Tx aggregation setting */
	usb_AggSettingTxUpdate_8812A(Adapter);

	/* Rx aggregation setting */
	usb_AggSettingRxUpdate_8812A(Adapter);

	/* 201/12/10 MH Add for USB agg mode dynamic switch. */
	pHalData->UsbRxHighSpeedMode = _FALSE;
}

/*-----------------------------------------------------------------------------
 * Function:	USB_AggModeSwitch()
 *
 * Overview:	When RX traffic is more than 40M, we need to adjust some parameters to increase
 *			RX speed by increasing batch indication size. This will decrease TCP ACK speed, we
 *			need to monitor the influence of FTP/network share.
 *			For TX mode, we are still ubder investigation.
 *
 * Input:		_ADAPTER
 *
 * Output:		NONE
 *
 * Return:		NONE
 *
 * Revised History:
 *	When		Who		Remark
 *	12/10/2010	MHC		Create Version 0.
 *
 *---------------------------------------------------------------------------*/
VOID USB_AggModeSwitch(struct rtl_priv *Adapter)
{
}

static VOID _InitOperationMode_8812A(struct rtl_priv *Adapter)
{
}


/* Set CCK and OFDM Block "ON" */
static VOID _BBTurnOnBlock(struct rtl_priv *Adapter)
{
#if (DISABLE_BB_RF)
	return;
#endif

	rtl_set_bbreg(Adapter, rFPGA0_RFMOD, bCCKEn, 0x1);
	rtl_set_bbreg(Adapter, rFPGA0_RFMOD, bOFDMEn, 0x1);
}

static VOID _RfPowerSave(struct rtl_priv *Adapter)
{
}

enum {
	Antenna_Lfet = 1,
	Antenna_Right = 2,
};

static VOID _InitAntenna_Selection_8812A(struct rtl_priv *Adapter)
{
	HAL_DATA_TYPE	*pHalData	= GET_HAL_DATA(Adapter);

	if (pHalData->AntDivCfg == 0)
		return;
	DBG_8192C("==>  %s ....\n", __FUNCTION__);

	rtw_write8(Adapter, REG_LEDCFG2, 0x82);

	rtl_set_bbreg(Adapter, rFPGA0_XAB_RFParameter, BIT13, 0x01);

	if (rtl_get_bbreg(Adapter, rFPGA0_XA_RFInterfaceOE, 0x300) == MAIN_ANT)
		pHalData->CurAntenna = MAIN_ANT;
	else
		pHalData->CurAntenna = AUX_ANT;
	DBG_8192C("%s,Cur_ant:(%x)%s\n", __FUNCTION__, pHalData->CurAntenna, (pHalData->CurAntenna == MAIN_ANT) ? "MAIN_ANT" : "AUX_ANT");


}

/*
 * 2010/08/26 MH Add for selective suspend mode check.
 * If Efuse 0x0e bit1 is not enabled, we can not support selective suspend for Minicard and
 * slim card.
 */
static VOID HalDetectSelectiveSuspendMode(struct rtl_priv *Adapter)
{
}
/*-----------------------------------------------------------------------------
 * Function:	HwSuspendModeEnable92Cu()
 *
 * Overview:	HW suspend mode switch.
 *
 * Input:		NONE
 *
 * Output:	NONE
 *
 * Return:	NONE
 *
 * Revised History:
 *	When		Who		Remark
 *	08/23/2010	MHC		HW suspend mode switch test..
 *---------------------------------------------------------------------------*/
static VOID HwSuspendModeEnable_8812AU(struct rtl_priv *pAdapter, uint8_t Type)
{
	/* PRT_USB_DEVICE 		pDevice = GET_RT_USB_DEVICE(pAdapter); */
	uint16_t	reg = rtw_read16(pAdapter, REG_GPIO_MUXCFG);

	/* if (!pDevice->RegUsbSS) */
	{
		return;
	}

	/*
	 * 2010/08/23 MH According to Alfred's suggestion, we need to to prevent HW
	 * to enter suspend mode automatically. Otherwise, it will shut down major power
	 * domain and 8051 will stop. When we try to enter selective suspend mode, we
	 * need to prevent HW to enter D2 mode aumotmatically. Another way, Host will
	 * issue a S10 signal to power domain. Then it will cleat SIC setting(from Yngli).
	 * We need to enable HW suspend mode when enter S3/S4 or disable. We need
	 * to disable HW suspend mode for IPS/radio_off.
	 *
	 * RT_TRACE(COMP_RF, DBG_LOUD, ("HwSuspendModeEnable92Cu = %d\n", Type));
	 */
	if (Type == _FALSE) {
		reg |= BIT14;
		/* RT_TRACE(COMP_RF, DBG_LOUD, ("REG_GPIO_MUXCFG = %x\n", reg)); */
		rtw_write16(pAdapter, REG_GPIO_MUXCFG, reg);
		reg |= BIT12;
		/* RT_TRACE(COMP_RF, DBG_LOUD, ("REG_GPIO_MUXCFG = %x\n", reg)); */
		rtw_write16(pAdapter, REG_GPIO_MUXCFG, reg);
	} else {
		reg &= (~BIT12);
		rtw_write16(pAdapter, REG_GPIO_MUXCFG, reg);
		reg &= (~BIT14);
		rtw_write16(pAdapter, REG_GPIO_MUXCFG, reg);
	}
}

rt_rf_power_state RfOnOffDetect(struct rtl_priv *pAdapter)
{
	HAL_DATA_TYPE		*pHalData = GET_HAL_DATA(pAdapter);
	uint8_t	val8;
	rt_rf_power_state rfpowerstate = rf_off;

	if (pAdapter->pwrctrlpriv.bHWPowerdown) {
		val8 = rtw_read8(pAdapter, REG_HSISR);
		DBG_8192C("pwrdown, 0x5c(BIT7)=%02x\n", val8);
		rfpowerstate = (val8 & BIT7) ? rf_off : rf_on;
	} else { /* rf on/off */
		rtw_write8(pAdapter, REG_MAC_PINMUX_CFG, rtw_read8(pAdapter, REG_MAC_PINMUX_CFG)&~(BIT3));
		val8 = rtw_read8(pAdapter, REG_GPIO_IO_SEL);
		DBG_8192C("GPIO_IN=%02x\n", val8);
		rfpowerstate = (val8 & BIT3) ? rf_on : rf_off;
	}
	return rfpowerstate;
}

void _ps_open_RF(struct rtl_priv *padapter)
{
	/*
	 * here call with bRegSSPwrLvl 1, bRegSSPwrLvl 2 needs to be verified
	 * phy_SsPwrSwitch92CU(padapter, rf_on, 1);
	 */
}

void _ps_close_RF(struct rtl_priv *padapter)
{
	/*
	 * here call with bRegSSPwrLvl 1, bRegSSPwrLvl 2 needs to be verified
	 * phy_SsPwrSwitch92CU(padapter, rf_off, 1);
	 */
}

uint32_t rtl8812au_hal_init(struct rtl_priv *Adapter)
{
	uint8_t	value8 = 0, u1bRegCR;
	uint16_t  value16;
	uint8_t	txpktbuf_bndy;
	uint32_t	status = _SUCCESS;
	HAL_DATA_TYPE *pHalData = GET_HAL_DATA(Adapter);
	struct pwrctrl_priv *pwrctrlpriv = &Adapter->pwrctrlpriv;
	struct registry_priv *pregistrypriv = &Adapter->registrypriv;

	DBG_871X(" ULLI: Call rtl8812au_hal_init in usb_halinit.c\n");


	rt_rf_power_state eRfPowerStateToSet;

	uint32_t init_start_time = jiffies;

#ifdef DBG_HAL_INIT_PROFILING
	enum HAL_INIT_STAGES {
		HAL_INIT_STAGES_BEGIN = 0,
		HAL_INIT_STAGES_INIT_PW_ON,
		HAL_INIT_STAGES_INIT_LLTT,
		HAL_INIT_STAGES_DOWNLOAD_FW,
		HAL_INIT_STAGES_MAC,
		HAL_INIT_STAGES_MISC01,
		HAL_INIT_STAGES_MISC02,
		HAL_INIT_STAGES_BB,
		HAL_INIT_STAGES_RF,
		HAL_INIT_STAGES_TURN_ON_BLOCK,
		HAL_INIT_STAGES_INIT_SECURITY,
		HAL_INIT_STAGES_MISC11,
		HAL_INIT_STAGES_INIT_HAL_DM,
		/* HAL_INIT_STAGES_RF_PS, */
		HAL_INIT_STAGES_IQK,
		HAL_INIT_STAGES_PW_TRACK,
		HAL_INIT_STAGES_LCK,
		HAL_INIT_STAGES_MISC21,
		/* HAL_INIT_STAGES_INIT_PABIAS, */
		/* HAL_INIT_STAGES_ANTENNA_SEL, */
		HAL_INIT_STAGES_MISC31,
		HAL_INIT_STAGES_END,
		HAL_INIT_STAGES_NUM
	};

	char *hal_init_stages_str[] = {
		"HAL_INIT_STAGES_BEGIN",
		"HAL_INIT_STAGES_INIT_PW_ON",
		"HAL_INIT_STAGES_INIT_LLTT",
		"HAL_INIT_STAGES_DOWNLOAD_FW",
		"HAL_INIT_STAGES_MAC",
		"HAL_INIT_STAGES_MISC01",
		"HAL_INIT_STAGES_MISC02",
		"HAL_INIT_STAGES_BB",
		"HAL_INIT_STAGES_RF",
		"HAL_INIT_STAGES_TURN_ON_BLOCK",
		"HAL_INIT_STAGES_INIT_SECURITY",
		"HAL_INIT_STAGES_MISC11",
		"HAL_INIT_STAGES_INIT_HAL_DM",
		/* "HAL_INIT_STAGES_RF_PS", */
		"HAL_INIT_STAGES_IQK",
		"HAL_INIT_STAGES_PW_TRACK",
		"HAL_INIT_STAGES_LCK",
		"HAL_INIT_STAGES_MISC21",
		/* "HAL_INIT_STAGES_ANTENNA_SEL", */
		"HAL_INIT_STAGES_MISC31",
		"HAL_INIT_STAGES_END",
	};

	int hal_init_profiling_i;
	uint32_t hal_init_stages_timestamp[HAL_INIT_STAGES_NUM]; /* used to record the time of each stage's starting point */

	for (hal_init_profiling_i = 0; hal_init_profiling_i < HAL_INIT_STAGES_NUM; hal_init_profiling_i++)
		hal_init_stages_timestamp[hal_init_profiling_i] = 0;

	#define HAL_INIT_PROFILE_TAG(stage) do { hal_init_stages_timestamp[(stage)] = jiffies; }  while ()
#else
	#define HAL_INIT_PROFILE_TAG(stage) do {} while (0)
#endif


HAL_INIT_PROFILE_TAG(HAL_INIT_STAGES_BEGIN);

	if (Adapter->pwrctrlpriv.bkeepfwalive) {
		_ps_open_RF(Adapter);

		if (pHalData->odmpriv.RFCalibrateInfo.bIQKInitialized) {
			/* PHY_IQCalibrate_8812A(Adapter,_TRUE); */
		} else {
			/* PHY_IQCalibrate_8812A(Adapter,_FALSE); */
			pHalData->odmpriv.RFCalibrateInfo.bIQKInitialized = _TRUE;
		}

		/*
		 * ODM_TXPowerTrackingCheck(&pHalData->odmpriv );
		 * PHY_LCCalibrate_8812A(Adapter);
		 */

		goto exit;
	}

	/* Check if MAC has already power on. by tynli. 2011.05.27. */
	value8 = rtw_read8(Adapter, REG_SYS_CLKR+1);
	u1bRegCR = PlatformEFIORead1Byte(Adapter, REG_CR);
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
	if (!IS_HARDWARE_TYPE_8821(Adapter)) {
		rtw_write8(Adapter, REG_RF_CTRL, 5);
		rtw_write8(Adapter, REG_RF_CTRL, 7);
		rtw_write8(Adapter, REG_RF_B_CTRL_8812, 5);
		rtw_write8(Adapter, REG_RF_B_CTRL_8812, 7);
	}

HAL_INIT_PROFILE_TAG(HAL_INIT_STAGES_INIT_PW_ON);
	status = _InitPowerOn8812AU(Adapter);
	if (status == _FAIL) {
		RT_TRACE(_module_hci_hal_init_c_, _drv_err_, ("Failed to init power on!\n"));
		goto exit;
	}

HAL_INIT_PROFILE_TAG(HAL_INIT_STAGES_INIT_LLTT);
	if (!pregistrypriv->wifi_spec) {
		if (IS_HARDWARE_TYPE_8812(Adapter))
			txpktbuf_bndy = TX_PAGE_BOUNDARY_8812;
		else
			txpktbuf_bndy = TX_PAGE_BOUNDARY_8821;
	} else {
		/* for WMM */
		if (IS_HARDWARE_TYPE_8812(Adapter))
			txpktbuf_bndy = WMM_NORMAL_TX_PAGE_BOUNDARY_8812;
		else
			txpktbuf_bndy = WMM_NORMAL_TX_PAGE_BOUNDARY_8821;
	}

	status =  InitLLTTable8812(Adapter, txpktbuf_bndy);
	if (status == _FAIL) {
		RT_TRACE(_module_hci_hal_init_c_, _drv_err_, ("Failed to init LLT table\n"));
		goto exit;
	}

#if ENABLE_USB_DROP_INCORRECT_OUT
	_InitHardwareDropIncorrectBulkOut_8812A(Adapter);
#endif

	if (pHalData->bRDGEnable) {
		_InitRDGSetting_8812A(Adapter);
	}

HAL_INIT_PROFILE_TAG(HAL_INIT_STAGES_DOWNLOAD_FW);
	{
		status = FirmwareDownload8812(Adapter, _FALSE);
		if (status != _SUCCESS) {
			DBG_871X("%s: Download Firmware failed!!\n", __FUNCTION__);
			Adapter->bFWReady = _FALSE;
			pHalData->fw_ractrl = _FALSE;
			/* return status; */
		} else {
			DBG_871X("%s: Download Firmware Success!!\n", __FUNCTION__);
			Adapter->bFWReady = _TRUE;
			pHalData->fw_ractrl = _TRUE;
		}
	}

	InitializeFirmwareVars8812(Adapter);

	if (pwrctrlpriv->reg_rfoff == _TRUE) {
		pwrctrlpriv->rf_pwrstate = rf_off;
	}

	/*
	 * 2010/08/09 MH We need to check if we need to turnon or off RF after detecting
	 * HW GPIO pin. Before PHY_RFConfig8192C.
	 * HalDetectPwrDownMode(Adapter);
	 * 2010/08/26 MH If Efuse does not support sective suspend then disable the function.
	 * HalDetectSelectiveSuspendMode(Adapter);
	 */

	/*
	 * Save target channel
	 * <Roger_Notes> Current Channel will be updated again later.
	 */
	pHalData->CurrentChannel = 0;	/* set 0 to trigger switch correct channel */

HAL_INIT_PROFILE_TAG(HAL_INIT_STAGES_MAC);
#if (HAL_MAC_ENABLE == 1)
	status = PHY_MACConfig8812(Adapter);
	if (status == _FAIL) {
		goto exit;
	}
#endif

HAL_INIT_PROFILE_TAG(HAL_INIT_STAGES_MISC01);
	if (IS_HARDWARE_TYPE_8812(Adapter)) {
		_InitQueueReservedPage_8812AUsb(Adapter);
		_InitTxBufferBoundary_8812AUsb(Adapter);
	} else if (IS_HARDWARE_TYPE_8821(Adapter)) {
		_InitQueueReservedPage_8821AUsb(Adapter);
		_InitTxBufferBoundary_8821AUsb(Adapter);
	}

	_InitQueuePriority_8812AUsb(Adapter);
	_InitPageBoundary_8812AUsb(Adapter);

	if (IS_HARDWARE_TYPE_8812(Adapter))
		_InitTransferPageSize_8812AUsb(Adapter);

HAL_INIT_PROFILE_TAG(HAL_INIT_STAGES_MISC02);
	/* Get Rx PHY status in order to report RSSI and others. */
	_InitDriverInfoSize_8812A(Adapter, DRVINFO_SZ);

	_InitInterrupt_8812AU(Adapter);
	_InitID_8812A(Adapter);			/* set mac_address */
	_InitNetworkType_8812A(Adapter);	/* set msr */
	_InitWMACSetting_8812A(Adapter);
	_InitAdaptiveCtrl_8812AUsb(Adapter);
	_InitEDCA_8812AUsb(Adapter);

	_InitRetryFunction_8812A(Adapter);
	init_UsbAggregationSetting_8812A(Adapter);
	_InitOperationMode_8812A(Adapter);	/* todo */
	_InitBeaconParameters_8812A(Adapter);
	_InitBeaconMaxError_8812A(Adapter, _TRUE);

	_InitBurstPktLen(Adapter);  /* added by page. 20110919 */

	/*
	 * Init CR MACTXEN, MACRXEN after setting RxFF boundary REG_TRXFF_BNDY to patch
	 * Hw bug which Hw initials RxFF boundry size to a value which is larger than the real Rx buffer size in 88E.
	 * 2011.08.05. by tynli.
	 */
	value8 = rtw_read8(Adapter, REG_CR);
	rtw_write8(Adapter, REG_CR, (value8|MACTXEN|MACRXEN));

#if defined(CONFIG_TX_MCAST2UNI)

#ifdef CONFIG_TX_MCAST2UNI
	rtw_write16(Adapter, REG_PKT_VO_VI_LIFE_TIME, 0x0400);	/* unit: 256us. 256ms */
	rtw_write16(Adapter, REG_PKT_BE_BK_LIFE_TIME, 0x0400);	/* unit: 256us. 256ms */
#else
	rtw_write16(Adapter, REG_PKT_VO_VI_LIFE_TIME, 0x3000);	/* unit: 256us. 3s */
	rtw_write16(Adapter, REG_PKT_BE_BK_LIFE_TIME, 0x3000);	/* unit: 256us. 3s */
#endif
#endif


	_InitHWLed(Adapter);

	/*
	 * d. Initialize BB related configurations.
	 */

HAL_INIT_PROFILE_TAG(HAL_INIT_STAGES_BB);

#if (HAL_BB_ENABLE == 1)
	status = PHY_BBConfig8812(Adapter);
	if (status == _FAIL) {
		goto exit;
	}
#endif

	/*
	 * 92CU use 3-wire to r/w RF
	 * pHalData->Rf_Mode = RF_OP_By_SW_3wire;
	 */

HAL_INIT_PROFILE_TAG(HAL_INIT_STAGES_RF);
#if (HAL_RF_ENABLE == 1)
	status = PHY_RFConfig8812(Adapter);
	if (status == _FAIL) {
		goto exit;
	}

	if (pHalData->rf_type == RF_1T1R && IS_HARDWARE_TYPE_8812AU(Adapter))
		PHY_BB8812_Config_1T(Adapter);
#endif

	if (Adapter->registrypriv.channel <= 14)
		PHY_SwitchWirelessBand8812(Adapter, BAND_ON_2_4G);
	else
		PHY_SwitchWirelessBand8812(Adapter, BAND_ON_5G);

	rtw_hal_set_chnl_bw(Adapter, Adapter->registrypriv.channel,
		CHANNEL_WIDTH_20, HAL_PRIME_CHNL_OFFSET_DONT_CARE, HAL_PRIME_CHNL_OFFSET_DONT_CARE);

HAL_INIT_PROFILE_TAG(HAL_INIT_STAGES_TURN_ON_BLOCK);

HAL_INIT_PROFILE_TAG(HAL_INIT_STAGES_INIT_SECURITY);

	invalidate_cam_all(Adapter);

HAL_INIT_PROFILE_TAG(HAL_INIT_STAGES_MISC11);

	_InitAntenna_Selection_8812A(Adapter);

	/*
	 * HW SEQ CTRL
	 * set 0x0 to 0xFF by tynli. Default enable HW SEQ NUM.
	 */
	rtw_write8(Adapter, REG_HWSEQ_CTRL, 0xFF);

	/*
	 * Disable BAR, suggested by Scott
	 * 2010.04.09 add by hpfan
	 */
	rtw_write32(Adapter, REG_BAR_MODE_CTRL, 0x0201ffff);

	if (pregistrypriv->wifi_spec)
		rtw_write16(Adapter, REG_FAST_EDCA_CTRL, 0);

	/* Nav limit , suggest by scott */
	rtw_write8(Adapter, 0x652, 0x0);

HAL_INIT_PROFILE_TAG(HAL_INIT_STAGES_INIT_HAL_DM);

	rtl8812_InitHalDm(Adapter);

		{
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
		rtw_write8(Adapter, REG_QUEUE_CTRL, rtw_read8(Adapter, REG_QUEUE_CTRL)&0xF7);

		/* enable Tx report. */
		rtw_write8(Adapter,  REG_FWHW_TXQ_CTRL+1, 0x0F);

		/* Suggested by SD1 pisa. Added by tynli. 2011.10.21. */
		rtw_write8(Adapter, REG_EARLY_MODE_CONTROL_8812+3, 0x01);/* Pretx_en, for WEP/TKIP SEC */

		/* tynli_test_tx_report. */
		rtw_write16(Adapter, REG_TX_RPT_TIME, 0x3DF0);

		/* Reset USB mode switch setting */
		rtw_write8(Adapter, REG_SDIO_CTRL_8812, 0x0);
		rtw_write8(Adapter, REG_ACLK_MON, 0x0);

		/*
		 * RT_TRACE(COMP_INIT, DBG_TRACE, ("InitializeAdapter8188EUsb() <====\n"));
		 */

HAL_INIT_PROFILE_TAG(HAL_INIT_STAGES_IQK);
		/* 2010/08/26 MH Merge from 8192CE. */
		if (pwrctrlpriv->rf_pwrstate == rf_on) {
			if (IS_HARDWARE_TYPE_8812AU(Adapter)) {
#if (RTL8812A_SUPPORT == 1)
				pHalData->odmpriv.RFCalibrateInfo.bNeedIQK = _TRUE;
				if (pHalData->odmpriv.RFCalibrateInfo.bIQKInitialized)
					PHY_IQCalibrate_8812A(Adapter, _TRUE);
				else {
					PHY_IQCalibrate_8812A(Adapter, _FALSE);
					pHalData->odmpriv.RFCalibrateInfo.bIQKInitialized = _TRUE;
				}
#endif
			}

HAL_INIT_PROFILE_TAG(HAL_INIT_STAGES_PW_TRACK);

			/* ODM_TXPowerTrackingCheck(&pHalData->odmpriv ); */


HAL_INIT_PROFILE_TAG(HAL_INIT_STAGES_LCK);
			/* PHY_LCCalibrate_8812A(Adapter); */
		}
	}

HAL_INIT_PROFILE_TAG(HAL_INIT_STAGES_MISC21);


	/* HAL_INIT_PROFILE_TAG(HAL_INIT_STAGES_INIT_PABIAS);
	 * _InitPABias(Adapter);
	 */

	/*
	 *  2010/08/23 MH According to Alfred's suggestion, we need to to prevent HW enter
	 *  suspend mode automatically.
	 * HwSuspendModeEnable92Cu(Adapter, _FALSE);
	 */

HAL_INIT_PROFILE_TAG(HAL_INIT_STAGES_MISC31);

	rtw_write8(Adapter, REG_USB_HRPWM, 0);

	/* misc */
	{
		int i;
		uint8_t mac_addr[6];
		for (i = 0; i < 6; i++) {
			mac_addr[i] = rtw_read8(Adapter, REG_MACID+i);
		}

		DBG_8192C("MAC Address from REG_MACID = "MAC_FMT"\n", MAC_ARG(mac_addr));
	}

exit:
HAL_INIT_PROFILE_TAG(HAL_INIT_STAGES_END);

	DBG_871X("%s in %dms\n", __FUNCTION__, rtw_get_passing_time_ms(init_start_time));

#ifdef DBG_HAL_INIT_PROFILING
	hal_init_stages_timestamp[HAL_INIT_STAGES_END] = jiffies;

	for (hal_init_profiling_i = 0; hal_init_profiling_i < HAL_INIT_STAGES_NUM-1; hal_init_profiling_i++) {
		DBG_871X("DBG_HAL_INIT_PROFILING: %35s, %u, %5u, %5u\n"
			, hal_init_stages_str[hal_init_profiling_i]
			, hal_init_stages_timestamp[hal_init_profiling_i]
			, (hal_init_stages_timestamp[hal_init_profiling_i+1]-hal_init_stages_timestamp[hal_init_profiling_i])
			, rtw_get_time_interval_ms(hal_init_stages_timestamp[hal_init_profiling_i], hal_init_stages_timestamp[hal_init_profiling_i+1])
		);
	}
#endif

	return status;
}

VOID CardDisableRTL8812AU(struct rtl_priv *Adapter)
{
	uint8_t	u1bTmp;
	uint8_t 	val8;
	uint16_t	val16;
	uint32_t	val32;

	/* DBG_871X("CardDisableRTL8188EU\n"); */

	/* Stop Tx Report Timer. 0x4EC[Bit1]=b'0 */
	u1bTmp = rtw_read8(Adapter, REG_TX_RPT_CTRL);
	rtw_write8(Adapter, REG_TX_RPT_CTRL, val8&(~BIT1));

	/* stop rx */
	rtw_write8(Adapter, REG_CR, 0x0);

	/* Run LPS WL RFOFF flow */
	if (IS_HARDWARE_TYPE_8821U(Adapter))
		HalPwrSeqCmdParsing(Adapter, PWR_CUT_ALL_MSK, PWR_FAB_ALL_MSK, PWR_INTF_USB_MSK, Rtl8821A_NIC_LPS_ENTER_FLOW);
	else
		HalPwrSeqCmdParsing(Adapter, PWR_CUT_ALL_MSK, PWR_FAB_ALL_MSK, PWR_INTF_USB_MSK, Rtl8812_NIC_LPS_ENTER_FLOW);

	if ((rtw_read8(Adapter, REG_MCUFWDL)&RAM_DL_SEL) && Adapter->bFWReady) {
		  /* 8051 RAM code */
		_8051Reset8812(Adapter);
	}

	/* Reset MCU. Suggested by Filen. 2011.01.26. by tynli. */
	u1bTmp = rtw_read8(Adapter, REG_SYS_FUNC_EN+1);
	rtw_write8(Adapter, REG_SYS_FUNC_EN+1, (u1bTmp&(~BIT2)));

	/* MCUFWDL 0x80[1:0]=0
	 * reset MCU ready status
	 */
	rtw_write8(Adapter, REG_MCUFWDL, 0x00);

	/* Card disable power action flow */
	if (IS_HARDWARE_TYPE_8821U(Adapter))
		HalPwrSeqCmdParsing(Adapter, PWR_CUT_ALL_MSK, PWR_FAB_ALL_MSK, PWR_INTF_USB_MSK, Rtl8821A_NIC_DISABLE_FLOW);
	else
		HalPwrSeqCmdParsing(Adapter, PWR_CUT_ALL_MSK, PWR_FAB_ALL_MSK, PWR_INTF_USB_MSK, Rtl8812_NIC_DISABLE_FLOW);
}

static void rtl8812au_hw_power_down(struct rtl_priv *padapter)
{
	/*
	 *  2010/-8/09 MH For power down module, we need to enable register block contrl reg at 0x1c.
	 * Then enable power down control bit of register 0x04 BIT4 and BIT15 as 1.
	 */

	/* Enable register area 0x0-0xc. */
	rtw_write8(padapter, REG_RSV_CTRL, 0x0);
	rtw_write16(padapter, REG_APS_FSMCO, 0x8812);
}

uint32_t rtl8812au_hal_deinit(struct rtl_priv *Adapter)
 {

	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(Adapter);
	DBG_8192C("==> %s \n", __FUNCTION__);

	{
		rtw_write16(Adapter, REG_GPIO_MUXCFG, rtw_read16(Adapter, REG_GPIO_MUXCFG)&(~BIT12));
	}

	if (pHalData->bSupportUSB3 == _TRUE) {
		/*
		 * set Reg 0xf008[3:4] to 2'11 to eable U1/U2 Mode in USB3.0. added by page, 20120712
		 */
		rtw_write8(Adapter, 0xf008, rtw_read8(Adapter, 0xf008)|0x18);
	}

	rtw_write32(Adapter, REG_HISR0_8812, 0xFFFFFFFF);
	rtw_write32(Adapter, REG_HISR1_8812, 0xFFFFFFFF);
	rtw_write32(Adapter, REG_HIMR0_8812, IMR_DISABLED_8812);
	rtw_write32(Adapter, REG_HIMR1_8812, IMR_DISABLED_8812);

	{
		if (Adapter->hw_init_completed == _TRUE) {
			CardDisableRTL8812AU(Adapter);

			if ((Adapter->pwrctrlpriv.bHWPwrPindetect) && (Adapter->pwrctrlpriv.bHWPowerdown))
				rtl8812au_hw_power_down(Adapter);
		}
	}
	return _SUCCESS;
 }


unsigned int rtl8812au_inirp_init(struct rtl_priv *Adapter)
{
	uint8_t i;
	struct recv_buf *precvbuf;
	uint	status;
	struct dvobj_priv *pdev = adapter_to_dvobj(Adapter);
	struct intf_hdl *pintfhdl = &Adapter->iopriv.intf;
	struct recv_priv *precvpriv = &(Adapter->recvpriv);
	uint32_t (*_read_port)(struct intf_hdl *pintfhdl, uint32_t addr, uint32_t cnt, uint8_t *pmem);

	_read_port = pintfhdl->io_ops._read_port;

	status = _SUCCESS;

	RT_TRACE(_module_hci_hal_init_c_, _drv_info_, ("===> usb_inirp_init \n"));

	precvpriv->ff_hwaddr = RECV_BULK_IN_ADDR;

	/* issue Rx irp to receive data */
	precvbuf = (struct recv_buf *)precvpriv->precv_buf;
	for (i = 0; i < NR_RECVBUFF; i++) {
		if (_read_port(pintfhdl, precvpriv->ff_hwaddr, 0, (unsigned char *) precvbuf) == _FALSE) {
			RT_TRACE(_module_hci_hal_init_c_, _drv_err_, ("usb_rx_init: usb_read_port error \n"));
			status = _FAIL;
			goto exit;
		}

		precvbuf++;
		precvpriv->free_recv_buf_queue_cnt--;
	}

exit:

	RT_TRACE(_module_hci_hal_init_c_, _drv_info_, ("<=== usb_inirp_init \n"));

	return status;

}

unsigned int rtl8812au_inirp_deinit(struct rtl_priv *Adapter)
{
	RT_TRACE(_module_hci_hal_init_c_, _drv_info_, ("\n ===> usb_rx_deinit \n"));

	rtw_read_port_cancel(Adapter);

	RT_TRACE(_module_hci_hal_init_c_, _drv_info_, ("\n <=== usb_rx_deinit \n"));

	return _SUCCESS;
}

/*
 * -------------------------------------------------------------------
 *
 * 	EEPROM/EFUSE Content Parsing
 *
 * -------------------------------------------------------------------
 */
VOID hal_ReadIDs_8812AU(struct rtl_priv *Adapter, pu1Byte PROMContent,
	BOOLEAN	AutoloadFail)
{
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(Adapter);
	EEPROM_EFUSE_PRIV	*pEEPROM = GET_EEPROM_EFUSE_PRIV(Adapter);

	if (!AutoloadFail) {
		/* VID, PID */
		if (IS_HARDWARE_TYPE_8812AU(Adapter)) {
			pHalData->EEPROMVID = EF2Byte(*(uint16_t *)&PROMContent[EEPROM_VID_8812AU]);
			pHalData->EEPROMPID = EF2Byte(*(uint16_t *)&PROMContent[EEPROM_PID_8812AU]);
		} else if (IS_HARDWARE_TYPE_8821U(Adapter)) {
			pHalData->EEPROMVID = EF2Byte(*(uint16_t *)&PROMContent[EEPROM_VID_8821AU]);
			pHalData->EEPROMPID = EF2Byte(*(uint16_t *)&PROMContent[EEPROM_PID_8821AU]);
		}

		/* Customer ID, 0x00 and 0xff are reserved for Realtek. */
		pHalData->EEPROMCustomerID = *(uint8_t *)&PROMContent[EEPROM_CustomID_8812];
		pHalData->EEPROMSubCustomerID = EEPROM_Default_SubCustomerID;

	} else {
		pHalData->EEPROMVID = EEPROM_Default_VID;
		pHalData->EEPROMPID = EEPROM_Default_PID;

		/* Customer ID, 0x00 and 0xff are reserved for Realtek. */
		pHalData->EEPROMCustomerID		= EEPROM_Default_CustomerID;
		pHalData->EEPROMSubCustomerID	= EEPROM_Default_SubCustomerID;

	}

	if ((pHalData->EEPROMVID == 0x050D) && (pHalData->EEPROMPID == 0x1106))		/* SerComm for Belkin. */
		pEEPROM->CustomerID = RT_CID_819x_Sercomm_Belkin;
	else if ((pHalData->EEPROMVID == 0x0846) && (pHalData->EEPROMPID == 0x9051))	/* SerComm for Netgear. */
		pEEPROM->CustomerID = RT_CID_819x_Sercomm_Netgear;
	else if ((pHalData->EEPROMVID == 0x2001) && (pHalData->EEPROMPID == 0x330e))	/* add by ylb 20121012 for customer led for alpha */
		pEEPROM->CustomerID = RT_CID_819x_ALPHA_Dlink;
	else if ((pHalData->EEPROMVID == 0x0B05) && (pHalData->EEPROMPID == 0x17D2))	/* Edimax for ASUS */
		pEEPROM->CustomerID = RT_CID_819x_Edimax_ASUS;

	DBG_871X("VID = 0x%04X, PID = 0x%04X\n", pHalData->EEPROMVID, pHalData->EEPROMPID);
	DBG_871X("Customer ID: 0x%02X, SubCustomer ID: 0x%02X\n", pHalData->EEPROMCustomerID, pHalData->EEPROMSubCustomerID);
}

VOID hal_ReadMACAddress_8812AU(struct rtl_priv *Adapter, u8 *PROMContent,
	BOOLEAN	AutoloadFail)
{
	EEPROM_EFUSE_PRIV *pEEPROM = GET_EEPROM_EFUSE_PRIV(Adapter);

	if (_FALSE == AutoloadFail) {
		if (IS_HARDWARE_TYPE_8812AU(Adapter)) {
			/* Read Permanent MAC address and set value to hardware */
			memcpy(pEEPROM->mac_addr, &PROMContent[EEPROM_MAC_ADDR_8812AU], ETH_ALEN);
		} else if (IS_HARDWARE_TYPE_8821U(Adapter)) {
			/*  Read Permanent MAC address and set value to hardware */
			memcpy(pEEPROM->mac_addr, &PROMContent[EEPROM_MAC_ADDR_8821AU], ETH_ALEN);
		}
	} else {
		/* Random assigh MAC address */
		uint8_t	sMacAddr[ETH_ALEN] = {0x00, 0xE0, 0x4C, 0x88, 0x12, 0x00};
		/* sMacAddr[5] = (u8)GetRandomNumber(1, 254); */
		memcpy(pEEPROM->mac_addr, sMacAddr, ETH_ALEN);
	}

	DBG_8192C("%s MAC Address from EFUSE = "MAC_FMT"\n", __FUNCTION__, MAC_ARG(pEEPROM->mac_addr));
}

VOID hal_InitPGData_8812A(struct rtl_priv *padapter, u8 *PROMContent)
{
	EEPROM_EFUSE_PRIV *pEEPROM = GET_EEPROM_EFUSE_PRIV(padapter);
	/* HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(padapter); */
	uint32_t			i;
	uint16_t			value16;

	if (_FALSE == pEEPROM->bautoload_fail_flag) { /* autoload OK. */
		if (is_boot_from_eeprom(padapter)) {
			/* Read all Content from EEPROM or EFUSE. */
			for (i = 0; i < HWSET_MAX_SIZE_JAGUAR; i += 2) {
				/*
				 * value16 = EF2Byte(ReadEEprom(pAdapter, (uint16_t) (i>>1)));
				 * *((uint16_t *)(&PROMContent[i])) = value16;
				 */
			}
		} else {
			/* Read EFUSE real map to shadow. */
			EFUSE_ShadowMapUpdate(padapter, EFUSE_WIFI);
		}
	} else {	/* autoload fail */
		RT_TRACE(_module_hci_hal_init_c_, _drv_notice_, ("AutoLoad Fail reported from CR9346!!\n"));
		/*
		 * pHalData->AutoloadFailFlag = _TRUE;
		 * update to default value 0xFF
		 */
		if (!is_boot_from_eeprom(padapter))
			EFUSE_ShadowMapUpdate(padapter, EFUSE_WIFI);
	}
}

VOID hal_CustomizedBehavior_8812AU(struct rtl_priv *Adapter)
{
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(Adapter);
	EEPROM_EFUSE_PRIV	*pEEPROM = GET_EEPROM_EFUSE_PRIV(Adapter);
	struct led_priv	*pledpriv = &(Adapter->ledpriv);

	/* Led mode */
	switch (pEEPROM->CustomerID) {
	case RT_CID_DEFAULT:
		pledpriv->LedStrategy = SW_LED_MODE9;
		pledpriv->bRegUseLed = _TRUE;
		break;

	case RT_CID_819x_HP:
		pledpriv->LedStrategy = SW_LED_MODE6; /* Customize Led mode */
		break;

	/* ULLI check RT_CID_819x values */

	case RT_CID_819x_Sercomm_Belkin:
		pledpriv->LedStrategy = SW_LED_MODE9;
		break;

	case RT_CID_819x_Sercomm_Netgear:
		pledpriv->LedStrategy = SW_LED_MODE10;
		break;

	case RT_CID_819x_ALPHA_Dlink:	/* add by ylb 20121012 for customer led for alpha */
		pledpriv->LedStrategy = SW_LED_MODE1;
		break;

	case RT_CID_819x_Edimax_ASUS:
		pledpriv->LedStrategy = SW_LED_MODE11;
		break;

	case RT_CID_WNC_NEC:
		pledpriv->LedStrategy = SW_LED_MODE12;
		break;

	case RT_CID_NETGEAR:
		pledpriv->LedStrategy = SW_LED_MODE13;
		break;

	default:
		pledpriv->LedStrategy = SW_LED_MODE9;
		break;
	}

	pHalData->bLedOpenDrain = _TRUE;	/* Support Open-drain arrangement for controlling the LED. Added by Roger, 2009.10.16. */
}

static void hal_CustomizeByCustomerID_8812AU(struct rtl_priv *pAdapter)
{
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(pAdapter);
	EEPROM_EFUSE_PRIV	*pEEPROM = GET_EEPROM_EFUSE_PRIV(pAdapter);

	/* For customized behavior. */
	if ((pHalData->EEPROMVID == 0x103C) && (pHalData->EEPROMPID == 0x1629))/* HP Lite-On for RTL8188CUS Slim Combo. */
		pEEPROM->CustomerID = RT_CID_819x_HP;
	else if ((pHalData->EEPROMVID == 0x9846) && (pHalData->EEPROMPID == 0x9041))
		pEEPROM->CustomerID = RT_CID_NETGEAR;
	else if ((pHalData->EEPROMVID == 0x2019) && (pHalData->EEPROMPID == 0x1201))
		pEEPROM->CustomerID = RT_CID_PLANEX;
	else if ((pHalData->EEPROMVID == 0x0BDA) && (pHalData->EEPROMPID == 0x5088))
		pEEPROM->CustomerID = RT_CID_CC_C;

	DBG_871X("PID= 0x%x, VID=  %x\n", pHalData->EEPROMPID, pHalData->EEPROMVID);

	/* Decide CustomerID according to VID/DID or EEPROM */
	switch (pHalData->EEPROMCustomerID) {
	case EEPROM_CID_DEFAULT:
		if ((pHalData->EEPROMVID == 0x2001) && (pHalData->EEPROMPID == 0x3308))
			pEEPROM->CustomerID = RT_CID_DLINK;
		else if ((pHalData->EEPROMVID == 0x2001) && (pHalData->EEPROMPID == 0x3309))
			pEEPROM->CustomerID = RT_CID_DLINK;
		else if ((pHalData->EEPROMVID == 0x2001) && (pHalData->EEPROMPID == 0x330a))
			pEEPROM->CustomerID = RT_CID_DLINK;
		else if ((pHalData->EEPROMVID == 0x0BFF) && (pHalData->EEPROMPID == 0x8160)) {
			/* pHalData->bAutoConnectEnable = _FALSE; */
			pEEPROM->CustomerID = RT_CID_CHINA_MOBILE;
		} else if ((pHalData->EEPROMVID == 0x0BDA) && (pHalData->EEPROMPID == 0x5088))
			pEEPROM->CustomerID = RT_CID_CC_C;
		else if ((pHalData->EEPROMVID == 0x0846) && (pHalData->EEPROMPID == 0x9052))
			pEEPROM->CustomerID = RT_CID_NETGEAR;

		DBG_871X("PID= 0x%x, VID=  %x\n", pHalData->EEPROMPID, pHalData->EEPROMVID);
		break;
	case EEPROM_CID_WHQL:
		/*
		 * padapter->bInHctTest = TRUE;
		 *
		 * pMgntInfo->bSupportTurboMode = FALSE;
		 * pMgntInfo->bAutoTurboBy8186 = FALSE;
		 *
		 * pMgntInfo->PowerSaveControl.bInactivePs = FALSE;
		 * pMgntInfo->PowerSaveControl.bIPSModeBackup = FALSE;
		 * pMgntInfo->PowerSaveControl.bLeisurePs = FALSE;
		 * pMgntInfo->PowerSaveControl.bLeisurePsModeBackup = FALSE;
		 * pMgntInfo->keepAliveLevel = 0;
		 *
		 * padapter->bUnloadDriverwhenS3S4 = FALSE;
		 */
		break;
	default:
		pEEPROM->CustomerID = RT_CID_DEFAULT;
		break;

	}
	DBG_871X("Customer ID: 0x%2x\n", pEEPROM->CustomerID);

	hal_CustomizedBehavior_8812AU(pAdapter);
}

VOID hal_ReadUsbModeSwitch_8812AU(struct rtl_priv *Adapter, u8 *PROMContent, BOOLEAN AutoloadFail)
{
}

static VOID ReadLEDSetting_8812AU(struct rtl_priv *Adapter,
	u8 *PROMContent, BOOLEAN AutoloadFail)
{
	struct led_priv *pledpriv = &(Adapter->ledpriv);

	pledpriv->bRegUseLed = _TRUE;
}

VOID InitAdapterVariablesByPROM_8812AU(struct rtl_priv *Adapter)
{
	EEPROM_EFUSE_PRIV *pEEPROM = GET_EEPROM_EFUSE_PRIV(Adapter);

	hal_InitPGData_8812A(Adapter, pEEPROM->efuse_eeprom_data);
	Hal_EfuseParseIDCode8812A(Adapter, pEEPROM->efuse_eeprom_data);

	Hal_ReadPROMVersion8812A(Adapter, pEEPROM->efuse_eeprom_data, pEEPROM->bautoload_fail_flag);
	hal_ReadIDs_8812AU(Adapter, pEEPROM->efuse_eeprom_data, pEEPROM->bautoload_fail_flag);
	hal_ReadMACAddress_8812AU(Adapter, pEEPROM->efuse_eeprom_data, pEEPROM->bautoload_fail_flag);
	Hal_ReadTxPowerInfo8812A(Adapter, pEEPROM->efuse_eeprom_data, pEEPROM->bautoload_fail_flag);
	Hal_ReadBoardType8812A(Adapter, pEEPROM->efuse_eeprom_data, pEEPROM->bautoload_fail_flag);

	/*
	 * Read Bluetooth co-exist and initialize
	 */

	Hal_EfuseParseBTCoexistInfo8812A(Adapter, pEEPROM->efuse_eeprom_data, pEEPROM->bautoload_fail_flag);

	Hal_ReadChannelPlan8812A(Adapter, pEEPROM->efuse_eeprom_data, pEEPROM->bautoload_fail_flag);
	Hal_EfuseParseXtal_8812A(Adapter, pEEPROM->efuse_eeprom_data, pEEPROM->bautoload_fail_flag);
	Hal_ReadThermalMeter_8812A(Adapter, pEEPROM->efuse_eeprom_data, pEEPROM->bautoload_fail_flag);
	Hal_ReadAntennaDiversity8812A(Adapter, pEEPROM->efuse_eeprom_data, pEEPROM->bautoload_fail_flag);

	if (IS_HARDWARE_TYPE_8821U(Adapter)) {
		Hal_ReadPAType_8821A(Adapter, pEEPROM->efuse_eeprom_data, pEEPROM->bautoload_fail_flag);
	} else {
		Hal_ReadPAType_8812A(Adapter, pEEPROM->efuse_eeprom_data, pEEPROM->bautoload_fail_flag);
		Hal_ReadRFEType_8812A(Adapter, pEEPROM->efuse_eeprom_data, pEEPROM->bautoload_fail_flag);
	}

	hal_ReadUsbModeSwitch_8812AU(Adapter, pEEPROM->efuse_eeprom_data, pEEPROM->bautoload_fail_flag);
	hal_CustomizeByCustomerID_8812AU(Adapter);

	ReadLEDSetting_8812AU(Adapter, pEEPROM->efuse_eeprom_data, pEEPROM->bautoload_fail_flag);

	/* 2013/04/15 MH Add for different board type recognize. */
	hal_ReadUsbType_8812AU(Adapter, pEEPROM->efuse_eeprom_data, pEEPROM->bautoload_fail_flag);
}

static void Hal_ReadPROMContent_8812A(struct rtl_priv *Adapter)
{
	EEPROM_EFUSE_PRIV *pEEPROM = GET_EEPROM_EFUSE_PRIV(Adapter);
	uint8_t			eeValue;

	/* check system boot selection */
	eeValue = rtw_read8(Adapter, REG_9346CR);
	pEEPROM->EepromOrEfuse		= (eeValue & BOOT_FROM_EEPROM) ? _TRUE : _FALSE;
	pEEPROM->bautoload_fail_flag	= (eeValue & EEPROM_EN) ? _FALSE : _TRUE;

	DBG_8192C("Boot from %s, Autoload %s !\n", (pEEPROM->EepromOrEfuse ? "EEPROM" : "EFUSE"),
				(pEEPROM->bautoload_fail_flag ? "Fail" : "OK"));

	/* pHalData->EEType = IS_BOOT_FROM_EEPROM(Adapter) ? EEPROM_93C46 : EEPROM_BOOT_EFUSE; */

	InitAdapterVariablesByPROM_8812AU(Adapter);
}

VOID hal_ReadRFType_8812A(struct rtl_priv *Adapter)
{
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(Adapter);

#if DISABLE_BB_RF
	pHalData->rf_chip = RF_PSEUDO_11N;
#else
	pHalData->rf_chip = RF_6052;
#endif


	if (IsSupported24G(Adapter->registrypriv.wireless_mode) &&
		IsSupported5G(Adapter->registrypriv.wireless_mode))
		pHalData->BandSet = BAND_ON_BOTH;
	else if (IsSupported5G(Adapter->registrypriv.wireless_mode))
		pHalData->BandSet = BAND_ON_5G;
	else
		pHalData->BandSet = BAND_ON_2_4G;

	/*
	 * if (Adapter->bInHctTest)
	 * 	pHalData->BandSet = BAND_ON_2_4G;
	 */
}

VOID hal_CustomizedBehavior_8812AUsb(struct rtl_priv * Adapter)
{
}

void ReadAdapterInfo8812AU(struct rtl_priv *Adapter)
{
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(Adapter);

	DBG_871X("====> ReadAdapterInfo8812AU\n");

	/* Read all content in Efuse/EEPROM. */
	Hal_ReadPROMContent_8812A(Adapter);

	/* We need to define the RF type after all PROM value is recognized. */
	hal_ReadRFType_8812A(Adapter);

	/* 2011/02/09 MH We gather the same value for all USB series IC. */
	hal_CustomizedBehavior_8812AUsb(Adapter);

	DBG_871X("ReadAdapterInfo8812AU <====\n");
}

void UpdateInterruptMask8812AU(struct rtl_priv *padapter, uint8_t bHIMR0, uint32_t AddMSR, uint32_t RemoveMSR)
{
	HAL_DATA_TYPE *pHalData;

	uint32_t *himr;
	pHalData = GET_HAL_DATA(padapter);

	if (bHIMR0)
		himr = &(pHalData->IntrMask[0]);
	else
		himr = &(pHalData->IntrMask[1]);

	if (AddMSR)
		*himr |= AddMSR;

	if (RemoveMSR)
		*himr &= (~RemoveMSR);

	if (bHIMR0)
		rtw_write32(padapter, REG_HIMR0_8812, *himr);
	else
		rtw_write32(padapter, REG_HIMR1_8812, *himr);

}

void SetHwReg8812AU(struct rtl_priv *Adapter, uint8_t variable, u8 *val)
{
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(Adapter);
	struct dm_priv	*pdmpriv = &pHalData->dmpriv;
	DM_ODM_T 		*podmpriv = &pHalData->odmpriv;

	switch (variable) {
	case HW_VAR_RXDMA_AGG_PG_TH:
#ifdef CONFIG_USB_RX_AGGREGATION
		{
			/*uint8_t	threshold = *((uint8_t *)val);
			if ( threshold == 0)
			{
				threshold = pHalData->UsbRxAggPageCount;
			}
			rtw_write8(Adapter, REG_RXDMA_AGG_PG_TH, threshold);*/
		}
#endif
		break;
	case HW_VAR_SET_RPWM:
		break;
	case HW_VAR_USB_MODE:
		if (*val == 1)
			rtw_write8(Adapter, REG_OPT_CTRL_8812, 0x4);
		else if (*val == 2)
			rtw_write8(Adapter, REG_OPT_CTRL_8812, 0x8);

		rtw_write8(Adapter, REG_SDIO_CTRL_8812, 0x2);
		rtw_write8(Adapter, REG_ACLK_MON, 0x1);
		/*
		 * 2013/01/29 MH Test with chunchu/cheng, in Alpha AMD platform. when
		 * U2/U3 switch 8812AU will be recognized as another card and then
		 * OS will treat it as a new card and assign a new GUID. Then SWUSB
		 * service can not work well. We need to delay the card switch time to U3
		 * Then OS can unload the previous U2 port card and load new U3 port card later.
		 * The strange sympton can disappear.
		 */
		rtw_write8(Adapter, REG_CAL_TIMER+1, 0x40);
		/* rtw_write8(Adapter, REG_CAL_TIMER+1, 0x3); */
		rtw_write8(Adapter, REG_APS_FSMCO+1, 0x80);
		break;
	default:
		SetHwReg8812A(Adapter, variable, val);
		break;
	}
}

void GetHwReg8812AU(struct rtl_priv *Adapter, uint8_t variable, u8 *val)
{
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(Adapter);
	DM_ODM_T 		*podmpriv = &pHalData->odmpriv;

	switch (variable) {
	default:
		GetHwReg8812A(Adapter, variable, val);
		break;
	}

}

/*
 * 	Description:
 *		Change default setting of specified variable.
 */
u8 SetHalDefVar8812AUsb(struct rtl_priv *Adapter, HAL_DEF_VARIABLE eVariable,
	PVOID pValue)
{
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(Adapter);
	uint8_t			bResult = _SUCCESS;

	switch (eVariable) {
	default:
		SetHalDefVar8812A(Adapter, eVariable, pValue);
		break;
	}

	return bResult;
}

/*
 *	Description:
 *		Query setting of specified variable.
 */
u8 GetHalDefVar8812AUsb(struct rtl_priv *Adapter, HAL_DEF_VARIABLE eVariable,
	PVOID pValue)
{
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(Adapter);
	uint8_t			bResult = _SUCCESS;

	switch (eVariable) {
	default:
		GetHalDefVar8812A(Adapter, eVariable, pValue);
		break;
	}

	return bResult;
}

void _update_response_rate(struct rtl_priv *padapter, unsigned int mask)
{
	uint8_t	RateIndex = 0;
	/* Set RRSR rate table. */
	rtw_write8(padapter, REG_RRSR, mask&0xff);
	rtw_write8(padapter, REG_RRSR+1, (mask>>8)&0xff);

	/* Set RTS initial rate */
	while (mask > 0x1) {
		mask = (mask >> 1);
		RateIndex++;
	}
	rtw_write8(padapter, REG_INIRTS_RATE_SEL, RateIndex);
}

static void rtl8812au_init_default_value(struct rtl_priv *padapter)
{
	PHAL_DATA_TYPE pHalData;
	struct pwrctrl_priv *pwrctrlpriv;
	struct dm_priv *pdmpriv;
	uint8_t i;

	pHalData = GET_HAL_DATA(padapter);
	pwrctrlpriv = &padapter->pwrctrlpriv;
	pdmpriv = &pHalData->dmpriv;


	/* init default value */
	pHalData->fw_ractrl = _FALSE;
	if (!pwrctrlpriv->bkeepfwalive)
		pHalData->LastHMEBoxNum = 0;

	/* init dm default value */
	pHalData->bChnlBWInitialzed = _FALSE;
	pHalData->odmpriv.RFCalibrateInfo.bIQKInitialized = _FALSE;
	pHalData->odmpriv.RFCalibrateInfo.TM_Trigger = 0;/* for IQK */
	pHalData->pwrGroupCnt = 0;
	pHalData->PGMaxGroup = MAX_PG_GROUP;
	pHalData->odmpriv.RFCalibrateInfo.ThermalValue_HP_index = 0;
	for (i = 0; i < HP_THERMAL_NUM; i++)
		pHalData->odmpriv.RFCalibrateInfo.ThermalValue_HP[i] = 0;

	pHalData->IntrMask[0]	= (u32)(	\
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

	pHalData->IntrMask[1] 	= (u32)(	\
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

static uint8_t rtl8812au_ps_func(struct rtl_priv *Adapter, HAL_INTF_PS_FUNC efunc_id, uint8_t *val)
{
	uint8_t bResult = _TRUE;
	switch (efunc_id) {
	default:
		break;
	}
	return bResult;
}

/*
 *
 * ULLI
 * It's messy but we need here some prototypes
 */

void rtl8812_free_hal_data(struct rtl_priv *padapter);
void UpdateHalRAMask8812A(struct rtl_priv *padapter, uint32_t mac_id, uint8_t rssi_level);
void ReadChipVersion8812A(struct rtl_priv *Adapter);
VOID rtl8812_EfusePowerSwitch(struct rtl_priv *pAdapter, uint8_t bWrite, uint8_t PwrState);
VOID rtl8812_ReadEFuse(struct rtl_priv *Adapter, uint8_t efuseType, uint16_t _offset, uint16_t 	_size_byte, uint8_t *pbuf);
VOID rtl8812_EFUSE_GetEfuseDefinition(struct rtl_priv *pAdapter, uint8_t efuseType, uint8_t type, void *pOut);
u16 rtl8812_EfuseGetCurrentSize(struct rtl_priv *pAdapter, uint8_t efuseType);
int rtl8812_Efuse_PgPacketRead(struct rtl_priv *pAdapter, uint8_t offset, uint8_t *data);
int rtl8812_Efuse_PgPacketWrite(struct rtl_priv *pAdapter, uint8_t offset, uint8_t word_en, uint8_t *data);
u8 rtl8812_Efuse_WordEnableDataWrite(struct rtl_priv *pAdapter, uint16_t efuse_addr, uint8_t word_en, uint8_t *data);
void rtl8812_GetHalODMVar(struct rtl_priv *Adapter, HAL_ODM_VARIABLE eVariable, PVOID pValue1, BOOLEAN bSet);
void rtl8812_SetHalODMVar(struct rtl_priv *Adapter, HAL_ODM_VARIABLE eVariable,	PVOID 	pValue1, BOOLEAN bSet);
void hal_notch_filter_8812(struct rtl_priv *adapter, bool enable);


struct hal_ops rtl8812au_hal_ops = {
	/*
	 * New HAL functions with struct net_device  as first param
	 * this can be (hopefully)switched to struct ieee80211_hw
	 */

	.init_sw_leds = 		rtl8821au_init_sw_leds,
	.read_bbreg = 	rtl8821au_phy_query_bb_reg,
	.write_bbreg = 	rtl8821au_phy_set_bb_reg,

	/* Old HAL functions */

	.hal_init =	 	rtl8812au_hal_init,
	.hal_deinit = 		rtl8812au_hal_deinit,

	/* .free_hal_data = rtl8192c_free_hal_data, */

	.inirp_init =		rtl8812au_inirp_init,
	.inirp_deinit =		rtl8812au_inirp_deinit,

	.init_xmit_priv =	rtl8812au_init_xmit_priv,
	.free_xmit_priv =	rtl8812au_free_xmit_priv,

	.init_recv_priv =	rtl8812au_init_recv_priv,
	.free_recv_priv =	rtl8812au_free_recv_priv,
	.DeInitSwLeds =		rtl8812au_DeInitSwLeds,
	.init_default_value =	rtl8812au_init_default_value,
	.intf_chip_configure =	rtl8812au_interface_configure,
	.read_adapter_info =	ReadAdapterInfo8812AU,

	/* .set_bwmode_handler = 	PHY_SetBWMode8192C; */
	/* .set_channel_handler = 	PHY_SwChnl8192C; */

	/* .hal_dm_watchdog = 	rtl8192c_HalDmWatchDog; */


	.SetHwRegHandler = 	SetHwReg8812AU,
	.GetHwRegHandler = 	GetHwReg8812AU,
	.GetHalDefVarHandler = 	GetHalDefVar8812AUsb,
	.SetHalDefVarHandler = 	SetHalDefVar8812AUsb,

	.SetBeaconRelatedRegistersHandler = 	SetBeaconRelatedRegisters8812A,

	/* .Add_RateATid = &rtl8192c_Add_RateATid, */

	.hal_xmit = 		rtl8812au_hal_xmit,
	.mgnt_xmit = 		rtl8812au_mgnt_xmit,
	.hal_xmitframe_enqueue = 	rtl8812au_hal_xmitframe_enqueue,

	.interface_ps_func = 	rtl8812au_ps_func,

	.free_hal_data =	rtl8812_free_hal_data,

	.dm_init =		rtl8812_init_dm_priv,
	.dm_deinit =		rtl8812_deinit_dm_priv,

	.UpdateRAMaskHandler =	UpdateHalRAMask8812A,

	.read_chip_version =	ReadChipVersion8812A,

	.set_bwmode_handler =	PHY_SetBWMode8812,
	.set_channel_handler =	PHY_SwChnl8812,
	.set_chnl_bw_handler =	PHY_SetSwChnlBWMode8812,

	.hal_dm_watchdog =	rtl8812_HalDmWatchDog,

	.Add_RateATid =		rtl8812_Add_RateATid,
#ifdef CONFIG_ANTENNA_DIVERSITY
	.AntDivBeforeLinkHandler =	AntDivBeforeLink8812,
	.AntDivCompareHandler =		AntDivCompare8812,
#endif

	.read_rfreg = 	PHY_QueryRFReg8812,
	.write_rfreg = 	PHY_SetRFReg8812,


	/* Efuse related function */
	.EfusePowerSwitch =	rtl8812_EfusePowerSwitch,
	.ReadEFuse =		rtl8812_ReadEFuse,
	.EFUSEGetEfuseDefinition =	rtl8812_EFUSE_GetEfuseDefinition,
	.EfuseGetCurrentSize =	rtl8812_EfuseGetCurrentSize,
	.Efuse_PgPacketRead =	rtl8812_Efuse_PgPacketRead,
	.Efuse_PgPacketWrite =	rtl8812_Efuse_PgPacketWrite,
	.Efuse_WordEnableDataWrite = &rtl8812_Efuse_WordEnableDataWrite,

#ifdef DBG_CONFIG_ERROR_DETECT
	.sreset_init_value =	sreset_init_value,
	.sreset_reset_value =	sreset_reset_value,
	.silentreset =		sreset_reset,
	.sreset_xmit_status_check =	rtl8812_sreset_xmit_status_check,
	.sreset_linked_status_check =	rtl8812_sreset_linked_status_check,
	.sreset_get_wifi_status  =	sreset_get_wifi_status,
	.sreset_inprogress =		sreset_inprogress,
#endif

	.GetHalODMVarHandler = rtl8812_GetHalODMVar,
	.SetHalODMVarHandler = rtl8812_SetHalODMVar,
	.hal_notch_filter = hal_notch_filter_8812,

	.SetBeaconRelatedRegistersHandler =	SetBeaconRelatedRegisters8812A,
};

