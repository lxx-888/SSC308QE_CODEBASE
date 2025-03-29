/*
 * mtd_spinand.c- Sigmastar
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
#include <malloc.h>
#include <linux/err.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/nand_ecc.h>
#include <asm/io.h>
#include <clk.h>
#include <dm.h>
#include <dm/device-internal.h>
#include <flash_blk.h>
#include <nand.h>
#include <drv_flash_os_impl.h>
#include <mdrv_spinand.h>
#include <drv_spinand.h>

#if 1
#define spi_nand_msg(fmt, ...) printk(KERN_NOTICE "%s: " fmt "\n", __func__, ##__VA_ARGS__)
#define spi_nand_debug(fmt, ...)
#else
#define spi_nand_msg(fmt, ...)
#define spi_nand_debug(fmt, ...) printk(KERN_NOTICE "%s: " fmt "\n", __func__, ##__VA_ARGS__)
#endif

static struct nand_ecclayout nand_oob_256 = {.eccbytes = 0, .oobavail = 0};
struct command_ops
{
    u32 u32_command_ctrl;
    u32 u32_r_ptr;
    u32 u32_w_ptr;
    u32 u32_buf_size;
};

struct sstar_spinand_chip
{
    u8                    mtd_dev_name[8];
    struct command_ops    command_ops;
    struct spinand_handle handle;
    u8 *                  bdma_buf;
};

struct sstar_spinand_config
{
    u8 *cis_mapping_buffer;
    u8 *fcie_buffer;
    u8 *sni_list;
};

static struct sstar_spinand_config spinand_config = {
    .cis_mapping_buffer = NULL,
    .fcie_buffer        = NULL,
    .sni_list           = NULL,
};

static uint8_t spi_nand_read_byte(struct mtd_info *mtd)
{
    u8                         u8_byte;
    u8 *                       pu8_data;
    struct nand_chip *         nand         = mtd_to_nand(mtd);
    struct sstar_spinand_chip *spinand_chip = (struct sstar_spinand_chip *)nand->priv;
    struct command_ops *       ops          = &spinand_chip->command_ops;

    pu8_data = (u8 *)spinand_chip->bdma_buf;

    u8_byte = pu8_data[ops->u32_r_ptr];

    ops->u32_r_ptr += 1;

    if (ops->u32_r_ptr == ops->u32_w_ptr)
        ops->u32_r_ptr = 0;

    return u8_byte;
}

static u16 spi_nand_read_word(struct mtd_info *mtd)
{
    u16                        u16_word;
    u8 *                       pu8_data;
    struct nand_chip *         nand         = mtd_to_nand(mtd);
    struct sstar_spinand_chip *spinand_chip = (struct sstar_spinand_chip *)nand->priv;
    struct command_ops *       ops          = &spinand_chip->command_ops;

    pu8_data = (u8 *)spinand_chip->bdma_buf;

    u16_word = pu8_data[ops->u32_r_ptr];

    ops->u32_r_ptr += 1;

    if (ops->u32_r_ptr == ops->u32_w_ptr)
        ops->u32_r_ptr = 0;

    u16_word |= (pu8_data[ops->u32_r_ptr] << 8);

    ops->u32_r_ptr += 1;

    if (ops->u32_r_ptr == ops->u32_w_ptr)
        ops->u32_r_ptr = 0;

    return u16_word;
}

static void spi_nand_read_buf(struct mtd_info *mtd, uint8_t *buf, int len)
{
    u32                        u32_read_byte;
    struct nand_chip *         nand         = mtd_to_nand(mtd);
    struct sstar_spinand_chip *spinand_chip = (struct sstar_spinand_chip *)nand->priv;
    struct command_ops *       ops          = &spinand_chip->command_ops;

    while (len != 0)
    {
        u32_read_byte = (ops->u32_w_ptr - ops->u32_r_ptr);
        u32_read_byte = (u32_read_byte > len) ? len : u32_read_byte;

        memcpy((void *)buf, (const void *)(spinand_chip->bdma_buf + ops->u32_r_ptr), u32_read_byte);

        len -= u32_read_byte;
        buf += u32_read_byte;
        spinand_chip->command_ops.u32_r_ptr += u32_read_byte;

        if (ops->u32_r_ptr == ops->u32_w_ptr)
            ops->u32_r_ptr = 0;
    }
}

static void spi_nand_select_chip(struct mtd_info *mtd, int chip)
{
    spi_nand_debug("spi_nand_select_chip  Not support\r\n");
}

static void spi_nand_cmd_ctrl(struct mtd_info *mtd, int dat, unsigned int ctrl)
{
    spi_nand_debug("spi_nand_cmd_ctrl Not support\r\n");
}

static int spi_nand_dev_ready(struct mtd_info *mtd)
{
    spi_nand_debug("spi_nand_dev_ready Not support\r\n");

    return 1;
}

static void spi_nand_cmdfunc(struct mtd_info *mtd, unsigned command, int column, int page_addr)
{
    struct nand_chip *         nand         = mtd_to_nand(mtd);
    struct sstar_spinand_chip *spinand_chip = (struct sstar_spinand_chip *)nand->priv;

    spinand_chip->command_ops.u32_r_ptr = 0;

    switch (command)
    {
        case NAND_CMD_STATUS:
            spi_nand_debug("NAND_CMD_STATUS");
            spinand_chip->command_ops.u32_command_ctrl = NAND_CMD_STATUS;
            spinand_chip->command_ops.u32_w_ptr        = 1;
            mdrv_spinand_read_status(&spinand_chip->handle, (u8 *)spinand_chip->bdma_buf);
            break;

        case NAND_CMD_PAGEPROG:
            spi_nand_debug("NAND_CMD_PAGEPROG");
            nand->pagebuf = -1;
            break;

        case NAND_CMD_READOOB:
            spi_nand_debug("NAND_CMD_READOOB page_addr: 0x%x", page_addr);
            spinand_chip->command_ops.u32_command_ctrl = NAND_CMD_READOOB;
            spinand_chip->command_ops.u32_w_ptr        = 1;
            mdrv_spinand_page_read(&spinand_chip->handle, page_addr, (u16)(column + mtd->writesize),
                                   (u8 *)spinand_chip->bdma_buf, 1);
            break;

        case NAND_CMD_READID:
            spi_nand_debug("NAND_CMD_READID\r\n");
            spinand_chip->command_ops.u32_command_ctrl = NAND_CMD_READID;
            spinand_chip->command_ops.u32_w_ptr        = 6;
            mdrv_spinand_read_id(&spinand_chip->handle, (u8 *)spinand_chip->bdma_buf, 6);
            break;

        case NAND_CMD_ERASE2:
            spi_nand_debug("NAND_CMD_ERASE2\r\n");
            break;

        case NAND_CMD_ERASE1:
            spi_nand_debug("NAND_CMD_ERASE1, page_addr: 0x%x", page_addr);
            mdrv_spinand_block_erase(&spinand_chip->handle, page_addr);
            break;
        case NAND_CMD_READ0:
            break;
        case NAND_CMD_SEQIN:
            spi_nand_debug("NAND_CMD_SEQIN");
            nand->pagebuf = page_addr;
            break;
        case NAND_CMD_RESET:
            spi_nand_debug("NAND_CMD_RESET");
            mdrv_spinand_reset(&spinand_chip->handle);
            mdrv_spinand_setup_by_volatile(&spinand_chip->handle);
            break;
        case NAND_CMD_DO_ECC:
            spi_nand_debug("NAND_CMD_DO_ECC");
            if (column < 2)
            {
                mdrv_spinand_set_ecc_mode(&spinand_chip->handle, column);
                if (column == mdrv_spinand_get_ecc_mode(&spinand_chip->handle))
                {
                    printf("ecc mode set successfully, 0x%x!\r\n", column);
                }
                else
                {
                    printf("ecc mode set fail, 0x%x!\r\n", column);
                }
            }
            else if (column == 2)
            {
                if (!mdrv_spinand_get_ecc_status(&spinand_chip->handle))
                {
                    printf("page is clean!\r\n");
                }
                else
                {
                    printf("page is not clean!\r\n");
                }
            }
            break;
        default:
            spi_nand_msg("unsupported command %02Xh\n", command);
            break;
    }
    return;
}

static int spi_nand_waitfunc(struct mtd_info *mtd, struct nand_chip *this)
{
    u8                         u8_status;
    struct nand_chip *         nand         = mtd_to_nand(mtd);
    struct sstar_spinand_chip *spinand_chip = (struct sstar_spinand_chip *)nand->priv;

    spi_nand_debug("spi_nand_waitfunc\r\n");

    u8_status = mdrv_spinand_read_status(&spinand_chip->handle, NULL);

    return (ERR_SPINAND_E_FAIL > u8_status) ? NAND_STATUS_READY : NAND_STATUS_FAIL;
}

static void spi_nand_ecc_hwctl(struct mtd_info *mtd, int mode)
{
    spi_nand_debug(" spi_nand_ecc_hwctl Not support");
}

static int spi_nand_ecc_calculate(struct mtd_info *mtd, const uint8_t *dat, uint8_t *ecc_code)
{
    spi_nand_debug("spi_nand_ecc_calculate Not support");
    return 0;
}

static int spi_nand_ecc_correct(struct mtd_info *mtd, uint8_t *dat, uint8_t *read_ecc, uint8_t *calc_ecc)
{
    spi_nand_debug(" spi_nand_ecc_correct Not support");
    return 0;
}

static int spi_nand_ecc_read_page_raw(struct mtd_info *mtd, struct nand_chip *chip, uint8_t *buf, int oob_required,
                                      int page)
{
    u32                        u32_read_size;
    struct nand_chip *         nand         = mtd_to_nand(mtd);
    struct sstar_spinand_chip *spinand_chip = (struct sstar_spinand_chip *)nand->priv;

    u32_read_size = oob_required ? (mtd->writesize + mtd->oobsize) : mtd->writesize;

    if (ERR_SPINAND_TIMEOUT
        <= mdrv_spinand_page_read_raw(&spinand_chip->handle, page, 0, spinand_chip->bdma_buf, u32_read_size))
    {
        return -EIO;
    }

    memcpy((void *)buf, (const void *)spinand_chip->bdma_buf, mtd->writesize);

    if (oob_required)
    {
        memcpy((void *)(chip->oob_poi), (const void *)(spinand_chip->bdma_buf + mtd->writesize), mtd->oobsize);
    }

    return 0;
}

static int spi_nand_ecc_write_page_raw(struct mtd_info *mtd, struct nand_chip *chip, const uint8_t *buf,
                                       int oob_required, int page)
{
    u32                        u32_write_size;
    struct nand_chip *         nand         = mtd_to_nand(mtd);
    struct sstar_spinand_chip *spinand_chip = (struct sstar_spinand_chip *)nand->priv;

    spi_nand_debug("spi_nand_ecc_write_page_raw\r\n");

    u32_write_size = oob_required ? (mtd->writesize + mtd->oobsize) : mtd->writesize;

    memcpy((void *)spinand_chip->bdma_buf, (const void *)buf, mtd->writesize);

    if (oob_required)
    {
        memcpy((void *)(spinand_chip->bdma_buf + mtd->writesize), (const void *)(chip->oob_poi), mtd->oobsize);
    }

    if (ERR_SPINAND_SUCCESS
        != mdrv_spinand_page_program_raw(&spinand_chip->handle, page, 0, spinand_chip->bdma_buf, u32_write_size))
    {
        return -EIO;
    }

    return 0;
}

static int spi_nand_ecc_read_page(struct mtd_info *mtd, struct nand_chip *chip, uint8_t *buf, int oob_required,
                                  int page)
{
    u8                         u8_status = 0;
    u32                        u32_read_size;
    struct nand_chip *         nand         = mtd_to_nand(mtd);
    struct sstar_spinand_chip *spinand_chip = (struct sstar_spinand_chip *)nand->priv;

    u32_read_size = oob_required ? (mtd->writesize + mtd->oobsize) : mtd->writesize;

    u8_status = mdrv_spinand_page_read(&spinand_chip->handle, page, 0, spinand_chip->bdma_buf, u32_read_size);

    memcpy((void *)buf, (const void *)spinand_chip->bdma_buf, mtd->writesize);

    if (oob_required)
    {
        memcpy((void *)(chip->oob_poi), (const void *)(spinand_chip->bdma_buf + mtd->writesize), mtd->oobsize);
    }

    if (ERR_SPINAND_ECC_NOT_CORRECTED == u8_status)
        mtd->ecc_stats.failed++;
    else if (ERR_SPINAND_ECC_CORRECTED == u8_status)
        mtd->ecc_stats.corrected++;
    else if (ERR_SPINAND_TIMEOUT <= u8_status)
        return -EIO;

    return 0;
}

static int spi_nand_ecc_read_subpage(struct mtd_info *mtd, struct nand_chip *chip, uint32_t offs, uint32_t len,
                                     uint8_t *buf, int page)
{
    u8                         u8_status;
    u32                        u32_read_size;
    u32                        aligne_size  = 0;
    u32                        aligne_addr  = 0;
    struct nand_chip *         nand         = mtd_to_nand(mtd);
    struct sstar_spinand_chip *spinand_chip = (struct sstar_spinand_chip *)nand->priv;

    spi_nand_debug("page = 0x%x, offs = 0x%x, len = 0x%x", page, offs, len);

    aligne_size   = offs % CONFIG_FLASH_ADDR_ALIGN;
    aligne_addr   = offs - aligne_size;
    u32_read_size = len + aligne_size;

    u8_status =
        mdrv_spinand_page_read(&spinand_chip->handle, page, (u16)aligne_addr, spinand_chip->bdma_buf, u32_read_size);

    memcpy((void *)(buf + offs), (const void *)(spinand_chip->bdma_buf + aligne_size), len);

    if (ERR_SPINAND_ECC_NOT_CORRECTED == u8_status)
        mtd->ecc_stats.failed++;
    else if (ERR_SPINAND_ECC_CORRECTED == u8_status)
        mtd->ecc_stats.corrected++;
    else if (ERR_SPINAND_TIMEOUT <= u8_status)
        return -EIO;

    return 0;
}

static int spi_nand_ecc_write_subpage(struct mtd_info *mtd, struct nand_chip *chip, uint32_t offset, uint32_t data_len,
                                      const uint8_t *data_buf, int oob_required, int page)
{
    u32                        u32_write_size;
    struct nand_chip *         nand         = mtd_to_nand(mtd);
    struct sstar_spinand_chip *spinand_chip = (struct sstar_spinand_chip *)nand->priv;

    spi_nand_debug("spi_nand_ecc_write_subpage\r\n");

    u32_write_size = oob_required ? (data_len + mtd->oobsize) : data_len;

    memcpy((void *)spinand_chip->bdma_buf, (const void *)data_buf, data_len);

    if (oob_required)
    {
        memcpy((void *)(spinand_chip->bdma_buf + data_len), (const void *)(chip->oob_poi), mtd->oobsize);
    }

    if (ERR_SPINAND_SUCCESS
        != mdrv_spinand_page_program(&spinand_chip->handle, page, (u16)offset, spinand_chip->bdma_buf, u32_write_size))
    {
        return -EIO;
    }

    return 0;
}

static int spi_nand_ecc_write_page(struct mtd_info *mtd, struct nand_chip *chip, const uint8_t *buf, int oob_required,
                                   int page)
{
    u32                        u32_write_size;
    struct nand_chip *         nand         = mtd_to_nand(mtd);
    struct sstar_spinand_chip *spinand_chip = (struct sstar_spinand_chip *)nand->priv;

    spi_nand_debug("spi_nand_ecc_write_page\r\n");

    u32_write_size = oob_required ? (mtd->writesize + mtd->oobsize) : mtd->writesize;

    memcpy((void *)spinand_chip->bdma_buf, (const void *)buf, mtd->writesize);

    if (oob_required)
    {
        memcpy((void *)(spinand_chip->bdma_buf + mtd->writesize), (const void *)(chip->oob_poi), mtd->oobsize);
    }

    if (ERR_SPINAND_SUCCESS
        != mdrv_spinand_page_program(&spinand_chip->handle, page, 0, spinand_chip->bdma_buf, u32_write_size))
    {
        return -EIO;
    }

    return 0;
}

static int spi_nand_read_oob_std(struct mtd_info *mtd, struct nand_chip *chip, int page)
{
    struct nand_chip *         nand         = mtd_to_nand(mtd);
    struct sstar_spinand_chip *spinand_chip = (struct sstar_spinand_chip *)nand->priv;

    if (ERR_SPINAND_TIMEOUT <= mdrv_spinand_page_read(&spinand_chip->handle, page, (u16)(mtd->writesize),
                                                      spinand_chip->bdma_buf, mtd->oobsize))
    {
        return -EIO;
    }

    memcpy((void *)chip->oob_poi, (const void *)spinand_chip->bdma_buf, mtd->oobsize);

    return 0;
}

static int spi_nand_write_oob_std(struct mtd_info *mtd, struct nand_chip *chip, int page)
{
    struct nand_chip *         nand         = mtd_to_nand(mtd);
    struct sstar_spinand_chip *spinand_chip = (struct sstar_spinand_chip *)nand->priv;

    memcpy((void *)spinand_chip->bdma_buf, (const void *)chip->oob_poi, mtd->oobsize);

    if (ERR_SPINAND_SUCCESS
        != mdrv_spinand_page_program(&spinand_chip->handle, page, (u16)(mtd->writesize), spinand_chip->bdma_buf,
                                     mtd->oobsize))
    {
        return -EIO;
    }

    return 0;
}

int board_nand_init(struct nand_chip *nand)
{
    struct nand_flash_dev *    type         = NULL;
    struct sstar_spinand_chip *spinand_chip = NULL;
    struct spinand_init        init;
    flash_nand_info_t          st_flash_nand_info;
    u8                         bus = 0;
    u8                         cs  = 0;

    cs  = ((unsigned long)nand->IO_ADDR_R >> 0) & 0xFF;
    bus = ((unsigned long)nand->IO_ADDR_R >> 8) & 0xFF;

    do
    {
        if (NULL == (spinand_chip = (struct sstar_spinand_chip *)malloc(sizeof(struct sstar_spinand_chip))))
        {
            printf("nand init malloc fail!!\n");
            break;
        }

        memset((void *)spinand_chip, 0, sizeof(struct sstar_spinand_chip));

        if (!spinand_config.cis_mapping_buffer)
        {
            if (NULL
                == (spinand_config.cis_mapping_buffer =
                        (u8 *)memalign(ARCH_DMA_MINALIGN, CIS_PAGE_SIZE * CIS_MAPPING_CNT)))
            {
                break;
            }
        }

        spinand_chip->handle.ctrl_id       = spiflash_get_master(bus);
        spinand_chip->handle.msg.cs_select = cs;

        if (0xFF == spinand_chip->handle.ctrl_id)
            break;

        mdrv_spinand_setup_by_default(&spinand_chip->handle);

#if defined(CONFIG_FLASH_SOC_ECC)
        if (spinand_chip->handle.soc_ecc_en && !spinand_config.fcie_buffer)
        {
            if (NULL == (spinand_config.fcie_buffer = (u8 *)memalign(ARCH_DMA_MINALIGN, SSTAR_FCIE_ECC_BUFFER_SIZE)))
            {
                break;
            }
        }
#endif

        if (ERR_SPINAND_SUCCESS != mdrv_spinand_reset(&spinand_chip->handle))
        {
            printk("nand flash reset fail\r\n");
            break;
        }

        strcpy((char *)spinand_chip->mtd_dev_name, "nand0");
        spinand_chip->mtd_dev_name[4] = '0' + (bus << 1) + cs;

        if (!spinand_config.sni_list)
        {
            init.bus                  = bus;
            init.cs_select            = cs;
            init.dev_name             = spinand_chip->mtd_dev_name;
            init.cis_map              = spinand_config.cis_mapping_buffer;
            init.cis_cnt              = CIS_MAPPING_CNT;
            init.fcie_buf             = spinand_config.fcie_buffer;
            init.fcie_path            = FCIE_ECC_PATH_MIU;
            init.bypass_io            = 0;
            init.autok_parrtern_check = NULL;

            if (!sstar_spinand_init(&init))
            {
                break;
            }

            if (!sstar_cis_get_sni(&spinand_chip->handle))
            {
                printk("No find match sni!\r\n");
                break;
            }

            mdrv_spinand_setup_by_sni(&spinand_chip->handle);
        }
        else
        {
            if (!sstar_cis_get_sni_from_dram(&spinand_chip->handle, spinand_config.sni_list))
            {
                printk("No find match sni!\r\n");
                break;
            }

            if (mdrv_spinand_hardware_init(&spinand_chip->handle))
            {
                break;
            }
        }

#if defined(CONFIG_FLASH_SOC_ECC)
        if (spinand_chip->handle.soc_ecc_en)
        {
            if (ERR_SPINAND_SUCCESS
                != mdrv_spinand_soc_ecc_init(&spinand_chip->handle, FCIE_ECC_PATH_MIU, 0, spinand_config.fcie_buffer))
            {
                break;
            }

            mdrv_spinand_set_ecc_mode(&spinand_chip->handle, 0);
        }
#endif

        mdrv_spinand_info(&spinand_chip->handle, &st_flash_nand_info);

        if (NULL
            == (spinand_chip->bdma_buf =
                    (u8 *)memalign(ARCH_DMA_MINALIGN, st_flash_nand_info.page_size + st_flash_nand_info.oob_size)))
        {
            printf("nand init malloc fail!!\n");
            break;
        }

        for (type = nand_flash_ids; type->name != NULL; type++)
        {
            if (!strncmp(type->name, "nand", 4))
            {
                printf("[FLASH] dev_id = 0x%x\r\n", type->dev_id);
                type->mfr_id = st_flash_nand_info.id[0];
                type->dev_id = st_flash_nand_info.id[1];
                type->id_len = st_flash_nand_info.id_byte_cnt;
                strncpy((char *)type->id, (const char *)st_flash_nand_info.id, st_flash_nand_info.id_byte_cnt);
                type->chipsize        = st_flash_nand_info.capacity >> 20;
                type->pagesize        = st_flash_nand_info.page_size;
                type->oobsize         = st_flash_nand_info.oob_size;
                type->erasesize       = st_flash_nand_info.block_size;
                type->ecc.strength_ds = st_flash_nand_info.sector_size;
                type->ecc.step_ds     = st_flash_nand_info.page_size / st_flash_nand_info.sector_size;
                printf("[FLASH] mfr_id = 0x%x, dev_id= 0x%x id_len = 0x%x\r\n", type->id[0], type->id[1], type->id_len);
                break;
            }
        }

        nand->mtd.name  = spinand_chip->mtd_dev_name;
        nand->priv      = spinand_chip;
        nand->options   = NAND_BROKEN_XD | NAND_SUBPAGE_READ | NAND_SKIP_BBTSCAN | NAND_NO_SUBPAGE_WRITE;
        nand->read_byte = spi_nand_read_byte;
        nand->read_word = spi_nand_read_word;
        nand->read_buf  = spi_nand_read_buf;

        nand->select_chip = spi_nand_select_chip;
        nand->cmd_ctrl    = spi_nand_cmd_ctrl;
        nand->dev_ready   = spi_nand_dev_ready;
        nand->cmdfunc     = spi_nand_cmdfunc;
        nand->waitfunc    = spi_nand_waitfunc;

        nand->bits_per_cell      = 1;
        nand->chip_delay         = 0;
        nand->bbt_options        = NAND_BBT_USE_FLASH;
        nand->ecc.mode           = NAND_ECC_HW;
        nand->ecc.size           = st_flash_nand_info.sector_size;
        nand->ecc.steps          = st_flash_nand_info.page_size / nand->ecc.size;
        nand->ecc.strength       = nand->ecc.steps;
        nand->ecc.hwctl          = spi_nand_ecc_hwctl;
        nand->ecc.calculate      = spi_nand_ecc_calculate;
        nand->ecc.correct        = spi_nand_ecc_correct;
        nand->ecc.read_page_raw  = spi_nand_ecc_read_page_raw;
        nand->ecc.write_page_raw = spi_nand_ecc_write_page_raw;
        nand->ecc.read_page      = spi_nand_ecc_read_page;
        nand->ecc.read_subpage   = spi_nand_ecc_read_subpage;
        nand->ecc.write_page     = spi_nand_ecc_write_page;
        nand->ecc.write_subpage  = spi_nand_ecc_write_subpage;
        nand->ecc.read_oob       = spi_nand_read_oob_std;
        nand->ecc.write_oob      = spi_nand_write_oob_std;
        if (st_flash_nand_info.oob_size == 256)
            nand->ecc.layout = &nand_oob_256;

        return 0;
    } while (0);

    if (spinand_chip && spinand_chip->bdma_buf)
    {
        kfree((void *)spinand_chip->bdma_buf);
    }

    if (spinand_chip)
    {
        free((void *)spinand_chip);
    }

    return -1;
}

int spi_nand_probe(unsigned long column)
{
    struct mtd_info *          mtd          = NULL;
    struct nand_chip *         nand         = NULL;
    struct sstar_spinand_chip *spinand_chip = NULL;

    if (!column)
        return 0;

    spinand_config.sni_list = (u8 *)column;

    if (nand_curr_device != -1)
    {
        mtd          = get_nand_dev_by_index(nand_curr_device);
        nand         = mtd_to_nand(mtd);
        spinand_chip = (struct sstar_spinand_chip *)nand->priv;

        del_mtd_device(mtd);
    }

    if (spinand_chip && spinand_chip->bdma_buf)
    {
        kfree((void *)spinand_chip->bdma_buf);
    }

    if (spinand_chip)
    {
        free((void *)spinand_chip);
    }

    nand_init();

    return 0;
}

#if defined(CONFIG_SSTAR_FLASH_DM)
static int spi_nand_bread(struct udevice *udev, u32 start, u32 blkcnt, void *p_data)
{
    int                      ret;
    u32                      addr;
    size_t                   read_size;
    struct sstar_flash_info *priv    = dev_get_priv(udev);
    struct mtd_info *        mtd     = priv->mtd;
    u32                      blksize = mtd->erasesize;

    addr      = start * blksize;
    read_size = blkcnt * blksize;

    ret = nand_read_skip_bad(mtd, addr, &read_size, NULL, mtd->size, (u_char *)p_data);
    if (unlikely(ret != 0))
    {
        return ret;
    }

    return blkcnt;
}

static int spi_nand_bwrite(struct udevice *udev, u32 start, u32 blkcnt, const void *p_data)
{
    int                      ret;
    u32                      addr;
    size_t                   rwsize;
    struct sstar_flash_info *priv    = dev_get_priv(udev);
    struct mtd_info *        mtd     = priv->mtd;
    u32                      blksize = mtd->erasesize;

    addr   = start * blksize;
    rwsize = blkcnt * blksize;

    ret = nand_write_skip_bad(mtd, addr, &rwsize, NULL, mtd->size, (u_char *)p_data, WITH_WR_VERIFY);
    if (unlikely(ret != 0))
    {
        return ret;
    }

    return blkcnt;
}

static int spi_nand_berase(struct udevice *udev, u32 start, u32 blkcnt)
{
    int                      ret;
    u32                      addr;
    u32                      len;
    nand_erase_options_t     opts;
    struct sstar_flash_info *priv    = dev_get_priv(udev);
    struct mtd_info *        mtd     = priv->mtd;
    u32                      blksize = mtd->erasesize;

    addr = start * blksize;
    len  = blkcnt * blksize;

    memset(&opts, 0, sizeof(opts));
    opts.offset = addr;
    opts.length = len;

    ret = nand_erase_opts(mtd, &opts);
    if (unlikely(ret != 0))
    {
        return ret;
    }

    return blkcnt;
}

static int spi_nand_dm_bind(struct udevice *udev)
{
    u32             bus = 0;
    u32             cs  = 0;
    struct udevice *bdev;
    int             ret = 0;

    bus = dev_read_u32_default(udev, "engine", 0);
    cs  = dev_read_u32_default(udev, "cs-select", 0);
    bus &= 0xFF;
    cs &= 0xFF;

    ret = blk_create_devicef(udev, "sstar_flash_blk", "spinand.blk", IF_TYPE_SPINAND, (bus << 1) + cs, 512, 0, &bdev);
    if (ret)
    {
        debug("Cannot create block device\n");
        return ret;
    }

    return 0;
}

static int spi_nand_dm_probe(struct udevice *udev)
{
    u32                        bus = 0;
    u32                        cs  = 0;
    flash_nand_info_t          st_flash_nand_info;
    struct sstar_flash_info *  priv         = dev_get_priv(udev);
    struct nand_chip *         nand         = NULL;
    struct sstar_spinand_chip *spinand_chip = NULL;

    bus = dev_read_u32_default(udev, "engine", 0);
    cs  = dev_read_u32_default(udev, "cs-select", 0);
    bus &= 0xFF;
    cs &= 0xFF;

    do
    {
        if (NULL == (nand = (struct nand_chip *)malloc(sizeof(struct nand_chip))))
        {
            printf("spi_nand_dm_probe malloc fail !!!\n");
            break;
        }

        memset((void *)nand, 0, sizeof(struct nand_chip));

        nand->IO_ADDR_R = nand->IO_ADDR_W = (void __iomem *)((bus << 8) & cs);
        spinand_config.sni_list           = NULL;

        if (board_nand_init(nand))
            break;

        if (nand_scan(&nand->mtd, CONFIG_SYS_NAND_MAX_CHIPS))
            break;

        add_mtd_device(&nand->mtd);

        spinand_chip = (struct sstar_spinand_chip *)nand->priv;

        mdrv_spinand_info(&spinand_chip->handle, &st_flash_nand_info);

        priv->flash_con_type = IF_TYPE_SPINAND;
        priv->mtd            = nand_to_mtd(nand);
        priv->density        = st_flash_nand_info.capacity;
        priv->read           = spi_nand_bread;
        priv->write          = spi_nand_bwrite;
        priv->erase          = spi_nand_berase;

        debug("%s probe success\n", __func__);

        return 0;
    } while (0);

    if (nand)
    {
        free((void *)nand);
    }

    return -1;
}

static const struct udevice_id nandflash_blk_ids[] = {{.compatible = "sstar,nandflash"}, {}};

U_BOOT_DRIVER(nandflash) = {
    .name      = "nandflash",
    .id        = UCLASS_SPI_FLASH,
    .of_match  = nandflash_blk_ids,
    .bind      = spi_nand_dm_bind,
    .probe     = spi_nand_dm_probe,
    .priv_auto = sizeof(struct sstar_flash_info),
};

#endif /* CONFIG_SSTAR_FLASH_DM */
