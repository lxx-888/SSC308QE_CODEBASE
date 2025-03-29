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
# Makefile for SStar cam_os_wrapper driver.
#

subdir-ccflags-y := -DSSTAR_CODING_STYLE_SANITIZE_MODULE
CONFIG_SSTAR_CHIP_NAME := $(subst ",,$(CONFIG_SSTAR_CHIP_NAME))
# general options
EXTRA_CFLAGS      +=  -I$(srctree)/drivers/sstar/include
EXTRA_CFLAGS      +=  -I$(srctree)/drivers/sstar/include/$(CONFIG_SSTAR_CHIP_NAME)
EXTRA_CFLAGS      +=  -I$(srctree)/drivers/sstar/cam_os_wrapper
EXTRA_CFLAGS      +=  -I$(srctree)/drivers/sstar/cam_os_wrapper/pub
EXTRA_CFLAGS      +=  -I$(srctree)/drivers/sstar/cam_device_wrapper/pub
ifndef CONFIG_CC_IS_CLANG
EXTRA_CFLAGS      +=  -Wno-sync-nand
endif
EXTRA_CFLAGS      +=  -Wno-sync-fetch-and-nand-semantics-changed
EXTRA_CFLAGS      +=  -Wno-unused-variable -Wno-unused-label -Wno-unused-function

# specific options

# files
obj-$(CONFIG_CAM_DEVICE_WRAPPER) += src/cam_device_wrapper.o


# export header files
EXPORT_H_FILES    += cam_device_wrapper.h
