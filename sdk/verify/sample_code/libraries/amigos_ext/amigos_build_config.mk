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
CONFIG_DB_JSON ?= 1
CONFIG_DB_INI ?= 0
CONFIG_MOD_CMD ?= 1
CONFIG_ONVIF ?= 1
ifeq ($(CONFIG_DB_JSON), 1)
CODEDEFINE += -DCONFIG_DB_JSON
endif
ifeq ($(CONFIG_DB_INI), 1)
CODEDEFINE += -DCONFIG_DB_INI
endif
ifeq ($(CONFIG_MOD_CMD), 1)
CONFIG_DB_UT ?= 1
CODEDEFINE += -DCONFIG_MOD_CMD
ifeq ($(CONFIG_DB_UT), 1)
CODEDEFINE += -DCONFIG_DB_UT
endif
endif
ifeq ($(CONFIG_ONVIF), 1)
CODEDEFINE += -DCONFIG_ONVIF
endif
GCCFLAGS += -Werror
