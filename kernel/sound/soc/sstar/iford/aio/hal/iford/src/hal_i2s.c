/*
 * hal_i2s.c - Sigmastar
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

#include <drv_padmux.h>
#include <drv_puse.h>
#include <drv_gpio.h>

static int g_BachPllVal = 0;

AudI2sCfg_t g_aI2sCfg[E_CHIP_AIO_I2S_TOTAL];
#if 0
#if 1
static int g_nBachPLL[4][7][3] = {
    {{8192000, 12288000, 196608000}, {8192000, 16384000, 196608000}, {8192000, 0, 196608000}},
    {{12288000, 12288000, 196608000},
     {12288000, 16384000, 196608000},
     {12288000, 18432000, 147456000},
     {12288000, 24576000, 196608000},
     {12288000, 0, 196608000}},
    {{24576000, 12288000, 196608000},
     {24576000, 16384000, 196608000},
     {24576000, 18432000, 147456000},
     {24576000, 24576000, 196608000},
     {24576000, 32768000, 196608000},
     {24576000, 49152000, 196608000},
     {24576000, 0, 196608000}},
    {{49152000, 12288000, 196608000},
     {49152000, 16384000, 196608000},
     {49152000, 18432000, 147456000},
     {49152000, 24576000, 196608000},
     {49152000, 32768000, 196608000},
     {49152000, 49152000, 196608000},
     {49152000, 0, 196608000}}};
#else
static int g_nBachPLL[24][3][3] = {
    {{256000, 12288000, 49152000}, {256000, 16384000, 32768000}, {256000, 19200000, 76800000}},
    {{352800, 11289600, 22579200}, {0, 0, 0}, {0, 0, 0}},
    {{384000, 12288000, 49152000}, {384000, 19200000, 76800000}, {0, 0, 0}},
    {{512000, 12288000, 49152000}, {512000, 16384000, 32768000}, {512000, 19200000, 76800000}},
    {{705600, 11289600, 22579200}, {0, 0, 0}, {0, 0, 0}},
    {{768000, 12288000, 49152000}, {768000, 19200000, 76800000}},
    {{1024000, 12288000, 49152000}, {1024000, 16384000, 32768000}, {1024000, 19200000, 76800000}},
    {{1411200, 11289600, 22579200}},
    {{1536000, 12288000, 49152000}, {1536000, 19200000, 76800000}},
    {{2048000, 12288000, 49152000}, {2048000, 16384000, 32768000}, {2048000, 19200000, 307200000}},
    {{2822400, 11289600, 22579200}},
    {{3072000, 12288000, 49152000}, {3072000, 19200000, 307200000}},
    {{4096000, 12288000, 49152000}, {4096000, 16384000, 32768000}, {4096000, 19200000, 307200000}},
    {{5644800, 11289600, 22579200}},
    {{6144000, 12288000, 49152000}, {6144000, 19200000, 307200000}},
    {{8192000, 12288000, 49152000}, {8192000, 16384000, 32768000}},
    {{11289600, 11289600, 22579200}},
    {{12288000, 12288000, 49152000}, {12288000, 19200000, 307200000}},
    {{24576000, 12288000, 49152000}},
    {{49152000, 12288000, 49152000}}};
#endif
#endif

static const char *_HalAudI2sEnumToString(CHIP_AIO_I2S_e eAioI2s)
{
    switch (eAioI2s)
    {
        GENERATE_CASE(E_CHIP_AIO_I2S_RX_A, I2S_RX_A)
        GENERATE_CASE(E_CHIP_AIO_I2S_TX_A, I2S_TX_A)
        default:
        {
            return "Unknown I2s";
        }
    }
}

static U32 _float_to_hex(int bachPll, U32 nMckFreq)
{
    U32 data = 0;
    U32 max  = 1000000; // There are six decimals behind the decimal point.
    U64 f    = bachPll % nMckFreq;
    U8  i    = 0;

    if (f == 0)
    {
        return f;
    }

    f *= max;
    f /= nMckFreq;

    for (i = 0; i < 16; i++)
    {
        data <<= 1;
        f *= 2;
        if (f >= max)
        {
            data |= 0x01;
            f -= max;
        }
        if (f == 0)
        {
            data <<= 15 - i;
            break;
        }
    }

    return data;
}

static BOOL _HalAudI2sCanDisable(CHIP_AIO_I2S_e eAioI2s)
{
    switch (eAioI2s)
    {
        case E_CHIP_AIO_I2S_RX_A:
            if (g_aI2sCfg[eAioI2s].eWireMode == E_AUD_I2S_WIRE_6 || !g_aI2sCfg[E_CHIP_AIO_I2S_TX_A].bBckActive)
            {
                return true;
            }
            break;
        case E_CHIP_AIO_I2S_TX_A:
            if (g_aI2sCfg[eAioI2s].eWireMode == E_AUD_I2S_WIRE_4 && !g_aI2sCfg[E_CHIP_AIO_I2S_RX_A].bBckActive)
            {
                return true;
            }
            break;
        default:
            break;
    }

    return false;
}

static int _HalAudI2sSetBck(CHIP_AIO_I2S_e eAioI2s, U32 nNfValue)
{
    U16 bckValueLo = 0;
    U16 bckValueHi = 0;
    U32 rate       = 0;
    U32 width      = 0;
    U32 nBckFreq   = 0;
    U16 nDiv       = 0;

    if (nNfValue == 0)
    {
        ERRMSG("nNfValue %d Fail !", nNfValue);
        goto FAIL;
    }

    bckValueLo = nNfValue & 0xFFFF;
    bckValueHi = (nNfValue >> 16) & 0xFFFF;

    rate  = HalAudApiEnumToRate(g_aI2sCfg[eAioI2s].eRate);
    width = HalAudApiEnumToBitWidth(g_aI2sCfg[eAioI2s].enI2sWidth);
    if (rate <= 0 || width <= 0)
    {
        goto FAIL;
    }

    nBckFreq = rate * width * g_aI2sCfg[eAioI2s].nChannelNum;
    if (nBckFreq > I2S_8DIV_MAX_FREQ)
    {
        nDiv = 0;
    }
    else if (nBckFreq > I2S_AUPLL_DIV_SUITABLE_FREQ)
    {
        nDiv = 3;
    }
    else
    {
        nDiv = 0;
    }
#if SSTAR_FOR_FPGA_TEST
    nDiv = 0;
#endif
    switch (eAioI2s)
    {
        case E_CHIP_AIO_I2S_RX_A:
            // for master RX BCK div1/div2/div4/div8 selection
            HalBachWriteReg(E_BACH_REG_BANK2, AUDIO_BANK2_REG_I2S_CFG_00_ADDR,
                            AUDIO_BANK2_REG_I2S_CFG_00_REG_SEL_CLK_CODEC_I2S_RX_BCK_MASK,
                            nDiv << AUDIO_BANK2_REG_I2S_CFG_00_REG_SEL_CLK_CODEC_I2S_RX_BCK_SHIFT);
            //
            HalBachWriteReg(E_BACH_REG_BANK4, AUDIO_BANK4_REG_NF_SYNTH_RX_BCK_01_ADDR,
                            AUDIO_BANK4_REG_NF_SYNTH_RX_BCK_01_REG_CODEC_RX_BCK_SYNTH_NF_VALUE_LOW_MASK, bckValueLo);
            HalBachWriteReg(E_BACH_REG_BANK4, AUDIO_BANK4_REG_NF_SYNTH_RX_BCK_02_ADDR,
                            AUDIO_BANK4_REG_NF_SYNTH_RX_BCK_02_REG_CODEC_RX_BCK_SYNTH_NF_VALUE_HIGH_MASK, bckValueHi);
            break;

        case E_CHIP_AIO_I2S_TX_A:
            if (g_aI2sCfg[eAioI2s].eWireMode == E_AUD_I2S_WIRE_6)
            {
                HalBachWriteReg(E_BACH_REG_BANK2, AUDIO_BANK2_REG_I2S_CFG_00_ADDR,
                                AUDIO_BANK2_REG_I2S_CFG_00_REG_SEL_CLK_CODEC_I2S_TX_BCK_MASK,
                                nDiv << AUDIO_BANK2_REG_I2S_CFG_00_REG_SEL_CLK_CODEC_I2S_TX_BCK_SHIFT);
                //
                HalBachWriteReg(E_BACH_REG_BANK4, AUDIO_BANK4_REG_NF_SYNTH_TX_BCK_01_ADDR,
                                AUDIO_BANK4_REG_NF_SYNTH_TX_BCK_01_REG_CODEC_TX_BCK_SYNTH_NF_VALUE_LOW_MASK,
                                bckValueLo);
                HalBachWriteReg(E_BACH_REG_BANK4, AUDIO_BANK4_REG_NF_SYNTH_TX_BCK_02_ADDR,
                                AUDIO_BANK4_REG_NF_SYNTH_TX_BCK_02_REG_CODEC_TX_BCK_SYNTH_NF_VALUE_HIGH_MASK,
                                bckValueHi);
            }
            else
            {
                // Support 4-wire mode I2S TXA running alone
                HalBachWriteReg(E_BACH_REG_BANK2, AUDIO_BANK2_REG_I2S_CFG_00_ADDR,
                                AUDIO_BANK2_REG_I2S_CFG_00_REG_SEL_CLK_CODEC_I2S_RX_BCK_MASK,
                                nDiv << AUDIO_BANK2_REG_I2S_CFG_00_REG_SEL_CLK_CODEC_I2S_RX_BCK_SHIFT);
                //
                HalBachWriteReg(E_BACH_REG_BANK4, AUDIO_BANK4_REG_NF_SYNTH_RX_BCK_01_ADDR,
                                AUDIO_BANK4_REG_NF_SYNTH_RX_BCK_01_REG_CODEC_RX_BCK_SYNTH_NF_VALUE_LOW_MASK,
                                bckValueLo);
                HalBachWriteReg(E_BACH_REG_BANK4, AUDIO_BANK4_REG_NF_SYNTH_RX_BCK_02_ADDR,
                                AUDIO_BANK4_REG_NF_SYNTH_RX_BCK_02_REG_CODEC_RX_BCK_SYNTH_NF_VALUE_HIGH_MASK,
                                bckValueHi);
            }
            break;
        default:
            goto FAIL;
            break;
    }
    DBGMSG(AUDIO_DBG_LEVEL_I2S, "eAioI2s[%s] nNfValue[%d] nSampleRate[%d] Width[%d] nBckFreq[%d]",
           _HalAudI2sEnumToString(eAioI2s), nNfValue, HalAudApiEnumToRate(g_aI2sCfg[eAioI2s].eRate), width, nBckFreq);

    return AIO_OK;

FAIL:
    ERRMSG("eAioI2s[%s] nNfValue[%d] nSampleRate[%d] Width[%d]!", _HalAudI2sEnumToString(eAioI2s), nNfValue,
           HalAudApiEnumToRate(g_aI2sCfg[eAioI2s].eRate), width);
    return AIO_NG;
}

static U32 _HalAudBckCalculate(AudRate_e eRate, AudBitWidth_e enI2sWidth, U16 Chn)
{
    U32 value    = 0;
    U32 rate     = 0;
    U32 width    = 0;
    U32 channel  = Chn;
    U32 nBckFreq = 0;

    rate  = HalAudApiEnumToRate(eRate);
    width = HalAudApiEnumToBitWidth(enI2sWidth);
    if (rate <= 0 || width <= 0)
    {
        goto FAIL;
    }

    nBckFreq = rate * width * channel;
    if (nBckFreq > I2S_AUPLL_DIV_SUITABLE_FREQ)
    {
        if (nBckFreq > I2S_8DIV_MAX_FREQ)
        {
            // div0, because of BACH_RX_DIV_SEL_MSK and AUDIO_BANK2_REG_I2S_CFG_00_REG_SEL_CLK_CODEC_I2S_TX_BCK_MASK set
            // to 0x0            HalBachWriteReg(E_BACH_REG_BANK4, AUDIO_BANK4_REG_NF_SYNTH_RX_BCK_00_ADDR,
            // AUDIO_BANK4_REG_NF_SYNTH_RX_BCK_00_REG_SEL_CLK_NF_SYNTH_RX_REF_MASK,
            // 0x0<<AUDIO_BANK4_REG_NF_SYNTH_RX_BCK_00_REG_SEL_CLK_NF_SYNTH_RX_REF_SHIFT);
            // HalBachWriteReg(E_BACH_REG_BANK4, AUDIO_BANK4_REG_NF_SYNTH_TX_BCK_00_ADDR,
            // AUDIO_BANK4_REG_NF_SYNTH_TX_BCK_00_REG_SEL_CLK_NF_SYNTH_TX_REF_MASK,
            // 0x0<<AUDIO_BANK4_REG_NF_SYNTH_TX_BCK_00_REG_SEL_CLK_NF_SYNTH_TX_REF_SHIFT);
            value = ((1 << 22) * REF_FREQ / (rate * width * channel / 2000)) * 1000;
        }
        else
        {
            // div8, because of BACH_RX_DIV_SEL_MSK and AUDIO_BANK2_REG_I2S_CFG_00_REG_SEL_CLK_CODEC_I2S_TX_BCK_MASK set
            // to 0x3
            HalBachWriteReg(E_BACH_REG_BANK4, AUDIO_BANK4_REG_NF_SYNTH_RX_BCK_00_ADDR,
                            AUDIO_BANK4_REG_NF_SYNTH_RX_BCK_00_REG_SEL_CLK_NF_SYNTH_RX_REF_MASK,
                            0x0 << AUDIO_BANK4_REG_NF_SYNTH_RX_BCK_00_REG_SEL_CLK_NF_SYNTH_RX_REF_SHIFT);
            HalBachWriteReg(E_BACH_REG_BANK4, AUDIO_BANK4_REG_NF_SYNTH_TX_BCK_00_ADDR,
                            AUDIO_BANK4_REG_NF_SYNTH_TX_BCK_00_REG_SEL_CLK_NF_SYNTH_TX_REF_MASK,
                            0x0 << AUDIO_BANK4_REG_NF_SYNTH_TX_BCK_00_REG_SEL_CLK_NF_SYNTH_TX_REF_SHIFT);
            value = ((1 << 22) * REF_FREQ / (rate * width * channel / 2000) / 8) * 1000;
        }
    }
    else
    {
        // I2S RX TX CLK Select 48M for fpga
        HalBachWriteReg(E_BACH_REG_BANK4, AUDIO_BANK4_REG_NF_SYNTH_RX_BCK_00_ADDR,
                        AUDIO_BANK4_REG_NF_SYNTH_RX_BCK_00_REG_SEL_CLK_NF_SYNTH_RX_REF_MASK,
                        0x1 << AUDIO_BANK4_REG_NF_SYNTH_RX_BCK_00_REG_SEL_CLK_NF_SYNTH_RX_REF_SHIFT);
        HalBachWriteReg(E_BACH_REG_BANK4, AUDIO_BANK4_REG_NF_SYNTH_TX_BCK_00_ADDR,
                        AUDIO_BANK4_REG_NF_SYNTH_TX_BCK_00_REG_SEL_CLK_NF_SYNTH_TX_REF_MASK,
                        0x1 << AUDIO_BANK4_REG_NF_SYNTH_TX_BCK_00_REG_SEL_CLK_NF_SYNTH_TX_REF_SHIFT);
        // div0, because of BACH_RX_DIV_SEL_MSK and AUDIO_BANK2_REG_I2S_CFG_00_REG_SEL_CLK_CODEC_I2S_TX_BCK_MASK set to
        // 0x0
        value = ((1 << 22) * REF_FREQ_AUPLL / (rate * width * channel / 2000)) * 1000;
    }
    DBGMSG(AUDIO_DBG_LEVEL_I2S, "eRate[%d] enI2sWidth[%d] Chn[%d] value[%d]", eRate, enI2sWidth, Chn, value);

    return value;
FAIL:
    ERRMSG("eRate[%d] enI2sWidth[%d] Chn[%d] Fail!", eRate, enI2sWidth, Chn);
    return 0;
}

static int _HalAudI2sSetBachPLL(CHIP_AIO_I2S_e eAioI2s, AudRate_e eRate, AudBitWidth_e eI2sWidth, int nChannelNum)
{
    U32 rate, width, nBckFreq, nMckFreq;
    U32 ret = 0;

    rate  = HalAudApiEnumToRate(eRate);
    width = HalAudApiEnumToBitWidth(eI2sWidth);
    if (rate <= 0 || width <= 0)
    {
        goto FAIL;
    }

    nBckFreq = rate * width * nChannelNum;

    nMckFreq = HalAudApiEnumToMck(g_aI2sCfg[eAioI2s].eMck);
    HalAudI2sSetBckClkSrc(eAioI2s, E_CLK_SRC_384M);
    if (ret != AIO_OK)
    {
        ERRMSG("set Bach PLL %d Fail !", nMckFreq);
        goto FAIL;
    }

    DBGMSG(AUDIO_DBG_LEVEL_I2S, "eAioI2s[%s] eRate[%d] eI2sWidth[%d] nChannelNum[%d] nBckFreq[%d] nMckFreq[%d]",
           _HalAudI2sEnumToString(eAioI2s), eRate, eI2sWidth, nChannelNum, nBckFreq, nMckFreq);

    return AIO_OK;

FAIL:
    ERRMSG("eAioI2s[%s] eRate[%d], eI2sWidth[%d], nChannelNum[%d]", _HalAudI2sEnumToString(eAioI2s), eRate, eI2sWidth,
           nChannelNum);
    return AIO_NG;
}

//--------------------------------------------------------------------------------

int HalAudI2sSetBckClkSrc(CHIP_AIO_I2S_e eAioI2s, CLK_SRC_e clk_src)
{
#if SSTAR_FOR_FPGA_TEST
    clk_src = 0;
#endif

    switch (eAioI2s)
    {
        case E_CHIP_AIO_I2S_TX_A:
            HalBachWriteReg(E_BACH_REG_BANK4, AUDIO_BANK4_REG_NF_SYNTH_TX_BCK_00_ADDR,
                            AUDIO_BANK4_REG_NF_SYNTH_TX_BCK_00_REG_SEL_CLK_NF_SYNTH_TX_REF_MASK,
                            clk_src << AUDIO_BANK4_REG_NF_SYNTH_TX_BCK_00_REG_SEL_CLK_NF_SYNTH_TX_REF_SHIFT);
            break;
        case E_CHIP_AIO_I2S_RX_A:
            HalBachWriteReg(E_BACH_REG_BANK4, AUDIO_BANK4_REG_NF_SYNTH_RX_BCK_00_ADDR,
                            AUDIO_BANK4_REG_NF_SYNTH_RX_BCK_00_REG_SEL_CLK_NF_SYNTH_RX_REF_MASK,
                            clk_src << AUDIO_BANK4_REG_NF_SYNTH_RX_BCK_00_REG_SEL_CLK_NF_SYNTH_RX_REF_SHIFT);
            break;
        default:
            break;
    }

    // Use fix 384M
    if (clk_src == E_CLK_SRC_384M)
    {
        g_BachPllVal = FREQ_384MHZ;
    }

    return AIO_OK;
}

int HalAudI2sSetMckClkSrc(CHIP_AIO_I2S_e eAioI2s, CLK_SRC_e clk_src)
{
#if SSTAR_FOR_FPGA_TEST
    clk_src = 0;
#endif
    switch (eAioI2s)
    {
        case E_CHIP_AIO_I2S_RX_A:
        case E_CHIP_AIO_I2S_TX_A:
            HalBachWriteReg(E_BACH_REG_BANK4, AUDIO_BANK4_REG_NF_SYNTH_MCK_00_ADDR,
                            AUDIO_BANK4_REG_NF_SYNTH_MCK_00_REG_SEL_CLK_NF_SYNTH_REF_MASK,
                            clk_src << AUDIO_BANK4_REG_NF_SYNTH_MCK_00_REG_SEL_CLK_NF_SYNTH_REF_SHIFT);
            break;
        default:
            break;
    }
    return AIO_OK;
}

int HalAudI2sSetTdmDetails(CHIP_AIO_I2S_e eAioI2s)
{
    AudReg_t nPgm, nWsInv, nWsWidth, nBckInv, nSwap, nWsDly, nSlot = {0};
    switch (eAioI2s)
    {
        case E_CHIP_AIO_I2S_RX_A:
            nPgm.addr      = AUDIO_BANK2_REG_I2S_TDM_CFG_00_ADDR;
            nPgm.mask      = AUDIO_BANK2_REG_I2S_TDM_CFG_00_REG_CODEC_I2S_RX_WS_FMT_MASK;
            nPgm.key       = I2SA_RX0_TDM_WS_PGM;
            nWsInv.addr    = AUDIO_BANK2_REG_I2S_TDM_CFG_00_ADDR;
            nWsInv.mask    = AUDIO_BANK2_REG_I2S_TDM_CFG_00_REG_CODEC_I2S_RX_WS_INV_MASK;
            nWsInv.key     = I2SA_RX0_TDM_WS_INV;
            nWsWidth.addr  = AUDIO_BANK2_REG_I2S_TDM_CFG_01_ADDR;
            nWsWidth.mask  = AUDIO_BANK2_REG_I2S_TDM_CFG_01_REG_CODEC_I2S_RX_WS_WDTH_MASK;
            nWsWidth.shift = AUDIO_BANK2_REG_I2S_TDM_CFG_01_REG_CODEC_I2S_RX_WS_WDTH_SHIFT;
            nWsWidth.key   = I2SA_RX0_TDM_WS_WIDTH;
            nSwap.addr     = AUDIO_BANK2_REG_I2S_TDM_CFG_02_ADDR;
            nSwap.mask     = AUDIO_BANK2_REG_I2S_TDM_CFG_02_REG_CODEC_I2S_RX_CH_SWAP_MASK;
            nSwap.shift    = AUDIO_BANK2_REG_I2S_TDM_CFG_02_REG_CODEC_I2S_RX_CH_SWAP_SHIFT;
            nSwap.key      = I2SA_RX0_TDM_CH_SWAP;
            nBckInv.addr   = AUDIO_BANK2_REG_I2S_CFG_00_ADDR;
            nBckInv.mask   = AUDIO_BANK2_REG_I2S_CFG_00_REG_INV_CLK_CODEC_I2S_RX_BCK_MASK;
            nBckInv.shift  = AUDIO_BANK2_REG_I2S_CFG_00_REG_INV_CLK_CODEC_I2S_RX_BCK_SHIFT;
            nBckInv.key    = I2SA_RX0_TDM_BCK_INV;
            nWsDly.addr    = AUDIO_BANK2_REG_I2S_TDM_CFG_02_ADDR;
            nWsDly.mask    = AUDIO_BANK2_REG_I2S_TDM_CFG_02_REG_CODEC_I2S_RX_WS_DLY_MASK;
            nWsDly.shift   = AUDIO_BANK2_REG_I2S_TDM_CFG_02_REG_CODEC_I2S_RX_WS_DLY_SHIFT;
            nWsDly.key     = I2SA_RX0_SHORT_FF_MODE;
            break;

        case E_CHIP_AIO_I2S_TX_A:
            nPgm.addr      = AUDIO_BANK2_REG_I2S_TDM_CFG_05_ADDR;
            nPgm.mask      = AUDIO_BANK2_REG_I2S_TDM_CFG_05_REG_CODEC_I2S_TX_WS_FMT_MASK;
            nPgm.key       = I2SA_TX0_TDM_WS_PGM;
            nWsInv.addr    = AUDIO_BANK2_REG_I2S_TDM_CFG_05_ADDR;
            nWsInv.mask    = AUDIO_BANK2_REG_I2S_TDM_CFG_05_REG_CODEC_I2S_TX_WS_INV_MASK;
            nWsInv.key     = I2SA_TX0_TDM_WS_INV;
            nWsWidth.addr  = AUDIO_BANK2_REG_I2S_TDM_CFG_01_ADDR;
            nWsWidth.mask  = AUDIO_BANK2_REG_I2S_TDM_CFG_01_REG_CODEC_I2S_TX_WS_WDTH_MASK;
            nWsWidth.shift = AUDIO_BANK2_REG_I2S_TDM_CFG_01_REG_CODEC_I2S_TX_WS_WDTH_SHIFT;
            nWsWidth.key   = I2SA_TX0_TDM_WS_WIDTH;
            nSwap.addr     = AUDIO_BANK2_REG_I2S_TDM_CFG_02_ADDR;
            nSwap.mask     = AUDIO_BANK2_REG_I2S_TDM_CFG_02_REG_CODEC_I2S_TX_CH_SWAP_MASK;
            nSwap.shift    = AUDIO_BANK2_REG_I2S_TDM_CFG_02_REG_CODEC_I2S_TX_CH_SWAP_SHIFT;
            nSwap.key      = I2SA_TX0_TDM_CH_SWAP;
            nSlot.addr     = AUDIO_BANK2_REG_I2S_TDM_CFG_06_ADDR;
            nSlot.mask     = AUDIO_BANK2_REG_I2S_TDM_CFG_06_REG_CODEC_I2S_TX_ACT_SLOT_MASK;
            nSlot.shift    = AUDIO_BANK2_REG_I2S_TDM_CFG_06_REG_CODEC_I2S_TX_ACT_SLOT_SHIFT;
            nSlot.key      = I2SA_TX0_TDM_ACTIVE_SLOT;
            nBckInv.addr   = AUDIO_BANK2_REG_I2S_CFG_00_ADDR;
            nBckInv.mask   = AUDIO_BANK2_REG_I2S_CFG_00_REG_INV_CLK_CODEC_I2S_TX_BCK_MASK;
            nBckInv.shift  = AUDIO_BANK2_REG_I2S_CFG_00_REG_INV_CLK_CODEC_I2S_TX_BCK_SHIFT;
            nBckInv.key    = I2SA_TX0_TDM_BCK_INV;
            nWsDly.addr    = AUDIO_BANK2_REG_I2S_TDM_CFG_02_ADDR;
            nWsDly.mask    = AUDIO_BANK2_REG_I2S_TDM_CFG_02_REG_CODEC_I2S_TX_SDO_DLY_MASK;
            nWsDly.shift   = AUDIO_BANK2_REG_I2S_TDM_CFG_02_REG_CODEC_I2S_TX_SDO_DLY_SHIFT;
            nWsDly.key     = I2SA_TX0_SHORT_FF_MODE;
            break;

        default:
            goto FAIL;
            break;
    }
    nSwap.val = 0;
    nSwap.val |= HalAudApiGetDtsValues(nSwap.key, 0) << 1;
    nSwap.val |= HalAudApiGetDtsValues(nSwap.key, 1) << 0;
    nSwap.val |= HalAudApiGetDtsValues(nSwap.key, 2) << 3;
    nSwap.val |= HalAudApiGetDtsValues(nSwap.key, 3) << 2;
    HalBachWriteReg(E_BACH_REG_BANK2, nPgm.addr, nPgm.mask, (HalAudApiGetDtsValue(nPgm.key) ? nPgm.mask : 0));
    HalBachWriteReg(E_BACH_REG_BANK2, nWsInv.addr, nWsInv.mask, (HalAudApiGetDtsValue(nWsInv.key) ? nWsInv.mask : 0));
    HalBachWriteReg(E_BACH_REG_BANK2, nWsWidth.addr, nWsWidth.mask,
                    (HalAudApiGetDtsValue(nWsWidth.key) << nWsWidth.shift));
    HalBachWriteReg(E_BACH_REG_BANK2, nSwap.addr, nSwap.mask, (nSwap.val << nSwap.shift));
    HalBachWriteReg(E_BACH_REG_BANK2, nBckInv.addr, nBckInv.mask, (HalAudApiGetDtsValue(nBckInv.key) << nBckInv.shift));
    HalBachWriteReg(E_BACH_REG_BANK2, nWsDly.addr, nWsDly.mask, (HalAudApiGetDtsValue(nWsDly.key) << nWsDly.shift));
    if (nSlot.addr != 0)
    {
        HalBachWriteReg(E_BACH_REG_BANK2, nSlot.addr, nSlot.mask, (HalAudApiGetDtsValue(nSlot.key) << nSlot.shift));
    }
    DBGMSG(AUDIO_DBG_LEVEL_I2S, "eAioI2s[%s] wsPgm[%d] wsInv[%d] wsWidth[%d] nSwap[%d] bckInv[%d] shortFFMode[%d]",
           _HalAudI2sEnumToString(eAioI2s), HalAudApiGetDtsValue(nPgm.key), HalAudApiGetDtsValue(nWsInv.key),
           HalAudApiGetDtsValue(nWsWidth.key), nSwap.val, HalAudApiGetDtsValue(nBckInv.key),
           HalAudApiGetDtsValue(nWsDly.key));

    return AIO_OK;
FAIL:
    ERRMSG("eAioI2s[%d] Fail!", eAioI2s);
    return AIO_NG;
}

int HalAudI2sSetRate(CHIP_AIO_I2S_e eAioI2s, AudRate_e eRate)
{
    g_aI2sCfg[eAioI2s].eRate = eRate;

    return _HalAudI2sSetBck(eAioI2s, _HalAudBckCalculate(g_aI2sCfg[eAioI2s].eRate, g_aI2sCfg[eAioI2s].enI2sWidth,
                                                         g_aI2sCfg[eAioI2s].nChannelNum));
}

int HalAudI2sSetMckDivAndEnable(CHIP_AIO_I2S_e eAioI2s)
{
    U16         nDuty    = 0;
    U32         nNf_Hi   = 0;
    U32         nNf_Lo   = 0;
    AudI2sMck_e eMck     = 0;
    int         ret      = 0;
    U32         nMckFreq = 0;

    eMck = g_aI2sCfg[eAioI2s].eMck;

    DBGMSG(AUDIO_DBG_LEVEL_I2S, "eAioI2s[%s] eMck[%d]", _HalAudI2sEnumToString(eAioI2s), eMck);

    if (eMck == E_AUD_I2S_MCK_NULL)
    {
        // do nothing
        return AIO_OK;
    }

    nMckFreq = HalAudApiEnumToMck(eMck);

    nNf_Hi = g_BachPllVal / nMckFreq;
    nNf_Lo = _float_to_hex(g_BachPllVal, nMckFreq);
    nDuty  = (U16)((U32)(g_BachPllVal / 1000) * 50 / (nMckFreq / 1000) / 100);

    DBGMSG(AUDIO_DBG_LEVEL_I2S, "eAioI2s[%s] nMckFreq[%d] nNf_Hi[0x%x] nNf_Lo[0x%x] nDuty[0x%x]",
           _HalAudI2sEnumToString(eAioI2s), nMckFreq, nNf_Hi, nNf_Lo, nDuty);

    switch (eAioI2s)
    {
        case E_CHIP_AIO_I2S_TX_A:
        case E_CHIP_AIO_I2S_RX_A:
            HalBachWriteReg(E_BACH_REG_BANK4, AUDIO_BANK4_REG_NF_SYNTH_MCK_02_ADDR,
                            AUDIO_BANK4_REG_NF_SYNTH_MCK_02_REG_CODEC_MCK_SYNTH_NF_VALUE_HIGH_MASK,
                            (nNf_Hi)&AUDIO_BANK4_REG_NF_SYNTH_MCK_02_REG_CODEC_MCK_SYNTH_NF_VALUE_HIGH_MASK);
            HalBachWriteReg(E_BACH_REG_BANK4, AUDIO_BANK4_REG_NF_SYNTH_MCK_01_ADDR,
                            AUDIO_BANK4_REG_NF_SYNTH_MCK_01_REG_CODEC_MCK_SYNTH_NF_VALUE_LOW_MASK,
                            (nNf_Lo)&AUDIO_BANK4_REG_NF_SYNTH_MCK_01_REG_CODEC_MCK_SYNTH_NF_VALUE_LOW_MASK);
            // Duty
            HalBachWriteReg(E_BACH_REG_BANK4, AUDIO_BANK4_REG_NF_SYNTH_MCK_02_ADDR,
                            AUDIO_BANK4_REG_NF_SYNTH_MCK_02_REG_CODEC_MCK_EXPAND_VALUE_MASK,
                            (nDuty << AUDIO_BANK4_REG_NF_SYNTH_MCK_02_REG_CODEC_MCK_EXPAND_VALUE_SHIFT));

            break;
        default:
            goto FAIL;
    }

    ret |= HalAudI2sEnableMck(eAioI2s, true);
    if (ret != AIO_OK)
    {
        goto FAIL;
    }

    return AIO_OK;

FAIL:
    ERRMSG("eAioI2s[%s] eMck[%d] Fail !", _HalAudI2sEnumToString(eAioI2s), eMck);
    return AIO_NG;
}

int HalAudI2sSetClkRef(CHIP_AIO_I2S_e eAioI2s, AudI2sClkRef_e eI2sClkRef)
{
    U16            nClkRefSetting = 0;
    AudReg_t       nReadVaild     = {0};
    CHIP_AIO_I2S_e i;

    for (i = E_CHIP_AIO_I2S_RX_START; i <= E_CHIP_AIO_I2S_RX_END; i++)
    {
        if (i == E_CHIP_AIO_I2S_RX_A)
        {
            nReadVaild.addr  = AUDIO_BANK1_REG_DMA_SOURCE_CFG_02_ADDR;
            nReadVaild.mask  = AUDIO_BANK1_REG_DMA_SOURCE_CFG_02_REG_I2S_TDM_RX_AFIFO_READ_VALID_SEL_MASK;
            nReadVaild.shift = AUDIO_BANK1_REG_DMA_SOURCE_CFG_02_REG_I2S_TDM_RX_AFIFO_READ_VALID_SEL_SHIFT;
        }

        switch (eI2sClkRef)
        {
            case E_AUD_I2S_CLK_REF_DMIC:
                nClkRefSetting = 0;
                break;
            case E_AUD_I2S_CLK_REF_ADC:
                nClkRefSetting = 1;
                break;
            case E_AUD_I2S_CLK_REF_I2S_TDM_RX:
                nClkRefSetting = 2;
                break;
            case E_AUD_I2S_CLK_REF_SRC:
                nClkRefSetting = 3;
                break;

            default:
                ERRMSG("eAioI2s[%s] eI2sClkRef[%d] Fail !", _HalAudI2sEnumToString(eAioI2s), eI2sClkRef);
                goto FAIL;
                break;
        }

        HalBachWriteReg(E_BACH_REG_BANK1, nReadVaild.addr, nReadVaild.mask, (nClkRefSetting << nReadVaild.shift));
    }

    return AIO_OK;
FAIL:
    return AIO_NG;
}

int HalAudI2sSetTdmMode(CHIP_AIO_I2S_e eAioI2s, AudI2sMode_e enI2sMode)
{
    g_aI2sCfg[eAioI2s].enI2sMode = enI2sMode;

    return AIO_OK;
}

int HalAudI2sSetMsMode(CHIP_AIO_I2S_e eAioI2s, AudI2sMsMode_e eMsMode)
{
    U16 nMsMode;

    switch (eMsMode)
    {
        case E_AUD_I2S_MSMODE_MASTER:
            nMsMode = 1;
            break;
        case E_AUD_I2S_MSMODE_SLAVE:
            nMsMode = 0;
            break;
        default:
            ERRMSG("eMsMode[%d] Fail !", eMsMode);
            goto FAIL;
            break;
    }

    switch (eAioI2s)
    {
        case E_CHIP_AIO_I2S_RX_A:
            HalBachWriteReg(E_BACH_REG_BANK2, AUDIO_BANK2_REG_I2S_TDM_CFG_00_ADDR,
                            AUDIO_BANK2_REG_I2S_TDM_CFG_00_REG_CODEC_I2S_RX_MS_MODE_MASK,
                            (nMsMode ? AUDIO_BANK2_REG_I2S_TDM_CFG_00_REG_CODEC_I2S_RX_MS_MODE_MASK : 0));
            break;
        case E_CHIP_AIO_I2S_TX_A:
            // Support 4-wire mode I2S TXA running alone
            if (g_aI2sCfg[eAioI2s].eWireMode == E_AUD_I2S_WIRE_4 && eMsMode == E_AUD_I2S_MSMODE_MASTER)
            {
                HalBachWriteReg(E_BACH_REG_BANK2, AUDIO_BANK2_REG_I2S_TDM_CFG_00_ADDR,
                                AUDIO_BANK2_REG_I2S_TDM_CFG_00_REG_CODEC_I2S_RX_MS_MODE_MASK,
                                AUDIO_BANK2_REG_I2S_TDM_CFG_00_REG_CODEC_I2S_RX_MS_MODE_MASK);
            }
            HalBachWriteReg(E_BACH_REG_BANK2, AUDIO_BANK2_REG_I2S_TDM_CFG_05_ADDR,
                            AUDIO_BANK2_REG_I2S_TDM_CFG_05_REG_CODEC_I2S_TX_MS_MODE_MASK,
                            (nMsMode ? AUDIO_BANK2_REG_I2S_TDM_CFG_05_REG_CODEC_I2S_TX_MS_MODE_MASK : 0));
            break;
        default:
            ERRMSG("eAioI2s[%d] Fail !", eAioI2s);
            goto FAIL;
            break;
    }

#if (SSTAR_FOR_FPGA_TEST == 0)
    switch (eAioI2s)
    {
        case E_CHIP_AIO_I2S_TX_A:
            /* enable Tx out mode when Tx as slave */ // FPGA test close this setting
            HalBachWriteReg(
                E_BACH_REG_BANK2, AUDIO_BANK2_REG_I2S_TDM_CFG_05_ADDR,
                AUDIO_BANK2_REG_I2S_TDM_CFG_05_REG_CODEC_I2S_TX_OUT_MODE_MASK,
                eMsMode == E_AUD_I2S_MSMODE_SLAVE ? AUDIO_BANK2_REG_I2S_TDM_CFG_05_REG_CODEC_I2S_TX_OUT_MODE_MASK : 0);
            break;
        default:
            break;
    }
#endif

    DBGMSG(AUDIO_DBG_LEVEL_I2S, "eAioI2s[%s] eMsMode[%d]", _HalAudI2sEnumToString(eAioI2s), eMsMode);

    g_aI2sCfg[eAioI2s].eMsMode = eMsMode;

    return AIO_OK;
FAIL:
    return AIO_NG;
}

int HalAudI2sSetFmt(CHIP_AIO_I2S_e eAioI2s, AudI2sFmt_e enI2sFmt)
{
    U16 nSel;

    switch (enI2sFmt)
    {
        case E_AUD_I2S_FMT_I2S:
            nSel = 0;
            break;
        case E_AUD_I2S_FMT_LEFT_JUSTIFY:
            nSel = 1;
            break;
        default:
            ERRMSG("enI2sFmt[%d] Fail !", enI2sFmt);
            goto FAIL;
            break;
    }

    switch (eAioI2s)
    {
        case E_CHIP_AIO_I2S_RX_A:
            HalBachWriteReg(E_BACH_REG_BANK2, AUDIO_BANK2_REG_I2S_TDM_CFG_00_ADDR,
                            AUDIO_BANK2_REG_I2S_TDM_CFG_00_REG_CODEC_I2S_RX_FMT_MASK,
                            (nSel ? AUDIO_BANK2_REG_I2S_TDM_CFG_00_REG_CODEC_I2S_RX_FMT_MASK : 0));
            break;
        case E_CHIP_AIO_I2S_TX_A:
            // Support 4-wire mode I2S TXA running alone
            if (g_aI2sCfg[eAioI2s].eWireMode == E_AUD_I2S_WIRE_4)
            {
                HalBachWriteReg(E_BACH_REG_BANK2, AUDIO_BANK2_REG_I2S_TDM_CFG_00_ADDR,
                                AUDIO_BANK2_REG_I2S_TDM_CFG_00_REG_CODEC_I2S_RX_FMT_MASK,
                                (nSel ? AUDIO_BANK2_REG_I2S_TDM_CFG_00_REG_CODEC_I2S_RX_FMT_MASK : 0));
            }
            HalBachWriteReg(E_BACH_REG_BANK2, AUDIO_BANK2_REG_I2S_TDM_CFG_05_ADDR,
                            AUDIO_BANK2_REG_I2S_TDM_CFG_05_REG_CODEC_I2S_TX_FMT_MASK,
                            (nSel ? AUDIO_BANK2_REG_I2S_TDM_CFG_05_REG_CODEC_I2S_TX_FMT_MASK : 0));
            break;
        default:
            goto FAIL;
            break;
    }

    g_aI2sCfg[eAioI2s].eFormat = enI2sFmt;
    DBGMSG(AUDIO_DBG_LEVEL_I2S, "eAioI2s[%s] enI2sFmt[%d]", _HalAudI2sEnumToString(eAioI2s), enI2sFmt);

    return AIO_OK;
FAIL:
    ERRMSG("eAioI2s[%d] enI2sFmt[%d] Fail !", eAioI2s, enI2sFmt);
    return AIO_NG;
}

int HalAudI2sSetWidth(CHIP_AIO_I2S_e eAioI2s, AudBitWidth_e enI2sWidth)
{
    U16 nTxSel, nRxSel;

    switch (enI2sWidth)
    {
        case E_AUD_BITWIDTH_16:
            nRxSel = 0;
            nTxSel = 0;
            break;
        case E_AUD_BITWIDTH_24:
            nRxSel = 2;
            nTxSel = 1;
            break;
        case E_AUD_BITWIDTH_32:
            nRxSel = 1;
            nTxSel = 2;
            break;
        default:
            goto FAIL;
            break;
    }

    switch (eAioI2s)
    {
        case E_CHIP_AIO_I2S_RX_A:
            HalBachWriteReg(E_BACH_REG_BANK2, AUDIO_BANK2_REG_I2S_TDM_CFG_00_ADDR,
                            AUDIO_BANK2_REG_I2S_TDM_CFG_00_REG_CODEC_I2S_RX_ENC_WDTH_MASK,
                            (nRxSel << AUDIO_BANK2_REG_I2S_TDM_CFG_00_REG_CODEC_I2S_RX_ENC_WDTH_SHIFT));
            break;
        case E_CHIP_AIO_I2S_TX_A:
            // Support 4-wire mode I2S TXA running alone
            if (g_aI2sCfg[eAioI2s].eWireMode == E_AUD_I2S_WIRE_4)
            {
                HalBachWriteReg(E_BACH_REG_BANK2, AUDIO_BANK2_REG_I2S_TDM_CFG_00_ADDR,
                                AUDIO_BANK2_REG_I2S_TDM_CFG_00_REG_CODEC_I2S_RX_ENC_WDTH_MASK,
                                (nRxSel << AUDIO_BANK2_REG_I2S_TDM_CFG_00_REG_CODEC_I2S_RX_ENC_WDTH_SHIFT));
            }
            HalBachWriteReg(E_BACH_REG_BANK2, AUDIO_BANK2_REG_I2S_TDM_CFG_05_ADDR,
                            AUDIO_BANK2_REG_I2S_TDM_CFG_05_REG_CODEC_I2S_TX_ENC_WDTH_MASK,
                            (nTxSel << AUDIO_BANK2_REG_I2S_TDM_CFG_05_REG_CODEC_I2S_TX_ENC_WDTH_SHIFT));
            break;
        default:
            goto FAIL;
            break;
    }

    g_aI2sCfg[eAioI2s].enI2sWidth = enI2sWidth;
    DBGMSG(AUDIO_DBG_LEVEL_I2S, "eAioI2s[%s] enI2sWidth[%d]", _HalAudI2sEnumToString(eAioI2s), enI2sWidth);

    return AIO_OK;
FAIL:
    ERRMSG("eAioI2s[%d] enI2sWidth[%d] Fail !", eAioI2s, enI2sWidth);
    return AIO_NG;
}

int HalAudI2sSetChannel(CHIP_AIO_I2S_e eAioI2s, U16 nChannel)
{
    U16 nCh = 0;

    switch (nChannel)
    {
        case E_AUD_CHANNEL_NUM_1:
            nCh = 4;
            break;
        case E_AUD_CHANNEL_NUM_2:
            nCh = 0;
            break;
        case E_AUD_CHANNEL_NUM_4:
            nCh = 1;
            break;
        case E_AUD_CHANNEL_NUM_8:
            nCh = 2;
            break;
        case E_AUD_CHANNEL_NUM_16:
            nCh = 3;
            break;
        default:
            ERRMSG("nChannel %d Fail !", nChannel);
            goto FAIL;
            break;
    }

    switch (eAioI2s)
    {
        case E_CHIP_AIO_I2S_RX_A:
            HalBachWriteReg(E_BACH_REG_BANK2, AUDIO_BANK2_REG_I2S_TDM_CFG_00_ADDR,
                            AUDIO_BANK2_REG_I2S_TDM_CFG_00_REG_CODEC_I2S_TDM_CHLEN_MASK,
                            (nCh << AUDIO_BANK2_REG_I2S_TDM_CFG_00_REG_CODEC_I2S_TDM_CHLEN_SHIFT));
            break;

        case E_CHIP_AIO_I2S_TX_A:
            // Support 4-wire mode I2S TXA running alone
            if (g_aI2sCfg[eAioI2s].eWireMode == E_AUD_I2S_WIRE_4)
            {
                HalBachWriteReg(E_BACH_REG_BANK2, AUDIO_BANK2_REG_I2S_TDM_CFG_00_ADDR,
                                AUDIO_BANK2_REG_I2S_TDM_CFG_00_REG_CODEC_I2S_TDM_CHLEN_MASK,
                                (nCh << AUDIO_BANK2_REG_I2S_TDM_CFG_00_REG_CODEC_I2S_TDM_CHLEN_SHIFT));
            }
            HalBachWriteReg(E_BACH_REG_BANK2, AUDIO_BANK2_REG_I2S_TDM_CFG_05_ADDR,
                            AUDIO_BANK2_REG_I2S_TDM_CFG_05_REG_CODEC_I2S_TDM_TX_CHLEN0_MASK,
                            ((nCh & 0x1) ? AUDIO_BANK2_REG_I2S_TDM_CFG_05_REG_CODEC_I2S_TDM_TX_CHLEN0_MASK : 0));
            HalBachWriteReg(E_BACH_REG_BANK2, AUDIO_BANK2_REG_I2S_TDM_CFG_05_ADDR,
                            AUDIO_BANK2_REG_I2S_TDM_CFG_05_REG_CODEC_I2S_TX_CHLEN_1_MASK,
                            (((nCh >> 1) & 0x1) ? AUDIO_BANK2_REG_I2S_TDM_CFG_05_REG_CODEC_I2S_TX_CHLEN_1_MASK : 0));
            HalBachWriteReg(E_BACH_REG_BANK2, AUDIO_BANK2_REG_I2S_TDM_CFG_05_ADDR,
                            AUDIO_BANK2_REG_I2S_TDM_CFG_05_REG_CODEC_I2S_TX_CHLEN2_MASK,
                            (((nCh >> 2) & 0x1) ? AUDIO_BANK2_REG_I2S_TDM_CFG_05_REG_CODEC_I2S_TX_CHLEN2_MASK : 0));
            break;

        default:
            goto FAIL;
            break;
    }

    g_aI2sCfg[eAioI2s].nChannelNum = nChannel;

    DBGMSG(AUDIO_DBG_LEVEL_I2S, "eAioI2s[%s] nChannel[%d]", _HalAudI2sEnumToString(eAioI2s), nChannel);
    return AIO_OK;
FAIL:

    ERRMSG("eAioI2s[%d] nChannel[%d] Fail !", eAioI2s, nChannel);
    return AIO_NG;
}

int HalAudI2sSetWireMode(CHIP_AIO_I2S_e eAioI2s, AudWireMode_e eWireMode)
{
    U16 mask;
    U16 addr;

    DBGMSG(AUDIO_DBG_LEVEL_I2S, "eAioI2s[%s] eWireMode[%d]", _HalAudI2sEnumToString(eAioI2s), eWireMode);
    g_aI2sCfg[eAioI2s].eWireMode = eWireMode;
    switch (eAioI2s)
    {
        case E_CHIP_AIO_I2S_TX_A:
            addr = AUDIO_BANK2_REG_I2S_TDM_CFG_05_ADDR;
            mask = AUDIO_BANK2_REG_I2S_TDM_CFG_05_REG_CODEC_I2S_TX_4WIRE_MODE_MASK;
            break;
        default:
            g_aI2sCfg[eAioI2s].eWireMode = E_AUD_I2S_WIRE_NULL;
            return AIO_OK;
    }
    switch (eWireMode)
    {
        case E_AUD_I2S_WIRE_4:
            HalBachWriteReg(E_BACH_REG_BANK2, addr, mask, mask);
            break;

        case E_AUD_I2S_WIRE_6:
            HalBachWriteReg(E_BACH_REG_BANK2, addr, mask, 0);
            break;

        default:
            goto FAIL;
            break;
    }

    return AIO_OK;
FAIL:
    ERRMSG("eAioI2s[%s] eWireMode[%d] Fail!", _HalAudI2sEnumToString(eAioI2s), eWireMode);
    return AIO_NG;
}

int HalAudI2sGetChannel(CHIP_AIO_I2S_e eAioI2s)
{
    return g_aI2sCfg[eAioI2s].nChannelNum;
}

int HalAudI2sSetTdmSlotConfig(CHIP_AIO_I2S_e eAioI2s, U16 nSlotMsk, AudTdmChnMap_e eMap)
{
    switch (eAioI2s)
    {
        case E_CHIP_AIO_I2S_RX_A:
            break;

        case E_CHIP_AIO_I2S_TX_A:
            HalBachWriteReg(E_BACH_REG_BANK2, AUDIO_BANK2_REG_I2S_TDM_CFG_06_ADDR,
                            AUDIO_BANK2_REG_I2S_TDM_CFG_06_REG_CODEC_I2S_TX_ACT_SLOT_MASK,
                            (nSlotMsk << AUDIO_BANK2_REG_I2S_TDM_CFG_06_REG_CODEC_I2S_TX_ACT_SLOT_SHIFT));
            HalBachWriteReg(E_BACH_REG_BANK2, AUDIO_BANK2_REG_I2S_TDM_CFG_06_ADDR,
                            AUDIO_BANK2_REG_I2S_TDM_CFG_02_REG_CODEC_I2S_TX_CH_SWAP_MASK,
                            (eMap << AUDIO_BANK2_REG_I2S_TDM_CFG_02_REG_CODEC_I2S_TX_CH_SWAP_SHIFT));
            break;

        default:
            ERRMSG("eAioI2s %d Fail !", eAioI2s);
            goto FAIL;
            break;
    }

    return AIO_OK;

FAIL:

    return AIO_NG;
}

int HalAudI2sEnableMck(CHIP_AIO_I2S_e eAioI2s, BOOL bEn)
{
    U16 enTimeGen = AUDIO_BANK4_REG_NF_SYNTH_MCK_00_REG_ENABLE_CLK_NF_SYNTH_REF_MASK
                    | AUDIO_BANK4_REG_NF_SYNTH_MCK_00_REG_CODEC_MCK_EN_TIME_GEN_MASK
                    | AUDIO_BANK4_REG_NF_SYNTH_MCK_00_REG_CODEC_MCK_NF_SYNTH_TRIG_MASK;
    AudReg_t nMck, nClk;

    if (((bEn == TRUE) && (g_aI2sCfg[eAioI2s].eMck == E_AUD_I2S_MCK_NULL)) || eAioI2s > E_CHIP_AIO_I2S_TX_END)
    {
        // do nothing
        DBGMSG(AUDIO_DBG_LEVEL_I2S, "do nothing");
        return AIO_OK;
    }

    g_aI2sCfg[eAioI2s].bMckActive = TRUE;

    switch (eAioI2s)
    {
        case E_CHIP_AIO_I2S_TX_A:
        case E_CHIP_AIO_I2S_RX_A:
            nMck.addr = AUDIO_BANK4_REG_NF_SYNTH_MCK_00_ADDR;
            nMck.mask = enTimeGen;
            nClk.addr = AUDIO_BANK2_REG_I2S_CFG_00_ADDR;
            nClk.mask = AUDIO_BANK2_REG_I2S_CFG_00_REG_ENABLE_CLK_CODEC_I2S_MCK_MASK;
            break;
        default:
            break;
    }

    if (bEn)
    {
        // enable clk for mck
        HalBachWriteReg(E_BACH_REG_BANK2, nClk.addr, nClk.mask, nClk.mask);

        // enable clk ref and trigger
        HalBachWriteReg(E_BACH_REG_BANK4, nMck.addr, nMck.mask, nMck.mask);
    }
    else
    {
        g_aI2sCfg[eAioI2s].bMckActive = FALSE;

        switch (eAioI2s)
        {
            case E_CHIP_AIO_I2S_RX_A:
            case E_CHIP_AIO_I2S_TX_A:
                if ((g_aI2sCfg[E_CHIP_AIO_I2S_RX_A].bMckActive == FALSE)
                    && (g_aI2sCfg[E_CHIP_AIO_I2S_TX_A].bMckActive == FALSE))
                {
                    break;
                }
                return AIO_OK;
            default:
                return AIO_OK;
        }
        HalBachWriteReg(E_BACH_REG_BANK4, nMck.addr, nMck.mask, 0);
        HalBachWriteReg(E_BACH_REG_BANK2, nClk.addr, nClk.addr, 0);
    }

    return AIO_OK;
}

static int _HalAudI2sSetMck(CHIP_AIO_I2S_e eAioI2s, AudI2sMck_e eMck)
{
    U16 addr;

    if (eMck < E_AUD_I2S_MCK_NULL || eMck > E_AUD_I2S_MCK_48M)
    {
        goto FAIL;
    }

    switch (eAioI2s)
    {
        case E_CHIP_AIO_I2S_TX_A:
        case E_CHIP_AIO_I2S_RX_A:
            addr = AUDIO_BANK2_REG_I2S_CFG_00_ADDR;
            break;
        default:
            goto FAIL;
    }

    HalBachWriteReg(E_BACH_REG_BANK2, addr, AUDIO_BANK2_REG_I2S_CFG_00_REG_SEL_CLK_CODEC_I2S_MCK_MASK,
                    2 << AUDIO_BANK2_REG_I2S_CFG_00_REG_SEL_CLK_CODEC_I2S_MCK_SHIFT);
    DBGMSG(AUDIO_DBG_LEVEL_I2S, "eAioI2s[%s] eMck[%d]", _HalAudI2sEnumToString(eAioI2s), eMck);

    g_aI2sCfg[eAioI2s].eMck = eMck;

    return AIO_OK;
FAIL:

    ERRMSG("eAioI2s[%s] eMck[%d] Fail !", _HalAudI2sEnumToString(eAioI2s), eMck);
    return AIO_NG;
}

static int _HalAudI2sEnableClkGate(CHIP_AIO_I2S_e eAioI2s, BOOL bEn)
{
    if (bEn)
    {
        switch (eAioI2s)
        {
            case E_CHIP_AIO_I2S_RX_A:
                HalBachWriteReg(E_BACH_REG_BANK2, AUDIO_BANK2_REG_I2S_CFG_00_ADDR,
                                AUDIO_BANK2_REG_I2S_CFG_00_REG_ENABLE_CLK_CODEC_I2S_RX_BCK_MASK,
                                AUDIO_BANK2_REG_I2S_CFG_00_REG_ENABLE_CLK_CODEC_I2S_RX_BCK_MASK);
                break;

            case E_CHIP_AIO_I2S_TX_A:
                HalBachWriteReg(E_BACH_REG_BANK2, AUDIO_BANK2_REG_I2S_CFG_00_ADDR,
                                AUDIO_BANK2_REG_I2S_CFG_00_REG_ENABLE_CLK_CODEC_I2S_TX_BCK_MASK,
                                AUDIO_BANK2_REG_I2S_CFG_00_REG_ENABLE_CLK_CODEC_I2S_TX_BCK_MASK);
                break;

            default:
                goto FAIL;
                break;
        }
    }
    else
    {
        switch (eAioI2s)
        {
            case E_CHIP_AIO_I2S_RX_A:
                HalBachWriteReg(E_BACH_REG_BANK2, AUDIO_BANK2_REG_I2S_CFG_00_ADDR,
                                AUDIO_BANK2_REG_I2S_CFG_00_REG_ENABLE_CLK_CODEC_I2S_RX_BCK_MASK, 0);
                break;

            case E_CHIP_AIO_I2S_TX_A:
                HalBachWriteReg(E_BACH_REG_BANK2, AUDIO_BANK2_REG_I2S_CFG_00_ADDR,
                                AUDIO_BANK2_REG_I2S_CFG_00_REG_ENABLE_CLK_CODEC_I2S_TX_BCK_MASK, 0);
                break;

            default:
                goto FAIL;
                break;
        }
    }
    DBGMSG(AUDIO_DBG_LEVEL_I2S, "eAioI2s[%s] bEn[%d]", _HalAudI2sEnumToString(eAioI2s), bEn);

    return AIO_OK;
FAIL:
    ERRMSG("eAioI2s[%s] bEn[%d] FAIL!", _HalAudI2sEnumToString(eAioI2s), bEn);
    return AIO_NG;
}

int HalAudI2sEnableBck(CHIP_AIO_I2S_e eAioI2s, BOOL bEn)
{
    U16 enTimeGen = AUDIO_BANK4_REG_NF_SYNTH_RX_BCK_00_REG_ENABLE_CLK_NF_SYNTH_RX_REF_MASK
                    | AUDIO_BANK4_REG_NF_SYNTH_RX_BCK_00_REG_CODEC_RX_BCK_EN_TIME_GEN_MASK;

    if (bEn)
    {
        switch (eAioI2s)
        {
            case E_CHIP_AIO_I2S_RX_A:
                HalBachWriteReg(E_BACH_REG_BANK4, AUDIO_BANK4_REG_NF_SYNTH_RX_BCK_00_ADDR, enTimeGen, enTimeGen);
                HalBachWriteReg(E_BACH_REG_BANK4, AUDIO_BANK4_REG_NF_SYNTH_RX_BCK_00_ADDR,
                                enTimeGen | AUDIO_BANK4_REG_NF_SYNTH_RX_BCK_00_REG_CODEC_RX_BCK_NF_SYNTH_TRIG_MASK,
                                enTimeGen | AUDIO_BANK4_REG_NF_SYNTH_RX_BCK_00_REG_CODEC_RX_BCK_NF_SYNTH_TRIG_MASK);
                HalBachWriteReg(E_BACH_REG_BANK4, AUDIO_BANK4_REG_NF_SYNTH_RX_BCK_00_ADDR,
                                enTimeGen | AUDIO_BANK4_REG_NF_SYNTH_RX_BCK_00_REG_CODEC_RX_BCK_NF_SYNTH_TRIG_MASK,
                                enTimeGen);
                break;

            case E_CHIP_AIO_I2S_TX_A:
                if (g_aI2sCfg[eAioI2s].eWireMode == E_AUD_I2S_WIRE_6)
                {
                    HalBachWriteReg(E_BACH_REG_BANK4, AUDIO_BANK4_REG_NF_SYNTH_TX_BCK_00_ADDR, enTimeGen, enTimeGen);
                    HalBachWriteReg(E_BACH_REG_BANK4, AUDIO_BANK4_REG_NF_SYNTH_TX_BCK_00_ADDR,
                                    enTimeGen | AUDIO_BANK4_REG_NF_SYNTH_TX_BCK_00_REG_CODEC_TX_BCK_NF_SYNTH_TRIG_MASK,
                                    enTimeGen | AUDIO_BANK4_REG_NF_SYNTH_TX_BCK_00_REG_CODEC_TX_BCK_NF_SYNTH_TRIG_MASK);
                    HalBachWriteReg(E_BACH_REG_BANK4, AUDIO_BANK4_REG_NF_SYNTH_TX_BCK_00_ADDR,
                                    enTimeGen | AUDIO_BANK4_REG_NF_SYNTH_TX_BCK_00_REG_CODEC_TX_BCK_NF_SYNTH_TRIG_MASK,
                                    enTimeGen);
                }
                else
                {
                    HalAudI2sEnableBck(E_CHIP_AIO_I2S_RX_A, bEn);
                }
                break;

            default:
                goto FAIL;
                break;
        }
    }
    else
    {
        BOOL canDisable = _HalAudI2sCanDisable(eAioI2s);
        switch (eAioI2s)
        {
            case E_CHIP_AIO_I2S_RX_A:
                if (canDisable)
                {
                    HalBachWriteReg(E_BACH_REG_BANK4, AUDIO_BANK4_REG_NF_SYNTH_RX_BCK_00_ADDR, enTimeGen, 0);
                }
                break;
            case E_CHIP_AIO_I2S_TX_A:
                if (canDisable)
                {
                    HalBachWriteReg(E_BACH_REG_BANK4, AUDIO_BANK4_REG_NF_SYNTH_RX_BCK_00_ADDR, enTimeGen, 0);
                }
                HalBachWriteReg(E_BACH_REG_BANK4, AUDIO_BANK4_REG_NF_SYNTH_TX_BCK_00_ADDR, enTimeGen, 0);
                break;
            default:
                goto FAIL;
                break;
        }
    }
    DBGMSG(AUDIO_DBG_LEVEL_I2S, "eAioI2s[%s] bEn[%d]", _HalAudI2sEnumToString(eAioI2s), bEn);

    return AIO_OK;
FAIL:
    ERRMSG("eAioI2s[%s] bEn[%d] FAIL!", _HalAudI2sEnumToString(eAioI2s), bEn);
    return AIO_NG;
}

int HalAudI2sEnable(CHIP_AIO_I2S_e eAioI2s, BOOL bEn)
{
    int ret  = AIO_OK;
    int line = 0;

    if ((unsigned int)eAioI2s >= E_CHIP_AIO_I2S_TOTAL)
    {
        ERRMSG("eAioI2s >= E_CHIP_AIO_I2S_TOTAL\n");
        return AIO_NG;
    }

    ret |= HalAudI2sEnableMck(eAioI2s, bEn);
    if (ret != AIO_OK)
    {
        line = __LINE__;
        goto FAIL;
    }

    ret |= _HalAudI2sEnableClkGate(eAioI2s, bEn);

    if (g_aI2sCfg[eAioI2s].eMsMode == E_AUD_I2S_MSMODE_MASTER)
    {
        ret |= HalAudI2sEnableBck(eAioI2s, bEn);
        g_aI2sCfg[eAioI2s].bBckActive = bEn;
    }

    // Set slave mode
    if (!bEn && _HalAudI2sCanDisable(eAioI2s))
    {
        // HalAudI2sSetMsMode(eAioI2s, E_AUD_I2S_MSMODE_SLAVE);
    }

    if (ret != AIO_OK)
    {
        line = __LINE__;
        goto FAIL;
    }
    g_aI2sCfg[eAioI2s].bActive = bEn;

    DBGMSG(AUDIO_DBG_LEVEL_I2S, "eAioI2s[%s] eMsMode[%s] bEn[%d]", _HalAudI2sEnumToString(eAioI2s),
           g_aI2sCfg[eAioI2s].eMsMode == E_AUD_I2S_MSMODE_MASTER ? "master" : "slave", bEn);

    return AIO_OK;
FAIL:
    ERRMSG("LINE:%d eAioI2s[%s] eMsMode[%s] bEn[%d] FAIL!", line, _HalAudI2sEnumToString(eAioI2s),
           g_aI2sCfg[eAioI2s].eMsMode == E_AUD_I2S_MSMODE_MASTER ? "master" : "slave", bEn);
    return AIO_NG;
}

BOOL HalAudI2sGetEnable(CHIP_AIO_I2S_e eAioI2s)
{
    return g_aI2sCfg[eAioI2s].bActive;
}

// To-Do : MISC I2s slave mode with MCLK
int HalAudI2sConfig(CHIP_AIO_I2S_e eAioI2s, AudI2sCfg_t *ptI2sCfg)
{
    int ret = AIO_OK;

    // update i2s cfg
    ret |= HalAudI2sUpdateCfg(eAioI2s, ptI2sCfg);
    if (((ptI2sCfg->nChannelNum % 2) != 0) && ptI2sCfg->nChannelNum != 1)
    {
        goto FAIL;
    }

    if (ptI2sCfg->nChannelNum != 2)
    {
        ptI2sCfg->enI2sMode = E_AUD_I2S_MODE_TDM;
    }
    else
    {
        ptI2sCfg->enI2sMode = E_AUD_I2S_MODE_I2S;
    }
    //
    ret |= HalAudI2sSetWireMode(eAioI2s, ptI2sCfg->eWireMode);
    ret |= HalAudI2sSetFmt(eAioI2s, ptI2sCfg->eFormat);
    ret |= HalAudI2sSetTdmMode(eAioI2s, ptI2sCfg->enI2sMode);
    ret |= HalAudI2sSetMsMode(eAioI2s, ptI2sCfg->eMsMode);
    ret |= HalAudI2sSetWidth(eAioI2s, ptI2sCfg->enI2sWidth);
    ret |= HalAudI2sSetChannel(eAioI2s, ptI2sCfg->nChannelNum);
    ret |= HalAudI2sSetTdmDetails(eAioI2s);

    // refer from Tiramisu old framework default set to E_AUD_I2S_CLK_REF_I2S_TDM_RX is necessary to config frome dts
    ret |= HalAudI2sSetClkRef(eAioI2s, E_AUD_I2S_CLK_REF_I2S_TDM_RX);

    // set BACH PLL clock depend on BCK/MCK
    ret |= _HalAudI2sSetBachPLL(eAioI2s, ptI2sCfg->eRate, ptI2sCfg->enI2sWidth, ptI2sCfg->nChannelNum);

    // set MCK div  and enable MCK
    ret |= HalAudI2sSetMckDivAndEnable(eAioI2s);

    // Set I2S Rate, must be after HalAudI2sSetWidth & HalAudI2sSetChannel, if you need to set bck, you need width and
    // channel info.

    if (ptI2sCfg->eMsMode == E_AUD_I2S_MSMODE_MASTER)
    {
        ret |= HalAudI2sSetRate(eAioI2s, ptI2sCfg->eRate);
        ret |= HalAudI2sEnableBck(eAioI2s, TRUE);
        g_aI2sCfg[eAioI2s].bBckActive = TRUE;
    }

    if (ret != AIO_OK)
    {
        goto FAIL;
    }

    return AIO_OK;
FAIL:
    ERRMSG("eAioI2s[%s] nChannelNum[%d] eMsMode[%s] FAIL!", _HalAudI2sEnumToString(eAioI2s), ptI2sCfg->nChannelNum,
           g_aI2sCfg[eAioI2s].eMsMode == E_AUD_I2S_MSMODE_MASTER ? "master" : "slave");
    return AIO_NG;
}

void HalAudI2sSetDriving(U8 level)
{
#ifdef __KERNEL__
    int puse_table[] = {
        MDRV_PUSE_I2S0_TX_WCK, MDRV_PUSE_I2S0_TX_SDO, MDRV_PUSE_I2S0_TX_BCK,
        MDRV_PUSE_I2S0_RX_BCK, MDRV_PUSE_I2S0_RX_WCK, MDRV_PUSE_I2S0_RX_SDI,
    };
    int i = 0;

    for (i = 0; i < AIO_ARRAY_SIZE(puse_table); i++)
    {
        int u8IndexGPIO = 0xFFFF;
        // I2S TXA

#ifdef CONFIG_SSTAR_PADMUX
        u8IndexGPIO = drv_padmux_getpad(puse_table[i]);
#endif

        if (u8IndexGPIO != 0XFFFF)
        {
#if SSTAR_FOR_FPGA_TEST
            ERRMSG("FPGA not support gpio config!");
#else
            sstar_gpio_drv_set(u8IndexGPIO, level);
#endif
        }
    }

#endif
    HalAudApiSetDtsValue(I2S_DRIVING, level);
}

int HalAudI2sUpdateCfg(CHIP_AIO_I2S_e eAioI2s, AudI2sCfg_t *ptI2sCfg)
{
    AudI2sCfg_t i2scfg = {.enI2sMode   = E_AUD_I2S_MODE_I2S,
                          .eMsMode     = E_AUD_I2S_MSMODE_MASTER,
                          .eFormat     = E_AUD_I2S_FMT_I2S,
                          .enI2sWidth  = E_AUD_BITWIDTH_16,
                          .eMck        = E_AUD_I2S_MCK_NULL,
                          .nChannelNum = 2,
                          .eRate       = E_AUD_RATE_48K,
                          .eWireMode   = E_AUD_I2S_WIRE_6};

    switch (eAioI2s)
    {
        case E_CHIP_AIO_I2S_TX_A:
            i2scfg.eMsMode     = HalAudApiGetDtsValue(I2SA_TX0_TDM_MODE);
            i2scfg.eWireMode   = HalAudApiGetDtsValue(I2SA_TX0_TDM_WIREMODE);
            i2scfg.eFormat     = HalAudApiGetDtsValue(I2SA_TX0_TDM_FMT);
            i2scfg.nChannelNum = HalAudApiGetDtsValue(I2SA_TX0_CHANNEL);
            break;
        case E_CHIP_AIO_I2S_RX_A:
            i2scfg.eMsMode     = HalAudApiGetDtsValue(I2SA_RX0_TDM_MODE);
            i2scfg.eWireMode   = HalAudApiGetDtsValue(I2SA_RX0_TDM_WIREMODE);
            i2scfg.eFormat     = HalAudApiGetDtsValue(I2SA_RX0_TDM_FMT);
            i2scfg.nChannelNum = HalAudApiGetDtsValue(I2SA_RX0_CHANNEL);
            break;
        default:
            break;
    }

    DBGMSG(AUDIO_DBG_LEVEL_I2S,
           "Before update eAioI2s[%s] eMsMode[%d] eWireMode[%d] eFormat[%d] nChannelNum[%d] nBitWidth[%d]",
           _HalAudI2sEnumToString(eAioI2s), ptI2sCfg->eMsMode, ptI2sCfg->eWireMode, ptI2sCfg->eFormat,
           ptI2sCfg->nChannelNum, ptI2sCfg->enI2sWidth);

    if (!ptI2sCfg->eMsMode)
    {
        ptI2sCfg->eMsMode = i2scfg.eMsMode;
    }

    if (!ptI2sCfg->eWireMode)
    {
        ptI2sCfg->eWireMode = i2scfg.eWireMode;
    }

    if (!ptI2sCfg->eFormat)
    {
        ptI2sCfg->eFormat = i2scfg.eFormat;
    }

    if (!ptI2sCfg->nChannelNum)
    {
        ptI2sCfg->nChannelNum = i2scfg.nChannelNum;
    }

    DBGMSG(AUDIO_DBG_LEVEL_I2S,
           "After update eAioI2s[%s] eMsMode[%d] eWireMode[%d] eFormat[%d] nChannelNum[%d] enI2sWidth[%d]",
           _HalAudI2sEnumToString(eAioI2s), ptI2sCfg->eMsMode, ptI2sCfg->eWireMode, ptI2sCfg->eFormat,
           ptI2sCfg->nChannelNum, ptI2sCfg->enI2sWidth);

    return AIO_OK;
}

int HalAudI2sTxDataMute(AUDIO_TDM_e nTdm, BOOL bEnable)
{
    switch (nTdm)
    {
        case E_AUDIO_TDM_TXA:
            if (bEnable)
            {
                HalBachWriteReg(E_BACH_REG_BANK2, AUDIO_BANK2_REG_I2S_TDM_CFG_00_ADDR,
                                AUDIO_BANK2_REG_I2S_TDM_CFG_00_REG_CODEC_I2S_TDM_CHLEN_MASK,
                                5 << AUDIO_BANK2_REG_I2S_TDM_CFG_00_REG_CODEC_I2S_TDM_CHLEN_SHIFT);
            }
            break;

        case E_AUDIO_TDM_TXB:
            if (bEnable)
            {
                // HalBachWriteReg(E_BACH_REG_BANK4, E_BACH_I2S_TDM_CFG01, BACH_I2S_TDM_TX_SEL_MSK,
                //                 5 << BACH_I2S_TDM_TX_SEL_POS);
            }
            break;

        case E_AUDIO_TDM_TXC:
            if (bEnable)
            {
                // HalBachWriteReg(E_BACH_REG_BANK4, E_BACH_I2S_TDM_CFG01, BACH_I2S_TDM_TX_SEL_MSK,
                //                 5 << BACH_I2S_TDM_TX_SEL_POS);
            }
            break;

        default:
            ERRMSG("nTdm [%d] Fail !", nTdm);
            break;
    }

    return 0;
}

int HalAudI2sSetMck(MHAL_MCLK_ID_e eMclkID, AudI2sMck_e eMck, BOOL bEn)
{
    int ret = 0;

    if (eMclkID == E_MHAL_MCLK_ID_0)
    {
        ret |= _HalAudI2sSetMck(E_CHIP_AIO_I2S_RX_A, eMck);
        ret |= _HalAudI2sSetMck(E_CHIP_AIO_I2S_TX_A, eMck);
    }

    if (ret != AIO_OK)
    {
        goto FAIL;
    }
#if 0
    ret |= HalAudI2sEnableMck(eAioI2s, bEn);
    if (ret != AIO_OK)
    {
        goto FAIL;
    }
#endif

    return AIO_OK;
FAIL:
    return AIO_NG;
}
