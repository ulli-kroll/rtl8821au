#ifndef __RTL8821AU_PHY_H__
#define __RTL8821AU_PHY_H__

u32 rtl8821au_phy_query_bb_reg(struct rtl_priv *rtlpriv, u32 RegAddr, u32 BitMask);
void rtl8821au_phy_set_bb_reg(struct rtl_priv *	rtlpriv, u32 RegAddr, u32 BitMask, u32 Data);
u32 rtl8821au_phy_query_rf_reg(struct rtl_priv *rtlpriv, u32 eRFPath, u32 RegAddr, u32 BitMask);
void rtl8821au_phy_set_rf_reg(struct rtl_priv *rtlpriv, u32 eRFPath, u32 RegAddr, 
	u32 BitMask, u32 Data);

void rtl8821au_phy_iq_calibrate(struct rtl_priv *rtlpriv, BOOLEAN bReCovery);


void _rtl8821au_phy_config_mac_with_headerfile(struct rtl_priv *rtlpriv);
void _rtl8821au_phy_read_and_config_txpwr_lmt(struct rtl_priv *rtlpriv);

void rtl8821au_phy_config_rf_with_headerfile(struct rtl_priv *rtlpriv, enum radio_path eRFPath);
void rtl8812au_phy_config_rf_with_headerfile(struct rtl_priv *rtlpriv, enum radio_path eRFPath);
void rtl8821au_phy_switch_wirelessband(struct rtl_priv *rtlpriv, u8 Band);

uint32_t phy_get_tx_swing_8821au(struct rtl_priv *rtlpriv, enum band_type Band,
	uint8_t	RFPath);
	
enum _ANT_DIV_TYPE {
	NO_ANTDIV		= 0xFF,
	CG_TRX_HW_ANTDIV	= 0x01,
	CGCS_RX_HW_ANTDIV 	= 0x02,
	FIXED_HW_ANTDIV		= 0x03,
	CG_TRX_SMART_ANTDIV	= 0x04,
	CGCS_RX_SW_ANTDIV	= 0x05,
	S0S1_HW_ANTDIV          = 0x06, //8723B intrnal switch S0 S1
};



	
/* Not in rtlwifi */

BOOLEAN phy_SwBand8812(struct rtl_priv *rtlpriv, uint8_t channelToSW);

#endif
