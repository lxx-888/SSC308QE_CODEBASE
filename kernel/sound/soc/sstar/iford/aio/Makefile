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

CONFIG_SSTAR_CHIP_NAME := $(subst ",,$(CONFIG_SSTAR_CHIP_NAME))

EXTRA_CFLAGS += -I$(srctree)/drivers/sstar/include
EXTRA_CFLAGS += -I$(srctree)/drivers/sstar/warp/drv/inc
EXTRA_CFLAGS += -I$(srctree)/drivers/sstar/include/$(CONFIG_SSTAR_CHIP_NAME)
EXTRA_CFLAGS += -I$(srctree)/sound/soc/sstar/$(CONFIG_SSTAR_CHIP_NAME)/aio/drv/inc
EXTRA_CFLAGS += -I$(srctree)/sound/soc/sstar/$(CONFIG_SSTAR_CHIP_NAME)/aio/drv/pub
EXTRA_CFLAGS += -I$(srctree)/sound/soc/sstar/$(CONFIG_SSTAR_CHIP_NAME)/aio/hal/common
EXTRA_CFLAGS += -I$(srctree)/sound/soc/sstar/$(CONFIG_SSTAR_CHIP_NAME)/aio/hal/$(CONFIG_SSTAR_CHIP_NAME)/pub
EXTRA_CFLAGS += -I$(srctree)/sound/soc/sstar/$(CONFIG_SSTAR_CHIP_NAME)/aio/hal/$(CONFIG_SSTAR_CHIP_NAME)/inc
EXTRA_CFLAGS += -I$(srctree)/sound/soc/sstar/$(CONFIG_SSTAR_CHIP_NAME)/aio/hal/$(CONFIG_SSTAR_CHIP_NAME)/cmode
#EXTRA_CFLAGS += -DUSE_PHYSICAL_ADDR

snd-soc-sstar-aio-objs := \
    drv/src/common/mhal_audio.o \
    drv/src/linux/drv_audio_export.o \
    drv/src/linux/audio_proc.o \
    hal/$(CONFIG_SSTAR_CHIP_NAME)/src/hal_audio.o \
    hal/$(CONFIG_SSTAR_CHIP_NAME)/src/hal_atop.o \
    hal/$(CONFIG_SSTAR_CHIP_NAME)/src/hal_dmic.o \
    hal/$(CONFIG_SSTAR_CHIP_NAME)/src/hal_dma.o \
    hal/$(CONFIG_SSTAR_CHIP_NAME)/src/hal_i2s.o \
    hal/$(CONFIG_SSTAR_CHIP_NAME)/src/hal_sinegen.o \
    hal/$(CONFIG_SSTAR_CHIP_NAME)/src/hal_spdif.o \
    hal/$(CONFIG_SSTAR_CHIP_NAME)/src/hal_audio_os_api_linux.o \
    hal/$(CONFIG_SSTAR_CHIP_NAME)/src/hal_audio_sys.o \
    hal/$(CONFIG_SSTAR_CHIP_NAME)/src/hal_audio_reg.o \
    hal/$(CONFIG_SSTAR_CHIP_NAME)/src/hal_audio_config.o

obj-$(CONFIG_SSTAR_SND_SOC) += snd-soc-sstar-aio.o
