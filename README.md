rtl8821au linux
======================

rtl8821 linux kernel driver for AC1200 (801.11ac) Wireless Dual-Band USB Adapter

NOTE:
I renamed this driver from the original rtl8812au driver found in the
EW-7822UAC_Linux_driver_v1.0.1.6.zip source file to

rtl8821au because this name (rtl8821ae) is also found in the linux kernel tree
drivers/net/wireless/rtlwifi/rtl8821ae
this driver work with both rtl8812ae and rtl8812ae chipsets

simple do a

make

to build this driver and install this with
insmod rtl8821au.ko

Currently this fireware is buil in trhe driver

