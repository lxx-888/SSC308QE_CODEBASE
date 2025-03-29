/*
 * drv_adclp.c- Sigmastar
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

#include <dm.h>
#include <adc.h>
#ifdef CONFIG_SSTAR_CLK
#include <clk.h>
#endif
#include <hal_adclp.h>
#include <hal_adclp_cfg.h>

struct adclp_control
{
    u32                  chan_num;
    struct hal_adclp_dev adclp_dev;
};

static int sstar_adclp_start_channel(struct udevice *dev, int channel)
{
    struct adclp_control *adclp_ctrl = dev_get_priv(dev);

    adclp_ctrl->adclp_dev.channel = channel;

    return 0;
}

static int sstar_adclp_channel_data(struct udevice *dev, int channel, unsigned int *data)
{
    struct adclp_control *adclp_ctrl = dev_get_priv(dev);

    *data = (unsigned int)hal_adclp_get_data(&adclp_ctrl->adclp_dev);

    return 0;
}

static int sstar_adclp_stop(struct udevice *dev)
{
    return 0;
}

int sstar_adclp_set_srclk(struct udevice *dev)
{
#ifdef CONFIG_SSTAR_CLK
    int                   ret;
    struct clk            adclp_clk;
    struct adclp_control *adclp_ctrl = NULL;

    adclp_ctrl = (struct adclp_control *)dev_get_priv(dev);
    if (!adclp_ctrl)
    {
        adclp_err("fail to get adclp device pointer\n");
        return -ENOMEM;
    }

    ret = clk_get_by_index(dev, 0, &adclp_clk);
    if (ret < 0)
    {
        return -ENOENT;
    }

    ret = clk_enable(&adclp_clk);
    if (ret < 0)
    {
        adclp_err("fail to enable adclp clk\n");
        return ret;
    }

    ret = clk_set_rate(&adclp_clk, HAL_ADCLP_SRCCLK);
    if (ret < 0)
    {
        adclp_err("fail to set adclp clk rate\n");
        return ret;
    }

#else
    HAL_ADCLP_WRITE_WORD_MASK((adclp_clk_reg.bank_base + adclp_clk_reg.reg_offset),
                              adclp_clk_src.clk_mod << adclp_clk_reg.bit_shift, adclp_clk_reg.bit_mask);
#endif
    return 0;
}

static int sstar_adclp_of_to_plat(struct udevice *dev)
{
    u8                      i          = 0;
    int                     ret        = 0;
    fdt_addr_t              fdt_addr   = 0;
    struct adclp_control *  adclp_ctrl = NULL;
    struct adc_uclass_plat *uc_pdata   = dev_get_uclass_plat(dev);

    adclp_ctrl = (struct adclp_control *)dev_get_priv(dev);
    if (!adclp_ctrl)
    {
        adclp_err("fail to get adclp device pointer\n");
        return -ENOMEM;
    }

    ret = dev_read_u32(dev, "chan-num", &adclp_ctrl->chan_num);
    if (ret)
    {
        adclp_err("fail to get adclp channel number\n");
        return ret;
    }

    fdt_addr = dev_read_addr(dev);
    if (fdt_addr == FDT_ADDR_T_NONE)
    {
        adclp_err("fail to get adclp base addr\n");
        return -EINVAL;
    }

    ret = sstar_adclp_set_srclk(dev);
    if (ret < 0)
    {
        adclp_err("fail to set adclp clock\n");
        return ret;
    }

    adclp_ctrl->adclp_dev.lower_bound = 0;
    adclp_ctrl->adclp_dev.upper_bound = 0x3FF;
    adclp_ctrl->adclp_dev.base        = (phys_addr_t)fdt_addr;
    adclp_ctrl->adclp_dev.ref_vol     = dev_read_u32_default(dev, "ref-voltage", 1);

    uc_pdata->data_mask   = 0x3FF;
    uc_pdata->data_format = ADC_DATA_FORMAT_BIN;
    for (i = 0; i < adclp_ctrl->chan_num; i++)
    {
        uc_pdata->channel_mask |= 0x1 << i;
        adclp_ctrl->adclp_dev.channel = i;
        hal_adclp_init(&adclp_ctrl->adclp_dev);
    }
    adclp_ctrl->adclp_dev.channel = 0;

    return 0;
}

static const struct adc_ops sstar_adclp_ops = {
    .start_channel = sstar_adclp_start_channel,
    .channel_data  = sstar_adclp_channel_data,
    .stop          = sstar_adclp_stop,
};

static const struct udevice_id sstar_adclp_ids[] = {{.compatible = "sstar,adclp"}, {}};

U_BOOT_DRIVER(sstar_adclp) = {
    .name       = "sstar-adclp",
    .id         = UCLASS_ADC,
    .of_match   = sstar_adclp_ids,
    .ops        = &sstar_adclp_ops,
    .of_to_plat = sstar_adclp_of_to_plat,
    .priv_auto  = sizeof(struct adclp_control),
};
