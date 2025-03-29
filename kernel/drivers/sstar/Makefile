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

#subdir-ccflags-y := -DSSTAR_CODING_STYLE_SANITIZE_MODULE

obj-y += irqchip/
obj-y += clocksource/
obj-y += uart/
ifneq ($(CONFIG_CAM_CLK),y)
obj-y += clk/
endif
obj-$(CONFIG_SSTAR_VOLTAGE_CTRL) += voltage/
obj-$(CONFIG_SSTAR_VOLTAGE_IDAC_CTRL) += voltage_idac/
obj-$(CONFIG_SSTAR_BDMA) += bdma/
obj-$(CONFIG_XOR_DMA) += xor_dma/
obj-y += msys/
#nor-flash bootup bringup
obj-y += cam_os_wrapper/
obj-y += cam_device_wrapper/
obj-y += cam_proc_wrapper/
obj-y += cam_drv_poll/
#obj-y += cam_drv_buffer/
obj-y += cam_fs_wrapper/
obj-y += cam_sysfs/
obj-y += cam_clkgen/
obj-$(CONFIG_CAM_CLK) += camclk/
obj-$(CONFIG_SSTAR_SDMMC) += sdmmc/
obj-$(CONFIG_USB_SUPPORT) += usb/host/
obj-$(CONFIG_SSTAR_NETPHY) += netphy/
obj-$(CONFIG_SSTAR_EMAC) += emac/
obj-$(CONFIG_SSTAR_IR) += ir/
obj-$(CONFIG_SSTAR_RTC) += rtc/
obj-$(CONFIG_SSTAR_GPIO) += gpio/
obj-$(CONFIG_SSTAR_PADMUX) += padmux/
obj-$(CONFIG_SSTAR_WATCHDOG) += watchdog/
obj-$(CONFIG_SSTAR_TIMER) += timer/
obj-$(CONFIG_SSTAR_ADCLP) += sar/
obj-$(CONFIG_MS_SARKEY) += sar_key/
obj-$(CONFIG_SSTAR_I2C) += i2c/
obj-$(CONFIG_MS_FLASH_ISP) += flash_isp/
obj-y += crypto/
obj-$(CONFIG_SSTAR_CPU_FREQ) += cpufreq/
obj-$(CONFIG_SSTAR_DEVFREQ) += devfreq/
obj-$(CONFIG_SSTAR_PWM) += pwm/
obj-$(CONFIG_MS_SPINAND) += spinand/
obj-$(CONFIG_SSTAR_MSPI) += mspi/
obj-$(CONFIG_SSTAR_PSPI) += pspi/
obj-$(CONFIG_SSTAR_IVE) += ive/
obj-$(CONFIG_MS_WARP) += warp/
obj-$(CONFIG_MS_NOTIFY) += notify/
obj-$(CONFIG_CAM_DRIVERS) += camdriver/
obj-$(CONFIG_DLA_DRIVER) += cambricon/
obj-$(CONFIG_MSTAR_MMAHEAP) += mma_heap/
obj-$(CONFIG_SSTAR_MIU) += miu/
obj-$(CONFIG_SS_SATA_HOST) += sata_host/
obj-$(CONFIG_SS_PROXYFS) += proxyfs/
obj-$(CONFIG_VCORE_DVFS) += vcore_dvfs/
obj-$(CONFIG_MS_IOPOWER) += iopower/
obj-y += cpu/
obj-$(CONFIG_USB_GADGET) += usb/gadget/
obj-$(CONFIG_SS_GYRO) += gyro/
obj-$(CONFIG_SS_KEYPAD) += keypad/
obj-$(CONFIG_SYSDESC) += sysdesc/
obj-y += rpmsg/
obj-$(CONFIG_SS_DUALOS) += dualos/
obj-$(CONFIG_SSTAR_FSP_QSPI) += fsp_qspi/
obj-$(CONFIG_SSTAR_FCIE) += fcie/
obj-$(CONFIG_SSTAR_FLASH) += flash/
obj-$(CONFIG_SSTAR_DUMMY_BATTERY) += power/
obj-y += copro/
obj-y += usb/phy/
obj-y += usb/dwc3/
obj-y += usb/typec/
obj-$(CONFIG_SSTAR_MCU) += mcu/
obj-$(CONFIG_SSTAR_UT) += ut/
obj-$(CONFIG_SSTAR_RIU_DBG) += riu/
obj-$(CONFIG_SSTAR_SEM) += sem/
obj-$(CONFIG_SSTAR_MBX) += mbx/
obj-$(CONFIG_SSTAR_UPDATE_DQ) += miu_dq/
obj-$(CONFIG_SSTAR_SWLA) += swla/
