#ifndef __RTL8821AU_PHY_H__
#define __RTL8821AU_PHY_H__

u32 rtl8821au_phy_query_bb_reg(struct rtl_priv *Adapter, u32 RegAddr, u32 BitMask);
void rtl8821au_phy_set_bb_reg(struct rtl_priv *	Adapter, u32 RegAddr, u32 BitMask, u32 Data);

void rtl8821au_phy_iq_calibrate(struct rtl_priv *pAdapter, BOOLEAN bReCovery);
#endif
