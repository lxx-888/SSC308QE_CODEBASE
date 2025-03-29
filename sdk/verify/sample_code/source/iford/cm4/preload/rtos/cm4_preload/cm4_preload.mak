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

ifeq ($(call FIND_COMPILER_OPTION, CONFIG_APPLICATION_CM4), TRUE)
MODULE_PATH := $(BUILD_TOP)/source/$(CHIP)/cm4/preload/rtos/cm4_preload
SUBDIRS     := $(MODULE_PATH)/src
INC         := $(PATH_mi_impl)/sensor/mi/inc
endif
