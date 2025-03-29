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
EXEFILE := $(patsubst prog_source_$(CHIP)_%,prog_%,$(EXEFILE))

INC += $(BUILD_TOP)/source/internal/common
SUBDIRS += ./

LIBS += -lmi_common -lAEC_LINUX

MODULE_REL_FILES +=  $(BUILD_TOP)/source/$(CHIP)/audio/alg/aec_demo/resource
