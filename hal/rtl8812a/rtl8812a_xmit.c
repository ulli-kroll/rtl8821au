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
#define _RTL8812A_XMIT_C_

//#include <drv_types.h>
#include <rtl8812a_hal.h>

void _dbg_dump_tx_info(struct rtl_priv	*padapter,int frame_tag, uint8_t *ptxdesc)
{
	uint8_t bDumpTxPkt;
	uint8_t bDumpTxDesc = _FALSE;
	rtw_hal_get_def_var(padapter, HAL_DEF_DBG_DUMP_TXPKT, &(bDumpTxPkt));

	if(bDumpTxPkt ==1){		/* dump txdesc for data frame */
		DBG_871X("dump tx_desc for data frame\n");
		if((frame_tag&0x0f) == DATA_FRAMETAG){
			bDumpTxDesc = _TRUE;
		}
	} else if(bDumpTxPkt ==2){	/* dump txdesc for mgnt frame */
		DBG_871X("dump tx_desc for mgnt frame\n");
		if((frame_tag&0x0f) == MGNT_FRAMETAG){
			bDumpTxDesc = _TRUE;
		}
	}
	else if(bDumpTxPkt ==3){	/* dump early info */
	}

	if(bDumpTxDesc){
		/*
		 * ptxdesc->txdw4 = cpu_to_le32(0x00001006);//RTS Rate=24M
		 * ptxdesc->txdw6 = 0x6666f800;
		 */
		DBG_8192C("=====================================\n");
		DBG_8192C("Offset00(0x%08x)\n",*((uint32_t *)(ptxdesc)));
		DBG_8192C("Offset04(0x%08x)\n",*((uint32_t *)(ptxdesc+4)));
		DBG_8192C("Offset08(0x%08x)\n",*((uint32_t *)(ptxdesc+8)));
		DBG_8192C("Offset12(0x%08x)\n",*((uint32_t *)(ptxdesc+12)));
		DBG_8192C("Offset16(0x%08x)\n",*((uint32_t *)(ptxdesc+16)));
		DBG_8192C("Offset20(0x%08x)\n",*((uint32_t *)(ptxdesc+20)));
		DBG_8192C("Offset24(0x%08x)\n",*((uint32_t *)(ptxdesc+24)));
		DBG_8192C("Offset28(0x%08x)\n",*((uint32_t *)(ptxdesc+28)));
		DBG_8192C("Offset32(0x%08x)\n",*((uint32_t *)(ptxdesc+32)));
		DBG_8192C("Offset36(0x%08x)\n",*((uint32_t *)(ptxdesc+36)));
		DBG_8192C("=====================================\n");
	}

}

/*
 * Description:
 *	Aggregation packets and send to hardware
 *
 * Return:
 *	0	Success
 *	-1	Hardware resource(TX FIFO) not ready
 *	-2	Software resource(xmitbuf) not ready
 */

void rtl8812a_cal_txdesc_chksum(uint8_t *ptxdesc)
{
	uint16_t *usPtr;
	uint32_t count;
	uint32_t index;
	uint16_t checksum = 0;

	usPtr = (uint16_t *)ptxdesc;
	/*
	 * checksume is always calculated by first 32 bytes,
	 * and it doesn't depend on TX DESC length.
	 * Thomas,Lucas@SD4,20130515
	 */
	count = 16;

	/* Clear first */
	SET_TX_DESC_TX_DESC_CHECKSUM_8812(ptxdesc, 0);

	for (index = 0; index < count; index++) {
		checksum = checksum ^ le16_to_cpu(*(usPtr + index));
	}

	SET_TX_DESC_TX_DESC_CHECKSUM_8812(ptxdesc, checksum);
}

/*
 * Description: In normal chip, we should send some packet to Hw which will be used by Fw
 * in FW LPS mode. The function is to fill the Tx descriptor of this packets, then
 * Fw can tell Hw to send these packet derectly.
 *
*/

void rtl8812a_fill_fake_txdesc(struct rtl_priv *padapter, uint8_t *pDesc,
	uint32_t BufferLen, uint8_t IsPsPoll, uint8_t IsBTQosNull)
{
	struct mlme_ext_priv	*pmlmeext = &padapter->mlmeextpriv;

	/* Clear all status */
	memset(pDesc, 0, TXDESC_SIZE);

	SET_TX_DESC_FIRST_SEG_8812(pDesc, 1);
	SET_TX_DESC_LAST_SEG_8812(pDesc, 1);

	SET_TX_DESC_OFFSET_8812(pDesc, TXDESC_SIZE);

	SET_TX_DESC_PKT_SIZE_8812(pDesc, BufferLen);

	SET_TX_DESC_QUEUE_SEL_8812(pDesc,  QSLT_MGNT);

	if (pmlmeext->cur_wireless_mode & WIRELESS_11B) {
		SET_TX_DESC_RATE_ID_8812(pDesc, RATEID_IDX_B);
	} else {
		SET_TX_DESC_RATE_ID_8812(pDesc, RATEID_IDX_G);
	}

	/*
	 * Set NAVUSEHDR to prevent Ps-poll AId filed to be changed to error vlaue by Hw.
	 */
	if (IsPsPoll) {
		SET_TX_DESC_NAV_USE_HDR_8812(pDesc, 1);
	} else {
		SET_TX_DESC_HWSEQ_EN_8812(pDesc, 1); // Hw set sequence number
	}

	if (IsBTQosNull) {
		SET_TX_DESC_BT_INT_8812(pDesc, 1);
	}

	SET_TX_DESC_USE_RATE_8812(pDesc, 1);
	SET_TX_DESC_OWN_8812(pDesc, 1);

	SET_TX_DESC_TX_RATE_8812(pDesc, MRateToHwRate(pmlmeext->tx_rate));

	// USB interface drop packet if the checksum of descriptor isn't correct.
	// Using this checksum can let hardware recovery from packet bulk out error (e.g. Cancel URC, Bulk out error.).
	rtl8812a_cal_txdesc_chksum(pDesc);
}

void rtl8812a_fill_txdesc_sectype(struct pkt_attrib *pattrib, uint8_t *ptxdesc)
{
	if ((pattrib->encrypt > 0) && !pattrib->bswenc) {
		switch (pattrib->encrypt) {
		/* SEC_TYPE : 0:NO_ENC,1:WEP40/TKIP,2:WAPI,3:AES */
		case _WEP40_:
		case _WEP104_:
		case _TKIP_:
		case _TKIP_WTMIC_:
			SET_TX_DESC_SEC_TYPE_8812(ptxdesc, 0x1);
			break;
		case _AES_:
			SET_TX_DESC_SEC_TYPE_8812(ptxdesc, 0x3);
			break;
		case _NO_PRIVACY_:
		default:
			SET_TX_DESC_SEC_TYPE_8812(ptxdesc, 0x0);
			break;

		}

	}

}

void rtl8812a_fill_txdesc_vcs(struct rtl_priv *padapter, struct pkt_attrib *pattrib, uint8_t *ptxdesc)
{
	struct mlme_ext_priv	*pmlmeext = &padapter->mlmeextpriv;
	struct mlme_ext_info	*pmlmeinfo = &(pmlmeext->mlmext_info);

	//DBG_8192C("vcs_mode=%d\n", pattrib->vcs_mode);

	if (pattrib->vcs_mode) {
		switch(pattrib->vcs_mode) {
		case RTS_CTS:
			SET_TX_DESC_RTS_ENABLE_8812(ptxdesc, 1);
			break;
		case CTS_TO_SELF:
			SET_TX_DESC_CTS2SELF_8812(ptxdesc, 1);
			break;
		case NONE_VCS:
		default:
			break;
		}
		if (pmlmeinfo->preamble_mode == PREAMBLE_SHORT)
			SET_TX_DESC_RTS_SHORT_8812(ptxdesc, 1);

		SET_TX_DESC_RTS_RATE_8812(ptxdesc, 0x8);	/*RTS Rate=24M */

		SET_TX_DESC_RTS_RATE_FB_LIMIT_8812(ptxdesc, 0xf);

		/*
		 * Enable HW RTS
		 * SET_TX_DESC_HW_RTS_ENABLE_8812(ptxdesc, 1);
		 */
	}
}

void rtl8812a_fill_txdesc_phy(struct rtl_priv *padapter, struct pkt_attrib *pattrib, uint8_t *ptxdesc)
{
	/* DBG_8192C("bwmode=%d, ch_off=%d\n", pattrib->bwmode, pattrib->ch_offset); */

	if (pattrib->ht_en) {
		/*  Set Bandwidth and sub-channel settings. */
		SET_TX_DESC_DATA_BW_8812(ptxdesc, BWMapping_8812(padapter,pattrib));
		/* SET_TX_DESC_DATA_SC_8812(ptxdesc, SCMapping_8812(padapter,pattrib)); */
	}
}

u8 BWMapping_8812(struct rtl_priv *Adapter, struct pkt_attrib *pattrib)
{
	uint8_t	BWSettingOfDesc = 0;
	PHAL_DATA_TYPE	pHalData = GET_HAL_DATA(Adapter);

	/*
	 * DBG_871X("BWMapping pHalData->CurrentChannelBW %d, pattrib->bwmode %d \n",pHalData->CurrentChannelBW,pattrib->bwmode);
	 */

	if (pHalData->CurrentChannelBW == CHANNEL_WIDTH_80) {
		if (pattrib->bwmode == CHANNEL_WIDTH_80)
			BWSettingOfDesc= 2;
		else if (pattrib->bwmode == CHANNEL_WIDTH_40)
			BWSettingOfDesc = 1;
		else
			BWSettingOfDesc = 0;
	} else if(pHalData->CurrentChannelBW== CHANNEL_WIDTH_40) {
		if((pattrib->bwmode == CHANNEL_WIDTH_40) || (pattrib->bwmode == CHANNEL_WIDTH_80))
			BWSettingOfDesc = 1;
		else
			BWSettingOfDesc = 0;
	} else
		BWSettingOfDesc = 0;

	return BWSettingOfDesc;
}

u8 SCMapping_8812(struct rtl_priv *Adapter, struct pkt_attrib *pattrib)
{
	uint8_t	SCSettingOfDesc = 0;
	PHAL_DATA_TYPE	pHalData = GET_HAL_DATA(Adapter);

	/*
	 * DBG_871X("SCMapping: pHalData->CurrentChannelBW %d, pHalData->nCur80MhzPrimeSC %d, pHalData->nCur40MhzPrimeSC %d \n",pHalData->CurrentChannelBW,pHalData->nCur80MhzPrimeSC,pHalData->nCur40MhzPrimeSC);
	 */

	if (pHalData->CurrentChannelBW == CHANNEL_WIDTH_80) {
		if(pattrib->bwmode == CHANNEL_WIDTH_80) {
			SCSettingOfDesc = VHT_DATA_SC_DONOT_CARE;
		} else if(pattrib->bwmode == CHANNEL_WIDTH_40) {
			if(pHalData->nCur80MhzPrimeSC == HAL_PRIME_CHNL_OFFSET_LOWER)
				SCSettingOfDesc = VHT_DATA_SC_40_LOWER_OF_80MHZ;
			else if(pHalData->nCur80MhzPrimeSC == HAL_PRIME_CHNL_OFFSET_UPPER)
				SCSettingOfDesc = VHT_DATA_SC_40_UPPER_OF_80MHZ;
			else
				DBG_871X("SCMapping: Not Correct Primary40MHz Setting \n");
		} else {
			if((pHalData->nCur40MhzPrimeSC == HAL_PRIME_CHNL_OFFSET_LOWER) && (pHalData->nCur80MhzPrimeSC == HAL_PRIME_CHNL_OFFSET_LOWER))
				SCSettingOfDesc = VHT_DATA_SC_20_LOWEST_OF_80MHZ;
			else if((pHalData->nCur40MhzPrimeSC == HAL_PRIME_CHNL_OFFSET_UPPER) && (pHalData->nCur80MhzPrimeSC == HAL_PRIME_CHNL_OFFSET_LOWER))
				SCSettingOfDesc = VHT_DATA_SC_20_LOWER_OF_80MHZ;
			else if((pHalData->nCur40MhzPrimeSC == HAL_PRIME_CHNL_OFFSET_LOWER) && (pHalData->nCur80MhzPrimeSC == HAL_PRIME_CHNL_OFFSET_UPPER))
				SCSettingOfDesc = VHT_DATA_SC_20_UPPER_OF_80MHZ;
			else if((pHalData->nCur40MhzPrimeSC == HAL_PRIME_CHNL_OFFSET_UPPER) && (pHalData->nCur80MhzPrimeSC == HAL_PRIME_CHNL_OFFSET_UPPER))
				SCSettingOfDesc = VHT_DATA_SC_20_UPPERST_OF_80MHZ;
			else
				DBG_871X("SCMapping: Not Correct Primary40MHz Setting \n");
		}
	} else if(pHalData->CurrentChannelBW== CHANNEL_WIDTH_40) {
		/*
		 * DBG_871X("SCMapping: HT Case: pHalData->CurrentChannelBW %d, pHalData->nCur40MhzPrimeSC %d \n",pHalData->CurrentChannelBW,pHalData->nCur40MhzPrimeSC);
		 */

		if(pattrib->bwmode == CHANNEL_WIDTH_40) {
			SCSettingOfDesc = VHT_DATA_SC_DONOT_CARE;
		} else if(pattrib->bwmode == CHANNEL_WIDTH_20) {
			if(pHalData->nCur40MhzPrimeSC == HAL_PRIME_CHNL_OFFSET_UPPER) {
				SCSettingOfDesc = VHT_DATA_SC_20_UPPER_OF_80MHZ;
			} else if(pHalData->nCur40MhzPrimeSC == HAL_PRIME_CHNL_OFFSET_LOWER) {
				SCSettingOfDesc = VHT_DATA_SC_20_LOWER_OF_80MHZ;
			} else 		{
				SCSettingOfDesc = VHT_DATA_SC_DONOT_CARE;
			}

		}
	} else {
		SCSettingOfDesc = VHT_DATA_SC_DONOT_CARE;
	}

	return SCSettingOfDesc;
}


