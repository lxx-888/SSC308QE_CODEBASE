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

#
# Makefile for kernel IR UT_TEST.
#

KERNEL = $(pwd)../../../..
CC = $(CROSS_COMPILE)gcc -O2 -Wall -g
ALL_TARGETS := ut_ir

all: $(ALL_TARGETS)

ut_ir: ut_ir.c
	$(CC) -o $@ $<

clean:
	-rm $(ALL_TARGETS)

.PHONY: all clean


