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

subdir-ccflags-y := -DSSTAR_CODING_STYLE_SANITIZER
PWD ?= $(shell pwd)
#CONFIG_SSTAR_CHIP_NAME := $(subst ",,$(CONFIG_SSTAR_CHIP_NAME))

KDIR = $(PWD)/../..
EXTRA_CFLAGS      +=  -I$(KDIR)/include
EXTRA_CFLAGS      +=  -I$(KDIR)/drivers/sstar/include
EXTRA_CFLAGS      +=  -I$(KDIR)/drivers/sstar/camdriver/common
EXTRA_CFLAGS      +=   -Wno-pointer-to-int-cast -Wno-unused-variable -Wno-unused-function -Wno-return-type

obj-m := chk_otp.o
chk_otp-objs :=  chkotp.o chkotp_data.o
all:
	@echo EXTRA_CFLAGS=$(EXTRA_CFLAGS)
	@echo KERNEL_DIR = $(KDIR)
	make modules -C $(KDIR)  M=$(PWD)

clean:
	rm -rf *.ko module* Module* *.mod* *.o

