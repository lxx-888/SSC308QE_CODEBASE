/*
 * hal_iic.h- Sigmastar
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

#ifndef _HAL_IIC_H_
#define _HAL_IIC_H_

#include <iic_os.h>

#define HAL_I2C_NS_2_CNT(ns, clk) ((ns * clk / 1000) + !!(ns * clk % 1000))

#define HAL_I2C_CNT_DEC_12M (6)
#define HAL_I2C_CNT_DEC_54M (6)
#define HAL_I2C_CNT_DEC_72M (7)

#define HAL_I2C_SPEED_100KHZ  (100000)
#define HAL_I2C_SPEED_400KHZ  (400000)
#define HAL_I2C_SPEED_700KHZ  (700000)
#define HAL_I2C_SPEED_1000KHZ (1000000)
#define HAL_I2C_SPEED_1500KHZ (1500000)

#define HAL_I2C_NO_START (0x4000)

#define HAL_I2C_MAX_SPEED (1500000)
#define HAL_I2C_MIN_SPEED (1000)

#define HAL_I2C_SET_WRITEBIT_INDATA (0xFE)
#define HAL_I2C_SET_READBIT_INDATA  (0x01)

#define HAL_I2C_SM_CNT_TSUSTA     HAL_I2C_NS_2_CNT(4700, 12)
#define HAL_I2C_SM_CNT_THDSTA     HAL_I2C_NS_2_CNT(4000, 12)
#define HAL_I2C_SM_CNT_TSUSTO     HAL_I2C_NS_2_CNT(4000, 12)
#define HAL_I2C_SM_CNT_THDSTO     HAL_I2C_NS_2_CNT(4700, 12)
#define HAL_I2C_SM_CNT_THDDAT     HAL_I2C_NS_2_CNT(5000, 12)
#define HAL_I2C_SM_CNT_TSUDAT_MIN HAL_I2C_NS_2_CNT(250, 12)

#define HAL_I2C_FM_CNT_TSUSTA HAL_I2C_NS_2_CNT(600, 54)
#define HAL_I2C_FM_CNT_THDSTO HAL_I2C_NS_2_CNT(600, 54)

#define HAL_I2C_SUPPLY_TSUSTA (3)
#define HAL_I2C_SUPPLY_THDSTA (1)
#define HAL_I2C_SUPPLY_TSUSTO (3)
#define HAL_I2C_SUPPLY_TSUDAT (1)
#define HAL_I2C_SUPPLY_THDDAT (2)
#define HAL_I2C_SUPPLY_RCKDLY (1)

typedef enum
{
    HAL_I2C_OK = 0,
    HAL_I2C_ERR,
    HAL_I2C_INIT,         // 2 INIT ERR
    HAL_I2C_MST_SETUP,    // 3 I2C MASTER SET UP ERR
    HAL_I2C_CNT_SETUP,    // 4 COUNT SET UP ERR
    HAL_I2C_DMA_SETUP,    // 5 DMA SET UP ERR
    HAL_I2C_SRCCLK_SETUP, // 6 SET SOURCE CLK ERR
    HAL_I2C_WRITE,        // 7 WRITE ERR
    HAL_I2C_READ,         // 8 READ ERR
    HAL_I2C_PARAMETER,    // 9 PARAMETER ERR
    HAL_I2C_TIMEOUT,      // 10 TIMEOUT ERR
    HAL_I2C_STOP_CMD,     // 11 STOP SIGNAL ERR
    HAL_I2C_RETRY,
} i2c_err_num;

enum i2c_output_mode
{
    HAL_I2C_OPEN_DRAIN = 1,
    HAL_I2C_OPEN_DRAIN_OEN,
    HAL_I2C_OPEN_DRAIN_OEN_MULT,
    HAL_I2C_PUSH_PULL,
};

enum i2c_addr_mode
{
    HAL_I2C_ADDRMODE_NORMAL = 0,
    HAL_I2C_ADDRMODE_10BIT,
    HAL_I2C_ADDRMODE_MAX,
};

enum i2c_miu_priority
{
    HAL_I2C_MIUPRI_LOW = 0,
    HAL_I2C_MIUPRI_HIGH,
    HAL_I2C_MIUPRI_MAX,
};

enum i2c_miu_channel
{
    HAL_I2C_MIU_CHANNEL0 = 0,
    HAL_I2C_MIU_CHANNEL1,
    HAL_I2C_MIU_MAX,
};

struct hal_i2c_clkcnt
{
    u16 cnt_scl_high;
    u16 cnt_scl_low;
    u16 cnt_sda_hold;
    u16 cnt_sda_latch;
    u16 cnt_timeout_delay;
    u16 cnt_start_setup;
    u16 cnt_start_hold;
    u16 cnt_stop_setup;
    u16 cnt_stop_hold;
    u16 cnt_rck_scl_dly;
    u16 cnt_rck_sda_dly;
};

struct hal_i2c_clkcnt_ns
{
    u32 ns_start_setup;
    u32 ns_start_hold;
    u32 ns_stop_setup;
    u32 ns_stop_hold;
    u32 ns_data_setup;
    u32 ns_data_hold;
    u32 ns_rck_dly;
};

struct hal_i2c_dma_addr
{
    u64 dma_phys_addr;
    u8 *dma_virt_addr;
    u64 dma_miu_addr;
};

struct hal_i2c_dma_ctrl
{
    enum i2c_addr_mode      dma_addr_mode;
    enum i2c_miu_priority   dma_miu_prioty;
    enum i2c_miu_channel    dma_miu_chnnel;
    struct hal_i2c_dma_addr dma_addr_msg;
    u8                      dma_intr_en;
};

enum hal_i2c_speed_mode
{
    HAL_I2C_SM = 0,
    HAL_I2C_FM,
    HAL_I2C_FM_PLUS,
    HAL_I2C_HS,
};

struct hal_i2c_ctrl
{
    u64                      bank_addr;
    u32                      dma_en;
    u32                      speed;
    u32                      group;
    u32                      output_mode;
    u32                      match_rate;
    u16                      clkcnt_per_us;
    u8                       config;
    u8                       speed_mode;
    struct hal_i2c_clkcnt    clock_count;
    struct hal_i2c_clkcnt_ns clock_attr_time;
    struct hal_i2c_dma_ctrl  dma_ctrl;
    s32 (*calbak_dma_transfer)(void *);
};

extern s32 hal_i2c_cnt_reg_set(struct hal_i2c_ctrl *para_hal_ctrl);
extern s32 hal_i2c_dma_reset(struct hal_i2c_ctrl *para_hal_ctrl, u8 para_disenable);
extern s32 hal_i2c_reset(struct hal_i2c_ctrl *para_hal_ctrl, u8 para_rst);
extern s32 hal_i2c_dma_intr_en(struct hal_i2c_ctrl *para_hal_ctrl, u8 para_disenable);
extern u32 hal_i2c_dma_trans_cnt(struct hal_i2c_ctrl *para_hal_ctrl);
extern s32 hal_i2c_dma_trigger(struct hal_i2c_ctrl *para_hal_ctrl);
extern s32 hal_i2c_dma_done_clr(struct hal_i2c_ctrl *para_hal_ctrl, u8 para_val);
extern s32 hal_i2c_dma_stop_set(struct hal_i2c_ctrl *para_hal_ctrl, u8 para_stop);
extern s32 hal_i2c_release(struct hal_i2c_ctrl *para_hal_ctrl);
extern s32 hal_i2c_wn_mode_clr(struct hal_i2c_ctrl *para_hal_ctrl);
extern s32 hal_i2c_wn_write(struct hal_i2c_ctrl *para_hal_ctrl, u16 para_slvadr, u8 *para_pdata, u32 para_len,
                            u8 para_wnlen, u16 para_waitcnt);
extern s32 hal_i2c_dma_async_read(struct hal_i2c_ctrl *para_hal_ctrl, u16 para_slvadr, u8 *para_pdata, u32 para_len);
extern s32 hal_i2c_wn_async_write(struct hal_i2c_ctrl *para_hal_ctrl, u16 para_slvadr, u8 *para_pdata, u32 para_len,
                                  u8 para_wnlen, u16 para_waitcnt);
extern s32 hal_i2c_dma_async_write(struct hal_i2c_ctrl *para_hal_ctrl, u16 para_slvadr, u8 *para_pdata, u32 para_len);
extern s32 hal_i2c_write(struct hal_i2c_ctrl *para_hal_ctrl, u16 para_slvadr, u8 *para_pdata, u32 para_len,
                         u16 para_flag);
extern s32 hal_i2c_read(struct hal_i2c_ctrl *para_hal_ctrl, u16 para_slvadr, u8 *para_pdata, u32 para_len,
                        u16 para_flag);
extern s32 hal_i2c_speed_calc(struct hal_i2c_ctrl *para_hal_ctrl);
extern s32 hal_i2c_init(struct hal_i2c_ctrl *para_hal_ctrl);
#endif
