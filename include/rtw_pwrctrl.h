/******************************************************************************
 *
 * Copyright(c) 2007 - 2012 Realtek Corporation. All rights reserved.
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
#ifndef __RTW_PWRCTRL_H_
#define __RTW_PWRCTRL_H_


#define FW_PWR0	0
#define FW_PWR1 	1
#define FW_PWR2 	2
#define FW_PWR3 	3


#define HW_PWR0	7
#define HW_PWR1 	6
#define HW_PWR2 	2
#define HW_PWR3	0
#define HW_PWR4	8

#define FW_PWRMSK	0x7


#define XMIT_ALIVE	BIT(0)
#define RECV_ALIVE	BIT(1)
#define CMD_ALIVE	BIT(2)
#define EVT_ALIVE	BIT(3)


enum Power_Mgnt
{
	PS_MODE_ACTIVE	= 0	,
	PS_MODE_MIN			,
	PS_MODE_MAX			,
	PS_MODE_DTIM			,
	PS_MODE_VOIP			,
	PS_MODE_UAPSD_WMM	,
	PS_MODE_UAPSD			,
	PS_MODE_IBSS			,
	PS_MODE_WWLAN		,
	PM_Radio_Off			,
	PM_Card_Disable		,
	PS_MODE_NUM,
};


/*
	BIT[2:0] = HW state
	BIT[3] = Protocol PS state,   0: register active state , 1: register sleep state
	BIT[4] = sub-state
*/

#define PS_DPS				BIT(0)
#define PS_LCLK				(PS_DPS)
#define PS_RF_OFF			BIT(1)
#define PS_ALL_ON			BIT(2)
#define PS_ST_ACTIVE		BIT(3)

#define PS_ISR_ENABLE		BIT(4)
#define PS_IMR_ENABLE		BIT(5)
#define PS_ACK				BIT(6)
#define PS_TOGGLE			BIT(7)

#define PS_STATE_MASK		(0x0F)
#define PS_STATE_HW_MASK	(0x07)
#define PS_SEQ_MASK			(0xc0)

#define PS_STATE(x)		(PS_STATE_MASK & (x))
#define PS_STATE_HW(x)	(PS_STATE_HW_MASK & (x))
#define PS_SEQ(x)		(PS_SEQ_MASK & (x))

#define PS_STATE_S0		(PS_DPS)
#define PS_STATE_S1		(PS_LCLK)
#define PS_STATE_S2		(PS_RF_OFF)
#define PS_STATE_S3		(PS_ALL_ON)
#define PS_STATE_S4		((PS_ST_ACTIVE) | (PS_ALL_ON))


#define PS_IS_RF_ON(x)	((x) & (PS_ALL_ON))
#define PS_IS_ACTIVE(x)	((x) & (PS_ST_ACTIVE))
#define CLR_PS_STATE(x)	((x) = ((x) & (0xF0)))


struct reportpwrstate_parm {
	unsigned char mode;
	unsigned char state; //the CPWM value
	unsigned short rsvd;
};


#define LPS_DELAY_TIME	1*HZ // 1 sec

#define EXE_PWR_NONE	0x01
#define EXE_PWR_IPS		0x02
#define EXE_PWR_LPS		0x04

// RF state.
typedef enum _rt_rf_power_state
{
	rf_on,		// RF is on after RFSleep or RFOff
	rf_sleep,	// 802.11 Power Save mode
	rf_off,		// HW/SW Radio OFF or Inactive Power Save
	//=====Add the new RF state above this line=====//
	rf_max
}rt_rf_power_state;

// RF Off Level for IPS or HW/SW radio off
#define	RT_RF_OFF_LEVL_ASPM			BIT(0)	// PCI ASPM
#define	RT_RF_OFF_LEVL_CLK_REQ		BIT(1)	// PCI clock request
#define	RT_RF_OFF_LEVL_PCI_D3			BIT(2)	// PCI D3 mode
#define	RT_RF_OFF_LEVL_HALT_NIC		BIT(3)	// NIC halt, re-initialize hw parameters
#define	RT_RF_OFF_LEVL_FREE_FW		BIT(4)	// FW free, re-download the FW
#define	RT_RF_OFF_LEVL_FW_32K		BIT(5)	// FW in 32k
#define	RT_RF_PS_LEVEL_ALWAYS_ASPM	BIT(6)	// Always enable ASPM and Clock Req in initialization.
#define	RT_RF_LPS_DISALBE_2R			BIT(30)	// When LPS is on, disable 2R if no packet is received or transmittd.
#define	RT_RF_LPS_LEVEL_ASPM			BIT(31)	// LPS with ASPM

#define	RT_IN_PS_LEVEL(ppsc, _PS_FLAG)		((ppsc->cur_ps_level & _PS_FLAG) ? _TRUE : _FALSE)
#define	RT_CLEAR_PS_LEVEL(ppsc, _PS_FLAG)	(ppsc->cur_ps_level &= (~(_PS_FLAG)))
#define	RT_SET_PS_LEVEL(ppsc, _PS_FLAG)		(ppsc->cur_ps_level |= _PS_FLAG)


enum _PS_BBRegBackup_ {
	PSBBREG_RF0 = 0,
	PSBBREG_RF1,
	PSBBREG_RF2,
	PSBBREG_AFE0,
	PSBBREG_TOTALCNT
};

enum { // for ips_mode
	IPS_NONE=0,
	IPS_NORMAL,
	IPS_LEVEL_2,
};

struct pwrctrl_priv
{
	struct semaphore lock;
	volatile uint8_t rpwm; // requested power state for fw
	volatile uint8_t cpwm; // fw current power state. updated when 1. read from HCPWM 2. driver lowers power level
	volatile uint8_t tog; // toggling
	volatile uint8_t cpwm_tog; // toggling

	uint8_t	pwr_mode;
	uint8_t	smart_ps;
	uint8_t	bcn_ant_mode;

	u32	alives;
	struct work_struct cpwm_event;
#ifdef CONFIG_LPS_RPWM_TIMER
	uint8_t brpwmtimeout;
	struct work_struct rpwmtimeoutwi;
	_timer pwr_rpwm_timer;
#endif // CONFIG_LPS_RPWM_TIMER
	uint8_t	bpower_saving;

	uint8_t	b_hw_radio_off;
	uint8_t	reg_rfoff;
	uint8_t	reg_pdnmode; //powerdown mode
	u32	rfoff_reason;

	//RF OFF Level
	u32	cur_ps_level;
	u32	reg_rfps_level;


	uint 	ips_enter_cnts;
	uint 	ips_leave_cnts;

	uint8_t	ips_mode;
	uint8_t	ips_mode_req; // used to accept the mode setting request, will update to ipsmode later
	uint bips_processing;
	u32 ips_deny_time; /* will deny IPS when system time is smaller than this */
	uint8_t ps_processing; /* temporarily used to mark whether in rtw_ps_processor */

	uint8_t	bLeisurePs;
	uint8_t	LpsIdleCount;
	uint8_t	power_mgnt;
	uint8_t	bFwCurrentInPSMode;
	u32	DelayLPSLastTimeStamp;
	uint8_t 	btcoex_rfon;
	int32_t		pnp_current_pwr_state;
	uint8_t		pnp_bstop_trx;


	uint8_t		bInternalAutoSuspend;
	uint8_t		bInSuspend;
	uint8_t		bSupportRemoteWakeup;
	_timer 	pwr_state_check_timer;
	int		pwr_state_check_interval;
	uint8_t		pwr_state_check_cnts;

	int 		ps_flag;

	rt_rf_power_state	rf_pwrstate;//cur power state
	//rt_rf_power_state 	current_rfpwrstate;
	rt_rf_power_state	change_rfpwrstate;

	uint8_t		wepkeymask;
	uint8_t		bHWPowerdown;//if support hw power down
	uint8_t		bHWPwrPindetect;
	uint8_t		bkeepfwalive;
	uint8_t		brfoffbyhw;
	unsigned long PS_BBRegBackup[PSBBREG_TOTALCNT];
};

#define rtw_get_ips_mode_req(pwrctrlpriv) \
	(pwrctrlpriv)->ips_mode_req

#define rtw_ips_mode_req(pwrctrlpriv, ips_mode) \
	(pwrctrlpriv)->ips_mode_req = (ips_mode)

#define RTW_PWR_STATE_CHK_INTERVAL 2000

#define _rtw_set_pwr_state_check_timer(pwrctrlpriv, ms) \
	do { \
		/*DBG_871X("%s _rtw_set_pwr_state_check_timer(%p, %d)\n", __FUNCTION__, (pwrctrlpriv), (ms));*/ \
		_set_timer(&(pwrctrlpriv)->pwr_state_check_timer, (ms)); \
	} while(0)

#define rtw_set_pwr_state_check_timer(pwrctrlpriv) \
	_rtw_set_pwr_state_check_timer((pwrctrlpriv), (pwrctrlpriv)->pwr_state_check_interval)

extern void rtw_init_pwrctrl_priv(struct rtl_priv *rtlpriv);

extern void rtw_set_ps_mode(struct rtl_priv *rtlpriv, uint8_t ps_mode, uint8_t smart_ps, uint8_t bcn_ant_mode);
extern void rtw_set_rpwm(struct rtl_priv * rtlpriv, uint8_t val8);
extern void LeaveAllPowerSaveMode(struct rtl_priv *rtlpriv);
#ifdef CONFIG_IPS
void ips_enter(struct rtl_priv * rtlpriv);
int ips_leave(struct rtl_priv * rtlpriv);
#endif

void rtw_ps_processor(struct rtl_priv*rtlpriv);

#ifdef CONFIG_AUTOSUSPEND
int autoresume_enter(struct rtl_priv* rtlpriv);
#endif

#ifdef CONFIG_LPS
int32_t LPS_RF_ON_check(struct rtl_priv *rtlpriv, u32 delay_ms);
void LPS_Enter(struct rtl_priv *rtlpriv);
void LPS_Leave(struct rtl_priv *rtlpriv);
#endif

/* ULLI resolve these #define's */

#define rtw_set_do_late_resume(pwrpriv, enable) do {} while (0)
#define rtw_register_early_suspend(pwrpriv) do {} while (0)
#define rtw_unregister_early_suspend(pwrpriv) do {} while (0)

int _rtw_pwr_wakeup(struct rtl_priv *rtlpriv, u32 ips_deffer_ms, const char *caller);
#define rtw_pwr_wakeup(rtlpriv) _rtw_pwr_wakeup(rtlpriv, RTW_PWR_STATE_CHK_INTERVAL, __FUNCTION__)
#define rtw_pwr_wakeup_ex(rtlpriv, ips_deffer_ms) _rtw_pwr_wakeup(rtlpriv, ips_deffer_ms, __FUNCTION__)
int rtw_pm_set_ips(struct rtl_priv *rtlpriv, uint8_t mode);
int rtw_pm_set_lps(struct rtl_priv *rtlpriv, uint8_t mode);

#endif  //__RTL871X_PWRCTRL_H_

