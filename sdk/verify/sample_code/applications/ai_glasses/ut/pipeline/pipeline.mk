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
INC += $(BUILD_TOP)/source/internal/isp
INC += $(BUILD_TOP)/source/internal/scl
INC += $(BUILD_TOP)/source/internal/ldc
INC += $(BUILD_TOP)/source/internal/venc
INC += $(BUILD_TOP)/source/internal/ai_glasses
INC += $(LIBRARY_PATH)/ptree
INC += $(LIBRARY_PATH)/ptree_ext/api
INC += $(BUILD_TOP)/../common/ssos/platform
ifeq ($(CC), gcc)
INC += $(BUILD_TOP)/../common/ssos/platform/posix
else
INC += $(BUILD_TOP)/../common/ssos/platform/camos
endif

INC  += ./
SUBDIRS += ./
LIBS += -lptree
LIBS += -lmi_common
LIBS += -lmi_sensor
LIBS += -lmi_vif
LIBS += -lmi_isp
LIBS += -lmi_scl
LIBS += -lmi_ldc
LIBS += -lmi_venc
LIBS += -lcus3a
LIBS += -lispalgo

CLANG_FORMAT_ENABLE=yes
CLANG_TIDY_ENABLE=yes
