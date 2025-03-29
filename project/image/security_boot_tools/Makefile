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

.PHONY:clean

all: image_prepare secure_boot

UBOOTENV_SIGCMD_ENABLE=YES

SECEREBOOT_PWD := $(shell pwd)
PROJECT_PWD := $(SECEREBOOT_PWD)/../../
SHELL_OPTIONS:=

OUTPUTDIR:=$(PROJECT_PWD)/image/output
IMAGEDIR:=$(PROJECT_PWD)/image/output/images
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
UBIVOLID:=_ubivolid
BLKENV:=_blkenv
CUSMK:=_cusmk
CUSCMD:=_cuscmd
OTAMASK:=_otamask
_FINAL:=_final


define filesize
$(shell printf 0x%x $(shell stat -c "%s" $(1)))
endef
define sum
$(shell val=0;$(foreach n,$(1),val=$$[$${val}+$(n)];)printf "0x%x" $${val})
endef
define multiplyhex
$(shell printf "0x%x" $(shell echo $$[$(1)*$(2)]))
endef
define alignup
$(shell printf "0x%x" $(shell echo $$[$$[$(1)+$(2)-1]&$$[~$$[$(2)-1]]]))
endef
define getval
$(shell if [ -n "$(2)" ]; then echo $(2); else echo general/$(1); fi;)
endef

define addhex
$(shell printf "0x%x" $(shell echo $$[$(1)+$(2)]))
endef

#hard code for dual os kernel(0x400000--certainly) and the ramdisk are in the same partition.
#kernel_end=0x400000
#ramdisk_size=0x100000
#define filesize_1
#$(shell [ $(1) = kernel ] && [ $(DUAL_OS) = on ] && echo $(call addhex, $(kernel_end),$(ramdisk_size)) || echo $(call filesize, $2))
#endef
define filesize_1
$(call filesize, $2)
endef

#for project dir
-include $(PROJECT_PWD)/configs/current.configs
-include $(PROJECT_PWD)/image/configs/general/$(IMAGE_CONFIG)
#for customer dir
-include $(SECEREBOOT_PWD)/include/current.configs
-include $(SECEREBOOT_PWD)/include/$(IMAGE_CONFIG)

# flash default settings
ifeq ($(FLASH_TYPE), nor)
FLASH_INFO_SUFFIX = nri
FLASH_BLK_SIZE ?= 0x10000
FLASH_BLK_CNT ?= 256
FLASH_PG_SIZE ?= 0x1000
FLASH_SIZE=$(FLASH_PG_SIZE)
else
ifeq ($(FLASH_TYPE), spinand)
FLASH_INFO_SUFFIX = sni
FLASH_BLK_SIZE ?= 0x20000
FLASH_BLK_CNT ?= 1024
FLASH_PG_SIZE ?= 0x800
FLASH_SIZE=$(FLASH_BLK_SIZE)
else
ifeq ($(FLASH_TYPE), emmc)
FLASH_BLK_SIZE ?= 0
FLASH_BLK_CNT ?= 0
FLASH_PG_SIZE ?= 0
else
ifeq ($(FLASH_TYPE), sdmmc)
FLASH_BLK_SIZE ?= 0
FLASH_BLK_CNT ?= 0
FLASH_PG_SIZE ?= 0
endif
endif
endif
endif

TARGET_UBIFS_SIZE:=$(shell echo $(foreach n,$(TARGET_UBIFS),$($(n)$(PATSIZE))) | tr ' ' '\n' | awk '{sum += strtonum($$0)} END {print sum}')

#nor flash image name don't need apped _$(FLASH_TYPE)
ifneq ($(findstring nor,$(FLASH_TYPE)),nor)
_FLASH_TYPE:=_$(FLASH_TYPE)
ifeq ($(FLASH_TYPE), sdmmc)
_FLASH_TYPE:=
endif
endif


#secureboot output image
OUTPUT_SE_BT_IMAGE:=$(SECEREBOOT_PWD)/image_secure
#generate secureboot image directory
SIGN_IMAGE_PATH:=$(SECEREBOOT_PWD)/sboot_sign
IMAGE_SIZE_CONFIG=$(SIGN_IMAGE_PATH)/config
REBUILD_CIS=$(OUTPUT_SE_BT_IMAGE)/$(lastword $(subst /, ,$(cis$(RESOURCE))))

PROJECT_FILES:= $(PROJECT_PWD)/image/makefiletools/bin/snigenerator $(PROJECT_PWD)/image/makefiletools/bin/nrigenerator $(PROJECT_PWD)/image/makefiletools/bin/pnigenerator \
				$(PROJECT_PWD)/image/makefiletools/script/make_sd_upgrade_sigmastar.sh    $(PROJECT_PWD)/image/makefiletools/script/make_usb_upgrade_sigmastar.sh \
				$(PROJECT_PWD)/image/makefiletools/script/make_emmc_upgrade_sigmastar.sh  $(PROJECT_PWD)/image/makefiletools/script/make_usb_factory_sigmastar.sh \
				$(PROJECT_PWD)/image/makefiletools/bin/eccgenerator

ENV_CONFIG_FILES:= $(PROJ_ROOT)/image/definitions.mk $(PROJ_ROOT)/image/script.mk $(PROJECT_PWD)/image/configs/general/env.config

ifeq ($(FLASH_TYPE), sdmmc)
BOOT_IMAGE_LIST=$(fat$(SYSTAB))
IMAGE_LIST=$(system$(SYSTAB))
endif
TARGET_BOOT_IMAGE:=$(foreach n,$(BOOT_IMAGE_LIST),$(n)_mkboot)
SECUREBOOT_IMAGE:=$(foreach n,$(BOOT_IMAGE_LIST),$(n)_secure_mk)

#kernel rootfs
MAY_SIGN_LIST :=kernel rtos pm_rtos pm51 riscvfw earlyinit
COMMON_LIST_1 := $(filter $(MAY_SIGN_LIST), $(IMAGE_LIST))
EXTRA_SIG_LIST := $(addsuffix _secure_mk_ext, $(COMMON_LIST_1))


SBOOT_IMAGE_SIZE:=$(foreach n,$(BOOT_IMAGE_LIST),$(n)_sboot_image_size)

DEAL_LIST := rootfs ramdisk
COMMON_LIST := $(filter $(DEAL_LIST), $(IMAGE_LIST))
FSIMAGE_LIST:=$(filter-out $(patsubst %_fs__,%,$(filter %_fs__, $(foreach n,$(COMMON_LIST),$(n)_fs_$($(n)$(FSTYPE))_))),$(COMMON_LIST))
SE_TARGET_FSIMAGE:=$(foreach n,$(FSIMAGE_LIST),$(n)_$(FLASH_TYPE)_$($(n)$(FSTYPE))_fsimage)


get_alignment = $(if $(filter $(FLASH_TYPE),nor),$(if $(filter $(1),$(IMAGE_LIST)),0x10000,$(FLASH_PG_SIZE)),$(if $(filter $(FLASH_TYPE),emmc),$(if $(filter $(1),$(BOOT_IMAGE_LIST)), 0x1, $(FLASH_BLK_SIZE)),$(FLASH_BLK_SIZE)))

#sigmastar use
IPL_PROJECT_ENABLE := NO
IPL_PROJECT_PATH :=
IPL_BUILD_NAME :=IPL.*
IPLCUST_BUILD_NAME :=IPL_CUST.*
IPLX_BUILD_NAME :=IPLX.*

sys_prepare:
ifneq ($(wildcard $(IMAGEDIR)),)
	@if [ ! -d "$(SIGN_IMAGE_PATH)/" ]; then mkdir -p $(SIGN_IMAGE_PATH); fi

	$(foreach file,$(COMMON_LIST_1),cp $($(file)$(RESOURCE)) $(SIGN_IMAGE_PATH)/ ;)

ifneq (,$(findstring gz_fsimage,$(SE_TARGET_FSIMAGE)))
	cp $(IMAGEDIR)/ramdisk.gz  $(SIGN_IMAGE_PATH)/
endif

ifneq (,$(findstring ramfs_fsimage,$(SE_TARGET_FSIMAGE)))
	cp $(IMAGEDIR)/$(patsubst %_$(FLASH_TYPE)_ramfs_fsimage,%,$@).$($(patsubst %_$(FLASH_TYPE)_ramfs_fsimage,%,$@)$(FSTYPE)) $(SIGN_IMAGE_PATH)/
endif

ifneq (,$(findstring _ubifs_fsimage,$(SE_TARGET_FSIMAGE)))
	cp -rpf $(IMAGEDIR)/$(patsubst %_$(FLASH_TYPE)_ubifs_fsimage,%,$(filter %_$(FLASH_TYPE)_ubifs_fsimage,$(SE_TARGET_FSIMAGE))).ubifs  $(SIGN_IMAGE_PATH)/
endif

ifneq (,$(findstring _squashfs_fsimage,$(SE_TARGET_FSIMAGE)))
	cp -rpf $(IMAGEDIR)/$(patsubst %_$(FLASH_TYPE)_squashfs_fsimage,%,$(filter %_$(FLASH_TYPE)_squashfs_fsimage,$(SE_TARGET_FSIMAGE))).sqfs $(SIGN_IMAGE_PATH)/
endif

ifneq (,$(findstring _ext4_fsimage,$(SE_TARGET_FSIMAGE)))
	cp -rpf $(IMAGEDIR)/$(patsubst %_$(FLASH_TYPE)_ext4_fsimage,%,$(filter %_$(FLASH_TYPE)_ext4_fsimage,$(SE_TARGET_FSIMAGE))).ext4 $(SIGN_IMAGE_PATH)/
endif

endif

image_prepare: sys_prepare
ifneq ($(wildcard $(IMAGEDIR)),)
	if [ ! -d "$(OUTPUT_SE_BT_IMAGE)" ]; then mkdir -p $(OUTPUT_SE_BT_IMAGE); fi
	cp -rf $(PROJECT_PWD)/image/output/images/* $(OUTPUT_SE_BT_IMAGE)/
	if [ ! -d "$(SECEREBOOT_PWD)/include/" ]; then mkdir -p $(SECEREBOOT_PWD)/include/; fi
	cp $(PROJECT_PWD)/configs/current.configs $(SECEREBOOT_PWD)/include/
	cp $(PROJECT_PWD)/image/configs/general/$(IMAGE_CONFIG) $(SECEREBOOT_PWD)/include/
ifneq (,${FLASH_INFO_SUFFIX})
	cp -vf $(PROJECT_PWD)/board/$(CHIP)/boot/$(FLASH_TYPE)/partition/* $(SIGN_IMAGE_PATH)/
	cp -vf $(PROJ_ROOT)/board/$(CHIP)/boot/$(FLASH_TYPE)/partition/* $(OUTPUT_SE_BT_IMAGE)/boot/
endif
	$(foreach file,$(ENV_CONFIG_FILES),cp $(file) $(SECEREBOOT_PWD)/include/;)

	@if [ ! -d "$(SIGN_IMAGE_PATH)/tools/makefiletools/" ]; then mkdir -p $(SIGN_IMAGE_PATH); fi
	@if [ ! -d "$(SECEREBOOT_PWD)/tools/makefiletools/" ]; then mkdir -p $(SECEREBOOT_PWD)/tools/makefiletools/; fi
	$(foreach file,$(PROJECT_FILES),cp $(file) $(SECEREBOOT_PWD)/tools/makefiletools/;)

ifeq (1,${IPL_IN_CIS})
	cp -f $(PROJ_ROOT)/board/$(CHIP)/boot/$(FLASH_TYPE)/ipl/IPL.bin $(OUTPUT_SE_BT_IMAGE)/boot/
endif
	$(foreach file,$(TARGET_BOOT_IMAGE),cp $($(patsubst %_mkboot,%,$(file))$(RESOURCE)) $(SIGN_IMAGE_PATH)/;)
ifeq ($(IPL_PROJECT_ENABLE),YES)
ifeq ($(FLASH_TYPE), emmc)
	cp -vf $(IPL_PROJECT_PATH)/$(IPLX_BUILD_NAME) $(SIGN_IMAGE_PATH)/$(lastword $(subst /, ,$(iplx$(RESOURCE))))
endif
	cp -vf $(IPL_PROJECT_PATH)/$(IPL_BUILD_NAME) $(SIGN_IMAGE_PATH)/$(lastword $(subst /, ,$(ipl$(RESOURCE))))
	cp -vf $(IPL_PROJECT_PATH)/$(IPLCUST_BUILD_NAME) $(SIGN_IMAGE_PATH)/$(lastword $(subst /, ,$(ipl_cust$(RESOURCE))))
endif

	if [ ! -d "$(SIGN_IMAGE_PATH)/usb/" ]; then mkdir -p $(SIGN_IMAGE_PATH)/usb/; fi
	cp -rpf $(PROJ_ROOT)/board/$(CHIP)/boot/usb/upgrade/* $(SIGN_IMAGE_PATH)/usb/
	@echo "[[$@ done!]]"
	@echo " "
endif


%_secure_mk:
	@echo "[[$@]]"
	$(SECEREBOOT_PWD)/tools/deal_image.sh $(SIGN_IMAGE_PATH)/$(lastword $(subst /, ,$($(patsubst %_secure_mk,%,$@)$(RESOURCE)))) $(_FINAL) $(patsubst %_secure_mk,%,$@);
	if [ "$(FLASH_TYPE)" = "sdmmc" ]; then	\
		cp -vf $(SIGN_IMAGE_PATH)/$(lastword $(subst /, ,$($(patsubst %_secure_mk,%,$@)$(RESOURCE))))$(_FINAL) $(OUTPUT_SE_BT_IMAGE)/boot/$(shell echo $(patsubst %_secure_mk,%,$@) |  tr '[a-z]' '[A-Z]');\
	else\
		cp -vf $(SIGN_IMAGE_PATH)/$(lastword $(subst /, ,$($(patsubst %_secure_mk,%,$@)$(RESOURCE))))$(_FINAL) $(OUTPUT_SE_BT_IMAGE)/boot/$(lastword $(subst /, ,$($(patsubst %_secure_mk,%,$@)$(RESOURCE))));\
	fi;

%_secure_mk_ext:
	@echo "[[$@]]"
ifeq ($(wildcard $(OUTPUT_SE_BT_IMAGE)),)
	@echo " [[ pls make image_prepare in project ]]";
	@false;
endif
	$(SECEREBOOT_PWD)/tools/deal_image.sh $(SIGN_IMAGE_PATH)/$(lastword $(subst /, ,$($(patsubst %_secure_mk_ext,%,$@)$(RESOURCE)))) $(_FINAL) $(patsubst %_secure_mk_ext,%,$@);\
	cp $(SIGN_IMAGE_PATH)/$(lastword $(subst /, ,$($(patsubst %_secure_mk_ext,%,$@)$(RESOURCE))))$(_FINAL) $(OUTPUT_SE_BT_IMAGE)/$(patsubst %_secure_mk_ext,%,$@)


%_$(FLASH_TYPE)_gz_fsimage:
	@echo [[$@]]
	$(SECEREBOOT_PWD)/tools/deal_image.sh $(SIGN_IMAGE_PATH)/ramdisk.gz $(_FINAL) $(patsubst %_$(FLASH_TYPE)_gz_fsimage,%,$@);\
	cp $(SIGN_IMAGE_PATH)/ramdisk.gz$(_FINAL) $(OUTPUT_SE_BT_IMAGE)/ramdisk.gz ;


%_$(FLASH_TYPE)_ubifs_fsimage:
	@echo [[$@]]
	$(SECEREBOOT_PWD)/tools/deal_image.sh $(SIGN_IMAGE_PATH)/$(patsubst %_$(FLASH_TYPE)_ubifs_fsimage,%,$@).ubifs $(_FINAL) $(patsubst %_$(FLASH_TYPE)_ubifs_fsimage,%,$@);\
	cp $(SIGN_IMAGE_PATH)/$(patsubst %_$(FLASH_TYPE)_ubifs_fsimage,%,$@).ubifs$(_FINAL) $(OUTPUT_SE_BT_IMAGE)/$(patsubst %_$(FLASH_TYPE)_ubifs_fsimage,%,$@).ubifs

%_$(FLASH_TYPE)_ramfs_fsimage:
	@echo [[$@]]
	$(SECEREBOOT_PWD)/tools/deal_image.sh $(SIGN_IMAGE_PATH)/$(patsubst %_$(FLASH_TYPE)_ramfs_fsimage,%,$@).$($(patsubst %_$(FLASH_TYPE)_ramfs_fsimage,%,$@)$(FSTYPE)) $(_FINAL) $(patsubst %_$(FLASH_TYPE)_ramfs_fsimage,%,$@);\
	mv $(SIGN_IMAGE_PATH)/$(patsubst %_$(FLASH_TYPE)_ramfs_fsimage,%,$@).$($(patsubst %_$(FLASH_TYPE)_ramfs_fsimage,%,$@)$(FSTYPE))$(_FINAL) $(OUTPUT_SE_BT_IMAGE)/$(patsubst %_$(FLASH_TYPE)_ramfs_fsimage,%,$@).$($(patsubst %_$(FLASH_TYPE)_ramfs_fsimage,%,$@)$(FSTYPE));        \


%_$(FLASH_TYPE)_squashfs_fsimage:
	@echo [[$@]]
	$(SECEREBOOT_PWD)/tools/deal_image.sh $(SIGN_IMAGE_PATH)/$(patsubst %_$(FLASH_TYPE)_squashfs_fsimage,%,$@).sqfs $(_FINAL) $(patsubst %_$(FLASH_TYPE)_squashfs_fsimage,%,$@);\
	cp $(SIGN_IMAGE_PATH)/$(patsubst %_$(FLASH_TYPE)_squashfs_fsimage,%,$@).sqfs$(_FINAL) $(OUTPUT_SE_BT_IMAGE)/$(patsubst %_$(FLASH_TYPE)_squashfs_fsimage,%,$@).sqfs


%_$(FLASH_TYPE)_ext4_fsimage:
	@echo [[$@]]
	$(SECEREBOOT_PWD)/tools/deal_image.sh $(SIGN_IMAGE_PATH)/$(patsubst %_$(FLASH_TYPE)_ext4_fsimage,%,$@).ext4 $(_FINAL) $(patsubst %_$(FLASH_TYPE)_ext4_fsimage,%,$@);\
	cp $(SIGN_IMAGE_PATH)/$(patsubst %_$(FLASH_TYPE)_ext4_fsimage,%,$@).ext4$(_FINAL) $(OUTPUT_SE_BT_IMAGE)/$(patsubst %_$(FLASH_TYPE)_ext4_fsimage,%,$@).ext4


%_$(FLASH_TYPE)_jffs2_fsimage:
	@echo [[$@]]

%_$(FLASH_TYPE)_lfs_fsimage:
	@echo [[$@]]

%_$(FLASH_TYPE)_fwfs_fsimage:
	@echo [[$@]]

%_sboot_image_size:
	$(eval def_size=$($(patsubst %_sboot_image_size,%,$@)$(DATASIZE)))
	$(eval nbot_size=$(call alignup, $(call filesize, $(SIGN_IMAGE_PATH)/$(lastword $(subst /, ,$($(patsubst %_sboot_image_size,%,$@)$(RESOURCE))))), $(call get_alignment, $(patsubst %_sboot_image_size,%,$@))))
	$(eval sbot_size=$(call alignup, $(call filesize, $(SIGN_IMAGE_PATH)/$(lastword $(subst /, ,$($(patsubst %_sboot_image_size,%,$@)$(RESOURCE))))$(_FINAL)), $(call get_alignment, $(patsubst %_sboot_image_size,%,$@))))

	$(shell $(SECEREBOOT_PWD)/tools/deal_size.sh $(patsubst %_sboot_image_size,%,$@)$(DATASIZE) $(def_size) $(nbot_size) $(sbot_size) $(IMAGE_SIZE_CONFIG))
	@echo "--->def_size=$(def_size), nbot_size=$(nbot_size), sbot_size=$(sbot_size), $(patsubst %_sboot_image_size,%,$@)$(DATASIZE)= $($(patsubst %_sboot_image_size,%,$@)$(DATASIZE))"

-include $(IMAGE_SIZE_CONFIG)

cis_nofsimage:
	@if [ -e $(REBUILD_CIS) ]; then rm $(REBUILD_CIS);fi
	@if [ -e $(OUTPUT_SE_BT_IMAGE)/boot/partinfo.pni ]; then rm $(OUTPUT_SE_BT_IMAGE)/boot/partinfo.pni;fi
	$(SECEREBOOT_PWD)/tools/makefiletools/pnigenerator -u $(TARGET_UBIFS_SIZE) -c $(FLASH_BLK_CNT) -s $(FLASH_BLK_SIZE) -a "$(cis$(BOOTTAB0))" -b "$(cis$(BOOTTAB1))" -m "$(BOOT_IMAGE_LIST)" -t "$(cis$(SYSTAB))" -e "$(cis$(EXTTAB))" -o $(OUTPUT_SE_BT_IMAGE)/boot/partinfo.pni	\
			$(filter-out %=,$(strip $(foreach n, $(IMAGE_LIST), $(n)_bbm=$($(n)$(BBMCFG))))) > $(OUTPUT_SE_BT_IMAGE)/partition_layout.txt
	cat $(OUTPUT_SE_BT_IMAGE)/partition_layout.txt
	if [ "$(FLASH_TYPE)" = "spinand" ]; then	\
		$(SECEREBOOT_PWD)/tools/makefiletools/snigenerator -a "${BLK_PB0_OFFSET}" -b "${BLK_PB1_OFFSET}" -c "${BLPINB}" -d "${BAKCNT}" -e "${BAKOFS}" -p "$(shell printf "%d" $(FLASH_PG_SIZE))" -s "$(shell printf "%d" $(FLASH_SPARE_SIZE))" -t "${FLASH_PLANE_CNT}" -i $(SIGN_IMAGE_PATH)/flash.$(FLASH_INFO_SUFFIX) -o $(OUTPUT_SE_BT_IMAGE)/boot/flash.$(FLASH_INFO_SUFFIX);       \
		if [ "$(DUAL_OS)" = "on" ] || [ "$(PURE_RTOS)" = "on" ]; then	\
			$(SECEREBOOT_PWD)/tools/makefiletools/snigenerator -a "${BLK_PB0_OFFSET}" -b "${BLK_PB1_OFFSET}" -c "${BLPINB}" -d "${BAKCNT}" -e "${BAKOFS}" -i $(SIGN_IMAGE_PATH)/flash_list-fb.$(FLASH_INFO_SUFFIX) -o $(OUTPUT_SE_BT_IMAGE)/boot/flash_list-fb.$(FLASH_INFO_SUFFIX); \
		else \
			$(SECEREBOOT_PWD)/tools/makefiletools/snigenerator -a "${BLK_PB0_OFFSET}" -b "${BLK_PB1_OFFSET}" -c "${BLPINB}" -d "${BAKCNT}" -e "${BAKOFS}" -i $(SIGN_IMAGE_PATH)/flash_list.$(FLASH_INFO_SUFFIX) -o $(OUTPUT_SE_BT_IMAGE)/boot/flash_list.$(FLASH_INFO_SUFFIX); \
		fi;\
	fi;

	if [ "$(FLASH_TYPE)" = "nor" ]; then	\
		if [ "$(DUAL_OS)" = "on" ] || [ "$(PURE_RTOS)" = "on" ]; then	\
			$(SECEREBOOT_PWD)/tools/makefiletools/nrigenerator -i $(SIGN_IMAGE_PATH)/flash_list-fb.$(FLASH_INFO_SUFFIX) -o $(OUTPUT_SE_BT_IMAGE)/boot/flash_list-fb.$(FLASH_INFO_SUFFIX); \
		else \
			$(SECEREBOOT_PWD)/tools/makefiletools/nrigenerator -i $(SIGN_IMAGE_PATH)/flash_list.$(FLASH_INFO_SUFFIX) -o $(OUTPUT_SE_BT_IMAGE)/boot/flash_list.$(FLASH_INFO_SUFFIX); \
		fi; \
	fi;

	dd if=/dev/zero bs=$(shell printf "%d" $(FLASH_PG_SIZE)) count=2 | tr '\000' '\377' > $(REBUILD_CIS)
	dd if=$(OUTPUT_SE_BT_IMAGE)/boot/flash.$(FLASH_INFO_SUFFIX) of=$(REBUILD_CIS) bs=$(shell printf "%d" $(FLASH_PG_SIZE)) count=1 conv=notrunc seek=0
	dd if=$(OUTPUT_SE_BT_IMAGE)/boot/partinfo.pni of=$(REBUILD_CIS)  bs=$(shell printf "%d" $(FLASH_PG_SIZE)) count=1 conv=notrunc seek=1
ifeq (1,${IPL_IN_CIS})
	dd if=$(OUTPUT_SE_BT_IMAGE)/boot/IPL.bin of=$(REBUILD_CIS) bs=$(shell printf "%d" $(FLASH_PG_SIZE)) count=32 conv=notrunc seek=2 status=none
	if [ "$(DUAL_OS)" = "on" ] || [ "$(PURE_RTOS)" = "on" ]; then	\
		dd if=${OUTPUT_SE_BT_IMAGE}/boot/flash_list-fb.$(FLASH_INFO_SUFFIX) of=$(REBUILD_CIS) bs=$(shell printf "%d" $(FLASH_PG_SIZE)) conv=notrunc seek=34 status=none; \
	else \
		dd if=${OUTPUT_SE_BT_IMAGE}/boot/flash_list.$(FLASH_INFO_SUFFIX) of=$(REBUILD_CIS) bs=$(shell printf "%d" $(FLASH_PG_SIZE)) conv=notrunc seek=34 status=none; \
	fi;
else
	if [ "$(DUAL_OS)" = "on" ] || [ "$(PURE_RTOS)" = "on" ]; then	\
		dd if=/dev/zero bs=$(shell printf "%d" $(call alignup, $(call filesize, $(SIGN_IMAGE_PATH)/flash_list-fb.$(FLASH_INFO_SUFFIX)), $(FLASH_PG_SIZE))) count=1 | tr '\000' '\377' >> $(REBUILD_CIS); \
		dd if=$(OUTPUT_SE_BT_IMAGE)/boot/flash_list-fb.$(FLASH_INFO_SUFFIX) of=$(REBUILD_CIS) bs=$(shell printf "%d" $(FLASH_PG_SIZE)) conv=notrunc seek=2; \
	else \
		dd if=/dev/zero bs=$(shell printf "%d" $(call alignup, $(call filesize, $(SIGN_IMAGE_PATH)/flash_list.$(FLASH_INFO_SUFFIX)), $(FLASH_PG_SIZE))) count=1 | tr '\000' '\377' >> $(REBUILD_CIS); \
		dd if=$(OUTPUT_SE_BT_IMAGE)/boot/flash_list.$(FLASH_INFO_SUFFIX) of=$(REBUILD_CIS) bs=$(shell printf "%d" $(FLASH_PG_SIZE)) conv=notrunc seek=2; \
	fi;

ifeq (1,${ALL_FLASH_LIST})
	if [ "$(FLASH_TYPE)" = "nor" ]; then	\
		$(SECEREBOOT_PWD)/tools/makefiletools/nrigenerator -i $(SIGN_IMAGE_PATH)/flash_list.nri -o $(OUTPUT_SE_BT_IMAGE)/boot/flash_list.nri;     \
		cat $(OUTPUT_SE_BT_IMAGE)/boot/flash_list.nri >> $(REBUILD_CIS);   \
	fi;
	if [ "$(FLASH_TYPE)" = "spinand" ]; then	\
		if [ "$(DUAL_OS)" = "on" ]; then	\
			$(SECEREBOOT_PWD)/tools/makefiletools/snigenerator -i $(SIGN_IMAGE_PATH)/flash_list-fb.sni -o $(OUTPUT_SE_BT_IMAGE)/boot/flash_list-fb.sni;     \
			cat $(IMAGEDIR)/boot/flash_list-fb.sni >> $(REBUILD_CIS);    \
		else \
			$(SECEREBOOT_PWD)/tools/makefiletools/snigenerator -i $(SIGN_IMAGE_PATH)/flash_list.sni -o $(OUTPUT_SE_BT_IMAGE)/boot/flash_list.sni;     \
			cat $(OUTPUT_SE_BT_IMAGE)/boot/flash_list.sni >> $(REBUILD_CIS);    \
		fi;\
	fi;
endif
endif
	cp $(REBUILD_CIS) $(SIGN_IMAGE_PATH)/$(lastword $(subst /, ,$(cis$(RESOURCE))))$(_FINAL)

%_mkboot:
	@echo "[[$@]]"

	@if [ $(shell printf "%d" $($(patsubst %_mkboot,%,$@)$(DATASIZE))) -lt  $(shell printf "%d" $(call filesize, $(SIGN_IMAGE_PATH)/$(lastword $(subst /, ,$($(patsubst %_mkboot,%,$@)$(RESOURCE))))$(_FINAL))) ] ; then \
		echo "warning !!!!! $(patsubst %_mkboot,%,$@) resource file size is little than data size in config file,$(shell printf "%d" $(call alignup, $(call filesize, $(SIGN_IMAGE_PATH)/$(lastword $(subst /, ,$($(patsubst %_mkboot,%,$@)$(RESOURCE))))$(_FINAL)) , $(FLASH_PG_SIZE))) $(shell printf "%d" $(call filesize, $(SIGN_IMAGE_PATH)/$(lastword $(subst /, ,$($(patsubst %_mkboot,%,$@)$(RESOURCE))))$(_FINAL)))"; \
	fi;

	dd if=/dev/zero bs=$(shell printf "%d" $($(patsubst %_mkboot,%,$@)$(DATASIZE))) count=1 | tr '\000' '\377' > $(OUTPUT_SE_BT_IMAGE)/$@;
	dd if=$(SIGN_IMAGE_PATH)/$(lastword $(subst /, ,$($(patsubst %_mkboot,%,$@)$(RESOURCE))))$(_FINAL) of=$(OUTPUT_SE_BT_IMAGE)/$@ bs=$(shell printf "%d" $($(patsubst %_mkboot,%,$@)$(DATASIZE))) count=1 conv=notrunc seek=0;
	if [ "$($(patsubst %_mkboot,%,$@)$(COPIES))" != "" ]; then \
		for((Row=1;Row<$($(patsubst %_mkboot,%,$@)$(COPIES));Row++));do \
			dd if=$(OUTPUT_SE_BT_IMAGE)/$@ of=$(OUTPUT_SE_BT_IMAGE)/$@ bs=$(shell printf "%d" $($(patsubst %_mkboot,%,$@)$(DATASIZE))) count=1 conv=notrunc seek=$${Row}; \
		done; \
	fi;

get_number = $(firstword $(subst 0x,,$(filter 0x%,$(subst ' ',' ', $(1)))))
SCRIPTDIR:=$(OUTPUT_SE_BT_IMAGE)/scripts/
ifeq ($(UBOOTENV_SIGCMD_ENABLE),YES)
ifeq ($(DUAL_OS), on)
KERNEL_SIGN$(BOOTCMD)=sigauth \$${loados_addr} 0x21000000\;
RTOS_SIGN$(BOOTCMD)=sigauth \$${loados_addr} 0x21000000\;
VMM_SIGN$(BOOTCMD)=sigauth \$${loados_addr} 0x21000000\;
RAMDISK_SIGN$(BOOTCMD)=sigauth \$${loados_addr} 0x21000000\;
else
ifneq ($(kernel$(BOOTCMD)),)
KERNEL_SIGN$(BOOTCMD)="sigauth $(call get_number,$(kernel$(BOOTCMD))) 0x21000000;"
endif
endif
ifneq ($(rootfs$(BOOTCMD)),)
ROOTFS_SIGN$(BOOTCMD)="sigauth $(call get_number,$(rootfs$(BOOTCMD))) 0x21000000;"
endif
endif



do_env_config:
	@echo "[[$@]]"
	#@$(SECEREBOOT_PWD)/tools/sign_img.sh uboot_env $(SIGN_IMAGE_PATH)/aes_flag
	make $(FLASH_TYPE)_config_script
	make set_partition

	make boot_$(FLASH_TYPE)__script

ifneq ($(rootfs$(BOOTCMD)),)
	@sed -i '/saveenv/i setenv rootfs_size $(call filesize,$(shell ls -d $(OUTPUT_SE_BT_IMAGE)/rootfs*))' $(OUTPUT_SE_BT_IMAGE)/scripts/set_config
endif

-include $(SECEREBOOT_PWD)/include/script.mk
-include $(SECEREBOOT_PWD)/include/definitions.mk
$(call import_env_config,$(SECEREBOOT_PWD)/include/env.config)


emmc_upgrade_sigmastar:
	$(SECEREBOOT_PWD)/tools/makefiletools/make_$(patsubst %_upgrade_sigmastar,%,$@)_upgrade_sigmastar.sh

sd_upgrade_sigmastar:
	$(SECEREBOOT_PWD)/tools/makefiletools/make_$(patsubst %_upgrade_sigmastar,%,$@)_upgrade_sigmastar.sh $(SHELL_OPTIONS)

usb_upgrade_sigmastar:
	$(SECEREBOOT_PWD)/tools/makefiletools/make_$(patsubst %_upgrade_sigmastar,%,$@)_upgrade_sigmastar.sh

usb_factory_sigmastar_usage:
	@echo "usage: "
	@echo "    make usb_factory_sigmastar IPL=xxx UBOOT=xxx"
	@echo "support IPL:"
	@echo "    $(shell ls -1 $(SIGN_IMAGE_PATH)/usb/ipl)"
	@echo " "
	@echo "support UBOOT:"
	@echo "    $(shell ls -1 $(SIGN_IMAGE_PATH)/usb/uboot)"
	false

usb_boot:
	if [ ! -d "$(OUTPUT_SE_BT_IMAGE)/DebugUpgrade" ]; then mkdir -p $(OUTPUT_SE_BT_IMAGE)/DebugUpgrade; fi

	cp $(SIGN_IMAGE_PATH)/usb/ipl/$(IPL)$(_FINAL) $(OUTPUT_SE_BT_IMAGE)/DebugUpgrade/usb_updater.bin
	@if [  -f "$(SIGN_IMAGE_PATH)/usb/tf_a/u-bl31.bin" ]; then \
		cat $(SIGN_IMAGE_PATH)/usb/tf_a/u-bl31.bin$(_FINAL) $(SIGN_IMAGE_PATH)/usb/uboot/$(UBOOT)$(_FINAL) > $(OUTPUT_SE_BT_IMAGE)/DebugUpgrade/tf_a_uboot.bin ;\
		cp $(SIGN_IMAGE_PATH)/usb/tf_a/u-bl31.bin$(_FINAL) $(OUTPUT_SE_BT_IMAGE)/DebugUpgrade/tf_a.bin ;\
		cp $(SIGN_IMAGE_PATH)/usb/uboot/$(UBOOT)$(_FINAL) $(OUTPUT_SE_BT_IMAGE)/DebugUpgrade/u-boot.bin ;\
	else\
		cat $(SIGN_IMAGE_PATH)/usb/uboot/$(UBOOT)$(_FINAL) > $(OUTPUT_SE_BT_IMAGE)/DebugUpgrade/u-boot.bin;\
	fi;

usb_factory_sigmastar:
	@echo $(SIGN_IMAGE_PATH)/usb/ipl/$(IPL)
	@echo $(SIGN_IMAGE_PATH)/usb/uboot/$(UBOOT)
	@if [ ! -f "$(SIGN_IMAGE_PATH)/usb/ipl/$(IPL)" ]; then make usb_factory_sigmastar_usage; fi;
	@if [ ! -f "$(SIGN_IMAGE_PATH)/usb/uboot/$(UBOOT)" ]; then make usb_factory_sigmastar_usage; fi;

	$(SECEREBOOT_PWD)/tools/deal_image.sh $(SIGN_IMAGE_PATH)/usb/ipl/$(IPL) $(_FINAL) ipl usb
	$(SECEREBOOT_PWD)/tools/deal_image.sh $(SIGN_IMAGE_PATH)/usb/uboot/$(UBOOT) $(_FINAL) uboot usb
	@if [  -f "$(SIGN_IMAGE_PATH)/usb/tf_a/u-bl31.bin" ]; then \
		$(SECEREBOOT_PWD)/tools/deal_image.sh $(SIGN_IMAGE_PATH)/usb/tf_a/u-bl31.bin $(_FINAL) tf_a usb;\
	fi;

	$(SECEREBOOT_PWD)/tools/makefiletools/make_$(patsubst %_factory_sigmastar,%,$@)_factory_sigmastar.sh -i $(IPL)$(_FINAL) -u $(UBOOT)$(_FINAL) -s $(SHELL_OPTIONS)
	make usb_boot


FS_IMAGE:
	@echo "$(SE_TARGET_FSIMAGE)"
ifneq (,${SE_TARGET_FSIMAGE})
	make $(SE_TARGET_FSIMAGE)
endif

secure_boot:image_prepare
	$(SECEREBOOT_PWD)/tools/log.sh log_init
	make $(SECUREBOOT_IMAGE)
	make $(EXTRA_SIG_LIST)
	make FS_IMAGE
	@if [ -e $(IMAGE_SIZE_CONFIG) ]; then rm $(IMAGE_SIZE_CONFIG);fi
	make $(SBOOT_IMAGE_SIZE)
ifneq (,${FLASH_INFO_SUFFIX})
	make cis_nofsimage
endif
ifneq ($(FLASH_TYPE), sdmmc)
	make $(TARGET_BOOT_IMAGE)
	@echo "$(foreach n,$(TARGET_BOOT_IMAGE),$(OUTPUT_SE_BT_IMAGE)/$(n)) > $(OUTPUT_SE_BT_IMAGE)/boot.bin"
	cat $(foreach n,$(TARGET_BOOT_IMAGE),$(OUTPUT_SE_BT_IMAGE)/$(n)) > $(OUTPUT_SE_BT_IMAGE)/boot.bin
	@rm -rfv $(foreach n,$(TARGET_BOOT_IMAGE),$(OUTPUT_SE_BT_IMAGE)/$(n))
	@if [ $(FLASH_ECC_TYPE) ]; then	\
		$(SECEREBOOT_PWD)/tools/makefiletools/eccgenerator -i "$(ECCGENERATOR)" -o ${OUTPUT_SE_BT_IMAGE}/ -s ${OUTPUT_SE_BT_IMAGE}/boot/flash.sni -e $(FLASH_ECC_TYPE) > /dev/null; \
	fi;
endif
	make do_env_config
	$(SECEREBOOT_PWD)/tools/log.sh log_end
	@echo " "
	@echo "[[$@ done!]]"
	@echo " "

log_open:
	@sed -i 's/enable_log=0/enable_log=1/g' $(SECEREBOOT_PWD)/tools/key_proc.py
	@sed -i 's/enable_log=0/enable_log=1/g' $(SECEREBOOT_PWD)/tools/sign_img.sh
	@echo "[[$@ done!]]"

log_close:
	@sed -i 's/enable_log=1/enable_log=0/g' $(SECEREBOOT_PWD)/tools/key_proc.py
	@sed -i 's/enable_log=1/enable_log=0/g' $(SECEREBOOT_PWD)/tools/sign_img.sh
	@echo "[[$@ done!]]"

help:
	@echo "usage: "
	@echo "    make clean;make"
	@echo " "
	@echo "extra usage: "
	@echo "    [make image_prepare]            --> Prepare images and tools to sboot_sign"
	@echo "    [make secure_boot]              --> Sign, repack and set uboot env"
	@echo "    [make clean]                    --> Clean tmpfile"
	@echo "    [make log_open]                 --> Open sign image process log"
	@echo "    [make log_close]                --> Close sign image process log"
	@echo "    [make sd_upgrade_sigmastar]                    --> Generate sd_upgrade image"
	@echo "    [make usb_upgrade_sigmastar]                   --> Generate usb_upgrade image"
	@echo "    [make emmc_upgrade_sigmastar]                  --> Generate emmc_upgrade image"
	@echo "    [make usb_factory_sigmastar] IPL=xxx UBOOT=xxx --> Generate usb_factory image"
	@echo " "


clean :
	@if [ -d "$(IMAGEDIR)" ]; then\
		rm -rf $(SECEREBOOT_PWD)/include;\
		rm -rf $(OUTPUT_SE_BT_IMAGE);\
		rm -rf $(SIGN_IMAGE_PATH);\
		rm -rf $(SECEREBOOT_PWD)/tools/makefiletools/;\
		rm -rf $(SECEREBOOT_PWD)/tools/mk_fs;\
		rm -rf $(SECEREBOOT_PWD)/tools/rsaKey.*;\
		$(SECEREBOOT_PWD)/tools/log.sh log_rm \
	else\
		@echo "[[do nothing!]]";\
	fi;
	@echo "[[$@ done!]]"
