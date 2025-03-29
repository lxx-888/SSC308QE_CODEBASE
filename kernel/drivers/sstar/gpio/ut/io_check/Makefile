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
# Makefile for MStar GPIO device drivers.
#

export CC LD CFLAGS
KERNEL = $(CURDIR)/../../../../..

CHIP_NAME = $(shell grep -oP 'CONFIG_SSTAR_CHIP_NAME="\K[^"]+' $(KERNEL)/.config)

INCLUDES := -I$(KERNEL)/include
INCLUDES += -I$(KERNEL)/drivers/sstar/include
INCLUDES += -I$(KERNEL)/drivers/sstar/include/$(CHIP_NAME)/
INCLUDES += -I$(KERNEL)/drivers/sstar/gpio/$(CHIP_NAME)/
INCLUDES += -I./

TARGET_NAME  = prog_io_check

C_SRCS    :=  $(wildcard *.c */*.c)
C_SRCS    +=  $(KERNEL)/drivers/sstar/gpio/$(CHIP_NAME)/hal_pinmux.c
C_SRCS    +=  $(KERNEL)/drivers/sstar/gpio/$(CHIP_NAME)/hal_gpio_common.c
C_OBJS    = $(patsubst %.c, %.c.o, $(C_SRCS))
GCC = $(CROSS_COMPILE)gcc -O2 -Wall -g

all: clean $(TARGET_NAME)

$(TARGET_NAME):$(C_OBJS)
	@echo "generate $@"
	@$(GCC) -o $@ $(C_OBJS)

%.c.o : %.c
	@echo "sanitize $<"
	@echo "compile $@"
	$(GCC) $(C_FLAGS) $(CFLAGS) $(INCLUDES) -c $< -o $@ -DCHIP_NAME=\"${CHIP_NAME}\"

clean:
	-rm -rf *.o
	-rm -rf $(KERNEL)/drivers/sstar/gpio/$(CHIP_NAME)/src/*.o
	-rm -rf $(TARGET_NAME)

.PHONY: all clean
