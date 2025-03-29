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

obj-m       := cam_fs_test.o
cam_fs_test-objs := cam_fs_linux_kernel_test.o

#export ARCH=arm
#export CROSS_COMPILE=arm-linux-gnueabihf-

KERNEL = $(PWD)/../../../../
CC  = $(CROSS_COMPILE)gcc
APP = cam_fs_linux_user_test
EXTRA_CFLAGS := -I$(KERNEL)/drivers/sstar/include/ -I$(KERNEL)/include/
USER_CFLAGS := -Wall -Werror -g -O2 -ffunction-sections -funwind-tables -fstack-protector -I../pub/ -I$(KERNEL)/drivers/sstar/include/
USER_STATIC_LDFLAGS := ../.build/lib/libcam_fs_wrapper.a -ldl -lpthread

all:
	make modules -C $(KERNEL)  M=`pwd`
	$(CC) $(USER_CFLAGS) $(APP).c $(USER_STATIC_LDFLAGS) -o $(APP)

clean:
	make modules clean -C $(KERNEL)  M=`pwd`
	rm -rf $(APP)
