/*
 * drv_spinand.c- Sigmastar
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

#include <drv_flash_os_impl.h>
#include <mdrv_spinand.h>
#include <drv_spinand.h>
#include <drv_part.h>

#define SPINAND_CHIP_MAX_CNT 4

struct nand_chip
{
    u8                       store_cnt;
    flash_nand_info_t        info;
    struct spinand_handle    handle[SPINAND_CHIP_MAX_CNT];
    struct sstar_part_device part_dev[SPINAND_CHIP_MAX_CNT];
};

static struct nand_chip sstar_nand_config = {
    .store_cnt = 0,
};

#define FLASH_PAGES_PER_BLOCK      (sstar_nand_config.info.blk_page_cnt)
#define FLASH_PAGES_PER_BLOCK_MASK (sstar_nand_config.info.blk_page_cnt - 1)
#define FLASH_PAGE_SIZE            (sstar_nand_config.info.page_size)
#define FLASH_BLOCK_SIZE           (sstar_nand_config.info.block_size)
#define FLASH_PAGE_SIZE_MASK       (sstar_nand_config.info.page_size - 1)
#define FLASH_BLOCK_SIZE_MASK      (sstar_nand_config.info.block_size - 1)

static u32 sstar_spinand_get_bytes_left(u32 u32_bytes, u32 u32_limit)
{
    if (u32_limit > u32_bytes)
    {
        return u32_bytes;
    }

    return u32_limit;
}

static u32 sstar_spinand_get_page_size_left(u32 u32_offset)
{
    return (FLASH_PAGE_SIZE - (u32_offset & FLASH_PAGE_SIZE_MASK));
}

static u32 sstar_spinand_get_block_size_left(u32 u32_page)
{
    return (FLASH_BLOCK_SIZE - ((u32_page & FLASH_PAGES_PER_BLOCK_MASK) * FLASH_PAGE_SIZE));
}

static u32 sstar_spinand_align_block(u32 u32_page)
{
    return (u32_page & ~FLASH_PAGES_PER_BLOCK_MASK);
}

static u32 sstar_spinand_align_next_block(u32 u32_page)
{
    return ((u32_page + FLASH_PAGES_PER_BLOCK) & ~FLASH_PAGES_PER_BLOCK_MASK);
}

static u32 sstar_spinand_offset_to_align_page_size(u32 u32_offset)
{
    return ((u32_offset + FLASH_PAGE_SIZE_MASK) & ~FLASH_PAGE_SIZE_MASK);
}

static u32 sstar_spinand_offset_to_page_address(u32 u32_offset)
{
    return ((u32_offset & ~FLASH_PAGE_SIZE_MASK) >> flash_impl_count_bits(FLASH_PAGE_SIZE));
}

static u32 sstar_spinand_read_skip_bad(void *handle, u32 u32_offset, u32 u32_size, u32 u32_limit, u8 *pu8_data)
{
    u32 u32_bytes_read;
    u32 u32_page;
    u32 u32_page_end;
    u32 u32_page_offset;
    u32 u32_bytes_left;

    mdrv_spinand_info(handle, &sstar_nand_config.info);

    u32_page = sstar_spinand_offset_to_page_address(u32_offset);
    u32_page_end =
        sstar_spinand_offset_to_page_address(sstar_spinand_offset_to_align_page_size(u32_offset + u32_limit));
    u32_bytes_read = 0;
    u32_bytes_left = u32_size;

    if (u32_offset & FLASH_PAGE_SIZE_MASK)
    {
        u32_bytes_read = sstar_spinand_get_bytes_left(u32_bytes_left, sstar_spinand_get_page_size_left(u32_offset));

        while (mdrv_spinand_block_isbad(handle, sstar_spinand_align_block(u32_page)))
        {
            u32_page += FLASH_PAGES_PER_BLOCK;

            if (u32_page_end < u32_page)
            {
                return 0;
            }
        }

        if (ERR_SPINAND_ECC_NOT_CORRECTED <= mdrv_spinand_page_read(
                handle, u32_page++, (u16)(u32_offset & FLASH_PAGE_SIZE_MASK), pu8_data, u32_bytes_read))
        {
            return 0;
        }

        pu8_data += u32_bytes_read;
        u32_bytes_left -= u32_bytes_read;
    }

    if (0 != u32_bytes_left)
    {
        u32_page_offset = u32_page;
        u32_bytes_read  = 0;

        while (u32_page_end > u32_page_offset)
        {
            if ((u32_bytes_left == u32_bytes_read)
                || mdrv_spinand_block_isbad(handle, sstar_spinand_align_block(u32_page_offset)))
            {
                if (0 != u32_bytes_read)
                {
                    if (u32_bytes_read != mdrv_spinand_pages_read(handle, u32_page, pu8_data, u32_bytes_read))
                    {
                        break;
                    }

                    u32_bytes_left -= u32_bytes_read;
                    pu8_data += u32_bytes_read;
                    u32_bytes_read = 0;
                }

                if (0 == u32_bytes_left)
                {
                    break;
                }

                u32_page = sstar_spinand_align_next_block(u32_page_offset);
            }
            else
            {
                u32_bytes_read += sstar_spinand_get_bytes_left(u32_bytes_left - u32_bytes_read,
                                                               sstar_spinand_get_block_size_left(u32_page_offset));
            }

            if (u32_bytes_left != u32_bytes_read)
            {
                u32_page_offset = sstar_spinand_align_next_block(u32_page_offset);
            }
        }
    }

    u32_size -= u32_bytes_left;

    return u32_size;
}

#if defined(CONFIG_FLASH_XZDEC)
static u32 sstar_spinand_read_to_xzdec_skip_bad(void *handle, u32 u32_offset, u32 u32_size, u32 u32_limit, u8 *pu8_data)
{
    u32 u32_bytes_read;
    u32 u32_page;
    u32 u32_page_end;
    u32 u32_page_offset;
    u32 u32_bytes_left;

    mdrv_spinand_info(handle, &sstar_nand_config.info);

    u32_page = sstar_spinand_offset_to_page_address(u32_offset);
    u32_page_end =
        sstar_spinand_offset_to_page_address(sstar_spinand_offset_to_align_page_size(u32_offset + u32_limit));
    u32_bytes_read = 0;
    u32_bytes_left = u32_size;

    if (u32_offset & FLASH_PAGE_SIZE_MASK)
    {
        u32_bytes_read = sstar_spinand_get_bytes_left(u32_bytes_left, sstar_spinand_get_page_size_left(u32_offset));

        while (mdrv_spinand_block_isbad(handle, sstar_spinand_align_block(u32_page)))
        {
            u32_page += FLASH_PAGES_PER_BLOCK;

            if (u32_page_end < u32_page)
            {
                return 0;
            }
        }

        if (ERR_SPINAND_ECC_NOT_CORRECTED <= mdrv_spinand_page_read_to_xzdec(
                handle, u32_page++, (u16)(u32_offset & FLASH_PAGE_SIZE_MASK), pu8_data, u32_bytes_read))
        {
            return 0;
        }

        pu8_data += u32_bytes_read;
        u32_bytes_left -= u32_bytes_read;
    }

    if (0 != u32_bytes_left)
    {
        u32_page_offset = u32_page;
        u32_bytes_read  = 0;

        while (u32_page_end > u32_page_offset)
        {
            if (u32_bytes_left == u32_bytes_read
                || mdrv_spinand_block_isbad(handle, sstar_spinand_align_block(u32_page_offset)))
            {
                if (0 != u32_bytes_read)
                {
                    if (u32_bytes_read != mdrv_spinand_pages_read_to_xzdec(handle, u32_page, pu8_data, u32_bytes_read))
                    {
                        break;
                    }

                    u32_bytes_left -= u32_bytes_read;
                    pu8_data += u32_bytes_read;
                    u32_bytes_read = 0;
                }

                if (0 == u32_bytes_left)
                {
                    break;
                }

                u32_page = sstar_spinand_align_next_block(u32_page_offset);
            }
            else
            {
                u32_bytes_read += sstar_spinand_get_bytes_left(u32_bytes_left - u32_bytes_read,
                                                               sstar_spinand_get_block_size_left(u32_page_offset));
            }

            if (u32_bytes_left != u32_bytes_read)
            {
                u32_page_offset = sstar_spinand_align_next_block(u32_page_offset);
            }
        }
    }

    u32_size -= u32_bytes_left;

    return u32_size;
}
#endif

static u32 sstar_spinand_write_skip_bad(void *handle, u32 u32_offset, u32 u32_size, u32 u32_limit, u8 *pu8_data)
{
    u32 block_page_start = 0;
    u32 block_offset     = 0;
    u32 page_start       = 0;
    u32 page_offset      = 0;
    u32 write_size       = 0;
    u32 left_to_write    = u32_size;

    mdrv_spinand_info(handle, &sstar_nand_config.info);

    while (left_to_write > 0)
    {
        block_page_start = sstar_spinand_align_block(sstar_spinand_offset_to_page_address(u32_offset));
        block_offset     = u32_offset & FLASH_BLOCK_SIZE_MASK;
        page_start       = u32_offset & ~FLASH_PAGE_SIZE_MASK;

        if (mdrv_spinand_block_isbad(handle, block_page_start))
        {
            u32_offset += (FLASH_BLOCK_SIZE - block_offset);
            u32_limit -= (FLASH_BLOCK_SIZE - block_offset);

            if (left_to_write > u32_limit)
            {
                u32_size -= left_to_write;
                return u32_size;
            }

            continue;
        }

        while (page_start < (block_page_start + FLASH_PAGES_PER_BLOCK))
        {
            page_offset = u32_offset & FLASH_PAGE_SIZE_MASK;

            if (left_to_write < (FLASH_PAGE_SIZE - page_offset))
                write_size = left_to_write;
            else
                write_size = FLASH_PAGE_SIZE - page_offset;

            if (ERR_SPINAND_SUCCESS != mdrv_spinand_page_program(handle, page_start, page_offset, pu8_data, write_size))
            {
                u32_size -= left_to_write;
                return u32_size;
            }

            u32_offset += write_size;
            pu8_data += write_size;
            page_start += 1;
            left_to_write -= write_size;
        }
    }

    return u32_size;
}

static u32 sstar_spinand_erase_skip_bad(void *handle, u32 u32_offset, u32 u32_size, u32 u32_limit)
{
    u32 block_page_start = 0;
    u32 left_to_erase    = u32_size;

    mdrv_spinand_info(handle, &sstar_nand_config.info);

    if ((u32_offset & FLASH_BLOCK_SIZE_MASK) || (u32_size & FLASH_BLOCK_SIZE_MASK))
        return 0;

    while (left_to_erase > 0)
    {
        block_page_start = sstar_spinand_align_block(sstar_spinand_offset_to_page_address(u32_offset));

        if (mdrv_spinand_block_isbad(handle, block_page_start))
        {
            u32_offset += FLASH_BLOCK_SIZE;
            u32_limit -= FLASH_BLOCK_SIZE;

            if (left_to_erase > u32_limit)
            {
                break;
            }

            continue;
        }

        if (ERR_SPINAND_SUCCESS != mdrv_spinand_block_erase(handle, block_page_start))
        {
            break;
        }

        u32_offset += FLASH_BLOCK_SIZE;
        u32_limit -= FLASH_BLOCK_SIZE;
        left_to_erase -= FLASH_BLOCK_SIZE;
    }

    u32_size -= left_to_erase;

    return u32_size;
}

u8 sstar_spinand_init(struct spinand_init *init)
{
    u8                        index;
    struct spinand_handle *   handle   = NULL;
    struct sstar_part_device *part_dev = NULL;
    flash_nand_info_t         info;

    for (index = 0; index < sstar_nand_config.store_cnt; index++)
    {
        handle = &sstar_nand_config.handle[index];

        if ((handle->ctrl_id == spiflash_get_master(init->bus)) && (handle->msg.cs_select == init->cs_select))
            break;
    }

    if (sstar_nand_config.store_cnt >= SPINAND_CHIP_MAX_CNT)
        return 0;

    handle   = &sstar_nand_config.handle[index];
    part_dev = &sstar_nand_config.part_dev[index];

    handle->msg.cs_select = init->cs_select;
    handle->ctrl_id       = spiflash_get_master(init->bus);

    if (0xFF == handle->ctrl_id)
        return 0;

    mdrv_spinand_setup_by_default(handle);

    sstar_cis_init(init->cis_map, init->cis_cnt, init->fcie_buf, init->fcie_path, !init->bypass_io);

    if (!sstar_cis_get_sni(handle))
    {
        flash_impl_printf("[FLASH] No find match sni!\r\n");
        return 0;
    }

    if (init->bypass_io)
    {
        if (mdrv_spinand_setup_by_sni(handle))
            return 0;
    }
    else
    {
        if (mdrv_spinand_hardware_init(handle))
            return 0;
    }

    mdrv_spinand_info(handle, &info);

    if (init->dev_name)
    {
        flash_impl_memcpy(sstar_nand_config.part_dev[sstar_nand_config.store_cnt].dev_name, (const u8 *)init->dev_name,
                          6);
        sstar_nand_config.part_dev[sstar_nand_config.store_cnt].dev_name[7] = 0;
    }

    part_dev->engine        = init->bus;
    part_dev->cs_select     = init->cs_select;
    part_dev->capacity      = info.capacity;
    part_dev->erase_size    = info.block_size;
    part_dev->handle        = (void *)handle;
    part_dev->read_skip_bad = sstar_spinand_read_skip_bad;
#if defined(CONFIG_FLASH_XZDEC)
    part_dev->xzdec_en               = handle->msg.bdma_en && (!handle->soc_ecc_en);
    part_dev->read_to_xzdec_skip_bad = sstar_spinand_read_to_xzdec_skip_bad;
#else
    part_dev->xzdec_en               = 0;
    part_dev->read_to_xzdec_skip_bad = NULL;
#endif
    part_dev->write_skip_bad = sstar_spinand_write_skip_bad;
    part_dev->erase_skip_bad = sstar_spinand_erase_skip_bad;

    if (!sstar_part_register(part_dev))
        return 0;

    if (index == sstar_nand_config.store_cnt)
        sstar_nand_config.store_cnt++;

    return 1;
}
