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

static const char* const uart_parents[] = {"CLK_mpll_345m_div2", "CLK_mpll_288m_div2", "CLK_xtali_24m",
                                           "CLK_xtali_12m"};

static const char* const fuart0_synth_parents[] = {"CLK_mpll_432m", "CLK_mpll_432m_div2"};

static const char* const fuart_parents[] = {"CLK_mpll_345m_div2", "CLK_mpll_288m_div2", "CLK_xtali_12m",
                                            "CLK_fuart0_synth"};

static const char* const gmac0_parents[] = {"CLK_gmacpll_250_div2", "CLK_gmacpll_250_div10", "CLK_gmacpll_250_div100",
                                            "CLK_gmac0_external_ref_buf"};

static const char* const gmac1_parents[] = {"CLK_gmacpll_250_div2", "CLK_gmacpll_250_div10", "CLK_gmacpll_250_div100",
                                            "CLK_gmac1_external_ref_buf"};

static const char* const ipuff_parents[] = {"CLK_mpll_432m", "CLK_upll_384m", "CLK_upll_320m", "CLK_upll_480m_div2"};

static const char* const pcie_parents[] = {"CLK_mpll_432m", "CLK_upll_384m", "CLK_mpll_345m", "CLK_mpll_288m"};

static const char* const mspi_parents[] = {"CLK_mpll_432m_div4", "CLK_mpll_432m_div8", "CLK_xtali_12m",
                                           "CLK_mpll_288m_div2"};

static const char* const miic_parents[] = {"CLK_mpll_288m_div4", "CLK_mpll_432m_div8", "CLK_xtali_12m", "CLK_void"};

static const char* const pwm_parents[] = {
    "CLK_xtali_12m",       "CLK_xtali_12m_div2", "CLK_xtali_12m_div4", "CLK_xtali_12m_div8",
    "CLK_xtali_12m_div16", "CLK_mpll_86m",       "CLK_void",           "CLK_void"};

const sstar_clk_data clk_list[] = {
    /* fixed rate clocks */
    SSTAR_CLK_FIXED_RATE(SSTAR_CLK_DUMMY, "CLK_void", 0),
    SSTAR_CLK_FIXED_RATE(SSTAR_CLK_XTAL_12M, "CLK_xtali_12m", 12000000),
    SSTAR_CLK_FIXED_RATE(SSTAR_CLK_XTAL_24M, "CLK_xtali_24m", 24000000),
    SSTAR_CLK_FIXED_RATE(SSTAR_CLK_MPLL_691M, "CLK_mpll_691m", 691000000),
    SSTAR_CLK_FIXED_RATE(SSTAR_CLK_MPLL_576MH, "CLK_mpll_576m", 576000000),
    SSTAR_CLK_FIXED_RATE(SSTAR_CLK_MPLL_432M, "CLK_mpll_432m", 432000000),
    SSTAR_CLK_FIXED_RATE(SSTAR_CLK_MPLL_345M, "CLK_mpll_345m", 345000000),
    SSTAR_CLK_FIXED_RATE(SSTAR_CLK_MPLL_288M, "CLK_mpll_288m", 288000000),
    SSTAR_CLK_FIXED_RATE(SSTAR_CLK_MPLL_246M, "CLK_mpll_246m", 246800000),
    SSTAR_CLK_FIXED_RATE(SSTAR_CLK_MPLL_216M, "CLK_mpll_216m", 216000000),
    SSTAR_CLK_FIXED_RATE(SSTAR_CLK_MPLL_172M, "CLK_mpll_172m", 172800000),
    SSTAR_CLK_FIXED_RATE(SSTAR_CLK_MPLL_144M, "CLK_mpll_144m", 144000000),
    SSTAR_CLK_FIXED_RATE(SSTAR_CLK_MPLL_123M, "CLK_mpll_123m", 123400000),
    SSTAR_CLK_FIXED_RATE(SSTAR_CLK_MPLL_86M, "CLK_mpll_86m", 86400000),
    SSTAR_CLK_FIXED_RATE(SSTAR_CLK_UPLL_480M, "CLK_upll_480m", 480000000),
    SSTAR_CLK_FIXED_RATE(SSTAR_CLK_UPLL_384M, "CLK_upll_384m", 384000000),
    SSTAR_CLK_FIXED_RATE(SSTAR_CLK_UPLL_320M, "CLK_upll_320m", 320000000),
    SSTAR_CLK_FIXED_RATE(SSTAR_CLK_SYSPLL_600M, "CLK_syspll_600m", 600000000),
    SSTAR_CLK_FIXED_RATE(SSTAR_CLK_SYSPLL_400M, "CLK_syspll_400m", 400000000),
    SSTAR_CLK_FIXED_RATE(SSTAR_CLK_GMACPLL, "CLK_gmacpll_250", 250000000),
    SSTAR_CLK_FIXED_RATE(SSTAR_CLK_UTMI_240M, "CLK_utmi_240m", 240000000),
    SSTAR_CLK_FIXED_RATE(SSTAR_CLK_UTMI_192M, "CLK_utmi_192m", 192000000),
    SSTAR_CLK_FIXED_RATE(SSTAR_CLK_UTMI_160M, "CLK_utmi_160m", 160000000),
    SSTAR_CLK_FIXED_RATE(SSTAR_CLK_GMAC0_EXT100M, "CLK_gmac0_external_ref_buf", 100000000),
    SSTAR_CLK_FIXED_RATE(SSTAR_CLK_GMAC1_EXT100M, "CLK_gmac1_external_ref_buf", 100000000),

    /* fixed factor clocks */
    SSTAR_CLK_FIXED_FACTOR(SSTAR_CLK_MPLL_432M_DIV2, "CLK_mpll_432m_div2", "CLK_mpll_432m", 1, 2),
    SSTAR_CLK_FIXED_FACTOR(SSTAR_CLK_MPLL_432M_DIV4, "CLK_mpll_432m_div4", "CLK_mpll_432m", 1, 4),
    SSTAR_CLK_FIXED_FACTOR(SSTAR_CLK_MPLL_432M_DIV8, "CLK_mpll_432m_div8", "CLK_mpll_432m", 1, 8),
    SSTAR_CLK_FIXED_FACTOR(SSTAR_CLK_MPLL_432M_DIV16, "CLK_mpll_432m_div16", "CLK_mpll_432m", 1, 16),
    SSTAR_CLK_FIXED_FACTOR(SSTAR_CLK_MPLL_345M_DIV2, "CLK_mpll_345m_div2", "CLK_mpll_345m", 1, 2),
    SSTAR_CLK_FIXED_FACTOR(SSTAR_CLK_MPLL_288M_DIV2, "CLK_mpll_288m_div2", "CLK_mpll_288m", 1, 2),
    SSTAR_CLK_FIXED_FACTOR(SSTAR_CLK_MPLL_288M_DIV4, "CLK_mpll_288m_div4", "CLK_mpll_288m", 1, 4),
    SSTAR_CLK_FIXED_FACTOR(SSTAR_CLK_MPLL_288M_DIV8, "CLK_mpll_288m_div8", "CLK_mpll_288m", 1, 8),
    SSTAR_CLK_FIXED_FACTOR(SSTAR_CLK_MPLL_246M_DIV2, "CLK_mpll_246m_div2", "CLK_mpll_246m", 1, 2),
    SSTAR_CLK_FIXED_FACTOR(SSTAR_CLK_MPLL_246M_DIV4, "CLK_mpll_246m_div4", "CLK_mpll_246m", 1, 4),
    SSTAR_CLK_FIXED_FACTOR(SSTAR_CLK_MPLL_216M_DIV2, "CLK_mpll_216m_div2", "CLK_mpll_216m", 1, 2),
    SSTAR_CLK_FIXED_FACTOR(SSTAR_CLK_MPLL_216M_DIV4, "CLK_mpll_216m_div4", "CLK_mpll_216m", 1, 4),
    SSTAR_CLK_FIXED_FACTOR(SSTAR_CLK_MPLL_216M_DIV8, "CLK_mpll_216m_div8", "CLK_mpll_216m", 1, 8),
    SSTAR_CLK_FIXED_FACTOR(SSTAR_CLK_MPLL_144M_DIV2, "CLK_mpll_144m_div2", "CLK_mpll_144m", 1, 2),
    SSTAR_CLK_FIXED_FACTOR(SSTAR_CLK_MPLL_144M_DIV4, "CLK_mpll_144m_div4", "CLK_mpll_144m", 1, 4),
    SSTAR_CLK_FIXED_FACTOR(SSTAR_CLK_MPLL_123M_DIV2, "CLK_mpll_123m_div2", "CLK_mpll_123m", 1, 2),
    SSTAR_CLK_FIXED_FACTOR(SSTAR_CLK_MPLL_86M_DIV2, "CLK_mpll_86m_div2", "CLK_mpll_86m", 1, 2),
    SSTAR_CLK_FIXED_FACTOR(SSTAR_CLK_MPLL_86M_DIV4, "CLK_mpll_86m_div4", "CLK_mpll_86m", 1, 4),
    SSTAR_CLK_FIXED_FACTOR(SSTAR_CLK_MPLL_86M_DIV16, "CLK_mpll_86m_div16", "CLK_mpll_246m", 1, 16),
    SSTAR_CLK_FIXED_FACTOR(SSTAR_CLK_UPLL_480M_DIV2, "CLK_upll_480m_div2", "CLK_upll_480m", 1, 2),
    SSTAR_CLK_FIXED_FACTOR(SSTAR_CLK_UPLL_384M_DIV2, "CLK_upll_384m_div2", "CLK_upll_384m", 1, 2),
    SSTAR_CLK_FIXED_FACTOR(SSTAR_CLK_UPLL_384M_DIV8, "CLK_upll_384m_div8", "CLK_upll_384m", 1, 8),
    SSTAR_CLK_FIXED_FACTOR(SSTAR_CLK_UPLL_320M_DIV2, "CLK_upll_320m_div2", "CLK_upll_320m", 1, 2),
    SSTAR_CLK_FIXED_FACTOR(SSTAR_CLK_UPLL_320M_DIV8, "CLK_upll_320m_div8", "CLK_upll_320m", 1, 8),
    SSTAR_CLK_FIXED_FACTOR(SSTAR_CLK_UPLL_320M_DIV10, "CLK_upll_320m_div10", "CLK_upll_320m", 1, 10),
    SSTAR_CLK_FIXED_FACTOR(SSTAR_CLK_UPLL_320M_DIV16, "CLK_upll_320m_div16", "CLK_upll_320m", 1, 16),
    SSTAR_CLK_FIXED_FACTOR(SSTAR_CLK_SYSPLL_400M_DIV2, "CLK_syspll_400m_div2", "CLK_syspll_400m", 1, 2),
    SSTAR_CLK_FIXED_FACTOR(SSTAR_CLK_SYSPLL_400M_DIV4, "CLK_syspll_400m_div4", "CLK_syspll_400m", 1, 4),
    SSTAR_CLK_FIXED_FACTOR(SSTAR_CLK_SYSPLL_400M_DIV8, "CLK_syspll_400m_div8", "CLK_syspll_400m", 1, 8),
    SSTAR_CLK_FIXED_FACTOR(SSTAR_CLK_GMACPLL_DIV2, "CLK_gmacpll_250_div2", "CLK_gmacpll_250", 1, 2),
    SSTAR_CLK_FIXED_FACTOR(SSTAR_CLK_GMACPLL_DIV5, "CLK_gmacpll_250_div5", "CLK_gmacpll_250", 1, 5),
    SSTAR_CLK_FIXED_FACTOR(SSTAR_CLK_GMACPLL_DIV10, "CLK_gmacpll_250_div10", "CLK_gmacpll_250", 1, 10),
    SSTAR_CLK_FIXED_FACTOR(SSTAR_CLK_GMACPLL_DIV100, "CLK_gmacpll_250_div100", "CLK_gmacpll_250", 1, 100),
    SSTAR_CLK_FIXED_FACTOR(SSTAR_CLK_UTMI_192M_DIV4, "CLK_utmi_192m_div4", "CLK_utmi_192m", 1, 4),
    SSTAR_CLK_FIXED_FACTOR(SSTAR_CLK_UTMI_160M_DIV4, "CLK_utmi_160m_div4", "CLK_utmi_160m", 1, 4),
    SSTAR_CLK_FIXED_FACTOR(SSTAR_CLK_UTMI_160M_DIV5, "CLK_utmi_160m_div5", "CLK_utmi_160m", 1, 5),
    SSTAR_CLK_FIXED_FACTOR(SSTAR_CLK_UTMI_160M_DIV8, "CLK_utmi_160m_div8", "CLK_utmi_160m", 1, 8),
    SSTAR_CLK_FIXED_FACTOR(SSTAR_CLK_XTAL_12M_DIV2, "CLK_xtali_12m_div2", "CLK_xtali_12m", 1, 2),
    SSTAR_CLK_FIXED_FACTOR(SSTAR_CLK_XTAL_12M_DIV4, "CLK_xtali_12m_div4", "CLK_xtali_12m", 1, 4),
    SSTAR_CLK_FIXED_FACTOR(SSTAR_CLK_XTAL_12M_DIV8, "CLK_xtali_12m_div8", "CLK_xtali_12m", 1, 8),
    SSTAR_CLK_FIXED_FACTOR(SSTAR_CLK_XTAL_12M_DIV16, "CLK_xtali_12m_div16", "CLK_xtali_12m", 1, 16),
    SSTAR_CLK_FIXED_FACTOR(SSTAR_CLK_XTAL_12M_DIV40, "CLK_xtali_12m_div40", "CLK_xtali_12m", 1, 40),

    /* composite clocks */
    SSTAR_CLK_COMPOSITE(SSTAR_CLK_UART0, "CLK_uart0", uart_parents, ARRAY_SIZE(uart_parents), REG_CKG_UART0_BASE,
                        REG_CKG_UART0_OFFSET + 2, 2, REG_CKG_UART0_OFFSET, 0),
    SSTAR_CLK_COMPOSITE(SSTAR_CLK_UART1, "CLK_uart1", uart_parents, ARRAY_SIZE(uart_parents), REG_CKG_UART1_BASE,
                        REG_CKG_UART1_OFFSET + 2, 2, REG_CKG_UART1_OFFSET, 0),
    SSTAR_CLK_COMPOSITE(SSTAR_CLK_UART2, "CLK_uart2", uart_parents, ARRAY_SIZE(uart_parents), REG_CKG_UART2_BASE,
                        REG_CKG_UART2_OFFSET + 2, 2, REG_CKG_UART2_OFFSET, 0),
    SSTAR_CLK_COMPOSITE(SSTAR_CLK_UART3, "CLK_uart3", uart_parents, ARRAY_SIZE(uart_parents), REG_CKG_UART3_BASE,
                        REG_CKG_UART3_OFFSET + 2, 2, REG_CKG_UART3_OFFSET, 0),
    SSTAR_CLK_COMPOSITE(SSTAR_CLK_UART4, "CLK_uart4", uart_parents, ARRAY_SIZE(uart_parents), REG_CKG_UART4_BASE,
                        REG_CKG_UART4_OFFSET + 2, 2, REG_CKG_UART4_OFFSET, 0),
    SSTAR_CLK_COMPOSITE(SSTAR_CLK_UART5, "CLK_uart5", uart_parents, ARRAY_SIZE(uart_parents), REG_CKG_UART5_BASE,
                        REG_CKG_UART5_OFFSET + 2, 2, REG_CKG_UART5_OFFSET, 0),
    SSTAR_CLK_COMPOSITE(SSTAR_CLK_FUART0_SYNTH, "CLK_fuart0_synth", fuart0_synth_parents,
                        ARRAY_SIZE(fuart0_synth_parents), REG_CKG_FUART0_SYNTH_IN_BASE,
                        REG_CKG_FUART0_SYNTH_IN_OFFSET + 2, 2, REG_CKG_FUART0_SYNTH_IN_OFFSET, 0),
    SSTAR_CLK_COMPOSITE(SSTAR_CLK_FUART, "CLK_fuart", fuart_parents, ARRAY_SIZE(fuart_parents), REG_CKG_FUART_BASE,
                        REG_CKG_FUART_OFFSET + 2, 2, REG_CKG_FUART_OFFSET, 0),
    SSTAR_CLK_COMPOSITE(SSTAR_CLK_GMAC0_GMII, "CLK_gmac0_gmii", gmac0_parents, ARRAY_SIZE(gmac0_parents),
                        REG_CKG_GMAC0_GMII_BASE, REG_CKG_GMAC0_GMII_OFFSET + 2, 2, REG_CKG_GMAC0_GMII_OFFSET, 0),
    SSTAR_CLK_COMPOSITE(SSTAR_CLK_GMAC1_GMII, "CLK_gmac1_gmii", gmac1_parents, ARRAY_SIZE(gmac1_parents),
                        REG_CKG_GMAC1_GMII_BASE, REG_CKG_GMAC1_GMII_OFFSET + 2, 2, REG_CKG_GMAC1_GMII_OFFSET, 0),
    SSTAR_CLK_COMPOSITE(SSTAR_CLK_IPUFF, "CLK_ipuff", ipuff_parents, ARRAY_SIZE(ipuff_parents), REG_CKG_IPUFF_BASE,
                        REG_CKG_IPUFF_OFFSET + 2, 2, REG_CKG_IPUFF_OFFSET, 0),
    SSTAR_CLK_COMPOSITE(SSTAR_CLK_PCIE0, "CLK_pcie0", pcie_parents, ARRAY_SIZE(pcie_parents), REG_CKG_PCIE0_BASE,
                        REG_CKG_PCIE0_OFFSET + 2, 2, REG_CKG_PCIE0_OFFSET, 0),
    SSTAR_CLK_COMPOSITE(SSTAR_CLK_PCIE1, "CLK_pcie1", pcie_parents, ARRAY_SIZE(pcie_parents), REG_CKG_PCIE1_BASE,
                        REG_CKG_PCIE1_OFFSET + 2, 2, REG_CKG_PCIE1_OFFSET, 0),
    SSTAR_CLK_COMPOSITE(SSTAR_CLK_MSPI0, "CLK_mspi0", mspi_parents, ARRAY_SIZE(mspi_parents), REG_CKG_MSPI0_BASE,
                        REG_CKG_MSPI0_OFFSET + 2, 2, REG_CKG_MSPI0_OFFSET, 0),
    SSTAR_CLK_COMPOSITE(SSTAR_CLK_MSPI1, "CLK_mspi1", mspi_parents, ARRAY_SIZE(mspi_parents), REG_CKG_MSPI1_BASE,
                        REG_CKG_MSPI1_OFFSET + 2, 2, REG_CKG_MSPI1_OFFSET, 0),
    SSTAR_CLK_COMPOSITE(SSTAR_CLK_MSPI2, "CLK_mspi2", mspi_parents, ARRAY_SIZE(mspi_parents), REG_CKG_MSPI2_BASE,
                        REG_CKG_MSPI2_OFFSET + 2, 2, REG_CKG_MSPI2_OFFSET, 0),
    SSTAR_CLK_COMPOSITE(SSTAR_CLK_MSPI3, "CLK_mspi3", mspi_parents, ARRAY_SIZE(mspi_parents), REG_CKG_MSPI3_BASE,
                        REG_CKG_MSPI3_OFFSET + 2, 2, REG_CKG_MSPI3_OFFSET, 0),
    SSTAR_CLK_COMPOSITE(SSTAR_CLK_MIIC0, "CLK_miic0", miic_parents, ARRAY_SIZE(miic_parents), REG_CKG_MIIC0_BASE,
                        REG_CKG_MIIC0_OFFSET + 2, 2, REG_CKG_MIIC0_OFFSET, 0),
    SSTAR_CLK_COMPOSITE(SSTAR_CLK_MIIC1, "CLK_miic1", miic_parents, ARRAY_SIZE(miic_parents), REG_CKG_MIIC1_BASE,
                        REG_CKG_MIIC1_OFFSET + 2, 2, REG_CKG_MIIC1_OFFSET, 0),
    SSTAR_CLK_COMPOSITE(SSTAR_CLK_MIIC2, "CLK_miic2", miic_parents, ARRAY_SIZE(miic_parents), REG_CKG_MIIC2_BASE,
                        REG_CKG_MIIC2_OFFSET + 2, 2, REG_CKG_MIIC2_OFFSET, 0),
    SSTAR_CLK_COMPOSITE(SSTAR_CLK_MIIC3, "CLK_miic3", miic_parents, ARRAY_SIZE(miic_parents), REG_CKG_MIIC3_BASE,
                        REG_CKG_MIIC3_OFFSET + 2, 2, REG_CKG_MIIC3_OFFSET, 0),
    SSTAR_CLK_COMPOSITE(SSTAR_CLK_MIIC4, "CLK_miic4", miic_parents, ARRAY_SIZE(miic_parents), REG_CKG_MIIC4_BASE,
                        REG_CKG_MIIC4_OFFSET + 2, 2, REG_CKG_MIIC4_OFFSET, 0),
    SSTAR_CLK_COMPOSITE(SSTAR_CLK_MIIC5, "CLK_miic5", miic_parents, ARRAY_SIZE(miic_parents), REG_CKG_MIIC5_BASE,
                        REG_CKG_MIIC5_OFFSET + 2, 2, REG_CKG_MIIC5_OFFSET, 0),
    SSTAR_CLK_COMPOSITE(SSTAR_CLK_MIIC6, "CLK_miic6", miic_parents, ARRAY_SIZE(miic_parents), REG_CKG_MIIC6_BASE,
                        REG_CKG_MIIC6_OFFSET + 2, 2, REG_CKG_MIIC6_OFFSET, 0),
    SSTAR_CLK_COMPOSITE(SSTAR_CLK_MIIC7, "CLK_miic7", miic_parents, ARRAY_SIZE(miic_parents), REG_CKG_MIIC7_BASE,
                        REG_CKG_MIIC7_OFFSET + 2, 2, REG_CKG_MIIC7_OFFSET, 0),
    SSTAR_CLK_COMPOSITE(SSTAR_CLK_MIIC8, "CLK_miic8", miic_parents, ARRAY_SIZE(miic_parents), REG_CKG_MIIC8_BASE,
                        REG_CKG_MIIC8_OFFSET + 2, 2, REG_CKG_MIIC8_OFFSET, 0),
    SSTAR_CLK_COMPOSITE(SSTAR_CLK_MIIC9, "CLK_miic9", miic_parents, ARRAY_SIZE(miic_parents), REG_CKG_MIIC9_BASE,
                        REG_CKG_MIIC9_OFFSET + 2, 2, REG_CKG_MIIC9_OFFSET, 0),
    SSTAR_CLK_COMPOSITE(SSTAR_CLK_MIIC10, "CLK_miic10", miic_parents, ARRAY_SIZE(miic_parents), REG_CKG_MIIC10_BASE,
                        REG_CKG_MIIC10_OFFSET + 2, 2, REG_CKG_MIIC10_OFFSET, 0),
    SSTAR_CLK_COMPOSITE(SSTAR_CLK_PWM, "CLK_pwm", pwm_parents, ARRAY_SIZE(pwm_parents), REG_CKG_PWM_BASE,
                        REG_CKG_PWM_OFFSET + 2, 3, REG_CKG_PWM_OFFSET, 0),

};

const sstar_clk_data* hal_clk_data_list(int* count)
{
    if (count)
        *count = ARRAY_SIZE(clk_list);

    debug("sstar clocks count %d\r\n", *count);
    return clk_list;
}
