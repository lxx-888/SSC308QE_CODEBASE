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

subdir-ccflags-y := -DSSTAR_CODING_STYLE_SANITIZE_MODULE
CONFIG_SSTAR_CHIP_NAME := $(subst ",,$(CONFIG_SSTAR_CHIP_NAME))

EXTRA_CFLAGS      +=  -I$(srctree)/drivers/sstar/include
EXTRA_CFLAGS      +=  -I$(srctree)/drivers/sstar/usb/phy/include
EXTRA_CFLAGS      +=  -I$(srctree)/drivers/sstar/include/$(CONFIG_SSTAR_CHIP_NAME)

obj-$(CONFIG_SSTAR_USB3_PHY) += sstar-usb3-phy.o
obj-$(CONFIG_SSTAR_USB2_PHY) += sstar-usb2-phy.o

sstar-usb3-phy-y += sstar_usb3_phy_u3phy.o sstar_usb3_phy_utmi.o sstar_usb3_phy_pipe.o sstar_usb3_phy_port.o sstar_usb3_phy_pipe_debugfs.o sstar_usb3_phy_utmi_debugfs.o
sstar-usb2-phy-y += sstar_usb2_phy.o sstar_usb2_phy_debugfs.o

obj-$(CONFIG_SSTAR_USB_RESET_CONTROL) += reset-usb.o

