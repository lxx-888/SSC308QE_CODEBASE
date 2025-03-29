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
IMAGE_INSTALL_DIR:=$(OUTPUTDIR)
-include $(PROJ_ROOT)/../sdk/verify/application/app.mk

.PHONY: customer app

customer: customer_$(PRODUCT)

define aov_insmod_cmd_lines
if [ -s  $(customer$(RESOURCE))/demo.sh.in ]; then \
	echo "if [ ! -e /proc/dualos/rtos ];then" >> $(OUTPUTDIR)/customer/demo.sh; \
	$(call indent_insmod_cmd_lines,$(customer$(RESOURCE))/demo.sh.in,$(customer$(RESOURCE))/demo.sh,4) \
	echo "fi" >> $(OUTPUTDIR)/customer/demo.sh; \
fi;
endef

customer_$(PRODUCT):
	@echo [================= $@ =================]

	mkdir -p $(customer$(RESOURCE))
	# pack board cfg/ini file
	cp -rf $(PROJ_ROOT)/board/ini/* $(customer$(RESOURCE))
	-cp -rf $(PROJ_ROOT)/board/$(CHIP)/$(BOARD_NAME)/config/config.json $(customer$(RESOURCE))/config.json
	# pack debug tool
	if [ "$(FLASH_TYPE)" != "nor" ]; then \
		chmod 755 $(LIB_DIR_PATH)/bin/debug/ssh/bin/*; \
		chmod 755 $(LIB_DIR_PATH)/bin/debug/ssh/sbin/*; \
		chmod 755 $(LIB_DIR_PATH)/bin/debug/*; \
		cp -rf $(LIB_DIR_PATH)/bin/debug/* $(customer$(RESOURCE))/; \
	else \
		cp -rf $(LIB_DIR_PATH)/bin/debug/riu* $(customer$(RESOURCE))/; \
		chmod 777 $(customer$(RESOURCE))/riu*; \
	fi;

        # pack cardv
	if [ "$(PRODUCT)" == "cardv" ]; then \
		cp -rf $(LIB_DIR_PATH)/bin/cardv/ $(customer$(RESOURCE))/ ; \
		find $(customer$(RESOURCE))/cardv/ -name "cardv" | xargs $(STRIP) $(STRIP_OPTION); \
		cp -ar $(LIB_DIR_PATH)/bin/audio $(customer$(RESOURCE))/; \
		cp -rf $(LIB_DIR_PATH)/bin/UI $(customer$(RESOURCE))/; \
		$(STRIP)  --strip-unneeded $(customer$(RESOURCE))/UI/bin/*; \
		$(STRIP)  --strip-unneeded $(customer$(RESOURCE))/UI/lib/*; \
	fi;

	# pack sample_code/mi_demo
	@if [[ "X$(DEBUG)" == "X" ]] || ([[ "X$(DEBUG)" != "X" ]] && !((( $(DEBUG) & 1 )) || (( $(DEBUG) & 256 )))); then \
		if [ $(verify_sample_code) = "enable" ]; then \
			cp -rf $(LIB_DIR_PATH)/bin/sample_code $(customer$(RESOURCE))/ ; \
		fi; \
		if [ $(verify_mi_demo) = "enable" ]; then \
			cp -rf $(LIB_DIR_PATH)/bin/mi_demo/ $(customer$(RESOURCE))/ ; \
		fi; \
	fi;

	# pack wifi
	if [ $(SIGMA_WIFI) != "no_wifi" ]; then	\
		mkdir -p  $(customer$(RESOURCE))/wifi ; \
		if [ $(FLASH_TYPE) = "spinand" ]; then \
			cp -rf $(LIB_DIR_PATH)/wifi/libs/libdns  $(customer$(RESOURCE)) -rfd ; \
			cp -rf $(LIB_DIR_PATH)/wifi/libs/ap/*   $(customer$(RESOURCE))/wifi ; \
			cp -rf $(LIB_DIR_PATH)/wifi/bin/ap/*   $(customer$(RESOURCE))/wifi ; \
		fi;	\
		find $(LIB_DIR_PATH)/wifi/bin/ -maxdepth 1 -type f -exec cp -P {}  $(customer$(RESOURCE))/wifi \; ;\
		find $(LIB_DIR_PATH)/wifi/bin/ -maxdepth 1 -type l -exec cp -P {}  $(customer$(RESOURCE))/wifi \; ;\
		find $(LIB_DIR_PATH)/wifi/libs/ -maxdepth 1 -type f -exec cp -P {}  $(customer$(RESOURCE))/wifi \; ;\
		find $(LIB_DIR_PATH)/wifi/libs/ -maxdepth 1 -type l -exec cp -P {}  $(customer$(RESOURCE))/wifi \; ;\
		cp -rf $(LIB_DIR_PATH)/wifi/modules/${SIGMA_WIFI}/*   $(customer$(RESOURCE))/wifi ; \
		cp -rf $(LIB_DIR_PATH)/wifi/configs/*   $(customer$(RESOURCE))/wifi ; \
		sed "s/sigma_wifi_ssw10xb/${SIGMA_WIFI}/g" $(customer$(RESOURCE))/wifi/sigma_wifi_init.sh > $(customer$(RESOURCE))/wifi/sigma_wifi_init.sh.out;	\
		mv $(customer$(RESOURCE))/wifi/sigma_wifi_init.sh.out $(customer$(RESOURCE))/wifi/sigma_wifi_init.sh; \
		chmod 777 $(customer$(RESOURCE))/wifi/sigma_wifi_init.sh; \
		if [[ "$(PRODUCT)" == "cardv" ]] && [[ -d "$(LIB_DIR_PATH)/wifi/webserver" ]]; then \
			cp -rf $(LIB_DIR_PATH)/wifi/webserver $(customer$(RESOURCE))/wifi ; \
			cp -rf $(LIB_DIR_PATH)/wifi/nvconf $(customer$(RESOURCE))/wifi ; \
			cp -rf $(LIB_DIR_PATH)/wifi/goahead $(customer$(RESOURCE))/wifi ; \
			cp -rf $(LIB_DIR_PATH)/wifi/bin/ap/hostapd $(customer$(RESOURCE))/wifi ; \
			cp -rf $(customer$(RESOURCE))/wifi/webserver/www/cgi-bin/cgi_config.bin $(customer$(RESOURCE))/wifi/cgi_config.bin ; \
			cp -rf $(customer$(RESOURCE))/wifi/webserver/www/cgi-bin/net_config.bin $(customer$(RESOURCE))/wifi/net_config.bin ; \
		fi; \
		find $(customer$(RESOURCE))/wifi/ -name "*.so" | xargs $(STRIP) $(STRIP_OPTION);\
		find $(customer$(RESOURCE))/wifi/ -name "hostapd" | xargs $(STRIP) --strip-unneeded;\
		find $(customer$(RESOURCE))/wifi/ -name "*.ko" | xargs $(STRIP) --strip-unneeded;\
	fi;

	# pack optee
	if [ $(OPTEE_CONFIG)x != ""x ]; then	\
		mkdir -p  $(customer$(RESOURCE))/tee ; \
		mkdir -p  $(customer$(RESOURCE))/tee/lib ; \
		mkdir -p  $(customer$(RESOURCE))/tee/lib/optee_armtz ; \
		cp -rf $(PROJ_ROOT)/board/$(CHIP)/boot/optee/tee-supplicant   $(customer$(RESOURCE))/tee/ ; \
		cp -rf $(PROJ_ROOT)/board/$(CHIP)/boot/optee/optee_example_hello_world   $(customer$(RESOURCE))/tee/ ; \
		cp -rf $(PROJ_ROOT)/board/$(CHIP)/boot/optee/libteec.so.*   $(customer$(RESOURCE))/tee/lib/ ; \
		cp -rf $(PROJ_ROOT)/board/$(CHIP)/boot/optee/*.ta   $(customer$(RESOURCE))/tee/lib/optee_armtz/ ; \
		if [ "$(OPTEE_TEST)" == "y" ]; then \
			mkdir -p  $(customer$(RESOURCE))/tee/lib/tee-supplicant/plugins ; \
			cp -rf $(PROJ_ROOT)/board/$(CHIP)/boot/optee/xtest   $(customer$(RESOURCE))/tee/ ; \
			cp -rf $(PROJ_ROOT)/board/$(CHIP)/boot/optee/*.plugin   $(customer$(RESOURCE))/tee/lib/tee-supplicant/plugins/ ; \
		fi; \
		chmod 777 -R $(customer$(RESOURCE))/tee; \
	fi;
	# pack board cfg/bin/resource file for rtos
	if [[ "$(DUAL_OS)" = "on" ]] && [[ "$(FLASH_TYPE)"x != "nor"x ]] ; then \
		cp -rf $(PROJ_ROOT)/board/$(CHIP)/mtd_utils $(customer$(RESOURCE))/ ; \
		cp -rf $(PROJ_ROOT)/board/$(CHIP)/sz_tool $(customer$(RESOURCE))/ ; \
	fi;\

	echo current setting $(USE_SSH)
	if [ "$(USE_SSH)" = "on" ]; then \
		if [ "$(FLASH_TYPE)"x != "nor"x ]; then \
			cp -rfd $(LIB_DIR_PATH)/bin/debug/ssh/lib/* $(OUTPUTDIR)/rootfs/lib/  ;\
		fi; \
	fi;

	# remove sshd in nor flash default
	if [[ "$(FLASH_TYPE)" = "nor" ]] && [[ -d "$(customer$(RESOURCE))/ssh" ]]; then \
		rm -rf $(customer$(RESOURCE))/ssh; \
	fi;

	# scripts
	if [ -f "$(customer$(RESOURCE))/demo.sh" ]; then \
		rm  $(customer$(RESOURCE))/demo.sh; \
	fi;
	touch $(customer$(RESOURCE))/demo.sh
	chmod 755 $(customer$(RESOURCE))/demo.sh
	if [ "$(PRODUCT)" != "android" ]; then \
		if [ "$(CONFIG_ENABLE_POWER_SAVE_AOV)" == "y" ];then \
			echo 'echo 0 > /proc/sys/kernel/printk' >> $(customer$(RESOURCE))/demo.sh; \
		else \
			echo 'echo 4 > /proc/sys/kernel/printk' >> $(customer$(RESOURCE))/demo.sh; \
		fi; \
	fi;
	echo >> $(customer$(RESOURCE))/demo.sh

	$(call gen_mods_insmod_cmd,$(KERN_MOD_LIST),/config/modules/$(KERNEL_VERSION),$(customer$(RESOURCE))/demo.sh)

	if [[ "$(DUAL_OS)" != "on" || "$(CONFIG_ENABLE_POWER_SAVE_AOV)" == "y" ]]; then \
		if [ "$(CONFIG_ENABLE_POWER_SAVE_AOV)" == "y" ];then \
			$(call gen_mods_insmod_cmd,$(MISC_MOD_LIST),/config/modules/$(KERNEL_VERSION),$(customer$(RESOURCE))/demo.sh.in) \
			if [ "$(PURELINUX_FASTBOOT_ENABLE)" != "y" ]; then \
				$(call gen_mi_mods_insmod_cmd,$(MI_MOD_LIST),/config/modules/$(KERNEL_VERSION),$(customer$(RESOURCE))/demo.sh.in) \
			fi; \
			$(call gen_mods_insmod_cmd,$(MISC_MOD_LATE_LIST),/config/modules/$(KERNEL_VERSION),$(customer$(RESOURCE))/demo.sh.in) \
			$(call aov_insmod_cmd_lines) \
			echo >> $(OUTPUTDIR)/customer/demo.sh; \
		else \
			$(call gen_mods_insmod_cmd,$(MISC_MOD_LIST),/config/modules/$(KERNEL_VERSION),$(customer$(RESOURCE))/demo.sh) \
			if [ "$(PURELINUX_FASTBOOT_ENABLE)" != "y" ]; then \
				$(call gen_mi_mods_insmod_cmd,$(MI_MOD_LIST),/config/modules/$(KERNEL_VERSION),$(customer$(RESOURCE))/demo.sh) \
			fi; \
			$(call gen_mods_insmod_cmd,$(MISC_MOD_LATE_LIST),/config/modules/$(KERNEL_VERSION),$(customer$(RESOURCE))/demo.sh) \
		fi; \
		sed -i 's/mi_common.ko/mi_common.ko g_ModParamPath=\/config\/modparam.json/g' $(customer$(RESOURCE))/demo.sh; \
		if [ -n "$(COMBINE_MODULE_NAME)" ]; then \
			sed -i 's/$(COMBINE_MODULE_NAME).ko/$(COMBINE_MODULE_NAME).ko common_init.g_ModParamPath=\/config\/modparam.json/g' $(customer$(RESOURCE))/demo.sh; \
		fi; \
	fi;

	$(call gen_mods_insmod_cmd,$(KERN_MOD_LATE_LIST),/config/modules/$(KERNEL_VERSION),$(customer$(RESOURCE))/demo.sh)

	if [[ "$(DUAL_OS)" != "on" || "$(CONFIG_ENABLE_POWER_SAVE_AOV)" == "y" ]]; then \
		if [ "$(CONFIG_ENABLE_POWER_SAVE_AOV)" == "y" ];then \
			if [ "$(PURELINUX_FASTBOOT_ENABLE)" != "y" ]; then \
				$(call gen_snr_mods_insmod_cmd,/config/modules/$(KERNEL_VERSION),$(customer$(RESOURCE))/demo.sh.in) \
			fi; \
			$(call aov_insmod_cmd_lines) \
		else \
			if [ "$(PURELINUX_FASTBOOT_ENABLE)" != "y" ]; then \
				$(call gen_snr_mods_insmod_cmd,/config/modules/$(KERNEL_VERSION),$(customer$(RESOURCE))/demo.sh) \
			fi; \
		fi; \
		echo >> $(customer$(RESOURCE))/demo.sh; \
	fi;

	echo current setting $(USE_ADB)
	if [ "$(USE_ADB)" = "on" ]; then \
		cp -rf $(LIB_DIR_PATH)/adb/bin/*   $(OUTPUTDIR)/customer ; \
		cp -rf $(LIB_DIR_PATH)/adb/configs/*   $(OUTPUTDIR)/customer ; \
		chmod 777 $(OUTPUTDIR)/customer/start_adbd.sh; \
		chmod 777 $(OUTPUTDIR)/customer/adbd; \
		echo "/customer/start_adbd.sh &" >> $(OUTPUTDIR)/customer/demo.sh;	\
	fi;

	if [ "$(DUAL_OS)" == "on" ]; then \
		if [ "$(CONFIG_ENABLE_POWER_SAVE_AOV)" == "y" ];then \
			$(call gen_mods_insmod_cmd,$(DUALOS_MOD_LIST),/config/modules/$(KERNEL_VERSION),$(customer$(RESOURCE))/demo.sh.in) \
			echo "if [ -e /proc/dualos/rtos ];then" >> $(OUTPUTDIR)/customer/demo.sh; \
			$(call indent_insmod_cmd_lines,$(customer$(RESOURCE))/demo.sh.in,$(customer$(RESOURCE))/demo.sh,4) \
			echo "    echo cli loglevel 7 > /proc/dualos/rtos" >> $(OUTPUTDIR)/customer/demo.sh; \
			echo "fi" >> $(OUTPUTDIR)/customer/demo.sh; \
		else \
			$(call gen_mods_insmod_cmd,$(DUALOS_MOD_LIST),/config/modules/$(KERNEL_VERSION),$(customer$(RESOURCE))/demo.sh) \
			echo "echo cli loglevel 7 > /proc/dualos/rtos" >> $(OUTPUTDIR)/customer/demo.sh; \
		fi; \
		echo >> $(OUTPUTDIR)/customer/demo.sh; \
	fi;

	if [ -n "$(PACK_MOD_LIST)" ]; then \
		echo "$(PACK_MOD_LIST)" | sed "s/ /\n/g" | xargs -i sed "/{}/d" -i $(customer$(RESOURCE))/demo.sh; \
	fi;

	echo mdev -s >> $(customer$(RESOURCE))/demo.sh

	#for writeback.sh schipt
	if [[ "$(DUAL_OS)" = "on" ]] && [[ "$(FLASH_TYPE)"x != "nor"x ]]; then \
		echo "chmod 777 /customer/writeback.sh" >> $(OUTPUTDIR)/customer/demo.sh; \
		echo "chmod 777 /customer/mtd_utils/*" >> $(OUTPUTDIR)/customer/demo.sh; \
		echo "chmod 777 /customer/sz_tool/*" >> $(OUTPUTDIR)/customer/demo.sh; \
		if [ -f "$(customer$(RESOURCE))/writeback.sh" ]; then \
			rm  $(customer$(RESOURCE))/writeback.sh; \
		fi; \
		touch $(customer$(RESOURCE))/writeback.sh; \
		echo "umount /misc" >> $(customer$(RESOURCE))/writeback.sh; \
		echo "cd /customer/mtd_utils" >> $(customer$(RESOURCE))/writeback.sh; \
		echo "./nanddump -s 0 -l $(misc$(PATSIZE)) -f /customer/sz_tool/misc.$(misc$(FSTYPE)) $(misc$(MOUNTPT))" >> $(customer$(RESOURCE))/writeback.sh; \
		echo "./flash_erase $(misc_rtos$(MOUNTPT)) 0 0" >> $(customer$(RESOURCE))/writeback.sh; \
		if [ "$(misc_rtos$(OPTIONS))" = "sz" ]; then	\
			echo "cd /customer/sz_tool" >> $(customer$(RESOURCE))/writeback.sh; \
			echo "./mkimage -A arm -O linux -C none -a $(RTOS_RAMDISK_LOAD_ADDR) -e $(RTOS_RAMDISK_LOAD_ADDR) -n \"misc.$(misc$(FSTYPE))\" -d misc.$(misc$(FSTYPE)) u-misc.$(misc$(FSTYPE))" >> $(customer$(RESOURCE))/writeback.sh; \
			echo "./sstar_sz_arm.sh -d u-misc.$(misc$(FSTYPE)) -b 4" >> $(customer$(RESOURCE))/writeback.sh; \
			echo "./mkimage -A arm -O linux -C lzma2 -a $(shell awk 'BEGIN{printf("%#x", '${RTOS_RAMDISK_LOAD_ADDR}-0x40')}') -e 0 -n \"misc.$(misc_rtos$(FSTYPE)).$(misc_rtos$(OPTIONS))TeSt\" -d u-misc.$(misc$(FSTYPE)).$(misc_rtos$(OPTIONS)) misc.$(misc_rtos$(FSTYPE)).$(misc_rtos$(OPTIONS))" >> $(customer$(RESOURCE))/writeback.sh; \
		fi; \
		echo "cd /customer/mtd_utils" >> $(customer$(RESOURCE))/writeback.sh; \
		if [ "$(misc_rtos$(OPTIONS))" = "sz" ]; then	\
			echo "./nandwrite -p -s 0x800 --input-skip=0x800 $(misc_rtos$(MOUNTPT)) ../sz_tool/misc.$(misc_rtos$(FSTYPE)).$(misc_rtos$(OPTIONS))" >> $(customer$(RESOURCE))/writeback.sh; \
			echo "./nandwrite -p -s 0 --input-size=0x800 $(misc_rtos$(MOUNTPT)) ../sz_tool/misc.$(misc_rtos$(FSTYPE)).$(misc_rtos$(OPTIONS))" >> $(customer$(RESOURCE))/writeback.sh; \
		else \
			echo "./nandwrite -p -s 0x800 --input-skip=0x800 $(misc_rtos$(MOUNTPT)) ../sz_tool/misc.$(misc_rtos$(FSTYPE))" >> $(customer$(RESOURCE))/writeback.sh; \
			echo "./nandwrite -p -s 0 --input-size=0x800 $(misc_rtos$(MOUNTPT)) ../sz_tool/misc.$(misc_rtos$(FSTYPE))" >> $(customer$(RESOURCE))/writeback.sh; \
		fi; \
		echo "cd /customer/sz_tool" >> $(customer$(RESOURCE))/writeback.sh; \
		echo "ls -l" >> $(customer$(RESOURCE))/writeback.sh; \
		echo "rm -rvf misc*" >> $(customer$(RESOURCE))/writeback.sh; \
		echo "rm -rvf u-*" >> $(customer$(RESOURCE))/writeback.sh; \
	fi;
	$(MAKE) app;

	if [[ "$(BOOL_PM_RTOS_ENABLE)" == "y" ]] && [[ "$(ENABLE_CM4_PRELOAD)" == "y" ]] ; then       \
		cp -rf $(PROJ_ROOT)/board/$(CHIP)/pm_rtos/$(PM_RTOS_BIN) $(customer$(RESOURCE))/sample_code/bin/pm_rtos ;\
	fi;

	# pack slot_mark.sh
	if [[ "$(USE_AB_SYSTEM_BOOT)" == "y" ]] ; then	\
		cp -rf $(PROJ_ROOT)/image/makefiletools/script/slot_mark.sh $(customer$(RESOURCE)); \
		echo '/customer/slot_mark.sh' >> $(customer$(RESOURCE))/demo.sh; \
	fi;

