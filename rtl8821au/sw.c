#include "sw.h"

#include "dm.h"
#include "phy.h"
#include "reg.h"
#include "trx.h"
#include "hw.h"
#include "led.h"


void rtl8812_free_hal_data(struct rtl_priv *rtlpriv);
void UpdateHalRAMask8812A(struct rtl_priv *rtlpriv, uint32_t mac_id, uint8_t rssi_level);
void rtl8812_EfusePowerSwitch(struct rtl_priv *rtlpriv, uint8_t bWrite, uint8_t PwrState);
void rtl8812_ReadEFuse(struct rtl_priv *rtlpriv, uint8_t efuseType, u16 _offset, u16 	_size_byte, uint8_t *pbuf);
void rtl8812_EFUSE_GetEfuseDefinition(struct rtl_priv *rtlpriv, uint8_t efuseType, uint8_t type, void *pOut);
u16 rtl8812_EfuseGetCurrentSize(struct rtl_priv *rtlpriv, uint8_t efuseType);
int rtl8812_Efuse_PgPacketRead(struct rtl_priv *rtlpriv, uint8_t offset, uint8_t *data);
int rtl8812_Efuse_PgPacketWrite(struct rtl_priv *rtlpriv, uint8_t offset, uint8_t word_en, uint8_t *data);
u8 rtl8812_Efuse_WordEnableDataWrite(struct rtl_priv *rtlpriv, u16 efuse_addr, uint8_t word_en, uint8_t *data);
void rtl8812_SetHalODMVar(struct rtl_priv *rtlpriv, HAL_ODM_VARIABLE eVariable,	void *	pValue1, BOOLEAN bSet);
void hal_notch_filter_8812(struct rtl_priv *rtlpriv, bool enable);

static struct rtl_hal_ops rtl8821au_hal_ops = {
	/*
	 * New HAL functions with struct net_device  as first param
	 * this can be (hopefully)switched to struct ieee80211_hw
	 */

	.get_bbreg = 	rtl8821au_phy_query_bb_reg,
	.set_bbreg = 	rtl8821au_phy_set_bb_reg,
	.get_rfreg = 	rtl8821au_phy_query_rf_reg,
	.set_rfreg = 	rtl8821au_phy_set_rf_reg,

	.init_sw_leds = rtl8821au_init_sw_leds,
	.deinit_sw_leds = rtl8812au_deinit_sw_leds,
	.led_control	= rtl8821au_led_control,

	/** ** */

	.fill_fake_txdesc	= rtl8821au_fill_fake_txdesc,

	/* Old HAL functions */

	.hal_init =	 	rtl8812au_hal_init,
	.hal_deinit = 		rtl8812au_hal_deinit,

	/* .free_hal_data = rtl8192c_free_hal_data, */

	.inirp_init =		rtl8812au_inirp_init,
	.inirp_deinit =		rtl8812au_inirp_deinit,

	.init_xmit_priv =	rtl8812au_init_xmit_priv,
	.free_xmit_priv =	rtl8812au_free_xmit_priv,

	.init_recv_priv =	rtl8812au_init_recv_priv,
	.free_recv_priv =	rtl8812au_free_recv_priv,
	.init_default_value =	rtl8812au_init_default_value,
	.intf_chip_configure =	rtl8812au_interface_configure,
	.read_adapter_info =	_rtl8821au_read_adapter_info,

	/* .set_bwmode_handler = 	PHY_SetBWMode8192C; */
	/* .set_channel_handler = 	PHY_SwChnl8192C; */

	/* .hal_dm_watchdog = 	rtl8192c_HalDmWatchDog; */


	.set_hw_reg =	 	rtl8821au_set_hw_reg,
	.get_hw_reg = 		rtl8821au_get_hw_reg,
	.GetHalDefVarHandler = 	rtl8821au_get_hal_def_var,
	.SetHalDefVarHandler = 	rtl8821au_set_hal_def_var,

	.SetBeaconRelatedRegistersHandler = 	rtl8821au_set_beacon_related_registers,

	/* .Add_RateATid = &rtl8192c_Add_RateATid, */

	.hal_xmit = 		rtl8812au_hal_xmit,
	.mgnt_xmit = 		rtl8812au_mgnt_xmit,
	.hal_xmitframe_enqueue = 	rtl8812au_hal_xmitframe_enqueue,

	.free_hal_data =	rtl8812_free_hal_data,

	.dm_init =		rtl8812_init_dm_priv,
	.dm_deinit =		rtl8812_deinit_dm_priv,

	.UpdateRAMaskHandler =	UpdateHalRAMask8812A,

	.read_chip_version =	rtl8821au_read_chip_version,

	.set_bwmode_handler =	PHY_SetBWMode8812,
	.set_channel_handler =	PHY_SwChnl8812,
	.set_chnl_bw_handler =	PHY_SetSwChnlBWMode8812,

	.hal_dm_watchdog =	rtl8821au_dm_watchdog,

	.Add_RateATid =		rtl8812_Add_RateATid,
#ifdef CONFIG_ANTENNA_DIVERSITY
	.AntDivBeforeLinkHandler =	AntDivBeforeLink8812,
	.AntDivCompareHandler =		AntDivCompare8812,
#endif

	/* Efuse related function */
	.EfusePowerSwitch =	rtl8812_EfusePowerSwitch,
	.ReadEFuse =		rtl8812_ReadEFuse,
	.EFUSEGetEfuseDefinition =	rtl8812_EFUSE_GetEfuseDefinition,
	.EfuseGetCurrentSize =	rtl8812_EfuseGetCurrentSize,
	.Efuse_PgPacketRead =	rtl8812_Efuse_PgPacketRead,
	.Efuse_PgPacketWrite =	rtl8812_Efuse_PgPacketWrite,
	.Efuse_WordEnableDataWrite = &rtl8812_Efuse_WordEnableDataWrite,

	.SetHalODMVarHandler = rtl8812_SetHalODMVar,
	.hal_notch_filter = hal_notch_filter_8812,
};

static struct rtl_hal_cfg rtl8821au_hal_cfg = {
	.name = "rtl8821au",
	.fw_name = "rtlwifi/rtl8821aufw.bin",	/* ULLI note two files */
	.ops = &rtl8821au_hal_ops,
};



extern int rtw_ht_enable;
extern int rtw_bw_mode;
extern int rtw_ampdu_enable;	/* for enable tx_ampdu */

static int rtw_drv_init(struct usb_interface *pusb_intf,const struct usb_device_id *pdid);
static void rtw_dev_remove(struct usb_interface *pusb_intf);

#define USB_VENDER_ID_REALTEK		0x0BDA


/* DID_USB_v916_20130116 */
static struct usb_device_id rtw_usb_id_tbl[] ={
	/* RTL8812AU */
	/*=== Realtek demoboard ===*/
	{USB_DEVICE(USB_VENDER_ID_REALTEK, 0x8812),.driver_info = RTL8812},/* Default ID */
	{USB_DEVICE(USB_VENDER_ID_REALTEK, 0x881A),.driver_info = RTL8812},/* Default ID */
	{USB_DEVICE(USB_VENDER_ID_REALTEK, 0x881B),.driver_info = RTL8812},/* Default ID */
	{USB_DEVICE(USB_VENDER_ID_REALTEK, 0x881C),.driver_info = RTL8812},/* Default ID */
	/*=== Customer ID ===*/
	{USB_DEVICE(0x050D, 0x1109),.driver_info = RTL8812}, /* Belkin F9L1109 - SerComm */
	{USB_DEVICE(0x2001, 0x330E),.driver_info = RTL8812}, /* D-Link - ALPHA */
	{USB_DEVICE(0x7392, 0xA822),.driver_info = RTL8812}, /* Edimax - Edimax */
	{USB_DEVICE(0x0DF6, 0x0074),.driver_info = RTL8812}, /* Sitecom - Edimax */
	{USB_DEVICE(0x04BB, 0x0952),.driver_info = RTL8812}, /* I-O DATA - Edimax */
	{USB_DEVICE(0x0789, 0x016E),.driver_info = RTL8812}, /* Logitec - Edimax */
	{USB_DEVICE(0x0409, 0x0408),.driver_info = RTL8812}, /* NEC - */
	{USB_DEVICE(0x0B05, 0x17D2),.driver_info = RTL8812}, /* ASUS - Edimax */
	{USB_DEVICE(0x0E66, 0x0022),.driver_info = RTL8812}, /* HAWKING - Edimax */
	{USB_DEVICE(0x0586, 0x3426),.driver_info = RTL8812}, /* ZyXEL - */
	{USB_DEVICE(0x2001, 0x3313),.driver_info = RTL8812}, /* D-Link - ALPHA */
	{USB_DEVICE(0x1058, 0x0632),.driver_info = RTL8812}, /* WD - Cybertan*/
	{USB_DEVICE(0x1740, 0x0100),.driver_info = RTL8812}, /* EnGenius - EnGenius */
	{USB_DEVICE(0x2019, 0xAB30),.driver_info = RTL8812}, /* Planex - Abocom */
	{USB_DEVICE(0x07B8, 0x8812),.driver_info = RTL8812}, /* Abocom - Abocom */
	{USB_DEVICE(0x2001, 0x3315),.driver_info = RTL8812}, /* D-Link - Cameo */
	{USB_DEVICE(0x2001, 0x3316),.driver_info = RTL8812}, /* D-Link - Cameo */
	{USB_DEVICE(0x20F4, 0x805B),.driver_info = RTL8812}, /* TRENDnet - Cameo */
	{USB_DEVICE(0x13B1, 0x003F),.driver_info = RTL8812}, /* Linksys - SerComm */
	{USB_DEVICE(0x2357, 0x0101),.driver_info = RTL8812}, /* TP-Link - T4U */
	{USB_DEVICE(0x0BDA, 0x8812),.driver_info = RTL8812}, /* Alfa AWUS036AC */

	/* RTL8821AU */
        /*=== Realtek demoboard ===*/
	{USB_DEVICE(USB_VENDER_ID_REALTEK, 0x0811),.driver_info = RTL8821},/* Default ID */
	{USB_DEVICE(USB_VENDER_ID_REALTEK, 0x0821),.driver_info = RTL8821},/* Default ID */
	{USB_DEVICE(USB_VENDER_ID_REALTEK, 0x8822),.driver_info = RTL8821},/* Default ID */
	/*=== Customer ID ===*/
	{USB_DEVICE(0x7392, 0xA811),.driver_info = RTL8821}, /* Edimax - Edimax */
	{USB_DEVICE(0x7392, 0xA812),.driver_info = RTL8821}, /* Edimax - Edimax */
	{USB_DEVICE(0x2001, 0x3314),.driver_info = RTL8821}, /* D-Link - Cameo */
	{USB_DEVICE(0x0846, 0x9052),.driver_info = RTL8821}, /* Netgear - A6100 */
	{USB_DEVICE(0x0411, 0x0242),.driver_info = RTL8821}, /* BUFFALO - Edimax */
	{}	/* Terminating entry */
};

MODULE_DEVICE_TABLE(usb, rtw_usb_id_tbl);

extern void rtw_usb_if1_deinit(struct rtl_priv *rtlpriv);
extern void usb_dvobj_deinit(struct usb_interface *usb_intf);


static int rtl8821au_probe(struct usb_interface *pusb_intf, const struct usb_device_id *pdid)
{
	return rtw_usb_probe(pusb_intf, pdid, &rtl8821au_hal_cfg);
}


static struct usb_driver rtl8821au_usb_drv = {
	.name = "rtl8821au",
	.probe = rtl8821au_probe,
	.disconnect = rtw_usb_disconnect,
	.id_table = rtw_usb_id_tbl,
#if 0
	.suspend =  rtl8821au_suspend,
	.resume = rtl8821au_resume,
  	.reset_resume   = rtl8821au_resume,
#endif
#ifdef CONFIG_AUTOSUSPEND
	.supports_autosuspend = 1,
#endif

};

module_usb_driver(rtl8821au_usb_drv)

