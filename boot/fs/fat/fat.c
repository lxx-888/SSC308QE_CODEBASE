// SPDX-License-Identifier: GPL-2.0+
/*
 * fat.c
 *
 * R/O (V)FAT 12/16/32 filesystem implementation by Marcus Sundberg
 *
 * 2002-07-28 - rjones@nexus-tech.net - ported to ppcboot v1.1.6
 * 2003-03-10 - kharris@nexus-tech.net - ported to uboot
 */

#include <common.h>
#include <blk.h>
#include <config.h>
#include <exports.h>
#include <fat.h>
#include <fs.h>
#include <log.h>
#include <asm/byteorder.h>
#include <part.h>
#include <malloc.h>
#include <memalign.h>
#include <asm/cache.h>
#include <linux/compiler.h>
#include <linux/ctype.h>

/*
 * Convert a string to lowercase.  Converts at most 'len' characters,
 * 'len' may be larger than the length of 'str' if 'str' is NULL
 * terminated.
 */
static void downcase(char *str, size_t len)
{
	while (*str != '\0' && len--) {
		*str = tolower(*str);
		str++;
	}
}

static struct blk_desc *cur_dev;
static struct disk_partition cur_part_info;

#define DOS_BOOT_MAGIC_OFFSET	0x1fe
#define DOS_FS_TYPE_OFFSET	0x36
#define DOS_FS32_TYPE_OFFSET	0x52

static int disk_read(__u32 block, __u32 nr_blocks, void *buf)
{
	ulong ret;

	if (!cur_dev)
		return -1;

	ret = blk_dread(cur_dev, cur_part_info.start + block, nr_blocks, buf);

	if (ret != nr_blocks)
		return -1;

	return ret;
}

int fat_set_blk_dev(struct blk_desc *dev_desc, struct disk_partition *info)
{
	ALLOC_CACHE_ALIGN_BUFFER(unsigned char, buffer, dev_desc->blksz);

	cur_dev = dev_desc;
	cur_part_info = *info;

	/* Make sure it has a valid FAT header */
	if (disk_read(0, 1, buffer) != 1) {
		cur_dev = NULL;
		return -1;
	}

	/* Check if it's actually a DOS volume */
	if (memcmp(buffer + DOS_BOOT_MAGIC_OFFSET, "\x55\xAA", 2)) {
		cur_dev = NULL;
		return -1;
	}

	/* Check for FAT12/FAT16/FAT32 filesystem */
	if (!memcmp(buffer + DOS_FS_TYPE_OFFSET, "FAT", 3))
		return 0;
	if (!memcmp(buffer + DOS_FS32_TYPE_OFFSET, "FAT32", 5))
		return 0;

	cur_dev = NULL;
	return -1;
}

int fat_register_device(struct blk_desc *dev_desc, int part_no)
{
	struct disk_partition info;

	/* First close any currently found FAT filesystem */
	cur_dev = NULL;

	/* Read the partition table, if present */
	if (part_get_info(dev_desc, part_no, &info)) {
		if (part_no != 0) {
			printf("** Partition %d not valid on device %d **\n",
					part_no, dev_desc->devnum);
			return -1;
		}

		info.start = 0;
		info.size = dev_desc->lba;
		info.blksz = dev_desc->blksz;
		info.name[0] = 0;
		info.type[0] = 0;
		info.bootable = 0;
#if CONFIG_IS_ENABLED(PARTITION_UUIDS)
		info.uuid[0] = 0;
#endif
	}

	return fat_set_blk_dev(dev_desc, &info);
}

/*
 * Extract zero terminated short name from a directory entry.
 */
static void get_name(dir_entry *dirent, char *s_name)
{
	char *ptr;

	memcpy(s_name, dirent->nameext.name, 8);
	s_name[8] = '\0';
	ptr = s_name;
	while (*ptr && *ptr != ' ')
		ptr++;
	if (dirent->lcase & CASE_LOWER_BASE)
		downcase(s_name, (unsigned)(ptr - s_name));
	if (dirent->nameext.ext[0] && dirent->nameext.ext[0] != ' ') {
		*ptr++ = '.';
		memcpy(ptr, dirent->nameext.ext, 3);
		if (dirent->lcase & CASE_LOWER_EXT)
			downcase(ptr, 3);
		ptr[3] = '\0';
		while (*ptr && *ptr != ' ')
			ptr++;
	}
	*ptr = '\0';
	if (*s_name == DELETED_FLAG)
		*s_name = '\0';
	else if (*s_name == aRING)
		*s_name = DELETED_FLAG;
}

static int flush_dirty_fat_buffer(fsdata *mydata);

#if !CONFIG_IS_ENABLED(FAT_WRITE)
/* Stub for read only operation */
int flush_dirty_fat_buffer(fsdata *mydata)
{
	(void)(mydata);
	return 0;
}
#endif

/*
 * Get the entry at index 'entry' in a FAT (12/16/32) table.
 * On failure 0x00 is returned.
 */
static __u32 get_fatent(fsdata *mydata, __u32 entry)
{
	__u32 bufnum;
	__u32 offset, off8;
	__u32 ret = 0x00;

	if (CHECK_CLUST(entry, mydata->fatsize)) {
		printf("Error: Invalid FAT entry: 0x%08x\n", entry);
		return ret;
	}

	switch (mydata->fatsize) {
	case 32:
		bufnum = entry / FAT32BUFSIZE;
		offset = entry - bufnum * FAT32BUFSIZE;
		break;
	case 16:
		bufnum = entry / FAT16BUFSIZE;
		offset = entry - bufnum * FAT16BUFSIZE;
		break;
	case 12:
		bufnum = entry / FAT12BUFSIZE;
		offset = entry - bufnum * FAT12BUFSIZE;
		break;

	default:
		/* Unsupported FAT size */
		return ret;
	}

	debug("FAT%d: entry: 0x%08x = %d, offset: 0x%04x = %d\n",
	       mydata->fatsize, entry, entry, offset, offset);

	/* Read a new block of FAT entries into the cache. */
	if (bufnum != mydata->fatbufnum) {
		__u32 getsize = FATBUFBLOCKS;
		__u8 *bufptr = mydata->fatbuf;
		__u32 fatlength = mydata->fatlength;
		__u32 startblock = bufnum * FATBUFBLOCKS;

		/* Cap length if fatlength is not a multiple of FATBUFBLOCKS */
		if (startblock + getsize > fatlength)
			getsize = fatlength - startblock;

		startblock += mydata->fat_sect;	/* Offset from start of disk */

		/* Write back the fatbuf to the disk */
		if (flush_dirty_fat_buffer(mydata) < 0)
			return -1;

		if (disk_read(startblock, getsize, bufptr) < 0) {
			debug("Error reading FAT blocks\n");
			return ret;
		}
		mydata->fatbufnum = bufnum;
	}

	/* Get the actual entry from the table */
	switch (mydata->fatsize) {
	case 32:
		ret = FAT2CPU32(((__u32 *) mydata->fatbuf)[offset]);
		break;
	case 16:
		ret = FAT2CPU16(((__u16 *) mydata->fatbuf)[offset]);
		break;
	case 12:
		off8 = (offset * 3) / 2;
		/* fatbut + off8 may be unaligned, read in byte granularity */
		ret = mydata->fatbuf[off8] + (mydata->fatbuf[off8 + 1] << 8);

		if (offset & 0x1)
			ret >>= 4;
		ret &= 0xfff;
	}
	debug("FAT%d: ret: 0x%08x, entry: 0x%08x, offset: 0x%04x\n",
	       mydata->fatsize, ret, entry, offset);

	return ret;
}

/*
 * Read at most 'size' bytes from the specified cluster into 'buffer'.
 * Return 0 on success, -1 otherwise.
 */
static int
get_cluster(fsdata *mydata, __u32 clustnum, __u8 *buffer, unsigned long size)
{
	__u32 startsect;
	int ret;

	if (clustnum > 0) {
		startsect = clust_to_sect(mydata, clustnum);
	} else {
		startsect = mydata->rootdir_sect;
	}

	debug("gc - clustnum: %d, startsect: %d\n", clustnum, startsect);

	if ((unsigned long)buffer & (ARCH_DMA_MINALIGN - 1)) {
		ALLOC_CACHE_ALIGN_BUFFER(__u8, tmpbuf, mydata->sect_size);

		debug("FAT: Misaligned buffer address (%p)\n", buffer);

		while (size >= mydata->sect_size) {
			ret = disk_read(startsect++, 1, tmpbuf);
			if (ret != 1) {
				debug("Error reading data (got %d)\n", ret);
				return -1;
			}

			memcpy(buffer, tmpbuf, mydata->sect_size);
			buffer += mydata->sect_size;
			size -= mydata->sect_size;
		}
	} else {
		__u32 idx;

		idx = size / mydata->sect_size;
		if (idx == 0)
			ret = 0;
		else
			ret = disk_read(startsect, idx, buffer);
		if (ret != idx) {
			debug("Error reading data (got %d)\n", ret);
			return -1;
		}
		startsect += idx;
		idx *= mydata->sect_size;
		buffer += idx;
		size -= idx;
	}
	if (size) {
		ALLOC_CACHE_ALIGN_BUFFER(__u8, tmpbuf, mydata->sect_size);

		ret = disk_read(startsect, 1, tmpbuf);
		if (ret != 1) {
			debug("Error reading data (got %d)\n", ret);
			return -1;
		}

		memcpy(buffer, tmpbuf, size);
	}

	return 0;
}

/**
 * get_contents() - read from file
 *
 * Read at most 'maxsize' bytes from 'pos' in the file associated with 'dentptr'
 * into 'buffer'. Update the number of bytes read in *gotsize or return -1 on
 * fatal errors.
 *
 * @mydata:	file system description
 * @dentprt:	directory entry pointer
 * @pos:	position from where to read
 * @buffer:	buffer into which to read
 * @maxsize:	maximum number of bytes to read
 * @gotsize:	number of bytes actually read
 * Return:	-1 on error, otherwise 0
 */
static int get_contents(fsdata *mydata, dir_entry *dentptr, loff_t pos,
			__u8 *buffer, loff_t maxsize, loff_t *gotsize)
{
	loff_t filesize = FAT2CPU32(dentptr->size);
	unsigned int bytesperclust = mydata->clust_size * mydata->sect_size;
	__u32 curclust = START(dentptr);
	__u32 endclust, newclust;
	loff_t actsize;

	*gotsize = 0;
	debug("Filesize: %llu bytes\n", filesize);

	if (pos >= filesize) {
		debug("Read position past EOF: %llu\n", pos);
		return 0;
	}

	if (maxsize > 0 && filesize > pos + maxsize)
		filesize = pos + maxsize;

	debug("%llu bytes\n", filesize);

	actsize = bytesperclust;

	/* go to cluster at pos */
	while (actsize <= pos) {
		curclust = get_fatent(mydata, curclust);
		if (CHECK_CLUST(curclust, mydata->fatsize)) {
			debug("curclust: 0x%x\n", curclust);
			printf("Invalid FAT entry\n");
			return -1;
		}
		actsize += bytesperclust;
	}

	/* actsize > pos */
	actsize -= bytesperclust;
	filesize -= actsize;
	pos -= actsize;

	/* align to beginning of next cluster if any */
	if (pos) {
		__u8 *tmp_buffer;

		actsize = min(filesize, (loff_t)bytesperclust);
		tmp_buffer = malloc_cache_aligned(actsize);
		if (!tmp_buffer) {
			debug("Error: allocating buffer\n");
			return -1;
		}

		if (get_cluster(mydata, curclust, tmp_buffer, actsize) != 0) {
			printf("Error reading cluster\n");
			free(tmp_buffer);
			return -1;
		}
		filesize -= actsize;
		actsize -= pos;
		memcpy(buffer, tmp_buffer + pos, actsize);
		free(tmp_buffer);
		*gotsize += actsize;
		if (!filesize)
			return 0;
		buffer += actsize;

		curclust = get_fatent(mydata, curclust);
		if (CHECK_CLUST(curclust, mydata->fatsize)) {
			debug("curclust: 0x%x\n", curclust);
			printf("Invalid FAT entry\n");
			return -1;
		}
	}

	actsize = bytesperclust;
	endclust = curclust;

	do {
		/* search for consecutive clusters */
		while (actsize < filesize) {
			newclust = get_fatent(mydata, endclust);
			if ((newclust - 1) != endclust)
				goto getit;
			if (CHECK_CLUST(newclust, mydata->fatsize)) {
				debug("curclust: 0x%x\n", newclust);
				printf("Invalid FAT entry\n");
				return -1;
			}
			endclust = newclust;
			actsize += bytesperclust;
		}

		/* get remaining bytes */
		actsize = filesize;
		if (get_cluster(mydata, curclust, buffer, (int)actsize) != 0) {
			printf("Error reading cluster\n");
			return -1;
		}
		*gotsize += actsize;
		return 0;
getit:
		if (get_cluster(mydata, curclust, buffer, (int)actsize) != 0) {
			printf("Error reading cluster\n");
			return -1;
		}
		*gotsize += (int)actsize;
		filesize -= actsize;
		buffer += actsize;

		curclust = get_fatent(mydata, endclust);
		if (CHECK_CLUST(curclust, mydata->fatsize)) {
			debug("curclust: 0x%x\n", curclust);
			printf("Invalid FAT entry\n");
			return -1;
		}
		actsize = bytesperclust;
		endclust = curclust;
	} while (1);
}

/*
 * Extract the file name information from 'slotptr' into 'l_name',
 * starting at l_name[*idx].
 * Return 1 if terminator (zero byte) is found, 0 otherwise.
 */
static int slot2str(dir_slot *slotptr, char *l_name, int *idx)
{
	int j;

	for (j = 0; j <= 8; j += 2) {
		l_name[*idx] = slotptr->name0_4[j];
		if (l_name[*idx] == 0x00)
			return 1;
		(*idx)++;
	}
	for (j = 0; j <= 10; j += 2) {
		l_name[*idx] = slotptr->name5_10[j];
		if (l_name[*idx] == 0x00)
			return 1;
		(*idx)++;
	}
	for (j = 0; j <= 2; j += 2) {
		l_name[*idx] = slotptr->name11_12[j];
		if (l_name[*idx] == 0x00)
			return 1;
		(*idx)++;
	}

	return 0;
}

/* Calculate short name checksum */
static __u8 mkcksum(struct nameext *nameext)
{
	int i;
	u8 *pos = (void *)nameext;

	__u8 ret = 0;

	for (i = 0; i < 11; i++)
		ret = (((ret & 1) << 7) | ((ret & 0xfe) >> 1)) + pos[i];

	return ret;
}

/*
 * Read boot sector and volume info from a FAT filesystem
 */
static int
read_bootsectandvi(boot_sector *bs, volume_info *volinfo, int *fatsize)
{
	__u8 *block;
	volume_info *vistart;
	int ret = 0;

	if (cur_dev == NULL) {
		debug("Error: no device selected\n");
		return -1;
	}

	block = malloc_cache_aligned(cur_dev->blksz);
	if (block == NULL) {
		debug("Error: allocating block\n");
		return -1;
	}

	if (disk_read(0, 1, block) < 0) {
		debug("Error: reading block\n");
		goto fail;
	}

	memcpy(bs, block, sizeof(boot_sector));
	bs->reserved = FAT2CPU16(bs->reserved);
	bs->fat_length = FAT2CPU16(bs->fat_length);
	bs->secs_track = FAT2CPU16(bs->secs_track);
	bs->heads = FAT2CPU16(bs->heads);
	bs->total_sect = FAT2CPU32(bs->total_sect);

	/* FAT32 entries */
	if (bs->fat_length == 0) {
		/* Assume FAT32 */
		bs->fat32_length = FAT2CPU32(bs->fat32_length);
		bs->flags = FAT2CPU16(bs->flags);
		bs->root_cluster = FAT2CPU32(bs->root_cluster);
		bs->info_sector = FAT2CPU16(bs->info_sector);
		bs->backup_boot = FAT2CPU16(bs->backup_boot);
		vistart = (volume_info *)(block + sizeof(boot_sector));
		*fatsize = 32;
	} else {
		vistart = (volume_info *)&(bs->fat32_length);
		*fatsize = 0;
	}
	memcpy(volinfo, vistart, sizeof(volume_info));

	if (*fatsize == 32) {
		if (strncmp(FAT32_SIGN, vistart->fs_type, SIGNLEN) == 0)
			goto exit;
	} else {
		if (strncmp(FAT12_SIGN, vistart->fs_type, SIGNLEN) == 0) {
			*fatsize = 12;
			goto exit;
		}
		if (strncmp(FAT16_SIGN, vistart->fs_type, SIGNLEN) == 0) {
			*fatsize = 16;
			goto exit;
		}
	}

	debug("Error: broken fs_type sign\n");
fail:
	ret = -1;
exit:
	free(block);
	return ret;
}

static int get_fs_info(fsdata *mydata)
{
	boot_sector bs;
	volume_info volinfo;
	int ret;

	ret = read_bootsectandvi(&bs, &volinfo, &mydata->fatsize);
	if (ret) {
		debug("Error: reading boot sector\n");
		return ret;
	}

	if (mydata->fatsize == 32) {
		mydata->fatlength = bs.fat32_length;
		mydata->total_sect = bs.total_sect;
	} else {
		mydata->fatlength = bs.fat_length;
		mydata->total_sect = (bs.sectors[1] << 8) + bs.sectors[0];
		if (!mydata->total_sect)
			mydata->total_sect = bs.total_sect;
	}
	if (!mydata->total_sect) /* unlikely */
		mydata->total_sect = (u32)cur_part_info.size;

	mydata->fats = bs.fats;
	mydata->fat_sect = bs.reserved;

	mydata->rootdir_sect = mydata->fat_sect + mydata->fatlength * bs.fats;

	mydata->sect_size = (bs.sector_size[1] << 8) + bs.sector_size[0];
	mydata->clust_size = bs.cluster_size;
	if (mydata->sect_size != cur_part_info.blksz) {
		printf("Error: FAT sector size mismatch (fs=%hu, dev=%lu)\n",
				mydata->sect_size, cur_part_info.blksz);
		return -1;
	}
	if (mydata->clust_size == 0) {
		printf("Error: FAT cluster size not set\n");
		return -1;
	}
	if ((unsigned int)mydata->clust_size * mydata->sect_size >
	    MAX_CLUSTSIZE) {
		printf("Error: FAT cluster size too big (cs=%u, max=%u)\n",
		       (unsigned int)mydata->clust_size * mydata->sect_size,
		       MAX_CLUSTSIZE);
		return -1;
	}

	if (mydata->fatsize == 32) {
		mydata->data_begin = mydata->rootdir_sect -
					(mydata->clust_size * 2);
		mydata->root_cluster = bs.root_cluster;
	} else {
		mydata->rootdir_size = ((bs.dir_entries[1]  * (int)256 +
					 bs.dir_entries[0]) *
					 sizeof(dir_entry)) /
					 mydata->sect_size;
		mydata->data_begin = mydata->rootdir_sect +
					mydata->rootdir_size -
					(mydata->clust_size * 2);

		/*
		 * The root directory is not cluster-aligned and may be on a
		 * "negative" cluster, this will be handled specially in
		 * fat_next_cluster().
		 */
		mydata->root_cluster = 0;
	}

	mydata->fatbufnum = -1;
	mydata->fat_dirty = 0;
	mydata->fatbuf = malloc_cache_aligned(FATBUFSIZE);
	if (mydata->fatbuf == NULL) {
		debug("Error: allocating memory\n");
		return -1;
	}

	debug("FAT%d, fat_sect: %d, fatlength: %d\n",
	       mydata->fatsize, mydata->fat_sect, mydata->fatlength);
	debug("Rootdir begins at cluster: %d, sector: %d, offset: %x\n"
	       "Data begins at: %d\n",
	       mydata->root_cluster,
	       mydata->rootdir_sect,
	       mydata->rootdir_sect * mydata->sect_size, mydata->data_begin);
	debug("Sector size: %d, cluster size: %d\n", mydata->sect_size,
	      mydata->clust_size);

	return 0;
}

/**
 * struct fat_itr - directory iterator, to simplify filesystem traversal
 *
 * Implements an iterator pattern to traverse directory tables,
 * transparently handling directory tables split across multiple
 * clusters, and the difference between FAT12/FAT16 root directory
 * (contiguous) and subdirectories + FAT32 root (chained).
 *
 * Rough usage
 *
 * .. code-block:: c
 *
 *     for (fat_itr_root(&itr, fsdata); fat_itr_next(&itr); ) {
 *         // to traverse down to a subdirectory pointed to by
 *         // current iterator position:
 *         fat_itr_child(&itr, &itr);
 *     }
 *
 * For a more complete example, see fat_itr_resolve().
 */
struct fat_itr {
	/**
	 * @fsdata:		filesystem parameters
	 */
	fsdata *fsdata;
	/**
	 * @start_clust:	first cluster
	 */
	unsigned int start_clust;
	/**
	 * @clust:		current cluster
	 */
	unsigned int clust;
	/**
	 * @next_clust:		next cluster if remaining == 0
	 */
	unsigned int next_clust;
	/**
	 * @last_cluster:	set if last cluster of directory reached
	 */
	int last_cluster;
	/**
	 * @is_root:		is iterator at root directory
	 */
	int is_root;
	/**
	 * @remaining:		remaining directory entries in current cluster
	 */
	int remaining;
	/**
	 * @dent:		current directory entry
	 */
	dir_entry *dent;
	/**
	 * @dent_rem:		remaining entries after long name start
	 */
	int dent_rem;
	/**
	 * @dent_clust:		cluster of long name start
	 */
	unsigned int dent_clust;
	/**
	 * @dent_start:		first directory entry for long name
	 */
	dir_entry *dent_start;
	/**
	 * @l_name:		long name of current directory entry
	 */
	char l_name[VFAT_MAXLEN_BYTES];
	/**
	 * @s_name:		short 8.3 name of current directory entry
	 */
	char s_name[14];
	/**
	 * @name:		l_name if there is one, else s_name
	 */
	char *name;
	/**
	 * @block:		buffer for current cluster
	 */
	u8 block[MAX_CLUSTSIZE] __aligned(ARCH_DMA_MINALIGN);
};

static int fat_itr_isdir(fat_itr *itr);

/**
 * fat_itr_root() - initialize an iterator to start at the root
 * directory
 *
 * @itr: iterator to initialize
 * @fsdata: filesystem data for the partition
 * @return 0 on success, else -errno
 */
static int fat_itr_root(fat_itr *itr, fsdata *fsdata)
{
	if (get_fs_info(fsdata))
		return -ENXIO;

	itr->fsdata = fsdata;
	itr->start_clust = fsdata->root_cluster;
	itr->clust = fsdata->root_cluster;
	itr->next_clust = fsdata->root_cluster;
	itr->dent = NULL;
	itr->remaining = 0;
	itr->last_cluster = 0;
	itr->is_root = 1;

	return 0;
}

/**
 * fat_itr_child() - initialize an iterator to descend into a sub-
 * directory
 *
 * Initializes 'itr' to iterate the contents of the directory at
 * the current cursor position of 'parent'.  It is an error to
 * call this if the current cursor of 'parent' is pointing at a
 * regular file.
 *
 * Note that 'itr' and 'parent' can be the same pointer if you do
 * not need to preserve 'parent' after this call, which is useful
 * for traversing directory structure to resolve a file/directory.
 *
 * @itr: iterator to initialize
 * @parent: the iterator pointing at a directory entry in the
 *    parent directory of the directory to iterate
 */
static void fat_itr_child(fat_itr *itr, fat_itr *parent)
{
	fsdata *mydata = parent->fsdata;  /* for silly macros */
	unsigned clustnum = START(parent->dent);

	assert(fat_itr_isdir(parent));

	itr->fsdata = parent->fsdata;
	itr->start_clust = clustnum;
	if (clustnum > 0) {
		itr->clust = clustnum;
		itr->next_clust = clustnum;
		itr->is_root = 0;
	} else {
		itr->clust = parent->fsdata->root_cluster;
		itr->next_clust = parent->fsdata->root_cluster;
		itr->start_clust = parent->fsdata->root_cluster;
		itr->is_root = 1;
	}
	itr->dent = NULL;
	itr->remaining = 0;
	itr->last_cluster = 0;
}

/**
 * fat_next_cluster() - load next FAT cluster
 *
 * The function is used when iterating through directories. It loads the
 * next cluster with directory entries
 *
 * @itr:	directory iterator
 * @nbytes:	number of bytes read, 0 on error
 * Return:	first directory entry, NULL on error
 */
void *fat_next_cluster(fat_itr *itr, unsigned int *nbytes)
{
	int ret;
	u32 sect;
	u32 read_size;

	/* have we reached the end? */
	if (itr->last_cluster)
		return NULL;

	if (itr->is_root && itr->fsdata->fatsize != 32) {
		/*
		 * The root directory is located before the data area and
		 * cannot be indexed using the regular unsigned cluster
		 * numbers (it may start at a "negative" cluster or not at a
		 * cluster boundary at all), so consider itr->next_clust to be
		 * a offset in cluster-sized units from the start of rootdir.
		 */
		unsigned sect_offset = itr->next_clust * itr->fsdata->clust_size;
		unsigned remaining_sects = itr->fsdata->rootdir_size - sect_offset;
		sect = itr->fsdata->rootdir_sect + sect_offset;
		/* do not read past the end of rootdir */
		read_size = min_t(u32, itr->fsdata->clust_size,
				  remaining_sects);
	} else {
		sect = clust_to_sect(itr->fsdata, itr->next_clust);
		read_size = itr->fsdata->clust_size;
	}

	log_debug("FAT read(sect=%d), clust_size=%d, read_size=%u\n",
		  sect, itr->fsdata->clust_size, read_size);

	/*
	 * NOTE: do_fat_read_at() had complicated logic to deal w/
	 * vfat names that span multiple clusters in the fat16 case,
	 * which get_dentfromdir() probably also needed (and was
	 * missing).  And not entirely sure what fat32 didn't have
	 * the same issue..  We solve that by only caring about one
	 * dent at a time and iteratively constructing the vfat long
	 * name.
	 */
	ret = disk_read(sect, read_size, itr->block);
	if (ret < 0) {
		debug("Error: reading block\n");
		return NULL;
	}

	*nbytes = read_size * itr->fsdata->sect_size;
	itr->clust = itr->next_clust;
	if (itr->is_root && itr->fsdata->fatsize != 32) {
		itr->next_clust++;
		if (itr->next_clust * itr->fsdata->clust_size >=
		    itr->fsdata->rootdir_size) {
			debug("nextclust: 0x%x\n", itr->next_clust);
			itr->last_cluster = 1;
		}
	} else {
		itr->next_clust = get_fatent(itr->fsdata, itr->next_clust);
		if (CHECK_CLUST(itr->next_clust, itr->fsdata->fatsize)) {
			debug("nextclust: 0x%x\n", itr->next_clust);
			itr->last_cluster = 1;
		}
	}

	return itr->block;
}

static dir_entry *next_dent(fat_itr *itr)
{
	if (itr->remaining == 0) {
		unsigned nbytes;
		struct dir_entry *dent = fat_next_cluster(itr, &nbytes);

		/* have we reached the last cluster? */
		if (!dent) {
			/* a sign for no more entries left */
			itr->dent = NULL;
			return NULL;
		}

		itr->remaining = nbytes / sizeof(dir_entry) - 1;
		itr->dent = dent;
	} else {
		itr->remaining--;
		itr->dent++;
	}

	/* have we reached the last valid entry? */
	if (itr->dent->nameext.name[0] == 0)
		return NULL;

	return itr->dent;
}

static dir_entry *extract_vfat_name(fat_itr *itr)
{
	struct dir_entry *dent = itr->dent;
	int seqn = itr->dent->nameext.name[0] & ~LAST_LONG_ENTRY_MASK;
	u8 chksum, alias_checksum = ((dir_slot *)dent)->alias_checksum;
	int n = 0;

	while (seqn--) {
		char buf[13];
		int idx = 0;

		slot2str((dir_slot *)dent, buf, &idx);

		if (n + idx >= sizeof(itr->l_name))
			return NULL;

		/* shift accumulated long-name up and copy new part in: */
		memmove(itr->l_name + idx, itr->l_name, n);
		memcpy(itr->l_name, buf, idx);
		n += idx;

		dent = next_dent(itr);
		if (!dent)
			return NULL;
	}

	/*
	 * We are now at the short file name entry.
	 * If it is marked as deleted, just skip it.
	 */
	if (dent->nameext.name[0] == DELETED_FLAG ||
	    dent->nameext.name[0] == aRING)
		return NULL;

	itr->l_name[n] = '\0';

	chksum = mkcksum(&dent->nameext);

	/* checksum mismatch could mean deleted file, etc.. skip it: */
	if (chksum != alias_checksum) {
		debug("** chksum=%x, alias_checksum=%x, l_name=%s, s_name=%8s.%3s\n",
		      chksum, alias_checksum, itr->l_name, dent->nameext.name,
		      dent->nameext.ext);
		return NULL;
	}

	return dent;
}

/**
 * fat_itr_next() - step to the next entry in a directory
 *
 * Must be called once on a new iterator before the cursor is valid.
 *
 * @itr: the iterator to iterate
 * @return boolean, 1 if success or 0 if no more entries in the
 *    current directory
 */
static int fat_itr_next(fat_itr *itr)
{
	dir_entry *dent;

	itr->name = NULL;

	/*
	 * One logical directory entry consist of following slots:
	 *				name[0]	Attributes
	 *   dent[N - N]: LFN[N - 1]	N|0x40	ATTR_VFAT
	 *   ...
	 *   dent[N - 2]: LFN[1]	2	ATTR_VFAT
	 *   dent[N - 1]: LFN[0]	1	ATTR_VFAT
	 *   dent[N]:     SFN			ATTR_ARCH
	 */

	while (1) {
		dent = next_dent(itr);
		if (!dent) {
			itr->dent_start = NULL;
			return 0;
		}
		itr->dent_rem = itr->remaining;
		itr->dent_start = itr->dent;
		itr->dent_clust = itr->clust;
		if (dent->nameext.name[0] == DELETED_FLAG)
			continue;

		if (dent->attr & ATTR_VOLUME) {
			if ((dent->attr & ATTR_VFAT) == ATTR_VFAT &&
			    (dent->nameext.name[0] & LAST_LONG_ENTRY_MASK)) {
				/* long file name */
				dent = extract_vfat_name(itr);
				/*
				 * If succeeded, dent has a valid short file
				 * name entry for the current entry.
				 * If failed, itr points to a current bogus
				 * entry. So after fetching a next one,
				 * it may have a short file name entry
				 * for this bogus entry so that we can still
				 * check for a short name.
				 */
				if (!dent)
					continue;
				itr->name = itr->l_name;
				break;
			} else {
				/* Volume label or VFAT entry, skip */
				continue;
			}
		}

		/* short file name */
		break;
	}

	get_name(dent, itr->s_name);
	if (!itr->name)
		itr->name = itr->s_name;

	return 1;
}

/**
 * fat_itr_isdir() - is current cursor position pointing to a directory
 *
 * @itr: the iterator
 * @return true if cursor is at a directory
 */
static int fat_itr_isdir(fat_itr *itr)
{
	return !!(itr->dent->attr & ATTR_DIR);
}

/*
 * Helpers:
 */

#define TYPE_FILE 0x1
#define TYPE_DIR  0x2
#define TYPE_ANY  (TYPE_FILE | TYPE_DIR)

/**
 * fat_itr_resolve() - traverse directory structure to resolve the
 * requested path.
 *
 * Traverse directory structure to the requested path.  If the specified
 * path is to a directory, this will descend into the directory and
 * leave it iterator at the start of the directory.  If the path is to a
 * file, it will leave the iterator in the parent directory with current
 * cursor at file's entry in the directory.
 *
 * @itr: iterator initialized to root
 * @path: the requested path
 * @type: bitmask of allowable file types
 * @return 0 on success or -errno
 */
static int fat_itr_resolve(fat_itr *itr, const char *path, unsigned type)
{
	const char *next;

	/* chomp any extra leading slashes: */
	while (path[0] && ISDIRDELIM(path[0]))
		path++;

	/* are we at the end? */
	if (strlen(path) == 0) {
		if (!(type & TYPE_DIR))
			return -ENOENT;
		return 0;
	}

	/* find length of next path entry: */
	next = path;
	while (next[0] && !ISDIRDELIM(next[0]))
		next++;

	if (itr->is_root) {
		/* root dir doesn't have "." nor ".." */
		if ((((next - path) == 1) && !strncmp(path, ".", 1)) ||
		    (((next - path) == 2) && !strncmp(path, "..", 2))) {
			/* point back to itself */
			itr->clust = itr->fsdata->root_cluster;
			itr->next_clust = itr->fsdata->root_cluster;
			itr->start_clust = itr->fsdata->root_cluster;
			itr->dent = NULL;
			itr->remaining = 0;
			itr->last_cluster = 0;

			if (next[0] == 0) {
				if (type & TYPE_DIR)
					return 0;
				else
					return -ENOENT;
			}

			return fat_itr_resolve(itr, next, type);
		}
	}

	while (fat_itr_next(itr)) {
		int match = 0;
		unsigned n = max(strlen(itr->name), (size_t)(next - path));

		/* check both long and short name: */
		if (!strncasecmp(path, itr->name, n))
			match = 1;
		else if (itr->name != itr->s_name &&
			 !strncasecmp(path, itr->s_name, n))
			match = 1;

		if (!match)
			continue;

		if (fat_itr_isdir(itr)) {
			/* recurse into directory: */
			fat_itr_child(itr, itr);
			return fat_itr_resolve(itr, next, type);
		} else if (next[0]) {
			/*
			 * If next is not empty then we have a case
			 * like: /path/to/realfile/nonsense
			 */
			debug("bad trailing path: %s\n", next);
			return -ENOENT;
		} else if (!(type & TYPE_FILE)) {
			return -ENOTDIR;
		} else {
			return 0;
		}
	}

	return -ENOENT;
}

int file_fat_detectfs(void)
{
	boot_sector bs;
	volume_info volinfo;
	int fatsize;
	char vol_label[12];

	if (cur_dev == NULL) {
		printf("No current device\n");
		return 1;
	}

	if (IS_ENABLED(CONFIG_HAVE_BLOCK_DEVICE)) {
		printf("Interface:  %s\n", blk_get_if_type_name(cur_dev->if_type));
		printf("  Device %d: ", cur_dev->devnum);
		dev_print(cur_dev);
	}

	if (read_bootsectandvi(&bs, &volinfo, &fatsize)) {
		printf("\nNo valid FAT fs found\n");
		return 1;
	}

	memcpy(vol_label, volinfo.volume_label, 11);
	vol_label[11] = '\0';
	volinfo.fs_type[5] = '\0';

	printf("Filesystem: %s \"%s\"\n", volinfo.fs_type, vol_label);

	return 0;
}

int fat_exists(const char *filename)
{
	fsdata fsdata;
	fat_itr *itr;
	int ret;

	itr = malloc_cache_aligned(sizeof(fat_itr));
	if (!itr)
		return 0;
	ret = fat_itr_root(itr, &fsdata);
	if (ret)
		goto out;

	ret = fat_itr_resolve(itr, filename, TYPE_ANY);
	free(fsdata.fatbuf);
out:
	free(itr);
	return ret == 0;
}

/**
 * fat2rtc() - convert FAT time stamp to RTC file stamp
 *
 * @date:	FAT date
 * @time:	FAT time
 * @tm:		RTC time stamp
 */
static void __maybe_unused fat2rtc(u16 date, u16 time, struct rtc_time *tm)
{
	tm->tm_mday = date & 0x1f;
	tm->tm_mon = (date & 0x1e0) >> 4;
	tm->tm_year = (date >> 9) + 1980;

	tm->tm_sec = (time & 0x1f) << 1;
	tm->tm_min = (time & 0x7e0) >> 5;
	tm->tm_hour = time >> 11;

	rtc_calc_weekday(tm);
	tm->tm_yday = 0;
	tm->tm_isdst = 0;
}

int fat_size(const char *filename, loff_t *size)
{
	fsdata fsdata;
	fat_itr *itr;
	int ret;

	itr = malloc_cache_aligned(sizeof(fat_itr));
	if (!itr)
		return -ENOMEM;
	ret = fat_itr_root(itr, &fsdata);
	if (ret)
		goto out_free_itr;

	ret = fat_itr_resolve(itr, filename, TYPE_FILE);
	if (ret) {
		/*
		 * Directories don't have size, but fs_size() is not
		 * expected to fail if passed a directory path:
		 */
		free(fsdata.fatbuf);
		ret = fat_itr_root(itr, &fsdata);
		if (ret)
			goto out_free_itr;
		ret = fat_itr_resolve(itr, filename, TYPE_DIR);
		if (!ret)
			*size = 0;
		goto out_free_both;
	}

	*size = FAT2CPU32(itr->dent->size);
out_free_both:
	free(fsdata.fatbuf);
out_free_itr:
	free(itr);
	return ret;
}

int file_fat_read_at(const char *filename, loff_t pos, void *buffer,
		     loff_t maxsize, loff_t *actread)
{
	fsdata fsdata;
	fat_itr *itr;
	int ret;

	itr = malloc_cache_aligned(sizeof(fat_itr));
	if (!itr)
		return -ENOMEM;
	ret = fat_itr_root(itr, &fsdata);
	if (ret)
		goto out_free_itr;

	ret = fat_itr_resolve(itr, filename, TYPE_FILE);
	if (ret)
		goto out_free_both;

	debug("reading %s at pos %llu\n", filename, pos);

	/* For saving default max clustersize memory allocated to malloc pool */
	dir_entry *dentptr = itr->dent;

	ret = get_contents(&fsdata, dentptr, pos, buffer, maxsize, actread);

out_free_both:
	free(fsdata.fatbuf);
out_free_itr:
	free(itr);
	return ret;
}

int file_fat_read(const char *filename, void *buffer, int maxsize)
{
	loff_t actread;
	int ret;

	ret =  file_fat_read_at(filename, 0, buffer, maxsize, &actread);
	if (ret)
		return ret;
	else
		return actread;
}

int fat_read_file(const char *filename, void *buf, loff_t offset, loff_t len,
		  loff_t *actread)
{
	int ret;

	ret = file_fat_read_at(filename, offset, buf, len, actread);
	if (ret)
		printf("** Unable to read file %s **\n", filename);

	return ret;
}

typedef struct {
	struct fs_dir_stream parent;
	struct fs_dirent dirent;
	fsdata fsdata;
	fat_itr itr;
} fat_dir;

int fat_opendir(const char *filename, struct fs_dir_stream **dirsp)
{
	fat_dir *dir;
	int ret;

	dir = malloc_cache_aligned(sizeof(*dir));
	if (!dir)
		return -ENOMEM;
	memset(dir, 0, sizeof(*dir));

	ret = fat_itr_root(&dir->itr, &dir->fsdata);
	if (ret)
		goto fail_free_dir;

	ret = fat_itr_resolve(&dir->itr, filename, TYPE_DIR);
	if (ret)
		goto fail_free_both;

	*dirsp = (struct fs_dir_stream *)dir;
	return 0;

fail_free_both:
	free(dir->fsdata.fatbuf);
fail_free_dir:
	free(dir);
	return ret;
}

int fat_readdir(struct fs_dir_stream *dirs, struct fs_dirent **dentp)
{
	fat_dir *dir = (fat_dir *)dirs;
	struct fs_dirent *dent = &dir->dirent;

	if (!fat_itr_next(&dir->itr))
		return -ENOENT;

	memset(dent, 0, sizeof(*dent));
	strcpy(dent->name, dir->itr.name);
	if (CONFIG_IS_ENABLED(EFI_LOADER)) {
		dent->attr = dir->itr.dent->attr;
		fat2rtc(le16_to_cpu(dir->itr.dent->cdate),
			le16_to_cpu(dir->itr.dent->ctime), &dent->create_time);
		fat2rtc(le16_to_cpu(dir->itr.dent->date),
			le16_to_cpu(dir->itr.dent->time), &dent->change_time);
		fat2rtc(le16_to_cpu(dir->itr.dent->adate),
			0, &dent->access_time);
	}
	if (fat_itr_isdir(&dir->itr)) {
		dent->type = FS_DT_DIR;
	} else {
		dent->type = FS_DT_REG;
		dent->size = FAT2CPU32(dir->itr.dent->size);
	}

	*dentp = dent;

	return 0;
}

void fat_closedir(struct fs_dir_stream *dirs)
{
	fat_dir *dir = (fat_dir *)dirs;
	free(dir->fsdata.fatbuf);
	free(dir);
}

void fat_close(void)
{
}

int fat_uuid(char *uuid_str)
{
	boot_sector bs;
	volume_info volinfo;
	int fatsize;
	int ret;
	u8 *id;

	ret = read_bootsectandvi(&bs, &volinfo, &fatsize);
	if (ret)
		return ret;

	id = volinfo.volume_id;
	sprintf(uuid_str, "%02X%02X-%02X%02X", id[3], id[2], id[1], id[0]);

	return 0;
}

#if defined(CONFIG_ARCH_SSTAR) && defined(CONFIG_SSTAR_MMC_HOST)
#define SECTOR_SIZE     512
static unsigned long part_offset = 0;
static unsigned int cur_part = 1;

/*
 * Copy string, padding with spaces.
 */
static void setstr(u_int8_t *dest, const char *src, size_t len)
{
	while (len--)
	*dest++ = *src ? *src++ : ' ';
}

static int write_pbr(struct blk_desc *dev_desc, struct disk_partition *info)
{
	struct bs *bs;
	struct bsbpb *bsbpb;
	struct bsxbpb *bsxbpb;
	struct bsx *bsx;
	int img_offset = 0;
//	int reserved_cnt= 0;
	int i;
	int fat_size = 0;
	ALLOC_CACHE_ALIGN_BUFFER(__u8, img, sizeof(__u8)*512);

	memset(img, 0x0, sizeof(__u8)*512);


	/* Erase Reserved Sector(PBR) */
	for (i = 0;i < RESERVED_CNT; i++) {
		if (blk_dwrite(dev_desc, info->start + i, 1, (ulong *)img) != 1) {
			printf ("Can't erase reserved sector~~~!!!\n");
			return -1;
		}
	}

	/* Set bs */
	bs = (struct bs *)img;
	img_offset += sizeof(struct bs);

	mk1(bs->jmp[0], 0xeb);
	mk1(bs->jmp[1], 0x58);
	mk1(bs->jmp[2], 0x90); /* Jump Boot Code */
	setstr(bs->oem, "SSTAR", sizeof(bs->oem)); /* OEM Name */

	uint spc;
	/* Set bsbpb */
	bsbpb = (struct bsbpb *)(img + img_offset);
	img_offset += sizeof(struct bsbpb);

	mk2(bsbpb->bps, 512); /* Byte Per Sector */
	/* Sector Per Cluster */
	if (info->size < 0x10000) { /* partition size >= 32Mb */
		printf("Can't format less than 32Mb partition!!\n");
		return -1;
	}
	if (info->size <= 0x20000) { /* under 64M -> 512 bytes */
		printf("Under 64M\n");
		mk1(bsbpb->spc, 1);
		spc = 1;
	}
	else if (info->size <= 0x40000) { /* under 128M -> 1K */
		printf("Under 128M\n");
		mk1(bsbpb->spc, 2);
		spc = 2;
	}
	else if (info->size <= 0x80000) { /* under 256M -> 2K */
		printf("Under 256M\n");
		mk1(bsbpb->spc, 4);
		spc = 4;
	}
	else if (info->size <= 0xFA0000) { /* under 8G -> 4K */
		printf("Under 8G\n");
		mk1(bsbpb->spc, 8);
		spc = 8;
	}
	else if (info->size <= 0x1F40000) { /* under 16G -> 8K */
		printf("Under 16G\n");
		mk1(bsbpb->spc, 16);
		spc = 16;
	}
	else {
		printf("16G~\n");
		mk1(bsbpb->spc, 32);
		spc = 32;
	}

	mk2(bsbpb->res, RESERVED_CNT); /* Reserved Sector Count */
	mk1(bsbpb->nft, 2); /* Number of FATs */
	mk2(bsbpb->rde, 0); /* Root Directory Entry Count : It's no use in FAT32 */
	mk2(bsbpb->sec, 0); /* Total Sector : It's no use in FAT32 */
	mk1(bsbpb->mid, 0xF8); /* Media */
	mk2(bsbpb->spf, 0); /* FAT Size 16 : It's no use in FAT32 */
	mk2(bsbpb->spt, 0); /* Sector Per Track */
	mk2(bsbpb->hds, 0); /* Number Of Heads */
	mk4(bsbpb->hid, 0); /* Hidden Sector */
	mk4(bsbpb->bsec, info->size); /* Total Sector For FAT32 */

	/* Set bsxbpb */
	bsxbpb = (struct bsxbpb *)(img + img_offset);
	img_offset += sizeof(struct bsxbpb);

	mk4(bsxbpb->bspf, (info->size / (spc * 128))); /* FAT Size 32 */
	fat_size = info->size / (spc * 128);
	printf("Fat size : 0x%lx\n", info->size / (spc * 128));
	mk2(bsxbpb->xflg, 0); /* Ext Flags */
	mk2(bsxbpb->vers, 0); /* File System Version */
	mk4(bsxbpb->rdcl, 2); /* Root Directory Cluster */
	mk2(bsxbpb->infs, 1); /* File System Information */
	mk2(bsxbpb->bkbs, 0); /* Boot Record Backup Sector */

	/* Set bsx */
	bsx = (struct bsx *)(img + img_offset);
	mk1(bsx->drv, 0); /* Drive Number */
	mk1(bsx->sig, 0x29); /* Boot Signature */
	mk4(bsx->volid, 0x3333); /* Volume ID : 0x3333 means nothing */
	setstr(bsx->label, "NO NAME ", sizeof(bsx->label)); /* Volume Label */
	setstr(bsx->type, "FAT32", sizeof(bsx->type)); /* File System Type */

	/* Set Magic Number */
	mk2(img + BYTE_PER_SEC - 2, 0xaa55); /* Signature */

	/*
	printf("Print Boot Recode\n");
	for(i = 0;i<512;i++) {
		if(img[i] == 0)
			printf("00 ");
		else
			printf("%2x ", img[i]);
		if (!((i+1) % 16))
			printf("\n");
	}
	*/

	if (blk_dwrite(dev_desc, info->start, 1, (ulong *)img) != 1) {
		printf ("Can't write PBR~~~!!!\n");
		return -1;
	}

	return fat_size;
}

static int write_reserved(struct blk_desc *dev_desc, struct disk_partition *info)
{
	/* Set Reserved Region */
	ALLOC_CACHE_ALIGN_BUFFER(__u8, img, sizeof(__u8)*512);
//	int i;

	memset(img, 0x0, sizeof(__u8)*512);

	mk4(img, 0x41615252); /* Lead Signature */
	mk4(img + BYTE_PER_SEC - 28, 0x61417272); /* Struct Signature */
	mk4(img + BYTE_PER_SEC - 24, 0xffffffff); /* Free Cluster Count */
	mk4(img + BYTE_PER_SEC - 20, 0x3); /* Next Free Cluster */
	mk2(img + BYTE_PER_SEC - 2, 0xaa55); /* Trail Signature */

/*
	printf("Print Reserved Region\n");
	for(i = 0;i<512;i++) {
		if(img[i] == 0)
			printf("00 ");
		else
			printf("%2x ", img[i]);
		if (!((i+1) % 16))
			printf("\n");
	}
*/

	/* Write Reserved region */
	if (blk_dwrite(dev_desc, info->start+1, 1, (ulong *)img) != 1) {
		printf ("Can't write reserved region~~~!!!\n");
		return -1;
	}

	return 1;
}
static int write_fat(struct blk_desc *dev_desc, struct disk_partition *info, int
fat_size)
{
	int i;
	int ret = -1;
	/* Create buffer for FAT */
	ALLOC_CACHE_ALIGN_BUFFER(__u8, img, sizeof(__u8)*512);
	/* Create buffer for erase */
	ALLOC_CACHE_ALIGN_BUFFER(__u8, dummy, sizeof(__u8) * 8192);

	memset(img, 0x0, sizeof(__u8) * 512);
	memset(dummy, 0x0, sizeof(__u8) * 8192);

	/* Erase FAT Region */
	int erase_block_cnt = (fat_size * 2);
//	int temp = 0;
	printf("Erase FAT region");
	for (i = 0;i < erase_block_cnt + 10; i+=16) {
		if (blk_dwrite(dev_desc, info->start +
			RESERVED_CNT + i, 16, (ulong *)dummy) != 16) {
			printf ("Can't erase FAT region~~!!!\n");
		}
		if((i % 160) == 0)
			printf(".");

	}
	printf("\n");

	mk4(img, 0x0ffffff8);
	mk4(img+4, 0x0fffffff);
	mk4(img+8, 0x0fffffff); /* Root Directory */

	/*
	printf("Print FAT Region\n");
	for(i = 0;i<512;i++) {
		if(img[i] == 0)
			printf("00 ");
		else
			printf("%2x ", img[i]);
		if (!((i+1) % 16))
			printf("\n");
	}
	*/
	/* Write FAT Region */
	if (blk_dwrite(dev_desc, info->start + RESERVED_CNT, 1, (ulong *)img) != 1) {
		printf ("Can't write FAT~~~!!!\n");
		goto write_err;
	}

	ret = 1;

write_err:
	return ret;
}

/*
 * Format device
 */
int fat_format_device(struct blk_desc *dev_desc, int part_no)
{
	struct disk_partition info;
	ALLOC_CACHE_ALIGN_BUFFER(unsigned char, buffer, sizeof(unsigned char)*SECTOR_SIZE);

	if (!dev_desc)
		return -1;
	cur_dev = dev_desc;
	/* check if we have a MBR (on floppies we have only a PBR) */
	if (blk_dread(dev_desc, 0, 1, (ulong *) buffer) != 1) {
		printf ("** Can't read from blk device %d **\n", dev_desc->devnum);
		return -1;
	}
	if (buffer[DOS_BOOT_MAGIC_OFFSET] != 0x55 ||
		buffer[DOS_BOOT_MAGIC_OFFSET + 1] != 0xaa) {
		printf("** MBR is broken **\n");
		/* no signature found */
		return -1;
	}
#if (defined(CONFIG_CMD_IDE) || defined(CONFIG_CMD_SCSI) || defined(CONFIG_CMD_USB) || defined(CONFIG_MMC) || defined(CONFIG_SYSTEMACE) )
	/* First we assume, there is a MBR */
	if (!part_get_info (dev_desc, part_no, &info)) {
		part_offset = info.start;
		cur_part = part_no;
	} else if (!strncmp((char *)&buffer[DOS_FS_TYPE_OFFSET], "FAT", 3)) {
		/* ok, we assume we are on a PBR only */
		cur_part = 1;
		part_offset = 0;
	} else {
		printf ("** Partition %d not valid on blk device %d **\n", part_no, dev_desc->devnum);
		return -1;
	}
#endif

	printf("Partition%d: Start Address(0x%lx), Size(0x%lx)\n", part_no, info.start, info.size);

	int fat_size;
	fat_size = write_pbr(dev_desc, &info);
	if(fat_size < 0)
		return -1;
	if(write_reserved(dev_desc, &info) < 0)
		return -1;
	if(write_fat(dev_desc, &info, fat_size) < 0)
		return -1;
	printf("Partition%d format complete.\n", part_no);

	return 0;
}
#endif
