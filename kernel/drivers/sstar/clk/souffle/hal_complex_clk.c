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

int hal_clk_cpuclk_init(struct clk_hw *clk_hw)
{
    return 0;
}

void hal_clk_cpuclk_dvfs_disable(void)
{
    return;
}
EXPORT_SYMBOL(hal_clk_cpuclk_dvfs_disable);

struct clk_ops sstar_cpuclk_ops = {
    .round_rate  = hal_clk_cpuclk_round_rate,
    .recalc_rate = hal_clk_cpuclk_recalc_rate,
    .set_rate    = hal_clk_cpuclk_set_rate,
    .init        = hal_clk_cpuclk_init,
};

static int hal_clk_venpll_enable(struct clk_hw *hw)
{
    /*
     * reg_ven_pll_pd=0
     */
    CLRREG16(BASE_REG_VENPLL_PA + REG_ID_01, BIT8);
    return 0;
}

static void hal_clk_venpll_disable(struct clk_hw *hw)
{
    /*
     * reg_ven_pll_pd=1
     */
    SETREG16(BASE_REG_VENPLL_PA + REG_ID_01, BIT8);
}

static int hal_clk_venpll_is_enabled(struct clk_hw *hw)
{
    return ((INREG16(BASE_REG_VENPLL_PA + REG_ID_01) & BIT8) == 0x0000);
}

static unsigned long hal_clk_venpll_recalc_rate(struct clk_hw *hw, unsigned long parent_rate)
{
    return (parent_rate * INREG8(BASE_REG_VENPLL_PA + REG_ID_03) / 2);
}

static long hal_clk_venpll_round_rate(struct clk_hw *clk_hw, unsigned long rate, unsigned long *parent_rate)
{
    if (rate <= 516000000)
    {
        return 516000000;
    }
    else if (rate <= 552000000)
    {
        return 552000000;
    }
    else if (rate <= 612000000)
    {
        return 612000000;
    }
    else if (rate <= 636000000)
    {
        return 636000000;
    }
    else
    {
        return 672000000;
    }
}

static int hal_clk_venpll_set_rate(struct clk_hw *clk_hw, unsigned long rate, unsigned long parent_rate)
{
    if ((rate == 672000000) || (rate == 636000000) || (rate == 612000000) || (rate == 552000000) || (rate == 516000000))
    {
        const unsigned int u32_input_clk = 24000000;
        // PLL input-divider control:
        // 2'b00: /1,
        // 2'b01: /2
        const unsigned char u8_input_divreg = 0x0; // 1
        const unsigned char u8_input_div    = 1;
        // First-Stage loop-divider control
        // 000 for /1,
        // 001 for /2,
        // 010 for /4,
        // 011 for /8
        const unsigned char u8_loop_div_firstreg = 0x1;
        const unsigned char u8_loop_div_first    = 2;
        // 1XX for /1,
        // 011 for /5,
        // 010 for /4,
        // 001 for /3,
        // 000 for /2
        const unsigned char u8_post_divreg = 0x0; // 2
        const unsigned char u8_post_div    = 0x2;
        const unsigned char u8_output_div  = 0x1; // 1
        //
        // u32RegAddr = VEN_PLL_CLK_REG_BASE + REG_ID_01;
        CLRREG16(BASE_REG_VENPLL_PA + REG_ID_01, 0xFF);

        // u32RegAddr = VEN_PLL_CLK_REG_BASE + REG_ID_02;
        // u16RegVal = *((volatile uint16 *)u32RegAddr);
        // u16RegVal &= ~(0x730);

        CLRREG16(BASE_REG_VENPLL_PA + REG_ID_02, 0x730);

        // set inputdiv &  loopdivfirst to  0x2
        //*((volatile uint16 *)u32RegAddr)= u16RegVal | (u8InputDivReg << 4) | (u8LoopDivFirstReg << 8);
        SETREG16(BASE_REG_VENPLL_PA + REG_ID_02, (u8_input_divreg << 4) | (u8_loop_div_firstreg << 8));

        // u32RegAddr = VEN_PLL_CLK_REG_BASE + REG_ID_03;
        // u16RegVal = *((volatile uint16 *)u32RegAddr);
        // u16RegVal &= ~(0x7FF);
        CLRREG16(BASE_REG_VENPLL_PA + REG_ID_03, 0x7FF);
        // Targetrate = (InputClk / InputDiv * (LoopDivFirst * LoopSecond)) / PostDiv / OutputDiv
        // compute LoopSecond = Targetrate * PostDiv * OutputDiv * InputDiv / InputClk / LoopDivFirst
        // set  LoopSecond &  postdiv to  0x3
        //*((volatile uint16 *)u32RegAddr) = (u16RegVal) | (u32TargetClkRate * u8PostDiv  * u8OutputDiv * u8InputDiv /
        // u32InputClk / u8LoopDivFirst) | (u8PostDivReg << 8);
        SETREG16(BASE_REG_VENPLL_PA + REG_ID_03,
                 (rate * u8_post_div * u8_output_div * u8_input_div / u32_input_clk / u8_loop_div_first)
                     | (u8_post_divreg << 8));
    }
    else
    {
        clk_err("\nunsupported venpll rate %lu\n\n", rate);
        return -1;
    }
    return 0;
}

struct clk_ops sstar_venpll_ops = {
    .enable      = hal_clk_venpll_enable,
    .disable     = hal_clk_venpll_disable,
    .is_enabled  = hal_clk_venpll_is_enabled,
    .round_rate  = hal_clk_venpll_round_rate,
    .recalc_rate = hal_clk_venpll_recalc_rate,
    .set_rate    = hal_clk_venpll_set_rate,
};

static int hal_clk_riscvpll_enable(struct clk_hw *hw)
{
    /*
     * reg_mipspll_en_cpuclk = 1
     */
    SETREG16(BASE_REG_RISCVPLL_PA + REG_ID_11, BIT7);
    /*
     * reg_riscv_pll_pd=0
     */
    CLRREG16(BASE_REG_RISCVPLL_PA + REG_ID_11, BIT8);
    return 0;
}

static void hal_clk_riscvpll_disable(struct clk_hw *hw)
{
    /*
     * reg_riscv_pll_pd=1
     */
    SETREG16(BASE_REG_RISCVPLL_PA + REG_ID_11, BIT8);
}

static int hal_clk_riscvpll_is_enabled(struct clk_hw *hw)
{
    return ((INREG16(BASE_REG_RISCVPLL_PA + REG_ID_11) & BIT8) == 0x0000);
}

static unsigned long hal_clk_riscvpll_recalc_rate(struct clk_hw *hw, unsigned long parent_rate)
{
    unsigned int val         = 0;
    unsigned int rate_in_mhz = 0;

    val         = INREG16(BASE_REG_RISCVPLL_PA + REG_ID_60);
    val         = val | INREG8(BASE_REG_RISCVPLL_PA + REG_ID_61) << 16;
    rate_in_mhz = 432 * 262144 / val;

    return rate_in_mhz * 1000000;
}

static long hal_clk_riscvpll_round_rate(struct clk_hw *clk_hw, unsigned long rate, unsigned long *parent_rate)
{
    if (rate <= 528000000)
    {
        return 528000000;
    }
    else if (rate <= 608000000)
    {
        return 608000000;
    }
    else if (rate <= 654000000)
    {
        return 654000000;
    }
    else if (rate <= 700000000)
    {
        return 700000000;
    }
    else if (rate <= 730000000)
    {
        return 730000000;
    }
    else if (rate <= 776000000)
    {
        return 776000000;
    }
    else if (rate <= 805000000)
    {
        return 805000000;
    }
    else
    {
        return 852000000;
    }
}

static int hal_clk_riscvpll_set_rate(struct clk_hw *clk_hw, unsigned long rate, unsigned long parent_rate)
{
    if ((rate == 852000000) || (rate == 805000000) || (rate == 776000000) || (rate == 730000000) || (rate == 700000000)
        || (rate == 654000000) || (rate == 608000000) || (rate == 528000000))
    {
        const unsigned char u8_loop_div  = 32; // 0x102b 0x12 = 0x1088
        const unsigned char u8_input_div = 1;
        // TargetRate = 524288 * 432 / SynthCLK/ InputDIV * LoopDIV / PostDiv / OutputDiv / 2
        // SynthCLK = 524288 * 432  / (TargetRate * 2 * OutputDiv * PostDiv / LoopDIV * InputDIV)
        unsigned int u32_synth_clk = 524288 * 432 / ((rate / 1000000 * 2 * 1 * 2) / u8_loop_div * u8_input_div);

        OUTREG16(BASE_REG_RISCVPLL_PA + REG_ID_60, u32_synth_clk & 0xFFFF);
        OUTREG16(BASE_REG_RISCVPLL_PA + REG_ID_61, (u32_synth_clk >> 16) & 0xFF);
        OUTREG16(BASE_REG_RISCVPLL_PA + REG_ID_62, 0x1);
        CLRREG16(BASE_REG_RISCVPLL_PA + REG_ID_11, 0x7F);
        OUTREG16(BASE_REG_RISCVPLL_PA + REG_ID_14, 0x00);
        OUTREG16(BASE_REG_RISCVPLL_PA + REG_ID_12, 0x1088);
    }
    else
    {
        clk_err("\nunsupported venpll rate %lu\n\n", rate);
        return -1;
    }
    return 0;
}

struct clk_ops sstar_riscvpll_ops = {
    .enable      = hal_clk_riscvpll_enable,
    .disable     = hal_clk_riscvpll_disable,
    .is_enabled  = hal_clk_riscvpll_is_enabled,
    .round_rate  = hal_clk_riscvpll_round_rate,
    .recalc_rate = hal_clk_riscvpll_recalc_rate,
    .set_rate    = hal_clk_riscvpll_set_rate,
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

static int hal_clk_fuart2_synth_init(struct clk_hw *hw)
{
    /*
     * disable reset
     */
    SETREG16(BASE_REG_CLKGEN_PA + REG_ID_3D, BIT9);

    /*
     * set rate to 192M
     */
    OUTREG16(BASE_REG_CLKGEN_PA + REG_ID_3E, 0x1200);

    return 0;
}

static int hal_clk_fuart2_synth_out_enable(struct clk_hw *hw)
{
    SETREG16(BASE_REG_CLKGEN_PA + REG_ID_3D, BIT8);
    return 0;
}

static void hal_clk_fuart2_synth_out_disable(struct clk_hw *hw)
{
    CLRREG16(BASE_REG_CLKGEN_PA + REG_ID_3D, BIT8);
}

static int hal_clk_fuart2_synth_out_is_enabled(struct clk_hw *hw)
{
    return ((INREG16(BASE_REG_CLKGEN_PA + REG_ID_3D) & (0x300)) == 0x0300);
}

static unsigned long hal_clk_fuart2_synth_out_recalc_rate(struct clk_hw *hw, unsigned long parent_rate)
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

static long hal_clk_fuart2_synth_out_round_rate(struct clk_hw *clk_hw, unsigned long rate, unsigned long *parent_rate)
{
    U16 i;
    for (i = 0; i < sizeof(fuart2_synth_out_tbl) / sizeof(hal_clk_fuart_tbl); i++)
    {
        if (rate <= fuart2_synth_out_tbl[i].frequency)
        {
            return fuart2_synth_out_tbl[i].frequency;
        }
    }
    return 216000000;
}

static int hal_clk_fuart2_synth_out_set_rate(struct clk_hw *clk_hw, unsigned long rate, unsigned long parent_rate)
{
    U16 i;

    /*
     * synth clk should be (50MHz ~ 133.33MHz), and output clk = synth_clk * 24 / div(default 4),
     * so if not change div, out clk range is 300MHz ~ 800MHz
     */
    if ((rate == 216000000) || (rate == 203294000) || (rate == 192000000) || (rate == 181895000) || (rate == 172800000)
        || (rate == 164571000))
    {
        for (i = 0; i < sizeof(fuart2_synth_out_tbl) / sizeof(hal_clk_fuart_tbl); i++)
        {
            if (fuart2_synth_out_tbl[i].frequency == rate)
            {
                OUTREG16(BASE_REG_CLKGEN_PA + REG_ID_3E, fuart2_synth_out_tbl[i].val);
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

struct clk_ops sstar_fuart2_synth_out_ops = {
    .init        = hal_clk_fuart2_synth_init,
    .enable      = hal_clk_fuart2_synth_out_enable,
    .disable     = hal_clk_fuart2_synth_out_disable,
    .is_enabled  = hal_clk_fuart2_synth_out_is_enabled,
    .recalc_rate = hal_clk_fuart2_synth_out_recalc_rate,
    .round_rate  = hal_clk_fuart2_synth_out_round_rate,
    .set_rate    = hal_clk_fuart2_synth_out_set_rate,
};

static int hal_clk_fuart3_synth_init(struct clk_hw *hw)
{
    /*
     * disable reset
     */
    SETREG16(BASE_REG_CLKGEN_PA + REG_ID_24, BIT9);

    /*
     * set rate to 192M
     */
    OUTREG16(BASE_REG_CLKGEN_PA + REG_ID_25, 0x1200);

    return 0;
}

static int hal_clk_fuart3_synth_out_enable(struct clk_hw *hw)
{
    SETREG16(BASE_REG_CLKGEN_PA + REG_ID_24, BIT8);
    return 0;
}

static void hal_clk_fuart3_synth_out_disable(struct clk_hw *hw)
{
    CLRREG16(BASE_REG_CLKGEN_PA + REG_ID_24, BIT8);
}

static int hal_clk_fuart3_synth_out_is_enabled(struct clk_hw *hw)
{
    return ((INREG16(BASE_REG_CLKGEN_PA + REG_ID_24) & (0x300)) == 0x0300);
}

static unsigned long hal_clk_fuart3_synth_out_recalc_rate(struct clk_hw *hw, unsigned long parent_rate)
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

static long hal_clk_fuart3_synth_out_round_rate(struct clk_hw *clk_hw, unsigned long rate, unsigned long *parent_rate)
{
    U16 i;
    for (i = 0; i < sizeof(fuart3_synth_out_tbl) / sizeof(hal_clk_fuart_tbl); i++)
    {
        if (rate <= fuart3_synth_out_tbl[i].frequency)
        {
            return fuart3_synth_out_tbl[i].frequency;
        }
    }
    return 216000000;
}

static int hal_clk_fuart3_synth_out_set_rate(struct clk_hw *clk_hw, unsigned long rate, unsigned long parent_rate)
{
    U16 i;

    /*
     * synth clk should be (50MHz ~ 133.33MHz), and output clk = synth_clk * 24 / div(default 4),
     * so if not change div, out clk range is 300MHz ~ 800MHz
     */
    if ((rate == 216000000) || (rate == 203294000) || (rate == 192000000) || (rate == 181895000) || (rate == 172800000)
        || (rate == 164571000))
    {
        for (i = 0; i < sizeof(fuart3_synth_out_tbl) / sizeof(hal_clk_fuart_tbl); i++)
        {
            if (fuart3_synth_out_tbl[i].frequency == rate)
            {
                OUTREG16(BASE_REG_CLKGEN_PA + REG_ID_25, fuart3_synth_out_tbl[i].val);
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

struct clk_ops sstar_fuart3_synth_out_ops = {
    .init        = hal_clk_fuart3_synth_init,
    .enable      = hal_clk_fuart3_synth_out_enable,
    .disable     = hal_clk_fuart3_synth_out_disable,
    .is_enabled  = hal_clk_fuart3_synth_out_is_enabled,
    .recalc_rate = hal_clk_fuart3_synth_out_recalc_rate,
    .round_rate  = hal_clk_fuart3_synth_out_round_rate,
    .set_rate    = hal_clk_fuart3_synth_out_set_rate,
};

static int hal_clk_fuart4_synth_init(struct clk_hw *hw)
{
    /*
     * disable reset
     */
    SETREG16(BASE_REG_CLKGEN_PA + REG_ID_27, BIT9);

    /*
     * set rate to 192M
     */
    OUTREG16(BASE_REG_CLKGEN_PA + REG_ID_28, 0x1200);

    return 0;
}

static int hal_clk_fuart4_synth_out_enable(struct clk_hw *hw)
{
    SETREG16(BASE_REG_CLKGEN_PA + REG_ID_27, BIT8);
    return 0;
}

static void hal_clk_fuart4_synth_out_disable(struct clk_hw *hw)
{
    CLRREG16(BASE_REG_CLKGEN_PA + REG_ID_27, BIT8);
}

static int hal_clk_fuart4_synth_out_is_enabled(struct clk_hw *hw)
{
    return ((INREG16(BASE_REG_CLKGEN_PA + REG_ID_27) & (0x300)) == 0x0300);
}

static unsigned long hal_clk_fuart4_synth_out_recalc_rate(struct clk_hw *hw, unsigned long parent_rate)
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

static long hal_clk_fuart4_synth_out_round_rate(struct clk_hw *clk_hw, unsigned long rate, unsigned long *parent_rate)
{
    U16 i;
    for (i = 0; i < sizeof(fuart4_synth_out_tbl) / sizeof(hal_clk_fuart_tbl); i++)
    {
        if (rate <= fuart4_synth_out_tbl[i].frequency)
        {
            return fuart4_synth_out_tbl[i].frequency;
        }
    }
    return 216000000;
}

static int hal_clk_fuart4_synth_out_set_rate(struct clk_hw *clk_hw, unsigned long rate, unsigned long parent_rate)
{
    U16 i;

    /*
     * synth clk should be (50MHz ~ 133.33MHz), and output clk = synth_clk * 24 / div(default 4),
     * so if not change div, out clk range is 300MHz ~ 800MHz
     */
    if ((rate == 216000000) || (rate == 203294000) || (rate == 192000000) || (rate == 181895000) || (rate == 172800000)
        || (rate == 164571000))
    {
        for (i = 0; i < sizeof(fuart4_synth_out_tbl) / sizeof(hal_clk_fuart_tbl); i++)
        {
            if (fuart4_synth_out_tbl[i].frequency == rate)
            {
                OUTREG16(BASE_REG_CLKGEN_PA + REG_ID_28, fuart4_synth_out_tbl[i].val);
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

struct clk_ops sstar_fuart4_synth_out_ops = {
    .init        = hal_clk_fuart4_synth_init,
    .enable      = hal_clk_fuart4_synth_out_enable,
    .disable     = hal_clk_fuart4_synth_out_disable,
    .is_enabled  = hal_clk_fuart4_synth_out_is_enabled,
    .recalc_rate = hal_clk_fuart4_synth_out_recalc_rate,
    .round_rate  = hal_clk_fuart4_synth_out_round_rate,
    .set_rate    = hal_clk_fuart4_synth_out_set_rate,
};

static int hal_clk_fuart5_synth_init(struct clk_hw *hw)
{
    /*
     * disable reset
     */
    SETREG16(BASE_REG_CLKGEN_PA + REG_ID_2A, BIT9);

    /*
     * set rate to 192M
     */
    OUTREG16(BASE_REG_CLKGEN_PA + REG_ID_2B, 0x1200);

    return 0;
}

static int hal_clk_fuart5_synth_out_enable(struct clk_hw *hw)
{
    SETREG16(BASE_REG_CLKGEN_PA + REG_ID_2A, BIT8);
    return 0;
}

static void hal_clk_fuart5_synth_out_disable(struct clk_hw *hw)
{
    CLRREG16(BASE_REG_CLKGEN_PA + REG_ID_2A, BIT8);
}

static int hal_clk_fuart5_synth_out_is_enabled(struct clk_hw *hw)
{
    return ((INREG16(BASE_REG_CLKGEN_PA + REG_ID_2A) & (0x300)) == 0x0300);
}

static unsigned long hal_clk_fuart5_synth_out_recalc_rate(struct clk_hw *hw, unsigned long parent_rate)
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

static long hal_clk_fuart5_synth_out_round_rate(struct clk_hw *clk_hw, unsigned long rate, unsigned long *parent_rate)
{
    U16 i;
    for (i = 0; i < sizeof(fuart5_synth_out_tbl) / sizeof(hal_clk_fuart_tbl); i++)
    {
        if (rate <= fuart5_synth_out_tbl[i].frequency)
        {
            return fuart5_synth_out_tbl[i].frequency;
        }
    }
    return 216000000;
}

static int hal_clk_fuart5_synth_out_set_rate(struct clk_hw *clk_hw, unsigned long rate, unsigned long parent_rate)
{
    U16 i;

    /*
     * synth clk should be (50MHz ~ 133.33MHz), and output clk = synth_clk * 24 / div(default 4),
     * so if not change div, out clk range is 300MHz ~ 800MHz
     */
    if ((rate == 216000000) || (rate == 203294000) || (rate == 192000000) || (rate == 181895000) || (rate == 172800000)
        || (rate == 164571000))
    {
        for (i = 0; i < sizeof(fuart5_synth_out_tbl) / sizeof(hal_clk_fuart_tbl); i++)
        {
            if (fuart5_synth_out_tbl[i].frequency == rate)
            {
                OUTREG16(BASE_REG_CLKGEN_PA + REG_ID_2B, fuart5_synth_out_tbl[i].val);
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

struct clk_ops sstar_fuart5_synth_out_ops = {
    .init        = hal_clk_fuart5_synth_init,
    .enable      = hal_clk_fuart5_synth_out_enable,
    .disable     = hal_clk_fuart5_synth_out_disable,
    .is_enabled  = hal_clk_fuart5_synth_out_is_enabled,
    .recalc_rate = hal_clk_fuart5_synth_out_recalc_rate,
    .round_rate  = hal_clk_fuart5_synth_out_round_rate,
    .set_rate    = hal_clk_fuart5_synth_out_set_rate,
};

static int hal_clk_fuart6_synth_init(struct clk_hw *hw)
{
    /*
     * disable reset
     */
    SETREG16(BASE_REG_CLKGEN2_PA + REG_ID_00, BIT9);

    /*
     * set rate to 192M
     */
    OUTREG16(BASE_REG_CLKGEN2_PA + REG_ID_01, 0x1200);

    return 0;
}

static int hal_clk_fuart6_synth_out_enable(struct clk_hw *hw)
{
    SETREG16(BASE_REG_CLKGEN2_PA + REG_ID_00, BIT8);
    return 0;
}

static void hal_clk_fuart6_synth_out_disable(struct clk_hw *hw)
{
    CLRREG16(BASE_REG_CLKGEN2_PA + REG_ID_00, BIT8);
}

static int hal_clk_fuart6_synth_out_is_enabled(struct clk_hw *hw)
{
    return ((INREG16(BASE_REG_CLKGEN2_PA + REG_ID_00) & (0x300)) == 0x0300);
}

static unsigned long hal_clk_fuart6_synth_out_recalc_rate(struct clk_hw *hw, unsigned long parent_rate)
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

static long hal_clk_fuart6_synth_out_round_rate(struct clk_hw *clk_hw, unsigned long rate, unsigned long *parent_rate)
{
    U16 i;
    for (i = 0; i < sizeof(fuart6_synth_out_tbl) / sizeof(hal_clk_fuart_tbl); i++)
    {
        if (rate <= fuart6_synth_out_tbl[i].frequency)
        {
            return fuart6_synth_out_tbl[i].frequency;
        }
    }
    return 216000000;
}

static int hal_clk_fuart6_synth_out_set_rate(struct clk_hw *clk_hw, unsigned long rate, unsigned long parent_rate)
{
    U16 i;

    /*
     * synth clk should be (50MHz ~ 133.33MHz), and output clk = synth_clk * 24 / div(default 4),
     * so if not change div, out clk range is 300MHz ~ 800MHz
     */
    if ((rate == 216000000) || (rate == 203294000) || (rate == 192000000) || (rate == 181895000) || (rate == 172800000)
        || (rate == 164571000))
    {
        for (i = 0; i < sizeof(fuart6_synth_out_tbl) / sizeof(hal_clk_fuart_tbl); i++)
        {
            if (fuart6_synth_out_tbl[i].frequency == rate)
            {
                OUTREG16(BASE_REG_CLKGEN2_PA + REG_ID_01, fuart6_synth_out_tbl[i].val);
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

struct clk_ops sstar_fuart6_synth_out_ops = {
    .init        = hal_clk_fuart6_synth_init,
    .enable      = hal_clk_fuart6_synth_out_enable,
    .disable     = hal_clk_fuart6_synth_out_disable,
    .is_enabled  = hal_clk_fuart6_synth_out_is_enabled,
    .recalc_rate = hal_clk_fuart6_synth_out_recalc_rate,
    .round_rate  = hal_clk_fuart6_synth_out_round_rate,
    .set_rate    = hal_clk_fuart6_synth_out_set_rate,
};

static int hal_clk_fuart7_synth_init(struct clk_hw *hw)
{
    /*
     * disable reset
     */
    SETREG16(BASE_REG_CLKGEN2_PA + REG_ID_03, BIT9);

    /*
     * set rate to 192M
     */
    OUTREG16(BASE_REG_CLKGEN2_PA + REG_ID_04, 0x1200);

    return 0;
}

static int hal_clk_fuart7_synth_out_enable(struct clk_hw *hw)
{
    SETREG16(BASE_REG_CLKGEN2_PA + REG_ID_03, BIT8);
    return 0;
}

static void hal_clk_fuart7_synth_out_disable(struct clk_hw *hw)
{
    CLRREG16(BASE_REG_CLKGEN2_PA + REG_ID_03, BIT8);
}

static int hal_clk_fuart7_synth_out_is_enabled(struct clk_hw *hw)
{
    return ((INREG16(BASE_REG_CLKGEN2_PA + REG_ID_03) & (0x300)) == 0x0300);
}

static unsigned long hal_clk_fuart7_synth_out_recalc_rate(struct clk_hw *hw, unsigned long parent_rate)
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

static long hal_clk_fuart7_synth_out_round_rate(struct clk_hw *clk_hw, unsigned long rate, unsigned long *parent_rate)
{
    U16 i;
    for (i = 0; i < sizeof(fuart7_synth_out_tbl) / sizeof(hal_clk_fuart_tbl); i++)
    {
        if (rate <= fuart7_synth_out_tbl[i].frequency)
        {
            return fuart7_synth_out_tbl[i].frequency;
        }
    }
    return 216000000;
}

static int hal_clk_fuart7_synth_out_set_rate(struct clk_hw *clk_hw, unsigned long rate, unsigned long parent_rate)
{
    U16 i;

    /*
     * synth clk should be (50MHz ~ 133.33MHz), and output clk = synth_clk * 24 / div(default 4),
     * so if not change div, out clk range is 300MHz ~ 800MHz
     */
    if ((rate == 216000000) || (rate == 203294000) || (rate == 192000000) || (rate == 181895000) || (rate == 172800000)
        || (rate == 164571000))
    {
        for (i = 0; i < sizeof(fuart7_synth_out_tbl) / sizeof(hal_clk_fuart_tbl); i++)
        {
            if (fuart7_synth_out_tbl[i].frequency == rate)
            {
                OUTREG16(BASE_REG_CLKGEN2_PA + REG_ID_04, fuart7_synth_out_tbl[i].val);
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

struct clk_ops sstar_fuart7_synth_out_ops = {
    .init        = hal_clk_fuart7_synth_init,
    .enable      = hal_clk_fuart7_synth_out_enable,
    .disable     = hal_clk_fuart7_synth_out_disable,
    .is_enabled  = hal_clk_fuart7_synth_out_is_enabled,
    .recalc_rate = hal_clk_fuart7_synth_out_recalc_rate,
    .round_rate  = hal_clk_fuart7_synth_out_round_rate,
    .set_rate    = hal_clk_fuart7_synth_out_set_rate,
};

static int hal_clk_gmacpll_init(struct clk_hw *hw)
{
    /*
     * turn on xtal HV
     */
    OUTREG8(BASE_REG_XTAL_ATOP_PA + REG_ID_19, 0x00);

    return 0;
}

static int hal_clk_gmacpll_enable(struct clk_hw *hw)
{
    /*
     * turn on gmacpll, wait around 200us for PLL to be stable
     */
    CLRREG16(BASE_REG_GMACPLLTOP_PA + REG_ID_01, BIT0);
    udelay(200);

    return 0;
}

static void hal_clk_gmacpll_disable(struct clk_hw *hw)
{
    SETREG16(BASE_REG_GMACPLLTOP_PA + REG_ID_01, BIT0);
}

static int hal_clk_gmacpll_is_enabled(struct clk_hw *hw)
{
    return ((INREG16(BASE_REG_GMACPLLTOP_PA + REG_ID_01) & (BIT0)) == 0x0000);
}

static unsigned long hal_clk_gmacpll_recalc_rate(struct clk_hw *hw, unsigned long parent_rate)
{
    unsigned long ret = 0;

    ret = 0xB2D05E00;

    return ret;
}

struct clk_ops sstar_gmacpll_ops = {
    .init        = hal_clk_gmacpll_init,
    .enable      = hal_clk_gmacpll_enable,
    .disable     = hal_clk_gmacpll_disable,
    .is_enabled  = hal_clk_gmacpll_is_enabled,
    .recalc_rate = hal_clk_gmacpll_recalc_rate,
};

static int hal_clk_upll_960m_enable(struct clk_hw *hw)
{
    CLRREG8(BASE_REG_UPLL2_PA + REG_ID_00, BIT1);
    return 0;
}

static void hal_clk_upll_960m_disable(struct clk_hw *hw)
{
    SETREG8(BASE_REG_UPLL2_PA + REG_ID_00, BIT1);
}

static int hal_clk_upll_960m_is_enabled(struct clk_hw *hw)
{
    return ((INREG16(BASE_REG_UPLL2_PA + REG_ID_00) & (BIT1)) == 0x0000);
}

static unsigned long hal_clk_upll_960m_recalc_rate(struct clk_hw *hw, unsigned long parent_rate)
{
    int ret = 0;

    ret = 960000000;

    return ret;
}

struct clk_ops sstar_upll_960m_ops = {
    .enable      = hal_clk_upll_960m_enable,
    .disable     = hal_clk_upll_960m_disable,
    .is_enabled  = hal_clk_upll_960m_is_enabled,
    .recalc_rate = hal_clk_upll_960m_recalc_rate,
};

static int hal_clk_upll_480m_enable(struct clk_hw *hw)
{
    SETREG8(BASE_REG_UPLL2_PA + REG_ID_07, BIT4);
    return 0;
}

static void hal_clk_upll_480m_disable(struct clk_hw *hw)
{
    CLRREG8(BASE_REG_UPLL2_PA + REG_ID_07, BIT4);
}

static int hal_clk_upll_480m_is_enabled(struct clk_hw *hw)
{
    return ((INREG16(BASE_REG_UPLL2_PA + REG_ID_07) & (BIT4)) == 0x0010);
}

static unsigned long hal_clk_upll_480m_recalc_rate(struct clk_hw *hw, unsigned long parent_rate)
{
    int ret = 0;

    ret = 480000000;

    return ret;
}

struct clk_ops sstar_upll_480m_ops = {
    .enable      = hal_clk_upll_480m_enable,
    .disable     = hal_clk_upll_480m_disable,
    .is_enabled  = hal_clk_upll_480m_is_enabled,
    .recalc_rate = hal_clk_upll_480m_recalc_rate,
};

static int hal_clk_upll_384m_enable(struct clk_hw *hw)
{
    SETREG8(BASE_REG_UPLL2_PA + REG_ID_07, BIT0);
    return 0;
}

static void hal_clk_upll_384m_disable(struct clk_hw *hw)
{
    CLRREG8(BASE_REG_UPLL2_PA + REG_ID_07, BIT0);
}

static int hal_clk_upll_384m_is_enabled(struct clk_hw *hw)
{
    return ((INREG16(BASE_REG_UPLL2_PA + REG_ID_07) & (BIT0)) == 0x0001);
}

static unsigned long hal_clk_upll_384m_recalc_rate(struct clk_hw *hw, unsigned long parent_rate)
{
    int ret = 0;

    ret = 384000000;

    return ret;
}

struct clk_ops sstar_upll_384m_ops = {
    .enable      = hal_clk_upll_384m_enable,
    .disable     = hal_clk_upll_384m_disable,
    .is_enabled  = hal_clk_upll_384m_is_enabled,
    .recalc_rate = hal_clk_upll_384m_recalc_rate,
};

static int hal_clk_upll_320m_enable(struct clk_hw *hw)
{
    CLRREG8(BASE_REG_UPLL2_PA + REG_ID_00, BIT5);
    return 0;
}

static void hal_clk_upll_320m_disable(struct clk_hw *hw)
{
    SETREG8(BASE_REG_UPLL2_PA + REG_ID_00, BIT5);
}

static int hal_clk_upll_320m_is_enabled(struct clk_hw *hw)
{
    return ((INREG16(BASE_REG_UPLL2_PA + REG_ID_00) & (BIT5)) == 0x0000);
}

static unsigned long hal_clk_upll_320m_recalc_rate(struct clk_hw *hw, unsigned long parent_rate)
{
    int ret = 0;
    ret     = 320000000;
    return ret;
}

struct clk_ops sstar_upll_320m_ops = {
    .enable      = hal_clk_upll_320m_enable,
    .disable     = hal_clk_upll_320m_disable,
    .is_enabled  = hal_clk_upll_320m_is_enabled,
    .recalc_rate = hal_clk_upll_320m_recalc_rate,
};

static int hal_clk_upll_20m_enable(struct clk_hw *hw)
{
    CLRREG8(BASE_REG_UPLL2_PA + REG_ID_00, BIT4);
    return 0;
}

static void hal_clk_upll_20m_disable(struct clk_hw *hw)
{
    SETREG8(BASE_REG_UPLL2_PA + REG_ID_00, BIT4);
}

static int hal_clk_upll_20m_is_enabled(struct clk_hw *hw)
{
    return ((INREG16(BASE_REG_UPLL2_PA + REG_ID_00) & (BIT4)) == 0x0000);
}

static unsigned long hal_clk_upll_20m_recalc_rate(struct clk_hw *hw, unsigned long parent_rate)
{
    int ret = 0;

    ret = 20000000;

    return ret;
}

struct clk_ops sstar_upll_20m_ops = {
    .enable      = hal_clk_upll_20m_enable,
    .disable     = hal_clk_upll_20m_disable,
    .is_enabled  = hal_clk_upll_20m_is_enabled,
    .recalc_rate = hal_clk_upll_20m_recalc_rate,
};

static int hal_clk_ipupll_init(struct clk_hw *hw)
{
    /*
     * set ipupll to 800M
     * set synthesizer = 97.88MHz
     */
    OUTREG16(BASE_REG_IPUPLL_PA + REG_ID_60, 0x1EB8);
    OUTREG8(BASE_REG_IPUPLL_PA + REG_ID_61, 0x45);

    /*
     * Update IPU frequency synthesizer
     */
    OUTREG16(BASE_REG_IPUPLL_PA + REG_ID_62, 0x0001);

    /*
     * set pll vco = 1600MHz
     */
    OUTREG16(BASE_REG_IPUPLL_PA + REG_ID_12, 0x0088);

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

static int hal_clk_ipupll_set_rate(struct clk_hw *hw, unsigned long rate, unsigned long parent_rate)
{
    int ret = 0;
    U32 regset;
    u8  vco_div = 0;

    vco_div = (INREG16(BASE_REG_IPUPLL_PA + REG_ID_12) & (BIT12)) ? 4 : 2;
    regset  = (432 << 24) / (vco_div) / ((U32)rate / 1000000);
    clk_dbg("[%s] rate = %d regset = 0x%x \r\n", __func__, (int)rate, regset);

    OUTREG8(BASE_REG_IPUPLL_PA + REG_ID_61, (regset & 0xFFFF0000) >> 16);
    OUTREG16(BASE_REG_IPUPLL_PA + REG_ID_60, (regset & 0xFFFF));

    OUTREG8(BASE_REG_IPUPLL_PA + REG_ID_62, 0x01);
    return ret;
}

static long hal_clk_ipupll_round_rate(struct clk_hw *hw, unsigned long rate, unsigned long *parent_rate)
{
    unsigned long round_rate = 0;

    round_rate = rate;

    return round_rate;
}

static unsigned long hal_clk_ipupll_recalc_rate(struct clk_hw *hw, unsigned long parent_rate)
{
    int recalc_rate = 0;
    U32 reglow, reghigh;
    u8  vco_div = 0;

    vco_div = (INREG16(BASE_REG_IPUPLL_PA + REG_ID_12) & (BIT12)) ? 4 : 2;
    reglow  = INREG16(BASE_REG_IPUPLL_PA + REG_ID_60);
    reghigh = INREG16(BASE_REG_IPUPLL_PA + REG_ID_61);
    reghigh = reghigh << 16;

    recalc_rate = (((432 << 24) / (vco_div) / (reghigh | reglow)) * 1000000);
    clk_dbg("[%s]recalc_rate %d \r\n", __func__, (int)recalc_rate);

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

static int hal_clk_bachpll_384m_init(struct clk_hw *hw)
{
    /*
     * enable 384MHz clock
     */
    OUTREG16(BASE_REG_BACHPLL_PA + REG_ID_00, 0x11F0);
    OUTREG16(BASE_REG_BACHPLL_PA + REG_ID_00, 0x11B0);
    OUTREG16(BASE_REG_BACHPLL_PA + REG_ID_00, 0x11B0);
    OUTREG16(BASE_REG_BACHPLL_PA + REG_ID_00, 0x1080);
    OUTREG16(BASE_REG_BACHPLL_PA + REG_ID_01, 0x1498);
    OUTREG16(BASE_REG_BACHPLL_PA + REG_ID_07, 0x0011);
    OUTREG16(BASE_REG_BACHPLL_PA + REG_ID_08, 0x0081);

    /*
     * wait around 100us for PLL to be stable
     */
    udelay(100);

    return 0;
}

static int hal_clk_bachpll_384m_enable(struct clk_hw *hw)
{
    CLRREG8(BASE_REG_BACHPLL_PA + REG_ID_00, BIT1);
    return 0;
}

static void hal_clk_bachpll_384m_disable(struct clk_hw *hw)
{
    SETREG8(BASE_REG_BACHPLL_PA + REG_ID_00, BIT1);
}

static int hal_clk_bachpll_384m_is_enabled(struct clk_hw *hw)
{
    return ((INREG16(BASE_REG_BACHPLL_PA + REG_ID_00) & (BIT1)) == 0x0000);
}

static int hal_clk_bachpll_384m_set_rate(struct clk_hw *hw, unsigned long rate, unsigned long parent_rate)
{
    int ret          = 0;
    int post_div1    = 0;
    int post_div2    = 0;
    int syn_reg_code = 0;
    int loop_div     = 0;

    switch (rate)
    {
        case 147456000:
            post_div1    = 2;
            post_div2    = 2;
            syn_reg_code = 0x232800;
            loop_div     = 6;
            break;
        case 196608000:
            post_div1    = 2;
            post_div2    = 2;
            syn_reg_code = 0x1A5E00;
            loop_div     = 6;
            break;
        case 22579200:
            post_div1    = 16;
            post_div2    = 2;
            syn_reg_code = 0x1CB2F0;
            loop_div     = 6;
            break;
        case 32768000:
            post_div1    = 12;
            post_div2    = 2;
            syn_reg_code = 0x1A5E00;
            loop_div     = 6;
            break;
        case 36864000:
            post_div1    = 12;
            post_div2    = 2;
            syn_reg_code = 0x177000;
            loop_div     = 6;
            break;
        case 49152000:
            post_div1    = 8;
            post_div2    = 2;
            syn_reg_code = 0x1A5E00;
            loop_div     = 6;
            break;
        case 76800000:
            post_div1    = 6;
            post_div2    = 2;
            syn_reg_code = 0x168000;
            loop_div     = 6;
            break;
        case 307200000:
            post_div1    = 1;
            post_div2    = 2;
            syn_reg_code = 0x21C000;
            loop_div     = 6;
            break;
        case 384000000:
            post_div1    = 1;
            post_div2    = 2;
            syn_reg_code = 0x1B0000;
            loop_div     = 6;
            break;
        default:
            post_div1    = 0;
            post_div2    = 0;
            syn_reg_code = 0;
            loop_div     = 0;
            break;
    }

    /*
     * init set
     */
    OUTREGMSK16(BASE_REG_BACHPLL_PA + REG_ID_00, BIT12, BIT12 | BIT13 | BIT14);
    OUTREGMSK16(BASE_REG_BACHPLL_PA + REG_ID_00, 0, BIT7);
    OUTREGMSK16(BASE_REG_BACHPLL_PA + REG_ID_01, BIT3, BIT1 | BIT2 | BIT3);
    OUTREGMSK16(BASE_REG_BACHPLL_PA + REG_ID_01, BIT7, BIT7);
    OUTREGMSK16(BASE_REG_BACHPLL_PA + REG_ID_01, BIT9 | BIT10, 0xFF00);
    OUTREGMSK16(BASE_REG_BACHPLL_PA + REG_ID_01, 0, BIT4 | BIT5);

    /*
     * loop_div_set
     */
    OUTREGMSK16(BASE_REG_BACHPLL_PA + REG_ID_01, loop_div << 8, 0xFF << 8);

    /*
     * post_div1
     */
    OUTREGMSK16(BASE_REG_BACHPLL_PA + REG_ID_08, post_div1 << 0, 0x3F << 0);

    /*
     * post_div2
     */
    OUTREGMSK16(BASE_REG_BACHPLL_PA + REG_ID_08, post_div2 << 6, BIT6 | BIT7);

    /*
     * synth set
     */
    OUTREGMSK16(BASE_REG_BACHPLL_PA + REG_ID_0B, syn_reg_code & 0xFFFF, 0xFFFF);
    OUTREGMSK16(BASE_REG_BACHPLL_PA + REG_ID_0C, (syn_reg_code >> 16) & 0xFF & 0xFFFF, 0xFF);

    /*
     * open power
     */
    CLRREG8(BASE_REG_BACHPLL_PA + REG_ID_00, BIT1);

    return ret;
}

static long hal_clk_bachpll_384m_round_rate(struct clk_hw *hw, unsigned long rate, unsigned long *parent_rate)
{
    return rate;
}

static unsigned long hal_clk_bachpll_384m_recalc_rate(struct clk_hw *hw, unsigned long parent_rate)
{
    unsigned long recalc_rate = 0;
    U32           regdat      = 0;

    regdat = INREG16(BASE_REG_BACHPLL_PA + REG_ID_0B);
    regdat |= (INREG16(BASE_REG_BACHPLL_PA + REG_ID_0C) << 16);

    switch (regdat)
    {
        case 0x1CB2F0:
            recalc_rate = 22579200;
            break;
        case 0x1A5E00:
            regdat = INREG16(BASE_REG_BACHPLL_PA + REG_ID_08);
            regdat &= 0x3F;
            if (regdat == 12)
                recalc_rate = 32768000;
            else if (regdat == 8)
                recalc_rate = 49152000;

            break;
        case 0x177000:
            recalc_rate = 36864000;
            break;
        case 0x168000:
            recalc_rate = 76800000;
            break;
        case 0x21C000:
            recalc_rate = 307200000;
            break;
        case 0x1B0000:
            recalc_rate = 384000000;
            break;
        default:
            recalc_rate = 0;
            break;
    }

    return recalc_rate;
}

struct clk_ops sstar_bachpll_384m_ops = {
    .init        = hal_clk_bachpll_384m_init,
    .enable      = hal_clk_bachpll_384m_enable,
    .disable     = hal_clk_bachpll_384m_disable,
    .is_enabled  = hal_clk_bachpll_384m_is_enabled,
    .set_rate    = hal_clk_bachpll_384m_set_rate,
    .round_rate  = hal_clk_bachpll_384m_round_rate,
    .recalc_rate = hal_clk_bachpll_384m_recalc_rate,
};

static int hal_clk_isppll_init(struct clk_hw *hw)
{
    OUTREG16(BASE_REG_ISPPLL_PA + REG_ID_02, 0x0100);
    OUTREG16(BASE_REG_ISPPLL_PA + REG_ID_03, 0x0016);

    return 0;
}

static int hal_clk_isppll_enable(struct clk_hw *hw)
{
    OUTREG16(BASE_REG_ISPPLL_PA + REG_ID_01, 0x0000);
    return 0;
}

static void hal_clk_isppll_disable(struct clk_hw *hw)
{
    OUTREG16(BASE_REG_ISPPLL_PA + REG_ID_01, 0x0100);
}

static int hal_clk_isppll_is_enabled(struct clk_hw *hw)
{
    return ((INREG16(BASE_REG_ISPPLL_PA + REG_ID_01) & (0x0100)) == 0x0000);
}

static int hal_clk_isppll_set_rate(struct clk_hw *hw, unsigned long rate, unsigned long parent_rate)
{
    switch (rate)
    {
        case 696000000:
        case 672000000:
        case 648000000:
        case 624000000:
        case 600000000:
        case 576000000:
        case 552000000:
        case 528000000:
        case 504000000:
        case 480000000:
        case 456000000:
        case 432000000:
        case 408000000:
        case 384000000:
        case 360000000:
        case 336000000:
        case 312000000:
        {
            const unsigned int u32_input_clk = 24000000;
            // PLL input-divider control:
            // 2'b00: /1,
            // 2'b01: /2
            const unsigned char u8_input_divreg = 0x0; // 1
            const unsigned char u8_input_div    = 1;
            // First-Stage loop-divider control
            // 000 for /1,
            // 001 for /2,
            // 010 for /4,
            // 011 for /8
            const unsigned char u8_loop_div_firstreg = 0x1;
            const unsigned char u8_loop_div_first    = 2;
            // 1XX for /1,
            // 011 for /5,
            // 010 for /4,
            // 001 for /3,
            // 000 for /2
            const unsigned char u8_post_divreg = 0x0; // 2
            const unsigned char u8_post_div    = 0x2;
            const unsigned char u8_output_div  = 0x1; // 1
            //
            // u32RegAddr = ISP_PLL_CLK_REG_BASE + REG_ID_01;
            //*((volatile uint16 *)u32RegAddr) = 0x0;

            OUTREG16(BASE_REG_ISPPLL_PA + REG_ID_01, 0x0);

            // u32RegAddr = ISP_PLL_CLK_REG_BASE + REG_ID_02;
            // u16RegVal = *((volatile uint16 *)u32RegAddr);
            // u16RegVal &= ~(0x730);

            CLRREG16(BASE_REG_ISPPLL_PA + REG_ID_02, 0x730);

            // set inputdiv &  loopdivfirst to  0x2
            //*((volatile uint16 *)u32RegAddr)= u16RegVal | (u8InputDivReg << 4) | (u8LoopDivFirstReg << 8);
            SETREG16(BASE_REG_ISPPLL_PA + REG_ID_02, (u8_input_divreg << 4) | (u8_loop_div_firstreg << 8));

            // u32RegAddr = ISP_PLL_CLK_REG_BASE + REG_ID_03;
            // u16RegVal = *((volatile uint16 *)u32RegAddr);
            // u16RegVal &= ~(0x7FF);
            CLRREG16(BASE_REG_ISPPLL_PA + REG_ID_03, 0x7FF);
            // Targetrate = (InputClk / InputDiv * (LoopDivFirst * LoopSecond)) / PostDiv / OutputDiv
            // compute LoopSecond = Targetrate * PostDiv * OutputDiv * InputDiv / InputClk / LoopDivFirst
            // set  LoopSecond &  postdiv to  0x3
            //*((volatile uint16 *)u32RegAddr) = (u16RegVal) | (u32TargetClkRate * u8PostDiv  * u8OutputDiv * u8InputDiv
            ///
            // u32InputClk / u8LoopDivFirst) | (u8PostDivReg << 8);
            SETREG16(BASE_REG_ISPPLL_PA + REG_ID_03,
                     (rate * u8_post_div * u8_output_div * u8_input_div / u32_input_clk / u8_loop_div_first)
                         | (u8_post_divreg << 8));
            clk_dbg("[%s] rate = %lu \r\n", __func__, rate);
        }
        break;
        default:
        {
            clk_err("\nunsupported isppll rate %lu\n\n", rate);
            return -1;
        }
        break;
    }
    return 0;
}

static long hal_clk_isppll_round_rate(struct clk_hw *hw, unsigned long rate, unsigned long *parent_rate)
{
    unsigned long return_rate;

    return_rate = (rate <= 312000000)   ? (312000000)
                  : (rate <= 336000000) ? (336000000)
                  : (rate <= 360000000) ? (360000000)
                  : (rate <= 384000000) ? (384000000)
                  : (rate <= 408000000) ? (408000000)
                  : (rate <= 432000000) ? (432000000)
                  : (rate <= 456000000) ? (456000000)
                  : (rate <= 480000000) ? (480000000)
                  : (rate <= 504000000) ? (504000000)
                  : (rate <= 528000000) ? (528000000)
                  : (rate <= 552000000) ? (552000000)
                  : (rate <= 576000000) ? (576000000)
                  : (rate <= 600000000) ? (600000000)
                  : (rate <= 624000000) ? (624000000)
                  : (rate <= 648000000) ? (648000000)
                  : (rate <= 672000000) ? (672000000)
                                        : (696000000);
    return return_rate;
}

static unsigned long hal_clk_isppll_recalc_rate(struct clk_hw *hw, unsigned long parent_rate)
{
    int input_div         = 0;
    int output_div        = 0;
    int loop_div_f        = 0;
    int loop_div_s        = 0;
    int recalc_rate       = 0;
    int input_div_arr[2]  = {1, 2};
    int output_div_arr[8] = {2, 3, 4, 5, 1, 1, 1, 1};
    int loop_div_arr[4]   = {1, 2, 4, 8};

    input_div  = INREGMSK16(BASE_REG_ISPPLL_PA + REG_ID_02, BIT4 | BIT5) >> 4;
    output_div = INREGMSK16(BASE_REG_ISPPLL_PA + REG_ID_03, BIT8 | BIT9 | BIT10) >> 8;
    loop_div_f = INREGMSK16(BASE_REG_ISPPLL_PA + REG_ID_02, BIT8 | BIT9 | BIT10) >> 8;
    loop_div_s = INREG8(BASE_REG_ISPPLL_PA + REG_ID_03);

    recalc_rate = (24 / input_div_arr[input_div]) * (loop_div_arr[loop_div_f] * loop_div_s) / output_div_arr[output_div]
                  * 1000000;
    clk_dbg("[%s]recalc_rate %d \r\n", __func__, (int)recalc_rate);

    return recalc_rate;
}

struct clk_ops sstar_isppll_ops = {
    .init        = hal_clk_isppll_init,
    .enable      = hal_clk_isppll_enable,
    .disable     = hal_clk_isppll_disable,
    .is_enabled  = hal_clk_isppll_is_enabled,
    .set_rate    = hal_clk_isppll_set_rate,
    .round_rate  = hal_clk_isppll_round_rate,
    .recalc_rate = hal_clk_isppll_recalc_rate,
};

static int hal_clk_sd_1x_p_init(struct clk_hw *hw)
{
    OUTREG16(BASE_REG_SDPLL_PA + REG_ID_03, 0x0000);
    OUTREG16(BASE_REG_SDPLL_PA + REG_ID_1C, 0x0000);
    OUTREG16(BASE_REG_SDPLL_PA + REG_ID_68, 0x0000);
    OUTREG16(BASE_REG_SDPLL_PA + REG_ID_69, 0x0040);
    OUTREG16(BASE_REG_SDPLL_PA + REG_ID_6A, 0x0000);
    OUTREG16(BASE_REG_SDPLL_PA + REG_ID_6B, 0x0000);
    OUTREG16(BASE_REG_SDPLL_PA + REG_ID_6C, 0x0000);
    OUTREG16(BASE_REG_SDPLL_PA + REG_ID_6D, 0x0000);
    OUTREG16(BASE_REG_SDPLL_PA + REG_ID_70, 0x0000);
    OUTREG16(BASE_REG_SDPLL_PA + REG_ID_71, 0xFFFF);

    return 0;
}

static int hal_clk_sd_1x_p_enable(struct clk_hw *hw)
{
    CLRREG8(BASE_REG_SDPLL_PA + REG_ID_04, BIT7);

    return 0;
}

static void hal_clk_sd_1x_p_disable(struct clk_hw *hw)
{
    SETREG8(BASE_REG_SDPLL_PA + REG_ID_04, BIT7);
}

static int hal_clk_sd_1x_p_is_enabled(struct clk_hw *hw)
{
    return ((INREG16(BASE_REG_SDPLL_PA + REG_ID_04) & (BIT7)) == 0x0000);
}

static int hal_clk_sd_1x_p_set_rate(struct clk_hw *hw, unsigned long rate, unsigned long parent_rate)
{
    int ret = 0;

    /*
     * timing
     */

    /*
     * SDR50 and SDR104
     */
    if (rate == 6)
    {
        OUTREG16(BASE_REG_SDPLL_PA + REG_ID_1C, 0x0100);
        OUTREG16(BASE_REG_SDPLL_PA + REG_ID_68, 0x0001);
        OUTREG16(BASE_REG_SDPLL_PA + REG_ID_6A, 0x0001);
        OUTREG16(BASE_REG_SDPLL_PA + REG_ID_6B, 0x0413);
        OUTREG16(BASE_REG_SDPLL_PA + REG_ID_6D, 0x0000);
        OUTREG16(BASE_REG_SDPLL_PA + REG_ID_70, 0x0D00);
        OUTREG16(BASE_REG_SDPLL_PA + REG_ID_71, 0x4000);
    }
    /*
     * DDR50
     */
    else if (rate == 7)
    {
        OUTREG16(BASE_REG_SDPLL_PA + REG_ID_1C, 0x0200);
        OUTREG16(BASE_REG_SDPLL_PA + REG_ID_68, 0x0001);
        OUTREG16(BASE_REG_SDPLL_PA + REG_ID_6A, 0x0001);
        OUTREG16(BASE_REG_SDPLL_PA + REG_ID_6B, 0x0213);
        OUTREG16(BASE_REG_SDPLL_PA + REG_ID_6D, 0x0001);
        OUTREG16(BASE_REG_SDPLL_PA + REG_ID_70, 0x0000);
        OUTREG16(BASE_REG_SDPLL_PA + REG_ID_71, 0xFFFF);
    }
    /*
     * HS
     */
    else if (rate == 2)
    {
        OUTREG16(BASE_REG_SDPLL_PA + REG_ID_1C, 0x0000);
        OUTREG16(BASE_REG_SDPLL_PA + REG_ID_68, 0x0000);
        OUTREG16(BASE_REG_SDPLL_PA + REG_ID_6A, 0x0000);
        OUTREG16(BASE_REG_SDPLL_PA + REG_ID_6B, 0x0000);
        OUTREG16(BASE_REG_SDPLL_PA + REG_ID_6D, 0x0000);
        OUTREG16(BASE_REG_SDPLL_PA + REG_ID_70, 0x0000);
        OUTREG16(BASE_REG_SDPLL_PA + REG_ID_71, 0xFFFF);
    }

    /*
     * if freq = 200M
     */
    if (rate == 200000000)
    {
        OUTREG16(BASE_REG_SDPLL_PA + REG_ID_04, 0x0000);
        CLRREG8(BASE_REG_SDPLL_PA + REG_ID_05, BIT2 | BIT1 | BIT0);
        OUTREG16(BASE_REG_SDPLL_PA + REG_ID_19, 0x0021);
        OUTREG16(BASE_REG_SDPLL_PA + REG_ID_18, 0x3B13);
        SETREG8(BASE_REG_SDPLL_PA + REG_ID_04, BIT4 | BIT1);
        SETREG8(BASE_REG_SDPLL_PA + REG_ID_05, 0);
    }
    /*
     * if freq = 100M
     */
    else if (rate == 100000000)
    {
        OUTREG16(BASE_REG_SDPLL_PA + REG_ID_04, 0x0000);
        CLRREG8(BASE_REG_SDPLL_PA + REG_ID_05, BIT2 | BIT1 | BIT0);
        OUTREG16(BASE_REG_SDPLL_PA + REG_ID_19, 0x0021);
        OUTREG16(BASE_REG_SDPLL_PA + REG_ID_18, 0x3B13);
        SETREG8(BASE_REG_SDPLL_PA + REG_ID_04, BIT3 | BIT0);
        SETREG8(BASE_REG_SDPLL_PA + REG_ID_05, 0);
    }
    /*
     * if freq = 50M
     */
    else if (rate == 48000000)
    {
        OUTREG16(BASE_REG_SDPLL_PA + REG_ID_04, 0x0000);
        CLRREG8(BASE_REG_SDPLL_PA + REG_ID_05, BIT2 | BIT1 | BIT0);
        OUTREG16(BASE_REG_SDPLL_PA + REG_ID_19, 0x0022);
        OUTREG16(BASE_REG_SDPLL_PA + REG_ID_18, 0x8F5C);
        SETREG8(BASE_REG_SDPLL_PA + REG_ID_04, BIT4 | BIT1);
        SETREG8(BASE_REG_SDPLL_PA + REG_ID_05, 0);
    }

    return ret;
}

static long hal_clk_sd_1x_p_round_rate(struct clk_hw *hw, unsigned long rate, unsigned long *parent_rate)
{
    unsigned long round_rate = 0;
    round_rate               = rate;
    return round_rate;
}

static unsigned long hal_clk_sd_1x_p_recalc_rate(struct clk_hw *hw, unsigned long parent_rate)
{
    U16 clk_val_1  = 0;
    U16 clk_val_2  = 0;
    U16 input_clk  = 0;
    U16 output_clk = 0;
    U8  fb_div_val = 0;
    U8  fb_div     = 0;
    U8  p_div_val  = 0;
    U8  p_div      = 0;

    clk_val_1  = INREG16(BASE_REG_SDPLL_PA + REG_ID_18);
    clk_val_2  = INREG16(BASE_REG_SDPLL_PA + REG_ID_19);
    fb_div_val = INREG8(BASE_REG_SDPLL_PA + REG_ID_04) & (BIT0 | BIT1);
    p_div_val  = INREG8(BASE_REG_SDPLL_PA + REG_ID_05) & (BIT0 | BIT1 | BIT2);

    if (fb_div_val == 0)
        fb_div = 1;
    else
        fb_div = 2 << (fb_div_val - 1);

    if (p_div_val < 2)
        p_div = 2;
    else if (p_div_val == 2)
        p_div = 4;
    else if (p_div_val == 3)
        p_div = 6;
    else if (p_div_val == 4)
        p_div = 8;
    else if (p_div_val == 5)
        p_div = 10;
    else if (p_div_val > 5)
        p_div = 16;
    else
        p_div = 1;

    if ((clk_val_2 == 0x22) && clk_val_1 == 0x8F5C)
        input_clk = 100;
    if ((clk_val_2 == 0x21) && clk_val_1 == 0x3B13)
        input_clk = 104;

    /*
     * output_clk = input_clk * (2 ^ fb_div) / p_div
     */
    output_clk = input_clk * fb_div / p_div;

    return output_clk;
}

struct clk_ops sstar_sd_1x_p_ops = {
    .init        = hal_clk_sd_1x_p_init,
    .enable      = hal_clk_sd_1x_p_enable,
    .disable     = hal_clk_sd_1x_p_disable,
    .is_enabled  = hal_clk_sd_1x_p_is_enabled,
    .set_rate    = hal_clk_sd_1x_p_set_rate,
    .round_rate  = hal_clk_sd_1x_p_round_rate,
    .recalc_rate = hal_clk_sd_1x_p_recalc_rate,
};

static int hal_clk_fcie_1x_p_init(struct clk_hw *hw)
{
    SETREG8(BASE_REG_EMMCPLL_PA + REG_ID_06, BIT0);
    CLRREG8(BASE_REG_EMMCPLL_PA + REG_ID_06, BIT0);

    return 0;
}

static int hal_clk_fcie_1x_p_enable(struct clk_hw *hw)
{
    CLRREG8(BASE_REG_EMMCPLL_PA + REG_ID_04, BIT5);

    return 0;
}

static void hal_clk_fcie_1x_p_disable(struct clk_hw *hw)
{
    /*
     * do not disable for stability reasons
     */
    // SETREG8(GET_REG_ADDR8(BASE_REG_RIU_PA, 0x00141A08), BIT(5));
}

static int hal_clk_fcie_1x_p_is_enabled(struct clk_hw *hw)
{
    return ((INREG16(BASE_REG_EMMCPLL_PA + REG_ID_04) & (BIT5)) == 0x0000);
}

static int hal_clk_fcie_1x_p_set_rate(struct clk_hw *hw, unsigned long rate, unsigned long parent_rate)
{
    /*
     * SDR
     */
    if (rate == 2)
    {
        OUTREG16(BASE_REG_EMMCPLL_PA + REG_ID_1C, 0x0000);
        OUTREG16(BASE_REG_EMMCPLL_PA + REG_ID_1D, 0x0000);
        OUTREG16(BASE_REG_EMMCPLL_PA + REG_ID_20, 0x0000);
        OUTREG16(BASE_REG_EMMCPLL_PA + REG_ID_68, 0x0000);
        OUTREG16(BASE_REG_EMMCPLL_PA + REG_ID_69, 0x0000);
        OUTREG16(BASE_REG_EMMCPLL_PA + REG_ID_6B, 0x0000);
        SETREG16(BASE_REG_EMMCPLL_PA + REG_ID_6C, BIT1);
        OUTREG16(BASE_REG_EMMCPLL_PA + REG_ID_70, 0x00FF);
        OUTREG16(BASE_REG_EMMCPLL_PA + REG_ID_71, 0xFFFF);
        OUTREG16(BASE_REG_EMMCPLL_PA + REG_ID_73, 0xFFFF);
        OUTREG16(BASE_REG_EMMCPLL_PA + REG_ID_74, 0x80FF);
    }
    /*
     * if bus_timing = HS200
     */
    else
    {
        OUTREG16(BASE_REG_EMMCPLL_PA + REG_ID_03, 0x0000);
        OUTREG16(BASE_REG_EMMCPLL_PA + REG_ID_1C, 0x0300);
        OUTREG16(BASE_REG_EMMCPLL_PA + REG_ID_1D, 0x0100);
        SETREG8(BASE_REG_EMMCPLL_PA + REG_ID_1D, BIT1);
        OUTREG16(BASE_REG_EMMCPLL_PA + REG_ID_20, 0x0600);
        OUTREG16(BASE_REG_EMMCPLL_PA + REG_ID_68, 0x0001);
        OUTREG16(BASE_REG_EMMCPLL_PA + REG_ID_69, 0x0040);
        /*
         * 4 bit
         */
        if (rate == 4)
            OUTREG16(BASE_REG_EMMCPLL_PA + REG_ID_6B, 0x0413);
        /*
         * 8 bit
         */
        else if (rate == 8)
            OUTREG16(BASE_REG_EMMCPLL_PA + REG_ID_6B, 0x0213);
        SETREG16(BASE_REG_EMMCPLL_PA + REG_ID_6C, BIT1);
        OUTREG16(BASE_REG_EMMCPLL_PA + REG_ID_70, 0x0103);
        OUTREG16(BASE_REG_EMMCPLL_PA + REG_ID_71, 0x0000);
        OUTREG16(BASE_REG_EMMCPLL_PA + REG_ID_73, 0xFD00);
        OUTREG16(BASE_REG_EMMCPLL_PA + REG_ID_74, 0x0000);
        OUTREG16(BASE_REG_EMMCPLL_PA + REG_ID_18, 0x8F5C);
        OUTREG16(BASE_REG_EMMCPLL_PA + REG_ID_19, 0x0022);
        OUTREG16(BASE_REG_EMMCPLL_PA + REG_ID_07, 0x0000);
        OUTREG16(BASE_REG_EMMCPLL_PA + REG_ID_05, 0x0000);
        OUTREG16(BASE_REG_EMMCPLL_PA + REG_ID_22, 0x0003);
    }

    return 0;
}

static long hal_clk_fcie_1x_p_round_rate(struct clk_hw *hw, unsigned long rate, unsigned long *parent_rate)
{
    unsigned long round_rate = 0;

    round_rate = rate;

    return round_rate;
}

static unsigned long hal_clk_fcie_1x_p_recalc_rate(struct clk_hw *hw, unsigned long parent_rate)
{
    U16 clk_val_1  = 0;
    U16 clk_val_2  = 0;
    U16 input_clk  = 0;
    U16 output_clk = 0;
    U8  fb_div_val = 0;
    U8  fb_div     = 0;
    U8  p_div_val  = 0;
    U8  p_div      = 0;

    /*
     * emmc
     */
    clk_val_1  = INREG16(BASE_REG_EMMCPLL_PA + REG_ID_18);
    clk_val_2  = INREG16(BASE_REG_EMMCPLL_PA + REG_ID_19);
    fb_div_val = INREG8(BASE_REG_EMMCPLL_PA + REG_ID_04) & (BIT0 | BIT1);
    p_div_val  = INREG8(BASE_REG_EMMCPLL_PA + REG_ID_05) & (BIT0 | BIT1 | BIT2);

    if (fb_div_val == 0)
        fb_div = 1;
    else
        fb_div = 2 << (fb_div_val - 1);

    if (p_div_val < 2)
        p_div = 2;
    else if (p_div_val == 2)
        p_div = 4;
    else if (p_div_val == 3)
        p_div = 6;
    else if (p_div_val == 4)
        p_div = 8;
    else if (p_div_val == 5)
        p_div = 10;
    else if (p_div_val > 5)
        p_div = 16;

    if ((clk_val_2 == 0x22) && clk_val_1 == 0x8F5C)
        input_clk = 100;
    if ((clk_val_2 == 0x21) && clk_val_1 == 0x3B13)
        input_clk = 104;

    /*
     * output_clk = input_clk * (2 ^ fb_div) / p_div
     */
    output_clk = input_clk * fb_div / p_div;

    return output_clk;
}

struct clk_ops sstar_fcie_1x_p_ops = {
    .init        = hal_clk_fcie_1x_p_init,
    .enable      = hal_clk_fcie_1x_p_enable,
    .disable     = hal_clk_fcie_1x_p_disable,
    .is_enabled  = hal_clk_fcie_1x_p_is_enabled,
    .set_rate    = hal_clk_fcie_1x_p_set_rate,
    .round_rate  = hal_clk_fcie_1x_p_round_rate,
    .recalc_rate = hal_clk_fcie_1x_p_recalc_rate,
};

static int hal_clk_fcie_2x_p_init(struct clk_hw *hw)
{
    SETREG8(BASE_REG_EMMCPLL_PA + REG_ID_06, BIT0);
    CLRREG8(BASE_REG_EMMCPLL_PA + REG_ID_06, BIT0);

    return 0;
}

static int hal_clk_fcie_2x_p_enable(struct clk_hw *hw)
{
    CLRREG8(BASE_REG_EMMCPLL_PA + REG_ID_04, BIT5);

    return 0;
}

static void hal_clk_fcie_2x_p_disable(struct clk_hw *hw)
{
    // SETREG8(GET_REG_ADDR8(BASE_REG_RIU_PA, 0x00141A08), BIT(5));
}

static int hal_clk_fcie_2x_p_is_enabled(struct clk_hw *hw)
{
    return ((INREG16(BASE_REG_EMMCPLL_PA + REG_ID_04) & (BIT5)) == 0x0000);
}

static int hal_clk_fcie_2x_p_set_rate(struct clk_hw *hw, unsigned long rate, unsigned long parent_rate)
{
    U16 reg_val = 0;

    /*
     * if bus_timing = HS400
     * set_rate
     */
    SETREG8(BASE_REG_EMMCPLL_PA + REG_ID_03, BIT6);
    SETREG16(BASE_REG_EMMCPLL_PA + REG_ID_1C, BIT8 | BIT9);
    SETREG16(BASE_REG_EMMCPLL_PA + REG_ID_1D, BIT9);
    SETREG16(BASE_REG_EMMCPLL_PA + REG_ID_20, BIT10);
    SETREG8(BASE_REG_EMMCPLL_PA + REG_ID_63, BIT0);
    SETREG8(BASE_REG_EMMCPLL_PA + REG_ID_68, BIT0 | BIT1);
    OUTREG16(BASE_REG_EMMCPLL_PA + REG_ID_1D, 0x2600);
    CLRREG8(BASE_REG_EMMCPLL_PA + REG_ID_69, 0xF0);
    SETREG8(BASE_REG_EMMCPLL_PA + REG_ID_69, BIT5 | BIT6);
    if (rate == 4)
    {
        SETREG8(BASE_REG_EMMCPLL_PA + REG_ID_6A, BIT0);
        OUTREG16(BASE_REG_EMMCPLL_PA + REG_ID_6B, 0x0213);
    }
    else if (rate == 8)
    {
        SETREG8(BASE_REG_EMMCPLL_PA + REG_ID_6A, BIT1);
        OUTREG16(BASE_REG_EMMCPLL_PA + REG_ID_6B, 0x0110);
    }
    OUTREG16(BASE_REG_EMMCPLL_PA + REG_ID_70, 0x0103);
    OUTREG16(BASE_REG_EMMCPLL_PA + REG_ID_71, 0x0000);
    OUTREG16(BASE_REG_EMMCPLL_PA + REG_ID_73, 0xFD00);
    OUTREG16(BASE_REG_EMMCPLL_PA + REG_ID_74, 0x0000);
    OUTREG16(BASE_REG_EMMCPLL_PA + REG_ID_6F, 0x0000);
    OUTREG16(BASE_REG_EMMCPLL_PA + REG_ID_6F, 0x0003);

    OUTREG16(BASE_REG_EMMCPLL_PA + REG_ID_18, 0x8F5C);
    OUTREG16(BASE_REG_EMMCPLL_PA + REG_ID_19, 0x0022);

    OUTREG16(BASE_REG_EMMCPLL_PA + REG_ID_05, 0x0000);
    OUTREG16(BASE_REG_EMMCPLL_PA + REG_ID_30, 0x0006);
    OUTREG16(BASE_REG_EMMCPLL_PA + REG_ID_30, 0x0002);
    OUTREG16(BASE_REG_EMMCPLL_PA + REG_ID_30, 0x0302);
    OUTREG16(BASE_REG_EMMCPLL_PA + REG_ID_33, 0x8000);
    OUTREG16(BASE_REG_EMMCPLL_PA + REG_ID_02, 0x8000);
    OUTREG16(BASE_REG_EMMCPLL_PA + REG_ID_01, 0x2004);
    OUTREG16(BASE_REG_EMMCPLL_PA + REG_ID_09, 0x0012);
    OUTREG16(BASE_REG_EMMCPLL_PA + REG_ID_30, 0x13F0);

    /*
     * delay 80us
     */
    udelay(80);
    OUTREG16(BASE_REG_EMMCPLL_PA + REG_ID_30, 0x13F2);

    /*
     * delay 1us
     */
    udelay(1);
    reg_val = INREG16(BASE_REG_EMMCPLL_PA + REG_ID_33);
    reg_val &= 0x03FF;
    OUTREG16(BASE_REG_EMMCPLL_PA + REG_ID_34, (reg_val - 1) / 2);
    OUTREG16(BASE_REG_EMMCPLL_PA + REG_ID_09, 0x0001);
    OUTREG16(BASE_REG_EMMCPLL_PA + REG_ID_30, 0x00f0);
    OUTREG16(BASE_REG_EMMCPLL_PA + REG_ID_02, 0x0000);
    OUTREG16(BASE_REG_EMMCPLL_PA + REG_ID_09, 0x0013);
    CLRREG16(BASE_REG_EMMCPLL_PA + REG_ID_30, 0xFF00);
    OUTREG16(BASE_REG_EMMCPLL_PA + REG_ID_02, 0x0000);

    return 0;
}

static long hal_clk_fcie_2x_p_round_rate(struct clk_hw *hw, unsigned long rate, unsigned long *parent_rate)
{
    unsigned long round_rate = 0;

    round_rate = rate;

    return round_rate;
}

static unsigned long hal_clk_fcie_2x_p_recalc_rate(struct clk_hw *hw, unsigned long parent_rate)
{
    U16 clk_val_1  = 0;
    U16 clk_val_2  = 0;
    U16 input_clk  = 0;
    U16 output_clk = 0;
    U8  fb_div_val = 0;
    U8  fb_div     = 0;
    U8  p_div_val  = 0;
    U8  p_div      = 0;

    /*
     * emmc
     */
    clk_val_1  = INREG16(BASE_REG_EMMCPLL_PA + REG_ID_18);
    clk_val_2  = INREG16(BASE_REG_EMMCPLL_PA + REG_ID_18);
    fb_div_val = INREG8(BASE_REG_EMMCPLL_PA + REG_ID_04) & (BIT0 | BIT1);
    p_div_val  = INREG8(BASE_REG_EMMCPLL_PA + REG_ID_05) & (BIT0 | BIT1 | BIT2);

    if (fb_div_val == 0)
        fb_div = 1;
    else
        fb_div = 2 << (fb_div_val - 1);

    if (p_div_val < 2)
        p_div = 2;
    else if (p_div_val == 2)
        p_div = 4;
    else if (p_div_val == 3)
        p_div = 6;
    else if (p_div_val == 4)
        p_div = 8;
    else if (p_div_val == 5)
        p_div = 10;
    else if (p_div_val > 5)
        p_div = 16;

    if ((clk_val_2 == 0x22) && clk_val_1 == 0x8F5C)
        input_clk = 100;
    if ((clk_val_2 == 0x21) && clk_val_1 == 0x3B13)
        input_clk = 104;

    /*
     * output_clk = input_clk * (2 ^ fb_div) / p_div
     */
    output_clk = input_clk * fb_div / p_div;

    return output_clk;
}

struct clk_ops sstar_fcie_2x_p_ops = {
    .init        = hal_clk_fcie_2x_p_init,
    .enable      = hal_clk_fcie_2x_p_enable,
    .disable     = hal_clk_fcie_2x_p_disable,
    .is_enabled  = hal_clk_fcie_2x_p_is_enabled,
    .set_rate    = hal_clk_fcie_2x_p_set_rate,
    .round_rate  = hal_clk_fcie_2x_p_round_rate,
    .recalc_rate = hal_clk_fcie_2x_p_recalc_rate,
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
                OUTREG16(BASE_REG_RIU_PA + ((lpll_setting_tbl[idx][reg_idx].address) << 1),
                         lpll_setting_tbl[idx][reg_idx].value);
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

    if (IS_DATA_LANE_LESS_9M(rate))
    {
        idx = HAL_CLK_DISP_PNL_SUPPORTED_LPLL_MAX;
        ret = 0;
    }
    else if (IS_DATA_LANE_BPS_9M_TO_9_5M(rate))
        idx = HAL_CLK_DISP_PNL_SUPPORTED_LPLL_MAX;
    else if (IS_DATA_LANE_BPS_12_5M_TO_25M(rate))
        idx = HAL_CLK_DISP_PNL_SUPPORTED_LPLL_HS_LVDS_CH_12D5TO25MHZ;
    else if (IS_DATA_LANE_BPS_25M_TO_50M(rate))
        idx = HAL_CLK_DISP_PNL_SUPPORTED_LPLL_HS_LVDS_CH_25TO50MHZ;
    else if (IS_DATA_LANE_BPS_50M_TO_100M(rate))
        idx = HAL_CLK_DISP_PNL_SUPPORTED_LPLL_HS_LVDS_CH_50TO100MHZ;
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
     * set disppll to 148.5M
     */
    OUTREG16(BASE_REG_DISPPLL_PA + REG_ID_48, 0x8BA3);
    OUTREG8(BASE_REG_DISPPLL_PA + REG_ID_49, 0x2E);

    OUTREG16(BASE_REG_DISPPLL_PA + REG_ID_40, 0x0A81);
    OUTREG16(BASE_REG_DISPPLL_PA + REG_ID_41, 0x0420);
    OUTREG16(BASE_REG_DISPPLL_PA + REG_ID_42, 0x0013);

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
    unsigned long recalc_rate = 0;

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

static int hal_clk_aupll_384m_init(struct clk_hw *hw)
{
    /*
     * enable audio clock
     */
    OUTREG8(BASE_REG_AUPLL_PA + REG_ID_07, 0x11);

    OUTREG16(BASE_REG_AUPLL_PA + REG_ID_0B, 0x999a);
    OUTREG8(BASE_REG_AUPLL_PA + REG_ID_0C, 0x15);

    return 0;
}

static int hal_clk_aupll_384m_enable(struct clk_hw *hw)
{
    CLRREG8(BASE_REG_AUPLL_PA + REG_ID_00, BIT1);
    udelay(200);

    return 0;
}

static void hal_clk_aupll_384m_disable(struct clk_hw *hw)
{
    SETREG8(BASE_REG_AUPLL_PA + REG_ID_00, BIT1);
}

static int hal_clk_aupll_384m_is_enabled(struct clk_hw *hw)
{
    return ((INREG16(BASE_REG_AUPLL_PA + REG_ID_00) & (BIT1)) == 0x0000);
}

static int hal_clk_aupll_384m_set_rate(struct clk_hw *hw, unsigned long rate, unsigned long parent_rate)
{
    int ret      = 0;
    int div_low  = 0;
    int div_high = 0;

    switch (rate)
    {
        case 48000000:
            div_high = 0x15;
            div_low  = 0x999a;
            break;
        case 47999000:
            div_high = 0x15;
            div_low  = 0x99B7;
            break;
        case 48001000:
            div_high = 0x15;
            div_low  = 0x997c;
            break;
        case 0:
            div_high = 0;
            div_low  = 0;
            break;
        default:
            clk_err("no support to set rate(%ld)\n", rate);
            return -1;
            break;
    }

    OUTREG16(BASE_REG_AUPLL_PA + REG_ID_0B, div_low);
    OUTREG8(BASE_REG_AUPLL_PA + REG_ID_0C, div_high);

    return ret;
}

static long hal_clk_aupll_384m_round_rate(struct clk_hw *hw, unsigned long rate, unsigned long *parent_rate)
{
    unsigned long round_rate = 0;

    round_rate = rate;

    return round_rate;
}

static unsigned long hal_clk_aupll_384m_recalc_rate(struct clk_hw *hw, unsigned long parent_rate)
{
    unsigned long rate = 0;
    int           div  = INREG16(GET_REG_ADDR8(BASE_REG_RIU_PA, 0x00141d16));
    switch (div)
    {
        case 0x999a:
            rate = 48000000;
            break;
        case 0x99B7:
            rate = 47999000;
            break;
        case 0x997c:
            rate = 48001000;
            break;
        case 0:
            rate = 0;
            break;
        default:
            clk_err("no support to recalc rate,current div is %d\n", div);
            break;
    }

    return rate;
}

struct clk_ops sstar_aupll_384m_ops = {
    .init        = hal_clk_aupll_384m_init,
    .enable      = hal_clk_aupll_384m_enable,
    .disable     = hal_clk_aupll_384m_disable,
    .is_enabled  = hal_clk_aupll_384m_is_enabled,
    .set_rate    = hal_clk_aupll_384m_set_rate,
    .round_rate  = hal_clk_aupll_384m_round_rate,
    .recalc_rate = hal_clk_aupll_384m_recalc_rate,
};

static int hal_clk_rtcpll_init(struct clk_hw *hw)
{
    /*
     * set rtcpll to 196.6M
     */
    OUTREG8(BASE_REG_RTCPLL_PA + REG_ID_00, 0x00);
    OUTREG8(BASE_REG_RTCPLL_PA + REG_ID_01, 0x00);

    OUTREG16(BASE_REG_RTCPLL_PA + REG_ID_02, 0x064B);

    OUTREG16(BASE_REG_RTCPLL_PA + REG_ID_03, 0x0000);

    return 0;
}

static int hal_clk_rtcpll_enable(struct clk_hw *hw)
{
    CLRREG16(BASE_REG_RTCPLL_PA + REG_ID_00, BIT0);
    return 0;
}

static void hal_clk_rtcpll_disable(struct clk_hw *hw)
{
    SETREG16(BASE_REG_RTCPLL_PA + REG_ID_00, BIT0);
}

static int hal_clk_rtcpll_is_enabled(struct clk_hw *hw)
{
    return ((INREG16(BASE_REG_RTCPLL_PA + REG_ID_00) & (BIT0)) == 0x0000);
}

struct clk_ops sstar_rtcpll_ops = {
    .init       = hal_clk_rtcpll_init,
    .enable     = hal_clk_rtcpll_enable,
    .is_enabled = hal_clk_rtcpll_is_enabled,
    .disable    = hal_clk_rtcpll_disable,
};

static int hal_clk_mtcmos_lpc2_enable(struct clk_hw *hw)
{
    /*
     * enable bist clk, bist_pm_p enable
     */
    OUTREG8(BASE_REG_PMSLEEP_PA + REG_ID_42, 0x1C);

    /*
     * mtcmos , power on
     */
    OUTREG8(BASE_REG_LPC2_TOP_PA + REG_ID_20, 0x03);
    udelay(1);

    /*
     * de-reset  isp_all (isp+jpe+scl) sw rstz -> 1
     */
    OUTREG8(BASE_REG_LPC2_TOP_PA + REG_ID_30, 0xFF);

    /*
     * enable BIST MD
     */
    OUTREG8(BASE_REG_PATGEN_LPC2_PA + REG_ID_21, 0x01);

    /*
     * sram pwr on,pg sram - 0
     * sram pwr on,pg sram - 1
     * sram pwr on,pg sram - 2
     * sram pwr on,pg sram - 3
     * sram pwr on,pg sram - 4
     */
    OUTREG8(BASE_REG_LPC2_TOP_PA + REG_ID_03, 0x1E);
    udelay(1);
    OUTREG8(BASE_REG_LPC2_TOP_PA + REG_ID_03, 0x1C);
    udelay(1);
    OUTREG8(BASE_REG_LPC2_TOP_PA + REG_ID_03, 0x18);
    udelay(1);
    OUTREG8(BASE_REG_LPC2_TOP_PA + REG_ID_03, 0x10);
    udelay(1);
    OUTREG8(BASE_REG_LPC2_TOP_PA + REG_ID_03, 0x00);
    udelay(1);

    /*
     * leave BIST-MD
     */
    OUTREG8(BASE_REG_PATGEN_LPC2_PA + REG_ID_21, 0x00);

    /*
     * disable bist clk, bist_pm_p enable
     */
    OUTREG8(BASE_REG_PMSLEEP_PA + REG_ID_42, 0x01);

    return 0;
}

static void hal_clk_mtcmos_lpc2_disable(struct clk_hw *hw)
{
    /*
     * enable bist clk, bist_pm_p enable
     */
    OUTREG8(BASE_REG_PMSLEEP_PA + REG_ID_42, 0x1C);

    /*
     * iso en
     */
    OUTREG8(BASE_REG_LPC2_TOP_PA + REG_ID_20, 0x03);
    udelay(1);

    /*
     * enable BIST MD
     */
    OUTREG8(BASE_REG_PATGEN_LPC2_PA + REG_ID_21, 0x01);

    /*
     * sram pwr down,pg sram - 0
     * sram pwr down,pg sram - 1
     * sram pwr down,pg sram - 2
     * sram pwr down,pg sram - 3
     * sram pwr down,pg sram - 4
     */
    OUTREG8(BASE_REG_LPC2_TOP_PA + REG_ID_03, 0x01);
    udelay(1);
    OUTREG8(BASE_REG_LPC2_TOP_PA + REG_ID_03, 0x03);
    udelay(1);
    OUTREG8(BASE_REG_LPC2_TOP_PA + REG_ID_03, 0x07);
    udelay(1);
    OUTREG8(BASE_REG_LPC2_TOP_PA + REG_ID_03, 0x0F);
    udelay(1);
    OUTREG8(BASE_REG_LPC2_TOP_PA + REG_ID_03, 0x1F);
    udelay(1);

    /*
     * enter reset ， isp_all (isp+jpe+scl) sw rstz -> 0
     */
    OUTREG8(BASE_REG_LPC2_TOP_PA + REG_ID_30, 0xFE);
    udelay(1);

    /*
     * mtcmos diable, power off
     */
    OUTREG8(BASE_REG_LPC2_TOP_PA + REG_ID_20, 0x02);
    udelay(1);

    /*
     * disable BIST-MD
     */
    OUTREG8(BASE_REG_PATGEN_LPC2_PA + REG_ID_21, 0x00);

    /*
     * disable bist clk, bist_pm_p enable
     */
    OUTREG8(BASE_REG_PMSLEEP_PA + REG_ID_42, 0x01);
}

struct clk_ops sstar_mtcmos_lpc2_ops = {
    .enable  = hal_clk_mtcmos_lpc2_enable,
    .disable = hal_clk_mtcmos_lpc2_disable,
};

static int hal_clk_mtcmos_usb_enable(struct clk_hw *hw)
{
    /*
     * enable bist clk, bist_usb30_drd_gp_p enable
     */
    OUTREGMSK16(BASE_REG_PMSLEEP_PA + REG_ID_24, 0x1C00, 0xFF00);

    /*
     * mtcmos , power on
     */
    OUTREG8(BASE_REG_PMTOP_PA + REG_ID_50, 0x02);
    udelay(1);

    /*
     * de-reset  usb sw rst -> 0
     */
    OUTREGMSK16(BASE_REG_PMTOP_PA + REG_ID_30, 0x0200, 0xFF00);

    /*
     * enable BIST MD
     */
    OUTREG8(BASE_REG_PATGEN_USB30_PA + REG_ID_21, 0x01);

    /*
     * sram pwr on,pg sram - usb20
     */
    OUTREG8(BASE_REG_PMTOP_PA + REG_ID_45, 0x04);
    udelay(1);

    /*
     * sram pwr on,pg sram - usb30
     */
    OUTREG8(BASE_REG_PMTOP_PA + REG_ID_45, 0x00);
    udelay(1);

    /*
     * leave BIST-MD
     */
    OUTREG8(BASE_REG_PATGEN_USB30_PA + REG_ID_21, 0x00);

    /*
     * de-reset  usb sw rst -> 1
     */
    OUTREGMSK16(BASE_REG_PMTOP_PA + REG_ID_30, 0x0300, 0xFF00);

    /*
     * iso disable
     */
    OUTREG8(BASE_REG_PMTOP_PA + REG_ID_50, 0x00);
    udelay(1);

    /*
     * disable BIST MD
     */
    OUTREG8(BASE_REG_PATGEN_USB30_PA + REG_ID_21, 0x00);

    /*
     * disble bist clk, bist_usb30_drd_gp_p enable
     */
    OUTREGMSK16(BASE_REG_PMSLEEP_PA + REG_ID_24, 0x0100, 0xFF00);
    udelay(1);

    /*
     * de-reset  usb sw rst -> 0
     */
    OUTREGMSK16(BASE_REG_PMTOP_PA + REG_ID_30, 0x0200, 0xFF00);

    /*
     * pre config axi garb (optional, garb locate in SC block)
     */
    OUTREG16(BASE_REG_MIU_GRP_SC0 + REG_ID_60, 0xFFFF);
    OUTREG16(BASE_REG_MIU_GRP_SC0 + REG_ID_61, 0xFFFF);
    OUTREG16(BASE_REG_MIU_GRP_SC0 + REG_ID_43, 0x0017);
    OUTREG16(BASE_REG_MIU_GRP_SC1 + REG_ID_60, 0xFFFF);
    OUTREG16(BASE_REG_MIU_GRP_SC1 + REG_ID_61, 0xFFFF);
    OUTREG16(BASE_REG_MIU_GRP_SC1 + REG_ID_43, 0x0017);

    return 0;
}

static void hal_clk_mtcmos_usb_disable(struct clk_hw *hw)
{
    /*
     * enable bist clk, bist_usb30_drd_gp_p enable
     */
    OUTREGMSK16(BASE_REG_PMSLEEP_PA + REG_ID_24, 0x1C00, 0xFF00);

    /*
     * iso en
     */
    OUTREG8(BASE_REG_PMTOP_PA + REG_ID_50, 0x02);
    udelay(1);

    /*
     * de-reset  usb sw rst -> 0
     */
    OUTREGMSK16(BASE_REG_PMTOP_PA + REG_ID_30, 0x0200, 0xFF00);

    /*
     * enable BIST MD
     */
    OUTREG8(BASE_REG_PATGEN_USB30_PA + REG_ID_21, 0x01);

    /*
     * sram pwr down,pg sram - usb20
     */
    OUTREG8(BASE_REG_PMTOP_PA + REG_ID_45, 0x02);
    udelay(1);

    /*
     * sram pwr down,pg sram - usb30
     */
    OUTREG8(BASE_REG_PMTOP_PA + REG_ID_45, 0x06);
    udelay(1);

    /*
     * enter reset ， usb sw rst -> 1
     */
    OUTREGMSK16(BASE_REG_PMTOP_PA + REG_ID_30, 0x0300, 0xFF00);
    udelay(1);

    /*
     * mtcmos diable, power off
     */
    OUTREG8(BASE_REG_PMTOP_PA + REG_ID_50, 0x03);
    udelay(1);

    /*
     * //enable bist clk, bist_usb30_drd_gp_p disable
     */
    OUTREGMSK16(BASE_REG_PMSLEEP_PA + REG_ID_24, 0x0100, 0xFF00);
    udelay(1);

    /*
     * disable BIST MD
     */
    OUTREG8(BASE_REG_PATGEN_USB30_PA + REG_ID_21, 0x00);
}

struct clk_ops sstar_mtcmos_usb_ops = {
    .enable  = hal_clk_mtcmos_usb_enable,
    .disable = hal_clk_mtcmos_usb_disable,
};

void hal_clk_emmc_ana_pd(void)
{
    OUTREG16(BASE_REG_EMMCPLL_PA + REG_ID_22, 0x0002);
    OUTREG16(BASE_REG_EMMCPLL_PA + REG_ID_22, 0x0003);
    OUTREG16(BASE_REG_EMMCPLL_PA + REG_ID_71, 0x0FFF);
    OUTREG16(BASE_REG_EMMCPLL_PA + REG_ID_47, 0x0FFF);
    OUTREG16(BASE_REG_EMMCPLL_PA + REG_ID_49, 0x0FFF);
    OUTREG8(BASE_REG_EMMCPLL_PA + REG_ID_40, 0x01);
    OUTREG16(BASE_REG_EMMCPLL_PA + REG_ID_43, 0xFFFF);
}

void hal_clk_eth_ana_pd(void)
{
    OUTREG16(BASE_REG_ALBANY1_PA + REG_ID_7E, 0x0102);
    OUTREG16(BASE_REG_ALBANY1_PA + REG_ID_7E, 0x0102);
    OUTREGMSK16(BASE_REG_ALBANY1_PA + REG_ID_5D, 0xC400, 0xFF00);
    OUTREG8(BASE_REG_ALBANY1_PA + REG_ID_66, 0x50);
    OUTREGMSK16(BASE_REG_ALBANY2_PA + REG_ID_50, 0x3000, 0xFF00);
    OUTREGMSK16(BASE_REG_ALBANY2_PA + REG_ID_50, 0x3000, 0xFF00);
    OUTREGMSK16(BASE_REG_ALBANY2_PA + REG_ID_50, 0x3000, 0xFF00);
    OUTREG8(BASE_REG_ALBANY2_PA + REG_ID_1D, 0xF3);
    OUTREG8(BASE_REG_ALBANY2_PA + REG_ID_1D, 0xF3);
    OUTREGMSK16(BASE_REG_ALBANY2_PA + REG_ID_78, 0x3C00, 0xFF00);
    OUTREGMSK16(BASE_REG_ALBANY2_PA + REG_ID_78, 0x3C00, 0xFF00);
    OUTREGMSK16(BASE_REG_ALBANY2_PA + REG_ID_78, 0x3C00, 0xFF00);
}

void hal_clk_sar12b_ana_pd(void)
{
    OUTREG8(BASE_REG_RADAR_ADC_PA + REG_ID_30, 0x00);
    OUTREG8(BASE_REG_RADAR_ADC_PA + REG_ID_50, 0x00);
}

void hal_clk_sdio_ana_pd(void)
{
    OUTREG8(BASE_REG_CHIPTOP_PA + REG_ID_77, 0x02);
    OUTREG16(BASE_REG_CHIPTOP_PA + REG_ID_75, 0x4046);
    OUTREG8(BASE_REG_CHIPTOP_PA + REG_ID_76, 0x04);
    OUTREG8(BASE_REG_CHIPTOP_PA + REG_ID_12, 0x10);
    OUTREGMSK16(BASE_REG_CHIPTOP_PA + REG_ID_50, 0x0000, 0xFF00);

    OUTREG16(BASE_REG_SDPLL_PA + REG_ID_71, 0xFFFF);
    OUTREG16(BASE_REG_SDPLL_PA + REG_ID_2B, 0x0000);
    OUTREG16(BASE_REG_SDPLL_PA + REG_ID_29, 0x0000);
    OUTREG16(BASE_REG_SDPLL_PA + REG_ID_40, 0x0001);
    OUTREG16(BASE_REG_SDPLL_PA + REG_ID_43, 0x003F);
    OUTREG16(BASE_REG_SDPLL_PA + REG_ID_41, 0x0000);

    OUTREG8(BASE_REG_PAD_GPIO_PA + REG_ID_0A, 0x0C);
    OUTREG8(BASE_REG_PAD_GPIO_PA + REG_ID_09, 0x0C);
    OUTREG8(BASE_REG_PAD_GPIO_PA + REG_ID_08, 0x0C);
}

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
        .name = "CLK_venpll_clk",
        .data = &sstar_venpll_ops,
    },
    {
        .name = "CLK_riscvpll_clk",
        .data = &sstar_riscvpll_ops,
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
    {
        .name = "CLK_fuart2_synth_out",
        .data = &sstar_fuart2_synth_out_ops,
    },
    {
        .name = "CLK_fuart3_synth_out",
        .data = &sstar_fuart3_synth_out_ops,
    },
    {
        .name = "CLK_fuart4_synth_out",
        .data = &sstar_fuart4_synth_out_ops,
    },
    {
        .name = "CLK_fuart5_synth_out",
        .data = &sstar_fuart5_synth_out_ops,
    },
    {
        .name = "CLK_fuart6_synth_out",
        .data = &sstar_fuart6_synth_out_ops,
    },
    {
        .name = "CLK_fuart7_synth_out",
        .data = &sstar_fuart7_synth_out_ops,
    },
    {
        .name = "CLK_gmacpll_top",
        .data = &sstar_gmacpll_ops,
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
        .name = "CLK_upll_20m",
        .data = &sstar_upll_20m_ops,
    },
    {
        .name = "CLK_ipupll_clk",
        .data = &sstar_ipupll_ops,
    },
    {
        .name = "CLK_bachpll_384m",
        .data = &sstar_bachpll_384m_ops,
    },
    {
        .name = "CLK_isppll_clk",
        .data = &sstar_isppll_ops,
    },
    {
        .name = "CLK_sd_1x_p",
        .data = &sstar_sd_1x_p_ops,
    },
    {
        .name = "CLK_fcie_1x_p",
        .data = &sstar_fcie_1x_p_ops,
    },
    {
        .name = "CLK_fcie_2x_p",
        .data = &sstar_fcie_2x_p_ops,
    },
    {
        .name = "CLK_disppll_clk",
        .data = &sstar_disppll_ops,
    },
    {
        .name = "CLK_aupll_384m",
        .data = &sstar_aupll_384m_ops,
    },
#if 0
    {
        .name = "CLK_rtcpll_clk",
        .data = &sstar_rtcpll_ops,
    },
#endif
    {
        .name = "CLK_mtcmos_lpc2",
        .data = &sstar_mtcmos_lpc2_ops,
    },
    {
        .name = "CLK_mtcmos_usb",
        .data = &sstar_mtcmos_usb_ops,
    },
    {},
};
MODULE_DEVICE_TABLE(of, hal_clk_complex_clk_match);

#ifdef CONFIG_PM_SLEEP
static const struct hal_clk_pll_info hal_clk_pll_match[] = {
    {
        .name = "CLK_fuart_synth_out",
        .init = hal_clk_fuart_synth_init,
    },
    {
        .name = "CLK_fuart0_synth_out",
        .init = hal_clk_fuart0_synth_init,
    },
    {
        .name = "CLK_fuart1_synth_out",
        .init = hal_clk_fuart1_synth_init,
    },
    {
        .name = "CLK_fuart2_synth_out",
        .init = hal_clk_fuart2_synth_init,
    },
    {
        .name = "CLK_fuart3_synth_out",
        .init = hal_clk_fuart3_synth_init,
    },
    {
        .name = "CLK_fuart4_synth_out",
        .init = hal_clk_fuart4_synth_init,
    },
    {
        .name = "CLK_fuart5_synth_out",
        .init = hal_clk_fuart5_synth_init,
    },
    {
        .name = "CLK_fuart6_synth_out",
        .init = hal_clk_fuart6_synth_init,
    },
    {
        .name = "CLK_fuart7_synth_out",
        .init = hal_clk_fuart7_synth_init,
    },
    {
        .name = "CLK_gmacpll_top",
        .init = hal_clk_gmacpll_init,
    },
    {
        .name = "CLK_ipupll_clk",
        .init = hal_clk_ipupll_init,
    },
    {
        .name = "CLK_bachpll_384m",
        .init = hal_clk_bachpll_384m_init,
    },
    {
        .name = "CLK_isppll_clk",
        .init = hal_clk_isppll_init,
    },
    {
        .name = "CLK_sd_1x_p",
        .init = hal_clk_sd_1x_p_init,
    },
    {
        .name = "CLK_fcie_1x_p",
        .init = hal_clk_fcie_1x_p_init,
    },
    {
        .name = "CLK_fcie_2x_p",
        .init = hal_clk_fcie_2x_p_init,
    },
    {
        .name = "CLK_disppll_clk",
        .init = hal_clk_disppll_init,
    },
    {
        .name = "CLK_aupll_384m",
        .init = hal_clk_aupll_384m_init,
    },
#if 0
    {
        .name = "CLK_rtcpll_clk",
        .init = hal_clk_rtcpll_init,
    },
#endif
};
unsigned long cpu_freq = 0;

static int hal_complex_clk_suspend(void)
{
    clk_dbg("hal_complex_clk_suspend\n");
    hal_clk_emmc_ana_pd();
    hal_clk_eth_ana_pd();
    hal_clk_sar12b_ana_pd();
    hal_clk_sdio_ana_pd();

    cpu_freq = hal_clk_cpuclk_recalc_rate(NULL, 0);
    hal_clk_cpuclk_set_rate(NULL, 12000000, 0);
    return 0;
}

static void hal_complex_clk_resume(void)
{
    struct hal_clk_pll_info *ss_clk;

    hal_clk_cpuclk_set_rate(NULL, cpu_freq, 0);
    list_for_each_entry(ss_clk, &hal_pll_init_list, list)
    {
        ss_clk->init(NULL);
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
#ifdef CONFIG_PM_SLEEP
    struct hal_clk_pll_info *complex_clk = NULL;
    u8                       index       = 0;
#endif

    clk_dbg(" @ %s ,node_name:%s \r\n", __func__, node->name);

    match  = of_match_node(hal_clk_complex_clk_match, node);
    clk_hw = kzalloc(sizeof(struct clk_hw), GFP_KERNEL);
#ifdef CONFIG_PM_SLEEP
    complex_clk = kzalloc(sizeof(struct hal_clk_pll_info), GFP_KERNEL);
#endif

    if (!clk_hw)
        goto fail;

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
    complex_clk->name = node->name;
    for (index = 0; index < (sizeof(hal_clk_pll_match) / sizeof(struct hal_clk_pll_info)); index++)
    {
        if (!strcmp(node->name, hal_clk_pll_match[index].name))
        {
            complex_clk->init = hal_clk_pll_match[index].init;
            INIT_LIST_HEAD(&complex_clk->list);
            list_add(&complex_clk->list, &hal_pll_init_list);
            break;
        }
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
