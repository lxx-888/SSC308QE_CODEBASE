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
    "CLK_xtali_12m_div16", "CLK_mpll_86m",       "CLK_xtali_24m",      "CLK_void"};

static const char* const miic_parents[] = {"CLK_mpll_288m_div4", "CLK_mpll_432m_div8", "CLK_xtali_12m", "CLK_void"};

static const char* const mspi_parents[] = {"CLK_mpll_432m_div4", "CLK_mpll_288m_div2", "CLK_xtali_12m",
                                           "CLK_spi_synth_pll"};

static const char* const pm_pwm_parents[] = {"CLK_12m_mux",          "CLK_24m_mux",      "CLK_12m_mux_div2",
                                             "CLK_12m_mux_div4",     "CLK_12m_mux_div8", "CLK_rtcpll_clk_div16",
                                             "CLK_rtcpll_clk_div96", "CLK_VOID"};

static const char* const pm_miic0_parents[] = {"CLK_24m_mux", "CLK_VOID", "CLK_VOID", "CLK_VOID"};

static const char* const sar_parents[] = {"CLK_12m_mux", "CLK_VOID", "CLK_VOID", "CLK_VOID"};

static const char* const wdt_parents[] = {"CLK_xtali_12m", "CLK_24m_mux"};

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
    SSTAR_CLK_FIXED_RATE(SSTAR_CLK_SPI_SYNTH_PLL, "CLK_spi_synth_pll", 104000000),
    SSTAR_CLK_FIXED_RATE(SSTAR_CLK_12M_MUX, "CLK_12m_mux", 12000000),
    SSTAR_CLK_FIXED_RATE(SSTAR_CLK_24M_MUX, "CLK_24m_mux", 24000000),
    SSTAR_CLK_FIXED_RATE(SSTAR_CLK_RTCPLL_CLK, "CLK_rtcpll_clk", 196600000),
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
    SSTAR_CLK_FIXED_FACTOR(SSTAR_CLK_12M_MUX_DIV2, "CLK_12m_mux_div2", "CLK_12m_mux", 1, 2),
    SSTAR_CLK_FIXED_FACTOR(SSTAR_CLK_12M_MUX_DIV4, "CLK_12m_mux_div4", "CLK_12m_mux", 1, 4),
    SSTAR_CLK_FIXED_FACTOR(SSTAR_CLK_12M_MUX_DIV8, "CLK_12m_mux_div8", "CLK_12m_mux", 1, 8),
    SSTAR_CLK_FIXED_FACTOR(SSTAR_CLK_RTCPLL_CLK_DIV16, "CLK_rtcpll_clk_div16", "CLK_rtcpll_clk", 1, 16),
    SSTAR_CLK_FIXED_FACTOR(SSTAR_CLK_RTCPLL_CLK_DIV96, "CLK_rtcpll_clk_div96", "CLK_rtcpll_clk", 1, 96),
    /* composite clocks */
    SSTAR_CLK_COMPOSITE(SSTAR_CLK_PWM, "CLK_pwm", pwm_parents, ARRAY_SIZE(pwm_parents), REG_CKG_PWM_BASE,
                        REG_CKG_PWM_OFFSET + 2, 3, REG_CKG_PWM_OFFSET, 0),
    SSTAR_CLK_COMPOSITE(SSTAR_CLK_MIIC0, "CLK_miic0", miic_parents, ARRAY_SIZE(miic_parents), REG_CKG_MIIC0_BASE,
                        REG_CKG_MIIC0_OFFSET + 2, 2, REG_CKG_MIIC0_OFFSET, 0),
    SSTAR_CLK_COMPOSITE(SSTAR_CLK_MIIC1, "CLK_miic1", miic_parents, ARRAY_SIZE(miic_parents), REG_CKG_MIIC1_BASE,
                        REG_CKG_MIIC1_OFFSET + 2, 2, REG_CKG_MIIC1_OFFSET, 0),
    SSTAR_CLK_COMPOSITE(SSTAR_CLK_MIIC2, "CLK_miic2", miic_parents, ARRAY_SIZE(miic_parents), REG_CKG_MIIC2_BASE,
                        REG_CKG_MIIC2_OFFSET + 2, 2, REG_CKG_MIIC2_OFFSET, 0),
    SSTAR_CLK_COMPOSITE(SSTAR_CLK_MSPI0, "CLK_mspi0", mspi_parents, ARRAY_SIZE(mspi_parents), REG_CKG_MSPI0_BASE,
                        REG_CKG_MSPI0_OFFSET + 2, 3, REG_CKG_MSPI0_OFFSET, 0),
    SSTAR_CLK_COMPOSITE(SSTAR_CLK_PM_PWM, "CLK_pm_pwm", pm_pwm_parents, ARRAY_SIZE(pm_pwm_parents), REG_CKG_PM_PWM_BASE,
                        REG_CKG_PM_PWM_OFFSET + 2, 3, REG_CKG_PM_PWM_OFFSET, 0),
    SSTAR_CLK_COMPOSITE(SSTAR_CLK_PM_MIIC0, "CLK_pm_miic0", pm_miic0_parents, ARRAY_SIZE(pm_miic0_parents),
                        REG_CKG_PM_MIIC0_BASE, REG_CKG_PM_MIIC0_OFFSET + 2, 2, REG_CKG_PM_MIIC0_OFFSET, 0),
    SSTAR_CLK_COMPOSITE(SSTAR_CLK_SAR, "CLK_sar", sar_parents, ARRAY_SIZE(sar_parents), REG_CKG_SAR_BASE,
                        REG_CKG_SAR_OFFSET + 2, 2, REG_CKG_SAR_OFFSET, 0),
    SSTAR_CLK_COMPOSITE(SSTAR_CLK_WDT, "CLK_wdt", wdt_parents, ARRAY_SIZE(wdt_parents), REG_CKG_WDT_BASE,
                        REG_CKG_WDT_OFFSET + 3, 2, REG_CKG_WDT_OFFSET, 0),
};

const sstar_clk_data* hal_clk_data_list(int* count)
{
    if (count)
        *count = ARRAY_SIZE(clk_list);

    debug("sstar clocks count %d\r\n", *count);
    return clk_list;
}
