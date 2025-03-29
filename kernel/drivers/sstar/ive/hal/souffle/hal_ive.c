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
    handle->base_addr2 = u64BankBase[2];
    handle->base_addr3 = u64BankBase[3];
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
 * ive_hal_dummy_eco_set
 *   set eco switch.
 *
 * Parameters:
 *   handle: IVE HAL handle
 *
 * Return:
 *   none
 */
static void ive_hal_dummy_eco_set(ive_hal_handle *handle)
{
    if (CamOsChipRevision() >= 0x2)
    {
        /*
         * RDM-issue: 731fe320-9a3c-4ac8-95c5-800dbd054f30
         */
        handle->reg_bank0.dummy_bpp32_eco_ctrl = 1;
        /*
         * RDM-issue: 383d9368-77a2-49b8-8bcb-6e64f4a56dcc
         */
        handle->reg_bank0.dummy_sad1_eco_ctrl = 1;
        /*
         * RDM-issue: 9334ebd4-e536-465c-9039-d06891237642
         */
        handle->reg_bank0.dummy_sad2_eco_ctrl = 1;

        REGW(handle->base_addr0, 0x42, handle->reg_bank0.reg42);
    }
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

    if (IVE_IOC_OP_TYPE_GMM == config->op_type)
    {
        ive_hal_set_gmm_images(handle, &config->coeff_gmm);
        return;
    }

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

    if (IVE_IOC_OP_TYPE_RESIZE == handle->reg_bank1.op_type)
    {
        handle->reg_bank1.reg_ive_dst_frame_width  = config->output.width - 1;
        handle->reg_bank1.reg_ive_dst_frame_height = config->output.height - 1;
    }

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
    REGW(handle->base_addr1, 0x4c, handle->reg_bank1.reg4c);
    REGW(handle->base_addr1, 0x4d, handle->reg_bank1.reg4d);
}

/*******************************************************************************************************************
 * ive_hal_set_gmm_images
 *   Set input & output image of GMM mode.
 *
 * Parameters:
 *   handle: IVE HAL handle
 *   coeff_gmm: Gmm params.
 *
 * Return:
 *   none
 */
void ive_hal_set_gmm_images(ive_hal_handle *handle, ive_ioc_coeff_gmm *coeff_gmm)
{
    u64 u64GmmSrc1 = 0, u64GmmDst1 = 0, u64GmmDst2 = 0, u64GmmDst3 = 0;

    IVE_MSG(IVE_MSG_DBG, "%s\n", __FUNCTION__);

    handle->reg_bank1.infmt  = coeff_gmm->stSrc[0].format;
    handle->reg_bank1.outfmt = coeff_gmm->stSrc[0].format;

    handle->reg_bank1.frame_width  = coeff_gmm->stSrc[0].width - 1;
    handle->reg_bank1.frame_height = coeff_gmm->stSrc[0].height - 1;

#if defined(SUPPORT_GMM_MULTIFRAME_MODE)
    if (coeff_gmm->eFrameMode >= IVE_IOC_MODE_GMM_FOUR_FRAME)
    {
        u64 u64GmmSrc4 = 0, u64GmmDst8 = 0, u64GmmDst9 = 0;
        u64GmmSrc4 = coeff_gmm->stSrc[3].address[0];
        u64GmmDst8 = coeff_gmm->stDstFg[3].address[0];
        u64GmmDst9 = coeff_gmm->stDstBg[3].address[0];

        handle->reg_bank1.reg_ive_src4_addr_low  = ADDR_LOW(u64GmmSrc4);
        handle->reg_bank1.reg_ive_src4_addr_high = ADDR_HIGH(u64GmmSrc4);
        handle->reg_bank1.reg_ive_src4_addr_h    = ADDR_HIGH_EXT(u64GmmSrc4);
        handle->reg_bank1.reg_ive_dst8_addr_low  = ADDR_LOW(u64GmmDst8);
        handle->reg_bank1.reg_ive_dst8_addr_high = ADDR_HIGH(u64GmmDst8);
        handle->reg_bank1.reg_ive_dst8_addr_h    = ADDR_HIGH_EXT(u64GmmDst8);
        handle->reg_bank1.reg_ive_dst9_addr_low  = ADDR_LOW(u64GmmDst9);
        handle->reg_bank1.reg_ive_dst9_addr_high = ADDR_HIGH(u64GmmDst9);
        handle->reg_bank1.reg_ive_dst9_addr_h    = ADDR_HIGH_EXT(u64GmmDst9);
        handle->reg_bank1.reg_ive_src4_stride    = coeff_gmm->stSrc[3].stride[0] - 1;
        handle->reg_bank1.reg_ive_dst8_stride    = coeff_gmm->stDstFg[3].stride[0] - 1;
        handle->reg_bank1.reg_ive_dst9_stride    = coeff_gmm->stDstBg[3].stride[0] - 1;

        REGW(handle->base_addr1, 0x4f, handle->reg_bank1.reg4f);
        REGW(handle->base_addr1, 0x50, handle->reg_bank1.reg50);
        REGW(handle->base_addr1, 0x51, handle->reg_bank1.reg51);
        REGW(handle->base_addr1, 0x63, handle->reg_bank1.reg63);
        REGW(handle->base_addr1, 0x64, handle->reg_bank1.reg64);
        REGW(handle->base_addr1, 0x65, handle->reg_bank1.reg65);
        REGW(handle->base_addr1, 0x67, handle->reg_bank1.reg67);
        REGW(handle->base_addr1, 0x68, handle->reg_bank1.reg68);
        REGW(handle->base_addr1, 0x69, handle->reg_bank1.reg69);
        REGW(handle->base_addr1, 0x52, handle->reg_bank1.reg52);
        REGW(handle->base_addr1, 0x66, handle->reg_bank1.reg66);
        REGW(handle->base_addr1, 0x6a, handle->reg_bank1.reg6a);
    }
    if (coeff_gmm->eFrameMode >= IVE_IOC_MODE_GMM_THREE_FRAME)
    {
        u64 u64GmmSrc3 = 0, u64GmmDst6 = 0, u64GmmDst7 = 0;
        u64GmmSrc3 = coeff_gmm->stSrc[2].address[0];
        u64GmmDst6 = coeff_gmm->stDstFg[2].address[0];
        u64GmmDst7 = coeff_gmm->stDstBg[2].address[0];

        handle->reg_bank1.src3_addr_low           = ADDR_LOW(u64GmmSrc3);
        handle->reg_bank1.src3_addr_high          = ADDR_HIGH(u64GmmSrc3);
        handle->reg_bank1.reg_ive_src3_addr_35_32 = ADDR_HIGH_EXT(u64GmmSrc3);
        handle->reg_bank1.reg_ive_dst6_addr_low   = ADDR_LOW(u64GmmDst6);
        handle->reg_bank1.reg_ive_dst6_addr_high  = ADDR_HIGH(u64GmmDst6);
        handle->reg_bank1.reg_ive_dst6_addr_h     = ADDR_HIGH_EXT(u64GmmDst6);
        handle->reg_bank1.reg_ive_dst7_addr_low   = ADDR_LOW(u64GmmDst7);
        handle->reg_bank1.reg_ive_dst7_addr_high  = ADDR_HIGH(u64GmmDst7);
        handle->reg_bank1.reg_ive_dst7_addr_h     = ADDR_HIGH_EXT(u64GmmDst7);
        handle->reg_bank1.src3_stride             = coeff_gmm->stSrc[2].stride[0] - 1;
        handle->reg_bank1.reg_ive_dst6_stride     = coeff_gmm->stDstFg[2].stride[0] - 1;
        handle->reg_bank1.reg_ive_dst7_stride     = coeff_gmm->stDstBg[2].stride[0] - 1;

        REGW(handle->base_addr1, 0x10, handle->reg_bank1.reg10);
        REGW(handle->base_addr1, 0x11, handle->reg_bank1.reg11);
        REGW(handle->base_addr1, 0x4a, handle->reg_bank1.reg4a);
        REGW(handle->base_addr1, 0x5b, handle->reg_bank1.reg5b);
        REGW(handle->base_addr1, 0x5c, handle->reg_bank1.reg5c);
        REGW(handle->base_addr1, 0x5d, handle->reg_bank1.reg5d);
        REGW(handle->base_addr1, 0x5f, handle->reg_bank1.reg5f);
        REGW(handle->base_addr1, 0x60, handle->reg_bank1.reg60);
        REGW(handle->base_addr1, 0x61, handle->reg_bank1.reg61);

        REGW(handle->base_addr1, 0x18, handle->reg_bank1.reg18);
        REGW(handle->base_addr1, 0x5e, handle->reg_bank1.reg5e);
        REGW(handle->base_addr1, 0x62, handle->reg_bank1.reg62);
    }
    if (coeff_gmm->eFrameMode >= IVE_IOC_MODE_GMM_TWO_FRAME)
    {
        u64 u64GmmSrc2 = 0, u64GmmDst4 = 0, u64GmmDst5 = 0;
        u64GmmSrc2 = coeff_gmm->stSrc[1].address[0];
        u64GmmDst4 = coeff_gmm->stDstFg[1].address[0];
        u64GmmDst5 = coeff_gmm->stDstBg[1].address[0];

        handle->reg_bank1.src2_addr_low           = ADDR_LOW(u64GmmSrc2);
        handle->reg_bank1.src2_addr_high          = ADDR_HIGH(u64GmmSrc2);
        handle->reg_bank1.reg_ive_src2_addr_35_32 = ADDR_HIGH_EXT(u64GmmSrc2);
        handle->reg_bank1.reg_ive_dst4_addr_low   = ADDR_LOW(u64GmmDst4);
        handle->reg_bank1.reg_ive_dst4_addr_high  = ADDR_HIGH(u64GmmDst4);
        handle->reg_bank1.reg_ive_dst4_addr_h     = ADDR_HIGH_EXT(u64GmmDst4);
        handle->reg_bank1.reg_ive_dst5_addr_low   = ADDR_LOW(u64GmmDst5);
        handle->reg_bank1.reg_ive_dst5_addr_high  = ADDR_HIGH(u64GmmDst5);
        handle->reg_bank1.reg_ive_dst5_addr_h     = ADDR_HIGH_EXT(u64GmmDst5);
        handle->reg_bank1.src2_stride             = coeff_gmm->stSrc[1].stride[0] - 1;
        handle->reg_bank1.reg_ive_dst4_stride     = coeff_gmm->stDstFg[1].stride[0] - 1;
        handle->reg_bank1.reg_ive_dst5_stride     = coeff_gmm->stDstBg[1].stride[0] - 1;

        REGW(handle->base_addr1, 0x0c, handle->reg_bank1.reg0C);
        REGW(handle->base_addr1, 0x0d, handle->reg_bank1.reg0D);
        REGW(handle->base_addr1, 0x48, handle->reg_bank1.reg48);
        REGW(handle->base_addr1, 0x53, handle->reg_bank1.reg53);
        REGW(handle->base_addr1, 0x54, handle->reg_bank1.reg54);
        REGW(handle->base_addr1, 0x55, handle->reg_bank1.reg55);
        REGW(handle->base_addr1, 0x57, handle->reg_bank1.reg57);
        REGW(handle->base_addr1, 0x58, handle->reg_bank1.reg58);
        REGW(handle->base_addr1, 0x59, handle->reg_bank1.reg59);

        REGW(handle->base_addr1, 0x16, handle->reg_bank1.reg16);
        REGW(handle->base_addr1, 0x56, handle->reg_bank1.reg56);
        REGW(handle->base_addr1, 0x5a, handle->reg_bank1.reg5a);
    }
#endif

    u64GmmSrc1 = coeff_gmm->stSrc[0].address[0];
    u64GmmDst1 = coeff_gmm->stDstFg[0].address[0];
    u64GmmDst2 = coeff_gmm->stDstBg[0].address[0];
    u64GmmDst3 = coeff_gmm->u64ModelAddr;

    handle->reg_bank1.src1_addr_low           = ADDR_LOW(u64GmmSrc1);
    handle->reg_bank1.src1_addr_high          = ADDR_HIGH(u64GmmSrc1);
    handle->reg_bank1.reg_ive_src1_addr_35_32 = ADDR_HIGH_EXT(u64GmmSrc1);
    handle->reg_bank1.dst1_addr_low           = ADDR_LOW(u64GmmDst1);
    handle->reg_bank1.dst1_addr_high          = ADDR_HIGH(u64GmmDst1);
    handle->reg_bank1.reg_ive_dst1_addr_35_32 = ADDR_HIGH_EXT(u64GmmDst1);
    handle->reg_bank1.dst2_addr_low           = ADDR_LOW(u64GmmDst2);
    handle->reg_bank1.dst2_addr_high          = ADDR_HIGH(u64GmmDst2);
    handle->reg_bank1.reg_ive_dst2_addr_35_32 = ADDR_HIGH_EXT(u64GmmDst2);
    handle->reg_bank1.dst3_addr_low           = ADDR_LOW(u64GmmDst3);
    handle->reg_bank1.dst3_addr_high          = ADDR_HIGH(u64GmmDst3);
    handle->reg_bank1.reg_ive_dst3_addr_35_32 = ADDR_HIGH_EXT(u64GmmDst3);
    handle->reg_bank1.src1_stride             = coeff_gmm->stSrc[0].stride[0] - 1;
    handle->reg_bank1.dst1_stride             = coeff_gmm->stDstFg[0].stride[0] - 1;
    handle->reg_bank1.dst2_stride             = coeff_gmm->stDstBg[0].stride[0] - 1;
    handle->reg_bank1.dst3_stride             = coeff_gmm->stDstBg[0].stride[0] - 1;

    REGW(handle->base_addr1, 0x05, handle->reg_bank1.reg05);
    REGW(handle->base_addr1, 0x06, handle->reg_bank1.reg06);
    REGW(handle->base_addr1, 0x07, handle->reg_bank1.reg07);
    REGW(handle->base_addr1, 0x08, handle->reg_bank1.reg08);
    REGW(handle->base_addr1, 0x09, handle->reg_bank1.reg09);
    REGW(handle->base_addr1, 0x0a, handle->reg_bank1.reg0A);
    REGW(handle->base_addr1, 0x0b, handle->reg_bank1.reg0B);
    REGW(handle->base_addr1, 0x0e, handle->reg_bank1.reg0E);
    REGW(handle->base_addr1, 0x0f, handle->reg_bank1.reg0F);
    REGW(handle->base_addr1, 0x12, handle->reg_bank1.reg12);
    REGW(handle->base_addr1, 0x13, handle->reg_bank1.reg13);
    REGW(handle->base_addr1, 0x14, handle->reg_bank1.reg14);
    REGW(handle->base_addr1, 0x15, handle->reg_bank1.reg15);
    REGW(handle->base_addr1, 0x17, handle->reg_bank1.reg17);
    REGW(handle->base_addr1, 0x19, handle->reg_bank1.reg19);
    REGW(handle->base_addr1, 0x46, handle->reg_bank1.reg46);
    REGW(handle->base_addr1, 0x47, handle->reg_bank1.reg47);
    REGW(handle->base_addr1, 0x49, handle->reg_bank1.reg49);
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
    u64 u64MaskC0Addr = 0, u64MaskC1Addr = 0, u64BgSoriC0Addr = 0, u64BgSoriC1Addr = 0, u64OriC0Addr = 0,
        u64OriC1Addr = 0;
    u64 u64DstC0Addr = 0, u64DstC1Addr = 0;
    u8  bYUV420SPFlag = (IVE_IOC_IMAGE_FORMAT_420SP == config->coeff_bgblur.stSrcOri.format);

    handle->reg_bank3.reg_bgb_infmt  = bYUV420SPFlag ? 0 : 1;
    handle->reg_bank3.reg_bgb_outfmt = bYUV420SPFlag ? 0 : 1;

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
    handle->reg_bank3.reg_bgb_src0_c1_stride    = ALIGN_UP(4, config->coeff_bgblur.stSrcYMask.stride[0] - 1);

    u64OriC0Addr                                = config->coeff_bgblur.stSrcOri.address[0];
    handle->reg_bank3.reg_bgb_src3_c0_addr_low  = ADDR_LOW(u64OriC0Addr);
    handle->reg_bank3.reg_bgb_src3_c0_addr_high = ADDR_HIGH(u64OriC0Addr);
    handle->reg_bank3.reg_bgb_src3_c0_addr_h    = ADDR_HIGH_EXT(u64OriC0Addr);
    handle->reg_bank3.reg_bgb_src3_c0_stride =
        ALIGN_UP(4, config->coeff_bgblur.stSrcOri.stride[0] - 1); // Uv mask image use the stride of Y mask.
    if (bYUV420SPFlag)
    {
        u64OriC1Addr                                = config->coeff_bgblur.stSrcOri.address[1];
        handle->reg_bank3.reg_bgb_src3_c1_addr_low  = ADDR_LOW(u64OriC1Addr);
        handle->reg_bank3.reg_bgb_src3_c1_addr_high = ADDR_HIGH(u64OriC1Addr);
        handle->reg_bank3.reg_bgb_src3_c1_addr_h    = ADDR_HIGH_EXT(u64OriC1Addr);
        handle->reg_bank3.reg_bgb_src3_c1_stride    = ALIGN_UP(4, config->coeff_bgblur.stSrcOri.stride[1] - 1);
    }

    if (IVE_IOC_MODE_BGBLUR_BLUR == config->coeff_bgblur.eBgBlurMode)
    {
        u64 u64Temp0C0Addr = 0, u64Temp0C1Addr = 0, u64Temp1C0Addr = 0, u64Temp1C1Addr = 0;
        u64 u64Temp2C0Addr = 0, u64Temp2C1Addr = 0, u64Temp3C0Addr = 0, u64Temp3C1Addr = 0;
        u64BgSoriC0Addr                             = config->coeff_bgblur.stSrcRepBg.address[0];
        u64BgSoriC1Addr                             = config->coeff_bgblur.stSrcRepBg.address[1];
        handle->reg_bank3.reg_bgb_src1_c0_addr_low  = ADDR_LOW(u64BgSoriC0Addr);
        handle->reg_bank3.reg_bgb_src1_c0_addr_high = ADDR_HIGH(u64BgSoriC0Addr);
        handle->reg_bank3.reg_bgb_src1_c0_addr_h    = ADDR_HIGH_EXT(u64BgSoriC0Addr);
        handle->reg_bank3.reg_bgb_src1_c0_stride    = ALIGN_UP(4, config->coeff_bgblur.stSrcRepBg.stride[0] - 1);
        handle->reg_bank3.reg_bgb_src1_c1_addr_low  = ADDR_LOW(u64BgSoriC1Addr);
        handle->reg_bank3.reg_bgb_src1_c1_addr_high = ADDR_HIGH(u64BgSoriC1Addr);
        handle->reg_bank3.reg_bgb_src1_c1_addr_h    = ADDR_HIGH_EXT(u64BgSoriC1Addr);
        handle->reg_bank3.reg_bgb_src1_c1_stride    = ALIGN_UP(4, config->coeff_bgblur.stSrcRepBg.stride[1] - 1);

        u64Temp0C0Addr                               = config->coeff_bgblur.stMarkFillImg.address[0];
        u64Temp0C1Addr                               = config->coeff_bgblur.stMarkFillImg.address[1];
        handle->reg_bank3.reg_bgb_temp0_c0_addr_low  = ADDR_LOW(u64Temp0C0Addr);
        handle->reg_bank3.reg_bgb_temp0_c0_addr_high = ADDR_HIGH(u64Temp0C0Addr);
        handle->reg_bank3.reg_bgb_temp0_c0_addr_h    = ADDR_HIGH_EXT(u64Temp0C0Addr);
        handle->reg_bank3.reg_bgb_temp0_c0_stride    = ALIGN_UP(4, config->coeff_bgblur.stMarkFillImg.stride[0] - 1);
        handle->reg_bank3.reg_bgb_temp0_c1_addr_low  = ADDR_LOW(u64Temp0C1Addr);
        handle->reg_bank3.reg_bgb_temp0_c1_addr_high = ADDR_HIGH(u64Temp0C1Addr);
        handle->reg_bank3.reg_bgb_temp0_c1_addr_h    = ADDR_HIGH_EXT(u64Temp0C1Addr);
        handle->reg_bank3.reg_bgb_temp0_c1_stride    = ALIGN_UP(4, config->coeff_bgblur.stMarkFillImg.stride[1] - 1);

        u64Temp1C0Addr                               = config->coeff_bgblur.stSmallImg.address[0];
        u64Temp1C1Addr                               = config->coeff_bgblur.stSmallImg.address[1];
        handle->reg_bank3.reg_bgb_temp1_c0_addr_low  = ADDR_LOW(u64Temp1C0Addr);
        handle->reg_bank3.reg_bgb_temp1_c0_addr_high = ADDR_HIGH(u64Temp1C0Addr);
        handle->reg_bank3.reg_bgb_temp1_c0_addr_h    = ADDR_HIGH_EXT(u64Temp1C0Addr);
        handle->reg_bank3.reg_bgb_temp1_c0_stride    = ALIGN_UP(4, config->coeff_bgblur.stSmallImg.stride[0] - 1);
        handle->reg_bank3.reg_bgb_temp1_c1_addr_low  = ADDR_LOW(u64Temp1C1Addr);
        handle->reg_bank3.reg_bgb_temp1_c1_addr_high = ADDR_HIGH(u64Temp1C1Addr);
        handle->reg_bank3.reg_bgb_temp1_c1_addr_h    = ADDR_HIGH_EXT(u64Temp1C1Addr);
        handle->reg_bank3.reg_bgb_temp1_c1_stride    = ALIGN_UP(4, config->coeff_bgblur.stSmallImg.stride[1] - 1);

        u64Temp2C0Addr                               = config->coeff_bgblur.stBlurImg.address[0];
        u64Temp2C1Addr                               = config->coeff_bgblur.stBlurImg.address[1];
        handle->reg_bank3.reg_bgb_temp2_c0_addr_low  = ADDR_LOW(u64Temp2C0Addr);
        handle->reg_bank3.reg_bgb_temp2_c0_addr_high = ADDR_HIGH(u64Temp2C0Addr);
        handle->reg_bank3.reg_bgb_temp2_c0_addr_h    = ADDR_HIGH_EXT(u64Temp2C0Addr);
        handle->reg_bank3.reg_bgb_temp2_c0_stride    = ALIGN_UP(4, config->coeff_bgblur.stBlurImg.stride[0] - 1);
        handle->reg_bank3.reg_bgb_temp2_c1_addr_low  = ADDR_LOW(u64Temp2C1Addr);
        handle->reg_bank3.reg_bgb_temp2_c1_addr_high = ADDR_HIGH(u64Temp2C1Addr);
        handle->reg_bank3.reg_bgb_temp2_c1_addr_h    = ADDR_HIGH_EXT(u64Temp2C1Addr);
        handle->reg_bank3.reg_bgb_temp2_c1_stride    = ALIGN_UP(4, config->coeff_bgblur.stBlurImg.stride[1] - 1);

        u64Temp3C0Addr                               = config->coeff_bgblur.stAlphaBlendingImg.address[0];
        u64Temp3C1Addr                               = config->coeff_bgblur.stAlphaBlendingImg.address[1];
        handle->reg_bank3.reg_bgb_temp3_c0_addr_low  = ADDR_LOW(u64Temp3C0Addr);
        handle->reg_bank3.reg_bgb_temp3_c0_addr_high = ADDR_HIGH(u64Temp3C0Addr);
        handle->reg_bank3.reg_bgb_temp3_c0_addr_h    = ADDR_HIGH_EXT(u64Temp3C0Addr);
        handle->reg_bank3.reg_bgb_temp3_c0_stride = ALIGN_UP(4, config->coeff_bgblur.stAlphaBlendingImg.stride[0] - 1);
        handle->reg_bank3.reg_bgb_temp3_c1_addr_low  = ADDR_LOW(u64Temp3C1Addr);
        handle->reg_bank3.reg_bgb_temp3_c1_addr_high = ADDR_HIGH(u64Temp3C1Addr);
        handle->reg_bank3.reg_bgb_temp3_c1_addr_h    = ADDR_HIGH_EXT(u64Temp3C1Addr);
        handle->reg_bank3.reg_bgb_temp3_c1_stride = ALIGN_UP(4, config->coeff_bgblur.stAlphaBlendingImg.stride[1] - 1);
    }
    else
    {
        u64BgSoriC0Addr                             = config->coeff_bgblur.stSrcRepBg.address[0];
        handle->reg_bank3.reg_bgb_src2_c0_addr_low  = ADDR_LOW(config->coeff_bgblur.stSrcRepBg.address[0]);
        handle->reg_bank3.reg_bgb_src2_c0_addr_high = ADDR_HIGH(config->coeff_bgblur.stSrcRepBg.address[0]);
        handle->reg_bank3.reg_bgb_src2_c0_addr_h    = ADDR_HIGH_EXT(config->coeff_bgblur.stSrcRepBg.address[0]);
        handle->reg_bank3.reg_bgb_src2_c0_stride    = ALIGN_UP(4, config->coeff_bgblur.stSrcRepBg.stride[0] - 1);
        if (bYUV420SPFlag)
        {
            u64BgSoriC1Addr                             = config->coeff_bgblur.stSrcRepBg.address[1];
            handle->reg_bank3.reg_bgb_src2_c1_addr_low  = ADDR_LOW(config->coeff_bgblur.stSrcRepBg.address[1]);
            handle->reg_bank3.reg_bgb_src2_c1_addr_high = ADDR_HIGH(config->coeff_bgblur.stSrcRepBg.address[1]);
            handle->reg_bank3.reg_bgb_src2_c1_addr_h    = ADDR_HIGH_EXT(config->coeff_bgblur.stSrcRepBg.address[1]);
            handle->reg_bank3.reg_bgb_src2_c1_stride    = ALIGN_UP(4, config->coeff_bgblur.stSrcRepBg.stride[1] - 1);
        }
    }

    u64DstC0Addr                               = config->output.address[0];
    handle->reg_bank3.reg_bgb_dst_c0_addr_low  = ADDR_LOW(u64DstC0Addr);
    handle->reg_bank3.reg_bgb_dst_c0_addr_high = ADDR_HIGH(u64DstC0Addr);
    handle->reg_bank3.reg_bgb_dst_c0_addr_h    = ADDR_HIGH_EXT(u64DstC0Addr);
    handle->reg_bank3.reg_bgb_dst_c0_stride    = ALIGN_UP(4, config->output.stride[0] - 1);
    if (bYUV420SPFlag)
    {
        u64DstC1Addr                               = config->output.address[1];
        handle->reg_bank3.reg_bgb_dst_c1_addr_low  = ADDR_LOW(u64DstC1Addr);
        handle->reg_bank3.reg_bgb_dst_c1_addr_high = ADDR_HIGH(u64DstC1Addr);
        handle->reg_bank3.reg_bgb_dst_c1_addr_h    = ADDR_HIGH_EXT(u64DstC1Addr);
        handle->reg_bank3.reg_bgb_dst_c1_stride    = ALIGN_UP(4, config->output.stride[1] - 1);
    }

    if (IVE_IOC_MODE_BGBLUR_BLUR == config->coeff_bgblur.eBgBlurMode)
    {
        handle->reg_bank3.reg_bgb_img1_width  = config->coeff_bgblur.stSmallImg.width - 1;
        handle->reg_bank3.reg_bgb_img1_height = config->coeff_bgblur.stSmallImg.height - 1;
    }
    else
    {
        handle->reg_bank3.reg_bgb_img2_width  = config->coeff_bgblur.stSrcRepBg.width - 1;
        handle->reg_bank3.reg_bgb_img2_height = config->coeff_bgblur.stSrcRepBg.height - 1;
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
    REGW(handle->base_addr3, 0x31, handle->reg_bank3.reg31);
    REGW(handle->base_addr3, 0x32, handle->reg_bank3.reg32);
    REGW(handle->base_addr3, 0x33, handle->reg_bank3.reg33);
    REGW(handle->base_addr3, 0x34, handle->reg_bank3.reg34);
    REGW(handle->base_addr3, 0x35, handle->reg_bank3.reg35);
    REGW(handle->base_addr3, 0x36, handle->reg_bank3.reg36);
    REGW(handle->base_addr3, 0x37, handle->reg_bank3.reg37);
    REGW(handle->base_addr3, 0x38, handle->reg_bank3.reg38);
    REGW(handle->base_addr3, 0x39, handle->reg_bank3.reg39);
    REGW(handle->base_addr3, 0x3a, handle->reg_bank3.reg3a);
    REGW(handle->base_addr3, 0x3b, handle->reg_bank3.reg3b);
    REGW(handle->base_addr3, 0x3c, handle->reg_bank3.reg3c);
    REGW(handle->base_addr3, 0x3d, handle->reg_bank3.reg3d);
    REGW(handle->base_addr3, 0x3e, handle->reg_bank3.reg3e);
    REGW(handle->base_addr3, 0x3f, handle->reg_bank3.reg3f);
    REGW(handle->base_addr3, 0x40, handle->reg_bank3.reg40);
    REGW(handle->base_addr3, 0x41, handle->reg_bank3.reg41);
    REGW(handle->base_addr3, 0x42, handle->reg_bank3.reg42);
    REGW(handle->base_addr3, 0x43, handle->reg_bank3.reg43);
    REGW(handle->base_addr3, 0x44, handle->reg_bank3.reg44);
    REGW(handle->base_addr3, 0x45, handle->reg_bank3.reg45);
    REGW(handle->base_addr3, 0x46, handle->reg_bank3.reg46);
    REGW(handle->base_addr3, 0x47, handle->reg_bank3.reg47);
    REGW(handle->base_addr3, 0x48, handle->reg_bank3.reg48);
    REGW(handle->base_addr3, 0x49, handle->reg_bank3.reg49);
    REGW(handle->base_addr3, 0x4a, handle->reg_bank3.reg4a);
    REGW(handle->base_addr3, 0x4b, handle->reg_bank3.reg4b);
    REGW(handle->base_addr3, 0x4c, handle->reg_bank3.reg4c);
    REGW(handle->base_addr3, 0x4d, handle->reg_bank3.reg4d);
    REGW(handle->base_addr3, 0x4e, handle->reg_bank3.reg4e);
    REGW(handle->base_addr3, 0x4f, handle->reg_bank3.reg4f);
    REGW(handle->base_addr3, 0x50, handle->reg_bank3.reg50);
    REGW(handle->base_addr3, 0x51, handle->reg_bank3.reg51);
    REGW(handle->base_addr3, 0x52, handle->reg_bank3.reg52);
    REGW(handle->base_addr3, 0x53, handle->reg_bank3.reg53);
    REGW(handle->base_addr3, 0x54, handle->reg_bank3.reg54);
    REGW(handle->base_addr3, 0x55, handle->reg_bank3.reg55);
    REGW(handle->base_addr3, 0x56, handle->reg_bank3.reg56);
    REGW(handle->base_addr3, 0x57, handle->reg_bank3.reg57);
    REGW(handle->base_addr3, 0x58, handle->reg_bank3.reg58);
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
    REGW(handle->base_addr3, 0x65, handle->reg_bank3.reg65);
    REGW(handle->base_addr3, 0x66, handle->reg_bank3.reg66);
    REGW(handle->base_addr3, 0x67, handle->reg_bank3.reg67);
    REGW(handle->base_addr3, 0x68, handle->reg_bank3.reg68);
    REGW(handle->base_addr3, 0x6a, handle->reg_bank3.reg6a);
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
    handle->reg_bank1.thresh_16bit_1 = coeff->thresh - 1;

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
#if defined(SUPPORT_BAT_AWLEN_PATCH)
    u16 u16CurrAwlen = REGR(handle->base_addr0, 0x4e);

    /*
     * write cmd count of dst1 = the height x 8 bits / busrt length, burst length is 4/8/16/32 x 128 bits.
     * write cmd count of dst2 = the width  x 8 bits / busrt length
     *
     * Bug prerequisite:
     * 1.write cmd count of dst1 == 1
     * 2.write cmd count of dst2 > 1
     *
     * if reg_axi_aw_max_outstanding == 0, result will be NG, dst2 output is imcomplete.
     * if reg_axi_aw_max_outstanding > 0, result may be NG.
     */
    if (handle->reg_bank1.frame_height <= AXI_CMD_DATA_LENGTH(u16CurrAwlen)
        && handle->reg_bank1.frame_width > AXI_CMD_DATA_LENGTH(u16CurrAwlen))
    {
        handle->reg_bank0.reg_axi_awlen = 0;
        REGW(handle->base_addr0, 0x4e, handle->reg_bank0.reg4e);

        if (REGR(handle->base_addr0, 0x50) == 0)
        {
            IVE_MSG(IVE_MSG_WRN, "force to set axi_aw_max_outstanding from 0 to 1.\n");
            handle->reg_bank0.reg_axi_aw_max_outstanding = 1;
            REGW(handle->base_addr0, 0x50, handle->reg_bank0.reg50);
        }
    }
#endif
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
 * _ive_hal_set_coeff_ccl
 *   Set ccl coefficient
 *
 * Parameters:
 *   handle: IVE HAL handle
 *   coeff: coefficient
 *
 * Return:
 *   none
 */
static void _ive_hal_set_coeff_ccl(ive_hal_handle *handle, ive_ioc_coeff_ccl *coeff)
{
    handle->reg_bank1.infmt                    = 0;
    handle->reg_bank1.outfmt                   = 0;
    handle->reg_bank2.reg_ive_ccl_mode         = coeff->eCclMode;
    handle->reg_bank2.reg_ive_ccl_init_area_th = coeff->u16InitAreaThr;
    handle->reg_bank2.reg_ive_ccl_step         = coeff->u16Step;

    REGW(handle->base_addr1, 0x05, handle->reg_bank1.reg05);
    REGW(handle->base_addr2, 0x00, handle->reg_bank2.reg00);
    REGW(handle->base_addr2, 0x01, handle->reg_bank2.reg01);
    REGW(handle->base_addr2, 0x02, handle->reg_bank2.reg02);
}

/*******************************************************************************************************************
 * _ive_hal_set_coeff_gmm
 *   Set gmm coefficient
 *
 * Parameters:
 *   handle: IVE HAL handle
 *   coeff: coefficient
 *
 * Return:
 *   none
 */
static void _ive_hal_set_coeff_gmm(ive_hal_handle *handle, ive_ioc_coeff_gmm *coeff)
{
    u64 u64InitVarOfModel = (IVE_IOC_IMAGE_FORMAT_B8C1 == handle->reg_bank1.infmt) ? GMM_INIT_VAR_OF_MODEL_U8C1
                                                                                   : GMM_INIT_VAR_OF_MODEL_U8C3PACK;
    handle->reg_bank2.reg_ive_gmm_need_to_init    = coeff->bRestartLearning;
    handle->reg_bank2.reg_ive_gmm_max_var_low     = ADDR_LOW(coeff->u64MaxVariance);
    handle->reg_bank2.reg_ive_gmm_max_var_high    = ADDR_HIGH(coeff->u64MaxVariance);
    handle->reg_bank2.reg_ive_gmm_min_var_low     = ADDR_LOW(coeff->u64MinVariance);
    handle->reg_bank2.reg_ive_gmm_min_var_high    = ADDR_HIGH(coeff->u64MinVariance);
    handle->reg_bank2.reg_ive_gmm_learn_rate      = coeff->u16LearnRate;
    handle->reg_bank2.reg_ive_gmm_bg_ratio        = coeff->u16BgRatio;
    handle->reg_bank2.reg_ive_gmm_var_thr_low     = ADDR_LOW(coeff->u64VarianceThr);
    handle->reg_bank2.reg_ive_gmm_var_thr_high    = ADDR_HIGH(coeff->u64VarianceThr);
    handle->reg_bank2.reg_ive_gmm_init_var_low    = ADDR_LOW(u64InitVarOfModel);
    handle->reg_bank2.reg_ive_gmm_init_var_high   = ADDR_HIGH(u64InitVarOfModel);
    handle->reg_bank2.reg_ive_gmm_model_num       = coeff->u8ModelNum;
    handle->reg_bank2.reg_ive_gmm_var_thr_bg_low  = ADDR_LOW(GMM_VAR_THR_BG);
    handle->reg_bank2.reg_ive_gmm_var_thr_bg_high = ADDR_HIGH(GMM_VAR_THR_BG);
    handle->reg_bank2.reg_ive_gmm_ct              = GMM_COMPLEXITY_REDUCTION_THR;
    handle->reg_bank2.reg_ive_gmm_frame_mode      = coeff->eFrameMode;

    REGW(handle->base_addr2, 0x10, handle->reg_bank2.reg10);
    REGW(handle->base_addr2, 0x11, handle->reg_bank2.reg11);
    REGW(handle->base_addr2, 0x12, handle->reg_bank2.reg12);
    REGW(handle->base_addr2, 0x13, handle->reg_bank2.reg13);
    REGW(handle->base_addr2, 0x14, handle->reg_bank2.reg14);
    REGW(handle->base_addr2, 0x15, handle->reg_bank2.reg15);
    REGW(handle->base_addr2, 0x16, handle->reg_bank2.reg16);
    REGW(handle->base_addr2, 0x17, handle->reg_bank2.reg17);
    REGW(handle->base_addr2, 0x18, handle->reg_bank2.reg18);
    REGW(handle->base_addr2, 0x19, handle->reg_bank2.reg19);
    REGW(handle->base_addr2, 0x1a, handle->reg_bank2.reg1a);
    REGW(handle->base_addr2, 0x1b, handle->reg_bank2.reg1b);
    REGW(handle->base_addr2, 0x1c, handle->reg_bank2.reg1c);
    REGW(handle->base_addr2, 0x1d, handle->reg_bank2.reg1d);
    REGW(handle->base_addr2, 0x1e, handle->reg_bank2.reg1e);
    REGW(handle->base_addr2, 0x1f, handle->reg_bank2.reg1f);
}

/*******************************************************************************************************************
 * _ive_hal_set_coeff_resize
 *   Set Resize coefficient
 *
 * Parameters:
 *   handle: IVE HAL handle
 *   coeff: coefficient
 *
 * Return:
 *   none
 */
static void _ive_hal_set_coeff_resize(ive_hal_handle *handle, ive_ioc_coeff_resize *coeff)
{
    u32 u32SrcWidth         = handle->reg_bank1.frame_width + 1;
    u32 u32SrcHeight        = handle->reg_bank1.frame_height + 1;
    u32 u32DstWidth         = handle->reg_bank1.reg_ive_dst_frame_width + 1;
    u32 u32DstHeight        = handle->reg_bank1.reg_ive_dst_frame_height + 1;
    u32 u32ResizeAreaXCoeff = 0, u32ResizeAreaYCoeff = 0, u32ResizeRatopX = 0, u32ResizeRatopY = 0;

    handle->reg_bank1.infmt               = coeff->eResizeFmt;
    handle->reg_bank1.outfmt              = coeff->eResizeFmt;
    handle->reg_bank2.reg_ive_resize_mode = coeff->eResizeMode;
    if (handle->reg_bank2.reg_ive_resize_mode)
    {
        u32ResizeAreaXCoeff = ((u32DstWidth << 18) / u32SrcWidth + 1) / 2;
        u32ResizeAreaYCoeff = ((u32DstHeight << 18) / u32SrcHeight + 1) / 2;
    }
    else
    {
        u32ResizeRatopX = ((u32SrcWidth << 18) / u32DstWidth + 1) / 2;
        u32ResizeRatopY = ((u32SrcHeight << 18) / u32DstHeight + 1) / 2;
    }

    handle->reg_bank2.reg_ive_resize_area_xcoef_low  = ADDR_LOW(u32ResizeAreaXCoeff);
    handle->reg_bank2.reg_ive_resize_area_xcoef_high = ADDR_HIGH(u32ResizeAreaXCoeff);
    handle->reg_bank2.reg_ive_resize_area_ycoef_low  = ADDR_LOW(u32ResizeAreaYCoeff);
    handle->reg_bank2.reg_ive_resize_area_ycoef_high = ADDR_HIGH(u32ResizeAreaYCoeff);
    handle->reg_bank2.reg_ive_resize_ratio_x_low     = ADDR_LOW(u32ResizeRatopX);
    handle->reg_bank2.reg_ive_resize_ratio_x_high    = ADDR_HIGH(u32ResizeRatopX);
    handle->reg_bank2.reg_ive_resize_ratio_y_low     = ADDR_LOW(u32ResizeRatopY);
    handle->reg_bank2.reg_ive_resize_ratio_y_high    = ADDR_HIGH(u32ResizeRatopY);

    REGW(handle->base_addr1, 0x05, handle->reg_bank1.reg05);
    REGW(handle->base_addr2, 0x28, handle->reg_bank2.reg28);
    REGW(handle->base_addr2, 0x29, handle->reg_bank2.reg29);
    REGW(handle->base_addr2, 0x2a, handle->reg_bank2.reg2a);
    REGW(handle->base_addr2, 0x2b, handle->reg_bank2.reg2b);
    REGW(handle->base_addr2, 0x2c, handle->reg_bank2.reg2c);
    REGW(handle->base_addr2, 0x2d, handle->reg_bank2.reg2d);
    REGW(handle->base_addr2, 0x2e, handle->reg_bank2.reg2e);
    REGW(handle->base_addr2, 0x2f, handle->reg_bank2.reg2f);
    REGW(handle->base_addr2, 0x30, handle->reg_bank2.reg30);
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
    u32 u32Xcoef = 0, u32Ycoef = 0, u32RatioX = 0, u32RatioY = 0;
    u64 u64TempWidth0 = 0, u64TempHeight0 = 0, u64TempWidth1 = 0, u64TempHeight1 = 0;

    handle->reg_bank3.reg_bgb_op_type = coeff->eBgBlurMode;

    handle->reg_bank3.reg_bgb_mask_th_sml = coeff->u8MaskThr;
    handle->reg_bank3.reg_bgb_blur_alpha  = coeff->u8AlphaBlendingVal;

    if (IVE_IOC_MODE_BGBLUR_BLUR == coeff->eBgBlurMode)
    {
        u64TempWidth0  = coeff->stSmallImg.width;
        u64TempHeight0 = coeff->stSmallImg.height;
        u64TempWidth1  = coeff->stSrcYMask.width;
        u64TempHeight1 = coeff->stSrcYMask.height;

        u32Xcoef = ((u64TempWidth0 << 18) / u64TempWidth1 + 1) / 2;
        u32Ycoef = ((u64TempHeight0 << 18) / u64TempHeight1 + 1) / 2;

        handle->reg_bank3.reg_bgb_g1_resize_area_xcoef_low  = ADDR_LOW(u32Xcoef);
        handle->reg_bank3.reg_bgb_g1_resize_area_xcoef_high = ADDR_HIGH(u32Xcoef);
        handle->reg_bank3.reg_bgb_g1_resize_area_ycoef_low  = ADDR_LOW(u32Ycoef);
        handle->reg_bank3.reg_bgb_g1_resize_area_ycoef_high = ADDR_HIGH(u32Ycoef);

        u32RatioX = ((u64TempWidth0 << 18) / u64TempWidth1 + 1) / 2;
        u32RatioY = ((u64TempHeight0 << 18) / u64TempHeight1 + 1) / 2;

        handle->reg_bank3.reg_bgb_g3_resize_ratio_x_low  = ADDR_LOW(u32RatioX);
        handle->reg_bank3.reg_bgb_g3_resize_ratio_x_high = ADDR_HIGH(u32RatioX);
        handle->reg_bank3.reg_bgb_g3_resize_ratio_y_low  = ADDR_LOW(u32RatioY);
        handle->reg_bank3.reg_bgb_g3_resize_ratio_y_high = ADDR_HIGH(u32RatioY);
    }

    u64TempWidth1  = coeff->stSrcOri.width;
    u64TempHeight1 = coeff->stSrcOri.height;

    if (IVE_IOC_MODE_BGBLUR_BLUR == coeff->eBgBlurMode)
    {
        u64TempWidth0  = coeff->stSrcYMask.width;
        u64TempHeight0 = coeff->stSrcYMask.height;

        u32RatioX = ((u64TempWidth0 << 18) / u64TempWidth1 + 1) / 2;
        u32RatioY = ((u64TempHeight0 << 18) / u64TempHeight1 + 1) / 2;
    }
    else
    {
        u64TempWidth0  = coeff->stSrcRepBg.width;
        u64TempHeight0 = coeff->stSrcRepBg.height;

        u32RatioX = ((u64TempWidth0 << 18) / u64TempWidth1 + 1) / 2;
        u32RatioY = ((u64TempHeight0 << 18) / u64TempHeight1 + 1) / 2;
    }

    handle->reg_bank3.reg_bgb_g4_resize0_ratio_x_low  = ADDR_LOW(u32RatioX);
    handle->reg_bank3.reg_bgb_g4_resize0_ratio_x_high = ADDR_HIGH(u32RatioX);
    handle->reg_bank3.reg_bgb_g4_resize0_ratio_y_low  = ADDR_LOW(u32RatioY);
    handle->reg_bank3.reg_bgb_g4_resize0_ratio_y_high = ADDR_HIGH(u32RatioY);

    u64TempWidth0  = coeff->stSrcYMask.width;
    u64TempHeight0 = coeff->stSrcYMask.height;

    u32RatioX = ((u64TempWidth0 << 18) / u64TempWidth1 + 1) / 2;
    u32RatioY = ((u64TempHeight0 << 18) / u64TempHeight1 + 1) / 2;

    handle->reg_bank3.reg_bgb_g4_resize1_ratio_x_low  = ADDR_LOW(u32RatioX);
    handle->reg_bank3.reg_bgb_g4_resize1_ratio_x_high = ADDR_HIGH(u32RatioX);
    handle->reg_bank3.reg_bgb_g4_resize1_ratio_y_low  = ADDR_LOW(u32RatioY);
    handle->reg_bank3.reg_bgb_g4_resize1_ratio_y_high = ADDR_HIGH(u32RatioY);

    // uv resize y.
    if (coeff->stSrcOri.format == IVE_IOC_IMAGE_FORMAT_422YUYV)
    {
        u64TempHeight0 = coeff->stSrcYMask.height / 2;
        u64TempHeight1 = coeff->stSrcOri.height;

        u32RatioY = ((u64TempHeight0 << 18) / u64TempHeight1 + 1) / 2;

        handle->reg_bank3.reg_bgb_g4_uv_resize_ratio_y_low  = ADDR_LOW(u32RatioY);
        handle->reg_bank3.reg_bgb_g4_uv_resize_ratio_y_high = ADDR_HIGH(u32RatioY);
    }

    REGW(handle->base_addr3, 0x13, handle->reg_bank3.reg13);
    REGW(handle->base_addr3, 0x69, handle->reg_bank3.reg69);
    REGW(handle->base_addr3, 0x6c, handle->reg_bank3.reg6c);
    REGW(handle->base_addr3, 0x6d, handle->reg_bank3.reg6d);
    REGW(handle->base_addr3, 0x6e, handle->reg_bank3.reg6e);
    REGW(handle->base_addr3, 0x6f, handle->reg_bank3.reg6f);
    REGW(handle->base_addr3, 0x70, handle->reg_bank3.reg70);
    REGW(handle->base_addr3, 0x71, handle->reg_bank3.reg71);
    REGW(handle->base_addr3, 0x72, handle->reg_bank3.reg72);
    REGW(handle->base_addr3, 0x73, handle->reg_bank3.reg73);
    REGW(handle->base_addr3, 0x74, handle->reg_bank3.reg74);
    REGW(handle->base_addr3, 0x75, handle->reg_bank3.reg75);
    REGW(handle->base_addr3, 0x76, handle->reg_bank3.reg76);
    REGW(handle->base_addr3, 0x77, handle->reg_bank3.reg77);
    REGW(handle->base_addr3, 0x78, handle->reg_bank3.reg78);
    REGW(handle->base_addr3, 0x79, handle->reg_bank3.reg79);
    REGW(handle->base_addr3, 0x7a, handle->reg_bank3.reg7a);
    REGW(handle->base_addr3, 0x7b, handle->reg_bank3.reg7b);
    REGW(handle->base_addr3, 0x7c, handle->reg_bank3.reg7c);
    REGW(handle->base_addr3, 0x7d, handle->reg_bank3.reg7d);
    REGW(handle->base_addr3, 0x7e, handle->reg_bank3.reg7e);
    REGW(handle->base_addr3, 0x7f, handle->reg_bank3.reg7f);
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

    // register value 0/1/2/3 means burst length 4/8/16/32.
    if (u8SysArlen)
    {
        u8RealArlen = u8SysArlen > 8 ? (u8SysArlen > 16 ? 3 : 2) : (u8SysArlen > 4 ? 1 : 0);
    }

    if (u8SysAwlen)
    {
        u8RealAwlen = u8SysAwlen > 8 ? (u8SysAwlen > 16 ? 3 : 2) : (u8SysAwlen > 4 ? 1 : 0);
    }

    handle->reg_bank0.reg_axi_arlen              = u8RealArlen;
    handle->reg_bank0.reg_axi_awlen              = u8RealAwlen;
    handle->reg_bank0.reg_axi_ar_max_outstanding = u8SysArOut ? (u8SysArOut - 1) : AXI_AR_MAX_OUTSTANDING_DEFAULT;
    handle->reg_bank0.reg_axi_aw_max_outstanding = u8SysAwOut ? (u8SysAwOut - 1) : AXI_AR_MAX_OUTSTANDING_DEFAULT;

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
    ive_hal_dummy_eco_set(handle);

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
    handle->reg_bank0.sw_rst         = 1;
    handle->reg_bank3.reg_bgb_sw_rst = 1;

    REGW(handle->base_addr0, 0x02, handle->reg_bank0.reg02);
    REGW(handle->base_addr3, 0x01, handle->reg_bank3.reg01);

    /*
     * when MCM enable and clk_miu is divided by 16, the reset time duration of clk_miu will increase.
     * in this situation, insufficient reset time duration may only trigger clk_ive reset, this behavior will cause
     * clk_miu generate frame_end impluse, and next task will get interrupt immediately and output empty result.
     *
     * here add soft reset time duration to ensure clk_ive and clk_miu domain are reset successfully.
     * soft reset time duration should be at least 2 * 16 * / min(clk_ive, clk_miu/n), n is slow down ratio of MCM.
     */
    CamOsUsDelay(10);

    handle->reg_bank0.sw_rst         = 0; // write one clear
    handle->reg_bank3.reg_bgb_sw_rst = 0;

    REGW(handle->base_addr0, 0x02, handle->reg_bank0.reg02);
    REGW(handle->base_addr3, 0x01, handle->reg_bank3.reg01);

    memset(&handle->reg_bank0, 0, sizeof(handle->reg_bank0));
    memset(&handle->reg_bank1, 0, sizeof(handle->reg_bank1));
    memset(&handle->reg_bank2, 0, sizeof(handle->reg_bank2));
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

        case IVE_IOC_OP_TYPE_RESIZE:
            *peCheckFlag = IVE_CONFIG_WH_CHECK_INPUT | IVE_CONFIG_WH_CHECK_OUTPUT;
            *pu16MinW    = MIN_W_RESIZE;
            *pu16MinH    = MIN_H_RESIZE;
            *pu16MaxW    = MAX_W_RESIZE;
            *pu16MaxH    = MAX_H_RESIZE;
            break;

        // check input / output w/h and w/h match.
        case IVE_IOC_OP_TYPE_CCL:
            *peCheckFlag = IVE_CONFIG_WH_CHECK_MATCH | IVE_CONFIG_WH_CHECK_INPUT;
            *pu16MinW    = MIN_W_CCL;
            *pu16MinH    = MIN_H_CCL;
            *pu16MaxW    = MAX_W_CCL;
            *pu16MaxH    = MAX_H_CCL;
            break;

        case IVE_IOC_OP_TYPE_GMM:
            *peCheckFlag = IVE_CONFIG_WH_CHECK_MATCH | IVE_CONFIG_WH_CHECK_INPUT;
            *pu16MinW    = MIN_W_GMM;
            *pu16MinH    = MIN_H_GMM;
            *pu16MaxW    = MAX_W_GMM;
            *pu16MaxH    = MAX_H_GMM;
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

        case IVE_IOC_OP_TYPE_CCL:
            _ive_hal_set_coeff_ccl(pstHandle, &pstConfig->coeff_ccl);
            break;

        case IVE_IOC_OP_TYPE_GMM:
            _ive_hal_set_coeff_gmm(pstHandle, &pstConfig->coeff_gmm);
            break;

        case IVE_IOC_OP_TYPE_RESIZE:
            _ive_hal_set_coeff_resize(pstHandle, &pstConfig->coeff_resize);
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
