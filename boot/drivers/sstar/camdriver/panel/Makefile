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

CONFIG_SYS_CONFIG_NAME := $(subst ",,$(CONFIG_SYS_CONFIG_NAME))

ccflags-y += -I$(srctree)/$(src)/../common
ccflags-y += -I$(srctree)/$(src)/../../include
ccflags-y += -I$(srctree)/$(src)/../../include/$(CONFIG_SYS_CONFIG_NAME)
ccflags-y += -I$(srctree)/$(src)/../../gpio
ccflags-y += -I$(srctree)/$(src)/../../padmux
ccflags-y += -I$(srctree)/$(src)/drv/mipnl/pub
ccflags-y += -I$(srctree)/$(src)/drv/pnl/inc
ccflags-y += -I$(srctree)/$(src)/drv/pnl/pub
ccflags-y += -I$(srctree)/$(src)/drv/pnl/inc/uboot
ccflags-y += -I$(srctree)/$(src)/hal/$(CONFIG_SYS_CONFIG_NAME)/inc
ccflags-y += -I$(srctree)/$(src)/hal/$(CONFIG_SYS_CONFIG_NAME)/pub
ccflags-y += -I$(srctree)/$(src)/hal/pub

obj-y += drv/mipnl/src/mhal_pnl.o
obj-y += drv/pnl/src/uboot/drv_pnl_os.o
obj-y += drv/pnl/src/drv_pnl_ctx.o
obj-y += drv/pnl/src/drv_pnl_if.o
obj-y += hal/$(CONFIG_SYS_CONFIG_NAME)/src/hal_pnl_if.o
obj-y += hal/$(CONFIG_SYS_CONFIG_NAME)/src/hal_pnl.o
