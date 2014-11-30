#ifndef __RTL8821AU_TRX_H__
#define __RTL8821AU_TRX_H__

void rtl8821au_fill_fake_txdesc(struct rtl_priv *padapter, uint8_t *pDesc,
	uint32_t BufferLen, uint8_t IsPsPoll, uint8_t IsBTQosNull);

#endif
