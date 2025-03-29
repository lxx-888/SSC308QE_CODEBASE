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

SS_ARCH_NAME := $(subst ",,$(CONFIG_SYS_CONFIG_NAME))

# general options
ccflags-y                +=  -I$(srctree)/$(src)/../include/$(SS_ARCH_NAME)
ccflags-y                +=  -I$(srctree)/$(src)/../../../include/linux
ccflags-y                +=  -I$(srctree)/$(src)/../../../include/
ccflags-y                +=  -I$(srctree)/$(src)/../camdriver/common
ccflags-y             	 +=  -I$(srctree)/$(src)/../

#----------------------- drv -----------------------
ccflags-y               +=  -I$(srctree)/$(src)/drv/inc
ccflags-y               +=  -I$(srctree)/$(src)/drv/pub

#----------------------- hal -----------------------
ccflags-y                +=  -I$(srctree)/$(src)/hal/common
ccflags-y                +=  -Idrivers/sstar/aio/hal/$(SS_ARCH_NAME)/pub
ccflags-y                +=  -Idrivers/sstar/aio/hal/$(SS_ARCH_NAME)/inc
# specific options
#EXTRA_CFLAGS += -DUSE_PHYSICAL_ADDR
#$(SS_ARCH_NAME)
#--------------------- sources ---------------------
obj-y       := aio.o
aio-y :=  \
	drv/src/common/mhal_audio.o \
	drv/src/common/drv_audio.o \
	hal/$(SS_ARCH_NAME)/src/hal_audio.o \
	hal/$(SS_ARCH_NAME)/src/hal_audio_sys.o \
	hal/$(SS_ARCH_NAME)/src/hal_audio_reg.o \
	hal/$(SS_ARCH_NAME)/src/hal_audio_config.o \
	drv/src/common/audio_lineout.o

#hal/infinity7/src/hal_audio_os_api_linux.o \
