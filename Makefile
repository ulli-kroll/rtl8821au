EXTRA_CFLAGS += -Wno-unused-label
EXTRA_CFLAGS += -Wno-unused

EXTRA_CFLAGS += -Wno-uninitialized

EXTRA_CFLAGS += -I$(src)/include

CONFIG_RTL8812A = y
CONFIG_RTL8821A = y

CONFIG_POWER_SAVING = y
CONFIG_PLATFORM_I386_PC = y

export TopDIR ?= $(shell pwd)

RTL8821AU_FILES	:=	usb.o \
			rtl8821au/dm.o \
			rtl8821au/fw.o \
			rtl8821au/hw.o \
			rtl8821au/led.o \
			rtl8821au/phy.o \
			rtl8821au/rf.o \
			rtl8821au/sw.o \
			rtl8821au/table.o \
			rtl8821au/trx.o

_OS_INTFS_FILES :=	os_dep/osdep_service.o \
			os_dep/os_intfs.o \
			os_dep/usb_ops_linux.o \
			os_dep/ioctl_linux.o \
			os_dep/xmit_linux.o \
			os_dep/mlme_linux.o \
			os_dep/recv_linux.o \
			os_dep/usb_halinit.o 

_HAL_INTFS_FILES :=	hal/hal_intf.o \
			hal/hal_com.o \
			hal/hal_phy.o \
			hal/odm.o

_HAL_INTFS_FILES +=	hal/HalPwrSeqCmd.o \
			hal/Hal8812PwrSeq.o \
			hal/Hal8821APwrSeq.o\
			hal/rtl8812a_xmit.o \
			hal/rtl8812a_hal_init.o \
			hal/rtl8812a_phycfg.o \
			hal/rtl8812a_dm.o \
			hal/rtl8812a_cmd.o \
			hal/rtl8812au_xmit.o \
			hal/rtl8812au_recv.o \
			hal/odm_RegConfig8821A.o

rtk_core :=		core/rtw_cmd.o \
			core/rtw_security.o \
			core/rtw_ioctl_set.o \
			core/rtw_ieee80211.o \
			core/rtw_mlme.o \
			core/rtw_mlme_ext.o \
			core/rtw_wlan_util.o \
			core/rtw_vht.o \
			core/rtw_pwrctrl.o \
			core/rtw_rf.o \
			core/rtw_recv.o \
			core/rtw_sta_mgt.o \
			core/rtw_ap.o \
			core/rtw_xmit.o	\
			core/efuse/rtw_efuse.o

EXTRA_CFLAGS += -DCONFIG_RTL8812A
EXTRA_CFLAGS += -DCONFIG_RTL8821A

MODULE_NAME = rtl8821au

$(MODULE_NAME)-y += 	$(rtk_core) \
			$(_OS_INTFS_FILES) \
			$(_HAL_INTFS_FILES) \
			$(_OUTSRC_FILES) \
			
$(MODULE_NAME)-y += $(RTL8821AU_FILES)

########### END OF PATH  #################################
ifeq ($(CONFIG_POWER_SAVING), y)
EXTRA_CFLAGS += -DCONFIG_POWER_SAVING
endif

ifeq ($(CONFIG_PLATFORM_I386_PC), y)
EXTRA_CFLAGS += -DCONFIG_LITTLE_ENDIAN
SUBARCH := $(shell uname -m | sed -e s/i.86/i386/)
ARCH ?= $(SUBARCH)
CROSS_COMPILE ?=
KVER  := $(shell uname -r)
KSRC := /lib/modules/$(KVER)/build
MODDESTDIR := /lib/modules/$(KVER)/kernel/drivers/net/wireless/
INSTALL_PREFIX :=
endif

ifneq ($(KERNELRELEASE),)

obj-$(CONFIG_RTL8812AU_8821AU) := $(MODULE_NAME).o

else

export CONFIG_RTL8812AU_8821AU = m

all: modules

modules:
	$(MAKE) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) -C $(KSRC) M=$(shell pwd)  modules

strip:
	$(CROSS_COMPILE)strip $(MODULE_NAME).ko --strip-unneeded

config_r:
	@echo "make config"
	/bin/bash script/Configure script/config.in

installfw:
	mkdir -p /lib/firmware/rtlwifi
	cp -n firmware/* /lib/firmware/rtlwifi/.

.PHONY: modules clean

clean:
	rm -fr */*.mod.c */*.mod */*.o */.*.cmd */*/.*.cmd */*/*/.*cmd
	rm -f *.mod.c *.o .*.cmd *.ko
	rm -f Module.symvers Module.markers modules.order
	rm -fr .tmp_versions
endif

