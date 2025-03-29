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


ccflags-y += -I$(srctree)/$(src)
ccflags-y += -I$(srctree)/include
ccflags-y += -I$(srctree)/$(src)/os
ccflags-y += -I$(srctree)/$(src)/$(CONFIG_SYS_CONFIG_NAME)
#ccflags-y += -I$(srctree)/include/$(CONFIG_SYS_CONFIG_NAME)
ccflags-y += -I$(srctree)/arch/arm/include/asm/arch/mach


# files
obj-$(CONFIG_SSTAR_IIC) += drv_iic.o $(CONFIG_SYS_CONFIG_NAME)/hal_iic.o

