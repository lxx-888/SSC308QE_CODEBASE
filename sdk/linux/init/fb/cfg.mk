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

DEP_MODULE:= common sys

ifeq ($(DUAL_OS),)
ifeq ($(MI_FB_NORMAL), y)
DEP_MODULE+= disp
EX_CFLAGS+= -DMI_FB_NORMAL
endif

ifeq ($(MI_FB_VIDEO), y)
DEP_MODULE+= rgn
EX_CFLAGS+= -DMI_FB_VIDEO
endif
endif
INIT_FILES+=fb_init.c
API_EXPORT_FILE:=fb_api_export.c
