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

LINK_TYPE := static
INTER_LINK_TYPE := static

EXEFILE  := test_ssos_task
GCCFLAGS += -Werror

INC += $(BUILD_TOP)/../common/ssos/task
INC += $(BUILD_TOP)/../common/ssos/list
INC += $(BUILD_TOP)/../common/ssos/platform
ifeq ($(CC), gcc)
INC += $(BUILD_TOP)/../common/ssos/platform/posix
else
INC += $(BUILD_TOP)/../common/ssos/platform/camos
endif

CLANG_FORMAT_ENABLE=yes
CLANG_TIDY_ENABLE=yes
