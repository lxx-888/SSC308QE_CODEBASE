/*
 * ehci-sstar.c- Sigmastar
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
#include <linux/of_irq.h>
#include <linux/platform_device.h>
#include <linux/dma-mapping.h>
#include <linux/gpio.h>
#include <linux/reset.h>
#include <linux/clk.h>
#include "../../../usb/host/ehci.h"
#include "ehci-sstar.h"
#include "sstar_usb2_phy.h"
#include "../../../usb/core/phy.h"

#define hcd_to_sstar_ehci_priv(h) ((struct sstar_ehci_priv *)hcd_to_ehci(h)->priv)

struct sstar_ehci_priv
{
    struct clk_bulk_data *clks;
    int                   num_clks;
    unsigned              gpio_vbus_power;
    struct phy *          phy;
};

extern phys_addr_t lx_mem_size;

static struct hc_driver __read_mostly ehci_sstar_hc_driver;

static const struct ehci_driver_overrides ehci_sstar_overrides __initconst = {
    .extra_priv_size = sizeof(struct sstar_ehci_priv),
};

void ehci_hcd_sstar_miu_select(struct regmap *usbc)
{
    printk("[USB] config miu select [%02x] [%02x] [%02x] [%02x]\n", USB_MIU_SEL0, USB_MIU_SEL1, USB_MIU_SEL2,
           USB_MIU_SEL3);

    /* [3:0]  : Lower bound of miu sel0 */
    /* [7:4]  : Upper bound of miu sel0 */
    regmap_write(usbc, (0x0A << 2), USB_MIU_SEL0);

    /* [3:0]  : Lower bound of miu sel1 */
    /* [7:4]  : Upper bound of miu sel1 */
    /* [11:8] : Lower bound of miu sel2 */
    /* [15:12]: Upper bound of miu sel2 */
    regmap_write(usbc, (0x0B << 2), (USB_MIU_SEL2 << 8) | USB_MIU_SEL1);

    /* [3:0] : Lower bound of miu sel3 */
    /* [7:4] : Upper bound of miu sel3 */
    /* [8]   : Enable miu partition mechanism */
    regmap_update_bits(usbc, (0x0C << 2), 0xFF, USB_MIU_SEL3);
    regmap_set_bits(usbc, (0x0C << 2), BIT(8));
    regmap_clear_bits(usbc, (0x07 << 2), BIT(8));
}

void ehci_hcd_sstar_dma_over4GB_enable(struct regmap *usbc, dma_addr_t dma)
{
    u16 dma_extend_addr;

    if (!usbc)
        return;

    if ((dma >> 31) & 0x1)
        regmap_set_bits(usbc, (0x00 << 2), BIT(4));
    else
        regmap_clear_bits(usbc, (0x00 << 2), BIT(4));

    dma_extend_addr = (dma >> 20) & 0xF000;
    /* Support over 4GB memory, write MSB 4bits to extend reg */
    regmap_update_bits(usbc, (0x00 << 2), 0xF000, dma_extend_addr);
}

void ehci_hcd_sstar_usbc_settings(struct regmap *usbc)
{
    if (!usbc)
        return;

    /* Enable use eof2 to reset state machine (power noise) */
    regmap_set_bits(usbc, (0x01 << 2), BIT(6));

    /* HS connection fail problem (Gate into VFALL state) */
    regmap_set_bits(usbc, (0x08 << 2), BIT(9));

    /* ENABLE_PV2MI_BRIDGE_ECO */
    regmap_set_bits(usbc, (0x05 << 2), BIT(6));

    /* _USB_MINI_PV2MI_BURST_SIZE */
    regmap_clear_bits(usbc, (0x05 << 2), BIT(12) | BIT(11) | BIT(10) | BIT(9));

    /* [11]: reg_preamble_en, to enable Faraday Preamble */
    regmap_set_bits(usbc, (0x07 << 2), BIT(11));

    /* [0]: reg_preamble_babble_fix, to patch Babble occurs in Preamble */
    /* [1]: reg_preamble_fs_within_pre_en, to patch FS crash problem */
    regmap_set_bits(usbc, (0x08 << 2), BIT(1) | BIT(0));

    /* Enable HS ISO IN Camera Cornor case ECO function */
    regmap_set_bits(usbc, (0x09 << 2), BIT(8));

    /* [0]: UHC speed type report should be reset by device disconnection */
    /* [1]: Port Change Detect (PCD) is triggered by babble. Pulse trigger will not hang this condition. */
    /* [2]: ENABLE_HC_RESET_FAIL_ECO, generation of hhc_reset_u */
    regmap_set_bits(usbc, (0x10 << 2), BIT(2) | BIT(1) | BIT(0));

    /* Use 2 SOFs to check disconnection */
    regmap_update_bits(usbc, (0x01 << 2), 0xFF00, BIT(10) | BIT(8));

    return;
}
/**
 * ehci_hcd_sstar_drv_probe
 * Context: !in_interrupt()
 *
 * Allocates basic resources for this USB host controller, and
 * then invokes the start() method for the HCD associated with it
 * through the hotplug entry's driver_data.
 *
 */
int ehci_hcd_sstar_drv_probe(struct platform_device *pdev)
{
    struct resource *       res;
    int                     ret = 0;
    int                     irq;
    struct usb_hcd *        hcd;
    struct ehci_hcd *       ehci;
    u64                     dma_mask;
    struct device_node *    node = pdev->dev.of_node;
    struct sstar_u2phy *    priv = NULL;
    struct sstar_ehci_priv *ehci_priv;
    struct device *         dev = &pdev->dev;
    struct reset_control *  reset;

    if (usb_disabled())
        return -ENODEV;

    dev_info(dev, "Initializing SSTAR-SoC USB Host Controller");

    irq = platform_get_irq(pdev, 0);
    if (irq <= 0)
        return irq ? irq : -EINVAL;

    dev_info(dev, "[USB] %s irq --> %d\n", pdev->name, irq);

    if (IS_ENABLED(CONFIG_ARM64) && IS_ENABLED(CONFIG_ZONE_DMA))
    {
#if defined(EHC_DMA_BIT_MASK)
        dma_mask = EHC_DMA_BIT_MASK;
#else
        /* default: 32bit to mask lowest 4G address */
        dma_mask = DMA_BIT_MASK(32);
#endif
    }
    else
    {
        dma_mask = DMA_BIT_MASK(64);
    }
    ret = dma_set_mask_and_coherent(dev, dma_mask);
    if (ret)
        return ret;

    dev_info(dev, "dma mask 0x%llx\n", dma_mask);
    ret = dma_direct_set_offset(dev, MIU0_BASE, 0, lx_mem_size);
    if (ret)
        return ret;

    hcd = usb_create_hcd(&ehci_sstar_hc_driver, dev, "sstar");
    if (!hcd)
        return -ENOMEM;

    hcd->regs = devm_platform_get_and_ioremap_resource(pdev, 0, &res);
    if (IS_ERR(hcd->regs))
    {
        ret = PTR_ERR(hcd->regs);
        goto fail1;
    }
    spin_lock_init(&hcd->usb_reset_lock);
    hcd->rsrc_start              = res->start;
    hcd->rsrc_len                = resource_size(res);
    hcd->ehc_base                = (struct regmap *)hcd->regs;
    hcd->has_tt                  = 1;
    hcd->skip_phy_initialization = 1;
    ehci                         = hcd_to_ehci(hcd);
    ehci->caps                   = hcd->regs;

    ehci_priv = hcd_to_sstar_ehci_priv(hcd);

    reset = devm_reset_control_array_get_optional_shared(dev);
    if (IS_ERR(reset))
    {
        ret = PTR_ERR(reset);
        goto fail1;
    }

    ret = reset_control_deassert(reset);
    if (ret)
    {
        ret = PTR_ERR(reset);
        goto fail1;
    }
    // clock enable
    ret = devm_clk_bulk_get_all(dev, &ehci_priv->clks);
    if (ret == -EPROBE_DEFER)
    {
        goto fail2;
    }
    if (ret < 0)
    {
        ehci_priv->num_clks = 0;
    }
    else
    {
        ehci_priv->num_clks = ret;
    }
    ret = clk_bulk_prepare_enable(ehci_priv->num_clks, ehci_priv->clks);
    if (ret)
        goto fail2;

    // phy
    ehci_priv->phy = devm_of_phy_get_by_index(dev, node, 0);
    if (ehci_priv->phy)
    {
        priv = phy_get_drvdata(ehci_priv->phy);
        if (priv)
        {
            priv->reset_lock = &hcd->usb_reset_lock;
            hcd->usb_phy     = &priv->usb_phy;
        }
        hcd->utmi_base          = priv->utmi;
        hcd->bc_base            = priv->bc;
        hcd->usbc_base          = priv->usbc;
        hcd->usb_monitor_enable = priv->usb_monitor_enable;
    }

    ehci_hcd_sstar_miu_select(priv->usbc);
    ehci_hcd_sstar_usbc_settings(priv->usbc);
    ehci_hcd_sstar_dma_over4GB_enable(priv->usbc, ehci->periodic_dma);

    phy_init(ehci_priv->phy);
    phy_set_mode(ehci_priv->phy, PHY_MODE_USB_HOST);
    phy_power_on(ehci_priv->phy);
    phy_calibrate(ehci_priv->phy);

    ret = usb_add_hcd(hcd, irq, 0);
    if (ret != 0)
        goto fail3;

    // vbus ctrl
    if (device_property_read_u32(hcd->self.controller, "gpio_vbus_power", &ehci_priv->gpio_vbus_power) == 0)
    {
        ret = gpio_request(ehci_priv->gpio_vbus_power, "VBUS Ctrl");
        if (ret < 0)
        {
            dev_err(dev, "Failed to request USB0-power-enable GPIO(%d)\n", ehci_priv->gpio_vbus_power);
        }
        else
        {
            gpio_direction_output(ehci_priv->gpio_vbus_power, 1);
            dev_info(hcd->self.controller, "enable vbus-power-gpio:%d\r\n", ehci_priv->gpio_vbus_power);
        }
    }

    return ret;

fail3:
    clk_bulk_disable_unprepare(ehci_priv->num_clks, ehci_priv->clks);
fail2:
    reset_control_assert(reset);
fail1:
    usb_put_hcd(hcd);

    return ret;
}

/**
 * ehci_hcd_sstar_drv_remove
 * @dev: USB Host Controller being removed
 * Context: !in_interrupt()
 *
 * Reverses the effect of ehci_hcd_sstar_drv_remove(), first invoking
 * the HCD's stop() method.  It is always called from a thread
 * context, normally "rmmod", "apmd", or something similar.
 */
static int ehci_hcd_sstar_drv_remove(struct platform_device *pdev)
{
    struct usb_hcd *        hcd = platform_get_drvdata(pdev);
    struct sstar_ehci_priv *ehci_priv;

    ehci_priv = hcd_to_sstar_ehci_priv(hcd);
    usb_remove_hcd(hcd);
    phy_power_off(ehci_priv->phy);
    phy_exit(ehci_priv->phy);
    clk_bulk_disable_unprepare(ehci_priv->num_clks, ehci_priv->clks);
    usb_put_hcd(hcd);
    gpio_free(ehci_priv->gpio_vbus_power);

    return 0;
}

static struct of_device_id sstar_ehci_of_device_ids[] = {
    {.compatible = "sstar-ehci-0"}, {.compatible = "sstar-ehci-1"}, {.compatible = "sstar-ehci-2"},
    {.compatible = "sstar-ehci-3"}, {.compatible = "sstar-ehci-4"}, {},
};

#ifdef CONFIG_PM
static int sstar_ehci_suspend(struct device *dev)
{
    struct usb_hcd *        hcd       = dev_get_drvdata(dev);
    bool                    do_wakeup = device_may_wakeup(dev);
    struct sstar_ehci_priv *ehci_priv;

    ehci_priv = hcd_to_sstar_ehci_priv(hcd);

    // vbus control
    if (ehci_priv->gpio_vbus_power)
        gpio_direction_output(ehci_priv->gpio_vbus_power, 0);

    disable_irq(hcd->irq);
    phy_power_off(ehci_priv->phy);
    phy_exit(ehci_priv->phy);
    ehci_suspend(hcd, do_wakeup);
    clk_bulk_disable_unprepare(ehci_priv->num_clks, ehci_priv->clks);

    return 0;
}

static int sstar_ehci_resume(struct device *dev)
{
    struct usb_hcd *        hcd = dev_get_drvdata(dev);
    struct sstar_ehci_priv *ehci_priv;
    int                     ret = 0;

    ehci_priv = hcd_to_sstar_ehci_priv(hcd);
    ret       = clk_bulk_prepare_enable(ehci_priv->num_clks, ehci_priv->clks);
    if (ret)
        return ret;

    phy_init(ehci_priv->phy);
    phy_set_mode(ehci_priv->phy, PHY_MODE_USB_HOST);
    phy_power_on(ehci_priv->phy);
    phy_calibrate(ehci_priv->phy);
    ehci_resume(hcd, false);
    enable_irq(hcd->irq);
    // vbus control
    if (ehci_priv->gpio_vbus_power)
        gpio_direction_output(ehci_priv->gpio_vbus_power, 1);

    return 0;
}
#else
#define sstar_ehci_suspend NULL
#define sstar_ehci_resume  NULL
#endif

static const struct dev_pm_ops sstar_ehci_pm_ops = {
    .suspend = sstar_ehci_suspend,
    .resume  = sstar_ehci_resume,
};

static struct platform_driver ehci_hcd_sstar_driver = {
    .probe  = ehci_hcd_sstar_drv_probe,
    .remove = ehci_hcd_sstar_drv_remove,
    .driver =
        {
            .name           = "sstar-ehci",
            .pm             = &sstar_ehci_pm_ops,
            .of_match_table = sstar_ehci_of_device_ids,
        },
};
