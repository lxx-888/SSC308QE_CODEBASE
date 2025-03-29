/*
 * hal_adclp.h- Sigmastar
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

#ifndef _HAL_ADCLP_H_
#define _HAL_ADCLP_H_

#include <adclp_os.h>

#ifdef CONFIG_ARM64
#define HAL_ADCLP_READ_WORD(_reg)        (*(volatile u16 *)(u64)(_reg))
#define HAL_ADCLP_WRITE_WORD(_reg, _val) ((*(volatile u16 *)(u64)(_reg)) = (u16)(_val))
#define HAL_ADCLP_WRITE_WORD_MASK(_reg, _val, _mask) \
    ((*(volatile u16 *)(u64)(_reg)) = ((*(volatile u16 *)(u64)(_reg)) & ~(_mask)) | ((u16)(_val) & (_mask)))
#define HAL_ADCLP_SET_WORD(_reg, _mask) \
    ((*(volatile u16 *)(u64)(_reg)) = (u16)((*(volatile u16 *)(u64)(_reg)) | (_mask)))
#define HAL_ADCLP_CLR_WORD(_reg, _mask) \
    ((*(volatile u16 *)(u64)(_reg)) = (u16)((*(volatile u16 *)(u64)(_reg)) & ~(_mask)))
#else
#define HAL_ADCLP_READ_WORD(_reg)        (*(volatile u16 *)(u32)(_reg))
#define HAL_ADCLP_WRITE_WORD(_reg, _val) ((*(volatile u16 *)(u32)(_reg)) = (u16)(_val))
#define HAL_ADCLP_WRITE_WORD_MASK(_reg, _val, _mask) \
    ((*(volatile u16 *)(u32)(_reg)) = ((*(volatile u16 *)(u32)(_reg)) & ~(_mask)) | ((u16)(_val) & (_mask)))
#define HAL_ADCLP_SET_WORD(_reg, _mask) \
    ((*(volatile u16 *)(u32)(_reg)) = (u16)((*(volatile u16 *)(u32)(_reg)) | (_mask)))
#define HAL_ADCLP_CLR_WORD(_reg, _mask) \
    ((*(volatile u16 *)(u32)(_reg)) = (u16)((*(volatile u16 *)(u32)(_reg)) & ~(_mask)))
#endif

#if !defined(BIT0) && !defined(BIT1)
#define BIT0  0x0001
#define BIT1  0x0002
#define BIT2  0x0004
#define BIT3  0x0008
#define BIT4  0x0010
#define BIT5  0x0020
#define BIT6  0x0040
#define BIT7  0x0080
#define BIT8  0x0100
#define BIT9  0x0200
#define BIT10 0x0400
#define BIT11 0x0800
#define BIT12 0x1000
#define BIT13 0x2000
#define BIT14 0x4000
#define BIT15 0x8000
#endif

/* adclp register*/

#define HAL_ADCLP_REG_CTRL0              (0x00 << 2)
#define HAL_ADCLP_REG_CKSAMP_PRD         (0x01 << 2)
#define HAL_ADCLP_REG_GCR_SAR_CH8        (0x02 << 2)
#define HAL_ADCLP_REG_AISEL_CTRL         (0x11 << 2)
#define HAL_ADCLP_REG_GPIO_CTRL          (0x12 << 2)
#define HAL_ADCLP_REG_INT_MASK           (0x14 << 2)
#define HAL_ADCLP_REG_INT_CLR            (0x15 << 2)
#define HAL_ADCLP_REG_INT_FORCE          (0x16 << 2)
#define HAL_ADCLP_REG_INT_STATUS         (0x17 << 2)
#define HAL_ADCLP_REG_CMP_OUT_RDY        (0x18 << 2)
#define HAL_ADCLP_REG_CH_REF_V_SEL       (0x19 << 2)
#define HAL_ADCLP_REG_CH1_UPB            (0x20 << 2)
#define HAL_ADCLP_REG_CH2_UPB            (0x21 << 2)
#define HAL_ADCLP_REG_CH3_UPB            (0x22 << 2)
#define HAL_ADCLP_REG_CH4_UPB            (0x23 << 2)
#define HAL_ADCLP_REG_CH5_UPB            (0x24 << 2)
#define HAL_ADCLP_REG_CH6_UPB            (0x25 << 2)
#define HAL_ADCLP_REG_CH7_UPB            (0x26 << 2)
#define HAL_ADCLP_REG_CH8_UPB            (0x27 << 2)
#define HAL_ADCLP_REG_CH1_LOB            (0x30 << 2)
#define HAL_ADCLP_REG_CH2_LOB            (0x31 << 2)
#define HAL_ADCLP_REG_CH3_LOB            (0x32 << 2)
#define HAL_ADCLP_REG_CH4_LOB            (0x33 << 2)
#define HAL_ADCLP_REG_CH5_LOB            (0x34 << 2)
#define HAL_ADCLP_REG_CH6_LOB            (0x35 << 2)
#define HAL_ADCLP_REG_CH7_LOB            (0x36 << 2)
#define HAL_ADCLP_REG_CH8_LOB            (0x37 << 2)
#define HAL_ADCLP_REG_CH1_DATA           (0x40 << 2)
#define HAL_ADCLP_REG_CH2_DATA           (0x41 << 2)
#define HAL_ADCLP_REG_CH3_DATA           (0x42 << 2)
#define HAL_ADCLP_REG_CH4_DATA           (0x43 << 2)
#define HAL_ADCLP_REG_CH5_DATA           (0x44 << 2)
#define HAL_ADCLP_REG_CH6_DATA           (0x45 << 2)
#define HAL_ADCLP_REG_CH7_DATA           (0x46 << 2)
#define HAL_ADCLP_REG_CH8_DATA           (0x47 << 2)
#define HAL_ADCLP_REG_SMCARD_CTRL        (0x50 << 2)
#define HAL_ADCLP_REG_FCIE_INT_CTRL      (0x51 << 2)
#define HAL_ADCLP_REG_INT_DIRECT2TOP_SEL (0x60 << 2)

#define HAL_ADCLP_SINGLE_CH_BIT     BIT2 | BIT1 | BIT0
#define HAL_ADCLP_SINGLE_CH_EN_BIT  BIT4
#define HAL_ADCLP_SAR_CH8_CPU_BIT   BIT0
#define HAL_ADCLP_SAR_CH8_IPU_BIT   BIT1
#define HAL_ADCLP_SAR_CH8_EN_BIT    BIT8
#define HAL_ADCLP_SAR_CH8_MUX_BIT   BIT2 | BIT1 | BIT0
#define HAL_ADCLP_SAR_OPERA_MODE    BIT5
#define HAL_ADCLP_DIG_PWR_DOWN_BIT  BIT6
#define HAL_ADCLP_START_BIT         BIT7
#define HAL_ADCLP_ATOP_PWR_DOWN_BIT BIT8
#define HAL_ADCLP_ATOP_FREERUN_BIT  BIT9
#define HAL_ADCLP_8_CHANNEL_EN_BIT  BIT11
#define HAL_ADCLP_LOAD_EN_BIT       BIT14

#define HAL_ADCLP_VDD_CPU_CH 7
#define HAL_ADCLP_VDD_IPU_CH 7

enum hal_adclp_vol
{
    HAL_ADCLP_VOL_1P0 = 1000,
    HAL_ADCLP_VOL_1P8 = 1800,
    HAL_ADCLP_VOL_3P3 = 3300,
};

struct hal_adclp_dev
{
    u64 base;
    u32 channel;
    u32 ref_vol;
    u16 upper_bound;
    u16 lower_bound;
};

void hal_adclp_int_clear(struct hal_adclp_dev *adclp_dev);
u16  hal_adclp_int_status(struct hal_adclp_dev *adclp_dev);
void hal_adclp_vdd_cpu_enable(struct hal_adclp_dev *adclp_dev);
void hal_adclp_vdd_ipu_enable(struct hal_adclp_dev *adclp_dev);
void hal_adclp_muxsel_enbale(struct hal_adclp_dev *adclp_dev, u8 enable);
void hal_adclp_set_bound(struct hal_adclp_dev *adclp_dev, u16 max, u16 min);
u16  hal_adclp_get_data(struct hal_adclp_dev *adclp_dev);
void hal_adclp_init(struct hal_adclp_dev *adclp_dev);
void hal_adclp_deinit(struct hal_adclp_dev *adclp_dev);

#endif
