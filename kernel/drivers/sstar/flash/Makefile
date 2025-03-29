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

subdir-ccflags-y := -DSSTAR_CODING_STYLE_SANITIZE_MODULE
CONFIG_SSTAR_CHIP_NAME := $(subst ",,$(CONFIG_SSTAR_CHIP_NAME))

obj-$(CONFIG_SSTAR_FLASH) += sstar_flash.o

obj-$(CONFIG_SSTAR_NAND) += sstar_spinand.o
obj-$(CONFIG_SSTAR_NOR) += sstar_spinor.o

# general options

EXTRA_CFLAGS      +=  -I$(srctree)/drivers/sstar/bdma/$(CONFIG_SSTAR_CHIP_NAME)
EXTRA_CFLAGS      +=  -I$(srctree)/drivers/sstar/include
EXTRA_CFLAGS      +=  -I$(srctree)/drivers/sstar/flash/
EXTRA_CFLAGS      +=  -I$(srctree)/drivers/sstar/flash/nand/
EXTRA_CFLAGS      +=  -I$(srctree)/drivers/sstar/flash/nor/
EXTRA_CFLAGS      +=  -I$(srctree)/drivers/sstar/flash/os/
EXTRA_CFLAGS      +=  -I$(srctree)/drivers/sstar/fsp_qspi/
EXTRA_CFLAGS      +=  -I$(srctree)/drivers/mtd
EXTRA_CFLAGS      +=  -I$(srctree)/drivers/sstar/fcie/
EXTRA_CFLAGS      +=  -I$(srctree)/drivers/sstar/fcie/hal/

# specific options
EXTRA_CFLAGS      +=  -DMSOS_TYPE_LINUX
EXTRA_CFLAGS      +=  -DENABLE_NAND_ONEBIN

sstar_flash-objs := spi_flash_controller.o \
                   cis.o \
                   os/drv_flash_os_impl.o
ifdef CONFIG_SSTAR_NAND
sstar_flash-objs += nand/mdrv_spinand.o
endif
ifdef CONFIG_SSTAR_NOR
sstar_flash-objs += nor/mdrv_spinor.o
endif

sstar_spinand-objs := nand/mtd_spinand.o

sstar_spinor-objs  := nor/mtd_spinor.o
