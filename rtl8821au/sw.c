#include "sw.h"

#include "phy.h"
#include "reg.h"
#include "trx.h"
#include "hw.h"
#include "led.h"


void rtl8812_free_hal_data(struct rtl_priv *rtlpriv);
void UpdateHalRAMask8812A(struct rtl_priv *rtlpriv, uint32_t mac_id, uint8_t rssi_level);
void ReadChipVersion8812A(struct rtl_priv *rtlpriv);
VOID rtl8812_EfusePowerSwitch(struct rtl_priv *rtlpriv, uint8_t bWrite, uint8_t PwrState);
VOID rtl8812_ReadEFuse(struct rtl_priv *rtlpriv, uint8_t efuseType, u16 _offset, u16 	_size_byte, uint8_t *pbuf);
VOID rtl8812_EFUSE_GetEfuseDefinition(struct rtl_priv *rtlpriv, uint8_t efuseType, uint8_t type, void *pOut);
u16 rtl8812_EfuseGetCurrentSize(struct rtl_priv *rtlpriv, uint8_t efuseType);
int rtl8812_Efuse_PgPacketRead(struct rtl_priv *rtlpriv, uint8_t offset, uint8_t *data);
int rtl8812_Efuse_PgPacketWrite(struct rtl_priv *rtlpriv, uint8_t offset, uint8_t word_en, uint8_t *data);
u8 rtl8812_Efuse_WordEnableDataWrite(struct rtl_priv *rtlpriv, u16 efuse_addr, uint8_t word_en, uint8_t *data);
void rtl8812_GetHalODMVar(struct rtl_priv *rtlpriv, HAL_ODM_VARIABLE eVariable, PVOID pValue1, BOOLEAN bSet);
void rtl8812_SetHalODMVar(struct rtl_priv *rtlpriv, HAL_ODM_VARIABLE eVariable,	PVOID 	pValue1, BOOLEAN bSet);
void hal_notch_filter_8812(struct rtl_priv *rtlpriv, bool enable);



struct rtl_hal_ops rtl8821au_hal_ops = {
	/*
	 * New HAL functions with struct net_device  as first param
	 * this can be (hopefully)switched to struct ieee80211_hw
	 */

	.get_bbreg = 	rtl8821au_phy_query_bb_reg,
	.set_bbreg = 	rtl8821au_phy_set_bb_reg,
	.get_rfreg = 	PHY_QueryRFReg8812,
	.set_rfreg = 	PHY_SetRFReg8812,
	
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
	.read_adapter_info =	ReadAdapterInfo8812AU,

	/* .set_bwmode_handler = 	PHY_SetBWMode8192C; */
	/* .set_channel_handler = 	PHY_SwChnl8192C; */

	/* .hal_dm_watchdog = 	rtl8192c_HalDmWatchDog; */


	.set_hw_reg =	 	rtl8821au_set_hw_reg,
	.get_hw_reg = 		rtl8821au_get_hw_reg,
	.GetHalDefVarHandler = 	rtl8821au_get_hal_def_var,
	.SetHalDefVarHandler = 	rtl8821au_set_hal_def_var,

	.SetBeaconRelatedRegistersHandler = 	SetBeaconRelatedRegisters8812A,

	/* .Add_RateATid = &rtl8192c_Add_RateATid, */

	.hal_xmit = 		rtl8812au_hal_xmit,
	.mgnt_xmit = 		rtl8812au_mgnt_xmit,
	.hal_xmitframe_enqueue = 	rtl8812au_hal_xmitframe_enqueue,

	.free_hal_data =	rtl8812_free_hal_data,

	.dm_init =		rtl8812_init_dm_priv,
	.dm_deinit =		rtl8812_deinit_dm_priv,

	.UpdateRAMaskHandler =	UpdateHalRAMask8812A,

	.read_chip_version =	ReadChipVersion8812A,

	.set_bwmode_handler =	PHY_SetBWMode8812,
	.set_channel_handler =	PHY_SwChnl8812,
	.set_chnl_bw_handler =	PHY_SetSwChnlBWMode8812,

	.hal_dm_watchdog =	rtl8812_HalDmWatchDog,

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

	.GetHalODMVarHandler = rtl8812_GetHalODMVar,
	.SetHalODMVarHandler = rtl8812_SetHalODMVar,
	.hal_notch_filter = hal_notch_filter_8812,

	.SetBeaconRelatedRegistersHandler =	SetBeaconRelatedRegisters8812A,
};


extern int rtw_ht_enable;
extern int rtw_bw_mode;
extern int rtw_ampdu_enable;	/* for enable tx_ampdu */

extern int pm_netdev_open(struct net_device *ndev,uint8_t bnormal);
static int rtw_suspend(struct usb_interface *intf, pm_message_t message);
static int rtw_resume(struct usb_interface *intf);
int rtw_resume_process(struct rtl_priv *rtlpriv);


static int rtw_drv_init(struct usb_interface *pusb_intf,const struct usb_device_id *pdid);
static void rtw_dev_remove(struct usb_interface *pusb_intf);

#define USB_VENDER_ID_REALTEK		0x0BDA


/* DID_USB_v916_20130116 */
static struct usb_device_id rtw_usb_id_tbl[] ={





#ifdef CONFIG_RTL8812A
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
#endif

#ifdef CONFIG_RTL8821A
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
#endif

	{}	/* Terminating entry */
};

MODULE_DEVICE_TABLE(usb, rtw_usb_id_tbl);

int const rtw_usb_id_len = sizeof(rtw_usb_id_tbl) / sizeof(struct usb_device_id);

static inline int RT_usb_endpoint_dir_in(const struct usb_endpoint_descriptor *epd)
{
	return ((epd->bEndpointAddress & USB_ENDPOINT_DIR_MASK) == USB_DIR_IN);
}

static inline int RT_usb_endpoint_dir_out(const struct usb_endpoint_descriptor *epd)
{
	return ((epd->bEndpointAddress & USB_ENDPOINT_DIR_MASK) == USB_DIR_OUT);
}

static inline int RT_usb_endpoint_xfer_int(const struct usb_endpoint_descriptor *epd)
{
	return ((epd->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK) == USB_ENDPOINT_XFER_INT);
}

static inline int RT_usb_endpoint_xfer_bulk(const struct usb_endpoint_descriptor *epd)
{
 	return ((epd->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK) == USB_ENDPOINT_XFER_BULK);
}

static inline int RT_usb_endpoint_is_bulk_in(const struct usb_endpoint_descriptor *epd)
{
	return (RT_usb_endpoint_xfer_bulk(epd) && RT_usb_endpoint_dir_in(epd));
}

static inline int RT_usb_endpoint_is_bulk_out(const struct usb_endpoint_descriptor *epd)
{
	return (RT_usb_endpoint_xfer_bulk(epd) && RT_usb_endpoint_dir_out(epd));
}

static inline int RT_usb_endpoint_num(const struct usb_endpoint_descriptor *epd)
{
	return epd->bEndpointAddress & USB_ENDPOINT_NUMBER_MASK;
}

static uint8_t rtw_init_intf_priv(struct rtl_usb *dvobj)
{
	uint8_t rst = _SUCCESS;

#ifdef CONFIG_USB_VENDOR_REQ_MUTEX
	mutex_init(&dvobj->usb_vendor_req_mutex);
#endif


#ifdef CONFIG_USB_VENDOR_REQ_BUFFER_PREALLOC
	dvobj->usb_alloc_vendor_req_buf = rtw_zmalloc(MAX_USB_IO_CTL_SIZE);
	if (dvobj->usb_alloc_vendor_req_buf == NULL) {
		DBG_871X("alloc usb_vendor_req_buf failed... /n");
		rst = _FAIL;
		goto exit;
	}
	dvobj->usb_vendor_req_buf  =
		(uint8_t *)N_BYTE_ALIGMENT((SIZE_PTR)(dvobj->usb_alloc_vendor_req_buf ), ALIGNMENT_UNIT);
exit:
#endif

	return rst;

}

static uint8_t rtw_deinit_intf_priv(struct rtl_usb *dvobj)
{
	uint8_t rst = _SUCCESS;

#ifdef CONFIG_USB_VENDOR_REQ_BUFFER_PREALLOC
	if(dvobj->usb_vendor_req_buf)
		rtw_mfree(dvobj->usb_alloc_vendor_req_buf);
#endif

#ifdef CONFIG_USB_VENDOR_REQ_MUTEX
	mutex_destroy(&dvobj->usb_vendor_req_mutex);
#endif

	return rst;
}

static struct rtl_usb *usb_dvobj_init(struct usb_interface *usb_intf, struct rtl_usb *pdvobjpriv)
{
	int	i;
	uint8_t	val8;
	int	status = _FAIL;
	struct usb_device_descriptor 	*pdev_desc;
	struct usb_host_config		*phost_conf;
	struct usb_config_descriptor	*pconf_desc;
	struct usb_host_interface	*phost_iface;
	struct usb_interface_descriptor	*piface_desc;
	struct usb_host_endpoint	*phost_endp;
	struct usb_endpoint_descriptor	*pendp_desc;
	struct usb_device			*pusbd;

	mutex_init(&pdvobjpriv->hw_init_mutex);
	mutex_init(&pdvobjpriv->h2c_fwcmd_mutex);
	mutex_init(&pdvobjpriv->setch_mutex);
	mutex_init(&pdvobjpriv->setbw_mutex);

	spin_lock_init(&pdvobjpriv->lock);

	pdvobjpriv->macid[1] = _TRUE; 	/* macid=1 for bc/mc stainfo */


	pdvobjpriv->pusbintf = usb_intf ;
	pusbd = pdvobjpriv->pusbdev = interface_to_usbdev(usb_intf);
	usb_set_intfdata(usb_intf, pdvobjpriv);

	pdvobjpriv->RtNumInPipes = 0;
	pdvobjpriv->RtNumOutPipes = 0;

	/*
	 * rtlpriv->EepromAddressSize = 6;
	 * pdvobjpriv->nr_endpoint = 6;
	 */

	pdev_desc = &pusbd->descriptor;


	phost_conf = pusbd->actconfig;
	pconf_desc = &phost_conf->desc;


	/*
	 * DBG_871X("\n****** num of altsetting = (%d) ******\n", pusb_interface->num_altsetting);
	 */


	phost_iface = &usb_intf->altsetting[0];
	piface_desc = &phost_iface->desc;


	pdvobjpriv->NumInterfaces = pconf_desc->bNumInterfaces;
	pdvobjpriv->InterfaceNumber = piface_desc->bInterfaceNumber;
	pdvobjpriv->nr_endpoint = piface_desc->bNumEndpoints;

	/* DBG_871X("\ndump usb_endpoint_descriptor:\n"); */

	for (i = 0; i < pdvobjpriv->nr_endpoint; i++) {
		phost_endp = phost_iface->endpoint + i;
		if (phost_endp) 		{
			pendp_desc = &phost_endp->desc;

			DBG_871X("\nusb_endpoint_descriptor(%d):\n", i);
			DBG_871X("bLength=%x\n",pendp_desc->bLength);
			DBG_871X("bDescriptorType=%x\n",pendp_desc->bDescriptorType);
			DBG_871X("bEndpointAddress=%x\n",pendp_desc->bEndpointAddress);
			/* DBG_871X("bmAttributes=%x\n",pendp_desc->bmAttributes); */
			DBG_871X("wMaxPacketSize=%d\n",le16_to_cpu(pendp_desc->wMaxPacketSize));
			DBG_871X("bInterval=%x\n",pendp_desc->bInterval);
			/* DBG_871X("bRefresh=%x\n",pendp_desc->bRefresh); */
			/* DBG_871X("bSynchAddress=%x\n",pendp_desc->bSynchAddress); */

			if (RT_usb_endpoint_is_bulk_in(pendp_desc)) {
				DBG_871X("RT_usb_endpoint_is_bulk_in = %x\n", RT_usb_endpoint_num(pendp_desc));
				pdvobjpriv->RtInPipe[pdvobjpriv->RtNumInPipes] = RT_usb_endpoint_num(pendp_desc);
				pdvobjpriv->RtNumInPipes++;
			} else if (RT_usb_endpoint_is_bulk_out(pendp_desc)) {
				DBG_871X("RT_usb_endpoint_is_bulk_out = %x\n", RT_usb_endpoint_num(pendp_desc));
				pdvobjpriv->RtOutPipe[pdvobjpriv->RtNumOutPipes] = RT_usb_endpoint_num(pendp_desc);
				pdvobjpriv->RtNumOutPipes++;
			}
			pdvobjpriv->ep_num[i] = RT_usb_endpoint_num(pendp_desc);
		}
	}

	DBG_871X("nr_endpoint=%d, in_num=%d, out_num=%d\n\n", pdvobjpriv->nr_endpoint, pdvobjpriv->RtNumInPipes, pdvobjpriv->RtNumOutPipes);

	switch(pusbd->speed) {
	case USB_SPEED_LOW:
		DBG_871X("USB_SPEED_LOW\n");
		pdvobjpriv->usb_speed = RTW_USB_SPEED_1_1;
		break;
	case USB_SPEED_FULL:
		DBG_871X("USB_SPEED_FULL\n");
		pdvobjpriv->usb_speed = RTW_USB_SPEED_1_1;
		break;
	case USB_SPEED_HIGH:
		DBG_871X("USB_SPEED_HIGH\n");
		pdvobjpriv->usb_speed = RTW_USB_SPEED_2;
		break;
	case USB_SPEED_SUPER:
		DBG_871X("USB_SPEED_SUPER\n");
		pdvobjpriv->usb_speed = RTW_USB_SPEED_3;
		break;
	default:
		DBG_871X("USB_SPEED_UNKNOWN(%x)\n",pusbd->speed);
		pdvobjpriv->usb_speed = RTW_USB_SPEED_UNKNOWN;
		break;
	}

	if (pdvobjpriv->usb_speed == RTW_USB_SPEED_UNKNOWN) {
		DBG_871X("UNKNOWN USB SPEED MODE, ERROR !!!\n");
		goto free_dvobj;
	}

	if (rtw_init_intf_priv(pdvobjpriv) == _FAIL) {
		goto free_dvobj;
	}

	/* .3 misc */
	sema_init(&(pdvobjpriv->usb_suspend_sema), 0);
	rtw_reset_continual_urb_error(pdvobjpriv);

	usb_get_dev(pusbd);

	status = _SUCCESS;

free_dvobj:
	if (status != _SUCCESS && pdvobjpriv) {
		usb_set_intfdata(usb_intf, NULL);
		mutex_destroy(&pdvobjpriv->hw_init_mutex);
		mutex_destroy(&pdvobjpriv->h2c_fwcmd_mutex);
		mutex_destroy(&pdvobjpriv->setch_mutex);
		mutex_destroy(&pdvobjpriv->setbw_mutex);
		pdvobjpriv = NULL;
	}
exit:
	return pdvobjpriv;
}

static void usb_dvobj_deinit(struct usb_interface *usb_intf)
{
	struct rtl_usb *dvobj = usb_get_intfdata(usb_intf);

	usb_set_intfdata(usb_intf, NULL);
	if (dvobj) {
		/* Modify condition for 92DU DMDP 2010.11.18, by Thomas */
		if ((dvobj->NumInterfaces != 2 && dvobj->NumInterfaces != 3)
			|| (dvobj->InterfaceNumber == 1)) {
			if (interface_to_usbdev(usb_intf)->state != USB_STATE_NOTATTACHED) {
				/*
				 * If we didn't unplug usb dongle and remove/insert modlue, driver fails on sitesurvey for the first time when device is up .
				 * Reset usb port for sitesurvey fail issue. 2009.8.13, by Thomas
				 */
				DBG_871X("usb attached..., try to reset usb device\n");
				usb_reset_device(interface_to_usbdev(usb_intf));
			}
		}
		rtw_deinit_intf_priv(dvobj);
		mutex_destroy(&dvobj->hw_init_mutex);
		mutex_destroy(&dvobj->h2c_fwcmd_mutex);
		mutex_destroy(&dvobj->setch_mutex);
		mutex_destroy(&dvobj->setbw_mutex);
	}

	/* DBG_871X("%s %d\n", __func__, atomic_read(&usb_intf->dev.kobj.kref.refcount)); */
	usb_put_dev(interface_to_usbdev(usb_intf));


}

static void rtw_decide_chip_type_by_usb_info(struct rtl_priv *rtlpriv, const struct usb_device_id *pdid)
{
	rtlpriv->chip_type = pdid->driver_info;





	#if defined(CONFIG_RTL8812A) || defined(CONFIG_RTL8821A)
	if(rtlpriv->chip_type == RTL8812 || rtlpriv->chip_type == RTL8821)
		rtl8812au_set_hw_type(rtlpriv);
	#endif
}


static void usb_intf_start(struct rtl_priv *rtlpriv)
{
	rtw_hal_inirp_init(rtlpriv);
}

static void usb_intf_stop(struct rtl_priv *rtlpriv)
{
	/* disabel_hw_interrupt */
	if (rtlpriv->bSurpriseRemoved == _FALSE) {
		/* device still exists, so driver can do i/o operation */
		/* TODO: */
	}

	/* cancel in irp */
	rtw_hal_inirp_deinit(rtlpriv);

	/* cancel out irp */
	usb_write_port_cancel(rtlpriv);

	/* todo:cancel other irps */

}

static void rtw_dev_unload(struct rtl_priv *rtlpriv)
{
	struct net_device *ndev= (struct net_device*)rtlpriv->ndev;
	uint8_t val8;

	if (rtlpriv->bup == _TRUE) {
		DBG_871X("===> rtw_dev_unload\n");

		rtlpriv->bDriverStopped = _TRUE;
		/* s3. */
		if(rtlpriv->intf_stop)
			rtlpriv->intf_stop(rtlpriv);

		/* s4. */
		if(!rtlpriv->pwrctrlpriv.bInternalAutoSuspend )
		rtw_stop_drv_threads(rtlpriv);


		/* s5. */
		if(rtlpriv->bSurpriseRemoved == _FALSE) {
			rtw_hal_deinit(rtlpriv);
			rtlpriv->bSurpriseRemoved = _TRUE;
		}

		rtlpriv->bup = _FALSE;
	} else {
		;
	}

	DBG_871X("<=== rtw_dev_unload\n");
}


static int rtw_suspend(struct usb_interface *pusb_intf, pm_message_t message)
{
	struct rtl_usb *dvobj = usb_get_intfdata(pusb_intf);
	struct rtl_priv *rtlpriv = dvobj->rtlpriv;
	struct net_device *ndev = rtlpriv->ndev;
	struct mlme_priv *pmlmepriv = &rtlpriv->mlmepriv;
	struct pwrctrl_priv *pwrpriv = &rtlpriv->pwrctrlpriv;
	struct usb_device *usb_dev = interface_to_usbdev(pusb_intf);

	int ret = 0;
	u32 start_time = jiffies;

	DBG_871X("==> %s (%s:%d)\n",__FUNCTION__, current->comm, current->pid);

	if((!rtlpriv->bup) || (rtlpriv->bDriverStopped)||(rtlpriv->bSurpriseRemoved)) {
		DBG_871X("rtlpriv->bup=%d bDriverStopped=%d bSurpriseRemoved = %d\n",
			rtlpriv->bup, rtlpriv->bDriverStopped,rtlpriv->bSurpriseRemoved);
		goto exit;
	}

	if(pwrpriv->bInternalAutoSuspend )
	{
	}
	pwrpriv->bInSuspend = _TRUE;
	rtw_cancel_all_timer(rtlpriv);
	LeaveAllPowerSaveMode(rtlpriv);

	down(&pwrpriv->lock);
	/*
	 * rtlpriv->net_closed = _TRUE;
	 * s1.
	 */
	if (ndev) {
		netif_carrier_off(ndev);
		rtw_netif_stop_queue(ndev);
	}

	/* s2. */
	rtw_disassoc_cmd(rtlpriv, 0, _FALSE);

	/* s2-2.  indicate disconnect to os */
	rtw_indicate_disconnect(rtlpriv);
	/* s2-3. */
	rtw_free_assoc_resources(rtlpriv, 1);
#ifdef CONFIG_AUTOSUSPEND
	if(!pwrpriv->bInternalAutoSuspend )
#endif
	/* s2-4. */
	rtw_free_network_queue(rtlpriv, _TRUE);

	rtw_dev_unload(rtlpriv);
#ifdef CONFIG_AUTOSUSPEND
	pwrpriv->rf_pwrstate = rf_off;
	pwrpriv->bips_processing = _FALSE;
#endif
	up(&pwrpriv->lock);

	if (check_fwstate(pmlmepriv, _FW_UNDER_SURVEY))
		rtw_indicate_scan_done(rtlpriv, 1);

	if (check_fwstate(pmlmepriv, _FW_UNDER_LINKING))
		rtw_indicate_disconnect(rtlpriv);

exit:
	DBG_871X("<===  %s return %d.............. in %dms\n", __FUNCTION__
		, ret, rtw_get_passing_time_ms(start_time));

	return ret;
}

static int rtw_resume(struct usb_interface *pusb_intf)
{
	struct rtl_usb *dvobj = usb_get_intfdata(pusb_intf);
	struct rtl_priv *rtlpriv = dvobj->rtlpriv;

	return rtw_resume_process(rtlpriv);
}

int rtw_resume_process(struct rtl_priv *rtlpriv)
{
	struct net_device *ndev;
	struct pwrctrl_priv *pwrpriv;
	int ret = -1;
	u32 start_time = jiffies;

	DBG_871X("==> %s (%s:%d)\n",__FUNCTION__, current->comm, current->pid);

	if(rtlpriv) {
		ndev= rtlpriv->ndev;
		pwrpriv = &rtlpriv->pwrctrlpriv;
	} else
		goto exit;

	down(&pwrpriv->lock);
	rtw_reset_drv_sw(rtlpriv);
	pwrpriv->bkeepfwalive = _FALSE;

	DBG_871X("bkeepfwalive(%x)\n",pwrpriv->bkeepfwalive);
	if(pm_netdev_open(ndev,_TRUE) != 0){
		up(&pwrpriv->lock);
		goto exit;
	}

	netif_device_attach(ndev);
	netif_carrier_on(ndev);

#ifdef CONFIG_AUTOSUSPEND
	if (pwrpriv->bInternalAutoSuspend) {
		pwrpriv->bInternalAutoSuspend = _FALSE;
		pwrpriv->brfoffbyhw = _FALSE;
		DBG_871X("enc_algorithm(%x),wepkeymask(%x)\n",
			rtlpriv->securitypriv.dot11PrivacyAlgrthm,pwrpriv->wepkeymask);
		if  ((_WEP40_ == rtlpriv->securitypriv.dot11PrivacyAlgrthm) ||
			(_WEP104_ == rtlpriv->securitypriv.dot11PrivacyAlgrthm)) {
			sint keyid;

			for (keyid = 0; keyid < 4; keyid++) {
				if (pwrpriv->wepkeymask & BIT(keyid)) {
					if (keyid == rtlpriv->securitypriv.dot11PrivacyKeyIndex)
						rtw_set_key(rtlpriv,&rtlpriv->securitypriv, keyid, 1);
					else
						rtw_set_key(rtlpriv,&rtlpriv->securitypriv, keyid, 0);
				}
			}
		}
	}
#endif
	up(&pwrpriv->lock);

	if (rtlpriv->pid[1]!=0) {
		DBG_871X("pid[1]:%d\n",rtlpriv->pid[1]);
		rtw_signal_process(rtlpriv->pid[1], SIGUSR2);
	}

	ret = 0;
exit:
	pwrpriv->bInSuspend = _FALSE;
	DBG_871X("<===  %s return %d.............. in %dms\n", __FUNCTION__
		, ret, rtw_get_passing_time_ms(start_time));

	return ret;
}

#ifdef CONFIG_AUTOSUSPEND
void autosuspend_enter(struct rtl_priv* rtlpriv)
{
	struct pwrctrl_priv *pwrpriv = &rtlpriv->pwrctrlpriv;
	struct rtl_usb *dvobj = rtl_usbdev(rtlpriv);

	DBG_871X("==>autosuspend_enter...........\n");

	pwrpriv->bInternalAutoSuspend = _TRUE;
	pwrpriv->bips_processing = _TRUE;

	if (rf_off == pwrpriv->change_rfpwrstate) {
		usb_enable_autosuspend(dvobj->pusbdev);

			usb_autopm_put_interface(dvobj->pusbintf);
	}
	DBG_871X("...pm_usage_cnt(%d).....\n", atomic_read(&(dvobj->pusbintf->pm_usage_cnt)));

}

int autoresume_enter(struct rtl_priv* rtlpriv)
{
	int result = _SUCCESS;
	struct pwrctrl_priv *pwrpriv = &rtlpriv->pwrctrlpriv;
	struct security_priv* psecuritypriv=&(rtlpriv->securitypriv);
	struct mlme_ext_priv	*pmlmeext = &rtlpriv->mlmeextpriv;
	struct mlme_ext_info	*pmlmeinfo = &(pmlmeext->mlmext_info);
	struct rtl_usb *dvobj = rtl_usbdev(rtlpriv);

	DBG_871X("====> autoresume_enter \n");

	if (rf_off == pwrpriv->rf_pwrstate) {
		pwrpriv->ps_flag = _FALSE;
			if (usb_autopm_get_interface(dvobj->pusbintf) < 0) {
				DBG_871X( "can't get autopm: %d\n", result);
				result = _FAIL;
				goto error_exit;
			}

		DBG_871X("...pm_usage_cnt(%d).....\n", atomic_read(&(dvobj->pusbintf->pm_usage_cnt)));
	}
	DBG_871X("<==== autoresume_enter \n");
error_exit:

	return result;
}
#endif

/*
 * ULLI messy but needed
 */

int netdev_open(struct net_device *ndev);
int netdev_close(struct net_device *ndev);
int rtw_net_set_mac_address(struct net_device *ndev, void *p);
struct net_device_stats *rtw_net_get_stats(struct net_device *ndev);
uint loadparam(struct rtl_priv *rtlpriv, struct net_device *ndev);

static const struct net_device_ops rtw_netdev_ops = {
	.ndo_open = netdev_open,
	.ndo_stop = netdev_close,
	.ndo_start_xmit = rtw_xmit_entry,
	.ndo_set_mac_address = rtw_net_set_mac_address,
	.ndo_get_stats = rtw_net_get_stats,
	.ndo_do_ioctl = rtw_ioctl,
};


/*
 * drv_init() - a device potentially for us
 *
 * notes: drv_init() is called when the bus driver has located a card for us to support.
 *        We accept the new device by returning 0.
*/

struct rtl_priv  *rtw_sw_export = NULL;

static char *ifname = "wlan%d";

static int rtw_init_netdev_name(struct net_device *ndev, const char *ifname)
{
	if (dev_alloc_name(ndev, ifname) < 0)
		/*
		 * RT_TRACE(_module_os_intfs_c_, _drv_err_, ("dev_alloc_name, fail!\n"));
		 */

	netif_carrier_off(ndev);
	return 0;
}

static struct rtl_priv *rtw_usb_if1_init(struct usb_interface *pusb_intf, const struct usb_device_id *pdid)
{
	struct rtl_usb *rtlusb;
	struct rtl_priv *rtlpriv = NULL;
	struct net_device *ndev = NULL;
	int status = _FAIL;

	ndev = alloc_etherdev_mq(sizeof(*rtlpriv), 4);
	if (!ndev)
		goto exit;

	rtlpriv = netdev_priv(ndev);
	rtlusb = rtl_usbdev(rtlpriv);
	usb_dvobj_init(pusb_intf, rtlusb);

	rtlpriv = netdev_priv(ndev);
	rtlpriv->ndev = ndev;

	rtlusb->rtlpriv = rtlpriv;

	rtlpriv->bDriverStopped=_TRUE;

	/* step 1-1., decide the chip_type via driver_info */
	rtlpriv->interface_type = RTW_USB;
	rtw_decide_chip_type_by_usb_info(rtlpriv, pdid);

	/* ndev->init = NULL; */

	DBG_871X("register rtw_netdev_ops to netdev_ops\n");
	ndev->netdev_ops = &rtw_netdev_ops;

#ifdef CONFIG_TCP_CSUM_OFFLOAD_TX
	ndev->features |= NETIF_F_IP_CSUM;
#endif
	/* ndev->tx_timeout = NULL; */
	ndev->watchdog_timeo = HZ*3; /* 3 second timeout */
#ifdef CONFIG_WIRELESS_EXT
	ndev->wireless_handlers = (struct iw_handler_def *)&rtw_handlers_def;
#endif


	/* step 2. */
	loadparam(rtlpriv, ndev);

	SET_NETDEV_DEV(ndev, dvobj_to_dev(rtlusb));
	rtlpriv = rtl_priv(ndev);

	/* step 2. hook HalFunc, allocate HalData */
	/* hal_set_hal_ops(rtlpriv); */
	rtlpriv->HalData = rtw_zmalloc(sizeof( struct _rtw_hal));
	if (rtlpriv->HalData == NULL) {
		DBG_8192C("cant not alloc memory for HAL DATA \n");
	}

	rtlpriv->HalFunc = &rtl8821au_hal_ops;

	rtlpriv->intf_start=&usb_intf_start;
	rtlpriv->intf_stop=&usb_intf_stop;

	/* step read_chip_version */
	rtw_hal_read_chip_version(rtlpriv);

	/* step usb endpoint mapping */
	rtw_hal_chip_configure(rtlpriv);

	/* step read efuse/eeprom data and get mac_addr */
	rtw_hal_read_chip_info(rtlpriv);

	/* step 5. */
	if(rtl8821au_init_sw_vars(ndev) ==_FAIL) {
		goto free_hal_data;
	}

#ifdef CONFIG_PM
	if (rtlpriv->pwrctrlpriv.bSupportRemoteWakeup) {
		rtlusb->pusbdev->do_remote_wakeup=1;
		pusb_intf->needs_remote_wakeup = 1;
		device_init_wakeup(&pusb_intf->dev, 1);
		DBG_871X("\n  rtlpriv->pwrctrlpriv.bSupportRemoteWakeup~~~~~~\n");
		DBG_871X("\n  rtlpriv->pwrctrlpriv.bSupportRemoteWakeup~~~[%d]~~~\n",device_may_wakeup(&pusb_intf->dev));
	}
#endif

#ifdef CONFIG_AUTOSUSPEND
	if (rtlpriv->registrypriv.power_mgnt != PS_MODE_ACTIVE) {
		if(rtlpriv->registrypriv.usbss_enable ){ 	/* autosuspend (2s delay) */
			dvobj->pusbdev->dev.power.autosuspend_delay = 0 * HZ;//15 * HZ; idle-delay time

			usb_enable_autosuspend(dvobj->pusbdev);

			/* usb_autopm_get_interface(rtl_usbdev(rtlpriv)->pusbintf );//init pm_usage_cnt ,let it start from 1 */

			DBG_871X("%s...pm_usage_cnt(%d).....\n",__FUNCTION__,atomic_read(&(dvobj->pusbintf ->pm_usage_cnt)));
		}
	}
#endif
	/* 2012-07-11 Move here to prevent the 8723AS-VAU BT auto suspend influence */
	if (usb_autopm_get_interface(pusb_intf) < 0) {
		DBG_871X( "can't get autopm: \n");
	}

	/*  set mac addr */
	rtw_init_netdev_name(ndev, ifname);
	rtw_macaddr_cfg(rtlpriv->eeprompriv.mac_addr);

	DBG_871X("bDriverStopped:%d, bSurpriseRemoved:%d, bup:%d, hw_init_completed:%d\n"
		, rtlpriv->bDriverStopped
		, rtlpriv->bSurpriseRemoved
		, rtlpriv->bup
		, rtlpriv->hw_init_completed
	);

	status = _SUCCESS;

	netif_carrier_off(rtlpriv->ndev);
	/* rtw_netif_stop_queue(ndev); */

/*
 * 	ULLI : for support older kernel < 3.14,
 *	ether_addr_copy(ndev->dev_addr, rtlpriv->eeprompriv.mac_addr);
 */
	memcpy(ndev->dev_addr,  rtlpriv->eeprompriv.mac_addr, ETH_ALEN);

	/* Tell the network stack we exist */
	if (register_netdev(rtlpriv->ndev) != 0) {
		DBG_871X(FUNC_NDEV_FMT "Failed!\n", FUNC_NDEV_ARG(ndev));
		status = _FAIL;
		goto free_hal_data;
	}

free_hal_data:
	if (status != _SUCCESS && rtlpriv->HalData)
		rtw_mfree(rtlpriv->HalData);

free_adapter:
	if (status != _SUCCESS) {
		if (ndev)
			free_netdev(ndev);

		rtlpriv = NULL;
	}
exit:
	return rtlpriv;
}

static void rtw_usb_if1_deinit(struct rtl_priv *rtlpriv)
{
	struct net_device *ndev = rtlpriv->ndev;
	struct mlme_priv *pmlmepriv= &rtlpriv->mlmepriv;

	if(check_fwstate(pmlmepriv, _FW_LINKED))
		rtw_disassoc_cmd(rtlpriv, 0, _FALSE);


#ifdef CONFIG_AP_MODE
	free_mlme_ap_info(rtlpriv);
#endif

	if (rtlpriv->DriverState != DRIVER_DISAPPEAR) {
		if (ndev) {
			unregister_netdev(ndev); /* will call netdev_close() */
			rtw_proc_remove_one(ndev);
		}
	}

	rtw_cancel_all_timer(rtlpriv);
	rtw_dev_unload(rtlpriv);

	DBG_871X("+r871xu_dev_remove, hw_init_completed=%d\n", rtlpriv->hw_init_completed);

	rtw_free_drv_sw(rtlpriv);

	if(ndev)
		free_netdev(ndev);
}


static void dump_usb_interface(struct usb_interface *usb_intf)
{
	int	i;
	uint8_t	val8;

	struct usb_device				*udev = interface_to_usbdev(usb_intf);
	struct usb_device_descriptor 	*dev_desc = &udev->descriptor;

	struct usb_host_config			*act_conf = udev->actconfig;
	struct usb_config_descriptor	*act_conf_desc = &act_conf->desc;

	struct usb_host_interface		*host_iface;
	struct usb_interface_descriptor	*iface_desc;
	struct usb_host_endpoint		*host_endp;
	struct usb_endpoint_descriptor	*endp_desc;

#if 1 /* The usb device this usb interface belongs to */
	DBG_871X("usb_interface:%p, usb_device:%p(num:%d, path:%s), usb_device_descriptor:%p\n", usb_intf, udev, udev->devnum, udev->devpath, dev_desc);
	DBG_871X("bLength:%u\n", dev_desc->bLength);
	DBG_871X("bDescriptorType:0x%02x\n", dev_desc->bDescriptorType);
	DBG_871X("bcdUSB:0x%04x\n", le16_to_cpu(dev_desc->bcdUSB));
	DBG_871X("bDeviceClass:0x%02x\n", dev_desc->bDeviceClass);
	DBG_871X("bDeviceSubClass:0x%02x\n", dev_desc->bDeviceSubClass);
	DBG_871X("bDeviceProtocol:0x%02x\n", dev_desc->bDeviceProtocol);
	DBG_871X("bMaxPacketSize0:%u\n", dev_desc->bMaxPacketSize0);
	DBG_871X("idVendor:0x%04x\n", le16_to_cpu(dev_desc->idVendor));
	DBG_871X("idProduct:0x%04x\n", le16_to_cpu(dev_desc->idProduct));
	DBG_871X("bcdDevice:0x%04x\n", le16_to_cpu(dev_desc->bcdDevice));
	DBG_871X("iManufacturer:0x02%x\n", dev_desc->iManufacturer);
	DBG_871X("iProduct:0x%02x\n", dev_desc->iProduct);
	DBG_871X("iSerialNumber:0x%02x\n", dev_desc->iSerialNumber);
	DBG_871X("bNumConfigurations:%u\n", dev_desc->bNumConfigurations);
#endif


#if 1 /* The acting usb_config_descriptor */
	DBG_871X("\nact_conf_desc:%p\n", act_conf_desc);
	DBG_871X("bLength:%u\n", act_conf_desc->bLength);
	DBG_871X("bDescriptorType:0x%02x\n", act_conf_desc->bDescriptorType);
	DBG_871X("wTotalLength:%u\n", le16_to_cpu(act_conf_desc->wTotalLength));
	DBG_871X("bNumInterfaces:%u\n", act_conf_desc->bNumInterfaces);
	DBG_871X("bConfigurationValue:0x%02x\n", act_conf_desc->bConfigurationValue);
	DBG_871X("iConfiguration:0x%02x\n", act_conf_desc->iConfiguration);
	DBG_871X("bmAttributes:0x%02x\n", act_conf_desc->bmAttributes);
	DBG_871X("bMaxPower=%u\n", act_conf_desc->bMaxPower);
#endif


	DBG_871X("****** num of altsetting = (%d) ******/\n", usb_intf->num_altsetting);
	/* Get he host side alternate setting (the current alternate setting) for this interface*/
	host_iface = usb_intf->cur_altsetting;
	iface_desc = &host_iface->desc;

#if 1 /* The current alternate setting*/
	DBG_871X("\nusb_interface_descriptor:%p:\n", iface_desc);
	DBG_871X("bLength:%u\n", iface_desc->bLength);
	DBG_871X("bDescriptorType:0x%02x\n", iface_desc->bDescriptorType);
	DBG_871X("bInterfaceNumber:0x%02x\n", iface_desc->bInterfaceNumber);
	DBG_871X("bAlternateSetting=%x\n", iface_desc->bAlternateSetting);
	DBG_871X("bNumEndpoints=%x\n", iface_desc->bNumEndpoints);
	DBG_871X("bInterfaceClass=%x\n", iface_desc->bInterfaceClass);
	DBG_871X("bInterfaceSubClass=%x\n", iface_desc->bInterfaceSubClass);
	DBG_871X("bInterfaceProtocol=%x\n", iface_desc->bInterfaceProtocol);
	DBG_871X("iInterface=%x\n", iface_desc->iInterface);
#endif


#if 1
	/* DBG_871X("\ndump usb_endpoint_descriptor:\n"); */

	for (i = 0; i < iface_desc->bNumEndpoints; i++) {
		host_endp = host_iface->endpoint + i;
		if (host_endp) {
			endp_desc = &host_endp->desc;

			DBG_871X("\nusb_endpoint_descriptor(%d):\n", i);
			DBG_871X("bLength=%x\n",endp_desc->bLength);
			DBG_871X("bDescriptorType=%x\n",endp_desc->bDescriptorType);
			DBG_871X("bEndpointAddress=%x\n",endp_desc->bEndpointAddress);
			DBG_871X("bmAttributes=%x\n",endp_desc->bmAttributes);
			DBG_871X("wMaxPacketSize=%x\n",endp_desc->wMaxPacketSize);
			DBG_871X("wMaxPacketSize=%x\n",le16_to_cpu(endp_desc->wMaxPacketSize));
			DBG_871X("bInterval=%x\n",endp_desc->bInterval);
			/* DBG_871X("bRefresh=%x\n",pendp_desc->bRefresh); */
			/* DBG_871X("bSynchAddress=%x\n",pendp_desc->bSynchAddress); */

			if (RT_usb_endpoint_is_bulk_in(endp_desc)) {
				DBG_871X("RT_usb_endpoint_is_bulk_in = %x\n", RT_usb_endpoint_num(endp_desc));
				/* pdvobjpriv->RtNumInPipes++; */
			} else if (RT_usb_endpoint_is_bulk_out(endp_desc)) {
				DBG_871X("RT_usb_endpoint_is_bulk_out = %x\n", RT_usb_endpoint_num(endp_desc));
				/* pdvobjpriv->RtNumOutPipes++; */
			}
			/* pdvobjpriv->ep_num[i] = RT_usb_endpoint_num(pendp_desc); */
		}
	}

	/*
	 * DBG_871X("nr_endpoint=%d, in_num=%d, out_num=%d\n\n", pdvobjpriv->nr_endpoint, pdvobjpriv->RtNumInPipes, pdvobjpriv->RtNumOutPipes);
	 */
#endif

	if (udev->speed == USB_SPEED_HIGH)
		DBG_871X("USB_SPEED_HIGH\n");
	else
		DBG_871X("NON USB_SPEED_HIGH\n");

}

static int rtw_drv_init(struct usb_interface *pusb_intf, const struct usb_device_id *pdid)
{
	struct rtl_priv *rtlpriv = NULL;
	int status;
	struct rtl_usb *dvobj;

	/* DBG_871X("+rtw_drv_init\n"); */

	/* Initialize dvobj_priv */
	if ((rtlpriv = rtw_usb_if1_init(pusb_intf, pdid)) == NULL) {
		DBG_871X("rtw_usb_if1_init Failed!\n");
		goto free_dvobj;
	}

	status = _SUCCESS;

free_if1:
	if (status != _SUCCESS && rtlpriv) {
		rtw_usb_if1_deinit(rtlpriv);
	}
free_dvobj:
	if (status != _SUCCESS)
		usb_dvobj_deinit(pusb_intf);
exit:
	return status == _SUCCESS?0:-ENODEV;
}

/*
 * dev_remove() - our device is being removed
*/
/*
 * rmmod module & unplug(SurpriseRemoved) will call r871xu_dev_remove() => how to recognize both
 */
static void rtw_dev_remove(struct usb_interface *pusb_intf)
{
	struct rtl_usb *dvobj = usb_get_intfdata(pusb_intf);
	struct rtl_priv *rtlpriv = dvobj->rtlpriv;
	struct net_device *ndev = rtlpriv->ndev;
	struct mlme_priv *pmlmepriv= &rtlpriv->mlmepriv;

	DBG_871X("+rtw_dev_remove\n");


	rtw_pm_set_ips(rtlpriv, IPS_NONE);
	rtw_pm_set_lps(rtlpriv, PS_MODE_ACTIVE);

	LeaveAllPowerSaveMode(rtlpriv);

	rtw_usb_if1_deinit(rtlpriv);

	usb_dvobj_deinit(pusb_intf);

	DBG_871X("-r871xu_dev_remove, done\n");

	return;
}

static struct usb_driver rtl8821au_usb_drv = {
	.name = "rtl8821au",
	.probe = rtw_drv_init,
	.disconnect = rtw_dev_remove,
	.id_table = rtw_usb_id_tbl,
	.suspend =  rtw_suspend,
	.resume = rtw_resume,
  	.reset_resume   = rtw_resume,
#ifdef CONFIG_AUTOSUSPEND
	.supports_autosuspend = 1,
#endif
	
};

module_usb_driver(rtl8821au_usb_drv)

