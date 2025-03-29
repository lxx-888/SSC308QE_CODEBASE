/*
 * hal_ir.h- Sigmastar
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

#ifndef _HAL_IR_H_
#define _HAL_IR_H_

#include <cam_os_wrapper.h>

#define IR_DEBUG 0
#if IR_DEBUG
#define ir_err(fmt, ...) CamOsPrintf("%s: " fmt, __func__, ##__VA_ARGS__)
#define ir_dbg(fmt, ...) CamOsPrintf("%s: " fmt, __func__, ##__VA_ARGS__)
#else
#define ir_err(fmt, ...) CamOsPrintf("%s: " fmt, __func__, ##__VA_ARGS__)
#define ir_dbg(fmt, ...)
#endif

typedef signed char        s8;
typedef unsigned char      u8;
typedef signed short       s16;
typedef unsigned short     u16;
typedef signed int         s32;
typedef unsigned int       u32;
typedef signed long long   s64;
typedef unsigned long long u64;

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

#define HAL_IR_READ_REG(bank, offset)                  READ_WORD((bank + (offset << 2)))
#define HAL_IR_WRITE_REG(bank, offset, val)            WRITE_WORD((bank + (offset << 2)), val)
#define HAL_IR_WRITE_REG_MASK(bank, offset, val, mask) WRITE_WORD_MASK((bank + (offset << 2)), val, mask)

#define HAL_IR_XTAL_CLKFREQ 12000000 // 12 MHz
#define HAL_IR_RAW_DATA     4

/*
 * IR decode mode options
 */
#define HAL_IR_FULL_MODE 1
#define HAL_IR_RAW_MODE  2
#define HAL_IR_RC5_MODE  3
#define HAL_IR_SW_MODE   4 // only support at linux
#define HAL_IR_RC6_MODE  5 // not support now
#define HAL_IR_SPEC_MODE 6 // only support at linux and IR_TYPE_SEL == IR_TYPE_RCMM

/*
 * IR system parameter define for H/W setting (Please don't modify them)
 */
#define HAL_IR_CLK       (HAL_IR_XTAL_CLKFREQ / 1000000)
#define HAL_IR_CLKDIV    ((HAL_IR_XTAL_CLKFREQ / 1000000) - 1)
#define HAL_IR_RC_CLKDIV ((HAL_IR_XTAL_CLKFREQ / 1000000) - 4)

#define HAL_IR_GET_MINCNT(time, tolerance) \
    ((u32)(((double)time * ((double)HAL_IR_CLK) / (HAL_IR_CLKDIV + 1)) * ((double)1 - tolerance)))

#define HAL_IR_GET_MAXCNT(time, tolerance) \
    ((u32)(((double)time * ((double)HAL_IR_CLK) / (HAL_IR_CLKDIV + 1)) * ((double)1 + tolerance)))

#define HAL_IR_GETCNT(time) ((u32)((double)time * ((double)HAL_IR_CLK) / (HAL_IR_CLKDIV + 1)))

#define HAL_IR_RC_GETCNT(time) ((u32)(double)time * ((double)HAL_IR_CLK) / (HAL_IR_RC_CLKDIV + 1))

#define HAL_IR_RP_TIMEOUT HAL_IR_GETCNT(HAL_IR_TIMEOUT_CYC)
#define HAL_IR_HDC_UPB    HAL_IR_GET_MAXCNT(HAL_IR_HEADER_CODE_TIME, 0.2)
#define HAL_IR_HDC_LOB    HAL_IR_GET_MINCNT(HAL_IR_HEADER_CODE_TIME, 0.2)
#define HAL_IR_OFC_UPB    HAL_IR_GET_MAXCNT(HAL_IR_OFF_CODE_TIME, 0.2)
#define HAL_IR_OFC_LOB    HAL_IR_GET_MINCNT(HAL_IR_OFF_CODE_TIME, 0.2)
#define HAL_IR_OFC_RPUPB  HAL_IR_GET_MAXCNT(HAL_IR_OFF_CODE_RPTIME, 0.2)
#define HAL_IR_OFC_RPLOB  HAL_IR_GET_MINCNT(HAL_IR_OFF_CODE_RPTIME, 0.2)
#define HAL_IR_LG01H_UPB  HAL_IR_GET_MAXCNT(HAL_IR_LOGI_01H_TIME, 0.35)
#define HAL_IR_LG01H_LOB  HAL_IR_GET_MINCNT(HAL_IR_LOGI_01H_TIME, 0.3)
#define HAL_IR_LG0_UPB    HAL_IR_GET_MAXCNT(HAL_IR_LOGI_0_TIME, 0.2)
#define HAL_IR_LG0_LOB    HAL_IR_GET_MINCNT(HAL_IR_LOGI_0_TIME, 0.2)
#define HAL_IR_LG1_UPB    HAL_IR_GET_MAXCNT(HAL_IR_LOGI_1_TIME, 0.2)
#define HAL_IR_LG1_LOB    HAL_IR_GET_MINCNT(HAL_IR_LOGI_1_TIME, 0.2)
#define HAL_IR_RC5_SHOTPL HAL_IR_RC_GETCNT(HAL_IR_RC5_1T_TIME)
#define HAL_IR_RC5_LONGPL HAL_IR_RC_GETCNT((HAL_IR_RC5_2T_TIME + HAL_IR_RC5_1T_TIME))

/*
 * IR Timing define
 */
#define HAL_IR_HEADER_CODE_TIME 9000   // us
#define HAL_IR_OFF_CODE_TIME    4500   // us
#define HAL_IR_OFF_CODE_RPTIME  2500   // us
#define HAL_IR_LOGI_01H_TIME    560    // us
#define HAL_IR_LOGI_0_TIME      1120   // us
#define HAL_IR_LOGI_1_TIME      2240   // us
#define HAL_IR_TIMEOUT_CYC      140000 // us
#define HAL_IR_EVENT_TIMEOUT    220    // us
#define HAL_IR_RC5_1T_TIME      444    // us
#define HAL_IR_RC5_2T_TIME      888    // us

/*
 * Register setting
 */
#define HAL_IR_REG_RC_CFG                 0x00
#define HAL_IR_REG_RC_LONGPL_THR          0x01
#define HAL_IR_REG_RC_DIV                 0x03
#define HAL_IR_REG_RC_KEY                 0x06
#define HAL_IR_REG_RC_FIFO_STATUS         0x08
#define HAL_IR_REG_RC_FIFO_RD             0x09
#define HAL_IR_REG_RC_WAKE_UP             0x0A
#define HAL_IR_REG_CTRL                   0x40
#define HAL_IR_REG_HDC_UPB                0x41
#define HAL_IR_REG_HDC_LOB                0x42
#define HAL_IR_REG_OFC_UPB                0x43
#define HAL_IR_REG_OFC_LOB                0x44
#define HAL_IR_REG_OFC_RP_UPB             0x45
#define HAL_IR_REG_OFC_RP_LOB             0x46
#define HAL_IR_REG_LG01H_UPB              0x47
#define HAL_IR_REG_LG01H_LOB              0x48
#define HAL_IR_REG_LG0_UPB                0x49
#define HAL_IR_REG_LG0_LOB                0x4A
#define HAL_IR_REG_LG1_UPB                0x4B
#define HAL_IR_REG_LG1_LOB                0x4C
#define HAL_IR_REG_SEPR_UPB               0x4D
#define HAL_IR_REG_SEPR_LOB               0x4E
#define HAL_IR_REG_TIMEOUT_CYC_L          0x4F
#define HAL_IR_REG_TIMEOUT_CYC_H          0x50
#define HAL_IR_REG_SEPR_BIT_FIFO_CTRL     0x51
#define HAL_IR_REG_CCODE                  0x52
#define HAL_IR_REG_GLHRM_NUM              0x53
#define HAL_IR_REG_CKDIV_NUM_KEY_DATA     0x54
#define HAL_IR_REG_SHOT_CNT_L             0x55
#define HAL_IR_REG_SHOT_CNT_H_FIFO_STATUS 0x56
#define HAL_IR_REG_FIFO_RD_PULSE          0x58

#define HAL_IR_GENERAL_CTRL      0x01BF
#define HAL_IR_GENERAL_GLHRM     0x0804
#define HAL_IR_GENERAL_FIFO_CTRL 0x0F00
#define HAL_IR_FIFO_CLEARL       (0x01 << 15)
#define HAL_IR_FIFO_RDPULSE      0x0001
#define HAL_IR_FIFO_EMPTY        0x0200
#define HAL_IR_CCB_CB            0x9F00 // ir_ccode_byte:1+ir_code_bit_num:32
#define HAL_IR_GET_KEYDATA       0X0008
#define HAL_IR_RPT_FLAG          0x0100

#define HAL_IR_FULL_GLHRM   (0x03 << 12)
#define HAL_IR_FULL_WAKE_UP 0x0020

#define HAL_IR_RAW_CTRL    0x01B3
#define HAL_IR_RAW_GLHRM   (0x02 << 12)
#define HAL_IR_RAW_WAKE_UP 0x0020

#define HAL_IR_RC_KEY_FLAG      0x8000
#define HAL_IR_RC_KEY_ADDR      0x001F
#define HAL_IR_RC_KEY_CMD       0x7F00
#define HAL_IR_RC_FIFO_EMPTY    0x0001
#define HAL_IR_RC_CLKDIV_MASE   0x1F00
#define HAL_IR_RC5_WAKE_UP      (0X1 << 9)
#define HAL_IR_RC5_FIFO_EMPTY   0x0001
#define HAL_IR_RC5_FIFO_RDPULSE 0x0001
#define HAL_IR_RC5_DECODE       0x0100
#define HAL_IR_RC5_EXT_DECODE   0x0500
#define HAL_IR_RC6_DECODE       0x0300

#define HAL_IR_SW_GLHRM       (0x01 << 12)
#define HAL_IR_SW_EDGE_PSHOT  (0x01 << 12)
#define HAL_IR_SW_EDGE_NSHOT  0x2F00
#define HAL_IR_SW_EDGE_ALL    (0x03 << 12)
#define HAL_IR_SW_CLKDIV      0x00CF
#define HAL_IR_SW_FIFO_EN     (0x01 << 14)
#define HAL_IR_SW_RECOV_SHOT  (0x01 << 6)
#define HAL_IR_SW_FIFO_STATUS 0X0200
#define HAL_IR_SW_SHOT_H      0X0007
#define HAL_IR_SW_SHOT_TYPE   0x0010

/*
 * Struct declaration
 */

struct hal_ir_key_info
{
    u8  key;
    u8  flag;
    u8  valid;
    u16 addr;
};

struct hal_ir_dev
{
    u8  prev_fullkey;
    s32 rept_full;
    u32 full_ccode[2];
    u64 prev_time;
    u64 current_time;

    u8  prev_rawkey;
    u16 prev_rawaddr;
    u8  key_count;
    u8  raw_key[HAL_IR_RAW_DATA];
    s32 rept_raw;

    u8  sw_shot_type[64];
    u32 sw_shot_count[64];
    u32 sw_shot_total;

    u32 decode_mode;
    u64 membase;
    u64 (*calbak_get_sys_time)(void);
    struct hal_ir_key_info *decode_info;
};

/*
 * Function declaration
 */
extern int  hal_ir_init(struct hal_ir_dev *ir_dev);
extern void hal_ir_config(struct hal_ir_dev *ir_dev);
extern u16  hal_ir_get_status(struct hal_ir_dev *ir_dev);
extern u32  hal_ir_get_key(struct hal_ir_dev *ir_dev);
extern void hal_ir_set_software(struct hal_ir_dev *ir_dev, int enable);

#endif
