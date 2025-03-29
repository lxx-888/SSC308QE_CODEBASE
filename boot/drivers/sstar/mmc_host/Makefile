#
# Copyright (c) [2019~2020] SigmaStar Technology.
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

ccflags-y += -I$(srctree)/$(src)/$(CONFIG_SYS_CONFIG_NAME)
ccflags-y += -I$(srctree)/$(src)
ccflags-y += -I$(srctree)/drivers/sstar/mmc_host/inc
ccflags-y += -I$(srctree)/drivers/sstar/mmc_host/inc/$(CONFIG_SYS_CONFIG_NAME)
ccflags-y += -I$(srctree)/drivers/sstar/include
ccflags-y += -I$(srctree)/drivers/sstar/include/$(CONFIG_SYS_CONFIG_NAME)
ccflags-y += -I$(srctree)/drivers/sstar/gpio
ccflags-y += -I$(srctree)/drivers/sstar/padmux

obj-y += sstar_mmc.o
obj-$(CONFIG_SSTAR_SUPPORT_SDIO) += sstar_sdio.o
obj-y += src/hal_sdmmc_v5.o
obj-y += src/hal_card_intr_v5.o
obj-y += src/hal_card_timer.o
obj-y += src/hal_card_regs.o
obj-y += src/hal_card_platform_common.o
obj-y += src/$(subst ",,$(CONFIG_SYS_CONFIG_NAME))/hal_card_platform.o
