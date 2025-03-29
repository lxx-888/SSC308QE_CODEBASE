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
	$(PATH_camdriver)/aio/drv/src/common

PATH_H += \
	$(PATH_camdriver)/aio/drv/pub \
	$(PATH_camdriver)/aio/drv/inc \
	$(PATH_driver)/hal/infinity/int/pub \
	$(PATH_system)/MsWrapper/pub

#-------------------------------------------------------------------------------
#   List of source files of the library or executable to generate
#-------------------------------------------------------------------------------
SRC_C_LIST += \
	abi_headers_aio.c \
	mhal_audio.c \
	isw_audio.c \
	isw_audio_data.c \
	audio_proc.c
