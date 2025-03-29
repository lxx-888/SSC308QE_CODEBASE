// SPDX-License-Identifier: BSD-2-Clause
/*
 * Copyright (C) 2016 The Android Open Source Project
 */

#include <android_bootloader.h>
#include <common.h>
#include <command.h>
#include <part.h>

static int boot_android(struct cmd_tbl *cmdtp, int flag, int argc,
			char * const argv[], bool verify)
{
	unsigned long load_address;
	int ret = CMD_RET_SUCCESS;
	char *addr_arg_endp, *addr_str;
	struct blk_desc *dev_desc;
	struct blk_desc *persistant_dev_desc = NULL;
	struct disk_partition part_info;
	const char *misc_part_iface;
	const char *misc_part_desc;
	const char *slot = NULL;
	if (argc < 3)
		return CMD_RET_USAGE;
	if (argc > 5)
		return CMD_RET_USAGE;

	if (argc >= 5) {
		load_address = simple_strtoul(argv[4], &addr_arg_endp, 16);
		if (addr_arg_endp == argv[4] || *addr_arg_endp != '\0')
			return CMD_RET_USAGE;
	} else {
		addr_str = env_get("loadaddr");
		if (addr_str)
			load_address = simple_strtoul(addr_str, NULL, 16);
		else
			load_address = CONFIG_SYS_LOAD_ADDR;
	}
	if (argc >= 4)
		slot = argv[3];

	/* Lookup the "misc" partition from argv[1] and argv[2] */
	misc_part_iface = argv[1];
	misc_part_desc = argv[2];
	/* Split the part_name if passed as "$dev_num;part_name". */
	if (part_get_info_by_dev_and_name_or_num(misc_part_iface,
						 misc_part_desc,
						 &dev_desc, &part_info,
						 false) < 0)
		return CMD_RET_FAILURE;
#ifdef CONFIG_ANDROID_PERSISTENT_RAW_DISK_DEVICE
	/* Get the persistent disk that contains the bootconfig partition */
	persistant_dev_desc = blk_get_dev(misc_part_iface, CONFIG_ANDROID_PERSISTENT_RAW_DISK_DEVICE);
	if (!persistant_dev_desc) {
		printf("Failed to get blk device with dev_num: %d\n", CONFIG_ANDROID_PERSISTENT_RAW_DISK_DEVICE);
		return CMD_RET_FAILURE;
	}
#endif /* CONFIG_ANDROID_PERSISTENT_RAW_DISK_DEVICE */
	ret = android_bootloader_boot_flow(misc_part_iface, misc_part_desc,
					   dev_desc, &part_info, slot, verify,
					   load_address, persistant_dev_desc);
	if (ret < 0) {
		printf("Android boot failed, error %d.\n", ret);
		return CMD_RET_FAILURE;
	}
	return CMD_RET_SUCCESS;
}

static int do_boot_android(struct cmd_tbl *cmdtp, int flag, int argc,
			   char * const argv[]) {
	bool verify = false;
	return boot_android(cmdtp, flag, argc, argv, verify);
}

static int do_verified_boot_android(struct cmd_tbl *cmdtp, int flag, int argc,
				    char * const argv[]) {
	bool verify = true;
	return boot_android(cmdtp, flag, argc, argv, verify);
}

U_BOOT_CMD(
	boot_android, 5, 0, do_boot_android,
	"Execute the Android Bootloader flow.",
	"<interface> <dev[:part|;part_name]> [<slot>] [<kernel_addr>]\n"
	"    - Load the Boot Control Block (BCB) from the partition 'part' on\n"
	"      device type 'interface' instance 'dev' to determine the boot\n"
	"      mode, and load and execute the appropriate kernel.\n"
	"      In normal and recovery mode, the kernel will be loaded from\n"
	"      the corresponding \"boot\" partition. In bootloader mode, the\n"
	"      command defined in the \"fastbootcmd\" variable will be\n"
	"      executed.\n"
	"      On Android devices with multiple slots, the pass 'slot' is\n"
	"      used to load the appropriate kernel. The standard slot names\n"
	"      are 'a' and 'b'.\n"
	"    - If 'part_name' is passed, preceded with a ; instead of :, the\n"
	"      partition name whose label is 'part_name' will be looked up in\n"
	"      the partition table. This is commonly the \"misc\" partition.\n"
);
U_BOOT_CMD(
	verified_boot_android, 5, 0, do_verified_boot_android,
	"Execute the Android Verified Boot flow. The arguments are the same \n",
	"as \"boot_android\"."
);
