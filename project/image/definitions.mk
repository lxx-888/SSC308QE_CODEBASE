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

##########################################################
# common define variant
##########################################################
-include ../configs/current.configs

OUTPUTDIR?=$(PROJ_ROOT)/image/output
IMAGEDIR?=$(PROJ_ROOT)/image/output/images
RESOURCE:=_resource
FSTYPE:=_fstype
UBIVOLID:=_ubivolid
PATSIZE:=_patsize
PATNAME:=_patname
MOUNTPT:=_mountpoint
MOUNTTG:=_mounttarget
OPTIONS:=_options
BOOTENV:=_bootenv
MTDPART:=_mtdpart
BOOTTAB0:=_boottab0
BOOTTAB1:=_boottab1
SYSTAB:=_systab
EXTTAB:=_exttab
CISSIZE:=_cissize
BOOTADDR:=_bootaddr
BOOTCMD:=_bootcmd
BOOTREC:=_bootrec
COPIES:=_copies
BLKSIZE:=_blksize
DATASIZE:=_datasize
OTABLK:=_otablk
BBMCFG:=_bbmcfg
BLKCMD:=_blkcmd
BLKENV:=_blkenv
CUSMK:=_cusmk
CUSCMD:=_cuscmd
OTAMASK:=_otamask
MOUNTPARAM:=_mountparam

ifeq ($(CUSTOMIZE_CONFIG), y)
ifneq ($(wildcard configs/customize/$(IMAGE_CONFIG)),)
include configs/customize/$(IMAGE_CONFIG)
endif
else
ifneq ($(wildcard configs/general/$(IMAGE_CONFIG)),)
include configs/general/$(IMAGE_CONFIG)
endif
endif

ifeq ($(FLASH_TYPE), nor)
FLASH_ALIAS:=nor
else ifeq ($(FLASH_TYPE), spinand)
FLASH_ALIAS:=nand
else ifeq ($(FLASH_TYPE), emmc)
FLASH_ALIAS:=emmc
else ifeq ($(FLASH_TYPE), sdmmc)
FLASH_ALIAS:=sdmmc
endif

KBUILD_ROOT:=$(PROJ_ROOT)/kbuild/$(KERNEL_VERSION)

KERN_MODS_PATH:=$(KBUILD_ROOT)/modules
ifeq ($(DUAL_OS), on)
KBUILD_CUST:=$(KBUILD_ROOT)/customize/$(FLASH_TYPE)
else
KBUILD_CUST:=$(KBUILD_ROOT)/customize
endif

RELEASE_ROOT=$(PROJ_ROOT)/release/chip/$(CHIP)/$(PRODUCT)/common/$(TOOLCHAIN)/$(TOOLCHAIN_VERSION)/release
LIB_DIR_PATH:=$(PROJ_ROOT)/release/chip/$(CHIP)/$(PRODUCT)/common/$(TOOLCHAIN)/$(TOOLCHAIN_VERSION)/release
DEBUG_ASAN:=$(shell if [[ "$(DEBUG)X" != "X"  ]] && ((( $(DEBUG) & 256 )) || (( $(DEBUG) & 1 ))); then echo "1"; fi)
SIGMA_COM_LIB_DIR_PATH:=$(PROJ_ROOT)/release/chip/$(CHIP)/sigma_common_libs/$(TOOLCHAIN)/$(TOOLCHAIN_VERSION)/$(if $(DEBUG_ASAN),debug,release)
EARLYINIT_SETTING_JSON_PATH = $(PROJ_ROOT)/../rtos/proj/sc/customer/earlyinit_setting/earlyinit_setting/$(CHIP)
EARLYINIT_SETTING_JSON:=earlyinit_setting.json

KERN_MOD_LIST:=$(KBUILD_CUST)/kernel_mod_list
KERN_MOD_LATE_LIST:=$(KBUILD_CUST)/kernel_mod_list_late
MISC_MOD_LIST:=$(LIB_DIR_PATH)/modules/$(KERNEL_VERSION)/misc_mod_list
MISC_MOD_LATE_LIST:=$(LIB_DIR_PATH)/modules/$(KERNEL_VERSION)/misc_mod_list_late
DUALOS_MOD_LIST:=$(LIB_DIR_PATH)/modules/$(KERNEL_VERSION)/dualos_mod_list
MI_MOD_LIST:=$(LIB_DIR_PATH)/modules/$(KERNEL_VERSION)/.mods_depend
MI_LIB_LIST:=$(LIB_DIR_PATH)/mi_libs/.libs_depend

##########################################################
# common define function
##########################################################

define filesize
$(shell printf 0x%x $(shell if [ -f $(1) ]; then  stat -Lc "%s" $(1); else echo "0"; fi))
endef

define sum
$(shell val=0;$(foreach n,$(1),val=$$[$${val}+$(n)];)printf "0x%x" $${val})
endef

define sub
$(shell val=0;val=$$[$${val}+$(1)]; $(foreach n,$(2),val=$$[$${val}-$(n)];) printf "0x%x" $${val})
endef

define multiplyhex
$(shell printf "0x%x" $(shell echo $$[$(1)*$(2)]))
endef

define alignup
$(shell printf "0x%x" $(shell echo $$[$$[$(1)+$(2)-1]&$$[~$$[$(2)-1]]]))
endef

define max
$(shell if [ $(1) \> $(2) ]; then printf "0x%x" $(1); else printf "0x%x" $(2); fi)
endef

define calc_val_size
$(shell val=0;$(foreach n,$(1),val=$$[$${val}+1];)printf "%d" $${val})
endef

define copy_if_exists
if [ -e "$(1)" ];then \
	cp -rf $(1) $(2); \
fi;
endef

##########################################################
# pack modules to a specific folder
# $(1): a file that contain module list
# $(2): src folder
# $(3): dest folder
##########################################################
define pack_mods
cat $(1) | sed 's/\.ko.*$$//' | sed 's#\(.*\)#$(2)/\1.ko#' | xargs -i cp -rvf {} $(3);
endef

##########################################################
# pack modules to a specific folder
# $(1): a file that contain module list
# $(2): src folder
# $(3): dest folder
##########################################################
define pack_libs
cat $(1) | sed 's#\(.*\)#$(2)/\1#' |  xargs -i cp -rvf {} $(3);
endef

##########################################################
# gen mods insmod cmd to a specific file
# $(1): a file that contain module list
# $(2): a folder where modules should be placed
# $(3): output file that contain insmod cmd
##########################################################
define gen_mods_insmod_cmd
if [ -f "$(1)" ]; then \
	cat $(1) | sed "s#\(.*\).ko#insmod $(2)/\1.ko#" >> $(3); \
	echo >> $(3); \
fi;
endef

define gen_mi_mods_insmod_cmd
echo "#mi module" >> $(3); \
if [ "$(COMBINE_MI_MODULE)" != "y" ]; then \
	for line in `cat $(1)`; \
	do \
		if [ -f $(LIB_DIR_PATH)/modules/$(KERNEL_VERSION)/$${line}.ko ]; then \
			echo "insmod $(2)/$${line}.ko" >> $(3); \
		fi; \
	done; \
else \
	echo "insmod $(2)/$(COMBINE_MODULE_NAME).ko" >> $(3); \
	for line in `cat $(1)`; \
	do \
		if [ "$$(echo "$(COMBINE_IGNORE_MODULES)"|grep -w $${line#*_})" != "" ]; then \
			echo "insmod $(2)/$${line}.ko" >> $(3); \
		fi; \
	done; \
fi; \
echo >> $(3);
endef

##########################################################
# gen mods insmod cmd to a specific file
# $(1): a folder where modules should be placed
# $(2): output file that contain insmod cmd
##########################################################
define gen_snr_mods_insmod_cmd
echo "#mi sensor" >> $(2); \
if [ "$(SENSOR0)" != "" ]; then \
	echo "insmod $(1)/$(SENSOR0) $(SENSOR0_OPT)" >> $(2); \
fi; \
if [ "$(SENSOR1)" != "" ]; then \
	echo "insmod $(1)/$(SENSOR1) $(SENSOR1_OPT)" >> $(2); \
fi; \
if [ "$(SENSOR2)" != "" ]; then \
	echo "insmod $(1)/$(SENSOR2) $(SENSOR2_OPT)" >> $(2); \
fi; \
if [ "$(VCM0)" != "" ]; then \
	echo "insmod $(1)/$(VCM0) $(VCM0_OPT)" >> $(2); \
fi; \
if [ "$(VCM1)" != "" ]; then \
	echo "insmod $(1)/$(VCM1) $(VCM1_OPT)" >> $(2); \
fi; \
if [ "$(VCM2)" != "" ]; then \
	echo "insmod $(1)/$(VCM2) $(VCM2_OPT)" >> $(2); \
fi; \
found_path=$$(find $(customer$(RESOURCE)) -type f -name 'light_misc_control.ko'); \
if [ -n "$${found_path}" ]; then \
	echo "insmod /customer$${found_path##$(customer$(RESOURCE))}" >> $(2); \
fi; \
echo >> $(2);
endef

##########################################################
# insmod cmd line indent
# $(1): a tmp file that contain no indented insmod cmd line
# $(2): a new file that contain indented insmod cmd line
# #(3): indent length
##########################################################
define indent_insmod_cmd_lines
SPACE=$$(printf '%*s' $(3)); \
cat $(1) | sed "/^$$/d" | sed "s/^/$${SPACE}/g" >> $(2); \
rm -rf $(1);
endef

##########################################################
# pack iq bin files to a specific folder
# $(1): sensor type
# $(2): sensor index
# $(3): dest folder
##########################################################
define pack_iqfiles
	$(eval ISP_IQ_FILE := iqfile$(2).bin) \
	if [ "$(1)" != "" ]; then \
		$(call copy_if_exists,$(PROJ_ROOT)/board/$(CHIP)/iqfile/$(1)/$(1)_iqfile.bin,$(3)/$(ISP_IQ_FILE)) \
	fi;
endef

##########################################################
# pack sensor api bin files to a specific folder
# $(1): sensor type
# $(2): sensor index
# $(3): dest folder
##########################################################
define pack_apifiles
	$(eval ISP_API_FILE := isp_api$(2).bin) \
	$(eval ISP_API_HDR_FILE := isp_api_hdr$(2).bin) \
	$(eval ISP_API_HDR_3F_FILE := isp_api_hdr_3f$(2).bin) \
	$(eval ISP_AWB_FILE := isp_awb_cali$(2).data) \
	$(eval ISP_ALSC_FILE := isp_alsc_cali$(2).data) \
	if [ "$(1)" != "" ]; then \
		$(call copy_if_exists,$(PROJ_ROOT)/board/$(CHIP)/iqfile/$(1)/$(1)_api.bin,$(3)/$(ISP_API_FILE)) \
		$(call copy_if_exists,$(PROJ_ROOT)/board/$(CHIP)/iqfile/$(1)/$(1)_api_hdr.bin,$(3)/$(ISP_API_HDR_FILE)) \
		$(call copy_if_exists,$(PROJ_ROOT)/board/$(CHIP)/iqfile/$(1)/$(1)_api_hdr_3f.bin,$(3)/$(ISP_API_HDR_3F_FILE)) \
		if [ "$(ISP_AWB1)" != "" ]; then \
			$(call copy_if_exists,$(PROJ_ROOT)/board/$(CHIP)/iqfile/$(1)/$(1)_awb_cali.data,$(3)/$(ISP_AWB_FILE)) \
		fi;\
		if [ "$(ISP_ALSC1)" != "" ]; then \
			$(call copy_if_exists,$(PROJ_ROOT)/board/$(CHIP)/iqfile/$(1)/$(1)_alsc_cali.data,$(3)/$(ISP_ALSC_FILE)) \
		fi;\
	fi;
endef

##########################################################
# pack littlefs-fuse files to a specific folder
# $(1): dest folder
##########################################################
define pack_littlefs_fuse
	if [ "$(TOOLCHAIN)" = "glibc" ] && [ "$(TOOLCHAIN_VERSION)" = "10.2.1" ]; then \
		tar -vxf $(PROJ_ROOT)/image/fuse/fuse-2.9.9-aarch64-linux-glibc-10.2.1.tar.gz -C $(1); \
		tar -vxf $(PROJ_ROOT)/image/firmwarefs-fuse/firmwarefs-fuse-2.2.0-aarch64-linux-gnu-glibc-9.2.1.tar.gz -C $(1); \
	fi; \
	if [ "$(TOOLCHAIN)" = "glibc" ] && [ "$(TOOLCHAIN_VERSION)" = "9.1.0" ]; then \
		tar -vxf $(PROJ_ROOT)/image/fuse/fuse-2.9.9-arm-linux-gnueabihf-glibc-9.1.0.tar.gz -C $(1); \
		tar -vxf $(PROJ_ROOT)/image/littlefs-fuse/littlefs-fuse-2.2.0-arm-linux-gnueabihf-glibc-9.1.0.tar.gz -C $(1); \
	fi; \
	if [ "$(TOOLCHAIN)" = "glibc" ] && [ "$(TOOLCHAIN_VERSION)" = "8.2.1" ]; then \
		tar -vxf $(PROJ_ROOT)/image/fuse/fuse-2.9.9-arm-linux-gnueabihf-glibc-8.2.1.tar.gz -C $(1); \
		tar -vxf $(PROJ_ROOT)/image/littlefs-fuse/littlefs-fuse-2.2.0-arm-linux-gnueabihf-glibc-8.2.1.tar.gz -C $(1); \
	fi;
endef

##########################################################
# pack firmwarefs-fuse files to a specific folder
# $(1): dest folder
##########################################################
define pack_firmwarefs_fuse
	if [ "$(ARCH)" = "arm64" ] && [ "$(TOOLCHAIN)" = "glibc" ] && [ "$(TOOLCHAIN_VERSION)" = "10.2.1" ]; then \
		tar -vxf $(PROJ_ROOT)/image/fuse/fuse-2.9.9-aarch64-linux-glibc-10.2.1.tar.gz -C $(1); \
		tar -vxf $(PROJ_ROOT)/image/firmwarefs-fuse/firmwarefs-fuse-2.2.0-aarch64-linux-gnu-glibc-9.2.1.tar.gz -C $(1); \
	fi; \
	if [ "$(TOOLCHAIN)" = "glibc" ] && [ "$(TOOLCHAIN_VERSION)" != "" ]; then \
		tar -vxf $(PROJ_ROOT)/image/fuse/fuse-2.9.9-arm-linux-gnueabihf-glibc-$(TOOLCHAIN_VERSION).tar.gz -C $(1); \
		tar -vxf $(PROJ_ROOT)/image/firmwarefs-fuse/firmwarefs-fuse-2.2.0-arm-linux-gnueabihf-glibc-$(TOOLCHAIN_VERSION).tar.gz -C $(1); \
	fi; \
	if [ "$(TOOLCHAIN)" = "uclibc" ] && [ "$(TOOLCHAIN_VERSION)" != "" ]; then \
		tar -vxf $(PROJ_ROOT)/image/fuse/fuse-2.9.9-arm-sigmastar-linux-uclibcgnueabihf-$(TOOLCHAIN_VERSION).tar.gz -C $(1); \
		tar -vxf $(PROJ_ROOT)/image/firmwarefs-fuse/firmwarefs-fuse-2.2.0-arm-sigmastar-linux-uclibcgnueabihf-$(TOOLCHAIN_VERSION).tar.gz -C $(1); \
	fi;
endef

##########################################################
# gen fwfs/lwfs/lfs mount cmd to a specific file
# $(1): partition identifier
# $(2): output file that contain mount cmd
##########################################################
define gen_mount_fs_cmd
	if [ -n "$($(1)$(MOUNTTG))" ]; then \
		mkdir -p $(3)/$($(1)$(MOUNTTG)); \
	fi; \
	if [ "$($(1)$(FSTYPE))" = "lfs" ]; then \
		$(PYTHON) $(PROJ_ROOT)/image/makefiletools/script/lfs_mount.py --part_size=$($(1)$(PATSIZE)) --rcs_dir=$(2) $($(1)$(MOUNTPT)) $($(1)$(MOUNTTG)); \
	elif [ "$($(1)$(FSTYPE))" = "fwfs" ]; then \
		$(PYTHON) $(PROJ_ROOT)/image/makefiletools/script/fwfs_mount.py --flash_type=$(FLASH_TYPE) --block_size=$(FLASH_BLK_SIZE) --page_size=$(FLASH_PG_SIZE) --part_size=$($(1)$(PATSIZE)) --rcs_dir=$(2) $($(1)$(MOUNTPT)) $($(1)$(MOUNTTG)); \
	elif [ "$($(1)$(FSTYPE))" = "lwfs" ]; then \
		echo "mount -t lwfs $($(1)$(MOUNTPT)) $($(1)$(MOUNTTG))" >> $(2); \
	fi;
endef

##########################################################
# process env config
# 1.import env variables from env config
# 2.gen setenv cmd to burn scripts
# 3.check env consistency by compare env config and burn scripts
##########################################################
define import_env_config
$(if $(wildcard $(1)), \
	$(eval env_var_list := $(shell cat $(1) | grep '^\S\+\s*=.*' | awk -F "=" '{print $$1}' | \
				sed -n 's/\([^:?]\+\)[:?]*$$/\1/p' | sort -u)) \
	$(foreach env,$(env_var_list), \
		$(if $(filter-out "undefined","$(origin $(env))"), \
			$(error can not override uboot env variables: $(env) !!!))) \
	$(eval include $(1)) \
	$(foreach env,$(env_var_list), \
		$(if $($(env)), \
			$(eval __env_var_$(env) := $(value $(env))) \
			$(eval __env_final_var_list += $(env)))) \
)
endef

define gen_setenv_cmd
$(foreach env,$(1), \
	$(if $(filter $(env),$(__env_final_var_list)), \
		echo setenv $(env) $(__env_var_$(env)) >> $(2); \
		$(eval __env_setted_var_list += $(env)))) \
$(if $(__env_setted_var_list), \
	$(if $(filter 1,$(3)),echo saveenv >> $(2);))
endef

define check_env_consistency
$(foreach env,$(__env_final_var_list),__env_var_$(env)=$$(echo $(__env_var_$(env)));) \
if [ -d $(1) ];then \
	declare -A envs=(); \
	script_files=$$(find $(1) -type f); \
	for f in $${script_files};do \
		while read line; \
		do \
			env_line=$$(echo $${line} | sed -n 's#setenv\s\+\(.*\)#\1#p'); \
			if [ "$${env_line}" != "" ];then \
				env=$$(echo $${env_line} | sed -n 's#^\(\S\+\)\s\+\(.*\)#\1#p'); \
				[ -z "$${env}" ] && env=$${env_line}; \
				value=$$(echo $${env_line} | sed -n 's#^\(\S\+\)\s\+\(.*\)#\2#p'); \
				envs[$${env}]="$${value}"; \
			fi; \
		done < $${f}; \
	done; \
	for env in $${!envs[@]};do \
		eval __env_var_value=\$${__env_var_$${env}}; \
		if [ "$$(echo $(__env_final_var_list) | grep -w $${env})" = "" ];then \
			echo -e "\033[31m[[ unknown env variable: $${env} , please check env.config ]]\033[0m"; \
			exit 1; \
		elif [ "$${envs[$${env}]}" != "$${__env_var_value}" ];then \
			echo -e "\033[31m[[ inconsistent env variable: $${env} , please check env.config ]]\033[0m"; \
			exit 1; \
		fi; \
	done; \
	echo "check env consistency pass"; \
fi;
endef

define get_env_size
$(shell \
	path=$(PROJ_ROOT)/board/uboot/$(UBOOT_CONFIG); \
	if [ -f $$path ]; then \
		size=`cat $$path | grep CONFIG_ENV_SIZE | cut -d'=' -f2`; \
		echo $$size; \
	else \
		echo 0x0; \
	fi \
)
endef
