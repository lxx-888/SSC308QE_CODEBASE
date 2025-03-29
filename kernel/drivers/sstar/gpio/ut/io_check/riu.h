/*
 * riu.h- Sigmastar
 *
 * Copyright (c) [2019~2020] SigmaStar Technology.
 *
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License version 2 for more details.
 *
 */

#ifndef RIU_H
#define RIU_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <libgen.h>
#include <stdbool.h>

bool riu_w(unsigned long bank, unsigned long offset, unsigned short content, bool show_tag);
bool riu_r(unsigned long bank, unsigned long offset, unsigned short *content, bool show_tag);

bool          riu_write(unsigned long phy_addr, unsigned short val, unsigned int mask, bool mask_flag);
unsigned long riu_read(unsigned long phy_addr, unsigned int mask, bool mask_flag);

#endif
