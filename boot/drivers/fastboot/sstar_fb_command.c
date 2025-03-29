/*
 * sstar_fb_command.c - Sigmastar
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
#include <command.h>
#include <malloc.h>
#include <fs.h>
#include "sstar_fastboot.h"
#include <sstar_rpmb.h>


#define MAX_KEY_LEN          32
#define MAX_VALUE_LEN        64
#define MAX_SERAIL_NO_NUM    6
#define MAX_FILE_BUFFER_SIZE 512
#define SERIAL_NO_FILE_PATH  "/serial_number"
#define HDCP_KEY_FILE_PATH   "/hdcp.bin"

struct serial_number  {
	char key[MAX_KEY_LEN];
	char value[MAX_VALUE_LEN];
};

static void oem_run_cmd(char *cmd_parameter, char *response)
{
	char cmd_buf[64] = {'\0'};
	sprintf(cmd_buf, "%s", cmd_parameter);
	if (run_command(cmd_buf, 0))
		fastboot_response("FAIL", response, "command %s not recognized.", cmd_buf);
	else
		fastboot_okay(NULL, response);
	return;
}

static void oem_auth(char *cmd_parameter, char *response)
{
#ifdef CONFIG_SSTAR_FASTBOOT_PERMISSION_CONTROL
	// TODO
	fastboot_response("FAIL", response, "Current oem auth not support");
#else
	fastboot_response("FAIL", response, "Current oem auth not support");
#endif
	return;
}

static void oem_get_lock_state(char *cmd_parameter, char *response)
{
#ifdef CONFIG_SSTAR_RPMB
	uint64_t lock_flags = 0;
	int ret = 0;

	ret = sstar_rpmb_get_lock_flags(&lock_flags);
	if (ret)
	{
		fastboot_response("FAIL", response, "Fail to get fastboot flags");
		return;
	}

	snprintf(response, FASTBOOT_RESPONSE_LEN, "unlocked:%lld", lock_flags & 0x01);
	if (fastboot_sendinfo_callback)
		fastboot_sendinfo_callback(response);

	snprintf(response, FASTBOOT_RESPONSE_LEN, "unlock_critical:%lld", (lock_flags >> 1) & 0x01);
	if (fastboot_sendinfo_callback)
		fastboot_sendinfo_callback(response);

	fastboot_okay(NULL, response);
	return;
#else
	fastboot_response("FAIL", response, "Current oem get_lock_state not support");
#endif
	return;
}

static void set_active(char *cmd_parameter, char *response)
{
#ifdef CONFIG_SSTAR_ANDROID_AB
	struct bootloader_control *abc = NULL;
	char misc_part_str[16] = {'\0'};
	struct blk_desc *dev_desc;
	struct disk_partition part_info;
	int active_slot_index = cmd_parameter[0] - 'a';

	if ((active_slot_index < 0) || (active_slot_index > 1)) {
		goto fail;
	}

	sprintf(misc_part_str, "%d#%s", CONFIG_SSTAR_BOOT_DEV_ID, CONFIG_SSTAR_MISC_PART_NAME);
	if (part_get_info_by_dev_and_name_or_num(CONFIG_SSTAR_BOOT_DEV,
				misc_part_str,
				&dev_desc, &part_info, false) < 0) {
		goto fail;
	}

	if (ab_control_create_from_disk(dev_desc, &part_info, &abc) < 0) {
		goto fail;
	}

	abc->slot_info[active_slot_index].priority = 15;
	abc->slot_info[active_slot_index].tries_remaining = 3;
	abc->slot_info[active_slot_index].successful_boot = 0;
	abc->slot_info[active_slot_index].verity_corrupted = 0,
	abc->slot_info[active_slot_index].reserved = 0;
	abc->slot_suffix[1] = 'a' + active_slot_index;

	/* Lower the priority of another slot. */
	for (int i = 0; i < abc->nb_slot; ++i) {
		if (i == active_slot_index)
			continue;
		if (abc->slot_info[i].priority >= 15)
			abc->slot_info[i].priority = 15 - 1;
	}

	abc->crc32_le = ab_control_compute_crc(abc);
	ab_control_store(dev_desc, &part_info, abc);
	fastboot_okay(NULL, response);
	free(abc);
	return;

fail:
	fastboot_response("FAIL", response, "fail to set slot %s active", cmd_parameter);
#else
	return;
#endif
}

static void fastboot_locking_entry(bool lock, bool critical, char *response)
{
#ifdef CONFIG_SSTAR_FASTBOOT_PERMISSION_CONTROL
	// TODO:
	// Set the current permission level with the command "fastboot oem auth"
	// Refuse the lock/unlock operation if not currently at a qualifying level.
	// Refuse unlock critical if not do unlock first.
#endif

#ifdef CONFIG_SSTAR_RPMB
	uint64_t lock_flags = 0;
	int ret = 0;

	ret = sstar_rpmb_get_lock_flags(&lock_flags);
	if (ret)
	{
		fastboot_response("FAIL", response, "Fail to get fastboot flags");
		return;
	}

	/* set 0 lock, set 1 unlock */
	if (lock)
		lock_flags &= ~(1ULL << critical);
	else
		lock_flags |= 1 << critical;

	ret = sstar_rpmb_set_lock_flags(lock_flags);
	if (ret)
	{
		fastboot_response("FAIL", response, "Fail to set fastboot flags");
		return;
	}
	fastboot_okay(NULL, response);
#else
	fastboot_response("FAIL", response, "Current fastboot lock/unlock not support");
#endif
	return;
}

static void flashing_lock(char *cmd_parameter, char *response)
{
	fastboot_locking_entry(true, false, response);
	return;
}

static void flashing_unlock(char *cmd_parameter, char *response)
{
	fastboot_locking_entry(false, false, response);
	return;
}

static void flashing_lock_critical(char *cmd_parameter, char *response)
{
	fastboot_locking_entry(true, true, response);
	return;
}

static void flashing_unlock_critical(char *cmd_parameter, char *response)
{
	fastboot_locking_entry(false, true, response);
	return;
}

static void flashing_get_unlock_ability(char *cmd_parameter, char *response)
{
#ifdef CONFIG_SSTAR_RPMB
	/* Current we always allow unlock */
	snprintf(response, FASTBOOT_RESPONSE_LEN, "%d", true);
	if (fastboot_sendinfo_callback)
		fastboot_sendinfo_callback(response);
	fastboot_okay(NULL, response);
#else
	fastboot_response("FAIL", response, "Current fastboot get_unlock_ability not support");
#endif
	return;
}

static void oem_get_serial_no(char *cmd_parameter, char *response)
{
	loff_t  act_read                           = 0;
	char    key[MAX_KEY_LEN]                   = {0};
	char    val[MAX_VALUE_LEN]                 = {0};
	char    file_buf[MAX_FILE_BUFFER_SIZE]     = {0};
	char    *buf                               = NULL;
	char    *fail_reason                       = NULL;
	struct  serial_number serial_nos[MAX_SERAIL_NO_NUM];
	uint8_t serial_no_count                    = 0;

	if (cmd_parameter == NULL)
	{
		fail_reason = "bad cmd parameter";
		goto fail;
	}

	memset(serial_nos, 0x0, sizeof(serial_nos));
	fs_set_blk_dev("mmc", "0#factory", FS_TYPE_EXT);
	if (fs_read(SERIAL_NO_FILE_PATH, (ulong)file_buf, 0, 0, &act_read) != 0)
	{
		fail_reason = "read serial number file error";
		goto fail;
	}

	buf = strtok(file_buf,"\n");
	while(buf)
	{
		if (sscanf(buf, "%[^=]=%s", key, val) == 2)
		{
			if (serial_no_count < MAX_SERAIL_NO_NUM)
			{
				snprintf(serial_nos[serial_no_count].key, MAX_KEY_LEN, "%s", key);
				snprintf(serial_nos[serial_no_count].value, MAX_VALUE_LEN, "%s", val);
				serial_no_count++;
			}
		}

		buf = strtok(NULL,"\n");
	}

	for (int i = 0; i < serial_no_count; i++)
	{
		if (strncmp(cmd_parameter, serial_nos[i].key, strlen(cmd_parameter)) == 0)
		{
			snprintf(response, FASTBOOT_RESPONSE_LEN, "%s:%s", cmd_parameter, serial_nos[i].value);
			if (fastboot_sendinfo_callback)
				fastboot_sendinfo_callback(response);
			fastboot_okay(NULL, response);
			return;
		}
	}

fail:
	fastboot_response("FAIL", response, "Fail to get serial no(%s).", fail_reason);
}

static void oem_set_serial_no(char *cmd_parameter, char *response)
{
	int     find                               = 0;
	loff_t  act_rw                             = 0;
	char    key[MAX_KEY_LEN]                   = {0};
	char    val[MAX_VALUE_LEN]                 = {0};
	char    in_key[MAX_KEY_LEN]                = {0};
	char    in_val[MAX_VALUE_LEN]              = {0};
	char    key_val[MAX_KEY_LEN+MAX_VALUE_LEN] = {0};
	char    file_buf[MAX_FILE_BUFFER_SIZE]     = {0};
	char    *buf                               = NULL;
	char    *fail_reason                       = NULL;
	struct  serial_number serial_nos[MAX_SERAIL_NO_NUM];
	uint8_t serial_no_count                    = 0;

	if (cmd_parameter == NULL)
	{
		fail_reason = "cmd parameter null";
		goto fail;
	}

	if (sscanf(cmd_parameter, "%[^#]#%s", in_key, in_val) != 2)
	{
		fail_reason = "bad cmd parameter";
		goto fail;
	}

	memset(serial_nos, 0x0, sizeof(serial_nos));
	fs_set_blk_dev("mmc", "0#factory", FS_TYPE_EXT);
	if (fs_read(SERIAL_NO_FILE_PATH, (ulong)file_buf, 0, 0, &act_rw) == 0)
	{
		buf = strtok(file_buf,"\n");
		while(buf)
		{
			if (sscanf(buf, "%[^=]=%s", key, val) == 2)
			{
				if (serial_no_count < MAX_SERAIL_NO_NUM)
				{
					snprintf(serial_nos[serial_no_count].key, MAX_KEY_LEN, "%s", key);
					snprintf(serial_nos[serial_no_count].value, MAX_VALUE_LEN, "%s", val);
					serial_no_count++;
				}
			}

			buf = strtok(NULL,"\n");
		}

		for (int i=0; i<serial_no_count; i++)
		{
			if (strncmp(in_key, serial_nos[i].key, strlen(in_key)) == 0)
			{
				find = 1;
				snprintf(serial_nos[i].value, MAX_VALUE_LEN, "%s", in_val);
			}
		}

		if (!find)
		{
			if (serial_no_count < MAX_SERAIL_NO_NUM)
			{
				snprintf(serial_nos[serial_no_count].key, MAX_KEY_LEN, "%s", in_key);
				snprintf(serial_nos[serial_no_count].value, MAX_VALUE_LEN, "%s", in_val);
				serial_no_count++;
			}
		}

		memset(file_buf, 0x0, MAX_FILE_BUFFER_SIZE);
		for (int i = 0; i < serial_no_count; i++)
		{
			snprintf(key_val, MAX_KEY_LEN+MAX_VALUE_LEN, "%s=%s\n", serial_nos[i].key, serial_nos[i].value);
			strncat(file_buf, key_val, strlen(key_val));
		}
	}
	else
	{
		snprintf(file_buf, MAX_FILE_BUFFER_SIZE, "%s=%s", in_key, in_val);
	}

	fs_set_blk_dev("mmc", "0#factory", FS_TYPE_EXT);
	if (fs_write(SERIAL_NO_FILE_PATH, (ulong)file_buf, 0, strlen(file_buf), &act_rw) != 0)
	{
		fail_reason = "write serial number file error";
		goto fail;
	}

	fastboot_okay(NULL, response);
	return;

fail:
	fastboot_response("FAIL", response, "Fail to set serial no(%s).", fail_reason);
}

static void oem_set_hdcp_keys(char *cmd_parameter, char *response)
{
	loff_t  act_rw                             = 0;
	char    in_key[MAX_KEY_LEN]                = {0};
	char    in_val[MAX_VALUE_LEN]              = {0};
	char    file_buf[MAX_FILE_BUFFER_SIZE]     = {0};
	char    tmp[3]                             = {0};
	char    *fail_reason                       = NULL;
	int     write_len                          = 0;

	if (cmd_parameter == NULL)
	{
		fail_reason = "cmd parameter null";
		goto fail;
	}

	fs_set_blk_dev("mmc", "0#factory", FS_TYPE_EXT);
	fs_read(HDCP_KEY_FILE_PATH, (ulong)file_buf, 0, 0, &act_rw);
	if ( 2 == sscanf(cmd_parameter, "%[^#]#%s", in_key, in_val))
	{
		if( 0 != act_rw )
		{
			memset(file_buf, 0, MAX_FILE_BUFFER_SIZE);
			act_rw = 0;
		}
		strncpy(cmd_parameter, in_val, MAX_VALUE_LEN);
	}

	for(int i = 0; i < strlen(cmd_parameter)-1; i+=2 )
	{
		strncpy(tmp, cmd_parameter+i, 2);
		tmp[2] = '\0';

		file_buf[act_rw+write_len] = (char)simple_strtol(tmp, NULL, 16);
		write_len++;
	}

	fs_set_blk_dev("mmc", "0#factory", FS_TYPE_EXT);
	if (fs_write(HDCP_KEY_FILE_PATH, (ulong)file_buf, 0, act_rw+write_len, &act_rw) != 0)
	{
		fail_reason = "write hdcp key file error";
		goto fail;
	}

	fastboot_okay(NULL, response);
	return;

fail:
	fastboot_response("FAIL", response, "Fail to set hdcp key file(%s).", fail_reason);
}

/**
 * oem_emmc_wrrel() - Execute the OEM emmc_wrrel command
 *
 * @cmd_parameter: Pointer to command parameter
 * @response: Pointer to fastboot response buffer
 */
static void oem_emmc_wrrel(char *cmd_parameter, char *response)
{
	char cmdbuf[128];
	int err = -1;

	if (!cmd_parameter)
	{
		fastboot_fail("Expected command parameter", response);
		return;
	}

	if (!strcmp(cmd_parameter, "existence"))
	{
		fastboot_okay(NULL, response);
		return;
	}

	/* execute 'emmc_wrrel' command with cmd_parameter arguments */
	snprintf(cmdbuf, sizeof(cmdbuf), "emmc_wrrel %s fastboot", cmd_parameter);
	printf("Execute: %s\n", cmdbuf);
	err = run_command(cmdbuf, 0);
	if (err == 0)
	{
		fastboot_okay(NULL, response);
	} else
	{
		if (!strcmp(cmd_parameter, "on"))
			fastboot_fail("please power-cycle to make effective: turn on wrrel", response);
		else if (!strcmp(cmd_parameter, "off"))
			fastboot_fail("please power-cycle to make effective: turn off wrrel", response);
		else
			fastboot_fail("emmc_wrrel return error", response);
	}
}


fastboot_cmd_desc sstar_commands[] = {
	{
		.command = "oem run_cmd",
		.dispatch = oem_run_cmd
	}, {
		.command = "oem auth",
		.dispatch = oem_auth
	}, {
		.command = "oem get_lock_state",
		.dispatch = oem_get_lock_state
	}, {
		.command = "set_active",
		.dispatch = set_active
	}, {
		.command = "flashing lock",
		.dispatch = flashing_lock
	}, {
		.command = "flashing unlock",
		.dispatch = flashing_unlock
	}, {
		.command = "flashing lock_critical",
		.dispatch = flashing_lock_critical
	}, {
		.command = "flashing unlock_critical",
		.dispatch = flashing_unlock_critical
	}, {
		.command = "flashing get_unlock_ability",
		.dispatch = flashing_get_unlock_ability
	}, {
		.command = "oem get_serial_no",
		.dispatch = oem_get_serial_no
	}, {
		.command = "oem set_serial_no",
		.dispatch = oem_set_serial_no
	}, {
		.command = "oem set_hdcp_keys",
		.dispatch = oem_set_hdcp_keys
	}, {
		.command = "oem emmc_wrrel",
		.dispatch = oem_emmc_wrrel
	}, {
		.command = NULL,
		.dispatch = NULL
	}
};
