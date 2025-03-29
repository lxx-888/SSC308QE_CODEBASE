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
KERNEL = $(CURDIR)/../../../../
GCC = $(CROSS_COMPILE)gcc -O2 -Wall -g
EXTRA_CFLAGS += -I$(CURDIR)/../../pwm
EXTRA_CFLAGS +=  -I$(CURDIR)/../../include
EXTRA_CFLAGS += -I$(KERNEL)include/generated

SOURCES :=
ifeq ($(ut), ut_pwm)
	SOURCES += ut_pwm.c
endif
ifeq ($(ut), ut_spwm)
	SOURCES += ut_spwm.c
endif

#all: $(ALL_TARGET)

$(ut): $(SOURCES)
	$(GCC) -o $@ $< $(EXTRA_CFLAGS)

#ut_pwm: ut_pwm.c
#	$(GCC) -o $@ $< $(EXTRA_CFLAGS)

#ut_spwm: ut_spwm.c
#	$(GCC) -o $@ $< $(EXTRA_CFLAGS)

clean:
	-rm $(ut)

.PHONY: all clean