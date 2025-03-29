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

ifeq ($(CONFIG_SSTAR_DISP_HDMITX_VGA),y)
EXTRA_CFLAGS += -DHDMITX_VGA_SUPPORTED
endif

ccflags-y += -I$(srctree)/$(src)/hal_common
ccflags-y += -I$(srctree)/$(src)/hal_common/$(CONFIG_SYS_CONFIG_NAME)
ccflags-y += -I$(srctree)/$(src)/hal_uboot
ccflags-y += -I$(srctree)/$(src)/include/common
ccflags-y += -I$(srctree)/$(src)/include/disp
ccflags-y += -I$(srctree)/$(src)/include/sys
ccflags-y += -I$(srctree)/$(src)/../common
ccflags-y += -I$(srctree)/$(src)/../cmdq/drv/pub
ccflags-y += -I$(srctree)/$(src)/../../include
ccflags-y += -I$(srctree)/$(src)/../../include/$(CONFIG_SYS_CONFIG_NAME)
ccflags-y += -I$(srctree)/$(src)/../../gpio
ccflags-y += -I$(srctree)/$(src)/../../padmux
ccflags-y += -I$(srctree)/$(src)/hal/pub
EXTRA_CFLAGS += -DDISP_UBOOT_SUPPORTED
obj-y +=./hal_uboot/drv_disp_os.o
obj-y +=./hal_common/drv_disp_if.o
obj-y +=./hal_common/drv_disp_ctx.o
obj-y +=./hal_common/drv_disp_irq.o
obj-y +=./hal_common/drv_disp_debug.o
obj-y +=./hal_common/$(CONFIG_SYS_CONFIG_NAME)/hal_disp.o
obj-y +=./hal_common/$(CONFIG_SYS_CONFIG_NAME)/hal_disp_if.o
obj-y +=./hal_common/$(CONFIG_SYS_CONFIG_NAME)/hal_disp_irq.o