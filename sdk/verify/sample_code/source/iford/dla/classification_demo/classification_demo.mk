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
INC += $(BUILD_TOP)/source/internal/dla
INC += $(BUILD_TOP)/source/internal/common
INC += $(BUILD_TOP)/source/internal/vif
INC += $(BUILD_TOP)/source/internal/isp
INC += $(BUILD_TOP)/source/internal/scl
INC += $(BUILD_TOP)/source/internal/venc
INC += $(BUILD_TOP)/source/internal/rgn
INC += $(BUILD_TOP)/source/internal/rtsp_video
INC += $(BUILD_TOP)/../common/ss_rtsp
INC += $(BUILD_TOP)/../common/list
INC += $(BUILD_TOP)/../common/ss_font
INC  += ./

SUBDIRS += ./

LIBS += -lmi_common

MODULE_REL_FILES +=  $(BUILD_TOP)/source/$(CHIP)/dla/facerecognition_demo/resource

LIBS += -lsstaralgo_cls -lmi_ipu
LIBS += -lmi_sensor
LIBS += -lmi_vif
LIBS += -lmi_isp
LIBS += -lmi_scl
LIBS += -lmi_venc
LIBS += -lmi_rgn
LIBS += -lcus3a
LIBS += -lispalgo
LIBS += -lstdc++
