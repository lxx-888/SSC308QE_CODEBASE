/*
 * mtd_spinor.c- Sigmastar
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
#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/interrupt.h>
#include <linux/mutex.h>
#include <linux/version.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/partitions.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/clk.h>
#include <linux/clk-provider.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include <ms_platform.h>
#include <ms_msys.h>
#include <cam_sysfs.h>
#include <drv_flash_os_impl.h>
#include <mdrv_spinor.h>
#include <cis.h>

#define FLASH_DBG 0

#define BLOCK_ERASE_SIZE 0x10000

#if FLASH_DBG
#define spi_nor_msg(fmt, ...) printk(KERN_NOTICE "%s: " fmt "\n", __func__, ##__VA_ARGS__)
#define spi_nor_debug(fmt, ...)
#else
#define spi_nor_msg(fmt, ...)   printk(KERN_NOTICE "%s: " fmt "\n", __func__, ##__VA_ARGS__)
#define spi_nor_debug(fmt, ...) printk(KERN_NOTICE "%s: " fmt "\n", __func__, ##__VA_ARGS__)
#endif

struct sstar_spinor_chip
{
    struct mutex         lock;
    struct mtd_info      mtd;
    struct spinor_handle handle;
    u8 *                 cis_mapping_buffer;
    u8 *                 bdma_buf;
    u8                   suspended;
};

static void sstar_spinor_release_device(struct sstar_spinor_chip *spinor_chip)
{
    mutex_unlock(&spinor_chip->lock);
}

static int sstar_spinor_get_device(struct sstar_spinor_chip *spinor_chip)
{
    mutex_lock(&spinor_chip->lock);
    if (spinor_chip->suspended)
    {
        mutex_unlock(&spinor_chip->lock);
        return -EBUSY;
    }

    return 0;
}

static void sstar_spinor_reboot(struct mtd_info *mtd)
{
    struct sstar_spinor_chip *spinor_chip = (struct sstar_spinor_chip *)mtd->priv;

    mutex_lock(&spinor_chip->lock);
    spinor_chip->suspended = 1;
    mutex_unlock(&spinor_chip->lock);
}

/* Erase flash fully or part of it */
static int sstar_spinor_erase(struct mtd_info *mtd, struct erase_info *instr)
{
    struct sstar_spinor_chip *spinor_chip = (struct sstar_spinor_chip *)mtd->priv;
    uint64_t                  addr_temp, len_temp;
    u8                        status       = 0;
    u32                       bytes_offset = 0;
    u32                       size         = 0;
    int                       ret          = 0;

    /* sanity checks */
    if (!instr->len)
    {
        return 0;
    }

    /* range and alignment check */
    if (instr->addr + instr->len > mtd->size)
    {
        return -EINVAL;
    }

    addr_temp = instr->addr;
    len_temp  = instr->len;
    if ((do_div(addr_temp, mtd->erasesize) != 0) || (do_div(len_temp, mtd->erasesize) != 0))
    {
        return -EINVAL;
    }

    ret = sstar_spinor_get_device(spinor_chip);
    if (ret)
        return ret;

    bytes_offset = instr->addr;
    size         = instr->len;

    status = mdrv_spinor_erase(&spinor_chip->handle, bytes_offset, size);

    if (ERR_SPINOR_DEVICE_FAILURE == status)
    {
        spi_nor_msg(KERN_ERR "[FLASH_ERR] erase fail\r\n");
        sstar_spinor_release_device(spinor_chip);
        return -EIO;
    }
    else if (ERR_SPINOR_INVALID == status)
    {
        sstar_spinor_release_device(spinor_chip);
        return -EINVAL;
    }

    sstar_spinor_release_device(spinor_chip);
    return 0;
}

/*
 * Read an address range from the flash chip.  The address range
 * may be any size provided it is within the physical boundaries.
 */
static int sstar_spinor_read(struct mtd_info *mtd, loff_t from, size_t len, size_t *retlen, u_char *buf)
{
    struct sstar_spinor_chip *spinor_chip = (struct sstar_spinor_chip *)mtd->priv;
    u8                        status      = 0;
    u8                        aligne_size = 0;
    u32                       aligne_addr = 0;
    u32                       read_size;
    int                       ret = 0;

    /* sanity checks */
    if (!len)
    {
        return 0;
    }
    if (from + len > spinor_chip->mtd.size)
    {
        return -EINVAL;
    }

    ret = sstar_spinor_get_device(spinor_chip);
    if (ret)
        return ret;

    *retlen = len;

#if !defined(CONFIG_FLASH_FIX_BDMA_ALIGN)
    aligne_size = from % CONFIG_FLASH_ADDR_ALIGN;
#endif

    if (aligne_size)
    {
        aligne_addr = from - aligne_size;
        len += aligne_size;
    }
    else
        aligne_addr = from;

    while (0 != len)
    {
        read_size = BLOCK_ERASE_SIZE;
        if (len < read_size)
            read_size = len;
        if (ERR_SPINOR_SUCCESS
            != (status = mdrv_spinor_read(&spinor_chip->handle, aligne_addr, spinor_chip->bdma_buf, read_size)))
        {
            *retlen = 0;
            sstar_spinor_release_device(spinor_chip);
            return (-EIO);
        }

        memcpy((void *)buf, (const void *)(spinor_chip->bdma_buf + aligne_size), (read_size - aligne_size));

        len -= read_size;
        buf = buf + read_size - aligne_size;
        aligne_addr += read_size;
        aligne_size = 0;
    }

    sstar_spinor_release_device(spinor_chip);
    return 0;
}

/*
 * Write an address range to the flash chip.  Data must be written in
 * FLASH_PAGESIZE chunks.  The address range may be any size provided
 * it is within the physical boundaries.
 */
static int sstar_spinor_write(struct mtd_info *mtd, loff_t to, size_t len, size_t *retlen, const u_char *buf)
{
    struct sstar_spinor_chip *spinor_chip = (struct sstar_spinor_chip *)mtd->priv;
    u32                       write_size  = 0;
    int                       ret         = 0;

    if (retlen)
        *retlen = 0;

    /* sanity checks */
    if (!len)
    {
        return (0);
    }
    if (to + len > spinor_chip->mtd.size)
    {
        return -EINVAL;
    }

    ret = sstar_spinor_get_device(spinor_chip);
    if (ret)
        return ret;

    *retlen = len; // if success,return the input length

    while (0 != len)
    {
        write_size = BLOCK_ERASE_SIZE;
        if (len < write_size)
            write_size = len;

        memcpy((void *)spinor_chip->bdma_buf, (const void *)(buf), write_size);
        if (ERR_SPINOR_SUCCESS != mdrv_spinor_program(&spinor_chip->handle, to, spinor_chip->bdma_buf, write_size))
        {
            *retlen = 0;
            sstar_spinor_release_device(spinor_chip);
            return (-EIO);
        }

        len -= write_size;
        buf += write_size;
        to += write_size;
    }

    sstar_spinor_release_device(spinor_chip);
    return 0;
}

static int sstar_spinor_probe(struct platform_device *pdev)
{
    u8 *                      name        = NULL;
    u32                       engine      = 0;
    u32                       cs_select   = 0;
    struct sstar_spinor_chip *spinor_chip = NULL;
    flash_nor_info_t          st_flash_nor_info;

    do
    {
        if (CamofPropertyReadU32(pdev->dev.of_node, "cs-select", &cs_select))
        {
            break;
        }

        if (CamofPropertyReadU32(pdev->dev.of_node, "engine", &engine))
        {
            break;
        }

        if (NULL == (spinor_chip = (struct sstar_spinor_chip *)kzalloc(sizeof(struct sstar_spinor_chip), GFP_KERNEL)))
        {
            break;
        }

        if (NULL == (spinor_chip->handle.nri = (spinor_nri_t *)kzalloc(CIS_PAGE_SIZE, GFP_KERNEL)))
        {
            break;
        }

        if (NULL == (spinor_chip->cis_mapping_buffer = (u8 *)kzalloc(CIS_PAGE_SIZE * CIS_MAPPING_CNT, GFP_KERNEL)))
        {
            break;
        }

        if (NULL == (name = (u8 *)kzalloc(8, GFP_KERNEL)))
        {
            break;
        }

        spinor_chip->handle.msg.cs_select = cs_select;
        spinor_chip->handle.ctrl_id       = spiflash_get_master(engine);

        mdrv_spinor_setup_by_default(&spinor_chip->handle);

        if (ERR_SPINOR_SUCCESS != mdrv_spinor_reset(&spinor_chip->handle))
        {
            printk("[FLASH] reset fail\r\n");
            break;
        }

        sstar_cis_init(spinor_chip->cis_mapping_buffer, CIS_MAPPING_CNT, NULL, 0, 1);

        if (!sstar_cis_get_nri(&spinor_chip->handle))
        {
            printk("[FLASH] No sni!\r\n");
            break;
        }

        if (mdrv_spinor_hardware_init(&spinor_chip->handle))
        {
            spi_nor_msg("[sstar_spinor_probe] flash init failed!\n");
            break;
        }

        mdrv_spinor_info(&spinor_chip->handle, &st_flash_nor_info);

        if (NULL == (spinor_chip->bdma_buf = (u8 *)kzalloc(BLOCK_ERASE_SIZE, GFP_KERNEL)))
        {
            break;
        }

        mutex_init(&spinor_chip->lock);

        strcpy((char *)name, "nor0");
        name[3]                       = '0' + (engine << 1) + cs_select;
        spinor_chip->mtd.name         = name;
        spinor_chip->mtd.priv         = spinor_chip;
        spinor_chip->mtd.type         = MTD_NORFLASH;
        spinor_chip->mtd.writesize    = 1;
        spinor_chip->mtd.writebufsize = spinor_chip->mtd.writesize;
        spinor_chip->mtd.flags        = MTD_CAP_NORFLASH;
        spinor_chip->mtd.size         = st_flash_nor_info.capacity;
        spinor_chip->mtd._erase       = sstar_spinor_erase;
        spinor_chip->mtd._read        = sstar_spinor_read;
        spinor_chip->mtd._write       = sstar_spinor_write;
        spinor_chip->mtd._reboot      = sstar_spinor_reboot;
        spinor_chip->mtd.erasesize    = st_flash_nor_info.sector_size;
        spi_nor_msg(KERN_DEBUG
                    "mtd .name = %s, .size = 0x%.8x (%uMiB)\n"
                    " .erasesize = 0x%.8x .numeraseregions = %d\n",
                    spinor_chip->mtd.name, (unsigned int)spinor_chip->mtd.size,
                    (unsigned int)spinor_chip->mtd.size / (1024 * 1024), (unsigned int)spinor_chip->mtd.erasesize,
                    spinor_chip->mtd.numeraseregions);

        platform_set_drvdata(pdev, &spinor_chip->mtd);

        return mtd_device_register(&spinor_chip->mtd, NULL, 0);
    } while (0);

    if (spinor_chip && spinor_chip->cis_mapping_buffer)
    {
        kfree((void *)spinor_chip->cis_mapping_buffer);
    }

    if (spinor_chip && spinor_chip->bdma_buf)
    {
        kfree((void *)spinor_chip->bdma_buf);
    }

    if (spinor_chip && spinor_chip->handle.nri)
    {
        kfree((void *)spinor_chip->handle.nri);
    }

    if (spinor_chip)
    {
        kfree((void *)spinor_chip);
    }

    if (name)
    {
        kfree((void *)name);
    }

    return -ENOMEM;
}

static int sstar_spinor_remove(struct platform_device *pdev)
{
    struct mtd_info *         mtd;
    struct sstar_spinor_chip *spinor_chip;

    mtd         = platform_get_drvdata(pdev);
    spinor_chip = (struct sstar_spinor_chip *)mtd->priv;

    sstar_cis_deinit();

    if (spinor_chip && spinor_chip->cis_mapping_buffer)
    {
        kfree((void *)spinor_chip->cis_mapping_buffer);
    }

    if (spinor_chip && spinor_chip->bdma_buf)
    {
        kfree((void *)spinor_chip->bdma_buf);
    }

    if (spinor_chip && spinor_chip->handle.nri)
    {
        kfree((void *)spinor_chip->handle.nri);
    }

    if (spinor_chip)
    {
        kfree((void *)spinor_chip);
    }

    platform_set_drvdata(pdev, NULL);

    return 0;
}

static void sstar_spinor_shutdown(struct platform_device *pdev) {}

static int sstar_spinor_suspend(struct device *dev)
{
    return 0;
}

static int sstar_spinor_resume(struct device *dev)
{
    struct sstar_spinor_chip *spinor_chip;
    struct mtd_info *         mtd;

    mtd         = dev_get_drvdata(dev);
    spinor_chip = (struct sstar_spinor_chip *)mtd->priv;

    if (ERR_SPINOR_SUCCESS != mdrv_spinor_reset(&spinor_chip->handle))
    {
        printk("[FLASH] reset fail\r\n");
        return -EIO;
    }

    if (mdrv_spinor_hardware_init(&spinor_chip->handle))
    {
        spi_nor_msg("[sstar_spinor_resume] flash init failed!\n");
        return -EIO;
    }

    return 0;
}

static const struct of_device_id sstar_spinor_of_device_ids[] = {
    {.compatible = "sstar-norflash"},
    {},
};

static const struct dev_pm_ops sstar_norflash_pm_ops = {
    .resume  = sstar_spinor_resume,
    .suspend = sstar_spinor_suspend,
};

static struct platform_driver sstar_norflash_driver = {
    .probe    = sstar_spinor_probe,
    .remove   = sstar_spinor_remove,
    .shutdown = sstar_spinor_shutdown,
    .driver =
        {
            .name           = "sstar-norflash",
            .owner          = THIS_MODULE,
            .of_match_table = sstar_spinor_of_device_ids,
            .pm             = &sstar_norflash_pm_ops,
        },
};

module_platform_driver(sstar_norflash_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Tao.Zhou");
MODULE_DESCRIPTION("MTD Mstar driver for spi flash chips");
