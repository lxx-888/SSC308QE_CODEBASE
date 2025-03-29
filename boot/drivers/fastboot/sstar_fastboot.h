/*
 * sstar_fastboot.h - Sigmastar
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

#ifndef _SSTAR_FASTBOOT_H_
#define _SSTAR_FASTBOOT_H_

#include <fastboot.h>
#include <android_bootloader_message.h>
#include <blk.h>
#include <part.h>
#include <fastboot-internal.h>

typedef struct {
	const char *command;
	void (*dispatch)(char *cmd_parameter, char *response);
} fastboot_cmd_desc;
extern fastboot_cmd_desc sstar_commands[];

typedef struct {
	const char *variable;
	void (*dispatch)(char *var_parameter, char *response);
} getvar_dispatch_desc;
extern getvar_dispatch_desc sstar_getvar_dispatch[];

/* export functions from android_ab.c */
extern int ab_control_create_from_disk(struct blk_desc *dev_desc,
				       const struct disk_partition *part_info,
				       struct bootloader_control **abc);
extern uint32_t ab_control_compute_crc(struct bootloader_control *abc);
extern int ab_control_store(struct blk_desc *dev_desc,
			    const struct disk_partition *part_info,
			    struct bootloader_control *abc);

#endif /* _SSTAR_FASTBOOT_H_ */
