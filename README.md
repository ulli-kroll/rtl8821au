rtl8821au linux (or rtl8812au)
==============================

rtl8821/rtl8812 linux kernel driver Wireless Dual-Band USB Adapter

For issues with the hardware and driver see at the end of this readme.  
If one USB-ID is missing, please mail me.

Building and install driver
---------------------------

for building type  
`make`  

for load the driver  
`sudo insmod rtl8821au.ko`  

You need to install the needed fw with  
`sudo make installfw`  

If you need to crosscompile use  
`ARCH= CROSS_COMPILE= KSRC=`  
while calling  
`make`

i.e.  
`make ARCH="mips" CROSS_COMPILE=mipsel-softfloat-linux-uclibc- KSRC=/home/user/openwrt/build_dir/target-mipsel_24kec+dsp_uClibc-0.9.33.2/linux-ramips_rt305x/linux-3.18.7/ modules`  

This driver is based Edimax Linux driver  
EW-7822UAC_Linux_driver_v1.0.1.6.zip  
**and heavently reworked**

TESTED DEVICES:
---------------
* D-Link DWA 171  
* Digitus Wirelss AC  
* TP-Link T4U AC 1200  
* Linksys WUSB 6300  

with kernel 3.19 and up, lower kernel down to 3.10 will work too.

STATUS:
-------
* Currently driver works with old wireless extension **only**
* Support for 'iw' only ap and sta modes, no monitor mode !!!

ISSUES:
-------
- With low traffic (150kBit/s), the driver will go into low power mode. Currently maybe fixed.
- All USB3 devices will **not** work in USB3 (XHCI) mode.

EHCI mode on USB3 ports

Every USB3 port consists of a XHCI and EHCI controller. 
The RTL8812AU device connects only to the EHCI controller.  
And I don't know why. this in an HW issues.  
You can see this via `lsusb` or `usbview`.


TODO:
-----
- more checkpatch fixes, code rewriting  
- more adjustments for rtlwifi
- regulation fix for 2.4G/5G band (errors currently disabled)
- move intoi to rtlwifi, going upstream  


Hans Ulli Kroll <ulli.kroll@googlemail.com>
