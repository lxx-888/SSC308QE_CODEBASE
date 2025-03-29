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
.PHONY: boot boot_clean
-include ../../configs/current.configs
ifneq ($(PRODUCT), android)
BOOT_ROOT=$(PROJ_ROOT)/../boot
else
BOOT_ROOT=$(PROJ_ROOT)/../boot/u-boot
endif
BOOT_DIR=$(BOOT_ROOT)/configs
UBOOT_ROOT=$(PROJ_ROOT)/board/uboot

boot:
	@echo "building uboot..."
	@if [ "$(PRODUCT)" = "android" ]; then \
		source $(BOOT_ROOT)/envsetup.sh;\
	fi;\
	if [[ -f $(BOOT_DIR)/$(UBOOT_CONFIG) ]]; then \
		echo "boot-config: \"$(UBOOT_CONFIG)\""; \
		$(MAKE) -C $(BOOT_ROOT) $(UBOOT_CONFIG); \
		$(MAKE) -C $(BOOT_ROOT) || exit 1; \
		mkdir -p $(UBOOT_ROOT); \
		ln -sf $(BOOT_DIR)/$(UBOOT_CONFIG) $(UBOOT_ROOT)/$(UBOOT_CONFIG); \
		for file in $(BOOT_ROOT)/u-boot*.img.bin; do \
			ln -sf "$$file" "$(UBOOT_ROOT)/$$(echo "$$(basename "$$file")" | sed 's/_spinand//;s/_emmc//')"; \
		done \
	elif [[ ! -n "$(UBOOT_CONFIG)" ]]; then \
	    echo -e "\033[33m boot-config is null, skip uboot\033[0m"; \
	else \
		echo -e "\033[31m UBOOT_CONFIG: \"$(UBOOT_CONFIG)\" is unknown, please check!\033[0m"; \
		exit 1; \
	fi

boot_clean:
	@if [ "$(PRODUCT)" = "android" ]; then \
		source $(BOOT_ROOT)/envsetup.sh;\
	fi;\
	if [[ -f $(BOOT_DIR)/$(UBOOT_CONFIG) ]]; then \
		$(MAKE) -C $(BOOT_ROOT) clean; \
		rm -f $(UBOOT_ROOT)/u-boot*.img.bin; \
		rm -f $(UBOOT_ROOT)/$(UBOOT_CONFIG); \
	elif [[ ! -n "$(UBOOT_CONFIG)" ]]; then \
		echo -e "\033[33m boot-config is null, skip uboot\033[0m"; \
	else \
		echo -e "\033[31m UBOOT_CONFIG: \"$(UBOOT_CONFIG)\" is unknown, please check!\033[0m"; \
		exit 1; \
	fi
