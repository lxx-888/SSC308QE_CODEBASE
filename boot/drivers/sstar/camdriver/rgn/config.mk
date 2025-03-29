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

include raw-impl-config.mk
DEPS:=common cmdq
MACROS$(os_linux)+=

RGNHAL_DIR := $(subst ",,$(CONFIG_SSTAR_CHIP_NAME))

ENV_INC_PATH$(os_rtos):=system/fc system/sys
DRV_PUB_INCS:=pub/mhal_rgn.h pub/mhal_rgn_datatype.h
DRV_INC_PATH:=pub inc ../hal/$(RGNHAL_DIR)/inc/ ../hal/common/
DRV_INC_PATH$(os_linux)+=inc/linux

ifneq ($(filter rtos,$(IPC_ROUTER)),)
DRV_INC_PATH$(os_rtos)+=inc/rtk
endif

DRV_SRCS:=src/mhal_rgn.c
#DRV_SRCS+=src/drv_rgn_ctx.c
DRV_SRCS+=src/linux/drv_rgn_export.c
#DRV_SRCS+=src/linux/drv_rgn_os.c
#DRV_SRCS+=src/drv_rgn_clk.c
#DRV_SRCS+=src/drv_rgn_if.c
#DRV_SRCS+=src/linux/drv_rgn_module.c
DRV_SRCS+=src/linux/rgn_module.c
DRV_SRCS+=src/linux/rgn_sysfs.c


#ifneq ($(filter rtos,$(IPC_ROUTER)),)
#DRV_SRCS$(os_rtos)+=src/rtk/drv_rgn_os.c
#endif

HAL_PUB_PATH:=$(RGNHAL_DIR)/inc/ ../drv/inc/ ../drv/inc/linux ../drv/pub/
#HAL_PUB_PATH$(os_linux)+=../drv/inc/linux
#ifneq ($(filter rtos,$(IPC_ROUTER)),)
#HAL_PUB_PATH$(os_rtos)+=../drv/inc/rtk
#endif

HAL_INC_PATH:=$(RGNHAL_DIR)/inc/ common/

HAL_SRCS+=$(RGNHAL_DIR)/src/hal_rgn_if.c
HAL_SRCS+=$(RGNHAL_DIR)/src/hal_rgn_cmdq.c
HAL_SRCS+=$(RGNHAL_DIR)/src/hal_rgn_gop.c
HAL_SRCS+=$(RGNHAL_DIR)/src/hal_rgn_cover.c
ifeq ($(SS_ARCH_NAME),infinity6f)
HAL_SRCS+=$(RGNHAL_DIR)/src/hal_rgn_frame.c
HAL_SRCS+=$(RGNHAL_DIR)/src/hal_rgn_ci.c
endif
HAL_SRCS+=$(RGNHAL_DIR)/src/hal_rgn_ifcd.c


#CLANG_TIDY_ENABLE=yes
#CLANG_FORMAT_ENABLE=yes

include add-config.mk
