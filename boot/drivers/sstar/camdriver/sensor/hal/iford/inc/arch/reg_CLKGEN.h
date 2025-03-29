/*
 * reg_CLKGEN.h - Sigmastar
 *
 * Copyright (c) [2019~2022] SigmaStar Technology.
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

#ifndef __REG_CLKGEN__
#define __REG_CLKGEN__
typedef struct
{
// h0000, bit: 3
/* clk_xtali clock setting
[0]:disable clock
[1]:invert clock
[3:2]: select clock source
0 : 12m*/
#define offset_of_clkgen_reg_ckg_xtali (0)
#define mask_of_clkgen_reg_ckg_xtali   (0xf)
    unsigned int reg_ckg_xtali : 4;

// h0000, bit: 7
/* clk_xtali_sc_gp clock setting
[0]:disable clock
[1]:invert clock
[3:2]: select clock source
0 : 12m*/
#define offset_of_clkgen_reg_ckg_xtali_sc_gp (0)
#define mask_of_clkgen_reg_ckg_xtali_sc_gp   (0xf0)
    unsigned int reg_ckg_xtali_sc_gp : 4;

// h0000, bit: 11
/* clk_live clock setting
[0]:disable clock
[1]:invert clock
[3:2]: select clock source
0 : 12m*/
#define offset_of_clkgen_reg_ckg_live (0)
#define mask_of_clkgen_reg_ckg_live   (0xf00)
    unsigned int reg_ckg_live : 4;

    // h0000, bit: 14
    /* */
    unsigned int : 4;

    // h0000
    unsigned int /* padding 16 bit */ : 16;

// h0001, bit: 4
/* clk_mcu clock setting
[0]:disable clock
[1]:invert clock
[4:2]: select clock source
0 : 216m
1 : 288m
2 : 320m
3 : 108m*/
#define offset_of_clkgen_reg_ckg_mcu (2)
#define mask_of_clkgen_reg_ckg_mcu   (0x1f)
    unsigned int reg_ckg_mcu : 5;

    // h0001, bit: 7
    /* */
    unsigned int : 3;

// h0001, bit: 11
/* clk_riubrdg clock setting
[0]:disable clock
[1]:invert clock
[3:2]: select clock source
0 : clk_mcu_p*/
#define offset_of_clkgen_reg_ckg_riubrdg (2)
#define mask_of_clkgen_reg_ckg_riubrdg   (0xf00)
    unsigned int reg_ckg_riubrdg : 4;

    // h0001, bit: 14
    /* */
    unsigned int : 4;

    // h0001
    unsigned int /* padding 16 bit */ : 16;

// h0002, bit: 3
/* clk_bist clock setting
[0]:disable clock
[1]:invert clock
[3:2]: select clock source
0 : 320m
1 : 216m
2 : 144m
3 : 12m*/
#define offset_of_clkgen_reg_ckg_bist (4)
#define mask_of_clkgen_reg_ckg_bist   (0xf)
    unsigned int reg_ckg_bist : 4;

// h0002, bit: 7
/* clk_bist isp clock setting
[0]:disable clock
[1]:invert clock
[3:2]: select clock source
0 : 320m
1 : 216m
2 : 144m
3 : 12m*/
#define offset_of_clkgen_reg_ckg_bist_isp_gp (4)
#define mask_of_clkgen_reg_ckg_bist_isp_gp   (0xf0)
    unsigned int reg_ckg_bist_isp_gp : 4;

    // h0002, bit: 14
    /* */
    unsigned int : 8;

    // h0002
    unsigned int /* padding 16 bit */ : 16;

// h0003, bit: 3
/* clk_bist sc clock setting
[0]:disable clock
[1]:invert clock
[3:2]: select clock source
0 : 320m
1 : 216m
2 : 144m
3 : 12m*/
#define offset_of_clkgen_reg_ckg_bist_sc_gp (6)
#define mask_of_clkgen_reg_ckg_bist_sc_gp   (0xf)
    unsigned int reg_ckg_bist_sc_gp : 4;

    // h0003, bit: 7
    /* */
    unsigned int : 4;

// h0003, bit: 11
/* clk_bist vhe clock setting
[0]:disable clock
[1]:invert clock
[3:2]: select clock source
0 : 216m
1 : 123.4m
2 : 144m
3 : 12m*/
#define offset_of_clkgen_reg_ckg_bist_vhe_gp (6)
#define mask_of_clkgen_reg_ckg_bist_vhe_gp   (0xf00)
    unsigned int reg_ckg_bist_vhe_gp : 4;

    // h0003, bit: 14
    /* */
    unsigned int : 4;

    // h0003
    unsigned int /* padding 16 bit */ : 16;

// h0004, bit: 3
/* clk_pwr_ctl clock setting
[0]:disable clock
[1]:invert clock
[3:2]: select clock source
0 : 750K
1 : 1.5m
2 : 12m*/
#define offset_of_clkgen_reg_ckg_pwr_ctl (8)
#define mask_of_clkgen_reg_ckg_pwr_ctl   (0xf)
    unsigned int reg_ckg_pwr_ctl : 4;

    // h0004, bit: 14
    /* */
    unsigned int : 12;

    // h0004
    unsigned int /* padding 16 bit */ : 16;

// h0005, bit: 3
/* clk_bist ipu clock setting
[0]:disable clock
[1]:invert clock
[3:2]: select clock source
0 : 320m
1 : 216m
2 : 144m
3 : 12m*/
#define offset_of_clkgen_reg_ckg_bist_ipu_gp (10)
#define mask_of_clkgen_reg_ckg_bist_ipu_gp   (0xf)
    unsigned int reg_ckg_bist_ipu_gp : 4;

    // h0005, bit: 14
    /* */
    unsigned int : 12;

    // h0005
    unsigned int /* padding 16 bit */ : 16;

    // h0006, bit: 14
    /* */
    unsigned int : 16;

    // h0006
    unsigned int /* padding 16 bit */ : 16;

// h0007, bit: 4
/* clk_ddr_hw for cpu - axi clock setting
[0]: disable clock
[1]: invert clock
[3:2]: select clock source
  00: clk_ddr_to_dig_buf
  01: reserved
  10: reserved
  11: reserved
[4]: enable select from xtal 12M
  1 : sel fast clk
  0 : sel xtal 12M*/
#define offset_of_clkgen_reg_ckg_ddr_hw (14)
#define mask_of_clkgen_reg_ckg_ddr_hw   (0x1f)
    unsigned int reg_ckg_ddr_hw : 5;

    // h0007, bit: 14
    /* */
    unsigned int : 11;

    // h0007
    unsigned int /* padding 16 bit */ : 16;

// h0008, bit: 3
/* clk_boot_p clock setting
[0]: disable clock
[1]: invert clock
[3:2]: select clock source
  00: 12MHz (xtali)
  01: reserved
  10: reserved
  11: reserved*/
#define offset_of_clkgen_reg_ckg_boot (16)
#define mask_of_clkgen_reg_ckg_boot   (0xf)
    unsigned int reg_ckg_boot : 4;

    // h0008, bit: 14
    /* */
    unsigned int : 12;

    // h0008
    unsigned int /* padding 16 bit */ : 16;

// h0009, bit: 3
/* clk_bist mcu clock setting
[0]:disable clock
[1]:invert clock
[3:2]: select clock source
0 : 216m
1 : 123m
2 : 144m
3 : 12m*/
#define offset_of_clkgen_reg_ckg_bist_mcu (18)
#define mask_of_clkgen_reg_ckg_bist_mcu   (0xf)
    unsigned int reg_ckg_bist_mcu : 4;

    // h0009, bit: 14
    /* */
    unsigned int : 12;

    // h0009
    unsigned int /* padding 16 bit */ : 16;

// h000a, bit: 3
/* clk_live clock setting
[0]:disable clock
[1]:invert clock
[3:2]: select clock source
0 : 12m(CLK_12M_CORE_v_live)*/
#define offset_of_clkgen_reg_ckg_real_live (20)
#define mask_of_clkgen_reg_ckg_real_live   (0xf)
    unsigned int reg_ckg_real_live : 4;

    // h000a, bit: 14
    /* */
    unsigned int : 12;

    // h000a
    unsigned int /* padding 16 bit */ : 16;

    // h000b, bit: 14
    /* */
    unsigned int : 16;

    // h000b
    unsigned int /* padding 16 bit */ : 16;

    // h000c, bit: 14
    /* */
    unsigned int : 16;

    // h000c
    unsigned int /* padding 16 bit */ : 16;

    // h000d, bit: 14
    /* */
    unsigned int : 16;

    // h000d
    unsigned int /* padding 16 bit */ : 16;

    // h000e, bit: 14
    /* */
    unsigned int : 16;

    // h000e
    unsigned int /* padding 16 bit */ : 16;

    // h000f, bit: 14
    /* */
    unsigned int : 16;

    // h000f
    unsigned int /* padding 16 bit */ : 16;

// h0010, bit: 3
/* clk_mpll_syn clock setting
[0]: disable clock
[1]: invert clock
000:  240 MHz (upll)
001:  216 MHz (mpll)
010:  192 MHz (upll)*/
#define offset_of_clkgen_reg_ckg_pm_high_ext (32)
#define mask_of_clkgen_reg_ckg_pm_high_ext   (0xf)
    unsigned int reg_ckg_pm_high_ext : 4;

    // h0010, bit: 14
    /* */
    unsigned int : 12;

    // h0010
    unsigned int /* padding 16 bit */ : 16;

    // h0011, bit: 14
    /* */
    unsigned int : 16;

    // h0011
    unsigned int /* padding 16 bit */ : 16;

    // h0012, bit: 14
    /* */
    unsigned int : 16;

    // h0012
    unsigned int /* padding 16 bit */ : 16;

    // h0013, bit: 14
    /* */
    unsigned int : 16;

    // h0013
    unsigned int /* padding 16 bit */ : 16;

    // h0014, bit: 14
    /* */
    unsigned int : 16;

    // h0014
    unsigned int /* padding 16 bit */ : 16;

    // h0015, bit: 14
    /* */
    unsigned int : 16;

    // h0015
    unsigned int /* padding 16 bit */ : 16;

    // h0016, bit: 14
    /* */
    unsigned int : 16;

    // h0016
    unsigned int /* padding 16 bit */ : 16;

// h0017, bit: 4
/* clk_miu clock setting
[0]: disable clock
[1]: invert clock
[3:2]: select clock source
  00: 12MHz (xtali)
  01: 12MHz (xtali)
  10: MIU_1288BUS_PLL_CLK_VC0_OUT_BUF
  11: 216MHz
[4]: glitch free select
  0: 12MHz (xtali)
  1: select the output according to reg_ckg_miu[3:2]*/
#define offset_of_clkgen_reg_ckg_miu (46)
#define mask_of_clkgen_reg_ckg_miu   (0x1f)
    unsigned int reg_ckg_miu : 5;

    // h0017, bit: 14
    /* */
    unsigned int : 11;

    // h0017
    unsigned int /* padding 16 bit */ : 16;

// h0018, bit: 3
/* clk_miu_rec clock setting
[0]:disable clock
[1]:invert clock
[3:2]: select clock source
0 : 1.5m
1 : 750K
2 : 187K
3 : 93K*/
#define offset_of_clkgen_reg_ckg_miu_rec (48)
#define mask_of_clkgen_reg_ckg_miu_rec   (0xf)
    unsigned int reg_ckg_miu_rec : 4;

    // h0018, bit: 14
    /* */
    unsigned int : 12;

    // h0018
    unsigned int /* padding 16 bit */ : 16;

// h0019, bit: 3
/* clk_ddr_syn clock setting
[0]:disable clock
[1]:invert clock
[3:2]: select clock source
0 : 432m
1 : 216m
2 : 12m*/
#define offset_of_clkgen_reg_ckg_ddr_syn (50)
#define mask_of_clkgen_reg_ckg_ddr_syn   (0xf)
    unsigned int reg_ckg_ddr_syn : 4;

    // h0019, bit: 14
    /* */
    unsigned int : 12;

    // h0019
    unsigned int /* padding 16 bit */ : 16;

    // h001a, bit: 14
    /* */
    unsigned int : 16;

    // h001a
    unsigned int /* padding 16 bit */ : 16;

    // h001b, bit: 14
    /* */
    unsigned int : 16;

    // h001b
    unsigned int /* padding 16 bit */ : 16;

    // h001c, bit: 14
    /* */
    unsigned int : 16;

    // h001c
    unsigned int /* padding 16 bit */ : 16;

    // h001d, bit: 14
    /* */
    unsigned int : 16;

    // h001d
    unsigned int /* padding 16 bit */ : 16;

    // h001e, bit: 14
    /* */
    unsigned int : 16;

    // h001e
    unsigned int /* padding 16 bit */ : 16;

    // h001f, bit: 14
    /* */
    unsigned int : 16;

    // h001f
    unsigned int /* padding 16 bit */ : 16;

// h0020, bit: 2
/* [0]:reserved
[1]:reserved
[2]: select clock source
  0: select BOOT clock 12MHz (xtali)
  1: select the output according to reg_ckg_miu[4:0]*/
#define offset_of_clkgen_reg_ckg_miu_boot (64)
#define mask_of_clkgen_reg_ckg_miu_boot   (0x7)
    unsigned int reg_ckg_miu_boot : 3;

    // h0020, bit: 14
    /* */
    unsigned int : 13;

    // h0020
    unsigned int /* padding 16 bit */ : 16;

// h0021, bit: 3
/* clk_ddr_syn clock setting
[0]:disable clock
[1]:invert clock
[3:2]: select clock source
0 : 432m
1 : 216m
2 : reserved*/
#define offset_of_clkgen_reg_ckg_hemcu (66)
#define mask_of_clkgen_reg_ckg_hemcu   (0xf)
    unsigned int reg_ckg_hemcu : 4;

    // h0021, bit: 14
    /* */
    unsigned int : 12;

    // h0021
    unsigned int /* padding 16 bit */ : 16;

    // h0022, bit: 14
    /* */
    unsigned int : 16;

    // h0022
    unsigned int /* padding 16 bit */ : 16;

    // h0023, bit: 14
    /* */
    unsigned int : 16;

    // h0023
    unsigned int /* padding 16 bit */ : 16;

    // h0024, bit: 14
    /* */
    unsigned int : 16;

    // h0024
    unsigned int /* padding 16 bit */ : 16;

    // h0025, bit: 14
    /* */
    unsigned int : 16;

    // h0025
    unsigned int /* padding 16 bit */ : 16;

    // h0026, bit: 14
    /* */
    unsigned int : 16;

    // h0026
    unsigned int /* padding 16 bit */ : 16;

    // h0027, bit: 14
    /* */
    unsigned int : 16;

    // h0027
    unsigned int /* padding 16 bit */ : 16;

    // h0028, bit: 14
    /* */
    unsigned int : 16;

    // h0028
    unsigned int /* padding 16 bit */ : 16;

    // h0029, bit: 14
    /* */
    unsigned int : 16;

    // h0029
    unsigned int /* padding 16 bit */ : 16;

    // h002a, bit: 14
    /* */
    unsigned int : 16;

    // h002a
    unsigned int /* padding 16 bit */ : 16;

    // h002b, bit: 14
    /* */
    unsigned int : 16;

    // h002b
    unsigned int /* padding 16 bit */ : 16;

    // h002c, bit: 14
    /* */
    unsigned int : 16;

    // h002c
    unsigned int /* padding 16 bit */ : 16;

    // h002d, bit: 3
    /* */
    unsigned int : 4;

// h002d, bit: 7
/* clk_fuart0_synth_in clock setting
[0]:disable clock
[1]:invert clock
[3:2]: select clock source
0 : 432m
1 : 216m*/
#define offset_of_clkgen_reg_ckg_fuart0_synth_in (90)
#define mask_of_clkgen_reg_ckg_fuart0_synth_in   (0xf0)
    unsigned int reg_ckg_fuart0_synth_in : 4;

// h002d, bit: 8
/* FAST UART enable signal*/
#define offset_of_clkgen_reg_uart0_stnthesizer_enable (90)
#define mask_of_clkgen_reg_uart0_stnthesizer_enable   (0x100)
    unsigned int reg_uart0_stnthesizer_enable : 1;

// h002d, bit: 9
/* FAST UART SW resetz*/
#define offset_of_clkgen_reg_uart0_stnthesizer_sw_rstz (90)
#define mask_of_clkgen_reg_uart0_stnthesizer_sw_rstz   (0x200)
    unsigned int reg_uart0_stnthesizer_sw_rstz : 1;

    // h002d, bit: 14
    /* */
    unsigned int : 6;

    // h002d
    unsigned int /* padding 16 bit */ : 16;

// h002e, bit: 14
/* FAST UART0 synthesizer config */
#define offset_of_clkgen_reg_uart0_stnthesizer_fix_nf_freq (92)
#define mask_of_clkgen_reg_uart0_stnthesizer_fix_nf_freq   (0xffff)
    unsigned int reg_uart0_stnthesizer_fix_nf_freq : 16;

    // h002e
    unsigned int /* padding 16 bit */ : 16;

// h002f, bit: 14
/* FAST UART0 synthesizer config */
#define offset_of_clkgen_reg_uart0_stnthesizer_fix_nf_freq_1 (94)
#define mask_of_clkgen_reg_uart0_stnthesizer_fix_nf_freq_1   (0xffff)
    unsigned int reg_uart0_stnthesizer_fix_nf_freq_1 : 16;

    // h002f
    unsigned int /* padding 16 bit */ : 16;

// h0030, bit: 3
/* clk_tck clock setting
[0]: disable clock
[1]: invert clock*/
#define offset_of_clkgen_reg_ckg_tck (96)
#define mask_of_clkgen_reg_ckg_tck   (0xf)
    unsigned int reg_ckg_tck : 4;

    // h0030, bit: 14
    /* */
    unsigned int : 12;

    // h0030
    unsigned int /* padding 16 bit */ : 16;

// h0031, bit: 3
/* clk_fuart0 clock setting
[0]:disable clock
[1]:invert clock
[3:2]: select clock source
0 : 172m
1 : 144m
2 : 24m
3 : clk_fuart0_synth_out*/
#define offset_of_clkgen_reg_ckg_fuart0 (98)
#define mask_of_clkgen_reg_ckg_fuart0   (0xf)
    unsigned int reg_ckg_fuart0 : 4;

// h0031, bit: 7
/* clk_fuart1 clock setting
[0]:disable clock
[1]:invert clock
[3:2]: select clock source
0 : 172m
1 : 144m
2 : 24m
3 : clk_fuart1_synth_out*/
#define offset_of_clkgen_reg_ckg_fuart1 (98)
#define mask_of_clkgen_reg_ckg_fuart1   (0xf0)
    unsigned int reg_ckg_fuart1 : 4;

    // h0031, bit: 14
    /* */
    unsigned int : 8;

    // h0031
    unsigned int /* padding 16 bit */ : 16;

// h0032, bit: 3
/* clk_spi_nonpm clock setting
[0]:disable clock
[1]:invert clock
[3:2]: select clock source
0 : 24m
1 : 86m
2 : 108m
3: 54m*/
#define offset_of_clkgen_reg_ckg_spi_nonpm (100)
#define mask_of_clkgen_reg_ckg_spi_nonpm   (0xf)
    unsigned int reg_ckg_spi_nonpm : 4;

    // h0032, bit: 14
    /* */
    unsigned int : 12;

    // h0032
    unsigned int /* padding 16 bit */ : 16;

// h0033, bit: 3
/* clk_mspi0 clock setting
[0]:disable clock
[1]:invert clock
[3:2]: select clock source
0 : 108m
1 : 144m
2 : 12m
3: spi_syn_pll*/
#define offset_of_clkgen_reg_ckg_mspi0 (102)
#define mask_of_clkgen_reg_ckg_mspi0   (0xf)
    unsigned int reg_ckg_mspi0 : 4;

    // h0033, bit: 14
    /* */
    unsigned int : 12;

    // h0033
    unsigned int /* padding 16 bit */ : 16;

// h0034, bit: 3
/* clk_fuart clock setting
[0]:disable clock
[1]:invert clock
[3:2]: select clock source
0 : 172m
1 : 144m
2 : 24m
3 : clk_fuart_synth_out*/
#define offset_of_clkgen_reg_ckg_fuart (104)
#define mask_of_clkgen_reg_ckg_fuart   (0xf)
    unsigned int reg_ckg_fuart : 4;

// h0034, bit: 7
/* clk_fuart_synth_in clock setting
[0]:disable clock
[1]:invert clock
[3:2]: select clock source
0 : 432m
1 : 216m*/
#define offset_of_clkgen_reg_ckg_fuart_synth_in (104)
#define mask_of_clkgen_reg_ckg_fuart_synth_in   (0xf0)
    unsigned int reg_ckg_fuart_synth_in : 4;

// h0034, bit: 8
/* FAST UART enable signal*/
#define offset_of_clkgen_reg_uart_stnthesizer_enable (104)
#define mask_of_clkgen_reg_uart_stnthesizer_enable   (0x100)
    unsigned int reg_uart_stnthesizer_enable : 1;

// h0034, bit: 9
/* FAST UART SW resetz*/
#define offset_of_clkgen_reg_uart_stnthesizer_sw_rstz (104)
#define mask_of_clkgen_reg_uart_stnthesizer_sw_rstz   (0x200)
    unsigned int reg_uart_stnthesizer_sw_rstz : 1;

    // h0034, bit: 14
    /* */
    unsigned int : 6;

    // h0034
    unsigned int /* padding 16 bit */ : 16;

// h0035, bit: 14
/* FAST UART synthesizer config */
#define offset_of_clkgen_reg_uart_stnthesizer_fix_nf_freq (106)
#define mask_of_clkgen_reg_uart_stnthesizer_fix_nf_freq   (0xffff)
    unsigned int reg_uart_stnthesizer_fix_nf_freq : 16;

    // h0035
    unsigned int /* padding 16 bit */ : 16;

// h0036, bit: 14
/* FAST UART synthesizer config */
#define offset_of_clkgen_reg_uart_stnthesizer_fix_nf_freq_1 (108)
#define mask_of_clkgen_reg_uart_stnthesizer_fix_nf_freq_1   (0xffff)
    unsigned int reg_uart_stnthesizer_fix_nf_freq_1 : 16;

    // h0036
    unsigned int /* padding 16 bit */ : 16;

// h0037, bit: 3
/* clk_miic0 clock setting
[0]:disable clock
[1]:invert clock
[3:2]: select clock source
0 : 72m
1 : 54m
2 : 12m*/
#define offset_of_clkgen_reg_ckg_miic0 (110)
#define mask_of_clkgen_reg_ckg_miic0   (0xf)
    unsigned int reg_ckg_miic0 : 4;

    // h0037, bit: 7
    /* */
    unsigned int : 4;

// h0037, bit: 11
/* clk_miic1 clock setting
[0]:disable clock
[1]:invert clock
[3:2]: select clock source
0 : 72m
1 : 54m
2 : 12m*/
#define offset_of_clkgen_reg_ckg_miic1 (110)
#define mask_of_clkgen_reg_ckg_miic1   (0xf00)
    unsigned int reg_ckg_miic1 : 4;

// h0037, bit: 14
/* clk_miic2 clock setting
[0]:disable clock
[1]:invert clock
[3:2]: select clock source
0 : 72m
1 : 54m
2 : 12m*/
#define offset_of_clkgen_reg_ckg_miic2 (110)
#define mask_of_clkgen_reg_ckg_miic2   (0xf000)
    unsigned int reg_ckg_miic2 : 4;

    // h0037
    unsigned int /* padding 16 bit */ : 16;

    // h0038, bit: 7
    /* */
    unsigned int : 8;

// h0038, bit: 12
/* clk_pwm clock setting
[0]:disable clock
[1]:invert clock
[4:2]: select clock source
0 : 12m
1 : 6m
2 : 3m
3 : 1.5m
4 : 750k
5 : 86m*/
#define offset_of_clkgen_reg_ckg_pwm (112)
#define mask_of_clkgen_reg_ckg_pwm   (0x1f00)
    unsigned int reg_ckg_pwm : 5;

    // h0038, bit: 14
    /* */
    unsigned int : 3;

    // h0038
    unsigned int /* padding 16 bit */ : 16;

    // h0039, bit: 14
    /* */
    unsigned int : 16;

    // h0039
    unsigned int /* padding 16 bit */ : 16;

    // h003a, bit: 3
    /* */
    unsigned int : 4;

// h003a, bit: 7
/* clk_fuart1_synth_in clock setting
[0]:disable clock
[1]:invert clock
[3:2]: select clock source
0 : 432m
1 : 216m*/
#define offset_of_clkgen_reg_ckg_fuart1_synth_in (116)
#define mask_of_clkgen_reg_ckg_fuart1_synth_in   (0xf0)
    unsigned int reg_ckg_fuart1_synth_in : 4;

// h003a, bit: 8
/* FAST UART enable signal*/
#define offset_of_clkgen_reg_uart1_stnthesizer_enable (116)
#define mask_of_clkgen_reg_uart1_stnthesizer_enable   (0x100)
    unsigned int reg_uart1_stnthesizer_enable : 1;

// h003a, bit: 9
/* FAST UART SW resetz*/
#define offset_of_clkgen_reg_uart1_stnthesizer_sw_rstz (116)
#define mask_of_clkgen_reg_uart1_stnthesizer_sw_rstz   (0x200)
    unsigned int reg_uart1_stnthesizer_sw_rstz : 1;

    // h003a, bit: 14
    /* */
    unsigned int : 6;

    // h003a
    unsigned int /* padding 16 bit */ : 16;

// h003b, bit: 14
/* FAST UART1 synthesizer config */
#define offset_of_clkgen_reg_uart1_stnthesizer_fix_nf_freq (118)
#define mask_of_clkgen_reg_uart1_stnthesizer_fix_nf_freq   (0xffff)
    unsigned int reg_uart1_stnthesizer_fix_nf_freq : 16;

    // h003b
    unsigned int /* padding 16 bit */ : 16;

// h003c, bit: 14
/* FAST UART1 synthesizer config */
#define offset_of_clkgen_reg_uart1_stnthesizer_fix_nf_freq_1 (120)
#define mask_of_clkgen_reg_uart1_stnthesizer_fix_nf_freq_1   (0xffff)
    unsigned int reg_uart1_stnthesizer_fix_nf_freq_1 : 16;

    // h003c
    unsigned int /* padding 16 bit */ : 16;

    // h003d, bit: 14
    /* */
    unsigned int : 16;

    // h003d
    unsigned int /* padding 16 bit */ : 16;

    // h003e, bit: 14
    /* */
    unsigned int : 16;

    // h003e
    unsigned int /* padding 16 bit */ : 16;

    // h003f, bit: 14
    /* */
    unsigned int : 16;

    // h003f
    unsigned int /* padding 16 bit */ : 16;

    // h0040, bit: 14
    /* */
    unsigned int : 16;

    // h0040
    unsigned int /* padding 16 bit */ : 16;

    // h0041, bit: 14
    /* */
    unsigned int : 16;

    // h0041
    unsigned int /* padding 16 bit */ : 16;

// h0042, bit: 3
/* clk_emac_ahb clock setting
[0]:disable clock
[1]:invert clock
[3:2]: select clock source
0 : 144m
1 : 123m
2 : 86m
3 : clk_emac_testrx125_in_lan*/
#define offset_of_clkgen_reg_ckg_emac_ahb (132)
#define mask_of_clkgen_reg_ckg_emac_ahb   (0xf)
    unsigned int reg_ckg_emac_ahb : 4;

    // h0042, bit: 14
    /* */
    unsigned int : 12;

    // h0042
    unsigned int /* padding 16 bit */ : 16;

// h0043, bit: 7
/* clk_sd clock setting
[0]:disable clock
[1]:invert clock
[5:2]: select clock source
0 : 48m
1 : 43.2m
2 : 40m
3 : 36m
4 : 32m
5 : 20m
6 : 12m
7 : 300k
8 : spi_synth_pll
9 : 432m
10: 384m
11:320m
12:288m
13:216m
14:108m
15:86m
[6]: glitch free select
  0: 12MHz (xtali)
  1: select the output according to reg_ckg_sd[5:2]
[7]:select clock clk_sd_boot
0:clk_miu_sd_gp_p
1:clk_sd_p_div4*/
#define offset_of_clkgen_reg_ckg_sd (134)
#define mask_of_clkgen_reg_ckg_sd   (0xff)
    unsigned int reg_ckg_sd : 8;

// h0043, bit: 14
/* clk_sd clock setting for different phase
[0]: enable tx different phase function
[2:1]: set clk_sd to pad (tx write data) phase
00: phase 0
01: phase 90
10: phase 180
11: phase270
[4:3]: set clk_sd to sdr_macro (read latch) phase
00: phase 0
01: phase 90
10: phase 180
11: phase270
[5]: enable rx different phase function
[6]: inverse clk for sd_sdr_macro
[7]: sw rst , low active
*/
#define offset_of_clkgen_reg_ckg_sd_phase (134)
#define mask_of_clkgen_reg_ckg_sd_phase   (0xff00)
    unsigned int reg_ckg_sd_phase : 8;

    // h0043
    unsigned int /* padding 16 bit */ : 16;

// h0044, bit: 3
/* clk_ecc clock setting
[0]: disable clock
[1]: invert clock
[3:2]: select clock source
  00: 160MHz
  01: 108MHz
  10: 54MHz
  11: 12MHz (xtali)   */
#define offset_of_clkgen_reg_ckg_ecc (136)
#define mask_of_clkgen_reg_ckg_ecc   (0xf)
    unsigned int reg_ckg_ecc : 4;

    // h0044, bit: 7
    /* */
    unsigned int : 4;

// h0044, bit: 12
/* clk_sd_syn clock setting
[0]:disable clock
[1]:invert clock
[3:2]: select clock source
0 : 216m
1 : 432m
2 : 12m*/
#define offset_of_clkgen_reg_ckg_sd_syn (136)
#define mask_of_clkgen_reg_ckg_sd_syn   (0x1f00)
    unsigned int reg_ckg_sd_syn : 5;

    // h0044, bit: 14
    /* */
    unsigned int : 3;

    // h0044
    unsigned int /* padding 16 bit */ : 16;

    // h0045, bit: 14
    /* */
    unsigned int : 16;

    // h0045
    unsigned int /* padding 16 bit */ : 16;

    // h0046, bit: 7
    /* */
    unsigned int : 8;

// h0046, bit: 12
/* clk_dsc_enc1 clock setting
[0]:disable clock
[1]:invert clock
[4:2]: select clock source
0 : 192m
1 : 160m
2 : 144m
3 : 288m*/
#define offset_of_clkgen_reg_ckg_dsc_enc1 (140)
#define mask_of_clkgen_reg_ckg_dsc_enc1   (0x1f00)
    unsigned int reg_ckg_dsc_enc1 : 5;

    // h0046, bit: 14
    /* */
    unsigned int : 3;

    // h0046
    unsigned int /* padding 16 bit */ : 16;

    // h0047, bit: 14
    /* */
    unsigned int : 16;

    // h0047
    unsigned int /* padding 16 bit */ : 16;

    // h0048, bit: 14
    /* */
    unsigned int : 16;

    // h0048
    unsigned int /* padding 16 bit */ : 16;

// h0049, bit: 3
/* clk_disp_pixel_0 clock setting
[0]:disable clock
[1]:invert clock
[4:2]: select clock source
0 : 72m
1 : 54m
2 : disppll
3 : disppll_div2*/
#define offset_of_clkgen_reg_ckg_disp_pixel_0 (146)
#define mask_of_clkgen_reg_ckg_disp_pixel_0   (0xf)
    unsigned int reg_ckg_disp_pixel_0 : 4;

    // h0049, bit: 14
    /* */
    unsigned int : 12;

    // h0049
    unsigned int /* padding 16 bit */ : 16;

// h004a, bit: 4
/* clk_jpd clock setting
[0]:disable clock
[1]:invert clock
[4:2]: select clock source
0 : 600m
1 : 533m
2 : 480m
3 : 432m
4 : 384m
5 : 288m*/
#define offset_of_clkgen_reg_ckg_jpd (148)
#define mask_of_clkgen_reg_ckg_jpd   (0x1f)
    unsigned int reg_ckg_jpd : 5;

    // h004a, bit: 14
    /* */
    unsigned int : 11;

    // h004a
    unsigned int /* padding 16 bit */ : 16;

// h004b, bit: 7
/* clk_fcie clock setting
[0]:disable clock
[1]:invert clock
[5:2]: select clock source
0 : 48m
1 : 43.2m
2 : 40m
3 : 36m
4 : 32m
5 : 20m
6 : 12m
7 : 300K
8 : clk_spi_synth_pll
9 : 432m
10 : 384m
11 : 320m
12 : 288m
13 : 216m
14 : 1'b0
15 : 1'b0
[6]:select clock clk_fcie_boot
0:clk_fcie_buf_p
1:clk_miu_sd_gp_p
[7]: 0: sel clk_fcie_p, 1: sel clk_spi_p*/
#define offset_of_clkgen_reg_ckg_fcie (150)
#define mask_of_clkgen_reg_ckg_fcie   (0xff)
    unsigned int reg_ckg_fcie : 8;

// h004b, bit: 14
/* clk_fcie clock setting for different phase
[0]: enable tx different phase function
[2:1]: set clk_sd to pad (tx write data) phase
00: phase 0
01: phase 90
10: phase 180
11: phase270
[4:3]: set clk_sd to sdr_macro (read latch) phase
00: phase 0
01: phase 90
10: phase 180
11: phase270
[5]: enable rx different phase function
[6]: inverse clk for sd_sdr_macro
[7]: sw rst , low active
*/
#define offset_of_clkgen_reg_ckg_fcie_phase (150)
#define mask_of_clkgen_reg_ckg_fcie_phase   (0xff00)
    unsigned int reg_ckg_fcie_phase : 8;

    // h004b
    unsigned int /* padding 16 bit */ : 16;

    // h004c, bit: 7
    /* */
    unsigned int : 8;

// h004c, bit: 12
/* clk_dsc_dec1 clock setting
[0]:disable clock
[1]:invert clock
[4:2]: select clock source
0 : 192m
1 : 160m
2 : 144m
3 : 288m*/
#define offset_of_clkgen_reg_ckg_dsc_dec1 (152)
#define mask_of_clkgen_reg_ckg_dsc_dec1   (0x1f00)
    unsigned int reg_ckg_dsc_dec1 : 5;

    // h004c, bit: 14
    /* */
    unsigned int : 3;

    // h004c
    unsigned int /* padding 16 bit */ : 16;

// h004d, bit: 4
/* clk_fcie_syn clock setting
[0]:disable clock
[1]:invert clock
[3:2]: select clock source
0 : 216m
1 : 432m
2 : 12m*/
#define offset_of_clkgen_reg_ckg_fcie_syn (154)
#define mask_of_clkgen_reg_ckg_fcie_syn   (0x1f)
    unsigned int reg_ckg_fcie_syn : 5;

    // h004d, bit: 14
    /* */
    unsigned int : 11;

    // h004d
    unsigned int /* padding 16 bit */ : 16;

// h004e, bit: 2
/* clk_sd clock setting for different phase
extend 3 bit for reg_ckg_sd_phase[10:0]
[8] 1: enable 8 phase mode
[9 & 2:1]: set clk_sd to pad (tx write data) phase
000: phase0
001: phase1
010: phase2
011: phase3
100: phase4
101: phase5
110: phase6
111: phase7
[10 & 4:3]: set clk_sd to sdr_macro (read latch) phase
000: phase0
001: phase1
010: phase2
011: phase3
100: phase4
101: phase5
110: phase6
111: phase7

*/
#define offset_of_clkgen_reg_ckg_sd_phase_ext (156)
#define mask_of_clkgen_reg_ckg_sd_phase_ext   (0x7)
    unsigned int reg_ckg_sd_phase_ext : 3;

    // h004e, bit: 7
    /* */
    unsigned int : 5;

// h004e, bit: 10
/* clk_fcie clock setting for different phase
extend 3 bit for reg_ckg_fcie_phase[10:0]
[8] 1: enable 8 phase mode
[9 & 2:1]: set clk_fcie to pad (tx write data) phase
000: phase0
001: phase1
010: phase2
011: phase3
100: phase4
101: phase5
110: phase6
111: phase7
[10 & 4:3]: set clk_fcie to sdr_macro (read latch) phase
000: phase0
001: phase1
010: phase2
011: phase3
100: phase4
101: phase5
110: phase6
111: phase7*/
#define offset_of_clkgen_reg_ckg_fcie_phase_ext (156)
#define mask_of_clkgen_reg_ckg_fcie_phase_ext   (0x700)
    unsigned int reg_ckg_fcie_phase_ext : 3;

    // h004e, bit: 14
    /* */
    unsigned int : 5;

    // h004e
    unsigned int /* padding 16 bit */ : 16;

    // h004f, bit: 14
    /* */
    unsigned int : 16;

    // h004f
    unsigned int /* padding 16 bit */ : 16;

// h0050, bit: 3
/* clk_ipu clock setting
[0]:disable clock
[1]:invert clock
[4:2]: select clock source
0 : 12m
3 : 320m
6 : 432m
7 : 480m*/
#define offset_of_clkgen_reg_ckg_ipu (160)
#define mask_of_clkgen_reg_ckg_ipu   (0xf)
    unsigned int reg_ckg_ipu : 4;

    // h0050, bit: 7
    /* */
    unsigned int : 4;

// h0050, bit: 11
/* clk_ipuff clock setting
[0]:disable clock
[1]:invert clock
[3:2]: select clock source
0 : 432m
1 : 320m
2 : 288m
3 : 216m*/
#define offset_of_clkgen_reg_ckg_ipuff (160)
#define mask_of_clkgen_reg_ckg_ipuff   (0xf00)
    unsigned int reg_ckg_ipuff : 4;

    // h0050, bit: 14
    /* */
    unsigned int : 4;

    // h0050
    unsigned int /* padding 16 bit */ : 16;

    // h0051, bit: 14
    /* */
    unsigned int : 16;

    // h0051
    unsigned int /* padding 16 bit */ : 16;

    // h0052, bit: 14
    /* */
    unsigned int : 16;

    // h0052
    unsigned int /* padding 16 bit */ : 16;

// h0053, bit: 3
/* clk_ldcfeye clock setting
[0]:disable clock
[1]:invert clock
[3:2]: select clock source
0 : 384m
1 : 216m
2 : 172m
3 : 123m
4 : 432m*/
#define offset_of_clkgen_reg_ckg_ldcfeye (166)
#define mask_of_clkgen_reg_ckg_ldcfeye   (0x1f)
    unsigned int reg_ckg_ldcfeye : 5;

    // h0053, bit: 14
    /* */
    unsigned int : 11;

    // h0053
    unsigned int /* padding 16 bit */ : 16;

// h0054, bit: 6
/* clk_bt656_clock setting
[1]: invert clock
[4]: enable phase delay
[6:5]: phase delay selection*/
#define offset_of_clkgen_reg_ckg_bt656_0 (168)
#define mask_of_clkgen_reg_ckg_bt656_0   (0x7f)
    unsigned int reg_ckg_bt656_0 : 7;

    // h0054, bit: 7
    /* */
    unsigned int : 1;

// h0054, bit: 14
/* clk_bt656_clock setting
[1]: invert clock
[4]: enable phase delay
[6:5]: phase delay selection*/
#define offset_of_clkgen_reg_ckg_bt656_1 (168)
#define mask_of_clkgen_reg_ckg_bt656_1   (0x7f00)
    unsigned int reg_ckg_bt656_1 : 7;

    // h0054, bit: 15
    /* */
    unsigned int : 1;

    // h0054
    unsigned int /* padding 16 bit */ : 16;

    // h0055, bit: 14
    /* */
    unsigned int : 16;

    // h0055
    unsigned int /* padding 16 bit */ : 16;

    // h0056, bit: 14
    /* */
    unsigned int : 16;

    // h0056
    unsigned int /* padding 16 bit */ : 16;

    // h0057, bit: 14
    /* */
    unsigned int : 16;

    // h0057
    unsigned int /* padding 16 bit */ : 16;

// h0058, bit: 3
/* clk_csi_mac_lptx_top_i_a0 clock setting
[0]:disable clock
[1]:invert clock
[3:2]: select clock source
0 : 288m
1 : 240m
2 : 216m
4 : 172m*/
#define offset_of_clkgen_reg_ckg_csi0_mac_lptx_top_i (176)
#define mask_of_clkgen_reg_ckg_csi0_mac_lptx_top_i   (0xf)
    unsigned int reg_ckg_csi0_mac_lptx_top_i : 4;

    // h0058, bit: 7
    /* */
    unsigned int : 4;

// h0058, bit: 11
/* clk_csi_mac_top_i_a0 clock setting
[0]:disable clock
[1]:invert clock
[3:2]: select clock source
0 : 288m
1 : 240m
2 : 216m
4 : 172m*/
#define offset_of_clkgen_reg_ckg_csi0_mac_top_i (176)
#define mask_of_clkgen_reg_ckg_csi0_mac_top_i   (0xf00)
    unsigned int reg_ckg_csi0_mac_top_i : 4;

    // h0058, bit: 14
    /* */
    unsigned int : 4;

    // h0058
    unsigned int /* padding 16 bit */ : 16;

// h0059, bit: 3
/* clk_ns_top_i_a0 clock setting
[0]:disable clock
[1]:invert clock
[3:2]: select clock source
0 : 288m
1 : 240m
2 : 216m
4 : 172m*/
#define offset_of_clkgen_reg_ckg_csi0_ns_top_i (178)
#define mask_of_clkgen_reg_ckg_csi0_ns_top_i   (0xf)
    unsigned int reg_ckg_csi0_ns_top_i : 4;

    // h0059, bit: 7
    /* */
    unsigned int : 4;

// h0059, bit: 11
/* clk_csi_mac_lptx_top_i_a1 clock setting
[0]:disable clock
[1]:invert clock
[3:2]: select clock source
0 : 288m
1 : 240m
2 : 216m
4 : 172m*/
#define offset_of_clkgen_reg_ckg_csi1_mac_lptx_top_i (178)
#define mask_of_clkgen_reg_ckg_csi1_mac_lptx_top_i   (0xf00)
    unsigned int reg_ckg_csi1_mac_lptx_top_i : 4;

    // h0059, bit: 14
    /* */
    unsigned int : 4;

    // h0059
    unsigned int /* padding 16 bit */ : 16;

// h005a, bit: 3
/* clk_csi_mac_top_i_a1 clock setting
[0]:disable clock
[1]:invert clock
[3:2]: select clock source
0 : 288m
1 : 240m
2 : 216m
4 : 172m*/
#define offset_of_clkgen_reg_ckg_csi1_mac_top_i (180)
#define mask_of_clkgen_reg_ckg_csi1_mac_top_i   (0xf)
    unsigned int reg_ckg_csi1_mac_top_i : 4;

    // h005a, bit: 7
    /* */
    unsigned int : 4;

// h005a, bit: 11
/* clk_ns_top_i_a1 clock setting
[0]:disable clock
[1]:invert clock
[3:2]: select clock source
0 : 288m
1 : 240m
2 : 216m
4 : 172m*/
#define offset_of_clkgen_reg_ckg_csi1_ns_top_i (180)
#define mask_of_clkgen_reg_ckg_csi1_ns_top_i   (0xf00)
    unsigned int reg_ckg_csi1_ns_top_i : 4;

    // h005a, bit: 14
    /* */
    unsigned int : 4;

    // h005a
    unsigned int /* padding 16 bit */ : 16;

    // h005b, bit: 14
    /* */
    unsigned int : 16;

    // h005b
    unsigned int /* padding 16 bit */ : 16;

    // h005c, bit: 14
    /* */
    unsigned int : 16;

    // h005c
    unsigned int /* padding 16 bit */ : 16;

    // h005d, bit: 14
    /* */
    unsigned int : 16;

    // h005d
    unsigned int /* padding 16 bit */ : 16;

    // h005e, bit: 14
    /* */
    unsigned int : 16;

    // h005e
    unsigned int /* padding 16 bit */ : 16;

    // h005f, bit: 14
    /* */
    unsigned int : 16;

    // h005f
    unsigned int /* padding 16 bit */ : 16;

// h0060, bit: 4
/* clk_bdma clock setting
[0]: disable clock
[1]: invert clock
[3:2]: select clock source
  0: select clk_miu_p according to reg_ckg_miu
  1~3: no clocks
[4]: select BOOT clock source (See reg_ckg_boot)  (glitch free)
0: select BOOT clock 12MHz (xtali)
1: select BDMA clock source*/
#define offset_of_clkgen_reg_ckg_bdma (192)
#define mask_of_clkgen_reg_ckg_bdma   (0x1f)
    unsigned int reg_ckg_bdma : 5;

    // h0060, bit: 14
    /* */
    unsigned int : 11;

    // h0060
    unsigned int /* padding 16 bit */ : 16;

// h0061, bit: 4
/* clk_aesdma clock setting
[0]: disable clock
[1]: invert clock
[3:2]: select clock source
  00: miupll_clk_buf
  01: 216MHz
  10: 172MHz
  11: clk_otp_p
  others: no clocks
[4]: gltich free select
  0: 12MHz (xtai)
  1: select the output according to reg_ckg_aesdma[3:2]*/
#define offset_of_clkgen_reg_ckg_aesdma (194)
#define mask_of_clkgen_reg_ckg_aesdma   (0x1f)
    unsigned int reg_ckg_aesdma : 5;

    // h0061, bit: 7
    /* */
    unsigned int : 3;

// h0061, bit: 11
/* clk_isp clock setting
[0]:disable clock
[1]:invert clock
[3:2]: select clock source
0 : 288m
1 : 240m
2 : 192m
3 : 384m*/
#define offset_of_clkgen_reg_ckg_isp (194)
#define mask_of_clkgen_reg_ckg_isp   (0xf00)
    unsigned int reg_ckg_isp : 4;

    // h0061, bit: 14
    /* */
    unsigned int : 4;

    // h0061
    unsigned int /* padding 16 bit */ : 16;

// h0062, bit: 3
/* clk_sr clock setting (sensor pixel clock)
[0]: disable clock
[1]: invert clock
[3:2]: select clock source
  00: clk_sr from sr_pad
  01: clk_sr from mipi_atop
  10: 40MHz
  11: 86MHz*/
#define offset_of_clkgen_reg_ckg_sr (196)
#define mask_of_clkgen_reg_ckg_sr   (0xf)
    unsigned int reg_ckg_sr : 4;

    // h0062, bit: 7
    /* */
    unsigned int : 4;

// h0062, bit: 13
/* clk_sr00_mclk clock setting
[0]:disable clock
[1]:invert clock
[5:2]: select clock source
0 : 27m
1 : 72m
2 : 61.7m
3 : 54m
4 : 48m
5 : 43.2m
6 : 36m
7 : 24m
8 : 21.6m
9 : 12m
10 : 5.4m
11 : lpll_clk(37.125M)
12 : div2
13 : div4
14 : div8
15 : 1'b0*/
#define offset_of_clkgen_reg_ckg_sr00_mclk (196)
#define mask_of_clkgen_reg_ckg_sr00_mclk   (0x3f00)
    unsigned int reg_ckg_sr00_mclk : 6;

    // h0062, bit: 14
    /* */
    unsigned int : 2;

    // h0062
    unsigned int /* padding 16 bit */ : 16;

// h0063, bit: 3
/* clk_idclk clock setting
[0]: disable clock
[1]: invert clock
[3:2]: select clock source
00: select clk_isp_p according to reg_ckg_sip
01: clk_idclk from sr_pad*/
#define offset_of_clkgen_reg_ckg_idclk (198)
#define mask_of_clkgen_reg_ckg_idclk   (0xf)
    unsigned int reg_ckg_idclk : 4;

    // h0063, bit: 14
    /* */
    unsigned int : 12;

    // h0063
    unsigned int /* padding 16 bit */ : 16;

// h0064, bit: 3
/* clk_fclk1 clock setting
[0]:disable clock
[1]:invert clock
[3:2]: select clock source
0 : 192m
1 : 288m
2 : 172m
3 : 320m
4:  240m
5: 216m*/
#define offset_of_clkgen_reg_ckg_fclk1 (200)
#define mask_of_clkgen_reg_ckg_fclk1   (0x1f)
    unsigned int reg_ckg_fclk1 : 5;

    // h0064, bit: 14
    /* */
    unsigned int : 11;

    // h0064
    unsigned int /* padding 16 bit */ : 16;

// h0065, bit: 5
/* clk_sr01_mclk clock setting
[0]:disable clock
[1]:invert clock
[5:2]: select clock source
0 : 27m
1 : 72m
2 : 61.7m
3 : 54m
4 : 48m
5 : 43.2m
6 : 36m
7 : 24m
8 : 21.6m
9 : 12m
10 : 5.4m
11 : lpll_clk(37.125m)
12 : div2
13 : div4
14 : div8
15 : 1'b0*/
#define offset_of_clkgen_reg_ckg_sr01_mclk (202)
#define mask_of_clkgen_reg_ckg_sr01_mclk   (0x3f)
    unsigned int reg_ckg_sr01_mclk : 6;

    // h0065, bit: 7
    /* */
    unsigned int : 2;

// h0065, bit: 13
/* clk_sr02_mclk clock setting
[0]:disable clock
[1]:invert clock
[5:2]: select clock source
0 : 27m
1 : 72m
2 : 61.7m
3 : 54m
4 : 48m
5 : 43.2m
6 : 36m
7 : 24m
8 : 21.6m
9 : 12m
10 : 5.4m
11 : lpll_clk(37.125m)
12 : div2
13 : div4
14 : div8
15 : 1'b0*/
#define offset_of_clkgen_reg_ckg_sr02_mclk (202)
#define mask_of_clkgen_reg_ckg_sr02_mclk   (0x3f00)
    unsigned int reg_ckg_sr02_mclk : 6;

    // h0065, bit: 14
    /* */
    unsigned int : 2;

    // h0065
    unsigned int /* padding 16 bit */ : 16;

// h0066, bit: 3
/* clk_odclk clock setting
[0]:disable clock
[1]:invert clock
[3:2]: select clock source
0 : 86m
1 : 43.2m
2 : 21.6m
3 : lpll_clk*/
#define offset_of_clkgen_reg_ckg_odclk (204)
#define mask_of_clkgen_reg_ckg_odclk   (0xf)
    unsigned int reg_ckg_odclk : 4;

    // h0066, bit: 7
    /* */
    unsigned int : 4;

// h0066, bit: 13
/* clk_sr03_mclk clock setting
[0]:disable clock
[1]:invert clock
[5:2]: select clock source
0 : 27m
1 : 72m
2 : 61.7m
3 : 54m
4 : 48m
5 : 43.2m
6 : 36m
7 : 24m
8 : 21.6m
9 : 12m
10 : 5.4m
11 : lpll_clk(37.125M)
12 : div2
13 : div4
14 : div8
15 : 1'b0*/
#define offset_of_clkgen_reg_ckg_sr03_mclk (204)
#define mask_of_clkgen_reg_ckg_sr03_mclk   (0x3f00)
    unsigned int reg_ckg_sr03_mclk : 6;

    // h0066, bit: 14
    /* */
    unsigned int : 2;

    // h0066
    unsigned int /* padding 16 bit */ : 16;

// h0067, bit: 3
/* NOT USED:
clk_gop clock setting
[0]: disable clock
[1]: invert clock
[3:2]: select clock source
*/
#define offset_of_clkgen_reg_ckg_gop (206)
#define mask_of_clkgen_reg_ckg_gop   (0xf)
    unsigned int reg_ckg_gop : 4;

    // h0067, bit: 14
    /* */
    unsigned int : 12;

    // h0067
    unsigned int /* padding 16 bit */ : 16;

// h0068, bit: 4
/* clk_vhe clock setting
[0]:Reserved
[1]:invert clock
[3:2]: select clock source
0 : 216m
1 : 240m
2 : 345m
3 : 384m
4 : 432m
5 : 480m
6 : 288m
7 : 192m
*/
#define offset_of_clkgen_reg_ckg_vhe (208)
#define mask_of_clkgenreg_ckg_vhe    (0x1f)
    unsigned int reg_ckg_vhe : 5;

    // h0068, bit: 6
    /* */
    unsigned int : 2;

// h0068, bit: 7
/* clk_vhe clock gating enable*/
#define offset_of_clkgen_reg_ckg_vhe_gate (208)
#define mask_of_clkgen_reg_ckg_vhe_gate   (0x80)
    unsigned int reg_ckg_vhe_gate : 1;

    // h0068, bit: 14
    /* */
    unsigned int : 8;

    // h0068
    unsigned int /* padding 16 bit */ : 16;

// h0069, bit: 4
/* clk_mfe clock setting
[0]:disable clock
[1]:invert clock
[3:2]: select clock source
0 : 288m
1 : 320m
2 : 432m
3 : 480m
4 : 216m
5 : 384m
6 : 345m
7 : 144m
*/
#define offset_of_clkgen_reg_ckg_mfe (210)
#define mask_of_clkgen_reg_ckg_mfe   (0x1f)
    unsigned int reg_ckg_mfe : 5;

    // h0069, bit: 7
    /* */
    unsigned int : 3;

// h0069, bit: 12
/* clk_ven_axi clock setting
[0]:disable clock
[1]:invert clock
[3:2]: select clock source
0 : 216m
1 : 240m
2 : 345m
3 : 384m
4 : 432m
5 : 480m
6 : 288m
7 : 192m*/
#define offset_of_clkgen_reg_ckg_ven_axi (210)
#define mask_of_clkgen_reg_ckg_ven_axi   (0x1f00)
    unsigned int reg_ckg_ven_axi : 5;

    // h0069, bit: 14
    /* */
    unsigned int : 3;

    // h0069
    unsigned int /* padding 16 bit */ : 16;

// h006a, bit: 3
/* clk_jpe clock setting
[0]:Reserved
[1]:invert clock
[3:2]: select clock source
0 : 192m
1 : 240m
2 : 160m
3 : 123m*/
#define offset_of_clkgen_reg_ckg_jpe (212)
#define mask_of_clkgen_reg_ckg_jpe   (0xf)
    unsigned int reg_ckg_jpe : 4;

    // h006a, bit: 6
    /* */
    unsigned int : 3;

// h006a, bit: 7
/* clk_jpe clock gating enable*/
#define offset_of_clkgen_reg_ckg_jpe_gate (212)
#define mask_of_clkgen_reg_ckg_jpe_gate   (0x80)
    unsigned int reg_ckg_jpe_gate : 1;

// h006a, bit: 11
/* clk_ive clock setting
[0]:disable clock
[1]:invert clock
[3:2]: select clock source
0 : 216m
1 : 288m
2 : 320m
*/
#define offset_of_clkgen_reg_ckg_ive (212)
#define mask_of_clkgen_reg_ckg_ive   (0xf00)
    unsigned int reg_ckg_ive : 4;

    // h006a, bit: 14
    /* */
    unsigned int : 4;

    // h006a
    unsigned int /* padding 16 bit */ : 16;

// h006b, bit: 3
/* clk_ns_p clock setting
[0]: disable clock
[1]: invert clock
[3:2]: select clock source
  00: 12MHz (xtali)
  01: 216MHz*/
#define offset_of_clkgen_reg_ckg_ns (214)
#define mask_of_clkgen_reg_ckg_ns   (0xf)
    unsigned int reg_ckg_ns : 4;

    // h006b, bit: 14
    /* */
    unsigned int : 12;

    // h006b
    unsigned int /* padding 16 bit */ : 16;

// h006c, bit: 4
/* clk_csi_mac_p clock setting
[0]: disable clock
[1]: invert clock
[4:2]: select clock source
  000: 12MHz (xtali)
  001: 216MHz
  010: 288MHz
  011: 172MHz
  100: 123MHz
  101: 86MHz
  others: no clocks*/
#define offset_of_clkgen_reg_ckg_csi_mac (216)
#define mask_of_clkgen_reg_ckg_csi_mac   (0x1f)
    unsigned int reg_ckg_csi_mac : 5;

    // h006c, bit: 7
    /* */
    unsigned int : 3;

// h006c, bit: 12
/* clk_mac_lptx_p clock setting
[0]: disable clock
[1]: invert clock
[4:2]: select clock source
  000: 12MHz (xtali)
  001: 216MHz
  010: 288MHz
  011: 172MHz
  100: 123MHz
  101: 86MHz
  others: no clocks*/
#define offset_of_clkgen_reg_ckg_mac_lptx (216)
#define mask_of_clkgen_reg_ckg_mac_lptx   (0x1f00)
    unsigned int reg_ckg_mac_lptx : 5;

    // h006c, bit: 14
    /* */
    unsigned int : 3;

    // h006c
    unsigned int /* padding 16 bit */ : 16;

// h006d, bit: 0
/* clk_hemcu_216m_p clock setting
[0]: disable clock*/
#define offset_of_clkgen_reg_ckg_hemcu_216m (218)
#define mask_of_clkgen_reg_ckg_hemcu_216m   (0x1)
    unsigned int reg_ckg_hemcu_216m : 1;

// h006d, bit: 1
/* 216m clock to digpm setting, default on
[0]: disable clock*/
#define offset_of_clkgen_reg_ckg_216m_2digpm (218)
#define mask_of_clkgen_reg_ckg_216m_2digpm   (0x2)
    unsigned int reg_ckg_216m_2digpm : 1;

// h006d, bit: 2
/* 172m clock to digpm setting, default off
[0]: disable clock*/
#define offset_of_clkgen_reg_ckg_172m_2digpm (218)
#define mask_of_clkgen_reg_ckg_172m_2digpm   (0x4)
    unsigned int reg_ckg_172m_2digpm : 1;

// h006d, bit: 3
/* 144m clock to digpm setting, default off
[0]: disable clock*/
#define offset_of_clkgen_reg_ckg_144m_2digpm (218)
#define mask_of_clkgen_reg_ckg_144m_2digpm   (0x8)
    unsigned int reg_ckg_144m_2digpm : 1;

// h006d, bit: 4
/* 123m clock to digpm setting, default off
[0]: disable clock*/
#define offset_of_clkgen_reg_ckg_123m_2digpm (218)
#define mask_of_clkgen_reg_ckg_123m_2digpm   (0x10)
    unsigned int reg_ckg_123m_2digpm : 1;

// h006d, bit: 5
/* 86m clock to digpm setting, default off
[0]: disable clock*/
#define offset_of_clkgen_reg_ckg_86m_2digpm (218)
#define mask_of_clkgen_reg_ckg_86m_2digpm   (0x20)
    unsigned int reg_ckg_86m_2digpm : 1;

// h006d, bit: 6
/* 432m clock to bach setting, default off
[0]: disable clock*/
#define offset_of_clkgen_reg_ckg_432m_2bach (218)
#define mask_of_clkgen_reg_ckg_432m_2bach   (0x40)
    unsigned int reg_ckg_432m_2bach : 1;

// h006d, bit: 7
/* 384m clock to bach setting, default off
[0]: disable clock*/
#define offset_of_clkgen_reg_ckg_384m_2bach (218)
#define mask_of_clkgen_reg_ckg_384m_2bach   (0x80)
    unsigned int reg_ckg_384m_2bach : 1;

    // h006d, bit: 14
    /* */
    unsigned int : 8;

    // h006d
    unsigned int /* padding 16 bit */ : 16;

    // h006e, bit: 14
    /* */
    unsigned int : 16;

    // h006e
    unsigned int /* padding 16 bit */ : 16;

// h006f, bit: 0
/* */
#define offset_of_clkgen_reg_clk_sd_dfs_en (222)
#define mask_of_clkgen_reg_clk_sd_dfs_en   (0x1)
    unsigned int reg_clk_sd_dfs_en : 1;

// h006f, bit: 5
/* */
#define offset_of_clkgen_reg_clk_sd_dfs_cfg (222)
#define mask_of_clkgen_reg_clk_sd_dfs_cfg   (0x3e)
    unsigned int reg_clk_sd_dfs_cfg : 5;

    // h006f, bit: 14
    /* */
    unsigned int : 10;

    // h006f
    unsigned int /* padding 16 bit */ : 16;

// h0070, bit: 0
/* one way register to lock all force on registers*/
#define offset_of_clkgen_reg_pll_gater_force_on_lock (224)
#define mask_of_clkgen_reg_pll_gater_force_on_lock   (0x1)
    unsigned int reg_pll_gater_force_on_lock : 1;

// h0070, bit: 1
/* one way register to lock all force off registers*/
#define offset_of_clkgen_reg_pll_gater_force_off_lock (224)
#define mask_of_clkgen_reg_pll_gater_force_off_lock   (0x2)
    unsigned int reg_pll_gater_force_off_lock : 1;

    // h0070, bit: 14
    /* */
    unsigned int : 14;

    // h0070
    unsigned int /* padding 16 bit */ : 16;

// h0071, bit: 0
/* upll_384_force_on*/
#define offset_of_clkgen_reg_upll_384_force_on (226)
#define mask_of_clkgen_reg_upll_384_force_on   (0x1)
    unsigned int reg_upll_384_force_on : 1;

// h0071, bit: 1
/* upll_320_force_on*/
#define offset_of_clkgen_reg_upll_320_force_on (226)
#define mask_of_clkgen_reg_upll_320_force_on   (0x2)
    unsigned int reg_upll_320_force_on : 1;

// h0071, bit: 2
/* upll_160_force_on*/
#define offset_of_clkgen_reg_upll_160_force_on (226)
#define mask_of_clkgen_reg_upll_160_force_on   (0x4)
    unsigned int reg_upll_160_force_on : 1;

// h0071, bit: 3
/* utmi_192_force_on*/
#define offset_of_clkgen_reg_upll_192_force_on (226)
#define mask_of_clkgen_reg_upll_192_force_on   (0x8)
    unsigned int reg_upll_192_force_on : 1;

// h0071, bit: 4
/* utmi_240_force_on*/
#define offset_of_clkgen_reg_upll_240_force_on (226)
#define mask_of_clkgen_reg_upll_240_force_on   (0x10)
    unsigned int reg_upll_240_force_on : 1;

// h0071, bit: 5
/* utmi_480_force_on*/
#define offset_of_clkgen_reg_upll_480_force_on (226)
#define mask_of_clkgen_reg_upll_480_force_on   (0x20)
    unsigned int reg_upll_480_force_on : 1;

// h0071, bit: 6
/* mpll_432_force_on*/
#define offset_of_clkgen_reg_mpll_432_force_on (226)
#define mask_of_clkgen_reg_mpll_432_force_on   (0x40)
    unsigned int reg_mpll_432_force_on : 1;

// h0071, bit: 7
/* mpll_345_force_on*/
#define offset_of_clkgen_reg_mpll_345_force_on (226)
#define mask_of_clkgen_reg_mpll_345_force_on   (0x80)
    unsigned int reg_mpll_345_force_on : 1;

// h0071, bit: 8
/* mpll_288_force_on*/
#define offset_of_clkgen_reg_mpll_288_force_on (226)
#define mask_of_clkgen_reg_mpll_288_force_on   (0x100)
    unsigned int reg_mpll_288_force_on : 1;

// h0071, bit: 9
/* mpll_216_force_on*/
#define offset_of_clkgen_reg_mpll_216_force_on (226)
#define mask_of_clkgen_reg_mpll_216_force_on   (0x200)
    unsigned int reg_mpll_216_force_on : 1;

// h0071, bit: 10
/* mpll_172_force_on*/
#define offset_of_clkgen_reg_mpll_172_force_on (226)
#define mask_of_clkgen_reg_mpll_172_force_on   (0x400)
    unsigned int reg_mpll_172_force_on : 1;

// h0071, bit: 11
/* mpll_144_force_on*/
#define offset_of_clkgen_reg_mpll_144_force_on (226)
#define mask_of_clkgen_reg_mpll_144_force_on   (0x800)
    unsigned int reg_mpll_144_force_on : 1;

// h0071, bit: 12
/* mpll_123_force_on*/
#define offset_of_clkgen_reg_mpll_123_force_on (226)
#define mask_of_clkgen_reg_mpll_123_force_on   (0x1000)
    unsigned int reg_mpll_123_force_on : 1;

// h0071, bit: 13
/* mpll_124_force_on*/
#define offset_of_clkgen_reg_mpll_124_force_on (226)
#define mask_of_clkgen_reg_mpll_124_force_on   (0x2000)
    unsigned int reg_mpll_124_force_on : 1;

// h0071, bit: 14
/* mpll_86_force_on*/
#define offset_of_clkgen_reg_mpll_86_force_on (226)
#define mask_of_clkgen_reg_mpll_86_force_on   (0x4000)
    unsigned int reg_mpll_86_force_on : 1;

// h0071, bit: 15
/* disppll_force_on*/
#define offset_of_clkgen_reg_disppll_force_on (226)
#define mask_of_clkgen_reg_disppll_force_on   (0x8000)
    unsigned int reg_disppll_force_on : 1;

    // h0071
    unsigned int /* padding 16 bit */ : 16;

// h0072, bit: 0
/* upll_384_force_off*/
#define offset_of_clkgen_reg_upll_384_force_off (228)
#define mask_of_clkgen_reg_upll_384_force_off   (0x1)
    unsigned int reg_upll_384_force_off : 1;

// h0072, bit: 1
/* upll_320_force_off*/
#define offset_of_clkgen_reg_upll_320_force_off (228)
#define mask_of_clkgen_reg_upll_320_force_off   (0x2)
    unsigned int reg_upll_320_force_off : 1;

// h0072, bit: 2
/* utmi_160_force_off*/
#define offset_of_clkgen_reg_upll_160_force_off (228)
#define mask_of_clkgen_reg_upll_160_force_off   (0x4)
    unsigned int reg_upll_160_force_off : 1;

// h0072, bit: 3
/* utmi_192_force_off*/
#define offset_of_clkgen_reg_upll_192_force_off (228)
#define mask_of_clkgen_reg_upll_192_force_off   (0x8)
    unsigned int reg_upll_192_force_off : 1;

// h0072, bit: 4
/* utmi_240_force_off*/
#define offset_of_clkgen_reg_upll_240_force_off (228)
#define mask_of_clkgen_reg_upll_240_force_off   (0x10)
    unsigned int reg_upll_240_force_off : 1;

// h0072, bit: 5
/* utmi_480_force_off*/
#define offset_of_clkgen_reg_upll_480_force_off (228)
#define mask_of_clkgen_reg_upll_480_force_off   (0x20)
    unsigned int reg_upll_480_force_off : 1;

// h0072, bit: 6
/* mpll_432_force_off*/
#define offset_of_clkgen_reg_mpll_432_force_off (228)
#define mask_of_clkgen_reg_mpll_432_force_off   (0x40)
    unsigned int reg_mpll_432_force_off : 1;

// h0072, bit: 7
/* mpll_345_force_off*/
#define offset_of_clkgen_reg_mpll_345_force_off (228)
#define mask_of_clkgen_reg_mpll_345_force_off   (0x80)
    unsigned int reg_mpll_345_force_off : 1;

// h0072, bit: 8
/* mpll_288_force_off*/
#define offset_of_clkgen_reg_mpll_288_force_off (228)
#define mask_of_clkgen_reg_mpll_288_force_off   (0x100)
    unsigned int reg_mpll_288_force_off : 1;

// h0072, bit: 9
/* mpll_216_force_off*/
#define offset_of_clkgen_reg_mpll_216_force_off (228)
#define mask_of_clkgen_reg_mpll_216_force_off   (0x200)
    unsigned int reg_mpll_216_force_off : 1;

// h0072, bit: 10
/* mpll_172_force_off*/
#define offset_of_clkgen_reg_mpll_172_force_off (228)
#define mask_of_clkgen_reg_mpll_172_force_off   (0x400)
    unsigned int reg_mpll_172_force_off : 1;

// h0072, bit: 11
/* mpll_144_force_off*/
#define offset_of_clkgen_reg_mpll_144_force_off (228)
#define mask_of_clkgen_reg_mpll_144_force_off   (0x800)
    unsigned int reg_mpll_144_force_off : 1;

// h0072, bit: 12
/* mpll_123_force_off*/
#define offset_of_clkgen_reg_mpll_123_force_off (228)
#define mask_of_clkgen_reg_mpll_123_force_off   (0x1000)
    unsigned int reg_mpll_123_force_off : 1;

// h0072, bit: 13
/* mpll_124_force_off*/
#define offset_of_clkgen_reg_mpll_124_force_off (228)
#define mask_of_clkgen_reg_mpll_124_force_off   (0x2000)
    unsigned int reg_mpll_124_force_off : 1;

// h0072, bit: 14
/* mpll_86_force_off*/
#define offset_of_clkgen_reg_mpll_86_force_off (228)
#define mask_of_clkgen_reg_mpll_86_force_off   (0x4000)
    unsigned int reg_mpll_86_force_off : 1;

// h0072, bit: 15
/* disppll_force_off*/
#define offset_of_clkgen_reg_disppll_force_off (228)
#define mask_of_clkgen_reg_disppll_force_off   (0x8000)
    unsigned int reg_disppll_force_off : 1;

    // h0072
    unsigned int /* padding 16 bit */ : 16;

// h0073, bit: 0
/* read back upll_384_en */
#define offset_of_clkgen_reg_upll_384_en_rd (230)
#define mask_of_clkgen_reg_upll_384_en_rd   (0x1)
    unsigned int reg_upll_384_en_rd : 1;

// h0073, bit: 1
/* read back upll_320_en */
#define offset_of_clkgen_reg_upll_320_en_rd (230)
#define mask_of_clkgen_reg_upll_320_en_rd   (0x2)
    unsigned int reg_upll_320_en_rd : 1;

// h0073, bit: 2
/* read back utmi_160_en */
#define offset_of_clkgen_reg_upll_160_en_rd (230)
#define mask_of_clkgen_reg_upll_160_en_rd   (0x4)
    unsigned int reg_upll_160_en_rd : 1;

// h0073, bit: 3
/* read back utmi_192_en */
#define offset_of_clkgen_reg_upll_192_en_rd (230)
#define mask_of_clkgen_reg_upll_192_en_rd   (0x8)
    unsigned int reg_upll_192_en_rd : 1;

// h0073, bit: 4
/* read back utmi_240_en */
#define offset_of_clkgen_reg_upll_240_en_rd (230)
#define mask_of_clkgen_reg_upll_240_en_rd   (0x10)
    unsigned int reg_upll_240_en_rd : 1;

// h0073, bit: 5
/* read back utmi_480_en */
#define offset_of_clkgen_reg_upll_480_en_rd (230)
#define mask_of_clkgen_reg_upll_480_en_rd   (0x20)
    unsigned int reg_upll_480_en_rd : 1;

// h0073, bit: 6
/* read back mpll_432_en */
#define offset_of_clkgen_reg_mpll_432_en_rd (230)
#define mask_of_clkgen_reg_mpll_432_en_rd   (0x40)
    unsigned int reg_mpll_432_en_rd : 1;

    // h0073, bit: 7
    /* */
    unsigned int : 1;

// h0073, bit: 8
/* read back mpll_288_en */
#define offset_of_clkgen_reg_mpll_288_en_rd (230)
#define mask_of_clkgen_reg_mpll_288_en_rd   (0x100)
    unsigned int reg_mpll_288_en_rd : 1;

// h0073, bit: 9
/* read back mpll_216_en */
#define offset_of_clkgen_reg_mpll_216_en_rd (230)
#define mask_of_clkgen_reg_mpll_216_en_rd   (0x200)
    unsigned int reg_mpll_216_en_rd : 1;

// h0073, bit: 10
/* read back mpll_172_en */
#define offset_of_clkgen_reg_mpll_172_en_rd (230)
#define mask_of_clkgen_reg_mpll_172_en_rd   (0x400)
    unsigned int reg_mpll_172_en_rd : 1;

// h0073, bit: 11
/* read back mpll_144_en */
#define offset_of_clkgen_reg_mpll_144_en_rd (230)
#define mask_of_clkgen_reg_mpll_144_en_rd   (0x800)
    unsigned int reg_mpll_144_en_rd : 1;

// h0073, bit: 12
/* read back mpll_123_en */
#define offset_of_clkgen_reg_mpll_123_en_rd (230)
#define mask_of_clkgen_reg_mpll_123_en_rd   (0x1000)
    unsigned int reg_mpll_123_en_rd : 1;

    // h0073, bit: 13
    /* */
    unsigned int : 1;

// h0073, bit: 14
/* read back mpll_86_en */
#define offset_of_clkgen_reg_mpll_86_en_rd (230)
#define mask_of_clkgen_reg_mpll_86_en_rd   (0x4000)
    unsigned int reg_mpll_86_en_rd : 1;

    // h0073, bit: 15
    /* */
    unsigned int : 1;

    // h0073
    unsigned int /* padding 16 bit */ : 16;

// h0074, bit: 3
/* clkgen testbus selection*/
#define offset_of_clkgen_reg_testclk_sel (232)
#define mask_of_clkgen_reg_testclk_sel   (0xf)
    unsigned int reg_testclk_sel : 4;

    // h0074, bit: 7
    /* */
    unsigned int : 4;

// h0074, bit: 8
/* reg_armpll_37p125m_force_off*/
#define offset_of_clkgen_reg_armpll_37p125m_force_off (232)
#define mask_of_clkgen_reg_armpll_37p125m_force_off   (0x100)
    unsigned int reg_armpll_37p125m_force_off : 1;

// h0074, bit: 9
/* reg_spipll_force_off*/
#define offset_of_clkgen_reg_spipll_force_off (232)
#define mask_of_clkgen_reg_spipll_force_off   (0x200)
    unsigned int reg_spipll_force_off : 1;

// h0074, bit: 10
/* reg_miupll_force_off*/
#define offset_of_clkgen_reg_miupll_force_off (232)
#define mask_of_clkgen_reg_miupll_force_off   (0x400)
    unsigned int reg_miupll_force_off : 1;

// h0074, bit: 11
/* reg_armpll_37p125m_force_on*/
#define offset_of_clkgen_reg_armpll_37p125m_force_on (232)
#define mask_of_clkgen_reg_armpll_37p125m_force_on   (0x800)
    unsigned int reg_armpll_37p125m_force_on : 1;

// h0074, bit: 12
/* reg_spipll_force_on*/
#define offset_of_clkgen_reg_spipll_force_on (232)
#define mask_of_clkgen_reg_spipll_force_on   (0x1000)
    unsigned int reg_spipll_force_on : 1;

// h0074, bit: 13
/* reg_miupll_force_on*/
#define offset_of_clkgen_reg_miupll_force_on (232)
#define mask_of_clkgen_reg_miupll_force_on   (0x2000)
    unsigned int reg_miupll_force_on : 1;

    // h0074, bit: 14
    /* */
    unsigned int : 2;

    // h0074
    unsigned int /* padding 16 bit */ : 16;

// h0075, bit: 0
/* */
#define offset_of_clkgen_reg_clk_isp_dfs_en (234)
#define mask_of_clkgen_reg_clk_isp_dfs_en   (0x1)
    unsigned int reg_clk_isp_dfs_en : 1;

// h0075, bit: 5
/* */
#define offset_of_clkgen_reg_clk_isp_dfs_cfg (234)
#define mask_of_clkgen_reg_clk_isp_dfs_cfg   (0x3e)
    unsigned int reg_clk_isp_dfs_cfg : 5;

    // h0075, bit: 7
    /* */
    unsigned int : 2;

// h0075, bit: 8
/* */
#define offset_of_clkgen_reg_clk_emac_dfs_en (234)
#define mask_of_clkgen_reg_clk_emac_dfs_en   (0x100)
    unsigned int reg_clk_emac_dfs_en : 1;

// h0075, bit: 13
/* */
#define offset_of_clkgen_reg_clk_emac_dfs_cfg (234)
#define mask_of_clkgen_reg_clk_emac_dfs_cfg   (0x3e00)
    unsigned int reg_clk_emac_dfs_cfg : 5;

    // h0075, bit: 14
    /* */
    unsigned int : 2;

    // h0075
    unsigned int /* padding 16 bit */ : 16;

// h0076, bit: 0
/* */
#define offset_of_clkgen_reg_clk_sc_dfs_en (236)
#define mask_of_clkgen_reg_clk_sc_dfs_en   (0x1)
    unsigned int reg_clk_sc_dfs_en : 1;

// h0076, bit: 5
/* */
#define offset_of_clkgen_reg_clk_sc_dfs_cfg (236)
#define mask_of_clkgen_reg_clk_sc_dfs_cfg   (0x3e)
    unsigned int reg_clk_sc_dfs_cfg : 5;

    // h0076, bit: 7
    /* */
    unsigned int : 2;

// h0076, bit: 8
/* */
#define offset_of_clkgen_reg_clk_sr00_mclk_dfs_en (236)
#define mask_of_clkgen_reg_clk_sr00_mclk_dfs_en   (0x100)
    unsigned int reg_clk_sr00_mclk_dfs_en : 1;

// h0076, bit: 13
/* */
#define offset_of_clkgen_reg_clk_sr00_mclk_dfs_cfg (236)
#define mask_of_clkgen_reg_clk_sr00_mclk_dfs_cfg   (0x3e00)
    unsigned int reg_clk_sr00_mclk_dfs_cfg : 5;

    // h0076, bit: 14
    /* */
    unsigned int : 2;

    // h0076
    unsigned int /* padding 16 bit */ : 16;

// h0077, bit: 0
/* */
#define offset_of_clkgen_reg_clk_disp_pixel0_dfs_en (238)
#define mask_of_clkgen_reg_clk_disp_pixel0_dfs_en   (0x1)
    unsigned int reg_clk_disp_pixel0_dfs_en : 1;

// h0077, bit: 5
/* */
#define offset_of_clkgen_reg_clk_disp_pixel0_dfs_cfg (238)
#define mask_of_clkgen_reg_clk_disp_pixel0_dfs_cfg   (0x3e)
    unsigned int reg_clk_disp_pixel0_dfs_cfg : 5;

    // h0077, bit: 7
    /* */
    unsigned int : 2;

// h0077, bit: 8
/* */
#define offset_of_clkgen_reg_clk_sr01_mclk_dfs_en (238)
#define mask_of_clkgen_reg_clk_sr01_mclk_dfs_en   (0x100)
    unsigned int reg_clk_sr01_mclk_dfs_en : 1;

// h0077, bit: 13
/* */
#define offset_of_clkgen_reg_clk_sr01_mclk_dfs_cfg (238)
#define mask_of_clkgen_reg_clk_sr01_mclk_dfs_cfg   (0x3e00)
    unsigned int reg_clk_sr01_mclk_dfs_cfg : 5;

    // h0077, bit: 14
    /* */
    unsigned int : 2;

    // h0077
    unsigned int /* padding 16 bit */ : 16;

// h0078, bit: 0
/* */
#define offset_of_clkgen_reg_clk_ldcfeye_dfs_en (240)
#define mask_of_clkgen_reg_clk_ldcfeye_dfs_en   (0x1)
    unsigned int reg_clk_ldcfeye_dfs_en : 1;

// h0078, bit: 5
/* */
#define offset_of_clkgen_reg_clk_ldcfeye_dfs_cfg (240)
#define mask_of_clkgen_reg_clk_ldcfeye_dfs_cfg   (0x3e)
    unsigned int reg_clk_ldcfeye_dfs_cfg : 5;

    // h0078, bit: 7
    /* */
    unsigned int : 2;

// h0078, bit: 8
/* */
#define offset_of_clkgen_reg_clk_sr02_mclk_dfs_en (240)
#define mask_of_clkgen_reg_clk_sr02_mclk_dfs_en   (0x100)
    unsigned int reg_clk_sr02_mclk_dfs_en : 1;

// h0078, bit: 13
/* */
#define offset_of_clkgen_reg_clk_sr02_mclk_dfs_cfg (240)
#define mask_of_clkgen_reg_clk_sr02_mclk_dfs_cfg   (0x3e00)
    unsigned int reg_clk_sr02_mclk_dfs_cfg : 5;

    // h0078, bit: 14
    /* */
    unsigned int : 2;

    // h0078
    unsigned int /* padding 16 bit */ : 16;

// h0079, bit: 0
/* */
#define offset_of_clkgen_reg_clk_ive_dfs_en (242)
#define mask_of_clkgen_reg_clk_ive_dfs_en   (0x1)
    unsigned int reg_clk_ive_dfs_en : 1;

// h0079, bit: 5
/* */
#define offset_of_clkgen_reg_clk_ive_dfs_cfg (242)
#define mask_of_clkgen_reg_clk_ive_dfs_cfg   (0x3e)
    unsigned int reg_clk_ive_dfs_cfg : 5;

    // h0079, bit: 7
    /* */
    unsigned int : 2;

// h0079, bit: 8
/* */
#define offset_of_clkgen_reg_clk_sr03_mclk_dfs_en (242)
#define mask_of_clkgen_reg_clk_sr03_mclk_dfs_en   (0x100)
    unsigned int reg_clk_sr03_mclk_dfs_en : 1;

// h0079, bit: 13
/* */
#define offset_of_clkgen_reg_clk_sr03_mclk_dfs_cfg (242)
#define mask_of_clkgen_reg_clk_sr03_mclk_dfs_cfg   (0x3e00)
    unsigned int reg_clk_sr03_mclk_dfs_cfg : 5;

    // h0079, bit: 14
    /* */
    unsigned int : 2;

    // h0079
    unsigned int /* padding 16 bit */ : 16;

// h007a, bit: 0
/* */
#define offset_of_clkgen_reg_clk_aesdma_dfs_en (244)
#define mask_of_clkgen_reg_clk_aesdma_dfs_en   (0x1)
    unsigned int reg_clk_aesdma_dfs_en : 1;

// h007a, bit: 5
/* */
#define offset_of_clkgen_reg_clk_aesdma_dfs_cfg (244)
#define mask_of_clkgen_reg_clk_aesdma_dfs_cfg   (0x3e)
    unsigned int reg_clk_aesdma_dfs_cfg : 5;

    // h007a, bit: 7
    /* */
    unsigned int : 2;

// h007a, bit: 8
/* */
#define offset_of_clkgen_reg_clk_csi0_mac_lptx_dfs_en (244)
#define mask_of_clkgen_reg_clk_csi0_mac_lptx_dfs_en   (0x100)
    unsigned int reg_clk_csi0_mac_lptx_dfs_en : 1;

// h007a, bit: 13
/* */
#define offset_of_clkgen_reg_clk_csi0_mac_lptx_dfs_cfg (244)
#define mask_of_clkgen_reg_clk_csi0_mac_lptx_dfs_cfg   (0x3e00)
    unsigned int reg_clk_csi0_mac_lptx_dfs_cfg : 5;

    // h007a, bit: 14
    /* */
    unsigned int : 2;

    // h007a
    unsigned int /* padding 16 bit */ : 16;

// h007b, bit: 0
/* */
#define offset_of_clkgen_reg_clk_jpe_dfs_en (246)
#define mask_of_clkgen_reg_clk_jpe_dfs_en   (0x1)
    unsigned int reg_clk_jpe_dfs_en : 1;

// h007b, bit: 5
/* */
#define offset_of_clkgen_reg_clk_jpe_dfs_cfg (246)
#define mask_of_clkgen_reg_clk_jpe_dfs_cfg   (0x3e)
    unsigned int reg_clk_jpe_dfs_cfg : 5;

    // h007b, bit: 7
    /* */
    unsigned int : 2;

// h007b, bit: 8
/* */
#define offset_of_clkgen_reg_clk_csi0_ns_top_i_dfs_en (246)
#define mask_of_clkgen_reg_clk_csi0_ns_top_i_dfs_en   (0x100)
    unsigned int reg_clk_csi0_ns_top_i_dfs_en : 1;

// h007b, bit: 13
/* */
#define offset_of_clkgen_reg_clk_csi0_ns_top_i_dfs_cfg (246)
#define mask_of_clkgen_reg_clk_csi0_ns_top_i_dfs_cfg   (0x3e00)
    unsigned int reg_clk_csi0_ns_top_i_dfs_cfg : 5;

    // h007b, bit: 14
    /* */
    unsigned int : 2;

    // h007b
    unsigned int /* padding 16 bit */ : 16;

// h007c, bit: 0
/* */
#define offset_of_clkgen_reg_clk_csi0_mac_top_i_dfs_en (248)
#define mask_of_clkgen_reg_clk_csi0_mac_top_i_dfs_en   (0x1)
    unsigned int reg_clk_csi0_mac_top_i_dfs_en : 1;

// h007c, bit: 5
/* */
#define offset_of_clkgen_reg_clk_csi0_mac_top_i_dfs_cfg (248)
#define mask_of_clkgen_reg_clk_csi0_mac_top_i_dfs_cfg   (0x3e)
    unsigned int reg_clk_csi0_mac_top_i_dfs_cfg : 5;

    // h007c, bit: 7
    /* */
    unsigned int : 2;

// h007c, bit: 8
/* */
#define offset_of_clkgen_reg_clk_csi1_mac_lptx_dfs_en (248)
#define mask_of_clkgen_reg_clk_csi1_mac_lptx_dfs_en   (0x100)
    unsigned int reg_clk_csi1_mac_lptx_dfs_en : 1;

// h007c, bit: 13
/* */
#define offset_of_clkgen_reg_clk_csi1_mac_lptx_dfs_cfg (248)
#define mask_of_clkgen_reg_clk_csi1_mac_lptx_dfs_cfg   (0x3e00)
    unsigned int reg_clk_csi1_mac_lptx_dfs_cfg : 5;

    // h007c, bit: 14
    /* */
    unsigned int : 2;

    // h007c
    unsigned int /* padding 16 bit */ : 16;

// h007d, bit: 0
/* */
#define offset_of_clkgen_reg_clk_csi1_ns_top_i_dfs_en (250)
#define mask_of_clkgen_reg_clk_csi1_ns_top_i_dfs_en   (0x1)
    unsigned int reg_clk_csi1_ns_top_i_dfs_en : 1;

// h007d, bit: 5
/* */
#define offset_of_clkgen_reg_clk_csi1_ns_top_i_dfs_cfg (250)
#define mask_of_clkgen_reg_clk_csi1_ns_top_i_dfs_cfg   (0x3e)
    unsigned int reg_clk_csi1_ns_top_i_dfs_cfg : 5;

    // h007d, bit: 7
    /* */
    unsigned int : 2;

// h007d, bit: 8
/* */
#define offset_of_clkgen_reg_clk_csi1_mac_top_i_dfs_en (250)
#define mask_of_clkgen_reg_clk_csi1_mac_top_i_dfs_en   (0x100)
    unsigned int reg_clk_csi1_mac_top_i_dfs_en : 1;

// h007d, bit: 13
/* */
#define offset_of_clkgen_reg_clk_csi1_mac_top_i_dfs_cfg (250)
#define mask_of_clkgen_reg_clk_csi1_mac_top_i_dfs_cfg   (0x3e00)
    unsigned int reg_clk_csi1_mac_top_i_dfs_cfg : 5;

    // h007d, bit: 14
    /* */
    unsigned int : 2;

    // h007d
    unsigned int /* padding 16 bit */ : 16;

// h007e, bit: 14
/* Reserved Registers*/
#define offset_of_clkgen_reg_clkgen0_reserved0 (252)
#define mask_of_clkgen_reg_clkgen0_reserved0   (0xffff)
    unsigned int reg_clkgen0_reserved0 : 16;

    // h007e
    unsigned int /* padding 16 bit */ : 16;

// h007f, bit: 14
/* Reserved Registers*/
#define offset_of_clkgen_reg_clkgen0_reserved1 (254)
#define mask_of_clkgen_reg_clkgen0_reserved1   (0xffff)
    unsigned int reg_clkgen0_reserved1 : 16;

    // h007f
    unsigned int /* padding 16 bit */ : 16;

} __attribute__((packed, aligned(1))) reg_CLKGEN;
#endif
