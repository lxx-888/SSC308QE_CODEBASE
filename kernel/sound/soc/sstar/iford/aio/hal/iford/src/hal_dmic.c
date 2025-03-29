/*
 * hal_dmic.c - Sigmastar
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

int HalAudDmicEnable(CHIP_AI_DMIC_e eDmic, BOOL bEn, AudMchSel_e eSel)
{
    U16 nMask;

    switch (eSel)
    {
        case E_AUD_MCH_SEL_DMIC01:
            nMask = AUDIO_BANK3_REG_DMIC_CTRL0_REG_DIG_MIC_PD_CH1_MASK;
            break;
        default:
            goto FAIL;
            break;
    }

    switch (eDmic)
    {
        case E_CHIP_AI_DMIC_A:
            if (bEn)
            {
                HalBachWriteReg(E_BACH_REG_BANK3, AUDIO_BANK3_REG_VREC_CTRL01_ADDR,
                                AUDIO_BANK3_REG_VREC_CTRL01_REG_DMA_PAUSE_MASK, 0);
                HalBachWriteReg(E_BACH_REG_BANK3, AUDIO_BANK3_REG_DMIC_CTRL0_ADDR, nMask, 0);
            }
            else
            {
                HalBachWriteReg(E_BACH_REG_BANK3, AUDIO_BANK3_REG_VREC_CTRL01_ADDR,
                                AUDIO_BANK3_REG_VREC_CTRL01_REG_DMA_PAUSE_MASK,
                                AUDIO_BANK3_REG_VREC_CTRL01_REG_DMA_PAUSE_MASK);
                HalBachWriteReg(E_BACH_REG_BANK3, AUDIO_BANK3_REG_DMIC_CTRL0_ADDR, nMask, nMask);
            }
            break;

        default:
            goto FAIL;
            break;
    }
    DBGMSG(AUDIO_DBG_LEVEL_DMIC, "eDmic[%d] eSel[%d] bEn[%d]\n", eDmic, eSel, bEn);

    return AIO_OK;

FAIL:
    ERRMSG("eDmic[%d] eSel[%d] Fail !\n", eDmic, eSel);
    return AIO_NG;
}

int HalAudDmicSetChannel(CHIP_AI_DMIC_e eDmic, U16 nCh)
{
    U16 nConfigValue;

    if (nCh > MAX_DIGMIC_CHN)
    {
        goto FAIL;
    }

    if (nCh == 1)
    {
        nConfigValue = 0;
    }
    else if (nCh == 2)
    {
        nConfigValue = 1;
    }
    else
    {
        goto FAIL;
    }

    switch (eDmic)
    {
        case E_CHIP_AI_DMIC_A:
            HalBachWriteReg(E_BACH_REG_BANK3, AUDIO_BANK3_REG_VREC_CTRL01_ADDR,
                            AUDIO_BANK3_REG_VREC_CTRL01_REG_DIG_MIC_CH_MASK,
                            (nConfigValue << AUDIO_BANK3_REG_VREC_CTRL01_REG_DIG_MIC_CH_SHIFT));
            break;

        default:
            goto FAIL;
            break;
    }

    DBGMSG(AUDIO_DBG_LEVEL_DMIC, "eDmic[%d] nCh[%d]\n", eDmic, nCh);

    return AIO_OK;
FAIL:
    ERRMSG("eDmic[%d] nCh[%d] Fail!\n", eDmic, nCh);
    return AIO_NG;
}

int HalAudDmicSetRate(CHIP_AI_DMIC_e eDmic, u32 nSampleRate)
{
    AudRate_e eRate;
    int       rateCfg = 0;

    eRate = HalAudApiRateToEnum(nSampleRate);

    switch (eRate)
    {
        case E_AUD_RATE_8K:
            rateCfg = 0;
            break;
        case E_AUD_RATE_16K:
            rateCfg = 1;
            break;
        case E_AUD_RATE_32K:
            rateCfg = 2;
            break;
        case E_AUD_RATE_48K:
            rateCfg = 3;
            break;
        default:
            goto FAIL;
    }

    switch (eDmic)
    {
        case E_CHIP_AI_DMIC_A:
            // 0: dmic clk from internal clock gen
            // 1: dmic_clk from I2S_A clock (I2S_TDM_RX_BCK)
            if (HalAudApiGetDtsValue(DMIC_BCK_EXT_MODE) == 0) // 0: dmic clk from internal clock gen
            {
                // 0: internal clock gen
                HalBachWriteReg(E_BACH_REG_BANK3, AUDIO_BANK3_REG_VREC_CTRL07_ADDR,
                                AUDIO_BANK3_REG_VREC_CTRL07_REG_DIG_MIC_CLK_SEL_MASK, 0);
                // dmic_clk from DMIC_BCK
                HalBachWriteReg(E_BACH_REG_BANK3, AUDIO_BANK3_REG_DIG_MIC_CTRL0_ADDR,
                                AUDIO_BANK3_REG_DIG_MIC_CTRL0_REG_VREC_BCK_SEL_MASK, 0); // 150458[3:3]
                // auto mode
                HalBachWriteReg(E_BACH_REG_BANK3, AUDIO_BANK3_REG_VREC_CTRL05_ADDR,
                                AUDIO_BANK3_REG_VREC_CTRL05_REG_MODE_AUTO_MASK,
                                AUDIO_BANK3_REG_VREC_CTRL05_REG_MODE_AUTO_MASK);

                HalBachWriteReg(
                    E_BACH_REG_BANK3, AUDIO_BANK3_REG_VREC_CTRL04_ADDR, AUDIO_BANK3_REG_VREC_CTRL04_REG_MODE_ALL_MASK,
                    HalAudApiGetDtsValues(DMIC_BCK_MODE, rateCfg) << AUDIO_BANK3_REG_VREC_CTRL04_REG_MODE_ALL_SHIFT);

                if ((eRate == E_AUD_RATE_8K) || (eRate == E_AUD_RATE_16K))
                {
                    HalBachWriteReg(E_BACH_REG_BANK3, AUDIO_BANK3_REG_CIC_CTRL1_ADDR,
                                    AUDIO_BANK3_REG_CIC_CTRL1_REG_CIC_THREE_STAGE_1LR_MASK,
                                    AUDIO_BANK3_REG_CIC_CTRL1_REG_CIC_THREE_STAGE_1LR_MASK);
                }
                if ((eRate == E_AUD_RATE_32K) || (eRate == E_AUD_RATE_48K))
                {
                    HalBachWriteReg(E_BACH_REG_BANK3, AUDIO_BANK3_REG_CIC_CTRL1_ADDR,
                                    AUDIO_BANK3_REG_CIC_CTRL1_REG_CIC_THREE_STAGE_1LR_MASK, 0);
                }
            }
            else // 1: dmic_clk from external clock (I2S_TDM_RX_BCK)
            {
                DBGMSG(AUDIO_DBG_LEVEL_DMIC, "DMIC BCK is share I2S !\n");
                // 1: external clock gen
                HalBachWriteReg(E_BACH_REG_BANK3, AUDIO_BANK3_REG_VREC_CTRL07_ADDR,
                                AUDIO_BANK3_REG_VREC_CTRL07_REG_DIG_MIC_CLK_SEL_MASK, 1);
                // dmic bck de-glitch enable
                HalBachWriteReg(E_BACH_REG_BANK3, AUDIO_BANK3_REG_VREC_CTRL07_ADDR,
                                AUDIO_BANK3_REG_VREC_CTRL07_REG_DMIC_EXT_CLK_DEGLITCH_ENABLE_MASK,
                                AUDIO_BANK3_REG_VREC_CTRL07_REG_DMIC_EXT_CLK_DEGLITCH_ENABLE_MASK); //
                // de-glitch level
                HalBachWriteReg(E_BACH_REG_BANK3, AUDIO_BANK3_REG_VREC_CTRL07_ADDR,
                                AUDIO_BANK3_REG_VREC_CTRL07_REG_DMIC_EXT_CLK_DEGLITCH_NUM_MASK,
                                0x1 << AUDIO_BANK3_REG_VREC_CTRL07_REG_DMIC_EXT_CLK_DEGLITCH_NUM_SHIFT);
                // 0x8: 2chn 16bpp,Down sampling is 32;
                if (HalAudI2sGetChannel(E_CHIP_AIO_I2S_RX_A) == E_AUD_CHANNEL_NUM_2)
                {
                    HalBachWriteReg(E_BACH_REG_BANK3, AUDIO_BANK3_REG_VREC_CTRL05_ADDR,
                                    AUDIO_BANK3_REG_VREC_CTRL05_REG_MODE_CIC_MASK,
                                    0x8 << AUDIO_BANK3_REG_VREC_CTRL05_REG_MODE_CIC_SHIFT); // 15040A[7:4]
                }
                // 0x9: 4chn 16bpp,Down sampling is 64;
                else if (HalAudI2sGetChannel(E_CHIP_AIO_I2S_RX_A) == E_AUD_CHANNEL_NUM_4)
                {
                    HalBachWriteReg(E_BACH_REG_BANK3, AUDIO_BANK3_REG_VREC_CTRL05_ADDR,
                                    AUDIO_BANK3_REG_VREC_CTRL05_REG_MODE_CIC_MASK,
                                    0x9 << AUDIO_BANK3_REG_VREC_CTRL05_REG_MODE_CIC_SHIFT); // 15040A[7:4]
                }
                // 0x0B: 8chn 16bit,Down sampling is 128;
                else if (HalAudI2sGetChannel(E_CHIP_AIO_I2S_RX_A) == E_AUD_CHANNEL_NUM_8)
                {
                    HalBachWriteReg(E_BACH_REG_BANK3, AUDIO_BANK3_REG_VREC_CTRL05_ADDR,
                                    AUDIO_BANK3_REG_VREC_CTRL05_REG_MODE_CIC_MASK,
                                    0xB << AUDIO_BANK3_REG_VREC_CTRL05_REG_MODE_CIC_SHIFT); // 15040A[7:4]
                }
                else
                {
                    /* do nothing */
                }
                // dmic_clk from I2S_TDM_RX_BCK
                HalBachWriteReg(E_BACH_REG_BANK3, AUDIO_BANK3_REG_DIG_MIC_CTRL0_ADDR,
                                AUDIO_BANK3_REG_DIG_MIC_CTRL0_REG_VREC_BCK_SEL_MASK,
                                AUDIO_BANK3_REG_DIG_MIC_CTRL0_REG_VREC_BCK_SEL_MASK); // 150458[3:3]
                // disable auto mode
                HalBachWriteReg(E_BACH_REG_BANK3, AUDIO_BANK3_REG_VREC_CTRL05_ADDR,
                                AUDIO_BANK3_REG_VREC_CTRL05_REG_MODE_AUTO_MASK, 0);
                // disable DEC FILTER
                HalBachWriteReg(E_BACH_REG_BANK3, AUDIO_BANK3_REG_CIC_CTRL1_ADDR,
                                AUDIO_BANK3_REG_CIC_CTRL1_REG_CIC_THREE_STAGE_1LR_MASK, 0);
                // tdm rx bck de-glitch enable
                HalBachWriteReg(E_BACH_REG_BANK2, AUDIO_BANK2_REG_I2S_TDM_CFG_00_ADDR,
                                AUDIO_BANK2_REG_I2S_TDM_CFG_00_REG_CODEC_I2S_RX_BCK_DG_EN_MASK,
                                AUDIO_BANK2_REG_I2S_TDM_CFG_00_REG_CODEC_I2S_RX_BCK_DG_EN_MASK);
                // de-glitch level
                HalBachWriteReg(E_BACH_REG_BANK2, AUDIO_BANK2_REG_I2S_TDM_CFG_00_ADDR,
                                AUDIO_BANK2_REG_I2S_TDM_CFG_00_REG_CODEC_I2S_RX_BCK_DG_NUM_MASK,
                                0x1 << AUDIO_BANK2_REG_I2S_TDM_CFG_00_REG_CODEC_I2S_RX_BCK_DG_NUM_SHIFT);
            }

            break;

        default:
            goto FAIL;
            break;
    }
    DBGMSG(AUDIO_DBG_LEVEL_DMIC, "eDmic[%d] nSampleRate[%d]\n", eDmic, nSampleRate);

    return AIO_OK;
FAIL:
    ERRMSG("eDmic[%d] nSampleRate[%d] Fail !\n", eDmic, nSampleRate);
    return AIO_NG;
}

int HalAudDmicInit(void)
{
    // main enable bit
    HalBachWriteReg(E_BACH_REG_BANK3, AUDIO_BANK3_REG_VREC_CTRL01_ADDR, AUDIO_BANK3_REG_VREC_CTRL01_REG_DIG_MIC_EN_MASK,
                    AUDIO_BANK3_REG_VREC_CTRL01_REG_DIG_MIC_EN_MASK);

    // initial working RAM
    HalBachWriteReg(E_BACH_REG_BANK3, AUDIO_BANK3_REG_VREC_CTRL06_ADDR,
                    AUDIO_BANK3_REG_VREC_CTRL06_REG_INIT_SRAM_EN_VREC_MASK,
                    AUDIO_BANK3_REG_VREC_CTRL06_REG_INIT_SRAM_EN_VREC_MASK);
    _UDelay(50);
    HalBachWriteReg(E_BACH_REG_BANK3, AUDIO_BANK3_REG_VREC_CTRL06_ADDR,
                    AUDIO_BANK3_REG_VREC_CTRL06_REG_INIT_SRAM_EN_VREC_MASK, 0);

    // enable decimation filter
    HalBachWriteReg(E_BACH_REG_BANK3, AUDIO_BANK3_REG_VREC_CTRL06_ADDR, AUDIO_BANK3_REG_VREC_CTRL06_REG_TG_EN_MASK,
                    AUDIO_BANK3_REG_VREC_CTRL06_REG_TG_EN_MASK);
    HalBachWriteReg(E_BACH_REG_BANK3, AUDIO_BANK3_REG_VREC_CTRL06_ADDR,
                    AUDIO_BANK3_REG_VREC_CTRL06_REG_TIME_GEN_EN_VREC_MASK,
                    AUDIO_BANK3_REG_VREC_CTRL06_REG_TIME_GEN_EN_VREC_MASK);
    HalBachWriteReg(E_BACH_REG_BANK3, AUDIO_BANK3_REG_VREC_CTRL06_ADDR,
                    AUDIO_BANK3_REG_VREC_CTRL06_REG_EN_DEC_1_VREC_MASK,
                    AUDIO_BANK3_REG_VREC_CTRL06_REG_EN_DEC_1_VREC_MASK);

    // enable CIC
    HalBachWriteReg(E_BACH_REG_BANK3, AUDIO_BANK3_REG_CIC_CTRL0_ADDR, AUDIO_BANK3_REG_CIC_CTRL0_REG_CIC_EN_MASK,
                    AUDIO_BANK3_REG_CIC_CTRL0_REG_CIC_EN_MASK);

    // not active reset cic filter
    HalBachWriteReg(E_BACH_REG_BANK3, AUDIO_BANK3_REG_CIC_CTRL0_ADDR, AUDIO_BANK3_REG_CIC_CTRL0_REG_CIC_PD_1R_MASK, 0);
    HalBachWriteReg(E_BACH_REG_BANK3, AUDIO_BANK3_REG_CIC_CTRL0_ADDR, AUDIO_BANK3_REG_CIC_CTRL0_REG_CIC_PD_1L_MASK, 0);

    // dtsi enable HPF
    HalAudSetHpf(E_AUD_HPF_DMIC, HalAudApiGetDtsValues(HPF_DMIC_LEVEL, 0) /* enable */,
                 HalAudApiGetDtsValues(HPF_DMIC_LEVEL, 1) /* value */);

    return AIO_OK;
}
