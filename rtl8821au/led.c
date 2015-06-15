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

#include <drv_types.h>

/*
 * ULLI led.c is a mess, we have two different paths for
 * rtl8812au and rtl8821au why ??
 *
 * And up too three leds ...
 */



static void SwLedOn(struct rtl_priv *rtlpriv, struct rtl_led *pLed)
{
	struct rtl_usb_priv *usbpriv = rtl_usbpriv(rtlpriv);

	usbpriv->ledpriv.SwLedOn(rtlpriv, pLed);
}

static void SwLedOff(struct rtl_priv *rtlpriv, struct rtl_led *pLed)
{
	struct rtl_usb_priv *usbpriv = rtl_usbpriv(rtlpriv);

	usbpriv->ledpriv.SwLedOff(rtlpriv, pLed);
}

/*
 * 	Description:
 *		Implementation of LED blinking behavior.
 *		It toggle off LED and schedule corresponding timer if necessary.
 */

static void SwLedBlink1(struct rtl_led *pLed)
{
	struct rtl_priv	 *rtlpriv = pLed->rtlpriv;
	struct rtl_hal *rtlhal = rtl_hal(rtlpriv);
	EEPROM_EFUSE_PRIV *pEEPROM = GET_EEPROM_EFUSE_PRIV(rtlpriv);
	struct rtl_usb_priv *usbpriv = rtl_usbpriv(rtlpriv);
	struct rtl_led_ctl *ledpriv = &(usbpriv->ledpriv);
	struct mlme_priv *pmlmepriv = &(rtlpriv->mlmepriv);
	struct rtl_led *pLed1 = &(ledpriv->SwLed1);
	u8		bStopBlinking = _FALSE;

	u32 uLedBlinkNoLinkInterval = LED_BLINK_NO_LINK_INTERVAL_ALPHA;	/* add by ylb 20121012 for customer led for alpha */
	if (rtlhal->oem_id == RT_CID_ALPHA_Dlink)
		uLedBlinkNoLinkInterval = LED_BLINK_NO_LINK_INTERVAL_ALPHA_500MS;

	/* Change LED according to BlinkingLedState specified. */
	if (pLed->BlinkingLedState == RTW_LED_ON) {
		SwLedOn(rtlpriv, pLed);
	} else {
		SwLedOff(rtlpriv, pLed);
	}


	if (rtlhal->oem_id == RT_CID_DEFAULT) {
		if (check_fwstate(pmlmepriv, _FW_LINKED) == _TRUE) {
			if (!pLed1->bSWLedCtrl) {
				SwLedOn(rtlpriv, pLed1);
				pLed1->bSWLedCtrl = _TRUE;
			} else if (!pLed1->bLedOn)
				SwLedOn(rtlpriv, pLed1);
		} else {
			if (!pLed1->bSWLedCtrl) {
				SwLedOff(rtlpriv, pLed1);
				pLed1->bSWLedCtrl = _TRUE;
			} else if (pLed1->bLedOn)
				SwLedOff(rtlpriv, pLed1);
		}
	}

	switch (pLed->CurrLedState) {
	case LED_BLINK_SLOWLY:
		if (pLed->bLedOn)
			pLed->BlinkingLedState = RTW_LED_OFF;
		else
			pLed->BlinkingLedState = RTW_LED_ON;
		_set_timer(&(pLed->BlinkTimer), uLedBlinkNoLinkInterval);	/* change by ylb 20121012 for customer led for alpha */
		break;

	case LED_BLINK_NORMAL:
		if (pLed->bLedOn)
			pLed->BlinkingLedState = RTW_LED_OFF;
		else
			pLed->BlinkingLedState = RTW_LED_ON;
		_set_timer(&(pLed->BlinkTimer), LED_BLINK_LINK_INTERVAL_ALPHA);
		break;

	case LED_BLINK_SCAN:
		pLed->BlinkTimes--;
		if (pLed->BlinkTimes == 0) {
			bStopBlinking = _TRUE;
		}

		if (bStopBlinking) {
			if (rtlpriv->pwrctrlpriv.rf_pwrstate != rf_on) {
				SwLedOff(rtlpriv, pLed);
			} else if (check_fwstate(pmlmepriv, _FW_LINKED) == _TRUE) {
				pLed->bLedLinkBlinkInProgress = _TRUE;
				pLed->CurrLedState = LED_BLINK_NORMAL;
				if (pLed->bLedOn)
					pLed->BlinkingLedState = RTW_LED_OFF;
				else
					pLed->BlinkingLedState = RTW_LED_ON;
				_set_timer(&(pLed->BlinkTimer), LED_BLINK_LINK_INTERVAL_ALPHA);
			} else if (check_fwstate(pmlmepriv, _FW_LINKED) == _FALSE) {
				pLed->bLedNoLinkBlinkInProgress = _TRUE;
				pLed->CurrLedState = LED_BLINK_SLOWLY;
				if (pLed->bLedOn)
					pLed->BlinkingLedState = RTW_LED_OFF;
				else
					pLed->BlinkingLedState = RTW_LED_ON;
				_set_timer(&(pLed->BlinkTimer), uLedBlinkNoLinkInterval);
			}
			pLed->bLedScanBlinkInProgress = _FALSE;
		} else {
			if (rtlpriv->pwrctrlpriv.rf_pwrstate != rf_on) {
				SwLedOff(rtlpriv, pLed);
			} else {
				if (pLed->bLedOn)
					pLed->BlinkingLedState = RTW_LED_OFF;
				else
					pLed->BlinkingLedState = RTW_LED_ON;
				_set_timer(&(pLed->BlinkTimer), LED_BLINK_SCAN_INTERVAL_ALPHA);
			}
		}
		break;

	case LED_BLINK_TXRX:
		pLed->BlinkTimes--;
		if (pLed->BlinkTimes == 0) {
			bStopBlinking = _TRUE;
		}

		if (bStopBlinking) {
			if (rtlpriv->pwrctrlpriv.rf_pwrstate != rf_on) {
				SwLedOff(rtlpriv, pLed);
			} else if (check_fwstate(pmlmepriv, _FW_LINKED) == _TRUE) {
				pLed->bLedLinkBlinkInProgress = _TRUE;
				pLed->CurrLedState = LED_BLINK_NORMAL;
				if (pLed->bLedOn)
					pLed->BlinkingLedState = RTW_LED_OFF;
				else
					pLed->BlinkingLedState = RTW_LED_ON;
				_set_timer(&(pLed->BlinkTimer), LED_BLINK_LINK_INTERVAL_ALPHA);
			} else if (check_fwstate(pmlmepriv, _FW_LINKED) == _FALSE) {
				pLed->bLedNoLinkBlinkInProgress = _TRUE;
				pLed->CurrLedState = LED_BLINK_SLOWLY;
				if (pLed->bLedOn)
					pLed->BlinkingLedState = RTW_LED_OFF;
				else
					pLed->BlinkingLedState = RTW_LED_ON;
				_set_timer(&(pLed->BlinkTimer), uLedBlinkNoLinkInterval);
			}
			pLed->BlinkTimes = 0;
			pLed->bLedBlinkInProgress = _FALSE;
		} else {
			if (rtlpriv->pwrctrlpriv.rf_pwrstate != rf_on) {
				SwLedOff(rtlpriv, pLed);
			} else {
				if (pLed->bLedOn)
					pLed->BlinkingLedState = RTW_LED_OFF;
				else
					pLed->BlinkingLedState = RTW_LED_ON;
				_set_timer(&(pLed->BlinkTimer), LED_BLINK_FASTER_INTERVAL_ALPHA);
			}
		}
		break;

	case LED_BLINK_WPS:
		if (pLed->bLedOn)
			pLed->BlinkingLedState = RTW_LED_OFF;
		else
			pLed->BlinkingLedState = RTW_LED_ON;
		_set_timer(&(pLed->BlinkTimer), LED_BLINK_SCAN_INTERVAL_ALPHA);
		break;

	case LED_BLINK_WPS_STOP:			/* WPS success */
		if (pLed->BlinkingLedState == RTW_LED_ON) {
			pLed->BlinkingLedState = RTW_LED_OFF;
			_set_timer(&(pLed->BlinkTimer), LED_BLINK_WPS_SUCESS_INTERVAL_ALPHA);
			bStopBlinking = _FALSE;
		} else {
			bStopBlinking = _TRUE;
		}

		if (bStopBlinking) {
			if (rtlpriv->pwrctrlpriv.rf_pwrstate != rf_on) {
				SwLedOff(rtlpriv, pLed);
			} else {
				pLed->bLedLinkBlinkInProgress = _TRUE;
				pLed->CurrLedState = LED_BLINK_NORMAL;
				if (pLed->bLedOn)
					pLed->BlinkingLedState = RTW_LED_OFF;
				else
					pLed->BlinkingLedState = RTW_LED_ON;
				_set_timer(&(pLed->BlinkTimer), LED_BLINK_LINK_INTERVAL_ALPHA);
			}
			pLed->bLedWPSBlinkInProgress = _FALSE;
		}
		break;

	default:
		break;
	}

}

/* page added for Belkin AC950. 20120813 */
static void SwLedBlink9(struct rtl_led *pLed)
{
	struct rtl_priv *rtlpriv = pLed->rtlpriv;
	struct rtl_hal *rtlhal = rtl_hal(rtlpriv);
	struct mlme_priv *pmlmepriv = &(rtlpriv->mlmepriv);
	BOOLEAN bStopBlinking = _FALSE;

	/* Change LED according to BlinkingLedState specified. */
	if (pLed->BlinkingLedState == RTW_LED_ON) {
		SwLedOn(rtlpriv, pLed);
	} else {
		SwLedOff(rtlpriv, pLed);
	}


	switch (pLed->CurrLedState) {
	case RTW_LED_ON:
		SwLedOn(rtlpriv, pLed);
		break;

	case RTW_LED_OFF:
		SwLedOff(rtlpriv, pLed);
		break;

	case LED_BLINK_SLOWLY:
		if (pLed->bLedOn)
			pLed->BlinkingLedState = RTW_LED_OFF;
		else
			pLed->BlinkingLedState = RTW_LED_ON;
		_set_timer(&(pLed->BlinkTimer), LED_BLINK_NO_LINK_INTERVAL_ALPHA);
		break;

	case LED_BLINK_StartToBlink:
		if (pLed->bLedOn) {
			pLed->BlinkingLedState = RTW_LED_OFF;
			_set_timer(&(pLed->BlinkTimer), LED_BLINK_SLOWLY_INTERVAL);
		} else 	{
			pLed->BlinkingLedState = RTW_LED_ON;
			_set_timer(&(pLed->BlinkTimer), LED_BLINK_NORMAL_INTERVAL);
		}
		break;

	case LED_BLINK_SCAN:
		pLed->BlinkTimes--;
		if (pLed->BlinkTimes == 0) {
			bStopBlinking = _TRUE;
		}

		if (bStopBlinking) {
			if (rtlpriv->pwrctrlpriv.rf_pwrstate != rf_on) {
				SwLedOff(rtlpriv, pLed);
			} else if (check_fwstate(pmlmepriv, _FW_LINKED) == _TRUE) {
				pLed->bLedLinkBlinkInProgress = _TRUE;
				pLed->CurrLedState = LED_BLINK_ALWAYS_ON;

				_set_timer(&(pLed->BlinkTimer), LED_BLINK_LINK_INTERVAL_ALPHA);
			} else if (check_fwstate(pmlmepriv, _FW_LINKED) == _FALSE) {
				pLed->bLedNoLinkBlinkInProgress = _TRUE;
				pLed->CurrLedState = LED_BLINK_SLOWLY;
				if (pLed->bLedOn)
					pLed->BlinkingLedState = RTW_LED_OFF;
				else
					pLed->BlinkingLedState = RTW_LED_ON;
				_set_timer(&(pLed->BlinkTimer), LED_BLINK_NO_LINK_INTERVAL_ALPHA);
			}
			pLed->BlinkTimes = 0;
			pLed->bLedBlinkInProgress = _FALSE;
		} else {
			if (rtlpriv->pwrctrlpriv.rf_pwrstate != rf_on && rtlpriv->pwrctrlpriv.rfoff_reason > RF_CHANGE_BY_PS) {
				SwLedOff(rtlpriv, pLed);
			} else {
				if (pLed->bLedOn)
					pLed->BlinkingLedState = RTW_LED_OFF;
				else
					pLed->BlinkingLedState = RTW_LED_ON;
				_set_timer(&(pLed->BlinkTimer), LED_BLINK_SCAN_INTERVAL_ALPHA);
			}
		}
		break;

	case LED_BLINK_TXRX:
		pLed->BlinkTimes--;
		if (pLed->BlinkTimes == 0) {
			bStopBlinking = _TRUE;
		}

		if (bStopBlinking) {
			if (rtlpriv->pwrctrlpriv.rf_pwrstate != rf_on && rtlpriv->pwrctrlpriv.rfoff_reason > RF_CHANGE_BY_PS) {
				SwLedOff(rtlpriv, pLed);
			} else {
				pLed->bLedNoLinkBlinkInProgress = _TRUE;
				pLed->CurrLedState = LED_BLINK_ALWAYS_ON;
				if (pLed->bLedOn)
					pLed->BlinkingLedState = RTW_LED_OFF;
				else
					pLed->BlinkingLedState = RTW_LED_ON;
				_set_timer(&(pLed->BlinkTimer), LED_BLINK_FASTER_INTERVAL_ALPHA);
			}
			pLed->bLedBlinkInProgress = _FALSE;
		} else {
			if (rtlpriv->pwrctrlpriv.rf_pwrstate != rf_on && rtlpriv->pwrctrlpriv.rfoff_reason > RF_CHANGE_BY_PS) {
				SwLedOff(rtlpriv, pLed);
			} else {
				if (pLed->bLedOn)
					pLed->BlinkingLedState = RTW_LED_OFF;
				else
					pLed->BlinkingLedState = RTW_LED_ON;
				_set_timer(&(pLed->BlinkTimer), LED_BLINK_FASTER_INTERVAL_ALPHA);
			}
		}
		break;

	case LED_BLINK_WPS:
		if (pLed->bLedOn) {
			pLed->BlinkingLedState = RTW_LED_OFF;
			_set_timer(&(pLed->BlinkTimer), LED_BLINK_SLOWLY_INTERVAL);
		} else {
			pLed->BlinkingLedState = RTW_LED_ON;
			_set_timer(&(pLed->BlinkTimer), LED_BLINK_NORMAL_INTERVAL);
		}
		break;

	case LED_BLINK_WPS_STOP:		/* WPS authentication fail */
		if (pLed->bLedOn)
			pLed->BlinkingLedState = RTW_LED_OFF;
		else
			pLed->BlinkingLedState = RTW_LED_ON;

		_set_timer(&(pLed->BlinkTimer), LED_BLINK_NORMAL_INTERVAL);
		break;

	case LED_BLINK_WPS_STOP_OVERLAP:		/* WPS session overlap */
		pLed->BlinkTimes--;
		pLed->BlinkCounter--;
		if (pLed->BlinkCounter == 0) {
			pLed->BlinkingLedState = RTW_LED_OFF;
			pLed->CurrLedState = RTW_LED_OFF;
			_set_timer(&(pLed->BlinkTimer), LED_BLINK_NORMAL_INTERVAL);
		} else {
			if (pLed->BlinkTimes == 0) {
				if (pLed->bLedOn) {
					pLed->BlinkTimes = 1;
				} else {
					bStopBlinking = _TRUE;
				}
			}

			if (bStopBlinking) {
				pLed->BlinkTimes = 10;
				pLed->BlinkingLedState = RTW_LED_ON;
				_set_timer(&(pLed->BlinkTimer), LED_BLINK_LINK_INTERVAL_ALPHA);
			} else {
				if (pLed->bLedOn)
					pLed->BlinkingLedState = RTW_LED_OFF;
				else
					pLed->BlinkingLedState = RTW_LED_ON;

				_set_timer(&(pLed->BlinkTimer), LED_BLINK_NORMAL_INTERVAL);
			}
		}
		break;

	case LED_BLINK_ALWAYS_ON:
		pLed->BlinkTimes--;
		if (pLed->BlinkTimes == 0) {
			bStopBlinking = _TRUE;
		}
		if (bStopBlinking) {
			if (rtlpriv->pwrctrlpriv.rf_pwrstate != rf_on && rtlpriv->pwrctrlpriv.rfoff_reason > RF_CHANGE_BY_PS) {
				SwLedOff(rtlpriv, pLed);
			} else {
				if (IS_HARDWARE_TYPE_8812AU(rtlhal)) {
					pLed->BlinkingLedState = RTW_LED_ON;
					pLed->CurrLedState = LED_BLINK_ALWAYS_ON;
				} else {
					pLed->bLedNoLinkBlinkInProgress = _TRUE;
					pLed->CurrLedState = LED_BLINK_SLOWLY;
					if (pLed->bLedOn)
						pLed->BlinkingLedState = RTW_LED_OFF;
					else
						pLed->BlinkingLedState = RTW_LED_ON;
				}
				_set_timer(&(pLed->BlinkTimer), LED_BLINK_NO_LINK_INTERVAL_ALPHA);
			}
			pLed->bLedBlinkInProgress = _FALSE;
		} else {
			if (rtlpriv->pwrctrlpriv.rf_pwrstate != rf_on && rtlpriv->pwrctrlpriv.rfoff_reason > RF_CHANGE_BY_PS) {
				SwLedOff(rtlpriv, pLed);
			} else {
				if (IS_HARDWARE_TYPE_8812AU(rtlhal)) {
					pLed->BlinkingLedState = RTW_LED_ON;
				} else {
					if (pLed->bLedOn)
						pLed->BlinkingLedState = RTW_LED_OFF;
					else
						pLed->BlinkingLedState = RTW_LED_ON;
				}
				_set_timer(&(pLed->BlinkTimer), LED_BLINK_FASTER_INTERVAL_ALPHA);
			}
		}
		break;

	case LED_BLINK_LINK_IN_PROCESS:
		if (pLed->bLedOn) {
			pLed->BlinkingLedState = RTW_LED_OFF;
			_set_timer(&(pLed->BlinkTimer), LED_BLINK_LINK_INTERVAL_ON_BELKIN);
		} else {
			pLed->BlinkingLedState = RTW_LED_ON;
			_set_timer(&(pLed->BlinkTimer), LED_BLINK_LINK_INTERVAL_OFF_BELKIN);
		}
		break;

	case LED_BLINK_AUTH_ERROR:
		pLed->BlinkTimes--;
		if (pLed->BlinkTimes == 0) {
			bStopBlinking = _TRUE;
		}
		if (bStopBlinking == _FALSE) {
			if (pLed->bLedOn) {
				pLed->BlinkingLedState = RTW_LED_OFF;
				_set_timer(&(pLed->BlinkTimer), LED_BLINK_ERROR_INTERVAL_BELKIN);
			} else {
				pLed->BlinkingLedState = RTW_LED_ON;
				_set_timer(&(pLed->BlinkTimer), LED_BLINK_ERROR_INTERVAL_BELKIN);
			}
		} else {
			pLed->CurrLedState = RTW_LED_OFF;
			pLed->BlinkingLedState = RTW_LED_OFF;
			_set_timer(&(pLed->BlinkTimer), LED_BLINK_ERROR_INTERVAL_BELKIN);
		}
		break;

	default:
		break;
	}

}

/* page added for Netgear A6200V2. 20120827 */
static void SwLedBlink10(struct rtl_led *pLed)
{
	struct rtl_priv *rtlpriv = pLed->rtlpriv;
	struct rtl_hal *rtlhal = rtl_hal(rtlpriv);
	struct mlme_priv *pmlmepriv = &(rtlpriv->mlmepriv);
	BOOLEAN bStopBlinking = _FALSE;

	/* Change LED according to BlinkingLedState specified. */
	if (pLed->BlinkingLedState == RTW_LED_ON) {
		SwLedOn(rtlpriv, pLed);
	} else {
		SwLedOff(rtlpriv, pLed);
	}

	switch (pLed->CurrLedState) {
	case RTW_LED_ON:
		SwLedOn(rtlpriv, pLed);
		break;

	case RTW_LED_OFF:
		SwLedOff(rtlpriv, pLed);
		break;

	case LED_BLINK_SLOWLY:
		if (pLed->bLedOn)
			pLed->BlinkingLedState = RTW_LED_OFF;
		else
			pLed->BlinkingLedState = RTW_LED_ON;
		_set_timer(&(pLed->BlinkTimer), LED_BLINK_NO_LINK_INTERVAL_ALPHA);
		break;

	case LED_BLINK_StartToBlink:
		if (pLed->bLedOn) {
			pLed->BlinkingLedState = RTW_LED_OFF;
			_set_timer(&(pLed->BlinkTimer), LED_BLINK_SLOWLY_INTERVAL);
		} else {
			pLed->BlinkingLedState = RTW_LED_ON;
			_set_timer(&(pLed->BlinkTimer), LED_BLINK_NORMAL_INTERVAL);
		}
		break;

	case LED_BLINK_SCAN:
		pLed->BlinkTimes--;
		if (pLed->BlinkTimes == 0) {
			bStopBlinking = _TRUE;
		}

		if (bStopBlinking) {
			if (rtlpriv->pwrctrlpriv.rf_pwrstate != rf_on) 	{
				SwLedOff(rtlpriv, pLed);
			} else if (check_fwstate(pmlmepriv, _FW_LINKED) == _TRUE)	{
				pLed->bLedNoLinkBlinkInProgress = _FALSE;
				pLed->CurrLedState = RTW_LED_OFF;
				pLed->BlinkingLedState = RTW_LED_OFF;

				_set_timer(&(pLed->BlinkTimer), LED_BLINK_NO_LINK_INTERVAL_ALPHA);
			}
			pLed->BlinkTimes = 0;
			pLed->bLedBlinkInProgress = _FALSE;
		} else {
			if (rtlpriv->pwrctrlpriv.rf_pwrstate != rf_on && rtlpriv->pwrctrlpriv.rfoff_reason > RF_CHANGE_BY_PS) {
				SwLedOff(rtlpriv, pLed);
			} else {
				if (pLed->bLedOn) {
					pLed->BlinkingLedState = RTW_LED_OFF;
					_set_timer(&(pLed->BlinkTimer), LED_BLINK_LINK_INTERVAL_NETGEAR);
				} else {
					pLed->BlinkingLedState = RTW_LED_ON;
					_set_timer(&(pLed->BlinkTimer), LED_BLINK_LINK_SLOWLY_INTERVAL_NETGEAR+LED_BLINK_LINK_INTERVAL_NETGEAR);
				}
			}
		}
		break;

	case LED_BLINK_WPS:
		if (pLed->bLedOn) {
			pLed->BlinkingLedState = RTW_LED_OFF;
			_set_timer(&(pLed->BlinkTimer), LED_BLINK_LINK_INTERVAL_NETGEAR);
		} else {
			pLed->BlinkingLedState = RTW_LED_ON;
			_set_timer(&(pLed->BlinkTimer), LED_BLINK_NORMAL_INTERVAL+LED_BLINK_LINK_INTERVAL_NETGEAR);
		}
		break;

	case LED_BLINK_WPS_STOP:		/* WPS authentication fail */
		if (pLed->bLedOn)
			pLed->BlinkingLedState = RTW_LED_OFF;
		else
			pLed->BlinkingLedState = RTW_LED_ON;

		_set_timer(&(pLed->BlinkTimer), LED_BLINK_NORMAL_INTERVAL);
		break;

	case LED_BLINK_WPS_STOP_OVERLAP:	/* WPS session overlap */
		pLed->BlinkTimes--;
		pLed->BlinkCounter--;
		if (pLed->BlinkCounter == 0) {
			pLed->BlinkingLedState = RTW_LED_OFF;
			pLed->CurrLedState = RTW_LED_OFF;
			_set_timer(&(pLed->BlinkTimer), LED_BLINK_NORMAL_INTERVAL);
		} else {
			if (pLed->BlinkTimes == 0) {
				if (pLed->bLedOn) {
					pLed->BlinkTimes = 1;
				} else {
					bStopBlinking = _TRUE;
				}
			}

			if (bStopBlinking) {
				pLed->BlinkTimes = 10;
				pLed->BlinkingLedState = RTW_LED_ON;
				_set_timer(&(pLed->BlinkTimer), LED_BLINK_LINK_INTERVAL_ALPHA);
			} else {
				if (pLed->bLedOn)
					pLed->BlinkingLedState = RTW_LED_OFF;
				else
					pLed->BlinkingLedState = RTW_LED_ON;

				_set_timer(&(pLed->BlinkTimer), LED_BLINK_NORMAL_INTERVAL);
			}
		}
		break;

	case LED_BLINK_ALWAYS_ON:
		pLed->BlinkTimes--;
		if (pLed->BlinkTimes == 0) {
			bStopBlinking = _TRUE;
		}
		if (bStopBlinking) {
			if (rtlpriv->pwrctrlpriv.rf_pwrstate != rf_on && rtlpriv->pwrctrlpriv.rfoff_reason > RF_CHANGE_BY_PS) {
				SwLedOff(rtlpriv, pLed);
			} else {
				if (IS_HARDWARE_TYPE_8812AU(rtlhal)) {
					pLed->BlinkingLedState = RTW_LED_ON;
					pLed->CurrLedState = LED_BLINK_ALWAYS_ON;
				} else {
					pLed->bLedNoLinkBlinkInProgress = _TRUE;
					pLed->CurrLedState = LED_BLINK_SLOWLY;
					if (pLed->bLedOn)
						pLed->BlinkingLedState = RTW_LED_OFF;
					else
						pLed->BlinkingLedState = RTW_LED_ON;
				}
				_set_timer(&(pLed->BlinkTimer), LED_BLINK_NO_LINK_INTERVAL_ALPHA);
			}
			pLed->bLedBlinkInProgress = _FALSE;
		} else {
			if (rtlpriv->pwrctrlpriv.rf_pwrstate != rf_on && rtlpriv->pwrctrlpriv.rfoff_reason > RF_CHANGE_BY_PS) {
				SwLedOff(rtlpriv, pLed);
			} else {
				if (IS_HARDWARE_TYPE_8812AU(rtlhal)) {
					pLed->BlinkingLedState = RTW_LED_ON;
				} else {
					if (pLed->bLedOn)
						pLed->BlinkingLedState = RTW_LED_OFF;
					else
						pLed->BlinkingLedState = RTW_LED_ON;
				}
				_set_timer(&(pLed->BlinkTimer), LED_BLINK_FASTER_INTERVAL_ALPHA);
			}
		}
		break;

	case LED_BLINK_LINK_IN_PROCESS:
		if (pLed->bLedOn) {
			pLed->BlinkingLedState = RTW_LED_OFF;
			_set_timer(&(pLed->BlinkTimer), LED_BLINK_LINK_INTERVAL_ON_BELKIN);
		} else {
			pLed->BlinkingLedState = RTW_LED_ON;
			_set_timer(&(pLed->BlinkTimer), LED_BLINK_LINK_INTERVAL_OFF_BELKIN);
		}
		break;

	case LED_BLINK_AUTH_ERROR:
		pLed->BlinkTimes--;
		if (pLed->BlinkTimes == 0) {
			bStopBlinking = _TRUE;
		}
		if (bStopBlinking == _FALSE) {
			if (pLed->bLedOn) {
				pLed->BlinkingLedState = RTW_LED_OFF;
				_set_timer(&(pLed->BlinkTimer), LED_BLINK_ERROR_INTERVAL_BELKIN);
			} else {
				pLed->BlinkingLedState = RTW_LED_ON;
				_set_timer(&(pLed->BlinkTimer), LED_BLINK_ERROR_INTERVAL_BELKIN);
			}
		} else {
			pLed->CurrLedState = RTW_LED_OFF;
			pLed->BlinkingLedState = RTW_LED_OFF;
			_set_timer(&(pLed->BlinkTimer), LED_BLINK_ERROR_INTERVAL_BELKIN);
		}
		break;

	default:
		break;
	}
}

static void SwLedBlink11(struct rtl_led *pLed)
{
	struct rtl_priv *rtlpriv = pLed->rtlpriv;
	struct mlme_priv *pmlmepriv = &(rtlpriv->mlmepriv);
	BOOLEAN bStopBlinking = _FALSE;

	/* Change LED according to BlinkingLedState specified. */
	if (pLed->BlinkingLedState == RTW_LED_ON) {
		SwLedOn(rtlpriv, pLed);
	} else {
		SwLedOff(rtlpriv, pLed);
	}

	switch (pLed->CurrLedState) {
	case LED_BLINK_TXRX:
		if (rtlpriv->pwrctrlpriv.rf_pwrstate != rf_on && rtlpriv->pwrctrlpriv.rfoff_reason > RF_CHANGE_BY_PS) {
			SwLedOff(rtlpriv, pLed);
		} else {
			if (pLed->bLedOn)
				pLed->BlinkingLedState = RTW_LED_OFF;
			else
				pLed->BlinkingLedState = RTW_LED_ON;
			_set_timer(&(pLed->BlinkTimer), LED_BLINK_SCAN_INTERVAL_ALPHA);
		}

		break;

	case LED_BLINK_WPS:
		if (pLed->BlinkTimes == 5) {
			SwLedOn(rtlpriv, pLed);
			_set_timer(&(pLed->BlinkTimer), LED_CM11_LINK_ON_INTERVEL);
		} else {
			if (pLed->bLedOn) {
				pLed->BlinkingLedState = RTW_LED_OFF;
				_set_timer(&(pLed->BlinkTimer), LED_CM11_BLINK_INTERVAL);
			} else {
				pLed->BlinkingLedState = RTW_LED_ON;
				_set_timer(&(pLed->BlinkTimer), LED_CM11_BLINK_INTERVAL);
			}
		}
		pLed->BlinkTimes--;
		if (pLed->BlinkTimes == 0) {
			bStopBlinking = _TRUE;
		}
		if (bStopBlinking == _TRUE)
			pLed->BlinkTimes = 5;
		break;

	case LED_BLINK_WPS_STOP:		/* WPS authentication fail */
		if (check_fwstate(pmlmepriv, _FW_LINKED) == _TRUE) {
			if (pLed->bLedOn)
				pLed->BlinkingLedState = RTW_LED_OFF;
			else
				pLed->BlinkingLedState = RTW_LED_ON;
			_set_timer(&(pLed->BlinkTimer), LED_BLINK_SCAN_INTERVAL_ALPHA);
		} else {
			pLed->CurrLedState = RTW_LED_ON;
			pLed->BlinkingLedState = RTW_LED_ON;
			SwLedOn(rtlpriv, pLed);
			_set_timer(&(pLed->BlinkTimer), 0);
		}
		break;

	default:
		break;
	}

}

static void SwLedBlink13(IN struct rtl_led *pLed)
{
	struct rtl_priv *rtlpriv = pLed->rtlpriv;
	struct mlme_priv *pmlmepriv = &(rtlpriv->mlmepriv);
	BOOLEAN bStopBlinking = _FALSE;
	static u8 LinkBlinkCnt = 0;

	/* Change LED according to BlinkingLedState specified. */
	if (pLed->BlinkingLedState == RTW_LED_ON) {
		SwLedOn(rtlpriv, pLed);
	} else {
		if (pLed->CurrLedState != LED_BLINK_WPS_STOP)
			SwLedOff(rtlpriv, pLed);
	}

	switch (pLed->CurrLedState) {
	case LED_BLINK_LINK_IN_PROCESS:
		if (!pLed->bLedWPSBlinkInProgress)
			LinkBlinkCnt++;

		if (LinkBlinkCnt > 15) {
			LinkBlinkCnt = 0;
			pLed->bLedBlinkInProgress = _FALSE;
			break;
		}
		if (pLed->bLedOn) {
			pLed->BlinkingLedState = RTW_LED_OFF;
			_set_timer(&(pLed->BlinkTimer), 500);
		} else {
			pLed->BlinkingLedState = RTW_LED_ON;
			_set_timer(&(pLed->BlinkTimer), 500);
		}

		break;

	case LED_BLINK_WPS:
		if (pLed->bLedOn) {
			pLed->BlinkingLedState = RTW_LED_OFF;
			_set_timer(&(pLed->BlinkTimer), LED_WPS_BLINK_ON_INTERVAL_NETGEAR);
		} else {
			pLed->BlinkingLedState = RTW_LED_ON;
			_set_timer(&(pLed->BlinkTimer), LED_WPS_BLINK_OFF_INTERVAL_NETGEAR);
		}

		break;

	case LED_BLINK_WPS_STOP:		/* WPS success */
		SwLedOff(rtlpriv, pLed);
		pLed->bLedWPSBlinkInProgress = _FALSE;
		break;

	default:
		LinkBlinkCnt = 0;
		break;
	}
}

/*
 * 	Description:
 * 		Handler function of LED Blinking.
 * 		We dispatch acture LED blink action according to LedStrategy.
 */

static void BlinkHandler(struct rtl_led *pLed)
{
	struct rtl_priv	*rtlpriv = pLed->rtlpriv;
	struct rtl_usb_priv *usbpriv = rtl_usbpriv(rtlpriv);
	struct rtl_led_ctl *ledpriv = &(usbpriv->ledpriv);

	/* DBG_871X("%s (%s:%d)\n",__FUNCTION__, current->comm, current->pid); */

	if ((rtlpriv->bSurpriseRemoved == _TRUE) || (rtlpriv->bDriverStopped == _TRUE)) {
		/* DBG_871X("%s bSurpriseRemoved:%d, bDriverStopped:%d\n", __FUNCTION__, rtlpriv->bSurpriseRemoved, rtlpriv->bDriverStopped); */
		return;
	}

	switch (ledpriv->LedStrategy) {
	case SW_LED_MODE1:
		SwLedBlink1(pLed);
		break;

	case SW_LED_MODE9:
		SwLedBlink9(pLed);
		break;

	case SW_LED_MODE10:
		SwLedBlink10(pLed);
		break;

	case SW_LED_MODE11:
		SwLedBlink11(pLed);
		break;

	case SW_LED_MODE13:
		SwLedBlink13(pLed);
		break;

	default:
		/*
		 * RT_TRACE(COMP_LED, DBG_LOUD, ("BlinkWorkItemCallback 0x%x \n", pHalData->LedStrategy));
		 * SwLedBlink(pLed);
		 */
		break;
	}
}

/*
 * 	Description:
 * 		Callback function of LED BlinkTimer,
 * 		it just schedules to corresponding BlinkWorkItem/led_blink_hdl
 */
static void BlinkTimerCallback(void *data)
{
	struct rtl_led *pLed = (struct rtl_led *)data;
	struct rtl_priv	*rtlpriv = pLed->rtlpriv;

	/* DBG_871X("%s\n", __FUNCTION__); */

	if ((rtlpriv->bSurpriseRemoved == _TRUE) || (rtlpriv->bDriverStopped == _TRUE)) {
		/* DBG_871X("%s bSurpriseRemoved:%d, bDriverStopped:%d\n", __FUNCTION__, rtlpriv->bSurpriseRemoved, rtlpriv->bDriverStopped); */
		return;
	}

	schedule_work(&(pLed->BlinkWorkItem));
}

/*
 * 	Description:
 * 		Callback function of LED BlinkWorkItem.
 * 		We dispatch acture LED blink action according to LedStrategy.
 */
static void BlinkWorkItemCallback(struct work_struct *work)
{
	struct rtl_led *pLed = container_of(work, struct rtl_led, BlinkWorkItem);
	BlinkHandler(pLed);
}

/* ALPHA, added by chiyoko, 20090106 */
static void SwLedControlMode1(struct rtl_priv *rtlpriv, enum led_ctl_mode LedAction)
{
	struct rtl_hal *rtlhal = rtl_hal(rtlpriv);
	struct rtl_usb_priv *usbpriv = rtl_usbpriv(rtlpriv);
	struct rtl_led_ctl *ledpriv = &(usbpriv->ledpriv);
	struct rtl_led *pLed = &(ledpriv->SwLed0);
	struct mlme_priv *pmlmepriv = &(rtlpriv->mlmepriv);
	EEPROM_EFUSE_PRIV *pEEPROM = GET_EEPROM_EFUSE_PRIV(rtlpriv);

	u32 uLedBlinkNoLinkInterval = LED_BLINK_NO_LINK_INTERVAL_ALPHA; /* add by ylb 20121012 for customer led for alpha */
	if (rtlhal->oem_id == RT_CID_ALPHA_Dlink)
		uLedBlinkNoLinkInterval = LED_BLINK_NO_LINK_INTERVAL_ALPHA_500MS;

	switch (LedAction) {
	case LED_CTL_POWER_ON:
	case LED_CTL_START_TO_LINK:
	case LED_CTL_NO_LINK:
		if (pLed->bLedNoLinkBlinkInProgress == _FALSE) {
			if (pLed->CurrLedState == LED_BLINK_SCAN || IS_LED_WPS_BLINKING(pLed)) {
				return;
			}
			if (pLed->bLedLinkBlinkInProgress == _TRUE) {
				_cancel_timer_ex(&(pLed->BlinkTimer));
				pLed->bLedLinkBlinkInProgress = _FALSE;
			}
			if (pLed->bLedBlinkInProgress == _TRUE) {
				_cancel_timer_ex(&(pLed->BlinkTimer));
				pLed->bLedBlinkInProgress = _FALSE;
			}

			pLed->bLedNoLinkBlinkInProgress = _TRUE;
			pLed->CurrLedState = LED_BLINK_SLOWLY;
			if (pLed->bLedOn)
				pLed->BlinkingLedState = RTW_LED_OFF;
			else
				pLed->BlinkingLedState = RTW_LED_ON;
			_set_timer(&(pLed->BlinkTimer), uLedBlinkNoLinkInterval);	/* change by ylb 20121012 for customer led for alpha */
		}
		break;

	case LED_CTL_LINK:
		if (pLed->bLedLinkBlinkInProgress == _FALSE) {
			if (pLed->CurrLedState == LED_BLINK_SCAN || IS_LED_WPS_BLINKING(pLed)) {
				return;
			}
			if (pLed->bLedNoLinkBlinkInProgress == _TRUE) {
				_cancel_timer_ex(&(pLed->BlinkTimer));
				pLed->bLedNoLinkBlinkInProgress = _FALSE;
			}
			if (pLed->bLedBlinkInProgress == _TRUE) {
				_cancel_timer_ex(&(pLed->BlinkTimer));
				pLed->bLedBlinkInProgress = _FALSE;
			}
			pLed->bLedLinkBlinkInProgress = _TRUE;
			pLed->CurrLedState = LED_BLINK_NORMAL;
			if (pLed->bLedOn)
				pLed->BlinkingLedState = RTW_LED_OFF;
			else
				pLed->BlinkingLedState = RTW_LED_ON;
			_set_timer(&(pLed->BlinkTimer), LED_BLINK_LINK_INTERVAL_ALPHA);
		}
		break;

	case LED_CTL_SITE_SURVEY:
		if ((pmlmepriv->LinkDetectInfo.bBusyTraffic) && (check_fwstate(pmlmepriv, _FW_LINKED) == _TRUE))
			;
		else if (pLed->bLedScanBlinkInProgress == _FALSE) {
			if (IS_LED_WPS_BLINKING(pLed))
				return;

			if (pLed->bLedNoLinkBlinkInProgress == _TRUE) {
				_cancel_timer_ex(&(pLed->BlinkTimer));
				pLed->bLedNoLinkBlinkInProgress = _FALSE;
			}
			if (pLed->bLedLinkBlinkInProgress == _TRUE) {
				_cancel_timer_ex(&(pLed->BlinkTimer));
				 pLed->bLedLinkBlinkInProgress = _FALSE;
			}
			if (pLed->bLedBlinkInProgress == _TRUE) {
				_cancel_timer_ex(&(pLed->BlinkTimer));
				pLed->bLedBlinkInProgress = _FALSE;
			}
			pLed->bLedScanBlinkInProgress = _TRUE;
			pLed->CurrLedState = LED_BLINK_SCAN;
			pLed->BlinkTimes = 24;
			if (pLed->bLedOn)
				pLed->BlinkingLedState = RTW_LED_OFF;
			else
				pLed->BlinkingLedState = RTW_LED_ON;

			if (rtlpriv->pwrctrlpriv.rf_pwrstate != rf_on && rtlpriv->pwrctrlpriv.rfoff_reason == RF_CHANGE_BY_IPS)
				_set_timer(&(pLed->BlinkTimer), LED_INITIAL_INTERVAL);
			else
				_set_timer(&(pLed->BlinkTimer), LED_BLINK_SCAN_INTERVAL_ALPHA);

		 }
		break;

	case LED_CTL_TX:
	case LED_CTL_RX:
		if (pLed->bLedBlinkInProgress == _FALSE) {
			if (pLed->CurrLedState == LED_BLINK_SCAN || IS_LED_WPS_BLINKING(pLed)) {
				return;
			}
			if (pLed->bLedNoLinkBlinkInProgress == _TRUE) {
				_cancel_timer_ex(&(pLed->BlinkTimer));
				pLed->bLedNoLinkBlinkInProgress = _FALSE;
			}
			if (pLed->bLedLinkBlinkInProgress == _TRUE) {
				_cancel_timer_ex(&(pLed->BlinkTimer));
				pLed->bLedLinkBlinkInProgress = _FALSE;
			}
			pLed->bLedBlinkInProgress = _TRUE;
			pLed->CurrLedState = LED_BLINK_TXRX;
			pLed->BlinkTimes = 2;
			if (pLed->bLedOn)
				pLed->BlinkingLedState = RTW_LED_OFF;
			else
				pLed->BlinkingLedState = RTW_LED_ON;
			_set_timer(&(pLed->BlinkTimer), LED_BLINK_FASTER_INTERVAL_ALPHA);
		}
		break;

	case LED_CTL_START_WPS:		/* wait until xinpin finish */
	case LED_CTL_START_WPS_BOTTON:
		 if (pLed->bLedWPSBlinkInProgress == _FALSE) {
			if (pLed->bLedNoLinkBlinkInProgress == _TRUE) {
				_cancel_timer_ex(&(pLed->BlinkTimer));
				pLed->bLedNoLinkBlinkInProgress = _FALSE;
			}
			if (pLed->bLedLinkBlinkInProgress == _TRUE) {
				_cancel_timer_ex(&(pLed->BlinkTimer));
				 pLed->bLedLinkBlinkInProgress = _FALSE;
			}
			if (pLed->bLedBlinkInProgress == _TRUE) {
				_cancel_timer_ex(&(pLed->BlinkTimer));
				pLed->bLedBlinkInProgress = _FALSE;
			}
			if (pLed->bLedScanBlinkInProgress == _TRUE) {
				_cancel_timer_ex(&(pLed->BlinkTimer));
				pLed->bLedScanBlinkInProgress = _FALSE;
			}
			pLed->bLedWPSBlinkInProgress = _TRUE;
			pLed->CurrLedState = LED_BLINK_WPS;
			if (pLed->bLedOn)
				pLed->BlinkingLedState = RTW_LED_OFF;
			else
				pLed->BlinkingLedState = RTW_LED_ON;
			_set_timer(&(pLed->BlinkTimer), LED_BLINK_SCAN_INTERVAL_ALPHA);
		}
		break;


	case LED_CTL_STOP_WPS:
		if (pLed->bLedNoLinkBlinkInProgress == _TRUE) {
			_cancel_timer_ex(&(pLed->BlinkTimer));
			pLed->bLedNoLinkBlinkInProgress = _FALSE;
		}
		if (pLed->bLedLinkBlinkInProgress == _TRUE) {
			_cancel_timer_ex(&(pLed->BlinkTimer));
			 pLed->bLedLinkBlinkInProgress = _FALSE;
		}
		if (pLed->bLedBlinkInProgress == _TRUE) {
			_cancel_timer_ex(&(pLed->BlinkTimer));
			pLed->bLedBlinkInProgress = _FALSE;
		}
		if (pLed->bLedScanBlinkInProgress == _TRUE) {
			_cancel_timer_ex(&(pLed->BlinkTimer));
			pLed->bLedScanBlinkInProgress = _FALSE;
		}
		if (pLed->bLedWPSBlinkInProgress) {
			_cancel_timer_ex(&(pLed->BlinkTimer));
		} else {
			pLed->bLedWPSBlinkInProgress = _TRUE;
		}

		pLed->CurrLedState = LED_BLINK_WPS_STOP;
		if (pLed->bLedOn) {
			pLed->BlinkingLedState = RTW_LED_OFF;
			_set_timer(&(pLed->BlinkTimer), LED_BLINK_WPS_SUCESS_INTERVAL_ALPHA);
		} else {
			pLed->BlinkingLedState = RTW_LED_ON;
			_set_timer(&(pLed->BlinkTimer), 0);
		}
		break;

	case LED_CTL_STOP_WPS_FAIL:
		if (pLed->bLedWPSBlinkInProgress) {
			_cancel_timer_ex(&(pLed->BlinkTimer));
			pLed->bLedWPSBlinkInProgress = _FALSE;
		}

		pLed->bLedNoLinkBlinkInProgress = _TRUE;
		pLed->CurrLedState = LED_BLINK_SLOWLY;
		if (pLed->bLedOn)
			pLed->BlinkingLedState = RTW_LED_OFF;
		else
			pLed->BlinkingLedState = RTW_LED_ON;
		_set_timer(&(pLed->BlinkTimer), uLedBlinkNoLinkInterval);/* change by ylb 20121012 for customer led for alpha */
		break;

	case LED_CTL_POWER_OFF:
		pLed->CurrLedState = RTW_LED_OFF;
		pLed->BlinkingLedState = RTW_LED_OFF;
		if (pLed->bLedNoLinkBlinkInProgress) {
			_cancel_timer_ex(&(pLed->BlinkTimer));
			pLed->bLedNoLinkBlinkInProgress = _FALSE;
		}
		if (pLed->bLedLinkBlinkInProgress) {
			_cancel_timer_ex(&(pLed->BlinkTimer));
			pLed->bLedLinkBlinkInProgress = _FALSE;
		}
		if (pLed->bLedBlinkInProgress) {
			_cancel_timer_ex(&(pLed->BlinkTimer));
			pLed->bLedBlinkInProgress = _FALSE;
		}
		if (pLed->bLedWPSBlinkInProgress) {
			_cancel_timer_ex(&(pLed->BlinkTimer));
			pLed->bLedWPSBlinkInProgress = _FALSE;
		}
		if (pLed->bLedScanBlinkInProgress) {
			_cancel_timer_ex(&(pLed->BlinkTimer));
			pLed->bLedScanBlinkInProgress = _FALSE;
		}

		SwLedOff(rtlpriv, pLed);
		break;

	default:
		break;

	}
}


/* page added for Belkin AC950, 20120813 */
void SwLedControlMode9(struct rtl_priv *rtlpriv, enum led_ctl_mode LedAction)
{
	struct rtl_hal *rtlhal = rtl_hal(rtlpriv);
	struct rtl_usb_priv *usbpriv = rtl_usbpriv(rtlpriv);
	struct rtl_led_ctl *ledpriv = &(usbpriv->ledpriv);
	struct mlme_priv	*pmlmepriv = &rtlpriv->mlmepriv;
	struct rtl_led *pLed = &(ledpriv->SwLed0);
	struct rtl_led *pLed1 = &(ledpriv->SwLed1);
	struct rtl_led *pLed2 = &(ledpriv->SwLed2);
	BOOLEAN  bWPSOverLap = _FALSE;

	switch (LedAction) {
	case LED_CTL_START_TO_LINK:
		if (pLed2->bLedBlinkInProgress == _FALSE) {
			pLed2->bLedBlinkInProgress = _TRUE;
			pLed2->BlinkingLedState = RTW_LED_ON;
			pLed2->CurrLedState = LED_BLINK_LINK_IN_PROCESS;

			_set_timer(&(pLed2->BlinkTimer), 0);
		}
		break;

	case LED_CTL_LINK:
	case LED_CTL_NO_LINK:
		/* LED1 settings */
		if (LedAction == LED_CTL_NO_LINK) {
			/* if (pMgntInfo->AuthStatus == AUTH_STATUS_FAILED) */
			if (0) {
				pLed1->CurrLedState = LED_BLINK_AUTH_ERROR;
					if (pLed1->bLedOn)
						pLed1->BlinkingLedState = RTW_LED_OFF;
					else
						pLed1->BlinkingLedState = RTW_LED_ON;
					_set_timer(&(pLed1->BlinkTimer), 0);
			} else {
				pLed1->CurrLedState = RTW_LED_OFF;
				pLed1->BlinkingLedState = RTW_LED_OFF;
				if (pLed1->bLedOn)
					_set_timer(&(pLed1->BlinkTimer), 0);
			}
		} else {
			pLed1->CurrLedState = RTW_LED_OFF;
			pLed1->BlinkingLedState = RTW_LED_OFF;
			if (pLed1->bLedOn)
				_set_timer(&(pLed1->BlinkTimer), 0);
		}

		/* LED2 settings */
		if (LedAction == LED_CTL_LINK) {
			if (rtlpriv->securitypriv.dot11PrivacyAlgrthm != _NO_PRIVACY_) {
				if (pLed2->bLedBlinkInProgress == _TRUE) {
					_cancel_timer_ex(&(pLed2->BlinkTimer));
					pLed2->bLedBlinkInProgress = _FALSE;
				}
				pLed2->CurrLedState = RTW_LED_ON;
				pLed2->bLedNoLinkBlinkInProgress = _TRUE;
				if (!pLed2->bLedOn)
					_set_timer(&(pLed2->BlinkTimer), 0);
			} else {
				if (pLed2->bLedWPSBlinkInProgress != _TRUE) {
					pLed2->CurrLedState = RTW_LED_OFF;
					pLed2->BlinkingLedState = RTW_LED_OFF;
					if (pLed2->bLedOn)
						_set_timer(&(pLed2->BlinkTimer), 0);
				}
			}
		} else {	/* NO_LINK */
			if (pLed2->bLedWPSBlinkInProgress == _FALSE) {
				pLed2->CurrLedState = RTW_LED_OFF;
				pLed2->BlinkingLedState = RTW_LED_OFF;
				if (pLed2->bLedOn)
					_set_timer(&(pLed2->BlinkTimer), 0);
			}
		}

		/* LED0 settings */
		if (pLed->bLedNoLinkBlinkInProgress == _FALSE) {
			if (pLed->CurrLedState == LED_BLINK_SCAN || IS_LED_WPS_BLINKING(pLed)) {
				return;
			}
			if (pLed->bLedBlinkInProgress == _TRUE) {
				_cancel_timer_ex(&(pLed->BlinkTimer));
				pLed->bLedBlinkInProgress = _FALSE;
			}

			pLed->bLedNoLinkBlinkInProgress = _TRUE;
			if (IS_HARDWARE_TYPE_8812AU(rtlhal)) {
				if (LedAction == LED_CTL_LINK) {
					pLed->BlinkingLedState = RTW_LED_ON;
					pLed->CurrLedState = LED_BLINK_ALWAYS_ON;
				} else {
					pLed->CurrLedState = LED_BLINK_SLOWLY;
					if (pLed->bLedOn)
						pLed->BlinkingLedState = RTW_LED_OFF;
					else
						pLed->BlinkingLedState = RTW_LED_ON;
				}
			} else {
				pLed->CurrLedState = LED_BLINK_SLOWLY;
				if (pLed->bLedOn)
					pLed->BlinkingLedState = RTW_LED_OFF;
				else
					pLed->BlinkingLedState = RTW_LED_ON;
			}
			_set_timer(&(pLed->BlinkTimer), LED_BLINK_NO_LINK_INTERVAL_ALPHA);
		}

		break;

	case LED_CTL_SITE_SURVEY:
		 if ((pmlmepriv->LinkDetectInfo.bBusyTraffic) && (check_fwstate(pmlmepriv, _FW_LINKED) == _TRUE))
			;
		 else { 	/* if (pLed->bLedScanBlinkInProgress ==FALSE) */
			if (IS_LED_WPS_BLINKING(pLed))
				return;

			if (pLed->bLedNoLinkBlinkInProgress == _TRUE) {
				_cancel_timer_ex(&(pLed->BlinkTimer));
				pLed->bLedNoLinkBlinkInProgress = _FALSE;
			}
			if (pLed->bLedBlinkInProgress == _TRUE) {
				_cancel_timer_ex(&(pLed->BlinkTimer));
				pLed->bLedBlinkInProgress = _FALSE;
			}
			pLed->bLedScanBlinkInProgress = _TRUE;
				pLed->CurrLedState = LED_BLINK_SCAN;
			pLed->BlinkTimes = 24;
			if (pLed->bLedOn)
				pLed->BlinkingLedState = RTW_LED_OFF;
			else
				pLed->BlinkingLedState = RTW_LED_ON;
			_set_timer(&(pLed->BlinkTimer), LED_BLINK_SCAN_INTERVAL_ALPHA);

		}
		break;

	case LED_CTL_TX:
	case LED_CTL_RX:
		if (pLed->bLedBlinkInProgress == _FALSE)	{
			if (pLed->CurrLedState == LED_BLINK_SCAN || IS_LED_WPS_BLINKING(pLed)) {
				return;
			}
			if (pLed->bLedNoLinkBlinkInProgress == _TRUE) {
				_cancel_timer_ex(&(pLed->BlinkTimer));
				pLed->bLedNoLinkBlinkInProgress = _FALSE;
			}
			pLed->bLedBlinkInProgress = _TRUE;
			pLed->CurrLedState = LED_BLINK_TXRX;
			pLed->BlinkTimes = 2;
			if (pLed->bLedOn)
				pLed->BlinkingLedState = RTW_LED_OFF;
			else
				pLed->BlinkingLedState = RTW_LED_ON;
			_set_timer(&(pLed->BlinkTimer), LED_BLINK_FASTER_INTERVAL_ALPHA);
		}
		break;

	case LED_CTL_START_WPS: /* wait until xinpin finish */
	case LED_CTL_START_WPS_BOTTON:
		pLed2->bLedBlinkInProgress = _TRUE;
		pLed2->BlinkingLedState = RTW_LED_ON;
		pLed2->CurrLedState = LED_BLINK_LINK_IN_PROCESS;
		pLed2->bLedWPSBlinkInProgress = _TRUE;

		_set_timer(&(pLed2->BlinkTimer), 0);
		break;

	case LED_CTL_STOP_WPS:	/* WPS connect success */
		/* LED2 settings */
		if (pLed2->bLedWPSBlinkInProgress == _TRUE) {
			_cancel_timer_ex(&(pLed2->BlinkTimer));
			pLed2->bLedBlinkInProgress = _FALSE;
			pLed2->bLedWPSBlinkInProgress = _FALSE;
		}
		pLed2->CurrLedState = RTW_LED_ON;
		pLed2->bLedNoLinkBlinkInProgress = _TRUE;
		if (!pLed2->bLedOn)
			_set_timer(&(pLed2->BlinkTimer), 0);

		/* LED1 settings */
		_cancel_timer_ex(&(pLed1->BlinkTimer));
		pLed1->CurrLedState = RTW_LED_OFF;
		pLed1->BlinkingLedState = RTW_LED_OFF;
		if (pLed1->bLedOn)
			_set_timer(&(pLed1->BlinkTimer), 0);


		break;

	case LED_CTL_STOP_WPS_FAIL:	/* WPS authentication fail */
		/* LED1 settings */
		if (bWPSOverLap == _FALSE) {
			pLed1->CurrLedState = LED_BLINK_AUTH_ERROR;
			pLed1->BlinkTimes = 50;
			if (pLed1->bLedOn)
				pLed1->BlinkingLedState = RTW_LED_OFF;
			else
				pLed1->BlinkingLedState = RTW_LED_ON;
			_set_timer(&(pLed1->BlinkTimer), 0);
		} else {
			bWPSOverLap = _FALSE;
			pLed1->CurrLedState = RTW_LED_OFF;
			pLed1->BlinkingLedState = RTW_LED_OFF;
			_set_timer(&(pLed1->BlinkTimer), 0);
		}

		/* LED2 settings */
		pLed2->CurrLedState = RTW_LED_OFF;
		pLed2->BlinkingLedState = RTW_LED_OFF;
		pLed2->bLedWPSBlinkInProgress = _FALSE;
		if (pLed2->bLedOn)
			_set_timer(&(pLed2->BlinkTimer), 0);

		break;

	case LED_CTL_STOP_WPS_FAIL_OVERLAP:	/* WPS session overlap */
		/* LED1 settings */
		bWPSOverLap = _TRUE;
		pLed1->CurrLedState = LED_BLINK_WPS_STOP_OVERLAP;
		pLed1->BlinkTimes = 10;
		pLed1->BlinkCounter = 50;
		if (pLed1->bLedOn)
			pLed1->BlinkingLedState = RTW_LED_OFF;
		else
			pLed1->BlinkingLedState = RTW_LED_ON;
		_set_timer(&(pLed1->BlinkTimer), 0);

		/* LED2 settings */
		pLed2->CurrLedState = RTW_LED_OFF;
		pLed2->BlinkingLedState = RTW_LED_OFF;
		pLed2->bLedWPSBlinkInProgress = _FALSE;
		if (pLed2->bLedOn)
			_set_timer(&(pLed2->BlinkTimer), 0);

		break;

	case LED_CTL_POWER_OFF:
		pLed->CurrLedState = RTW_LED_OFF;
		pLed->BlinkingLedState = RTW_LED_OFF;

		if (pLed->bLedNoLinkBlinkInProgress) {
			_cancel_timer_ex(&(pLed->BlinkTimer));
			pLed->bLedNoLinkBlinkInProgress = _FALSE;
		}
		if (pLed->bLedLinkBlinkInProgress) {
			_cancel_timer_ex(&(pLed->BlinkTimer));
			pLed->bLedLinkBlinkInProgress = _FALSE;
		}
		if (pLed->bLedBlinkInProgress) {
			_cancel_timer_ex(&(pLed->BlinkTimer));
			pLed->bLedBlinkInProgress = _FALSE;
		}
		if (pLed->bLedWPSBlinkInProgress) {
			_cancel_timer_ex(&(pLed->BlinkTimer));
			pLed->bLedWPSBlinkInProgress = _FALSE;
		}
		if (pLed->bLedScanBlinkInProgress) {
			_cancel_timer_ex(&(pLed->BlinkTimer));
			pLed->bLedScanBlinkInProgress = _FALSE;
		}
		if (pLed->bLedStartToLinkBlinkInProgress) {
			_cancel_timer_ex(&(pLed->BlinkTimer));
			pLed->bLedStartToLinkBlinkInProgress = _FALSE;
		}

		if (pLed1->bLedWPSBlinkInProgress) {
			_cancel_timer_ex(&(pLed1->BlinkTimer));
			pLed1->bLedWPSBlinkInProgress = _FALSE;
		}


		pLed1->BlinkingLedState = LED_UNKNOWN;
		SwLedOff(rtlpriv, pLed);
		SwLedOff(rtlpriv, pLed1);
		break;

	case LED_CTL_CONNECTION_NO_TRANSFER:
		 if (pLed->bLedBlinkInProgress == _FALSE) {
			if (pLed->bLedNoLinkBlinkInProgress == _TRUE) {
				_cancel_timer_ex(&(pLed->BlinkTimer));
				pLed->bLedNoLinkBlinkInProgress = _FALSE;
			}
			pLed->bLedBlinkInProgress = _TRUE;

			pLed->CurrLedState = LED_BLINK_ALWAYS_ON;
			pLed->BlinkingLedState = RTW_LED_ON;
			_set_timer(&(pLed->BlinkTimer), LED_BLINK_NO_LINK_INTERVAL_ALPHA);
		 }
		break;

	default:
		break;

	}
}

/* page added for Netgear A6200V2, 20120827 */
void SwLedControlMode10(struct rtl_priv *rtlpriv, enum led_ctl_mode LedAction)
{
	struct _rtw_hal	*pHalData = GET_HAL_DATA(rtlpriv);
	struct rtl_usb_priv *usbpriv = rtl_usbpriv(rtlpriv);
	struct rtl_led_ctl *ledpriv = &(usbpriv->ledpriv);
	struct mlme_priv	*pmlmepriv = &rtlpriv->mlmepriv;
	struct rtl_led *pLed = &(ledpriv->SwLed0);
	struct rtl_led *pLed1 = &(ledpriv->SwLed1);

	switch (LedAction) {
	case LED_CTL_START_TO_LINK:
		if (pLed1->bLedBlinkInProgress == _FALSE) {
			pLed1->bLedBlinkInProgress = _TRUE;
			pLed1->BlinkingLedState = RTW_LED_ON;
			pLed1->CurrLedState = LED_BLINK_LINK_IN_PROCESS;

			_set_timer(&(pLed1->BlinkTimer), 0);
		}
		break;

	case LED_CTL_LINK:
	case LED_CTL_NO_LINK:
		if (LedAction == LED_CTL_LINK) {
			if (pLed->bLedWPSBlinkInProgress == _TRUE || pLed1->bLedWPSBlinkInProgress == _TRUE)
				;
			else {
				if (rtlpriv->rtlhal.current_bandtype == BAND_ON_2_4G) { 		/* LED0 settings */
					pLed->CurrLedState = RTW_LED_ON;
					pLed->BlinkingLedState = RTW_LED_ON;
					if (pLed->bLedBlinkInProgress == _TRUE) {
						_cancel_timer_ex(&(pLed->BlinkTimer));
						pLed->bLedBlinkInProgress = _FALSE;
					}
					_set_timer(&(pLed->BlinkTimer), 0);

					pLed1->CurrLedState = RTW_LED_OFF;
					pLed1->BlinkingLedState = RTW_LED_OFF;
					_set_timer(&(pLed1->BlinkTimer), 0);
				} else if (rtlpriv->rtlhal.current_bandtype == BAND_ON_5G) {		/* LED1 settings */
					pLed1->CurrLedState = RTW_LED_ON;
					pLed1->BlinkingLedState = RTW_LED_ON;
					if (pLed1->bLedBlinkInProgress == _TRUE) {
						_cancel_timer_ex(&(pLed1->BlinkTimer));
						pLed1->bLedBlinkInProgress = _FALSE;
					}
					_set_timer(&(pLed1->BlinkTimer), 0);

					pLed->CurrLedState = RTW_LED_OFF;
					pLed->BlinkingLedState = RTW_LED_OFF;
					_set_timer(&(pLed->BlinkTimer), 0);
				}
			}
		} else if (LedAction == LED_CTL_NO_LINK) {  /* TODO by page */
			if (pLed->bLedWPSBlinkInProgress == _TRUE || pLed1->bLedWPSBlinkInProgress == _TRUE)
				;
			else {
				pLed->CurrLedState = RTW_LED_OFF;
				pLed->BlinkingLedState = RTW_LED_OFF;
				if (pLed->bLedOn)
					_set_timer(&(pLed->BlinkTimer), 0);

				pLed1->CurrLedState = RTW_LED_OFF;
				pLed1->BlinkingLedState = RTW_LED_OFF;
				if (pLed1->bLedOn)
					_set_timer(&(pLed1->BlinkTimer), 0);
			}
		}

		break;

	case LED_CTL_SITE_SURVEY:
		 if (check_fwstate(pmlmepriv, _FW_LINKED) == _TRUE)
			;	/* don't blink when media connect */
		 else {	/* if (pLed->bLedScanBlinkInProgress ==FALSE) */
			if (IS_LED_WPS_BLINKING(pLed) || IS_LED_WPS_BLINKING(pLed1))
				return;

			if (pLed->bLedNoLinkBlinkInProgress == _TRUE) {
				_cancel_timer_ex(&(pLed->BlinkTimer));
				pLed->bLedNoLinkBlinkInProgress = _FALSE;
			}
			if (pLed->bLedBlinkInProgress == _TRUE) {
				_cancel_timer_ex(&(pLed->BlinkTimer));
				pLed->bLedBlinkInProgress = _FALSE;
			}
			pLed->bLedScanBlinkInProgress = _TRUE;
			pLed->CurrLedState = LED_BLINK_SCAN;
			pLed->BlinkTimes = 12;
			pLed->BlinkingLedState = LED_BLINK_SCAN;
			_set_timer(&(pLed->BlinkTimer), 0);

			if (pLed1->bLedNoLinkBlinkInProgress == _TRUE) {
				_cancel_timer_ex(&(pLed1->BlinkTimer));
				pLed1->bLedNoLinkBlinkInProgress = _FALSE;
			}
			if (pLed1->bLedBlinkInProgress == _TRUE) {
				_cancel_timer_ex(&(pLed1->BlinkTimer));
				pLed1->bLedBlinkInProgress = _FALSE;
			}
			pLed1->bLedScanBlinkInProgress = _TRUE;
			pLed1->CurrLedState = LED_BLINK_SCAN;
			pLed1->BlinkTimes = 12;
			pLed1->BlinkingLedState = LED_BLINK_SCAN;
			_set_timer(&(pLed1->BlinkTimer), LED_BLINK_LINK_SLOWLY_INTERVAL_NETGEAR);

		}
		break;

	case LED_CTL_START_WPS: /* wait until xinpin finish */
	case LED_CTL_START_WPS_BOTTON:
		/* LED0 settings */
		if (pLed->bLedBlinkInProgress == _FALSE) {
			pLed->bLedBlinkInProgress = _TRUE;
			pLed->bLedWPSBlinkInProgress = _TRUE;
			pLed->BlinkingLedState = LED_BLINK_WPS;
			pLed->CurrLedState = LED_BLINK_WPS;
			_set_timer(&(pLed->BlinkTimer), 0);
		}

		/* LED1 settings */
		if (pLed1->bLedBlinkInProgress == _FALSE) {
			pLed1->bLedBlinkInProgress = _TRUE;
			pLed1->bLedWPSBlinkInProgress = _TRUE;
			pLed1->BlinkingLedState = LED_BLINK_WPS;
			pLed1->CurrLedState = LED_BLINK_WPS;
			_set_timer(&(pLed1->BlinkTimer), LED_BLINK_NORMAL_INTERVAL+LED_BLINK_LINK_INTERVAL_NETGEAR);
		}
		break;

	case LED_CTL_STOP_WPS:	/* WPS connect success */
		if (rtlpriv->rtlhal.current_bandtype == BAND_ON_2_4G) {	/* LED0 settings */
			pLed->bLedWPSBlinkInProgress = _FALSE;
			pLed->CurrLedState = RTW_LED_ON;
			pLed->BlinkingLedState = RTW_LED_ON;
			if (pLed->bLedBlinkInProgress == _TRUE) {
				_cancel_timer_ex(&(pLed->BlinkTimer));
				pLed->bLedBlinkInProgress = _FALSE;
			}
			_set_timer(&(pLed->BlinkTimer), 0);

			pLed1->CurrLedState = RTW_LED_OFF;
			pLed1->BlinkingLedState = RTW_LED_OFF;
			_set_timer(&(pLed1->BlinkTimer), 0);
		} else if (rtlpriv->rtlhal.current_bandtype == BAND_ON_5G) {	/* LED1 settings */
			pLed1->bLedWPSBlinkInProgress = _FALSE;
			pLed1->CurrLedState = RTW_LED_ON;
			pLed1->BlinkingLedState = RTW_LED_ON;
			if (pLed1->bLedBlinkInProgress == _TRUE) {
				_cancel_timer_ex(&(pLed1->BlinkTimer));
				pLed1->bLedBlinkInProgress = _FALSE;
			}
			_set_timer(&(pLed1->BlinkTimer), 0);

			pLed->CurrLedState = RTW_LED_OFF;
			pLed->BlinkingLedState = RTW_LED_OFF;
			_set_timer(&(pLed->BlinkTimer), 0);
		}

		break;

	case LED_CTL_STOP_WPS_FAIL:		/* WPS authentication fail */
		/* LED1 settings */
		pLed1->bLedWPSBlinkInProgress = _FALSE;
		pLed1->CurrLedState = RTW_LED_OFF;
		pLed1->BlinkingLedState = RTW_LED_OFF;
		_set_timer(&(pLed1->BlinkTimer), 0);

		/* LED0 settings */
		pLed->bLedWPSBlinkInProgress = _FALSE;
		pLed->CurrLedState = RTW_LED_OFF;
		pLed->BlinkingLedState = RTW_LED_OFF;
		if (pLed->bLedOn)
			_set_timer(&(pLed->BlinkTimer), 0);

		break;


	default:
		break;

	}
}

/* Edimax-ASUS, added by Page, 20121221 */
void SwLedControlMode11(struct rtl_priv *rtlpriv, enum led_ctl_mode LedAction)
{
	struct rtl_usb_priv *usbpriv = rtl_usbpriv(rtlpriv);
	struct rtl_led_ctl *ledpriv = &(usbpriv->ledpriv);
	struct mlme_priv	*pmlmepriv = &rtlpriv->mlmepriv;
	struct rtl_led *pLed = &(ledpriv->SwLed0);

	switch (LedAction) {
	case LED_CTL_POWER_ON:
	case LED_CTL_START_TO_LINK:
	case LED_CTL_NO_LINK:
		pLed->CurrLedState = RTW_LED_ON;
		pLed->BlinkingLedState = RTW_LED_ON;
		_set_timer(&(pLed->BlinkTimer), 0);
		break;

	case LED_CTL_LINK:
		if (pLed->bLedBlinkInProgress == _TRUE) {
			_cancel_timer_ex(&(pLed->BlinkTimer));
			pLed->bLedBlinkInProgress = _FALSE;
		}
		pLed->bLedBlinkInProgress = _TRUE;
		pLed->CurrLedState = LED_BLINK_TXRX;
		if (pLed->bLedOn)
			pLed->BlinkingLedState = RTW_LED_OFF;
		else
			pLed->BlinkingLedState = RTW_LED_ON;
		_set_timer(&(pLed->BlinkTimer), LED_BLINK_SCAN_INTERVAL_ALPHA);
		break;

	case LED_CTL_START_WPS: /* wait until xinpin finish */
	case LED_CTL_START_WPS_BOTTON:
		if (pLed->bLedBlinkInProgress == _TRUE) {
			_cancel_timer_ex(&(pLed->BlinkTimer));
			pLed->bLedBlinkInProgress = _FALSE;
		}
		pLed->bLedWPSBlinkInProgress = _TRUE;
		pLed->bLedBlinkInProgress = _TRUE;
		pLed->CurrLedState = LED_BLINK_WPS;
		if (pLed->bLedOn)
			pLed->BlinkingLedState = RTW_LED_OFF;
		else
			pLed->BlinkingLedState = RTW_LED_ON;
		pLed->BlinkTimes = 5;
		_set_timer(&(pLed->BlinkTimer), 0);

		break;


	case LED_CTL_STOP_WPS:
	case LED_CTL_STOP_WPS_FAIL:
		if (pLed->bLedBlinkInProgress == _TRUE) {
			_cancel_timer_ex(&(pLed->BlinkTimer));
			pLed->bLedBlinkInProgress = _FALSE;
		}
		pLed->CurrLedState = LED_BLINK_WPS_STOP;
		_set_timer(&(pLed->BlinkTimer), 0);
		break;

	case LED_CTL_POWER_OFF:
		pLed->CurrLedState = RTW_LED_OFF;
		pLed->BlinkingLedState = RTW_LED_OFF;

		if (pLed->bLedNoLinkBlinkInProgress) {
			_cancel_timer_ex(&(pLed->BlinkTimer));
			pLed->bLedNoLinkBlinkInProgress = _FALSE;
		}
		if (pLed->bLedLinkBlinkInProgress) {
			_cancel_timer_ex(&(pLed->BlinkTimer));
			pLed->bLedLinkBlinkInProgress = _FALSE;
		}
		if (pLed->bLedBlinkInProgress) {
			_cancel_timer_ex(&(pLed->BlinkTimer));
			pLed->bLedBlinkInProgress = _FALSE;
		}
		if (pLed->bLedWPSBlinkInProgress) {
			_cancel_timer_ex(&(pLed->BlinkTimer));
			pLed->bLedWPSBlinkInProgress = _FALSE;
		}
		if (pLed->bLedScanBlinkInProgress) {
			_cancel_timer_ex(&(pLed->BlinkTimer));
			pLed->bLedScanBlinkInProgress = _FALSE;
		}

		SwLedOff(rtlpriv, pLed);
		break;

	default:
		break;

	}
}


/* Maddest add for NETGEAR R6100 */

static void SwLedControlMode13(struct rtl_priv *rtlpriv, enum led_ctl_mode LedAction)
{
	struct rtl_usb_priv *usbpriv = rtl_usbpriv(rtlpriv);
	struct rtl_led_ctl *ledpriv = &(usbpriv->ledpriv);
	struct mlme_priv	*pmlmepriv = &rtlpriv->mlmepriv;
	struct rtl_led *pLed = &(ledpriv->SwLed0);

	switch (LedAction) {
	case LED_CTL_LINK:
		if (pLed->bLedWPSBlinkInProgress) {
			return;
		}

		pLed->CurrLedState = RTW_LED_ON;
		pLed->BlinkingLedState = RTW_LED_ON;
		if (pLed->bLedBlinkInProgress) {
			_cancel_timer_ex(&(pLed->BlinkTimer));
			pLed->bLedBlinkInProgress = _FALSE;
		}

		if (pLed->bLedScanBlinkInProgress) {
			_cancel_timer_ex(&(pLed->BlinkTimer));
			pLed->bLedScanBlinkInProgress = _FALSE;
		}

		_set_timer(&(pLed->BlinkTimer), 0);
		break;

	case LED_CTL_START_WPS: /* wait until xinpin finish */
	case LED_CTL_START_WPS_BOTTON:
		if (pLed->bLedWPSBlinkInProgress == _FALSE) {
			if (pLed->bLedBlinkInProgress == _TRUE) {
				_cancel_timer_ex(&(pLed->BlinkTimer));
				pLed->bLedBlinkInProgress = _FALSE;
			}
			if (pLed->bLedScanBlinkInProgress == _TRUE) {
				_cancel_timer_ex(&(pLed->BlinkTimer));
				pLed->bLedScanBlinkInProgress = _FALSE;
			}
			pLed->bLedWPSBlinkInProgress = _TRUE;
			pLed->CurrLedState = LED_BLINK_WPS;
			if (pLed->bLedOn) {
				pLed->BlinkingLedState = RTW_LED_OFF;
				_set_timer(&(pLed->BlinkTimer), LED_WPS_BLINK_OFF_INTERVAL_NETGEAR);
			} else {
				pLed->BlinkingLedState = RTW_LED_ON;
				_set_timer(&(pLed->BlinkTimer), LED_WPS_BLINK_ON_INTERVAL_NETGEAR);
			}
		}
		break;

	case LED_CTL_STOP_WPS:
		if (pLed->bLedWPSBlinkInProgress) {
			_cancel_timer_ex(&(pLed->BlinkTimer));
			pLed->bLedWPSBlinkInProgress = _FALSE;
		} else {
			pLed->bLedWPSBlinkInProgress = _TRUE;
		}

		pLed->bLedWPSBlinkInProgress = _FALSE;
		pLed->CurrLedState = LED_BLINK_WPS_STOP;
		if (pLed->bLedOn) {
			pLed->BlinkingLedState = RTW_LED_OFF;

			_set_timer(&(pLed->BlinkTimer), 0);
		}

		break;


	case LED_CTL_STOP_WPS_FAIL:
	case LED_CTL_STOP_WPS_FAIL_OVERLAP: /* WPS session overlap */
		if (pLed->bLedWPSBlinkInProgress) {
			_cancel_timer_ex(&(pLed->BlinkTimer));
			pLed->bLedWPSBlinkInProgress = _FALSE;
		}

		pLed->CurrLedState = RTW_LED_OFF;
		pLed->BlinkingLedState = RTW_LED_OFF;
		_set_timer(&(pLed->BlinkTimer), 0);
		break;

	case LED_CTL_START_TO_LINK:
		if ((pLed->bLedBlinkInProgress == _FALSE) && (pLed->bLedWPSBlinkInProgress == _FALSE)) {
			pLed->bLedBlinkInProgress = _TRUE;
			pLed->BlinkingLedState = RTW_LED_ON;
			pLed->CurrLedState = LED_BLINK_LINK_IN_PROCESS;

			_set_timer(&(pLed->BlinkTimer), 0);
		}
		break;

	case LED_CTL_NO_LINK:
		if (pLed->bLedWPSBlinkInProgress) {
			return;
		}
		if (pLed->bLedBlinkInProgress) {
			_cancel_timer_ex(&(pLed->BlinkTimer));
			pLed->bLedBlinkInProgress = _FALSE;
		}
		if (pLed->bLedScanBlinkInProgress) {
			_cancel_timer_ex(&(pLed->BlinkTimer));
			pLed->bLedScanBlinkInProgress = _FALSE;
		}
		/* if (!IS_LED_BLINKING(pLed)) */
		{
			pLed->CurrLedState = RTW_LED_OFF;
			pLed->BlinkingLedState = RTW_LED_OFF;
			_set_timer(&(pLed->BlinkTimer), 0);
		}
		break;

	case LED_CTL_POWER_OFF:
	case LED_CTL_POWER_ON:
		pLed->CurrLedState = RTW_LED_OFF;
		pLed->BlinkingLedState = RTW_LED_OFF;
		if (pLed->bLedBlinkInProgress) {
			_cancel_timer_ex(&(pLed->BlinkTimer));
			pLed->bLedBlinkInProgress = _FALSE;
		}
		if (pLed->bLedScanBlinkInProgress) {
			_cancel_timer_ex(&(pLed->BlinkTimer));
			pLed->bLedScanBlinkInProgress = _FALSE;
		}
		if (pLed->bLedWPSBlinkInProgress) {
			_cancel_timer_ex(&(pLed->BlinkTimer));
			pLed->bLedWPSBlinkInProgress = _FALSE;
		}

		if (LedAction == LED_CTL_POWER_ON)
			_set_timer(&(pLed->BlinkTimer), 0);
		else
			SwLedOff(rtlpriv, pLed);
		break;

	default:
		break;

	}


}

void rtl8821au_led_control(struct rtl_priv *rtlpriv, enum led_ctl_mode LedAction)
{
	struct rtl_usb_priv *usbpriv = rtl_usbpriv(rtlpriv);
	struct rtl_led_ctl *ledpriv = &(usbpriv->ledpriv);

	if ((rtlpriv->bSurpriseRemoved == _TRUE) || (rtlpriv->hw_init_completed == _FALSE)) {
		return;
	}

	/* if (priv->bInHctTest)
	 * 	return;
	 */

	if ((rtlpriv->pwrctrlpriv.rf_pwrstate != rf_on &&
		rtlpriv->pwrctrlpriv.rfoff_reason > RF_CHANGE_BY_PS) &&
		(LedAction == LED_CTL_TX || LedAction == LED_CTL_RX ||
		 LedAction == LED_CTL_SITE_SURVEY ||
		 LedAction == LED_CTL_LINK ||
		 LedAction == LED_CTL_NO_LINK ||
		 LedAction == LED_CTL_POWER_ON)) {
		return;
	}

	switch (ledpriv->LedStrategy) {
	case SW_LED_MODE1:
		SwLedControlMode1(rtlpriv, LedAction);
		break;

	case SW_LED_MODE9:
		SwLedControlMode9(rtlpriv, LedAction);
		break;

	case SW_LED_MODE10:
		SwLedControlMode10(rtlpriv, LedAction);
		break;

	case SW_LED_MODE11:
		SwLedControlMode11(rtlpriv, LedAction);
		break;

	case SW_LED_MODE13:
		SwLedControlMode13(rtlpriv, LedAction);
		break;

	default:
		break;
	}
}

/*
 * 	Description:
 * 		Reset status of LED_871x object.
 */
static void ResetLedStatus(struct rtl_led *pLed)
{
	pLed->CurrLedState = RTW_LED_OFF;	/* Current LED state. */
	pLed->bLedOn = _FALSE; 			/* true if LED is ON, false if LED is OFF. */

	pLed->bLedBlinkInProgress = _FALSE;	/* true if it is blinking, false o.w.. */
	pLed->bLedWPSBlinkInProgress = _FALSE;

	pLed->BlinkTimes = 0; 			/* Number of times to toggle led state for blinking. */
	pLed->BlinkCounter = 0;
	pLed->BlinkingLedState = LED_UNKNOWN; 	/* Next state for blinking, either RTW_LED_ON or RTW_LED_OFF are. */

	pLed->bLedNoLinkBlinkInProgress = _FALSE;
	pLed->bLedLinkBlinkInProgress = _FALSE;
	pLed->bLedStartToLinkBlinkInProgress = _FALSE;
	pLed->bLedScanBlinkInProgress = _FALSE;
}

/*
 * 	Description:
 * 		Initialize an LED_871x object.
 */
static void InitLed(struct rtl_priv *rtlpriv,	struct rtl_led *pLed, enum rtl_led_pin LedPin)
{
	pLed->rtlpriv = rtlpriv;
	pLed->LedPin = LedPin;

	ResetLedStatus(pLed);

	_init_timer(&(pLed->BlinkTimer), rtlpriv->ndev, BlinkTimerCallback, pLed);

	INIT_WORK(&(pLed->BlinkWorkItem), BlinkWorkItemCallback);
}


/*
 * 	Description:
 * 		DeInitialize an LED_871x object.
 */
static void DeInitLed(struct rtl_led *pLed)
{
	cancel_work_sync(&(pLed->BlinkWorkItem));
	_cancel_timer_ex(&(pLed->BlinkTimer));
	ResetLedStatus(pLed);
}

static void SwLedOn_8812AU(struct rtl_priv *rtlpriv, struct rtl_led *pLed)
{
	uint8_t	LedCfg;
	 struct _rtw_hal	*pHalData = GET_HAL_DATA(rtlpriv);

	if ((rtlpriv->bSurpriseRemoved == _TRUE) || (rtlpriv->bDriverStopped == _TRUE)) {
		return;
	}

	if (RT_GetInterfaceSelection(rtlpriv) == INTF_SEL2_MINICARD
	 || RT_GetInterfaceSelection(rtlpriv) == INTF_SEL3_USB_Solo
	 || RT_GetInterfaceSelection(rtlpriv) == INTF_SEL4_USB_Combo) {
		LedCfg = rtl_read_byte(rtlpriv, REG_LEDCFG2);
		switch (pLed->LedPin) {
		case LED_PIN_GPIO0:
			break;

		case LED_PIN_LED0:
			LedCfg = rtl_read_byte(rtlpriv, REG_LEDCFG2);
			rtl_write_byte(rtlpriv, REG_LEDCFG2, (LedCfg&0xf0)|BIT5|BIT6); /* SW control led0 on. */
			break;

		case LED_PIN_LED1:
			rtl_write_byte(rtlpriv, REG_LEDCFG2, (LedCfg&0x0f)|BIT5); /* SW control led1 on. */
			break;

		default:
			break;
		}
	} else {
		switch (pLed->LedPin) {
		case LED_PIN_GPIO0:
			break;

		case LED_PIN_LED0:
			if (pHalData->AntDivCfg == 0) {
				LedCfg = rtl_read_byte(rtlpriv, REG_LEDCFG0);
				rtl_write_byte(rtlpriv, REG_LEDCFG0, (LedCfg&0x70)|BIT5); /* SW control led0 on. */
			} else {
				LedCfg = rtl_read_byte(rtlpriv, REG_LEDCFG2);
				rtl_write_byte(rtlpriv, REG_LEDCFG2, (LedCfg&0xe0)|BIT7|BIT6|BIT5); /* SW control led0 on. */
			}
			break;

		case LED_PIN_LED1:
			LedCfg = rtl_read_byte(rtlpriv, (REG_LEDCFG1));
			rtl_write_byte(rtlpriv, (REG_LEDCFG1), (LedCfg&0x70)|BIT5); /* SW control led1 on. */
			break;

		case LED_PIN_LED2:
			LedCfg = rtl_read_byte(rtlpriv, (REG_LEDCFG2));
			rtl_write_byte(rtlpriv, (REG_LEDCFG2), (LedCfg&0x70)|BIT5); /* SW control led1 on. */
			break;

		default:
			break;
		}
	}

	pLed->bLedOn = _TRUE;
}

/*
 * 	Description:
 * 	Turn off LED according to LedPin specified.
 */

static void SwLedOff_8812AU(struct rtl_priv *rtlpriv, struct rtl_led *pLed)
{
	uint8_t	LedCfg;
	 struct _rtw_hal	*pHalData = GET_HAL_DATA(rtlpriv);

	if (rtlpriv->bSurpriseRemoved == _TRUE) {
		return;
	}

	if (RT_GetInterfaceSelection(rtlpriv) == INTF_SEL2_MINICARD
	 || RT_GetInterfaceSelection(rtlpriv) == INTF_SEL3_USB_Solo
	 || RT_GetInterfaceSelection(rtlpriv) == INTF_SEL4_USB_Combo) {
		LedCfg = rtl_read_byte(rtlpriv, REG_LEDCFG2);

		/*
		 * 2009/10/23 MH Issau eed to move the LED GPIO from bit  0 to bit3.
		 * 2009/10/26 MH Issau if tyhe device is 8c DID is 0x8176, we need to enable bit6 to
		 * enable GPIO8 for controlling LED.
		 * 2010/07/02 Supprt Open-drain arrangement for controlling the LED. Added by Roger.
		 */
		switch (pLed->LedPin) {
		case LED_PIN_GPIO0:
			break;

		case LED_PIN_LED0:
			if (pHalData->bLedOpenDrain == _TRUE) {
				LedCfg &= 0x90; 	/* Set to software control. */
				rtl_write_byte(rtlpriv, REG_LEDCFG2, (LedCfg|BIT3));
				LedCfg = rtl_read_byte(rtlpriv, REG_MAC_PINMUX_CFG);
				LedCfg &= 0xFE;
				rtl_write_byte(rtlpriv, REG_MAC_PINMUX_CFG, LedCfg);
			} else {
				rtl_write_byte(rtlpriv, REG_LEDCFG2, (LedCfg|BIT3|BIT5|BIT6));
			}
			break;

		case LED_PIN_LED1:
			LedCfg &= 0x0f; 	/* Set to software control. */
			rtl_write_byte(rtlpriv, REG_LEDCFG2, (LedCfg|BIT3));
			break;

		default:
			break;
		}
	} else {
		switch (pLed->LedPin) {
		case LED_PIN_GPIO0:
			break;

		case LED_PIN_LED0:
			 if (pHalData->AntDivCfg == 0) {
				LedCfg = rtl_read_byte(rtlpriv, REG_LEDCFG0);
				LedCfg &= 0x70; 	/* Set to software control. */
				rtl_write_byte(rtlpriv, REG_LEDCFG0, (LedCfg|BIT3|BIT5));
			} else {
				LedCfg = rtl_read_byte(rtlpriv, REG_LEDCFG2);
				LedCfg &= 0xe0; 	/* Set to software control. */
				rtl_write_byte(rtlpriv, REG_LEDCFG2, (LedCfg|BIT3|BIT7|BIT6|BIT5));
			}
			break;

		case LED_PIN_LED1:
			LedCfg = rtl_read_byte(rtlpriv, REG_LEDCFG1);
			LedCfg &= 0x70; 	/* Set to software control. */
			rtl_write_byte(rtlpriv, REG_LEDCFG1, (LedCfg|BIT3|BIT5));
			break;

		case LED_PIN_LED2:
			LedCfg = rtl_read_byte(rtlpriv, REG_LEDCFG2);
			LedCfg &= 0x70; 	/* Set to software control. */
			rtl_write_byte(rtlpriv, REG_LEDCFG2, (LedCfg|BIT3|BIT5));
			break;

		default:
			break;
		}
	}

	pLed->bLedOn = _FALSE;
}

/*
 * Description:
 * Turn on LED according to LedPin specified.
 */

static void SwLedOn_8821AU(struct rtl_priv *rtlpriv, struct rtl_led *pLed)
{
	uint8_t	LedCfg;
	struct rtl_hal *rtlhal = rtl_hal(rtlpriv);

	if ((rtlpriv->bSurpriseRemoved == _TRUE) || (rtlpriv->bDriverStopped == _TRUE)) {
		return;
	}

	if (RT_GetInterfaceSelection(rtlpriv) == INTF_SEL2_MINICARD
	 || RT_GetInterfaceSelection(rtlpriv) == INTF_SEL3_USB_Solo
	 || RT_GetInterfaceSelection(rtlpriv) == INTF_SEL4_USB_Combo) {
		LedCfg = rtl_read_byte(rtlpriv, REG_LEDCFG2);
		switch (pLed->LedPin) {
		case LED_PIN_GPIO0:
			break;

		case LED_PIN_LED0:

			LedCfg = rtl_read_byte(rtlpriv, REG_LEDCFG2);
			rtl_write_byte(rtlpriv, REG_LEDCFG2, (LedCfg&0xf0)|BIT5|BIT6); /* SW control led0 on. */
			break;

		case LED_PIN_LED1:
			rtl_write_byte(rtlpriv, REG_LEDCFG2, (LedCfg&0x0f)|BIT5); /* SW control led1 on. */
			break;

		default:
			break;

		}
	} else {
		switch (pLed->LedPin) {
		case LED_PIN_GPIO0:
			break;

		case LED_PIN_LED0:
		case LED_PIN_LED1:
		case LED_PIN_LED2:
			if (IS_HARDWARE_TYPE_8821U(rtlhal)) {
				LedCfg = rtl_read_byte(rtlpriv, REG_LEDCFG2);
				rtl_write_byte(rtlpriv, REG_LEDCFG2, ((LedCfg&0x20) & (~BIT3))|BIT5); /* SW control led0 on. */
			}

			break;

		default:
			break;
		}
	}
	pLed->bLedOn = _TRUE;
}


/*
 * Description:
 * Turn off LED according to LedPin specified.
 */

static void SwLedOff_8821AU(struct rtl_priv *rtlpriv, struct rtl_led *pLed)
{
	struct rtl_hal *rtlhal = rtl_hal(rtlpriv);
	struct _rtw_hal *pHalData = GET_HAL_DATA(rtlpriv);
	uint8_t	LedCfg;

	if (rtlpriv->bSurpriseRemoved == _TRUE) {
		return;
	}

	if (RT_GetInterfaceSelection(rtlpriv) == INTF_SEL2_MINICARD
	 || RT_GetInterfaceSelection(rtlpriv) == INTF_SEL3_USB_Solo
	 || RT_GetInterfaceSelection(rtlpriv) == INTF_SEL4_USB_Combo) {
		LedCfg = rtl_read_byte(rtlpriv, REG_LEDCFG2);

		/*
		 * 2009/10/23 MH Issau eed to move the LED GPIO from bit  0 to bit3.
		 * 2009/10/26 MH Issau if tyhe device is 8c DID is 0x8176, we need to enable bit6 to
		 * enable GPIO8 for controlling LED.
		 * 2010/07/02 Supprt Open-drain arrangement for controlling the LED. Added by Roger.
		 */

		switch (pLed->LedPin) {
		case LED_PIN_GPIO0:
			break;

		case LED_PIN_LED0:
			if (pHalData->bLedOpenDrain == _TRUE) {
				LedCfg &= 0x90; /* Set to software control. */
				rtl_write_byte(rtlpriv, REG_LEDCFG2, (LedCfg|BIT3));
				LedCfg = rtl_read_byte(rtlpriv, REG_MAC_PINMUX_CFG);
				LedCfg &= 0xFE;
				rtl_write_byte(rtlpriv, REG_MAC_PINMUX_CFG, LedCfg);
			} else {
				rtl_write_byte(rtlpriv, REG_LEDCFG2, (LedCfg|BIT3|BIT5|BIT6));
			}
			break;

		case LED_PIN_LED1:
			LedCfg &= 0x0f; /* Set to software control. */
			rtl_write_byte(rtlpriv, REG_LEDCFG2, (LedCfg|BIT3));
			break;

		default:
			break;
		}
	} else {
		switch (pLed->LedPin) {
		case LED_PIN_GPIO0:
			break;

		case LED_PIN_LED0:
		case LED_PIN_LED1:
		case LED_PIN_LED2:
			 if (IS_HARDWARE_TYPE_8821U(rtlhal)) {
				LedCfg = rtl_read_byte(rtlpriv, REG_LEDCFG2);
				LedCfg &= 0x20; 	/* Set to software control. */
				rtl_write_byte(rtlpriv, REG_LEDCFG2, (LedCfg|BIT3|BIT5));
			 }

			break;


		default:
			break;
		}
	}

	pLed->bLedOn = _FALSE;
}

/*
 * Description:
 * Initialize all LED_871x objects.
 */
void rtl8821au_init_sw_leds(struct rtl_priv *rtlpriv)
{
	struct rtl_hal *rtlhal = rtl_hal(rtlpriv);
	struct rtl_usb_priv *usbpriv = rtl_usbpriv(rtlpriv);
	struct rtl_led_ctl *ledpriv = &(usbpriv->ledpriv);

	if (IS_HARDWARE_TYPE_8812(rtlhal)) {
		ledpriv->SwLedOn = SwLedOn_8812AU;
		ledpriv->SwLedOff = SwLedOff_8812AU;
	} else {
		ledpriv->SwLedOn = SwLedOn_8821AU;
		ledpriv->SwLedOff = SwLedOff_8821AU;
	}

	InitLed(rtlpriv, &(ledpriv->SwLed0), LED_PIN_LED0);
	InitLed(rtlpriv, &(ledpriv->SwLed1), LED_PIN_LED1);
	InitLed(rtlpriv, &(ledpriv->SwLed2), LED_PIN_LED2);
}

/*
 * Description:
 * DeInitialize all LED_819xUsb objects.
 */
void rtl8812au_deinit_sw_leds(struct rtl_priv *rtlpriv)
{
	struct rtl_usb_priv *usbpriv = rtl_usbpriv(rtlpriv);
	struct rtl_led_ctl *ledpriv = &(usbpriv->ledpriv);

	DeInitLed(&(ledpriv->SwLed0));
	DeInitLed(&(ledpriv->SwLed1));
	DeInitLed(&(ledpriv->SwLed2));
}


