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
FSIMAGE_LIST:=$(filter-out $(patsubst %_fs__,%,$(filter %_fs__, $(foreach n,$(IMAGE_LIST),$(n)_fs_$($(n)$(FSTYPE))_))),$(IMAGE_LIST))
TARGET_FSIMAGE:=$(foreach n,$(FSIMAGE_LIST),$(n)_$(FLASH_TYPE)_$($(n)$(FSTYPE))_fsimage)
TARGET_NOFSIMAGE:=$(foreach n,$(filter-out $(FSIMAGE_LIST), $(IMAGE_LIST)),$(n)_nofsimage)

BOOT_FSIMAGE_LIST:=$(filter-out $(patsubst %_fs__,%,$(filter %_fs__, $(foreach n,$(BOOT_IMAGE_LIST),$(n)_fs_$($(n)$(FSTYPE))_))),$(BOOT_IMAGE_LIST))
BOOT_TARGET_FSIMAGE:=$(foreach n,$(BOOT_FSIMAGE_LIST),$(n)_$(FLASH_TYPE)_$($(n)$(FSTYPE))_fsimage)
BOOT_TARGET_NOFSIMAGE:=$(foreach n,$(filter-out $(BOOT_FSIMAGE_LIST), $(BOOT_IMAGE_LIST)),$(n)_nofsimage)

# flash default settings
ifeq ($(FLASH_TYPE), nor)
FLASH_INFO_SUFFIX = nri
FLASH_BLK_SIZE ?= 0x10000
FLASH_BLK_CNT ?= 256
FLASH_PG_SIZE ?= 0x1000
else
ifeq ($(FLASH_TYPE), spinand)
FLASH_INFO_SUFFIX = sni
FLASH_BLK_SIZE ?= 0x20000
FLASH_BLK_CNT ?= 1024
FLASH_PG_SIZE ?= 0x800
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

define image_size_check
	if [ ! -f $(1) ]; then \
		echo -e "\e[1;32m$(notdir $(1)) not exist!\e[0m";  \
	else \
		cmp0=`stat -c "%s" $(1)`; \
		dst_size=`printf 0x%x $${cmp0}`; \
		cmp1=`printf "%d" $(2)`; \
		if [ $${cmp0} -gt $${cmp1} ]; then \
			echo -e "\e[1;31mError $(notdir $(1)) is too large! Please check! [$${dst_size}]>= THD:[$(2)] \e[0m";  \
			exit -1; \
		else \
			echo -e "\e[1;32mCheck $(notdir $(1)) size pass!  DST:[$${dst_size}] THD:[$(2)] \e[0m";  \
		fi; \
	fi;
endef

#1: load address
#2: header name
#3: bin name
define image_compress_process
	$(PROJ_ROOT)/kbuild/$(KERNEL_VERSION)/scripts/mkimage -A arm -O linux -C none -a $(1) -e $(1) -n $(2) -d $(IMAGEDIR)/$(3) $(IMAGEDIR)/u-$(3); \
	$(PROJ_ROOT)/kbuild/$(KERNEL_VERSION)/scripts/sstar_sz.sh -d $(IMAGEDIR)/u-$(3) -b 4; \
	$(PROJ_ROOT)/kbuild/$(KERNEL_VERSION)/scripts/mkimage -A arm -O linux -C lzma2 -a $(shell awk 'BEGIN{printf("%#x", '${1}-0x40')}') -e 0 -n $(2) -d $(IMAGEDIR)/u-$(3).sz $(IMAGEDIR)/$(3); \
	rm -rvf $(IMAGEDIR)/u-$(3) $(IMAGEDIR)/u-$(3).sz
endef

images: images_check

images_check: $(TARGET_FSIMAGE) $(TARGET_NOFSIMAGE)
	@$(call image_size_check,$(IMAGEDIR)/kernel,$(kernel$(PATSIZE)))
	@$(call image_size_check,$(boot$(RESOURCE)),$(boot$(PATSIZE)))
	@$(foreach i,$(FSIMAGE_LIST),$(call image_size_check,$(IMAGEDIR)/$(i).$(patsubst squashfs,sqfs,$($(i)$(FSTYPE))),$($(i)$(PATSIZE))))
	@if [ "$(RTOS_ENABLE)" == "on" ]; then \
		$(call image_size_check,$(rtos$(RESOURCE)),$(rtos$(PATSIZE))) \
	fi;
	@echo "image patsize check end!"

%_cponly_fsimage:
	@echo [[$@]]

%_ext4_fsimage:
	@echo [[$@]]
	make_ext4fs -S ./build/file_contexts -l $($(patsubst %_ext4_fsimage,%,$@)$(PATSIZE)) -b 1024 $(IMAGEDIR)/$(patsubst %_ext4_fsimage,%,$@).img $($(patsubst %_ext4_fsimage,%,$@)$(RESOURCE))

%_$(FLASH_TYPE)_ext4_fsimage:
	@echo [[$@]]
	mke2fs -d $($(patsubst %_$(FLASH_TYPE)_ext4_fsimage,%,$@)$(RESOURCE)) -t ext4 $(IMAGEDIR)/$(patsubst %_$(FLASH_TYPE)_ext4_fsimage,%,$@).ext4 $$(($($(patsubst %_$(FLASH_TYPE)_ext4_fsimage,%,$@)$(DATASIZE))/1024/1024))M

%_$(FLASH_TYPE)_squashfs_fsimage:
	@echo [[$@]]
	mksquashfs $($(patsubst %_$(FLASH_TYPE)_squashfs_fsimage,%,$@)$(RESOURCE)) $(IMAGEDIR)/$(patsubst %_$(FLASH_TYPE)_squashfs_fsimage,%,$@).sqfs -comp $($(patsubst %_$(FLASH_TYPE)_squashfs_fsimage,%,$@)$(OPTIONS)) -all-root;
	@if [ "$($(patsubst %_$(FLASH_TYPE)_squashfs_fsimage,%,$@)$(UBIVOLID))" != "" ]; then	\
		echo -e $(foreach n,$(patsubst %_$(FLASH_TYPE)_squashfs_fsimage,%,$@),[$(n)]\\nmode=ubi\\nimage=$(IMAGEDIR)/$(n).sqfs\\nvol_id=$($(n)$(UBIVOLID))\\nvol_type=dynamic\\nvol_name=$(n)\\nvol_alignment=1\\n) \
		>> $(IMAGEDIR)/ubinize$(patsubst %_$(FLASH_TYPE)_squashfs_fsimage,%,$@).cfg;	\
		ubinize -p $(FLASH_BLK_SIZE) -m $(FLASH_PG_SIZE) -o $(IMAGEDIR)/$(patsubst %_$(FLASH_TYPE)_squashfs_fsimage,%,$@).ubi.bin $(IMAGEDIR)/ubinize$(patsubst %_$(FLASH_TYPE)_squashfs_fsimage,%,$@).cfg;	\
	fi;

%_$(FLASH_TYPE)_jffs2_fsimage:
	@echo [[$@]]
	mkfs.jffs2  $($(patsubst %_$(FLASH_TYPE)_jffs2_fsimage,%,$@)$(PATSIZE)) --pad=$($(patsubst %_$(FLASH_TYPE)_jffs2_fsimage,%,$@)$(PATSIZE)) --eraseblock=$(FLASH_BLK_SIZE) -d $($(patsubst %_$(FLASH_TYPE)_jffs2_fsimage,%,$@)$(RESOURCE)) -o $(IMAGEDIR)/$(patsubst %_$(FLASH_TYPE)_jffs2_fsimage,%,$@).jffs2

%_$(FLASH_TYPE)_ubifs_fsimage:
	@echo [[$@]]
	mkfs.ubifs -F -r $($(patsubst %_$(FLASH_TYPE)_ubifs_fsimage,%,$@)$(RESOURCE)) -o $(IMAGEDIR)/$(patsubst %_$(FLASH_TYPE)_ubifs_fsimage,%,$@).ubifs -m $(FLASH_PG_SIZE) -e $(shell expr $(shell printf %d $(FLASH_BLK_SIZE)) - 2 \* $(shell printf %d $(FLASH_PG_SIZE))) -c `./build/calc_nand_mfs.sh $(patsubst %_$(FLASH_TYPE)_ubifs_fsimage,%,$@) $(FLASH_PG_SIZE) $(FLASH_BLK_SIZE) 0 $($(patsubst %_$(FLASH_TYPE)_ubifs_fsimage,%,$@)$(PATSIZE))`

%_$(FLASH_TYPE)_lwfs_fsimage:
	@echo [[$@]]
	$(PYTHON) $(PROJ_ROOT)/image/makefiletools/script/lwfs_pack.py $($(patsubst %_$(FLASH_TYPE)_lwfs_fsimage,%,$@)$(RESOURCE)) $(IMAGEDIR)/$(patsubst %_$(FLASH_TYPE)_lwfs_fsimage,%,$@).lwfs -a 64

%_$(FLASH_TYPE)_lfs_fsimage: ./build/mklittlefs
	@echo [[$@]]
	$(PYTHON) $(PROJ_ROOT)/image/makefiletools/script/lfs_pack.py --part_size=$($(patsubst %_$(FLASH_TYPE)_lfs_fsimage,%,$@)$(PATSIZE)) $($(patsubst %_$(FLASH_TYPE)_lfs_fsimage,%,$@)$(RESOURCE)) $(IMAGEDIR)/$(patsubst %_$(FLASH_TYPE)_lfs_fsimage,%,$@).lfs

%_$(FLASH_TYPE)_fwfs_fsimage: ./build/mkfwfs
	@echo [[$@]]
	$(PYTHON) $(PROJ_ROOT)/image/makefiletools/script/fwfs_pack.py --flash_type=$(FLASH_TYPE) --block_size=$(FLASH_BLK_SIZE) --page_size=$(FLASH_PG_SIZE) --part_size=$($(patsubst %_$(FLASH_TYPE)_fwfs_fsimage,%,$@)$(PATSIZE)) $($(patsubst %_$(FLASH_TYPE)_fwfs_fsimage,%,$@)$(RESOURCE)) $(IMAGEDIR)/$(patsubst %_$(FLASH_TYPE)_fwfs_fsimage,%,$@).fwfs
	@if [ "$($(patsubst %_$(FLASH_TYPE)_fwfs_fsimage,%,$@)$(OPTIONS))" = "sz" ]; then	\
		$(call image_compress_process,$(RTOS_RAMDISK_LOAD_ADDR),"misc.fwfs",$(patsubst %_$(FLASH_TYPE)_fwfs_fsimage,%,$@).fwfs); \
	fi;

%_$(FLASH_TYPE)_ramfs_fsimage:
	@echo [[$@]]
	cd $(PROJ_ROOT)/kbuild/$(KERNEL_VERSION);./usr/gen_initramfs.sh -o $(IMAGEDIR)/$(patsubst %_$(FLASH_TYPE)_ramfs_fsimage,%,$@).$($(patsubst %_$(FLASH_TYPE)_ramfs_fsimage,%,$@)$(FSTYPE)) -u 0 -g 0 $(OUTPUTDIR)/rootfs/
	@if [ "$($(patsubst %_$(FLASH_TYPE)_ramfs_fsimage,%,$@)$(OPTIONS))" = "sz" ]; then	\
		$(call image_compress_process,$(INITRAMFSLOADADDR),"ramfs",$(patsubst %_$(FLASH_TYPE)_ramfs_fsimage,%,$@).$($(patsubst %_$(FLASH_TYPE)_ramfs_fsimage,%,$@)$(FSTYPE))); \
	fi;

%_$(FLASH_TYPE)_gz_fsimage:
	@echo [[$@]]
	@cd $(PROJ_ROOT)/kbuild/$(KERNEL_VERSION) && ./usr/gen_initramfs.sh -o $(IMAGEDIR)/ramdisk.img -u 0 -g 0 $(OUTPUTDIR)/tmprd/;
	@if [ "$($(patsubst %_$(FLASH_TYPE)_gz_fsimage,%,$@)$(OPTIONS))" = "sz" ]; then	\
		cp -vf $(IMAGEDIR)/ramdisk.img $(IMAGEDIR)/ramdisk.gz; \
		$(call image_compress_process,$(INITRAMFSLOADADDR),"ramdisk",ramdisk.gz); \
	else \
		$(PROJ_ROOT)/kbuild/$(KERNEL_VERSION)/scripts/mkimage -A arm -O linux -C none -a $(INITRAMFSLOADADDR) -e $(INITRAMFSLOADADDR) -n "ramdisk" -d $(IMAGEDIR)/ramdisk.img $(IMAGEDIR)/ramdisk.gz; \
	fi;

define makebin
if [ $(1) != "0" ]; then \
	dd if=/dev/zero bs=$(1) count=1 | tr '\000' '\377' > $(2)_tmp;	\
	dd if=$(3) of=$(2)_tmp bs=$(1) count=1 conv=notrunc seek=0;	\
	for Row in {1..$(4)};do	\
		dd if=$(2)_tmp of=$(2) bs=$(1) count=1 conv=notrunc oflag=append;	\
	done;	\
	rm -rf $(2)_tmp;	\
fi;
endef

TARGET_BOOT_IMAGE:=$(foreach n,$(BOOT_IMAGE_LIST),$(n)_mkboot)
BOOT_IMAGES_PATH:=$(foreach n,$(TARGET_BOOT_IMAGE),$(if $(wildcard $(IMAGEDIR)/$(n)), $(IMAGEDIR)/$(n)))

boot_bin:$(BOOT_IMAGES_PATH)
	@echo "$@"
	cat $^ > $(boot$(RESOURCE))

boot_images:
	$(MAKE) $(TARGET_BOOT_IMAGE)
	$(MAKE) boot_bin
	rm -rfv $(foreach n,$(TARGET_BOOT_IMAGE),$(IMAGEDIR)/$(n))
	@if [ $(FLASH_ECC_TYPE) ]; then	\
	$(PROJ_ROOT)/image/makefiletools/bin/eccgenerator -i "$(ECCGENERATOR)" -o ${IMAGEDIR}/ -s ${IMAGEDIR}/boot/flash.sni -e $(FLASH_ECC_TYPE) > /dev/null; \
	fi;

%_mkboot:
	@if [ $(shell printf "%d" $($(patsubst %_mkboot,%,$@)$(DATASIZE))) -lt $(shell printf "%d" $(call filesize, $($(patsubst %_mkboot,%,$@)$(RESOURCE)))) ] ; then \
		echo "ERROR !!!!! $(patsubst %_mkboot,%,$@) resource file size is little than data size in config file"; \
		exit 1;  \
	fi;
	if [ $(shell printf "%d" $($(patsubst %_mkboot,%,$@)$(DATASIZE))) -ne 0 ]; then \
		dd if=/dev/zero bs=$(shell printf "%d" $($(patsubst %_mkboot,%,$@)$(DATASIZE))) count=1 | tr '\000' '\377' > $(IMAGEDIR)/$@; \
	fi;
	if [ -f "$($(patsubst %_mkboot,%,$@)$(RESOURCE))" ]; then \
		dd if=$($(patsubst %_mkboot,%,$@)$(RESOURCE)) of=$(IMAGEDIR)/$@ bs=$(shell printf "%d" $($(patsubst %_mkboot,%,$@)$(DATASIZE))) count=1 conv=notrunc seek=0; \
	else \
		echo "$($(patsubst %_mkboot,%,$@)$(RESOURCE)) is not a file, skip"; \
	fi;
	if [ "$($(patsubst %_mkboot,%,$@)$(COPIES))" != "" ]; then \
		for((Row=1;Row<$($(patsubst %_mkboot,%,$@)$(COPIES));Row++));do \
			dd if=$(IMAGEDIR)/$@ of=$(IMAGEDIR)/$@ bs=$(shell printf "%d" $($(patsubst %_mkboot,%,$@)$(DATASIZE))) count=1 conv=notrunc seek=$${Row}; \
		done; \
	fi;

ubi%_nofsimage: $(TARGET_FSIMAGE)
	@echo [[$@]];
	@if [ "$(filter $@_volid_,$(foreach n,$(TARGET_UBIFS),$($(n)$(OPTIONS))_nofsimage_$(patsubst %,volid,$($(n)$(UBIVOLID)))_))" != "" ]; then	\
		echo -e $(foreach n,$(foreach p,$(filter %_$@,$(foreach m,$(TARGET_UBIFS),$(m)_$($(m)$(OPTIONS))_nofsimage)),$(patsubst %_$@,%,$(p))),[$(n)]\\nmode=ubi\\nimage=$(IMAGEDIR)/$(n).ubifs\\nvol_id=$($(n)$(UBIVOLID))\\nvol_size=$($(n)$(PATSIZE))\\nvol_type=dynamic\\nvol_name=$(n)\\nvol_alignment=1\\n) \
		>> $(IMAGEDIR)/ubinize$(patsubst ubi%_nofsimage,%,$@).cfg;	\
		ubinize -p $(FLASH_BLK_SIZE) -m $(FLASH_PG_SIZE) -o $(IMAGEDIR)/ubi$(patsubst ubi%_nofsimage,%,$@).bin $(IMAGEDIR)/ubinize$(patsubst ubi%_nofsimage,%,$@).cfg;	\
	fi;


cis_nofsimage:
	@echo [[$@]]
	cp -vf $(PROJ_ROOT)/board/$(CHIP)/boot/$(FLASH_TYPE)/partition/* $(IMAGEDIR)/boot/
ifeq (1,${IPL_IN_CIS})
	cp -f $(PROJ_ROOT)/board/$(CHIP)/boot/$(FLASH_TYPE)/ipl/IPL.bin $(IMAGEDIR)/boot/
endif
	$(PROJ_ROOT)/image/makefiletools/bin/pnigenerator -u $(TARGET_UBIFS_SIZE) -c $(FLASH_BLK_CNT) -s $(FLASH_BLK_SIZE) -a "$(cis$(BOOTTAB0))" -b "$(cis$(BOOTTAB1))" -m "$(BOOT_IMAGE_LIST)" -t "$(cis$(SYSTAB))" -e "$(cis$(EXTTAB))" -o $(IMAGEDIR)/boot/partinfo.pni	\
			$(filter-out %=,$(strip $(foreach n, $(IMAGE_LIST), $(n)_bbm=$($(n)$(BBMCFG))))) > $(IMAGEDIR)/partition_layout.txt
	cat $(IMAGEDIR)/partition_layout.txt
	if [ "$(FLASH_TYPE)" = "spinand" ]; then	\
		$(PROJ_ROOT)/image/makefiletools/bin/snigenerator -a "${BLK_PB0_OFFSET}" -b "${BLK_PB1_OFFSET}" -c "${BLPINB}" -d "${BAKCNT}" -e "${BAKOFS}" -p "$(shell printf "%d" $(FLASH_PG_SIZE))" -s "$(shell printf "%d" $(FLASH_SPARE_SIZE))" -t "${FLASH_PLANE_CNT}" -i $(PROJ_ROOT)/board/$(CHIP)/boot/$(FLASH_TYPE)/partition/flash.$(FLASH_INFO_SUFFIX) -o $(IMAGEDIR)/boot/flash.$(FLASH_INFO_SUFFIX);       \
		if [ "$(DUAL_OS)" = "on" ] || [ "$(PURE_RTOS)" = "on" ]; then	\
			$(PROJ_ROOT)/image/makefiletools/bin/snigenerator -a "${BLK_PB0_OFFSET}" -b "${BLK_PB1_OFFSET}" -c "${BLPINB}" -d "${BAKCNT}" -e "${BAKOFS}" -i $(PROJ_ROOT)/board/$(CHIP)/boot/$(FLASH_TYPE)/partition/flash_list-fb.$(FLASH_INFO_SUFFIX) -o $(IMAGEDIR)/boot/flash_list-fb.$(FLASH_INFO_SUFFIX); \
		else \
			$(PROJ_ROOT)/image/makefiletools/bin/snigenerator -a "${BLK_PB0_OFFSET}" -b "${BLK_PB1_OFFSET}" -c "${BLPINB}" -d "${BAKCNT}" -e "${BAKOFS}" -i $(PROJ_ROOT)/board/$(CHIP)/boot/$(FLASH_TYPE)/partition/flash_list.$(FLASH_INFO_SUFFIX) -o $(IMAGEDIR)/boot/flash_list.$(FLASH_INFO_SUFFIX); \
		fi;\
	fi;

	if [ "$(FLASH_TYPE)" = "nor" ]; then	\
		if [ "$(DUAL_OS)" = "on" ] || [ "$(PURE_RTOS)" = "on" ]; then	\
			$(PROJ_ROOT)/image/makefiletools/bin/nrigenerator -i $(PROJ_ROOT)/board/$(CHIP)/boot/$(FLASH_TYPE)/partition/flash_list-fb.$(FLASH_INFO_SUFFIX) -o $(IMAGEDIR)/boot/flash_list-fb.$(FLASH_INFO_SUFFIX); \
		else \
			$(PROJ_ROOT)/image/makefiletools/bin/nrigenerator -i $(PROJ_ROOT)/board/$(CHIP)/boot/$(FLASH_TYPE)/partition/flash_list.$(FLASH_INFO_SUFFIX) -o $(IMAGEDIR)/boot/flash_list.$(FLASH_INFO_SUFFIX); \
		fi; \
	fi;

	dd if=/dev/zero bs=$(shell printf "%d" $(FLASH_PG_SIZE)) count=2 | tr '\000' '\377' > $(cis$(RESOURCE))
	dd if=$(IMAGEDIR)/boot/flash.$(FLASH_INFO_SUFFIX) of=$(cis$(RESOURCE)) bs=$(shell printf "%d" $(FLASH_PG_SIZE)) count=1 conv=notrunc seek=0
	dd if=$(IMAGEDIR)/boot/partinfo.pni of=$(cis$(RESOURCE)) bs=$(shell printf "%d" $(FLASH_PG_SIZE)) count=1 conv=notrunc seek=1
ifeq (1,${IPL_IN_CIS})
	dd if=$(IMAGEDIR)/boot/IPL.bin of=$(cis$(RESOURCE)) bs=$(shell printf "%d" $(FLASH_PG_SIZE)) count=32 conv=notrunc seek=2 status=none
	if [ "$(DUAL_OS)" = "on" ] || [ "$(PURE_RTOS)" = "on" ]; then	\
		dd if=${IMAGEDIR}/boot/flash_list-fb.$(FLASH_INFO_SUFFIX) of=$(cis$(RESOURCE)) bs=$(shell printf "%d" $(FLASH_PG_SIZE)) conv=notrunc seek=34 status=none; \
	else \
		dd if=${IMAGEDIR}/boot/flash_list.$(FLASH_INFO_SUFFIX) of=$(cis$(RESOURCE)) bs=$(shell printf "%d" $(FLASH_PG_SIZE)) conv=notrunc seek=34 status=none; \
	fi;
else
	if [ "$(DUAL_OS)" = "on" ] || [ "$(PURE_RTOS)" = "on" ]; then	\
		dd if=/dev/zero bs=$(shell printf "%d" $(call alignup, $(call filesize, $(PROJ_ROOT)/board/$(CHIP)/boot/$(FLASH_TYPE)/partition/flash_list-fb.$(FLASH_INFO_SUFFIX)), $(FLASH_PG_SIZE))) count=1 | tr '\000' '\377' >> $(cis$(RESOURCE)); \
		dd if=$(IMAGEDIR)/boot/flash_list-fb.$(FLASH_INFO_SUFFIX) of=$(cis$(RESOURCE)) bs=$(shell printf "%d" $(FLASH_PG_SIZE)) conv=notrunc seek=2; \
	else \
		dd if=/dev/zero bs=$(shell printf "%d" $(call alignup, $(call filesize, $(PROJ_ROOT)/board/$(CHIP)/boot/$(FLASH_TYPE)/partition/flash_list.$(FLASH_INFO_SUFFIX)), $(FLASH_PG_SIZE))) count=1 | tr '\000' '\377' >> $(cis$(RESOURCE)); \
		dd if=$(IMAGEDIR)/boot/flash_list.$(FLASH_INFO_SUFFIX) of=$(cis$(RESOURCE)) bs=$(shell printf "%d" $(FLASH_PG_SIZE)) conv=notrunc seek=2; \
	fi;

ifeq (1,${ALL_FLASH_LIST})
	if [ "$(FLASH_TYPE)" = "nor" ]; then	\
		$(PROJ_ROOT)/image/makefiletools/bin/nrigenerator -i $(PROJ_ROOT)/board/$(CHIP)/boot/nor/partition/flash_list.nri -o $(IMAGEDIR)/boot/flash_list.nri;     \
		cat $(IMAGEDIR)/boot/flash_list.nri >> $(cis$(RESOURCE));    \
	fi;
	if [ "$(FLASH_TYPE)" = "spinand" ]; then	\
		if [ "$(DUAL_OS)" = "on" ] || [ "$(PURE_RTOS)" = "on" ]; then	\
			$(PROJ_ROOT)/image/makefiletools/bin/snigenerator -i $(PROJ_ROOT)/board/$(CHIP)/boot/spinand/partition/flash_list-fb.sni -o $(IMAGEDIR)/boot/flash_list-fb.sni;     \
			cat $(IMAGEDIR)/boot/flash_list-fb.sni >> $(cis$(RESOURCE));    \
		else \
			$(PROJ_ROOT)/image/makefiletools/bin/snigenerator -i $(PROJ_ROOT)/board/$(CHIP)/boot/spinand/partition/flash_list.sni -o $(IMAGEDIR)/boot/flash_list.sni;     \
			cat $(IMAGEDIR)/boot/flash_list.sni >> $(cis$(RESOURCE));    \
		fi;\
	fi;
endif
endif

key_cust_nofsimage:
	@echo [[$@]]

boot_nofsimage: $(BOOT_TARGET_NOFSIMAGE) $(BOOT_TARGET_FSIMAGE)
	@echo [[$@]]
	$(MAKE) boot_images

logo_nofsimage:
	@echo [[$@]]
	$(PROJ_ROOT)/image/makefiletools/bin/dispcfggen -c -o $(logo$(RESOURCE)) -p $(LOGO_ADDR) -s $(BOOTLOGO_BUFSIZE) -d $(DISP_OUT_NAME)
	$(PROJ_ROOT)/image/makefiletools/bin/logogen -a -i $(PROJ_ROOT)/board/ini/misc/$(BOOTLOGO_FILE) -o $(logo$(RESOURCE))
ifneq ($(UPGRADE_FILE),)
	$(PROJ_ROOT)/image/makefiletools/bin/logogen -a -i $(PROJ_ROOT)/board/ini/misc/$(UPGRADE_FILE) -o $(logo$(RESOURCE))
else
	$(PROJ_ROOT)/image/makefiletools/bin/logogen -a -i $(PROJ_ROOT)/board/ini/misc/upgrade.jpg -o $(logo$(RESOURCE))
endif

ipl_nofsimage ipl_cust_nofsimage iplx_nofsimage uboot_nofsimage freertos_nofsimage optee_nofsimage tf_a_nofsimage:
	@echo [[$@]]
	if [ -f "$($(patsubst %_nofsimage,%,$@)$(RESOURCE))" ]; then \
		cp -vf $($(patsubst %_nofsimage,%,$@)$(RESOURCE)) $(IMAGEDIR)/boot/; \
	else \
		echo -e "$($(patsubst %_nofsimage,%,$@)$(RESOURCE)) is not a file"; \
		@exit 1;\
	fi;

vmm_nofsimage:
	@echo [[$@]]
	@echo "vmm suffix is $(suffix $($(patsubst %_nofsimage,%,$@)$(RESOURCE)))"
	rm -rf $(dir $($(patsubst %_nofsimage,%,$@)$(RESOURCE)))/vmm.bin; \
	rm -rf $(dir $($(patsubst %_nofsimage,%,$@)$(RESOURCE)))/vmm.bin.sz; \
	rm -rf $(dir $($(patsubst %_nofsimage,%,$@)$(RESOURCE)))/vmm.sz; \
	$(dir $($(patsubst %_nofsimage,%,$@)$(RESOURCE)))/image_gen/mkimage_vmm -A arm -O sigmastar-hyp -C none -a $(VMM_LOAD_ADDR) -e $(VMM_LOAD_ADDR) -n sigmastar-hyp -d $(dir $($(patsubst %_nofsimage,%,$@)$(RESOURCE)))/vmm.raw $(dir $($(patsubst %_nofsimage,%,$@)$(RESOURCE)))/vmm.bin; \
	$(PROJ_ROOT)/kbuild/$(KERNEL_VERSION)/scripts/sstar_sz.sh -d $(dir $($(patsubst %_nofsimage,%,$@)$(RESOURCE)))/vmm.bin -b 4; \
	$(dir $($(patsubst %_nofsimage,%,$@)$(RESOURCE)))/image_gen/mkimage_vmm -A arm -O sigmastar-hyp -C lzma2 -a $(shell awk 'BEGIN{printf("%#x", '${VMM_LOAD_ADDR}-0x40')}') -e 0 -n sigmastar-hyp-sz -d $(dir $($(patsubst %_nofsimage,%,$@)$(RESOURCE)))/vmm.bin.sz $(dir $($(patsubst %_nofsimage,%,$@)$(RESOURCE)))/vmm.sz; \
	rm -rf $(dir $($(patsubst %_nofsimage,%,$@)$(RESOURCE)))/vmm.bin.sz; \
	if [ -f "$($(patsubst %_nofsimage,%,$@)$(RESOURCE))" ]; then \
		cp -vf $($(patsubst %_nofsimage,%,$@)$(RESOURCE)) $(IMAGEDIR)/boot/; \
	else \
		echo -e "$($(patsubst %_nofsimage,%,$@)$(RESOURCE)) is not a file"; \
		@exit 1;\
	fi;

rtos_nofsimage kernel_nofsimage dtb_nofsimage dtbo_nofsimage earlyinit_nofsimage pm_rtos_nofsimage pm51_nofsimage riscvfw_nofsimage:
	@echo [[$@]]
	cp -rvf $($(patsubst %_nofsimage,%,$@)$(RESOURCE)) $(IMAGEDIR)/$(patsubst %_nofsimage,%,$@)
	if [ "$($(patsubst %_nofsimage,%,$@)$(OPTIONS))" = "ssz" ]; then	\
		cp -rvf $($(patsubst %_nofsimage,%,$@)$(AES_TYPE)$(SIG_TYPE)$(RESOURCE)).$($(patsubst %_nofsimage,%,$@)$(OPTIONS)) $(IMAGEDIR)/$(patsubst %_nofsimage,%,$@).$($(patsubst %_nofsimage,%,$@)$(OPTIONS));	\
		rm -rvf $(IMAGEDIR)/$(patsubst %_nofsimage,%,$@);	\
		mv $(IMAGEDIR)/$(patsubst %_nofsimage,%,$@).$($(patsubst %_nofsimage,%,$@)$(OPTIONS)) $(IMAGEDIR)/$(patsubst %_nofsimage,%,$@);	\
	fi;

# sdmmc boot
TARGET_SYS_IMAGE       = $(filter-out $(patsubst %_fs__,%,$(filter %_fs__, $(foreach n,$(system$(SYSTAB)),$(n)_fs_$($(n)$(FSTYPE))_))),$(system$(SYSTAB)))
TARGET_SYS_FSIMAGE     = $(foreach n,$(TARGET_SYS_IMAGE),$(n)_$(FLASH_TYPE)_$($(n)$(FSTYPE))_fsimage)
TARGET_SYS_NOFSIMAGE   = $(foreach n,$(filter-out $(TARGET_SYS_IMAGE), $(system$(SYSTAB))),$(n)_nofsimage)

fat_nofsimage:
	$(MAKE) $(foreach n,$(fat$(SYSTAB)),$(n)_loadfile)

system_nofsimage:
	$(MAKE) $(TARGET_SYS_FSIMAGE)
	$(MAKE) $(TARGET_SYS_NOFSIMAGE)

%_loadfile:
	cp -vf $($(patsubst %_loadfile,%,$@)$(RESOURCE)) $(IMAGEDIR)/boot/$(shell echo $(patsubst %_loadfile,%,$@) |  tr '[a-z]' '[A-Z]')
