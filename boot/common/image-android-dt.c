// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2018 Linaro Ltd.
 * Sam Protsenko <semen.protsenko@linaro.org>
 */

#include <image-android-dt.h>
#include <dt_table.h>
#include <common.h>
#include <linux/libfdt.h>
#include <mapmem.h>
#ifdef CONFIG_SSTAR_DTB_OVERLAY_SUPPORT
#include <stdlib.h>
#include <string.h>
#include <fdt_support.h>
#include <memalign.h>
#include <linux/kernel.h>
#ifdef CONFIG_ANDROID_AB
#include <android_ab.h>
#endif
#endif

/**
 * Check if image header is correct.
 *
 * @param hdr_addr Start address of DT image
 * @return true if header is correct or false if header is incorrect
 */
bool android_dt_check_header(ulong hdr_addr)
{
	const struct dt_table_header *hdr;
	u32 magic;

	hdr = map_sysmem(hdr_addr, sizeof(*hdr));
	magic = fdt32_to_cpu(hdr->magic);
	unmap_sysmem(hdr);

	return magic == DT_TABLE_MAGIC;
}

/**
 * Get the address of FDT (dtb or dtbo) in memory by its index in image.
 *
 * @param hdr_addr Start address of DT image
 * @param index Index of desired FDT in image (starting from 0)
 * @param[out] addr If not NULL, will contain address to specified FDT
 * @param[out] size If not NULL, will contain size of specified FDT
 *
 * @return true on success or false on error
 */
bool android_dt_get_fdt_by_index(ulong hdr_addr, u32 index, ulong *addr,
				 u32 *size)
{
	const struct dt_table_header *hdr;
	const struct dt_table_entry *e;
	u32 entry_count, entries_offset, entry_size;
	ulong e_addr;
	u32 dt_offset, dt_size;

	hdr = map_sysmem(hdr_addr, sizeof(*hdr));
	entry_count = fdt32_to_cpu(hdr->dt_entry_count);
	entries_offset = fdt32_to_cpu(hdr->dt_entries_offset);
	entry_size = fdt32_to_cpu(hdr->dt_entry_size);
	unmap_sysmem(hdr);

	if (index >= entry_count) {
		printf("Error: index >= dt_entry_count (%u >= %u)\n", index,
		       entry_count);
		return false;
	}

	e_addr = hdr_addr + entries_offset + index * entry_size;
	e = map_sysmem(e_addr, sizeof(*e));
	dt_offset = fdt32_to_cpu(e->dt_offset);
	dt_size = fdt32_to_cpu(e->dt_size);
	unmap_sysmem(e);

	if (addr)
		*addr = hdr_addr + dt_offset;
	if (size)
		*size = dt_size;

	return true;
}

#if !defined(CONFIG_SPL_BUILD)
static void android_dt_print_fdt_info(const struct fdt_header *fdt)
{
	u32 fdt_size;
	int root_node_off;
	const char *compatible;

	root_node_off = fdt_path_offset(fdt, "/");
	if (root_node_off < 0) {
		printf("Error: Root node not found\n");
		return;
	}

	fdt_size = fdt_totalsize(fdt);
	compatible = fdt_getprop(fdt, root_node_off, "compatible",
				 NULL);

	printf("           (FDT)size = %d\n", fdt_size);
	printf("     (FDT)compatible = %s\n",
	       compatible ? compatible : "(unknown)");
}

/**
 * Print information about DT image structure.
 *
 * @param hdr_addr Start address of DT image
 */
void android_dt_print_contents(ulong hdr_addr)
{
	const struct dt_table_header *hdr;
	u32 entry_count, entries_offset, entry_size;
	u32 i;

	hdr = map_sysmem(hdr_addr, sizeof(*hdr));
	entry_count = fdt32_to_cpu(hdr->dt_entry_count);
	entries_offset = fdt32_to_cpu(hdr->dt_entries_offset);
	entry_size = fdt32_to_cpu(hdr->dt_entry_size);

	/* Print image header info */
	printf("dt_table_header:\n");
	printf("               magic = %08x\n", fdt32_to_cpu(hdr->magic));
	printf("          total_size = %d\n", fdt32_to_cpu(hdr->total_size));
	printf("         header_size = %d\n", fdt32_to_cpu(hdr->header_size));
	printf("       dt_entry_size = %d\n", entry_size);
	printf("      dt_entry_count = %d\n", entry_count);
	printf("   dt_entries_offset = %d\n", entries_offset);
	printf("           page_size = %d\n", fdt32_to_cpu(hdr->page_size));
	printf("             version = %d\n", fdt32_to_cpu(hdr->version));

	unmap_sysmem(hdr);

	/* Print image entries info */
	for (i = 0; i < entry_count; ++i) {
		const ulong e_addr = hdr_addr + entries_offset + i * entry_size;
		const struct dt_table_entry *e;
		const struct fdt_header *fdt;
		u32 dt_offset, dt_size;
		u32 j;

		e = map_sysmem(e_addr, sizeof(*e));
		dt_offset = fdt32_to_cpu(e->dt_offset);
		dt_size = fdt32_to_cpu(e->dt_size);

		printf("dt_table_entry[%d]:\n", i);
		printf("             dt_size = %d\n", dt_size);
		printf("           dt_offset = %d\n", dt_offset);
		printf("                  id = %08x\n", fdt32_to_cpu(e->id));
		printf("                 rev = %08x\n", fdt32_to_cpu(e->rev));
		for (j = 0; j < 4; ++j) {
			printf("           custom[%d] = %08x\n", j,
			       fdt32_to_cpu(e->custom[j]));
		}

		unmap_sysmem(e);

		/* Print FDT info for this entry */
		fdt = map_sysmem(hdr_addr + dt_offset, sizeof(*fdt));
		android_dt_print_fdt_info(fdt);
		unmap_sysmem(fdt);
	}
}
#endif

#if defined(CONFIG_SSTAR_DTB_OVERLAY_SUPPORT)
/**
 * Get the address of FDT (dtb or dtbo) in memory by its compatible.
 *
 * @param hdr_addr Start address of DT image
 * @param compatible prop of the root node
 * @param[out] addr If not NULL, will contain address to specified FDT
 * @param[out] size If not NULL, will contain size of specified FDT
 *
 * @return true on success or false on error, 'addr' must be freed after use.
 *
 */
bool android_dt_get_fdt_by_compatible(ulong hdr_addr, char *compatible, ulong *addr, u32 *size)
{
	void *fdt = NULL;
	u32 fdt_size = 0;
	bool match = false;

#ifdef DEBUG
	printf("Debug: find dtbo with compatible: '%s' at %#lx\n",  compatible, hdr_addr);
#endif
	if (!addr || !size) {
		printf("Error: Invalid argument addr or size\n");
		return false;
	}

	for (u32 index = 0; android_dt_get_fdt_by_index(hdr_addr, index, (ulong *)&fdt, &fdt_size); index++) {
		int root_node_off;
		const char *compatible_;

		root_node_off = fdt_path_offset(fdt, "/");
		if (root_node_off < 0) {
			printf("Warning: Root node not found at index %d\n", index);
			continue;
		}

		compatible_ = fdt_getprop(fdt, root_node_off, "compatible", NULL);
		if (!compatible_) {
			printf("Warning: Compatible not found at index %d\n", index);
			continue;
		}

		/* Confirm that 'compatible' and 'compatible_' are match exactly */
		if (strncmp(compatible, compatible_, max(strlen(compatible), strlen(compatible_))) != 0) {
			continue;
		}

#ifdef DEBUG
		printf("Debug: Compatible(%s) match at index %d, fdt=%#lx size=%d\n", compatible_, index, (ulong)fdt, fdt_size);
#endif
		match = true;
		break;
	}

	if (!match)
		return false;

	/* Alloc and copy to aonther fdt buffer to avoid error at next call. */
	*addr = (ulong)malloc(fdt_size);
	if (!*addr) {
		printf("Error: Malloc %d bytes fail\n", fdt_size);
		return false;
	}

	memcpy((void *)*addr, fdt, fdt_size);
	*size = fdt_size;

	return true;
}

/**
 * Get the dtbo image.
 *
 * @param dev_desc block descriptor of boot device
 * @param part_info partition info for which dtboimage store
 * @param[out] dtbo_addr Start address of dtbo image
 *
 * @return true on success or false on error.
 *
 */
static int android_get_dtbo(struct blk_desc *dev_desc, struct disk_partition *part_info, ulong *dtbo_addr)
{
	struct dt_table_header *dt_table_hdr = NULL;
	u32 blk_cnt = 0;
	void *dtbo_buf = NULL;
	int ret = -1;

	if (!dtbo_addr) {
		printf("Error: Invalid argument dtbo_addr\n");
		return -EINVAL;
	}

	/* Read 1 block from dtbo part and check dt table header */
	dt_table_hdr = malloc_cache_aligned(part_info->blksz);
	if (!dt_table_hdr) {
		printf("Error: Malloc %ld bytes fail\n", part_info->blksz);
		return -ENOMEM;
	}

	ret = blk_dread(dev_desc, part_info->start, 1, dt_table_hdr);
	if (ret != 1) {
		printf("Error: Part %s Read %ld bytes fail\n", part_info->name, part_info->blksz);
		ret = -EIO;
		goto out;
	}

	if (!android_dt_check_header((ulong)dt_table_hdr)) {
		printf("Error: Invalid dt table header: 0x%x\n", fdt32_to_cpu(dt_table_hdr->magic));
		ret = -EINVAL;
		goto out;
	}

#ifdef DEBUG
	android_dt_print_contents((ulong)dt_table_hdr);
#endif

	/* Read all dt image */
	blk_cnt = DIV_ROUND_UP(fdt32_to_cpu(dt_table_hdr->total_size), part_info->blksz);
	dtbo_buf = malloc_cache_aligned(part_info->blksz * blk_cnt);
	if (!dtbo_buf) {
		printf("Error: Malloc %ld bytes fail\n", part_info->blksz * blk_cnt);
		ret = -ENOMEM;
		goto out;
	}

	ret = blk_dread(dev_desc, part_info->start, blk_cnt, dtbo_buf);
	if (ret != blk_cnt) {
		printf("Error: Part %s Read %ld bytes fail\n", part_info->name, part_info->blksz * blk_cnt);
		free(dtbo_buf);
		ret = -EIO;
		goto out;
	}

	*dtbo_addr = (ulong)dtbo_buf;
	ret = 0;

out:
	free(dt_table_hdr);
	return ret;
}

/**
 * Apply dtb by dtbo compatible list
 *
 * @param dev_desc block descriptor of boot device
 * @param dtb_addr start addree of dtb which to be applied.
 * @param dtbo_list the string include all dtbo compatible
 * looks like "compatible_0;compatible_1;...".
 *
 * @return =0 if apply success and <0 if fail.
 *
 */
int android_dt_overlay_apply_verbose(void *dtb_addr, char *dtbo_list, struct blk_desc *dev_desc)
{
	void *dtboimg_addr = NULL;
	char dtbo_part_name[32] = {'\0'};
	struct disk_partition misc_part_info;
	struct disk_partition dtbo_part_info;
	char *compatible = NULL;
	const char *delim = ";";
	int ret = -1;

	if (!dtb_addr || !dtbo_list || !dev_desc ) {
		printf("Error: Invalid argument dtb_addr/dtbo_list/dev_desc\n");
		return -EINVAL;
	}

	strcat(dtbo_part_name, CONFIG_SSTAR_DTBO_PART_NAME);

	// If Android AB device, cat current slot surffix to dtbo part name
#ifdef CONFIG_ANDROID_AB
	ret = part_get_info_by_name(dev_desc, CONFIG_SSTAR_MISC_PART_NAME, &misc_part_info);
	if (ret < 0) {
		printf("Error: Get part info '%s' fail\n", CONFIG_SSTAR_MISC_PART_NAME);
		return -EIO;
	}
	strcat(dtbo_part_name, ab_get_current_slot_suffix(dev_desc, &misc_part_info));
#endif

	// Get device description and dtbo part info
	ret = part_get_info_by_name(dev_desc, dtbo_part_name, &dtbo_part_info);
	if (ret < 0) {
		printf("Error: Get part info '%s' fail\n", dtbo_part_name);
		return -EIO;
	}

#ifdef DEBUG
	printf("Debug: Loading dtbo image from %s part\n", dtbo_part_name);
#endif

	// Read android dt image to 'dtboimg_addr'
	ret = android_get_dtbo(dev_desc, &dtbo_part_info, (ulong *)&dtboimg_addr);
	if (ret != 0) {
		printf("Error: Get dtbo fail\n");
		return ret;
	}

	// Resize overlay target fdt to big enough
	ret = fdt_shrink_to_minimum(dtb_addr, 0x20000);
	if (ret < 0) {
		printf("Error: Resize dtb fail\n");
		goto out;
	}

	// If delim is ';', the dtbo_list string in env is:
	// <compatible-0>;<compatible-1>;<compatible-2>;...
	compatible = strtok(dtbo_list, delim);
	while (compatible != NULL) {
		void *dtbo_addr = NULL;
		u32 dtbo_size = 0;
		bool match = false;

		// Fetch fdt with compatible from android dt image
		match = android_dt_get_fdt_by_compatible((ulong)dtboimg_addr, compatible, (ulong *)&dtbo_addr, &dtbo_size);
		if (!match) {
			printf("Error: Get fdt by compatible(%s) fail\n", compatible);
			ret = -EINVAL;
			goto out;
		}

		// Do fdt overlay
		ret = fdt_overlay_apply_verbose(dtb_addr, dtbo_addr);
		if (ret < 0) {
			printf("Error: Apply dtbo with compatible(%s) fail\n", compatible);
			free(dtbo_addr);
			goto out;
		}

#ifdef DEBUG
		printf("Debug: Apply dtbo with compatible(%s) success\n", compatible);
#endif
		// Next compatible
		compatible = strtok(NULL, delim);
		free(dtbo_addr);
	}

	// Resize fdt to fixup size
	fdt_shrink_to_minimum(dtb_addr, 0);
	ret = 0;

out:
	if (NULL != dtboimg_addr)
		free(dtboimg_addr);
	return ret;
}
#endif
