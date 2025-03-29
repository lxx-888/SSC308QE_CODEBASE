/*
 * drv_spinor.c- Sigmastar
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

#include <drv_flash_os_impl.h>
#include <mdrv_spinor.h>
#include <drv_spinor.h>
#include <drv_part.h>

#define SPINOR_CHIP_MAX_CNT 4

struct nor_chip
{
    u8                       store_cnt;
    struct spinor_handle     handle[SPINOR_CHIP_MAX_CNT];
    struct sstar_part_device part_dev[SPINOR_CHIP_MAX_CNT];
};

static struct nor_chip sstar_nor_config = {
    .store_cnt = 0,
};

static u32 sstar_spinor_read(void *handle, u32 u32_offset, u32 u32_size, u32 u32_limit, u8 *pu8_data)
{
    if (u32_limit < u32_size)
        return 0;

    if (ERR_SPINOR_SUCCESS != mdrv_spinor_read(handle, u32_offset, pu8_data, u32_size))
    {
        return 0;
    }

    return u32_size;
}

#if defined(CONFIG_FLASH_XZDEC)
u32 sstar_spinor_read_to_xzdec(void *handle, u32 u32_offset, u32 u32_size, u32 u32_limit, u8 *pu8_data)
{
    if (u32_limit < u32_size)
        return 0;

    if (ERR_SPINOR_SUCCESS != mdrv_spinor_read_to_xzdec(handle, u32_offset, pu8_data, u32_size))
    {
        return 0;
    }

    return u32_size;
}
#endif

static u32 sstar_spinor_write(void *handle, u32 u32_offset, u32 u32_size, u32 u32_limit, u8 *pu8_data)
{
    if (u32_limit < u32_size)
        return 0;

    if (ERR_SPINOR_SUCCESS != mdrv_spinor_program(handle, u32_offset, pu8_data, u32_size))
    {
        return 0;
    }

    return u32_size;
}

static u32 sstar_spinor_erase(void *handle, u32 u32_offset, u32 u32_size, u32 u32_limit)
{
    if (u32_limit < u32_size)
        return 0;

    if (ERR_SPINOR_SUCCESS != mdrv_spinor_erase(handle, u32_offset, u32_size))
    {
        return 0;
    }

    return u32_size;
}

u8 sstar_spinor_init(struct spinor_init *init)
{
    u8                        index    = 0;
    struct spinor_handle *    handle   = NULL;
    struct sstar_part_device *part_dev = NULL;
    flash_nor_info_t          info;

    for (index = 0; index < sstar_nor_config.store_cnt; index++)
    {
        handle = &sstar_nor_config.handle[index];

        if ((handle->ctrl_id == spiflash_get_master(init->bus)) && (handle->msg.cs_select == init->cs_select))
            break;
    }

    if (sstar_nor_config.store_cnt >= SPINOR_CHIP_MAX_CNT)
        return 0;

    handle   = &sstar_nor_config.handle[index];
    part_dev = &sstar_nor_config.part_dev[index];

    handle->msg.cs_select = init->cs_select;
    handle->ctrl_id       = spiflash_get_master(init->bus);

    if (0xFF == handle->ctrl_id)
        return 0;

    mdrv_spinor_setup_by_default(handle);

    if (!init->bypass_io && ERR_SPINOR_SUCCESS != mdrv_spinor_reset(handle))
    {
        flash_impl_printf("[FLASH] reset fail\r\n");
        return 0;
    }

    sstar_cis_init(init->cis_map, init->cis_cnt, NULL, 0, !init->bypass_io);

    if (!sstar_cis_get_nri(handle))
    {
        flash_impl_printf("[FLASH] No find match nri!\r\n");
        return 0;
    }

    if (init->bypass_io)
    {
        if (mdrv_spinor_setup_by_nri(handle))
        {
            return 0;
        }
    }
    else
    {
        if (mdrv_spinor_hardware_init(handle))
        {
            return 0;
        }
    }

    if (init->dev_name)
    {
        flash_impl_memcpy(sstar_nor_config.part_dev[sstar_nor_config.store_cnt].dev_name, (const u8 *)init->dev_name,
                          5);
        sstar_nor_config.part_dev[sstar_nor_config.store_cnt].dev_name[7] = 0;
    }

    mdrv_spinor_info(handle, &info);

    part_dev->engine        = init->bus;
    part_dev->cs_select     = init->cs_select;
    part_dev->capacity      = info.capacity;
    part_dev->erase_size    = info.sector_size;
    part_dev->handle        = (void *)handle;
    part_dev->read_skip_bad = sstar_spinor_read;
#if defined(CONFIG_FLASH_XZDEC)
    part_dev->xzdec_en               = handle->msg.bdma_en;
    part_dev->read_to_xzdec_skip_bad = sstar_spinor_read_to_xzdec;
#else
    part_dev->xzdec_en               = 0;
    part_dev->read_to_xzdec_skip_bad = NULL;
#endif
    part_dev->write_skip_bad = sstar_spinor_write;
    part_dev->erase_skip_bad = sstar_spinor_erase;

    if (!sstar_part_register(part_dev))
        return 0;

    if (index == sstar_nor_config.store_cnt)
        sstar_nor_config.store_cnt++;

    return 1;
}
