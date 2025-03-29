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

#
# Makefile for SigmaStar camdriver.

#-------------------------------------------------------------------------------
#   Description of some variables owned by the library
#-------------------------------------------------------------------------------
# Library module (lib) or Binary module (bin)
PROCESS = lib
#PATH_sensor_earlyinit_hal = $(PATH_sensor_earlyinit)/hal/$(BB_CHIP_ID)

PATH_C +=\
    $(PATH_sensordriver)/drv/src\
    $(PATH_sensordriver)/earlyinit/drv/src

PATH_H +=\
    $(PATH_cam_os_wrapper)/pub\
    $(PATH_sensordriver)/drv/inc\
    $(PATH_sensordriver)/earlyinit/drv/pub\
    $(PATH_camdriver)/earlyinit/drv/pub\
    $(PATH_camdriver)/earlyinit/drv/inc\
    $(PATH_camdriver)/sensorif/drv/pub\
    $(PATH_camdriver)/common\
    $(PATH_camdriver)/cmdq_service/drv/pub\
    $(PATH_earlyinit_main)/entry/pub \
    $(PATH_earlyinit_impl)/common/i2c/pub\

#-------------------------------------------------------------------------------
#   List of source files of the library or executable to generate
#-------------------------------------------------------------------------------

ifeq ($(call FIND_COMPILER_OPTION, _SENSOR_EARLYINIT_), TRUE)
  SRC_C_LIST += abi_headers_sensor_earlyinit.c
endif

ifeq ($(call FIND_COMPILER_OPTION, _SENSOR_EARLYINIT_), FALSE)
  SRC_C_LIST += drv_earlyinit_dummy_sensor.c
else

ifneq (,$(filter _SENSOR_PS5520_,$(SENSOR_SET)))
  SRC_C_LIST += drv_earlyinit_ps5520.c
endif

ifneq (,$(filter _SENSOR_IMX323_,$(SENSOR_SET)))
  SRC_C_LIST += drv_earlyinit_imx323.c
endif

ifneq (,$(filter _SENSOR_PS5250_,$(SENSOR_SET)))
  SRC_C_LIST += drv_earlyinit_ps5250.c
endif

ifneq (,$(filter _SENSOR_PS5250_,$(SENSOR_SET)))
  SRC_C_LIST += drv_earlyinit_ps5250.c
endif

ifneq (,$(filter _SENSOR_PS5270_,$(SENSOR_SET)))
  SRC_C_LIST += drv_earlyinit_ps5270.c
endif

ifneq (,$(filter _SENSOR_PS5268_,$(SENSOR_SET)))
  SRC_C_LIST += drv_earlyinit_ps5268.c
endif

ifneq (,$(filter _SENSOR_IMX307_,$(SENSOR_SET)))
  SRC_C_LIST += drv_earlyinit_imx307.c
endif

ifneq (,$(filter _SENSOR_SC4238_,$(SENSOR_SET)))
  SRC_C_LIST += drv_earlyinit_sc4238.c
endif

ifneq (,$(filter _SENSOR_IMX415_,$(SENSOR_SET)))
  SRC_C_LIST += drv_earlyinit_imx415.c
endif

ifneq (,$(filter _SENSOR_F32_,$(SENSOR_SET)))
  SRC_C_LIST += drv_earlyinit_f32.c
endif

ifneq (,$(filter _SENSOR_PS5258_,$(SENSOR_SET)))
  SRC_C_LIST += drv_earlyinit_ps5258.c
endif

ifneq (,$(filter _SENSOR_PS5260_,$(SENSOR_SET)))
  SRC_C_LIST += drv_earlyinit_ps5260.c
endif

ifneq (,$(filter _SENSOR_SC210IOT_,$(SENSOR_SET)))
  SRC_C_LIST += drv_earlyinit_sc210iot.c
endif

ifneq (,$(filter _SENSOR_H66_,$(SENSOR_SET)))
  SRC_C_LIST += drv_earlyinit_h66.c
endif

ifneq (,$(filter _SENSOR_GC2053_,$(SENSOR_SET)))
  SRC_C_LIST += drv_earlyinit_gc2053.c
endif

ifneq (,$(filter _SENSOR_GC4653_,$(SENSOR_SET)))
  SRC_C_LIST += drv_earlyinit_gc4653.c
endif

ifneq (,$(filter _SENSOR_SC4336_,$(SENSOR_SET)))
  SRC_C_LIST += drv_earlyinit_sc4336.c
endif

ifneq (,$(filter _SENSOR_K351P_,$(SENSOR_SET)))
  SRC_C_LIST += drv_earlyinit_K351P.c
endif

ifneq (,$(filter _SENSOR_SC4336P_,$(SENSOR_SET)))
  SRC_C_LIST += drv_earlyinit_sc4336p.c
endif

ifneq (,$(filter _SENSOR_SC4336PDUALSNRAOV_,$(SENSOR_SET)))
  SRC_C_LIST += drv_earlyinit_sc4336p_dualsnr_aov.c
endif

ifneq (,$(filter _SENSOR_PS5416_,$(SENSOR_SET)))
  SRC_C_LIST += drv_earlyinit_ps5416.c
endif

ifneq (,$(filter _SENSOR_OS04D10_,$(SENSOR_SET)))
  SRC_C_LIST += drv_earlyinit_os04d10.c
endif

ifneq (,$(filter _SENSOR_SC830AI_,$(SENSOR_SET)))
  SRC_C_LIST += drv_earlyinit_sc830ai.c
endif

ifneq (,$(filter _SENSOR_IMX664_,$(SENSOR_SET)))
  SRC_C_LIST += drv_earlyinit_imx664.c
endif

ifneq (,$(filter _SENSOR_OS02N10_,$(SENSOR_SET)))
  SRC_C_LIST += drv_earlyinit_os02n10.c
endif

ifneq (,$(filter _SENSOR_IMX678_,$(SENSOR_SET)))
  SRC_C_LIST += drv_earlyinit_imx678.c
endif

ifneq (,$(filter _SENSOR_SC2336_,$(SENSOR_SET)))
  SRC_C_LIST += drv_earlyinit_sc2336.c
endif

ifneq (,$(filter _SENSOR_SC431HAI_,$(SENSOR_SET)))
  SRC_C_LIST += drv_earlyinit_sc431hai.c
endif

ifneq (,$(filter _SENSOR_SC2336P_,$(SENSOR_SET)))
  SRC_C_LIST += drv_earlyinit_sc2336p.c
endif

ifneq (,$(filter _SENSOR_SC535HAI_,$(SENSOR_SET)))
  SRC_C_LIST += drv_earlyinit_sc535hai.c
endif

ifneq (,$(filter _SENSOR_SC2356_,$(SENSOR_SET)))
  SRC_C_LIST += drv_earlyinit_sc2356.c
endif

endif
