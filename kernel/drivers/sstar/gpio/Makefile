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
# Makefile for MStar GPIO device drivers.
#
KASAN_SANITIZE_hal_pinmux.o := n

subdir-ccflags-y := -DSSTAR_CODING_STYLE_SANITIZE_MODULE
CONFIG_SSTAR_CHIP_NAME := $(subst ",,$(CONFIG_SSTAR_CHIP_NAME))
# general options
EXTRA_CFLAGS      +=  -I$(srctree)/drivers/sstar/include
EXTRA_CFLAGS      +=  -I$(srctree)/drivers/sstar/gpio
EXTRA_CFLAGS      +=  -I$(srctree)/drivers/sstar/gpio/os
EXTRA_CFLAGS      +=  -I$(srctree)/drivers/sstar/gpio/$(CONFIG_SSTAR_CHIP_NAME)
EXTRA_CFLAGS      +=  -I$(srctree)/drivers/sstar/include/$(CONFIG_SSTAR_CHIP_NAME)

# specific options
EXTRA_CFLAGS      +=  -DMSOS_TYPE_LINUX

# files
obj-$(CONFIG_SSTAR_GPIO)        += sstar_gpio.o
sstar_gpio-$(CONFIG_SSTAR_GPIO) += drv_gpio.o drv_gpio_io.o
sstar_gpio-$(CONFIG_SSTAR_GPIO) += $(CONFIG_SSTAR_CHIP_NAME)/hal_gpio_common.o
sstar_gpio-$(CONFIG_SSTAR_GPIO) += $(CONFIG_SSTAR_CHIP_NAME)/hal_gpio.o
sstar_gpio-$(CONFIG_SSTAR_GPIO) += $(CONFIG_SSTAR_CHIP_NAME)/hal_pinmux.o
