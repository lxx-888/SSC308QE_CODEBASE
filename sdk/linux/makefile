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
M ?= $(CURDIR)
PROJ_DIR?=$(M)/../../project
include $(PROJ_DIR)/configs/current.configs

INSTALL_DIR?=$(PROJ_ROOT)/release/chip/$(CHIP)/$(PRODUCT)/common/$(TOOLCHAIN)/$(TOOLCHAIN_VERSION)/release/modules/$(KERNEL_VERSION)
STRIP?=$(CROSS_COMPILE)strip

define filter_enable_modules
$(patsubst %_enable_,%,$(filter-out %__,$(filter-out %_disable_,$(foreach m,$(1),$(m)_$(interface_$(m))_))))
endef

FILTER_OUT_MODULES ?=

MODULES_DIR := $(patsubst init/%/cfg.mk,%,$(wildcard init/*/cfg.mk))
INIT_MODULES := $(filter-out $(FILTER_OUT_MODULES),$(call filter_enable_modules,$(MODULES_DIR)))

ifeq ($(DUAL_OS),on)
    ifneq ($(CONFIG_ENABLE_POWER_SAVE_AOV),y)
    INIT_MODULES := $(foreach m,$(INIT_MODULES),$(if $(wildcard $(CURDIR)/obj/$(TOOLCHAIN)/$(TOOLCHAIN_VERSION)/$(m)_dualos.o),$(m)))
    endif
endif

TARGET_MODULE_CLEAN := $(patsubst %, %_module_clean, $(MODULES_DIR))
TARGET_MAKEFILE := $(patsubst %, %_makefile, $(MODULES_DIR))

.PHONY: all install clean strip $(INIT_MODULES) $(TARGET_MODULE_CLEAN) $(TARGET_MAKEFILE) combine_module combine_module_clean combine_module_install combine_makefile

ifeq ($(COMBINE_MI_MODULE),y)
COMBINE_MI_MODULES ?= $(filter-out $(COMBINE_IGNORE_MODULES),$(INIT_MODULES))
export COMBINE_MI_MODULES
endif

all:

$(TARGET_MAKEFILE): %_makefile:
	@ln -sf $(CURDIR)/link_modules.mk $(CURDIR)/init/$(patsubst %_makefile,%,$@)/Makefile

$(INIT_MODULES):%:
	@$(MAKE) $@_makefile
	@$(MAKE) -C $(CURDIR)/init/$@ -f Makefile link_module

$(TARGET_MODULE_CLEAN):%_module_clean:%_makefile
	@$(MAKE) -C $(CURDIR)/init/$(patsubst %_module_clean,%,$@) -f Makefile module_clean
	@rm $(CURDIR)/init/$(patsubst %_module_clean,%,$@)/Makefile

all:
ifneq ($(strip $(INIT_MODULES)),)
	@$(MAKE) $(INIT_MODULES)
endif
ifeq ($(COMBINE_MI_MODULE),y)
	@$(MAKE) combine_module
endif

clean:
	@$(MAKE) $(TARGET_MODULE_CLEAN)
	@$(MAKE) combine_module_clean

install:
ifeq ($(COMBINE_MI_MODULE),y)
	@$(MAKE) combine_module_install
else
	@for mod in $(INIT_MODULES);do \
		if [[ -f $(CURDIR)/init/$${mod}/mi_$${mod}.ko ]];then \
			cp -f $(CURDIR)/init/$${mod}/mi_$${mod}.ko $(INSTALL_DIR); \
		fi; \
		if [[ "$(DUAL_OS)" = "on" ]] && [[ -f $(CURDIR)/init/$${mod}/mi_$${mod}_dualos.ko ]];then \
			cp -f $(CURDIR)/init/$${mod}/mi_$${mod}_dualos.ko $(INSTALL_DIR); \
		fi; \
	done
endif

strip:
	$(STRIP) --strip-unneeded $(INSTALL_DIR)/*.ko

combine_module:combine_makefile
	@$(MAKE) -C $(CURDIR)/init -f Makefile link_module
	@rm -f $(CURDIR)/init/Makefile

combine_module_clean:combine_makefile
	@$(MAKE) -C $(CURDIR)/init -f Makefile module_clean
	@rm -f $(CURDIR)/init/Makefile

combine_module_install:combine_makefile
	@$(MAKE) -C $(CURDIR)/init -f Makefile module_install INSTALL_DIR=$(INSTALL_DIR)
	@rm -f $(CURDIR)/init/Makefile

combine_makefile:
	@ln -sf $(CURDIR)/link_combine.mk $(CURDIR)/init/Makefile

help:
	@echo -e 'make all [PROJ_DIR=$$SSTAR_PROJ_DIR] [KDIR=$$SSTAR_KERNEL_DIR] [SENSOR_DRIVER_DIR=$$SSTAR_SENSOR_DRIVER_DIR]'
