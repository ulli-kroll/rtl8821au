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
#define _RTW_MP_C_

#include <drv_types.h>


#include "../hal/OUTSRC/odm_precomp.h"


#ifdef CONFIG_MP_INCLUDED

void write_bbreg(struct _ADAPTER *padapter, uint32_t addr, uint32_t bitmask,
	uint32_t val)
{
	rtw_hal_write_bbreg(padapter, addr, bitmask, val);
}

uint32_t _read_rfreg(struct _ADAPTER *padapter, uint8_t rfpath, uint32_t addr,
	uint32_t bitmask)
{
	return rtw_hal_read_rfreg(padapter, rfpath, addr, bitmask);
}

void _write_rfreg(struct _ADAPTER *padapter, uint8_t rfpath, uint32_t addr, uint32_t	 bitmask, uint32_t val)
{
	rtw_hal_write_rfreg(padapter, rfpath, addr, bitmask, val);
}

uint32_t read_rfreg(struct _ADAPTER *padapter, uint8_t rfpath, uint32_t	addr)
{
	return _read_rfreg(padapter, rfpath, addr, bRFRegOffsetMask);
}

void write_rfreg(struct _ADAPTER *padapter, uint8_t rfpath, uint32_t addr, uint32_t	val)
{
	_write_rfreg(padapter, rfpath, addr, bRFRegOffsetMask, val);
}

static void _init_mp_priv_(struct mp_priv *pmp_priv)
{
	WLAN_BSSID_EX *pnetwork;

	memset(pmp_priv, 0, sizeof(struct mp_priv));

	pmp_priv->mode = MP_OFF;

	pmp_priv->channel = 1;
	pmp_priv->bandwidth = CHANNEL_WIDTH_20;
	pmp_priv->prime_channel_offset = HAL_PRIME_CHNL_OFFSET_DONT_CARE;
	pmp_priv->rateidx = MPT_RATE_1M;
	pmp_priv->txpoweridx = 0x2A;

	pmp_priv->antenna_tx = ANTENNA_A;
	pmp_priv->antenna_rx = ANTENNA_AB;

	pmp_priv->check_mp_pkt = 0;

	pmp_priv->tx_pktcount = 0;

	pmp_priv->rx_pktcount = 0;
	pmp_priv->rx_crcerrpktcount = 0;

	pmp_priv->network_macaddr[0] = 0x00;
	pmp_priv->network_macaddr[1] = 0xE0;
	pmp_priv->network_macaddr[2] = 0x4C;
	pmp_priv->network_macaddr[3] = 0x87;
	pmp_priv->network_macaddr[4] = 0x66;
	pmp_priv->network_macaddr[5] = 0x55;

	pnetwork = &pmp_priv->mp_network.network;
	memcpy(pnetwork->MacAddress, pmp_priv->network_macaddr, ETH_ALEN);

	pnetwork->Ssid.SsidLength = 8;
	memcpy(pnetwork->Ssid.Ssid, "mp_871x", pnetwork->Ssid.SsidLength);
}


#ifdef PLATFORM_LINUX
static int init_mp_priv_by_os(struct mp_priv *pmp_priv)
{
	int i, res;
	struct mp_xmit_frame *pmp_xmitframe;

	if (pmp_priv == NULL)
		return _FAIL;

	_rtw_init_queue(&pmp_priv->free_mp_xmitqueue);

	pmp_priv->pallocated_mp_xmitframe_buf = NULL;
	pmp_priv->pallocated_mp_xmitframe_buf = rtw_zmalloc(NR_MP_XMITFRAME * sizeof(struct mp_xmit_frame) + 4);
	if (pmp_priv->pallocated_mp_xmitframe_buf == NULL) {
		res = _FAIL;
		goto _exit_init_mp_priv;
	}

	pmp_priv->pmp_xmtframe_buf = pmp_priv->pallocated_mp_xmitframe_buf + 4 - ((SIZE_PTR) (pmp_priv->pallocated_mp_xmitframe_buf) & 3);

	pmp_xmitframe = (struct mp_xmit_frame*)pmp_priv->pmp_xmtframe_buf;

	for (i = 0; i < NR_MP_XMITFRAME; i++) {
		_rtw_init_listhead(&pmp_xmitframe->list);
		rtw_list_insert_tail(&pmp_xmitframe->list, &pmp_priv->free_mp_xmitqueue.queue);

		pmp_xmitframe->pkt = NULL;
		pmp_xmitframe->frame_tag = MP_FRAMETAG;
		pmp_xmitframe->padapter = pmp_priv->papdater;

		pmp_xmitframe++;
	}

	pmp_priv->free_mp_xmitframe_cnt = NR_MP_XMITFRAME;

	res = _SUCCESS;

_exit_init_mp_priv:

	return res;
}
#endif

static void mp_init_xmit_attrib(struct mp_tx *pmptx, struct _ADAPTER *padapter)
{
	struct pkt_attrib *pattrib;
	struct tx_desc *desc;

	// init xmitframe attribute
	pattrib = &pmptx->attrib;
	memset(pattrib, 0, sizeof(struct pkt_attrib));
	desc = &pmptx->desc;
	memset(desc, 0, TXDESC_SIZE);

	pattrib->ether_type = 0x8712;
	//memcpy(pattrib->src, padapter->eeprompriv.mac_addr, ETH_ALEN);
//	memcpy(pattrib->ta, pattrib->src, ETH_ALEN);
	memset(pattrib->dst, 0xFF, ETH_ALEN);

//	pattrib->dhcp_pkt = 0;
//	pattrib->pktlen = 0;
	pattrib->ack_policy = 0;
//	pattrib->pkt_hdrlen = ETH_HLEN;
	pattrib->hdrlen = WLAN_HDR_A3_LEN;
	pattrib->subtype = WIFI_DATA;
	pattrib->priority = 0;
	pattrib->qsel = pattrib->priority;
//	do_queue_select(padapter, pattrib);
	pattrib->nr_frags = 1;
	pattrib->encrypt = 0;
	pattrib->bswenc = _FALSE;
	pattrib->qos_en = _FALSE;
}

int32_t init_mp_priv(struct _ADAPTER *padapter)
{
	struct mp_priv *pmppriv = &padapter->mppriv;

	_init_mp_priv_(pmppriv);
	pmppriv->papdater = padapter;

	pmppriv->tx.stop = 1;
	mp_init_xmit_attrib(&pmppriv->tx, padapter);

	switch (padapter->registrypriv.rf_config) {
	case RF_1T1R:
		pmppriv->antenna_tx = ANTENNA_A;
		pmppriv->antenna_rx = ANTENNA_A;
		break;
	case RF_1T2R:
	default:
		pmppriv->antenna_tx = ANTENNA_A;
		pmppriv->antenna_rx = ANTENNA_AB;
		break;
	case RF_2T2R:
	case RF_2T2R_GREEN:
		pmppriv->antenna_tx = ANTENNA_AB;
		pmppriv->antenna_rx = ANTENNA_AB;
		break;
	case RF_2T4R:
		pmppriv->antenna_tx = ANTENNA_AB;
		pmppriv->antenna_rx = ANTENNA_ABCD;
		break;
	}

	return _SUCCESS;
}

void free_mp_priv(struct mp_priv *pmp_priv)
{
	if (pmp_priv->pallocated_mp_xmitframe_buf) {
		rtw_mfree(pmp_priv->pallocated_mp_xmitframe_buf);
		pmp_priv->pallocated_mp_xmitframe_buf = NULL;
	}
	pmp_priv->pmp_xmtframe_buf = NULL;
}


static VOID PHY_IQCalibrate_default(
	IN	struct _ADAPTER *pAdapter,
	IN	BOOLEAN 	bReCovery
	)
{
	DBG_871X("%s\n", __func__);
}

static VOID PHY_LCCalibrate_default(
	IN	struct _ADAPTER *pAdapter
	)
{
	DBG_871X("%s\n", __func__);
}

static VOID PHY_SetRFPathSwitch_default(
	IN	struct _ADAPTER *pAdapter,
	IN	BOOLEAN		bMain
	)
{
	DBG_871X("%s\n", __func__);
}


#
#if defined(CONFIG_RTL8812A) || defined(CONFIG_RTL8821A)
/*
#define PHY_IQCalibrate(a,b)	PHY_IQCalibrate_8812A(a,b)
#define PHY_LCCalibrate(a)	PHY_LCCalibrate_8812A(&(GET_HAL_DATA(a)->odmpriv))
#define PHY_SetRFPathSwitch(a,b) PHY_SetRFPathSwitch_8812A(a,b)
*/

#ifndef CONFIG_RTL8812A
#define	PHY_IQCalibrate_8812A
#define	PHY_LCCalibrate_8812A
#define	PHY_SetRFPathSwitch_8812A
#endif

#ifndef CONFIG_RTL8821A
#define	PHY_IQCalibrate_8821A
#define	PHY_LCCalibrate_8821A
#define	PHY_SetRFPathSwitch_8812A
#endif

#define PHY_IQCalibrate(_Adapter, b)	\
		IS_HARDWARE_TYPE_8812(_Adapter) ? PHY_IQCalibrate_8812A(_Adapter, b) : \
		IS_HARDWARE_TYPE_8821(_Adapter) ? PHY_IQCalibrate_8821A(_Adapter, b) : \
		PHY_IQCalibrate_default(_Adapter, b)

#define PHY_LCCalibrate(_Adapter)	\
		IS_HARDWARE_TYPE_8812(_Adapter) ? PHY_LCCalibrate_8812A(&(GET_HAL_DATA(_Adapter)->odmpriv)) : \
		IS_HARDWARE_TYPE_8821(_Adapter) ? PHY_LCCalibrate_8821A(&(GET_HAL_DATA(_Adapter)->odmpriv)) : \
		PHY_LCCalibrate_default(_Adapter)

#define PHY_SetRFPathSwitch(_Adapter, b)	\
		(IS_HARDWARE_TYPE_JAGUAR(_Adapter)) ? PHY_SetRFPathSwitch_8812A(_Adapter, b) : \
		PHY_SetRFPathSwitch_default(_Adapter, b)

#endif //#if defined(CONFIG_RTL8812A) || defined(CONFIG_RTL8821A)


int32_t MPT_InitializeAdapter(struct _ADAPTER *pAdapter,
	uint8_t	Channel)
{
	HAL_DATA_TYPE *pHalData = GET_HAL_DATA(pAdapter);
	int32_t	rtStatus = _SUCCESS;
	PMPT_CONTEXT pMptCtx = &pAdapter->mppriv.MptCtx;
	uint32_t ledsetting;
	struct mlme_priv *pmlmepriv = &pAdapter->mlmepriv;

	//-------------------------------------------------------------------------
	// HW Initialization for 8190 MPT.
	//-------------------------------------------------------------------------
	//-------------------------------------------------------------------------
	// SW Initialization for 8190 MP.
	//-------------------------------------------------------------------------
	pMptCtx->bMptDrvUnload = _FALSE;
	pMptCtx->bMassProdTest = _FALSE;
	pMptCtx->bMptIndexEven = _TRUE;	//default gain index is -6.0db
	pMptCtx->h2cReqNum = 0x0;
	/* Init mpt event. */
#if 0 // for Windows
	NdisInitializeEvent( &(pMptCtx->MptWorkItemEvent) );
	NdisAllocateSpinLock( &(pMptCtx->MptWorkItemSpinLock) );

	PlatformInitializeWorkItem(
		Adapter,
		&(pMptCtx->MptWorkItem),
		(RT_WORKITEM_CALL_BACK)MPT_WorkItemCallback,
		(PVOID)Adapter,
		"MptWorkItem");
#endif
	//init for BT MP

	pMptCtx->bMptWorkItemInProgress = _FALSE;
	pMptCtx->CurrMptAct = NULL;
	//-------------------------------------------------------------------------

#if 1
	// Don't accept any packets
	rtw_write32(pAdapter, REG_RCR, 0);
#else
	// Accept CRC error and destination address
	//pHalData->ReceiveConfig |= (RCR_ACRC32|RCR_AAP);
	//rtw_write32(pAdapter, REG_RCR, pHalData->ReceiveConfig);
	rtw_write32(pAdapter, REG_RCR, 0x70000101);
#endif

#if 0
	// If EEPROM or EFUSE is empty,we assign as RF 2T2R for MP.
	if (pHalData->AutoloadFailFlag == TRUE)
	{
		pHalData->RF_Type = RF_2T2R;
	}
#endif

	//ledsetting = rtw_read32(pAdapter, REG_LEDCFG0);
	//rtw_write32(pAdapter, REG_LEDCFG0, ledsetting & ~LED0DIS);

	//rtw_write32(pAdapter, REG_LEDCFG0, 0x08080);
	ledsetting = rtw_read32(pAdapter, REG_LEDCFG0);


	PHY_IQCalibrate(pAdapter, _FALSE);
	dm_CheckTXPowerTracking(&pHalData->odmpriv);	//trigger thermal meter
	PHY_LCCalibrate(pAdapter);


	pMptCtx->backup0xc50 = (u1Byte)PHY_QueryBBReg(pAdapter, rOFDM0_XAAGCCore1, bMaskByte0);
	pMptCtx->backup0xc58 = (u1Byte)PHY_QueryBBReg(pAdapter, rOFDM0_XBAGCCore1, bMaskByte0);
	pMptCtx->backup0xc30 = (u1Byte)PHY_QueryBBReg(pAdapter, rOFDM0_RxDetector1, bMaskByte0);

	//set ant to wifi side in mp mode
	rtw_write16(pAdapter, 0x870, 0x300);
	rtw_write16(pAdapter, 0x860, 0x110);

	if (pAdapter->registrypriv.mp_mode == 1)
		pmlmepriv->fw_state = WIFI_MP_STATE;

	return	rtStatus;
}

/*-----------------------------------------------------------------------------
 * Function:	MPT_DeInitAdapter()
 *
 * Overview:	Extra DeInitialization for Mass Production Test.
 *
 * Input:		struct _ADAPTER *p	pAdapter
 *
 * Output:		NONE
 *
 * Return:		NONE
 *
 * Revised History:
 *	When		Who		Remark
 *	05/08/2007	MHC		Create Version 0.
 *	05/18/2007	MHC		Add normal driver MPHalt code.
 *
 *---------------------------------------------------------------------------*/
VOID MPT_DeInitAdapter(struct _ADAPTER *pAdapter)
{
	PMPT_CONTEXT		pMptCtx = &pAdapter->mppriv.MptCtx;

	pMptCtx->bMptDrvUnload = _TRUE;
#if 0 // for Windows
	PlatformFreeWorkItem( &(pMptCtx->MptWorkItem) );

	while(pMptCtx->bMptWorkItemInProgress) {
		if(NdisWaitEvent(&(pMptCtx->MptWorkItemEvent), 50)) {
			break;
		}
	}
	NdisFreeSpinLock( &(pMptCtx->MptWorkItemSpinLock) );
#endif
}

static uint8_t mpt_ProStartTest(struct _ADAPTER *padapter)
{
	PMPT_CONTEXT pMptCtx = &padapter->mppriv.MptCtx;

	pMptCtx->bMassProdTest = _TRUE;
	pMptCtx->bStartContTx = _FALSE;
	pMptCtx->bCckContTx = _FALSE;
	pMptCtx->bOfdmContTx = _FALSE;
	pMptCtx->bSingleCarrier = _FALSE;
	pMptCtx->bCarrierSuppression = _FALSE;
	pMptCtx->bSingleTone = _FALSE;

	return _SUCCESS;
}

/*
 * General use
 */
int32_t SetPowerTracking(struct _ADAPTER *padapter, uint8_t enable)
{

	Hal_SetPowerTracking( padapter, enable );
	return 0;
}

void GetPowerTracking(struct _ADAPTER *padapter, uint8_t *enable)
{
	Hal_GetPowerTracking( padapter, enable );
}

static void disable_dm(struct _ADAPTER *padapter)
{
	uint8_t v8;

	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(padapter);
	struct dm_priv	*pdmpriv = &pHalData->dmpriv;


	//3 1. disable firmware dynamic mechanism
	// disable Power Training, Rate Adaptive
	v8 = rtw_read8(padapter, REG_BCN_CTRL);
	v8 &= ~EN_BCN_FUNCTION;
	rtw_write8(padapter, REG_BCN_CTRL, v8);

	//3 2. disable driver dynamic mechanism
	// disable Dynamic Initial Gain
	// disable High Power
	// disable Power Tracking
	Switch_DM_Func(padapter, DYNAMIC_FUNC_DISABLE, _FALSE);

	// enable APK, LCK and IQK but disable power tracking
#if (defined(CONFIG_RTL8812A) || defined(CONFIG_RTL8821A))
	pdmpriv->TxPowerTrackControl = _FALSE;
#endif
	Switch_DM_Func(padapter, DYNAMIC_RF_CALIBRATION, _TRUE);
}

//This function initializes the DUT to the MP test mode
int32_t mp_start_test(struct _ADAPTER *padapter)
{
	WLAN_BSSID_EX bssid;
	struct sta_info *psta;
	uint32_t	 length;
	uint8_t val8;

	_irqL irqL;
	int32_t res = _SUCCESS;

	struct mp_priv *pmppriv = &padapter->mppriv;
	struct mlme_priv *pmlmepriv = &padapter->mlmepriv;
	struct wlan_network *tgt_network = &pmlmepriv->cur_network;

	padapter->registrypriv.mp_mode = 1;
	pmppriv->bSetTxPower=0;		//for  manually set tx power

	//3 disable dynamic mechanism
	disable_dm(padapter);
#ifdef CONFIG_RTL8812A
	rtl8812_InitHalDm(padapter);
#endif
	//3 0. update mp_priv

	if (padapter->registrypriv.rf_config == RF_MAX_TYPE) {
//		switch (phal->rf_type) {
		switch (GET_RF_TYPE(padapter)) {
		case RF_1T1R:
			pmppriv->antenna_tx = ANTENNA_A;
			pmppriv->antenna_rx = ANTENNA_A;
			break;
		case RF_1T2R:
		default:
			pmppriv->antenna_tx = ANTENNA_A;
			pmppriv->antenna_rx = ANTENNA_AB;
			break;
		case RF_2T2R:
		case RF_2T2R_GREEN:
			pmppriv->antenna_tx = ANTENNA_AB;
			pmppriv->antenna_rx = ANTENNA_AB;
			break;
		case RF_2T4R:
			pmppriv->antenna_tx = ANTENNA_AB;
			pmppriv->antenna_rx = ANTENNA_ABCD;
			break;
		}
	}

	mpt_ProStartTest(padapter);

	//3 1. initialize a new WLAN_BSSID_EX
//	memset(&bssid, 0, sizeof(WLAN_BSSID_EX));
	memcpy(bssid.MacAddress, pmppriv->network_macaddr, ETH_ALEN);
	bssid.Ssid.SsidLength = strlen("mp_pseudo_adhoc");
	memcpy(bssid.Ssid.Ssid, (uint8_t *)"mp_pseudo_adhoc", bssid.Ssid.SsidLength);
	bssid.InfrastructureMode = Ndis802_11IBSS;
	bssid.NetworkTypeInUse = Ndis802_11DS;
	bssid.IELength = 0;

	length = get_WLAN_BSSID_EX_sz(&bssid);
	if (length % 4)
		bssid.Length = ((length >> 2) + 1) << 2; //round up to multiple of 4 bytes.
	else
		bssid.Length = length;

	_enter_critical_bh(&pmlmepriv->lock, &irqL);

	if (check_fwstate(pmlmepriv, WIFI_MP_STATE) == _TRUE)
		goto end_of_mp_start_test;

	//init mp_start_test status
	if (check_fwstate(pmlmepriv, _FW_LINKED) == _TRUE) {
		rtw_disassoc_cmd(padapter, 500, _TRUE);
		rtw_indicate_disconnect(padapter);
		rtw_free_assoc_resources(padapter, 1);
	}
	pmppriv->prev_fw_state = get_fwstate(pmlmepriv);
	if (padapter->registrypriv.mp_mode == 1)
		pmlmepriv->fw_state = WIFI_MP_STATE;
#if 0
	if (pmppriv->mode == _LOOPBOOK_MODE_) {
		set_fwstate(pmlmepriv, WIFI_MP_LPBK_STATE); //append txdesc
		RT_TRACE(_module_mp_, _drv_notice_, ("+start mp in Lookback mode\n"));
	} else {
		RT_TRACE(_module_mp_, _drv_notice_, ("+start mp in normal mode\n"));
	}
#endif
	set_fwstate(pmlmepriv, _FW_UNDER_LINKING);

	//3 2. create a new psta for mp driver
	//clear psta in the cur_network, if any
	psta = rtw_get_stainfo(&padapter->stapriv, tgt_network->network.MacAddress);
	if (psta) rtw_free_stainfo(padapter, psta);

	psta = rtw_alloc_stainfo(&padapter->stapriv, bssid.MacAddress);
	if (psta == NULL) {
		RT_TRACE(_module_mp_, _drv_err_, ("mp_start_test: Can't alloc sta_info!\n"));
		pmlmepriv->fw_state = pmppriv->prev_fw_state;
		res = _FAIL;
		goto end_of_mp_start_test;
	}

	//3 3. join psudo AdHoc
	tgt_network->join_res = 1;
	tgt_network->aid = psta->aid = 1;
	memcpy(&tgt_network->network, &bssid, length);

	rtw_indicate_connect(padapter);
	_clr_fwstate_(pmlmepriv, _FW_UNDER_LINKING);

end_of_mp_start_test:

	_exit_critical_bh(&pmlmepriv->lock, &irqL);

	if (res == _SUCCESS)
	{
		// set MSR to WIFI_FW_ADHOC_STATE
#if  !defined (CONFIG_RTL8712)
		val8 = rtw_read8(padapter, MSR) & 0xFC; // 0x0102
		val8 |= WIFI_FW_ADHOC_STATE;
		rtw_write8(padapter, MSR, val8); // Link in ad hoc network
#endif


#if  defined (CONFIG_RTL8712)
		rtw_write8(padapter, MSR, 1); // Link in ad hoc network
		rtw_write8(padapter, RCR, 0); // RCR : disable all pkt, 0x10250048
		rtw_write8(padapter, RCR+2, 0x57); // RCR disable Check BSSID, 0x1025004a

		// disable RX filter map , mgt frames will put in RX FIFO 0
		rtw_write16(padapter, RXFLTMAP0, 0x0); // 0x10250116

		val8 = rtw_read8(padapter, EE_9346CR); // 0x1025000A
		if (!(val8 & _9356SEL))//boot from EFUSE
			efuse_change_max_size(padapter);
#endif
	}

	return res;
}
//------------------------------------------------------------------------------
//This function change the DUT from the MP test mode into normal mode
void mp_stop_test(struct _ADAPTER *padapter)
{
	struct mp_priv *pmppriv = &padapter->mppriv;
	struct mlme_priv *pmlmepriv = &padapter->mlmepriv;
	struct wlan_network *tgt_network = &pmlmepriv->cur_network;
	struct sta_info *psta;

	_irqL irqL;

	if (pmppriv->mode == MP_ON) {
		pmppriv->bSetTxPower=0;
		_enter_critical_bh(&pmlmepriv->lock, &irqL);

		if (check_fwstate(pmlmepriv, WIFI_MP_STATE) == _FALSE)
			goto end_of_mp_stop_test;

			//3 1. disconnect psudo AdHoc
			rtw_indicate_disconnect(padapter);

		//3 2. clear psta used in mp test mode.
		//	rtw_free_assoc_resources(padapter, 1);
		psta = rtw_get_stainfo(&padapter->stapriv, tgt_network->network.MacAddress);
		if (psta)
			rtw_free_stainfo(padapter, psta);

		//3 3. return to normal state (default:station mode)
		pmlmepriv->fw_state = pmppriv->prev_fw_state; // WIFI_STATION_STATE;

		//flush the cur_network
		memset(tgt_network, 0, sizeof(struct wlan_network));

		_clr_fwstate_(pmlmepriv, WIFI_MP_STATE);

end_of_mp_stop_test:
		_exit_critical_bh(&pmlmepriv->lock, &irqL);
	}
}
/*---------------------------hal\rtl8192c\MPT_Phy.c---------------------------*/
#if 0
//#ifdef CONFIG_USB_HCI
static VOID mpt_AdjustRFRegByRateByChan92CU(struct _ADAPTER *pAdapter,
	uint8_t RateIdx, uint8_t Channel, uint8_t BandWidthID)
{
	uint8_t		eRFPath;
	uint32_t		rfReg0x26;
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(pAdapter);


	if (RateIdx < MPT_RATE_6M) {	// CCK rate,for 88cu
		rfReg0x26 = 0xf400;
	} else if ((RateIdx >= MPT_RATE_6M) && (RateIdx <= MPT_RATE_54M)) {// OFDM rate,for 88cu
		if ((4 == Channel) || (8 == Channel) || (12 == Channel))
			rfReg0x26 = 0xf000;
		else if ((5 == Channel) || (7 == Channel) || (13 == Channel) || (14 == Channel))
			rfReg0x26 = 0xf400;
		else
			rfReg0x26 = 0x4f200;
	} else
		if ((RateIdx >= MPT_RATE_MCS0) && (RateIdx <= MPT_RATE_MCS15)) {// MCS 20M ,for 88cu // MCS40M rate,for 88cu
			if (CHANNEL_WIDTH_20 == BandWidthID) {
				if ((4 == Channel) || (8 == Channel))
					rfReg0x26 = 0xf000;
				else if ((5 == Channel) || (7 == Channel) || (13 == Channel) || (14 == Channel))
					rfReg0x26 = 0xf400;
				else
					rfReg0x26 = 0x4f200;
			} else{
				if ((4 == Channel) || (8 == Channel))
					rfReg0x26 = 0xf000;
				else if ((5 == Channel) || (7 == Channel))
					rfReg0x26 = 0xf400;
				else
					rfReg0x26 = 0x4f200;
			}
		}

//	RT_TRACE(COMP_CMD, DBG_LOUD, ("\n mpt_AdjustRFRegByRateByChan92CU():Chan:%d Rate=%d rfReg0x26:0x%08x\n",Channel, RateIdx,rfReg0x26));
		for (eRFPath = 0; eRFPath < pHalData->NumTotalRFPath; eRFPath++) {
			write_rfreg(pAdapter, eRFPath, RF_SYN_G2, rfReg0x26);
	}
}
#endif
/*-----------------------------------------------------------------------------
 * Function:	mpt_SwitchRfSetting
 *
 * Overview:	Change RF Setting when we siwthc channel/rate/BW for MP.
 *
 * Input:       IN	struct _ADAPTER *p				pAdapter
 *
 * Output:      NONE
 *
 * Return:      NONE
 *
 * Revised History:
 * When			Who		Remark
 * 01/08/2009	MHC		Suggestion from SD3 Willis for 92S series.
 * 01/09/2009	MHC		Add CCK modification for 40MHZ. Suggestion from SD3.
 *
 *---------------------------------------------------------------------------*/
static void mpt_SwitchRfSetting(struct _ADAPTER *pAdapter)
{
	Hal_mpt_SwitchRfSetting(pAdapter);
}

/*---------------------------hal\rtl8192c\MPT_Phy.c---------------------------*/
/*---------------------------hal\rtl8192c\MPT_HelperFunc.c---------------------------*/

static void MPT_CCKTxPowerAdjust(struct _ADAPTER *Adapter, BOOLEAN bInCH14)
{
	Hal_MPT_CCKTxPowerAdjust(Adapter,bInCH14);
}

static void MPT_CCKTxPowerAdjustbyIndex(struct _ADAPTER *pAdapter, BOOLEAN beven)
{
	Hal_MPT_CCKTxPowerAdjustbyIndex(pAdapter,beven);
}

/*---------------------------hal\rtl8192c\MPT_HelperFunc.c---------------------------*/

/*
 * SetChannel
 * Description
 *	Use H2C command to change channel,
 *	not only modify rf register, but also other setting need to be done.
 */
void SetChannel(struct _ADAPTER *pAdapter)
{
	Hal_SetChannel(pAdapter);
}

/*
 * Notice
 *	Switch bandwitdth may change center frequency(channel)
 */

void SetBandwidth(struct _ADAPTER *pAdapter)
{
	Hal_SetBandwidth(pAdapter);

}

static void SetCCKTxPower(struct _ADAPTER *pAdapter, uint8_t *TxPower)
{
	Hal_SetCCKTxPower(pAdapter,TxPower);
}

static void SetOFDMTxPower(struct _ADAPTER *pAdapter, uint8_t *TxPower)
{
	Hal_SetOFDMTxPower(pAdapter,TxPower);
}


void SetAntenna(struct _ADAPTER *pAdapter)
	{
	Hal_SetAntenna(pAdapter);
}

void SetAntennaPathPower(struct _ADAPTER *pAdapter)
{
	Hal_SetAntennaPathPower(pAdapter);
}

int SetTxPower(struct _ADAPTER *pAdapter)
{
	HAL_DATA_TYPE	*pHalData	= GET_HAL_DATA(pAdapter);
	u1Byte			CurrChannel;
	BOOLEAN 		bResult = _TRUE;
	PMPT_CONTEXT	pMptCtx = &(pAdapter->mppriv.MptCtx);
	u1Byte			rf, TxPower[2];

	uint8_t u1TxPower = pAdapter->mppriv.txpoweridx;
	CurrChannel = pMptCtx->MptChannelToSw;

	if(HAL_IsLegalChannel(pAdapter, CurrChannel) == _FALSE)
	{
		DBG_871X("SetTxPower(): CurrentChannel:%d is not valid\n", CurrChannel);
		return _FALSE;
	}

	TxPower[ODM_RF_PATH_A] = (u1Byte)(u1TxPower&0xff);
	TxPower[ODM_RF_PATH_B] = (u1Byte)((u1TxPower&0xff00)>>8);
	DBG_871X("TxPower(A, B) = (0x%x, 0x%x)\n", TxPower[ODM_RF_PATH_A], TxPower[ODM_RF_PATH_B]);

	for(rf=0; rf<2; rf++)
	{
		if(TxPower[rf] > MAX_TX_PWR_INDEX_N_MODE) {
			DBG_871X("===> SetTxPower: The power index is too large.\n");
			return _FALSE;
		}
		pMptCtx->TxPwrLevel[rf] = TxPower[rf];
	}
	Hal_SetTxPower(pAdapter);
	return _TRUE;
}

void SetTxAGCOffset(struct _ADAPTER *pAdapter, uint32_t	 ulTxAGCOffset)
{
	uint32_t	 TxAGCOffset_B, TxAGCOffset_C, TxAGCOffset_D,tmpAGC;

	TxAGCOffset_B = (ulTxAGCOffset&0x000000ff);
	TxAGCOffset_C = ((ulTxAGCOffset&0x0000ff00)>>8);
	TxAGCOffset_D = ((ulTxAGCOffset&0x00ff0000)>>16);

	tmpAGC = (TxAGCOffset_D<<8 | TxAGCOffset_C<<4 | TxAGCOffset_B);
	write_bbreg(pAdapter, rFPGA0_TxGainStage,
			(bXBTxAGC|bXCTxAGC|bXDTxAGC), tmpAGC);
}

void SetDataRate(struct _ADAPTER *pAdapter)
{
	Hal_SetDataRate(pAdapter);
}

void MP_PHY_SetRFPathSwitch(struct _ADAPTER *pAdapter ,BOOLEAN bMain)
{

	PHY_SetRFPathSwitch(pAdapter,bMain);

}


int32_t SetThermalMeter(struct _ADAPTER *pAdapter, uint8_t target_ther)
{
	return Hal_SetThermalMeter( pAdapter, target_ther);
}

static void TriggerRFThermalMeter(struct _ADAPTER *pAdapter)
{
	Hal_TriggerRFThermalMeter(pAdapter);
}

static uint8_t ReadRFThermalMeter(struct _ADAPTER *pAdapter)
{
	return Hal_ReadRFThermalMeter(pAdapter);
}

void GetThermalMeter(struct _ADAPTER *pAdapter, uint8_t *value)
{
	Hal_GetThermalMeter(pAdapter,value);
}

void SetSingleCarrierTx(struct _ADAPTER *pAdapter, uint8_t bStart)
{
	PhySetTxPowerLevel(pAdapter);
	Hal_SetSingleCarrierTx(pAdapter,bStart);
}

void SetSingleToneTx(struct _ADAPTER *pAdapter, uint8_t bStart)
{
	PhySetTxPowerLevel(pAdapter);
	Hal_SetSingleToneTx(pAdapter,bStart);
}

void SetCarrierSuppressionTx(struct _ADAPTER *pAdapter, uint8_t bStart)
{
	PhySetTxPowerLevel(pAdapter);
	Hal_SetCarrierSuppressionTx(pAdapter, bStart);
}

void SetCCKContinuousTx(struct _ADAPTER *pAdapter, uint8_t bStart)
{
	PhySetTxPowerLevel(pAdapter);
	Hal_SetCCKContinuousTx(pAdapter,bStart);
}

void SetOFDMContinuousTx(struct _ADAPTER *pAdapter, uint8_t bStart)
{
	PhySetTxPowerLevel(pAdapter);
   	Hal_SetOFDMContinuousTx( pAdapter, bStart);
}/* mpt_StartOfdmContTx */

void SetContinuousTx(struct _ADAPTER *pAdapter, uint8_t bStart)
{
	PhySetTxPowerLevel(pAdapter);
	Hal_SetContinuousTx(pAdapter,bStart);
}


void PhySetTxPowerLevel(struct _ADAPTER *pAdapter)
{
	struct mp_priv *pmp_priv = &pAdapter->mppriv;

	if (pmp_priv->bSetTxPower==0) // for NO manually set power index
	{
#if defined(CONFIG_RTL8812A) || defined(CONFIG_RTL8821A)
		PHY_SetTxPowerLevel8812(pAdapter,pmp_priv->channel);
#endif

	}
}

//------------------------------------------------------------------------------
static void dump_mpframe(struct _ADAPTER *padapter, struct xmit_frame *pmpframe)
{
	rtw_hal_mgnt_xmit(padapter, pmpframe);
}

static struct xmit_frame *alloc_mp_xmitframe(struct xmit_priv *pxmitpriv)
{
	struct xmit_frame	*pmpframe;
	struct xmit_buf	*pxmitbuf;

	if ((pmpframe = rtw_alloc_xmitframe(pxmitpriv)) == NULL)
	{
		return NULL;
	}

	if ((pxmitbuf = rtw_alloc_xmitbuf(pxmitpriv)) == NULL)
	{
		rtw_free_xmitframe(pxmitpriv, pmpframe);
		return NULL;
	}

	pmpframe->frame_tag = MP_FRAMETAG;

	pmpframe->pxmitbuf = pxmitbuf;

	pmpframe->buf_addr = pxmitbuf->pbuf;

	pxmitbuf->priv_data = pmpframe;

	return pmpframe;

}

static thread_return mp_xmit_packet_thread(thread_context context)
{
	struct xmit_frame	*pxmitframe;
	struct mp_tx		*pmptx;
	struct mp_priv	*pmp_priv;
	struct xmit_priv	*pxmitpriv;
	struct _ADAPTER *padapter;

	pmp_priv = (struct mp_priv *)context;
	pmptx = &pmp_priv->tx;
	padapter = pmp_priv->papdater;
	pxmitpriv = &(padapter->xmitpriv);

	thread_enter("RTW_MP_THREAD");

	DBG_871X("%s:pkTx Start\n", __func__);
	while (1) {
		pxmitframe = alloc_mp_xmitframe(pxmitpriv);
		if (pxmitframe == NULL) {
			if (pmptx->stop ||
			    padapter->bSurpriseRemoved ||
			    padapter->bDriverStopped) {
				goto exit;
			}
			else {
				rtw_msleep_os(1);
				continue;
			}
		}
		memcpy((uint8_t *)(pxmitframe->buf_addr+TXDESC_OFFSET), pmptx->buf, pmptx->write_size);
		memcpy(&(pxmitframe->attrib), &(pmptx->attrib), sizeof(struct pkt_attrib));

		dump_mpframe(padapter, pxmitframe);
		pmptx->sended++;
		pmp_priv->tx_pktcount++;

		if (pmptx->stop ||
		    padapter->bSurpriseRemoved ||
		    padapter->bDriverStopped)
			goto exit;
		if ((pmptx->count != 0) &&
		    (pmptx->count == pmptx->sended))
			goto exit;

		flush_signals_thread();
	}

exit:
	//DBG_871X("%s:pkTx Exit\n", __func__);
	/* ULLI check usage of pmptx->buf_size */
	rtw_mfree(pmptx->pallocated_buf);
	pmptx->pallocated_buf = NULL;
	pmptx->stop = 1;

	thread_exit();
}

void fill_txdesc_for_mp(struct _ADAPTER *padapter, struct tx_desc *ptxdesc)
{
	struct mp_priv *pmp_priv = &padapter->mppriv;
	memcpy(ptxdesc, &(pmp_priv->tx.desc), TXDESC_SIZE);
}


#if defined(CONFIG_RTL8812A) || defined(CONFIG_RTL8821A)
void fill_tx_desc_8812a(struct _ADAPTER *padapter)
{
	struct mp_priv *pmp_priv = &padapter->mppriv;
	//struct tx_desc *pDesc   = &(pmp_priv->tx.desc);
	uint8_t *pDesc   = (uint8_t *)&(pmp_priv->tx.desc);
	struct pkt_attrib *pattrib = &(pmp_priv->tx.attrib);

	uint32_t	pkt_size = pattrib->last_txcmdsz;
	int32_t bmcast = IS_MCAST(pattrib->ra);
	uint8_t data_rate,pwr_status,offset;

	SET_TX_DESC_FIRST_SEG_8812(pDesc, 1);
	SET_TX_DESC_LAST_SEG_8812(pDesc, 1);
	SET_TX_DESC_OWN_8812(pDesc, 1);

	SET_TX_DESC_PKT_SIZE_8812(pDesc, pkt_size);

	offset = TXDESC_SIZE + OFFSET_SZ;

	SET_TX_DESC_OFFSET_8812(pDesc, offset);
	SET_TX_DESC_PKT_OFFSET_8812(pDesc, 1);

	if (bmcast) {
		SET_TX_DESC_BMC_8812(pDesc, 1);
	}

	SET_TX_DESC_MACID_8812(pDesc, pattrib->mac_id);
	SET_TX_DESC_RATE_ID_8812(pDesc, pattrib->raid);

	//SET_TX_DESC_RATE_ID_8812(pDesc, RATEID_IDX_G);
	SET_TX_DESC_QUEUE_SEL_8812(pDesc,  pattrib->qsel);
	//SET_TX_DESC_QUEUE_SEL_8812(pDesc,  QSLT_MGNT);

	if (!pattrib->qos_en) {
		SET_TX_DESC_HWSEQ_EN_8812(pDesc, 1); // Hw set sequence number
	} else {
		SET_TX_DESC_SEQ_8812(pDesc, pattrib->seqnum);
	}

	SET_TX_DESC_DISABLE_FB_8812(pDesc, 1);
	SET_TX_DESC_USE_RATE_8812(pDesc, 1);
	SET_TX_DESC_TX_RATE_8812(pDesc, pmp_priv->rateidx);

}
#endif


void SetPacketTx(struct _ADAPTER *padapter)
{
	uint8_t *ptr, *pkt_start, *pkt_end;
	uint32_t	 pkt_size,offset;
	struct tx_desc *desc;
	struct rtw_ieee80211_hdr *hdr;
	uint8_t payload;
	int32_t bmcast;
	struct pkt_attrib *pattrib;
	struct mp_priv *pmp_priv;

	pmp_priv = &padapter->mppriv;

	if (pmp_priv->tx.stop) return;
	pmp_priv->tx.sended = 0;
	pmp_priv->tx.stop = 0;
	pmp_priv->tx_pktcount = 0;

	//3 1. update_attrib()
	pattrib = &pmp_priv->tx.attrib;
	memcpy(pattrib->src, padapter->eeprompriv.mac_addr, ETH_ALEN);
	memcpy(pattrib->ta, pattrib->src, ETH_ALEN);
	memcpy(pattrib->ra, pattrib->dst, ETH_ALEN);
	bmcast = IS_MCAST(pattrib->ra);
	if (bmcast) {
		pattrib->mac_id = 1;
		pattrib->psta = rtw_get_bcmc_stainfo(padapter);
	} else {
		pattrib->mac_id = 0;
		pattrib->psta = rtw_get_stainfo(&padapter->stapriv, get_bssid(&padapter->mlmepriv));
	}

	pattrib->last_txcmdsz = pattrib->hdrlen + pattrib->pktlen;

	//3 2. allocate xmit buffer
	pkt_size = pattrib->last_txcmdsz;

	if (pmp_priv->tx.pallocated_buf) {
		/* ULLI check usage of pmp_priv->tx.buf_size */
		rtw_mfree(pmp_priv->tx.pallocated_buf);
	}
	pmp_priv->tx.write_size = pkt_size;
	pmp_priv->tx.buf_size = pkt_size + XMITBUF_ALIGN_SZ;
	pmp_priv->tx.pallocated_buf = rtw_zmalloc(pmp_priv->tx.buf_size);
	if (pmp_priv->tx.pallocated_buf == NULL) {
		DBG_871X("%s: malloc(%d) fail!!\n", __func__, pmp_priv->tx.buf_size);
		return;
	}
	pmp_priv->tx.buf = (uint8_t *)N_BYTE_ALIGMENT((SIZE_PTR)(pmp_priv->tx.pallocated_buf), XMITBUF_ALIGN_SZ);
	ptr = pmp_priv->tx.buf;

	desc = &(pmp_priv->tx.desc);
	memset(desc, 0, TXDESC_SIZE);
	pkt_start = ptr;
	pkt_end = pkt_start + pkt_size;

	//3 3. init TX descriptor


#if defined(CONFIG_RTL8812A) || defined(CONFIG_RTL8821A)
	if(IS_HARDWARE_TYPE_8812(padapter))
		fill_tx_desc_8812a(padapter);
#endif



	//3 4. make wlan header, make_wlanhdr()
	hdr = (struct rtw_ieee80211_hdr *)pkt_start;
	SetFrameSubType(&hdr->frame_ctl, pattrib->subtype);
	memcpy(hdr->addr1, pattrib->dst, ETH_ALEN); // DA
	memcpy(hdr->addr2, pattrib->src, ETH_ALEN); // SA
	memcpy(hdr->addr3, get_bssid(&padapter->mlmepriv), ETH_ALEN); // RA, BSSID

	//3 5. make payload
	ptr = pkt_start + pattrib->hdrlen;

	switch (pmp_priv->tx.payload) {
		case 0:
			payload = 0x00;
			break;
		case 1:
			payload = 0x5a;
			break;
		case 2:
			payload = 0xa5;
			break;
		case 3:
			payload = 0xff;
			break;
		default:
			payload = 0x00;
			break;
	}

	memset(ptr, payload, pkt_end - ptr);

	//3 6. start thread
#ifdef PLATFORM_LINUX
	pmp_priv->tx.PktTxThread = kthread_run(mp_xmit_packet_thread, pmp_priv, "RTW_MP_THREAD");
	if (IS_ERR(pmp_priv->tx.PktTxThread))
		DBG_871X("Create PktTx Thread Fail !!!!!\n");
#endif
}

void SetPacketRx(struct _ADAPTER *pAdapter, uint8_t bStartRx)
{
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(pAdapter);

	if(bStartRx)
	{
		// Accept CRC error and destination address
#if 1
//ndef CONFIG_RTL8723A
		pHalData->ReceiveConfig = AAP | APM | AM | AB | APP_ICV | ADF | AMF | HTC_LOC_CTRL | APP_MIC | APP_PHYSTS;

		pHalData->ReceiveConfig |= ACRC32;

		rtw_write32(pAdapter, REG_RCR, pHalData->ReceiveConfig);

		// Accept all data frames
		rtw_write16(pAdapter, REG_RXFLTMAP2, 0xFFFF);
#else
		rtw_write32(pAdapter, REG_RCR, 0x70000101);
#endif
	}
	else
	{
		rtw_write32(pAdapter, REG_RCR, 0);
	}
}

void ResetPhyRxPktCount(struct _ADAPTER *pAdapter)
{
	uint32_t	 i, phyrx_set = 0;

	for (i = 0; i <= 0xF; i++) {
		phyrx_set = 0;
		phyrx_set |= _RXERR_RPT_SEL(i);	//select
		phyrx_set |= RXERR_RPT_RST;	// set counter to zero
		rtw_write32(pAdapter, REG_RXERR_RPT, phyrx_set);
	}
}

static uint32_t	 GetPhyRxPktCounts(struct _ADAPTER *pAdapter, uint32_t	 selbit)
{
	//selection
	uint32_t	 phyrx_set = 0, count = 0;

	phyrx_set = _RXERR_RPT_SEL(selbit & 0xF);
	rtw_write32(pAdapter, REG_RXERR_RPT, phyrx_set);

	//Read packet count
	count = rtw_read32(pAdapter, REG_RXERR_RPT) & RXERR_COUNTER_MASK;

	return count;
}

uint32_t	 GetPhyRxPktReceived(struct _ADAPTER *pAdapter)
{
	uint32_t	 OFDM_cnt = 0, CCK_cnt = 0, HT_cnt = 0;

	OFDM_cnt = GetPhyRxPktCounts(pAdapter, RXERR_TYPE_OFDM_MPDU_OK);
	CCK_cnt = GetPhyRxPktCounts(pAdapter, RXERR_TYPE_CCK_MPDU_OK);
	HT_cnt = GetPhyRxPktCounts(pAdapter, RXERR_TYPE_HT_MPDU_OK);

	return OFDM_cnt + CCK_cnt + HT_cnt;
}

uint32_t	 GetPhyRxPktCRC32Error(struct _ADAPTER *pAdapter)
{
	uint32_t	 OFDM_cnt = 0, CCK_cnt = 0, HT_cnt = 0;

	OFDM_cnt = GetPhyRxPktCounts(pAdapter, RXERR_TYPE_OFDM_MPDU_FAIL);
	CCK_cnt = GetPhyRxPktCounts(pAdapter, RXERR_TYPE_CCK_MPDU_FAIL);
	HT_cnt = GetPhyRxPktCounts(pAdapter, RXERR_TYPE_HT_MPDU_FAIL);

	return OFDM_cnt + CCK_cnt + HT_cnt;
}

//reg 0x808[9:0]: FFT data x
//reg 0x808[22]:  0  -->  1  to get 1 FFT data y
//reg 0x8B4[15:0]: FFT data y report
static uint32_t	 rtw_GetPSDData(struct _ADAPTER *pAdapter, uint32_t	 point)
{
	uint32_t	 psd_val=0;

#if defined(CONFIG_RTL8812A) //MP PSD for 8812A
	uint16_t psd_reg = 0x910;
	uint16_t psd_regL= 0xF44;

#else
	uint16_t psd_reg = 0x808;
	uint16_t psd_regL= 0x8B4;

#endif

	psd_val = rtw_read32(pAdapter, psd_reg);

	psd_val &= 0xFFBFFC00;
	psd_val |= point;

	rtw_write32(pAdapter, psd_reg, psd_val);
	rtw_mdelay_os(1);
	psd_val |= 0x00400000;

	rtw_write32(pAdapter, psd_reg, psd_val);
	rtw_mdelay_os(1);

	psd_val = rtw_read32(pAdapter, psd_regL);
	psd_val &= 0x0000FFFF;

	return psd_val;
}

/*
 * pts	start_point_min		stop_point_max
 * 128	64			64 + 128 = 192
 * 256	128			128 + 256 = 384
 * 512	256			256 + 512 = 768
 * 1024	512			512 + 1024 = 1536
 *
 */
uint32_t	 mp_query_psd(struct _ADAPTER *pAdapter, uint8_t *data)
{
	uint32_t	 i, psd_pts=0, psd_start=0, psd_stop=0;
	uint32_t	 psd_data=0;


#ifdef PLATFORM_LINUX
	if (!netif_running(pAdapter->ndev)) {
		RT_TRACE(_module_mp_, _drv_warning_, ("mp_query_psd: Fail! interface not opened!\n"));
		return 0;
	}
#endif

	if (check_fwstate(&pAdapter->mlmepriv, WIFI_MP_STATE) == _FALSE) {
		RT_TRACE(_module_mp_, _drv_warning_, ("mp_query_psd: Fail! not in MP mode!\n"));
		return 0;
	}

	if (strlen(data) == 0) { //default value
		psd_pts = 128;
		psd_start = 64;
		psd_stop = 128;
	} else {
		sscanf(data, "pts=%d,start=%d,stop=%d", &psd_pts, &psd_start, &psd_stop);
	}

	memset(data, '\0', sizeof(data));

	i = psd_start;
	while (i < psd_stop)
	{
		if (i >= psd_pts) {
			psd_data = rtw_GetPSDData(pAdapter, i-psd_pts);
		} else {
			psd_data = rtw_GetPSDData(pAdapter, i);
		}
		sprintf(data, "%s%x ", data, psd_data);
		i++;
	}

	#ifdef CONFIG_LONG_DELAY_ISSUE
	rtw_msleep_os(100);
	#else
	rtw_mdelay_os(100);
	#endif

	return strlen(data)+1;
}



void _rtw_mp_xmit_priv (struct xmit_priv *pxmitpriv)
{
	   int i,res;
	  struct _ADAPTER *padapter = pxmitpriv->adapter;
	struct xmit_frame	*pxmitframe = (struct xmit_frame*) pxmitpriv->pxmit_frame_buf;
	struct xmit_buf *pxmitbuf = (struct xmit_buf *)pxmitpriv->pxmitbuf;

	uint32_t	 max_xmit_extbuf_size = MAX_XMIT_EXTBUF_SZ;
	uint32_t	 num_xmit_extbuf = NR_XMIT_EXTBUFF;
	if(padapter->registrypriv.mp_mode ==0)
	{
		max_xmit_extbuf_size = MAX_XMIT_EXTBUF_SZ;
		num_xmit_extbuf = NR_XMIT_EXTBUFF;
	}
	else
	{
		max_xmit_extbuf_size = 20000;
		num_xmit_extbuf = 1;
	}

	pxmitbuf = (struct xmit_buf *)pxmitpriv->pxmit_extbuf;
	for(i=0; i<num_xmit_extbuf; i++)
	{
		rtw_os_xmit_resource_free(padapter, pxmitbuf,(max_xmit_extbuf_size + XMITBUF_ALIGN_SZ), _FALSE);

		pxmitbuf++;
	}

	if(pxmitpriv->pallocated_xmit_extbuf) {
		rtw_vmfree(pxmitpriv->pallocated_xmit_extbuf);
	}

	if(padapter->registrypriv.mp_mode ==0)
	{
		max_xmit_extbuf_size = 20000;
		num_xmit_extbuf = 1;
	}
	else
	{
		max_xmit_extbuf_size = MAX_XMIT_EXTBUF_SZ;
		num_xmit_extbuf = NR_XMIT_EXTBUFF;
	}

	// Init xmit extension buff
	_rtw_init_queue(&pxmitpriv->free_xmit_extbuf_queue);

	pxmitpriv->pallocated_xmit_extbuf = rtw_zvmalloc(num_xmit_extbuf * sizeof(struct xmit_buf) + 4);

	if (pxmitpriv->pallocated_xmit_extbuf  == NULL){
		RT_TRACE(_module_rtl871x_xmit_c_,_drv_err_,("alloc xmit_extbuf fail!\n"));
		res= _FAIL;
		goto exit;
	}

	pxmitpriv->pxmit_extbuf = (uint8_t *)N_BYTE_ALIGMENT((SIZE_PTR)(pxmitpriv->pallocated_xmit_extbuf), 4);

	pxmitbuf = (struct xmit_buf *)pxmitpriv->pxmit_extbuf;

	for (i = 0; i < num_xmit_extbuf; i++)
	{
		_rtw_init_listhead(&pxmitbuf->list);

		pxmitbuf->priv_data = NULL;
		pxmitbuf->padapter = padapter;
		pxmitbuf->buf_tag = XMITBUF_MGNT;

		if((res=rtw_os_xmit_resource_alloc(padapter, pxmitbuf,max_xmit_extbuf_size + XMITBUF_ALIGN_SZ, _FALSE)) == _FAIL) {
			res= _FAIL;
			goto exit;
		}


		rtw_list_insert_tail(&pxmitbuf->list, &(pxmitpriv->free_xmit_extbuf_queue.queue));
		pxmitbuf++;

	}

	pxmitpriv->free_xmit_extbuf_cnt = num_xmit_extbuf;

exit:
	;
}



ULONG getPowerDiffByRate8188E(
	IN	struct _ADAPTER *pAdapter,
	IN	u1Byte		CurrChannel,
	IN	ULONG		RfPath
	)
{
	PMPT_CONTEXT			pMptCtx = &(pAdapter->mppriv.MptCtx);
	HAL_DATA_TYPE	*pHalData	= GET_HAL_DATA(pAdapter);
	ULONG	PwrGroup=0;
	ULONG	TxPower=0, Limit=0;
	ULONG	Pathmapping = (RfPath == ODM_RF_PATH_A?0:8);

	switch(pHalData->EEPROMRegulatory) {
	case 0: // driver-defined maximum power offset for longer communication range
			// refer to power by rate table
		PwrGroup = 0;
		Limit = 0xff;
		break;
	case 1: // Power-limit table-defined maximum power offset range
			// choosed by min(power by rate, power limit).
		{
			if(pHalData->pwrGroupCnt == 1)
				PwrGroup = 0;
			if(pHalData->pwrGroupCnt >= 3)
			{
				if(CurrChannel <= 3)
					PwrGroup = 0;
				else if(CurrChannel >= 4 && CurrChannel <= 9)
					PwrGroup = 1;
				else if(CurrChannel > 9)
					PwrGroup = 2;

				if(pHalData->CurrentChannelBW == CHANNEL_WIDTH_20)
					PwrGroup++;
				else
					PwrGroup+=4;
			}
			Limit = 0xff;
		}
		break;
	case 2: // not support power offset by rate.
			// don't increase any power diff
		PwrGroup = 0;
		Limit = 0;
		break;
	default:
		PwrGroup = 0;
		Limit = 0xff;
		break;
	}


	{
		switch(pMptCtx->MptRateIndex) {
		case MPT_RATE_1M:
		case MPT_RATE_2M:
		case MPT_RATE_55M:
		case MPT_RATE_11M:
			//CCK rates, don't add any tx power index.
			//RT_DISP(FPHY, PHY_TXPWR,("CCK rates!\n"));
			break;
		case MPT_RATE_6M:	//0xe00 [31:0] = 18M,12M,09M,06M
			TxPower += ((pHalData->MCSTxPowerLevelOriginalOffset[PwrGroup][0+Pathmapping])&0xff);
			//RT_DISP(FPHY, PHY_TXPWR,("MCSTxPowerLevelOriginalOffset[%d][0] = 0x%x, OFDM 6M, TxPower = %d\n",
			//	PwrGroup, pHalData->MCSTxPowerLevelOriginalOffset[PwrGroup][0], TxPower));
			break;
		case MPT_RATE_9M:
			TxPower += (((pHalData->MCSTxPowerLevelOriginalOffset[PwrGroup][0+Pathmapping])&0xff00)>>8);
			//RT_DISP(FPHY, PHY_TXPWR,("MCSTxPowerLevelOriginalOffset[%d][0] = 0x%x, OFDM 9M, TxPower = %d\n",
			//	PwrGroup, pHalData->MCSTxPowerLevelOriginalOffset[PwrGroup][0], TxPower));
			break;
		case MPT_RATE_12M:
			TxPower += (((pHalData->MCSTxPowerLevelOriginalOffset[PwrGroup][0+Pathmapping])&0xff0000)>>16);
			//RT_DISP(FPHY, PHY_TXPWR,("MCSTxPowerLevelOriginalOffset[%d][0] = 0x%x, OFDM 12M, TxPower = %d\n",
			//	PwrGroup, pHalData->MCSTxPowerLevelOriginalOffset[PwrGroup][0], TxPower));
			break;
		case MPT_RATE_18M:
			TxPower += (((pHalData->MCSTxPowerLevelOriginalOffset[PwrGroup][0+Pathmapping])&0xff000000)>>24);
			//RT_DISP(FPHY, PHY_TXPWR,("MCSTxPowerLevelOriginalOffset[%d][0] = 0x%x, OFDM 24M, TxPower = %d\n",
			//	PwrGroup, pHalData->MCSTxPowerLevelOriginalOffset[PwrGroup][0], TxPower));
			break;
		case MPT_RATE_24M:	//0xe04[31:0] = 54M,48M,36M,24M
			TxPower += ((pHalData->MCSTxPowerLevelOriginalOffset[PwrGroup][1+Pathmapping])&0xff);
			//RT_DISP(FPHY, PHY_TXPWR,("MCSTxPowerLevelOriginalOffset[%d][1] = 0x%x, OFDM 24M, TxPower = %d\n",
			//	PwrGroup, pHalData->MCSTxPowerLevelOriginalOffset[PwrGroup][1], TxPower));
			break;
		case MPT_RATE_36M:
			TxPower += (((pHalData->MCSTxPowerLevelOriginalOffset[PwrGroup][1+Pathmapping])&0xff00)>>8);
			//RT_DISP(FPHY, PHY_TXPWR,("MCSTxPowerLevelOriginalOffset[%d][1] = 0x%x, OFDM 36M, TxPower = %d\n",
			//	PwrGroup, pHalData->MCSTxPowerLevelOriginalOffset[PwrGroup][1], TxPower));
			break;
		case MPT_RATE_48M:
			TxPower += (((pHalData->MCSTxPowerLevelOriginalOffset[PwrGroup][1+Pathmapping])&0xff0000)>>16);
			//RT_DISP(FPHY, PHY_TXPWR,("MCSTxPowerLevelOriginalOffset[%d][1] = 0x%x, OFDM 48M, TxPower = %d\n",
			//	PwrGroup, pHalData->MCSTxPowerLevelOriginalOffset[PwrGroup][1], TxPower));
			break;
		case MPT_RATE_54M:
			TxPower += (((pHalData->MCSTxPowerLevelOriginalOffset[PwrGroup][1+Pathmapping])&0xff000000)>>24);
			//RT_DISP(FPHY, PHY_TXPWR,("MCSTxPowerLevelOriginalOffset[%d][1] = 0x%x, OFDM 54M, TxPower = %d\n",
			//	PwrGroup, pHalData->MCSTxPowerLevelOriginalOffset[PwrGroup][1], TxPower));
			break;
		case MPT_RATE_MCS0: //0xe10[31:0]= MCS=03,02,01,00
			TxPower += ((pHalData->MCSTxPowerLevelOriginalOffset[PwrGroup][2+Pathmapping])&0xff);
			//RT_DISP(FPHY, PHY_TXPWR,("MCSTxPowerLevelOriginalOffset[%d][2] = 0x%x, MCS0, TxPower = %d\n",
			//	PwrGroup, pHalData->MCSTxPowerLevelOriginalOffset[PwrGroup][2], TxPower));
			break;
		case MPT_RATE_MCS1:
			TxPower += (((pHalData->MCSTxPowerLevelOriginalOffset[PwrGroup][2+Pathmapping])&0xff00)>>8);
			//RT_DISP(FPHY, PHY_TXPWR,("MCSTxPowerLevelOriginalOffset[%d][2] = 0x%x, MCS1, TxPower = %d\n",
			//	PwrGroup, pHalData->MCSTxPowerLevelOriginalOffset[PwrGroup][2], TxPower));
			break;
		case MPT_RATE_MCS2:
			TxPower += (((pHalData->MCSTxPowerLevelOriginalOffset[PwrGroup][2+Pathmapping])&0xff0000)>>16);
			//RT_DISP(FPHY, PHY_TXPWR,("MCSTxPowerLevelOriginalOffset[%d][2] = 0x%x, MCS2, TxPower = %d\n",
			//	PwrGroup, pHalData->MCSTxPowerLevelOriginalOffset[PwrGroup][2], TxPower));
			break;
		case MPT_RATE_MCS3:
			TxPower += (((pHalData->MCSTxPowerLevelOriginalOffset[PwrGroup][2+Pathmapping])&0xff000000)>>24);
			//RT_DISP(FPHY, PHY_TXPWR,("MCSTxPowerLevelOriginalOffset[%d][2] = 0x%x, MCS3, TxPower = %d\n",
			//	PwrGroup, pHalData->MCSTxPowerLevelOriginalOffset[PwrGroup][2], TxPower));
			break;
		case MPT_RATE_MCS4: //0xe14[31:0]= MCS=07,06,05,04
			TxPower += ((pHalData->MCSTxPowerLevelOriginalOffset[PwrGroup][3+Pathmapping])&0xff);
			//RT_DISP(FPHY, PHY_TXPWR,("MCSTxPowerLevelOriginalOffset[%d][3] = 0x%x, MCS4, TxPower = %d\n",
			//	PwrGroup, pHalData->MCSTxPowerLevelOriginalOffset[PwrGroup][3], TxPower));
			break;
		case MPT_RATE_MCS5:
			TxPower += (((pHalData->MCSTxPowerLevelOriginalOffset[PwrGroup][3+Pathmapping])&0xff00)>>8);
			//RT_DISP(FPHY, PHY_TXPWR,("MCSTxPowerLevelOriginalOffset[%d][3] = 0x%x, MCS5, TxPower = %d\n",
			//	PwrGroup, pHalData->MCSTxPowerLevelOriginalOffset[PwrGroup][3], TxPower));
			break;
		case MPT_RATE_MCS6:
			TxPower += (((pHalData->MCSTxPowerLevelOriginalOffset[PwrGroup][3+Pathmapping])&0xff0000)>>16);
			//RT_DISP(FPHY, PHY_TXPWR,("MCSTxPowerLevelOriginalOffset[%d][3] = 0x%x, MCS6, TxPower = %d\n",
			//	PwrGroup, pHalData->MCSTxPowerLevelOriginalOffset[PwrGroup][3], TxPower));
			break;
		case MPT_RATE_MCS7:
			TxPower += (((pHalData->MCSTxPowerLevelOriginalOffset[PwrGroup][3+Pathmapping])&0xff000000)>>24);
			//RT_DISP(FPHY, PHY_TXPWR,("MCSTxPowerLevelOriginalOffset[%d][3] = 0x%x, MCS7, TxPower = %d\n",
			//	PwrGroup, pHalData->MCSTxPowerLevelOriginalOffset[PwrGroup][3], TxPower));
			break;

		case MPT_RATE_MCS8: //0xe18[31:0]= MCS=11,10,09,08
			TxPower += ((pHalData->MCSTxPowerLevelOriginalOffset[PwrGroup][4+Pathmapping])&0xff);
			//RT_DISP(FPHY, PHY_TXPWR,("MCSTxPowerLevelOriginalOffset[%d][4] = 0x%x, MCS8, TxPower = %d\n",
			//	PwrGroup, pHalData->MCSTxPowerLevelOriginalOffset[PwrGroup][4], TxPower));
			break;
		case MPT_RATE_MCS9:
			TxPower += (((pHalData->MCSTxPowerLevelOriginalOffset[PwrGroup][4+Pathmapping])&0xff00)>>8);
			//RT_DISP(FPHY, PHY_TXPWR,("MCSTxPowerLevelOriginalOffset[%d][4] = 0x%x, MCS9, TxPower = %d\n",
			//	PwrGroup, pHalData->MCSTxPowerLevelOriginalOffset[PwrGroup][4], TxPower));
			break;
		case MPT_RATE_MCS10:
			TxPower += (((pHalData->MCSTxPowerLevelOriginalOffset[PwrGroup][4+Pathmapping])&0xff0000)>>16);
			//RT_DISP(FPHY, PHY_TXPWR,("MCSTxPowerLevelOriginalOffset[%d][4] = 0x%x, MCS10, TxPower = %d\n",
			//	PwrGroup, pHalData->MCSTxPowerLevelOriginalOffset[PwrGroup][4], TxPower));
			break;
		case MPT_RATE_MCS11:
			TxPower += (((pHalData->MCSTxPowerLevelOriginalOffset[PwrGroup][4+Pathmapping])&0xff000000)>>24);
			//RT_DISP(FPHY, PHY_TXPWR,("MCSTxPowerLevelOriginalOffset[%d][4] = 0x%x, MCS11, TxPower = %d\n",
			//	PwrGroup, pHalData->MCSTxPowerLevelOriginalOffset[PwrGroup][4], TxPower));
			break;
		case MPT_RATE_MCS12:	//0xe1c[31:0]= MCS=15,14,13,12
			TxPower += ((pHalData->MCSTxPowerLevelOriginalOffset[PwrGroup][5+Pathmapping])&0xff);
			//RT_DISP(FPHY, PHY_TXPWR,("MCSTxPowerLevelOriginalOffset[%d][5] = 0x%x, MCS12, TxPower = %d\n",
			//	PwrGroup, pHalData->MCSTxPowerLevelOriginalOffset[PwrGroup][5], TxPower));
			break;
		case MPT_RATE_MCS13:
			TxPower += (((pHalData->MCSTxPowerLevelOriginalOffset[PwrGroup][5+Pathmapping])&0xff00)>>8);
			//RT_DISP(FPHY, PHY_TXPWR,("MCSTxPowerLevelOriginalOffset[%d][5] = 0x%x, MCS13, TxPower = %d\n",
			//	PwrGroup, pHalData->MCSTxPowerLevelOriginalOffset[PwrGroup][5], TxPower));
			break;
		case MPT_RATE_MCS14:
			TxPower += (((pHalData->MCSTxPowerLevelOriginalOffset[PwrGroup][5+Pathmapping])&0xff0000)>>16);
			//RT_DISP(FPHY, PHY_TXPWR,("MCSTxPowerLevelOriginalOffset[%d][5] = 0x%x, MCS14, TxPower = %d\n",
			//	PwrGroup, pHalData->MCSTxPowerLevelOriginalOffset[PwrGroup][5], TxPower));
			break;
		case MPT_RATE_MCS15:
			TxPower += (((pHalData->MCSTxPowerLevelOriginalOffset[PwrGroup][5+Pathmapping])&0xff000000)>>24);
			//RT_DISP(FPHY, PHY_TXPWR,("MCSTxPowerLevelOriginalOffset[%d][5] = 0x%x, MCS15, TxPower = %d\n",
			//	PwrGroup, pHalData->MCSTxPowerLevelOriginalOffset[PwrGroup][5], TxPower));
			break;
		default:
			break;
		}
	}

	if(TxPower > Limit)
		TxPower = Limit;

	return TxPower;
}



static	ULONG
mpt_ProQueryCalTxPower_8188E(
	IN	struct _ADAPTER *pAdapter,
	IN	u1Byte			RfPath
	)
{
	HAL_DATA_TYPE		*pHalData = GET_HAL_DATA(pAdapter);
	u1Byte				TxCount=TX_1S, i = 0;	//default set to 1S
	//PMGNT_INFO			pMgntInfo = &(pAdapter->MgntInfo);
	ULONG				TxPower = 1, PwrGroup=0, PowerDiffByRate=0;
	ULONG				TxPowerCCK = 1, TxPowerOFDM = 1, TxPowerBW20 = 1, TxPowerBW40 = 1 ;
	PMPT_CONTEXT		pMptCtx = &(pAdapter->mppriv.MptCtx);
	u1Byte				CurrChannel = pHalData->CurrentChannel;
	u1Byte				index = (CurrChannel -1);
	u1Byte				rf_path=(RfPath), rfPath;
	u1Byte				limit = 0, rate = 0;

	if (HAL_IsLegalChannel(pAdapter, CurrChannel) == FALSE) {
		CurrChannel = 1;
	}

	if (pMptCtx->MptRateIndex >= MPT_RATE_1M
	   && pMptCtx->MptRateIndex <= MPT_RATE_11M) {
		TxPower = pHalData->Index24G_CCK_Base[rf_path][index];
	} else
		if (pMptCtx->MptRateIndex >= MPT_RATE_6M
		   && pMptCtx->MptRateIndex <= MPT_RATE_54M) {
			TxPower = pHalData->Index24G_BW40_Base[rf_path][index];
		} else
			if (pMptCtx->MptRateIndex >= MPT_RATE_MCS0
			   && pMptCtx->MptRateIndex <= MPT_RATE_MCS7) {
				TxPower = pHalData->Index24G_BW40_Base[rf_path][index];
			}

	//RT_DISP(FPHY, PHY_TXPWR, ("HT40 rate(%d) Tx power(RF-%c) = 0x%x\n", pMptCtx->MptRateIndex, ((rf_path==0)?'A':'B'), TxPower));


	if(pMptCtx->MptRateIndex >= MPT_RATE_6M &&
		pMptCtx->MptRateIndex <= MPT_RATE_54M )
	{
		TxPower += pHalData->OFDM_24G_Diff[rf_path][TxCount];
		///RT_DISP(FPHY, PHY_TXPWR, ("+OFDM_PowerDiff(RF-%c) = 0x%x\n", ((rf_path==0)?'A':'B'),
		//	pHalData->OFDM_24G_Diff[rf_path][TxCount]));
	}

	if(pMptCtx->MptRateIndex >= MPT_RATE_MCS0)
	{
		if (pHalData->CurrentChannelBW == CHANNEL_WIDTH_20)
		{
			TxPower += pHalData->BW20_24G_Diff[rf_path][TxCount];
		//	RT_DISP(FPHY, PHY_TXPWR, ("+HT20_PowerDiff(RF-%c) = 0x%x\n", ((rf_path==0)?'A':'B'),
		//		pHalData->BW20_24G_Diff[rf_path][TxCount]));
		}
	}


#ifdef ENABLE_POWER_BY_RATE
	PowerDiffByRate = getPowerDiffByRate8188E(pAdapter, CurrChannel, RfPath);
#else
	PowerDiffByRate = 0;
#endif


	//RT_DISP(FPHY, PHY_TXPWR, ("+PowerDiffByRate(RF-%c) = 0x%x\n", ((rf_path==0)?'A':'B'),
	//	PowerDiffByRate));
	TxPower += PowerDiffByRate;
//	RT_DISP(FPHY, PHY_TXPWR, ("Final TxPower(RF-%c) = %d(0x%x)\n", ((rf_path==0)?'A':'B'),
	//	TxPower, TxPower));

	/*
	if(TxPower > 0x3f)
		TxPower = 0x3f;
	*/

	// 2012/11/02 Awk: add power limit mechansim
	if( pMptCtx->MptRateIndex >= MPT_RATE_1M &&
		pMptCtx->MptRateIndex <= MPT_RATE_11M )
	{
		rate = MGN_1M;
	}
	else if(pMptCtx->MptRateIndex >= MPT_RATE_6M &&
		pMptCtx->MptRateIndex <= MPT_RATE_54M )
	{
		rate = MGN_54M;
	}
	else if(pMptCtx->MptRateIndex >= MPT_RATE_MCS0 &&
		pMptCtx->MptRateIndex <= MPT_RATE_MCS7 )
	{
		rate = MGN_MCS7;
	}
	#ifdef CONFIG_8192E
	limit = PHY_GetPowerLimitValue(pAdapter, pMptCtx->RegTxPwrLimit,
								   pHalData->CurrentBandType,
								   pHalData->CurrentChannelBW,RfPath,
								   rate, CurrChannel);
	#endif
	TxPower = TxPower > limit ? limit : TxPower;

	return TxPower;
}


ULONG mpt_ProQueryCalTxPower(
	struct _ADAPTER *pAdapter,
		uint8_t		RfPath
	)
{

	HAL_DATA_TYPE	*pHalData	= GET_HAL_DATA(pAdapter);
	ULONG			TxPower = 1, PwrGroup=0, PowerDiffByRate=0;
	PMPT_CONTEXT	pMptCtx = &(pAdapter->mppriv.MptCtx);
	u1Byte			limit = 0, rate = 0;
	rate=pMptCtx->MptRateIndex;

#ifdef CONFIG_8812A
	TxPower = PHY_GetTxPowerIndex_8812A(pAdapter, RfPath, rate,pHalData->CurrentChannelBW, pHalData->CurrentChannel);
#endif
	return TxPower;
}


void Hal_ProSetCrystalCap (struct _ADAPTER *pAdapter , uint32_t	 CrystalCapVal)
{
	HAL_DATA_TYPE		*pHalData	= GET_HAL_DATA(pAdapter);

		CrystalCapVal = pHalData->CrystalCap & 0x3F;

	if(IS_HARDWARE_TYPE_8812(pAdapter))
	{
		// write 0x2C[30:25] = 0x2C[24:19] = CrystalCap
		PHY_SetBBReg(pAdapter, REG_MAC_PHY_CTRL, 0x7FF80000, (CrystalCapVal | (CrystalCapVal << 6)));
	}
	else if(IS_HARDWARE_TYPE_8821(pAdapter))
	{
		// write 0x2C[23:18] = 0x2C[17:12] = CrystalCap
		PHY_SetBBReg(pAdapter, REG_MAC_PHY_CTRL, 0xFFF000, (CrystalCapVal | (CrystalCapVal << 6)));
	}
	else
	{
		PHY_SetBBReg(pAdapter, 0x2c, 0xFFF000, (CrystalCapVal | (CrystalCapVal << 6)));
	}
}



#endif

