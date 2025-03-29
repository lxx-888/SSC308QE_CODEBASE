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

#ifndef _HAL_PWM_H_
#define _HAL_PWM_H_

#include <pwm_os.h>

#ifdef CONFIG_ARM64
#define READ_WORD(_reg)        (*(volatile u16 *)(u64)(_reg))
#define WRITE_WORD(_reg, _val) ((*(volatile u16 *)(u64)(_reg)) = (u16)(_val))
#define WRITE_WORD_MASK(_reg, _val, _mask) \
    ((*(volatile u16 *)(u64)(_reg)) = ((*(volatile u16 *)(u64)(_reg)) & ~(_mask)) | ((u16)(_val) & (_mask)))
#else
#define READ_WORD(_reg)        (*(volatile u16 *)(u32)(_reg))
#define WRITE_WORD(_reg, _val) ((*(volatile u16 *)(u32)(_reg)) = (u16)(_val))
#define WRITE_WORD_MASK(_reg, _val, _mask) \
    ((*(volatile u16 *)(u32)(_reg)) = ((*(volatile u16 *)(u32)(_reg)) & ~(_mask)) | ((u16)(_val) & (_mask)))
#endif

#define HAL_PWM_READ_REG(bank, offset)                  READ_WORD((bank + (offset << 2)))
#define HAL_PWM_WRITE_REG(bank, offset, val)            WRITE_WORD((bank + (offset << 2)), val)
#define HAL_PWM_WRITE_REG_MASK(bank, offset, val, mask) WRITE_WORD_MASK((bank + (offset << 2)), val, mask)

enum hal_pwm_errno
{
    HAL_PWM_CHANNEL_ERR = 1,
    HAL_PWM_GROUP_ERR,
    HAL_PWM_DIV_ERR,
    HAL_PWM_DUTY_ERR,
    HAL_PWM_SHIFT_ERR,
    HAL_PWM_PERIOD_ERR,
};

struct hal_pwm_attr
{
    u64 duty;
    u64 shift;
    u32 reset;
    u32 polar;
    u64 period;
    u64 divider;
    u64 duty_wf;
    u64 shift_wf;
    u64 period_wf;
    u8  duty_sync;
    u8  polar_sync;
    u8  shift_sync;
    u8  period_sync;
};

struct hal_pwm_cfg
{
    u32                 group;
    u32                 spwm_gp;
    u32                 channel;
    u32                 hz_to_ns;
    u32                 clk_freq;
    u64                 ddt_base;
    struct hal_pwm_attr pwm_attr;
    u64                 addr_base;
    u64                 bank_base;

#ifdef CONFIG_SSTAR_PWM_DDT
    struct hal_ddt_attr ddt_attr;
#endif
};

struct hal_pwm_com_res
{
    u32 bit;
    u32 offset;
    u32 bank;
};

#define HAL_PWM_BIT0  0x00000001
#define HAL_PWM_BIT1  0x00000002
#define HAL_PWM_BIT2  0x00000004
#define HAL_PWM_BIT3  0x00000008
#define HAL_PWM_BIT4  0x00000010
#define HAL_PWM_BIT5  0x00000020
#define HAL_PWM_BIT6  0x00000040
#define HAL_PWM_BIT7  0x00000080
#define HAL_PWM_BIT8  0x00000100
#define HAL_PWM_BIT9  0x00000200
#define HAL_PWM_BIT10 0x00000400
#define HAL_PWM_BIT11 0x00000800
#define HAL_PWM_BIT12 0x00001000
#define HAL_PWM_BIT13 0x00002000
#define HAL_PWM_BIT14 0x00004000
#define HAL_PWM_BIT15 0x00008000
#define HAL_PWM_BIT16 0x00010000
#define HAL_PWM_BIT17 0x00020000
#define HAL_PWM_BIT18 0x00040000
#define HAL_PWM_BIT19 0x00080000
#define HAL_PWM_BIT20 0x00100000
#define HAL_PWM_BIT21 0x00200000
#define HAL_PWM_BIT22 0x00400000
#define HAL_PWM_BIT23 0x00800000
#define HAL_PWM_BIT24 0x01000000
#define HAL_PWM_BIT25 0x02000000
#define HAL_PWM_BIT26 0x04000000
#define HAL_PWM_BIT27 0x08000000
#define HAL_PWM_BIT28 0x10000000
#define HAL_PWM_BIT29 0x20000000
#define HAL_PWM_BIT30 0x40000000
#define HAL_PWM_BIT31 0x80000000

extern u32  hal_pwm_get_group_reset(u64 addr, u32 group);
extern void hal_pwm_set_group_reset(u64 addr, u32 group, u8 reset);
extern u32  hal_pwm_get_group_status(u64 addr, u32 group);
extern void hal_pwm_group_enbale(u64 addr, u32 group, u8 enable);
extern u32  hal_pwm_get_hold_int(u64 addr, u32 group);
extern u32  hal_pwm_get_round_int(u64 addr, u32 group);
extern u32  hal_pwm_get_hold_mode1_status(u64 addr);
extern void hal_pwm_hold_mode1_enable(u64 addr, u8 enable);
extern u32  hal_pwm_get_hold_mode_status(u64 addr, u32 group);
extern void hal_pwm_hold_mode_enable(u64 addr, u32 group, u8 enable);
extern u32  hal_pwm_get_round_cnt(u64 addr, u32 group);
extern u32  hal_pwm_get_round_num(u64 addr, u32 group);
extern void hal_pwm_set_round_num(u64 addr, u32 group, u32 value);
extern u32  hal_pwm_get_stop_mode_status(u64 addr, u32 group);
extern void hal_pwm_stop_mode_enable(u64 addr, u32 group, u8 enable);
extern u32  hal_pwm_get_sync_mode_status(struct hal_pwm_cfg *pwm_cfg);
extern void hal_pwm_sync_mode_enable(struct hal_pwm_cfg *pwm_cfg, u8 enable);
extern u32  hal_pwm_get_channel_reset(struct hal_pwm_cfg *pwm_cfg);
extern void hal_pwm_set_channel_reset(struct hal_pwm_cfg *pwm_cfg, u8 reset);
extern u32  hal_pwm_get_div(struct hal_pwm_cfg *pwm_cfg);
extern u64  hal_pwm_get_period_hz(struct hal_pwm_cfg *pwm_cfg);
extern int  hal_pwm_set_period(struct hal_pwm_cfg *pwm_cfg, u64 period_wf, u8 sync);
extern u64  hal_pwm_get_duty_pct(struct hal_pwm_cfg *pwm_cfg);
extern int  hal_pwm_set_duty(struct hal_pwm_cfg *pwm_cfg, u64 duty_wf, u8 sync);
extern u64  hal_pwm_get_shift_pct(struct hal_pwm_cfg *pwm_cfg);
extern int  hal_pmw_set_shift(struct hal_pwm_cfg *pwm_cfg, u64 shift_wf, u8 sync);
extern u32  hal_pwm_get_polarity(struct hal_pwm_cfg *pwm_cfg);
extern void hal_pwm_set_polarity(struct hal_pwm_cfg *pwm_cfg, u8 value, u8 sync);
extern u8   hal_pwm_sync_config(struct hal_pwm_cfg *pwm_cfg);
extern void hal_pwm_deinit(struct hal_pwm_cfg *pwm_cfg);
extern int  hal_pwm_init(struct hal_pwm_cfg *pwm_cfg);
#ifdef CONFIG_PM_SLEEP
extern void hal_pwm_suspend_channel(struct hal_pwm_cfg *pwm_cfg);
extern void hal_pwm_suspend_group(u64 addr, u32 group);
extern void hal_pwm_resume_channel(struct hal_pwm_cfg *pwm_cfg);
extern void hal_pwm_resume_group(u64 addr, u32 group);
#endif

#define HAL_PWM_MAX_GROUP        2
#define HAL_PWM_MAX_CHANNEL      12
#define HAL_PWM_MAX_ADD_GROUP_CH 8
#define HAL_PWM_HIGH_PRECISION   1000000000

/* General register */
#define HAL_PWM_REG_SHIFT_L      0X00
#define HAL_PWM_REG_SHIFT_H      0X01
#define HAL_PWM_REG_DUTY_L       0X02
#define HAL_PWM_REG_DUTY_H       0X03
#define HAL_PWM_REG_PERIOD_L     0X04
#define HAL_PWM_REG_PERIOD_H     0X05
#define HAL_PWM_REG_DIVIDER      0X06
#define HAL_PWM_REG_VDBEN_SW     0X07
#define HAL_PWM_REG_DBEN         0X07
#define HAL_PWM_REG_DIFFP_EN     0X07
#define HAL_PWM_REG_POLARITY     0X07
#define HAL_PWM_REG_SHIFT2       0X08
#define HAL_PWM_REG_DUTY2        0X09
#define HAL_PWM_REG_SHIFT3       0X0A
#define HAL_PWM_REG_DUTY3        0X0B
#define HAL_PWM_REG_SHIFT4       0X0C
#define HAL_PWM_REG_DUTY4        0X0D
#define HAL_PWM_REG_DUTY_QE0     0X76
#define HAL_PWM_REG_HOLD_MODE_1  0X77
#define HAL_PWM_REG_DUTY0_POL_EN 0X78

#define HAL_PWM_VDBEN_SW_BIT  0x0
#define HAL_PWM_DBEN_BIT      0x1
#define HAL_PWM_DIFFP_EN_BIT  0x2
#define HAL_PWM_POLARITY_BIT  0x4
#define HAL_PWM_VDBEN_SW_MASK HAL_PWM_BIT0
#define HAL_PWM_DBEN_MASK     HAL_PWM_BIT1
#define HAL_PWM_DIFFP_EN_MASK HAL_PWM_BIT2
#define HAL_PWM_POLARITY_MASK HAL_PWM_BIT4

#define HAL_PWM_DUTY_QE0_BIT      0x0
#define HAL_PWM_HOLD_MODE_1_BIT   0x0
#define HAL_PWM_DUTY0_POL_EN_BIT  0x0
#define HAL_PWM_DUTY_QE0_MASK     HAL_PWM_BIT0
#define HAL_PWM_HOLD_MODE_1_MASK  HAL_PWM_BIT0
#define HAL_PWM_DUTY0_POL_EN_MASK HAL_PWM_BIT0

static const struct hal_pwm_com_res pwm_round_mode[] = {
    {.bank = 0x1019, .offset = 0x10},
    {.bank = 0x1019, .offset = 0x30},
};

static const struct hal_pwm_com_res pwm_hold_mode[] = {
    {.bank = 0x1019, .offset = 0x71, .bit = HAL_PWM_BIT0},
    {.bank = 0x1019, .offset = 0x71, .bit = HAL_PWM_BIT1},
};

static const struct hal_pwm_com_res pwm_stop_mode[] = {
    {.bank = 0x1019, .offset = 0x72, .bit = HAL_PWM_BIT0},
    {.bank = 0x1019, .offset = 0x72, .bit = HAL_PWM_BIT1},
};

static const struct hal_pwm_com_res pwm_group_enable[] = {
    {.bank = 0x1019, .offset = 0x73, .bit = HAL_PWM_BIT0},
    {.bank = 0x1019, .offset = 0x73, .bit = HAL_PWM_BIT1},
};

static const struct hal_pwm_com_res pwm_sync_mode[] = {
    {.bank = 0x1019, .offset = 0x74, .bit = HAL_PWM_BIT0}, {.bank = 0x1019, .offset = 0x74, .bit = HAL_PWM_BIT1},
    {.bank = 0x1019, .offset = 0x74, .bit = HAL_PWM_BIT2}, {.bank = 0x1019, .offset = 0x74, .bit = HAL_PWM_BIT3},
    {.bank = 0x1019, .offset = 0x74, .bit = HAL_PWM_BIT4}, {.bank = 0x1019, .offset = 0x74, .bit = HAL_PWM_BIT5},
    {.bank = 0x1019, .offset = 0x74, .bit = HAL_PWM_BIT6}, {.bank = 0x1019, .offset = 0x74, .bit = HAL_PWM_BIT7},
};

static const struct hal_pwm_com_res pwm_hold_int[] = {
    {.bank = 0x1019, .offset = 0x75, .bit = HAL_PWM_BIT0},
    {.bank = 0x1019, .offset = 0x75, .bit = HAL_PWM_BIT1},
};

static const struct hal_pwm_com_res pwm_round_int[] = {
    {.bank = 0x1019, .offset = 0x75, .bit = HAL_PWM_BIT3},
    {.bank = 0x1019, .offset = 0x75, .bit = HAL_PWM_BIT4},
};

static const struct hal_pwm_com_res pwm_duty_qe0[] = {
    {.bank = 0x1019, .offset = 0x76, .bit = HAL_PWM_BIT0},
    {.bank = 0x001A, .offset = 0x76, .bit = HAL_PWM_BIT0},
};

static const struct hal_pwm_com_res pwm_hold_mode1[] = {
    {.bank = 0x1019, .offset = 0x77, .bit = HAL_PWM_BIT0},
};

static const struct hal_pwm_com_res pwm_duty0_pol[] = {
    {.bank = 0x1019, .offset = 0x78, .bit = HAL_PWM_BIT0},
    {.bank = 0x001A, .offset = 0x78, .bit = HAL_PWM_BIT0},
};

static const struct hal_pwm_com_res pwm_channel_reset[] = {
    {.bank = 0x1019, .offset = 0x7F, .bit = HAL_PWM_BIT0}, {.bank = 0x1019, .offset = 0x7F, .bit = HAL_PWM_BIT1},
    {.bank = 0x1019, .offset = 0x7F, .bit = HAL_PWM_BIT2}, {.bank = 0x1019, .offset = 0x7F, .bit = HAL_PWM_BIT3},
    {.bank = 0x1019, .offset = 0x7F, .bit = HAL_PWM_BIT4}, {.bank = 0x1019, .offset = 0x7F, .bit = HAL_PWM_BIT5},
    {.bank = 0x1019, .offset = 0x7F, .bit = HAL_PWM_BIT6}, {.bank = 0x1019, .offset = 0x7F, .bit = HAL_PWM_BIT7},
    {.bank = 0x001A, .offset = 0x7F, .bit = HAL_PWM_BIT0}, {.bank = 0x001A, .offset = 0x7F, .bit = HAL_PWM_BIT1},
    {.bank = 0x001A, .offset = 0x7F, .bit = HAL_PWM_BIT2}, {.bank = 0x001A, .offset = 0x7F, .bit = HAL_PWM_BIT3},
};

static const struct hal_pwm_com_res pwm_group_reset[] = {
    {.bank = 0x1019, .offset = 0x7F, .bit = HAL_PWM_BIT11},
    {.bank = 0x1019, .offset = 0x7F, .bit = HAL_PWM_BIT12},
};

#endif
