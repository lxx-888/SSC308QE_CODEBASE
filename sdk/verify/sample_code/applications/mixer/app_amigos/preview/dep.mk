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
include $(LIBRARY_PATH)/amigos_ext/amigos_build_config.mk

INC += $(LIBRARY_PATH)/amigos_ext/modules
INC += $(LIBRARY_PATH)/amigos_ext/surface
INC += $(LIBRARY_PATH)/amigos_ext/database
INC += $(LIBRARY_PATH)/amigos_ext/commands
PREBUILD_LIBS += luajit
DEP += libraries/amigos_ext ../amigos/amigos_instance ../amigos/amigos_base
DEP += ../common/ss_thread ../common/ss_rtsp ../common/live555 ../common/ss_exp
ifeq ($(CONFIG_DB_INI), 1)
DEP += ../common/iniparser
endif
ifneq ($(ANDROID), y)
ifeq ($(CONFIG_ONVIF), 1)
DEP += ../common/onvif
endif
endif
ifeq ($(CONFIG_MOD_CMD), 1)
DEP += ../common/ss_cmd_base ../common/ss_console
DEP += ../common/ss_cmd_common
endif
DEP += ../common/ss_util
ifneq ($(filter $(CUSTOMER_ENABLED), snr9931), )
DEP += ../common/dh9931
endif
ifeq ($(interface_rgn), enable)
DEP += ../common/ss_graph ../common/stb_truetype
endif
ifeq ($(interface_sys), enable)
DEP += ../common/ss_uvc ../common/ss_uac
endif
ifeq ($(sort $(interface_ao) $(interface_ai)), enable)
DEP += ../common/G711 ../common/G726
endif

#3RD_PARTY_DEP += z png jpeg
