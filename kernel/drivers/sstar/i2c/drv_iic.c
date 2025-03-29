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

#include <asm/io.h>
#include <linux/cdev.h>
#include <linux/clk.h>
#include <linux/completion.h>
#include <linux/delay.h>
#include <linux/dma-mapping.h>
#include <linux/err.h>
#include <linux/errno.h>
#include <linux/i2c.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/kernel.h>
#include <linux/kthread.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/of_irq.h>
#include <linux/platform_device.h>
#include <ms_platform.h>

#include <cam_os_wrapper.h>
#include <cam_sysfs.h>
#include <drv_iic.h>
#include <gpio.h>
#include <linux/gpio/consumer.h>
#include <hal_iic.h>
#include <hal_iic_reg.h>
#include <drv_gpio.h>
#include <drv_padmux.h>
#include <drv_puse.h>
#include <ms_msys.h>

/*
 * debug mesg
 */
#define err_msg(fmt, ...)                       \
    do                                          \
    {                                           \
        CamOsPrintf("err:" fmt, ##__VA_ARGS__); \
    } while (0)

//#define I2C_DEBUG_MSG
#ifdef I2C_DEBUG_MSG
#define dbg_msg(fmt, ...)                       \
    do                                          \
    {                                           \
        CamOsPrintf("dbg:" fmt, ##__VA_ARGS__); \
    } while (0)
#else
#define dbg_msg(fmt, ...)
#endif

#if !defined(CONFIG_CFI_CLANG)
#if (defined(CONFIG_SSTAR_PADMUX) && defined(CONFIG_SSTAR_GPIO))
#define MIIC_SUPPORT_RECOVER 1
#else
#define MIIC_SUPPORT_RECOVER 0
#endif
#else
#define MIIC_SUPPORT_RECOVER 0
#endif

#define MIIC_DMA_DONE_TIMEOUT    (10000) // millseconds
#define MIIC_DMA_ALLOC_SIZE      (4096)
#define MIIC_DMA_MIU_BASE_OFFSET (0x40)

/*
 * value type
 */
struct i2c_async_msg
{
    u32                    size;
    u8 *                   buffer;
    u8                     task_event;
    u16                    msg_flags;
    void *                 reserved;
    struct task_struct *   async_task;
    sstar_i2c_async_calbck async_callback;
};
struct i2c_control
{
    u32  irq_num;
    u8   irq_name[20];
    u32  use_dma;
    u8   clk_inited;
    u32  srcclk_num;
    u32 *srcclk_rate;
#ifdef CONFIG_CAM_CLK
    void *camclk;
#else
    struct clk *i2c_clk;
#endif
#if MIIC_SUPPORT_RECOVER
    s32                          pad_idx[2];
    s32                          pad_mode[2];
    struct i2c_bus_recovery_info rinfo;
#endif
    struct hal_i2c_ctrl  hal_i2c;
    struct i2c_async_msg async_msg;
    struct device *      dev;
    struct device *      sysdev;
    struct i2c_adapter   adapter;
    CamOsTsem_t          tsem_id;
    struct completion    complete;
};

/*
 * function definition
 */
static DECLARE_WAIT_QUEUE_HEAD(async_i2c_wait);

#if MIIC_SUPPORT_RECOVER
static void drv_i2c_gpio_mode_en(struct i2c_control *para_i2c_ctrl, u8 enable)
{
    if (enable)
    {
        sstar_gpio_pad_set(para_i2c_ctrl->pad_idx[0]);
        sstar_gpio_pad_set(para_i2c_ctrl->pad_idx[1]);
        sstar_gpio_pad_oen(para_i2c_ctrl->pad_idx[0]);
        sstar_gpio_pad_oen(para_i2c_ctrl->pad_idx[1]);
    }
    else
    {
        sstar_gpio_pad_val_set(para_i2c_ctrl->pad_idx[0], para_i2c_ctrl->pad_mode[0]);
        sstar_gpio_pad_val_set(para_i2c_ctrl->pad_idx[1], para_i2c_ctrl->pad_mode[1]);
    }
}

static s32 drv_i2c_gpio_check(struct i2c_control *para_i2c_ctrl)
{
    u8 scl_sta;
    u8 sda_sta;

    if (para_i2c_ctrl->pad_idx[0] == PAD_UNKNOWN || para_i2c_ctrl->pad_idx[1] == PAD_UNKNOWN)
        return -EINVAL;

    sstar_gpio_pad_read(para_i2c_ctrl->pad_idx[0], &scl_sta);
    sstar_gpio_pad_read(para_i2c_ctrl->pad_idx[1], &sda_sta);

    return scl_sta & sda_sta;
}

static s32 drv_i2c_status_check(struct i2c_control *para_i2c_ctrl)
{
    s32 ret;

    ret = drv_i2c_gpio_check(para_i2c_ctrl);
    if (ret < 0)
    {
        dbg_msg("fail to get i2c%u pad index\n", para_i2c_ctrl->hal_i2c.group);
        return 0;
    }
    else if (!ret)
    {
        mdelay(20);
        if (!drv_i2c_gpio_check(para_i2c_ctrl))
        {
            drv_i2c_gpio_mode_en(para_i2c_ctrl, 1);
            ret = i2c_recover_bus(&para_i2c_ctrl->adapter);
            drv_i2c_gpio_mode_en(para_i2c_ctrl, 0);
        }
    }

    return ret;
}
#endif

static void drv_i2c_match_srcclk(struct i2c_control *i2c_ctrl, u32 speed)
{
    u32 i;
    u32 diff;
    u32 min_diff;
    u32 tmp_rate;
    u32 match_rate;
#ifdef CONFIG_CAM_CLK
    CAMCLK_Set_Attribute camclk_attr_set;
#endif

    if (speed <= HAL_I2C_SPEED_100KHZ)
    {
        tmp_rate                     = 12000000;
        i2c_ctrl->hal_i2c.speed_mode = HAL_I2C_SM;
    }
    else if (speed <= HAL_I2C_SPEED_400KHZ)
    {
        tmp_rate                     = 54000000;
        i2c_ctrl->hal_i2c.speed_mode = HAL_I2C_FM;
    }
    else if (speed <= HAL_I2C_SPEED_1000KHZ)
    {
        tmp_rate                     = 72000000;
        i2c_ctrl->hal_i2c.speed_mode = HAL_I2C_FM_PLUS;
    }
    else
    {
        tmp_rate                     = 72000000;
        i2c_ctrl->hal_i2c.speed_mode = HAL_I2C_HS;
    }

    min_diff   = U32_MAX;
    match_rate = i2c_ctrl->srcclk_rate[0];
    for (i = 0; i < i2c_ctrl->srcclk_num; i++)
    {
        diff = abs(i2c_ctrl->srcclk_rate[i] - tmp_rate);
        if ((i2c_ctrl->srcclk_rate[i]) > 1 && (diff < min_diff))
        {
            min_diff   = diff;
            match_rate = i2c_ctrl->srcclk_rate[i];
        }
    }
    i2c_ctrl->hal_i2c.match_rate    = match_rate;
    i2c_ctrl->hal_i2c.clkcnt_per_us = match_rate / 1000000;

#ifdef CONFIG_CAM_CLK
    CAMCLK_SETRATE_ROUNDUP(camclk_attr_set, i2c_ctrl->hal_i2c->match_rate);
    CamClkAttrSet(i2c_ctrl->camclk, &camclk_attr_set);
#else
    clk_set_rate(i2c_ctrl->i2c_clk, i2c_ctrl->hal_i2c.match_rate);
#endif
}

static s32 drv_i2c_set_srclk(struct i2c_control *para_i2c_ctrl)
{
    s32                     ret = 0;
    struct i2c_control *    i2c_ctrl;
    struct hal_i2c_ctrl *   hal_ctrl;
    struct platform_device *plat_dev;

#ifdef CONFIG_CAM_CLK
    u32                  camclk_id;
    u32                  camclk_num_parent;
    u32                  camclk_parent_rate;
    CAMCLK_Set_Attribute camclk_attr_set;
    CAMCLK_Get_Attribute camclk_attr_get;
    char *               camclk_name[32];
    CAMCLK_RET_e         camclk_ret;
    u32                  i;
#else
    struct clk_hw *parent_clk_hw;
    s32            num_parent;
    s32            i;
    u32            parent_rate;
#endif

    i2c_ctrl = para_i2c_ctrl;
    hal_ctrl = &para_i2c_ctrl->hal_i2c;
    plat_dev = container_of(i2c_ctrl->dev, struct platform_device, dev);
    if (!plat_dev)
    {
        err_msg("get i2c-%d platform device pointer err\n", i2c_ctrl->adapter.nr);
        return -HAL_I2C_SRCCLK_SETUP;
    }
#ifdef CONFIG_CAM_CLK

    ret = of_property_read_u32_index(plat_dev->dev.of_node, "camclk", &camclk_id) if (ret)
    {
        err_msg("find i2c-%d camclk failure\n", hal_ctrl->group);
        ret = -HAL_I2C_SRCCLK_SETUP;
        goto out;
    }

    ret = snprintf(camclk_name, sizeof(camclk_name), "i2c-%d camclk", hal_ctrl->group);
    if (!ret)
    {
        err_msg("set i2c-%d camclk name failure\n", );
        ret = -HAL_I2C_SRCCLK_SETUP;
        goto out;
    }

    camclk_ret = CamClkRegister(camclk_name, camclk_id, &(i2c_ctrl->camclk));
    if (camclk_ret)
    {
        err_msg("i2c-%d camclk register failure\n", hal_ctrl->group);
        ret = -HAL_I2C_SRCCLK_SETUP;
        goto out;
    }

    camclk_ret = CamClkAttrGet(i2c_ctrl->camclk, &camclk_attr_get);
    if (camclk_ret)
    {
        err_msg("i2c-%d camclk attribution getting failure\n", hal_ctrl->group);
        ret = -HAL_I2C_SRCCLK_SETUP;
        CamClkUnregister(i2c_ctrl->camclk);
        goto out;
    }

    camclk_num_parent     = camclk_attr_get.u32NodeCount;
    i2c_ctrl->srcclk_num  = camclk_num_parent;
    i2c_ctrl->srcclk_rate = devm_kzalloc(&plat_dev->dev, camclk_num_parent * sizeof(int), GFP_KERNEL);
    if (!i2c_ctrl->srcclk_rate)
    {
        err_msg("devm_kzalloc failed, group:%d!\n", i2c_ctrl->hal_i2c.group);
        ret = -HAL_I2C_SRCCLK_SETUP;
        CamClkUnregister(i2c_ctrl->camclk);
        goto out;
    }
    for (i = 0; i < camclk_num_parent; i++)
    {
        camclk_parent_rate       = CamClkRateGet(camclk_attr_get.u32Parent[i]);
        i2c_ctrl->srcclk_rate[i] = camclk_parent_rate;
        CAMCLK_SETRATE_ROUNDUP(camclk_attr_set, camclk_parent_rate);
    }

    drv_i2c_match_srcclk(i2c_ctrl, hal_ctrl->speed);
#else // else CONFIG_CAM_CLK(no define)

    i2c_ctrl->i2c_clk = of_clk_get(plat_dev->dev.of_node, 0);
    if (IS_ERR_OR_NULL(i2c_ctrl->i2c_clk))
    {
        err_msg("i2c-%d of_clk_get err!\n", i2c_ctrl->adapter.nr);
        ret = -HAL_I2C_SRCCLK_SETUP;
        goto out;
    }

    num_parent            = clk_hw_get_num_parents(__clk_get_hw(i2c_ctrl->i2c_clk));
    i2c_ctrl->srcclk_num  = num_parent;
    i2c_ctrl->srcclk_rate = devm_kzalloc(&plat_dev->dev, num_parent * sizeof(int), GFP_KERNEL);
    if (!i2c_ctrl->srcclk_rate)
    {
        err_msg("devm_kzalloc failed, group:%d!\n", i2c_ctrl->hal_i2c.group);
        ret = -HAL_I2C_SRCCLK_SETUP;
        clk_put(i2c_ctrl->i2c_clk);
        goto out;
    }
    for (i = 0; i < num_parent; i++)
    {
        parent_clk_hw = clk_hw_get_parent_by_index(__clk_get_hw(i2c_ctrl->i2c_clk), i);
        if (IS_ERR_OR_NULL(parent_clk_hw))
        {
            err_msg("i2c-%d clk_hw_get_parent_by_index error!\n", i2c_ctrl->adapter.nr);
            ret = -HAL_I2C_SRCCLK_SETUP;
            clk_put(i2c_ctrl->i2c_clk);
            goto out;
        }
        parent_rate = (u32)clk_hw_get_rate(parent_clk_hw);
        if (!parent_rate)
        {
            err_msg("i2c-%d clk_hw_get_rate error!\n", i2c_ctrl->adapter.nr);
            ret = -HAL_I2C_SRCCLK_SETUP;
            clk_put(i2c_ctrl->i2c_clk);
            goto out;
        }
        i2c_ctrl->srcclk_rate[i] = parent_rate;
        dbg_msg("i2c-%d get parent index-%d rate: %d\n", hal_ctrl->group, i, parent_rate);
        clk_set_rate(i2c_ctrl->i2c_clk, parent_rate);
    }

    drv_i2c_match_srcclk(i2c_ctrl, hal_ctrl->speed);
    ret = clk_prepare_enable(i2c_ctrl->i2c_clk);
    if (ret)
    {
        err_msg("failed to enable i2c clock\n");
        goto out;
    }
#endif
    i2c_ctrl->clk_inited = 1;

out:
    return ret;
}

static s32 drv_i2c_dma_callback(void *para_i2c_base)
{
    struct hal_i2c_ctrl *hal_ctrl = para_i2c_base;
    struct i2c_control * i2c_ctrl = container_of(para_i2c_base, struct i2c_control, hal_i2c);
    u32                  timeout  = 0;
    int                  ret      = 0;

    reinit_completion(&i2c_ctrl->complete);
    hal_i2c_dma_trigger(hal_ctrl);

    if (i2c_ctrl->async_msg.async_callback)
    {
        dbg_msg("i2c-%d, use async mode\n", hal_ctrl->group);
        return HAL_I2C_OK;
    }

    timeout = wait_for_completion_timeout(&i2c_ctrl->complete, msecs_to_jiffies(MIIC_DMA_DONE_TIMEOUT));
    if (!timeout)
    {
        err_msg("i2c-%d DMA TIMEOUT!\n", hal_ctrl->group);
        ret |= hal_i2c_dma_reset(para_i2c_base, 1);
        ret |= hal_i2c_dma_reset(para_i2c_base, 0);
        return -HAL_I2C_TIMEOUT;
    }
    else
    {
        dbg_msg("i2c-%d DMA DONE!\n", hal_ctrl->group);
    }

    return HAL_I2C_OK;
}

static ssize_t drv_i2c_speed_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    int                 ret;
    u32                 speed;
    struct i2c_control *i2c_ctrl = (struct i2c_control *)dev_get_drvdata(dev);

    ret = kstrtou32(buf, 0, &speed);
    if (ret)
    {
        err_msg("correct format: echo [speed] > speed\n");
        return ret;
    }

    CamOsTsemDown(&i2c_ctrl->tsem_id);
    i2c_ctrl->hal_i2c.speed = speed;
    drv_i2c_match_srcclk(i2c_ctrl, i2c_ctrl->hal_i2c.speed);
    hal_i2c_speed_calc(&i2c_ctrl->hal_i2c);
    hal_i2c_cnt_reg_set(&i2c_ctrl->hal_i2c);
    CamOsTsemUp(&i2c_ctrl->tsem_id);

    return count;
}

static ssize_t drv_i2c_speed_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    struct i2c_control *i2c_ctrl = (struct i2c_control *)dev_get_drvdata(dev);

    return sprintf(buf, "%u\n", i2c_ctrl->hal_i2c.speed);
}
DEVICE_ATTR(speed, 0644, drv_i2c_speed_show, drv_i2c_speed_store);

static ssize_t drv_i2c_tsusta_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    int                 ret;
    u32                 ns;
    struct i2c_control *i2c_ctrl = (struct i2c_control *)dev_get_drvdata(dev);

    ret = kstrtou32(buf, 0, &ns);
    if (ret)
    {
        err_msg("correct format: echo [tsusta] > tsusta\n");
        return ret;
    }

    CamOsTsemDown(&i2c_ctrl->tsem_id);
    i2c_ctrl->hal_i2c.clock_attr_time.ns_start_setup = ns;
    hal_i2c_speed_calc(&i2c_ctrl->hal_i2c);
    hal_i2c_cnt_reg_set(&i2c_ctrl->hal_i2c);
    CamOsTsemUp(&i2c_ctrl->tsem_id);

    return count;
}

static ssize_t drv_i2c_tsusta_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    struct i2c_control *i2c_ctrl = (struct i2c_control *)dev_get_drvdata(dev);

    return sprintf(buf, "%u\n", i2c_ctrl->hal_i2c.clock_attr_time.ns_start_setup);
}
DEVICE_ATTR(tsusta, 0644, drv_i2c_tsusta_show, drv_i2c_tsusta_store);

static ssize_t drv_i2c_thdsta_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    int                 ret;
    u32                 ns;
    struct i2c_control *i2c_ctrl = (struct i2c_control *)dev_get_drvdata(dev);

    ret = kstrtou32(buf, 0, &ns);
    if (ret)
    {
        err_msg("correct format: echo [thdsta] > thdsta\n");
        return ret;
    }

    CamOsTsemDown(&i2c_ctrl->tsem_id);
    i2c_ctrl->hal_i2c.clock_attr_time.ns_start_hold = ns;
    hal_i2c_speed_calc(&i2c_ctrl->hal_i2c);
    hal_i2c_cnt_reg_set(&i2c_ctrl->hal_i2c);
    CamOsTsemUp(&i2c_ctrl->tsem_id);

    return count;
}

static ssize_t drv_i2c_thdsta_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    struct i2c_control *i2c_ctrl = (struct i2c_control *)dev_get_drvdata(dev);
    return sprintf(buf, "%u\n", i2c_ctrl->hal_i2c.clock_attr_time.ns_start_hold);
}
DEVICE_ATTR(thdsta, 0644, drv_i2c_thdsta_show, drv_i2c_thdsta_store);

static ssize_t drv_i2c_tsusto_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    int                 ret;
    u32                 ns;
    struct i2c_control *i2c_ctrl = (struct i2c_control *)dev_get_drvdata(dev);

    ret = kstrtou32(buf, 0, &ns);
    if (ret)
    {
        err_msg("correct format: echo [tsusto] > tsusto\n");
        return ret;
    }

    CamOsTsemDown(&i2c_ctrl->tsem_id);
    i2c_ctrl->hal_i2c.clock_attr_time.ns_stop_setup = ns;
    hal_i2c_speed_calc(&i2c_ctrl->hal_i2c);
    hal_i2c_cnt_reg_set(&i2c_ctrl->hal_i2c);
    CamOsTsemUp(&i2c_ctrl->tsem_id);

    return count;
}

static ssize_t drv_i2c_tsusto_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    struct i2c_control *i2c_ctrl = (struct i2c_control *)dev_get_drvdata(dev);
    return sprintf(buf, "%u\n", i2c_ctrl->hal_i2c.clock_attr_time.ns_stop_setup);
}
DEVICE_ATTR(tsusto, 0644, drv_i2c_tsusto_show, drv_i2c_tsusto_store);

static ssize_t drv_i2c_thdsto_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    int                 ret;
    u32                 ns;
    struct i2c_control *i2c_ctrl = (struct i2c_control *)dev_get_drvdata(dev);

    ret = kstrtou32(buf, 0, &ns);
    if (ret)
    {
        err_msg("correct format: echo [thdsto] > thdsto\n");
        return ret;
    }

    CamOsTsemDown(&i2c_ctrl->tsem_id);
    i2c_ctrl->hal_i2c.clock_attr_time.ns_stop_hold = ns;
    hal_i2c_speed_calc(&i2c_ctrl->hal_i2c);
    hal_i2c_cnt_reg_set(&i2c_ctrl->hal_i2c);
    CamOsTsemUp(&i2c_ctrl->tsem_id);

    return count;
}

static ssize_t drv_i2c_thdsto_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    struct i2c_control *i2c_ctrl = (struct i2c_control *)dev_get_drvdata(dev);

    return sprintf(buf, "%u\n", i2c_ctrl->hal_i2c.clock_attr_time.ns_stop_hold);
}
DEVICE_ATTR(thdsto, 0644, drv_i2c_thdsto_show, drv_i2c_thdsto_store);

static ssize_t drv_i2c_tsudat_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    int                 ret;
    u32                 ns;
    struct i2c_control *i2c_ctrl = (struct i2c_control *)dev_get_drvdata(dev);

    ret = kstrtou32(buf, 0, &ns);
    if (ret)
    {
        err_msg("correct format: echo [tsudat] > tsudat\n");
        return ret;
    }

    CamOsTsemDown(&i2c_ctrl->tsem_id);
    i2c_ctrl->hal_i2c.clock_attr_time.ns_data_setup = ns;
    hal_i2c_speed_calc(&i2c_ctrl->hal_i2c);
    hal_i2c_cnt_reg_set(&i2c_ctrl->hal_i2c);
    CamOsTsemUp(&i2c_ctrl->tsem_id);

    return count;
}

static ssize_t drv_i2c_tsudat_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    struct i2c_control *i2c_ctrl = (struct i2c_control *)dev_get_drvdata(dev);

    return sprintf(buf, "%u\n", i2c_ctrl->hal_i2c.clock_attr_time.ns_data_setup);
}
DEVICE_ATTR(tsudat, 0644, drv_i2c_tsudat_show, drv_i2c_tsudat_store);

static ssize_t drv_i2c_thddat_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    int                 ret;
    u32                 ns;
    struct i2c_control *i2c_ctrl = (struct i2c_control *)dev_get_drvdata(dev);

    ret = kstrtou32(buf, 0, &ns);
    if (ret)
    {
        err_msg("correct format: echo [tsudat] > tsudat\n");
        return ret;
    }

    CamOsTsemDown(&i2c_ctrl->tsem_id);
    i2c_ctrl->hal_i2c.clock_attr_time.ns_data_hold = ns;
    hal_i2c_speed_calc(&i2c_ctrl->hal_i2c);
    hal_i2c_cnt_reg_set(&i2c_ctrl->hal_i2c);
    CamOsTsemUp(&i2c_ctrl->tsem_id);

    return count;
}

static ssize_t drv_i2c_thddat_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    struct i2c_control *i2c_ctrl = (struct i2c_control *)dev_get_drvdata(dev);

    return sprintf(buf, "%u\n", i2c_ctrl->hal_i2c.clock_attr_time.ns_data_hold);
}
DEVICE_ATTR(thddat, 0644, drv_i2c_thddat_show, drv_i2c_thddat_store);

static ssize_t drv_i2c_rckdly_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    int                 ret;
    u32                 ns;
    struct i2c_control *i2c_ctrl = (struct i2c_control *)dev_get_drvdata(dev);

    ret = kstrtou32(buf, 0, &ns);
    if (ret)
    {
        err_msg("correct format: echo [rckdly] > rckdly\n");
        return ret;
    }

    CamOsTsemDown(&i2c_ctrl->tsem_id);
    i2c_ctrl->hal_i2c.clock_attr_time.ns_rck_dly = ns;
    hal_i2c_speed_calc(&i2c_ctrl->hal_i2c);
    hal_i2c_cnt_reg_set(&i2c_ctrl->hal_i2c);
    CamOsTsemUp(&i2c_ctrl->tsem_id);

    return count;
}

static ssize_t drv_i2c_rckdly_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    struct i2c_control *i2c_ctrl = (struct i2c_control *)dev_get_drvdata(dev);

    return sprintf(buf, "%u\n", i2c_ctrl->hal_i2c.clock_attr_time.ns_rck_dly);
}
DEVICE_ATTR(rckdly, 0644, drv_i2c_rckdly_show, drv_i2c_rckdly_store);

static s32 drv_i2c_init(struct i2c_control *para_i2c_ctrl)
{
    s32                      ret = 0;
    struct hal_i2c_dma_addr *dma_addr;
    MSYS_DMEM_INFO           i2c_mem_info;

    dma_addr = &para_i2c_ctrl->hal_i2c.dma_ctrl.dma_addr_msg;
    if (para_i2c_ctrl->use_dma && (!dma_addr->dma_virt_addr))
    {
        i2c_mem_info.length = MIIC_DMA_ALLOC_SIZE;
        scnprintf(i2c_mem_info.name, sizeof(i2c_mem_info.name), "Sstar IIC %d DMA", para_i2c_ctrl->adapter.nr);
        ret = msys_request_dmem(&i2c_mem_info);
        if (ret)
        {
            err_msg("i2c-%d alloc mem for dma failed\n", para_i2c_ctrl->adapter.nr);
            goto out;
        }
        dma_addr->dma_virt_addr = i2c_mem_info.kvirt;
        dma_addr->dma_phys_addr = i2c_mem_info.phys;
        dma_addr->dma_miu_addr  = (u64)Chip_Phys_to_MIU((ss_phys_addr_t)dma_addr->dma_phys_addr);
        /*
         *  here offset 0x10,because it may alloc an memory begin miu addr 0x0
         *  if so, as MIIC dma asking for (miu_addr - 1) while asking miu data,
         *  it will hit miu protect, so we add an 0x10 offset
         */
        dma_addr->dma_miu_addr += MIIC_DMA_MIU_BASE_OFFSET;
        dma_addr->dma_phys_addr += MIIC_DMA_MIU_BASE_OFFSET;
        dma_addr->dma_virt_addr += MIIC_DMA_MIU_BASE_OFFSET;
    }

    if (!para_i2c_ctrl->clk_inited)
    {
        ret = drv_i2c_set_srclk(para_i2c_ctrl);
        if (ret)
            goto out;
    }

    ret = hal_i2c_init(&para_i2c_ctrl->hal_i2c);
out:
    return ret;
}

static s32 drv_i2c_deinit(struct i2c_control *para_i2c_ctrl)
{
    struct hal_i2c_dma_addr *dma_addr;
    MSYS_DMEM_INFO           i2c_mem_info;

    dma_addr = &para_i2c_ctrl->hal_i2c.dma_ctrl.dma_addr_msg;
    if (para_i2c_ctrl->use_dma)
    {
        i2c_mem_info.length = MIIC_DMA_ALLOC_SIZE;
        scnprintf(i2c_mem_info.name, sizeof(i2c_mem_info.name), "Sstar IIC %d DMA", para_i2c_ctrl->adapter.nr);
        dma_addr->dma_miu_addr -= MIIC_DMA_MIU_BASE_OFFSET;
        dma_addr->dma_phys_addr -= MIIC_DMA_MIU_BASE_OFFSET;
        dma_addr->dma_virt_addr -= MIIC_DMA_MIU_BASE_OFFSET;
        i2c_mem_info.kvirt = dma_addr->dma_virt_addr;
        i2c_mem_info.phys  = dma_addr->dma_phys_addr;
        msys_release_dmem(&i2c_mem_info);
        dma_addr->dma_miu_addr = 0x0;
    }
    CamOsTsemDeinit(&para_i2c_ctrl->tsem_id);
    i2c_del_adapter(&para_i2c_ctrl->adapter);

    return 0;
}

static u32 drv_i2c_func(struct i2c_adapter *padapter)
{
    return I2C_FUNC_I2C | I2C_FUNC_SMBUS_EMUL | I2C_FUNC_NOSTART;
}

static int drv_i2c_async_thread(void *para_i2c_crtl)
{
    struct i2c_control *i2c_ctrl = (struct i2c_control *)para_i2c_crtl;
    u32                 dam_transfer_cnt;
    u8 *                dma_virt_addr = NULL;

    dma_virt_addr = i2c_ctrl->hal_i2c.dma_ctrl.dma_addr_msg.dma_virt_addr;

    while (!kthread_should_stop())
    {
        wait_event_interruptible(async_i2c_wait, i2c_ctrl->async_msg.task_event);
        i2c_ctrl->async_msg.task_event = 0;

        dam_transfer_cnt = hal_i2c_dma_trans_cnt(&i2c_ctrl->hal_i2c);

        if (dam_transfer_cnt != i2c_ctrl->async_msg.size)
        {
            err_msg("i2c-%d, dma async transfer failed\n", i2c_ctrl->hal_i2c.group);
            i2c_ctrl->async_msg.async_callback(0, i2c_ctrl->async_msg.reserved);
        }
        else
        {
            if (i2c_ctrl->async_msg.msg_flags & I2C_M_RD)
            {
                CamOsMemInvalidate(dma_virt_addr, i2c_ctrl->async_msg.size);
                memcpy(i2c_ctrl->async_msg.buffer, dma_virt_addr, i2c_ctrl->async_msg.size);
                i2c_ctrl->async_msg.size = 0;
            }
            i2c_ctrl->async_msg.async_callback(1, i2c_ctrl->async_msg.reserved);
        }

        CamOsTsemUp(&i2c_ctrl->tsem_id);
    }

    return HAL_I2C_OK;
}

s32 sstar_i2c_async_cb_set(struct i2c_adapter *para_adapter, sstar_i2c_async_calbck para_cb, void *reserved)
{
    struct i2c_control *i2c_ctrl = para_adapter->dev.driver_data;

    if (!i2c_ctrl)
    {
        err_msg("parameter err\n");
        return -HAL_I2C_ERR;
    }

    i2c_ctrl->async_msg.async_callback = para_cb;
    i2c_ctrl->async_msg.reserved       = reserved;
    if (!i2c_ctrl->async_msg.async_task)
    {
        i2c_ctrl->async_msg.async_task = kthread_run(drv_i2c_async_thread, (void *)i2c_ctrl, "i2c_async_thread");
    }
    return HAL_I2C_OK;
}

static void drv_i2c_interrupt(u32 irq, void *para_platform_dev)
{
    s32                 ret;
    struct i2c_control *i2c_ctrl;
    struct device *     dev;

    dev      = para_platform_dev;
    i2c_ctrl = dev->driver_data;

    ret = hal_i2c_dma_done_clr(&i2c_ctrl->hal_i2c, 1);
    if (ret)
    {
        hal_i2c_wn_mode_clr(&i2c_ctrl->hal_i2c);
        if (i2c_ctrl->async_msg.async_callback)
        {
            i2c_ctrl->async_msg.task_event = 1;
            wake_up_interruptible(&async_i2c_wait);
        }
        else
        {
            complete(&(i2c_ctrl->complete));
        }
    }
}
static s32 drv_i2c_remove_srclk(struct platform_device *para_pdev)
{
    s32                 ret;
    struct i2c_control *i2c_ctrl;
    struct clk_hw *     parent_clk_hw;

    i2c_ctrl = platform_get_drvdata(para_pdev);

#ifdef CONFIG_CAM_CLK
    if (i2c_ctrl->camclk)
    {
        CamClkSetOnOff(i2c_ctrl->camclk, 0);
        CamClkUnregister(i2c_ctrl->camclk);
    }

#else
    if (i2c_ctrl->i2c_clk)
    {
        parent_clk_hw = clk_hw_get_parent_by_index(__clk_get_hw(i2c_ctrl->i2c_clk), 0);
        if (IS_ERR_OR_NULL(parent_clk_hw))
        {
            err_msg("i2c-%d clk_hw_get_parent_by_index error!\n", i2c_ctrl->adapter.nr);
            return -HAL_I2C_SRCCLK_SETUP;
        }
        ret = clk_set_parent(i2c_ctrl->i2c_clk, parent_clk_hw->clk);
        if (ret)
        {
            err_msg("i2c-%d clk_set_parent error : %d!\n", i2c_ctrl->adapter.nr, ret);
            return ret;
        }
        clk_disable_unprepare(i2c_ctrl->i2c_clk);
        clk_put(i2c_ctrl->i2c_clk);
    }
#endif

    return 0;
}

s32 sstar_i2c_master_xfer(struct i2c_adapter *para_adapter, struct i2c_msg *para_msg, s32 para_num)
{
    s32                 ret;
    s32                 s_ret;
    s32                 msg_cnt;
    struct i2c_control *i2c_ctrl = para_adapter->dev.driver_data;
    struct i2c_msg *    msgs     = para_msg;

    if (i2c_ctrl->async_msg.async_callback)
    {
        err_msg("async callback exist,do not call sync API or set it NULL\n");
        return -1;
    }

    CamOsTsemDown(&i2c_ctrl->tsem_id);

    hal_i2c_reset(&i2c_ctrl->hal_i2c, 1);
    hal_i2c_reset(&i2c_ctrl->hal_i2c, 0);

#if MIIC_SUPPORT_RECOVER
    if (para_adapter->bus_recovery_info)
    {
        ret = drv_i2c_status_check(i2c_ctrl);
        if (ret < 0)
        {
            err_msg("i2c%u bus lock-up\n", i2c_ctrl->hal_i2c.group);
            CamOsTsemUp(&i2c_ctrl->tsem_id);
            return -EBUSY;
        }
    }
#endif

    for (msg_cnt = 0; msg_cnt < para_num; msg_cnt++, msgs++)
    {
        // dma need set stop format before transfer
        if (i2c_ctrl->hal_i2c.dma_en)
        {
            if (!(msgs->flags & 0x02) && (para_num > 1))
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
            dbg_msg("IN READ \n");
            ret = hal_i2c_read(&i2c_ctrl->hal_i2c, msgs->addr, msgs->buf, (u32)msgs->len, msgs->flags);
        }
        else
        {
            dbg_msg("IN WRITE \n");
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
        err_msg("i2c-%d xfer error: %d\n", para_adapter->nr, ret);
        para_num = -1;
    }
    else
    {
        dbg_msg("OK return xfer\n");
    }
    CamOsTsemUp(&i2c_ctrl->tsem_id);
    return para_num;
}

s32 sstar_i2c_master_async_xfer(struct i2c_adapter *para_adapter, struct i2c_msg *para_msg, s32 para_num)
{
    s32                 ret;
    struct i2c_control *i2c_ctrl = para_adapter->dev.driver_data;
    struct i2c_msg *    msgs     = para_msg;

    if (!i2c_ctrl->async_msg.async_callback)
    {
        err_msg("async callback NULL, please setting first\n");
        ret = -EBUSY;
        goto err_out;
    }

    CamOsTsemDown(&i2c_ctrl->tsem_id);

    hal_i2c_reset(&i2c_ctrl->hal_i2c, 1);
    hal_i2c_reset(&i2c_ctrl->hal_i2c, 0);

#if MIIC_SUPPORT_RECOVER
    if (para_adapter->bus_recovery_info)
    {
        ret = drv_i2c_status_check(i2c_ctrl);
        if (ret < 0)
        {
            err_msg("i2c%u bus lock-up\n", i2c_ctrl->hal_i2c.group);
            CamOsTsemUp(&i2c_ctrl->tsem_id);
            return -EBUSY;
        }
    }
#endif

    if (1 != para_num)
    {
        err_msg("if u want send i2c_msg more than 1,do not use async_xfer\n");
        ret = -EINVAL;
        goto err_out;
    }
    // dma need set stop format before transfer
    if (!i2c_ctrl->hal_i2c.dma_en)
    {
        err_msg("async not support in riu mode\n");
        ret = -EBUSY;
        goto err_out;
    }
    if ((msgs->flags & 0x02) || msgs->flags & I2C_M_RD)
    {
        hal_i2c_dma_stop_set(&i2c_ctrl->hal_i2c, 1);
    }
    else
    {
        hal_i2c_dma_stop_set(&i2c_ctrl->hal_i2c, 0);
    }

    dbg_msg("para number from user is : %d,flags is : 0x%x\n", para_num, msgs->flags);

    i2c_ctrl->async_msg.msg_flags = msgs->flags;
    i2c_ctrl->async_msg.size      = msgs->len;
    if ((msgs->flags) & I2C_M_RD)
    {
        ret                        = hal_i2c_dma_async_read(&i2c_ctrl->hal_i2c, msgs->addr, msgs->buf, (u32)msgs->len);
        i2c_ctrl->async_msg.buffer = msgs->buf;
    }
    else
    {
        ret = hal_i2c_dma_async_write(&i2c_ctrl->hal_i2c, msgs->addr, msgs->buf, (u32)msgs->len);
    }

    return 0;

err_out:
    CamOsTsemUp(&i2c_ctrl->tsem_id);
    return ret;
}

s32 sstar_i2c_wnwrite_xfer(struct i2c_adapter *para_adapter, struct i2c_msg *para_msg, s32 para_num, u8 para_wnlen,
                           u16 para_waitcnt)
{
    s32                 ret;
    struct i2c_control *i2c_ctrl = para_adapter->dev.driver_data;
    struct i2c_msg *    msgs     = para_msg;

    if (i2c_ctrl->async_msg.async_callback)
    {
        err_msg("async callback exist,do not call sync API or set it NULL\n");
        return -EBUSY;
    }

    CamOsTsemDown(&i2c_ctrl->tsem_id);

    hal_i2c_reset(&i2c_ctrl->hal_i2c, 1);
    hal_i2c_reset(&i2c_ctrl->hal_i2c, 0);

#if MIIC_SUPPORT_RECOVER
    if (para_adapter->bus_recovery_info)
    {
        ret = drv_i2c_status_check(i2c_ctrl);
        if (ret < 0)
        {
            err_msg("i2c%u bus lock-up\n", i2c_ctrl->hal_i2c.group);
            CamOsTsemUp(&i2c_ctrl->tsem_id);
            return -EBUSY;
        }
    }
#endif

    if (1 != para_num)
    {
        err_msg("i2c-%d, wn write please send only 1 i2c_msg\n", i2c_ctrl->hal_i2c.group);
        ret = -EINVAL;
        goto err_out;
    }

    if (!i2c_ctrl->use_dma)
    {
        err_msg("i2c-%d, riu mode do not support wn write\n", i2c_ctrl->hal_i2c.group);
        ret = -EBUSY;
        goto err_out;
    }

    if (msgs->flags & I2C_M_RD)
    {
        err_msg("i2c-%d, please do not use wn_write API for reading\n", i2c_ctrl->hal_i2c.group);
        ret = -EINVAL;
        goto err_out;
    }
    else
    {
        ret = hal_i2c_wn_write(&i2c_ctrl->hal_i2c, msgs->addr, msgs->buf, msgs->len, para_wnlen, para_waitcnt);
        if (ret)
        {
            err_msg("i2c-%d, wn write failed\n", i2c_ctrl->hal_i2c.group);
            ret = -EIO;
        }
    }

err_out:
    CamOsTsemUp(&i2c_ctrl->tsem_id);
    return ret;
}

s32 sstar_i2c_wnwrite_async_xfer(struct i2c_adapter *para_adapter, struct i2c_msg *para_msg, s32 para_num,
                                 u8 para_wnlen, u16 para_waitcnt)
{
    s32                 ret      = 0;
    struct i2c_control *i2c_ctrl = para_adapter->dev.driver_data;
    struct i2c_msg *    msgs     = para_msg;

    if (!i2c_ctrl->async_msg.async_callback)
    {
        err_msg("async callback NULL, please setting first\n");
        ret = -EBUSY;
        goto err_out;
    }

    CamOsTsemDown(&i2c_ctrl->tsem_id);

    hal_i2c_reset(&i2c_ctrl->hal_i2c, 1);
    hal_i2c_reset(&i2c_ctrl->hal_i2c, 0);

#if MIIC_SUPPORT_RECOVER
    if (para_adapter->bus_recovery_info)
    {
        ret = drv_i2c_status_check(i2c_ctrl);
        if (ret < 0)
        {
            err_msg("i2c%u bus lock-up\n", i2c_ctrl->hal_i2c.group);
            CamOsTsemUp(&i2c_ctrl->tsem_id);
            return -EBUSY;
        }
    }
#endif

    if (1 != para_num)
    {
        err_msg("i2c-%d, wn write please send only 1 i2c_msg\n", i2c_ctrl->hal_i2c.group);
        ret = -EINVAL;
        goto err_out;
    }

    if (!i2c_ctrl->use_dma)
    {
        err_msg("i2c-%d, riu mode do not support wn write\n", i2c_ctrl->hal_i2c.group);
        ret = -EBUSY;
        goto err_out;
    }

    if (msgs->flags & I2C_M_RD)
    {
        err_msg("i2c-%d, please do not use wn_write API for reading\n", i2c_ctrl->hal_i2c.group);
        ret = -EINVAL;
        goto err_out;
    }
    else
    {
        i2c_ctrl->async_msg.size      = msgs->len;
        i2c_ctrl->async_msg.msg_flags = msgs->flags;
        ret = hal_i2c_wn_async_write(&i2c_ctrl->hal_i2c, msgs->addr, msgs->buf, msgs->len, para_wnlen, para_waitcnt);
        if (ret)
        {
            err_msg("i2c-%d, wn write failed\n", i2c_ctrl->hal_i2c.group);
            ret = -EIO;
            goto err_out;
        }
    }
    return 0;
err_out:
    CamOsTsemUp(&i2c_ctrl->tsem_id);
    return ret;
}

static struct i2c_algorithm gsstr_i2c_algo = {
    .master_xfer   = sstar_i2c_master_xfer,
    .functionality = drv_i2c_func,
};

static s32 drv_i2c_probe(struct platform_device *para_pdev)
{
    struct i2c_control *i2c_ctrl  = NULL;
    u32                 speed     = 0;
    u32                 irq_num   = 0;
    u32                 dma_en    = 0;
    u32                 group     = 0;
    u32                 t_hd_sto  = 0;
    u32                 t_su_sta  = 0;
    u32                 t_hd_sta  = 0;
    u32                 t_su_sto  = 0;
    u32                 t_su_dat  = 0;
    u32                 t_hd_dat  = 0;
    u32                 t_rck_dly = 0;
    u32                 output    = 1;
    void __iomem *      io_addr;
    s32                 ret;
#if MIIC_SUPPORT_RECOVER
    s32 puse[2];
#endif
    struct resource *resource_msg;

    group = 0;

    resource_msg = platform_get_resource(para_pdev, IORESOURCE_MEM, 0);
    if (!resource_msg)
    {
        err_msg("get dev resource -ENOMEM\n");
        ret = -ENOMEM;
        goto err_out;
    }

    i2c_ctrl = devm_kzalloc(&(para_pdev->dev), sizeof(*i2c_ctrl), GFP_KERNEL);
    if (!i2c_ctrl)
    {
        err_msg("devm_kzalloc failed, group:%d!\n", group);
        ret = -ENOMEM;
        goto err_out;
    }

    ret = CamofPropertyReadU32(para_pdev->dev.of_node, "group", &group);
    if (ret)
    {
        err_msg("get property i2c group failed, group:%d!\n", group);
        ret = -ENOENT;
        goto err_out;
    }

    ret = CamofPropertyReadU32(para_pdev->dev.of_node, "speed", &speed);
    if (ret)
    {
        err_msg("get property i2c speed failed, group:%d!\n", group);
        ret = -ENOENT;
        goto err_out;
    }

    dma_en = of_property_read_bool(para_pdev->dev.of_node, "dma-enable");
    if (!dma_en)
    {
        dbg_msg("use fifo mode config, group:%d!\n", group);
    }

    ret = CamofPropertyReadU32(para_pdev->dev.of_node, "output-mode", &output);
    if (ret)
    {
        dbg_msg("get property i2c output-mode failed, group:%d!\n", group);
    }

    ret = CamofPropertyReadU32(para_pdev->dev.of_node, "t-su-sta", &t_su_sta);
    if (ret)
    {
        dbg_msg("get property i2c t-su-sta failed, group:%d!\n", group);
    }

    ret = CamofPropertyReadU32(para_pdev->dev.of_node, "t-hd-sta", &t_hd_sta);
    if (ret)
    {
        dbg_msg("get property i2c t-hd-sta failed, group:%d!\n", group);
    }

    ret = CamofPropertyReadU32(para_pdev->dev.of_node, "t-su-sto", &t_su_sto);
    if (ret)
    {
        dbg_msg("get property i2c t-su-sto failed, group:%d!\n", group);
    }

    ret = CamofPropertyReadU32(para_pdev->dev.of_node, "t-hd-sto", &t_hd_sto);
    if (ret)
    {
        dbg_msg("get property i2c t-hd-sto failed, group:%d!\n", group);
    }

    ret = CamofPropertyReadU32(para_pdev->dev.of_node, "t-su-dat", &t_su_dat);
    if (ret)
    {
        dbg_msg("get property i2c t-su-dat failed, group:%d!\n", group);
    }

    ret = CamofPropertyReadU32(para_pdev->dev.of_node, "t-hd-dat", &t_hd_dat);
    if (ret)
    {
        dbg_msg("get property i2c t-hd-dat failed, group:%d!\n", group);
    }

    ret = CamofPropertyReadU32(para_pdev->dev.of_node, "rd-ack-delay", &t_rck_dly);
    if (ret)
    {
        dbg_msg("get property i2c rd-ack-delay failed, group:%d!\n", group);
    }

    /*master mode and base addr*/
    io_addr                                          = (void *)(IO_ADDRESS(resource_msg->start));
    i2c_ctrl->use_dma                                = dma_en;
    i2c_ctrl->dev                                    = &(para_pdev->dev);
    i2c_ctrl->async_msg.async_callback               = NULL;
    i2c_ctrl->async_msg.async_task                   = NULL;
    i2c_ctrl->async_msg.buffer                       = NULL;
    i2c_ctrl->hal_i2c.output_mode                    = output;
    i2c_ctrl->hal_i2c.dma_en                         = i2c_ctrl->use_dma;
    i2c_ctrl->hal_i2c.group                          = group;
    i2c_ctrl->hal_i2c.speed                          = speed;
    i2c_ctrl->hal_i2c.clock_attr_time.ns_data_setup  = t_su_dat;
    i2c_ctrl->hal_i2c.clock_attr_time.ns_data_hold   = t_hd_dat;
    i2c_ctrl->hal_i2c.clock_attr_time.ns_start_setup = t_su_sta;
    i2c_ctrl->hal_i2c.clock_attr_time.ns_start_hold  = t_hd_sta;
    i2c_ctrl->hal_i2c.clock_attr_time.ns_stop_setup  = t_su_sto;
    i2c_ctrl->hal_i2c.clock_attr_time.ns_stop_hold   = t_hd_sto;
    i2c_ctrl->hal_i2c.clock_attr_time.ns_rck_dly     = t_rck_dly;
    i2c_ctrl->hal_i2c.bank_addr                      = (unsigned long)io_addr;
    i2c_ctrl->hal_i2c.calbak_dma_transfer            = drv_i2c_dma_callback;
    CamOsTsemInit(&i2c_ctrl->tsem_id, 1);

    /*save i2c ctrl struct into device->driver_data*/
    platform_set_drvdata(para_pdev, i2c_ctrl);

    /*i2c adapt device init&add*/
    i2c_ctrl->adapter.owner = THIS_MODULE;
    i2c_ctrl->adapter.class = I2C_CLASS_DEPRECATED;
    ret = scnprintf(i2c_ctrl->adapter.name, sizeof(i2c_ctrl->adapter.name), "Sstar I2C adapter %d", group);
    i2c_ctrl->adapter.algo        = &gsstr_i2c_algo;
    i2c_ctrl->adapter.dev.parent  = &(para_pdev->dev);
    i2c_ctrl->adapter.nr          = group;
    i2c_ctrl->adapter.dev.of_node = para_pdev->dev.of_node;

#if MIIC_SUPPORT_RECOVER
    puse[0]               = drv_padmux_getpuse(PUSE_I2C, group, I2C_SCL);
    puse[1]               = drv_padmux_getpuse(PUSE_I2C, group, I2C_SDA);
    i2c_ctrl->pad_idx[0]  = drv_padmux_getpad(puse[0]);
    i2c_ctrl->pad_idx[1]  = drv_padmux_getpad(puse[1]);
    i2c_ctrl->pad_mode[0] = drv_padmux_getmode(puse[0]);
    i2c_ctrl->pad_mode[1] = drv_padmux_getmode(puse[1]);
    if (i2c_ctrl->pad_idx[0] == PAD_UNKNOWN || i2c_ctrl->pad_idx[1] == PAD_UNKNOWN)
    {
        dbg_msg("fail to get i2c%u pad index\n", group);
        i2c_ctrl->adapter.bus_recovery_info = NULL;
    }
    else
    {
        i2c_ctrl->rinfo.scl_gpiod           = gpio_to_desc(i2c_ctrl->pad_idx[0]);
        i2c_ctrl->rinfo.sda_gpiod           = gpio_to_desc(i2c_ctrl->pad_idx[1]);
        i2c_ctrl->rinfo.recover_bus         = i2c_generic_scl_recovery;
        i2c_ctrl->adapter.bus_recovery_info = &i2c_ctrl->rinfo;
        drv_i2c_gpio_mode_en(i2c_ctrl, 1);
    }
#endif

    i2c_ctrl->sysdev = device_create(msys_get_sysfs_class(), NULL, MKDEV(0, 0), NULL, "i2c%u", group);
    if (i2c_ctrl->sysdev)
    {
        dev_set_drvdata(i2c_ctrl->sysdev, (void *)i2c_ctrl);
        device_create_file(i2c_ctrl->sysdev, &dev_attr_speed);
        device_create_file(i2c_ctrl->sysdev, &dev_attr_tsusta);
        device_create_file(i2c_ctrl->sysdev, &dev_attr_thdsta);
        device_create_file(i2c_ctrl->sysdev, &dev_attr_tsusto);
        device_create_file(i2c_ctrl->sysdev, &dev_attr_thdsto);
        device_create_file(i2c_ctrl->sysdev, &dev_attr_tsudat);
        device_create_file(i2c_ctrl->sysdev, &dev_attr_thddat);
        device_create_file(i2c_ctrl->sysdev, &dev_attr_rckdly);
    }
    else
    {
        dbg_msg("fail to create sysfs attribute file for i2c%u device\n", group);
    }

    ret = drv_i2c_init(i2c_ctrl);
    if (ret)
    {
        err_msg("i2c%u init failed\n", group);
        goto err_out;
    }

    i2c_set_adapdata(&i2c_ctrl->adapter, i2c_ctrl);

    ret = i2c_add_numbered_adapter(&i2c_ctrl->adapter);
    if (ret)
    {
        err_msg("add adapter err,group : %u\n", group);
        goto err_out;
    }

    if (i2c_ctrl->use_dma)
    {
        irq_num = CamIrqOfParseAndMap(para_pdev->dev.of_node, 0);
        if (irq_num == 0)
        {
            err_msg("can't find interrupts property, group:%d!\n", group);
            ret = -ENOENT;
            goto err_adap;
        }

        if (!snprintf(i2c_ctrl->irq_name, sizeof(i2c_ctrl->irq_name), "i2c%d_Isr", group))
        {
            err_msg("find irq reformat failed, group:%d!\n", group);
            ret = -ENOENT;
            goto err_adap;
        }

        ret = CamOsIrqRequest(irq_num, drv_i2c_interrupt, i2c_ctrl->irq_name, (void *)&para_pdev->dev);
        if (ret == 0)
        {
            dbg_msg("%d registered\n", irq_num);
        }
        else
        {
            err_msg("%d register failed\n", irq_num);
            ret = -ENOENT;
            goto err_adap;
        }
        i2c_ctrl->irq_num = irq_num;
        init_completion(&i2c_ctrl->complete);
    }

#if MIIC_SUPPORT_RECOVER
    drv_i2c_gpio_mode_en(i2c_ctrl, 0);
#endif

    return 0;

err_adap:
    i2c_del_adapter(&i2c_ctrl->adapter);
err_out:
    return ret;
}

static s32 drv_i2c_remove(struct platform_device *para_pdev)
{
    struct i2c_control *i2c_ctrl;
    s32                 ret = 0;

    i2c_ctrl = (struct i2c_control *)platform_get_drvdata(para_pdev);
    if (!i2c_ctrl)
    {
        err_msg("i2c control is NULL!\n");
        return -1;
    }

    if (i2c_ctrl->sysdev)
        device_unregister(i2c_ctrl->sysdev);
    ret |= drv_i2c_deinit(i2c_ctrl);
    if (i2c_ctrl->use_dma)
    {
        CamOsIrqFree(i2c_ctrl->irq_num, (void *)&para_pdev->dev);
    }
    ret |= drv_i2c_remove_srclk(para_pdev);

    return ret;
}

#ifdef CONFIG_PM_SLEEP
static s32 drv_i2c_suspend(struct device *dev)
{
    int                 ret = 0;
    struct i2c_control *i2c_ctrl;

    i2c_ctrl = dev_get_drvdata(dev);
    if (!i2c_ctrl)
    {
        err_msg("i2c control is NULL!\n");
        return -1;
    }

    CamOsTsemDeinit(&i2c_ctrl->tsem_id);
    hal_i2c_dma_intr_en(&i2c_ctrl->hal_i2c, 0);
    hal_i2c_dma_reset(&i2c_ctrl->hal_i2c, 1);
    hal_i2c_reset(&i2c_ctrl->hal_i2c, 1);

    return ret;
}

static s32 drv_i2c_resume(struct device *dev)
{
    s32                 ret;
    struct i2c_control *i2c_ctrl;

    i2c_ctrl = dev_get_drvdata(dev);
    if (!i2c_ctrl)
    {
        err_msg("i2c control is NULL!\n");
        return -1;
    }

    ret = drv_i2c_init(i2c_ctrl);
    drv_i2c_match_srcclk(i2c_ctrl, i2c_ctrl->hal_i2c.speed);
    CamOsTsemInit(&i2c_ctrl->tsem_id, 1);

    return ret;
}

#endif // end #ifdef CONFIG_PM_SLEEP

static const struct of_device_id i2c_of_device_id[] = {
    {.compatible = "sstar,i2c", 0},
    {},
};
MODULE_DEVICE_TABLE(of, i2c_of_device_id);

#ifdef CONFIG_PM_SLEEP
static const struct dev_pm_ops sstar_i2c_pm_ops = {
    .suspend_late = drv_i2c_suspend,
    .resume_early = drv_i2c_resume,
};
#endif /* CONFIG_PM_SLEEP */

static struct platform_driver i2c_platform_driver = {
    .probe  = drv_i2c_probe,
    .remove = drv_i2c_remove,
    .driver =
        {
            .name           = "sstar,i2c",
            .owner          = THIS_MODULE,
            .of_match_table = i2c_of_device_id,
#ifdef CONFIG_PM_SLEEP
            .pm = &sstar_i2c_pm_ops,
#endif /* CONFIG_PM_SLEEP */
        },
};

static s32 __init drv_i2c_init_driver(void)
{
    dbg_msg("init driver\n");
    return CamPlatformDriverRegister(&i2c_platform_driver);
}

static void __exit drv_i2c_exit_driver(void)
{
    CamPlatformDriverUnregister(&i2c_platform_driver);
}
module_init(drv_i2c_init_driver);
module_exit(drv_i2c_exit_driver);

MODULE_DESCRIPTION("Sstar I2C Bus Controller driver");
MODULE_AUTHOR("SSTAR");
MODULE_LICENSE("GPL v2");
