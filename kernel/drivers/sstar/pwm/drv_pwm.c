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

#include <pwm_os.h>
#include <drv_pwm.h>
#include <hal_pwm.h>
#include <ms_msys.h>
#include <linux/fs.h>
#include <linux/pwm.h>
#include <linux/clk.h>
#include <linux/err.h>
#include <linux/list.h>
#include <linux/slab.h>
#include <cam_drv_pwm.h>
#include <linux/module.h>
#include <linux/of_irq.h>
#include <drv_camclk_Api.h>
#include <linux/interrupt.h>
#include <linux/miscdevice.h>
#include <linux/clk-provider.h>
#include <linux/platform_device.h>
#include "ms_platform.h"

#define PWM_NOADD_GROUP      0xFFFF
#define SPWM_NOADD_GROUP     0xFFFF
#define PWM_RES_REQUEST      0x0001
#define PWM_RES_FREE         0x0000
#define PWM_SYSFS_CREATED    0x0001
#define PWM_SYSFS_NO_CREATE  0x0000
#define PWM_LIST_ADD_GROUP   0x0001
#define PWM_LIST_ADD_CHANNEL 0x0000

struct pwm_group
{
    u8                     res;
    int                    irq;
    u32                    group;
    u32                    spwm_gp;
    u32                    channel;
    void *                 dev_data;
    struct file_operations pwm_fops;
    u64                    addr_base;
    struct list_head       group_node;
    struct miscdevice      pwm_miscdev;
    struct list_head       channel_head;
    char                   irq_name[12];
    u8                     sysfs_created;

#ifdef CONFIG_SSTAR_SPWM
    u8  s_init;
    u8  s_step;
    u64 s_period;
    u8  s_duty_idx;
    u8  s_duty_len;
    u64 s_duty_ns[HAL_SPWM_MAX_DUTY];
#endif
    u64 g_duty;
    u64 g_shift;
    u64 g_period;
    u64 g_polarity;
};

struct pwm_channel
{
    bool                   ddt;
    struct device *        dev;
    struct pwm_chip        chip;
    u32                    group;
    u32                    spwm_gp;
    u32                    channel;
    struct hal_pwm_cfg *   pwm_cfg;
    struct file_operations pwm_fops;
    u8                     list_add;
    u32                    clock_en;
    struct miscdevice      pwm_miscdev;
    struct list_head       channel_node;

#ifdef CONFIG_CAM_CLK
    u32   camclk_id;
    void *camclk_handle;
#else
    struct clk *   pwm_clk;
    struct clk_hw *parent_hw;
#endif
};

static LIST_HEAD(pwm_group_list);
static LIST_HEAD(pwm_channel_list);

static u32            sstar_pwm_dev_node  = 0;
static u32            sstar_pwm_last_init = 0;
static struct device *pwm_basic_dev       = NULL;

static int sstar_pwm_set_up(struct platform_device *pdev, struct pwm_channel *pwm_dev)
{
    int ret;
    int sync;
    u32 period;
    u32 duty_cycle;
    u32 polarity;

    ret = of_property_read_u32(pdev->dev.of_node, "period", &period);
    if (ret < 0 || !period)
    {
        pwm_dbg("fail to get pwm%u period from dts\n", pwm_dev->channel);
        return -EINVAL;
    }

    sync = pwm_dev->group >= 0 ? 1 : 0;
    if (sync)
        hal_pwm_set_group_reset(pwm_dev->pwm_cfg->addr_base, pwm_dev->group, 1);

    hal_pwm_set_channel_reset(pwm_dev->pwm_cfg, 1);

    ret = hal_pwm_set_period(pwm_dev->pwm_cfg, period, sync);
    if (ret)
        return ret;

    ret = of_property_read_u32(pdev->dev.of_node, "duty_cycle", &duty_cycle);
    if (ret < 0)
        pwm_dbg("fail to get pwm%u duty_cycle from dts\n", pwm_dev->channel);
    else
        hal_pwm_set_duty(pwm_dev->pwm_cfg, duty_cycle, sync);

    ret = of_property_read_u32(pdev->dev.of_node, "polarity", &polarity);
    if (ret < 0)
        pwm_dbg("fail to get pwm%u polarity from dts\n", pwm_dev->channel);
    else
        hal_pwm_set_polarity(pwm_dev->pwm_cfg, polarity, sync);

    if (!pwm_dev->clock_en)
    {
        ret = clk_prepare_enable(pwm_dev->pwm_clk);
        if (ret)
        {
            pwm_err("fail to enable pwm%u clk\n", pwm_dev->channel);
            return ret;
        }
        pwm_dev->clock_en++;
    }
    hal_pwm_set_channel_reset(pwm_dev->pwm_cfg, 0);

    return 0;
}

static irqreturn_t sstar_pwm_int_handler(int irq, void *dev_id)
{
#ifdef CONFIG_SSTAR_SPWM
    u8  i   = 0;
    int ret = 0;
#endif
    volatile u8         event  = 0;
    volatile u8         sync   = 0;
    struct pwm_channel *pos_ch = NULL;
    struct pwm_group *  gp_dev = (struct pwm_group *)dev_id;

    if (gp_dev->group != PWM_NOADD_GROUP)
    {
        pwm_dbg("pwm group%u enter irq handler\n", gp_dev->group);
        event = hal_pwm_get_hold_int(gp_dev->addr_base, gp_dev->group);
        if (event)
        {
            list_for_each_entry(pos_ch, &gp_dev->channel_head, channel_node)
            {
                if (pos_ch->group == gp_dev->group)
                    hal_pwm_sync_config(pos_ch->pwm_cfg);
            }
            hal_pwm_hold_mode_enable(gp_dev->addr_base, gp_dev->group, 0);
        }

        event = hal_pwm_get_round_int(gp_dev->addr_base, gp_dev->group);
        if (event)
        {
            hal_pwm_set_group_reset(gp_dev->addr_base, gp_dev->group, 1);
            list_for_each_entry(pos_ch, &gp_dev->channel_head, channel_node)
            {
                if (pos_ch->group == gp_dev->group)
                    sync = hal_pwm_sync_config(pos_ch->pwm_cfg);
            }

            if (!sync)
            {
                hal_pwm_set_round_num(gp_dev->addr_base, gp_dev->group, 0);
            }
            else
            {
                hal_pwm_group_enbale(gp_dev->addr_base, gp_dev->group, 1);
                hal_pwm_set_group_reset(gp_dev->addr_base, gp_dev->group, 0);
            }
        }
    }

#ifdef CONFIG_SSTAR_SPWM
    if (gp_dev->spwm_gp != SPWM_NOADD_GROUP)
    {
        pwm_dbg("spwm group%u enter irq handler\n", gp_dev->spwm_gp);
        event = hal_spwm_periods_int_status(gp_dev->addr_base, gp_dev->spwm_gp);
        if (event)
        {
            hal_spwm_vdben_sw_en(gp_dev->addr_base, gp_dev->spwm_gp, 0);
            list_for_each_entry(pos_ch, &gp_dev->channel_head, channel_node)
            {
                if (pos_ch->spwm_gp == gp_dev->spwm_gp)
                {
                    hal_pwm_set_period(pos_ch->pwm_cfg, gp_dev->s_period, 0);
                }
            }
            hal_spwm_vdben_sw_en(gp_dev->addr_base, gp_dev->spwm_gp, 1);
            hal_spwm_periods_int_clear(gp_dev->addr_base, gp_dev->spwm_gp);
            hal_spwm_periods_int_en(gp_dev->addr_base, gp_dev->spwm_gp, 0);
        }

        event = hal_spwm_sram_data_int_status(gp_dev->addr_base, gp_dev->spwm_gp);
        if (event)
        {
            list_for_each_entry(pos_ch, &gp_dev->channel_head, channel_node)
            {
                if (pos_ch->spwm_gp == gp_dev->spwm_gp)
                {
                    ret = hal_spwm_match_step(pos_ch->pwm_cfg, (u16)gp_dev->s_duty_idx);
                    if (!ret)
                    {
                        for (i = 0; i < gp_dev->s_duty_len; i++)
                        {
                            if ((gp_dev->s_duty_idx + i) < HAL_SPWM_MAX_DUTY)
                            {
                                hal_spwm_sram_duty(gp_dev->addr_base, gp_dev->spwm_gp, gp_dev->s_period,
                                                   (gp_dev->s_duty_idx + i), gp_dev->s_duty_ns[i]);
                            }
                            else
                            {
                                hal_spwm_sram_duty(gp_dev->addr_base, gp_dev->spwm_gp, gp_dev->s_period,
                                                   (gp_dev->s_duty_idx + i - HAL_SPWM_MAX_DUTY), gp_dev->s_duty_ns[i]);
                            }
                        }
                    }
                }
            }
            hal_spwm_sram_data_int_en(gp_dev->addr_base, gp_dev->spwm_gp, 0);
            hal_spwm_sram_data_int_clear(gp_dev->addr_base, gp_dev->spwm_gp);
        }

        event = hal_pwm_get_round_int(gp_dev->addr_base, gp_dev->spwm_gp);
        if (event)
        {
            hal_pwm_set_group_reset(gp_dev->addr_base, gp_dev->spwm_gp, 1);
            hal_pwm_set_round_num(gp_dev->addr_base, gp_dev->spwm_gp, 0);
            list_for_each_entry(pos_ch, &gp_dev->channel_head, channel_node)
            {
                if (pos_ch->spwm_gp == gp_dev->spwm_gp)
                {
                    ret = hal_spwm_set_init_pointer(pos_ch->pwm_cfg, gp_dev->s_step);
                    if (ret)
                        return -EINVAL;
                }
            }
        }
    }
#endif

    return IRQ_HANDLED;
}

static inline struct pwm_channel *to_pwm_channel(struct pwm_chip *chip)
{
    return (struct pwm_channel *)container_of(chip, struct pwm_channel, chip);
}

static int sstar_pwm_config(struct pwm_chip *chip, struct pwm_device *pwm, int duty_ns, int period_ns)
{
    struct pwm_channel *pwm_ch_dev = to_pwm_channel(chip);

    if (hal_pwm_set_period(pwm_ch_dev->pwm_cfg, period_ns, 0))
        return -EINVAL;
    if (hal_pwm_set_duty(pwm_ch_dev->pwm_cfg, duty_ns, 0))
        return -EINVAL;
    if (pwm_ch_dev->pwm_cfg->pwm_attr.shift)
    {
        if (hal_pmw_set_shift(pwm_ch_dev->pwm_cfg, 0, 0))
            return -EINVAL;
    }

    return 0;
}

static int sstar_pwm_enable(struct pwm_chip *chip, struct pwm_device *pwm)
{
    int                 ret        = 0;
    struct pwm_channel *pwm_ch_dev = to_pwm_channel(chip);

    if (!pwm_ch_dev->clock_en)
    {
        ret = clk_prepare_enable(pwm_ch_dev->pwm_clk);
        if (ret)
        {
            pwm_err("fail to enable pwm%u clk\n", pwm_ch_dev->channel);
            return ret;
        }
        pwm_ch_dev->clock_en++;
    }
    hal_pwm_set_channel_reset(pwm_ch_dev->pwm_cfg, 0);

    return 0;
}

static void sstar_pwm_disable(struct pwm_chip *chip, struct pwm_device *pwm)
{
    struct pwm_channel *pwm_ch_dev = to_pwm_channel(chip);

    hal_pwm_set_channel_reset(pwm_ch_dev->pwm_cfg, 1);
    if (pwm_ch_dev->clock_en)
    {
        clk_disable_unprepare(pwm_ch_dev->pwm_clk);
        pwm_ch_dev->clock_en--;
    }
}

static int sstar_pwm_set_polarity(struct pwm_chip *chip, struct pwm_device *pwm, enum pwm_polarity polarity)
{
    struct pwm_channel *pwm_ch_dev = to_pwm_channel(chip);

    hal_pwm_set_polarity(pwm_ch_dev->pwm_cfg, (u8)polarity, 0);

    return 0;
}

static void sstar_pwm_get_state(struct pwm_chip *chip, struct pwm_device *pwm, struct pwm_state *state)
{
    struct pwm_channel *pwm_ch_dev = to_pwm_channel(chip);

    state->period     = pwm_ch_dev->pwm_cfg->pwm_attr.period_wf;
    state->polarity   = hal_pwm_get_polarity(pwm_ch_dev->pwm_cfg);
    state->enabled    = hal_pwm_get_channel_reset(pwm_ch_dev->pwm_cfg) ? 0 : 1;
    state->duty_cycle = pwm_ch_dev->pwm_cfg->pwm_attr.duty_wf - pwm_ch_dev->pwm_cfg->pwm_attr.shift_wf;
}

static const struct pwm_ops sstar_pwm_ops = {
    .config       = sstar_pwm_config,
    .enable       = sstar_pwm_enable,
    .disable      = sstar_pwm_disable,
    .set_polarity = sstar_pwm_set_polarity,
    .get_state    = sstar_pwm_get_state,
    .owner        = THIS_MODULE,
};

static int sstar_pwm_chip_init(struct platform_device *pdev, struct pwm_channel *pwm_dev)
{
    int ret;

    pwm_dev->chip.dev            = &pdev->dev;
    pwm_dev->chip.ops            = &sstar_pwm_ops;
    pwm_dev->chip.of_xlate       = of_pwm_xlate_with_flags;
    pwm_dev->chip.of_pwm_n_cells = 3;
    pwm_dev->chip.base           = pwm_dev->channel;
    pwm_dev->chip.npwm           = 1;
    ret                          = pwmchip_add(&pwm_dev->chip);
    if (ret < 0)
    {
        pwm_err("fail to register a new pwm chip\n");
        return ret;
    }

    return 0;
}

static long sstar_pwm_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    int                 ret = 0;
    struct pwm_ch_cfg   ch_cfg;
    struct pwm_gp_cfg   gp_cfg;
    struct pwm_channel *pos_ch;
    struct pwm_channel *pwm_ch_dev = NULL;
    struct pwm_group *  pwm_gp_dev = NULL;

    if ((PWM_IOC_MAGIC != _IOC_TYPE(cmd)) || (_IOC_NR(cmd) > PWM_IOC_MAXNR))
    {
        return -ENOTTY;
    }

    if (_IOC_DIR(cmd) & _IOC_READ)
    {
        ret = !access_ok((void __user *)arg, _IOC_SIZE(cmd));
    }
    else if (_IOC_DIR(cmd) & _IOC_WRITE)
    {
        ret = !access_ok((void __user *)arg, _IOC_SIZE(cmd));
    }
    if (ret)
    {
        return -EFAULT;
    }

    switch (cmd)
    {
        case IOCTL_PWM_SET_CHAN_CFG:
            if (copy_from_user(&ch_cfg, (struct pwm_ch_cfg __user *)arg, sizeof(struct pwm_ch_cfg)))
            {
                return -EFAULT;
            }

            pwm_ch_dev = (struct pwm_channel *)container_of(file->f_op, struct pwm_channel, pwm_fops);
            if (ch_cfg.enable)
            {
                hal_pwm_set_period(pwm_ch_dev->pwm_cfg, ch_cfg.period, 0);
                hal_pwm_set_duty(pwm_ch_dev->pwm_cfg, ch_cfg.duty, 0);
                hal_pmw_set_shift(pwm_ch_dev->pwm_cfg, ch_cfg.shift, 0);
                hal_pwm_set_polarity(pwm_ch_dev->pwm_cfg, ch_cfg.polarity, 0);
                if (!pwm_ch_dev->clock_en)
                {
                    ret = clk_prepare_enable(pwm_ch_dev->pwm_clk);
                    if (ret)
                    {
                        pwm_err("fail to enable pwm%u clk\n", pwm_ch_dev->channel);
                        return ret;
                    }
                    pwm_ch_dev->clock_en++;
                }
                hal_pwm_set_channel_reset(pwm_ch_dev->pwm_cfg, 0);
            }
            else
            {
                hal_pwm_set_channel_reset(pwm_ch_dev->pwm_cfg, 1);
                if (pwm_ch_dev->clock_en)
                {
                    clk_disable_unprepare(pwm_ch_dev->pwm_clk);
                    pwm_ch_dev->clock_en--;
                }
            }
#ifdef CONFIG_SSTAR_PWM_DDT
            if (ch_cfg.ddt_en)
            {
                if (!pwm_ch_dev->ddt)
                {
                    pwm_err("pwm%u does not support dead time\n", pwm_ch_dev->channel);
                    return -EINVAL;
                }

                ret = hal_pwm_get_channel_reset(pwm_ch_dev->pwm_cfg);
                if (ret)
                {
                    pwm_err("pwm%u should output waveform first\n", pwm_ch_dev->channel);
                    return -EINVAL;
                }

                hal_pwmddt_cnt(pwm_ch_dev->pwm_cfg, HAL_PWM_DDT_OUTP, ch_cfg.p_ddt);
                hal_pwmddt_cnt(pwm_ch_dev->pwm_cfg, HAL_PWM_DDT_OUTN, ch_cfg.n_ddt);
                hal_pwmddt_enable(pwm_ch_dev->pwm_cfg, HAL_PWM_DDT_OUTP, 1);
                hal_pwmddt_enable(pwm_ch_dev->pwm_cfg, HAL_PWM_DDT_OUTN, 1);
                hal_pwmddt_sync_trig(pwm_ch_dev->pwm_cfg, 1);
            }
            else
            {
                if (!pwm_ch_dev->ddt)
                {
                    return 0;
                }

                hal_pwmddt_enable(pwm_ch_dev->pwm_cfg, HAL_PWM_DDT_OUTP, 0);
                hal_pwmddt_enable(pwm_ch_dev->pwm_cfg, HAL_PWM_DDT_OUTN, 0);
                hal_pwmddt_sync_trig(pwm_ch_dev->pwm_cfg, 0);
            }
#endif
            break;

        case IOCTL_PWM_GET_CHAN_CFG:
            if (copy_from_user(&ch_cfg, (struct pwm_ch_cfg __user *)arg, sizeof(struct pwm_ch_cfg)))
            {
                return -EFAULT;
            }

            pwm_ch_dev      = (struct pwm_channel *)container_of(file->f_op, struct pwm_channel, pwm_fops);
            ch_cfg.enable   = !(hal_pwm_get_channel_reset(pwm_ch_dev->pwm_cfg));
            ch_cfg.period   = pwm_ch_dev->pwm_cfg->pwm_attr.period_wf;
            ch_cfg.duty     = pwm_ch_dev->pwm_cfg->pwm_attr.duty_wf;
            ch_cfg.shift    = pwm_ch_dev->pwm_cfg->pwm_attr.shift_wf;
            ch_cfg.polarity = hal_pwm_get_polarity(pwm_ch_dev->pwm_cfg);
#ifdef CONFIG_SSTAR_PWM_DDT
            if (!pwm_ch_dev->ddt)
            {
                return 0;
            }

            ch_cfg.ddt_en = pwm_ch_dev->pwm_cfg->ddt_attr.enable;
            ch_cfg.p_ddt  = pwm_ch_dev->pwm_cfg->ddt_attr.p_ddt;
            ch_cfg.n_ddt  = pwm_ch_dev->pwm_cfg->ddt_attr.n_ddt;
#endif
            if (copy_to_user((struct pwm_ch_cfg __user *)arg, &ch_cfg, sizeof(struct pwm_ch_cfg)))
            {
                return -EFAULT;
            }
            break;

        case IOCTL_PWM_SET_GROUP_CFG:
            if (copy_from_user(&gp_cfg, (struct pwm_gp_cfg __user *)arg, sizeof(struct pwm_gp_cfg)))
            {
                return -EFAULT;
            }

            pwm_gp_dev = (struct pwm_group *)container_of(file->f_op, struct pwm_group, pwm_fops);
            if (gp_cfg.enable)
            {
                list_for_each_entry(pos_ch, &pwm_gp_dev->channel_head, channel_node)
                {
                    if (pos_ch->group == pwm_gp_dev->group)
                    {
                        hal_pwm_set_period(pos_ch->pwm_cfg, gp_cfg.period, 1);
                        hal_pwm_set_duty(pos_ch->pwm_cfg, gp_cfg.duty, 1);
                        hal_pmw_set_shift(pos_ch->pwm_cfg, gp_cfg.shift, 1);
                        hal_pwm_set_polarity(pos_ch->pwm_cfg, gp_cfg.polarity, 1);
                        if (!pos_ch->clock_en)
                        {
                            ret = clk_prepare_enable(pos_ch->pwm_clk);
                            if (ret)
                            {
                                pwm_err("fail to enable pwm%u clk\n", pos_ch->channel);
                                return ret;
                            }
                            pos_ch->clock_en++;
                        }
                        hal_pwm_set_channel_reset(pos_ch->pwm_cfg, 0);
                    }
                }
                hal_pwm_group_enbale(pwm_gp_dev->addr_base, pwm_gp_dev->group, 1);
                hal_pwm_set_group_reset(pwm_gp_dev->addr_base, pwm_gp_dev->group, 0);
                hal_pwm_hold_mode_enable(pwm_gp_dev->addr_base, pwm_gp_dev->group, 1);
            }
            else
            {
                hal_pwm_set_group_reset(pwm_gp_dev->addr_base, pwm_gp_dev->group, 1);
                hal_pwm_group_enbale(pwm_gp_dev->addr_base, pwm_gp_dev->group, 0);
                list_for_each_entry(pos_ch, &pwm_gp_dev->channel_head, channel_node)
                {
                    hal_pwm_set_channel_reset(pos_ch->pwm_cfg, 1);
                    if (pos_ch->clock_en)
                    {
                        clk_disable_unprepare(pos_ch->pwm_clk);
                        pos_ch->clock_en--;
                    }
                }
            }
            pwm_gp_dev->g_duty     = gp_cfg.duty;
            pwm_gp_dev->g_shift    = gp_cfg.shift;
            pwm_gp_dev->g_period   = gp_cfg.period;
            pwm_gp_dev->g_polarity = gp_cfg.polarity;
            break;

        case IOCTL_PWM_GET_GROUP_CFG:
            if (copy_from_user(&gp_cfg, (struct pwm_gp_cfg __user *)arg, sizeof(struct pwm_gp_cfg)))
            {
                return -EFAULT;
            }

            pwm_gp_dev    = (struct pwm_group *)container_of(file->f_op, struct pwm_group, pwm_fops);
            gp_cfg.enable = hal_pwm_get_group_status(pwm_gp_dev->addr_base, pwm_gp_dev->group)
                            | !(hal_pwm_get_group_reset(pwm_gp_dev->addr_base, pwm_gp_dev->group));

            gp_cfg.duty      = pwm_gp_dev->g_duty;
            gp_cfg.shift     = pwm_gp_dev->g_shift;
            gp_cfg.period    = pwm_gp_dev->g_period;
            gp_cfg.polarity  = pwm_gp_dev->g_polarity;
            gp_cfg.stop_en   = hal_pwm_get_stop_mode_status(pwm_gp_dev->addr_base, pwm_gp_dev->group);
            gp_cfg.round_num = hal_pwm_get_round_num(pwm_gp_dev->addr_base, pwm_gp_dev->group);

            if (copy_to_user((struct pwm_gp_cfg __user *)arg, &gp_cfg, sizeof(struct pwm_gp_cfg)))
            {
                return -EFAULT;
            }
            break;

        case IOCTL_PWM_GROUP_STOP:
            if (copy_from_user(&gp_cfg, (struct pwm_gp_cfg __user *)arg, sizeof(struct pwm_gp_cfg)))
            {
                return -EFAULT;
            }

            pwm_gp_dev = (struct pwm_group *)container_of(file->f_op, struct pwm_group, pwm_fops);
            if (gp_cfg.stop_en)
            {
                hal_pwm_stop_mode_enable(pwm_gp_dev->addr_base, pwm_gp_dev->group, 1);
            }
            else
            {
                hal_pwm_stop_mode_enable(pwm_gp_dev->addr_base, pwm_gp_dev->group, 0);
                hal_pwm_set_group_reset(pwm_gp_dev->addr_base, pwm_gp_dev->group, 1);
                hal_pwm_group_enbale(pwm_gp_dev->addr_base, pwm_gp_dev->group, 1);
                hal_pwm_set_group_reset(pwm_gp_dev->addr_base, pwm_gp_dev->group, 0);
            }
            break;

        case IOCTL_PWM_GROUP_ROUND:
            if (copy_from_user(&gp_cfg, (struct pwm_gp_cfg __user *)arg, sizeof(struct pwm_gp_cfg)))
            {
                return -EFAULT;
            }

            pwm_gp_dev = (struct pwm_group *)container_of(file->f_op, struct pwm_group, pwm_fops);
            if (hal_pwm_get_round_num(pwm_gp_dev->addr_base, pwm_gp_dev->group))
            {
                pwm_err("group%u round number was set up already\n", pwm_gp_dev->group);
                return -EINVAL;
            }

            if (gp_cfg.round_num > 0)
            {
                hal_pwm_set_group_reset(pwm_gp_dev->addr_base, pwm_gp_dev->group, 1);
                hal_pwm_set_round_num(pwm_gp_dev->addr_base, pwm_gp_dev->group, gp_cfg.round_num);
                hal_pwm_group_enbale(pwm_gp_dev->addr_base, pwm_gp_dev->group, 1);
                hal_pwm_set_group_reset(pwm_gp_dev->addr_base, pwm_gp_dev->group, 0);
            }
            else
            {
                pwm_err("group%u round number should be greater than 0!\n", pwm_gp_dev->group);
                return -EINVAL;
            }
            break;

        default:
            pwm_err("unknown command\n");
            return -EINVAL;
    }
    return 0;
}

#ifdef CONFIG_SSTAR_PWM_DDT
static ssize_t hw_break_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    int                 ret;
    u8                  bk_pol;
    u8                  enable;
    struct pwm_channel *pwm_ch_dev = (struct pwm_channel *)dev_get_drvdata(dev);

    ret = sscanf(buf, "%hhu %hhu", &bk_pol, &enable);
    if (ret != 2)
    {
        pwm_err("correct format: echo [break pol] [enable] > hw_break\n");
        return -EINVAL;
    }

    hal_pwmddt_break_pol(pwm_ch_dev->pwm_cfg, bk_pol);
    hal_pwmddt_hw_break_en(pwm_ch_dev->pwm_cfg, enable);

    return count;
}
static DEVICE_ATTR_WO(hw_break);

static ssize_t ddt_en_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    u32                 ret;
    const char *        status     = "unknown";
    struct pwm_channel *pwm_ch_dev = (struct pwm_channel *)dev_get_drvdata(dev);

    ret = pwm_ch_dev->pwm_cfg->ddt_attr.enable;

    switch (ret)
    {
        case 0:
            status = "disable";
            break;

        case 1:
            status = "enable";
            break;
    }

    return sprintf(buf, "%s\n", status);
}

static ssize_t ddt_en_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    int                 ret;
    u8                  enable;
    struct pwm_channel *pwm_ch_dev = (struct pwm_channel *)dev_get_drvdata(dev);

    ret = kstrtou8(buf, 0, &enable);
    if (ret)
    {
        pwm_err("correct format: echo [1/0] > ddt_en\n");
        return ret;
    }

    ret = hal_pwm_get_channel_reset(pwm_ch_dev->pwm_cfg);
    if (ret)
    {
        pwm_err("pwm should output waveform first\n");
        return -EINVAL;
    }

    if (enable)
        hal_pwmddt_hw_break_en(pwm_ch_dev->pwm_cfg, 0);

    hal_pwmddt_enable(pwm_ch_dev->pwm_cfg, HAL_PWM_DDT_OUTP, enable);
    hal_pwmddt_enable(pwm_ch_dev->pwm_cfg, HAL_PWM_DDT_OUTN, enable);
    hal_pwmddt_sync_trig(pwm_ch_dev->pwm_cfg, enable);

    return count;
}
static DEVICE_ATTR_RW(ddt_en);

static ssize_t ddt_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    char *              str        = buf;
    char *              end        = buf + PAGE_SIZE;
    struct pwm_channel *pwm_ch_dev = (struct pwm_channel *)dev_get_drvdata(dev);

    str += scnprintf(str, end - str,
                     "p_ddt  :   %llu\n"
                     "n_ddt  :   %llu\n",
                     pwm_ch_dev->pwm_cfg->ddt_attr.p_ddt, pwm_ch_dev->pwm_cfg->ddt_attr.n_ddt);

    return (str - buf);
}

static ssize_t ddt_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    int                 ret;
    u64                 p_ddt;
    u64                 n_ddt;
    struct pwm_channel *pwm_ch_dev = (struct pwm_channel *)dev_get_drvdata(dev);

    ret = sscanf(buf, "%llu %llu", &p_ddt, &n_ddt);
    if (ret != 2)
    {
        pwm_err("correct format: echo [p_ddt] [n_ddt] > ddt\n");
        return -EINVAL;
    }

    hal_pwmddt_cnt(pwm_ch_dev->pwm_cfg, HAL_PWM_DDT_OUTP, p_ddt);
    hal_pwmddt_cnt(pwm_ch_dev->pwm_cfg, HAL_PWM_DDT_OUTN, n_ddt);

    return count;
}
static DEVICE_ATTR_RW(ddt);

static int sstar_pwm_ddt_config(struct platform_device *pdev, struct pwm_channel *pwm_ch_dev)
{
    int              ret;
    u32              idle_sta[2] = {0, 0};
    struct resource *res         = NULL;

    res = platform_get_resource(pdev, IORESOURCE_MEM, 1);
    if (!res->start)
    {
        pwm_err("fail to get a resource from pwm ddt device\n");
        return PTR_ERR(res);
    }
    else
    {
        pwm_ch_dev->pwm_cfg->ddt_base = IO_ADDRESS(res->start);
    }

    ret = of_property_read_u32_array(pdev->dev.of_node, "idle-status", idle_sta, 2);
    if (ret < 0)
    {
        pwm_dbg("pwm%u ddt idle status it the default value of low level\n", pwm_ch_dev->channel);
    }
    pwm_ch_dev->pwm_cfg->ddt_attr.p_idle_sta = (u8)idle_sta[0];
    pwm_ch_dev->pwm_cfg->ddt_attr.n_idle_sta = (u8)idle_sta[1];

    ret = device_create_file(pwm_ch_dev->dev, &dev_attr_ddt);
    if (ret)
    {
        pwm_err("fail to add pwm%u ddt sysfs file\n", pwm_ch_dev->channel);
        return ret;
    }

    ret = device_create_file(pwm_ch_dev->dev, &dev_attr_ddt_en);
    if (ret)
    {
        pwm_err("fail to add pwm%u ddt enable sysfs file\n", pwm_ch_dev->channel);
        return ret;
    }

    ret = device_create_file(pwm_ch_dev->dev, &dev_attr_hw_break);
    if (ret)
    {
        pwm_err("fail to add pwm%u ddt hw break sysfs file\n", pwm_ch_dev->channel);
        return ret;
    }
    hal_pwmddt_init(pwm_ch_dev->pwm_cfg);

    return 0;
}
#endif

static void sstar_pwm_dev_release(struct device *dev)
{
    kfree(dev);
}

#ifdef CONFIG_SSTAR_SPWM
static long sstar_spwm_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    u8                  i;
    int                 ret = 0;
    u32                 status;
    u8                  tmp_len;
    u8                  tmp_step;
    u8                  tmp_mode;
    struct spwm_gp_cfg  gp_cfg;
    struct pwm_channel *pos_ch;
    struct pwm_group *  spwm_gp_dev = NULL;

    if ((SPWM_IOC_MAGIC != _IOC_TYPE(cmd)) || (_IOC_NR(cmd) > SPWM_IOC_MAXNR))
    {
        return -ENOTTY;
    }

    if (_IOC_DIR(cmd) & _IOC_READ)
    {
        ret = !access_ok((void __user *)arg, _IOC_SIZE(cmd));
    }
    else if (_IOC_DIR(cmd) & _IOC_WRITE)
    {
        ret = !access_ok((void __user *)arg, _IOC_SIZE(cmd));
    }
    if (ret)
    {
        return -EFAULT;
    }

    switch (cmd)
    {
        case IOCTL_SPWM_SET_GROUP_CFG:
        {
            if (copy_from_user(&gp_cfg, (struct spwm_gp_cfg __user *)arg, sizeof(struct spwm_gp_cfg)))
            {
                return -EFAULT;
            }

            spwm_gp_dev = (struct pwm_group *)container_of(file->f_op, struct pwm_group, pwm_fops);

            if (!gp_cfg.enable)
            {
                hal_pwm_group_enbale(spwm_gp_dev->addr_base, spwm_gp_dev->spwm_gp, 0);
                list_for_each_entry(pos_ch, &spwm_gp_dev->channel_head, channel_node)
                {
                    if (pos_ch->group == spwm_gp_dev->group)
                    {
                        hal_pwm_set_channel_reset(pos_ch->pwm_cfg, 1);
                        if (pos_ch->clock_en)
                        {
                            clk_disable_unprepare(pos_ch->pwm_clk);
                            pos_ch->clock_en--;
                        }
                    }
                }
                break;
            }

            if (gp_cfg.clamp_en)
            {
                list_for_each_entry(pos_ch, &spwm_gp_dev->channel_head, channel_node)
                {
                    if (pos_ch->spwm_gp == spwm_gp_dev->spwm_gp)
                    {
                        hal_pwm_dben(pos_ch->pwm_cfg, 0);
                        hal_spwm_clamp_enable(spwm_gp_dev->addr_base, spwm_gp_dev->spwm_gp, 1);
                        hal_spwm_clamp_config(pos_ch->pwm_cfg, gp_cfg.period, gp_cfg.clamp_max, gp_cfg.clamp_min);
                        hal_pwm_dben(pos_ch->pwm_cfg, 1);
                    }
                }
            }

            ret = hal_spwm_pointer_config(spwm_gp_dev->addr_base, spwm_gp_dev->spwm_gp, gp_cfg.mode, gp_cfg.duty_len);
            if (ret)
                return -EINVAL;

            hal_spwm_vdben_sw_en(spwm_gp_dev->addr_base, spwm_gp_dev->spwm_gp, 0);
            list_for_each_entry(pos_ch, &spwm_gp_dev->channel_head, channel_node)
            {
                if (pos_ch->spwm_gp == spwm_gp_dev->spwm_gp)
                {
                    ret = hal_spwm_set_init_pointer(pos_ch->pwm_cfg, gp_cfg.step);
                    if (ret)
                        return -EINVAL;

                    hal_spwm_config_mode(pos_ch->pwm_cfg, 1);

                    ret = hal_pwm_set_period(pos_ch->pwm_cfg, gp_cfg.period, 0);
                    if (ret)
                        return -EINVAL;
                }
            }
            spwm_gp_dev->s_step   = gp_cfg.step;
            spwm_gp_dev->s_period = gp_cfg.period;

            for (i = 0; i < gp_cfg.duty_len; i++)
            {
                ret =
                    hal_spwm_sram_duty(spwm_gp_dev->addr_base, spwm_gp_dev->spwm_gp, gp_cfg.period, i, gp_cfg.duty[i]);
                if (ret)
                    return -EINVAL;
                spwm_gp_dev->s_duty_ns[i] = gp_cfg.duty[i];
            }
            spwm_gp_dev->s_duty_len = gp_cfg.duty_len;

            if (gp_cfg.scale)
                hal_spwm_scale_config(spwm_gp_dev->addr_base, spwm_gp_dev->spwm_gp, (gp_cfg.scale - 1));

            list_for_each_entry(pos_ch, &spwm_gp_dev->channel_head, channel_node)
            {
                if (pos_ch->group == spwm_gp_dev->group)
                {
                    if (!pos_ch->clock_en)
                    {
                        ret = clk_prepare_enable(pos_ch->pwm_clk);
                        if (ret)
                        {
                            pwm_err("fail to enable pwm%u clk\n", pos_ch->channel);
                            return ret;
                        }
                        pos_ch->clock_en++;
                    }
                    hal_pwm_set_channel_reset(pos_ch->pwm_cfg, 0);
                }
            }
            hal_spwm_vdben_sw_en(spwm_gp_dev->addr_base, spwm_gp_dev->spwm_gp, 1);
            CamOsUsDelay(1000);
            hal_pwm_group_enbale(spwm_gp_dev->addr_base, spwm_gp_dev->spwm_gp, 1);
            spwm_gp_dev->s_init = 1;
            break;
        }

        case IOCTL_SPWM_GET_GROUP_CFG:
        {
            if (copy_from_user(&gp_cfg, (struct spwm_gp_cfg __user *)arg, sizeof(struct spwm_gp_cfg)))
            {
                return -EFAULT;
            }

            spwm_gp_dev = (struct pwm_group *)container_of(file->f_op, struct pwm_group, pwm_fops);
            hal_spwm_get_pointer_config(spwm_gp_dev->addr_base, spwm_gp_dev->spwm_gp, &gp_cfg.mode, &gp_cfg.duty_len);
            for (i = 0; i < gp_cfg.duty_len; i++)
            {
                gp_cfg.duty[i] = spwm_gp_dev->s_duty_ns[i];
            }
            gp_cfg.step     = spwm_gp_dev->s_step;
            gp_cfg.period   = spwm_gp_dev->s_period;
            gp_cfg.enable   = hal_pwm_get_group_status(spwm_gp_dev->addr_base, spwm_gp_dev->spwm_gp);
            gp_cfg.scale    = hal_spwm_get_scale(spwm_gp_dev->addr_base, spwm_gp_dev->spwm_gp);
            gp_cfg.clamp_en = hal_spwm_get_clamp_status(spwm_gp_dev->addr_base, spwm_gp_dev->spwm_gp);
            hal_spwm_get_clamp_config(spwm_gp_dev->addr_base, spwm_gp_dev->spwm_gp, spwm_gp_dev->s_period,
                                      &gp_cfg.clamp_max, &gp_cfg.clamp_min);
            gp_cfg.stop_en   = hal_pwm_get_stop_mode_status(spwm_gp_dev->addr_base, spwm_gp_dev->spwm_gp);
            gp_cfg.round_num = hal_pwm_get_round_num(spwm_gp_dev->addr_base, spwm_gp_dev->spwm_gp);

            if (copy_to_user((struct spwm_gp_cfg __user *)arg, &gp_cfg, sizeof(struct spwm_gp_cfg)))
            {
                return -EFAULT;
            }
            break;
        }

        case IOCTL_SPWM_UPDATE_PERIOD:
        {
            if (copy_from_user(&gp_cfg, (struct spwm_gp_cfg __user *)arg, sizeof(struct spwm_gp_cfg)))
            {
                return -EFAULT;
            }

            spwm_gp_dev = (struct pwm_group *)container_of(file->f_op, struct pwm_group, pwm_fops);
            if (!spwm_gp_dev->s_init)
            {
                pwm_err("fail to update spwm%u period, must be configured first\n", spwm_gp_dev->spwm_gp);
                return -EINVAL;
            }
            else
            {
                spwm_gp_dev->s_period = gp_cfg.period;
                hal_spwm_periods_int_en(spwm_gp_dev->addr_base, spwm_gp_dev->spwm_gp, 1);
            }

            break;
        }

        case IOCTL_SPWM_UPDATE_DUTY:
        {
            if (copy_from_user(&gp_cfg, (struct spwm_gp_cfg __user *)arg, sizeof(struct spwm_gp_cfg)))
            {
                return -EFAULT;
            }

            spwm_gp_dev = (struct pwm_group *)container_of(file->f_op, struct pwm_group, pwm_fops);
            if (!spwm_gp_dev->s_init)
            {
                pwm_err("fail to update spwm%u duty, must be configured first\n", spwm_gp_dev->spwm_gp);
                return -EINVAL;
            }
            else
            {
                spwm_gp_dev->s_duty_idx = gp_cfg.index;
                spwm_gp_dev->s_duty_len = gp_cfg.duty_len;
                for (i = 0; i < gp_cfg.duty_len; i++)
                {
                    spwm_gp_dev->s_duty_ns[i] = gp_cfg.duty[i];
                }
                hal_spwm_sram_data_int_clear(spwm_gp_dev->addr_base, spwm_gp_dev->spwm_gp);
                hal_spwm_sram_data_mody_cnt(spwm_gp_dev->addr_base, spwm_gp_dev->spwm_gp, 0xF);
                hal_spwm_sram_data_int_en(spwm_gp_dev->addr_base, spwm_gp_dev->spwm_gp, 1);
            }

            break;
        }

        case IOCTL_SPWM_GROUP_STOP:
        {
            if (copy_from_user(&gp_cfg, (struct spwm_gp_cfg __user *)arg, sizeof(struct spwm_gp_cfg)))
            {
                return -EFAULT;
            }

            spwm_gp_dev = (struct pwm_group *)container_of(file->f_op, struct pwm_group, pwm_fops);
            if (gp_cfg.stop_en)
            {
                hal_pwm_stop_mode_enable(spwm_gp_dev->addr_base, spwm_gp_dev->spwm_gp, 1);
            }
            else
            {
                hal_pwm_stop_mode_enable(spwm_gp_dev->addr_base, spwm_gp_dev->spwm_gp, 0);
                hal_pwm_group_enbale(spwm_gp_dev->addr_base, spwm_gp_dev->spwm_gp, 0);
                hal_pwm_group_enbale(spwm_gp_dev->addr_base, spwm_gp_dev->spwm_gp, 1);
            }

            break;
        }

        case IOCTL_SPWM_GROUP_ROUND:
        {
            if (copy_from_user(&gp_cfg, (struct spwm_gp_cfg __user *)arg, sizeof(struct spwm_gp_cfg)))
            {
                return -EFAULT;
            }

            spwm_gp_dev = (struct pwm_group *)container_of(file->f_op, struct pwm_group, pwm_fops);
            status      = hal_pwm_get_round_num(spwm_gp_dev->addr_base, spwm_gp_dev->spwm_gp);
            if (status)
            {
                pwm_err("spwm group%u round number was set up already!\n", spwm_gp_dev->spwm_gp);
                return -EINVAL;
            }

            if (gp_cfg.round_num > 0)
            {
                hal_pwm_group_enbale(spwm_gp_dev->addr_base, spwm_gp_dev->spwm_gp, 0);
                hal_spwm_get_pointer_config(spwm_gp_dev->addr_base, spwm_gp_dev->spwm_gp, &tmp_mode, &tmp_len);
                switch (tmp_mode)
                {
                    case HAL_SPWM_SYMM_HALF:
                        tmp_len = tmp_len * 4 - 1;
                        break;
                    case HAL_SPWM_HALF_WAVE:
                        tmp_len = tmp_len * 2 - 1;
                        break;
                    case HAL_SPWM_FULL_WAVE:
                        tmp_len = tmp_len - 1;
                        break;
                }
                list_for_each_entry(pos_ch, &spwm_gp_dev->channel_head, channel_node)
                {
                    if (pos_ch->spwm_gp == spwm_gp_dev->spwm_gp)
                    {
                        tmp_step = hal_spwm_get_init_pointer(pos_ch->pwm_cfg);
                        if (!tmp_step)
                        {
                            tmp_step = tmp_len;
                        }
                        else
                        {
                            tmp_step = tmp_step - 1;
                        }
                        ret = hal_spwm_set_init_pointer(pos_ch->pwm_cfg, tmp_step);
                        if (ret)
                            return -EINVAL;
                    }
                }
                hal_pwm_set_round_num(spwm_gp_dev->addr_base, spwm_gp_dev->spwm_gp, gp_cfg.round_num);
                hal_pwm_group_enbale(spwm_gp_dev->addr_base, spwm_gp_dev->spwm_gp, 1);
                hal_pwm_set_group_reset(spwm_gp_dev->addr_base, spwm_gp_dev->spwm_gp, 0);
            }
            else
            {
                hal_pwm_set_group_reset(spwm_gp_dev->addr_base, spwm_gp_dev->spwm_gp, 0);
            }

            break;
        }

        default:
            pwm_err("unknown command\n");
            return -EINVAL;
    }
    return 0;
}

static ssize_t duty_show_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    struct pwm_group *spwm_gp_dev = (struct pwm_group *)dev_get_drvdata(dev);

    return sprintf(buf, "%llu\n", spwm_gp_dev->s_duty_ns[spwm_gp_dev->s_duty_idx]);
}

static ssize_t duty_show_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    int               ret;
    u8                index;
    struct pwm_group *spwm_gp_dev = (struct pwm_group *)dev_get_drvdata(dev);

    ret = kstrtou8(buf, 0, &index);
    if (ret)
    {
        pwm_err("correct format: echo [index] > duty_show\n");
        return ret;
    }

    spwm_gp_dev->s_duty_idx = index;

    return count;
}
static DEVICE_ATTR_RW(duty_show);

static ssize_t sg_stop_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    u32               ret;
    const char *      stop        = "unknown";
    struct pwm_group *spwm_gp_dev = (struct pwm_group *)dev_get_drvdata(dev);

    ret = hal_pwm_get_stop_mode_status(spwm_gp_dev->addr_base, spwm_gp_dev->spwm_gp);

    switch (ret)
    {
        case 0:
            stop = "disable";
            break;

        case 1:
            stop = "enbale";
            break;
    }

    return sprintf(buf, "%s\n", stop);
}

static ssize_t sg_stop_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    int ret;
    u8  value;

    struct pwm_group *spwm_gp_dev = (struct pwm_group *)dev_get_drvdata(dev);

    ret = kstrtou8(buf, 0, &value);
    if (ret)
    {
        pwm_err("correct format: echo [enable] > stop\n");
        return ret;
    }

    if (value)
    {
        hal_pwm_stop_mode_enable(spwm_gp_dev->addr_base, spwm_gp_dev->spwm_gp, 1);
    }
    else
    {
        hal_pwm_stop_mode_enable(spwm_gp_dev->addr_base, spwm_gp_dev->spwm_gp, 0);
        hal_pwm_group_enbale(spwm_gp_dev->addr_base, spwm_gp_dev->spwm_gp, 0);
        hal_pwm_group_enbale(spwm_gp_dev->addr_base, spwm_gp_dev->spwm_gp, 1);
    }

    return count;
}
static DEVICE_ATTR_RW(sg_stop);

static ssize_t sg_round_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    u32               round;
    struct pwm_group *spwm_gp_dev = (struct pwm_group *)dev_get_drvdata(dev);

    round = hal_pwm_get_round_num(spwm_gp_dev->addr_base, spwm_gp_dev->spwm_gp);

    return sprintf(buf, "%u\n", round);
}

static ssize_t sg_round_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    int                 ret;
    struct pwm_channel *pos_ch;
    u32                 status;
    u8                  tmp_len;
    u8                  tmp_step;
    u8                  tmp_mode;
    u16                 round_num;
    struct pwm_group *  spwm_gp_dev = (struct pwm_group *)dev_get_drvdata(dev);

    ret = kstrtou16(buf, 0, &round_num);
    if (ret)
    {
        pwm_err("correct format: echo [round_num] > round\n");
        return ret;
    }

    status = hal_pwm_get_round_num(spwm_gp_dev->addr_base, spwm_gp_dev->spwm_gp);
    if (status)
    {
        pwm_err("spwm group%u round number was set up already!\n", spwm_gp_dev->spwm_gp);
        return -EINVAL;
    }

    if (round_num > 0)
    {
        hal_pwm_group_enbale(spwm_gp_dev->addr_base, spwm_gp_dev->spwm_gp, 0);
        hal_spwm_get_pointer_config(spwm_gp_dev->addr_base, spwm_gp_dev->spwm_gp, &tmp_mode, &tmp_len);
        switch (tmp_mode)
        {
            case HAL_SPWM_SYMM_HALF:
                tmp_len = tmp_len * 4 - 1;
                break;
            case HAL_SPWM_HALF_WAVE:
                tmp_len = tmp_len * 2 - 1;
                break;
            case HAL_SPWM_FULL_WAVE:
                tmp_len = tmp_len - 1;
                break;
        }
        list_for_each_entry(pos_ch, &spwm_gp_dev->channel_head, channel_node)
        {
            if (pos_ch->spwm_gp == spwm_gp_dev->spwm_gp)
            {
                tmp_step = hal_spwm_get_init_pointer(pos_ch->pwm_cfg);
                if (!tmp_step)
                {
                    tmp_step = tmp_len;
                }
                else
                {
                    tmp_step = tmp_step - 1;
                }
                ret = hal_spwm_set_init_pointer(pos_ch->pwm_cfg, tmp_step);
                if (ret)
                    return -EINVAL;
            }
        }
        hal_pwm_set_round_num(spwm_gp_dev->addr_base, spwm_gp_dev->spwm_gp, round_num);
        hal_pwm_group_enbale(spwm_gp_dev->addr_base, spwm_gp_dev->spwm_gp, 1);
        hal_pwm_set_group_reset(spwm_gp_dev->addr_base, spwm_gp_dev->spwm_gp, 0);
    }
    else
    {
        hal_pwm_set_group_reset(spwm_gp_dev->addr_base, spwm_gp_dev->spwm_gp, 0);
    }

    return count;
}
static DEVICE_ATTR_RW(sg_round);

static ssize_t sg_scale_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    u8 scale;

    struct pwm_group *spwm_gp_dev = (struct pwm_group *)dev_get_drvdata(dev);

    scale = hal_spwm_get_scale(spwm_gp_dev->addr_base, spwm_gp_dev->spwm_gp);

    return sprintf(buf, "%hhu\n", scale);
}

static ssize_t sg_scale_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    int ret;
    u8  scale;

    struct pwm_group *spwm_gp_dev = (struct pwm_group *)dev_get_drvdata(dev);

    ret = kstrtou8(buf, 0, &scale);
    if ((ret) || (!scale))
    {
        pwm_err("correct format: echo [1~8] > scale\n");
        return ret;
    }

    hal_spwm_scale_config(spwm_gp_dev->addr_base, spwm_gp_dev->spwm_gp, (scale - 1));

    return count;
}
static DEVICE_ATTR_RW(sg_scale);

static ssize_t sg_clamp_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    u64               upper;
    u64               lower;
    char *            str         = buf;
    char *            end         = buf + PAGE_SIZE;
    struct pwm_group *spwm_gp_dev = (struct pwm_group *)dev_get_drvdata(dev);

    if (!spwm_gp_dev->s_period)
    {
        pwm_err("spwm preriod must be configured first\n");
        goto out;
    }

    hal_spwm_get_clamp_config(spwm_gp_dev->addr_base, spwm_gp_dev->spwm_gp, spwm_gp_dev->s_period, &upper, &lower);

    str += scnprintf(str, end - str,
                     "upper limit  :   %llu\n"
                     "lower limit  :   %llu\n",
                     upper, lower);
out:
    return (str - buf);
}

static ssize_t sg_clamp_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    int                 ret;
    u64                 upper;
    u64                 lower;
    struct pwm_channel *pos_ch      = NULL;
    struct pwm_group *  spwm_gp_dev = (struct pwm_group *)dev_get_drvdata(dev);

    ret = sscanf(buf, "%llu %llu", &upper, &lower);
    if (ret != 2)
    {
        pwm_err("invalid argument (upper limit, lower limit)\n");
        goto out;
    }

    if (!spwm_gp_dev->s_period)
    {
        pwm_err("spwm preriod must be configured first\n");
        goto out;
    }

    list_for_each_entry(pos_ch, &spwm_gp_dev->channel_head, channel_node)
    {
        if (pos_ch->spwm_gp == spwm_gp_dev->spwm_gp)
        {
            hal_pwm_dben(pos_ch->pwm_cfg, 0);
            hal_spwm_clamp_enable(spwm_gp_dev->addr_base, spwm_gp_dev->spwm_gp, 1);
            hal_spwm_clamp_config(pos_ch->pwm_cfg, spwm_gp_dev->s_period, upper, lower);
            hal_pwm_dben(pos_ch->pwm_cfg, 1);
        }
    }

out:
    return count;
}
static DEVICE_ATTR_RW(sg_clamp);

static ssize_t sg_duty_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    int               ret;
    u8                index;
    u64               duty;
    struct pwm_group *spwm_gp_dev = (struct pwm_group *)dev_get_drvdata(dev);

    ret = sscanf(buf, "%hhu %llu", &index, &duty);
    if (ret != 2)
    {
        pwm_err("correct format 2: echo [index] [duty ns] > sg_duty\n");
        return -EINVAL;
    }

    ret = hal_spwm_sram_duty(spwm_gp_dev->addr_base, spwm_gp_dev->spwm_gp, spwm_gp_dev->s_period, index, duty);
    if (!ret)
        spwm_gp_dev->s_duty_ns[index] = duty;

    return count;
}
static DEVICE_ATTR_WO(sg_duty);

static ssize_t sg_enable_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    u32               ret;
    const char *      status      = "unknown";
    struct pwm_group *spwm_gp_dev = (struct pwm_group *)dev_get_drvdata(dev);

    ret = hal_pwm_get_group_status(spwm_gp_dev->addr_base, spwm_gp_dev->spwm_gp);

    switch (ret)
    {
        case 0:
            status = "disable";
            break;

        case 1:
            status = "enable";
            break;
    }

    return sprintf(buf, "%s\n", status);
}

static ssize_t sg_enable_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    int                 ret;
    u8                  value;
    struct pwm_channel *pos_ch      = NULL;
    struct pwm_group *  spwm_gp_dev = (struct pwm_group *)dev_get_drvdata(dev);

    ret = kstrtou8(buf, 0, &value);
    if (ret)
    {
        pwm_err("correct format: echo [enable] > sg_enable\n");
        return ret;
    }

    if (value)
    {
        list_for_each_entry(pos_ch, &spwm_gp_dev->channel_head, channel_node)
        {
            if (pos_ch->group == spwm_gp_dev->group)
            {
                if (!pos_ch->clock_en)
                {
                    ret = clk_prepare_enable(pos_ch->pwm_clk);
                    if (ret)
                    {
                        pwm_err("fail to enable pwm%u clk\n", pos_ch->channel);
                        return ret;
                    }
                    pos_ch->clock_en++;
                }
                hal_pwm_set_channel_reset(pos_ch->pwm_cfg, 0);
            }
        }
        hal_spwm_vdben_sw_en(spwm_gp_dev->addr_base, spwm_gp_dev->spwm_gp, 1);
        CamOsUsDelay(1000);
        hal_pwm_group_enbale(spwm_gp_dev->addr_base, spwm_gp_dev->spwm_gp, 1);
    }
    else
    {
        hal_pwm_group_enbale(spwm_gp_dev->addr_base, spwm_gp_dev->spwm_gp, 0);
        list_for_each_entry(pos_ch, &spwm_gp_dev->channel_head, channel_node)
        {
            if (pos_ch->group == spwm_gp_dev->group)
            {
                hal_pwm_set_channel_reset(pos_ch->pwm_cfg, 1);
                if (pos_ch->clock_en)
                {
                    clk_disable_unprepare(pos_ch->pwm_clk);
                    pos_ch->clock_en--;
                }
            }
        }
    }

    return count;
}
static DEVICE_ATTR_RW(sg_enable);

static ssize_t sg_step_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    struct pwm_group *spwm_gp_dev = (struct pwm_group *)dev_get_drvdata(dev);

    return sprintf(buf, "%hhu\n", spwm_gp_dev->s_step);
}

static ssize_t sg_step_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    int                 ret;
    u8                  step;
    u8                  channel;
    u8                  arg[2];
    struct pwm_channel *pos_ch      = NULL;
    struct pwm_group *  spwm_gp_dev = (struct pwm_group *)dev_get_drvdata(dev);

    ret = sscanf(buf, "%hhu %hhu", &arg[0], &arg[1]);
    if (ret == 1)
    {
        channel = 0xFF;
        step    = arg[0];
    }
    else if (ret == 2)
    {
        channel = arg[0];
        step    = arg[1];
    }
    else
    {
        pwm_err("correct format 1: echo [step] > sg_step\n");
        pwm_err("correct format 2: echo [channel] [step] > sg_step\n");
        return -EINVAL;
    }

    switch (ret)
    {
        case 1:
            list_for_each_entry(pos_ch, &spwm_gp_dev->channel_head, channel_node)
            {
                if (pos_ch->spwm_gp == spwm_gp_dev->spwm_gp)
                {
                    hal_spwm_set_init_pointer(pos_ch->pwm_cfg, step);
                    hal_spwm_config_mode(pos_ch->pwm_cfg, 1);
                }
            }
            spwm_gp_dev->s_step = step;
            break;

        case 2:
            list_for_each_entry(pos_ch, &spwm_gp_dev->channel_head, channel_node)
            {
                if ((pos_ch->spwm_gp == spwm_gp_dev->spwm_gp) && (pos_ch->channel == channel))
                {
                    hal_spwm_set_init_pointer(pos_ch->pwm_cfg, step);
                    hal_spwm_config_mode(pos_ch->pwm_cfg, 1);
                }
            }
            break;
    }

    return count;
}
static DEVICE_ATTR_RW(sg_step);

static ssize_t sg_period_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    struct pwm_group *spwm_gp_dev = (struct pwm_group *)dev_get_drvdata(dev);

    return sprintf(buf, "%llu\n", spwm_gp_dev->s_period);
}

static ssize_t sg_period_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    int                 ret;
    u64                 period;
    struct pwm_channel *pos_ch      = NULL;
    struct pwm_group *  spwm_gp_dev = (struct pwm_group *)dev_get_drvdata(dev);

    ret = kstrtou64(buf, 0, &period);
    if (ret)
    {
        pwm_err("correct format: echo [period ns] > sg_period\n");
        return ret;
    }

    if (!spwm_gp_dev->s_init)
    {
        hal_spwm_vdben_sw_en(spwm_gp_dev->addr_base, spwm_gp_dev->spwm_gp, 0);
        list_for_each_entry(pos_ch, &spwm_gp_dev->channel_head, channel_node)
        {
            if (pos_ch->spwm_gp == spwm_gp_dev->spwm_gp)
            {
                ret = hal_pwm_set_period(pos_ch->pwm_cfg, period, 0);
                if (ret)
                    return count;
            }
        }
        hal_spwm_vdben_sw_en(spwm_gp_dev->addr_base, spwm_gp_dev->spwm_gp, 1);

        spwm_gp_dev->s_init   = 1;
        spwm_gp_dev->s_period = period;
    }
    else
    {
        spwm_gp_dev->s_period = period;
        hal_spwm_periods_int_en(spwm_gp_dev->addr_base, spwm_gp_dev->spwm_gp, 1);
    }

    return count;
}
static DEVICE_ATTR_RW(sg_period);

static ssize_t sg_config_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    u8                mode;
    u8                duty_len;
    char *            str         = buf;
    char *            end         = buf + PAGE_SIZE;
    struct pwm_group *spwm_gp_dev = (struct pwm_group *)dev_get_drvdata(dev);

    hal_spwm_get_pointer_config(spwm_gp_dev->addr_base, spwm_gp_dev->spwm_gp, &mode, &duty_len);
    str += scnprintf(str, end - str,
                     "mode        :   %hhu\n"
                     "duty lenth  :   %hhu\n",
                     mode, duty_len);

    return (str - buf);
}

static ssize_t sg_config_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    int               ret;
    u8                mode;
    u8                duty_len;
    struct pwm_group *spwm_gp_dev = (struct pwm_group *)dev_get_drvdata(dev);

    ret = sscanf(buf, "%hhu %hhu", &mode, &duty_len);
    if (ret != 2)
    {
        pwm_err("invalid argument (spwm mode, duty lenth)\n");
        goto out;
    }

    hal_spwm_pointer_config(spwm_gp_dev->addr_base, spwm_gp_dev->spwm_gp, mode, duty_len);

out:
    return count;
}
static DEVICE_ATTR_RW(sg_config);

static int sstar_spwm_create_group_file(struct device *parent, struct pwm_group *pos)
{
    int            ret;
    char *         group_prop[2];
    struct device *group_dev = NULL;

    struct attribute *func_attrs[] = {
        &dev_attr_sg_config.attr,
        &dev_attr_sg_period.attr,
        &dev_attr_sg_step.attr,
        &dev_attr_sg_enable.attr,
        &dev_attr_sg_duty.attr,
        &dev_attr_sg_clamp.attr,
        &dev_attr_sg_scale.attr,
        &dev_attr_sg_round.attr,
        &dev_attr_sg_stop.attr,
        &dev_attr_duty_show.attr,
        NULL,
    };

    struct attribute_group func_group = {
        .attrs = func_attrs,
    };

    struct attribute_group *func_groups[] = {
        &func_group,
        NULL,
    };

    pwm_dbg("spwm group%u creat device in sysfs\n", pos->spwm_gp);
    group_dev = (struct device *)kzalloc(sizeof(struct device), GFP_KERNEL);
    if (!group_dev)
    {
        pwm_err("spwm group%u device can not be allocated memory\n", pos->spwm_gp);
        return -ENOMEM;
    }

    group_dev->release = sstar_pwm_dev_release;
    group_dev->parent  = parent;
    group_dev->devt    = MKDEV(0, 0);
    group_dev->groups  = (const struct attribute_group **)func_groups;
    dev_set_name(group_dev, "spwm_group%u", pos->spwm_gp);
    dev_set_drvdata(group_dev, (void *)pos);

    ret = device_register(group_dev);
    if (ret)
    {
        pwm_err("spwm group%u device can not register\n", pos->spwm_gp);
        put_device(group_dev);
        return ret;
    }
    pos->dev_data = (void *)group_dev;
    group_prop[0] = kasprintf(GFP_KERNEL, "EXPORT=spwm_group%u", pos->spwm_gp);
    group_prop[1] = NULL;
    kobject_uevent_env(&parent->kobj, KOBJ_CHANGE, group_prop);
    kfree(group_prop[0]);

    return 0;
}

static int sstar_spwm_group_setup(struct platform_device *pdev, struct pwm_channel *pwm_ch_dev)
{
    int               ret;
    char              miscdev_name[16];
    struct pwm_group *pos_gp      = NULL;
    struct pwm_group *spwm_gp_dev = NULL;

    spwm_gp_dev = devm_kzalloc(&pdev->dev, sizeof(struct pwm_group), GFP_KERNEL);
    if (!spwm_gp_dev)
    {
        pwm_err("fail to allocate memory\n");
        return -ENOMEM;
    }

    spwm_gp_dev->spwm_gp = pwm_ch_dev->spwm_gp;
    list_for_each_entry(pos_gp, &pwm_group_list, group_node)
    {
        if (pos_gp->spwm_gp == spwm_gp_dev->spwm_gp)
        {
            pwm_dbg("spwm group%u alreadly in pwm_group_list\n", spwm_gp_dev->spwm_gp);
            goto group_out;
        }
    }

    spwm_gp_dev->pwm_fops.unlocked_ioctl = sstar_spwm_ioctl;
    spwm_gp_dev->pwm_miscdev.minor       = MISC_DYNAMIC_MINOR;
    spwm_gp_dev->pwm_miscdev.fops        = &spwm_gp_dev->pwm_fops;
    if ((snprintf(miscdev_name, sizeof(miscdev_name), "spwm_group%u", spwm_gp_dev->spwm_gp) >= 0))
        spwm_gp_dev->pwm_miscdev.name = (const char *)(char *)miscdev_name;
    ret = misc_register(&spwm_gp_dev->pwm_miscdev);
    if (ret < 0)
    {
        pwm_err("fail to register spwm group%u miscellaneous device\n", spwm_gp_dev->spwm_gp);
        return ret;
    }

    spwm_gp_dev->irq = irq_of_parse_and_map(pdev->dev.of_node, 0);
    if (!spwm_gp_dev->irq)
    {
        pwm_err("fail to parse and map spwm group%u interrupt into linux virq space\n", spwm_gp_dev->spwm_gp);
        misc_deregister(&spwm_gp_dev->pwm_miscdev);
        return -EINVAL;
    }
    snprintf(spwm_gp_dev->irq_name, sizeof(spwm_gp_dev->irq_name), "spwm_group%u", spwm_gp_dev->spwm_gp);
    ret = request_irq(spwm_gp_dev->irq, sstar_pwm_int_handler, IRQF_SHARED, (const char *)spwm_gp_dev->irq_name,
                      spwm_gp_dev);
    if (ret < 0)
    {
        pwm_err("fail to request spwm group%u irq, errno %d\n", spwm_gp_dev->spwm_gp, ret);
        misc_deregister(&spwm_gp_dev->pwm_miscdev);
        return ret;
    }

    pwm_dbg("add spwm group[%u] into pwm_group_list\n", spwm_gp_dev->spwm_gp);
    INIT_LIST_HEAD(&spwm_gp_dev->channel_head);
    INIT_LIST_HEAD(&spwm_gp_dev->group_node);
    spwm_gp_dev->s_init        = 0;
    spwm_gp_dev->s_period      = 0;
    spwm_gp_dev->s_duty_len    = 0;
    spwm_gp_dev->s_duty_idx    = 0xFF;
    spwm_gp_dev->dev_data      = NULL;
    spwm_gp_dev->res           = PWM_RES_REQUEST;
    spwm_gp_dev->sysfs_created = PWM_SYSFS_NO_CREATE;
    spwm_gp_dev->group         = pwm_ch_dev->group;
    spwm_gp_dev->spwm_gp       = pwm_ch_dev->spwm_gp;
    spwm_gp_dev->channel       = pwm_ch_dev->channel;
    spwm_gp_dev->addr_base     = pwm_ch_dev->pwm_cfg->addr_base;

    list_add(&spwm_gp_dev->group_node, &pwm_group_list);
    return 0;

group_out:
    devm_kfree(&pdev->dev, spwm_gp_dev);
    return 0;
}
#endif

static ssize_t info_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    u32                 div;
    u32                 polar;
    u32                 enable;
    u64                 duty;
    u64                 shift;
    u64                 period;
    u64                 duty_cycle;
    char *              str        = buf;
    char *              end        = buf + PAGE_SIZE;
    const char *        polarity   = "unknown";
    struct pwm_channel *pwm_ch_dev = (struct pwm_channel *)dev_get_drvdata(dev);

    div    = hal_pwm_get_div(pwm_ch_dev->pwm_cfg);
    duty   = hal_pwm_get_duty_pct(pwm_ch_dev->pwm_cfg);
    shift  = hal_pwm_get_shift_pct(pwm_ch_dev->pwm_cfg);
    period = hal_pwm_get_period_hz(pwm_ch_dev->pwm_cfg);
    polar  = hal_pwm_get_polarity(pwm_ch_dev->pwm_cfg);
    enable = hal_pwm_get_channel_reset(pwm_ch_dev->pwm_cfg) ? 0 : 1;

    switch (polar)
    {
        case PWM_POLARITY_NORMAL:
            polarity   = "normal";
            duty_cycle = duty - shift;
            break;

        case PWM_POLARITY_INVERSED:
            polarity   = "inversed";
            duty_cycle = (100 - duty) + shift;
            break;
    }

    str += scnprintf(str, end - str, "\t enable     : %u \n", enable);
    if (pwm_ch_dev->group < 0)
        str += scnprintf(str, end - str, "\t group      : no join\n");
    else
        str += scnprintf(str, end - str, "\t group      : %u\n", pwm_ch_dev->group);
    str += scnprintf(str, end - str, "\t div        : %u\n", div++);
    str += scnprintf(str, end - str, "\t duty       : %llu%%\n", duty);
    str += scnprintf(str, end - str, "\t shift      : %llu%%\n", shift);
    str += scnprintf(str, end - str, "\t duty_cycle : %llu%%\n", duty_cycle);
    str += scnprintf(str, end - str, "\t period     : %lluHZ\n", period);
    str += scnprintf(str, end - str, "\t polarity   : %s\n", polarity);

    return (str - buf);
}
static DEVICE_ATTR_RO(info);

static ssize_t enable_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    u32                 ret;
    const char *        status     = "unknown";
    struct pwm_channel *pwm_ch_dev = (struct pwm_channel *)dev_get_drvdata(dev);

    ret = hal_pwm_get_channel_reset(pwm_ch_dev->pwm_cfg);

    switch (ret)
    {
        case 0:
            status = "enable";
            break;

        case 1:
            status = "disable";
            break;
    }

    return sprintf(buf, "%s\n", status);
}

static ssize_t enable_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    u8  enable;
    int ret;

    struct pwm_channel *pwm_ch_dev = (struct pwm_channel *)dev_get_drvdata(dev);

    ret = kstrtou8(buf, 0, &enable);
    if (ret)
    {
        pwm_err("correct format: echo [enable] > enable\n");
        return ret;
    }

    if (enable)
    {
        if (!pwm_ch_dev->clock_en)
        {
            ret = clk_prepare_enable(pwm_ch_dev->pwm_clk);
            if (ret)
            {
                pwm_err("fail to enable pwm%u clk\n", pwm_ch_dev->channel);
                return ret;
            }
            pwm_ch_dev->clock_en++;
        }
        hal_pwm_set_channel_reset(pwm_ch_dev->pwm_cfg, 0);
    }
    else
    {
        hal_pwm_set_channel_reset(pwm_ch_dev->pwm_cfg, 1);
        if (pwm_ch_dev->clock_en)
        {
            clk_disable_unprepare(pwm_ch_dev->pwm_clk);
            pwm_ch_dev->clock_en--;
        }
    }

    return count;
}
static DEVICE_ATTR_RW(enable);

static ssize_t polarity_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    u32                 ret;
    const char *        polarity   = "unknown";
    struct pwm_channel *pwm_ch_dev = (struct pwm_channel *)dev_get_drvdata(dev);

    ret = hal_pwm_get_polarity(pwm_ch_dev->pwm_cfg);

    switch (ret)
    {
        case PWM_POLARITY_NORMAL:
            polarity = "normal";
            break;

        case PWM_POLARITY_INVERSED:
            polarity = "inversed";
            break;
    }

    return sprintf(buf, "%s\n", polarity);
}

static ssize_t polarity_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    enum pwm_polarity polarity;

    struct pwm_channel *pwm_ch_dev = (struct pwm_channel *)dev_get_drvdata(dev);

    if (sysfs_streq(buf, "normal"))
        polarity = PWM_POLARITY_NORMAL;
    else if (sysfs_streq(buf, "inversed"))
        polarity = PWM_POLARITY_INVERSED;
    else
        return -EINVAL;

    hal_pwm_set_polarity(pwm_ch_dev->pwm_cfg, polarity, 0);

    return count;
}
static DEVICE_ATTR_RW(polarity);

static ssize_t shift_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    struct pwm_channel *pwm_ch_dev = (struct pwm_channel *)dev_get_drvdata(dev);

    return sprintf(buf, "%llu\n", pwm_ch_dev->pwm_cfg->pwm_attr.shift_wf);
}

static ssize_t shift_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    int ret;
    u64 shift;

    struct pwm_channel *pwm_ch_dev = (struct pwm_channel *)dev_get_drvdata(dev);

    ret = kstrtou64(buf, 0, &shift);
    if (ret)
    {
        pwm_err("correct format: echo [shift] > shift\n");
        return ret;
    }

    hal_pmw_set_shift(pwm_ch_dev->pwm_cfg, shift, 0);

    return count;
}
static DEVICE_ATTR_RW(shift);

static ssize_t duty_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    struct pwm_channel *pwm_ch_dev = (struct pwm_channel *)dev_get_drvdata(dev);

    return sprintf(buf, "%llu\n", pwm_ch_dev->pwm_cfg->pwm_attr.duty_wf);
}

static ssize_t duty_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    int ret;
    u64 duty;

    struct pwm_channel *pwm_ch_dev = (struct pwm_channel *)dev_get_drvdata(dev);

    ret = kstrtou64(buf, 0, &duty);
    if (ret)
    {
        pwm_err("correct format: echo [duty] > duty\n");
        return ret;
    }

    hal_pwm_set_duty(pwm_ch_dev->pwm_cfg, duty, 0);

    return count;
}
static DEVICE_ATTR_RW(duty);

static ssize_t period_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    struct pwm_channel *pwm_ch_dev = (struct pwm_channel *)dev_get_drvdata(dev);

    return sprintf(buf, "%llu\n", pwm_ch_dev->pwm_cfg->pwm_attr.period_wf);
}

static ssize_t period_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    int ret;
    u64 period;

    struct pwm_channel *pwm_ch_dev = (struct pwm_channel *)dev_get_drvdata(dev);

    ret = kstrtou64(buf, 0, &period);
    if (ret)
    {
        pwm_err("correct format: echo [period] > period\n");
        return ret;
    }

    hal_pwm_set_period(pwm_ch_dev->pwm_cfg, period, 0);

    return count;
}
static DEVICE_ATTR_RW(period);

static int sstar_pwm_create_channel_file(struct device *parent, struct pwm_channel *pwm_ch_dev)
{
    int   ret;
    char *channel_prop[2];

    struct attribute *channel_attrs[] = {
        &dev_attr_period.attr,
        &dev_attr_duty.attr,
        &dev_attr_shift.attr,
        &dev_attr_polarity.attr,
        &dev_attr_enable.attr,
        &dev_attr_info.attr,
        NULL,
    };

    struct attribute_group channel_group = {
        .attrs = channel_attrs,
    };

    struct attribute_group *channel_groups[] = {
        &channel_group,
        NULL,
    };

    pwm_dbg("pwm%u creat device in sysfs\n", pwm_ch_dev->channel);
    pwm_ch_dev->dev = (struct device *)kzalloc(sizeof(struct device), GFP_KERNEL);
    if (!pwm_ch_dev->dev)
    {
        pwm_err("pwm%u device can not be allocated memory\n", pwm_ch_dev->channel);
        return -ENOMEM;
    }

    pwm_ch_dev->dev->release = sstar_pwm_dev_release;
    pwm_ch_dev->dev->parent  = parent;
    pwm_ch_dev->dev->devt    = MKDEV(0, 0);
    pwm_ch_dev->dev->groups  = (const struct attribute_group **)channel_groups;
    dev_set_name(pwm_ch_dev->dev, "pwm%u", pwm_ch_dev->channel);
    dev_set_drvdata(pwm_ch_dev->dev, (void *)pwm_ch_dev);

    ret = device_register(pwm_ch_dev->dev);
    if (ret)
    {
        pwm_err("pwm%u device can not register\n", pwm_ch_dev->channel);
        put_device(pwm_ch_dev->dev);
        return ret;
    }

    channel_prop[0] = kasprintf(GFP_KERNEL, "EXPORT=pwm%u", pwm_ch_dev->channel);
    channel_prop[1] = NULL;
    kobject_uevent_env(&parent->kobj, KOBJ_CHANGE, channel_prop);
    kfree(channel_prop[0]);

    return 0;
}

static ssize_t g_info_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    u32                 stop;
    u32                 round;
    u32                 status;
    u32                 div;
    u32                 duty;
    u32                 shift;
    u32                 enable;
    u32                 period;
    u32                 polar;
    u64                 duty_cycle;
    char *              str        = buf;
    char *              end        = buf + PAGE_SIZE;
    const char *        polarity   = "unknown";
    struct pwm_channel *pos_ch     = NULL;
    struct pwm_group *  pwm_gp_dev = (struct pwm_group *)dev_get_drvdata(dev);

    status = hal_pwm_get_group_status(pwm_gp_dev->addr_base, pwm_gp_dev->group)
             | !(hal_pwm_get_group_reset(pwm_gp_dev->addr_base, pwm_gp_dev->group));
    stop  = hal_pwm_get_stop_mode_status(pwm_gp_dev->addr_base, pwm_gp_dev->group);
    round = hal_pwm_get_round_cnt(pwm_gp_dev->addr_base, pwm_gp_dev->group);

    str += scnprintf(str, end - str, "\t group      : %u\n", pwm_gp_dev->group);
    str += scnprintf(str, end - str, "\t enable     : %u\n", status);
    str += scnprintf(str, end - str, "\t stop       : %u\n", stop);
    str += scnprintf(str, end - str, "\t round      : %u\n", round);

    list_for_each_entry_reverse(pos_ch, &pwm_gp_dev->channel_head, channel_node)
    {
        div    = hal_pwm_get_div(pos_ch->pwm_cfg);
        duty   = hal_pwm_get_duty_pct(pos_ch->pwm_cfg);
        shift  = hal_pwm_get_shift_pct(pos_ch->pwm_cfg);
        period = hal_pwm_get_period_hz(pos_ch->pwm_cfg);
        polar  = hal_pwm_get_polarity(pos_ch->pwm_cfg);
        enable = hal_pwm_get_channel_reset(pos_ch->pwm_cfg) ? 0 : 1;

        switch (polar)
        {
            case PWM_POLARITY_NORMAL:
                polarity   = "normal";
                duty_cycle = duty - shift;
                break;

            case PWM_POLARITY_INVERSED:
                polarity   = "inversed";
                duty_cycle = (100 - duty) + shift;
                break;
        }

        str += scnprintf(str, end - str, "\n\t channel    : %u\n", pos_ch->channel);
        str += scnprintf(str, end - str, "\t enable     : %u \n", enable);
        str += scnprintf(str, end - str, "\t div        : %u\n", div);
        str += scnprintf(str, end - str, "\t duty       : %u%%\n", duty);
        str += scnprintf(str, end - str, "\t shift      : %u%%\n", shift);
        str += scnprintf(str, end - str, "\t duty_cycle : %llu%%\n", duty_cycle);
        str += scnprintf(str, end - str, "\t period     : %uHZ\n", period);
        str += scnprintf(str, end - str, "\t polarity   : %s\n", polarity);
    }

    return (str - buf);
}
static DEVICE_ATTR_RO(g_info);

static ssize_t g_enable_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    u32               ret;
    const char *      status     = "unknown";
    struct pwm_group *pwm_gp_dev = (struct pwm_group *)dev_get_drvdata(dev);

    ret = hal_pwm_get_group_status(pwm_gp_dev->addr_base, pwm_gp_dev->group)
          | !(hal_pwm_get_group_reset(pwm_gp_dev->addr_base, pwm_gp_dev->group));

    switch (ret)
    {
        case 0:
            status = "disable";
            break;

        case 1:
            status = "enable";
            break;
    }

    return sprintf(buf, "%s\n", status);
}

static ssize_t g_enable_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    int                 ret;
    u8                  value;
    struct pwm_channel *pos_ch = NULL;

    struct pwm_group *pwm_gp_dev = (struct pwm_group *)dev_get_drvdata(dev);

    ret = kstrtou8(buf, 0, &value);
    if (ret)
    {
        pwm_err("correct format: echo [enable] > g_enable\n");
        return ret;
    }

    if (value)
    {
        list_for_each_entry(pos_ch, &pwm_gp_dev->channel_head, channel_node)
        {
            if (pos_ch->group == pwm_gp_dev->group)
            {
                if (!pos_ch->clock_en)
                {
                    ret = clk_prepare_enable(pos_ch->pwm_clk);
                    if (ret)
                    {
                        pwm_err("fail to enable pwm%u clk\n", pos_ch->channel);
                        return ret;
                    }
                    pos_ch->clock_en++;
                }
                hal_pwm_set_channel_reset(pos_ch->pwm_cfg, 0);
            }
        }
        hal_pwm_set_group_reset(pwm_gp_dev->addr_base, pwm_gp_dev->group, 0);
        hal_pwm_group_enbale(pwm_gp_dev->addr_base, pwm_gp_dev->group, 1);
    }
    else
    {
        hal_pwm_group_enbale(pwm_gp_dev->addr_base, pwm_gp_dev->group, 0);
        hal_pwm_set_group_reset(pwm_gp_dev->addr_base, pwm_gp_dev->group, 1);
        list_for_each_entry(pos_ch, &pwm_gp_dev->channel_head, channel_node)
        {
            if (pos_ch->group == pwm_gp_dev->group)
            {
                hal_pwm_set_channel_reset(pos_ch->pwm_cfg, 1);
                if (pos_ch->clock_en)
                {
                    clk_disable_unprepare(pos_ch->pwm_clk);
                    pos_ch->clock_en--;
                }
            }
        }
    }

    return count;
}
static DEVICE_ATTR_RW(g_enable);

static ssize_t g_polarity_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    int                 ret;
    u32                 channel;
    u32                 arg[2];
    enum pwm_polarity   polarity;
    struct pwm_channel *pos_ch     = NULL;
    struct pwm_group *  pwm_gp_dev = (struct pwm_group *)dev_get_drvdata(dev);

    ret = sscanf(buf, "%u %u", &arg[0], &arg[1]);
    if (ret == 1)
    {
        channel  = 0xFF;
        polarity = arg[0];
    }
    else if (ret == 2)
    {
        channel  = arg[0];
        polarity = arg[1];
    }
    else
    {
        pwm_err("correct format 1: echo [0/1] > g_polarity\n");
        pwm_err("correct format 2: echo [channel] [0/1] > g_polarity\n");
        return -EINVAL;
    }

    switch (ret)
    {
        case 1:
            list_for_each_entry(pos_ch, &pwm_gp_dev->channel_head, channel_node)
            {
                if (pos_ch->group == pwm_gp_dev->group)
                    hal_pwm_set_polarity(pos_ch->pwm_cfg, polarity, 1);
            }
            break;

        case 2:
            list_for_each_entry(pos_ch, &pwm_gp_dev->channel_head, channel_node)
            {
                if ((pos_ch->group == pwm_gp_dev->group) && (pos_ch->channel == channel))
                    hal_pwm_set_polarity(pos_ch->pwm_cfg, polarity, 1);
            }
            break;
    }

    return count;
}
static DEVICE_ATTR_WO(g_polarity);

static ssize_t g_shift_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    int                 ret;
    u32                 channel;
    u64                 shift;
    u64                 arg[2];
    struct pwm_channel *pos_ch     = NULL;
    struct pwm_group *  pwm_gp_dev = (struct pwm_group *)dev_get_drvdata(dev);

    ret = sscanf(buf, "%llu %llu", &arg[0], &arg[1]);
    if (ret == 1)
    {
        channel = 0xFF;
        shift   = arg[0];
    }
    else if (ret == 2)
    {
        channel = (u32)arg[0];
        shift   = arg[1];
    }
    else
    {
        pwm_err("correct format 1: echo [shift] > g_shift\n");
        pwm_err("correct format 2: echo [channel] [shift] > g_shift\n");
        return -EINVAL;
    }

    switch (ret)
    {
        case 1:
            list_for_each_entry(pos_ch, &pwm_gp_dev->channel_head, channel_node)
            {
                if (pos_ch->group == pwm_gp_dev->group)
                    hal_pmw_set_shift(pos_ch->pwm_cfg, shift, 1);
            }
            break;

        case 2:
            list_for_each_entry(pos_ch, &pwm_gp_dev->channel_head, channel_node)
            {
                if ((pos_ch->group == pwm_gp_dev->group) && (pos_ch->channel == channel))
                    hal_pmw_set_shift(pos_ch->pwm_cfg, shift, 1);
            }
            break;
    }

    return count;
}

static DEVICE_ATTR_WO(g_shift);

static ssize_t g_duty_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    int                 ret;
    u32                 channel;
    u64                 duty;
    u64                 arg[2];
    struct pwm_channel *pos_ch     = NULL;
    struct pwm_group *  pwm_gp_dev = (struct pwm_group *)dev_get_drvdata(dev);

    ret = sscanf(buf, "%llu %llu", &arg[0], &arg[1]);
    if (ret == 1)
    {
        channel = 0xFF;
        duty    = arg[0];
    }
    else if (ret == 2)
    {
        channel = (u32)arg[0];
        duty    = arg[1];
    }
    else
    {
        pwm_err("correct format 1: echo [duty] > g_duty\n");
        pwm_err("correct format 2: echo [channel] [duty] > g_duty\n");
        return -EINVAL;
    }

    switch (ret)
    {
        case 1:
            list_for_each_entry(pos_ch, &pwm_gp_dev->channel_head, channel_node)
            {
                if (pos_ch->group == pwm_gp_dev->group)
                    hal_pwm_set_duty(pos_ch->pwm_cfg, duty, 1);
            }
            break;

        case 2:
            list_for_each_entry(pos_ch, &pwm_gp_dev->channel_head, channel_node)
            {
                if ((pos_ch->group == pwm_gp_dev->group) && (pos_ch->channel == channel))
                    hal_pwm_set_duty(pos_ch->pwm_cfg, duty, 1);
            }
            break;
    }

    return count;
}

static DEVICE_ATTR_WO(g_duty);

static ssize_t g_period_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    int                 ret;
    u32                 channel;
    u64                 period;
    u64                 arg[2];
    struct pwm_channel *pos_ch     = NULL;
    struct pwm_group *  pwm_gp_dev = (struct pwm_group *)dev_get_drvdata(dev);

    ret = sscanf(buf, "%llu %llu", &arg[0], &arg[1]);
    if (ret == 1)
    {
        channel = 0xFF;
        period  = arg[0];
    }
    else if (ret == 2)
    {
        channel = (u32)arg[0];
        period  = arg[1];
    }
    else
    {
        pwm_err("correct format 1: echo [period] > g_period\n");
        pwm_err("correct format 2: echo [channel] [period] > g_period\n");
        return -EINVAL;
    }

    switch (ret)
    {
        case 1:
            list_for_each_entry(pos_ch, &pwm_gp_dev->channel_head, channel_node)
            {
                if (pos_ch->group == pwm_gp_dev->group)
                    hal_pwm_set_period(pos_ch->pwm_cfg, period, 1);
            }
            break;

        case 2:
            list_for_each_entry(pos_ch, &pwm_gp_dev->channel_head, channel_node)
            {
                if ((pos_ch->group == pwm_gp_dev->group) && (pos_ch->channel == channel))
                    hal_pwm_set_period(pos_ch->pwm_cfg, period, 1);
            }
            break;
    }

    return count;
}
static DEVICE_ATTR_WO(g_period);

static ssize_t update_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    int               ret;
    u8                value;
    struct pwm_group *pwm_gp_dev = (struct pwm_group *)dev_get_drvdata(dev);

    ret = kstrtou8(buf, 0, &value);
    if (ret)
    {
        pwm_err("correct format: echo [1] > update\n");
        return ret;
    }

    if (value)
        hal_pwm_hold_mode_enable(pwm_gp_dev->addr_base, pwm_gp_dev->group, 1);

    return count;
}
static DEVICE_ATTR_WO(update);

static ssize_t round_cnt_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    u32               pulse;
    struct pwm_group *pwm_gp_dev = (struct pwm_group *)dev_get_drvdata(dev);

    pulse = hal_pwm_get_round_cnt(pwm_gp_dev->addr_base, pwm_gp_dev->group);

    return sprintf(buf, "%u\n", pulse);
}
static DEVICE_ATTR_RO(round_cnt);

static ssize_t round_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    u32               round;
    struct pwm_group *pwm_gp_dev = (struct pwm_group *)dev_get_drvdata(dev);

    round = hal_pwm_get_round_num(pwm_gp_dev->addr_base, pwm_gp_dev->group);

    return sprintf(buf, "%u\n", round);
}

static ssize_t round_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    int               ret;
    u16               round_num;
    u32               status;
    struct pwm_group *pwm_gp_dev = (struct pwm_group *)dev_get_drvdata(dev);

    ret = kstrtou16(buf, 0, &round_num);
    if (ret)
    {
        pwm_err("correct format: echo [round_num] > round\n");
        return ret;
    }

    status = hal_pwm_get_round_num(pwm_gp_dev->addr_base, pwm_gp_dev->group);
    if (status)
    {
        pwm_err("group%u round number was set up already!\n", pwm_gp_dev->group);
        return -EINVAL;
    }

    if (round_num > 0)
    {
        hal_pwm_set_group_reset(pwm_gp_dev->addr_base, pwm_gp_dev->group, 1);
        hal_pwm_set_round_num(pwm_gp_dev->addr_base, pwm_gp_dev->group, round_num);
        hal_pwm_group_enbale(pwm_gp_dev->addr_base, pwm_gp_dev->group, 1);
        hal_pwm_set_group_reset(pwm_gp_dev->addr_base, pwm_gp_dev->group, 0);
    }
    return count;
}
static DEVICE_ATTR_RW(round);

static ssize_t stop_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    u32               ret;
    const char *      stop       = "unknown";
    struct pwm_group *pwm_gp_dev = (struct pwm_group *)dev_get_drvdata(dev);

    ret = hal_pwm_get_stop_mode_status(pwm_gp_dev->addr_base, pwm_gp_dev->group);

    switch (ret)
    {
        case 0:
            stop = "disable";
            break;

        case 1:
            stop = "enbale";
            break;
    }

    return sprintf(buf, "%s\n", stop);
}

static ssize_t stop_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    int ret;
    u8  value;

    struct pwm_group *pwm_gp_dev = (struct pwm_group *)dev_get_drvdata(dev);

    ret = kstrtou8(buf, 0, &value);
    if (ret)
    {
        pwm_err("correct format: echo [enable] > stop\n");
        return ret;
    }

    if (value)
    {
        hal_pwm_stop_mode_enable(pwm_gp_dev->addr_base, pwm_gp_dev->group, 1);
    }
    else
    {
        hal_pwm_stop_mode_enable(pwm_gp_dev->addr_base, pwm_gp_dev->group, 0);
        hal_pwm_group_enbale(pwm_gp_dev->addr_base, pwm_gp_dev->group, 1);
        hal_pwm_set_group_reset(pwm_gp_dev->addr_base, pwm_gp_dev->group, 1);
        hal_pwm_set_group_reset(pwm_gp_dev->addr_base, pwm_gp_dev->group, 0);
    }

    return count;
}
static DEVICE_ATTR_RW(stop);

static ssize_t join_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    u32                 status;
    char *              str        = buf;
    char *              end        = buf + PAGE_SIZE;
    struct pwm_channel *pos_ch     = NULL;
    struct pwm_group *  pwm_gp_dev = (struct pwm_group *)dev_get_drvdata(dev);

    list_for_each_entry_reverse(pos_ch, &pwm_gp_dev->channel_head, channel_node)
    {
        status = hal_pwm_get_sync_mode_status(pos_ch->pwm_cfg);
        if (status)
            str += scnprintf(str, end - str, "\t channel : %u, join group\n", pos_ch->channel);
        else
            str += scnprintf(str, end - str, "\t channel : %u, exit group\n", pos_ch->channel);
    }

    return (str - buf);
}

static ssize_t join_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    int                 ret;
    u32                 channel;
    u32                 enable;
    struct pwm_channel *pos_ch     = NULL;
    struct pwm_group *  pwm_gp_dev = (struct pwm_group *)dev_get_drvdata(dev);

    ret = sscanf(buf, "%d %d", &channel, &enable);
    if (ret != 2)
    {
        pwm_err("correct format: echo [channel] [1/0] > join\n");
        return ret;
    }

    list_for_each_entry(pos_ch, &pwm_gp_dev->channel_head, channel_node)
    {
        if (pos_ch->channel == channel)
        {
            if (enable)
            {
                pos_ch->group = pwm_gp_dev->group;
                hal_pwm_sync_mode_enable(pos_ch->pwm_cfg, 1);
            }
            else
            {
                pos_ch->group = PWM_NOADD_GROUP;
                hal_pwm_sync_mode_enable(pos_ch->pwm_cfg, 0);
            }
            break;
        }
    }

    return count;
}
static DEVICE_ATTR_RW(join);

static int sstar_pwm_create_group_file(struct device *parent, struct pwm_group *pos)
{
    int            ret;
    char *         group_prop[2];
    struct device *group_dev = NULL;

    struct attribute *func_attrs[] = {
        &dev_attr_join.attr,       &dev_attr_stop.attr,     &dev_attr_round.attr,  &dev_attr_round_cnt.attr,
        &dev_attr_update.attr,     &dev_attr_g_period.attr, &dev_attr_g_duty.attr, &dev_attr_g_shift.attr,
        &dev_attr_g_polarity.attr, &dev_attr_g_enable.attr, &dev_attr_g_info.attr, NULL,
    };

    struct attribute_group func_group = {
        .attrs = func_attrs,
    };

    struct attribute_group *func_groups[] = {
        &func_group,
        NULL,
    };

    pwm_dbg("group%u creat device in sysfs\n", pos->group);
    group_dev = (struct device *)kzalloc(sizeof(struct device), GFP_KERNEL);
    if (!group_dev)
    {
        pwm_err("group%u device can not be allocated memory\n", pos->group);
        return -ENOMEM;
    }

    group_dev->release = sstar_pwm_dev_release;
    group_dev->parent  = parent;
    group_dev->devt    = MKDEV(0, 0);
    group_dev->groups  = (const struct attribute_group **)func_groups;
    dev_set_name(group_dev, "group%u", pos->group);
    dev_set_drvdata(group_dev, (void *)pos);

    ret = device_register(group_dev);
    if (ret)
    {
        pwm_err("group%u device can not register\n", pos->group);
        put_device(group_dev);
        return ret;
    }
    pos->dev_data = (void *)group_dev;
    group_prop[0] = kasprintf(GFP_KERNEL, "EXPORT=group%u", pos->group);
    group_prop[1] = NULL;
    kobject_uevent_env(&parent->kobj, KOBJ_CHANGE, group_prop);
    kfree(group_prop[0]);

    return 0;
}

static int sstar_pwm_create_bus_file(struct pwm_channel *pwm_ch_dev)
{
    struct device *dev = NULL;

    dev = device_create(msys_get_sysfs_class(), NULL, MKDEV(0, 0), NULL, "pwm");
    if (IS_ERR_OR_NULL(dev))
    {
        pwm_err("fail to creates pwm device and registers it with sysfs\n");
        return -ENODEV;
    }
    dev_set_drvdata(dev, (void *)pwm_ch_dev);
    pwm_basic_dev = dev;

    return 0;
}

static int sstar_pwm_create_sysfs_file(struct pwm_channel *pwm_ch_dev)
{
    int               ret;
    struct device *   group_dev = NULL;
    struct pwm_group *pos_gp    = NULL;

    if (!pwm_basic_dev)
    {
        ret = sstar_pwm_create_bus_file(pwm_ch_dev);
        if (ret)
            return ret;
    }

    if ((pwm_ch_dev->group != PWM_NOADD_GROUP) && (pwm_ch_dev->spwm_gp == SPWM_NOADD_GROUP))
    {
        list_for_each_entry(pos_gp, &pwm_group_list, group_node)
        {
            if ((!pos_gp->sysfs_created) && (pos_gp->group == pwm_ch_dev->group))
            {
                ret = sstar_pwm_create_group_file(pwm_basic_dev, pos_gp);
                if (!ret)
                {
                    pos_gp->sysfs_created = PWM_SYSFS_CREATED;
                    break;
                }
                else
                    return ret;
            }
        }

        list_for_each_entry(pos_gp, &pwm_group_list, group_node)
        {
            if ((pos_gp->dev_data != NULL) && (pos_gp->group == pwm_ch_dev->group))
            {
                group_dev = (struct device *)pos_gp->dev_data;
                ret       = sstar_pwm_create_channel_file(group_dev, pwm_ch_dev);
                if (ret)
                    return ret;
                break;
            }
        }
    }
    else if ((pwm_ch_dev->group == PWM_NOADD_GROUP) && (pwm_ch_dev->spwm_gp != SPWM_NOADD_GROUP))
    {
#ifdef CONFIG_SSTAR_SPWM
        list_for_each_entry(pos_gp, &pwm_group_list, group_node)
        {
            if ((!pos_gp->sysfs_created) && (pos_gp->spwm_gp == pwm_ch_dev->spwm_gp))
            {
                ret = sstar_spwm_create_group_file(pwm_basic_dev, pos_gp);
                if (!ret)
                {
                    pos_gp->sysfs_created = PWM_SYSFS_CREATED;
                    break;
                }
                else
                    return ret;
            }
        }
#endif
    }
    else if ((pwm_ch_dev->group == PWM_NOADD_GROUP) && (pwm_ch_dev->spwm_gp == SPWM_NOADD_GROUP))
    {
        ret = sstar_pwm_create_channel_file(pwm_basic_dev, pwm_ch_dev);
        if (ret)
            return ret;
    }
    else
    {
        pwm_err("pwm[%u] can not join group and spwm at the same time\n", pwm_ch_dev->channel);
        return -EINVAL;
    }

    return 0;
}

static void sstar_pwm_group_release(struct pwm_group *pwm_gp_dev)
{
    misc_deregister(&pwm_gp_dev->pwm_miscdev);
    disable_irq(pwm_gp_dev->irq);
    free_irq(pwm_gp_dev->irq, (void *)pwm_gp_dev);
    pwm_gp_dev->res = PWM_RES_FREE;
}

static int sstar_pwm_group_setup(struct platform_device *pdev, struct pwm_channel *pwm_ch_dev)
{
    int               ret;
    char              miscdev_name[16];
    struct pwm_group *pos_gp     = NULL;
    struct pwm_group *pwm_gp_dev = NULL;

    pwm_gp_dev = devm_kzalloc(&pdev->dev, sizeof(struct pwm_group), GFP_KERNEL);
    if (!pwm_gp_dev)
    {
        pwm_err("fail to allocate memory\n");
        return -ENOMEM;
    }

    pwm_gp_dev->group = pwm_ch_dev->group;
    list_for_each_entry(pos_gp, &pwm_group_list, group_node)
    {
        if (pos_gp->group == pwm_gp_dev->group)
        {
            pwm_dbg("group[%u] alreadly in pwm_group_list\n", pwm_gp_dev->group);
            goto group_out;
        }
    }

    pwm_gp_dev->pwm_fops.unlocked_ioctl = sstar_pwm_ioctl;
    pwm_gp_dev->pwm_miscdev.minor       = MISC_DYNAMIC_MINOR;
    pwm_gp_dev->pwm_miscdev.fops        = &pwm_gp_dev->pwm_fops;

    pwm_dbg("add group[%u] into pwm_group_list\n", pwm_gp_dev->group);
    INIT_LIST_HEAD(&pwm_gp_dev->channel_head);
    INIT_LIST_HEAD(&pwm_gp_dev->group_node);
    pwm_gp_dev->dev_data      = NULL;
    pwm_gp_dev->res           = PWM_RES_REQUEST;
    pwm_gp_dev->sysfs_created = PWM_SYSFS_NO_CREATE;
    pwm_gp_dev->group         = pwm_ch_dev->group;
    pwm_gp_dev->spwm_gp       = pwm_ch_dev->spwm_gp;
    pwm_gp_dev->channel       = pwm_ch_dev->channel;
    pwm_gp_dev->addr_base     = pwm_ch_dev->pwm_cfg->addr_base;

    if ((snprintf(miscdev_name, sizeof(miscdev_name), "pwm_group%u", pwm_gp_dev->group) >= 0))
        pwm_gp_dev->pwm_miscdev.name = (const char *)(char *)miscdev_name;
    ret = misc_register(&pwm_gp_dev->pwm_miscdev);
    if (ret < 0)
    {
        pwm_err("fail to register group%u miscellaneous device\n", pwm_gp_dev->group);
        return ret;
    }

    pwm_gp_dev->irq = irq_of_parse_and_map(pdev->dev.of_node, 0);
    if (!pwm_gp_dev->irq)
    {
        pwm_err("fail to parse and map pwm group%u interrupt into linux virq space\n", pwm_gp_dev->group);
        misc_deregister(&pwm_gp_dev->pwm_miscdev);
        return -EINVAL;
    }
    snprintf(pwm_gp_dev->irq_name, sizeof(pwm_gp_dev->irq_name), "pwm_group%u", pwm_gp_dev->group);
    ret = request_irq(pwm_gp_dev->irq, sstar_pwm_int_handler, IRQF_SHARED, (const char *)pwm_gp_dev->irq_name,
                      pwm_gp_dev);
    if (ret < 0)
    {
        pwm_err("fail to request group%u irq, errno %d\n", pwm_gp_dev->group, ret);
        misc_deregister(&pwm_gp_dev->pwm_miscdev);
        return ret;
    }

    list_add(&pwm_gp_dev->group_node, &pwm_group_list);
    return 0;

group_out:
    devm_kfree(&pdev->dev, pwm_gp_dev);
    return 0;
}

static void sstar_pwm_clock_release(struct pwm_channel *pwm_ch_dev)
{
#ifdef CONFIG_CAM_CLK
    CAMCLK_Set_Attribute set_cfg;
    CAMCLK_Get_Attribute get_cfg;
#endif

#ifdef CONFIG_CAM_CLK
    CamClkAttrGet(pwm_ch_dev->camclk_handle, &get_cfg);
    CAMCLK_SETPARENT(set_cfg, get_cfg.u32Parent[0]);
    CamClkAttrSet(pwm_ch_dev->camclk_handle, &set_cfg);
    CamClkSetOnOff(pwm_ch_dev->camclk_handle, 0);
    CamClkUnregister(pwm_ch_dev->camclk_handle);
#else
    pwm_ch_dev->parent_hw = clk_hw_get_parent_by_index(__clk_get_hw(pwm_ch_dev->pwm_clk), 0);
    clk_set_parent(pwm_ch_dev->pwm_clk, pwm_ch_dev->parent_hw->clk);
    clk_put(pwm_ch_dev->pwm_clk);
#endif
}

static int sstar_pwm_clock_set(struct platform_device *pdev, struct pwm_channel *pwm_ch_dev)
{
    int ret;
    u32 clk_level;
#ifdef CONFIG_CAM_CLK
    CAMCLK_Set_Attribute set_cfg;
    CAMCLK_Get_Attribute get_cfg;
#endif

    ret = of_property_read_u32(pdev->dev.of_node, "clk-select", &clk_level);
    if (ret < 0)
    {
        pwm_err("fail to get clock select from dts\n");
        return ret;
    }

#ifdef CONFIG_CAM_CLK
    of_property_read_u32_index(pdev->dev.of_node, "camclk", 0, &pwm_ch_dev->camclk_id);
    if (!pwm_ch_dev->camclk_id)
    {
        pwm_err("fail to get camclk from dts\n", __func__);
        return -CAMCLK_RET_NOTSUPPORT;
    }
    else
    {
        if (CamClkRegister((u8 *)"PWM", pwm_ch_dev->camclk_id, &pwm_ch_dev->camclk_handle) == CAMCLK_RET_OK)
        {
            CamClkAttrGet(pwm_ch_dev->camclk_handle, &get_cfg);
            CAMCLK_SETPARENT(set_cfg, get_cfg.u32Parent[clk_level]);
            CamClkAttrSet(pwm_ch_dev->camclk_handle, &set_cfg);
            CamClkSetOnOff(pwm_ch_dev->camclk_handle, 1);
            pwm_ch_dev->pwm_cfg->clk_freq = CamClkRateGet(pwm_ch_dev->camclk_id);
        }
        else
        {
            pwm_err("fail to set camclk\n", __func__);
            return -CAMCLK_RET_FAIL;
        }
    }

#else
    pwm_ch_dev->pwm_clk = of_clk_get(pdev->dev.of_node, 0);
    if (IS_ERR_OR_NULL(pwm_ch_dev->pwm_clk))
    {
        pwm_err("fail to allocate pwm%u and link clk to clk_core\n", pwm_ch_dev->channel);
        return PTR_ERR(pwm_ch_dev->pwm_clk);
    }
    pwm_ch_dev->parent_hw = clk_hw_get_parent_by_index(__clk_get_hw(pwm_ch_dev->pwm_clk), clk_level);
    if (IS_ERR_OR_NULL(pwm_ch_dev->parent_hw))
    {
        pwm_err("failed to get pwm%u hw parent clk by index\n", pwm_ch_dev->channel);
        return PTR_ERR(pwm_ch_dev->parent_hw);
    }
    clk_set_parent(pwm_ch_dev->pwm_clk, pwm_ch_dev->parent_hw->clk);
    pwm_ch_dev->pwm_cfg->clk_freq = clk_get_rate(pwm_ch_dev->pwm_clk);
#endif

    return 0;
}

static int sstar_pwm_probe(struct platform_device *pdev)
{
    int                 ret;
    char                miscdev_name[16];
    struct resource *   res        = NULL;
    struct pwm_group *  pos_gp     = NULL;
    struct pwm_channel *pwm_ch_dev = NULL;
    struct device_node *dev_node   = NULL;

    pwm_ch_dev = devm_kzalloc(&pdev->dev, sizeof(struct pwm_channel), GFP_KERNEL);
    if (!pwm_ch_dev)
    {
        pwm_err("fail to allocate memory\n");
        return -ENOMEM;
    }
    pwm_ch_dev->pwm_cfg = devm_kzalloc(&pdev->dev, sizeof(struct hal_pwm_cfg), GFP_KERNEL);
    if (!pwm_ch_dev->pwm_cfg)
    {
        pwm_err("fail to allocate memory\n");
        return -ENOMEM;
    }

    ret = of_property_read_u32(pdev->dev.of_node, "channel", &pwm_ch_dev->channel);
    if (ret < 0)
    {
        pwm_err("fail to get pwm channel from dts\n");
        return ret;
    }
    else
    {
        pwm_ch_dev->pwm_cfg->channel = pwm_ch_dev->channel;
    }

    res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
    if (!res->start)
    {
        pwm_err("fail to get a resource from pwm device\n");
        return PTR_ERR(res);
    }
    else
    {
        pwm_ch_dev->pwm_cfg->bank_base = IO_ADDRESS(res->start);
        pwm_ch_dev->pwm_cfg->addr_base = pwm_ch_dev->pwm_cfg->bank_base >> 0x18;
        pwm_ch_dev->pwm_cfg->addr_base = pwm_ch_dev->pwm_cfg->addr_base << 0x18;
    }

    ret = sstar_pwm_clock_set(pdev, pwm_ch_dev);
    if (ret < 0)
        return ret;
    else if (!pwm_ch_dev->pwm_cfg->clk_freq)
    {
        pwm_err("fail to get clock frequency\n");
        goto out1;
    }

    ret = of_property_read_u32(pdev->dev.of_node, "group", &pwm_ch_dev->group);
    if (ret < 0)
    {
        pwm_dbg("fail to get group from dts\n");
        pwm_ch_dev->group = PWM_NOADD_GROUP;
    }
    pwm_ch_dev->pwm_cfg->group = pwm_ch_dev->group;

    ret = of_property_read_u32(pdev->dev.of_node, "spwm-group", &pwm_ch_dev->spwm_gp);
    if (ret < 0)
    {
        pwm_dbg("fail to get spwm group from dts\n");
        pwm_ch_dev->spwm_gp = SPWM_NOADD_GROUP;
    }
    pwm_ch_dev->pwm_cfg->spwm_gp = pwm_ch_dev->spwm_gp;

    if ((pwm_ch_dev->group != PWM_NOADD_GROUP) && (pwm_ch_dev->spwm_gp == SPWM_NOADD_GROUP))
    {
        /* pwm channel add group list*/
        ret = sstar_pwm_group_setup(pdev, pwm_ch_dev);
        if (ret < 0)
        {
            goto out1;
        }

        list_for_each_entry(pos_gp, &pwm_group_list, group_node)
        {
            if (pos_gp->group == pwm_ch_dev->group)
            {
                pwm_dbg("pwm[%u] add into channel_head in group list\n", pwm_ch_dev->channel);
                INIT_LIST_HEAD(&pwm_ch_dev->channel_node);
                list_add(&pwm_ch_dev->channel_node, &pos_gp->channel_head);
                pwm_ch_dev->list_add = PWM_LIST_ADD_GROUP;
                break;
            }
        }
    }
    else if ((pwm_ch_dev->group == PWM_NOADD_GROUP) && (pwm_ch_dev->spwm_gp != SPWM_NOADD_GROUP))
    {
        /* pwm channel add spwm list*/
#ifdef CONFIG_SSTAR_SPWM
        ret = sstar_spwm_group_setup(pdev, pwm_ch_dev);
        if (ret < 0)
        {
            goto out1;
        }

        list_for_each_entry(pos_gp, &pwm_group_list, group_node)
        {
            if (pos_gp->spwm_gp == pwm_ch_dev->spwm_gp)
            {
                pwm_dbg("pwm[%u] add into channel_head in spwm group list\n", pwm_ch_dev->channel);
                INIT_LIST_HEAD(&pwm_ch_dev->channel_node);
                list_add(&pwm_ch_dev->channel_node, &pos_gp->channel_head);
                pwm_ch_dev->list_add = PWM_LIST_ADD_GROUP;
                break;
            }
        }
#else
        pwm_err("fail to config spwm\n");
        goto out1;
#endif
    }
    else if ((pwm_ch_dev->group == PWM_NOADD_GROUP) && (pwm_ch_dev->spwm_gp == SPWM_NOADD_GROUP))
    {
        /* pwm channel add channel list*/
        pwm_dbg("pwm[%u] add into pwm_channel_list\n", pwm_ch_dev->channel);
        INIT_LIST_HEAD(&pwm_ch_dev->channel_node);
        list_add(&pwm_ch_dev->channel_node, &pwm_channel_list);
        pwm_ch_dev->list_add = PWM_LIST_ADD_CHANNEL;
    }
    else
    {
        pwm_err("pwm[%u] can not join group and spwm at the same time\n", pwm_ch_dev->channel);
        ret = -EINVAL;
        goto out1;
    }

    ret = sstar_pwm_create_sysfs_file(pwm_ch_dev);
    if (ret < 0)
        goto out2;

    if (pwm_ch_dev->spwm_gp == SPWM_NOADD_GROUP)
    {
        pwm_ch_dev->pwm_fops.unlocked_ioctl = sstar_pwm_ioctl;
        if ((snprintf(miscdev_name, sizeof(miscdev_name), "pwm%u", pwm_ch_dev->channel) >= 0))
            pwm_ch_dev->pwm_miscdev.name = (const char *)(char *)miscdev_name;
    }
    else
    {
#ifdef CONFIG_SSTAR_SPWM
        pwm_ch_dev->pwm_fops.unlocked_ioctl = sstar_spwm_ioctl;
#endif
        if ((snprintf(miscdev_name, sizeof(miscdev_name), "spwm%u", pwm_ch_dev->channel) >= 0))
            pwm_ch_dev->pwm_miscdev.name = (const char *)(char *)miscdev_name;
    }
    pwm_ch_dev->pwm_miscdev.minor = MISC_DYNAMIC_MINOR;
    pwm_ch_dev->pwm_miscdev.fops  = &pwm_ch_dev->pwm_fops;
    ret                           = misc_register(&pwm_ch_dev->pwm_miscdev);
    if (ret < 0)
    {
        pwm_err("fail to register a miscellaneous device\n");
        goto out3;
    }

    if (pwm_ch_dev->spwm_gp != SPWM_NOADD_GROUP)
    {
#ifdef CONFIG_SSTAR_SPWM
        ret = hal_spwm_init(pwm_ch_dev->pwm_cfg);
        if (ret)
            goto out4;
#endif
    }
    else
    {
        ret = sstar_pwm_chip_init(pdev, pwm_ch_dev);
        if (ret < 0)
        {
            pwm_err("fail to register a miscellaneous device\n");
            goto out4;
        }

        /*spwm do not support dead time */
        pwm_ch_dev->ddt = of_property_read_bool(pdev->dev.of_node, "dead-time");
        if (pwm_ch_dev->ddt)
        {
#ifdef CONFIG_SSTAR_PWM_DDT
            ret = sstar_pwm_ddt_config(pdev, pwm_ch_dev);
            if (ret)
                goto out5;
#else
            pwm_err("fail to config dead time\n");
#endif
        }
        ret = hal_pwm_init(pwm_ch_dev->pwm_cfg);
        if (ret)
            goto out5;

        if (sstar_pwm_set_up(pdev, pwm_ch_dev))
            pwm_dbg("pwm%u fail to set parameters at initialization\n", pwm_ch_dev->channel);

        if (!sstar_pwm_dev_node)
        {
            for_each_compatible_node(dev_node, NULL, "sstar,pwm")
            {
                if (of_device_is_available(dev_node))
                    sstar_pwm_dev_node++;
            }
        }
        sstar_pwm_last_init++;

        if (sstar_pwm_last_init == sstar_pwm_dev_node)
        {
            list_for_each_entry(pos_gp, &pwm_group_list, group_node)
            {
                if (hal_pwm_get_group_reset(pos_gp->addr_base, pos_gp->group))
                {
                    hal_pwm_hold_mode_enable(pos_gp->addr_base, pos_gp->group, 1);
                    hal_pwm_set_group_reset(pos_gp->addr_base, pos_gp->group, 0);
                }
            }
        }
    }

    platform_set_drvdata(pdev, pwm_ch_dev);

    return 0;

out5:
    pwmchip_remove(&pwm_ch_dev->chip);
out4:
    misc_deregister(&pwm_ch_dev->pwm_miscdev);
out3:
    if (pwm_basic_dev)
    {
        device_unregister(pwm_basic_dev);
        pwm_basic_dev = NULL;
    }
out2:
    list_del(&pwm_ch_dev->channel_node);
    list_for_each_entry(pos_gp, &pwm_group_list, group_node)
    {
        if (pos_gp->channel == pwm_ch_dev->channel)
        {
            sstar_pwm_group_release(pos_gp);
            list_del(&pos_gp->group_node);
            break;
        }
    }
out1:
    sstar_pwm_clock_release(pwm_ch_dev);
    return ret;
}

int CamPwmConfig(void *pwm_handler, enum CamPWMArgs args, struct CamPwmState *cam_state)
{
    int                ret;
    struct pwm_state   state;
    struct pwm_device *pwm_dev = (struct pwm_device *)pwm_handler;

    switch (args)
    {
        case CAM_PWM_PERIOD:
            pwm_get_state(pwm_dev, &state);
            state.period = cam_state->period;
            ret          = pwm_apply_state(pwm_dev, &state);
            break;
        case CAM_PWM_DUTY:
            pwm_get_state(pwm_dev, &state);
            state.duty_cycle = cam_state->duty;
            ret              = pwm_apply_state(pwm_dev, &state);
            break;
        case CAM_PWM_POLAR:
            pwm_get_state(pwm_dev, &state);
            state.polarity = cam_state->polarity;
            ret            = pwm_apply_state(pwm_dev, &state);
            break;
        case CAM_PWM_ONOFF:
            pwm_get_state(pwm_dev, &state);
            state.enabled = cam_state->enabled;
            ret           = pwm_apply_state(pwm_dev, &state);
            break;
        case CAM_PWM_ALL:
            state.period      = cam_state->period;
            state.duty_cycle  = cam_state->duty;
            state.polarity    = cam_state->polarity;
            state.enabled     = cam_state->enabled;
            state.output_type = PWM_OUTPUT_FIXED;
            ret               = pwm_apply_state(pwm_dev, &state);
            break;
        default:
            pwm_err("pwm configuration is not supported\n");
    }

    if (ret < 0)
    {
        pwm_err("failed to config pwm\n");
        return ret;
    }

    return 0;
}
EXPORT_SYMBOL(CamPwmConfig);

void CamPwmFree(void *pwm_handler)
{
    struct pwm_device *pwm_dev = (struct pwm_device *)pwm_handler;
    pwm_put(pwm_dev);
}
EXPORT_SYMBOL(CamPwmFree);

void *CamPwmRequest(int channel, const char *label)
{
    struct pwm_device *pwm_dev;
    void *             pwm_handler = NULL;

    pwm_dev = pwm_request(channel, label);
    if (IS_ERR(pwm_dev))
    {
        pwm_err("failed to request pwm device\n");
        return NULL;
    }

    pwm_handler = (void *)pwm_dev;

    return pwm_handler;
}
EXPORT_SYMBOL(CamPwmRequest);

static int sstar_pwm_remove(struct platform_device *pdev)
{
    int                 ret;
    struct pwm_group *  tmp_gp     = NULL;
    struct pwm_group *  pos_gp     = NULL;
    struct pwm_channel *tmp_ch     = NULL;
    struct pwm_channel *pos_ch     = NULL;
    struct pwm_channel *pwm_ch_dev = platform_get_drvdata(pdev);

    if (pwm_ch_dev->spwm_gp == SPWM_NOADD_GROUP)
    {
        ret = pwmchip_remove(&pwm_ch_dev->chip);
        if (ret < 0)
        {
            pwm_err("fail to remove pwm%u chip\n", pwm_ch_dev->channel);
            return ret;
        }

        hal_pwm_deinit(pwm_ch_dev->pwm_cfg);
    }
    else
    {
#ifdef CONFIG_SSTAR_SPWM
        hal_spwm_deinit(pwm_ch_dev->pwm_cfg);
#endif
    }

    misc_deregister(&pwm_ch_dev->pwm_miscdev);

    if (pwm_basic_dev)
    {
        device_unregister(pwm_basic_dev);
        pwm_basic_dev = NULL;
    }

    list_for_each_entry_safe(pos_gp, tmp_gp, &pwm_group_list, group_node)
    {
        if (pos_gp->res)
        {
            sstar_pwm_group_release(pos_gp);
            list_for_each_entry_safe(pos_ch, tmp_ch, &pos_gp->channel_head, channel_node)
            {
                if (pos_ch->list_add)
                    list_del(&pos_ch->channel_node);
            }
            list_del(&pos_gp->group_node);
        }
    }

    list_for_each_entry_safe(pos_ch, tmp_ch, &pwm_channel_list, channel_node)
    {
        if (!pos_ch->list_add)
            list_del(&pos_ch->channel_node);
    }

    if (pwm_ch_dev->clock_en)
    {
        clk_disable_unprepare(pwm_ch_dev->pwm_clk);
    }
    sstar_pwm_clock_release(pwm_ch_dev);

    return 0;
}

#ifdef CONFIG_PM_SLEEP
static int sstar_pwm_suspend(struct platform_device *pdev, pm_message_t state)
{
    struct pwm_group *  pos_gp     = NULL;
    struct pwm_channel *pwm_ch_dev = platform_get_drvdata(pdev);

    if (!pwm_ch_dev)
    {
        pwm_err("pwm channel not supported\n");
        return -ENODEV;
    }

    list_for_each_entry(pos_gp, &pwm_group_list, group_node)
    {
        if ((pos_gp->res) && (pos_gp->group == pwm_ch_dev->group))
        {
            disable_irq(pos_gp->irq);
            hal_pwm_suspend_group(pos_gp->addr_base, pos_gp->group);
            pos_gp->res = PWM_RES_FREE;
            break;
        }
    }

    hal_pwm_suspend_channel(pwm_ch_dev->pwm_cfg);

    return 0;
}

static int sstar_pwm_resume(struct platform_device *pdev)
{
    struct pwm_group *  pos_gp     = NULL;
    struct pwm_channel *pwm_ch_dev = platform_get_drvdata(pdev);

    if (!pwm_ch_dev)
    {
        pwm_err("pwm channel not supported\n");
        return -ENODEV;
    }

    hal_pwm_resume_channel(pwm_ch_dev->pwm_cfg);

    sstar_pwm_last_init--;
    if (!sstar_pwm_last_init)
    {
        sstar_pwm_last_init = sstar_pwm_dev_node;
        list_for_each_entry(pos_gp, &pwm_group_list, group_node)
        {
            enable_irq(pos_gp->irq);
            hal_pwm_resume_group(pos_gp->addr_base, pos_gp->group);
            pos_gp->res = PWM_RES_REQUEST;
        }
    }

    return 0;
}
#endif

static const struct of_device_id sstar_pwm_of_match_table[] = {{.compatible = "sstar,pwm"}, {}};

MODULE_DEVICE_TABLE(of, sstar_pwm_of_match_table);

static struct platform_driver sstar_pwm_driver = {
    .remove = sstar_pwm_remove,
    .probe  = sstar_pwm_probe,
#ifdef CONFIG_PM_SLEEP
    .suspend = sstar_pwm_suspend,
    .resume  = sstar_pwm_resume,
#endif
    .driver =
        {
            .name           = "sstar-pwm",
            .owner          = THIS_MODULE,
            .of_match_table = sstar_pwm_of_match_table,
        },
};

module_platform_driver(sstar_pwm_driver);

MODULE_AUTHOR("SSTAR");
MODULE_DESCRIPTION("Sigmastar PWM driver");
MODULE_LICENSE("GPL v2");
