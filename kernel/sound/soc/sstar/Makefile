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

subdir-ccflags-y := -DSSTAR_CODING_STYLE_SANITIZE_MODULE
CONFIG_SSTAR_CHIP_NAME := $(subst ",,$(CONFIG_SSTAR_CHIP_NAME))

EXTRA_CFLAGS += -I$(srctree)/drivers/sstar/include
EXTRA_CFLAGS += -I$(srctree)/drivers/sstar/warp/drv/inc
EXTRA_CFLAGS += -I$(srctree)/sound/core
EXTRA_CFLAGS += -I$(srctree)/sound/soc/sstar/$(CONFIG_SSTAR_CHIP_NAME)
EXTRA_CFLAGS += -I$(srctree)/drivers/sstar/sar

# Audio input/output driver
ifneq ($(CONFIG_SSTAR_SND_SOC),)
obj-$(CONFIG_SSTAR_SND_SOC) +=  $(CONFIG_SSTAR_CHIP_NAME)/aio/
EXTRA_CFLAGS += -I$(srctree)/drivers/sstar/include/$(CONFIG_SSTAR_CHIP_NAME)
EXTRA_CFLAGS += -I$(srctree)/sound/soc/sstar/$(CONFIG_SSTAR_CHIP_NAME)/aio/drv/inc
EXTRA_CFLAGS += -I$(srctree)/sound/soc/sstar/$(CONFIG_SSTAR_CHIP_NAME)/aio/drv/pub
EXTRA_CFLAGS += -I$(srctree)/sound/soc/sstar/$(CONFIG_SSTAR_CHIP_NAME)/aio/hal/common
EXTRA_CFLAGS += -I$(srctree)/sound/soc/sstar/$(CONFIG_SSTAR_CHIP_NAME)/aio/hal/$(CONFIG_SSTAR_CHIP_NAME)/pub
EXTRA_CFLAGS += -I$(srctree)/sound/soc/sstar/$(CONFIG_SSTAR_CHIP_NAME)/aio/hal/$(CONFIG_SSTAR_CHIP_NAME)/inc
EXTRA_CFLAGS += -I$(srctree)/sound/soc/sstar/$(CONFIG_SSTAR_CHIP_NAME)/aio/hal/$(CONFIG_SSTAR_CHIP_NAME)/cmode
endif


# Platform drivers
sstar-snd-bach-objs := $(CONFIG_SSTAR_CHIP_NAME)/sstar_bach.o \
                           $(CONFIG_SSTAR_CHIP_NAME)/sstar_pcm.o

obj-$(CONFIG_SSTAR_SND_SOC) += sstar-snd-bach.o


# Machine drivers
sstar-snd-asoc-card-objs :=  $(CONFIG_SSTAR_CHIP_NAME)/sstar_asoc_card.o

obj-$(CONFIG_SSTAR_SND_SOC) += sstar-snd-asoc-card.o
