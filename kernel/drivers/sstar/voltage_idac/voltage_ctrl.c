/*
 * voltage_ctrl.c- Sigmastar
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
#include <linux/kernel.h>
#include <linux/vmalloc.h>
#include <linux/of.h>
#include <linux/device.h>
#include <linux/module.h>
#include <linux/gpio.h>
#include <linux/list.h>
#include <linux/slab.h>
#include <linux/platform_device.h>

#include <registers.h>
#include <ms_platform.h>
#include <drv_gpio.h>
#include "voltage_ctrl.h"
#include "voltage_ctrl_demander.h"
#include "voltage_request_init.h"
#if defined(CONFIG_SS_DUALOS) && defined(CONFIG_SSTAR_VOLTAGE_IDAC_CTRL_BY_RTOS)
#include <cam_inter_os.h>
#include <drv_dualos.h>
#endif

#define VOLCTRL_DEBUG 0

#if VOLCTRL_DEBUG
#define VOLCTRL_DBG(fmt, arg...) printk(KERN_INFO fmt, ##arg)
#else
#define VOLCTRL_DBG(fmt, arg...)
#endif
#define VOLCTRL_ERR(fmt, arg...) printk(KERN_ERR fmt, ##arg)

struct voltage_ctrl
{
    const char *            name;
    u32                     current_voltage;
    u8                      voltage_gpio_inited;
    struct device           dev_core;
    u8                      enable_scaling_voltage;
    struct platform_device *pdev;
    struct mutex            voltage_mutex;
    u32                     gpio_vid_pin;
    u32                     idac_max_voltage;
    u32                     idac_min_voltage;
    u32                     idac_base_vol;
    u32                     voltage_demander_request_value[VOLTAGE_DEMANDER_MAX];
    struct list_head        list;
};

const char *voltage_demander_name[] = {FOREACH_DEMANDER(GENERATE_STRING)};

static LIST_HEAD(voltage_ctrl_list);

static u8 voltage_subsys_registered;

static struct bus_type voltage_subsys = {
    .name     = "voltage",
    .dev_name = "voltage",
};

#if defined(CONFIG_SS_DUALOS) && defined(CONFIG_SSTAR_VOLTAGE_IDAC_CTRL_BY_RTOS)
static int controller_name_to_id(const char *name)
{
    if (!strcmp(name, "core_power"))
        return VOLTAGE_CONTROLLER_CORE;

    if (!strcmp(name, "cpu_power"))
        return VOLTAGE_CONTROLLER_CPU;

    return -1;
}
#endif

struct voltage_ctrl *get_voltage_ctrl(const char *name)
{
    struct voltage_ctrl *ctrl = NULL;
    list_for_each_entry(ctrl, &voltage_ctrl_list, list)
    {
        if (!strcmp(ctrl->name, name))
        {
            return ctrl;
        }
    }
    return NULL;
}

static int check_voltage_valid(struct voltage_ctrl *ctrl, u32 mV)
{
    if (mV > ctrl->idac_max_voltage)
    {
        mV = ctrl->idac_max_voltage;
    }
    else if (mV < ctrl->idac_min_voltage)
    {
        mV = ctrl->idac_min_voltage;
    }
    return mV;
}

static void sync_core_voltage(struct voltage_ctrl *ctrl)
{
    u8  i         = 0;
    u32 vcore_max = 0;

    mutex_lock(&ctrl->voltage_mutex);

    if (ctrl->enable_scaling_voltage)
    {
        for (i = 0; i < VOLTAGE_DEMANDER_MAX; i++)
        {
            vcore_max = max(vcore_max, ctrl->voltage_demander_request_value[i]);
        }
        if (!vcore_max)
            goto _skip_sync;
        vcore_max = check_voltage_valid(ctrl, vcore_max);
        VOLCTRL_DBG("[Core Voltage] %s: maximum request is %dmV\n", __FUNCTION__, vcore_max);

        idac_set_voltage(ctrl->name, ctrl->idac_base_vol, vcore_max);

        ctrl->current_voltage = vcore_max;
    }
    else
    {
        VOLCTRL_DBG("[Core Voltage] %s: voltage scaling not enable\n", __FUNCTION__);
    }

    VOLCTRL_DBG("[Core Voltage] %s: current core voltage %dmV\n", __FUNCTION__, ctrl->current_voltage);

_skip_sync:
    mutex_unlock(&ctrl->voltage_mutex);
}

int set_core_voltage(const char *name, VOLTAGE_DEMANDER_E demander, u32 mV)
{
    struct voltage_ctrl *ctrl;
#if defined(CONFIG_SS_DUALOS) && defined(CONFIG_SSTAR_VOLTAGE_IDAC_CTRL_BY_RTOS)
    int controller;
#endif
    u8  i         = 0;
    u32 vcore_max = 0;

    if (demander >= VOLTAGE_DEMANDER_MAX)
    {
        VOLCTRL_DBG("[Core Voltage] %s: demander number out of range (%d)\n", __FUNCTION__, demander);
        return -EINVAL;
    }

    ctrl = get_voltage_ctrl(name);
    if (!ctrl)
    {
        VOLCTRL_DBG("[Core Voltage] %s: volage ctrl is null (%s)\n", __FUNCTION__, name);
        return -EINVAL;
    }
#if defined(CONFIG_SS_DUALOS) && defined(CONFIG_SSTAR_VOLTAGE_IDAC_CTRL_BY_RTOS)
    if (dualos_rtos_exist())
    {
        if (ctrl->enable_scaling_voltage)
        {
            controller = controller_name_to_id(name);
            if (controller < 0)
            {
                VOLCTRL_DBG("[Core Voltage] %s: invalid controller name %s\n", __FUNCTION__, name);
                return -EINVAL;
            }

            CamInterOsSignal(INTEROS_SC_L2R_CORE_VOLTAGE_SET, (u32)controller, (u32)demander, (u32)mV);
        }
    }
    else
#endif
    {
        mutex_lock(&ctrl->voltage_mutex);

        ctrl->voltage_demander_request_value[demander] = mV;
        VOLCTRL_DBG("[Core Voltage] %s: %s request %dmV\n", __FUNCTION__, voltage_demander_name[demander], mV);

        if (ctrl->enable_scaling_voltage)
        {
            for (i = 0; i < VOLTAGE_DEMANDER_MAX; i++)
            {
                vcore_max = max(vcore_max, ctrl->voltage_demander_request_value[i]);
            }
            vcore_max = check_voltage_valid(ctrl, vcore_max);
            VOLCTRL_DBG("[Core Voltage] %s: maximum request is %dmV\n", __FUNCTION__, vcore_max);

            idac_set_voltage(ctrl->name, ctrl->idac_base_vol, vcore_max);

            ctrl->current_voltage = vcore_max;
        }
        else
        {
            VOLCTRL_DBG("[Core Voltage] %s: voltage scaling not enable\n", __FUNCTION__);
        }

        VOLCTRL_DBG("[Core Voltage] %s: current core voltage %dmV\n", __FUNCTION__, ctrl->current_voltage);

        mutex_unlock(&ctrl->voltage_mutex);
    }

    return 0;
}
EXPORT_SYMBOL(set_core_voltage);

int get_core_voltage(const char *name, u32 *mV)
{
#if defined(CONFIG_SS_DUALOS) && defined(CONFIG_SSTAR_VOLTAGE_IDAC_CTRL_BY_RTOS)
    int controller;
#endif
    struct voltage_ctrl *ctrl;

    ctrl = get_voltage_ctrl(name);
    if (!ctrl)
    {
        VOLCTRL_DBG("[Core Voltage] %s: voltage ctrl is null (%s)\n", __FUNCTION__, name);
        return -EINVAL;
    }

#if defined(CONFIG_SS_DUALOS) && defined(CONFIG_SSTAR_VOLTAGE_IDAC_CTRL_BY_RTOS)
    if (dualos_rtos_exist())
    {
        controller = controller_name_to_id(name);
        if (controller < 0)
        {
            VOLCTRL_DBG("[Core Voltage] %s: invalid controller name %s\n", __FUNCTION__, name);
            return -EINVAL;
        }
        *mV = (int)CamInterOsSignal(INTEROS_SC_L2R_CORE_VOLTAGE_GET, (u32)controller, 0, 0);
    }
    else
#endif
    {
        if (idac_get_voltage(name, ctrl->idac_base_vol, &ctrl->current_voltage))
        {
            VOLCTRL_DBG("[Core Voltage] %s: voltage get fail (%s)\n", __FUNCTION__, name);
            return -EIO;
        }
        *mV = ctrl->current_voltage;
    }

    return 0;
}
EXPORT_SYMBOL(get_core_voltage);

static ssize_t show_scaling_voltage(struct device *dev, struct device_attribute *attr, char *buf)
{
    char *               str = buf;
    char *               end = buf + PAGE_SIZE;
    struct voltage_ctrl *ctrl;

    ctrl = dev_get_drvdata(dev);
    if (!ctrl)
    {
        VOLCTRL_ERR("[Core Voltage] %s: volage ctrl is null\n", __FUNCTION__);
        str += scnprintf(str, end - str, "%s\n", "get ctrl error");
        return (str - buf);
    }

    mutex_lock(&ctrl->voltage_mutex);
    str += scnprintf(str, end - str, "%d\n", ctrl->enable_scaling_voltage);
    mutex_unlock(&ctrl->voltage_mutex);

    return (str - buf);
}

static ssize_t store_scaling_voltage(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    u32                  enable;
    struct voltage_ctrl *ctrl;

    if (sscanf(buf, "%d", &enable) <= 0)
        return 0;

    ctrl = dev_get_drvdata(dev);
    if (!ctrl)
    {
        VOLCTRL_ERR("[Core Voltage] %s: volage ctrl is null\n", __FUNCTION__);
        return count;
    }

    mutex_lock(&ctrl->voltage_mutex);
    if (enable)
    {
        ctrl->enable_scaling_voltage = 1;
        VOLCTRL_DBG("[Core Voltage] %s: scaling ON\n", __FUNCTION__);
    }
    else
    {
        ctrl->enable_scaling_voltage = 0;
        VOLCTRL_DBG("[Core Voltage] %s: scaling OFF\n", __FUNCTION__);
    }
    mutex_unlock(&ctrl->voltage_mutex);

#if defined(CONFIG_SS_DUALOS) && defined(CONFIG_SSTAR_VOLTAGE_IDAC_CTRL_BY_RTOS)
    if (!dualos_rtos_exist())
#endif
    {
        sync_core_voltage(ctrl);
    }

    return count;
}
DEVICE_ATTR(scaling_voltage, 0644, show_scaling_voltage, store_scaling_voltage);

static ssize_t show_voltage_available(struct device *dev, struct device_attribute *attr, char *buf)
{
    char *               str = buf;
    char *               end = buf + PAGE_SIZE;
    struct voltage_ctrl *ctrl;

    ctrl = dev_get_drvdata(dev);
    if (!ctrl)
    {
        VOLCTRL_ERR("[Core Voltage] %s: volage ctrl is null\n", __FUNCTION__);
        str += scnprintf(str, end - str, "%s\n", "get ctrl error");
        return (str - buf);
    }

    str += scnprintf(str, end - str, "Vmin(mV)\tVmax(mV)\n");
    str += scnprintf(str, end - str, "%d\t\t%d\n", ctrl->idac_min_voltage, ctrl->idac_max_voltage);

    return (str - buf);
}
DEVICE_ATTR(voltage_available, 0444, show_voltage_available, NULL);

static ssize_t show_voltage_current(struct device *dev, struct device_attribute *attr, char *buf)
{
#if defined(CONFIG_SS_DUALOS) && defined(CONFIG_SSTAR_VOLTAGE_IDAC_CTRL_BY_RTOS)
    u32 mV;
    int controller;
#endif
    u8                   i;
    char *               str = buf;
    char *               end = buf + PAGE_SIZE;
    u32                  voltage_mV;
    struct voltage_ctrl *ctrl;

    ctrl = dev_get_drvdata(dev);
    if (!ctrl)
    {
        VOLCTRL_ERR("[Core Voltage] %s: volage ctrl is null\n", __FUNCTION__);
        str += scnprintf(str, end - str, "%s\n", "get ctrl error");
        return (str - buf);
    }

    if (!get_core_voltage(ctrl->name, &voltage_mV))
    {
        str += scnprintf(str, end - str, "%d\n", voltage_mV);
    }
    else
    {
        str += scnprintf(str, end - str, "Get core voltage fail\n");
    }

#if defined(CONFIG_SS_DUALOS) && defined(CONFIG_SSTAR_VOLTAGE_IDAC_CTRL_BY_RTOS)
    if (dualos_rtos_exist())
    {
        controller = controller_name_to_id(ctrl->name);
        if (controller < 0)
        {
            VOLCTRL_ERR("[Core Voltage] %s: invalid controller name %s\n", __FUNCTION__, ctrl->name);
            str += scnprintf(str, end - str, "invalid controller name %s\n", ctrl->name);
            return (str - buf);
        }

        for (i = 0; i < VOLTAGE_DEMANDER_MAX; i++)
        {
            mV = (int)CamInterOsSignal(INTEROS_SC_L2R_CORE_VOLTAGE_GET, (u32)controller, 1, i);

            if (mV)
                str += scnprintf(str, end - str, "    %-32s%d\n", voltage_demander_name[i], mV);
            else
                str += scnprintf(str, end - str, "    %-32s-\n", voltage_demander_name[i]);
        }
    }
    else
#endif
    {
        for (i = 0; i < VOLTAGE_DEMANDER_MAX; i++)
        {
            if (ctrl->voltage_demander_request_value[i])
                str += scnprintf(str, end - str, "    %-32s%d\n", voltage_demander_name[i],
                                 ctrl->voltage_demander_request_value[i]);
            else
                str += scnprintf(str, end - str, "    %-32s-\n", voltage_demander_name[i]);
        }
    }

    return (str - buf);
}

static ssize_t store_voltage_current(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    u32                  voltage = 0;
    struct voltage_ctrl *ctrl;
#if defined(CONFIG_SS_DUALOS) && defined(CONFIG_SSTAR_VOLTAGE_IDAC_CTRL_BY_RTOS)
    int controller;
#endif

    if (sscanf(buf, "%d", &voltage) <= 0)
        return 0;

    ctrl = dev_get_drvdata(dev);
    if (!ctrl)
    {
        VOLCTRL_ERR("[Core Voltage] %s: volage ctrl is null\n", __FUNCTION__);
        return count;
    }
#if defined(CONFIG_SS_DUALOS) && defined(CONFIG_SSTAR_VOLTAGE_IDAC_CTRL_BY_RTOS)
    if (dualos_rtos_exist())
    {
        controller = controller_name_to_id(ctrl->name);
        if (controller < 0)
        {
            VOLCTRL_ERR("[Core Voltage] %s: invalid controller name %s\n", __FUNCTION__, ctrl->name);
            return count;
        }

        voltage =
            (int)CamInterOsSignal(INTEROS_SC_L2R_CORE_VOLTAGE_SET, (u32)controller, VOLTAGE_DEMANDER_USER, voltage);

        if (!voltage)
            VOLCTRL_ERR("%s: ERR-> %s @ %d\n", __func__, voltage_demander_name[VOLTAGE_DEMANDER_USER], voltage);
    }
    else
#endif
    {
        if (ctrl->enable_scaling_voltage)
            set_core_voltage(ctrl->name, VOLTAGE_DEMANDER_USER, voltage);
        else
            pr_err("[Core Voltage] voltage scaling not enable\n");
    }

    return count;
}
DEVICE_ATTR(voltage_current, 0644, show_voltage_current, store_voltage_current);

static ssize_t show_vid_gpio_map(struct device *dev, struct device_attribute *attr, char *buf)
{
    const u8 *           name;
    char *               str = buf;
    char *               end = buf + PAGE_SIZE;
    struct voltage_ctrl *ctrl;

    ctrl = dev_get_drvdata(dev);
    if (!ctrl)
    {
        VOLCTRL_ERR("[Core Voltage] %s: volage ctrl is null\n", __FUNCTION__);
        str += scnprintf(str, end - str, "%s\n", "get ctrl error");
        return (str - buf);
    }

    if (!sstar_gpio_num_to_name(ctrl->gpio_vid_pin, &name))
    {
        str += scnprintf(str, end - str, "vid_%d=%d (%s)\n", 0, ctrl->gpio_vid_pin, name);
    }
    else
    {
        str += scnprintf(str, end - str, "vid_%d=%d\n", 0, ctrl->gpio_vid_pin);
    }

    return (str - buf);
}
DEVICE_ATTR(vid_gpio_map, 0444, show_vid_gpio_map, NULL);

static int idac_get_gpio(struct voltage_ctrl *ctrl)
{
    struct device_node *np = ctrl->pdev->dev.of_node;
    char                name[10];
    int                 ret;

    mutex_lock(&ctrl->voltage_mutex);

    if (0 != of_property_read_u32(np, "vid_gpio", &ctrl->gpio_vid_pin))
        goto voltage_get_gpio_err;

    sprintf(name, "vid%d", ctrl->gpio_vid_pin);
    ret = gpio_request(ctrl->gpio_vid_pin, (const char *)name);
    if (ret)
        goto voltage_get_gpio_err;
    gpio_export(ctrl->gpio_vid_pin, 0);
    gpio_direction_input(ctrl->gpio_vid_pin);

    /* idac - set gpio to analog mode for idac control*/
    if (0 != idac_set_gpio_analog_mode(ctrl->gpio_vid_pin))
        goto voltage_get_gpio_err;

    ctrl->voltage_gpio_inited = 1;

    mutex_unlock(&ctrl->voltage_mutex);

    return 0;

voltage_get_gpio_err:
    ctrl->gpio_vid_pin = 0;

    mutex_unlock(&ctrl->voltage_mutex);
    return -1;
}

static int idac_init(struct voltage_ctrl *ctrl)
{
    int                 ret;
    struct device_node *np = ctrl->pdev->dev.of_node;

    mutex_lock(&ctrl->voltage_mutex);

    if (0 != of_property_read_u32(np, "base_voltage", &ctrl->idac_base_vol))
    {
        VOLCTRL_ERR("[Voltage] %s: base_voltage is null\n", __FUNCTION__);
        goto voltage_get_base_vol_err;
    }

    ret = idac_get_max_vol_offset(ctrl->name);
    if (ret < 0)
    {
        goto voltage_idac_init_err;
    }
    ctrl->idac_max_voltage = ctrl->idac_base_vol + ret;
    ctrl->idac_min_voltage = ctrl->idac_base_vol - ret;

#if defined(CONFIG_SS_DUALOS) && defined(CONFIG_SSTAR_VOLTAGE_IDAC_CTRL_BY_RTOS)
    if (!dualos_rtos_exist())
#endif
    {
        if (idac_init_one(ctrl->name))
        {
            goto voltage_idac_init_err;
        }
    }

    mutex_unlock(&ctrl->voltage_mutex);

    return 0;

voltage_get_base_vol_err:
voltage_idac_init_err:

    ctrl->idac_base_vol    = 0;
    ctrl->idac_max_voltage = 0;
    ctrl->idac_min_voltage = 0;

    mutex_unlock(&ctrl->voltage_mutex);
    return -1;
}

static int sstar_voltage_idac_ctrl_probe(struct platform_device *pdev)
{
    int                  ret = 0;
    struct voltage_ctrl *ctrl;

    ctrl = kzalloc(sizeof(struct voltage_ctrl), GFP_KERNEL);
    if (!ctrl)
    {
        ret = -ENOMEM;
        goto mem_err;
    }

    ctrl->pdev = pdev;
    ctrl->name = pdev->dev.of_node->name;

    ctrl->dev_core.kobj.name = ctrl->name;
    ctrl->dev_core.bus       = &voltage_subsys;

    mutex_init(&ctrl->voltage_mutex);

#if defined(CONFIG_SS_DUALOS) && defined(CONFIG_SSTAR_VOLTAGE_IDAC_CTRL_BY_RTOS)
    if (!dualos_rtos_exist())
#endif
    {
        ret = idac_get_gpio(ctrl);
        if (ret)
        {
            VOLCTRL_ERR("Failed to get gpio for voltage control!! %d\n", ret);
            goto voltage_control_init_err;
        }
    }

    ret = idac_init(ctrl);
    if (ret)
    {
        VOLCTRL_ERR("Failed to do idac_init!! %d\n", ret);
        goto voltage_control_init_err;
    }

    VOLCTRL_DBG("[Core Voltage] %s: register sub system\n", __FUNCTION__);

    if (!voltage_subsys_registered)
    {
        ret = subsys_system_register(&voltage_subsys, NULL);
        if (ret)
        {
            VOLCTRL_ERR("Failed to register voltage sub system!! %d\n", ret);
            goto voltage_control_init_err;
        }
        voltage_subsys_registered = 1;
    }

    ret = device_register(&ctrl->dev_core);
    if (ret)
    {
        VOLCTRL_ERR("Failed to register voltage core device!! %d\n", ret);
        goto voltage_control_init_err;
    }

    platform_set_drvdata(pdev, ctrl);
    dev_set_drvdata(&ctrl->dev_core, ctrl);

    list_add_tail(&ctrl->list, &voltage_ctrl_list);

    device_create_file(&ctrl->dev_core, &dev_attr_scaling_voltage);
    device_create_file(&ctrl->dev_core, &dev_attr_voltage_available);
#if defined(CONFIG_SS_DUALOS) && defined(CONFIG_SSTAR_VOLTAGE_IDAC_CTRL_BY_RTOS)
    if (!dualos_rtos_exist())
#endif
    {
        device_create_file(&ctrl->dev_core, &dev_attr_vid_gpio_map);
    }

    device_create_file(&ctrl->dev_core, &dev_attr_voltage_current);

    // Initialize voltage request for specific IP by chip
    voltage_request_chip(ctrl->name);

    // Enable core voltage scaling
    VOLCTRL_DBG("[Core Voltage] %s: turn-on core voltage scaling\n", __FUNCTION__);
    ctrl->enable_scaling_voltage = 1;

#if defined(CONFIG_SS_DUALOS) && defined(CONFIG_SSTAR_VOLTAGE_IDAC_CTRL_BY_RTOS)
    if (!dualos_rtos_exist())
#endif
    {
        sync_core_voltage(ctrl);
    }

    return 0;

voltage_control_init_err:
mem_err:
    kfree(ctrl);
    return ret;
}

#ifdef CONFIG_PM_SLEEP
static int sstar_voltage_idac_ctrl_resume(struct platform_device *dev)
{
    struct voltage_ctrl *ctrl = platform_get_drvdata(dev);
#if defined(CONFIG_SS_DUALOS) && defined(CONFIG_SSTAR_VOLTAGE_IDAC_CTRL_BY_RTOS)
    if (!dualos_rtos_exist())
#endif
    {
        idac_set_gpio_analog_mode(ctrl->gpio_vid_pin);
        idac_init_one(ctrl->name);
        sync_core_voltage(ctrl);
    }
    return 0;
}
#endif /* CONFIG_PM_SLEEP */

static const struct of_device_id sstar_voltage_ctrl_of_match_table[] = {{.compatible = "sstar,voltage-idac-ctrl"}, {}};
MODULE_DEVICE_TABLE(of, sstar_voltage_ctrl_of_match_table);

static struct platform_driver sstar_voltage_idac_ctrl_driver = {
    .probe = sstar_voltage_idac_ctrl_probe,
#ifdef CONFIG_PM_SLEEP
    .resume = sstar_voltage_idac_ctrl_resume,
#endif
    .driver =
        {
            .name           = "sstar,voltage-idac-ctrl",
            .owner          = THIS_MODULE,
            .of_match_table = sstar_voltage_ctrl_of_match_table,
        },
};

builtin_platform_driver(sstar_voltage_idac_ctrl_driver);

MODULE_AUTHOR("SSTAR");
MODULE_DESCRIPTION("SStar Voltage IDAC Control Driver");
MODULE_LICENSE("GPL v2");
