# SPDX-License-Identifier: GPL-2.0+

obj-y += fb_common.o
obj-y += fb_getvar.o
obj-y += fb_command.o
obj-$(CONFIG_FASTBOOT_FLASH_MMC) += fb_mmc.o
obj-$(CONFIG_FASTBOOT_FLASH_NAND) += fb_nand.o
obj-$(CONFIG_FASTBOOT_FLASH_SSTAR_CUS) += sstar_fb_cus.o
CFLAGS_sstar_fb_cus.o += -I$(srctree)/drivers/sstar/flash

obj-$(CONFIG_FASTBOOT_SSTAR_GETVAR) += sstar_fb_getvar.o
obj-$(CONFIG_FASTBOOT_SSTAR_CMDS) += sstar_fb_command.o
CFLAGS_sstar_fb_command.o += -I$(srctree)/drivers/sstar/rpmb
