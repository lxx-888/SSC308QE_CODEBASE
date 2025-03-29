/*
 * hal_atop.c - Sigmastar
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

static BOOL g_bAtopStatus[E_CHIP_AIO_ATOP_TOTAL];

static const char *_HalAudAtopEnumToString(CHIP_AIO_ATOP_e eAtop)
{
    switch (eAtop)
    {
        GENERATE_CASE(E_CHIP_AIO_ATOP_DAC_AB, DAC_AB)
        GENERATE_CASE(E_CHIP_AIO_ATOP_ADC_AB, ADC_AB)
        default:
        {
            return "Unknown eAtop";
        }
    }
}

static const char *_HalAudAtopAoEnumToString(CHIP_AO_DAC_e eDac)
{
    switch (eDac)
    {
        GENERATE_CASE(E_CHIP_AO_DAC_A, DAC_A)
        default:
        {
            return "Unknown DAC";
        }
    }
}

static const char *_HalAudAtopAiEnumToString(CHIP_AI_ADC_e eDac)
{
    switch (eDac)
    {
        GENERATE_CASE(E_CHIP_AI_ADC_A, ADC_A)
        GENERATE_CASE(E_CHIP_AI_ADC_B, ADC_B)
        default:
        {
            return "Unknown ADC";
        }
    }
}

static int _HalAudAtopEnableRef(BOOL bEnable)
{
    U16 nMask;

    nMask = AUDIO_AUSDM_REG_AUSDM_03_REG_PD_IGEN_MASK;                                              // 11
    HalBachWriteReg(E_BACH_REG_BANK5, AUDIO_AUSDM_REG_AUSDM_03_ADDR, nMask, (bEnable ? 0 : nMask)); // 103406[11]

    DBGMSG(AUDIO_DBG_LEVEL_ATOP, "bEnable[%d]\n", bEnable);

    return AIO_OK;
}

static int _HalAudAtopDac(CHIP_AO_DAC_e eDac, BOOL bEnable)
{
    U16 nMask;

    switch (eDac)
    {
        case E_CHIP_AO_DAC_A:
            nMask = (AUDIO_AUSDM_REG_AUSDM_03_REG_PD_BIAS_DAC_MASK | AUDIO_AUSDM_REG_AUSDM_03_REG_PD_L0_DAC_MASK
                     | AUDIO_AUSDM_REG_AUSDM_03_REG_PD_R0_DAC_MASK
                     | AUDIO_AUSDM_REG_AUSDM_03_REG_PD_REF_DAC_MASK); // 1 4 9 10

            if (bEnable)
            {
                HalBachWriteReg(E_BACH_REG_BANK5, AUDIO_AUSDM_REG_AUSDM_03_ADDR, nMask, 0); //
                // HalBachWriteReg(E_BACH_REG_BANK5, AUDIO_AUSDM_REG_AUSDM_00_ADDR,
                // AUDIO_AUSDM_REG_AUSDM_00_REG_EN_CK_DAC_MASK, 0x1 >> AUDIO_AUSDM_REG_AUSDM_00_REG_EN_CK_DAC_SHIFT);
            }
            else
            {
                HalBachWriteReg(E_BACH_REG_BANK5, AUDIO_AUSDM_REG_AUSDM_03_ADDR, nMask, nMask);
                // HalBachWriteReg(E_BACH_REG_BANK5, AUDIO_AUSDM_REG_AUSDM_00_ADDR,
                // AUDIO_AUSDM_REG_AUSDM_00_REG_EN_CK_DAC_MASK, 0x0 >> AUDIO_AUSDM_REG_AUSDM_00_REG_EN_CK_DAC_SHIFT);
            }
            break;

        default:
            goto FAIL;
            break;
    }
    DBGMSG(AUDIO_DBG_LEVEL_ATOP, "eDac[%s] bEnable[%d]", _HalAudAtopAoEnumToString(eDac), bEnable);

    return AIO_OK;
FAIL:
    ERRMSG("eDac[%s] bEnable[%d] Fail !", _HalAudAtopAoEnumToString(eDac), bEnable);
    return AIO_NG;
}

// todo later ADC 0,1 & 2,3
static int _HalAudAtopEnablePreamp(CHIP_AI_ADC_e eAdc, BOOL bEnable)
{
    U16 nMask;

    switch (eAdc)
    {
        case E_CHIP_AI_ADC_A:
        case E_CHIP_AI_ADC_B:
            nMask = AUDIO_AUSDM_REG_AUSDM_03_REG_PD_INT_L_MASK | AUDIO_AUSDM_REG_AUSDM_03_REG_PD_INT_R_MASK;
            HalBachWriteReg(E_BACH_REG_BANK5, AUDIO_AUSDM_REG_AUSDM_03_ADDR, nMask, bEnable ? 0 : nMask);
            break;
        default:
            goto FAIL;
            break;
    }

    DBGMSG(AUDIO_DBG_LEVEL_ATOP, "eADC[%s] bEnable[%d]", _HalAudAtopAiEnumToString(eAdc), bEnable);

    return AIO_OK;

FAIL:
    ERRMSG("eAdc[%d] Fail !", eAdc);
    return AIO_NG;
}

static int _HalAudAtopAdc(CHIP_AI_ADC_e eAdc, BOOL bEnable)
{
    U16 nMask;

    switch (eAdc)
    {
        case E_CHIP_AI_ADC_A:
        case E_CHIP_AI_ADC_B:
            nMask =
                (AUDIO_AUSDM_REG_AUSDM_03_REG_PD_INMUX_L_MASK | AUDIO_AUSDM_REG_AUSDM_03_REG_PD_INMUX_R_MASK
                 | AUDIO_AUSDM_REG_AUSDM_03_REG_PD_INT_L_MASK | AUDIO_AUSDM_REG_AUSDM_03_REG_PD_INT_R_MASK); // 2 3 7 8
            HalBachWriteReg(E_BACH_REG_BANK5, AUDIO_AUSDM_REG_AUSDM_03_ADDR, nMask, (bEnable ? 0 : nMask));

            nMask = (AUDIO_AUSDM_REG_AUSDM_31_REG_PD_SAR_L_MASK | AUDIO_AUSDM_REG_AUSDM_31_REG_PD_SAR_R_MASK);
            HalBachWriteReg(E_BACH_REG_BANK5, AUDIO_AUSDM_REG_AUSDM_31_ADDR, nMask, (bEnable ? 0 : nMask));
            //_MSleep(300);
            nMask =
                (AUDIO_AUSDM_REG_AUSDM_31_REG_EN_SAR_LOGIC_L_MASK | AUDIO_AUSDM_REG_AUSDM_31_REG_EN_SAR_LOGIC_R_MASK);
            HalBachWriteReg(E_BACH_REG_BANK5, AUDIO_AUSDM_REG_AUSDM_31_ADDR, nMask, (bEnable ? nMask : 0));
            break;

        default:
            goto FAIL;
            break;
    }
    DBGMSG(AUDIO_DBG_LEVEL_ATOP, "eADC[%s] bEnable[%d]", _HalAudAtopAiEnumToString(eAdc), bEnable);
    return AIO_OK;
FAIL:
    ERRMSG("eAdc[%d] bEnable[%d] Fail !", eAdc, bEnable);
    return AIO_NG;
}

static int _HalAudAtopSwitch(CHIP_AIO_ATOP_e eAtop, BOOL bEnable)
{
    switch (eAtop)
    {
        case E_CHIP_AIO_ATOP_ADC_AB:
            if (bEnable)
            {
                _HalAudAtopAdc(E_CHIP_AI_ADC_A, TRUE);

                _HalAudAtopEnablePreamp(E_CHIP_AI_ADC_A, TRUE);

                _MSleep(50); // delay for analog initialize completed
            }
            else
            {
                _HalAudAtopAdc(E_CHIP_AI_ADC_A, FALSE);
                _HalAudAtopEnablePreamp(E_CHIP_AI_ADC_A, FALSE);
            }
            break;

        case E_CHIP_AIO_ATOP_DAC_AB:
            if (bEnable)
            {
                _HalAudAtopDac(E_CHIP_AO_DAC_A, TRUE);

                if (!HalAudApiGetDtsValue(KEEP_DAC_POWER_ON))
                {
                    // enable gpio for line-out, should after atop enable
                    _MSleep(10);
                    HalAudApiAoAmpEnable(bEnable, 0); // chn0
                    HalAudApiAoAmpEnable(bEnable, 1); // chn1
                }
            }
            else
            {
                if (!HalAudApiGetDtsValue(KEEP_DAC_POWER_ON))
                {
                    // disable gpio for line-out, should before atop disable
                    HalAudApiAoAmpEnable(bEnable, 0); // chn0
                    HalAudApiAoAmpEnable(bEnable, 1); // chn1
                    _MSleep(10);
                    _HalAudAtopDac(E_CHIP_AO_DAC_A, FALSE);
                }
            }
            break;
        default:
            goto FAIL;
            break;
    }

    DBGMSG(AUDIO_DBG_LEVEL_ATOP, "eAtop[%s] bEnable[%d]", _HalAudAtopEnumToString(eAtop), bEnable);

    return AIO_OK;

FAIL:
    ERRMSG("eAtop[%d] bEnable[%d] Fail !", eAtop, bEnable);
    return AIO_NG;
}

int HalAudAtopEnable(CHIP_AIO_ATOP_e eAtop, BOOL bEnable)
{
    int  ret           = AIO_OK;
    int  i             = 0;
    BOOL bIsAllAtopOff = TRUE;

    if (!CHIP_AIO_ATOP_IDX_VALID(eAtop))
    {
        ERRMSG("eAtop = %d\n", eAtop);
        goto FAIL;
    }

    if (bEnable)
    {
        if (g_bAtopStatus[eAtop] == FALSE)
        {
            if (HalAudApiGetDtsValue(KEEP_ADC_POWER_ON) == 1 && eAtop <= E_CHIP_AIO_ATOP_ADC_AB)
            {
                _HalAudAtopSwitch(eAtop, TRUE);
            }
            else if (HalAudApiGetDtsValue(KEEP_DAC_POWER_ON) == 1 && eAtop >= E_CHIP_AIO_ATOP_DAC_AB)
            {
                HalAudAtopAoSetMute(E_AO_CH_DAC_A_0, FALSE);
                HalAudAtopAoSetMute(E_AO_CH_DAC_B_0, FALSE);
            }
            else
            {
                for (i = 0; i < E_CHIP_AIO_ATOP_TOTAL; i++)
                {
                    if (g_bAtopStatus[i] == TRUE)
                    {
                        bIsAllAtopOff = FALSE;
                        break;
                    }
                }

                if (!(HalAudApiGetDtsValue(KEEP_ADC_POWER_ON) || HalAudApiGetDtsValue(KEEP_DAC_POWER_ON))
                    && bIsAllAtopOff == TRUE)
                {
                    ret |= _HalAudAtopEnableRef(TRUE);
                    if (ret != AIO_OK)
                    {
                        ERRMSG("AtopEnableRef Fail !");
                        goto FAIL;
                    }
                }

                ret |= _HalAudAtopSwitch(eAtop, TRUE);
                if (ret != AIO_OK)
                {
                    ERRMSG("AtopSwitch Fail !");
                    goto FAIL;
                }
            }
            g_bAtopStatus[eAtop] = TRUE;
        }
    }
    else
    {
        if (g_bAtopStatus[eAtop] == TRUE)
        {
            if (HalAudApiGetDtsValue(KEEP_ADC_POWER_ON) == 1 && eAtop <= E_CHIP_AIO_ATOP_ADC_AB)
            {
                // do nothing
            }
            else if (HalAudApiGetDtsValue(KEEP_DAC_POWER_ON) == 1 && eAtop >= E_CHIP_AIO_ATOP_DAC_AB)
            {
                HalAudAtopAoSetMute(E_AO_CH_DAC_A_0, TRUE);
                HalAudAtopAoSetMute(E_AO_CH_DAC_B_0, TRUE);
            }
            else
            {
                for (i = 0; i < E_CHIP_AIO_ATOP_TOTAL; i++)
                {
                    // If there is another item use atop verf, we can't close the verf.
                    if (g_bAtopStatus[i] == TRUE && i != eAtop)
                    {
                        bIsAllAtopOff = FALSE;
                        break;
                    }
                }

                ret |= _HalAudAtopSwitch(eAtop, FALSE);
                if (ret != AIO_OK)
                {
                    ERRMSG("AtopSwitch Fail !");
                    goto FAIL;
                }

                if (!(HalAudApiGetDtsValue(KEEP_ADC_POWER_ON) || HalAudApiGetDtsValue(KEEP_DAC_POWER_ON))
                    && bIsAllAtopOff == TRUE)
                {
                    ret |= _HalAudAtopEnableRef(FALSE);
                    if (ret != AIO_OK)
                    {
                        ERRMSG("AtopEnableRef Fail !");
                        goto FAIL;
                    }
                }
            }
            g_bAtopStatus[eAtop] = FALSE;
        }
    }

    return AIO_OK;

FAIL:

    return AIO_NG;
}

static int _HalAudAtopSetAdcGain(CHIP_AI_ADC_e eAdc, U16 nSel)
{
    U16 nRegMsk, nPos;
    U8  nOffset;

    switch (eAdc)
    {
        case E_CHIP_AI_ADC_A:
            nOffset = AUDIO_AUSDM_REG_AUSDM_08_ADDR;
            nRegMsk = AUDIO_AUSDM_REG_AUSDM_08_REG_INT_GAIN_L_MASK;
            nPos    = AUDIO_AUSDM_REG_AUSDM_08_REG_INT_GAIN_L_SHIFT;
            break;

        case E_CHIP_AI_ADC_B:
            nOffset = AUDIO_AUSDM_REG_AUSDM_08_ADDR;
            nRegMsk = AUDIO_AUSDM_REG_AUSDM_08_REG_INT_GAIN_R_MASK;
            nPos    = AUDIO_AUSDM_REG_AUSDM_08_REG_INT_GAIN_R_SHIFT;
            break;

        default:
            ERRMSG("eAdc[%d] Fail !", eAdc);
            goto FAIL;
            break;
    }

    if (nSel > 0x3)
    {
        goto FAIL;
    }

    DBGMSG(AUDIO_DBG_LEVEL_ATOP, "eAdc[%s] nSel[%d]", _HalAudAtopAiEnumToString(eAdc), nSel);

    HalBachWriteReg(E_BACH_REG_BANK5, nOffset, nRegMsk, (nSel << nPos));

    return AIO_OK;

FAIL:
    ERRMSG("eAdc[%s] nSel[%d] Fail !", _HalAudAtopAiEnumToString(eAdc), nSel);
    return AIO_NG;
}

int HalAudAtopSetAdcMux(CHIP_AI_ADC_e eAdc, CHIP_ADC_MUX_e eAdcMux)
{
    switch (eAdc)
    {
        case E_CHIP_AI_ADC_A:
            if (eAdcMux == E_CHIP_ADC_MUX_LINEIN)
            {
                HalBachWriteReg(E_BACH_REG_BANK5, AUDIO_AUSDM_REG_AUSDM_05_ADDR,
                                AUDIO_AUSDM_REG_AUSDM_05_REG_CH_INMUX_L_MASK,
                                0x0 << AUDIO_AUSDM_REG_AUSDM_05_REG_CH_INMUX_L_SHIFT);
            }
            else if (eAdcMux == E_CHIP_ADC_MUX_MICIN)
            {
                HalBachWriteReg(E_BACH_REG_BANK5, AUDIO_AUSDM_REG_AUSDM_05_ADDR,
                                AUDIO_AUSDM_REG_AUSDM_05_REG_CH_INMUX_L_MASK,
                                0x3 << AUDIO_AUSDM_REG_AUSDM_05_REG_CH_INMUX_L_SHIFT);
            }
            break;

        case E_CHIP_AI_ADC_B:
            if (eAdcMux == E_CHIP_ADC_MUX_LINEIN)
            {
                HalBachWriteReg(E_BACH_REG_BANK5, AUDIO_AUSDM_REG_AUSDM_05_ADDR,
                                AUDIO_AUSDM_REG_AUSDM_05_REG_CH_INMUX_R_MASK,
                                0x0 << AUDIO_AUSDM_REG_AUSDM_05_REG_CH_INMUX_R_SHIFT);
            }
            else if (eAdcMux == E_CHIP_ADC_MUX_MICIN)
            {
                HalBachWriteReg(E_BACH_REG_BANK5, AUDIO_AUSDM_REG_AUSDM_05_ADDR,
                                AUDIO_AUSDM_REG_AUSDM_05_REG_CH_INMUX_R_MASK,
                                0x3 << AUDIO_AUSDM_REG_AUSDM_05_REG_CH_INMUX_R_SHIFT);
            }
            break;
        default:
            goto FAIL;
            break;
    }
    DBGMSG(AUDIO_DBG_LEVEL_ATOP, "eAdc[%s] eAdcMux[%d]", _HalAudAtopAiEnumToString(eAdc), eAdcMux);

    return AIO_OK;
FAIL:
    ERRMSG("eAdc[%d] eAdcMux[%d] Fail !", eAdc, eAdcMux);
    return AIO_NG;
}

static int _HalAudAtopMicAmpGain(CHIP_AI_ADC_e eAdc, U16 nSel)
{
    U16 nRegMsk, nPos;
    U8  nOffset;

    switch (eAdc)
    {
        case E_CHIP_AI_ADC_A:
            nOffset = AUDIO_AUSDM_REG_AUSDM_06_ADDR;
            nRegMsk = AUDIO_AUSDM_REG_AUSDM_06_REG_GAIN_INMUX_L_MASK;
            nPos    = AUDIO_AUSDM_REG_AUSDM_06_REG_GAIN_INMUX_L_SHIFT;
            break;

        case E_CHIP_AI_ADC_B:
            nOffset = AUDIO_AUSDM_REG_AUSDM_06_ADDR;
            nRegMsk = AUDIO_AUSDM_REG_AUSDM_06_REG_GAIN_INMUX_R_MASK;
            nPos    = AUDIO_AUSDM_REG_AUSDM_06_REG_GAIN_INMUX_R_SHIFT;
            break;
        default:
            goto FAIL;
            break;
    }

    if (nSel > 0xD)
    {
        goto FAIL;
    }

    DBGMSG(AUDIO_DBG_LEVEL_ATOP, "eAdc[%s] nSel[%d]", _HalAudAtopAiEnumToString(eAdc), nSel);
    HalBachWriteReg(E_BACH_REG_BANK5, nOffset, nRegMsk, (nSel << nPos));

    return AIO_OK;
FAIL:
    ERRMSG("eAdc[%d] nSel[%d] Fail !", eAdc, nSel);
    return AIO_NG;
}

int HalAudAtopAiSetGain(AI_CH_e eAiCh, S16 s16Gain)
{
    int ret     = AIO_OK;
    int v       = 0;
    U16 adcGain = 0;
    U16 preGain = 0;
    int line    = 0;

    //
    if ((s16Gain >= CHIP_ADC_GAIN_STEP_TOTAL) || (s16Gain < 0))
    {
        line = __LINE__;
        goto FAIL;
    }

    //
    preGain = g_aMicInCombineGainTable[s16Gain][0];
    adcGain = g_aMicInCombineGainTable[s16Gain][1];

    //
    switch (eAiCh)
    {
        case E_AI_CH_ADC_A_0:
            v = CHIP_AI_ADC_IDX_BY_DEV(E_AI_DEV_ADC_A);
            break;
        case E_AI_CH_ADC_B_0:
            v = CHIP_AI_ADC_IDX_BY_DEV(E_AI_DEV_ADC_B);
            break;
        case E_AI_CH_ADC_C_0:
            v = CHIP_AI_ADC_IDX_BY_DEV(E_AI_DEV_ADC_C);
            break;
        case E_AI_CH_ADC_D_0:
            v = CHIP_AI_ADC_IDX_BY_DEV(E_AI_DEV_ADC_D);
            break;
        default:
            line = __LINE__;
            goto FAIL;
            break;
    }
    ret |= _HalAudAtopSetAdcGain((CHIP_AI_ADC_e)v, adcGain); // 0~3:0dB, 1:6dB, 2:12dB, 3:18dB
    if (ret != AIO_OK)
    {
        line = __LINE__;
        goto FAIL;
    }

    ret |= _HalAudAtopMicAmpGain((CHIP_AI_ADC_e)v, preGain); // 0~D : -6dB~39dB
    if (ret != AIO_OK)
    {
        line = __LINE__;
        goto FAIL;
    }
    DBGMSG(AUDIO_DBG_LEVEL_ATOP, "eAiCh[%d] Gain[%d]", eAiCh, s16Gain);

    return AIO_OK;

FAIL:
    ERRMSG("Line:%d eAiCh[%d] Gain[%d] Fail !", line, eAiCh, s16Gain);

    return AIO_NG;
}

int HalAudAtopAoSetMute(AO_CH_e eAoCh, BOOL bEnable)
{
    U16 pos, msk;

    switch (eAoCh)
    {
        case E_AO_CH_DAC_A_0:
            pos = AUDIO_BANK4_REG_MIX1_SEL_REG_DAC_DIN_L_SEL_SHIFT;
            msk = AUDIO_BANK4_REG_MIX1_SEL_REG_DAC_DIN_L_SEL_MASK;
            break;
        case E_AO_CH_DAC_B_0:
            pos = AUDIO_BANK4_REG_MIX1_SEL_REG_DAC_DIN_R_SEL_SHIFT;
            msk = AUDIO_BANK4_REG_MIX1_SEL_REG_DAC_DIN_R_SEL_MASK;
            break;

        default:
            ERRMSG("AudDacSel_e[%d] error !", (int)eAoCh);
            return AIO_NG;
    }
    DBGMSG(AUDIO_DBG_LEVEL_ATOP, "eAoCh[%d] bEnable[%d]", eAoCh, bEnable);
    HalBachWriteReg(E_BACH_REG_BANK4, AUDIO_BANK4_REG_MIX1_SEL_ADDR, msk, !bEnable << pos);
    return AIO_OK;
}

int HalAudAtopAiSetSampleRate(AI_IF_e eAiIf, u32 nSampleRate)
{
    AudRate_e eRate   = 0;
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
            ERRMSG("eAiIf %d nSampleRate %d Fail !\n", eAiIf, nSampleRate);
            goto FAIL;
    }

    switch (eAiIf)
    {
        case E_MHAL_AI_IF_ADC_A_0_B_0:
            HalBachWriteReg(E_BACH_REG_BANK4, AUDIO_BANK4_REG_SR0_SEL_ADDR, AUDIO_BANK4_REG_SR0_SEL_REG_CODEC_SEL_MASK,
                            rateCfg << AUDIO_BANK4_REG_SR0_SEL_REG_CODEC_SEL_SHIFT);
            HalBachWriteReg(E_BACH_REG_BANK4, AUDIO_BANK4_REG_SR0_SEL_ADDR, AUDIO_BANK4_REG_SR0_SEL_REG_CIC_3_SEL_MASK,
                            rateCfg << AUDIO_BANK4_REG_SR0_SEL_REG_CIC_3_SEL_SHIFT);
            break;

        default:
            break;
    }

    return AIO_OK;
FAIL:
    return AIO_NG;
}

int HalAudAtopInit(void)
{
    S32 i;

    HalBachWriteRegByte(0x00103404, 0xff, 0x80);
    HalBachWriteRegByte(0x00103404, 0xff, 0x00);
    HalBachWriteRegByte(0x00103400, 0xff, 0x10);
    HalBachWriteRegByte(0x00103401, 0xff, 0x98);
    HalBachWriteRegByte(0x0010340c, 0xff, 0x66);
    HalBachWriteRegByte(0x0010340d, 0xff, 0x00);
    HalBachWriteRegByte(0x0010340a, 0xff, 0x00);
    HalBachWriteRegByte(0x0010340b, 0xff, 0x00);
    HalBachWriteRegByte(0x00103406, 0xff, 0x41);
    HalBachWriteRegByte(0x00103407, 0xff, 0xe0);
    HalBachWriteRegByte(0x00103460, 0xff, 0x03);
    HalBachWriteRegByte(0x00103461, 0xff, 0x00);
    HalBachWriteRegByte(0x00103434, 0xff, 0x04);
    HalBachWriteRegByte(0x00103435, 0xff, 0x00);
    HalBachWriteRegByte(0x00103430, 0xff, 0x00);
    HalBachWriteRegByte(0x00103431, 0xff, 0x00);
    HalBachWriteRegByte(0x00103406, 0xff, 0xbe);
    HalBachWriteRegByte(0x00103407, 0xff, 0x27);
    HalBachWriteRegByte(0x00103412, 0xff, 0x00);
    HalBachWriteRegByte(0x00103413, 0xff, 0x04);
    HalBachWriteRegByte(0x00103406, 0xff, 0xbe);
    HalBachWriteRegByte(0x00103407, 0xff, 0x27);
    HalBachWriteRegByte(0x00103406, 0xff, 0x9e);
    HalBachWriteRegByte(0x00103407, 0xff, 0x27);
    HalBachWriteRegByte(0x00103404, 0xff, 0x80);
    HalBachWriteRegByte(0x00103405, 0xff, 0x00);
    HalBachWriteRegByte(0x00103400, 0xff, 0x10);
    // HalBachWriteRegByte(0x00103401, 0xff, 0x00);
    HalBachWriteRegByte(0x00103406, 0xff, 0x8e);
    HalBachWriteRegByte(0x00103407, 0xff, 0x27);
    HalBachWriteRegByte(0x00103406, 0xff, 0x8e);
    HalBachWriteRegByte(0x00103407, 0xff, 0x25);
    HalBachWriteRegByte(0x00103406, 0xff, 0x8c);
    HalBachWriteRegByte(0x00103407, 0xff, 0x25);
    HalBachWriteRegByte(0x00103406, 0xff, 0x8c);
    HalBachWriteRegByte(0x00103407, 0xff, 0x21);

    HalBachWriteRegByte(0x00103406, 0xff, 0x88);
    HalBachWriteRegByte(0x00103407, 0xff, 0x21);
    HalBachWriteRegByte(0x00103406, 0xff, 0x80);
    HalBachWriteRegByte(0x00103407, 0xff, 0x21);

    HalBachWriteRegByte(0x0010340c, 0xff, 0x00);
    HalBachWriteRegByte(0x0010340d, 0xff, 0x00);
    HalBachWriteRegByte(0x0010340a, 0xff, 0x00);
    HalBachWriteRegByte(0x0010340b, 0xff, 0x00);

    HalBachWriteRegByte(0x00103406, 0xff, 0x00);
    HalBachWriteRegByte(0x00103407, 0xff, 0x21);
    HalBachWriteRegByte(0x00103406, 0xff, 0x00);
    HalBachWriteRegByte(0x00103407, 0xff, 0x20);

    HalBachWriteRegByte(0x00103460, 0xff, 0x08);
    HalBachWriteRegByte(0x00103461, 0xff, 0x00);
    HalBachWriteRegByte(0x00103460, 0xff, 0x00);
    HalBachWriteRegByte(0x00103461, 0xff, 0x00);
    HalBachWriteRegByte(0x00103460, 0xff, 0x01);
    HalBachWriteRegByte(0x00103461, 0xff, 0x00);
    HalBachWriteRegByte(0x00103460, 0xff, 0x03);
    HalBachWriteRegByte(0x00103461, 0xff, 0x00);
    HalBachWriteRegByte(0x00103425, 0xff, 0x40);
    HalBachWriteReg(E_BACH_REG_BANK5, AUDIO_AUSDM_REG_AUSDM_10_ADDR,
                    AUDIO_AUSDM_REG_AUSDM_10_REG_DIT_ADC_L_MASK | AUDIO_AUSDM_REG_AUSDM_10_REG_DIT_ADC_R_MASK,
                    0); // POP noise
    HalBachWriteReg(E_BACH_REG_BANK5, AUDIO_AUSDM_REG_AUSDM_10_ADDR, AUDIO_AUSDM_REG_AUSDM_10_REG_DLF_DEM_L_MASK,
                    0x1 << AUDIO_AUSDM_REG_AUSDM_10_REG_DLF_DEM_L_SHIFT);
    HalBachWriteReg(E_BACH_REG_BANK5, AUDIO_AUSDM_REG_AUSDM_10_ADDR, AUDIO_AUSDM_REG_AUSDM_10_REG_DLF_DEM_R_MASK,
                    0x1 << AUDIO_AUSDM_REG_AUSDM_10_REG_DLF_DEM_R_SHIFT);

    if (HalAudApiGetDtsValue(KEEP_ADC_POWER_ON) || HalAudApiGetDtsValue(KEEP_DAC_POWER_ON))
    {
        _HalAudAtopEnableRef(TRUE);
        if (!HalAudApiGetDtsValue(KEEP_ADC_POWER_ON))
        {
            for (i = 0; i < E_CHIP_AIO_ATOP_DAC_AB; i++)
            {
                _HalAudAtopSwitch(i, FALSE);
            }
        }
        if (HalAudApiGetDtsValue(KEEP_DAC_POWER_ON))
        {
            HalAudAtopAoSetMute(E_AO_CH_DAC_A_0, TRUE);
            HalAudAtopAoSetMute(E_AO_CH_DAC_B_0, TRUE);
            _HalAudAtopSwitch(E_CHIP_AIO_ATOP_DAC_AB, TRUE);
            //_MSleep(10);
            HalAudApiAoAmpEnable(TRUE, 0); // chn0
            HalAudApiAoAmpEnable(TRUE, 1); // chn1
        }
        else
        {
            _HalAudAtopSwitch(E_CHIP_AIO_ATOP_DAC_AB, FALSE);
        }
    }
    else
    {
        for (i = 0; i < E_CHIP_AIO_ATOP_TOTAL; i++)
        {
            _HalAudAtopSwitch(i, FALSE);
        }
    }

    return AIO_OK;
}

void HalAudAtopDeInit(void)
{
    memset(g_bAtopStatus, 0, sizeof(BOOL) * ARRAY_SIZE(g_bAtopStatus));
    _HalAudAtopEnableRef(FALSE);
}
