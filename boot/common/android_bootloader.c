/*
 * Copyright (C) 2016 The Android Open Source Project
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <android_bootloader.h>
#include <android_bootloader_message.h>

#include <android_ab.h>
#include <cli.h>
#include <common.h>
#include <image.h>
#include <env.h>
#include <log.h>
#include <malloc.h>
#include <part.h>
#include <avb_verify.h>

#define ANDROID_PARTITION_BOOT "boot"
#define ANDROID_PARTITION_VENDOR_BOOT "vendor_boot"
#define ANDROID_PARTITION_RECOVERY "recovery"
#define ANDROID_PARTITION_SYSTEM "system"
#define ANDROID_PARTITION_BOOTCONFIG "bootconfig"

#define ANDROID_ARG_SLOT_SUFFIX "androidboot.slot_suffix="
#define ANDROID_ARG_ROOT "root="

#define ANDROID_NORMAL_BOOT "androidboot.force_normal_boot=1"
#ifdef CONFIG_SSTAR_ANDROID_BOOTLOADER
#define ANDROID_ARG_SERIALNO "androidboot.serialno="
#endif

static int android_bootloader_message_load(
	struct blk_desc *dev_desc,
	const struct disk_partition *part_info,
	struct bootloader_message *message)
{
	ulong message_blocks = sizeof(struct bootloader_message) /
	    part_info->blksz;
	if (message_blocks > part_info->size) {
		printf("misc partition too small.\n");
		return -1;
	}

	if (blk_dread(dev_desc, part_info->start, message_blocks, message) !=
	    message_blocks) {
		printf("Could not read from misc partition\n");
		return -1;
	}
	debug("ANDROID: Loaded BCB, %lu blocks.\n", message_blocks);
	return 0;
}

static int android_bootloader_message_write(
	struct blk_desc *dev_desc,
	const struct disk_partition *part_info,
	struct bootloader_message *message)
{
	ulong message_blocks = sizeof(struct bootloader_message) /
	    part_info->blksz;
	if (message_blocks > part_info->size) {
		printf("misc partition too small.\n");
		return -1;
	}

	if (blk_dwrite(dev_desc, part_info->start, message_blocks, message) !=
	    message_blocks) {
		printf("Could not write to misc partition\n");
		return -1;
	}
	debug("ANDROID: Wrote new BCB, %lu blocks.\n", message_blocks);
	return 0;
}

static enum android_boot_mode android_bootloader_load_and_clear_mode(
	struct blk_desc *dev_desc,
	const struct disk_partition *misc_part_info)
{
	struct bootloader_message bcb;

#ifdef CONFIG_FASTBOOT
	char *bootloader_str;

	/* Check for message from bootloader stored in RAM from a previous boot.
	 */
	bootloader_str = (char *)CONFIG_FASTBOOT_BUF_ADDR;
	if (!strcmp("reboot-bootloader", bootloader_str)) {
		bootloader_str[0] = '\0';
		return ANDROID_BOOT_MODE_BOOTLOADER;
	}
#endif

	/* Check and update the BCB message if needed. */
	if (android_bootloader_message_load(dev_desc, misc_part_info, &bcb) <
	    0) {
		printf("WARNING: Unable to load the BCB.\n");
		return ANDROID_BOOT_MODE_NORMAL;
	}

	if (!strcmp("bootonce-bootloader", bcb.command)) {
		/* Erase the message in the BCB since this value should be used
		 * only once.
		 */
		memset(bcb.command, 0, sizeof(bcb.command));
		android_bootloader_message_write(dev_desc, misc_part_info,
						 &bcb);
		return ANDROID_BOOT_MODE_BOOTLOADER;
	}

	if (!strcmp("boot-recovery", bcb.command))
		return ANDROID_BOOT_MODE_RECOVERY;

	return ANDROID_BOOT_MODE_NORMAL;
}

/**
 * Return the reboot reason string for the passed boot mode.
 *
 * @param mode	The Android Boot mode.
 * @return a pointer to the reboot reason string for mode.
 */
static const char *android_boot_mode_str(enum android_boot_mode mode)
{
	switch (mode) {
	case ANDROID_BOOT_MODE_NORMAL:
		return "(none)";
	case ANDROID_BOOT_MODE_RECOVERY:
		return "recovery";
	case ANDROID_BOOT_MODE_BOOTLOADER:
		return "bootloader";
	}
	return NULL;
}

static int android_part_get_info_by_name_suffix(struct blk_desc *dev_desc,
						const char *base_name,
						const char *slot_suffix,
						struct disk_partition *part_info)
{
	char *part_name;
	int part_num;
	size_t part_name_len;

	part_name_len = strlen(base_name) + 1;
	if (slot_suffix)
		part_name_len += strlen(slot_suffix);
	part_name = malloc(part_name_len);
	if (!part_name)
		return -1;
	strcpy(part_name, base_name);
	if (slot_suffix)
		strcat(part_name, slot_suffix);

	part_num = part_get_info_by_name(dev_desc, part_name, part_info);
	if (part_num < 0) {
		debug("ANDROID: Could not find partition \"%s\"\n", part_name);
		part_num = -1;
	}

	free(part_name);
	return part_num;
}

static int android_bootloader_boot_bootloader(void)
{
	const char *fastboot_cmd = env_get("fastbootcmd");

	if (fastboot_cmd)
		return run_command(fastboot_cmd, CMD_FLAG_ENV);
	return -1;
}

static char hex_to_char(uint8_t nibble) {
	if (nibble < 10) {
		return '0' + nibble;
	} else {
		return 'a' + nibble - 10;
	}
}
// Helper function to convert 32bit int to a hex string
static void hex_to_str(char* str, ulong input) {
	str[0] = '0'; str[1] = 'x';
	size_t str_idx = 2;
	uint32_t byte_extracted;
	uint8_t nibble;
	// Assume that this is on a little endian system.
	for(int byte_idx = 3; byte_idx >= 0; byte_idx--) {
		byte_extracted = ((0xFF << (byte_idx*8)) & input) >> (byte_idx*8);
		nibble = byte_extracted & 0xF0;
		nibble = nibble >> 4;
		nibble = nibble & 0xF;
		str[str_idx] = hex_to_char(nibble);
		str_idx++;
		nibble = byte_extracted & 0xF;
		str[str_idx] = hex_to_char(nibble);
		str_idx++;
	}
	str[str_idx] = 0;
}
__weak int android_bootloader_boot_kernel(const struct andr_boot_info* boot_info)
{
	ulong kernel_addr, kernel_size, ramdisk_addr, ramdisk_size;
	char *ramdisk_size_str, *fdt_addr = env_get("fdtaddr");
	char kernel_addr_str[12], ramdisk_addr_size_str[22];
	char *boot_args[] = {
		NULL, kernel_addr_str, ramdisk_addr_size_str, fdt_addr, NULL };

	if (android_image_get_kernel(boot_info, images.verify, NULL, &kernel_size))
		return -1;
	if (android_image_get_ramdisk(boot_info, &ramdisk_addr, &ramdisk_size))
		return -1;

	kernel_addr = android_image_get_kload(boot_info);
	hex_to_str(kernel_addr_str, kernel_addr);
	hex_to_str(ramdisk_addr_size_str, ramdisk_addr);
	ramdisk_size_str = &ramdisk_addr_size_str[strlen(ramdisk_addr_size_str)];
	*ramdisk_size_str = ':';
	hex_to_str(ramdisk_size_str + 1, ramdisk_size);

	printf("Booting kernel at %s with fdt at %s ramdisk %s...\n\n\n",
	       kernel_addr_str, fdt_addr, ramdisk_addr_size_str);
#if defined(CONFIG_ARM) && !defined(CONFIG_ARM64)
	do_bootz(NULL, 0, 4, boot_args);
#else
	do_booti(NULL, 0, 4, boot_args);
#endif

	return -1;
}

static char *strjoin(const char **chunks, char separator)
{
	int len, joined_len = 0;
	char *ret, *current;
	const char **p;

	for (p = chunks; *p; p++)
		joined_len += strlen(*p) + 1;

	if (!joined_len) {
		ret = malloc(1);
		if (ret)
			ret[0] = '\0';
		return ret;
	}

	ret = malloc(joined_len);
	current = ret;
	if (!ret)
		return ret;

	for (p = chunks; *p; p++) {
		len = strlen(*p);
		memcpy(current, *p, len);
		current += len;
		*current = separator;
		current++;
	}
	/* Replace the last separator by a \0. */
	current[-1] = '\0';
	return ret;
}

/** android_assemble_cmdline - Assemble the command line to pass to the kernel
 * @return a newly allocated string
 */
static char *android_assemble_cmdline(const char *slot_suffix,
				      const char *extra_args,
				      const bool normal_boot,
				      const char *android_kernel_cmdline,
				      const bool bootconfig_used,
				      const char *avb_cmdline)
{
	const char *cmdline_chunks[16];
	const char **current_chunk = cmdline_chunks;
	char *env_cmdline, *cmdline, *rootdev_input;
	char *allocated_suffix = NULL;
	char *allocated_rootdev = NULL;
	unsigned long rootdev_len;
#ifdef CONFIG_SSTAR_ANDROID_BOOTLOADER
	char *serialno = NULL;
	char *allocated_serialno = NULL;
#endif

	if (android_kernel_cmdline)
		*(current_chunk++) = android_kernel_cmdline;

	env_cmdline = env_get("bootargs");
	if (env_cmdline)
		*(current_chunk++) = env_cmdline;

	/* The |slot_suffix| needs to be passed to Android init to know what
	 * slot to boot from. This is done through bootconfig when supported.
	 */
	if (slot_suffix && !bootconfig_used) {
		allocated_suffix = malloc(strlen(ANDROID_ARG_SLOT_SUFFIX) +
					  strlen(slot_suffix));
		strcpy(allocated_suffix, ANDROID_ARG_SLOT_SUFFIX);
		strcat(allocated_suffix, slot_suffix);
		*(current_chunk++) = allocated_suffix;
	}

	rootdev_input = env_get("android_rootdev");
	if (rootdev_input) {
		rootdev_len = strlen(ANDROID_ARG_ROOT) + CONFIG_SYS_CBSIZE + 1;
		allocated_rootdev = malloc(rootdev_len);
		strcpy(allocated_rootdev, ANDROID_ARG_ROOT);
		cli_simple_process_macros(rootdev_input,
					  allocated_rootdev +
					  strlen(ANDROID_ARG_ROOT),
					  rootdev_len);
		/* Make sure that the string is null-terminated since the
		 * previous could not copy to the end of the input string if it
		 * is too big.
		 */
		allocated_rootdev[rootdev_len - 1] = '\0';
		*(current_chunk++) = allocated_rootdev;
	}

	if (extra_args) {
		*(current_chunk++) = extra_args;
	}

	if (avb_cmdline && !bootconfig_used) {
		*(current_chunk++) = avb_cmdline;
	}

#ifdef CONFIG_SSTAR_ANDROID_BOOTLOADER
	serialno = env_get("serial#");
	if (serialno) {
		allocated_serialno = malloc(strlen(ANDROID_ARG_SERIALNO) + strlen(serialno) + 1);
		strcpy(allocated_serialno, ANDROID_ARG_SERIALNO);
		strcat(allocated_serialno, serialno);
		*(current_chunk++) = allocated_serialno;
	}
#endif
#ifdef CONFIG_ANDROID_USES_RECOVERY_AS_BOOT
	/* The force_normal_boot param must be passed to android's init sequence
	 * to avoid booting into recovery mode when using recovery as boot.
	 * This is done through bootconfig when supported.
	 * Refer to link below under "Early Init Boot Sequence"
	 * https://source.android.com/devices/architecture/kernel/mounting-partitions-early
	 */
	if (normal_boot && !bootconfig_used) {
		*(current_chunk++) = ANDROID_NORMAL_BOOT;
	}
#endif

	*(current_chunk++) = NULL;
	cmdline = strjoin(cmdline_chunks, ' ');
	free(allocated_suffix);
	free(allocated_rootdev);
#ifdef CONFIG_SSTAR_ANDROID_BOOTLOADER
	free(allocated_serialno);
#endif
	return cmdline;
}

/**
 * Calls avb_verify() with ops allocated for iface and devnum.
 *
 * Returns AvbSlotVerifyData and kernel command line parameters as out arguments and either
 * CMD_RET_SUCCESS or CMD_RET_FAILURE as the return value.
 */
static int do_avb_verify(const char *iface,
		         const char *devstr,
		         const char *slot_suffix,
		         AvbSlotVerifyData **out_data,
		         char **out_cmdline)
{
	int ret = CMD_RET_FAILURE;
	struct AvbOps *ops;
	char *devnum = strdup(devstr);
	char *hash_char = NULL;

	if (devnum == NULL) {
		 printf("OOM when copying devstr\n");
		 return CMD_RET_FAILURE;
	}

	hash_char = strchr(devnum, '#');
	if (hash_char != NULL) {
		*hash_char = '\0';
	}

	ops = avb_ops_alloc(iface, devnum);
	if (ops == NULL) {
		 printf("Failed to initialize avb2\n");
		 goto out;
	}

	ret = avb_verify(ops, slot_suffix, out_data, out_cmdline);

	if (ops != NULL) {
		avb_ops_free(ops);
	}

out:
	free(devnum);
	return ret;
}

int android_bootloader_boot_flow(const char* iface_str,
				 const char* dev_str,
				 struct blk_desc *dev_desc,
				 const struct disk_partition *misc_part_info,
				 const char *slot,
				 bool verify,
				 unsigned long kernel_address,
				 struct blk_desc *persistant_dev_desc)
{
	enum android_boot_mode mode;
	struct disk_partition boot_part_info;
	struct disk_partition vendor_boot_part_info;
	int boot_part_num, vendor_boot_part_num;
	char *command_line;
	char slot_suffix[3];
	const char *mode_cmdline = NULL;
	char *avb_cmdline = NULL;
	char *avb_bootconfig = NULL;
	const char *boot_partition = ANDROID_PARTITION_BOOT;
	const char *vendor_boot_partition = ANDROID_PARTITION_VENDOR_BOOT;
#ifdef CONFIG_ANDROID_SYSTEM_AS_ROOT
	int system_part_num
	struct disk_partition system_part_info;
#endif

	/* Determine the boot mode and clear its value for the next boot if
	 * needed.
	 */
	mode = android_bootloader_load_and_clear_mode(dev_desc, misc_part_info);
	printf("ANDROID: reboot reason: \"%s\"\n", android_boot_mode_str(mode));

#ifndef CONFIG_SSTAR_ANDROID_BOOTLOADER
	// TODO (rammuthiah) fastboot isn't suported on cuttlefish yet.
	// Once it is, these lines can be removed.
	if (mode == ANDROID_BOOT_MODE_BOOTLOADER) {
		mode = ANDROID_BOOT_MODE_NORMAL;
	}
#endif

	bool normal_boot = (mode == ANDROID_BOOT_MODE_NORMAL);
	switch (mode) {
	case ANDROID_BOOT_MODE_NORMAL:
#ifdef CONFIG_ANDROID_SYSTEM_AS_ROOT
		/* In normal mode, we load the kernel from "boot" but append
		 * "skip_initramfs" to the cmdline to make it ignore the
		 * recovery initramfs in the boot partition.
		 */
		mode_cmdline = "skip_initramfs";
#endif
		break;
	case ANDROID_BOOT_MODE_RECOVERY:
#if defined(CONFIG_ANDROID_SYSTEM_AS_ROOT) || defined(CONFIG_ANDROID_USES_RECOVERY_AS_BOOT)
		/* In recovery mode we still boot the kernel from "boot" but
		 * don't skip the initramfs so it boots to recovery.
		 * If on Android device using Recovery As Boot, there is no
		 * recovery  partition.
		 */
#else
		boot_partition = ANDROID_PARTITION_RECOVERY;
#endif
		break;
	case ANDROID_BOOT_MODE_BOOTLOADER:
		/* Bootloader mode enters fastboot. If this operation fails we
		 * simply return since we can't recover from this situation by
		 * switching to another slot.
		 */
		return android_bootloader_boot_bootloader();
	}

	/* Look for an optional slot suffix override. */
	if (!slot || !slot[0])
		slot = env_get("android_slot_suffix");

	slot_suffix[0] = '\0';
	if (slot && slot[0]) {
		slot_suffix[0] = '_';
		slot_suffix[1] = slot[0];
		slot_suffix[2] = '\0';
	} else {
#ifdef CONFIG_ANDROID_AB
		int slot_num = ab_select_slot(dev_desc, misc_part_info, normal_boot);
		if (slot_num < 0) {
			log_err("Could not determine Android boot slot.\n");
#ifdef CONFIG_SSTAR_ANDROID_BOOTLOADER
			android_bootloader_boot_bootloader();
#endif
			return -1;
		}
		slot_suffix[0] = '_';
		slot_suffix[1] = BOOT_SLOT_NAME(slot_num);
		slot_suffix[2] = '\0';
#endif
	}

	/* Run AVB if requested. During the verification, the bits from the
	 * partitions are loaded by libAVB and are stored in avb_out_data.
	 * We need to use the verified data and shouldn't read data from the
	 * disk again.*/
	AvbSlotVerifyData *avb_out_data = NULL;
	AvbPartitionData *verified_boot_img = NULL;
	AvbPartitionData *verified_vendor_boot_img = NULL;
	if (verify) {
		if (do_avb_verify(iface_str, dev_str, slot_suffix, &avb_out_data, &avb_cmdline) ==
			CMD_RET_FAILURE) {
			goto bail;
		}
		for (int i = 0; i < avb_out_data->num_loaded_partitions; i++) {
			AvbPartitionData *p =
			    &avb_out_data->loaded_partitions[i];
			if (strcmp("boot", p->partition_name) == 0) {
				verified_boot_img = p;
			}
			if (strcmp("vendor_boot", p->partition_name) == 0) {
				verified_vendor_boot_img = p;
			}
		}
		if (verified_boot_img == NULL || verified_vendor_boot_img == NULL) {
			debug("verified partition not found");
			goto bail;
		}
	}

	/* Load the kernel from the desired "boot" partition. */
	boot_part_num =
	    android_part_get_info_by_name_suffix(dev_desc, boot_partition,
						 slot_suffix, &boot_part_info);
	/* Load the vendor boot partition if there is one. */
	vendor_boot_part_num =
	    android_part_get_info_by_name_suffix(dev_desc, vendor_boot_partition,
						 slot_suffix,
						 &vendor_boot_part_info);
	struct disk_partition *bootconfig_part_info_ptr = NULL;
#ifdef CONFIG_ANDROID_PERSISTENT_RAW_DISK_DEVICE
	struct disk_partition bootconfig_part_info;
	const char *bootconfig_partition = ANDROID_PARTITION_BOOTCONFIG;
	int bootconfig_part_num = android_part_get_info_by_name_suffix(persistant_dev_desc,
						 bootconfig_partition,
						 NULL,
						 &bootconfig_part_info);
	if (bootconfig_part_num < 0) {
		log_err("Failed to find device specific bootconfig.\n");
	} else {
		bootconfig_part_info_ptr = &bootconfig_part_info;
	}
#endif /* CONFIG_ANDROID_PERSISTENT_RAW_DISK_DEVICE */
	if (boot_part_num < 0)
		goto bail;
	debug("ANDROID: Loading kernel from \"%s\", partition %d.\n",
	      boot_part_info.name, boot_part_num);

#ifdef CONFIG_ANDROID_SYSTEM_AS_ROOT
	system_part_num =
	    android_part_get_info_by_name_suffix(dev_desc,
						 ANDROID_PARTITION_SYSTEM,
						 slot_suffix,
						 &system_part_info);
	if (system_part_num < 0)
		goto bail;
	debug("ANDROID: Using system image from \"%s\", partition %d.\n",
	      system_part_info.name, system_part_num);
#endif

	struct disk_partition *vendor_boot_part_info_ptr = &vendor_boot_part_info;
	if (vendor_boot_part_num < 0) {
		vendor_boot_part_info_ptr = NULL;
	} else {
		printf("ANDROID: Loading vendor ramdisk from \"%s\", partition"
		       " %d.\n", vendor_boot_part_info.name,
		       vendor_boot_part_num);
	}

	// convert avb_cmdline into avb_bootconfig by replacing ' ' with '\n'.
	if (avb_cmdline != NULL) {
		size_t len = strlen(avb_cmdline);
		// Why +2? One byte is for the last '\n', one another byte is
		// for the null terminator
		size_t newlen = len + 2;
		avb_bootconfig = (char *)malloc(newlen);
		strncpy(avb_bootconfig, avb_cmdline, len);
		for (char *p = avb_bootconfig; *p; p++) {
			if (*p == ' ') *p = '\n';
		}
		avb_bootconfig[len] = '\n';
		avb_bootconfig[len + 1] = 0;
	}

	struct andr_boot_info* boot_info = android_image_load(dev_desc, &boot_part_info,
				 vendor_boot_part_info_ptr,
				 kernel_address, slot_suffix, normal_boot, avb_bootconfig,
				 persistant_dev_desc, bootconfig_part_info_ptr,
				 verified_boot_img, verified_vendor_boot_img);

	if (!boot_info)
		goto bail;


#ifdef CONFIG_ANDROID_SYSTEM_AS_ROOT
	/* Set Android root variables. */
	env_set_ulong("android_root_devnum", dev_desc->devnum);
	env_set_ulong("android_root_partnum", system_part_num);
#endif
	env_set("android_slotsufix", slot_suffix);

	/* Assemble the command line */
	command_line = android_assemble_cmdline(slot_suffix, mode_cmdline, normal_boot,
							android_image_get_kernel_cmdline(boot_info),
							android_image_is_bootconfig_used(boot_info),
							avb_cmdline);
	env_set("bootargs", command_line);

#if defined(CONFIG_SSTAR_ANDROID_AB) && defined(CONFIG_SSTAR_RPMB)
	/* Update locate rollback index before jump to kernel */
	extern int sstar_rpmb_get_rollback_index(uint8_t location, uint64_t *out_rollback_index);
	extern int sstar_rpmb_set_rollback_index(uint8_t location, uint64_t rollback_index);
	if (verify) {
		if (true == ab_has_current_slot_boot_successful(dev_desc, misc_part_info)) {
			for (int i = 0; i < avb_out_data->num_vbmeta_images; i++ ) {
				uint64_t local_rollback_index = -1UL;
				sstar_rpmb_get_rollback_index(i, &local_rollback_index);
				if (local_rollback_index < avb_out_data->rollback_indexes[i]) {
					printf("ANDROID: Update %s rollback index from %lld to %lld ...\n", avb_out_data->vbmeta_images[i].partition_name,
							local_rollback_index,
							avb_out_data->rollback_indexes[i]);
					if (sstar_rpmb_set_rollback_index(i, avb_out_data->rollback_indexes[i])) {
						printf("ANDROID: Update rollback index fail.\n");
						goto bail;
					}
				}
			}
		}
	}
#endif

	debug("ANDROID: bootargs: \"%s\"\n", command_line);
	android_bootloader_boot_kernel(boot_info);

	/* TODO: If the kernel doesn't boot mark the selected slot as bad. */
	goto bail;

bail:
	if (avb_out_data != NULL) {
		avb_slot_verify_data_free(avb_out_data);
	}
	if (avb_cmdline != NULL) {
		free(avb_cmdline);
	}
	if (avb_bootconfig != NULL) {
		free(avb_bootconfig);
	}
#if defined(CONFIG_SSTAR_ANDROID_BOOTLOADER) && defined(CONFIG_SSTAR_ANDROID_AB)
	/* mark the selected slot as bad and reboot */
	printf("ANDROID: slot %s boot failed, mark unbootable and reboot...\n", slot_suffix);
	ab_mark_current_slot_unbootable(dev_desc, misc_part_info);
	do_reset(NULL, 0, 0, NULL);
#endif
	return -1;
}
