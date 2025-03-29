DEP += source/internal/audio source/internal/common

$(eval $(call CHECKIF_CONFIG_SET,enable,interface_ai))
