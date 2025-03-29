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
.PHONY: rtos rtos_clean gen_earlyinit_setting_header
-include ../../configs/current.configs
RTOS_ROOT=$(PROJ_ROOT)/../rtos/proj
RTOS_CONFIGS_DIR=$(RTOS_ROOT)/mak/defconfigs
RTOS_IMAGES_ROOT=$(PROJ_ROOT)/board/rtos
RTOS_OUT=${RTOS_CONFIG:%_defconfig=%}

#
# param $(1) : the rtos config file that need to modify.
#
# only set CONFIG_RTOS_MCU_ASAN_SUPPORT=y here because
# make 'rtos' target will make the defconfig later,
# that will set only check 4 bytes default.
#
# make with DEBUG=2048 will set CONFIG_RTOS_MCU_ASAN_SUPPORT
# make without DEBUG=2048 will reset CONFIG_RTOS_MCU_ASAN_SUPPORT to not set
#
define modify_stacksize_for_rtos_asan
	if [ "$(DEBUG)X" != "X" ] && [ $$(($(DEBUG) & 2048)) -ne 0 ]; then \
		python3 $(PROJ_ROOT)/board/rtos/modify_stacksize_for_rtos_asan.py $(1) true;\
	else \
		python3 $(PROJ_ROOT)/board/rtos/modify_stacksize_for_rtos_asan.py $(1) false;\
	fi;
endef

rtos_build_check:
	@if [[ ! -n "$(RTOS_CONFIG)" ]]; then \
	    echo -e "\033[33m rtos-config is null, skip rtos\033[0m"; \
	elif [[ ! -f $(RTOS_CONFIGS_DIR)/$(RTOS_CONFIG) ]]; then \
		echo -e "\033[31m RTOS_CONFIG: \"$(RTOS_CONFIG)\" is unknown, please check!\033[0m"; \
		exit 1; \
	fi

rtos_full_build: rtos_build_check
	@echo "building rtos images..."
	@$(MAKE) gen_earlyinit_setting_header || exit 1;
	$(call modify_stacksize_for_rtos_asan, $(RTOS_CONFIGS_DIR)/$(RTOS_CONFIG) )
	cd $(RTOS_ROOT); $(MAKE) $(RTOS_CONFIG); \
	$(MAKE) -C $(RTOS_ROOT) -j8 || exit 1;

rtos_build_post:
	@mkdir -p $(RTOS_IMAGES_ROOT);
#   @ln -f $(RTOS_ROOT)/build/$(RTOS_OUT)/out/$(RTOS_OUT).bin $(RTOS_IMAGES_ROOT)/$(RTOS_OUT).bin;
#	@ln -f $(RTOS_ROOT)/build/$(RTOS_OUT)/out/$(RTOS_OUT).elf $(RTOS_IMAGES_ROOT)/$(RTOS_OUT).elf;
#	@ln -f $(RTOS_ROOT)/build/$(RTOS_OUT)/out/$(RTOS_OUT).map $(RTOS_IMAGES_ROOT)/$(RTOS_OUT).map;
#	@ln -f $(RTOS_ROOT)/build/$(RTOS_OUT)/out/u-$(RTOS_OUT).bin $(RTOS_IMAGES_ROOT)/u-$(RTOS_OUT).bin;
	@ln -f $(RTOS_ROOT)/build/$(RTOS_OUT)/out/$(RTOS_OUT).elf $(RTOS_IMAGES_ROOT)/rtos.elf;
	@ln -f $(RTOS_ROOT)/build/$(RTOS_OUT)/out/u-$(RTOS_OUT).bin $(RTOS_IMAGES_ROOT)/rtos;
	@if [[ -f $(RTOS_ROOT)/build/$(RTOS_OUT)/out/u-$(RTOS_OUT).sz ]]; then \
		ln -f $(RTOS_ROOT)/build/$(RTOS_OUT)/out/u-$(RTOS_OUT).sz $(RTOS_IMAGES_ROOT)/rtos.sz; \
	fi;
	@if [[ -f $(RTOS_ROOT)/build/$(RTOS_OUT)/out/u-earlyinit_fw.bin ]]; then \
		ln -f $(RTOS_ROOT)/build/$(RTOS_OUT)/out/u-earlyinit_fw.bin $(RTOS_IMAGES_ROOT)/earlyinit.bin; \
	fi;


rtos: rtos_full_build
	@$(MAKE) rtos_build_post
	@echo "building rtos images done"

rtos_part_build: rtos_build_check
	@echo "building rtos part libs..."
	@cd $(RTOS_ROOT); \
	$(MAKE) part_update -j8 || exit 1;

rtos_part_update: rtos_part_build
	@$(MAKE) rtos_build_post
	@echo "rtos images update part lib done"

rtos_clean:
	@if [[ -f $(RTOS_CONFIGS_DIR)/$(RTOS_CONFIG) ]]; then \
		cd $(RTOS_ROOT); $(MAKE) -C $(RTOS_ROOT) clean; \
#		rm -f $(RTOS_IMAGES_ROOT)/$(RTOS_OUT).bin; \
#		rm -f $(RTOS_IMAGES_ROOT)/$(RTOS_OUT).elf; \
#		rm -f $(RTOS_IMAGES_ROOT)/$(RTOS_OUT).map; \
#		rm -f $(RTOS_IMAGES_ROOT)/u-$(RTOS_OUT).bin; \
		rm -f $(RTOS_IMAGES_ROOT)/rtos; \
		rm -f $(RTOS_IMAGES_ROOT)/earlyinit.bin; \
	elif [[ ! -n "$(RTOS_CONFIG)" ]]; then \
		echo -e "\033[33m rtos-config is null, skip rtos\033[0m"; \
	else \
		echo -e "\033[31m RTOS_CONFIG: \"$(RTOS_CONFIG)\" is unknown, please check!\033[0m"; \
		exit 1; \
	fi
