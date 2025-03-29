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
# Makefile for SStar MIU_UT device drivers.
#

PROJ_ROOT=$(PWD)/../../../../..
KERNEL_PATH=$(PROJ_ROOT)/kernel
subdir-ccflags-y := -DSSTAR_CODING_STYLE_SANITIZER


EXTRA_CFLAGS      +=  -I$(srctree)/drivers/sstar/include/
EXTRA_CFLAGS      +=  -I$(srctree)/drivers/sstar/include/$(CONFIG_SSTAR_CHIP_NAME)
EXTRA_CFLAGS      +=  -I$(srctree)/drivers/sstar/miu
EXTRA_CFLAGS      +=  -I$(srctree)/drivers/sstar/miu/ut
EXTRA_CFLAGS      +=  -I$(srctree)/drivers/sstar/miu/$(CONFIG_SSTAR_CHIP_NAME)
EXTRA_CFLAGS      +=  -I$(srctree)/include/linux

EXTRA_CFLAGS      +=   -Wno-pointer-to-int-cast -Wno-unused-variable -Wno-unused-function -Wno-return-type -Wno-int-to-pointer-cast

ifneq ($(findstring miu/ut,$(PWD)),)
MIU_BUILD_METHOD := m
else
MIU_BUILD_METHOD := $(CONFIG_SSTAR_UT)
endif

obj-$(MIU_BUILD_METHOD)		+= miu_ut.o

all:
	@echo "kernel path:"$(KERNEL_PATH)
	make -C $(KERNEL_PATH)  M=$(PWD) modules
clean:
	make -C $(KERNEL_PATH)  M=$(PWD) clean
