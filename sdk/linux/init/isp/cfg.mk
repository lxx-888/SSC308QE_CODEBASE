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
DEP_MODULE:=common sys vif sensor scl
EXTRA_CFLAGS += -D__DISABLE_MHAL_ISR_REGISTER__
EX_CFLAGS += -D__I6F_ISP__

INIT_FILES:=isp_init.c ispmid_init.c mload_init.c
API_EXPORT_FILE+=isp_api_export.c