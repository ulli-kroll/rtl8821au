rtl8821au linux
======================

rtl8821 linux kernel driver for AC1200 (801.11ac) Wireless Dual-Band USB Adapter

NOTE:
I renamed this driver from the original rtl8812au driver found in the
EW-7822UAC_Linux_driver_v1.0.1.6.zip source file to

rtl8821au because this name (rtl8821ae) is also found in the linux kernel tree
drivers/net/wireless/rtlwifi/rtl8821ae

rtl8821au is the usb port of the rtl8821ae device.

This driver works with both rtl8812ae and rtl8812ae chipsets !

simple do a

make

to build this driver and install this with
insmod rtl8821au.ko

Currently the firmware is built in the driver.

STATUS:

This driver works with the old Wireless Extension WE or Wext API
see on
http://wireless.kernel.org/en/developers/Documentation/Wireless-Extensions
The new API is ieee80211 framework.

So no support for the iw tool, thus no support for other modes as STA and AP

I'm currently rewriting the hal layer, the met with the hal layer in
drivers/net/wireless/rtlwifi


TESTED DEVICES:
This driver is tested with
D-Link DWA 171
TP-Link T4U AC 1200

FUTURE PLAN:

The new driver is located in
rtl8821au
You see the same layout as in linux sources
drivers/net(wireless/rtlwifi/rtl8821ae

TOTO:
- checkpatch fixes, code rewriting
- firmware loading support, the firmware files are already stripped
- move to rtlwife-lib
- regulation fix for 5G band when using TP-Link T4U

