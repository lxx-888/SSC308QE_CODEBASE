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

LINK_TYPE := dynamic
INTER_LINK_TYPE := static

include $(LIBRARY_PATH)/ptree_ext/pipeline_tree_build_config.mk
ifeq ($(USING_BINARY), 1)
EXEFILE := preload_bin
else
EXEFILE := preload
endif

GCCFLAGS += -Werror

INC += $(LIBRARY_PATH)/ptree
INC += $(LIBRARY_PATH)/ptree/core
INC += $(LIBRARY_PATH)/ptree_ext/db
INC += $(BUILD_TOP)/../common/ssos/list

ifeq ($(interface_sensor), enable)
LIBS += -lmi_sensor
endif
ifeq ($(interface_vif), enable)
LIBS += -lmi_vif
endif
ifeq ($(interface_isp), enable)
LIBS += -lmi_isp
endif
ifeq ($(interface_cus3a), enable)
LIBS += -lcus3a
endif
ifeq ($(interface_ispalgo), enable)
LIBS += -lispalgo
endif
ifeq ($(interface_scl), enable)
LIBS += -lmi_scl
endif
ifeq ($(interface_sys), enable)
LIBS += -lmi_sys
endif
ifeq ($(interface_venc), enable)
LIBS += -lmi_venc
endif
ifeq ($(interface_ldc), enable)
LIBS += -lmi_ldc
endif
ifeq ($(interface_rgn), enable)
LIBS += -lmi_rgn
endif
ifeq ($(interface_disp), enable)
LIBS += -lmi_disp
endif
ifeq ($(interface_vdf), enable)
LIBS += -lmi_vdf
LIBS += -lMD_LINUX
LIBS += -lOD_LINUX
LIBS += -lVG_LINUX
LIBS += -lmi_ive
LIBS += -lmi_shadow
endif
ifeq ($(interface_ai), enable)
LIBS += -lmi_ai
endif
ifeq ($(interface_ao), enable)
LIBS += -lmi_ao
endif
ifeq ($(interface_ipu), enable)
LIBS += -lsstaralgo_det -lmi_ipu
ifeq ($(interface_shadow), enable)
LIBS += -lsstaralgo_hseg
endif
LIBS += -lstdc++
endif

CLANG_FORMAT_ENABLE=yes
CLANG_TIDY_ENABLE=yes
ifeq ($(verify_release_sample_ptree), enable)
APP_REL_PREFIX := $(EXEFILE)
MODULE_REL_FILES := $(LIBRARY_PATH)/ptree_ext/resource/$(CHIP)/*.json
endif
