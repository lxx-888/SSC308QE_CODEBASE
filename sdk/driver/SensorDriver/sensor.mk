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

M ?= $(CURDIR)
ifneq ($(CUSTOMER_OPTIONS),)
-include $(PROJ_ROOT)/release/customer_options/$(CUSTOMER_OPTIONS)
endif
include $(PROJ_ROOT)/configs/current.configs

KDIR?=$(PROJ_ROOT)/kbuild/$(KERNEL_VERSION)

ifeq ($(CHIP), almonds)
MSTAR_INC_DIR:= $(PROJ_ROOT)/../sdk/driver/SensorDriver/drv/inc \
				$(PROJ_ROOT)/../sdk/driver/SensorDriver/drv/pub \
				$(PROJ_ROOT)/release/$(PRODUCT)/include \
				$(PROJ_ROOT)/release/$(PRODUCT)/include/drivers/sensorif
else
MSTAR_INC_DIR:= $(PROJ_ROOT)/../sdk/driver/SensorDriver/drv/inc \
				$(PROJ_ROOT)/../sdk/driver/SensorDriver/drv/pub \
				$(PROJ_ROOT)/release/include \
				$(PROJ_ROOT)/release/include/drivers/sensorif
endif

EXTRA_CFLAGS:= -g -Werror -Wall -Wno-unused-result -pipe \
			-DEXTRA_MODULE_NAME=$(MODULE_DIR) $(EXTRA_INCS) \
			-DMI_DBG=$(MI_DBG) \
			-DSUPPORT_HDMI_VGA_DIRECT_MODE=${SUPPORT_HDMI_VGA_DIRECT_MODE} \
			-DSUPPORT_DIVP_USE_GE_SCALING_UP=${SUPPORT_DIVP_USE_GE_SCALING_UP} \
			-DSUPPORT_VIF_USE_GE_FILL_BUF=${SUPPORT_VIF_USE_GE_FILL_BUF} \
			-DSUPPORT_MsOS_MPool_Add_PA2VARange=${SUPPORT_MsOS_MPool_Add_PA2VARange} \
			-DSUPPORT_DISP_ALIGN_UP_OFFSET32=${SUPPORT_DISP_ALIGN_UP_OFFSET32} \
			$(foreach n,$(MSTAR_INC_DIR),-I$(n)) $(foreach n,$(INTERFACE_ENABLED),-DINTERFACE_$(shell tr 'a-z' 'A-Z' <<< $(n))=1) $(foreach n,$(INTERFACE_DISABLED),-DINTERFACE_$(shell tr 'a-z' 'A-Z' <<< $(n))=0) $(foreach n,$(MHAL_ENABLED),-DMHAL_$(shell tr 'a-z' 'A-Z' <<< $(n))=1) $(foreach n,$(MHAL_DISABLED),-DMHAL_$(shell tr 'a-z' 'A-Z' <<< $(n))=0)

ifeq ($(CHIP), almonds)
EXTRA_CFLAGS+= -DSUPPORT_VDEC_MULTI_RES=1
EXTRA_CFLAGS+= -DCONFIG_MSTAR_CHIP_ALMONDS=1
else ifeq ($(CHIP), cashews)
	EXTRA_CFLAGS+= -DCONFIG_MSTAR_CHIP_CASHEWS=1
else ifeq ($(CHIP), walnuts)
EXTRA_CFLAGS+= -DCONFIG_MSTAR_CHIP_WALNUTS=1
else ifeq ($(CHIP), pudding)
EXTRA_CFLAGS+= -DCONFIG_SIGMASTAR_CHIP_PUDDING=1
else ifeq ($(CHIP), ispahan)
EXTRA_CFLAGS+= -DCONFIG_SIGMASTAR_CHIP_ISPAHAN=1
endif

KBUILD_EXTRA_SYMBOLS:=

ifeq (y, $(ANDROID))
EXTRA_CFLAGS+= -DANDROID
#KBUILD_EXTRA_SYMBOLS+=$(PROJ_ROOT)/../sdk/interface/src/common/Module.symvers
endif

ifneq ($(DEBUG), )
EXTRA_CFLAGS += -DSUPPORT_GPL_SYMBOL
endif
ifeq ($(SENSOR_SUPPORT_IR),y)
EXTRA_CFLAGS += -DCONFIG_SENSOR_SUPPORT_IR
endif
EXTRA_CFLAGS+= -I$(KDIR)/drivers/sstar/include/ -I$(KDIR)/drivers/sstar/cpu/include/ -I$(KDIR)/drivers/sstar/miu/ -I$(KDIR)/drivers/sstar/include/$(CHIP_FULL_NAME)

ifeq ($(PROJ_ROOT)/../sdk/mhal/$(CHIP)/utpa, $(wildcard $(PROJ_ROOT)/../sdk/mhal/$(CHIP)/utpa$))
MSTAR_INC_DIR+=$(PROJ_ROOT)/../sdk/mhal/include/utopia
EXTRA_CFLAGS += -I$(PROJ_ROOT)/../sdk/mhal/include/utopia
endif

MVXV_START := MVX3
MVXV_LIB_TYPE := $(shell echo $(CHIP) | tr a-z A-Z)
MS_PLATFORM_ID := __
COMMITNUMBER_SENSOR := g$(shell cd $(PROJ_ROOT)/../sdk/driver/SensorDriver; git log --format=%h -n 1 2> /dev/null)$(shell git diff --quiet 2> /dev/null || echo -dirty )$(shell cd -;)
COMP_ID_SENSOR := [sensor_module]
MVXV_EXT_SENSOR := [$(shell cd $(PROJ_ROOT)/../sdk/driver/SensorDriver; git rev-parse --abbrev-ref HEAD 2> /dev/null | sed -e 's/\//_/g')$(shell cd -;) build.$(shell date +%Y%m%d%H%M)
MVXV_END := ]XVM
EXTRA_CFLAGS += -DSENSOR_MODULE_VERSION="$(MVXV_START)$(MVXV_LIB_TYPE)$(MS_PLATFORM_ID)$(COMMITNUMBER_SENSOR)$(COMP_ID_SENSOR)$(MVXV_EXT_SENSOR)$(MVXV_END)"
SENSOR_HEADER ?= drv_ss_

MI_SENSOR_NAME:=$(MOD_PREFIX)$(SENSOR_NAME)

ifneq ($(SENSOR_FILES),)
obj-m := $(patsubst %.c, %.o, $(SENSOR_FILES))
THIS_KO:=$(M)/*.ko
ifneq ($(3RD_SENSORS), )
include $(foreach f,$(3RD_SENSORS),$(M)/../3rd/$(f)/$(f).mk)
endif
else
ifneq ($(3RD_SENSOR), )
include $(M)/../3rd/$(3RD_SENSOR)/$(3RD_SENSOR).mk
else
obj-m := $(SENSOR_FILE).o
THIS_KO:=$(M)/$(MI_SENSOR_NAME).ko
endif
endif

$(warning obj-m:$(obj-m))

## src file need fix
##$(MI_SENSOR_NAME)-objs+= $(patsubst %.c, %.o, $(WRAPPER_FILE))

KBUILD_LDFLAGS_MODULE += -Map=$(patsubst %.ko,%_ko.map,$@)

modules:
	@echo $(MSTAR_INC_DIR)
	$(MAKE) -C $(KDIR) M=$(CURDIR) modules
	@rename -f 's/$(SENSOR_HEADER)//g' $(THIS_KO)

modules_install:
	$(MAKE) module_install

modules_clean:
	$(MAKE) module_clean

ifeq ($(filter $(SENSOR_FILE).c, $(SOURCE_FILE)),)
module:
	@echo $(MSTAR_INC_DIR)
	$(MAKE) -C $(KDIR) M=$(CURDIR) modules MI_MODULE_NAME=$(MI_SENSOR_NAME)
	@rename -f 's/$(SENSOR_HEADER)//g' $(M)/$(SENSOR_HEADER)$(MI_SENSOR_NAME).ko
else
module:
	@echo rename source file $(MI_SENSOR_NAME).c
	@exit;
endif

module_install:
ifeq ($(CHIP), pretzel)
	cp -f $(THIS_KO) $(PROJ_ROOT)/release/$(PRODUCT)/$(CHIP)/$(BOARD)/$(TOOLCHAIN)/$(TOOLCHAIN_VERSION)/lib/modules/$(KERNEL_VERSION)
else ifeq ($(CHIP), almonds)
	cp -f $(THIS_KO) $(PROJ_ROOT)/release/$(PRODUCT)/$(CHIP)/$(BOARD)/$(TOOLCHAIN)/$(TOOLCHAIN_VERSION)/lib/modules/$(KERNEL_VERSION)
else
	cp -f $(THIS_KO) $(PROJ_ROOT)/release/chip/$(CHIP)/$(PRODUCT)/common/$(TOOLCHAIN)/$(TOOLCHAIN_VERSION)/release/modules/$(KERNEL_VERSION)
endif

module_clean:
	$(MAKE) -C $(KDIR) M=$(CURDIR) clean
	@rm -f $(M)/*_ko.map
