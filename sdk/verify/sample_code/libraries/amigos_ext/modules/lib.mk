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
CODEDEFINE += -D_FILE_OFFSET_BITS=64
SRC += $(MODULE_PATH)/modules/amigos_module_rtsp.cpp
SRC += $(MODULE_PATH)/modules/amigos_module_file.cpp
SRC += $(MODULE_PATH)/modules/amigos_module_timer.cpp
SRC += $(MODULE_PATH)/modules/amigos_module_env_monitor.cpp
SRC += $(MODULE_PATH)/modules/amigos_module_sync.cpp
SRC += $(MODULE_PATH)/modules/amigos_module_switch.cpp
SRC += $(MODULE_PATH)/modules/amigos_module_tick.cpp
SRC += $(MODULE_PATH)/modules/amigos_module_pass.cpp
SRC += $(MODULE_PATH)/modules/amigos_module_empty.cpp
SRC += $(MODULE_PATH)/modules/amigos_module_exp.cpp
INC += $(BUILD_TOP)/../common/ss_exp
ifeq ($(interface_sys), enable)
SRC += $(MODULE_PATH)/modules/amigos_module_mi_base.cpp
SRC += $(MODULE_PATH)/modules/amigos_module_osd.cpp
SRC += $(MODULE_PATH)/modules/amigos_module_uac.cpp
SRC += $(MODULE_PATH)/modules/amigos_module_uvc.cpp
SRC += $(MODULE_PATH)/modules/amigos_module_slot.cpp
SRC += $(MODULE_PATH)/modules/amigos_module_pool.cpp
SRC += $(MODULE_PATH)/modules/amigos_module_pares.cpp
endif
ifeq ($(interface_ai), enable)
SRC += $(MODULE_PATH)/modules/amigos_module_ai.cpp
SRC += $(MODULE_PATH)/modules/amigos_module_aec.cpp
endif
ifeq ($(interface_ao), enable)
SRC += $(MODULE_PATH)/modules/amigos_module_ao.cpp
endif
ifeq ($(sort $(interface_ai) $(interface_ao)), enable)
INC += ${BUILD_TOP}/../common/G711
INC += ${BUILD_TOP}/../common/G726
SRC += $(MODULE_PATH)/modules/amigos_module_audioalgo.cpp
SRC += $(MODULE_PATH)/modules/amigos_module_audiodecode.cpp
SRC += $(MODULE_PATH)/modules/amigos_module_audioencode.cpp
endif
ifeq ($(interface_vif), enable)
SRC += $(MODULE_PATH)/modules/amigos_module_vif.cpp
endif
ifeq ($(interface_isp), enable)
SRC += $(MODULE_PATH)/modules/amigos_module_isp.cpp
SRC += $(MODULE_PATH)/modules/amigos_module_gae.cpp
SRC += $(MODULE_PATH)/modules/amigos_module_sae.cpp
INC += $(PROJ_ROOT)/release/include/isp
endif
ifeq ($(interface_ldc), enable)
SRC += $(MODULE_PATH)/modules/amigos_module_ldc.cpp
endif
ifeq ($(interface_nir), enable)
SRC += $(MODULE_PATH)/modules/amigos_module_nir.cpp
endif
#ifeq ($(interface_cus3a), enable)
#endif
ifeq ($(interface_venc), enable)
SRC += $(MODULE_PATH)/modules/amigos_module_venc.cpp
endif
ifeq ($(interface_vdec), enable)
SRC += $(MODULE_PATH)/modules/amigos_module_vdec.cpp
endif
ifeq ($(interface_scl), enable)
SRC += $(MODULE_PATH)/modules/amigos_module_scl_base.cpp
SRC += $(MODULE_PATH)/modules/amigos_module_scl.cpp
SRC += $(MODULE_PATH)/modules/amigos_module_scl_stretch.cpp
endif
ifeq ($(interface_disp), enable)
SRC += $(MODULE_PATH)/modules/amigos_module_disp.cpp
SRC += $(MODULE_PATH)/modules/amigos_module_wbc.cpp
endif
ifeq ($(interface_iqserver), enable)
SRC += $(MODULE_PATH)/modules/amigos_module_iq.cpp
endif
ifeq ($(interface_sensor), enable)
SRC += $(MODULE_PATH)/modules/amigos_module_snr.cpp
endif
ifeq ($(interface_vdisp), enable)
SRC += $(MODULE_PATH)/modules/amigos_module_vdisp.cpp
endif
ifeq ($(interface_pcie), enable)
ifeq ($(CONFIG_MOD_CMD), 1)
INC += $(BUILD_TOP)/../common/ss_cmd_base
SRC += $(MODULE_PATH)/modules/amigos_module_pcie.cpp
endif
endif
ifneq ($(filter $(CUSTOMER_ENABLED), snr9931), )
INC += $(BUILD_TOP)/../common/sensor/dh9931
SRC += $(MODULE_PATH)/modules/amigos_module_snr9931.cpp
endif
ifeq ($(interface_jpd), enable)
SRC += $(MODULE_PATH)/modules/amigos_module_jpd.cpp
endif
ifeq ($(interface_vdf), enable)
SRC += $(MODULE_PATH)/modules/amigos_module_vdf.cpp
endif
ifeq ($(interface_ive), enable)
SRC += $(MODULE_PATH)/modules/amigos_module_ive.cpp
endif
ifeq ($(interface_rgn), enable)
INC += $(BUILD_TOP)/../common/ss_graph
INC += $(BUILD_TOP)/../common/stb_truetype
SRC += $(MODULE_PATH)/modules/amigos_module_rgn.cpp
endif
ifeq ($(interface_ipu), enable)
SRC += $(MODULE_PATH)/modules/amigos_module_det.cpp
endif
ifeq ($(GLES_LDC_ENABLE),y)
#INC += $(PROJ_ROOT)/sdk/osdk/gpu/opengles/ldc
#SRC += $(MODULE_PATH)/amigos_module_gpu.cpp
#CODEDEFINE += -DGLES_LDC_ENABLE
#CODEDEFINE += -DAMIGOS_DMABUF_ENABLE
endif
ifeq ($(OSDK_DRM),y)
#SRC += $(MODULE_PATH)/amigos_module_drm.cpp
#INC += $(BUILD_TOP)/../prebuild_libs/drm/include
#CODEDEFINE += -DDRM_ENABLE
#CODEDEFINE += -DAMIGOS_DMABUF_ENABLE
endif
ifeq ($(interface_hdmirx), enable)
SRC += $(MODULE_PATH)/modules/amigos_module_hdmirx.cpp
endif
ifeq ($(interface_hvp), enable)
SRC += $(MODULE_PATH)/modules/amigos_module_hvp.cpp
SRC += $(MODULE_PATH)/modules/amigos_module_signal_monitor.cpp
endif
ifeq ($(interface_shadow), enable)
ifeq (${interface_ipu}, enable)
SRC += $(MODULE_PATH)/modules/amigos_module_hseg.cpp
endif
endif
