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
ifeq ($(CONFIG_DB_INI), 1)
INC += $(BUILD_TOP)/../common/iniparser
SRC += $(MODULE_PATH)/database/amigos_database_ini.cpp
endif
ifeq ($(CONFIG_DB_JSON), 1)
INC += $(BUILD_TOP)/../common/nlohmann
SRC += $(MODULE_PATH)/database/amigos_database_json.cpp
endif
ifeq ($(CONFIG_DB_UT), 1)
INC += $(BUILD_TOP)/../common/ss_cmd_base
SRC += $(MODULE_PATH)/database/amigos_database_ut.cpp
endif
SRC += $(MODULE_PATH)/database/amigos_database_factory.cpp
