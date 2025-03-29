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
.PHONY: ramdisk

ifneq ($(ramdisk$(RESOURCE)),)
ramdisk:$(FLASH_TYPE)_ramdisk
else
ramdisk:
	@echo NO RAMDISK RES!
endif

RAMDISK_DIR?=$(OUTPUTDIR)/tmprd
RAMDISK_IMG?=$(ramdisk$(RESOURCE))

LFS_MOUNT_BLK := $(foreach m, $(filter %_lfs, $(foreach block, $(USR_MOUNT_BLOCKS), $(block)_$($(block)$(FSTYPE)))), $(patsubst %_lfs, %, $(m)))
FWFS_MOUNT_BLK := $(foreach m, $(filter %_fwfs, $(foreach block, $(USR_MOUNT_BLOCKS), $(block)_$($(block)$(FSTYPE)))), $(patsubst %_fwfs, %, $(m)))
LWFS_MOUNT_BLK := $(foreach m, $(filter %_lwfs, $(foreach block, $(USR_MOUNT_BLOCKS), $(block)_$($(block)$(FSTYPE)))), $(patsubst %_lwfs, %, $(m)))

%_ramdisk:
	@echo [================= $@ =================]

	# create ramdisk
	rm -rf $(RAMDISK_DIR)
	mkdir -p $(RAMDISK_DIR)
	cd $(RAMDISK_DIR); cpio -i -F $(RAMDISK_IMG)

	# rename linuxrc to init
	cp -R $(OUTPUTDIR)/tmprd/linuxrc $(OUTPUTDIR)/tmprd/init;

	# console done
	@echo "#!/bin/sh" > $(OUTPUTDIR)/tmprd/etc/init.d/rcS
	@echo "echo Console done" >> $(OUTPUTDIR)/tmprd/etc/init.d/rcS
	@echo "mount -a" >> $(OUTPUTDIR)/tmprd/etc/init.d/rcS
#
# when enable rtos asan(DEBUG&2048), not allow linux ocupy the rtos core #
#
	if [ "$(DUAL_OS)" = "on" ] && ([ -z "$(DEBUG)" ] || [ $$(($(DEBUG) & 2048)) -eq 0 ]); then \
		echo "echo 1 > /sys/devices/system/cpu/cpu1/online" >> $(OUTPUTDIR)/tmprd/etc/init.d/rcS; \
	fi;\

	rm -rf $(OUTPUTDIR)/tmprd/usr/lib/modules/*

	if [ "$(PURELINUX_FASTBOOT_ENABLE)" == "y" ]; then \
		if [ "$(COMBINE_MI_MODULE)" != "y" ]; then \
			if [ -f "$(MI_MOD_LIST)" ]; then \
				$(call pack_mods,$(MI_MOD_LIST),$(LIB_DIR_PATH)/modules/$(KERNEL_VERSION),$(OUTPUTDIR)/tmprd/usr/lib/modules) \
			fi; \
			$(call gen_mi_mods_insmod_cmd,$(MI_MOD_LIST),/usr/lib/modules,$(OUTPUTDIR)/tmprd/etc/init.d/rcS) \
		else \
			cp -vf $(LIB_DIR_PATH)/modules/$(KERNEL_VERSION)/$(COMBINE_MODULE_NAME).ko $(OUTPUTDIR)/tmprd/usr/lib/modules; \
			echo "insmod /usr/lib/modules/$(COMBINE_MODULE_NAME).ko" common_init.g_ModParamPath=/config/modparam.json >> $(OUTPUTDIR)/tmprd/etc/init.d/rcS; \
		fi; \
		sed -i 's/mi_common.ko/mi_common.ko g_ModParamPath=\/config\/modparam.json/g' $(OUTPUTDIR)/tmprd/etc/init.d/rcS; \
		cp -rvf $(foreach n,$(SENSOR_LIST),$(LIB_DIR_PATH)/modules/$(KERNEL_VERSION)/$(n)) $(OUTPUTDIR)/tmprd/usr/lib/modules; \
		$(call gen_snr_mods_insmod_cmd,/usr/lib/modules,$(OUTPUTDIR)/tmprd/etc/init.d/rcS) \
		if [ -n "$(CUST_MODPARAM)" ]; then \
			cp -rf $(PROJ_ROOT)/board/$(CHIP)/json/$(PRODUCT)/$(CUST_MODPARAM) $(OUTPUTDIR)/tmprd/config/modparam.json; \
		else \
			cp -rf $(PROJ_ROOT)/board/$(CHIP)/json/$(PRODUCT)/modparam_release.json $(OUTPUTDIR)/tmprd/config/modparam.json; \
		fi; \
		$(PREFIX)strip $(STRIP_OPTION) $(OUTPUTDIR)/tmprd/usr/lib/modules/*; \
	fi;

	# pack modules
	if [ -e $(KBUILD_CUST)/ramdisk_rc1 ]; then \
		$(call pack_mods,$(KBUILD_CUST)/ramdisk_rc1,$(KERN_MODS_PATH),$(OUTPUTDIR)/tmprd/usr/lib/modules) \
		$(call pack_mods,$(KBUILD_CUST)/ramdisk_rc2,$(KERN_MODS_PATH),$(OUTPUTDIR)/tmprd/usr/lib/modules) \
		$(PREFIX)strip $(STRIP_OPTION) $(OUTPUTDIR)/tmprd/usr/lib/modules/*; \
		$(call gen_mods_insmod_cmd,$(KBUILD_CUST)/ramdisk_rc1,/usr/lib/modules,$(OUTPUTDIR)/tmprd/etc/init.d/rcS) \
		$(call gen_mods_insmod_cmd,$(KBUILD_CUST)/ramdisk_rc2,/usr/lib/modules,$(OUTPUTDIR)/tmprd/etc/init.d/rcS) \
	elif [ -e $(KBUILD_CUST)/$(FLASH_TYPE)/ramdisk_rc1 ]; then \
		$(call pack_mods,$(KBUILD_CUST)/$(FLASH_TYPE)/ramdisk_rc1,$(KERN_MODS_PATH),$(OUTPUTDIR)/tmprd/usr/lib/modules) \
		$(call pack_mods,$(KBUILD_CUST)/$(FLASH_TYPE)/ramdisk_rc2,$(KERN_MODS_PATH),$(OUTPUTDIR)/tmprd/usr/lib/modules) \
		$(PREFIX)strip $(STRIP_OPTION) $(OUTPUTDIR)/tmprd/usr/lib/modules/*; \
		$(call gen_mods_insmod_cmd,$(KBUILD_CUST)/$(FLASH_TYPE)/ramdisk_rc1,/usr/lib/modules,$(OUTPUTDIR)/tmprd/etc/init.d/rcS) \
		$(call gen_mods_insmod_cmd,$(KBUILD_CUST)/$(FLASH_TYPE)/ramdisk_rc2,/usr/lib/modules,$(OUTPUTDIR)/tmprd/etc/init.d/rcS) \
	fi;

	# mount first
	if [ "$(FLASH_TYPE)" = "spinand" ]; then \
		echo "ubiattach /dev/ubi_ctrl -m $(UBI_AT_MTD)" >> $(OUTPUTDIR)/tmprd/etc/init.d/rcS ; \
	fi;
	if [ "$(FLASH_TYPE)" = "emmc" ]; then \
		echo "echo 1 > $(rootfs$(DEVNODE))/sdmmc_wait_rescan" >> $(OUTPUTDIR)/tmprd/etc/init.d/rcS ; \
	fi;
	echo mount -t $(rootfs$(FSTYPE)) -o ro $(rootfs$(MOUNTPT)) $(rootfs$(MOUNTTG)) >> $(OUTPUTDIR)/tmprd/etc/init.d/rcS
	echo mount -t $(miservice$(FSTYPE)) $(miservice$(MOUNTPT)) $(miservice$(MOUNTTG)) >> $(OUTPUTDIR)/tmprd/etc/init.d/rcS

	# fast demo
	if [ "$(FAST_DEMO)" == "on" ]; then \
		mkdir -p $(OUTPUTDIR)/rootfs/ttlv ; \
        cp -rf $(PROJ_ROOT)/board/$(CHIP)/rtos/ttlv_demo.sh $(OUTPUTDIR)/rootfs/ttlv/ ; \
        cp -rf $(LIB_DIR_PATH)/bin/mi_demo/prog_rtos_preload $(OUTPUTDIR)/rootfs/ttlv/ ; \
	    echo "export LD_LIBRARY_PATH=\$$LD_LIBRARY_PATH:/usr/ttlv:/config/lib" >> ${OUTPUTDIR}/tmprd/etc/init.d/rcS; \
	    echo "export PATH=/usr/bin:/usr/sbin:/bin:/sbin:/usr/usr/bin:/usr/usr/sbin:/usr/ttlv" >> $(OUTPUTDIR)/tmprd/etc/init.d/rcS; \
	    echo "/usr/bin/busybox sh /usr/ttlv/ttlv_demo.sh" >> $(OUTPUTDIR)/tmprd/etc/init.d/rcS; \
	fi;

	# deferred initcall
	echo "cat /proc/deferred_initcalls" >> $(OUTPUTDIR)/tmprd/etc/init.d/rcS;

	# mount second
	echo mount -t $(customer$(FSTYPE)) $(customer$(MOUNTPT)) $(customer$(MOUNTTG)) >> $(OUTPUTDIR)/tmprd/etc/init.d/rcS
	@echo -e mdev -s >> $(OUTPUTDIR)/tmprd/etc/init.d/rcS

	$(foreach block,$(LWFS_MOUNT_BLK),$(call gen_mount_fs_cmd,$(block),$(RAMDISK_DIR)/etc/init.d/rcS,$(RAMDISK_DIR)))
	if [ "$(LFS_MOUNT_BLK)" != "" ]; then \
		$(call pack_littlefs_fuse,$(OUTPUTDIR)/rootfs) \
		$(foreach block,$(LFS_MOUNT_BLK),$(call gen_mount_fs_cmd,$(block),$(RAMDISK_DIR)/etc/init.d/rcS,$(RAMDISK_DIR))) \
	fi;

	if [ "$(FWFS_MOUNT_BLK)" != "" ]; then \
		$(call pack_firmwarefs_fuse,$(OUTPUTDIR)/rootfs) \
		$(foreach block,$(filter-out misc_rtos,$(FWFS_MOUNT_BLK)),$(call gen_mount_fs_cmd,$(block),$(RAMDISK_DIR)/etc/init.d/rcS,$(RAMDISK_DIR))) \
	fi;

	@echo "mkdir -p /dev/shm" >> $(OUTPUTDIR)/tmprd/etc/init.d/rcS
	@echo "mkdir -p /dev/pts" >> $(OUTPUTDIR)/tmprd/etc/init.d/rcS
	@echo "mount -t devpts for_telnetd /dev/pts" >> $(OUTPUTDIR)/tmprd/etc/init.d/rcS
	@echo "mount --bind /usr/bin/sh /bin/sh" >> $(OUTPUTDIR)/tmprd/etc/init.d/rcS
	if [ "$(DUAL_OS)" = "on" ]; then \
		echo -e 'if [ -f /proc/dualos/rtos ]; then\n    echo cli mount proxyfs none / > /proc/dualos/rtos\nfi' >> $(OUTPUTDIR)/tmprd/etc/init.d/rcS; \
	fi;

	# pack fw_printenv
	@echo "mkdir -p /var/lock" >> $(OUTPUTDIR)/tmprd/etc/init.d/rcS
	cp -rvf $(PROJ_ROOT)/tools/$(TOOLCHAIN)/$(TOOLCHAIN_VERSION)/fw_printenv/* $(OUTPUTDIR)/tmprd/etc/
	echo "$(ENV_CFG)" > $(OUTPUTDIR)/tmprd/etc/fw_env.config
	if [ "$(ENV_TYPE)" == "dualenv" ]; then \
		echo "$(ENV1_CFG)" >> $(OUTPUTDIR)/tmprd/etc/fw_env.config; \
	fi;
	cd $(OUTPUTDIR)/tmprd/etc/;ln -sf fw_printenv fw_setenv;chmod 777 fw_printenv

	# set env
	@echo "#!/bin/sh" > $(OUTPUTDIR)/tmprd/etc/profile
	@echo "export PATH=/usr/bin:/usr/sbin:/bin:/sbin:/usr/usr/bin:/usr/usr/sbin" >> $(OUTPUTDIR)/tmprd/etc/profile
	@echo "export LD_LIBRARY_PATH=/lib" >> ${OUTPUTDIR}/tmprd/etc/profile;
	@echo "ulimit -c unlimited" >> ${OUTPUTDIR}/tmprd/etc/profile;
	@echo "export PATH=\$$PATH:/config" >> ${OUTPUTDIR}/tmprd/etc/profile;
	@echo "export TERMINFO=/config/terminfo" >> ${OUTPUTDIR}/tmprd/etc/profile
	@echo "export LD_LIBRARY_PATH=\$$LD_LIBRARY_PATH:/config/lib" >> ${OUTPUTDIR}/tmprd/etc/profile;
	if [ "$(OPTEE_CONFIG)"x != ""x  ]; then \
		echo export LD_LIBRARY_PATH=\$$LD_LIBRARY_PATH:/customer/tee/lib >> ${OUTPUTDIR}/tmprd/etc/profile ;\
	fi;

	# telnet
	@echo "/usr/usr/sbin/telnetd -l /usr/bin/ash" >> $(OUTPUTDIR)/tmprd/etc/profile

	# demo.sh
	@echo "/usr/bin/busybox sh /customer/demo.sh" >> $(OUTPUTDIR)/tmprd/etc/profile

	# pack ramdisk
	cd $(RAMDISK_DIR); find | cpio -o -H newc -O $(RAMDISK_IMG)
