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
include raw-impl-config.mk
SS_ARCH_NAME:=$(subst ",,$(CONFIG_SSTAR_CHIP_NAME))
DEPS:=common

DRV_PUB_INCS:=pub/mhal_audio.h pub/mhal_audio_datatype.h pub/mhal_audio_common.h
DRV_INC_PATH:=pub inc

DRV_SRCS:=src/common/mhal_audio.c
DRV_SRCS$(os_linux)+=src/linux/drv_audio_export.c
DRV_SRCS$(os_linux)+=src/linux/audio_proc.c

HAL_PUB_PATH:=$(SS_ARCH_NAME)/pub common
HAL_INC_PATH:=$(SS_ARCH_NAME)/inc
HAL_INC_PATH+=$(SS_ARCH_NAME)/../../drv/pub

HAL_SRCS:=$(SS_ARCH_NAME)/src/hal_audio.c
HAL_SRCS+=$(SS_ARCH_NAME)/src/hal_audio_os_api_linux.c
HAL_SRCS+=$(SS_ARCH_NAME)/src/hal_audio_sys.c
HAL_SRCS+=$(SS_ARCH_NAME)/src/hal_audio_reg.c
HAL_SRCS+=$(SS_ARCH_NAME)/src/hal_audio_config.c

#CLANG_TIDY_ENABLE=yes
#CLANG_FORMAT_ENABLE=yes

include add-config.mk
