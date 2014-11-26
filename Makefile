EXTRA_CFLAGS += $(USER_EXTRA_CFLAGS)
EXTRA_CFLAGS += -O1
#EXTRA_CFLAGS += -O3
#EXTRA_CFLAGS += -Wall
#EXTRA_CFLAGS += -Wextra
#EXTRA_CFLAGS += -Werror
#EXTRA_CFLAGS += -pedantic
#EXTRA_CFLAGS += -Wshadow -Wpointer-arith -Wcast-qual -Wstrict-prototypes -Wmissing-prototypes

EXTRA_CFLAGS += -Wno-unused-variable
EXTRA_CFLAGS += -Wno-unused-value
EXTRA_CFLAGS += -Wno-unused-label
EXTRA_CFLAGS += -Wno-unused-parameter
EXTRA_CFLAGS += -Wno-unused-function
EXTRA_CFLAGS += -Wno-unused

EXTRA_CFLAGS += -Wno-uninitialized

EXTRA_CFLAGS += -I$(src)/include

CONFIG_RTL8812A = y
CONFIG_RTL8821A = y

CONFIG_POWER_SAVING = y
CONFIG_PLATFORM_I386_PC = y

export TopDIR ?= $(shell pwd)

RTL8821AU_FILES	:=	rtl8821au/dm.o \
			rtl8821au/phy.o \
			rtl8821au/rf.o \
			rtl8821au/table.o

_OS_INTFS_FILES :=	os_dep/osdep_service.o \
			os_dep/linux/os_intfs.o \
			os_dep/linux/usb_intf.o \
			os_dep/linux/usb_ops_linux.o \
			os_dep/linux/ioctl_linux.o \
			os_dep/linux/xmit_linux.o \
			os_dep/linux/mlme_linux.o \
			os_dep/linux/recv_linux.o

_HAL_INTFS_FILES :=	hal/hal_intf.o \
			hal/hal_com.o \
			hal/hal_phy.o \
			hal/led/hal_usb_led.o

_OUTSRC_FILES := hal/OUTSRC/odm_debug.o	\
		hal/OUTSRC/odm_HWConfig.o\
		hal/OUTSRC/odm.o\
		hal/OUTSRC/HalPhyRf.o

########### HAL_RTL8812A_RTL8821A #################################

ifneq ($(CONFIG_RTL8812A)_$(CONFIG_RTL8821A), n_n)

MODULE_NAME = rtl8821au

_HAL_INTFS_FILES +=  hal/HalPwrSeqCmd.o \
					hal/rtl8812a/Hal8812PwrSeq.o \
					hal/rtl8812a/Hal8821APwrSeq.o\
					hal/rtl8812a/rtl8812a_xmit.o\
					hal/rtl8812a/rtl8812a_sreset.o

_HAL_INTFS_FILES +=	hal/rtl8812a/rtl8812a_hal_init.o \
			hal/rtl8812a/rtl8812a_phycfg.o \
			hal/rtl8812a/rtl8812a_rf6052.o \
			hal/rtl8812a/rtl8812a_dm.o \
			hal/rtl8812a/rtl8812a_rxdesc.o \
			hal/rtl8812a/rtl8812a_cmd.o \
			hal/rtl8812a/usb/usb_halinit.o \
			hal/rtl8812a/usb/rtl8812au_led.o \
			hal/rtl8812a/usb/rtl8812au_xmit.o \
			hal/rtl8812a/usb/rtl8812au_recv.o

_HAL_INTFS_FILES += hal/rtl8812a/usb/usb_ops_linux.o

ifeq ($(CONFIG_RTL8812A), y)
EXTRA_CFLAGS += -DCONFIG_RTL8812A
_OUTSRC_FILES += hal/OUTSRC/rtl8812a/HalHWImg8812A_FW.o\
		hal/OUTSRC/rtl8812a/HalHWImg8812A_BB.o\
		hal/OUTSRC/rtl8812a/HalHWImg8812A_RF.o\
		hal/OUTSRC/rtl8812a/HalPhyRf_8812A.o\
		hal/OUTSRC/rtl8812a/odm_RegConfig8812A.o
endif

ifeq ($(CONFIG_RTL8821A), y)


EXTRA_CFLAGS += -DCONFIG_RTL8821A
_OUTSRC_FILES += hal/OUTSRC/rtl8821a/HalHWImg8821A_FW.o\
		hal/OUTSRC/rtl8821a/HalHWImg8821A_BB.o\
		hal/OUTSRC/rtl8821a/HalHWImg8821A_RF.o\
		hal/OUTSRC/rtl8812a/HalPhyRf_8812A.o\
		hal/OUTSRC/rtl8821a/HalPhyRf_8821A.o\
		hal/OUTSRC/rtl8821a/odm_RegConfig8821A.o
endif


endif

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

rtk_core :=	core/rtw_cmd.o \
		core/rtw_security.o \
		core/rtw_debug.o \
		core/rtw_io.o \
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
		core/rtw_sreset.o\
		core/efuse/rtw_efuse.o

$(MODULE_NAME)-y += $(rtk_core)

$(MODULE_NAME)-y += $(_OS_INTFS_FILES)
$(MODULE_NAME)-y += $(_HAL_INTFS_FILES)
$(MODULE_NAME)-y += $(_OUTSRC_FILES)

$(MODULE_NAME)-y += $(RTL8821AU_FILES)


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

.PHONY: modules clean

clean:
	rm -fr */*.mod.c */*.mod */*.o */.*.cmd */*/.*.cmd */*/*/.*cmd
	rm -f *.mod.c *.o .*.cmd *.ko
	rm -f Module.symvers Module.markers modules.order
	rm -fr .tmp_versions
endif

