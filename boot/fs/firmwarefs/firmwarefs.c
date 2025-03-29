/*
 * firmwarefs.c - Sigmastar
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

#include <inttypes.h>
#include <stdlib.h>
#include <linux/mtd/mtd.h>
#include <common.h>
#include <fwfs.h>
#include <fwfs_util.h>
#include <firmwarefs.h>
#include <spi.h>
#include <spi_flash.h>

#if defined(CONFIG_FIRMWAREFS_ON_EMMC)
#include <part.h>
#else
#if defined(CONFIG_SSTAR_NAND)
#include <nand.h>
#else
static struct spi_flash *spinor_flash;
#endif
#endif

#if defined(CONFIG_FS_FIRMWAREFS)

#define BBT_MAP_INVALID (0xffffffff)

struct fwfs_mnt_handle
{
    fwfs_t             fwfs;
    struct fwfs_config cfg;
};

struct fwfs_file_handle
{
    fwfs_t *    fwfs_file;
    fwfs_file_t fwfs_fd;
} FirmwarefsFileHandle;

#if defined(CONFIG_FIRMWAREFS_ON_EMMC)
struct fwfs_dev_and_part_context
{
    struct blk_desc *     dev_desc;
    struct disk_partition info;
};
#endif

extern int printf(const char *fmt, ...);
#if !defined(CONFIG_FIRMWAREFS_ON_EMMC)
static int get_mtdpart_range(char *partition, u64 *offset, u64 *size)
{
    struct mtd_device *dev;
    struct part_info * part;
    u8                 pnum;
    int                ret;

    ret = mtdparts_init();
    if (ret)
    {
        fprintf(stderr, "Failed to init mtdparts:%d\n", ret);
        return ret;
    }

    ret = find_dev_and_part(partition, &dev, &pnum, &part);
    if (ret)
    {
        fprintf(stderr, "Failed to find part %s:%d\n", partition, ret);
        return ret;
    }

    *offset = part->offset;
    *size   = part->size;

    return 0;
}
#endif

static struct fwfs_mnt_handle g_mnt_handle;

#if defined(CONFIG_SSTAR_NAND)
static void block_device_bbt_map_build(struct fwfs_config *c)
{
    int           flag;
    fwfs_block_t  bad_num = 0;
    fwfs_block_t *maps;
    fwfs_block_t  j = 0;

    maps = (fwfs_block_t *)calloc(sizeof(fwfs_block_t), c->block_count);
    if (maps == NULL)
        return;

    for (fwfs_block_t i = 0; i < c->block_count; i++)
    {
        flag = c->bbt(c, i);
        if (flag == 0)
        {
            maps[j++] = i;
        }
        else
        {
            printf("%s: block 0x%" PRIx32 " is a factory bad block (0x%x)\n", __FUNCTION__, i, flag);
            bad_num++;
        }
    }

    for (; j < c->block_count; j++)
    {
        maps[j] = BBT_MAP_INVALID;
    }

#if 0
    printf("BBT map table begin:\n");
    for (j = 0; j < c->block_count; j++)
    {
        if (j != 0 && (j % 8) == 0)
            printf("\n");
        printf("0x%08"PRIx32",0x%08"PRIx32";",
                    j, maps[j]);
    }
    printf("\nBBT map table end:\n");
#endif

    if (bad_num == 0)
    {
        free(maps);
        return;
    }
    c->context = (void *)maps;
}

static fwfs_block_t block_device_bbt_map(const struct fwfs_config *c, fwfs_block_t block)
{
    fwfs_block_t  nblock = block;
    fwfs_block_t *maps   = (fwfs_block_t *)c->context;

    if (maps == NULL)
        return block;

    nblock = maps[nblock];
    return nblock;
}
#endif

// block device operations
static int block_device_read(const struct fwfs_config *c, fwfs_block_t block, fwfs_off_t off, void *buffer,
                             fwfs_size_t size)
{
#if defined(CONFIG_FIRMWAREFS_ON_EMMC)
    lbaint_t                          lba_addr;
    lbaint_t                          blk_cnt;
    uint32_t                          ret;
    struct blk_desc *                 dev_desc;
    struct disk_partition *           info;
    struct fwfs_dev_and_part_context *context;

    context  = (struct fwfs_dev_and_part_context *)g_mnt_handle.cfg.context;
    dev_desc = context->dev_desc;
    info     = &context->info;

    lba_addr = (c->block_size * block + off) / info->blksz + info->start;
    blk_cnt  = size / info->blksz;

    // ret = dev_desc->block_read(dev_desc->dev, lba_addr, blk_cnt, buffer);
    ret = blk_dread(dev_desc, lba_addr, blk_cnt, buffer);
    if (ret != blk_cnt)
        return -EIO;

    return 0;
#else
    uint32_t         u32Addr;
#if defined(CONFIG_SSTAR_NAND)
    struct mtd_info *nand;
    size_t           retlen;

    block = block_device_bbt_map(c, block);
    if (block == BBT_MAP_INVALID)
        return -EIO;
    u32Addr = c->block_size * (block + c->block_offset) + off;

    nand = get_nand_dev_by_index(0);
    return mtd_read(nand, u32Addr, size, &retlen, buffer);
#else
    u32Addr = c->block_size * (block + c->block_offset) + off;
    return spi_flash_read(spinor_flash, u32Addr, size, buffer);
#endif
#endif
}

static int block_device_prog(const struct fwfs_config *c, fwfs_block_t block, fwfs_off_t off, const void *buffer,
                             fwfs_size_t size)
{
#if defined(CONFIG_FIRMWAREFS_ON_EMMC)
    lbaint_t                          lba_addr;
    lbaint_t                          blk_cnt;
    uint32_t                          ret;
    struct blk_desc *                 dev_desc;
    struct disk_partition *           info;
    struct fwfs_dev_and_part_context *context;

    context  = (struct fwfs_dev_and_part_context *)g_mnt_handle.cfg.context;
    dev_desc = context->dev_desc;
    info     = &context->info;

    lba_addr = (c->block_size * block + off) / info->blksz + info->start;
    blk_cnt  = size / info->blksz;

    // ret = dev_desc->block_write(dev_desc->dev, lba_addr, blk_cnt, buffer);
    ret = blk_dwrite(dev_desc, lba_addr, blk_cnt, buffer);
    if (ret != blk_cnt)
        return -EIO;

    return 0;
#else
    u32              u32Addr;
#if defined(CONFIG_SSTAR_NAND)
    struct mtd_info *nand;
    size_t           retlen;

    block = block_device_bbt_map(c, block);
    if (block == BBT_MAP_INVALID)
        return -EIO;
    u32Addr = c->block_size * (block + c->block_offset) + off;

    nand = get_nand_dev_by_index(0);
    return mtd_write(nand, u32Addr, size, &retlen, buffer);
#else
    u32Addr = c->block_size * (block + c->block_offset) + off;
    return spi_flash_write(spinor_flash, u32Addr, size, buffer);
#endif
#endif
}

static int block_device_erase(const struct fwfs_config *c, fwfs_block_t block)
{
#if defined(CONFIG_FIRMWAREFS_ON_EMMC)
    return 0;
#else
    u32               u32Addr;
#if defined(CONFIG_SSTAR_NAND)
    struct erase_info instr;
    struct mtd_info * nand;

    block = block_device_bbt_map(c, block);
    if (block == BBT_MAP_INVALID)
        return -EIO;
    u32Addr = c->block_size * (block + c->block_offset);

    nand           = get_nand_dev_by_index(0);
    instr.mtd      = nand;
    instr.addr     = u32Addr;
    instr.len      = c->block_size;
    instr.callback = 0;
    return mtd_erase(nand, &instr);
#else
    u32Addr = c->block_size * (block + c->block_offset);
    return spi_flash_erase(spinor_flash, u32Addr, c->block_size);
#endif
#endif
}

static int block_device_sync(const struct fwfs_config *c)
{
    // CamOsPrintf("### block_device_sync\n");
    return 0;
}

static int block_device_bbt(const struct fwfs_config *c, fwfs_block_t block)
{
#if defined(CONFIG_SSTAR_NAND)
    static struct mtd_info *nand = NULL;
    u32 u32Addr;

    if (!nand)
    {
        nand = get_nand_dev_by_index(0);
    }

    if (!nand)
    {
        return 0;
    }

    u32Addr = c->block_size * (block + c->block_offset);
    return mtd_block_isbad(nand, u32Addr); // return value 0:good block; 1:bad block
#else
    return 0;
#endif
}

static int block_device_bypass(const struct fwfs_config *c, uint8_t *buffer)
{
    return CACHE_LINE_ALIGNED(buffer);
}

#if defined(CONFIG_FIRMWAREFS_ON_EMMC)
struct fwfs_dev_and_part_context *firmwarefs_find_partition(char *partition)
{
    int                               part;
    char                              dev_part_str[64];
    int                               i;
    bool                              found = false;
    struct fwfs_dev_and_part_context *context;

    context = (struct fwfs_dev_and_part_context *)calloc(sizeof(struct fwfs_dev_and_part_context), 1);
    if (context == NULL)
        return NULL;

    for (i = 1; i <= 20; i++)
    {
        snprintf(dev_part_str, sizeof(dev_part_str), "0:%d", i);
        part = blk_get_device_part_str("mmc", dev_part_str, &context->dev_desc, &context->info, 1);
        if (part < 0)
            break;

        if ((strcmp((char *)context->info.name, partition) == 0)
            || (strcmp(partition, "MISC") == 0 && strcmp((char *)context->info.name, "misca") == 0))
        {
            found = true;
            break;
        }
    }

    if (!found)
    {
        free(context);
        context = NULL;
    }
    return context;
}
#endif

int32_t firmwarefs_mount(char *partition, char *mnt_path)
{
#if defined(CONFIG_FIRMWAREFS_ON_EMMC)
    uint32_t                          FIRMWAREFS_CACHE_SIZE      = 2048;
    uint32_t                          FIRMWAREFS_FILE_CACHE_SIZE = 8192;
    uint32_t                          FIRMWAREFS_READ_SIZE       = 2048;
    uint32_t                          FIRMWAREFS_PROG_SIZE       = 2048;
    uint32_t                          FIRMWAREFS_BLK_SIZE        = 131072;
    uint32_t                          FIRMWAREFS_SUBBLK_SIZE     = 32768;
    uint32_t                          FIRMWAREFS_CACHE_POOL_SIZE = 8;
    struct fwfs_dev_and_part_context *context;
#else
#if defined(CONFIG_SSTAR_NAND)
    uint32_t         FIRMWAREFS_CACHE_SIZE      = 2048;
    uint32_t         FIRMWAREFS_FILE_CACHE_SIZE = 8192;
    uint32_t         FIRMWAREFS_READ_SIZE       = 2048;
    uint32_t         FIRMWAREFS_PROG_SIZE       = 2048;
    uint32_t         FIRMWAREFS_BLK_SIZE        = 131072;
    uint32_t         FIRMWAREFS_SUBBLK_SIZE     = 32768;
    uint32_t         FIRMWAREFS_CACHE_POOL_SIZE = 8;
    struct mtd_info *mtd;
#else
    uint32_t FIRMWAREFS_CACHE_SIZE      = 2048;
    uint32_t FIRMWAREFS_FILE_CACHE_SIZE = 2048;
    uint32_t FIRMWAREFS_READ_SIZE       = 2048;
    uint32_t FIRMWAREFS_PROG_SIZE       = 2048;
    uint32_t FIRMWAREFS_BLK_SIZE        = 65536;
    uint32_t FIRMWAREFS_SUBBLK_SIZE     = 32768;
    uint32_t FIRMWAREFS_CACHE_POOL_SIZE = 4;
#endif
    u64              part_offset;
    u64              part_size;
#endif
    int err = 0;

    memset(&g_mnt_handle.cfg.fwfs_partition, 0x0, sizeof(g_mnt_handle.cfg.fwfs_partition));
#if defined(CONFIG_FIRMWAREFS_ON_EMMC)
    context = firmwarefs_find_partition(partition);
    if (context == NULL)
        return -1;
    g_mnt_handle.cfg.context = context;
#else
#if defined(CONFIG_SSTAR_NAND)
    mtd = get_mtd_device(NULL, 0);
    if (mtd)
    {
        FIRMWAREFS_BLK_SIZE        = mtd->erasesize;
        FIRMWAREFS_CACHE_SIZE      = mtd->writesize;
        FIRMWAREFS_FILE_CACHE_SIZE = mtd->writesize * 4;
        FIRMWAREFS_READ_SIZE       = mtd->writesize;
        FIRMWAREFS_PROG_SIZE       = mtd->writesize;

        put_mtd_device(mtd);
        mtd = NULL;
    }
#endif

#if !defined(CONFIG_SSTAR_NAND)
    if (spinor_flash == NULL)
    {
        spinor_flash = spi_flash_probe(0, 0, 1000000, SPI_MODE_3);
        if (!spinor_flash)
        {
            return -ENODEV;
        }
    }
#endif

    g_mnt_handle.cfg.fwfs_partition.u8_UseExternBlockSize = 0;
    if (get_mtdpart_range(partition, &part_offset, &part_size))
    {
        printf("get_mtdpart_range return error\n");
        return -1;
    }
#endif

    // block device operations
    g_mnt_handle.cfg.read   = block_device_read;
    g_mnt_handle.cfg.prog   = block_device_prog;
    g_mnt_handle.cfg.erase  = block_device_erase;
    g_mnt_handle.cfg.sync   = block_device_sync;
    g_mnt_handle.cfg.bbt    = block_device_bbt;
    g_mnt_handle.cfg.bypass = block_device_bypass;

    // block device configuration
#if defined(CONFIG_FIRMWAREFS_ON_EMMC)
    g_mnt_handle.cfg.page_size    = FIRMWAREFS_PROG_SIZE;
    g_mnt_handle.cfg.block_size   = FIRMWAREFS_BLK_SIZE;
    g_mnt_handle.cfg.block_count  = (context->info.size * context->info.blksz) / g_mnt_handle.cfg.block_size;
    g_mnt_handle.cfg.block_offset = 0;
#else
    // spinad need set flash page size, value is 2048, spinor not limit
    g_mnt_handle.cfg.page_size = FIRMWAREFS_PROG_SIZE;
    // block size is a multiple of cache size
    g_mnt_handle.cfg.block_size   = FIRMWAREFS_BLK_SIZE;
    g_mnt_handle.cfg.block_count  = ((u32)part_size) / g_mnt_handle.cfg.block_size;
    g_mnt_handle.cfg.block_offset = ((u32)part_offset) / g_mnt_handle.cfg.block_size;
#endif
    g_mnt_handle.cfg.subblock_size = FIRMWAREFS_SUBBLK_SIZE;

    g_mnt_handle.cfg.read_size       = FIRMWAREFS_READ_SIZE;
    g_mnt_handle.cfg.cache_pool_size = FIRMWAREFS_CACHE_POOL_SIZE;
    // cache size is a multiple of read sizes
    g_mnt_handle.cfg.cache_size = FIRMWAREFS_CACHE_SIZE;
    g_mnt_handle.cfg.prog_size  = FIRMWAREFS_PROG_SIZE;
    // Must be a multiple of the read sizes
    g_mnt_handle.cfg.file_cache_size = FIRMWAREFS_FILE_CACHE_SIZE;
    g_mnt_handle.cfg.block_cycles    = 500;
    g_mnt_handle.cfg.lookahead_size  = 8;

#if defined(CONFIG_SSTAR_NAND)
    block_device_bbt_map_build(&g_mnt_handle.cfg);
#endif

    // mount the filesystem
    err = fwfs_mount(&g_mnt_handle.fwfs, &g_mnt_handle.cfg);

    return err;
}

void firmwarefs_unmount(void)
{
    fwfs_unmount(&g_mnt_handle.fwfs);
#if (defined(CONFIG_SSTAR_NAND) || defined(CONFIG_FIRMWAREFS_ON_EMMC))
    if (g_mnt_handle.cfg.context != NULL)
    {
        free(g_mnt_handle.cfg.context);
        g_mnt_handle.cfg.context = NULL;
    }
#else
    if (spinor_flash != NULL)
    {
        spi_flash_free(spinor_flash);
        spinor_flash = NULL;
    }
#endif
}

void *firmwarefs_open(char *filename, uint32_t flags, uint32_t mode)
{
    int                      err;
    int                      nflags      = 0x0;
    void *                   ret         = NULL;
    struct fwfs_file_handle *file_handle = NULL;

    file_handle = (struct fwfs_file_handle *)calloc(sizeof(struct fwfs_file_handle), 1);
    if (!file_handle)
    {
        printf("%s: alloc file_handle fail\n", __FUNCTION__);
        goto firmwarefs_open_end;
    }

    if (flags & O_RDONLY)
        nflags |= FWFS_O_RDONLY;

    if (flags & O_WRONLY)
        nflags |= FWFS_O_WRONLY;

    if (flags & O_RDWR)
        nflags |= FWFS_O_RDWR;

    if (flags & O_CREAT)
        nflags |= FWFS_O_CREAT;

    if (flags & O_APPEND)
        nflags |= FWFS_O_APPEND;

    if (flags & O_TRUNC)
        nflags |= FWFS_O_TRUNC;

    if (nflags == 0x0)
        nflags = FWFS_O_RDONLY;

    err = fwfs_file_open(&g_mnt_handle.fwfs, &file_handle->fwfs_fd, filename, nflags);
    if (err)
    {
        printf("%s: open %s fail(%d)\n", __FUNCTION__, filename, err);
        free((void *)file_handle);
        goto firmwarefs_open_end;
    }

    file_handle->fwfs_file = &g_mnt_handle.fwfs;
    ret                    = file_handle;

firmwarefs_open_end:
    return ret;
}

int32_t firmwarefs_close(void *fd)
{
    int                      ret         = 0;
    struct fwfs_file_handle *file_handle = (struct fwfs_file_handle *)fd;
    if (fd)
    {
        ret = fwfs_file_close(file_handle->fwfs_file, &file_handle->fwfs_fd);
        if (ret)
        {
            printf("%s: close fail(%d)\n", __FUNCTION__, ret);
        }
        free(fd);
        fd = NULL;
    }
    return ret;
}

int32_t firmwarefs_read(void *fd, void *buf, uint32_t count)
{
    struct fwfs_file_handle *file_handle = (struct fwfs_file_handle *)fd;

    return fwfs_file_read(file_handle->fwfs_file, &file_handle->fwfs_fd, buf, count);
}

int32_t firmwarefs_write(void *fd, void *buf, uint32_t count)
{
    struct fwfs_file_handle *file_handle = (struct fwfs_file_handle *)fd;

    return fwfs_file_write(file_handle->fwfs_file, &file_handle->fwfs_fd, buf, count);
}

int32_t firmwarefs_lseek(void *fd, int32_t offset, int32_t whence)
{
    struct fwfs_file_handle *file_handle = (struct fwfs_file_handle *)fd;
    uint32_t                 ret;

    switch (whence)
    {
        case SEEK_SET:
            ret = fwfs_file_seek(file_handle->fwfs_file, &file_handle->fwfs_fd, offset, FWFS_SEEK_SET);
            break;
        case SEEK_CUR:
            ret = fwfs_file_seek(file_handle->fwfs_file, &file_handle->fwfs_fd, offset, FWFS_SEEK_CUR);
        case SEEK_END:
            ret = fwfs_file_size(file_handle->fwfs_file, &file_handle->fwfs_fd);
            break;
        default:
            ret = 0;
            break;
    }

    return ret;
}
#endif
