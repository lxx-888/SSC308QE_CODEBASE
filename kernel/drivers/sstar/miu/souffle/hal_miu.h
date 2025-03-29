/*
 * hal_miu.h - Sigmastar
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
#ifndef __HAL_MIU_H__
#define __HAL_MIU_H__

#include <cam_os_wrapper.h>
#include "ms_platform.h"
#include "registers.h"
#include <drv_miu.h>

//=================================================================================================
//                                     Reg MIU Base Operation
//=================================================================================================
#define reg_miu_write(bank, offset, value) OUTREG16(GET_REG_ADDR(bank, offset), value)
#define reg_miu_write_32(bank, offset, value)    \
    reg_miu_write(bank, offset, value & 0xFFFF); \
    reg_miu_write(bank, offset + 1, value >> 16)

#define reg_miu_read(bank, offset)    INREG16(GET_REG_ADDR(bank, offset))
#define reg_miu_read_32(bank, offset) ((u32)reg_miu_read(bank, offset) | ((u32)reg_miu_read(bank, offset + 1) << 16))

#define reg_miu_write_mask(bank, offset, mask, value) \
    reg_miu_write(bank, offset, ((reg_miu_read(bank, offset) & (~(mask))) | (value)))
#define reg_miu_read_mask(bank, offset, mask) INREGMSK16(GET_REG_ADDR(bank, offset), mask)

#define reg_miu_set(bank, offset, value) SETREG16(GET_REG_ADDR(bank, offset), value)
#define reg_miu_clr(bank, offset, value) CLRREG16(GET_REG_ADDR(bank, offset), value)
#ifndef BIT
#define BIT(bit) (1 << (bit))
#endif
#define BITS_RANGE(range)           (BIT(((1)?range)+1) - BIT((0)?range))

//=================================================================================================
//                                     Reg MIU DRAM
//=================================================================================================

#define REG_DRAM_SIZE      (0x0d)
#define REG_DRAM_SIZE_MASK BITS_RANGE(7 : 0)

#define REG_DRAM_STATUS (0x1)

enum reg_miu_dram_type
{
    REG_MIU_DRAM_DDR4   = 0,
    REG_MIU_DRAM_DDR3   = 1,
    REG_MIU_DRAM_DDR2   = 2,
    REG_MIU_DRAM_LPDDR4 = 4,
    REG_MIU_DRAM_LPDDR3 = 5,
    REG_MIU_DRAM_LPDDR2 = 6,
    REG_MIU_DRAM_UNKNOW = 15,
};

struct reg_miu_dram_status
{
    u32 dram_type : 2;
    u32 dram_bus : 2;
    u32 : 4;
    u32 data_ratio : 2;
    u32 : 6;
    u32 : 16;
};

//=================================================================================================
//                                     Reg MIU Protect
//=================================================================================================
// for protect
#define REG_PROTECT_ADDR_ALIGN (12) // 4KB

#define REG_MIU_PROTECT0_WRITE_ENABLE (0x50)
#define REG_MMU_PROTECT0_WRITE_ENABLE (0x58)

#define REG_MIU_PROTECT0_INVERT REG_MIU_PROTECT0_WRITE_ENABLE
#define REG_MMU_PROTECT0_INVERT REG_MMU_PROTECT0_WRITE_ENABLE

#define REG_MIU_PROTECT0_ID0 (0x00)
#define REG_MMU_PROTECT0_ID0 (0x40)

#define REG_MIU_PROTECT0_ID_MASK BITS_RANGE(8 : 0)
#define REG_MMU_PROTECT0_ID_MASK BITS_RANGE(8 : 0)

#define REG_MIU_PROTECT0_ID_ENABLE (0x40)
#define REG_MMU_PROTECT0_ID_ENABLE (0x40)

#define REG_MIU_PROTECT0_START (0x00)
#define REG_MMU_PROTECT0_START (0x00)

#define REG_MIU_PROTECT0_END (0x01)
#define REG_MMU_PROTECT0_END (0x01)

#define REG_MIU_PROTECT0_START_23_16 (0x00)
#define REG_MMU_PROTECT0_START_23_16 (0x20)

#define REG_MIU_PROTECT_HIT_ADDR_WRITE (0x2A)
#define REG_MMU_PROTECT_HIT_ADDR_WRITE (0x3A)

#define REG_PROTECT_LOG (0x21)

struct reg_miu_protect_log
{
    // 0x21
    u32 miu_log_clr : 1;
    u32 miu_irq_mask : 1;
    u32 mmu_log_clr : 1;
    u32 mmu_irq_mask : 1;
    u32 : 12;
    u32 : 16;
    u32 no_use1[2];
    // 0x24
    u32 miu_r_flag : 1;
    u32 miu_r_block : 6;
    u32 miu_r_id : 9;
    u32 : 16;
    // 0x25
    u32 miu_w_flag : 1;
    u32 miu_w_block : 6;
    u32 miu_w_id : 9;
    u32 : 16;
    u32 no_use[14];
    // 0x34
    u32 mmu_r_flag : 1;
    u32 mmu_r_block : 6;
    u32 mmu_r_id : 9;
    u32 : 16;
    // 0x35
    u32 mmu_w_flag : 1;
    u32 mmu_w_block : 6;
    u32 mmu_w_id : 9;
};

//=================================================================================================
//                                     Reg MIU MMU
//=================================================================================================

#define REG_MMU_CTRL (0x00)

// Note, please check mmu entry bit carefully!!!
struct reg_miu_mmu
{
    // 0x00
    u32 enable : 1;
    u32 page_size : 2;
    u32 : 1;
    u32 reset : 1;
    u32 init_val : 1;
    u32 : 2;
    u32 bist_fail_r : 1;
    u32 bist_fail_w : 1;
    u32 : 2;
    u32 init_done_r : 1;
    u32 init_done_w : 1;
    u32 : 2;
    u32 : 16;

    // 0x01
    u32 vpa_region : 8;
    u32 pa_region : 8;
    u32 : 16;

    // 0x02
    u32 entry : 13;
    u32 : 2;
    u32 entry_rw : 1;
    u32 : 16;

    // 0x03
    u32 w_data : 13;
    u32 : 3;
    u32 : 16;

    // 0x04
    u32 r_data : 13;
    u32 : 3;
    u32 : 16;

    u32 no_use0[3]; // 0x05 ~ 0x7

    // 0x08
    u32 collison_clr_r : 1;
    u32 collison_mask_r : 1;
    u32 collison_clr_w : 1;
    u32 collison_mask_w : 1;
    u32 : 4;
    u32 invalid_clr_r : 1;
    u32 invalid_mask_r : 1;
    u32 invalid_clr_w : 1;
    u32 invalid_mask_w : 1;
    u32 : 4;
    u32 : 16;

    // 0x09
    u32 collison_flag_r : 1;
    u32 collison_flag_w : 1;
    u32 : 6;
    u32 invalid_flag_r : 1;
    u32 invalid_flag_w : 1;
    u32 : 6;
    u32 : 16;

    // 0x0A
    u32 access : 2;
    u32 : 30;

    // 0x0B ~ 0x0C
    u32 invalid_id_r : 8;
    u32 : 8;
    u32 : 16;

    u32 invalid_id_w : 8;
    u32 : 8;
    u32 : 16;

    u32 : 32; // 0x0D

    // 0x0E
    u32 vpa_region_msb : 1;
    u32 : 7;
    u32 pa_region_msb : 1;
    u32 : 7;
    u32 : 16;

    u32 : 32; // 0x0F

    // 0x10
    u32 collison_entry_r : 13;
    u32 : 3;
    u32 : 16;
    // 0x11
    u32 collison_entry_w : 13;
    u32 : 3;
    u32 : 16;

    u32 : 32; // 0x12
    u32 : 32; // 0x13

    // 0x14
    u32 invalid_entry_r : 13;
    u32 : 3;
    u32 : 16;
    // 0x15
    u32 invalid_entry_w : 13;
    u32 : 3;
    u32 : 16;
};

#if defined(CAM_OS_LINUX_KERNEL)
//=================================================================================================
//                                     Reg MIU CHIPTOP BIST
//=================================================================================================

#define REG_BIST_CHIPTOP_STATUS (0x70)
struct reg_miu_bist_chiptop_status
{
    u32 enable : 1;
    u32 : 3;
    u32 loop : 1;
    u32 : 3;
    u32 read_only : 1;
    u32 write_only : 1;
    u32 : 3;
    u32 fail : 2;
    u32 finish : 1;
    u32 : 16;
};

#define REG_BIST_CHIPTOP_BASE           (0x71)
#define REG_BIST_CHIPTOP_BASE_EXT       (0x6f)
#define REG_BIST_CHIPTOP_BASE_EXT_MASK  (BIT2 | BIT3)
#define REG_BIST_CHIPTOP_BASE_EXT_SHIFT (2)
#define REG_BIST_CHIPTOP_BASE_ALIGN     (13) // 8KB

#define REG_BIST_CHIPTOP_LENGTH_LO    (0x72)
#define REG_BIST_CHIPTOP_LENGTH_HI    (0x73)
#define REG_BIST_CHIPTOP_LENGTH_ALIGN (4) // 16B

#define REG_BIST_CHIPTOP_DATA    (0x74)
#define REG_BIST_CHIPTOP_BUS_SEL (0x7f)

//=================================================================================================
//                                     Reg MIU GROUP BIST
//=================================================================================================

#define REG_BIST_GROUP_STATUS (0x50)
struct reg_miu_bist_group_status
{
    u32 enable : 1;
    u32 mode : 2;
    u32 inv : 1;
    u32 loop : 1;
    u32 read_only : 1;
    u32 write_only : 1;
    u32 : 1;
    u32 burst_len : 2;
    u32 cmd_len : 2;
    u32 addr_sel : 3;
    u32 : 17;
};

#define REG_BIST_GROUP_DATA_BYTE    (0x51)
#define REG_BIST_GROUP_LENGTH_LO    (0x54)
#define REG_BIST_GROUP_LENGTH_HI    (0x55)
#define REG_BIST_GROUP_LENGTH_ALIGN (4) // 16B
#define REG_BIST_GROUP_DATA         (0x56)
#define REG_BIST_GROUP_BASE_LO      (0x57)
#define REG_BIST_GROUP_BASE_HI      (0x58)
#define REG_BIST_GROUP_BASE_ALIGN   (10) // 1KB

#define REG_BIST_GROUP_RESULT (0x59)
struct reg_miu_bist_group_result
{
    u32 fail : 2;
    u32 finish : 1;
    u32 clear : 1;
    u32 reset : 1;
    u32 : 27;
};

//=================================================================================================
//                                     Reg MIU BWLA
//=================================================================================================

#endif // defined(CAM_OS_LINUX_KERNEL)

//=================================================================================================
//                                     Reg MIU API
//=================================================================================================

// reg_miu_dram
u32  reg_miu_dram_get_size(void);
void reg_miu_dram_set_size(u32 size);
u32  reg_miu_dram_get_freq(void);
u32  reg_miu_dram_get_pll(void);
u8   reg_miu_dram_get_type(void);
u8   reg_miu_dram_get_data_rate(void);
u8   reg_miu_dram_get_bus_width(void);
u8   reg_miu_dram_get_ssc(void);

// reg_miu_protect
int  reg_miu_protect_set_id_list(u16* ids, int num);
int  reg_miu_protect_get_id_list(u16* ids, int num);
int  reg_miu_protect_set_id_enable(int index, u32 id_enable);
u32  reg_miu_protect_get_id_enable(int index);
int  reg_miu_protect_set_addr(int index, ss_phys_addr_t start, ss_phys_addr_t end);
int  reg_miu_protect_get_addr(int index, ss_phys_addr_t* start, ss_phys_addr_t* end);
int  reg_miu_protect_set_invert(int index, int invert);
bool reg_miu_protect_get_invert(int index);
int  reg_miu_protect_set_enable(int index, int enable);
bool reg_miu_protect_get_enable(int index);
int reg_miu_protect_block(int index, bool enable, bool invert, ss_phys_addr_t start, ss_phys_addr_t end, u32 id_enable);

// reg_mmu_protect
int  reg_mmu_protect_set_id_list(u16* ids, int num);
int  reg_mmu_protect_get_id_list(u16* ids, int num);
int  reg_mmu_protect_set_id_enable(int index, u32 id_enable);
u32  reg_mmu_protect_get_id_enable(int index);
int  reg_mmu_protect_set_addr(int index, ss_phys_addr_t start, ss_phys_addr_t end);
int  reg_mmu_protect_get_addr(int index, ss_phys_addr_t* start, ss_phys_addr_t* end);
int  reg_mmu_protect_set_invert(int index, int invert);
bool reg_mmu_protect_get_invert(int index);
int  reg_mmu_protect_set_enable(int index, int enable);
bool reg_mmu_protect_get_enable(int index);
int reg_mmu_protect_block(int index, bool enable, bool invert, ss_phys_addr_t start, ss_phys_addr_t end, u32 id_enable);

// reg_miu_protect_log
int            reg_miu_protect_flag(bool mmu);
int            reg_miu_protect_hit_block(bool mmu);
int            reg_miu_protect_hit_id(bool mmu);
int            reg_miu_protect_irq_mask(bool mmu, int mask);
int            reg_miu_protect_log_clr(bool mmu, int clr);
ss_phys_addr_t reg_miu_protect_hit_addr(bool mmu);

#if defined(CONFIG_MIU_HW_MMU)
void reg_miu_mmu_set_page_size(u8 page_size);
void reg_miu_mmu_set_region(u16 vpa_region, u16 pa_region);
void reg_miu_mmu_set_entry(u16 vpa_entry, u16 pa_entry);
u16  reg_miu_mmu_get_entry(u16 vpa_entry);
void reg_miu_mmu_enable(bool enable);
void reg_miu_mmu_reset(void);

u32  reg_miu_mmu_get_irq_status(u16* entry, u16* id, u8* is_write);
void reg_miu_mmu_irq_mask(int mask);
void reg_miu_mmu_irq_clr(int clr);
#endif

#if defined(CAM_OS_LINUX_KERNEL)
// reg_miu_bist
void reg_miu_bist_init(u32 group_bank, u32 effi_bank);
void reg_miu_bist_set_base(u32 effi_bank, ss_miu_addr_t addr);
void reg_miu_bist_set_length(u32 effi_bank, u32 length);
void reg_miu_bist_set_loop(u32 effi_bank, bool loop);
void reg_miu_bist_set_rw_only(u32 effi_bank, bool read_only);
void reg_miu_bist_start(u32 effi_bank);
void reg_miu_bist_stop(u32 effi_bank);
u16  reg_miu_bist_get_result(u32 effi_bank);
bool reg_miu_bist_is_finish(u32 effi_bank);
bool reg_miu_bist_is_success(u32 effi_bank);

// reg_miu_bwla
void reg_miu_bwla_reset(void);
void reg_miu_bwla_init(void);
void reg_miu_bwla_set_addr(ss_miu_addr_t addr);
void reg_miu_bwla_set_round(int round);
void reg_miu_bwla_set_enable(bool enable);
void reg_miu_bwla_set_ip(int index, u16 id);
bool reg_miu_bwla_is_done(void);

#endif // defined(CAM_OS_LINUX_KERNEL)

void reg_miu_bw_init(void);

#endif // #ifndef __HAL_MIU_H__
