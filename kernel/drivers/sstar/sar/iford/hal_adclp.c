/*
 * hal_adclp.c- Sigmastar
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

#include "hal_adclp.h"

void hal_adclp_int_clear(struct hal_adclp_dev *adclp_dev)
{
    HAL_ADCLP_WRITE_WORD_MASK(adclp_dev->base + HAL_ADCLP_REG_INT_CLR, 0x1 << adclp_dev->channel,
                              0x1 << adclp_dev->channel);
}

u16 hal_adclp_int_status(struct hal_adclp_dev *adclp_dev)
{
    u16 status;

    status = HAL_ADCLP_READ_WORD(adclp_dev->base + HAL_ADCLP_REG_INT_STATUS);
    status &= (0x1 << adclp_dev->channel);

    return status ? 1 : 0;
}

void hal_adclp_vdd_cpu_enable(struct hal_adclp_dev *adclp_dev)
{
    HAL_ADCLP_WRITE_WORD_MASK(adclp_dev->base + HAL_ADCLP_REG_GCR_SAR_CH8, HAL_ADCLP_SAR_CH8_CPU_BIT,
                              HAL_ADCLP_SAR_CH8_MUX_BIT);
}

void hal_adclp_vdd_ipu_enable(struct hal_adclp_dev *adclp_dev)
{
    HAL_ADCLP_WRITE_WORD_MASK(adclp_dev->base + HAL_ADCLP_REG_GCR_SAR_CH8, HAL_ADCLP_SAR_CH8_IPU_BIT,
                              HAL_ADCLP_SAR_CH8_MUX_BIT);
}

void hal_adclp_muxsel_enbale(struct hal_adclp_dev *adclp_dev, u8 enable)
{
    HAL_ADCLP_WRITE_WORD_MASK(adclp_dev->base + HAL_ADCLP_REG_GCR_SAR_CH8, enable, HAL_ADCLP_SAR_CH8_EN_BIT);
}

void hal_adclp_set_bound(struct hal_adclp_dev *adclp_dev, u16 max, u16 min)
{
    HAL_ADCLP_WRITE_WORD((adclp_dev->base + HAL_ADCLP_REG_CH1_UPB + (adclp_dev->channel << 2)), max & 0x3FF);
    HAL_ADCLP_WRITE_WORD((adclp_dev->base + HAL_ADCLP_REG_CH1_LOB + (adclp_dev->channel << 2)), min & 0x3FF);
    adclp_dev->upper_bound = max & 0x3FF;
    adclp_dev->lower_bound = min & 0x3FF;
}

void hal_adclp_set_vol(struct hal_adclp_dev *adclp_dev, u32 ref_vol)
{
    switch (ref_vol)
    {
        case HAL_ADCLP_VOL_1P0:
            HAL_ADCLP_WRITE_WORD_MASK(adclp_dev->base + HAL_ADCLP_REG_CH_REF_V_SEL, 0, BIT0 << (adclp_dev->channel));
            break;

        case HAL_ADCLP_VOL_1P8:
            HAL_ADCLP_WRITE_WORD_MASK(adclp_dev->base + HAL_ADCLP_REG_CH_REF_V_SEL, BIT0 << (adclp_dev->channel),
                                      BIT0 << (adclp_dev->channel));
            break;

        default:
            HAL_ADCLP_WRITE_WORD_MASK(adclp_dev->base + HAL_ADCLP_REG_CH_REF_V_SEL, BIT0 << (adclp_dev->channel),
                                      BIT0 << (adclp_dev->channel));
            break;
    }
}

u16 hal_adclp_get_data(struct hal_adclp_dev *adclp_dev)
{
    u16 value = 0;

    HAL_ADCLP_WRITE_WORD_MASK(adclp_dev->base + HAL_ADCLP_REG_CTRL0, HAL_ADCLP_LOAD_EN_BIT, HAL_ADCLP_LOAD_EN_BIT);
    value = HAL_ADCLP_READ_WORD(adclp_dev->base + HAL_ADCLP_REG_CH1_DATA + (adclp_dev->channel << 2));

    return value;
}

void hal_adclp_init(struct hal_adclp_dev *adclp_dev)
{
    HAL_ADCLP_WRITE_WORD(adclp_dev->base + HAL_ADCLP_REG_CTRL0, 0x0A20);
    hal_adclp_set_vol(adclp_dev, adclp_dev->ref_vol);
    hal_adclp_set_bound(adclp_dev, adclp_dev->upper_bound, adclp_dev->lower_bound);
    /* before interrupt unmasking, the flag needs to be cleared once */
    hal_adclp_int_clear(adclp_dev);
    HAL_ADCLP_WRITE_WORD_MASK(adclp_dev->base + HAL_ADCLP_REG_INT_MASK, 0, 0x1 << adclp_dev->channel);
}

void hal_adclp_deinit(struct hal_adclp_dev *adclp_dev)
{
    HAL_ADCLP_WRITE_WORD_MASK(adclp_dev->base + HAL_ADCLP_REG_INT_MASK, 0x1 << adclp_dev->channel,
                              0x1 << adclp_dev->channel);
    hal_adclp_set_vol(adclp_dev, 0);
    hal_adclp_set_bound(adclp_dev, 0, 0);
    hal_adclp_int_clear(adclp_dev);
}
