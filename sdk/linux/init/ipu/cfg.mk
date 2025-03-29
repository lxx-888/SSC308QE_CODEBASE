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
DEP_MODULE:=common sys

ifeq ($(CHIP),muffin)
EX_CFLAGS += -DUSE_IPU_OPP_TABLE=0
else ifeq ($(CHIP), mochi)
EX_CFLAGS += -DUSE_IPU_OPP_TABLE=0
else ifeq ($(CHIP), maruko)
EX_CFLAGS += -DUSE_IPU_OPP_TABLE=0
else ifeq ($(CHIP), opera)
EX_CFLAGS += -DUSE_IPU_OPP_TABLE=0
else ifeq ($(CHIP), souffle)
EX_CFLAGS += -DUSE_IPU_OPP_TABLE=0
else ifeq ($(CHIP), iford)
EX_CFLAGS += -DUSE_IPU_OPP_TABLE=0 -DSUPPORT_INVOKE_BY_KERNEL=1
endif
