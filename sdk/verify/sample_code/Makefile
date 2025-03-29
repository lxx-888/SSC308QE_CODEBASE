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

BUILD_TOP:=$(shell pwd)
LIBRARY_PATH:=$(BUILD_TOP)/libraries
APPLICATION_PATH:=$(BUILD_TOP)/applications
ALKAID_PATH ?= $(BUILD_TOP)/../../..
OUT_PATH:=$(BUILD_TOP)/out
ALKAID_PROJ:=$(ALKAID_PATH)/project
ALKAID_PROJ_CONFIG:=$(ALKAID_PATH)/project/configs/current.configs
3PARTY_PATH:=$(BUILD_TOP)/../3rdparty
IMAGE_PATH := $(BUILD_TOP)/release

define CHECKIF_CONFIG_UNSET
	$(eval CHECK_DEMO := $(patsubst $(BUILD_TOP)/%,%,$(dir $(lastword $(MAKEFILE_LIST)))))
	$(eval CHECK_CFG  := $(value $(2)))
ifeq ($(CHECK_CFG),$(1))
	FILTER_CHECK   += "\033[1;41;43m$(CHECK_DEMO) didn't compiled, because $(2) is set to $(1)!!!\033[0m\n"
	MODULES_FILTER += $(CHECK_DEMO)
endif
endef

define CHECKIF_CONFIG_SET
	$(eval CHECK_DEMO := $(patsubst $(BUILD_TOP)/%,%,$(dir $(lastword $(MAKEFILE_LIST)))))
	$(eval CHECK_CFG  := $(value $(2)))
ifneq ($(CHECK_CFG),$(1))
	FILTER_CHECK   += "\033[1;41;43m$(CHECK_DEMO) didn't compiled, because $(2) is not set to $(1)!!!\033[0m\n"
	MODULES_FILTER += $(CHECK_DEMO)
endif
endef
include $(BUILD_TOP)/build/mi_dep.mk

MODULES := $(sort $(foreach f,$(shell find -type f -name "*.mk"),$(patsubst %/,%,$(dir $(wildcard $(dir $(f))$(notdir $(patsubst %/$(notdir $(f)),%,$(f))).mk)))))
#Filter out build because build/build.mk is not an application's makefile.
MODULES := $(filter-out build,$(foreach m,$(MODULES),$(patsubst ./%,%,$(m))))

ifneq ($(MODULES_IN),)
-include $(foreach mod,$(MODULES_IN),$(BUILD_TOP)/$(mod)/dep.mk)
else
-include $(foreach mod,$(MODULES),$(BUILD_TOP)/$(mod)/dep.mk)
endif

MODULES_FILTER    := $(sort $(foreach m,$(MODULES_FILTER),$(patsubst %/,%,$(m))))
MODULES           := $(filter-out $(MODULES_FILTER),$(MODULES))
MODULES_IN_SOURCE := $(filter source/$(CHIP)/%,$(MODULES))
MODULES           := $(filter-out source/%,$(MODULES))
MODULES           += $(MODULES_IN_SOURCE)

#$(warning MODULES := $(MODULES))

MODULES_CLEAN       := $(foreach m,$(MODULES),$(m)_clean)
MODULES_APP_ALL     := $(foreach m,$(MODULES),$(m)_app_all)
MODULES_APP_CLEAN   := $(foreach m,$(MODULES),$(m)_app_clean)
MODULES_APP_INSTALL := $(foreach m,$(MODULES),$(m)_install)

INTERNAL_LIBS        := $(sort $(foreach d,$(DEP),$(wildcard $(d))))
INTERNAL_LIBS        := $(foreach m,$(INTERNAL_LIBS),$(patsubst %/,%,$(m)))
MODULES_KERNEL       := $(foreach m,$(wildcard $(foreach m,$(INTERNAL_LIBS),$(m)/module.mk)),$(patsubst %/module.mk,%,$(m)))
MODULES_KERNEL_ALL   := $(foreach m,$(MODULES_KERNEL),$(m)_module_all)
MODULES_KERNEL_CLEAN := $(foreach m,$(MODULES_KERNEL),$(m)_module_clean)
MODULES_RTOS         := $(foreach m,$(wildcard $(foreach m,$(INTERNAL_LIBS),$(m)/lib.mak)),$(patsubst %/lib.mak,%,$(m)))
MODULES_USER         := $(foreach m,$(wildcard $(foreach m,$(INTERNAL_LIBS),$(m)/lib.mk)),$(patsubst %/lib.mk,%,$(m)))
MODULES_NONE         := $(filter-out $(MODULES_KERNEL) $(MODULES_RTOS) $(MODULES_USER),$(INTERNAL_LIBS))
# Filter out kernel module and rtos module and user with 'lib.mk', the left is also for user lib.
MODULES_USER       := $(filter $(MODULES_USER) $(MODULES_NONE),$(INTERNAL_LIBS))
MODULES_OBJS_ALL   := $(foreach m,$(MODULES_USER),$(m)_obj_all)
MODULES_OBJS_CLEAN := $(foreach m,$(MODULES_USER),$(m)_obj_clean)
MODULES_LIBS_ALL   := $(foreach m,$(MODULES_USER),$(m)_lib_all)
MODULES_LIBS_CLEAN := $(foreach m,$(MODULES_USER),$(m)_lib_clean)

export CHIP PRODUCT BOARD TOOLCHAIN TOOLCHAIN_VERSION TOOLCHAIN_REL KERNEL_VERSION CUSTOMER_TAILOR CUSTOMER_OPTIONS MOD_PREFIX INTERFACE_ENABLED INTERFACE_DISABLED MHAL_ENABLED MHAL_DISABLED DUAL_OS CHIP_ALIAS MY_TOOLCHAIN
export BUILD_TOP LIBRARY_PATH APPLICATION_PATH
export OUT_PATH IMAGE_PATH
export ALKAID_PROJ
export ALKAID_PROJ_CONFIG
export TARGET_OUT
export ARCH CROSS_COMPILE

.PHONY: all lib obj clean install depend_internal depend_internal_clean $(MODULES) $(MODULES_CLEAN) $(MODULES_APP_ALL) $(MODULES_APP_CLEAN) $(MODULES_OBJS_ALL) $(MODULES_OBJS_CLEAN) $(MODULES_LIBS_ALL) $(MODULES_LIBS_CLEAN)

all:
	@$(MAKE) MODULES_IN="$(MODULES)" depend_internal
ifneq ($(MODULES_APP_ALL), )
	@$(MAKE) $(MODULES_APP_ALL)
endif
	@echo -e $(FILTER_CHECK)

clean:
	@$(MAKE) $(foreach mod,$(MODULES),$(patsubst %,%_app_clean,$(mod)))
	@$(MAKE) MODULES_IN="$(MODULES)" depend_internal_clean
	@rm $(OUT_PATH)/$(ARCH)/app -rf
	@rm $(IMAGE_PATH) -rf

define check_source_file
	if [[ -d "$(BUILD_TOP)/.git" ]] && [[ -z "`git status $(BUILD_TOP)/| grep "nothing to commit, working tree clean"`" ]]; then \
		echo -e "\033[1;41;33mYour commit files should be added and pushed after doing clang-format, please check!!!\033[0m"; \
		exit -1; \
	fi
endef

install:
	@rm -rvf $(PROJ_ROOT)/release/chip/$(CHIP)/$(PRODUCT)/common/$(TOOLCHAIN)/$(TOOLCHAIN_VERSION)/release/bin/sample_code
	@mkdir -p $(PROJ_ROOT)/release/chip/$(CHIP)/$(PRODUCT)/common/$(TOOLCHAIN)/$(TOOLCHAIN_VERSION)/release/bin/sample_code/
	@$(MAKE) $(MODULES_APP_INSTALL)
	@if [ -d $(IMAGE_PATH) ]; then  \
		cp -rvf $(IMAGE_PATH)/* $(PROJ_ROOT)/release/chip/$(CHIP)/$(PRODUCT)/common/$(TOOLCHAIN)/$(TOOLCHAIN_VERSION)/release/bin/sample_code/;  \
	fi;
	@$(call check_source_file)

depend_internal:
ifneq ($(MODULES_KERNEL), )
	@$(MAKE) $(foreach m,$(MODULES_KERNEL),$(m)_module_all)
endif
ifneq ($(MODULES_USER), )
	@$(MAKE) $(foreach m,$(MODULES_USER),$(m)_obj_all)
	@$(MAKE) $(foreach m,$(MODULES_USER),$(m)_lib_all)
endif

depend_internal_clean:
ifneq ($(MODULES_USER), )
	@$(MAKE) $(foreach m,$(MODULES_USER),$(m)_obj_clean)
	@$(MAKE) $(foreach m,$(MODULES_USER),$(m)_lib_clean)
endif
ifneq ($(MODULES_KERNEL), )
	@$(MAKE) $(foreach m,$(MODULES_KERNEL),$(m)_module_clean)
endif

$(MODULES_APP_INSTALL):
	@$(MAKE) MODULE_NAME=$(strip $(subst /,_,$(patsubst %_install,%,$@))) MODULE_PATH=$(patsubst %_install,%,$@) APP_MODULE=1 -f $(BUILD_TOP)/build/build.mk module_install

$(MODULES):
	@$(MAKE) MODULES_IN=$@ depend_internal
	@$(MAKE) MODULES_IN=$@ $(patsubst %,%_app_all,$@)

$(MODULES_CLEAN):
	@$(MAKE) MODULES_IN=$(patsubst %_clean,%,$@) $(patsubst %_clean,%_app_clean,$@)
	@$(MAKE) MODULES_IN=$(patsubst %_clean,%,$@) depend_internal_clean
	@echo -e "\033[1;32mmake $@ done\033[0m"

$(MODULES_APP_ALL):
	@$(MAKE) MODULE_PATH=$(patsubst %_app_all,%,$@) -f $(BUILD_TOP)/build/build.mk script_start
	@$(MAKE) MODULE_PATH=$(patsubst %_app_all,%,$@) APP_MODULE=1 -f $(BUILD_TOP)/build/build.mk format_check
	@$(MAKE) MODULE_NAME=$(strip $(subst /,_,$(patsubst %_app_all,%,$@))) MODULE_PATH=$(patsubst %_app_all,%,$@) APP_MODULE=1 -f $(BUILD_TOP)/build/build.mk gen_exe

$(MODULES_APP_CLEAN):
	@$(MAKE) MODULE_NAME=$(strip $(subst /,_,$(patsubst %_app_clean,%,$@))) MODULE_PATH=$(patsubst %_app_clean,%,$@) APP_MODULE=1 -f $(BUILD_TOP)/build/build.mk clean_files
	@$(MAKE) MODULE_PATH=$(patsubst %_app_clean,%,$@) -f $(BUILD_TOP)/build/build.mk script_end
	@echo -e "\033[1;32mmake $@ done\033[0m"

$(MODULES_OBJS_ALL):
	@$(MAKE) MODULE_PATH=$(patsubst %_obj_all,%,$@) -f $(BUILD_TOP)/build/build.mk script_start
	@$(MAKE) MODULE_PATH=$(patsubst %_obj_all,%,$@) -f $(BUILD_TOP)/build/build.mk format_check
	@$(MAKE) MODULE_PATH=$(patsubst %_obj_all,%,$@) -f $(BUILD_TOP)/build/build.mk gen_obj

$(MODULES_OBJS_CLEAN):
	@$(MAKE) MODULE_PATH=$(patsubst %_obj_clean,%,$@) -f $(BUILD_TOP)/build/build.mk clean_files
	@$(MAKE) MODULE_PATH=$(patsubst %_obj_clean,%,$@) -f $(BUILD_TOP)/build/build.mk script_end
	@echo -e "\033[1;32mmake $@ done\033[0m"

$(MODULES_KERNEL_ALL):
	@$(MAKE) MODULE_PATH=$(patsubst %_module_all,%,$@) -f $(BUILD_TOP)/build/build.mk script_start
	@$(MAKE) MODULE_PATH=$(patsubst %_module_all,%,$@) -f $(BUILD_TOP)/build/build.mk format_check
	@$(MAKE) MODULE_PATH=$(patsubst %_module_all,%,$@) KERNEL_MODULE=1 -f $(BUILD_TOP)/build/build.mk kernel_module

$(MODULES_KERNEL_CLEAN):
	@$(MAKE) MODULE_PATH=$(patsubst %_module_clean,%,$@) KERNEL_MODULE=1 -f $(BUILD_TOP)/build/build.mk kernel_module_clean
	@$(MAKE) MODULE_PATH=$(patsubst %_module_clean,%,$@) -f $(BUILD_TOP)/build/build.mk script_end
	@echo -e "\033[1;32mmake $@ done\033[0m"

$(MODULES_LIBS_ALL):
	@$(MAKE) MODULE_NAME=$(patsubst %_lib_all,%,$(notdir $@)) MODULE_PATH=$(patsubst %_lib_all,%,$@) LIB_TYPE=static -f $(BUILD_TOP)/build/build.mk gen_lib
	@$(MAKE) MODULE_NAME=$(patsubst %_lib_all,%,$(notdir $@)) MODULE_PATH=$(patsubst %_lib_all,%,$@) LIB_TYPE=dynamic -f $(BUILD_TOP)/build/build.mk gen_lib

$(MODULES_LIBS_CLEAN):
	@$(MAKE) MODULE_NAME=$(patsubst %_lib_clean,%,$(notdir $@)) MODULE_PATH=$(patsubst %_lib_clean,%,$@) LIB_TYPE=static -f $(BUILD_TOP)/build/build.mk clean_files
	@$(MAKE) MODULE_NAME=$(patsubst %_lib_clean,%,$(notdir $@)) MODULE_PATH=$(patsubst %_lib_clean,%,$@) LIB_TYPE=dynamic -f $(BUILD_TOP)/build/build.mk clean_files
	@echo -e "\033[1;32mmake $@ done\033[0m"
