/*
 * sstar_phy_pipe.c - Sigmastar
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

struct sstar_phy_pipe
{
    void __iomem *  reg;
    struct clk_bulk clks;
};

static void sstar_pipe_show_trim_value(struct phy *phy)
{
    struct udevice *       dev       = phy->dev;
    struct sstar_phy_pipe *sstar_phy = dev_get_priv(dev);
    void __iomem *         pipe_phyd = sstar_phy->reg;

    dev_dbg(dev, "%s tx_r50 = 0x%04x\n", __func__, readw(pipe_phyd + (0x2a << 2)) & 0x0f80);
    dev_dbg(dev, "%s rx_r50 = 0x%04x\n", __func__, readw(pipe_phyd + (0x3c << 2)) & 0x01f0);
    dev_dbg(dev, "%s tx_ibias = 0x%04x\n", __func__, readw(pipe_phyd + (0x50 << 2)) & 0xfc00);
    dev_dbg(dev, "%s completed\n", __func__);
}

static void sstar_phy_pipe_tx_swing_and_de_emphasis(struct phy *phy)
{
    struct udevice *       dev       = phy->dev;
    struct sstar_phy_pipe *sstar_phy = dev_get_priv(dev);
    void __iomem *         pipe_phyd, *pipe_phya2;
    int                    tx_swing[3];

    pipe_phyd  = sstar_phy->reg;
    pipe_phya2 = sstar_phy->reg + (0x200 * 3);

    if (ofnode_read_u32_array(dev_ofnode(dev), "sstar,tx-swing-and-de-emphasis", tx_swing, ARRAY_SIZE(tx_swing)))
    {
        return;
    }

    setbits_16(pipe_phyd + (0x0a << 2), BIT(5) | BIT(6));

    clrbits_16(pipe_phyd + (0x41 << 2), 0x00fc);
    // setbits_16(pipe_phyd + (0x41 << 2), 0x00c8);
    setbits_16(pipe_phyd + (0x41 << 2), 0x00fc & (tx_swing[0] << 2));
    clrbits_16(pipe_phyd + (0x41 << 2), 0x3f00);
    // setbits_16(pipe_phyd + (0x41 << 2), 0x2800);
    setbits_16(pipe_phyd + (0x41 << 2), 0x3f00 & (tx_swing[1] << 8));

    setbits_16(pipe_phyd + (0x0a << 2), BIT(0) | BIT(1));

    if (ofnode_device_is_compatible(dev_ofnode(dev), "sstar,generic-pipe"))
    {
        setbits_16(pipe_phyd + (0x26 << 2), BIT(0));
        clrsetbits_16(pipe_phyd + (0x26 << 2), 0x000e, 0x000e & (tx_swing[2] << 1));
    }
    else
    {
        setbits_16(pipe_phya2 + (0x0f << 2), BIT(12));
        clrsetbits_16(pipe_phya2 + (0x11 << 2), 0xf000, 0xf000 & (tx_swing[2] << 12));
    }

    setbits_16(pipe_phya2 + (0x2f << 2), BIT(4));

    dev_dbg(dev, "%s completed.\n", __func__);
}

void sstar_phy_pipe_tx_polarity_inv_disabled(struct phy *phy)
{
    struct udevice *       dev       = phy->dev;
    struct sstar_phy_pipe *sstar_phy = dev_get_priv(dev);
    void __iomem *         pipe_phyd = sstar_phy->reg;

    // clrbits_16(base + PHYD_REG_RG_TX_POLARTY_INV, BIT_TX_POLARTY_INV_EN);
    writew(0x420f, pipe_phyd + (0x12 << 2));
    dev_dbg(dev, "%s, pipe_phyd\n", __func__);
}

void sstar_phy_pipe_LPFS_det_power_on(struct phy *phy)
{
    struct udevice *       dev       = phy->dev;
    struct sstar_phy_pipe *sstar_phy = dev_get_priv(dev);
    void __iomem *         pipe_phyd = sstar_phy->reg;

    clrbits_16(pipe_phyd + (0x34 << 2), 0x4000); // RG_SSUSB_LFPS_PWD[14] = 0 // temp add here
    dev_dbg(phy->dev, "%s, pipe_phyd\n", __func__);
}

void sstar_phy_pipe_LPFS_det_hw_mode(struct phy *phy)
{
    struct udevice *       dev        = phy->dev;
    struct sstar_phy_pipe *sstar_phy  = dev_get_priv(dev);
    void __iomem *         pipe_phyd  = sstar_phy->reg;
    void __iomem *         pipe_phya0 = pipe_phyd + (0x200 * 1);

    clrbits_16(pipe_phyd + (0xc << 2), BIT(15));
    clrbits_16(pipe_phya0 + (0x3 << 2), BIT(0));
    dev_dbg(phy->dev, "%s completed.\n", __func__);
}

void sstar_phy_pipe_deassert_hw_reset(struct phy *phy)
{
    struct udevice *       dev        = phy->dev;
    struct sstar_phy_pipe *sstar_phy  = dev_get_priv(dev);
    void __iomem *         pipe_phya1 = sstar_phy->reg + (0x200 * 2);

    writew(0x0001, pipe_phya1 + (0x00 << 2));
    dev_dbg(phy->dev, "%s completed\n", __func__);
}

void sstar_phy_pipe_assert_sw_reset(struct phy *phy)
{
    struct udevice *       dev        = phy->dev;
    struct sstar_phy_pipe *sstar_phy  = dev_get_priv(dev);
    void __iomem *         pipe_phya1 = sstar_phy->reg + (0x200 * 2);

    clrbits_16(pipe_phya1 + (0x00 << 2), 0x0010);
    dev_dbg(phy->dev, "%s, pipe_phya1[bit4]\n", __func__);
}

void sstar_phy_pipe_deassert_sw_reset(struct phy *phy)
{
    struct udevice *       dev        = phy->dev;
    struct sstar_phy_pipe *sstar_phy  = dev_get_priv(dev);
    void __iomem *         pipe_phya1 = sstar_phy->reg + (0x200 * 2);

    setbits_16(pipe_phya1 + (0x00 << 2), 0x0010);
    dev_dbg(phy->dev, "%s, pipe_phya1[bit4]\n", __func__);
}

void sstar_phy_pipe_sw_reset(struct phy *phy)
{
    struct udevice *       dev        = phy->dev;
    struct sstar_phy_pipe *sstar_phy  = dev_get_priv(dev);
    void __iomem *         pipe_phya0 = sstar_phy->reg + (0x200 * 1);

    // writew(base + (0x06 << 2), 0x0003);
    setbits_16(pipe_phya0 + (0x06 << 2), 0x0001);
    dev_dbg(phy->dev, "%s, pipe_phya0\n", __func__);
}

void sstar_phy_pipe_synthesis_enabled(struct phy *phy)
{
    struct udevice *       dev        = phy->dev;
    struct sstar_phy_pipe *sstar_phy  = dev_get_priv(dev);
    void __iomem *         pipe_phya0 = sstar_phy->reg + (0x200 * 1);
    u32                    syn_clk;

    if (!ofnode_read_u32(dev_ofnode(dev), "sstar,synthesiszer-clk", &syn_clk))
    {
        clrbits_16(pipe_phya0 + (0x44 << 2), 0x0001); // synth clk enabled
        mdelay(1);
        writew((syn_clk >> 16) & 0xffff, pipe_phya0 + (0x41 << 2)); // synth clk
        writew(syn_clk & 0xffff, pipe_phya0 + (0x40 << 2));         // synth clk
        setbits_16(pipe_phya0 + (0x44 << 2), 0x0001);               // synth clk enabled
        mdelay(1);
        // clrbits_16(pipe_phya0 + (0x44 << 2), 0x0001);
        dev_dbg(phy->dev, "%s, completed\n", __func__);
    }
}

void sstar_phy_pipe_tx_loop_div(struct phy *phy)
{
    struct udevice *       dev        = phy->dev;
    struct sstar_phy_pipe *sstar_phy  = dev_get_priv(dev);
    void __iomem *         pipe_phya0 = sstar_phy->reg + (0x200 * 1);

    if (ofnode_device_is_compatible(dev_ofnode(dev), "sstar,generic-pipe"))
        writew(0x0005, pipe_phya0 + (0x21 << 2));
    else
        writew(0x0010, pipe_phya0 + (0x21 << 2)); // txpll loop div second

    dev_dbg(phy->dev, "%s, pipe_phya0\n", __func__);
}

void sstar_pipe_tx_post_div_and_test(struct phy *phy)
{
    struct udevice *       dev        = phy->dev;
    struct sstar_phy_pipe *sstar_phy  = dev_get_priv(dev);
    void __iomem *         pipe_phya0 = sstar_phy->reg + (0x200 * 1);

    if (ofnode_device_is_compatible(dev_ofnode(dev), "sstar,generic-pipe"))
    {
        writew(0x0a00, pipe_phya0 + (0x11 << 2)); // gcr_sata_test
    }
    else
    {
        writew(0x2a00, pipe_phya0 + (0x11 << 2)); // gcr_sata_test
    }

    writew(0x0000, pipe_phya0 + (0x10 << 2)); // gcr_sata_test
    dev_dbg(phy->dev, "%s completed.\n", __func__);
}

void sstar_phy_pipe_EQ_rstep(struct phy *phy)
{
    struct udevice *       dev       = phy->dev;
    struct sstar_phy_pipe *sstar_phy = dev_get_priv(dev);
    void __iomem *         pipe_phyd = sstar_phy->reg;

    writew(0x5582, pipe_phyd + (0x42 << 2));
    dev_dbg(phy->dev, "%s, pipe_phyd\n", __func__);
}

void sstar_phy_pipe_DFE(struct phy *phy)
{
    struct udevice *       dev        = phy->dev;
    struct sstar_phy_pipe *sstar_phy  = dev_get_priv(dev);
    void __iomem *         pipe_phya2 = sstar_phy->reg + (0x200 * 3);

    writew(0x0, pipe_phya2 + (0x0d << 2));
    writew(0x0, pipe_phya2 + (0x0e << 2));
    writew(0x0171, pipe_phya2 + (0x0f << 2));
    dev_dbg(phy->dev, "%s, sstar_phy_pipe_DFE\n", __func__);
}

void sstar_sata_rx_tx_pll_frq_det_enabled(struct phy *phy)
{
    struct udevice *       dev        = phy->dev;
    struct sstar_phy_pipe *sstar_phy  = dev_get_priv(dev);
    void __iomem *         pipe_phya0 = sstar_phy->reg + (0x200 * 1);

    clrbits_16(pipe_phya0 + 0x54 * 4, 0xf); // The continue lock number to judge rxpll frequency is lock or not
    clrbits_16(pipe_phya0 + 0x73 * 4,
               0xff); // PLL State :
                      // The lock threshold distance between predict counter number and real counter number of rxpll
    clrbits_16(pipe_phya0 + 0x77 * 4,
               0xff); // CDR State :
                      // The lock threshold distance between predict counter number and real counter number of rxpll
    writew(0, pipe_phya0 + 0x56 * 4);
    setbits_16(pipe_phya0 + 0x56 * 4, 0x5dc0); // the time out reset reserve for PLL unlock for cdr_pd fsm PLL_MODE
                                               // state waiting time default : 20us  (24MHz base : 480 * 41.667ns)

    writew(0, pipe_phya0 + 0x57 * 4);
    setbits_16(pipe_phya0 + 0x57 * 4, 0x1e0); // the time out reset reserve for CDR unlock

    clrbits_16(pipe_phya0 + 0x70 * 4, 0x04); // reg_sata_phy_rxpll_det_hw_mode_always[2] = 0
    // Enable RXPLL frequency lock detection
    setbits_16(pipe_phya0 + 0x70 * 4, 0x02); // reg_sata_phy_rxpll_det_sw_enable_always[1] = 1
    // Enable TXPLL frequency lock detection
    setbits_16(pipe_phya0 + 0x60 * 4, 0x02);   // reg_sata_phy_txpll_det_sw_enable_always[1] = 1
    setbits_16(pipe_phya0 + 0x50 * 4, 0x2000); // cdr state change to freq_unlokc_det[13] = 1
    dev_dbg(phy->dev, "%s, pipe_phya0\n", __func__);
}

void sstar_phy_pipe_txpll_vco_ldo(struct phy *phy)
{
    struct udevice *       dev        = phy->dev;
    struct sstar_phy_pipe *sstar_phy  = dev_get_priv(dev);
    void __iomem *         pipe_phya2 = sstar_phy->reg + (0x200 * 3);

    writew(0x0180, pipe_phya2 + (0x17 << 2));
    dev_dbg(phy->dev, "%s, pipe_phya2\n", __func__);
}

void sstar_phy_pipe_txpll_en_d2s(struct phy *phy)
{
    struct udevice *       dev        = phy->dev;
    struct sstar_phy_pipe *sstar_phy  = dev_get_priv(dev);
    void __iomem *         pipe_phya2 = sstar_phy->reg + (0x200 * 3);

    writew(0x000f, pipe_phya2 + (0x1c << 2));
    dev_dbg(phy->dev, "%s, pipe_phya2\n", __func__);
}

void sstar_phy_pipe_txpll_en_vco(struct phy *phy)
{
    struct udevice *       dev        = phy->dev;
    struct sstar_phy_pipe *sstar_phy  = dev_get_priv(dev);
    void __iomem *         pipe_phya2 = sstar_phy->reg + (0x200 * 3);

    writew(0x0100, (pipe_phya2 + (0x1b << 2)));
    dev_dbg(phy->dev, "%s, pipe_phya2\n", __func__);
}

void sstar_phy_pipe_txpll_ictrl_new(struct phy *phy)
{
    struct udevice *       dev        = phy->dev;
    struct sstar_phy_pipe *sstar_phy  = dev_get_priv(dev);
    void __iomem *         pipe_phya2 = sstar_phy->reg + (0x200 * 3);

    writew(0x0020, pipe_phya2 + (0x15 << 2));
    dev_dbg(phy->dev, "%s, pipe_phya2\n", __func__);
}

void sstar_phy_pipe_rxpll_en_d2s(struct phy *phy)
{
    struct udevice *       dev        = phy->dev;
    struct sstar_phy_pipe *sstar_phy  = dev_get_priv(dev);
    void __iomem *         pipe_phya2 = sstar_phy->reg + (0x200 * 3);

    writew(0x000f, pipe_phya2 + (0x21 << 2));
    dev_dbg(phy->dev, "%s, pipe_phya2\n", __func__);
}

void sstar_phy_pipe_rxpll_ctrk_r2(struct phy *phy)
{
    struct udevice *       dev        = phy->dev;
    struct sstar_phy_pipe *sstar_phy  = dev_get_priv(dev);
    void __iomem *         pipe_phya2 = sstar_phy->reg + (0x200 * 3);

    writew(0x0000, pipe_phya2 + (0x08 << 2));
    dev_dbg(phy->dev, "%s, pipe_phya2\n", __func__);
}

void sstar_phy_pipe_rxpll_d2s_ldo_ref(struct phy *phy)
{
    struct udevice *       dev        = phy->dev;
    struct sstar_phy_pipe *sstar_phy  = dev_get_priv(dev);
    void __iomem *         pipe_phya2 = sstar_phy->reg + (0x200 * 3);

    if (ofnode_device_is_compatible(dev_ofnode(dev), "sstar,generic-pipe"))
        writew(0x0060, pipe_phya2 + (0x06 << 2));
    else
        writew(0x6060, pipe_phya2 + (0x06 << 2));
    dev_dbg(phy->dev, "%s, pipe_phya2\n", __func__);
}

void sstar_phy_pipe_rxpll_vote_sel(struct phy *phy)
{
    struct udevice *       dev        = phy->dev;
    struct sstar_phy_pipe *sstar_phy  = dev_get_priv(dev);
    void __iomem *         pipe_phya2 = sstar_phy->reg + (0x200 * 3);

    writew(0x0200, pipe_phya2 + (0x05 << 2));
    dev_dbg(phy->dev, "%s\n", __func__);
}
void sstar_phy_pipe_rx_ictrl(struct phy *phy)
{
    struct udevice *       dev        = phy->dev;
    struct sstar_phy_pipe *sstar_phy  = dev_get_priv(dev);
    void __iomem *         pipe_phya0 = sstar_phy->reg + (0x200 * 1);

    writew(0x0002, pipe_phya0 + (0x09 << 2));
    dev_dbg(phy->dev, "%s, pipe_phya0\n", __func__);
}

void sstar_phy_pipe_rx_div_setting(struct phy *phy)
{
    struct udevice *       dev        = phy->dev;
    struct sstar_phy_pipe *sstar_phy  = dev_get_priv(dev);
    void __iomem *         pipe_phya0 = sstar_phy->reg + (0x200 * 1);
    // writew(base + PHYA0_REG_SATA_RXPLL, 0x100e); // rxoll_lop_div_first
    writew(0x1009, pipe_phya0 + (0x30 << 2));
    writew(0x0005, pipe_phya0 + (0x31 << 2)); // rxpll_loop_div_second
    writew(0x0505, pipe_phya0 + (0x33 << 2));
    dev_dbg(phy->dev, "%s, pipe_phya0\n", __func__);
}

void sstar_phy_pipe_rxpll_ictrl_CDR(struct phy *phy)
{
    struct udevice *       dev        = phy->dev;
    struct sstar_phy_pipe *sstar_phy  = dev_get_priv(dev);
    void __iomem *         pipe_phya2 = sstar_phy->reg + (0x200 * 3);

    writew(0x0001, pipe_phya2 + (0x09 << 2));
    dev_dbg(phy->dev, "%s\n", __func__);
}

void sstar_phy_pipe_rxpll_vco_ldo_voltage(struct phy *phy)
{
    struct udevice *       dev        = phy->dev;
    struct sstar_phy_pipe *sstar_phy  = dev_get_priv(dev);
    void __iomem *         pipe_phya2 = sstar_phy->reg + (0x200 * 3);

    writew(0x6060, pipe_phya2 + (0x06 << 2));
    dev_dbg(phy->dev, "%s\n", __func__);
}

void sstar_phy_pipe_ssc(struct phy *phy)
{
    struct udevice *       dev       = phy->dev;
    struct sstar_phy_pipe *sstar_phy = dev_get_priv(dev);
    void __iomem *         pipe_phya0;

    // U3 S(spread)S(spectrum)C(clock) setting
    dev_dbg(phy->dev, "%s SSC\n", __func__);

    pipe_phya0 = sstar_phy->reg + (0x200 * 1);

    writew(0x55ff, pipe_phya0 + (0x40 << 2));
    writew(0x002c, pipe_phya0 + (0x41 << 2));
    writew(0x000a, pipe_phya0 + (0x42 << 2));
    writew(0x0271, pipe_phya0 + (0x43 << 2));
    writew(0x0001, pipe_phya0 + (0x44 << 2));

    dev_dbg(phy->dev, "%s, U3 SSC\n", __func__);
}

void sstar_phy_pipe_tx_swing_setting(struct phy *phy)
{
    struct udevice *       dev       = phy->dev;
    struct sstar_phy_pipe *sstar_phy = dev_get_priv(dev);
    void __iomem *         pipe_phyd = sstar_phy->reg;

    sstar_phy_pipe_tx_swing_and_de_emphasis(phy);
    writew(0x241d, pipe_phyd + (0x2f << 2));
    /*
        setbits_16(pipe_phyd + (0x0b << 2), 0x0050);

        clrbits_16(pipe_phyd + (0x2a << 2), 0x1F << 7);
        setbits_16(pipe_phyd + (0x2a << 2), 0x10 << 7);
    */
    dev_dbg(phy->dev, "%s, pipe_phyd\n", __func__);
}
/*
void sstar_phy_pipe_tx_ibiasi(struct sstar_phy_phy *port)
{
    void __iomem *pipe_phya2 = phy->reg + (0x200 * 3);
    OUTREG8(pipe_phya2 + ((0x11 << 2) + 1), 0x70);
    dev_dbg(phy->dev, "%s\n", __func__);
}
*/
void sstar_phy_pipe_ibias_trim(struct phy *phy)
{
    struct udevice *       dev       = phy->dev;
    struct sstar_phy_pipe *sstar_phy = dev_get_priv(dev);
    void __iomem *         pipe_phyd = sstar_phy->reg;
    writew(0x2802, pipe_phyd + (0x46 << 2));
    writew(0x0300, pipe_phyd + (0x50 << 2));
    dev_dbg(phy->dev, "%s, pipe_phyd\n", __func__);
}

void sstar_phy_pipe_rx_imp_sel(struct phy *phy)
{
    struct udevice *       dev       = phy->dev;
    struct sstar_phy_pipe *sstar_phy = dev_get_priv(dev);
    void __iomem *         pipe_phyd = sstar_phy->reg;
    writew(0x8904, pipe_phyd + (0x0e << 2));
    writew(0x0100, pipe_phyd + (0x3c << 2));
    dev_dbg(phy->dev, "%s, pipe_phyd\n", __func__);
}

void sstar_phy_pipe_set_phya1(struct phy *phy)
{
    struct udevice *       dev        = phy->dev;
    struct sstar_phy_pipe *sstar_phy  = dev_get_priv(dev);
    void __iomem *         pipe_phya1 = sstar_phy->reg + (0x200 * 2);

    setbits_16(pipe_phya1 + 0x20 * 4, 0x04); // guyo
    writew(0, pipe_phya1 + 0x25 * 4);

    clrbits_16(pipe_phya1 + 0x49 * 4, 0x200);
    setbits_16(pipe_phya1 + 0x49 * 4, 0xc4e); // reg_rx_lfps_t_burst_gap = 3150

    if (ofnode_device_is_compatible(dev_ofnode(dev), "sstar,generic-pipe"))
    {
        clrbits_16(pipe_phya1 + 0x51 * 4, 0x001f);
        setbits_16(pipe_phya1 + 0x51 * 4, 0x0004);
    }
    dev_dbg(phy->dev, "%s, pipe_phya1\n", __func__);
}

void sstar_phy_pipe_eco_enabled(struct phy *phy)
{
    struct udevice *       dev        = phy->dev;
    struct sstar_phy_pipe *sstar_phy  = dev_get_priv(dev);
    void __iomem *         pipe_phya0 = sstar_phy->reg + (0x200 * 1);

    // Enable ECO
    clrbits_16(pipe_phya0 + 0x03 * 4, 0x0f);
    setbits_16(pipe_phya0 + 0x03 * 4, 0x0d);
    dev_dbg(phy->dev, "%s, pipe_phya0\n", __func__);
}

static int sstar_phy_pipe_init(struct phy *phy)
{
    struct udevice *dev = phy->dev;
    ;
    struct sstar_phy_pipe *sstar_phy = dev_get_priv(dev);

    void __iomem *pipe_phyd, *pipe_phya0, *pipe_phya1, *pipe_phya2;

    pipe_phyd  = sstar_phy->reg;
    pipe_phya0 = pipe_phyd + (0x200 * 1);
    pipe_phya1 = pipe_phyd + (0x200 * 2);
    pipe_phya2 = pipe_phyd + (0x200 * 3);

    setbits_16(pipe_phya2 + (0x1f << 2), BIT(15));
    // sstar_phy_pipe_deassert_hw_reset(phy);
    sstar_phy_pipe_sw_reset(phy);
    sstar_phy_pipe_synthesis_enabled(phy);

    if (ofnode_device_is_compatible(dev_ofnode(dev), "sstar,infinity7-pipe"))
    {
        /* EQ default */
        clrbits_16(pipe_phya1 + (0x51 << 2), (BIT(5) - 1));
        setbits_16(pipe_phya1 + (0x51 << 2), BIT(4));
    }

    sstar_phy_pipe_eco_enabled(phy);
    sstar_phy_pipe_tx_polarity_inv_disabled(phy);
    sstar_phy_pipe_tx_loop_div(phy);
    sstar_pipe_tx_post_div_and_test(phy);
    sstar_phy_pipe_rx_div_setting(phy);
    sstar_phy_pipe_rxpll_ictrl_CDR(phy);
    sstar_phy_pipe_rxpll_vco_ldo_voltage(phy);

    sstar_phy_pipe_EQ_rstep(phy);

    sstar_phy_pipe_DFE(phy);

    sstar_phy_pipe_rxpll_d2s_ldo_ref(phy);

    sstar_phy_pipe_rxpll_vote_sel(phy);

    sstar_phy_pipe_rx_ictrl(phy);

    sstar_phy_pipe_txpll_vco_ldo(phy);

    sstar_phy_pipe_txpll_en_d2s(phy);

    // mdelay(1);
    sstar_phy_pipe_txpll_en_vco(phy);

    // mdelay(1);
    sstar_phy_pipe_rxpll_en_d2s(phy);

    // mdelay(1);
    sstar_phy_pipe_txpll_ictrl_new(phy);

    sstar_phy_pipe_rxpll_ctrk_r2(phy);

    sstar_phy_pipe_tx_swing_setting(phy);

    // sstar_phy_pipe_tx_ibiasi(phy);

    sstar_sata_rx_tx_pll_frq_det_enabled(phy);

    sstar_phy_pipe_set_phya1(phy);
    setbits_16(pipe_phya0 + (0x58 * 4), 0x0003);
    /* for I7U02 LFPS setting*/
    // clrbits_16(pipe_phya2 + (0x2f << 2), BIT(8) | BIT(7) | BIT(6) | BIT(5) | BIT(4) | BIT(3));
    // setbits_16(pipe_phya2 + (0x2f << 2), 0x28 << 3);
    sstar_phy_pipe_LPFS_det_hw_mode(phy);
    dev_dbg(phy->dev, "%s\n", __func__);
    return 0;
}

static int sstar_phy_pipe_exit(struct phy *phy)
{
    dev_dbg(phy->dev, "%s\n", __func__);
    return 0;
}

static int sstar_phy_pipe_reset(struct phy *phy)
{
    sstar_phy_pipe_assert_sw_reset(phy);
    udelay(10);
    sstar_phy_pipe_deassert_sw_reset(phy);
    dev_dbg(phy->dev, "%s, pipe_phya1\n", __func__);
    return 0;
}

static int sstar_phy_pipe_power_on(struct phy *phy)
{
    struct udevice *dev = phy->dev;
    ;
    struct sstar_phy_pipe *sstar_phy = dev_get_priv(dev);

    void __iomem *pipe_phya0 = sstar_phy->reg + (0x200 * 1);
    void __iomem *pipe_phya2 = sstar_phy->reg + (0x200 * 3);

    sstar_pipe_show_trim_value(phy);
    sstar_phy_pipe_deassert_sw_reset(phy);

    setbits_16(pipe_phya2 + (0x16 << 2), BIT(14)); // txpll en
    setbits_16(pipe_phya0 + (0x15 << 2), BIT(6));  // rxpll en

    writew(0x0100, pipe_phya0 + (0x20 << 2));
    writew(0x100e, pipe_phya0 + (0x30 << 2));

    // sstar_phy_pipe_LPFS_det_power_on(phy);
    //  sstar_phy_pipe_recal(phy);
    dev_dbg(phy->dev, "%s end\n", __func__);
    return 0;
}

static int sstar_phy_pipe_power_off(struct phy *phy)
{
    struct udevice *dev = phy->dev;
    ;
    struct sstar_phy_pipe *sstar_phy = dev_get_priv(dev);

    void __iomem *pipe_phya0, *pipe_phya2;
    pipe_phya0 = sstar_phy->reg + (0x200 * 1);
    pipe_phya2 = sstar_phy->reg + (0x200 * 3);
    // base = re->pipe_phyd;
    // setbits_16(base + PHYD_REG_RG_TX_POLARTY_INV, BIT_TX_POLARTY_INV_EN);

    setbits_16(pipe_phya0 + (0x20 << 2), BIT(0));
    setbits_16(pipe_phya0 + (0x30 << 2), BIT(0));

    clrbits_16(pipe_phya2 + (0x1C << 2), 0x0F);
    clrbits_16(pipe_phya2 + (0x1B << 2), BIT(8));
    clrbits_16(pipe_phya2 + (0x21 << 2), 0x0F);

    dev_dbg(phy->dev, "%s\n", __func__);
    return 0;
}

const struct phy_ops sstar_phy_pipe_ops = {
    .init      = sstar_phy_pipe_init,
    .exit      = sstar_phy_pipe_exit,
    .power_on  = sstar_phy_pipe_power_on,
    .power_off = sstar_phy_pipe_power_off,
    .reset     = sstar_phy_pipe_reset,
};

static int sstar_phy_pipe_of_to_plat(struct udevice *dev)
{
    struct sstar_phy_pipe *phy = dev_get_priv(dev);

    phy->reg = devfdt_remap_addr(dev);

    if (NULL == phy->reg)
    {
        return 0;
    }
    return 0;
}

static int sstar_phy_pipe_probe(struct udevice *dev)
{
    struct sstar_phy_pipe *phy = dev_get_priv(dev);
    int                    ret;
    dev_dbg(dev, "%s start.\r\n", __func__);

    ret = clk_get_bulk(dev, &phy->clks);
    if (ret == -ENOSYS || ret == -ENOENT)
        goto out;
    if (ret)
        return ret;

#if CONFIG_IS_ENABLED(SSTAR_CLK)
    dev_dbg(dev, "%s clk_enable_bulk\r\n", __func__);
    ret = clk_enable_bulk(&phy->clks);
    if (ret)
    {
        clk_release_bulk(&phy->clks);
        return ret;
    }
#endif
out:
    if (ret)
    {
        /*turn on CLK_ssusb_phy_108, CLK_ssusb_phy_432*/
        setbits_16(phys_to_virt(0x1f207000 + (0x6f << 2)), BIT(0) | BIT(4));
        // clrbits_16(phys_to_virt(0x1f207000 + (0x6f << 2)), BIT(2) | BIT(3) | BIT(6) | BIT(7));
        clrbits_16(phys_to_virt(0x1f207000 + (0x6f << 2)), BIT(0) | BIT(4));
    }

    dev_dbg(dev, "%s end.\r\n", __func__);
    return 0;
}

static int sstar_phy_pipe_remove(struct udevice *dev)
{
#if CONFIG_IS_ENABLED(CLK)
    struct sstar_phy_pipe *phy = dev_get_priv(dev);

    clk_release_bulk(&phy->clks);
#endif
    return 0;
}

static const struct udevice_id sstar_phy_pipe_ids[] = {
    {.compatible = "sstar,infinity7-pipe"}, {.compatible = "sstar,generic-pipe"}, {}};

U_BOOT_DRIVER(sstar_phy_pipe) = {
    .name       = "sstar-phy-pipe",
    .id         = UCLASS_PHY,
    .of_match   = sstar_phy_pipe_ids,
    .of_to_plat = sstar_phy_pipe_of_to_plat,
    .ops        = &sstar_phy_pipe_ops,
    .probe      = sstar_phy_pipe_probe,
    .remove     = sstar_phy_pipe_remove,
    .priv_auto  = sizeof(struct sstar_phy_pipe),
};
