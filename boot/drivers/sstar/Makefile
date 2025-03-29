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

obj-y += uart/
obj-$(CONFIG_SSTAR_AIO) += aio/
obj-$(CONFIG_SSTAR_AESDMA) += aesdma/
obj-$(CONFIG_SSTAR_BDMA) += bdma/
obj-$(CONFIG_SSTAR_CLK) += clk/
obj-$(CONFIG_SSTAR_EMAC) += emac/
obj-$(CONFIG_SSTAR_GMAC) += gmac/
obj-$(CONFIG_SSTAR_GPIO) += gpio/
obj-$(CONFIG_SSTAR_IIC) += iic/
obj-$(CONFIG_SSTAR_MMC_HOST) += mmc_host/
obj-$(CONFIG_SSTAR_FSP_QSPI) += fsp_qspi/
obj-$(CONFIG_SSTAR_FLASH) += flash/
ifneq ($(CONFIG_SSTAR_PCIE)$(CONFIG_SSTAR_PCIE_ENDPOINT),)
obj-y += pcie/
endif
obj-$(CONFIG_SSTAR_PADMUX) += padmux/
obj-$(CONFIG_SSTAR_PWM) += pwm/
obj-$(CONFIG_SSTAR_ADCLP) += sar/
obj-$(CONFIG_SSTAR_USB_DWC3) += usb/dwc3/
obj-$(CONFIG_SSTAR_USB_GADGET_MSB250X) += usb/gadget/udc/usb20/
obj-$(CONFIG_SSTAR_USB_EHCI) += usb/host/
obj-$(CONFIG_SSTAR_USB_PHY) += usb/phy/
obj-$(CONFIG_SSTAR_USB_RESET) += usb/phy/
obj-$(CONFIG_SSTAR_WDT) += wdt/
obj-$(CONFIG_PARTITIONS) += disk/
obj-$(CONFIG_SSTAR_CAMDRIVER) += camdriver/
obj-$(CONFIG_SSTAR_MSPI) += mspi/
obj-$(CONFIG_SSTAR_RPMB) += rpmb/
obj-$(CONFIG_SSTAR_CJSON_PARSER) += cjson/
obj-$(CONFIG_SSTAR_RTC)  += rtc/
obj-$(CONFIG_SSTAR_MCU)  += mcu/
obj-$(CONFIG_SSTAR_XZDEC) += xzdec/
obj-$(CONFIG_SSTAR_FCIE) += fcie/
