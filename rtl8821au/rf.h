#ifndef __RTL8821AU_PR_H__
#define __RTL8821AU_PR_H__

void rtl8821au_phy_rf6052_set_bandwidth(struct rtl_priv *rtlpriv, enum CHANNEL_WIDTH	Bandwidth);
int rtl8821au_phy_rf6052_config(struct rtl_priv *rtlpriv);
void rtl8821au_phy_rf6052_set_cck_txpower(struct rtl_priv *rtlpriv, uint8_t *pPowerlevel);

#endif
