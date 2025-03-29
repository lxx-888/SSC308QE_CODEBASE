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
.PHONY: miservice

OUTPUT_KOFILE_STACK_THD=0x400
OUTPUT_SOFILE_STACK_THD=0x80000

miservice_$(PRODUCT):
miservice: miservice_$(PRODUCT)

miservice_$(PRODUCT):
	@echo [================= $@ =================]

	mkdir -p $(miservice$(RESOURCE))
ifneq ($(DEBUG_MODPARAM), 0)
	-cp -rf $(PROJ_ROOT)/board/$(CHIP)/json/$(PRODUCT)/modparam_debug.json $(miservice$(RESOURCE))/modparam.json
else
	-if [ "$(PURELINUX_FASTBOOT_ENABLE)" != "y" ]; then \
		if [ -n "$(CUST_MODPARAM)" ]; then \
			cp -rf $(PROJ_ROOT)/board/$(CHIP)/json/$(PRODUCT)/$(CUST_MODPARAM) $(miservice$(RESOURCE))/modparam.json; \
		else \
			cp -rf $(PROJ_ROOT)/board/$(CHIP)/json/$(PRODUCT)/modparam_release.json $(miservice$(RESOURCE))/modparam.json; \
		fi; \
	fi
endif

	# pack board cfg/bin file
	-cp -rf $(PROJ_ROOT)/board/$(CHIP)/json/$(PRODUCT)/PowerDomain.json $(miservice$(RESOURCE))/PowerDomain.json
	cp -rf $(PROJ_ROOT)/board/$(CHIP)/$(BOARD_NAME)/config/* $(miservice$(RESOURCE))
	rm -rf $(miservice$(RESOURCE))/config_*.json
	if [ "$(MMAP)" != "" ]; then \
		cp -vf $(PROJ_ROOT)/board/$(CHIP)/mmap/$(MMAP) $(miservice$(RESOURCE))/mmap.ini; \
	fi;
	if [ -d "$(LIB_DIR_PATH)/bin/config_tool/" ]; then \
		cp -rvf $(LIB_DIR_PATH)/bin/config_tool/* $(miservice$(RESOURCE)); \
	fi;

	mkdir -p $(miservice$(RESOURCE))/iqfile/
	$(call copy_if_exists,$(PROJ_ROOT)/board/$(CHIP)/iqfile/isp_api.xml,$(miservice$(RESOURCE))/iqfile/)
	$(call copy_if_exists,$(PROJ_ROOT)/board/$(CHIP)/iqfile/imx307/IMX307_MIPI_HDR_LEF_api.bin,$(miservice$(RESOURCE))/iqfile/)
	$(call copy_if_exists,$(PROJ_ROOT)/board/$(CHIP)/iqfile/nir_api.xml,$(miservice$(RESOURCE))/iqfile/)

	if [ "$(IQ0)" != "" ]; then \
		cp -rf $(PROJ_ROOT)/board/$(CHIP)/iqfile/$(IQ0) $(miservice$(RESOURCE))/iqfile/ -vf; \
		cd $(miservice$(RESOURCE))/iqfile; chmod +x $(shell echo $(IQ0) | awk -F'/' '{print $$NF}'); ln -sf $(shell echo $(IQ0) | awk -F'/' '{print $$NF}') iqfile0.bin; cd -; \
	fi;
	if [ "$(IQ1)" != "" ]; then \
		cp -rf $(PROJ_ROOT)/board/$(CHIP)/iqfile/$(IQ1) $(miservice$(RESOURCE))/iqfile/ -vf; \
		cd $(miservice$(RESOURCE))/iqfile; chmod +x $(shell echo $(IQ1) | awk -F'/' '{print $$NF}'); ln -sf $(shell echo $(IQ1) | awk -F'/' '{print $$NF}') iqfile1.bin; cd -; \
	fi;
	if [ "$(IQ2)" != "" ]; then \
		cp -rf $(PROJ_ROOT)/board/$(CHIP)/iqfile/$(IQ2) $(miservice$(RESOURCE))/iqfile/ -vf; \
		cd $(miservice$(RESOURCE))/iqfile; chmod +x $(shell echo $(IQ2) | awk -F'/' '{print $$NF}'); ln -sf $(shell echo $(IQ2) | awk -F'/' '{print $$NF}') iqfile2.bin; cd -; \
	fi;
	if [ "$(IQ3)" != "" ]; then \
		cp -rf $(PROJ_ROOT)/board/$(CHIP)/iqfile/$(IQ3) $(miservice$(RESOURCE))/iqfile/ -vf; \
		cd $(miservice$(RESOURCE))/iqfile; chmod +x $(shell echo $(IQ3) | awk -F'/' '{print $$NF}'); ln -sf $(shell echo $(IQ3) | awk -F'/' '{print $$NF}') iqfile3.bin; cd -; \
	fi;

	if [[ ! "$(USR_MOUNT_BLOCKS)" =~ "rofiles" ]]; then \
		if [ "$(IQ_API_LIST)" != "" ]; then \
			for f in $(IQ_API_LIST); do \
				$(call copy_if_exists,$(PROJ_ROOT)/board/$(CHIP)/iqfile/$$f,$(miservice$(RESOURCE))/iqfile/) \
			done; \
		fi; \
	fi;

	$(call copy_if_exists,$(PROJ_ROOT)/board/$(CHIP)/venc_fw,$(miservice$(RESOURCE)))

	if [ -d $(PROJ_ROOT)/board/$(CHIP)/dla_file ]; then \
		mkdir $(miservice$(RESOURCE))/dla; \
		cp $(PROJ_ROOT)/board/$(CHIP)/dla_file/ipu_lfw.bin $(miservice$(RESOURCE))/dla; \
		if [ "$(DLA_FIRMWARE_LIST)" != "" -a "$(DUAL_OS)" != "on" ]; then \
			cp -rf $(foreach n,$(DLA_FIRMWARE_LIST),$(PROJ_ROOT)/board/$(CHIP)/dla_file/$(n)) $(miservice$(RESOURCE))/dla; \
		fi;\
	fi;

	if [ -d $(PROJ_ROOT)/board/$(CHIP)/stitch_fw ]; then \
		mkdir $(miservice$(RESOURCE))/stitch_fw; \
		cp $(PROJ_ROOT)/board/$(CHIP)/stitch_fw/multiband_blending $(miservice$(RESOURCE))/stitch_fw; \
	fi;

	if [ -d $(PROJ_ROOT)/board/$(CHIP)/nir_file ]; then \
		mkdir $(miservice$(RESOURCE))/nir_file; \
		cp -rf $(PROJ_ROOT)/board/$(CHIP)/nir_file $(miservice$(RESOURCE)); \
	fi;

	# pack modules
	mkdir -p $(miservice$(RESOURCE))/modules/$(KERNEL_VERSION)
	-if [ -f "$(KERN_MOD_LIST)" ]; then \
		$(call pack_mods,$(KERN_MOD_LIST),$(KERN_MODS_PATH),$(miservice$(RESOURCE))/modules/$(KERNEL_VERSION)) \
	fi;

	-if [ -f "$(KERN_MOD_LATE_LIST)" ]; then \
		$(call pack_mods,$(KERN_MOD_LATE_LIST),$(KERN_MODS_PATH),$(miservice$(RESOURCE))/modules/$(KERNEL_VERSION)) \
	fi;

	if [[ "$(DUAL_OS)" != "on" || "$(CONFIG_ENABLE_POWER_SAVE_AOV)" == "y" ]]; then \
		if [ -f "$(MISC_MOD_LIST)" ]; then \
			$(call pack_mods,$(MISC_MOD_LIST),$(LIB_DIR_PATH)/modules/$(KERNEL_VERSION),$(miservice$(RESOURCE))/modules/$(KERNEL_VERSION)) \
		fi; \
		if [ "$(COMBINE_MI_MODULE)" != "y" ]; then \
			if [ -f "$(MI_MOD_LIST)" ]; then \
				if [ "$(PURELINUX_FASTBOOT_ENABLE)" != "y" ]; then \
					$(call pack_mods,$(MI_MOD_LIST),$(LIB_DIR_PATH)/modules/$(KERNEL_VERSION),$(miservice$(RESOURCE))/modules/$(KERNEL_VERSION)) \
				fi; \
			fi; \
		else \
			if [ "$(PURELINUX_FASTBOOT_ENABLE)" != "y" ]; then \
				cp -vf $(LIB_DIR_PATH)/modules/$(KERNEL_VERSION)/$(COMBINE_MODULE_NAME).ko $(miservice$(RESOURCE))/modules/$(KERNEL_VERSION); \
			fi; \
			for m in $(COMBINE_IGNORE_MODULES); do \
				$(call copy_if_exists,$(LIB_DIR_PATH)/modules/$(KERNEL_VERSION)/mi_$${m}.ko,$(miservice$(RESOURCE))/modules/$(KERNEL_VERSION)) \
			done; \
		fi; \
		if [ -f "$(MISC_MOD_LATE_LIST)" ]; then \
			$(call pack_mods,$(MISC_MOD_LATE_LIST),$(LIB_DIR_PATH)/modules/$(KERNEL_VERSION),$(miservice$(RESOURCE))/modules/$(KERNEL_VERSION)) \
		fi; \
		if [ "$(SENSOR_LIST)" != "" ]; then \
			if [ "$(PURELINUX_FASTBOOT_ENABLE)" != "y" ]; then \
				cp -rvf $(foreach n,$(SENSOR_LIST),$(LIB_DIR_PATH)/modules/$(KERNEL_VERSION)/$(n)) $(miservice$(RESOURCE))/modules/$(KERNEL_VERSION); \
			fi; \
		fi; \
		if [ "$(VCM_LIST)" != "" ]; then \
			cp -rvf $(foreach n,$(VCM_LIST),$(LIB_DIR_PATH)/modules/$(KERNEL_VERSION)/$(n)) $(miservice$(RESOURCE))/modules/$(KERNEL_VERSION); \
		fi; \
	fi;

	# kernel module that will be packed into image but not insmod
	if [ -n "$(PACK_MOD_LIST)" ]; then \
		echo "$(PACK_MOD_LIST)" | sed "s/ /\n/g" | sed 's#\(.*\).ko\(.*\)#$(KERN_MODS_PATH)/\1.ko#g' | xargs -i cp -rvf {} $(miservice$(RESOURCE))/modules/$(KERNEL_VERSION); \
	fi;

	if [ "$(DUAL_OS)" == "on" ]; then \
		if [ -f "$(DUALOS_MOD_LIST)" ]; then \
			$(call pack_mods,$(DUALOS_MOD_LIST),$(LIB_DIR_PATH)/modules/$(KERNEL_VERSION),$(miservice$(RESOURCE))/modules/$(KERNEL_VERSION)) \
		fi; \
	fi;

	if [ "$(TOOLCHAIN)" = "llvm" ]; then \
		find $(miservice$(RESOURCE))/modules/$(KERNEL_VERSION) -name "*.ko" | xargs llvm-strip $(STRIP_OPTION); \
	else \
		find $(miservice$(RESOURCE))/modules/$(KERNEL_VERSION) -name "*.ko" | xargs $(STRIP) $(STRIP_OPTION); \
	fi

	# pack library
	mkdir -p $(miservice$(RESOURCE))/lib
	-if [ "$(PURELINUX_FASTBOOT_ENABLE)" != "y" ]; then \
		$(call pack_libs, $(MI_LIB_LIST), $(LIB_DIR_PATH)/mi_libs/dynamic, $(miservice$(RESOURCE))/lib) \
	fi;

	if [ "$(PRODUCT)" = "android" ]; then \
		if [[ "$(NDK_CLANG_PREFIX)" =~ "aarch64" ]]; then \
			-cp $(LIB_DIR_PATH)/3rd_party_libs/dynamic/lib64/* $(miservice$(RESOURCE))/lib/; \
			cp $(SIGMA_COM_LIB_DIR_PATH)/dynamic/lib64/* $(miservice$(RESOURCE))/lib/; \
		else \
			-cp $(LIB_DIR_PATH)/3rd_party_libs/dynamic/lib/* $(miservice$(RESOURCE))/lib/; \
			cp $(SIGMA_COM_LIB_DIR_PATH)/dynamic/lib/* $(miservice$(RESOURCE))/lib/; \
		fi; \
	else \
		cp -P $(LIB_DIR_PATH)/3rd_party_libs/dynamic/* $(miservice$(RESOURCE))/lib/; \
		cp $(SIGMA_COM_LIB_DIR_PATH)/dynamic/* $(miservice$(RESOURCE))/lib/; \
		for lib_to_remove in $(REMOVE_LIBS_LIST); do \
			rm -f $(miservice$(RESOURCE))/lib/$$lib_to_remove; \
		done;\
	fi

	if (($(DEBUG) & 256)) || (($(DEBUG) & 1)); then \
		if [ -d $(SIGMA_COM_LIB_DIR_PATH)/asanlib ]; then \
			cp -rf $(SIGMA_COM_LIB_DIR_PATH)/asanlib/* $(miservice$(RESOURCE))/lib/; \
		fi; \
		if [ -f ${OUTPUTDIR}/rootfs/etc/profile ]; then \
			echo export ASAN_OPTIONS=halt_on_error=0:fast_unwind_on_fatal=1:fast_unwind_on_check=1:fast_unwind_on_malloc=0:detect_leaks=1:use_sigaltstack=0:print_full_thread_history=1 >> ${OUTPUTDIR}/rootfs/etc/profile; \
		fi; \
	fi;
	if (($(DEBUG) & 1024)); then \
		if [ -d $(SIGMA_COM_LIB_DIR_PATH)/tsanlib ]; then \
			cp -rf $(SIGMA_COM_LIB_DIR_PATH)/tsanlib/* $(miservice$(RESOURCE))/lib/; \
		fi; \
	fi;
	find $(miservice$(RESOURCE))/lib/ -name "*.so*" -type f | xargs $(STRIP) $(STRIP_OPTION);

	if [ -f $(PROJ_ROOT)/../kernel/scripts/checkstack.pl ]; then \
		echo -e "[\e[1;36msdk/interface/src stack monitor:]\e[0m";\
		if [ "" != "`find $(PROJ_ROOT)/../sdk/interface/ -name *.ko`" ]; then find $(PROJ_ROOT)/../sdk/interface/ -name "*.ko" | xargs $(OBJDUMP) -d | $(PROJ_ROOT)/../kernel/scripts/checkstack.pl $(ARCH) | awk -F':' '{ if( $$NF >= $(OUTPUT_KOFILE_STACK_THD) ) { printf("\033[30;31m%s >= THD:[%d], stack check error.\033[0m\n", $$0,$(OUTPUT_KOFILE_STACK_THD)); if("X$(DEBUG)" == "X") { exit 1;}}}';if [ "$$?" -eq 1 ];then exit -1; fi;fi; \
		echo -e "[\e[1;36mso stack monitor:]\e[0m";\
		for file in `ls $(miservice$(RESOURCE))/lib/*.so`; do { $(OBJDUMP) -d $$file | $(PROJ_ROOT)/../kernel/scripts/checkstack.pl $(ARCH) | awk -F':' '{ if( $$NF >= $(OUTPUT_SOFILE_STACK_THD) ) printf("\033[30;35m%s > THD:[0X%x]\033[0m\n", $$0, $(OUTPUT_SOFILE_STACK_THD))}'; } done; \
	else \
		echo -e "\e[1;31m$(PROJ_ROOT)/../kernel/scripts/checkstack.pl is not exist, monitor stack fail!\e[0m"; \
	fi;
