/* SigmaStar trade secret */
/* Copyright (c) [2019~2020] SigmaStar Technology.
All rights reserved.

Unless otherwise stipulated in writing, any and all information contained
herein regardless in any format shall remain the sole proprietary of
SigmaStar and be kept in strict confidence
(SigmaStar Confidential Information) by the recipient.
Any unauthorized act including without limitation unauthorized disclosure,
copying, use, reproduction, sale, distribution, modification, disassembling,
reverse engineering and compiling of the contents of SigmaStar Confidential
Information is unlawful and strictly prohibited. SigmaStar hereby reserves the
rights to any and all damages, losses, costs and expenses resulting therefrom.
*/

#ifndef __AIO_INIT_H__
#define __AIO_INIT_H__

#ifndef AIO_INIT_OK
#define AIO_INIT_OK (0)
#endif

#ifndef AIO_INIT_NG
#define AIO_INIT_NG (-1)
#endif



typedef enum
{
    // u32
    E_INFO_TYPE_IRQ_NUM,

    E_INFO_TYPE_I2S_RX0_TDM_WS_PGM,
    E_INFO_TYPE_I2S_RX0_TDM_WS_WIDTH,
    E_INFO_TYPE_I2S_RX0_TDM_WS_INV,
    E_INFO_TYPE_I2S_RX0_TDM_BCK_INV,

    E_INFO_TYPE_I2S_TX0_TDM_WS_PGM,
    E_INFO_TYPE_I2S_TX0_TDM_WS_WIDTH,
    E_INFO_TYPE_I2S_TX0_TDM_WS_INV,
    E_INFO_TYPE_I2S_TX0_TDM_BCK_INV,
    E_INFO_TYPE_I2S_TX0_TDM_ACTIVE_SLOT,

    E_INFO_TYPE_DMIC_BCK_EXT_MODE,
    E_INFO_TYPE_KEEP_ADC_POWER_ON,
    E_INFO_TYPE_KEEP_DAC_POWER_ON,
    E_INFO_TYPE_I2S_RX_MODE,
    E_INFO_TYPE_I2S_RX_SHORT_FF_MODE,
    E_INFO_TYPE_I2S_TX_SHORT_FF_MODE,
    E_INFO_TYPE_DMIC_DELAY,
    E_INFO_TYPE_ADC_CH_INMUX,

    // array
    E_INFO_TYPE_AMP_GPIO,
    E_INFO_TYPE_I2S_RX0_TDM_CH_SWAP,
    E_INFO_TYPE_I2S_TX0_TDM_CH_SWAP,
    E_INFO_TYPE_HPF_ADC1_LEVEL,
    E_INFO_TYPE_HPF_ADC2_LEVEL,
    E_INFO_TYPE_HPF_DMIC_LEVEL,
    E_INFO_TYPE_DMIC_BCK_MODE,
    E_INFO_TYPE_ADC_OUT_SEL,
} InfoType_e;

MI_S32 _GetU32InfoFromDtsByType(InfoType_e eType, MI_U32 *pu32Value);
MI_S32 _GetArrayInfoFromDtsByType(InfoType_e eType, MI_U32 *pu32Array, MI_U32 u32ArrayItems);
MI_S32 AudClkEnable(MI_S8 clk_index, MI_S8 parent_clk_index, MI_BOOL bEn, MI_S64 nFreq);

#endif
