rtl8821au linux (or rtl8812au)
==============================

rtl8821 linux kernel driver for AC1200 (801.11ac) 
Wireless Dual-Band USB Adapter

This driver is for rtl8812au *and* rtl8821au devices
I found this driver on the Edimax Site
original file :
EW-7822UAC_Linux_driver_v1.0.1.6.zip source file to

Why this name, look in the 
driver/net/wireless/rtlwifi
and you will see ;-)

ISSUES:
- With low traffic (150kBit/s), the driver will go into low power mode.
  (currently fixed, diabled powersaving)

TESTED DEVICES:  
* D-Link DWA 171  
* TP-Link T4U AC 1200  

with kernel 3.19 and up, lower kernel down to 3.10 will work too.

STATUS:
Currently driver works with old wireless extension *only*
Support for 'iw' only ap and sta modes, no monitor mode !!!

Build type  
`make`  
in sourcetree

and  
`sudo insmod rtl8821au.ko`  
will load the driver

for installing the needed firmware type  
`sudo make installfw`  

INFO:  
If you have a Belkin AC950 please contact me.
I need some information about the leds (how many)
This device will work, but the driver uses three leds.

For crosscompile use  
ARCH= CROSS_COMPILE= KSRC=

i.e.  
'make ARCH="mips" CROSS_COMPILE=mipsel-softfloat-linux-uclibc- KSRC=/home/user/openwrt/build_dir/target-mipsel_24kec+dsp_uClibc-0.9.33.2/linux-ramips_rt305x/linux-3.18.7/ modules'  

TOTO:  
- (more) checkpatch fixes, code rewriting  
- move to rtlwife-lib  
- regulation fix for 2.4G/5G band (errors currently disabled)

Hans Ulli Kroll <ulli.kroll@googlemail.com>
