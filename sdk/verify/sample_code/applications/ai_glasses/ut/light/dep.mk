DEP += source/internal/common source/internal/vif source/internal/light_misc_control source/$(CHIP)/light_misc_control/driver
DEP += libraries/ssos

$(eval $(call CHECKIF_CONFIG_SET,enable,interface_sensor))
$(eval $(call CHECKIF_CONFIG_SET,enable,interface_vif))

# MODULE_REL_FILES +=  $(MODULE_OUT)/light_misc_control.ko
