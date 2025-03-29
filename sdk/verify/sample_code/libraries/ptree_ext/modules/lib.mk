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
ifeq ($(interface_sys), enable)
SRC += $(MODULE_PATH)/modules/ptree_mod_sys.c
SRC += $(MODULE_PATH)/modules/ptree_mod_pool.c
SRC += $(MODULE_PATH)/modules/ptree_dma_packet.c
SRC += $(MODULE_PATH)/modules/ptree_mma_packet.c
endif
ifeq ($(interface_sensor), enable)
SRC += $(MODULE_PATH)/modules/ptree_mod_snr.c
endif
ifeq ($(interface_vif), enable)
SRC += $(MODULE_PATH)/modules/ptree_mod_vif.c
endif
ifeq ($(interface_isp), enable)
SRC += $(MODULE_PATH)/modules/ptree_mod_isp.c
SRC += $(MODULE_PATH)/modules/ptree_mod_aestable.c
endif
ifeq ($(interface_cus3a), enable)
SRC += $(MODULE_PATH)/modules/ptree_mod_iq.c
endif
ifeq ($(interface_scl), enable)
SRC += $(MODULE_PATH)/modules/ptree_mod_scl.c
SRC += $(MODULE_PATH)/modules/ptree_mod_scl_base.c
SRC += $(MODULE_PATH)/modules/ptree_mod_scl_stretch.c
endif
ifeq ($(interface_venc), enable)
SRC += $(MODULE_PATH)/modules/ptree_mod_venc.c
endif
ifeq ($(interface_ldc), enable)
SRC += $(MODULE_PATH)/modules/ptree_mod_ldc.c
endif
ifeq ($(interface_disp), enable)
SRC += $(MODULE_PATH)/modules/ptree_mod_disp.c
endif
ifeq ($(interface_rgn), enable)
SRC += $(MODULE_PATH)/modules/ptree_mod_rgn.c
endif
ifeq ($(interface_vdf), enable)
SRC += $(MODULE_PATH)/modules/ptree_mod_vdf.c
endif
ifeq ($(interface_ai), enable)
SRC += $(MODULE_PATH)/modules/ptree_mod_ai.c
endif
ifeq ($(interface_ao), enable)
SRC += $(MODULE_PATH)/modules/ptree_mod_ao.c
endif
ifeq ($(interface_ipu), enable)
SRC += $(MODULE_PATH)/modules/ptree_mod_det.c
#ifeq ($(interface_shadow), enable)
#SRC += $(MODULE_PATH)/modules/ptree_mod_hseg.c
#endif
endif
ifneq ($(filter empty, $(PTREE_EXTERNAL_MODULES)),)
SRC += $(MODULE_PATH)/modules/ptree_mod_empty.c
endif
ifneq ($(filter file, $(PTREE_EXTERNAL_MODULES)),)
SRC += $(MODULE_PATH)/modules/ptree_mod_file.c
endif
ifneq ($(filter tick, $(PTREE_EXTERNAL_MODULES)),)
SRC += $(MODULE_PATH)/modules/ptree_mod_tick.c
endif
ifneq ($(filter sync, $(PTREE_EXTERNAL_MODULES)),)
SRC += $(MODULE_PATH)/modules/ptree_mod_sync.c
endif
ifneq ($(filter stdio, $(PTREE_EXTERNAL_MODULES)),)
SRC += $(MODULE_PATH)/modules/ptree_mod_stdio.c
endif
ifneq ($(filter pass, $(PTREE_EXTERNAL_MODULES)),)
SRC += $(MODULE_PATH)/modules/ptree_mod_pass.c
endif
ifeq ($(call FIND_COMPILER_OPTION, CONFIG_USB_GADGET_UVC_SUPPORT), TRUE)
SRC += $(MODULE_PATH)/modules/ptree_mod_uvc.c
endif
SRC += $(MODULE_PATH)/modules/ptree_rgn_packet.c
