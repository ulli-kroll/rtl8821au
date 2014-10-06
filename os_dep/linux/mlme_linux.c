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


#define _MLME_OSDEP_C_

#include <drv_types.h>

/*
void sitesurvey_ctrl_handler(void *FunctionContext)
{
	_adapter *adapter = (_adapter *)FunctionContext;

	_sitesurvey_ctrl_handler(adapter);

	_set_timer(&adapter->mlmepriv.sitesurveyctrl.sitesurvey_ctrl_timer, 3000);
}
*/

void rtw_join_timeout_handler (void *FunctionContext)
{
	_adapter *adapter = (_adapter *)FunctionContext;
	_rtw_join_timeout_handler(adapter);
}


void _rtw_scan_timeout_handler (void *FunctionContext)
{
	_adapter *adapter = (_adapter *)FunctionContext;
	rtw_scan_timeout_handler(adapter);
}


void _dynamic_check_timer_handlder (void *FunctionContext)
{
	_adapter *adapter = (_adapter *)FunctionContext;

#if (MP_DRIVER == 1)
if (adapter->registrypriv.mp_mode == 1)
	return;
#endif
	rtw_dynamic_check_timer_handlder(adapter);

	_set_timer(&adapter->mlmepriv.dynamic_chk_timer, 2000);
}

#ifdef CONFIG_SET_SCAN_DENY_TIMER
void _rtw_set_scan_deny_timer_hdl(void *FunctionContext)
{
	_adapter *adapter = (_adapter *)FunctionContext;
	rtw_set_scan_deny_timer_hdl(adapter);
}
#endif


void rtw_init_mlme_timer(_adapter *padapter)
{
	struct	mlme_priv *pmlmepriv = &padapter->mlmepriv;

	_init_timer(&(pmlmepriv->assoc_timer), padapter->ndev, rtw_join_timeout_handler, padapter);
	//_init_timer(&(pmlmepriv->sitesurveyctrl.sitesurvey_ctrl_timer), padapter->ndev, sitesurvey_ctrl_handler, padapter);
	_init_timer(&(pmlmepriv->scan_to_timer), padapter->ndev, _rtw_scan_timeout_handler, padapter);

	_init_timer(&(pmlmepriv->dynamic_chk_timer), padapter->ndev, _dynamic_check_timer_handlder, padapter);

	#ifdef CONFIG_SET_SCAN_DENY_TIMER
	_init_timer(&(pmlmepriv->set_scan_deny_timer), padapter->ndev, _rtw_set_scan_deny_timer_hdl, padapter);
	#endif

}

extern void rtw_indicate_wx_assoc_event(_adapter *padapter);
extern void rtw_indicate_wx_disassoc_event(_adapter *padapter);

void rtw_os_indicate_connect(_adapter *adapter)
{

_func_enter_;

	rtw_indicate_wx_assoc_event(adapter);
	netif_carrier_on(adapter->ndev);

	if(adapter->pid[2] !=0)
		rtw_signal_process(adapter->pid[2], SIGALRM);

_func_exit_;

}

extern void indicate_wx_scan_complete_event(_adapter *padapter);
void rtw_os_indicate_scan_done( _adapter *padapter, bool aborted)
{
	indicate_wx_scan_complete_event(padapter);
}

static RT_PMKID_LIST   backupPMKIDList[ NUM_PMKID_CACHE ];
void rtw_reset_securitypriv( _adapter *adapter )
{
	uint8_t	backupPMKIDIndex = 0;
	uint8_t	backupTKIPCountermeasure = 0x00;
	u32	backupTKIPcountermeasure_time = 0;

	if(adapter->securitypriv.dot11AuthAlgrthm == dot11AuthAlgrthm_8021X)//802.1x
	{
		// Added by Albert 2009/02/18
		// We have to backup the PMK information for WiFi PMK Caching test item.
		//
		// Backup the btkip_countermeasure information.
		// When the countermeasure is trigger, the driver have to disconnect with AP for 60 seconds.

		memset( &backupPMKIDList[ 0 ], 0x00, sizeof( RT_PMKID_LIST ) * NUM_PMKID_CACHE );

		memcpy( &backupPMKIDList[ 0 ], &adapter->securitypriv.PMKIDList[ 0 ], sizeof( RT_PMKID_LIST ) * NUM_PMKID_CACHE );
		backupPMKIDIndex = adapter->securitypriv.PMKIDIndex;
		backupTKIPCountermeasure = adapter->securitypriv.btkip_countermeasure;
		backupTKIPcountermeasure_time = adapter->securitypriv.btkip_countermeasure_time;

		memset((unsigned char *)&adapter->securitypriv, 0, sizeof (struct security_priv));
		//_init_timer(&(adapter->securitypriv.tkip_timer),adapter->ndev, rtw_use_tkipkey_handler, adapter);

		// Added by Albert 2009/02/18
		// Restore the PMK information to securitypriv structure for the following connection.
		memcpy( &adapter->securitypriv.PMKIDList[ 0 ], &backupPMKIDList[ 0 ], sizeof( RT_PMKID_LIST ) * NUM_PMKID_CACHE );
		adapter->securitypriv.PMKIDIndex = backupPMKIDIndex;
		adapter->securitypriv.btkip_countermeasure = backupTKIPCountermeasure;
		adapter->securitypriv.btkip_countermeasure_time = backupTKIPcountermeasure_time;

		adapter->securitypriv.ndisauthtype = Ndis802_11AuthModeOpen;
		adapter->securitypriv.ndisencryptstatus = Ndis802_11WEPDisabled;

	}
	else //reset values in securitypriv
	{
		//if(adapter->mlmepriv.fw_state & WIFI_STATION_STATE)
		//{
		struct security_priv *psec_priv=&adapter->securitypriv;

		psec_priv->dot11AuthAlgrthm =dot11AuthAlgrthm_Open;  //open system
		psec_priv->dot11PrivacyAlgrthm = _NO_PRIVACY_;
		psec_priv->dot11PrivacyKeyIndex = 0;

		psec_priv->dot118021XGrpPrivacy = _NO_PRIVACY_;
		psec_priv->dot118021XGrpKeyid = 1;

		psec_priv->ndisauthtype = Ndis802_11AuthModeOpen;
		psec_priv->ndisencryptstatus = Ndis802_11WEPDisabled;
		//}
	}
}

void rtw_os_indicate_disconnect( _adapter *adapter )
{
   //RT_PMKID_LIST   backupPMKIDList[ NUM_PMKID_CACHE ];

_func_enter_;

	netif_carrier_off(adapter->ndev); // Do it first for tx broadcast pkt after disconnection issue!

	rtw_indicate_wx_disassoc_event(adapter);

	 rtw_reset_securitypriv( adapter );

_func_exit_;

}

void rtw_report_sec_ie(_adapter *adapter,uint8_t authmode,uint8_t *sec_ie)
{
	uint	len;
	uint8_t	*buff,*p,i;
	union iwreq_data wrqu;

_func_enter_;

	RT_TRACE(_module_mlme_osdep_c_,_drv_info_,("+rtw_report_sec_ie, authmode=%d\n", authmode));

	buff = NULL;
	if(authmode==_WPA_IE_ID_)
	{
		RT_TRACE(_module_mlme_osdep_c_,_drv_info_,("rtw_report_sec_ie, authmode=%d\n", authmode));

		buff = rtw_malloc(IW_CUSTOM_MAX);

		memset(buff,0,IW_CUSTOM_MAX);

		p=buff;

		p+=sprintf(p,"ASSOCINFO(ReqIEs=");

		len = sec_ie[1]+2;
		len =  (len < IW_CUSTOM_MAX) ? len:IW_CUSTOM_MAX;

		for(i=0;i<len;i++){
			p+=sprintf(p,"%02x",sec_ie[i]);
		}

		p+=sprintf(p,")");

		memset(&wrqu,0,sizeof(wrqu));

		wrqu.data.length=p-buff;

		wrqu.data.length = (wrqu.data.length<IW_CUSTOM_MAX) ? wrqu.data.length:IW_CUSTOM_MAX;

		wireless_send_event(adapter->ndev,IWEVCUSTOM,&wrqu,buff);

		if(buff)
		    rtw_mfree(buff);

	}

_func_exit_;

}

void _survey_timer_hdl (void *FunctionContext)
{
	_adapter *padapter = (_adapter *)FunctionContext;

	survey_timer_hdl(padapter);
}

void _link_timer_hdl (void *FunctionContext)
{
	_adapter *padapter = (_adapter *)FunctionContext;
	link_timer_hdl(padapter);
}

void _addba_timer_hdl(void *FunctionContext)
{
	struct sta_info *psta = (struct sta_info *)FunctionContext;
	addba_timer_hdl(psta);
}

void init_addba_retry_timer(_adapter *padapter, struct sta_info *psta)
{

	_init_timer(&psta->addba_retry_timer, padapter->ndev, _addba_timer_hdl, psta);
}

/*
void _reauth_timer_hdl(void *FunctionContext)
{
	_adapter *padapter = (_adapter *)FunctionContext;
	reauth_timer_hdl(padapter);
}

void _reassoc_timer_hdl(void *FunctionContext)
{
	_adapter *padapter = (_adapter *)FunctionContext;
	reassoc_timer_hdl(padapter);
}
*/

void init_mlme_ext_timer(_adapter *padapter)
{
	struct	mlme_ext_priv *pmlmeext = &padapter->mlmeextpriv;

	_init_timer(&pmlmeext->survey_timer, padapter->ndev, _survey_timer_hdl, padapter);
	_init_timer(&pmlmeext->link_timer, padapter->ndev, _link_timer_hdl, padapter);
	//_init_timer(&pmlmeext->ADDBA_timer, padapter->ndev, _addba_timer_hdl, padapter);

	//_init_timer(&pmlmeext->reauth_timer, padapter->ndev, _reauth_timer_hdl, padapter);
	//_init_timer(&pmlmeext->reassoc_timer, padapter->ndev, _reassoc_timer_hdl, padapter);
}

uint8_t rtw_handle_tkip_countermeasure(_adapter* padapter)
{
	uint8_t status = _SUCCESS;
	u32 cur_time = 0;

	if (padapter->securitypriv.btkip_countermeasure == _TRUE) {
		cur_time = rtw_get_current_time();

		if( (cur_time - padapter->securitypriv.btkip_countermeasure_time) > 60 * HZ )
		{
			padapter->securitypriv.btkip_countermeasure = _FALSE;
			padapter->securitypriv.btkip_countermeasure_time = 0;
		}
		else
		{
			status = _FAIL;
		}
	}

	return status;

}

#ifdef CONFIG_AP_MODE

void rtw_indicate_sta_assoc_event(_adapter *padapter, struct sta_info *psta)
{
	union iwreq_data wrqu;
	struct sta_priv *pstapriv = &padapter->stapriv;

	if(psta==NULL)
		return;

	if(psta->aid > NUM_STA)
		return;

	if(pstapriv->sta_aid[psta->aid - 1] != psta)
		return;


	wrqu.addr.sa_family = ARPHRD_ETHER;

	memcpy(wrqu.addr.sa_data, psta->hwaddr, ETH_ALEN);

	DBG_871X("+rtw_indicate_sta_assoc_event\n");

	wireless_send_event(padapter->ndev, IWEVREGISTERED, &wrqu, NULL);

}

void rtw_indicate_sta_disassoc_event(_adapter *padapter, struct sta_info *psta)
{
	union iwreq_data wrqu;
	struct sta_priv *pstapriv = &padapter->stapriv;

	if(psta==NULL)
		return;

	if(psta->aid > NUM_STA)
		return;

	if(pstapriv->sta_aid[psta->aid - 1] != psta)
		return;


	wrqu.addr.sa_family = ARPHRD_ETHER;

	memcpy(wrqu.addr.sa_data, psta->hwaddr, ETH_ALEN);

	DBG_871X("+rtw_indicate_sta_disassoc_event\n");

	wireless_send_event(padapter->ndev, IWEVEXPIRED, &wrqu, NULL);

}


#endif

