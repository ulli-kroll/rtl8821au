#ifndef __RTL8821AU_PHY_H__
#define __RTL8821AU_PHY_H__

u32 rtl8821au_phy_query_bb_reg(struct rtl_priv *Adapter, u32 RegAddr, u32 BitMask);
void rtl8821au_phy_set_bb_reg(struct rtl_priv *	Adapter, u32 RegAddr, u32 BitMask, u32 Data);

void rtl8821au_phy_iq_calibrate(struct rtl_priv *pAdapter, BOOLEAN bReCovery);


HAL_STATUS _rtl8821au_phy_config_mac_with_headerfile(PDM_ODM_T pDM_Odm);
HAL_STATUS _rtl8821au_phy_read_and_config_txpwr_lmt(PDM_ODM_T pDM_Odm);

void ODM_ReadAndConfig_MP_8821A_RadioA(PDM_ODM_T  pDM_Odm);
void ODM_ReadAndConfig_MP_8812A_RadioA(PDM_ODM_T  pDM_Odm);
void ODM_ReadAndConfig_MP_8812A_RadioB(PDM_ODM_T  pDM_Odm);


#endif
