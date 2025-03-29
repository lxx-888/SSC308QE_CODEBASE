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
DOWNLOADADDR?=0x21000000
KERNELBOOTADDR?=0x22000000
DTBBOOTADDR?=0x26D00000
INITRAMFSLOADADDR?=0x21800000
SPLIT_EACH_FILE_SIZE?=0x2000000

ifeq ($(BURN_CODE_ON_FREERTOS), on)
ifeq ($(FLASH_TYPE), nor)
FLASH_INIT = ""
FLASH_PROBE = ""
FLASH_WRITE = "sf write"
FLASH_ERASE_PART = "sf erase"
FLASH_WRITE_PART = "sf write"
FLASH_WRITE_CONTINUE = "sf write"
FLASH_READ = "sf read"
else
ifeq ($(FLASH_TYPE), spinand)
FLASH_INIT = ""
FLASH_PROBE = ""
FLASH_WRITE = "nand write"
FLASH_ERASE_PART = "nand erase"
FLASH_WRITE_PART = "nand write"
FLASH_WRITE_CONTINUE = "nand write"
FLASH_READ = "nand read"
endif
endif
else
ifeq ($(FLASH_TYPE), nor)
ifeq ($(DUAL_OS), on)
FLASH_INIT = "$(LOAD_CMD) $(DOWNLOADADDR) boot/flash_list-fb.nri\\nsf probe $(DOWNLOADADDR)"
else ifeq ($(PURE_RTOS), on)
FLASH_INIT = "$(LOAD_CMD) $(DOWNLOADADDR) boot/flash_list-fb.nri\\nsf probe $(DOWNLOADADDR)"
else
FLASH_INIT = "$(LOAD_CMD) $(DOWNLOADADDR) boot/flash_list.nri\\nsf probe $(DOWNLOADADDR)"
endif
FLASH_PROBE = "sf probe 0"
FLASH_WRITE = "sf write"
FLASH_ERASE_PART = "sf erase"
FLASH_WRITE_PART = "sf write"
FLASH_WRITE_CONTINUE = "sf write"
FLASH_READ = "sf read"
else
ifeq ($(FLASH_TYPE), spinand)
ifeq ($(DUAL_OS), on)
FLASH_INIT = "$(LOAD_CMD) $(DOWNLOADADDR) boot/flash_list-fb.sni\\nnand probe $(DOWNLOADADDR)"
else ifeq ($(PURE_RTOS), on)
FLASH_INIT = "$(LOAD_CMD) $(DOWNLOADADDR) boot/flash_list-fb.sni\\nnand probe $(DOWNLOADADDR)"
else
FLASH_INIT = "$(LOAD_CMD) $(DOWNLOADADDR) boot/flash_list.sni\\nnand probe $(DOWNLOADADDR)"
endif
FLASH_PROBE = ""
FLASH_WRITE = "nand write"
FLASH_ERASE_PART = "nand erase.part"
FLASH_WRITE_PART = "nand write.e"
FLASH_WRITE_CONTINUE = "nand write.continue"
FLASH_READ = "nand read.e"
else
ifeq ($(FLASH_TYPE), emmc)
FLASH_INIT = "mmc bootbus 0 2 0 0\\nemmc_wrrel on\\nemmc rmgpt\\n$(foreach n,$(TARGET_USERPART),$(foreach m,$($(n)$(PATNAME)),emmc create $(m) $($(n)$(PATSIZE))\\n))"
FLASH_PROBE = ""
FLASH_WRITE = "emmc write"
FLASH_ERASE_PART = "emmc erase.p"
FLASH_WRITE_PART = "emmc write.p"
FLASH_WRITE_CONTINUE = "emmc write.p.continue"
FLASH_READ = "emmc read.e"
else
ifeq ($(FLASH_TYPE), sdmmc)
FLASH_INIT = "mmc rescan\\nfdisk -d 0\\n$(foreach n,$(SYSTEM_PART_LIST),fdisk -c 0 $($(n)$(CNT))\\n)"
FLASH_PROBE = ""
FLASH_WRITE = "fatwrite mmc 0:1"
FLASH_ERASE_PART = "fdisk -e 0:"
FLASH_WRITE_PART = "fdisk -w 0:"
FLASH_READ = "fdisk -r 0:"
endif
endif
endif
endif
endif

ifneq ($(UPGRADE_TYPE), sd)
STAR_CMD = "estar"
LOAD_CMD = "tftp"
FILE_TYPE = "es"
else
STAR_CMD = "dstar -i $(SD_UPGRADE_DEV) -f"
LOAD_CMD = "fatload mmc $(SD_UPGRADE_DEV)"
FILE_TYPE = "ds"
endif

define docuscmd
	if [ -n $(1) ]; then	\
		echo -e $(1) >> $(2);	\
	fi;
endef

define doflashwrite
	echo $(FLASH_PROBE) >> $(SCRIPTDIR)/$(2); \
	if [ -n "$($(3)$(PATNAME))" ]; then \
		for i in $($(3)$(PATNAME));do \
			echo $(FLASH_ERASE_PART) $$i >> $(SCRIPTDIR)/$(2); \
		done; \
	else \
		echo $(FLASH_ERASE_PART) $(3);  \
		echo $(FLASH_ERASE_PART) $(3) >> $(SCRIPTDIR)/$(2); \
	fi; \
	if [ -e $(IMAGEDIR)/$(1) ]; then \
		SPLIT_SIZE="`printf %d $(SPLIT_EACH_FILE_SIZE)`"; \
		IMAGE_SIZE="`stat --format=%s $(IMAGEDIR)/$(1)`"; \
		if [ $$IMAGE_SIZE -gt $$SPLIT_SIZE ]; then \
			echo -e "\e[1;32m $(3) too large, do split! \e[0m"; \
			offset=0; \
			split -b $$SPLIT_SIZE $(IMAGEDIR)/$(1) $(IMAGEDIR)/$(1)_; \
			for i in `ls $(IMAGEDIR)/ | grep $(1)_`; do \
				offset_16=0x`echo "obase=16;$$offset"|bc`; \
				echo $(LOAD_CMD) $(DOWNLOADADDR) $$i >> $(SCRIPTDIR)/$(2); \
				if [ -n "$($(3)$(PATNAME))" ]; then \
					for j in $($(3)$(PATNAME));do \
						echo $(FLASH_WRITE_CONTINUE) $(DOWNLOADADDR) $$j \$${filesize} $$offset_16 >> $(SCRIPTDIR)/$(2); \
					done; \
				else \
					echo $(FLASH_WRITE_CONTINUE) $(DOWNLOADADDR) $(3) \$${filesize} $$offset_16 >> $(SCRIPTDIR)/$(2); \
				fi; \
				offset=$$(($$offset + $$SPLIT_SIZE)); \
			done; \
		else \
			echo $(LOAD_CMD) $(DOWNLOADADDR) $(1) >> $(SCRIPTDIR)/$(2);	\
			if [ -n "$($(3)$(PATNAME))" ]; then \
				for i in $($(3)$(PATNAME));do \
					echo $(FLASH_WRITE_PART) $(DOWNLOADADDR) $$i \$${filesize} >> $(SCRIPTDIR)/$(2); \
				done; \
			else \
				echo $(FLASH_WRITE_PART) $(DOWNLOADADDR) $(3) \$${filesize} >> $(SCRIPTDIR)/$(2); \
			fi; \
		fi; \
	fi;
endef

TARGET_SCRIPT:=$(foreach n,$(IMAGE_LIST),$(n)_$(FLASH_TYPE)_$($(n)$(FSTYPE))_script) $(FLASH_TYPE)_config_script
TARGET_FS:=$(filter-out $(patsubst %_fs__,%,$(filter %_fs__, $(foreach n,$(IMAGE_LIST),$(n)_fs_$($(n)$(FSTYPE))_))), $(IMAGE_LIST))
TARGET_UBIFS := $(patsubst %_fs_ubifs_, %, $(filter %_fs_ubifs_, $(foreach n,$(TARGET_FS),$(n)_fs_$($(n)$(FSTYPE))_)))
TARGET_SQUAFS := $(patsubst %_fs_squashfs_, %,$(filter %_fs_squashfs_, $(foreach n,$(TARGET_FS),$(n)_fs_$($(n)$(FSTYPE))_)))
TARGET_RAMFS := $(patsubst %_fs_ramfs_, %,$(filter %_fs_ramfs_, $(foreach n,$(TARGET_FS),$(n)_fs_$($(n)$(FSTYPE))_)))
TARGET_JIFFS2 := $(patsubst %_fs_jffs2_, %, $(filter %_fs_jffs2_, $(foreach n,$(TARGET_FS),$(n)_fs_$($(n)$(FSTYPE))_)))
TARGET_NONEFS := $(filter-out $(TARGET_FS), $(filter-out $(patsubst %_fs__sz__, %, $(filter %_fs__sz__, $(foreach n,$(IMAGE_LIST),$(n)_fs_$($(n)$(FSTYPE))_sz_$($(n)$(PATSIZE))_))), $(IMAGE_LIST)))
TARGET_USERPART := $(filter-out boot, $(IMAGE_LIST))
ifeq ($(PRODUCT), android)
TARGET_USERPART += env
endif
SCRIPTDIR?=$(IMAGEDIR)/scripts

TARGET_OUT_SCRIPT:=%_$(FLASH_TYPE)_ubifs_script
TARGET_OUT_IMG:=$(TARGET_UBIFS)
TARGET_UPDATE_SCRIPT:=$(filter-out $(TARGET_OUT_SCRIPT),$(TARGET_SCRIPT))
TARGET_UPDATE_LIST:=$(filter-out $(TARGET_OUT_IMG),$(IMAGE_LIST))

scripts:
	mkdir -p $(SCRIPTDIR)
	$(MAKE) set_partition
	$(MAKE) $(TARGET_UPDATE_SCRIPT)
ifeq ($(DUAL_OS), on)
ifeq ($(BOOL_FPGA), y)
	$(MAKE) rtos_tftp_script
endif
endif
	@echo "# <- this is for comment / total file size must be less than 4KB" > $(IMAGEDIR)/auto_update.txt
	@echo $(STAR_CMD) scripts/[[set_partition.$(FILE_TYPE) >> $(IMAGEDIR)/auto_update.txt
	@$(foreach n,$(TARGET_UPDATE_LIST),echo "$(STAR_CMD) scripts/[[$(n).$(FILE_TYPE)" >> $(IMAGEDIR)/auto_update.txt;)
	@echo $(STAR_CMD) scripts/set_config >> $(IMAGEDIR)/auto_update.txt
	@echo saveenv >> $(IMAGEDIR)/auto_update.txt
	@echo printenv >> $(IMAGEDIR)/auto_update.txt
	@echo reset >> $(IMAGEDIR)/auto_update.txt
	@echo "% <- this is end of file symbol" >> $(IMAGEDIR)/auto_update.txt
ifeq ($(FLASH_TYPE), spinand)
	@if [ $(FLASH_ECC_TYPE) ]; then	\
		echo cis.ecc.bin : $(shell printf 0x%X $(shell expr 0 \* $(shell printf %d $(FLASH_BLK_SIZE)))) > $(IMAGEDIR)/OnebinnandBurnImgConfig_ECC.cfg; \
		echo cis.ecc.bin : $(shell printf 0x%X $(shell expr 1 \* $(shell printf %d $(FLASH_BLK_SIZE)))) >> $(IMAGEDIR)/OnebinnandBurnImgConfig_ECC.cfg; \
		echo cis.ecc.bin : $(shell printf 0x%X $(shell expr 2 \* $(shell printf %d $(FLASH_BLK_SIZE)))) >> $(IMAGEDIR)/OnebinnandBurnImgConfig_ECC.cfg; \
		echo cis.ecc.bin : $(shell printf 0x%X $(shell expr 3 \* $(shell printf %d $(FLASH_BLK_SIZE)))) >> $(IMAGEDIR)/OnebinnandBurnImgConfig_ECC.cfg; \
		echo cis.ecc.bin : $(shell printf 0x%X $(shell expr 4 \* $(shell printf %d $(FLASH_BLK_SIZE)))) >> $(IMAGEDIR)/OnebinnandBurnImgConfig_ECC.cfg; \
		echo boot.ecc.bin : $(shell printf 0x%X $(shell expr 10 \* $(shell printf %d $(FLASH_BLK_SIZE)))) >> $(IMAGEDIR)/OnebinnandBurnImgConfig_ECC.cfg; \
	else \
		echo cis.bin : $(shell printf 0x%X $(shell expr 0 \* $(shell printf %d $(FLASH_BLK_SIZE)))) > $(IMAGEDIR)/OnebinnandBurnImgConfig.cfg; \
		echo cis.bin : $(shell printf 0x%X $(shell expr 1 \* $(shell printf %d $(FLASH_BLK_SIZE)))) >> $(IMAGEDIR)/OnebinnandBurnImgConfig.cfg; \
		echo cis.bin : $(shell printf 0x%X $(shell expr 2 \* $(shell printf %d $(FLASH_BLK_SIZE)))) >> $(IMAGEDIR)/OnebinnandBurnImgConfig.cfg; \
		echo cis.bin : $(shell printf 0x%X $(shell expr 3 \* $(shell printf %d $(FLASH_BLK_SIZE)))) >> $(IMAGEDIR)/OnebinnandBurnImgConfig.cfg; \
		echo cis.bin : $(shell printf 0x%X $(shell expr 4 \* $(shell printf %d $(FLASH_BLK_SIZE)))) >> $(IMAGEDIR)/OnebinnandBurnImgConfig.cfg; \
		echo cis.bin : $(shell printf 0x%X $(shell expr 5 \* $(shell printf %d $(FLASH_BLK_SIZE)))) >> $(IMAGEDIR)/OnebinnandBurnImgConfig.cfg; \
		echo cis.bin : $(shell printf 0x%X $(shell expr 6 \* $(shell printf %d $(FLASH_BLK_SIZE)))) >> $(IMAGEDIR)/OnebinnandBurnImgConfig.cfg; \
		echo cis.bin : $(shell printf 0x%X $(shell expr 7 \* $(shell printf %d $(FLASH_BLK_SIZE)))) >> $(IMAGEDIR)/OnebinnandBurnImgConfig.cfg; \
		echo cis.bin : $(shell printf 0x%X $(shell expr 8 \* $(shell printf %d $(FLASH_BLK_SIZE)))) >> $(IMAGEDIR)/OnebinnandBurnImgConfig.cfg; \
		echo cis.bin : $(shell printf 0x%X $(shell expr 9 \* $(shell printf %d $(FLASH_BLK_SIZE)))) >> $(IMAGEDIR)/OnebinnandBurnImgConfig.cfg; \
		echo boot.bin : $(shell printf 0x%X $(shell expr 10 \* $(shell printf %d $(FLASH_BLK_SIZE)))) >> $(IMAGEDIR)/OnebinnandBurnImgConfig.cfg; \
	fi;
endif
	@$(call check_env_consistency,$(IMAGEDIR)/scripts)

set_partition:
	@echo "# <- this is for comment / total file size must be less than 4KB" > $(SCRIPTDIR)/[[set_partition.$(FILE_TYPE)
ifneq ($(BURN_CODE_ON_FREERTOS), on)
	@echo -e $(FLASH_INIT) >> $(SCRIPTDIR)/[[set_partition.$(FILE_TYPE)
ifeq ($(FLASH_TYPE), sdmmc)
	@PART_TOTAL_CNT=0; \
	for i in $(foreach n,$(SYSTEM_PART_LIST),$($(n)$(CNT))); do \
		PART_TOTAL_CNT=$$(($$PART_TOTAL_CNT+$$i)); \
	done; \
	if [ $$PART_TOTAL_CNT -gt $$(($(FLASH_BLK_CNT))) ]; then \
		echo -e "\033[41;37m Warning: partition takes capacity($$PART_TOTAL_CNT) is greater than total capacity($(FLASH_BLK_CNT)) \033[0m"; \
	fi;
	@echo -e mmc rescan >> $(SCRIPTDIR)/[[set_partition.$(FILE_TYPE)
endif

ifeq ($(EMMC_SYSTEM_TYPE), double)
	@$(call gen_setenv_cmd,recargs,$(SCRIPTDIR)/[[set_partition.$(FILE_TYPE))
endif

ifneq ($(filter $(FLASH_TYPE),nor spinand),)
	@$(call gen_setenv_cmd,mtdids,$(SCRIPTDIR)/[[set_partition.$(FILE_TYPE))
	@$(call gen_setenv_cmd,mtdparts,$(SCRIPTDIR)/[[set_partition.$(FILE_TYPE))
endif

ifeq ($(USE_AB_SYSTEM_BOOT), y)
	@$(call gen_setenv_cmd,slot_metadata,$(SCRIPTDIR)/[[set_partition.$(FILE_TYPE))
	@$(call gen_setenv_cmd,slot_number,$(SCRIPTDIR)/[[set_partition.$(FILE_TYPE))
endif

	@echo saveenv >> $(SCRIPTDIR)/[[set_partition.$(FILE_TYPE)
endif
	@echo "% <- this is end of file symbol" >> $(SCRIPTDIR)/[[set_partition.$(FILE_TYPE)


ubi%_$(FLASH_TYPE)__script:
	@echo "# <- this is for comment / total file size must be less than 4KB" > $(SCRIPTDIR)/[[$(patsubst %_$(FLASH_TYPE)__script,%,$@).$(FILE_TYPE);
	@$(call doflashwrite,ubi$(patsubst ubi%_$(FLASH_TYPE)__script,%,$@).bin,[[$(patsubst %_$(FLASH_TYPE)__script,%,$@).$(FILE_TYPE),$(patsubst %_$(FLASH_TYPE)__script,%,$@))
	$(foreach b,$(foreach n,$(filter %_$(patsubst %_$(FLASH_TYPE)__script,%,$@), $(foreach m,$(TARGET_UBIFS),$(m)_$($(m)$(OPTIONS)))),$(patsubst %_$(patsubst %_$(FLASH_TYPE)__script,%,$@),%_$(FLASH_TYPE)_ubifs_script,$(n))),$(MAKE) $(b);)
	@$(call docuscmd,"$(ubi$(patsubst ubi%_$(FLASH_TYPE)__script,%,$@)$(CUSCMD))",$(SCRIPTDIR)/[[ubi$(patsubst ubi%_$(FLASH_TYPE)__script,%,$@).$(FILE_TYPE))
	@echo "% <- this is end of file symbol" >> $(SCRIPTDIR)/[[$(patsubst %_$(FLASH_TYPE)__script,%,$@).$(FILE_TYPE);

cis_$(FLASH_TYPE)__script:
	@echo "# <- this is for comment / total file size must be less than 4KB" > $(SCRIPTDIR)/[[cis.$(FILE_TYPE)
	@echo $(LOAD_CMD) $(DOWNLOADADDR) cis.bin >> $(SCRIPTDIR)/[[cis.$(FILE_TYPE)
	@echo $(FLASH_PROBE) >> $(SCRIPTDIR)/[[cis.$(FILE_TYPE)
	@echo $(FLASH_ERASE_PART) CIS >> $(SCRIPTDIR)/[[cis.$(FILE_TYPE)
	@echo $(FLASH_WRITE) $(DOWNLOADADDR) CIS \$${filesize} >> $(SCRIPTDIR)/[[cis.$(FILE_TYPE)
	@echo mtdparts del CIS >> $(SCRIPTDIR)/[[cis.$(FILE_TYPE)
	@$(call docuscmd,"$(cis$(CUSCMD))",$(SCRIPTDIR)/[[cis.$(FILE_TYPE))
	@echo "% <- this is end of file symbol" >> $(SCRIPTDIR)/[[cis.$(FILE_TYPE)

kernel_$(FLASH_TYPE)__script:
	@echo "# <- this is for comment / total file size must be less than 4KB" > $(SCRIPTDIR)/[[kernel.$(FILE_TYPE)
	@echo $(LOAD_CMD) $(DOWNLOADADDR) kernel >> $(SCRIPTDIR)/[[kernel.$(FILE_TYPE)
	@echo $(FLASH_PROBE) >> $(SCRIPTDIR)/[[kernel.$(FILE_TYPE)
	@if [ -n "$(kernel$(PATNAME))" ]; then \
		for i in $(kernel$(PATNAME));do \
			echo $(FLASH_ERASE_PART) $$i >> $(SCRIPTDIR)/[[kernel.$(FILE_TYPE); \
			echo $(FLASH_WRITE_PART) $(DOWNLOADADDR) $$i \$${filesize} >> $(SCRIPTDIR)/[[kernel.$(FILE_TYPE); \
		done; \
	else \
		echo $(FLASH_ERASE_PART) KERNEL >> $(SCRIPTDIR)/[[kernel.$(FILE_TYPE); \
		echo $(FLASH_WRITE_PART) $(DOWNLOADADDR) KERNEL \$${filesize} >> $(SCRIPTDIR)/[[kernel.$(FILE_TYPE); \
	fi;
	@$(call docuscmd,"$(kernel$(CUSCMD))",$(SCRIPTDIR)/[[kernel.$(FILE_TYPE))
	@$(call gen_setenv_cmd,kernel_file_size recovery_file_size,$(SCRIPTDIR)/[[kernel.$(FILE_TYPE),1)
	@echo "% <- this is end of file symbol" >> $(SCRIPTDIR)/[[kernel.$(FILE_TYPE)
	@echo kernel-image done!!!

ramdisk_$(FLASH_TYPE)_gz_script:
	@echo "# <- this is for comment / total file size must be less than 4KB" > $(SCRIPTDIR)/[[ramdisk.$(FILE_TYPE)
	echo $(LOAD_CMD) $(DOWNLOADADDR) ramdisk.gz >> $(SCRIPTDIR)/[[ramdisk.$(FILE_TYPE);
	echo $(FLASH_PROBE) >> $(SCRIPTDIR)/[[ramdisk.$(FILE_TYPE);
	if [ -n "$(ramdisk$(PATNAME))" ]; then \
		for i in $(ramdisk$(PATNAME));do \
			echo $(FLASH_ERASE_PART) $$i >> $(SCRIPTDIR)/[[ramdisk.$(FILE_TYPE); \
			echo $(FLASH_WRITE_PART) $(DOWNLOADADDR) $$i \$${filesize} >> $(SCRIPTDIR)/[[ramdisk.$(FILE_TYPE); \
		done; \
	else \
		echo $(FLASH_ERASE_PART) RAMDISK >> $(SCRIPTDIR)/[[ramdisk.$(FILE_TYPE); \
		echo $(FLASH_WRITE_PART) $(DOWNLOADADDR) RAMDISK \$${filesize} >> $(SCRIPTDIR)/[[ramdisk.$(FILE_TYPE); \
	fi;
	$(call docuscmd,"$(ramdisk$(CUSCMD))",$(SCRIPTDIR)/[[ramdisk.$(FILE_TYPE))
	$(call gen_setenv_cmd,initrd_high initrd_size,$(SCRIPTDIR)/[[ramdisk.$(FILE_TYPE),1)
	@echo "% <- this is end of file symbol" >> $(SCRIPTDIR)/[[ramdisk.$(FILE_TYPE)

pm_rtos_$(FLASH_TYPE)__script:
	@echo "# <- this is for comment / total file size must be less than 4KB" > $(SCRIPTDIR)/[[pm_rtos.$(FILE_TYPE)
	@echo $(LOAD_CMD) $(DOWNLOADADDR) pm_rtos >> $(SCRIPTDIR)/[[pm_rtos.$(FILE_TYPE)
	@echo $(FLASH_PROBE) >> $(SCRIPTDIR)/[[pm_rtos.$(FILE_TYPE)
	@echo $(FLASH_ERASE_PART) PM_RTOS >> $(SCRIPTDIR)/[[pm_rtos.$(FILE_TYPE)
	@echo $(FLASH_WRITE_PART) $(DOWNLOADADDR) PM_RTOS \$${filesize} >> $(SCRIPTDIR)/[[pm_rtos.$(FILE_TYPE)
	@echo "% <- this is end of file symbol" >> $(SCRIPTDIR)/[[pm_rtos.$(FILE_TYPE)

rtos_tftp_script:
	@echo "# <- this is for comment / total file size must be less than 4KB" > $(SCRIPTDIR)/[[rtos_tftp.es
	@$(call gen_setenv_cmd,bootargs_rtos,$(SCRIPTDIR)/[[rtos_tftp.es,1)
	@echo tftp 0x2f008000 rtos >> $(SCRIPTDIR)/[[rtos_tftp.es
	@echo tftp 0x20007fc0 kernel >> $(SCRIPTDIR)/[[rtos_tftp.es
	@echo tftp 0x30000000 rootfs.sqfs >> $(SCRIPTDIR)/[[rtos_tftp.es
	@echo tftp 0x30400000 miservice.sqfs >> $(SCRIPTDIR)/[[rtos_tftp.es
	@echo tftp 0x30800000 customer.jffs2 >> $(SCRIPTDIR)/[[rtos_tftp.es
	@echo tftp 0x20e00000 ramdisk.gz >> $(SCRIPTDIR)/[[rtos_tftp.es
	@echo tftp $(RTOS_RAMDISK_LOAD_ADDR) misc.fwfs >> $(SCRIPTDIR)/[[rtos_tftp.es
	@echo dcache off >> $(SCRIPTDIR)/[[rtos_tftp.es
	@echo bootm $(RTOS_LOAD_ADDR) >> $(SCRIPTDIR)/[[rtos_tftp.es
	@echo "% <- this is end of file symbol" >> $(SCRIPTDIR)/[[rtos_tftp.es

rtos_$(FLASH_TYPE)__script:
	@echo "# <- this is for comment / total file size must be less than 4KB" > $(SCRIPTDIR)/[[rtos.$(FILE_TYPE)
	@echo $(LOAD_CMD) $(DOWNLOADADDR) rtos >> $(SCRIPTDIR)/[[rtos.$(FILE_TYPE)
	@echo $(FLASH_PROBE) >> $(SCRIPTDIR)/[[rtos.$(FILE_TYPE)
	@if [ -n "$(rtos$(PATNAME))" ]; then \
		for i in $(rtos$(PATNAME));do \
			echo $(FLASH_ERASE_PART) $$i >> $(SCRIPTDIR)/[[rtos.$(FILE_TYPE); \
			echo $(FLASH_WRITE_PART) $(DOWNLOADADDR) $$i \$${filesize} >> $(SCRIPTDIR)/[[rtos.$(FILE_TYPE); \
		done; \
	else \
		echo $(FLASH_ERASE_PART) RTOS >> $(SCRIPTDIR)/[[rtos.$(FILE_TYPE); \
		echo $(FLASH_WRITE_PART) $(DOWNLOADADDR) RTOS \$${filesize} >> $(SCRIPTDIR)/[[rtos.$(FILE_TYPE); \
	fi;
	@$(call docuscmd,"$(rtos$(CUSCMD))",$(SCRIPTDIR)/[[rtos.$(FILE_TYPE))
	@echo "% <- this is end of file symbol" >> $(SCRIPTDIR)/[[rtos.$(FILE_TYPE)

earlyinit_$(FLASH_TYPE)__script:
	@echo "# <- this is for comment / total file size must be less than 4KB" > $(SCRIPTDIR)/[[earlyinit.$(FILE_TYPE)
	if [ ! -z "$(earlyinit$(RESOURCE))" ]; then \
		echo $(LOAD_CMD) $(DOWNLOADADDR) earlyinit >> $(SCRIPTDIR)/[[earlyinit.$(FILE_TYPE); \
		if [ -n "$(earlyinit$(PATNAME))" ]; then \
			for i in $(earlyinit$(PATNAME));do \
				echo $(FLASH_ERASE_PART) $$i >> $(SCRIPTDIR)/[[earlyinit.$(FILE_TYPE); \
				echo $(FLASH_WRITE_PART) $(DOWNLOADADDR) $$i \$${filesize} >> $(SCRIPTDIR)/[[earlyinit.$(FILE_TYPE); \
			done; \
		else \
			echo $(FLASH_ERASE_PART) EIB >> $(SCRIPTDIR)/[[earlyinit.$(FILE_TYPE); \
			echo $(FLASH_WRITE_PART) $(DOWNLOADADDR) EIB \$${filesize} >> $(SCRIPTDIR)/[[earlyinit.$(FILE_TYPE); \
		fi; \
	fi;
	@echo "% <- this is end of file symbol" >> $(SCRIPTDIR)/[[earlyinit.$(FILE_TYPE)

riscvfw_$(FLASH_TYPE)__script:
	@echo "# <- this is for comment / total file size must be less than 4KB" > $(SCRIPTDIR)/[[riscvfw.$(FILE_TYPE)
	@echo $(LOAD_CMD) $(DOWNLOADADDR) riscvfw >> $(SCRIPTDIR)/[[riscvfw.$(FILE_TYPE)
	@echo $(FLASH_PROBE) >> $(SCRIPTDIR)/[[riscvfw.$(FILE_TYPE)
	if [ -n "$(riscvfw$(PATNAME))" ]; then \
		for i in $(riscvfw$(PATNAME));do \
			echo $(FLASH_ERASE_PART) $$i >> $(SCRIPTDIR)/[[riscvfw.$(FILE_TYPE); \
			echo $(FLASH_WRITE_PART) $(DOWNLOADADDR) $$i \$${filesize} >> $(SCRIPTDIR)/[[riscvfw.$(FILE_TYPE); \
		done; \
	else \
		echo $(FLASH_ERASE_PART) RISCVFW >> $(SCRIPTDIR)/[[riscvfw.$(FILE_TYPE); \
		echo $(FLASH_WRITE_PART) $(DOWNLOADADDR) RISCVFW \$${filesize} >> $(SCRIPTDIR)/[[riscvfw.$(FILE_TYPE); \
	fi;
	@echo "% <- this is end of file symbol" >> $(SCRIPTDIR)/[[riscvfw.$(FILE_TYPE)

boot_$(FLASH_TYPE)__script:
	@echo "# <- this is for comment / total file size must be less than 4KB" > $(SCRIPTDIR)/[[boot.$(FILE_TYPE)
	@echo $(LOAD_CMD) $(DOWNLOADADDR) boot.bin >> $(SCRIPTDIR)/[[boot.$(FILE_TYPE)
ifeq ($(FLASH_TYPE), emmc)
	if  [ $(EMMC_BACKUPS) == "y" ]; then \
		echo mmc partconf 0 >>  $(SCRIPTDIR)/[[boot.$(FILE_TYPE); \
	fi;
	@echo mmc partconf 0 0  $(EMMC_PART_CONF) $(EMMC_BOOT_PART) >> $(SCRIPTDIR)/[[boot.$(FILE_TYPE)
	@echo $(FLASH_WRITE) $(DOWNLOADADDR) 0x0 $(boot$(PATSIZE)) >> $(SCRIPTDIR)/[[boot.$(FILE_TYPE)
	@echo mmc partconf 0 0 $(EMMC_BOOT_PART) $(EMMC_USER_PART) >> $(SCRIPTDIR)/[[boot.$(FILE_TYPE)
else
	@echo $(FLASH_PROBE) >> $(SCRIPTDIR)/[[boot.$(FILE_TYPE)
	@echo $(FLASH_ERASE_PART) BOOT >> $(SCRIPTDIR)/[[boot.$(FILE_TYPE)
	@echo $(FLASH_WRITE_PART) $(DOWNLOADADDR) BOOT \$${filesize} >> $(SCRIPTDIR)/[[boot.$(FILE_TYPE)
endif
ifneq ($(cis$(BOOTTAB1)), )
	@echo $(FLASH_ERASE_PART) BOOT_BAK >> $(SCRIPTDIR)/[[boot.$(FILE_TYPE)
	@echo $(FLASH_WRITE_PART) $(DOWNLOADADDR) BOOT_BAK \$${filesize} >> $(SCRIPTDIR)/[[boot.$(FILE_TYPE)
endif
	@$(call docuscmd,"$(boot$(CUSCMD))",$(SCRIPTDIR)/[[boot.$(FILE_TYPE))
	@echo "% <- this is end of file symbol" >> $(SCRIPTDIR)/[[boot.$(FILE_TYPE)

pm51_$(FLASH_TYPE)__script:
	@echo "# <- this is for comment / total file size must be less than 4KB" > $(SCRIPTDIR)/[[pm51.$(FILE_TYPE)
	@echo $(LOAD_CMD) $(DOWNLOADADDR) pm51 >> $(SCRIPTDIR)/[[pm51.$(FILE_TYPE)
	@echo $(FLASH_PROBE) >> $(SCRIPTDIR)/[[pm51.$(FILE_TYPE)
	@echo $(FLASH_ERASE_PART) PM51 >> $(SCRIPTDIR)/[[pm51.$(FILE_TYPE)
	@echo $(FLASH_WRITE_PART) $(DOWNLOADADDR) PM51 \$${filesize} >> $(SCRIPTDIR)/[[pm51.$(FILE_TYPE)
ifneq ($(cis$(BOOTTAB1)), )
	@echo $(FLASH_ERASE_PART) PM51_BAK >> $(SCRIPTDIR)/[[pm51.$(FILE_TYPE)
	@echo $(FLASH_WRITE_PART) $(DOWNLOADADDR) PM51_BAK \$${filesize} >> $(SCRIPTDIR)/[[pm51.$(FILE_TYPE)
endif
	@$(call docuscmd,"$(pm51$(CUSCMD))",$(SCRIPTDIR)/[[pm51.$(FILE_TYPE))
	@echo "% <- this is end of file symbol" >> $(SCRIPTDIR)/[[pm51.$(FILE_TYPE)

dtb_$(FLASH_TYPE)__script:
	@echo "# <- this is for comment / total file size must be less than 4KB" > $(SCRIPTDIR)/[[dtb.$(FILE_TYPE)
	@echo $(LOAD_CMD) $(DOWNLOADADDR) dtb >> $(SCRIPTDIR)/[[dtb.$(FILE_TYPE)
	@echo $(FLASH_PROBE) >> $(SCRIPTDIR)/[[dtb.$(FILE_TYPE)
	@echo $(FLASH_ERASE_PART) $(dtb$(PATNAME)) >> $(SCRIPTDIR)/[[dtb.$(FILE_TYPE);
	@echo $(FLASH_WRITE_PART) $(DOWNLOADADDR) $(dtb$(PATNAME)) \$${filesize} >> $(SCRIPTDIR)/[[dtb.$(FILE_TYPE);
	@$(call gen_setenv_cmd,fdt_high dtb_file_size,$(SCRIPTDIR)/[[dtb.$(FILE_TYPE),1)
	@echo "% <- this is end of file symbol" >> $(SCRIPTDIR)/[[dtb.$(FILE_TYPE)

dtbo_$(FLASH_TYPE)__script:
	@echo "# <- this is for comment / total file size must be less than 4KB" > $(SCRIPTDIR)/[[dtbo.$(FILE_TYPE)
	@echo $(LOAD_CMD) $(DOWNLOADADDR) dtbo >> $(SCRIPTDIR)/[[dtbo.$(FILE_TYPE)
	@echo $(FLASH_PROBE) >> $(SCRIPTDIR)/[[dtbo.$(FILE_TYPE)
	@echo $(FLASH_ERASE_PART) $(dtbo$(PATNAME)) >> $(SCRIPTDIR)/[[dtbo.$(FILE_TYPE);
	@echo $(FLASH_WRITE_PART) $(DOWNLOADADDR) $(dtbo$(PATNAME)) \$${filesize} >> $(SCRIPTDIR)/[[dtbo.$(FILE_TYPE);
	@$(call gen_setenv_cmd,kernel_dtbo_list uboot_dtbo_list,$(SCRIPTDIR)/[[dtbo.$(FILE_TYPE),1)
	@echo "% <- this is end of file symbol" >> $(SCRIPTDIR)/[[dtbo.$(FILE_TYPE)

%_$(FLASH_TYPE)_cponly_script:
	@echo [[$@]]

tf-a_$(FLASH_TYPE)__script:
	@echo [[$@]]
%_$(FLASH_TYPE)_lwfs_script:
	@echo "# <- this is for comment / total file size must be less than 4KB" > $(SCRIPTDIR)/[[$(patsubst %_$(FLASH_TYPE)_lwfs_script,%,$@).$(FILE_TYPE)
	@$(call doflashwrite,$(patsubst %_$(FLASH_TYPE)_lwfs_script,%,$@).lwfs,[[$(patsubst %_$(FLASH_TYPE)_lwfs_script,%,$@).$(FILE_TYPE),$(patsubst %_$(FLASH_TYPE)_lwfs_script,%,$@))
	@echo "% <- this is end of file symbol" >> $(SCRIPTDIR)/[[$(patsubst %_$(FLASH_TYPE)_lwfs_script,%,$@).$(FILE_TYPE)

%_$(FLASH_TYPE)_lfs_script:
	@echo "# <- this is for comment / total file size must be less than 4KB" > $(SCRIPTDIR)/[[$(patsubst %_$(FLASH_TYPE)_lfs_script,%,$@).$(FILE_TYPE)
	@$(call doflashwrite,$(patsubst %_$(FLASH_TYPE)_lfs_script,%,$@).lfs,[[$(patsubst %_$(FLASH_TYPE)_lfs_script,%,$@).$(FILE_TYPE),$(patsubst %_$(FLASH_TYPE)_lfs_script,%,$@))
	@$(call docuscmd,"$($(patsubst %_$(FLASH_TYPE)_lfs_script,%,$@)$(BLKCMD))",$(SCRIPTDIR)/[[$(patsubst %_$(FLASH_TYPE)_lfs_script,%,$@).$(FILE_TYPE))
	@echo "% <- this is end of file symbol" >> $(SCRIPTDIR)/[[$(patsubst %_$(FLASH_TYPE)_lfs_script,%,$@).$(FILE_TYPE)

%_$(FLASH_TYPE)_fwfs_script:
	@echo "# <- this is for comment / total file size must be less than 4KB" > $(SCRIPTDIR)/[[$(patsubst %_$(FLASH_TYPE)_fwfs_script,%,$@).$(FILE_TYPE)
	@$(call doflashwrite,$(patsubst %_$(FLASH_TYPE)_fwfs_script,%,$@).fwfs,[[$(patsubst %_$(FLASH_TYPE)_fwfs_script,%,$@).$(FILE_TYPE),$(patsubst %_$(FLASH_TYPE)_fwfs_script,%,$@))
	@$(call docuscmd,"$($(patsubst %_$(FLASH_TYPE)_fwfs_script,%,$@)$(BLKCMD))",$(SCRIPTDIR)/[[$(patsubst %_$(FLASH_TYPE)_fwfs_script,%,$@).$(FILE_TYPE))
	@echo "% <- this is end of file symbol" >> $(SCRIPTDIR)/[[$(patsubst %_$(FLASH_TYPE)_fwfs_script,%,$@).$(FILE_TYPE)

%_$(FLASH_TYPE)_squashfs_script:
	@echo "# <- this is for comment / total file size must be less than 4KB" > $(SCRIPTDIR)/[[$(patsubst %_$(FLASH_TYPE)_squashfs_script,%,$@).$(FILE_TYPE)
	@if [ "$($(patsubst %_$(FLASH_TYPE)_squashfs_script,%,$@)$(UBIVOLID))" != "" ]; then \
		$(call doflashwrite,$(patsubst %_$(FLASH_TYPE)_squashfs_script,%,$@).ubi.bin,[[$(patsubst %_$(FLASH_TYPE)_squashfs_script,%,$@).$(FILE_TYPE),$(patsubst %_$(FLASH_TYPE)_squashfs_script,%,$@)) \
	else \
		$(call doflashwrite,$(patsubst %_$(FLASH_TYPE)_squashfs_script,%,$@).sqfs,[[$(patsubst %_$(FLASH_TYPE)_squashfs_script,%,$@).$(FILE_TYPE),$(patsubst %_$(FLASH_TYPE)_squashfs_script,%,$@)) \
	fi;
	@$(call docuscmd,"$($(patsubst %_$(FLASH_TYPE)_squashfs_script,%,$@)$(BLKCMD))",$(SCRIPTDIR)/[[$(patsubst %_$(FLASH_TYPE)_squashfs_script,%,$@).$(FILE_TYPE))
	@echo "% <- this is end of file symbol" >> $(SCRIPTDIR)/[[$(patsubst %_$(FLASH_TYPE)_squashfs_script,%,$@).$(FILE_TYPE)

%_$(FLASH_TYPE)_ramfs_script:
	@echo "# <- this is for comment / total file size must be less than 4KB" > $(SCRIPTDIR)/[[$(patsubst %_$(FLASH_TYPE)_ramfs_script,%,$@).$(FILE_TYPE)
	@$(call doflashwrite,$(patsubst %_$(FLASH_TYPE)_ramfs_script,%,$@).ramfs,[[$(patsubst %_$(FLASH_TYPE)_ramfs_script,%,$@).$(FILE_TYPE),$(patsubst %_$(FLASH_TYPE)_ramfs_script,%,$@))
	@$(call docuscmd,"$($(patsubst %_$(FLASH_TYPE)_ramfs_script,%,$@)$(BLKCMD))",$(SCRIPTDIR)/[[$(patsubst %_$(FLASH_TYPE)_ramfs_script,%,$@).$(FILE_TYPE))
	@$(call gen_setenv_cmd,rootfs_file_size initrd_high initrd_size initrd_block initrd_comp, \
		$(SCRIPTDIR)/[[$(patsubst %_$(FLASH_TYPE)_ramfs_script,%,$@).$(FILE_TYPE),1)
	@echo "% <- this is end of file symbol" >> $(SCRIPTDIR)/[[$(patsubst %_$(FLASH_TYPE)_ramfs_script,%,$@).$(FILE_TYPE)

%_$(FLASH_TYPE)_jffs2_script:
	@echo "# <- this is for comment / total file size must be less than 4KB" > $(SCRIPTDIR)/[[$(patsubst %_$(FLASH_TYPE)_jffs2_script,%,$@).$(FILE_TYPE)
	@$(call doflashwrite,$(patsubst %_$(FLASH_TYPE)_jffs2_script,%,$@).jffs2,[[$(patsubst %_$(FLASH_TYPE)_jffs2_script,%,$@).$(FILE_TYPE),$(patsubst %_$(FLASH_TYPE)_jffs2_script,%,$@))
	@$(call docuscmd,"$($(patsubst %_$(FLASH_TYPE)_jffs2_script,%,$@)$(BLKCMD))",$(SCRIPTDIR)/[[$(patsubst %_$(FLASH_TYPE)_jffs2_script,%,$@).$(FILE_TYPE))
	@echo  "% <- this is end of file symbol" >> $(SCRIPTDIR)/[[$(patsubst %_$(FLASH_TYPE)_jffs2_script,%,$@).$(FILE_TYPE)

%_$(FLASH_TYPE)_ubifs_script:
	@if [ -z "$($(patsubst %_$(FLASH_TYPE)_ubifs_script,%,$@)$(UBIVOLID))" ]; then \
		echo "# <- this is for comment / total file size must be less than 4KB" > $(SCRIPTDIR)/[[$(patsubst %_$(FLASH_TYPE)_ubifs_script,%,$@).$(FILE_TYPE);	\
		echo $(FLASH_PROBE) >> $(SCRIPTDIR)/[[$(patsubst %_$(FLASH_TYPE)_ubifs_script,%,$@).$(FILE_TYPE);	\
		echo ubi part $($(patsubst %_$(FLASH_TYPE)_ubifs_script,%,$@)$(OPTIONS)) >> $(SCRIPTDIR)/[[$(patsubst %_$(FLASH_TYPE)_ubifs_script,%,$@).$(FILE_TYPE);	\
		SPLIT_SIZE="`printf %d $(SPLIT_EACH_FILE_SIZE)`"; \
		IMAGE_SIZE="`stat --format=%s $(IMAGEDIR)/$(patsubst %_$(FLASH_TYPE)_ubifs_script,%,$@).ubifs`"; \
		IMAGE_SIZE_16="0x`echo "obase=16;$$IMAGE_SIZE"|bc`"; \
		if [ -n "$($(patsubst %_$(FLASH_TYPE)_ubifs_script,%,$@)$(PATNAME))" ]; then \
			for i in $($(patsubst %_$(FLASH_TYPE)_ubifs_script,%,$@)$(PATNAME));do \
				echo ubi create $$i >> $(SCRIPTDIR)/[[$(patsubst %_$(FLASH_TYPE)_ubifs_script,%,$@).$(FILE_TYPE); \
			done; \
		else \
			echo ubi create $(patsubst %_$(FLASH_TYPE)_ubifs_script,%,$@) $($(patsubst %_$(FLASH_TYPE)_ubifs_script,%,$@)$(PATSIZE)) >> $(SCRIPTDIR)/[[$(patsubst %_$(FLASH_TYPE)_ubifs_script,%,$@).$(FILE_TYPE); \
		fi; \
		if [ $$IMAGE_SIZE -gt $$SPLIT_SIZE ]; then \
			echo -e "\e[1;32m $(patsubst %_$(FLASH_TYPE)_ubifs_script,%,$@) too large, do split! \e[0m"; \
			split -b $$SPLIT_SIZE $(IMAGEDIR)/$(patsubst %_$(FLASH_TYPE)_ubifs_script,%,$@).ubifs $(IMAGEDIR)/$(patsubst %_$(FLASH_TYPE)_ubifs_script,%,$@).ubifs_; \
			for i in `ls $(IMAGEDIR)/|grep $(patsubst %_$(FLASH_TYPE)_ubifs_script,%,$@).ubifs_`; do \
				echo $(LOAD_CMD) $(DOWNLOADADDR) $$i >> $(SCRIPTDIR)/[[$(patsubst %_$(FLASH_TYPE)_ubifs_script,%,$@).$(FILE_TYPE); \
				if [ -n "$($(patsubst %_$(FLASH_TYPE)_ubifs_script,%,$@)$(PATNAME))" ]; then \
					for i in $($(patsubst %_$(FLASH_TYPE)_ubifs_script,%,$@)$(PATNAME));do \
						echo ubi write.part ${DOWNLOADADDR} $$i \$${filesize} $$IMAGE_SIZE_16 >> $(SCRIPTDIR)/[[$(patsubst %_$(FLASH_TYPE)_ubifs_script,%,$@).$(FILE_TYPE); \
					done; \
				else \
						echo ubi write.part ${DOWNLOADADDR} $(patsubst %_$(FLASH_TYPE)_ubifs_script,%,$@) \$${filesize} $$IMAGE_SIZE_16 >> $(SCRIPTDIR)/[[$(patsubst %_$(FLASH_TYPE)_ubifs_script,%,$@).$(FILE_TYPE); \
				fi; \
				IMAGE_SIZE_16=; \
			done; \
		else \
			echo $(LOAD_CMD) $(DOWNLOADADDR) $(patsubst %_$(FLASH_TYPE)_ubifs_script,%,$@).ubifs >> $(SCRIPTDIR)/[[$(patsubst %_$(FLASH_TYPE)_ubifs_script,%,$@).$(FILE_TYPE);	\
			if [ -n "$($(patsubst %_$(FLASH_TYPE)_ubifs_script,%,$@)$(PATNAME))" ]; then \
				for i in $($(patsubst %_$(FLASH_TYPE)_ubifs_script,%,$@)$(PATNAME));do \
					echo ubi write $$i \$${filesize} >> $(SCRIPTDIR)/[[$(patsubst %_$(FLASH_TYPE)_ubifs_script,%,$@).$(FILE_TYPE);	\
				done; \
			else \
				echo ubi write $(DOWNLOADADDR) $(patsubst %_$(FLASH_TYPE)_ubifs_script,%,$@) \$${filesize} >> $(SCRIPTDIR)/[[$(patsubst %_$(FLASH_TYPE)_ubifs_script,%,$@).$(FILE_TYPE);	\
			fi; \
		fi; \
		$(call docuscmd,"$($(patsubst %_$(FLASH_TYPE)_ubifs_script,%,$@)$(BLKCMD))",$(SCRIPTDIR)/[[$(patsubst %_$(FLASH_TYPE)_ubifs_script,%,$@).$(FILE_TYPE))	\
		echo "% <- this is end of file symbol" >>  $(SCRIPTDIR)/[[$(patsubst %_$(FLASH_TYPE)_ubifs_script,%,$@).$(FILE_TYPE);	\
		echo estar scripts/[[$(patsubst %_$(FLASH_TYPE)_ubifs_script,%,$@).$(FILE_TYPE) >> $(SCRIPTDIR)/[[$($(patsubst %_$(FLASH_TYPE)_ubifs_script,%,$@)$(OPTIONS)).$(FILE_TYPE);	\
	else \
		echo [[$@]]; \
	fi;

%_$(FLASH_TYPE)_ext4_script:
	@echo "# <- this is for comment /" > $(SCRIPTDIR)/[[$(patsubst %_$(FLASH_TYPE)_ext4_script,%,$@).$(FILE_TYPE)
	echo $(FLASH_PROBE) >> $(SCRIPTDIR)/[[$(patsubst %_$(FLASH_TYPE)_ext4_script,%,$@).$(FILE_TYPE); \
	if [ -n "$($(patsubst %_$(FLASH_TYPE)_ext4_script,%,$@)$(PATNAME))" ]; then \
		for i in $($(patsubst %_$(FLASH_TYPE)_ext4_script,%,$@)$(PATNAME));do \
			echo $(FLASH_ERASE_PART) $$i >> $(SCRIPTDIR)/[[$(patsubst %_$(FLASH_TYPE)_ext4_script,%,$@).$(FILE_TYPE); \
		done; \
	else \
		echo $(FLASH_ERASE_PART) $(patsubst %_$(FLASH_TYPE)_ext4_script,%,$@) >> $(SCRIPTDIR)/[[$(patsubst %_$(FLASH_TYPE)_ext4_script,%,$@).$(FILE_TYPE); \
	fi; \
	SPLIT_SIZE="`printf %d $(SPLIT_EACH_FILE_SIZE)`"; \
	SPLIT_SIZE_16=0x`echo "obase=16;$$SPLIT_SIZE"|bc`; \
	IMAGE_SIZE="`stat --format=%s $(IMAGEDIR)/$(patsubst %_$(FLASH_TYPE)_ext4_script,%,$@).ext4`"; \
	IMAGE_SIZE_16=0x`echo "obase=16;$$IMAGE_SIZE"|bc`; \
	if [ $$IMAGE_SIZE -gt $$SPLIT_SIZE ]; then \
		echo -e "\e[1;32m $(patsubst %_$(FLASH_TYPE)_ext4_script,%,$@) too large, do split! \e[0m"; \
		offset_blk=0; \
		split -b $$SPLIT_SIZE $(IMAGEDIR)/$(patsubst %_$(FLASH_TYPE)_ext4_script,%,$@).ext4 $(IMAGEDIR)/$(patsubst %_$(FLASH_TYPE)_ext4_script,%,$@).ext4_; \
		for i in `ls $(IMAGEDIR)/ | grep $(patsubst %_$(FLASH_TYPE)_ext4_script,%,$@).ext4_`; do \
			offset_blk_16=0x`echo "obase=16;$$offset_blk"|bc`; \
			echo $(LOAD_CMD) $(DOWNLOADADDR) $$i >> $(SCRIPTDIR)/[[$(patsubst %_$(FLASH_TYPE)_ext4_script,%,$@).$(FILE_TYPE); \
			if [ -n "$($(patsubst %_$(FLASH_TYPE)_ext4_script,%,$@)$(PATNAME))" ]; then \
				for i in $($(patsubst %_$(FLASH_TYPE)_ext4_script,%,$@)$(PATNAME));do \
					echo $(FLASH_WRITE_CONTINUE) $(DOWNLOADADDR) $$i $$offset_blk_16 \$${filesize} >> $(SCRIPTDIR)/[[$(patsubst %_$(FLASH_TYPE)_ext4_script,%,$@).$(FILE_TYPE); \
				done; \
			else \
				echo $(FLASH_WRITE_CONTINUE) $(DOWNLOADADDR) $(patsubst %_$(FLASH_TYPE)_ext4_script,%,$@) $$offset_blk_16 \$${filesize} >> $(SCRIPTDIR)/[[$(patsubst %_$(FLASH_TYPE)_ext4_script,%,$@).$(FILE_TYPE); \
			fi; \
			IMAGE_BLK_SIZE=$$(($$SPLIT_SIZE/$(FLASH_BLK_SIZE))); \
			offset_blk=$$(($$offset_blk + $$IMAGE_BLK_SIZE)); \
		done; \
	else \
		echo $(LOAD_CMD) $(DOWNLOADADDR) $(patsubst %_$(FLASH_TYPE)_ext4_script,%,$@).ext4 >> $(SCRIPTDIR)/[[$(patsubst %_$(FLASH_TYPE)_ext4_script,%,$@).$(FILE_TYPE);	\
		if [ -n "$($(patsubst %_$(FLASH_TYPE)_ext4_script,%,$@)$(PATNAME))" ]; then \
			for i in $($(patsubst %_$(FLASH_TYPE)_ext4_script,%,$@)$(PATNAME));do \
				echo $(FLASH_WRITE_PART) $(DOWNLOADADDR) $$i $$IMAGE_SIZE_16 >> $(SCRIPTDIR)/[[$(patsubst %_$(FLASH_TYPE)_ext4_script,%,$@).$(FILE_TYPE);	\
			done; \
		else \
			echo $(FLASH_WRITE_PART) $(DOWNLOADADDR) $(patsubst %_$(FLASH_TYPE)_ext4_script,%,$@) $$IMAGE_SIZE_16 >> $(SCRIPTDIR)/[[$(patsubst %_$(FLASH_TYPE)_ext4_script,%,$@).$(FILE_TYPE);	\
		fi; \
	fi;
	@$(call docuscmd,"$($(patsubst %_$(FLASH_TYPE)_ext4_script,%,$@)$(BLKCMD))",$(SCRIPTDIR)/[[$(patsubst %_$(FLASH_TYPE)_ext4_script,%,$@).$(FILE_TYPE))
	@echo  "% <- this is end of file symbol" >> $(SCRIPTDIR)/[[$(patsubst %_$(FLASH_TYPE)_ext4_script,%,$@).$(FILE_TYPE)

$(FLASH_TYPE)_config_script:
	@echo "# <- this is for comment / total file size must be less than 4KB" > $(SCRIPTDIR)/set_config
	@$(call gen_setenv_cmd,bootargs,$(SCRIPTDIR)/set_config)
	@$(call gen_setenv_cmd,bootcmd,$(SCRIPTDIR)/set_config)

ifeq ($(USE_AB_SYSTEM_BOOT), y)
	@$(call gen_setenv_cmd,bootcmd_a,$(SCRIPTDIR)/set_config)
	@$(call gen_setenv_cmd,bootcmd_b,$(SCRIPTDIR)/set_config)
endif

ifeq ($(RTOS_ENABLE), on)
	@$(call gen_setenv_cmd,bootargs_rtos,$(SCRIPTDIR)/set_config)
	@$(call gen_setenv_cmd,ep_addr_rtos,$(SCRIPTDIR)/set_config)
	@$(call gen_setenv_cmd,rtos_base,$(SCRIPTDIR)/set_config)
endif

ifeq ($(DUAL_OS), on)
ifeq ($(DUAL_OS_TYPE), HYP)
	@$(call gen_setenv_cmd,bootargs_hyp,$(SCRIPTDIR)/set_config)
endif
	@$(call gen_setenv_cmd,bootargs_linux_only,$(SCRIPTDIR)/set_config)
ifneq ($(CONFIG_ENABLE_POWER_SAVE_AOV), y)
	@$(call gen_setenv_cmd,bootcmd_linux_only,$(SCRIPTDIR)/set_config)
else
	@$(call gen_setenv_cmd,bootcmd_dualos,$(SCRIPTDIR)/set_config)
endif # CONFIG_ENABLE_POWER_SAVE_AOV
endif # DUAL_OS

	@$(call gen_setenv_cmd,overdrive,$(SCRIPTDIR)/set_config)

	@echo saveenv >> $(SCRIPTDIR)/set_config
	@echo reset >> $(SCRIPTDIR)/set_config
	@echo "% <- this is end of file symbol" >> $(SCRIPTDIR)/set_config

fat_sdmmc__script:
	@echo "# <- this is for comment / total file size must be less than 4KB" > $(SCRIPTDIR)/[[fat.$(FILE_TYPE)
	@echo -e fatformat mmc 0:1 >> $(SCRIPTDIR)/[[fat.$(FILE_TYPE)
	$(MAKE) $(foreach n,$(fat$(SYSTAB)),$(n)_$(FLASH_TYPE)_fat_script) -j1
	@echo "% <- this is end of file symbol" >> $(SCRIPTDIR)/[[fat.$(FILE_TYPE)

%_sdmmc_fat_script:
	@echo -e $(LOAD_CMD) $(DOWNLOADADDR) boot/$(shell echo $(patsubst %_sdmmc_fat_script,%,$@) |  tr '[a-z]' '[A-Z]') >> $(SCRIPTDIR)/[[fat.$(FILE_TYPE)
	@echo -e $(FLASH_WRITE) $(DOWNLOADADDR) $(shell echo $(patsubst %_sdmmc_fat_script,%,$@) |  tr '[a-z]' '[A-Z]') \$${filesize} >> $(SCRIPTDIR)/[[fat.$(FILE_TYPE)

system_sdmmc__script:
	@echo "# <- this is for comment / total file size must be less than 4KB" > $(SCRIPTDIR)/[[system.$(FILE_TYPE)
	$(MAKE) $(foreach n,$(system$(SYSTAB)),$(n)_$(FLASH_TYPE)_dos_script) -j1
	@$(foreach n,$(system$(SYSTAB)),echo "$(STAR_CMD) scripts/[[$(n)" >> $(SCRIPTDIR)/[[system.$(FILE_TYPE);)
	@echo  "% <- this is end of file symbol" >> $(SCRIPTDIR)/[[system.$(FILE_TYPE)

# sd boot partition -- kernel, rootfs, miservice, customer, misc
%_sdmmc_dos_script:
	@echo "# <- this is for comment /" > $(SCRIPTDIR)/[[$(patsubst %_sdmmc_dos_script,%,$@)
	@IMAGE_NAME=$(patsubst %_sdmmc_dos_script,%,$@); \
	IMAGE_INDEX=$($(patsubst %_sdmmc_dos_script,%,$@)$(INDEX)); \
	if [ "$($(patsubst %_sdmmc_dos_script,%,$@)$(FSTYPE))" == "" ]; then \
		IMAGE_PATH=$(IMAGEDIR)/$$IMAGE_NAME; \
	else \
		IMAGE_PATH=$(IMAGEDIR)/$$IMAGE_NAME.$($(patsubst %_sdmmc_dos_script,%,$@)$(FSTYPE)); \
		IMAGE_NAME=$$IMAGE_NAME.$($(patsubst %_sdmmc_dos_script,%,$@)$(FSTYPE)); \
	fi; \
	IMAGE_SIZE="`stat --format=%s $$IMAGE_PATH`"; \
	IMAGE_SIZE_16="0x`echo "obase=16;$$IMAGE_SIZE"|bc`"; \
	IMAGE_ALIGN=$$(($$IMAGE_SIZE%512)); \
	IMAGE_BLK_SIZE=$$(($$IMAGE_SIZE/512)); \
	if [ $$IMAGE_ALIGN -gt 0 ]; then \
		IMAGE_BLK_SIZE=$$(($$IMAGE_BLK_SIZE+1)); \
	fi; \
	IMAGE_BLK_SIZE_16="0x`echo "obase=16;$$IMAGE_BLK_SIZE"|bc`"; \
	if [ $$IMAGE_SIZE -gt $(SPLIT_EACH_FILE_SIZE) ]; then \
		split -b $(SPLIT_EACH_FILE_SIZE) $$IMAGE_PATH $${IMAGE_PATH}_; \
		echo $(FLASH_ERASE_PART)$$IMAGE_INDEX >> $(SCRIPTDIR)/[[$(patsubst %_sdmmc_dos_script,%,$@); \
		offset_blk=0; \
		SPLIT_IMAGE_SIZE_16="0x`echo "obase=16;$(SPLIT_EACH_FILE_SIZE)"|bc`"; \
		SPLIT_IMAGE_ALIGN=$$(($(SPLIT_EACH_FILE_SIZE)%512)); \
		SPLIT_IMAGE_BLK_SIZE=$$(($(SPLIT_EACH_FILE_SIZE)/512)); \
		if [ $$SPLIT_IMAGE_ALIGN -gt 0 ]; then \
			SPLIT_IMAGE_BLK_SIZE=$$(($$SPLIT_IMAGE_BLK_SIZE+1)); \
		fi; \
		SPLIT_IMAGE_BLK_SIZE_16="0x`echo "obase=16;$$SPLIT_IMAGE_BLK_SIZE"|bc`"; \
		for i in `ls $(IMAGEDIR)/|grep $${IMAGE_NAME}_`; do \
			offset_blk_16=0x`echo "obase=16;$$offset_blk"|bc`; \
			echo $(LOAD_CMD) $(DOWNLOADADDR) $$i >> $(SCRIPTDIR)/[[$(patsubst %_sdmmc_dos_script,%,$@); \
			echo $(FLASH_WRITE_PART)$$IMAGE_INDEX $(DOWNLOADADDR) $$offset_blk_16 $$SPLIT_IMAGE_BLK_SIZE_16 >> $(SCRIPTDIR)/[[$(patsubst %_sdmmc_dos_script,%,$@); \
			offset_blk=$$(($$offset_blk + $$SPLIT_IMAGE_BLK_SIZE)); \
		done; \
	else \
		echo $(FLASH_ERASE_PART)$$IMAGE_INDEX >> $(SCRIPTDIR)/[[$(patsubst %_sdmmc_dos_script,%,$@); \
		echo $(LOAD_CMD) $(DOWNLOADADDR) $$IMAGE_NAME >> $(SCRIPTDIR)/[[$(patsubst %_sdmmc_dos_script,%,$@); \
		if [ "$(patsubst %_sdmmc_dos_script,%,$@)" == "kernel" ]; then \
			echo setexpr image_blk_size_tmp \$${filesize} / 200 >> $(SCRIPTDIR)/[[$(patsubst %_sdmmc_dos_script,%,$@); \
			echo setexpr image_blk_size \$${image_blk_size_tmp} + 1 >> $(SCRIPTDIR)/[[$(patsubst %_sdmmc_dos_script,%,$@); \
			echo $(FLASH_WRITE_PART)$$IMAGE_INDEX $(DOWNLOADADDR) 0 \$${image_blk_size} >> $(SCRIPTDIR)/[[$(patsubst %_sdmmc_dos_script,%,$@); \
		else \
			echo $(FLASH_WRITE_PART)$$IMAGE_INDEX $(DOWNLOADADDR) 0 $$IMAGE_BLK_SIZE_16 >> $(SCRIPTDIR)/[[$(patsubst %_sdmmc_dos_script,%,$@); \
		fi; \
	fi;\
	if [ "$(patsubst %_sdmmc_dos_script,%,$@)"x == "misc"x ]; then \
		echo saveenv >> $(SCRIPTDIR)/[[$(patsubst %_sdmmc_dos_script,%,$@); \
	fi;
	@echo "% <- this is end of file symbol" >> $(SCRIPTDIR)/[[$(patsubst %_sdmmc_dos_script,%,$@)
