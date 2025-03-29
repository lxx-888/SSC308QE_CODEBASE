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

ccflags-y += -I$(srctree)/$(src)/drv/hdmitx/inc
ccflags-y += -I$(srctree)/$(src)/drv/hdmitx/inc/uboot
ccflags-y += -I$(srctree)/$(src)/drv/hdmitx/pub
ccflags-y += -I$(srctree)/$(src)/drv/hdmitx/src/api/hdmitx
ccflags-y += -I$(srctree)/$(src)/drv/hdmitx/src/drv/hdmitx/include
ccflags-y += -I$(srctree)/$(src)/drv/mihdmitx/pub
ccflags-y += -I$(srctree)/$(src)/hal/$(CONFIG_SYS_CONFIG_NAME)/hdmitx/include
ccflags-y += -I$(srctree)/$(src)/hal/$(CONFIG_SYS_CONFIG_NAME)/hdmitx
ccflags-y += -I$(srctree)/$(src)/hal/$(CONFIG_SYS_CONFIG_NAME)/hdcp
ccflags-y += -I$(srctree)/$(src)/hal/$(CONFIG_SYS_CONFIG_NAME)/pub
ccflags-y += -I$(srctree)/$(src)/hal/$(CONFIG_SYS_CONFIG_NAME)/inc
ccflags-y += -I$(srctree)/$(src)/hal/pub
ccflags-y += -I$(srctree)/$(src)/../common

ccflags-y += -DHDMITX_OS_TYPE_UBOOT

obj-y +=./drv/hdmitx/src/uboot/drv_hdmitx_os.o
obj-y +=./drv/hdmitx/src/drv_hdmitx_debug.o
obj-y +=./drv/hdmitx/src/drv_hdmitx_if.o
obj-y +=./drv/hdmitx/src/drv_hdmitx_ctx.o
obj-y +=./drv/mihdmitx/src/mhal_hdmitx.o
obj-y +=./hal/$(CONFIG_SYS_CONFIG_NAME)/src/halHDMIUtilTx.o
obj-y +=./hal/$(CONFIG_SYS_CONFIG_NAME)/src/hal_hdmitx.o
obj-y +=./hal/$(CONFIG_SYS_CONFIG_NAME)/src/hal_hdmitx_if.o
