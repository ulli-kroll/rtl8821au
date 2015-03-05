#ifndef __RTL_USB_H__
#define __RTL_USB_H__

#include <linux/usb.h>

struct rtl_usb {
        struct rtl_priv *padapter;

	//for local/global synchronization
	//
	spinlock_t	lock;
	int macid[NUM_STA];

	struct mutex hw_init_mutex;
	struct mutex h2c_fwcmd_mutex;
	struct mutex setch_mutex;
	struct mutex setbw_mutex;

	unsigned char	oper_channel; //saved channel info when call set_channel_bw
	unsigned char	oper_bwmode;
	unsigned char	oper_ch_offset;//PRIME_CHNL_OFFSET

	//For 92D, DMDP have 2 interface.
	uint8_t	InterfaceNumber;
	uint8_t	NumInterfaces;

	//In /Out Pipe information
	int	RtInPipe[2];
	int	RtOutPipe[3];
	uint8_t	Queue2Pipe[HW_QUEUE_ENTRY];//for out pipe mapping

	uint8_t	irq_alloc;

/*-------- below is for SDIO INTERFACE --------*/

#ifdef INTF_DATA
	INTF_DATA intf_data;
#endif

/*-------- below is for USB INTERFACE --------*/


	uint8_t	usb_speed; // 1.1, 2.0 or 3.0
	uint8_t	nr_endpoint;
	uint8_t	RtNumInPipes;
	uint8_t	RtNumOutPipes;
	int	ep_num[6]; //endpoint number

	int	RegUsbSS;

	struct  semaphore	usb_suspend_sema;

#ifdef CONFIG_USB_VENDOR_REQ_MUTEX
	struct mutex usb_vendor_req_mutex;
#endif

#ifdef CONFIG_USB_VENDOR_REQ_BUFFER_PREALLOC
	uint8_t * usb_alloc_vendor_req_buf;
	uint8_t * usb_vendor_req_buf;
#endif

	struct usb_interface *pusbintf;
	struct usb_device *pusbdev;

	ATOMIC_T continual_urb_error;

/*-------- below is for PCIE INTERFACE --------*/

};

struct rtl_usb_priv {
	struct rtl_usb	dev;
	struct	rtl_led_ctl	ledpriv;
};

#define rtl_usbpriv(rtl)       (&(rtl->priv))

static inline struct device *dvobj_to_dev(struct rtl_usb *dvobj)
{
	/* todo: get interface type from dvobj and the return the dev accordingly */
	return &dvobj->pusbintf->dev;
}

uint8_t usb_read8(struct rtl_priv *rtlpriv, uint32_t addr);
u16 usb_read16(struct rtl_priv *rtlpriv, uint32_t addr);
uint32_t usb_read32(struct rtl_priv *rtlpriv, uint32_t addr);

int usb_write8(struct rtl_priv *rtlpriv, uint32_t addr, uint8_t val);
int usb_write16(struct rtl_priv *rtlpriv, uint32_t addr, u16 val);
int usb_write32(struct rtl_priv *rtlpriv, uint32_t addr, uint32_t val);

void usb_write_port_cancel(struct rtl_priv *rtlpriv);
void usb_read_port_cancel(struct rtl_priv *rtlprivl);

uint32_t usb_read_port(struct rtl_priv *padapter, uint32_t addr, uint32_t cnt, uint8_t *rmem);
u32 usb_write_port(struct rtl_priv *padapter, u32 addr, u32 cnt, struct xmit_buf *pxmitbu);
int usb_writeN(struct rtl_priv *rtlpriv, uint32_t addr, uint32_t length, uint8_t *pdata);


#endif
