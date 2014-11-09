struct rtl_priv {
	int	DriverState;// for disable driver using module, use dongle to replace module.
	int	pid[3];//process id from UI, 0:wps, 1:hostapd, 2:dhcpcd
	int	bDongle;//build-in module or external dongle
	u16 	chip_type;
	u16	HardwareType;
	u16	interface_type;//USB,SDIO,SPI,PCI

	struct dvobj_priv *dvobj;
	struct	mlme_priv mlmepriv;
	struct	mlme_ext_priv mlmeextpriv;
	struct	cmd_priv	cmdpriv;
	struct	evt_priv	evtpriv;
	//struct	io_queue	*pio_queue;
	struct 	io_priv	iopriv;
	struct	xmit_priv	xmitpriv;
	struct	recv_priv	recvpriv;
	struct	sta_priv	stapriv;
	struct	security_priv	securitypriv;
	struct	registry_priv	registrypriv;
	struct	pwrctrl_priv	pwrctrlpriv;
	struct 	eeprom_priv eeprompriv;
	struct	led_priv	ledpriv;

#ifdef CONFIG_AP_MODE
	struct	hostapd_priv	*phostapdpriv;
#endif

	u32	setband;
	PVOID			HalData;
	struct hal_ops	*HalFunc;

	int32_t	bDriverStopped;
	int32_t	bSurpriseRemoved;
	int32_t  bCardDisableWOHSM;

	u32	IsrContent;
	u32	ImrContent;

	uint8_t	EepromAddressSize;
	uint8_t	hw_init_completed;
	uint8_t	bDriverIsGoingToUnload;
	uint8_t	init_adpt_in_progress;
	uint8_t	bHaltInProgress;

	_thread_hdl_ cmdThread;
	_thread_hdl_ evtThread;
	_thread_hdl_ xmitThread;
	_thread_hdl_ recvThread;

#ifndef PLATFORM_LINUX
	NDIS_STATUS (*dvobj_init)(struct dvobj_priv *dvobj);
	void (*dvobj_deinit)(struct dvobj_priv *dvobj);
#endif

	void (*intf_start)(struct rtl_priv * adapter);
	void (*intf_stop)(struct rtl_priv * adapter);


#ifdef PLATFORM_LINUX
	struct net_device *ndev;

	int bup;
	struct net_device_stats stats;
	struct iw_statistics iwstats;
	struct proc_dir_entry *dir_dev;// for proc directory

#endif //end of PLATFORM_LINUX

	int net_closed;

	uint8_t bFWReady;
	uint8_t bBTFWReady;
	uint8_t bReadPortCancel;
	uint8_t bWritePortCancel;
	uint8_t bLinkInfoDump;
	//	Added by Albert 2012/10/26
	//	The driver will show up the desired channel number when this flag is 1.
	uint8_t bNotifyChannelChange;
#ifdef CONFIG_AUTOSUSPEND
	uint8_t	bDisableAutosuspend;
#endif
        uint8_t    fix_rate;

	unsigned char     in_cta_test;

};
