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
#define _RTW_PWRCTRL_C_

#include <drv_types.h>

// Should not include hal dependent herader here, it will remove later. Lucas@20130123

#ifdef CONFIG_IPS

void ips_enter(struct _ADAPTER *padapter)
{
	struct pwrctrl_priv *pwrpriv = &padapter->pwrctrlpriv;

	down(&pwrpriv->lock);

	pwrpriv->bips_processing = _TRUE;

	/* syn ips_mode with request */
	pwrpriv->ips_mode = pwrpriv->ips_mode_req;

	pwrpriv->ips_enter_cnts++;
	DBG_871X("==>ips_enter cnts:%d\n",pwrpriv->ips_enter_cnts);
	if (rf_off == pwrpriv->change_rfpwrstate) {
		pwrpriv->bpower_saving = _TRUE;
		DBG_871X_LEVEL(_drv_always_, "nolinked power save enter\n");

		if(pwrpriv->ips_mode == IPS_LEVEL_2)
			pwrpriv->bkeepfwalive = _TRUE;

		rtw_ips_pwr_down(padapter);
		pwrpriv->rf_pwrstate = rf_off;
	}
	pwrpriv->bips_processing = _FALSE;

	up(&pwrpriv->lock);

}

int ips_leave(struct _ADAPTER * padapter)
{
	struct pwrctrl_priv *pwrpriv = &padapter->pwrctrlpriv;
	struct security_priv* psecuritypriv=&(padapter->securitypriv);
	struct mlme_priv *pmlmepriv = &(padapter->mlmepriv);
	int result = _SUCCESS;
	sint keyid;

	down(&pwrpriv->lock);

	if ((pwrpriv->rf_pwrstate == rf_off) &&(!pwrpriv->bips_processing)) {
		pwrpriv->bips_processing = _TRUE;
		pwrpriv->change_rfpwrstate = rf_on;
		pwrpriv->ips_leave_cnts++;
		DBG_871X("==>ips_leave cnts:%d\n",pwrpriv->ips_leave_cnts);

		if ((result = rtw_ips_pwr_up(padapter)) == _SUCCESS) {
			pwrpriv->rf_pwrstate = rf_on;
		}
		DBG_871X_LEVEL(_drv_always_, "nolinked power save leave\n");

		if ((_WEP40_ == psecuritypriv->dot11PrivacyAlgrthm)
		   ||(_WEP104_ == psecuritypriv->dot11PrivacyAlgrthm)) {
			DBG_871X("==>%s,channel(%d),processing(%x)\n",__FUNCTION__,padapter->mlmeextpriv.cur_channel,pwrpriv->bips_processing);
			set_channel_bwmode(padapter, padapter->mlmeextpriv.cur_channel, HAL_PRIME_CHNL_OFFSET_DONT_CARE, CHANNEL_WIDTH_20);
			for (keyid = 0; keyid < 4; keyid++) {
				if (pmlmepriv->key_mask & BIT(keyid)) {
					if (keyid == psecuritypriv->dot11PrivacyKeyIndex)
						result=rtw_set_key(padapter,psecuritypriv, keyid, 1);
					else
						result=rtw_set_key(padapter,psecuritypriv, keyid, 0);
				}
			}
		}

		DBG_871X("==> ips_leave.....LED(0x%08x)...\n",rtw_read32(padapter,0x4c));
		pwrpriv->bips_processing = _FALSE;

		pwrpriv->bkeepfwalive = _FALSE;
		pwrpriv->bpower_saving = _FALSE;
	}

	up(&pwrpriv->lock);

	return result;
}


#endif

#ifdef CONFIG_AUTOSUSPEND
extern void autosuspend_enter(struct _ADAPTER* padapter);
extern int autoresume_enter(struct _ADAPTER* padapter);
#endif

bool rtw_pwr_unassociated_idle(struct _ADAPTER *adapter)
{
	struct mlme_priv *pmlmepriv = &(adapter->mlmepriv);
	struct xmit_priv *pxmit_priv = &adapter->xmitpriv;

	bool ret = _FALSE;

	if (adapter->pwrctrlpriv.ips_deny_time >= jiffies) {
		//DBG_871X("%s ips_deny_time\n", __func__);
		goto exit;
	}

	if (check_fwstate(pmlmepriv, WIFI_ASOC_STATE|WIFI_SITE_MONITOR)
		|| check_fwstate(pmlmepriv, WIFI_UNDER_LINKING|WIFI_UNDER_WPS)
		|| check_fwstate(pmlmepriv, WIFI_AP_STATE)
		|| check_fwstate(pmlmepriv, WIFI_ADHOC_MASTER_STATE|WIFI_ADHOC_STATE)
	) {
		goto exit;
	}

	if (pxmit_priv->free_xmitbuf_cnt != NR_XMITBUFF ||
		pxmit_priv->free_xmit_extbuf_cnt != NR_XMIT_EXTBUFF) {
		DBG_871X_LEVEL(_drv_always_, "There are some pkts to transmit\n");
		DBG_871X_LEVEL(_drv_info_, "free_xmitbuf_cnt: %d, free_xmit_extbuf_cnt: %d\n",
			pxmit_priv->free_xmitbuf_cnt, pxmit_priv->free_xmit_extbuf_cnt);
		goto exit;
	}

	ret = _TRUE;

exit:
	return ret;
}

void rtw_ps_processor(struct _ADAPTER *padapter)
{
	struct pwrctrl_priv *pwrpriv = &padapter->pwrctrlpriv;
	struct mlme_priv *pmlmepriv = &(padapter->mlmepriv);

	pwrpriv->ps_processing = _TRUE;

	if (pwrpriv->ips_mode_req == IPS_NONE
	)
		goto exit;

	if (rtw_pwr_unassociated_idle(padapter) == _FALSE)
		goto exit;

	if ((pwrpriv->rf_pwrstate == rf_on)
	   && ((pwrpriv->pwr_state_check_cnts%4)==0)) {
		DBG_871X("==>%s .fw_state(%x)\n",__FUNCTION__,get_fwstate(pmlmepriv));
		pwrpriv->change_rfpwrstate = rf_off;
#ifdef CONFIG_AUTOSUSPEND
		if (padapter->registrypriv.usbss_enable) {
			if(pwrpriv->bHWPwrPindetect)
				pwrpriv->bkeepfwalive = _TRUE;

			if(padapter->net_closed == _TRUE)
				pwrpriv->ps_flag = _TRUE;

			padapter->bCardDisableWOHSM = _TRUE;
			autosuspend_enter(padapter);
		} else if(pwrpriv->bHWPwrPindetect) {
		}
		else
#endif
		{

#ifdef CONFIG_IPS
			ips_enter(padapter);
#endif
		}
	}
exit:
	rtw_set_pwr_state_check_timer(&padapter->pwrctrlpriv);
	pwrpriv->ps_processing = _FALSE;
	return;
}


static void pwr_state_check_handler(RTW_TIMER_HDL_ARGS)
{
	struct _ADAPTER *padapter = (struct _ADAPTER *)FunctionContext;
	rtw_ps_cmd(padapter);
}


#ifdef CONFIG_LPS
/*
 *
 * Parameters
 *	padapter
 *	pslv			power state level, only could be PS_STATE_S0 ~ PS_STATE_S4
 *
 */
void rtw_set_rpwm(struct _ADAPTER *padapter, uint8_t pslv)
{
	uint8_t	rpwm;
	struct pwrctrl_priv *pwrpriv = &padapter->pwrctrlpriv;

_func_enter_;

	pslv = PS_STATE(pslv);

	if (_TRUE == pwrpriv->btcoex_rfon) {
		if (pslv < PS_STATE_S4)
			pslv = PS_STATE_S3;
	}

#ifdef CONFIG_LPS_RPWM_TIMER
	if (pwrpriv->brpwmtimeout == _TRUE) {
		DBG_871X("%s: RPWM timeout, force to set RPWM(0x%02X) again!\n", __FUNCTION__, pslv);
	} else
#endif // CONFIG_LPS_RPWM_TIMER
	{
	if ( (pwrpriv->rpwm == pslv)
		)
	{
		RT_TRACE(_module_rtl871x_pwrctrl_c_,_drv_err_,
			("%s: Already set rpwm[0x%02X], new=0x%02X!\n", __FUNCTION__, pwrpriv->rpwm, pslv));
		return;
	}
	}

	if ((padapter->bSurpriseRemoved == _TRUE)
	   || (padapter->hw_init_completed == _FALSE)) {
		RT_TRACE(_module_rtl871x_pwrctrl_c_, _drv_err_,
			 ("%s: SurpriseRemoved(%d) hw_init_completed(%d)\n",
			  __FUNCTION__, padapter->bSurpriseRemoved, padapter->hw_init_completed));

		pwrpriv->cpwm = PS_STATE_S4;
		return;
	}

	if (padapter->bDriverStopped == _TRUE) {
		RT_TRACE(_module_rtl871x_pwrctrl_c_, _drv_err_,
			 ("%s: change power state(0x%02X) when DriverStopped\n", __FUNCTION__, pslv));

		if (pslv < PS_STATE_S2) {
			RT_TRACE(_module_rtl871x_pwrctrl_c_, _drv_err_,
				 ("%s: Reject to enter PS_STATE(0x%02X) lower than S2 when DriverStopped!!\n", __FUNCTION__, pslv));
			return;
		}
	}

	rpwm = pslv | pwrpriv->tog;
	RT_TRACE(_module_rtl871x_pwrctrl_c_, _drv_notice_,
		 ("rtw_set_rpwm: rpwm=0x%02x cpwm=0x%02x\n", rpwm, pwrpriv->cpwm));

	pwrpriv->rpwm = pslv;

#ifdef CONFIG_LPS_RPWM_TIMER
	if (rpwm & PS_ACK)
		_set_timer(&pwrpriv->pwr_rpwm_timer, LPS_RPWM_WAIT_MS);
#endif // CONFIG_LPS_RPWM_TIMER
	rtw_hal_set_hwreg(padapter, HW_VAR_SET_RPWM, (uint8_t *)(&rpwm));

	pwrpriv->tog += 0x80;

	{
		pwrpriv->cpwm = pslv;
	}

_func_exit_;
}

static uint8_t PS_RDY_CHECK(struct _ADAPTER *padapter)
{
	uint32_t	 curr_time, delta_time;
	struct pwrctrl_priv	*pwrpriv = &padapter->pwrctrlpriv;
	struct mlme_priv	*pmlmepriv = &(padapter->mlmepriv);

	curr_time = jiffies;

	delta_time = curr_time -pwrpriv->DelayLPSLastTimeStamp;

	if(delta_time < LPS_DELAY_TIME) {
		return _FALSE;
	}

	if ((check_fwstate(pmlmepriv, _FW_LINKED) == _FALSE) ||
		(check_fwstate(pmlmepriv, _FW_UNDER_SURVEY) == _TRUE) ||
		(check_fwstate(pmlmepriv, WIFI_AP_STATE) == _TRUE) ||
		(check_fwstate(pmlmepriv, WIFI_ADHOC_MASTER_STATE) == _TRUE) ||
		(check_fwstate(pmlmepriv, WIFI_ADHOC_STATE) == _TRUE) )
		return _FALSE;
	if (_TRUE == pwrpriv->bInSuspend )
		return _FALSE;
	if ((padapter->securitypriv.dot11AuthAlgrthm == dot11AuthAlgrthm_8021X)
	  && (padapter->securitypriv.binstallGrpkey == _FALSE)) {
		DBG_871X("Group handshake still in progress !!!\n");
		return _FALSE;
	}

	return _TRUE;
}

void rtw_set_ps_mode(struct _ADAPTER *padapter, uint8_t ps_mode, uint8_t smart_ps, uint8_t bcn_ant_mode)
{
	struct pwrctrl_priv *pwrpriv = &padapter->pwrctrlpriv;

_func_enter_;

	RT_TRACE(_module_rtl871x_pwrctrl_c_, _drv_notice_,
			 ("%s: PowerMode=%d Smart_PS=%d\n",
			  __FUNCTION__, ps_mode, smart_ps));

	if (ps_mode > PM_Card_Disable) {
		RT_TRACE(_module_rtl871x_pwrctrl_c_,_drv_err_,("ps_mode:%d error\n", ps_mode));
		return;
	}

	if (pwrpriv->pwr_mode == ps_mode) {
		if (PS_MODE_ACTIVE == ps_mode)
			return;

		if ((pwrpriv->smart_ps == smart_ps)
		   && (pwrpriv->bcn_ant_mode == bcn_ant_mode)) {
			return;
		}
	}

	//if(pwrpriv->pwr_mode == PS_MODE_ACTIVE)
	if(ps_mode == PS_MODE_ACTIVE)
	{
		{
			DBG_871X("rtw_set_ps_mode: Leave 802.11 power save\n");


			pwrpriv->pwr_mode = ps_mode;
			rtw_set_rpwm(padapter, PS_STATE_S4);
			rtw_hal_set_hwreg(padapter, HW_VAR_H2C_FW_PWRMODE, (uint8_t *)(&ps_mode));
			pwrpriv->bFwCurrentInPSMode = _FALSE;
		}
	} else 	{
		if (PS_RDY_CHECK(padapter)) {
			DBG_871X("%s: Enter 802.11 power save\n", __FUNCTION__);


			pwrpriv->bFwCurrentInPSMode = _TRUE;
			pwrpriv->pwr_mode = ps_mode;
			pwrpriv->smart_ps = smart_ps;
			pwrpriv->bcn_ant_mode = bcn_ant_mode;
			rtw_hal_set_hwreg(padapter, HW_VAR_H2C_FW_PWRMODE, (uint8_t *)(&ps_mode));

			rtw_set_rpwm(padapter, PS_STATE_S2);
		}
	}

_func_exit_;
}

/*
 * Return:
 *	0:	Leave OK
 *	-1:	Timeout
 *	-2:	Other error
 */
int32_t LPS_RF_ON_check(struct _ADAPTER *padapter, uint32_t	 delay_ms)
{
	uint32_t	 start_time;
	uint8_t bAwake = _FALSE;
	int32_t err = 0;


	start_time = jiffies;
	while (1) {
		rtw_hal_get_hwreg(padapter, HW_VAR_FWLPS_RF_ON, &bAwake);
		if (_TRUE == bAwake)
			break;

		if (_TRUE == padapter->bSurpriseRemoved) {
			err = -2;
			DBG_871X("%s: device surprise removed!!\n", __FUNCTION__);
			break;
		}

		if (rtw_get_passing_time_ms(start_time) > delay_ms) {
			err = -1;
			DBG_871X("%s: Wait for FW LPS leave more than %u ms!!!\n", __FUNCTION__, delay_ms);
			break;
		}
		rtw_usleep_os(100);
	}

	return err;
}

//
//	Description:
//		Enter the leisure power save mode.
//
void LPS_Enter(struct _ADAPTER *padapter)
{
	struct pwrctrl_priv	*pwrpriv = &padapter->pwrctrlpriv;
	struct mlme_priv	*pmlmepriv = &(padapter->mlmepriv);

_func_enter_;

//	DBG_871X("+LeisurePSEnter\n");


	if (PS_RDY_CHECK(padapter) == _FALSE)
		return;

	if (pwrpriv->bLeisurePs) {
		// Idle for a while if we connect to AP a while ago.
		if(pwrpriv->LpsIdleCount >= 2) { //  4 Sec
			if(pwrpriv->pwr_mode == PS_MODE_ACTIVE) {
				pwrpriv->bpower_saving = _TRUE;
				rtw_set_ps_mode(padapter, pwrpriv->power_mgnt, padapter->registrypriv.smart_ps, 0);
			}
		} else
			pwrpriv->LpsIdleCount++;
	}

//	DBG_871X("-LeisurePSEnter\n");

_func_exit_;
}

//
//	Description:
//		Leave the leisure power save mode.
//
void LPS_Leave(struct _ADAPTER *padapter)
{
#define LPS_LEAVE_TIMEOUT_MS 100

	struct pwrctrl_priv	*pwrpriv = &padapter->pwrctrlpriv;
	uint32_t	 start_time;
	uint8_t bAwake = _FALSE;

_func_enter_;


//	DBG_871X("+LeisurePSLeave\n");

	if (pwrpriv->bLeisurePs) {
		if(pwrpriv->pwr_mode != PS_MODE_ACTIVE) {
			rtw_set_ps_mode(padapter, PS_MODE_ACTIVE, 0, 0);

			if(pwrpriv->pwr_mode == PS_MODE_ACTIVE)
				LPS_RF_ON_check(padapter, LPS_LEAVE_TIMEOUT_MS);
		}
	}

	pwrpriv->bpower_saving = _FALSE;

//	DBG_871X("-LeisurePSLeave\n");

_func_exit_;
}
#endif

//
// Description: Leave all power save mode: LPS, FwLPS, IPS if needed.
// Move code to function by tynli. 2010.03.26.
//
void LeaveAllPowerSaveMode(IN struct _ADAPTER *Adapter)
{
	struct mlme_priv	*pmlmepriv = &(Adapter->mlmepriv);
	uint8_t	enqueue = 0;

_func_enter_;

	//DBG_871X("%s.....\n",__FUNCTION__);
	if (check_fwstate(pmlmepriv, _FW_LINKED) == _TRUE)
	{ //connect
#ifdef CONFIG_LPS
		rtw_lps_ctrl_wk_cmd(Adapter, LPS_CTRL_LEAVE, enqueue);
#endif

	} else {
		if(Adapter->pwrctrlpriv.rf_pwrstate== rf_off) {
#ifdef CONFIG_AUTOSUSPEND
			if(Adapter->registrypriv.usbss_enable) 	{
				usb_disable_autosuspend(adapter_to_dvobj(Adapter)->pusbdev);
			} else
#endif
			{
			}
		}
	}

_func_exit_;
}

void rtw_init_pwrctrl_priv(struct _ADAPTER *padapter)
{
	struct pwrctrl_priv *pwrctrlpriv = &padapter->pwrctrlpriv;

_func_enter_;

	sema_init(&pwrctrlpriv->lock, 1);
	pwrctrlpriv->rf_pwrstate = rf_on;
	pwrctrlpriv->ips_enter_cnts=0;
	pwrctrlpriv->ips_leave_cnts=0;
	pwrctrlpriv->bips_processing = _FALSE;

	pwrctrlpriv->ips_mode = padapter->registrypriv.ips_mode;
	pwrctrlpriv->ips_mode_req = padapter->registrypriv.ips_mode;

	pwrctrlpriv->pwr_state_check_interval = RTW_PWR_STATE_CHK_INTERVAL;
	pwrctrlpriv->pwr_state_check_cnts = 0;
	pwrctrlpriv->bInternalAutoSuspend = _FALSE;
	pwrctrlpriv->bInSuspend = _FALSE;
	pwrctrlpriv->bkeepfwalive = _FALSE;

	pwrctrlpriv->LpsIdleCount = 0;
	//pwrctrlpriv->FWCtrlPSMode =padapter->registrypriv.power_mgnt;// PS_MODE_MIN;
	if (padapter->registrypriv.mp_mode == 1)
		pwrctrlpriv->power_mgnt =PS_MODE_ACTIVE ;
	else
		pwrctrlpriv->power_mgnt =padapter->registrypriv.power_mgnt;// PS_MODE_MIN;
	pwrctrlpriv->bLeisurePs = (PS_MODE_ACTIVE != pwrctrlpriv->power_mgnt)?_TRUE:_FALSE;

	pwrctrlpriv->bFwCurrentInPSMode = _FALSE;

	pwrctrlpriv->rpwm = 0;
	pwrctrlpriv->cpwm = PS_STATE_S4;

	pwrctrlpriv->pwr_mode = PS_MODE_ACTIVE;
	pwrctrlpriv->smart_ps = padapter->registrypriv.smart_ps;
	pwrctrlpriv->bcn_ant_mode = 0;

	pwrctrlpriv->tog = 0x80;

	pwrctrlpriv->btcoex_rfon = _FALSE;

	rtw_init_timer(&pwrctrlpriv->pwr_state_check_timer, padapter, pwr_state_check_handler);

_func_exit_;

}


void rtw_free_pwrctrl_priv(struct _ADAPTER *adapter)
{
	struct pwrctrl_priv *pwrctrlpriv = &adapter->pwrctrlpriv;

_func_enter_;

	//memset((unsigned char *)pwrctrlpriv, 0, sizeof(struct pwrctrl_priv));

_func_exit_;
}



uint8_t rtw_interface_ps_func(struct _ADAPTER *padapter,HAL_INTF_PS_FUNC efunc_id,uint8_t * val)
{
	uint8_t bResult = _TRUE;
	rtw_hal_intf_ps_func(padapter,efunc_id,val);

	return bResult;
}


inline void rtw_set_ips_deny(struct _ADAPTER *padapter, uint32_t	 ms)
{
	struct pwrctrl_priv *pwrpriv = &padapter->pwrctrlpriv;
	pwrpriv->ips_deny_time = jiffies + rtw_ms_to_systime(ms);
}

/*
* rtw_pwr_wakeup - Wake the NIC up from: 1)IPS. 2)USB autosuspend
* @adapter: pointer to struct _ADAPTER structure
* @ips_deffer_ms: the ms wiil prevent from falling into IPS after wakeup
* Return _SUCCESS or _FAIL
*/

int _rtw_pwr_wakeup(struct _ADAPTER *padapter, uint32_t	 ips_deffer_ms, const char *caller)
{
	struct pwrctrl_priv *pwrpriv = &padapter->pwrctrlpriv;
	struct mlme_priv *pmlmepriv = &padapter->mlmepriv;
	int ret = _SUCCESS;
	uint32_t	 start = jiffies;


	if (pwrpriv->ips_deny_time < jiffies + rtw_ms_to_systime(ips_deffer_ms))
		pwrpriv->ips_deny_time = jiffies + rtw_ms_to_systime(ips_deffer_ms);


	if (pwrpriv->ps_processing) {
		DBG_871X("%s wait ps_processing...\n", __func__);
		while (pwrpriv->ps_processing && rtw_get_passing_time_ms(start) <= 3000)
			msleep(10);
		if (pwrpriv->ps_processing)
			DBG_871X("%s wait ps_processing timeout\n", __func__);
		else
			DBG_871X("%s wait ps_processing done\n", __func__);
	}

#ifdef DBG_CONFIG_ERROR_DETECT
	if (rtw_hal_sreset_inprogress(padapter)) {
		DBG_871X("%s wait sreset_inprogress...\n", __func__);
		while (rtw_hal_sreset_inprogress(padapter) && rtw_get_passing_time_ms(start) <= 4000)
			msleep(10);
		if (rtw_hal_sreset_inprogress(padapter))
			DBG_871X("%s wait sreset_inprogress timeout\n", __func__);
		else
			DBG_871X("%s wait sreset_inprogress done\n", __func__);
	}
#endif

	if (pwrpriv->bInternalAutoSuspend == _FALSE && pwrpriv->bInSuspend) {
		DBG_871X("%s wait bInSuspend...\n", __func__);
		while (pwrpriv->bInSuspend
			&& ((rtw_get_passing_time_ms(start) <= 3000)
				|| (rtw_get_passing_time_ms(start) <= 500))
		) {
			msleep(10);
		}

		if (pwrpriv->bInSuspend)
			DBG_871X("%s wait bInSuspend timeout\n", __func__);
		else
			DBG_871X("%s wait bInSuspend done\n", __func__);
	}

	//System suspend is not allowed to wakeup
	if ((pwrpriv->bInternalAutoSuspend == _FALSE) && (_TRUE == pwrpriv->bInSuspend )){
		ret = _FAIL;
		goto exit;
	}

	//block???
	if ((pwrpriv->bInternalAutoSuspend == _TRUE)  && (padapter->net_closed == _TRUE)) {
		ret = _FAIL;
		goto exit;
	}

	//I think this should be check in IPS, LPS, autosuspend functions...
	if (check_fwstate(pmlmepriv, _FW_LINKED) == _TRUE) {
		ret = _SUCCESS;
		goto exit;
	}

	if (rf_off == pwrpriv->rf_pwrstate ) {
#ifdef CONFIG_AUTOSUSPEND
		if (pwrpriv->brfoffbyhw==_TRUE) {
			DBG_8192C("hw still in rf_off state ...........\n");
			ret = _FAIL;
			goto exit;
		} else if (padapter->registrypriv.usbss_enable) {
			DBG_8192C("%s call autoresume_enter....\n",__FUNCTION__);
			if (_FAIL ==  autoresume_enter(padapter)) {
				DBG_8192C("======> autoresume fail.............\n");
				ret = _FAIL;
				goto exit;
			}
		} else
#endif
		{
#ifdef CONFIG_IPS
			DBG_8192C("%s call ips_leave....\n",__FUNCTION__);
			if (_FAIL ==  ips_leave(padapter)) {
				DBG_8192C("======> ips_leave fail.............\n");
				ret = _FAIL;
				goto exit;
			}
#endif
		}
	}

	//TODO: the following checking need to be merged...
	if (padapter->bDriverStopped
		|| !padapter->bup
		|| !padapter->hw_init_completed
	){
		DBG_8192C("%s: bDriverStopped=%d, bup=%d, hw_init_completed=%u\n"
			, caller
		   	, padapter->bDriverStopped
		   	, padapter->bup
		   	, padapter->hw_init_completed);
		ret= _FALSE;
		goto exit;
	}

exit:
	if (pwrpriv->ips_deny_time < jiffies + rtw_ms_to_systime(ips_deffer_ms))
		pwrpriv->ips_deny_time = jiffies + rtw_ms_to_systime(ips_deffer_ms);
	return ret;

}

int rtw_pm_set_lps(struct _ADAPTER *padapter, uint8_t mode)
{
	int	ret = 0;
	struct pwrctrl_priv *pwrctrlpriv = &padapter->pwrctrlpriv;

	if (mode < PS_MODE_NUM) {
		if (pwrctrlpriv->power_mgnt !=mode) {
			if (PS_MODE_ACTIVE == mode) {
				LeaveAllPowerSaveMode(padapter);
			} else {
				pwrctrlpriv->LpsIdleCount = 2;
			}
			pwrctrlpriv->power_mgnt = mode;
			pwrctrlpriv->bLeisurePs = (PS_MODE_ACTIVE != pwrctrlpriv->power_mgnt)?_TRUE:_FALSE;
		}
	} else {
		ret = -EINVAL;
	}

	return ret;
}

int rtw_pm_set_ips(struct _ADAPTER *padapter, uint8_t mode)
{
	struct pwrctrl_priv *pwrctrlpriv = &padapter->pwrctrlpriv;

	if (mode == IPS_NORMAL || mode == IPS_LEVEL_2 ) {
		rtw_ips_mode_req(pwrctrlpriv, mode);
		DBG_871X("%s %s\n", __FUNCTION__, mode == IPS_NORMAL?"IPS_NORMAL":"IPS_LEVEL_2");
		return 0;
	} else if ( mode ==IPS_NONE) {
		rtw_ips_mode_req(pwrctrlpriv, mode);
		DBG_871X("%s %s\n", __FUNCTION__, "IPS_NONE");
		if ((padapter->bSurpriseRemoved ==0)&&(_FAIL == rtw_pwr_wakeup(padapter)) )
			return -EFAULT;
	} else {
		return -EINVAL;
	}
	return 0;
}


