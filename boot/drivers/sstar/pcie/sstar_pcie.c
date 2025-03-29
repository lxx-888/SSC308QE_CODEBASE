/*
 * sstar_pcie.c - Sigmastar
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
#include <pci.h>
#include <asm/io.h>
#include <dm/device_compat.h>
#include <linux/delay.h>
#include <linux/bitfield.h>
#ifdef CONFIG_SSTAR_CLK
#include <clk.h>
#endif
#include "pcie_dw_common.h"
#include "drv_pcieif.h"

/* PCIE_LINK_WIDTH_SPEED_CONTROL */
#define PORT_LOGIC_AUTO_LANE_FLIP    BIT(16)
#define PORT_LOGIC_FAST_TRAIN_SEQ(s) ((s)&0xFF)

#define PCIE_PORT_DEBUG0            (0x728)
#define PORT_LOGIC_LTSSM_STATE_MASK (0x1f)
#define PORT_LOGIC_LTSSM_STATE_L0   (0x11)

/* Parameters for the waiting for link up routine */
#define LINK_WAIT_MAX_RETRIES (10)
#define LINK_WAIT_MSLEEP      (10)

typedef struct sstar_pcie_plat
{
    struct pcie_dw dw;
    u8             id; /* controller id */
#ifdef CONFIG_SSTAR_CLK
    struct clk_bulk clks;
#endif
} sstar_pcie_plat_t;

static int is_link_up(sstar_pcie_plat_t *priv)
{
    u32 val;

    val = sstar_pcieif_readl_dbi(priv->id, PCIE_PORT_DEBUG0);
    val &= PORT_LOGIC_LTSSM_STATE_MASK;

    return (val == PORT_LOGIC_LTSSM_STATE_L0);
}

/**
 * sstar_pcie_configure() - Configure link capabilities and speed
 *
 * @ss_pcie: Pointer to the PCI controller state
 * @cap_speed: The capabilities and speed to configure
 *
 * Configure the link capabilities and speed in the PCIe root complex.
 */
static void sstar_pcie_configure(sstar_pcie_plat_t *priv, u32 cap_speed)
{
    struct pcie_dw *pcie = &(priv->dw);
    u32             val;

    dw_pcie_dbi_write_enable(pcie, true);

    val = sstar_pcieif_readl_dbi(priv->id, PCIE_LINK_CAPABILITY);
    val &= ~TARGET_LINK_SPEED_MASK;
    val |= cap_speed;
    sstar_pcieif_writel_dbi(priv->id, PCIE_LINK_CAPABILITY, val);

    val = sstar_pcieif_readl_dbi(priv->id, PCIE_LINK_CTL_2);
    val &= ~TARGET_LINK_SPEED_MASK;
    val |= cap_speed;
    sstar_pcieif_writel_dbi(priv->id, PCIE_LINK_CTL_2, val);

    dw_pcie_dbi_write_enable(pcie, false);
}

/**
 * sstar_pcie_link_up() - Wait for the link to come up
 *
 * @ss_pcie: Pointer to the PCI controller state
 * @cap_speed: Desired link speed
 *
 * Return: 1 (true) for active line and negetive (false) for no link (timeout)
 */
static int sstar_pcie_link_up(sstar_pcie_plat_t *priv, u32 cap_speed)
{
    int retries;

    if (is_link_up(priv))
    {
        printf("PCIe Link already up before configuration!\n");
        return 1;
    }

    /* Link configurations */
    sstar_pcie_configure(priv, cap_speed);

    /* Enable LTSSM */
    sstar_pcieif_start_link(priv->id);

    for (retries = 0; retries < LINK_WAIT_MAX_RETRIES; retries++)
    {
        if (is_link_up(priv))
        {
            dev_err(priv->dw.dev, "PCIe%d phy link came up\n", priv->id);
            return 0;
        }

        mdelay(LINK_WAIT_MSLEEP);
    }

    dev_err(priv->dw.dev, "PCIe%d phy link never came up\n", priv->id);

    /* Link maybe in Gen switch recovery but we need to wait more 1s */
    mdelay(1000);

    return -EIO;
}

static int sstar_pcie_host_init(struct udevice *dev)
{
    int                ret;
    sstar_pcie_plat_t *priv = dev_get_priv(dev);
    fdt_addr_t         cfg_base;
    fdt_size_t         cfg_size;

    sstar_pcieif_set_mode(priv->id, PCIE_IF_MODE_RC);

    sstar_pcieif_writel_dbi(priv->id, PCI_VENDOR_ID, 0x47AE19E5);
    sstar_pcieif_writel_dbi(priv->id, PCIE_LINK_WIDTH_SPEED_CONTROL,
                            PORT_LOGIC_AUTO_LANE_FLIP | PORT_LOGIC_LINK_WIDTH_2_LANES
                                | PORT_LOGIC_FAST_TRAIN_SEQ(0x2C));

    pcie_dw_setup_host(&priv->dw);

    /* config address & size is changed in dw common driver.
     * to comply config address & size in dts, overwritten them here again
     */
    cfg_base = dev_read_addr_size_name(dev, "config", &cfg_size);
    if (cfg_base != FDT_ADDR_T_NONE)
    {
        priv->dw.cfg_base = (void __iomem *)cfg_base;
        priv->dw.cfg_size = cfg_size;
    }

    ret = sstar_pcie_link_up(priv, LINK_SPEED_GEN_2);
    if (ret < 0)
        return ret;

    return 0;
}

static void sstar_pcie_clk_enable(sstar_pcie_plat_t *priv)
{
#ifdef CONFIG_SSTAR_CLK
    clk_enable_bulk(&priv->clks);
#else
    sstar_pcieif_clk_enable(priv->id);
#endif
}

static void sstar_pcie_writel_atu(struct pcie_dw *pci, u32 index, u32 reg, u32 val)
{
    sstar_pcie_plat_t *priv = dev_get_priv(pci->dev);

    reg += PCIE_GET_ATU_OUTB_UNR_REG_OFFSET(index);
    reg += DEFAULT_DBI_ATU_OFFSET;

    sstar_pcieif_writel_dbi(priv->id, reg, val);
}

static u32 sstar_pcie_readl_atu(struct pcie_dw *pci, u32 index, u32 reg)
{
    sstar_pcie_plat_t *priv = dev_get_priv(pci->dev);

    reg += PCIE_GET_ATU_OUTB_UNR_REG_OFFSET(index);
    reg += DEFAULT_DBI_ATU_OFFSET;

    return sstar_pcieif_readl_dbi(priv->id, reg);
}

static inline u64 sstar_pcie_cpu_addr_fixup(struct pcie_dw *pci, u64 cpu_addr)
{
    return cpu_addr - (u64)pci->cfg_base;
}

/**
 * sstar_pcie_prog_ob_atu_unroll() - Configure ATU for outbound accesses
 *
 * @pcie: Pointer to the PCI controller state
 * @index: ATU region index
 * @type: ATU accsess type
 * @cpu_addr: the physical address for the translation entry
 * @pci_addr: the pcie bus address for the translation entry
 * @size: the size of the translation entry
 *
 * Return: 0 is successful and -1 is failure
 */
int sstar_pcie_prog_ob_atu_unroll(struct pcie_dw *pci, int index, int type, u64 cpu_addr, u64 pci_addr, u32 size)
{
    u32 retries, val;

    cpu_addr = sstar_pcie_cpu_addr_fixup(pci, cpu_addr);

    dev_dbg(pci->dev, "OB ATU: index: %d, type: %d, cpu addr: %8llx, pci addr: %8llx, size: %8x\n", index, type,
            cpu_addr, pci_addr, size);

    sstar_pcie_writel_atu(pci, index, PCIE_ATU_UNR_LOWER_BASE, lower_32_bits(cpu_addr));
    sstar_pcie_writel_atu(pci, index, PCIE_ATU_UNR_UPPER_BASE, upper_32_bits(cpu_addr));
    sstar_pcie_writel_atu(pci, index, PCIE_ATU_UNR_LIMIT, lower_32_bits(cpu_addr + size - 1));
    sstar_pcie_writel_atu(pci, index, PCIE_ATU_UNR_LOWER_TARGET, lower_32_bits(pci_addr));
    sstar_pcie_writel_atu(pci, index, PCIE_ATU_UNR_UPPER_TARGET, upper_32_bits(pci_addr));
    sstar_pcie_writel_atu(pci, index, PCIE_ATU_UNR_REGION_CTRL1, type);
    sstar_pcie_writel_atu(pci, index, PCIE_ATU_UNR_REGION_CTRL2, PCIE_ATU_ENABLE);

    /*
     * Make sure ATU enable takes effect before any subsequent config
     * and I/O accesses.
     */
    for (retries = 0; retries < LINK_WAIT_MAX_IATU_RETRIES; retries++)
    {
        val = sstar_pcie_readl_atu(pci, index, PCIE_ATU_UNR_REGION_CTRL2);
        if (val & PCIE_ATU_ENABLE)
            return 0;

        udelay(LINK_WAIT_IATU);
    }
    dev_err(pci->dev, "outbound iATU is not being enabled\n");

    return -1;
}

/**
 * sstar_pcie_set_cfg_addr() - Configure the PCIe controller config space access
 *
 * @pcie: Pointer to the PCI controller state
 * @d: PCI device to access
 * @where: Offset in the configuration space
 *
 * Configures the PCIe controller to access the configuration space of
 * a specific PCIe device and returns the address to use for this
 * access.
 *
 * Return: Address that can be used to access the configation space
 *         of the requested device / offset
 */
static uintptr_t sstar_pcie_set_cfg_addr(struct pcie_dw *pcie, pci_dev_t d, uint where)
{
    int       bus = PCI_BUS(d) - pcie->first_busno;
    uintptr_t va_address;
    u32       atu_type;
    int       ret;

    /* Use dbi_base for own configuration read and write */
    if (!bus)
    {
        va_address = (uintptr_t)pcie->dbi_base;
        goto out;
    }

    if (bus == 1)
        /*
         * For local bus whose primary bus number is root bridge,
         * change TLP Type field to 4.
         */
        atu_type = PCIE_ATU_TYPE_CFG0;
    else
        /* Otherwise, change TLP Type field to 5. */
        atu_type = PCIE_ATU_TYPE_CFG1;

    /*
     * Not accessing root port configuration space?
     * Region #0 is used for Outbound CFG space access.
     * Direction = Outbound
     * Region Index = 0
     */
    d   = PCI_MASK_BUS(d);
    d   = PCI_ADD_BUS(bus, d);
    ret = sstar_pcie_prog_ob_atu_unroll(pcie, PCIE_ATU_REGION_INDEX1, atu_type, (u64)pcie->cfg_base, d << 8,
                                        pcie->cfg_size);
    if (ret)
        return (uintptr_t)ret;

    va_address = (uintptr_t)pcie->cfg_base;

out:
    va_address += where & ~0x3;

    return va_address;
}

/**
 * sstar_pcie_addr_valid() - Check for valid bus address
 *
 * @d: The PCI device to access
 * @first_busno: Bus number of the PCIe controller root complex
 *
 * Return 1 (true) if the PCI device can be accessed by this controller.
 *
 * Return: 1 on valid, 0 on invalid
 */
static int sstar_pcie_addr_valid(pci_dev_t d, int first_busno)
{
    if ((PCI_BUS(d) == first_busno) && (PCI_DEV(d) > 0))
        return 0;

    if ((PCI_BUS(d) == first_busno + 1) && (PCI_DEV(d) > 0))
        return 0;

    return 1;
}

/**
 * sstar_pcie_read_config() - Read from configuration space
 *
 * @bus: Pointer to the PCI bus
 * @bdf: Identifies the PCIe device to access
 * @offset: The offset into the device's configuration space
 * @valuep: A pointer at which to store the read value
 * @size: Indicates the size of access to perform
 *
 * Read a value of size @size from offset @offset within the configuration
 * space of the device identified by the bus, device & function numbers in @bdf
 * on the PCI bus @bus.
 *
 * Return: 0 on success
 */
int sstar_pcie_read_config(const struct udevice *bus, pci_dev_t bdf, uint offset, ulong *valuep, enum pci_size_t size)
{
    sstar_pcie_plat_t *priv = dev_get_priv(bus);
    struct pcie_dw *   pcie = &(priv->dw);
    uintptr_t          va_address;
    ulong              value;

    if (!sstar_pcie_addr_valid(bdf, pcie->first_busno))
    {
        debug("bdf out of range\n");
        *valuep = pci_get_ff(size);
        return 0;
    }

    va_address = sstar_pcie_set_cfg_addr(pcie, bdf, offset);

    value = readl((void __iomem *)va_address);

    debug("BDF%08x cfgrd(x%lx) = x%lx\n", bdf, va_address, value);
    *valuep = pci_conv_32_to_size(value, offset, size);

    return sstar_pcie_prog_ob_atu_unroll(pcie, PCIE_ATU_REGION_INDEX1, PCIE_ATU_TYPE_IO, pcie->io.phys_start,
                                         pcie->io.bus_start, pcie->io.size);
}

/**
 * sstar_pcie_write_config() - Write to configuration space
 *
 * @bus: Pointer to the PCI bus
 * @bdf: Identifies the PCIe device to access
 * @offset: The offset into the device's configuration space
 * @value: The value to write
 * @size: Indicates the size of access to perform
 *
 * Write the value @value of size @size from offset @offset within the
 * configuration space of the device identified by the bus, device & function
 * numbers in @bdf on the PCI bus @bus.
 *
 * Return: 0 on success
 */
int sstar_pcie_write_config(struct udevice *bus, pci_dev_t bdf, uint offset, ulong value, enum pci_size_t size)
{
    sstar_pcie_plat_t *priv = dev_get_priv(bus);
    struct pcie_dw *   pcie = &(priv->dw);
    uintptr_t          va_address;
    ulong              old;

    if (!sstar_pcie_addr_valid(bdf, pcie->first_busno))
    {
        debug("bdf out of range\n");
        return 0;
    }

    va_address = sstar_pcie_set_cfg_addr(pcie, bdf, offset);

    old   = readl((void __iomem *)va_address);
    value = pci_conv_size_to_32(old, value, offset, size);
    writel(value, (void __iomem *)va_address);
    debug("BDF%08x cfgwr(x%lx) x%lx -> x%lx\n", bdf, va_address, old, value);

    return sstar_pcie_prog_ob_atu_unroll(pcie, PCIE_ATU_REGION_INDEX1, PCIE_ATU_TYPE_IO, pcie->io.phys_start,
                                         pcie->io.bus_start, pcie->io.size);
}

/**
 * Probe the PCIe bus for active link
 *
 * @dev: A pointer to the device being operated on
 *
 * Probe for an active link on the PCIe bus and configure the controller
 * to enable this port.
 *
 * Return: 0 on success, else -ENODEV
 */
static int sstar_pcie_probe(struct udevice *dev)
{
    sstar_pcie_plat_t *    priv = dev_get_priv(dev);
    struct udevice *       ctlr = pci_get_controller(dev);
    struct pci_controller *hose = dev_get_uclass_priv(ctlr);
    int                    ret  = 0;

    priv->dw.first_busno = dev_seq(dev);
    priv->dw.dev         = dev;

#ifdef CONFIG_SSTAR_CLK
    ret = clk_get_bulk(dev, &priv->clks);
    if (ret)
    {
        dev_err(dev, "Failed to get clks");
        return ret;
    }
#endif
    sstar_pcie_clk_enable(priv);

    ret = sstar_pcie_host_init(dev);
    if (ret)
        return ret;

    dev_dbg(dev, "PCIe-%d: Gen%d-x%d, Bus%d\n", dev_seq(dev), pcie_dw_get_link_speed(&priv->dw),
            pcie_dw_get_link_width(&priv->dw), hose->first_busno);

    return sstar_pcie_prog_ob_atu_unroll(&priv->dw, PCIE_ATU_REGION_INDEX0, PCIE_ATU_TYPE_MEM, priv->dw.mem.phys_start,
                                         priv->dw.mem.bus_start, priv->dw.mem.size);
}

static int sstar_pcie_of_to_plat(struct udevice *dev)
{
    sstar_pcie_plat_t *priv             = dev_get_priv(dev);
    u32                portid           = 0;
    u32                use_internal_clk = 0;

    priv->dw.dbi_base = (void *)dev_read_addr_name(dev, "dbi");
    if (!priv->dw.dbi_base)
        return -ENODEV;

    priv->dw.cfg_base = (void *)dev_read_addr_size_name(dev, "config", &priv->dw.cfg_size);
    if (!priv->dw.cfg_base)
        return -ENODEV;

    dev_read_u32(dev, "portid", &portid);
    priv->id = portid;

    dev_read_u32(dev, "use_internal_clk", &use_internal_clk);
    // ref 100M_Clock reg-setting
    if (use_internal_clk)
        sstar_pcieif_internalclk_en(portid);

    dev_dbg(dev, "= portid %d =\n", priv->id);
    dev_dbg(dev, "[dbi] at 0x%p\n", priv->dw.dbi_base);
    dev_dbg(dev, "[cfg] at 0x%p\n", priv->dw.cfg_base);

    return 0;
}

static const struct dm_pci_ops sstar_pcie_ops = {
    .read_config  = sstar_pcie_read_config,
    .write_config = sstar_pcie_write_config,
};

static const struct udevice_id sstar_pcie_ids[] = {{.compatible = "snps,dw-pcie"}, {}};

U_BOOT_DRIVER(sstar_pcie) = {
    .name       = "sstar_pcie",
    .id         = UCLASS_PCI,
    .of_match   = sstar_pcie_ids,
    .of_to_plat = sstar_pcie_of_to_plat,
    .ops        = &sstar_pcie_ops,
    .probe      = sstar_pcie_probe,
    .priv_auto  = sizeof(struct sstar_pcie_plat),
};
