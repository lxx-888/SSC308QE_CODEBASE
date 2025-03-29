/*
 * reg_padtop.h - Sigmastar
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

#ifndef __REG_PADTOP__
#define __REG_PADTOP__
typedef struct
{
// h0000, bit: 14
/* test bus*/
#define offset_of_padtop_reg_test_out (0)
#define mask_of_padtop_reg_test_out   (0xffff)
    unsigned int reg_test_out : 16;

    // h0000
    unsigned int /* padding 16 bit */ : 16;

// h0001, bit: 7
/* test bus*/
#define offset_of_padtop_reg_test_out_1 (2)
#define mask_of_padtop_reg_test_out_1   (0xff)
    unsigned int reg_test_out_1 : 8;

    // h0001, bit: 14
    /* */
    unsigned int : 8;

    // h0001
    unsigned int /* padding 16 bit */ : 16;

// h0002, bit: 14
/* dummy register*/
#define offset_of_padtop_reg_padmux_dummy (4)
#define mask_of_padtop_reg_padmux_dummy   (0xffff)
    unsigned int reg_padmux_dummy : 16;

    // h0002
    unsigned int /* padding 16 bit */ : 16;

// h0003, bit: 0
/* TEST*/
#define offset_of_padtop_reg_gpio_st_test (6)
#define mask_of_padtop_reg_gpio_st_test   (0x1)
    unsigned int reg_gpio_st_test : 1;

// h0003, bit: 1
/* TEST*/
#define offset_of_padtop_reg_gpio_drv0_test (6)
#define mask_of_padtop_reg_gpio_drv0_test   (0x2)
    unsigned int reg_gpio_drv0_test : 1;

// h0003, bit: 2
/* TEST*/
#define offset_of_padtop_reg_gpio_drv1_test (6)
#define mask_of_padtop_reg_gpio_drv1_test   (0x4)
    unsigned int reg_gpio_drv1_test : 1;

// h0003, bit: 3
/* TEST*/
#define offset_of_padtop_reg_gpio_drv2_test (6)
#define mask_of_padtop_reg_gpio_drv2_test   (0x8)
    unsigned int reg_gpio_drv2_test : 1;

// h0003, bit: 4
/* TEST*/
#define offset_of_padtop_reg_gpio_drv3_test (6)
#define mask_of_padtop_reg_gpio_drv3_test   (0x10)
    unsigned int reg_gpio_drv3_test : 1;

// h0003, bit: 5
/* TEST*/
#define offset_of_padtop_reg_gpio_ps_test (6)
#define mask_of_padtop_reg_gpio_ps_test   (0x20)
    unsigned int reg_gpio_ps_test : 1;

// h0003, bit: 6
/* TEST*/
#define offset_of_padtop_reg_gpio_pe_test (6)
#define mask_of_padtop_reg_gpio_pe_test   (0x40)
    unsigned int reg_gpio_pe_test : 1;

// h0003, bit: 7
/* TEST*/
#define offset_of_padtop_reg_gpio_ie_test (6)
#define mask_of_padtop_reg_gpio_ie_test   (0x80)
    unsigned int reg_gpio_ie_test : 1;

    // h0003, bit: 14
    /* */
    unsigned int : 8;

    // h0003
    unsigned int /* padding 16 bit */ : 16;

// h0004, bit: 5
/* GPIO 3318 mode*/
#define offset_of_padtop_reg_ms_sel (8)
#define mask_of_padtop_reg_ms_sel   (0x3f)
    unsigned int reg_ms_sel : 6;

    // h0004, bit: 14
    /* */
    unsigned int : 10;

    // h0004
    unsigned int /* padding 16 bit */ : 16;

// h0005, bit: 5
/* GPIO IRTE mode*/
#define offset_of_padtop_reg_ms_reg (10)
#define mask_of_padtop_reg_ms_reg   (0x3f)
    unsigned int reg_ms_reg : 6;

    // h0005, bit: 14
    /* */
    unsigned int : 10;

    // h0005
    unsigned int /* padding 16 bit */ : 16;

    // h0006, bit: 14
    /* */
    unsigned int : 16;

    // h0006
    unsigned int /* padding 16 bit */ : 16;

    // h0007, bit: 14
    /* */
    unsigned int : 16;

    // h0007
    unsigned int /* padding 16 bit */ : 16;

    // h0008, bit: 14
    /* */
    unsigned int : 16;

    // h0008
    unsigned int /* padding 16 bit */ : 16;

    // h0009, bit: 14
    /* */
    unsigned int : 16;

    // h0009
    unsigned int /* padding 16 bit */ : 16;

    // h000a, bit: 14
    /* */
    unsigned int : 16;

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
/* BT656/1120 SR0 clk*/
#define offset_of_padtop_reg_ckg_sr0_clk (32)
#define mask_of_padtop_reg_ckg_sr0_clk   (0xf)
    unsigned int reg_ckg_sr0_clk : 4;

// h0010, bit: 7
/* BT656/1120 SR1 clk*/
#define offset_of_padtop_reg_ckg_sr1_clk (32)
#define mask_of_padtop_reg_ckg_sr1_clk   (0xf0)
    unsigned int reg_ckg_sr1_clk : 4;

// h0010, bit: 11
/* BT656/1120 SR2 clk*/
#define offset_of_padtop_reg_ckg_sr2_clk (32)
#define mask_of_padtop_reg_ckg_sr2_clk   (0xf00)
    unsigned int reg_ckg_sr2_clk : 4;

// h0010, bit: 14
/* BT656/1120 SR3 clk*/
#define offset_of_padtop_reg_ckg_sr3_clk (32)
#define mask_of_padtop_reg_ckg_sr3_clk   (0xf000)
    unsigned int reg_ckg_sr3_clk : 4;

    // h0010
    unsigned int /* padding 16 bit */ : 16;

// h0011, bit: 3
/* BT656/1120 SR4 clk*/
#define offset_of_padtop_reg_ckg_sr4_clk (34)
#define mask_of_padtop_reg_ckg_sr4_clk   (0xf)
    unsigned int reg_ckg_sr4_clk : 4;

// h0011, bit: 7
/* BT656/1120 SR5 clk*/
#define offset_of_padtop_reg_ckg_sr5_clk (34)
#define mask_of_padtop_reg_ckg_sr5_clk   (0xf0)
    unsigned int reg_ckg_sr5_clk : 4;

// h0011, bit: 11
/* BT656/1120 SR6 clk*/
#define offset_of_padtop_reg_ckg_sr6_clk (34)
#define mask_of_padtop_reg_ckg_sr6_clk   (0xf00)
    unsigned int reg_ckg_sr6_clk : 4;

// h0011, bit: 14
/* BT656/1120 SR7 clk*/
#define offset_of_padtop_reg_ckg_sr7_clk (34)
#define mask_of_padtop_reg_ckg_sr7_clk   (0xf000)
    unsigned int reg_ckg_sr7_clk : 4;

    // h0011
    unsigned int /* padding 16 bit */ : 16;

// h0012, bit: 3
/* BT656/1120 SR clk*/
#define offset_of_padtop_reg_ckg_sr4_bt1120_clk (36)
#define mask_of_padtop_reg_ckg_sr4_bt1120_clk   (0xf)
    unsigned int reg_ckg_sr4_bt1120_clk : 4;

    // h0012, bit: 7
    /* */
    unsigned int : 4;

// h0012, bit: 8
/* BT656/1120 SR PLL clk en*/
#define offset_of_padtop_reg_sr0_pll_rx_en (36)
#define mask_of_padtop_reg_sr0_pll_rx_en   (0x100)
    unsigned int reg_sr0_pll_rx_en : 1;

    // h0012, bit: 14
    /* */
    unsigned int : 7;

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

    // h0017, bit: 14
    /* */
    unsigned int : 16;

    // h0017
    unsigned int /* padding 16 bit */ : 16;

// h0018, bit: 0
/* BT656/1120 SR0 RX enable*/
#define offset_of_padtop_reg_isp0_itu_en (48)
#define mask_of_padtop_reg_isp0_itu_en   (0x1)
    unsigned int reg_isp0_itu_en : 1;

    // h0018, bit: 7
    /* */
    unsigned int : 7;

// h0018, bit: 14
/* */
#define offset_of_padtop_reg_isp0_dummy_mode (48)
#define mask_of_padtop_reg_isp0_dummy_mode   (0xff00)
    unsigned int reg_isp0_dummy_mode : 8;

    // h0018
    unsigned int /* padding 16 bit */ : 16;

// h0019, bit: 0
/* BT656/1120 SR1 RX enable*/
#define offset_of_padtop_reg_isp1_itu_en (50)
#define mask_of_padtop_reg_isp1_itu_en   (0x1)
    unsigned int reg_isp1_itu_en : 1;

    // h0019, bit: 7
    /* */
    unsigned int : 7;

// h0019, bit: 14
/* */
#define offset_of_padtop_reg_isp1_dummy_mode (50)
#define mask_of_padtop_reg_isp1_dummy_mode   (0xff00)
    unsigned int reg_isp1_dummy_mode : 8;

    // h0019
    unsigned int /* padding 16 bit */ : 16;

// h001a, bit: 5
/* for audio pad enable*/
#define offset_of_padtop_reg_aud_pad_en (52)
#define mask_of_padtop_reg_aud_pad_en   (0x3f)
    unsigned int reg_aud_pad_en : 6;

    // h001a, bit: 14
    /* */
    unsigned int : 10;

    // h001a
    unsigned int /* padding 16 bit */ : 16;

// h001b, bit: 14
/* GPIO3318 testmode - internal clk*/
#define offset_of_padtop_reg_gpio3318_test_mode3_en (54)
#define mask_of_padtop_reg_gpio3318_test_mode3_en   (0xffff)
    unsigned int reg_gpio3318_test_mode3_en : 16;

    // h001b
    unsigned int /* padding 16 bit */ : 16;

// h001c, bit: 11
/* GPIO3318 testmode - c-i*/
#define offset_of_padtop_reg_gpio3318_test_mode2_en (56)
#define mask_of_padtop_reg_gpio3318_test_mode2_en   (0xfff)
    unsigned int reg_gpio3318_test_mode2_en : 12;

    // h001c, bit: 14
    /* */
    unsigned int : 4;

    // h001c
    unsigned int /* padding 16 bit */ : 16;

// h001d, bit: 11
/* GPIO3318 testmode - internal clk*/
#define offset_of_padtop_reg_gpio3318_test_mode4_en (58)
#define mask_of_padtop_reg_gpio3318_test_mode4_en   (0xfff)
    unsigned int reg_gpio3318_test_mode4_en : 12;

    // h001d, bit: 14
    /* */
    unsigned int : 4;

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

    // h0020, bit: 14
    /* */
    unsigned int : 16;

    // h0020
    unsigned int /* padding 16 bit */ : 16;

    // h0021, bit: 14
    /* */
    unsigned int : 16;

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

    // h002d, bit: 14
    /* */
    unsigned int : 16;

    // h002d
    unsigned int /* padding 16 bit */ : 16;

    // h002e, bit: 14
    /* */
    unsigned int : 16;

    // h002e
    unsigned int /* padding 16 bit */ : 16;

    // h002f, bit: 14
    /* */
    unsigned int : 16;

    // h002f
    unsigned int /* padding 16 bit */ : 16;

// h0030, bit: 14
/* MIPI RX 0 gpio mode en*/
#define offset_of_padtop_reg_mipi_rx0_gpio_en (96)
#define mask_of_padtop_reg_mipi_rx0_gpio_en   (0xffff)
    unsigned int reg_mipi_rx0_gpio_en : 16;

    // h0030
    unsigned int /* padding 16 bit */ : 16;

// h0031, bit: 14
/* MIPI RX 1 gpio mode en*/
#define offset_of_padtop_reg_mipi_rx1_gpio_en (98)
#define mask_of_padtop_reg_mipi_rx1_gpio_en   (0xffff)
    unsigned int reg_mipi_rx1_gpio_en : 16;

    // h0031
    unsigned int /* padding 16 bit */ : 16;

// h0032, bit: 14
/* MIPI RX 2 gpio mode en*/
#define offset_of_padtop_reg_mipi_rx2_gpio_en (100)
#define mask_of_padtop_reg_mipi_rx2_gpio_en   (0xffff)
    unsigned int reg_mipi_rx2_gpio_en : 16;

    // h0032
    unsigned int /* padding 16 bit */ : 16;

// h0033, bit: 14
/* MIPI RX 3 gpio mode en*/
#define offset_of_padtop_reg_mipi_rx3_gpio_en (102)
#define mask_of_padtop_reg_mipi_rx3_gpio_en   (0xffff)
    unsigned int reg_mipi_rx3_gpio_en : 16;

    // h0033
    unsigned int /* padding 16 bit */ : 16;

// h0034, bit: 14
/* MIPI TX gpio mode en*/
#define offset_of_padtop_reg_mipi_tx_gpio_en (104)
#define mask_of_padtop_reg_mipi_tx_gpio_en   (0xffff)
    unsigned int reg_mipi_tx_gpio_en : 16;

    // h0034
    unsigned int /* padding 16 bit */ : 16;

// h0035, bit: 0
/* BT1120 Mode
Reference IO setting table*/
#define offset_of_padtop_reg_bt1120_ext_mode (106)
#define mask_of_padtop_reg_bt1120_ext_mode   (0x1)
    unsigned int reg_bt1120_ext_mode : 1;

    // h0035, bit: 3
    /* */
    unsigned int : 3;

// h0035, bit: 4
/* BT1120 Mode
Reference IO setting table*/
#define offset_of_padtop_reg_bt1120_mode (106)
#define mask_of_padtop_reg_bt1120_mode   (0x10)
    unsigned int reg_bt1120_mode : 1;

    // h0035, bit: 7
    /* */
    unsigned int : 3;

// h0035, bit: 9
/* BT1120 Mode
Reference IO setting table*/
#define offset_of_padtop_reg_bt1120_phase_mode (106)
#define mask_of_padtop_reg_bt1120_phase_mode   (0x300)
    unsigned int reg_bt1120_phase_mode : 2;

    // h0035, bit: 11
    /* */
    unsigned int : 2;

// h0035, bit: 12
/* BT1120 Mode
Reference IO setting table*/
#define offset_of_padtop_reg_bt1120_sync_mode (106)
#define mask_of_padtop_reg_bt1120_sync_mode   (0x1000)
    unsigned int reg_bt1120_sync_mode : 1;

    // h0035, bit: 14
    /* */
    unsigned int : 2;

// h0035, bit: 15
/* BT1120 Mode
Reference IO setting table*/
#define offset_of_padtop_reg_bt1120_sw_rst (106)
#define mask_of_padtop_reg_bt1120_sw_rst   (0x8000)
    unsigned int reg_bt1120_sw_rst : 1;

    // h0035
    unsigned int /* padding 16 bit */ : 16;

// h0036, bit: 9
/* EMAC GPIO external mode enable
Reference IO setting table*/
#define offset_of_padtop_reg_eth_ext_en (108)
#define mask_of_padtop_reg_eth_ext_en   (0x3ff)
    unsigned int reg_eth_ext_en : 10;

    // h0036, bit: 14
    /* */
    unsigned int : 6;

    // h0036
    unsigned int /* padding 16 bit */ : 16;

    // h0037, bit: 14
    /* */
    unsigned int : 16;

    // h0037
    unsigned int /* padding 16 bit */ : 16;

// h0038, bit: 0
/* GPHY0 ref Mode
Reference IO setting table*/
#define offset_of_padtop_reg_gphy0_ref_mode (112)
#define mask_of_padtop_reg_gphy0_ref_mode   (0x1)
    unsigned int reg_gphy0_ref_mode : 1;

    // h0038, bit: 3
    /* */
    unsigned int : 3;

// h0038, bit: 5
/* GPHY0 ref Mode
Reference IO setting table*/
#define offset_of_padtop_reg_gphy1_ref_mode (112)
#define mask_of_padtop_reg_gphy1_ref_mode   (0x30)
    unsigned int reg_gphy1_ref_mode : 2;

    // h0038, bit: 14
    /* */
    unsigned int : 10;

    // h0038
    unsigned int /* padding 16 bit */ : 16;

// h0039, bit: 5
/* SPI external mode enable*/
#define offset_of_padtop_reg_spi_ext_en (114)
#define mask_of_padtop_reg_spi_ext_en   (0x3f)
    unsigned int reg_spi_ext_en : 6;

    // h0039, bit: 14
    /* */
    unsigned int : 10;

    // h0039
    unsigned int /* padding 16 bit */ : 16;

// h003a, bit: 5
/* emmc external mode enable*/
#define offset_of_padtop_reg_emmc_ext_en (116)
#define mask_of_padtop_reg_emmc_ext_en   (0x3f)
    unsigned int reg_emmc_ext_en : 6;

    // h003a, bit: 14
    /* */
    unsigned int : 10;

    // h003a
    unsigned int /* padding 16 bit */ : 16;

    // h003b, bit: 14
    /* */
    unsigned int : 16;

    // h003b
    unsigned int /* padding 16 bit */ : 16;

    // h003c, bit: 14
    /* */
    unsigned int : 16;

    // h003c
    unsigned int /* padding 16 bit */ : 16;

    // h003d, bit: 14
    /* */
    unsigned int : 16;

    // h003d
    unsigned int /* padding 16 bit */ : 16;

// h003e, bit: 2
/* I2C1 1 to 2 Mode
Reference IO setting table*/
#define offset_of_padtop_reg_i2c1_1to2_mode (124)
#define mask_of_padtop_reg_i2c1_1to2_mode   (0x7)
    unsigned int reg_i2c1_1to2_mode : 3;

    // h003e, bit: 14
    /* */
    unsigned int : 13;

    // h003e
    unsigned int /* padding 16 bit */ : 16;

// h003f, bit: 7
/* I2C1 1 to 8 Mode
Reference IO setting table*/
#define offset_of_padtop_reg_i2c1_1to8_disable (126)
#define mask_of_padtop_reg_i2c1_1to8_disable   (0xff)
    unsigned int reg_i2c1_1to8_disable : 8;

// h003f, bit: 11
/* I2C2 1 to 4 Mode
Reference IO setting table*/
#define offset_of_padtop_reg_i2c2_1to4_disable (126)
#define mask_of_padtop_reg_i2c2_1to4_disable   (0xf00)
    unsigned int reg_i2c2_1to4_disable : 4;

    // h003f, bit: 14
    /* */
    unsigned int : 4;

    // h003f
    unsigned int /* padding 16 bit */ : 16;

// h0040, bit: 3
/* gpio sl*/
#define offset_of_padtop_reg_gpio_sl (128)
#define mask_of_padtop_reg_gpio_sl   (0xf)
    unsigned int reg_gpio_sl : 4;

    // h0040, bit: 14
    /* */
    unsigned int : 12;

    // h0040
    unsigned int /* padding 16 bit */ : 16;

// h0041, bit: 0
/* RGMII0 Mode
Reference IO setting table*/
#define offset_of_padtop_reg_rgmii0_mode (130)
#define mask_of_padtop_reg_rgmii0_mode   (0x1)
    unsigned int reg_rgmii0_mode : 1;

// h0041, bit: 1
/* RGMII0 Mode
Reference IO setting table*/
#define offset_of_padtop_reg_rgmii1_mode (130)
#define mask_of_padtop_reg_rgmii1_mode   (0x2)
    unsigned int reg_rgmii1_mode : 1;

    // h0041, bit: 14
    /* */
    unsigned int : 14;

    // h0041
    unsigned int /* padding 16 bit */ : 16;

// h0042, bit: 14
/* RGMII0 external mode enable*/
#define offset_of_padtop_reg_rgmii0_ext_en (132)
#define mask_of_padtop_reg_rgmii0_ext_en   (0xffff)
    unsigned int reg_rgmii0_ext_en : 16;

    // h0042
    unsigned int /* padding 16 bit */ : 16;

// h0043, bit: 14
/* RGMII1 external mode enable*/
#define offset_of_padtop_reg_rgmii1_ext_en (134)
#define mask_of_padtop_reg_rgmii1_ext_en   (0xffff)
    unsigned int reg_rgmii1_ext_en : 16;

    // h0043
    unsigned int /* padding 16 bit */ : 16;

// h0044, bit: 2
/* I2C6 mode
Reference IO setting table*/
#define offset_of_padtop_reg_i2c6_mode (136)
#define mask_of_padtop_reg_i2c6_mode   (0x7)
    unsigned int reg_i2c6_mode : 3;

    // h0044, bit: 3
    /* */
    unsigned int : 1;

// h0044, bit: 6
/* I2C7 mode
Reference IO setting table*/
#define offset_of_padtop_reg_i2c7_mode (136)
#define mask_of_padtop_reg_i2c7_mode   (0x70)
    unsigned int reg_i2c7_mode : 3;

    // h0044, bit: 7
    /* */
    unsigned int : 1;

// h0044, bit: 10
/* I2C8 mode
Reference IO setting table*/
#define offset_of_padtop_reg_i2c8_mode (136)
#define mask_of_padtop_reg_i2c8_mode   (0x700)
    unsigned int reg_i2c8_mode : 3;

    // h0044, bit: 11
    /* */
    unsigned int : 1;

// h0044, bit: 14
/* I2C9 mode
Reference IO setting table*/
#define offset_of_padtop_reg_i2c9_mode (136)
#define mask_of_padtop_reg_i2c9_mode   (0x7000)
    unsigned int reg_i2c9_mode : 3;

    // h0044, bit: 15
    /* */
    unsigned int : 1;

    // h0044
    unsigned int /* padding 16 bit */ : 16;

// h0045, bit: 2
/* I2C10 mode
Reference IO setting table*/
#define offset_of_padtop_reg_i2c10_mode (138)
#define mask_of_padtop_reg_i2c10_mode   (0x7)
    unsigned int reg_i2c10_mode : 3;

    // h0045, bit: 3
    /* */
    unsigned int : 1;

// h0045, bit: 4
/* IRIN mode
Reference IO setting table*/
#define offset_of_padtop_reg_ir_in_mode (138)
#define mask_of_padtop_reg_ir_in_mode   (0x10)
    unsigned int reg_ir_in_mode : 1;

    // h0045, bit: 7
    /* */
    unsigned int : 3;

// h0045, bit: 9
/* PWM11 mode
Reference IO setting table*/
#define offset_of_padtop_reg_pwm11_mode (138)
#define mask_of_padtop_reg_pwm11_mode   (0x300)
    unsigned int reg_pwm11_mode : 2;

    // h0045, bit: 14
    /* */
    unsigned int : 6;

    // h0045
    unsigned int /* padding 16 bit */ : 16;

// h0046, bit: 1
/* I2S RX mode
Reference IO setting table*/
#define offset_of_padtop_reg_i2s_rx_mode (140)
#define mask_of_padtop_reg_i2s_rx_mode   (0x3)
    unsigned int reg_i2s_rx_mode : 2;

    // h0046, bit: 3
    /* */
    unsigned int : 2;

// h0046, bit: 5
/* I2S TX mode
Reference IO setting table*/
#define offset_of_padtop_reg_i2s_tx_mode (140)
#define mask_of_padtop_reg_i2s_tx_mode   (0x30)
    unsigned int reg_i2s_tx_mode : 2;

    // h0046, bit: 7
    /* */
    unsigned int : 2;

// h0046, bit: 9
/* I2S  RXTX mode
Reference IO setting table*/
#define offset_of_padtop_reg_i2s_rxtx_mode (140)
#define mask_of_padtop_reg_i2s_rxtx_mode   (0x300)
    unsigned int reg_i2s_rxtx_mode : 2;

    // h0046, bit: 14
    /* */
    unsigned int : 6;

    // h0046
    unsigned int /* padding 16 bit */ : 16;

// h0047, bit: 2
/* SPI3 mode
Reference IO setting table*/
#define offset_of_padtop_reg_spi3_mode (142)
#define mask_of_padtop_reg_spi3_mode   (0x7)
    unsigned int reg_spi3_mode : 3;

    // h0047, bit: 14
    /* */
    unsigned int : 13;

    // h0047
    unsigned int /* padding 16 bit */ : 16;

// h0048, bit: 1
/* SR0 BT1120 mode
Reference IO setting table*/
#define offset_of_padtop_reg_sr0_bt1120_mode (144)
#define mask_of_padtop_reg_sr0_bt1120_mode   (0x3)
    unsigned int reg_sr0_bt1120_mode : 2;

    // h0048, bit: 3
    /* */
    unsigned int : 2;

// h0048, bit: 5
/* SR1 BT1120 mode
Reference IO setting table*/
#define offset_of_padtop_reg_sr1_bt1120_mode (144)
#define mask_of_padtop_reg_sr1_bt1120_mode   (0x30)
    unsigned int reg_sr1_bt1120_mode : 2;

    // h0048, bit: 7
    /* */
    unsigned int : 2;

// h0048, bit: 10
/* SR2 BT1120 mode
Reference IO setting table*/
#define offset_of_padtop_reg_sr2_bt1120_mode (144)
#define mask_of_padtop_reg_sr2_bt1120_mode   (0x700)
    unsigned int reg_sr2_bt1120_mode : 3;

    // h0048, bit: 11
    /* */
    unsigned int : 1;

// h0048, bit: 13
/* SR3 BT1120 mode
Reference IO setting table*/
#define offset_of_padtop_reg_sr3_bt1120_mode (144)
#define mask_of_padtop_reg_sr3_bt1120_mode   (0x3000)
    unsigned int reg_sr3_bt1120_mode : 2;

    // h0048, bit: 14
    /* */
    unsigned int : 1;

// h0048, bit: 15
/* SR4 BT1120 mode
Reference IO setting table*/
#define offset_of_padtop_reg_sr4_bt1120_mode (144)
#define mask_of_padtop_reg_sr4_bt1120_mode   (0x8000)
    unsigned int reg_sr4_bt1120_mode : 1;

    // h0048
    unsigned int /* padding 16 bit */ : 16;

// h0049, bit: 2
/* UART4  mode
Reference IO setting table*/
#define offset_of_padtop_reg_uart4_mode (146)
#define mask_of_padtop_reg_uart4_mode   (0x7)
    unsigned int reg_uart4_mode : 3;

    // h0049, bit: 3
    /* */
    unsigned int : 1;

// h0049, bit: 6
/* UART5  mode
Reference IO setting table*/
#define offset_of_padtop_reg_uart5_mode (146)
#define mask_of_padtop_reg_uart5_mode   (0x70)
    unsigned int reg_uart5_mode : 3;

    // h0049, bit: 14
    /* */
    unsigned int : 9;

    // h0049
    unsigned int /* padding 16 bit */ : 16;

// h004a, bit: 1
/* SR0 BT656 mode
Reference IO setting table*/
#define offset_of_padtop_reg_sr0_bt656_mode (148)
#define mask_of_padtop_reg_sr0_bt656_mode   (0x3)
    unsigned int reg_sr0_bt656_mode : 2;

    // h004a, bit: 14
    /* */
    unsigned int : 14;

    // h004a
    unsigned int /* padding 16 bit */ : 16;

// h004b, bit: 0
/* UART GPIO mode
Reference IO setting table*/
#define offset_of_padtop_reg_uart_is_gpio (150)
#define mask_of_padtop_reg_uart_is_gpio   (0x1)
    unsigned int reg_uart_is_gpio : 1;

    // h004b, bit: 14
    /* */
    unsigned int : 15;

    // h004b
    unsigned int /* padding 16 bit */ : 16;

// h004c, bit: 2
/* CA7 EJTAG mode
Reference IO setting table*/
#define offset_of_padtop_reg_ca7_ej_mode (152)
#define mask_of_padtop_reg_ca7_ej_mode   (0x7)
    unsigned int reg_ca7_ej_mode : 3;

    // h004c, bit: 3
    /* */
    unsigned int : 1;

// h004c, bit: 6
/* DSP EJTAG mode
Reference IO setting table*/
#define offset_of_padtop_reg_dsp_ej_mode (152)
#define mask_of_padtop_reg_dsp_ej_mode   (0x70)
    unsigned int reg_dsp_ej_mode : 3;

    // h004c, bit: 7
    /* */
    unsigned int : 1;

// h004c, bit: 9
/* ISP IR OUT mode
Reference IO setting table*/
#define offset_of_padtop_reg_isp0_ir_out_mode (152)
#define mask_of_padtop_reg_isp0_ir_out_mode   (0x300)
    unsigned int reg_isp0_ir_out_mode : 2;

    // h004c, bit: 11
    /* */
    unsigned int : 2;

// h004c, bit: 13
/* ISP IR OUT mode
Reference IO setting table*/
#define offset_of_padtop_reg_isp1_ir_out_mode (152)
#define mask_of_padtop_reg_isp1_ir_out_mode   (0x3000)
    unsigned int reg_isp1_ir_out_mode : 2;

    // h004c, bit: 14
    /* */
    unsigned int : 2;

    // h004c
    unsigned int /* padding 16 bit */ : 16;

// h004d, bit: 1
/* SD2 mode
Reference IO setting table*/
#define offset_of_padtop_reg_sd2_mode (154)
#define mask_of_padtop_reg_sd2_mode   (0x3)
    unsigned int reg_sd2_mode : 2;

    // h004d, bit: 3
    /* */
    unsigned int : 2;

// h004d, bit: 5
/* SD2 CDZ mode
Reference IO setting table*/
#define offset_of_padtop_reg_sd2_cdz_mode (154)
#define mask_of_padtop_reg_sd2_cdz_mode   (0x30)
    unsigned int reg_sd2_cdz_mode : 2;

    // h004d, bit: 14
    /* */
    unsigned int : 10;

    // h004d
    unsigned int /* padding 16 bit */ : 16;

// h004e, bit: 3
/* SR0 Slave mode
Reference IO setting table*/
#define offset_of_padtop_reg_sr0_slave_mode (156)
#define mask_of_padtop_reg_sr0_slave_mode   (0xf)
    unsigned int reg_sr0_slave_mode : 4;

// h004e, bit: 7
/* SR1 Slave mode
Reference IO setting table*/
#define offset_of_padtop_reg_sr1_slave_mode (156)
#define mask_of_padtop_reg_sr1_slave_mode   (0xf0)
    unsigned int reg_sr1_slave_mode : 4;

    // h004e, bit: 14
    /* */
    unsigned int : 8;

    // h004e
    unsigned int /* padding 16 bit */ : 16;

// h004f, bit: 1
/* I2S0 RX_TX mode
Reference IO setting table*/
#define offset_of_padtop_reg_i2s0_rxtx_mode (158)
#define mask_of_padtop_reg_i2s0_rxtx_mode   (0x3)
    unsigned int reg_i2s0_rxtx_mode : 2;

    // h004f, bit: 3
    /* */
    unsigned int : 2;

// h004f, bit: 6
/* I2S0 RX_TX mode
Reference IO setting table*/
#define offset_of_padtop_reg_i2s1_rxtx_mode (158)
#define mask_of_padtop_reg_i2s1_rxtx_mode   (0x70)
    unsigned int reg_i2s1_rxtx_mode : 3;

    // h004f, bit: 14
    /* */
    unsigned int : 9;

    // h004f
    unsigned int /* padding 16 bit */ : 16;

// h0050, bit: 0
/* MII0 Mode
Reference IO setting table*/
#define offset_of_padtop_reg_mii0_mode (160)
#define mask_of_padtop_reg_mii0_mode   (0x1)
    unsigned int reg_mii0_mode : 1;

// h0050, bit: 1
/* MII1 Mode
Reference IO setting table*/
#define offset_of_padtop_reg_mii1_mode (160)
#define mask_of_padtop_reg_mii1_mode   (0x2)
    unsigned int reg_mii1_mode : 1;

    // h0050, bit: 3
    /* */
    unsigned int : 2;

// h0050, bit: 4
/* GMAC0 REF CLK Mode
Reference IO setting table*/
#define offset_of_padtop_reg_gmac0_ref_mode (160)
#define mask_of_padtop_reg_gmac0_ref_mode   (0x10)
    unsigned int reg_gmac0_ref_mode : 1;

// h0050, bit: 5
/* GMAC1 REF CLK Mode
Reference IO setting table*/
#define offset_of_padtop_reg_gmac1_ref_mode (160)
#define mask_of_padtop_reg_gmac1_ref_mode   (0x20)
    unsigned int reg_gmac1_ref_mode : 1;

    // h0050, bit: 14
    /* */
    unsigned int : 10;

    // h0050
    unsigned int /* padding 16 bit */ : 16;

// h0051, bit: 1
/* SR PDN Mode
Reference IO setting table*/
#define offset_of_padtop_reg_sr00_pdn_mode (162)
#define mask_of_padtop_reg_sr00_pdn_mode   (0x3)
    unsigned int reg_sr00_pdn_mode : 2;

// h0051, bit: 3
/* SR PDN Mode
Reference IO setting table*/
#define offset_of_padtop_reg_sr01_pdn_mode (162)
#define mask_of_padtop_reg_sr01_pdn_mode   (0xc)
    unsigned int reg_sr01_pdn_mode : 2;

// h0051, bit: 5
/* SR PDN Mode
Reference IO setting table*/
#define offset_of_padtop_reg_sr02_pdn_mode (162)
#define mask_of_padtop_reg_sr02_pdn_mode   (0x30)
    unsigned int reg_sr02_pdn_mode : 2;

// h0051, bit: 7
/* SR PDN Mode
Reference IO setting table*/
#define offset_of_padtop_reg_sr03_pdn_mode (162)
#define mask_of_padtop_reg_sr03_pdn_mode   (0xc0)
    unsigned int reg_sr03_pdn_mode : 2;

// h0051, bit: 8
/* SR PDN Mode
Reference IO setting table*/
#define offset_of_padtop_reg_sr10_pdn_mode (162)
#define mask_of_padtop_reg_sr10_pdn_mode   (0x100)
    unsigned int reg_sr10_pdn_mode : 1;

    // h0051, bit: 9
    /* */
    unsigned int : 1;

// h0051, bit: 10
/* SR PDN Mode
Reference IO setting table*/
#define offset_of_padtop_reg_sr11_pdn_mode (162)
#define mask_of_padtop_reg_sr11_pdn_mode   (0x400)
    unsigned int reg_sr11_pdn_mode : 1;

    // h0051, bit: 11
    /* */
    unsigned int : 1;

// h0051, bit: 12
/* SR PDN Mode
Reference IO setting table*/
#define offset_of_padtop_reg_sr12_pdn_mode (162)
#define mask_of_padtop_reg_sr12_pdn_mode   (0x1000)
    unsigned int reg_sr12_pdn_mode : 1;

    // h0051, bit: 13
    /* */
    unsigned int : 1;

// h0051, bit: 14
/* SR PDN Mode
Reference IO setting table*/
#define offset_of_padtop_reg_sr13_pdn_mode (162)
#define mask_of_padtop_reg_sr13_pdn_mode   (0x4000)
    unsigned int reg_sr13_pdn_mode : 1;

    // h0051, bit: 15
    /* */
    unsigned int : 1;

    // h0051
    unsigned int /* padding 16 bit */ : 16;

// h0052, bit: 0
/* SR PCLK Mode
Reference IO setting table*/
#define offset_of_padtop_reg_sr0_pclk_mode (164)
#define mask_of_padtop_reg_sr0_pclk_mode   (0x1)
    unsigned int reg_sr0_pclk_mode : 1;

    // h0052, bit: 3
    /* */
    unsigned int : 3;

// h0052, bit: 5
/* SR RST Mode
Reference IO setting table*/
#define offset_of_padtop_reg_sr0_rst_mode (164)
#define mask_of_padtop_reg_sr0_rst_mode   (0x30)
    unsigned int reg_sr0_rst_mode : 2;

    // h0052, bit: 7
    /* */
    unsigned int : 2;

// h0052, bit: 9
/* SR PDN Mode
Reference IO setting table*/
#define offset_of_padtop_reg_sr0_pdn_mode (164)
#define mask_of_padtop_reg_sr0_pdn_mode   (0x300)
    unsigned int reg_sr0_pdn_mode : 2;

// h0052, bit: 10
/* SR Hsync Mode
Reference IO setting table*/
#define offset_of_padtop_reg_sr_hsync_mode (164)
#define mask_of_padtop_reg_sr_hsync_mode   (0x400)
    unsigned int reg_sr_hsync_mode : 1;

    // h0052, bit: 11
    /* */
    unsigned int : 1;

// h0052, bit: 12
/* SR Vsync Mode
Reference IO setting table*/
#define offset_of_padtop_reg_sr_vsync_mode (164)
#define mask_of_padtop_reg_sr_vsync_mode   (0x1000)
    unsigned int reg_sr_vsync_mode : 1;

    // h0052, bit: 14
    /* */
    unsigned int : 3;

    // h0052
    unsigned int /* padding 16 bit */ : 16;

// h0053, bit: 2
/* I2C1 mode
Reference IO setting table*/
#define offset_of_padtop_reg_i2c1_mode (166)
#define mask_of_padtop_reg_i2c1_mode   (0x7)
    unsigned int reg_i2c1_mode : 3;

    // h0053, bit: 14
    /* */
    unsigned int : 13;

    // h0053
    unsigned int /* padding 16 bit */ : 16;

// h0054, bit: 1
/* SR00 RST mode
Reference IO setting table*/
#define offset_of_padtop_reg_sr00_rst_mode (168)
#define mask_of_padtop_reg_sr00_rst_mode   (0x3)
    unsigned int reg_sr00_rst_mode : 2;

// h0054, bit: 3
/* SR01 RST mode
Reference IO setting table*/
#define offset_of_padtop_reg_sr01_rst_mode (168)
#define mask_of_padtop_reg_sr01_rst_mode   (0xc)
    unsigned int reg_sr01_rst_mode : 2;

// h0054, bit: 5
/* SR02 RST mode
Reference IO setting table*/
#define offset_of_padtop_reg_sr02_rst_mode (168)
#define mask_of_padtop_reg_sr02_rst_mode   (0x30)
    unsigned int reg_sr02_rst_mode : 2;

// h0054, bit: 7
/* SR03 RST mode
Reference IO setting table*/
#define offset_of_padtop_reg_sr03_rst_mode (168)
#define mask_of_padtop_reg_sr03_rst_mode   (0xc0)
    unsigned int reg_sr03_rst_mode : 2;

// h0054, bit: 9
/* SR10 RST mode
Reference IO setting table*/
#define offset_of_padtop_reg_sr10_rst_mode (168)
#define mask_of_padtop_reg_sr10_rst_mode   (0x300)
    unsigned int reg_sr10_rst_mode : 2;

// h0054, bit: 11
/* SR11 RST mode
Reference IO setting table*/
#define offset_of_padtop_reg_sr11_rst_mode (168)
#define mask_of_padtop_reg_sr11_rst_mode   (0xc00)
    unsigned int reg_sr11_rst_mode : 2;

// h0054, bit: 13
/* SR12 RST mode
Reference IO setting table*/
#define offset_of_padtop_reg_sr12_rst_mode (168)
#define mask_of_padtop_reg_sr12_rst_mode   (0x3000)
    unsigned int reg_sr12_rst_mode : 2;

// h0054, bit: 14
/* SR13 RST mode
Reference IO setting table*/
#define offset_of_padtop_reg_sr13_rst_mode (168)
#define mask_of_padtop_reg_sr13_rst_mode   (0xc000)
    unsigned int reg_sr13_rst_mode : 2;

    // h0054
    unsigned int /* padding 16 bit */ : 16;

// h0055, bit: 1
/* SR00 MCLK SEL
00: mclk form clkgen
01: reserved
10: isp0_mclk
11: isp1_mclk*/
#define offset_of_padtop_reg_sr00_mclk_sel (170)
#define mask_of_padtop_reg_sr00_mclk_sel   (0x3)
    unsigned int reg_sr00_mclk_sel : 2;

// h0055, bit: 3
/* SR01 MCLK SEL
00: mclk form clkgen
01: reserved
10: isp0_mclk
11: isp1_mclk*/
#define offset_of_padtop_reg_sr01_mclk_sel (170)
#define mask_of_padtop_reg_sr01_mclk_sel   (0xc)
    unsigned int reg_sr01_mclk_sel : 2;

// h0055, bit: 5
/* SR02 MCLK SEL
00: mclk form clkgen
01: reserved
10: isp0_mclk
11: isp1_mclk*/
#define offset_of_padtop_reg_sr02_mclk_sel (170)
#define mask_of_padtop_reg_sr02_mclk_sel   (0x30)
    unsigned int reg_sr02_mclk_sel : 2;

// h0055, bit: 7
/* SR03 MCLK SEL
00: mclk form clkgen
01: reserved
10: isp0_mclk
11: isp1_mclk*/
#define offset_of_padtop_reg_sr03_mclk_sel (170)
#define mask_of_padtop_reg_sr03_mclk_sel   (0xc0)
    unsigned int reg_sr03_mclk_sel : 2;

// h0055, bit: 9
/* SR10 MCLK SEL00: mclk form clkgen
01: reserved
10: isp0_mclk
11: isp1_mclk*/
#define offset_of_padtop_reg_sr10_mclk_sel (170)
#define mask_of_padtop_reg_sr10_mclk_sel   (0x300)
    unsigned int reg_sr10_mclk_sel : 2;

// h0055, bit: 11
/* SR11MCLK SEL
00: mclk form clkgen
01: reserved
10: isp0_mclk
11: isp1_mclk*/
#define offset_of_padtop_reg_sr11_mclk_sel (170)
#define mask_of_padtop_reg_sr11_mclk_sel   (0xc00)
    unsigned int reg_sr11_mclk_sel : 2;

// h0055, bit: 13
/* SR12 MCLK SEL
00: mclk form clkgen
01: reserved
10: isp0_mclk
11: isp1_mclk*/
#define offset_of_padtop_reg_sr12_mclk_sel (170)
#define mask_of_padtop_reg_sr12_mclk_sel   (0x3000)
    unsigned int reg_sr12_mclk_sel : 2;

// h0055, bit: 14
/* SR13 MCLK SEL
00: mclk form clkgen
01: reserved
10: isp0_mclk
11: isp1_mclk*/
#define offset_of_padtop_reg_sr13_mclk_sel (170)
#define mask_of_padtop_reg_sr13_mclk_sel   (0xc000)
    unsigned int reg_sr13_mclk_sel : 2;

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

// h0058, bit: 14
/* TEST Mode*/
#define offset_of_padtop_reg_pad_test0_mode (176)
#define mask_of_padtop_reg_pad_test0_mode   (0xffff)
    unsigned int reg_pad_test0_mode : 16;

    // h0058
    unsigned int /* padding 16 bit */ : 16;

// h0059, bit: 14
/* TEST Mode*/
#define offset_of_padtop_reg_pad_test1_mode (178)
#define mask_of_padtop_reg_pad_test1_mode   (0xffff)
    unsigned int reg_pad_test1_mode : 16;

    // h0059
    unsigned int /* padding 16 bit */ : 16;

// h005a, bit: 14
/* TEST Mode*/
#define offset_of_padtop_reg_pad_test2_mode (180)
#define mask_of_padtop_reg_pad_test2_mode   (0xffff)
    unsigned int reg_pad_test2_mode : 16;

    // h005a
    unsigned int /* padding 16 bit */ : 16;

// h005b, bit: 14
/* TEST Mode*/
#define offset_of_padtop_reg_pad_test3_mode (182)
#define mask_of_padtop_reg_pad_test3_mode   (0xffff)
    unsigned int reg_pad_test3_mode : 16;

    // h005b
    unsigned int /* padding 16 bit */ : 16;

// h005c, bit: 14
/* TEST Mode*/
#define offset_of_padtop_reg_pad_test4_mode (184)
#define mask_of_padtop_reg_pad_test4_mode   (0xffff)
    unsigned int reg_pad_test4_mode : 16;

    // h005c
    unsigned int /* padding 16 bit */ : 16;

// h005d, bit: 14
/* TEST Mode*/
#define offset_of_padtop_reg_pad_test5_mode (186)
#define mask_of_padtop_reg_pad_test5_mode   (0xffff)
    unsigned int reg_pad_test5_mode : 16;

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

// h0060, bit: 3
/* DMIC Mode
Reference IO setting table*/
#define offset_of_padtop_reg_dmic_mode (192)
#define mask_of_padtop_reg_dmic_mode   (0xf)
    unsigned int reg_dmic_mode : 4;

// h0060, bit: 4
/* BT656 output Mode
Reference IO setting table*/
#define offset_of_padtop_reg_bt656_out_mode (192)
#define mask_of_padtop_reg_bt656_out_mode   (0x10)
    unsigned int reg_bt656_out_mode : 1;

    // h0060, bit: 6
    /* */
    unsigned int : 2;

// h0060, bit: 9
/* EJ Mode
Reference IO setting table*/
#define offset_of_padtop_reg_ej_mode (192)
#define mask_of_padtop_reg_ej_mode   (0x380)
    unsigned int reg_ej_mode : 3;

    // h0060, bit: 14
    /* */
    unsigned int : 6;

    // h0060
    unsigned int /* padding 16 bit */ : 16;

// h0061, bit: 1
/* EMMC0 4 bit mode
Reference IO setting table*/
#define offset_of_padtop_reg_sd0_mode (194)
#define mask_of_padtop_reg_sd0_mode   (0x3)
    unsigned int reg_sd0_mode : 2;

    // h0061, bit: 3
    /* */
    unsigned int : 2;

// h0061, bit: 5
/* EMMC0 Reset mode
Reference IO setting table*/
#define offset_of_padtop_reg_sd0_cdz_mode (194)
#define mask_of_padtop_reg_sd0_cdz_mode   (0x30)
    unsigned int reg_sd0_cdz_mode : 2;

    // h0061, bit: 7
    /* */
    unsigned int : 2;

// h0061, bit: 9
/* EMMC0 set as RST
Reference IO setting table*/
#define offset_of_padtop_reg_emmc_rst_mode (194)
#define mask_of_padtop_reg_emmc_rst_mode   (0x300)
    unsigned int reg_emmc_rst_mode : 2;

// h0061, bit: 11
/* EMMC0 4bit mode
Reference IO setting table*/
#define offset_of_padtop_reg_emmc4b_mode (194)
#define mask_of_padtop_reg_emmc4b_mode   (0xc00)
    unsigned int reg_emmc4b_mode : 2;

// h0061, bit: 12
/* EMMC0  4bit boot mode
Reference IO setting table*/
#define offset_of_padtop_reg_emmc4b_boot_mode (194)
#define mask_of_padtop_reg_emmc4b_boot_mode   (0x1000)
    unsigned int reg_emmc4b_boot_mode : 1;

    // h0061, bit: 14
    /* */
    unsigned int : 3;

    // h0061
    unsigned int /* padding 16 bit */ : 16;

// h0062, bit: 1
/* I2S0 MCK mode
Reference IO setting table*/
#define offset_of_padtop_reg_i2s_mck_mode (196)
#define mask_of_padtop_reg_i2s_mck_mode   (0x3)
    unsigned int reg_i2s_mck_mode : 2;

    // h0062, bit: 3
    /* */
    unsigned int : 2;

// h0062, bit: 5
/* I2S VAD mode
Reference IO setting table*/
#define offset_of_padtop_reg_vad_mode (196)
#define mask_of_padtop_reg_vad_mode   (0x30)
    unsigned int reg_vad_mode : 2;

    // h0062, bit: 7
    /* */
    unsigned int : 2;

// h0062, bit: 9
/* I2S RX mode
Reference IO setting table*/
#define offset_of_padtop_reg_i2s0_rx_mode (196)
#define mask_of_padtop_reg_i2s0_rx_mode   (0x300)
    unsigned int reg_i2s0_rx_mode : 2;

    // h0062, bit: 11
    /* */
    unsigned int : 2;

// h0062, bit: 13
/* I2S TX mode
Reference IO setting table*/
#define offset_of_padtop_reg_i2s0_tx_mode (196)
#define mask_of_padtop_reg_i2s0_tx_mode   (0x3000)
    unsigned int reg_i2s0_tx_mode : 2;

    // h0062, bit: 14
    /* */
    unsigned int : 2;

    // h0062
    unsigned int /* padding 16 bit */ : 16;

// h0063, bit: 1
/* LED0 mode
Reference IO setting table*/
#define offset_of_padtop_reg_led0_mode (198)
#define mask_of_padtop_reg_led0_mode   (0x3)
    unsigned int reg_led0_mode : 2;

    // h0063, bit: 3
    /* */
    unsigned int : 2;

// h0063, bit: 5
/* LED1 mode
Reference IO setting table*/
#define offset_of_padtop_reg_led1_mode (198)
#define mask_of_padtop_reg_led1_mode   (0x30)
    unsigned int reg_led1_mode : 2;

    // h0063, bit: 14
    /* */
    unsigned int : 10;

    // h0063
    unsigned int /* padding 16 bit */ : 16;

// h0064, bit: 1
/* MIPI TX mode
Reference IO setting table*/
#define offset_of_padtop_reg_mipi_tx_mode (200)
#define mask_of_padtop_reg_mipi_tx_mode   (0x3)
    unsigned int reg_mipi_tx_mode : 2;

    // h0064, bit: 7
    /* */
    unsigned int : 6;

// h0064, bit: 8
/* OTP test
Reference IO setting table*/
#define offset_of_padtop_reg_otp_test (200)
#define mask_of_padtop_reg_otp_test   (0x100)
    unsigned int reg_otp_test : 1;

    // h0064, bit: 14
    /* */
    unsigned int : 7;

    // h0064
    unsigned int /* padding 16 bit */ : 16;

// h0065, bit: 2
/* PWM0 mode
Reference IO setting table*/
#define offset_of_padtop_reg_pwm0_mode (202)
#define mask_of_padtop_reg_pwm0_mode   (0x7)
    unsigned int reg_pwm0_mode : 3;

    // h0065, bit: 3
    /* */
    unsigned int : 1;

// h0065, bit: 6
/* PWM1 mode
Reference IO setting table*/
#define offset_of_padtop_reg_pwm1_mode (202)
#define mask_of_padtop_reg_pwm1_mode   (0x70)
    unsigned int reg_pwm1_mode : 3;

    // h0065, bit: 7
    /* */
    unsigned int : 1;

// h0065, bit: 10
/* PWM2 mode
Reference IO setting table*/
#define offset_of_padtop_reg_pwm2_mode (202)
#define mask_of_padtop_reg_pwm2_mode   (0x700)
    unsigned int reg_pwm2_mode : 3;

    // h0065, bit: 11
    /* */
    unsigned int : 1;

// h0065, bit: 14
/* PWM3 mode
Reference IO setting table*/
#define offset_of_padtop_reg_pwm3_mode (202)
#define mask_of_padtop_reg_pwm3_mode   (0x7000)
    unsigned int reg_pwm3_mode : 3;

    // h0065, bit: 15
    /* */
    unsigned int : 1;

    // h0065
    unsigned int /* padding 16 bit */ : 16;

// h0066, bit: 2
/* PWM4 mode
Reference IO setting table*/
#define offset_of_padtop_reg_pwm4_mode (204)
#define mask_of_padtop_reg_pwm4_mode   (0x7)
    unsigned int reg_pwm4_mode : 3;

    // h0066, bit: 3
    /* */
    unsigned int : 1;

// h0066, bit: 6
/* PWM5 mode
Reference IO setting table*/
#define offset_of_padtop_reg_pwm5_mode (204)
#define mask_of_padtop_reg_pwm5_mode   (0x70)
    unsigned int reg_pwm5_mode : 3;

    // h0066, bit: 7
    /* */
    unsigned int : 1;

// h0066, bit: 10
/* PWM6 mode
Reference IO setting table*/
#define offset_of_padtop_reg_pwm6_mode (204)
#define mask_of_padtop_reg_pwm6_mode   (0x700)
    unsigned int reg_pwm6_mode : 3;

    // h0066, bit: 11
    /* */
    unsigned int : 1;

// h0066, bit: 14
/* PWM7 mode
Reference IO setting table*/
#define offset_of_padtop_reg_pwm7_mode (204)
#define mask_of_padtop_reg_pwm7_mode   (0x7000)
    unsigned int reg_pwm7_mode : 3;

    // h0066, bit: 15
    /* */
    unsigned int : 1;

    // h0066
    unsigned int /* padding 16 bit */ : 16;

// h0067, bit: 1
/* PWM8 mode
Reference IO setting table*/
#define offset_of_padtop_reg_pwm8_mode (206)
#define mask_of_padtop_reg_pwm8_mode   (0x3)
    unsigned int reg_pwm8_mode : 2;

    // h0067, bit: 3
    /* */
    unsigned int : 2;

// h0067, bit: 6
/* PWM9 mode
Reference IO setting table*/
#define offset_of_padtop_reg_pwm9_mode (206)
#define mask_of_padtop_reg_pwm9_mode   (0x70)
    unsigned int reg_pwm9_mode : 3;

    // h0067, bit: 10
    /* */
    unsigned int : 4;

// h0067, bit: 12
/* SDIO CDZ mode
Reference IO setting table*/
#define offset_of_padtop_reg_sd0_rstn_mode (206)
#define mask_of_padtop_reg_sd0_rstn_mode   (0x1800)
    unsigned int reg_sd0_rstn_mode : 2;

    // h0067, bit: 14
    /* */
    unsigned int : 3;

    // h0067
    unsigned int /* padding 16 bit */ : 16;

// h0068, bit: 2
/* SPI0 mode
Reference IO setting table*/
#define offset_of_padtop_reg_spi0_mode (208)
#define mask_of_padtop_reg_spi0_mode   (0x7)
    unsigned int reg_spi0_mode : 3;

    // h0068, bit: 3
    /* */
    unsigned int : 1;

// h0068, bit: 6
/* SPI1 mode
Reference IO setting table*/
#define offset_of_padtop_reg_spi1_mode (208)
#define mask_of_padtop_reg_spi1_mode   (0x70)
    unsigned int reg_spi1_mode : 3;

    // h0068, bit: 11
    /* */
    unsigned int : 5;

// h0068, bit: 13
/* SDIO RST mode
Reference IO setting table*/
#define offset_of_padtop_reg_sdio_rst_mode (208)
#define mask_of_padtop_reg_sdio_rst_mode   (0x3000)
    unsigned int reg_sdio_rst_mode : 2;

    // h0068, bit: 14
    /* */
    unsigned int : 2;

    // h0068
    unsigned int /* padding 16 bit */ : 16;

// h0069, bit: 2
/* SR0 MIPI mode
Reference IO setting table*/
#define offset_of_padtop_reg_sr0_mipi_mode (210)
#define mask_of_padtop_reg_sr0_mipi_mode   (0x7)
    unsigned int reg_sr0_mipi_mode : 3;

    // h0069, bit: 3
    /* */
    unsigned int : 1;

// h0069, bit: 5
/* SR1 MIPI mode
Reference IO setting table*/
#define offset_of_padtop_reg_sr1_mipi_mode (210)
#define mask_of_padtop_reg_sr1_mipi_mode   (0x30)
    unsigned int reg_sr1_mipi_mode : 2;

    // h0069, bit: 14
    /* */
    unsigned int : 10;

    // h0069
    unsigned int /* padding 16 bit */ : 16;

// h006a, bit: 1
/* SR00 MCLK mode
Reference IO setting table*/
#define offset_of_padtop_reg_sr00_mclk_mode (212)
#define mask_of_padtop_reg_sr00_mclk_mode   (0x3)
    unsigned int reg_sr00_mclk_mode : 2;

// h006a, bit: 3
/* SR01 MCLK mode
Reference IO setting table*/
#define offset_of_padtop_reg_sr01_mclk_mode (212)
#define mask_of_padtop_reg_sr01_mclk_mode   (0xc)
    unsigned int reg_sr01_mclk_mode : 2;

// h006a, bit: 5
/* SR02 MCLK mode
Reference IO setting table*/
#define offset_of_padtop_reg_sr02_mclk_mode (212)
#define mask_of_padtop_reg_sr02_mclk_mode   (0x30)
    unsigned int reg_sr02_mclk_mode : 2;

// h006a, bit: 7
/* SR03 MCLK mode
Reference IO setting table*/
#define offset_of_padtop_reg_sr03_mclk_mode (212)
#define mask_of_padtop_reg_sr03_mclk_mode   (0xc0)
    unsigned int reg_sr03_mclk_mode : 2;

// h006a, bit: 9
/* SR10 MCLK mode
Reference IO setting table*/
#define offset_of_padtop_reg_sr10_mclk_mode (212)
#define mask_of_padtop_reg_sr10_mclk_mode   (0x300)
    unsigned int reg_sr10_mclk_mode : 2;

// h006a, bit: 10
/* SR11 MCLK mode
Reference IO setting table*/
#define offset_of_padtop_reg_sr11_mclk_mode (212)
#define mask_of_padtop_reg_sr11_mclk_mode   (0x400)
    unsigned int reg_sr11_mclk_mode : 1;

    // h006a, bit: 11
    /* */
    unsigned int : 1;

// h006a, bit: 13
/* SR12 MCLK mode
Reference IO setting table*/
#define offset_of_padtop_reg_sr12_mclk_mode (212)
#define mask_of_padtop_reg_sr12_mclk_mode   (0x3000)
    unsigned int reg_sr12_mclk_mode : 2;

// h006a, bit: 14
/* SR13 MCLK mode
Reference IO setting table*/
#define offset_of_padtop_reg_sr13_mclk_mode (212)
#define mask_of_padtop_reg_sr13_mclk_mode   (0xc000)
    unsigned int reg_sr13_mclk_mode : 2;

    // h006a
    unsigned int /* padding 16 bit */ : 16;

// h006b, bit: 1
/* SR0 mode Select
Reference IO setting table*/
#define offset_of_padtop_reg_sr0_mode (214)
#define mask_of_padtop_reg_sr0_mode   (0x3)
    unsigned int reg_sr0_mode : 2;

    // h006b, bit: 14
    /* */
    unsigned int : 14;

    // h006b
    unsigned int /* padding 16 bit */ : 16;

// h006c, bit: 1
/* MSPI0_CZ1 mode
Reference IO setting table*/
#define offset_of_padtop_reg_mspi0_cz1_mode (216)
#define mask_of_padtop_reg_mspi0_cz1_mode   (0x3)
    unsigned int reg_mspi0_cz1_mode : 2;

    // h006c, bit: 3
    /* */
    unsigned int : 2;

// h006c, bit: 6
/* MSPI0  mode
Reference IO setting table*/
#define offset_of_padtop_reg_mspi0_mode (216)
#define mask_of_padtop_reg_mspi0_mode   (0x70)
    unsigned int reg_mspi0_mode : 3;

    // h006c, bit: 7
    /* */
    unsigned int : 1;

// h006c, bit: 8
/* RMII  mode
Reference IO setting table*/
#define offset_of_padtop_reg_rmii_mode (216)
#define mask_of_padtop_reg_rmii_mode   (0x100)
    unsigned int reg_rmii_mode : 1;

    // h006c, bit: 14
    /* */
    unsigned int : 7;

    // h006c
    unsigned int /* padding 16 bit */ : 16;

// h006d, bit: 1
/* UART0  mode
Reference IO setting table*/
#define offset_of_padtop_reg_uart0_mode (218)
#define mask_of_padtop_reg_uart0_mode   (0x3)
    unsigned int reg_uart0_mode : 2;

    // h006d, bit: 3
    /* */
    unsigned int : 2;

// h006d, bit: 5
/* UART1  mode
Reference IO setting table*/
#define offset_of_padtop_reg_uart1_mode (218)
#define mask_of_padtop_reg_uart1_mode   (0x30)
    unsigned int reg_uart1_mode : 2;

    // h006d, bit: 7
    /* */
    unsigned int : 2;

// h006d, bit: 9
/* UART2  mode
Reference IO setting table*/
#define offset_of_padtop_reg_uart2_mode (218)
#define mask_of_padtop_reg_uart2_mode   (0x300)
    unsigned int reg_uart2_mode : 2;

    // h006d, bit: 11
    /* */
    unsigned int : 2;

// h006d, bit: 14
/* UART3  mode
Reference IO setting table*/
#define offset_of_padtop_reg_uart3_mode (218)
#define mask_of_padtop_reg_uart3_mode   (0x7000)
    unsigned int reg_uart3_mode : 3;

    // h006d, bit: 15
    /* */
    unsigned int : 1;

    // h006d
    unsigned int /* padding 16 bit */ : 16;

// h006e, bit: 1
/* ETH mode
Reference IO setting table*/
#define offset_of_padtop_reg_eth_mode (220)
#define mask_of_padtop_reg_eth_mode   (0x3)
    unsigned int reg_eth_mode : 2;

    // h006e, bit: 3
    /* */
    unsigned int : 2;

// h006e, bit: 7
/* ETH1 mode
Reference IO setting table*/
#define offset_of_padtop_reg_eth1_mode (220)
#define mask_of_padtop_reg_eth1_mode   (0xf0)
    unsigned int reg_eth1_mode : 4;

// h006e, bit: 10
/* Fast UART mode
Reference IO setting table*/
#define offset_of_padtop_reg_fuart_mode (220)
#define mask_of_padtop_reg_fuart_mode   (0x700)
    unsigned int reg_fuart_mode : 3;

    // h006e, bit: 11
    /* */
    unsigned int : 1;

// h006e, bit: 14
/* Fast UART 2 wire mode
Reference IO setting table*/
#define offset_of_padtop_reg_fuart_2w_mode (220)
#define mask_of_padtop_reg_fuart_2w_mode   (0x7000)
    unsigned int reg_fuart_2w_mode : 3;

    // h006e, bit: 15
    /* */
    unsigned int : 1;

    // h006e
    unsigned int /* padding 16 bit */ : 16;

// h006f, bit: 2
/* I2C0 mode
Reference IO setting table*/
#define offset_of_padtop_reg_i2c0_mode (222)
#define mask_of_padtop_reg_i2c0_mode   (0x7)
    unsigned int reg_i2c0_mode : 3;

    // h006f, bit: 14
    /* */
    unsigned int : 13;

    // h006f
    unsigned int /* padding 16 bit */ : 16;

// h0070, bit: 1
/* SPI1 CZ1 mode
Reference IO setting table*/
#define offset_of_padtop_reg_spi1_cz1_mode (224)
#define mask_of_padtop_reg_spi1_cz1_mode   (0x3)
    unsigned int reg_spi1_cz1_mode : 2;

    // h0070, bit: 3
    /* */
    unsigned int : 2;

// h0070, bit: 5
/* SPI0 CZ1 mode
Reference IO setting table*/
#define offset_of_padtop_reg_spi0_cz1_mode (224)
#define mask_of_padtop_reg_spi0_cz1_mode   (0x30)
    unsigned int reg_spi0_cz1_mode : 2;

    // h0070, bit: 14
    /* */
    unsigned int : 10;

    // h0070
    unsigned int /* padding 16 bit */ : 16;

// h0071, bit: 1
/* DLA EJTAG mode
Reference IO setting table*/
#define offset_of_padtop_reg_dla_ej_mode (226)
#define mask_of_padtop_reg_dla_ej_mode   (0x3)
    unsigned int reg_dla_ej_mode : 2;

    // h0071, bit: 14
    /* */
    unsigned int : 14;

    // h0071
    unsigned int /* padding 16 bit */ : 16;

// h0072, bit: 1
/* BT1120 output Mode
Reference IO setting table*/
#define offset_of_padtop_reg_bt1120_out_mode (228)
#define mask_of_padtop_reg_bt1120_out_mode   (0x3)
    unsigned int reg_bt1120_out_mode : 2;

    // h0072, bit: 3
    /* */
    unsigned int : 2;

// h0072, bit: 5
/* BT601 output Mode
Reference IO setting table*/
#define offset_of_padtop_reg_bt601_out_mode (228)
#define mask_of_padtop_reg_bt601_out_mode   (0x30)
    unsigned int reg_bt601_out_mode : 2;

    // h0072, bit: 14
    /* */
    unsigned int : 10;

    // h0072
    unsigned int /* padding 16 bit */ : 16;

// h0073, bit: 2
/* I2C3 mode
Reference IO setting table*/
#define offset_of_padtop_reg_i2c3_mode (230)
#define mask_of_padtop_reg_i2c3_mode   (0x7)
    unsigned int reg_i2c3_mode : 3;

    // h0073, bit: 7
    /* */
    unsigned int : 5;

// h0073, bit: 10
/* I2C5 mode
Reference IO setting table*/
#define offset_of_padtop_reg_i2c5_mode (230)
#define mask_of_padtop_reg_i2c5_mode   (0x700)
    unsigned int reg_i2c5_mode : 3;

    // h0073, bit: 14
    /* */
    unsigned int : 5;

    // h0073
    unsigned int /* padding 16 bit */ : 16;

// h0074, bit: 2
/* PWM10 mode
Reference IO setting table*/
#define offset_of_padtop_reg_pwm10_mode (232)
#define mask_of_padtop_reg_pwm10_mode   (0x7)
    unsigned int reg_pwm10_mode : 3;

    // h0074, bit: 7
    /* */
    unsigned int : 5;

// h0074, bit: 9
/* RGB 8 bit mode mode
Reference IO setting table*/
#define offset_of_padtop_reg_rgb8_mode (232)
#define mask_of_padtop_reg_rgb8_mode   (0x300)
    unsigned int reg_rgb8_mode : 2;

    // h0074, bit: 11
    /* */
    unsigned int : 2;

// h0074, bit: 12
/* RGB 16 bit mode mode
Reference IO setting table*/
#define offset_of_padtop_reg_rgb16_mode (232)
#define mask_of_padtop_reg_rgb16_mode   (0x1000)
    unsigned int reg_rgb16_mode : 1;

    // h0074, bit: 14
    /* */
    unsigned int : 3;

    // h0074
    unsigned int /* padding 16 bit */ : 16;

    // h0075, bit: 14
    /* */
    unsigned int : 16;

    // h0075
    unsigned int /* padding 16 bit */ : 16;

    // h0076, bit: 14
    /* */
    unsigned int : 16;

    // h0076
    unsigned int /* padding 16 bit */ : 16;

    // h0077, bit: 7
    /* */
    unsigned int : 8;

// h0077, bit: 8
/* VGA HSYNC mode
Reference IO setting table*/
#define offset_of_padtop_reg_vga_hsync_mode (238)
#define mask_of_padtop_reg_vga_hsync_mode   (0x100)
    unsigned int reg_vga_hsync_mode : 1;

// h0077, bit: 9
/* VGA VSYNC mode
Reference IO setting table*/
#define offset_of_padtop_reg_vga_vsync_mode (238)
#define mask_of_padtop_reg_vga_vsync_mode   (0x200)
    unsigned int reg_vga_vsync_mode : 1;

    // h0077, bit: 14
    /* */
    unsigned int : 6;

    // h0077
    unsigned int /* padding 16 bit */ : 16;

// h0078, bit: 1
/* Ttl 16bit mode
Reference IO setting table*/
#define offset_of_padtop_reg_ttl16_mode (240)
#define mask_of_padtop_reg_ttl16_mode   (0x3)
    unsigned int reg_ttl16_mode : 2;

    // h0078, bit: 14
    /* */
    unsigned int : 14;

    // h0078
    unsigned int /* padding 16 bit */ : 16;

// h0079, bit: 1
/* DMIC 4 ch Mode
Reference IO setting table*/
#define offset_of_padtop_reg_dmic_4ch_mode (242)
#define mask_of_padtop_reg_dmic_4ch_mode   (0x3)
    unsigned int reg_dmic_4ch_mode : 2;

    // h0079, bit: 7
    /* */
    unsigned int : 6;

// h0079, bit: 10
/* DMIC 6 ch Mode
Reference IO setting table*/
#define offset_of_padtop_reg_dmic_2ch_mode (242)
#define mask_of_padtop_reg_dmic_2ch_mode   (0x700)
    unsigned int reg_dmic_2ch_mode : 3;

    // h0079, bit: 14
    /* */
    unsigned int : 5;

    // h0079
    unsigned int /* padding 16 bit */ : 16;

// h007a, bit: 0
/* SD0 BOOT Mode
Reference IO setting table*/
#define offset_of_padtop_reg_sd0_boot_mode (244)
#define mask_of_padtop_reg_sd0_boot_mode   (0x1)
    unsigned int reg_sd0_boot_mode : 1;

    // h007a, bit: 3
    /* */
    unsigned int : 3;

// h007a, bit: 4
/* SD PWR Mode
Reference IO setting table*/
#define offset_of_padtop_reg_sd_pwr_on (244)
#define mask_of_padtop_reg_sd_pwr_on   (0x10)
    unsigned int reg_sd_pwr_on : 1;

    // h007a, bit: 7
    /* */
    unsigned int : 3;

// h007a, bit: 8
/* SD VCTRL Mode
Reference IO setting table*/
#define offset_of_padtop_reg_sd_vctrl (244)
#define mask_of_padtop_reg_sd_vctrl   (0x100)
    unsigned int reg_sd_vctrl : 1;

    // h007a, bit: 11
    /* */
    unsigned int : 3;

// h007a, bit: 12
/* SD CDZ In
Reference IO setting table*/
#define offset_of_padtop_reg_sd_cdz (244)
#define mask_of_padtop_reg_sd_cdz   (0x1000)
    unsigned int reg_sd_cdz : 1;

    // h007a, bit: 14
    /* */
    unsigned int : 3;

    // h007a
    unsigned int /* padding 16 bit */ : 16;

// h007b, bit: 0
/* SATA LED mode
Reference IO setting table*/
#define offset_of_padtop_reg_sata0_led_mode (246)
#define mask_of_padtop_reg_sata0_led_mode   (0x1)
    unsigned int reg_sata0_led_mode : 1;

    // h007b, bit: 3
    /* */
    unsigned int : 3;

// h007b, bit: 4
/* SATA LED mode
Reference IO setting table*/
#define offset_of_padtop_reg_sata1_led_mode (246)
#define mask_of_padtop_reg_sata1_led_mode   (0x10)
    unsigned int reg_sata1_led_mode : 1;

    // h007b, bit: 14
    /* */
    unsigned int : 11;

    // h007b
    unsigned int /* padding 16 bit */ : 16;

// h007c, bit: 0
/* SPI HOLDN mode*/
#define offset_of_padtop_reg_spiholdn_mode (248)
#define mask_of_padtop_reg_spiholdn_mode   (0x1)
    unsigned int reg_spiholdn_mode : 1;

    // h007c, bit: 14
    /* */
    unsigned int : 15;

    // h007c
    unsigned int /* padding 16 bit */ : 16;

// h007d, bit: 0
/* SPI CSZ2 mode*/
#define offset_of_padtop_reg_qspicsz2_mode (250)
#define mask_of_padtop_reg_qspicsz2_mode   (0x1)
    unsigned int reg_qspicsz2_mode : 1;

    // h007d, bit: 14
    /* */
    unsigned int : 15;

    // h007d
    unsigned int /* padding 16 bit */ : 16;

// h007e, bit: 5
/* */
#define offset_of_padtop_reg_sdio_drv (252)
#define mask_of_padtop_reg_sdio_drv   (0x3f)
    unsigned int reg_sdio_drv : 6;

    // h007e, bit: 7
    /* */
    unsigned int : 2;

// h007e, bit: 8
/* 0: general mode
1: CMD CLK change, D0 D3 change, D1 D2 change*/
#define offset_of_padtop_reg_sd30_pad_mux_mode (252)
#define mask_of_padtop_reg_sd30_pad_mux_mode   (0x100)
    unsigned int reg_sd30_pad_mux_mode : 1;

    // h007e, bit: 14
    /* */
    unsigned int : 7;

    // h007e
    unsigned int /* padding 16 bit */ : 16;

// h007f, bit: 5
/* */
#define offset_of_padtop_reg_sdio_pe (254)
#define mask_of_padtop_reg_sdio_pe   (0x3f)
    unsigned int reg_sdio_pe : 6;

    // h007f, bit: 7
    /* */
    unsigned int : 2;

// h007f, bit: 13
/* */
#define offset_of_padtop_reg_sdio_ps (254)
#define mask_of_padtop_reg_sdio_ps   (0x3f00)
    unsigned int reg_sdio_ps : 6;

    // h007f, bit: 14
    /* */
    unsigned int : 2;

    // h007f
    unsigned int /* padding 16 bit */ : 16;

} __attribute__((packed, aligned(1))) reg_padtop;
#endif
