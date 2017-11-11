/*
 * RTL8821AU quirks
 *
 * stupipity found around the USB3 issue
 *
 * So USB3 on RTL8821AU  is only for *power* ??
 *
 */

/*
 * Found on a MT7612U device
 * this is a 'true' USB3 wifi device
 * plugged in a USB2 HUB/Port

Binary Object Store Descriptor:
  bLength                 5
  bDescriptorType        15
  wTotalLength           22
  bNumDeviceCaps          2
  USB 2.0 Extension Device Capability:
    bLength                 7
    bDescriptorType        16
    bDevCapabilityType      2
    bmAttributes   0x0000f41e
      BESL Link Power Management (LPM) Supported
    BESL value     1024 us 
    Deep BESL value    61440 us 
  SuperSpeed USB Device Capability:
    bLength                10
    bDescriptorType        16
    bDevCapabilityType      3
    bmAttributes         0x00
    wSpeedsSupported   0x000e
      Device can operate at Full Speed (12Mbps)
      Device can operate at High Speed (480Mbps)
      Device can operate at SuperSpeed (5Gbps)
    bFunctionalitySupport   1
      Lowest fully-functional device speed is Full Speed (12Mbps)
    bU1DevExitLat          10 micro seconds
    bU2DevExitLat         180 micro seconds
can't get debug descriptor: Resource temporarily unavailable
 */
 
/*
 * This is the *only* thing I get from the RTL8821AU devices
 * which are supposed to be USB3

Device Qualifier (for other device speed):
  bLength                10
  bDescriptorType         6
  bcdUSB               2.00
  bDeviceClass            0 
  bDeviceSubClass         0 
  bDeviceProtocol         0 
  bMaxPacketSize0        64
  bNumConfigurations      1

*/

#ifdef CONFIG_RTLWIFI

#include <../drivers/net/wireless/realtek/rtlwifi/wifi.h>
#include <../drivers/net/wireless/realtek/rtlwifi/base.h>

#else

#include <drv_types.h>
#include "dm.h"
#include "phy.h"
#include "reg.h"
#include "fw.h"
#include "quirks.h"

#endif

/*
*/

#define MAX_USBCTRL_VENDORREQ_TIMES		10
#define REALTEK_USB_VENQT_WRITE          	0x40
#define REALTEK_USB_VENQT_READ             	0xC0
#define REALTEK_USB_VENQT_CMD_REQ  		0x05
#define REALTEK_USB_VENQT_CMD_IDX		0x00
#define MAX_USBCTRL_VENDORREQ_TIMES             10

#define REALTEK_USB_VENQT_MAX_BUF_SIZE		254

static int __usbctrl_vendorreq_sync_read(struct usb_device *udev, u8 request,
					 u16 value, u16 index, void *pdata,
					 u16 len)
{
	unsigned int pipe;
	int status;
	u8 reqtype;
	int vendorreq_times = 0;
	static int count;
	const u16 databuf_maxlen = REALTEK_USB_VENQT_MAX_BUF_SIZE;
	__le32 *databuf;

	pipe = usb_rcvctrlpipe(udev, 0); /* read_in */
	reqtype =  REALTEK_USB_VENQT_READ;

	databuf = kzalloc(databuf_maxlen, GFP_ATOMIC);
	if (!databuf) {
		return -ENOMEM;
	}

	/* data are already in little-endian order */
	memcpy(databuf, pdata, len);

	do {
		status = usb_control_msg(udev, pipe, request, reqtype, value,
					 index, databuf, len, 1000);
		if (status >= 0)
			break;

	} while (++vendorreq_times < MAX_USBCTRL_VENDORREQ_TIMES);

	kfree(databuf);

	if (status < 0 && count++ < 4)
		pr_err("reg 0x%x, usbctrl_vendorreq TimeOut! status:0x%x value=0x%x\n",
		       value, status, *(u32 *)pdata);
	return status;
}

static int __usbctrl_vendorreq_sync_write(struct usb_device *udev, u8 request,
					 u16 value, u16 index, void *pdata,
					 u16 len)
{
	unsigned int pipe;
	int status;
	u8 reqtype;
	int vendorreq_times = 0;
	static int count;
	const u16 databuf_maxlen = REALTEK_USB_VENQT_MAX_BUF_SIZE;
	__le32 *databuf;

	pipe = usb_sndctrlpipe(udev, 0); /* write_out */
	reqtype =  REALTEK_USB_VENQT_WRITE;

	databuf = kzalloc(databuf_maxlen, GFP_ATOMIC);
	if (!databuf) {
		return -ENOMEM;
	}

	/* data are already in little-endian order */
	memcpy(databuf, pdata, len);

	do {
		status = usb_control_msg(udev, pipe, request, reqtype, value,
					 index, databuf, len, 1000);
		if (status >= 0)
			break;

	} while (++vendorreq_times < MAX_USBCTRL_VENDORREQ_TIMES);

	kfree(databuf);

	if (status < 0 && count++ < 4)
		pr_err("reg 0x%x, usbctrl_vendorreq TimeOut! status:0x%x value=0x%x\n",
		       value, status, *(u32 *)pdata);
	return status;
}

u32 __usb_read_sync(struct usb_device *udev, u32 addr, u16 len)
{
	u8 request;
	u16 wvalue;
	u16 index;
	__le32 data;

	request = REALTEK_USB_VENQT_CMD_REQ;
	index = REALTEK_USB_VENQT_CMD_IDX; /* n/a */

	wvalue = (u16)addr;
	__usbctrl_vendorreq_sync_read(udev, request, wvalue, index, &data, len);
	return le32_to_cpu(data);
}

u8 __usb_read8_sync(struct usb_device *udev, u32 addr)
{
	return (u8)__usb_read_sync(udev, addr, 1);
}


void __usb_write_sync(struct usb_device *udev, u32 addr, u32 val,
			     u16 len)
{
	u8 request;
	u16 wvalue;
	u16 index;
	__le32 data;

	request = REALTEK_USB_VENQT_CMD_REQ;
	index = REALTEK_USB_VENQT_CMD_IDX; /* n/a */
	wvalue = (u16)(addr&0x0000ffff);
	data = cpu_to_le32(val);
	__usbctrl_vendorreq_sync_write(udev, request, wvalue, index, &data,
				       len);
}

void __usb_write8_sync(struct usb_device *udev, u32 addr, u8 val)
{
	__usb_write_sync(udev, addr, val, 1);
}


/*
 * Realtek use some 'clever' method to detect SUPER_SPEED USB capable
 * devices can run in this mode
 * The purpose of this function to set into USB3 or USB2
 */


bool rtl8812au_test_usb3_switch(struct usb_device *udev, int speed)
{
	bool ret = false;

	if (udev == NULL)
		dev_info(&udev->dev, "NULL Pointer in rtl8812au_test_usb3_switch : udev\n");

	dev_info(&udev->dev, "rtl8812au_test_usb3_switch : %04x:%04x\n",
		 (int) le16_to_cpu(udev->descriptor.idVendor),
		 (int) le16_to_cpu(udev->descriptor.idProduct));

	switch (speed) {
	case USB_SPEED_SUPER:
		dev_info(&udev->dev, "try switch from USB2 to USB3\n");
		if (udev->speed == USB_SPEED_HIGH) {
			dev_info(&udev->dev, "device running in USB2 mode\n");
			/* check if transceiver can see USB3 lines */
			if ((__usb_read8_sync(udev, 0x74) & (BIT(2)|BIT(3))) != BIT(3)) {
				dev_info(&udev->dev, "switch to USB3\n");

				__usb_write8_sync(udev, 0x74, 0x8);
				__usb_write8_sync(udev, 0x70, 0x2);
				__usb_write8_sync(udev, 0x3e, 0x1);
				__usb_write8_sync(udev, 0x3d, 0x3);
				/* usb disconnect */

				dev_info(&udev->dev, "do reconnect");
				__usb_write8_sync(udev, 0x5, 0x80);
				dev_info(&udev->dev, "try settle down");
				msleep(5);
				ret = true;
			}
		} else if (udev->speed == USB_SPEED_SUPER) {
			dev_info(&udev->dev, "in USB3 do unknown command\n");

			__usb_write8_sync(udev, 0x70, __usb_read8_sync(udev, 0x70) & (~BIT(1)));
			__usb_write8_sync(udev, 0x3e, __usb_read8_sync(udev, 0x3e) & (~BIT(0)));
		}
		break;

	case USB_SPEED_HIGH:
		dev_info(&udev->dev, "try switch from USB3 to USB2\n");
		if (udev->speed == USB_SPEED_SUPER) {
			dev_info(&udev->dev, "device running in USB3 mode\n");
			/* check if transceiver can see USB2 lines */
			if ((__usb_read8_sync(udev, 0x74) & (BIT(2)|BIT(3))) != BIT(2)) {
				__usb_write8_sync(udev, 0x74, 0x4);
				__usb_write8_sync(udev, 0x70, 0x2);
				__usb_write8_sync(udev, 0x3e, 0x1);
				__usb_write8_sync(udev, 0x3d, 0x3);
				/* usb disconnect */

				dev_info(&udev->dev, "do reconnect");
				__usb_write8_sync(udev, 0x5, 0x80);
				dev_info(&udev->dev, "try settle down");
				msleep(5);
				ret = true;
			}
		} else if (udev->speed == USB_SPEED_HIGH) {
			dev_info(&udev->dev, "in USB2 do unknown command\n");

			__usb_write8_sync(udev, 0x70, __usb_read8_sync(udev, 0x70) & (~BIT(1)));
			__usb_write8_sync(udev, 0x3e, __usb_read8_sync(udev, 0x3e) & (~BIT(0)));
		}
		break;
	}

	return ret;
}

