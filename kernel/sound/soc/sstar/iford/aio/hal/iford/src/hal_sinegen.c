/*
 * hal_sinegen.c - Sigmastar
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

#include "hal_audio_common.h"
#include "hal_audio_sys.h"
#include "hal_audio_reg.h"
#include "hal_audio_types.h"
#include "hal_audio_api.h"
#include "hal_audio_os_api.h"

int HalAudSineGenEnable(SINE_GEN_e enSineGen, BOOL bEn)
{
    U16 I2SSINMASK = AUDIO_BANK2_REG_I2S_TDM_CFG_03_REG_SINE_GEN_ENA_CH67_MASK
                     | AUDIO_BANK2_REG_I2S_TDM_CFG_03_REG_SINE_GEN_ENA_CH45_MASK
                     | AUDIO_BANK2_REG_I2S_TDM_CFG_03_REG_SINE_GEN_ENA_CH23_MASK
                     | AUDIO_BANK2_REG_I2S_TDM_CFG_03_REG_SINE_GEN_ENA_CH01_MASK;
    switch (enSineGen)
    {
        case E_MHAL_SINEGEN_AO_DMA_A:
            HalBachWriteReg(E_BACH_REG_BANK1, AUDIO_BANK1_REG_DMA_TEST_CTRL5_ADDR,
                            AUDIO_BANK1_REG_DMA_TEST_CTRL5_REG_SINE_GEN_EN_DMA1_MASK,
                            (bEn ? AUDIO_BANK1_REG_DMA_TEST_CTRL5_REG_SINE_GEN_EN_DMA1_MASK : 0));
            HalBachWriteReg(E_BACH_REG_BANK1, AUDIO_BANK1_REG_DMA_TEST_CTRL5_ADDR,
                            AUDIO_BANK1_REG_DMA_TEST_CTRL5_REG_SINE_GEN_L_DMA1_MASK
                                | AUDIO_BANK1_REG_DMA_TEST_CTRL5_REG_SINE_GEN_R_DMA1_MASK,
                            AUDIO_BANK1_REG_DMA_TEST_CTRL5_REG_SINE_GEN_L_DMA1_MASK
                                | AUDIO_BANK1_REG_DMA_TEST_CTRL5_REG_SINE_GEN_R_DMA1_MASK);
            HalBachWriteReg(E_BACH_REG_BANK1, AUDIO_BANK1_REG_DMA_TEST_CTRL5_ADDR,
                            AUDIO_BANK1_REG_DMA_TEST_CTRL5_REG_SINE_GEN_RD_WR_DMA1_MASK, 0);
            break;

        case E_MHAL_SINEGEN_AI_DMA_A:
            HalBachWriteReg(E_BACH_REG_BANK1, AUDIO_BANK1_REG_DMA_TEST_CTRL5_ADDR,
                            AUDIO_BANK1_REG_DMA_TEST_CTRL5_REG_SINE_GEN_EN_DMA1_MASK,
                            (bEn ? AUDIO_BANK1_REG_DMA_TEST_CTRL5_REG_SINE_GEN_EN_DMA1_MASK : 0));
            HalBachWriteReg(E_BACH_REG_BANK1, AUDIO_BANK1_REG_DMA_TEST_CTRL5_ADDR,
                            AUDIO_BANK1_REG_DMA_TEST_CTRL5_REG_SINE_GEN_L_DMA1_MASK
                                | AUDIO_BANK1_REG_DMA_TEST_CTRL5_REG_SINE_GEN_R_DMA1_MASK,
                            AUDIO_BANK1_REG_DMA_TEST_CTRL5_REG_SINE_GEN_L_DMA1_MASK
                                | AUDIO_BANK1_REG_DMA_TEST_CTRL5_REG_SINE_GEN_R_DMA1_MASK);
            HalBachWriteReg(E_BACH_REG_BANK1, AUDIO_BANK1_REG_DMA_TEST_CTRL5_ADDR,
                            AUDIO_BANK1_REG_DMA_TEST_CTRL5_REG_SINE_GEN_RD_WR_DMA1_MASK,
                            AUDIO_BANK1_REG_DMA_TEST_CTRL5_REG_SINE_GEN_RD_WR_DMA1_MASK);
            break;

        case E_MHAL_SINEGEN_AI_IF_DMIC_A:
            HalBachWriteReg(E_BACH_REG_BANK3, AUDIO_BANK3_REG_VREC_CTRL03_ADDR,
                            AUDIO_BANK3_REG_VREC_CTRL03_SIN_PATH_SEL_MASK,
                            AUDIO_BANK3_REG_VREC_CTRL03_SIN_PATH_SEL_MASK);
            HalBachWriteReg(E_BACH_REG_BANK3, AUDIO_BANK3_REG_VREC_CTRL03_ADDR, AUDIO_BANK3_REG_VREC_CTRL03_SIN_EN_MASK,
                            (bEn ? AUDIO_BANK3_REG_VREC_CTRL03_SIN_EN_MASK : 0));
            break;

        case E_MHAL_SINEGEN_AI_IF_AMIC_A:
            HalBachWriteReg(E_BACH_REG_BANK4, AUDIO_BANK4_REG_EXT_I2S_TEST_MD_ADDR,
                            AUDIO_BANK4_REG_EXT_I2S_TEST_MD_REG_SINE_GEN_ENA_ADC_L_MASK
                                | AUDIO_BANK4_REG_EXT_I2S_TEST_MD_REG_SINE_GEN_ENA_ADC_R_MASK,
                            (bEn ? AUDIO_BANK4_REG_EXT_I2S_TEST_MD_REG_SINE_GEN_ENA_ADC_L_MASK
                                       | AUDIO_BANK4_REG_EXT_I2S_TEST_MD_REG_SINE_GEN_ENA_ADC_R_MASK
                                 : 0));
            break;

        case E_MHAL_SINEGEN_AI_IF_I2S_A:
            HalBachWriteReg(E_BACH_REG_BANK2, AUDIO_BANK2_REG_I2S_TDM_CFG_03_ADDR, I2SSINMASK, (bEn ? I2SSINMASK : 0));
            break;

        default:
            ERRMSG("Func:%s, Line:%d enSineGen %d bEn %d Fail !\n", __FUNCTION__, __LINE__, enSineGen, bEn);
            goto FAIL;
            break;
    }

    return AIO_OK;
FAIL:
    return AIO_NG;
}

int HalAudSineGenSetting(SINE_GEN_e enSineGen, BOOL bEnable, U8 u8Freq, S8 s8Gain)
{
    int ret = AIO_OK;

    if (u8Freq > 15 || s8Gain > 15) // 4 bits setting for gain
    {
        ERRMSG("enSineGen %d u8Freq %d s8Gain %d Fail !\n", enSineGen, u8Freq, s8Gain);
        goto FAIL;
    }

    ret |= HalAudSineGenEnable(enSineGen, bEnable);
    ret |= HalAudSineGenSetFreq(enSineGen, u8Freq);
    ret |= HalAudSineGenSetGain(enSineGen, s8Gain);
    if (ret != AIO_OK)
    {
        ERRMSG("enSineGen %d u8Freq %d s8Gain %d Fail !\n", enSineGen, u8Freq, s8Gain);
        goto FAIL;
    }

    return AIO_OK;
FAIL:
    return AIO_NG;
}

int HalAudSineGenSetFreq(SINE_GEN_e enSineGen, U8 u8Freq)
{
    if (u8Freq > 15) // 4 bits setting for gain
    {
        ERRMSG("Func:%s, Line:%d enSineGen %d u8Freq %d Fail !\n", __FUNCTION__, __LINE__, enSineGen, u8Freq);
        goto FAIL;
    }

    switch (enSineGen)
    {
        case E_MHAL_SINEGEN_AO_DMA_A:
            HalBachWriteReg(E_BACH_REG_BANK1, AUDIO_BANK1_REG_DMA_TEST_CTRL5_ADDR,
                            AUDIO_BANK1_REG_DMA_TEST_CTRL5_REG_SINE_GEN_FREQ_DMA1_MASK,
                            u8Freq << AUDIO_BANK1_REG_DMA_TEST_CTRL5_REG_SINE_GEN_FREQ_DMA1_SHIFT);
            break;

        case E_MHAL_SINEGEN_AI_DMA_A:
            HalBachWriteReg(E_BACH_REG_BANK1, AUDIO_BANK1_REG_DMA_TEST_CTRL5_ADDR,
                            AUDIO_BANK1_REG_DMA_TEST_CTRL5_REG_SINE_GEN_FREQ_DMA1_MASK,
                            u8Freq << AUDIO_BANK1_REG_DMA_TEST_CTRL5_REG_SINE_GEN_FREQ_DMA1_SHIFT);
            break;

        case E_MHAL_SINEGEN_AI_IF_DMIC_A:
            HalBachWriteReg(E_BACH_REG_BANK3, AUDIO_BANK3_REG_VREC_CTRL03_ADDR,
                            AUDIO_BANK3_REG_VREC_CTRL03_SIN_FREQ_SEL_MASK,
                            u8Freq << AUDIO_BANK3_REG_VREC_CTRL03_SIN_FREQ_SEL_SHIFT);
            break;

        case E_MHAL_SINEGEN_AI_IF_AMIC_A:
            HalBachWriteReg(E_BACH_REG_BANK4, AUDIO_BANK4_REG_EXT_I2S_TEST_MD_ADDR,
                            AUDIO_BANK4_REG_EXT_I2S_TEST_MD_REG_SINE_GEN_FREQ_ADC_MASK,
                            u8Freq << AUDIO_BANK4_REG_EXT_I2S_TEST_MD_REG_SINE_GEN_FREQ_ADC_SHIFT);
            break;

        case E_MHAL_SINEGEN_AI_IF_I2S_A:
            HalBachWriteReg(E_BACH_REG_BANK2, AUDIO_BANK2_REG_I2S_TDM_CFG_03_ADDR,
                            AUDIO_BANK2_REG_I2S_TDM_CFG_03_REG_SINE_GEN_FREQ_MASK,
                            u8Freq << AUDIO_BANK2_REG_I2S_TDM_CFG_03_REG_SINE_GEN_FREQ_SHIFT);
            break;

        default:
            ERRMSG("Func:%s, Line:%d enSineGen %d u8Freq %d Fail !\n", __FUNCTION__, __LINE__, enSineGen, u8Freq);
            goto FAIL;
            break;
    }

    return AIO_OK;
FAIL:
    return AIO_NG;
}

int HalAudSineGenSetGain(SINE_GEN_e enSineGen, S8 s8Gain)
{
    if (s8Gain > 15) // 4 bits setting for gain
    {
        ERRMSG("Func:%s, Line:%d enSineGen %d s8Gain %d Fail !\n", __FUNCTION__, __LINE__, enSineGen, s8Gain);
        goto FAIL;
    }

    switch (enSineGen)
    {
        case E_MHAL_SINEGEN_AO_DMA_A:
            HalBachWriteReg(E_BACH_REG_BANK1, AUDIO_BANK1_REG_DMA_TEST_CTRL5_ADDR,
                            AUDIO_BANK1_REG_DMA_TEST_CTRL5_REG_SINE_GEN_GAIN_DMA1_MASK,
                            s8Gain << AUDIO_BANK1_REG_DMA_TEST_CTRL5_REG_SINE_GEN_GAIN_DMA1_SHIFT);
            break;

        case E_MHAL_SINEGEN_AI_DMA_A:
            HalBachWriteReg(E_BACH_REG_BANK1, AUDIO_BANK1_REG_DMA_TEST_CTRL5_ADDR,
                            AUDIO_BANK1_REG_DMA_TEST_CTRL5_REG_SINE_GEN_GAIN_DMA1_MASK,
                            s8Gain << AUDIO_BANK1_REG_DMA_TEST_CTRL5_REG_SINE_GEN_GAIN_DMA1_SHIFT);
            break;

        case E_MHAL_SINEGEN_AI_IF_DMIC_A:
            HalBachWriteReg(E_BACH_REG_BANK3, AUDIO_BANK3_REG_VREC_CTRL03_ADDR,
                            AUDIO_BANK3_REG_VREC_CTRL03_SIN_GAIN_MASK,
                            s8Gain << AUDIO_BANK3_REG_VREC_CTRL03_SIN_GAIN_SHIFT);
            break;

        case E_MHAL_SINEGEN_AI_IF_AMIC_A:
            HalBachWriteReg(E_BACH_REG_BANK4, AUDIO_BANK4_REG_EXT_I2S_TEST_MD_ADDR,
                            AUDIO_BANK4_REG_EXT_I2S_TEST_MD_REG_SINE_GEN_GAIN_ADC_MASK,
                            s8Gain << AUDIO_BANK4_REG_EXT_I2S_TEST_MD_REG_SINE_GEN_GAIN_ADC_SHIFT);
            break;

        case E_MHAL_SINEGEN_AI_IF_I2S_A:
            HalBachWriteReg(E_BACH_REG_BANK2, AUDIO_BANK2_REG_I2S_TDM_CFG_03_ADDR,
                            AUDIO_BANK2_REG_I2S_TDM_CFG_03_REG_SINE_GEN_GAIN_MASK,
                            s8Gain << AUDIO_BANK2_REG_I2S_TDM_CFG_03_REG_SINE_GEN_GAIN_SHIFT);
            break;

        default:
            ERRMSG("enSineGen %d s8Gain %d Fail !\n", enSineGen, s8Gain);
            goto FAIL;
            break;
    }

    return AIO_OK;
FAIL:
    return AIO_NG;
}

int HalAudSineGenGetEnable(SINE_GEN_e enSineGen, BOOL *pbEn)
{
    u16 nValue    = 0;
    u16 nDMicMask = AUDIO_BANK3_REG_VREC_CTRL03_SIN_EN_MASK;
    u16 nADCMask  = AUDIO_BANK4_REG_EXT_I2S_TEST_MD_REG_SINE_GEN_ENA_ADC_L_MASK
                   | AUDIO_BANK4_REG_EXT_I2S_TEST_MD_REG_SINE_GEN_ENA_ADC_R_MASK;
    u16 nI2SRXMask = AUDIO_BANK2_REG_I2S_TDM_CFG_03_REG_SINE_GEN_ENA_CH67_MASK
                     | AUDIO_BANK2_REG_I2S_TDM_CFG_03_REG_SINE_GEN_ENA_CH45_MASK
                     | AUDIO_BANK2_REG_I2S_TDM_CFG_03_REG_SINE_GEN_ENA_CH23_MASK
                     | AUDIO_BANK2_REG_I2S_TDM_CFG_03_REG_SINE_GEN_ENA_CH01_MASK;
    switch (enSineGen)
    {
        case E_MHAL_SINEGEN_AO_DMA_A:

            nValue = HalBachReadReg(E_BACH_REG_BANK1, AUDIO_BANK1_REG_DMA_TEST_CTRL5_ADDR);

            if (((nValue & AUDIO_BANK1_REG_DMA_TEST_CTRL5_REG_SINE_GEN_EN_DMA1_MASK)
                 == AUDIO_BANK1_REG_DMA_TEST_CTRL5_REG_SINE_GEN_EN_DMA1_MASK)
                && ((nValue
                     & (AUDIO_BANK1_REG_DMA_TEST_CTRL5_REG_SINE_GEN_L_DMA1_MASK
                        | AUDIO_BANK1_REG_DMA_TEST_CTRL5_REG_SINE_GEN_R_DMA1_MASK))
                    == (AUDIO_BANK1_REG_DMA_TEST_CTRL5_REG_SINE_GEN_L_DMA1_MASK
                        | AUDIO_BANK1_REG_DMA_TEST_CTRL5_REG_SINE_GEN_R_DMA1_MASK))
                && ((nValue & AUDIO_BANK1_REG_DMA_TEST_CTRL5_REG_SINE_GEN_RD_WR_DMA1_MASK) == 0))
            {
                *pbEn = TRUE;
            }
            else
            {
                *pbEn = FALSE;
            }

            break;

        case E_MHAL_SINEGEN_AI_DMA_A:

            nValue = HalBachReadReg(E_BACH_REG_BANK1, AUDIO_BANK1_REG_DMA_TEST_CTRL5_ADDR);

            if (((nValue & AUDIO_BANK1_REG_DMA_TEST_CTRL5_REG_SINE_GEN_EN_DMA1_MASK)
                 == AUDIO_BANK1_REG_DMA_TEST_CTRL5_REG_SINE_GEN_EN_DMA1_MASK)
                && ((nValue
                     & (AUDIO_BANK1_REG_DMA_TEST_CTRL5_REG_SINE_GEN_L_DMA1_MASK
                        | AUDIO_BANK1_REG_DMA_TEST_CTRL5_REG_SINE_GEN_R_DMA1_MASK))
                    == (AUDIO_BANK1_REG_DMA_TEST_CTRL5_REG_SINE_GEN_L_DMA1_MASK
                        | AUDIO_BANK1_REG_DMA_TEST_CTRL5_REG_SINE_GEN_R_DMA1_MASK))
                && ((nValue & AUDIO_BANK1_REG_DMA_TEST_CTRL5_REG_SINE_GEN_RD_WR_DMA1_MASK)
                    == AUDIO_BANK1_REG_DMA_TEST_CTRL5_REG_SINE_GEN_RD_WR_DMA1_MASK))
            {
                *pbEn = TRUE;
            }
            else
            {
                *pbEn = FALSE;
            }

            break;

        case E_MHAL_SINEGEN_AI_IF_DMIC_A:
            nValue = HalBachReadReg(E_BACH_REG_BANK3, AUDIO_BANK3_REG_VREC_CTRL03_ADDR);
            if ((nValue & nDMicMask) != 0)
            {
                *pbEn = TRUE;
            }
            else
            {
                *pbEn = FALSE;
            }
            break;
        case E_MHAL_SINEGEN_AI_IF_AMIC_A:
            nValue = HalBachReadReg(E_BACH_REG_BANK4, AUDIO_BANK4_REG_EXT_I2S_TEST_MD_ADDR);
            if ((nValue & nADCMask) != 0)
            {
                *pbEn = TRUE;
            }
            else
            {
                *pbEn = FALSE;
            }
            break;
        case E_MHAL_SINEGEN_AI_IF_I2S_A:
            nI2SRXMask = AUDIO_BANK2_REG_I2S_TDM_CFG_03_REG_SINE_GEN_ENA_CH67_MASK
                         | AUDIO_BANK2_REG_I2S_TDM_CFG_03_REG_SINE_GEN_ENA_CH45_MASK
                         | AUDIO_BANK2_REG_I2S_TDM_CFG_03_REG_SINE_GEN_ENA_CH23_MASK
                         | AUDIO_BANK2_REG_I2S_TDM_CFG_03_REG_SINE_GEN_ENA_CH01_MASK;
            nValue = HalBachReadReg(E_BACH_REG_BANK2, AUDIO_BANK2_REG_I2S_TDM_CFG_03_ADDR);
            if ((nValue & nI2SRXMask) != 0)
            {
                *pbEn = TRUE;
            }
            else
            {
                *pbEn = FALSE;
            }
            break;

        default:
            break;
    }

    return AIO_OK;
}

int HalAudSineGenGetSetting(SINE_GEN_e enSineGen, U8 *pu8Freq, S8 *ps8Gain)
{
    u16 nValue = 0;

    switch (enSineGen)
    {
        case E_MHAL_SINEGEN_AO_DMA_A:

            nValue = HalBachReadReg(E_BACH_REG_BANK1, AUDIO_BANK1_REG_DMA_TEST_CTRL5_ADDR);

            *ps8Gain = (nValue & AUDIO_BANK1_REG_DMA_TEST_CTRL5_REG_SINE_GEN_GAIN_DMA1_MASK)
                       >> AUDIO_BANK1_REG_DMA_TEST_CTRL5_REG_SINE_GEN_GAIN_DMA1_SHIFT;
            *pu8Freq = (nValue & AUDIO_BANK1_REG_DMA_TEST_CTRL5_REG_SINE_GEN_FREQ_DMA1_MASK)
                       >> AUDIO_BANK1_REG_DMA_TEST_CTRL5_REG_SINE_GEN_FREQ_DMA1_SHIFT;

            break;

        case E_MHAL_SINEGEN_AI_DMA_A:

            nValue = HalBachReadReg(E_BACH_REG_BANK1, AUDIO_BANK1_REG_DMA_TEST_CTRL5_ADDR);

            *ps8Gain = (nValue & AUDIO_BANK1_REG_DMA_TEST_CTRL5_REG_SINE_GEN_GAIN_DMA1_MASK)
                       >> AUDIO_BANK1_REG_DMA_TEST_CTRL5_REG_SINE_GEN_GAIN_DMA1_SHIFT;
            *pu8Freq = (nValue & AUDIO_BANK1_REG_DMA_TEST_CTRL5_REG_SINE_GEN_FREQ_DMA1_MASK)
                       >> AUDIO_BANK1_REG_DMA_TEST_CTRL5_REG_SINE_GEN_FREQ_DMA1_SHIFT;

            break;

        case E_MHAL_SINEGEN_AI_IF_DMIC_A:

            nValue = HalBachReadReg(E_BACH_REG_BANK3, AUDIO_BANK3_REG_VREC_CTRL03_ADDR);

            *ps8Gain =
                (nValue & AUDIO_BANK3_REG_VREC_CTRL03_SIN_GAIN_MASK) >> AUDIO_BANK3_REG_VREC_CTRL03_SIN_GAIN_SHIFT;
            *pu8Freq = (nValue & AUDIO_BANK3_REG_VREC_CTRL03_SIN_FREQ_SEL_MASK)
                       >> AUDIO_BANK3_REG_VREC_CTRL03_SIN_FREQ_SEL_SHIFT;

            break;

        case E_MHAL_SINEGEN_AI_IF_AMIC_A:

            nValue = HalBachReadReg(E_BACH_REG_BANK4, AUDIO_BANK4_REG_EXT_I2S_TEST_MD_ADDR);

            *ps8Gain = (nValue & AUDIO_BANK4_REG_EXT_I2S_TEST_MD_REG_SINE_GEN_GAIN_ADC_MASK)
                       >> AUDIO_BANK4_REG_EXT_I2S_TEST_MD_REG_SINE_GEN_GAIN_ADC_SHIFT;
            *pu8Freq = (nValue & AUDIO_BANK4_REG_EXT_I2S_TEST_MD_REG_SINE_GEN_FREQ_ADC_MASK)
                       >> AUDIO_BANK4_REG_EXT_I2S_TEST_MD_REG_SINE_GEN_FREQ_ADC_SHIFT;

            break;

        case E_MHAL_SINEGEN_AI_IF_I2S_A:

            nValue = HalBachReadReg(E_BACH_REG_BANK2, AUDIO_BANK2_REG_I2S_TDM_CFG_03_ADDR);

            *ps8Gain = (nValue & AUDIO_BANK2_REG_I2S_TDM_CFG_03_REG_SINE_GEN_GAIN_MASK)
                       >> AUDIO_BANK2_REG_I2S_TDM_CFG_03_REG_SINE_GEN_GAIN_SHIFT;
            *pu8Freq = (nValue & AUDIO_BANK2_REG_I2S_TDM_CFG_03_REG_SINE_GEN_FREQ_MASK)
                       >> AUDIO_BANK2_REG_I2S_TDM_CFG_03_REG_SINE_GEN_FREQ_SHIFT;

            break;

        default:
            break;
    }

    return AIO_OK;
}
