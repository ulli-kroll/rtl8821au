rtl8821au linux (or rtl8812au)
==============================

rtl8821 linux kernel driver for AC1200 (801.11ac) 
Wireless Dual-Band USB Adapter

IMPORTANT UPDATE:  2015/04/06
-----------------------------
As of today the firmware binary is no longer part of
the kernel driver rtl8821au
You need to install the firmware binaries with

make installfw



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

ISSUES:
- With low traffic (150kBit/s), the driver will go into low power mode.
  (currently fixed, diabled powersaving)

TESTED DEVICES:
D-Link DWA 171
TP-Link T4U AC 1200
with kernel 3.19


STATUS:
Currently driver works with old wireless extension *only*
No support for iw 

INFO:
If you have a Belkin AC950 please contact me.
I need some information about the leds (how many)
This device will work, but the driver uses three leds.

BUILD:
make

to build this driver and install this with
insmod rtl8821au.ko

For crosscompiling your need the right toolchain !
i.e. for  Carambola with RT3052 (MIPS) and OpenWRT do

make ARCH="mips" CROSS_COMPILE=mipsel-softfloat-linux-uclibc- KSRC=/home/user/openwrt/build_dir/target-mipsel_24kec+dsp_uClibc-0.9.33.2/linux-ramips_rt305x/linux-3.18.7/ modules

TOTO:
- checkpatch fixes, code rewriting
- firmware loading support, the firmware files are already stripped
- move to rtlwife-lib
- regulation fix for 5G band when using TP-Link T4U

Hans Ulli Kroll <ulli.kroll@googlemail.com>
