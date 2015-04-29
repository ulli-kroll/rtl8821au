#ifndef __RTL8821AU_FW_H__
#define __RTL8821AU_FW_H__

#include <rtl8812a_hal.h>
#include "../wifi.h"


//_RSVDPAGE_LOC_CMD0
#define SET_8812_H2CCMD_RSVDPAGE_LOC_PROBE_RSP(__pH2CCmd, __Value)		SET_BITS_TO_LE_1BYTE(__pH2CCmd, 0, 8, __Value)
#define SET_8812_H2CCMD_RSVDPAGE_LOC_PSPOLL(__pH2CCmd, __Value)			SET_BITS_TO_LE_1BYTE((__pH2CCmd)+1, 0, 8, __Value)
#define SET_8812_H2CCMD_RSVDPAGE_LOC_NULL_DATA(__pH2CCmd, __Value)		SET_BITS_TO_LE_1BYTE((__pH2CCmd)+2, 0, 8, __Value)
#define SET_8812_H2CCMD_RSVDPAGE_LOC_QOS_NULL_DATA(__pH2CCmd, __Value)		SET_BITS_TO_LE_1BYTE((__pH2CCmd)+3, 0, 8, __Value)
#define SET_8812_H2CCMD_RSVDPAGE_LOC_BT_QOS_NULL_DATA(__pH2CCmd, __Value)		SET_BITS_TO_LE_1BYTE((__pH2CCmd)+4, 0, 8, __Value)

//_MEDIA_STATUS_RPT_PARM_CMD1
#define SET_8812_H2CCMD_MSRRPT_PARM_OPMODE(__pH2CCmd, __Value)		SET_BITS_TO_LE_1BYTE(__pH2CCmd, 0, 1, __Value)
#define SET_8812_H2CCMD_MSRRPT_PARM_MACID_IND(__pH2CCmd, __Value)		SET_BITS_TO_LE_1BYTE(__pH2CCmd, 1, 1, __Value)
#define SET_8812_H2CCMD_MSRRPT_PARM_MACID(__pH2CCmd, __Value)			SET_BITS_TO_LE_1BYTE(__pH2CCmd+1, 0, 8, __Value)
#define SET_8812_H2CCMD_MSRRPT_PARM_MACID_END(__pH2CCmd, __Value)			SET_BITS_TO_LE_1BYTE(__pH2CCmd+2, 0, 8, __Value)

//_SETPWRMODE_PARM
#define SET_8812_H2CCMD_PWRMODE_PARM_MODE(__pH2CCmd, __Value)			SET_BITS_TO_LE_1BYTE(__pH2CCmd, 0, 8, __Value)
#define SET_8812_H2CCMD_PWRMODE_PARM_RLBM(__pH2CCmd, __Value)			SET_BITS_TO_LE_1BYTE((__pH2CCmd)+1, 0, 4, __Value)
#define SET_8812_H2CCMD_PWRMODE_PARM_SMART_PS(__pH2CCmd, __Value)		SET_BITS_TO_LE_1BYTE((__pH2CCmd)+1, 4, 4, __Value)
#define SET_8812_H2CCMD_PWRMODE_PARM_BCN_PASS_TIME(__pH2CCmd, __Value)	SET_BITS_TO_LE_1BYTE((__pH2CCmd)+2, 0, 8, __Value)
#define SET_8812_H2CCMD_PWRMODE_PARM_ALL_QUEUE_UAPSD(__pH2CCmd, __Value)	SET_BITS_TO_LE_1BYTE((__pH2CCmd)+3, 0, 8, __Value)
#define SET_8812_H2CCMD_PWRMODE_PARM_PWR_STATE(__pH2CCmd, __Value)	SET_BITS_TO_LE_1BYTE((__pH2CCmd)+4, 0, 8, __Value)

#define GET_8812_H2CCMD_PWRMODE_PARM_MODE(__pH2CCmd)					LE_BITS_TO_1BYTE(__pH2CCmd, 0, 8)

//_P2P_PS_OFFLOAD
#define SET_8812_H2CCMD_P2P_PS_OFFLOAD_ENABLE(__pH2CCmd, __Value)			SET_BITS_TO_LE_1BYTE(__pH2CCmd, 0, 1, __Value)
#define SET_8812_H2CCMD_P2P_PS_OFFLOAD_ROLE(__pH2CCmd, __Value)			SET_BITS_TO_LE_1BYTE(__pH2CCmd, 1, 1, __Value)
#define SET_8812_H2CCMD_P2P_PS_OFFLOAD_CTWINDOW_EN(__pH2CCmd, __Value)		SET_BITS_TO_LE_1BYTE(__pH2CCmd, 2, 1, __Value)
#define SET_8812_H2CCMD_P2P_PS_OFFLOAD_NOA0_EN(__pH2CCmd, __Value)	SET_BITS_TO_LE_1BYTE(__pH2CCmd, 3, 1, __Value)
#define SET_8812_H2CCMD_P2P_PS_OFFLOAD_NOA1_EN(__pH2CCmd, __Value)	SET_BITS_TO_LE_1BYTE(__pH2CCmd, 4, 1, __Value)
#define SET_8812_H2CCMD_P2P_PS_OFFLOAD_ALLSTASLEEP(__pH2CCmd, __Value)	SET_BITS_TO_LE_1BYTE(__pH2CCmd, 5, 1, __Value)
#define SET_8812_H2CCMD_P2P_PS_OFFLOAD_DISCOVERY(__pH2CCmd, __Value)	SET_BITS_TO_LE_1BYTE(__pH2CCmd, 6, 1, __Value)


void rtl8812au_set_fw_pwrmode_cmd(struct rtl_priv *rtlpriv, uint8_t PSMode);
int32_t rtl8821au_download_fw(struct rtl_priv *rtlpriv, BOOLEAN bUsedWoWLANFw);

#endif
