DEP += source/internal/common source/internal/scl source/internal/rgn ../common/ss_font source/internal/venc source/internal/rtsp_video ../common/ss_rtsp ../common/live555

$(eval $(call CHECKIF_CONFIG_SET,enable,interface_scl))
$(eval $(call CHECKIF_CONFIG_SET,enable,interface_venc))
$(eval $(call CHECKIF_CONFIG_SET,enable,interface_rgn))
