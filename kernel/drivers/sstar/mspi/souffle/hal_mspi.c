/*
 * hal_mspi.c- Sigmastar
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

#include <mspi_os.h>
#if (defined CONFIG_BDMA_SUPPORT) || (defined CONFIG_SSTAR_BDMA)
#include <hal_bdma.h>
#endif
#if (defined CONFIG_SSTAR_GPIO) || (defined CONFIG_GPIO_SUPPORT)
#include <drv_gpio.h>
#endif
#include <hal_mspi.h>
#include <hal_mspireg.h>

#define HAL_MSPI_USE_LOCK 0

#ifdef CONFIG_ARM64
#define HAL_MSPI_READ_WORD(_reg)        (*(volatile u16 *)(u64)(_reg))
#define HAL_MSPI_WRITE_WORD(_reg, _val) ((*(volatile u16 *)(u64)(_reg)) = (u16)(_val))
#define HAL_MSPI_WRITE_WORD_MASK(_reg, _val, _mask) \
    ((*(volatile u16 *)(u64)(_reg)) = ((*(volatile u16 *)(u64)(_reg)) & ~(_mask)) | ((u16)(_val) & (_mask)))
#else
#define HAL_MSPI_READ_WORD(_reg)        (*(volatile u16 *)(u32)(_reg))
#define HAL_MSPI_WRITE_WORD(_reg, _val) ((*(volatile u16 *)(u32)(_reg)) = (u16)(_val))
#define HAL_MSPI_WRITE_WORD_MASK(_reg, _val, _mask) \
    ((*(volatile u16 *)(u32)(_reg)) = ((*(volatile u16 *)(u32)(_reg)) & ~(_mask)) | ((u16)(_val) & (_mask)))
#endif

#define HAL_MSPI_READ(_reg_)                    HAL_MSPI_READ_WORD(hal->base + ((_reg_) << 2))
#define HAL_MSPI_WRITE(_reg_, _val_)            HAL_MSPI_WRITE_WORD(hal->base + ((_reg_) << 2), (_val_))
#define HAL_MSPI_WRITE_MASK(_reg_, _val_, mask) HAL_MSPI_WRITE_WORD_MASK(hal->base + ((_reg_) << 2), (_val_), (mask))

#define HAL_MSPI_READ_INDEX  0x0
#define HAL_MSPI_WRITE_INDEX 0x1

#define HAL_MSPI_TIMEOUT_CNT 30000

#define HAL_MSPI_DMA_MODE_MAX 3

typedef enum
{
    HAL_MSPI_OFF = 0,
    HAL_MSPI_ON,
} hal_mspi_cs_state;

typedef enum
{
    HAL_MSPI_CS0 = 0,
    HAL_MSPI_CS1,
    HAL_MSPI_CS2,
} hal_mspi_cs_ch;

typedef struct
{
    u8 tr_start; // time from trigger to first SPI clock
    u8 tr_end;   // time from last SPI clock to transferred done
    u8 tb;       // time between byte to byte transfer
    u8 trw;      // time between last write and first read
} hal_mspi_dc;

typedef enum
{
    HAL_MSPI_TRSTART,
    HAL_MSPI_TREND,
    HAL_MSPI_TB,
    HAL_MSPI_TRW
} hal_mspi_dc_type;

typedef enum
{
    HAL_MSPI_POL,
    HAL_MSPI_PHA,
    HAL_MSPI_CLK
} hal_mspi_clk_type;

typedef struct
{
    u8 write_bits[8]; // bits will be transferred in write buffer
    u8 read_bits[8];  // bits Will be transferred in read buffer
} hal_mspi_frame;

#if HAL_MSPI_USE_LOCK
static CamOsMutex_t hal_mspi_lock;
#define hal_mspi_lock()   CamOsMutexLock(&hal_mspi_lock);
#define hal_mspi_unlock() CamOsMutexUnlock(&hal_mspi_lock);
#else
#define hal_mspi_lock()
#define hal_mspi_unlock()
#endif

#if (defined CONFIG_BDMA_SUPPORT) || (defined CONFIG_SSTAR_BDMA)
static int hal_mspi_get_bdma_ch(u8 channel)
{
    switch (channel)
    {
        case 0:
            return HAL_BDMA1_CH0;
        case 1:
            return HAL_BDMA1_CH0;
        case 2:
            return HAL_BDMA1_CH0;
        case 3:
            return HAL_BDMA1_CH0;
        default:
            return -HAL_MSPI_BDMA_CH_INVAILD;
    }
}
#define HAL_MSPI_BDMA_SEL_WRITE (0x0)
#define HAL_MSPI_BDMA_SEL_READ  (0x1)
static int hal_mspi_get_bdma_path(u8 ch, u8 mode)
{
    // mode 0:write;  1:read
    if (mode)
    {
        switch (ch)
        {
            case 0:
                return HAL_BDMA_MSPI0_TO_MIU;
            case 1:
                return HAL_BDMA_MSPI1_TO_MIU;
            case 2:
                return HAL_BDMA_MSPI2_TO_MIU;
            case 3:
                return HAL_BDMA_MSPI3_TO_MIU;
            default:
                return -HAL_MSPI_BDMA_PATH_INVAILD;
        }
    }
    else
    {
        switch (ch)
        {
            case 0:
                return HAL_BDMA_MIU_TO_MSPI0;
            case 1:
                return HAL_BDMA_MIU_TO_MSPI1;
            case 2:
                return HAL_BDMA_MIU_TO_MSPI2;
            case 3:
                return HAL_BDMA_MIU_TO_MSPI3;
            default:
                return -HAL_MSPI_BDMA_PATH_INVAILD;
        }
    }

    return -HAL_MSPI_BDMA_CH_INVAILD;
}
#endif

static void hal_mspi_enable(struct hal_mspi *hal, u8 enable)
{
    hal_mspi_lock();
    if (enable)
    {
        HAL_MSPI_WRITE(HAL_MSPI_CTRL, HAL_MSPI_READ(HAL_MSPI_CTRL) | HAL_MSPI_REG_INT_ENABLE);
    }
    else
    {
        HAL_MSPI_WRITE(HAL_MSPI_CTRL, HAL_MSPI_READ(HAL_MSPI_CTRL) & (~HAL_MSPI_REG_INT_ENABLE));
    }
    hal_mspi_unlock();
}

static void hal_mspi_init(struct hal_mspi *hal)
{
    hal_mspi_lock();
    HAL_MSPI_WRITE(HAL_MSPI_CTRL, HAL_MSPI_READ(HAL_MSPI_CTRL) | (HAL_MSPI_REG_RESET | HAL_MSPI_REG_ENABLE));

    if (hal->clk_out_mode)
    {
        HAL_MSPI_WRITE_MASK(HAL_MSPI_REG_CLK_NOT_GATED, HAL_MSPI_REG_CLK_NOT_GATED_MASK,
                            HAL_MSPI_REG_CLK_NOT_GATED_MASK);
    }

    hal_mspi_unlock();
    hal_mspi_enable(hal, 1);
}

static void hal_mspi_reset_dc_cfg(struct hal_mspi *hal)
{
    hal_mspi_lock();
    // DC reset
    HAL_MSPI_WRITE(HAL_MSPI_REG_DC_TR_START, 0x00);
    HAL_MSPI_WRITE(HAL_MSPI_REG_DC_TB, 0x00);
    hal_mspi_unlock();
}

static u8 hal_mspi_dc_cfg_max(hal_mspi_dc_type type)
{
    switch (type)
    {
        case HAL_MSPI_TRSTART:
            return HAL_MSPI_REG_DC_TRSTART_MAX;
        case HAL_MSPI_TREND:
            return HAL_MSPI_REG_DC_TREND_MAX;
        case HAL_MSPI_TB:
            return HAL_MSPI_REG_DC_TB_MAX;
        case HAL_MSPI_TRW:
            return HAL_MSPI_REG_DC_TRW_MAX;
        default:
            return 0;
    }
}

static void hal_mspi_set_dc_timing(struct hal_mspi *hal, hal_mspi_dc_type type, u8 timing)
{
    u16 value = 0;
    hal_mspi_lock();
    switch (type)
    {
        case HAL_MSPI_TRSTART:
            value = HAL_MSPI_READ(HAL_MSPI_REG_DC_TR_START);
            value &= (~HAL_MSPI_REG_DC_MASK);
            value |= timing;
            HAL_MSPI_WRITE(HAL_MSPI_REG_DC_TR_START, value);
            break;
        case HAL_MSPI_TREND:
            value = HAL_MSPI_READ(HAL_MSPI_REG_DC_TR_END);
            value &= HAL_MSPI_REG_DC_MASK;
            value |= timing << HAL_MSPI_REG_DC_BIT;
            HAL_MSPI_WRITE(HAL_MSPI_REG_DC_TR_END, value);
            break;
        case HAL_MSPI_TB:
            value = HAL_MSPI_READ(HAL_MSPI_REG_DC_TB);
            value &= (~HAL_MSPI_REG_DC_MASK);
            value |= timing;
            HAL_MSPI_WRITE(HAL_MSPI_REG_DC_TB, value);
            break;
        case HAL_MSPI_TRW:
            value = HAL_MSPI_READ(HAL_MSPI_REG_DC_TRW);
            value &= HAL_MSPI_REG_DC_MASK;
            value |= timing << HAL_MSPI_REG_DC_BIT;
            HAL_MSPI_WRITE(HAL_MSPI_REG_DC_TRW, value);
            break;
    }

    hal_mspi_unlock();
}

static int hal_dc_set_tr_start(struct hal_mspi *hal, u8 tr_start)
{
    u8  tr_start_max;
    int ret = HAL_MSPI_OK;

    tr_start_max = hal_mspi_dc_cfg_max(HAL_MSPI_TRSTART);
    if (tr_start > tr_start_max)
        ret = -HAL_MSPI_PARAM_OVERFLOW;
    else
        hal_mspi_set_dc_timing(hal, HAL_MSPI_TRSTART, tr_start);
    return ret;
}

static int hal_dc_set_tr_end(struct hal_mspi *hal, u8 tr_end)
{
    u8  tr_end_max;
    int ret = HAL_MSPI_OK;

    tr_end_max = hal_mspi_dc_cfg_max(HAL_MSPI_TREND);
    if (tr_end > tr_end_max)
        ret = -HAL_MSPI_PARAM_OVERFLOW;
    else
        hal_mspi_set_dc_timing(hal, HAL_MSPI_TREND, tr_end);
    return ret;
}

static int hal_dc_set_tb(struct hal_mspi *hal, u8 tb)
{
    u8  tb_max;
    int ret = HAL_MSPI_OK;

    tb_max = hal_mspi_dc_cfg_max(HAL_MSPI_TB);
    if (tb > tb_max)
        ret = -HAL_MSPI_PARAM_OVERFLOW;
    else
        hal_mspi_set_dc_timing(hal, HAL_MSPI_TB, tb);
    return ret;
}

static int hal_dc_set_trw(struct hal_mspi *hal, u8 trw)
{
    u8  trw_max;
    int ret = HAL_MSPI_OK;

    trw_max = hal_mspi_dc_cfg_max(HAL_MSPI_TRW);
    if (trw > trw_max)
        ret = -HAL_MSPI_PARAM_OVERFLOW;
    else
        hal_mspi_set_dc_timing(hal, HAL_MSPI_TRW, trw);
    return ret;
}

static int hal_mspi_dc_cfg(struct hal_mspi *hal, hal_mspi_dc *cfg)
{
    int ret = HAL_MSPI_OK;

    if (cfg == NULL)
    {
        hal_mspi_reset_dc_cfg(hal);
        return HAL_MSPI_OK;
    }
    ret = hal_dc_set_tr_start(hal, cfg->tr_start);
    if (ret != HAL_MSPI_OK)
        goto err_exit;
    ret = hal_dc_set_tr_end(hal, cfg->tr_end);
    if (ret != HAL_MSPI_OK)
        goto err_exit;
    ret = hal_dc_set_tb(hal, cfg->tb);
    if (ret != HAL_MSPI_OK)
        goto err_exit;
    ret = hal_dc_set_trw(hal, cfg->trw);
    if (ret != HAL_MSPI_OK)
        goto err_exit;
    return HAL_MSPI_OK;

err_exit:
    ret = -HAL_MSPI_DCCONFIG_ERROR;
    return ret;
}

static void hal_mspi_set_clk_timing(struct hal_mspi *hal, hal_mspi_clk_type type, u8 value)
{
    u16 reg_value = 0;
    hal_mspi_lock();
    switch (type)
    {
        case HAL_MSPI_POL:
            reg_value = HAL_MSPI_READ(HAL_MSPI_REG_CLK_CLOCK);
            reg_value &= ~(HAL_MSPI_REG_CLK_POLARITY_MASK);
            reg_value |= value << HAL_MSPI_REG_CLK_POLARITY_BIT;
            break;
        case HAL_MSPI_PHA:
            reg_value = HAL_MSPI_READ(HAL_MSPI_REG_CLK_CLOCK);
            reg_value &= ~(HAL_MSPI_REG_CLK_PHASE_MASK);
            reg_value |= value << HAL_MSPI_REG_CLK_PHASE_BIT;
            break;
        case HAL_MSPI_CLK:
            reg_value = HAL_MSPI_READ(HAL_MSPI_REG_CLK_CLOCK);
            reg_value &= HAL_MSPI_REG_CLK_CLOCK_MASK;
            reg_value |= value << HAL_MSPI_REG_CLK_CLOCK_BIT;
            break;
    }
    HAL_MSPI_WRITE(HAL_MSPI_REG_CLK_CLOCK, reg_value);
    hal_mspi_unlock();
}

static void hal_mspi_reset_clk_cfg(struct hal_mspi *hal)
{
    u16 value;
    hal_mspi_lock();
    // reset clock
    value = HAL_MSPI_READ(HAL_MSPI_CTRL);
    value &= 0x3F;
    HAL_MSPI_WRITE(HAL_MSPI_CTRL, value);
    hal_mspi_unlock();
}

static void hal_mspi_reset_frame_cfg(struct hal_mspi *hal)
{
    hal_mspi_lock();
    // Frame reset
    HAL_MSPI_WRITE(HAL_MSPI_REG_FRAME_WBIT, 0xFFF);
    HAL_MSPI_WRITE(HAL_MSPI_REG_FRAME_WBIT + 1, 0xFFF);
    HAL_MSPI_WRITE(HAL_MSPI_REG_FRAME_RBIT, 0xFFF);
    HAL_MSPI_WRITE(HAL_MSPI_REG_FRAME_RBIT + 1, 0xFFF);
    hal_mspi_unlock();
}

static void hal_mspi_set_per_frame_size(struct hal_mspi *hal, u8 direct, u8 offset, u8 size)
{
    u8  index      = 0;
    u16 value      = 0;
    u8  bit_offset = 0;
    u16 reg_index  = 0;

    hal_mspi_lock();
    if (direct == HAL_MSPI_READ_INDEX)
    {
        reg_index = HAL_MSPI_REG_FRAME_RBIT;
    }
    else
    {
        reg_index = HAL_MSPI_REG_FRAME_WBIT;
    }
    if (offset >= 4)
    {
        index++;
        offset -= 4;
    }
    bit_offset = offset * HAL_MSPI_REG_FRAME_BIT_FIELD;
    value      = HAL_MSPI_READ(reg_index + index);
    value &= ~(HAL_MSPI_REG_FRAME_BIT_MASK << bit_offset);
    value |= size << bit_offset;
    HAL_MSPI_WRITE((reg_index + index), value);
    hal_mspi_unlock();
}

static int hal_mspi_frame_cfg(struct hal_mspi *mspi, hal_mspi_frame *frame)
{
    int ret   = HAL_MSPI_OK;
    u8  index = 0;

    if (frame == NULL)
    {
        hal_mspi_reset_frame_cfg(mspi);
        return HAL_MSPI_OK;
    }
    // read buffer bit config
    for (index = 0; index < HAL_MSPI_REG_READ_BUF_MAX; index++)
    {
        if (frame->read_bits[index] > HAL_MSPI_REG_FRAME_BIT_MAX)
        {
            ret = -HAL_MSPI_PARAM_OVERFLOW;
        }
        else
        {
            hal_mspi_set_per_frame_size(mspi, HAL_MSPI_READ_INDEX, index, frame->read_bits[index]);
        }
    }
    // write buffer bit config
    for (index = 0; index < HAL_MSPI_REG_WRITE_BUF_MAX; index++)
    {
        if (frame->write_bits[index] > HAL_MSPI_REG_FRAME_BIT_MAX)
        {
            ret = -HAL_MSPI_PARAM_OVERFLOW;
        }
        else
        {
            hal_mspi_set_per_frame_size(mspi, HAL_MSPI_WRITE_INDEX, index, frame->write_bits[index]);
        }
    }
    return ret;
}

static void hal_mspi_rw_buf_size(struct hal_mspi *hal, u8 direct, u8 size)
{
    u16 data = HAL_MSPI_READ(HAL_MSPI_REG_RBF_SIZE);

    if (direct == HAL_MSPI_READ_INDEX)
    {
        data &= HAL_MSPI_REG_RWSIZE_MASK;
        data |= size << HAL_MSPI_REG_RSIZE_BIT;
    }
    else
    {
        data &= ~HAL_MSPI_REG_RWSIZE_MASK;
        data |= size;
    }
    HAL_MSPI_WRITE(HAL_MSPI_REG_RBF_SIZE, data);
}

static int hal_mspi_transfer(struct hal_mspi *hal)
{
    int ret     = 0;
    u32 timeout = HAL_MSPI_TIMEOUT_CNT;
    // trigger operation

    if (hal->wait_done)
    {
        ret = hal->wait_done(hal);
    }
    else
    {
        while (!hal_mspi_check_done(hal) && --timeout)
        {
            hal_mspi_delay_us(10);
        }

        if (0 == timeout)
        {
            hal_mspi_err("wait done polling timeout\n");
            return -HAL_MSPI_TIMEOUT;
        }
        else
        {
            hal_mspi_clear_done(hal);
        }
    }

    HAL_MSPI_WRITE(HAL_MSPI_REG_RBF_SIZE, 0x0);

    return ret;
}

static int hal_mspi_full_duplex_buf(struct hal_mspi *hal, u8 *rx_buff, u8 *tx_buff, u16 size)
{
    u8  index    = 0;
    u16 value    = 0;
    int ret      = HAL_MSPI_OK;
    u8  shift    = 0;
    u8  msb_mode = !(HAL_MSPI_READ(HAL_MSPI_REG_LSB_FIRST) & BIT0);

    hal_mspi_lock();

    for (index = 0; index < size; index++)
    {
        if (index & 1)
        {
            if (hal->bits_per_word <= 8)
            {
                shift = (msb_mode) ? (8 - hal->bits_per_word) : 0;
                value = (tx_buff[index] << (shift + 8)) | (tx_buff[index - 1] << shift);
            }
            else
            {
                shift = 16 - hal->bits_per_word;
                if (msb_mode)
                {
                    value = (tx_buff[index] << shift) | (tx_buff[index - 1] << 8);
                }
                else
                {
                    // NO LSB
                }
            }
            HAL_MSPI_WRITE((HAL_MSPI_REG_WRITE_BUF + (index >> 1)), value);
        }
        else if (index == (size - 1))
        {
            shift = (msb_mode) ? (8 - hal->bits_per_word) : 0;
            HAL_MSPI_WRITE((HAL_MSPI_REG_WRITE_BUF + (index >> 1)), tx_buff[index] << shift);
        }
    }

    hal_mspi_rw_buf_size(hal, HAL_MSPI_WRITE_INDEX, size);
    ret = hal_mspi_transfer(hal);
    if (ret)
    {
        goto err_out;
    }

    for (index = 0; index < size; index++)
    {
        if (index & 1)
        {
            value = HAL_MSPI_READ((HAL_MSPI_REG_FULL_DEPLUX + (index >> 1)));
            if (msb_mode)
            {
                if (hal->bits_per_word <= 8)
                {
                    shift              = hal->bits_per_word;
                    rx_buff[index]     = (value >> 8) & ((0x1 << shift) - 0x1);
                    rx_buff[index - 1] = value & ((0x1 << shift) - 0x1);
                }
                else
                {
                    shift              = hal->bits_per_word - 8;
                    rx_buff[index]     = value & ((0x1 << shift) - 0x1);
                    rx_buff[index - 1] = value >> 8;
                }
            }
            else
            {
                // NO LSB
            }
        }
        else if (index == (size - 1))
        {
            value          = HAL_MSPI_READ((HAL_MSPI_REG_FULL_DEPLUX + (index >> 1)));
            shift          = hal->bits_per_word;
            rx_buff[index] = value & ((0x1 << shift) - 0x1);
        }
    }

err_out:
    hal_mspi_unlock();
    return ret;
}

static int hal_mspi_read_buf(struct hal_mspi *hal, u8 *data, u16 size)
{
    u8  index = 0;
    u16 value = 0;
    u16 i = 0, j = 0;
    int ret = HAL_MSPI_OK;
    u8  shift;
    u8  msb_mode = !(HAL_MSPI_READ(HAL_MSPI_REG_LSB_FIRST) & BIT0);

    hal_mspi_lock();
    for (i = 0; i < size; i += HAL_MSPI_REG_READ_BUF_MAX)
    {
        value = size - i;
        if (value > HAL_MSPI_REG_READ_BUF_MAX)
        {
            j = HAL_MSPI_REG_READ_BUF_MAX;
        }
        else
        {
            j = value;
        }
        hal_mspi_rw_buf_size(hal, HAL_MSPI_READ_INDEX, j);

        ret = hal_mspi_transfer(hal);
        if (ret)
        {
            goto err_out;
        }

        for (index = 0; index < j; index++)
        {
            if (index & 1)
            {
                value = HAL_MSPI_READ((HAL_MSPI_REG_READ_BUF + (index >> 1)));
                if (msb_mode)
                {
                    // MSB MODE
                    if (hal->bits_per_word <= 8)
                    {
                        // printk("value    0x%x\n",value);
                        shift           = hal->bits_per_word;
                        data[index]     = (value >> 8) & ((0x1 << shift) - 0x1);
                        data[index - 1] = value & ((0x1 << shift) - 0x1);
                    }
                    else // bits_per_word=9~15
                    {
                        shift           = hal->bits_per_word - 8;
                        data[index]     = value & ((0x1 << shift) - 0x1);
                        data[index - 1] = value >> 8;
                    }
                }
                else
                {
                    // NO LSB
                }
            }
            else if (index == (j - 1))
            {
                value       = HAL_MSPI_READ((HAL_MSPI_REG_READ_BUF + (index >> 1)));
                shift       = hal->bits_per_word;
                data[index] = value & ((0x1 << shift) - 0x1);
            }
        }
        data += j;
    }

err_out:
    hal_mspi_unlock();
    return ret;
}

static int hal_mspi_write_buf(struct hal_mspi *hal, u8 *data, u16 size)
{
    u8  index    = 0;
    u16 value    = 0;
    int ret      = HAL_MSPI_OK;
    u8  shift    = 0;
    u8  msb_mode = !(HAL_MSPI_READ(HAL_MSPI_REG_LSB_FIRST) & BIT0);

    hal_mspi_lock();

    for (index = 0; index < size; index++)
    {
        if (index & 1)
        {
            if (hal->bits_per_word <= 8)
            {
                shift = (msb_mode) ? (8 - hal->bits_per_word) : 0;
                value = (data[index] << (shift + 8)) | (data[index - 1] << shift);
            }
            else
            {
                shift = 16 - hal->bits_per_word;
                if (msb_mode)
                {
                    value = (data[index] << shift) | (data[index - 1] << 8);
                }
                else
                {
                    // NO LSB
                }
            }
            HAL_MSPI_WRITE((HAL_MSPI_REG_WRITE_BUF + (index >> 1)), value);
        }
        else if (index == (size - 1))
        {
            shift = 8 - hal->bits_per_word;
            HAL_MSPI_WRITE((HAL_MSPI_REG_WRITE_BUF + (index >> 1)), data[index] << shift);
        }
    }

    hal_mspi_rw_buf_size(hal, HAL_MSPI_WRITE_INDEX, size);
    ret = hal_mspi_transfer(hal);
    if (ret)
    {
        goto err_out;
    }
    // set write data size
err_out:
    hal_mspi_unlock();
    return ret;
}

int hal_mspi_trigger(struct hal_mspi *hal)
{
    HAL_MSPI_WRITE(HAL_MSPI_REG_TRIGGER, HAL_MSPI_REG_TRIG);

    return HAL_MSPI_OK;
}

int hal_mspi_set_mode(struct hal_mspi *hal, hal_mspi_mode mode)
{
    if (mode >= HAL_MSPI_MODE_MAX)
    {
        return -HAL_MSPI_PARAM_OVERFLOW;
    }
    switch (mode)
    {
        case HAL_MSPI_MODE0:
            hal_mspi_set_clk_timing(hal, HAL_MSPI_POL, false);
            hal_mspi_set_clk_timing(hal, HAL_MSPI_PHA, false);

            break;
        case HAL_MSPI_MODE1:
            hal_mspi_set_clk_timing(hal, HAL_MSPI_POL, false);
            hal_mspi_set_clk_timing(hal, HAL_MSPI_PHA, true);
            break;
        case HAL_MSPI_MODE2:
            hal_mspi_set_clk_timing(hal, HAL_MSPI_POL, true);
            hal_mspi_set_clk_timing(hal, HAL_MSPI_PHA, false);
            break;
        case HAL_MSPI_MODE3:
            hal_mspi_set_clk_timing(hal, HAL_MSPI_POL, true);
            hal_mspi_set_clk_timing(hal, HAL_MSPI_PHA, true);
            break;
        default:
            hal_mspi_reset_clk_cfg(hal);
            return -HAL_MSPI_OPERATION_ERROR;
    }

    return HAL_MSPI_OK;
}

void hal_mspi_chip_select(struct hal_mspi *hal, u8 enable, u8 select)
{
    u16 reg_data = 0;
    u8  bit_mask = 0;
    hal_mspi_lock();

    if (select < hal->cs_num)
    {
        reg_data = HAL_MSPI_READ(HAL_MSPI_REG_CHIP_CS);
        if (enable)
        {
            bit_mask = ~(1 << select);
            reg_data &= bit_mask;
        }
        else
        {
            bit_mask = (1 << select);
            reg_data |= bit_mask;
        }
        HAL_MSPI_WRITE(HAL_MSPI_REG_CHIP_CS, reg_data);
    }
    else
    {
#if (defined CONFIG_SSTAR_GPIO) || (defined CONFIG_GPIO_SUPPORT)
        if (enable)
            sstar_gpio_set_low(hal->cs_ext[select - hal->cs_num]);
        else
            sstar_gpio_set_high(hal->cs_ext[select - hal->cs_num]);
#else
        hal_mspi_err("not support hal cs-ext,because GPIO API do not exit\n");
#endif
    }
    hal_mspi_unlock();
}

int hal_mspi_set_lsb(struct hal_mspi *hal, u8 enable)
{
    hal_mspi_lock();

    HAL_MSPI_WRITE(HAL_MSPI_REG_LSB_FIRST, enable);

    hal_mspi_unlock();
    return HAL_MSPI_OK;
}

int hal_mspi_set_3wire_mode(struct hal_mspi *hal, u8 enable)
{
    hal_mspi_lock();

    HAL_MSPI_WRITE_MASK(HAL_MSPI_CTRL, enable, HAL_MSPI_REG_3WIREMODE_MASK);

    hal_mspi_unlock();
    return HAL_MSPI_OK;
}

int hal_mspi_config(struct hal_mspi *hal)
{
    int ret = HAL_MSPI_OK;

    hal_mspi_dc    dc_cfg;
    hal_mspi_frame frame_cfg;
    hal_mspi_mode  spi_mode = HAL_MSPI_MODE0;

    dc_cfg.tb       = 0;
    dc_cfg.tr_end   = 0x0;
    dc_cfg.tr_start = 0x0;
    dc_cfg.trw      = 0;

#if HAL_MSPI_USE_LOCK
    CamOsMutexInit(&hal_mspi_lock);
#endif

    memset(&frame_cfg, 0x07, sizeof(hal_mspi_frame));
    hal_mspi_init(hal);

    ret = hal_mspi_dc_cfg(hal, &dc_cfg);
    if (ret)
    {
        return ret;
    }

    ret = hal_mspi_set_mode(hal, spi_mode);
    if (ret)
    {
        return ret;
    }

    ret = hal_mspi_frame_cfg(hal, &frame_cfg);
    if (ret)
    {
        return ret;
    }

    hal_mspi_chip_select(hal, HAL_MSPI_OFF, HAL_MSPI_CS0);
    ret = hal_mspi_set_lsb(hal, 0);
    return ret;
}

int hal_mspi_full_duplex(u8 ch, struct hal_mspi *hal, u8 *rx_buff, u8 *tx_buff, u16 size)
{
    u16 index            = 0;
    u16 frame_count      = 0;
    u16 last_frame_count = 0;
    int ret              = HAL_MSPI_OK;

    if (rx_buff == NULL || tx_buff == NULL)
    {
        return -HAL_MSPI_NULL;
    }

    frame_count      = size / HAL_MSPI_REG_WRITE_BUF_MAX; // Cut data to frame by max frame size
    last_frame_count = size % HAL_MSPI_REG_WRITE_BUF_MAX; // Last data less than a HAL_MSPI_REG_WRITE_BUF_MAX fame

    for (index = 0; index < frame_count; index++)
    {
        ret = hal_mspi_full_duplex_buf(hal, rx_buff + index * HAL_MSPI_REG_READ_BUF_MAX,
                                       tx_buff + index * HAL_MSPI_REG_WRITE_BUF_MAX, HAL_MSPI_REG_WRITE_BUF_MAX);
        if (ret)
        {
            return -HAL_MSPI_OPERATION_ERROR;
        }
    }

    if (last_frame_count)
    {
        ret = hal_mspi_full_duplex_buf(hal, rx_buff + frame_count * HAL_MSPI_REG_READ_BUF_MAX,
                                       tx_buff + frame_count * HAL_MSPI_REG_WRITE_BUF_MAX, last_frame_count);
    }

    if (ret)
    {
        return -HAL_MSPI_OPERATION_ERROR;
    }
    return HAL_MSPI_OK;
}

int hal_mspi_write(u8 ch, struct hal_mspi *hal, u8 *data, u16 size)
{
    u16 index            = 0;
    u16 frame_count      = 0;
    u16 last_frame_count = 0;
    int ret              = HAL_MSPI_OK;

    if (data == NULL)
    {
        return -HAL_MSPI_NULL;
    }

    frame_count      = size / HAL_MSPI_REG_WRITE_BUF_MAX; // Cut data to frame by max frame size
    last_frame_count = size % HAL_MSPI_REG_WRITE_BUF_MAX; // Last data less than a HAL_MSPI_REG_WRITE_BUF_MAX fame

    for (index = 0; index < frame_count; index++)
    {
        ret = hal_mspi_write_buf(hal, data + index * HAL_MSPI_REG_WRITE_BUF_MAX, HAL_MSPI_REG_WRITE_BUF_MAX);
        if (ret)
        {
            return -HAL_MSPI_OPERATION_ERROR;
        }
    }

    if (last_frame_count)
    {
        ret = hal_mspi_write_buf(hal, data + frame_count * HAL_MSPI_REG_WRITE_BUF_MAX, last_frame_count);
    }

    if (ret)
    {
        return -HAL_MSPI_OPERATION_ERROR;
    }
    return HAL_MSPI_OK;
}

int hal_mspi_read(u8 ch, struct hal_mspi *hal, u8 *data, u16 size)
{
    u16 index            = 0;
    u16 frame_count      = 0;
    u16 last_frame_count = 0;
    u8  ret              = HAL_MSPI_OK;

    if (data == NULL)
    {
        return -HAL_MSPI_NULL;
    }

    frame_count      = size / HAL_MSPI_REG_WRITE_BUF_MAX; // Cut data to frame by max frame size
    last_frame_count = size % HAL_MSPI_REG_WRITE_BUF_MAX; // Last data less than a HAL_MSPI_REG_WRITE_BUF_MAX fame
    for (index = 0; index < frame_count; index++)
    {
        ret = hal_mspi_read_buf(hal, data + index * HAL_MSPI_REG_WRITE_BUF_MAX, HAL_MSPI_REG_WRITE_BUF_MAX);
        if (ret)
        {
            return -HAL_MSPI_OPERATION_ERROR;
        }
    }
    if (last_frame_count)
    {
        ret = hal_mspi_read_buf(hal, data + frame_count * HAL_MSPI_REG_WRITE_BUF_MAX, last_frame_count);
    }
    if (ret)
    {
        return -HAL_MSPI_OPERATION_ERROR;
    }
    return HAL_MSPI_OK;
}

#if (defined CONFIG_BDMA_SUPPORT) || (defined CONFIG_SSTAR_BDMA)
int hal_mspi_dma_write(u8 ch, struct hal_mspi *hal, u8 *data, u32 size)
{
    int            ret       = 0;
    s8             dma_ch    = 0;
    u64            phys_addr = 0;
    u64            data_addr = 0;
    hal_bdma_param bdma_para = {0};
    if (data == NULL)
    {
        return -HAL_MSPI_NULL;
    }
    phys_addr = hal_mspi_virt_to_phys(data);
    data_addr = hal_mspi_phys_to_miu(phys_addr);
    dma_ch    = hal_mspi_get_bdma_ch(ch);
    if (dma_ch == -1)
    {
        hal_mspi_err("bus-%d WRONG DMA CHANNEL\n", ch);
        return -HAL_MSPI_ERR;
    }
    memset(&bdma_para, 0, sizeof(hal_bdma_param));
    bdma_para.bIntMode     = 1; // 0:use polling mode
    bdma_para.ePathSel     = hal_mspi_get_bdma_path(ch, HAL_MSPI_BDMA_SEL_WRITE);
    bdma_para.eDstAddrMode = HAL_BDMA_ADDR_INC; // address increase
    bdma_para.u32TxCount   = size;
    bdma_para.u32Pattern   = 0;
    bdma_para.pSrcAddr     = (phys_addr_t)data_addr;
    bdma_para.pfTxCbFunc   = NULL;

    hal_mspi_cache_flush(data, size);
    hal_mspi_lock();

    HAL_MSPI_WRITE(HAL_MSPI_REG_DMA_ENABLE, 0x01);
    HAL_MSPI_WRITE(HAL_MSPI_REG_DMA_RW_MODE, HAL_MSPI_REG_DMA_WRITE);

    HAL_MSPI_WRITE(HAL_MSPI_REG_DMA_DATA_LEN_L, size & 0xFFFF);
    HAL_MSPI_WRITE(HAL_MSPI_REG_DMA_DATA_LEN_H, ((size >> 16) & 0xFFFF));

    hal_mspi_rw_buf_size(hal, HAL_MSPI_WRITE_INDEX, 0);
    hal_bdma_initialize((u8)dma_ch);
    if (HAL_BDMA_PROC_DONE != hal_bdma_transfer((u8)dma_ch, &bdma_para))
    {
        hal_mspi_err("bus-%d bdma set fail write\n", ch);
        return -HAL_MSPI_ERR;
    }
    ret = hal_mspi_transfer(hal);
    if (ret)
    {
        goto err_out;
    }

err_out:
    HAL_MSPI_WRITE(HAL_MSPI_REG_DMA_ENABLE, 0x00);
    hal_mspi_unlock();
    return ret;
}

int hal_mspi_dma_read(u8 ch, struct hal_mspi *hal, u8 *data, u32 size)
{
    u8             ret       = 0;
    s8             dma_ch    = 0;
    u64            phys_addr = 0;
    u64            data_addr = 0;
    hal_bdma_param bdma_para = {0};
    if (data == NULL)
    {
        return -HAL_MSPI_NULL;
    }

    phys_addr = hal_mspi_virt_to_phys(data);
    data_addr = hal_mspi_phys_to_miu(phys_addr);
    dma_ch    = hal_mspi_get_bdma_ch(ch);
    if (dma_ch == -1)
    {
        hal_mspi_err("bus-%d WRONG DMA CHANNEL\n", ch);
        return -HAL_MSPI_ERR;
    }
    memset(&bdma_para, 0, sizeof(hal_bdma_param));
    bdma_para.bIntMode     = 1;
    bdma_para.ePathSel     = hal_mspi_get_bdma_path(ch, HAL_MSPI_BDMA_SEL_READ);
    bdma_para.eDstAddrMode = HAL_BDMA_ADDR_INC;
    bdma_para.u32TxCount   = size;
    bdma_para.u32Pattern   = 0;
    bdma_para.pDstAddr     = (phys_addr_t)data_addr;
    bdma_para.pfTxCbFunc   = NULL;

    hal_mspi_lock();

    HAL_MSPI_WRITE(HAL_MSPI_REG_DMA_ENABLE, 0x01);
    HAL_MSPI_WRITE(HAL_MSPI_REG_DMA_RW_MODE, HAL_MSPI_REG_DMA_READ);

    HAL_MSPI_WRITE(HAL_MSPI_REG_DMA_DATA_LEN_L, size & 0xFFFF);
    HAL_MSPI_WRITE(HAL_MSPI_REG_DMA_DATA_LEN_H, (size >> 16) & 0x00FF); // total 24bit

    hal_mspi_rw_buf_size(hal, HAL_MSPI_READ_INDEX, 0); // spi length = 0
    hal_bdma_initialize((u8)dma_ch);
    if (HAL_BDMA_PROC_DONE != hal_bdma_transfer((u8)dma_ch, &bdma_para))
    {
        hal_mspi_err("bus-%d bdma set fail read\n", ch);
        return -HAL_MSPI_ERR;
    }

    hal_mspi_cache_invalidate(data, size);

    ret = hal_mspi_transfer(hal);
    if (ret)
    {
        goto err_out;
    }

    hal_mspi_cache_invalidate(data, size);

err_out:
    HAL_MSPI_WRITE(HAL_MSPI_REG_DMA_ENABLE, 0x00);
    hal_mspi_unlock();
    return ret;
}
#else
int hal_mspi_dma_write(u8 ch, struct hal_mspi *hal, u8 *data, u32 size)
{
    hal_mspi_err("hal-%d dma write not support\n", ch);
    return 0;
}

int hal_mspi_dma_read(u8 ch, struct hal_mspi *hal, u8 *data, u32 size)
{
    hal_mspi_err("hal-%d dma read not support\n", ch);
    return 0;
}

#endif
u16 hal_mspi_check_done(struct hal_mspi *hal)
{
    return HAL_MSPI_READ(HAL_MSPI_REG_DONE);
}

void hal_mspi_clear_done(struct hal_mspi *hal)
{
    HAL_MSPI_WRITE(HAL_MSPI_REG_DONE_CLEAR, HAL_MSPI_REG_CLEAR_DONE);
}

void hal_mspi_set_div_clk(struct hal_mspi *hal, u8 div)
{
    u16 reg_value = 0;

    reg_value = HAL_MSPI_READ(HAL_MSPI_REG_CLK_CLOCK);
    reg_value &= HAL_MSPI_REG_CLK_CLOCK_MASK;
    reg_value |= div << HAL_MSPI_REG_CLK_CLOCK_BIT;
    HAL_MSPI_WRITE(HAL_MSPI_REG_CLK_CLOCK, reg_value);
}

int hal_mspi_check_dma_mode(u8 ch)
{
    if (ch > HAL_MSPI_DMA_MODE_MAX)
    {
        return -HAL_MSPI_HW_NOT_SUPPORT;
    }
    else
    {
        return HAL_MSPI_OK;
    }
}

int hal_mspi_set_frame_cfg(struct hal_mspi *hal, int bits_per_word)
{
    int            i;
    hal_mspi_frame frame_cfg;

    if (bits_per_word > HAL_MSPI_MAX_SUPPORT_BITS)
    {
        return -HAL_MSPI_PARAM_OVERFLOW;
    }
    else if (bits_per_word > 8)
    {
        for (i = 0; i < HAL_MSPI_REG_WRITE_BUF_MAX; i += 2)
        {
            frame_cfg.write_bits[i]     = bits_per_word - 8 - 1;
            frame_cfg.write_bits[i + 1] = 8 - 1;
        }
        for (i = 0; i < HAL_MSPI_REG_READ_BUF_MAX; i += 2)
        {
            frame_cfg.read_bits[i]     = bits_per_word - 8 - 1;
            frame_cfg.read_bits[i + 1] = 8 - 1;
        }
    }
    else
    {
        for (i = 0; i < HAL_MSPI_REG_WRITE_BUF_MAX; i++)
        {
            frame_cfg.write_bits[i] = bits_per_word - 1;
        }
        for (i = 0; i < HAL_MSPI_REG_READ_BUF_MAX; i++)
        {
            frame_cfg.read_bits[i] = bits_per_word - 1;
        }
    }
    hal_mspi_frame_cfg(hal, &frame_cfg);

    return HAL_MSPI_OK;
}
