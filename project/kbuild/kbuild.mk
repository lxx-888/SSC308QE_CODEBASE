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
.PHONY: symbol_link symbol_link_clean linux-kernel linux-kernel_clean FORCE
-include configs/current.configs

ifeq ("$(PRODUCT)", "android")
.PHONY: aosp_common_kernel
KERNEL_ROOT = $(PROJ_ROOT)/../kernel/common
    ifeq ($(FPGA),1)
        LTO ?= none
    else
        LTO ?= thin
    endif

    ifeq ("$(ack_build_config)", "")
        ACK_BUILD_CONFIG = $(KERNEL_CONFIG)
    else
        ACK_BUILD_CONFIG = $(ack_build_config)
    endif

    ifeq ("$(ack_out_dir)", "")
        ACK_OUT_DIR = "out"
        KERNEL_OUTPUT_DIR = $(KERNEL_ROOT)/../out/common
    else
        ACK_OUT_DIR = $(ack_out_dir)
        KERNEL_OUTPUT_DIR = $(ACK_OUT_DIR)/common
    endif

    ifeq ("$(ack_dist_dir)", "")
        ACK_DIST_DIR = "out/dist"
        KERNEL_DIST_DIR = $(KERNEL_ROOT)/../out/dist
    else
        ACK_DIST_DIR = $(ack_dist_dir)
        KERNEL_DIST_DIR = $(ack_dist_dir)
    endif
else
KERNEL_ROOT= $(PROJ_ROOT)/../kernel
endif

KERNEL_DIR= $(KERNEL_ROOT)/arch/$(ARCH)/configs
KERNEL_MK= $(KERNEL_ROOT)/Makefile

KBUILD_ROOT = $(PROJ_ROOT)/kbuild/$(KERNEL_VERSION)
DEBUG_ASAN:=$(shell if [[ "$(DEBUG)X" != "X"  ]] && ((( $(DEBUG) & 256 )) || (( $(DEBUG) & 1 ))); then echo "1"; fi)
ifneq ($(DEBUG_ASAN), )
KERNEL_LIB_RELEASE_DIR = $(PROJ_ROOT)/release/chip/$(CHIP)/sigma_common_libs/$(TOOLCHAIN)/$(TOOLCHAIN_VERSION)/debug
else
KERNEL_LIB_RELEASE_DIR = $(PROJ_ROOT)/release/chip/$(CHIP)/sigma_common_libs/$(TOOLCHAIN)/$(TOOLCHAIN_VERSION)/release
endif

linux-kernel-debug:
	@echo "build kernel: MAKEFLAGS <$(MAKEFLAGS)>"
	@if [[ -f $(KERNEL_DIR)/$(KERNEL_CONFIG) ]]; then \
		cp -rf $(KERNEL_DIR)/$(KERNEL_CONFIG) $(KERNEL_DIR)/$(KERNEL_CONFIG)_debug_defconfig;\
		#enable KASAN;\
		if (( $(DEBUG) & 2 )) || (( $(DEBUG) & 1 )); then sed -i '$$a\CONFIG_KASAN=y\nCONFIG_KASAN_INLINE=y\nCONFIG_ARM_MODULE_PLTS=y' $(KERNEL_DIR)/$(KERNEL_CONFIG)_debug_defconfig; fi\
		#enable LOCKDEP;\
		if (( $(DEBUG) & 4 )) || (( $(DEBUG) & 1 )); then sed -i '$$a\CONFIG_DEBUG_LOCK_ALLOC=y\nCONFIG_PROVE_LOCKING=y\nCONFIG_LOCKDEP=y\nCONFIG_LOCK_STAT=y\nCONFIG_DEBUG_LOCKDEP=y\nCONFIG_TRACE_IRQFLAGS=y\nCONFIG_DEBUG_ATOMIC_SLEEP=y' $(KERNEL_DIR)/$(KERNEL_CONFIG)_debug_defconfig; fi\
		#enable MemLeak;\
		if (( $(DEBUG) & 8 )) || (( $(DEBUG) & 1 )); then sed -i '$$a\CONFIG_DEBUG_KMEMLEAK=y\nCONFIG_DEBUG_KMEMLEAK_EARLY_LOG_SIZE=40000' $(KERNEL_DIR)/$(KERNEL_CONFIG)_debug_defconfig; fi\
		#enable SLABINFO;\
		if (( $(DEBUG) & 16 )) || (( $(DEBUG) & 1 )); then sed -i '$$a\CONFIG_SLUB=y\nCONFIG_SLUB_DEBUG=y\nCONFIG_SLUB_DEBUG_ON=y\nCONFIG_SLUB_STATS=y' $(KERNEL_DIR)/$(KERNEL_CONFIG)_debug_defconfig; fi\
		#enable MP_IRQ;\
		if (( $(DEBUG) & 32 )); then sed -i '$$a\CONFIG_SSTAR_IRQ_DEBUG_TRACE=y' $(KERNEL_DIR)/$(KERNEL_CONFIG)_debug_defconfig; fi\
		#enable TRACERS;\
		if (( $(DEBUG) & 64 )); then \
		sed -i '$$a\CONFIG_TRACEPOINTS=y' $(KERNEL_DIR)/$(KERNEL_CONFIG)_debug_defconfig;\
		sed -i '$$a\CONFIG_NOP_TRACER=y\nCONFIG_TRACER_MAX_TRACE=y\nCONFIG_TRACE_CLOCK=y\nCONFIG_RING_BUFFER=y\nCONFIG_EVENT_TRACING=y\nCONFIG_CONTEXT_SWITCH_TRACER=y\nCONFIG_RING_BUFFER_ALLOW_SWAP=y\nCONFIG_TRACING=y\nCONFIG_GENERIC_TRACER=y' $(KERNEL_DIR)/$(KERNEL_CONFIG)_debug_defconfig;\
		sed -i '$$a\CONFIG_FTRACE=y\nCONFIG_IRQSOFF_TRACER=y\nCONFIG_PREEMPT_TRACER=y\nCONFIG_SCHED_TRACER=y\nCONFIG_TRACER_SNAPSHOT=y\nCONFIG_TRACER_SNAPSHOT_PER_CPU_SWAP=y\nCONFIG_BRANCH_PROFILE_NONE=y\nCONFIG_TRACING_EVENTS_GPIO=y' $(KERNEL_DIR)/$(KERNEL_CONFIG)_debug_defconfig;\
		sed -i '$$a\CONFIG_BINARY_PRINTF=y' $(KERNEL_DIR)/$(KERNEL_CONFIG)_debug_defconfig;\
		fi; \
		#enable GCOV;\
		if (( $(DEBUG) & 512 )); then \
		sed -i '$$a\CONFIG_DEBUG_FS=y' $(KERNEL_DIR)/$(KERNEL_CONFIG)_debug_defconfig; \
		sed -i '$$a\CONFIG_GCOV_KERNEL=y' $(KERNEL_DIR)/$(KERNEL_CONFIG)_debug_defconfig; \
		sed -i '$$a\CONFIG_GCOV_FORMAT_AUTODETECT=y' $(KERNEL_DIR)/$(KERNEL_CONFIG)_debug_defconfig; \
		sed -i '$$a\CONFIG_ARCH_HAS_GCOV_PROFILE_ALL=y' $(KERNEL_DIR)/$(KERNEL_CONFIG)_debug_defconfig; \
		sed -i '$$a\CONFIG_GCOV_PROFILE_ALL=y' $(KERNEL_DIR)/$(KERNEL_CONFIG)_debug_defconfig; \
		sed -i '$$a\CONFIG_MODULES_AREA_SIZE=0x01000000' $(KERNEL_DIR)/$(KERNEL_CONFIG)_debug_defconfig; \
		sed -i '$$a\CONFIG_ARM_MODULE_PLTS=y' $(KERNEL_DIR)/$(KERNEL_CONFIG)_debug_defconfig; \
		fi; \
		#enable msys test; \
		if (( $(DEBUG) & 4096 )); then \
		sed -i '$$a\CONFIG_MS_MSYS=y' $(KERNEL_DIR)/$(KERNEL_CONFIG)_debug_defconfig; \
		sed -i '$$a\CONFIG_MSYS_PERF_TEST=y' $(KERNEL_DIR)/$(KERNEL_CONFIG)_debug_defconfig; \
		fi; \
		echo "linux-config: \"$(KERNEL_CONFIG)_debug_defconfig\" DEBUG=$(DEBUG)";\
		$(MAKE) -C $(KERNEL_ROOT) $(KERNEL_CONFIG)_debug_defconfig;\
		if [[ "$(mi_debug_trace_cam_os_mem)" == "enable" ]]; then \
			sed -i '/CONFIG_TRACE_CAM_OS_MEM/d' $(KERNEL_ROOT)/.config; \
			echo "CONFIG_TRACE_CAM_OS_MEM=y" >> $(KERNEL_ROOT)/.config; \
		fi; \
		$(MAKE) -C $(KERNEL_ROOT);\
		if [[ "$(LINUX_ARCH_ARM)" == "y" ]]; then \
			$(MAKE) -C $(KERNEL_ROOT) uImage; \
		fi; \
		$(MAKE) cp_cam_os_libs; \
	else \
		echo "ignore linux-config: \"$(KERNEL_CONFIG)\" is unknown"; \
	fi

linux-kernel:
	@echo "build kernel: MAKEFLAGS <$(MAKEFLAGS)>"
	@if [[ -f $(KERNEL_DIR)/$(KERNEL_CONFIG) ]]; then \
		echo "linux-config: \"$(KERNEL_CONFIG)\"";\
		$(MAKE) -C $(KERNEL_ROOT) $(KERNEL_CONFIG) $(KBUILD_CONFIGS);\
		if [[ "$(mi_debug_trace_cam_os_mem)" == "enable" ]]; then \
			sed -i '/CONFIG_TRACE_CAM_OS_MEM/d' $(KERNEL_ROOT)/.config; \
			echo "CONFIG_TRACE_CAM_OS_MEM=y" >> $(KERNEL_ROOT)/.config; \
		fi; \
		$(MAKE) -C $(KERNEL_ROOT) $(KBUILD_CONFIGS) || exit "$$?";\
		if [[ "$(LINUX_ARCH_ARM)" == "y" ]]; then \
			$(MAKE) -C $(KERNEL_ROOT) uImage $(KBUILD_CONFIGS); \
		fi; \
		if [[ "$(KERNEL_DTBO_NAME)" != "" ]];then \
		$(MAKE) -C $(KERNEL_ROOT) dtbo_image || exit "$$?"; \
		fi; \
		$(MAKE) cp_cam_os_libs; \
	else \
		echo "ignore linux-config: \"$(KERNEL_CONFIG)\" is unknown"; \
	fi

linux-kernel_clean:
	@if [[ -f $(KERNEL_DIR)/$(KERNEL_CONFIG) ]]; then \
		$(MAKE) -C $(KERNEL_ROOT) clean $(KBUILD_CONFIGS) || exit "$$?"; \
	else \
		echo "ignore linux-config: \"$(KERNEL_CONFIG)\" is unknown"; \
	fi

aosp_common_kernel:
ifneq ($(SKIP_BUILD_KERNEL), 1)
	@echo "building kernel..."
	@$(MAKE) -C $(KERNEL_ROOT) mrproper $(KBUILD_CONFIGS)
	@pushd $(PROJ_ROOT)/../kernel; \
		LTO=$(LTO) BUILD_CONFIG=common/$(ACK_BUILD_CONFIG) OUT_DIR=$(ACK_OUT_DIR) DIST_DIR=$(ACK_DIST_DIR) \
		build/build.sh -j32 || exit "$$?"; \
		popd;
	@echo "build kernel done"
else
	@echo "skip build kernel..."
endif

VERSION=$(shell sed -n "/^VERSION/p" $(KERNEL_ROOT)/Makefile | tr -cd "[0-9]")
PATCHLEVEL=$(shell sed -n "/^PATCHLEVEL/p" $(KERNEL_ROOT)/Makefile| tr -cd "[0-9]")
SUBLEVEL=$(shell sed -n "/^SUBLEVEL/p" $(KERNEL_ROOT)/Makefile | tr -cd "[0-9]")
CUR_KERNEL_VERSION=$(VERSION)$(if $(PATCHLEVEL),.$(PATCHLEVEL)$(if $(SUBLEVEL),.$(SUBLEVEL)))

$(KBUILD_ROOT): FORCE
	@if [[ "$(KERNEL_VERSION)" == "" ]]; then \
		exit 0 ; \
	elif [[ "$(CUR_KERNEL_VERSION)" =~ "$(KERNEL_VERSION)" ]]; then \
		mkdir -p $(KBUILD_ROOT); \
	else \
		echo "ERROR: kernel config version:$(KERNEL_VERSION) , current version: $(CUR_KERNEL_VERSION)"; \
		exit 1; \
	fi

symbol_link: $(KBUILD_ROOT)
ifneq ($(PRODUCT), android)
	ln -sf $(KERNEL_ROOT)/modules $(KBUILD_ROOT)/
	ln -sf $(KERNEL_ROOT)/arch $(KBUILD_ROOT)/
	ln -sf $(KERNEL_ROOT)/drivers $(KBUILD_ROOT)/
	ln -sf $(KERNEL_ROOT)/include $(KBUILD_ROOT)/
	ln -sf $(KERNEL_ROOT)/scripts $(KBUILD_ROOT)/
	ln -sf $(KERNEL_ROOT)/usr $(KBUILD_ROOT)/
	ln -sf $(KERNEL_ROOT)/Makefile $(KBUILD_ROOT)/
	ln -sf $(KERNEL_ROOT)/Module.symvers $(KBUILD_ROOT)/
	ln -sf $(KERNEL_ROOT)/.sstar_chip.txt $(KBUILD_ROOT)/
	ln -sf $(KERNEL_ROOT)/.config $(KBUILD_ROOT)/
	ln -snf $(PROJ_ROOT)/kbuild/customize/$(KERNEL_VERSION)/$(CHIP)/$(PRODUCT) $(KBUILD_ROOT)/customize
else
	ln -sf $(KERNEL_DIST_DIR) $(KBUILD_ROOT)/dist
	ln -sf $(KERNEL_OUTPUT_DIR)/arch $(KBUILD_ROOT)/
	ln -sf $(KERNEL_OUTPUT_DIR)/scripts $(KBUILD_ROOT)/
	ln -sf $(KERNEL_OUTPUT_DIR)/Makefile $(KBUILD_ROOT)/
	ln -sf $(KERNEL_OUTPUT_DIR)/modules $(KBUILD_ROOT)/
	ln -sf $(KERNEL_OUTPUT_DIR)/include $(KBUILD_ROOT)/
	ln -sf $(KERNEL_OUTPUT_DIR)/.config $(KBUILD_ROOT)/
	ln -sf $(KERNEL_OUTPUT_DIR)/Module.symvers $(KBUILD_ROOT)/
	ln -sf $(KERNEL_ROOT)/drivers $(KBUILD_ROOT)/
	ln -sf $(KERNEL_ROOT)/.sstar_chip.txt $(KBUILD_ROOT)/
	ln -snf $(PROJ_ROOT)/kbuild/customize/$(KERNEL_VERSION)/$(CHIP)/$(PRODUCT) $(KBUILD_ROOT)/customize
endif

symbol_link_clean:
	rm $(KBUILD_ROOT) -rf

cp_cam_os_libs:
	echo "release cam_os_wrapper user space lib to alkaid"; \
        cd $(KERNEL_ROOT)/drivers/sstar/cam_os_wrapper; \
        $(MAKE) -f Makefile_lib clean all; \
        cp -rf .build/lib/libcam_os_wrapper.a $(KERNEL_LIB_RELEASE_DIR)/static; \
        cp -rf .build/lib/libcam_os_wrapper.so $(KERNEL_LIB_RELEASE_DIR)/dynamic; \
        $(MAKE) -f Makefile_lib clean; \
        echo "release cam_fs_wrapper user space lib to alkaid"; \
        cd $(KERNEL_ROOT)/drivers/sstar/cam_fs_wrapper; \
        $(MAKE) -f Makefile_lib clean all; \
        cp -rf .build/lib/libcam_fs_wrapper.a $(KERNEL_LIB_RELEASE_DIR)/static; \
        cp -rf .build/lib/libcam_fs_wrapper.so $(KERNEL_LIB_RELEASE_DIR)/dynamic; \
        $(MAKE) -f Makefile_lib clean; \
        echo "release ss_mbx user space lib to alkaid if necessary"; \
        cd $(KERNEL_ROOT)/drivers/sstar/mbx; \
        $(MAKE) -f Makefile_lib clean all; \
        if [ -f ".build/lib/libss_mbx.a" ] && [ -f ".build/lib/libss_mbx.so" ]; then \
                cp -rf .build/lib/libss_mbx.a $(KERNEL_LIB_RELEASE_DIR)/static; \
                cp -rf .build/lib/libss_mbx.so $(KERNEL_LIB_RELEASE_DIR)/dynamic; \
        fi; \
        $(MAKE) -f Makefile_lib clean; \

FORCE:
