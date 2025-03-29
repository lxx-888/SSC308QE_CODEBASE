/*
 * drv_iic.c- Sigmastar
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
#ifdef CONFIG_SSTAR_CLK
#include <clk.h>
#endif
#include <i2c.h>
#include <common.h>
#include <errno.h>
#include <fdtdec.h>
#include <memalign.h>
#include <dm/devres.h>
#include <asm/arch/mach/platform.h>
#include <iic_os.h>
#include <drv_iic.h>
#include <hal_iic.h>
#include <hal_iic_cfg.h>

//#define DMSG_I2C_DRIVER_DEBUG
#define dmsg_i2c_drverr(fmt, ...)                                    \
    do                                                               \
    {                                                                \
        printf("[drv_i2c_err] <%s>, " fmt, __func__, ##__VA_ARGS__); \
    } while (0)
#ifdef DMSG_I2C_DRIVER_DEBUG
#define dmsg_i2c_drvwarn(fmt, ...)                                    \
    do                                                                \
    {                                                                 \
        printf("[drv_i2c_warn] <%s>, " fmt, __func__, ##__VA_ARGS__); \
    } while (0)
#else
#define dmsg_i2c_drvwarn(fmt, ...)
#endif

struct i2c_control
{
    struct hal_i2c_ctrl hal_i2c;
    u32                 srcclk_num;
    u32 *               srcclk_rate;
#ifdef CONFIG_DM_I2C
    struct udevice *udev;
#endif
#ifdef CONFIG_SSTAR_CLK
    struct clk i2c_clk;
#endif
};

static void sstar_i2c_match_srcclk(struct i2c_control *i2c_ctrl, u32 speed)
{
    u32 i;
    u32 index;
    u32 diff;
    u32 min_diff;
    u32 tmp_rate;
    u32 match_rate = 12000000;

    if (speed <= HAL_I2C_SPEED_200KHZ)
    {
        tmp_rate = 12000000;
    }
    else if (speed <= HAL_I2C_SPEED_700KHZ)
    {
        tmp_rate = 54000000;
    }
    else
    {
        tmp_rate = 72000000;
    }

    min_diff = U32_MAX;
    for (i = 0; i < i2c_ctrl->srcclk_num; i++)
    {
        diff = abs(i2c_ctrl->srcclk_rate[i] - tmp_rate);
        if ((i2c_ctrl->srcclk_rate[i]) > 1 && (diff < min_diff))
        {
            min_diff   = diff;
            match_rate = i2c_ctrl->srcclk_rate[i];
            index      = i;
        }
    }
    i2c_ctrl->hal_i2c.match_rate = match_rate;

#ifdef CONFIG_SSTAR_CLK
    clk_set_rate(&i2c_ctrl->i2c_clk, i2c_ctrl->hal_i2c.match_rate);
#else
    HAL_I2C_WRITE_WORD_MASK(
        (i2c_clk_reg[i2c_ctrl->hal_i2c.group].bank_base + i2c_clk_reg[i2c_ctrl->hal_i2c.group].reg_offset),
        i2c_clk_cfg[i2c_ctrl->hal_i2c.group].clk_mod[index] << i2c_clk_reg[i2c_ctrl->hal_i2c.group].bit_shift,
        i2c_clk_reg[i2c_ctrl->hal_i2c.group].bit_mask);
#endif
}

int sstar_i2c_set_srclk(struct udevice *dev)
{
    int                 i;
    int                 ret;
    struct i2c_control *i2c_ctrl;

    i2c_ctrl = (struct i2c_control *)dev_get_priv(dev);
    if (!i2c_ctrl)
    {
        dmsg_i2c_drverr("failed to get i2c device pointer\n");
        return -1;
    }

#ifdef CONFIG_SSTAR_CLK
    ret = clk_get_by_index(dev, 0, &i2c_ctrl->i2c_clk);
    if (ret < 0)
    {
        return -ENOENT;
    }

    ret = clk_enable(&i2c_ctrl->i2c_clk);
    if (ret < 0)
    {
        dmsg_i2c_drverr("failed to enable i2c-%u clk\n", i2c_ctrl->hal_i2c.group);
        return ret;
    }
#endif

    i2c_ctrl->srcclk_num  = HAL_I2C_SRCCLK_NUM;
    i2c_ctrl->srcclk_rate = devm_kzalloc(dev, i2c_ctrl->srcclk_num * sizeof(int), (gfp_t)0);
    if (!i2c_ctrl->srcclk_rate)
    {
        dmsg_i2c_drverr("devm_kzalloc failed, group:%d!\n", i2c_ctrl->hal_i2c.group);
        ret = -ENOMEM;
        return ret;
    }
    for (i = 0; i < i2c_ctrl->srcclk_num; i++)
    {
        i2c_ctrl->srcclk_rate[i] = i2c_clk_cfg[i2c_ctrl->hal_i2c.group].clk_freq[i];
    }

    return 0;
}

int sstar_i2c_master_xfer(struct udevice *bus, struct i2c_msg *msg, int nmsgs)
{
    int                 l_num;
    int                 ret      = 0;
    int                 s_ret    = 0;
    struct i2c_msg *    msgs     = msg;
    struct i2c_control *i2c_ctrl = NULL;

    i2c_ctrl = (struct i2c_control *)dev_get_priv(bus);
    if (!i2c_ctrl)
    {
        dmsg_i2c_drverr("failed to get i2c device pointer\n");
        return -EINVAL;
    }

    hal_i2c_reset(&i2c_ctrl->hal_i2c, 1);
    hal_i2c_reset(&i2c_ctrl->hal_i2c, 0);

    for (l_num = 0; l_num < nmsgs; l_num++, msgs++)
    {
        if (i2c_ctrl->hal_i2c.dma_en)
        {
            if (!(msgs->flags & 0x02) && (nmsgs > 1))
            {
                hal_i2c_dma_stop_set(&i2c_ctrl->hal_i2c, ((msgs->flags) & I2C_M_RD));
            }
            else
            {
                hal_i2c_dma_stop_set(&i2c_ctrl->hal_i2c, 1);
            }
        }

        if ((msgs->flags) & I2C_M_RD)
        {
            dmsg_i2c_drvwarn("i2c-%u read\n", i2c_ctrl->hal_i2c.group);
            ret = hal_i2c_read(&i2c_ctrl->hal_i2c, msgs->addr, msgs->buf, (u32)msgs->len, msgs->flags);
        }
        else
        {
            dmsg_i2c_drvwarn("i2c-%u write\n", i2c_ctrl->hal_i2c.group);
            ret = hal_i2c_write(&i2c_ctrl->hal_i2c, msgs->addr, msgs->buf, (u32)msgs->len, msgs->flags);
        }

        if (!ret && msgs->flags & 0x02)
            ret = hal_i2c_release(&i2c_ctrl->hal_i2c);

        if (ret)
        {
            break;
        }
    }

    s_ret = hal_i2c_release(&i2c_ctrl->hal_i2c);
    if (s_ret && !ret)
        ret = s_ret;

    if (ret)
    {
        dmsg_i2c_drverr("i2c-%d xfer error: %d\n", i2c_ctrl->hal_i2c.group, ret);
        nmsgs = -1;
        return ret;
    }
    else
    {
        dmsg_i2c_drvwarn("i2c-%d xfer done\n", i2c_ctrl->hal_i2c.group);
    }

    return 0;
}

static int sstar_i2c_probe(struct udevice *bus, unsigned int chip_addr, unsigned int chip_flags)
{
    u8                  val     = 0;
    int                 ret     = 0;
    struct i2c_msg      i2c_msg = {0};
    struct dm_i2c_chip *chip    = dev_get_parent_plat(bus);

    chip->chip_addr             = chip_addr;
    chip->flags                 = chip_flags;
    chip->chip_addr_offset_mask = 0x00;
    chip->offset_len            = 0;

    i2c_msg.addr  = chip_addr;
    i2c_msg.flags = chip_flags;
    i2c_msg.len   = 0;
    i2c_msg.buf   = &val;
    ret |= sstar_i2c_master_xfer(bus, &i2c_msg, 1);

    return ret;
}

static int sstar_i2c_set_bus_speed(struct udevice *bus, unsigned int speed)
{
    struct i2c_control *i2c_ctrl = (struct i2c_control *)dev_get_priv(bus);

    if (!i2c_ctrl)
    {
        dmsg_i2c_drverr("failed to get i2c device pointer\n");
        return -EINVAL;
    }

    i2c_ctrl->hal_i2c.speed = speed;
    sstar_i2c_match_srcclk(i2c_ctrl, i2c_ctrl->hal_i2c.speed);
    hal_i2c_speed_calc(&i2c_ctrl->hal_i2c);
    hal_i2c_cnt_reg_set(&i2c_ctrl->hal_i2c);

    return 0;
}

static int sstar_i2c_get_bus_speed(struct udevice *bus)
{
    struct i2c_control *i2c_ctrl = (struct i2c_control *)dev_get_priv(bus);

    if (!i2c_ctrl)
    {
        dmsg_i2c_drverr("failed to get i2c device pointer\n");
        return -EINVAL;
    }

    return i2c_ctrl->hal_i2c.speed;
}

static int sstar_i2c_of_to_plat(struct udevice *dev)
{
    int                      ret      = 0;
    fdt_addr_t               fdt_addr = 0;
    struct i2c_control *     i2c_ctrl = 0;
    struct hal_i2c_dma_addr *dma_addr = 0;

    i2c_ctrl = (struct i2c_control *)dev_get_priv(dev);
    if (!i2c_ctrl)
    {
        dmsg_i2c_drverr("failed to get i2c device pointer\n");
        return -EINVAL;
    }
    i2c_ctrl->udev = dev;

    if (dev_read_u32(dev, "group", &i2c_ctrl->hal_i2c.group))
    {
        dmsg_i2c_drverr("failed to get i2c bus num\n");
        return -EINVAL;
    }

    fdt_addr = dev_read_addr(dev);
    if (fdt_addr == FDT_ADDR_T_NONE)
    {
        dmsg_i2c_drverr("failed to get i2c-%u addr\n", i2c_ctrl->hal_i2c.group);
        return -EINVAL;
    }
    i2c_ctrl->hal_i2c.bank_addr = (phys_addr_t)fdt_addr;

    i2c_ctrl->hal_i2c.speed                       = I2C_SPEED_STANDARD_RATE;
    i2c_ctrl->hal_i2c.calbak_dma_transfer         = NULL;
    i2c_ctrl->hal_i2c.dma_en                      = dev_read_bool(dev, "dma-enable");
    i2c_ctrl->hal_i2c.clock_count.cnt_stop_hold   = (u16)dev_read_u32_default(dev, "t-hd-sto", 0);
    i2c_ctrl->hal_i2c.clock_count.cnt_start_setup = (u16)dev_read_u32_default(dev, "t-su-sta", 0);
    i2c_ctrl->hal_i2c.clock_count.cnt_start_hold  = (u16)dev_read_u32_default(dev, "t-hd-sta", 0);
    i2c_ctrl->hal_i2c.clock_count.cnt_stop_setup  = (u16)dev_read_u32_default(dev, "t-su-sto", 0);
    i2c_ctrl->hal_i2c.output_mode                 = (u16)dev_read_u32_default(dev, "output-mode", 2);

    ret = sstar_i2c_set_srclk(dev);
    if (ret)
        return ret;
    sstar_i2c_match_srcclk(i2c_ctrl, i2c_ctrl->hal_i2c.speed);

    dma_addr = &i2c_ctrl->hal_i2c.dma_ctrl.dma_addr_msg;
    if (i2c_ctrl->hal_i2c.dma_en)
    {
        dma_addr->dma_virt_addr = (u8 *)malloc_cache_aligned(4096);
        dma_addr->dma_phys_addr = (phys_addr_t)dma_addr->dma_virt_addr;
        dma_addr->dma_miu_addr  = (phys_addr_t)(dma_addr->dma_phys_addr) - MIU0_START_ADDR;
    }

    ret |= hal_i2c_init(&i2c_ctrl->hal_i2c);

    return ret;
}

static const struct dm_i2c_ops sstar_i2c_ops = {
    .xfer          = sstar_i2c_master_xfer,
    .probe_chip    = sstar_i2c_probe,
    .set_bus_speed = sstar_i2c_set_bus_speed,
    .get_bus_speed = sstar_i2c_get_bus_speed,
};

static const struct udevice_id sstar_i2c_ids[] = {{.compatible = "sstar,i2c"}, {}};

U_BOOT_DRIVER(ssatr_i2c) = {
    .name       = "ssatr-i2c",
    .id         = UCLASS_I2C,
    .of_match   = sstar_i2c_ids,
    .of_to_plat = sstar_i2c_of_to_plat,
    .priv_auto  = sizeof(struct i2c_control),
    .ops        = &sstar_i2c_ops,
};
