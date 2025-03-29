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

subdir-ccflags-y := -DSSTAR_CODING_STYLE_SANITIZE_MODULE
obj-$(CONFIG_SSTAR_SDMMC)      += kdrv_sdmmc.o

EXTRA_CFLAGS                += -I$(srctree)/drivers/sstar/include
EXTRA_CFLAGS                += -I$(srctree)/drivers/sstar/include/$(CONFIG_SSTAR_CHIP_NAME)
EXTRA_CFLAGS                += -I$(srctree)/drivers/sstar/sdmmc/drv/common
EXTRA_CFLAGS                += -I$(srctree)/drivers/sstar/sdmmc/drv/linux
EXTRA_CFLAGS                += -I$(srctree)/drivers/sstar/sdmmc/hal/inc/common
EXTRA_CFLAGS                += -I$(srctree)/drivers/sstar/sdmmc/hal/inc/$(CONFIG_SSTAR_CHIP_NAME)
EXTRA_CFLAGS                += -I$(srctree)/drivers/mmc/core/
EXTRA_CFLAGS                += -I$(srctree)/drivers/mmc/host

kdrv_sdmmc-y                += drv/common/drv_sdmmc_common.o
kdrv_sdmmc-y                += drv/linux/drv_sdmmc_lnx.o
kdrv_sdmmc-y                += drv/linux/drv_sdmmc_debug.o
kdrv_sdmmc-y                += hal/src/common/hal_sdmmc_regs.o
kdrv_sdmmc-y                += hal/src/common/hal_sdmmc_timer.o
kdrv_sdmmc-y                += hal/src/common/hal_sdmmc_intr.o
kdrv_sdmmc-y                += hal/src/common/hal_sdmmc_v5.o
kdrv_sdmmc-y                += hal/src/common/hal_sdmmc_platform_common.o
kdrv_sdmmc-y                += hal/src/$(subst ",,$(CONFIG_SSTAR_CHIP_NAME))/hal_sdmmc_platform.o
kdrv_sdmmc-$(CONFIG_SUPPORT_SDMMC_COMMAND) += drv/linux/drv_sdmmc_command.o
kdrv_sdmmc-$(CONFIG_SUPPORT_SDMMC_UT_VERIFY) += drv/linux/drv_sdmmc_verify.o
