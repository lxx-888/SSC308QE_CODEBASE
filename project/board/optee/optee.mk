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
.PHONY: optee optee_clean
-include ../../configs/current.configs
OPTEE_OS_ROOT=$(PROJ_ROOT)/../optee/optee_os
OPTEE_CLIENT_ROOT=$(PROJ_ROOT)/../optee/optee_client
OPTEE_EXAMPLES_ROOT=$(PROJ_ROOT)/../optee/optee_examples/hello_world
OPTEE_TEST_ROOT=$(PROJ_ROOT)/../optee/optee_test
RELEAS_DIR=$(PROJ_ROOT)/board/$(CHIP)/boot/optee

optee:
	@echo "building optee..."
	@if [[ -d $(OPTEE_OS_ROOT) ]]; then \
		$(MAKE) -C $(OPTEE_OS_ROOT) $(OPTEE_OS_MAKE_OPTION) -j8 || exit -1; \
		cp $(OPTEE_OS_ROOT)/out/$(CHIP)/core/tee.bin $(RELEAS_DIR)/tee.bin; \
		cp $(OPTEE_OS_ROOT)/out/$(CHIP)/core/tee.sz $(RELEAS_DIR)/tee.sz; \
		$(MAKE) -C $(OPTEE_CLIENT_ROOT) -j8 || exit -1; \
		cp -rf $(OPTEE_CLIENT_ROOT)/out/tee-supplicant/tee-supplicant $(RELEAS_DIR)/; \
		cp -rf $(OPTEE_CLIENT_ROOT)/out/libteec/* $(RELEAS_DIR)/; \
		$(MAKE) -C $(OPTEE_EXAMPLES_ROOT)/host/ TEEC_EXPORT=$(OPTEE_CLIENT_ROOT)/out/export/usr --no-builtin-variables || exit -1; \
		cp $(OPTEE_EXAMPLES_ROOT)/host/optee_example_hello_world $(RELEAS_DIR)/; \
		$(MAKE) -C $(OPTEE_EXAMPLES_ROOT)/ta/ TA_DEV_KIT_DIR=$(OPTEE_OS_ROOT)/out/$(CHIP)/export-ta_arm32 --no-builtin-variables PLATFORM=$(CHIP) || exit -1; \
		cp $(OPTEE_EXAMPLES_ROOT)/ta/*.ta $(RELEAS_DIR)/; \
	else \
		echo -e "\033[33m $(OPTEE_OS_ROOT) is not exist, skip optee\033[0m"; \
		exit 0; \
	fi

optee_test:
	@echo "building optee_test..."
	@if [[ -d $(OPTEE_TEST_ROOT) ]]; then \
		$(MAKE) -C $(OPTEE_TEST_ROOT) OPTEE_CLIENT_EXPORT=$(OPTEE_CLIENT_ROOT)/out/export/usr TA_DEV_KIT_DIR=$(OPTEE_OS_ROOT)/out/$(CHIP)/export-ta_arm32 -j8 || exit -1; \
		cp $(OPTEE_TEST_ROOT)/out/xtest/xtest $(RELEAS_DIR)/; \
		cp $(OPTEE_TEST_ROOT)/out/supp_plugin/*.plugin $(RELEAS_DIR)/; \
		for d in $(OPTEE_TEST_ROOT)/out/ta/*/; do \
			cp $${d}/*.ta $(RELEAS_DIR)/; \
		done \
	else \
		echo -e "\033[33m $(OPTEE_TEST_ROOT) is not exist, skip optee_test\033[0m"; \
		exit 0; \
	fi
	
optee_clean:
	@if [[ -d $(OPTEE_OS_ROOT) ]]; then \
		$(MAKE) -C $(OPTEE_EXAMPLES_ROOT) TA_DEV_KIT_DIR=$(OPTEE_OS_ROOT)/out/$(CHIP)/export-ta_arm32 clean; \
		$(MAKE) -C $(OPTEE_CLIENT_ROOT) clean; \
		$(MAKE) -C $(OPTEE_OS_ROOT) $(OPTEE_OS_MAKE_OPTION) clean; \
		else \
		echo -e "\033[33m $(OPTEE_OS_ROOT) is not exist, skip optee\033[0m"; \
		exit 0; \
	fi

optee_test_clean:
	@if [[ -d $(OPTEE_TEST_ROOT) ]]; then \
		$(MAKE) -C $(OPTEE_TEST_ROOT) OPTEE_CLIENT_EXPORT=$(OPTEE_CLIENT_ROOT)/out/export/usr TA_DEV_KIT_DIR=$(OPTEE_OS_ROOT)/out/$(CHIP)/export-ta_arm32 clean; \
		else \
		echo -e "\033[33m $(OPTEE_TEST_ROOT) is not exist, skip optee_test\033[0m"; \
		exit 0; \
	fi
