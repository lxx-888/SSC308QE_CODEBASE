#CONFIG_TX_NO_CONFIRM
#CONFIG_USB_AGGR_URB_TX
#CONFIG_TX_NO_CONFIRM
#
# 可以用来选择不同是模式，CONFIG_USB_AGGR_URB_TX 定义上时一定需要定义CONFIG_USE_DMA_ADDR_BUFFER
# 1> CONFIG_USB_AGGR_URB_TX + CONFIG_TX_NO_CONFIRM，每个urb 发送多个txframe 并且不需要confirm，需要打开lmac的NO_NEED_CONFIRM 宏，并且ratecontrol在lmac 实现
# 2> CONFIG_USB_AGGR_URB_TX 每个urb 发送多个txframe 并且需要confirm，需要关闭lmac的NO_NEED_CONFIRM 宏
# 3> 所有宏都不定义，每个urb 发送一个txframe 并且需要confirm，并且urb直接使用skbbuffer，需要关闭lmac的NO_NEED_CONFIRM 宏
# 4> 定义CONFIG_USE_DMA_ADDR_BUFFER，每个urb 发送一个txframe 并且需要confirm，并且urb直接DMAbuffer，需要把数据从skb copy到DMAbuffer，需要关闭lmac的NO_NEED_CONFIRM 宏，
# 5> 定义CONFIG_USE_DMA_ADDR_BUFFER+ CONFIG_TX_NO_CONFIRM，每个urb 发送一个txframe 并且不需要confirm，并且urb直接DMAbuffer，需要把数据从skb copy到DMAbuffer，需要开lmac的NO_NEED_CONFIRM 宏，并且ratecontrol在lmac 实现
# 6> 定义CONFIG_TX_NO_CONFIRM，每个urb 发送一个txframe 并且不需要confirm，并且urb直接使用skbbuffer，需要打开lmac的NO_NEED_CONFIRM 宏，并且ratecontrol在lmac 实现
#
##对于调度慢+cpu强的板子使用 模式1 或者 模式2
##对于调度快+cpu慢的板子使用 模式3 或者 模式6
## 默认使用模式 3
#####################################################################################################################################################################################################################
k-dir:=$(if $(filter /%,$(src)),$(src),$(srctree)/$(src))
NOSTDINC_FLAGS := -I$(k-dir)/../include/ \
        -include $(k-dir)/../include/linux/compat-2.6.h \
        -DCOMPAT_STATIC

#export
ifeq ($(CONFIG_SSTAR_MODULE_NAME),)
CONFIG_SSTAR_MODULE_NAME = Sstar_wifi
endif
ifeq ($(CONFIG_SSTAR_WIFIIF1_NAME),)
CONFIG_SSTAR_WIFIIF1_NAME = "wlan%d"
endif
ifeq ($(CONFIG_SSTAR_WIFIIF2_NAME),)
CONFIG_SSTAR_WIFIIF2_NAME = "p2p%d"
endif
ifeq ($(CONFIG_SSTAR_USB_VID),)
CONFIG_SSTAR_USB_VID = 0x007a
endif
ifeq ($(CONFIG_SSTAR_USB_PID),)
CONFIG_SSTAR_USB_PID = 0x8888
endif

ifeq ($(CONFIG_SSTAR_SDIO_VID),)
CONFIG_SSTAR_SDIO_VID = 0x007a
endif
ifeq ($(CONFIG_SSTAR_SDIO_PID),)
CONFIG_SSTAR_SDIO_PID = 0x6011
endif
ifeq ($(CONFIG_SSTAR_MODULE_DRIVER_NAME),)
CONFIG_SSTAR_MODULE_DRIVER_NAME = "Sstar_wlan"
endif
ifeq ($(CONFIG_SSTAR_PLATFORM_DEVICE_NAME),)
CONFIG_SSTAR_PLATFORM_DEVICE_NAME = "Sstar_dev_wifi"
endif
ifeq ($(CONFIG_SSTAR_MODULE_PM_STAYAWK),)
CONFIG_SSTAR_PLATFORM_DEVICE_NAME = "pm_stayawake"
endif
ifeq ($(CONFIG_SSTAR_FW_NAME),)
CONFIG_SSTAR_FW_NAME="fw.bin"
endif
#export
SDIO_HOST      ?= $(shell echo $(CONFIG_SSTAR_SDIO_MMCx))
IF1NAME        ?= $(shell echo $(CONFIG_SSTAR_WIFIIF1_NAME))
IF2NAME        ?= $(shell echo $(CONFIG_SSTAR_WIFIIF2_NAME))
FW             ?= $(shell echo $(CONFIG_SSTAR_FW_NAME))
MODULES_NAME   ?= $(shell echo $(CONFIG_SSTAR_MODULE_NAME))
USBVID         ?= $(shell echo $(CONFIG_SSTAR_USB_VID))
USBPID         ?= $(shell echo $(CONFIG_SSTAR_USB_PID))
SDIOVID        ?=$(shell echo $(CONFIG_SSTAR_SDIO_VID))
SDIOPID        ?=$(shell echo $(CONFIG_SSTAR_SDIO_PID))
MODDRVNAME     ?= $(shell echo $(CONFIG_SSTAR_MODULE_DRIVER_NAME))
PLFDEVNAME     ?= $(shell echo $(CONFIG_SSTAR_PLATFORM_DEVICE_NAME))
MODPMSTAYAWK   ?= $(shell echo $(CONFIG_SSTAR_MODULE_PM_STAYAWK))

SAEAUTHEN      ?= $(CONFIG_SSTAR_FUNC_SAE_AUTHEN)
LOAD_FW_H      ?= $(CONFIG_SSTAR_USE_FIRMWARE_H)
SKB_DEBUG      ?= $(CONFIG_SSTAR_FUNC_SKB_DEBUG)
MEM_DEBUG      ?= $(CONFIG_SSTAR_FUNC_SKB_DEBUG)
BRIDGE         ?= $(CONFIG_SSTAR_SUPPORT_BRIDGE)
MONITOR        ?= $(CONFIG_SSTAR_FUNC_MONITOR)
EARLYSUSPEND   ?= $(CONFIG_SSTAR_FUNC_EARLYSUSPEND)
NOTXCONFIRM    ?= $(CONFIG_SSTAR_FUNC_NOTXCONFIRM)
CH5G           ?= $(CONFIG_SSTAR_FUNC_CHANNEL_5G_PRETEND_2G)
ONLY_HT20		?= $(CONFIG_SSTAR_WITHBAND_ONLY_HT20)
USBAGGTX       ?= $(CONFIG_SSTAR_FUNC_USB_AGGRTX)
USBDMABUFF     ?= $(CONFIG_SSTAR_FUNC_USB_DMABUFF)
USBCMDENHANCE  ?= $(CONFIG_SSTAR_FUNC_USB_CMD_ENHANCE)
USBDATAENHANCE ?= $(CONFIG_SSTAR_FUNC_USB_DATA_ENHANCE)
PMRELODDFW     ?= $(CONFIG_SSTAR_FUNC_PS_WAKEUP_RELOAD_FW)
USB_BUS        ?= $(CONFIG_SSTAR_USB_BUS)
SDIO_BUS       ?= $(CONFIG_SSTAR_SDIO_BUS)
SPI_BUS        ?= $(CONFIG_SSTAR_SPI_BUS)
CHECKSUM       ?= $(CONFIG_SSTAR_FUNC_HW_CHSUM)
SWRATECTRL     ?= $(CONFIG_SSTAR_SW_RATE_CTRL)
P2PENABLE      ?= $(CONFIG_SSTAR_FUNC_P2P_ENABLE)
SWENC          ?= $(CONFIG_SSTAR_FUNC_SW_ENC)
MODULEFS       ?= $(CONFIG_SSTAR_FUNC_MODULE_FS)
DEVCTRL        ?= $(CONFIG_SSTAR_FUNC_DEV_CTRL_API)
SMARTCONFIG    ?= $(CONFIG_SSTAR_FUNC_SMARTCONFIG)
CONFIG_ATHENAB ?= $(CONFIG_SSW101x)$(CONFIG_SSW101)
CONFIG_ARESB   ?= $(CONFIG_SSW101B)
CONFIG_HERA    ?= $(CONFIG_SSTAR6041)
NEED_SCOND_INTERFACE ?= $(CONFIG_NEED_P2P0_INTERFACE)
CUSTORMSPECIAL ?= y
CONFIG_NOT_SUPPORT_40M_CHW ?= $(CONFIG_SSW101x)
DRVLOADERFAST  ?= $(CONFIG_SSTAR_FUNC_DRV_LOADER_FAST)
PRI_IE         ?= $(CONFIG_SSTAR_FUNC_PRIVE_IE)
MONHDRPRISM         ?= $(CONFIG_SSTAR_FUNC_MONITOR_HDR_PRISM)
##################################################
#export
#ifeq ($(CONFIG_SSTAR_APOLLO),)
CONFIG_SSTAR_APOLLO=m
#endif


#ccflags-y += -DCONFIG_SSTAR_APOLLO_5GHZ_SUPPORT
#ccflags-y += -DCONFIG_SSTAR_APOLLO_WAPI_SUPPORT
#ccflags-y  += -I$(shell pwd)/../include/ \
        -include $(shell pwd)/../include/linux/compat-2.6.h \
        -DCOMPAT_STATIC
##指定配置文件路径，当配置文件存在时使用配置文件中的dcxo值不使用efuse中的dcxo，配置文件中的delta_gain为修正值，如果文件不存在则直接使用efuse中的数据
SSTAR_CONFIG_FILE="/tmp/Sstar_txpwer_dcxo_cfg.txt"
ccflags-y += -DCONFIG_TXPOWER_DCXO_VALUE=\"$(SSTAR_CONFIG_FILE)\"
# use the deltagain value in file :"$(SSTAR_CONFIG_FILE)",when file exist
SSTAR_RATE_POWER_CONFIG_FILE="/tmp/set_rate_power.txt"
ccflags-y += -DCONFIG_RATE_TXPOWER=\"$(SSTAR_RATE_POWER_CONFIG_FILE)\"

ifneq ($(IF1NAME),)
ccflags-y += -DCONFIG_IF1NAME=\"$(IF1NAME)\"
endif
ifneq ($(IF2NAME),)
ccflags-y += -DCONFIG_IF2NAME=\"$(IF2NAME)\"
endif
ifneq ($(USBVID),)
ccflags-y += -DCONFIG_USBVID=$(USBVID)
endif
ifneq ($(USBPID),)
ccflags-y += -DCONFIG_USBPID=$(USBPID)
endif
ifneq ($(SDIOVID),)
ccflags-y += -DCONFIG_SDIOVID=$(SDIOVID)
endif
ifneq ($(SDIOPID),)
ccflags-y += -DCONFIG_SDIOPID=$(SDIOPID)
endif


ifneq ($(MODDRVNAME),)
ccflags-y += -DCONFIG_MODDRVNAME=\"$(MODDRVNAME)\"
endif
ifneq ($(PLFDEVNAME),)
ccflags-y += -DCONFIG_PLFDEVNAME=\"$(PLFDEVNAME)\"
endif

ifneq ($(MODPMSTAYAWK),)
ccflags-y += -DCONFIG_MODPMSTAYAWK=\"$(PLFDEVNAME)\"
endif

SSTAR_MODULES_NAME ?= $(MODULES_NAME)
ifneq ($(FW),)
ccflags-y += -DCONFIG_FW_NAME=\"$(FW)\"
endif
ifneq ($(SDIO_HOST),)
ccflags-y += -DCONFIG_SSTAR_SDIO_MMC_ID=\"$(SDIO_HOST)\"
endif
ccflags-y += $(SSTAR_WIFI__EXT_CCFLAGS)
ccflags-y += -DCONFIG_SSTAR_APOLLO_DISABLE_BEACON_HINTS
ccflags-y += -DOLD_RATE_POLICY=1
#ccflags-y += -DCONFIG_SSTAR_SCAN_SPLIT
ccflags-y += -DCONFIG_MAC80211_BRIDGE_MULTI_PORT
ccflags-y += -DMINSTREL_RSSI_USED
ccflags-y += -DSSTAR_P2P_ADDR_USE_LOCAL_BIT
ccflags-y += -DSSTAR_WIFI_QUEUE_LOCK_BUG
ccflags-y += -DSSTAR_CHANGE_AP_TIMESTAMP_TO_BOOTTIME
ccflags-y += -DSSTAR_FUNCTION
#ccflags-y += -DSSTAR_USE_SAVED_FW
#ccflags-y += -DSSTAR_AP_SME
#ccflags-y += -DP2P_MULTIVIF
ccflags-y += -DSSTAR_P2P_CHANGE
ccflags-y += -DSSTAR_WIFI_NO_P2P_PS
ccflags-y += -DSSTAR_SPECIAL_PKG_DOWN_RATE
#ccflags-y += -DMCAST_FWDING
ccflags-y += -DSSTAR_APOLLO_SUSPEND_RESUME_FILTER_ENABLE
ccflags-y += -DCONFIG_SSTAR_IOCTRL
ccflags-y += -DOPER_CLOCK_USE_SEM
ccflags-y += -DEXIT_MODULE_RESET_USB=0
ccflags-y += -DSSTAR_VIF_LIST_USE_RCU_LOCK
#ccflags-y += -DSSTAR_SUPPORT_SMARTCONFIG
#ccflags-y += -DIEEE80211_TASKLET
#ccflags-y += -DCONFIG_RATE_HW_CONTROL
ccflags-y += -DCONFIG_SSTAR_SUPPORT_SG
#ccflags-y += -DCONFIG_SSTAR_SUPPORT_COUNTRY_CODE
#add AP+STA mode , sta channel change send event to hostapd
ccflags-y += -DCONFIG_SSTAR_AP_CHANNEL_CHANGE_EVENT
#ccflags-y += -DCONFIG_SSTAR_SELF_WORKQUEUE
ifeq ($(platform),PLATFORM_AMLOGIC_S805)
ccflags-y += -DCONFIG_SSTAR_APOLLO_USE_GPIO_IRQ
endif
ifeq ($(platform),PLATFORM_AMLOGIC)
#ccflags-y += -DCONFIG_SSTAR_APOLLO_USE_GPIO_IRQ
endif
#ccflags-y += -DCONFIG_SSTAR_APOLLO_USE_GPIO_IRQ
#Select GPIO23 or GPIO1 use for sdio irq
#ccflags-y += -DUSE_GPIO_23=1
ccflags-y += -DUSE_GPIO_1=1

#ccflags-y += -DCONFIG_SSTAR_GET_GPIO4

#ccflags-y += -DSSTAR_PKG_REORDER
ccflags-y += -DSSTAR_SUPPORT_WIDTH_40M
#ccflags-y += -DIPC_AP_USED_11G_NO_RTS
#p2p mode always in client mode, used in 4.4.2 android ,because monitor interface is not open
#(ANDRIOD_P2P in wpa_supplicant is open will cause this bug )
#ccflags-y += -DP2P_ALWAYS_CLIENT
#ccflags-y += -DSSTAR_11W_TEST

#wifi alliance test ,
#some function will cause pass wifi alliance certificate,but not good for some AP
#ccflags-y += -DWIFI_ALLIANCE_CERTIF
#ccflags-y += -DSSTAR_11W_TEST
#if used sdio download firmware
ccflags-y += -DWIFI_FW_DOWNLOAD
# Extra IE for probe response from upper layer is needed in P2P GO
# For offloading probe response to FW, the extra IE must be included
# in the probe response template
ccflags-y += -DSSTAR_PROBE_RESP_EXTRA_IE
#ccflags-y += -DCONFIG_SSTAR_APOLLO_DEBUG
#ccflags-y += -DCONFIG_SSTAR_STA_LISTEN
ccflags-y += -DCONFIG_SSTAR_SA_QUERY


ccflags-y += -DCONFIG_SSTAR_IWPRIV_USELESS
ccflags-y += -DCONFIG_SSTAR_SUPPORT_AP_CONFIG
#ccflags-y += -DCONFIG_SSTAR_STA_DYNAMIC_PS
ccflags-y += -DCONFIG_SSTAR_SUPPORT_REALTIME_CHANGE_MAC
ccflags-y += -DCONFIG_LMAC_RATECTRL_RATE_BUG_FIX
ccflags-y += -DAP_MODE_SEND_PROBE_REQ
#ccflags-y += -DAP_MODE_SUPPORT_SLEEP
ccflags-y += -DCONFIG_SSTAR_SET_COUNTRY_CODE
###################################################################
#used scond interface p2p0
###################################################################
ifeq ($(NEED_SCOND_INTERFACE),y)
ccflags-y += -DNEED_P2P0_INTERFACE=1
else
ccflags-y += -DNEED_P2P0_INTERFACE=0
endif

###################################################################
#monitor hdr prism
###################################################################
ccflags-$(MONHDRPRISM) += -DCONFIG_SSTAR_MONITOR_HDR_PRISM

###################################################################
#loader firmware fast
###################################################################
ccflags-$(DRVLOADERFAST) += -DCONDIF_SSTAR_CTRL_REQUEST_ASYNC
###################################################################
#private protocl
###################################################################
ccflags-$(PRI_IE) += -DSSTAR_PRIVATE_IE
#ifneq ($(PRI_IE),y)
ccflags-y += -DCONFIG_SSTAR_STA_LISTEN
#endif
###################################################################
#SSTAR_SUSPEND_REMOVE_INTERFACE
#before enter suspend state ,remove all interface
###################################################################
ccflags-$(PMRELODDFW) += -DSSTAR_SUSPEND_REMOVE_INTERFACE
###################################################################
#SSTAR_PM_USE_EARLYSUSPEND
#early suspend enbale
###################################################################
ccflags-$(EARLYSUSPEND) += -DSSTAR_PM_USE_EARLYSUSPEND
###################################################################
#SSTAR_ALLOC_MEM_DEBUG
#debug mem leakage
###################################################################
ccflags-$(MEM_DEBUG) += -DSSTAR_ALLOC_MEM_DEBUG
###################################################################
#SSTAR_ALLOC_SKB_DEBUG
#debug skb leakage
###################################################################
ccflags-$(SKB_DEBUG) += -DSSTAR_ALLOC_SKB_DEBUG
####################################################################
#CONFIG_USE_FW_H
#use firmware.h as firmware
####################################################################
ccflags-$(LOAD_FW_H) += -DCONFIG_USE_FW_H
####################################################################
#SSTAR_NOT_SUPPORT_40M_CHW
#only support 20m channel
####################################################################
#CONFIG_NOT_SUPPORT_40M_CHW=$(ONLY_HT20)
#ccflags-$(CONFIG_NOT_SUPPORT_40M_CHW) += -DSSTAR_NOT_SUPPORT_40M_CHW
ccflags-$(ONLY_HT20) += -DSSTAR_NOT_SUPPORT_40M_CHW
####################################################################
#SSTAR_SUPPORT_PKG_MONITOR
#monitor funciton
####################################################################
ccflags-$(MONITOR) += -DSSTAR_SUPPORT_PKG_MONITOR
####################################################################
#CONFIG_MAC80211_BRIDGE
#enable bridge
####################################################################
ccflags-$(BRIDGE) += -DCONFIG_MAC80211_BRIDGE
####################################################################
#CONFIG_TX_NO_CONFIRM
#enable tx noconfirm
####################################################################
ccflags-$(NOTXCONFIRM) += -DCONFIG_TX_NO_CONFIRM
####################################################################
#CONFIG_USB_AGGR_URB_TX
#enable usb aggr tx
####################################################################
ccflags-$(USBAGGTX) += -DCONFIG_USB_AGGR_URB_TX
####################################################################
#CONFIG_USE_DMA_ADDR_BUFFER
#urb use dma buff
####################################################################
ccflags-$(USBDMABUFF) += -DCONFIG_USE_DMA_ADDR_BUFFER
####################################################################
#CONFIG_WSM_CMD_XMIT_DIRECTLY
#cmd transmit directly
####################################################################
ccflags-$(USBCMDENHANCE) += -DCONFIG_WSM_CMD_XMIT_DIRECTLY
####################################################################
#CONFIG_USB_DATA_XMIT_DIRECTLY
#usb data xmit directly
####################################################################
ccflags-$(USBDATAENHANCE) += -DCONFIG_USB_DATA_XMIT_DIRECTLY
####################################################################
#CONFIG_SSTAR_5G_PRETEND_2G
#wifi use 5G channel to pretend 2g special channel
####################################################################
ccflags-$(CH5G) += -DCONFIG_SSTAR_5G_PRETEND_2G
####################################################################
#CONFIG_SSTAR_SUPPORT_P2P
#enable p2p function
###################################################################
ccflags-$(P2PENABLE) += -DCONFIG_SSTAR_SUPPORT_P2P
####################################################################
#rate ctrl select
#hw or sw
####################################################################
ifneq ($(SWRATECTRL),y)
ccflags-y += -DCONFIG_RATE_HW_CONTROL
else
CONFIG_MAC80211_SSTAR_RC_MINSTREL = y
CONFIG_MAC80211_SSTAR_RC_MINSTREL_HT = y
endif
####################################################################
####################################################################
#enable sw enc
####################################################################
ccflags-$(SWENC) += -DCONFIG_SSTAR_USE_SW_ENC
####################################################################
#CONFIG_SSTAR_MOULE_FS
#enable module fs
####################################################################
ccflags-$(MODULEFS) += -DCONFIG_SSTAR_MOULE_FS
####################################################################
#CONFIG_SSTAR_DEV_IOCTL
#enabel dev ioctl
####################################################################
ccflags-$(DEVCTRL) += -DCONFIG_SSTAR_DEV_IOCTL
####################################################################
#SSTAR_SUPPORT_SMARTCONFIG
#enabel smart config
####################################################################
ccflags-$(SMARTCONFIG) += -DSSTAR_SUPPORT_SMARTCONFIG
####################################################################
ccflags-$(SPECTRUMMGMT) += -DCONFIG_SSTAR_SPECTRUM_MGMT
####################################################################
ccflags-$(CUSTORMSPECIAL) += -DCONFIG_IEEE80211_SPECIAL_FILTER
####################################################################
#SAE authen for wpa3
#
####################################################################
ccflags-$(SAEAUTHEN) += -DCONFIG_SSTAR_SUPPORT_SAE
#CONFIG_SSTAR_PRODUCT_TEST_USE_GOLDEN_LED
#wifi product  test use Golden LED
####################################################################
ccflags-y += -DCONFIG_SSTAR_PRODUCT_TEST_USE_GOLDEN_LED
####################################################################
#SSTAR_PRODUCT_TEST_USE_FEATURE_ID
#wifi product test use feature ID
####################################################################
ccflags-y += -DSSTAR_PRODUCT_TEST_USE_FEATURE_ID

####################################################################
#sigmstar change iwlist scan print
#add noise, b/g/n ,max rate , need to change cfg80211,
#if need this feature
####################################################################
#ccflags-y += -DSIGMSTAR_SCAN_FEATURE
#ccflags-y += -DIPV6_FILTERING
#ccflags-y += -DCONFIG_SSTAR_APOLLO_BH_DEBUG
#ccflags-y += -DCONFIG_SSTAR_APOLLO_WSM_DEBUG
#ccflags-y += -DCONFIG_SSTAR_APOLLO_WSM_DUMPS
#ccflags-y += -DCONFIG_SSTAR_APOLLO_WSM_DUMPS_SHORT
#ccflags-y += -DCONFIG_SSTAR_APOLLO_TXRX_DEBUG
#ccflags-y += -DCONFIG_SSTAR_APOLLO_TX_POLICY_DEBUG
#ccflags-y += -DCONFIG_SSTAR_APOLLO_STA_DEBUG
#ccflags-y += -DCONFIG_SSTAR_APOLLO_DUMP_ON_ERROR
#ccflags-y += -DCONFIG_SSTAR_APOLLO_ITP
#ccflags-y += -DCONFIG_SSTAR_APOLLO_TESTMODE
# use the mac addr in file :"/data/.mac.info"
ccflags-y += -DCUSTOM_FEATURE_MAC
#ccflags-y += -DTEST_RF_POWER
#ccflags-y += -DUSB_CMD_UES_EP0
#ccflags-y += -DRESET_CHANGE
#ccflags-y += -DCONFIG_SSTARWIFI_WDT
#ccflags-y += -DCONFIG_SSTAR_ALIGNED_ACCESS


#PLATFORM_XUNWEI
#PLATFORM_SUN6I
#PLATFORM_FRIENDLY
#PLATFORM_SUN6I_64
#ccflags-y += -DSSTAR_WIFI_PLATFORM=PLATFORM_HIS
ifeq ($(CONFIG_ARES),y)
ccflags-y += -DPROJ_TYPE=ARES_A
endif
ifeq ($(CONFIG_ARESB),y)
ccflags-y += -DPROJ_TYPE=ARES_B
ccflags-$(CHECKSUM) += -DCHKSUM_HW_SUPPORT
ccflags-y += -DDPLL_CLOCK=DPLL_CLOCK_24M
endif

ifeq ($(CONFIG_HERA),y)
ccflags-y += -DPROJ_TYPE=HERA
ccflags-$(CHECKSUM) += -DCHKSUM_HW_SUPPORT
ccflags-y += -DDPLL_CLOCK=DPLL_CLOCK_24M
endif

ifeq ($(CONFIG_ATHENAB),y)
ccflags-y += -DPROJ_TYPE=ATHENA_B
ifeq ($(SDIO_BUS),y)
ccflags-y += -DSSTAR_SDIO_PATCH
ccflags-y += -DCONFIG_TX_NO_CONFIRM
endif
endif
ifeq ($(CONFIG_FPGA),y)
#ccflags-y += -DDCXO_USE_SMU_REG
ccflags-y += -DSDIO_CHANGE=1
#ccflags-y += -DRESET_CHANGE
#ccflags-y += -DTEST_DCXO_CONFIG=1
endif
#if TEST_DCXO_CONFIG is not config,DCXO_TYPE is not used
#if TEST_DPLL_CONFIG is not config,DPLL_CLOCK is not used
ifeq ($(CONFIG_1601),y)
ccflags-y += -DDCXO_USE_SMU_REG
ccflags-y += -DSDIO_CHANGE=0
ccflags-y += -DTEST_DCXO_CONFIG
ccflags-y += -DTEST_DPLL_CONFIG
ccflags-y += -DCHIP_V1601=1
ccflags-y += -DPROJ_TYPE=APOLLO_1601
ccflags-y += -DDCXO_TYPE=CRYSTAL_MODE
ccflags-y += -DDPLL_CLOCK=DPLL_CLOCK_40M
endif
ifeq ($(CONFIG_1606),y)
ccflags-y += -DDCXO_USE_SMU_REG
ccflags-y += -DSDIO_CHANGE=0
ccflags-y += -DTEST_DCXO_CONFIG
ccflags-y += -DTEST_DPLL_CONFIG
ccflags-y += -DCHIP_V1601=0
ccflags-y += -DPROJ_TYPE=APOLLO_1606
ccflags-y += -DDCXO_TYPE=CRYSTAL_MODE
ccflags-y += -DDPLL_CLOCK=DPLL_CLOCK_40M
endif
ifeq ($(CONFIG_APOLLOC),y)
#ccflags-y += -DDCXO_USE_SMU_REG
#ccflags-y += -DTEST_DCXO_CONFIG
#ccflags-y += -DTEST_DPLL_CONFIG
ccflags-y += -DCHIP_V1601=0
ccflags-y += -DSDIO_CHANGE=1
ccflags-y += -DPROJ_TYPE=APOLLO_C
ccflags-y += -DDCXO_TYPE=CRYSTAL_MODE
ccflags-y += -DDPLL_CLOCK=DPLL_CLOCK_40M
#ccflags-y += -DCONFIG_SSTAR_APOLLO_SUPPORT_SGI
endif

ifeq ($(CONFIG_APOLLOD),y)
ccflags-y += -DDCXO_USE_SMU_REG
ccflags-y += -DTEST_DCXO_CONFIG
ccflags-y += -DTEST_DPLL_CONFIG
ccflags-y += -DCHIP_V1601=0
ccflags-y += -DSDIO_CHANGE=1
ccflags-y += -DPROJ_TYPE=APOLLO_C
ccflags-y += -DDCXO_TYPE=CRYSTAL_MODE
ccflags-y += -DDPLL_CLOCK=DPLL_CLOCK_40M
#ccflags-y += -DCONFIG_SSTAR_APOLLO_SUPPORT_SGI
endif
ifeq ($(CONFIG_APOLLOE),y)
ccflags-y += -DCHIP_V1601=0
ccflags-y += -DSDIO_CHANGE=1
ccflags-y += -DPROJ_TYPE=APOLLO_E
ccflags-y += -DCONFIG_SSTAR_APOLLO_SUPPORT_SGI
endif
ifeq ($(CONFIG_ATHENAB_24M),y)
ccflags-y += -DDCXO_USE_SMU_REG
ccflags-y += -DTEST_DCXO_CONFIG
ccflags-y += -DTEST_DPLL_CONFIG
ccflags-y += -DPROJ_TYPE=ATHENA_B
ccflags-y += -DDCXO_TYPE=CRYSTAL_MODE
ccflags-y += -DDPLL_CLOCK=DPLL_CLOCK_24M
ccflags-y += -DSSTAR_SDIO_PATCH
ccflags-y += -DCONFIG_TX_NO_CONFIRM

endif
ifeq ($(CONFIG_ATHENALITE),y)
ccflags-y += -DCHKSUM_HW_SUPPORT
ccflags-y += -DCONFIG_USB_AGGR_URB_TX
ccflags-y += -DCONFIG_USE_DMA_ADDR_BUFFER
ccflags-y += -DPROJ_TYPE=ATHENA_LITE
endif
ifeq ($(CONFIG_ATHENALITE_ECO),y)
ccflags-y += -DCHKSUM_HW_SUPPORT
ccflags-y += -DCONFIG_USB_AGGR_URB_TX
ccflags-y += -DCONFIG_USE_DMA_ADDR_BUFFER
ccflags-y += -DPROJ_TYPE=ATHENA_LITE_ECO
endif
ifeq ($(CONFIG_ARES_24M),y)
#ccflags-y += -DDCXO_USE_SMU_REG
ccflags-y += -DTEST_DCXO_CONFIG
ccflags-y += -DTEST_DPLL_CONFIG
ccflags-y += -DPROJ_TYPE=ARES_A
ccflags-y += -DDCXO_TYPE=CRYSTAL_MODE
ccflags-y += -DDPLL_CLOCK=DPLL_CLOCK_24M
endif

ccflags-y += -DCONFIG_SSTAR_APOLLO_SUPPORT_SGI
ifeq ($(USB_BUS),y)
ccflags-y += -DUSB_BUS
ccflags-y += -DUSB_USE_TASTLET_TXRX
ccflags-y += -DUSB_BUS_BUG
#ccflags-y += -DCONFIG_USB_URB_RX_SUBMIT_DIRECTLY
#ccflags-y += -DCONFIG_WSM_CMD_XMIT_DIRECTLY
#ccflags-y += -DCONFIG_USB_DATA_XMIT_DIRECTLY
#ccflags-y += -DCONFIG_USE_DMA_ADDR_BUFFER
#ccflags-y += -DCONFIG_USB_AGGR_URB_TX
#ccflags-y += -DCONFIG_TX_NO_CONFIRM
#ccflags-y += -DSSTAR_IPC_ASSOC
#ccflags-y += -DSSTAR_PRIVATE_IE
#ccflags-y += -DSIGMASTAR_FILTER_MACADDR_ONLY
#ccflags-y += -DCONFIG_SSTAR_5G_PRETEND_2G
endif
ifeq ($(TX_NO_CONFIRM),y)
#ccflags-y += -DCONFIG_USE_DMA_ADDR_BUFFER
ccflags-y += -DCONFIG_USB_AGGR_URB_TX
ccflags-y += -DCONFIG_TX_NO_CONFIRM
endif
ifeq ($(SDIO_BUS),y)
ccflags-y += -DMCAST_FWDING
ccflags-y += -DRESET_CHANGE
ccflags-y += -DSDIO_BUS
#ccflags-y += -DCONFIG_SDIO_IRQ_THREAD_PROCESS_DATA
ccflags-y += -DCONFIG_SSTAR_SDIO_TX_HOLD
ccflags-y += -DCONFIG_SSTAR_SDIO_TX_THREAD_FIFO
#ccflags-y += -DCONFIG_SDIO_TXRX_WORKQUEUE
#ccflags-y += -DSSTAR_SDIO_TXRX_ENHANCE
#ccflags-y += -DSSTAR_WSM_SDIO_TX_MULT
#ccflags-y += -DRESET_CHIP
endif
ifeq ($(SPI_BUS),y)
ccflags-y += -DSPI_BUS
endif
#ccflags-y += -DPS_SETUP
MODFLAGS =-DMODULE -fno-pic
CFLAGS_MODULE =$(MODFLAGS)

ifeq ($(CONFIG_SSTAR_APOLLO_DEBUGFS),y)
ccflags-y  += -DCONFIG_SSTAR_APOLLO_DEBUGFS
endif
#ccflags-y  += -DCONFIG_SSTAR_APOLLO_DEBUG
ccflags-y += -DCONFIG_MAC80211_SSTAR_RC_DEFAULT_MINSTREL
#ccflags-y += -DMINSTREL_SAMPLE_SKIP_LOW_RATE_HT
#ccflags-y += -DMINSTREL_SAMPLE_SKIP_LOW_RATE

#ccflags-y+= -DCONFIG_MAC80211_LEDS
# ccflags-y += -DCONFIG_MAC80211_NOINLINE
# ccflags-y += -DCONFIG_MAC80211_VERBOSE_DEBUG
# ccflags-y += -DCONFIG_MAC80211_HT_DEBUG
# ccflags-y += -DCONFIG_MAC80211_TKIP_DEBUG
# ccflags-y += -DCONFIG_MAC80211_IBSS_DEBUG
# ccflags-y += -DCONFIG_MAC80211_VERBOSE_PS_DEBUG
# ccflags-y += -DCONFIG_MAC80211_VERBOSE_MPL_DEBUG
# ccflags-y += -DCONFIG_MAC80211_VERBOSE_MHWMP_DEBUG
# ccflags-y += -DCONFIG_MAC80211_DEBUG_COUNTERS
ccflags-y  += -DPROBE_RESP_EXTRA_IE
#ccflags-y  += -DIPV6_FILTERING
#ccflags-y += -DCONFIG_SSTAR_APOLLO_TESTMODE
#ccflags-y += -DCONFIG_NL80211_TESTMODE
ccflags-y += -DCONFIG_MAC80211_SSTAR_ROAMING_CHANGES
ifeq ($(CONFIG_MAC80211_SSTAR_DEBUGFS),y)
ccflags-y  += -DCONFIG_MAC80211_SSTAR_DEBUGFS
endif
ccflags-$(CONFIG_MAC80211_SSTAR_RC_MINSTREL) += -DCONFIG_MAC80211_SSTAR_RC_MINSTREL
ccflags-$(CONFIG_MAC80211_SSTAR_RC_MINSTREL_HT) += -DCONFIG_MAC80211_SSTAR_RC_MINSTREL_HT
$(SSTAR_MODULES_NAME)-y := \
        mac80211/main.o \
        mac80211/status.o \
        mac80211/sta_info.o \
        mac80211/scan.o \
        mac80211/ht.o mac80211/agg-rx.o \
        mac80211/mlme.o mac80211/work.o \
        mac80211/iface.o \
        mac80211/cfg.o \
        mac80211/rx.o \
        mac80211/tx.o \
        mac80211/util.o \
        mac80211/wme.o \
        mac80211/chan.o \
	mac80211/wext_cfg.o		\
	mac80211/Sstar_workqueue.o

$(SSTAR_MODULES_NAME)-$(SWENC) += \
        mac80211/wep.o \
        mac80211/wpa.o \
        mac80211/wapi.o \
        mac80211/tkip.o \
        mac80211/michael.o \
        mac80211/aes_ccm.o \
        mac80211/aes_cmac.o
$(SSTAR_MODULES_NAME)-y += mac80211/key.o
$(SSTAR_MODULES_NAME)-$(CUSTORMSPECIAL) += mac80211/special_filter.o
$(SSTAR_MODULES_NAME)-$(APSME)    += mac80211/ap_sme.o
$(SSTAR_MODULES_NAME)-$(MICREPORT)    += mac80211/event.o
$(SSTAR_MODULES_NAME)-$(SPECTRUMMGMT) += mac80211/spectmgmt.o
$(SSTAR_MODULES_NAME)-$(IBSSENABLE) += mac80211/ibss.o
$(SSTAR_MODULES_NAME)-$(P2PENABLE) += mac80211/offchannel.o
$(SSTAR_MODULES_NAME)-$(BRIDGE) += mac80211/bridge.o
$(SSTAR_MODULES_NAME)-$(MEM_DEBUG) += mac80211/ieee80211_Sstar_mem.o
$(SSTAR_MODULES_NAME)-$(SKB_DEBUG) += mac80211/ieee80211_Sstar_skb.o
$(SSTAR_MODULES_NAME)-$(SWRATECTRL) += mac80211/rate.o
$(SSTAR_MODULES_NAME)-$(CONFIG_MAC80211_SSTAR_LEDS) += mac80211/led.o
$(SSTAR_MODULES_NAME)-$(CONFIG_MAC80211_SSTAR_DEBUGFS) += \
        mac80211/debugfs.o \
        mac80211/debugfs_sta.o \
        mac80211/debugfs_netdev.o \
        mac80211/debugfs_key.o

$(SSTAR_MODULES_NAME)-$(CONFIG_MAC80211_SSTAR_MESH) += \
        mac80211/mesh.o \
        mac80211/mesh_pathtbl.o \
        mac80211/mesh_plink.o \
        mac80211/mesh_hwmp.o

$(SSTAR_MODULES_NAME)-$(CONFIG_PM) += mac80211/pm.o

$(SSTAR_MODULES_NAME)-$(CONFIG_MAC80211_SSTAR_DRIVER_API_TRACER) += mac80211/driver-trace.o
CFLAGS_driver-trace.o := -I$(src)

# objects for PID algorithm
rc80211_pid-$(CONFIG_MAC80211_SSTAR_DEBUGFS) := mac80211/rc80211_pid_algo.o
rc80211_pid-$(CONFIG_MAC80211_SSTAR_DEBUGFS) += mac80211/rc80211_pid_debugfs.o

rc80211_minstrel-y := mac80211/rc80211_minstrel.o
rc80211_minstrel-$(CONFIG_MAC80211_SSTAR_DEBUGFS) += mac80211/rc80211_minstrel_debugfs.o

rc80211_minstrel_ht-y := mac80211/rc80211_minstrel_ht.o
rc80211_minstrel_ht-$(CONFIG_MAC80211_SSTAR_DEBUGFS) += mac80211/rc80211_minstrel_ht_debugfs.o

$(SSTAR_MODULES_NAME)-$(CONFIG_MAC80211_SSTAR_RC_PID) += $(rc80211_pid-y)
$(SSTAR_MODULES_NAME)-$(CONFIG_MAC80211_SSTAR_RC_MINSTREL) += $(rc80211_minstrel-y)
$(SSTAR_MODULES_NAME)-$(CONFIG_MAC80211_SSTAR_RC_MINSTREL_HT) += $(rc80211_minstrel_ht-y)

$(SSTAR_MODULES_NAME)-y += \
		fwio.o \
		txrx.o \
		queue.o \
		wsm.o \
		sta.o \
		ap.o \
		scan.o \
		main.o \
		util.o \
		internal_cmd.o
$(SSTAR_MODULES_NAME)-$(P2PENABLE) += Sstar_p2p.o
$(SSTAR_MODULES_NAME)-$(MODULEFS) += module_fs.o
$(SSTAR_MODULES_NAME)-$(DEVCTRL) += dev_ioctl.o
$(SSTAR_MODULES_NAME)-$(SMARTCONFIG) += Sstar_smartconfig.o \
                                       smartconfig.o
$(SSTAR_MODULES_NAME)-$(DBGEVENT) += dbg_event.o
ifeq ($(USB_BUS),y)
$(SSTAR_MODULES_NAME)-y += apollo_usb.o Sstar_platform.o hwio_usb.o bh_usb.o
else ifeq ($(SDIO_BUS),y)
$(SSTAR_MODULES_NAME)-y += apollo_sdio.o Sstar_platform.o hwio_sdio.o bh_sdio.o dcxo_dpll.o
else ifeq ($(SPI_BUS),y)
$(SSTAR_MODULES_NAME)-y += apollo_spi.o Sstar_platform.o hwio_spi.o bh_spi.o
endif
$(SSTAR_MODULES_NAME)-$(SSTARDEBUG)  += debug.o
$(SSTAR_MODULES_NAME)-$(CONFIG_PM)  += pm.o


obj-$(CONFIG_SSTAR_APOLLO) += $(SSTAR_MODULES_NAME).o

ccflags-y += -D__CHECK_ENDIAN__
MODFLAGS =-DMODULE -fno-pic
CFLAGS_MODULE =$(MODFLAGS)

