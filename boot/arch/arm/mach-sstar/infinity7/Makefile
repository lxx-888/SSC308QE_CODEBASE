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

obj-y	:= chip.o
obj-y	+= board.o
ifdef CONFIG_ARM64
obj-y	+= reset64.o
else
obj-y	+= reset.o
endif
obj-$(CONFIG_SSTAR_PIU_TIMER)	+= timer.o
obj-$(CONFIG_SSTAR_BOOTING_PERF)	+= prof.o
