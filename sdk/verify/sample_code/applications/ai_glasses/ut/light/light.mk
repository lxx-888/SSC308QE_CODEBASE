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
EXEFILE := $(patsubst prog_applications_%,prog_%,$(EXEFILE))

INC += $(BUILD_TOP)/source/internal/common
INC += $(BUILD_TOP)/source/internal/vif


INC  += ./
SUBDIRS += ./
LIBS += -lssos
LIBS += -lmi_common
LIBS += -lmi_sensor
LIBS += -lmi_vif

CLANG_FORMAT_ENABLE=yes
CLANG_TIDY_ENABLE=yes
