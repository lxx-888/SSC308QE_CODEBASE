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
#
# Makefile for SStar MIU device drivers.
#

subdir-ccflags-y := -DSSTAR_CODING_STYLE_SANITIZE_MODULE
CONFIG_SSTAR_CHIP_NAME := $(subst ",,$(CONFIG_SSTAR_CHIP_NAME))
ifdef CONFIG_MSTAR_PROJECT_NAME
    CONFIG_MSTAR_PROJECT_NAME := $(subst ",,$(CONFIG_MSTAR_PROJECT_NAME))
endif

# general options
EXTRA_CFLAGS      +=  -I$(srctree)/drivers/sstar/include/
EXTRA_CFLAGS      +=  -I$(srctree)/drivers/sstar/include/$(CONFIG_SSTAR_CHIP_NAME)
EXTRA_CFLAGS      +=  -I$(srctree)/drivers/sstar/miu
EXTRA_CFLAGS      +=  -I$(srctree)/drivers/sstar/miu/$(CONFIG_SSTAR_CHIP_NAME)
EXTRA_CFLAGS      +=  -I$(srctree)/include/linux
# specific options
# EXTRA_CFLAGS      += -DCONFIG_MIU_HW_MMU

ifneq ($(CONFIG_SSTAR_CHIP_NAME),ifado)
EXTRA_CFLAGS      += -DCONFIG_MIU_HW_MMU
# EXTRA_CFLAGS      += -DCONFIG_MIU_BWLA
endif


# files
obj-$(CONFIG_SSTAR_MIU)             := drv-miu.o
drv-miu-objs    :=  drv_miu.o \
                    drv_miu_sysfs.o \
					$(CONFIG_SSTAR_CHIP_NAME)/hal_miu_if.o \
					$(CONFIG_SSTAR_CHIP_NAME)/hal_miu.o \

obj-y += ut/
