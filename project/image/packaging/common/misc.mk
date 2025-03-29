# SigmaStar trade secret
# Copyright (c) [2019~2020] SigmaStar Technology.
# All rights reserved.
#
# Unless otherwise stipulated in writing, any and all information contained
# herein regardless in any format shall remain the sole proprietary of
# SigmaStar and be kept in strict confidence
# (SigmaStar Confidential Information) by the recipient.
# Any unauthorized act including without limitation unauthorized disclosure,
# copying, use, reproduction, sale, distribution, modification, disassembling,
# reverse engineering and compiling of the contents of SigmaStar Confidential
# Information is unlawful and strictly prohibited. SigmaStar hereby reserves the
# rights to any and all damages, losses, costs and expenses resulting therefrom.
#
.PHONY: misc

misc_$(PRODUCT):
misc: misc_$(PRODUCT)

misc_$(PRODUCT):
	@echo [================= $@ =================]
	mkdir -p $(misc$(RESOURCE))

	# pack board cfg/resource file for rtos
	if [[ "$(DUAL_OS)" = "on" || "$(PURE_RTOS)" = "on" ]]; then \
		if [[ "$(CONFIG_RTOS_APPLICATION)" = "dualos_camera" || "$(CONFIG_RTOS_APPLICATION)" = "usb_gadget_app" || "$(CONFIG_RTOS_APPLICATION)" = "aov_preload" || "$(CONFIG_RTOS_APPLICATION)" = "app_selector" ]]; then \
			cp $(PROJ_ROOT)/board/rtos/$(PRELOAD_FILE) $(misc$(RESOURCE))/PreloadSetting.txt ; \
		fi;\
		if [ -n "$(CONFIG_PIPELINE_TREE_FILE)" ]; then \
			$(call copy_if_exists,$(PROJ_ROOT)/../sdk/verify/ptree/base/ptree/resource/$(CHIP)/$(CONFIG_PIPELINE_TREE_FILE),$(misc$(RESOURCE))/ptree.json) \
		fi;\
	fi;

	# pack board cfg/resource file
	if [[ "$(DUAL_OS)" = "on" || "$(PURE_RTOS)" = "on" ]]; then \
		if [ "$(DEBUG_MODPARAM)" != "0" ]; then \
			cp -rf $(PROJ_ROOT)/board/json/modparam_debug.json $(misc$(RESOURCE))/modparam.json; \
		elif [ -n "$(CUST_MODPARAM_RTOS)" ]; then  \
			cp -rf $(PROJ_ROOT)/board/$(CHIP)/json/$(PRODUCT)/$(CUST_MODPARAM_RTOS) $(misc$(RESOURCE))/modparam.json; \
		else \
			cp -rf $(PROJ_ROOT)/board/$(CHIP)/json/$(PRODUCT)/modparam_release.json $(misc$(RESOURCE))/modparam.json; \
		fi;\
	fi

	-cp -rf $(PROJ_ROOT)/board/$(CHIP)/$(BOARD_NAME)/config/config.json $(misc$(RESOURCE));

	if [[ "$(USE_BOOTLOGO)" = "on" ]]; then \
		cp -rf $(PROJ_ROOT)/board/ini/misc/sigmastar1024_600.jpg $(misc$(RESOURCE)); \
		cp -rf $(PROJ_ROOT)/board/ini/misc/upgrade.jpg $(misc$(RESOURCE)); \
		cp -rf $(PROJ_ROOT)/board/ini/misc/logo_configuration.json $(misc$(RESOURCE)); \
	fi


	if [ -n "$(CONFIG_EARLYINIT_SETTING_JSON)" ];then \
		$(call copy_if_exists,$(EARLYINIT_SETTING_JSON_PATH)/$(CONFIG_EARLYINIT_SETTING_JSON),$(misc$(RESOURCE))/$(EARLYINIT_SETTING_JSON)) \
	fi

	if [[ ! "$(USR_MOUNT_BLOCKS)" =~ "rofiles" ]]; then \
		if [[ "$(DUAL_OS)" = "on" || "$(PURE_RTOS)" = "on" ]]; then \
			cp -rf $(PROJ_ROOT)/board/rtos/ascii_8x16 $(misc$(RESOURCE))/ ; \
			cp -rf $(PROJ_ROOT)/board/rtos/hanzi_16x16 $(misc$(RESOURCE))/ ; \
			cp -rf $(PROJ_ROOT)/board/rtos/200X131.argb1555 $(misc$(RESOURCE))/200X131.argb ; \
			cp -rf $(PROJ_ROOT)/board/rtos/200X133.bmp $(misc$(RESOURCE))/ ; \
			if [[ "$(DLA_FIRMWARE_LIST)" != "" ]]; then \
				cp -rf $(foreach n,$(DLA_FIRMWARE_LIST),$(PROJ_ROOT)/board/$(CHIP)/dla_file/$(n)) $(misc$(RESOURCE))/; \
			fi; \
		fi; \
		$(call pack_apifiles,$(SENSOR_TYPE),0,$(misc$(RESOURCE))) \
		$(call pack_apifiles,$(SENSOR_TYPE1),1,$(misc$(RESOURCE))) \
		$(call pack_apifiles,$(SENSOR_TYPE2),2,$(misc$(RESOURCE))) \
	fi

	if [ "$(PURE_RTOS)" = "on" ]; then \
		$(call copy_if_exists,$(PROJ_ROOT)/board/$(CHIP)/iqfile/isp_api.xml, $(misc$(RESOURCE))) \
	fi
	$(call pack_iqfiles,$(SENSOR_TYPE),0,$(misc$(RESOURCE)))
	$(call pack_iqfiles,$(SENSOR_TYPE1),1,$(misc$(RESOURCE)))
	$(call pack_iqfiles,$(SENSOR_TYPE2),2,$(misc$(RESOURCE)))

	if [ "$(DUAL_OS)" = "on" ]; then \
		if [ "$(misc_rtos$(RESOURCE))" != "" ]; then \
			cp -rf $(misc$(RESOURCE))/* $(misc_rtos$(RESOURCE)); \
		fi;\
	fi
