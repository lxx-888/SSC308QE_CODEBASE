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

include $(MODULE_PATH)/pipeline_tree_build_config.mk
include $(MODULE_PATH)/common.mk

GCCFLAGS += -Werror

AUTO_GEN_BIN_FILE := $(MODULE_PATH)/resource/auto_gen_ptree_bin.h

CLANG_FORMAT_ENABLE:=yes
CLANG_TIDY_ENABLE:=yes
CLANG_FORMAT_FILTER += $(wildcard $(MODULE_PATH)/cjson/*)
CLANG_TIDY_FILTER   += $(wildcard $(MODULE_PATH)/cjson/*)
CLANG_FORMAT_FILTER += $(wildcard $(MODULE_PATH)/iniparser/*)
CLANG_TIDY_FILTER   += $(wildcard $(MODULE_PATH)/iniparser/*)
CLANG_TIDY_FILTER   += $(MODULE_PATH)/ptree_kernel.h
CLANG_FORMAT_FILTER += $(AUTO_GEN_BIN_FILE)
CLANG_TIDY_FILTER   += $(AUTO_GEN_BIN_FILE)
