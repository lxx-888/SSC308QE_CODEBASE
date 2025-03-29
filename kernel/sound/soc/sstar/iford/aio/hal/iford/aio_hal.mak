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
PROCESS = lib

PATH_C += \
          $(PATH_camdriver)/aio/hal/$(BB_CHIP_ID)/src

PATH_H += \
          $(PATH_camdriver)/aio/hal/$(BB_CHIP_ID)/pub \
          $(PATH_camdriver)/aio/hal/$(BB_CHIP_ID)/inc \
          $(PATH_camdriver)/aio/hal/common

#-------------------------------------------------------------------------------
#   List of source files of the library or executable to generate
#-------------------------------------------------------------------------------
