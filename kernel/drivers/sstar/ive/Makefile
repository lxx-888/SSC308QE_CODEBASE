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

subdir-ccflags-y                := -DSSTAR_CODING_STYLE_SANITIZE_MODULE
ccflags-$(CONFIG_SSTAR_IVE_SYS) += -DSUPPORT_IVE_SYS_FS

ifeq ($(CONFIG_SSTAR_CHIP_NAME), "souffle")
    EXTRA_CFLAGS        += -DCONFIG_CHIP_SOUFFLE
    EXTRA_CFLAGS        += -DSUPPORT_BGBLUR
    EXTRA_CFLAGS        += -DSUPPORT_REQUEST_PRIORITY
    EXTRA_CFLAGS        += -DSUPPORT_BAT_AWLEN_PATCH
    EXTRA_CFLAGS        += -DSUPPORT_GMM_MULTIFRAME_MODE
else ifeq ($(CONFIG_SSTAR_CHIP_NAME), "iford")
    EXTRA_CFLAGS        += -DCONFIG_CHIP_IFORD
    EXTRA_CFLAGS        += -DSUPPORT_BGBLUR
    EXTRA_CFLAGS        += -DSUPPORT_REQUEST_PRIORITY
    EXTRA_CFLAGS        += -DSUPPORT_BACK_PRESSURE
endif

ifeq ($(CONFIG_SSTAR_CHIP_NAME), "souffle")
    CHIP                := souffle
else ifeq ($(CONFIG_SSTAR_CHIP_NAME), "iford")
    CHIP                := iford
endif
CHIP                    ?= iford

# general options
EXTRA_CFLAGS            += -I$(srctree)/drivers/sstar/include
EXTRA_CFLAGS            += -I$(srctree)/drivers/sstar/ive/drv/common
EXTRA_CFLAGS            += -I$(srctree)/drivers/sstar/ive/drv/linux
EXTRA_CFLAGS            += -I$(srctree)/drivers/sstar/ive/hal/$(CHIP)
EXTRA_CFLAGS            += -I$(srctree)/drivers/sstar/ive/hal/common

ccflags-$(CONFIG_SSTAR_IVE)    += -DMSOS_TYPE_LINUX_KERNEL

# files
obj-$(CONFIG_SSTAR_IVE) += sstar_ive.o
sstar_ive-y := drv/common/drv_ive_common.o \
               drv/common/drv_ive.o \
               drv/linux/drv_ive_clk.o \
               drv/linux/drv_ive_module.o \
               hal/$(CHIP)/hal_ive.o

ifeq ($(CONFIG_SSTAR_IVE_SYS), y)
    sstar_ive-y += drv/linux/drv_ive_sys.o
endif
