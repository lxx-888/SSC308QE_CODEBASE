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

#define GRP_ARB_NUM       (3)
#define GROUP(client_id)  ((client_id) >> 4)
#define CLIENT(client_id) (client_id & 0xF)

#define SET_BIT_MASK(x, mask, val) x = (((x) & ~(mask)) | (val))

//=================================================================================================
//                                     Reg MIU DRAM
//=================================================================================================

#define REG_DRAM_SIZE      (0x0d)
#define REG_DRAM_SIZE_MASK BITS_RANGE(7 : 0)

#define REG_DRAM_STATUS (0x1)

#define GARB_RIU_SW_MCG_EN (BIT9)
#define PREA_RIU_SW_MCG_EN (BIT9)
#define PA_RIU_SW_MCG_EN   (BIT0)

enum reg_miu_dram_type
{
    REG_MIU_DRAM_DDR4    = 0,
    REG_MIU_DRAM_DDR3    = 1,
    REG_MIU_DRAM_DDR2    = 2,
    REG_MIU_DRAM_LPDDR4  = 4,
    REG_MIU_DRAM_LPDDR3  = 5,
    REG_MIU_DRAM_LPDDR2  = 6,
    REG_MIU_DRAM_LPDDR4X = 7,
    REG_MIU_DRAM_UNKNOW  = 15,
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
//                                     Reg MIU Group
//=================================================================================================
typedef struct
{
    u32 enable : 1;
    u32 mode : 1;
    u32 pass_period : 2;
    u32 client_id : 4;
    u32 mask_period : 8;
    u32 padding : 16;
} miu_reg_grp_flowctrl;

typedef struct
{
    u32                  _no_use_00_0f[16]; // 0x0 ~ 0xf
    u32                  r_hpmask;          // 0x10
    u32                  _no_use_11_12[2];  // 0x11 ~ 0x12
    u32                  r_toggle;          // 0x13
    u32                  r_request_mask;    // 0x14
    u32                  _no_use_15;        // 0x15
    u32                  r_flow[0];
    miu_reg_grp_flowctrl r_flowctrl[4];        // 0x16 ~ 0x19
    u32                  r_flowctrl_force;     // 0x1a
    u32                  r_dont_car_1b_21[5];  // 0x1b ~ 0x1f
    u32                  r_burst_opt[2];       // 0x20 ~ 0x21
    u32                  r_burst_sel[2];       // 0x22 ~ 0x23
    u32                  r_dont_care_24_28[5]; // 0x24 ~ 0x28
    u32                  r_fix_priority[4];    // 0x29 ~ 0x2c
    u32                  r_priority[2];        // 0x2d ~ 0x2e
    u32                  r_dont_care_2f;       // 0x2f

    u32                  _no_use_30_3f[16];    // 0x30 ~ 0x3f
    u32                  w_hpmask;             // 0x40
    u32                  w_dont_care_41_42[2]; // 0x41 ~ 0x42
    u32                  w_toggle;             // 0x43
    u32                  w_request_mask;       // 0x44
    u32                  _no_use_45;           // 0x45
    u32                  w_flow[0];
    miu_reg_grp_flowctrl w_flowctrl[4];         // 0x46 ~ 0x49
    u32                  w_flowctrl_force;      // 0x4a
    u32                  w_dont_car_4b_51[5];   // 0x4b ~ 0x4f
    u32                  w_burst_opt[2];        // 0x50 ~ 0x51
    u32                  w_burst_sel[2];        // 0x52 ~ 0x53
    u32                  w_dont_care_54_63[16]; // 0x54 ~ 0x63
    u32                  w_fix_priority[4];     // 0x64 ~ 0x67
    u32                  w_priority[2];         // 0x68 ~ 0x69
    u32                  _no_use_6A;            // 0x6A
    u32                  mcg_diable;            // 0x6B
} miu_reg_grp;

typedef struct
{
    u32 limit : 8;
    u32 mask : 1;
    u32 hp_mask : 1;
    u32 limit_mask : 1;
    u32 limit_last : 1;
    u32 priority : 4;
    u32 padding : 16;
} miu_reg_pa_port;

typedef struct
{
    u32 enable : 1;
    u32 mode : 1;
    u32 rq_mask : 1;
    u32 : 1;
    u32 pass_period : 2;
    u32 : 2;
    u32 mask_period : 8;
    u32 padding : 16;
} miu_reg_pa_flowctrl;

typedef struct
{
    miu_reg_pa_port     w_port1[3];        // 0x00 ~ 0x02
    u32                 w_ddr_stall;       // 0x03
    u32                 _no_use_04_16[19]; // 0x04 ~ 0x16
    u32                 w_flow[0];
    miu_reg_pa_flowctrl w_flowctrl[8]; // 0x17 ~ 0x1e
    u32                 w_toggle;      // 0x1f

    miu_reg_pa_port     r_port[8]; // 0x20 ~ 0x27
    u32                 r_flow[0];
    miu_reg_pa_flowctrl r_flowctrl[8];     // 0x28 ~ 0x2f
    u32                 _no_use_30;        // 0x30
    u32                 r_ddr_stall;       // 0x31
    u32                 _no_use_32_3e[11]; // 0x32 ~ 0x3c
    u32                 mcg_diable;        // 0x3d
    u32                 _no_use_3e;        // 0x3e
    u32                 r_toggle;          // 0x3f
    miu_reg_pa_port     w_port2[5];        // 0x40 ~ 0x44
} miu_reg_pa;

typedef struct
{
    u32 _no_use_00_0f[0x0f - 0x00 + 1]; // 0x00 ~ 0x0f
    u32 r_hpmask;                       // 0x10
    u32 _no_use_11_12[0x12 - 0x11 + 1]; // 0x11 ~ 0x12
    u32 r_toggle;                       // 0x13
    u32 _no_use_14_1f[0x1f - 0x14 + 1]; // 0x14 ~ 0x1f
    u32 r_burst[2];                     // 0x20 ~ 0x21
    u32 _no_use_22_2c[0x2c - 0x22 + 1]; // 0x22 ~ 0x2c
    u32 r_priority;                     // 0x2d
    u32 _no_use_2e_3f[0x3f - 0x2e + 1]; // 0x2e ~ 0x3f
    u32 w_hpmask;                       // 0x40
    u32 _no_use_41_42[0x42 - 0x41 + 1]; // 0x41 ~ 0x42
    u32 w_toggle;                       // 0x43
    u32 _no_use_44_4f[0x4f - 0x44 + 1]; // 0x44 ~ 0x4f
    u32 w_burst[2];                     // 0x50 ~ 0x51
    u32 _no_use_52_5c[0x5c - 0x52 + 1]; // 0x52 ~ 0x5c
    u32 w_priority;                     // 0x5d
    u32 _no_use_5E_62[0x62 - 0x5E + 1]; // 0x5E ~ 0x62
    u32 mcg_diable;                     // 0x63
} miu_reg_pre;

typedef struct
{
    u32 enable : 1;
    u32 : 1;
    u32 latency : 2;
    u32 : 28;
} miu_reg_pa_vprw;

typedef struct
{
    u32 _no_use_00_02[3];  // 0x00 ~ 0x02
    u32 port1_ctrl;        // 0x03
    u32 _no_use_00_3f[60]; // 0x04 ~ 0x3f

    u32 vpw[32]; // 0x40 ~ 0x5f
    u32 vpr[32]; // 0x60 ~ 0x7f
} miu_reg_pa_cfg;

typedef struct
{
    u32 _no_use_00_0f[16]; // 0x00 ~ 0x0f

    u32 clean_wcmd[16]; // 0x10 ~ 0x1f
    u32 clean_rcmd[16]; // 0x20 ~ 0x2f
    u32 status_wcmd;    // 0x30
    u32 status_rcmd;    // 0 31
} miu_reg_ip_clean;

typedef struct arbiter_handle
{
    miu_reg_grp*      group[GRP_ARB_NUM];
    int               group_num;
    miu_reg_pre*      prea;
    miu_reg_pa*       pa;
    miu_reg_pa_cfg*   pa_cfg;
    miu_reg_ip_clean* ip_clean[GRP_ARB_NUM];
} arbiter_handle;

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

#define REG_MIU_PROTECT_HIT_ADDR_READ  (0x28)
#define REG_MIU_PROTECT_HIT_ADDR_WRITE (0x2A)
#define REG_MMU_PROTECT_HIT_ADDR_READ  (0x38)
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
    u32 enable : 1; // 0
    u32 page_size : 2;
    u32 : 1;
    u32 reset : 1; // 4
    u32 init_val : 1;
    u32 : 2;
    u32 bist_fail_r : 1; // 8
    u32 bist_fail_w : 1;
    u32 : 2;
    u32 init_done_r : 1; // 12
    u32 init_done_w : 1;
    u32 : 2;
    u32 : 16;

    // 0x01
    u32 vpa_region : 8;
    u32 pa_region : 8;
    u32 : 16;

    // 0x02
    u32 entry : 12;
    u32 : 3;
    u32 entry_rw : 1;
    u32 : 16;

    // 0x03
    u32 w_data : 12;
    u32 : 4;
    u32 : 16;

    // 0x04
    u32 r_data : 12;
    u32 : 4;
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
    u32 collison_entry_r : 12;
    u32 : 4;
    u32 : 16;
    // 0x11
    u32 collison_entry_w : 12;
    u32 : 4;
    u32 : 16;

    u32 : 32; // 0x12
    u32 : 32; // 0x13

    // 0x14
    u32 invalid_entry_r : 12;
    u32 : 4;
    u32 : 16;
    // 0x15
    u32 invalid_entry_w : 12;
    u32 : 4;
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

#define REG_BIST_GROUP_BIST_SEL (0x59)
#define REG_BIST_GROUP_RESULT   (0x59)
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
// reg_miu_client
void reg_miu_client_set_stall(u16 id, bool write, bool enable);

void reg_miu_client_set_parb_burst_normal(u16 id, bool write, u32 burst);

void reg_miu_client_set_burst_opt_normal(u16 grp_id, bool write, u32 burst_id, u32 burst);

void reg_miu_client_set_burst_sel_normal(u16 id, bool write, u32 burst);
u32  reg_miu_client_get_burst_sel_normal(u16 id, bool write);

void reg_miu_client_set_parb_priority_normal(u16 id, bool write, u32 priority);

void reg_miu_client_set_priority_normal(u16 id, bool write, u32 priority);
u32  reg_miu_client_get_priority_normal(u16 id, bool write);

void reg_miu_client_set_mask_normal(u16 id, bool write, bool mask);
bool reg_miu_client_get_mask_normal(u16 id, bool write);

void reg_miu_client_set_urgent_normal(u16 id, bool write, bool urgent);
bool reg_miu_client_get_urgent_normal(u16 id, bool write);

int reg_miu_client_set_flowctrl_normal(u16 id, bool write, bool enable, u32 mask_period, u32 pass_period);
u32 reg_miu_client_get_mask_period_normal(u16 id, bool write);
u32 reg_miu_client_get_pass_period_normal(u16 id, bool write);

// reg_miu_client
void reg_miu_client_set_burst_high(u16 id, bool write, u32 burst);
u32  reg_miu_client_get_burst_high(u16 id, bool write);

void reg_miu_client_set_priority_high(u16 id, bool write, u32 priority);
u32  reg_miu_client_get_priority_high(u16 id, bool write);

void reg_miu_client_set_mask_high(u16 id, bool write, bool mask);
bool reg_miu_client_get_mask_high(u16 id, bool write);

void reg_miu_client_set_urgent_high(u16 id, bool write, bool urgent);
bool reg_miu_client_get_urgent_high(u16 id, bool write);

int reg_miu_client_set_flowctrl_high(u16 id, bool write, bool enable, u32 mask_period, u32 pass_period);
u32 reg_miu_client_get_mask_period_high(u16 id, bool write);
u32 reg_miu_client_get_pass_period_high(u16 id, bool write);

void reg_miu_client_set_vp(u16 id, bool write, u32 vp);
u32  reg_miu_client_get_vp(u16 id, bool write);

int reg_miu_client_module_reset(u16 id, bool write);

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
int            reg_miu_protect_flag(bool mmu, bool* bwr);
int            reg_miu_protect_hit_block(bool mmu, bool bwr);
int            reg_miu_protect_hit_id(bool mmu, bool bwr);
int            reg_miu_protect_irq_mask(bool mmu, int mask);
int            reg_miu_protect_log_clr(bool mmu, int clr);
ss_phys_addr_t reg_miu_protect_hit_addr(bool mmu, bool bwr);
int            reg_miu_protect_clear_irq(void);

#if defined(CONFIG_MIU_HW_MMU)
void reg_miu_mmu_set_page_size(u8 page_size);
void reg_miu_mmu_set_region(u16 vpa_region, u16 pa_region);
void reg_miu_mmu_set_entry(u16 vpa_entry, u16 pa_entry);
u16  reg_miu_mmu_get_entry(u16 vpa_entry);
void reg_miu_mmu_enable(bool enable);
u8   reg_miu_mmu_enable_status(void);
u8   reg_miu_mmu_reset(void);

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
void reg_miu_bist_set_cmd_mode(u32 effi_bank, u16 cmd_mode);
void reg_miu_bist_start(u32 effi_bank);
void reg_miu_bist_stop(u32 effi_bank);
u16  reg_miu_bist_get_result(u32 effi_bank);
bool reg_miu_bist_is_finish(u32 effi_bank);
bool reg_miu_bist_is_success(u32 effi_bank);
#endif // defined(CAM_OS_LINUX_KERNEL)

#if defined(CONFIG_MIU_BWLA)
// reg_miu_bwla
void reg_miu_bwla_reset(void);
void reg_miu_bwla_init(void);
void reg_miu_bwla_set_mode(int mode);
void reg_miu_bwla_set_addr(ss_miu_addr_t addr);
void reg_miu_bwla_set_round(int round);
void reg_miu_bwla_set_enable(bool enable);
void reg_miu_bwla_set_ip(int index, u16 id);
bool reg_miu_bwla_is_done(void);
#endif
#endif // #ifndef __HAL_MIU_H__
