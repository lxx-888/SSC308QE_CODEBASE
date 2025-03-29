#
# Copyright (c) [2019~2022] SigmaStar Technology.
#
#
# This software is licensed under the terms of the GNU General Public
# License version 2, as published by the Free Software Foundation, and
# may be copied, distributed, and modified under those terms.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License version 2 for more details.
#

subdir-ccflags-y := -DSSTAR_CODING_STYLE_SANITIZER
PWD ?= $(shell pwd)
#CONFIG_SSTAR_CHIP_NAME := $(subst ",,$(CONFIG_SSTAR_CHIP_NAME))

KDIR = $(PWD)/../../../../../kernel

VMFILE = $(KDIR)/vmlinux.o

ifneq ($(VMFILE), $(wildcard $(VMFILE)))
	KDIR = $(PWD)/../../../../../
endif

EXTRA_CFLAGS      +=  -I$(KDIR)/drivers/sstar/include
EXTRA_CFLAGS      +=  -I$(KDIR)/drivers/sstar/camdriver/common
EXTRA_CFLAGS      +=   -Wno-pointer-to-int-cast -Wno-unused-variable -Wno-unused-function -Wno-return-type

#ifeq ($(CONFIG_SSTAR_CHIP_NAME), "mercury6p")
#	EXTRA_CFLAGS += -DSUPPORT_PKA
#	EXTRA_CFLAGS += -DSUPPORT_RSA4096
#endif
#ifneq ($(CONFIG_SSTAR_CHIP_NAME), "infinity7")
#ifneq ($(CONFIG_SSTAR_CHIP_NAME), "infinity6c")
#EXTRA_CFLAGS += -DSUPPORT_RSA4096
#endif
#endif

obj-m += ut_aes.o

all:
	@echo EXTRA_CFLAGS=$(EXTRA_CFLAGS)
	@echo KERNEL_DIR = $(KERNEL_DIR)
	#make -C $(KDIR) SUBDIRS=$(shell pwd) modules
	make modules -C $(KDIR)  M=`pwd`

clean:
	rm -rf *.ko module* Module* *.mod* *.o
