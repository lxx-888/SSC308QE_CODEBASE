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

CONFIG_SYS_CONFIG_NAME := $(subst ",,$(CONFIG_SYS_CONFIG_NAME))


ccflags-y += -I$(srctree)/$(src)/nand/
ccflags-y += -I$(srctree)/$(src)/../fcie/
ccflags-y += -I$(srctree)/$(src)/../fcie/hal/
ccflags-y += -I$(srctree)/$(src)/nor/
ccflags-y += -I$(srctree)/$(src)/
ccflags-y += -I$(srctree)/$(src)/os/
ccflags-y += -I$(srctree)/arch/arm/include/asm/arch-$(CONFIG_SYS_CONFIG_NAME)/mach
ccflags-y += -I$(srctree)/$(src)/../bdma/$(CONFIG_SYS_CONFIG_NAME)

obj-$(CONFIG_SSTAR_FLASH) += cis.o
obj-$(CONFIG_SSTAR_FLASH) += spi_flash_controller.o
obj-$(CONFIG_SSTAR_FLASH) += drv_part.o
obj-$(CONFIG_SSTAR_FLASH) += os/drv_flash_os_impl.o

obj-$(CONFIG_SSTAR_FLASH_DM) += flash_blk.o

obj-$(CONFIG_SSTAR_NAND) += nand/mdrv_spinand.o
obj-$(CONFIG_SSTAR_NAND) += nand/drv_spinand.o
obj-$(CONFIG_SSTAR_NAND) += nand/mtd_spinand.o

obj-$(CONFIG_SSTAR_NOR) += nor/mdrv_spinor.o
obj-$(CONFIG_SSTAR_NOR) += nor/drv_spinor.o
obj-$(CONFIG_SSTAR_NOR) += nor/mtd_spinor.o
