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


KDIR?=$(PROJ_ROOT)/kbuild/$(KERNEL_VERSION)

SRC := st_light_misc_control_driver.c light_misc_control_main.c light_misc_control_hw.c light_misc_control_i2c.c

INC += $(KDIR)/include/ \
       $(KDIR)/drivers/sstar/include/ \
       $(KDIR)/drivers/sstar/include/$(CHIP)
INC += $(BUILD_TOP)/source/internal/light_misc_control/ \
       $(PROJ_ROOT)/release/include
KO_NAME := light_misc_control

APP_DIR:= $(OUT_PATH)/$(ARCH)/app

CODEDEFINE += -DLIGHT_USE_HRTIMER
CODEDEFINE += -DLIGHT_SUPPORT_MULTI_VIFDEVICE
# please set LIGHT_SENSOR_ONLY if project no need control led light
# CODEDEFINE += -DLIGHT_SENSOR_ONLY
# CODEDEFINE += -DLIGHT_USE_PM_NOTIFY
# CODEDEFINE += -DLIGHT_SUPPORT_LIGHTSENSOR_POWEROFF
