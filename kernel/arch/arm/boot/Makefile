#
# arch/arm/boot/Makefile
#
# This file is included by the global makefile so that you can add your own
# architecture-specific flags and dependencies.
#
# This file is subject to the terms and conditions of the GNU General Public
# License.  See the file "COPYING" in the main directory of this archive
# for more details.
#
# Copyright (C) 1995-2002 Russell King
#

OBJCOPYFLAGS	:=-O binary -R .comment -S

ifneq ($(CONFIG_ARM_PATCH_PHYS_VIRT),)
LOADADDR = 0x20008000
else
LOADADDR = $(shell /bin/bash -c 'printf "0x%08x" $$[$(CONFIG_PHYS_OFFSET) + 0x8000]')
endif

MKIMAGE_BIN = $(srctree)/scripts/mkimage
MZ_BIN = $(srctree)/scripts/mz
SZ_BIN = $(srctree)/scripts/sstar_sz.sh
XZ_BIN = xz
KERNEL_RELEASE_FILE = include/config/kernel.release
ROOTFS = $(obj)/ramdisk_linaro4.8_minit.img.xz

ifneq ($(wildcard $(srctree)/arch/arm/boot/dts/$(subst ",,$(CONFIG_SSTAR_DTB_NAME)).dts),)
SSTAR_DTB_NAME=arch/arm/boot/dts/$(subst ",,$(CONFIG_SSTAR_DTB_NAME)).dtb
endif

CHIP_NAME := $(subst ",,$(CONFIG_SSTAR_CHIP_NAME))

ifeq ($(findstring fpga, $(SSTAR_DTB_NAME)), fpga)
SSTAR_FPGA_DTB_NAME = $(SSTAR_DTB_NAME)
else
SSTAR_FPGA_DTB_NAME = arch/arm/boot/dts/$(CHIP_NAME)-fpga.dtb
endif

ifneq ($(MACHINE),)
include $(MACHINE)/Makefile.boot
endif

# Note: the following conditions must always be true:
#   ZRELADDR == virt_to_phys(PAGE_OFFSET + TEXT_OFFSET)
#   PARAMS_PHYS must be within 4MB of ZRELADDR
#   INITRD_PHYS must be in RAM
ZRELADDR    := $(zreladdr-y)
PARAMS_PHYS := $(params_phys-y)
INITRD_PHYS := $(initrd_phys-y)

export ZRELADDR INITRD_PHYS PARAMS_PHYS

targets := Image zImage xipImage bootpImage uImage

ifeq ($(CONFIG_XIP_KERNEL),y)

cmd_deflate_xip_data = $(CONFIG_SHELL) -c \
	'$(srctree)/$(src)/deflate_xip_data.sh $< $@'

ifeq ($(CONFIG_XIP_DEFLATED_DATA),y)
quiet_cmd_mkxip = XIPZ    $@
cmd_mkxip = $(cmd_objcopy) && $(cmd_deflate_xip_data)
else
quiet_cmd_mkxip = $(quiet_cmd_objcopy)
cmd_mkxip = $(cmd_objcopy)
endif

$(obj)/xipImage: vmlinux FORCE
	$(call if_changed,mkxip)
	@$(kecho) '  Physical Address of xipImage: $(CONFIG_XIP_PHYS_ADDR)'

$(obj)/Image $(obj)/zImage: FORCE
	@echo 'Kernel configured for XIP (CONFIG_XIP_KERNEL=y)'
	@echo 'Only the xipImage target is available in this case'
	@false

else

$(obj)/xipImage: FORCE
	@echo 'Kernel not configured for XIP (CONFIG_XIP_KERNEL!=y)'
	@false

$(obj)/Image: vmlinux FORCE
	$(call if_changed,objcopy)

$(obj)/compressed/vmlinux: $(obj)/Image FORCE
	$(Q)$(MAKE) $(build)=$(obj)/compressed $@

$(obj)/zImage:	$(obj)/compressed/vmlinux FORCE
	$(call if_changed,objcopy)

endif

ifneq ($(LOADADDR),)
  UIMAGE_LOADADDR=$(LOADADDR)
else
  ifeq ($(CONFIG_ZBOOT_ROM),y)
    UIMAGE_LOADADDR=$(CONFIG_ZBOOT_ROM_TEXT)
  else
    UIMAGE_LOADADDR=$(ZRELADDR)
  endif
endif

check_for_multiple_loadaddr = \
if [ $(words $(UIMAGE_LOADADDR)) -ne 1 ]; then \
	echo 'multiple (or no) load addresses: $(UIMAGE_LOADADDR)'; \
	echo 'This is incompatible with uImages'; \
	echo 'Specify LOADADDR on the commandline to build an uImage'; \
	false; \
fi

clean-files += uImage.xz uImage.mz Image.xz Image.mz Image-fpga
ifeq ($(CONFIG_ARCH_SSTAR),)
$(obj)/uImage:	$(obj)/zImage FORCE
	@$(check_for_multiple_loadaddr)
	$(call if_changed,uimage)
else
# add dtc flags '-@' to support dtb overlay
DTC_FLAGS_$(basename $(notdir $(SSTAR_DTB_NAME))) := -@
DTC_FLAGS_$(basename $(notdir $(SSTAR_FPGA_DTB_NAME))) := -@
$(obj)/uImage:	$(obj)/Image $(SSTAR_DTB_NAME) $(SSTAR_FPGA_DTB_NAME) FORCE
ifeq ($(CONFIG_SSTAR_BUILTIN_DTB), y)
		#update builtin DTB
		@test "${SSTAR_DTB_NAME}" = "" || \
		if [ -e ${SSTAR_DTB_NAME} ]; then \
			echo "  IMAGE   $(obj)/Image"; \
			echo "  BNDTB ${SSTAR_DTB_NAME}"; \
			python $(srctree)/scripts/ms_builtin_dtb_update.py $(obj)/Image ${SSTAR_DTB_NAME}; \
			echo; \
		fi;
		#update Image-fpga DTB
		@if [ -e ${SSTAR_FPGA_DTB_NAME} ]; then \
			echo "  IMAGE   $(obj)/Image-fpga"; \
			cp -f $(obj)/Image $(obj)/Image-fpga; \
			echo "  BNDTB   ${SSTAR_FPGA_DTB_NAME}"; \
			python $(srctree)/scripts/ms_builtin_dtb_update.py $(obj)/Image-fpga ${SSTAR_FPGA_DTB_NAME}; \
			echo; \
		fi;
endif

ifeq ($(CONFIG_SS_BUILTIN_UNFDT), y)
		#generate dtb2unfdt
		@echo "    Generate dtb2unfdt binary ...";
		@pushd scripts/dtc/dtb2unfdt/; \
		./build_dtb2unfdt.sh; \
		popd;
		@if [ -e scripts/dtc/dtb2unfdt/dtb2unfdt ]; \
		then  \
			chmod 755 scripts/dtc/dtb2unfdt/dtb2unfdt; \
		else \
			printf "\n\033[31m============================================\n"; \
			printf " Generate dtb2unfdt failed !!!\n\n"; \
			printf " Please check if 32bit glibc installed.\n\n"; \
			printf " If not, install by the following command:\n\n"; \
			printf "   sudo apt-get install gcc-multilib\n"; \
			printf "============================================\033[0m\n\n"; \
			exit 1; \
		fi
		#update builtin UNFDT
		@test "${SSTAR_DTB_NAME}" = "" || \
		if [ -e ${SSTAR_DTB_NAME} ]; then \
			echo "  BNDTB ${SSTAR_DTB_NAME}"; \
			scripts/dtc/dtb2unfdt/dtb2unfdt ${SSTAR_DTB_NAME}  $(obj)/unfdt.bin; \
			python $(srctree)/scripts/ms_builtin_unfdt_update.py $(obj)/Image $(obj)/unfdt.bin; \
			echo; \
		fi;
endif

ifneq ($(wildcard $(srctree)/scripts/ss_padmux_check.py),)
		@echo "$(srctree)/scripts/dtc/dtx_diff ${SSTAR_DTB_NAME} > $(CONFIG_SSTAR_DTB_NAME).dts"
		@$(srctree)/scripts/dtc/dtx_diff ${SSTAR_DTB_NAME} > $(CONFIG_SSTAR_DTB_NAME).dts
ifneq ($(wildcard drivers/sstar/gpio/$(CHIP_NAME)/hal_pinmux.o),)
		@$(CROSS_COMPILE)objdump -D -j .rodata drivers/sstar/gpio/$(CHIP_NAME)/hal_pinmux.o > pinmux_table
		@python $(srctree)/scripts/ss_padmux_check.py $(CONFIG_SSTAR_DTB_NAME).dts pinmux_table
		@rm pinmux_table
endif
		@rm $(CONFIG_SSTAR_DTB_NAME).dts
endif

ifneq ($(wildcard scripts/ss_puse_check.py),)
		@python scripts/ss_puse_check.py drivers/sstar/include/drv_puse.h
endif

	#update the image size into Image
	@python $(srctree)/scripts/ms_bin_option_update_int.py  $(obj)/Image '#IMG_SZ#' $$(stat -c %s $(obj)/Image)
	#build uImage
	@IMGNAME=$(shell /usr/bin/strings -a -T binary $(obj)/Image | grep 'MVX' | grep 'LX'  | sed 's/\\*MVX/MVX/g' | cut -c 1-32 ); \
	if [ -e ${MKIMAGE_BIN} ]; then \
		${MKIMAGE_BIN} -A arm -O linux -T kernel -C none -a ${LOADADDR} -e ${LOADADDR} -n $${IMGNAME} -d $(obj)/Image $(obj)/uImage; \
		echo '$(IMGNAME)'; \
		echo 'Compress Kernel Image'; \
		${MZ_BIN} c  $(obj)/Image  $(obj)/Image.mz ; \
		${SZ_BIN} -d $(obj)/uImage -b 4; \
		${XZ_BIN} -z -k -f $(obj)/Image; \
		echo; \
		${MKIMAGE_BIN} -A arm -O linux -C lzma -a ${LOADADDR} -e ${LOADADDR} -n $${IMGNAME} -d $(obj)/Image.xz $(obj)/uImage.xz; \
		echo; \
		${MKIMAGE_BIN} -A arm -O linux -C mz -a ${LOADADDR} -e ${LOADADDR} -n $${IMGNAME} -d $(obj)/Image.mz $(obj)/uImage.mz; \
		echo; \
		${MKIMAGE_BIN} -A arm -O linux -C lzma2 -a $(shell awk 'BEGIN{printf("%#x", '${LOADADDR}-0x40')}') -e 0 -n $${IMGNAME} -d $(obj)/uImage.sz $(obj)/u1Image.sz; \
		mv $(obj)/u1Image.sz $(obj)/uImage.sz; \
		echo; \
		lz4 -9 -f -q $(obj)/Image $(obj)/Image.lz4; \
		${MKIMAGE_BIN} -A arm -O linux -C lz4 -a ${LOADADDR} -e ${LOADADDR} -n $${IMGNAME} -d $(obj)/Image.lz4 $(obj)/uImage.lz4; \
		echo; \
	else \
		echo ">> Can't find $${MKIMAGE_BIN}. <<"; \
	fi; \
	if [ -e $(ROOTFS) ]; then \
		${MKIMAGE_BIN} -A arm -O linux -T multi -C none -a ${LOADADDR} -e ${LOADADDR} -n $${IMGNAME} -d $(obj)/Image:$(ROOTFS) $(obj)/kernel.img; \
		echo; \
		${MKIMAGE_BIN} -A arm -O linux -T multi -C lzma -a ${LOADADDR} -e ${LOADADDR} -n $${IMGNAME} -d $(obj)/Image.xz:$(ROOTFS) $(obj)/kernel.xz.img; \
		echo; \
		${MKIMAGE_BIN} -A arm -O linux -T multi -C mz -a ${LOADADDR} -e ${LOADADDR} -n $${IMGNAME} -d $(obj)/Image.mz:$(ROOTFS) $(obj)/kernel.mz.img; \
		echo; \
		${MKIMAGE_BIN} -A arm -O linux -T multi -C lz4 -a ${LOADADDR} -e ${LOADADDR} -n $${IMGNAME} -d $(obj)/Image.lz4:$(ROOTFS) $(obj)/kernel.lz4.img; \
		echo; \
	fi;
	@echo
endif

$(obj)/bootp/bootp: $(obj)/zImage initrd FORCE
	$(Q)$(MAKE) $(build)=$(obj)/bootp $@

$(obj)/bootpImage: $(obj)/bootp/bootp FORCE
	$(call if_changed,objcopy)

PHONY += initrd install zinstall uinstall
initrd:
	@test "$(INITRD_PHYS)" != "" || \
	(echo This machine does not support INITRD; exit -1)
	@test "$(INITRD)" != "" || \
	(echo You must specify INITRD; exit -1)

install:
	$(CONFIG_SHELL) $(srctree)/$(src)/install.sh "$(KERNELRELEASE)" \
	$(obj)/Image System.map "$(INSTALL_PATH)"

zinstall:
	$(CONFIG_SHELL) $(srctree)/$(src)/install.sh "$(KERNELRELEASE)" \
	$(obj)/zImage System.map "$(INSTALL_PATH)"

uinstall:
	$(CONFIG_SHELL) $(srctree)/$(src)/install.sh "$(KERNELRELEASE)" \
	$(obj)/uImage System.map "$(INSTALL_PATH)"

subdir-	    := bootp compressed dts
