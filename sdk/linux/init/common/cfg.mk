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
$(info rgnsub compile: $(mi_common_rgnsub))
ifeq ($(mi_common_rgnsub), enable)
EXTRA_CFLAGS += -DMI_COMMON_RGNSUB
endif

ifeq ($(mi_common_cjson_write), enable)
EXTRA_CFLAGS += -DCONFIG_MI_COMMON_SUPPORT_CJSON_WRITE
endif