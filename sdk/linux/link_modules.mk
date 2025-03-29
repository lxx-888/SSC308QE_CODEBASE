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

PROJ_DIR?=$(M)/../../../../project
include $(PROJ_DIR)/configs/current.configs
KDIR?=$(PROJ_ROOT)/kbuild/$(KERNEL_VERSION)
SENSOR_DRIVER_DIR?=$(PROJ_ROOT)/../sdk/driver/SensorDriver
CONFIG_SSTAR_CHIP_NAME:=$(CHIP_FULL_NAME)

MODULE_DIR?=$(lastword $(subst /, , $(M)))
MOD_PREFIX?=mi_
MI_MODULE_NAME?=$(MOD_PREFIX)$(MODULE_DIR)
export MODULE_DIR MOD_PREFIX MI_MODULE_NAME

SIGMASTAR_INC_DIR+=$(foreach mod,$(MODULE_DIR) $(DEP_MODULE),$(M)/../$(mod))
SIGMASTAR_INC_DIR+=$(SENSOR_DRIVER_DIR)/drv/pub
SIGMASTAR_INC_DIR+=$(PROJ_ROOT)/release/include/
SIGMASTAR_INC_DIR+=$(KDIR)/include/
SIGMASTAR_INC_DIR+=$(KDIR)/drivers/sstar/include
SIGMASTAR_INC_DIR+=$(KDIR)/drivers/sstar/include/$(CONFIG_SSTAR_CHIP_NAME)/

# mi_$(MODULE_DIR)_kapi could be define in current.configs / kconfig
# the priority of KAPI_ENABLE in cfg.mk is greater than the priority of $(mi_$(MODULE_DIR)_kapi)
KAPI_ENABLE:=$(mi_$(MODULE_DIR)_kapi)
-include $(M)/cfg.mk

EXTRA_CFLAGS+= -g -Werror -Wall -Wno-unused-variable -Wno-unused-result -pipe \
			-DEXTRA_MODULE_NAME=$(MODULE_DIR) \
			$(foreach n,$(SIGMASTAR_INC_DIR),-I$(n))

EXTRA_CFLAGS+=$(EX_CFLAGS)
EXTRA_CFLAGS+=-include linux/version.h

EXTRA_CFLAGS+=-DALKAID_SDK
ifeq ($(COVERITY), 1)
EXTRA_CFLAGS += -DSUPPORT_COVERITY
EXTRA_CFLAGS += $(call cc-disable-warning, unused-variable)
EXTRA_CFLAGS += $(call cc-disable-warning, unused-function)
endif

SUPPORT_GPL_SYMBOL_MODULES:=

ifneq ($(filter $(MI_MODULE_NAME), $(SUPPORT_GPL_SYMBOL_MODULES)),)
EXTRA_CFLAGS += -DNON_COMMERCIAL_FOR_INTERNAL_TEST_ONLY
endif
EXTRA_CFLAGS += -DCONFIG_MI_$(shell tr 'a-z' 'A-Z' <<< $(MODULE_DIR))=1

ifeq ($(CONFIG_ENABLE_MI_NORMAL_STR), y)
EXTRA_CFLAGS += -DCONFIG_ENABLE_MI_NORMAL_STR
endif
ifeq ($(CONFIG_ENABLE_MI_RTPM), y)
EXTRA_CFLAGS += -DCONFIG_ENABLE_MI_RTPM
endif

THIS_KO:=$(M)/$(MI_MODULE_NAME).ko
MAP_FILE:=$(M)/$(MI_MODULE_NAME)_ko.map

EXTRA_LDFLAGS += -Map=$(MAP_FILE)

# default $(M)/*.c,also can specify by cfg.mk
INIT_FILES?=$(notdir $(filter-out $(wildcard $(M)/*.mod.c),$(wildcard $(M)/*.c)))
ifeq ($(ENABLE_MI_KAPI),y)
ifeq ($(KAPI_ENABLE),yes)
INIT_FILES+=$(API_EXPORT_FILE)
endif
endif

ifeq ($(COMBINE_MI_MODULE),y)
ifeq ($(filter $(MODULE_DIR), $(COMBINE_IGNORE_MODULES)),)
COMBINE_BUILD:=y
endif
endif

OPEN_SOURCE_OBJ_DIR:=../../obj/$(TOOLCHAIN)/$(TOOLCHAIN_VERSION)

ifeq ($(COMBINE_BUILD),y)
lib-y := $(OPEN_SOURCE_OBJ_DIR)/$(MODULE_DIR).o $(patsubst %.c, %.o, $(INIT_FILES))
EXTRA_CFLAGS += -DCOMBINE_MI_MODULE
else
    ifneq ($(DUAL_OS),on)
        obj-m := $(MI_MODULE_NAME).o
        $(MI_MODULE_NAME)-objs := $(OPEN_SOURCE_OBJ_DIR)/$(MODULE_DIR).o $(patsubst %.c, %.o, $(INIT_FILES))
    else
        ifeq ($(CONFIG_ENABLE_POWER_SAVE_AOV),y)
            obj-m := $(MI_MODULE_NAME).o
            $(MI_MODULE_NAME)-objs := $(OPEN_SOURCE_OBJ_DIR)/$(MODULE_DIR).o $(patsubst %.c, %.o, $(INIT_FILES))
        endif
        ifneq ($(wildcard $(M)/$(OPEN_SOURCE_OBJ_DIR)/$(MODULE_DIR)_dualos.o),)
            obj-m += $(MI_MODULE_NAME)_dualos.o
            $(MI_MODULE_NAME)_dualos-objs := $(OPEN_SOURCE_OBJ_DIR)/$(MODULE_DIR)_dualos.o $(patsubst %.c, %.o, $(INIT_FILES))
        endif
    endif # DUAL_OS
    KBUILD_EXTRA_SYMBOLS := $(foreach m, $(DEP_MODULE), $(M)/../$(m)/Module.symvers)
endif # COMBINE_BUILD

RCS_FIND_IGNORE := \( -name SCCS -o -name BitKeeper -o -name .svn -o    \
	-name CVS -o -name .pc -o -name .hg -o -name .git -o -name $(OPEN_SOURCE_OBJ_DIR)/$(MODULE_DIR).o \) \
	-prune -o

BLACK_DIRS:=sdk/impl ../../impl sdk/interface ../../interface
HIT_DIRS:=$(foreach n,$(BLACK_DIRS),$(findstring $(n),$(EXTRA_CFLAGS)))
HIT_DIRS:=$(strip $(HIT_DIRS))
ifneq ($(HIT_DIRS),)
$(error hit BLACK_DIRS:"$(HIT_DIRS)")
endif

link_module:
	@echo PROJ_DIR = $(PROJ_DIR)
	@echo INIT_FILES = $(INIT_FILES)
	@echo obj-m = $(obj-m)
	@echo lib-y = $(lib-y)
ifneq ($(obj-m),)
	@echo $(MI_MODULE_NAME)-objs = $($(MI_MODULE_NAME)-objs)
	@echo $(MI_MODULE_NAME)_dualos-objs = $($(MI_MODULE_NAME)_dualos-objs)
ifneq ($(strip $($(MI_MODULE_NAME)-objs) $($(MI_MODULE_NAME)_dualos-objs)),)
	$(MAKE) -f Makefile -C $(KDIR) M=$(CURDIR) modules
endif
else ifneq ($(strip $(lib-y)),)
	$(MAKE) -f Makefile -C $(KDIR) M=$(CURDIR)
endif # obj-m

module_clean:
	$(MAKE) -f Makefile -C $(KDIR) M=$(CURDIR) clean RCS_FIND_IGNORE="$(RCS_FIND_IGNORE)"
	rm -rf $(MAP_FILE)
