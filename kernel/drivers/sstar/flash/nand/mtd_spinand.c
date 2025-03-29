/*
 * mtd_spinand.c- Sigmastar
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
#include <linux/string.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/spinlock.h>
#include <linux/printk.h>
#include <linux/platform_device.h>
#include <linux/dma-mapping.h>
#include <linux/delay.h>
#include <linux/sched.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/rawnand.h>
#include <linux/mtd/partitions.h>
#include <linux/version.h>
#include <linux/slab.h>
#include <linux/clk.h>
#include <linux/clk-provider.h>
#include <linux/types.h>
#include <linux/crc32.h>
#include <ms_platform.h>
#include <mtdcore.h>
#include <cam_sysfs.h>
#include <ms_msys.h>
#include <generated/uapi/linux/version.h>
#include <nand/raw/internals.h>
#include <mdrv_spinand.h>
#include <mdrv_spinand_bbtbbm.h>
#include <cis.h>

struct command_ops
{
    u32 command_ctrl;
    u32 r_ptr;
    u32 w_ptr;
    u32 buf_size;
};

struct bdma_alloc
{
    dma_addr_t  bdma_phy_addr;
    const char *DMEM_BDMA_INPUT;
    u8 *        bdma_vir_addr;
};

struct sstar_spinand_chip
{
    struct mutex             lock;
    struct nand_chip         nand;
    struct mtd_ooblayout_ops nand_ooblayout;
    struct command_ops       command_ops;
    struct spinand_handle    handle;
    u8 *                     cis_mapping_buffer;
    u8 *                     fcie_buffer;
    u8 *                     bdma_buf;
};

#if 1
#define spi_nand_msg(fmt, ...) printk(KERN_NOTICE "%s: " fmt "\n", __func__, ##__VA_ARGS__)
#define spi_nand_debug(fmt, ...)
#else
#define spi_nand_msg(fmt, ...)
#define spi_nand_debug(fmt, ...) printk(KERN_NOTICE "%s: " fmt "\n", __func__, ##__VA_ARGS__)
#endif

static u8 sstar_spinand_read_byte(struct nand_chip *chip)
{
    u8                         u8_byte;
    u8 *                       data;
    struct mtd_info *          mtd          = nand_to_mtd(chip);
    struct sstar_spinand_chip *spinand_chip = (struct sstar_spinand_chip *)mtd->priv;
    mutex_lock(&spinand_chip->lock);

    data = spinand_chip->bdma_buf;

    u8_byte = data[spinand_chip->command_ops.r_ptr];

    spinand_chip->command_ops.r_ptr += 1;

    if (spinand_chip->command_ops.r_ptr == spinand_chip->command_ops.w_ptr)
        spinand_chip->command_ops.r_ptr = 0;

    mutex_unlock(&spinand_chip->lock);
    return u8_byte;
}

static void sstar_spinand_read_buf(struct nand_chip *chip, u8 *buf, int len)
{
    u32                        read_byte;
    struct mtd_info *          mtd          = nand_to_mtd(chip);
    struct sstar_spinand_chip *spinand_chip = mtd->priv;

    mutex_lock(&spinand_chip->lock);

    while (len != 0)
    {
        read_byte = (spinand_chip->command_ops.w_ptr - spinand_chip->command_ops.r_ptr);
        read_byte = (read_byte > len) ? len : read_byte;

        memcpy((void *)buf, (const void *)(spinand_chip->bdma_buf + spinand_chip->command_ops.r_ptr), read_byte);

        len -= read_byte;
        buf += read_byte;
        spinand_chip->command_ops.r_ptr += read_byte;

        if (spinand_chip->command_ops.r_ptr == spinand_chip->command_ops.w_ptr)
            spinand_chip->command_ops.r_ptr = 0;
    }
    mutex_unlock(&spinand_chip->lock);
}

static void sstar_spinand_select_chip(struct nand_chip *chip, int cs)
{
    spi_nand_debug("sstar_spinand_select_chip  Not support\r\n");
}

static void sstar_spinand_cmd_ctrl(struct nand_chip *chip, int dat, unsigned int ctrl)
{
    spi_nand_debug("sstar_spinand_cmd_ctrl Not support\r\n");
}

static int sstar_spinand_dev_ready(struct nand_chip *chip)
{
    spi_nand_debug("sstar_spinand_dev_ready Not support\r\n");

    return 1;
}

static void sstar_spinand_cmdfunc(struct nand_chip *chip, unsigned command, int column, int page_addr)
{
    struct mtd_info *          mtd          = nand_to_mtd(chip);
    struct sstar_spinand_chip *spinand_chip = (struct sstar_spinand_chip *)mtd->priv;

    mutex_lock(&spinand_chip->lock);

    spinand_chip->command_ops.r_ptr = 0;

    switch (command)
    {
        case NAND_CMD_STATUS:
            spi_nand_debug("NAND_CMD_STATUS");
            spinand_chip->command_ops.command_ctrl = NAND_CMD_STATUS;
            spinand_chip->command_ops.w_ptr        = 1;
            mdrv_spinand_read_status(&spinand_chip->handle, spinand_chip->bdma_buf);
            break;

        case NAND_CMD_READOOB:
            spi_nand_debug("NAND_CMD_READOOB %d", column);
            spinand_chip->command_ops.command_ctrl = NAND_CMD_READOOB;
            spinand_chip->command_ops.w_ptr        = 1;
            mdrv_spinand_page_read(&spinand_chip->handle, page_addr, (u16)(column + mtd->writesize),
                                   spinand_chip->bdma_buf, 1);
            break;

        case NAND_CMD_READID:
            spi_nand_debug("NAND_CMD_READID");
            spinand_chip->command_ops.command_ctrl = NAND_CMD_READID;
            spinand_chip->command_ops.w_ptr        = 6;
            mdrv_spinand_read_id(&spinand_chip->handle, spinand_chip->bdma_buf, 6);
            break;

        case NAND_CMD_ERASE2:
            spi_nand_debug("NAND_CMD_ERASE2");
            break;

        case NAND_CMD_ERASE1:
            spi_nand_debug("NAND_CMD_ERASE1, page_addr: 0x%x", page_addr);
            mdrv_spinand_block_erase(&spinand_chip->handle, page_addr);
            break;

        case NAND_CMD_RESET:
            spi_nand_debug("NAND_CMD_RESET");
            mdrv_spinand_reset(&spinand_chip->handle);
            mdrv_spinand_setup(&spinand_chip->handle);
            break;

        default:
            printk("unsupported command %02Xh", command);
            break;
    }
    mutex_unlock(&spinand_chip->lock);
    return;
}

static int sstar_spinand_waitfunc(struct nand_chip *chip)
{
    u8                         status;
    struct mtd_info *          mtd          = nand_to_mtd(chip);
    struct sstar_spinand_chip *spinand_chip = mtd->priv;

    mutex_lock(&spinand_chip->lock);

    spi_nand_debug("sstar_spinand_waitfunc\r\n");

    status = mdrv_spinand_read_status(&spinand_chip->handle, NULL);

    mutex_unlock(&spinand_chip->lock);
    return (ERR_SPINAND_E_FAIL > status) ? NAND_STATUS_READY : NAND_STATUS_FAIL;
}

static void sstar_spinand_ecc_hwctl(struct nand_chip *chip, int mode)
{
    spi_nand_debug(" sstar_spinand_ecc_hwctl Not support");
}

static int sstar_spinand_ecc_calculate(struct nand_chip *chip, const uint8_t *dat, uint8_t *ecc_code)
{
    spi_nand_debug("sstar_spinand_ecc_calculate Not support");
    return 0;
}

static int sstar_spinand_ecc_correct(struct nand_chip *chip, uint8_t *dat, uint8_t *read_ecc, uint8_t *calc_ecc)
{
    spi_nand_debug(" sstar_spinand_ecc_correct Not support");
    return 0;
}

static int sstar_spinand_ecc_read_page_raw(struct nand_chip *chip, uint8_t *buf, int oob_required, int page)
{
    struct mtd_info *          mtd          = nand_to_mtd(chip);
    struct sstar_spinand_chip *spinand_chip = mtd->priv;
    u8 *                       data;
    u32                        read_size;

    mutex_lock(&spinand_chip->lock);

    data      = spinand_chip->bdma_buf;
    read_size = oob_required ? (mtd->writesize + mtd->oobsize) : mtd->writesize;

    if (ERR_SPINAND_TIMEOUT <= mdrv_spinand_page_read_raw(&spinand_chip->handle, page, 0, data, read_size))
    {
        mutex_unlock(&spinand_chip->lock);
        return -EIO;
    }

    memcpy((void *)buf, (const void *)data, mtd->writesize);
    if (oob_required)
    {
        memcpy((void *)(chip->oob_poi), (const void *)(data + mtd->writesize), mtd->oobsize);
    }

    mutex_unlock(&spinand_chip->lock);
    return 0;
}

static int sstar_spinand_ecc_write_page_raw(struct nand_chip *chip, const uint8_t *buf, int oob_required, int page)
{
    struct mtd_info *          mtd          = nand_to_mtd(chip);
    struct sstar_spinand_chip *spinand_chip = mtd->priv;
    u8 *                       data;
    u32                        write_size;

    mutex_lock(&spinand_chip->lock);
    spi_nand_debug("sstar_spinand_ecc_write_page_raw\r\n");

    data       = spinand_chip->bdma_buf;
    write_size = oob_required ? (mtd->writesize + mtd->oobsize) : mtd->writesize;

    memcpy((void *)data, (const void *)buf, mtd->writesize);
    if (oob_required)
        memcpy((void *)(data + mtd->writesize), (const void *)(chip->oob_poi), mtd->oobsize);

    if (ERR_SPINAND_SUCCESS != mdrv_spinand_page_program_raw(&spinand_chip->handle, page, 0, data, write_size))
    {
        mutex_unlock(&spinand_chip->lock);
        return -EIO;
    }

    mutex_unlock(&spinand_chip->lock);
    return 0;
}

static int sstar_spinand_ecc_read_page(struct nand_chip *chip, uint8_t *buf, int oob_required, int page)
{
    struct mtd_info *          mtd          = nand_to_mtd(chip);
    struct sstar_spinand_chip *spinand_chip = mtd->priv;
    u8                         status;
    u8 *                       data;
    u32                        read_size;

    mutex_lock(&spinand_chip->lock);

    data      = spinand_chip->bdma_buf;
    read_size = oob_required ? (mtd->writesize + mtd->oobsize) : mtd->writesize;

    status = mdrv_spinand_page_read(&spinand_chip->handle, page, 0, data, read_size);

    memcpy((void *)buf, (const void *)data, mtd->writesize);
    if (oob_required)
        memcpy((void *)(chip->oob_poi), (const void *)(data + mtd->writesize), mtd->oobsize);

    if (ERR_SPINAND_ECC_NOT_CORRECTED == status)
    {
        mtd->ecc_stats.failed++;
        spi_nand_msg("ecc failed");
    }
    else if (ERR_SPINAND_ECC_CORRECTED == status)
        mtd->ecc_stats.corrected++;
    else if (ERR_SPINAND_TIMEOUT <= status)
    {
        mutex_unlock(&spinand_chip->lock);
        return -EIO;
    }

    mutex_unlock(&spinand_chip->lock);
    return (ERR_SPINAND_ECC_CORRECTED == status) ? 1 : 0;
}

static int sstar_spinand_ecc_read_subpage(struct nand_chip *chip, uint32_t offs, uint32_t len, uint8_t *buf, int page)
{
    struct mtd_info *          mtd          = nand_to_mtd(chip);
    struct sstar_spinand_chip *spinand_chip = mtd->priv;
    u8                         status;
    u8 *                       data;
    u8                         len_align = 0;
    u8                         off_align = 0;
    u16                        column;
    u32                        size;

    mutex_lock(&spinand_chip->lock);
    spi_nand_debug("page = 0x%x, offs = 0x%x, len = 0x%x", page, offs, len);

    data = spinand_chip->bdma_buf;

    off_align = offs - ((offs >> CONFIG_ALIGN_BIT) << CONFIG_ALIGN_BIT);
    column    = offs - off_align;

    size      = len + off_align;
    len_align = CONFIG_FLASH_ADDR_ALIGN - (size - ((size >> CONFIG_ALIGN_BIT) << CONFIG_ALIGN_BIT));
    size      = size + len_align;

    if (size > mtd->writesize)
    {
        size = mtd->writesize;
    }

    status = mdrv_spinand_page_read(&spinand_chip->handle, page, (u16)column, data, size);

    memcpy((void *)(buf + offs), (const void *)(data + off_align), len);

    if (ERR_SPINAND_ECC_NOT_CORRECTED == status)
    {
        mtd->ecc_stats.failed++;
        spi_nand_msg("ecc failed");
    }
    else if (ERR_SPINAND_ECC_CORRECTED == status)
        mtd->ecc_stats.corrected++;
    else if (ERR_SPINAND_TIMEOUT <= status)
    {
        mutex_unlock(&spinand_chip->lock);
        return -EIO;
    }

    mutex_unlock(&spinand_chip->lock);
    return (ERR_SPINAND_ECC_CORRECTED == status) ? 1 : 0;
}

static int sstar_spinand_ecc_write_subpage(struct nand_chip *chip, uint32_t offset, uint32_t data_len,
                                           const uint8_t *data_buf, int oob_required, int page)
{
    struct mtd_info *          mtd          = nand_to_mtd(chip);
    struct sstar_spinand_chip *spinand_chip = mtd->priv;
    u8 *                       data;
    u32                        write_size;

    mutex_lock(&spinand_chip->lock);

    spi_nand_debug("sstar_spinand_ecc_write_subpage\r\n");

    data       = spinand_chip->bdma_buf;
    write_size = oob_required ? (data_len + mtd->oobsize) : data_len;

    memcpy((void *)data, (const void *)data_buf, data_len);
    if (oob_required)
        memcpy((void *)(data + data_len), (const void *)(chip->oob_poi), mtd->oobsize);

    if (ERR_SPINAND_SUCCESS != mdrv_spinand_page_program(&spinand_chip->handle, page, (u16)offset, data, write_size))
    {
        mutex_unlock(&spinand_chip->lock);
        return -EIO;
    }

    mutex_unlock(&spinand_chip->lock);
    return 0;
}

static int sstar_spinand_ecc_write_page(struct nand_chip *chip, const uint8_t *buf, int oob_required, int page)
{
    struct mtd_info *          mtd          = nand_to_mtd(chip);
    struct sstar_spinand_chip *spinand_chip = mtd->priv;
    u8 *                       data;
    u32                        write_size;

    mutex_lock(&spinand_chip->lock);
    spi_nand_debug("sstar_spinand_ecc_write_page\r\n");

    data       = spinand_chip->bdma_buf;
    write_size = oob_required ? (mtd->writesize + mtd->oobsize) : mtd->writesize;

    memcpy((void *)data, (const void *)buf, mtd->writesize);
    if (oob_required)
        memcpy((void *)(data + mtd->writesize), (const void *)(chip->oob_poi), mtd->oobsize);

    if (ERR_SPINAND_SUCCESS != mdrv_spinand_page_program(&spinand_chip->handle, page, 0, data, write_size))
    {
        mutex_unlock(&spinand_chip->lock);
        return -EIO;
    }

    mutex_unlock(&spinand_chip->lock);
    return 0;
}

static int sstar_spinand_read_oob_std(struct nand_chip *chip, int page)
{
    struct mtd_info *          mtd          = nand_to_mtd(chip);
    struct sstar_spinand_chip *spinand_chip = mtd->priv;
    u8 *                       data;

    mutex_lock(&spinand_chip->lock);

    data = spinand_chip->bdma_buf;

    if (ERR_SPINAND_TIMEOUT
        <= mdrv_spinand_page_read(&spinand_chip->handle, page, (u16)(mtd->writesize), data, mtd->oobsize))
    {
        mutex_unlock(&spinand_chip->lock);
        return -EIO;
    }

    memcpy((void *)(chip->oob_poi), (const void *)data, mtd->oobsize);

    mutex_unlock(&spinand_chip->lock);
    return 0;
}

static int sstar_spinand_write_oob_std(struct nand_chip *chip, int page)
{
    struct mtd_info *          mtd          = nand_to_mtd(chip);
    struct sstar_spinand_chip *spinand_chip = mtd->priv;
    u8 *                       data;

    mutex_lock(&spinand_chip->lock);

    data = spinand_chip->bdma_buf;

    memcpy((void *)data, (const void *)chip->oob_poi, mtd->oobsize);

    if (ERR_SPINAND_SUCCESS
        != mdrv_spinand_page_program(&spinand_chip->handle, page, (u16)(mtd->writesize), data, mtd->oobsize))
    {
        mutex_unlock(&spinand_chip->lock);
        return -EIO;
    }

    mutex_unlock(&spinand_chip->lock);
    return 0;
}

static int sstar_spinand_get_fact_prot_info(struct mtd_info *mtd, size_t len, size_t *retlen, struct otp_info *buf)
{
    unsigned int               start, length;
    struct sstar_spinand_chip *spinand_chip = mtd->priv;

    if (ERR_SPINAND_SUCCESS != mdrv_spinand_get_otp_layout(&spinand_chip->handle, &start, &length, 0))
        return -EIO;

    *retlen = 0;

    if (length)
    {
        buf[0].start  = start;
        buf[0].length = length;
        buf[0].locked = 1;
        *retlen       = sizeof(struct otp_info);
    }

    return 0;
}

static int sstar_spinand_get_user_prot_info(struct mtd_info *mtd, size_t len, size_t *retlen, struct otp_info *buf)
{
    unsigned int               start, length;
    struct sstar_spinand_chip *spinand_chip = (struct sstar_spinand_chip *)mtd->priv;

    if (ERR_SPINAND_SUCCESS != mdrv_spinand_get_otp_layout(&spinand_chip->handle, &start, &length, 1))
        return -EIO;

    mutex_lock(&spinand_chip->lock);
    *retlen = 0;

    if (length)
    {
        buf[0].start  = start;
        buf[0].length = length;
        buf[0].locked = mdrv_spinand_get_otp_lock(&spinand_chip->handle);
        *retlen       = sizeof(struct otp_info);
    }
    mutex_unlock(&spinand_chip->lock);

    return 0;
}

static int sstar_spinand_read_fact_prot_reg(struct mtd_info *mtd, loff_t from, size_t len, size_t *retlen, u_char *buf)
{
    unsigned int               start, length, end;
    struct sstar_spinand_chip *spinand_chip = (struct sstar_spinand_chip *)mtd->priv;

    if (ERR_SPINAND_SUCCESS != mdrv_spinand_get_otp_layout(&spinand_chip->handle, &start, &length, 0))
        return -EIO;

    if (buf == NULL)
        return ERR_SPINAND_SUCCESS;

    end = start + length;
    if (from < start || from >= end)
    {
        *retlen = 0;
        return -EFAULT;
    }

    len = min_t(size_t, end - from, len);

    mutex_lock(&spinand_chip->lock);
    *retlen = mdrv_spinand_read_otp(&spinand_chip->handle, from / mtd->writesize, from % mtd->writesize, buf, len);
    mutex_unlock(&spinand_chip->lock);

    return 0;
}

static int sstar_spinand_read_user_prot_reg(struct mtd_info *mtd, loff_t from, size_t len, size_t *retlen, u_char *buf)
{
    unsigned int               start, length, end;
    struct sstar_spinand_chip *spinand_chip = mtd->priv;

    if (ERR_SPINAND_SUCCESS != mdrv_spinand_get_otp_layout(&spinand_chip->handle, &start, &length, 1))
        return -EIO;

    if (buf == NULL)
        return 0;

    end = start + length;
    if (from < start || from >= end)
    {
        *retlen = 0;
        return -EFAULT;
    }

    len = min_t(size_t, end - from, len);

    mutex_lock(&spinand_chip->lock);
    *retlen = mdrv_spinand_read_otp(&spinand_chip->handle, from / mtd->writesize, from % mtd->writesize, buf, len);
    mutex_unlock(&spinand_chip->lock);

    return 0;
}

static int sstar_spinand_write_user_prot_reg(struct mtd_info *mtd, loff_t to, size_t len, size_t *retlen, u_char *buf)
{
    unsigned int               start, length, end;
    struct sstar_spinand_chip *spinand_chip = mtd->priv;

    if (ERR_SPINAND_SUCCESS != mdrv_spinand_get_otp_layout(&spinand_chip->handle, &start, &length, 1))
        return -EIO;

    if (buf == NULL)
        return 0;

    end = start + length;
    if (to < start || to >= end)
    {
        *retlen = 0;
        return -EFAULT;
    }

    len = min_t(size_t, end - to, len);

    mutex_lock(&spinand_chip->lock);
    *retlen = mdrv_spinand_write_otp(&spinand_chip->handle, to / mtd->writesize, to % mtd->writesize, buf, len);
    mutex_unlock(&spinand_chip->lock);

    return 0;
}

static int sstar_spinand_lock_user_prot_reg(struct mtd_info *mtd, loff_t from, size_t len)
{
    int                        ret;
    unsigned int               start, length;
    struct sstar_spinand_chip *spinand_chip = mtd->priv;

    if (ERR_SPINAND_SUCCESS != mdrv_spinand_get_otp_layout(&spinand_chip->handle, &start, &length, 1))
        return -EIO;

    if (from != start || len != length)
    {
        return -EFAULT;
    }

    mutex_lock(&spinand_chip->lock);
    ret = mdrv_spinand_set_otp_lock(&spinand_chip->handle);
    mutex_unlock(&spinand_chip->lock);

    return (ret != ERR_SPINAND_SUCCESS) ? -EIO : ret;
}

static int sstar_spinand_attach_chip(struct nand_chip *chip)
{
    /*The nand_scan_ident will change the value of bits_per_cell,
    so we should change it back for the flow of nand_scan_tail*/
    if (chip)
        chip->base.memorg.bits_per_cell = 1;
    else
        return -EIO;

    return 0;
}

static const struct nand_controller_ops sstar_spinand_controller_ops = {
    .attach_chip = sstar_spinand_attach_chip,
};

static int sstar_spinand_probe(struct platform_device *pdev)
{
    u8  dev_id    = 0;
    int ret       = 0;
    u32 index     = 0;
    u32 cs_select = 0;
#if defined(CONFIG_FLASH_SOC_ECC)
    u32 fcie_interface = 0;
#endif
    u32                        flash_size   = 0;
    struct nand_flash_dev *    type         = NULL;
    struct mtd_info *          mtd          = NULL;
    struct sstar_spinand_chip *spinand_chip = NULL;
    flash_nand_info_t          st_flash_nand_info;

    do
    {
        ret = CamofPropertyReadU32(pdev->dev.of_node, "cs-select", &cs_select);
        if (ret)
        {
            break;
        }

        ret = CamofPropertyReadU32(pdev->dev.of_node, "engine", &index);
        if (ret)
        {
            break;
        }

        if (NULL
            == (spinand_chip = (struct sstar_spinand_chip *)kzalloc(sizeof(struct sstar_spinand_chip), GFP_KERNEL)))
        {
            break;
        }

        if (NULL == (spinand_chip->cis_mapping_buffer = kzalloc(CIS_PAGE_SIZE * CIS_MAPPING_CNT, GFP_KERNEL)))
        {
            break;
        }

        spinand_chip->handle.msg.cs_select = cs_select;
        spinand_chip->handle.ctrl_id       = spiflash_get_master(index);

        mtd       = nand_to_mtd(&spinand_chip->nand);
        mtd->priv = spinand_chip;

        // load sni by default
        mdrv_spinand_setup_by_default(&spinand_chip->handle);

#if defined(CONFIG_FLASH_SOC_ECC)
        if (!CamofPropertyReadU32(pdev->dev.of_node, "fcie-interface", &fcie_interface))
        {
            spinand_chip->handle.fcie_if = fcie_interface & 0xFF;
        }

        if (spinand_chip->handle.soc_ecc_en)
        {
            if (NULL == (spinand_chip->fcie_buffer = (u8 *)kzalloc(SSTAR_FCIE_ECC_BUFFER_SIZE, GFP_KERNEL)))
            {
                break;
            }
        }
#endif

        sstar_cis_init(spinand_chip->cis_mapping_buffer, CIS_MAPPING_CNT, spinand_chip->fcie_buffer, FCIE_ECC_PATH_MIU,
                       1);

        if (!sstar_cis_get_sni(&spinand_chip->handle))
        {
            printk("[FLASH] No sni!\r\n");
            break;
        }

        if (ERR_SPINAND_SUCCESS != mdrv_spinand_hardware_init(&spinand_chip->handle))
        {
            break;
        }

#if defined(CONFIG_FLASH_SOC_ECC)
        if (spinand_chip->handle.soc_ecc_en)
        {
            if (ERR_SPINAND_SUCCESS
                != mdrv_spinand_soc_ecc_init(&spinand_chip->handle, FCIE_ECC_PATH_MIU, 0, spinand_chip->fcie_buffer))
            {
                break;
            }
        }
#endif

        mdrv_spinand_info(&spinand_chip->handle, &st_flash_nand_info);

        flash_size = st_flash_nand_info.capacity;

        dev_id = 0xEE;

        for (type = nand_flash_ids; type->name != NULL; type++)
        {
            if (dev_id == type->dev_id)
            {
                printk("[FLASH] dev_id = 0x%x\r\n", type->dev_id);
                type->mfr_id = st_flash_nand_info.id[0];
                type->dev_id = st_flash_nand_info.id[1];
                type->id_len = st_flash_nand_info.id_byte_cnt;
                strncpy(type->id, st_flash_nand_info.id, st_flash_nand_info.id_byte_cnt);
                type->chipsize        = flash_size >> 20;
                type->pagesize        = st_flash_nand_info.page_size;
                type->oobsize         = st_flash_nand_info.oob_size;
                type->erasesize       = st_flash_nand_info.block_size;
                type->ecc.strength_ds = st_flash_nand_info.sector_size;
                type->ecc.step_ds     = st_flash_nand_info.page_size / st_flash_nand_info.sector_size;
                printk("[FLASH] mfr_id = 0x%x, dev_id= 0x%x id_len = 0x%x\r\n", type->id[0], type->id[1], type->id_len);
                break;
            }
        }

        if (NULL
            == (spinand_chip->bdma_buf =
                    (u8 *)kzalloc(st_flash_nand_info.page_size + st_flash_nand_info.oob_size, GFP_KERNEL)))
        {
            break;
        }

        mutex_init(&spinand_chip->lock);

        spinand_chip->nand.options = NAND_BROKEN_XD | NAND_SKIP_BBTSCAN | NAND_SUBPAGE_READ | NAND_NO_SUBPAGE_WRITE;
        spinand_chip->nand.legacy.read_byte = sstar_spinand_read_byte;
        // nand->read_word = spi_nand_read_word;
        spinand_chip->nand.legacy.read_buf             = sstar_spinand_read_buf;
        spinand_chip->nand.legacy.select_chip          = sstar_spinand_select_chip;
        spinand_chip->nand.legacy.cmd_ctrl             = sstar_spinand_cmd_ctrl;
        spinand_chip->nand.legacy.dev_ready            = sstar_spinand_dev_ready;
        spinand_chip->nand.legacy.cmdfunc              = sstar_spinand_cmdfunc;
        spinand_chip->nand.legacy.waitfunc             = sstar_spinand_waitfunc;
        spinand_chip->nand.legacy.dummy_controller.ops = &sstar_spinand_controller_ops;
        spinand_chip->nand.legacy.chip_delay           = 0;
        spinand_chip->nand.ecc.engine_type             = NAND_ECC_ENGINE_TYPE_ON_DIE;
        spinand_chip->nand.bbt_options                 = NAND_BBT_USE_FLASH;
        spinand_chip->nand.ecc.size                    = st_flash_nand_info.sector_size;
        spinand_chip->nand.ecc.steps                   = st_flash_nand_info.page_size / spinand_chip->nand.ecc.size;
        spinand_chip->nand.ecc.strength                = spinand_chip->nand.ecc.steps;
        spinand_chip->nand.ecc.hwctl                   = sstar_spinand_ecc_hwctl;
        spinand_chip->nand.ecc.calculate               = sstar_spinand_ecc_calculate;
        spinand_chip->nand.ecc.correct                 = sstar_spinand_ecc_correct;
        spinand_chip->nand.ecc.read_page_raw           = sstar_spinand_ecc_read_page_raw;
        spinand_chip->nand.ecc.write_page_raw          = sstar_spinand_ecc_write_page_raw;
        spinand_chip->nand.ecc.read_page               = sstar_spinand_ecc_read_page;
        spinand_chip->nand.ecc.read_subpage            = sstar_spinand_ecc_read_subpage;
        spinand_chip->nand.ecc.write_page              = sstar_spinand_ecc_write_page;
        spinand_chip->nand.ecc.write_subpage           = sstar_spinand_ecc_write_subpage;
        spinand_chip->nand.ecc.read_oob                = sstar_spinand_read_oob_std;
        spinand_chip->nand.ecc.write_oob               = sstar_spinand_write_oob_std;
        spinand_chip->nand.priv                        = NULL;

        if (!mdrv_spinand_is_support_otp(&spinand_chip->handle))
        {
            mtd->_get_fact_prot_info  = sstar_spinand_get_fact_prot_info;
            mtd->_read_fact_prot_reg  = sstar_spinand_read_fact_prot_reg;
            mtd->_get_user_prot_info  = sstar_spinand_get_user_prot_info;
            mtd->_read_user_prot_reg  = sstar_spinand_read_user_prot_reg;
            mtd->_write_user_prot_reg = sstar_spinand_write_user_prot_reg;
            mtd->_lock_user_prot_reg  = sstar_spinand_lock_user_prot_reg;
        }

        mtd->ooblayout         = &spinand_chip->nand_ooblayout;
        mtd->bitflip_threshold = 0xFF;

        if (mdrv_spinand_is_support_ubibbm(&spinand_chip->handle))
        {
            mtd->bitflip_threshold = 1;
        }

        mtd->name  = "nand0";
        mtd->owner = THIS_MODULE;

        if (0 != nand_scan(&spinand_chip->nand, 1))
        {
            break;
        }

        mtd->ooblayout = NULL;
        platform_set_drvdata(pdev, mtd);

        return mtd_device_register(mtd, NULL, 0);
    } while (0);

    if (spinand_chip && spinand_chip->bdma_buf)
    {
        kfree((void *)spinand_chip->bdma_buf);
    }

    if (spinand_chip && spinand_chip->fcie_buffer)
    {
        kfree((void *)spinand_chip->fcie_buffer);
    }

    if (spinand_chip && spinand_chip->cis_mapping_buffer)
    {
        kfree((void *)spinand_chip->cis_mapping_buffer);
    }

    if (spinand_chip)
    {
        kfree((void *)spinand_chip);
    }

    return -1;
}

static int sstar_spinand_remove(struct platform_device *pdev)
{
    struct sstar_spinand_chip *spinand_chip;
    struct mtd_info *          mtd;

    mtd          = platform_get_drvdata(pdev);
    spinand_chip = (struct sstar_spinand_chip *)mtd->priv;

    sstar_cis_deinit();

    mutex_destroy(&spinand_chip->lock);

    if (spinand_chip && spinand_chip->bdma_buf)
    {
        kfree((void *)spinand_chip->bdma_buf);
    }

    if (spinand_chip && spinand_chip->cis_mapping_buffer)
    {
        kfree((void *)spinand_chip->cis_mapping_buffer);
    }

    if (spinand_chip)
    {
        kfree((void *)spinand_chip);
    }

    platform_set_drvdata(pdev, NULL);
    return 0;
}

static void sstar_spinand_shutdown(struct platform_device *pdev)
{
    spi_nand_debug("%s:%d enter \n", __func__, __LINE__);
}

static int sstar_spinand_suspend(struct device *dev)
{
    spi_nand_debug("%s:%d enter \n", __func__, __LINE__);
    return 0;
}

static int sstar_spinand_resume(struct device *dev)
{
    struct sstar_spinand_chip *spinand_chip;
    struct mtd_info *          mtd;

    mtd          = dev_get_drvdata(dev);
    spinand_chip = (struct sstar_spinand_chip *)mtd->priv;

    mdrv_spinand_setup(&spinand_chip->handle);
    return 0;
}

static const struct of_device_id spinand_of_dt_ids[] = {{.compatible = "sstar-nandflash"}, {/* sentinel */}};

static const struct dev_pm_ops sstar_nandflash_pm_ops = {
    .resume  = sstar_spinand_resume,
    .suspend = sstar_spinand_suspend,
};

static struct platform_driver sstar_nandflash_driver = {
    .probe    = sstar_spinand_probe,
    .remove   = sstar_spinand_remove,
    .shutdown = sstar_spinand_shutdown,
    .driver =
        {
            .name           = "sstar-nandflash",
            .owner          = THIS_MODULE,
            .of_match_table = spinand_of_dt_ids,
            .pm             = &sstar_nandflash_pm_ops,
        },
};

module_platform_driver(sstar_nandflash_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("SSTAR");
MODULE_DESCRIPTION("Sstar MTD SPI NAND driver");
