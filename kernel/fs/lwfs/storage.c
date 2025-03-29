/*
* storage.c- Sigmastar
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

#include <linux/fs.h>
#include <linux/mtd/super.h>
#include <linux/buffer_head.h>
#include <linux/slab.h>
#include "internal.h"

#if !defined(CONFIG_LWFS_ON_MTD) && !defined(CONFIG_LWFS_ON_BLOCK)
#error no LWFS backing store interface configured
#endif

#ifdef CONFIG_LWFS_ON_MTD
#define LWFS_MTD_READ(sb, ...) mtd_read((sb)->s_mtd, ##__VA_ARGS__)
/*
 * Free Bad Block Table
 */
static void lwfs_mtd_bbt_map_free(struct super_block *sb)
{
    struct lwfs_sb_info *sbi = sb->s_fs_info;

    if (sbi && sbi->bbt_maps)
    {
        kfree(sbi->bbt_maps);
        sbi->bbt_maps = NULL;
    }
}

/*
 * Get Suitable Block by BBT
 */
static int lwfs_mtd_get_suitable_pos(struct super_block *sb, unsigned int pos_pre, unsigned int *pos_new)
{
    struct lwfs_sb_info *sbi = sb->s_fs_info;
    unsigned int block_size  = sb->s_mtd->erasesize;
    unsigned int pos_blk_idx_new  = 0;
    unsigned int pos_blk_idx_pre  = pos_pre/block_size;
    unsigned int pos_off = pos_pre % sb->s_mtd->erasesize;

    if (sbi->bbt_maps == NULL)
    {
        *pos_new = pos_pre;
        return 0;
    }

    pos_blk_idx_new = sbi->bbt_maps[pos_blk_idx_pre];
    if (pos_blk_idx_new == BBT_MAP_INVALID)
    {
        return -EINVAL;
    }
    *pos_new = pos_off + pos_blk_idx_new * sb->s_mtd->erasesize;
    return 0;
}

/*
 * Build a Bad Block Table if device support
 */
static int lwfs_mtd_bbt_map_build(struct super_block *sb)
{
    bool flag;
    unsigned int bad_num = 0;
    unsigned int *maps = NULL;
    unsigned int i = 0, j = 0;
    struct lwfs_sb_info *sbi = sb->s_fs_info;
    unsigned int block_count = sb->s_mtd->size / sb->s_mtd->erasesize;
    unsigned int block_size = sb->s_mtd->erasesize;

    pr_debug("mtd type:%d, blockcnt:%d\n", sb->s_mtd->type, block_count);

    if (block_count == 0)
    {
        pr_err("block count invalid\n");
        return -EINVAL;
    }

    maps = kzalloc(sizeof(*maps) * block_count, GFP_KERNEL);
    if (maps == NULL)
        return -ENOMEM;

    for (i = 0; i < block_count; i++)
    {
        flag = mtd_block_isbad((sb)->s_mtd, i * block_size);
        if (flag == false)
        {
            maps[j++] = i;
        } else
        {
            pr_info("block %d is a factory bad block\n", i);
            bad_num++;
        }
    }

    for (; j < block_count; j++)
    {
        maps[j] = BBT_MAP_INVALID;
    }

    pr_debug("BBT map table begin:\n");
    for (j = 0; j < block_count; j++)
    {
        if (j != 0 && (j % 8) == 0)
            pr_debug("\n");
        pr_debug("%08dx, %08dx;", j, maps[j]);
    }
    pr_debug("\nBBT map table end:\n");

    sbi->bbt_maps = maps;
    return 0;
}

/*
 * Read content and skip bad block if exists
 */
static int lwfs_mtd_read_skip_bad(struct super_block *sb, unsigned long pos,
           void *buf, size_t buflen)
{
    unsigned int rlen = 0, llen=0, alen = 0;
    unsigned int blksize = sb->s_mtd->erasesize;
    unsigned int pos_c = 0, pos_d = 0;
    int ret = 0;

    pos_c = pos;
    llen = buflen;
    do
    {
        rlen = blksize - pos_c % blksize;
        rlen = llen > rlen ? rlen : llen;

        ret = lwfs_mtd_get_suitable_pos(sb, pos_c, &pos_d);
        if (ret)
        {
            pr_err("fail to found block, pos:%d\n", pos_c);
            break;
        }

        ret = LWFS_MTD_READ(sb, pos_d, rlen, &alen, buf + buflen - llen);
        if (ret < 0 || rlen != alen)
        {
            ret = -EIO;
            break;
        }
        llen -= rlen;
        pos_c += rlen;
    } while (llen > 0);
    return ret;
}

/*
 * read data from an lwfs image on an MTD device
 */
static int lwfs_mtd_read(struct super_block *sb, unsigned long pos,
              void *buf, size_t buflen)
{
    size_t rlen;
    int ret;

    ret = LWFS_MTD_READ(sb, pos, buflen, &rlen, buf);
    return (ret < 0 || rlen != buflen) ? -EIO : 0;
}
#endif /* CONFIG_LWFS_ON_MTD */

#ifdef CONFIG_LWFS_ON_BLOCK
/*
 * read data from an lwfs image on a block device
 */
static int lwfs_blk_read(struct super_block *sb, unsigned long pos,
              void *buf, size_t buflen)
{
    struct buffer_head *bh;
    unsigned long offset;
    size_t segment;

    /* copy the string up to blocksize bytes at a time */
    while (buflen > 0) {
        offset = pos & (LWBSIZE - 1);
        segment = min_t(size_t, buflen, LWBSIZE - offset);
        bh = sb_bread(sb, pos >> LWBSBITS);
        if (!bh)
            return -EIO;
        memcpy(buf, bh->b_data + offset, segment);
        brelse(bh);
        buf += segment;
        buflen -= segment;
        pos += segment;
    }

    return 0;
}
#endif /* CONFIG_LWFS_ON_BLOCK */

/*
 * read data from the lwfs image
 */
int lwfs_dev_read(struct super_block *sb, unsigned long pos,
           void *buf, size_t buflen)
{
    size_t limit;

    limit = lwfs_maxsize(sb);
    if (pos >= limit || buflen > limit - pos)
        return -EIO;

#ifdef CONFIG_LWFS_ON_MTD
    if (sb->s_mtd)
    {
        if (mtd_type_is_nand(sb->s_mtd))
        {
            return lwfs_mtd_read_skip_bad(sb, pos, buf, buflen);
        }
        else
        {
            return lwfs_mtd_read(sb, pos, buf, buflen);
        }
    }
#elif CONFIG_LWFS_ON_BLOCK
    if (sb->s_bdev)
        return lwfs_blk_read(sb, pos, buf, buflen);
#endif
    return -EIO;
}

int lwfs_dev_init(struct super_block *sb)
{
    int ret = 0;
#ifdef CONFIG_LWFS_ON_MTD
    if (sb->s_mtd)
    {
        if (mtd_type_is_nand(sb->s_mtd))
        {
            ret = lwfs_mtd_bbt_map_build(sb);
        }
    }
#endif
    return ret;

}

void lwfs_dev_exit(struct super_block *sb)
{
#ifdef CONFIG_LWFS_ON_MTD
    if (sb->s_mtd)
    {
        if (mtd_type_is_nand(sb->s_mtd))
        {
            lwfs_mtd_bbt_map_free(sb);
        }
    }
#endif
}
