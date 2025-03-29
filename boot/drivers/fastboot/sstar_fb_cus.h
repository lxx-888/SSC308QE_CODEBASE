/*
 * sstar_fb_cus.h - Sigmastar
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

#ifndef _SSTAR_FB_CUS_H_
#define _SSTAR_FB_CUS_H_
#include <jffs2/load_kernel.h>

#define MAX_PARTNAME_LEN 20
typedef struct part_ops_s
{
    char partname[MAX_PARTNAME_LEN];
    int (*erase)(const char *partname);
    int (*write)(const char *partname, void *download_buffer, unsigned int file_size);
    int (*probe_info)(const char *partname, struct part_info *part_info);
} part_ops_t;

int fastboot_cus_get_part_info(const char *part_name,
                struct part_info *part_info, char *response);
void fastboot_cus_flash_write(const char *cmd, void *download_buffer,
			unsigned int download_bytes, char *response);
void fastboot_cus_erase(const char *cmd, char *response);

#endif

