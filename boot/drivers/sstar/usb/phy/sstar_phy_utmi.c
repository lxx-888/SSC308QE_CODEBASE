/*
 * sstar_phy_utmi.c - Sigmastar
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
#include <clk.h>
#include <dm.h>
#include <asm/global_data.h>
#include <dm/device_compat.h>
#include <dm/lists.h>
#include <generic-phy.h>
#include <asm/io.h>
#include <regmap.h>
#include <syscon.h>
#include <linux/delay.h>
#include <linux/iopoll.h>

// 0x00: 550mv, 0x20: 575, 0x40: 600, 0x60: 625
#define UTMI_DISCON_LEVEL_2A              (0x62)
#define TX_RX_RESET_CLK_GATING_ECO_BITSET BIT(5)
#define LS_CROSS_POINT_ECO_BITSET         BIT(6)

struct sstar_phy_utmi
{
    void __iomem *  reg;
    struct clk_bulk clks;
    bool            dp_dm_swap;
    bool            only_for_dwc3;
};

static void sstar_utmi_sync_trim_value(struct phy *phy)
{
    // int value;
    struct udevice *       dev       = phy->dev;
    struct sstar_phy_utmi *sstar_phy = dev_get_priv(dev);
    void __iomem *         base      = sstar_phy->reg;
    struct regmap *        regmap;
    phys_addr_t            otp_base;

    int rterm, hs_tx_current;

    regmap = syscon_regmap_lookup_by_phandle(dev, "sstar,otp-syscon");

    if (!IS_ERR(regmap))
    {
        if (ofnode_device_is_compatible(dev_ofnode(dev), "sstar,infinity7-pipe"))
        {
            otp_base = regmap->ranges[0].start;

            if (readw(otp_base + (0x56 << 2)) & BIT(6))
            {
                rterm = readw(otp_base + (0x55 << 2));
                rterm = rterm >> 10;
                rterm &= 0x3f;
                hs_tx_current = readw(otp_base + (0x56 << 2));
                hs_tx_current &= 0x3f;
                clrbits_16(base + (0x45 << 2), 0x003f);
                setbits_16(base + (0x45 << 2), rterm);
                clrbits_16(base + (0x45 << 2), 0x03f0);
                setbits_16(base + (0x45 << 2), hs_tx_current << 4);
            }
        }
    }

    dev_dbg(dev, "%s rterm = 0x%04x\r\n", __func__, readw(base + (0x45 << 2)) & 0x003f);
    dev_dbg(dev, "%s hs_tx_current = 0x%04x\r\n", __func__, readw(base + (0x44 << 2)) & 0x03f0);
    dev_dbg(dev, "%s\r\n", __func__);
}

static void __maybe_unused sstar_phy_utmi_avoid_floating(struct phy *phy)
{
    struct udevice *       dev       = phy->dev;
    struct sstar_phy_utmi *sstar_phy = dev_get_priv(dev);
    void __iomem *         base      = sstar_phy->reg;

    clrbits_16(base + (0x0 << 2), (BIT(3) | BIT(4) | BIT(5))); // DP_PUEN = 0 DM_PUEN = 0 R_PUMODE = 0

    /*
     * patch for DM always keep high issue
     * init overwrite register
     */
    setbits_16(base + (0x05 << 2), BIT(6)); // hs_txser_en_cb = 1
    clrbits_16(base + (0x05 << 2), BIT(7)); // hs_se0_cb = 0

    /* Turn on overwirte mode for D+/D- floating issue when UHC reset
     * Before UHC reset, R_DP_PDEN = 1, R_DM_PDEN = 1, tern_ov = 1 */
    setbits_16(base + (0x0 << 2), (BIT(7) | BIT(6) | BIT(1)));
    /* new HW term overwrite: on */
    // setbits_16(base + (0x29 << 2), (BIT5 | BIT4 | BIT3 | BIT2 | BIT1 | BIT0));
}

static void sstar_phy_utmi_disconnect_window_select(struct phy *phy)
{
    struct udevice *       dev       = phy->dev;
    struct sstar_phy_utmi *sstar_phy = dev_get_priv(dev);
    void __iomem *         base      = sstar_phy->reg;

    clrbits_16(base + (0x01 << 2), BIT(11) | BIT(12) | BIT(13)); // Disconnect window select
    setbits_16(base + (0x01 << 2), (0x05 << 11));
    dev_dbg(phy->dev, "%s\r\n", __func__);
}

static void sstar_phy_utmi_ISI_effect_improvement(struct phy *phy)
{
    struct udevice *       dev       = phy->dev;
    struct sstar_phy_utmi *sstar_phy = dev_get_priv(dev);
    void __iomem *         base      = sstar_phy->reg;

    setbits_16(base + (0x04 << 2), BIT(8)); // ISI effect improvement
    dev_dbg(phy->dev, "%s\r\n", __func__);
}

static void sstar_phy_utmi_RX_anti_dead_loc(struct phy *phy)
{
    struct udevice *       dev       = phy->dev;
    struct sstar_phy_utmi *sstar_phy = dev_get_priv(dev);
    void __iomem *         base      = sstar_phy->reg;

    setbits_16(base + (0x04 << 2), BIT(15)); // UTMI RX anti-dead-loc
    dev_dbg(phy->dev, "%s\r\n", __func__);
}

static void sstar_phy_utmi_chirp_signal_source_select(struct phy *phy)
{
    struct udevice *       dev       = phy->dev;
    struct sstar_phy_utmi *sstar_phy = dev_get_priv(dev);
    void __iomem *         base      = sstar_phy->reg;

    setbits_16(base + (0x0a << 2), BIT(13)); // Chirp signal source select
    dev_dbg(phy->dev, "%s\r\n", __func__);
}

static void sstar_phy_utmi_disable_improved_CDR(struct phy *phy)
{
    struct udevice *       dev       = phy->dev;
    struct sstar_phy_utmi *sstar_phy = dev_get_priv(dev);
    void __iomem *         base      = sstar_phy->reg;

    clrbits_16(base + (0x03 << 2), BIT(9)); // Disable improved CDR
    dev_dbg(phy->dev, "%s\r\n", __func__);
}

static void sstar_phy_utmi_RX_HS_CDR_stage_control(struct phy *phy)
{
    struct udevice *       dev       = phy->dev;
    struct sstar_phy_utmi *sstar_phy = dev_get_priv(dev);
    void __iomem *         base      = sstar_phy->reg;

    // RX HS CDR stage control
    clrbits_16(base + (0x03 << 2), (BIT(5) | BIT(6)));
    setbits_16(base + (0x03 << 2), BIT(6));
    dev_dbg(phy->dev, "%s\r\n", __func__);
}

static int sstar_phy_utmi_calibrate(struct phy *phy)
{
    struct udevice *       dev       = phy->dev;
    struct sstar_phy_utmi *sstar_phy = dev_get_priv(dev);
    void __iomem *         base      = sstar_phy->reg;

    u16 u16_result;

    dev_dbg(phy->dev, "%s start\r\n", __func__);
    /* Init UTMI squelch level setting befor CA */
    writew((UTMI_DISCON_LEVEL_2A & (BIT(3) | BIT(2) | BIT(1) | BIT(0))), base + (0x15 << 2));
    dev_dbg(phy->dev, "%s, squelch level 0x%02x\n", __func__, readb(base + (0x15 << 2)));

    setbits_16(base + (0x1e << 2), BIT(0)); // set CA_START as 1
    mdelay(1);

    clrbits_16(base + (0x1e << 2), BIT(0)); // release CA_START

    if (0 > readw_poll_timeout((base + (0x1e << 2)), u16_result, u16_result & BIT(1),
                               10000000)) // polling bit <1> (CA_END)
    {
        dev_dbg(phy->dev, "%s, calibration timeout\n", __func__);
        return -ETIMEDOUT;
    }

#if CONFIG_IS_ENABLED(SSTAR_USB_DWC3_HOST)
    writew(UTMI_DISCON_LEVEL_2A, (base + (0x15 << 2)));
#endif
    dev_dbg(phy->dev, "%s\r\n", __func__);
    return 0;
}

static void sstar_phy_utmi_reset_assert(struct phy *phy)
{
    struct udevice *       dev       = phy->dev;
    struct sstar_phy_utmi *sstar_phy = dev_get_priv(dev);
    void __iomem *         base      = sstar_phy->reg;

    setbits_16(base + (0x03 << 2),
               BIT(0) | BIT(1) | BIT(8));    // bit0: RX sw reset; bit1: Tx sw reset; bit8: Tx FSM sw reset;
    setbits_16(base + (0x08 << 2), BIT(12)); // bit12: pwr good reset
}

static void sstar_phy_utmi_reset_deassert(struct phy *phy)
{
    struct udevice *       dev       = phy->dev;
    struct sstar_phy_utmi *sstar_phy = dev_get_priv(dev);
    void __iomem *         base      = sstar_phy->reg;

    clrbits_16(base + (0x03 << 2),
               BIT(0) | BIT(1) | BIT(8));    // bit0: RX sw reset; bit1: Tx sw reset; bit8: Tx FSM sw reset;
    clrbits_16(base + (0x08 << 2), BIT(12)); // bit12: pwr good reset
}

static int sstar_phy_utmi_reset(struct phy *phy)
{
    sstar_phy_utmi_reset_assert(phy);
    mdelay(1);
    sstar_phy_utmi_reset_deassert(phy);

    dev_dbg(phy->dev, "%s\n", __func__);
    return 0;
}

static int sstar_phy_utmi_exit(struct phy *phy)
{
    struct udevice *       dev       = phy->dev;
    struct sstar_phy_utmi *sstar_phy = dev_get_priv(dev);
    void __iomem *         base      = sstar_phy->reg;

    clrbits_8(base + (0x02 << 2), BIT(7));
    setbits_16(base + (0x03 << 2), BIT(0) | BIT(1) | BIT(8));
    setbits_16(base + (0x08 << 2), BIT(12));
    mdelay(1);
    clrbits_16(base + (0x03 << 2), BIT(0) | BIT(1) | BIT(8));
    clrbits_16(base + (0x08 << 2), BIT(12));

    return 0;
}

static int sstar_phy_utmi_power_off(struct phy *phy)
{
    u16 u16_val;

    struct udevice *       dev       = phy->dev;
    struct sstar_phy_utmi *sstar_phy = dev_get_priv(dev);
    void __iomem *         base      = sstar_phy->reg;

    u16_val = BIT(0) | BIT(1) | BIT(8) | BIT(9) | BIT(10) | BIT(11) | BIT(12) | BIT(13) | BIT(14);

    setbits_16(base + (0x0 << 2), u16_val);
    mdelay(5);

    dev_dbg(phy->dev, "%s\n", __func__);
    return 0;
}

static int sstar_phy_utmi_power_on(struct phy *phy)
{
    sstar_phy_utmi_reset_deassert(phy);
    sstar_phy_utmi_calibrate(phy);
    sstar_utmi_sync_trim_value(phy);
    dev_dbg(phy->dev, "%s\n", __func__);
    return 0;
}

static int sstar_phy_utmi_init(struct phy *phy)
{
    struct udevice *       dev       = phy->dev;
    struct sstar_phy_utmi *sstar_phy = dev_get_priv(dev);
    void __iomem *         base      = sstar_phy->reg;

    writew(0x0C2F, base + (0x04 << 2));

    if (sstar_phy->only_for_dwc3)
    {
        setbits_16(base + (0x1f << 2), 0x0100); // for DesignWare USB3 DRD Controller
    }

    // for extra utmi0 setting
    clrbits_16(phys_to_virt(0x1f284200 + (0x6f << 2)), BIT(15));

    sstar_phy_utmi_power_off(phy);

    sstar_phy_utmi_reset_assert(phy);
    /*
        [15] : enable regulator of usb2_atop
        [12] : enable FS/LS transceiver of usb2_atop
        [10] : enable HS squelch circuit of usb2_atop
        [7] : enable FS/LS DM pull-down resistor
        [6] : enable FS/LS DP pull-down resistor
        [2] : enable reference block of usb2_atop
        [1] : enable FS/LS termination force mode
    */
    writew(0x6BC3, (base + (0x00 << 2)));
    mdelay(1);
    // OUTREG8(base + (0x00 << 2), 0x69); // Turn on UPLL, reg_pdn: bit<9>
    // dev_dbg(&phy->dev, "%s 0x69\n", __func__);
    // mdelay(2);

    /*
        [14] : enable HS current reference block of usb2_atop
        [13] : enable VBUS detector of usb2_atop
        [11] : enable HS pre-amplifier of usb2_atop
        [8] : enable HS de-serializer of usb2_atop
        [7] : disable FS/LS DM pull-down resistor
        [6] : disable FS/LS DP pull-down resistor
        [1] : disable FS/LS termination force mode
    */
    writew(0x0001, (base + (0x00 << 2))); // Turn all (including hs_current) use override mode
    // Turn on UPLL, reg_pdn: bit<9>
    mdelay(3);

    if (sstar_phy->dp_dm_swap)
    {
        clrbits_8(base + (0x5 << 2) + 1, BIT(5)); // dp dm swap
    }

    sstar_phy_utmi_RX_HS_CDR_stage_control(phy);
    sstar_phy_utmi_disable_improved_CDR(phy);
    sstar_phy_utmi_chirp_signal_source_select(phy);
    sstar_phy_utmi_disconnect_window_select(phy);
    sstar_phy_utmi_disable_improved_CDR(phy);
    sstar_phy_utmi_RX_anti_dead_loc(phy);
    sstar_phy_utmi_ISI_effect_improvement(phy);

#if CONFIG_IS_ENABLED(SSTAR_USB_DWC3_HOST)
    /* bit<3> for 240's phase as 120's clock set 1, bit<4> for 240Mhz in mac 0 for faraday 1 for etron */
    setbits_16(base + (0x04 << 2), BIT(3));
    dev_dbg(phy->dev, "Init UTMI disconnect level setting\r\n");
    /* Init UTMI eye diagram parameter setting */
    setbits_16(base + (0x16 << 2), 0x210);
    setbits_16(base + (0x17 << 2), 0x8100);
    dev_dbg(phy->dev, "Init UTMI eye diagram parameter setting\r\n");

    /* Enable hw auto deassert sw reset(tx/rx reset) */
    setbits_8(base + (0x02 << 2), TX_RX_RESET_CLK_GATING_ECO_BITSET);

    /* Change override to hs_txser_en.	Dm always keep high issue */
    setbits_16(base + (0x08 << 2), BIT(6));
#else
    setbits_16(base + (0x16 << 2), 0x0290);
    setbits_16(base + (0x17 << 2), 0110);

    setbits_16(base + (0x05 << 2), BIT(15)); // set reg_ck_inv_reserved[6] to solve timing problem
    dev_dbg(phy->dev, "%s, set reg_ck_inv_reserved[6] to solve timing problem.\n", __func__);
    setbits_16(base + (0x02 << 2), BIT(7)); // avoid glitch

    setbits_16(base + (0x1A << 2), 0x62); // Chirp k detection level: 0x80 => 400mv, 0x20 => 575mv
#endif
    return 0;
}

static struct phy_ops sstar_phy_utmi_ops = {
    .init      = sstar_phy_utmi_init,
    .exit      = sstar_phy_utmi_exit,
    .reset     = sstar_phy_utmi_reset,
    .power_on  = sstar_phy_utmi_power_on,
    .power_off = sstar_phy_utmi_power_off,
    .exit      = sstar_phy_utmi_exit,
};

static int sstar_phy_utmi_of_to_plat(struct udevice *dev)
{
    struct sstar_phy_utmi *phy  = dev_get_priv(dev);
    ofnode                 node = dev_ofnode(dev);

    phy->reg = devfdt_remap_addr(dev);

    if (NULL == phy->reg)
    {
        return 0;
    }

    phy->dp_dm_swap    = ofnode_read_bool(node, "sstar,utmi-dp-dm-swap");
    phy->only_for_dwc3 = ofnode_read_bool(node, "sstar,only-for-dwc3");

    return 0;
}

static int sstar_phy_utmi_probe(struct udevice *dev)
{
    struct sstar_phy_utmi *phy = dev_get_priv(dev);
    int                    ret;
    dev_dbg(dev, "%s start.\r\n", __func__);
    ret = clk_get_bulk(dev, &phy->clks);
    if (ret == -ENOSYS || ret == -ENOENT)
        goto out;
    if (ret)
        return ret;

#if CONFIG_IS_ENABLED(SSTAR_CLK)
    ret = clk_enable_bulk(&phy->clks);
    if (ret)
    {
        clk_release_bulk(&phy->clks);
        return ret;
    }
#endif
out:
    /*
            [11] : enable PLL of usb2_atop without register
            [7] : enable bandgap current of usb2_atop
   */
    writew(0x0C2F, phy->reg + (0x04 << 2));
    dev_dbg(dev, "%s end.\r\n", __func__);
    return 0;
}

static int sstar_phy_utmi_remove(struct udevice *dev)
{
#if CONFIG_IS_ENABLED(CLK)
    struct sstar_phy_utmi *phy = dev_get_priv(dev);

    clk_release_bulk(&phy->clks);
#endif
    return 0;
}

static const struct udevice_id sstar_phy_utmi_ids[] = {
    {.compatible = "sstar,infinity7-phy-utmi"}, {.compatible = "sstar,generic-utmi"}, {}};

U_BOOT_DRIVER(sstar_phy_utmi) = {
    .name       = "sstar-phy-utmi",
    .id         = UCLASS_PHY,
    .of_match   = sstar_phy_utmi_ids,
    .of_to_plat = sstar_phy_utmi_of_to_plat,
    .ops        = &sstar_phy_utmi_ops,
    .probe      = sstar_phy_utmi_probe,
    .remove     = sstar_phy_utmi_remove,
    .priv_auto  = sizeof(struct sstar_phy_utmi),
};
