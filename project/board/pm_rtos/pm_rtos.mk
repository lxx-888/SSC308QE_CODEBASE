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
.PHONY: pm_rtos pm_rtos_clean
-include ../../configs/current.configs
PM_RTOS_ROOT=$(PROJ_ROOT)/../pm_rtos/proj
PM_RTOS_CONFIGS_DIR=$(RTOS_ROOT)/mak/
PM_RTOS_IMAGES_ROOT=$(PROJ_ROOT)/board/$(CHIP)/pm_rtos/
PM_RTOS_OUTPUT_CONFIG=$(PM_RTOS_ROOT)/config.mak

pm_rtos:
	@echo "building pm_rtos images..."
	@if [[ -n $(PM_RTOS_CONFIGS_DIR)/options_$(PM_RTOS_CONFIG).mak ]]; then \
		cd $(PM_RTOS_ROOT); \
		echo -e "$(PM_RTOS_CONFIG)\n\n\n" | ./tng/configure.pl; \
		sed -i '/COMPILER_EXECUTABLE/ s/.*/COMPILER_EXECUTABLE=$(PM_RTOS_TOOLCHAIN)-gcc/' $(PM_RTOS_OUTPUT_CONFIG); \
		$(MAKE) -C $(PM_RTOS_ROOT) clean; \
		$(MAKE) -C $(PM_RTOS_ROOT) -j8 || exit 1; \
		ln -f $(PM_RTOS_ROOT)/build/$(PM_RTOS_CONFIG)/out/$(PM_RTOS_CONFIG).bin $(PM_RTOS_IMAGES_ROOT)/$(PM_RTOS_BIN); \
	elif [[ ! -n "$(PM_RTOS_CONFIG)" ]]; then \
	    echo -e "\033[33m pm_rtos-config is null, skip pm_rtos\033[0m"; \
	else \
		echo -e "\033[31m PM_RTOS_CONFIG: \"$(PM_RTOS_CONFIG)\" is unknown, please check!\033[0m"; \
		exit 1; \
	fi

pm_rtos_clean:
	@if [[ -n $(PM_RTOS_CONFIGS_DIR)/options_$(PM_RTOS_CONFIG).mak ]]; then \
		cd $(PM_RTOS_ROOT); $(MAKE) -C $(PM_RTOS_ROOT) clean; \
	elif [[ ! -n "$(PM_RTOS_CONFIG)" ]]; then \
		echo -e "\033[33m pm_rtos-config is null, skip pm_rtos\033[0m"; \
	else \
		echo -e "\033[31m PM_RTOS_CONFIG: \"$(PM_RTOS_CONFIG)\" is unknown, please check!\033[0m"; \
		exit 1; \
	fi
