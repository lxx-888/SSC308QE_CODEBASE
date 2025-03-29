/*
 * sstar_pcie_ep.c - Sigmastar
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
#include <common.h>
#include <dm.h>
#include <pci_ep.h>
#include "drv_pcieif.h"
#include "drv_pcie.h"

#define to_ep_from_dev(dev) ((PCIE_EP *)dev_get_priv(dev))

static int sstar_pcie_ep_write_header(struct udevice *dev, uint func, struct pci_ep_header *hdr)
{
    PCIE_EP *ep = to_ep_from_dev(dev);

    Drv_PCIE_EpcWriteHeader(&ep->epc, hdr);
    return 0;
}

static int sstar_pcie_ep_set_bar(struct udevice *dev, uint func, struct pci_bar *bar)
{
    PCIE_EP *ep = to_ep_from_dev(dev);

    return Drv_PCIE_EpcSetBar(&ep->epc, bar);
}

static int sstar_pcie_ep_map_addr(struct udevice *dev, uint func, phys_addr_t addr, u64 pci_addr, size_t size)
{
    PCIE_EP *ep = to_ep_from_dev(dev);

    return Drv_PCIE_EpcMapAddr(&ep->epc, addr, pci_addr, size);
}

static int sstar_pcie_ep_unmap_addr(struct udevice *dev, uint func, phys_addr_t addr)
{
    PCIE_EP *ep = to_ep_from_dev(dev);

    Drv_PCIE_EpcUnmapAddr(&ep->epc, addr);
    return 0;
}

static int sstar_pcie_ep_raise_irq(struct udevice *dev, uint func, enum pci_ep_irq_type type, uint intr_num)
{
    PCIE_EP *ep = to_ep_from_dev(dev);

    if (type == PCI_EP_IRQ_MSI)
        return Drv_PCIE_EpcRaiseMsiIrq(&ep->epc, intr_num);
    else if (type == PCI_EP_IRQ_MSIX)
        return Drv_PCIE_EpcRaiseMsiXIrq(&ep->epc, intr_num);
    else
        return -EINVAL;
}

static int sstar_pcie_ep_start(struct udevice *dev)
{
    PCIE_EP *ep = to_ep_from_dev(dev);

    Drv_PCIE_EpcStart(&ep->epc);
    return 0;
}

static struct pci_ep_ops sstar_pcie_ep_ops = {
    .write_header = sstar_pcie_ep_write_header,
    .set_bar      = sstar_pcie_ep_set_bar,
    .map_addr     = sstar_pcie_ep_map_addr,
    .unmap_addr   = sstar_pcie_ep_unmap_addr,
    .raise_irq    = sstar_pcie_ep_raise_irq,
    .start        = sstar_pcie_ep_start,
};

static int sstar_pcie_ep_probe(struct udevice *dev)
{
    PCIE_EP *      ep  = to_ep_from_dev(dev);
    int            ret = 0;
    enum pci_barno bar;

#ifdef CONFIG_SSTAR_CLK
    ret = clk_get_bulk(dev, &ep->clks);
    if (ret)
    {
        PCIE_ERR("epc get clks failed\r\n");
        return ret;
    }
    if (ep->clk_cnt && (ep->clk_src != PCIE_CLK_SRC_INT))
    {
        int i;
        for (i = 0; i < ep->clk_cnt; i++)
            clk_enable(&ep->clks.clks[i]);
    }
    else
    {
        clk_enable_bulk(&ep->clks);
    }
#else
    sstar_pcieif_clk_enable(ep->port_id);
#endif

    if (ep->clk_src == PCIE_CLK_SRC_INT)
        sstar_pcieif_internalclk_en(ep->port_id);

    ret = Drv_PCIE_EpcInit(ep);
    if (ret)
    {
        PCIE_ERR("epc init failed\r\n");
        return ret;
    }

    for (bar = BAR_0; bar <= BAR_5; bar++)
    {
        Drv_PCIE_EpcResetBar(ep, bar);
    }

    ret = Drv_PCIE_DmaInit(&ep->edma);
    if (ret)
    {
        PCIE_ERR("edma init failed");
        return ret;
    }

    return 0;
}

static int sstar_pcie_ep_of_to_plat(struct udevice *dev)
{
    PCIE_EP *ep       = to_ep_from_dev(dev);
    u32      property = 0;

    if (dev_read_u32(dev, "portid", &property) == 0)
        ep->port_id = property;

    if (dev_read_u32(dev, "num_clks_mandatory", &property) == 0)
        ep->clk_cnt = property;

    /* power on internal clock source if needed */
    if (dev_read_u32(dev, "use_internal_clk", &property) == 0)
    {
        if (property)
        {
            ep->clk_src = PCIE_CLK_SRC_INT;
        }
    }

    /* number of lanes */
    if (dev_read_u32(dev, "num-lanes", &property) == 0)
        ep->num_lanes = property;
    else
        ep->num_lanes = 1;

    /* link speed */
    if (dev_read_u32(dev, "max-link-speed", &property) == 0)
        ep->link_gen = property;
    else
        ep->link_gen = 1;

    PCIE_DBG("pcie ep %d: num-lanes %d, link gen %d\n", ep->port_id, ep->num_lanes, ep->link_gen);

    return 0;
}

static int sstar_pcie_ep_remove(struct udevice *dev)
{
    return 0;
}

const struct udevice_id sstar_pcie_ep_of_match[] = {{.compatible = "snps,dw-pcie-ep"}, {}};

U_BOOT_DRIVER(sstar_pcie_ep) = {
    .name       = "sstar_pcie_ep",
    .id         = UCLASS_PCI_EP,
    .of_match   = sstar_pcie_ep_of_match,
    .of_to_plat = sstar_pcie_ep_of_to_plat,
    .ops        = &sstar_pcie_ep_ops,
    .probe      = sstar_pcie_ep_probe,
    .remove     = sstar_pcie_ep_remove,
    .priv_auto  = sizeof(PCIE_EP),
};
