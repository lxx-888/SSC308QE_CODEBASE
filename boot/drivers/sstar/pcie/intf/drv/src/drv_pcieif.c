/*
 * drv_pcieif.c- Sigmastar
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
#include <asm/io.h>
#include "asm/arch/mach/io.h"
#include "drv_pcieif.h"
#include "hal_pcieif.h"

struct pcie_intf
{
    void __iomem *dbi; /* dbi register address */
    void __iomem *phy; /* phy register address */
    void __iomem *mac; /* mac register address */
};

struct sstar_pcieif
{
    struct pcie_intf intf[PCIE_INTF_NUM]; /* for each controller */
    u32              num;                 /* number of controller */
};

static struct sstar_pcieif m_pcieif = {
    .intf[0] =
        {
            .dbi = REG_ADDR_BASE_PCIE_DBI(0),
            .phy = REG_ADDR_BASE_PCIE_PHY(0),
            .mac = REG_ADDR_BASE_PCIE_MAC(0),
        },
#if (PCIE_INTF_NUM > 1)
    .intf[1] =
        {
            .dbi = REG_ADDR_BASE_PCIE_DBI(1),
            .phy = REG_ADDR_BASE_PCIE_PHY(1),
            .mac = REG_ADDR_BASE_PCIE_MAC(1),
        },
#endif
#if (PCIE_INTF_NUM > 2)
    .intf[2] =
        {
            .dbi = REG_ADDR_BASE_PCIE_DBI(2),
            .phy = REG_ADDR_BASE_PCIE_PHY(2),
            .mac = REG_ADDR_BASE_PCIE_MAC(2),
        },
#endif

    .num = PCIE_INTF_NUM,
};

u16 sstar_pcieif_readw_phy(u8 id, u32 reg)
{
    struct pcie_intf *intf = &m_pcieif.intf[id];

    return readw(intf->phy + reg);
}

void sstar_pcieif_writew_phy(u8 id, u32 reg, u16 val)
{
    struct pcie_intf *intf = &m_pcieif.intf[id];

    writew(val, intf->phy + reg);
}

u16 sstar_pcieif_readw_mac(u8 id, u32 reg)
{
    struct pcie_intf *intf = &m_pcieif.intf[id];

    return readw(intf->mac + reg);
}

void sstar_pcieif_writew_mac(u8 id, u32 reg, u16 val)
{
    struct pcie_intf *intf = &m_pcieif.intf[id];

    writew(val, intf->mac + reg);
}

u32 sstar_pcieif_readl_dbi(u8 id, u32 reg)
{
    u32               val;
    u32               addr;
    struct pcie_intf *intf = &m_pcieif.intf[id];

    addr = hal_prog_dbi_addr(intf->mac, reg);
    val  = readl(intf->dbi + addr);

    hal_rst_dbi_addr(intf->mac);

    return val;
}

u16 sstar_pcieif_readw_dbi(u8 id, u32 reg)
{
    u16               val;
    u32               addr;
    struct pcie_intf *intf = &m_pcieif.intf[id];

    addr = hal_prog_dbi_addr(intf->mac, reg);
    val  = readw(intf->dbi + addr);
    hal_rst_dbi_addr(intf->mac);

    return val;
}

u8 sstar_pcieif_readb_dbi(u8 id, u32 reg)
{
    u8                val;
    u32               addr;
    struct pcie_intf *intf = &m_pcieif.intf[id];

    addr = hal_prog_dbi_addr(intf->mac, reg);
    val  = readb(intf->dbi + addr);
    hal_rst_dbi_addr(intf->mac);

    return val;
}

void sstar_pcieif_writel_dbi(u8 id, u32 reg, u32 val)
{
    u32               addr;
    struct pcie_intf *intf = &m_pcieif.intf[id];

    addr = hal_prog_dbi_addr(intf->mac, reg);
    writel(val, intf->dbi + addr);
    readl(intf->dbi + addr); // dummy read to make sure x32 write has token effect

    hal_rst_dbi_addr(intf->mac);
}

void sstar_pcieif_writew_dbi(u8 id, u32 reg, u16 val)
{
    u32               addr;
    struct pcie_intf *intf = &m_pcieif.intf[id];

    addr = hal_prog_dbi_addr(intf->mac, reg);
    writew(val, intf->dbi + addr);
    readw(intf->dbi + addr); // dummy read to make sure x32 write has token effect
    hal_rst_dbi_addr(intf->mac);
}

void sstar_pcieif_writeb_dbi(u8 id, u32 reg, u8 val)
{
    u32               addr;
    struct pcie_intf *intf = &m_pcieif.intf[id];

    addr = hal_prog_dbi_addr(intf->mac, reg);
    writeb(val, intf->dbi + addr);
    readb(intf->dbi + addr); // dummy read to make sure x32 write has token effect
    hal_rst_dbi_addr(intf->mac);
}

void sstar_pcieif_clk_enable(u8 id)
{
    hal_clk_enable(id);
}

void sstar_pcieif_start_link(u8 id)
{
    sstar_pcieif_writew_mac(id, REG_PCIE_MAC_CTRL, 0x0010);
}

void sstar_pcieif_set_mode(u8 id, enum ss_pcieif_mode mode)
{
    if (mode == PCIE_IF_MODE_RC)
    {
        sstar_pcieif_writew_mac(id, REG_PCIE_DEVICE_TYPE, PCIE_DEVICE_RC);
        sstar_pcieif_writew_mac(id, REG_PCIE_IRQ2_MASK, ~(IRQ2_INTR_MSI));
    }
    else if (mode == PCIE_IF_MODE_EP)
    {
        sstar_pcieif_writew_mac(id, REG_PCIE_DEVICE_TYPE, PCIE_DEVICE_EP);
        sstar_pcieif_writew_mac(id, REG_PCIE_IRQ2_MASK, 0xFFFF);
    }
    sstar_pcieif_writew_phy(id, REG_SLV_READ_BYPASS, 0x0100); // slv_read_bypass
    sstar_pcieif_writew_mac(id, REG_SLV_WSTRB_MASK, 0xFFFF);
    sstar_pcieif_writew_phy(id, REG_PHYVREG_BPS, 1); // Bypass to 1.8V
}

void sstar_pcieif_internalclk_en(u8 id)
{
    hal_internalclk_en(id);
}
