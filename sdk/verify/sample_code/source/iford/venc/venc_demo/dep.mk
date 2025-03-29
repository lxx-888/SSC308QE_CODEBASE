DEP += source/internal/common  source/internal/venc

$(eval $(call CHECKIF_CONFIG_SET,enable,interface_venc))
