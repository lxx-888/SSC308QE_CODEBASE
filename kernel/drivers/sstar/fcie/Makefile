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
CONFIG_SSTAR_CHIP_NAME := $(subst ",,$(CONFIG_SSTAR_CHIP_NAME))

obj-y += drv_fcie.o
obj-y += hal/$(CONFIG_SSTAR_CHIP_NAME)/hal_fcie.o

EXTRA_CFLAGS      +=  -I$(srctree)/drivers/sstar/bdma/$(CONFIG_SSTAR_CHIP_NAME)
EXTRA_CFLAGS      +=  -I$(srctree)/drivers/sstar/include
EXTRA_CFLAGS      +=  -I$(srctree)/drivers/sstar/flash/
EXTRA_CFLAGS      +=  -I$(srctree)/drivers/sstar/flash/nand/
EXTRA_CFLAGS      +=  -I$(srctree)/drivers/sstar/flash/nor/
EXTRA_CFLAGS      +=  -I$(srctree)/drivers/sstar/flash/os/
EXTRA_CFLAGS      +=  -I$(srctree)/drivers/sstar/fsp_qspi/
EXTRA_CFLAGS      +=  -I$(srctree)/drivers/mtd
EXTRA_CFLAGS      +=  -I$(srctree)/drivers/sstar/fcie/
EXTRA_CFLAGS      +=  -I$(srctree)/drivers/sstar/fcie/hal/