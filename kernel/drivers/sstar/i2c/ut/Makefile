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

export CC LD CFLAGS
KERNEL = $(CURDIR)../../../../
ALL_TARGET := ut_i2cdev i2c_read_write
GCC = $(CROSS_COMPILE)gcc -O2 -Wall -g

all: $(ALL_TARGET)

ut_i2cdev: ut_i2cdev.c
	$(GCC) -o $@ $<

i2c_read_write: i2c_read_write.c
	$(GCC) -o $@ $<

clean:
	-rm $(ALL_TARGET)

.PHONY: all clean
