# SigmaStar trade secret
# Copyright (c) [2019~2022] SigmaStar Technology.
# All rights reserved.
#
# Unless otherwise stipulated in writing, any and all information contained
# herein regardless in any format shall remain the sole proprietary of
# SigmaStar and be kept in strict confidence
# (SigmaStar Confidential Information) by the recipient.
# Any unauthorized act including without limitation unauthorized disclosure,
# copying, use, reproduction, sale, distribution, modification, disassembling,
# reverse engineering and compiling of the contents of SigmaStar Confidential
# Information is unlawful and strictly prohibited. SigmaStar hereby reserves the
# rights to any and all damages, losses, costs and expenses resulting therefrom.
#

CONFIG_MSTAR_CHIP_NAME := $(subst ",,$(CONFIG_MSTAR_CHIP_NAME))
EXTRA_CFLAGS      +=  -Idrivers/sstar/camdriver/sensordriver/drv/inc
EXTRA_CFLAGS      +=  -Idrivers/sstar/camdriver/sensordriver/drv/pub
EXTRA_CFLAGS      +=  -Idrivers/sstar/camdriver/sensorif/drv/pub
EXTRA_CFLAGS      +=  -Idrivers/sstar/camdriver/sensorif/drv/inc
EXTRA_CFLAGS      +=  -Idrivers/sstar/include
EXTRA_CFLAGS      +=  -Idrivers/sstar/cam_os_wrapper/pub


MVXV_START := MVX3
MVXV_LIB_TYPE := I2
MS_PLATFORM_ID := __
COMMITNUMBER := g$(shell git log --format=%h -n 1 2> /dev/null)$(shell git diff --quiet || echo -dirty )
COMP_ID := [sensor_module]
MVXV_EXT := [$(shell git rev-parse --abbrev-ref HEAD 2> /dev/null | sed -e 's/\//_/g') build.$(shell date +%Y%m%d%H%M)
MVXV_END := ]XVM
EXTRA_CFLAGS += -DSENSOR_MODULE_VERSION="$(MVXV_START)$(MVXV_LIB_TYPE)$(MS_PLATFORM_ID)$(COMMITNUMBER)$(COMP_ID)$(MVXV_EXT)$(MVXV_END)"


obj-$(CONFIG_MS_SENSORDRIVER) +=  sensor_imx323.o
sensor_imx323-y := src/drv_ss_cus_imx323.o

obj-$(CONFIG_MS_SENSORDRIVER) +=  sensor_imx327.o
sensor_imx327-y := src/drv_ss_cus_imx327_mipi.o

obj-$(CONFIG_MS_SENSORDRIVER) +=  sensor_imx317_hdr.o
sensor_imx317_hdr-y := src/drv_ss_cus_imx317_mipi.o

obj-$(CONFIG_MS_SENSORDRIVER) +=  sensor_nvp6124b.o
sensor_nvp6124b-y := src/drv_ss_cus_nvp6124b.o

obj-$(CONFIG_MS_SENSORDRIVER) +=  sensor_nvp6158c.o
sensor_nvp6158c-y := src/drv_ss_cus_nvp6158c.o

obj-$(CONFIG_MS_SENSORDRIVER) +=  sensor_imx291.o
sensor_imx291-y := src/drv_ss_cus_imx291_mipi.o

obj-$(CONFIG_MS_SENSORDRIVER) +=  sensor_imx307_hdr.o
sensor_imx307_hdr-y := src/drv_ss_cus_imx307_mipi.o

obj-$(CONFIG_MS_SENSORDRIVER) +=  sensor_os08a10.o
sensor_os08a10-y := src/drv_ss_cus_os08a10_mipi.o

obj-$(CONFIG_MS_SENSORDRIVER) +=  sensor_ar0237_rgbir.o
sensor_ar0237_rgbir-y := src/drv_ss_cus_ar0237_RGBIR.o

obj-$(CONFIG_MS_SENSORDRIVER) +=  sensor_imx230.o
sensor_imx230-y := src/drv_ss_cus_imx230_mipi.o

obj-$(CONFIG_MS_SENSORDRIVER) +=  sensor_os05a10.o
sensor_os05a10-y := src/drv_ss_cus_os05a10_mipi.o

obj-$(CONFIG_MS_SENSORDRIVER) += sensor_imx274_hdr.o
sensor_imx274_hdr-y := src/drv_ss_cus_imx274_mipi.o

obj-$(CONFIG_MS_SENSORDRIVER) += sensor_ps5260.o
sensor_ps5260-y := src/drv_ss_cus_ps5260_mipi.o

obj-$(CONFIG_MS_SENSORDRIVER) += sensor_ps5268.o
sensor_ps5268-y := src/drv_ss_cus_ps5268_mipi.o

obj-$(CONFIG_MS_SENSORDRIVER) += sensor_SC4238.o
sensor_SC4238-y := src/drv_ss_cus_sc4238_mipi.o

obj-$(CONFIG_MS_SENSORDRIVER) += sensor_sc2315.o
sensor_sc2315-y := src/drv_ss_cus_sc2315_mipi.o

obj-$(CONFIG_MS_SENSORDRIVER) += sensor_ps5250.o
sensor_ps5250-y := src/drv_ss_cus_ps5250_mipi.o

obj-$(CONFIG_MS_SENSORDRIVER) += sensor_ps5520.o
sensor_ps5520-y := src/drv_ss_cus_ps5520_mipi.o

obj-$(CONFIG_MS_SENSORDRIVER) += sensor_imx412_hdr.o
sensor_imx412_hdr-y := src/drv_ss_cus_imx412_mipi.o

obj-$(CONFIG_MS_SENSORDRIVER) += sensor_imx415_hdr.o
sensor_imx415_hdr-y := src/drv_ss_cus_imx415_mipi.o

obj-$(CONFIG_MS_SENSORDRIVER) += sensor_imx477.o
sensor_imx477-y := src/drv_ss_cus_imx477_mipi.o

obj-$(CONFIG_MS_SENSORDRIVER) += sensor_ov2718.o
sensor_ov2718-y := src/drv_ss_cus_ov2718_mipi.o

obj-$(CONFIG_MS_SENSORDRIVER) += sensor_rn6852.o
sensor_rn6852-y := src/drv_ss_cus_rn6852_mipi.o

obj-$(CONFIG_MS_SENSORDRIVER) += sensor_rx_mipi.o
sensor_rx_mipi-y := src/drv_ss_cus_rx_mipi.o

obj-$(CONFIG_MS_SENSORDRIVER) += sensor_imx307_lvds.o
sensor_imx307_lvds-y := src/drv_ss_cus_imx307_lvds.o

obj-$(CONFIG_MS_SENSORDRIVER) += sensor_imx305_lvds.o
sensor_imx305_lvds-y := src/drv_ss_cus_imx305_lvds.o

obj-$(CONFIG_MS_SENSORDRIVER) += sensor_imx485.o
sensor_imx485-y := src/drv_ss_cus_imx485_mipi.o

obj-$(CONFIG_MS_SENSORDRIVER) += sensor_imx485_dol.o
sensor_imx485_dol-y := src/drv_ss_cus_imx485_dol_mipi.o

obj-$(CONFIG_MS_SENSORDRIVER) += sensor_os02n10_mipi.o
sensor_os02n10_mipi-y := src/drv_ss_os02n10_mipi.o

obj-$(CONFIG_MS_SENSORDRIVER) += sensor_os02n10_mipi.o
sensor_os02n10_mipi-y := src/drv_ss_os02n10_mipi.o
obj-$(CONFIG_MS_SENSORDRIVER) += sensor_sc2336_mipi.o
sensor_sc2336_mipi-y := src/drv_ss_sc2336_mipi.o

obj-$(CONFIG_MS_SENSORDRIVER) += sensor_sc431hai_hdr.o
sensor_sc431hai_hdr-y := src/drv_ss_sc431hai_mipi.o
obj-$(CONFIG_MS_SENSORDRIVER) += sensor_sc2336p_mipi.o
sensor_sc2336p_mipi-y := src/drv_ss_sc2336p_mipi.o
#ifeq ($(CONFIG_SSTAR_CHIP_NAME), "infinity7")
#obj-$(CONFIG_MS_SENSORDRIVER) += dh9931.o
#dh9931-objs := 3rd/dh9931/drv_ss_cus_dh9931.o 3rd/dh9931/dh9931_sdk_glibc64_10_2_1.a
#endif

ifeq ($(CONFIG_SSTAR_CHIP_NAME), "mercury6p")
obj-$(CONFIG_MS_SENSORDRIVER) += dh9931.o
dh9931-objs := 3rd/dh9931/drv_ss_cus_dh9931.o 3rd/dh9931/dh9931_sdk_glibc64_11.1.0.a
#dh9931-objs := 3rd/dh9931/drv_ss_cus_dh9931_1120.o 3rd/dh9931/dh9931_sdk_glibc64_11.1.0.a
endif
