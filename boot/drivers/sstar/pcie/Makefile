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

ccflags-y += -I$(srctree)/$(src)/intf/drv/pub
ccflags-y += -I$(srctree)/$(src)/intf/hal/$(CONFIG_SYS_CONFIG_NAME)/inc
ifdef CONFIG_SSTAR_PCIE
ccflags-y += -I$(srctree)/drivers/pci
endif
ifdef CONFIG_SSTAR_PCIE_ENDPOINT
ccflags-y += -I$(srctree)/$(src)/endpoint/drv/pub
ccflags-y += -I$(srctree)/$(src)/endpoint/hal/$(CONFIG_SYS_CONFIG_NAME)/inc
endif

obj-y += sstar_pcie_generic.o intf/drv/src/drv_pcieif.o

ifdef CONFIG_SSTAR_PCIE
obj-$(CONFIG_SSTAR_PCIE) += sstar_pcie.o
endif

ifdef CONFIG_SSTAR_PCIE_ENDPOINT
obj-$(CONFIG_SSTAR_PCIE_ENDPOINT) += endpoint/sstar_pcie_ep.o \
                                     endpoint/drv/src/drv_pcie_epc.o \
                                     endpoint/drv/src/drv_pcie_edma.o
endif
