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

ccflags-y += -I$(srctree)/$(src)/../cmdq/drv/pub
ccflags-y += -I$(srctree)/$(src)/../common
ccflags-y += -I$(srctree)/$(src)/drv/inc
ccflags-y += -I$(srctree)/$(src)/drv/pub
ccflags-y += -I$(srctree)/$(src)/drv/inc/uboot
ccflags-y += -I$(srctree)/$(src)/hal/$(CONFIG_SYS_CONFIG_NAME)/inc
ccflags-y += -I$(srctree)/$(src)/hal/$(CONFIG_SYS_CONFIG_NAME)/pub
ccflags-y += -I$(srctree)/$(src)/hal/common

obj-y += drv/src/mhal_rgn.o
#obj-y += drv/src/uboot/drv_rgn_os.o
obj-y += hal/$(CONFIG_SYS_CONFIG_NAME)/src/hal_rgn_if.o
obj-y += hal/$(CONFIG_SYS_CONFIG_NAME)/src/hal_rgn_cmdq.o
obj-y += hal/$(CONFIG_SYS_CONFIG_NAME)/src/hal_rgn_gop.o

