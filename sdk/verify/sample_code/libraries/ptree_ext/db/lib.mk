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

ifneq ($(USING_BINARY), 1)
ifeq ($(USING_JSON), 1)
ifeq ($(PTREE_OS_RTK), )
SRC += $(MODULE_PATH)/cjson/cjson.c
INC += $(MODULE_PATH)/cjson
endif
SRC += $(MODULE_PATH)/db/ptree_db_json.c
endif
ifeq ($(USING_INI), 1)
INC += $(MODULE_PATH)/iniparser
SRC += $(MODULE_PATH)/iniparser/iniparser.c
SRC += $(MODULE_PATH)/iniparser/dictionary.c
SRC += $(MODULE_PATH)/db/ptree_db_ini.c
endif
endif
