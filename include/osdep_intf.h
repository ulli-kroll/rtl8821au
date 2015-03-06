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

#ifndef __OSDEP_INTF_H_
#define __OSDEP_INTF_H_


struct intf_priv {

	uint8_t *intf_dev;
	u32	max_iosz; 	//USB2.0: 128, USB1.1: 64, SDIO:64
	u32	max_xmitsz; //USB2.0: unlimited, SDIO:512
	u32	max_recvsz; //USB2.0: unlimited, SDIO:512

	volatile uint8_t *io_rwmem;
	volatile uint8_t *allocated_io_rwmem;
	u32	io_wsz; //unit: 4bytes
	u32	io_rsz;//unit: 4bytes
	uint8_t intf_status;

	void (*_bus_io)(uint8_t *priv);

/*
Under Sync. IRP (SDIO/USB)
A protection mechanism is necessary for the io_rwmem(read/write protocol)

Under Async. IRP (SDIO/USB)
The protection mechanism is through the pending queue.
*/

	struct mutex ioctl_mutex;	/* ULLI: never used */


#ifdef PLATFORM_LINUX
	// when in USB, IO is through interrupt in/out endpoints
	struct usb_device 	*udev;
	PURB	piorw_urb;
	uint8_t io_irp_cnt;
	uint8_t bio_irp_pending;
	struct semaphore io_retevt;
	_timer	io_timer;
	uint8_t bio_irp_timeout;
	uint8_t bio_timer_cancel;
#endif


};


#ifdef CONFIG_R871X_TEST
int rtw_start_pseudo_adhoc(struct rtl_priv *rtlpriv);
int rtw_stop_pseudo_adhoc(struct rtl_priv *rtlpriv);
#endif

int rtl8821au_init_sw_vars(struct net_device *ndev);
uint8_t rtw_free_drv_sw(struct rtl_priv *rtlpriv);
uint8_t rtw_reset_drv_sw(struct rtl_priv *rtlpriv);

u32 rtw_start_drv_threads(struct rtl_priv *rtlpriv);
void rtw_stop_drv_threads (struct rtl_priv *rtlpriv);
void rtw_cancel_all_timer(struct rtl_priv *rtlpriv);

#ifdef PLATFORM_LINUX
int rtw_ioctl(struct net_device *dev, struct ifreq *rq, int cmd);

u16 rtw_recv_select_queue(struct sk_buff *skb);

/*
 *	#ifdef CONFIG_PROC_DEBUG
 *	void rtw_proc_init_one(struct net_device *dev);
 *	void rtw_proc_remove_one(struct net_device *dev);
 *	#else //!CONFIG_PROC_DEBUG
*/
static void rtw_proc_init_one(struct net_device *dev){}
static void rtw_proc_remove_one(struct net_device *dev){}
/*
 *	#endif //!CONFIG_PROC_DEBUG
*/
#endif //PLATFORM_LINUX



void rtw_ips_dev_unload(struct rtl_priv *rtlpriv);
#ifdef CONFIG_IPS
int rtw_ips_pwr_up(struct rtl_priv *rtlpriv);
void rtw_ips_pwr_down(struct rtl_priv *rtlpriv);
#endif

void rtw_ndev_destructor(struct  net_device *ndev);

#endif	//_OSDEP_INTF_H_

