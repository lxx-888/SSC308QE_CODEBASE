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


ifeq ($(call FIND_COMPILER_OPTION, CONFIG_APPLICATION_DUALOS_SAMPLE), TRUE)
MODULE_PATH := $(BUILD_TOP)/source/internal/dualos_sample_manager/rtos

SUBDIRS := $(MODULE_PATH)/
INC     := $(PATH_rpmsg-lite)/pub\
           $(PATH_rpmsg-lite)/lib/include/platform/sstar\
           $(PATH_rpmsg-lite)/lib/include\
           $(PATH_rpmsg-lite)/inc\
           $(PATH_rpmsg-lite)/pub
endif
