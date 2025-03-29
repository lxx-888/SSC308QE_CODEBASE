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

JSON_RESOURCE := $(wildcard $(MODULE_PATH)/resource/$(CHIP)/*.json)

define make_include
echo -e "#include \"$(1).h\"" >> $(AUTO_GEN_BIN_FILE);
endef
define make_macro
echo -e "    PTREE_BIN_FILL_INFO($(1))," >> $(AUTO_GEN_BIN_FILE);
endef
define gen_header
	$(BUILD_TOP)/out/x86/app/sstar_arena_bin -i $(MODULE_PATH)/resource/$(1)/$(2)_ptree.json -o $(MODULE_PATH)/resource/$(2).h -n $(2);
endef

script_start:
ifneq ($(ARCH),x86)
	@$(MAKE) applications/mixer/app_ptree/sstar_arena_bin ARCH=x86 USING_BINARY=0 USING_JSON=1 CLANG_FORMAT_ENABLE= CLANG_TIDY_ENABLE=
	@$(foreach n, $(JSON_RESOURCE), $(call gen_header,$(CHIP),$(patsubst %_ptree.json,%,$(notdir $(n)))))
	@echo -e "/* This is auto gen header file, please don't modify it. */\n" > $(AUTO_GEN_BIN_FILE)
	@echo -e "#ifndef __AUTO_GEN_PTREE_BIN__" >> $(AUTO_GEN_BIN_FILE)
	@echo -e "#define __AUTO_GEN_PTREE_BIN__" >> $(AUTO_GEN_BIN_FILE)
	@$(foreach n, $(JSON_RESOURCE), $(call make_include,$(patsubst %_ptree.json,%,$(notdir $(n)))))
	@echo -e "static const PTREE_BIN_Info_t G_BINARY_INFO_ARRAY[] = { // NOLINT" >> $(AUTO_GEN_BIN_FILE)
	@$(foreach n, $(JSON_RESOURCE), $(call make_macro,$(patsubst %_ptree.json,%,$(notdir $(n)))))
	@echo -e "};" >> $(AUTO_GEN_BIN_FILE)
	@echo -e "#endif /* __AUTO_GEN_PTREE_BIN__ */" >> $(AUTO_GEN_BIN_FILE)
endif

script_end:
ifneq ($(ARCH),x86)
	@$(MAKE) applications/mixer/app_ptree/sstar_arena_bin_clean USING_BINARY=0 USING_JSON=1 ARCH=x86;
endif

