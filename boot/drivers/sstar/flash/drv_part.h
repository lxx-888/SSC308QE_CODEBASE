/*
 * drv_part.h- Sigmastar
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

#ifndef _DRV_PART_H_
#define _DRV_PART_H_

typedef signed char        s8;
typedef unsigned char      u8;
typedef signed short       s16;
typedef unsigned short     u16;
typedef signed int         s32;
typedef unsigned int       u32;
typedef signed long long   s64;
typedef unsigned long long u64;

struct sstar_part_device
{
    u8    engine;
    u8    cs_select;
    u8    xzdec_en;
    u8    dev_name[8];
    u32   capacity;
    u32   erase_size;
    void *handle;
    u32 (*read_skip_bad)(void *, u32, u32, u32, u8 *);
    u32 (*read_to_xzdec_skip_bad)(void *, u32, u32, u32, u8 *);
    u32 (*write_skip_bad)(void *, u32, u32, u32, u8 *);
    u32 (*erase_skip_bad)(void *, u32, u32, u32);
};

struct sstar_part
{
    u8                        trunk;
    u8                        backup_trunk;
    u32                       offset;
    u32                       size;
    u32                       block_size;
    const u8 *                part_name;
    struct sstar_part_device *part_dev;
};

u8 sstar_part_mark_active(u8 *part_name, u8 trunk);
u8 sstar_part_get(u8 *part_name, u8 trunk, struct sstar_part *part);
u8 sstar_part_get_active(u8 *part_name, struct sstar_part *part);
u8 sstar_part_get_nm(u8 part_id, struct sstar_part *part);
u8 sstar_part_get_nm_by_dev(u8 *dev_name, u8 part_id, struct sstar_part *part);
u8 sstar_part_get_dev(u8 *dev_name, struct sstar_part *part);
u8 sstar_part_is_support_xzdec(struct sstar_part *part);

u32 sstar_part_load(struct sstar_part *part, u32 u32_offset, u8 *pu8_data, u32 u32_size);
u32 sstar_part_load_to_xzdec(struct sstar_part *part, u32 u32_offset, u8 *pu8_data, u32 u32_size);
u32 sstar_part_program(struct sstar_part *part, u32 u32_offset, u8 *pu8_data, u32 u32_size);
u32 sstar_part_erase(struct sstar_part *part, u32 u32_offset, u32 u32_size);

u8 sstar_part_register(struct sstar_part_device *part_dev);

#endif /* _DRV_PART_H_ */
