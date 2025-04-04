/*
 * hal_pwm.h- Sigmastar
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
#include "sstar_types.h"
#include <common.h>

//------------------------------------------------------------------------------
//  Define
//------------------------------------------------------------------------------

#define PWM_CLK       (12000000)
#define PWM_GROUP_NUM (3)
#define PWM_PER_GROUP (4)
#define PWM_NUM       (11)
// Common PWM registers
#define u16REG_PWM_SHIFT_L  (0x00 * 4)
#define u16REG_PWM_SHIFT_H  (0x01 * 4)
#define u16REG_PWM_DUTY_L   (0x02 * 4)
#define u16REG_PWM_DUTY_H   (0x03 * 4)
#define u16REG_PWM_PERIOD_L (0x04 * 4) // reg_pwm0_period
#define u16REG_PWM_PERIOD_H (0x05 * 4)
#define u16REG_PWM_DIV      (0x06 * 4)
#define u16REG_PWM_CTRL     (0x07 * 4)
#define VDBEN_SW_BIT        0
#define DBEN_BIT            1
#define DIFF_P_EN_BIT       2
#define SHIFT_GAT_BIT       3
#define POLARITY_BIT        4

#define u16REG_PWM_SHIFT2 (0x08 * 4)
#define u16REG_PWM_DUTY2  (0x09 * 4)
#define u16REG_PWM_SHIFT3 (0x0A * 4)
#define u16REG_PWM_DUTY3  (0x0B * 4)
#define u16REG_PWM_SHIFT4 (0x0C * 4)
#define u16REG_PWM_DUTY4  (0x0D * 4)

#define u16REG_GROUP_ENABLE      (0x73 * 4)
#define u16REG_GROUP_ENABLE_SHFT (0x0)

#define u16REG_GROUP_JOIN        (0x74 * 4)
#define u16REG_GROUP_JOIN_SHFT   (0x0)
#define u16REG_PWM_DUTY_QE0      (0x76 * 4)
#define u16REG_PWM_DUTY_QE0_SHFT (0x0)

#define u16REG_SW_RESET (0x7F * 4)
#define REG_CH_OFFSET   (0x20 * 4)

#define RIU_BASE_ADDR 0x1F000000

#define CHIPTOP_BANK_ADDR   0x101E00
#define BASE_REG_CHIPTOP_PA RIU_BASE_ADDR + (CHIPTOP_BANK_ADDR * 2)
#define REG_ID_07           (0x07 * 2)

#define PMSLEEP_BANK_ADDR   0x000E00
#define BASE_REG_PMSLEEP_PA RIU_BASE_ADDR + (PMSLEEP_BANK_ADDR * 2)
#define REG_ID_28           (0x28 * 2)

#define PWM_BANK_ADDR   0x101900
#define BASE_REG_PWM_PA RIU_BASE_ADDR + (PWM_BANK_ADDR * 2)

#define CLKGEN_BANK_ADDR   (0x103800)
#define BASE_REG_CLKGEN_PA RIU_BASE_ADDR + (CLKGEN_BANK_ADDR * 2)

#define BITS_PER_LONG 32
#define GENMASK(h, l) (((~0UL) - (1UL << (l)) + 1) & (~0UL >> (BITS_PER_LONG - 1 - (h))))

#define BASE_REG_NULL 0xFFFFFFFF

typedef struct
{
    U32 u32PadId;
    U32 u32Mode;
} pwmPadTbl_t;

//------------------------------------------------------------------------------
//  Local Functions
//----------------------------------------------------------------------
void Hal_PWMGetGrpAddr(U32* u32addr, U32* u32PwmOffs, U8 u8Id);
void HalPWMEnableGet(U8 u8Id, U8* pu8Val);
void HalPWMEnable(U8 u8Id, U8 u8Val, U32 u32Pad);
void HalPWMSetConfig(U8 u8Id, U32 duty, U32 period);
void HalPWMGetConfig(U8 u8Id, U32* Duty, U32* Period);
void HalPWMSetDben(U8 u8Id, U8 u8Val);
void HalPWMSetPolarity(U8 u8Id, U8 u8Val);
void HalPWMGetPolarity(U8 u8Id, U8* pu8Val);
void HalPWMInit(U8 u8Id);
void HalPWMPadSet(U8 u8Id, U32 u32Val);
void HalPWMAllGrpEnable(void);
void HalPWMDutyQE0(U8 u8GroupId, U8 u8Val);
int  HalPWMGroupJoin(U8 u8PWMId, U8 u8Val);
int  HalGroupEnable(U8 u8GroupId, U8 u8Val);