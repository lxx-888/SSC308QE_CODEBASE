/*
 * mtd_spinor.c- Sigmastar
 *
 * Copyright (c) [2019~2022] SigmaStar Technology.
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
#include <flash.h>
#include <malloc.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/concat.h>
#include <spi_flash.h>
#include <linux/err.h>
#include "asm/arch/mach/platform.h"
#include <asm/cache.h>
#include <flash_blk.h>
#include <clk.h>
#include <dm.h>
#include <dm/device-internal.h>
#include <spi_flash.h>
#include <drv_flash_os_impl.h>
#include <mdrv_spinor.h>
#include <drv_spinor.h>

#define SSTAR_SPINOR_MAX_CHIP 8

struct sstar_spinor_chip
{
    struct spi_flash     flash;
    struct spinor_handle handle;
    u8 *                 bdma_buf;
};

struct sstar_spinor_config
{
    u8                        block_device_init;
    u8 *                      cis_mapping_buffer;
    struct sstar_spinor_chip *chip[SSTAR_SPINOR_MAX_CHIP];
};

static struct sstar_spinor_config spinor_config = {
    .block_device_init  = 0,
    .cis_mapping_buffer = NULL,
};

unsigned long flash_init(void)
{
    unsigned int      bus        = CONFIG_SF_DEFAULT_BUS;
    unsigned int      cs         = CONFIG_SF_DEFAULT_CS;
    unsigned int      speed      = CONFIG_SF_DEFAULT_SPEED;
    unsigned int      mode       = CONFIG_SF_DEFAULT_MODE;
    unsigned long     flash_size = 0;
    struct spi_flash *flash      = NULL;

    memset((void *)spinor_config.chip, 0, sizeof(spinor_config.chip));

    flash = spi_flash_probe(bus, cs, speed, mode);

    if (!flash)
        return 0;

    flash_size = flash->size;

    spi_flash_free(flash);

    return flash_size;
}

int _spi_flash_read(struct mtd_info *mtd, loff_t from, size_t len, size_t *retlen, u_char *buf)
{
    u8                        status      = 0;
    u32                       read_size   = 0;
    u32                       aligne_addr = 0;
    u32                       aligne_size = 0;
    u32                       block_size  = len;
    u8 *                      read_buf    = buf;
    struct sstar_spinor_chip *spinor_chip = (struct sstar_spinor_chip *)mtd->priv;

    /* sanity checks */
    if (!len)
        return -EINVAL;

    if (from + len > mtd->size)
        return -EINVAL;

    if ((unsigned long)buf % ARCH_DMA_MINALIGN)
    {
        read_buf   = spinor_chip->bdma_buf;
        block_size = mtd->erasesize;
    }

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
        read_size = block_size;

        if (len < read_size)
            read_size = len;

        if (ERR_SPINOR_SUCCESS != (status = mdrv_spinor_read(&spinor_chip->handle, aligne_addr, read_buf, read_size)))
        {
            *retlen = 0;
            return (-EIO);
        }

        if (read_buf != buf)
            memcpy((void *)buf, (const void *)(read_buf + aligne_size), (read_size - aligne_size));

        len -= read_size;
        buf = buf + read_size - aligne_size;
        aligne_addr += read_size;
        aligne_size = 0;
    }

    return 0;
}

int _spi_flash_write(struct mtd_info *mtd, loff_t to, size_t len, size_t *retlen, const u_char *buf)
{
    printk(KERN_WARNING "%s to 0x%llx, len 0x%lx from 0x%lx \r\n", __func__, to, (unsigned long)len,
           (unsigned long)buf);
    unsigned long             starttime      = get_timer(0);
    u16                       u16_write_size = 0;
    u8 *                      pu8_data       = (u8 *)buf;
    u8 *                      pu8_write_data;
    unsigned long             total;
    unsigned long             percent            = 0;
    unsigned long             pre_percent        = 0;
    unsigned long             u32_page_size      = mtd->writesize;
    unsigned long             u32_page_size_mask = u32_page_size - 1;
    struct sstar_spinor_chip *spinor_chip        = (struct sstar_spinor_chip *)mtd->priv;

    /* sanity checks */
    if (!len)
        return (-EINVAL);
    if (to + len > mtd->size)
        return (-EINVAL);

    total = len;
    while (0 != len)
    {
        u16_write_size = u32_page_size - (u32_page_size_mask & to);
        if (u16_write_size > len)
        {
            u16_write_size = len;
        }

        if (((unsigned long)pu8_data % ARCH_DMA_MINALIGN) || u16_write_size % ARCH_DMA_MINALIGN)
        {
            memcpy((void *)spinor_chip->bdma_buf, (const void *)pu8_data, u16_write_size);
            pu8_write_data = (u8 *)spinor_chip->bdma_buf;
        }
        else
        {
            pu8_write_data = pu8_data;
        }

        if (ERR_SPINOR_SUCCESS != mdrv_spinor_program(&spinor_chip->handle, to, pu8_write_data, u16_write_size))
        {
            printk("[FLASH_ERR] Program page fail\r\n");
            return (-EIO);
        }

        to += u16_write_size;
        pu8_data += u16_write_size;
        len -= u16_write_size;

        percent = (total - len) * 100 / total;
        if (((percent - pre_percent) >= 5) || (percent == 100))
        {
            printk(KERN_WARNING "\rWriting at 0x%llx -- %3ld%% complete.", to, percent);
            pre_percent = percent;
        }
    }

    printk(KERN_WARNING "(cost %ld ms)\n", get_timer(0) - starttime);
    return 0;
}

int _spi_flash_erase(struct mtd_info *mtd, struct erase_info *instr)
{
    printk(KERN_WARNING "%s: addr 0x%llx, len 0x%llx \r\n", __func__, instr->addr, instr->len);

    unsigned long             u32_bytes_left = 0;
    unsigned long             starttime      = get_timer(0);
    unsigned long             percent        = 0;
    unsigned long             pre_percent    = 0;
    unsigned long             u32_erase_size;
    unsigned long             u32_sector_size;
    unsigned long             u32_block_size;
    flash_nor_info_t          st_flash_nor_info;
    struct sstar_spinor_chip *spinor_chip = (struct sstar_spinor_chip *)mtd->priv;

    mdrv_spinor_info(&spinor_chip->handle, &st_flash_nor_info);

    u32_sector_size = st_flash_nor_info.sector_size;
    u32_block_size  = st_flash_nor_info.block_size;
    /* range and alignment check */
    if (!instr->len)
        return (-EINVAL);
    if (instr->addr + instr->len > mtd->size || instr->len % u32_sector_size || instr->addr % u32_sector_size)
        return (-EINVAL);

    u32_bytes_left = instr->len;

    while (0 != u32_bytes_left)
    {
        if ((0 != (~(u32_block_size - 1) & u32_bytes_left)) && (0 == ((u32_block_size - 1) & instr->addr)))
        {
            if (ERR_SPINOR_SUCCESS != mdrv_spinor_erase(&spinor_chip->handle, instr->addr, u32_block_size))
            {
                printk(KERN_WARNING "block erase failed!\n");
                return (-EIO);
            }
            u32_erase_size = u32_block_size;
        }
        else
        {
            if (ERR_SPINOR_SUCCESS != mdrv_spinor_erase(&spinor_chip->handle, instr->addr, u32_sector_size))
            {
                printk(KERN_WARNING "sector erase failed!\n");
                return (-EIO);
            }
            u32_erase_size = u32_sector_size;
        }

        instr->addr += u32_erase_size;
        u32_bytes_left -= u32_erase_size;

        percent = (instr->len - u32_bytes_left) * 100 / instr->len;
        if (((percent - pre_percent) >= 5) || (percent == 100))
        {
            printk(KERN_WARNING "\rErasing at 0x%llx -- %3ld%% complete.", instr->addr, percent);
            pre_percent = percent;
        }
    }

    printk(KERN_WARNING "(cost %ld ms)\n", get_timer(0) - starttime);
    return 0;
}

struct spi_flash *spi_flash_probe(unsigned int bus, unsigned int cs, unsigned int max_hz, unsigned int spi_mode)
{
    struct sstar_spinor_chip *spinor_chip = NULL;
    struct spinor_init        init;
    flash_nor_info_t          st_flash_nor_info;

    if (((bus << 1) + cs) >= SSTAR_SPINOR_MAX_CHIP)
        return NULL;

    if (!spinor_config.block_device_init && spinor_config.chip[(bus << 1) + cs])
        return (&spinor_config.chip[(bus << 1) + cs]->flash);

    do
    {
        if (NULL == (spinor_chip = (struct sstar_spinor_chip *)malloc(sizeof(struct sstar_spinor_chip))))
        {
            printf("spinor probe: Failed to allocate memory\n");
            break;
        }

        memset((void *)spinor_chip, 0, sizeof(struct sstar_spinor_chip));

        if (!spinor_config.cis_mapping_buffer)
        {
            if (NULL == (spinor_config.cis_mapping_buffer = (u8 *)kzalloc(CIS_PAGE_SIZE * CIS_MAPPING_CNT, GFP_KERNEL)))
            {
                break;
            }
        }

        spinor_chip->handle.ctrl_id       = spiflash_get_master(bus);
        spinor_chip->handle.msg.cs_select = cs;

        if (0xFF == spinor_chip->handle.ctrl_id)
            break;

        strcpy((char *)spinor_chip->flash.mtd_name, "nor");
        spinor_chip->flash.mtd_name[3] = '0' + (bus << 1) + cs;

        init.bus                  = (u8)bus;
        init.cs_select            = (u8)cs;
        init.dev_name             = spinor_chip->flash.mtd_name;
        init.cis_map              = spinor_config.cis_mapping_buffer;
        init.cis_cnt              = CIS_MAPPING_CNT;
        init.bypass_io            = 0;
        init.autok_parrtern_check = NULL;

        if (!sstar_spinor_init(&init))
        {
            break;
        }

        if (!sstar_cis_get_nri(&spinor_chip->handle))
        {
            printk("No nri!\r\n");
            break;
        }

        mdrv_spinor_setup_by_nri(&spinor_chip->handle);

        mdrv_spinor_info(&spinor_chip->handle, &st_flash_nor_info);

        if (NULL == (spinor_chip->bdma_buf = (u8 *)memalign(ARCH_DMA_MINALIGN, st_flash_nor_info.block_size)))
        {
            break;
        }

        spinor_chip->flash.name        = spinor_chip->flash.mtd_name;
        spinor_chip->flash.priv        = spinor_chip;
        spinor_chip->flash.spi         = NULL;
        spinor_chip->flash.size        = st_flash_nor_info.capacity;
        spinor_chip->flash.page_size   = st_flash_nor_info.page_size;
        spinor_chip->flash.sector_size = st_flash_nor_info.sector_size;
        spinor_chip->flash.erase_size  = st_flash_nor_info.sector_size;

        spinor_chip->flash.mtd.name      = spinor_chip->flash.mtd_name;
        spinor_chip->flash.mtd.priv      = spinor_chip;
        spinor_chip->flash.mtd.type      = MTD_NORFLASH;
        spinor_chip->flash.mtd.writesize = st_flash_nor_info.page_size;
        spinor_chip->flash.mtd.erasesize = st_flash_nor_info.sector_size;
        spinor_chip->flash.mtd.flags     = MTD_CAP_NORFLASH;
        spinor_chip->flash.mtd.size      = st_flash_nor_info.capacity;
        spinor_chip->flash.mtd._erase    = _spi_flash_erase;
        spinor_chip->flash.mtd._read     = _spi_flash_read;
        spinor_chip->flash.mtd._write    = _spi_flash_write;
        printk(
            "mtd .name = %s, .size = 0x%.8x (%uMiB) "
            ".erasesize = 0x%.8x .\n",
            spinor_chip->flash.mtd.name, (unsigned int)spinor_chip->flash.mtd.size,
            (unsigned int)(spinor_chip->flash.mtd.size >> 20), (unsigned int)spinor_chip->flash.mtd.erasesize);

        if (add_mtd_device(&spinor_chip->flash.mtd))
        {
            printk("add mtd device fail!!!\n");
            break;
        }

        printk("Detected %s with total size ", spinor_chip->flash.name);
        print_size(spinor_chip->flash.size, "\n");

        if (!spinor_config.block_device_init)
            spinor_config.chip[(bus << 1) + cs] = spinor_chip;

        return (&spinor_chip->flash);
    } while (0);

    if (spinor_chip && spinor_chip->bdma_buf)
    {
        kfree((void *)spinor_chip->bdma_buf);
    }

    if (spinor_chip)
    {
        free((void *)spinor_chip);
    }

    return NULL;
}

struct spi_flash *spi_flash_probe_driver(unsigned long u32_address, unsigned int bus, unsigned int cs,
                                         unsigned int max_hz, unsigned int spi_mode)
{
    struct sstar_spinor_chip *spinor_chip = NULL;
    flash_nor_info_t          st_flash_nor_info;

    if (((bus << 1) + cs) >= SSTAR_SPINOR_MAX_CHIP)
        return NULL;

    if (spinor_config.chip[(bus << 1) + cs])
    {
        spinor_chip = spinor_config.chip[(bus << 1) + cs];

        del_mtd_device(&spinor_chip->flash.mtd);

        kfree((void *)spinor_chip->bdma_buf);

        free((void *)spinor_chip);

        spinor_config.chip[(bus << 1) + cs] = NULL;
    }

    do
    {
        if (NULL == (spinor_chip = (struct sstar_spinor_chip *)malloc(sizeof(struct sstar_spinor_chip))))
        {
            printf("spinor probe: Failed to allocate memory\n");
            break;
        }

        memset((void *)spinor_chip, 0, sizeof(struct sstar_spinor_chip));

        spinor_chip->handle.ctrl_id       = spiflash_get_master(bus);
        spinor_chip->handle.msg.cs_select = cs;

        if (0xFF == spinor_chip->handle.ctrl_id)
            break;

        mdrv_spinor_setup_by_default(&spinor_chip->handle);

        if (ERR_SPINOR_SUCCESS != mdrv_spinor_reset(&spinor_chip->handle))
        {
            printk("[FLASH] reset fail\r\n");
            break;
        }

        if (!sstar_cis_get_nri_from_dram(&spinor_chip->handle, (u8 *)u32_address))
        {
            printk("[FLASH] No sni!\r\n");
            break;
        }

        if (mdrv_spinor_hardware_init(&spinor_chip->handle))
        {
            printk("flash init failed!\n");
            break;
        }

        mdrv_spinor_info(&spinor_chip->handle, &st_flash_nor_info);

        if (NULL == (spinor_chip->bdma_buf = (u8 *)kzalloc(st_flash_nor_info.page_size, GFP_KERNEL)))
        {
            break;
        }

        strcpy((char *)spinor_chip->flash.mtd_name, "nor");
        spinor_chip->flash.mtd_name[3] = '0' + (bus << 1) + cs;

        spinor_chip->flash.name        = spinor_chip->flash.mtd_name;
        spinor_chip->flash.priv        = spinor_chip;
        spinor_chip->flash.spi         = NULL;
        spinor_chip->flash.size        = st_flash_nor_info.capacity;
        spinor_chip->flash.page_size   = st_flash_nor_info.page_size;
        spinor_chip->flash.sector_size = st_flash_nor_info.sector_size;
        spinor_chip->flash.erase_size  = st_flash_nor_info.sector_size;

        spinor_chip->flash.mtd.name      = spinor_chip->flash.mtd_name;
        spinor_chip->flash.mtd.priv      = spinor_chip;
        spinor_chip->flash.mtd.type      = MTD_NORFLASH;
        spinor_chip->flash.mtd.writesize = st_flash_nor_info.page_size;
        spinor_chip->flash.mtd.erasesize = st_flash_nor_info.sector_size;
        spinor_chip->flash.mtd.flags     = MTD_CAP_NORFLASH;
        spinor_chip->flash.mtd.size      = st_flash_nor_info.capacity;
        spinor_chip->flash.mtd._erase    = _spi_flash_erase;
        spinor_chip->flash.mtd._read     = _spi_flash_read;
        spinor_chip->flash.mtd._write    = _spi_flash_write;
        printk(
            "mtd .name = %s, .size = 0x%.8x (%uMiB) "
            ".erasesize = 0x%.8x .\n",
            spinor_chip->flash.mtd.name, (unsigned int)spinor_chip->flash.mtd.size,
            (unsigned int)(spinor_chip->flash.mtd.size >> 20), (unsigned int)spinor_chip->flash.mtd.erasesize);

        if (add_mtd_device(&spinor_chip->flash.mtd))
        {
            break;
        }

        printk("SF: add mtd device successfully\n");

        printk("SF: Detected %s with total size ", spinor_chip->flash.name);
        print_size(spinor_chip->flash.size, "\n");

        spinor_config.chip[(bus << 1) + cs] = spinor_chip;

        return (&spinor_chip->flash);
    } while (0);

    if (spinor_chip && spinor_chip->bdma_buf)
    {
        kfree((void *)spinor_chip->bdma_buf);
    }

    if (spinor_chip)
    {
        free((void *)spinor_chip);
    }

    return NULL;
}

void spi_flash_free(struct spi_flash *flash) {}

#if defined(CONFIG_SSTAR_FLASH_DM)
static int _spi_flash_bread(struct udevice *udev, u32 start, u32 blkcnt, void *p_data)
{
    int                      ret;
    u32                      addr;
    size_t                   retlen;
    size_t                   len;
    struct sstar_flash_info *priv    = dev_get_priv(udev);
    struct mtd_info *        mtd     = priv->mtd;
    u32                      blksize = mtd->erasesize;

    addr = start * blksize;
    len  = blkcnt * blksize;

    ret = mtd->_read(mtd, addr, len, &retlen, p_data);
    if (unlikely(ret < 0))
    {
        return ret;
    }

    return blkcnt;
}

static int _spi_flash_bwrite(struct udevice *udev, u32 start, u32 blkcnt, const void *p_data)
{
    int                      ret;
    u32                      addr;
    size_t                   retlen;
    size_t                   len;
    struct sstar_flash_info *priv    = dev_get_priv(udev);
    struct mtd_info *        mtd     = priv->mtd;
    u32                      blksize = mtd->erasesize;

    addr = start * blksize;
    len  = blkcnt * blksize;

    ret = mtd->_write(mtd, addr, len, &retlen, p_data);
    if (unlikely(ret < 0))
    {
        return ret;
    }

    return blkcnt;
}

static int _spi_flash_berase(struct udevice *udev, u32 start, u32 blkcnt)
{
    int                      ret;
    struct erase_info        instr;
    struct sstar_flash_info *priv    = dev_get_priv(udev);
    struct mtd_info *        mtd     = priv->mtd;
    u32                      blksize = mtd->erasesize;
    u32                      addr    = start * blksize;
    u32                      len     = blkcnt * blksize;

    if (addr % mtd->erasesize || len % mtd->erasesize)
    {
        printk("SF: Erase offset/length not multiple of erase size\n");
        return -EINVAL;
    }

    memset(&instr, 0, sizeof(instr));
    instr.addr = addr;
    instr.len  = len;

    ret = mtd->_erase(mtd, &instr);
    if (unlikely(ret < 0))
    {
        return ret;
    }

    return blkcnt;
}

static int _spi_flash_dm_bind(struct udevice *udev)
{
    u32             bus = 0;
    u32             cs  = 0;
    struct udevice *bdev;
    int             ret = 0;

    bus = dev_read_u32_default(udev, "engine", 0);
    cs  = dev_read_u32_default(udev, "cs-select", 0);
    bus &= 0xFF;
    cs &= 0xFF;

    ret = blk_create_devicef(udev, "sstar_flash_blk", "spinor.blk", IF_TYPE_SPINOR, (bus << 1) + cs, 512, 0, &bdev);
    if (ret)
    {
        printk("Cannot create block device\n");
        return ret;
    }

    return 0;
}

static int _spi_flash_dm_probe(struct udevice *udev)
{
    struct sstar_flash_info *priv  = dev_get_priv(udev);
    struct spi_flash *       flash = NULL;
    u32                      bus   = 0;
    u32                      cs    = 0;

    bus = dev_read_u32_default(udev, "engine", 0);
    cs  = dev_read_u32_default(udev, "cs-select", 0);
    bus &= 0xFF;
    cs &= 0xFF;

    spinor_config.block_device_init = 1;

    flash = spi_flash_probe(bus, cs, 0, 0);

    spinor_config.block_device_init = 0;

    if (!flash)
    {
        goto err;
    }

    priv->flash_con_type = IF_TYPE_SPINOR;
    priv->mtd            = &flash->mtd;
    priv->density        = flash->size;
    priv->read           = _spi_flash_bread;
    priv->write          = _spi_flash_bwrite;
    priv->erase          = _spi_flash_berase;

    debug("%s probe successful\n", __func__);

    return 0;
err:
    return -1;
}

static const struct udevice_id norflash_ids[] = {{.compatible = "sstar,norflash"}, {}};

U_BOOT_DRIVER(norflash) = {
    .name      = "norflash",
    .id        = UCLASS_SPI_FLASH,
    .of_match  = norflash_ids,
    .bind      = _spi_flash_dm_bind,
    .probe     = _spi_flash_dm_probe,
    .priv_auto = sizeof(struct sstar_flash_info),
};

#endif /* CONFIG_SSTAR_FLASH_DM */
