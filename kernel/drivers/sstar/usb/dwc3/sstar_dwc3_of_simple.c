/*
 * sstar_dwc3_of_simple.c- Sigmastar
 *
 * Copyright (c) [2019~2021] SigmaStar Technology.
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

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/platform_device.h>
#include <linux/dma-mapping.h>
#include <linux/clk.h>
#include <linux/of.h>
#include <linux/of_platform.h>
#include <linux/pm_runtime.h>
#include <linux/reset.h>
#include <linux/io.h>
#include "core.h"
#include "io.h"
#include <linux/regmap.h>
#include <linux/mfd/syscon.h>
#include "sstar-dwc3-of-simple.h"
#include "sstar-dwc3-debugfs.h"
#include <linux/timer.h>
#include "sstar_usb3_phy_u3phy.h"

extern unsigned long lx_mem_size;

u32 global_debug_ltssm[256] = {0};
u8  ltssm_index             = 0;

#define INREG16(x)                    (*((volatile unsigned short *)(x)))
#define OUTREG16(x, y)                (*((volatile unsigned short *)(x)) = (y))
#define CLRREG16(x, y)                OUTREG16((x), (INREG16(x) & ~(y)))
#define SETREG16(x, y)                OUTREG16((x), (INREG16(x) | (y)))
#define CLRSETREG16(addr, clear, set) OUTREG16(addr, (INREG16(addr) & ~(clear)) | (set))

__maybe_unused static void timer_ltssm_value_monitor(struct timer_list *t)
{
    u32                    ltssm_state;
    unsigned long          flags;
    struct dwc3_of_simple *simple = from_timer(simple, t, ltssm_timer);
    struct dwc3 *          dwc    = simple->dwc;

    spin_lock_irqsave(&dwc->lock, flags);
    ltssm_state = dwc3_readl(dwc->regs, DWC3_GDBGLTSSM);
    if (0 == global_debug_ltssm[ltssm_index] && 0 == ltssm_index)
    {
        global_debug_ltssm[ltssm_index] = ltssm_state;
    }
    else if (ltssm_state != global_debug_ltssm[ltssm_index])
    {
        ltssm_index++;
        ltssm_index                     = ltssm_index % ARRAY_SIZE(global_debug_ltssm);
        global_debug_ltssm[ltssm_index] = ltssm_state;
    }
    spin_unlock_irqrestore(&dwc->lock, flags);

    mod_timer(t, jiffies + (msecs_to_jiffies(1) / 2));
    return;
}

static void dwc3_of_simple_stop_timer(void *data)
{
#if IS_ENABLED(CONFIG_ENABLE_LTSSM_MONITOR)
    struct dwc3_of_simple *simple = data;

    del_timer_sync(&simple->ltssm_timer);
#endif
}

static void dwc3_of_simple_setup_timer(struct dwc3_of_simple *simple)
{
#if IS_ENABLED(CONFIG_ENABLE_LTSSM_MONITOR)
    //#pragma message("CONFIG_ENABLE_LTSSM_MONITOR")
    timer_setup(&simple->ltssm_timer, timer_ltssm_value_monitor, 0);
    mod_timer(&simple->ltssm_timer, jiffies + (msecs_to_jiffies(1) / 2));
#endif
}

static int dwc3_of_simple_get_phy(struct dwc3_of_simple *simple)
{
    int                 ret = 0;
    struct device *     dev = simple->dev;
    struct sstar_u3phy *sstar_phy;

    simple->usb_phy = devm_usb_get_phy_by_phandle(dev, "usb-phy", 0);
    if (IS_ERR(simple->usb_phy))
    {
        ret = PTR_ERR(simple->usb_phy);
        dev_err(dev, "failed to get usb phy: %d\n", ret);
        if (ret == -EPROBE_DEFER)
            simple->usb_phy = NULL;
        else
            goto err;
    }

    simple->usb_phy->otg = devm_kzalloc(simple->dev, sizeof(*simple->usb_phy->otg), GFP_KERNEL);
    if (!simple->usb_phy->otg)
    {
        ret = -ENOMEM;
        goto err;
    }

    simple->usb_phy->otg->usb_phy = simple->usb_phy;
    if (simple->usb_phy && simple->usb_phy->edev)
    {
        if (simple->dwc && (simple->dr_mode == USB_DR_MODE_PERIPHERAL))
            simple->usb_phy->otg->gadget = simple->dwc->gadget;

        sstar_phy = container_of(simple->usb_phy, struct sstar_u3phy, usb_phy);
        if (simple->dr_mode == USB_DR_MODE_PERIPHERAL)
            ret = devm_extcon_register_notifier(dev, simple->usb_phy->edev, EXTCON_USB, &sstar_phy->usb3_typec_nb);
        else if (simple->dr_mode == USB_DR_MODE_HOST)
            ret = devm_extcon_register_notifier(dev, simple->usb_phy->edev, EXTCON_USB_HOST, &sstar_phy->usb3_typec_nb);

        if (ret < 0)
        {
            dev_err(dev, "register extcon notifier failed\n");
            goto err;
        }
    }

    return 0;
err:
    return ret;
}

static int dwc3_of_simple_init_mode(struct platform_device *pdev, struct dwc3_of_simple *simple)
{
    struct device *     dev = &pdev->dev;
    struct device_node *np  = dev->of_node, *child;
    struct regmap *     drd_ctrl;
    int                 x2a_off[4] = {0};
    int                 ret        = 0;
    const char *        dr;

    child = of_get_child_by_name(np, "dwc3");

    if (!child)
    {
        dev_err(dev, "failed to find dwc3 core node\n");
        return -ENODEV;
    }

    dr              = of_get_property(child, "dr_mode", NULL);
    simple->dr_mode = USB_DR_MODE_UNKNOWN;

    if (dr)
    {
        if (!strcmp("host", dr))
        {
            simple->dr_mode = USB_DR_MODE_HOST;
        }
        else if (!strcmp("peripheral", dr))
        {
            simple->dr_mode = USB_DR_MODE_PERIPHERAL;
        }
    }
    else
    {
        dr = "unknown";
    }

    dev_info(dev, "dr mode: %s\n", dr);

    switch (simple->dr_mode)
    {
        case USB_DR_MODE_PERIPHERAL:
        case USB_DR_MODE_HOST:
            break;
        default:
#if IS_ENABLED(CONFIG_USB_DWC3_HOST)
            simple->dr_mode = USB_DR_MODE_HOST;
            dev_dbg(dev, "force dwc3 to host mode.\r\n");
#else
            simple->dr_mode = USB_DR_MODE_PERIPHERAL;
            dev_dbg(dev, "force dwc3 to gadget mode.\r\n");
#endif
    }

    if (USB_DR_MODE_HOST == simple->dr_mode)
    {
        if (!device_property_read_u32_array(dev, "sstar,x2a-addr-off", x2a_off, ARRAY_SIZE(x2a_off)))
        {
            int i, reg;

            drd_ctrl = syscon_regmap_lookup_by_phandle(dev->of_node, "sstar,usb3drd-mac-syscon");

            if (IS_ERR(drd_ctrl))
            {
                dev_err(dev, "remap fail %ld\n", PTR_ERR(drd_ctrl));
                ret = PTR_ERR(drd_ctrl);
                goto err;
            }
            reg = 0x50;
            for (i = 0; i < (sizeof x2a_off / sizeof(int)); i++)
            {
                dev_info(dev, "x2a-addr-off[0]: 0x%08x\n", x2a_off[i]);
                if ((ret = regmap_write(drd_ctrl, (reg << 2), x2a_off[i] & 0xffff))
                    || (ret = regmap_write(drd_ctrl, ((reg + 1) << 2), (x2a_off[i] >> 16) & 0x00ff)))
                {
                    dev_err(dev, "address mapping fail\n");
                    goto err;
                }
                reg += 2;
            }
            if (ret)
            {
                dev_err(dev, "reg = %d: address mapping fail(%d)\n", reg, ret);
                goto err;
            }
        }
    }

    if (device_property_read_bool(dev, "sstar,force-mcm-enabled"))
    {
        drd_ctrl = syscon_regmap_lookup_by_phandle(dev->of_node, "sstar,usb3drd-mcm-syscon");
        if (IS_ERR(drd_ctrl))
        {
            ret = PTR_ERR(drd_ctrl);
            goto err;
        }
        regmap_update_bits(drd_ctrl, (0x0 << 2), 0xff, 0x40);
    }

    drd_ctrl = syscon_regmap_lookup_by_phandle(dev->of_node, "sstar,usb3drd-gp-syscon");
    if (IS_ERR(drd_ctrl))
    {
        dev_err(dev, "regmap lookup fail %ld\n", PTR_ERR(drd_ctrl));
        ret = PTR_ERR(drd_ctrl);
        goto err;
    }
    regmap_clear_bits(drd_ctrl, (0x21 << 2), BIT(1));

    return 0;

err:
    return ret;
}

static int dwc3_of_simple_probe(struct platform_device *pdev)
{
    struct dwc3_of_simple * simple;
    struct device *         dev = &pdev->dev;
    struct device_node *    np  = dev->of_node, *child;
    struct platform_device *child_pdev;
    struct dwc3 *           dwc;
    int                     ret;

    simple = devm_kzalloc(dev, sizeof(*simple), GFP_KERNEL);
    if (!simple)
    {
        dev_err(dev, "devm_kzalloc fail\n");
        return -ENOMEM;
    }

    platform_set_drvdata(pdev, simple);
    simple->dev = dev;

    if (of_device_is_compatible(dev->of_node, "sstar,infinity6f-dwc3"))
    {
        simple->reset = of_reset_control_array_get_exclusive(dev->of_node);
    }
    else
    {
        simple->reset = devm_reset_control_array_get_optional_shared(dev);
    }

    if (IS_ERR(simple->reset))
        return PTR_ERR(simple->reset);
    ret = reset_control_deassert(simple->reset);
    if (ret)
        return ret;

    ret = dma_set_mask_and_coherent(dev, DMA_BIT_MASK(64));
    if (ret)
    {
        return ret;
    }

#ifdef CONFIG_TEST_ALL_LINUX_MMU_MAP_ADDRESS
    if (of_device_is_compatible(np, "sstar,infinity7-dwc3"))
    {
        dma_direct_set_offset(dev, CONFIG_MIU0_BUS_BASE, CONFIG_TEST_ALL_LINUX_MMU_MAP_ADDRESS, lx_mem_size);
    }
    else
    {
        dma_direct_set_offset(dev, CONFIG_MIU0_BUS_BASE, CONFIG_TEST_ALL_LINUX_MMU_MAP_ADDRESS + (128 << 10),
                              lx_mem_size);
    }
#else
    dma_direct_set_offset(dev, CONFIG_MIU0_BUS_BASE, 0, lx_mem_size);
#endif

    ret = dwc3_of_simple_init_mode(pdev, simple);

    if (ret)
    {
        dev_err(dev, "dwc3_of_simple_init_mode fail\n");
        return ret;
    }

    ret = clk_bulk_get_all(simple->dev, &simple->clks);
    if (ret < 0)
        goto err_clk_put;

    simple->num_clocks = ret;
    ret                = clk_bulk_prepare_enable(simple->num_clocks, simple->clks);
    if (ret)
        goto err_clk_put;

    ret = of_platform_populate(np, NULL, NULL, dev);
    if (ret)
        goto err_clk_put;

    child = of_get_child_by_name(np, "dwc3");
    if (!child)
    {
        dev_err(dev, "failed to find dwc3 core node\n");
        ret = -ENODEV;
        goto err_clk_put;
    }

    child_pdev = of_find_device_by_node(child);
    if (!child_pdev)
    {
        dev_err(dev, "failed to find dwc3 core device\n");
        ret = -ENODEV;
        goto err_clk_put;
    }

    dwc = platform_get_drvdata(child_pdev);
    if (!dwc)
    {
        dev_dbg(dev, "failed to get drvdata dwc3\n");
        ret = -EPROBE_DEFER;
        goto err_clk_put;
    }

    simple->dwc = dwc;
    dwc3_of_simple_get_phy(simple);
    pm_runtime_set_active(dev);
    pm_runtime_enable(dev);
    pm_runtime_get_sync(dev);

    dwc3_of_simple_setup_timer(simple);
    devm_add_action_or_reset(dwc->dev, dwc3_of_simple_stop_timer, simple);
    sstar_dwc3_debugfs_init(simple);

    return 0;

err_clk_put:
    clk_bulk_disable_unprepare(simple->num_clocks, simple->clks);
    clk_bulk_put_all(simple->num_clocks, simple->clks);
    dev_err(dev, "%s fail\n", __func__);
    return ret;
}

static void __dwc3_of_simple_teardown(struct dwc3_of_simple *simple)
{
    of_platform_depopulate(simple->dev);

    clk_bulk_disable_unprepare(simple->num_clocks, simple->clks);
    clk_bulk_put_all(simple->num_clocks, simple->clks);
    simple->num_clocks = 0;

    pm_runtime_disable(simple->dev);
    pm_runtime_put_noidle(simple->dev);
    pm_runtime_set_suspended(simple->dev);
#if IS_ENABLED(CONFIG_USB_DWC3_HOST)
    sstar_dwc3_debugfs_exit(simple);
#endif
}

static int dwc3_of_simple_remove(struct platform_device *pdev)
{
    struct dwc3_of_simple *simple = platform_get_drvdata(pdev);
    __dwc3_of_simple_teardown(simple);

    return 0;
}

static void dwc3_of_simple_shutdown(struct platform_device *pdev)
{
    struct dwc3_of_simple *simple = platform_get_drvdata(pdev);

    __dwc3_of_simple_teardown(simple);
}

static int __maybe_unused dwc3_of_simple_runtime_suspend(struct device *dev)
{
    struct dwc3_of_simple *simple = dev_get_drvdata(dev);

    clk_bulk_disable(simple->num_clocks, simple->clks);

    return 0;
}

static int __maybe_unused dwc3_of_simple_runtime_resume(struct device *dev)
{
    struct dwc3_of_simple *simple = dev_get_drvdata(dev);

    return clk_bulk_enable(simple->num_clocks, simple->clks);
}

static int __maybe_unused dwc3_of_simple_suspend(struct device *dev)
{
    struct dwc3_of_simple *simple = dev_get_drvdata(dev);

    if (simple->dr_mode == USB_DR_MODE_HOST)
    {
        dwc3_of_simple_stop_timer(simple);
        clk_bulk_disable_unprepare(simple->num_clocks, simple->clks);
        dev_info(dev, "%s Global clear registers\n", __func__);
        reset_control_assert(simple->reset);
    }
    else
    {
        if (of_device_is_compatible(dev->of_node, "sstar,infinity6f-dwc3"))
        {
            struct dwc3_of_simple *simple = dev_get_drvdata(dev);
            struct regmap *        block_mask;
            block_mask = syscon_regmap_lookup_by_phandle(dev->of_node, "sstar,block-mask");
            if (IS_ERR(block_mask))
            {
                dev_err(dev, "%s remap fail %ld\n", __func__, PTR_ERR(block_mask));
                return PTR_ERR(block_mask);
            }
            regmap_update_bits(block_mask, (0x39 << 2), 0x0001, 0x0001);
            udelay(1);
            reset_control_assert(simple->reset);
            {
                u32 base = 0xfd000000 + (0x1650 * 0x200);

                OUTREG16(base + (0x5a << 2), 0x4171);
                dev_warn(dev, "%s wr_cmd = 0x%x, wr_data = 0x%x, wr_b_respond = 0x%x\n", __func__,
                         INREG16(base + (0x5b << 2)) & 0x00ff, INREG16(base + (0x5b << 2)) >> 8,
                         INREG16(base + (0x5c << 2)) & 0x00ff);
                OUTREG16(base + (0x5a << 2), 0x4181);
                dev_warn(dev, "%s rd_cmd = 0x%x, rd_data = 0x%x\n", __func__, INREG16(base + (0x5b << 2)) & 0x00ff,
                         INREG16(base + (0x5b << 2)) >> 8);
            }
        }
    }
    return 0;
}

static int __maybe_unused dwc3_of_simple_resume(struct device *dev)
{
    int                     ret;
    struct platform_device *pdev   = to_platform_device(dev);
    struct dwc3_of_simple * simple = dev_get_drvdata(dev);

    if (simple->dr_mode == USB_DR_MODE_HOST)
    {
        dev_info(dev, "%s Release global reset\n", __func__);
        ret = reset_control_deassert(simple->reset);
        if (ret)
            return ret;
        ret = dwc3_of_simple_init_mode(pdev, simple);
        if (ret)
            return ret;

        ret = clk_bulk_prepare_enable(simple->num_clocks, simple->clks);
        if (ret)
            return ret;

        dwc3_of_simple_setup_timer(simple);
        return ret;
    }
    else
    {
        if (of_device_is_compatible(dev->of_node, "sstar,infinity6f-dwc3"))
        {
            struct dwc3_of_simple *simple = dev_get_drvdata(dev);
            struct regmap *        block_mask;
            u32                    base;
            base = 0xfd000000 + (0x1650 * 0x200);
            OUTREG16(base + (0x5a << 2), 0x4001);
            OUTREG16(base + (0x13 << 2), 0x0017);
            OUTREG16(base + (0x43 << 2), 0x0017);

            block_mask = syscon_regmap_lookup_by_phandle(dev->of_node, "sstar,block-mask");
            if (IS_ERR(block_mask))
            {
                dev_err(dev, "%s remap fail %ld\n", __func__, PTR_ERR(block_mask));
                return PTR_ERR(block_mask);
            }
            reset_control_deassert(simple->reset);
            udelay(1);
            return regmap_clear_bits(block_mask, (0x39 << 2), 0x0001);
        }
        return 0;
    }
}

static const struct dev_pm_ops dwc3_of_simple_dev_pm_ops = {
    SET_SYSTEM_SLEEP_PM_OPS(dwc3_of_simple_suspend, dwc3_of_simple_resume)
        SET_RUNTIME_PM_OPS(dwc3_of_simple_runtime_suspend, dwc3_of_simple_runtime_resume, NULL)};

static const struct of_device_id of_dwc3_simple_match[] = {
    {.compatible = "sstar,generic-dwc3"},
    {.compatible = "sstar,infinity7-dwc3"},
    {.compatible = "sstar,infinity6f-dwc3"},
    {/* Sentinel */},
};
MODULE_DEVICE_TABLE(of, of_dwc3_simple_match);

static struct platform_driver dwc3_of_simple_driver = {
    .probe    = dwc3_of_simple_probe,
    .remove   = dwc3_of_simple_remove,
    .shutdown = dwc3_of_simple_shutdown,
    .driver =
        {
            .name           = "sstar-dwc3-of-simple",
            .of_match_table = of_dwc3_simple_match,
            .pm             = &dwc3_of_simple_dev_pm_ops,
        },
};

module_platform_driver(dwc3_of_simple_driver);
MODULE_SOFTDEP("pre: dwc3 usb3-phy");
MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("Sstar DesignWare USB3 OF Simple Glue Layer");
MODULE_AUTHOR("Raul Wang <raul.wang@isgmastar.com.tw>");
