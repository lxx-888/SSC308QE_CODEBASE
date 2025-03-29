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

include definitions.mk
include script.mk
include image.mk
include ota.mk

define getval
$(shell if [ -n "$(2)" ]; then echo $(2); else echo packaging/common/$(1); fi;)
endef

PARTS_TARGETS:=rootfs_recovery
ifeq ($(FLASH_TYPE), sdmmc)
PARTS_TARGETS += $(filter-out rootfs ramdisk, $(TARGET_SYS_IMAGE))
else
PARTS_TARGETS += $(filter-out rootfs ramdisk, $(FSIMAGE_LIST))
endif

ifneq ($(GLOBAL_MK),)
include configs/$(GLOBAL_MK)
else
include $(call getval,rootfs.mk,$(rootfs$(CUSMK)))
-include $(call getval,ramdisk.mk,$(ramdisk$(CUSMK)))
-include $(foreach n,$(PARTS_TARGETS),$(call getval,$(n).mk,$($(n)$(CUSMK))))
endif

$(call import_env_config,configs/general/env.config)

usb_storage_images:
	cd $(PROJ_ROOT);$(PROJ_ROOT)/image/makefiletools/script/make_usb_upgrade_sigmastar.sh -$(MODE);cd -;

usb_ufu_images:
	cp $(PROJ_ROOT)/../boot/$(USB_UPGRADE_UBOOT_CONFIG) $(PROJ_ROOT)/board/$(CHIP)/boot/usb/upgrade/uboot
	cd $(PROJ_ROOT);$(PROJ_ROOT)/image/makefiletools/script/make_usb_factory_sigmastar.sh -i $(USB_UPGRADE_IPL_CONFIG) -u $(USB_UPGRADE_UBOOT_CONFIG) -f -n USB_UPGRADE_UFU.bin;cd -;
	mkdir -p $(PROJ_ROOT)/image/output/images/Usb_Ufu_Upgrade
	mv $(PROJ_ROOT)/image/output/images/USB_UPGRADE_UFU.bin $(PROJ_ROOT)/image/output/images/Usb_Ufu_Upgrade/
	cp -r -n $(PROJ_ROOT)/tools/usb_factory_tool/exe $(PROJ_ROOT)/image/output/images/Usb_Ufu_Upgrade/

sd_images:
	cd $(PROJ_ROOT);$(PROJ_ROOT)/image/makefiletools/script/make_sd_upgrade_sigmastar.sh;cd -;
	mkdir -p $(PROJ_ROOT)/image/output/images/Sd_Upgrade
	mv $(PROJ_ROOT)/image/output/images/SigmastarUpgradeSD.bin $(PROJ_ROOT)/image/output/images/Sd_Upgrade/
	cp $(PROJ_ROOT)/board/$(CHIP)/boot/sdmmc/ipl/$(SD_UPGRADE_IPL_CONFIG) $(PROJ_ROOT)/image/output/images/Sd_Upgrade/
	mv $(PROJ_ROOT)/image/output/images/Sd_Upgrade/$(SD_UPGRADE_IPL_CONFIG) $(PROJ_ROOT)/image/output/images/Sd_Upgrade/IPL
	cp $(PROJ_ROOT)/board/$(CHIP)/boot/sdmmc/ipl/$(SD_UPGRADE_IPL_CUST_CONFIG) $(PROJ_ROOT)/image/output/images/Sd_Upgrade/
	mv $(PROJ_ROOT)/image/output/images/Sd_Upgrade/$(SD_UPGRADE_IPL_CUST_CONFIG) $(PROJ_ROOT)/image/output/images/Sd_Upgrade/IPL_CUST
	cp $(PROJ_ROOT)/../boot/$(USB_UPGRADE_UBOOT_CONFIG) $(PROJ_ROOT)/image/output/images/Sd_Upgrade/
	mv $(PROJ_ROOT)/image/output/images/Sd_Upgrade/$(USB_UPGRADE_UBOOT_CONFIG)  $(PROJ_ROOT)/image/output/images/Sd_Upgrade/UBOOT

emmc_images:
	cd $(PROJ_ROOT);$(PROJ_ROOT)/image/makefiletools/script/make_emmc_upgrade_sigmastar.sh -$(MODE);cd -;

PERF:=$(patsubst %,tar xf build/%.tar.gz -C $(OUTPUTDIR)/rootfs,$(PERF_NAME))
STAP:=$(patsubst %,tar xf build/%.tar.gz -C $(OUTPUTDIR)/rootfs,$(STAP_NAME))

# Fake Object to be occupied
$(PARTS_TARGETS):

CONFIG_JSON_ENABLE_MODULES := fb disp

CONFIG_JSON_MODULES:=$(patsubst %_enable_,%,$(filter-out %__, $(filter-out %_disable_, $(foreach n,$(CONFIG_JSON_ENABLE_MODULES),$(n)_$(interface_$(n))_))))

CONFIG_JSON_MODULES_NUM=$(call calc_val_size,$(CONFIG_JSON_MODULES))

OUTPUT_JSON:=$(PROJ_ROOT)/board/$(CHIP)/$(BOARD_NAME)/config/config.json


gen_json:
	@echo OUTPUT_JSON=$(OUTPUT_JSON)
	@echo CONFIG_JSON_MODULES=$(CONFIG_JSON_MODULES)
	@echo CONFIG_JSON_MODULES_NUM=$(CONFIG_JSON_MODULES_NUM)
	@rm -rf $(OUTPUT_JSON)
	@if [ $(CONFIG_JSON_MODULES_NUM) -ne 0 ]; then \
		echo { > $(OUTPUT_JSON);\
		i=0;for module in $(CONFIG_JSON_MODULES); do\
			cat $(PROJ_ROOT)/board/$(CHIP)/$(BOARD_NAME)/config/config_$$module.json >> $(OUTPUT_JSON);\
			i=$$[$$i+1];\
			if [ $$i -ne $(CONFIG_JSON_MODULES_NUM) ]; then \
				echo , >> $(OUTPUT_JSON);\
			fi;\
		done;\
		echo } >> $(OUTPUT_JSON); \
	fi;

	if [ -f $(OUTPUT_JSON) ]; then \
		cat $(OUTPUT_JSON) | $(PYTHON) -c "import json, sys; dump=json.dumps(json.load(sys.stdin), indent=4);pfh=open('$(OUTPUT_JSON)','w'); pfh.write(dump); pfh.close()"; \
	fi;

prepare:
	@mkdir -p $(OUTPUTDIR)
	@mkdir -p $(IMAGEDIR)
	@mkdir -p $(IMAGEDIR)/boot
ifneq ($(USR_MOUNT_BLOCKS),)
	@mkdir -p $(foreach n,$(USR_MOUNT_BLOCKS),$($(n)$(RESOURCE)))
endif
	@make gen_json
ifeq ($(DUAL_OS), on)
	@cp -rvf $(PROJ_ROOT)/board/rtos/rtos.elf $(IMAGEDIR)/
endif

MAP_SUBDIR:=ko so

map_install:
	@mkdir -p $(foreach n,$(MAP_SUBDIR),$(OUTPUTDIR)/map/$(n))
	@for prex in $(MAP_SUBDIR); do { for mapfile in `find $(CURDIR)/../../sdk -name "*_$$prex.map"`; do { cp -f $$mapfile $(OUTPUTDIR)/map/$$prex; } done; } done

define parser_loadaddr
	if [ -f $(1) ]; then \
		$(PYTHON) $(PROJ_ROOT)/scripts/parser_loadaddr.py --infile $(1) --out $(IMAGEDIR)/load_addr.txt; \
	fi;
endef

load_addr_check:
	@$(call parser_loadaddr,$(IMAGEDIR)/kernel)
	@$(call parser_loadaddr,$(IMAGEDIR)/earlyinit)
	@$(call parser_loadaddr,$(IMAGEDIR)/ramdisk.gz)
ifneq ($(FLASH_TYPE), emmc)
	@$(call parser_loadaddr,$(ipl_cust$(RESOURCE)))
else
	@$(call parser_loadaddr,$(iplx$(RESOURCE)))
endif
	@$(call parser_loadaddr,$(uboot$(RESOURCE)))
	@$(call parser_loadaddr,$(IMAGEDIR)/rtos)
	@$(call parser_loadaddr,$(IMAGEDIR)/boot/$(VMM_CONFIG))
	@$(call parser_loadaddr,$(IMAGEDIR)/boot/$(OPTEE_CONFIG))
	@$(call parser_loadaddr,$(IMAGEDIR)/boot/$(TFA_CONFIG))
	@$(PYTHON) $(PROJ_ROOT)/scripts/loadaddr_layout_graph.py $(IMAGEDIR)/load_addr.txt $(IMAGEDIR)/load_addr.svg
	@echo "Gen the layout graph of image load addr ["$(IMAGEDIR)/load_addr.svg"] over"
ifneq ($(findstring purertos,$(PRODUCT)), )
	@$(PYTHON) $(PROJ_ROOT)/scripts/check_loadaddr.py --lxaddr $(CONFIG_MMA_HEAP_SIZE) --layout $(IMAGEDIR)/load_addr.txt
else
	@$(PYTHON) $(PROJ_ROOT)/scripts/check_loadaddr.py --lxaddr $(LX_MEM_SIZE) --layout $(IMAGEDIR)/load_addr.txt
endif
	@echo "check load addr end!"

all: prepare
ifeq ($(findstring purertos,$(PRODUCT)), )
	$(MAKE) rootfs
	$(MAKE) ramdisk
endif
	$(MAKE) $(PARTS_TARGETS)
	$(MAKE) images
	$(MAKE) scripts
	$(MAKE) map_install
	if [ $(FPGA) != 1  ]; then \
	    if [ $(PZ1) != 1  ]; then \
		$(MAKE) load_addr_check; \
	    fi; \
	fi;
clean:
	rm -rf $(OUTPUTDIR)

