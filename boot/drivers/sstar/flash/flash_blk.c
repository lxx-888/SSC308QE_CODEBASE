/*
 * flash_blk.c- Sigmastar
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
#include <dm.h>
#include <dm/device-internal.h>
#include <dm/lists.h>
#include <dm/root.h>
#include <flash_blk.h>
#include <part.h>
#include <drv_part.h>

static int get_partition_info_flash(struct blk_desc *dev_desc, int part_index, struct disk_partition *info)
{
    u8                       dev_name[8] = {0};
    struct sstar_part        part;
    struct udevice *         udev = dev_desc->bdev;
    struct sstar_flash_info *priv = dev_get_priv(udev->parent);
    struct mtd_info *        mtd  = priv->mtd;

    if (NULL == mtd)
    {
        return -EINVAL;
    }

    if (part_index < 0)
    {
        return -EINVAL;
    }

    if (priv->flash_con_type == IF_TYPE_SPINOR)
    {
        strcpy((char *)dev_name, "nor0");
        dev_name[3] = '0' + dev_desc->devnum;
        strcpy((char *)info->type, PART_TYPE_SPINOR);
    }
    else if (priv->flash_con_type == IF_TYPE_SPINAND)
    {
        strcpy((char *)dev_name, "nand0");
        dev_name[4] = '0' + dev_desc->devnum;
        strcpy((char *)info->type, PART_TYPE_SPINAND);
    }

    if (!sstar_part_get_nm_by_dev(dev_name, part_index, &part))
    {
        return -EINVAL;
    }

    info->blksz = mtd->erasesize;
    info->start = part.offset / info->blksz;
    info->size  = part.size / info->blksz;
    strncpy(info->name, part.part_name, PART_TYPE_LEN);
    info->name[PART_TYPE_LEN - 1] = 0;

    return 0;
}

static void print_part_flash(struct blk_desc *dev_desc)
{
    // printk("Not support print_part\n");
    return;
}

static int test_part_flash(struct blk_desc *dev_desc)
{
    if ((dev_desc->if_type != IF_TYPE_SPINOR) && (dev_desc->if_type != IF_TYPE_SPINAND))
    {
        return -ENODEV;
    }

    return 0;
}

ulong sstar_flash_bread(struct udevice *udev, lbaint_t start, lbaint_t blkcnt, void *dst)
{
    struct sstar_flash_info *priv = dev_get_priv(udev->parent);
    if (blkcnt == 0)
    {
        return 0;
    }

    if (!priv->read)
    {
        return -EINVAL;
    }

    return (ulong)priv->read(udev->parent, (u32)start, (u32)blkcnt, dst);
}

ulong sstar_flash_bwrite(struct udevice *udev, lbaint_t start, lbaint_t blkcnt, const void *src)
{
    struct sstar_flash_info *priv = dev_get_priv(udev->parent);

    if (blkcnt == 0)
        return 0;

    if (!priv->write)
        return -EINVAL;

    if (!priv->erase)
        return -EINVAL;

    if (blkcnt != (ulong)priv->erase(udev->parent, (u32)start, (u32)blkcnt))
        return -EINVAL;

    return (ulong)priv->write(udev->parent, (u32)start, (u32)blkcnt, src);
}

ulong sstar_flash_berase(struct udevice *udev, lbaint_t start, lbaint_t blkcnt)
{
    struct sstar_flash_info *priv = dev_get_priv(udev->parent);

    if (blkcnt == 0)
        return 0;

    if (!priv->erase)
        return -EINVAL;

    return (ulong)priv->erase(udev->parent, (u32)start, (u32)blkcnt);
}

static int sstar_flash_blk_probe(struct udevice *udev)
{
    struct blk_desc *        desc = dev_get_uclass_plat(udev);
    struct sstar_flash_info *priv = dev_get_priv(udev->parent);

    if (desc->if_type != priv->flash_con_type)
    {
        return -ENODEV;
    }

    desc->bdev      = udev;
    desc->part_type = PART_TYPE_BLK_FLASH;
    desc->lba       = (lbaint_t)priv->density;
    desc->blksz     = priv->mtd->erasesize; // per block size in flash

    part_init(desc);

    return 0;
}

static const struct blk_ops sstar_flash_blk_ops = {
    .read  = sstar_flash_bread,
    .write = sstar_flash_bwrite,
    .erase = sstar_flash_berase,
};

// blk_desc
U_BOOT_PART_TYPE(BLK_FLASH) = {
    .name        = "BLK_FLASH",
    .part_type   = PART_TYPE_BLK_FLASH,
    .max_entries = FLASH_RESERVED_FOR_MAP,
    .get_info    = part_get_info_ptr(get_partition_info_flash),
    .print       = part_print_ptr(print_part_flash),
    .test        = test_part_flash,
};

U_BOOT_DRIVER(sstar_flash_blk) = {
    .name  = "sstar_flash_blk",
    .id    = UCLASS_BLK,
    .ops   = &sstar_flash_blk_ops,
    .probe = sstar_flash_blk_probe,
};