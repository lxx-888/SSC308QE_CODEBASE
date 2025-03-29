/*
 * hal_dq.c- Sigmastar
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

#include <linux/module.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/sched.h>
#include <linux/kthread.h>
#include <linux/threads.h>
#include <linux/delay.h>
#include <linux/time.h>
#include <linux/miscdevice.h>
#include <linux/dma-mapping.h>
#include <linux/of_device.h>
#include <linux/slab.h>
#include <linux/mm.h>
#include "tsensor.h"
#include "ms_platform.h"
#include "registers.h"
#include "hal_dq.h"

#define GET_CLKPH_ID(kcode, clkph)  (clkph >= 0x80 ? (clkph - 0x80 + kcode / 2) : clkph)
#define GET_CLKPH(kcode, clkid)     (clkid)
#define GET_CLKPH_128(kcode, clkid) (clkid >= kcode / 2 ? (clkid + 0x80 - kcode / 2) : clkid)
#define WORD_GROUP_MODE             false
#define MIU0_SYNOPSYS_BASE          0x161300
#define MIU0_ATOP_BASE              0x164700

//#define REG_ERR_MSG                 (0x100400+(0x28 << 1))   // 0x1004, offset 0x28
#define MAX_OEN_SKEW    0xF
#define WRITE_DQS_PHASE 4

#define MIU_BUS_16BIT 1

#define SS_INREG16(x)     INREG16(GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, x))
#define SS_OUTREG16(x, y) OUTREG16(GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, x), y)
#define SS_OUTREG32(x, y) OUTREG32(GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, x), y)

#define DBG_PHASE_EN 0

typedef struct ddr_reg
{
    U32 bank_addr;
    U16 bank_mask;
} RegBase;

typedef enum
{
    MOVE_BY_DQ = 0,
    MOVE_BY_BYTE,
    MOVE_ALL,
} WR_DQ_MOVE_FLAG;

typedef enum
{
    MODE_BY_128 = 0,
    MODE_BY_KCODE,
} WR_DQ_PH_MODE;

#if 0
static int def_ddrfreq      = 0;
static int def_ddfset       = 0;
static int def_tx_kcode     = 0;
static int def_UI_X10       = 0;
static int def_dlycell_X10  = 0;
static int def_dqs_tree_X10 = 0;
#endif

// static int wait_time_period = 60;
static int g_deltaD      = 0;
static int dcdl_scl_mode = 0x0f00;
static int mr4_en        = 0;
static int mr23_en       = 0;

static char def_dq_skew[4];
static char def_dq_clkph[36];
static char def_oendq_skew[4];
static char def_delay_1t[36];

#if 1 // miu register and parameters
#define MAX_MIU_NUM 1

RegBase _pRegResult_MRR[MAX_MIU_NUM];
RegBase _pRegRef_MROsc[MAX_MIU_NUM];
RegBase _pRegDynUpd_EN[MAX_MIU_NUM];
RegBase _pRegDynUpd_halt[MAX_MIU_NUM];

RegBase _pRegWriteDQSkew[MAX_MIU_NUM][4];
RegBase _pRegWriteDQoenSkew[MAX_MIU_NUM][4];
RegBase _pRegWriteDQS[MAX_MIU_NUM][2][18];

RegBase _pRegWDQ1t[MAX_MIU_NUM][2][18];

RegBase _pRegWriteDQsSkew[MAX_MIU_NUM][4];
RegBase _pRegWriteDQsPhase[MAX_MIU_NUM][4];

RegBase _pRegDllTxREFCode[MAX_MIU_NUM];
RegBase _pRegDllTxPD[MAX_MIU_NUM];
RegBase _pRegDllTxKCode[MAX_MIU_NUM];

RegBase _pRegReadDqsmPhase[MAX_MIU_NUM][4];
RegBase _pRegWriteClkPhase[MAX_MIU_NUM];
RegBase _pRegWriteClk1Phase[MAX_MIU_NUM];
RegBase _pRegWriteCsPhase[MAX_MIU_NUM];
RegBase _pRegWriteCs1Phase[MAX_MIU_NUM];
RegBase _pRegWriteCmdPhase[MAX_MIU_NUM][28];
#endif

static char dvs_definit_update = 0;
static char dvs_def_refkcode   = 0;
static char dvs_def_ReadDqsmPhase[4];
static char dvs_def_WriteDqsmPhase[4];
static char dvs_def_WriteDQS[2][18];
static char dvs_def_WriteClkPhase;
static char dvs_def_WriteClk1Phase;
static char dvs_def_WriteCsPhase;
static char dvs_def_WriteCs1Phase;
static char dvs_def_WriteCmdPhase[28];

static u16 cal_average_kcode(u8 miu);
static u16 do_phase_scaling(u16 k_code, u16 ref_kcode, RegBase reg_base, u16 defval, u8 set_reg);

void DumpBankRegs_part(U16 bank, U8 start, U8 end)
{
    U32 i;
    U32 addr = 0;

    // UNUSED_VAR(addr); // for NO_ERR_MSG Warning

    printk("==================\r\n");
    printk("===== BANK: 0x%X\r\n", bank);
    printk("==================\r\n");
    for (i = start; i < end; i = i + 8)
    {
        addr = (bank << 8) + (i << 1);
        printk("%02X: %04X %04X %04X %04X %04X %04X %04X %04X", i, SS_INREG16(addr), SS_INREG16(addr + (1 << 1)),
               SS_INREG16(addr + (2 << 1)), SS_INREG16(addr + (3 << 1)), SS_INREG16(addr + (4 << 1)),
               SS_INREG16(addr + (5 << 1)), SS_INREG16(addr + (6 << 1)), SS_INREG16(addr + (7 << 1)));
    }
    printk("\r\n");
}

U8 RegBitShift(U16 mask)
{
    U8 i, lsh;

    lsh = 0;
    for (i = 0; i < 16; i++)
    {
        if (((mask >> i) & BIT0) == BIT0)
        {
            lsh = i;
            break;
        }
    }
    return lsh;
}

U16 Read_Reg16(RegBase reg)
{
    U16 reg_val = 0;
    U8  lsh;

    lsh = RegBitShift(reg.bank_mask);

    // reg_val = EXTREG16(reg.bank_addr, reg.bank_mask, lsh);
    reg_val = (INREG16(reg.bank_addr) & reg.bank_mask) >> lsh;

    return reg_val;
}

void Write_Reg16(RegBase reg, U16 reg_val)
{
    U8 lsh;

    lsh = RegBitShift(reg.bank_mask);

    // INSREG16(reg.bank_addr, reg.bank_mask, reg_val << lsh);
    OUTREGMSK16(reg.bank_addr, reg_val << lsh, reg.bank_mask);
}

void ReloadDFSReg(void)
{
    unsigned int pll_ch;

    g_deltaD = 0;
    pll_ch   = INREGMSK16(BASE_REG_MIU_DFS1_AC + REG_ID_58, 0x01);

    if (!pll_ch)
    {
        // dfs0
        _pRegRef_MROsc[0].bank_addr = Ref_MROsc_0_Reg;
        _pRegRef_MROsc[0].bank_mask = Ref_MROsc_0_Mask;

        _pRegWriteDQSkew[0][0].bank_addr = WriteDQSkew_0_0_Reg;
        _pRegWriteDQSkew[0][0].bank_mask = WriteDQSkew_0_0_Mask;
        _pRegWriteDQSkew[0][1].bank_addr = WriteDQSkew_0_1_Reg;
        _pRegWriteDQSkew[0][1].bank_mask = WriteDQSkew_0_1_Mask;
        _pRegWriteDQSkew[0][2].bank_addr = WriteDQSkew_0_2_Reg;
        _pRegWriteDQSkew[0][2].bank_mask = WriteDQSkew_0_2_Mask;
        _pRegWriteDQSkew[0][3].bank_addr = WriteDQSkew_0_3_Reg;
        _pRegWriteDQSkew[0][3].bank_mask = WriteDQSkew_0_3_Mask;

        _pRegWriteDQoenSkew[0][0].bank_addr = WriteDQoenSkew_0_0_Reg;
        _pRegWriteDQoenSkew[0][0].bank_mask = WriteDQoenSkew_0_0_Mask;
        _pRegWriteDQoenSkew[0][1].bank_addr = WriteDQoenSkew_0_1_Reg;
        _pRegWriteDQoenSkew[0][1].bank_mask = WriteDQoenSkew_0_1_Mask;
        _pRegWriteDQoenSkew[0][2].bank_addr = WriteDQoenSkew_0_2_Reg;
        _pRegWriteDQoenSkew[0][2].bank_mask = WriteDQoenSkew_0_2_Mask;
        _pRegWriteDQoenSkew[0][3].bank_addr = WriteDQoenSkew_0_3_Reg;
        _pRegWriteDQoenSkew[0][3].bank_mask = WriteDQoenSkew_0_3_Mask;

        _pRegWriteDQS[0][0][0].bank_addr  = WriteDQS_0_0_0_Reg;
        _pRegWriteDQS[0][0][0].bank_mask  = WriteDQS_0_0_0_Mask;
        _pRegWriteDQS[0][0][1].bank_addr  = WriteDQS_0_0_1_Reg;
        _pRegWriteDQS[0][0][1].bank_mask  = WriteDQS_0_0_1_Mask;
        _pRegWriteDQS[0][0][2].bank_addr  = WriteDQS_0_0_2_Reg;
        _pRegWriteDQS[0][0][2].bank_mask  = WriteDQS_0_0_2_Mask;
        _pRegWriteDQS[0][0][3].bank_addr  = WriteDQS_0_0_3_Reg;
        _pRegWriteDQS[0][0][3].bank_mask  = WriteDQS_0_0_3_Mask;
        _pRegWriteDQS[0][0][4].bank_addr  = WriteDQS_0_0_4_Reg;
        _pRegWriteDQS[0][0][4].bank_mask  = WriteDQS_0_0_4_Mask;
        _pRegWriteDQS[0][0][5].bank_addr  = WriteDQS_0_0_5_Reg;
        _pRegWriteDQS[0][0][5].bank_mask  = WriteDQS_0_0_5_Mask;
        _pRegWriteDQS[0][0][6].bank_addr  = WriteDQS_0_0_6_Reg;
        _pRegWriteDQS[0][0][6].bank_mask  = WriteDQS_0_0_6_Mask;
        _pRegWriteDQS[0][0][7].bank_addr  = WriteDQS_0_0_7_Reg;
        _pRegWriteDQS[0][0][7].bank_mask  = WriteDQS_0_0_7_Mask;
        _pRegWriteDQS[0][0][8].bank_addr  = WriteDQS_0_0_8_Reg;
        _pRegWriteDQS[0][0][8].bank_mask  = WriteDQS_0_0_8_Mask;
        _pRegWriteDQS[0][0][9].bank_addr  = WriteDQS_0_0_9_Reg;
        _pRegWriteDQS[0][0][9].bank_mask  = WriteDQS_0_0_9_Mask;
        _pRegWriteDQS[0][0][10].bank_addr = WriteDQS_0_0_10_Reg;
        _pRegWriteDQS[0][0][10].bank_mask = WriteDQS_0_0_10_Mask;
        _pRegWriteDQS[0][0][11].bank_addr = WriteDQS_0_0_11_Reg;
        _pRegWriteDQS[0][0][11].bank_mask = WriteDQS_0_0_11_Mask;
        _pRegWriteDQS[0][0][12].bank_addr = WriteDQS_0_0_12_Reg;
        _pRegWriteDQS[0][0][12].bank_mask = WriteDQS_0_0_12_Mask;
        _pRegWriteDQS[0][0][13].bank_addr = WriteDQS_0_0_13_Reg;
        _pRegWriteDQS[0][0][13].bank_mask = WriteDQS_0_0_13_Mask;
        _pRegWriteDQS[0][0][14].bank_addr = WriteDQS_0_0_14_Reg;
        _pRegWriteDQS[0][0][14].bank_mask = WriteDQS_0_0_14_Mask;
        _pRegWriteDQS[0][0][15].bank_addr = WriteDQS_0_0_15_Reg;
        _pRegWriteDQS[0][0][15].bank_mask = WriteDQS_0_0_15_Mask;
        _pRegWriteDQS[0][0][16].bank_addr = WriteDQS_0_0_16_Reg;
        _pRegWriteDQS[0][0][16].bank_mask = WriteDQS_0_0_16_Mask;
        _pRegWriteDQS[0][0][17].bank_addr = WriteDQS_0_0_17_Reg;
        _pRegWriteDQS[0][0][17].bank_mask = WriteDQS_0_0_17_Mask;
        _pRegWriteDQS[0][1][0].bank_addr  = WriteDQS_0_1_0_Reg;
        _pRegWriteDQS[0][1][0].bank_mask  = WriteDQS_0_1_0_Mask;
        _pRegWriteDQS[0][1][1].bank_addr  = WriteDQS_0_1_1_Reg;
        _pRegWriteDQS[0][1][1].bank_mask  = WriteDQS_0_1_1_Mask;
        _pRegWriteDQS[0][1][2].bank_addr  = WriteDQS_0_1_2_Reg;
        _pRegWriteDQS[0][1][2].bank_mask  = WriteDQS_0_1_2_Mask;
        _pRegWriteDQS[0][1][3].bank_addr  = WriteDQS_0_1_3_Reg;
        _pRegWriteDQS[0][1][3].bank_mask  = WriteDQS_0_1_3_Mask;
        _pRegWriteDQS[0][1][4].bank_addr  = WriteDQS_0_1_4_Reg;
        _pRegWriteDQS[0][1][4].bank_mask  = WriteDQS_0_1_4_Mask;
        _pRegWriteDQS[0][1][5].bank_addr  = WriteDQS_0_1_5_Reg;
        _pRegWriteDQS[0][1][5].bank_mask  = WriteDQS_0_1_5_Mask;
        _pRegWriteDQS[0][1][6].bank_addr  = WriteDQS_0_1_6_Reg;
        _pRegWriteDQS[0][1][6].bank_mask  = WriteDQS_0_1_6_Mask;
        _pRegWriteDQS[0][1][7].bank_addr  = WriteDQS_0_1_7_Reg;
        _pRegWriteDQS[0][1][7].bank_mask  = WriteDQS_0_1_7_Mask;
        _pRegWriteDQS[0][1][8].bank_addr  = WriteDQS_0_1_8_Reg;
        _pRegWriteDQS[0][1][8].bank_mask  = WriteDQS_0_1_8_Mask;
        _pRegWriteDQS[0][1][9].bank_addr  = WriteDQS_0_1_9_Reg;
        _pRegWriteDQS[0][1][9].bank_mask  = WriteDQS_0_1_9_Mask;
        _pRegWriteDQS[0][1][10].bank_addr = WriteDQS_0_1_10_Reg;
        _pRegWriteDQS[0][1][10].bank_mask = WriteDQS_0_1_10_Mask;
        _pRegWriteDQS[0][1][11].bank_addr = WriteDQS_0_1_11_Reg;
        _pRegWriteDQS[0][1][11].bank_mask = WriteDQS_0_1_11_Mask;
        _pRegWriteDQS[0][1][12].bank_addr = WriteDQS_0_1_12_Reg;
        _pRegWriteDQS[0][1][12].bank_mask = WriteDQS_0_1_12_Mask;
        _pRegWriteDQS[0][1][13].bank_addr = WriteDQS_0_1_13_Reg;
        _pRegWriteDQS[0][1][13].bank_mask = WriteDQS_0_1_13_Mask;
        _pRegWriteDQS[0][1][14].bank_addr = WriteDQS_0_1_14_Reg;
        _pRegWriteDQS[0][1][14].bank_mask = WriteDQS_0_1_14_Mask;
        _pRegWriteDQS[0][1][15].bank_addr = WriteDQS_0_1_15_Reg;
        _pRegWriteDQS[0][1][15].bank_mask = WriteDQS_0_1_15_Mask;
        _pRegWriteDQS[0][1][16].bank_addr = WriteDQS_0_1_16_Reg;
        _pRegWriteDQS[0][1][16].bank_mask = WriteDQS_0_1_16_Mask;
        _pRegWriteDQS[0][1][17].bank_addr = WriteDQS_0_1_17_Reg;
        _pRegWriteDQS[0][1][17].bank_mask = WriteDQS_0_1_17_Mask;

        _pRegWDQ1t[0][0][0].bank_addr  = WDQ1t_0_0_0_Reg;
        _pRegWDQ1t[0][0][0].bank_mask  = WDQ1t_0_0_0_Mask;
        _pRegWDQ1t[0][0][1].bank_addr  = WDQ1t_0_0_1_Reg;
        _pRegWDQ1t[0][0][1].bank_mask  = WDQ1t_0_0_1_Mask;
        _pRegWDQ1t[0][0][2].bank_addr  = WDQ1t_0_0_2_Reg;
        _pRegWDQ1t[0][0][2].bank_mask  = WDQ1t_0_0_2_Mask;
        _pRegWDQ1t[0][0][3].bank_addr  = WDQ1t_0_0_3_Reg;
        _pRegWDQ1t[0][0][3].bank_mask  = WDQ1t_0_0_3_Mask;
        _pRegWDQ1t[0][0][4].bank_addr  = WDQ1t_0_0_4_Reg;
        _pRegWDQ1t[0][0][4].bank_mask  = WDQ1t_0_0_4_Mask;
        _pRegWDQ1t[0][0][5].bank_addr  = WDQ1t_0_0_5_Reg;
        _pRegWDQ1t[0][0][5].bank_mask  = WDQ1t_0_0_5_Mask;
        _pRegWDQ1t[0][0][6].bank_addr  = WDQ1t_0_0_6_Reg;
        _pRegWDQ1t[0][0][6].bank_mask  = WDQ1t_0_0_6_Mask;
        _pRegWDQ1t[0][0][7].bank_addr  = WDQ1t_0_0_7_Reg;
        _pRegWDQ1t[0][0][7].bank_mask  = WDQ1t_0_0_7_Mask;
        _pRegWDQ1t[0][0][8].bank_addr  = WDQ1t_0_0_8_Reg;
        _pRegWDQ1t[0][0][8].bank_mask  = WDQ1t_0_0_8_Mask;
        _pRegWDQ1t[0][0][9].bank_addr  = WDQ1t_0_0_9_Reg;
        _pRegWDQ1t[0][0][9].bank_mask  = WDQ1t_0_0_9_Mask;
        _pRegWDQ1t[0][0][10].bank_addr = WDQ1t_0_0_10_Reg;
        _pRegWDQ1t[0][0][10].bank_mask = WDQ1t_0_0_10_Mask;
        _pRegWDQ1t[0][0][11].bank_addr = WDQ1t_0_0_11_Reg;
        _pRegWDQ1t[0][0][11].bank_mask = WDQ1t_0_0_11_Mask;
        _pRegWDQ1t[0][0][12].bank_addr = WDQ1t_0_0_12_Reg;
        _pRegWDQ1t[0][0][12].bank_mask = WDQ1t_0_0_12_Mask;
        _pRegWDQ1t[0][0][13].bank_addr = WDQ1t_0_0_13_Reg;
        _pRegWDQ1t[0][0][13].bank_mask = WDQ1t_0_0_13_Mask;
        _pRegWDQ1t[0][0][14].bank_addr = WDQ1t_0_0_14_Reg;
        _pRegWDQ1t[0][0][14].bank_mask = WDQ1t_0_0_14_Mask;
        _pRegWDQ1t[0][0][15].bank_addr = WDQ1t_0_0_15_Reg;
        _pRegWDQ1t[0][0][15].bank_mask = WDQ1t_0_0_15_Mask;
        _pRegWDQ1t[0][0][16].bank_addr = WDQ1t_0_0_16_Reg;
        _pRegWDQ1t[0][0][16].bank_mask = WDQ1t_0_0_16_Mask;
        _pRegWDQ1t[0][0][17].bank_addr = WDQ1t_0_0_17_Reg;
        _pRegWDQ1t[0][0][17].bank_mask = WDQ1t_0_0_17_Mask;
#if 1
        _pRegWDQ1t[0][1][0].bank_addr  = WDQ1t_0_1_0_Reg;
        _pRegWDQ1t[0][1][0].bank_mask  = WDQ1t_0_1_0_Mask;
        _pRegWDQ1t[0][1][1].bank_addr  = WDQ1t_0_1_1_Reg;
        _pRegWDQ1t[0][1][1].bank_mask  = WDQ1t_0_1_1_Mask;
        _pRegWDQ1t[0][1][2].bank_addr  = WDQ1t_0_1_2_Reg;
        _pRegWDQ1t[0][1][2].bank_mask  = WDQ1t_0_1_2_Mask;
        _pRegWDQ1t[0][1][3].bank_addr  = WDQ1t_0_1_3_Reg;
        _pRegWDQ1t[0][1][3].bank_mask  = WDQ1t_0_1_3_Mask;
        _pRegWDQ1t[0][1][4].bank_addr  = WDQ1t_0_1_4_Reg;
        _pRegWDQ1t[0][1][4].bank_mask  = WDQ1t_0_1_4_Mask;
        _pRegWDQ1t[0][1][5].bank_addr  = WDQ1t_0_1_5_Reg;
        _pRegWDQ1t[0][1][5].bank_mask  = WDQ1t_0_1_5_Mask;
        _pRegWDQ1t[0][1][6].bank_addr  = WDQ1t_0_1_6_Reg;
        _pRegWDQ1t[0][1][6].bank_mask  = WDQ1t_0_1_6_Mask;
        _pRegWDQ1t[0][1][7].bank_addr  = WDQ1t_0_1_7_Reg;
        _pRegWDQ1t[0][1][7].bank_mask  = WDQ1t_0_1_7_Mask;
        _pRegWDQ1t[0][1][8].bank_addr  = WDQ1t_0_1_8_Reg;
        _pRegWDQ1t[0][1][8].bank_mask  = WDQ1t_0_1_8_Mask;
        _pRegWDQ1t[0][1][9].bank_addr  = WDQ1t_0_1_9_Reg;
        _pRegWDQ1t[0][1][9].bank_mask  = WDQ1t_0_1_9_Mask;
        _pRegWDQ1t[0][1][10].bank_addr = WDQ1t_0_1_10_Reg;
        _pRegWDQ1t[0][1][10].bank_mask = WDQ1t_0_1_10_Mask;
        _pRegWDQ1t[0][1][11].bank_addr = WDQ1t_0_1_11_Reg;
        _pRegWDQ1t[0][1][11].bank_mask = WDQ1t_0_1_11_Mask;
        _pRegWDQ1t[0][1][12].bank_addr = WDQ1t_0_1_12_Reg;
        _pRegWDQ1t[0][1][12].bank_mask = WDQ1t_0_1_12_Mask;
        _pRegWDQ1t[0][1][13].bank_addr = WDQ1t_0_1_13_Reg;
        _pRegWDQ1t[0][1][13].bank_mask = WDQ1t_0_1_13_Mask;
        _pRegWDQ1t[0][1][14].bank_addr = WDQ1t_0_1_14_Reg;
        _pRegWDQ1t[0][1][14].bank_mask = WDQ1t_0_1_14_Mask;
        _pRegWDQ1t[0][1][15].bank_addr = WDQ1t_0_1_15_Reg;
        _pRegWDQ1t[0][1][15].bank_mask = WDQ1t_0_1_15_Mask;
        _pRegWDQ1t[0][1][16].bank_addr = WDQ1t_0_1_16_Reg;
        _pRegWDQ1t[0][1][16].bank_mask = WDQ1t_0_1_16_Mask;
        _pRegWDQ1t[0][1][17].bank_addr = WDQ1t_0_1_17_Reg;
        _pRegWDQ1t[0][1][17].bank_mask = WDQ1t_0_1_17_Mask;
#endif
        _pRegDllTxPD[0].bank_addr    = DllTxPD_0_Reg;
        _pRegDllTxPD[0].bank_mask    = DllTxPD_0_Mask;
        _pRegDllTxKCode[0].bank_addr = DllTxKCode_0_Reg;
        _pRegDllTxKCode[0].bank_mask = DllTxKCode_0_Mask;
    }
    else
    {
        // dfs1
        _pRegRef_MROsc[0].bank_addr = DFS1_Ref_MROsc_0_Reg;
        _pRegRef_MROsc[0].bank_mask = DFS1_Ref_MROsc_0_Mask;

        _pRegWriteDQSkew[0][0].bank_addr = DFS1_WriteDQSkew_0_0_Reg;
        _pRegWriteDQSkew[0][0].bank_mask = DFS1_WriteDQSkew_0_0_Mask;
        _pRegWriteDQSkew[0][1].bank_addr = DFS1_WriteDQSkew_0_1_Reg;
        _pRegWriteDQSkew[0][1].bank_mask = DFS1_WriteDQSkew_0_1_Mask;
        _pRegWriteDQSkew[0][2].bank_addr = DFS1_WriteDQSkew_0_2_Reg;
        _pRegWriteDQSkew[0][2].bank_mask = DFS1_WriteDQSkew_0_2_Mask;
        _pRegWriteDQSkew[0][3].bank_addr = DFS1_WriteDQSkew_0_3_Reg;
        _pRegWriteDQSkew[0][3].bank_mask = DFS1_WriteDQSkew_0_3_Mask;

        _pRegWriteDQoenSkew[0][0].bank_addr = DFS1_WriteDQoenSkew_0_0_Reg;
        _pRegWriteDQoenSkew[0][0].bank_mask = DFS1_WriteDQoenSkew_0_0_Mask;
        _pRegWriteDQoenSkew[0][1].bank_addr = DFS1_WriteDQoenSkew_0_1_Reg;
        _pRegWriteDQoenSkew[0][1].bank_mask = DFS1_WriteDQoenSkew_0_1_Mask;
        _pRegWriteDQoenSkew[0][2].bank_addr = DFS1_WriteDQoenSkew_0_2_Reg;
        _pRegWriteDQoenSkew[0][2].bank_mask = DFS1_WriteDQoenSkew_0_2_Mask;
        _pRegWriteDQoenSkew[0][3].bank_addr = DFS1_WriteDQoenSkew_0_3_Reg;
        _pRegWriteDQoenSkew[0][3].bank_mask = DFS1_WriteDQoenSkew_0_3_Mask;

        _pRegWriteDQS[0][0][0].bank_addr  = DFS1_WriteDQS_0_0_0_Reg;
        _pRegWriteDQS[0][0][0].bank_mask  = DFS1_WriteDQS_0_0_0_Mask;
        _pRegWriteDQS[0][0][1].bank_addr  = DFS1_WriteDQS_0_0_1_Reg;
        _pRegWriteDQS[0][0][1].bank_mask  = DFS1_WriteDQS_0_0_1_Mask;
        _pRegWriteDQS[0][0][2].bank_addr  = DFS1_WriteDQS_0_0_2_Reg;
        _pRegWriteDQS[0][0][2].bank_mask  = DFS1_WriteDQS_0_0_2_Mask;
        _pRegWriteDQS[0][0][3].bank_addr  = DFS1_WriteDQS_0_0_3_Reg;
        _pRegWriteDQS[0][0][3].bank_mask  = DFS1_WriteDQS_0_0_3_Mask;
        _pRegWriteDQS[0][0][4].bank_addr  = DFS1_WriteDQS_0_0_4_Reg;
        _pRegWriteDQS[0][0][4].bank_mask  = DFS1_WriteDQS_0_0_4_Mask;
        _pRegWriteDQS[0][0][5].bank_addr  = DFS1_WriteDQS_0_0_5_Reg;
        _pRegWriteDQS[0][0][5].bank_mask  = DFS1_WriteDQS_0_0_5_Mask;
        _pRegWriteDQS[0][0][6].bank_addr  = DFS1_WriteDQS_0_0_6_Reg;
        _pRegWriteDQS[0][0][6].bank_mask  = DFS1_WriteDQS_0_0_6_Mask;
        _pRegWriteDQS[0][0][7].bank_addr  = DFS1_WriteDQS_0_0_7_Reg;
        _pRegWriteDQS[0][0][7].bank_mask  = DFS1_WriteDQS_0_0_7_Mask;
        _pRegWriteDQS[0][0][8].bank_addr  = DFS1_WriteDQS_0_0_8_Reg;
        _pRegWriteDQS[0][0][8].bank_mask  = DFS1_WriteDQS_0_0_8_Mask;
        _pRegWriteDQS[0][0][9].bank_addr  = DFS1_WriteDQS_0_0_9_Reg;
        _pRegWriteDQS[0][0][9].bank_mask  = DFS1_WriteDQS_0_0_9_Mask;
        _pRegWriteDQS[0][0][10].bank_addr = DFS1_WriteDQS_0_0_10_Reg;
        _pRegWriteDQS[0][0][10].bank_mask = DFS1_WriteDQS_0_0_10_Mask;
        _pRegWriteDQS[0][0][11].bank_addr = DFS1_WriteDQS_0_0_11_Reg;
        _pRegWriteDQS[0][0][11].bank_mask = DFS1_WriteDQS_0_0_11_Mask;
        _pRegWriteDQS[0][0][12].bank_addr = DFS1_WriteDQS_0_0_12_Reg;
        _pRegWriteDQS[0][0][12].bank_mask = DFS1_WriteDQS_0_0_12_Mask;
        _pRegWriteDQS[0][0][13].bank_addr = DFS1_WriteDQS_0_0_13_Reg;
        _pRegWriteDQS[0][0][13].bank_mask = DFS1_WriteDQS_0_0_13_Mask;
        _pRegWriteDQS[0][0][14].bank_addr = DFS1_WriteDQS_0_0_14_Reg;
        _pRegWriteDQS[0][0][14].bank_mask = DFS1_WriteDQS_0_0_14_Mask;
        _pRegWriteDQS[0][0][15].bank_addr = DFS1_WriteDQS_0_0_15_Reg;
        _pRegWriteDQS[0][0][15].bank_mask = DFS1_WriteDQS_0_0_15_Mask;
        _pRegWriteDQS[0][0][16].bank_addr = DFS1_WriteDQS_0_0_16_Reg;
        _pRegWriteDQS[0][0][16].bank_mask = DFS1_WriteDQS_0_0_16_Mask;
        _pRegWriteDQS[0][0][17].bank_addr = DFS1_WriteDQS_0_0_17_Reg;
        _pRegWriteDQS[0][0][17].bank_mask = DFS1_WriteDQS_0_0_17_Mask;
        _pRegWriteDQS[0][1][0].bank_addr  = DFS1_WriteDQS_0_1_0_Reg;
        _pRegWriteDQS[0][1][0].bank_mask  = DFS1_WriteDQS_0_1_0_Mask;
        _pRegWriteDQS[0][1][1].bank_addr  = DFS1_WriteDQS_0_1_1_Reg;
        _pRegWriteDQS[0][1][1].bank_mask  = DFS1_WriteDQS_0_1_1_Mask;
        _pRegWriteDQS[0][1][2].bank_addr  = DFS1_WriteDQS_0_1_2_Reg;
        _pRegWriteDQS[0][1][2].bank_mask  = DFS1_WriteDQS_0_1_2_Mask;
        _pRegWriteDQS[0][1][3].bank_addr  = DFS1_WriteDQS_0_1_3_Reg;
        _pRegWriteDQS[0][1][3].bank_mask  = DFS1_WriteDQS_0_1_3_Mask;
        _pRegWriteDQS[0][1][4].bank_addr  = DFS1_WriteDQS_0_1_4_Reg;
        _pRegWriteDQS[0][1][4].bank_mask  = DFS1_WriteDQS_0_1_4_Mask;
        _pRegWriteDQS[0][1][5].bank_addr  = DFS1_WriteDQS_0_1_5_Reg;
        _pRegWriteDQS[0][1][5].bank_mask  = DFS1_WriteDQS_0_1_5_Mask;
        _pRegWriteDQS[0][1][6].bank_addr  = DFS1_WriteDQS_0_1_6_Reg;
        _pRegWriteDQS[0][1][6].bank_mask  = DFS1_WriteDQS_0_1_6_Mask;
        _pRegWriteDQS[0][1][7].bank_addr  = DFS1_WriteDQS_0_1_7_Reg;
        _pRegWriteDQS[0][1][7].bank_mask  = DFS1_WriteDQS_0_1_7_Mask;
        _pRegWriteDQS[0][1][8].bank_addr  = DFS1_WriteDQS_0_1_8_Reg;
        _pRegWriteDQS[0][1][8].bank_mask  = DFS1_WriteDQS_0_1_8_Mask;
        _pRegWriteDQS[0][1][9].bank_addr  = DFS1_WriteDQS_0_1_9_Reg;
        _pRegWriteDQS[0][1][9].bank_mask  = DFS1_WriteDQS_0_1_9_Mask;
        _pRegWriteDQS[0][1][10].bank_addr = DFS1_WriteDQS_0_1_10_Reg;
        _pRegWriteDQS[0][1][10].bank_mask = DFS1_WriteDQS_0_1_10_Mask;
        _pRegWriteDQS[0][1][11].bank_addr = DFS1_WriteDQS_0_1_11_Reg;
        _pRegWriteDQS[0][1][11].bank_mask = DFS1_WriteDQS_0_1_11_Mask;
        _pRegWriteDQS[0][1][12].bank_addr = DFS1_WriteDQS_0_1_12_Reg;
        _pRegWriteDQS[0][1][12].bank_mask = DFS1_WriteDQS_0_1_12_Mask;
        _pRegWriteDQS[0][1][13].bank_addr = DFS1_WriteDQS_0_1_13_Reg;
        _pRegWriteDQS[0][1][13].bank_mask = DFS1_WriteDQS_0_1_13_Mask;
        _pRegWriteDQS[0][1][14].bank_addr = DFS1_WriteDQS_0_1_14_Reg;
        _pRegWriteDQS[0][1][14].bank_mask = DFS1_WriteDQS_0_1_14_Mask;
        _pRegWriteDQS[0][1][15].bank_addr = DFS1_WriteDQS_0_1_15_Reg;
        _pRegWriteDQS[0][1][15].bank_mask = DFS1_WriteDQS_0_1_15_Mask;
        _pRegWriteDQS[0][1][16].bank_addr = DFS1_WriteDQS_0_1_16_Reg;
        _pRegWriteDQS[0][1][16].bank_mask = DFS1_WriteDQS_0_1_16_Mask;
        _pRegWriteDQS[0][1][17].bank_addr = DFS1_WriteDQS_0_1_17_Reg;
        _pRegWriteDQS[0][1][17].bank_mask = DFS1_WriteDQS_0_1_17_Mask;

        _pRegWDQ1t[0][0][0].bank_addr  = DFS1_WDQ1t_0_0_0_Reg;
        _pRegWDQ1t[0][0][0].bank_mask  = DFS1_WDQ1t_0_0_0_Mask;
        _pRegWDQ1t[0][0][1].bank_addr  = DFS1_WDQ1t_0_0_1_Reg;
        _pRegWDQ1t[0][0][1].bank_mask  = DFS1_WDQ1t_0_0_1_Mask;
        _pRegWDQ1t[0][0][2].bank_addr  = DFS1_WDQ1t_0_0_2_Reg;
        _pRegWDQ1t[0][0][2].bank_mask  = DFS1_WDQ1t_0_0_2_Mask;
        _pRegWDQ1t[0][0][3].bank_addr  = DFS1_WDQ1t_0_0_3_Reg;
        _pRegWDQ1t[0][0][3].bank_mask  = DFS1_WDQ1t_0_0_3_Mask;
        _pRegWDQ1t[0][0][4].bank_addr  = DFS1_WDQ1t_0_0_4_Reg;
        _pRegWDQ1t[0][0][4].bank_mask  = DFS1_WDQ1t_0_0_4_Mask;
        _pRegWDQ1t[0][0][5].bank_addr  = DFS1_WDQ1t_0_0_5_Reg;
        _pRegWDQ1t[0][0][5].bank_mask  = DFS1_WDQ1t_0_0_5_Mask;
        _pRegWDQ1t[0][0][6].bank_addr  = DFS1_WDQ1t_0_0_6_Reg;
        _pRegWDQ1t[0][0][6].bank_mask  = DFS1_WDQ1t_0_0_6_Mask;
        _pRegWDQ1t[0][0][7].bank_addr  = DFS1_WDQ1t_0_0_7_Reg;
        _pRegWDQ1t[0][0][7].bank_mask  = DFS1_WDQ1t_0_0_7_Mask;
        _pRegWDQ1t[0][0][8].bank_addr  = DFS1_WDQ1t_0_0_8_Reg;
        _pRegWDQ1t[0][0][8].bank_mask  = DFS1_WDQ1t_0_0_8_Mask;
        _pRegWDQ1t[0][0][9].bank_addr  = DFS1_WDQ1t_0_0_9_Reg;
        _pRegWDQ1t[0][0][9].bank_mask  = DFS1_WDQ1t_0_0_9_Mask;
        _pRegWDQ1t[0][0][10].bank_addr = DFS1_WDQ1t_0_0_10_Reg;
        _pRegWDQ1t[0][0][10].bank_mask = DFS1_WDQ1t_0_0_10_Mask;
        _pRegWDQ1t[0][0][11].bank_addr = DFS1_WDQ1t_0_0_11_Reg;
        _pRegWDQ1t[0][0][11].bank_mask = DFS1_WDQ1t_0_0_11_Mask;
        _pRegWDQ1t[0][0][12].bank_addr = DFS1_WDQ1t_0_0_12_Reg;
        _pRegWDQ1t[0][0][12].bank_mask = DFS1_WDQ1t_0_0_12_Mask;
        _pRegWDQ1t[0][0][13].bank_addr = DFS1_WDQ1t_0_0_13_Reg;
        _pRegWDQ1t[0][0][13].bank_mask = DFS1_WDQ1t_0_0_13_Mask;
        _pRegWDQ1t[0][0][14].bank_addr = DFS1_WDQ1t_0_0_14_Reg;
        _pRegWDQ1t[0][0][14].bank_mask = DFS1_WDQ1t_0_0_14_Mask;
        _pRegWDQ1t[0][0][15].bank_addr = DFS1_WDQ1t_0_0_15_Reg;
        _pRegWDQ1t[0][0][15].bank_mask = DFS1_WDQ1t_0_0_15_Mask;
        _pRegWDQ1t[0][0][16].bank_addr = DFS1_WDQ1t_0_0_16_Reg;
        _pRegWDQ1t[0][0][16].bank_mask = DFS1_WDQ1t_0_0_16_Mask;
        _pRegWDQ1t[0][0][17].bank_addr = DFS1_WDQ1t_0_0_17_Reg;
        _pRegWDQ1t[0][0][17].bank_mask = DFS1_WDQ1t_0_0_17_Mask;

        _pRegDllTxPD[0].bank_addr    = DFS1_DllTxPD_0_Reg;
        _pRegDllTxPD[0].bank_mask    = DFS1_DllTxPD_0_Mask;
        _pRegDllTxKCode[0].bank_addr = DFS1_DllTxKCode_0_Reg;
        _pRegDllTxKCode[0].bank_mask = DFS1_DllTxKCode_0_Mask;
    }
}

void LoadRegMiu(void)
{
    _pRegResult_MRR[0].bank_addr  = Result_MRR_0_Reg;
    _pRegResult_MRR[0].bank_mask  = Result_MRR_0_Mask;
    _pRegDynUpd_EN[0].bank_addr   = DynUpd_EN_0_Reg;
    _pRegDynUpd_EN[0].bank_mask   = DynUpd_EN_0_Mask;
    _pRegDynUpd_halt[0].bank_addr = DynUpd_halt_0_Reg;
    _pRegDynUpd_halt[0].bank_mask = DynUpd_halt_0_Mask;
    _pRegRef_MROsc[0].bank_addr   = Ref_MROsc_0_Reg;
    _pRegRef_MROsc[0].bank_mask   = Ref_MROsc_0_Mask;

    _pRegWriteDQSkew[0][0].bank_addr = WriteDQSkew_0_0_Reg;
    _pRegWriteDQSkew[0][0].bank_mask = WriteDQSkew_0_0_Mask;
    _pRegWriteDQSkew[0][1].bank_addr = WriteDQSkew_0_1_Reg;
    _pRegWriteDQSkew[0][1].bank_mask = WriteDQSkew_0_1_Mask;
    _pRegWriteDQSkew[0][2].bank_addr = WriteDQSkew_0_2_Reg;
    _pRegWriteDQSkew[0][2].bank_mask = WriteDQSkew_0_2_Mask;
    _pRegWriteDQSkew[0][3].bank_addr = WriteDQSkew_0_3_Reg;
    _pRegWriteDQSkew[0][3].bank_mask = WriteDQSkew_0_3_Mask;

    _pRegWriteDQoenSkew[0][0].bank_addr = WriteDQoenSkew_0_0_Reg;
    _pRegWriteDQoenSkew[0][0].bank_mask = WriteDQoenSkew_0_0_Mask;
    _pRegWriteDQoenSkew[0][1].bank_addr = WriteDQoenSkew_0_1_Reg;
    _pRegWriteDQoenSkew[0][1].bank_mask = WriteDQoenSkew_0_1_Mask;
    _pRegWriteDQoenSkew[0][2].bank_addr = WriteDQoenSkew_0_2_Reg;
    _pRegWriteDQoenSkew[0][2].bank_mask = WriteDQoenSkew_0_2_Mask;
    _pRegWriteDQoenSkew[0][3].bank_addr = WriteDQoenSkew_0_3_Reg;
    _pRegWriteDQoenSkew[0][3].bank_mask = WriteDQoenSkew_0_3_Mask;

    _pRegWriteDQS[0][0][0].bank_addr  = WriteDQS_0_0_0_Reg;
    _pRegWriteDQS[0][0][0].bank_mask  = WriteDQS_0_0_0_Mask;
    _pRegWriteDQS[0][0][1].bank_addr  = WriteDQS_0_0_1_Reg;
    _pRegWriteDQS[0][0][1].bank_mask  = WriteDQS_0_0_1_Mask;
    _pRegWriteDQS[0][0][2].bank_addr  = WriteDQS_0_0_2_Reg;
    _pRegWriteDQS[0][0][2].bank_mask  = WriteDQS_0_0_2_Mask;
    _pRegWriteDQS[0][0][3].bank_addr  = WriteDQS_0_0_3_Reg;
    _pRegWriteDQS[0][0][3].bank_mask  = WriteDQS_0_0_3_Mask;
    _pRegWriteDQS[0][0][4].bank_addr  = WriteDQS_0_0_4_Reg;
    _pRegWriteDQS[0][0][4].bank_mask  = WriteDQS_0_0_4_Mask;
    _pRegWriteDQS[0][0][5].bank_addr  = WriteDQS_0_0_5_Reg;
    _pRegWriteDQS[0][0][5].bank_mask  = WriteDQS_0_0_5_Mask;
    _pRegWriteDQS[0][0][6].bank_addr  = WriteDQS_0_0_6_Reg;
    _pRegWriteDQS[0][0][6].bank_mask  = WriteDQS_0_0_6_Mask;
    _pRegWriteDQS[0][0][7].bank_addr  = WriteDQS_0_0_7_Reg;
    _pRegWriteDQS[0][0][7].bank_mask  = WriteDQS_0_0_7_Mask;
    _pRegWriteDQS[0][0][8].bank_addr  = WriteDQS_0_0_8_Reg;
    _pRegWriteDQS[0][0][8].bank_mask  = WriteDQS_0_0_8_Mask;
    _pRegWriteDQS[0][0][9].bank_addr  = WriteDQS_0_0_9_Reg;
    _pRegWriteDQS[0][0][9].bank_mask  = WriteDQS_0_0_9_Mask;
    _pRegWriteDQS[0][0][10].bank_addr = WriteDQS_0_0_10_Reg;
    _pRegWriteDQS[0][0][10].bank_mask = WriteDQS_0_0_10_Mask;
    _pRegWriteDQS[0][0][11].bank_addr = WriteDQS_0_0_11_Reg;
    _pRegWriteDQS[0][0][11].bank_mask = WriteDQS_0_0_11_Mask;
    _pRegWriteDQS[0][0][12].bank_addr = WriteDQS_0_0_12_Reg;
    _pRegWriteDQS[0][0][12].bank_mask = WriteDQS_0_0_12_Mask;
    _pRegWriteDQS[0][0][13].bank_addr = WriteDQS_0_0_13_Reg;
    _pRegWriteDQS[0][0][13].bank_mask = WriteDQS_0_0_13_Mask;
    _pRegWriteDQS[0][0][14].bank_addr = WriteDQS_0_0_14_Reg;
    _pRegWriteDQS[0][0][14].bank_mask = WriteDQS_0_0_14_Mask;
    _pRegWriteDQS[0][0][15].bank_addr = WriteDQS_0_0_15_Reg;
    _pRegWriteDQS[0][0][15].bank_mask = WriteDQS_0_0_15_Mask;
    _pRegWriteDQS[0][0][16].bank_addr = WriteDQS_0_0_16_Reg;
    _pRegWriteDQS[0][0][16].bank_mask = WriteDQS_0_0_16_Mask;
    _pRegWriteDQS[0][0][17].bank_addr = WriteDQS_0_0_17_Reg;
    _pRegWriteDQS[0][0][17].bank_mask = WriteDQS_0_0_17_Mask;
    _pRegWriteDQS[0][1][0].bank_addr  = WriteDQS_0_1_0_Reg;
    _pRegWriteDQS[0][1][0].bank_mask  = WriteDQS_0_1_0_Mask;
    _pRegWriteDQS[0][1][1].bank_addr  = WriteDQS_0_1_1_Reg;
    _pRegWriteDQS[0][1][1].bank_mask  = WriteDQS_0_1_1_Mask;
    _pRegWriteDQS[0][1][2].bank_addr  = WriteDQS_0_1_2_Reg;
    _pRegWriteDQS[0][1][2].bank_mask  = WriteDQS_0_1_2_Mask;
    _pRegWriteDQS[0][1][3].bank_addr  = WriteDQS_0_1_3_Reg;
    _pRegWriteDQS[0][1][3].bank_mask  = WriteDQS_0_1_3_Mask;
    _pRegWriteDQS[0][1][4].bank_addr  = WriteDQS_0_1_4_Reg;
    _pRegWriteDQS[0][1][4].bank_mask  = WriteDQS_0_1_4_Mask;
    _pRegWriteDQS[0][1][5].bank_addr  = WriteDQS_0_1_5_Reg;
    _pRegWriteDQS[0][1][5].bank_mask  = WriteDQS_0_1_5_Mask;
    _pRegWriteDQS[0][1][6].bank_addr  = WriteDQS_0_1_6_Reg;
    _pRegWriteDQS[0][1][6].bank_mask  = WriteDQS_0_1_6_Mask;
    _pRegWriteDQS[0][1][7].bank_addr  = WriteDQS_0_1_7_Reg;
    _pRegWriteDQS[0][1][7].bank_mask  = WriteDQS_0_1_7_Mask;
    _pRegWriteDQS[0][1][8].bank_addr  = WriteDQS_0_1_8_Reg;
    _pRegWriteDQS[0][1][8].bank_mask  = WriteDQS_0_1_8_Mask;
    _pRegWriteDQS[0][1][9].bank_addr  = WriteDQS_0_1_9_Reg;
    _pRegWriteDQS[0][1][9].bank_mask  = WriteDQS_0_1_9_Mask;
    _pRegWriteDQS[0][1][10].bank_addr = WriteDQS_0_1_10_Reg;
    _pRegWriteDQS[0][1][10].bank_mask = WriteDQS_0_1_10_Mask;
    _pRegWriteDQS[0][1][11].bank_addr = WriteDQS_0_1_11_Reg;
    _pRegWriteDQS[0][1][11].bank_mask = WriteDQS_0_1_11_Mask;
    _pRegWriteDQS[0][1][12].bank_addr = WriteDQS_0_1_12_Reg;
    _pRegWriteDQS[0][1][12].bank_mask = WriteDQS_0_1_12_Mask;
    _pRegWriteDQS[0][1][13].bank_addr = WriteDQS_0_1_13_Reg;
    _pRegWriteDQS[0][1][13].bank_mask = WriteDQS_0_1_13_Mask;
    _pRegWriteDQS[0][1][14].bank_addr = WriteDQS_0_1_14_Reg;
    _pRegWriteDQS[0][1][14].bank_mask = WriteDQS_0_1_14_Mask;
    _pRegWriteDQS[0][1][15].bank_addr = WriteDQS_0_1_15_Reg;
    _pRegWriteDQS[0][1][15].bank_mask = WriteDQS_0_1_15_Mask;
    _pRegWriteDQS[0][1][16].bank_addr = WriteDQS_0_1_16_Reg;
    _pRegWriteDQS[0][1][16].bank_mask = WriteDQS_0_1_16_Mask;
    _pRegWriteDQS[0][1][17].bank_addr = WriteDQS_0_1_17_Reg;
    _pRegWriteDQS[0][1][17].bank_mask = WriteDQS_0_1_17_Mask;

    _pRegWDQ1t[0][0][0].bank_addr  = WDQ1t_0_0_0_Reg;
    _pRegWDQ1t[0][0][0].bank_mask  = WDQ1t_0_0_0_Mask;
    _pRegWDQ1t[0][0][1].bank_addr  = WDQ1t_0_0_1_Reg;
    _pRegWDQ1t[0][0][1].bank_mask  = WDQ1t_0_0_1_Mask;
    _pRegWDQ1t[0][0][2].bank_addr  = WDQ1t_0_0_2_Reg;
    _pRegWDQ1t[0][0][2].bank_mask  = WDQ1t_0_0_2_Mask;
    _pRegWDQ1t[0][0][3].bank_addr  = WDQ1t_0_0_3_Reg;
    _pRegWDQ1t[0][0][3].bank_mask  = WDQ1t_0_0_3_Mask;
    _pRegWDQ1t[0][0][4].bank_addr  = WDQ1t_0_0_4_Reg;
    _pRegWDQ1t[0][0][4].bank_mask  = WDQ1t_0_0_4_Mask;
    _pRegWDQ1t[0][0][5].bank_addr  = WDQ1t_0_0_5_Reg;
    _pRegWDQ1t[0][0][5].bank_mask  = WDQ1t_0_0_5_Mask;
    _pRegWDQ1t[0][0][6].bank_addr  = WDQ1t_0_0_6_Reg;
    _pRegWDQ1t[0][0][6].bank_mask  = WDQ1t_0_0_6_Mask;
    _pRegWDQ1t[0][0][7].bank_addr  = WDQ1t_0_0_7_Reg;
    _pRegWDQ1t[0][0][7].bank_mask  = WDQ1t_0_0_7_Mask;
    _pRegWDQ1t[0][0][8].bank_addr  = WDQ1t_0_0_8_Reg;
    _pRegWDQ1t[0][0][8].bank_mask  = WDQ1t_0_0_8_Mask;
    _pRegWDQ1t[0][0][9].bank_addr  = WDQ1t_0_0_9_Reg;
    _pRegWDQ1t[0][0][9].bank_mask  = WDQ1t_0_0_9_Mask;
    _pRegWDQ1t[0][0][10].bank_addr = WDQ1t_0_0_10_Reg;
    _pRegWDQ1t[0][0][10].bank_mask = WDQ1t_0_0_10_Mask;
    _pRegWDQ1t[0][0][11].bank_addr = WDQ1t_0_0_11_Reg;
    _pRegWDQ1t[0][0][11].bank_mask = WDQ1t_0_0_11_Mask;
    _pRegWDQ1t[0][0][12].bank_addr = WDQ1t_0_0_12_Reg;
    _pRegWDQ1t[0][0][12].bank_mask = WDQ1t_0_0_12_Mask;
    _pRegWDQ1t[0][0][13].bank_addr = WDQ1t_0_0_13_Reg;
    _pRegWDQ1t[0][0][13].bank_mask = WDQ1t_0_0_13_Mask;
    _pRegWDQ1t[0][0][14].bank_addr = WDQ1t_0_0_14_Reg;
    _pRegWDQ1t[0][0][14].bank_mask = WDQ1t_0_0_14_Mask;
    _pRegWDQ1t[0][0][15].bank_addr = WDQ1t_0_0_15_Reg;
    _pRegWDQ1t[0][0][15].bank_mask = WDQ1t_0_0_15_Mask;
    _pRegWDQ1t[0][0][16].bank_addr = WDQ1t_0_0_16_Reg;
    _pRegWDQ1t[0][0][16].bank_mask = WDQ1t_0_0_16_Mask;
    _pRegWDQ1t[0][0][17].bank_addr = WDQ1t_0_0_17_Reg;
    _pRegWDQ1t[0][0][17].bank_mask = WDQ1t_0_0_17_Mask;
#if 1
    _pRegWDQ1t[0][1][0].bank_addr  = WDQ1t_0_1_0_Reg;
    _pRegWDQ1t[0][1][0].bank_mask  = WDQ1t_0_1_0_Mask;
    _pRegWDQ1t[0][1][1].bank_addr  = WDQ1t_0_1_1_Reg;
    _pRegWDQ1t[0][1][1].bank_mask  = WDQ1t_0_1_1_Mask;
    _pRegWDQ1t[0][1][2].bank_addr  = WDQ1t_0_1_2_Reg;
    _pRegWDQ1t[0][1][2].bank_mask  = WDQ1t_0_1_2_Mask;
    _pRegWDQ1t[0][1][3].bank_addr  = WDQ1t_0_1_3_Reg;
    _pRegWDQ1t[0][1][3].bank_mask  = WDQ1t_0_1_3_Mask;
    _pRegWDQ1t[0][1][4].bank_addr  = WDQ1t_0_1_4_Reg;
    _pRegWDQ1t[0][1][4].bank_mask  = WDQ1t_0_1_4_Mask;
    _pRegWDQ1t[0][1][5].bank_addr  = WDQ1t_0_1_5_Reg;
    _pRegWDQ1t[0][1][5].bank_mask  = WDQ1t_0_1_5_Mask;
    _pRegWDQ1t[0][1][6].bank_addr  = WDQ1t_0_1_6_Reg;
    _pRegWDQ1t[0][1][6].bank_mask  = WDQ1t_0_1_6_Mask;
    _pRegWDQ1t[0][1][7].bank_addr  = WDQ1t_0_1_7_Reg;
    _pRegWDQ1t[0][1][7].bank_mask  = WDQ1t_0_1_7_Mask;
    _pRegWDQ1t[0][1][8].bank_addr  = WDQ1t_0_1_8_Reg;
    _pRegWDQ1t[0][1][8].bank_mask  = WDQ1t_0_1_8_Mask;
    _pRegWDQ1t[0][1][9].bank_addr  = WDQ1t_0_1_9_Reg;
    _pRegWDQ1t[0][1][9].bank_mask  = WDQ1t_0_1_9_Mask;
    _pRegWDQ1t[0][1][10].bank_addr = WDQ1t_0_1_10_Reg;
    _pRegWDQ1t[0][1][10].bank_mask = WDQ1t_0_1_10_Mask;
    _pRegWDQ1t[0][1][11].bank_addr = WDQ1t_0_1_11_Reg;
    _pRegWDQ1t[0][1][11].bank_mask = WDQ1t_0_1_11_Mask;
    _pRegWDQ1t[0][1][12].bank_addr = WDQ1t_0_1_12_Reg;
    _pRegWDQ1t[0][1][12].bank_mask = WDQ1t_0_1_12_Mask;
    _pRegWDQ1t[0][1][13].bank_addr = WDQ1t_0_1_13_Reg;
    _pRegWDQ1t[0][1][13].bank_mask = WDQ1t_0_1_13_Mask;
    _pRegWDQ1t[0][1][14].bank_addr = WDQ1t_0_1_14_Reg;
    _pRegWDQ1t[0][1][14].bank_mask = WDQ1t_0_1_14_Mask;
    _pRegWDQ1t[0][1][15].bank_addr = WDQ1t_0_1_15_Reg;
    _pRegWDQ1t[0][1][15].bank_mask = WDQ1t_0_1_15_Mask;
    _pRegWDQ1t[0][1][16].bank_addr = WDQ1t_0_1_16_Reg;
    _pRegWDQ1t[0][1][16].bank_mask = WDQ1t_0_1_16_Mask;
    _pRegWDQ1t[0][1][17].bank_addr = WDQ1t_0_1_17_Reg;
    _pRegWDQ1t[0][1][17].bank_mask = WDQ1t_0_1_17_Mask;
#endif

    _pRegDllTxREFCode[0].bank_addr = DllTxREFCode_0_Reg;
    _pRegDllTxREFCode[0].bank_mask = DllTxREFCode_0_Mask;

    _pRegWriteDQsSkew[0][0].bank_addr  = WriteDQsSkew_0_0_Reg;
    _pRegWriteDQsSkew[0][0].bank_mask  = WriteDQsSkew_0_0_Mask;
    _pRegWriteDQsSkew[0][1].bank_addr  = WriteDQsSkew_0_1_Reg;
    _pRegWriteDQsSkew[0][1].bank_mask  = WriteDQsSkew_0_1_Mask;
    _pRegWriteDQsSkew[0][2].bank_addr  = WriteDQsSkew_0_2_Reg;
    _pRegWriteDQsSkew[0][2].bank_mask  = WriteDQsSkew_0_2_Mask;
    _pRegWriteDQsSkew[0][3].bank_addr  = WriteDQsSkew_0_3_Reg;
    _pRegWriteDQsSkew[0][3].bank_mask  = WriteDQsSkew_0_3_Mask;
    _pRegWriteDQsPhase[0][0].bank_addr = WriteDQsPhase_0_0_Reg;
    _pRegWriteDQsPhase[0][0].bank_mask = WriteDQsPhase_0_0_Mask;
    _pRegWriteDQsPhase[0][1].bank_addr = WriteDQsPhase_0_1_Reg;
    _pRegWriteDQsPhase[0][1].bank_mask = WriteDQsPhase_0_1_Mask;
    _pRegWriteDQsPhase[0][2].bank_addr = WriteDQsPhase_0_2_Reg;
    _pRegWriteDQsPhase[0][2].bank_mask = WriteDQsPhase_0_2_Mask;
    _pRegWriteDQsPhase[0][3].bank_addr = WriteDQsPhase_0_3_Reg;
    _pRegWriteDQsPhase[0][3].bank_mask = WriteDQsPhase_0_3_Mask;

    _pRegDllTxPD[0].bank_addr    = DllTxPD_0_Reg;
    _pRegDllTxPD[0].bank_mask    = DllTxPD_0_Mask;
    _pRegDllTxKCode[0].bank_addr = DllTxKCode_0_Reg;
    _pRegDllTxKCode[0].bank_mask = DllTxKCode_0_Mask;

#if 1
    _pRegReadDqsmPhase[0][0].bank_addr = ReadDqsmPhase_0_0_Reg;
    _pRegReadDqsmPhase[0][0].bank_mask = ReadDqsmPhase_0_0_Mask;
    _pRegReadDqsmPhase[0][1].bank_addr = ReadDqsmPhase_0_1_Reg;
    _pRegReadDqsmPhase[0][1].bank_mask = ReadDqsmPhase_0_1_Mask;
    _pRegReadDqsmPhase[0][2].bank_addr = ReadDqsmPhase_0_2_Reg;
    _pRegReadDqsmPhase[0][2].bank_mask = ReadDqsmPhase_0_2_Mask;
    _pRegReadDqsmPhase[0][3].bank_addr = ReadDqsmPhase_0_3_Reg;
    _pRegReadDqsmPhase[0][3].bank_mask = ReadDqsmPhase_0_3_Mask;

    _pRegWriteClkPhase[0].bank_addr  = WriteClkPhase_0_Reg;
    _pRegWriteClkPhase[0].bank_mask  = WriteClkPhase_0_Mask;
    _pRegWriteClk1Phase[0].bank_addr = WriteClk1Phase_0_Reg;
    _pRegWriteClk1Phase[0].bank_mask = WriteClk1Phase_0_Mask;
    _pRegWriteCsPhase[0].bank_addr   = WriteCsPhase_0_Reg;
    _pRegWriteCsPhase[0].bank_mask   = WriteCsPhase_0_Mask;
    _pRegWriteCs1Phase[0].bank_addr  = WriteCs1Phase_0_Reg;
    _pRegWriteCs1Phase[0].bank_mask  = WriteCs1Phase_0_Mask;

    _pRegWriteCmdPhase[0][0].bank_addr  = WriteCmdPhase_0_0_Reg;
    _pRegWriteCmdPhase[0][0].bank_mask  = WriteCmdPhase_0_0_Mask;
    _pRegWriteCmdPhase[0][1].bank_addr  = WriteCmdPhase_0_1_Reg;
    _pRegWriteCmdPhase[0][1].bank_mask  = WriteCmdPhase_0_1_Mask;
    _pRegWriteCmdPhase[0][2].bank_addr  = WriteCmdPhase_0_2_Reg;
    _pRegWriteCmdPhase[0][2].bank_mask  = WriteCmdPhase_0_2_Mask;
    _pRegWriteCmdPhase[0][3].bank_addr  = WriteCmdPhase_0_3_Reg;
    _pRegWriteCmdPhase[0][3].bank_mask  = WriteCmdPhase_0_3_Mask;
    _pRegWriteCmdPhase[0][4].bank_addr  = WriteCmdPhase_0_4_Reg;
    _pRegWriteCmdPhase[0][4].bank_mask  = WriteCmdPhase_0_4_Mask;
    _pRegWriteCmdPhase[0][5].bank_addr  = WriteCmdPhase_0_5_Reg;
    _pRegWriteCmdPhase[0][5].bank_mask  = WriteCmdPhase_0_5_Mask;
    _pRegWriteCmdPhase[0][6].bank_addr  = WriteCmdPhase_0_6_Reg;
    _pRegWriteCmdPhase[0][6].bank_mask  = WriteCmdPhase_0_6_Mask;
    _pRegWriteCmdPhase[0][7].bank_addr  = WriteCmdPhase_0_7_Reg;
    _pRegWriteCmdPhase[0][7].bank_mask  = WriteCmdPhase_0_7_Mask;
    _pRegWriteCmdPhase[0][8].bank_addr  = WriteCmdPhase_0_8_Reg;
    _pRegWriteCmdPhase[0][8].bank_mask  = WriteCmdPhase_0_8_Mask;
    _pRegWriteCmdPhase[0][9].bank_addr  = WriteCmdPhase_0_9_Reg;
    _pRegWriteCmdPhase[0][9].bank_mask  = WriteCmdPhase_0_9_Mask;
    _pRegWriteCmdPhase[0][10].bank_addr = WriteCmdPhase_0_10_Reg;
    _pRegWriteCmdPhase[0][10].bank_mask = WriteCmdPhase_0_10_Mask;
    _pRegWriteCmdPhase[0][11].bank_addr = WriteCmdPhase_0_11_Reg;
    _pRegWriteCmdPhase[0][11].bank_mask = WriteCmdPhase_0_11_Mask;
    _pRegWriteCmdPhase[0][12].bank_addr = WriteCmdPhase_0_12_Reg;
    _pRegWriteCmdPhase[0][12].bank_mask = WriteCmdPhase_0_12_Mask;
    _pRegWriteCmdPhase[0][13].bank_addr = WriteCmdPhase_0_13_Reg;
    _pRegWriteCmdPhase[0][13].bank_mask = WriteCmdPhase_0_13_Mask;
    _pRegWriteCmdPhase[0][14].bank_addr = WriteCmdPhase_0_14_Reg;
    _pRegWriteCmdPhase[0][14].bank_mask = WriteCmdPhase_0_14_Mask;
    _pRegWriteCmdPhase[0][15].bank_addr = WriteCmdPhase_0_15_Reg;
    _pRegWriteCmdPhase[0][15].bank_mask = WriteCmdPhase_0_15_Mask;
    _pRegWriteCmdPhase[0][16].bank_addr = WriteCmdPhase_0_16_Reg;
    _pRegWriteCmdPhase[0][16].bank_mask = WriteCmdPhase_0_16_Mask;
    _pRegWriteCmdPhase[0][17].bank_addr = WriteCmdPhase_0_17_Reg;
    _pRegWriteCmdPhase[0][17].bank_mask = WriteCmdPhase_0_17_Mask;
    _pRegWriteCmdPhase[0][18].bank_addr = WriteCmdPhase_0_18_Reg;
    _pRegWriteCmdPhase[0][18].bank_mask = WriteCmdPhase_0_18_Mask;
    _pRegWriteCmdPhase[0][19].bank_addr = WriteCmdPhase_0_19_Reg;
    _pRegWriteCmdPhase[0][19].bank_mask = WriteCmdPhase_0_19_Mask;
    _pRegWriteCmdPhase[0][20].bank_addr = WriteCmdPhase_0_20_Reg;
    _pRegWriteCmdPhase[0][20].bank_mask = WriteCmdPhase_0_20_Mask;
    _pRegWriteCmdPhase[0][21].bank_addr = WriteCmdPhase_0_21_Reg;
    _pRegWriteCmdPhase[0][21].bank_mask = WriteCmdPhase_0_21_Mask;
    _pRegWriteCmdPhase[0][22].bank_addr = WriteCmdPhase_0_22_Reg;
    _pRegWriteCmdPhase[0][22].bank_mask = WriteCmdPhase_0_22_Mask;
    _pRegWriteCmdPhase[0][23].bank_addr = WriteCmdPhase_0_23_Reg;
    _pRegWriteCmdPhase[0][23].bank_mask = WriteCmdPhase_0_23_Mask;
    _pRegWriteCmdPhase[0][24].bank_addr = WriteCmdPhase_0_24_Reg;
    _pRegWriteCmdPhase[0][24].bank_mask = WriteCmdPhase_0_24_Mask;
    _pRegWriteCmdPhase[0][25].bank_addr = WriteCmdPhase_0_25_Reg;
    _pRegWriteCmdPhase[0][25].bank_mask = WriteCmdPhase_0_25_Mask;
    _pRegWriteCmdPhase[0][26].bank_addr = WriteCmdPhase_0_26_Reg;
    _pRegWriteCmdPhase[0][26].bank_mask = WriteCmdPhase_0_26_Mask;
    _pRegWriteCmdPhase[0][27].bank_addr = WriteCmdPhase_0_27_Reg;
    _pRegWriteCmdPhase[0][27].bank_mask = WriteCmdPhase_0_27_Mask;
#endif
}

int Hal_ReloadPhaseRegs(void)
{
    ReloadDFSReg();

    return 0;
}

int Hal_Check_LPDDR4(void)
{
    U32 bank_atop;

    bank_atop = MIU0_ATOP_BASE;

    if (SS_INREG16(bank_atop + (0x5A << 1)) & BIT8)
    {
        if ((SS_INREG16(bank_atop + (0x5B << 1)) & BIT15) == 0)
        {
            return 1;
        }
    }

    return 0;
}
#if 0
void Hal_Set_WaitTime(int wait_time)
{
    wait_time_period = wait_time;
}

int Hal_Get_WaitTime(void)
{
    return wait_time_period;
}
#endif
void Hal_Set_DCDL_Mode(int dcdl_mode)
{
    dcdl_scl_mode = dcdl_mode;
}

int Hal_Get_DCDL_Mode(void)
{
    return dcdl_scl_mode;
}

void Issue_MRW23(U8 miu)
{
    U16 i;
    U32 bank_base0;

    // UNUSED_VAR(miu);

    bank_base0 = MIU0_SYNOPSYS_BASE;

    // wriu -w 0x161340 0x0605
    // SS_OUTREG32(bank_base0+0x40, 0x0605);

    SS_OUTREG32(bank_base0 + 0x22, 0x1740);
    SS_OUTREG32(bank_base0 + 0x20, 0x8010);

    // wait [0] single cmd launched done
    i = 0;
    while ((SS_INREG16(bank_base0 + 0x2A) & BIT0) != BIT0)
    {
        udelay(1);
        i++;
        if (i > 1000)
        {
            printk("issue MRW23 wait more time\r\n");
            i = 0;
        }
    }

    SS_OUTREG32((bank_base0 + 0x20), 0x8100);
    SS_OUTREG32((bank_base0 + 0x20), 0x8000);
}

U16 Issue_MRR(U8 miu, U8 mr_val)
{
    U16 i;
    U16 mr_value;
    U32 bank_base0;

    // UNUSED_VAR(miu);

    bank_base0 = MIU0_SYNOPSYS_BASE;

    SS_OUTREG32(bank_base0 + 0x22, 0x0000 | (mr_val << 8));
    SS_OUTREG32(bank_base0 + 0x20, 0x8018);

#if 0
    printk("issue_MRR 0x%X\r\n", mr_val);
#endif

    // wait [0] & [1] single cmd launched done
    i = 0;
    while ((SS_INREG16(bank_base0 + 0x2A) & BIT0) != BIT0 || (SS_INREG16(bank_base0 + 0x2A) & BIT1) != BIT1)
    {
        udelay(1);
        i++;
        if (i > 1000)
        {
            printk("Issue_MRR 0x%X wait more time\r\n", mr_val);
            i = 0;
        }
    }

    mr_value = Read_Reg16(_pRegResult_MRR[miu]);

    SS_OUTREG32((bank_base0 + 0x20), 0x8005);
    SS_OUTREG32((bank_base0 + 0x20), 0x8108);
    SS_OUTREG32((bank_base0 + 0x20), 0x8008);

    return mr_value;
}

void Issue_EN_SCL_1(void)
{
    OUTREGMSK16(GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164700 + (0x1f << 1)), BIT0, BIT0);
    udelay(1);
    OUTREGMSK16(GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164800 + (0x67 << 1)), BIT0, BIT0);
    OUTREGMSK16(GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164800 + (0x67 << 1)), BIT1, BIT1);
    OUTREGMSK16(GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164700 + (0x44 << 1)), dcdl_scl_mode, 0x0f00);
    OUTREGMSK16(GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164700 + (0x74 << 1)), BIT4, BIT4);
    udelay(1);
    OUTREGMSK16(GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164800 + (0x54 << 1)), 0, BIT1);
    udelay(1);
    OUTREGMSK16(GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164700 + (0x1f << 1)), 0, BIT0);
}

void Set_DQS_Osc_start(U8 miu)
{
    U16 i;
    U32 bank_base0;

    // UNUSED_VAR(miu);

    bank_base0 = MIU0_SYNOPSYS_BASE;

    // single command MPC DQS Osc Start
    SS_OUTREG32(bank_base0 + 0x22, 0x004b);
    SS_OUTREG32(bank_base0 + 0x20, 0x8019);

    // wait [0] single cmd launched done
    i = 0;
    while ((SS_INREG16(bank_base0 + 0x2A) & BIT0) != BIT0)
    {
        udelay(1);
        i++;
        if (i > 1000)
        {
            printk("DQS Osc start wait more time\r\n");
            i = 0;
        }
    }

    SS_OUTREG32((bank_base0 + 0x20), 0x8109);
    SS_OUTREG32((bank_base0 + 0x20), 0x8009);

    udelay(10);
}

void Issue_dqs_mask_tracking(U8 mask_en)
{
    if (mask_en)
    {
        OUTREGMSK16(GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164800 + (0x54 << 1)), BIT0, BIT0);
        OUTREGMSK16(GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164800 + (0x54 << 1)), BIT1, BIT1);
        OUTREGMSK16(GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164800 + (0x54 << 1)), 0x0700, 0x0F00);

        OUTREGMSK16(GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164800 + (0x44 << 1)), BIT8, BIT8);
        OUTREGMSK16(GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164800 + (0x44 << 1)), BIT10, BIT10);
        OUTREGMSK16(GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164800 + (0x44 << 1)), BIT11, BIT11);
        OUTREGMSK16(GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164800 + (0x44 << 1)), BIT12, BIT12);
    }
    else
    {
        OUTREGMSK16(GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164800 + (0x44 << 1)), 0, BIT12);
    }
}

void get_dq_skew_clkph(U8 miu, U8 cmd_id, U8 idx, U8 *dqs_skew, U8 *clkph)
{
    if (cmd_id < 4)
    {
        *dqs_skew = Read_Reg16(_pRegWriteDQSkew[miu][cmd_id]);
    }

    if (!cmd_id)
    {
        if (idx < 8)
        {
            *clkph = Read_Reg16(_pRegWriteDQS[miu][0][idx]);
        }
        else
        {
            *clkph = Read_Reg16(_pRegWriteDQS[miu][0][16]);
        }
    }
    else if (cmd_id == 1)
    {
        if (idx < 8)
        {
            *clkph = Read_Reg16(_pRegWriteDQS[miu][0][idx + 8]);
        }
        else
        {
            *clkph = Read_Reg16(_pRegWriteDQS[miu][0][17]);
        }
    }
    else if (cmd_id == 2)
    {
        if (idx < 8)
        {
            *clkph = Read_Reg16(_pRegWriteDQS[miu][1][idx]);
        }
        else
        {
            *clkph = Read_Reg16(_pRegWriteDQS[miu][1][16]);
        }
    }
    else
    {
        if (idx < 8)
        {
            *clkph = Read_Reg16(_pRegWriteDQS[miu][1][idx + 8]);
        }
        else
        {
            *clkph = Read_Reg16(_pRegWriteDQS[miu][1][17]);
        }
    }
}

void set_dq_skew_clkph(U8 miu, U8 cmd_id, U8 idx, U8 dq_skew, U8 clkph)
{
    if (cmd_id < 4)
    {
        Write_Reg16(_pRegWriteDQSkew[miu][cmd_id], dq_skew);
    }

    if (!cmd_id)
    {
        if (idx < 8)
        {
            Write_Reg16(_pRegWriteDQS[miu][0][idx], clkph);
        }
        else
        {
            Write_Reg16(_pRegWriteDQS[miu][0][16], clkph);
        }
    }
    else if (cmd_id == 1)
    {
        if (idx < 8)
        {
            Write_Reg16(_pRegWriteDQS[miu][0][idx + 8], clkph);
        }
        else
        {
            Write_Reg16(_pRegWriteDQS[miu][0][17], clkph);
        }
    }
    else if (cmd_id == 2)
    {
        if (idx < 8)
        {
            Write_Reg16(_pRegWriteDQS[miu][1][idx], clkph);
        }
        else
        {
            Write_Reg16(_pRegWriteDQS[miu][1][16], clkph);
        }
    }
    else
    {
        if (idx < 8)
        {
            Write_Reg16(_pRegWriteDQS[miu][1][idx + 8], clkph);
        }
        else
        {
            Write_Reg16(_pRegWriteDQS[miu][1][17], clkph);
        }
    }
}

void get_oendq_skew(U8 miu, U8 cmd_id, U8 *skew)
{
    *skew = Read_Reg16(_pRegWriteDQoenSkew[miu][cmd_id]);
}

void set_oendq_skew(U8 miu, U8 cmd_id, U8 skew)
{
    U8 oen_skew;

    oen_skew = (skew > MAX_OEN_SKEW) ? MAX_OEN_SKEW : skew;
    Write_Reg16(_pRegWriteDQoenSkew[miu][cmd_id], oen_skew);
}

void get_delay_1t(U8 miu, U8 cmd_id, U8 idx, U8 *dalay_1t_en)
{
    if (!cmd_id)
    {
        if (idx < 8)
        {
            *dalay_1t_en = Read_Reg16(_pRegWDQ1t[miu][0][idx]);
        }
        else
        {
            *dalay_1t_en = Read_Reg16(_pRegWDQ1t[miu][0][16]);
        }
    }
    else if (cmd_id == 1)
    {
        if (idx < 8)
        {
            *dalay_1t_en = Read_Reg16(_pRegWDQ1t[miu][0][idx + 8]);
        }
        else
        {
            *dalay_1t_en = Read_Reg16(_pRegWDQ1t[miu][0][17]);
        }
    }
    else if (cmd_id == 2)
    {
        if (idx < 8)
        {
            *dalay_1t_en = Read_Reg16(_pRegWDQ1t[miu][1][idx]);
        }
        else
        {
            *dalay_1t_en = Read_Reg16(_pRegWDQ1t[miu][1][16]);
        }
    }
    else
    {
        if (idx < 8)
        {
            *dalay_1t_en = Read_Reg16(_pRegWDQ1t[miu][1][idx + 8]);
        }
        else
        {
            *dalay_1t_en = Read_Reg16(_pRegWDQ1t[miu][1][17]);
        }
    }
}

void set_delay_1t(U8 miu, U8 cmd_id, U8 idx, U8 dalay_1t_en)
{
    if (!cmd_id)
    {
        if (idx < 8)
        {
            Write_Reg16(_pRegWDQ1t[miu][0][idx], dalay_1t_en);
        }
        else
        {
            Write_Reg16(_pRegWDQ1t[miu][0][16], dalay_1t_en);
        }
    }
    else if (cmd_id == 1)
    {
        if (idx < 8)
        {
            Write_Reg16(_pRegWDQ1t[miu][0][idx + 8], dalay_1t_en);
        }
        else
        {
            Write_Reg16(_pRegWDQ1t[miu][0][17], dalay_1t_en);
        }
    }
    else if (cmd_id == 2)
    {
        if (idx < 8)
        {
            Write_Reg16(_pRegWDQ1t[miu][1][idx], dalay_1t_en);
        }
        else
        {
            Write_Reg16(_pRegWDQ1t[miu][1][16], dalay_1t_en);
        }
    }
    else
    {
        if (idx < 8)
        {
            Write_Reg16(_pRegWDQ1t[miu][1][idx + 8], dalay_1t_en);
        }
        else
        {
            Write_Reg16(_pRegWDQ1t[miu][1][17], dalay_1t_en);
        }
    }
}

void get_dqs_skew_clkph(U8 miu, U8 idx, U8 *dqs_skew, U8 *clkph)
{
    if (idx < WRITE_DQS_PHASE)
    {
        *clkph    = Read_Reg16(_pRegWriteDQsPhase[miu][idx]);
        *dqs_skew = Read_Reg16(_pRegWriteDQsSkew[miu][idx]);
    }
}

U16 get_ddr_freq(void)
{
    U16 tmp, loop_div_reg, loop_div_val;
    U32 ddfset;

    ddfset = INREG16(GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164700 + (0x18 << 1)));
    ddfset |= INREG16(GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164700 + (0x19 << 1))) << 16;

    loop_div_reg = INREG16(GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164700 + (0x1A << 1)));
    loop_div_val = 0;
    tmp          = loop_div_reg & 0x03;
    switch (tmp)
    {
        case 0:
            loop_div_val = 1;
            break;
        case 1:
            loop_div_val = 2;
            break;
        case 2:
            loop_div_val = 4;
            break;
        case 3:
            loop_div_val = 8;
            break;
        default:
            break;
    }
    tmp = (loop_div_reg & 0xFC) >> 2;
    switch (tmp)
    {
        case 0:
        case 1:
            loop_div_val *= 1;
            break;
        case 2:
            loop_div_val *= 2;
            break;
        case 3:
            loop_div_val *= 3;
            break;
        default:
            break;
    }

    return (U16)(((U64)(432 * loop_div_val) << 21) / ddfset);
}

U32 ddr_convert_ddfset(U16 ddr_freq)
{
    U16 tmp, loop_div_reg, loop_div_val;

    loop_div_reg = INREG16(GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164700 + (0x1A << 1)));
    loop_div_val = 0;
    tmp          = loop_div_reg & 0x03;
    switch (tmp)
    {
        case 0:
            loop_div_val = 1;
            break;
        case 1:
            loop_div_val = 2;
            break;
        case 2:
            loop_div_val = 4;
            break;
        case 3:
            loop_div_val = 8;
            break;
        default:
            break;
    }
    tmp = (loop_div_reg & 0xFC) >> 2;
    switch (tmp)
    {
        case 0:
        case 1:
            loop_div_val *= 1;
            break;
        case 2:
            loop_div_val *= 2;
            break;
        case 3:
            loop_div_val *= 3;
            break;
        default:
            break;
    }

    return (U32)(((U64)(432 * loop_div_val) << 21) / ddr_freq);
}

void ddr_update_ddfset(U32 tar_ddfset)
{
    U32 ddfset;

    ddfset = INREG16(GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164700 + (0x18 << 1)));
    ddfset |= INREG16(GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164700 + (0x19 << 1))) << 16;

    if (tar_ddfset > ddfset)
    {
        OUTREG16(GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164700 + (0x19 << 1)), (tar_ddfset >> 16));
        OUTREG16(GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164700 + (0x18 << 1)), tar_ddfset);
    }
    else
    {
        OUTREG16(GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164700 + (0x18 << 1)), tar_ddfset);
        OUTREG16(GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164700 + (0x19 << 1)), (tar_ddfset >> 16));
    }
}

U16 get_ddr_DQS_treedly(U16 u16_UI)
{
    U16 cnt;
    U8  miu = 0;

    // MRW23
    Issue_MRW23(0);
    // start DQS_Osc
    Set_DQS_Osc_start(0);
    udelay(5);

    // MRR18
    cnt = Issue_MRR(miu, 0x12);
    printk("mr18: 0x%X\r\n", cnt);

    // MRR19
    cnt |= (Issue_MRR(miu, 0x13) << 8);
    printk("mr19|mr18: 0x%X\r\n", cnt);

    return (u16_UI * 2048 * 2) / (2 * cnt);
}

void Hal_save_dq_dm_phase(void)
{
    int  i;
    char miu = 0;

    // printk("[%s]\r\n", __FUNCTION__);

    for (i = 0; i < 36; i++)
    {
        if (i < 32)
        {
            get_dq_skew_clkph(miu, (i / 8), (i & 0x07), &def_dq_skew[i / 8], &def_dq_clkph[i]);
            get_delay_1t(miu, (i / 8), (i & 0x07), &def_delay_1t[i]);
        }
        else
        {
            get_dq_skew_clkph(miu, i % 4, 8, &def_dq_skew[i % 4], &def_dq_clkph[i]);
            get_delay_1t(miu, i % 4, 8, &def_delay_1t[i]);
        }
    }

    for (i = 0; i < 4; i++)
    {
        get_oendq_skew(miu, i, &def_oendq_skew[i]);
    }

#if 0
    DumpBankRegs_part(0x1648, 0x00, 0x20);
    DumpBankRegs_part(0x1649, 0x00, 0x10);
#endif
}

void Hal_restore_dq_dm_phase(void)
{
    int  i;
    char miu = 0;

    for (i = 0; i < 36; i++)
    {
        if (i < 32)
        {
            set_dq_skew_clkph(miu, (i / 8), (i & 0x07), def_dq_skew[i / 8], def_dq_clkph[i]);
            set_delay_1t(miu, (i / 8), (i & 0x07), def_delay_1t[i]);
        }
        else
        {
            set_dq_skew_clkph(miu, i % 4, 8, def_dq_skew[i % 4], def_dq_clkph[i]);
            set_delay_1t(miu, i % 4, 8, def_delay_1t[i]);
        }
    }

    for (i = 0; i < 4; i++)
    {
        set_oendq_skew(miu, i, def_oendq_skew[i]);
    }
}

void move_dq_dm_step(U8 miu, U16 k_code, S16 *s16step, U8 flag, U8 ph_mode)
{
    u8  u8skew[4], u8clkph, tmp_clkph, u8oenskew[4];
    u16 i, j, k;
    s8  max_right[4], max_left[4], s8tmp;
    s8  clkph_idx[36];
    u8  delay_1t[36];
    s8  dvs_step = 0;

    for (i = 0; i < 36; i++)
    {
        if (i < 32)
        {
#if defined(MIU_BUS_16BIT)
            if (i >= 16)
                continue;
#endif
            get_dq_skew_clkph(miu, (i / 8), (i & 0x07), &u8skew[i / 8], &u8clkph);
            get_delay_1t(miu, (i / 8), (i & 0x07), &delay_1t[i]);
        }
        else
        {
#if defined(MIU_BUS_16BIT)
            if (i >= 34)
                continue;
#endif
            get_dq_skew_clkph(miu, i % 4, 8, &u8skew[i % 4], &u8clkph);
            get_delay_1t(miu, i % 4, 8, &delay_1t[i]);
        }

        if (flag == MOVE_ALL)
            clkph_idx[i] = GET_CLKPH_ID(k_code, u8clkph) + delay_1t[i] * k_code + s16step[0];
        else if (flag == MOVE_BY_BYTE)
        {
            if (i < 32)
                clkph_idx[i] = GET_CLKPH_ID(k_code, u8clkph) + delay_1t[i] * k_code + s16step[i / 8];
            else
                clkph_idx[i] = GET_CLKPH_ID(k_code, u8clkph) + delay_1t[i] * k_code + s16step[i % 4];
        }
        else
            clkph_idx[i] = GET_CLKPH_ID(k_code, u8clkph) + delay_1t[i] * k_code + s16step[i];

        if (dvs_definit_update)
        {
            dvs_step = do_phase_scaling(k_code, dvs_def_refkcode, _pRegWriteDQS[miu][0][0], u8clkph, 0);
#if DBG_PHASE_EN
            printk(KERN_DEBUG "dvs_step: %X - %X\r\n", dvs_step, u8clkph);
#endif
            dvs_step -= u8clkph;
            clkph_idx[i] += dvs_step;
#if DBG_PHASE_EN
            if (dvs_step >= 0)
                printk(KERN_DEBUG "dvs_step: 0x%X\r\n", dvs_step);
            else
                printk(KERN_DEBUG "dvs_step: -0x%X\r\n", (~dvs_step) + 1);
#endif
        }
#if defined(DBG_DQS_OSC)
        ERR_MSG("id 0x");
        ERR_VAR8(i);
        if (s16step >= 0)
        {
            ERR_MSG(", move ");
            ERR_VAR8(*s16step);
        }
        else
        {
            ERR_MSG(", move -");
            ERR_VAR8((~*s16step) + 1);
        }
        ERR_MSG(", clkph id 0x");
        ERR_VAR8(clkph_idx[i]);
        ERR_MSG("\r\n");
#endif
    }

    for (i = 0; i < 4; i++)
    {
#if defined(MIU_BUS_16BIT)
        if (i >= 2)
            continue;
#endif
        max_right[i] = 0;
        max_left[i]  = 0;
        for (j = i * 8; j < (i + 1) * 8; j++)
        {
            s8tmp = clkph_idx[j] - clkph_idx[i * 8];
            if (max_right[i] < s8tmp)
                max_right[i] = s8tmp;
            else if (max_left[i] > s8tmp)
                max_left[i] = s8tmp;
        }
        s8tmp = clkph_idx[32 + i] - clkph_idx[i * 8];
        if (max_right[i] < s8tmp)
            max_right[i] = s8tmp;
        else if (max_left[i] > s8tmp)
            max_left[i] = s8tmp;
#if defined(DBG_DQS_OSC)
        ERR_MSG("grp");
        ERR_VAR8(i);
        ERR_MSG(", idx ");
        ERR_VAR8(max_left[i] + clkph_idx[i * 8]);
        ERR_MSG(" to ");
        ERR_VAR8(max_right[i] + clkph_idx[i * 8]);
        ERR_MSG("\r\n");
#endif
    }

    if (WORD_GROUP_MODE)
    {
        for (i = 0; i < 4; i += 2)
        {
#if defined(MIU_BUS_16BIT)
            if (i >= 2)
                continue;
#endif
            if (max_left[i] < max_left[i + 1])
            {
                max_left[i + 1] = max_left[i];
            }
            else
            {
                max_left[i] = max_left[i + 1];
            }

            if (max_right[i] < max_right[i + 1])
            {
                max_right[i] = max_right[i + 1];
            }
            else
            {
                max_right[i + 1] = max_right[i];
            }
        }
    }

    for (i = 0; i < 4; i++)
    {
#if defined(MIU_BUS_16BIT)
        if (i >= 2)
            continue;
#endif
        get_oendq_skew(miu, i, &u8oenskew[i]);
        if (max_right[i] - max_left[i] > k_code - 1)
        {
            printk("group %d left and right across more than one skew\r\n", i);
            // SS_OUTREG16(REG_ERR_MSG, DDRERR_WRDQ_OUTRANGE | miu);
            return;
        }

        s8tmp = clkph_idx[i * 8] + max_left[i];
        if (s8tmp < 0)
        {
            for (j = 0; j < 9; j++)
            {
                k = (j < 8) ? (i * 8 + j) : (32 + i);
                if (clkph_idx[k] >= 0)
                {
                    if (ph_mode == MODE_BY_KCODE)
                        tmp_clkph = GET_CLKPH(k_code, clkph_idx[k]);
                    else
                        tmp_clkph = GET_CLKPH_128(k_code, clkph_idx[k]);
                    set_dq_skew_clkph(miu, i, j, u8skew[i] - 1, tmp_clkph);
                    set_delay_1t(miu, i, j, 1);
                    set_oendq_skew(miu, i, u8oenskew[i] - 1);
                }
                else
                {
                    tmp_clkph = k_code + clkph_idx[k];
                    if (ph_mode == MODE_BY_KCODE)
                        tmp_clkph = GET_CLKPH(k_code, tmp_clkph);
                    else
                        tmp_clkph = GET_CLKPH_128(k_code, tmp_clkph);
                    set_dq_skew_clkph(miu, i, j, u8skew[i] - 1, tmp_clkph);
                    set_delay_1t(miu, i, j, 0);
                    set_oendq_skew(miu, i, u8oenskew[i] - 1);
                }
            }
        }
        else if (s8tmp >= k_code)
        {
            for (j = 0; j < 9; j++)
            {
                k = (j < 8) ? (i * 8 + j) : (32 + i);
                if (clkph_idx[k] >= 2 * k_code)
                {
                    tmp_clkph = clkph_idx[k] - 2 * k_code;
                    if (ph_mode == MODE_BY_KCODE)
                        tmp_clkph = GET_CLKPH(k_code, tmp_clkph);
                    else
                        tmp_clkph = GET_CLKPH_128(k_code, tmp_clkph);
                    set_dq_skew_clkph(miu, i, j, u8skew[i] + 1, tmp_clkph);
                    set_delay_1t(miu, i, j, 1);
                    set_oendq_skew(miu, i, u8oenskew[i] + 1);
                }
                else
                {
                    tmp_clkph = clkph_idx[k] - k_code;
                    if (ph_mode == MODE_BY_KCODE)
                        tmp_clkph = GET_CLKPH(k_code, tmp_clkph);
                    else
                        tmp_clkph = GET_CLKPH_128(k_code, tmp_clkph);
                    set_dq_skew_clkph(miu, i, j, u8skew[i] + 1, tmp_clkph);
                    set_delay_1t(miu, i, j, 0);
                    set_oendq_skew(miu, i, u8oenskew[i] + 1);
                }
            }
        }
        else
        {
            for (j = 0; j < 9; j++)
            {
                k = (j < 8) ? (i * 8 + j) : (32 + i);
                if (clkph_idx[k] >= k_code)
                {
                    tmp_clkph = clkph_idx[k] - k_code;
                    if (ph_mode == MODE_BY_KCODE)
                        tmp_clkph = GET_CLKPH(k_code, tmp_clkph);
                    else
                        tmp_clkph = GET_CLKPH_128(k_code, tmp_clkph);
                    set_dq_skew_clkph(miu, i, j, u8skew[i], tmp_clkph);
                    set_delay_1t(miu, i, j, 1);
                }
                else
                {
                    if (ph_mode == MODE_BY_KCODE)
                        tmp_clkph = GET_CLKPH(k_code, clkph_idx[k]);
                    else
                        tmp_clkph = GET_CLKPH_128(k_code, clkph_idx[k]);
                    set_dq_skew_clkph(miu, i, j, u8skew[i], tmp_clkph);
                    set_delay_1t(miu, i, j, 0);
                }
            }
        }

#if defined(DBG_DQS_OSC)
        for (j = 0; j < 9; j++)
        {
            get_dq_skew_clkph(miu, i, j, &u8skew[i], &tmp_clkph);
            get_delay_1t(miu, i, j, &delay_1t[0]);
            if (j == 8)
            {
                ERR_MSG("DM id 0x");
                ERR_VAR8(i);
            }
            else
            {
                ERR_MSG("DQ id 0x");
                ERR_VAR8(i * 8 + j);
            }
            ERR_MSG(", skew/clkph 0x");
            ERR_VAR8(s8tmp);
            ERR_MSG(", 0x");
            ERR_VAR8(tmp_clkph);
            ERR_MSG("\r\n");
        }
#endif
    }

    return;
}

void dvs_phase_defsave(u8 miu)
{
    u16 i, j;

    dvs_def_refkcode = Read_Reg16(_pRegDllTxREFCode[miu]);
    // ReadDqsmPhase scaling
    for (i = 0; i < 4; i++)
    {
        dvs_def_ReadDqsmPhase[i] = Read_Reg16(_pRegReadDqsmPhase[miu][i]);
    }

    // WriteDQsPhase scaling
    for (i = 0; i < 4; i++)
    {
        dvs_def_WriteDqsmPhase[i] = Read_Reg16(_pRegWriteDQsPhase[miu][i]);
    }

    // WriteDQS scaling
    for (i = 0; i < 2; i++)
    {
        for (j = 0; j < 18; j++)
        {
            dvs_def_WriteDQS[i][j] = Read_Reg16(_pRegWriteDQS[miu][i][j]);
        }
    }

    // WriteClkPhase, WriteCsPhase scaling
    dvs_def_WriteClkPhase  = Read_Reg16(_pRegWriteClkPhase[miu]);
    dvs_def_WriteClk1Phase = Read_Reg16(_pRegWriteClkPhase[miu]);
    dvs_def_WriteCsPhase   = Read_Reg16(_pRegWriteCsPhase[miu]);
    dvs_def_WriteCs1Phase  = Read_Reg16(_pRegWriteCs1Phase[miu]);

    // WriteCmdPhase scaling
    for (i = 0; i < 28; i++)
    {
        dvs_def_WriteCmdPhase[i] = Read_Reg16(_pRegWriteCmdPhase[miu][i]);
    }

    dvs_definit_update = 1;
}

u16 do_phase_scaling(u16 k_code, u16 ref_kcode, RegBase reg_base, u16 defval, u8 set_reg)
{
    u16 clkph, clkph_tmp;

    clkph     = defval;
    clkph_tmp = clkph & 0x7F;
    clkph_tmp = (clkph_tmp * k_code) / ref_kcode;
    if (clkph >= 0x80)
        clkph_tmp = clkph_tmp | 0x80;
    else if (clkph_tmp > 75)
        clkph_tmp = 75;
    if (set_reg)
        Write_Reg16(reg_base, clkph_tmp);

    return clkph_tmp;
}

void dvs_phase_scaling(u8 miu)
{
    u16 k_code, ref_kcode;
    u16 i, j;

    k_code    = cal_average_kcode(miu);
    ref_kcode = dvs_def_refkcode;
    printk(KERN_DEBUG "%s %d: kcode=0x%x, ref_kcode=0x%X\r\n", __FUNCTION__, __LINE__, k_code, ref_kcode);
    if ((k_code == 0) || (ref_kcode == 0) || (k_code == ref_kcode))
        return;

    Write_Reg16(_pRegDynUpd_EN[miu], 1);
    Write_Reg16(_pRegDynUpd_halt[miu], 1);

    // ReadDqsmPhase scaling
    for (i = 0; i < 4; i++)
    {
        do_phase_scaling(k_code, ref_kcode, _pRegReadDqsmPhase[miu][i], dvs_def_ReadDqsmPhase[i], 1);
    }

    // WriteDQsPhase scaling
    for (i = 0; i < 4; i++)
    {
        do_phase_scaling(k_code, ref_kcode, _pRegWriteDQsPhase[miu][i], dvs_def_WriteDqsmPhase[i], 1);
    }

    // WriteDQS scaling
    if (!Hal_Check_LPDDR4())
    {
        // DDR3 and DDR4
        for (i = 0; i < 2; i++)
        {
            for (j = 0; j < 18; j++)
            {
                do_phase_scaling(k_code, ref_kcode, _pRegWriteDQS[miu][i][j], dvs_def_WriteDQS[i][j], 1);
            }
        }
    }
    else
    {
#if DBG_PHASE_EN
        if (g_deltaD >= 0)
            printk(KERN_DEBUG "g_deltaD: 0x%X\r\n", g_deltaD);
        else
            printk(KERN_DEBUG "g_deltaD: -0x%X\r\n", (~g_deltaD) + 1);
#endif
        Hal_restore_dq_dm_phase();
        move_dq_dm_step(miu, k_code, (s16 *)&g_deltaD, MOVE_ALL, MODE_BY_KCODE);
    }

    // WriteClkPhase, WriteCsPhase scaling
    do_phase_scaling(k_code, ref_kcode, _pRegWriteClkPhase[miu], dvs_def_WriteClkPhase, 1);
    do_phase_scaling(k_code, ref_kcode, _pRegWriteClk1Phase[miu], dvs_def_WriteClk1Phase, 1);
    do_phase_scaling(k_code, ref_kcode, _pRegWriteCsPhase[miu], dvs_def_WriteCsPhase, 1);
    do_phase_scaling(k_code, ref_kcode, _pRegWriteCs1Phase[miu], dvs_def_WriteCs1Phase, 1);

    // WriteCmdPhase scaling
    for (i = 0; i < 28; i++)
    {
        do_phase_scaling(k_code, ref_kcode, _pRegWriteCmdPhase[miu][i], dvs_def_WriteCmdPhase[i], 1);
    }

    Write_Reg16(_pRegDynUpd_halt[miu], 0);

    Write_Reg16(_pRegDllTxREFCode[miu], k_code);
    printk(KERN_DEBUG "%s %d: write kcode=0x%x to ref_kcode\r\n", __FUNCTION__, __LINE__, k_code);
}

u16 get_txkcode_value(u8 miu)
{
    u16 k_code;

    Write_Reg16(_pRegDllTxPD[miu], 1);
    usleep_range(1, 2);
    k_code = Read_Reg16(_pRegDllTxKCode[miu]);
    Write_Reg16(_pRegDllTxPD[miu], 0);

    return k_code;
}

u16 cal_average_kcode(u8 miu)
{
    u16 u16min = 0, u16max = 0;
    u16 k_code;
    u8  j, u8loops = 10;

    k_code = get_txkcode_value(miu);
    u16min = k_code;
    u16max = k_code;
    for (j = 0; j < u8loops; j++)
    {
        usleep_range(1, 2);
        k_code = get_txkcode_value(miu);
        if (u16min > k_code)
            u16min = k_code;
        if (u16max < k_code)
            u16max = k_code;
    }
    k_code = (u16min + u16max) >> 1;
    printk(KERN_DEBUG "read kcode min max: 0x%X 0x%X\r\n", u16min, u16max);

    return k_code;
}

void Hal_DQ_Init(void)
{
    LoadRegMiu();
    if (Hal_Check_LPDDR4())
    {
        Hal_save_dq_dm_phase();
    }

    if (!dvs_definit_update)
    {
        dvs_phase_defsave(0);
    }

    g_deltaD = 0;
#if 0
    def_ddrfreq      = get_ddr_freq();
    def_tx_kcode     = INREG16(GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164800 + (0x61 << 1)));
    def_UI_X10       = (1000000 * 10) / def_ddrfreq;
    def_dlycell_X10  = def_UI_X10 / def_tx_kcode;
    def_dqs_tree_X10 = get_ddr_DQS_treedly(def_UI_X10);

    def_ddfset = ddr_convert_ddfset(def_ddrfreq);
#endif
}

void Hal_DQ_Exit(void)
{
    Hal_restore_dq_dm_phase();
    g_deltaD = 0;
}

// u16 dbg_num = 0;
// return 0:pass, 1:fail
U8 Hal_Set_DQS_Osc(U8 miu)
{
    U16 cnt;
    S16 deltaD, sign;
    U16 ref_MROsc, k_code;
    S32 tmp;
    U32 tmp1;
#if 0
    U16     wait_period, u16ckeon;
#else
    U16 u16ckeon;
#endif
    RegBase _pRegCkeOn;

#if 0 // for dbg

    if (!dbg_num)
    {
        Write_Reg16(_pRegRef_MROsc[miu], 0x4C0);
        dbg_num = 1;
    }
    else
    {
        Write_Reg16(_pRegRef_MROsc[miu], 0x490);
        dbg_num = 0;
    }

#endif

    if (!mr23_en)
    {
        // MRW23
        Issue_MRW23(0);
        mr23_en = 1;
    }

    _pRegCkeOn.bank_addr = GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x101200 + 0x06);
    _pRegCkeOn.bank_mask = 0x1000;
    u16ckeon             = Read_Reg16(_pRegCkeOn);
    Write_Reg16(_pRegCkeOn, 1);

    printk(KERN_DEBUG "==== do dqs osc flow\r\n");
    // k_code    = Read_Reg16(_pRegDllTxREFCode[miu]);
    k_code = cal_average_kcode(miu);
    printk(KERN_DEBUG "read kcode: 0x%X\r\n", k_code);
    ref_MROsc = Read_Reg16(_pRegRef_MROsc[miu]);
    printk(KERN_DEBUG "ref_MROsc: 0x%X\r\n", ref_MROsc);
    if (ref_MROsc != 0 && ref_MROsc != 0xC0C0)
    {
        // printk("start DQS osc\r\n");
        //  start DQS_Osc
        Set_DQS_Osc_start(miu);
        udelay(5);

        // MRR18
        cnt = Issue_MRR(miu, 0x12);
        // printk("mr18: 0x%X\r\n", cnt);

        // MRR19
        cnt |= (Issue_MRR(miu, 0x13) << 8);
        printk(KERN_DEBUG "mr19|mr18: 0x%X\r\n", cnt);

        ref_MROsc = Read_Reg16(_pRegRef_MROsc[miu]);
        // printk("MROsc: 0x%X, k_code: 0x%X\r\n", ref_MROsc, k_code);
        deltaD = 0;
        if (0 != cnt && 0 != ref_MROsc)
        {
            // deltaD = 2048*k_code*(ref_MROsc - cnt)/(cnt*ref_MROsc);
            tmp  = 2048 * k_code * (ref_MROsc - cnt);
            tmp1 = cnt * ref_MROsc;
            sign = ref_MROsc > cnt ? 1 : -1;
            while (tmp * sign >= tmp1 * deltaD)
            {
                deltaD++;
            }
            deltaD--;
            deltaD *= sign;
        }
        else
        {
            printk("mr19|mr18=0\r\n");
            goto DQS_OSC_EXIT;
        }
#if 1
        if (deltaD >= 0)
            printk(KERN_DEBUG "deltaD: 0x%X\r\n", deltaD);
        else
            printk(KERN_DEBUG "deltaD: -0x%X\r\n", (~deltaD) + 1);
#endif
        Write_Reg16(_pRegDynUpd_EN[miu], 1);
        Write_Reg16(_pRegDynUpd_halt[miu], 1);
        udelay(1);

#if 0
        if (deltaD != g_deltaD)
        {
            Hal_restore_dq_dm_phase();
            if (deltaD != 0)
                move_dq_dm_step(miu, k_code, &deltaD, MOVE_ALL, MODE_BY_KCODE);
            // DumpBankRegs_part(0x1648, 0x00, 0x20);
            // DumpBankRegs_part(0x1649, 0x00, 0x10);
            // Issue_EN_SCL_1();
            g_deltaD = deltaD;
            udelay(1);
        }
#else
        // if (deltaD != g_deltaD)
        {
            Hal_restore_dq_dm_phase();
            // if (deltaD != 0)
            move_dq_dm_step(miu, k_code, &deltaD, MOVE_ALL, MODE_BY_KCODE);
            // DumpBankRegs_part(0x1648, 0x00, 0x20);
            // DumpBankRegs_part(0x1649, 0x00, 0x10);
            // Issue_EN_SCL_1();
            g_deltaD = deltaD;
            udelay(1);
        }
#endif
        Write_Reg16(_pRegDynUpd_halt[miu], 0);

        // MRR4
        if (mr4_en)
        {
            cnt = Issue_MRR(miu, 0x04);
            printk(KERN_DEBUG "mr4: 0x%X\r\n", cnt);
        }

        Write_Reg16(_pRegCkeOn, u16ckeon);
        // printk("Wait %d sec\r\n", wait_time_period);
#if 0
        wait_period = 0;
        while (wait_period < (wait_time_period * 100))
        {
            // ssleep(1);
            msleep(10);
            wait_period++;

            if (kthread_should_stop())
                break;
#if 0
            if ((wait_period % 100) == 0)
                printk("*       %d\r\n", wait_period);
#endif
        }
#endif
    }

    printk(KERN_DEBUG "==== end dqs osc flow\r\n");
DQS_OSC_EXIT:

    return 0;
}

u8 Hal_Set_DvsPhaseScaling(u8 miu)
{
    // if (!Hal_Check_LPDDR4())
    //{
    //  only support DDR3/DDR4
    dvs_phase_scaling(miu);
    //}

    return 0;
}

U8 Hal_Set_Freq_DQPh(U8 miu, U16 tar_freq)
{
#if 0
    U16 curr_ddrfreq, tx_kcode, new_kcode, UI_X10;
    U8  i, multi_10 = 10;
    U8  dqs_skew, dqs_clkph, wr_dq_skew, wr_dq_clkph;
    U8  tmp_clkph;
    U16 ideal_dqs_clkph[4];
    S16 diff_dqs_clkph[4];
    U32 curr_ddfset, tar_ddfset;
    int tmp_val, move_ddfset;

    curr_ddrfreq = get_ddr_freq();
    // tx_kcode = INREG16(GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164800+(0x61 << 1)));

    curr_ddfset = ddr_convert_ddfset(curr_ddrfreq);
    tar_ddfset  = ddr_convert_ddfset(tar_freq);

    if (tar_freq == curr_ddrfreq)
        return 0;

#if 1
    printk("ddrfeq: %d\r\n", curr_ddrfreq);
    printk("curr_ddfset: %X\r\n", curr_ddfset);
    printk("tar_ddfset: %X\r\n", tar_ddfset);
#endif

    if (curr_ddrfreq == def_ddrfreq)
    {
        def_tx_kcode     = INREG16(GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164800 + (0x61 << 1)));
        def_dlycell_X10  = def_UI_X10 / def_tx_kcode;
        def_dqs_tree_X10 = get_ddr_DQS_treedly(def_UI_X10);

        for (i = 0; i < 4; i++)
        {
            get_dqs_skew_clkph(0, 0, &dqs_skew, &dqs_clkph);
            // get_dq_skew_clkph(0, 0, 0, &wr_dq_skew, &wr_dq_clkph);
            wr_dq_skew = def_dq_skew[i];
            tmp_clkph  = GET_CLKPH_ID(def_tx_kcode, dqs_clkph);
            tmp_val    = (dqs_skew * def_UI_X10 + tmp_clkph * def_dlycell_X10 + def_dqs_tree_X10);
            tmp_val -= (wr_dq_skew * def_UI_X10 + def_UI_X10 / 2);

            printk("%d %d", i, tmp_clkph);
            printk("%d, %d %d %d\r\n", i, dqs_skew * def_UI_X10, tmp_clkph * def_dlycell_X10, def_dqs_tree_X10);
            printk("%d, %d %d\r\n", i, wr_dq_skew * def_UI_X10, def_UI_X10 / 2);
            printk("%d, tmp_val: %d\r\n", i, tmp_val);
            tmp_val = tmp_val / def_dlycell_X10;
            printk("%d, ideal wr dqsph: %d\r\n", i, tmp_val);
            ideal_dqs_clkph[i] = tmp_val;
        }
    }
    else
    {
        // adjust write dq phase
        UI_X10   = (1000000 * multi_10) / get_ddr_freq();
        tx_kcode = UI_X10 / def_dlycell_X10;
        // printk("new UI_X10: %d, kcode: %d", UI_X10, new_kcode);

        for (i = 0; i < 4; i++)
        {
            get_dqs_skew_clkph(0, 0, &dqs_skew, &dqs_clkph);
            get_dq_skew_clkph(0, 0, 0, &wr_dq_skew, &wr_dq_clkph);
            tmp_clkph = GET_CLKPH_ID(tx_kcode, dqs_clkph);
            tmp_val   = (dqs_skew * UI_X10 + tmp_clkph * def_dlycell_X10 + def_dqs_tree_X10);
            tmp_val -= (wr_dq_skew * UI_X10 + UI_X10 / 2);

            tmp_val = tmp_val / def_dlycell_X10;
            printk("%d, ideal wr dqsph: %d\r\n", i, tmp_val);
            ideal_dqs_clkph[i] = tmp_val;
        }
    }

    Issue_dqs_mask_tracking(1);

    if (curr_ddfset < tar_ddfset)
        move_ddfset = 0x8000;
    else
        move_ddfset = -0x8000;

    printk("curr_freq_reg: %X, tar_freq_reg: %X, move_ddfset: %X", curr_ddfset, tar_ddfset, move_ddfset);
    while (1)
    {
        if (move_ddfset > 0)
        {
            if ((curr_ddfset + move_ddfset) <= tar_ddfset)
                curr_ddfset += move_ddfset;
            else
            {
                curr_ddfset = tar_ddfset;
            }
        }
        else
        {
            if ((curr_ddfset + move_ddfset) >= tar_ddfset)
                curr_ddfset += move_ddfset;
            else
            {
                curr_ddfset = tar_ddfset;
            }
        }
        ddr_update_ddfset(curr_ddfset);
        printk("set curr_freq_reg: %X, freq=%d", curr_ddfset, get_ddr_freq());
        ssleep(1);
#if 1
        // adjust write dq phase
        UI_X10    = (1000000 * multi_10) / get_ddr_freq();
        new_kcode = UI_X10 / def_dlycell_X10;
        printk("new UI_X10: %d, kcode: %d", UI_X10, new_kcode);

        for (i = 0; i < 4; i++)
        {
            get_dqs_skew_clkph(0, 0, &dqs_skew, &dqs_clkph);
            get_dq_skew_clkph(0, 0, 0, &wr_dq_skew, &wr_dq_clkph);
            tmp_clkph = GET_CLKPH_ID(new_kcode, dqs_clkph);
            tmp_val   = (dqs_skew * UI_X10 + tmp_clkph * def_dlycell_X10 + def_dqs_tree_X10);
            tmp_val -= (wr_dq_skew * UI_X10 + UI_X10 / 2);

            // printk("%d %d", i, tmp_clkph);
            // printk("%d, %d %d %d\r\n", i, dqs_skew*UI_X10, tmp_clkph*dlycell_X10, dqs_tree_X10);
            // printk("%d, %d %d\r\n", i, wr_dq_skew*UI_X10, UI_X10/2);
            // printk("%d, tmp_val: %d\r\n", i, tmp_val);
            tmp_val = tmp_val / def_dlycell_X10;
            printk("%d, ideal wr dqsph: %d\r\n", i, tmp_val);
            diff_dqs_clkph[i]  = tmp_val - ideal_dqs_clkph[i];
            ideal_dqs_clkph[i] = tmp_val;
        }
        // DumpBankRegs_part(0x1648, 0x00, 0x20);
        // DumpBankRegs_part(0x1649, 0x00, 0x10);
        //  for dbg
        move_dq_dm_step(miu, new_kcode, &diff_dqs_clkph[0], MOVE_BY_BYTE, MODE_BY_128);
        // DumpBankRegs_part(0x1648, 0x00, 0x20);
        // DumpBankRegs_part(0x1649, 0x00, 0x10);

        printk("*************************\r\n");
#endif

        if (curr_ddfset == tar_ddfset)
            break;
    }

    Issue_dqs_mask_tracking(0);

    if (get_ddr_freq() == def_ddrfreq)
        Hal_restore_dq_dm_phase();
#endif
    printk("[%s] Not support this function!!\r\n", __FUNCTION__);

    return 0;
}
