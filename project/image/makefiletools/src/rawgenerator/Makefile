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
.PHONY: all

TARGET_DISP := dispcfggen
TARGET_LOGO := logogen


all:
	gcc -m32 -Wall ss_raw_header.c logo_data_main.c -o ../../bin/$(TARGET_LOGO)
	gcc -m32 -Wall -I./pnl ss_raw_header.c disp_data_main.c -o ../../bin/$(TARGET_DISP)

	