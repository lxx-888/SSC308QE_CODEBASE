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

ifneq ($(filter empty, $(PTREE_EXTERNAL_MODULES)),)
SRC += $(MODULE_PATH)/commands/ptree_cmd_empty.c
endif
ifneq ($(filter file, $(PTREE_EXTERNAL_MODULES)),)
SRC += $(MODULE_PATH)/commands/ptree_cmd_file.c
endif
ifneq ($(filter tick, $(PTREE_EXTERNAL_MODULES)),)
SRC += $(MODULE_PATH)/commands/ptree_cmd_tick.c
endif
ifeq ($(interface_vif), enable)
SRC += $(MODULE_PATH)/commands/ptree_cmd_vif.c
endif
ifeq ($(interface_isp), enable)
SRC += $(MODULE_PATH)/commands/ptree_cmd_isp.c
endif
ifeq ($(interface_scl), enable)
SRC += $(MODULE_PATH)/commands/ptree_cmd_scl.c
endif
ifeq ($(interface_sensor), enable)
SRC += $(MODULE_PATH)/commands/ptree_cmd_snr.c
endif
ifeq ($(interface_ai), enable)
SRC += $(MODULE_PATH)/commands/ptree_cmd_ai.c
endif
ifeq ($(interface_ao), enable)
SRC += $(MODULE_PATH)/commands/ptree_cmd_ao.c
endif
ifeq ($(interface_ldc), enable)
SRC += $(MODULE_PATH)/commands/ptree_cmd_ldc.c
endif
ifeq ($(interface_venc), enable)
SRC += $(MODULE_PATH)/commands/ptree_cmd_venc.c
endif
ifeq ($(interface_vdf), enable)
SRC += $(MODULE_PATH)/commands/ptree_cmd_vdf.c
endif
ifeq ($(interface_ipu), enable)
#ifeq ($(interface_shadow), enable)
#SRC += $(MODULE_PATH)/commands/ptree_cmd_hseg.c
#endif
endif
ifneq ($(filter stdio, $(PTREE_EXTERNAL_MODULES)),)
SRC += $(MODULE_PATH)/commands/ptree_cmd_stdio.c
endif
ifneq ($(filter pass, $(PTREE_EXTERNAL_MODULES)),)
SRC += $(MODULE_PATH)/commands/ptree_cmd_pass.c
endif
ifeq ($(call FIND_COMPILER_OPTION, CONFIG_USB_GADGET_UVC_SUPPORT), TRUE)
SRC += $(MODULE_PATH)/commands/ptree_cmd_uvc.c
endif
