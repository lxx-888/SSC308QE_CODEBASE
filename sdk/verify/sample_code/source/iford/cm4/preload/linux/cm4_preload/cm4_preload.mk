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
INC += $(BUILD_TOP)/source/internal/mailbox
INC += $(BUILD_TOP)/source/internal/venc
INC += $(BUILD_TOP)/source/internal/disp
INC += $(BUILD_TOP)/source/internal/rtsp_video
INC += $(BUILD_TOP)/../common/ss_rtsp

SUBDIRS += ./
LIBS += -lmi_common -lmi_scl -lss_mbx -lmi_isp -lmi_venc -lcus3a -lispalgo -ldl
LIBS += -lstdc++

ifeq ($(CM4_DEMO), y)
APP_REL_PREFIX:= bin
endif
