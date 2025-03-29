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

ifeq ($(call FIND_COMPILER_OPTION, CONFIG_APPLICATION_PTREE), TRUE)
MODULE_PATH := $(BUILD_TOP)/applications/mixer/app_ptree/preload_rtos
SUBDIRS     := $(MODULE_PATH)

INC += $(LIBRARY_PATH)/ptree
INC += $(LIBRARY_PATH)/ptree/core
INC += $(BUILD_TOP)/../common/ssos/platform
endif
