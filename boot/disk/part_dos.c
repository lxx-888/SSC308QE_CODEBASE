// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2001
 * Raymond Lo, lo@routefree.com
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

/*
 * Support for harddisk partitions.
 *
 * To be compatible with LinuxPPC and Apple we use the standard Apple
 * SCSI disk partitioning scheme. For more information see:
 * http://developer.apple.com/techpubs/mac/Devices/Devices-126.html#MARKER-14-92
 */

#include <common.h>
#include <blk.h>
#include <command.h>
#include <ide.h>
#include <memalign.h>
#include <asm/unaligned.h>
#include <linux/compiler.h>
#include "part_dos.h"
#include <part.h>

#ifdef CONFIG_HAVE_BLOCK_DEVICE

#define DOS_PART_DEFAULT_SECTOR 512

/* should this be configurable? It looks like it's not very common at all
 * to use large numbers of partitions */
#define MAX_EXT_PARTS 256

#if defined(CONFIG_ARCH_SSTAR) && CONFIG_IS_ENABLED(CMD_SSTAR_MMC_FDISK)
static int gu32_PartNum = 0;
#endif

static inline int is_extended(int part_type)
{
    return (part_type == DOS_PART_TYPE_EXTENDED ||
	    part_type == DOS_PART_TYPE_EXTENDED_LBA ||
	    part_type == DOS_PART_TYPE_EXTENDED_LINUX);
}

static int get_bootable(dos_partition_t *p)
{
	int ret = 0;

	if (p->sys_ind == 0xef)
		ret |= PART_EFI_SYSTEM_PARTITION;
	if (p->boot_ind == 0x80)
		ret |= PART_BOOTABLE;
	return ret;
}

static void print_one_part(dos_partition_t *p, lbaint_t ext_part_sector,
			   int part_num, unsigned int disksig)
{
	lbaint_t lba_start = ext_part_sector + get_unaligned_le32(p->start4);
	lbaint_t lba_size  = get_unaligned_le32(p->size4);

#if defined(CONFIG_ARCH_SSTAR) && CONFIG_IS_ENABLED(CMD_SSTAR_MMC_FDISK)
	gu32_PartNum++;
#endif
	printf("%3d\t%-10" LBAFlength "u\t%-10" LBAFlength
		"u\t%08x-%02x\t%02x%s%s\n",
		part_num, lba_start, lba_size, disksig, part_num, p->sys_ind,
		(is_extended(p->sys_ind) ? " Extd" : ""),
		(get_bootable(p) ? " Boot" : ""));
}

static int test_block_type(unsigned char *buffer)
{
	int slot;
	struct dos_partition *p;
	int part_count = 0;

	if((buffer[DOS_PART_MAGIC_OFFSET + 0] != 0x55) ||
	    (buffer[DOS_PART_MAGIC_OFFSET + 1] != 0xaa) ) {
		return (-1);
	} /* no DOS Signature at all */
	p = (struct dos_partition *)&buffer[DOS_PART_TBL_OFFSET];

	/* Check that the boot indicators are valid and count the partitions. */
	for (slot = 0; slot < 4; ++slot, ++p) {
		if (p->boot_ind != 0 && p->boot_ind != 0x80)
			break;
		if (p->sys_ind)
			++part_count;
	}

	/*
	 * If the partition table is invalid or empty,
	 * check if this is a DOS PBR
	 */
	if (slot != 4 || !part_count) {
		if (!strncmp((char *)&buffer[DOS_PBR_FSTYPE_OFFSET],
			     "FAT", 3) ||
		    !strncmp((char *)&buffer[DOS_PBR32_FSTYPE_OFFSET],
			     "FAT32", 5))
			return DOS_PBR; /* This is a DOS PBR and not an MBR */
	}
	if (slot == 4)
		return DOS_MBR;	/* This is an DOS MBR */

	/* This is neither a DOS MBR nor a DOS PBR */
	return -1;
}

static int part_test_dos(struct blk_desc *dev_desc)
{
#ifndef CONFIG_SPL_BUILD
	ALLOC_CACHE_ALIGN_BUFFER(legacy_mbr, mbr,
			DIV_ROUND_UP(dev_desc->blksz, sizeof(legacy_mbr)));

	if (blk_dread(dev_desc, 0, 1, (ulong *)mbr) != 1)
		return -1;

	if (test_block_type((unsigned char *)mbr) != DOS_MBR)
		return -1;

	if (dev_desc->sig_type == SIG_TYPE_NONE &&
	    mbr->unique_mbr_signature != 0) {
		dev_desc->sig_type = SIG_TYPE_MBR;
		dev_desc->mbr_sig = mbr->unique_mbr_signature;
	}
#else
	ALLOC_CACHE_ALIGN_BUFFER(unsigned char, buffer, dev_desc->blksz);

	if (blk_dread(dev_desc, 0, 1, (ulong *)buffer) != 1)
		return -1;

	if (test_block_type(buffer) != DOS_MBR)
		return -1;
#endif

	return 0;
}

/*  Print a partition that is relative to its Extended partition table
 */
static void print_partition_extended(struct blk_desc *dev_desc,
				     lbaint_t ext_part_sector,
				     lbaint_t relative,
				     int part_num, unsigned int disksig)
{
	ALLOC_CACHE_ALIGN_BUFFER(unsigned char, buffer, dev_desc->blksz);
	dos_partition_t *pt;
	int i;

	/* set a maximum recursion level */
	if (part_num > MAX_EXT_PARTS)
	{
		printf("** Nested DOS partitions detected, stopping **\n");
		return;
    }

	if (blk_dread(dev_desc, ext_part_sector, 1, (ulong *)buffer) != 1) {
		printf ("** Can't read partition table on %d:" LBAFU " **\n",
			dev_desc->devnum, ext_part_sector);
		return;
	}
	i=test_block_type(buffer);
	if (i != DOS_MBR) {
		printf ("bad MBR sector signature 0x%02x%02x\n",
			buffer[DOS_PART_MAGIC_OFFSET],
			buffer[DOS_PART_MAGIC_OFFSET + 1]);
		return;
	}

	if (!ext_part_sector)
		disksig = get_unaligned_le32(&buffer[DOS_PART_DISKSIG_OFFSET]);

	/* Print all primary/logical partitions */
	pt = (dos_partition_t *) (buffer + DOS_PART_TBL_OFFSET);
	for (i = 0; i < 4; i++, pt++) {
		/*
		 * fdisk does not show the extended partitions that
		 * are not in the MBR
		 */

		if ((pt->sys_ind != 0) &&
		    (ext_part_sector == 0 || !is_extended (pt->sys_ind)) ) {
			print_one_part(pt, ext_part_sector, part_num, disksig);
		}

		/* Reverse engr the fdisk part# assignment rule! */
		if ((ext_part_sector == 0) ||
		    (pt->sys_ind != 0 && !is_extended (pt->sys_ind)) ) {
			part_num++;
		}
	}

	/* Follows the extended partitions */
	pt = (dos_partition_t *) (buffer + DOS_PART_TBL_OFFSET);
	for (i = 0; i < 4; i++, pt++) {
		if (is_extended (pt->sys_ind)) {
			lbaint_t lba_start
				= get_unaligned_le32 (pt->start4) + relative;

			print_partition_extended(dev_desc, lba_start,
				ext_part_sector == 0  ? lba_start : relative,
				part_num, disksig);
		}
	}

	return;
}


/*  Print a partition that is relative to its Extended partition table
 */
static int part_get_info_extended(struct blk_desc *dev_desc,
				  lbaint_t ext_part_sector, lbaint_t relative,
				  int part_num, int which_part,
				  struct disk_partition *info, uint disksig)
{
	ALLOC_CACHE_ALIGN_BUFFER(unsigned char, buffer, dev_desc->blksz);
	dos_partition_t *pt;
	int i;
	int dos_type;

	/* set a maximum recursion level */
	if (part_num > MAX_EXT_PARTS)
	{
		printf("** Nested DOS partitions detected, stopping **\n");
		return -1;
    }

	if (blk_dread(dev_desc, ext_part_sector, 1, (ulong *)buffer) != 1) {
		printf ("** Can't read partition table on %d:" LBAFU " **\n",
			dev_desc->devnum, ext_part_sector);
		return -1;
	}
	if (buffer[DOS_PART_MAGIC_OFFSET] != 0x55 ||
		buffer[DOS_PART_MAGIC_OFFSET + 1] != 0xaa) {
		printf ("bad MBR sector signature 0x%02x%02x\n",
			buffer[DOS_PART_MAGIC_OFFSET],
			buffer[DOS_PART_MAGIC_OFFSET + 1]);
		return -1;
	}

#if CONFIG_IS_ENABLED(PARTITION_UUIDS)
	if (!ext_part_sector)
		disksig = get_unaligned_le32(&buffer[DOS_PART_DISKSIG_OFFSET]);
#endif

	/* Print all primary/logical partitions */
	pt = (dos_partition_t *) (buffer + DOS_PART_TBL_OFFSET);
	for (i = 0; i < 4; i++, pt++) {
		/*
		 * fdisk does not show the extended partitions that
		 * are not in the MBR
		 */
		if (((pt->boot_ind & ~0x80) == 0) &&
		    (pt->sys_ind != 0) &&
		    (part_num == which_part) &&
		    (ext_part_sector == 0 || is_extended(pt->sys_ind) == 0)) {
			info->blksz = DOS_PART_DEFAULT_SECTOR;
			info->start = (lbaint_t)(ext_part_sector +
					get_unaligned_le32(pt->start4));
			info->size  = (lbaint_t)get_unaligned_le32(pt->size4);
			part_set_generic_name(dev_desc, part_num,
					      (char *)info->name);
			/* sprintf(info->type, "%d, pt->sys_ind); */
			strcpy((char *)info->type, "U-Boot");
			info->bootable = get_bootable(pt);
#if CONFIG_IS_ENABLED(PARTITION_UUIDS)
			sprintf(info->uuid, "%08x-%02x", disksig, part_num);
#endif
			info->sys_ind = pt->sys_ind;
			return 0;
		}

		/* Reverse engr the fdisk part# assignment rule! */
		if ((ext_part_sector == 0) ||
		    (pt->sys_ind != 0 && !is_extended (pt->sys_ind)) ) {
			part_num++;
		}
	}

	/* Follows the extended partitions */
	pt = (dos_partition_t *) (buffer + DOS_PART_TBL_OFFSET);
	for (i = 0; i < 4; i++, pt++) {
		if (is_extended (pt->sys_ind)) {
			lbaint_t lba_start
				= get_unaligned_le32 (pt->start4) + relative;

			return part_get_info_extended(dev_desc, lba_start,
				 ext_part_sector == 0 ? lba_start : relative,
				 part_num, which_part, info, disksig);
		}
	}

	/* Check for DOS PBR if no partition is found */
	dos_type = test_block_type(buffer);

	if (dos_type == DOS_PBR) {
		info->start = 0;
		info->size = dev_desc->lba;
		info->blksz = DOS_PART_DEFAULT_SECTOR;
		info->bootable = 0;
		strcpy((char *)info->type, "U-Boot");
#if CONFIG_IS_ENABLED(PARTITION_UUIDS)
		info->uuid[0] = 0;
#endif
		return 0;
	}

	return -1;
}

static void __maybe_unused part_print_dos(struct blk_desc *dev_desc)
{
#if defined(CONFIG_ARCH_SSTAR) && CONFIG_IS_ENABLED(CMD_SSTAR_MMC_FDISK)
	gu32_PartNum = 0;
#endif
	printf("Part\tStart Sector\tNum Sectors\tUUID\t\tType\n");
	print_partition_extended(dev_desc, 0, 0, 1, 0);
}

static int __maybe_unused part_get_info_dos(struct blk_desc *dev_desc, int part,
		      struct disk_partition *info)
{
	return part_get_info_extended(dev_desc, 0, 0, 1, part, info, 0);
}

int is_valid_dos_buf(void *buf)
{
	return test_block_type(buf) == DOS_MBR ? 0 : -1;
}

#if CONFIG_IS_ENABLED(CMD_MBR)
static void lba_to_chs(lbaint_t lba, unsigned char *rc, unsigned char *rh,
		       unsigned char *rs)
{
	unsigned int c, h, s;
	/* use fixed CHS geometry */
	unsigned int sectpertrack = 63;
	unsigned int heads = 255;

	c = (lba + 1) / sectpertrack / heads;
	h = (lba + 1) / sectpertrack - c * heads;
	s = (lba + 1) - (c * heads + h) * sectpertrack;

	if (c > 1023) {
		c = 1023;
		h = 254;
		s = 63;
	}

	*rc = c & 0xff;
	*rh = h;
	*rs = s + ((c & 0x300) >> 2);
}

static void mbr_fill_pt_entry(dos_partition_t *pt, lbaint_t start,
		lbaint_t relative, lbaint_t size, uchar sys_ind, bool bootable)
{
	pt->boot_ind = bootable ? 0x80 : 0x00;
	pt->sys_ind = sys_ind;
	lba_to_chs(start, &pt->cyl, &pt->head, &pt->sector);
	lba_to_chs(start + size - 1, &pt->end_cyl, &pt->end_head, &pt->end_sector);
	put_unaligned_le32(relative, &pt->start4);
	put_unaligned_le32(size, &pt->size4);
}

int write_mbr_partitions(struct blk_desc *dev,
		struct disk_partition *p, int count, unsigned int disksig)
{
	ALLOC_CACHE_ALIGN_BUFFER(unsigned char, buffer, dev->blksz);
	lbaint_t ext_part_start = 0, ext_part_size = 0, ext_part_sect = 0;
	dos_partition_t *pt;
	int i;

	memset(buffer, 0, dev->blksz);
	buffer[DOS_PART_MAGIC_OFFSET] = 0x55;
	buffer[DOS_PART_MAGIC_OFFSET + 1] = 0xaa;
	put_unaligned_le32(disksig, &buffer[DOS_PART_DISKSIG_OFFSET]);
	pt = (dos_partition_t *) (buffer + DOS_PART_TBL_OFFSET);

	/* create all primary partitions */
	for (i = 0; i < 4 && i < count; i++, pt++) {
		mbr_fill_pt_entry(pt, p[i].start, p[i].start, p[i].size,
				  p[i].sys_ind, p[i].bootable);
		if (is_extended(p[i].sys_ind)) {
			ext_part_start = p[i].start;
			ext_part_size = p[i].size;
			ext_part_sect = p[i].start;
		}
	}

	if (i < count && !ext_part_start) {
		printf("%s: extended partition is needed for more than 4 partitions\n",
		        __func__);
		return -1;
	}

	/* write MBR */
	if (blk_dwrite(dev, 0, 1, buffer) != 1) {
		printf("%s: failed writing 'MBR' (1 blks at 0x0)\n",
		       __func__);
		return -1;
	}

	/* create extended volumes */
	for (; i < count; i++) {
		lbaint_t next_ebr = 0;

		memset(buffer, 0, dev->blksz);
		buffer[DOS_PART_MAGIC_OFFSET] = 0x55;
		buffer[DOS_PART_MAGIC_OFFSET + 1] = 0xaa;
		pt = (dos_partition_t *) (buffer + DOS_PART_TBL_OFFSET);

		mbr_fill_pt_entry(pt, p[i].start, p[i].start - ext_part_sect,
				  p[i].size, p[i].sys_ind, p[i].bootable);

		if (i + 1 < count) {
			pt++;
			next_ebr = p[i].start + p[i].size;
			mbr_fill_pt_entry(pt, next_ebr,
					  next_ebr - ext_part_start,
					  p[i+1].start + p[i+1].size - next_ebr,
					  DOS_PART_TYPE_EXTENDED, 0);
		}

		/* write EBR */
		if (blk_dwrite(dev, ext_part_sect, 1, buffer) != 1) {
			printf("%s: failed writing 'EBR' (1 blks at 0x%lx)\n",
			       __func__, ext_part_sect);
			return -1;
		}
		ext_part_sect = next_ebr;
	}

	/* Update the partition table entries*/
	part_init(dev);

	return 0;
}

int layout_mbr_partitions(struct disk_partition *p, int count,
			  lbaint_t total_sectors)
{
	struct disk_partition *ext = NULL;
	int i, j;
	lbaint_t ext_vol_start;

	/* calculate primary partitions start and size if needed */
	if (!p[0].start)
		p[0].start = DOS_PART_DEFAULT_GAP;
	for (i = 0; i < 4 && i < count; i++) {
		if (!p[i].start)
			p[i].start = p[i - 1].start + p[i - 1].size;
		if (!p[i].size) {
			lbaint_t end = total_sectors;
			lbaint_t allocated = 0;

			for (j = i + 1; j < 4 && j < count; j++) {
				if (p[j].start) {
					end = p[j].start;
					break;
				}
				allocated += p[j].size;
			}
			p[i].size = end - allocated - p[i].start;
		}
		if (p[i].sys_ind == 0x05)
			ext = &p[i];
	}

	if (i >= 4 && !ext) {
		printf("%s: extended partition is needed for more than 4 partitions\n",
		        __func__);
		return -1;
	}

	/* calculate extended volumes start and size if needed */
	ext_vol_start = ext->start;
	for (i = 4; i < count; i++) {
		if (!p[i].start)
			p[i].start = ext_vol_start + DOS_PART_DEFAULT_GAP;
		if (!p[i].size) {
			lbaint_t end = ext->start + ext->size;
			lbaint_t allocated = 0;

			for (j = i + 1; j < count; j++) {
				if (p[j].start) {
					end = p[j].start - DOS_PART_DEFAULT_GAP;
					break;
				}
				allocated += p[j].size + DOS_PART_DEFAULT_GAP;
			}
			p[i].size = end - allocated - p[i].start;
		}
		ext_vol_start = p[i].start + p[i].size;
	}

	return 0;
}
#endif

int write_mbr_sector(struct blk_desc *dev_desc, void *buf)
{
	if (is_valid_dos_buf(buf))
		return -1;

	/* write MBR */
	if (blk_dwrite(dev_desc, 0, 1, buf) != 1) {
		printf("%s: failed writing '%s' (1 blks at 0x0)\n",
		       __func__, "MBR");
		return 1;
	}

	/* Update the partition table entries*/
	part_init(dev_desc);

	return 0;
}

#if defined(CONFIG_ARCH_SSTAR) && CONFIG_IS_ENABLED(CMD_SSTAR_MMC_FDISK)

static unsigned long long calc_unit(unsigned long long length, SDInfo sdInfo)
{
	// using LBA MODE
	if (sdInfo.addr_mode == CHS_MODE)
		if( (unsigned int)length % sdInfo.unit )
			return ( ((unsigned int)length / sdInfo.unit + 1 ) * sdInfo.unit);
		else
			return ( ((unsigned int)length / sdInfo.unit ) * sdInfo.unit);
	else
		return ( length );
}

static void encode_chs(int C, int H, int S, unsigned char *result)
{
	*result++ = (unsigned char) H;
	*result++ = (unsigned char) ( S + ((C & 0x00000300) >> 2) );
	*result   = (unsigned char) (C & 0x000000FF);
}

static void encode_partitionInfo(PartitionInfo partInfo, unsigned char *result)
{
	*result++ = partInfo.bootable;

	encode_chs(partInfo.C_start, partInfo.H_start, partInfo.S_start, result);
	result +=3;
	*result++ = partInfo.partitionId;

	encode_chs(partInfo.C_end, partInfo.H_end, partInfo.S_end, result);
	result += 3;

	memcpy(result, (unsigned char *)&(partInfo.block_start), 4);
	result += 4;

	memcpy(result, (unsigned char *)&(partInfo.block_count), 4);
}

static void decode_partitionInfo(unsigned char *in, PartitionInfo *partInfo)
{
	partInfo->bootable = *in;
	partInfo->partitionId = *(in + 4);

	memcpy((unsigned char *)&(partInfo->block_start), (in + 8), 4);
	memcpy((unsigned char *)&(partInfo->block_count), (in +12), 4);
}

static void get_SDInfo(unsigned    int block_count, SDInfo *sdInfo)
{
	int C, H, S;

	int C_max = 1023, H_max = 255, S_max = 63;
	int H_start = 1, S_start = 1;
	int diff_min = 0, diff = 0;

#if defined(CONFIG_MACH_MESON8_ODROIDC)
	sdInfo->addr_mode = LBA_MODE;
#else
	if(block_count >= _8_4GB)
			sdInfo->addr_mode = LBA_MODE;
	else
			sdInfo->addr_mode = CHS_MODE;
#endif

	//-----------------------------------------------------
	if (sdInfo->addr_mode == CHS_MODE)
	{
		diff_min = C_max;

		for (H = H_start; H <= H_max; H++)
			for (S	= S_start; S <= S_max; S++)
			{
				C = block_count / (H * S);

				if ( (C <= C_max) )
				{
					diff = C_max - C;
					if (diff <= diff_min)
					{
						diff_min = diff;
						sdInfo->C_end = C;
						sdInfo->H_end = H;
						sdInfo->S_end = S;
					}
				}
			}

	}
	//-----------------------------------------------------
	else
	{
		sdInfo->C_end = 1023;
		sdInfo->H_end = 254;
		sdInfo->S_end = 63;
	}

	//-----------------------------------------------------
	sdInfo->C_start 				= 0;
	sdInfo->H_start 				= 1;
	sdInfo->S_start 				= 1;

	sdInfo->total_block_count		= block_count;
	sdInfo->available_block 		= sdInfo->C_end * sdInfo->H_end * sdInfo->S_end;
	sdInfo->unit					= sdInfo->H_end * sdInfo->S_end;
}

static void make_partitionInfo(unsigned int LBA_start, unsigned int count, SDInfo sdInfo, PartitionInfo *partInfo, unsigned int ext_part_sector)
{
	int _10MB_unit;

	partInfo->block_start = LBA_start - ext_part_sector;

	//-----------------------------------------------------
	if (sdInfo.addr_mode == CHS_MODE)
	{
		partInfo->C_start = LBA_start / sdInfo.unit;
		partInfo->H_start = sdInfo.H_start - 1;
		partInfo->S_start = sdInfo.S_start;

		if (count == BLOCK_END)
		{
			_10MB_unit = calc_unit(CFG_PARTITION_START, sdInfo);
			partInfo->block_end = sdInfo.C_end * sdInfo.H_end * sdInfo.S_end - _10MB_unit - 1;
			partInfo->block_count = partInfo->block_end - LBA_start + 1;

			partInfo->C_end = partInfo->block_end / sdInfo.unit;
			partInfo->H_end = sdInfo.H_end - 1;
			partInfo->S_end = sdInfo.S_end;
		}
		else
		{
			partInfo->block_count = count;

			partInfo->block_end = LBA_start + count - 1;
			partInfo->C_end = partInfo->block_end / sdInfo.unit;

			partInfo->H_end = sdInfo.H_end - 1;
			partInfo->S_end = sdInfo.S_end;
		}
	}
	//-----------------------------------------------------
	else
	{
		partInfo->C_start = 0;
		partInfo->H_start = 1;
		partInfo->S_start = 1;

		partInfo->C_end = 1023;
		partInfo->H_end = 254;
		partInfo->S_end = 63;

		if (count == BLOCK_END)
		{
			_10MB_unit = calc_unit(CFG_PARTITION_START, sdInfo);
			partInfo->block_end = sdInfo.total_block_count - _10MB_unit - 1;
			partInfo->block_count = partInfo->block_end - LBA_start + 1;

		}
		else
		{
			partInfo->block_count = count;
			partInfo->block_end = LBA_start + count - 1;
		}
	}
}

static int make_mmc_partition(struct blk_desc *dev_desc,  int part_start_pre, int part_size_pre, int ext_part_sector, int relative, unsigned int part_num,	unsigned int part_size)
{
	ALLOC_CACHE_ALIGN_BUFFER(unsigned char, buffer, dev_desc->blksz);
	dos_partition_t *pt;
	SDInfo sdInfo;
	unsigned int block_start = 0, block_offset;
	PartitionInfo partInfo;
	int ext_part_lba;

	get_SDInfo(dev_desc->lba, &sdInfo);

	if (blk_dread (dev_desc, ext_part_sector, 1, (ulong *) buffer) != 1)
	{
		printf ("** Can't read partition table on %d:%d **\n", dev_desc->devnum, ext_part_sector);
		return 1;
	}

	pt = (dos_partition_t *) (buffer + DOS_PART_TBL_OFFSET);

	partInfo.bootable		= 0x00;

	switch(part_num)
	{
		case 1:
			partInfo.partitionId	= 0x0c;
			block_start = calc_unit((lbaint_t)CFG_PARTITION_START, sdInfo);
			block_offset = calc_unit(part_size, sdInfo);
			break;
		case 2:
		case 3:
			partInfo.partitionId	= 0x83;
			block_start = calc_unit((lbaint_t)(part_start_pre + part_size_pre), sdInfo);
			block_offset = calc_unit(part_size, sdInfo);
			break;
		case 4:
			partInfo.partitionId	= 0x05;
			block_start = calc_unit((lbaint_t)(part_start_pre + part_size_pre), sdInfo);
			block_offset = calc_unit(dev_desc->lba - block_start - sdInfo.unit, sdInfo);
			break;
		default:
			partInfo.partitionId	= 0x05;
			block_start = calc_unit((lbaint_t)(part_start_pre + part_size_pre), sdInfo);
			block_offset = calc_unit(part_size + CFG_EXT_PARTITION_RESERVE, sdInfo);
			break;
	}

	if(dev_desc->lba < block_start + block_offset)
		block_offset = dev_desc->lba - block_start - 1;

	make_partitionInfo(block_start, block_offset, sdInfo, &partInfo, relative);

	if(part_num <= 4)
		encode_partitionInfo(partInfo, (unsigned char *)&pt[part_num-1]);
	else
		encode_partitionInfo(partInfo, (unsigned char *)&pt[1]);

	buffer[DOS_PART_MAGIC_OFFSET] = 0x55;
	buffer[DOS_PART_MAGIC_OFFSET + 1] = 0xaa;

	blk_dwrite(dev_desc, ext_part_sector, 1, buffer);

	if(partInfo.partitionId == 0x05)
	{
		ext_part_lba = block_start;
		block_start += CFG_EXT_PARTITION_RESERVE;
		block_offset = calc_unit(block_start + part_size, sdInfo) - block_start;

		partInfo.bootable		= 0x00;
		partInfo.partitionId	= 0x83;
		make_partitionInfo(block_start, block_offset, sdInfo, &partInfo, ext_part_lba);

		memset(buffer, 0, dev_desc->blksz);
		encode_partitionInfo(partInfo, (unsigned char *)&pt[0]);
		buffer[DOS_PART_MAGIC_OFFSET] = 0x55;
		buffer[DOS_PART_MAGIC_OFFSET + 1] = 0xaa;

		blk_dwrite(dev_desc, ext_part_lba, 1, buffer);
	}

	return 0;
}

/*	create a partition that is relative to its Extended partition table
 */
static int create_partition_info_extended (struct blk_desc *dev_desc, int ext_part_sector,
				 int relative, int part_num, unsigned int part_size )
{
	ALLOC_CACHE_ALIGN_BUFFER(unsigned char, buffer, dev_desc->blksz);
	dos_partition_t *pt;
	int i;
	int ret = 0;
	int part_table;
	int part_start_pre = 0, part_size_pre = 0;

	if(relative==0)
		part_table = 4;
	else
		part_table = 2;

	if (blk_dread(dev_desc, ext_part_sector, 1, (ulong *) buffer) != 1)
	{
		printf ("** Can't read partition table on %d:%d **\n", dev_desc->devnum, ext_part_sector);
		return 1;
	}
	if (buffer[DOS_PART_MAGIC_OFFSET] != 0x55 || buffer[DOS_PART_MAGIC_OFFSET + 1] != 0xaa)
	{
		if(part_num == 1)
		{
			goto make_partition;
		}
		else
		{
			printf ("bad MBR sector signature 0x%02x%02x\n", buffer[DOS_PART_MAGIC_OFFSET], buffer[DOS_PART_MAGIC_OFFSET + 1]);
			return 1;
		}
	}

	/* Print all primary/logical partitions */
	pt = (dos_partition_t *) (buffer + DOS_PART_TBL_OFFSET);
	for (i = 0; i < part_table; i++, pt++)
	{
		if (((pt->boot_ind & ~0x80) == 0) && (pt->sys_ind != 0) && (ext_part_sector == 0 || !is_extended (pt->sys_ind)) )
		{
			part_start_pre = (lbaint_t)(ext_part_sector +get_unaligned_le32(pt->start4));
			part_size_pre = (lbaint_t)get_unaligned_le32(pt->size4);
			part_num++;
			continue;
		}
		else if(!is_extended (pt->sys_ind))
		{
			goto make_partition;
		}
	}

	pt = (dos_partition_t *) (buffer + DOS_PART_TBL_OFFSET);
	for (i = 0; i < part_table; i++, pt++)
	{
		if (is_extended (pt->sys_ind) )
		{
			int lba_start = get_unaligned_le32 (pt->start4) + relative;

			ret = create_partition_info_extended(dev_desc, lba_start, ext_part_sector == 0	? lba_start : relative, part_num, part_size);
		}
	}

	return ret;

make_partition:
	ret = make_mmc_partition(dev_desc, part_start_pre, part_size_pre, ext_part_sector, relative, part_num, part_size);
	if(ret)
		return ret;

	gu32_PartNum++;

	return ret;
}

static int delete_partition_info_extended (struct blk_desc *dev_desc, int ext_part_sector,
		 int relative, int part_num, unsigned int erase_parts )
{
	ALLOC_CACHE_ALIGN_BUFFER(unsigned char, buffer, dev_desc->blksz);
	dos_partition_t *pt;
	int i, ret = 0, lba_start;

	if (blk_dread(dev_desc, ext_part_sector, 1, (ulong *) buffer) != 1)
	{
		 printf ("** Can't read partition table on %d:%d **\n", dev_desc->devnum, ext_part_sector);
		 return -1;
	}
	if (buffer[DOS_PART_MAGIC_OFFSET] != 0x55 || buffer[DOS_PART_MAGIC_OFFSET + 1] != 0xaa)
	{
		 {
			 printf ("bad MBR sector signature 0x%02x%02x\n", buffer[DOS_PART_MAGIC_OFFSET], buffer[DOS_PART_MAGIC_OFFSET + 1]);
			 return -1;
		 }
	}

	/* Print all primary/logical partitions */

	pt = (dos_partition_t *) (buffer + DOS_PART_TBL_OFFSET);
	for (i = 0; i < 4; i++, pt++)
	{
		 if (((pt->boot_ind & ~0x80) == 0) && (pt->sys_ind != 0) && (!is_extended (pt->sys_ind)) && (part_num == erase_parts))
		 {
			 memset(pt, 0 , sizeof(dos_partition_t));
			 blk_dwrite(dev_desc, ext_part_sector, 1, buffer);
			 return 1;
		 }

		 /* Reverse engr the fdisk part# assignment rule! */
		 if ((ext_part_sector == 0) || (pt->sys_ind != 0 && !is_extended (pt->sys_ind)) )
		 {
			 part_num++;
		 }
	}
	pt = (dos_partition_t *) (buffer + DOS_PART_TBL_OFFSET);
	for (i = 0; i < 4; i++, pt++)
	{
		 if (is_extended (pt->sys_ind) )
		 {
			 lba_start = get_unaligned_le32 (pt->start4) + relative;

			 ret = delete_partition_info_extended(dev_desc, lba_start, ext_part_sector == 0  ? lba_start : relative, part_num, erase_parts);
			 if(ret == 1)
			 {
				 memset(pt, 0, sizeof(dos_partition_t));
				 blk_dwrite(dev_desc, ext_part_sector, 1, buffer);
				 return 0;
			 }
		 }
	}
	return ret;

}

int create_partition_dos(struct blk_desc *dev_desc, unsigned int part_size )
{
	return create_partition_info_extended(dev_desc, 0, 0, 1, part_size);
}

int delete_partition_dos(struct blk_desc *dev_desc, unsigned int erase_parts , int upgrade_flag)
{
	int ret = 0;
	if(gu32_PartNum == 0)
	{
		part_print_dos(dev_desc);
	}

	if(erase_parts > gu32_PartNum || erase_parts < 0)
	{
		printf("Partitino %u don't exist, max partition NO %u\n", erase_parts, gu32_PartNum);
		return -1;
	}

	if(erase_parts == 0)
	{
		while(gu32_PartNum)
		{
			if( upgrade_flag && (gu32_PartNum == 1) )
				break;

			ret = delete_partition_info_extended(dev_desc, 0, 0, 1, gu32_PartNum);
			if(ret == -1)
			{
				printf("Delete partition %u fail!\n", gu32_PartNum);
				return ret;
			}

			gu32_PartNum--;
		}
	}
	else
	{
		ret = delete_partition_info_extended(dev_desc, 0, 0, 1, erase_parts);
		if(ret != -1)
			gu32_PartNum--;
	}
	return ret;
}
#endif

U_BOOT_PART_TYPE(dos) = {
	.name		= "DOS",
	.part_type	= PART_TYPE_DOS,
	.max_entries	= DOS_ENTRY_NUMBERS,
	.get_info	= part_get_info_ptr(part_get_info_dos),
	.print		= part_print_ptr(part_print_dos),
	.test		= part_test_dos,
};

#endif
