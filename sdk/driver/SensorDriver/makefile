# SigmaStar trade secret
# Copyright (c) [2019~2022] SigmaStar Technology.
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

#
# Makefile for SStar sensor drivers.
#

PROJ_ROOT ?=../../../project
include $(PROJ_ROOT)/configs/current.configs


SENSOR_HEADER = drv_ss_

3RD_SENSORS := $(notdir $(wildcard drv/3rd/*))
3RD_SENSORS_CLEAN := $(patsubst %, %_clean, $(3RD_SENSORS))
3RD_SENSORS_INSTALL := $(patsubst %, %_install, $(3RD_SENSORS))
SENSOR_SRC_DIR=drv/src
SENSORSRC = $(notdir $(shell find $(SENSOR_SRC_DIR) -name *.c ))
SENSOR_SRC = $(filter-out %.mod.c ,$(SENSORSRC))
SSTAR_SENSORS?=$(patsubst $(SENSOR_HEADER)%,%,$(patsubst %.c,%, $(SENSOR_SRC)))
SSTAR_SENSORS_CLEAN:=$(patsubst %, %_clean, $(SSTAR_SENSORS))
SSTAR_SENSORS_INSTALL := $(patsubst %, %_install, $(SSTAR_SENSORS))


TARGET_MODULES := $(patsubst %, %_module, $(SSTAR_SENSORS) $(3RD_SENSORS))
TARGET_LIBS:= $(patsubst %, %_lib, $(SSTAR_SENSORS) $(3RD_SENSORS))

MOD_PREFIX:=


export PROJ_ROOT CHIP PRODUCT BOARD TOOLCHAIN TOOLCHAIN_VERSION KERNEL_VERSION CUSTOMER_OPTIONS MOD_PREFIX INTERFACE_ENABLED INTERFACE_DISABLED MHAL_ENABLED MHAL_DISABLED

ifneq (y,${ANDROID})
.PHONY: $(SSTAR_SENSORS) $(3RD_SENSORS)
else
.PHONY: $(SSTAR_SENSORS)
endif

all:build_makefile
ifeq (y,${ANDROID})
	$(MAKE) -C $(SENSOR_SRC_DIR) modules SENSOR_FILES="$(SENSOR_SRC)" SENSOR_HEADER=$(SENSOR_HEADER) $(KBUILD_CONFIGS)
else
	$(MAKE) -C $(SENSOR_SRC_DIR) modules SENSOR_FILES="$(SENSOR_SRC)" SENSOR_HEADER=$(SENSOR_HEADER) 3RD_SENSORS=$(3RD_SENSORS) $(KBUILD_CONFIGS)
endif
	@echo all done!

clean:build_makefile
	$(MAKE) -C $(SENSOR_SRC_DIR) modules_clean $(KBUILD_CONFIGS)
	@rm -f $(SENSOR_SRC_DIR)/Makefile
	@echo all clean!

install:build_makefile
	$(MAKE) -C $(SENSOR_SRC_DIR) modules_install SENSOR_FILES="$(SENSOR_SRC)" SENSOR_HEADER=$(SENSOR_HEADER) $(KBUILD_CONFIGS)
	@echo all install!

build_makefile:
	@ln -sf $(CURDIR)/sensor.mk $(SENSOR_SRC_DIR)/Makefile

$(SSTAR_SENSORS) $(3RD_SENSORS):%:build_makefile
	@$(MAKE) $@_module

$(SSTAR_SENSORS_INSTALL) $(3RD_SENSORS_INSTALL):%:build_makefile
	$(MAKE) -C $(SENSOR_SRC_DIR) module_install SENSOR_NAME=$(patsubst %_install,%,$@) SENSOR_FILE=$(patsubst %_install,%,$(patsubst %,$(SENSOR_HEADER)%,$@)) 3RD_SENSOR=$(filter $(patsubst %_install,%,$@), $(3RD_SENSORS)) $(KBUILD_CONFIGS)
	@if [[ "`ls -A $(CURDIR)/drv/pub`" != "" ]]; then \
		if [[ $(CHIP) == "i2" ]]; then \
			cp -rf $(CURDIR)/drv/pub/drv_ss_cus_sensor.h $(PROJ_ROOT)/release/$(PRODUCT)/include/drivers/sensorif; \
			cp -rf $(CURDIR)/drv/pub/sensor_i2c_api.h $(PROJ_ROOT)/release/$(PRODUCT)/include/drivers/sensorif; \
		else \
			cp -rf $(CURDIR)/drv/pub/drv_ss_cus_sensor.h $(PROJ_ROOT)/release/include/drivers/sensorif; \
			cp -rf $(CURDIR)/drv/pub/drv_ss_cus_vcm.h $(PROJ_ROOT)/release/include/drivers/sensorif; \
			cp -rf $(CURDIR)/drv/pub/sensor_i2c_api.h $(PROJ_ROOT)/release/include/drivers/sensorif; \
		fi; \
	fi;

$(TARGET_MODULES): %_module:build_makefile
	$(MAKE) -C $(SENSOR_SRC_DIR) module SENSOR_NAME=$(patsubst %_module,%,$@) SENSOR_FILE=$(patsubst %_module,$(SENSOR_HEADER)%,$@) 3RD_SENSOR=$(filter $(patsubst %_module,%,$@), $(3RD_SENSORS)) $(KBUILD_CONFIGS)

$(SSTAR_SENSORS_CLEAN) $(3RD_SENSORS_CLEAN):%:build_makefile
	$(MAKE) -C $(SENSOR_SRC_DIR) module_clean SENSOR_NAME=$(patsubst %_clean,%,$@) SENSOR_FILE=$(patsubst %_clean,%,$(patsubst %,$(SENSOR_HEADER)%,$@)) 3RD_SENSOR=$(filter $(patsubst %_clean,%,$@), $(3RD_SENSORS)) $(KBUILD_CONFIGS)
