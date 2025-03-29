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
LINK_TYPE := static
INTER_LINK_TYPE := static

EXEFILE := preview

LIBS += -lluajit
ifeq ($(GLES_LDC_ENABLE),y)
LIBS += -L$(PROJ_ROOT)/release/chip/$(CHIP)/$(PRODUCT)/common/glibc/$(TOOLCHAIN_VERSION)/3rd_party_libs/dynamic/
LIBS += -L$(PROJ_ROOT)/release/chip/$(CHIP)/$(PRODUCT)/common/glibc/$(TOOLCHAIN_VERSION)/mi_libs/dynamic/
LIBS += -lgles_ldc -lmali -lEGL -lGLESv1_CM -lGLESv2
CODEDEFINE += -DGLES_LDC_ENABLE
endif
include $(LIBRARY_PATH)/amigos_ext/amigos_build_config.mk

INC += $(LIBRARY_PATH)/amigos/amigos_base
INC += $(LIBRARY_PATH)/amigos/amigos_instance
INC += $(LIBRARY_PATH)/amigos_ext/amigos_surface
INC += $(BUILD_TOP)/../common/nlohmann

ifeq ($(interface_ai), enable)
LIBS += -lmi_ai
endif
ifeq ($(interface_ao), enable)
LIBS += -lmi_ao
endif
ifeq ($(sort $(interface_ao) $(interface_ai)), enable)
LIBS += -lAEC_LINUX
LIBS += -lAPC_LINUX
LIBS += -lSED_LINUX
LIBS += -lSRC_LINUX
endif
ifeq ($(interface_vif), enable)
LIBS += -lmi_vif
endif
ifeq ($(interface_isp), enable)
INC += $(PROJ_ROOT)/release/include/isp
LIBS += -lmi_isp
endif
ifeq ($(interface_cus3a), enable)
LIBS += -lcus3a -lispalgo
endif
ifeq ($(interface_venc), enable)
LIBS += -lmi_venc
endif
ifeq ($(interface_rgn), enable)
LIBS += -lmi_rgn
endif
ifeq ($(interface_vdec), enable)
LIBS += -lmi_vdec
endif
ifeq ($(interface_ldc), enable)
LIBS += -lmi_ldc
endif
ifeq ($(interface_nir), enable)
LIBS += -lmi_nir
endif
ifeq ($(interface_scl), enable)
LIBS += -lmi_scl
endif
ifeq ($(interface_disp), enable)
LIBS += -lmi_disp
endif
ifeq ($(interface_hdmi), enable)
LIBS += -lmi_hdmi
endif
ifeq ($(interface_iqserver), enable)
LIBS += -lmi_iqserver
LIBS += -lfbc_decode
endif
ifeq ($(interface_sensor), enable)
LIBS += -lmi_sensor
endif
ifeq ($(interface_vdisp), enable)
LIBS += -lmi_vdisp
endif
ifeq ($(interface_pcie), enable)
LIBS += -lmi_pcie
endif
ifneq ($(filter $(CUSTOMER_ENABLED), snr9931), )
LIBS += -L$(BUILD_TOP)/../common/sensor/dh9931/$(CHIP) -ldh9931_sdk
endif
ifeq ($(interface_jpd), enable)
LIBS += -lmi_jpd
endif
ifeq ($(OSDK_DRM), y)
LIBS += -ldrm
CODEDEFINE += -DDRM_ENABLE
PREBUILD_LIBS += drm
endif
ifeq ($(interface_hdmirx), enable)
LIBS += -lmi_hdmirx
endif
ifeq ($(interface_hvp), enable)
LIBS += -lmi_hvp
endif
ifeq ($(interface_ipu), enable)
LIBS += -lsstaralgo_det -lmi_ipu
endif
ifeq ($(interface_ive), enable)
ifeq ($(interface_shadow), enable)
ifeq ($(interface_vdf), enable)
LIBS += -lmi_vdf
LIBS += -lMD_LINUX
LIBS += -lOD_LINUX
LIBS += -lVG_LINUX
endif
LIBS += -lmi_shadow
endif
ifeq ($(interface_ipu), enable)
LIBS += -lsstaralgo_hseg
endif
LIBS += -lmi_ive
endif

ifeq ($(verify_release_sample_amigos), enable)
BWTOOL_REL_FILES := setting
APP_REL_PREFIX   := preview
MODULE_REL_FILES := $(APPLICATION_PATH)/mixer/app_amigos/preview/config/$(CHIP)
MODULE_REL_FILES += $(BWTOOL_REL_FILES).tar.gz
endif

module_ext_install:
ifeq ($(verify_release_sample_amigos), enable)
	@tar -czvf $(BWTOOL_REL_FILES).tar.gz -C $(LIBRARY_PATH)/amigos_ext/surface/ $(BWTOOL_REL_FILES)
endif
ifneq ($(wildcard $(BUILD_TOP)/../amigos), )
	@cd $(BUILD_TOP)/../amigos/scripts/pipeline_$(CHIP);./copy.sh && ./setup.sh selfscan=1 stage=0xFFFFFFFF traverse=random && cd -;
endif
