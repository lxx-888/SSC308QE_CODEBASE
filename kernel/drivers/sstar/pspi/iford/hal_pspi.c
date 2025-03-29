/*
 * hal_pspi.c- Sigmastar
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

#include <pspi_os.h>
#include <hal_pspi.h>
#include <hal_pspireg.h>

#define HAL_PSPI_USE_LOCK 0

#ifdef CONFIG_ARM64
#define HAL_PSPI_READ_BYTE(_reg_)         (*(volatile u8 *)(u64)(_reg_))
#define HAL_PSPI_WRITE_BYTE(_reg_, _val_) ((*(volatile u8 *)(u64)(_reg_)) = (u8)(_val_))
#define HAL_PSPI_WRITE_BYTE_MASK(_reg_, _val_, _mask) \
    ((*(volatile u8 *)(u64)(_reg_)) = ((*(volatile u8 *)(u64)(_reg_)) & ~(_mask)) | ((u8)(_val_) & (_mask)))
#define HAL_PSPI_READ_WORD(_reg)        (*(volatile u16 *)(u64)(_reg))
#define HAL_PSPI_WRITE_WORD(_reg, _val) ((*(volatile u16 *)(u64)(_reg)) = (u16)(_val))
#define HAL_PSPI_WRITE_WORD_MASK(_reg, _val, _mask) \
    ((*(volatile u16 *)(u64)(_reg)) = ((*(volatile u16 *)(u64)(_reg)) & ~(_mask)) | ((u16)(_val) & (_mask)))
#else
#define HAL_PSPI_READ_BYTE(_reg_)         (*(volatile u8 *)(u32)(_reg_))
#define HAL_PSPI_WRITE_BYTE(_reg_, _val_) ((*(volatile u8 *)(u32)(_reg_)) = (u8)(_val_))
#define HAL_PSPI_WRITE_BYTE_MASK(_reg_, _val_, _mask) \
    ((*(volatile u8 *)(u32)(_reg_)) = ((*(volatile u8 *)(u32)(_reg_)) & ~(_mask)) | ((u8)(_val_) & (_mask)))
#define HAL_PSPI_READ_WORD(_reg)        (*(volatile u16 *)(u32)(_reg))
#define HAL_PSPI_WRITE_WORD(_reg, _val) ((*(volatile u16 *)(u32)(_reg)) = (u16)(_val))
#define HAL_PSPI_WRITE_WORD_MASK(_reg, _val, _mask) \
    ((*(volatile u16 *)(u32)(_reg)) = ((*(volatile u16 *)(u32)(_reg)) & ~(_mask)) | ((u16)(_val) & (_mask)))
#endif

#define HAL_PSPI_U16_MASK                       (0xFF)
#define HAL_PSPI_ADDR0_CTRL(hal)                ((hal->pspi_base) + ((0x00) << 2))
#define HAL_PSPI_CLKDIV(hal)                    ((hal->pspi_base) + ((0x01) << 2))
#define HAL_PSPI_ADDR2_CTRL(hal)                ((hal->pspi_base) + ((0x02) << 2))
#define HAL_PSPI_ADDR3_CTRL(hal)                ((hal->pspi_base) + ((0x03) << 2))
#define HAL_PSPI_DCYCLE(hal)                    ((hal->pspi_base) + ((0x04) << 2))
#define HAL_PSPI_WCYCLE(hal)                    ((hal->pspi_base) + ((0x06) << 2))
#define HAL_PSPI_XCH(hal)                       ((hal->pspi_base) + ((0x08) << 2))
#define HAL_PSPI_TX_RX_FIFO_LEFT(hal)           ((hal->pspi_base) + ((0x0A) << 2))
#define HAL_PSPI_TX_RX_FIFO_THRESHOLD(hal)      ((hal->pspi_base) + ((0x0B) << 2))
#define HAL_PSPI_TX_FIFO_PORT_L(hal)            ((hal->pspi_base) + ((0x0C) << 2))
#define HAL_PSPI_TX_FIFO_PORT_H(hal)            ((hal->pspi_base) + ((0x0D) << 2))
#define HAL_PSPI_TX_FIFO_PORT_BYTE_1(hal)       ((hal->pspi_base) + ((0x0C) << 2))
#define HAL_PSPI_TX_FIFO_PORT_BYTE_2(hal)       ((hal->pspi_base) + ((0x0C) << 2) + 2)
#define HAL_PSPI_TX_FIFO_PORT_BYTE_3(hal)       ((hal->pspi_base) + ((0x0D) << 2))
#define HAL_PSPI_TX_FIFO_PORT_BYTE_4(hal)       ((hal->pspi_base) + ((0x0D) << 2) + 2)
#define HAL_PSPI_RX_FIFO_PORT_L(hal)            ((hal->pspi_base) + ((0x0E) << 2))
#define HAL_PSPI_RX_FIFO_PORT_H(hal)            ((hal->pspi_base) + ((0x0F) << 2))
#define HAL_PSPI_RX_FIFO_PORT_BYTE_1(hal)       ((hal->pspi_base) + ((0x0E) << 2))
#define HAL_PSPI_RX_FIFO_PORT_BYTE_2(hal)       ((hal->pspi_base) + ((0x0E) << 2) + 2)
#define HAL_PSPI_RX_FIFO_PORT_BYTE_3(hal)       ((hal->pspi_base) + ((0x0F) << 2))
#define HAL_PSPI_RX_FIFO_PORT_BYTE_4(hal)       ((hal->pspi_base) + ((0x0F) << 2) + 2)
#define HAL_PSPI_CPU_STATUS_1_2(hal)            ((hal->pspi_base) + ((0x10) << 2))
#define HAL_PSPI_CPU_STATUS_3(hal)              ((hal->pspi_base) + ((0x11) << 2))
#define HAL_PSPI_CPU_INTERRUPT_ENABLE_0_1(hal)  ((hal->pspi_base) + ((0x12) << 2))
#define HAL_PSPI_CPU_INTERRUPT_ENABLE_3(hal)    ((hal->pspi_base) + ((0x13) << 2))
#define HAL_PSPI_HOST_STATUS_0_1(hal)           ((hal->pspi_base) + ((0x14) << 2))
#define HAL_PSPI_HOST_STATUS_2(hal)             ((hal->pspi_base) + ((0x15) << 2))
#define HAL_PSPI_HOST_INTERRUPT_ENABLE_0_1(hal) ((hal->pspi_base) + ((0x16) << 2))
#define HAL_PSPI_HOST_INTERRUPT_ENABLE_2(hal)   ((hal->pspi_base) + ((0x17) << 2))
#define HAL_PSPI_TX_DMA_START_ADDRESS_L(hal)    ((hal->pspi_base) + ((0x18) << 2))
#define HAL_PSPI_TX_DMA_START_ADDRESS_H(hal)    ((hal->pspi_base) + ((0x19) << 2))
#define HAL_PSPI_RX_DMA_START_ADDRESS_L(hal)    ((hal->pspi_base) + ((0x1A) << 2))
#define HAL_PSPI_RX_DMA_START_ADDRESS_H(hal)    ((hal->pspi_base) + ((0x1B) << 2))
#define HAL_PSPI_TX_DMA_BYTE_COUNT_L(hal)       ((hal->pspi_base) + ((0x1C) << 2))
#define HAL_PSPI_TX_DMA_BYTE_COUNT_H(hal)       ((hal->pspi_base) + ((0x1D) << 2))
#define HAL_PSPI_RX_DMA_BYTE_COUNT_L(hal)       ((hal->pspi_base) + ((0x1E) << 2))
#define HAL_PSPI_RX_DMA_BYTE_COUNT_H(hal)       ((hal->pspi_base) + ((0x1F) << 2))
#define HAL_PSPI_TX_RX_565_MODE(hal)            ((hal->pspi_base) + ((0x20) << 2))
#define HAL_PSPI_ADDR21_CTRL(hal)               ((hal->pspi_base) + ((0x21) << 2))
#define HAL_PSPI_TE_TIME_SIZE(hal)              ((hal->pspi_base) + ((0x22) << 2))
#define HAL_PSPI_AUTO_TRIG_CTRL(hal)            ((hal->pspi_base) + ((0x23) << 2))
#define HAL_PSPI_ADDR24_CTRL(hal)               ((hal->pspi_base) + ((0x24) << 2))
#define HAL_PSPI_TIMEOUT_CNT                    2000

struct reg_t
{
    u64 reg_addr;
    u8  bit_shift;
    u16 bit_mask;
};

// pspi bank
#define HAL_PSPI_BANK \
    {                 \
        0x19          \
    }
// pspi reset
#define HAL_PSPI_RESET_REG               \
    {                                    \
        {                                \
            0x3CE8 + OS_BASEADDR, 0, 0x1 \
        }                                \
    }
// psppi data switch:3 or 4 line 0x1e 0x6f
#define HAL_PSPI_DATA_SWITCH_REG         \
    {                                    \
        {                                \
            0x3DBC + OS_BASEADDR, 0, 0x1 \
        }                                \
    }
// PM CLK 0xe 0x41
#define HAL_PM_IMI_CLK_REG               \
    {                                    \
        {                                \
            0x1d04 + OS_BASEADDR, 0, 0x1 \
        }                                \
    }
// 0x1e 0x30
#define HAL_BDMA2PMIMI_RST_REG           \
    {                                    \
        {                                \
            0x3CC0 + OS_BASEADDR, 0, 0x1 \
        }                                \
    }
// 0x1e 0x5b : 0:imi 2.psram
#define HAL_PSPI_MEM_SEL_REG             \
    {                                    \
        {                                \
            0x3D6C + OS_BASEADDR, 0, 0x3 \
        }                                \
    }

static u16          hal_pspi_bank[HAL_PSPI_NR_PORTS]            = HAL_PSPI_BANK;
static struct reg_t hal_pspi_reset_reg[HAL_PSPI_NR_PORTS]       = HAL_PSPI_RESET_REG;
static struct reg_t hal_pspi_data_switch_reg[HAL_PSPI_NR_PORTS] = HAL_PSPI_DATA_SWITCH_REG;
static struct reg_t hal_pm_imi_clk_reg[HAL_PSPI_NR_PORTS]       = HAL_PM_IMI_CLK_REG;
static struct reg_t hal_bamd2pmimi_rst_reg[HAL_PSPI_NR_PORTS]   = HAL_BDMA2PMIMI_RST_REG;
static struct reg_t hal_pspi_mem_sel_reg[HAL_PSPI_NR_PORTS]     = HAL_PSPI_MEM_SEL_REG;

#if HAL_PSPI_USE_LOCK
static CamOsMutex_t hal_pspi_lock;
#define hal_pspi_lock()   CamOsMutexLock(&hal_pspi_lock);
#define hal_pspi_unlock() CamOsMutexUnlock(&hal_pspi_lock);
#else
#define hal_pspi_lock()
#define hal_pspi_unlock()
#endif

int hal_pspi_to_gpiomode(struct hal_pspi *hal)
{
    int ret = 0;

    if (hal->cpol && (!hal->slave_mode))
    {
        if (hal->pad_mode > 0)
        {
            ret = pspi_gpio_pull_up(hal->pad_idx);
            pspi_gpio_pad_set(hal->pad_idx);
        }
        else
        {
            return -1;
        }
    }

    return ret;
}

int hal_pspi_to_pspimode(struct hal_pspi *hal)
{
    if (hal->cpol && (!hal->slave_mode))
    {
        return pspi_gpio_pad_set_val(hal->pad_idx, hal->pad_mode);
    }

    return 0;
}

void hal_pspi_cs_assert(struct hal_pspi *hal, u8 cs, u8 cs_keep)
{
    hal_pspi_lock();
    switch (hal->chip_select)
    {
        case 0:
            HAL_PSPI_WRITE_WORD_MASK(HAL_PSPI_ADDR21_CTRL(hal),
                                     hal->cs_level ? (0x2 << HAL_PSPI_CS_CTRL_SHIFT) : (0xa << HAL_PSPI_CS_CTRL_SHIFT),
                                     HAL_PSPI_CS_CTRL);
            break;
        case 1:
            HAL_PSPI_WRITE_WORD_MASK(HAL_PSPI_ADDR21_CTRL(hal),
                                     hal->cs_level ? (0x1 << HAL_PSPI_CS_CTRL_SHIFT) : (0x9 << HAL_PSPI_CS_CTRL_SHIFT),
                                     HAL_PSPI_CS_CTRL);
            break;
        default:
            HAL_PSPI_WRITE_WORD_MASK(HAL_PSPI_ADDR21_CTRL(hal),
                                     hal->cs_level ? (0x2 << HAL_PSPI_CS_CTRL_SHIFT) : (0xa << HAL_PSPI_CS_CTRL_SHIFT),
                                     HAL_PSPI_CS_CTRL);
            break;
    }
    hal_pspi_unlock();
}

void hal_pspi_cs_deassert(struct hal_pspi *hal, u8 cs, u8 cs_keep)
{
    hal_pspi_lock();
    if (!cs_keep)
    {
        switch (hal->chip_select)
        {
            case 0:
                HAL_PSPI_WRITE_WORD_MASK(HAL_PSPI_ADDR21_CTRL(hal),
                                         hal->cs_level ? (0xa << HAL_PSPI_CS_CTRL_SHIFT)
                                                       : (0x2 << HAL_PSPI_CS_CTRL_SHIFT),
                                         HAL_PSPI_CS_CTRL);
                break;
            case 1:
                HAL_PSPI_WRITE_WORD_MASK(HAL_PSPI_ADDR21_CTRL(hal),
                                         hal->cs_level ? (0x9 << HAL_PSPI_CS_CTRL_SHIFT)
                                                       : (0x1 << HAL_PSPI_CS_CTRL_SHIFT),
                                         HAL_PSPI_CS_CTRL);
                break;
            default:
                HAL_PSPI_WRITE_WORD_MASK(HAL_PSPI_ADDR21_CTRL(hal),
                                         hal->cs_level ? (0xa << HAL_PSPI_CS_CTRL_SHIFT)
                                                       : (0x2 << HAL_PSPI_CS_CTRL_SHIFT),
                                         HAL_PSPI_CS_CTRL);
                break;
        }
    }
    hal_pspi_unlock();
}

int hal_pspi_trigger(struct hal_pspi *hal)
{
    hal_pspi_lock();
    HAL_PSPI_WRITE_BYTE(HAL_PSPI_XCH(hal), 0x01);
    hal_pspi_unlock();

    return 0;
}

int hal_pspi_small_reset(struct hal_pspi *hal)
{
    int ret = 0;

    ret = hal_pspi_to_gpiomode(hal);

    hal_pspi_lock();
    HAL_PSPI_WRITE_WORD_MASK(HAL_PSPI_AUTO_TRIG_CTRL(hal), HAL_PSPI_SMALL_RESET, HAL_PSPI_SMALL_RESET);
    hal_pspi_unlock();

    ret |= hal_pspi_to_pspimode(hal);

    return ret;
}

void hal_pspi_transfer_en(struct hal_pspi *hal, bool is_tx)
{
    hal_pspi_lock();

    if (is_tx)
    {
        HAL_PSPI_WRITE_WORD_MASK(HAL_PSPI_ADDR2_CTRL(hal), HAL_PSPI_TX_ENABLR, HAL_PSPI_TX_RX_ENABLR);
    }
    else
    {
        HAL_PSPI_WRITE_WORD_MASK(HAL_PSPI_ADDR2_CTRL(hal), HAL_PSPI_RX_ENABLR, HAL_PSPI_TX_RX_ENABLR);
    }

    hal_pspi_unlock();
}

int hal_pspi_dma_write(struct hal_pspi *hal, u8 *data, u32 size)
{
    int ret = 0;

    hal_pspi_lock();

    // HAL_PSPI_WRITE_WORD_MASK(HAL_PSPI_AUTO_TRIG_CTRL(hal), HAL_PSPI_SMALL_RESET, HAL_PSPI_SMALL_RESET);

    HAL_PSPI_WRITE_WORD(HAL_PSPI_TX_DMA_BYTE_COUNT_L(hal), (size - 1) & 0xFFFF);
    HAL_PSPI_WRITE_WORD(HAL_PSPI_TX_DMA_BYTE_COUNT_H(hal), ((size - 1) >> 16) & 0xFFFF);

    ret = hal_pspi_small_reset(hal);
    if (ret)
    {
        return -1;
    }

    HAL_PSPI_WRITE_WORD_MASK(HAL_PSPI_ADDR0_CTRL(hal), 0x1 << HAL_PSPI_TX_DMA_ENABLE_SHIFT,
                             HAL_PSPI_TX_DMA_ENABLE | HAL_PSPI_RX_DMA_ENABLE);

    while (!(HAL_PSPI_TX_FIFO_READY & HAL_PSPI_READ_WORD(HAL_PSPI_ADDR24_CTRL(hal))))
        ;

    hal_pspi_trigger(hal);

    if (hal->wait_done)
    {
        ret = hal->wait_done(hal);
        if (ret)
        {
            size = 0;
        }
    }
    else
    {
        while (!(HAL_PSPI_TX_DONE_FLAG & HAL_PSPI_READ_WORD(HAL_PSPI_HOST_STATUS_0_1(hal))))
            ;
        ret = hal_pspi_to_gpiomode(hal);
        HAL_PSPI_WRITE_WORD_MASK(HAL_PSPI_AUTO_TRIG_CTRL(hal), HAL_PSPI_STATUS_CLEAR, HAL_PSPI_STATUS_CLEAR);
        HAL_PSPI_WRITE_WORD_MASK(HAL_PSPI_AUTO_TRIG_CTRL(hal), 0, HAL_PSPI_STATUS_CLEAR);
        ret |= hal_pspi_to_pspimode(hal);
        if (ret)
        {
            return -1;
        }
    }
    hal_pspi_unlock();

    return size;
}

int hal_pspi_dma_read(struct hal_pspi *hal, u8 *data, u32 size)
{
    int ret = 0;

    hal_pspi_lock();
    // HAL_PSPI_WRITE_WORD_MASK(HAL_PSPI_AUTO_TRIG_CTRL(hal), HAL_PSPI_SMALL_RESET, HAL_PSPI_SMALL_RESET);

    HAL_PSPI_WRITE_WORD(HAL_PSPI_TX_DMA_BYTE_COUNT_L(hal), (size - 1) & 0xFFFF);
    HAL_PSPI_WRITE_WORD(HAL_PSPI_TX_DMA_BYTE_COUNT_H(hal), ((size - 1) >> 16) & 0xFFFF);
    HAL_PSPI_WRITE_WORD(HAL_PSPI_RX_DMA_BYTE_COUNT_L(hal), (size - 1) & 0xFFFF);
    HAL_PSPI_WRITE_WORD(HAL_PSPI_RX_DMA_BYTE_COUNT_H(hal), ((size - 1) >> 16) & 0xFFFF);

    ret = hal_pspi_small_reset(hal);
    if (ret)
    {
        return -1;
    }

    HAL_PSPI_WRITE_WORD_MASK(HAL_PSPI_ADDR0_CTRL(hal), HAL_PSPI_RX_DMA_ENABLE,
                             HAL_PSPI_RX_DMA_ENABLE | HAL_PSPI_TX_DMA_ENABLE);

    hal_pspi_trigger(hal);

    if (hal->wait_done)
    {
        ret = hal->wait_done(hal);
        if (ret)
        {
            size = 0;
        }
    }
    else
    {
        while (!(HAL_PSPI_RX_DMA_INT_FLAG & HAL_PSPI_READ_WORD(HAL_PSPI_AUTO_TRIG_CTRL(hal))))
            ;
        ret = hal_pspi_to_gpiomode(hal);
        HAL_PSPI_WRITE_WORD_MASK(HAL_PSPI_AUTO_TRIG_CTRL(hal), HAL_PSPI_RX_DMA_INT_FLAG, HAL_PSPI_RX_DMA_INT_FLAG);
        ret |= hal_pspi_to_pspimode(hal);
        if (ret)
        {
            return -1;
        }
    }
    hal_pspi_unlock();

    return size;
}

int hal_pspi_fifo_write(struct hal_pspi *hal, u8 *data, u32 size)
{
    u32 size_left;
    u32 fifo_write_size;
    u32 fifo_write_left;
    u32 timeout_cnt;

    hal_pspi_lock();

    // fifo write config
    HAL_PSPI_WRITE_WORD_MASK(HAL_PSPI_ADDR0_CTRL(hal), 0x0 << HAL_PSPI_TX_DMA_ENABLE_SHIFT, HAL_PSPI_TX_DMA_ENABLE);
    HAL_PSPI_WRITE_WORD_MASK(HAL_PSPI_ADDR0_CTRL(hal), 0x0 << HAL_PSPI_RX_DMA_ENABLE_SHIFT, HAL_PSPI_RX_DMA_ENABLE);

    size_left = size;

    while (size_left)
    {
        fifo_write_size = (size_left > HAL_PSPI_FIFO_SIZE) ? HAL_PSPI_FIFO_SIZE : size_left;
        fifo_write_left = fifo_write_size;

        while (fifo_write_left)
        {
            if (fifo_write_left > 1)
            {
                HAL_PSPI_WRITE_WORD(HAL_PSPI_TX_FIFO_PORT_L(hal), *data | (*(data + 1) << 8));
                data += 2;
                fifo_write_left -= 2;
            }
            else
            {
                HAL_PSPI_WRITE_BYTE(HAL_PSPI_TX_FIFO_PORT_L(hal), *data);
                data++;
                fifo_write_left--;
            }
        }

        hal_pspi_trigger(hal);

        for (timeout_cnt = HAL_PSPI_TIMEOUT_COUNT; timeout_cnt > HAL_PSPI_TIMEOUT_CNT_END; timeout_cnt--)
        {
            if ((HAL_PSPI_READ_WORD(HAL_PSPI_CPU_STATUS_1_2(hal)) & 0x1040) == 0x1040)
            {
                break;
            }
        }

        if (timeout_cnt <= HAL_PSPI_TIMEOUT_CNT_END)
        {
            break;
        }

        size_left -= fifo_write_size;
    }
    hal_pspi_unlock();

    return (size - size_left);
}

int hal_pspi_fifo_read(struct hal_pspi *hal, u8 *data, u32 size)
{
    u8  j;
    u16 tmp_data;
    u32 size_left;
    u32 fifo_read_size;
    u32 timeout_cnt;

    hal_pspi_lock();

    HAL_PSPI_WRITE_WORD_MASK(HAL_PSPI_ADDR0_CTRL(hal), 0x0 << HAL_PSPI_TX_DMA_ENABLE_SHIFT, HAL_PSPI_TX_DMA_ENABLE);
    HAL_PSPI_WRITE_WORD_MASK(HAL_PSPI_ADDR0_CTRL(hal), 0x0 << HAL_PSPI_RX_DMA_ENABLE_SHIFT, HAL_PSPI_RX_DMA_ENABLE);

    size_left = size;

    while (size_left)
    {
        fifo_read_size = (size_left > HAL_PSPI_FIFO_SIZE) ? HAL_PSPI_FIFO_SIZE : size_left;

        HAL_PSPI_WRITE_WORD_MASK(HAL_PSPI_ADDR3_CTRL(hal), fifo_read_size << HAL_PSPI_RXFIFO_THRESHOLD_SHIFT,
                                 HAL_PSPI_RXFIFO_THRESHOLD);
        HAL_PSPI_WRITE_WORD(HAL_PSPI_TX_DMA_BYTE_COUNT_L(hal), fifo_read_size - 1);
        hal_pspi_trigger(hal);

        for (timeout_cnt = HAL_PSPI_TIMEOUT_COUNT; timeout_cnt > HAL_PSPI_TIMEOUT_CNT_END; timeout_cnt--)
        {
            if ((HAL_PSPI_READ_WORD(HAL_PSPI_CPU_STATUS_1_2(hal)) & 0x1000) == 0x1000)
            {
                break;
            }
        }

        for (j = 0; j < fifo_read_size; j += 2)
        {
            for (timeout_cnt = HAL_PSPI_TIMEOUT_COUNT; timeout_cnt > HAL_PSPI_TIMEOUT_CNT_END; timeout_cnt--)
            {
                if (!((HAL_PSPI_READ_WORD(HAL_PSPI_CPU_STATUS_1_2(hal))) & 0x4))
                {
                    break;
                }
            }
            tmp_data = HAL_PSPI_READ_WORD(HAL_PSPI_RX_FIFO_PORT_BYTE_1(hal));
            *data    = tmp_data & 0xFF;
            data++;
            if ((j + 1) < fifo_read_size)
            {
                *data = (tmp_data >> 8) & 0xFF;
                data++;
            }
        }

        size_left -= fifo_read_size;
    }
    hal_pspi_unlock();

    return (size - size_left);
}

void hal_pspi_init(struct hal_pspi *hal)
{
    u8 i;

    hal_pspi_lock();
    for (i = 0; i < (sizeof(hal_pspi_bank) / sizeof(u16)); i++)
    {
        if (hal_pspi_bank[i] == ((hal->pspi_base & 0xFFFF00) >> 9))
        {
            HAL_PSPI_WRITE_WORD_MASK(hal_pspi_reset_reg[i].reg_addr, 0 << hal_pspi_reset_reg[i].bit_shift,
                                     hal_pspi_reset_reg[i].bit_mask << hal_pspi_reset_reg[i].bit_shift);
            HAL_PSPI_WRITE_WORD_MASK(hal_pspi_reset_reg[i].reg_addr, 1 << hal_pspi_reset_reg[i].bit_shift,
                                     hal_pspi_reset_reg[i].bit_mask << hal_pspi_reset_reg[i].bit_shift);

            break;
        }
    }

    HAL_PSPI_WRITE_WORD_MASK(hal_pm_imi_clk_reg[0].reg_addr, 0 << hal_pm_imi_clk_reg[0].bit_shift,
                             hal_pm_imi_clk_reg[0].bit_mask << hal_pm_imi_clk_reg[0].bit_shift);
    HAL_PSPI_WRITE_WORD(hal_bamd2pmimi_rst_reg[0].reg_addr, 0);

    HAL_PSPI_WRITE_WORD_MASK(HAL_PSPI_ADDR0_CTRL(hal), 1 << HAL_PSPI_RX_FIFO_CLEAR_SHIFT, HAL_PSPI_RX_FIFO_CLEAR);
    HAL_PSPI_WRITE_WORD_MASK(HAL_PSPI_ADDR0_CTRL(hal), 1 << HAL_PSPI_TX_FIFO_CLEAR_SHIFT, HAL_PSPI_TX_FIFO_CLEAR);

    hal_pspi_unlock();
}

void hal_pspi_deinit(struct hal_pspi *hal)
{
    u8 i;

    hal_pspi_lock();
    for (i = 0; i < (sizeof(hal_pspi_bank) / sizeof(u16)); i++)
    {
        if (hal_pspi_bank[i] == ((hal->pspi_base & 0xFFFF00) >> 9))
        {
            HAL_PSPI_WRITE_WORD_MASK(hal_pspi_reset_reg[i].reg_addr, 0 << hal_pspi_reset_reg[i].bit_shift,
                                     hal_pspi_reset_reg[i].bit_mask << hal_pspi_reset_reg[i].bit_shift);
            HAL_PSPI_WRITE_WORD_MASK(hal_pspi_reset_reg[i].reg_addr, 1 << hal_pspi_reset_reg[i].bit_shift,
                                     hal_pspi_reset_reg[i].bit_mask << hal_pspi_reset_reg[i].bit_shift);
            break;
        }
    }
    HAL_PSPI_WRITE_WORD_MASK(HAL_PSPI_ADDR0_CTRL(hal), 1 << HAL_PSPI_RX_FIFO_CLEAR_SHIFT, HAL_PSPI_RX_FIFO_CLEAR);
    HAL_PSPI_WRITE_WORD_MASK(HAL_PSPI_ADDR0_CTRL(hal), 1 << HAL_PSPI_TX_FIFO_CLEAR_SHIFT, HAL_PSPI_TX_FIFO_CLEAR);
    hal_pspi_unlock();
}

void hal_pspi_controller_select(struct hal_pspi *hal)
{
    hal_pspi_lock();

    if (hal->slave_mode)
    {
        HAL_PSPI_WRITE_WORD_MASK(HAL_PSPI_ADDR2_CTRL(hal), 0 << HAL_PSPI_SPI_MODE_SELECT_SHIFT,
                                 HAL_PSPI_SPI_MODE_SELECT);
    }
    else
    {
        HAL_PSPI_WRITE_WORD_MASK(HAL_PSPI_ADDR2_CTRL(hal), 1 << HAL_PSPI_SPI_MODE_SELECT_SHIFT,
                                 HAL_PSPI_SPI_MODE_SELECT);
    }
    hal_pspi_unlock();
}

void hal_pspi_config(struct hal_pspi *hal)
{
    hal_pspi_lock();
    switch (hal->rgb_swap)
    {
        case 1:
            HAL_PSPI_WRITE_WORD_MASK(HAL_PSPI_TX_RX_565_MODE(hal), 0x1 << HAL_PSPI_565_MODE_SHIFT, HAL_PSPI_565_MODE);
            hal->data_lane     = 2;
            hal->bits_per_word = 9;
            break;
        case 2:
            HAL_PSPI_WRITE_WORD_MASK(HAL_PSPI_TX_RX_565_MODE(hal), 0x2 << HAL_PSPI_565_MODE_SHIFT, HAL_PSPI_565_MODE);
            hal->data_lane     = 2;
            hal->bits_per_word = 9;
            break;
        case 3:
            HAL_PSPI_WRITE_WORD_MASK(HAL_PSPI_TX_RX_565_MODE(hal), 0x3 << HAL_PSPI_565_MODE_SHIFT, HAL_PSPI_565_MODE);
            hal->data_lane     = 1;
            hal->bits_per_word = 9;
            break;
        case 4:
            HAL_PSPI_WRITE_WORD_MASK(HAL_PSPI_TX_RX_565_MODE(hal), 0x4 << HAL_PSPI_565_MODE_SHIFT, HAL_PSPI_565_MODE);
            hal->data_lane     = 1;
            hal->bits_per_word = 9;
            break;
        default:
            HAL_PSPI_WRITE_WORD_MASK(HAL_PSPI_TX_RX_565_MODE(hal), 0x0 << HAL_PSPI_565_MODE_SHIFT, HAL_PSPI_565_MODE);
            break;
    }

    switch (hal->data_lane)
    {
        case 1:
            HAL_PSPI_WRITE_WORD_MASK(HAL_PSPI_TX_RX_565_MODE(hal), 0 << HAL_PSPI_TX_MODE_SHIFT, HAL_PSPI_TX_MODE);
            HAL_PSPI_WRITE_WORD_MASK(HAL_PSPI_TX_RX_565_MODE(hal), 0 << HAL_PSPI_RX_MODE_SHIFT, HAL_PSPI_RX_MODE);
            break;
        case 2:
            HAL_PSPI_WRITE_WORD_MASK(HAL_PSPI_TX_RX_565_MODE(hal), 1 << HAL_PSPI_TX_MODE_SHIFT, HAL_PSPI_TX_MODE);
            HAL_PSPI_WRITE_WORD_MASK(HAL_PSPI_TX_RX_565_MODE(hal), 1 << HAL_PSPI_RX_MODE_SHIFT, HAL_PSPI_RX_MODE);
            break;
        case 4:
            HAL_PSPI_WRITE_WORD_MASK(HAL_PSPI_TX_RX_565_MODE(hal), 3 << HAL_PSPI_TX_MODE_SHIFT, HAL_PSPI_TX_MODE);
            HAL_PSPI_WRITE_WORD_MASK(HAL_PSPI_TX_RX_565_MODE(hal), 3 << HAL_PSPI_RX_MODE_SHIFT, HAL_PSPI_RX_MODE);
            break;
        default:
            HAL_PSPI_WRITE_WORD_MASK(HAL_PSPI_TX_RX_565_MODE(hal), 0 << HAL_PSPI_TX_MODE_SHIFT, HAL_PSPI_TX_MODE);
            HAL_PSPI_WRITE_WORD_MASK(HAL_PSPI_TX_RX_565_MODE(hal), 0 << HAL_PSPI_RX_MODE_SHIFT, HAL_PSPI_RX_MODE);
            break;
    }

    HAL_PSPI_WRITE_WORD_MASK(HAL_PSPI_TX_RX_565_MODE(hal), hal->lsb_first << HAL_PSPI_BIT_ORDER_INV_SHIFT,
                             HAL_PSPI_BIT_ORDER_INV);

    HAL_PSPI_WRITE_WORD_MASK(HAL_PSPI_CLKDIV(hal), (hal->divisor) << HAL_PSPI_DIV_SHIFT, HAL_PSPI_DIV);
    HAL_PSPI_WRITE_WORD_MASK(HAL_PSPI_ADDR2_CTRL(hal), (hal->cpol ? 1 : 0) << HAL_PSPI_POL_SHIFT, HAL_PSPI_POL);
    HAL_PSPI_WRITE_WORD_MASK(HAL_PSPI_ADDR2_CTRL(hal), (hal->cpha ? 1 : 0) << HAL_PSPI_PHA_SHIFT, HAL_PSPI_PHA);
    HAL_PSPI_WRITE_WORD_MASK(HAL_PSPI_ADDR2_CTRL(hal), (hal->cs_level ? 1 : 0) << HAL_PSPI_SSPOL_SHIFT, HAL_PSPI_SSPOL);
    HAL_PSPI_WRITE_WORD_MASK(HAL_PSPI_ADDR2_CTRL(hal), (hal->chip_select_timing ? 1 : 0) << HAL_PSPI_SSCTL_SHIFT,
                             HAL_PSPI_SSCTL);

    HAL_PSPI_WRITE_WORD_MASK(HAL_PSPI_ADDR2_CTRL(hal), 0x1 << HAL_PSPI_CLOCK_SELECT_SHIFT, HAL_PSPI_CLOCK_SELECT);

    HAL_PSPI_WRITE_WORD_MASK(HAL_PSPI_ADDR3_CTRL(hal), (hal->bits_per_word - 1) << HAL_PSPI_BIT_COUNT_SHIFT,
                             HAL_PSPI_BIT_COUNT);

    HAL_PSPI_WRITE_WORD(HAL_PSPI_DCYCLE(hal), hal->delay_cycle);
    HAL_PSPI_WRITE_WORD(HAL_PSPI_WCYCLE(hal), hal->wait_cycle);

    HAL_PSPI_WRITE_WORD(HAL_PSPI_TE_TIME_SIZE(hal), hal->te_time_delay);
    HAL_PSPI_WRITE_WORD_MASK(HAL_PSPI_ADDR21_CTRL(hal), (hal->te_polarity ? 1 : 0) << HAL_PSPI_TE_POL_SHIFT,
                             HAL_PSPI_TE_POL);
    HAL_PSPI_WRITE_WORD_MASK(HAL_PSPI_ADDR21_CTRL(hal), hal->te_skip_number << HAL_PSPI_TE_SKIP_NUMBER_SHIFT,
                             HAL_PSPI_TE_SKIP_NUMBER);

    if (hal->use_dma)
    {
        HAL_PSPI_WRITE_WORD(HAL_PSPI_TX_DMA_START_ADDRESS_L(hal), hal->mem_txaddr & 0xFFFF);
        HAL_PSPI_WRITE_WORD(HAL_PSPI_TX_DMA_START_ADDRESS_H(hal), (hal->mem_txaddr >> 16) & 0xFFFF);

        HAL_PSPI_WRITE_WORD(HAL_PSPI_RX_DMA_START_ADDRESS_L(hal), hal->mem_rxaddr & 0xFFFF);
        HAL_PSPI_WRITE_WORD(HAL_PSPI_RX_DMA_START_ADDRESS_H(hal), (hal->mem_rxaddr >> 16) & 0xFFFF);

        HAL_PSPI_WRITE_WORD_MASK(hal_pspi_mem_sel_reg[0].reg_addr, hal->mem_sel << hal_pspi_mem_sel_reg[0].bit_shift,
                                 hal_pspi_mem_sel_reg[0].bit_mask << hal_pspi_mem_sel_reg[0].bit_shift);
    }

    if (0 == hal->wire_3line)
    {
        HAL_PSPI_WRITE_WORD_MASK(hal_pspi_data_switch_reg[0].reg_addr, 1 << hal_pspi_data_switch_reg[0].bit_shift,
                                 hal_pspi_data_switch_reg[0].bit_mask << hal_pspi_data_switch_reg[0].bit_shift);
    }
    else
    {
        HAL_PSPI_WRITE_WORD_MASK(hal_pspi_data_switch_reg[0].reg_addr, 0 << hal_pspi_data_switch_reg[0].bit_shift,
                                 hal_pspi_data_switch_reg[0].bit_mask << hal_pspi_data_switch_reg[0].bit_shift);
    }

    hal_pspi_unlock();
}

int hal_pspi_irq_enable(struct hal_pspi *hal, enum hal_pspi_irq_type type, u8 u8_enable)
{
    int ret = 0;

    hal_pspi_lock();
    if (type == HAL_PSPI_IRQ_TX_DONE)
    {
        ret = hal_pspi_to_gpiomode(hal);
        HAL_PSPI_WRITE_WORD_MASK(HAL_PSPI_AUTO_TRIG_CTRL(hal), HAL_PSPI_STATUS_CLEAR, HAL_PSPI_STATUS_CLEAR);
        HAL_PSPI_WRITE_WORD_MASK(HAL_PSPI_AUTO_TRIG_CTRL(hal), 0, HAL_PSPI_STATUS_CLEAR);
        HAL_PSPI_WRITE_WORD_MASK(HAL_PSPI_HOST_INTERRUPT_ENABLE_0_1(hal), u8_enable ? HAL_PSPI_ENABLE_TX_DONE : 0,
                                 HAL_PSPI_ENABLE_TX_DONE);
        ret |= hal_pspi_to_pspimode(hal);
    }
    else if (type == HAL_PSPI_IRQ_RX_DMA_COMPLETED)
    {
        ret = hal_pspi_to_gpiomode(hal);
        HAL_PSPI_WRITE_WORD_MASK(HAL_PSPI_AUTO_TRIG_CTRL(hal), HAL_PSPI_RX_DMA_INT_FLAG, HAL_PSPI_RX_DMA_INT_FLAG);
        HAL_PSPI_WRITE_WORD_MASK(HAL_PSPI_AUTO_TRIG_CTRL(hal), u8_enable ? HAL_PSPI_ENABLE_RX_DMA_INT : 0,
                                 HAL_PSPI_ENABLE_RX_DMA_INT);
        ret |= hal_pspi_to_pspimode(hal);
    }
    if (ret)
    {
        return -1;
    }
    hal_pspi_unlock();

    return 0;
}

u32 hal_pspi_get_irq_type(struct hal_pspi *hal)
{
    u32 irq_type = 0;
    int ret      = 0;

    hal_pspi_lock();
    if (HAL_PSPI_TX_DONE_FLAG & HAL_PSPI_READ_WORD(HAL_PSPI_HOST_STATUS_0_1(hal)))
    {
        irq_type |= HAL_PSPI_IRQ_TX_DONE;
        ret = hal_pspi_to_gpiomode(hal);
        HAL_PSPI_WRITE_WORD_MASK(HAL_PSPI_AUTO_TRIG_CTRL(hal), HAL_PSPI_STATUS_CLEAR, HAL_PSPI_STATUS_CLEAR);
        HAL_PSPI_WRITE_WORD_MASK(HAL_PSPI_AUTO_TRIG_CTRL(hal), 0, HAL_PSPI_STATUS_CLEAR);
        ret |= hal_pspi_to_pspimode(hal);
    }
    else if (HAL_PSPI_RX_DMA_INT_FLAG & HAL_PSPI_READ_WORD(HAL_PSPI_AUTO_TRIG_CTRL(hal)))
    {
        irq_type |= HAL_PSPI_IRQ_RX_DMA_COMPLETED;
        ret = hal_pspi_to_gpiomode(hal);
        HAL_PSPI_WRITE_WORD_MASK(HAL_PSPI_AUTO_TRIG_CTRL(hal), HAL_PSPI_RX_DMA_INT_FLAG, HAL_PSPI_RX_DMA_INT_FLAG);
        ret |= hal_pspi_to_pspimode(hal);
    }

    if (ret)
    {
        irq_type = HAL_PSPI_IRQ_UNKNOWN;
    }

    hal_pspi_unlock();
    return irq_type;
}
