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

static const char* const fuart0_synth_parents[] = {"CLK_mpll_432m", "CLK_mpll_432m_div2"};

static const char* const fuart_parents[] = {"CLK_mpll_345m_div2", "CLK_mpll_288m_div2", "CLK_xtali_12m",
                                            "CLK_fuart0_synth"};

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
    SSTAR_CLK_FIXED_RATE(SSTAR_CLK_UTMI_240M, "CLK_utmi_240m", 240000000),
    SSTAR_CLK_FIXED_RATE(SSTAR_CLK_UTMI_192M, "CLK_utmi_192m", 192000000),
    SSTAR_CLK_FIXED_RATE(SSTAR_CLK_UTMI_160M, "CLK_utmi_160m", 160000000),


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

    SSTAR_CLK_COMPOSITE(SSTAR_CLK_FUART0_SYNTH, "CLK_fuart0_synth", fuart0_synth_parents,
                        ARRAY_SIZE(fuart0_synth_parents), REG_CKG_FUART0_SYNTH_IN_BASE,
                        REG_CKG_FUART0_SYNTH_IN_OFFSET + 2, 2, REG_CKG_FUART0_SYNTH_IN_OFFSET, 0),
    SSTAR_CLK_COMPOSITE(SSTAR_CLK_FUART, "CLK_fuart", fuart_parents, ARRAY_SIZE(fuart_parents), REG_CKG_FUART_BASE,
                        REG_CKG_FUART_OFFSET + 2, 2, REG_CKG_FUART_OFFSET, 0),
    SSTAR_CLK_COMPOSITE(SSTAR_CLK_MSPI0, "CLK_mspi0", mspi_parents, ARRAY_SIZE(mspi_parents), REG_CKG_MSPI0_BASE,
                        REG_CKG_MSPI0_OFFSET + 2, 2, REG_CKG_MSPI0_OFFSET, 0),
    SSTAR_CLK_COMPOSITE(SSTAR_CLK_MSPI1, "CLK_mspi1", mspi_parents, ARRAY_SIZE(mspi_parents), REG_CKG_MSPI1_BASE,
    SSTAR_CLK_COMPOSITE(SSTAR_CLK_MIIC0, "CLK_miic0", miic_parents, ARRAY_SIZE(miic_parents), REG_CKG_MIIC0_BASE,
                        REG_CKG_MIIC0_OFFSET + 2, 2, REG_CKG_MIIC0_OFFSET, 0),
    SSTAR_CLK_COMPOSITE(SSTAR_CLK_MIIC1, "CLK_miic1", miic_parents, ARRAY_SIZE(miic_parents), REG_CKG_MIIC1_BASE,
                        REG_CKG_MIIC1_OFFSET + 2, 2, REG_CKG_MIIC1_OFFSET, 0),
    SSTAR_CLK_COMPOSITE(SSTAR_CLK_MIIC2, "CLK_miic2", miic_parents, ARRAY_SIZE(miic_parents), REG_CKG_MIIC2_BASE,
                        REG_CKG_MIIC2_OFFSET + 2, 2, REG_CKG_MIIC2_OFFSET, 0),
    SSTAR_CLK_COMPOSITE(SSTAR_CLK_MIIC3, "CLK_miic3", miic_parents, ARRAY_SIZE(miic_parents), REG_CKG_MIIC3_BASE,
                        REG_CKG_MIIC3_OFFSET + 2, 2, REG_CKG_MIIC3_OFFSET, 0),
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
