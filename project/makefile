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
.PHONY: release image android android_dist

DECOUPLE_CHECK?=1
BSP_SCENE_TYPE:=
export DECOUPLE_CHECK

ifeq ($(filter %defconfig,$(MAKECMDGOALS)),)
-include configs/current.configs
ifneq ($(PURE_RTOS), on)
-include $(KBUILD_MK)
endif
-include $(SOURCE_MK)
-include $(UBOOT_MK)
ifeq ($(RTOS_ENABLE), on)
-include $(RTOS_MK)
endif
ifeq ($(PM_RTOS_ENABLE), on)
-include $(PM_RTOS_MK)
endif
ifneq ($(OPTEE_CONFIG), )
-include $(OPTEE_MK)
endif
endif

ifeq ($(JOBS), )
MAKE_PID:=$(shell echo $$PPID)
CMDLINE:=$(shell tr -d '\0' < /proc/${MAKE_PID}/cmdline)

ifeq ($(findstring -j,${CMDLINE}),-j)
JOBS:=$(shell echo ${CMDLINE} | sed 's/.*-j\([0-9]*\).*/\1/')
ifeq ($(JOBS), )
$(error "OPTION -j is empty. Please set it before running make. eg: make image -j8")
endif
ifeq ($(shell test $(JOBS) -gt 16; echo $$?), 0)
$(error "OPTION -j is too larger. It must be a value <= 16. eg: make image -j8")
endif
else
JOBS:=1
endif

export JOBS
endif

define check_need_build_open_source
$(shell \
	if [[ ! -d ../sdk/interface || "$(MODULES_REL_BUILD_SUPPORT)" = "y" ]] && [[ "$(PURE_RTOS)" != "on" ]];then \
		echo true; \
	else \
		echo false; \
	fi)
endef

image:
	$(call build_pre_process)
ifneq ($(PRODUCT), android)
ifneq ($(PURE_RTOS), on)
	$(MAKE) symbol_link
endif
	$(MAKE) boot
	$(MAKE) headfile_link
ifneq ($(PURE_RTOS), on)
ifneq ($(DEBUG),)
	$(MAKE) linux-kernel-debug
else
	$(MAKE) linux-kernel
endif
endif
	$(MAKE) release
ifeq ($(RTOS_ENABLE), on)
	$(MAKE) rtos
endif
ifeq ($(PM_RTOS_ENABLE), on)
	$(MAKE) pm_rtos
endif
ifneq ($(OPTEE_CONFIG), )
	$(MAKE) optee
ifeq ($(OPTEE_TEST), y)
	$(MAKE) optee_test
endif
endif
	$(MAKE) error_message
	$(MAKE) image_install
else
	$(MAKE) android
	$(MAKE) error_message
endif
	$(MAKE) codesize
ifneq ($(BSP_SCENE_TYPE),)
	$(MAKE) -C $(PROJ_ROOT)/../sdk/verify/bsp_demo bsp_scene_test BSP_SCENE_TYPE=$(BSP_SCENE_TYPE)
endif

image-nocheck:
	$(call build_pre_process)
ifneq ($(PRODUCT), android)
ifneq ($(PURE_RTOS), on)
	$(MAKE) symbol_link
endif
	$(MAKE) boot
	$(MAKE) headfile_link
ifneq ($(PURE_RTOS), on)
ifneq ($(DEBUG), )
	$(MAKE) linux-kernel-debug
else
	$(MAKE) linux-kernel
endif
endif
	$(MAKE) release
ifeq ($(RTOS_ENABLE), on)
	$(MAKE) rtos
endif
ifeq ($(PM_RTOS_ENABLE), on)
	$(MAKE) pm_rtos
endif
ifneq ($(OPTEE_CONFIG), )
	$(MAKE) optee
ifeq ($(OPTEE_TEST), y)
	$(MAKE) optee_test
endif
endif
	$(MAKE) image_install
	$(MAKE) warning_message
else
	$(MAKE) android
	$(MAKE) warning_message
endif
	$(MAKE) codesize

image-release:
	$(MAKE) headfile_link
ifneq ($(PURE_RTOS), on)
	$(MAKE) symbol_link
endif
	$(MAKE) cp_cam_os_libs
	$(MAKE) release
	$(MAKE) error_message
	$(MAKE) image_install
	$(MAKE) codesize

image-release-nocheck:
	$(MAKE) headfile_link
ifneq ($(PURE_RTOS), on)
	$(MAKE) symbol_link
endif
	$(MAKE) cp_cam_os_libs
	$(MAKE) release
	$(MAKE) image_install
	$(MAKE) codesize
	$(MAKE) warning_message

image-fast:
	$(MAKE) headfile_link
	$(MAKE) error_message
	$(MAKE) image_install

image-fast-nocheck:
	$(MAKE) headfile_link
	$(MAKE) image_install
	$(MAKE) warning_message

image-ota:
	$(MAKE) -C image ota_images

image-usb-storage:
	$(MAKE) -C image usb_storage_images

image-usb-ufu:
	$(MAKE) -C image usb_ufu_images

image-sd:
	$(MAKE) -C image sd_images

image-emmc:
	$(MAKE) -C image emmc_images

faker:
SOURCE_CLEAN?=faker

clean:
ifneq ($(PRODUCT), android)
ifneq ($(PURE_RTOS), on)
	-$(MAKE) $(SOURCE_CLEAN)
ifeq ($(check_need_build_open_source), true)
	-$(MAKE) -C $(PROJ_ROOT)/../sdk/linux clean
endif
	-$(MAKE) linux-kernel_clean
endif
ifeq ($(RTOS_ENABLE), on)
	-$(MAKE) rtos_clean
endif
ifeq ($(PM_RTOS_ENABLE), on)
	-$(MAKE) pm_rtos_clean
endif
ifneq ($(OPTEE_CONFIG), )
ifeq ($(OPTEE_TEST), y)
	$(MAKE) optee_test_clean
endif
	$(MAKE) optee_clean
endif
	$(MAKE) image_clean
ifneq ($(PURE_RTOS), on)
	-$(MAKE) symbol_link_clean
endif
	$(MAKE) headfile_link_clean
	$(MAKE) boot_clean
else
	$(MAKE) android_clean
endif
	$(MAKE) codesize_clean

rebuild_sdk_ko:
	$(MAKE) -C $(PROJ_ROOT)/../sdk/linux all
	$(MAKE) -C $(PROJ_ROOT)/../sdk/linux install

image_install: image_clean
	$(MAKE) -C image all

image-release_clean:
	$(MAKE) $(SOURCE_CLEAN)
	$(MAKE) image_clean
	$(MAKE) headfile_link_clean

image_clean:
	$(MAKE) -C image clean

android_dist:
ifneq ($(alkaid_out_dir),)
	@if [ -e $(PROJ_ROOT)/release_alkaid_to_aosp.sh ]; then \
		echo "================================================"; \
		echo "Installing alkaid modules into output directory"; \
		$(PROJ_ROOT)/release_alkaid_to_aosp.sh $(PROJ_ROOT) $(alkaid_out_dir) || exit "$$?"; \
		echo "================================================"; \
	fi
endif

android:
	$(MAKE) symbol_link
	$(MAKE) headfile_link
ifneq ($(SKIP_BUILD_BOOT), 1)
	$(MAKE) boot
endif
ifneq ($(SKIP_BUILD_KERNEL), 1)
	$(MAKE) aosp_common_kernel
endif
	$(MAKE) release
ifeq ($(alkaid_out_dir),)
	$(MAKE) image_install
endif
	$(MAKE) android_dist

android_clean:
	-$(MAKE) $(SOURCE_CLEAN)
ifeq ($(alkaid_out_dir),)
	$(MAKE) image_clean
else
	@rm -rf $(alkaid_out_dir)/*
endif
	-$(MAKE) symbol_link_clean
	$(MAKE) headfile_link_clean
ifneq ($(SKIP_BUILD_BOOT), 1)
	$(MAKE) boot_clean
endif

release:
ifneq ($(SOURCE_RELEASE), )
	$(MAKE) sdk
endif
ifeq ($(check_need_build_open_source), true)
	$(MAKE) -C $(PROJ_ROOT)/../sdk/linux all
	$(MAKE) -C $(PROJ_ROOT)/../sdk/linux install
endif

build_verify: $(SOURCE_RELEASE_DEPEND) clean image

headfile_link:
	@header_dirs=`find $(PROJ_ROOT)/release/include/* -maxdepth 0 -type d`; \
		ln -vsf `find -P $${header_dirs} $(PROJ_ROOT)/release/include/*/$(CHIP_ALIAS) -maxdepth 1 -name "*.h"` $(PROJ_ROOT)/release/include/;

headfile_link_clean:
	@find -P $(PROJ_ROOT)/release/include/ -maxdepth 1 -type l -name "*" | xargs rm -f

warning_message:
	@if [ -d ".git" ]; then \
		cd  release/include && \
		git status -s . | \
		awk -F' ' '{a = a "\033[7;34;42m""release/include/"$$2"\033[0m\n"} END {if(a){ \
				print "\033[5;41;33mWARNING!!!! You CAN NOT MODIFY HEAD FILES ,NEED MI OWNER TO REVIEW THE FOLLOWING INCONSISTENT HEAD FILES!!!\033[0m"; \
				print a;}}'; \
	fi;

error_message:warning_message
	@if [ -d ".git" ]; then \
		cd  release/include && \
		git status -s . | \
		awk -F' ' '{a = a "\033[7;34;42m""release/include/"$$2"\033[0m\n"} END {if(a){ \
				print "\033[5;41;33mIF YOU WANT DEBUG ONLY, pelase using command :[[[ make image-nocheck ]]] INSTEADED!!!\033[0m"; \
				exit 127;}}'; \
	fi;


# kbuild system config
srctree = $(PWD)
export srctree

HOSTCC = gcc
HOSTCFLAGS = -Wall -Wmissing-prototypes -Wstrict-prototypes -O2 -fomit-frame-pointer -std=gnu89
export HOSTCC HOSTCFLAGS

CODESIZE_STATISTICS:=$(CODESIZE_STATISTICS)
CODESIZE_DEFCONFIG:=$(PROJECT_DEFCONFIG)

export CODESIZE_DEFCONFIG CODESIZE_STATISTICS
ALKAID_MHAL_UT ?= 0
KCONFIG_CONFIG ?= .config
MENUCONFIG_CONFIG ?= configs/current.configs

# We need some generic definitions(do not try to remake the file).
scripts/Kbuild.include: ;
-include scripts/Kbuild.include

PHONY += scripts_basic
scripts_basic:
	$(MAKE) $(build)=scripts/basic

scripts/basic/%: scripts_basic ;

silentoldconfig:
	$(MAKE) $(build)=scripts/kconfig $@

config: scripts_basic FORCE
	$(MAKE) $(build)=scripts/kconfig $@
	$(MAKE) silentoldconfig
	$(call post_process_menuconfig)

defconfigs_target=$(foreach n,$(filter %_defconfig,$(wildcard $(PWD)/configs/verify/defconfigs/*) $(wildcard $(PWD)/configs/demo/defconfigs/*)),$(notdir $(n)))
PHONY += $(defconfigs_target)
$(defconfigs_target):scripts_basic FORCE
	$(MAKE) $(build)=scripts/kconfig $@
	$(MAKE) silentoldconfig
	$(call post_process_menuconfig)
	@echo PROJECT_DEFCONFIG=$@ >> $(MENUCONFIG_CONFIG)
savedefconfig:
	$(MAKE) $(build)=scripts/kconfig savedefconfig

codesize_clean:
ifneq ($(wildcard $(PROJ_ROOT)/image/codesize/$(PROJECT_DEFCONFIG)),)
	@rm -rf  $(PROJ_ROOT)/image/codesize/$(PROJECT_DEFCONFIG)/CODESIZE_*
	@echo {$@} done!
endif

codesize:
ifneq ($(CODESIZE_STATISTICS),)
	$(MAKE) codesize_envcheck
	$(MAKE) codesize_blueprint
	$(MAKE) codesize_statistics
	$(MAKE) codesize_xml
	$(MAKE) codesize_autocommit
endif
	@echo {$@} done!

codesize_envcheck:
	@python3  $(PROJ_ROOT)/tools/codesize/codesize_envcheck.py
	@echo -e "\033[0;36m"{$@} done"\033[0m"!

codesize_blueprint:
ifeq ($(wildcard $(PROJ_ROOT)/image/codesize/$(PROJECT_DEFCONFIG)),)
	@mkdir -p $(PROJ_ROOT)/image/codesize/$(PROJECT_DEFCONFIG)
endif
	@$(foreach n, $(notdir $(wildcard $(PROJ_ROOT)/tools/codesize/blueprint/*)), cp -rnf $(PROJ_ROOT)/tools/codesize/blueprint/$(n)  $(PROJ_ROOT)/image/codesize/$(PROJECT_DEFCONFIG)/;)
	@echo -e "\033[0;36m"{$@} done"\033[0m"!

codesize_statistics: codesize_blueprint
	@python3  $(PROJ_ROOT)/tools/codesize/codesize_statistics.py
	@echo -e "\033[0;36m"{$@} done"\033[0m"!
codesize_xml: codesize_blueprint
	@python3  $(PROJ_ROOT)/tools/codesize/codesize_xml.py
	@echo -e "\033[0;36m"{$@} done"\033[0m"!
codesize_autocommit: codesize_blueprint
	@python3  $(PROJ_ROOT)/tools/codesize/codesize_autocommit.py --commiTo=$(CODESIZE_STATISTICS)
	@echo -e "\033[0;36m"{$@} done"\033[0m"!

menuconfig: scripts_basic FORCE
	$(MAKE) $(build)=scripts/kconfig $@
	$(MAKE) silentoldconfig
	$(call post_process_menuconfig)
	@echo PROJECT_DEFCONFIG=$(PROJECT_DEFCONFIG) >> $(MENUCONFIG_CONFIG)

# To avoid any implicit rule to kick in, define an empty command
$(KCONFIG_CONFIG) include/config/auto.conf.cmd: ;

# If .config is newer than include/config/auto.conf, someone tinkered
# with it and forgot to run make oldconfig.
# if auto.conf.cmd is missing then we are probably in a cleaned tree so
# we execute the config step to be sure to catch updated Kconfig files
include/config/%.conf: $(KCONFIG_CONFIG) include/config/auto.conf.cmd
	$(MAKE) silentoldconfig

# configs/current.configs:
# remove blank line and comment line
# a=1 ==> a = 1
# remove CONFIG_
# remove "
# remove trailling space
define defconfig_pre_process
	@sed '/^$$/d; /[[:blank:]]*#/d' $(KCONFIG_CONFIG) > $(MENUCONFIG_CONFIG).in
	@sed -i 's/\([^[:blank:]]\)=\([^[:blank:]]\)/\1 = \2/' $(MENUCONFIG_CONFIG).in
	@sed -i 's/^CONFIG_//g' $(MENUCONFIG_CONFIG).in
	@sed -i 's/"//g' $(MENUCONFIG_CONFIG).in
	@sed -i 's/[[:blank:]]*$$//' $(MENUCONFIG_CONFIG).in
endef

# generate configs/current.configs
# include/generated/autoconf.h:
# remove CONFIG_
# remove KERNEL_VERSION
define post_process_menuconfig
	$(call defconfig_pre_process)
	@$(PWD)/setup_config.sh $(MENUCONFIG_CONFIG).in $(MENUCONFIG_CONFIG) $(ALKAID_MHAL_UT)
	@$(PWD)/build_pre_process_config.sh $(MENUCONFIG_CONFIG).in $(MENUCONFIG_CONFIG)
	@rm -f $(MENUCONFIG_CONFIG).in
	@sed -i 's/CONFIG_//' $(PWD)/include/generated/autoconf.h
	@sed -i '/ KERNEL_VERSION /d' $(PWD)/include/generated/autoconf.h
endef

define build_pre_process
	$(call defconfig_pre_process)
	@$(PROJ_ROOT)/build_pre_process_config.sh $(MENUCONFIG_CONFIG).in $(MENUCONFIG_CONFIG)
	@rm -f $(MENUCONFIG_CONFIG).in
endef
PHONY += FORCE
FORCE:

.PHONY: $(PHONY)
