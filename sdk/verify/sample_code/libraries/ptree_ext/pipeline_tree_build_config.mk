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

PTREE_EXTERNAL_MODULES := empty file tick sync stdio pass
CODEDEFINE += $(foreach n,$(PTREE_EXTERNAL_MODULES),-DPTREE_MOD_$(shell tr 'a-z' 'A-Z' <<< $(n)))

ifeq ($(call FIND_COMPILER_OPTION, CONFIG_PIPELINE_TREE_USING_BINARY), TRUE)
USING_BINARY  := 1
else
ifeq ($(call FIND_COMPILER_OPTION, CONFIG_PIPELINE_TREE_USING_JSON), TRUE)
USING_JSON    := 1
else
ifeq ($(call FIND_COMPILER_OPTION, CONFIG_PIPELINE_TREE_USING_INI), TRUE)
USING_INI     := 1
endif
endif
endif
USING_BINARY ?= 0
ifeq ($(USING_BINARY), 1)
CODEDEFINE += -DUSING_BINARY
else
USING_JSON := 1
USING_INI := 0
ifeq ($(USING_JSON), 1)
CODEDEFINE += -DUSING_JSON
endif
ifeq ($(USING_INI), 1)
CODEDEFINE += -DUSING_INI
endif
endif
