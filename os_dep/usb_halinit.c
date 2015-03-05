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


static void _dbg_dump_macreg(struct rtl_priv *rtlpriv)
{
	uint32_t offset = 0;
	uint32_t val32 = 0;
	uint32_t index = 0;

	for (index = 0; index < 64; index++) {
		offset = index*4;
		val32 = usb_read32(rtlpriv, offset);
		DBG_8192C("offset : 0x%02x ,val:0x%08x\n", offset, val32);
	}
}

static VOID _ConfigChipOutEP_8812(struct rtl_priv *rtlpriv, uint8_t NumOutPipe)
{
	struct _rtw_hal *pHalData = GET_HAL_DATA(rtlpriv);

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

static BOOLEAN HalUsbSetQueuePipeMapping8812AUsb(struct rtl_priv *rtlpriv,
	uint8_t	NumInPipe, uint8_t NumOutPipe)
{
	 struct _rtw_hal	*pHalData	= GET_HAL_DATA(rtlpriv);
	BOOLEAN		result		= _FALSE;

	_ConfigChipOutEP_8812(rtlpriv, NumOutPipe);

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

	result = Hal_MappingOutPipe(rtlpriv, NumOutPipe);

	return result;

}

void rtl8812au_interface_configure(struct rtl_priv *rtlpriv)
{
	struct rtl_hal *rtlhal = rtl_hal(rtlpriv);
	struct _rtw_hal	*pHalData	= GET_HAL_DATA(rtlpriv);
	struct rtl_usb	*pdvobjpriv = rtl_usbdev(rtlpriv);

	if (IS_SUPER_SPEED_USB(rtlpriv))
		pHalData->UsbBulkOutSize = USB_SUPER_SPEED_BULK_SIZE;	/* 1024 bytes */
	else if (IS_HIGH_SPEED_USB(rtlpriv))
		pHalData->UsbBulkOutSize = USB_HIGH_SPEED_BULK_SIZE;	/* 512 bytes */
	else
		pHalData->UsbBulkOutSize = USB_FULL_SPEED_BULK_SIZE; 	/*64 bytes */

	pHalData->interfaceIndex = pdvobjpriv->InterfaceNumber;

#ifdef CONFIG_USB_TX_AGGREGATION
	pHalData->UsbTxAggMode		= 1;
	pHalData->UsbTxAggDescNum	= 6;	/* only 4 bits */

	if (IS_HARDWARE_TYPE_8812AU(rtlhal))	/* page added for Jaguar */
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

	HalUsbSetQueuePipeMapping8812AUsb(rtlpriv,
				pdvobjpriv->RtNumInPipes, pdvobjpriv->RtNumOutPipes);

}

static VOID _InitBurstPktLen(IN struct rtl_priv *Adapter)
{
	struct rtl_hal *rtlhal = rtl_hal(Adapter);
	u8 speedvalue, provalue, temp;
	struct _rtw_hal	*pHalData = GET_HAL_DATA(Adapter);

	/*
	 * usb_write16(Adapter, REG_TRXDMA_CTRL_8195, 0xf5b0);
	 * usb_write16(Adapter, REG_TRXDMA_CTRL_8812, 0xf5b4);
	 */
	usb_write8(Adapter, 0xf050, 0x01);		/* usb3 rx interval */
	usb_write16(Adapter, REG_RXDMA_STATUS, 0x7400);	/* burset lenght=4, set 0x3400 for burset length=2 */
	usb_write8(Adapter, 0x289, 0xf5);		/* for rxdma control */
	/* usb_write8(Adapter, 0x3a, 0x46); */

	/*  0x456 = 0x70, sugguested by Zhilin */
	usb_write8(Adapter, REG_AMPDU_MAX_TIME_8812, 0x70);

	usb_write32(Adapter, 0x458, 0xffffffff);
	usb_write8(Adapter, REG_USTIME_TSF, 0x50);
	usb_write8(Adapter, REG_USTIME_EDCA, 0x50);

	if (IS_HARDWARE_TYPE_8821U(rtlhal))
		speedvalue = BIT7;
	else
		speedvalue = usb_read8(Adapter, 0xff); /* check device operation speed: SS 0xff bit7 */

	if (speedvalue & BIT7) {		/* USB2/1.1 Mode */
		temp = usb_read8(Adapter, 0xfe17);
		if (((temp >> 4) & 0x03) == 0) {
			pHalData->UsbBulkOutSize = USB_HIGH_SPEED_BULK_SIZE;
			provalue = usb_read8(Adapter, REG_RXDMA_PRO_8812);
			usb_write8(Adapter, REG_RXDMA_PRO_8812, ((provalue|BIT(4))&(~BIT(5)))); /* set burst pkt len=512B */
			usb_write16(Adapter, REG_RXDMA_PRO_8812, 0x1e);
		} else {
			pHalData->UsbBulkOutSize = 64;
			provalue = usb_read8(Adapter, REG_RXDMA_PRO_8812);
			usb_write8(Adapter, REG_RXDMA_PRO_8812, ((provalue|BIT(5))&(~BIT(4)))); /* set burst pkt len=64B */
		}

		usb_write16(Adapter, REG_RXDMA_AGG_PG_TH, 0x2005); /* dmc agg th 20K */

		/*
		 * usb_write8(Adapter, 0x10c, 0xb4);
		 * hal_UphyUpdate8812AU(Adapter);
		 */

		pHalData->bSupportUSB3 = _FALSE;
	} else {		/* USB3 Mode */
		pHalData->UsbBulkOutSize = USB_SUPER_SPEED_BULK_SIZE;
		provalue = usb_read8(Adapter, REG_RXDMA_PRO_8812);
		usb_write8(Adapter, REG_RXDMA_PRO_8812, provalue&(~(BIT5|BIT4))); /* set burst pkt len=1k */
		usb_write16(Adapter, REG_RXDMA_PRO_8812, 0x0e);
		pHalData->bSupportUSB3 = _TRUE;

		/*  set Reg 0xf008[3:4] to 2'00 to disable U1/U2 Mode to avoid 2.5G spur in USB3.0. added by page, 20120712 */
		usb_write8(Adapter, 0xf008, usb_read8(Adapter, 0xf008)&0xE7);
	}

#ifdef CONFIG_USB_TX_AGGREGATION
	/* usb_write8(Adapter, REG_TDECTRL_8195, 0x30); */
#else
	usb_write8(Adapter, REG_TDECTRL, 0x10);
#endif

	temp = usb_read8(Adapter, REG_SYS_FUNC_EN);
	usb_write8(Adapter, REG_SYS_FUNC_EN, temp&(~BIT(10))); 	/* reset 8051 */

	usb_write8(Adapter, REG_HT_SINGLE_AMPDU_8812, usb_read8(Adapter, REG_HT_SINGLE_AMPDU_8812)|BIT(7));	/* enable single pkt ampdu */
	usb_write8(Adapter, REG_RX_PKT_LIMIT, 0x18);		/* for VHT packet length 11K */

	usb_write8(Adapter, REG_PIFS, 0x00);

	/* Suggention by SD1 Jong and Pisa, by Maddest 20130107. */
	if (IS_HARDWARE_TYPE_8821U(rtlhal) && (Adapter->registrypriv.wifi_spec == _FALSE)) {
		usb_write16(Adapter, REG_MAX_AGGR_NUM, 0x0a0a);
		usb_write8(Adapter, REG_FWHW_TXQ_CTRL, 0x80);
		usb_write8(Adapter, REG_AMPDU_MAX_TIME_8812, 0x5e);
		usb_write32(Adapter, REG_FAST_EDCA_CTRL, 0x03087777);
	} else {
		usb_write8(Adapter, REG_MAX_AGGR_NUM, 0x1f);
		usb_write8(Adapter, REG_FWHW_TXQ_CTRL, usb_read8(Adapter, REG_FWHW_TXQ_CTRL)&(~BIT(7)));
	}

	if (pHalData->AMPDUBurstMode)
		usb_write8(Adapter, REG_AMPDU_BURST_MODE_8812, 0x5F);

	usb_write8(Adapter, 0x1c, usb_read8(Adapter, 0x1c) | BIT(5) | BIT(6));  /* to prevent mac is reseted by bus. 20111208, by Page */

	/* ARFB table 9 for 11ac 5G 2SS */
	usb_write32(Adapter, REG_ARFR0, 0x00000010);
	if (IS_NORMAL_CHIP(pHalData->VersionID))
		usb_write32(Adapter, REG_ARFR0+4, 0xfffff000);
	else
		usb_write32(Adapter, REG_ARFR0+4, 0x3e0ff000);

	/* ARFB table 10 for 11ac 5G 1SS */
	usb_write32(Adapter, REG_ARFR1, 0x00000010);
	if (IS_VENDOR_8812A_TEST_CHIP(Adapter))
		usb_write32(Adapter, REG_ARFR1_8812+4, 0x000ff000);
	else
		usb_write32(Adapter, REG_ARFR1_8812+4, 0x003ff000);

}

static uint32_t _InitPowerOn8812AU(struct rtl_priv *rtlpriv)
{
	u16	u2btmp = 0;
	uint8_t	u1btmp = 0;
	struct rtl_hal *rtlhal = rtl_hal(rtlpriv);

	if (IS_VENDOR_8821A_MP_CHIP(rtlpriv)) {
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
	usb_write16(rtlpriv, REG_CR, 0x00); 	/* suggseted by zhouzhou, by page, 20111230 */
	u2btmp = usb_read16(rtlpriv, REG_CR);
	u2btmp |= (HCI_TXDMA_EN | HCI_RXDMA_EN | TXDMA_EN | RXDMA_EN
				| PROTOCOL_EN | SCHEDULE_EN | ENSEC | CALTMR_EN);
	usb_write16(rtlpriv, REG_CR, u2btmp);

	/*
	 * Need remove below furture, suggest by Jackie.
	 * if 0xF0[24] =1 (LDO), need to set the 0x7C[6] to 1.
	 */
	if (IS_HARDWARE_TYPE_8821U(rtlhal)) {
		u1btmp = usb_read8(rtlpriv, REG_SYS_CFG+3);
		if (u1btmp & BIT0) { 	/* LDO mode. */
			u1btmp = usb_read8(rtlpriv, 0x7c);
			/* ULLI unknown register */
			usb_write8(rtlpriv, 0x7c, u1btmp | BIT6);
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
	struct _rtw_hal *pHalData = GET_HAL_DATA(Adapter);

	/* HIMR */
	usb_write32(Adapter, REG_HIMR0_8812, pHalData->IntrMask[0]&0xFFFFFFFF);
	usb_write32(Adapter, REG_HIMR1_8812, pHalData->IntrMask[1]&0xFFFFFFFF);
}

static VOID _InitQueueReservedPage_8821AUsb(struct rtl_priv *Adapter)
{
	struct _rtw_hal *pHalData = GET_HAL_DATA(Adapter);
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

		if (pHalData->OutEpQueueSel & TX_SELE_HQ)
			numHQ = NORMAL_PAGE_NUM_HPQ_8821;

		if (pHalData->OutEpQueueSel & TX_SELE_LQ)
			numLQ = NORMAL_PAGE_NUM_LPQ_8821;

		/* NOTE: This step shall be proceed before writting REG_RQPN. */
		if (pHalData->OutEpQueueSel & TX_SELE_NQ)
			numNQ = NORMAL_PAGE_NUM_NPQ_8821;

	} else { /* WMM */
		numPubQ = WMM_NORMAL_PAGE_NUM_PUBQ_8821;

		if (pHalData->OutEpQueueSel & TX_SELE_HQ)
			numHQ = WMM_NORMAL_PAGE_NUM_HPQ_8821;

		if (pHalData->OutEpQueueSel & TX_SELE_LQ)
			numLQ = WMM_NORMAL_PAGE_NUM_LPQ_8821;

		/* NOTE: This step shall be proceed before writting REG_RQPN. */
		if (pHalData->OutEpQueueSel & TX_SELE_NQ)
			numNQ = WMM_NORMAL_PAGE_NUM_NPQ_8821;
	}

	value8 = (u8)_NPQ(numNQ);
	usb_write8(Adapter, REG_RQPN_NPQ, value8);

	/* TX DMA */
	value32 = _HPQ(numHQ) | _LPQ(numLQ) | _PUBQ(numPubQ) | LD_RQPN;
	usb_write32(Adapter, REG_RQPN, value32);
}

static VOID _InitQueueReservedPage_8812AUsb(struct rtl_priv *Adapter)
{
	struct _rtw_hal *pHalData = GET_HAL_DATA(Adapter);
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

		if (pHalData->OutEpQueueSel & TX_SELE_HQ)
			numHQ = NORMAL_PAGE_NUM_HPQ_8812;

		if (pHalData->OutEpQueueSel & TX_SELE_LQ)
			numLQ = NORMAL_PAGE_NUM_LPQ_8812;

		/* NOTE: This step shall be proceed before writting REG_RQPN. */
		if (pHalData->OutEpQueueSel & TX_SELE_NQ)
			numNQ = NORMAL_PAGE_NUM_NPQ_8812;
	} else { /* WMM */
		numPubQ = WMM_NORMAL_PAGE_NUM_PUBQ_8812;

		if (pHalData->OutEpQueueSel & TX_SELE_HQ)
			numHQ = WMM_NORMAL_PAGE_NUM_HPQ_8812;

		if (pHalData->OutEpQueueSel & TX_SELE_LQ)
			numLQ = WMM_NORMAL_PAGE_NUM_LPQ_8812;

		/* NOTE: This step shall be proceed before writting REG_RQPN. */
		if (pHalData->OutEpQueueSel & TX_SELE_NQ)
			numNQ = WMM_NORMAL_PAGE_NUM_NPQ_8812;
	}

	value8 = (u8)_NPQ(numNQ);
	usb_write8(Adapter, REG_RQPN_NPQ, value8);

	/* TX DMA */
	value32 = _HPQ(numHQ) | _LPQ(numLQ) | _PUBQ(numPubQ) | LD_RQPN;
	usb_write32(Adapter, REG_RQPN, value32);
}

static void _InitID_8812A(IN  struct rtl_priv *Adapter)
{
	hal_init_macaddr(Adapter);	/* set mac_address */
}

static VOID _InitTxBufferBoundary_8821AUsb(struct rtl_priv *Adapter)
{
	struct registry_priv *pregistrypriv = &Adapter->registrypriv;
	uint8_t	txpktbuf_bndy;

	if (!pregistrypriv->wifi_spec)
		txpktbuf_bndy = TX_PAGE_BOUNDARY_8821;
	else	/* for WMM */
		txpktbuf_bndy = WMM_NORMAL_TX_PAGE_BOUNDARY_8821;

	usb_write8(Adapter, REG_BCNQ_BDNY, txpktbuf_bndy);
	usb_write8(Adapter, REG_MGQ_BDNY, txpktbuf_bndy);
	usb_write8(Adapter, REG_WMAC_LBK_BF_HD, txpktbuf_bndy);
	usb_write8(Adapter, REG_TRXFF_BNDY, txpktbuf_bndy);
	usb_write8(Adapter, REG_TDECTRL+1, txpktbuf_bndy);
}

static VOID _InitTxBufferBoundary_8812AUsb(struct rtl_priv *Adapter)
{
	struct registry_priv *pregistrypriv = &Adapter->registrypriv;
	uint8_t	txpktbuf_bndy;

	if (!pregistrypriv->wifi_spec)
		txpktbuf_bndy = TX_PAGE_BOUNDARY_8812;
	else	/* for WMM */
		txpktbuf_bndy = WMM_NORMAL_TX_PAGE_BOUNDARY_8812;

	usb_write8(Adapter, REG_BCNQ_BDNY, txpktbuf_bndy);
	usb_write8(Adapter, REG_MGQ_BDNY, txpktbuf_bndy);
	usb_write8(Adapter, REG_WMAC_LBK_BF_HD, txpktbuf_bndy);
	usb_write8(Adapter, REG_TRXFF_BNDY, txpktbuf_bndy);
	usb_write8(Adapter, REG_TDECTRL+1, txpktbuf_bndy);
}

static VOID _InitPageBoundary_8812AUsb(struct rtl_priv *Adapter)
{
	struct rtl_hal *rtlhal = rtl_hal(Adapter);
	/*
	 * u16 			rxff_bndy;
	 * u16			Offset;
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

	if (IS_HARDWARE_TYPE_8812(rtlhal))
		usb_write16(Adapter, (REG_TRXFF_BNDY + 2), MAX_RX_DMA_BUFFER_SIZE_8812-1);
	else
		usb_write16(Adapter, (REG_TRXFF_BNDY + 2), MAX_RX_DMA_BUFFER_SIZE_8821-1);

}


static VOID _InitNormalChipRegPriority_8812AUsb(struct rtl_priv *Adapter,
	u16 beQ, u16 bkQ, u16 viQ,
	u16 voQ, u16 mgtQ, u16 hiQ)
{
	u16 value16 = (usb_read16(Adapter, REG_TRXDMA_CTRL) & 0x7);

	value16 |= _TXDMA_BEQ_MAP(beQ) 	| _TXDMA_BKQ_MAP(bkQ) |
		   _TXDMA_VIQ_MAP(viQ) 	| _TXDMA_VOQ_MAP(voQ) |
		   _TXDMA_MGQ_MAP(mgtQ) | _TXDMA_HIQ_MAP(hiQ);

	usb_write16(Adapter, REG_TRXDMA_CTRL, value16);
}

static VOID _InitNormalChipTwoOutEpPriority_8812AUsb(struct rtl_priv *Adapter)
{
	 struct _rtw_hal	*pHalData	= GET_HAL_DATA(Adapter);
	struct registry_priv *pregistrypriv = &Adapter->registrypriv;
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

	if (!pregistrypriv->wifi_spec) {
		beQ	= valueLow;
		bkQ	= valueLow;
		viQ	= valueHi;
		voQ	= valueHi;
		mgtQ	= valueHi;
		hiQ	= valueHi;
	} else {	/* for WMM ,CONFIG_OUT_EP_WIFI_MODE */
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
	u16	beQ, bkQ, viQ, voQ, mgtQ, hiQ;

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
	 struct _rtw_hal	*pHalData	= GET_HAL_DATA(Adapter);

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
	uint32_t value32 = usb_read32(Adapter, REG_TXDMA_OFFSET_CHK);
	value32 |= DROP_DATA_EN;
	usb_write32(Adapter, REG_TXDMA_OFFSET_CHK, value32);
}

static VOID _InitNetworkType_8812A(struct rtl_priv *Adapter)
{
	uint32_t	value32;

	value32 = usb_read32(Adapter, REG_CR);
	/*  TODO: use the other function to set network type */
	value32 = (value32 & ~MASK_NETTYPE) | _NETTYPE(NT_LINK_AP);

	usb_write32(Adapter, REG_CR, value32);
}

static VOID _InitTransferPageSize_8812AUsb(struct rtl_priv *Adapter)
{
	uint8_t	value8;
	value8 = _PSTX(PBP_512);

	usb_write8(Adapter, REG_PBP, value8);
}

static VOID _InitDriverInfoSize_8812A(struct rtl_priv *Adapter, uint8_t	drvInfoSize)
{
	usb_write8(Adapter, REG_RX_DRVINFO_SZ, drvInfoSize);
}

static VOID _InitWMACSetting_8812A(struct rtl_priv *Adapter)
{
	struct rtl_hal *rtlhal = rtl_hal(Adapter);
	/* uint32_t			value32; */
	/* u16			value16; */
	 struct _rtw_hal	*pHalData = GET_HAL_DATA(Adapter);

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

	if (IS_HARDWARE_TYPE_8812AU(rtlhal) || IS_HARDWARE_TYPE_8821U(rtlhal))
		pHalData->ReceiveConfig |= FORCEACK;

	/*
	 *  some REG_RCR will be modified later by phy_ConfigMACWithHeaderFile()
	 */
	usb_write32(Adapter, REG_RCR, pHalData->ReceiveConfig);

	/* Accept all multicast address */
	usb_write32(Adapter, REG_MAR, 0xFFFFFFFF);
	usb_write32(Adapter, REG_MAR + 4, 0xFFFFFFFF);


	/*
	 *  Accept all data frames
	 * value16 = 0xFFFF;
	 * usb_write16(Adapter, REG_RXFLTMAP2, value16);
	 */

	/*
	 * 2010.09.08 hpfan
	 * Since ADF is removed from RCR, ps-poll will not be indicate to driver,
	 * RxFilterMap should mask ps-poll to gurantee AP mode can rx ps-poll.
	 * value16 = 0x400;
	 * usb_write16(Adapter, REG_RXFLTMAP1, value16);
	 */

	/*
	 *  Accept all management frames
	 * value16 = 0xFFFF;
	 * usb_write16(Adapter, REG_RXFLTMAP0, value16);
	 */

	/*
	 * enable RX_SHIFT bits
	 * usb_write8(Adapter, REG_TRXDMA_CTRL, usb_read8(Adapter, REG_TRXDMA_CTRL)|BIT(1));
	 */

}

static VOID _InitAdaptiveCtrl_8812AUsb(IN struct rtl_priv *Adapter)
{
	u16	value16;
	uint32_t	value32;

	/* Response Rate Set */
	value32 = usb_read32(Adapter, REG_RRSR);
	value32 &= ~RATE_BITMAP_ALL;

	if (Adapter->registrypriv.wireless_mode & WIRELESS_11B)
		value32 |= RATE_RRSR_CCK_ONLY_1M;
	else
		value32 |= RATE_RRSR_WITHOUT_CCK;

	value32 |= RATE_RRSR_CCK_ONLY_1M;
	usb_write32(Adapter, REG_RRSR, value32);

	/*
	 * CF-END Threshold
	 * m_spIoBase->usb_write8(REG_CFEND_TH, 0x1);
	 */

	/* SIFS (used in NAV) */
	value16 = _SPEC_SIFS_CCK(0x10) | _SPEC_SIFS_OFDM(0x10);
	usb_write16(Adapter, REG_SPEC_SIFS, value16);

	/* Retry Limit */
	value16 = _LRL(0x30) | _SRL(0x30);
	usb_write16(Adapter, REG_RL, value16);

}

static VOID _InitEDCA_8812AUsb(struct rtl_priv *Adapter)
{
	/* Set Spec SIFS (used in NAV) */
	usb_write16(Adapter, REG_SPEC_SIFS, 0x100a);
	usb_write16(Adapter, REG_MAC_SPEC_SIFS, 0x100a);

	/* Set SIFS for CCK */
	usb_write16(Adapter, REG_SIFS_CTX, 0x100a);

	/* Set SIFS for OFDM */
	usb_write16(Adapter, REG_SIFS_TRX, 0x100a);

	/* TXOP */
	usb_write32(Adapter, REG_EDCA_BE_PARAM, 0x005EA42B);
	usb_write32(Adapter, REG_EDCA_BK_PARAM, 0x0000A44F);
	usb_write32(Adapter, REG_EDCA_VI_PARAM, 0x005EA324);
	usb_write32(Adapter, REG_EDCA_VO_PARAM, 0x002FA226);

	/* 0x50 for 80MHz clock */
	usb_write8(Adapter, REG_USTIME_TSF, 0x50);
	usb_write8(Adapter, REG_USTIME_EDCA, 0x50);
}


static VOID _InitBeaconMaxError_8812A(struct rtl_priv *Adapter, BOOLEAN	InfraMode)
{
	/* ULLI: looks here is some hacking done, wrong nams ?? */
#ifdef RTL8192CU_ADHOC_WORKAROUND_SETTING
	usb_write8(Adapter, REG_BCN_MAX_ERR, 0xFF);
#else
	/* usb_write8(Adapter, REG_BCN_MAX_ERR, (InfraMode ? 0xFF : 0x10)); */
#endif
}

static VOID _InitRDGSetting_8812A(struct rtl_priv *Adapter)
{
	usb_write8(Adapter, REG_RD_CTRL, 0xFF);
	usb_write16(Adapter, REG_RD_NAV_NXT, 0x200);
	usb_write8(Adapter, REG_RD_RESP_PKT_TH, 0x05);
}

static VOID _InitRxSetting_8812AU(struct rtl_priv *Adapter)
{
	usb_write32(Adapter, REG_MACID, 0x87654321);
	/* ULLI unknown register */
	usb_write32(Adapter, 0x0700, 0x87654321);
}

static VOID _InitRetryFunction_8812A(IN  struct rtl_priv *Adapter)
{
	uint8_t	value8;

	value8 = usb_read8(Adapter, REG_FWHW_TXQ_CTRL);
	value8 |= EN_AMPDU_RTY_NEW;
	usb_write8(Adapter, REG_FWHW_TXQ_CTRL, value8);

	/*
	 * Set ACK timeout
	 * usb_write8(Adapter, REG_ACKTO, 0x40);  //masked by page for BCM IOT issue temporally
	 */
	usb_write8(Adapter, REG_ACKTO, 0x80);
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
	 struct _rtw_hal	*pHalData = GET_HAL_DATA(Adapter);
	uint32_t			value32;

	if (Adapter->registrypriv.wifi_spec)
		pHalData->UsbTxAggMode = _FALSE;

	if (pHalData->UsbTxAggMode) {
		value32 = usb_read32(Adapter, REG_TDECTRL);
		value32 = value32 & ~(BLK_DESC_NUM_MASK << BLK_DESC_NUM_SHIFT);
		value32 |= ((pHalData->UsbTxAggDescNum & BLK_DESC_NUM_MASK) << BLK_DESC_NUM_SHIFT);

		usb_write32(Adapter, REG_TDECTRL, value32);
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
	 struct _rtw_hal	*pHalData = GET_HAL_DATA(Adapter);
	uint8_t			valueDMA;
	uint8_t			valueUSB;

	valueDMA = usb_read8(Adapter, REG_TRXDMA_CTRL);

	switch (pHalData->UsbRxAggMode) {
	case USB_RX_AGG_DMA:
		valueDMA |= RXDMA_AGG_EN;

		/* usb_write8(Adapter, REG_RXDMA_AGG_PG_TH, 0x05); //dma agg mode, 20k
		 *
		 * 2012/10/26 MH For TX throught start rate temp fix.
		 */
		{
			u16			temp;

			/* ULLI DMA on USB Device WTF ??? */
			/* Adjust DMA page and thresh. */
			temp = pHalData->RegAcUsbDmaSize | (pHalData->RegAcUsbDmaTime<<8);
			usb_write16(Adapter, REG_RXDMA_AGG_PG_TH, temp);
		}
		break;
	case USB_RX_AGG_USB:
	case USB_RX_AGG_MIX:
	case USB_RX_AGG_DISABLE:
	default:
		/* TODO: */
		break;
	}

	usb_write8(Adapter, REG_TRXDMA_CTRL, valueDMA);
#endif
}

static VOID init_UsbAggregationSetting_8812A(struct rtl_priv *Adapter)
{
	 struct _rtw_hal	*pHalData = GET_HAL_DATA(Adapter);

	/* Tx aggregation setting */
	usb_AggSettingTxUpdate_8812A(Adapter);

	/* Rx aggregation setting */
	usb_AggSettingRxUpdate_8812A(Adapter);

	/* 201/12/10 MH Add for USB agg mode dynamic switch. */
	pHalData->UsbRxHighSpeedMode = _FALSE;
}

/* Set CCK and OFDM Block "ON" */
static VOID _BBTurnOnBlock(struct rtl_priv *Adapter)
{
	rtl_set_bbreg(Adapter, RFPGA0_RFMOD, bCCKEn, 0x1);
	rtl_set_bbreg(Adapter, RFPGA0_RFMOD, bOFDMEn, 0x1);
}

enum {
	Antenna_Lfet = 1,
	Antenna_Right = 2,
};

static VOID _InitAntenna_Selection_8812A(struct rtl_priv *Adapter)
{
	 struct _rtw_hal	*pHalData	= GET_HAL_DATA(Adapter);

	if (pHalData->AntDivCfg == 0)
		return;
	DBG_8192C("==>  %s ....\n", __FUNCTION__);

	usb_write8(Adapter, REG_LEDCFG2, 0x82);

	rtl_set_bbreg(Adapter, RFPGA0_XAB_RFPARAMETER, BIT13, 0x01);

	if (rtl_get_bbreg(Adapter, rFPGA0_XA_RFInterfaceOE, 0x300) == MAIN_ANT)
		pHalData->CurAntenna = MAIN_ANT;
	else
		pHalData->CurAntenna = AUX_ANT;
	DBG_8192C("%s,Cur_ant:(%x)%s\n", __FUNCTION__, pHalData->CurAntenna, (pHalData->CurAntenna == MAIN_ANT) ? "MAIN_ANT" : "AUX_ANT");


}

rt_rf_power_state RfOnOffDetect(struct rtl_priv *rtlpriv)
{
	struct _rtw_hal		*pHalData = GET_HAL_DATA(rtlpriv);
	uint8_t	val8;
	rt_rf_power_state rfpowerstate = rf_off;

	if (rtlpriv->pwrctrlpriv.bHWPowerdown) {
		val8 = usb_read8(rtlpriv, REG_HSISR);
		DBG_8192C("pwrdown, 0x5c(BIT7)=%02x\n", val8);
		rfpowerstate = (val8 & BIT7) ? rf_off : rf_on;
	} else { /* rf on/off */
		usb_write8(rtlpriv, REG_MAC_PINMUX_CFG, usb_read8(rtlpriv, REG_MAC_PINMUX_CFG)&~(BIT3));
		val8 = usb_read8(rtlpriv, REG_GPIO_IO_SEL);
		DBG_8192C("GPIO_IN=%02x\n", val8);
		rfpowerstate = (val8 & BIT3) ? rf_on : rf_off;
	}
	return rfpowerstate;
}

uint32_t rtl8812au_hal_init(struct rtl_priv *Adapter)
{
	struct rtl_hal *rtlhal = rtl_hal(Adapter);
	uint8_t	value8 = 0, u1bRegCR;
	u16  value16;
	uint8_t	txpktbuf_bndy;
	uint32_t	status = _SUCCESS;
	 struct _rtw_hal *pHalData = GET_HAL_DATA(Adapter);
	struct pwrctrl_priv *pwrctrlpriv = &Adapter->pwrctrlpriv;
	struct registry_priv *pregistrypriv = &Adapter->registrypriv;
	rt_rf_power_state eRfPowerStateToSet;
	uint32_t init_start_time = jiffies;

	DBG_871X(" ULLI: Call rtl8812au_hal_init in usb_halinit.c\n");

	if (Adapter->pwrctrlpriv.bkeepfwalive) {
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
	value8 = usb_read8(Adapter, REG_SYS_CLKR+1);
	u1bRegCR = usb_read8(Adapter, REG_CR);
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
		usb_write8(Adapter, REG_RF_CTRL, 5);
		usb_write8(Adapter, REG_RF_CTRL, 7);
		usb_write8(Adapter, REG_RF_B_CTRL_8812, 5);
		usb_write8(Adapter, REG_RF_B_CTRL_8812, 7);
	}

	status = _InitPowerOn8812AU(Adapter);
	if (status == _FAIL) {
		goto exit;
	}

	if (!pregistrypriv->wifi_spec) {
		if (IS_HARDWARE_TYPE_8812(rtlhal))
			txpktbuf_bndy = TX_PAGE_BOUNDARY_8812;
		else
			txpktbuf_bndy = TX_PAGE_BOUNDARY_8821;
	} else {
		/* for WMM */
		if (IS_HARDWARE_TYPE_8812(rtlhal))
			txpktbuf_bndy = WMM_NORMAL_TX_PAGE_BOUNDARY_8812;
		else
			txpktbuf_bndy = WMM_NORMAL_TX_PAGE_BOUNDARY_8821;
	}

	status =  InitLLTTable8812(Adapter, txpktbuf_bndy);
	if (status == _FAIL) {
		goto exit;
	}

#if ENABLE_USB_DROP_INCORRECT_OUT
	_InitHardwareDropIncorrectBulkOut_8812A(Adapter);
#endif

	if (pHalData->bRDGEnable)
		_InitRDGSetting_8812A(Adapter);

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

	InitializeFirmwareVars8812(Adapter);

	if (pwrctrlpriv->reg_rfoff == _TRUE)
		pwrctrlpriv->rf_pwrstate = rf_off;

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
	Adapter->phy.current_channel = 0;	/* set 0 to trigger switch correct channel */

	PHY_MACConfig8812(Adapter);

	if (IS_HARDWARE_TYPE_8812(rtlhal)) {
		_InitQueueReservedPage_8812AUsb(Adapter);
		_InitTxBufferBoundary_8812AUsb(Adapter);
	} else if (IS_HARDWARE_TYPE_8821(rtlhal)) {
		_InitQueueReservedPage_8821AUsb(Adapter);
		_InitTxBufferBoundary_8821AUsb(Adapter);
	}

	_InitQueuePriority_8812AUsb(Adapter);
	_InitPageBoundary_8812AUsb(Adapter);

	if (IS_HARDWARE_TYPE_8812(rtlhal))
		_InitTransferPageSize_8812AUsb(Adapter);

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
	_InitBeaconParameters_8812A(Adapter);
	_InitBeaconMaxError_8812A(Adapter, _TRUE);

	_InitBurstPktLen(Adapter);  /* added by page. 20110919 */

	/*
	 * Init CR MACTXEN, MACRXEN after setting RxFF boundary REG_TRXFF_BNDY to patch
	 * Hw bug which Hw initials RxFF boundry size to a value which is larger than the real Rx buffer size in 88E.
	 * 2011.08.05. by tynli.
	 */
	value8 = usb_read8(Adapter, REG_CR);
	usb_write8(Adapter, REG_CR, (value8|MACTXEN|MACRXEN));

#if defined(CONFIG_TX_MCAST2UNI)

#ifdef CONFIG_TX_MCAST2UNI
	usb_write16(Adapter, REG_PKT_VO_VI_LIFE_TIME, 0x0400);	/* unit: 256us. 256ms */
	usb_write16(Adapter, REG_PKT_BE_BK_LIFE_TIME, 0x0400);	/* unit: 256us. 256ms */
#else
	usb_write16(Adapter, REG_PKT_VO_VI_LIFE_TIME, 0x3000);	/* unit: 256us. 3s */
	usb_write16(Adapter, REG_PKT_BE_BK_LIFE_TIME, 0x3000);	/* unit: 256us. 3s */
#endif
#endif

	/*
	 * d. Initialize BB related configurations.
	 */


	status = PHY_BBConfig8812(Adapter);
	if (status == _FAIL)
		goto exit;

	/*
	 * 92CU use 3-wire to r/w RF
	 * pHalData->Rf_Mode = RF_OP_By_SW_3wire;
	 */

	status = PHY_RFConfig8812(Adapter);
	if (status == _FAIL)
		goto exit;

	if (Adapter->phy.rf_type == RF_1T1R && IS_HARDWARE_TYPE_8812AU(rtlhal))
		PHY_BB8812_Config_1T(Adapter);

	if (Adapter->registrypriv.channel <= 14)
		rtl8821au_phy_switch_wirelessband(Adapter, BAND_ON_2_4G);
	else
		rtl8821au_phy_switch_wirelessband(Adapter, BAND_ON_5G);

	rtw_hal_set_chnl_bw(Adapter, Adapter->registrypriv.channel,
		CHANNEL_WIDTH_20, HAL_PRIME_CHNL_OFFSET_DONT_CARE, HAL_PRIME_CHNL_OFFSET_DONT_CARE);

	invalidate_cam_all(Adapter);

	_InitAntenna_Selection_8812A(Adapter);

	/*
	 * HW SEQ CTRL
	 * set 0x0 to 0xFF by tynli. Default enable HW SEQ NUM.
	 */
	usb_write8(Adapter, REG_HWSEQ_CTRL, 0xFF);

	/*
	 * Disable BAR, suggested by Scott
	 * 2010.04.09 add by hpfan
	 */
	usb_write32(Adapter, REG_BAR_MODE_CTRL, 0x0201ffff);

	if (pregistrypriv->wifi_spec)
		usb_write16(Adapter, REG_FAST_EDCA_CTRL, 0);

	/* Nav limit , suggest by scott */
	usb_write8(Adapter, 0x652, 0x0);


	rtl8812_InitHalDm(Adapter);

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
	usb_write8(Adapter, REG_QUEUE_CTRL, usb_read8(Adapter, REG_QUEUE_CTRL)&0xF7);

	/* enable Tx report. */
	usb_write8(Adapter,  REG_FWHW_TXQ_CTRL+1, 0x0F);

	/* Suggested by SD1 pisa. Added by tynli. 2011.10.21. */
	usb_write8(Adapter, REG_EARLY_MODE_CONTROL_8812+3, 0x01);/* Pretx_en, for WEP/TKIP SEC */

	/* tynli_test_tx_report. */
	usb_write16(Adapter, REG_TX_RPT_TIME, 0x3DF0);

	/* Reset USB mode switch setting */
	usb_write8(Adapter, REG_SDIO_CTRL_8812, 0x0);
	usb_write8(Adapter, REG_ACLK_MON, 0x0);

	/*
	 * RT_TRACE(COMP_INIT, DBG_TRACE, ("InitializeAdapter8188EUsb() <====\n"));
	 */

	/* 2010/08/26 MH Merge from 8192CE. */
	if (pwrctrlpriv->rf_pwrstate == rf_on) {
		if (IS_HARDWARE_TYPE_8812AU(rtlhal)) {
			pHalData->odmpriv.RFCalibrateInfo.bNeedIQK = _TRUE;
			if (pHalData->odmpriv.RFCalibrateInfo.bIQKInitialized)
				rtl8812au_phy_iq_calibrate(Adapter, _TRUE);
			else {
				rtl8812au_phy_iq_calibrate(Adapter, _FALSE);
				pHalData->odmpriv.RFCalibrateInfo.bIQKInitialized = _TRUE;
			}
		}


		/* ODM_TXPowerTrackingCheck(&pHalData->odmpriv ); */


		/* PHY_LCCalibrate_8812A(Adapter); */
	}

	/* HAL_INIT_PROFILE_TAG(HAL_INIT_STAGES_INIT_PABIAS);
	 * _InitPABias(Adapter);
	 */

	/*
	 *  2010/08/23 MH According to Alfred's suggestion, we need to to prevent HW enter
	 *  suspend mode automatically.
	 * HwSuspendModeEnable92Cu(Adapter, _FALSE);
	 */


	usb_write8(Adapter, REG_USB_HRPWM, 0);

	/* misc */
	{
		/* ULLI reading MAC address again ?? */
		int i;
		uint8_t mac_addr[6];
		for (i = 0; i < 6; i++) {
			mac_addr[i] = usb_read8(Adapter, REG_MACID+i);
		}

		DBG_8192C("MAC Address from REG_MACID = "MAC_FMT"\n", MAC_ARG(mac_addr));
	}

exit:

	DBG_871X("%s in %dms\n", __FUNCTION__, rtw_get_passing_time_ms(init_start_time));

	return status;
}

VOID CardDisableRTL8812AU(struct rtl_priv *Adapter)
{
	struct rtl_hal *rtlhal = rtl_hal(Adapter);
	uint8_t	u1bTmp;
	uint8_t 	val8;
	u16	val16;
	uint32_t	val32;

	/* DBG_871X("CardDisableRTL8188EU\n"); */

	/* Stop Tx Report Timer. 0x4EC[Bit1]=b'0 */
	u1bTmp = usb_read8(Adapter, REG_TX_RPT_CTRL);
	usb_write8(Adapter, REG_TX_RPT_CTRL, val8&(~BIT1));

	/* stop rx */
	usb_write8(Adapter, REG_CR, 0x0);

	/* Run LPS WL RFOFF flow */
	if (IS_HARDWARE_TYPE_8821U(rtlhal))
		HalPwrSeqCmdParsing(Adapter, PWR_CUT_ALL_MSK, PWR_FAB_ALL_MSK, PWR_INTF_USB_MSK, Rtl8821A_NIC_LPS_ENTER_FLOW);
	else
		HalPwrSeqCmdParsing(Adapter, PWR_CUT_ALL_MSK, PWR_FAB_ALL_MSK, PWR_INTF_USB_MSK, Rtl8812_NIC_LPS_ENTER_FLOW);

	if ((usb_read8(Adapter, REG_MCUFWDL)&RAM_DL_SEL) && Adapter->bFWReady) {
		  /* 8051 RAM code */
		_8051Reset8812(Adapter);
	}

	/* Reset MCU. Suggested by Filen. 2011.01.26. by tynli. */
	u1bTmp = usb_read8(Adapter, REG_SYS_FUNC_EN+1);
	usb_write8(Adapter, REG_SYS_FUNC_EN+1, (u1bTmp&(~BIT2)));

	/* MCUFWDL 0x80[1:0]=0
	 * reset MCU ready status
	 */
	usb_write8(Adapter, REG_MCUFWDL, 0x00);

	/* Card disable power action flow */
	if (IS_HARDWARE_TYPE_8821U(rtlhal))
		HalPwrSeqCmdParsing(Adapter, PWR_CUT_ALL_MSK, PWR_FAB_ALL_MSK, PWR_INTF_USB_MSK, Rtl8821A_NIC_DISABLE_FLOW);
	else
		HalPwrSeqCmdParsing(Adapter, PWR_CUT_ALL_MSK, PWR_FAB_ALL_MSK, PWR_INTF_USB_MSK, Rtl8812_NIC_DISABLE_FLOW);
}

static void rtl8812au_hw_power_down(struct rtl_priv *rtlpriv)
{
	/*
	 *  2010/-8/09 MH For power down module, we need to enable register block contrl reg at 0x1c.
	 * Then enable power down control bit of register 0x04 BIT4 and BIT15 as 1.
	 */

	/* Enable register area 0x0-0xc. */
	usb_write8(rtlpriv, REG_RSV_CTRL, 0x0);
	usb_write16(rtlpriv, REG_APS_FSMCO, 0x8812);
}

uint32_t rtl8812au_hal_deinit(struct rtl_priv *Adapter)
{

	struct _rtw_hal	*pHalData = GET_HAL_DATA(Adapter);
	DBG_8192C("==> %s \n", __FUNCTION__);

	usb_write16(Adapter, REG_GPIO_MUXCFG, usb_read16(Adapter, REG_GPIO_MUXCFG)&(~BIT12));

	if (pHalData->bSupportUSB3 == _TRUE) {
		/*
		 * set Reg 0xf008[3:4] to 2'11 to eable U1/U2 Mode in USB3.0. added by page, 20120712
		 */
		usb_write8(Adapter, 0xf008, usb_read8(Adapter, 0xf008)|0x18);
	}

	usb_write32(Adapter, REG_HISR0_8812, 0xFFFFFFFF);
	usb_write32(Adapter, REG_HISR1_8812, 0xFFFFFFFF);
	usb_write32(Adapter, REG_HIMR0_8812, IMR_DISABLED_8812);
	usb_write32(Adapter, REG_HIMR1_8812, IMR_DISABLED_8812);

	if (Adapter->hw_init_completed == _TRUE) {
		CardDisableRTL8812AU(Adapter);

		if ((Adapter->pwrctrlpriv.bHWPwrPindetect) && (Adapter->pwrctrlpriv.bHWPowerdown))
			rtl8812au_hw_power_down(Adapter);
	}
	return _SUCCESS;
}


unsigned int rtl8812au_inirp_init(struct rtl_priv *Adapter)
{
	uint8_t i;
	struct recv_buf *precvbuf;
	uint	status;
	struct rtl_usb *pdev = rtl_usbdev(Adapter);
	struct recv_priv *precvpriv = &(Adapter->recvpriv);

	status = _SUCCESS;

	precvpriv->ff_hwaddr = RECV_BULK_IN_ADDR;

	/* issue Rx irp to receive data */
	precvbuf = (struct recv_buf *)precvpriv->precv_buf;
	for (i = 0; i < NR_RECVBUFF; i++) {
		if (usb_read_port(Adapter, precvpriv->ff_hwaddr, 0, (unsigned char *) precvbuf) == _FALSE) {
			status = _FAIL;
			goto exit;
		}

		precvbuf++;
		precvpriv->free_recv_buf_queue_cnt--;
	}

exit:

	return status;

}

unsigned int rtl8812au_inirp_deinit(struct rtl_priv *Adapter)
{
	usb_read_port_cancel(Adapter);
	return _SUCCESS;
}

/*
 * -------------------------------------------------------------------
 *
 * 	EEPROM/EFUSE Content Parsing
 *
 * -------------------------------------------------------------------
 */
VOID hal_ReadIDs_8812AU(struct rtl_priv *Adapter, u8 *PROMContent,
	BOOLEAN	AutoloadFail)
{
	struct rtl_efuse *efuse = rtl_efuse(Adapter);
	struct rtl_hal *rtlhal = rtl_hal(Adapter);
	struct _rtw_hal	*pHalData = GET_HAL_DATA(Adapter);
	EEPROM_EFUSE_PRIV	*pEEPROM = GET_EEPROM_EFUSE_PRIV(Adapter);

	if (!AutoloadFail) {
		/* VID, PID */
		if (IS_HARDWARE_TYPE_8812AU(rtlhal)) {
			efuse->eeprom_vid = EF2Byte(*(u16 *)&PROMContent[EEPROM_VID_8812AU]);
			efuse->EEPROMPID = EF2Byte(*(u16 *)&PROMContent[EEPROM_PID_8812AU]);
		} else if (IS_HARDWARE_TYPE_8821U(rtlhal)) {
			efuse->eeprom_vid = EF2Byte(*(u16 *)&PROMContent[EEPROM_VID_8821AU]);
			efuse->EEPROMPID = EF2Byte(*(u16 *)&PROMContent[EEPROM_PID_8821AU]);
		}

		/* Customer ID, 0x00 and 0xff are reserved for Realtek. */
		efuse->EEPROMCustomerID = *(uint8_t *)&PROMContent[EEPROM_CustomID_8812];
		efuse->EEPROMSubCustomerID = EEPROM_Default_SubCustomerID;

	} else {
		efuse->eeprom_vid = EEPROM_Default_VID;
		efuse->EEPROMPID = EEPROM_Default_PID;

		/* Customer ID, 0x00 and 0xff are reserved for Realtek. */
		efuse->EEPROMCustomerID		= EEPROM_Default_CustomerID;
		efuse->EEPROMSubCustomerID	= EEPROM_Default_SubCustomerID;

	}

	DBG_871X("VID = 0x%04X, PID = 0x%04X\n", efuse->eeprom_vid, efuse->EEPROMPID);
	DBG_871X("Customer ID: 0x%02X, SubCustomer ID: 0x%02X\n", efuse->EEPROMCustomerID, efuse->EEPROMSubCustomerID);
}

VOID hal_ReadMACAddress_8812AU(struct rtl_priv *Adapter, u8 *PROMContent,
	BOOLEAN	AutoloadFail)
{
	struct rtl_hal *rtlhal = rtl_hal(Adapter);
	EEPROM_EFUSE_PRIV *pEEPROM = GET_EEPROM_EFUSE_PRIV(Adapter);

	if (_FALSE == AutoloadFail) {
		if (IS_HARDWARE_TYPE_8812AU(rtlhal)) {
			/* Read Permanent MAC address and set value to hardware */
			memcpy(pEEPROM->mac_addr, &PROMContent[EEPROM_MAC_ADDR_8812AU], ETH_ALEN);
		} else if (IS_HARDWARE_TYPE_8821U(rtlhal)) {
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

VOID hal_InitPGData_8812A(struct rtl_priv *rtlpriv, u8 *PROMContent)
{
	EEPROM_EFUSE_PRIV *pEEPROM = GET_EEPROM_EFUSE_PRIV(rtlpriv);
	/*  struct _rtw_hal	*pHalData = GET_HAL_DATA(rtlpriv); */
	uint32_t			i;
	u16			value16;

	if (_FALSE == pEEPROM->bautoload_fail_flag) { /* autoload OK. */
		if (is_boot_from_eeprom(rtlpriv)) {
			/* Read all Content from EEPROM or EFUSE. */
			for (i = 0; i < HWSET_MAX_SIZE_JAGUAR; i += 2) {
				/*
				 * value16 = EF2Byte(ReadEEprom(rtlpriv, (u16) (i>>1)));
				 * *((u16 *)(&PROMContent[i])) = value16;
				 */
			}
		} else {
			/* Read EFUSE real map to shadow. */
			EFUSE_ShadowMapUpdate(rtlpriv, EFUSE_WIFI);
		}
	} else {	/* autoload fail */
		/*
		 * pHalData->AutoloadFailFlag = _TRUE;
		 * update to default value 0xFF
		 */
		if (!is_boot_from_eeprom(rtlpriv))
			EFUSE_ShadowMapUpdate(rtlpriv, EFUSE_WIFI);
	}
}



static void hal_CustomizeByCustomerID_8812AU(struct rtl_priv *rtlpriv)
{
	struct rtl_efuse *efuse = rtl_efuse(rtlpriv);
	struct _rtw_hal	*pHalData = GET_HAL_DATA(rtlpriv);
	EEPROM_EFUSE_PRIV	*pEEPROM = GET_EEPROM_EFUSE_PRIV(rtlpriv);
	struct rtl_usb_priv *usbpriv = rtl_usbpriv(rtlpriv);
	struct rtl_led_ctl *pledpriv = &(usbpriv->ledpriv);

	/* For customized behavior. */

	if ((efuse->eeprom_vid == 0x050D) && (efuse->EEPROMPID == 0x1106))		/* SerComm for Belkin. */
		pEEPROM->CustomerID = RT_CID_Sercomm_Belkin;	/* ULLI : RTL8812 */
	else if ((efuse->eeprom_vid == 0x0846) && (efuse->EEPROMPID == 0x9052))	/* SerComm for Netgear. */
		pEEPROM->CustomerID = RT_CID_Sercomm_Netgear;	/* ULLI :  posible typo for pid maybe 0x9052 */
	else if ((efuse->eeprom_vid == 0x2001) && (efuse->EEPROMPID == 0x330e))	/* add by ylb 20121012 for customer led for alpha */
		pEEPROM->CustomerID = RT_CID_ALPHA_Dlink;	/* ULLI : RTL8812 */
	else if ((efuse->eeprom_vid == 0x0B05) && (efuse->EEPROMPID == 0x17D2))	/* Edimax for ASUS */
		pEEPROM->CustomerID = RT_CID_Edimax_ASUS;	/* ULLI : RTL8812 */

	DBG_871X("PID= 0x%x, VID=  %x\n", efuse->EEPROMPID, efuse->eeprom_vid);

	/* Decide CustomerID according to VID/DID or EEPROM */
	switch (efuse->EEPROMCustomerID) {
	case EEPROM_CID_DEFAULT:
		if ((efuse->eeprom_vid == 0x0846) && (efuse->EEPROMPID == 0x9052))
			pEEPROM->CustomerID = RT_CID_NETGEAR;		/* ULLI : RTL8821 */

		DBG_871X("PID= 0x%x, VID=  %x\n", efuse->EEPROMPID, efuse->eeprom_vid);
		break;
	default:
		pEEPROM->CustomerID = RT_CID_DEFAULT;
		break;

	}
	DBG_871X("Customer ID: 0x%2x\n", pEEPROM->CustomerID);

	/* Led mode */
	switch (pEEPROM->CustomerID) {
	case RT_CID_DEFAULT:
		pledpriv->LedStrategy = SW_LED_MODE9;
		break;

	/* ULLI check RT_CID_819x values */

	case RT_CID_Sercomm_Belkin:
		pledpriv->LedStrategy = SW_LED_MODE9;
		break;

	case RT_CID_Sercomm_Netgear:
		pledpriv->LedStrategy = SW_LED_MODE10;
		break;

	case RT_CID_ALPHA_Dlink:	/* add by ylb 20121012 for customer led for alpha */
		pledpriv->LedStrategy = SW_LED_MODE1;
		break;

	case RT_CID_Edimax_ASUS:
		pledpriv->LedStrategy = SW_LED_MODE11;
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

static VOID ReadLEDSetting_8812AU(struct rtl_priv *Adapter,
	u8 *PROMContent, BOOLEAN AutoloadFail)
{
}

VOID InitAdapterVariablesByPROM_8812AU(struct rtl_priv *Adapter)
{
	EEPROM_EFUSE_PRIV *pEEPROM = GET_EEPROM_EFUSE_PRIV(Adapter);
	struct rtl_hal *rtlhal = rtl_hal(Adapter);


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

	if (IS_HARDWARE_TYPE_8821U(rtlhal)) {
		Hal_ReadPAType_8821A(Adapter, pEEPROM->efuse_eeprom_data, pEEPROM->bautoload_fail_flag);
	} else {
		Hal_ReadPAType_8812A(Adapter, pEEPROM->efuse_eeprom_data, pEEPROM->bautoload_fail_flag);
		Hal_ReadRFEType_8812A(Adapter, pEEPROM->efuse_eeprom_data, pEEPROM->bautoload_fail_flag);
	}

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
	eeValue = usb_read8(Adapter, REG_9346CR);
	pEEPROM->EepromOrEfuse		= (eeValue & BOOT_FROM_EEPROM) ? _TRUE : _FALSE;
	pEEPROM->bautoload_fail_flag	= (eeValue & EEPROM_EN) ? _FALSE : _TRUE;

	DBG_8192C("Boot from %s, Autoload %s !\n", (pEEPROM->EepromOrEfuse ? "EEPROM" : "EFUSE"),
				(pEEPROM->bautoload_fail_flag ? "Fail" : "OK"));

	/* pHalData->EEType = IS_BOOT_FROM_EEPROM(Adapter) ? EEPROM_93C46 : EEPROM_BOOT_EFUSE; */

	InitAdapterVariablesByPROM_8812AU(Adapter);
}

VOID hal_ReadRFType_8812A(struct rtl_priv *Adapter)
{
	 struct _rtw_hal	*pHalData = GET_HAL_DATA(Adapter);

	pHalData->rf_chip = RF_6052;

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

void ReadAdapterInfo8812AU(struct rtl_priv *Adapter)
{
	struct _rtw_hal	*pHalData = GET_HAL_DATA(Adapter);

	DBG_871X("====> ReadAdapterInfo8812AU\n");

	/* Read all content in Efuse/EEPROM. */
	Hal_ReadPROMContent_8812A(Adapter);

	/* We need to define the RF type after all PROM value is recognized. */
	hal_ReadRFType_8812A(Adapter);

	DBG_871X("ReadAdapterInfo8812AU <====\n");
}

void UpdateInterruptMask8812AU(struct rtl_priv *rtlpriv, uint8_t bHIMR0, uint32_t AddMSR, uint32_t RemoveMSR)
{
	struct _rtw_hal *pHalData;

	uint32_t *himr;
	pHalData = GET_HAL_DATA(rtlpriv);

	if (bHIMR0)
		himr = &(pHalData->IntrMask[0]);
	else
		himr = &(pHalData->IntrMask[1]);

	if (AddMSR)
		*himr |= AddMSR;

	if (RemoveMSR)
		*himr &= (~RemoveMSR);

	if (bHIMR0)
		usb_write32(rtlpriv, REG_HIMR0_8812, *himr);
	else
		usb_write32(rtlpriv, REG_HIMR1_8812, *himr);

}

void _update_response_rate(struct rtl_priv *rtlpriv, unsigned int mask)
{
	uint8_t	RateIndex = 0;
	/* Set RRSR rate table. */
	usb_write8(rtlpriv, REG_RRSR, mask&0xff);
	usb_write8(rtlpriv, REG_RRSR+1, (mask>>8)&0xff);

	/* Set RTS initial rate */
	while (mask > 0x1) {
		mask = (mask >> 1);
		RateIndex++;
	}
	usb_write8(rtlpriv, REG_INIRTS_RATE_SEL, RateIndex);
}

void rtl8812au_init_default_value(struct rtl_priv *rtlpriv)
{
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
