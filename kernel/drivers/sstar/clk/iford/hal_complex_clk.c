/*
 * hal_complex_clk.c- Sigmastar
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
#include <linux/kernel.h>
#include <linux/clkdev.h>
#include <linux/clk-provider.h>
#include <linux/clk.h>
#include <linux/delay.h>
#include <linux/module.h>
#include <ms_platform.h>
#include <registers.h>
#include <reg_clks.h>
#include <hal_complex_clk.h>
#ifdef CONFIG_PM_SLEEP
#include <linux/syscore_ops.h>
#endif

#ifdef CONFIG_PM_SLEEP
static u8 _is_syscore_register = 0;

static LIST_HEAD(hal_pll_init_list);
#endif
static long hal_clk_cpuclk_round_rate(struct clk_hw *clk_hw, unsigned long rate, unsigned long *parent_rate)
{
    clk_dbg("hal_clk_cpuclk_round_rate = %lu\n", rate);

    if (rate < 100000000)
    {
        return 100000000;
    }
    else if (rate > 1400000000)
    {
        return 1400000000;
    }
    else
    {
        return rate;
    }
}

unsigned long hal_clk_cpuclk_recalc_rate(struct clk_hw *clk_hw, unsigned long parent_rate)
{
    unsigned long rate;
    U32           lpf_value;
    U32           post_div;

    /*
     * get LPF high
     */
    lpf_value = INREG16(BASE_REG_CPUPLL_PA + REG_ID_52) + (INREG16(BASE_REG_CPUPLL_PA + REG_ID_53) << 16);
    post_div  = INREG16(BASE_REG_CPUPLL_PA + REG_ID_19) + 1;

    /*
     * special handling for 1st time aquire after system boot
     */
    if (lpf_value == 0)
    {
        lpf_value = (INREG8(BASE_REG_CPUPLL_PA + REG_ID_61) << 16) + INREG16(BASE_REG_CPUPLL_PA + REG_ID_60);
        clk_dbg("lpf_value = %u, post_div=%u\n", lpf_value, post_div);
    }

    /*
     * Calculate LPF value for DFS
     * LPF_value(5.19) = (432MHz / Ref_clk) * 2^19  =>  it's for post_div=2
     * Ref_clk = CPU_CLK * 2 / 32
     */
    rate = (div64_u64(432000000llu * 524288, lpf_value) * 2 / post_div * 32 / 2);

    clk_dbg("hal_clk_cpuclk_recalc_rate = %lu, prate=%lu\n", rate, parent_rate);

    return rate;
}

void hal_clk_cpu_dvfs(U32 target_lpf, U32 target_post_div)
{
    U32 current_post_div = 0;
    U32 temp_post_div    = 0;
    U32 current_lpf      = 0;

    current_post_div = INREGMSK16(BASE_REG_CPUPLL_PA + REG_ID_19, 0x000F) + 1;

    if (target_post_div > current_post_div)
    {
        temp_post_div = current_post_div;
        while (temp_post_div != target_post_div)
        {
            temp_post_div = temp_post_div << 1;
            OUTREGMSK16(BASE_REG_CPUPLL_PA + REG_ID_19, temp_post_div - 1, 0x000F);
        }
    }

    current_lpf = INREG16(BASE_REG_CPUPLL_PA + REG_ID_50) + (INREG16(BASE_REG_CPUPLL_PA + REG_ID_51) << 16);
    if (current_lpf == 0)
    {
        current_lpf = (INREG8(BASE_REG_CPUPLL_PA + REG_ID_61) << 16) + INREG16(BASE_REG_CPUPLL_PA + REG_ID_60);
        /*
         * store freq to LPF low
         */
        OUTREG16(BASE_REG_CPUPLL_PA + REG_ID_50, current_lpf & 0xFFFF);
        OUTREG16(BASE_REG_CPUPLL_PA + REG_ID_51, (current_lpf >> 16) & 0xFFFF);
    }

    /*
     * REG_ID_54: reg_lpf_enable = 0
     * REG_ID_57: reg_lpf_update_cnt = 32
     * REG_ID_52: set target freq to LPF high
     * REG_ID_53: set target freq to LPF high
     * REG_ID_58: switch to LPF control
     * REG_ID_59: from low to high
     * REG_ID_54: reg_lpf_enable = 1
     */
    OUTREG16(BASE_REG_CPUPLL_PA + REG_ID_54, 0x0000);
    OUTREG16(BASE_REG_CPUPLL_PA + REG_ID_57, 0x000F);
    OUTREG16(BASE_REG_CPUPLL_PA + REG_ID_52, target_lpf & 0xFFFF);
    OUTREG16(BASE_REG_CPUPLL_PA + REG_ID_53, (target_lpf >> 16) & 0xFFFF);
    OUTREG16(BASE_REG_CPUPLL_PA + REG_ID_58, 0x0001);
    SETREG16(BASE_REG_CPUPLL_PA + REG_ID_59, BIT12);
    OUTREG16(BASE_REG_CPUPLL_PA + REG_ID_54, 0x0001);

    /*
     * polling
     */
    while (!(INREG16(BASE_REG_CPUPLL_PA + REG_ID_5D) & BIT0))
        ;
    /*
     * store freq to LPF low
     */
    OUTREG16(BASE_REG_CPUPLL_PA + REG_ID_50, target_lpf & 0xFFFF);
    OUTREG16(BASE_REG_CPUPLL_PA + REG_ID_51, (target_lpf >> 16) & 0xFFFF);

    if (target_post_div < current_post_div)
    {
        temp_post_div = current_post_div;
        while (temp_post_div != target_post_div)
        {
            temp_post_div = temp_post_div >> 1;
            OUTREGMSK16(BASE_REG_CPUPLL_PA + REG_ID_19, temp_post_div - 1, 0x000F);
        }
    }
}

int hal_clk_cpuclk_set_rate(struct clk_hw *clk_hw, unsigned long rate, unsigned long parent_rate)
{
    int ret = 0;

    unsigned int lpf_value;
    unsigned int post_div = 2;

    clk_dbg("hal_clk_cpuclk_set_rate = %lu\n", rate);

    /*
     * The default of post_div is 2, choose appropriate post_div by CPU clock.
     */
    if (rate >= 800000000)
        post_div = 2;
    else if (rate >= 400000000)
        post_div = 4;
    else if (rate >= 200000000)
        post_div = 8;
    else
        post_div = 16;

    /*
     * Calculate LPF value for DFS
     * LPF_value(5.19) = (432MHz / Ref_clk) * 2^19  =>  it's for post_div=2
     * Ref_clk = CPU_CLK * 2 / 32
     */
    lpf_value = (U32)(div64_u64(432000000llu * 524288, (rate * 2 / 32) * post_div / 2));

    hal_clk_cpu_dvfs(lpf_value, post_div);

    return ret;
}

static int hal_clk_cpuclk_is_enabled(struct clk_hw *hw)
{
    return ((INREG16(BASE_REG_CPUPLL_PA + REG_ID_11) & (BIT8)) == 0x0000);
}

int hal_clk_cpuclk_init(struct clk_hw *clk_hw)
{
    /*
     * force on MPLL 216MHz for CPU DVFS
     */
    SETREG16(BASE_REG_CLKGEN_PA + REG_ID_71, BIT9);

    return 0;
}

struct clk_ops sstar_cpuclk_ops = {
    .round_rate  = hal_clk_cpuclk_round_rate,
    .recalc_rate = hal_clk_cpuclk_recalc_rate,
    .is_enabled  = hal_clk_cpuclk_is_enabled,
    .set_rate    = hal_clk_cpuclk_set_rate,
    .init        = hal_clk_cpuclk_init,
};

static int hal_clk_armpll_37p125m_is_enabled(struct clk_hw *hw)
{
    return ((INREG16(BASE_REG_CPUPLL_PA + REG_ID_17) & (BIT15)) == 0x0000);
}

static unsigned long hal_clk_armpll_37p125m_recalc_rate(struct clk_hw *hw, unsigned long parent_rate)
{
    return 37125000;
}

struct clk_ops sstar_armpll_37p125m_ops = {
    .is_enabled  = hal_clk_armpll_37p125m_is_enabled,
    .recalc_rate = hal_clk_armpll_37p125m_recalc_rate,
};

static int hal_clk_ipupll_init(struct clk_hw *hw)
{
    U16 u16regval;
    u16regval = INREG16(BASE_REG_IPUPLL_PA + REG_ID_11) & (BIT8);
    if (!u16regval)
    {
        // dualos case, ipu may be enable before linux bringup
        clk_err("[%s] ipupll already init, skip\n", __func__);
        return 0;
    }

    /*
     * set ipupll to 600M
     */
    OUTREG16(BASE_REG_IPUPLL_PA + REG_ID_12, 0x1088);
    OUTREG8(BASE_REG_IPUPLL_PA + REG_ID_61, 0x002E);
    OUTREG16(BASE_REG_IPUPLL_PA + REG_ID_60, 0x147B);
    OUTREG8(BASE_REG_IPUPLL_PA + REG_ID_18, 0x01);

    /*
     * Update IPU frequency synthesizer
     */
    OUTREG16(BASE_REG_IPUPLL_PA + REG_ID_62, 0x0001);

    OUTREG16(BASE_REG_IPUPLL_PA + REG_ID_14, 0x0000);
    OUTREG16(BASE_REG_IPUPLL_PA + REG_ID_11, 0x0180);

    return 0;
}

static int hal_clk_ipupll_enable(struct clk_hw *hw)
{
    CLRREG16(BASE_REG_IPUPLL_PA + REG_ID_11, BIT8);

    return 0;
}

static void hal_clk_ipupll_disable(struct clk_hw *hw)
{
    SETREG16(BASE_REG_IPUPLL_PA + REG_ID_11, BIT8);
}

static int hal_clk_ipupll_is_enabled(struct clk_hw *hw)
{
    return ((INREG16(BASE_REG_IPUPLL_PA + REG_ID_11) & (BIT8)) == 0x0000);
}

void hal_ipu_scaling_freq(IPU_FreqTable_t *pFreqTable)
{
    OUTREG8(BASE_REG_IPUPLL_PA + REG_ID_12, 0x88);

    // IPU PLL VCO output divider, 0 means "CLKO_CPU=VCO/2", 1 means "CLKO_CPU=VCO/4", DO NOT change it arbitrarily.
    // reg_mipspll_en_post=0x0, It's default setting.
    // reg[103525]#4
    OUTREG16(BASE_REG_IPUPLL_PA + REG_ID_12, (pFreqTable->outDiv << 8) | 0x88);

    // post divider, 1 means div2, 3 means div4, 7 means div8, f means div16
    OUTREG8(BASE_REG_IPUPLL_PA + REG_ID_18, pFreqTable->postDiv);

    // IPU frequency synthesizer N.f setting
    // Format: N.f
    // How to setting?
    // According to default IPU PLL loop divider setting, PLL output = 32 x PLL input.
    // The input of IPU frequency synthesizer is 432MHz. (Default).
    // For 1200MHz PLL output, the input of PLL should be (2400/32) 75MHz.
    // The N.f setting is 432/75 x 2^19, the rounding
    OUTREG8(BASE_REG_IPUPLL_PA + REG_ID_61, pFreqTable->high);
    OUTREG16(BASE_REG_IPUPLL_PA + REG_ID_60, pFreqTable->low | (pFreqTable->mid << 8));
    // Update IPU frequency synthesizer N.f setting
    OUTREG16(BASE_REG_IPUPLL_PA + REG_ID_62, 0x0001);
}

static long hal_clk_ipupll_round_rate(struct clk_hw *hw, unsigned long rate, unsigned long *parent_rate)
{
    int           index      = 0;
    unsigned long round_rate = 0;

    for (index = 0; index < sizeof(stIpuFreqTable) / sizeof(stIpuFreqTable[0]); index++)
    {
        if (rate == stIpuFreqTable[index].freq)
        {
            round_rate = stIpuFreqTable[index].freq;
            break;
        }

        // initial value
        if (round_rate == 0)
        {
            round_rate = stIpuFreqTable[index].freq;
            continue;
        }

        if (rate < stIpuFreqTable[index].freq)
        {
            // found the nearest round up clock
            if (round_rate < rate)
            {
                round_rate = stIpuFreqTable[index].freq;
            }
            else if (round_rate > stIpuFreqTable[index].freq)
            {
                round_rate = stIpuFreqTable[index].freq;
            }
        }
        else
        {
            // found the nearest round down clock
            if (round_rate > rate)
            {
                continue;
            }
            else if (round_rate < stIpuFreqTable[index].freq)
            {
                round_rate = stIpuFreqTable[index].freq;
            }
        }
    }
    clk_dbg("round rate:%lu\n", round_rate);

    return round_rate;
}

static int hal_clk_ipupll_set_rate(struct clk_hw *hw, unsigned long rate, unsigned long parent_rate)
{
    int ret = 0, index = 0;

    for (index = 0; index < sizeof(stIpuFreqTable) / sizeof(stIpuFreqTable[0]); index++)
    {
        if (rate == stIpuFreqTable[index].freq)
        {
            hal_ipu_scaling_freq(&stIpuFreqTable[index]);
            break;
        }
    }

    if (index == sizeof(stIpuFreqTable) / sizeof(stIpuFreqTable[0]))
    {
        clk_err("\nunsupported ipupll rate %lu\n\n", rate);
    }

    return ret;
}

static unsigned long hal_clk_ipupll_recalc_rate(struct clk_hw *hw, unsigned long parent_rate)
{
    int           recalc_rate = 0;
    unsigned long clk = 0, vco = 0;
    U32           synth_set = 0, value = 0;
    u8            select_xtal = 0, post_div = 0, output_div = 0, loop_div = 0;

    // out_clk  = vco / post_div / output_div
    // vco      = clk * loop_div
    // clk      = if (select_xtal==1) 24M; else 524288 * 432 / synth_set
    //
    // loop_div   :
    //    if 12[7-6] == 0:
    //        2 * 12[5:0]
    //    else:
    //        2^12[7:6] * 12[5:0]
    //
    // output_div :
    //     (12[12] + 1)
    //
    // post_div   :
    //     18[3:0] + 1
    //
    // synth_set:
    //     60[23:0]

    synth_set   = INREG16(BASE_REG_IPUPLL_PA + REG_ID_60) + (INREG16(BASE_REG_IPUPLL_PA + REG_ID_61) << 16);
    select_xtal = INREG8(BASE_REG_IPUPLL_PA + REG_ID_11) & 0x1;
    output_div  = ((INREG16(BASE_REG_IPUPLL_PA + REG_ID_12) >> 12) + 1);
    post_div    = (INREG8(BASE_REG_IPUPLL_PA + REG_ID_18) & 0xf) + 1;

    value = INREG16(BASE_REG_IPUPLL_PA + REG_ID_12) & 0xFF;
    if ((value >> 6) == 0)
        loop_div = 2 * (value & 0x3F);
    else
        loop_div = (1 << (value >> 6)) * (value & 0x3F);

    if (select_xtal == 1)
        clk = 24000000;
    else
        clk = div64_u64(432000000llu * 524288, synth_set);

    vco         = clk * loop_div;
    recalc_rate = vco / post_div / output_div;

    clk_dbg("[%s]recalc_rate %lu \r\n", __func__, (int)recalc_rate);

    return recalc_rate;
}

struct clk_ops sstar_ipupll_ops = {
    .init        = hal_clk_ipupll_init,
    .enable      = hal_clk_ipupll_enable,
    .disable     = hal_clk_ipupll_disable,
    .is_enabled  = hal_clk_ipupll_is_enabled,
    .set_rate    = hal_clk_ipupll_set_rate,
    .round_rate  = hal_clk_ipupll_round_rate,
    .recalc_rate = hal_clk_ipupll_recalc_rate,
};

static int hal_clk_miupll_is_enabled(struct clk_hw *hw)
{
    return ((INREG16(BASE_REG_MIUPLL_PA + REG_ID_01) & (BIT8)) == 0x0000);
}

static unsigned long hal_clk_miupll_recalc_rate(struct clk_hw *hw, unsigned long parent_rate)
{
    int recalc_rate = 0;
    U32 val         = 0;
    u8  input_div = 0, loop_div1 = 0, output_div = 0, loop_div2 = 0;

    // vco = 24 / input_div * (1 << loop_div1) * output_div / (loop_div2 + 2)
    //
    // input_div  : [1031, 0x02, ( 5: 4)]
    //     00,01: 1
    //     01,11: 2
    // loop_div1  : [1031, 0x02, ( 9: 8)]
    // output_div : [1031, 0x03, ( 7: 0)]
    // loop_div2  : [1031, 0x03, (10: 8)]

    val       = INREG16(BASE_REG_MIUPLL_PA + REG_ID_02);
    input_div = (((val & (BIT4 | BIT5)) >> 4) & 0x01) == 0 ? 1 : 2;
    loop_div1 = ((val & (BIT8 | BIT9)) >> 8);

    val        = INREG16(BASE_REG_MIUPLL_PA + REG_ID_03);
    output_div = (val & 0xff);
    loop_div2  = (val >> 8);

    recalc_rate = ((24000000 / input_div * (1 << loop_div1) * output_div / (loop_div2 + 2)));
    clk_dbg("[%s]recalc_rate %d \r\n", __func__, (int)recalc_rate);

    return recalc_rate;
}

struct clk_ops sstar_miupll_ops = {
    .is_enabled  = hal_clk_miupll_is_enabled,
    .recalc_rate = hal_clk_miupll_recalc_rate,
};

static int hal_clk_upll_960m_enable(struct clk_hw *hw)
{
    CLRREG8(BASE_REG_UPLL0_PA + REG_ID_00, BIT1);

    return 0;
}

static void hal_clk_upll_960m_disable(struct clk_hw *hw)
{
    SETREG8(BASE_REG_UPLL0_PA + REG_ID_00, BIT1);
}

static int hal_clk_upll_960m_is_enabled(struct clk_hw *hw)
{
    return ((INREG16(BASE_REG_UPLL0_PA + REG_ID_00) & (BIT1)) == 0x0000);
}

static unsigned long hal_clk_upll_960m_recalc_rate(struct clk_hw *hw, unsigned long parent_rate)
{
    return 960000000;
}

struct clk_ops sstar_upll_960m_ops = {
    .enable      = hal_clk_upll_960m_enable,
    .disable     = hal_clk_upll_960m_disable,
    .is_enabled  = hal_clk_upll_960m_is_enabled,
    .recalc_rate = hal_clk_upll_960m_recalc_rate,
};

static int hal_clk_upll_480m_enable(struct clk_hw *hw)
{
    SETREG8(BASE_REG_UPLL0_PA + REG_ID_07, BIT4);
    return 0;
}

static void hal_clk_upll_480m_disable(struct clk_hw *hw)
{
    CLRREG8(BASE_REG_UPLL0_PA + REG_ID_07, BIT4);
}

static int hal_clk_upll_480m_is_enabled(struct clk_hw *hw)
{
    return ((INREG16(BASE_REG_UPLL0_PA + REG_ID_07) & (BIT4)) == 0x0010);
}

static unsigned long hal_clk_upll_480m_recalc_rate(struct clk_hw *hw, unsigned long parent_rate)
{
    return 480000000;
}

struct clk_ops sstar_upll_480m_ops = {
    .enable      = hal_clk_upll_480m_enable,
    .disable     = hal_clk_upll_480m_disable,
    .is_enabled  = hal_clk_upll_480m_is_enabled,
    .recalc_rate = hal_clk_upll_480m_recalc_rate,
};

static int hal_clk_upll_384m_enable(struct clk_hw *hw)
{
    SETREG8(BASE_REG_UPLL0_PA + REG_ID_07, BIT0);
    return 0;
}

static void hal_clk_upll_384m_disable(struct clk_hw *hw)
{
    CLRREG8(BASE_REG_UPLL0_PA + REG_ID_07, BIT0);
}

static int hal_clk_upll_384m_is_enabled(struct clk_hw *hw)
{
    return ((INREG16(BASE_REG_UPLL0_PA + REG_ID_07) & (BIT0)) == 0x0001);
}

static unsigned long hal_clk_upll_384m_recalc_rate(struct clk_hw *hw, unsigned long parent_rate)
{
    return 384000000;
}

struct clk_ops sstar_upll_384m_ops = {
    .enable      = hal_clk_upll_384m_enable,
    .disable     = hal_clk_upll_384m_disable,
    .is_enabled  = hal_clk_upll_384m_is_enabled,
    .recalc_rate = hal_clk_upll_384m_recalc_rate,
};

static int hal_clk_upll_320m_enable(struct clk_hw *hw)
{
    CLRREG8(BASE_REG_UPLL0_PA + REG_ID_00, BIT5);
    return 0;
}

static void hal_clk_upll_320m_disable(struct clk_hw *hw)
{
    SETREG8(BASE_REG_UPLL0_PA + REG_ID_00, BIT5);
}

static int hal_clk_upll_320m_is_enabled(struct clk_hw *hw)
{
    return ((INREG16(BASE_REG_UPLL0_PA + REG_ID_00) & (BIT5)) == 0x0000);
}

static unsigned long hal_clk_upll_320m_recalc_rate(struct clk_hw *hw, unsigned long parent_rate)
{
    return 320000000;
}

struct clk_ops sstar_upll_320m_ops = {
    .enable      = hal_clk_upll_320m_enable,
    .disable     = hal_clk_upll_320m_disable,
    .is_enabled  = hal_clk_upll_320m_is_enabled,
    .recalc_rate = hal_clk_upll_320m_recalc_rate,
};

static int hal_clk_upll_48m_enable(struct clk_hw *hw)
{
    CLRREG8(BASE_REG_UPLL0_PA + REG_ID_07, BIT3);
    return 0;
}

static void hal_clk_upll_48m_disable(struct clk_hw *hw)
{
    SETREG8(BASE_REG_UPLL0_PA + REG_ID_07, BIT3);
}

static int hal_clk_upll_48m_is_enabled(struct clk_hw *hw)
{
    return ((INREG16(BASE_REG_UPLL0_PA + REG_ID_07) & (BIT3)) == 0x0000);
}

static unsigned long hal_clk_upll_48m_recalc_rate(struct clk_hw *hw, unsigned long parent_rate)
{
    return 48000000;
}

struct clk_ops sstar_upll_48m_ops = {
    .enable      = hal_clk_upll_48m_enable,
    .disable     = hal_clk_upll_48m_disable,
    .is_enabled  = hal_clk_upll_48m_is_enabled,
    .recalc_rate = hal_clk_upll_48m_recalc_rate,
};

static u16 hal_clk_disp_oplf_get_pnl_lpll_gain(u16 idx)
{
    u16 *p_tbl = NULL;

    p_tbl = lpll_loop_gain;

    return (idx < HAL_CLK_DISP_PNL_SUPPORTED_LPLL_MAX) ? p_tbl[idx] : 1;
}

static u16 hal_clk_disp_oplf_get_pnl_lpll_div(u16 idx)
{
    u16 *p_tbl = NULL;

    p_tbl = lpll_loop_div;

    return (idx < HAL_CLK_DISP_PNL_SUPPORTED_LPLL_MAX) ? p_tbl[idx] : 1;
}

static void hal_clk_disp_lpll_dump_setting(u16 idx, u8 b_lpll_1)
{
    u16 start_idx;
    u16 end_idx;
    u16 reg_idx;

    start_idx = 0;
    end_idx   = start_idx + HAL_CLK_DISP_PNL_LPLL_REG_NUM;

    clk_dbg("%s %d, Idx:%d, Num(%d %d), b_lpll_1:%d\n", __FUNCTION__, __LINE__, idx, start_idx, end_idx, b_lpll_1);

    if (idx < HAL_CLK_DISP_PNL_SUPPORTED_LPLL_MAX)
    {
        for (reg_idx = start_idx; reg_idx < end_idx; reg_idx++)
        {
            if (lpll_setting_tbl[idx][reg_idx].address == 0xFFFFFFF)
            {
                // DrvSclOsDelayTask(lpll_setting_tbl[idx][reg_idx].value);
                continue;
            }
            if (b_lpll_1)
            {
                // unsupport disppll1, only support disppll0
            }
            else
            {
                // don't change enable state
                clk_dbg("%lx %x\n", HAL_CLK_REG_DISP_LPLL_0_40_L, lpll_setting_tbl[idx][reg_idx].address);
                if (HAL_CLK_REG_DISP_LPLL_0_40_L == lpll_setting_tbl[idx][reg_idx].address)
                {
                    u16 value = (INREG16(BASE_REG_DISPPLL_PA + REG_ID_40) & (BIT15));
                    value     = (lpll_setting_tbl[idx][reg_idx].value & 0x7fff) | value;
                    clk_dbg("%x \n", value);

                    OUTREG16(BASE_REG_RIU_PA + ((lpll_setting_tbl[idx][reg_idx].address) << 1), value);
                }
                else
                {
                    OUTREG16(BASE_REG_RIU_PA + ((lpll_setting_tbl[idx][reg_idx].address) << 1),
                             lpll_setting_tbl[idx][reg_idx].value);
                }
            }
        }
    }
}

static void hal_clk_disp_lpll_set_lpll_set(u32 lpll_set, u8 b_lpll_1)
{
    u16 lpll_set_lo, lpll_set_hi;

    lpll_set_lo = (u16)(lpll_set & 0x0000FFFF);
    lpll_set_hi = (u16)((lpll_set & 0x00FF0000) >> 16);

    if (b_lpll_1)
    {
        // unsupport disppll1, only support disppll0
    }
    else
    {
        OUTREG16(BASE_REG_DISPPLL_PA + REG_ID_48, lpll_set_lo);
        OUTREG16(BASE_REG_DISPPLL_PA + REG_ID_49, lpll_set_hi);
    }
}

static int hal_clk_disp_pll_set_rate(unsigned long rate, u16 disp_path)
{
    u8  ret = 1;
    U16 idx, loop_gain, loop_div;
    u32 dividen, divisor, lpll_set;

    if (IS_DATA_LANE_LESS_2_8M(rate))
    {
        idx = HAL_CLK_DISP_PNL_SUPPORTED_LPLL_MAX;
        ret = 0;
    }
    else if (IS_DATA_LANE_BPS_2_8M_TO_3_25M(rate))
        idx = HAL_CLK_DISP_PNL_SUPPORTED_LPLL_HS_LVDS_CH_2D8TO3D25MHZ;
    else if (IS_DATA_LANE_BPS_3_25M_TO_6_5M(rate))
        idx = HAL_CLK_DISP_PNL_SUPPORTED_LPLL_HS_LVDS_CH_3D25TO6D5MHZ;
    else if (IS_DATA_LANE_BPS_6_5M_TO_12_5M(rate))
        idx = HAL_CLK_DISP_PNL_SUPPORTED_LPLL_HS_LVDS_CH_6D5TO12D5MHZ;
    else if (IS_DATA_LANE_BPS_12_5M_TO_25M(rate))
        idx = HAL_CLK_DISP_PNL_SUPPORTED_LPLL_HS_LVDS_CH_12D5TO25MHZ;
    else if (IS_DATA_LANE_BPS_25M_TO_50M(rate))
        idx = HAL_CLK_DISP_PNL_SUPPORTED_LPLL_HS_LVDS_CH_25TO50MHZ;
    else if (IS_DATA_LANE_BPS_74_25M(rate))
        idx = HAL_CLK_DISP_PNL_SUPPORTED_LPLL_HS_LVDS_CH_74D25MHZ;
    else if (IS_DATA_LANE_BPS_50M_TO_100M(rate))
        idx = HAL_CLK_DISP_PNL_SUPPORTED_LPLL_HS_LVDS_CH_50TO100MHZ;
    else if (IS_DATA_LANE_BPS_148_5M(rate))
        idx = HAL_CLK_DISP_PNL_SUPPORTED_LPLL_HS_LVDS_CH_148D5MHZ;
    else if (IS_DATA_LANE_BPS_100M_TO_312_5M(rate))
        idx = HAL_CLK_DISP_PNL_SUPPORTED_LPLL_HS_LVDS_CH_100TO312D5MHZ;
    else
    {
        idx = HAL_CLK_DISP_PNL_SUPPORTED_LPLL_MAX;
        ret = 0;
    }

    if (ret == 0)
    {
        clk_err("%s %d, DCLK out of range: %ld\n", __FUNCTION__, __LINE__, rate);
        return -1;
    }

    if (idx != HAL_CLK_DISP_PNL_SUPPORTED_LPLL_MAX)
    {
        loop_div  = hal_clk_disp_oplf_get_pnl_lpll_div(idx);
        loop_gain = hal_clk_disp_oplf_get_pnl_lpll_gain(idx);

        dividen  = ((u32)432 * (u32)524288 * (u32)loop_gain);
        divisor  = rate * (u32)loop_div / 1000000;
        lpll_set = dividen / divisor;

        clk_dbg("%s %d, Idx:%d, LoopGain:%d, LoopDiv:%d, dclk=%ld, Divden:0x%x, Divisor:0x%x, LpllSe:0x%x\n",
                __FUNCTION__, __LINE__, idx, loop_gain, loop_div, rate, dividen, divisor, lpll_set);

        hal_clk_disp_lpll_dump_setting(idx, disp_path);
        hal_clk_disp_lpll_set_lpll_set(lpll_set, disp_path);
    }
    else
        return -1;

    return 0;
}

static int hal_clk_disppll_init(struct clk_hw *hw)
{
    U16 u16regval;
    u16regval = INREG16(BASE_REG_DISPPLL_PA + REG_ID_40);
    if (!(u16regval & 0x8000))
    {
        clk_err("[%s] disppll already init, skip\n", __func__);
        return 0;
    }

    /*
     * set disppll to 74.25M
     */
    OUTREG16(BASE_REG_DISPPLL_PA + REG_ID_48, 0x8BA3);
    OUTREG8(BASE_REG_DISPPLL_PA + REG_ID_49, 0x2E);

    OUTREG16(BASE_REG_DISPPLL_PA + REG_ID_40, 0x8A81);
    OUTREG16(BASE_REG_DISPPLL_PA + REG_ID_41, 0x0420);
    OUTREG16(BASE_REG_DISPPLL_PA + REG_ID_42, 0x0023);

    return 0;
}

static int hal_clk_disppll_enable(struct clk_hw *hw)
{
    CLRREG16(BASE_REG_DISPPLL_PA + REG_ID_40, BIT15);
    return 0;
}

static void hal_clk_disppll_disable(struct clk_hw *hw)
{
    SETREG16(BASE_REG_DISPPLL_PA + REG_ID_40, BIT15);
}

static int hal_clk_disppll_is_enabled(struct clk_hw *hw)
{
    return ((INREG16(BASE_REG_DISPPLL_PA + REG_ID_40) & (BIT15)) == 0x0000);
}

static int hal_clk_disppll_set_rate(struct clk_hw *hw, unsigned long rate, unsigned long parent_rate)
{
    return hal_clk_disp_pll_set_rate(rate, 0);
}

static long hal_clk_disppll_round_rate(struct clk_hw *hw, unsigned long rate, unsigned long *parent_rate)
{
    unsigned long round_rate = 0;
    round_rate               = rate;
    return round_rate;
}

static unsigned long hal_clk_disppll_recalc_rate(struct clk_hw *hw, unsigned long parent_rate)
{
    unsigned long recalc_rate = 0, vco = 0;
    U16           input_div1, loop_div1, loop_div2, scalar_div1, scalar_div2;
    u32           val, lpllset, lpllset_lo, lpllset_hi;
    u32           div_3p5, div_2p5, scalar_div;
    // vco  = 432 * 524288 / lpllset / (1 << input_div1) * (1 << loop_div1) * loop_div2
    // freq = vco / (1 << scalar_div1) / scalar_div2
    //
    // lpllset     :  [0x1037, 0x48] + [0x1037, 0x49] << 16
    // input_div1  :  [0x1037, 0x41, ( 1, 0)]
    // loop_div1   :  [0x1037, 0x41, ( 5, 4)]
    // loop_div2   :  [0x1037, 0x41, (11, 8)]
    // scalar_div1 :  [0x1037, 0x42, ( 1, 0)]
    // scalar_div2 :  [0x1037, 0x42, ( 7, 4)]

    lpllset_lo = INREG16(BASE_REG_DISPPLL_PA + REG_ID_48);
    lpllset_hi = INREG8(BASE_REG_DISPPLL_PA + REG_ID_49);
    lpllset    = lpllset_lo + (lpllset_hi << 16);

    val        = INREG16(BASE_REG_DISPPLL_PA + REG_ID_41);
    input_div1 = (val & 0x00f);
    loop_div1  = (val & 0x0f0) >> 4;
    loop_div2  = (val & 0xf00) >> 8;

    val     = INREG16(BASE_REG_DISPPLL_PA + REG_ID_40);
    div_3p5 = (val & BIT3) >> 3;
    div_2p5 = (val & BIT8) >> 8;

    val         = INREG16(BASE_REG_DISPPLL_PA + REG_ID_42);
    scalar_div1 = (val & 0x0f);
    scalar_div2 = (val & 0xf0) >> 4;

    if (div_3p5 & BIT3)
    {
        scalar_div = 7;
    }
    else
    {
        if (scalar_div2)
        {
            scalar_div = scalar_div2 * (1 << scalar_div1);
            if (div_2p5)
            {
                scalar_div = 5 * scalar_div;
            }
            else
            {
                scalar_div = 2 * scalar_div;
            }
        }
        else
        {
            scalar_div = 2;
        }
    }

    vco         = div64_u64(432000000llu * 524288, lpllset) / (1 << input_div1) * loop_div2 * (1 << loop_div1);
    recalc_rate = vco / (scalar_div / 2);
    clk_dbg("lpllset:0x%x, input_div1:0x%x, loop_div1:0x%x, loop_div2:0x%x scalar_div:%d vco:%ld\n", lpllset,
            input_div1, loop_div1, loop_div2, scalar_div / 2, vco);
    return recalc_rate;
}

struct clk_ops sstar_disppll_ops = {
    .init        = hal_clk_disppll_init,
    .enable      = hal_clk_disppll_enable,
    .disable     = hal_clk_disppll_disable,
    .is_enabled  = hal_clk_disppll_is_enabled,
    .set_rate    = hal_clk_disppll_set_rate,
    .round_rate  = hal_clk_disppll_round_rate,
    .recalc_rate = hal_clk_disppll_recalc_rate,
};

static int hal_clk_rtcpll_is_enabled(struct clk_hw *hw)
{
    return ((INREG16(BASE_REG_RTCPLL_PA + REG_ID_00) & (BIT0)) == 0x0000);
}

static unsigned long hal_clk_rtcpll_recalc_rate(struct clk_hw *hw, unsigned long parent_rate)
{
    int recalc_rate = 250000000;

    return recalc_rate;
}

struct clk_ops sstar_rtcpll_ops = {
    .is_enabled  = hal_clk_rtcpll_is_enabled,
    .recalc_rate = hal_clk_rtcpll_recalc_rate,
};

static int hal_clk_spipll_init(struct clk_hw *hw)
{
    OUTREG16(BASE_REG_SPIPLL_PA + REG_ID_00, 0xD61C);
    OUTREG16(BASE_REG_SPIPLL_PA + REG_ID_01, 0x003E);
    OUTREG16(BASE_REG_SPIPLL_PA + REG_ID_36, 0x0000);
    OUTREG16(BASE_REG_SPIPLL_PA + REG_ID_32, 0x0001);
    OUTREG16(BASE_REG_SPIPLL_PA + REG_ID_33, 0x0000);
    OUTREG16(BASE_REG_SPIPLL_PA + REG_ID_34, 0x0002);
    OUTREG16(BASE_REG_SPIPLL_PA + REG_ID_35, 0x0000);
    OUTREG16(BASE_REG_SPIPLL_PA + REG_ID_37, 0x0000);

    // power down
    SETREG16(BASE_REG_SPIPLL_PA + REG_ID_36, BIT0);

    return 0;
}

static int hal_clk_spipll_enable(struct clk_hw *hw)
{
    CLRREG16(BASE_REG_SPIPLL_PA + REG_ID_36, BIT0);
    return 0;
}

static void hal_clk_spipll_disable(struct clk_hw *hw)
{
    SETREG16(BASE_REG_SPIPLL_PA + REG_ID_36, BIT0);
}

static int hal_clk_spipll_is_enabled(struct clk_hw *hw)
{
    return ((INREG16(BASE_REG_SPIPLL_PA + REG_ID_36) & (BIT0)) == 0x0000);
}

static unsigned long hal_clk_spipll_recalc_rate(struct clk_hw *hw, unsigned long parent_rate)
{
    int recalc_rate = 0;
    u32 synth_set;
    u8  loop_div_second = 0;
    u8  post_div_reg    = 0;
    u8  post_div        = 0;

    synth_set       = INREG16(BASE_REG_SPIPLL_PA + REG_ID_00) + (INREG16(BASE_REG_SPIPLL_PA + REG_ID_01) << 16);
    post_div_reg    = INREG8(BASE_REG_SPIPLL_PA + REG_ID_37);
    post_div        = (post_div_reg < 6) ? ((post_div_reg < 2) ? 2 : (post_div_reg * 2)) : 16;
    loop_div_second = INREG8(BASE_REG_SPIPLL_PA + REG_ID_35) / post_div;
    recalc_rate     = div64_u64(432000000llu * 524288, synth_set) * loop_div_second;

    clk_dbg("[%s]recalc_rate %lu \r\n", __func__, (int)recalc_rate);

    return recalc_rate;
}

static long hal_clk_spipll_round_rate(struct clk_hw *clk_hw, unsigned long rate, unsigned long *parent_rate)
{
    U16 index;
    for (index = 0; index < sizeof(spipll_synth_tbl) / sizeof(hal_spipll_table); index++)
    {
        if (rate <= spipll_synth_tbl[index].frequency)
        {
            return spipll_synth_tbl[index].frequency;
        }
    }
    return 216000000;
}

static int hal_clk_spipll_set_rate(struct clk_hw *clk_hw, unsigned long rate, unsigned long parent_rate)
{
    U16 index;

    if ((rate == 104000000) || (rate == 133000000) || (rate == 160000000) || (rate == 172000000) || (rate == 208000000)
        || (rate == 216000000))
    {
        for (index = 0; index < sizeof(spipll_synth_tbl) / sizeof(hal_spipll_table); index++)
        {
            if (spipll_synth_tbl[index].frequency == rate)
            {
                OUTREG16(BASE_REG_SPIPLL_PA + REG_ID_00, spipll_synth_tbl[index].synthesizer & 0xFFFF);
                OUTREG16(BASE_REG_SPIPLL_PA + REG_ID_01, (spipll_synth_tbl[index].synthesizer >> 16) & 0xFF);
                OUTREG16(BASE_REG_SPIPLL_PA + REG_ID_36, 0x0000);
                OUTREG16(BASE_REG_SPIPLL_PA + REG_ID_32, 0x0001);
                OUTREG16(BASE_REG_SPIPLL_PA + REG_ID_33, spipll_synth_tbl[index].input_div_first & 0x3);
                OUTREG16(BASE_REG_SPIPLL_PA + REG_ID_34, spipll_synth_tbl[index].loop_div_first & 0x3);
                OUTREG16(BASE_REG_SPIPLL_PA + REG_ID_35, spipll_synth_tbl[index].loop_div_second & 0xFF);
                OUTREG16(BASE_REG_SPIPLL_PA + REG_ID_37, spipll_synth_tbl[index].post_div);
                OUTREG16(BASE_REG_SPIPLL_PA + REG_ID_31, 0x0001);
            }
        }
    }
    else
    {
        clk_err("\nunsupported spipll rate %lu\n\n", rate);
        return -1;
    }
    return 0;
}

struct clk_ops sstar_spipll_ops = {
    .init        = hal_clk_spipll_init,
    .enable      = hal_clk_spipll_enable,
    .disable     = hal_clk_spipll_disable,
    .is_enabled  = hal_clk_spipll_is_enabled,
    .recalc_rate = hal_clk_spipll_recalc_rate,
    .round_rate  = hal_clk_spipll_round_rate,
    .set_rate    = hal_clk_spipll_set_rate,
};

static int hal_clk_pm_timer4_enable(struct clk_hw *hw)
{
    SETREG8(BASE_REG_PMSLEEP_PA + REG_ID_33, BIT0);

    return 0;
}

static int hal_clk_pm_timer4_is_enabled(struct clk_hw *hw)
{
    return ((INREG16(BASE_REG_PMSLEEP_PA + REG_ID_33) & (BIT0)) == BIT0);
}

static void hal_clk_pm_timer4_disable(struct clk_hw *hw)
{
    CLRREG8(BASE_REG_PMSLEEP_PA + REG_ID_33, BIT0);
}

struct clk_ops sstar_pm_timer4_ops = {
    .enable     = hal_clk_pm_timer4_enable,
    .is_enabled = hal_clk_pm_timer4_is_enabled,
    .disable    = hal_clk_pm_timer4_disable,
};

static int hal_clk_pm_timer5_enable(struct clk_hw *hw)
{
    SETREG8(BASE_REG_PMSLEEP_PA + REG_ID_33, BIT1);

    return 0;
}

static int hal_clk_pm_timer5_is_enabled(struct clk_hw *hw)
{
    return ((INREG16(BASE_REG_PMSLEEP_PA + REG_ID_33) & (BIT1)) == BIT1);
}

static void hal_clk_pm_timer5_disable(struct clk_hw *hw)
{
    CLRREG8(BASE_REG_PMSLEEP_PA + REG_ID_33, BIT1);
}

struct clk_ops sstar_pm_timer5_ops = {
    .enable     = hal_clk_pm_timer5_enable,
    .is_enabled = hal_clk_pm_timer5_is_enabled,
    .disable    = hal_clk_pm_timer5_disable,
};

static int hal_clk_pm_timer6_enable(struct clk_hw *hw)
{
    SETREG8(BASE_REG_PMSLEEP_PA + REG_ID_33, BIT2);

    return 0;
}

static int hal_clk_pm_timer6_is_enabled(struct clk_hw *hw)
{
    return ((INREG16(BASE_REG_PMSLEEP_PA + REG_ID_33) & (BIT2)) == BIT2);
}

static void hal_clk_pm_timer6_disable(struct clk_hw *hw)
{
    CLRREG8(BASE_REG_PMSLEEP_PA + REG_ID_33, BIT2);
}

struct clk_ops sstar_pm_timer6_ops = {
    .enable     = hal_clk_pm_timer6_enable,
    .is_enabled = hal_clk_pm_timer6_is_enabled,
    .disable    = hal_clk_pm_timer6_disable,
};

static int hal_clk_pm_timer7_enable(struct clk_hw *hw)
{
    SETREG8(BASE_REG_PMSLEEP_PA + REG_ID_33, BIT3);

    return 0;
}

static int hal_clk_pm_timer7_is_enabled(struct clk_hw *hw)
{
    return ((INREG16(BASE_REG_PMSLEEP_PA + REG_ID_33) & (BIT3)) == BIT3);
}

static void hal_clk_pm_timer7_disable(struct clk_hw *hw)
{
    CLRREG8(BASE_REG_PMSLEEP_PA + REG_ID_33, BIT3);
}

struct clk_ops sstar_pm_timer7_ops = {
    .enable     = hal_clk_pm_timer7_enable,
    .is_enabled = hal_clk_pm_timer7_is_enabled,
    .disable    = hal_clk_pm_timer7_disable,
};

static int hal_clk_fuart_synth_init(struct clk_hw *hw)
{
    /*
     * disable reset
     */
    SETREG16(BASE_REG_CLKGEN_PA + REG_ID_34, BIT9);

    /*
     * set rate to 192M
     */
    OUTREG16(BASE_REG_CLKGEN_PA + REG_ID_35, 0x1200);

    return 0;
}

static int hal_clk_fuart_synth_out_enable(struct clk_hw *hw)
{
    SETREG16(BASE_REG_CLKGEN_PA + REG_ID_34, BIT8);
    return 0;
}

static void hal_clk_fuart_synth_out_disable(struct clk_hw *hw)
{
    CLRREG16(BASE_REG_CLKGEN_PA + REG_ID_34, BIT8);
}

static int hal_clk_fuart_synth_out_is_enabled(struct clk_hw *hw)
{
    return ((INREG16(BASE_REG_CLKGEN_PA + REG_ID_34) & (0x300)) == 0x0300);
}

static unsigned long hal_clk_fuart_synth_out_recalc_rate(struct clk_hw *hw, unsigned long parent_rate)
{
    unsigned long rate;
    switch (parent_rate)
    {
        case 432000000:
            rate = 192000000;
            break;
        case 216000000:
            rate = 96000000;
            break;
        default:
            rate = 192000000;
    }
    return rate;
}

static long hal_clk_fuart_synth_out_round_rate(struct clk_hw *clk_hw, unsigned long rate, unsigned long *parent_rate)
{
    U16 i;
    for (i = 0; i < sizeof(fuart_synth_out_tbl) / sizeof(hal_clk_fuart_tbl); i++)
    {
        if (rate <= fuart_synth_out_tbl[i].frequency)
        {
            return fuart_synth_out_tbl[i].frequency;
        }
    }
    return 216000000;
}

static int hal_clk_fuart_synth_out_set_rate(struct clk_hw *clk_hw, unsigned long rate, unsigned long parent_rate)
{
    U16 i;

    /*
     * synth clk should be (50MHz ~ 133.33MHz), and output clk = synth_clk * 24 / div(default 4),
     * so if not change div, out clk range is 300MHz ~ 800MHz
     */
    if ((rate == 216000000) || (rate == 203294000) || (rate == 192000000) || (rate == 181895000) || (rate == 172800000)
        || (rate == 164571000))
    {
        for (i = 0; i < sizeof(fuart_synth_out_tbl) / sizeof(hal_clk_fuart_tbl); i++)
        {
            if (fuart_synth_out_tbl[i].frequency == rate)
            {
                OUTREG16(BASE_REG_CLKGEN_PA + REG_ID_35, fuart_synth_out_tbl[i].val);
            }
        }
    }
    else
    {
        clk_err("\nunsupported venpll rate %lu\n\n", rate);
        return -1;
    }
    return 0;
}

struct clk_ops sstar_fuart_synth_out_ops = {
    .init        = hal_clk_fuart_synth_init,
    .enable      = hal_clk_fuart_synth_out_enable,
    .disable     = hal_clk_fuart_synth_out_disable,
    .is_enabled  = hal_clk_fuart_synth_out_is_enabled,
    .recalc_rate = hal_clk_fuart_synth_out_recalc_rate,
    .round_rate  = hal_clk_fuart_synth_out_round_rate,
    .set_rate    = hal_clk_fuart_synth_out_set_rate,
};

static int hal_clk_fuart0_synth_init(struct clk_hw *hw)
{
    /*
     * disable reset
     */
    SETREG16(BASE_REG_CLKGEN_PA + REG_ID_2D, BIT9);

    /*
     * set rate to 192M
     */
    OUTREG16(BASE_REG_CLKGEN_PA + REG_ID_2E, 0x1200);

    return 0;
}

static int hal_clk_fuart0_synth_out_enable(struct clk_hw *hw)
{
    SETREG16(BASE_REG_CLKGEN_PA + REG_ID_2D, BIT8);
    return 0;
}

static void hal_clk_fuart0_synth_out_disable(struct clk_hw *hw)
{
    CLRREG16(BASE_REG_CLKGEN_PA + REG_ID_2D, BIT8);
}

static int hal_clk_fuart0_synth_out_is_enabled(struct clk_hw *hw)
{
    return ((INREG16(BASE_REG_CLKGEN_PA + REG_ID_2D) & (0x300)) == 0x0300);
}

static unsigned long hal_clk_fuart0_synth_out_recalc_rate(struct clk_hw *hw, unsigned long parent_rate)
{
    unsigned long rate;
    switch (parent_rate)
    {
        case 432000000:
            rate = 192000000;
            break;
        case 216000000:
            rate = 96000000;
            break;
        default:
            rate = 192000000;
    }
    return rate;
}

static long hal_clk_fuart0_synth_out_round_rate(struct clk_hw *clk_hw, unsigned long rate, unsigned long *parent_rate)
{
    U16 i;
    for (i = 0; i < sizeof(fuart0_synth_out_tbl) / sizeof(hal_clk_fuart_tbl); i++)
    {
        if (rate <= fuart0_synth_out_tbl[i].frequency)
        {
            return fuart0_synth_out_tbl[i].frequency;
        }
    }
    return 216000000;
}

static int hal_clk_fuart0_synth_out_set_rate(struct clk_hw *clk_hw, unsigned long rate, unsigned long parent_rate)
{
    U16 i;

    /*
     * synth clk should be (50MHz ~ 133.33MHz), and output clk = synth_clk * 24 / div(default 4),
     * so if not change div, out clk range is 300MHz ~ 800MHz
     */
    if ((rate == 216000000) || (rate == 203294000) || (rate == 192000000) || (rate == 181895000) || (rate == 172800000)
        || (rate == 164571000))
    {
        for (i = 0; i < sizeof(fuart0_synth_out_tbl) / sizeof(hal_clk_fuart_tbl); i++)
        {
            if (fuart0_synth_out_tbl[i].frequency == rate)
            {
                OUTREG16(BASE_REG_CLKGEN_PA + REG_ID_2E, fuart0_synth_out_tbl[i].val);
            }
        }
    }
    else
    {
        clk_err("\nunsupported venpll rate %lu\n\n", rate);
        return -1;
    }
    return 0;
}

struct clk_ops sstar_fuart0_synth_out_ops = {
    .init        = hal_clk_fuart0_synth_init,
    .enable      = hal_clk_fuart0_synth_out_enable,
    .disable     = hal_clk_fuart0_synth_out_disable,
    .is_enabled  = hal_clk_fuart0_synth_out_is_enabled,
    .recalc_rate = hal_clk_fuart0_synth_out_recalc_rate,
    .round_rate  = hal_clk_fuart0_synth_out_round_rate,
    .set_rate    = hal_clk_fuart0_synth_out_set_rate,
};

static int hal_clk_fuart1_synth_init(struct clk_hw *hw)
{
    /*
     * disable reset
     */
    SETREG16(BASE_REG_CLKGEN_PA + REG_ID_3A, BIT9);

    /*
     * set rate to 192M
     */
    OUTREG16(BASE_REG_CLKGEN_PA + REG_ID_3B, 0x1200);

    return 0;
}

static int hal_clk_fuart1_synth_out_enable(struct clk_hw *hw)
{
    SETREG16(BASE_REG_CLKGEN_PA + REG_ID_3A, BIT8);
    return 0;
}

static void hal_clk_fuart1_synth_out_disable(struct clk_hw *hw)
{
    CLRREG16(BASE_REG_CLKGEN_PA + REG_ID_3A, BIT8);
}

static int hal_clk_fuart1_synth_out_is_enabled(struct clk_hw *hw)
{
    return ((INREG16(BASE_REG_CLKGEN_PA + REG_ID_3A) & (0x300)) == 0x0300);
}

static unsigned long hal_clk_fuart1_synth_out_recalc_rate(struct clk_hw *hw, unsigned long parent_rate)
{
    unsigned long rate;
    switch (parent_rate)
    {
        case 432000000:
            rate = 192000000;
            break;
        case 216000000:
            rate = 96000000;
            break;
        default:
            rate = 192000000;
    }
    return rate;
}

static long hal_clk_fuart1_synth_out_round_rate(struct clk_hw *clk_hw, unsigned long rate, unsigned long *parent_rate)
{
    U16 i;
    for (i = 0; i < sizeof(fuart1_synth_out_tbl) / sizeof(hal_clk_fuart_tbl); i++)
    {
        if (rate <= fuart1_synth_out_tbl[i].frequency)
        {
            return fuart1_synth_out_tbl[i].frequency;
        }
    }
    return 216000000;
}

static int hal_clk_fuart1_synth_out_set_rate(struct clk_hw *clk_hw, unsigned long rate, unsigned long parent_rate)
{
    U16 i;

    /*
     * synth clk should be (50MHz ~ 133.33MHz), and output clk = synth_clk * 24 / div(default 4),
     * so if not change div, out clk range is 300MHz ~ 800MHz
     */
    if ((rate == 216000000) || (rate == 203294000) || (rate == 192000000) || (rate == 181895000) || (rate == 172800000)
        || (rate == 164571000))
    {
        for (i = 0; i < sizeof(fuart1_synth_out_tbl) / sizeof(hal_clk_fuart_tbl); i++)
        {
            if (fuart1_synth_out_tbl[i].frequency == rate)
            {
                OUTREG16(BASE_REG_CLKGEN_PA + REG_ID_3B, fuart1_synth_out_tbl[i].val);
            }
        }
    }
    else
    {
        clk_err("\nunsupported venpll rate %lu\n\n", rate);
        return -1;
    }
    return 0;
}

struct clk_ops sstar_fuart1_synth_out_ops = {
    .init        = hal_clk_fuart1_synth_init,
    .enable      = hal_clk_fuart1_synth_out_enable,
    .disable     = hal_clk_fuart1_synth_out_disable,
    .is_enabled  = hal_clk_fuart1_synth_out_is_enabled,
    .recalc_rate = hal_clk_fuart1_synth_out_recalc_rate,
    .round_rate  = hal_clk_fuart1_synth_out_round_rate,
    .set_rate    = hal_clk_fuart1_synth_out_set_rate,
};

int hal_clk_nullclk_init(struct clk_hw *clk_hw)
{
    printk(KERN_DEBUG "%s ops not implemented\r\n", clk_hw_get_name(clk_hw));
    return 0;
}

struct clk_ops sstar_nullclk_ops = {
    .init = hal_clk_nullclk_init,
};

static const struct of_device_id hal_clk_complex_clk_match[] = {
    {
        .name = "CLK_cpu_pll",
        .data = &sstar_cpuclk_ops,
    },
    {
        .name = "CLK_armpll_37p125m",
        .data = &sstar_armpll_37p125m_ops,
    },
    {
        .name = "CLK_ipupll_clk",
        .data = &sstar_ipupll_ops,
    },
    {
        .name = "CLK_miupll_clk",
        .data = &sstar_miupll_ops,
    },
    {
        .name = "CLK_rtcpll_clk",
        .data = &sstar_rtcpll_ops,
    },
    {
        .name = "CLK_upll_960m",
        .data = &sstar_upll_960m_ops,
    },
    {
        .name = "CLK_upll_480m",
        .data = &sstar_upll_480m_ops,
    },
    {
        .name = "CLK_upll_384m",
        .data = &sstar_upll_384m_ops,
    },
    {
        .name = "CLK_upll_320m",
        .data = &sstar_upll_320m_ops,
    },
    {
        .name = "CLK_upll_48m",
        .data = &sstar_upll_48m_ops,
    },
    {
        .name = "CLK_disppll_clk",
        .data = &sstar_disppll_ops,
    },
    {
        .name = "CLK_spi_synth_pll",
        .data = &sstar_spipll_ops,
    },
    {
        .name = "CLK_pm_timer4",
        .data = &sstar_pm_timer4_ops,
    },
    {
        .name = "CLK_pm_timer5",
        .data = &sstar_pm_timer5_ops,
    },
    {
        .name = "CLK_pm_timer6",
        .data = &sstar_pm_timer6_ops,
    },
    {
        .name = "CLK_pm_timer7",
        .data = &sstar_pm_timer7_ops,
    },
    {
        .name = "CLK_fuart_synth_out",
        .data = &sstar_fuart_synth_out_ops,
    },
    {
        .name = "CLK_fuart0_synth_out",
        .data = &sstar_fuart0_synth_out_ops,
    },
    {
        .name = "CLK_fuart1_synth_out",
        .data = &sstar_fuart1_synth_out_ops,
    },
    {},
};
MODULE_DEVICE_TABLE(of, hal_clk_complex_clk_match);

#ifdef CONFIG_PM_SLEEP
static int hal_complex_clk_suspend(void)
{
    struct hal_clk_pll_info *ss_clk;

    list_for_each_entry(ss_clk, &hal_pll_init_list, list)
    {
        if (ss_clk->pll_ops->is_enabled)
        {
            ss_clk->enable = ss_clk->pll_ops->is_enabled(NULL);
        }
        if (ss_clk->pll_ops->set_rate)
        {
            ss_clk->rate_save = ss_clk->pll_ops->recalc_rate(NULL, 0);
        }
    }

    clk_dbg("hal_complex_clk_suspend\n");

    return 0;
}

static void hal_complex_clk_resume(void)
{
    struct hal_clk_pll_info *ss_clk;
    unsigned long            round_rate = 0;

    list_for_each_entry(ss_clk, &hal_pll_init_list, list)
    {
        // ss_clk->pll_ops->init(NULL);
        if (ss_clk->pll_ops->enable && ss_clk->enable)
        {
            ss_clk->pll_ops->enable(NULL);
        }
        if (ss_clk->pll_ops->disable && !ss_clk->enable)
        {
            ss_clk->pll_ops->disable(NULL);
        }
        if (ss_clk->pll_ops->set_rate)
        {
            if (ss_clk->pll_ops->round_rate)
            {
                round_rate = ss_clk->pll_ops->round_rate(NULL, ss_clk->rate_save, 0);
            }
            else
            {
                round_rate = ss_clk->rate_save;
            }
            ss_clk->pll_ops->set_rate(NULL, round_rate, 0);
        }
    }

    clk_dbg("hal_complex_clk_resume\n");
}

struct syscore_ops hal_complex_clk_syscore_ops = {
    .suspend = hal_complex_clk_suspend,
    .resume  = hal_complex_clk_resume,
};
#endif

static void __init hal_clk_clk_complex_init(struct device_node *node)
{
    struct clk *               clk;
    struct clk_hw *            clk_hw       = NULL;
    struct clk_init_data       init         = {};
    const char **              parent_names = NULL;
    u32                        i;
    const struct of_device_id *match;
    u32                        ignore = 0;
#ifdef CONFIG_PM_SLEEP
    struct hal_clk_pll_info *complex_clk = NULL;
#endif

    clk_dbg(" @ %s ,node_name:%s \r\n", __func__, node->name);

    match  = of_match_node(hal_clk_complex_clk_match, node);
    clk_hw = kzalloc(sizeof(struct clk_hw), GFP_KERNEL);
#ifdef CONFIG_PM_SLEEP
    complex_clk = kzalloc(sizeof(struct hal_clk_pll_info), GFP_KERNEL);
#endif

    if (!clk_hw)
        goto fail;

    if (!of_property_read_u32(node, "ignore", &ignore))
    {
        if (ignore)
        {
            pr_debug("<%s> ignore gate clock\n", node->name);
            init.flags = CLK_IGNORE_UNUSED;
        }
    }

    clk_hw->init = &init;
    init.name    = node->name;

    if (match && match->data)
        init.ops = match->data;
    else
        init.ops = &sstar_nullclk_ops;

    init.num_parents = of_clk_get_parent_count(node);
    if (init.num_parents < 1)
    {
        clk_err("[%s] %s have no parent\n", __func__, node->name);
        goto fail;
    }
    else
    {
        parent_names = kzalloc(sizeof(char *) * init.num_parents, GFP_KERNEL);
        clk_dbg(" @parent_names 0x%lx \r\n", (long unsigned int)parent_names);
        if (!parent_names)
            goto fail;

        for (i = 0; i < init.num_parents; i++)
        {
            parent_names[i] = of_clk_get_parent_name(node, i);
        }

        init.parent_names = parent_names;
    }
    clk = clk_register(NULL, clk_hw);
    if (IS_ERR(clk))
    {
        clk_err("[%s] Fail to register %s\n", __func__, node->name);
        goto fail;
    }
    else
    {
        clk_dbg("[%s] %s register success\n", __func__, node->name);
    }
    of_clk_add_provider(node, of_clk_src_simple_get, clk);
    clk_register_clkdev(clk, node->name, NULL);

#ifdef CONFIG_PM_SLEEP
    if (!of_property_read_bool(node, "str-ignore"))
    {
        complex_clk->name    = node->name;
        complex_clk->pll_ops = init.ops;
        list_add_tail(&complex_clk->list, &hal_pll_init_list);
    }
    else
    {
        kfree(complex_clk);
        complex_clk = NULL;
    }
    if (!_is_syscore_register)
    {
        register_syscore_ops(&hal_complex_clk_syscore_ops);
        _is_syscore_register = 1;
    }
#endif

    kfree(parent_names);
    return;

fail:
    kfree(parent_names);
    kfree(clk_hw);
}

CLK_OF_DECLARE(hal_clk_clk_complex, "sstar,complex-clock", hal_clk_clk_complex_init);
