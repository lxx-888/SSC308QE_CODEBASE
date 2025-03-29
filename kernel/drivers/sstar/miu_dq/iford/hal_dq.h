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
#define WriteDQSkew_0_0_Mask 0x000F
#define WriteDQSkew_0_1_Reg  GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164800 + 0x2e)
#define WriteDQSkew_0_1_Mask 0x00F0
#define WriteDQSkew_0_2_Reg  GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164800 + 0x2e)
#define WriteDQSkew_0_2_Mask 0x0F00
#define WriteDQSkew_0_3_Reg  GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164800 + 0x2e)
#define WriteDQSkew_0_3_Mask 0xF000

#define WriteDQoenSkew_0_0_Reg  GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164800 + 0x30)
#define WriteDQoenSkew_0_0_Mask 0x000F
#define WriteDQoenSkew_0_1_Reg  GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164800 + 0x30)
#define WriteDQoenSkew_0_1_Mask 0x00F0
#define WriteDQoenSkew_0_2_Reg  GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164800 + 0x30)
#define WriteDQoenSkew_0_2_Mask 0x0F00
#define WriteDQoenSkew_0_3_Reg  GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164800 + 0x30)
#define WriteDQoenSkew_0_3_Mask 0xF000

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

#define WDQ1t_0_0_0_Reg   GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164900 + 0x2)
#define WDQ1t_0_0_0_Mask  0x8000
#define WDQ1t_0_0_1_Reg   GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164900 + 0x4)
#define WDQ1t_0_0_1_Mask  0x0001
#define WDQ1t_0_0_2_Reg   GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164900 + 0x4)
#define WDQ1t_0_0_2_Mask  0x0002
#define WDQ1t_0_0_3_Reg   GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164900 + 0x4)
#define WDQ1t_0_0_3_Mask  0x0004
#define WDQ1t_0_0_4_Reg   GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164900 + 0x4)
#define WDQ1t_0_0_4_Mask  0x0008
#define WDQ1t_0_0_5_Reg   GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164900 + 0x4)
#define WDQ1t_0_0_5_Mask  0x0010
#define WDQ1t_0_0_6_Reg   GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164900 + 0x4)
#define WDQ1t_0_0_6_Mask  0x0020
#define WDQ1t_0_0_7_Reg   GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164900 + 0x4)
#define WDQ1t_0_0_7_Mask  0x0040
#define WDQ1t_0_0_8_Reg   GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164900 + 0x4)
#define WDQ1t_0_0_8_Mask  0x0080
#define WDQ1t_0_0_9_Reg   GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164900 + 0x4)
#define WDQ1t_0_0_9_Mask  0x0100
#define WDQ1t_0_0_10_Reg  GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164900 + 0x4)
#define WDQ1t_0_0_10_Mask 0x0200
#define WDQ1t_0_0_11_Reg  GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164900 + 0x4)
#define WDQ1t_0_0_11_Mask 0x0400
#define WDQ1t_0_0_12_Reg  GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164900 + 0x4)
#define WDQ1t_0_0_12_Mask 0x0800
#define WDQ1t_0_0_13_Reg  GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164900 + 0x4)
#define WDQ1t_0_0_13_Mask 0x1000
#define WDQ1t_0_0_14_Reg  GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164900 + 0x4)
#define WDQ1t_0_0_14_Mask 0x2000
#define WDQ1t_0_0_15_Reg  GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164900 + 0x4)
#define WDQ1t_0_0_15_Mask 0x4000
#define WDQ1t_0_0_16_Reg  GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164900 + 0x4)
#define WDQ1t_0_0_16_Mask 0x8000
#define WDQ1t_0_0_17_Reg  GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164900 + 0x6)
#define WDQ1t_0_0_17_Mask 0x0001
#if 1
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
#endif
#define DllTxREFCode_0_Reg  GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164700 + 0x98)
#define DllTxREFCode_0_Mask 0x00FF

#define WriteDQsSkew_0_0_Reg   GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164800 + 0x2c)
#define WriteDQsSkew_0_0_Mask  0x000F
#define WriteDQsSkew_0_1_Reg   GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164800 + 0x2c)
#define WriteDQsSkew_0_1_Mask  0x00F0
#define WriteDQsSkew_0_2_Reg   GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164800 + 0x2c)
#define WriteDQsSkew_0_2_Mask  0x0F00
#define WriteDQsSkew_0_3_Reg   GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164800 + 0x2c)
#define WriteDQsSkew_0_3_Mask  0xF000
#define WriteDQsPhase_0_0_Reg  GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164800 + 0x24)
#define WriteDQsPhase_0_0_Mask 0x00FF
#define WriteDQsPhase_0_1_Reg  GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164800 + 0x24)
#define WriteDQsPhase_0_1_Mask 0xFF00
#define WriteDQsPhase_0_2_Reg  GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164800 + 0x26)
#define WriteDQsPhase_0_2_Mask 0x00FF
#define WriteDQsPhase_0_3_Reg  GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164800 + 0x26)
#define WriteDQsPhase_0_3_Mask 0xFF00

#define ReadDqsmDelay_0_0_Reg  GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164800 + 0x6a)
#define ReadDqsmDelay_0_0_Mask 0x001F
#define ReadDqsmDelay_0_1_Reg  GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164800 + 0x6a)
#define ReadDqsmDelay_0_1_Mask 0x1F00
#define ReadDqsmDelay_0_2_Reg  GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164800 + 0x6c)
#define ReadDqsmDelay_0_2_Mask 0x001F
#define ReadDqsmDelay_0_3_Reg  GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164800 + 0x6c)
#define ReadDqsmDelay_0_3_Mask 0x1F00
#define ReadDqsmSkew_0_0_Reg   GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164800 + 0x6e)
#define ReadDqsmSkew_0_0_Mask  0x0007
#define ReadDqsmSkew_0_1_Reg   GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164800 + 0x6e)
#define ReadDqsmSkew_0_1_Mask  0x0070
#define ReadDqsmSkew_0_2_Reg   GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164800 + 0x6e)
#define ReadDqsmSkew_0_2_Mask  0x0700
#define ReadDqsmSkew_0_3_Reg   GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164800 + 0x6e)
#define ReadDqsmSkew_0_3_Mask  0x7000
#define ReadDqsmPhase_0_0_Reg  GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164800 + 0x72)
#define ReadDqsmPhase_0_0_Mask 0x00FF
#define ReadDqsmPhase_0_1_Reg  GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164800 + 0x72)
#define ReadDqsmPhase_0_1_Mask 0xFF00
#define ReadDqsmPhase_0_2_Reg  GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164800 + 0x74)
#define ReadDqsmPhase_0_2_Mask 0x00FF
#define ReadDqsmPhase_0_3_Reg  GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164800 + 0x74)
#define ReadDqsmPhase_0_3_Mask 0xFF00

#define WriteClkPhase_0_Reg   GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164800 + 0x9a)
#define WriteClkPhase_0_Mask  0x00FF
#define WriteClk1Phase_0_Reg  GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164800 + 0x9a)
#define WriteClk1Phase_0_Mask 0xFF00
#define WriteClkSkew_0_Reg    GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164800 + 0x9e)
#define WriteClkSkew_0_Mask   0x0007
#define WriteClk1Skew_0_Reg   GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164800 + 0x9e)
#define WriteClk1Skew_0_Mask  0x0070

#define WriteCsPhase_0_Reg   GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164800 + 0x96)
#define WriteCsPhase_0_Mask  0x00FF
#define WriteCs1Phase_0_Reg  GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164800 + 0x96)
#define WriteCs1Phase_0_Mask 0xFF00
#define WriteCsSkew_0_Reg    GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164800 + 0xA0)
#define WriteCsSkew_0_Mask   0x0007
#define WriteCs1Skew_0_Reg   GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164800 + 0xA0)
#define WriteCs1Skew_0_Mask  0x0070

#define WriteCmdSkew_0_Reg      GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164800 + 0xA2)
#define WriteCmdSkew_0_Mask     0x0007
#define WriteCmd1Skew_0_Reg     GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164800 + 0xA2)
#define WriteCmd1Skew_0_Mask    0x0070
#define WriteCmdPhase_0_0_Reg   GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164800 + 0x7A)
#define WriteCmdPhase_0_0_Mask  0x00FF
#define WriteCmdPhase_0_1_Reg   GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164800 + 0x7A)
#define WriteCmdPhase_0_1_Mask  0xFF00
#define WriteCmdPhase_0_2_Reg   GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164800 + 0x7C)
#define WriteCmdPhase_0_2_Mask  0x00FF
#define WriteCmdPhase_0_3_Reg   GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164800 + 0x7C)
#define WriteCmdPhase_0_3_Mask  0xFF00
#define WriteCmdPhase_0_4_Reg   GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164800 + 0x7E)
#define WriteCmdPhase_0_4_Mask  0x00FF
#define WriteCmdPhase_0_5_Reg   GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164800 + 0x7E)
#define WriteCmdPhase_0_5_Mask  0xFF00
#define WriteCmdPhase_0_6_Reg   GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164800 + 0x80)
#define WriteCmdPhase_0_6_Mask  0x00FF
#define WriteCmdPhase_0_7_Reg   GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164800 + 0x80)
#define WriteCmdPhase_0_7_Mask  0xFF00
#define WriteCmdPhase_0_8_Reg   GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164800 + 0x82)
#define WriteCmdPhase_0_8_Mask  0x00FF
#define WriteCmdPhase_0_9_Reg   GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164800 + 0x82)
#define WriteCmdPhase_0_9_Mask  0xFF00
#define WriteCmdPhase_0_10_Reg  GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164800 + 0x84)
#define WriteCmdPhase_0_10_Mask 0x00FF
#define WriteCmdPhase_0_11_Reg  GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164800 + 0x84)
#define WriteCmdPhase_0_11_Mask 0xFF00
#define WriteCmdPhase_0_12_Reg  GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164800 + 0x86)
#define WriteCmdPhase_0_12_Mask 0x00FF
#define WriteCmdPhase_0_13_Reg  GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164800 + 0x86)
#define WriteCmdPhase_0_13_Mask 0xFF00
#define WriteCmdPhase_0_14_Reg  GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164800 + 0x88)
#define WriteCmdPhase_0_14_Mask 0x00FF
#define WriteCmdPhase_0_15_Reg  GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164800 + 0x88)
#define WriteCmdPhase_0_15_Mask 0xFF00
#define WriteCmdPhase_0_16_Reg  GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164800 + 0x8A)
#define WriteCmdPhase_0_16_Mask 0x00FF
#define WriteCmdPhase_0_17_Reg  GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164800 + 0x8A)
#define WriteCmdPhase_0_17_Mask 0xFF00
#define WriteCmdPhase_0_18_Reg  GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164800 + 0x8C)
#define WriteCmdPhase_0_18_Mask 0x00FF
#define WriteCmdPhase_0_19_Reg  GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164800 + 0x8C)
#define WriteCmdPhase_0_19_Mask 0xFF00
#define WriteCmdPhase_0_20_Reg  GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164800 + 0x8E)
#define WriteCmdPhase_0_20_Mask 0x00FF
#define WriteCmdPhase_0_21_Reg  GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164800 + 0x8E)
#define WriteCmdPhase_0_21_Mask 0xFF00
#define WriteCmdPhase_0_22_Reg  GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164800 + 0x90)
#define WriteCmdPhase_0_22_Mask 0x00FF
#define WriteCmdPhase_0_23_Reg  GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164800 + 0x90)
#define WriteCmdPhase_0_23_Mask 0xFF00
#define WriteCmdPhase_0_24_Reg  GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164800 + 0x92)
#define WriteCmdPhase_0_24_Mask 0x00FF
#define WriteCmdPhase_0_25_Reg  GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164800 + 0x92)
#define WriteCmdPhase_0_25_Mask 0xFF00
#define WriteCmdPhase_0_26_Reg  GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164800 + 0x94)
#define WriteCmdPhase_0_26_Mask 0x00FF
#define WriteCmdPhase_0_27_Reg  GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164800 + 0x94)
#define WriteCmdPhase_0_27_Mask 0xFF00

#define DllTxPD_0_Reg     GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164700 + 0x60)
#define DllTxPD_0_Mask    0x0002
#define DllTxKCode_0_Reg  GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164800 + 0xc2)
#define DllTxKCode_0_Mask 0x03FF

#define DFS1_Ref_MROsc_0_Reg  GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164800 + 0x2A)
#define DFS1_Ref_MROsc_0_Mask 0xFFFF

#define DFS1_WriteDQSkew_0_0_Reg  GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164B00 + 0x2e)
#define DFS1_WriteDQSkew_0_0_Mask 0x000F
#define DFS1_WriteDQSkew_0_1_Reg  GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164B00 + 0x2e)
#define DFS1_WriteDQSkew_0_1_Mask 0x00F0
#define DFS1_WriteDQSkew_0_2_Reg  GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164B00 + 0x2e)
#define DFS1_WriteDQSkew_0_2_Mask 0x0F00
#define DFS1_WriteDQSkew_0_3_Reg  GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164B00 + 0x2e)
#define DFS1_WriteDQSkew_0_3_Mask 0xF000

#define DFS1_WriteDQoenSkew_0_0_Reg  GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164B00 + 0x30)
#define DFS1_WriteDQoenSkew_0_0_Mask 0x000F
#define DFS1_WriteDQoenSkew_0_1_Reg  GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164B00 + 0x30)
#define DFS1_WriteDQoenSkew_0_1_Mask 0x00F0
#define DFS1_WriteDQoenSkew_0_2_Reg  GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164B00 + 0x30)
#define DFS1_WriteDQoenSkew_0_2_Mask 0x0F00
#define DFS1_WriteDQoenSkew_0_3_Reg  GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164B00 + 0x30)
#define DFS1_WriteDQoenSkew_0_3_Mask 0xF000

#define DFS1_WriteDQS_0_0_0_Reg   GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164B00 + 0x0)
#define DFS1_WriteDQS_0_0_0_Mask  0x00FF
#define DFS1_WriteDQS_0_0_1_Reg   GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164B00 + 0x0)
#define DFS1_WriteDQS_0_0_1_Mask  0xFF00
#define DFS1_WriteDQS_0_0_2_Reg   GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164B00 + 0x2)
#define DFS1_WriteDQS_0_0_2_Mask  0x00FF
#define DFS1_WriteDQS_0_0_3_Reg   GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164B00 + 0x2)
#define DFS1_WriteDQS_0_0_3_Mask  0xFF00
#define DFS1_WriteDQS_0_0_4_Reg   GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164B00 + 0x4)
#define DFS1_WriteDQS_0_0_4_Mask  0x00FF
#define DFS1_WriteDQS_0_0_5_Reg   GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164B00 + 0x4)
#define DFS1_WriteDQS_0_0_5_Mask  0xFF00
#define DFS1_WriteDQS_0_0_6_Reg   GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164B00 + 0x6)
#define DFS1_WriteDQS_0_0_6_Mask  0x00FF
#define DFS1_WriteDQS_0_0_7_Reg   GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164B00 + 0x6)
#define DFS1_WriteDQS_0_0_7_Mask  0xFF00
#define DFS1_WriteDQS_0_0_8_Reg   GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164B00 + 0x8)
#define DFS1_WriteDQS_0_0_8_Mask  0x00FF
#define DFS1_WriteDQS_0_0_9_Reg   GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164B00 + 0x8)
#define DFS1_WriteDQS_0_0_9_Mask  0xFF00
#define DFS1_WriteDQS_0_0_10_Reg  GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164B00 + 0xa)
#define DFS1_WriteDQS_0_0_10_Mask 0x00FF
#define DFS1_WriteDQS_0_0_11_Reg  GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164B00 + 0xa)
#define DFS1_WriteDQS_0_0_11_Mask 0xFF00
#define DFS1_WriteDQS_0_0_12_Reg  GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164B00 + 0xc)
#define DFS1_WriteDQS_0_0_12_Mask 0x00FF
#define DFS1_WriteDQS_0_0_13_Reg  GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164B00 + 0xc)
#define DFS1_WriteDQS_0_0_13_Mask 0xFF00
#define DFS1_WriteDQS_0_0_14_Reg  GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164B00 + 0xe)
#define DFS1_WriteDQS_0_0_14_Mask 0x00FF
#define DFS1_WriteDQS_0_0_15_Reg  GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164B00 + 0xe)
#define DFS1_WriteDQS_0_0_15_Mask 0xFF00
#define DFS1_WriteDQS_0_0_16_Reg  GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164B00 + 0x10)
#define DFS1_WriteDQS_0_0_16_Mask 0x00FF
#define DFS1_WriteDQS_0_0_17_Reg  GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164B00 + 0x10)
#define DFS1_WriteDQS_0_0_17_Mask 0xFF00
#define DFS1_WriteDQS_0_1_0_Reg   GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164B00 + 0x12)
#define DFS1_WriteDQS_0_1_0_Mask  0x00FF
#define DFS1_WriteDQS_0_1_1_Reg   GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164B00 + 0x12)
#define DFS1_WriteDQS_0_1_1_Mask  0xFF00
#define DFS1_WriteDQS_0_1_2_Reg   GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164B00 + 0x14)
#define DFS1_WriteDQS_0_1_2_Mask  0x00FF
#define DFS1_WriteDQS_0_1_3_Reg   GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164B00 + 0x14)
#define DFS1_WriteDQS_0_1_3_Mask  0xFF00
#define DFS1_WriteDQS_0_1_4_Reg   GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164B00 + 0x16)
#define DFS1_WriteDQS_0_1_4_Mask  0x00FF
#define DFS1_WriteDQS_0_1_5_Reg   GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164B00 + 0x16)
#define DFS1_WriteDQS_0_1_5_Mask  0xFF00
#define DFS1_WriteDQS_0_1_6_Reg   GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164B00 + 0x18)
#define DFS1_WriteDQS_0_1_6_Mask  0x00FF
#define DFS1_WriteDQS_0_1_7_Reg   GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164B00 + 0x18)
#define DFS1_WriteDQS_0_1_7_Mask  0xFF00
#define DFS1_WriteDQS_0_1_8_Reg   GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164B00 + 0x1a)
#define DFS1_WriteDQS_0_1_8_Mask  0x00FF
#define DFS1_WriteDQS_0_1_9_Reg   GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164B00 + 0x1a)
#define DFS1_WriteDQS_0_1_9_Mask  0xFF00
#define DFS1_WriteDQS_0_1_10_Reg  GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164B00 + 0x1c)
#define DFS1_WriteDQS_0_1_10_Mask 0x00FF
#define DFS1_WriteDQS_0_1_11_Reg  GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164B00 + 0x1c)
#define DFS1_WriteDQS_0_1_11_Mask 0xFF00
#define DFS1_WriteDQS_0_1_12_Reg  GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164B00 + 0x1e)
#define DFS1_WriteDQS_0_1_12_Mask 0x00FF
#define DFS1_WriteDQS_0_1_13_Reg  GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164B00 + 0x1e)
#define DFS1_WriteDQS_0_1_13_Mask 0xFF00
#define DFS1_WriteDQS_0_1_14_Reg  GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164B00 + 0x20)
#define DFS1_WriteDQS_0_1_14_Mask 0x00FF
#define DFS1_WriteDQS_0_1_15_Reg  GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164B00 + 0x20)
#define DFS1_WriteDQS_0_1_15_Mask 0xFF00
#define DFS1_WriteDQS_0_1_16_Reg  GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164B00 + 0x22)
#define DFS1_WriteDQS_0_1_16_Mask 0x00FF
#define DFS1_WriteDQS_0_1_17_Reg  GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164B00 + 0x22)
#define DFS1_WriteDQS_0_1_17_Mask 0xFF00

#define DFS1_WDQ1t_0_0_0_Reg   GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164B00 + 0x14)
#define DFS1_WDQ1t_0_0_0_Mask  0x8000
#define DFS1_WDQ1t_0_0_1_Reg   GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164B00 + 0x16)
#define DFS1_WDQ1t_0_0_1_Mask  0x0001
#define DFS1_WDQ1t_0_0_2_Reg   GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164B00 + 0x16)
#define DFS1_WDQ1t_0_0_2_Mask  0x0002
#define DFS1_WDQ1t_0_0_3_Reg   GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164B00 + 0x16)
#define DFS1_WDQ1t_0_0_3_Mask  0x0004
#define DFS1_WDQ1t_0_0_4_Reg   GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164B00 + 0x16)
#define DFS1_WDQ1t_0_0_4_Mask  0x0008
#define DFS1_WDQ1t_0_0_5_Reg   GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164B00 + 0x16)
#define DFS1_WDQ1t_0_0_5_Mask  0x0010
#define DFS1_WDQ1t_0_0_6_Reg   GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164B00 + 0x16)
#define DFS1_WDQ1t_0_0_6_Mask  0x0020
#define DFS1_WDQ1t_0_0_7_Reg   GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164B00 + 0x16)
#define DFS1_WDQ1t_0_0_7_Mask  0x0040
#define DFS1_WDQ1t_0_0_8_Reg   GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164B00 + 0x16)
#define DFS1_WDQ1t_0_0_8_Mask  0x0080
#define DFS1_WDQ1t_0_0_9_Reg   GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164B00 + 0x16)
#define DFS1_WDQ1t_0_0_9_Mask  0x0100
#define DFS1_WDQ1t_0_0_10_Reg  GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164B00 + 0x16)
#define DFS1_WDQ1t_0_0_10_Mask 0x0200
#define DFS1_WDQ1t_0_0_11_Reg  GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164B00 + 0x16)
#define DFS1_WDQ1t_0_0_11_Mask 0x0400
#define DFS1_WDQ1t_0_0_12_Reg  GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164B00 + 0x16)
#define DFS1_WDQ1t_0_0_12_Mask 0x0800
#define DFS1_WDQ1t_0_0_13_Reg  GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164B00 + 0x16)
#define DFS1_WDQ1t_0_0_13_Mask 0x1000
#define DFS1_WDQ1t_0_0_14_Reg  GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164B00 + 0x16)
#define DFS1_WDQ1t_0_0_14_Mask 0x2000
#define DFS1_WDQ1t_0_0_15_Reg  GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164B00 + 0x16)
#define DFS1_WDQ1t_0_0_15_Mask 0x4000
#define DFS1_WDQ1t_0_0_16_Reg  GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164B00 + 0x16)
#define DFS1_WDQ1t_0_0_16_Mask 0x8000
#define DFS1_WDQ1t_0_0_17_Reg  GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164B00 + 0x18)
#define DFS1_WDQ1t_0_0_17_Mask 0x0001

#define DFS1_DllTxPD_0_Reg     GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164A00 + 0x60)
#define DFS1_DllTxPD_0_Mask    0x0002
#define DFS1_DllTxKCode_0_Reg  GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x164B00 + 0xc2)
#define DFS1_DllTxKCode_0_Mask 0x03FF

void Hal_DQ_Init(void);
void Hal_DQ_Exit(void);
void LoadRegMiu(void);
void Hal_save_dq_dm_phase(void);
void Hal_restore_dq_dm_phase(void);
void Hal_Set_DCDL_Mode(int dcdl_mode);

int Hal_Get_DCDL_Mode(void);
#if 0
void Hal_Set_WaitTime(int wait_time);
int Hal_Get_WaitTime(void);
#endif
U8  Hal_Set_DQS_Osc(U8 miu);
U8  Hal_Set_Freq_DQPh(U8 miu, U16 tar_freq);
int Hal_Check_LPDDR4(void);
int Hal_ReloadPhaseRegs(void);
u8  Hal_Set_DvsPhaseScaling(u8 miu);
#endif // #ifndef __HAL_DQ__
