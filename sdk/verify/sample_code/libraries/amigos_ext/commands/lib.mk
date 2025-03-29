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
ifeq ($(CONFIG_MOD_CMD), 1)
INC += $(MODULE_PATH)/modules
SRC += $(MODULE_PATH)/commands/amigos_command.cpp
SRC += $(MODULE_PATH)/commands/amigos_command_base.cpp
SRC += $(MODULE_PATH)/commands/amigos_command_instance.cpp
SRC += $(MODULE_PATH)/commands/amigos_command_rtsp.cpp
SRC += $(MODULE_PATH)/commands/amigos_command_file.cpp
SRC += $(MODULE_PATH)/commands/amigos_command_sync.cpp
SRC += $(MODULE_PATH)/commands/amigos_command_switch.cpp
SRC += $(MODULE_PATH)/commands/amigos_command_tick.cpp
SRC += $(MODULE_PATH)/commands/amigos_command_pass.cpp
SRC += $(MODULE_PATH)/commands/amigos_command_empty.cpp
ifeq ($(interface_sys), enable)
SRC += $(MODULE_PATH)/commands/amigos_command_mi_base.cpp
SRC += $(MODULE_PATH)/commands/amigos_command_osd.cpp
SRC += $(MODULE_PATH)/commands/amigos_command_uac.cpp
SRC += $(MODULE_PATH)/commands/amigos_command_uvc.cpp
SRC += $(MODULE_PATH)/commands/amigos_command_slot.cpp
SRC += $(MODULE_PATH)/commands/amigos_command_pool.cpp
endif
ifeq ($(interface_ai), enable)
SRC += $(MODULE_PATH)/commands/amigos_command_ai.cpp
SRC += $(MODULE_PATH)/commands/amigos_command_aec.cpp
endif
ifeq ($(interface_ao), enable)
SRC += $(MODULE_PATH)/commands/amigos_command_ao.cpp
endif
ifeq ($(sort $(interface_ai) $(interface_ao)), enable)
SRC += $(MODULE_PATH)/commands/amigos_command_audioalgo.cpp
endif
ifeq ($(interface_vif), enable)
SRC += $(MODULE_PATH)/commands/amigos_command_vif.cpp
endif
ifeq ($(interface_isp), enable)
SRC += $(MODULE_PATH)/commands/amigos_command_isp.cpp
INC += $(PROJ_ROOT)/release/include/isp
endif
ifeq ($(interface_ldc), enable)
SRC += $(MODULE_PATH)/commands/amigos_command_ldc.cpp
endif
ifeq ($(interface_nir), enable)
SRC += $(MODULE_PATH)/commands/amigos_command_nir.cpp
endif
#ifeq ($(interface_cus3a), enable)
#endif
ifeq ($(interface_venc), enable)
SRC += $(MODULE_PATH)/commands/amigos_command_venc.cpp
endif
ifeq ($(interface_vdec), enable)
SRC += $(MODULE_PATH)/commands/amigos_command_vdec.cpp
endif
ifeq ($(interface_scl), enable)
SRC += $(MODULE_PATH)/commands/amigos_command_scl.cpp
SRC += $(MODULE_PATH)/commands/amigos_command_scl_stretch.cpp
endif
ifeq ($(interface_disp), enable)
SRC += $(MODULE_PATH)/commands/amigos_command_disp.cpp
SRC += $(MODULE_PATH)/commands/amigos_command_wbc.cpp
endif
ifeq ($(interface_iqserver), enable)
SRC += $(MODULE_PATH)/commands/amigos_command_iq.cpp
endif
ifeq ($(interface_sensor), enable)
SRC += $(MODULE_PATH)/commands/amigos_command_snr.cpp
endif
ifeq ($(interface_vdisp), enable)
SRC += $(MODULE_PATH)/commands/amigos_command_vdisp.cpp
endif
ifeq ($(interface_pcie), enable)
INC += $(BUILD_TOP)/../common/ss_cmd_basep
SRC += $(MODULE_PATH)/commands/amigos_command_pcie.cpp
endif
ifneq ($(filter $(CUSTOMER_ENABLED), snr9931), )
INC += $(BUILD_TOP)/../common/sensor/dh9931
SRC += $(MODULE_PATH)/commands/amigos_command_snr9931.cpp
endif
ifeq ($(interface_jpd), enable)
SRC += $(MODULE_PATH)/commands/amigos_command_jpd.cpp
endif
ifeq ($(interface_vdf), enable)
SRC += $(MODULE_PATH)/commands/amigos_command_vdf.cpp
endif
ifeq ($(interface_ive), enable)
SRC += $(MODULE_PATH)/commands/amigos_command_ive.cpp
endif
ifeq ($(interface_rgn), enable)
SRC += $(MODULE_PATH)/commands/amigos_command_rgn.cpp
endif
ifeq ($(interface_ipu), enable)
SRC += $(MODULE_PATH)/commands/amigos_command_det.cpp
endif
ifeq ($(interface_hvp), enable)
SRC += $(MODULE_PATH)/commands/amigos_command_hvp.cpp
endif
ifeq ($(interface_hdmirx), enable)
SRC += $(MODULE_PATH)/commands/amigos_command_hdmirx.cpp
SRC += $(MODULE_PATH)/commands/amigos_command_signal_monitor.cpp
endif
ifeq ($(interface_shadow), enable)
ifeq ($(interface_ipu), enable)
    SRC += $(MODULE_PATH)/commands/amigos_command_hseg.cpp
endif
endif
endif
