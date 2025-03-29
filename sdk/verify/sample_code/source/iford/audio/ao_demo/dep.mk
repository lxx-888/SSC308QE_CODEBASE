DEP += source/internal/audio

$(eval $(call CHECKIF_CONFIG_SET,enable,interface_ao))
