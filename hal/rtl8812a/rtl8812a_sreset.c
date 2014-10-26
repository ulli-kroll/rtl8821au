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
#define _RTL8812A_SRESET_C_

//#include <drv_types.h>
#include <rtl8812a_hal.h>

#ifdef DBG_CONFIG_ERROR_DETECT
void rtl8812_sreset_xmit_status_check(_adapter *padapter)
{
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(padapter);
	struct sreset_priv *psrtpriv = &pHalData->srestpriv;

	unsigned long current_time;
	struct xmit_priv	*pxmitpriv = &padapter->xmitpriv;
	unsigned int diff_time;
	uint32_t txdma_status;

	if( (txdma_status=rtw_read32(padapter, REG_TXDMA_STATUS)) !=0x00){
		DBG_871X("%s REG_TXDMA_STATUS:0x%08x\n", __FUNCTION__, txdma_status);
		rtw_hal_sreset_reset(padapter);
	}
	//total xmit irp = 4
	//DBG_8192C("==>%s free_xmitbuf_cnt(%d),txirp_cnt(%d)\n",__FUNCTION__,pxmitpriv->free_xmitbuf_cnt,pxmitpriv->txirp_cnt);
	//if(pxmitpriv->txirp_cnt == NR_XMITBUFF+1)
	current_time = rtw_get_current_time();

	if (0 == pxmitpriv->free_xmitbuf_cnt || 0 == pxmitpriv->free_xmit_extbuf_cnt) {

		diff_time = rtw_get_passing_time_ms(psrtpriv->last_tx_time);

		if (diff_time > 2000) {
			if (psrtpriv->last_tx_complete_time == 0) {
				psrtpriv->last_tx_complete_time = current_time;
			}
			else{
				diff_time = rtw_get_passing_time_ms(psrtpriv->last_tx_complete_time);
				if (diff_time > 4000) {
					//padapter->Wifi_Error_Status = WIFI_TX_HANG;
					DBG_871X("%s tx hang\n", __FUNCTION__);
					rtw_hal_sreset_reset(padapter);
				}
			}
		}
	}

	if (psrtpriv->dbg_trigger_point == SRESET_TGP_XMIT_STATUS) {
		psrtpriv->dbg_trigger_point = SRESET_TGP_NULL;
		rtw_hal_sreset_reset(padapter);
		return;
	}
}

void rtl8812_sreset_linked_status_check(_adapter *padapter)
{
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(padapter);
	struct sreset_priv *psrtpriv = &pHalData->srestpriv;

	uint32_t rx_dma_status = 0;
	rx_dma_status = rtw_read32(padapter,REG_RXDMA_STATUS);
	if(rx_dma_status!= 0x00){
		DBG_8192C("%s REG_RXDMA_STATUS:0x%08x\n",__FUNCTION__,rx_dma_status);
	}

	if (psrtpriv->dbg_trigger_point == SRESET_TGP_LINK_STATUS) {
		psrtpriv->dbg_trigger_point = SRESET_TGP_NULL;
		rtw_hal_sreset_reset(padapter);
		return;
	}
}
#endif

