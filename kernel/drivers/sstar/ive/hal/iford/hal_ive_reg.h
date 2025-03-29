/*
 * hal_ive_reg.h- Sigmastar
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
#ifndef _HAL_IVE_REG_H_
#define _HAL_IVE_REG_H_

#include "cam_os_wrapper.h"

typedef struct
{
    // global setting.
    union
    {
        struct
        {
            u16 sw_fire : 1;
            u16 : 15;
        };
        u16 reg00;
    };

    union
    {
        struct
        {
            u16 sw_rst : 1;
            u16 : 15;
        };
        u16 reg02;
    };

    // interrupt related.
    union
    {
        struct
        {
            u16 irq_mask : 8;
            u16 : 8;
        };
        u16 reg10;
    };

    union
    {
        struct
        {
            u16 irq_force : 8;
            u16 : 8;
        };
        u16 reg11;
    };

    union
    {
        struct
        {
            u16 irq_raw_status : 8;
            u16 : 8;
        };
        u16 reg12;
    };

    union
    {
        struct
        {
            u16 irq_final_status : 8;
            u16 : 8;
        };
        u16 reg13;
    };

    union
    {
        struct
        {
            u16 irq_sel : 1;
            u16 : 15;
        };
        u16 reg14;
    };

    union
    {
        struct
        {
            u16 woc_irq_clr : 8;
            u16 : 8;
        };
        u16 reg15;
    };

    union
    {
        struct
        {
            u16 y_cnt_hit_set0 : 11;
            u16 : 5;
        };
        u16 reg16;
    };

    union
    {
        struct
        {
            u16 y_cnt_hit_set1 : 11;
            u16 : 5;
        };
        u16 reg17;
    };

    union
    {
        struct
        {
            u16 y_cnt_hit_set2 : 11;
            u16 : 5;
        };
        u16 reg18;
    };

    // cmq bus trigger related.
    union
    {
        struct
        {
            u16 cmq_trig_mask : 8;
            u16 : 8;
        };
        u16 reg20;
    };

    union
    {
        struct
        {
            u16 cmq_trig_force : 8;
            u16 : 8;
        };
        u16 reg21;
    };

    union
    {
        struct
        {
            u16 cmq_trig_raw_status : 8;
            u16 : 8;
        };
        u16 reg22;
    };

    union
    {
        struct
        {
            u16 cmq_trig_final_status : 8;
            u16 : 8;
        };
        u16 reg23;
    };

    union
    {
        struct
        {
            u16 cmq_trig_sel : 1;
            u16 : 15;
        };
        u16 reg24;
    };

    union
    {
        struct
        {
            u16 woc_cmq_trig_clr : 8;
            u16 : 8;
        };
        u16 reg25;
    };

    // BIST.
    union
    {
        struct
        {
            u16 bist_fail_rd_low : 16;
        };
        u16 reg30;
    };

    union
    {
        struct
        {
            u16 bist_fail_rd_high : 16;
        };
        u16 reg31;
    };

    // Debug
    union
    {
        struct
        {
            u16 reg_ive_dbg_en : 1;
            u16 : 15;
        };
        u16 reg3f;
    };

    union
    {
        struct
        {
            u16 cycle_count_low : 16;
        };
        u16 reg40;
    };

    union
    {
        struct
        {
            u16 cycle_count_high : 16;
        };
        u16 reg41;
    };

    union
    {
        struct
        {
            u16 dummy_mcm_busy_ctrl : 1;
            u16 dummy_icg_ctrl : 1;
            u16 : 14;
        };
        u16 reg42;
    };

    union
    {
        struct
        {
            u16 reg_ive_dbg_sel : 5;
            u16 : 11;
        };
        u16 reg43;
    };

    union
    {
        struct
        {
            u16 reg_ive_dbg_status_low : 16;
        };
        u16 reg44;
    };

    union
    {
        struct
        {
            u16 reg_ive_dbg_status_high : 8;
            u16 : 8;
        };
        u16 reg45;
    };

    union
    {
        struct
        {
            u16 reg_ive_idle_status : 1;
            u16 : 15;
        };
        u16 reg46;
    };

    union
    {
        struct
        {
            u16 reg_ive_test_bus_sel : 16;
        };
        u16 reg48;
    };

    union
    {
        struct
        {
            u16 reg_ive_test_bus_r_low : 16;
        };
        u16 reg49;
    };

    union
    {
        struct
        {
            u16 reg_ive_test_bus_r_high : 8;
            u16 : 8;
        };
        u16 reg4a;
    };

    union
    {
        struct
        {
            u16 reg_ive_test_bus_w_low : 16;
        };
        u16 reg4b;
    };

    union
    {
        struct
        {
            u16 reg_ive_test_bus_w_high : 8;
            u16 : 8;
        };
        u16 reg4c;
    };

    // AXI interface
    union
    {
        struct
        {
            u16 reg_axi_arlen : 2;
            u16 : 14;
        };
        u16 reg4d;
    };

    union
    {
        struct
        {
            u16 reg_axi_awlen : 2;
            u16 : 14;
        };
        u16 reg4e;
    };

    union
    {
        struct
        {
            u16 reg_axi_ar_max_outstanding : 2;
            u16 : 14;
        };
        u16 reg4f;
    };

    union
    {
        struct
        {
            u16 reg_axi_aw_max_outstanding : 2;
            u16 : 14;
        };
        u16 reg50;
    };

    union
    {
        struct
        {
            u16 reg_axi_ar_qos : 4;
            u16 reg_axi_aw_qos : 4;
            u16 : 8;
        };
        u16 reg51;
    };

    union
    {
        struct
        {
            u16 reg_axi_rd_error : 1;
            u16 reg_axi_wr_error : 1;
            u16 : 14;
        };
        u16 reg52;
    };

    union
    {
        struct
        {
            u16 reg_axi_rd_error_clr : 1;
            u16 reg_axi_wr_error_clr : 1;
            u16 : 14;
        };
        u16 reg53;
    };

    union
    {
        struct
        {
            u16 reg_m2a_early_en : 1;
            u16 : 15;
        };
        u16 reg54;
    };
} ive_hal_reg_bank0;

typedef struct
{
    // Descriptor
    union
    {
        struct
        {
            u16 padding_mode : 2;
            u16 : 14;
        };
        u16 reg03;
    };

    union
    {
        struct
        {
            u16 op_type : 8;
            u16 op_mode : 8;
        };
        u16 reg04;
    };

    union
    {
        struct
        {
            u16 infmt : 8;
            u16 outfmt : 8;
        };
        u16 reg05;
    };

    union
    {
        struct
        {
            u16 frame_width : 16;
        };
        u16 reg06;
    };

    union
    {
        struct
        {
            u16 frame_height : 16;
        };
        u16 reg07;
    };

    union
    {
        struct
        {
            u16 src1_addr_low : 16;
        };
        u16 reg08;
    };

    union
    {
        struct
        {
            u16 src1_addr_high : 16;
        };
        u16 reg09;
    };

    union
    {
        struct
        {
            u16 dst1_addr_low : 16;
        };
        u16 reg0A;
    };

    union
    {
        struct
        {
            u16 dst1_addr_high : 16;
        };
        u16 reg0B;
    };

    union
    {
        struct
        {
            u16 src2_addr_low : 16;
        };
        u16 reg0C;
    };

    union
    {
        struct
        {
            u16 src2_addr_high : 16;
        };
        u16 reg0D;
    };

    union
    {
        struct
        {
            u16 dst2_addr_low : 16;
        };
        u16 reg0E;
    };

    union
    {
        struct
        {
            u16 dst2_addr_high : 16;
        };
        u16 reg0F;
    };

    union
    {
        struct
        {
            u16 src3_addr_low : 16;
        };
        u16 reg10;
    };

    union
    {
        struct
        {
            u16 src3_addr_high : 16;
        };
        u16 reg11;
    };

    union
    {
        struct
        {
            u16 dst3_addr_low : 16;
        };
        u16 reg12;
    };

    union
    {
        struct
        {
            u16 dst3_addr_high : 16;
        };
        u16 reg13;
    };

    union
    {
        struct
        {
            u16 src1_stride : 16;
        };
        u16 reg14;
    };

    union
    {
        struct
        {
            u16 dst1_stride : 16;
        };
        u16 reg15;
    };

    union
    {
        struct
        {
            u16 src2_stride : 16;
        };
        u16 reg16;
    };

    union
    {
        struct
        {
            u16 dst2_stride : 16;
        };
        u16 reg17;
    };

    union
    {
        struct
        {
            u16 src3_stride : 16;
        };
        u16 reg18;
    };

    union
    {
        struct
        {
            u16 dst3_stride : 16;
        };
        u16 reg19;
    };

    union
    {
        struct
        {
            u16 mask0 : 8;
            u16 mask1 : 8;
        };
        // shared reg between V100 and BGBlur.
        struct
        {
            u16 Bgb_mapkey1 : 8;
            u16 Bgb_mapkey2 : 8;
        };
        u16 reg1A;
    };

    union
    {
        struct
        {
            u16 mask2 : 8;
            u16 mask3 : 8;
        };
        // shared reg between V100 and BGBlur.
        struct
        {
            u16 Bgb_mapkey3 : 8;
            u16 Bgb_mapkey4 : 8;
        };
        u16 reg1B;
    };

    union
    {
        struct
        {
            u16 mask4 : 8;
            u16 mask5 : 8;
        };
        // shared reg between V100 and BGBlur.
        struct
        {
            u16 Bgb_mapkey5 : 8;
            u16 Bgb_mapkey6 : 8;
        };
        u16 reg1C;
    };

    union
    {
        struct
        {
            u16 mask6 : 8;
            u16 mask7 : 8;
        };
        // shared reg between V100 and BGBlur.
        struct
        {
            u16 Bgb_mapkey7 : 8;
            u16 Bgb_mapkey8 : 8;
        };
        u16 reg1D;
    };

    union
    {
        struct
        {
            u16 mask8 : 8;
            u16 mask9 : 8;
        };
        // shared reg between V100 and BGBlur.
        struct
        {
            u16 Bgb_mapkey9 : 8;
            u16 Bgb_mapkey10 : 8;
        };
        u16 reg1E;
    };

    union
    {
        struct
        {
            u16 mask10 : 8;
            u16 mask11 : 8;
        };
        // shared reg between V100 and BGBlur.
        struct
        {
            u16 Bgb_mapkey11 : 8;
            u16 Bgb_mapkey12 : 8;
        };
        u16 reg1F;
    };

    union
    {
        struct
        {
            u16 mask12 : 8;
            u16 mask13 : 8;
        };
        // shared reg between V100 and BGBlur.
        struct
        {
            u16 Bgb_mapkey13 : 8;
            u16 Bgb_mapkey14 : 8;
        };
        u16 reg20;
    };

    union
    {
        struct
        {
            u16 mask14 : 8;
            u16 mask15 : 8;
        };
        // shared reg between V100 and BGBlur.
        struct
        {
            u16 Bgb_mapkey15 : 8;
            u16 Bgb_mapval1 : 8;
        };
        u16 reg21;
    };

    union
    {
        struct
        {
            u16 mask16 : 8;
            u16 mask17 : 8;
        };
        // shared reg between V100 and BGBlur.
        struct
        {
            u16 Bgb_mapval2 : 8;
            u16 Bgb_mapval3 : 8;
        };
        u16 reg22;
    };

    union
    {
        struct
        {
            u16 mask18 : 8;
            u16 mask19 : 8;
        };
        // shared reg between V100 and BGBlur.
        struct
        {
            u16 Bgb_mapval4 : 8;
            u16 Bgb_mapval5 : 8;
        };
        u16 reg23;
    };

    union
    {
        struct
        {
            u16 mask20 : 8;
            u16 mask21 : 8;
        };
        // shared reg between V100 and BGBlur.
        struct
        {
            u16 Bgb_mapval6 : 8;
            u16 Bgb_mapval7 : 8;
        };
        u16 reg24;
    };

    union
    {
        struct
        {
            u16 mask22 : 8;
            u16 mask23 : 8;
        };
        // shared reg between V100 and BGBlur.
        struct
        {
            u16 Bgb_mapval8 : 8;
            u16 Bgb_mapval9 : 8;
        };
        u16 reg25;
    };

    union
    {
        struct
        {
            u16 mask24 : 8;
            u16 shift : 8;
        };
        // shared reg between V100 and BGBlur.
        struct
        {
            u16 Bgb_mapval10 : 8;
            u16 : 8;
        };
        u16 reg26;
    };

    union
    {
        struct
        {
            u16 thresh_16bit_1 : 16;
        };
        u16 reg28;
    };

    union
    {
        struct
        {
            u16 thresh_16bit_2 : 16;
        };
        u16 reg29;
    };

    union
    {
        struct
        {
            u16 fraction : 16;
        };
        u16 reg2A;
    };

    union
    {
        struct
        {
            u16 add_weight_x : 16;
        };
        u16 reg2B;
    };

    union
    {
        struct
        {
            u16 add_weight_y : 16;
        };
        u16 reg2C;
    };

    union
    {
        struct
        {
            u16 csc_coeff0 : 12;
            u16 : 4;
        };
        u16 reg30;
    };

    union
    {
        struct
        {
            u16 csc_coeff1 : 12;
            u16 : 4;
        };
        u16 reg31;
    };

    union
    {
        struct
        {
            u16 csc_coeff2 : 12;
            u16 : 4;
        };
        u16 reg32;
    };

    union
    {
        struct
        {
            u16 csc_coeff3 : 12;
            u16 : 4;
        };
        u16 reg33;
    };

    union
    {
        struct
        {
            u16 csc_coeff4 : 12;
            u16 : 4;
        };
        u16 reg34;
    };

    union
    {
        struct
        {
            u16 csc_coeff5 : 12;
            u16 : 4;
        };
        u16 reg35;
    };

    union
    {
        struct
        {
            u16 csc_coeff6 : 12;
            u16 : 4;
        };
        u16 reg36;
    };

    union
    {
        struct
        {
            u16 csc_coeff7 : 12;
            u16 : 4;
        };
        u16 reg37;
    };

    union
    {
        struct
        {
            u16 csc_coeff8 : 12;
            u16 : 4;
        };
        u16 reg38;
    };

    union
    {
        struct
        {
            u16 csc_offset0 : 12;
            u16 : 4;
        };
        u16 reg39;
    };

    union
    {
        struct
        {
            u16 csc_offset1 : 12;
            u16 : 4;
        };
        u16 reg3A;
    };

    union
    {
        struct
        {
            u16 csc_offset2 : 12;
            u16 : 4;
        };
        u16 reg3B;
    };

    union
    {
        struct
        {
            u16 csc_clamp0_low : 8;
            u16 csc_clamp0_high : 8;
        };
        u16 reg3C;
    };

    union
    {
        struct
        {
            u16 csc_clamp1_low : 8;
            u16 csc_clamp1_high : 8;
        };
        u16 reg3D;
    };

    union
    {
        struct
        {
            u16 csc_clamp2_low : 8;
            u16 csc_clamp2_high : 8;
        };
        u16 reg3E;
    };

    union
    {
        struct
        {
            u16 reg_ive_matrix_coeff0_low : 16;
        };
        u16 reg40;
    };

    union
    {
        struct
        {
            u16 reg_ive_matrix_coeff0_high : 16;
        };
        u16 reg41;
    };

    union
    {
        struct
        {
            u16 reg_ive_matrix_coeff1_low : 16;
        };
        u16 reg42;
    };

    union
    {
        struct
        {
            u16 reg_ive_matrix_coeff1_high : 16;
        };
        u16 reg43;
    };

    union
    {
        struct
        {
            u16 reg_ive_matrix_coeff2_low : 16;
        };
        u16 reg44;
    };

    union
    {
        struct
        {
            u16 reg_ive_matrix_coeff2_high : 16;
        };
        u16 reg45;
    };

    union
    {
        struct
        {
            u16 reg_ive_src1_addr_35_32 : 4;
            u16 : 12;
        };
        u16 reg46;
    };

    union
    {
        struct
        {
            u16 reg_ive_dst1_addr_35_32 : 4;
            u16 : 12;
        };
        u16 reg47;
    };

    union
    {
        struct
        {
            u16 reg_ive_src2_addr_35_32 : 4;
            u16 : 12;
        };
        u16 reg48;
    };

    union
    {
        struct
        {
            u16 reg_ive_dst2_addr_35_32 : 4;
            u16 : 12;
        };
        u16 reg49;
    };

    union
    {
        struct
        {
            u16 reg_ive_src3_addr_35_32 : 4;
            u16 : 12;
        };
        u16 reg4a;
    };

    union
    {
        struct
        {
            u16 reg_ive_dst3_addr_35_32 : 4;
            u16 : 12;
        };
        u16 reg4b;
    };

    union
    {
        struct
        {
            u16 mask25 : 8;
            u16 mask26 : 8;
        };
        // shared reg between V100 and BGBlur.
        struct
        {
            u16 Bgb_mapval11 : 8;
            u16 Bgb_mapval12 : 8;
        };
        u16 reg4c;
    };

    union
    {
        struct
        {
            u16 mask27 : 8;
            u16 mask28 : 8;
        };
        // shared reg between V100 and BGBlur.
        struct
        {
            u16 Bgb_mapval13 : 8;
            u16 Bgb_mapval14 : 8;
        };
        u16 reg4d;
    };

    union
    {
        struct
        {
            u16 mask29 : 8;
            u16 mask30 : 8;
        };
        // shared reg between V100 and BGBlur.
        struct
        {
            u16 Bgb_mapval15 : 8;
            u16 Bgb_mapval16 : 8;
        };
        u16 reg4e;
    };
} ive_hal_reg_bank1;

typedef struct
{
    // global setting.
    union
    {
        struct
        {
            u16 reg_bgb_sw_fire : 1;
            u16 : 15;
        };
        u16 reg00;
    };

    union
    {
        struct
        {
            u16 reg_bgb_sw_rst : 1;
            u16 : 15;
        };
        u16 reg01;
    };

    // interrupt related.
    union
    {
        struct
        {
            u16 reg_bgb_irq_mask : 4;
            u16 : 12;
        };
        u16 reg02;
    };

    union
    {
        struct
        {
            u16 reg_bgb_irq_force : 4;
            u16 : 12;
        };
        u16 reg03;
    };

    union
    {
        struct
        {
            u16 reg_bgb_irq_raw_status : 4;
            u16 : 12;
        };
        u16 reg04;
    };

    union
    {
        struct
        {
            u16 reg_bgb_irq_final_status : 4;
            u16 : 12;
        };
        u16 reg05;
    };

    union
    {
        struct
        {
            u16 reg_bgb_irq_sel : 1;
            u16 : 15;
        };
        u16 reg06;
    };

    union
    {
        struct
        {
            u16 reg_bgb_woc_irq_clr : 4;
            u16 : 12;
        };
        u16 reg07;
    };

    // cmq bus trigger related.
    union
    {
        struct
        {
            u16 reg_bgb_cmq_trig_mask : 4;
            u16 : 12;
        };
        u16 reg08;
    };

    union
    {
        struct
        {
            u16 reg_bgb_cmq_trig_force : 4;
            u16 : 12;
        };
        u16 reg09;
    };

    union
    {
        struct
        {
            u16 reg_bgb_cmq_trig_raw_status : 4;
            u16 : 12;
        };
        u16 reg0a;
    };

    union
    {
        struct
        {
            u16 reg_bgb_cmq_trig_final_status : 4;
            u16 : 12;
        };
        u16 reg0b;
    };

    union
    {
        struct
        {
            u16 reg_bgb_cmq_trig_sel : 1;
            u16 : 15;
        };
        u16 reg0c;
    };

    union
    {
        struct
        {
            u16 reg_bgb_woc_cmq_trig_clr : 4;
            u16 : 12;
        };
        u16 reg0d;
    };

    // BIST.
    union
    {
        struct
        {
            u16 reg_bgb_bist_fail_rd_low : 16;
        };
        u16 reg0e;
    };

    union
    {
        struct
        {
            u16 reg_bgb_bist_fail_rd_high : 16;
        };
        u16 reg0f;
    };

    // Debug.
    union
    {
        struct
        {
            u16 reg_bgb_dbg_en : 1;
            u16 : 15;
        };
        u16 reg10;
    };

    union
    {
        struct
        {
            u16 reg_bgb_uv_marker_overflow_row : 11;
            u16 : 5;
        };
        u16 reg11;
    };

    union
    {
        struct
        {
            u16 reg_bgb_y_marker_overflow_row : 11;
            u16 : 5;
        };
        u16 reg12;
    };

    union
    {
        struct
        {
            u16 reg_bgb_dummy_mcm_busy_ctrl : 1;
            u16 : 15;
        };
        u16 reg13;
    };

    union
    {
        struct
        {
            u16 reg_bgb_dbg_sel : 5;
            u16 : 11;
        };
        u16 reg14;
    };

    union
    {
        struct
        {
            u16 reg_bgb_dbg_status_low : 16;
        };
        u16 reg15;
    };

    union
    {
        struct
        {
            u16 reg_bgb_dbg_status_high : 8;
            u16 : 8;
        };
        u16 reg16;
    };

    union
    {
        struct
        {
            u16 reg_bgb_idle_status : 1;
            u16 : 15;
        };
        u16 reg17;
    };

    // Descriptor.
    union
    {
        struct
        {
            u16 reg_bgb_src0_c0_addr_low : 16;
        };
        u16 reg19;
    };

    union
    {
        struct
        {
            u16 reg_bgb_src0_c0_addr_high : 16;
        };
        u16 reg1a;
    };

    union
    {
        struct
        {
            u16 reg_bgb_src0_c0_addr_h : 4;
            u16 : 12;
        };
        u16 reg1b;
    };

    union
    {
        struct
        {
            u16 reg_bgb_src0_c0_stride : 16;
        };
        u16 reg1c;
    };

    union
    {
        struct
        {
            u16 reg_bgb_src0_c1_addr_low : 16;
        };
        u16 reg1d;
    };

    union
    {
        struct
        {
            u16 reg_bgb_src0_c1_addr_high : 16;
        };
        u16 reg1e;
    };

    union
    {
        struct
        {
            u16 reg_bgb_src0_c1_addr_h : 4;
            u16 : 12;
        };
        u16 reg1f;
    };

    union
    {
        struct
        {
            u16 reg_bgb_src0_c1_stride : 16;
        };
        u16 reg20;
    };

    union
    {
        struct
        {
            u16 reg_bgb_src1_c0_addr_low : 16;
        };
        u16 reg21;
    };

    union
    {
        struct
        {
            u16 reg_bgb_src1_c0_addr_high : 16;
        };
        u16 reg22;
    };

    union
    {
        struct
        {
            u16 reg_bgb_src1_c0_addr_h : 4;
            u16 : 12;
        };
        u16 reg23;
    };

    union
    {
        struct
        {
            u16 reg_bgb_src1_c0_stride : 16;
        };
        u16 reg24;
    };

    union
    {
        struct
        {
            u16 reg_bgb_src1_c1_addr_low : 16;
        };
        u16 reg25;
    };

    union
    {
        struct
        {
            u16 reg_bgb_src1_c1_addr_high : 16;
        };
        u16 reg26;
    };

    union
    {
        struct
        {
            u16 reg_bgb_src1_c1_addr_h : 4;
            u16 : 12;
        };
        u16 reg27;
    };

    union
    {
        struct
        {
            u16 reg_bgb_src1_c1_stride : 16;
        };
        u16 reg28;
    };

    union
    {
        struct
        {
            u16 reg_bgb_src2_c0_addr_low : 16;
        };
        u16 reg29;
    };

    union
    {
        struct
        {
            u16 reg_bgb_src2_c0_addr_high : 16;
        };
        u16 reg2a;
    };

    union
    {
        struct
        {
            u16 reg_bgb_src2_c0_addr_h : 4;
            u16 : 12;
        };
        u16 reg2b;
    };

    union
    {
        struct
        {
            u16 reg_bgb_src2_c0_stride : 16;
        };
        u16 reg2c;
    };

    union
    {
        struct
        {
            u16 reg_bgb_src2_c1_addr_low : 16;
        };
        u16 reg2d;
    };

    union
    {
        struct
        {
            u16 reg_bgb_src2_c1_addr_high : 16;
        };
        u16 reg2e;
    };

    union
    {
        struct
        {
            u16 reg_bgb_src2_c1_addr_h : 4;
            u16 : 12;
        };
        u16 reg2f;
    };

    union
    {
        struct
        {
            u16 reg_bgb_src2_c1_stride : 16;
        };
        u16 reg30;
    };

    union
    {
        struct
        {
            u16 reg_bgb_mid_c0_addr_low : 16;
        };
        u16 reg39;
    };

    union
    {
        struct
        {
            u16 reg_bgb_mid_c0_addr_high : 16;
        };
        u16 reg3a;
    };

    union
    {
        struct
        {
            u16 reg_bgb_mid_c0_addr_h : 4;
            u16 : 12;
        };
        u16 reg3b;
    };

    union
    {
        struct
        {
            u16 reg_bgb_mid_c0_stride : 16;
        };
        u16 reg3c;
    };

    union
    {
        struct
        {
            u16 reg_bgb_mid_c1_addr_low : 16;
        };
        u16 reg3d;
    };

    union
    {
        struct
        {
            u16 reg_bgb_mid_c1_addr_high : 16;
        };
        u16 reg3e;
    };

    union
    {
        struct
        {
            u16 reg_bgb_mid_c1_addr_h : 4;
            u16 : 12;
        };
        u16 reg3f;
    };

    union
    {
        struct
        {
            u16 reg_bgb_mid_c1_stride : 16;
        };
        u16 reg40;
    };

    union
    {
        struct
        {
            u16 reg_bgb_dst_c0_addr_low : 16;
        };
        u16 reg59;
    };

    union
    {
        struct
        {
            u16 reg_bgb_dst_c0_addr_high : 16;
        };
        u16 reg5a;
    };

    union
    {
        struct
        {
            u16 reg_bgb_dst_c0_addr_h : 4;
            u16 : 12;
        };
        u16 reg5b;
    };

    union
    {
        struct
        {
            u16 reg_bgb_dst_c0_stride : 16;
        };
        u16 reg5c;
    };

    union
    {
        struct
        {
            u16 reg_bgb_dst_c1_addr_low : 16;
        };
        u16 reg5d;
    };

    union
    {
        struct
        {
            u16 reg_bgb_dst_c1_addr_high : 16;
        };
        u16 reg5e;
    };

    union
    {
        struct
        {
            u16 reg_bgb_dst_c1_addr_h : 4;
            u16 : 12;
        };
        u16 reg5f;
    };

    union
    {
        struct
        {
            u16 reg_bgb_dst_c1_stride : 16;
        };
        u16 reg60;
    };

    union
    {
        struct
        {
            u16 reg_bgb_img0_width : 16;
        };
        u16 reg61;
    };

    union
    {
        struct
        {
            u16 reg_bgb_img0_height : 16;
        };
        u16 reg62;
    };

    union
    {
        struct
        {
            u16 reg_bgb_img1_width : 16;
        };
        u16 reg63;
    };

    union
    {
        struct
        {
            u16 reg_bgb_img1_height : 16;
        };
        u16 reg64;
    };

    union
    {
        struct
        {
            u16 reg_bgb_img3_width : 16;
        };
        u16 reg67;
    };

    union
    {
        struct
        {
            u16 reg_bgb_img3_height : 16;
        };
        u16 reg68;
    };

    // Format and Mode
    union
    {
        struct
        {
            u16 reg_bgb_op_type : 2;
            u16 : 14;
        };
        u16 reg69;
    };

    // Alu Setting

    union
    {
        struct
        {
            u16 reg_bgb_mask_map_en : 1;
            u16 : 15;
        };
        u16 reg6a;
    };

    union
    {
        struct
        {
            u16 reg_bgb_mask_fill_size : 9;
            u16 : 7;
        };
        u16 reg6b;
    };

    union
    {
        struct
        {
            u16 reg_bgb_mask_th_sml : 8;
            u16 : 8;
        };
        u16 reg6c;
    };

    union
    {
        struct
        {
            u16 reg_bgb_sat_level : 8;
            u16 : 8;
        };
        u16 reg6d;
    };

    union
    {
        struct
        {
            u16 reg_bgb_g1_g3_resize_ratio_x_low : 16;
        };
        u16 reg72;
    };

    union
    {
        struct
        {
            u16 reg_bgb_g1_g3_resize_ratio_x_high : 2;
            u16 : 14;
        };
        u16 reg73;
    };

    union
    {
        struct
        {
            u16 reg_bgb_g1_g3_resize_ratio_y_low : 16;
        };
        u16 reg74;
    };

    union
    {
        struct
        {
            u16 reg_bgb_g1_g3_resize_ratio_y_high : 2;
            u16 : 14;
        };
        u16 reg75;
    };

    union
    {
        struct
        {
            u16 reg_bgb_g5_resize_ratio_x_low : 16;
        };
        u16 reg76;
    };

    union
    {
        struct
        {
            u16 reg_bgb_g5_resize_ratio_x_high : 2;
            u16 : 14;
        };
        u16 reg77;
    };

    union
    {
        struct
        {
            u16 reg_bgb_g5_resize_ratio_y_low : 16;
        };
        u16 reg78;
    };

    union
    {
        struct
        {
            u16 reg_bgb_g5_resize_ratio_y_high : 2;
            u16 : 14;
        };
        u16 reg79;
    };

    union
    {
        struct
        {
            u16 reg_bgb_mosaic_block : 4;
            u16 : 12;
        };
        u16 reg7a;
    };

    // AXI interface enhance
    union
    {
        struct
        {
            u16 reg_bgb_auto_axiqos_en : 1;
            u16 : 15;
        };
        u16 reg7b;
    };

    union
    {
        struct
        {
            u16 reg_bgb_arqos_water_limit : 10;
            u16 : 6;
        };
        u16 reg7c;
    };

    union
    {
        struct
        {
            u16 reg_bgb_awqos_water_limit : 10;
            u16 : 6;
        };
        u16 reg7d;
    };

    union
    {
        struct
        {
            u16 reg_bgb_arqos_water_limit_g5 : 10;
            u16 : 6;
        };
        u16 reg7e;
    };
} ive_hal_reg_bank3;
#endif // _HAL_IVE_REG_H_
