/*
 * hal_miu.c - Sigmastar
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
#include <cam_os_wrapper.h>
#include "hal_miu.h"

//=================================================================================================
//                                     REG MIU DRAM Function
//=================================================================================================

u32 reg_miu_dram_get_size(void) // --> MB
{
    int size = reg_miu_read_mask(BASE_REG_MIU_MMU, REG_DRAM_SIZE, REG_DRAM_SIZE_MASK);

    if (size == 0)
    {
        size = 16;
    }

    if (size & 0xF0)
    {
        return (1 << (size & 0x0F)) + (1 << ((size & 0xF0) >> 4));
    }

    return (1 << (size & 0x0F));
}

void reg_miu_dram_set_size(u32 size) // in MB
{
}

u32 reg_miu_dram_get_freq(void)
{
    u32 ddr_region = 0;
    u32 ddfset     = 0;

    ddfset     = (reg_miu_read_mask(BASE_REG_MIU_ATOP, 0x19, 0xFF) << 16) + reg_miu_read(BASE_REG_MIU_ATOP, 0x18);
    ddr_region = reg_miu_read_mask(BASE_REG_MIU_ATOP, 0x1A, 0xFC) >> 2;
    ddr_region = ddr_region ? ddr_region : 1;
    ddr_region *= reg_miu_read_mask(BASE_REG_MIU_ATOP, 0x1A, 0x03);

    return ((u64)432 << 21) * ddr_region / ddfset;
}

u32 reg_miu_dram_get_pll(void)
{
    return 24 * reg_miu_read_mask(BASE_REG_MIUPLL_PA, 0x3, 0x00FF)
           / ((reg_miu_read_mask(BASE_REG_MIUPLL_PA, 0x3, 0x0700) >> 8) + 2);
}

u8 reg_miu_dram_get_type(void)
{
    return (reg_miu_read_mask(BASE_REG_MIU_ATOP, 0x5B, 0x8000) >> 16)
           | (reg_miu_read_mask(BASE_REG_MIU_DIG, 0x03, 0x4000) >> 13);
}

u8 reg_miu_dram_get_data_rate(void)
{
    return 4;
}

u8 reg_miu_dram_get_bus_width(void)
{
    if ((reg_miu_read(BASE_REG_MIU_PA, 0x3E) >> 4) >= 7)
    {
        return 16;
    }

    return 32;
}

u8 reg_miu_dram_get_ssc(void)
{
    return reg_miu_read_mask(BASE_REG_ATOP_PA, 0x14, 0xC000) != 0x8000;
}

//=================================================================================================
//                                     REG MIU Protect Function
//=================================================================================================

int reg_miu_protect_set_id_list(u16* ids, int num)
{
    int i;

    for (i = 0; i < num; i++)
    {
        reg_miu_write_mask(BASE_REG_PROTECT_ID, REG_MIU_PROTECT0_ID0 + i, REG_MIU_PROTECT0_ID_MASK, ids[i]);
    }

    return 0;
}

int reg_miu_protect_get_id_list(u16* ids, int num)
{
    int i;

    for (i = 0; i < num; i++)
    {
        ids[i] = reg_miu_read_mask(BASE_REG_PROTECT_ID, REG_MIU_PROTECT0_ID0 + i, REG_MIU_PROTECT0_ID_MASK);
    }

    return 0;
}

int reg_miu_protect_set_addr(int index, ss_phys_addr_t start, ss_phys_addr_t end)
{
    u32 msb;
    start >>= REG_PROTECT_ADDR_ALIGN;
    end = (end >> REG_PROTECT_ADDR_ALIGN) - 1;
    msb = (start >> 16) | ((end - 1) >> 16 << 8);

    reg_miu_write(BASE_REG_MIU_PROTECT, REG_MIU_PROTECT0_START + (index << 1), start & 0xFFFF);
    reg_miu_write(BASE_REG_MIU_PROTECT, REG_MIU_PROTECT0_END + (index << 1), end & 0xFFFF);

    reg_miu_write(BASE_REG_MIU_PROTECT_SUB, REG_MIU_PROTECT0_START_23_16 + index, msb);

    return 0;
}

int reg_miu_protect_get_addr(int index, ss_phys_addr_t* start, ss_phys_addr_t* end)
{
    int hi_start, hi_end;

    *start = reg_miu_read(BASE_REG_MIU_PROTECT, REG_MIU_PROTECT0_START + (index << 1));
    *end   = reg_miu_read(BASE_REG_MIU_PROTECT, REG_MIU_PROTECT0_END + (index << 1));

    hi_start = reg_miu_read(BASE_REG_MIU_PROTECT_SUB, REG_MIU_PROTECT0_START_23_16) & 0x00FF;
    hi_end   = reg_miu_read(BASE_REG_MIU_PROTECT_SUB, REG_MIU_PROTECT0_START_23_16) >> 8;

    *start = (*start | (hi_start << 16)) << REG_PROTECT_ADDR_ALIGN;
    *end   = (*end | (hi_end << 16)) << REG_PROTECT_ADDR_ALIGN;

    return 0;
}

int reg_miu_protect_set_id_enable(int index, u32 id_enable)
{
    reg_miu_write_32(BASE_REG_MIU_PROTECT, REG_MIU_PROTECT0_ID_ENABLE + (index << 1), id_enable);

    return 0;
}

u32 reg_miu_protect_get_id_enable(int index)
{
    return reg_miu_read_32(BASE_REG_MIU_PROTECT, REG_MIU_PROTECT0_ID_ENABLE + (index << 1));
}

int reg_miu_protect_set_enable(int index, int enable)
{
    int shift = index & 0x3;
    reg_miu_write_mask(BASE_REG_MIU_PROTECT_SUB, REG_MIU_PROTECT0_WRITE_ENABLE + (index >> 2), 0x1 << shift,
                       enable << shift);

    return 0;
}

bool reg_miu_protect_get_enable(int index)
{
    int shift = index & 0x3;
    return reg_miu_read_mask(BASE_REG_MIU_PROTECT_SUB, REG_MIU_PROTECT0_WRITE_ENABLE + (index >> 2), 0x1 << shift);
}

int reg_miu_protect_set_invert(int index, int invert)
{
    int shift = (index & 0x3) + 8;
    reg_miu_write_mask(BASE_REG_MIU_PROTECT_SUB, REG_MIU_PROTECT0_WRITE_ENABLE + (index >> 2), 0x1 << shift,
                       invert << shift);

    return 0;
}

bool reg_miu_protect_get_invert(int index)
{
    int shift = (index & 0x3) + 8;
    return reg_miu_read_mask(BASE_REG_MIU_PROTECT_SUB, REG_MIU_PROTECT0_WRITE_ENABLE + (index >> 2), 0x1 << shift);
}

int reg_miu_protect_block(int index, bool enable, bool invert, ss_phys_addr_t start, ss_phys_addr_t end, u32 id_enable)
{
    if (!enable)
    {
        reg_miu_protect_set_enable(index, enable);
        return 0;
    }

    reg_miu_protect_set_addr(index, start, end);
    reg_miu_protect_set_id_enable(index, id_enable);
    reg_miu_protect_set_invert(index, invert);
    reg_miu_protect_set_enable(index, enable);

    return 0;
}

//=================================================================================================
//                                     REG MMU Protect Function
//=================================================================================================
int reg_mmu_protect_set_id_list(u16* ids, int num)
{
    int i;

    for (i = 0; i < num; i++)
    {
        reg_miu_write_mask(BASE_REG_PROTECT_ID, REG_MMU_PROTECT0_ID0 + i, REG_MMU_PROTECT0_ID_MASK, ids[i]);
    }

    return 0;
}

int reg_mmu_protect_get_id_list(u16* ids, int num)
{
    int i;

    for (i = 0; i < num; i++)
    {
        ids[i] = reg_miu_read_mask(BASE_REG_PROTECT_ID, REG_MMU_PROTECT0_ID0 + i, REG_MMU_PROTECT0_ID_MASK);
    }

    return 0;
}

int reg_mmu_protect_set_addr(int index, ss_phys_addr_t start, ss_phys_addr_t end)
{
    u32 msb;
    start >>= REG_PROTECT_ADDR_ALIGN;
    end = (end >> REG_PROTECT_ADDR_ALIGN) - 1;
    msb = (start >> 16) | ((end - 1) >> 16 << 8);

    reg_miu_write(BASE_REG_MMU_PROTECT, REG_MMU_PROTECT0_START + (index << 1), start & 0xFFFF);
    reg_miu_write(BASE_REG_MMU_PROTECT, REG_MMU_PROTECT0_END + (index << 1), end & 0xFFFF);

    reg_miu_write(BASE_REG_MIU_PROTECT_SUB, REG_MMU_PROTECT0_START_23_16 + index, msb);

    return 0;
}

int reg_mmu_protect_get_addr(int index, ss_phys_addr_t* start, ss_phys_addr_t* end)
{
    int hi_start, hi_end;

    *start = reg_miu_read(BASE_REG_MMU_PROTECT, REG_MMU_PROTECT0_START + (index << 1));
    *end   = reg_miu_read(BASE_REG_MMU_PROTECT, REG_MMU_PROTECT0_END + (index << 1));

    hi_start = reg_miu_read(BASE_REG_MIU_PROTECT_SUB, REG_MMU_PROTECT0_START_23_16) & 0x00FF;
    hi_end   = reg_miu_read(BASE_REG_MIU_PROTECT_SUB, REG_MMU_PROTECT0_START_23_16) >> 8;

    *start = (*start | (hi_start << 16)) << REG_PROTECT_ADDR_ALIGN;
    *end   = (*end | (hi_end << 16)) << REG_PROTECT_ADDR_ALIGN;

    return 0;
}

int reg_mmu_protect_set_id_enable(int index, u32 id_enable)
{
    reg_miu_write_32(BASE_REG_MMU_PROTECT, REG_MMU_PROTECT0_ID_ENABLE + (index << 1), id_enable);

    return 0;
}

u32 reg_mmu_protect_get_id_enable(int index)
{
    return reg_miu_read_32(BASE_REG_MMU_PROTECT, REG_MMU_PROTECT0_ID_ENABLE + (index << 1));
}

int reg_mmu_protect_set_enable(int index, int enable)
{
    int shift = index & 0x3;
    reg_miu_write_mask(BASE_REG_MIU_PROTECT_SUB, REG_MMU_PROTECT0_WRITE_ENABLE + (index >> 2), 0x1 << shift,
                       enable << shift);

    return 0;
}

bool reg_mmu_protect_get_enable(int index)
{
    int shift = index & 0x3;
    return reg_miu_read_mask(BASE_REG_MIU_PROTECT_SUB, REG_MMU_PROTECT0_WRITE_ENABLE + (index >> 2), 0x1 << shift);
}

int reg_mmu_protect_set_invert(int index, int invert)
{
    int shift = (index & 0x3) + 8;
    reg_miu_write_mask(BASE_REG_MIU_PROTECT_SUB, REG_MMU_PROTECT0_WRITE_ENABLE + (index >> 2), 0x1 << shift,
                       invert << shift);

    return 0;
}

bool reg_mmu_protect_get_invert(int index)
{
    int shift = (index & 0x3) + 8;
    return reg_miu_read_mask(BASE_REG_MIU_PROTECT_SUB, REG_MMU_PROTECT0_WRITE_ENABLE + (index >> 2), 0x1 << shift);
}

int reg_mmu_protect_block(int index, bool enable, bool invert, ss_phys_addr_t start, ss_phys_addr_t end, u32 id_enable)
{
    if (!enable)
    {
        reg_mmu_protect_set_enable(index, enable);
        return 0;
    }

    reg_mmu_protect_set_addr(index, start, end);
    reg_mmu_protect_set_id_enable(index, id_enable);
    reg_mmu_protect_set_invert(index, invert);
    reg_mmu_protect_set_enable(index, enable);

    return 0;
}

//=================================================================================================
//                                     REG MMU Protect Log Function
//=================================================================================================

static struct reg_miu_protect_log* reg_miu_protect_log(void)
{
    return (struct reg_miu_protect_log*)IO_ADDRESS(GET_REG_ADDR(BASE_REG_MIU_MMU, REG_PROTECT_LOG));
}

int reg_miu_protect_flag(bool mmu)
{
    if (mmu)
    {
        return reg_miu_protect_log()->mmu_w_flag;
    }
    return reg_miu_protect_log()->miu_w_flag;
}

int reg_miu_protect_hit_block(bool mmu)
{
    if (mmu)
    {
        return reg_miu_protect_log()->mmu_w_block;
    }
    return reg_miu_protect_log()->miu_w_block;
}

int reg_miu_protect_hit_id(bool mmu)
{
    if (mmu)
    {
        return reg_miu_protect_log()->mmu_w_id;
    }
    return reg_miu_protect_log()->miu_w_id;
}

int reg_miu_protect_log_clr(bool mmu, int clr)
{
    if (mmu)
    {
        reg_miu_protect_log()->mmu_log_clr = clr;
    }
    else
    {
        reg_miu_protect_log()->miu_log_clr = clr;
    }

    return 0;
}

int reg_miu_protect_irq_mask(bool mmu, int mask)
{
    if (mmu)
    {
        reg_miu_protect_log()->mmu_irq_mask = mask;
    }
    else
    {
        reg_miu_protect_log()->miu_irq_mask = mask;
    }

    return 0;
}

ss_phys_addr_t reg_miu_protect_hit_addr(bool mmu)
{
    if (mmu)
    {
        return reg_miu_read_32(BASE_REG_MIU_MMU, REG_MMU_PROTECT_HIT_ADDR_WRITE);
    }
    return reg_miu_read_32(BASE_REG_MIU_MMU, REG_MIU_PROTECT_HIT_ADDR_WRITE);
}

//=================================================================================================
//                                     REG MIU MMU Function
//=================================================================================================

struct reg_miu_mmu* reg_miu_mmu(void)
{
    return (struct reg_miu_mmu*)IO_ADDRESS(GET_REG_ADDR(BASE_REG_MIU_MMU, REG_MMU_CTRL));
}

void reg_miu_mmu_set_page_size(u8 page_size)
{
    reg_miu_mmu()->page_size = page_size;
}

void reg_miu_mmu_set_region(u16 vpa_region, u16 pa_region)
{
    struct reg_miu_mmu* mmu = reg_miu_mmu();

    mmu->vpa_region = vpa_region & 0xFF;
    mmu->pa_region  = pa_region & 0xFF;

    mmu->vpa_region_msb = vpa_region >> 8;
    mmu->pa_region_msb  = pa_region >> 8;
}

void reg_miu_mmu_set_entry(u16 vpa_entry, u16 pa_entry)
{
    struct reg_miu_mmu* mmu = reg_miu_mmu();

    mmu->w_data   = pa_entry;
    mmu->entry_rw = 1;
    mmu->entry    = vpa_entry;
    mmu->access   = 0x3;
}

u16 reg_miu_mmu_get_entry(u16 vpa_entry)
{
    struct reg_miu_mmu* mmu = reg_miu_mmu();
    mmu->entry              = vpa_entry;
    mmu->entry_rw           = 0;

    mmu->access = 0x3;

    CamOsUsDelay(1);

    return mmu->r_data;
}

void reg_miu_mmu_enable(bool enable)
{
    struct reg_miu_mmu* mmu = reg_miu_mmu();

    mmu->enable          = enable;
    mmu->collison_mask_r = !enable;
    mmu->collison_mask_w = !enable;
    mmu->invalid_mask_r  = !enable;
    mmu->invalid_mask_w  = !enable;
}

void reg_miu_mmu_reset(void)
{
    struct reg_miu_mmu* mmu = reg_miu_mmu();

    mmu->enable      = 0;
    mmu->init_done_r = 0;
    mmu->init_done_w = 0;
    mmu->reset       = 1;
    mmu->init_val    = 1;

    while (!mmu->init_done_r || !mmu->init_done_w)
    {
        CamOsUsDelay(1);
    }

    mmu->reset = 0;
}

u32 reg_miu_mmu_get_irq_status(u16* entry, u16* id, u8* is_write)
{
    u32                 status = 0;
    struct reg_miu_mmu* mmu    = reg_miu_mmu();

    if (mmu->collison_flag_r)
    {
        *entry = mmu->collison_entry_r;
        *id    = 0;

        status |= 1;
    }

    if (mmu->collison_flag_w)
    {
        *entry = mmu->collison_entry_w;
        *id    = 0;

        status |= 1;
    }

    if (mmu->invalid_flag_r)
    {
        *entry = mmu->invalid_entry_r;
        *id    = mmu->invalid_id_r;

        status |= 2;
    }

    if (mmu->invalid_flag_w)
    {
        *entry = mmu->invalid_entry_w;
        *id    = mmu->invalid_id_w;

        status |= 4;
    }

    *is_write = mmu->invalid_flag_w || mmu->collison_flag_w;

    return status;
}

void reg_miu_mmu_irq_mask(int mask)
{
    struct reg_miu_mmu* mmu = reg_miu_mmu();
    mmu->collison_mask_r    = mask;
    mmu->collison_mask_w    = mask;
    mmu->invalid_mask_r     = mask;
    mmu->invalid_mask_w     = mask;
}

void reg_miu_mmu_irq_clr(int clr)
{
    struct reg_miu_mmu* mmu = reg_miu_mmu();
    mmu->collison_clr_r     = clr;
    mmu->collison_clr_w     = clr;
    mmu->invalid_clr_r      = clr;
    mmu->invalid_clr_w      = clr;
}

#if defined(CAM_OS_LINUX_KERNEL)
//=================================================================================================
//                                     REG MIU BIST Function
//=================================================================================================

static bool reg_miu_bist_is_chip_top(u32 effi_bank)
{
    return effi_bank == BASE_REG_MIU_DIG;
}

static struct reg_miu_bist_chiptop_status* reg_miu_bist_chiptop(u32 effi_bank)
{
    return (struct reg_miu_bist_chiptop_status*)IO_ADDRESS(GET_REG_ADDR(effi_bank, REG_BIST_CHIPTOP_STATUS));
}

static struct reg_miu_bist_group_status* reg_miu_bist_group_status(u32 effi_bank)
{
    return (struct reg_miu_bist_group_status*)IO_ADDRESS(GET_REG_ADDR(effi_bank, REG_BIST_GROUP_STATUS));
}

static struct reg_miu_bist_group_result* reg_miu_bist_group_result(u32 effi_bank)
{
    return (struct reg_miu_bist_group_result*)IO_ADDRESS(GET_REG_ADDR(effi_bank, REG_BIST_GROUP_RESULT));
}

void reg_miu_bist_init(u32 group_bank, u32 effi_bank)
{
    if (reg_miu_bist_is_chip_top(effi_bank))
    {
        reg_miu_write(effi_bank, REG_BIST_CHIPTOP_BUS_SEL, 0x80e1);
        reg_miu_write(effi_bank, REG_BIST_CHIPTOP_DATA, 0x5a5b);
    }
    else
    {
        reg_miu_write(effi_bank, REG_BIST_GROUP_STATUS, 0x0000);                 // clear bist
        reg_miu_write_mask(group_bank, REG_ID_59, BIT15 | BIT14, BIT15 | BIT14); // Bist mux
        reg_miu_write(effi_bank, REG_BIST_GROUP_DATA_BYTE, 0xFFFF);
    }
}

void reg_miu_bist_set_base(u32 effi_bank, ss_miu_addr_t addr)
{
    if (reg_miu_bist_is_chip_top(effi_bank))
    {
        addr >>= REG_BIST_CHIPTOP_BASE_ALIGN;
        reg_miu_write(effi_bank, REG_BIST_CHIPTOP_BASE, addr & 0xFFFF);
        addr = (addr >> 16) << REG_BIST_CHIPTOP_BASE_EXT_SHIFT;
        reg_miu_write_mask(effi_bank, REG_BIST_CHIPTOP_BASE_EXT, REG_BIST_CHIPTOP_BASE_EXT_MASK, addr);
    }
    else
    {
        addr >>= REG_BIST_GROUP_BASE_ALIGN;
        reg_miu_write_32(effi_bank, REG_BIST_GROUP_BASE_LO, addr);
    }
}

void reg_miu_bist_set_length(u32 effi_bank, u32 length)
{
    if (reg_miu_bist_is_chip_top(effi_bank))
    {
        length >>= REG_BIST_CHIPTOP_LENGTH_ALIGN;
        reg_miu_write_32(effi_bank, REG_BIST_CHIPTOP_LENGTH_LO, length);
    }
    else
    {
        length >>= REG_BIST_GROUP_LENGTH_ALIGN;
        reg_miu_write_32(effi_bank, REG_BIST_GROUP_LENGTH_LO, length);
    }
}

void reg_miu_bist_set_loop(u32 effi_bank, bool loop)
{
    if (reg_miu_bist_is_chip_top(effi_bank))
    {
        reg_miu_bist_chiptop(effi_bank)->loop = loop;
    }
    else
    {
        reg_miu_bist_group_status(effi_bank)->loop = loop;
    }
}

void reg_miu_bist_set_rw_only(u32 effi_bank, bool read_only)
{
    if (reg_miu_bist_is_chip_top(effi_bank))
    {
        if (read_only)
            reg_miu_bist_chiptop(effi_bank)->read_only = 1;
        else
            reg_miu_bist_chiptop(effi_bank)->write_only = 1;
    }
    else
    {
        if (read_only)
            reg_miu_bist_group_status(effi_bank)->read_only = 1;
        else
            reg_miu_bist_group_status(effi_bank)->write_only = 1;
    }
}

static void reg_miu_bist_set_status(u32 effi_bank, bool start)
{
    if (reg_miu_bist_is_chip_top(effi_bank))
    {
        reg_miu_bist_chiptop(effi_bank)->enable = start;
    }
    else
    {
        reg_miu_bist_group_status(effi_bank)->enable = start;
    }
}

void reg_miu_bist_start(u32 effi_bank)
{
    reg_miu_bist_set_status(effi_bank, true);
}

void reg_miu_bist_stop(u32 effi_bank)
{
    reg_miu_bist_set_status(effi_bank, false);
}

u16 reg_miu_bist_get_result(u32 effi_bank)
{
    if (reg_miu_bist_is_chip_top(effi_bank))
    {
        return reg_miu_read(effi_bank, REG_BIST_CHIPTOP_STATUS) & 0xE000; // bit13 ~ bit15
    }
    else
    {
        return reg_miu_read(effi_bank, REG_BIST_GROUP_RESULT) & 0x0007; // bit0 ~ bit2
    }
}

bool reg_miu_bist_is_finish(u32 effi_bank)
{
    if (reg_miu_bist_is_chip_top(effi_bank))
    {
        if (reg_miu_bist_chiptop(effi_bank)->loop) // chiptop loop have not finish flag
        {
            return !reg_miu_bist_chiptop(effi_bank)->enable;
        }
        return reg_miu_bist_chiptop(effi_bank)->finish;
    }
    else
    {
        return reg_miu_bist_group_result(effi_bank)->finish;
    }

    return true;
}

bool reg_miu_bist_is_success(u32 effi_bank)
{
    if (reg_miu_bist_is_chip_top(effi_bank))
    {
        return reg_miu_bist_chiptop(effi_bank)->fail == 0;
    }
    else
    {
        return reg_miu_bist_group_result(effi_bank)->fail == 0;
    }
    return true;
}

//=================================================================================================
//                                     REG MIU BW Function
//=================================================================================================
void reg_miu_bwla_reset(void) {}

void reg_miu_bwla_init(void) {}

void reg_miu_bwla_set_addr(ss_miu_addr_t addr) {}

void reg_miu_bwla_set_round(int round) {}

void reg_miu_bwla_set_enable(bool enable) {}

void reg_miu_bwla_set_ip(int index, u16 id) {}

bool reg_miu_bwla_is_done(void)
{
    return true;
}

#endif // defined(CAM_OS_LINUX_KERNEL)

static void reg_miu_garb_burst_init(int base)
{
    reg_miu_write(base, 0x20, 0x0C08);
    reg_miu_write(base, 0x21, 0x1410);

    reg_miu_write(base, 0x50, 0x0C08);
    reg_miu_write(base, 0x51, 0x1410);

    reg_miu_write(base, 0x13, 0x17);
    reg_miu_write(base, 0x43, 0x17);
}

static void reg_miu_parb_burst_init(int base)
{
    reg_miu_write(base, 0x20, 0x1010);
    reg_miu_write(base, 0x21, 0x1010);

    reg_miu_write(base, 0x50, 0x1010);
    reg_miu_write(base, 0x51, 0x1010);

    reg_miu_write(base, 0x22, 0xffff);
    reg_miu_write(base, 0x23, 0xffff);

    reg_miu_write(base, 0x13, 0x17);
    reg_miu_write(base, 0x43, 0x17);
}

void reg_miu_burst_init(void)
{
    // Default Burst for QOS
    reg_miu_garb_burst_init(BASE_REG_MIU_GRP_SC0);
    reg_miu_garb_burst_init(BASE_REG_MIU_GRP_SC1);
    reg_miu_garb_burst_init(BASE_REG_MIU_GRP_ISP0);
    reg_miu_garb_burst_init(BASE_REG_MIU_GRP_MISC0);

    reg_miu_parb_burst_init(BASE_REG_MIU_PRE_ARB0);
    reg_miu_parb_burst_init(BASE_REG_MIU_PRE_ARB1);

    // Special IP Init Begin

    // VIF/ISP
    reg_miu_write(BASE_REG_MIU_GRP_ISP0, 0x22, 0x55a5);
    reg_miu_write(BASE_REG_MIU_GRP_ISP0, 0x23, 0x5559);
    reg_miu_write(BASE_REG_MIU_GRP_ISP0, 0x52, 0xD96F);
    reg_miu_write(BASE_REG_MIU_GRP_ISP0, 0x53, 0x555B);

    // trigger
    reg_miu_write(BASE_REG_MIU_GRP_ISP0, 0x13, 0x0017);
    reg_miu_write(BASE_REG_MIU_GRP_ISP0, 0x43, 0x0017);
}

void reg_miu_priority_init(void)
{
    // Group: VIF/ISP
    reg_miu_write(BASE_REG_MIU_GRP_ISP0, 0x68, 0x37C8);
    reg_miu_write(BASE_REG_MIU_GRP_ISP0, 0x69, 0xFFF4);

    reg_miu_write(BASE_REG_MIU_GRP_ISP0, 0x45, 0x0183);

    // trigger
    reg_miu_write(BASE_REG_MIU_GRP_ISP0, 0x13, 0x0017);
    reg_miu_write(BASE_REG_MIU_GRP_ISP0, 0x43, 0x0017);

    // PreArb
    reg_miu_write(BASE_REG_MIU_PRE_ARB1, 0x5D, 0xF7FF);
    reg_miu_write(BASE_REG_MIU_PRE_ARB1, 0x13, 0x0017);
    reg_miu_write(BASE_REG_MIU_PRE_ARB1, 0x43, 0x0017);

    // PA
    reg_miu_write(BASE_REG_MIU_PA, 0x00, 0x2AFF);
    reg_miu_write(BASE_REG_MIU_PA, 0x20, 0x2AFF);
    reg_miu_write(BASE_REG_MIU_PA, 0x24, 0x0AFF);
    reg_miu_write(BASE_REG_MIU_PA, 0x3E, 0x0030);
    reg_miu_write(BASE_REG_MIU_PA, 0x41, 0x0AFF);
    reg_miu_write(BASE_REG_MIU_PA, 0x1F, 0x0001);
    reg_miu_write(BASE_REG_MIU_PA, 0x3F, 0x0001);
}

void reg_miu_bw_init(void)
{
    reg_miu_burst_init();
    reg_miu_priority_init();
}
