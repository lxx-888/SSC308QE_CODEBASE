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
            u16 dummy_bpp32_eco_ctrl : 1;
            u16 dummy_sad1_eco_ctrl : 1;
            u16 dummy_sad2_eco_ctrl : 1;
            u16 : 11;
        };
        u16 reg42;
    };

    // AXI Interface
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
            u16 reg_axi_ar_max_outstanding : 5;
            u16 : 11;
        };
        u16 reg4f;
    };

    union
    {
        struct
        {
            u16 reg_axi_aw_max_outstanding : 5;
            u16 : 11;
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
            u16 reg_m2a_skip_early : 1;
            u16 : 15;
        };
        u16 reg54;
    };
} ive_hal_reg_bank0;

typedef struct
{
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
        u16 reg1A;
    };

    union
    {
        struct
        {
            u16 mask2 : 8;
            u16 mask3 : 8;
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
        u16 reg1C;
    };

    union
    {
        struct
        {
            u16 mask6 : 8;
            u16 mask7 : 8;
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
        u16 reg1E;
    };

    union
    {
        struct
        {
            u16 mask10 : 8;
            u16 mask11 : 8;
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
        u16 reg20;
    };

    union
    {
        struct
        {
            u16 mask14 : 8;
            u16 mask15 : 8;
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
        u16 reg22;
    };

    union
    {
        struct
        {
            u16 mask18 : 8;
            u16 mask19 : 8;
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
        u16 reg24;
    };

    union
    {
        struct
        {
            u16 mask22 : 8;
            u16 mask23 : 8;
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
            u16 reg_ive_dst_frame_width : 16;
        };
        u16 reg4c;
    };

    union
    {
        struct
        {
            u16 reg_ive_dst_frame_height : 16;
        };
        u16 reg4d;
    };

    union
    {
        struct
        {
            u16 reg_ive_src4_addr_low : 16;
        };
        u16 reg4f;
    };

    union
    {
        struct
        {
            u16 reg_ive_src4_addr_high : 16;
        };
        u16 reg50;
    };

    union
    {
        struct
        {
            u16 reg_ive_src4_addr_h : 4;
            u16 : 12;
        };
        u16 reg51;
    };

    union
    {
        struct
        {
            u16 reg_ive_src4_stride : 16;
        };
        u16 reg52;
    };

    union
    {
        struct
        {
            u16 reg_ive_dst4_addr_low : 16;
        };
        u16 reg53;
    };

    union
    {
        struct
        {
            u16 reg_ive_dst4_addr_high : 16;
        };
        u16 reg54;
    };

    union
    {
        struct
        {
            u16 reg_ive_dst4_addr_h : 4;
            u16 : 12;
        };
        u16 reg55;
    };

    union
    {
        struct
        {
            u16 reg_ive_dst4_stride : 16;
        };
        u16 reg56;
    };

    union
    {
        struct
        {
            u16 reg_ive_dst5_addr_low : 16;
        };
        u16 reg57;
    };

    union
    {
        struct
        {
            u16 reg_ive_dst5_addr_high : 16;
        };
        u16 reg58;
    };

    union
    {
        struct
        {
            u16 reg_ive_dst5_addr_h : 4;
            u16 : 12;
        };
        u16 reg59;
    };

    union
    {
        struct
        {
            u16 reg_ive_dst5_stride : 16;
        };
        u16 reg5a;
    };

    union
    {
        struct
        {
            u16 reg_ive_dst6_addr_low : 16;
        };
        u16 reg5b;
    };

    union
    {
        struct
        {
            u16 reg_ive_dst6_addr_high : 16;
        };
        u16 reg5c;
    };

    union
    {
        struct
        {
            u16 reg_ive_dst6_addr_h : 4;
            u16 : 12;
        };
        u16 reg5d;
    };

    union
    {
        struct
        {
            u16 reg_ive_dst6_stride : 16;
        };
        u16 reg5e;
    };

    union
    {
        struct
        {
            u16 reg_ive_dst7_addr_low : 16;
        };
        u16 reg5f;
    };

    union
    {
        struct
        {
            u16 reg_ive_dst7_addr_high : 16;
        };
        u16 reg60;
    };

    union
    {
        struct
        {
            u16 reg_ive_dst7_addr_h : 4;
            u16 : 12;
        };
        u16 reg61;
    };

    union
    {
        struct
        {
            u16 reg_ive_dst7_stride : 16;
        };
        u16 reg62;
    };

    union
    {
        struct
        {
            u16 reg_ive_dst8_addr_low : 16;
        };
        u16 reg63;
    };

    union
    {
        struct
        {
            u16 reg_ive_dst8_addr_high : 16;
        };
        u16 reg64;
    };

    union
    {
        struct
        {
            u16 reg_ive_dst8_addr_h : 4;
            u16 : 12;
        };
        u16 reg65;
    };

    union
    {
        struct
        {
            u16 reg_ive_dst8_stride : 16;
        };
        u16 reg66;
    };

    union
    {
        struct
        {
            u16 reg_ive_dst9_addr_low : 16;
        };
        u16 reg67;
    };

    union
    {
        struct
        {
            u16 reg_ive_dst9_addr_high : 16;
        };
        u16 reg68;
    };

    union
    {
        struct
        {
            u16 reg_ive_dst9_addr_h : 4;
            u16 : 12;
        };
        u16 reg69;
    };

    union
    {
        struct
        {
            u16 reg_ive_dst9_stride : 16;
        };
        u16 reg6a;
    };
} ive_hal_reg_bank1;

typedef struct
{
    union
    {
        struct
        {
            u16 reg_ive_ccl_mode : 1;
            u16 : 15;
        };
        u16 reg00;
    };

    union
    {
        struct
        {
            u16 reg_ive_ccl_init_area_th : 16;
        };
        u16 reg01;
    };

    union
    {
        struct
        {
            u16 reg_ive_ccl_step : 16;
        };
        u16 reg02;
    };

    union
    {
        struct
        {
            u16 reg_ive_gmm_need_to_init : 1;
            u16 : 15;
        };
        u16 reg10;
    };

    union
    {
        struct
        {
            u16 reg_ive_gmm_max_var_low : 16;
        };
        u16 reg11;
    };

    union
    {
        struct
        {
            u16 reg_ive_gmm_max_var_high : 8;
            u16 : 8;
        };
        u16 reg12;
    };

    union
    {
        struct
        {
            u16 reg_ive_gmm_min_var_low : 16;
        };
        u16 reg13;
    };

    union
    {
        struct
        {
            u16 reg_ive_gmm_min_var_high : 8;
            u16 : 8;
        };
        u16 reg14;
    };

    union
    {
        struct
        {
            u16 reg_ive_gmm_learn_rate : 16;
        };
        u16 reg15;
    };

    union
    {
        struct
        {
            u16 reg_ive_gmm_bg_ratio : 16;
        };
        u16 reg16;
    };

    union
    {
        struct
        {
            u16 reg_ive_gmm_var_thr_low : 16;
        };
        u16 reg17;
    };

    union
    {
        struct
        {
            u16 reg_ive_gmm_var_thr_high : 8;
            u16 : 8;
        };
        u16 reg18;
    };

    union
    {
        struct
        {
            u16 reg_ive_gmm_init_var_low : 16;
        };
        u16 reg19;
    };

    union
    {
        struct
        {
            u16 reg_ive_gmm_init_var_high : 8;
            u16 : 8;
        };
        u16 reg1a;
    };

    union
    {
        struct
        {
            u16 reg_ive_gmm_model_num : 3;
            u16 : 13;
        };
        u16 reg1b;
    };

    union
    {
        struct
        {
            u16 reg_ive_gmm_var_thr_bg_low : 16;
        };
        u16 reg1c;
    };

    union
    {
        struct
        {
            u16 reg_ive_gmm_var_thr_bg_high : 8;
            u16 : 8;
        };
        u16 reg1d;
    };

    union
    {
        struct
        {
            u16 reg_ive_gmm_ct : 16;
        };
        u16 reg1e;
    };

    union
    {
        struct
        {
            u16 reg_ive_gmm_frame_mode : 2;
            u16 : 14;
        };
        u16 reg1f;
    };

    union
    {
        struct
        {
            u16 reg_ive_resize_mode : 1;
            u16 : 15;
        };
        u16 reg28;
    };

    union
    {
        struct
        {
            u16 reg_ive_resize_area_xcoef_low : 16;
        };
        u16 reg29;
    };

    union
    {
        struct
        {
            u16 reg_ive_resize_area_xcoef_high : 2;
            u16 : 14;
        };
        u16 reg2a;
    };

    union
    {
        struct
        {
            u16 reg_ive_resize_area_ycoef_low : 16;
        };
        u16 reg2b;
    };

    union
    {
        struct
        {
            u16 reg_ive_resize_area_ycoef_high : 2;
            u16 : 14;
        };
        u16 reg2c;
    };

    union
    {
        struct
        {
            u16 reg_ive_resize_ratio_x_low : 16;
        };
        u16 reg2d;
    };

    union
    {
        struct
        {
            u16 reg_ive_resize_ratio_x_high : 6;
            u16 : 10;
        };
        u16 reg2e;
    };

    union
    {
        struct
        {
            u16 reg_ive_resize_ratio_y_low : 16;
        };
        u16 reg2f;
    };

    union
    {
        struct
        {
            u16 reg_ive_resize_ratio_y_high : 6;
            u16 : 10;
        };
        u16 reg30;
    };

} ive_hal_reg_bank2;
typedef struct
{
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
            u16 reg_bgb_src3_c0_addr_low : 16;
        };
        u16 reg31;
    };

    union
    {
        struct
        {
            u16 reg_bgb_src3_c0_addr_high : 16;
        };
        u16 reg32;
    };

    union
    {
        struct
        {
            u16 reg_bgb_src3_c0_addr_h : 4;
            u16 : 12;
        };
        u16 reg33;
    };

    union
    {
        struct
        {
            u16 reg_bgb_src3_c0_stride : 16;
        };
        u16 reg34;
    };

    union
    {
        struct
        {
            u16 reg_bgb_src3_c1_addr_low : 16;
        };
        u16 reg35;
    };

    union
    {
        struct
        {
            u16 reg_bgb_src3_c1_addr_high : 16;
        };
        u16 reg36;
    };

    union
    {
        struct
        {
            u16 reg_bgb_src3_c1_addr_h : 4;
            u16 : 12;
        };
        u16 reg37;
    };

    union
    {
        struct
        {
            u16 reg_bgb_src3_c1_stride : 16;
        };
        u16 reg38;
    };

    union
    {
        struct
        {
            u16 reg_bgb_temp0_c0_addr_low : 16;
        };
        u16 reg39;
    };

    union
    {
        struct
        {
            u16 reg_bgb_temp0_c0_addr_high : 16;
        };
        u16 reg3a;
    };

    union
    {
        struct
        {
            u16 reg_bgb_temp0_c0_addr_h : 4;
            u16 : 12;
        };
        u16 reg3b;
    };

    union
    {
        struct
        {
            u16 reg_bgb_temp0_c0_stride : 16;
        };
        u16 reg3c;
    };

    union
    {
        struct
        {
            u16 reg_bgb_temp0_c1_addr_low : 16;
        };
        u16 reg3d;
    };

    union
    {
        struct
        {
            u16 reg_bgb_temp0_c1_addr_high : 16;
        };
        u16 reg3e;
    };

    union
    {
        struct
        {
            u16 reg_bgb_temp0_c1_addr_h : 4;
            u16 : 12;
        };
        u16 reg3f;
    };

    union
    {
        struct
        {
            u16 reg_bgb_temp0_c1_stride : 16;
        };
        u16 reg40;
    };

    union
    {
        struct
        {
            u16 reg_bgb_temp1_c0_addr_low : 16;
        };
        u16 reg41;
    };

    union
    {
        struct
        {
            u16 reg_bgb_temp1_c0_addr_high : 16;
        };
        u16 reg42;
    };

    union
    {
        struct
        {
            u16 reg_bgb_temp1_c0_addr_h : 4;
            u16 : 12;
        };
        u16 reg43;
    };

    union
    {
        struct
        {
            u16 reg_bgb_temp1_c0_stride : 16;
        };
        u16 reg44;
    };

    union
    {
        struct
        {
            u16 reg_bgb_temp1_c1_addr_low : 16;
        };
        u16 reg45;
    };

    union
    {
        struct
        {
            u16 reg_bgb_temp1_c1_addr_high : 16;
        };
        u16 reg46;
    };

    union
    {
        struct
        {
            u16 reg_bgb_temp1_c1_addr_h : 4;
            u16 : 12;
        };
        u16 reg47;
    };

    union
    {
        struct
        {
            u16 reg_bgb_temp1_c1_stride : 16;
        };
        u16 reg48;
    };

    union
    {
        struct
        {
            u16 reg_bgb_temp2_c0_addr_low : 16;
        };
        u16 reg49;
    };

    union
    {
        struct
        {
            u16 reg_bgb_temp2_c0_addr_high : 16;
        };
        u16 reg4a;
    };

    union
    {
        struct
        {
            u16 reg_bgb_temp2_c0_addr_h : 4;
            u16 : 12;
        };
        u16 reg4b;
    };

    union
    {
        struct
        {
            u16 reg_bgb_temp2_c0_stride : 16;
        };
        u16 reg4c;
    };

    union
    {
        struct
        {
            u16 reg_bgb_temp2_c1_addr_low : 16;
        };
        u16 reg4d;
    };

    union
    {
        struct
        {
            u16 reg_bgb_temp2_c1_addr_high : 16;
        };
        u16 reg4e;
    };

    union
    {
        struct
        {
            u16 reg_bgb_temp2_c1_addr_h : 4;
            u16 : 12;
        };
        u16 reg4f;
    };

    union
    {
        struct
        {
            u16 reg_bgb_temp2_c1_stride : 16;
        };
        u16 reg50;
    };

    union
    {
        struct
        {
            u16 reg_bgb_temp3_c0_addr_low : 16;
        };
        u16 reg51;
    };

    union
    {
        struct
        {
            u16 reg_bgb_temp3_c0_addr_high : 16;
        };
        u16 reg52;
    };

    union
    {
        struct
        {
            u16 reg_bgb_temp3_c0_addr_h : 4;
            u16 : 12;
        };
        u16 reg53;
    };

    union
    {
        struct
        {
            u16 reg_bgb_temp3_c0_stride : 16;
        };
        u16 reg54;
    };

    union
    {
        struct
        {
            u16 reg_bgb_temp3_c1_addr_low : 16;
        };
        u16 reg55;
    };

    union
    {
        struct
        {
            u16 reg_bgb_temp3_c1_addr_high : 16;
        };
        u16 reg56;
    };

    union
    {
        struct
        {
            u16 reg_bgb_temp3_c1_addr_h : 4;
            u16 : 12;
        };
        u16 reg57;
    };

    union
    {
        struct
        {
            u16 reg_bgb_temp3_c1_stride : 16;
        };
        u16 reg58;
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
            u16 reg_bgb_img2_width : 16;
        };
        u16 reg65;
    };

    union
    {
        struct
        {
            u16 reg_bgb_img2_height : 16;
        };
        u16 reg66;
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

    union
    {
        struct
        {
            u16 reg_bgb_op_type : 1;
            u16 : 15;
        };
        u16 reg69;
    };

    union
    {
        struct
        {
            u16 reg_bgb_infmt : 2;
            u16 : 6;
            u16 reg_bgb_outfmt : 2;
            u16 : 6;
        };
        u16 reg6a;
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
            u16 reg_bgb_blur_alpha : 8;
            u16 : 8;
        };
        u16 reg6d;
    };

    union
    {
        struct
        {
            u16 reg_bgb_g1_resize_area_xcoef_low : 16;
        };
        u16 reg6e;
    };

    union
    {
        struct
        {
            u16 reg_bgb_g1_resize_area_xcoef_high : 2;
            u16 : 14;
        };
        u16 reg6f;
    };

    union
    {
        struct
        {
            u16 reg_bgb_g1_resize_area_ycoef_low : 16;
        };
        u16 reg70;
    };

    union
    {
        struct
        {
            u16 reg_bgb_g1_resize_area_ycoef_high : 2;
            u16 : 14;
        };
        u16 reg71;
    };

    union
    {
        struct
        {
            u16 reg_bgb_g3_resize_ratio_x_low : 16;
        };
        u16 reg72;
    };

    union
    {
        struct
        {
            u16 reg_bgb_g3_resize_ratio_x_high : 6;
            u16 : 10;
        };
        u16 reg73;
    };

    union
    {
        struct
        {
            u16 reg_bgb_g3_resize_ratio_y_low : 16;
        };
        u16 reg74;
    };

    union
    {
        struct
        {
            u16 reg_bgb_g3_resize_ratio_y_high : 6;
            u16 : 10;
        };
        u16 reg75;
    };

    union
    {
        struct
        {
            u16 reg_bgb_g4_resize0_ratio_x_low : 16;
        };
        u16 reg76;
    };

    union
    {
        struct
        {
            u16 reg_bgb_g4_resize0_ratio_x_high : 6;
            u16 : 10;
        };
        u16 reg77;
    };

    union
    {
        struct
        {
            u16 reg_bgb_g4_resize0_ratio_y_low : 16;
        };
        u16 reg78;
    };

    union
    {
        struct
        {
            u16 reg_bgb_g4_resize0_ratio_y_high : 6;
            u16 : 10;
        };
        u16 reg79;
    };

    union
    {
        struct
        {
            u16 reg_bgb_g4_resize1_ratio_x_low : 16;
        };
        u16 reg7a;
    };

    union
    {
        struct
        {
            u16 reg_bgb_g4_resize1_ratio_x_high : 6;
            u16 : 10;
        };
        u16 reg7b;
    };

    union
    {
        struct
        {
            u16 reg_bgb_g4_resize1_ratio_y_low : 16;
        };
        u16 reg7c;
    };

    union
    {
        struct
        {
            u16 reg_bgb_g4_resize1_ratio_y_high : 6;
            u16 : 10;
        };
        u16 reg7d;
    };

    union
    {
        struct
        {
            u16 reg_bgb_g4_uv_resize_ratio_y_low : 16;
        };
        u16 reg7e;
    };

    union
    {
        struct
        {
            u16 reg_bgb_g4_uv_resize_ratio_y_high : 6;
            u16 : 10;
        };
        u16 reg7f;
    };
} ive_hal_reg_bank3;
#endif // _HAL_IVE_REG_H_
