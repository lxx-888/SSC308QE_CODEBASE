/*
 * hal_clk.c- Sigmastar
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
#include "hal_clk.h"
#include "sstar_clk.h"

static const char* const pwm_parents[] = {
    "CLK_xtali_12m",       "CLK_xtali_12m_div2", "CLK_xtali_12m_div4", "CLK_xtali_12m_div8",
    "CLK_xtali_12m_div16", "CLK_mpll_86m",       "CLK_void",           "CLK_void"};

static const char* const gmac0_parents[] = {"CLK_gmacpll_250_div2", "CLK_gmacpll_25a", "CLK_gmacpll_250_div100",
                                            "CLK_gmac0_external_ref_buf"};

static const char* const gmac1_parents[] = {"CLK_gmacpll_250_div2", "CLK_gmacpll_25b", "CLK_gmacpll_250_div100",
                                            "CLK_gmac1_external_ref_buf"};

static const char* const sof_usb30_drd[] = {"CLK_xtali_24m", "CLK_upll_384m_div8", "CLK_mpll_432m_div8",
                                            "CLK_mpll_432m_div4"};

static const char* const ssusb_phy_108[] = {"CLK_mpll_432m_div4", "CLK_void", "CLK_void", "CLK_void"};

static const char* const ssusb_phy_432[] = {"CLK_mpll_432m", "CLK_void", "CLK_void", "CLK_void"};

static const char* const ssusb_axi[] = {"CLK_mpll_288m", "CLK_mpll_216m", "CLK_mpll_172m", "CLK_mpll_432m_div4"};

static const char* const miic_parents[] = {"CLK_mpll_288m_div4", "CLK_mpll_432m_div8", "CLK_xtali_12m", "CLK_void"};

static const char* const uart_parents[] = {"CLK_mpll_345m_div2", "CLK_mpll_288m_div2", "CLK_xtali_24m",
                                           "CLK_xtali_12m"};

static const char* const fuart_parents[] = {"CLK_mpll_345m_div2", "CLK_mpll_288m_div2", "CLK_xtali_12m",
                                            "CLK_fuart0_synth"};

static const char* const mspi_parents[] = {"CLK_mpll_432m_div4", "CLK_mpll_432m_div8", "CLK_xtali_12m",
                                           "CLK_mpll_288m_div2"};

static const char* const gphy0_parents[] = {"CLK_gmacpll_250_div10", "CLK_gmacpll_250_div5", "CLK_void", "CLK_void"};

static const char* const gphy1_parents[] = {"CLK_gmacpll_250_div10", "CLK_gmacpll_250_div5", "CLK_void", "CLK_void"};

const sstar_clk_data clk_list[] = {
    /* fixed rate clocks */
    SSTAR_CLK_FIXED_RATE(SSTAR_CLK_DUMMY, "CLK_void", 0),
    SSTAR_CLK_FIXED_RATE(SSTAR_CLK_XTAL_12M, "CLK_xtali_12m", 12000000),
    SSTAR_CLK_FIXED_RATE(SSTAR_CLK_XTAL_24M, "CLK_xtali_24m", 24000000),
    SSTAR_CLK_FIXED_RATE(SSTAR_CLK_MPLL_86M, "CLK_mpll_86m", 86400000),
    SSTAR_CLK_FIXED_RATE(SSTAR_CLK_GMACPLL, "CLK_gmacpll_250", 250000000),
    SSTAR_CLK_FIXED_RATE(SSTAR_CLK_GMAC_25A, "CLK_gmacpll_25a", 25000000),
    SSTAR_CLK_FIXED_RATE(SSTAR_CLK_GMAC_25B, "CLK_gmacpll_25b", 25000000),
    SSTAR_CLK_FIXED_RATE(SSTAR_CLK_GMAC0_EXT100M, "CLK_gmac0_external_ref_buf", 100000000),
    SSTAR_CLK_FIXED_RATE(SSTAR_CLK_GMAC1_EXT100M, "CLK_gmac1_external_ref_buf", 100000000),
    SSTAR_CLK_FIXED_RATE(SSTAR_CLK_MPLL_432M, "CLK_mpll_432m", 432000000),
    SSTAR_CLK_FIXED_RATE(SSTAR_CLK_MPLL_288M, "CLK_mpll_288m", 288000000),
    SSTAR_CLK_FIXED_RATE(SSTAR_CLK_MPLL_216M, "CLK_mpll_216m", 216000000),
    SSTAR_CLK_FIXED_RATE(SSTAR_CLK_MPLL_172M, "CLK_mpll_172m", 172000000),
    SSTAR_CLK_FIXED_RATE(SSTAR_CLK_UPLL_384M, "CLK_upll_384m", 384000000),
    SSTAR_CLK_FIXED_RATE(SSTAR_CLK_MPLL_345M, "CLK_mpll_345m", 345000000),
    /* fixed factor clocks */
    SSTAR_CLK_FIXED_FACTOR(SSTAR_CLK_XTAL_12M_DIV2, "CLK_xtali_12m_div2", "CLK_xtali_12m", 1, 2),
    SSTAR_CLK_FIXED_FACTOR(SSTAR_CLK_XTAL_12M_DIV4, "CLK_xtali_12m_div4", "CLK_xtali_12m", 1, 4),
    SSTAR_CLK_FIXED_FACTOR(SSTAR_CLK_XTAL_12M_DIV8, "CLK_xtali_12m_div8", "CLK_xtali_12m", 1, 8),
    SSTAR_CLK_FIXED_FACTOR(SSTAR_CLK_XTAL_12M_DIV16, "CLK_xtali_12m_div16", "CLK_xtali_12m", 1, 16),
    SSTAR_CLK_FIXED_FACTOR(SSTAR_CLK_GMACPLL_DIV2, "CLK_gmacpll_250_div2", "CLK_gmacpll_250", 1, 2),
    SSTAR_CLK_FIXED_FACTOR(SSTAR_CLK_GMACPLL_DIV5, "CLK_gmacpll_250_div5", "CLK_gmacpll_250", 1, 5),
    SSTAR_CLK_FIXED_FACTOR(SSTAR_CLK_GMACPLL_DIV10, "CLK_gmacpll_250_div10", "CLK_gmacpll_250", 1, 10),
    SSTAR_CLK_FIXED_FACTOR(SSTAR_CLK_GMACPLL_DIV100, "CLK_gmacpll_250_div100", "CLK_gmacpll_250", 1, 100),
    SSTAR_CLK_FIXED_FACTOR(SSTAR_CLK_MPLL_432M_DIV4, "CLK_mpll_432m_div4", "CLK_mpll_432m", 1, 4),
    SSTAR_CLK_FIXED_FACTOR(SSTAR_CLK_MPLL_432M_DIV8, "CLK_mpll_432m_div8", "CLK_mpll_432m", 1, 8),
    SSTAR_CLK_FIXED_FACTOR(SSTAR_CLK_UPLL_384M_DIV8, "CLK_upll_384m_div8", "CLK_upll_384m", 1, 8),
    SSTAR_CLK_FIXED_FACTOR(SSTAR_CLK_MPLL_432M_DIV2, "CLK_mpll_432m_div2", "CLK_mpll_432m", 1, 2),
    SSTAR_CLK_FIXED_FACTOR(SSTAR_CLK_MPLL_432M_DIV16, "CLK_mpll_432m_div16", "CLK_mpll_432m", 1, 16),
    SSTAR_CLK_FIXED_FACTOR(SSTAR_CLK_MPLL_345M_DIV2, "CLK_mpll_345m_div2", "CLK_mpll_345m", 1, 2),
    SSTAR_CLK_FIXED_FACTOR(SSTAR_CLK_MPLL_288M_DIV2, "CLK_mpll_288m_div2", "CLK_mpll_288m", 1, 2),
    SSTAR_CLK_FIXED_FACTOR(SSTAR_CLK_MPLL_288M_DIV4, "CLK_mpll_288m_div4", "CLK_mpll_288m", 1, 4),
    SSTAR_CLK_FIXED_FACTOR(SSTAR_CLK_MPLL_288M_DIV8, "CLK_mpll_288m_div8", "CLK_mpll_288m", 1, 8),
    /* composite clocks */
    SSTAR_CLK_COMPOSITE(SSTAR_CLK_PWM, "CLK_pwm", pwm_parents, ARRAY_SIZE(pwm_parents), REG_CKG_PWM_BASE,
                        REG_CKG_PWM_OFFSET + 2, 3, REG_CKG_PWM_OFFSET, 0),
    SSTAR_CLK_COMPOSITE(SSTAR_CLK_GMAC0_GMII, "CLK_gmac0_gmii", gmac0_parents, ARRAY_SIZE(gmac0_parents),
                        REG_CKG_GMAC0_GMII_BASE, REG_CKG_GMAC0_GMII_OFFSET + 2, 2, REG_CKG_GMAC0_GMII_OFFSET, 0),
    SSTAR_CLK_COMPOSITE(SSTAR_CLK_GMAC1_GMII, "CLK_gmac1_gmii", gmac1_parents, ARRAY_SIZE(gmac1_parents),
                        REG_CKG_GMAC1_GMII_BASE, REG_CKG_GMAC1_GMII_OFFSET + 2, 2, REG_CKG_GMAC1_GMII_OFFSET, 0),
    SSTAR_CLK_COMPOSITE(SSTAR_CLK_SSUSB_PHY_108, "CLK_ssusb_phy_108", ssusb_phy_108, ARRAY_SIZE(ssusb_phy_108),
                        REG_CKG_SSUSB_PHY_108_BASE, REG_CKG_SSUSB_PHY_108_OFFSET + 2, 2, REG_CKG_SSUSB_PHY_108_OFFSET,
                        0),
    SSTAR_CLK_COMPOSITE(SSTAR_CLK_SSUSB_PHY_432, "CLK_ssusb_phy_432", ssusb_phy_432, ARRAY_SIZE(ssusb_phy_432),
                        REG_CKG_SSUSB_PHY_432_BASE, REG_CKG_SSUSB_PHY_432_OFFSET + 2, 2, REG_CKG_SSUSB_PHY_432_OFFSET,
                        0),
    SSTAR_CLK_COMPOSITE(SSTAR_CLK_SOF_USB30_DRD, "CLK_sof_usb30_drd", sof_usb30_drd, ARRAY_SIZE(sof_usb30_drd),
                        REG_CKG_SOF_USB30_DRD_BASE, REG_CKG_SOF_USB30_DRD_OFFSET + 2, 2, REG_CKG_SOF_USB30_DRD_OFFSET,
                        0),
    SSTAR_CLK_COMPOSITE(SSTAR_CLK_SSUSB_AXI, "CLK_ssusb_axi", ssusb_axi, ARRAY_SIZE(ssusb_axi), REG_CKG_SSUSB_AXI_BASE,
                        REG_CKG_SSUSB_AXI_OFFSET + 2, 2, REG_CKG_SSUSB_AXI_OFFSET, 0),
    SSTAR_CLK_COMPOSITE(SSTAR_CLK_MIIC0, "CLK_miic0", miic_parents, ARRAY_SIZE(miic_parents), REG_CKG_MIIC0_BASE,
                        REG_CKG_MIIC0_OFFSET + 2, 2, REG_CKG_MIIC0_OFFSET, 0),
    SSTAR_CLK_COMPOSITE(SSTAR_CLK_MIIC1, "CLK_miic1", miic_parents, ARRAY_SIZE(miic_parents), REG_CKG_MIIC1_BASE,
                        REG_CKG_MIIC1_OFFSET + 2, 2, REG_CKG_MIIC1_OFFSET, 0),
    SSTAR_CLK_COMPOSITE(SSTAR_CLK_MIIC2, "CLK_miic2", miic_parents, ARRAY_SIZE(miic_parents), REG_CKG_MIIC2_BASE,
                        REG_CKG_MIIC2_OFFSET + 2, 2, REG_CKG_MIIC2_OFFSET, 0),
    SSTAR_CLK_COMPOSITE(SSTAR_CLK_MIIC3, "CLK_miic3", miic_parents, ARRAY_SIZE(miic_parents), REG_CKG_MIIC3_BASE,
                        REG_CKG_MIIC3_OFFSET + 2, 2, REG_CKG_MIIC3_OFFSET, 0),
    SSTAR_CLK_COMPOSITE(SSTAR_CLK_UART0, "CLK_uart0", uart_parents, ARRAY_SIZE(uart_parents), REG_CKG_FUART0_BASE,
                        REG_CKG_FUART0_OFFSET + 2, 2, REG_CKG_FUART0_OFFSET, 0),
    SSTAR_CLK_COMPOSITE(SSTAR_CLK_UART1, "CLK_uart1", uart_parents, ARRAY_SIZE(uart_parents), REG_CKG_FUART1_BASE,
                        REG_CKG_FUART1_OFFSET + 2, 2, REG_CKG_FUART1_OFFSET, 0),
    SSTAR_CLK_COMPOSITE(SSTAR_CLK_UART2, "CLK_uart2", uart_parents, ARRAY_SIZE(uart_parents), REG_CKG_FUART2_BASE,
                        REG_CKG_FUART2_OFFSET + 2, 2, REG_CKG_FUART2_OFFSET, 0),
    SSTAR_CLK_COMPOSITE(SSTAR_CLK_FUART, "CLK_fuart", fuart_parents, ARRAY_SIZE(fuart_parents), REG_CKG_FUART_BASE,
                        REG_CKG_FUART_OFFSET + 2, 2, REG_CKG_FUART_OFFSET, 0),
    SSTAR_CLK_COMPOSITE(SSTAR_CLK_MSPI0, "CLK_mspi0", mspi_parents, ARRAY_SIZE(mspi_parents), REG_CKG_MSPI0_BASE,
                        REG_CKG_MSPI0_OFFSET + 2, 2, REG_CKG_MSPI0_OFFSET, 0),
    SSTAR_CLK_COMPOSITE(SSTAR_CLK_GPHY0_REF, "CLK_gphy0_gmii", gphy0_parents, ARRAY_SIZE(gphy0_parents),
                        REG_CKG_GPHY0_REF_BASE, REG_CKG_GPHY0_REF_OFFSET + 2, 2, REG_CKG_GPHY0_REF_OFFSET, 0),
    SSTAR_CLK_COMPOSITE(SSTAR_CLK_GPHY1_REF, "CLK_gphy1_gmii", gphy1_parents, ARRAY_SIZE(gphy1_parents),
                        REG_CKG_GPHY1_REF_BASE, REG_CKG_GPHY1_REF_OFFSET + 2, 2, REG_CKG_GPHY1_REF_OFFSET, 0),
};

const sstar_clk_data* hal_clk_data_list(int* count)
{
    if (count)
        *count = ARRAY_SIZE(clk_list);

    debug("sstar clocks count %d\r\n", *count);
    return clk_list;
}
