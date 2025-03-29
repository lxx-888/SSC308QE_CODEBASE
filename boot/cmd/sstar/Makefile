#
# Copyright (c) [2019~2022] SigmaStar Technology.
#
#
# This software is licensed under the terms of the GNU General Public
# License version 2, as published by the Free Software Foundation, and
# may be copied, distributed, and modified under those terms.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License version 2 for more details.
#

subdir-ccflags-y := -DSSTAR_CODING_STYLE_SANITIZE_MODULE

CONFIG_SYS_CONFIG_NAME := $(subst ",,$(CONFIG_SYS_CONFIG_NAME))

obj-y += cmd_sstar_common.o

# Android commands
obj-$(CONFIG_CMD_SSTAR_AB_RESET) += ab_reset.o
obj-$(CONFIG_CMD_SSTAR_RPMB) += rpmb.o

# Common commands
obj-$(CONFIG_CMD_SSTAR_AIO) += aio.o
obj-$(CONFIG_CMD_SSTAR_ESTAR) += estar.o
obj-$(CONFIG_CMD_SSTAR_UART) += uart.o
obj-$(CONFIG_CMD_SSTAR_RIU) += riu.o
obj-$(CONFIG_CMD_SSTAR_DEBUG) += debug.o
obj-$(CONFIG_CMD_SSTAR_MMC) += mmc.o
obj-$(CONFIG_CMD_SSTAR_MMC_FDISK) += mmc_fdisk.o
obj-$(CONFIG_CMD_SSTAR_SIGAUTH) += sigauth.o
obj-$(CONFIG_CMD_SSTAR_AES) += aes.o
obj-$(CONFIG_CMD_SSTAR_USBSTAR) += usbstar.o
obj-$(CONFIG_CMD_SSTAR_SPINAND) += spinand.o
obj-$(CONFIG_CMD_SSTAR_FLASH_PROGRAMMER) += flash_programmer.o
obj-$(CONFIG_CMD_SSTAR_FLASH_AUTOK) += flash_autok.o
obj-$(CONFIG_CMD_SSTAR_OTP) += otpctrl.o
obj-$(CONFIG_CMD_SSTAR_DISP) += disp.o
obj-$(CONFIG_CMD_SSTAR_JPD) += boot_jpd.o
obj-$(CONFIG_CMD_SSTAR_RGN) += rgn.o
obj-$(CONFIG_CMD_SSTAR_SRCFG) += srcfg.o
obj-$(CONFIG_CMD_SSTAR_BOOTLOGO) += bootlogo.o
obj-$(CONFIG_CMD_SSTAR_GMAC) += gmac.o
obj-$(CONFIG_CMD_SSTAR_PCIE) += pcie.o
obj-$(CONFIG_CMD_SSTAR_FWFS) += fwfs.o
obj-$(CONFIG_CMD_SSTAR_RNDIS) += rndis.o
obj-$(CONFIG_CMD_SSTAR_UFU) += ufu/
obj-$(CONFIG_CMD_SSTAR_ACM) += acm.o
obj-$(CONFIG_CMD_SSTAR_VENDOR) += vendor.o
obj-$(CONFIG_CMD_SSTAR_PM) += pm.o
obj-$(CONFIG_CMD_SSTAR_LOADOS) += loados.o
obj-$(CONFIG_CMD_SSTAR_VERSIG) += versig.o
obj-$(CONFIG_CMD_SSTAR_CHKOTP) += chkotp.o chkotp_data.o
obj-$(CONFIG_CMD_SSTAR_CPUWAIT) += cpu_wait.o
obj-$(CONFIG_CMD_SSTAR_RTC) += rtcpwc.o
obj-$(CONFIG_CMD_SSTAR_DDR_OTT) += ddr_ott.o
obj-$(CONFIG_CMD_SSTAR_BOOT_AB) += boot_ab.o

ifdef CONFIG_CMD_SSTAR_RPMB
CFLAGS_rpmb.o += -I$(srctree)/drivers/sstar/rpmb
endif

ifdef CONFIG_CMD_SSTAR_MMC
CFLAGS_mmc.o += -I$(srctree)/drivers/sstar/disk
endif
ifdef CONFIG_ARM_SMCCC
CFLAGS_otpctrl.o  += -I$(srctree)/drivers/tee/optee
endif

ifdef CONFIG_CMD_SSTAR_RGN
CFLAGS_rgn.o += -I$(srctree)/drivers/sstar/camdriver/cmdq/drv/pub
CFLAGS_rgn.o += -I$(srctree)/drivers/sstar/camdriver/common
CFLAGS_rgn.o += -I$(srctree)/drivers/sstar/camdriver/rgn/drv/pub
CFLAGS_rgn.o += -I$(srctree)/drivers/sstar/camdriver/rgn/drv/inc
CFLAGS_rgn.o += -I$(srctree)/drivers/sstar/camdriver/rgn/drv/inc/uboot
endif

ifdef CONFIG_CMD_SSTAR_JPD
CFLAGS_boot_jpd.o += -I$(srctree)/drivers/sstar/camdriver/common
ifdef CONFIG_JPD_SW
CFLAGS_boot_jpd.o += -I$(srctree)/drivers/sstar/camdriver/jpd/jpd_sw/inc
endif
ifdef CONFIG_JPD_HW
CFLAGS_boot_jpd.o += -I$(srctree)/drivers/sstar/camdriver/jpd/jpd_hw/drv/pub
endif
endif

ifdef CONFIG_CMD_SSTAR_DISP
CFLAGS_disp.o += -I$(srctree)/drivers/sstar/camdriver/cmdq/drv/pub
CFLAGS_disp.o += -I$(srctree)/drivers/sstar/camdriver/common
#CFLAGS_disp.o += -I$(srctree)/drivers/sstar/camdriver/disp/drv/disp/inc/uboot
#CFLAGS_disp.o += -I$(srctree)/drivers/sstar/camdriver/disp/drv/disp/inc
#CFLAGS_disp.o += -I$(srctree)/drivers/sstar/camdriver/disp/drv/disp/pub
#CFLAGS_disp.o += -I$(srctree)/drivers/sstar/camdriver/disp/drv/midisp/pub
#CFLAGS_disp.o += -I$(srctree)/drivers/sstar/camdriver/disp/hal/$(CONFIG_SYS_CONFIG_NAME)/inc
#CFLAGS_disp.o += -I$(srctree)/drivers/sstar/camdriver/disp/hal/$(CONFIG_SYS_CONFIG_NAME)/pub
#CFLAGS_disp.o += -I$(srctree)/drivers/sstar/camdriver/disp/hal/pub
#CFLAGS_disp.o += -I$(srctree)/drivers/sstar/camdriver/hdmitx/drv/mihdmitx/pub
#CFLAGS_disp.o += -I$(srctree)/drivers/sstar/camdriver/disp/drv/disp/inc/uboot
CFLAGS_disp.o += -I$(srctree)/drivers/sstar/camdriver/disp/hal_common
CFLAGS_disp.o += -I$(srctree)/drivers/sstar/camdriver/disp/hal_common/$(CONFIG_SYS_CONFIG_NAME)
CFLAGS_disp.o += -I$(srctree)/drivers/sstar/camdriver/disp/hal_uboot
CFLAGS_disp.o += -I$(srctree)/drivers/sstar/camdriver/disp/include/common
CFLAGS_disp.o += -I$(srctree)/drivers/sstar/camdriver/disp/include/disp
CFLAGS_disp.o += -I$(srctree)/drivers/sstar/camdriver/disp/include/sys
endif

ifdef CONFIG_CMD_SSTAR_BOOTLOGO
CFLAGS_bootlogo.o += -I$(srctree)/drivers/sstar/cjson
CFLAGS_bootlogo.o += -I$(srctree)/drivers/sstar/iniparser
CFLAGS_bootlogo.o += -I$(srctree)/drivers/sstar/camdriver/cmdq/drv/pub
CFLAGS_bootlogo.o += -I$(srctree)/drivers/sstar/camdriver/common
#CFLAGS_bootlogo.o += -I$(srctree)/drivers/sstar/camdriver/disp/drv/midisp/pub
#CFLAGS_bootlogo.o += -I$(srctree)/drivers/sstar/camdriver/hdmitx/drv/mihdmitx/pub
#CFLAGS_bootlogo.o += -I$(srctree)/drivers/sstar/camdriver/panel/drv/mipnl/pub
#CFLAGS_bootlogo.o += -I$(srctree)/drivers/sstar/camdriver/rgn/drv/pub
#CFLAGS_bootlogo.o += -I$(srctree)/drivers/sstar/camdriver/jpd/jpd_sw/inc
#CFLAGS_bootlogo.o += -I$(srctree)/drivers/sstar/camdriver/jpd/jpd_hw/drv/pub
CFLAGS_bootlogo.o += -I$(srctree)/cmd/sstar/include
CFLAGS_bootlogo.o += -I$(srctree)/drivers/sstar/camdriver/disp/hal_common
CFLAGS_bootlogo.o += -I$(srctree)/drivers/sstar/camdriver/disp/hal_common/$(CONFIG_SYS_CONFIG_NAME)
CFLAGS_bootlogo.o += -I$(srctree)/drivers/sstar/camdriver/disp/hal_uboot
CFLAGS_bootlogo.o += -I$(srctree)/drivers/sstar/camdriver/disp/include/common
CFLAGS_bootlogo.o += -I$(srctree)/drivers/sstar/camdriver/disp/include/disp
CFLAGS_bootlogo.o += -I$(srctree)/drivers/sstar/camdriver/disp/include/sys
CFLAGS_bootlogo.o += -I$(srctree)/drivers/sstar/camdriver/jpd/jpd_sw/inc
endif

ifdef CONFIG_CMD_SSTAR_SRCFG
CFLAGS_srcfg.o += -I$(srctree)/drivers/sstar/camdriver/common
CFLAGS_srcfg.o += -I$(srctree)/drivers/sstar/camdriver/sensor/drv/inc
CFLAGS_srcfg.o += -I$(srctree)/drivers/sstar/camdriver/sensor/hal/iford/pub
endif

ifdef CONFIG_CMD_SSTAR_GMAC
CFLAGS_gmac.o += -I$(srctree)/drivers/sstar/gmac/$(CONFIG_SYS_CONFIG_NAME)
endif

ifdef CONFIG_CMD_SSTAR_PCIE
CFLAGS_pcie.o += -I$(srctree)/drivers/sstar/pcie/intf/drv/pub
CFLAGS_pcie.o += -I$(srctree)/drivers/sstar/pcie/endpoint/drv/pub
endif

ifdef CONFIG_CMD_SSTAR_LOADOS
CFLAGS_loados.o += -I$(srctree)/drivers/sstar/flash/
CFLAGS_loados.o += -I$(srctree)/drivers/sstar/xzdec/
CFLAGS_loados.o += -I$(srctree)/drivers/sstar/xzdec/hal/
endif

ifdef CONFIG_CMD_SSTAR_FLASH_AUTOK
CFLAGS_flash_autok.o += -I$(srctree)/drivers/sstar/flash/
CFLAGS_flash_autok.o += -I$(srctree)/drivers/sstar/flash/os/
CFLAGS_flash_autok.o += -I$(srctree)/arch/arm/include/asm/arch-$(CONFIG_SYS_CONFIG_NAME)/mach/
CFLAGS_flash_autok.o += -I$(srctree)/drivers/sstar/bdma/$(CONFIG_SYS_CONFIG_NAME)/
endif

ifdef CONFIG_CMD_SSTAR_PM
ifdef CONFIG_SSTAR_MCU
CFLAGS_pm.o += -I$(srctree)/drivers/sstar/mcu
endif
ifdef CONFIG_SSTAR_FLASH
CFLAGS_pm.o += -I$(srctree)/drivers/sstar/fsp_qspi/drv/inc
endif
endif

ifdef CONFIG_SSTAR_RTC
CFLAGS_rtcpwc.o += -I$(srctree)/drivers/sstar/rtc
endif

ifdef CONFIG_CMD_SSTAR_DDR_OTT
CFLAGS_ddr_ott.o += -I$(srctree)/cmd/sstar/include
endif
