/*
 * reg_isp_vif.h - Sigmastar
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

#ifndef __REG_ISP_VIF__
#define __REG_ISP_VIF__
typedef struct
{
// h0000, bit: 0
/* Software Reset for Sensor0
# Low Active
 ( single buffer register )*/
#define offset_of_vif_reg_vif_ch0_sensor_sw_rstz (0)
#define mask_of_vif_reg_vif_ch0_sensor_sw_rstz   (0x1)
    unsigned int reg_vif_ch0_sensor_sw_rstz : 1;

// h0000, bit: 1
/* VIF IF status reset*/
#define offset_of_vif_reg_vif_ch0_if_state_rst (0)
#define mask_of_vif_reg_vif_ch0_if_state_rst   (0x2)
    unsigned int reg_vif_ch0_if_state_rst : 1;

// h0000, bit: 2
/* Reset Sensor0*/
#define offset_of_vif_reg_vif_ch0_sensor_rst (0)
#define mask_of_vif_reg_vif_ch0_sensor_rst   (0x4)
    unsigned int reg_vif_ch0_sensor_rst : 1;

// h0000, bit: 3
/* Power down sensor0*/
#define offset_of_vif_reg_vif_ch0_sensor_pwrdn (0)
#define mask_of_vif_reg_vif_ch0_sensor_pwrdn   (0x8)
    unsigned int reg_vif_ch0_sensor_pwrdn : 1;

// h0000, bit: 4
/* Line interleaved mode enable*/
#define offset_of_vif_reg_vif_ch0_li_mode_en (0)
#define mask_of_vif_reg_vif_ch0_li_mode_en   (0x10)
    unsigned int reg_vif_ch0_li_mode_en : 1;

// h0000, bit: 5
/* Line interleaved channel*/
#define offset_of_vif_reg_vif_ch0_li_ch (0)
#define mask_of_vif_reg_vif_ch0_li_ch   (0x20)
    unsigned int reg_vif_ch0_li_ch : 1;

    // h0000, bit: 7
    /* */
    unsigned int : 2;

// h0000, bit: 12
/* VIF source select
# 5'd0 MIPI
# 5'd3 Paraller
# 5'd4 new bt656/bt1120 (itu)
# 5'd31 pat gen*/
#define offset_of_vif_reg_vif_ch0_src_sel (0)
#define mask_of_vif_reg_vif_ch0_src_sel   (0x1f00)
    unsigned int reg_vif_ch0_src_sel : 5;

    // h0000, bit: 14
    /* */
    unsigned int : 2;

// h0000, bit: 15
/* VIF channel 0 enable*/
#define offset_of_vif_reg_vif_ch0_en (0)
#define mask_of_vif_reg_vif_ch0_en   (0x8000)
    unsigned int reg_vif_ch0_en : 1;

    // h0000
    unsigned int /* padding 16 bit */ : 16;

    // h0001, bit: 14
    /* */
    unsigned int : 16;

    // h0001
    unsigned int /* padding 16 bit */ : 16;

    // h0002, bit: 14
    /* */
    unsigned int : 16;

    // h0002
    unsigned int /* padding 16 bit */ : 16;

    // h0003, bit: 14
    /* */
    unsigned int : 16;

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

    // h0006, bit: 6
    /* */
    unsigned int : 7;

// h0006, bit: 7
/* swap sensor input data*/
#define offset_of_vif_reg_vif_ch0_sensor_bit_swap (12)
#define mask_of_vif_reg_vif_ch0_sensor_bit_swap   (0x80)
    unsigned int reg_vif_ch0_sensor_bit_swap : 1;

// h0006, bit: 8
/* Sensor Hsync Polarity
# 1'b0: high active
# 1'b1: low active
 (reserved do not use, use hsync invert first)*/
#define offset_of_vif_reg_vif_ch0_sensor_hsync_polarity (12)
#define mask_of_vif_reg_vif_ch0_sensor_hsync_polarity   (0x100)
    unsigned int reg_vif_ch0_sensor_hsync_polarity : 1;

// h0006, bit: 9
/* Sensor Vsync Polarity
# 1'b0: high active
# 1'b1: low active
 (reserved do not use, use vsync invert first)*/
#define offset_of_vif_reg_vif_ch0_sensor_vsync_polarity (12)
#define mask_of_vif_reg_vif_ch0_sensor_vsync_polarity   (0x200)
    unsigned int reg_vif_ch0_sensor_vsync_polarity : 1;

    // h0006, bit: 11
    /* */
    unsigned int : 2;

// h0006, bit: 12
/* Sensor Input Format
# 1'b0: YUV 422 format
# 1'b1: RGB pattern
 ( single buffer register )*/
#define offset_of_vif_reg_vif_ch0_sensor_rgb_in (12)
#define mask_of_vif_reg_vif_ch0_sensor_rgb_in   (0x1000)
    unsigned int reg_vif_ch0_sensor_rgb_in : 1;

    // h0006, bit: 14
    /* */
    unsigned int : 3;

    // h0006
    unsigned int /* padding 16 bit */ : 16;

// h0007, bit: 0
/* Sensor Vsync inverce
# 1'b0: High level Vsync input
# 1'b1: Low level Vsync input
 ( single buffer register )*/
#define offset_of_vif_reg_vif_ch0_vsync_invert (14)
#define mask_of_vif_reg_vif_ch0_vsync_invert   (0x1)
    unsigned int reg_vif_ch0_vsync_invert : 1;

// h0007, bit: 1
/* Sensor Hsync inverce
# 1'b0: High level Hsync input
# 1'b1: Low level Hsync input
 ( single buffer register )*/
#define offset_of_vif_reg_vif_ch0_hsync_invert (14)
#define mask_of_vif_reg_vif_ch0_hsync_invert   (0x2)
    unsigned int reg_vif_ch0_hsync_invert : 1;

    // h0007, bit: 5
    /* */
    unsigned int : 4;

// h0007, bit: 6
/* Sensor Input Format
# 1'b0: separate Y/C mode
# 1'b1: YC 16bit mode*/
#define offset_of_vif_reg_vif_ch0_sensor_yc16bit (14)
#define mask_of_vif_reg_vif_ch0_sensor_yc16bit   (0x40)
    unsigned int reg_vif_ch0_sensor_yc16bit : 1;

    // h0007, bit: 7
    /* */
    unsigned int : 1;

// h0007, bit: 9
/* Sensor vsync pulse delay
# 2'd0: vsync falling edge
# 2'd1: vsync rising edge delay 2 line
# 2'd2: vsync rising edge delay 1 line
# 2'd3: vsync rising edge*/
#define offset_of_vif_reg_vif_ch0_sensor_vs_dly (14)
#define mask_of_vif_reg_vif_ch0_sensor_vs_dly   (0x300)
    unsigned int reg_vif_ch0_sensor_vs_dly : 2;

    // h0007, bit: 11
    /* */
    unsigned int : 2;

// h0007, bit: 12
/* Sensor hsync pulse delay
# 1'b0: hsync rising edge
# 1'b1: hsync falling edge*/
#define offset_of_vif_reg_vif_ch0_sensor_hs_dly (14)
#define mask_of_vif_reg_vif_ch0_sensor_hs_dly   (0x1000)
    unsigned int reg_vif_ch0_sensor_hs_dly : 1;

    // h0007, bit: 14
    /* */
    unsigned int : 3;

    // h0007
    unsigned int /* padding 16 bit */ : 16;

// h0008, bit: 0
/* Mask sensor/csi ready to ISP_IF*/
#define offset_of_vif_reg_vif_ch0_sensor_mask_vc0 (16)
#define mask_of_vif_reg_vif_ch0_sensor_mask_vc0   (0x1)
    unsigned int reg_vif_ch0_sensor_mask_vc0 : 1;

    // h0008, bit: 7
    /* */
    unsigned int : 7;

// h0008, bit: 10
/* Mask VIF data output select
# 3'b000: sensor mask reference vc0
# 3'b001: sensor mask reference vc1
# 3'b010: sensor mask reference both vc0/vc1
# 3'b011: sensor mask refenerce vc2
# 3'b100: sensor mask reference vc0/vc2*/
#define offset_of_vif_reg_vif_ch0_sensor_mask_sel_vc0 (16)
#define mask_of_vif_reg_vif_ch0_sensor_mask_sel_vc0   (0x700)
    unsigned int reg_vif_ch0_sensor_mask_sel_vc0 : 3;

    // h0008, bit: 14
    /* */
    unsigned int : 4;

// h0008, bit: 15
/* Mask sensor/csi ready to ISP_IF*/
#define offset_of_vif_reg_vif_ch0_sensor_mask_status_vc0 (16)
#define mask_of_vif_reg_vif_ch0_sensor_mask_status_vc0   (0x8000)
    unsigned int reg_vif_ch0_sensor_mask_status_vc0 : 1;

    // h0008
    unsigned int /* padding 16 bit */ : 16;

// h0009, bit: 0
/* Mask sensor/csi ready to ISP_IF*/
#define offset_of_vif_reg_vif_ch0_sensor_mask_vc1 (18)
#define mask_of_vif_reg_vif_ch0_sensor_mask_vc1   (0x1)
    unsigned int reg_vif_ch0_sensor_mask_vc1 : 1;

    // h0009, bit: 7
    /* */
    unsigned int : 7;

// h0009, bit: 10
/* Mask VIF data output select
# 3'b000: sensor mask reference vc1
# 3'b001: sensor mask reference vc0
# 3'b010: sensor mask reference both vc1/vc0
# 3'b011: sensor mask refenerce vc2
# 3'b100: sensor mask reference vc1/vc2*/
#define offset_of_vif_reg_vif_ch0_sensor_mask_sel_vc1 (18)
#define mask_of_vif_reg_vif_ch0_sensor_mask_sel_vc1   (0x700)
    unsigned int reg_vif_ch0_sensor_mask_sel_vc1 : 3;

    // h0009, bit: 14
    /* */
    unsigned int : 4;

// h0009, bit: 15
/* Mask sensor/csi ready to ISP_IF*/
#define offset_of_vif_reg_vif_ch0_sensor_mask_status_vc1 (18)
#define mask_of_vif_reg_vif_ch0_sensor_mask_status_vc1   (0x8000)
    unsigned int reg_vif_ch0_sensor_mask_status_vc1 : 1;

    // h0009
    unsigned int /* padding 16 bit */ : 16;

// h000a, bit: 0
/* Mask sensor/csi ready to ISP_IF*/
#define offset_of_vif_reg_vif_ch0_sensor_mask_vc2 (20)
#define mask_of_vif_reg_vif_ch0_sensor_mask_vc2   (0x1)
    unsigned int reg_vif_ch0_sensor_mask_vc2 : 1;

    // h000a, bit: 7
    /* */
    unsigned int : 7;

// h000a, bit: 10
/* Mask VIF data output select
# 3'b000: sensor mask reference vc2
# 3'b001: sensor mask reference vc0
# 3'b010: sensor mask reference both vc2/vc0
# 3'b011: sensor mask refenerce vc1
# 3'b100: sensor mask reference vc2/vc1*/
#define offset_of_vif_reg_vif_ch0_sensor_mask_sel_vc2 (20)
#define mask_of_vif_reg_vif_ch0_sensor_mask_sel_vc2   (0x700)
    unsigned int reg_vif_ch0_sensor_mask_sel_vc2 : 3;

    // h000a, bit: 14
    /* */
    unsigned int : 4;

// h000a, bit: 15
/* Mask sensor/csi ready to ISP_IF*/
#define offset_of_vif_reg_vif_ch0_sensor_mask_status_vc2 (20)
#define mask_of_vif_reg_vif_ch0_sensor_mask_status_vc2   (0x8000)
    unsigned int reg_vif_ch0_sensor_mask_status_vc2 : 1;

    // h000a
    unsigned int /* padding 16 bit */ : 16;

// h000b, bit: 0
/* Mask sensor/csi ready to ISP_IF*/
#define offset_of_vif_reg_vif_ch0_sensor_mask_vc3 (22)
#define mask_of_vif_reg_vif_ch0_sensor_mask_vc3   (0x1)
    unsigned int reg_vif_ch0_sensor_mask_vc3 : 1;

    // h000b, bit: 14
    /* */
    unsigned int : 15;

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

    // h0017, bit: 14
    /* */
    unsigned int : 16;

    // h0017
    unsigned int /* padding 16 bit */ : 16;

// h0018, bit: 3
/* reg_vif_ch0_mask_no_frame_sync for each vc*/
#define offset_of_vif_reg_vif_ch0_mask_no_frame_sync (48)
#define mask_of_vif_reg_vif_ch0_mask_no_frame_sync   (0xf)
    unsigned int reg_vif_ch0_mask_no_frame_sync : 4;

    // h0018, bit: 14
    /* */
    unsigned int : 12;

    // h0018
    unsigned int /* padding 16 bit */ : 16;

// h0019, bit: 3
/* max/min pixel detect clr by vs*/
#define offset_of_vif_reg_vif_ch0_vs_clr_cnt (50)
#define mask_of_vif_reg_vif_ch0_vs_clr_cnt   (0xf)
    unsigned int reg_vif_ch0_vs_clr_cnt : 4;

// h0019, bit: 7
/* max/min pixel detect clr by sw*/
#define offset_of_vif_reg_vif_ch0_sw_clr_cnt (50)
#define mask_of_vif_reg_vif_ch0_sw_clr_cnt   (0xf0)
    unsigned int reg_vif_ch0_sw_clr_cnt : 4;

// h0019, bit: 11
/* total line cnt add over line cnt, for line cnt over irq*/
#define offset_of_vif_reg_vif_ch0_line_over (50)
#define mask_of_vif_reg_vif_ch0_line_over   (0xf00)
    unsigned int reg_vif_ch0_line_over : 4;

    // h0019, bit: 14
    /* */
    unsigned int : 4;

    // h0019
    unsigned int /* padding 16 bit */ : 16;

// h001a, bit: 12
/* max/min pixel detect from which line*/
#define offset_of_vif_reg_vif_ch0_detect_line_st (52)
#define mask_of_vif_reg_vif_ch0_detect_line_st   (0x1fff)
    unsigned int reg_vif_ch0_detect_line_st : 13;

    // h001a, bit: 14
    /* */
    unsigned int : 3;

    // h001a
    unsigned int /* padding 16 bit */ : 16;

// h001b, bit: 12
/* max/min pixel detect to which line*/
#define offset_of_vif_reg_vif_ch0_detect_line_end (54)
#define mask_of_vif_reg_vif_ch0_detect_line_end   (0x1fff)
    unsigned int reg_vif_ch0_detect_line_end : 13;

    // h001b, bit: 14
    /* */
    unsigned int : 3;

    // h001b
    unsigned int /* padding 16 bit */ : 16;

// h001c, bit: 12
/* VIF vc0 input line cnt*/
#define offset_of_vif_reg_vif_ch0_vc0_input_line_cnt (56)
#define mask_of_vif_reg_vif_ch0_vc0_input_line_cnt   (0x1fff)
    unsigned int reg_vif_ch0_vc0_input_line_cnt : 13;

    // h001c, bit: 14
    /* */
    unsigned int : 3;

    // h001c
    unsigned int /* padding 16 bit */ : 16;

// h001d, bit: 12
/* VIF vc1 input line cnt*/
#define offset_of_vif_reg_vif_ch0_vc1_input_line_cnt (58)
#define mask_of_vif_reg_vif_ch0_vc1_input_line_cnt   (0x1fff)
    unsigned int reg_vif_ch0_vc1_input_line_cnt : 13;

    // h001d, bit: 14
    /* */
    unsigned int : 3;

    // h001d
    unsigned int /* padding 16 bit */ : 16;

// h001e, bit: 12
/* VIF vc2 input line cnt*/
#define offset_of_vif_reg_vif_ch0_vc2_input_line_cnt (60)
#define mask_of_vif_reg_vif_ch0_vc2_input_line_cnt   (0x1fff)
    unsigned int reg_vif_ch0_vc2_input_line_cnt : 13;

    // h001e, bit: 14
    /* */
    unsigned int : 3;

    // h001e
    unsigned int /* padding 16 bit */ : 16;

// h001f, bit: 12
/* VIF vc3 input line cnt*/
#define offset_of_vif_reg_vif_ch0_vc3_input_line_cnt (62)
#define mask_of_vif_reg_vif_ch0_vc3_input_line_cnt   (0x1fff)
    unsigned int reg_vif_ch0_vc3_input_line_cnt : 13;

    // h001f, bit: 14
    /* */
    unsigned int : 3;

    // h001f
    unsigned int /* padding 16 bit */ : 16;

// h0020, bit: 0
/* timing generator enable*/
#define offset_of_vif_reg_vif_ch0_pat_tgen_en (64)
#define mask_of_vif_reg_vif_ch0_pat_tgen_en   (0x1)
    unsigned int reg_vif_ch0_pat_tgen_en : 1;

// h0020, bit: 1
/* data generator enable*/
#define offset_of_vif_reg_vif_ch0_pat_dgen_en (64)
#define mask_of_vif_reg_vif_ch0_pat_dgen_en   (0x2)
    unsigned int reg_vif_ch0_pat_dgen_en : 1;

// h0020, bit: 2
/* hsync generator*/
#define offset_of_vif_reg_vif_ch0_pat_hsgen_en (64)
#define mask_of_vif_reg_vif_ch0_pat_hsgen_en   (0x4)
    unsigned int reg_vif_ch0_pat_hsgen_en : 1;

// h0020, bit: 3
/* data generator  reset*/
#define offset_of_vif_reg_vif_ch0_pat_dgen_rst (64)
#define mask_of_vif_reg_vif_ch0_pat_dgen_rst   (0x8)
    unsigned int reg_vif_ch0_pat_dgen_rst : 1;

// h0020, bit: 5
/* bayer format*/
#define offset_of_vif_reg_vif_ch0_pat_sensor_array (64)
#define mask_of_vif_reg_vif_ch0_pat_sensor_array   (0x30)
    unsigned int reg_vif_ch0_pat_sensor_array : 2;

    // h0020, bit: 7
    /* */
    unsigned int : 2;

// h0020, bit: 10
/* data enable valid rate control*/
#define offset_of_vif_reg_vif_ch0_pat_de_rate (64)
#define mask_of_vif_reg_vif_ch0_pat_de_rate   (0x700)
    unsigned int reg_vif_ch0_pat_de_rate : 3;

    // h0020, bit: 11
    /* */
    unsigned int : 1;

// h0020, bit: 12
/* timing generator field mode, 0: off, 1: on(for itu, itu is bt656)*/
#define offset_of_vif_reg_vif_ch0_pat_fd_en (64)
#define mask_of_vif_reg_vif_ch0_pat_fd_en   (0x1000)
    unsigned int reg_vif_ch0_pat_fd_en : 1;

    // h0020, bit: 13
    /* */
    unsigned int : 1;

// h0020, bit: 14
/* data generator 4 pixel mode for MIPI, 0: 1 pixel, 1: 4 plxels*/
#define offset_of_vif_reg_vif_ch0_pat_dgen_4p_mode (64)
#define mask_of_vif_reg_vif_ch0_pat_dgen_4p_mode   (0x4000)
    unsigned int reg_vif_ch0_pat_dgen_4p_mode : 1;

// h0020, bit: 15
/* timing generator clear. 0: off, 1: clear*/
#define offset_of_vif_reg_vif_ch0_pat_tgen_clr (64)
#define mask_of_vif_reg_vif_ch0_pat_tgen_clr   (0x8000)
    unsigned int reg_vif_ch0_pat_tgen_clr : 1;

    // h0020
    unsigned int /* padding 16 bit */ : 16;

// h0021, bit: 12
/* image width minus one*/
#define offset_of_vif_reg_vif_ch0_pat_img_width_m1 (66)
#define mask_of_vif_reg_vif_ch0_pat_img_width_m1   (0x1fff)
    unsigned int reg_vif_ch0_pat_img_width_m1 : 13;

    // h0021, bit: 14
    /* */
    unsigned int : 3;

    // h0021
    unsigned int /* padding 16 bit */ : 16;

// h0022, bit: 12
/* image height minus one*/
#define offset_of_vif_reg_vif_ch0_pat_img_height_m1 (68)
#define mask_of_vif_reg_vif_ch0_pat_img_height_m1   (0x1fff)
    unsigned int reg_vif_ch0_pat_img_height_m1 : 13;

    // h0022, bit: 14
    /* */
    unsigned int : 3;

    // h0022
    unsigned int /* padding 16 bit */ : 16;

// h0023, bit: 7
/* vsync pulse position, line number before v-active*/
#define offset_of_vif_reg_vif_ch0_pat_vs_line (70)
#define mask_of_vif_reg_vif_ch0_pat_vs_line   (0xff)
    unsigned int reg_vif_ch0_pat_vs_line : 8;

// h0023, bit: 14
/* hsync pulse position, pixel number before v-active*/
#define offset_of_vif_reg_vif_ch0_pat_hs_pxl (70)
#define mask_of_vif_reg_vif_ch0_pat_hs_pxl   (0xff00)
    unsigned int reg_vif_ch0_pat_hs_pxl : 8;

    // h0023
    unsigned int /* padding 16 bit */ : 16;

// h0024, bit: 7
/* H-blanking time (x2)*/
#define offset_of_vif_reg_vif_ch0_pat_hblank (72)
#define mask_of_vif_reg_vif_ch0_pat_hblank   (0xff)
    unsigned int reg_vif_ch0_pat_hblank : 8;

// h0024, bit: 14
/* V-blanking time*/
#define offset_of_vif_reg_vif_ch0_pat_vblank (72)
#define mask_of_vif_reg_vif_ch0_pat_vblank   (0xff00)
    unsigned int reg_vif_ch0_pat_vblank : 8;

    // h0024
    unsigned int /* padding 16 bit */ : 16;

// h0025, bit: 12
/* pattern block width minus one*/
#define offset_of_vif_reg_vif_ch0_pat_blk_width_m1 (74)
#define mask_of_vif_reg_vif_ch0_pat_blk_width_m1   (0x1fff)
    unsigned int reg_vif_ch0_pat_blk_width_m1 : 13;

    // h0025, bit: 14
    /* */
    unsigned int : 3;

    // h0025
    unsigned int /* padding 16 bit */ : 16;

// h0026, bit: 12
/* pattern block height minus one*/
#define offset_of_vif_reg_vif_ch0_pat_blk_height_m1 (76)
#define mask_of_vif_reg_vif_ch0_pat_blk_height_m1   (0x1fff)
    unsigned int reg_vif_ch0_pat_blk_height_m1 : 13;

    // h0026, bit: 14
    /* */
    unsigned int : 3;

    // h0026
    unsigned int /* padding 16 bit */ : 16;

// h0027, bit: 2
/* initial color bar index*/
#define offset_of_vif_reg_vif_ch0_pat_color_init_idx (78)
#define mask_of_vif_reg_vif_ch0_pat_color_init_idx   (0x7)
    unsigned int reg_vif_ch0_pat_color_init_idx : 3;

    // h0027, bit: 3
    /* */
    unsigned int : 1;

// h0027, bit: 5
/* color bar value percentage*/
#define offset_of_vif_reg_vif_ch0_pat_color_percent (78)
#define mask_of_vif_reg_vif_ch0_pat_color_percent   (0x30)
    unsigned int reg_vif_ch0_pat_color_percent : 2;

    // h0027, bit: 14
    /* */
    unsigned int : 10;

    // h0027
    unsigned int /* padding 16 bit */ : 16;

// h0028, bit: 7
/* frame pattern change rate*/
#define offset_of_vif_reg_vif_ch0_pat_frm_chg_rate (80)
#define mask_of_vif_reg_vif_ch0_pat_frm_chg_rate   (0xff)
    unsigned int reg_vif_ch0_pat_frm_chg_rate : 8;

// h0028, bit: 11
/* pattern shift pixel number when frame change*/
#define offset_of_vif_reg_vif_ch0_pat_frm_chg_x (80)
#define mask_of_vif_reg_vif_ch0_pat_frm_chg_x   (0xf00)
    unsigned int reg_vif_ch0_pat_frm_chg_x : 4;

// h0028, bit: 14
/* pattern shift line number when frame change*/
#define offset_of_vif_reg_vif_ch0_pat_frm_chg_y (80)
#define mask_of_vif_reg_vif_ch0_pat_frm_chg_y   (0xf000)
    unsigned int reg_vif_ch0_pat_frm_chg_y : 4;

    // h0028
    unsigned int /* padding 16 bit */ : 16;

// h0029, bit: 12
/* input HSYNC count*/
#define offset_of_vif_reg_vif_ch0_input_hs_cnt (82)
#define mask_of_vif_reg_vif_ch0_input_hs_cnt   (0x1fff)
    unsigned int reg_vif_ch0_input_hs_cnt : 13;

    // h0029, bit: 14
    /* */
    unsigned int : 3;

    // h0029
    unsigned int /* padding 16 bit */ : 16;

// h002a, bit: 12
/* input DE count*/
#define offset_of_vif_reg_vif_ch0_input_de_cnt (84)
#define mask_of_vif_reg_vif_ch0_input_de_cnt   (0x1fff)
    unsigned int reg_vif_ch0_input_de_cnt : 13;

    // h002a, bit: 14
    /* */
    unsigned int : 3;

    // h002a
    unsigned int /* padding 16 bit */ : 16;

// h002b, bit: 0
/* test pattern yc swap*/
#define offset_of_vif_reg_vif_ch0_test_yc_swap (86)
#define mask_of_vif_reg_vif_ch0_test_yc_swap   (0x1)
    unsigned int reg_vif_ch0_test_yc_swap : 1;

    // h002b, bit: 14
    /* */
    unsigned int : 15;

    // h002b
    unsigned int /* padding 16 bit */ : 16;

// h002c, bit: 1
/* test pattern vc */
#define offset_of_vif_reg_vif_ch0_pat_vc (88)
#define mask_of_vif_reg_vif_ch0_pat_vc   (0x3)
    unsigned int reg_vif_ch0_pat_vc : 2;

    // h002c, bit: 14
    /* */
    unsigned int : 14;

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
    /* */
    unsigned int : 16;

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

    // h0034, bit: 2
    /* */
    unsigned int : 3;

// h0034, bit: 14
/* dummy register*/
#define offset_of_vif_reg_vif_ch0_dummy (104)
#define mask_of_vif_reg_vif_ch0_dummy   (0x7ff8)
    unsigned int reg_vif_ch0_dummy : 12;

    // h0034, bit: 15
    /* */
    unsigned int : 1;

    // h0034
    unsigned int /* padding 16 bit */ : 16;

// h0035, bit: 9
/* BT656 input data read (for debug)*/
#define offset_of_vif_reg_vif_ch0_bt656_in (106)
#define mask_of_vif_reg_vif_ch0_bt656_in   (0x3ff)
    unsigned int reg_vif_ch0_bt656_in : 10;

    // h0035, bit: 14
    /* */
    unsigned int : 6;

    // h0035
    unsigned int /* padding 16 bit */ : 16;

// h0036, bit: 14
/* tdm debug
[31:24]: BT3 XY
[23:16]: BT2 XY
[15:8]: BT1 XY
[7:0]: BT0 XY
XY[7]: error protect, 1 is error, 0 is OK
XY[6]: F
XY[5]: V
XY[4]: H
XY[3:0]: ch_num_embed = 1, channel num or polarity
XY[3:0]: ch_num_embed = 0, fix, BT0 = 0, … BT3 = 3*/
#define offset_of_vif_reg_vif_ch0_tdm_debug (108)
#define mask_of_vif_reg_vif_ch0_tdm_debug   (0xffff)
    unsigned int reg_vif_ch0_tdm_debug : 16;

    // h0036
    unsigned int /* padding 16 bit */ : 16;

// h0037, bit: 14
/* tdm debug
[31:24]: BT3 XY
[23:16]: BT2 XY
[15:8]: BT1 XY
[7:0]: BT0 XY
XY[7]: error protect, 1 is error, 0 is OK
XY[6]: F
XY[5]: V
XY[4]: H
XY[3:0]: ch_num_embed = 1, channel num or polarity
XY[3:0]: ch_num_embed = 0, fix, BT0 = 0, … BT3 = 3*/
#define offset_of_vif_reg_vif_ch0_tdm_debug_1 (110)
#define mask_of_vif_reg_vif_ch0_tdm_debug_1   (0xffff)
    unsigned int reg_vif_ch0_tdm_debug_1 : 16;

    // h0037
    unsigned int /* padding 16 bit */ : 16;

// h0038, bit: 12
/* input pixel count max value select by reg_vif_ch0_total_cnt0_select*/
#define offset_of_vif_reg_vif_ch0_max_pix_cnt0 (112)
#define mask_of_vif_reg_vif_ch0_max_pix_cnt0   (0x1fff)
    unsigned int reg_vif_ch0_max_pix_cnt0 : 13;

    // h0038, bit: 14
    /* */
    unsigned int : 3;

    // h0038
    unsigned int /* padding 16 bit */ : 16;

// h0039, bit: 12
/* input pixel count min value select by reg_vif_ch0_total_cnt0_select*/
#define offset_of_vif_reg_vif_ch0_min_pix_cnt0 (114)
#define mask_of_vif_reg_vif_ch0_min_pix_cnt0   (0x1fff)
    unsigned int reg_vif_ch0_min_pix_cnt0 : 13;

    // h0039, bit: 14
    /* */
    unsigned int : 3;

    // h0039
    unsigned int /* padding 16 bit */ : 16;

// h003a, bit: 12
/* input pixel count max value select by reg_vif_ch0_total_cnt1_select*/
#define offset_of_vif_reg_vif_ch0_max_pix_cnt1 (116)
#define mask_of_vif_reg_vif_ch0_max_pix_cnt1   (0x1fff)
    unsigned int reg_vif_ch0_max_pix_cnt1 : 13;

    // h003a, bit: 14
    /* */
    unsigned int : 3;

    // h003a
    unsigned int /* padding 16 bit */ : 16;

// h003b, bit: 12
/* input pixel count min value select by reg_vif_ch0_total_cnt1_select*/
#define offset_of_vif_reg_vif_ch0_min_pix_cnt1 (118)
#define mask_of_vif_reg_vif_ch0_min_pix_cnt1   (0x1fff)
    unsigned int reg_vif_ch0_min_pix_cnt1 : 13;

    // h003b, bit: 14
    /* */
    unsigned int : 3;

    // h003b
    unsigned int /* padding 16 bit */ : 16;

// h003c, bit: 0
/* new bt656/bt1120 enable*/
#define offset_of_vif_reg_vif_ch0_itu_tdm_en (120)
#define mask_of_vif_reg_vif_ch0_itu_tdm_en   (0x1)
    unsigned int reg_vif_ch0_itu_tdm_en : 1;

// h003c, bit: 1
/* bt1120 mode enable*/
#define offset_of_vif_reg_vif_ch0_itu_tdm_bt1120_en (120)
#define mask_of_vif_reg_vif_ch0_itu_tdm_bt1120_en   (0x2)
    unsigned int reg_vif_ch0_itu_tdm_bt1120_en : 1;

// h003c, bit: 2
/* dual edge enable*/
#define offset_of_vif_reg_vif_ch0_itu_tdm_dual_edge_en (120)
#define mask_of_vif_reg_vif_ch0_itu_tdm_dual_edge_en   (0x4)
    unsigned int reg_vif_ch0_itu_tdm_dual_edge_en : 1;

// h003c, bit: 3
/* yc swap in enable*/
#define offset_of_vif_reg_vif_ch0_itu_tdm_yc_in_swap_en (120)
#define mask_of_vif_reg_vif_ch0_itu_tdm_yc_in_swap_en   (0x8)
    unsigned int reg_vif_ch0_itu_tdm_yc_in_swap_en : 1;

// h003c, bit: 4
/* yc swapout  enable*/
#define offset_of_vif_reg_vif_ch0_itu_tdm_yc_out_swap_en (120)
#define mask_of_vif_reg_vif_ch0_itu_tdm_yc_out_swap_en   (0x10)
    unsigned int reg_vif_ch0_itu_tdm_yc_out_swap_en : 1;

// h003c, bit: 5
/* cbcr enable*/
#define offset_of_vif_reg_vif_ch0_itu_tdm_cbcr_swap_en (120)
#define mask_of_vif_reg_vif_ch0_itu_tdm_cbcr_swap_en   (0x20)
    unsigned int reg_vif_ch0_itu_tdm_cbcr_swap_en : 1;

// h003c, bit: 6
/* ch_num_embed*/
#define offset_of_vif_reg_vif_ch0_itu_tdm_ch_num_embed (120)
#define mask_of_vif_reg_vif_ch0_itu_tdm_ch_num_embed   (0x40)
    unsigned int reg_vif_ch0_itu_tdm_ch_num_embed : 1;

// h003c, bit: 7
/* Negative 0xFF check off, 0: on, 1: off, default is on*/
#define offset_of_vif_reg_vif_ch0_itu_tdm_neg_ff_off (120)
#define mask_of_vif_reg_vif_ch0_itu_tdm_neg_ff_off   (0x80)
    unsigned int reg_vif_ch0_itu_tdm_neg_ff_off : 1;

// h003c, bit: 9
/* ch_num
1to1: 0
1to2: 1
1to4: 3*/
#define offset_of_vif_reg_vif_ch0_itu_tdm_ch_num (120)
#define mask_of_vif_reg_vif_ch0_itu_tdm_ch_num   (0x300)
    unsigned int reg_vif_ch0_itu_tdm_ch_num : 2;

// h003c, bit: 10
/* ch0 field swap*/
#define offset_of_vif_reg_vif_ch0_itu_tdm_ch0_fd_swap (120)
#define mask_of_vif_reg_vif_ch0_itu_tdm_ch0_fd_swap   (0x400)
    unsigned int reg_vif_ch0_itu_tdm_ch0_fd_swap : 1;

// h003c, bit: 11
/* ch1 field swap*/
#define offset_of_vif_reg_vif_ch0_itu_tdm_ch1_fd_swap (120)
#define mask_of_vif_reg_vif_ch0_itu_tdm_ch1_fd_swap   (0x800)
    unsigned int reg_vif_ch0_itu_tdm_ch1_fd_swap : 1;

// h003c, bit: 12
/* ch2 field swap*/
#define offset_of_vif_reg_vif_ch0_itu_tdm_ch2_fd_swap (120)
#define mask_of_vif_reg_vif_ch0_itu_tdm_ch2_fd_swap   (0x1000)
    unsigned int reg_vif_ch0_itu_tdm_ch2_fd_swap : 1;

// h003c, bit: 13
/* ch3 field swap*/
#define offset_of_vif_reg_vif_ch0_itu_tdm_ch3_fd_swap (120)
#define mask_of_vif_reg_vif_ch0_itu_tdm_ch3_fd_swap   (0x2000)
    unsigned int reg_vif_ch0_itu_tdm_ch3_fd_swap : 1;

    // h003c, bit: 14
    /* */
    unsigned int : 1;

// h003c, bit: 15
/* mode
0: 8 bit input
1: 10 bit input */
#define offset_of_vif_reg_vif_ch0_itu_tdm_in_mode (120)
#define mask_of_vif_reg_vif_ch0_itu_tdm_in_mode   (0x8000)
    unsigned int reg_vif_ch0_itu_tdm_in_mode : 1;

    // h003c
    unsigned int /* padding 16 bit */ : 16;

    // h003d, bit: 14
    /* */
    unsigned int : 16;

    // h003d
    unsigned int /* padding 16 bit */ : 16;

// h003e, bit: 14
/* frame total pixel cnt read select by reg_vif_total_frame_pixel_cnt_sel*/
#define offset_of_vif_reg_vif_ch0_total_frame_pix_cnt_rd (124)
#define mask_of_vif_reg_vif_ch0_total_frame_pix_cnt_rd   (0xffff)
    unsigned int reg_vif_ch0_total_frame_pix_cnt_rd : 16;

    // h003e
    unsigned int /* padding 16 bit */ : 16;

// h003f, bit: 7
/* frame total pixel cnt read select by reg_vif_total_frame_pixel_cnt_sel*/
#define offset_of_vif_reg_vif_ch0_total_frame_pix_cnt_rd_1 (126)
#define mask_of_vif_reg_vif_ch0_total_frame_pix_cnt_rd_1   (0xff)
    unsigned int reg_vif_ch0_total_frame_pix_cnt_rd_1 : 8;

    // h003f, bit: 14
    /* */
    unsigned int : 8;

    // h003f
    unsigned int /* padding 16 bit */ : 16;

// h0040, bit: 3
/* vc config*/
#define offset_of_vif_reg_vif_ch0_cfg_addr (128)
#define mask_of_vif_reg_vif_ch0_cfg_addr   (0xf)
    unsigned int reg_vif_ch0_cfg_addr : 4;

    // h0040, bit: 7
    /* */
    unsigned int : 4;

// h0040, bit: 8
/* vc config write enable*/
#define offset_of_vif_reg_vif_ch0_cfg_we (128)
#define mask_of_vif_reg_vif_ch0_cfg_we   (0x100)
    unsigned int reg_vif_ch0_cfg_we : 1;

// h0040, bit: 12
/* 1: data format/ mask_sel/ mask_clr_all
2: data crop
3: data line cnt0/1/data total pix*/
#define offset_of_vif_reg_vif_ch0_cfg_sel (128)
#define mask_of_vif_reg_vif_ch0_cfg_sel   (0x1e00)
    unsigned int reg_vif_ch0_cfg_sel : 4;

    // h0040, bit: 14
    /* */
    unsigned int : 2;

// h0040, bit: 15
/* vc config read enable*/
#define offset_of_vif_reg_vif_ch0_cfg_re (128)
#define mask_of_vif_reg_vif_ch0_cfg_re   (0x8000)
    unsigned int reg_vif_ch0_cfg_re : 1;

    // h0040
    unsigned int /* padding 16 bit */ : 16;

    // h0041, bit: 14
    /* */
    unsigned int : 16;

    // h0041
    unsigned int /* padding 16 bit */ : 16;

// h0042, bit: 14
/* vc config write data*/
#define offset_of_vif_reg_vif_ch0_cfg_wd (132)
#define mask_of_vif_reg_vif_ch0_cfg_wd   (0xffff)
    unsigned int reg_vif_ch0_cfg_wd : 16;

    // h0042
    unsigned int /* padding 16 bit */ : 16;

// h0043, bit: 14
/* vc config write data*/
#define offset_of_vif_reg_vif_ch0_cfg_wd_1 (134)
#define mask_of_vif_reg_vif_ch0_cfg_wd_1   (0xffff)
    unsigned int reg_vif_ch0_cfg_wd_1 : 16;

    // h0043
    unsigned int /* padding 16 bit */ : 16;

// h0044, bit: 14
/* vc config write data*/
#define offset_of_vif_reg_vif_ch0_cfg_wd_2 (136)
#define mask_of_vif_reg_vif_ch0_cfg_wd_2   (0xffff)
    unsigned int reg_vif_ch0_cfg_wd_2 : 16;

    // h0044
    unsigned int /* padding 16 bit */ : 16;

// h0045, bit: 14
/* vc config write data*/
#define offset_of_vif_reg_vif_ch0_cfg_wd_3 (138)
#define mask_of_vif_reg_vif_ch0_cfg_wd_3   (0xffff)
    unsigned int reg_vif_ch0_cfg_wd_3 : 16;

    // h0045
    unsigned int /* padding 16 bit */ : 16;

// h0046, bit: 14
/* VIF0 table read data*/
#define offset_of_vif_reg_vif_ch0_cfg_rd (140)
#define mask_of_vif_reg_vif_ch0_cfg_rd   (0xffff)
    unsigned int reg_vif_ch0_cfg_rd : 16;

    // h0046
    unsigned int /* padding 16 bit */ : 16;

// h0047, bit: 14
/* VIF0 table read data*/
#define offset_of_vif_reg_vif_ch0_cfg_rd_1 (142)
#define mask_of_vif_reg_vif_ch0_cfg_rd_1   (0xffff)
    unsigned int reg_vif_ch0_cfg_rd_1 : 16;

    // h0047
    unsigned int /* padding 16 bit */ : 16;

// h0048, bit: 14
/* VIF0 table read data*/
#define offset_of_vif_reg_vif_ch0_cfg_rd_2 (144)
#define mask_of_vif_reg_vif_ch0_cfg_rd_2   (0xffff)
    unsigned int reg_vif_ch0_cfg_rd_2 : 16;

    // h0048
    unsigned int /* padding 16 bit */ : 16;

// h0049, bit: 14
/* VIF0 table read data*/
#define offset_of_vif_reg_vif_ch0_cfg_rd_3 (146)
#define mask_of_vif_reg_vif_ch0_cfg_rd_3   (0xffff)
    unsigned int reg_vif_ch0_cfg_rd_3 : 16;

    // h0049
    unsigned int /* padding 16 bit */ : 16;

// h004a, bit: 3
/* clear frame count*/
#define offset_of_vif_reg_vif_ch0_frame_mask_clr_all_vc (148)
#define mask_of_vif_reg_vif_ch0_frame_mask_clr_all_vc   (0xf)
    unsigned int reg_vif_ch0_frame_mask_clr_all_vc : 4;

    // h004a, bit: 14
    /* */
    unsigned int : 12;

    // h004a
    unsigned int /* padding 16 bit */ : 16;

    // h004b, bit: 0
    /* VIF crop enable*/
#define offset_of_vif_reg_vif_ch0_crop_en_all_vc0 (150)
#define mask_of_vif_reg_vif_ch0_crop_en_all_vc0   (0x1)
    unsigned int reg_vif_ch0_crop_en_all_vc0 : 1;

    // h004b, bit: 1
    /* VIF crop enable*/
#define offset_of_vif_reg_vif_ch0_crop_en_all_vc1 (150)
#define mask_of_vif_reg_vif_ch0_crop_en_all_vc1   (0x2)
    unsigned int reg_vif_ch0_crop_en_all_vc1 : 1;

    // h004b, bit: 2
    /* VIF crop enable*/
#define offset_of_vif_reg_vif_ch0_crop_en_all_vc2 (150)
#define mask_of_vif_reg_vif_ch0_crop_en_all_vc2   (0x4)
    unsigned int reg_vif_ch0_crop_en_all_vc2 : 1;

    // h004b, bit: 3
    /* VIF crop enable*/
#define offset_of_vif_reg_vif_ch0_crop_en_all_vc3 (150)
#define mask_of_vif_reg_vif_ch0_crop_en_all_vc3   (0x8)
    unsigned int reg_vif_ch0_crop_en_all_vc3 : 1;

    // h004b, bit: 14
    /* */
    unsigned int : 12;

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

// h0050, bit: 12
/* VIF crop pixel count (to ISP)*/
#define offset_of_vif_reg_vif_ch0_crop_pix_cnt0 (160)
#define mask_of_vif_reg_vif_ch0_crop_pix_cnt0   (0x1fff)
    unsigned int reg_vif_ch0_crop_pix_cnt0 : 13;

    // h0050, bit: 14
    /* */
    unsigned int : 3;

    // h0050
    unsigned int /* padding 16 bit */ : 16;

// h0051, bit: 12
/* VIF crop line count (to ISP)*/
#define offset_of_vif_reg_vif_ch0_crop_line_cnt0 (162)
#define mask_of_vif_reg_vif_ch0_crop_line_cnt0   (0x1fff)
    unsigned int reg_vif_ch0_crop_line_cnt0 : 13;

    // h0051, bit: 14
    /* */
    unsigned int : 3;

    // h0051
    unsigned int /* padding 16 bit */ : 16;

// h0052, bit: 12
/* VIF total pixel count*/
#define offset_of_vif_reg_vif_ch0_crop_pix_cnt1 (164)
#define mask_of_vif_reg_vif_ch0_crop_pix_cnt1   (0x1fff)
    unsigned int reg_vif_ch0_crop_pix_cnt1 : 13;

    // h0052, bit: 14
    /* */
    unsigned int : 3;

    // h0052
    unsigned int /* padding 16 bit */ : 16;

// h0053, bit: 12
/* VIF total line count*/
#define offset_of_vif_reg_vif_ch0_crop_line_cnt1 (166)
#define mask_of_vif_reg_vif_ch0_crop_line_cnt1   (0x1fff)
    unsigned int reg_vif_ch0_crop_line_cnt1 : 13;

    // h0053, bit: 14
    /* */
    unsigned int : 3;

    // h0053
    unsigned int /* padding 16 bit */ : 16;

// h0054, bit: 12
/* VIF crop pixel count (to ISP)*/
#define offset_of_vif_reg_vif_ch0_total_pix_cnt0 (168)
#define mask_of_vif_reg_vif_ch0_total_pix_cnt0   (0x1fff)
    unsigned int reg_vif_ch0_total_pix_cnt0 : 13;

    // h0054, bit: 14
    /* */
    unsigned int : 3;

    // h0054
    unsigned int /* padding 16 bit */ : 16;

// h0055, bit: 12
/* VIF crop line count (to ISP)*/
#define offset_of_vif_reg_vif_ch0_total_line_cnt0 (170)
#define mask_of_vif_reg_vif_ch0_total_line_cnt0   (0x1fff)
    unsigned int reg_vif_ch0_total_line_cnt0 : 13;

    // h0055, bit: 14
    /* */
    unsigned int : 3;

    // h0055
    unsigned int /* padding 16 bit */ : 16;

// h0056, bit: 12
/* VIF total pixel count*/
#define offset_of_vif_reg_vif_ch0_total_pix_cnt1 (172)
#define mask_of_vif_reg_vif_ch0_total_pix_cnt1   (0x1fff)
    unsigned int reg_vif_ch0_total_pix_cnt1 : 13;

    // h0056, bit: 14
    /* */
    unsigned int : 3;

    // h0056
    unsigned int /* padding 16 bit */ : 16;

// h0057, bit: 12
/* VIF total line count */
#define offset_of_vif_reg_vif_ch0_total_line_cnt1 (174)
#define mask_of_vif_reg_vif_ch0_total_line_cnt1   (0x1fff)
    unsigned int reg_vif_ch0_total_line_cnt1 : 13;

    // h0057, bit: 14
    /* */
    unsigned int : 3;

    // h0057
    unsigned int /* padding 16 bit */ : 16;

// h0058, bit: 3
/* VIF count debug select*/
#define offset_of_vif_reg_vif_ch0_crop_cnt0_select (176)
#define mask_of_vif_reg_vif_ch0_crop_cnt0_select   (0xf)
    unsigned int reg_vif_ch0_crop_cnt0_select : 4;

// h0058, bit: 7
/* VIF count debug select*/
#define offset_of_vif_reg_vif_ch0_crop_cnt1_select (176)
#define mask_of_vif_reg_vif_ch0_crop_cnt1_select   (0xf0)
    unsigned int reg_vif_ch0_crop_cnt1_select : 4;

// h0058, bit: 11
/* VIF count debug select*/
#define offset_of_vif_reg_vif_ch0_total_cnt0_select (176)
#define mask_of_vif_reg_vif_ch0_total_cnt0_select   (0xf00)
    unsigned int reg_vif_ch0_total_cnt0_select : 4;

// h0058, bit: 14
/* VIF count debug select*/
#define offset_of_vif_reg_vif_ch0_total_cnt1_select (176)
#define mask_of_vif_reg_vif_ch0_total_cnt1_select   (0xf000)
    unsigned int reg_vif_ch0_total_cnt1_select : 4;

    // h0058
    unsigned int /* padding 16 bit */ : 16;

// h0059, bit: 5
/* VIF debug bus select*/
#define offset_of_vif_reg_vif_debug_sel (178)
#define mask_of_vif_reg_vif_debug_sel   (0x3f)
    unsigned int reg_vif_debug_sel : 6;

    // h0059, bit: 11
    /* */
    unsigned int : 6;

// h0059, bit: 13
/* frame total pixel cnt read select */
#define offset_of_vif_reg_vif_ch0_total_frame_pixel_cnt_sel (178)
#define mask_of_vif_reg_vif_ch0_total_frame_pixel_cnt_sel   (0x3000)
    unsigned int reg_vif_ch0_total_frame_pixel_cnt_sel : 2;

    // h0059, bit: 14
    /* */
    unsigned int : 2;

    // h0059
    unsigned int /* padding 16 bit */ : 16;

// h005a, bit: 1
/* frame total pixel cnt read select */
#define offset_of_vif_reg_vif_ch0_irq_clk_en (0)
#define mask_of_vif_reg_vif_ch0_irq_clk_en   (0x0001)
    unsigned int reg_vif_ch0_irq_clk_en : 1;

    // h005a, bit: 15
    /* */
    unsigned int : 15;

    // h005a
    unsigned int /* padding 16 bit */ : 16;

// h005b, bit: 8
/* Interrupt Mask*/
#define offset_of_vif_reg_vif_ch0_c_irq_mask (182)
#define mask_of_vif_reg_vif_ch0_c_irq_mask   (0x1ff)
    unsigned int reg_vif_ch0_c_irq_mask : 9;

    // h005b, bit: 14
    /* */
    unsigned int : 7;

    // h005b
    unsigned int /* padding 16 bit */ : 16;

// h005c, bit: 8
/* Force Interrupt Enable*/
#define offset_of_vif_reg_vif_ch0_c_irq_force (184)
#define mask_of_vif_reg_vif_ch0_c_irq_force   (0x1ff)
    unsigned int reg_vif_ch0_c_irq_force : 9;

    // h005c, bit: 14
    /* */
    unsigned int : 7;

    // h005c
    unsigned int /* padding 16 bit */ : 16;

// h005d, bit: 8
/* Interrupt Clear*/
#define offset_of_vif_reg_vif_ch0_c_irq_clr (186)
#define mask_of_vif_reg_vif_ch0_c_irq_clr   (0x1ff)
    unsigned int reg_vif_ch0_c_irq_clr : 9;

    // h005d, bit: 14
    /* */
    unsigned int : 7;

    // h005d
    unsigned int /* padding 16 bit */ : 16;

// h005e, bit: 8
/* Status of Interrupt on CPU side*/
#define offset_of_vif_reg_vif_ch0_irq_final_status (188)
#define mask_of_vif_reg_vif_ch0_irq_final_status   (0x1ff)
    unsigned int reg_vif_ch0_irq_final_status : 9;

    // h005e, bit: 14
    /* */
    unsigned int : 7;

    // h005e
    unsigned int /* padding 16 bit */ : 16;

// h005f, bit: 8
/* Status of Interrupt on IP side
#[0]: Sensor Source VREF rising edge
#[1]: Sensor Source VREF falling edge
#[3]: PAD2VIF_VSYNC rising edge
#[4]: PAD2VIF_VSYNC falling edge
#[5]: VIF to ISP line count hit0
#[6]: VIF to ISP line count hit1
#[8]: frame total pixel count hit*/
#define offset_of_vif_reg_vif_ch0_irq_raw_status (190)
#define mask_of_vif_reg_vif_ch0_irq_raw_status   (0x1ff)
    unsigned int reg_vif_ch0_irq_raw_status : 9;

    // h005f, bit: 14
    /* */
    unsigned int : 7;

    // h005f
    unsigned int /* padding 16 bit */ : 16;

// h0060, bit: 14
/* Interrupt Mask*/
#define offset_of_vif_reg_vif_ch0_new_irq_mask (192)
#define mask_of_vif_reg_vif_ch0_new_irq_mask0  (0xf)
    unsigned int reg_vif_ch0_new_irq_mask0 : 4;
#define mask_of_vif_reg_vif_ch0_new_irq_mask1 (0xf)
    unsigned int reg_vif_ch0_new_irq_mask1 : 4;
#define mask_of_vif_reg_vif_ch0_new_irq_mask2 (0xf)
    unsigned int reg_vif_ch0_new_irq_mask2 : 4;
#define mask_of_vif_reg_vif_ch0_new_irq_mask3 (0xf)
    unsigned int reg_vif_ch0_new_irq_mask3 : 4;

    // h0060
    unsigned int /* padding 16 bit */ : 16;

// h0061, bit: 14
/* Interrupt Mask*/
#define offset_of_vif_reg_vif_irq_debug_mask0 (194)
#define mask_of_vif_reg_vif_irq_debug_mask0   (0xff)
    unsigned int reg_vif_ch0_irq_debug_mask0 : 8;
#define mask_of_vif_reg_vif_irq_debug_mask1 (0xff)
    unsigned int reg_vif_ch0_irq_debug_mask1 : 8;

    // h0061
    unsigned int /* padding 16 bit */ : 16;

// h0062, bit: 14
/* Interrupt Mask*/
#define offset_of_vif_reg_vif_irq_debug_mask2 (194)
#define mask_of_vif_reg_vif_irq_debug_mask2   (0xff)
    unsigned int reg_vif_ch0_irq_debug_mask2 : 8;
#define mask_of_vif_reg_vif_irq_debug_mask3 (0xff)
    unsigned int reg_vif_ch0_irq_debug_mask3 : 8;

    // h0062
    unsigned int /* padding 16 bit */ : 16;

// h0063, bit: 14
/* Interrupt Mask*/
#define offset_of_vif_reg_vif_ch0_new_irq_mask_3 (198)
#define mask_of_vif_reg_vif_ch0_new_irq_mask_3   (0xffff)
    unsigned int reg_vif_ch0_new_irq_mask_3 : 16;

    // h0063
    unsigned int /* padding 16 bit */ : 16;

    // h0064, bit: 14
    /* */
    unsigned int : 16;

    // h0064
    unsigned int /* padding 16 bit */ : 16;

// h0065, bit: 14
/* Force Interrupt Enable*/
#define offset_of_vif_reg_vif_ch0_new_irq_force (202)
#define mask_of_vif_reg_vif_ch0_new_irq_force   (0xffff)
    unsigned int reg_vif_ch0_new_irq_force : 16;

    // h0065
    unsigned int /* padding 16 bit */ : 16;

// h0066, bit: 14
/* Force Interrupt Enable*/
#define offset_of_vif_reg_vif_ch0_new_irq_force_1 (204)
#define mask_of_vif_reg_vif_ch0_new_irq_force_1   (0xffff)
    unsigned int reg_vif_ch0_new_irq_force_1 : 16;

    // h0066
    unsigned int /* padding 16 bit */ : 16;

// h0067, bit: 14
/* Force Interrupt Enable*/
#define offset_of_vif_reg_vif_ch0_new_irq_force_2 (206)
#define mask_of_vif_reg_vif_ch0_new_irq_force_2   (0xffff)
    unsigned int reg_vif_ch0_new_irq_force_2 : 16;

    // h0067
    unsigned int /* padding 16 bit */ : 16;

// h0068, bit: 14
/* Force Interrupt Enable*/
#define offset_of_vif_reg_vif_ch0_new_irq_force_3 (208)
#define mask_of_vif_reg_vif_ch0_new_irq_force_3   (0xffff)
    unsigned int reg_vif_ch0_new_irq_force_3 : 16;

    // h0068
    unsigned int /* padding 16 bit */ : 16;

    // h0069, bit: 14
    /* */
    unsigned int : 16;

    // h0069
    unsigned int /* padding 16 bit */ : 16;

// h006a, bit: 14
/* Interrupt Clear*/
#define offset_of_vif_reg_vif_ch0_new_irq_clr (212)
#define mask_of_vif_reg_vif_ch0_new_irq_clr   (0xffff)
    unsigned int reg_vif_ch0_new_irq_clr : 16;

    // h006a
    unsigned int /* padding 16 bit */ : 16;

// h006b, bit: 14
/* Interrupt Clear*/
#define offset_of_vif_reg_vif_ch0_new_irq_clr_1 (214)
#define mask_of_vif_reg_vif_ch0_new_irq_clr_1   (0xffff)
    unsigned int reg_vif_ch0_new_irq_clr_1 : 16;

    // h006b
    unsigned int /* padding 16 bit */ : 16;

// h006c, bit: 14
/* Interrupt Clear*/
#define offset_of_vif_reg_vif_ch0_new_irq_clr_2 (216)
#define mask_of_vif_reg_vif_ch0_new_irq_clr_2   (0xffff)
    unsigned int reg_vif_ch0_new_irq_clr_2 : 16;

    // h006c
    unsigned int /* padding 16 bit */ : 16;

// h006d, bit: 14
/* Interrupt Clear*/
#define offset_of_vif_reg_vif_ch0_new_irq_clr_3 (218)
#define mask_of_vif_reg_vif_ch0_new_irq_clr_3   (0xffff)
    unsigned int reg_vif_ch0_new_irq_clr_3 : 16;

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

// h0070, bit: 14
/* Status of Interrupt on CPU side*/
#define offset_of_vif_reg_vif_ch0_new_irq_final_status (224)
#define mask_of_vif_reg_vif_ch0_new_irq_final_status   (0xffff)
    unsigned int reg_vif_ch0_new_irq_final_status : 16;

    // h0070
    unsigned int /* padding 16 bit */ : 16;

// h0071, bit: 14
/* Status of Interrupt on CPU side*/
#define offset_of_vif_reg_vif_ch0_new_irq_final_status_1 (226)
#define mask_of_vif_reg_vif_ch0_new_irq_final_status_1   (0xffff)
    unsigned int reg_vif_ch0_new_irq_final_status_1 : 16;

    // h0071
    unsigned int /* padding 16 bit */ : 16;

// h0072, bit: 14
/* Status of Interrupt on CPU side*/
#define offset_of_vif_reg_vif_ch0_new_irq_final_status_2 (228)
#define mask_of_vif_reg_vif_ch0_new_irq_final_status_2   (0xffff)
    unsigned int reg_vif_ch0_new_irq_final_status_2 : 16;

    // h0072
    unsigned int /* padding 16 bit */ : 16;

// h0073, bit: 14
/* Status of Interrupt on CPU side*/
#define offset_of_vif_reg_vif_ch0_new_irq_final_status_3 (230)
#define mask_of_vif_reg_vif_ch0_new_irq_final_status_3   (0xffff)
    unsigned int reg_vif_ch0_new_irq_final_status_3 : 16;

    // h0073
    unsigned int /* padding 16 bit */ : 16;

    // h0074, bit: 14
    /* */
    unsigned int : 16;

    // h0074
    unsigned int /* padding 16 bit */ : 16;

// h0075, bit: 14
/* RAW Status of Interrupt on CPU side*/
#define offset_of_vif_reg_vif_ch0_new_irq_raw_status (234)
#define mask_of_vif_reg_vif_ch0_new_irq_raw_status   (0xffff)
    unsigned int reg_vif_ch0_new_irq_raw_status : 16;

    // h0075
    unsigned int /* padding 16 bit */ : 16;

// h0076, bit: 14
/* RAW Status of Interrupt on CPU side*/
#define offset_of_vif_reg_vif_ch0_new_irq_raw_status_1 (236)
#define mask_of_vif_reg_vif_ch0_new_irq_raw_status_1   (0xffff)
    unsigned int reg_vif_ch0_new_irq_raw_status_1 : 16;

    // h0076
    unsigned int /* padding 16 bit */ : 16;

// h0077, bit: 14
/* RAW Status of Interrupt on CPU side*/
#define offset_of_vif_reg_vif_ch0_new_irq_raw_status_2 (238)
#define mask_of_vif_reg_vif_ch0_new_irq_raw_status_2   (0xffff)
    unsigned int reg_vif_ch0_new_irq_raw_status_2 : 16;

    // h0077
    unsigned int /* padding 16 bit */ : 16;

// h0078, bit: 14
/* RAW Status of Interrupt on CPU side*/
#define offset_of_vif_reg_vif_ch0_new_irq_raw_status_3 (240)
#define mask_of_vif_reg_vif_ch0_new_irq_raw_status_3   (0xffff)
    unsigned int reg_vif_ch0_new_irq_raw_status_3 : 16;

    // h0078
    unsigned int /* padding 16 bit */ : 16;

    // h0079, bit: 14
    /* */
    unsigned int : 16;

    // h0079
    unsigned int /* padding 16 bit */ : 16;

    // h007a, bit: 14
    /* */
    unsigned int : 16;

    // h007a
    unsigned int /* padding 16 bit */ : 16;

    // h007b, bit: 14
    /* */
    unsigned int : 16;

    // h007b
    unsigned int /* padding 16 bit */ : 16;

    // h007c, bit: 14
    /* */
    unsigned int : 16;

    // h007c
    unsigned int /* padding 16 bit */ : 16;

// h007d, bit: 14
/* dummy register*/
#define offset_of_vif_reg_vif_ch0_dummy1 (250)
#define mask_of_vif_reg_vif_ch0_dummy1   (0xffff)
    unsigned int reg_vif_ch0_dummy1 : 16;

    // h007d
    unsigned int /* padding 16 bit */ : 16;

// h007e, bit: 14
/* sw used*/
#define offset_of_vif_reg_vif_ch0_sw_used0 (252)
#define mask_of_vif_reg_vif_ch0_sw_used0   (0xffff)
    unsigned int reg_vif_ch0_sw_used0 : 16;

    // h007e
    unsigned int /* padding 16 bit */ : 16;

// h007f, bit: 14
/* sw used*/
#define offset_of_vif_reg_vif_ch0_sw_used1 (254)
#define mask_of_vif_reg_vif_ch0_sw_used1   (0xffff)
    unsigned int reg_vif_ch0_sw_used1 : 16;

    // h007f
    unsigned int /* padding 16 bit */ : 16;

} __attribute__((packed, aligned(1))) reg_isp_vif;
#endif
