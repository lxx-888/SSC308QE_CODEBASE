/*
 * sstar_rpmb.c - Sigmastar
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
#include <memalign.h>
#include <mmc.h>
#include <part.h>
#include <blk.h>
#include <malloc.h>
#include <memalign.h>
#include <compiler.h>
#include <u-boot/sha256.h>
#include <sstar_rpmb.h>
#include <log.h>
#include <fs.h>

/* Global variable */
static bool        has_init                    = false;
static struct mmc *curr_mmc                    = NULL;
static int         curr_device                 = -1;
static uint64_t    rpmb_layout[RPMB_FIELD_MAX] = {
    [RPMB_FIELD_FASTBOOT_LOCK_FLAGS] = DIV_ROUND_UP(sizeof(uint64_t), RPMB_SZ_DATA),
    [RPMB_FIELD_ROLLBACK_INDEX]      = DIV_ROUND_UP(sizeof(uint64_t) * ROLLBACK_INDEX_LOCATION_COUNT, RPMB_SZ_DATA),
};

static uint64_t get_rpmb_field_start_block_offset(enum sstar_rpmb_fields field)
{
    uint64_t offset = 0;
    for (int i = 0; i < field; i++)
    {
        offset += rpmb_layout[i];
    }
    return offset;
}

static struct mmc *init_mmc_device(int dev, bool force_init, enum bus_mode speed_mode)
{
    struct mmc *mmc;
    mmc = find_mmc_device(dev);
    if (!mmc)
    {
        log_err("no mmc device at slot %x\n", dev);
        return NULL;
    }

    if (!mmc_getcd(mmc))
        force_init = true;

    if (force_init)
        mmc->has_init = 0;

    if (IS_ENABLED(CONFIG_MMC_SPEED_MODE_SET))
        mmc->user_speed_mode = speed_mode;

    if (mmc_init(mmc))
        return NULL;

#ifdef CONFIG_BLOCK_CACHE
    struct blk_desc *bd = mmc_get_blk_desc(mmc);
    blkcache_invalidate(bd->if_type, bd->devnum);
#endif

    return mmc;
}

static int get_mmc_device(void)
{
    struct mmc *mmc;
    if (get_mmc_num() <= 0)
    {
        log_err("No MMC device available\n");
        return -ENODEV;
    }
    curr_device = 0;

    mmc = init_mmc_device(curr_device, false, MMC_MODES_END);
    if (!mmc)
        return -ENODEV;

    if (!(mmc->version & MMC_VERSION_MMC))
    {
        log_err("It is not an eMMC device\n");
        return -ENODEV;
    }
    if (mmc->version < MMC_VERSION_4_41)
    {
        log_err("RPMB not supported before version 4.41\n");
        return -ENODEV;
    }
    curr_mmc = mmc;
    has_init = true;

    return 0;
}

static int rpmb_read_field(enum sstar_rpmb_fields field, void **field_context)
{
    uint64_t block_offset  = get_rpmb_field_start_block_offset(field);
    uint64_t block_cnt     = rpmb_layout[field];
    uint8_t  original_part = -1;
    void *   buf           = NULL;
    int      ret           = 0;
    int      n             = 0;

    if (!has_init && get_mmc_device())
    {
        log_err("Fail to get emmc device\n");
        ret = -ENODEV;
        goto out;
    }

    buf = malloc(block_cnt * RPMB_SZ_DATA);
    if (!buf)
    {
        log_err("Fail to malloc %#llx\n", block_cnt * RPMB_SZ_DATA);
        ret = -ENOMEM;
        goto out;
    }

    /* save current hw part then swith to part RPMB */
    original_part = get_mmc_hwpart(curr_mmc);
    if (blk_select_hwpart_devnum(IF_TYPE_MMC, curr_device, MMC_PART_RPMB) != 0)
    {
        log_err("Fail to switch hw part %d\n", MMC_PART_RPMB);
        ret = -EIO;
        goto free_out;
    }

    log_debug("Read rpmb: dev # %d, block # %lld, count %lld ...\n", curr_device, block_offset, block_cnt);
    n = mmc_rpmb_read(curr_mmc, buf, block_offset, block_cnt, &_binary_drivers_sstar_rpmb_rpmb_key_start);

    if (n != block_cnt)
    {
        log_err("Fail to read %lld(actual %d)\n", block_cnt, n);
        *field_context = NULL;
        ret            = -EIO;
    }
    else
    {
        *field_context = buf;
    }

    /* restore saved hw part */
    if (blk_select_hwpart_devnum(IF_TYPE_MMC, curr_device, original_part) != 0)
    {
        log_err("Fail to switch hw part %d\n", original_part);
        ret = -EIO;
        goto free_out;
    }

    return ret;

free_out:
    free(buf);
out:
    return ret;
}

static int rpmb_write_field(enum sstar_rpmb_fields field, void *field_context)
{
    uint64_t block_offset  = get_rpmb_field_start_block_offset(field);
    uint64_t block_cnt     = rpmb_layout[field];
    uint8_t  original_part = -1;
    int      ret           = 0;
    int      n             = 0;

    if (!has_init && get_mmc_device())
    {
        log_err("Fail to get emmc device\n");
        ret = -ENODEV;
        goto out;
    }

    /* save current hw part then swith to part RPMB */
    original_part = get_mmc_hwpart(curr_mmc);
    if (blk_select_hwpart_devnum(IF_TYPE_MMC, curr_device, MMC_PART_RPMB) != 0)
    {
        log_err("Fail to switch hw part %d\n", MMC_PART_RPMB);
        ret = -EIO;
        goto out;
    }

    log_debug("Write rpmb: dev # %d, block # %lld, count %lld ...\n", curr_device, block_offset, block_cnt);
    n = mmc_rpmb_write(curr_mmc, field_context, block_offset, block_cnt, &_binary_drivers_sstar_rpmb_rpmb_key_start);
    if (n != block_cnt)
    {
        log_err("Fail to write %lld(actual %d)\n", block_cnt, n);
        ret = -EIO;
    }

    /* restore saved hw part */
    if (blk_select_hwpart_devnum(IF_TYPE_MMC, curr_device, original_part) != 0)
    {
        log_err("Fail to switch hw part %d\n", original_part);
        ret = -EIO;
    }

out:
    return ret;
}

int sstar_rpmb_set_key(void)
{
    uint8_t original_part = -1;
    int     ret           = 0;

    if (!has_init && get_mmc_device())
    {
        log_err("Fail to get emmc device\n");
        ret = -ENODEV;
        goto out;
    }

    /* save current hw part then swith to part RPMB */
    original_part = get_mmc_hwpart(curr_mmc);
    if (blk_select_hwpart_devnum(IF_TYPE_MMC, curr_device, MMC_PART_RPMB) != 0)
    {
        log_err("Fail to switch hw part %d\n", MMC_PART_RPMB);
        ret = -EIO;
        goto out;
    }

    ret = mmc_rpmb_set_key(curr_mmc, &_binary_drivers_sstar_rpmb_rpmb_key_start);

    /* restore saved hw part */
    if (blk_select_hwpart_devnum(IF_TYPE_MMC, curr_device, original_part) != 0)
    {
        log_err("Fail to switch hw part %d\n", original_part);
        ret = -EIO;
    }

out:
    return ret;
}

int sstar_rpmb_get_lock_flags(uint64_t *out_flags)
{
    void *filed_context = NULL;
    int   ret;

    ret = rpmb_read_field(RPMB_FIELD_FASTBOOT_LOCK_FLAGS, &filed_context);
    if (ret)
    {
        log_err("Fail to read fastboot flags field:%d\n", ret);
        return ret;
    }
    *out_flags = *((uint64_t *)filed_context);

    free(filed_context);
    return ret;
}

int sstar_rpmb_set_lock_flags(uint64_t flags)
{
    void *filed_context = NULL;
    int   ret;

    /* read field frist of all, becus the min size of read/write action is one data block(256 byte) size */
    ret = rpmb_read_field(RPMB_FIELD_FASTBOOT_LOCK_FLAGS, &filed_context);
    if (ret)
    {
        log_err("Fail to read fastboot flags field:%d\n", ret);
        return ret;
    }

    *((uint64_t *)filed_context) = flags;

    /* writeback full data block */
    ret = rpmb_write_field(RPMB_FIELD_FASTBOOT_LOCK_FLAGS, filed_context);
    if (ret)
    {
        log_err("Fail to write fastboot flags field:%d\n", ret);
        return ret;
    }

    free(filed_context);
    return ret;
}

int sstar_rpmb_get_rollback_index(uint8_t location, uint64_t *out_rollback_index)
{
    void *filed_context = NULL;
    int   ret;

    if (location > ROLLBACK_INDEX_LOCATION_COUNT)
    {
        log_err("Invaild rollback index location %d\n", location);
        *out_rollback_index = -1ULL;
        ret                 = -EINVAL;
        goto out;
    }

    ret = rpmb_read_field(RPMB_FIELD_ROLLBACK_INDEX, &filed_context);
    if (ret)
    {
        log_err("Fail to read rollback index field:%d\n", ret);
        *out_rollback_index = -1ULL;
        goto out;
    }

    *out_rollback_index = *((uint64_t *)filed_context + location);

    free(filed_context);
out:
    return ret;
}

int sstar_rpmb_set_rollback_index(uint8_t location, uint64_t rollback_index)
{
    void *filed_context = NULL;
    int   ret;

    if (location > ROLLBACK_INDEX_LOCATION_COUNT)
    {
        log_err("Invaild rollback index location %d\n", location);
        ret = -EINVAL;
        goto out;
    }

    /* read field frist of all, becus the min size of read/write action is one data block(256 byte) size */
    ret = rpmb_read_field(RPMB_FIELD_ROLLBACK_INDEX, &filed_context);
    if (ret)
    {
        log_err("Fail to read rollback index field:%d\n", ret);
        goto out;
    }

    if (*((uint64_t *)filed_context + location) > rollback_index)
    {
        log_warning("!!! Warning: index:%d val:%lld, writing new rollback index val:%lld\n", location,
                    *((uint64_t *)filed_context + location), rollback_index);
    }

    *((uint64_t *)filed_context + location) = rollback_index;

    /* writeback full data block */
    ret = rpmb_write_field(RPMB_FIELD_ROLLBACK_INDEX, filed_context);
    if (ret)
    {
        log_err("Fail to write rollback index field:%d\n", ret);
    }

    free(filed_context);
out:
    return ret;
}
