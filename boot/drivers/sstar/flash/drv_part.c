/*
 * drv_part.c- Sigmastar
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
#include <drv_part.h>
#include <cis.h>

#define PART_DEVICE_MAX_CNT 4

struct sstar_part_config
{
    u8                        part_dev_store;
    struct parts_tbl *        tbl;
    struct sstar_part_device *part_dev[PART_DEVICE_MAX_CNT];
};

static struct sstar_part_config part_config = {.part_dev_store = 0, .tbl = NULL};

static void sstar_part_pni_init(void)
{
    if (part_config.tbl)
        return;

    part_config.tbl = sstar_cis_get_pni();
}

u8 sstar_part_mark_active(u8 *part_name, u8 trunk)
{
    u32                i        = 0;
    u32                u32Count = 0;
    struct parts_info *info     = NULL;

    sstar_part_pni_init();

    if (!part_config.tbl)
    {
        return 0;
    }

    info     = (struct parts_info *)(((u8 *)part_config.tbl) + sizeof(struct parts_tbl));
    u32Count = part_config.tbl->size / sizeof(struct parts_info);

    for (i = 0; i < u32Count; i++)
    {
        if (!flash_impl_strcmp(info->part_name, part_name))
        {
            info->active = 0;

            if (trunk == info->trunk)
            {
                info->active = 1;
            }
        }
        info++;
    }

    return sstar_cis_update_pni((u8 *)part_config.tbl);
}

u8 sstar_part_get(u8 *part_name, u8 trunk, struct sstar_part *part)
{
    u8                 u8_i;
    u32                u32Count = 0;
    struct parts_info *info     = NULL;

    sstar_part_pni_init();

    if (!part_config.tbl || !part)
    {
        return 0;
    }

    info     = (struct parts_info *)(((u8 *)part_config.tbl) + sizeof(struct parts_tbl));
    u32Count = part_config.tbl->size / sizeof(struct parts_info);

    for (u8_i = 0; u8_i < u32Count; u8_i++)
    {
        if (!flash_impl_strcmp(info->part_name, part_name) && (trunk == info->trunk))
        {
            part->trunk        = info->trunk;
            part->backup_trunk = info->backup_trunk;
            part->offset       = info->offset;
            part->size         = info->size;
            part->part_name    = info->part_name;
            break;
        }
        info++;
    }

    if (u8_i == u32Count)
    {
        return 0;
    }

    for (u8_i = 0; u8_i < part_config.part_dev_store; u8_i++)
    {
        if ((part_config.part_dev[u8_i]->engine == info->engine)
            && (part_config.part_dev[u8_i]->cs_select == info->cs_select))
        {
            part->part_dev = part_config.part_dev[u8_i];
            return 1;
        }
    }

    return 0;
}

u8 sstar_part_get_active(u8 *part_name, struct sstar_part *part)
{
    u8                 u8_i;
    u32                u32Count = 0;
    struct parts_info *info     = NULL;

    sstar_part_pni_init();

    if (!part_config.tbl || !part)
    {
        return 0;
    }

    info     = (struct parts_info *)(((u8 *)part_config.tbl) + sizeof(struct parts_tbl));
    u32Count = part_config.tbl->size / sizeof(struct parts_info);

    for (u8_i = 0; u8_i < u32Count; u8_i++)
    {
        if (info->active && !flash_impl_strcmp(info->part_name, part_name))
        {
            part->trunk        = info->trunk;
            part->backup_trunk = info->backup_trunk;
            part->offset       = info->offset;
            part->size         = info->size;
            part->part_name    = info->part_name;
            break;
        }
        info++;
    }

    if (u8_i == u32Count)
    {
        return 0;
    }

    for (u8_i = 0; u8_i < part_config.part_dev_store; u8_i++)
    {
        if ((part_config.part_dev[u8_i]->engine == info->engine)
            && (part_config.part_dev[u8_i]->cs_select == info->cs_select))
        {
            part->part_dev = part_config.part_dev[u8_i];
            return 1;
        }
    }

    return 0;
}

u8 sstar_part_get_nm(u8 part_id, struct sstar_part *part)
{
    u8                 u8_i;
    u32                u32Count = 0;
    struct parts_info *info     = NULL;

    sstar_part_pni_init();

    if (!part_config.tbl || !part)
    {
        return 0;
    }

    info     = (struct parts_info *)(((u8 *)part_config.tbl) + sizeof(struct parts_tbl));
    u32Count = part_config.tbl->size / sizeof(struct parts_info);

    if (part_id >= u32Count)
        return 0;

    part->trunk        = info[part_id].trunk;
    part->backup_trunk = info[part_id].backup_trunk;
    part->offset       = info[part_id].offset;
    part->size         = info[part_id].size;
    part->part_name    = info[part_id].part_name;

    for (u8_i = 0; u8_i < part_config.part_dev_store; u8_i++)
    {
        if ((part_config.part_dev[u8_i]->engine == info->engine)
            && (part_config.part_dev[u8_i]->cs_select == info->cs_select))
        {
            part->part_dev = part_config.part_dev[u8_i];
            return 1;
        }
    }

    return 0;
}

u8 sstar_part_get_nm_by_dev(u8 *dev_name, u8 part_id, struct sstar_part *part)
{
    u8                 u8_i;
    u8                 index    = 0;
    u32                u32Count = 0;
    struct parts_info *info     = NULL;

    sstar_part_pni_init();

    if (!part_config.tbl || !part)
    {
        return 0;
    }

    for (u8_i = 0; u8_i < part_config.part_dev_store; u8_i++)
    {
        if (!flash_impl_strcmp(part_config.part_dev[u8_i]->dev_name, dev_name))
        {
            part->part_dev = part_config.part_dev[u8_i];
            break;
        }
    }

    if (u8_i == part_config.part_dev_store)
    {
        return 0;
    }

    info     = (struct parts_info *)(((u8 *)part_config.tbl) + sizeof(struct parts_tbl));
    u32Count = part_config.tbl->size / sizeof(struct parts_info);

    for (u8_i = 0; u8_i < u32Count; u8_i++)
    {
        if ((part->part_dev->engine == info->engine) && (part->part_dev->cs_select == info->cs_select))
        {
            if (part_id == index)
            {
                part->offset    = info->offset;
                part->size      = info->size;
                part->part_name = info->part_name;

                break;
            }

            index++;
        }
        info++;
    }

    if (u8_i == u32Count)
    {
        part->offset    = 0;
        part->size      = part->part_dev->capacity;
        part->part_name = part->part_dev->dev_name;
        return 1;
    }

    return 1;
}

u8 sstar_part_get_dev(u8 *dev_name, struct sstar_part *part)
{
    u8 u8_i;

    for (u8_i = 0; u8_i < part_config.part_dev_store; u8_i++)
    {
        if (!flash_impl_strcmp(part_config.part_dev[u8_i]->dev_name, dev_name))
        {
            part->part_dev = part_config.part_dev[u8_i];
            break;
        }
    }

    if (u8_i == part_config.part_dev_store)
    {
        return 0;
    }

    part->trunk        = 0;
    part->backup_trunk = 0;
    part->offset       = 0;
    part->size         = part->part_dev->capacity;
    part->block_size   = part->part_dev->erase_size;
    part->part_name    = part->part_dev->dev_name;

    return 1;
}

u8 sstar_part_is_support_xzdec(struct sstar_part *part)
{
    return part->part_dev->xzdec_en;
}

u32 sstar_part_load(struct sstar_part *part, u32 u32_offset, u8 *pu8_data, u32 u32_size)
{
    u32 limit;
    u32 load_size = 0;

    flash_impl_printf_load_info((void *)part, u32_offset, (u32)pu8_data, u32_size);

    if (!part || (u32_offset + u32_size > part->size))
        return 0;

    if (!part->part_dev || !part->part_dev->read_skip_bad)
        return 0;

    limit = part->size - u32_offset;
    u32_offset += part->offset;

    flash_impl_get_time();

    load_size = part->part_dev->read_skip_bad(part->part_dev->handle, u32_offset, u32_size, limit, pu8_data);

    flash_impl_printf_time_diff();

    return load_size;
}

u32 sstar_part_load_to_xzdec(struct sstar_part *part, u32 u32_offset, u8 *pu8_data, u32 u32_size)
{
    u32 limit;

    if (!part || (u32_offset + u32_size > part->size))
        return 0;

    if (!part->part_dev || !part->part_dev->read_to_xzdec_skip_bad)
        return 0;

    limit = part->size - u32_offset;
    u32_offset += part->offset;

    return part->part_dev->read_to_xzdec_skip_bad(part->part_dev->handle, u32_offset, u32_size, limit, pu8_data);
}

u32 sstar_part_program(struct sstar_part *part, u32 u32_offset, u8 *pu8_data, u32 u32_size)
{
    u32 limit;

    if (!part || (u32_offset + u32_size > part->size))
        return 0;

    if (!part->part_dev || !part->part_dev->write_skip_bad)
        return 0;

    limit = part->size - u32_offset;
    u32_offset += part->offset;

    return part->part_dev->write_skip_bad(part->part_dev->handle, u32_offset, u32_size, limit, pu8_data);
}

u32 sstar_part_erase(struct sstar_part *part, u32 u32_offset, u32 u32_size)
{
    u32 limit;

    if (!part || (u32_offset + u32_size > part->size))
        return 0;

    if (!part->part_dev || !part->part_dev->erase_skip_bad)
        return 0;

    limit = part->size - u32_offset;
    u32_offset += part->offset;

    return part->part_dev->erase_skip_bad(part->part_dev->handle, u32_offset, u32_size, limit);
}

u8 sstar_part_register(struct sstar_part_device *part_dev)
{
    u8 index;

    if (!part_dev || (part_config.part_dev_store >= PART_DEVICE_MAX_CNT))
        return 0;

    for (index = 0; index < part_config.part_dev_store; index++)
    {
        if ((part_dev->engine == part_config.part_dev[index]->engine)
            && (part_dev->cs_select == part_config.part_dev[index]->cs_select))
            return 1;
    }

    part_config.part_dev[part_config.part_dev_store++] = part_dev;

    return 1;
}
