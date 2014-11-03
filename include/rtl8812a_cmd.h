/******************************************************************************
 *
 * Copyright(c) 2007 - 2011 Realtek Corporation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110, USA
 *
 *
 ******************************************************************************/
#ifndef __RTL8812A_CMD_H__
#define __RTL8812A_CMD_H__

typedef enum _RTL8812_H2C_CMD
{
	H2C_8812_RSVDPAGE = 0,
	H2C_8812_MSRRPT = 1,
	H2C_8812_SCAN = 2,
	H2C_8812_KEEP_ALIVE_CTRL = 3,
	H2C_8812_DISCONNECT_DECISION = 4,

	H2C_8812_INIT_OFFLOAD = 6,
	H2C_8812_AP_OFFLOAD = 8,
	H2C_8812_BCN_RSVDPAGE = 9,
	H2C_8812_PROBERSP_RSVDPAGE = 10,

	H2C_8812_SETPWRMODE = 0x20,
	H2C_8812_PS_TUNING_PARA = 0x21,
	H2C_8812_PS_TUNING_PARA2 = 0x22,
	H2C_8812_PS_LPS_PARA = 0x23,
	H2C_8812_P2P_PS_OFFLOAD = 0x24,
	H2C_8812_RA_MASK = 0x40,
	H2C_8812_RSSI_REPORT = 0x42,

	H2C_8812_WO_WLAN = 0x80,
	H2C_8812_REMOTE_WAKE_CTRL = 0x81,
	H2C_8812_AOAC_GLOBAL_INFO = 0x82,
	H2C_8812_AOAC_RSVDPAGE = 0x83,

	H2C_8812_TSF_RESET = 0xC0,

	MAX_8812_H2CCMD
}RTL8812_H2C_CMD;


typedef enum _RTL8812_C2H_EVT
{
	C2H_8812_DBG = 0,
	C2H_8812_LB = 1,
	C2H_8812_TXBF = 2,
	C2H_8812_TX_REPORT = 3,
	C2H_8812_BT_INFO = 9,
	C2H_8812_BT_MP = 11,
	C2H_8812_RA_RPT=12,

	C2H_8812_FW_SWCHNL = 0x10,
	C2H_8812_IQK_FINISH = 0x11,
	MAX_8812_C2HEVENT
}RTL8812_C2H_EVT;


struct cmd_msg_parm {
	uint8_t eid; //element id
	uint8_t sz; // sz
	uint8_t buf[6];
};

enum{
	PWRS
};

struct H2C_SS_RFOFF_PARAM{
	uint8_t ROFOn; // 1: on, 0:off
	u16 gpio_period; // unit: 1024 us
}__attribute__ ((packed));



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


void	Set_RA_LDPC_8812(struct sta_info	*psta, BOOLEAN bLDPC);

// host message to firmware cmd
void rtl8812_set_FwPwrMode_cmd(struct rtl_priv *padapter, uint8_t PSMode);
void rtl8812_set_FwJoinBssReport_cmd(struct rtl_priv *padapter, uint8_t mstatus);
uint8_t rtl8812_set_rssi_cmd(struct rtl_priv *padapter, uint8_t *param);
void rtl8812_set_raid_cmd(struct rtl_priv *padapter, u32 bitmap, u8* arg);
void rtl8812_Add_RateATid(struct rtl_priv *padapter, u32 bitmap, u8* arg, uint8_t rssi_level);

void CheckFwRsvdPageContent(struct rtl_priv *padapter);
void rtl8812_set_FwMediaStatus_cmd(struct rtl_priv *padapter, u16 mstatus_rpt );

#endif//__RTL8188E_CMD_H__


