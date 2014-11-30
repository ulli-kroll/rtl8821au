#include <rtl8812a_hal.h>
#include "trx.h"

void rtl8821au_fill_fake_txdesc(struct rtl_priv *padapter, uint8_t *pDesc,
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
