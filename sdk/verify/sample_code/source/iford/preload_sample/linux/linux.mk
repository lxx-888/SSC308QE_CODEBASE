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

EXEFILE := $(patsubst prog_source_$(CHIP)_%,prog_%,$(EXEFILE))

INC += $(BUILD_TOP)/source/internal/common
INC += $(BUILD_TOP)/source/internal/vif
INC += $(BUILD_TOP)/source/internal/isp
INC += $(BUILD_TOP)/source/internal/scl
INC += $(BUILD_TOP)/source/internal/venc
INC += $(BUILD_TOP)/source/internal/dla
INC += $(BUILD_TOP)/source/internal/rgn
INC += $(BUILD_TOP)/source/internal/aov
INC += $(BUILD_TOP)/source/internal/audio
INC += $(BUILD_TOP)/../common/list
INC += $(BUILD_TOP)/../common/ss_font
INC += $(BUILD_TOP)/source/internal/aov
INC += $(BUILD_TOP)/source/internal/light_misc_control

LIBS += -lmi_venc -lmi_ai -lmi_ao -lmi_iqserver -lmi_isp -lmi_vif -lmi_scl
LIBS += -lstdc++
