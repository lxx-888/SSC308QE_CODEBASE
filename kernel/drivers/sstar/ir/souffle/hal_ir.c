/*
 * hal_ir.c- Sigmastar
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

#include "hal_ir.h"

/*
 * function
 */
static void hal_set_timing(struct hal_ir_dev *ir_dev)
{
    switch (ir_dev->decode_mode)
    {
        case HAL_IR_FULL_MODE:
        case HAL_IR_RAW_MODE:
        case HAL_IR_SW_MODE:
            // header code upper/lower bound
            HAL_IR_WRITE_REG(ir_dev->membase, HAL_IR_REG_HDC_UPB, HAL_IR_HDC_UPB);
            HAL_IR_WRITE_REG(ir_dev->membase, HAL_IR_REG_HDC_LOB, HAL_IR_HDC_LOB);

            // off code upper/lower bound
            HAL_IR_WRITE_REG(ir_dev->membase, HAL_IR_REG_OFC_UPB, HAL_IR_OFC_UPB);
            HAL_IR_WRITE_REG(ir_dev->membase, HAL_IR_REG_OFC_LOB, HAL_IR_OFC_LOB);

            // off code repeat upper/lower bound
            HAL_IR_WRITE_REG(ir_dev->membase, HAL_IR_REG_OFC_RP_UPB, HAL_IR_OFC_RPUPB);
            HAL_IR_WRITE_REG(ir_dev->membase, HAL_IR_REG_OFC_RP_LOB, HAL_IR_OFC_RPLOB);

            // logical 0/1 high upper/lower bound
            HAL_IR_WRITE_REG(ir_dev->membase, HAL_IR_REG_LG01H_UPB, HAL_IR_LG01H_UPB);
            HAL_IR_WRITE_REG(ir_dev->membase, HAL_IR_REG_LG01H_LOB, HAL_IR_LG01H_LOB);

            // logical 0 upper/lower bound
            HAL_IR_WRITE_REG(ir_dev->membase, HAL_IR_REG_LG0_UPB, HAL_IR_LG0_UPB);
            HAL_IR_WRITE_REG(ir_dev->membase, HAL_IR_REG_LG0_LOB, HAL_IR_LG0_LOB);

            // logical 1 upper/lower bound
            HAL_IR_WRITE_REG(ir_dev->membase, HAL_IR_REG_LG1_UPB, HAL_IR_LG1_UPB);
            HAL_IR_WRITE_REG(ir_dev->membase, HAL_IR_REG_LG1_LOB, HAL_IR_LG1_LOB);

            // timeout cycles
            HAL_IR_WRITE_REG(ir_dev->membase, HAL_IR_REG_TIMEOUT_CYC_L, HAL_IR_RP_TIMEOUT & 0xFFFF);
            // set up ccode bytes and code bytes/bits num
            HAL_IR_WRITE_REG(ir_dev->membase, HAL_IR_REG_TIMEOUT_CYC_H,
                             HAL_IR_CCB_CB | 0x30UL | ((HAL_IR_RP_TIMEOUT >> 16) & 0x0F));

            HAL_IR_WRITE_REG(ir_dev->membase, HAL_IR_REG_CKDIV_NUM_KEY_DATA, HAL_IR_CLKDIV);
            break;
        case HAL_IR_RC5_MODE:
            HAL_IR_WRITE_REG(ir_dev->membase, HAL_IR_REG_RC_DIV,
                             (HAL_IR_READ_REG(ir_dev->membase, HAL_IR_REG_RC_DIV) & ~(HAL_IR_RC_CLKDIV_MASE))
                                 | (HAL_IR_RC_CLKDIV << 8));
            HAL_IR_WRITE_REG(ir_dev->membase, HAL_IR_REG_RC_LONGPL_THR, HAL_IR_RC5_LONGPL);
            break;
        default:
            ir_err("invalid decode mode\n");
    }
}

static void hal_clear_fifo(struct hal_ir_dev *ir_dev)
{
    HAL_IR_WRITE_REG_MASK(ir_dev->membase, HAL_IR_REG_SEPR_BIT_FIFO_CTRL, HAL_IR_FIFO_CLEARL, HAL_IR_FIFO_CLEARL);
}

static u8 hal_read_fifo(struct hal_ir_dev *ir_dev)
{
    u8 key_value = 0;

    key_value = HAL_IR_READ_REG(ir_dev->membase, HAL_IR_REG_CKDIV_NUM_KEY_DATA) >> HAL_IR_GET_KEYDATA;
    HAL_IR_WRITE_REG_MASK(ir_dev->membase, HAL_IR_REG_FIFO_RD_PULSE, HAL_IR_FIFO_RDPULSE, HAL_IR_FIFO_RDPULSE);
    return key_value;
}

static void hal_get_key_fullmode(struct hal_ir_dev *ir_dev, struct hal_ir_key_info *full_info)
{
    u8  current_key = 0;
    u16 reg_value   = 0;

    full_info->addr = (ir_dev->full_ccode[0] << 8) | ir_dev->full_ccode[1];
    reg_value       = HAL_IR_READ_REG(ir_dev->membase, HAL_IR_REG_SHOT_CNT_H_FIFO_STATUS);
    if (reg_value & HAL_IR_FIFO_EMPTY)
    {
        ir_dev->rept_full = 0;
        full_info->valid  = 0;
        return;
    }

    ir_dev->current_time = ir_dev->calbak_get_sys_time();
    if (ir_dev->current_time - ir_dev->prev_time >= HAL_IR_TIMEOUT_CYC * 1000)
    {
        current_key          = hal_read_fifo(ir_dev);
        ir_dev->rept_full    = 0;
        full_info->flag      = 0;
        ir_dev->prev_fullkey = current_key;
        full_info->key       = current_key;
        full_info->valid     = 1;
    }
    else
    {
        if (ir_dev->rept_full == 0)
        {
            ir_dev->rept_full = 1;
            full_info->valid  = 0;
        }
        else
        {
            reg_value        = HAL_IR_READ_REG(ir_dev->membase, HAL_IR_REG_SHOT_CNT_H_FIFO_STATUS);
            full_info->flag  = (reg_value & HAL_IR_RPT_FLAG) ? 1 : 0;
            current_key      = hal_read_fifo(ir_dev);
            full_info->key   = current_key;
            full_info->valid = ((full_info->flag) && (current_key == ir_dev->prev_fullkey));
        }
    }
    ir_dev->prev_time = ir_dev->current_time;

    while (!(reg_value & HAL_IR_FIFO_EMPTY))
    {
        current_key = hal_read_fifo(ir_dev);
        reg_value   = HAL_IR_READ_REG(ir_dev->membase, HAL_IR_REG_SHOT_CNT_H_FIFO_STATUS);
    }

    ir_dbg("in FULL decode mode, the value of addr  [0x%x]\n", full_info->addr);
    ir_dbg("in FULL decode mode, the value of key   [0x%x]\n", full_info->key);
    ir_dbg("in FULL decode mode, the value of flag  [0x%x]\n", full_info->flag);
    ir_dbg("in FULL decode mode, the value of valid [0x%x]\n", full_info->valid);
}

static void hal_get_key_rawmode(struct hal_ir_dev *ir_dev, struct hal_ir_key_info *raw_info)
{
    u8  i;
    u16 reg_value = 0;

    reg_value = HAL_IR_READ_REG(ir_dev->membase, HAL_IR_REG_SHOT_CNT_H_FIFO_STATUS);
    if (ir_dev->rept_raw == 1 && (reg_value & HAL_IR_RPT_FLAG) == HAL_IR_RPT_FLAG)
    {
        ir_dev->rept_raw = 0;
        raw_info->flag   = 1;
        raw_info->valid  = 1;
        raw_info->addr   = ir_dev->prev_rawaddr;
        raw_info->key    = ir_dev->prev_rawkey;
        hal_clear_fifo(ir_dev);
        return;
    }
    ir_dev->rept_raw = 1;

    for (i = 0; i < HAL_IR_RAW_DATA; i++)
    {
        reg_value = HAL_IR_READ_REG(ir_dev->membase, HAL_IR_REG_SHOT_CNT_H_FIFO_STATUS);
        if (reg_value & HAL_IR_FIFO_EMPTY)
        {
            ir_dev->rept_raw = 0;
            raw_info->valid  = 0;
            return;
        }

        ir_dev->raw_key[ir_dev->key_count++] = hal_read_fifo(ir_dev);
        if (ir_dev->key_count == HAL_IR_RAW_DATA)
        {
            ir_dev->key_count = 0;
            if (ir_dev->raw_key[2] == ((u8)(~ir_dev->raw_key[3])))
            {
                raw_info->flag       = 0;
                raw_info->valid      = 1;
                raw_info->addr       = (ir_dev->raw_key[0] << 8) | ir_dev->raw_key[1];
                raw_info->key        = ir_dev->raw_key[2];
                ir_dev->rept_raw     = 0;
                ir_dev->prev_rawaddr = (ir_dev->raw_key[0] << 8) | ir_dev->raw_key[1];
                ir_dev->prev_rawkey  = ir_dev->raw_key[2];
            }
        }
    }

    hal_clear_fifo(ir_dev);
    ir_dbg("in RAW decode mode, the value of addr  [0x%x]\n", raw_info->addr);
    ir_dbg("in RAW decode mode, the value of key   [0x%x]\n", raw_info->key);
    ir_dbg("in RAW decode mode, the value of flag  [0x%x]\n", raw_info->flag);
    ir_dbg("in RAW decode mode, the value of valid [0x%x]\n", raw_info->valid);
}

static void hal_get_key_rc5mode(struct hal_ir_dev *ir_dev, struct hal_ir_key_info *rc5_info)
{
    u16 rc5_code;
    u16 rc5_fifo;

    rc5_fifo = HAL_IR_READ_REG(ir_dev->membase, HAL_IR_REG_RC_FIFO_STATUS);
    if (rc5_fifo & HAL_IR_RC_FIFO_EMPTY)
    {
        rc5_info->valid = 0;
        return;
    }

    rc5_code        = HAL_IR_READ_REG(ir_dev->membase, HAL_IR_REG_RC_KEY);
    rc5_info->valid = 1;
    rc5_info->addr  = rc5_code & HAL_IR_RC_KEY_ADDR;
    rc5_info->key   = ((rc5_code & HAL_IR_RC_KEY_CMD) >> 0x8);
    rc5_info->flag  = ((rc5_code & HAL_IR_RC_KEY_FLAG) >> 0xF);
    HAL_IR_WRITE_REG(ir_dev->membase, HAL_IR_REG_RC_FIFO_RD, HAL_IR_RC5_FIFO_RDPULSE);

    ir_dbg("in RC5 decode mode, the value of addr  [0x%x]\n", rc5_info->addr);
    ir_dbg("in RC5 decode mode, the value of key   [0x%x]\n", rc5_info->key);
    ir_dbg("in RC5 decode mode, the value of flag  [0x%x]\n", rc5_info->flag);
    ir_dbg("in RC5 decode mode, the value of valid [0x%x]\n", rc5_info->valid);
}

static void hal_get_key_swmode(struct hal_ir_dev *ir_dev)
{
    u32 i;
    u32 status;

    status = (HAL_IR_READ_REG(ir_dev->membase, HAL_IR_REG_SHOT_CNT_H_FIFO_STATUS) & HAL_IR_SW_FIFO_STATUS)
             == HAL_IR_SW_FIFO_STATUS;

    for (i = 0; !status; i++)
    {
        ir_dev->sw_shot_count[i] =
            (HAL_IR_READ_REG(ir_dev->membase, HAL_IR_REG_SHOT_CNT_L)
             | ((HAL_IR_READ_REG(ir_dev->membase, HAL_IR_REG_SHOT_CNT_H_FIFO_STATUS) & HAL_IR_SW_SHOT_H) << 16));

        ir_dev->sw_shot_type[i] =
            ((HAL_IR_READ_REG(ir_dev->membase, HAL_IR_REG_SHOT_CNT_H_FIFO_STATUS) & HAL_IR_SW_SHOT_TYPE)
             == HAL_IR_SW_SHOT_TYPE);

        HAL_IR_WRITE_REG_MASK(ir_dev->membase, HAL_IR_REG_FIFO_RD_PULSE, HAL_IR_FIFO_RDPULSE, HAL_IR_FIFO_RDPULSE);

        status = (HAL_IR_READ_REG(ir_dev->membase, HAL_IR_REG_SHOT_CNT_H_FIFO_STATUS) & HAL_IR_SW_FIFO_STATUS)
                 == HAL_IR_SW_FIFO_STATUS;
        ir_dbg("in SW decode mode, sw_shot_count[%u] : %u\n", i, ir_dev->sw_shot_count[i]);
        ir_dbg("in SW decode mode, sw_shot_type [%u] : %u\n", i, ir_dev->sw_shot_type[i]);
    }
    ir_dev->sw_shot_total = i;
}

int hal_ir_init(struct hal_ir_dev *ir_dev)
{
    u16 reg_value = 0;

    ir_dev->prev_fullkey = 0;
    ir_dev->rept_full    = 0;
    ir_dev->prev_time    = 0;
    ir_dev->current_time = 0;

    ir_dev->rept_raw    = 0;
    ir_dev->prev_rawkey = 0;
    ir_dev->key_count   = 0;

    hal_set_timing(ir_dev);

    HAL_IR_WRITE_REG(ir_dev->membase, HAL_IR_REG_CCODE, ((ir_dev->full_ccode[1] << 8) | ir_dev->full_ccode[0]));
    HAL_IR_WRITE_REG(ir_dev->membase, HAL_IR_REG_CTRL, HAL_IR_GENERAL_CTRL);
    HAL_IR_WRITE_REG(ir_dev->membase, HAL_IR_REG_GLHRM_NUM, HAL_IR_GENERAL_GLHRM);
    HAL_IR_WRITE_REG(ir_dev->membase, HAL_IR_REG_SEPR_BIT_FIFO_CTRL, HAL_IR_GENERAL_FIFO_CTRL);

    if (ir_dev->decode_mode == HAL_IR_RAW_MODE)
    {
        HAL_IR_WRITE_REG(ir_dev->membase, HAL_IR_REG_CTRL, HAL_IR_RAW_CTRL);
        HAL_IR_WRITE_REG_MASK(ir_dev->membase, HAL_IR_REG_GLHRM_NUM, HAL_IR_RAW_GLHRM, HAL_IR_RAW_GLHRM);
        HAL_IR_WRITE_REG_MASK(ir_dev->membase, HAL_IR_REG_FIFO_RD_PULSE, HAL_IR_RAW_WAKE_UP, HAL_IR_RAW_WAKE_UP);
    }
    else if (ir_dev->decode_mode == HAL_IR_FULL_MODE)
    {
        HAL_IR_WRITE_REG_MASK(ir_dev->membase, HAL_IR_REG_GLHRM_NUM, HAL_IR_FULL_GLHRM, HAL_IR_FULL_GLHRM);
        HAL_IR_WRITE_REG_MASK(ir_dev->membase, HAL_IR_REG_FIFO_RD_PULSE, HAL_IR_FULL_WAKE_UP, HAL_IR_FULL_WAKE_UP);
    }
    else if (ir_dev->decode_mode == HAL_IR_RC5_MODE)
    {
        HAL_IR_WRITE_REG(ir_dev->membase, HAL_IR_REG_RC_CFG, HAL_IR_RC5_EXT_DECODE);
        HAL_IR_WRITE_REG_MASK(ir_dev->membase, HAL_IR_REG_RC_WAKE_UP, HAL_IR_RC5_WAKE_UP, HAL_IR_RC5_WAKE_UP);
    }
    else if (ir_dev->decode_mode == HAL_IR_SW_MODE)
    {
        HAL_IR_WRITE_REG_MASK(ir_dev->membase, HAL_IR_REG_GLHRM_NUM, HAL_IR_SW_GLHRM, HAL_IR_SW_GLHRM);
        HAL_IR_WRITE_REG_MASK(ir_dev->membase, HAL_IR_REG_SEPR_BIT_FIFO_CTRL, HAL_IR_SW_EDGE_ALL, HAL_IR_SW_EDGE_ALL);
        hal_ir_set_software(ir_dev, 1);
        HAL_IR_WRITE_REG(ir_dev->membase, HAL_IR_REG_CKDIV_NUM_KEY_DATA, HAL_IR_SW_CLKDIV);
    }
    else
    {
        HAL_IR_WRITE_REG_MASK(ir_dev->membase, HAL_IR_REG_GLHRM_NUM, HAL_IR_SW_GLHRM, HAL_IR_SW_GLHRM);
        if (ir_dev->decode_mode == HAL_IR_SPEC_MODE)
        {
            reg_value = HAL_IR_READ_REG(ir_dev->membase, HAL_IR_REG_SEPR_BIT_FIFO_CTRL) | HAL_IR_SW_EDGE_PSHOT;
            HAL_IR_WRITE_REG(ir_dev->membase, HAL_IR_REG_SEPR_BIT_FIFO_CTRL, reg_value);
        }
        else
        {
#ifdef IR_INT_NP_EDGE_TRIG
            reg_value = HAL_IR_READ_REG(ir_dev->membase, HAL_IR_REG_SEPR_BIT_FIFO_CTRL) | HAL_IR_SW_EDGE_ALL;
            HAL_IR_WRITE_REG(ir_dev->membase, HAL_IR_REG_SEPR_BIT_FIFO_CTRL, reg_value);
#else
            HAL_IR_WRITE_REG(ir_dev->membase, HAL_IR_REG_SEPR_BIT_FIFO_CTRL,
                             HAL_IR_SW_EDGE_NSHOT); //[10:8]: FIFO depth, [11]:Enable FIFO full
#endif
        }
    }

    if ((ir_dev->decode_mode == HAL_IR_RAW_MODE) || (ir_dev->decode_mode == HAL_IR_FULL_MODE)
        || (ir_dev->decode_mode == HAL_IR_SW_MODE))
    {
        hal_clear_fifo(ir_dev);
    }

    return 0;
}

void hal_ir_config(struct hal_ir_dev *ir_dev)
{
    hal_ir_init(ir_dev);
    switch (ir_dev->decode_mode)
    {
        case HAL_IR_FULL_MODE:
            ir_dbg("successed to change decode mode into FULL\n");
            break;
        case HAL_IR_RAW_MODE:
            ir_dbg("successed to change decode mode into RAW\n");
            break;
        case HAL_IR_SW_MODE:
            ir_dbg("successed to change decode mode into SW\n");
            break;
        case HAL_IR_RC5_MODE:
            ir_dbg("successed to change decode mode into RC5\n");
            break;
        default:
            ir_err("invalid decode mode\n");
    }
}

u16 hal_ir_get_status(struct hal_ir_dev *ir_dev)
{
    u16 ret = 1;

    switch (ir_dev->decode_mode)
    {
        case HAL_IR_FULL_MODE:
        case HAL_IR_RAW_MODE:
        case HAL_IR_SW_MODE:
            ret = HAL_IR_READ_REG(ir_dev->membase, HAL_IR_REG_SHOT_CNT_H_FIFO_STATUS) & HAL_IR_FIFO_EMPTY;
            break;
        case HAL_IR_RC5_MODE:
            ret = HAL_IR_READ_REG(ir_dev->membase, HAL_IR_REG_RC_FIFO_STATUS) & HAL_IR_RC5_FIFO_EMPTY;
            break;
        default:
            ir_err("invalid decode mode\n");
    }

    return ret;
}

u32 hal_ir_get_key(struct hal_ir_dev *ir_dev)
{
    ir_dev->decode_info->key   = 0;
    ir_dev->decode_info->addr  = 0;
    ir_dev->decode_info->flag  = 0;
    ir_dev->decode_info->valid = 0;

    if (ir_dev->decode_mode == HAL_IR_FULL_MODE)
    {
        ir_dbg("decoding by FULL decode mode\n");
        hal_get_key_fullmode(ir_dev, ir_dev->decode_info);
    }
    else if (ir_dev->decode_mode == HAL_IR_RAW_MODE)
    {
        ir_dbg("decoding by RAW decode mode\n");
        hal_get_key_rawmode(ir_dev, ir_dev->decode_info);
    }
    else if (ir_dev->decode_mode == HAL_IR_RC5_MODE)
    {
        ir_dbg("decoding by RC5 decode mode\n");
        hal_get_key_rc5mode(ir_dev, ir_dev->decode_info);
    }
    else if (ir_dev->decode_mode == HAL_IR_SW_MODE)
    {
        ir_dbg("decoding by SW decode mode\n");
        hal_get_key_swmode(ir_dev);
    }

    return (ir_dev->decode_info->valid) ? (ir_dev->decode_info->addr << 8 | ir_dev->decode_info->key)
                                        : (ir_dev->decode_info->valid);
}

void hal_ir_set_software(struct hal_ir_dev *ir_dev, int enable)
{
    u16 reg_value = 0;

    reg_value = HAL_IR_READ_REG(ir_dev->membase, HAL_IR_REG_SEPR_BIT_FIFO_CTRL);
    if (enable)
    {
        reg_value |= HAL_IR_SW_FIFO_EN;
        reg_value |= HAL_IR_SW_RECOV_SHOT;
    }
    else
    {
        reg_value &= ~HAL_IR_SW_FIFO_EN;
        reg_value &= ~HAL_IR_SW_RECOV_SHOT;
    }
    HAL_IR_WRITE_REG(ir_dev->membase, HAL_IR_REG_SEPR_BIT_FIFO_CTRL, reg_value);
}
