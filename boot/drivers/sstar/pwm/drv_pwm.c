/*
 * drv_pwm.c- Sigmastar
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
#include <clk.h>
#include <pwm.h>
#include <compiler.h>
#include <dm/devres.h>
#include <hal_pwm.h>
#include <hal_pwm_cfg.h>

struct sstar_pwm_plat
{
    u32                channel;
    struct hal_pwm_cfg pwm_cfg;
};

static int sstar_pwm_set_up(struct udevice *dev)
{
    int                    ret;
    u32                    period;
    u32                    polarity;
    u32                    duty_cycle;
    struct sstar_pwm_plat *pwm_plat = dev_get_plat(dev);

    ret = dev_read_u32(dev, "period", &period);
    if (ret < 0 || !period)
    {
        pwm_dbg("failed to get period from dts\n");
        return -EINVAL;
    }

    hal_pwm_set_channel_reset(&pwm_plat->pwm_cfg, 1);

    ret = hal_pwm_set_period(&pwm_plat->pwm_cfg, period, 0);
    if (ret)
    {
        pwm_dbg("fail to config pwm%u period\n", pwm_plat->channel);
        return ret;
    }

    ret = dev_read_u32(dev, "duty_cycle", &duty_cycle);
    if (ret < 0)
        pwm_dbg("failed to get duty_cycle from dts\n");
    else
        hal_pwm_set_duty(&pwm_plat->pwm_cfg, duty_cycle, 0);

    ret = dev_read_u32(dev, "polarity", &polarity);
    if (ret < 0)
        pwm_dbg("failed to get polarity from dts\n");
    else
        hal_pwm_set_polarity(&pwm_plat->pwm_cfg, polarity, 0);

    hal_pwm_set_channel_reset(&pwm_plat->pwm_cfg, 0);

    return 0;
}

static int sstar_pwm_set_config(struct udevice *dev, uint channel, uint period_ns, uint duty_ns)
{
    int                    ret;
    struct sstar_pwm_plat *pwm_plat = dev_get_plat(dev);

    ret = hal_pwm_set_period(&pwm_plat->pwm_cfg, period_ns, 0);
    if (ret)
    {
        pwm_err("fail to config pwm%u period\n", pwm_plat->channel);
        return ret;
    }

    ret = hal_pwm_set_duty(&pwm_plat->pwm_cfg, duty_ns, 0);
    if (ret)
    {
        pwm_err("fail to config pwm%u duty\n", pwm_plat->channel);
        return ret;
    }

    return 0;
}

static int sstar_pwm_set_invert(struct udevice *dev, uint channel, bool polarity)
{
    struct sstar_pwm_plat *pwm_plat = dev_get_plat(dev);

    hal_pwm_set_polarity(&pwm_plat->pwm_cfg, polarity, 0);

    return 0;
}

static int sstar_pwm_set_enable(struct udevice *dev, uint channel, bool enable)
{
    struct sstar_pwm_plat *pwm_plat = dev_get_plat(dev);

    hal_pwm_set_channel_reset(&pwm_plat->pwm_cfg, !enable);

    return 0;
}

#ifndef CONFIG_SSTAR_CLK
static void sstar_pwm_setclk(struct sstar_pwm_plat *pwm_plat)
{
    pwm_dbg("pwm%u clock setting by operation register\n", pwm_plat->channel);
    HAL_PWM_WRITE_REG_MASK(pwm_reg[pwm_plat->channel].bank_base, pwm_reg[pwm_plat->channel].reg_offset,
                           pwm_cfg[pwm_plat->channel].clk_mod << pwm_reg[pwm_plat->channel].bit_shift,
                           pwm_reg[pwm_plat->channel].bit_mask);
    pwm_plat->pwm_cfg.clk_freq = pwm_cfg[pwm_plat->channel].clk_freq;
}
#endif

static int sstar_pwm_init(struct udevice *dev)
{
    int ret;
#ifdef CONFIG_SSTAR_CLK
    struct clk pwm_clk;
#endif

    struct sstar_pwm_plat *pwm_plat = dev_get_plat(dev);

    fdt_addr_t addr = dev_read_addr(dev);
    if (addr == FDT_ADDR_T_NONE)
        return -EINVAL;
    pwm_plat->pwm_cfg.bank_base = (u64)addr;
    pwm_plat->pwm_cfg.addr_base = pwm_plat->pwm_cfg.bank_base >> 0x18;
    pwm_plat->pwm_cfg.addr_base = pwm_plat->pwm_cfg.addr_base << 0x18;

    ret = dev_read_u32(dev, "channel", &pwm_plat->channel);
    if (ret)
    {
        pwm_err("fail to get channel from dts\n");
        return ret;
    }
    pwm_plat->pwm_cfg.channel = pwm_plat->channel;

#ifdef CONFIG_SSTAR_CLK
    ret = dev_read_u32(dev, "clock-freq", &pwm_plat->pwm_cfg.clk_freq);
    if (ret)
    {
        pwm_err("fail to get clock-freq from dts\n");
        return ret;
    }

    clk_get_by_index(dev, 0, &pwm_clk);
    clk_enable(&pwm_clk);
    clk_set_rate(&pwm_clk, pwm_plat->pwm_cfg.clk_freq);
#else
    sstar_pwm_setclk(pwm_plat);
#endif

    pwm_plat->pwm_cfg.group = -1;
    hal_pwm_init(&pwm_plat->pwm_cfg);

    if (sstar_pwm_set_up(dev))
        pwm_dbg("pwm[%u] failed to set parameters at initialization\n", pwm_plat->channel);

    return 0;
}

static const struct pwm_ops sstar_pwm_ops = {
    .set_config = sstar_pwm_set_config,
    .set_enable = sstar_pwm_set_enable,
    .set_invert = sstar_pwm_set_invert,
};

static const struct udevice_id sstar_pwm_ids[] = {{.compatible = "sstar,pwm"}, {}};

U_BOOT_DRIVER(sstar_pwm) = {
    .name      = "ssatr_pwm",
    .id        = UCLASS_PWM,
    .of_match  = sstar_pwm_ids,
    .ops       = &sstar_pwm_ops,
    .bind      = sstar_pwm_init,
    .plat_auto = sizeof(struct sstar_pwm_plat),
};
