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

SUBDIRS := $(MODULE_PATH)/core
SUBDIRS += $(MODULE_PATH)/arena
SUBDIRS += $(MODULE_PATH)/obj
SUBDIRS += $(MODULE_PATH)/packet
SUBDIRS += $(MODULE_PATH)/message
SUBDIRS += $(MODULE_PATH)/linker
SUBDIRS += $(MODULE_PATH)/graph
ifeq ($(call FIND_COMPILER_OPTION, CONFIG_USB_GADGET_UVC_SUPPORT), TRUE)
SUBDIRS += $(MODULE_PATH)/uvc
endif
ifneq ($(USING_BINARY), 1)
SUBDIRS += $(MODULE_PATH)/surface
endif
include $(MODULE_PATH)/modules/lib.mk
include $(MODULE_PATH)/commands/lib.mk
include $(MODULE_PATH)/db/lib.mk
include $(MODULE_PATH)/api/lib.mk
INC += $(BUILD_TOP)/../common/ssos/platform
INC += $(BUILD_TOP)/../common/ssos/task
INC += $(BUILD_TOP)/../common/ssos/list
ifeq ($(CC), gcc)
INC += $(BUILD_TOP)/../common/ssos/platform/posix
else
INC += $(BUILD_TOP)/../common/ssos/platform/camos
endif
INC += $(MODULE_PATH)
INC += $(MODULE_PATH)/../ptree
INC += $(MODULE_PATH)/../ptree/core
INC += $(MODULE_PATH)/../ptree/packet
INC += $(MODULE_PATH)/../ptree/message
INC += $(MODULE_PATH)/../ptree/linker
INC += $(MODULE_PATH)/modules
INC += $(MODULE_PATH)/commands
INC += $(MODULE_PATH)/surface
INC += $(MODULE_PATH)/graph
INC += $(MODULE_PATH)/api
ifeq ($(call FIND_COMPILER_OPTION, CONFIG_USB_GADGET_UVC_SUPPORT), TRUE)
INC += $(MODULE_PATH)/uvc
endif
ifneq ($(USING_BINARY), 1)
INC += $(MODULE_PATH)/db
endif
SRC += $(MODULE_PATH)/ptree_preload.c
