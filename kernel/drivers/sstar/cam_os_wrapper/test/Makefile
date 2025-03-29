#
# Copyright (c) [2019~2020] SigmaStar Technology.
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

obj-m       := cam_os_test.o
cam_os_test-objs := cam_os_wrapper_linux_kernel_test.o

#export ARCH=arm
#export CROSS_COMPILE=arm-linux-gnueabihf-

KERNEL = $(PWD)/../../../../
CC  = $(CROSS_COMPILE)gcc
APP = cam_os_wrapper_test
CONFIG_SSTAR_CHIP_NAME := $(subst ",,$(CONFIG_SSTAR_CHIP_NAME))
EXTRA_CFLAGS := -I$(KERNEL)drivers/sstar/include/ -I$(KERNEL)/drivers/sstar/include/$(CONFIG_SSTAR_CHIP_NAME)
USER_CFLAGS := -Wall -Werror -Wno-sync-nand -g -O2 -ffunction-sections -funwind-tables -fstack-protector -I../pub/
USER_SHARE_LDFLAGS := -L$(PWD)/../.build/lib -lcam_os_wrapper -lpthread
USER_STATIC_LDFLAGS := ../.build/lib/libcam_os_wrapper.a -ldl -lpthread
all:
	make modules -C $(KERNEL)  M=`pwd`
	#$(CROSS_COMPILE)strip -g cam_os_wrapper_linux_ktest.ko
	$(CC) $(USER_CFLAGS) $(APP).c $(USER_SHARE_LDFLAGS) -o $(APP)_share
	$(CC) $(USER_CFLAGS) $(APP).c $(USER_STATIC_LDFLAGS) -o $(APP)_static

clean:
	make modules clean -C $(KERNEL)  M=`pwd`
	rm -rf $(APP)

