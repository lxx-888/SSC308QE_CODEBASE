/*
 * hal_riu_dbg.h- Sigmastar
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

#ifndef _HAL_RIU_DBG_H_
#define _HAL_RIU_DBG_H_

#include <cam_os_wrapper.h>

#define HAL_RIU_BRIDGE_NUM  5
#define HAL_RIU_FILTER_NUM  6
#define HAL_RIU_CAPTUER_NUM 4

struct riu_timeout_t
{
    u32  num_bank;
    u64 *banks;
};

struct riu_timeout_snapshot_t
{
    u16 bank;
    u16 offset;
};

struct riu_record_t
{
    u32   num_bank;
    u64 * banks;
    u32   addr_base;
    u32   addr_size;
    void *addr_base_virt;

    u8  filt_num[HAL_RIU_BRIDGE_NUM];
    u16 filt_start[HAL_RIU_BRIDGE_NUM][HAL_RIU_FILTER_NUM];
    u16 filt_end[HAL_RIU_BRIDGE_NUM][HAL_RIU_FILTER_NUM];

    u8  cap_num[HAL_RIU_BRIDGE_NUM];
    u16 cap_bank[HAL_RIU_BRIDGE_NUM][HAL_RIU_CAPTUER_NUM];
    u8  cap_offset[HAL_RIU_BRIDGE_NUM][HAL_RIU_CAPTUER_NUM];
    u32 cap_data[HAL_RIU_BRIDGE_NUM][HAL_RIU_CAPTUER_NUM];
    u8  cap_mask[HAL_RIU_BRIDGE_NUM][HAL_RIU_CAPTUER_NUM];
};

enum riu_record_irq_type
{
    RIU_RECORD_WDMA_IRQ,
    RIU_RECORD_WDMA_BUSY_IRQ,
    RIU_RECORD_IRQ,
};

struct riu_record_snapshot_t
{
    u8 irq_type;
    u8 bridge;
    u8 capture;
};

struct hal_riu_dbg_t
{
    struct riu_timeout_t timeout;
    struct riu_record_t  record;
};

extern void hal_riu_timeout_enable(struct hal_riu_dbg_t *hal);
extern void hal_riu_timeout_get_snapshot(u64 base, struct riu_timeout_snapshot_t *snapshort);
extern s8   hal_riu_bank_to_bridge(u16 bank);
extern void hal_riu_record_init(struct hal_riu_dbg_t *hal);
extern void hal_riu_record_enable(struct hal_riu_dbg_t *hal, u8 enable);
extern void hal_riu_record_interrupt(struct hal_riu_dbg_t *hal);
extern void hal_riu_record_get_snapshot(struct hal_riu_dbg_t *hal, struct riu_record_snapshot_t *snapshort);

#endif
