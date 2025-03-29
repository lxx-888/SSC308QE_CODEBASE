DEP += source/internal/aov source/internal/common
DEP += source/internal/ai_glasses source/internal/light_misc_control
DEP += libraries/ptree_ext ../ptree ../common/ssos

$(eval $(call CHECKIF_CONFIG_SET,enable,interface_ipu))
$(eval $(call CHECKIF_CONFIG_SET,enable,interface_sensor))
$(eval $(call CHECKIF_CONFIG_SET,enable,interface_vif))
$(eval $(call CHECKIF_CONFIG_SET,enable,interface_isp))
$(eval $(call CHECKIF_CONFIG_SET,enable,interface_scl))
$(eval $(call CHECKIF_CONFIG_SET,enable,interface_ldc))
$(eval $(call CHECKIF_CONFIG_SET,enable,interface_venc))
$(eval $(call CHECKIF_CONFIG_SET,enable,interface_cus3a))
$(eval $(call CHECKIF_CONFIG_SET,enable,interface_ispalgo))
$(eval $(call CHECKIF_CONFIG_SET,enable,interface_rgn))
$(eval $(call CHECKIF_CONFIG_SET,enable,interface_ai))
$(eval $(call CHECKIF_CONFIG_SET,enable,interface_ao))

ifneq ($(findstring glasses,$(IMAGE_CONFIG)),)
APP_REL_PREFIX:= bin
MODULE_REL_FILES +=  $(BUILD_TOP)/applications/ai_glasses/demo/resource
MODULE_REL_FILES +=  $(BUILD_TOP)/applications/ai_glasses/114bs_wifi/ssw6x5x*
MODULE_REL_FILES +=  $(MODULE_OUT)/light_misc_control.ko
endif
