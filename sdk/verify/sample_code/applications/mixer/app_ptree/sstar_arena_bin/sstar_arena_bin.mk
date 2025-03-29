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
EXEFILE := sstar_arena_bin
include $(LIBRARY_PATH)/ptree_ext/pipeline_tree_build_config.mk
INC += $(LIBRARY_PATH)/ptree
INC += $(LIBRARY_PATH)/ptree/core
INC += $(LIBRARY_PATH)/ptree/message
INC += $(LIBRARY_PATH)/ptree/packet
INC += $(LIBRARY_PATH)/ptree/linker
INC += $(LIBRARY_PATH)/ptree_ext/db
INC += $(LIBRARY_PATH)/ptree_ext/surface
INC += $(BUILD_TOP)/../common/ssos/platform
INC += $(BUILD_TOP)/../common/ssos/list
ifeq ($(CC), gcc)
INC += $(BUILD_TOP)/../common/ssos/platform/posix
else
INC += $(BUILD_TOP)/../common/ssos/platform/camos
endif

CLANG_FORMAT_ENABLE=yes
CLANG_TIDY_ENABLE=yes
