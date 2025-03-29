DEP += source/internal/common ../common/ss_font source/internal/vif source/internal/isp source/internal/scl source/internal/venc
DEP += source/internal/ai_glasses source/internal/aov
DEP += source/internal/light_misc_control source/$(CHIP)/light_misc_control/driver
DEP += libraries/ptree_ext ../ptree ../common/ssos

$(eval $(call CHECKIF_CONFIG_SET,enable,interface_sensor))
$(eval $(call CHECKIF_CONFIG_SET,enable,interface_vif))
$(eval $(call CHECKIF_CONFIG_SET,enable,interface_isp))
$(eval $(call CHECKIF_CONFIG_SET,enable,interface_scl))
$(eval $(call CHECKIF_CONFIG_SET,enable,interface_ldc))
$(eval $(call CHECKIF_CONFIG_SET,enable,interface_venc))
$(eval $(call CHECKIF_CONFIG_SET,enable,interface_cus3a))
$(eval $(call CHECKIF_CONFIG_SET,enable,interface_ispalgo))

ifneq ($(findstring glasses, $(IMAGE_CONFIG)),)
APP_REL_PREFIX:= bin
endif
