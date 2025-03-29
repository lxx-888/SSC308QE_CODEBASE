DEP += source/internal/venc ../common/ss_font source/internal/aov source/internal/rtsp_video source/internal/common ../common/ss_rtsp ../common/live555 source/internal/audio source/internal/fast_ae

$(eval $(call CHECKIF_CONFIG_SET,enable,interface_sensor))
$(eval $(call CHECKIF_CONFIG_SET,enable,interface_vif))
$(eval $(call CHECKIF_CONFIG_SET,enable,interface_isp))
$(eval $(call CHECKIF_CONFIG_SET,enable,interface_scl))
$(eval $(call CHECKIF_CONFIG_SET,enable,interface_venc))
$(eval $(call CHECKIF_CONFIG_SET,enable,interface_cus3a))
$(eval $(call CHECKIF_CONFIG_SET,enable,interface_ispalgo))
$(eval $(call CHECKIF_CONFIG_SET,enable,interface_ipu))
$(eval $(call CHECKIF_CONFIG_SET,enable,interface_ai))
$(eval $(call CHECKIF_CONFIG_SET,enable,interface_ao))

ifeq ($(CONFIG_ENABLE_POWER_SAVE_AOV),y)
APP_REL_PREFIX:= bin
endif

