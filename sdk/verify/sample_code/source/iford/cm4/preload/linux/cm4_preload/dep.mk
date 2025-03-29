DEP += source/internal/common source/internal/mailbox source/internal/scl source/internal/isp source/internal/venc source/internal/rtsp_video ../common/ss_rtsp ../common/live555

# $(eval $(call CHECKIF_CONFIG_SET,y,ENABLE_CM4_PRELOAD))
$(eval $(call CHECKIF_CONFIG_SET,enable,interface_venc))

ifeq ($(ENABLE_CM4_PRELOAD),y)
APP_REL_PREFIX:= bin
endif
