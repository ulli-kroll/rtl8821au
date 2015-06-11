#ifndef	__RTL8821AU_DM_H__
#define __RTL8821AU_DM_H__

#include <odm_types.h>
#include <odm.h>
#include <HalPhyRf.h>


//3===========================================================
//3 Dynamic Tx Power
//3===========================================================
//Dynamic Tx Power Control Threshold
#define		TX_POWER_NEAR_FIELD_THRESH_LVL2	74
#define		TX_POWER_NEAR_FIELD_THRESH_LVL1	67
#define		TX_POWER_NEAR_FIELD_THRESH_AP		0x3F

#define		TxHighPwrLevel_Normal		0
#define		TxHighPwrLevel_Level1		1
#define		TxHighPwrLevel_Level2		2
#define		TxHighPwrLevel_BT1			3
#define		TxHighPwrLevel_BT2			4
#define		TxHighPwrLevel_15			5
#define		TxHighPwrLevel_35			6
#define		TxHighPwrLevel_50			7
#define		TxHighPwrLevel_70			8
#define		TxHighPwrLevel_100			9



#define CCK_TABLE_SIZE				33

extern	u8 CCKSwingTable_Ch1_Ch13[CCK_TABLE_SIZE][8];
extern	u8 CCKSwingTable_Ch14 [CCK_TABLE_SIZE][8];
extern	u8 CCKSwingTable_Ch1_Ch13_New[CCK_TABLE_SIZE][8];
extern	u8 CCKSwingTable_Ch14_New [CCK_TABLE_SIZE][8];

void rtl8812au_get_delta_swing_table(struct rtl_priv *rtlpriv,
					    u8 **up_a, u8 **down_a,
					    u8 **up_b, u8 **down_b);
void rtl8821au_get_delta_swing_table(struct rtl_priv *rtlpriv,
					    u8 **up_a, u8 **down_a,
					    u8 **up_b, u8 **down_b);
void rtl8821au_check_tx_power_tracking_thermalmeter(struct _rtw_dm *pDM_Odm);
void rtl8821au_dm_watchdog(struct rtl_priv *rtlpriv);
void rtl8821au_dm_clean_txpower_tracking_state(struct rtl_priv *rtlpriv);

/* Old prototypes */

void odm_FalseAlarmCounterStatistics(struct _rtw_dm *pDM_Odm);
void odm_Adaptivity(struct _rtw_dm *pDM_Odm, u8 IGI);
void odm_RefreshRateAdaptiveMask(struct _rtw_dm *pDM_Odm);
void odm_CCKPacketDetectionThresh(struct rtl_priv *rtlpriv);


#endif
