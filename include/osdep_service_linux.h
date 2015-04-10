/******************************************************************************
 *
 * Copyright(c) 2007 - 2013 Realtek Corporation. All rights reserved.
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
#ifndef __OSDEP_LINUX_SERVICE_H_
#define __OSDEP_LINUX_SERVICE_H_

	#include <linux/version.h>
	#include <linux/spinlock.h>
	#include <linux/compiler.h>
	#include <linux/kernel.h>
	#include <linux/errno.h>
	#include <linux/init.h>
	#include <linux/slab.h>
	#include <linux/module.h>
	#include <linux/kref.h>
	//#include <linux/smp_lock.h>
	#include <linux/netdevice.h>
	#include <linux/skbuff.h>
	#include <linux/circ_buf.h>
	#include <asm/uaccess.h>
	#include <asm/byteorder.h>
	#include <asm/atomic.h>
	#include <asm/io.h>
	#include <linux/semaphore.h>
	#include <linux/sem.h>
	#include <linux/sched.h>
	#include <linux/etherdevice.h>
	#include <linux/wireless.h>
	#include <net/iw_handler.h>
	#include <linux/if_arp.h>
	#include <linux/rtnetlink.h>
	#include <linux/delay.h>
	#include <linux/proc_fs.h>	// Necessary because we use the proc fs
	#include <linux/interrupt.h>	// for struct tasklet_struct
	#include <linux/ip.h>
	#include <linux/kthread.h>
	#include <linux/list.h>
	#include <linux/vmalloc.h>

#ifdef CONFIG_TCP_CSUM_OFFLOAD_TX
	#include <linux/in.h>
	#include <linux/udp.h>
#endif
	#include <linux/usb.h>
	#include <linux/usb/ch9.h>

	typedef struct urb *  PURB;
#ifdef CONFIG_USB_SUSPEND
#define CONFIG_AUTOSUSPEND	1
#endif

	typedef struct timer_list _timer;

struct __queue {
		struct	list_head	queue;
		spinlock_t	lock;
};

	typedef	int	_OS_STATUS;

	typedef void timer_hdl_return;
	typedef void* timer_hdl_context;


__inline static struct list_head *get_next(struct list_head	*list)
{
	return list->next;
}

__inline static struct list_head	*get_list_head(struct __queue	*queue)
{
	return (&(queue->queue));
}


#define LIST_CONTAINOR(ptr, type, member) \
        ((type *)((char *)(ptr)-(SIZE_T)(&((type *)0)->member)))


__inline static void rtw_list_delete(struct list_head *plist)
{
	list_del_init(plist);
}

#define RTW_TIMER_HDL_ARGS void *FunctionContext

__inline static void _init_timer(_timer *ptimer,struct net_device *nic_hdl,void *pfunc,void* cntx)
{
	//setup_timer(ptimer, pfunc,(u32)cntx);
	ptimer->function = pfunc;
	ptimer->data = (unsigned long)cntx;
	init_timer(ptimer);
}

__inline static void _set_timer(_timer *ptimer,u32 delay_time)
{
	mod_timer(ptimer , (jiffies+(delay_time*HZ/1000)));
}

__inline static void _cancel_timer(_timer *ptimer,u8 *bcancelled)
{
	del_timer_sync(ptimer);
	*bcancelled=  _TRUE;//TRUE ==1; FALSE==0
}

//
// Global Mutex: can only be used at PASSIVE level.
//

#define ACQUIRE_GLOBAL_MUTEX(_MutexCounter)                              \
{                                                               \
	while (atomic_inc_return((atomic_t *)&(_MutexCounter)) != 1)\
	{                                                           \
		atomic_dec((atomic_t *)&(_MutexCounter));        \
		msleep(10);                          \
	}                                                           \
}

#define RELEASE_GLOBAL_MUTEX(_MutexCounter)                              \
{                                                               \
	atomic_dec((atomic_t *)&(_MutexCounter));        \
}

static inline int rtw_netif_queue_stopped(struct net_device *ndev)
{
	return (netif_tx_queue_stopped(netdev_get_tx_queue(ndev, 0)) &&
		netif_tx_queue_stopped(netdev_get_tx_queue(ndev, 1)) &&
		netif_tx_queue_stopped(netdev_get_tx_queue(ndev, 2)) &&
		netif_tx_queue_stopped(netdev_get_tx_queue(ndev, 3)) );
}

static inline void rtw_netif_wake_queue(struct net_device *ndev)
{
	netif_tx_wake_all_queues(ndev);
}

static inline void rtw_netif_start_queue(struct net_device *ndev)
{
	netif_tx_start_all_queues(ndev);
}

static inline void rtw_netif_stop_queue(struct net_device *ndev)
{
	netif_tx_stop_all_queues(ndev);
}

#define rtw_signal_process(pid, sig) kill_pid(find_vpid((pid)),(sig), 1)



// limitation of path length
#define PATH_LENGTH_MAX PATH_MAX

#define NDEV_FMT "%s"
#define NDEV_ARG(ndev) ndev->name
#define ADPT_FMT "%s"
#define ADPT_ARG(rtlpriv) rtlpriv->ndev->name
#define FUNC_NDEV_FMT "%s(%s)"
#define FUNC_NDEV_ARG(ndev) __func__, ndev->name
#define FUNC_ADPT_FMT "%s(%s)"
#define FUNC_ADPT_ARG(rtlpriv) __func__, rtlpriv->ndev->name


static inline struct rtl_priv *rtl_priv(struct net_device *ndev)
{
	return netdev_priv(ndev);
}

#define STRUCT_PACKED __attribute__ ((packed))


#endif

