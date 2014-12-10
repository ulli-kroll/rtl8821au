#ifndef	__RTL8821AU_DM_H__
#define __RTL8821AU_DM_H__

#include "../hal/OUTSRC/odm_types.h"
#include "../hal/OUTSRC/odm.h"
#include "../hal/OUTSRC/HalPhyRf.h"

#define CCK_TABLE_SIZE				33

extern	u8 CCKSwingTable_Ch1_Ch13[CCK_TABLE_SIZE][8];
extern	u8 CCKSwingTable_Ch14 [CCK_TABLE_SIZE][8];
extern	u8 CCKSwingTable_Ch1_Ch13_New[CCK_TABLE_SIZE][8];
extern	u8 CCKSwingTable_Ch14_New [CCK_TABLE_SIZE][8];

void PHY_LCCalibrate_8821A(struct rtl_dm *pDM_Odm);
void rtl8812au_get_delta_swing_table(struct rtl_dm *pDM_Odm,
					    u8 **up_a, u8 **down_a,
					    u8 **up_b, u8 **down_b);
void rtl8821au_get_delta_swing_table(struct rtl_dm *pDM_Odm,
	u8 **up_a, u8 **down_a,
	u8 **up_b, u8 **down_b);
void DoIQK_8812A(struct rtl_dm *pDM_Odm, u8 DeltaThermalIndex,
	u8 	ThermalValue, u8 Threshold);
void DoIQK_8821A(struct rtl_dm *pDM_Odm, u8 DeltaThermalIndex,
	u8 ThermalValue, u8 Threshold);
void ODM_TxPwrTrackSetPwr8812A(struct rtl_dm *pDM_Odm, PWRTRACK_METHOD Method,
	u8 	RFPath, u8 	ChannelMappedIndex);
void ODM_TxPwrTrackSetPwr8821A(struct rtl_dm *pDM_Odm, PWRTRACK_METHOD Method,
	u8 RFPath, u8 ChannelMappedIndex);



#endif
