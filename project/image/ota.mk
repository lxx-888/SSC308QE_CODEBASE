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
TARGET_OTA_IMAGE:=$(foreach n,$(OTA_IMAGE_LIST),$(n)_$(FLASH_TYPE)_mkota_$($(n)$(FSTYPE)))

define makeota
@read -p "Make $(1) ?(yes/no)" select;	\
if [ "$${select}" == "yes" -o "$${select}" == "y" ]; then	\
	$(PROJ_ROOT)/image/makefiletools/bin/otapack -s $(2) -d "$(3)" -t $(4) --default-mask "$(7)" -m $(5) $(6) -l "$(1)" -a $(IMAGEDIR)/SStarOta.bin; \
fi;
endef

ota_images: ota_image_creat $(TARGET_OTA_IMAGE) ota_image_end ota_image_compressed

ota_image_creat:
ifneq ($(TARGET_OTA_IMAGE), )
	@read -p "Start scripts:" start_scripts;	\
	read -p "End scripts:" end_scripts;	\
	if [ "$${start_scripts}" != "" -a "$${end_scripts}" != "" ]; then	\
		$(PROJ_ROOT)/image/makefiletools/bin/otapack -c $(IMAGEDIR)/SStarOta.bin -b $${start_scripts} -e $${end_scripts}; \
	elif [ "$${start_scripts}" != "" -a "$${end_scripts}" == "" ]; then   \
		$(PROJ_ROOT)/image/makefiletools/bin/otapack -c $(IMAGEDIR)/SStarOta.bin -b $${start_scripts}; \
	elif [ "$${start_scripts}" == "" -a "$${end_scripts}" != "" ]; then   \
		$(PROJ_ROOT)/image/makefiletools/bin/otapack -c $(IMAGEDIR)/SStarOta.bin -e $${end_scripts}; \
	elif [ "$${start_scripts}" == "" -a "$${end_scripts}" == "" ]; then   \
		$(PROJ_ROOT)/image/makefiletools/bin/otapack -c $(IMAGEDIR)/SStarOta.bin; \
	fi;
endif

ota_image_compressed:
ifneq ($(TARGET_OTA_IMAGE), )
	@gzip $(IMAGEDIR)/SStarOta.bin;
endif

ota_image_end:
	@$(PROJ_ROOT)/image/makefiletools/bin/otapack --out-layout $(IMAGEDIR)/SStarOtaLayout.txt -r $(IMAGEDIR)/SStarOta.bin;


ipl_spinand_mkota_:
	$(call makeota,$(patsubst %_spinand_mkota_,%,$@),$(IMAGEDIR)/ipl_s.bin,$($(patsubst %_spinand_mkota_,%,$@)$(OTABLK)),$($(patsubst %_spinand_mkota_,%,$@)$(PATSIZE)),0,--block-update,$($(patsubst %_spinand_mkota_,%,$@)$(OTAMASK)))

ipl_cust_spinand_mkota_:
	$(call makeota,$(patsubst %_spinand_mkota_,%,$@),$(IMAGEDIR)/ipl_cust_s.bin,$($(patsubst %_spinand_mkota_,%,$@)$(OTABLK)),$($(patsubst %_spinand_mkota_,%,$@)$(PATSIZE)),0,--block-update,$($(patsubst %_spinand_mkota_,%,$@)$(OTAMASK)))

uboot_spinand_mkota_:
	$(call makeota,$(patsubst %_spinand_mkota_,%,$@),$(IMAGEDIR)/uboot_s.bin,$($(patsubst %_spinand_mkota_,%,$@)$(OTABLK)),$($(patsubst %_spinand_mkota_,%,$@)$(PATSIZE)),0,--block-update,$($(patsubst %_spinand_mkota_,%,$@)$(OTAMASK)))

boot_$(FLASH_TYPE)_mkota_:
	$(call makeota,$(patsubst %_$(FLASH_TYPE)_mkota_,%,$@),$(IMAGEDIR)/boot.bin,$($(patsubst %_$(FLASH_TYPE)_mkota_,%,$@)$(OTABLK)),$($(patsubst %_$(FLASH_TYPE)_mkota_,%,$@)$(PATSIZE)),0,--block-update,$($(patsubst %_$(FLASH_TYPE)_mkota_,%,$@)$(OTAMASK)))

logo_$(FLASH_TYPE)_mkota_:
	$(call makeota,$(patsubst %_$(FLASH_TYPE)_mkota_,%,$@),$(IMAGEDIR)/logo,$($(patsubst %_$(FLASH_TYPE)_mkota_,%,$@)$(OTABLK)),$($(patsubst %_$(FLASH_TYPE)_mkota_,%,$@)$(PATSIZE)),0,--block-update,$($(patsubst %_$(FLASH_TYPE)_mkota_,%,$@)$(OTAMASK)))

kernel_$(FLASH_TYPE)_mkota_:
	$(call makeota,$(patsubst %_$(FLASH_TYPE)_mkota_,%,$@),$(IMAGEDIR)/kernel,$($(patsubst %_$(FLASH_TYPE)_mkota_,%,$@)$(OTABLK)),$($(patsubst %_$(FLASH_TYPE)_mkota_,%,$@)$(PATSIZE)),0,--block-update,$($(patsubst %_$(FLASH_TYPE)_mkota_,%,$@)$(OTAMASK)))
ifeq ($(FLASH_TYPE),spinand)
	@echo -e "kernel_file_size $$(printf %x $$(stat -c "%s" $(IMAGEDIR)/kernel))" | sed 's/^/\/etc\/fw_setenv /g' > $(IMAGEDIR)/kernel_otaenv.sh;
	@echo -e "recovery_file_size  $$(printf %x $$(stat -c "%s" $(IMAGEDIR)/kernel))" | sed 's/^/\/etc\/fw_setenv /g' >> $(IMAGEDIR)/kernel_otaenv.sh;
	@$(PROJ_ROOT)/image/makefiletools/bin/otapack -s $(IMAGEDIR)/kernel_otaenv.sh -d kernel_otaenv.sh -l kernel_otaenv.sh --script-add -a $(IMAGEDIR)/SStarOta.bin;
endif

%_$(FLASH_TYPE)_mkota_squashfs:
	$(call makeota,$(patsubst %_$(FLASH_TYPE)_mkota_squashfs,%,$@),$(IMAGEDIR)/$(patsubst %_$(FLASH_TYPE)_mkota_squashfs,%,$@).sqfs,$($(patsubst %_$(FLASH_TYPE)_mkota_squashfs,%,$@)$(OTABLK)),$($(patsubst %_$(FLASH_TYPE)_mkota_squashfs,%,$@)$(PATSIZE)),0,--block-update,$($(patsubst %_$(FLASH_TYPE)_mkota_squashfs,%,$@)$(OTAMASK)))
%_$(FLASH_TYPE)_mkota_jffs2:
	$(call makeota,$(patsubst %_$(FLASH_TYPE)_mkota_jffs2,%,$@),$(IMAGEDIR)/$(patsubst %_$(FLASH_TYPE)_mkota_jffs2,%,$@).jffs2,$($(patsubst %_$(FLASH_TYPE)_mkota_jffs2,%,$@)$(OTABLK)),$($(patsubst %_$(FLASH_TYPE)_mkota_jffs2,%,$@)$(PATSIZE)),0,--block-update,$($(patsubst %_$(FLASH_TYPE)_mkota_jffs2,%,$@)$(OTAMASK)))
%_$(FLASH_TYPE)_mkota_ubifs:
	$(call makeota,$(patsubst %_$(FLASH_TYPE)_mkota_ubifs,%,$@),$(IMAGEDIR)/$(patsubst %_$(FLASH_TYPE)_mkota_ubifs,%,$@).ubifs,$($(patsubst %_$(FLASH_TYPE)_mkota_ubifs,%,$@)$(OTABLK)),$($(patsubst %_$(FLASH_TYPE)_mkota_ubifs,%,$@)$(PATSIZE)),0,--ubi-update,$($(patsubst %_$(FLASH_TYPE)_mkota_ubifs,%,$@)$(OTAMASK)))
%_$(FLASH_TYPE)_mkota_lfs:
	$(call makeota,$(patsubst %_$(FLASH_TYPE)_mkota_lfs,%,$@),$(IMAGEDIR)/$(patsubst %_$(FLASH_TYPE)_mkota_lfs,%,$@).lfs,$($(patsubst %_$(FLASH_TYPE)_mkota_lfs,%,$@)$(OTABLK)),$($(patsubst %_$(FLASH_TYPE)_mkota_lfs,%,$@)$(PATSIZE)),0,--block-update,$($(patsubst %_$(FLASH_TYPE)_mkota_lfs,%,$@)$(OTAMASK)))
%_$(FLASH_TYPE)_mkota_fwfs:
	$(call makeota,$(patsubst %_$(FLASH_TYPE)_mkota_fwfs,%,$@),$(IMAGEDIR)/$(patsubst %_$(FLASH_TYPE)_mkota_fwfs,%,$@).fwfs,$($(patsubst %_$(FLASH_TYPE)_mkota_fwfs,%,$@)$(OTABLK)),$($(patsubst %_$(FLASH_TYPE)_mkota_fwfs,%,$@)$(PATSIZE)),0,--block-update,$($(patsubst %_$(FLASH_TYPE)_mkota_fwfs,%,$@)$(OTAMASK)))
%_$(FLASH_TYPE)_mkota_ext4:
	$(call makeota,$(patsubst %_$(FLASH_TYPE)_mkota_ext4,%,$@),$(IMAGEDIR)/$(patsubst %_$(FLASH_TYPE)_mkota_ext4,%,$@).ext4,$($(patsubst %_$(FLASH_TYPE)_mkota_ext4,%,$@)$(OTABLK)),$($(patsubst %_$(FLASH_TYPE)_mkota_ext4,%,$@)$(PATSIZE)),0,--block-update,$($(patsubst %_$(FLASH_TYPE)_mkota_ext4,%,$@)$(OTAMASK)))
