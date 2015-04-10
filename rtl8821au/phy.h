#ifndef __RTL8821AU_PHY_H__
#define __RTL8821AU_PHY_H__

u32 rtl8821au_phy_query_bb_reg(struct rtl_priv *rtlpriv, u32 RegAddr, u32 BitMask);
void rtl8821au_phy_set_bb_reg(struct rtl_priv *	rtlpriv, u32 RegAddr, u32 BitMask, u32 Data);

void rtl8821au_phy_iq_calibrate(struct rtl_priv *rtlpriv, BOOLEAN bReCovery);


void _rtl8821au_phy_config_mac_with_headerfile(struct _rtw_dm *pDM_Odm);
void _rtl8821au_phy_read_and_config_txpwr_lmt(struct _rtw_dm *pDM_Odm);

void rtl8821au_phy_config_rf_with_headerfile(struct rtl_priv *rtlpriv, enum radio_path eRFPath);
void rtl8812au_phy_config_rf_with_headerfile(struct rtl_priv *rtlpriv, enum radio_path eRFPath);
void rtl8821au_phy_switch_wirelessband(struct rtl_priv *rtlpriv, u8 Band);

uint32_t phy_get_tx_swing_8821au(struct rtl_priv *rtlpriv, BAND_TYPE Band,
	uint8_t	RFPath);
	
	
/* Not in rtlwifi */

BOOLEAN phy_SwBand8812(struct rtl_priv *rtlpriv, uint8_t channelToSW);

#endif
