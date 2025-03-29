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

ifneq ($(filter file, $(PTREE_EXTERNAL_MODULES)),)
SRC += $(MODULE_PATH)/api/ptree_api_file.c
endif
ifeq ($(interface_vif), enable)
SRC += $(MODULE_PATH)/api/ptree_api_vif.c
endif
ifeq ($(interface_isp), enable)
SRC += $(MODULE_PATH)/api/ptree_api_isp.c
SRC += $(MODULE_PATH)/api/ptree_api_aestable.c
endif
ifeq ($(interface_sys), enable)
SRC += $(MODULE_PATH)/api/ptree_api_sys.c
endif
ifeq ($(interface_sensor), enable)
SRC += $(MODULE_PATH)/api/ptree_api_snr.c
endif
