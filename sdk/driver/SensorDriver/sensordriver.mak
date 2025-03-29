# SigmaStar trade secret
# Copyright (c) [2019~2020] SigmaStar Technology.
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

# Makefile for SigmaStar camdriver.

#-------------------------------------------------------------------------------
#   Description of some variables owned by the library
#-------------------------------------------------------------------------------
# Library module (lib) or Binary module (bin)
PROCESS = lib

PATH_C +=\
    $(PATH_sensordriver)/drv/src\
    $(PATH_sensordriver)/rtk

PATH_H +=\
    $(PATH_cam_os_wrapper)/pub\
    $(PATH_sensordriver)/drv/inc\
    $(PATH_sensordriver)/drv/pub\
    $(PATH_camdriver)/sensorif/drv/pub
#-------------------------------------------------------------------------------
#   List of source files of the library or executable to generate
#-------------------------------------------------------------------------------
ifeq ($(SENSOR_SUPPORT_IR),y)
PP_OPT_COMMON+= CONFIG_SENSOR_SUPPORT_IR
endif
ifneq ($(ABI_VER),)
SRC_C_LIST += abi_headers_sensordriver.c
endif

SRC_C_LIST += rtk_sensor_module_init.c

SRC_C_LIST += $(SENSOR_SRC)
#-------------------------------------------------------------------------------
#   Add VCM source file
#-------------------------------------------------------------------------------
SRC_C_LIST += rtk_vcm_module_init.c

ifneq (,$(filter _VCM_DW9714_,$(VCM_SET)))
  SRC_C_LIST += drv_ss_dw9714_vcm.c
endif
ifneq (,$(filter _VCM_LC898249_,$(VCM_SET)))
  SRC_C_LIST += drv_ss_lc898249_vcm.c
endif
