/*
 * hal_ive.c- Sigmastar
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
#include "hal_ive.h"

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                                                                  //
//                                                                 IRQ API //
//                                                                                                                                                  //
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*******************************************************************************************************************
 * ive_hal_init
 *   init IVE HAL layer
 *
 * Parameters:
 *   handle: IVE HAL handle
 *   base_addr0: base address 0
 *   base_addr1: base address 1
 *
 * Return:
 *   none
 */
void ive_hal_init(ive_hal_handle *handle, ss_phys_addr_t *u64BankBase)
{
    memset(handle, 0, sizeof(ive_hal_handle));
    handle->base_addr0 = u64BankBase[0];
    handle->base_addr1 = u64BankBase[1];
    handle->base_addr3 = u64BankBase[2];
}

/*******************************************************************************************************************
 * ive_hal_set_irq_mask
 *   Set interrupt trigger mask
 *
 * Parameters:
 *   handle: IVE HAL handle
 *   mask: Bitwize mask, defined in IveHalIrqEvent_e
 *
 * Return:
 *   none
 */
void ive_hal_set_irq_mask(ive_hal_handle *handle, IVE_HAL_IRQ_MASK mask)
{
    handle->reg_bank0.irq_mask = ~mask;
    REGW(handle->base_addr0, 0x10, handle->reg_bank0.reg10);
}

/*******************************************************************************************************************
 * ive_hal_set_bgblur_irq_mask
 *   Set interrupt trigger mask of BGBlur
 *
 * Parameters:
 *   handle: IVE HAL handle
 *   mask: Bitwize mask, defined in IveHalIrqEvent_e
 *
 * Return:
 *   none
 */
void ive_hal_set_bgblur_irq_mask(ive_hal_handle *handle, IVE_HAL_IRQ_MASK mask)
{
    handle->reg_bank3.reg_bgb_irq_mask = ~mask;
    REGW(handle->base_addr3, 0x02, handle->reg_bank3.reg02);
}

/*******************************************************************************************************************
 * ive_hal_clear_irq
 *   Clear triggered interrupt
 *
 * Parameters:
 *   handle: IVE HAL handle
 *   mask: Bitwize mask, defined in IveHalIrqEvent_e
 *
 * Return:
 *   none
 */
void ive_hal_clear_irq(ive_hal_handle *handle, IVE_HAL_IRQ_MASK mask)
{
    handle->reg_bank0.woc_irq_clr = mask;
    REGW(handle->base_addr0, 0x15, handle->reg_bank0.reg15);
}

/*******************************************************************************************************************
 * ive_hal_clear_bgblur_irq
 *   Clear triggered interrupt of BGBlur.
 *
 * Parameters:
 *   handle: IVE HAL handle
 *   mask: Bitwize mask, defined in IveHalIrqEvent_e
 *
 * Return:
 *   none
 */
void ive_hal_clear_bgblur_irq(ive_hal_handle *handle, IVE_HAL_IRQ_MASK mask)
{
    handle->reg_bank3.reg_bgb_woc_irq_clr = mask;
    REGW(handle->base_addr3, 0x07, handle->reg_bank3.reg07);
}

/*******************************************************************************************************************
 * ive_hal_get_irq_check
 *   Check current IRQ status
 *
 * Parameters:
 *   handle: IVE HAL handle
 *   mask: Bitwise mask to be checked
 *
 * Return:
 *   Checked result, bitwise
 */
IVE_HAL_IRQ_MASK ive_hal_get_irq_check(ive_hal_handle *handle, IVE_HAL_IRQ_MASK mask)
{
    handle->reg_bank0.irq_final_status = REGR(handle->base_addr0, 0x13);

    return handle->reg_bank0.irq_final_status & mask;
}

/*******************************************************************************************************************
 * ive_hal_get_bgblur_irq_check
 *   Check current BGBlur IRQ status
 *
 * Parameters:
 *   handle: IVE HAL handle
 *   mask: Bitwise mask to be checked
 *
 * Return:
 *   Checked result, bitwise
 */
IVE_HAL_IRQ_MASK ive_hal_get_bgblur_irq_check(ive_hal_handle *handle, IVE_HAL_IRQ_MASK mask)
{
    handle->reg_bank3.reg_bgb_irq_final_status = REGR(handle->base_addr3, 0x05);

    return handle->reg_bank3.reg_bgb_irq_final_status & mask;
}

/*******************************************************************************************************************
 * ive_hal_get_irq
 *   Get current interrupt trigger status
 *
 * Parameters:
 *   handle: IVE HAL handle
 *   mask: Bitwize mask, defined in IveHalIrqEvent_e
 *
 * Return:
 *   Bitwize status
 */
u16 ive_hal_get_irq(ive_hal_handle *handle)
{
    handle->reg_bank0.irq_final_status = REGR(handle->base_addr0, 0x13);

    return handle->reg_bank0.irq_final_status;
}

/*******************************************************************************************************************
 * ive_hal_get_bgblur_irq
 *   Get current interrupt trigger status of BGBlur
 *
 * Parameters:
 *   handle: IVE HAL handle
 *   mask: Bitwize mask, defined in IveHalIrqEvent_e
 *
 * Return:
 *   Bitwize status
 */
u16 ive_hal_get_bgblur_irq(ive_hal_handle *handle)
{
    handle->reg_bank3.reg_bgb_irq_final_status = REGR(handle->base_addr3, 0x05);

    return handle->reg_bank3.reg_bgb_irq_final_status;
}

/*******************************************************************************************************************
 * ive_hal_set_operation
 *   Set operation of IVE HW
 *
 * Parameters:
 *   handle: IVE HAL handle
 *   op_type: operation type
 *   op_mode: operation mode
 *
 * Return:
 *   none
 */
void ive_hal_set_operation(ive_hal_handle *handle, IVE_IOC_OP_TYPE op_type)
{
    handle->reg_bank1.op_type = op_type;

    REGW(handle->base_addr1, 0x04, handle->reg_bank1.reg04);
}

/*******************************************************************************************************************
 * ive_hal_set_read_outstanding
 *   Set read outstanding
 *
 * Parameters:
 *   handle: IVE HAL handle
 *
 * Return:
 *   none
 */
void ive_hal_set_read_outstanding(ive_hal_handle *handle, IVE_HAL_READ_OUTSTANDING read_outstanding)
{
    handle->reg_bank0.reg_axi_ar_max_outstanding = read_outstanding;
    REGW(handle->base_addr0, 0x4f, handle->reg_bank0.reg4f);
}

/*******************************************************************************************************************
 * ive_hal_set_mcm_busy_ctrl
 *   Set mcm busy ctrl on/off
 *
 * Parameters:
 *   handle:   IVE HAL handle
 *   u8Enable: on/off switch
 *
 * Return:
 *   none
 */
void ive_hal_mcm_busy_on_off(ive_hal_handle *handle, u8 u8Enable)
{
    // dummy[0] 0: disable ive_busy MCM control.
    //          1: enable ive_busy MCM control.
    handle->reg_bank0.dummy_mcm_busy_ctrl = u8Enable;

    REGW(handle->base_addr0, 0x42, handle->reg_bank0.reg42);
}

/*******************************************************************************************************************
 * ive_hal_bgblur_mcm_busy_on_off
 *   Set bgblur mcm busy ctrl on/off
 *
 * Parameters:
 *   handle:   IVE HAL handle
 *   u8Enable: on/off switch
 *
 * Return:
 *   none
 */
void ive_hal_bgblur_mcm_busy_on_off(ive_hal_handle *handle, u8 u8Enable)
{
    // dummy[0] 0: disable bgb_busy MCM control.
    //          1: enable bgb_busy MCM control.
    handle->reg_bank3.reg_bgb_dummy_mcm_busy_ctrl = u8Enable;

    REGW(handle->base_addr3, 0x13, handle->reg_bank3.reg13);
}

/*******************************************************************************************************************
 * ive_hal_padding_mode_set
 *   Set padding mode value
 *
 * Parameters:
 *   handle: IVE HAL handle
 *
 * Return:
 *   none
 */
void ive_hal_padding_mode_set(ive_hal_handle *handle, int padding_mode)
{
    handle->reg_bank1.padding_mode = padding_mode;

    REGW(handle->base_addr1, 0x03, handle->reg_bank1.reg03);
}

/*******************************************************************************************************************
 * ive_hal_set_images
 *   Set input & output image
 *
 * Parameters:
 *   handle: IVE HAL handle
 *   input: input image
 *   output: output image
 *
 * Return:
 *   none
 */
void ive_hal_set_images(ive_hal_handle *handle, ive_ioc_config *config)
{
    u64 input_addr0, input_addr1, input_addr2, output_addr0, output_addr1, output_addr2;

    IVE_MSG(IVE_MSG_DBG, "%s\n", __FUNCTION__);

    input_addr0  = config->input.address[0];
    input_addr1  = config->input.address[1];
    input_addr2  = config->input.address[2];
    output_addr0 = config->output.address[0];
    output_addr1 = config->output.address[1];
    output_addr2 = config->output.address[2];

    handle->reg_bank1.infmt  = config->input.format;
    handle->reg_bank1.outfmt = config->output.format;

    handle->reg_bank1.frame_width  = config->input.width - 1;
    handle->reg_bank1.frame_height = config->input.height - 1;

    handle->reg_bank1.src1_addr_low           = ADDR_LOW(input_addr0);
    handle->reg_bank1.src1_addr_high          = ADDR_HIGH(input_addr0);
    handle->reg_bank1.reg_ive_src1_addr_35_32 = ADDR_HIGH_EXT(input_addr0);
    handle->reg_bank1.src2_addr_low           = ADDR_LOW(input_addr1);
    handle->reg_bank1.src2_addr_high          = ADDR_HIGH(input_addr1);
    handle->reg_bank1.reg_ive_src2_addr_35_32 = ADDR_HIGH_EXT(input_addr1);
    handle->reg_bank1.src3_addr_low           = ADDR_LOW(input_addr2);
    handle->reg_bank1.src3_addr_high          = ADDR_HIGH(input_addr2);
    handle->reg_bank1.reg_ive_src3_addr_35_32 = ADDR_HIGH_EXT(input_addr2);
    handle->reg_bank1.src1_stride             = config->input.stride[0] - 1;
    handle->reg_bank1.src2_stride             = config->input.stride[1] - 1;
    handle->reg_bank1.src3_stride             = config->input.stride[2] - 1;

    handle->reg_bank1.dst1_addr_low           = ADDR_LOW(output_addr0);
    handle->reg_bank1.dst1_addr_high          = ADDR_HIGH(output_addr0);
    handle->reg_bank1.reg_ive_dst1_addr_35_32 = ADDR_HIGH_EXT(output_addr0);
    handle->reg_bank1.dst2_addr_low           = ADDR_LOW(output_addr1);
    handle->reg_bank1.dst2_addr_high          = ADDR_HIGH(output_addr1);
    handle->reg_bank1.reg_ive_dst2_addr_35_32 = ADDR_HIGH_EXT(output_addr1);
    handle->reg_bank1.dst3_addr_low           = ADDR_LOW(output_addr2);
    handle->reg_bank1.dst3_addr_high          = ADDR_HIGH(output_addr2);
    handle->reg_bank1.reg_ive_dst3_addr_35_32 = ADDR_HIGH_EXT(output_addr2);
    handle->reg_bank1.dst1_stride             = config->output.stride[0] - 1;
    handle->reg_bank1.dst2_stride             = config->output.stride[1] - 1;
    handle->reg_bank1.dst3_stride             = config->output.stride[2] - 1;

    REGW(handle->base_addr1, 0x05, handle->reg_bank1.reg05);
    REGW(handle->base_addr1, 0x06, handle->reg_bank1.reg06);
    REGW(handle->base_addr1, 0x07, handle->reg_bank1.reg07);
    REGW(handle->base_addr1, 0x08, handle->reg_bank1.reg08);
    REGW(handle->base_addr1, 0x09, handle->reg_bank1.reg09);
    REGW(handle->base_addr1, 0x0A, handle->reg_bank1.reg0A);
    REGW(handle->base_addr1, 0x0B, handle->reg_bank1.reg0B);
    REGW(handle->base_addr1, 0x0C, handle->reg_bank1.reg0C);
    REGW(handle->base_addr1, 0x0D, handle->reg_bank1.reg0D);
    REGW(handle->base_addr1, 0x0E, handle->reg_bank1.reg0E);
    REGW(handle->base_addr1, 0x0F, handle->reg_bank1.reg0F);
    REGW(handle->base_addr1, 0x10, handle->reg_bank1.reg10);
    REGW(handle->base_addr1, 0x11, handle->reg_bank1.reg11);
    REGW(handle->base_addr1, 0x12, handle->reg_bank1.reg12);
    REGW(handle->base_addr1, 0x13, handle->reg_bank1.reg13);
    REGW(handle->base_addr1, 0x14, handle->reg_bank1.reg14);
    REGW(handle->base_addr1, 0x15, handle->reg_bank1.reg15);
    REGW(handle->base_addr1, 0x16, handle->reg_bank1.reg16);
    REGW(handle->base_addr1, 0x17, handle->reg_bank1.reg17);
    REGW(handle->base_addr1, 0x18, handle->reg_bank1.reg18);
    REGW(handle->base_addr1, 0x19, handle->reg_bank1.reg19);
    REGW(handle->base_addr1, 0x46, handle->reg_bank1.reg46);
    REGW(handle->base_addr1, 0x47, handle->reg_bank1.reg47);
    REGW(handle->base_addr1, 0x48, handle->reg_bank1.reg48);
    REGW(handle->base_addr1, 0x49, handle->reg_bank1.reg49);
    REGW(handle->base_addr1, 0x4a, handle->reg_bank1.reg4a);
    REGW(handle->base_addr1, 0x4b, handle->reg_bank1.reg4b);
}

/*******************************************************************************************************************
 * ive_hal_set_bgblur_images
 *   Set input & output image of BGB mode.
 *
 * Parameters:
 *   handle: IVE HAL handle
 *   coeff_bgblur: BGBlur params.
 *
 * Return:
 *   none
 */
void ive_hal_set_bgblur_images(ive_hal_handle *handle, ive_ioc_config *config)
{
    u64 u64MaskC0Addr = 0, u64MaskC1Addr = 0, u64SoriC0Addr = 0, u64SoriC1Addr = 0;
    u64 u64MidC0Addr = 0, u64MidC1Addr = 0;
    u64 u64OriC0Addr = 0, u64OriC1Addr = 0, u64DstC0Addr = 0, u64DstC1Addr = 0;

    IVE_MSG(IVE_MSG_DBG, "%s\n", __FUNCTION__);

    u64MaskC0Addr                               = config->coeff_bgblur.stSrcYMask.address[0];
    u64MaskC1Addr                               = config->coeff_bgblur.stSrcUvMask.address[0];
    handle->reg_bank3.reg_bgb_src0_c0_addr_low  = ADDR_LOW(u64MaskC0Addr);
    handle->reg_bank3.reg_bgb_src0_c0_addr_high = ADDR_HIGH(u64MaskC0Addr);
    handle->reg_bank3.reg_bgb_src0_c0_addr_h    = ADDR_HIGH_EXT(u64MaskC0Addr);
    handle->reg_bank3.reg_bgb_src0_c0_stride    = ALIGN_UP(4, config->coeff_bgblur.stSrcYMask.stride[0] - 1);
    handle->reg_bank3.reg_bgb_src0_c1_addr_low  = ADDR_LOW(u64MaskC1Addr);
    handle->reg_bank3.reg_bgb_src0_c1_addr_high = ADDR_HIGH(u64MaskC1Addr);
    handle->reg_bank3.reg_bgb_src0_c1_addr_h    = ADDR_HIGH_EXT(u64MaskC1Addr);
    handle->reg_bank3.reg_bgb_src0_c1_stride =
        ALIGN_UP(4, config->coeff_bgblur.stSrcYMask.stride[0] - 1); // Uv mask image use the stride of Y mask.

    u64OriC0Addr                                = config->coeff_bgblur.stSrcOri.address[0];
    u64OriC1Addr                                = config->coeff_bgblur.stSrcOri.address[1];
    handle->reg_bank3.reg_bgb_src2_c0_addr_low  = ADDR_LOW(u64OriC0Addr);
    handle->reg_bank3.reg_bgb_src2_c0_addr_high = ADDR_HIGH(u64OriC0Addr);
    handle->reg_bank3.reg_bgb_src2_c0_addr_h    = ADDR_HIGH_EXT(u64OriC0Addr);
    handle->reg_bank3.reg_bgb_src2_c0_stride    = ALIGN_UP(4, config->coeff_bgblur.stSrcOri.stride[0] - 1);
    handle->reg_bank3.reg_bgb_src2_c1_addr_low  = ADDR_LOW(u64OriC1Addr);
    handle->reg_bank3.reg_bgb_src2_c1_addr_high = ADDR_HIGH(u64OriC1Addr);
    handle->reg_bank3.reg_bgb_src2_c1_addr_h    = ADDR_HIGH_EXT(u64OriC1Addr);
    handle->reg_bank3.reg_bgb_src2_c1_stride    = ALIGN_UP(4, config->coeff_bgblur.stSrcOri.stride[1] - 1);

    u64SoriC0Addr                               = config->coeff_bgblur.stSrcRepBg.address[0];
    u64SoriC1Addr                               = config->coeff_bgblur.stSrcRepBg.address[1];
    handle->reg_bank3.reg_bgb_src1_c0_addr_low  = ADDR_LOW(u64SoriC0Addr);
    handle->reg_bank3.reg_bgb_src1_c0_addr_high = ADDR_HIGH(u64SoriC0Addr);
    handle->reg_bank3.reg_bgb_src1_c0_addr_h    = ADDR_HIGH_EXT(u64SoriC0Addr);
    handle->reg_bank3.reg_bgb_src1_c0_stride    = ALIGN_UP(4, config->coeff_bgblur.stSrcRepBg.stride[0] - 1);
    handle->reg_bank3.reg_bgb_src1_c1_addr_low  = ADDR_LOW(u64SoriC1Addr);
    handle->reg_bank3.reg_bgb_src1_c1_addr_high = ADDR_HIGH(u64SoriC1Addr);
    handle->reg_bank3.reg_bgb_src1_c1_addr_h    = ADDR_HIGH_EXT(u64SoriC1Addr);
    handle->reg_bank3.reg_bgb_src1_c1_stride    = ALIGN_UP(4, config->coeff_bgblur.stSrcRepBg.stride[1] - 1);

    u64MidC0Addr                               = config->coeff_bgblur.stMidImg.address[0];
    u64MidC1Addr                               = config->coeff_bgblur.stMidImg.address[1];
    handle->reg_bank3.reg_bgb_mid_c0_addr_low  = ADDR_LOW(u64MidC0Addr);
    handle->reg_bank3.reg_bgb_mid_c0_addr_high = ADDR_HIGH(u64MidC0Addr);
    handle->reg_bank3.reg_bgb_mid_c0_addr_h    = ADDR_HIGH_EXT(u64MidC0Addr);
    handle->reg_bank3.reg_bgb_mid_c0_stride    = ALIGN_UP(4, config->coeff_bgblur.stMidImg.stride[0] - 1);
    handle->reg_bank3.reg_bgb_mid_c1_addr_low  = ADDR_LOW(u64MidC1Addr);
    handle->reg_bank3.reg_bgb_mid_c1_addr_high = ADDR_HIGH(u64MidC1Addr);
    handle->reg_bank3.reg_bgb_mid_c1_addr_h    = ADDR_HIGH_EXT(u64MidC1Addr);
    handle->reg_bank3.reg_bgb_mid_c1_stride    = ALIGN_UP(4, config->coeff_bgblur.stMidImg.stride[1] - 1);

    u64DstC0Addr                               = config->output.address[0];
    u64DstC1Addr                               = config->output.address[1];
    handle->reg_bank3.reg_bgb_dst_c0_addr_low  = ADDR_LOW(u64DstC0Addr);
    handle->reg_bank3.reg_bgb_dst_c0_addr_high = ADDR_HIGH(u64DstC0Addr);
    handle->reg_bank3.reg_bgb_dst_c0_addr_h    = ADDR_HIGH_EXT(u64DstC0Addr);
    handle->reg_bank3.reg_bgb_dst_c0_stride    = ALIGN_UP(4, config->output.stride[0] - 1);
    handle->reg_bank3.reg_bgb_dst_c1_addr_low  = ADDR_LOW(u64DstC1Addr);
    handle->reg_bank3.reg_bgb_dst_c1_addr_high = ADDR_HIGH(u64DstC1Addr);
    handle->reg_bank3.reg_bgb_dst_c1_addr_h    = ADDR_HIGH_EXT(u64DstC1Addr);
    handle->reg_bank3.reg_bgb_dst_c1_stride    = ALIGN_UP(4, config->output.stride[1] - 1);

    if (IVE_IOC_MODE_BGBLUR_MOSAIC != config->coeff_bgblur.eBgBlurMode)
    {
        handle->reg_bank3.reg_bgb_img1_width  = config->coeff_bgblur.u16DownWidth - 1;
        handle->reg_bank3.reg_bgb_img1_height = config->coeff_bgblur.u16DownHeight - 1;
    }

    handle->reg_bank3.reg_bgb_img0_width  = config->coeff_bgblur.stSrcYMask.width - 1;
    handle->reg_bank3.reg_bgb_img0_height = config->coeff_bgblur.stSrcYMask.height - 1;
    handle->reg_bank3.reg_bgb_img3_width  = config->coeff_bgblur.stSrcOri.width - 1;
    handle->reg_bank3.reg_bgb_img3_height = config->coeff_bgblur.stSrcOri.height - 1;

    REGW(handle->base_addr3, 0x19, handle->reg_bank3.reg19);
    REGW(handle->base_addr3, 0x1a, handle->reg_bank3.reg1a);
    REGW(handle->base_addr3, 0x1b, handle->reg_bank3.reg1b);
    REGW(handle->base_addr3, 0x1c, handle->reg_bank3.reg1c);
    REGW(handle->base_addr3, 0x1d, handle->reg_bank3.reg1d);
    REGW(handle->base_addr3, 0x1e, handle->reg_bank3.reg1e);
    REGW(handle->base_addr3, 0x1f, handle->reg_bank3.reg1f);
    REGW(handle->base_addr3, 0x20, handle->reg_bank3.reg20);
    REGW(handle->base_addr3, 0x21, handle->reg_bank3.reg21);
    REGW(handle->base_addr3, 0x22, handle->reg_bank3.reg22);
    REGW(handle->base_addr3, 0x23, handle->reg_bank3.reg23);
    REGW(handle->base_addr3, 0x24, handle->reg_bank3.reg24);
    REGW(handle->base_addr3, 0x25, handle->reg_bank3.reg25);
    REGW(handle->base_addr3, 0x26, handle->reg_bank3.reg26);
    REGW(handle->base_addr3, 0x27, handle->reg_bank3.reg27);
    REGW(handle->base_addr3, 0x28, handle->reg_bank3.reg28);
    REGW(handle->base_addr3, 0x29, handle->reg_bank3.reg29);
    REGW(handle->base_addr3, 0x2a, handle->reg_bank3.reg2a);
    REGW(handle->base_addr3, 0x2b, handle->reg_bank3.reg2b);
    REGW(handle->base_addr3, 0x2c, handle->reg_bank3.reg2c);
    REGW(handle->base_addr3, 0x2d, handle->reg_bank3.reg2d);
    REGW(handle->base_addr3, 0x2e, handle->reg_bank3.reg2e);
    REGW(handle->base_addr3, 0x2f, handle->reg_bank3.reg2f);
    REGW(handle->base_addr3, 0x30, handle->reg_bank3.reg30);
    REGW(handle->base_addr3, 0x39, handle->reg_bank3.reg39);
    REGW(handle->base_addr3, 0x3a, handle->reg_bank3.reg3a);
    REGW(handle->base_addr3, 0x3b, handle->reg_bank3.reg3b);
    REGW(handle->base_addr3, 0x3c, handle->reg_bank3.reg3c);
    REGW(handle->base_addr3, 0x3d, handle->reg_bank3.reg3d);
    REGW(handle->base_addr3, 0x3e, handle->reg_bank3.reg3e);
    REGW(handle->base_addr3, 0x3f, handle->reg_bank3.reg3f);
    REGW(handle->base_addr3, 0x40, handle->reg_bank3.reg40);
    REGW(handle->base_addr3, 0x59, handle->reg_bank3.reg59);
    REGW(handle->base_addr3, 0x5a, handle->reg_bank3.reg5a);
    REGW(handle->base_addr3, 0x5b, handle->reg_bank3.reg5b);
    REGW(handle->base_addr3, 0x5c, handle->reg_bank3.reg5c);
    REGW(handle->base_addr3, 0x5d, handle->reg_bank3.reg5d);
    REGW(handle->base_addr3, 0x5e, handle->reg_bank3.reg5e);
    REGW(handle->base_addr3, 0x5f, handle->reg_bank3.reg5f);
    REGW(handle->base_addr3, 0x60, handle->reg_bank3.reg60);
    REGW(handle->base_addr3, 0x61, handle->reg_bank3.reg61);
    REGW(handle->base_addr3, 0x62, handle->reg_bank3.reg62);
    REGW(handle->base_addr3, 0x63, handle->reg_bank3.reg63);
    REGW(handle->base_addr3, 0x64, handle->reg_bank3.reg64);
    REGW(handle->base_addr3, 0x67, handle->reg_bank3.reg67);
    REGW(handle->base_addr3, 0x68, handle->reg_bank3.reg68);
}

/*******************************************************************************************************************
 * _ive_hal_set_coeff_mask
 *   Set mask coefficient
 *
 * Parameters:
 *   handle: IVE HAL handle
 *   mask: mask coefficient
 *
 * Return:
 *   none
 */
static void _ive_hal_set_coeff_mask(ive_hal_handle *handle, u8 *mask, u8 shift)
{
    handle->reg_bank1.mask0  = mask[0];
    handle->reg_bank1.mask1  = mask[1];
    handle->reg_bank1.mask2  = mask[2];
    handle->reg_bank1.mask3  = mask[3];
    handle->reg_bank1.mask4  = mask[4];
    handle->reg_bank1.mask5  = mask[5];
    handle->reg_bank1.mask6  = mask[6];
    handle->reg_bank1.mask7  = mask[7];
    handle->reg_bank1.mask8  = mask[8];
    handle->reg_bank1.mask9  = mask[9];
    handle->reg_bank1.mask10 = mask[10];
    handle->reg_bank1.mask11 = mask[11];
    handle->reg_bank1.mask12 = mask[12];
    handle->reg_bank1.mask13 = mask[13];
    handle->reg_bank1.mask14 = mask[14];
    handle->reg_bank1.mask15 = mask[15];
    handle->reg_bank1.mask16 = mask[16];
    handle->reg_bank1.mask17 = mask[17];
    handle->reg_bank1.mask18 = mask[18];
    handle->reg_bank1.mask19 = mask[19];
    handle->reg_bank1.mask20 = mask[20];
    handle->reg_bank1.mask21 = mask[21];
    handle->reg_bank1.mask22 = mask[22];
    handle->reg_bank1.mask23 = mask[23];
    handle->reg_bank1.mask24 = mask[24];
    handle->reg_bank1.shift  = shift;

    REGW(handle->base_addr1, 0x1A, handle->reg_bank1.reg1A);
    REGW(handle->base_addr1, 0x1B, handle->reg_bank1.reg1B);
    REGW(handle->base_addr1, 0x1C, handle->reg_bank1.reg1C);
    REGW(handle->base_addr1, 0x1D, handle->reg_bank1.reg1D);
    REGW(handle->base_addr1, 0x1E, handle->reg_bank1.reg1E);
    REGW(handle->base_addr1, 0x1F, handle->reg_bank1.reg1F);
    REGW(handle->base_addr1, 0x20, handle->reg_bank1.reg20);
    REGW(handle->base_addr1, 0x21, handle->reg_bank1.reg21);
    REGW(handle->base_addr1, 0x22, handle->reg_bank1.reg22);
    REGW(handle->base_addr1, 0x23, handle->reg_bank1.reg23);
    REGW(handle->base_addr1, 0x24, handle->reg_bank1.reg24);
    REGW(handle->base_addr1, 0x25, handle->reg_bank1.reg25);
    REGW(handle->base_addr1, 0x26, handle->reg_bank1.reg26);
}

/*******************************************************************************************************************
 * _ive_hal_set_coeff_filter
 *   Set filter coefficient
 *
 * Parameters:
 *   handle: IVE HAL handle
 *   coeff: coefficient
 *
 * Return:
 *   none
 */
static void _ive_hal_set_coeff_filter(ive_hal_handle *handle, ive_ioc_coeff_filter *coeff)
{
    _ive_hal_set_coeff_mask(handle, coeff->mask, coeff->shift);
}

/*******************************************************************************************************************
 * _ive_hal_set_coeff_csc
 *   Set CSC coefficient
 *
 * Parameters:
 *   handle: IVE HAL handle
 *   coeff: CSC coefficient
 *
 * Return:
 *   none
 */
static void _ive_hal_set_coeff_csc(ive_hal_handle *handle, ive_ioc_coeff_csc *coeff)
{
    handle->reg_bank1.csc_coeff0 = coeff->coeff[0];
    handle->reg_bank1.csc_coeff1 = coeff->coeff[1];
    handle->reg_bank1.csc_coeff2 = coeff->coeff[2];
    handle->reg_bank1.csc_coeff3 = coeff->coeff[3];
    handle->reg_bank1.csc_coeff4 = coeff->coeff[4];
    handle->reg_bank1.csc_coeff5 = coeff->coeff[5];
    handle->reg_bank1.csc_coeff6 = coeff->coeff[6];
    handle->reg_bank1.csc_coeff7 = coeff->coeff[7];
    handle->reg_bank1.csc_coeff8 = coeff->coeff[8];

    handle->reg_bank1.csc_offset0 = coeff->offset[0];
    handle->reg_bank1.csc_offset1 = coeff->offset[1];
    handle->reg_bank1.csc_offset2 = coeff->offset[2];

    handle->reg_bank1.csc_clamp0_low  = coeff->clamp[0].clamp_low;
    handle->reg_bank1.csc_clamp0_high = coeff->clamp[0].clamp_high;
    handle->reg_bank1.csc_clamp1_low  = coeff->clamp[1].clamp_low;
    handle->reg_bank1.csc_clamp1_high = coeff->clamp[1].clamp_high;
    handle->reg_bank1.csc_clamp2_low  = coeff->clamp[2].clamp_low;
    handle->reg_bank1.csc_clamp2_high = coeff->clamp[2].clamp_high;

    REGW(handle->base_addr1, 0x30, handle->reg_bank1.reg30);
    REGW(handle->base_addr1, 0x31, handle->reg_bank1.reg31);
    REGW(handle->base_addr1, 0x32, handle->reg_bank1.reg32);
    REGW(handle->base_addr1, 0x33, handle->reg_bank1.reg33);
    REGW(handle->base_addr1, 0x34, handle->reg_bank1.reg34);
    REGW(handle->base_addr1, 0x35, handle->reg_bank1.reg35);
    REGW(handle->base_addr1, 0x36, handle->reg_bank1.reg36);
    REGW(handle->base_addr1, 0x37, handle->reg_bank1.reg37);
    REGW(handle->base_addr1, 0x38, handle->reg_bank1.reg38);
    REGW(handle->base_addr1, 0x39, handle->reg_bank1.reg39);
    REGW(handle->base_addr1, 0x3A, handle->reg_bank1.reg3A);
    REGW(handle->base_addr1, 0x3B, handle->reg_bank1.reg3B);
    REGW(handle->base_addr1, 0x3C, handle->reg_bank1.reg3C);
    REGW(handle->base_addr1, 0x3D, handle->reg_bank1.reg3D);
    REGW(handle->base_addr1, 0x3E, handle->reg_bank1.reg3E);
}

/*******************************************************************************************************************
 * _ive_hal_set_coeff_sobel
 *   Set sobel coefficient
 *
 * Parameters:
 *   handle: IVE HAL handle
 *   coeff: coefficient
 *
 * Return:
 *   none
 */
static void _ive_hal_set_coeff_sobel(ive_hal_handle *handle, ive_ioc_coeff_sobel *coeff)
{
    _ive_hal_set_coeff_mask(handle, coeff->mask, 0);

    handle->reg_bank1.outfmt = coeff->mode;
    REGW(handle->base_addr1, 0x05, handle->reg_bank1.reg05);
}

/*******************************************************************************************************************
 * _ive_hal_set_coeff_mag_and_ang
 *   Set mag and ang coefficient
 *
 * Parameters:
 *   handle: IVE HAL handle
 *   coeff: coefficient
 *
 * Return:
 *   none
 */
static void _ive_hal_set_coeff_mag_and_ang(ive_hal_handle *handle, ive_ioc_coeff_mag_and_ang *coeff)
{
    _ive_hal_set_coeff_mask(handle, coeff->mask, 0);

    handle->reg_bank1.outfmt         = coeff->mode;
    handle->reg_bank1.thresh_16bit_1 = coeff->thresh;

    REGW(handle->base_addr1, 0x05, handle->reg_bank1.reg05);
    REGW(handle->base_addr1, 0x28, handle->reg_bank1.reg28);
}

/*******************************************************************************************************************
 * _ive_hal_set_coeff_ord_stat_filter
 *   Set order statistics filter coefficient
 *
 * Parameters:
 *   handle: IVE HAL handle
 *   coeff: coefficient
 *
 * Return:
 *   none
 */
static void _ive_hal_set_coeff_ord_stat_filter(ive_hal_handle *handle, ive_ioc_coeff_ord_stat_filter *coeff)
{
    handle->reg_bank1.op_mode = coeff->mode;

    REGW(handle->base_addr1, 0x04, handle->reg_bank1.reg04);
}

/*******************************************************************************************************************
 * _ive_hal_set_coeff_bernsen
 *   Set bernsen coefficient
 *
 * Parameters:
 *   handle: IVE HAL handle
 *   coeff: coefficient
 *
 * Return:
 *   none
 */
static void _ive_hal_set_coeff_bernsen(ive_hal_handle *handle, ive_ioc_coeff_bernsen *coeff)
{
    handle->reg_bank1.op_mode        = coeff->mode;
    handle->reg_bank1.thresh_16bit_1 = coeff->thresh;

    REGW(handle->base_addr1, 0x04, handle->reg_bank1.reg04);
    REGW(handle->base_addr1, 0x28, handle->reg_bank1.reg28);
}

/*******************************************************************************************************************
 * _ive_hal_set_coeff_dilate
 *   Set dilate coefficient
 *
 * Parameters:
 *   handle: IVE HAL handle
 *   coeff: coefficient
 *
 * Return:
 *   none
 */
static void _ive_hal_set_coeff_dilate(ive_hal_handle *handle, ive_ioc_coeff_dilate *coeff)
{
    _ive_hal_set_coeff_mask(handle, coeff->mask, 0);
}

/*******************************************************************************************************************
 * _ive_hal_set_coeff_erode
 *   Set erode coefficient
 *
 * Parameters:
 *   handle: IVE HAL handle
 *   coeff: coefficient
 *
 * Return:
 *   none
 */
static void _ive_hal_set_coeff_erode(ive_hal_handle *handle, ive_ioc_coeff_erode *coeff)
{
    _ive_hal_set_coeff_mask(handle, coeff->mask, 0);
}

/*******************************************************************************************************************
 * _ive_hal_set_coeff_thresh
 *   Set thresh coefficient
 *
 * Parameters:
 *   handle: IVE HAL handle
 *   coeff: coefficient
 *
 * Return:
 *   none
 */
static void _ive_hal_set_coeff_thresh(ive_hal_handle *handle, ive_ioc_coeff_thresh *coeff)
{
    handle->reg_bank1.op_mode        = coeff->mode;
    handle->reg_bank1.mask0          = coeff->min;
    handle->reg_bank1.mask1          = coeff->mid;
    handle->reg_bank1.mask2          = coeff->max;
    handle->reg_bank1.thresh_16bit_1 = coeff->low;
    handle->reg_bank1.thresh_16bit_2 = coeff->high;

    REGW(handle->base_addr1, 0x04, handle->reg_bank1.reg04);
    REGW(handle->base_addr1, 0x1A, handle->reg_bank1.reg1A);
    REGW(handle->base_addr1, 0x1B, handle->reg_bank1.reg1B);
    REGW(handle->base_addr1, 0x28, handle->reg_bank1.reg28);
    REGW(handle->base_addr1, 0x29, handle->reg_bank1.reg29);
}

/*******************************************************************************************************************
 * _ive_hal_set_coeff_thresh_s16
 *   Set thresh s16 coefficient
 *
 * Parameters:
 *   handle: IVE HAL handle
 *   coeff: coefficient
 *
 * Return:
 *   none
 */
static void _ive_hal_set_coeff_thresh_s16(ive_hal_handle *handle, ive_ioc_coeff_thresh_s16 *coeff)
{
    handle->reg_bank1.op_mode        = coeff->mode;
    handle->reg_bank1.mask0          = coeff->min;
    handle->reg_bank1.mask1          = coeff->mid;
    handle->reg_bank1.mask2          = coeff->max;
    handle->reg_bank1.thresh_16bit_1 = coeff->low;
    handle->reg_bank1.thresh_16bit_2 = coeff->high;

    REGW(handle->base_addr1, 0x04, handle->reg_bank1.reg04);
    REGW(handle->base_addr1, 0x1A, handle->reg_bank1.reg1A);
    REGW(handle->base_addr1, 0x1B, handle->reg_bank1.reg1B);
    REGW(handle->base_addr1, 0x28, handle->reg_bank1.reg28);
    REGW(handle->base_addr1, 0x29, handle->reg_bank1.reg29);
}

/*******************************************************************************************************************
 * _ive_hal_set_coeff_thresh_u16
 *   Set thresh u16 coefficient
 *
 * Parameters:
 *   handle: IVE HAL handle
 *   coeff: coefficient
 *
 * Return:
 *   none
 */
static void _ive_hal_set_coeff_thresh_u16(ive_hal_handle *handle, ive_ioc_coeff_thresh_u16 *coeff)
{
    handle->reg_bank1.op_mode        = coeff->mode;
    handle->reg_bank1.mask0          = coeff->min;
    handle->reg_bank1.mask1          = coeff->mid;
    handle->reg_bank1.mask2          = coeff->max;
    handle->reg_bank1.thresh_16bit_1 = coeff->low;
    handle->reg_bank1.thresh_16bit_2 = coeff->high;

    REGW(handle->base_addr1, 0x04, handle->reg_bank1.reg04);
    REGW(handle->base_addr1, 0x1A, handle->reg_bank1.reg1A);
    REGW(handle->base_addr1, 0x1B, handle->reg_bank1.reg1B);
    REGW(handle->base_addr1, 0x28, handle->reg_bank1.reg28);
    REGW(handle->base_addr1, 0x29, handle->reg_bank1.reg29);
}

/*******************************************************************************************************************
 * _ive_hal_set_coeff_add
 *   Set add coefficient
 *
 * Parameters:
 *   handle: IVE HAL handle
 *   coeff: coefficient
 *
 * Return:
 *   none
 */
static void _ive_hal_set_coeff_add(ive_hal_handle *handle, ive_ioc_coeff_add *coeff)
{
    handle->reg_bank1.op_mode      = coeff->mode;
    handle->reg_bank1.add_weight_x = coeff->weight_x;
    handle->reg_bank1.add_weight_y = coeff->weight_y;

    REGW(handle->base_addr1, 0x04, handle->reg_bank1.reg04);
    REGW(handle->base_addr1, 0x2B, handle->reg_bank1.reg2B);
    REGW(handle->base_addr1, 0x2C, handle->reg_bank1.reg2C);
}

/*******************************************************************************************************************
 * _ive_hal_set_coeff_sub
 *   Set sub coefficient
 *
 * Parameters:
 *   handle: IVE HAL handle
 *   coeff: coefficient
 *
 * Return:
 *   none
 */
static void _ive_hal_set_coeff_sub(ive_hal_handle *handle, ive_ioc_coeff_sub *coeff)
{
    handle->reg_bank1.op_mode = coeff->mode;

    REGW(handle->base_addr1, 0x04, handle->reg_bank1.reg04);
}

/*******************************************************************************************************************
 * ive_hal_set_coeff_16to8
 *   Set 16 to 8 coefficient
 *
 * Parameters:
 *   handle: IVE HAL handle
 *   coeff: coefficient
 *
 * Return:
 *   none
 */
static void _ive_hal_set_coeff_16to8(ive_hal_handle *handle, ive_ioc_coeff_16to8 *coeff)
{
    handle->reg_bank1.op_mode  = coeff->mode;
    handle->reg_bank1.fraction = (u16)(((u32)coeff->numerator << 16) / (u32)coeff->denominator);
    handle->reg_bank1.mask0    = coeff->bias;

    REGW(handle->base_addr1, 0x04, handle->reg_bank1.reg04);
    REGW(handle->base_addr1, 0x2A, handle->reg_bank1.reg2A);
    REGW(handle->base_addr1, 0x1A, handle->reg_bank1.reg1A);
}

/*******************************************************************************************************************
 * _ive_hal_set_coeff_map
 *   Set map coefficient
 *
 * Parameters:
 *   handle: IVE HAL handle
 *   coeff: coefficient
 *
 * Return:
 *   none
 */
static void _ive_hal_set_coeff_map(ive_hal_handle *handle, u64 map_addr)
{
    handle->reg_bank1.src1_addr_low           = ADDR_LOW(map_addr);
    handle->reg_bank1.src1_addr_high          = ADDR_HIGH(map_addr);
    handle->reg_bank1.reg_ive_src1_addr_35_32 = ADDR_HIGH_EXT(map_addr);

    REGW(handle->base_addr1, 0x08, handle->reg_bank1.reg08);
    REGW(handle->base_addr1, 0x09, handle->reg_bank1.reg09);
    REGW(handle->base_addr1, 0x46, handle->reg_bank1.reg46);
}

/*******************************************************************************************************************
 * _ive_hal_set_coeff_integral
 *   Set integral coefficient
 *
 * Parameters:
 *   handle: IVE HAL handle
 *   coeff: coefficient
 *
 * Return:
 *   none
 */
static void _ive_hal_set_coeff_integral(ive_hal_handle *handle, ive_ioc_coeff_integral *coeff)
{
    handle->reg_bank1.op_mode = coeff->mode;
    REGW(handle->base_addr1, 0x04, handle->reg_bank1.reg04);
}

/*******************************************************************************************************************
 * _ive_hal_set_coeff_sad
 *   Set SAD coefficient
 *
 * Parameters:
 *   handle: IVE HAL handle
 *   coeff: coefficient
 *
 * Return:
 *   none
 */
static void _ive_hal_set_coeff_sad(ive_hal_handle *handle, ive_ioc_coeff_sad *coeff)
{
    handle->reg_bank1.op_mode        = coeff->block_mode;
    handle->reg_bank1.outfmt         = coeff->out_mode;
    handle->reg_bank1.mask0          = coeff->min;
    handle->reg_bank1.mask1          = coeff->max;
    handle->reg_bank1.thresh_16bit_1 = coeff->thresh;

    REGW(handle->base_addr1, 0x04, handle->reg_bank1.reg04);
    REGW(handle->base_addr1, 0x05, handle->reg_bank1.reg05);
    REGW(handle->base_addr1, 0x1A, handle->reg_bank1.reg1A);
    REGW(handle->base_addr1, 0x28, handle->reg_bank1.reg28);
}

/*******************************************************************************************************************
 * _ive_hal_set_coeff_lbp
 *   Set LBP coefficient
 *
 * Parameters:
 *   handle: IVE HAL handle
 *   coeff: coefficient
 *
 * Return:
 *   none
 */
static void _ive_hal_set_coeff_ncc(ive_hal_handle *handle, u64 output_addr)
{
    handle->reg_bank1.dst1_addr_low           = ADDR_LOW(output_addr);
    handle->reg_bank1.dst1_addr_high          = ADDR_HIGH(output_addr);
    handle->reg_bank1.reg_ive_dst1_addr_35_32 = ADDR_HIGH_EXT(output_addr);
    // handle->reg_bank1.dst1_stride = handle->reg_bank1.src1_stride;

    REGW(handle->base_addr1, 0x0A, handle->reg_bank1.reg0A);
    REGW(handle->base_addr1, 0x0B, handle->reg_bank1.reg0B);
    REGW(handle->base_addr1, 0x47, handle->reg_bank1.reg47);
    // REGW(handle->base_addr1, 0x15, handle->reg_bank1.reg15);
}

/*******************************************************************************************************************
 * _ive_hal_set_coeff_lbp
 *   Set LBP coefficient
 *
 * Parameters:
 *   handle: IVE HAL handle
 *   coeff: coefficient
 *
 * Return:
 *   none
 */
static void _ive_hal_set_coeff_lbp(ive_hal_handle *handle, ive_ioc_coeff_lbp *coeff)
{
    handle->reg_bank1.op_mode        = coeff->mode;
    handle->reg_bank1.infmt          = coeff->chlmode;
    handle->reg_bank1.thresh_16bit_1 = coeff->thresh;

    REGW(handle->base_addr1, 0x04, handle->reg_bank1.reg04);
    REGW(handle->base_addr1, 0x05, handle->reg_bank1.reg05);
    REGW(handle->base_addr1, 0x28, handle->reg_bank1.reg28);
}

/*******************************************************************************************************************
 * _ive_hal_set_coeff_bat
 *   Set bat coefficient
 *
 * Parameters:
 *   handle: IVE HAL handle
 *   coeff: coefficient
 *
 * Return:
 *   none
 */
static void _ive_hal_set_coeff_bat(ive_hal_handle *handle, ive_ioc_coeff_bat *coeff)
{
    handle->reg_bank1.thresh_16bit_1 = coeff->h_times;
    handle->reg_bank1.thresh_16bit_2 = coeff->v_times;

    REGW(handle->base_addr1, 0x28, handle->reg_bank1.reg28);
    REGW(handle->base_addr1, 0x29, handle->reg_bank1.reg29);
}

/*******************************************************************************************************************
 * _ive_hal_set_coeff_adp_thresh
 *   Set adaptive threshold coefficient
 *
 * Parameters:
 *   handle: IVE HAL handle
 *   coeff: coefficient
 *
 * Return:
 *   none
 */
static void _ive_hal_set_coeff_adp_thresh(ive_hal_handle *handle, ive_ioc_coeff_adp_thresh *coeff)
{
    handle->reg_bank1.mask0          = coeff->u8HalfMaskx;
    handle->reg_bank1.mask1          = coeff->u8HalfMasky;
    handle->reg_bank1.shift          = coeff->s8Offset;
    handle->reg_bank1.thresh_16bit_1 = coeff->u8ValueThr;
    handle->reg_bank1.add_weight_x   = coeff->u8RateThr;

    REGW(handle->base_addr1, 0x1A, handle->reg_bank1.reg1A);
    REGW(handle->base_addr1, 0x26, handle->reg_bank1.reg26);
    REGW(handle->base_addr1, 0x28, handle->reg_bank1.reg28);
    REGW(handle->base_addr1, 0x2B, handle->reg_bank1.reg2B);
}

/*******************************************************************************************************************
 * _ive_hal_set_coeff_matrix_transform
 *   Set matrix transform coefficient
 *
 * Parameters:
 *   handle: IVE HAL handle
 *   coeff: coefficient
 *
 * Return:
 *   none
 */
static void _ive_hal_set_coeff_matrix_transform(ive_hal_handle *handle, ive_ioc_coeff_matrix_transform *coeff)
{
    handle->reg_bank1.op_mode = coeff->ctrl_mode;

    switch (coeff->chl_mode)
    {
        case IVE_IOC_MODE_MATRIX_TRANSFORM_C1:
            handle->reg_bank1.infmt  = 13;
            handle->reg_bank1.outfmt = 13;

            handle->reg_bank1.reg_ive_matrix_coeff0_low  = coeff->s32MatrixArray[0] & 0xFFFF;
            handle->reg_bank1.reg_ive_matrix_coeff0_high = (coeff->s32MatrixArray[0] & 0xFFFF0000) >> 16;
            handle->reg_bank1.reg_ive_matrix_coeff1_low  = 0;
            handle->reg_bank1.reg_ive_matrix_coeff1_high = 0;
            handle->reg_bank1.reg_ive_matrix_coeff2_low  = 0;
            handle->reg_bank1.reg_ive_matrix_coeff2_high = 0;

            handle->reg_bank1.mask0  = 0;
            handle->reg_bank1.mask1  = 0;
            handle->reg_bank1.mask2  = 0;
            handle->reg_bank1.mask3  = 0;
            handle->reg_bank1.mask4  = 0;
            handle->reg_bank1.mask5  = 0;
            handle->reg_bank1.mask6  = 0;
            handle->reg_bank1.mask7  = 0;
            handle->reg_bank1.mask8  = 0;
            handle->reg_bank1.mask9  = 0;
            handle->reg_bank1.mask10 = 0;
            handle->reg_bank1.mask11 = 0;
            handle->reg_bank1.mask12 = 0;
            handle->reg_bank1.mask13 = 0;
            handle->reg_bank1.mask14 = 0;
            handle->reg_bank1.mask15 = 0;
            handle->reg_bank1.mask16 = 0;
            handle->reg_bank1.mask17 = 0;
            handle->reg_bank1.mask18 = 0;
            handle->reg_bank1.mask19 = 0;
            handle->reg_bank1.mask20 = 0;
            handle->reg_bank1.mask21 = 0;
            handle->reg_bank1.mask22 = 0;
            handle->reg_bank1.mask23 = 0;
            break;

        case IVE_IOC_MODE_MATRIX_TRANSFORM_C2:
            handle->reg_bank1.infmt  = 14;
            handle->reg_bank1.outfmt = 14;

            handle->reg_bank1.reg_ive_matrix_coeff0_low  = coeff->s32MatrixArray[0] & 0xFFFF;
            handle->reg_bank1.reg_ive_matrix_coeff0_high = (coeff->s32MatrixArray[0] & 0xFFFF0000) >> 16;
            handle->reg_bank1.reg_ive_matrix_coeff1_low  = coeff->s32MatrixArray[1] & 0xFFFF;
            handle->reg_bank1.reg_ive_matrix_coeff1_high = (coeff->s32MatrixArray[1] & 0xFFFF0000) >> 16;
            handle->reg_bank1.reg_ive_matrix_coeff2_low  = 0;
            handle->reg_bank1.reg_ive_matrix_coeff2_high = 0;

            handle->reg_bank1.mask0  = coeff->s32MatrixArray[2] & 0xFF;
            handle->reg_bank1.mask1  = (coeff->s32MatrixArray[2] & (0xFF << 8)) >> 8;
            handle->reg_bank1.mask2  = (coeff->s32MatrixArray[2] & (0xFF << 16)) >> 16;
            handle->reg_bank1.mask3  = (coeff->s32MatrixArray[2] & (0xFF << 24)) >> 24;
            handle->reg_bank1.mask4  = coeff->s32MatrixArray[3] & 0xFF;
            handle->reg_bank1.mask5  = (coeff->s32MatrixArray[3] & (0xFF << 8)) >> 8;
            handle->reg_bank1.mask6  = (coeff->s32MatrixArray[3] & (0xFF << 16)) >> 16;
            handle->reg_bank1.mask7  = (coeff->s32MatrixArray[3] & (0xFF << 24)) >> 24;
            handle->reg_bank1.mask8  = 0;
            handle->reg_bank1.mask9  = 0;
            handle->reg_bank1.mask10 = 0;
            handle->reg_bank1.mask11 = 0;
            handle->reg_bank1.mask12 = 0;
            handle->reg_bank1.mask13 = 0;
            handle->reg_bank1.mask14 = 0;
            handle->reg_bank1.mask15 = 0;
            handle->reg_bank1.mask16 = 0;
            handle->reg_bank1.mask17 = 0;
            handle->reg_bank1.mask18 = 0;
            handle->reg_bank1.mask19 = 0;
            handle->reg_bank1.mask20 = 0;
            handle->reg_bank1.mask21 = 0;
            handle->reg_bank1.mask22 = 0;
            handle->reg_bank1.mask23 = 0;
            break;

        case IVE_IOC_MODE_MATRIX_TRANSFORM_C3:
            handle->reg_bank1.infmt  = 15;
            handle->reg_bank1.outfmt = 15;

            handle->reg_bank1.reg_ive_matrix_coeff0_low  = coeff->s32MatrixArray[0] & 0xFFFF;
            handle->reg_bank1.reg_ive_matrix_coeff0_high = (coeff->s32MatrixArray[0] & 0xFFFF0000) >> 16;
            handle->reg_bank1.reg_ive_matrix_coeff1_low  = coeff->s32MatrixArray[1] & 0xFFFF;
            handle->reg_bank1.reg_ive_matrix_coeff1_high = (coeff->s32MatrixArray[1] & 0xFFFF0000) >> 16;
            handle->reg_bank1.reg_ive_matrix_coeff2_low  = coeff->s32MatrixArray[2] & 0xFFFF;
            handle->reg_bank1.reg_ive_matrix_coeff2_high = (coeff->s32MatrixArray[2] & 0xFFFF0000) >> 16;

            handle->reg_bank1.mask0  = coeff->s32MatrixArray[3] & 0xFF;
            handle->reg_bank1.mask1  = (coeff->s32MatrixArray[3] & (0xFF << 8)) >> 8;
            handle->reg_bank1.mask2  = (coeff->s32MatrixArray[3] & (0xFF << 16)) >> 16;
            handle->reg_bank1.mask3  = (coeff->s32MatrixArray[3] & (0xFF << 24)) >> 24;
            handle->reg_bank1.mask4  = coeff->s32MatrixArray[4] & 0xFF;
            handle->reg_bank1.mask5  = (coeff->s32MatrixArray[4] & (0xFF << 8)) >> 8;
            handle->reg_bank1.mask6  = (coeff->s32MatrixArray[4] & (0xFF << 16)) >> 16;
            handle->reg_bank1.mask7  = (coeff->s32MatrixArray[4] & (0xFF << 24)) >> 24;
            handle->reg_bank1.mask8  = coeff->s32MatrixArray[5] & 0xFF;
            handle->reg_bank1.mask9  = (coeff->s32MatrixArray[5] & (0xFF << 8)) >> 8;
            handle->reg_bank1.mask10 = (coeff->s32MatrixArray[5] & (0xFF << 16)) >> 16;
            handle->reg_bank1.mask11 = (coeff->s32MatrixArray[5] & (0xFF << 24)) >> 24;
            handle->reg_bank1.mask12 = coeff->s32MatrixArray[6] & 0xFF;
            handle->reg_bank1.mask13 = (coeff->s32MatrixArray[6] & (0xFF << 8)) >> 8;
            handle->reg_bank1.mask14 = (coeff->s32MatrixArray[6] & (0xFF << 16)) >> 16;
            handle->reg_bank1.mask15 = (coeff->s32MatrixArray[6] & (0xFF << 24)) >> 24;
            handle->reg_bank1.mask16 = coeff->s32MatrixArray[7] & 0xFF;
            handle->reg_bank1.mask17 = (coeff->s32MatrixArray[7] & (0xFF << 8)) >> 8;
            handle->reg_bank1.mask18 = (coeff->s32MatrixArray[7] & (0xFF << 16)) >> 16;
            handle->reg_bank1.mask19 = (coeff->s32MatrixArray[7] & (0xFF << 24)) >> 24;
            handle->reg_bank1.mask20 = coeff->s32MatrixArray[8] & 0xFF;
            handle->reg_bank1.mask21 = (coeff->s32MatrixArray[8] & (0xFF << 8)) >> 8;
            handle->reg_bank1.mask22 = (coeff->s32MatrixArray[8] & (0xFF << 16)) >> 16;
            handle->reg_bank1.mask23 = (coeff->s32MatrixArray[8] & (0xFF << 24)) >> 24;
            break;
        default:
            IVE_MSG(IVE_MSG_ERR, "Invalid input type of matrix transform!!\n");
            break;
    }

    REGW(handle->base_addr1, 0x04, handle->reg_bank1.reg04);
    REGW(handle->base_addr1, 0x05, handle->reg_bank1.reg05);
    REGW(handle->base_addr1, 0x1A, handle->reg_bank1.reg1A);
    REGW(handle->base_addr1, 0x1B, handle->reg_bank1.reg1B);
    REGW(handle->base_addr1, 0x1C, handle->reg_bank1.reg1C);
    REGW(handle->base_addr1, 0x1D, handle->reg_bank1.reg1D);
    REGW(handle->base_addr1, 0x1E, handle->reg_bank1.reg1E);
    REGW(handle->base_addr1, 0x1F, handle->reg_bank1.reg1F);
    REGW(handle->base_addr1, 0x20, handle->reg_bank1.reg20);
    REGW(handle->base_addr1, 0x21, handle->reg_bank1.reg21);
    REGW(handle->base_addr1, 0x22, handle->reg_bank1.reg22);
    REGW(handle->base_addr1, 0x23, handle->reg_bank1.reg23);
    REGW(handle->base_addr1, 0x24, handle->reg_bank1.reg24);
    REGW(handle->base_addr1, 0x25, handle->reg_bank1.reg25);
    REGW(handle->base_addr1, 0x40, handle->reg_bank1.reg40);
    REGW(handle->base_addr1, 0x41, handle->reg_bank1.reg41);
    REGW(handle->base_addr1, 0x42, handle->reg_bank1.reg42);
    REGW(handle->base_addr1, 0x43, handle->reg_bank1.reg43);
    REGW(handle->base_addr1, 0x44, handle->reg_bank1.reg44);
    REGW(handle->base_addr1, 0x45, handle->reg_bank1.reg45);
}

/*******************************************************************************************************************
 * _ive_hal_set_coeff_image_dot
 *   Set image dot coefficient
 *
 * Parameters:
 *   handle: IVE HAL handle
 *   coeff: coefficient
 *
 * Return:
 *   none
 */
static void _ive_hal_set_coeff_image_dot(ive_hal_handle *handle, ive_ioc_coeff_image_dot *coeff)
{
    handle->reg_bank1.op_mode = coeff->mode;
    handle->reg_bank1.infmt   = 14;

    REGW(handle->base_addr1, 0x04, handle->reg_bank1.reg04);
    REGW(handle->base_addr1, 0x05, handle->reg_bank1.reg05);
}

/*******************************************************************************************************************
 * _ive_hal_set_coeff_alpha_blending
 *   Set alpha blending coefficient
 *
 * Parameters:
 *   handle: IVE HAL handle
 *   coeff: coefficient
 *
 * Return:
 *   none
 */
static void _ive_hal_set_coeff_alpha_blending(ive_hal_handle *handle)
{
    handle->reg_bank1.infmt = 2;

    REGW(handle->base_addr1, 0x05, handle->reg_bank1.reg05);
}

/*******************************************************************************************************************
 * ive_hal_set_param_bgblur
 *   Set BGBlur params.
 *
 * Parameters:
 *   handle: IVE HAL handle
 *   coeff: coefficient
 *
 * Return:
 *   none
 */
void ive_hal_set_coeff_bgblur(ive_hal_handle *handle, ive_ioc_coeff_bgblur *coeff)
{
    u32 u32RatioX = 0, u32RatioY = 0;

    handle->reg_bank3.reg_bgb_op_type        = coeff->eBgBlurMode;
    handle->reg_bank3.reg_bgb_mask_th_sml    = coeff->u8MaskThr;
    handle->reg_bank3.reg_bgb_sat_level      = coeff->u8SaturationLv;
    handle->reg_bank3.reg_bgb_mosaic_block   = coeff->u8MosaicSize;
    handle->reg_bank3.reg_bgb_mask_map_en    = coeff->eLutEn;
    handle->reg_bank3.reg_bgb_mask_fill_size = coeff->u16FgFillSize;

    if (IVE_IOC_MODE_BGBLUR_BLUR == coeff->eBgBlurMode || IVE_IOC_MODE_BGBLUR_BLUR_AND_MOSAIC == coeff->eBgBlurMode)
    {
        u32RatioX = ((coeff->u16DownWidth << 18) / coeff->stSrcYMask.width + 1) / 2;
        u32RatioY = ((coeff->u16DownHeight << 18) / coeff->stSrcYMask.height + 1) / 2;

        handle->reg_bank3.reg_bgb_g1_g3_resize_ratio_x_low  = ADDR_LOW(u32RatioX);
        handle->reg_bank3.reg_bgb_g1_g3_resize_ratio_x_high = ADDR_HIGH(u32RatioX);
        handle->reg_bank3.reg_bgb_g1_g3_resize_ratio_y_low  = ADDR_LOW(u32RatioY);
        handle->reg_bank3.reg_bgb_g1_g3_resize_ratio_y_high = ADDR_HIGH(u32RatioY);
    }

    u32RatioX = ((coeff->stSrcYMask.width << 18) / coeff->stSrcOri.width + 1) / 2;
    u32RatioY = ((coeff->stSrcYMask.height << 18) / coeff->stSrcOri.height + 1) / 2;

    handle->reg_bank3.reg_bgb_g5_resize_ratio_x_low  = ADDR_LOW(u32RatioX);
    handle->reg_bank3.reg_bgb_g5_resize_ratio_x_high = ADDR_HIGH(u32RatioX);
    handle->reg_bank3.reg_bgb_g5_resize_ratio_y_low  = ADDR_LOW(u32RatioY);
    handle->reg_bank3.reg_bgb_g5_resize_ratio_y_high = ADDR_HIGH(u32RatioY);

    if (IVE_IOC_LUT_ENABLE == coeff->eLutEn)
    {
        handle->reg_bank1.Bgb_mapkey1  = coeff->u8LutKey[0];
        handle->reg_bank1.Bgb_mapkey2  = coeff->u8LutKey[1];
        handle->reg_bank1.Bgb_mapkey3  = coeff->u8LutKey[2];
        handle->reg_bank1.Bgb_mapkey4  = coeff->u8LutKey[3];
        handle->reg_bank1.Bgb_mapkey5  = coeff->u8LutKey[4];
        handle->reg_bank1.Bgb_mapkey6  = coeff->u8LutKey[5];
        handle->reg_bank1.Bgb_mapkey7  = coeff->u8LutKey[6];
        handle->reg_bank1.Bgb_mapkey8  = coeff->u8LutKey[7];
        handle->reg_bank1.Bgb_mapkey9  = coeff->u8LutKey[8];
        handle->reg_bank1.Bgb_mapkey10 = coeff->u8LutKey[9];
        handle->reg_bank1.Bgb_mapkey11 = coeff->u8LutKey[10];
        handle->reg_bank1.Bgb_mapkey12 = coeff->u8LutKey[11];
        handle->reg_bank1.Bgb_mapkey13 = coeff->u8LutKey[12];
        handle->reg_bank1.Bgb_mapkey14 = coeff->u8LutKey[13];
        handle->reg_bank1.Bgb_mapkey15 = coeff->u8LutKey[14];

        handle->reg_bank1.Bgb_mapval1  = coeff->u8LutVal[0];
        handle->reg_bank1.Bgb_mapval2  = coeff->u8LutVal[1];
        handle->reg_bank1.Bgb_mapval3  = coeff->u8LutVal[2];
        handle->reg_bank1.Bgb_mapval4  = coeff->u8LutVal[3];
        handle->reg_bank1.Bgb_mapval5  = coeff->u8LutVal[4];
        handle->reg_bank1.Bgb_mapval6  = coeff->u8LutVal[5];
        handle->reg_bank1.Bgb_mapval7  = coeff->u8LutVal[6];
        handle->reg_bank1.Bgb_mapval8  = coeff->u8LutVal[7];
        handle->reg_bank1.Bgb_mapval9  = coeff->u8LutVal[8];
        handle->reg_bank1.Bgb_mapval10 = coeff->u8LutVal[9];
        handle->reg_bank1.Bgb_mapval11 = coeff->u8LutVal[10];
        handle->reg_bank1.Bgb_mapval12 = coeff->u8LutVal[11];
        handle->reg_bank1.Bgb_mapval13 = coeff->u8LutVal[12];
        handle->reg_bank1.Bgb_mapval14 = coeff->u8LutVal[13];
        handle->reg_bank1.Bgb_mapval15 = coeff->u8LutVal[14];
        handle->reg_bank1.Bgb_mapval16 = coeff->u8LutVal[15];

        REGW(handle->base_addr1, 0x1a, handle->reg_bank1.reg1A);
        REGW(handle->base_addr1, 0x1b, handle->reg_bank1.reg1B);
        REGW(handle->base_addr1, 0x1c, handle->reg_bank1.reg1C);
        REGW(handle->base_addr1, 0x1d, handle->reg_bank1.reg1D);
        REGW(handle->base_addr1, 0x1e, handle->reg_bank1.reg1E);
        REGW(handle->base_addr1, 0x1f, handle->reg_bank1.reg1F);
        REGW(handle->base_addr1, 0x20, handle->reg_bank1.reg20);
        REGW(handle->base_addr1, 0x21, handle->reg_bank1.reg21);
        REGW(handle->base_addr1, 0x22, handle->reg_bank1.reg22);
        REGW(handle->base_addr1, 0x23, handle->reg_bank1.reg23);
        REGW(handle->base_addr1, 0x24, handle->reg_bank1.reg24);
        REGW(handle->base_addr1, 0x25, handle->reg_bank1.reg25);
        REGW(handle->base_addr1, 0x26, handle->reg_bank1.reg26);
        REGW(handle->base_addr1, 0x4c, handle->reg_bank1.reg4c);
        REGW(handle->base_addr1, 0x4d, handle->reg_bank1.reg4d);
        REGW(handle->base_addr1, 0x4e, handle->reg_bank1.reg4e);
    }

    REGW(handle->base_addr3, 0x13, handle->reg_bank3.reg13);
    REGW(handle->base_addr3, 0x69, handle->reg_bank3.reg69);
    REGW(handle->base_addr3, 0x6a, handle->reg_bank3.reg6a);
    REGW(handle->base_addr3, 0x6b, handle->reg_bank3.reg6b);
    REGW(handle->base_addr3, 0x6c, handle->reg_bank3.reg6c);
    REGW(handle->base_addr3, 0x6d, handle->reg_bank3.reg6d);
    REGW(handle->base_addr3, 0x72, handle->reg_bank3.reg72);
    REGW(handle->base_addr3, 0x73, handle->reg_bank3.reg73);
    REGW(handle->base_addr3, 0x74, handle->reg_bank3.reg74);
    REGW(handle->base_addr3, 0x75, handle->reg_bank3.reg75);
    REGW(handle->base_addr3, 0x76, handle->reg_bank3.reg76);
    REGW(handle->base_addr3, 0x77, handle->reg_bank3.reg77);
    REGW(handle->base_addr3, 0x78, handle->reg_bank3.reg78);
    REGW(handle->base_addr3, 0x79, handle->reg_bank3.reg79);
    REGW(handle->base_addr3, 0x7a, handle->reg_bank3.reg7a);
}

/*******************************************************************************************************************
 * ive_hal_set_burst
 *   set burst length and outstanding.
 *
 * Parameters:
 *   handle: IVE HAL handle
 *
 * Return:
 *   none
 */

void ive_hal_set_burst(ive_hal_handle *handle)
{
    u8 u8SysArlen = IVE_GET_8BIT_BY_OFFSET(g_u32BurstOutstanding, 0);
    u8 u8SysAwlen = IVE_GET_8BIT_BY_OFFSET(g_u32BurstOutstanding, 8);
    u8 u8SysArOut = IVE_GET_8BIT_BY_OFFSET(g_u32BurstOutstanding, 16);
    u8 u8SysAwOut = IVE_GET_8BIT_BY_OFFSET(g_u32BurstOutstanding, 24);

    u8 u8RealArlen = AXI_ARLEN_DEFAULT;
    u8 u8RealAwlen = AXI_AWLEN_DEFAULT;
    u8 u8RealArOut = AXI_AR_MAX_OUTSTANDING_DEFAULT;
    u8 u8RealAwOut = AXI_AR_MAX_OUTSTANDING_DEFAULT;

    // register value 0/1/2/3 means burst length 4/8/16/32.
    if (u8SysArlen)
    {
        u8RealArlen = u8SysArlen > 8 ? (u8SysArlen > 16 ? 3 : 2) : (u8SysArlen > 4 ? 1 : 0);
    }

    if (u8SysAwlen)
    {
        u8RealAwlen = u8SysAwlen > 8 ? (u8SysAwlen > 16 ? 3 : 2) : (u8SysAwlen > 4 ? 1 : 0);
    }

    // register value 0/1/2/3 means outstanding threshold 32/64/128/256x128 bit.
    if (u8SysArOut)
    {
        u8RealArOut = u8SysArOut > 64 ? (u8SysArOut > 128 ? 3 : 2) : (u8SysArOut > 32 ? 1 : 0);
    }

    if (u8SysAwOut)
    {
        u8RealAwOut = u8SysAwOut > 64 ? (u8SysAwOut > 128 ? 3 : 2) : (u8SysAwOut > 32 ? 1 : 0);
    }

    handle->reg_bank0.reg_axi_arlen              = u8RealArlen;
    handle->reg_bank0.reg_axi_awlen              = u8RealAwlen;
    handle->reg_bank0.reg_axi_ar_max_outstanding = u8RealArOut;
    handle->reg_bank0.reg_axi_aw_max_outstanding = u8RealAwOut;

    REGW(handle->base_addr0, 0x4d, handle->reg_bank0.reg4d);
    REGW(handle->base_addr0, 0x4e, handle->reg_bank0.reg4e);
    REGW(handle->base_addr0, 0x4f, handle->reg_bank0.reg4f);
    REGW(handle->base_addr0, 0x50, handle->reg_bank0.reg50);
}
/*******************************************************************************************************************
 * ive_hal_start
 *   start IVE HW engine to process images
 *
 * Parameters:
 *   handle: IVE HAL handle
 *
 * Return:
 *   none
 */
void ive_hal_start(ive_hal_handle *handle)
{
    handle->reg_bank0.sw_fire = 1;

    REGW(handle->base_addr0, 0x00, handle->reg_bank0.reg00);

    handle->reg_bank0.sw_fire = 0; // write one clear
}

/*******************************************************************************************************************
 * ive_hal_start_bgblur
 *   start IVE BGBlur HW engine to process images
 *
 * Parameters:
 *   handle: IVE HAL handle
 *
 * Return:
 *   none
 */
void ive_hal_start_bgblur(ive_hal_handle *handle)
{
    handle->reg_bank3.reg_bgb_sw_fire = 1;

    REGW(handle->base_addr3, 0x00, handle->reg_bank3.reg00);
    handle->reg_bank3.reg_bgb_sw_fire = 0;
}

/*******************************************************************************************************************
 * ive_hal_sw_reset
 *   reset IVE & IVE BGBlur HW engine
 *
 * Parameters:
 *   handle: IVE HAL handle
 *
 * Return:
 *   none
 */
void ive_hal_sw_reset(ive_hal_handle *handle)
{
    const u16 u16MaxRetryCount       = 10;
    u16       u16RetryCount          = 0;
    handle->reg_bank0.sw_rst         = 1;
    handle->reg_bank3.reg_bgb_sw_rst = 1;

    // set IVE SW reset.
    REGW(handle->base_addr0, 0x02, handle->reg_bank0.reg02);
    REGW(handle->base_addr3, 0x01, handle->reg_bank3.reg01);

    // set clean on for IVE miu port.
    REGW(MIU_EFFI_GRP_SC1_BASE, MIU_IVE_WRITE_PORT_OFFSET, MIU_IVE_CLEAN_ON);
    REGW(MIU_EFFI_GRP_SC1_BASE, MIU_IVE_READ_PORT_OFFSET, MIU_IVE_CLEAN_ON);

    /* wait clean on over.
     * the wait time is depended on oustanding, when outstanding is n x 128 bit, the wait time is n x (1000 /
     * clk_miu(Mhz))ns. when min clk_miu is 300Mhz, max outstanding is 1024 x 128 bit, the wait time is about 3500ns.
     */
    while (u16RetryCount++ < u16MaxRetryCount
           && (0 != (REGR(MIU_EFFI_GRP_SC1_BASE, MIU_IVE_WRITE_CLEAN_STATUS_OFFSET) & MIU_IVE_GRAB_W_STATUS_BIT)
               || 0 != (REGR(MIU_EFFI_GRP_SC1_BASE, MIU_IVE_READ_CLEAN_STATUS_OFFSET) & MIU_IVE_GRAB_R_STATUS_BIT)))
    {
        CamOsUsDelay(1);
    }

    if (0 != (REGR(MIU_EFFI_GRP_SC1_BASE, MIU_IVE_WRITE_CLEAN_STATUS_OFFSET) & MIU_IVE_GRAB_W_STATUS_BIT)
        || 0 != (REGR(MIU_EFFI_GRP_SC1_BASE, MIU_IVE_READ_CLEAN_STATUS_OFFSET) & MIU_IVE_GRAB_R_STATUS_BIT))
    {
        IVE_MSG(IVE_MSG_ERR, "wait clean on timeout\n");
    }

    // set clean off for IVE miu port.
    REGW(MIU_EFFI_GRP_SC1_BASE, MIU_IVE_WRITE_PORT_OFFSET, MIU_IVE_CLEAN_OFF);
    REGW(MIU_EFFI_GRP_SC1_BASE, MIU_IVE_READ_PORT_OFFSET, MIU_IVE_CLEAN_OFF);

    handle->reg_bank0.sw_rst         = 0;
    handle->reg_bank3.reg_bgb_sw_rst = 0;
    // clear IVE SW reset.

    REGW(handle->base_addr0, 0x02, handle->reg_bank0.reg02);
    REGW(handle->base_addr3, 0x01, handle->reg_bank3.reg01);

    memset(&handle->reg_bank0, 0, sizeof(handle->reg_bank0));
    memset(&handle->reg_bank1, 0, sizeof(handle->reg_bank1));
    memset(&handle->reg_bank3, 0, sizeof(handle->reg_bank3));
}

/*******************************************************************************************************************
 * ive_hal_get_check_params
 *   get width&height check flag of input&output;
 *
 * Parameters:
 *   eOpType : op type of the current config.
 *   peCheckFlag : check flag for output.
 *   pu16MaxW : max check width for output.
 *   pu16MaxH : max check height for output.
 *   pu16MinW : min check width for output.
 *   pu16MinH : min check height for output.
 *
 * Return:
 *   IVE_HAL_FAIL    : op type is not supported.
 *   IVE_HAL_SUCCESS : op type is supported, set check flag and max/min width/height.
 */
IVE_HAL_RET_E ive_hal_get_check_params(IVE_IOC_OP_TYPE eOpType, IVE_CONFIG_WH_CHECK_E *peCheckFlag, u16 *pu16MaxW,
                                       u16 *pu16MaxH, u16 *pu16MinW, u16 *pu16MinH)
{
    switch (eOpType)
    {
        // no need to check w/h.
        case IVE_IOC_OP_TYPE_NCC:
        case IVE_IOC_OP_TYPE_MAP:
        case IVE_IOC_OP_TYPE_HISTOGRAM:
        case IVE_IOC_OP_TYPE_BAT:
            break;

        // check input w/h.
        case IVE_IOC_OP_TYPE_SAD:
            *peCheckFlag = IVE_CONFIG_WH_CHECK_INPUT;
            *pu16MinW    = MIN_WIDTH;
            *pu16MinH    = MIN_HEIGHT;
            break;

        // check input / output w/h.
        case IVE_IOC_OP_TYPE_BGBLUR:
            *peCheckFlag = IVE_CONFIG_WH_CHECK_MATCH | IVE_CONFIG_WH_CHECK_INPUT;
            *pu16MinW    = MIN_W_BGBLUR;
            *pu16MinH    = MIN_H_BGBLUR;
            *pu16MaxW    = MAX_W_BGBLUR;
            *pu16MaxH    = MAX_H_BGBLUR;
            break;

        case IVE_IOC_OP_TYPE_FILTER:
        case IVE_IOC_OP_TYPE_CSC:
        case IVE_IOC_OP_TYPE_FILTER_AND_CSC:
        case IVE_IOC_OP_TYPE_SOBEL:
        case IVE_IOC_OP_TYPE_MAG_AND_ANG:
        case IVE_IOC_OP_TYPE_ORD_STA_FILTER:
        case IVE_IOC_OP_TYPE_BERNSEN:
        case IVE_IOC_OP_TYPE_DILATE:
        case IVE_IOC_OP_TYPE_ERODE:
        case IVE_IOC_OP_TYPE_THRESH:
        case IVE_IOC_OP_TYPE_THRESH_S16:
        case IVE_IOC_OP_TYPE_THRESH_U16:
        case IVE_IOC_OP_TYPE_AND:
        case IVE_IOC_OP_TYPE_OR:
        case IVE_IOC_OP_TYPE_XOR:
        case IVE_IOC_OP_TYPE_ADD:
        case IVE_IOC_OP_TYPE_SUB:
        case IVE_IOC_OP_TYPE_16BIT_TO_8BIT:
        case IVE_IOC_OP_TYPE_INTEGRAL:
        case IVE_IOC_OP_TYPE_LBP:
        case IVE_IOC_OP_TYPE_ADP_THRESH:
        case IVE_IOC_OP_TYPE_MATRIX_TRANSFORM:
        case IVE_IOC_OP_TYPE_IMAGE_DOT:
        case IVE_IOC_OP_TYPE_ALPHA_BLENDING:
            *peCheckFlag = IVE_CONFIG_WH_CHECK_MATCH | IVE_CONFIG_WH_CHECK_INPUT;
            *pu16MinW    = MIN_WIDTH;
            *pu16MinH    = MIN_HEIGHT;
            break;

        default:
            IVE_MSG(IVE_MSG_ERR, "op(%d) is not supported\n", eOpType);
            return IVE_IOC_ERROR_PROC_CONFIG;
    }

    return IVE_IOC_ERROR_NONE;
}

/*******************************************************************************************************************
 * ive_hal_set_op_params
 *   set params according to config op type.
 *
 * Parameters:
 *   pstHandle : IVE HAL handle
 *   pstConfig : request config.
 *
 * Return:
 *   None.
 */
void ive_hal_set_op_params(ive_hal_handle *pstHandle, ive_ioc_config *pstConfig)
{
    switch (pstConfig->op_type)
    {
        case IVE_IOC_OP_TYPE_FILTER:
            _ive_hal_set_coeff_filter(pstHandle, &pstConfig->coeff_filter);
            break;

        case IVE_IOC_OP_TYPE_CSC:
            _ive_hal_set_coeff_csc(pstHandle, &pstConfig->coeff_csc);
            break;

        case IVE_IOC_OP_TYPE_FILTER_AND_CSC:
            _ive_hal_set_coeff_filter(pstHandle, &pstConfig->coeff_filter_csc.filter);
            _ive_hal_set_coeff_csc(pstHandle, &pstConfig->coeff_filter_csc.csc);
            break;

        case IVE_IOC_OP_TYPE_SOBEL:
            _ive_hal_set_coeff_sobel(pstHandle, &pstConfig->coeff_sobel);
            break;

        case IVE_IOC_OP_TYPE_MAG_AND_ANG:
            _ive_hal_set_coeff_mag_and_ang(pstHandle, &pstConfig->coeff_mag_and_ang);
            break;

        case IVE_IOC_OP_TYPE_ORD_STA_FILTER:
            _ive_hal_set_coeff_ord_stat_filter(pstHandle, &pstConfig->coeff_ord_stat_filter);
            break;

        case IVE_IOC_OP_TYPE_BERNSEN:
            _ive_hal_set_coeff_bernsen(pstHandle, &pstConfig->coeff_bernsen);
            break;

        case IVE_IOC_OP_TYPE_DILATE:
            _ive_hal_set_coeff_dilate(pstHandle, &pstConfig->coeff_dilate);
            break;

        case IVE_IOC_OP_TYPE_ERODE:
            _ive_hal_set_coeff_erode(pstHandle, &pstConfig->coeff_erode);
            break;

        case IVE_IOC_OP_TYPE_THRESH:
            _ive_hal_set_coeff_thresh(pstHandle, &pstConfig->coeff_thresh);
            break;

        case IVE_IOC_OP_TYPE_THRESH_S16:
            _ive_hal_set_coeff_thresh_s16(pstHandle, &pstConfig->coeff_thresh_s16);
            break;

        case IVE_IOC_OP_TYPE_THRESH_U16:
            _ive_hal_set_coeff_thresh_u16(pstHandle, &pstConfig->coeff_thresh_u16);
            break;

        case IVE_IOC_OP_TYPE_ADD:
            _ive_hal_set_coeff_add(pstHandle, &pstConfig->coeff_add);
            break;

        case IVE_IOC_OP_TYPE_SUB:
            _ive_hal_set_coeff_sub(pstHandle, &pstConfig->coeff_sub);
            break;

        case IVE_IOC_OP_TYPE_16BIT_TO_8BIT:
            _ive_hal_set_coeff_16to8(pstHandle, &pstConfig->coeff_16to8);
            break;

        case IVE_IOC_OP_TYPE_MAP:
            _ive_hal_set_coeff_map(pstHandle, (u64)pstConfig->coeff_map.map);
            break;

        case IVE_IOC_OP_TYPE_INTEGRAL:
            _ive_hal_set_coeff_integral(pstHandle, &pstConfig->coeff_integral);
            break;

        case IVE_IOC_OP_TYPE_SAD:
            _ive_hal_set_coeff_sad(pstHandle, &pstConfig->coeff_sad);
            break;

        case IVE_IOC_OP_TYPE_NCC:
            _ive_hal_set_coeff_ncc(pstHandle, (u64)pstConfig->coeff_ncc);
            break;

        case IVE_IOC_OP_TYPE_LBP:
            _ive_hal_set_coeff_lbp(pstHandle, &pstConfig->coeff_lbp);
            break;

        case IVE_IOC_OP_TYPE_BAT:
            _ive_hal_set_coeff_bat(pstHandle, &pstConfig->coeff_bat);
            break;

        case IVE_IOC_OP_TYPE_ADP_THRESH:
            _ive_hal_set_coeff_adp_thresh(pstHandle, &pstConfig->coeff_adp_thresh);
            break;

        case IVE_IOC_OP_TYPE_MATRIX_TRANSFORM:
            _ive_hal_set_coeff_matrix_transform(pstHandle, &pstConfig->coeff_matrix_transform);
            break;

        case IVE_IOC_OP_TYPE_IMAGE_DOT:
            _ive_hal_set_coeff_image_dot(pstHandle, &pstConfig->coeff_image_dot);
            break;

        case IVE_IOC_OP_TYPE_ALPHA_BLENDING:
            _ive_hal_set_coeff_alpha_blending(pstHandle);
            break;

        default:
            break;
    }
}

/*******************************************************************************************************************
 * ive_hal_reg_dump
 *   dump register on the basis of bank base for DEBUG.
 *
 * Parameters:
 *   phys_addr_t: IVE bank base.
 *
 * Return:
 *   none
 */
void ive_hal_reg_dump(ss_phys_addr_t u64BankBase)
{
    int i = 0;
    for (i = 0; i < 128; i += 8)
    {
        IVE_MSG(IVE_MSG_INF, "%x:%02x %04x %04x %04x %04x %04x %04x %04x\n", i, REGR(u64BankBase, i),
                REGR(u64BankBase, i + 1), REGR(u64BankBase, i + 2), REGR(u64BankBase, i + 3), REGR(u64BankBase, i + 4),
                REGR(u64BankBase, i + 5), REGR(u64BankBase, i + 6), REGR(u64BankBase, i + 7));
    }
}
