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
include $(PROJ_ROOT)/release/chip/$(CHIP)/$(PRODUCT)/$(BOARD)/bootscript/$(FLASH_TYPE)/nonpm_fastboot_sequence.mk

LFS_MOUNT_BLK := $(foreach m, $(filter %_lfs, $(foreach block, $(USR_MOUNT_BLOCKS), $(block)_$($(block)$(FSTYPE)))), $(patsubst %_lfs, %, $(m)))
FWFS_MOUNT_BLK := $(foreach m, $(filter %_fwfs, $(foreach block, $(USR_MOUNT_BLOCKS), $(block)_$($(block)$(FSTYPE)))), $(patsubst %_fwfs, %, $(m)))
MOUNT_BLK := $(filter-out $(LFS_MOUNT_BLK), $(USR_MOUNT_BLOCKS))
MOUNT_BLK := $(filter-out $(FWFS_MOUNT_BLK), $(MOUNT_BLK))

INIT_FILE=$(OUTPUTDIR)/rootfs/etc/init.sh
LATE_INIT_FILE=$(miservice$(RESOURCE))/demo.sh

DEMO=$(LIB_DIR_PATH)/bin/sample_code/bin/prog_cm4_nonPM_demo
DEMO_RELEASE_DIR=$(OUTPUTDIR)/rootfs/
DEMO_RUN_PATH=
DEMO_RUN_OPT=file 1 panel 1 &

MOD_PATH_KERNEL:=$(KERN_MODS_PATH)/
MOD_PATH_MI:=$(LIB_DIR_PATH)/modules/$(KERNEL_VERSION)/
MODULES_PATH:= $(MOD_PATH_KERNEL) $(MOD_PATH_MI)
MODULES_PATH_LIST_FIRST:=$(wildcard $(foreach mod,$(MODULES_1st),$(foreach path, $(MODULES_PATH), $(path)$(mod))))
MODULES_LIST_FIRST:=$(notdir $(wildcard $(foreach mod,$(MODULES_1st),$(foreach path, $(MODULES_PATH), $(path)$(mod)))))
MODULES_PATH_LIST_SECOND:=$(wildcard $(foreach mod,$(MODULES_2nd),$(foreach path, $(MODULES_PATH), $(path)$(mod))))
MODULES_LIST_SECOND:=$(notdir $(wildcard $(foreach mod,$(MODULES_2nd),$(foreach path, $(MODULES_PATH), $(path)$(mod)))))
MODULES_PATH_LIST_UBIFS:=$(wildcard $(foreach mod,$(MODULES_PERUSERMOUNT),$(foreach path, $(MODULES_PATH), $(path)$(mod))))
MODULES_PATH_UBIFS:=$(notdir $(wildcard $(foreach mod,$(MODULES_PERUSERMOUNT),$(foreach path, $(MODULES_PATH), $(path)$(mod)))))

THIRD_PARTY_LIBS=$(RELEASE_ROOT)/3rd_party_libs/dynamic/
EX_LIBS=$(SIGMA_COM_LIB_DIR_PATH)/dynamic/
MI_LIBS=$(LIB_DIR_PATH)/mi_libs/dynamic/
LIBS_PATH:= $(THIRD_PARTY_LIBS) $(MI_LIBS) $(EX_LIBS)
LIBS_PATH_LIST_FIRST:=$(wildcard $(foreach lib,$(LIBS_1st),$(foreach path, $(LIBS_PATH), $(path)$(lib))))
LIBS_LIST_FIRST:=$(notdir $(wildcard $(foreach lib,$(LIBS_1st),$(foreach path, $(LIBS_PATH), $(path)$(lib)))))
LIBS_PATH_LIST_SECOND:=$(wildcard $(foreach lib,$(LIBS_2nd),$(foreach path, $(LIBS_PATH), $(path)$(lib))))
LIBS_LIST_SECOND:=$(notdir $(wildcard $(foreach lib,$(LIBS_2nd),$(foreach path, $(LIBS_PATH), $(path)$(lib)))))

CONFIG_FILES_DIR=$(OUTPUTDIR)/rootfs/preconfig
CONFIG_MODPARAM=$(CONFIG_FILES_DIR)/modparam.json

ifeq ($(TOOLCHAIN), uclibc)
LIBC=$(strip $(patsubst %.tar.gz, %, $(word 1, $(notdir $(wildcard $(LIB_DIR_PATH)/package/libuclibc-*.tar.gz)))))
else
LIBC=$(strip $(patsubst %.tar.gz, %, $(word 1, $(notdir $(wildcard $(LIB_DIR_PATH)/package/libc-*.tar.gz)))))
endif

define release_demo
if [ -f $(DEMO) ]; then \
	cp -rf $(DEMO) $(DEMO_RELEASE_DIR); \
	chmod +x $(DEMO_RELEASE_DIR)/$(notdir $(DEMO)); \
	$(STRIP)  $(STRIP_OPTION) $(DEMO_RELEASE_DIR)/$(notdir $(DEMO)); \
	echo "$(DEMO_RUN_PATH)/$(notdir $(DEMO)) $(DEMO_RUN_OPT)" >> $(INIT_FILE); \
fi;
endef

.PHONY: rootfs

rootfs_$(PRODUCT):
rootfs: rootfs_$(PRODUCT)

rootfs_$(PRODUCT):
	@echo [================= $@ =================]

	# pack rootfs
	cd rootfs; tar xf rootfs.tar.gz -C $(OUTPUTDIR)

	# pack busybox
	tar xf busybox/$(BUSYBOX).tar.gz -C $(OUTPUTDIR)/rootfs

	# rename linuxrc to init
	mv $(OUTPUTDIR)/rootfs/linuxrc $(OUTPUTDIR)/rootfs/init ; \

	# pack library
	tar xf $(RELEASE_ROOT)/package/$(LIBC).tar.gz -C $(OUTPUTDIR)/rootfs/lib
	-if [ "$(LIBS_PATH_LIST_FIRST)" != "" ];then \
		cp -rvf $(LIBS_PATH_LIST_FIRST) $(OUTPUTDIR)/rootfs/lib/;\
	fi;
	mkdir -p $(miservice$(RESOURCE))/lib
	-if [ "$(LIBS_PATH_LIST_SECOND)" != "" ];then \
		cp -rvf $(LIBS_PATH_LIST_SECOND) $(miservice$(RESOURCE))/lib/;\
		find $(miservice$(RESOURCE))/lib/ -name "*.so*" -type f | xargs $(STRIP)  $(STRIP_OPTION); \
	fi;

	# pack base scripts
	cp -rf etc/* $(OUTPUTDIR)/rootfs/etc

	# pack fw_printenv
	cp -rvf $(PROJ_ROOT)/tools/$(TOOLCHAIN)/$(TOOLCHAIN_VERSION)/fw_printenv/* $(OUTPUTDIR)/rootfs/etc/
	echo "$(ENV_CFG)" > $(OUTPUTDIR)/rootfs/etc/fw_env.config
	cd $(OUTPUTDIR)/rootfs/etc/;ln -sf fw_printenv fw_setenv;chmod 777 fw_printenv

	# pack board cfg/bin file
	mkdir -p $(CONFIG_FILES_DIR)/
	cp -rf $(PROJ_ROOT)/board/ini/* $(OUTPUTDIR)/customer
	cp -rf $(PROJ_ROOT)/board/$(CHIP)/$(BOARD_NAME)/config/* $(CONFIG_FILES_DIR)
	if [ "$(MMAP)" != "" ]; then \
		cp -vf $(PROJ_ROOT)/board/$(CHIP)/mmap/$(MMAP) $(CONFIG_FILES_DIR)/mmap.ini; \
	fi;
	if [ $(DEBUG_MODPARAM) != 0 ]; then \
		cp -rf $(PROJ_ROOT)/board/$(CHIP)/json/$(PRODUCT)/modparam_debug.json $(CONFIG_MODPARAM); \
	else \
		cp -rf $(PROJ_ROOT)/board/$(CHIP)/json/$(PRODUCT)/modparam_release.json $(CONFIG_MODPARAM); \
	fi;

	# set env
	echo export PATH=\$$PATH:\/$(notdir $(CONFIG_FILES_DIR))\:/etc >> ${OUTPUTDIR}/rootfs/etc/profile

	#echo export TERMINFO=\/$(notdir $(CONFIG_FILES_DIR))/terminfo >> ${OUTPUTDIR}/rootfs/etc/profile
	echo export LD_LIBRARY_PATH=\$$LD_LIBRARY_PATH:/customer\/lib:$(miservice$(MOUNTTG))\/lib >> $(OUTPUTDIR)/rootfs/etc/profile;
	echo mdev -s >> $(OUTPUTDIR)/rootfs/etc/profile
	# init.sh
	if [ -f $(INIT_FILE) ]; then \
		rm  $(INIT_FILE); \
	fi;
	touch $(INIT_FILE)
	chmod 755 $(INIT_FILE)
	echo "if [ -e /etc/init.sh ]; then" >> $(OUTPUTDIR)/rootfs/etc/profile
	echo "    /etc/init.sh;" >> $(OUTPUTDIR)/rootfs/etc/profile
	echo "fi;" >> $(OUTPUTDIR)/rootfs/etc/profile

	# mount block
	$(foreach block, $(USR_MOUNT_BLOCKS), mkdir -p $(OUTPUTDIR)/rootfs/$($(block)$(MOUNTTG));)
	echo >> $(OUTPUTDIR)/rootfs/etc/profile
	echo mount -t tmpfs mdev /dev >> $(OUTPUTDIR)/rootfs/etc/init.d/rcS

	#insmod ubi ubifs ko
	mkdir -p $(OUTPUTDIR)/rootfs/lib/modules/$(KERNEL_VERSION)
	-if [ "$(filter ubifs,$(foreach block, $(MOUNT_BLK), $($(block)$(FSTYPE))))" != "" ]; then \
		if [ "$(MODULES_PATH_UBIFS)" != "" ];then \
			echo "#mount ubifs" >> $(OUTPUTDIR)/rootfs/etc/profile; \
			for mod in $(MODULES_PATH_UBIFS);do\
				echo "insmod /lib/modules/$(KERNEL_VERSION)/$$mod" >> $(OUTPUTDIR)/rootfs/etc/profile;\
			done;\
			cp -rvf $(MODULES_PATH_LIST_UBIFS) $(OUTPUTDIR)/rootfs/lib/modules/$(KERNEL_VERSION);\
		fi;\
	fi;

	if [ "$(filter ubi.ko, $(MODULES_PATH_UBIFS))" != "" ]; then \
		sed -i 's/ubi.ko/ubi.ko mtd=ubia,$(shell printf %d $(FLASH_PG_SIZE))/g' $(OUTPUTDIR)/rootfs/etc/profile; \
	fi;


	echo -e $(foreach block, $(MOUNT_BLK), "mount -t $($(block)$(FSTYPE)) $($(block)$(MOUNTPT)) $($(block)$(MOUNTTG))\n") >> $(OUTPUTDIR)/rootfs/etc/profile

	if [ "$(LFS_MOUNT_BLK)" != "" ]; then \
		$(call pack_littlefs_fuse,$(OUTPUTDIR)/rootfs) \
		$(foreach block,$(LFS_MOUNT_BLK),$(call gen_mount_fs_cmd,$(block),$(OUTPUTDIR)/rootfs/etc/profile,$(OUTPUTDIR)/rootfs)) \
	fi;

	if [ "$(FWFS_MOUNT_BLK)" != "" ]; then \
		$(call pack_firmwarefs_fuse,$(OUTPUTDIR)/rootfs) \
		$(foreach block,$(filter-out misc_rtos,$(FWFS_MOUNT_BLK)),$(call gen_mount_fs_cmd,$(block),$(OUTPUTDIR)/rootfs/etc/profile,$(OUTPUTDIR)/rootfs)) \
	fi;

	find $(OUTPUTDIR)/rootfs/lib/ -name "*.so*" -type f | xargs $(STRIP)  $(STRIP_OPTION);
	echo >> $(OUTPUTDIR)/rootfs/etc/profile

	# demo.sh
	if [ -f $(LATE_INIT_FILE) ]; then \
		rm  $(LATE_INIT_FILE); \
	fi;
	touch $(LATE_INIT_FILE)
	chmod 755 $(LATE_INIT_FILE)
	echo "if [ -e $(miservice$(MOUNTTG))/demo.sh ]; then" >> $(OUTPUTDIR)/rootfs/etc/profile
	echo "    $(miservice$(MOUNTTG))/demo.sh" >> $(OUTPUTDIR)/rootfs/etc/profile
	echo "fi;" >> $(OUTPUTDIR)/rootfs/etc/profile

	# pack 1st modules
	-if [ "$(MODULES_LIST_FIRST)" != "" ];then \
		for mod in $(MODULES_LIST_FIRST);do\
			echo "insmod /lib/modules/$(KERNEL_VERSION)/$$mod" >> $(INIT_FILE);\
		done;\
		cp -rvf $(MODULES_PATH_LIST_FIRST) $(OUTPUTDIR)/rootfs/lib/modules/$(KERNEL_VERSION);\
		echo "#mi module" >> $(INIT_FILE); \
	fi;
	if [ "$(SENSOR_LIST)" != "" ]; then \
		cp -rvf $(foreach n,$(SENSOR_LIST),$(RELEASE_ROOT)/modules/$(KERNEL_VERSION)/$(n)) $(OUTPUTDIR)/rootfs/lib/modules/$(KERNEL_VERSION); \
	fi;
	find $(OUTPUTDIR)/rootfs/lib/modules/$(KERNEL_VERSION) -name "*.ko" | xargs $(STRIP) $(STRIP_OPTION);

	if [ "$(SENSOR0)" != "" ]; then \
		echo insmod /lib/modules/$(KERNEL_VERSION)/$(SENSOR0) $(SENSOR0_OPT) >> $(INIT_FILE); \
	fi;
	if [ "$(SENSOR1)" != "" ]; then \
		echo insmod /lib/modules/$(KERNEL_VERSION)/$(SENSOR1) $(SENSOR1_OPT) >> $(INIT_FILE); \
	fi;
	if [ "$(SENSOR2)" != "" ]; then \
		echo insmod /lib/modules/$(KERNEL_VERSION)/$(SENSOR2) $(SENSOR2_OPT) >> $(INIT_FILE); \
	fi;

	sed -i 's/mi_common.ko/mi_common.ko g_ModParamPath=\/$(notdir $(CONFIG_FILES_DIR))\/modparam.json config_json_path=\/$(notdir $(CONFIG_FILES_DIR))\/config.json/g' $(INIT_FILE);
	sed -i 's/sstar_mi.ko/sstar_mi.ko common_init.g_ModParamPath=\/$(notdir $(CONFIG_FILES_DIR))\/modparam.json config_json_path=\/$(notdir $(CONFIG_FILES_DIR))\/config.json/g' $(INIT_FILE);

	# pack 2nd modules
	mkdir -p $(miservice$(RESOURCE))/modules/$(KERNEL_VERSION)
	-if [ "$(MODULES_LIST_SECOND)" != "" ];then \
		for mod in $(MODULES_LIST_SECOND);do\
			echo "insmod $(miservice$(MOUNTTG))/modules/$(KERNEL_VERSION)/$$mod" >> $(LATE_INIT_FILE);\
		done;\
		cp -rvf $(MODULES_PATH_LIST_SECOND) $(miservice$(RESOURCE))/modules/$(KERNEL_VERSION);\
		find $(miservice$(RESOURCE))/modules/$(KERNEL_VERSION) -name "*.ko" | xargs $(STRIP) $(STRIP_OPTION); \
	fi;

	# start demo
	mkdir -p $(DEMO_RELEASE_DIR)
	$(call release_demo)

ramdisk:
misc:
customer:
miservice:

