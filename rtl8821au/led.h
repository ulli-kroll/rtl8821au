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

#ifndef __RTL8821AU_LED_H__
#define __RTL8821AU_LED_H__

//================================================================================
//	LED Behavior Constant.
//================================================================================
// Default LED behavior.
//
#define LED_BLINK_NORMAL_INTERVAL	100
#define LED_BLINK_SLOWLY_INTERVAL	200
#define LED_BLINK_LONG_INTERVAL	400
#define LED_INITIAL_INTERVAL		1800

// LED Customerization

//ALPHA
#define LED_BLINK_NO_LINK_INTERVAL_ALPHA	1000
#define LED_BLINK_NO_LINK_INTERVAL_ALPHA_500MS 500 //add by ylb 20121012 for customer led for alpha
#define LED_BLINK_LINK_INTERVAL_ALPHA		500	//500
#define LED_BLINK_SCAN_INTERVAL_ALPHA		180 	//150
#define LED_BLINK_FASTER_INTERVAL_ALPHA		50
#define LED_BLINK_WPS_SUCESS_INTERVAL_ALPHA	5000

// 111122 by hpfan: Customized for Xavi
#define LED_CM11_BLINK_INTERVAL			300
#define LED_CM11_LINK_ON_INTERVEL		3000

//Netgear
#define LED_BLINK_LINK_INTERVAL_NETGEAR		500
#define LED_BLINK_LINK_SLOWLY_INTERVAL_NETGEAR		1000

#define LED_WPS_BLINK_OFF_INTERVAL_NETGEAR		100
#define LED_WPS_BLINK_ON_INTERVAL_NETGEAR		500

//Belkin AC950
#define LED_BLINK_LINK_INTERVAL_ON_BELKIN		200
#define LED_BLINK_LINK_INTERVAL_OFF_BELKIN		100
#define LED_BLINK_ERROR_INTERVAL_BELKIN		100

//================================================================================
// LED object.
//================================================================================

enum led_ctl_mode {
	LED_CTL_POWER_ON = 1,
	LED_CTL_LINK = 2,
	LED_CTL_NO_LINK = 3,
	LED_CTL_TX = 4,
	LED_CTL_RX = 5,
	LED_CTL_SITE_SURVEY = 6,
	LED_CTL_POWER_OFF = 7,
	LED_CTL_START_TO_LINK = 8,
	LED_CTL_START_WPS = 9,
	LED_CTL_STOP_WPS = 10,
	LED_CTL_START_WPS_BOTTON = 11, //added for runtop
	LED_CTL_STOP_WPS_FAIL = 12, //added for ALPHA
	LED_CTL_STOP_WPS_FAIL_OVERLAP = 13, //added for BELKIN
	LED_CTL_CONNECTION_NO_TRANSFER = 14,
};

enum LED_STATE{
	LED_UNKNOWN = 0,
	RTW_LED_ON = 1,
	RTW_LED_OFF = 2,
	LED_BLINK_NORMAL = 3,
	LED_BLINK_SLOWLY = 4,
	LED_BLINK_POWER_ON = 5,
	LED_BLINK_SCAN = 6, 	// LED is blinking during scanning period, the # of times to blink is depend on time for scanning.
	LED_BLINK_NO_LINK = 7, // LED is blinking during no link state.
	LED_BLINK_StartToBlink = 8, // Customzied for Sercomm Printer Server case
	LED_BLINK_TXRX = 9,
	LED_BLINK_WPS = 10,	// LED is blinkg during WPS communication
	LED_BLINK_WPS_STOP = 11,	//for ALPHA
	LED_BLINK_WPS_STOP_OVERLAP = 12,	//for BELKIN
	LED_BLINK_RUNTOP = 13, 	// Customized for RunTop
	LED_BLINK_CAMEO = 14,
	LED_BLINK_XAVI = 15,
	LED_BLINK_ALWAYS_ON = 16,
	LED_BLINK_LINK_IN_PROCESS = 17,  //Customized for Belkin AC950
	LED_BLINK_AUTH_ERROR = 18,  //Customized for Belkin AC950
	LED_BLINK_Azurewave_5Mbps = 19,
	LED_BLINK_Azurewave_10Mbps = 20,
	LED_BLINK_Azurewave_20Mbps = 21,
	LED_BLINK_Azurewave_40Mbps = 22,
	LED_BLINK_Azurewave_80Mbps = 23,
	LED_BLINK_Azurewave_MAXMbps = 24,
};

enum rtl_led_pin {
	LED_PIN_GPIO0,
	LED_PIN_LED0,
	LED_PIN_LED1,
	LED_PIN_LED2
};


//================================================================================
// PCIE LED Definition.
//================================================================================

#define IS_LED_WPS_BLINKING(_LED)	((_LED)->CurrLedState==LED_BLINK_WPS \
					|| (_LED)->CurrLedState==LED_BLINK_WPS_STOP \
					|| (_LED)->bLedWPSBlinkInProgress)

#define IS_LED_BLINKING(_LED) 	((_LED)->bLedWPSBlinkInProgress \
				||(_LED)->bLedScanBlinkInProgress)


enum LED_STRATEGY {
	SW_LED_MODE1 = 1, // 2 LEDs, through LED0 and LED1. For ALPHA.
	SW_LED_MODE9 = 9, //for Belkin AC950
	SW_LED_MODE10 = 10, //for Netgear A6200V2
	SW_LED_MODE11 = 11, //for Edimax / ASUS
	SW_LED_MODE13 = 13, //for Netgear A6100, 8811Au
};


struct rtl_led {
	struct rtl_priv *		rtlpriv;

	enum rtl_led_pin		LedPin;	// Identify how to implement this SW led.

	enum LED_STATE			CurrLedState; // Current LED state.
	BOOLEAN				bLedOn; // TRUE if LED is ON, FALSE if LED is OFF.

	BOOLEAN				bSWLedCtrl;

	BOOLEAN				bLedBlinkInProgress; // TRUE if it is blinking, FALSE o.w..
	// ALPHA, added by chiyoko, 20090106
	BOOLEAN				bLedNoLinkBlinkInProgress;
	BOOLEAN				bLedLinkBlinkInProgress;
	BOOLEAN				bLedStartToLinkBlinkInProgress;
	BOOLEAN				bLedScanBlinkInProgress;
	BOOLEAN				bLedWPSBlinkInProgress;

	u32					BlinkTimes; // Number of times to toggle led state for blinking.
	uint8_t					BlinkCounter; //Added for turn off overlap led after blinking a while, by page, 20120821
	enum LED_STATE			BlinkingLedState; // Next state for blinking, either LED_ON or LED_OFF are.

	_timer				BlinkTimer; // Timer object for led blinking.

	struct work_struct			BlinkWorkItem; // Workitem used by BlinkTimer to manipulate H/W to blink LED.
};

struct rtl_led_ctl {
	/* add for led controll */
	struct rtl_led SwLed0;
	struct rtl_led SwLed1;
	struct rtl_led SwLed2;
	enum LED_STRATEGY LedStrategy;
	void (*SwLedOn)(struct rtl_priv *rtlpriv, struct rtl_led *pLed);
	void (*SwLedOff)(struct rtl_priv *rtlpriv, struct rtl_led *pLed);
	/* add for led controll */
};

//hal...

//================================================================================
// Interface to manipulate LED objects.
//================================================================================

void rtl8821au_init_sw_leds(struct rtl_priv *rtlpriv);
void rtl8812au_deinit_sw_leds(struct rtl_priv *rtlpriv);
void rtl8821au_led_control(struct rtl_priv *rtlpriv, enum led_ctl_mode LedAction);

#endif
