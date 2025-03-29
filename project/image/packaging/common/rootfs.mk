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
.PHONY: rootfs

rootfs_$(PRODUCT):
rootfs: rootfs_$(PRODUCT)

LFS_MOUNT_BLK := $(foreach m, $(filter %_lfs, $(foreach block, $(USR_MOUNT_BLOCKS), $(block)_$($(block)$(FSTYPE)))), $(patsubst %_lfs, %, $(m)))
FWFS_MOUNT_BLK := $(foreach m, $(filter %_fwfs, $(foreach block, $(USR_MOUNT_BLOCKS), $(block)_$($(block)$(FSTYPE)))), $(patsubst %_fwfs, %, $(m)))
MOUNT_BLK := $(filter-out $(LFS_MOUNT_BLK), $(USR_MOUNT_BLOCKS))
MOUNT_BLK := $(filter-out $(FWFS_MOUNT_BLK), $(MOUNT_BLK))

ifeq ($(PRODUCT), android)
ifneq ($(findstring aarch64, $(NDK_CLANG_PREFIX)),)
LIBC=$(strip $(patsubst %.tar.gz, %, $(word 1, $(notdir $(wildcard $(LIB_DIR_PATH)/package/libc_64-*.tar.gz)))))
LINKER=linker64
else
LIBC=$(strip $(patsubst %.tar.gz, %, $(word 1, $(notdir $(wildcard $(LIB_DIR_PATH)/package/libc_32-*.tar.gz)))))
LINKER=linker
endif

else
ifeq ($(TOOLCHAIN), uclibc)
LIBC=$(strip $(patsubst %.tar.gz, %, $(word 1, $(notdir $(wildcard $(LIB_DIR_PATH)/package/libuclibc-*.tar.gz)))))
else
LIBC=$(strip $(patsubst %.tar.gz, %, $(word 1, $(notdir $(wildcard $(LIB_DIR_PATH)/package/libc-*.tar.gz)))))
endif
endif
KERNELRELEASE = $(shell cat $(PROJ_ROOT)/kbuild/$(KERNEL_VERSION)/include/config/kernel.release 2> /dev/null)

rootfs_$(PRODUCT):
	@echo [================= $@ =================]

	# pack rootfs
	cd rootfs;tar xf rootfs.tar.gz -C $(OUTPUTDIR)

	# pack busybox
	tar xf busybox/$(BUSYBOX).tar.gz -C $(OUTPUTDIR)/rootfs

	# pack libc
	tar xf $(LIB_DIR_PATH)/package/$(LIBC).tar.gz -C $(OUTPUTDIR)/rootfs/lib

        #pack /mnt/mmc /mnt/nfs UI config
	if [ "$(PRODUCT)" == "cardv" ]; then \
		mkdir -p  $(OUTPUTDIR)/rootfs/mnt/nfs; \
		mkdir -p  $(OUTPUTDIR)/rootfs/mnt/mmc; \
		cp -rf $(LIB_DIR_PATH)/bin/UI/etc/EasyUI.cfg ${OUTPUTDIR}/rootfs/etc/; \
	        cp -rf $(LIB_DIR_PATH)/bin/UI/etc/ts.conf ${OUTPUTDIR}/rootfs/etc/; \
	fi;

	# pack base scripts
	cp -rf etc/* $(OUTPUTDIR)/rootfs/etc
	if [ -d $(PROJ_ROOT)/board/$(CHIP)/dspfile ]; then \
		mkdir -p  $(OUTPUTDIR)/rootfs/lib/firmware; \
		cp  $(PROJ_ROOT)/board/$(CHIP)/dspfile/* $(OUTPUTDIR)/rootfs/lib/firmware; \
	fi;
	# pack fw_printenv
	cp -rvf $(PROJ_ROOT)/tools/$(TOOLCHAIN)/$(TOOLCHAIN_VERSION)/fw_printenv/* $(OUTPUTDIR)/rootfs/etc/
	echo "$(ENV_CFG)" > $(OUTPUTDIR)/rootfs/etc/fw_env.config
	if [ "$(ENV_TYPE)" == "dualenv" ]; then \
		echo "$(ENV1_CFG)" >> $(OUTPUTDIR)/rootfs/etc/fw_env.config; \
	fi;
	cd $(OUTPUTDIR)/rootfs/etc/;ln -sf fw_printenv fw_setenv;chmod 777 fw_printenv

	# set env
	echo export PATH=\$$PATH:/config >> ${OUTPUTDIR}/rootfs/etc/profile
	echo export TERMINFO=/config/terminfo >> ${OUTPUTDIR}/rootfs/etc/profile
	echo export LD_LIBRARY_PATH=\$$LD_LIBRARY_PATH:/config/lib >> ${OUTPUTDIR}/rootfs/etc/profile
	if [ "$(OPTEE_CONFIG)"x != ""x  ]; then \
		echo export LD_LIBRARY_PATH=\$$LD_LIBRARY_PATH:/customer/tee/lib >> ${OUTPUTDIR}/rootfs/etc/profile ;\
	fi;
	if [ "$(PRODUCT)" == "cardv" ]; then \
		echo export LD_LIBRARY_PATH=\$$LD_LIBRARY_PATH:/customer/UI/lib:/customer/wifi >> ${OUTPUTDIR}/rootfs/etc/profile ;\
		echo export PATH=\$$PATH:/customer/cardv:/customer/UI/bin:/customer/wifi >> ${OUTPUTDIR}/rootfs/etc/profile; \
		if [ "$(SIGMA_WIFI)" != "" ]; then \
			echo "export WIFI_MODULE=$(SIGMA_WIFI)" >> ${OUTPUTDIR}/rootfs/etc/profile; \
		fi; \
	fi;

	# mount block
	echo mount -t tmpfs mdev /dev >> ${OUTPUTDIR}/rootfs/etc/init.d/rcS
	$(foreach block, $(MOUNT_BLK), mkdir -p $(OUTPUTDIR)/rootfs/$($(block)$(MOUNTTG));)
	echo -e mdev -s >> $(OUTPUTDIR)/rootfs/etc/init.d/rcS
	if [ "$(FLASH_TYPE)"x = "emmc"x  ]; then \
		if [[ $(TOOLCHAIN_VERSION) = "10.2.1" ]]; then \
			cp -f $(PROJ_ROOT)/tools/glibc/$(TOOLCHAIN_VERSION)/mkfs.ext4 $(OUTPUTDIR)/rootfs/sbin/ ;\
			cp -f $(PROJ_ROOT)/tools/glibc/$(TOOLCHAIN_VERSION)/resize2fs $(OUTPUTDIR)/rootfs/sbin/ ;\
		else \
			cp -f $(PROJ_ROOT)/tools/mkfs.ext4 $(OUTPUTDIR)/rootfs/sbin/ ;\
			cp -f $(PROJ_ROOT)/tools/resize2fs $(OUTPUTDIR)/rootfs/sbin/ ;\
			cp -f $(PROJ_ROOT)/tools/fio $(OUTPUTDIR)/rootfs/sbin/ ;\
		fi; \
	fi;
	if [ $(PZ1) != 1  ]; then \
		echo -e $(foreach block, $(MOUNT_BLK), "resize2fs $($(block)$(MOUNTPT))\n") >> $(OUTPUTDIR)/rootfs/etc/init.d/rcS;\
		echo -e $(foreach block, $(MOUNT_BLK), "mount -t $($(block)$(FSTYPE)) $($(block)$(MOUNTPT)) $($(block)$(MOUNTTG)) $($(block)$(MOUNTPARAM))\n") >> $(OUTPUTDIR)/rootfs/etc/init.d/rcS ;\
	fi;

	if [ "$(LFS_MOUNT_BLK)" != "" ]; then \
		$(call pack_littlefs_fuse,$(OUTPUTDIR)/rootfs) \
		$(foreach block,$(LFS_MOUNT_BLK),$(call gen_mount_fs_cmd,$(block),$(OUTPUTDIR)/rootfs/etc/init.d/rcS,$(OUTPUTDIR)/rootfs)) \
	fi;

	if [ "$(FWFS_MOUNT_BLK)" != "" ]; then \
		$(call pack_firmwarefs_fuse,$(OUTPUTDIR)/rootfs) \
		$(foreach block,$(filter-out misc_rtos,$(FWFS_MOUNT_BLK)),$(call gen_mount_fs_cmd,$(block),$(OUTPUTDIR)/rootfs/etc/init.d/rcS,$(OUTPUTDIR)/rootfs)) \
	fi;

	# telnet
	echo mkdir -p /dev/pts >> $(OUTPUTDIR)/rootfs/etc/init.d/rcS
	echo mount -t devpts devpts /dev/pts >> $(OUTPUTDIR)/rootfs/etc/init.d/rcS
	echo "telnetd &" >> $(OUTPUTDIR)/rootfs/etc/init.d/rcS

	# demo.sh
	echo "if [ -e /customer/demo.sh ]; then" >> $(OUTPUTDIR)/rootfs/etc/init.d/rcS
	echo "    /customer/demo.sh" >> $(OUTPUTDIR)/rootfs/etc/init.d/rcS
	echo "fi;" >> $(OUTPUTDIR)/rootfs/etc/init.d/rcS

	# sshd
	if [ "$(FLASH_TYPE)" = "spinand" ]; then \
		if [[ $(TOOLCHAIN_VERSION) = "9.1.0" ]] || [[ $(TOOLCHAIN_VERSION) = "8.2.1" ]] || [[ $(TOOLCHAIN_VERSION) = "6.4.0" ]]; then \
			echo "sshd:x:74:74:Privilege-separated SSH:/var/empty/sshd:/sbin/nologin" >> $(OUTPUTDIR)/rootfs/etc/passwd; \
			echo "export LD_LIBRARY_PATH=\$$LD_LIBRARY_PATH:/customer/ssh/lib" >> ${OUTPUTDIR}/rootfs/etc/init.d/rcS; \
			echo "mkdir /var/empty" >> ${OUTPUTDIR}/rootfs/etc/init.d/rcS; \
			echo "/customer/ssh/sbin/sshd -f /customer/ssh/etc/sshd_config" >> ${OUTPUTDIR}/rootfs/etc/init.d/rcS; \
			echo "export LD_LIBRARY_PATH=\$$LD_LIBRARY_PATH:/customer/ssh/lib" >> ${OUTPUTDIR}/rootfs/etc/profile; \
		fi; \
	fi;

	# set default password NULL
	echo "root::0:0:Linux User,,,:/home/root:/bin/sh" >> $(OUTPUTDIR)/rootfs/etc/passwd;

	# create symbol link
	mkdir -p $(OUTPUTDIR)/rootfs/lib/modules/
	ln -fs /config/modules/$(KERNEL_VERSION) $(OUTPUTDIR)/rootfs/lib/modules/$(KERNELRELEASE)

	if [ "$(PRODUCT)" = "android" ]; then \
		mkdir -p $(OUTPUTDIR)/rootfs/system/bin; \
		cp -f $(PROJ_ROOT)/tools/$(TOOLCHAIN)/$(TOOLCHAIN_VERSION)/linker/$(LINKER) $(OUTPUTDIR)/rootfs/system/bin/; \
		ln -sf /bin/sh $(OUTPUTDIR)/rootfs/system/bin/sh; \
		mkdir -p $(OUTPUTDIR)/rootfs/linkerconfig/; \
		touch $(OUTPUTDIR)/rootfs/linkerconfig/ld.config.txt; \
	fi;
