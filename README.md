rtl8821au linux
======================

rtl8821 linux kernel driver for AC1200 (801.11ac) 
Wireless Dual-Band USB Adapter

This driver is for rtl8812au and rtl8821au devices
I found this driver on the Edimax Site
original file :
EW-7822UAC_Linux_driver_v1.0.1.6.zip source file to

However I have some newer one
rtl8812AU_linux_v4.3.8_12175.20140902.tar.gz
This one has not the needed sources for the rtl8821 devices. tough
Maybe this driver has the right tables I need. 

Why this name, look in the 
driver/net/wireless/rtlwifi
and you will see ;-)

I tested this driver with kernel version 3.18.7 on my main laptop and 
crosscompiled on Carambola (RT3052).
Build test done with v3.10 !

To build this do a simple

make

to build this driver and install this with
insmod rtl8821au.ko

For crosscompiling your need the right toolchain !
i.e. for  Carambola with RT3052 (MIPS) and OpenWRT do

make ARCH="mips" CROSS_COMPILE=mipsel-softfloat-linux-uclibc- KSRC=/home/user/openwrt/build_dir/target-mipsel_24kec+dsp_uClibc-0.9.33.2/linux-ramips_rt305x/linux-3.18.7/ modules

Currently the firmware is built in the driver.

STATUS:

This driver works with the old Wireless Extension WE or Wext API *ONLY*
see on
http://wireless.kernel.org/en/developers/Documentation/Wireless-Extensions

FUTURE
-------

To be more precisely :
The new API *will* be mac80211/cfg80211 framework.
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

Hans Ulli Kroll <ulli.kroll@googlemail.com>
