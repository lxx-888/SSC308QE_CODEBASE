/*
 * sstar_fb_getvar.c - Sigmastar
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

#include <common.h>
#include <env.h>
#include <android_ab.h>
#include <stdlib.h>
#include <malloc.h>
#include <part.h>
#include "sstar_fastboot.h"

static bool str_suffix(char *str, const char *suffix)
{
	char *sub_str;

	if (strlen(suffix) >= strlen(str))
		return false;

	sub_str = str + (strlen(str) - strlen(suffix));
	return strcmp(sub_str, suffix) ? false : true;
}

static struct part_driver *part_get_driver(struct blk_desc *dev_desc)
{
	struct part_driver *entry = part_driver_get_first();
	int    part_drv_count = part_driver_get_count();
	int    i;

	if (entry == NULL || part_drv_count == 0)
		return NULL;

	if (dev_desc->part_type == PART_TYPE_UNKNOWN) {
		for (i = 0; i < part_drv_count; i++) {
			int ret;

			ret = entry->test(dev_desc);
			if (!ret) {
				dev_desc->part_type = entry->part_type;
				return entry;
			}
			entry++;
		}
	} else {
		for (i = 0; i < part_drv_count; i++) {
			if (dev_desc->part_type == entry->part_type)
				return entry;
			entry++;
		}
	}

	return NULL;
}

static void getvar_all(char *var_parameter, char *response)
{
	struct blk_desc *dev_desc;
	struct part_driver *part_drv;
	struct disk_partition part_info;
	struct disk_partition next_part_info;
	char *part_name;
	char *next_part_name;
	char part_name_prefix[PART_NAME_LEN];
	int i;
	int ret;

	//get all of FASTBOOT_FLASH_MMC_DEV part name
	dev_desc = blk_get_dev(CONFIG_SSTAR_BOOT_DEV, CONFIG_SSTAR_BOOT_DEV_ID);
	if (!dev_desc)
		goto fail;

	part_drv = part_get_driver(dev_desc);
	if (!part_drv)
		goto fail;

	for (i = 1; i < part_drv->max_entries; i++) {
		ret = part_drv->get_info(dev_desc, i, &part_info);

		if (ret < 0) {
			break;
		}

		part_name = &part_info.name[0];
		snprintf(response, FASTBOOT_RESPONSE_LEN, "partition-size:%s:%#lx",
				part_name,
				part_info.size * part_info.blksz);
		if (fastboot_sendinfo_callback)
			fastboot_sendinfo_callback(response);
	}

	/* has-slot
	 * By default, if a partition has slot,
	 * slot a and slot b must be adjacent partitions
	 */
	for (i = 1; i < part_drv->max_entries; i++) {
		ret = part_drv->get_info(dev_desc, i, &part_info);
		if (ret < 0) {
			break;
		}

		part_name = &part_info.name[0];

		if (str_suffix(part_name, "_a")) {
			strncpy(part_name_prefix, part_name, strlen(part_name)-2);
			part_name_prefix[strlen(part_name)-2] = '\0';

			ret = part_drv->get_info(dev_desc, i+1, &next_part_info);
			if (ret < 0) {
				snprintf(response, FASTBOOT_RESPONSE_LEN, "has-slot:%s:no", part_name_prefix);
			} else {
				next_part_name = &next_part_info.name[0];
				if (!strncmp(part_name_prefix, next_part_name, strlen(part_name_prefix)) &&
						(str_suffix(next_part_name, "_b"))) {
					snprintf(response, FASTBOOT_RESPONSE_LEN, "has-slot:%s:yes", part_name_prefix);
					i++;
				} else {
					snprintf(response, FASTBOOT_RESPONSE_LEN, "has-slot:%s:no", part_name_prefix);
				}
			}
		} else {
			snprintf(response, FASTBOOT_RESPONSE_LEN, "has-slot:%s:no", part_name);
		}
		if (fastboot_sendinfo_callback)
			fastboot_sendinfo_callback(response);
	}

	//TODO:is-logical

	fastboot_okay(NULL, response);
	return;

fail:
	fastboot_fail("fail to getvar all", response);
}

static void getvar_current_slot(char *var_parameter, char *response)
{
	struct bootloader_control *abc = NULL;
	char misc_part_str[16] = {'\0'};
	struct blk_desc *dev_desc;
	struct disk_partition part_info;

	sprintf(misc_part_str, "%d#%s", CONFIG_SSTAR_BOOT_DEV_ID, CONFIG_SSTAR_MISC_PART_NAME);
	if (part_get_info_by_dev_and_name_or_num(CONFIG_SSTAR_BOOT_DEV,
				misc_part_str,
				&dev_desc, &part_info, false) < 0) {
		goto fail;
	}

	if (ab_control_create_from_disk(dev_desc, &part_info, &abc) < 0) {
		goto fail;
	}

	fastboot_response("OKAY", response, "%c", abc->slot_suffix[1]);
	free(abc);
	return;

fail:
	fastboot_fail("fail to get A/B metadata", response);
}

static void getvar_slot_count(char *var_parameter, char *response)
{
	struct bootloader_control *abc = NULL;
	char misc_part_str[16] = {'\0'};
	struct blk_desc *dev_desc;
	struct disk_partition part_info;


	sprintf(misc_part_str, "%d#%s", CONFIG_SSTAR_BOOT_DEV_ID, CONFIG_SSTAR_MISC_PART_NAME);
	if (part_get_info_by_dev_and_name_or_num(CONFIG_SSTAR_BOOT_DEV,
				misc_part_str,
				&dev_desc, &part_info, false) < 0) {
		goto fail;
	}

	if (ab_control_create_from_disk(dev_desc, &part_info, &abc) < 0) {
		goto fail;
	}

	fastboot_response("OKAY", response, "%d", abc->nb_slot);
	free(abc);
	return;

fail:
	fastboot_fail("fail to get A/B metadata", response);
}

getvar_dispatch_desc sstar_getvar_dispatch[] = {
	{
		.variable = "all",
		.dispatch = getvar_all
	}, {
		.variable = "current-slot",
		.dispatch = getvar_current_slot
	}, {
		.variable = "slot-count",
		.dispatch = getvar_slot_count
	}, {
		.variable = NULL,
		.dispatch = NULL
	}
};
