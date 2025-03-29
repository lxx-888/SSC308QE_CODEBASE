/*
* internal.h- Sigmastar
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

#ifndef __INTERNAL_H__
#define __INTERNAL_H__

typedef struct
{
    u32 magic;
    u32 ver;
    u32 size;
    u32 align_unit;
    u32 file_num;
    u8  dummy[44];
} LwFsPartitionInfo_t;

typedef struct
{
    char    name[32];
    u32     offset;
    u32     length;
    u32     padding;
    u32     crc32;
    u32     compressed;
    u8      dummy[12];
} LwFsFileInfo_t;

struct lwfs_inode_info {
    struct inode    vfs_inode;
    unsigned long    i_metasize;    /* size of non-data area */
    unsigned long    i_dataoffset;    /* from the start of fs */
};

#define LWBSIZE BLOCK_SIZE
#define LWBSBITS BLOCK_SIZE_BITS
#define LWBMASK (LWBSIZE-1)
#define LWFS_PH_SIZE    sizeof(LwFsPartitionInfo_t)
#define LWFS_FH_SIZE    sizeof(LwFsFileInfo_t)
#define LWFS_MAGIC      0x5346574C  //'L','W','F','S'
#define LWFS_MAXFN      32
#define BBT_MAP_INVALID (0xffffffff)

struct lwfs_sb_info
{
    size_t size;    // total size of super block
    int *bbt_maps;  // bad block table maps
};

static inline size_t lwfs_maxsize(struct super_block *sb)
{
    return ((struct lwfs_sb_info*) (sb->s_fs_info))->size;
}

static inline struct lwfs_inode_info *LWFS_I(struct inode *inode)
{
    return container_of(inode, struct lwfs_inode_info, vfs_inode);
}

/*
 * mmap-nommu.c
 */
#if !defined(CONFIG_MMU) && defined(CONFIG_LWFS_ON_MTD)
extern const struct file_operations lwfs_ro_fops;
#else
#define lwfs_ro_fops    generic_ro_fops
#endif

/*
 * storage.c
 */
int lwfs_dev_read(struct super_block *sb, unsigned long pos,
              void *buf, size_t buflen);
int lwfs_dev_init(struct super_block *sb);
void lwfs_dev_exit(struct super_block *sb);

#endif  // __INTERNAL_H__
