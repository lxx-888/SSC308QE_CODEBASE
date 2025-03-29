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
-include ../../project/configs/current.configs
-include ../configs/current.configs
PROJ_ROOT?=$(PDIR)
DEBUG_ASAN:=$(shell if [[ "$(DEBUG)X" != "X"  ]] && ((( $(DEBUG) & 256 )) || (( $(DEBUG) & 1 ))); then echo "1"; fi)
DEBUG_GCOV:=$(shell if [[ "$(DEBUG)X" != "X"  ]] && ((( $(DEBUG) & 512 ))); then echo "1"; fi)
DEBUG_TSAN:=$(shell if [[ "$(DEBUG)X" != "X"  ]] && (( $(DEBUG) & 1024 )); then echo "1"; fi)
export PROJ_ROOT DEBUG_ASAN DEBUG_GCOV DEBUG_TSAN

# tsan compatible check
ifeq ($(DEBUG_TSAN), 1)
ifeq ($(DEBUG_ASAN), 1)
$(error tsan is not compatible with asan)
endif
ifneq ($(ARCH), arm64)
$(error tsan can only be used on arm64)
endif
endif

all:
ifneq ($(wildcard mi_demo),)
ifeq ($(verify_mi_demo), enable)
	$(MAKE) -C mi_demo all
endif
endif
ifneq ($(wildcard py_ipu),)
ifeq ($(verify_py_ipu), enable)
	$(MAKE) -C py_ipu mi_simulator
endif
endif
ifneq ($(wildcard sample_code),)
ifeq ($(verify_sample_code), enable)
	$(MAKE) -C sample_code all
endif
endif
ifneq ($(wildcard bsp_demo),)
ifeq ($(verify_bsp_demo), enable)
	$(MAKE) -C bsp_demo all
endif
endif
ifneq ($(wildcard sstar_ota),)
ifeq ($(verify_sstar_ota), enable)
	$(MAKE) -C sstar_ota all
endif
endif
ifneq ($(wildcard system_app),)
ifeq ($(verify_system_app), enable)
	$(MAKE) -C system_app all
endif
endif

install:
ifneq ($(wildcard mi_demo),)
ifeq ($(verify_mi_demo), enable)
	$(MAKE) -C mi_demo install
endif
endif
ifneq ($(wildcard sample_code),)
ifeq ($(verify_sample_code), enable)
	$(MAKE) -C sample_code install
endif
endif
ifneq ($(wildcard bsp_demo),)
ifeq ($(verify_bsp_demo), enable)
	$(MAKE) -C bsp_demo install
endif
endif
ifneq ($(wildcard amigos),)
ifeq ($(verify_amigos), enable)
	$(MAKE) -C amigos install
endif
endif
ifneq ($(wildcard sstar_ota),)
ifeq ($(verify_sstar_ota), enable)
	$(MAKE) -C sstar_ota install
endif
endif
clean:
ifneq ($(wildcard sample_code),)
ifeq ($(verify_sample_code), enable)
	$(MAKE) -C sample_code clean
endif
endif
ifneq ($(wildcard bsp_demo),)
ifeq ($(verify_bsp_demo), enable)
	$(MAKE) -C bsp_demo clean
endif
endif
ifneq ($(wildcard mi_demo),)
ifeq ($(verify_mi_demo), enable)
	$(MAKE) -C mi_demo clean
endif
endif
ifneq ($(wildcard py_ipu),)
ifeq ($(verify_py_ipu), enable)
	$(MAKE) -C py_ipu clean
endif
endif
ifneq ($(wildcard sstar_ota),)
ifeq ($(verify_sstar_ota), enable)
	$(MAKE) -C sstar_ota clean
endif
endif
ifneq ($(wildcard system_app),)
ifeq ($(verify_system_app), enable)
	$(MAKE) -C system_app clean
endif
endif
