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
PROJ_DIR?=$(M)/../../../project
include $(PROJ_DIR)/configs/current.configs

KDIR?=$(PROJ_ROOT)/kbuild/$(KERNEL_VERSION)

.PHONY: FORCE

THIS_KO:=$(M)/$(COMBINE_MODULE_NAME).ko
MAP_FILE:=$(M)/$(COMBINE_MODULE_NAME)_ko.map

COMBINE_MI_LIBS:=$(foreach mod, $(COMBINE_MI_MODULES), $(patsubst %, %/lib.a, $(mod)))

obj-m:=$(COMBINE_MODULE_NAME).o
$(COMBINE_MODULE_NAME)-objs:=$(COMBINE_MI_LIBS) module.o

EXTRA_LDFLAGS += -Map=$(MAP_FILE)
EXTRA_LDFLAGS += --whole-archive

RCS_FIND_IGNORE := -maxdepth 1 \( -name SCCS -o -name BitKeeper -o -name .svn -o \
				   -name CVS -o -name .pc -o -name .hg -o -name .git \) -prune -o

MODULE_H:=module.h
mods_order:=

define get_mod_deps
$(strip $(shell \
	if [ -f $(1)/cfg.mk ]; then \
		deps=$$(cat $(1)/cfg.mk | grep -s 'DEP_MODULE' | awk -F'=' '{print $$2}'); \
		echo $${deps};
	fi))
endef

define sort_mods
$(foreach mod, $(1), $(if $(filter $(mod),$(mods_order)),, \
	$(eval deps:=$(foreach m,$(call get_mod_deps,$(mod)),$(filter $(m),$(1)))) \
	$(if $(deps),$(call sort_mods,$(deps)) $(eval mods_order+=$(mod)), \
			$(eval mods_order+=$(mod)))))
endef

define filechk
	$(call sort_mods,$(1))
	@echo \#define MODULES $$(echo $(mods_order) | tr ' ' ',') > $@.tmp
	@echo \#define EXIT_MODULES $$(echo $(mods_order) | awk '{for (i=NF; i>1; i--) printf "%s,", $$i; print $$1}') >> $@.tmp; \
	if [ -r $@ ] && cmp -s $@ $@.tmp; then \
		rm -f $@.tmp; \
	else \
		mv -f $@.tmp $@; \
	fi;
endef

$(MODULE_H):FORCE
	$(call filechk,$(COMBINE_MI_MODULES))

link_module:$(MODULE_H)
	@echo '$(COMBINE_MODULE_NAME)-objs' = $($(COMBINE_MODULE_NAME)-objs)
	$(MAKE) -f Makefile -C $(KDIR) M=$(CURDIR) modules

module_clean:
	$(MAKE) -f Makefile -C $(KDIR) M=$(CURDIR) clean RCS_FIND_IGNORE="$(RCS_FIND_IGNORE)"
	@rm -f $(MODULE_H)
	@rm -f *_ko.map

module_install:
	@mkdir -p $(INSTALL_DIR)
	cp -f $(COMBINE_MODULE_NAME).ko $(INSTALL_DIR)

FORCE:
