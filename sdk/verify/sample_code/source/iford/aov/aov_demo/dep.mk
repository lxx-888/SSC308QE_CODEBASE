DEP += source/internal/aov source/internal/common source/internal/rgn
DEP += source/internal/vif source/internal/isp source/internal/scl source/internal/venc
DEP += source/internal/dla source/internal/audio source/internal/fast_ae
DEP += ../common/ss_font
DEP += source/$(CHIP)/light_misc_control/driver source/internal/light_misc_control

$(eval $(call CHECKIF_CONFIG_SET,y,CONFIG_ENABLE_POWER_SAVE_AOV))
$(eval $(call CHECKIF_CONFIG_SET,enable,interface_ipu))
$(eval $(call CHECKIF_CONFIG_SET,enable,interface_sensor))
$(eval $(call CHECKIF_CONFIG_SET,enable,interface_vif))
$(eval $(call CHECKIF_CONFIG_SET,enable,interface_isp))
$(eval $(call CHECKIF_CONFIG_SET,enable,interface_scl))
$(eval $(call CHECKIF_CONFIG_SET,enable,interface_venc))
$(eval $(call CHECKIF_CONFIG_SET,enable,interface_cus3a))
$(eval $(call CHECKIF_CONFIG_SET,enable,interface_ispalgo))
$(eval $(call CHECKIF_CONFIG_SET,enable,interface_rgn))
$(eval $(call CHECKIF_CONFIG_SET,enable,interface_ai))
$(eval $(call CHECKIF_CONFIG_SET,enable,interface_ao))

ifeq ($(CONFIG_ENABLE_POWER_SAVE_AOV),y)
APP_REL_PREFIX:= bin
MODULE_REL_FILES +=  $(MODULE_OUT)/light_misc_control.ko
endif
