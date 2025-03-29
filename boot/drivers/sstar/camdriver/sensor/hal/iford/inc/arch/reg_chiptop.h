/*
 * reg_chiptop.h - Sigmastar
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

#ifndef __REG_CHIPTOP__
#define __REG_CHIPTOP__
typedef struct
{
// h0000, bit: 0
/* MIU clock select DFT clock
MPLL_SYN clock select DFT clock
MIU_REC clock select DFT clock
GE clock select DFT clock*/
#define offset_of_chiptop_reg_ckg_alldft (0)
#define mask_of_chiptop_reg_ckg_alldft   (0x1)
    unsigned int reg_ckg_alldft : 1;

    // h0000, bit: 14
    /* */
    unsigned int : 15;

    // h0000
    unsigned int /* padding 16 bit */ : 16;

    // h0001, bit: 8
    /* */
    unsigned int : 9;

// h0001, bit: 9
/* FT mode
1: enable FT mode
0: disable FT mode*/
#define offset_of_chiptop_reg_ft_mode (2)
#define mask_of_chiptop_reg_ft_mode   (0x200)
    unsigned int reg_ft_mode : 1;

    // h0001, bit: 11
    /* */
    unsigned int : 2;

// h0001, bit: 12
/* digital pads set high*/
#define offset_of_chiptop_reg_seth (2)
#define mask_of_chiptop_reg_seth   (0x1000)
    unsigned int reg_seth : 1;

// h0001, bit: 13
/* digital pads set low*/
#define offset_of_chiptop_reg_setl (2)
#define mask_of_chiptop_reg_setl   (0x2000)
    unsigned int reg_setl : 1;

    // h0001, bit: 14
    /* */
    unsigned int : 2;

    // h0001
    unsigned int /* padding 16 bit */ : 16;

    // h0002, bit: 14
    /* */
    unsigned int : 16;

    // h0002
    unsigned int /* padding 16 bit */ : 16;

// h0003, bit: 0
/* for dla rst protect. 1: enable*/
#define offset_of_chiptop_reg_dla_rst_protect_en (6)
#define mask_of_chiptop_reg_dla_rst_protect_en   (0x1)
    unsigned int reg_dla_rst_protect_en : 1;

    // h0003, bit: 14
    /* */
    unsigned int : 15;

    // h0003
    unsigned int /* padding 16 bit */ : 16;

    // h0004, bit: 14
    /* */
    unsigned int : 16;

    // h0004
    unsigned int /* padding 16 bit */ : 16;

    // h0005, bit: 14
    /* */
    unsigned int : 16;

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

    // h0010, bit: 14
    /* */
    unsigned int : 16;

    // h0010
    unsigned int /* padding 16 bit */ : 16;

    // h0011, bit: 14
    /* */
    unsigned int : 16;

    // h0011
    unsigned int /* padding 16 bit */ : 16;

// h0012, bit: 1
/* select TEST IN mode
2'd0: TEST_IN functions are not enabled
2'd1: TEST_IN[23:0] use FUART/SR/SPI0 pads
2'd2: TEST_IN[23:0] use I2C0/SD/USB/SAR/PM/ETH pads
2'd3: TEST_IN[14:0] use SAR/ETH/FUART/SPI0/SD/USB pads*/
#define offset_of_chiptop_reg_test_in_mode (36)
#define mask_of_chiptop_reg_test_in_mode   (0x3)
    unsigned int reg_test_in_mode : 2;

    // h0012, bit: 3
    /* */
    unsigned int : 2;

// h0012, bit: 5
/* select TEST_OUT mode
2'd0: TEST_OUT functions are not enabled.
2'd1: TEST_OUT[23:0] use FUART/SR/SPI0 pads
2'd2: TEST_OUT[23:0] use I2C0/SD/USB/SAR/PM/ETH pads
2'd3: TEST_OUT[15:0] use SR pads*/
#define offset_of_chiptop_reg_test_out_mode (36)
#define mask_of_chiptop_reg_test_out_mode   (0x30)
    unsigned int reg_test_out_mode : 2;

    // h0012, bit: 14
    /* */
    unsigned int : 10;

    // h0012
    unsigned int /* padding 16 bit */ : 16;

// h0013, bit: 0
/* Riu-record dma soft resetz*/
#define offset_of_chiptop_reg_dbg_dma_rstz (38)
#define mask_of_chiptop_reg_dbg_dma_rstz   (0x1)
    unsigned int reg_dbg_dma_rstz : 1;

    // h0013, bit: 14
    /* */
    unsigned int : 15;

    // h0013
    unsigned int /* padding 16 bit */ : 16;

    // h0014, bit: 0
    /* */
    unsigned int : 1;

// h0014, bit: 1
/* gating cpu porst for ca7*/
#define offset_of_chiptop_reg_gat_ca7_cpu_porst (40)
#define mask_of_chiptop_reg_gat_ca7_cpu_porst   (0x2)
    unsigned int reg_gat_ca7_cpu_porst : 1;

    // h0014, bit: 14
    /* */
    unsigned int : 14;

    // h0014
    unsigned int /* padding 16 bit */ : 16;

    // h0015, bit: 1
    /* */
    unsigned int : 2;

// h0015, bit: 2
/* gating mcu3 cpu rstz for ca7*/
#define offset_of_chiptop_reg_gat_ca7_mcu3_cpu_rstz (42)
#define mask_of_chiptop_reg_gat_ca7_mcu3_cpu_rstz   (0x4)
    unsigned int reg_gat_ca7_mcu3_cpu_rstz : 1;

    // h0015, bit: 14
    /* */
    unsigned int : 13;

    // h0015
    unsigned int /* padding 16 bit */ : 16;

    // h0016, bit: 2
    /* */
    unsigned int : 3;

// h0016, bit: 3
/* gating mcu3 rstz for ca7*/
#define offset_of_chiptop_reg_gat_ca7_mcu3_rstz (44)
#define mask_of_chiptop_reg_gat_ca7_mcu3_rstz   (0x8)
    unsigned int reg_gat_ca7_mcu3_rstz : 1;

    // h0016, bit: 14
    /* */
    unsigned int : 12;

    // h0016
    unsigned int /* padding 16 bit */ : 16;

    // h0017, bit: 14
    /* */
    unsigned int : 16;

    // h0017
    unsigned int /* padding 16 bit */ : 16;

    // h0018, bit: 14
    /* */
    unsigned int : 16;

    // h0018
    unsigned int /* padding 16 bit */ : 16;

    // h0019, bit: 14
    /* */
    unsigned int : 16;

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

    // h001c, bit: 11
    /* */
    unsigned int : 12;

// h001c, bit: 12
/* force all of the whole chip sram power-on*/
#define offset_of_chiptop_reg_force_allsram_on (56)
#define mask_of_chiptop_reg_force_allsram_on   (0x1000)
    unsigned int reg_force_allsram_on : 1;

    // h001c, bit: 13
    /* */
    unsigned int : 1;

// h001c, bit: 14
/* BIST mode start*/
#define offset_of_chiptop_reg_bist_start_ext (56)
#define mask_of_chiptop_reg_bist_start_ext   (0x4000)
    unsigned int reg_bist_start_ext : 1;

// h001c, bit: 15
/* BIST mode enable (disable in default)*/
#define offset_of_chiptop_reg_bist_mode_ext (56)
#define mask_of_chiptop_reg_bist_mode_ext   (0x8000)
    unsigned int reg_bist_mode_ext : 1;

    // h001c
    unsigned int /* padding 16 bit */ : 16;

// h001d, bit: 14
/* indicate sram done
[0]: dig_gp
[1]: pm_gp
[2]: sc_gp
[3]: ven_gp3
[4]: ven_gp2
[5]:ven_gp1
[6]:ven_gp0
[7]:pcie2_gp
[8]:pcie_gp
[9]:sata2_gp
[10]:sata1_gp
[11]:usb30_drd_gp
[12]:ipu_die
[13]:disp_gp
[14]: mcu_gp
[15]: mcu2_gp*/
#define offset_of_chiptop_reg_bist_done_0 (58)
#define mask_of_chiptop_reg_bist_done_0   (0xffff)
    unsigned int reg_bist_done_0 : 16;

    // h001d
    unsigned int /* padding 16 bit */ : 16;

// h001e, bit: 14
/* indicate sram done
[0]: dig_gp
[1]: pm_gp
[2]: sc_gp
[3]: ven_gp3
[4]: ven_gp2
[5]:ven_gp1
[6]:ven_gp0
[7]:pcie2_gp
[8]:pcie_gp
[9]:sata2_gp
[10]:sata1_gp
[11]:usb30_drd_gp
[12]:ipu_die
[13]:disp_gp
[14]: mcu_gp
[15]: mcu2_gp*/
#define offset_of_chiptop_reg_bist_fail_0 (60)
#define mask_of_chiptop_reg_bist_fail_0   (0xffff)
    unsigned int reg_bist_fail_0 : 16;

    // h001e
    unsigned int /* padding 16 bit */ : 16;

// h001f, bit: 0
/* BIST mode enable (disable in default)*/
#define offset_of_chiptop_reg_bist_mode_ext_dla (62)
#define mask_of_chiptop_reg_bist_mode_ext_dla   (0x1)
    unsigned int reg_bist_mode_ext_dla : 1;

    // h001f, bit: 6
    /* */
    unsigned int : 6;

// h001f, bit: 7
/* SC block sw reset*/
#define offset_of_chiptop_reg_sw_rstz_sc (62)
#define mask_of_chiptop_reg_sw_rstz_sc   (0x80)
    unsigned int reg_sw_rstz_sc : 1;

// h001f, bit: 8
/* DLA sw reset*/
#define offset_of_chiptop_reg_sw_rstz_dla (62)
#define mask_of_chiptop_reg_sw_rstz_dla   (0x100)
    unsigned int reg_sw_rstz_dla : 1;

    // h001f, bit: 9
    /* */
    unsigned int : 1;

// h001f, bit: 10
/* VENC0 sw reset*/
#define offset_of_chiptop_reg_sw_rstz_venc0 (62)
#define mask_of_chiptop_reg_sw_rstz_venc0   (0x400)
    unsigned int reg_sw_rstz_venc0 : 1;

    // h001f, bit: 13
    /* */
    unsigned int : 3;

// h001f, bit: 14
/* ISP0 sw reset*/
#define offset_of_chiptop_reg_sw_rstz_isp0 (62)
#define mask_of_chiptop_reg_sw_rstz_isp0   (0x4000)
    unsigned int reg_sw_rstz_isp0 : 1;

    // h001f, bit: 15
    /* */
    unsigned int : 1;

    // h001f
    unsigned int /* padding 16 bit */ : 16;

// h0020, bit: 14
/* dummy registers for CHIPTOP*/
#define offset_of_chiptop_reg_chiptop_dummy_0 (64)
#define mask_of_chiptop_reg_chiptop_dummy_0   (0xffff)
    unsigned int reg_chiptop_dummy_0 : 16;

    // h0020
    unsigned int /* padding 16 bit */ : 16;

// h0021, bit: 14
/* dummy registers for CHIPTOP*/
#define offset_of_chiptop_reg_chiptop_dummy_1 (66)
#define mask_of_chiptop_reg_chiptop_dummy_1   (0xffff)
    unsigned int reg_chiptop_dummy_1 : 16;

    // h0021
    unsigned int /* padding 16 bit */ : 16;

// h0022, bit: 14
/* dummy registers for CHIPTOP*/
#define offset_of_chiptop_reg_chiptop_dummy_2 (68)
#define mask_of_chiptop_reg_chiptop_dummy_2   (0xffff)
    unsigned int reg_chiptop_dummy_2 : 16;

    // h0022
    unsigned int /* padding 16 bit */ : 16;

// h0023, bit: 14
/* [0]:dft_RC_sel, 0 : C ; 1 : CRC; [15:1]:dummay reg*/
#define offset_of_chiptop_reg_chiptop_dummy_3 (70)
#define mask_of_chiptop_reg_chiptop_dummy_3   (0xffff)
    unsigned int reg_chiptop_dummy_3 : 16;

    // h0023
    unsigned int /* padding 16 bit */ : 16;

// h0024, bit: 0
/* reg_gate_xtal24_core*/
#define offset_of_chiptop_reg_gate_xtal24_core (72)
#define mask_of_chiptop_reg_gate_xtal24_core   (0x1)
    unsigned int reg_gate_xtal24_core : 1;

    // h0024, bit: 3
    /* */
    unsigned int : 3;

// h0024, bit: 4
/* reg_gate_xtal24_nodie*/
#define offset_of_chiptop_reg_gate_xtal24_nodie (72)
#define mask_of_chiptop_reg_gate_xtal24_nodie   (0x10)
    unsigned int reg_gate_xtal24_nodie : 1;

    // h0024, bit: 7
    /* */
    unsigned int : 3;

// h0024, bit: 8
/* reg_gate_xtal12_core*/
#define offset_of_chiptop_reg_gate_xtal12_core (72)
#define mask_of_chiptop_reg_gate_xtal12_core   (0x100)
    unsigned int reg_gate_xtal12_core : 1;

    // h0024, bit: 11
    /* */
    unsigned int : 3;

// h0024, bit: 12
/* reg_gate_xtal12_nodie*/
#define offset_of_chiptop_reg_gate_xtal12_nodie (72)
#define mask_of_chiptop_reg_gate_xtal12_nodie   (0x1000)
    unsigned int reg_gate_xtal12_nodie : 1;

    // h0024, bit: 14
    /* */
    unsigned int : 3;

    // h0024
    unsigned int /* padding 16 bit */ : 16;

// h0025, bit: 0
/* reg_wakeup_xtal_irq_clr*/
#define offset_of_chiptop_reg_wakeup_xtal_irq_clr (74)
#define mask_of_chiptop_reg_wakeup_xtal_irq_clr   (0x1)
    unsigned int reg_wakeup_xtal_irq_clr : 1;

    // h0025, bit: 14
    /* */
    unsigned int : 15;

    // h0025
    unsigned int /* padding 16 bit */ : 16;

// h0026, bit: 0
/* reg_vad_gpi_en*/
#define offset_of_chiptop_reg_vad_gpi_en (76)
#define mask_of_chiptop_reg_vad_gpi_en   (0x1)
    unsigned int reg_vad_gpi_en : 1;

// h0026, bit: 1
/* reg_vad_32k_mode*/
#define offset_of_chiptop_reg_vad_32k_mode (76)
#define mask_of_chiptop_reg_vad_32k_mode   (0x2)
    unsigned int reg_vad_32k_mode : 1;

// h0026, bit: 2
/* reg_vad_loop_mode*/
#define offset_of_chiptop_reg_vad_loop_mode (76)
#define mask_of_chiptop_reg_vad_loop_mode   (0x4)
    unsigned int reg_vad_loop_mode : 1;

// h0026, bit: 3
/* reg_wakeup_xtal_vad_event*/
#define offset_of_chiptop_reg_wakeup_xtal_vad_event (76)
#define mask_of_chiptop_reg_wakeup_xtal_vad_event   (0x8)
    unsigned int reg_wakeup_xtal_vad_event : 1;

    // h0026, bit: 14
    /* */
    unsigned int : 12;

    // h0026
    unsigned int /* padding 16 bit */ : 16;

// h0027, bit: 0
/* reg_xtal_usb_suspend_mode*/
#define offset_of_chiptop_reg_xtal_usb_suspend_mode (78)
#define mask_of_chiptop_reg_xtal_usb_suspend_mode   (0x1)
    unsigned int reg_xtal_usb_suspend_mode : 1;

// h0027, bit: 1
/* reg_wakeup_xtal_usb_event*/
#define offset_of_chiptop_reg_wakeup_xtal_usb_event (78)
#define mask_of_chiptop_reg_wakeup_xtal_usb_event   (0x2)
    unsigned int reg_wakeup_xtal_usb_event : 1;

    // h0027, bit: 14
    /* */
    unsigned int : 14;

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

// h002d, bit: 0
/* reg_wakeup_xtal_irq_en*/
#define offset_of_chiptop_reg_wakeup_xtal_irq_en (90)
#define mask_of_chiptop_reg_wakeup_xtal_irq_en   (0x1)
    unsigned int reg_wakeup_xtal_irq_en : 1;

    // h002d, bit: 14
    /* */
    unsigned int : 15;

    // h002d
    unsigned int /* padding 16 bit */ : 16;

// h002e, bit: 14
/* reg_wakeup_debounce_time*/
#define offset_of_chiptop_reg_wakeup_debounce_time (92)
#define mask_of_chiptop_reg_wakeup_debounce_time   (0xffff)
    unsigned int reg_wakeup_debounce_time : 16;

    // h002e
    unsigned int /* padding 16 bit */ : 16;

// h002f, bit: 0
/* reg_wakeup_xtal_irq*/
#define offset_of_chiptop_reg_wakeup_xtal_irq (94)
#define mask_of_chiptop_reg_wakeup_xtal_irq   (0x1)
    unsigned int reg_wakeup_xtal_irq : 1;

    // h002f, bit: 14
    /* */
    unsigned int : 15;

    // h002f
    unsigned int /* padding 16 bit */ : 16;

// h0030, bit: 0
/* reg sw dla nodie restz*/
#define offset_of_chiptop_reg_sw_rstz_dla_nodie (96)
#define mask_of_chiptop_reg_sw_rstz_dla_nodie   (0x1)
    unsigned int reg_sw_rstz_dla_nodie : 1;

    // h0030, bit: 14
    /* */
    unsigned int : 15;

    // h0030
    unsigned int /* padding 16 bit */ : 16;

    // h0031, bit: 14
    /* */
    unsigned int : 16;

    // h0031
    unsigned int /* padding 16 bit */ : 16;

    // h0032, bit: 14
    /* */
    unsigned int : 16;

    // h0032
    unsigned int /* padding 16 bit */ : 16;

    // h0033, bit: 14
    /* */
    unsigned int : 16;

    // h0033
    unsigned int /* padding 16 bit */ : 16;

    // h0034, bit: 14
    /* */
    unsigned int : 16;

    // h0034
    unsigned int /* padding 16 bit */ : 16;

    // h0035, bit: 14
    /* */
    unsigned int : 16;

    // h0035
    unsigned int /* padding 16 bit */ : 16;

    // h0036, bit: 14
    /* */
    unsigned int : 16;

    // h0036
    unsigned int /* padding 16 bit */ : 16;

    // h0037, bit: 14
    /* */
    unsigned int : 16;

    // h0037
    unsigned int /* padding 16 bit */ : 16;

// h0038, bit: 0
/* vad int mask*/
#define offset_of_chiptop_reg_vad_int_mask (112)
#define mask_of_chiptop_reg_vad_int_mask   (0x1)
    unsigned int reg_vad_int_mask : 1;

    // h0038, bit: 14
    /* */
    unsigned int : 15;

    // h0038
    unsigned int /* padding 16 bit */ : 16;

    // h0039, bit: 14
    /* */
    unsigned int : 16;

    // h0039
    unsigned int /* padding 16 bit */ : 16;

    // h003a, bit: 14
    /* */
    unsigned int : 16;

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

// h0040, bit: 1
/* clock mcu gating control
00: use mcu_bridge_en (HW saving mode 0)
01: use mcu_bridge_en_d (HW saving mode 1, prefer)
10: SW saving power mode
11: Always enable (default)*/
#define offset_of_chiptop_reg_mcu_bridge_en_mode (128)
#define mask_of_chiptop_reg_mcu_bridge_en_mode   (0x3)
    unsigned int reg_mcu_bridge_en_mode : 2;

    // h0040, bit: 14
    /* */
    unsigned int : 14;

    // h0040
    unsigned int /* padding 16 bit */ : 16;

// h0041, bit: 1
/* clock x32_mcu gating control
 00: use x32_mcu_bridge_en (HW saving mode 0)
 01: use x32_mcu_bridge_en_d (HW saving mode 1, prefer)
 10: SW saving power mode
 11: Always enable (default) */
#define offset_of_chiptop_reg_x32_mcu_bridge_en_mode (130)
#define mask_of_chiptop_reg_x32_mcu_bridge_en_mode   (0x3)
    unsigned int reg_x32_mcu_bridge_en_mode : 2;

    // h0041, bit: 14
    /* */
    unsigned int : 14;

    // h0041
    unsigned int /* padding 16 bit */ : 16;

    // h0042, bit: 14
    /* */
    unsigned int : 16;

    // h0042
    unsigned int /* padding 16 bit */ : 16;

    // h0043, bit: 14
    /* */
    unsigned int : 16;

    // h0043
    unsigned int /* padding 16 bit */ : 16;

// h0044, bit: 14
/*  riu write clock mask
     [0]: sc_gp
     [1]: vhe_gp
     [2]: hemcu_gp
     [3]: mipi_gp
     [4]: mcu_if_gp
     [5]: others*/
#define offset_of_chiptop_reg_riu_wclk_mask (136)
#define mask_of_chiptop_reg_riu_wclk_mask   (0xffff)
    unsigned int reg_riu_wclk_mask : 16;

    // h0044
    unsigned int /* padding 16 bit */ : 16;

// h0045, bit: 14
/* Reserved register*/
#define offset_of_chiptop_reg_reserved3 (138)
#define mask_of_chiptop_reg_reserved3   (0xffff)
    unsigned int reg_reserved3 : 16;

    // h0045
    unsigned int /* padding 16 bit */ : 16;

// h0046, bit: 14
/* Reserved register*/
#define offset_of_chiptop_reg_reserved4 (140)
#define mask_of_chiptop_reg_reserved4   (0xffff)
    unsigned int reg_reserved4 : 16;

    // h0046
    unsigned int /* padding 16 bit */ : 16;

// h0047, bit: 7
/* bonding overwrite enable*/
#define offset_of_chiptop_reg_bond_ov_en (142)
#define mask_of_chiptop_reg_bond_ov_en   (0xff)
    unsigned int reg_bond_ov_en : 8;

// h0047, bit: 14
/* bonding overwrite value*/
#define offset_of_chiptop_reg_bond_ov (142)
#define mask_of_chiptop_reg_bond_ov   (0xff00)
    unsigned int reg_bond_ov : 8;

    // h0047
    unsigned int /* padding 16 bit */ : 16;

// h0048, bit: 7
/* bonding value*/
#define offset_of_chiptop_reg_bond_in (144)
#define mask_of_chiptop_reg_bond_in   (0xff)
    unsigned int reg_bond_in : 8;

    // h0048, bit: 14
    /* */
    unsigned int : 8;

    // h0048
    unsigned int /* padding 16 bit */ : 16;

// h0049, bit: 6
/* enable xiu pipe function*/
#define offset_of_chiptop_reg_xiu_pipe_en (146)
#define mask_of_chiptop_reg_xiu_pipe_en   (0x7f)
    unsigned int reg_xiu_pipe_en : 7;

    // h0049, bit: 14
    /* */
    unsigned int : 9;

    // h0049
    unsigned int /* padding 16 bit */ : 16;

    // h004a, bit: 14
    /* */
    unsigned int : 16;

    // h004a
    unsigned int /* padding 16 bit */ : 16;

    // h004b, bit: 14
    /* */
    unsigned int : 16;

    // h004b
    unsigned int /* padding 16 bit */ : 16;

    // h004c, bit: 14
    /* */
    unsigned int : 16;

    // h004c
    unsigned int /* padding 16 bit */ : 16;

    // h004d, bit: 14
    /* */
    unsigned int : 16;

    // h004d
    unsigned int /* padding 16 bit */ : 16;

    // h004e, bit: 14
    /* */
    unsigned int : 16;

    // h004e
    unsigned int /* padding 16 bit */ : 16;

    // h004f, bit: 14
    /* */
    unsigned int : 16;

    // h004f
    unsigned int /* padding 16 bit */ : 16;

    // h0050, bit: 14
    /* */
    unsigned int : 15;

// h0050, bit: 15
/* 1: Set all pads (except PM) as input*/
#define offset_of_chiptop_reg_allpad_in (160)
#define mask_of_chiptop_reg_allpad_in   (0x8000)
    unsigned int reg_allpad_in : 1;

    // h0050
    unsigned int /* padding 16 bit */ : 16;

// h0051, bit: 14
/* indicate sram done
[0]: isp0_gp
[1]: isp1_gp
[2]: net_gp
[3]: miu1_gp
[4]:miu0_gp
[5]~[14]: N/A
[15]: all*/
#define offset_of_chiptop_reg_bist_done_1 (162)
#define mask_of_chiptop_reg_bist_done_1   (0xffff)
    unsigned int reg_bist_done_1 : 16;

    // h0051
    unsigned int /* padding 16 bit */ : 16;

// h0052, bit: 14
/* indicate sram done
[0]: isp0_gp
[1]: isp1_gp
[2]: net_gp
[3]: miu1_gp
[4]:miu0_gp
[5]~[14]: N/A
[15]: all*/
#define offset_of_chiptop_reg_bist_fail_1 (164)
#define mask_of_chiptop_reg_bist_fail_1   (0xffff)
    unsigned int reg_bist_fail_1 : 16;

    // h0052
    unsigned int /* padding 16 bit */ : 16;

// h0053, bit: 3
/* Select controller for PAD_PM_UART_RX and PAD_PM_UART_TX
0000:  N/A
0001:  FUART
0010:  UART0
0011:  UART1
0100:  UART2
0101:  UART3
0110：VEN_UART0
0111:  UART4
1000：UART5
1001:  VEN_UART1
1010:  VEN_UART2
1011:  VEN_UART3
1100:  IPU_UART0
1101:  IPU_UART1
Note: For PAD_PM_UART_RX and PAD_PM_UART_TX, please refer to the "reg_hk51_uart0_en" and "reg_uart_rx_enable" in
reg_pm_sleep.xls (a). "reg_hk51_uart0_en" == 0 (b). "reg_uart_rx_enable" == 1*/
#define offset_of_chiptop_reg_uart_sel0 (166)
#define mask_of_chiptop_reg_uart_sel0   (0xf)
    unsigned int reg_uart_sel0 : 4;

// h0053, bit: 7
/* Select controller for PAD_FUART_RX and PAD_FUART_TX*/
#define offset_of_chiptop_reg_uart_sel1 (166)
#define mask_of_chiptop_reg_uart_sel1   (0xf0)
    unsigned int reg_uart_sel1 : 4;

// h0053, bit: 11
/* Select controller for PAD_UART0_RX and PAD_UART0_TX*/
#define offset_of_chiptop_reg_uart_sel2 (166)
#define mask_of_chiptop_reg_uart_sel2   (0xf00)
    unsigned int reg_uart_sel2 : 4;

// h0053, bit: 14
/* Select controller for PAD_UART1_RX and PAD_UART1_TX
0000:  N/A
0001:  FUART
0010:  UART0
0011:  UART1
0100:  UART2
0101:  UART3
0110：VEN_UART0
0111:  UART4
1000：UART5
1001:  VEN_UART1
1010:  VEN_UART2
1011:  VEN_UART3
1100:  IPU_UART0
1101:  IPU_UART1*/
#define offset_of_chiptop_reg_uart_sel3 (166)
#define mask_of_chiptop_reg_uart_sel3   (0xf000)
    unsigned int reg_uart_sel3 : 4;

    // h0053
    unsigned int /* padding 16 bit */ : 16;

// h0054, bit: 3
/* Select controller for PAD_UART2_RX and PAD_UART2_TX*/
#define offset_of_chiptop_reg_uart_sel4 (168)
#define mask_of_chiptop_reg_uart_sel4   (0xf)
    unsigned int reg_uart_sel4 : 4;

// h0054, bit: 7
/* Select controller for PAD_UART3_RX and PAD_UART3_TX*/
#define offset_of_chiptop_reg_uart_sel5 (168)
#define mask_of_chiptop_reg_uart_sel5   (0xf0)
    unsigned int reg_uart_sel5 : 4;

    // h0054, bit: 14
    /* */
    unsigned int : 8;

    // h0054
    unsigned int /* padding 16 bit */ : 16;

// h0055, bit: 3
/* JTAG selection (useless on iNfinity)*/
#define offset_of_chiptop_reg_jtag_sel (170)
#define mask_of_chiptop_reg_jtag_sel   (0xf)
    unsigned int reg_jtag_sel : 4;

    // h0055, bit: 7
    /* */
    unsigned int : 4;

// h0055, bit: 11
/* invert PAD UART TX/RX*/
#define offset_of_chiptop_reg_uart_pad_inverse (170)
#define mask_of_chiptop_reg_uart_pad_inverse   (0xf00)
    unsigned int reg_uart_pad_inverse : 4;

    // h0055, bit: 14
    /* */
    unsigned int : 4;

    // h0055
    unsigned int /* padding 16 bit */ : 16;

// h0056, bit: 3
/* enable of inner loopback test for 8 sets of UART controller
[0]: N/A
[1]: FUART enable
[2]: UART0 enable
[3]: UART1 enable
[4]: UART2 enable
[5]: UART3 enable
*/
#define offset_of_chiptop_reg_uart_inner_loopback (172)
#define mask_of_chiptop_reg_uart_inner_loopback   (0xf)
    unsigned int reg_uart_inner_loopback : 4;

    // h0056, bit: 7
    /* */
    unsigned int : 4;

// h0056, bit: 11
/* enable of outer loopback test for 8 sets of UART pad
[0]: PM_UART enable
[1]: FUART enable
[2]: UART0 enable
[3]: UART1 enable
[4]: UART2 enable
[5]: UART3 enable*/
#define offset_of_chiptop_reg_uart_outer_loopback (172)
#define mask_of_chiptop_reg_uart_outer_loopback   (0xf00)
    unsigned int reg_uart_outer_loopback : 4;

    // h0056, bit: 14
    /* */
    unsigned int : 4;

    // h0056
    unsigned int /* padding 16 bit */ : 16;

// h0057, bit: 3
/* disable RX signals from PADs*/
#define offset_of_chiptop_reg_force_rx_disable (174)
#define mask_of_chiptop_reg_force_rx_disable   (0xf)
    unsigned int reg_force_rx_disable : 4;

    // h0057, bit: 14
    /* */
    unsigned int : 12;

    // h0057
    unsigned int /* padding 16 bit */ : 16;

    // h0058, bit: 14
    /* */
    unsigned int : 16;

    // h0058
    unsigned int /* padding 16 bit */ : 16;

// h0059, bit: 3
/* Select controller for PAD_PM_UART_RX and PAD_PM_UART_TX
0000:  N/A
0001:  FUART
0010:  UART0
0011:  UART1
0100:  UART2
0101:  UART3
0110：VEN_UART0
0111:  UART4
1000：UART5
1001:  VEN_UART1
1010:  VEN_UART2
1011:  VEN_UART3
1100:  IPU_UART0
1101:  IPU_UART1
Note: For PAD_PM_UART_RX and PAD_PM_UART_TX, please refer to the "reg_hk51_uart0_en" and "reg_uart_rx_enable" in
reg_pm_sleep.xls (a). "reg_hk51_uart0_en" == 0 (b). "reg_uart_rx_enable" == 1*/
#define offset_of_chiptop_reg_uart_sel6 (178)
#define mask_of_chiptop_reg_uart_sel6   (0xf)
    unsigned int reg_uart_sel6 : 4;

// h0059, bit: 7
/* Select controller for PAD_UART3_RX and PAD_UART3_TX*/
#define offset_of_chiptop_reg_uart_sel7 (178)
#define mask_of_chiptop_reg_uart_sel7   (0xf0)
    unsigned int reg_uart_sel7 : 4;

    // h0059, bit: 14
    /* */
    unsigned int : 8;

    // h0059
    unsigned int /* padding 16 bit */ : 16;

// h005a, bit: 14
/* The address of SDRAM for ca55*/
#define offset_of_chiptop_reg_sdram_offset (180)
#define mask_of_chiptop_reg_sdram_offset   (0xffff)
    unsigned int reg_sdram_offset : 16;

    // h005a
    unsigned int /* padding 16 bit */ : 16;

// h005b, bit: 10
/* The address of SDRAM for ca55*/
#define offset_of_chiptop_reg_sdram_offset_1 (182)
#define mask_of_chiptop_reg_sdram_offset_1   (0x7ff)
    unsigned int reg_sdram_offset_1 : 11;

    // h005b, bit: 14
    /* */
    unsigned int : 5;

    // h005b
    unsigned int /* padding 16 bit */ : 16;

// h005c, bit: 14
/* The address  of SDRAM for ca7*/
#define offset_of_chiptop_reg_ca7_sdram_offset (184)
#define mask_of_chiptop_reg_ca7_sdram_offset   (0xffff)
    unsigned int reg_ca7_sdram_offset : 16;

    // h005c
    unsigned int /* padding 16 bit */ : 16;

// h005d, bit: 10
/* The address  of SDRAM for ca7*/
#define offset_of_chiptop_reg_ca7_sdram_offset_1 (186)
#define mask_of_chiptop_reg_ca7_sdram_offset_1   (0x7ff)
    unsigned int reg_ca7_sdram_offset_1 : 11;

    // h005d, bit: 14
    /* */
    unsigned int : 5;

    // h005d
    unsigned int /* padding 16 bit */ : 16;

// h005e, bit: 0
/* reg_vca7_boot_code_size*/
#define offset_of_chiptop_reg_ca7_boot_code_size (188)
#define mask_of_chiptop_reg_ca7_boot_code_size   (0x1)
    unsigned int reg_ca7_boot_code_size : 1;

    // h005e, bit: 14
    /* */
    unsigned int : 15;

    // h005e
    unsigned int /* padding 16 bit */ : 16;

// h005f, bit: 0
/* reg_mic_bypass*/
#define offset_of_chiptop_reg_mic_bypass (190)
#define mask_of_chiptop_reg_mic_bypass   (0x1)
    unsigned int reg_mic_bypass : 1;

    // h005f, bit: 14
    /* */
    unsigned int : 15;

    // h005f
    unsigned int /* padding 16 bit */ : 16;

    // h0060, bit: 14
    /* */
    unsigned int : 16;

    // h0060
    unsigned int /* padding 16 bit */ : 16;

    // h0061, bit: 14
    /* */
    unsigned int : 16;

    // h0061
    unsigned int /* padding 16 bit */ : 16;

    // h0062, bit: 14
    /* */
    unsigned int : 16;

    // h0062
    unsigned int /* padding 16 bit */ : 16;

    // h0063, bit: 14
    /* */
    unsigned int : 16;

    // h0063
    unsigned int /* padding 16 bit */ : 16;

    // h0064, bit: 14
    /* */
    unsigned int : 16;

    // h0064
    unsigned int /* padding 16 bit */ : 16;

// h0065, bit: 4
/* CHIP_CONFIG status*/
#define offset_of_chiptop_reg_chip_config_stat (202)
#define mask_of_chiptop_reg_chip_config_stat   (0x1f)
    unsigned int reg_chip_config_stat : 5;

    // h0065, bit: 7
    /* */
    unsigned int : 3;

// h0065, bit: 8
/* in_sel_sbus*/
#define offset_of_chiptop_reg_in_sel_sbus (202)
#define mask_of_chiptop_reg_in_sel_sbus   (0x100)
    unsigned int reg_in_sel_sbus : 1;

// h0065, bit: 9
/* in_sel_dbus*/
#define offset_of_chiptop_reg_in_sel_dbus (202)
#define mask_of_chiptop_reg_in_sel_dbus   (0x200)
    unsigned int reg_in_sel_dbus : 1;

    // h0065, bit: 11
    /* */
    unsigned int : 2;

// h0065, bit: 12
/* POWERGOOD_AVDD status*/
#define offset_of_chiptop_reg_powergood_avdd (202)
#define mask_of_chiptop_reg_powergood_avdd   (0x1000)
    unsigned int reg_powergood_avdd : 1;

    // h0065, bit: 14
    /* */
    unsigned int : 3;

    // h0065
    unsigned int /* padding 16 bit */ : 16;

    // h0066, bit: 10
    /* */
    unsigned int : 11;

// h0066, bit: 11
/* [ASIC]
1: boot from DRAM
[FPGA]
1: boot from spi*/
#define offset_of_chiptop_reg_ca7_boot_from_sdram (204)
#define mask_of_chiptop_reg_ca7_boot_from_sdram   (0x800)
    unsigned int reg_ca7_boot_from_sdram : 1;

    // h0066, bit: 14
    /* */
    unsigned int : 4;

    // h0066
    unsigned int /* padding 16 bit */ : 16;

// h0067, bit: 14
/* boot from 0x20000_0000*/
#define offset_of_chiptop_reg_ca7_boot_from_sdram_offset (206)
#define mask_of_chiptop_reg_ca7_boot_from_sdram_offset   (0xffff)
    unsigned int reg_ca7_boot_from_sdram_offset : 16;

    // h0067
    unsigned int /* padding 16 bit */ : 16;

// h0068, bit: 10
/* boot from 0x20000_0000*/
#define offset_of_chiptop_reg_ca7_boot_from_sdram_offset_1 (208)
#define mask_of_chiptop_reg_ca7_boot_from_sdram_offset_1   (0x7ff)
    unsigned int reg_ca7_boot_from_sdram_offset_1 : 11;

    // h0068, bit: 14
    /* */
    unsigned int : 5;

    // h0068
    unsigned int /* padding 16 bit */ : 16;

    // h0069, bit: 10
    /* */
    unsigned int : 11;

// h0069, bit: 11
/* boot from SDRAM
1: enable boot from SDRAM
0: disable boot from SDRAM*/
#define offset_of_chiptop_reg_boot_from_sdram (210)
#define mask_of_chiptop_reg_boot_from_sdram   (0x800)
    unsigned int reg_boot_from_sdram : 1;

    // h0069, bit: 14
    /* */
    unsigned int : 4;

    // h0069
    unsigned int /* padding 16 bit */ : 16;

// h006a, bit: 14
/* The booting address of SDRAM*/
#define offset_of_chiptop_reg_boot_from_sdram_offset (212)
#define mask_of_chiptop_reg_boot_from_sdram_offset   (0xffff)
    unsigned int reg_boot_from_sdram_offset : 16;

    // h006a
    unsigned int /* padding 16 bit */ : 16;

// h006b, bit: 10
/* The booting address of SDRAM*/
#define offset_of_chiptop_reg_boot_from_sdram_offset_1 (214)
#define mask_of_chiptop_reg_boot_from_sdram_offset_1   (0x7ff)
    unsigned int reg_boot_from_sdram_offset_1 : 11;

    // h006b, bit: 14
    /* */
    unsigned int : 5;

    // h006b
    unsigned int /* padding 16 bit */ : 16;

// h006c, bit: 0
/* DLA block ISO enable*/
#define offset_of_chiptop_reg_dla_iso_en (216)
#define mask_of_chiptop_reg_dla_iso_en   (0x1)
    unsigned int reg_dla_iso_en : 1;

    // h006c, bit: 3
    /* */
    unsigned int : 3;

// h006c, bit: 4
/* Power off DLA block*/
#define offset_of_chiptop_reg_dla_power_off (216)
#define mask_of_chiptop_reg_dla_power_off   (0x10)
    unsigned int reg_dla_power_off : 1;

    // h006c, bit: 7
    /* */
    unsigned int : 3;

// h006c, bit: 8
/* DLA block power status*/
#define offset_of_chiptop_reg_dla_power_off_rb (216)
#define mask_of_chiptop_reg_dla_power_off_rb   (0x100)
    unsigned int reg_dla_power_off_rb : 1;

    // h006c, bit: 14
    /* */
    unsigned int : 7;

    // h006c
    unsigned int /* padding 16 bit */ : 16;

// h006d, bit: 0
/* PM 48m clock source selection*/
#define offset_of_chiptop_reg_pm_48m_sel (218)
#define mask_of_chiptop_reg_pm_48m_sel   (0x1)
    unsigned int reg_pm_48m_sel : 1;

// h006d, bit: 1
/* PM 12m clock source selection*/
#define offset_of_chiptop_reg_pm_12m_sel (218)
#define mask_of_chiptop_reg_pm_12m_sel   (0x2)
    unsigned int reg_pm_12m_sel : 1;

    // h006d, bit: 14
    /* */
    unsigned int : 14;

    // h006d
    unsigned int /* padding 16 bit */ : 16;

    // h006e, bit: 14
    /* */
    unsigned int : 16;

    // h006e
    unsigned int /* padding 16 bit */ : 16;

    // h006f, bit: 14
    /* */
    unsigned int : 16;

    // h006f
    unsigned int /* padding 16 bit */ : 16;

// h0070, bit: 0
/* reg_clk_calc_clr
1:clr
0:release*/
#define offset_of_chiptop_reg_clk_calc_clr (224)
#define mask_of_chiptop_reg_clk_calc_clr   (0x1)
    unsigned int reg_clk_calc_clr : 1;

    // h0070, bit: 7
    /* */
    unsigned int : 7;

// h0070, bit: 11
/* ring OSC output select
0000: select delay chain 0
0001: select delay chain 1
0010: select delay chain 2
0011: select delay chain 3
0100: select delay chain 4
0101: select delay chain 5
0110: select delay chain 6
0111: select delay chain 7
0111: select delay chain 7*/
#define offset_of_chiptop_reg_rosc_out_sel (224)
#define mask_of_chiptop_reg_rosc_out_sel   (0xf00)
    unsigned int reg_rosc_out_sel : 4;

    // h0070, bit: 14
    /* */
    unsigned int : 3;

// h0070, bit: 15
/* reg_clk_calc_en
1:enable
0:disable*/
#define offset_of_chiptop_reg_clk_calc_en (224)
#define mask_of_chiptop_reg_clk_calc_en   (0x8000)
    unsigned int reg_clk_calc_en : 1;

    // h0070
    unsigned int /* padding 16 bit */ : 16;

// h0071, bit: 14
/* reg_calc_cnt_report*/
#define offset_of_chiptop_reg_calc_cnt_report (226)
#define mask_of_chiptop_reg_calc_cnt_report   (0xffff)
    unsigned int reg_calc_cnt_report : 16;

    // h0071
    unsigned int /* padding 16 bit */ : 16;

    // h0072, bit: 14
    /* */
    unsigned int : 16;

    // h0072
    unsigned int /* padding 16 bit */ : 16;

// h0073, bit: 14
/* Reserved register*/
#define offset_of_chiptop_reg_reserved (230)
#define mask_of_chiptop_reg_reserved   (0xffff)
    unsigned int reg_reserved : 16;

    // h0073
    unsigned int /* padding 16 bit */ : 16;

// h0074, bit: 14
/* Reserved register*/
#define offset_of_chiptop_reg_reserved_1 (232)
#define mask_of_chiptop_reg_reserved_1   (0xffff)
    unsigned int reg_reserved_1 : 16;

    // h0074
    unsigned int /* padding 16 bit */ : 16;

// h0075, bit: 2
/* select TEST_CLK_OUT source
3'd0: TEST_CLK_OUT= TEST_BUS_GB[0]
3'd1: TEST_CLK_OUT= TEST_BUS_GB[1]
3'd2: TEST_CLK_OUT= TEST_BUS_GB[2]
3'd3: TEST_CLK_OUT= TEST_BUS_GB[3]
3'd4: TEST_CLK_OUT= TEST_BUS_GB[4]
3'd5: TEST_CLK_OUT= TEST_BUS_GB[5]
3'd6: TEST_CLK_OUT= TEST_BUS_GB[6]
3'd7: TEST_CLK_OUT= TEST_BUS_GB[7]*/
#define offset_of_chiptop_reg_clk_out_sel (234)
#define mask_of_chiptop_reg_clk_out_sel   (0x7)
    unsigned int reg_clk_out_sel : 3;

// h0075, bit: 3
/* swap MSB 12bits with LSB 12bits of test bus*/
#define offset_of_chiptop_reg_swaptest12bit (234)
#define mask_of_chiptop_reg_swaptest12bit   (0x8)
    unsigned int reg_swaptest12bit : 1;

    // h0075, bit: 4
    /* */
    unsigned int : 1;

// h0075, bit: 5
/* setting for the data arrangement on test bus*/
#define offset_of_chiptop_reg_test_rg (234)
#define mask_of_chiptop_reg_test_rg   (0x20)
    unsigned int reg_test_rg : 1;

// h0075, bit: 6
/* setting for the data arrangement on test bus*/
#define offset_of_chiptop_reg_test_gb (234)
#define mask_of_chiptop_reg_test_gb   (0x40)
    unsigned int reg_test_gb : 1;

// h0075, bit: 7
/* setting for the data arrangement on test bus*/
#define offset_of_chiptop_reg_test_rb (234)
#define mask_of_chiptop_reg_test_rb   (0x80)
    unsigned int reg_test_rb : 1;

// h0075, bit: 9
/* select CLK_TEST_OUT
2'd0:  select CLK_TEST_OUT[47:0]
2'd1:  select CLK_TEST_OUT[95:48]
2'd2:  select CLK_TEST_OUT[143:96]
2'd3:  reserved*/
#define offset_of_chiptop_reg_sel_clk_test_out (234)
#define mask_of_chiptop_reg_sel_clk_test_out   (0x300)
    unsigned int reg_sel_clk_test_out : 2;

// h0075, bit: 10
/* set ana delay ring*/
#define offset_of_chiptop_reg_ana_rosc_in_sel (234)
#define mask_of_chiptop_reg_ana_rosc_in_sel   (0x400)
    unsigned int reg_ana_rosc_in_sel : 1;

    // h0075, bit: 12
    /* */
    unsigned int : 2;

// h0075, bit: 13
/* testCLK_mode used in TEST_CTRL*/
#define offset_of_chiptop_reg_testclk_mode (234)
#define mask_of_chiptop_reg_testclk_mode   (0x2000)
    unsigned int reg_testclk_mode : 1;

// h0075, bit: 14
/* Enable test bus output (disable in default)*/
#define offset_of_chiptop_reg_testbus_en (234)
#define mask_of_chiptop_reg_testbus_en   (0x4000)
    unsigned int reg_testbus_en : 1;

// h0075, bit: 15
/* Select the input source of ring oscillator in CHIP_CONF
1: close-loop (enable ring oscillator)
0: open-loop (input from external digital input)*/
#define offset_of_chiptop_reg_rosc_in_sel (234)
#define mask_of_chiptop_reg_rosc_in_sel   (0x8000)
    unsigned int reg_rosc_in_sel : 1;

    // h0075
    unsigned int /* padding 16 bit */ : 16;

// h0076, bit: 2
/* select single CLK_OUT
3'd1: TEST_BUS[11] = TEST_CLK_OUT_d2
        TEST_BUS[10] = TEST_CLK_OUT
3'd2: TEST_BUS[11] = TEST_CLK_OUT_d2
        TEST_BUS[10] = TEST_CLK_OUT_d4
3'd3: TEST_BUS[11] = TEST_CLK_OUT_d2.
        TEST_BUS[10] = TEST_CLK_OUT_d8
3'd4: TEST_BUS[11] = TEST_CLK_OUT_d2
        TEST_BUS[10] = TEST_CLK_OUT_d16
3'd5: TEST_BUS[11] = TEST_CLK_OUT_d2
        TEST_BUS[10] = TEST_CLK_OUT_d32
3'd6: TEST_BUS[11] = TEST_CLK_OUT_d2
        TEST_BUS[10] = TEST_CLK_OUT_d64
Others: no TEST_CLK_OUT*/
#define offset_of_chiptop_reg_single_clk_out_sel (236)
#define mask_of_chiptop_reg_single_clk_out_sel   (0x7)
    unsigned int reg_single_clk_out_sel : 3;

    // h0076, bit: 14
    /* */
    unsigned int : 13;

    // h0076
    unsigned int /* padding 16 bit */ : 16;

// h0077, bit: 5
/* select TEST_BUS[23:0] source
6'd0: TEST_BUS = NET_GP_TEST_OUT
6'd1: TEST_BUS = UTMI_TESTBUS_P0
6'd2: TEST_BUS = ANA_MISC_TEST_OUT
6'd3: TEST_BUS = ANA_MISC_TEST_OUT1
6'd4: TEST_BUS = DIAMOND_TOP_WP_TEST_OUT - CA55
6'd5: TEST_BUS = SD_PLL_DEB_BUS
6'd6: TEST_BUS = UTMI_TESTBUS_P1
6'd7: TEST_BUS = ISP0_GP_TEST_OUT
6'd8: TEST_BUS = AUSDM_TEST_OUT
6'd9: TEST_BUS = DIG_PM_TEST_OUT
6'd10: TEST_BUS = MCU_IF_TEST_OUT
6'd11: TEST_BUS = DEBUG_OUT_VEN1
6'd12: TEST_BUS = DEBUG_OUT_VEN2
6'd13: TEST_BUS = DEBUG_OUT_VEN3
6'd14: TEST_BUS = DSP_TEST_OUT
6'd15: TEST_BUS = DISP_GP_TEST_OUT
6'd16: TEST_BUS = SC_GP_TEST_OUT
6'd17: TEST_BUS = DEBUG_VEN0_TEST_OUT
6'd18: TEST_BUS = DLA_GP_TEST_OUT
6'd19: TEST_BUS = DAC_ATOP_TEST_OUT
6'd20: TEST_BUS = HDMI_TX_ATOP_TESTBUS
6'd21: TEST_BUS = TEST_OUT_CSI_PHY_0
6'd22: TEST_BUS = TEST_OUT_CSI_MAC_0
6'd23: TEST_BUS = TEST_OUT_CSI_PHY_1
6'd24: TEST_BUS = TEST_OUT_CSI_MAC_1
6'd25: TEST_BUS = CLKGEN_TEST_OUT
6'd26: TEST_BUS = CLKGEN2_TEST_OUT
6'd27: TEST_BUS = USB30_DRD_GP_TEST_OUT
6'd28: TEST_BUS = MIPI_TX_DSI_DPHY_DEBUG_OUT
6'd29: TEST_BUS = MIPI_TX_CSI_DPHY_DEBUG_OUT
6'd30: TEST_BUS = OTP_DEBUG
6'd31: TEST_BUS = ISP1_GP_TEST_OUT
6'd32: TEST_BUS = MIU0_GP_TEST_OUT
6'd33: TEST_BUS = MIU1_GP_TEST_OUT
6'd34: TEST_BUS = PCIE0_GP_TEST_OUT
6'd35: TEST_BUS = PCIE1_GP_TEST_OUT
6'd36: TEST_BUS = SATA0_GP_TEST_OUT
6'd37: TEST_BUS = SATA1_GP_TEST_OUT
6'd38: TEST_BUS = HEMCU2_DEBUG
6'd39: TEST_BUS = CMDQ_VEN
6'd40: TEST_BUS = MIU_ARB
6'd41: TEST_BUS = M2A_DIG_WP
6'd42: TEST_BUS = TEST_GARB0_OUT
6'd43: TEST_BUS = TEST_GARB1_OUT
Others: no TEST_OUT*/
#define offset_of_chiptop_reg_test_bus24b_sel (238)
#define mask_of_chiptop_reg_test_bus24b_sel   (0x3f)
    unsigned int reg_test_bus24b_sel : 6;

    // h0077, bit: 14
    /* */
    unsigned int : 10;

    // h0077
    unsigned int /* padding 16 bit */ : 16;

// h0078, bit: 0
/* control diamond/ipu sram to NOD setting*/
#define offset_of_chiptop_reg_sram_tc_sel_hv (240)
#define mask_of_chiptop_reg_sram_tc_sel_hv   (0x1)
    unsigned int reg_sram_tc_sel_hv : 1;

// h0078, bit: 1
/* control others IP sram to NOD setting*/
#define offset_of_chiptop_reg_sram_tc_sel_lv (240)
#define mask_of_chiptop_reg_sram_tc_sel_lv   (0x2)
    unsigned int reg_sram_tc_sel_lv : 1;

// h0078, bit: 2
/* free run xiu gating*/
#define offset_of_chiptop_reg_clk_mcu_brg_free_run (240)
#define mask_of_chiptop_reg_clk_mcu_brg_free_run   (0x4)
    unsigned int reg_clk_mcu_brg_free_run : 1;

    // h0078, bit: 7
    /* */
    unsigned int : 5;

// h0078, bit: 8
/* control non-pm ld/nod setting*/
#define offset_of_chiptop_reg_sram_tc_sel_ld_non_pm (240)
#define mask_of_chiptop_reg_sram_tc_sel_ld_non_pm   (0x100)
    unsigned int reg_sram_tc_sel_ld_non_pm : 1;

// h0078, bit: 9
/* control pm ld /uld setting*/
#define offset_of_chiptop_reg_sram_tc_sel_ld_pm (240)
#define mask_of_chiptop_reg_sram_tc_sel_ld_pm   (0x200)
    unsigned int reg_sram_tc_sel_ld_pm : 1;

    // h0078, bit: 14
    /* */
    unsigned int : 6;

    // h0078
    unsigned int /* padding 16 bit */ : 16;

// h0079, bit: 10
/* debug rm glitch cnt*/
#define offset_of_chiptop_reg_debug_glhrm_num (242)
#define mask_of_chiptop_reg_debug_glhrm_num   (0x7ff)
    unsigned int reg_debug_glhrm_num : 11;

    // h0079, bit: 13
    /* */
    unsigned int : 3;

// h0079, bit: 14
/* debug rm glitch en*/
#define offset_of_chiptop_reg_debug_glhrm_en (242)
#define mask_of_chiptop_reg_debug_glhrm_en   (0xc000)
    unsigned int reg_debug_glhrm_en : 2;

    // h0079
    unsigned int /* padding 16 bit */ : 16;

    // h007a, bit: 14
    /* */
    unsigned int : 16;

    // h007a
    unsigned int /* padding 16 bit */ : 16;

// h007b, bit: 14
/* */
#define offset_of_chiptop_reg_chiptop_reserved (246)
#define mask_of_chiptop_reg_chiptop_reserved   (0xffff)
    unsigned int reg_chiptop_reserved : 16;

    // h007b
    unsigned int /* padding 16 bit */ : 16;

// h007c, bit: 14
/* */
#define offset_of_chiptop_reg_chk_clk_hemcu_freq_cmp_data (248)
#define mask_of_chiptop_reg_chk_clk_hemcu_freq_cmp_data   (0xffff)
    unsigned int reg_chk_clk_hemcu_freq_cmp_data : 16;

    // h007c
    unsigned int /* padding 16 bit */ : 16;

// h007d, bit: 3
/* 256bus miu 2x div enable*/
#define offset_of_chiptop_reg_256bus_2x_div_en (250)
#define mask_of_chiptop_reg_256bus_2x_div_en   (0xf)
    unsigned int reg_256bus_2x_div_en : 4;

    // h007d, bit: 14
    /* */
    unsigned int : 12;

    // h007d
    unsigned int /* padding 16 bit */ : 16;

// h007e, bit: 3
/* clk_miu2x_div sw rstz
[0]: MIU0
[3:1]: Reserved*/
#define offset_of_chiptop_reg_miu2x_div_rstz (252)
#define mask_of_chiptop_reg_miu2x_div_rstz   (0xf)
    unsigned int reg_miu2x_div_rstz : 4;

    // h007e, bit: 14
    /* */
    unsigned int : 12;

    // h007e
    unsigned int /* padding 16 bit */ : 16;

// h007f, bit: 0
/* mspi1- csz2 invert function; reserved usage*/
#define offset_of_chiptop_reg_mspi1_scz1_i_inver_en (254)
#define mask_of_chiptop_reg_mspi1_scz1_i_inver_en   (0x1)
    unsigned int reg_mspi1_scz1_i_inver_en : 1;

    // h007f, bit: 14
    /* */
    unsigned int : 15;

    // h007f
    unsigned int /* padding 16 bit */ : 16;

} __attribute__((packed, aligned(1))) reg_chiptop;
#endif
