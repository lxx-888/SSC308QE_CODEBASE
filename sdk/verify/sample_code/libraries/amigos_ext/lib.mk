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

SUBDIRS := $(MODULE_PATH)/surface $(MODULE_PATH)/

GCCFLAGS += -Werror

INC += $(BUILD_TOP)/../common/live555/BasicUsageEnvironment/include
INC += $(BUILD_TOP)/../common/live555/groupsock/include
INC += $(BUILD_TOP)/../common/live555/liveMedia/include
INC += $(BUILD_TOP)/../common/live555/UsageEnvironment/include
INC += $(BUILD_TOP)/../common/live555/mediaServer/include
INC += $(BUILD_TOP)/../common/onvif
INC += $(BUILD_TOP)/../common/iniparser
INC += $(BUILD_TOP)/../common/ss_thread
INC += $(BUILD_TOP)/../common/ss_rtsp
INC += $(BUILD_TOP)/../common/list
INC += $(BUILD_TOP)/../common/nlohmann
INC += $(BUILD_TOP)/../common/ss_util
INC += $(BUILD_TOP)/../common/ss_uvc
INC += $(BUILD_TOP)/../common/ss_uac
INC += $(BUILD_TOP)/../common/ss_cmd_base
INC += $(BUILD_TOP)/../common/ss_console

INC += $(MODULE_PATH)/
INC += $(MODULE_PATH)/database
INC += $(MODULE_PATH)/surface
INC += $(LIBRARY_PATH)/amigos/amigos_base
INC += $(LIBRARY_PATH)/amigos/amigos_instance

include $(MODULE_PATH)/amigos_build_config.mk
include $(MODULE_PATH)/database/lib.mk
include $(MODULE_PATH)/commands/lib.mk
include $(MODULE_PATH)/modules/lib.mk
