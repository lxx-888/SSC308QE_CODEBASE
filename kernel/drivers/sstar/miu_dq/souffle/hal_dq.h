/*
 * hal_dq.h- Sigmastar
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

#ifndef __HAL_DQ__
#define __HAL_DQ__

#define Result_MRR_0_Reg   GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x161300 + 0x24)
#define Result_MRR_0_Mask  0x00FF
#define DynUpd_EN_0_Reg    GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164800 + 0xa8)
#define DynUpd_EN_0_Mask   0x0001
#define DynUpd_halt_0_Reg  GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164800 + 0xa8)
#define DynUpd_halt_0_Mask 0x0002
#define Ref_MROsc_0_Reg    GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164800 + 0x28)
#define Ref_MROsc_0_Mask   0xFFFF

#define WriteDQSkew_0_0_Reg  GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164800 + 0x2e)
#define WriteDQSkew_0_0_Mask 0x0007
#define WriteDQSkew_0_1_Reg  GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164800 + 0x2e)
#define WriteDQSkew_0_1_Mask 0x0070
#define WriteDQSkew_0_2_Reg  GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164800 + 0x2e)
#define WriteDQSkew_0_2_Mask 0x0700
#define WriteDQSkew_0_3_Reg  GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164800 + 0x2e)
#define WriteDQSkew_0_3_Mask 0x7000

#define WriteDQoenSkew_0_0_Reg  GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164800 + 0x30)
#define WriteDQoenSkew_0_0_Mask 0x0007
#define WriteDQoenSkew_0_1_Reg  GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164800 + 0x30)
#define WriteDQoenSkew_0_1_Mask 0x0070
#define WriteDQoenSkew_0_2_Reg  GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164800 + 0x30)
#define WriteDQoenSkew_0_2_Mask 0x0700
#define WriteDQoenSkew_0_3_Reg  GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164800 + 0x30)
#define WriteDQoenSkew_0_3_Mask 0x7000

#define WriteDQS_0_0_0_Reg   GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164800 + 0x0)
#define WriteDQS_0_0_0_Mask  0x00FF
#define WriteDQS_0_0_1_Reg   GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164800 + 0x0)
#define WriteDQS_0_0_1_Mask  0xFF00
#define WriteDQS_0_0_2_Reg   GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164800 + 0x2)
#define WriteDQS_0_0_2_Mask  0x00FF
#define WriteDQS_0_0_3_Reg   GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164800 + 0x2)
#define WriteDQS_0_0_3_Mask  0xFF00
#define WriteDQS_0_0_4_Reg   GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164800 + 0x4)
#define WriteDQS_0_0_4_Mask  0x00FF
#define WriteDQS_0_0_5_Reg   GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164800 + 0x4)
#define WriteDQS_0_0_5_Mask  0xFF00
#define WriteDQS_0_0_6_Reg   GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164800 + 0x6)
#define WriteDQS_0_0_6_Mask  0x00FF
#define WriteDQS_0_0_7_Reg   GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164800 + 0x6)
#define WriteDQS_0_0_7_Mask  0xFF00
#define WriteDQS_0_0_8_Reg   GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164800 + 0x8)
#define WriteDQS_0_0_8_Mask  0x00FF
#define WriteDQS_0_0_9_Reg   GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164800 + 0x8)
#define WriteDQS_0_0_9_Mask  0xFF00
#define WriteDQS_0_0_10_Reg  GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164800 + 0xa)
#define WriteDQS_0_0_10_Mask 0x00FF
#define WriteDQS_0_0_11_Reg  GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164800 + 0xa)
#define WriteDQS_0_0_11_Mask 0xFF00
#define WriteDQS_0_0_12_Reg  GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164800 + 0xc)
#define WriteDQS_0_0_12_Mask 0x00FF
#define WriteDQS_0_0_13_Reg  GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164800 + 0xc)
#define WriteDQS_0_0_13_Mask 0xFF00
#define WriteDQS_0_0_14_Reg  GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164800 + 0xe)
#define WriteDQS_0_0_14_Mask 0x00FF
#define WriteDQS_0_0_15_Reg  GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164800 + 0xe)
#define WriteDQS_0_0_15_Mask 0xFF00
#define WriteDQS_0_0_16_Reg  GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164800 + 0x10)
#define WriteDQS_0_0_16_Mask 0x00FF
#define WriteDQS_0_0_17_Reg  GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164800 + 0x10)
#define WriteDQS_0_0_17_Mask 0xFF00
#define WriteDQS_0_1_0_Reg   GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164800 + 0x12)
#define WriteDQS_0_1_0_Mask  0x00FF
#define WriteDQS_0_1_1_Reg   GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164800 + 0x12)
#define WriteDQS_0_1_1_Mask  0xFF00
#define WriteDQS_0_1_2_Reg   GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164800 + 0x14)
#define WriteDQS_0_1_2_Mask  0x00FF
#define WriteDQS_0_1_3_Reg   GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164800 + 0x14)
#define WriteDQS_0_1_3_Mask  0xFF00
#define WriteDQS_0_1_4_Reg   GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164800 + 0x16)
#define WriteDQS_0_1_4_Mask  0x00FF
#define WriteDQS_0_1_5_Reg   GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164800 + 0x16)
#define WriteDQS_0_1_5_Mask  0xFF00
#define WriteDQS_0_1_6_Reg   GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164800 + 0x18)
#define WriteDQS_0_1_6_Mask  0x00FF
#define WriteDQS_0_1_7_Reg   GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164800 + 0x18)
#define WriteDQS_0_1_7_Mask  0xFF00
#define WriteDQS_0_1_8_Reg   GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164800 + 0x1a)
#define WriteDQS_0_1_8_Mask  0x00FF
#define WriteDQS_0_1_9_Reg   GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164800 + 0x1a)
#define WriteDQS_0_1_9_Mask  0xFF00
#define WriteDQS_0_1_10_Reg  GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164800 + 0x1c)
#define WriteDQS_0_1_10_Mask 0x00FF
#define WriteDQS_0_1_11_Reg  GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164800 + 0x1c)
#define WriteDQS_0_1_11_Mask 0xFF00
#define WriteDQS_0_1_12_Reg  GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164800 + 0x1e)
#define WriteDQS_0_1_12_Mask 0x00FF
#define WriteDQS_0_1_13_Reg  GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164800 + 0x1e)
#define WriteDQS_0_1_13_Mask 0xFF00
#define WriteDQS_0_1_14_Reg  GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164800 + 0x20)
#define WriteDQS_0_1_14_Mask 0x00FF
#define WriteDQS_0_1_15_Reg  GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164800 + 0x20)
#define WriteDQS_0_1_15_Mask 0xFF00
#define WriteDQS_0_1_16_Reg  GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164800 + 0x22)
#define WriteDQS_0_1_16_Mask 0x00FF
#define WriteDQS_0_1_17_Reg  GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164800 + 0x22)
#define WriteDQS_0_1_17_Mask 0xFF00

#define WDQ1t_0_0_0_Reg   GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164900 + 0x4)
#define WDQ1t_0_0_0_Mask  0x0010
#define WDQ1t_0_0_1_Reg   GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164900 + 0x4)
#define WDQ1t_0_0_1_Mask  0x0020
#define WDQ1t_0_0_2_Reg   GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164900 + 0x4)
#define WDQ1t_0_0_2_Mask  0x0040
#define WDQ1t_0_0_3_Reg   GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164900 + 0x4)
#define WDQ1t_0_0_3_Mask  0x0080
#define WDQ1t_0_0_4_Reg   GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164900 + 0x4)
#define WDQ1t_0_0_4_Mask  0x0100
#define WDQ1t_0_0_5_Reg   GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164900 + 0x4)
#define WDQ1t_0_0_5_Mask  0x0200
#define WDQ1t_0_0_6_Reg   GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164900 + 0x4)
#define WDQ1t_0_0_6_Mask  0x0400
#define WDQ1t_0_0_7_Reg   GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164900 + 0x4)
#define WDQ1t_0_0_7_Mask  0x0800
#define WDQ1t_0_0_8_Reg   GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164900 + 0x4)
#define WDQ1t_0_0_8_Mask  0x1000
#define WDQ1t_0_0_9_Reg   GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164900 + 0x4)
#define WDQ1t_0_0_9_Mask  0x2000
#define WDQ1t_0_0_10_Reg  GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164900 + 0x4)
#define WDQ1t_0_0_10_Mask 0x4000
#define WDQ1t_0_0_11_Reg  GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164900 + 0x4)
#define WDQ1t_0_0_11_Mask 0x8000
#define WDQ1t_0_0_12_Reg  GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164900 + 0x6)
#define WDQ1t_0_0_12_Mask 0x0001
#define WDQ1t_0_0_13_Reg  GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164900 + 0x6)
#define WDQ1t_0_0_13_Mask 0x0002
#define WDQ1t_0_0_14_Reg  GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164900 + 0x6)
#define WDQ1t_0_0_14_Mask 0x0004
#define WDQ1t_0_0_15_Reg  GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164900 + 0x6)
#define WDQ1t_0_0_15_Mask 0x0008
#define WDQ1t_0_0_16_Reg  GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164900 + 0x8)
#define WDQ1t_0_0_16_Mask 0x0010
#define WDQ1t_0_0_17_Reg  GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164900 + 0x8)
#define WDQ1t_0_0_17_Mask 0x0020
#define WDQ1t_0_1_0_Reg   GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164900 + 0x6)
#define WDQ1t_0_1_0_Mask  0x0010
#define WDQ1t_0_1_1_Reg   GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164900 + 0x6)
#define WDQ1t_0_1_1_Mask  0x0020
#define WDQ1t_0_1_2_Reg   GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164900 + 0x6)
#define WDQ1t_0_1_2_Mask  0x0040
#define WDQ1t_0_1_3_Reg   GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164900 + 0x6)
#define WDQ1t_0_1_3_Mask  0x0080
#define WDQ1t_0_1_4_Reg   GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164900 + 0x6)
#define WDQ1t_0_1_4_Mask  0x0100
#define WDQ1t_0_1_5_Reg   GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164900 + 0x6)
#define WDQ1t_0_1_5_Mask  0x0200
#define WDQ1t_0_1_6_Reg   GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164900 + 0x6)
#define WDQ1t_0_1_6_Mask  0x0400
#define WDQ1t_0_1_7_Reg   GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164900 + 0x6)
#define WDQ1t_0_1_7_Mask  0x0800
#define WDQ1t_0_1_8_Reg   GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164900 + 0x6)
#define WDQ1t_0_1_8_Mask  0x1000
#define WDQ1t_0_1_9_Reg   GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164900 + 0x6)
#define WDQ1t_0_1_9_Mask  0x2000
#define WDQ1t_0_1_10_Reg  GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164900 + 0x6)
#define WDQ1t_0_1_10_Mask 0x4000
#define WDQ1t_0_1_11_Reg  GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164900 + 0x6)
#define WDQ1t_0_1_11_Mask 0x8000
#define WDQ1t_0_1_12_Reg  GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164900 + 0x8)
#define WDQ1t_0_1_12_Mask 0x0001
#define WDQ1t_0_1_13_Reg  GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164900 + 0x8)
#define WDQ1t_0_1_13_Mask 0x0002
#define WDQ1t_0_1_14_Reg  GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164900 + 0x8)
#define WDQ1t_0_1_14_Mask 0x0004
#define WDQ1t_0_1_15_Reg  GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164900 + 0x8)
#define WDQ1t_0_1_15_Mask 0x0008
#define WDQ1t_0_1_16_Reg  GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164900 + 0x8)
#define WDQ1t_0_1_16_Mask 0x0040
#define WDQ1t_0_1_17_Reg  GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164900 + 0x8)
#define WDQ1t_0_1_17_Mask 0x0080

#define DllTxREFCode_0_Reg  GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164700 + 0x98)
#define DllTxREFCode_0_Mask 0x00FF

#define WriteDQsSkew_0_0_Reg   GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164800 + 0x2c)
#define WriteDQsSkew_0_0_Mask  0x0007
#define WriteDQsSkew_0_1_Reg   GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164800 + 0x2c)
#define WriteDQsSkew_0_1_Mask  0x0070
#define WriteDQsSkew_0_2_Reg   GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164800 + 0x2c)
#define WriteDQsSkew_0_2_Mask  0x0700
#define WriteDQsSkew_0_3_Reg   GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164800 + 0x2c)
#define WriteDQsSkew_0_3_Mask  0x7000
#define WriteDQsPhase_0_0_Reg  GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164800 + 0x24)
#define WriteDQsPhase_0_0_Mask 0x00FF
#define WriteDQsPhase_0_1_Reg  GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164800 + 0x24)
#define WriteDQsPhase_0_1_Mask 0xFF00
#define WriteDQsPhase_0_2_Reg  GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164800 + 0x26)
#define WriteDQsPhase_0_2_Mask 0x00FF
#define WriteDQsPhase_0_3_Reg  GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164800 + 0x26)
#define WriteDQsPhase_0_3_Mask 0xFF00

void Hal_DQ_Init(void);
void Hal_DQ_Exit(void);
void LoadRegMiu(void);
void Hal_save_dq_dm_phase(void);
void Hal_restore_dq_dm_phase(void);
void Hal_Set_WaitTime(int wait_time);
void Hal_Set_DCDL_Mode(int dcdl_mode);

int Hal_Get_DCDL_Mode(void);
int Hal_Get_WaitTime(void);

U8  Hal_Set_DQS_Osc(U8 miu);
U8  Hal_Set_Freq_DQPh(U8 miu, U16 tar_freq);
int Hal_Check_LPDDR4(void);
int Hal_ReloadPhaseRegs(void);

#endif // #ifndef __HAL_DQ__