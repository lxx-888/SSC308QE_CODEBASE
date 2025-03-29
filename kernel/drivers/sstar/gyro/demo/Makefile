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
CC  = $(CROSS_COMPILE)gcc
CPP = $(CROSS_COMPILE)g++
AR  = $(CROSS_COMPILE)ar
PROJECT_DIR = $(srctree)../../../../../project

ifeq (arm,$(ARCH))
COM_FLAGS = -Wall -O2 -fPIC -mcpu=cortex-a7 -mfpu=neon-fp16 -mfloat-abi=hard -mthumb-interwork -marm -g
endif
C_FLAGS += $(COM_FLAGS) -I$(PROJECT_DIR)/../kernel/drivers/sstar/include/
C_FLAGS += $(COM_FLAGS) -I$(PROJECT_DIR)/../kernel/include/
CPP_FLAGS = $(COM_FLAGS) -std=c++11


#LIB_PATH  = -L.

TARGET_NAME  = sstar_gyro_demo

CPP_SRCS  =  $(wildcard *.cpp */*.cpp)
C_SRCS    =  $(wildcard *.c */*.c)

CPP_OBJS  = $(patsubst %.cpp, %.cpp.o, $(CPP_SRCS))
C_OBJS    = $(patsubst %.c, %.c.o, $(C_SRCS))
#LIB_NAME  = -lmi_sys -lmi_common

.PHONY: all prepare clean

all: prepare $(TARGET_NAME) install

prepare:
	@echo
	@echo "TARGET_NAME = $(TARGET_NAME)"
	@echo


clean:
	@rm -rf $(CPP_OBJS)
	@rm -ff $(C_OBJS)
	@rm -rf $(TARGET_NAME)

install:
	@echo "Nothing to do"

$(TARGET_NAME): $(CPP_OBJS) $(CPP_SRCS) $(C_OBJS) $(C_SRCS)
	@echo "generate $@"
	@$(CPP) -o $@ $(C_OBJS) $(CPP_OBJS) $(JSON_OBJS) $(LIB_PATH) $(LIB_NAME) -ldl -lm

%.c.o : %.c
	@echo "compile $@"
	@$(CC) $(C_FLAGS) $(INCLUDES) -c $< -o $@

%.cpp.o : %.cpp
	@echo "compile $@"
	@$(CPP) $(CPP_FLAGS) $(INCLUDES) -c $< -o $@


