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

#-------------------------------------------------------------------------------
# Description of some variables owned by the library
#-------------------------------------------------------------------------------

PATH_C +=\
    $(PATH_cam_fs_wrapper)/src

PATH_H +=\
    $(PATH_cam_fs_wrapper)/pub\
    $(PATH_proxyfs)/pub\
    $(PATH_firmwarefs)/pub\
    $(PATH_littlefs)/pub\
    $(PATH_lwfs)/pub\
    $(PATH_bdma_hal)/inc\
    $(PATH_flash)/nand\
    $(PATH_flash)/nor\
    $(PATH_flash)/os\
    $(PATH_flash)/

#-------------------------------------------------------------------------------
# List of source files of the library or executable to generate
#-------------------------------------------------------------------------------
SRC_C_LIST =\
    cam_fs_wrapper.c \
    cam_fs_cli.c \
    cam_fs_test.c

FORCE_REL_H_PATH =\
    $(PATH_cam_fs_wrapper)/pub
