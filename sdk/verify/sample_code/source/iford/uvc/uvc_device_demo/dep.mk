DEP += source/internal/common source/internal/vif source/internal/isp source/internal/scl source/internal/venc ../common/ss_uvc

$(eval $(call CHECKIF_CONFIG_UNSET,on,DUAL_OS))
$(eval $(call CHECKIF_CONFIG_SET,usbcam,PRODUCT))
$(eval $(call CHECKIF_CONFIG_SET,enable,interface_sensor))
$(eval $(call CHECKIF_CONFIG_SET,enable,interface_vif))
$(eval $(call CHECKIF_CONFIG_SET,enable,interface_isp))
$(eval $(call CHECKIF_CONFIG_SET,enable,interface_scl))
$(eval $(call CHECKIF_CONFIG_SET,enable,interface_venc))
$(eval $(call CHECKIF_CONFIG_SET,enable,interface_cus3a))
$(eval $(call CHECKIF_CONFIG_SET,enable,interface_ispalgo))
$(eval $(call CHECKIF_CONFIG_SET,enable,interface_iqserver))
