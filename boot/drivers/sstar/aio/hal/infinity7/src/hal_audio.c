/*
 * hal_audio.c - Sigmastar
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
#include "hal_audio_config.h"
#include "hal_audio_reg.h"
#include "hal_audio_types.h"
#include "hal_audio_api.h"
#include "hal_audio_os_api.h"
#include <stdbool.h>

//------------------------------------------------------------------------------
#define REF_FREQ                 384 // 384MHz
#define I2S_TDM_RX_PADMUX_OFFSET 4
#define I2S_TDM_TX_PADMUX_OFFSET 8
#define I2S_MODE_PADMUX_OFFSET   12
#define I2S_MCK_PADMUX_OFFSET    0
#define DIGMIC_PADMUX_OFFSET     0

//------------------------------------------------------------------------------
static void _UDelay(U32 u32MicroSeconds)
{
    udelay(u32MicroSeconds);
}

static void _Msleep(U32 u32MilliSeconds)
{
    udelay(u32MilliSeconds * 1000);
}

//------------------------------------------------------------------------------
static BOOL        g_bAdcActive;
static BOOL        g_bDacActive;
static BOOL        g_bAtopStatus[E_CHIP_AIO_ATOP_TOTAL];
static AudI2sCfg_t g_aI2sCfg[E_CHIP_AIO_I2S_TOTAL];

static BOOL g_bI2MckActive[E_CHIP_AIO_I2S_TOTAL];

static U8 g_nDmicBckMode8K  = 7;
static U8 g_nDmicBckMode16K = 12;
static U8 g_nDmicBckMode32K = 15;
static U8 g_nDmicBckMode48K = 16;

static U8 g_nI2sRxTdmWsPgm   = FALSE;
static U8 g_nI2sRxTdmWsWidth = 0;
static U8 g_nI2sRxTdmWsInv   = 0;
static U8 g_nI2sRxTdmChSwap  = 0; //[0]: 0<->2, 1<->3, 4<->6, 5<->7 [1]: 0<->4, 1<->5, 2<->6, 3<->7

static U8 g_nI2sTxTdmWsPgm   = FALSE;
static U8 g_nI2sTxTdmWsWidth = 0;
static U8 g_nI2sTxTdmWsInv   = 0;
static U8 g_nI2sTxTdmChSwap  = 0; //[0]: 0<->2, 1<->3, 4<->6, 5<->7 [1]: 0<->4, 1<->5, 2<->6, 3<->7

static long g_nI2sTdmChSwap[E_AUDIO_TDM_TOTAL]  = {0}; //[0]: 0<->2, 1<->3, 4<->6, 5<->7 [1]: 0<->4, 1<->5, 2<->6, 3<->7
static long g_nI2sTdmWsPgm[E_AUDIO_TDM_TOTAL]   = {0};
static long g_nI2sTdmWsInv[E_AUDIO_TDM_TOTAL]   = {0};
static long g_nI2sTdmWsWidth[E_AUDIO_TDM_TOTAL] = {0};
static long g_nI2sTdmBckInv[E_AUDIO_TDM_TOTAL]  = {0};
static long g_nI2sRxMode                        = 0;

static U8 g_nAdc1HpfLevel = 0xf;
static U8 g_nDmicHpfLevel = 0x0;

static U8 g_nI2sTxTdmActiveSlot = 0;

static AudRate_e g_eDmicSmpRat[E_CHIP_AI_DMIC_TOTAL]  = {E_AUD_RATE_NULL};
static S8        g_nDmicGain[E_CHIP_AI_DMIC_TOTAL][4] = {0};
static S8        g_nDpgaGain[E_CHIP_DPGA_TOTAL][2]    = {0};

//------------------------------------------------------------------------------
static int  _HalAudSysInit(void);
static int  _HalAudAtopInit(void);
static int  _HalAudDmaFirstInit(void);
static int  _HalAudDmaWrMchInit(void);
static int  _HalAudDmicRegInit(void);
static int  _HalAudDpgaInit(void);
static int  _HalAudIntEnable(void);
static int  _HalAudAtopAdc(CHIP_AI_ADC_e eAdc, BOOL bEnable);
static int  _HalAudAtopDac(CHIP_AO_DAC_e eDac, BOOL bEnable);
static int  _HalAudAtopEnableRef(BOOL bEnable);
static int  _HalAudAtopEnablePreamp(CHIP_AI_ADC_e eAdc, BOOL bEnable);
static int  _HalAudAtopSwitch(CHIP_AIO_ATOP_e eAtop, BOOL bEnable);
static int  _HalAudDmaReInit(CHIP_AIO_DMA_e eAioDma);
static int  _HalAudDmaRdIntEnable(CHIP_AIO_DMA_e eAioDma, BOOL bUnderrun, BOOL bEmpty);
static int  _HalAudDmaWrIntEnable(CHIP_AIO_DMA_e eAioDma, BOOL bOverrun, BOOL bFull);
static int  _HalAudDmaGetRdInt(CHIP_AIO_DMA_e eAioDma, BOOL *bUnderrun, BOOL *bEmpty);
static int  _HalAudDmaGetWrInt(CHIP_AIO_DMA_e eAioDma, BOOL *bOverrun, BOOL *bFull);
static int  _HalAudDmaRdGetFlags(CHIP_AIO_DMA_e eAioDma, BOOL *pbUnderrun, BOOL *pbEmtpy, BOOL *pbLocalEmpty);
static int  _HalAudDmaWrGetFlags(CHIP_AIO_DMA_e eAioDma, BOOL *pbOverrun, BOOL *pbFull, BOOL *pbLocalFull);
static U32  _HalAudDmaGetRawMiuUnitLevelCnt(CHIP_AIO_DMA_e eAioDma);
static int  _HalAudDmaGetMchSelConfigValue(AudMchSel_e eMchSel, U16 *nChSel, EN_CHIP_AI_MCH eMchSet);
static U32  _HalAudBckCalculate(AudRate_e eRate, AudBitWidth_e enI2sWidth, U16 Chn);
static int  _HalAudI2sSetBck(CHIP_AIO_I2S_e eAioI2s, U32 nNfValue);
static int  _HalAudMckCalculate(CHIP_AIO_I2S_e eAioI2s, AudI2sMck_e eMck);
static int  _HalAudDmaSetMiuSel(CHIP_AIO_DMA_e eAioDma, U32 nBufAddrOffset);
static int  _HalAudDpgaCalGain(S8 s8Gain, U8 *pU8GainIdx);
static int  _HalAudSetAiPathCoreDmaA(U8 nDmaCh, AI_CH_e eAiCh);
static int  _HalAudSetAiPathCoreDmaB(U8 nDmaCh, AI_CH_e eAiCh);
static int  _HalAudSetAoPathCoreDmaA(U8 nDmaCh, AO_CH_e eAoCh);
static int  _HalAudSetAoPathCoreDmaB(U8 nDmaCh, AO_CH_e eAoCh);
static int  _HalAudDmaWrConfigMchCore(EN_CHIP_AI_MCH eMchSet, U16 nChNum, AudMchClkRef_e eMchClkRef,
                                      AudMchSel_e *mch_sel);
static BOOL _HalAudDacSetMute(AO_CH_e chn, BOOL bEnable);

//------------------------------------------------------------------------------

static BOOL _HalAudSetHpf(AudHpfDev_e eHfpDev, U8 level)
{
    U16 bank, nAddr, nRegMsk, nPos;

    if (eHfpDev >= E_AUD_HPF_DEV_NUM)
    {
        ERRMSG("Function - %s # %d - hpfdev = %d, max = %d, error !] \n", __func__, __LINE__, eHfpDev,
               E_AUD_HPF_DEV_NUM - 1);
        return FALSE;
    }
    if (level >= 0x9)
    {
        level = 0xf;
    }
    switch (eHfpDev)
    {
        case E_AUD_HPF_ADC1_DMIC_2CH:
            bank    = E_BACH_REG_BANK1;
            nAddr   = E_BACH_MUX1_SEL;
            nRegMsk = REG_ADC_HPF_N_MSK;
            nPos    = REG_ADC_HPF_N_POS;
            break;
        case E_AUD_HPF_DMIC_4CH:
            bank    = E_BACH_REG_BANK2;
            nAddr   = E_BACH_VREC_HPF_CFG00;
            nRegMsk = REG_BP_VERC_HPF_N_MSK;
            nPos    = REG_BP_VERC_HPF_N_POS;
            break;
        default:
            ERRMSG("Function - %s # %d - hpfdev = %d, max = %d, error !] \n", __func__, __LINE__, eHfpDev,
                   E_AUD_HPF_DEV_NUM - 1);
            return FALSE;
    }
    HalBachWriteReg(bank, nAddr, nRegMsk, level << nPos);
    return TRUE;
}

static int _HalAudPllInit(void)
{
    HalBachWriteRegByte(0x00141d00, 0xff, 0xb0);
    HalBachWriteRegByte(0x00141d01, 0xff, 0x11);
    HalBachWriteRegByte(0x00141d42, 0xff, 0x2b);
    HalBachWriteRegByte(0x00141d43, 0xff, 0x02);
    HalBachWriteRegByte(0x00141d46, 0xff, 0x28);
    HalBachWriteRegByte(0x00141d47, 0xff, 0x00);
    HalBachWriteRegByte(0x00141d4E, 0xff, 0xbc);
    HalBachWriteRegByte(0x00141d4F, 0xff, 0x00);
    HalBachWriteRegByte(0x00141d40, 0xff, 0x00);
    HalBachWriteRegByte(0x00141d41, 0xff, 0x31);
    HalBachWriteRegByte(0x00141d4A, 0xff, 0x63);
    HalBachWriteRegByte(0x00141d4B, 0xff, 0x00);
    HalBachWriteRegByte(0x00141d4C, 0xff, 0x80);
    HalBachWriteRegByte(0x00141d4D, 0xff, 0x01);

    return AIO_OK;
}

static int _HalAudSysInit(void)
{
    _HalAudAtopInit();

    HalBachWriteRegByte(0x00150200, 0xff, 0x00);
    HalBachWriteRegByte(0x00150201, 0xff, 0x40);
    HalBachWriteRegByte(0x00150200, 0xff, 0xff);
    // HalBachWriteRegByte(0x00150201, 0xff, 0x8d);
    HalBachWriteRegByte(0x00150201, 0xff, 0x89);
    HalBachWriteRegByte(0x00150202, 0xff, 0x88);
    // HalBachWriteRegByte(0x00150203, 0xff, 0xfA); // Reset to 32K sample rate
    HalBachWriteRegByte(0x00150203, 0xff, 0xff);
    HalBachWriteRegByte(0x00150204, 0xff, 0x03);
    HalBachWriteRegByte(0x00150205, 0xff, 0x00);
    HalBachWriteRegByte(0x00150206, 0xff, 0xB4);
    HalBachWriteRegByte(0x00150207, 0xff, 0x19);
    HalBachWriteRegByte(0x00150208, 0xff, 0x00);
    HalBachWriteRegByte(0x00150209, 0xff, 0xf0);
    _HalAudSetHpf(E_AUD_HPF_ADC1_DMIC_2CH, g_nAdc1HpfLevel); ////ADC0/1 HPF level get by dts
    HalBachWriteRegByte(0x0015020a, 0xff, 0x00);
    HalBachWriteRegByte(0x0015020b, 0xff, 0x80);
    HalBachWriteRegByte(0x0015020c, 0xff, 0x9a);
    HalBachWriteRegByte(0x0015020d, 0xff, 0xc0);
    HalBachWriteRegByte(0x0015020e, 0xff, 0x5a);
    HalBachWriteRegByte(0x0015020f, 0xff, 0x55);
    // HalBachWriteRegByte(0x00150212, 0xff, 0x05);
    HalBachWriteRegByte(0x00150212, 0xff, 0x09);
    HalBachWriteRegByte(0x00150213, 0xff, 0x02);
    HalBachWriteRegByte(0x00150214, 0xff, 0x00);
    HalBachWriteRegByte(0x00150215, 0xff, 0x00);
    HalBachWriteRegByte(0x00150216, 0xff, 0x7d);
    HalBachWriteRegByte(0x00150217, 0xff, 0x00);
    HalBachWriteRegByte(0x0015023a, 0xff, 0x1d);
    HalBachWriteRegByte(0x0015023b, 0xff, 0x02);
    HalBachWriteRegByte(0x0015023a, 0xff, 0x00);
    HalBachWriteRegByte(0x0015023b, 0xff, 0x00);
    HalBachWriteRegByte(0x0015031c, 0xff, 0x03);
    HalBachWriteRegByte(0x0015031d, 0xff, 0x00);
    HalBachWriteRegByte(0x0015032c, 0xff, 0x03);
    HalBachWriteRegByte(0x0015032d, 0xff, 0xc0); // disable ICG function(hw bug, will fix in i6)
    HalBachWriteRegByte(0x0015031d, 0xff, 0x00);
    HalBachWriteRegByte(0x00150226, 0xff, 0x00);
    HalBachWriteRegByte(0x00150227, 0xff, 0xd4);

    // correct IC default value
    HalBachWriteRegByte(0x00150248, 0xff, 0x07);
    HalBachWriteRegByte(0x00150249, 0xff, 0x00);
    HalBachWriteRegByte(0x00150250, 0xff, 0x07);

    // I2S TDM settings
    HalBachWriteRegByte(0x00150378, 0xff, 0x1C);
    HalBachWriteRegByte(0x00150379, 0xff, 0xd1);
    HalBachWriteRegByte(0x00150578, 0xff, 0x1C);
    HalBachWriteRegByte(0x00150579, 0xff, 0xd1);

    // ADC2 HPF
    HalBachWriteRegByte(0x001502c8, 0xff, 0xFD);
    HalBachWriteRegByte(0x001502c9, 0xff, 0x09);
    HalBachWriteRegByte(0x001502cc, 0xff, 0x00);
    HalBachWriteRegByte(0x001502cd, 0xff, 0xf0);

    return AIO_OK;
}

static int _HalAudAtopInit(void)
{
    S32 i;
    HalBachWriteRegByte(0x00103400, 0xff, 0x00);
    // HalBachWriteRegByte(0x00103401, 0xff, 0x02);
    HalBachWriteRegByte(0x00103401, 0xff, 0x08); // enable MSP, speed up charge VREF
    HalBachWriteRegByte(0x00103404, 0xff, 0x80); // REG_EN_SW_TEST
#if 1
    HalBachWriteRegByte(0x00103414, 0xff, 0x00); // ADC setting
    HalBachWriteRegByte(0x00103415, 0xff, 0x11);
    HalBachWriteRegByte(0x00103416, 0xff, 0x18);
    HalBachWriteRegByte(0x00103417, 0xff, 0x18);
    HalBachWriteRegByte(0x00103418, 0xff, 0x10);
    HalBachWriteRegByte(0x00103419, 0xff, 0x10);
    HalBachWriteRegByte(0x0010341A, 0xff, 0x0A);
    HalBachWriteRegByte(0x0010341B, 0xff, 0x0A);
#endif
    HalBachWriteRegByte(0x00103400, 0xff, 0x10);
    HalBachWriteRegByte(0x00103401, 0xff, 0x00);
    HalBachWriteRegByte(0x0010340C, 0xff, 0x00);
    HalBachWriteRegByte(0x0010340D, 0xff, 0x00);
    HalBachWriteRegByte(0x0010340A, 0xff, 0x00);
    HalBachWriteRegByte(0x0010340B, 0xff, 0x00);
    HalBachWriteRegByte(0x00103406, 0xff, 0x41);
    HalBachWriteRegByte(0x00103407, 0xff, 0xE0);
    HalBachWriteRegByte(0x00103460, 0xff, 0x03);
    HalBachWriteRegByte(0x00103461, 0xff, 0x00);

#if 0
    //ADC2
    HalBachWriteRegByte(0x00103412, 0xff, 0x04);
    HalBachWriteRegByte(0x00103413, 0xff, 0x02);
    HalBachWriteRegByte(0x00103414, 0xff, 0x10);
    HalBachWriteRegByte(0x00103415, 0xff, 0x00);
    HalBachWriteRegByte(0x00103416, 0xff, 0x00);
    HalBachWriteRegByte(0x00103417, 0xff, 0x00);
    HalBachWriteRegByte(0x00103418, 0xff, 0x77);
    HalBachWriteRegByte(0x00103419, 0xff, 0x00);
    HalBachWriteRegByte(0x0010341A, 0xff, 0x00);
    HalBachWriteRegByte(0x0010341B, 0xff, 0x30);
#endif
    //
    for (i = 0; i < E_AUD_ATOP_NUM; i++)
    {
        _HalAudAtopSwitch((AudAtopPath_e)i, FALSE);
    }

    //
    for (i = 0; i < E_CHIP_AI_ADC_TOTAL; i++)
    {
        HalAudAtopSetAdcMux(i, E_CHIP_ADC_MUX_MICIN);
    }

    return AIO_OK;
}

static int _HalAudDmaFirstInit(void)
{
    // reset DMA1 interal register
    HalBachWriteReg(E_BACH_REG_BANK1, E_BACH_DMA1_CTRL_4, 0xFFFF, 0);  // reset DMA 1 read size
    HalBachWriteReg(E_BACH_REG_BANK1, E_BACH_DMA1_CTRL_12, 0xFFFF, 0); // reset DMA 1 write size

    HalBachWriteReg(E_BACH_REG_BANK1, E_BACH_DMA1_CTRL_0, REG_SW_RST_DMA, REG_SW_RST_DMA); // DMA 1 software reset
    HalBachWriteReg(E_BACH_REG_BANK1, E_BACH_DMA1_CTRL_0, REG_SW_RST_DMA, 0);

    HalBachWriteReg(E_BACH_REG_BANK1, E_BACH_DMA1_CTRL_0, (REG_PRIORITY_KEEP_HIGH | REG_RD_LEVEL_CNT_LIVE_MASK),
                    (REG_PRIORITY_KEEP_HIGH | REG_RD_LEVEL_CNT_LIVE_MASK));

    // reset DMA2 interal register
    HalBachWriteReg(E_BACH_REG_BANK1, E_BACH_DMA2_CTRL_4, 0xFFFF, 0);  // reset DMA 2 read size
    HalBachWriteReg(E_BACH_REG_BANK1, E_BACH_DMA2_CTRL_12, 0xFFFF, 0); // reset DMA 2 write size

    HalBachWriteReg(E_BACH_REG_BANK1, E_BACH_DMA2_CTRL_0, REG_SW_RST_DMA, REG_SW_RST_DMA); // DMA 2 software reset
    HalBachWriteReg(E_BACH_REG_BANK1, E_BACH_DMA2_CTRL_0, REG_SW_RST_DMA, 0);

    HalBachWriteReg(E_BACH_REG_BANK1, E_BACH_DMA2_CTRL_0, (REG_PRIORITY_KEEP_HIGH | REG_RD_LEVEL_CNT_LIVE_MASK),
                    (REG_PRIORITY_KEEP_HIGH | REG_RD_LEVEL_CNT_LIVE_MASK));

    // reset DMA3 interal register
    HalBachWriteReg(E_BACH_REG_BANK4, E_BACH_DMA3_CTRL_4, 0xFFFF, 0);  // reset DMA 3 read size
    HalBachWriteReg(E_BACH_REG_BANK4, E_BACH_DMA3_CTRL_12, 0xFFFF, 0); // reset DMA 3 write size

    HalBachWriteReg(E_BACH_REG_BANK4, E_BACH_DMA3_CTRL_0, REG_SW_RST_DMA, REG_SW_RST_DMA); // DMA 2 software reset
    HalBachWriteReg(E_BACH_REG_BANK4, E_BACH_DMA3_CTRL_0, REG_SW_RST_DMA, 0);

    HalBachWriteReg(E_BACH_REG_BANK4, E_BACH_DMA3_CTRL_0, (REG_PRIORITY_KEEP_HIGH | REG_RD_LEVEL_CNT_LIVE_MASK),
                    (REG_PRIORITY_KEEP_HIGH | REG_RD_LEVEL_CNT_LIVE_MASK));

    // reset DMA4 interal register
    // HalBachWriteReg(E_BACH_REG_BANK4, E_BACH_DMA4_CTRL_4, 0xFFFF, 0);                //reset DMA 3 read size
    HalBachWriteReg(E_BACH_REG_BANK4, E_BACH_DMA4_CTRL_02, 0xFFFF, 0); // reset DMA 3 write size

    HalBachWriteReg(E_BACH_REG_BANK4, E_BACH_DMA4_CTRL_0, REG_SW_RST_DMA, REG_SW_RST_DMA); // DMA 2 software reset
    HalBachWriteReg(E_BACH_REG_BANK4, E_BACH_DMA4_CTRL_0, REG_SW_RST_DMA, 0);

    HalBachWriteReg(E_BACH_REG_BANK4, E_BACH_DMA4_CTRL_0, (REG_PRIORITY_KEEP_HIGH | REG_RD_LEVEL_CNT_LIVE_MASK),
                    (REG_PRIORITY_KEEP_HIGH | REG_RD_LEVEL_CNT_LIVE_MASK));

    // reset DMA5 interal register
    // HalBachWriteReg(E_BACH_REG_BANK4, E_BACH_DMA5_CTRL_4, 0xFFFF, 0);                //reset DMA 3 read size
    HalBachWriteReg(E_BACH_REG_BANK4, E_BACH_DMA5_CTRL_12, 0xFFFF, 0); // reset DMA 3 write size

    HalBachWriteReg(E_BACH_REG_BANK4, E_BACH_DMA5_CTRL_0, REG_SW_RST_DMA, REG_SW_RST_DMA); // DMA 2 software reset
    HalBachWriteReg(E_BACH_REG_BANK4, E_BACH_DMA5_CTRL_0, REG_SW_RST_DMA, 0);

    HalBachWriteReg(E_BACH_REG_BANK4, E_BACH_DMA5_CTRL_0, (REG_PRIORITY_KEEP_HIGH | REG_RD_LEVEL_CNT_LIVE_MASK),
                    (REG_PRIORITY_KEEP_HIGH | REG_RD_LEVEL_CNT_LIVE_MASK));

    return AIO_OK;
}

static int _HalAudDmaWrMchInit(void)
{
    // E_CHIP_AI_MCH_A
    HalBachWriteReg(E_BACH_REG_BANK2, E_BACH_DMA_SRC_CFG00, BACH_DMA_MCH_DEBUG_MODE, 0);
    HalBachWriteReg(E_BACH_REG_BANK2, E_BACH_DMA_SRC_CFG00, BACH_DMA_WR_BIT_MODE, 0);
    HalBachWriteReg(E_BACH_REG_BANK2, E_BACH_DMA_SRC_CFG00, BACH_MCH_32B_EN, BACH_MCH_32B_EN);
    // E_CHIP_AI_MCH_B
    HalBachWriteReg(E_BACH_REG_BANK2, E_BACH_DMA_SRC_CFG01, BACH_DMA_MCH_DEBUG_MODE, 0);
    HalBachWriteReg(E_BACH_REG_BANK2, E_BACH_DMA_SRC_CFG01, BACH_DMA_WR_BIT_MODE, 0);
    HalBachWriteReg(E_BACH_REG_BANK2, E_BACH_DMA_SRC_CFG01, BACH_MCH_32B_EN, BACH_MCH_32B_EN);
    // E_CHIP_AI_MCH_C
    HalBachWriteReg(E_BACH_REG_BANK4, E_BACH_DMA3_SRC_CFG01, BACH_DMA_MCH_DEBUG_MODE, 0);
    HalBachWriteReg(E_BACH_REG_BANK4, E_BACH_DMA3_SRC_CFG01, BACH_DMA_WR_BIT_MODE, 0);
    HalBachWriteReg(E_BACH_REG_BANK4, E_BACH_DMA3_SRC_CFG01, BACH_MCH_32B_EN, BACH_MCH_32B_EN);
    // E_CHIP_AI_MCH_D
    HalBachWriteReg(E_BACH_REG_BANK4, E_BACH_DMA4_SRC_CFG01, BACH_DMA_MCH_DEBUG_MODE, 0);
    HalBachWriteReg(E_BACH_REG_BANK4, E_BACH_DMA4_SRC_CFG01, BACH_DMA_WR_BIT_MODE, 0);
    HalBachWriteReg(E_BACH_REG_BANK4, E_BACH_DMA4_SRC_CFG01, BACH_MCH_32B_EN, BACH_MCH_32B_EN);
    // CHIP_AI_MCH_E
    HalBachWriteReg(E_BACH_REG_BANK4, E_BACH_DMA5_SRC_CFG01, BACH_DMA_MCH_DEBUG_MODE, 0);
    HalBachWriteReg(E_BACH_REG_BANK4, E_BACH_DMA5_SRC_CFG01, BACH_DMA_WR_BIT_MODE, 0);
    HalBachWriteReg(E_BACH_REG_BANK4, E_BACH_DMA5_SRC_CFG01, BACH_MCH_32B_EN, BACH_MCH_32B_EN);

    return AIO_OK;
}

static int _HalAudDmicRegInit(void)
{
    // digital mic settings
    HalBachWriteRegByte(0x0015040c, 0xff, 0x00);
    HalBachWriteRegByte(0x0015040d, 0xff, 0x80);
    HalBachWriteRegByte(0x0015040c, 0xff, 0x00);
    HalBachWriteRegByte(0x0015040d, 0xff, 0x00);
    HalBachWriteRegByte(0x00150402, 0xff, 0x21);
    HalBachWriteRegByte(0x00150403, 0xff, 0x00);
    HalBachWriteRegByte(0x00150406, 0xff, 0x00);
    HalBachWriteRegByte(0x00150407, 0xff, 0x00);
    HalBachWriteRegByte(0x00150408, 0xff, 0x0a);
    HalBachWriteRegByte(0x00150409, 0xff, 0x00);
    HalBachWriteRegByte(0x0015040c, 0xff, 0x1f);
    HalBachWriteRegByte(0x0015040d, 0xff, 0x01);
    HalBachWriteRegByte(0x00150420, 0xff, 0x00);
    HalBachWriteRegByte(0x00150421, 0xff, 0x00);
    HalBachWriteRegByte(0x00150422, 0xff, 0x01);
    HalBachWriteRegByte(0x00150423, 0xff, 0x00);
    HalBachWriteRegByte(0x00150424, 0xff, 0x00);
    HalBachWriteRegByte(0x00150425, 0xff, 0x80);
    HalBachWriteRegByte(0x00150426, 0xff, 0x01);
    HalBachWriteRegByte(0x00150427, 0xff, 0x00);
    HalBachWriteRegByte(0x00150428, 0xff, 0x00);
    HalBachWriteRegByte(0x00150429, 0xff, 0x80);
    HalBachWriteRegByte(0x00150440, 0xff, 0x01);
    HalBachWriteRegByte(0x00150441, 0xff, 0x00);

    // enable HPF for DMIC
    HalBachWriteRegByte(0x0015039e, 0xff, 0x00);
    _HalAudSetHpf(E_AUD_HPF_DMIC_4CH, g_nDmicHpfLevel); // HPF Level get by dts
    HalBachWriteRegByte(0x0015039f, 0xff, 0x00);

    return AIO_OK;
}

static int _HalAudDpgaInit(void)
{
    U8 i;

    for (i = 0; i < E_CHIP_DPGA_TOTAL; i++)
    {
        HalAudDpgaCtrl((EN_CHIP_DPGA)i, TRUE, TRUE, TRUE);
    }

    return AIO_OK;
}

static int _HalAudIntEnable(void)
{
    // HalBachWriteReg(E_BACH_REG_BANK2, E_BACH_INT_EN, REG_DMA_INT_EN, REG_DMA_INT_EN);

    return AIO_OK;
}

// todo later
static int _HalAudAtopAdc(CHIP_AI_ADC_e eAdc, BOOL bEnable)
{
    U16 nMask;

    switch (eAdc)
    {
        case E_CHIP_AI_ADC_A:
        case E_CHIP_AI_ADC_B:
            nMask = (REG_PD_INMUX_L | REG_PD_INMUX_R | REG_PD_INT_L | REG_PD_INT_R);
            HalBachWriteReg(E_BACH_REG_BANK5, E_BACH_ANALOG_CTRL03, nMask, (bEnable ? 0 : nMask));

            nMask = (REG_PD_SAR_L | REG_PD_SAR_R);
            HalBachWriteReg(E_BACH_REG_BANK5, E_BACH_ANALOG_CTRL30, nMask, (bEnable ? 0 : nMask));

            nMask = (REG_EN_SAR_LOFIC_L | REG_EN_SAR_LOFIC_R);
            HalBachWriteReg(E_BACH_REG_BANK5, E_BACH_ANALOG_CTRL30, nMask, (bEnable ? nMask : 0));
            break;

        default:
            ERRMSG("Func:%s, Line:%d eAdc %d Fail !\n", __FUNCTION__, __LINE__, eAdc);
            goto FAIL;
            break;
    }

    return AIO_OK;

FAIL:

    return AIO_NG;
}

#if 1
static int _HalAudAtopDac(CHIP_AO_DAC_e eDac, BOOL bEnable)
{
    U16 nMask;

    switch (eDac)
    {
        case E_CHIP_AO_DAC_A:
            nMask = (REG_PD_BIAS_DAC | REG_PD_L0_DAC | REG_PD_R0_DAC | REG_PD_REF_DAC); // 1 4 9 10

            if (bEnable)
            {
                HalBachWriteReg(E_BACH_REG_BANK5, E_BACH_ANALOG_CTRL03, nMask, 0); //
                // HalBachWriteReg(E_BACH_REG_BANK5, E_BACH_ANALOG_CTRL00, REG_EN_CK_DAC_MSK, 0x1 >> REG_EN_CK_DAC_POS);
            }
            else
            {
                HalBachWriteReg(E_BACH_REG_BANK5, E_BACH_ANALOG_CTRL03, nMask, nMask);
                // HalBachWriteReg(E_BACH_REG_BANK5, E_BACH_ANALOG_CTRL00, REG_EN_CK_DAC_MSK, 0x0 >> REG_EN_CK_DAC_POS);
            }
            break;

        default:
            ERRMSG("Func:%s, Line:%d eDac %d Fail !\n", __FUNCTION__, __LINE__, eDac);
            goto FAIL;
            break;
    }

    return AIO_OK;

FAIL:

    return AIO_NG;
}
#endif

static int _HalAudAtopEnableRef(BOOL bEnable)
{
    U16 nMask;

    nMask = (REG_PD_IGEN | REG_PD_VREF);
    HalBachWriteReg(E_BACH_REG_BANK5, E_BACH_ANALOG_CTRL03, nMask, (bEnable ? 0 : nMask)); // 103406[11:12]

    return AIO_OK;
}

// todo later ADC 0,1 & 2,3
static int _HalAudAtopEnablePreamp(CHIP_AI_ADC_e eAdc, BOOL bEnable)
{
    switch (eAdc)
    {
        case E_CHIP_AI_ADC_A:
        case E_CHIP_AI_ADC_B:
            if (bEnable)
            {
                HalBachWriteReg(E_BACH_REG_BANK5, E_BACH_ANALOG_CTRL03, REG_PD_INT_L | REG_PD_INT_R, 0);
            }
            else
            {
                HalBachWriteReg(E_BACH_REG_BANK5, E_BACH_ANALOG_CTRL03, REG_PD_INT_L | REG_PD_INT_R,
                                REG_PD_INT_L | REG_PD_INT_R);
            }
            break;

        default:
            ERRMSG("Func:%s, Line:%d eAdc %d Fail !\n", __FUNCTION__, __LINE__, eAdc);
            goto FAIL;
            break;
    }

    return AIO_OK;

FAIL:

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

                _Msleep(50); // delay for analog initialize completed
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
                // _HalAudAtopDac(E_CHIP_AO_DAC_B, TRUE);

                // enable gpio for line-out, should after atop enable
                _Msleep(10);
                // HalAudApiAoAmpEnable(bEnable, 0); // chn0
                // HalAudApiAoAmpEnable(bEnable, 1); // chn1
            }
            else
            {
                // disable gpio for line-out, should before atop disable
                // HalAudApiAoAmpEnable(bEnable, 0); // chn0
                // HalAudApiAoAmpEnable(bEnable, 1); // chn1

                _HalAudAtopDac(E_CHIP_AO_DAC_A, FALSE);
                // _HalAudAtopDac(E_CHIP_AO_DAC_B, FALSE);
            }
            break;

        default:
            ERRMSG("Func:%s, Line:%d eAtop %d Fail !\n", __FUNCTION__, __LINE__, eAtop);
            goto FAIL;
            break;
    }

    return AIO_OK;

FAIL:

    return AIO_NG;
}

static int _HalAudDmaReInit(CHIP_AIO_DMA_e eAioDma)
{
    switch (eAioDma)
    {
        case E_CHIP_AIO_DMA_AO_A:
            HalBachWriteReg(E_BACH_REG_BANK1, E_BACH_DMA1_CTRL_1, REG_RD_TRIG,
                            0); // prevent from triggering levelcount at toggling init step
            HalBachWriteReg(E_BACH_REG_BANK1, E_BACH_DMA1_CTRL_1, REG_RD_INIT, REG_RD_INIT);
            HalBachWriteReg(E_BACH_REG_BANK1, E_BACH_DMA1_CTRL_1, REG_RD_INIT, 0);
            break;

        case E_CHIP_AIO_DMA_AI_A:
            HalBachWriteReg(E_BACH_REG_BANK1, E_BACH_DMA1_CTRL_9, REG_WR_TRIG, 0);
            HalBachWriteReg(E_BACH_REG_BANK1, E_BACH_DMA1_CTRL_9, REG_WR_INIT, REG_WR_INIT);
            HalBachWriteReg(E_BACH_REG_BANK1, E_BACH_DMA1_CTRL_9, REG_WR_INIT, 0);
            break;

        case E_CHIP_AIO_DMA_AO_B:
            HalBachWriteReg(E_BACH_REG_BANK1, E_BACH_DMA2_CTRL_1, REG_RD_TRIG,
                            0); // prevent from triggering levelcount at toggling init step
            HalBachWriteReg(E_BACH_REG_BANK1, E_BACH_DMA2_CTRL_1, REG_RD_INIT, REG_RD_INIT);
            HalBachWriteReg(E_BACH_REG_BANK1, E_BACH_DMA2_CTRL_1, REG_RD_INIT, 0);
            break;

        case E_CHIP_AIO_DMA_AI_B:
            HalBachWriteReg(E_BACH_REG_BANK1, E_BACH_DMA2_CTRL_9, REG_WR_TRIG, 0);
            HalBachWriteReg(E_BACH_REG_BANK1, E_BACH_DMA2_CTRL_9, REG_WR_INIT, REG_WR_INIT);
            HalBachWriteReg(E_BACH_REG_BANK1, E_BACH_DMA2_CTRL_9, REG_WR_INIT, 0);
            break;

        case E_CHIP_AIO_DMA_AO_C:
            HalBachWriteReg(E_BACH_REG_BANK4, E_BACH_DMA3_CTRL_1, REG_RD_TRIG,
                            0); // prevent from triggering levelcount at toggling init step
            HalBachWriteReg(E_BACH_REG_BANK4, E_BACH_DMA3_CTRL_1, REG_RD_INIT, REG_RD_INIT);
            HalBachWriteReg(E_BACH_REG_BANK4, E_BACH_DMA3_CTRL_1, REG_RD_INIT, 0);
            break;

        case E_CHIP_AIO_DMA_AI_C:
            HalBachWriteReg(E_BACH_REG_BANK4, E_BACH_DMA3_CTRL_9, REG_WR_TRIG, 0);
            HalBachWriteReg(E_BACH_REG_BANK4, E_BACH_DMA3_CTRL_9, REG_WR_INIT, REG_WR_INIT);
            HalBachWriteReg(E_BACH_REG_BANK4, E_BACH_DMA3_CTRL_9, REG_WR_INIT, 0);
            break;

        case E_CHIP_AIO_DMA_AI_D:
            HalBachWriteReg(E_BACH_REG_BANK4, E_BACH_DMA4_CTRL_9, REG_WR_TRIG, 0);
            HalBachWriteReg(E_BACH_REG_BANK4, E_BACH_DMA4_CTRL_9, REG_WR_INIT, REG_WR_INIT);
            HalBachWriteReg(E_BACH_REG_BANK4, E_BACH_DMA4_CTRL_9, REG_WR_INIT, 0);
            break;

        case E_CHIP_AIO_DMA_AI_E:
            HalBachWriteReg(E_BACH_REG_BANK4, E_BACH_DMA5_CTRL_9, REG_WR_TRIG, 0);
            HalBachWriteReg(E_BACH_REG_BANK4, E_BACH_DMA5_CTRL_9, REG_WR_INIT, REG_WR_INIT);
            HalBachWriteReg(E_BACH_REG_BANK4, E_BACH_DMA5_CTRL_9, REG_WR_INIT, 0);
            break;

        default:
            ERRMSG("Func:%s, Line:%d eAioDma %d Fail !\n", __FUNCTION__, __LINE__, eAioDma);
            goto FAIL;
            break;
    }

    return AIO_OK;

FAIL:

    return AIO_NG;
}

static int _HalAudDmaRdIntEnable(CHIP_AIO_DMA_e eAioDma, BOOL bUnderrun, BOOL bEmpty)
{
    U8            nAddr;
    BachRegBank_e eBank;
    U16           nConfigValue;

    switch (eAioDma)
    {
        case E_CHIP_AIO_DMA_AO_A:
            eBank = E_BACH_REG_BANK1;
            nAddr = E_BACH_DMA1_CTRL_0;
            break;
        case E_CHIP_AIO_DMA_AO_B:
            eBank = E_BACH_REG_BANK1;
            nAddr = E_BACH_DMA2_CTRL_0;
            break;
        case E_CHIP_AIO_DMA_AO_C:
            eBank = E_BACH_REG_BANK4;
            nAddr = E_BACH_DMA3_CTRL_0;
            break;
        case E_CHIP_AIO_DMA_AO_D:
        case E_CHIP_AIO_DMA_AO_E:
        case E_CHIP_AIO_DMA_AO_DIRECT_A:
        case E_CHIP_AIO_DMA_AO_DIRECT_B:
            return AIO_OK;
        default:
            ERRMSG("Func:%s, Line:%d eAioDma %d Fail !\n", __FUNCTION__, __LINE__, eAioDma);
            goto FAIL;
            break;
    }

    nConfigValue = HalBachReadReg(eBank, nAddr);

    /* Underrun interrupt */
    if (bUnderrun)
    {
        nConfigValue |= REG_RD_UNDERRUN_INT_EN;
    }
    else
    {
        nConfigValue &= ~REG_RD_UNDERRUN_INT_EN;
    }

    /* Empty interrupt */
    if (bEmpty)
    {
        nConfigValue |= REG_RD_EMPTY_INT_EN;
    }
    else
    {
        nConfigValue &= ~REG_RD_EMPTY_INT_EN;
    }

    HalBachWriteReg(eBank, nAddr, (REG_RD_UNDERRUN_INT_EN | REG_RD_EMPTY_INT_EN), nConfigValue);

    return AIO_OK;

FAIL:

    return AIO_NG;
}

static int _HalAudDmaWrIntEnable(CHIP_AIO_DMA_e eAioDma, BOOL bOverrun, BOOL bFull)
{
    U8            nAddr;
    BachRegBank_e eBank;
    U16           nConfigValue;

    switch (eAioDma)
    {
        case E_CHIP_AIO_DMA_AI_A:
            eBank = E_BACH_REG_BANK1;
            nAddr = E_BACH_DMA1_CTRL_0;
            break;
        case E_CHIP_AIO_DMA_AI_B:
            eBank = E_BACH_REG_BANK1;
            nAddr = E_BACH_DMA2_CTRL_0;
            break;
        case E_CHIP_AIO_DMA_AI_C:
            eBank = E_BACH_REG_BANK4;
            nAddr = E_BACH_DMA3_CTRL_0;
            break;
        case E_CHIP_AIO_DMA_AI_D:
            eBank = E_BACH_REG_BANK4;
            nAddr = E_BACH_DMA4_CTRL_0;
            break;
        case E_CHIP_AIO_DMA_AI_E:
            eBank = E_BACH_REG_BANK4;
            nAddr = E_BACH_DMA5_CTRL_0;
            break;
        case E_CHIP_AIO_DMA_AI_DIRECT_A:
        case E_CHIP_AIO_DMA_AI_DIRECT_B:
            return AIO_OK;

        default:
            ERRMSG("Func:%s, Line:%d eAioDma %d Fail !\n", __FUNCTION__, __LINE__, eAioDma);
            goto FAIL;
            break;
    }

    nConfigValue = HalBachReadReg(eBank, nAddr);

    /* Overrun interrupt */
    if (bOverrun)
    {
        nConfigValue |= REG_WR_OVERRUN_INT_EN;
    }
    else
    {
        nConfigValue &= ~REG_WR_OVERRUN_INT_EN;
    }

    /* Full interrupt */
    if (bFull)
    {
        nConfigValue |= REG_WR_FULL_INT_EN;
    }
    else
    {
        nConfigValue &= ~REG_WR_FULL_INT_EN;
    }

    HalBachWriteReg(eBank, nAddr, (REG_WR_OVERRUN_INT_EN | REG_WR_FULL_INT_EN), nConfigValue);

    return AIO_OK;

FAIL:

    return AIO_NG;
}

static int _HalAudDmaGetRdInt(CHIP_AIO_DMA_e eAioDma, BOOL *bUnderrun, BOOL *bEmpty)
{
    BachRegBank_e eBank;
    U8            nAddr;
    U16           nConfigValue = 0;

    switch (eAioDma)
    {
        case E_CHIP_AIO_DMA_AO_A:
            eBank = E_BACH_REG_BANK1;
            nAddr = E_BACH_DMA1_CTRL_0;
            break;
        case E_CHIP_AIO_DMA_AO_B:
            eBank = E_BACH_REG_BANK1;
            nAddr = E_BACH_DMA2_CTRL_0;
            break;
        case E_CHIP_AIO_DMA_AO_C:
            eBank = E_BACH_REG_BANK4;
            nAddr = E_BACH_DMA3_CTRL_0;
            break;
        default:
            ERRMSG("Func:%s, Line:%d eAioDma %d Fail !\n", __FUNCTION__, __LINE__, eAioDma);
            goto FAIL;
            break;
    }

    nConfigValue = HalBachReadReg(eBank, nAddr);

    if ((nConfigValue & REG_RD_EMPTY_INT_EN) == 0)
    {
        *bEmpty = FALSE;
    }
    else
    {
        *bEmpty = TRUE;
    }

    if ((nConfigValue & REG_RD_UNDERRUN_INT_EN) == 0)
    {
        *bUnderrun = FALSE;
    }
    else
    {
        *bUnderrun = TRUE;
    }

    return AIO_OK;

FAIL:

    return AIO_NG;
}

static int _HalAudDmaGetWrInt(CHIP_AIO_DMA_e eAioDma, BOOL *bOverrun, BOOL *bFull)
{
    U8            nAddr;
    BachRegBank_e eBank;
    U16           nConfigValue = 0;

    switch (eAioDma)
    {
        case E_CHIP_AIO_DMA_AI_A:
            eBank = E_BACH_REG_BANK1;
            nAddr = E_BACH_DMA1_CTRL_0;
            break;
        case E_CHIP_AIO_DMA_AI_B:
            eBank = E_BACH_REG_BANK1;
            nAddr = E_BACH_DMA2_CTRL_0;
            break;
        case E_CHIP_AIO_DMA_AI_C:
            eBank = E_BACH_REG_BANK4;
            nAddr = E_BACH_DMA3_CTRL_0;
            break;
        case E_CHIP_AIO_DMA_AI_D:
            eBank = E_BACH_REG_BANK4;
            nAddr = E_BACH_DMA4_CTRL_0;
            break;
        case E_CHIP_AIO_DMA_AI_E:
            eBank = E_BACH_REG_BANK4;
            nAddr = E_BACH_DMA5_CTRL_0;
            break;
        default:
            ERRMSG("Func:%s, Line:%d eAioDma %d Fail !\n", __FUNCTION__, __LINE__, eAioDma);
            goto FAIL;
            break;
    }

    nConfigValue = HalBachReadReg(eBank, nAddr);

    if ((nConfigValue & REG_WR_FULL_INT_EN) == 0)
    {
        *bFull = FALSE;
    }
    else
    {
        *bFull = TRUE;
    }

    if ((nConfigValue & REG_WR_OVERRUN_INT_EN) == 0)
    {
        *bOverrun = FALSE;
    }
    else
    {
        *bOverrun = TRUE;
    }

    return AIO_OK;

FAIL:

    return AIO_NG;
}

static int _HalAudDmaRdGetFlags(CHIP_AIO_DMA_e eAioDma, BOOL *pbUnderrun, BOOL *pbEmtpy, BOOL *pbLocalEmpty)
{
    U8            nAddr;
    U16           nConfigValue;
    BachRegBank_e eBank;

    switch (eAioDma)
    {
        case E_CHIP_AIO_DMA_AO_A:
            eBank = E_BACH_REG_BANK1;
            nAddr = E_BACH_DMA1_CTRL_8;
            break;
        case E_CHIP_AIO_DMA_AO_B:
            eBank = E_BACH_REG_BANK1;
            nAddr = E_BACH_DMA2_CTRL_8;
            break;
        case E_CHIP_AIO_DMA_AO_C:
            eBank = E_BACH_REG_BANK4;
            nAddr = E_BACH_DMA3_CTRL_8;
            break;
        default:
            ERRMSG("Func:%s, Line:%d eAioDma %d Fail !\n", __FUNCTION__, __LINE__, eAioDma);
            goto FAIL;
            break;
    }

    nConfigValue  = HalBachReadReg(eBank, nAddr);
    *pbUnderrun   = (nConfigValue & REG_RD_UNDERRUN_FLAG) ? TRUE : FALSE;
    *pbEmtpy      = (nConfigValue & REG_RD_EMPTY_FLAG) ? TRUE : FALSE;
    *pbLocalEmpty = (nConfigValue & REG_RD_LOCALBUF_EMPTY) ? TRUE : FALSE;

    return AIO_OK;

FAIL:

    return AIO_NG;
}

static int _HalAudDmaWrGetFlags(CHIP_AIO_DMA_e eAioDma, BOOL *pbOverrun, BOOL *pbFull, BOOL *pbLocalFull)
{
    U8            nAddr;
    U16           nConfigValue;
    BachRegBank_e eBank;

    switch (eAioDma)
    {
        case E_CHIP_AIO_DMA_AI_A:
            eBank = E_BACH_REG_BANK1;
            nAddr = E_BACH_DMA1_CTRL_8;
            break;
        case E_CHIP_AIO_DMA_AI_B:
            eBank = E_BACH_REG_BANK1;
            nAddr = E_BACH_DMA2_CTRL_8;
            break;
        case E_CHIP_AIO_DMA_AI_C:
            eBank = E_BACH_REG_BANK4;
            nAddr = E_BACH_DMA3_CTRL_8;
            break;
        case E_CHIP_AIO_DMA_AI_D:
            eBank = E_BACH_REG_BANK4;
            nAddr = E_BACH_DMA4_CTRL_8;
            break;
        case E_CHIP_AIO_DMA_AI_E:
            eBank = E_BACH_REG_BANK4;
            nAddr = E_BACH_DMA5_CTRL_8;
            break;

        default:
            ERRMSG("Func:%s, Line:%d eAioDma %d Fail !\n", __FUNCTION__, __LINE__, eAioDma);
            goto FAIL;
            break;
    }

    nConfigValue = HalBachReadReg(eBank, nAddr);
    *pbOverrun   = (nConfigValue & REG_WR_OVERRUN_FLAG) ? TRUE : FALSE;
    *pbFull      = (nConfigValue & REG_WR_FULL_FLAG) ? TRUE : FALSE;
    *pbLocalFull = (nConfigValue & REG_WR_LOCALBUF_FULL) ? TRUE : FALSE;

    return AIO_OK;

FAIL:

    return AIO_NG;
}

static U32 _HalAudDmaGetRawMiuUnitLevelCnt(CHIP_AIO_DMA_e eAioDma)
{
    U16 nConfigValue = 0;

    switch (eAioDma)
    {
        case E_CHIP_AIO_DMA_AI_A:
            HalBachWriteReg(E_BACH_REG_BANK1, E_BACH_DMA1_CTRL_9, REG_WR_LEVEL_CNT_MASK, REG_WR_LEVEL_CNT_MASK);
            nConfigValue = HalBachReadReg(E_BACH_REG_BANK1, E_BACH_DMA1_CTRL_15);
            HalBachWriteReg(E_BACH_REG_BANK1, E_BACH_DMA1_CTRL_9, REG_WR_LEVEL_CNT_MASK, 0);
            break;

        case E_CHIP_AIO_DMA_AO_A:
            HalBachWriteReg(E_BACH_REG_BANK1, E_BACH_DMA1_CTRL_1, REG_RD_LEVEL_CNT_MASK, REG_RD_LEVEL_CNT_MASK);
            nConfigValue = HalBachReadReg(E_BACH_REG_BANK1, E_BACH_DMA1_CTRL_7);
            HalBachWriteReg(E_BACH_REG_BANK1, E_BACH_DMA1_CTRL_1, REG_RD_LEVEL_CNT_MASK, 0);
            break;

        case E_CHIP_AIO_DMA_AI_B:
            HalBachWriteReg(E_BACH_REG_BANK1, E_BACH_DMA2_CTRL_9, REG_WR_LEVEL_CNT_MASK, REG_WR_LEVEL_CNT_MASK);
            nConfigValue = HalBachReadReg(E_BACH_REG_BANK1, E_BACH_DMA2_CTRL_15);
            HalBachWriteReg(E_BACH_REG_BANK1, E_BACH_DMA2_CTRL_9, REG_WR_LEVEL_CNT_MASK, 0);
            break;

        case E_CHIP_AIO_DMA_AO_B:
            HalBachWriteReg(E_BACH_REG_BANK1, E_BACH_DMA2_CTRL_1, REG_RD_LEVEL_CNT_MASK, REG_RD_LEVEL_CNT_MASK);
            nConfigValue = HalBachReadReg(E_BACH_REG_BANK1, E_BACH_DMA2_CTRL_7);
            HalBachWriteReg(E_BACH_REG_BANK1, E_BACH_DMA2_CTRL_1, REG_RD_LEVEL_CNT_MASK, 0);
            break;

        case E_CHIP_AIO_DMA_AI_C:
            HalBachWriteReg(E_BACH_REG_BANK4, E_BACH_DMA3_CTRL_9, REG_WR_LEVEL_CNT_MASK, REG_WR_LEVEL_CNT_MASK);
            nConfigValue = HalBachReadReg(E_BACH_REG_BANK4, E_BACH_DMA3_CTRL_15);
            HalBachWriteReg(E_BACH_REG_BANK4, E_BACH_DMA3_CTRL_9, REG_WR_LEVEL_CNT_MASK, 0);
            break;

        case E_CHIP_AIO_DMA_AO_C:
            HalBachWriteReg(E_BACH_REG_BANK4, E_BACH_DMA3_CTRL_1, REG_RD_LEVEL_CNT_MASK, REG_RD_LEVEL_CNT_MASK);
            nConfigValue = HalBachReadReg(E_BACH_REG_BANK4, E_BACH_DMA3_CTRL_7);
            HalBachWriteReg(E_BACH_REG_BANK4, E_BACH_DMA3_CTRL_1, REG_RD_LEVEL_CNT_MASK, 0);
            break;

        case E_CHIP_AIO_DMA_AI_D:
            HalBachWriteReg(E_BACH_REG_BANK4, E_BACH_DMA4_CTRL_9, REG_WR_LEVEL_CNT_MASK, REG_WR_LEVEL_CNT_MASK);
            nConfigValue = HalBachReadReg(E_BACH_REG_BANK4, E_BACH_DMA4_CTRL_05);
            HalBachWriteReg(E_BACH_REG_BANK4, E_BACH_DMA4_CTRL_9, REG_WR_LEVEL_CNT_MASK, 0);
            break;

        case E_CHIP_AIO_DMA_AI_E:
            HalBachWriteReg(E_BACH_REG_BANK4, E_BACH_DMA5_CTRL_9, REG_WR_LEVEL_CNT_MASK, REG_WR_LEVEL_CNT_MASK);
            nConfigValue = HalBachReadReg(E_BACH_REG_BANK4, E_BACH_DMA5_CTRL_15);
            HalBachWriteReg(E_BACH_REG_BANK4, E_BACH_DMA5_CTRL_9, REG_WR_LEVEL_CNT_MASK, 0);
            break;

        default:
            ERRMSG("Func:%s, Line:%d eAioDma %d Fail !\n", __FUNCTION__, __LINE__, eAioDma);
            return 0;
    }

    return (U32)nConfigValue;
}

/* nChSel table
0000: dmic01	0001: dmic23
0100: adc01	    0101: adc23
1000: i2s_rx01	1001: i2s_rx23
1010: i2s_rx45	1011: i2s_rx67
1100: src
*/
static int _HalAudDmaGetMchSelConfigValue(AudMchSel_e eMchSel, U16 *nChSel, EN_CHIP_AI_MCH eMchSet)
{
    if (eMchSet == E_CHIP_AI_MCH_A || eMchSet == E_CHIP_AI_MCH_E)
    {
        switch (eMchSel)
        {
            case E_AUD_MCH_SEL_DMIC01:
                *nChSel = 0; // b'0000
                break;
            case E_AUD_MCH_SEL_DMIC23:
                *nChSel = 1; // b'0001
                break;
            case E_AUD_MCH_SEL_AMIC01:
                *nChSel = 4; // b'0100
                break;
            case E_AUD_MCH_SEL_AMIC23:
                *nChSel = 5; // b'0101
                break;
            case E_AUD_MCH_SEL_I2S_A_RX01:
                *nChSel = 8; // b'1000
                break;
            case E_AUD_MCH_SEL_I2S_A_RX23:
                *nChSel = 9; // b'1001
                break;
            case E_AUD_MCH_SEL_I2S_A_RX45:
                *nChSel = 10; // b'1010
                break;
            case E_AUD_MCH_SEL_I2S_A_RX67:
                *nChSel = 11; // b'1011
                break;
            case E_AUD_MCH_SEL_SRC:
                *nChSel = 12; // b'1100
                break;
            case E_AUD_MCH_SEL_NULL:
                *nChSel = 15; // b'1111
                break;
            default:
                ERRMSG("Func:%s, Line:%d eMchSet %d eMchSel %d Fail !\n", __FUNCTION__, __LINE__, eMchSet, eMchSel);
                goto FAIL;
                break;
        }
    }
    else if (eMchSet == E_CHIP_AI_MCH_B)
    {
        switch (eMchSel)
        {
            case E_AUD_MCH_SEL_DMIC01:
                *nChSel = 0; // b'0000
                break;
            case E_AUD_MCH_SEL_DMIC23:
                *nChSel = 1; // b'0001
                break;
            case E_AUD_MCH_SEL_AMIC01:
                *nChSel = 4; // b'0100
                break;
            case E_AUD_MCH_SEL_AMIC23:
                *nChSel = 5; // b'0101
                break;
            case E_AUD_MCH_SEL_I2S_B_RX01:
                *nChSel = 8; // b'1000
                break;
            case E_AUD_MCH_SEL_I2S_B_RX23:
                *nChSel = 9; // b'1001
                break;
            case E_AUD_MCH_SEL_I2S_B_RX45:
                *nChSel = 10; // b'1010
                break;
            case E_AUD_MCH_SEL_I2S_B_RX67:
                *nChSel = 11; // b'1011
                break;
            case E_AUD_MCH_SEL_SRC:
                *nChSel = 12; // b'1100
                break;
            case E_AUD_MCH_SEL_NULL:
                *nChSel = 15; // b'1111
                break;
            default:
                ERRMSG("Func:%s, Line:%d eMchSet %d eMchSel %d Fail !\n", __FUNCTION__, __LINE__, eMchSet, eMchSel);
                goto FAIL;
                break;
        }
    }
    else if (eMchSet == E_CHIP_AI_MCH_C)
    {
        switch (eMchSel)
        {
            case E_AUD_MCH_SEL_DMIC01:
                *nChSel = 0; // b'0000
                break;
            case E_AUD_MCH_SEL_DMIC23:
                *nChSel = 1; // b'0001
                break;
            case E_AUD_MCH_SEL_I2S_A_RX01:
                *nChSel = 4; // b'0100
                break;
            case E_AUD_MCH_SEL_I2S_A_RX23:
                *nChSel = 5; // b'0101
                break;
            case E_AUD_MCH_SEL_I2S_A_RX45:
                *nChSel = 6; // b'0110
                break;
            case E_AUD_MCH_SEL_I2S_A_RX67:
                *nChSel = 7; // b'0111
                break;
            case E_AUD_MCH_SEL_I2S_C_RX01:
                *nChSel = 8; // b'1000
                break;
            case E_AUD_MCH_SEL_I2S_C_RX23:
                *nChSel = 9; // b'1001
                break;
            case E_AUD_MCH_SEL_I2S_C_RX45:
                *nChSel = 10; // b'1010
                break;
            case E_AUD_MCH_SEL_I2S_C_RX67:
                *nChSel = 11; // b'1011
                break;
            case E_AUD_MCH_SEL_SRC:
                *nChSel = 12; // b'1100
                break;
            case E_AUD_MCH_SEL_NULL:
                *nChSel = 15; // b'1111
                break;
            default:
                ERRMSG("Func:%s, Line:%d eMchSet %d eMchSel %d Fail !\n", __FUNCTION__, __LINE__, eMchSet, eMchSel);
                goto FAIL;
                break;
        }
    }
    else if (eMchSet == E_CHIP_AI_MCH_D)
    {
        switch (eMchSel)
        {
            case E_AUD_MCH_SEL_DMIC01:
                *nChSel = 0; // b'0000
                break;
            case E_AUD_MCH_SEL_DMIC23:
                *nChSel = 1; // b'0001
                break;
            case E_AUD_MCH_SEL_I2S_B_RX01:
                *nChSel = 4; // b'0100
                break;
            case E_AUD_MCH_SEL_I2S_B_RX23:
                *nChSel = 5; // b'0101
                break;
            case E_AUD_MCH_SEL_I2S_B_RX45:
                *nChSel = 6; // b'0110
                break;
            case E_AUD_MCH_SEL_I2S_B_RX67:
                *nChSel = 7; // b'0111
                break;
            case E_AUD_MCH_SEL_I2S_D_RX01:
                *nChSel = 8; // b'1000
                break;
            case E_AUD_MCH_SEL_I2S_D_RX23:
                *nChSel = 9; // b'1001
                break;
            case E_AUD_MCH_SEL_I2S_D_RX45:
                *nChSel = 10; // b'1010
                break;
            case E_AUD_MCH_SEL_I2S_D_RX67:
                *nChSel = 11; // b'1011
                break;
            case E_AUD_MCH_SEL_SRC:
                *nChSel = 12; // b'1100
                break;
            case E_AUD_MCH_SEL_NULL:
                *nChSel = 15; // b'1111
                break;
            default:
                ERRMSG("Func:%s, Line:%d eMchSet %d eMchSel %d Fail !\n", __FUNCTION__, __LINE__, eMchSet, eMchSel);
                goto FAIL;
                break;
        }
    }
    else
    {
        ERRMSG("Func:%s, Line:%d eMchSet %d Fail !\n", __FUNCTION__, __LINE__, eMchSet);
        goto FAIL;
    }
    return AIO_OK;

FAIL:

    return AIO_NG;
}

static U32 _HalAudBckCalculate(AudRate_e eRate, AudBitWidth_e enI2sWidth, U16 Chn)
{
    U32 value   = 0;
    U32 rate    = 0;
    U32 width   = 0;
    U32 channel = Chn;

    switch (eRate)
    {
        case E_AUD_RATE_192K:
            rate = 192000;
            break;
        case E_AUD_RATE_96K:
            rate = 96000;
            break;
        case E_AUD_RATE_44K:
            rate = 44100;
            break;
        case E_AUD_RATE_48K:
            rate = 48000;
            break;
        case E_AUD_RATE_32K:
            rate = 32000;
            break;
        case E_AUD_RATE_16K:
            rate = 16000;
            break;
        case E_AUD_RATE_8K:
            rate = 8000;
            break;
        default:
            ERRMSG("Func:%s, Line:%d eRate %d Fail !\n", __FUNCTION__, __LINE__, eRate);
            return 0;
    }

    switch (enI2sWidth)
    {
        case E_AUD_BITWIDTH_16:
            width = 16;
            break;
        case E_AUD_BITWIDTH_32:
            width = 32;
            break;
        default:
            ERRMSG("Func:%s, Line:%d Width %d Fail !\n", __FUNCTION__, __LINE__, width);
            return 0;
    }

    // div8, because of BACH_RX_DIV_SEL_MSK and BACH_TX_DIV_SEL_MSK set to 0x3
    value = ((U64)(1 << 22) * REF_FREQ * 1000000 / (rate * width * channel / 2) / 8);
    return value;
}

static int _HalAudI2sSetBck(CHIP_AIO_I2S_e eAioI2s, U32 nNfValue)
{
    U16 bckValueLo = 0;
    U16 bckValueHi = 0;

    if (nNfValue == 0)
    {
        ERRMSG("Func:%s, Line:%d nNfValue %d Fail !\n", __FUNCTION__, __LINE__, nNfValue);
        goto FAIL;
    }

    bckValueLo = nNfValue & 0xFFFF;
    bckValueHi = (nNfValue >> 16) & 0xFFFF;

    switch (eAioI2s)
    {
        case E_CHIP_AIO_I2S_RX_A:
            // 0x150378[15:14] for master RX BCK div1/div2/div4/div8 selection
            HalBachWriteReg(E_BACH_REG_BANK2, E_BACH_I2S_CFG00, BACH_RX_DIV_SEL_MSK, 3 << BACH_RX_DIV_SEL_POS);
            //
            HalBachWriteReg(E_BACH_REG_BANK2, E_BACH_NF_SYNTH_RX_BCK01, BACH_CODEC_BCK_EN_SYNTH_NF_VALUE_LO,
                            bckValueLo);
            HalBachWriteReg(E_BACH_REG_BANK2, E_BACH_NF_SYNTH_RX_BCK02, BACH_CODEC_BCK_EN_SYNTH_NF_VALUE_HI,
                            bckValueHi);
            break;

        case E_CHIP_AIO_I2S_TX_A:
            // 0x150378[11:10] for master TX BCK div1/div2/div4/div8 selection
            HalBachWriteReg(E_BACH_REG_BANK2, E_BACH_I2S_CFG00, BACH_TX_DIV_SEL_MSK, 3 << BACH_TX_DIV_SEL_POS);
            //
            HalBachWriteReg(E_BACH_REG_BANK2, E_BACH_NF_SYNTH_TX_BCK01, BACH_CODEC_BCK_EN_SYNTH_NF_VALUE_LO,
                            bckValueLo);
            HalBachWriteReg(E_BACH_REG_BANK2, E_BACH_NF_SYNTH_TX_BCK02, BACH_CODEC_BCK_EN_SYNTH_NF_VALUE_HI,
                            bckValueHi);
            break;
            //
        case E_CHIP_AIO_I2S_RX_B:
            // 0x150378[15:14] for master RX BCK div1/div2/div4/div8 selection
            HalBachWriteReg(E_BACH_REG_BANK4, E_BACH_I2S1_CFG00, BACH_RX_DIV_SEL_MSK, 3 << BACH_RX_DIV_SEL_POS);
            //
            HalBachWriteReg(E_BACH_REG_BANK4, E_BACH_NF_SYNTH_RX1_BCK01, BACH_CODEC_BCK_EN_SYNTH_NF_VALUE_LO,
                            bckValueLo);
            HalBachWriteReg(E_BACH_REG_BANK4, E_BACH_NF_SYNTH_RX1_BCK02, BACH_CODEC_BCK_EN_SYNTH_NF_VALUE_HI,
                            bckValueHi);
            break;

        case E_CHIP_AIO_I2S_TX_B:
            // 0x150378[11:10] for master TX BCK div1/div2/div4/div8 selection
            HalBachWriteReg(E_BACH_REG_BANK4, E_BACH_I2S1_CFG00, BACH_TX_DIV_SEL_MSK, 3 << BACH_TX_DIV_SEL_POS);
            //
            HalBachWriteReg(E_BACH_REG_BANK4, E_BACH_NF_SYNTH_TX1_BCK01, BACH_CODEC_BCK_EN_SYNTH_NF_VALUE_LO,
                            bckValueLo);
            HalBachWriteReg(E_BACH_REG_BANK4, E_BACH_NF_SYNTH_TX1_BCK02, BACH_CODEC_BCK_EN_SYNTH_NF_VALUE_HI,
                            bckValueHi);
            break;

        case E_CHIP_AIO_I2S_RX_C:
            // 0x150378[15:14] for master RX BCK div1/div2/div4/div8 selection
            HalBachWriteReg(E_BACH_REG_BANK2, E_BACH_I2S_CFG00, BACH_RX_DIV_SEL_MSK, 3 << BACH_RX_DIV_SEL_POS);
            //
            HalBachWriteReg(E_BACH_REG_BANK2, E_BACH_NF_SYNTH_RX_BCK01, BACH_CODEC_BCK_EN_SYNTH_NF_VALUE_LO,
                            bckValueLo);
            HalBachWriteReg(E_BACH_REG_BANK2, E_BACH_NF_SYNTH_RX_BCK02, BACH_CODEC_BCK_EN_SYNTH_NF_VALUE_HI,
                            bckValueHi);
            break;

        case E_CHIP_AIO_I2S_RX_D:
            // 0x150378[15:14] for master RX BCK div1/div2/div4/div8 selection
            HalBachWriteReg(E_BACH_REG_BANK4, E_BACH_I2S1_CFG00, BACH_RX_DIV_SEL_MSK, 3 << BACH_RX_DIV_SEL_POS);
            //
            HalBachWriteReg(E_BACH_REG_BANK4, E_BACH_NF_SYNTH_RX1_BCK01, BACH_CODEC_BCK_EN_SYNTH_NF_VALUE_LO,
                            bckValueLo);
            HalBachWriteReg(E_BACH_REG_BANK4, E_BACH_NF_SYNTH_RX1_BCK02, BACH_CODEC_BCK_EN_SYNTH_NF_VALUE_HI,
                            bckValueHi);
            break;

        default:
            ERRMSG("Func:%s, Line:%d eAioI2s %d Fail !\n", __FUNCTION__, __LINE__, eAioI2s);
            goto FAIL;
            break;
    }

    return AIO_OK;

FAIL:

    return AIO_NG;
}

static int _HalAudMckCalculate(CHIP_AIO_I2S_e eAioI2s, AudI2sMck_e eMck)
{
    U16 mckValueLo = 0;
    U16 mckValueHi = 0;
    U16 nDuty;
    U32 nNf;
    U16 bank;

    switch (eAioI2s)
    {
        case E_CHIP_AIO_I2S_TX_A:
        case E_CHIP_AIO_I2S_RX_A:
            bank = E_BACH_REG_BANK2;
            break;
        case E_CHIP_AIO_I2S_TX_B:
        case E_CHIP_AIO_I2S_RX_B:
        case E_CHIP_AIO_I2S_RX_C:
        case E_CHIP_AIO_I2S_RX_D:
            bank = E_BACH_REG_BANK4;
            break;
        default:
            ERRMSG("Func:%s, Line:%d eAioI2s %d Fail !\n", __FUNCTION__, __LINE__, eAioI2s);
            goto FAIL;
    }

    if (eMck == E_AUD_I2S_MCK_NULL)
    {
        // do nothing
        return AIO_OK;
    }

    switch (eMck)
    {
        case E_AUD_I2S_MCK_12_288M:
            nNf   = (0x1F4000);
            nDuty = (U16)((U32)384000 * 50 / 12288 / 100);
            break;

        case E_AUD_I2S_MCK_16_384M:
            nNf   = (0x177000);
            nDuty = (U16)((U32)384000 * 50 / 16384 / 100);
            break;

        case E_AUD_I2S_MCK_18_432M:
            nNf   = (0x14D555);
            nDuty = (U16)((U32)384000 * 50 / 18432 / 100);
            break;

        case E_AUD_I2S_MCK_24_576M:
            nNf   = (0xFA000);
            nDuty = (U16)((U32)384000 * 50 / 24576 / 100);
            break;

        default:
            ERRMSG("Func:%s, Line:%d eMck %d Fail !\n", __FUNCTION__, __LINE__, eMck);
            goto FAIL;
            break;
    }

    mckValueHi = (U16)((nNf) >> BACH_MCK_NF_VALUE_HI_OFFSET) & BACH_MCK_NF_VALUE_HI_MSK;
    HalBachWriteReg(bank, E_BACH_NF_SYNTH_MCK02, BACH_MCK_NF_VALUE_HI_MSK, mckValueHi);
    mckValueLo = (U16)((nNf)&BACH_MCK_NF_VALUE_LO_MSK);
    HalBachWriteReg(bank, E_BACH_NF_SYNTH_MCK01, BACH_MCK_NF_VALUE_LO_MSK, mckValueLo);

    // Duty
    HalBachWriteReg(bank, E_BACH_NF_SYNTH_MCK02, BACH_MCK_EXPAND_MSK, (nDuty << BACH_MCK_EXPAND_POS));

    return AIO_OK;

FAIL:

    return AIO_NG;
}

static int _HalAudDmaSetMiuSel(CHIP_AIO_DMA_e eAioDma, U32 nBufAddrOffset)
{
    // reg_dma_test_ctrl7[5:4] is defined as  DMA1_MIU_SEL[1:0]
    // reg_dma_test_ctrl7[7:6] is defined as  DMA2_MIU_SEL[1:0]
    U8 nMiuSel = 0;
    U8 nValue  = 0;

    nMiuSel = 0; // nBufAddrOffset >= 0x80000000 ? 1 : 0;
    switch (eAioDma)
    {
        case E_CHIP_AIO_DMA_AI_A:
        case E_CHIP_AIO_DMA_AO_A:
            nValue = nMiuSel ? REG_DMA1_MIU_SEL0 : 0;
            HalBachWriteReg(E_BACH_REG_BANK1, E_BACH_DMA_TEST_CTRL7, REG_DMA1_MIU_SEL1 | REG_DMA1_MIU_SEL0, nValue);
            break;

        case E_CHIP_AIO_DMA_AI_B:
        case E_CHIP_AIO_DMA_AO_B:
            nValue = nMiuSel ? REG_DMA2_MIU_SEL0 : 0;
            HalBachWriteReg(E_BACH_REG_BANK1, E_BACH_DMA_TEST_CTRL7, REG_DMA2_MIU_SEL1 | REG_DMA2_MIU_SEL0, nValue);
            break;

        case E_CHIP_AIO_DMA_AI_C:
        case E_CHIP_AIO_DMA_AO_C:
            nValue = nMiuSel ? REG_DMA3_MIU_SEL0 : 0;
            HalBachWriteReg(E_BACH_REG_BANK4, E_BACH_DMA3_CTRL_17, REG_DMA3_MIU_SEL1 | REG_DMA3_MIU_SEL0, nValue);
            break;

        case E_CHIP_AIO_DMA_AI_D:
            nValue = nMiuSel ? REG_DMA3_MIU_SEL0 : 0;
            HalBachWriteReg(E_BACH_REG_BANK4, E_BACH_DMA3_CTRL_17, REG_DMA3_MIU_SEL1 | REG_DMA3_MIU_SEL0, nValue);
            break;

        case E_CHIP_AIO_DMA_AI_E:
            nValue = nMiuSel ? REG_DMA2_MIU_SEL0 : 0;
            HalBachWriteReg(E_BACH_REG_BANK4, E_BACH_DMA3_CTRL_17, REG_DMA3_MIU_SEL1 | REG_DMA3_MIU_SEL0, nValue);
            break;

        default:
            ERRMSG("Func:%s, Line:%d eAioDma %d Fail !\n", __FUNCTION__, __LINE__, eAioDma);
            goto FAIL;
            break;
    }

    return AIO_OK;

FAIL:

    return AIO_NG;
}

static int _HalAudDpgaCalGain(S8 s8Gain, U8 *pU8GainIdx)
{
    if (s8Gain > BACH_DPGA_GAIN_MAX_DB)
    {
        s8Gain = BACH_DPGA_GAIN_MAX_DB;
    }
    else if (s8Gain < BACH_DPGA_GAIN_MIN_DB)
    {
        s8Gain = BACH_DPGA_GAIN_MIN_DB;
    }

    if (s8Gain == BACH_DPGA_GAIN_MIN_DB)
    {
        *pU8GainIdx = BACH_DPGA_GAIN_MIN_IDX;
    }
    else
    {
        *pU8GainIdx = (U8)(-2 * s8Gain); // index = -2 * (gain) ,because step = -0.5dB
    }

    return AIO_OK;
}

static int _HalAudSetAiPathCoreDmaA(U8 nDmaCh, AI_CH_e eAiCh)
{
    int ret  = AIO_OK;
    int line = 0;

    //
    // nDmaCh may be used in new chip (P4)

    switch (eAiCh)
    {
        case E_AI_CH_ADC_A_0:
        case E_AI_CH_ADC_B_0:
        case E_AI_CH_DMIC_A_0:
        case E_AI_CH_DMIC_A_1:
        case E_AI_CH_DMIC_A_2:
        case E_AI_CH_DMIC_A_3:
        case E_AI_CH_ECHO_A_0:
        case E_AI_CH_ECHO_A_1:

            ret |= HalAudSetMux(E_AUD_MUX_DMAWR1, 1); // It may be no need.
            if (ret != AIO_OK)
            {
                line = __LINE__;
                goto FAIL;
            }

            ret |= HalAudSetMux(E_AUD_MUX_DMAWR1_MCH, 1);
            if (ret != AIO_OK)
            {
                line = __LINE__;
                goto FAIL;
            }

            break;

        case E_AI_CH_I2S_RX_A_0:
        case E_AI_CH_I2S_RX_A_1:
        case E_AI_CH_I2S_RX_A_2:
        case E_AI_CH_I2S_RX_A_3:
        case E_AI_CH_I2S_RX_A_4:
        case E_AI_CH_I2S_RX_A_5:
        case E_AI_CH_I2S_RX_A_6:
        case E_AI_CH_I2S_RX_A_7:

            ret |= HalAudSetMux(E_AUD_MUX_DMAWR1, 1); // It may be no need.
            if (ret != AIO_OK)
            {
                line = __LINE__;
                goto FAIL;
            }

            ret |= HalAudSetMux(E_AUD_MUX_DMAWR1_MCH, 1); // 150396[1] 1:selec DMA1 mch 0:select ADC
            if (ret != AIO_OK)
            {
                line = __LINE__;
                goto FAIL;
            }
            ret |= HalAudSetMux(E_AUD_MUX_I2STDM_RX, 1); // 150378[0] 1: I2STDM 0:Ext I2S
            if (ret != AIO_OK)
            {
                line = __LINE__;
                goto FAIL;
            }

            break;

        default:
            line = __LINE__;
            goto FAIL;
            break;
    }

    return AIO_OK;

FAIL:
    ERRMSG("Func:%s, Line:%d eAiCh %d Fail !\n", __FUNCTION__, line, eAiCh);

    return AIO_NG;
}

static int _HalAudSetAiPathCoreDmaB(U8 nDmaCh, AI_CH_e eAiCh)
{
    int ret  = AIO_OK;
    int line = 0;

    switch (eAiCh)
    {
        case E_AI_CH_ADC_A_0:
        case E_AI_CH_ADC_B_0:
        case E_AI_CH_DMIC_A_0:
        case E_AI_CH_DMIC_A_1:
        case E_AI_CH_DMIC_A_2:
        case E_AI_CH_DMIC_A_3:
        case E_AI_CH_ECHO_A_0:
        case E_AI_CH_ECHO_A_1:
            ret |= HalAudSetMux(E_AUD_MUX_DMAWR2_MCH, 1); // 0x150398[1] 1:selec DMA2 mch 0:selec ADC
            if (ret != AIO_OK)
            {
                line = __LINE__;
                goto FAIL;
            }

            break;

        case E_AI_CH_I2S_RX_B_0:
        case E_AI_CH_I2S_RX_B_1:
        case E_AI_CH_I2S_RX_B_2:
        case E_AI_CH_I2S_RX_B_3:
        case E_AI_CH_I2S_RX_B_4:
        case E_AI_CH_I2S_RX_B_5:
        case E_AI_CH_I2S_RX_B_6:
        case E_AI_CH_I2S_RX_B_7:
            ret |= HalAudSetMux(E_AUD_MUX_DMAWR2_MCH, 1); // 0x150398[1] 1:selec DMA2 mch 0:selec ADC
            if (ret != AIO_OK)
            {
                line = __LINE__;
                goto FAIL;
            }

            break;

        default:
            line = __LINE__;
            goto FAIL;
            break;
    }

    return AIO_OK;

FAIL:
    ERRMSG("Func:%s, Line:%d eAiCh %d Fail !\n", __FUNCTION__, line, eAiCh);

    return AIO_NG;
}

static int _HalAudSetAiPathCoreDmaC(U8 nDmaCh, AI_CH_e eAiCh)
{
    int ret  = AIO_OK;
    int line = 0;

    switch (eAiCh)
    {
        case E_AI_CH_DMIC_A_0:
        case E_AI_CH_DMIC_A_1:
        case E_AI_CH_DMIC_A_2:
        case E_AI_CH_DMIC_A_3:
        case E_AI_CH_ECHO_A_0:
        case E_AI_CH_ECHO_A_1:
            break;

        case E_AI_CH_I2S_RX_A_0:
        case E_AI_CH_I2S_RX_A_1:
        case E_AI_CH_I2S_RX_A_2:
        case E_AI_CH_I2S_RX_A_3:
        case E_AI_CH_I2S_RX_A_4:
        case E_AI_CH_I2S_RX_A_5:
        case E_AI_CH_I2S_RX_A_6:
        case E_AI_CH_I2S_RX_A_7:

            ret |= HalAudSetMux(E_AUD_MUX_I2STDM_RX, 1); // 150378[0] 1: I2STDM 0:Ext I2S
            if (ret != AIO_OK)
            {
                line = __LINE__;
                goto FAIL;
            }
            break;

        case E_AI_CH_I2S_RX_C_0:
        case E_AI_CH_I2S_RX_C_1:
        case E_AI_CH_I2S_RX_C_2:
        case E_AI_CH_I2S_RX_C_3:
        case E_AI_CH_I2S_RX_C_4:
        case E_AI_CH_I2S_RX_C_5:
        case E_AI_CH_I2S_RX_C_6:
        case E_AI_CH_I2S_RX_C_7:
            break;

        default:
            line = __LINE__;
            goto FAIL;
            break;
    }

    return AIO_OK;

FAIL:
    ERRMSG("Func:%s, Line:%d eAiCh %d Fail !\n", __FUNCTION__, line, eAiCh);

    return AIO_NG;
}

static int _HalAudSetAiPathCoreDmaD(U8 nDmaCh, AI_CH_e eAiCh)
{
    int line = 0;

    switch (eAiCh)
    {
        case E_AI_CH_ECHO_A_0:
        case E_AI_CH_ECHO_A_1:
        case E_AI_CH_DMIC_A_0:
        case E_AI_CH_DMIC_A_1:
        case E_AI_CH_DMIC_A_2:
        case E_AI_CH_DMIC_A_3:
            break;

        case E_AI_CH_I2S_RX_B_0:
        case E_AI_CH_I2S_RX_B_1:
        case E_AI_CH_I2S_RX_B_2:
        case E_AI_CH_I2S_RX_B_3:
        case E_AI_CH_I2S_RX_B_4:
        case E_AI_CH_I2S_RX_B_5:
        case E_AI_CH_I2S_RX_B_6:
        case E_AI_CH_I2S_RX_B_7:
            break;

        case E_AI_CH_I2S_RX_D_0:
        case E_AI_CH_I2S_RX_D_1:
        case E_AI_CH_I2S_RX_D_2:
        case E_AI_CH_I2S_RX_D_3:
        case E_AI_CH_I2S_RX_D_4:
        case E_AI_CH_I2S_RX_D_5:
        case E_AI_CH_I2S_RX_D_6:
        case E_AI_CH_I2S_RX_D_7:
            break;

        default:
            line = __LINE__;
            goto FAIL;
            break;
    }

    return AIO_OK;

FAIL:
    ERRMSG("Func:%s, Line:%d eAiCh %d Fail !\n", __FUNCTION__, line, eAiCh);

    return AIO_NG;
}

static int _HalAudSetAiPathCoreDmaE(U8 nDmaCh, AI_CH_e eAiCh)
{
    int line = 0;
    int ret  = AIO_OK;

    switch (eAiCh)
    {
        case E_AI_CH_ADC_A_0:
        case E_AI_CH_ADC_B_0:
            break;

        case E_AI_CH_DMIC_A_0:
        case E_AI_CH_DMIC_A_1:
        case E_AI_CH_DMIC_A_2:
        case E_AI_CH_DMIC_A_3:
            break;

        case E_AI_CH_ECHO_A_0:
        case E_AI_CH_ECHO_A_1:
            break;

        case E_AI_CH_I2S_RX_A_0:
        case E_AI_CH_I2S_RX_A_1:
        case E_AI_CH_I2S_RX_A_2:
        case E_AI_CH_I2S_RX_A_3:
        case E_AI_CH_I2S_RX_A_4:
        case E_AI_CH_I2S_RX_A_5:
        case E_AI_CH_I2S_RX_A_6:
        case E_AI_CH_I2S_RX_A_7:
            ret |= HalAudSetMux(E_AUD_MUX_I2STDM_RX, 1); // 150378[0] 1: I2STDM 0:Ext I2S
            if (ret != AIO_OK)
            {
                line = __LINE__;
                goto FAIL;
            }
            break;

        default:
            line = __LINE__;
            goto FAIL;
            break;
    }

    return AIO_OK;

FAIL:
    ERRMSG("Func:%s, Line:%d eAiCh %d Fail !\n", __FUNCTION__, line, eAiCh);

    return AIO_NG;
}

static int _HalAudSetAoPathCoreDmaA(U8 nDmaCh, AO_CH_e eAoCh)
{
    int ret  = AIO_OK;
    int line = 0;

    switch (eAoCh)
    {
        case E_AO_CH_DAC_A_0:

            ret |= HalAudSetMux(E_AUD_MUX_MMC1, 1); // 150206[5] 1:from RDMA1 0:from ASRC
            if (ret != AIO_OK)
            {
                line = __LINE__;
                goto FAIL;
            }

            if (nDmaCh == 0)
            {
                ret |= HalAudSetMux(E_AUD_MUX_SDM_DINL, 0); // AB1_L
            }
            else
            {
                ret |= HalAudSetMux(E_AUD_MUX_SDM_DINL, 1); // AB1_R
            }

            if (ret != AIO_OK)
            {
                line = __LINE__;
                goto FAIL;
            }

            break;

        case E_AO_CH_DAC_B_0:

            ret |= HalAudSetMux(E_AUD_MUX_MMC1, 1);
            if (ret != AIO_OK)
            {
                line = __LINE__;
                goto FAIL;
            }

            if (nDmaCh == 0)
            {
                ret |= HalAudSetMux(E_AUD_MUX_SDM_DINR, 1); // AB1_L
            }
            else
            {
                ret |= HalAudSetMux(E_AUD_MUX_SDM_DINR, 0); // 1602CC[4:6] AB1_R
            }
            if (ret != AIO_OK)
            {
                line = __LINE__;
                goto FAIL;
            }

            break;

        case E_AO_CH_DAC_C_0:
        case E_AO_CH_DAC_D_0:

            ERRMSG("Func:%s, Line:%d don't support this audio path !\n", __FUNCTION__, __LINE__);
            line = __LINE__;
            goto FAIL;

            break;

        case E_AO_CH_HDMI_A_0:
        case E_AO_CH_HDMI_A_1:

            ret |= HalAudSetMux(E_AUD_MUX_AO_HDMI, 0);
            if (ret != AIO_OK)
            {
                line = __LINE__;
                goto FAIL;
            }

            break;
        case E_AO_CH_ECHO_A_0:

            ret |= HalAudSetMux(E_AUD_MUX_MMC1, 1);
            if (ret != AIO_OK)
            {
                line = __LINE__;
                goto FAIL;
            }

            if (nDmaCh == 0)
            {
                ret |= HalAudSetMux(E_AUD_MUX_SDM_DINL, 0); // AB1_L
            }
            else
            {
                ret |= HalAudSetMux(E_AUD_MUX_SDM_DINL, 1); // AB1_R
            }

            if (ret != AIO_OK)
            {
                line = __LINE__;
                goto FAIL;
            }

            break;

        case E_AO_CH_ECHO_A_1:

            ret |= HalAudSetMux(E_AUD_MUX_MMC1, 1);
            if (ret != AIO_OK)
            {
                line = __LINE__;
                goto FAIL;
            }

            if (nDmaCh == 0)
            {
                ret |= HalAudSetMux(E_AUD_MUX_SDM_DINR, 1); // AB1_L
            }
            else
            {
                ret |= HalAudSetMux(E_AUD_MUX_SDM_DINR, 0); // AB1_R
            }

            if (ret != AIO_OK)
            {
                line = __LINE__;
                goto FAIL;
            }

            break;

        case E_AO_CH_I2S_TX_A_0:
        case E_AO_CH_I2S_TX_A_1:

            ret |= HalAudSetMux(E_AUD_MUX_I2S_CFG_00, 1); // 150378[2:1] I2S Tx data sel 1:RDMA1 2:RDMA2 3:RDMA3
            if (ret != AIO_OK)
            {
                line = __LINE__;
                goto FAIL;
            }

            ret |= HalAudSetMux(E_AUD_MUX_I2STDM_RX, 1); // 150378[0] pad sel 0:ext l2s trx 1:tdm trx
            if (ret != AIO_OK)
            {
                line = __LINE__;
                goto FAIL;
            }

            ret |= HalAudSetMux(E_AUD_MUX_TDM1_TX, 1); // 150378[3] pad sel 0:simple i2s 1:dma valid
            if (ret != AIO_OK)
            {
                line = __LINE__;
                goto FAIL;
            }

            ret |= HalAudSetMux(E_AUD_MUX_DMARD1_TDM_SEL, 0); // 15039A[7:6] RDMA1 I2STX Sel 0:TDM0 1:TDM1
            if (ret != AIO_OK)
            {
                line = __LINE__;
                goto FAIL;
            }

            break;

        case E_AO_CH_I2S_TX_B_0:
        case E_AO_CH_I2S_TX_B_1:

            ret |= HalAudSetMux(E_AUD_MUX_I2S_1_CFG_00, 1); // 150578[2:1] I2S1 Tx data sel 1:RDMA1 2:RDMA2 3:RDMA3
            if (ret != AIO_OK)
            {
                line = __LINE__;
                goto FAIL;
            }

            ret |= HalAudSetMux(E_AUD_MUX_DMARD1_TDM_SEL, 1); // 15039A[7:6] RDMA1 I2STX Sel 0:TDM0 1:TDM1
            if (ret != AIO_OK)
            {
                line = __LINE__;
                goto FAIL;
            }

            ret |= HalAudSetMux(E_AUD_MUX_TDM2_TX, 1); // 150578[3] pad sel 0:simple i2s 1:dma valid
            if (ret != AIO_OK)
            {
                line = __LINE__;
                goto FAIL;
            }

            break;

        default:
            line = __LINE__;
            goto FAIL;
            break;
    }

    return AIO_OK;

FAIL:
    ERRMSG("Func:%s, Line:%d eAoCh %d Fail !\n", __FUNCTION__, line, eAoCh);

    return AIO_NG;
}

static int _HalAudSetAoPathCoreDmaB(U8 nDmaCh, AO_CH_e eAoCh)
{
    int ret  = AIO_OK;
    int line = 0;

    switch (eAoCh)
    {
        case E_AO_CH_DAC_A_0:
        case E_AO_CH_DAC_B_0:
        case E_AO_CH_DAC_C_0:
        case E_AO_CH_DAC_D_0:
        case E_AO_CH_ECHO_A_0:
        case E_AO_CH_ECHO_A_1:

            ERRMSG("Func:%s, Line:%d don't support this audio path !\n", __FUNCTION__, __LINE__);
            line = __LINE__;
            goto FAIL;
            break;

        case E_AO_CH_HDMI_A_0:
        case E_AO_CH_HDMI_A_1:

            ret |= HalAudSetMux(E_AUD_MUX_AO_HDMI, 1);
            if (ret != AIO_OK)
            {
                line = __LINE__;
                goto FAIL;
            }

            break;

        case E_AO_CH_I2S_TX_A_0:
        case E_AO_CH_I2S_TX_A_1:

            ret |= HalAudSetMux(E_AUD_MUX_I2S_CFG_00, 2); // 150378[2:1] 1:RDMA1 2:RDMA2 3:RDMA3
            if (ret != AIO_OK)
            {
                line = __LINE__;
                goto FAIL;
            }

            ret |= HalAudSetMux(E_AUD_MUX_DMARD2_TDM_SEL, 0); // 15039A[5:4] RDMA2 I2STX Sel 0:TDM0 1:TDM1
            if (ret != AIO_OK)
            {
                line = __LINE__;
                goto FAIL;
            }

            ret |= HalAudSetMux(E_AUD_MUX_TDM1_TX, 1); // 150378[3] pad sel 0:simple i2s 1:dma valid
            if (ret != AIO_OK)
            {
                line = __LINE__;
                goto FAIL;
            }

            // TDM0 need to enable this bit TDM RX OR TX BOTH NEED Enable this bit
            ret |= HalAudSetMux(E_AUD_MUX_I2STDM_RX, 1); // 150378[0] pad sel 0:ext l2s trx 1:tdm trx
            if (ret != AIO_OK)
            {
                line = __LINE__;
                goto FAIL;
            }

            break;

        case E_AO_CH_I2S_TX_B_0:
        case E_AO_CH_I2S_TX_B_1:

            // extern I2S no use ?
            ret |= HalAudSetMux(E_AUD_MUX_I2S_1_CFG_00, 2); // 150578[2:1] I2S1 Tx data sel 1:RDMA1 2:RDMA2 3:RDMA3
            if (ret != AIO_OK)
            {
                line = __LINE__;
                goto FAIL;
            }

            ret |= HalAudSetMux(E_AUD_MUX_DMARD2_TDM_SEL, 1); // 15039A[5:4] RDMA2 I2STX Sel 0:TDM0 1:TDM1
            if (ret != AIO_OK)
            {
                line = __LINE__;
                goto FAIL;
            }

            ret |= HalAudSetMux(E_AUD_MUX_TDM2_TX, 1); // 150578[3] pad sel 0:simple i2s 1:dma valid
            if (ret != AIO_OK)
            {
                line = __LINE__;
                goto FAIL;
            }
            break;

        default:
            line = __LINE__;
            goto FAIL;
            break;
    }

    return AIO_OK;

FAIL:
    ERRMSG("Func:%s, Line:%d eAoCh %d Fail !\n", __FUNCTION__, line, eAoCh);

    return AIO_NG;
}

static int _HalAudSetAoPathCoreDmaC(U8 nDmaCh, AO_CH_e eAoCh)
{
    int ret  = AIO_OK;
    int line = 0;

    switch (eAoCh)
    {
        case E_AO_CH_DAC_A_0:
        case E_AO_CH_DAC_B_0:
        case E_AO_CH_DAC_C_0:
        case E_AO_CH_DAC_D_0:
        case E_AO_CH_ECHO_A_0:
        case E_AO_CH_ECHO_A_1:

            ERRMSG("Func:%s, Line:%d don't support this audio path !\n", __FUNCTION__, __LINE__);
            line = __LINE__;
            goto FAIL;
            break;

        case E_AO_CH_HDMI_A_0:
        case E_AO_CH_HDMI_A_1:

            ret |= HalAudSetMux(E_AUD_MUX_AO_HDMI, 2); // 150578[2:1] 1:DMA1 2:DMA2 3:DMA3
            if (ret != AIO_OK)
            {
                line = __LINE__;
                goto FAIL;
            }

            break;
        case E_AO_CH_I2S_TX_A_0:
        case E_AO_CH_I2S_TX_A_1:

            ret |= HalAudSetMux(E_AUD_MUX_I2S_CFG_00, 3); // 150378[2:1] I2S0 Sel 1:RDMA1 2:RDMA2 3:RDMA3
            if (ret != AIO_OK)
            {
                line = __LINE__;
                goto FAIL;
            }

            ret |= HalAudSetMux(E_AUD_MUX_TDM1_TX, 1); // 150378[3] pad sel 0:simple i2s 1:dma valid
            if (ret != AIO_OK)
            {
                line = __LINE__;
                goto FAIL;
            }

            ret |= HalAudSetMux(E_AUD_MUX_I2STDM_RX, 1); // 150378[0] pad sel 0:ext l2s trx 1:tdm trx
            if (ret != AIO_OK)
            {
                line = __LINE__;
                goto FAIL;
            }

            break;

        case E_AO_CH_I2S_TX_B_0:
        case E_AO_CH_I2S_TX_B_1:

            ret |= HalAudSetMux(E_AUD_MUX_I2S_1_CFG_00, 3); // 150578[2:1] I2S1 Tx data sel 1:RDMA1 2:RDMA2 3:RDMA3
            if (ret != AIO_OK)
            {
                line = __LINE__;
                goto FAIL;
            }

            ret |= HalAudSetMux(E_AUD_MUX_TDM2_TX, 1); // 150578[3] pad sel 0:simple i2s 1:dma valid
            if (ret != AIO_OK)
            {
                line = __LINE__;
                goto FAIL;
            }

            break;

        default:
            line = __LINE__;
            goto FAIL;
            break;
    }

    return AIO_OK;

FAIL:
    ERRMSG("Func:%s, Line:%d eAoCh %d Fail !\n", __FUNCTION__, line, eAoCh);

    return AIO_NG;
}

static int _HalAudDmaWrConfigMchCore(EN_CHIP_AI_MCH eMchSet, U16 nChNum, AudMchClkRef_e eMchClkRef,
                                     AudMchSel_e *mch_sel)
{
    U16 nChNumSetting = 0, nClkRefSetting = 0, nChSel01 = 0, nChSel23 = 0, nChSel45 = 0, nChSel67 = 0;
    int ret = AIO_OK;

    //
    if (nChNum == 1)
    {
        nChNumSetting = 0;
    }
    else if (nChNum == 2)
    {
        nChNumSetting = 1;
    }
    else if (nChNum == 4)
    {
        nChNumSetting = 2;
    }
    else if (nChNum == 8)
    {
        nChNumSetting = 3;
    }
    else
    {
        ERRMSG("Func:%s, Line:%d nChNum %d Fail !\n", __FUNCTION__, __LINE__, nChNum);
        goto FAIL;
    }

    //
    switch (eMchClkRef)
    {
        case E_AUD_MCH_CLK_REF_DMIC:
            nClkRefSetting = 0;
            break;
        case E_AUD_MCH_CLK_REF_ADC:
            nClkRefSetting = 1;
            break;
        case E_AUD_MCH_CLK_REF_I2S_TDM_RX:
            nClkRefSetting = 2;
            break;
        case E_AUD_MCH_CLK_REF_SRC:
            nClkRefSetting = 3;
            break;
        default:
            ERRMSG("Func:%s, Line:%d eMchClkRef %d Fail !\n", __FUNCTION__, __LINE__, eMchClkRef);
            goto FAIL;
            break;
    }

    //
    ret |= _HalAudDmaGetMchSelConfigValue(mch_sel[E_AUD_MCH_CH_BIND_01], &nChSel01, eMchSet);
    ret |= _HalAudDmaGetMchSelConfigValue(mch_sel[E_AUD_MCH_CH_BIND_23], &nChSel23, eMchSet);
    ret |= _HalAudDmaGetMchSelConfigValue(mch_sel[E_AUD_MCH_CH_BIND_45], &nChSel45, eMchSet);
    ret |= _HalAudDmaGetMchSelConfigValue(mch_sel[E_AUD_MCH_CH_BIND_67], &nChSel67, eMchSet);
    if (ret != AIO_OK)
    {
        ERRMSG("Func:%s, Line:%d GetMchSelConfigValue() Fail !\n", __FUNCTION__, __LINE__);
        goto FAIL;
    }

    switch (eMchSet)
    {
        case E_CHIP_AI_MCH_A:
            HalBachWriteReg(E_BACH_REG_BANK2, E_BACH_DMA_SRC_CFG03, BACH_DMA_WR1_CH67_SEL_MSK,
                            (nChSel67 << BACH_DMA_WR1_CH67_SEL_POS));
            HalBachWriteReg(E_BACH_REG_BANK2, E_BACH_DMA_SRC_CFG03, BACH_DMA_WR1_CH45_SEL_MSK,
                            (nChSel45 << BACH_DMA_WR1_CH45_SEL_POS));
            HalBachWriteReg(E_BACH_REG_BANK2, E_BACH_DMA_SRC_CFG00, BACH_DMA_WR_CH23_SEL_MSK,
                            (nChSel23 << BACH_DMA_WR_CH23_SEL_POS));
            HalBachWriteReg(E_BACH_REG_BANK2, E_BACH_DMA_SRC_CFG00, BACH_DMA_WR_CH01_SEL_MSK,
                            (nChSel01 << BACH_DMA_WR_CH01_SEL_POS));
            HalBachWriteReg(E_BACH_REG_BANK2, E_BACH_DMA_SRC_CFG00, BACH_DMA_WR_VALID_SEL_MSK,
                            (nClkRefSetting << BACH_DMA_WR_VALID_SEL_POS));
            HalBachWriteReg(E_BACH_REG_BANK2, E_BACH_DMA_SRC_CFG00, BACH_DMA_WR_CH_MODE_MSK,
                            (nChNumSetting << BACH_DMA_WR_CH_MODE_POS));
            break;

        case E_CHIP_AI_MCH_B:
            HalBachWriteReg(E_BACH_REG_BANK2, E_BACH_DMA_SRC_CFG03, BACH_DMA_WR2_CH67_SEL_MSK,
                            (nChSel67 << BACH_DMA_WR2_CH67_SEL_POS));
            HalBachWriteReg(E_BACH_REG_BANK2, E_BACH_DMA_SRC_CFG03, BACH_DMA_WR2_CH45_SEL_MSK,
                            (nChSel45 << BACH_DMA_WR2_CH45_SEL_POS));
            HalBachWriteReg(E_BACH_REG_BANK2, E_BACH_DMA_SRC_CFG01, BACH_DMA_WR_CH23_SEL_MSK,
                            (nChSel23 << BACH_DMA_WR_CH23_SEL_POS));
            HalBachWriteReg(E_BACH_REG_BANK2, E_BACH_DMA_SRC_CFG01, BACH_DMA_WR_CH01_SEL_MSK,
                            (nChSel01 << BACH_DMA_WR_CH01_SEL_POS));
            HalBachWriteReg(E_BACH_REG_BANK2, E_BACH_DMA_SRC_CFG01, BACH_DMA_WR_VALID_SEL_MSK,
                            (nClkRefSetting << BACH_DMA_WR_VALID_SEL_POS));
            HalBachWriteReg(E_BACH_REG_BANK2, E_BACH_DMA_SRC_CFG01, BACH_DMA_WR_CH_MODE_MSK,
                            (nChNumSetting << BACH_DMA_WR_CH_MODE_POS));
            break;

        case E_CHIP_AI_MCH_C:
            HalBachWriteReg(E_BACH_REG_BANK4, E_BACH_DMA3_SRC_CFG00, BACH_DMA_WR3_CH67_SEL_MSK,
                            (nChSel67 << BACH_DMA_WR3_CH67_SEL_POS));
            HalBachWriteReg(E_BACH_REG_BANK4, E_BACH_DMA3_SRC_CFG00, BACH_DMA_WR3_CH45_SEL_MSK,
                            (nChSel45 << BACH_DMA_WR3_CH45_SEL_POS));
            HalBachWriteReg(E_BACH_REG_BANK4, E_BACH_DMA3_SRC_CFG00, BACH_DMA_WR3_CH23_SEL_MSK,
                            (nChSel23 << BACH_DMA_WR3_CH23_SEL_POS));
            HalBachWriteReg(E_BACH_REG_BANK4, E_BACH_DMA3_SRC_CFG00, BACH_DMA_WR3_CH01_SEL_MSK,
                            (nChSel01 << BACH_DMA_WR3_CH01_SEL_POS));
            HalBachWriteReg(E_BACH_REG_BANK4, E_BACH_DMA3_SRC_CFG01, BACH_DMA_WR3_VALID_SEL_MSK,
                            (nClkRefSetting << BACH_DMA_WR3_VALID_SEL_POS));
            HalBachWriteReg(E_BACH_REG_BANK4, E_BACH_DMA3_SRC_CFG01, BACH_DMA_WR3_CH_MODE_MSK,
                            (nChNumSetting << BACH_DMA_WR3_CH_MODE_POS));
            break;

        case E_CHIP_AI_MCH_D:
            HalBachWriteReg(E_BACH_REG_BANK4, E_BACH_DMA4_SRC_CFG00, BACH_DMA_WR3_CH67_SEL_MSK,
                            (nChSel67 << BACH_DMA_WR3_CH67_SEL_POS));
            HalBachWriteReg(E_BACH_REG_BANK4, E_BACH_DMA4_SRC_CFG00, BACH_DMA_WR3_CH45_SEL_MSK,
                            (nChSel45 << BACH_DMA_WR3_CH45_SEL_POS));
            HalBachWriteReg(E_BACH_REG_BANK4, E_BACH_DMA4_SRC_CFG00, BACH_DMA_WR3_CH23_SEL_MSK,
                            (nChSel23 << BACH_DMA_WR3_CH23_SEL_POS));
            HalBachWriteReg(E_BACH_REG_BANK4, E_BACH_DMA4_SRC_CFG00, BACH_DMA_WR3_CH01_SEL_MSK,
                            (nChSel01 << BACH_DMA_WR3_CH01_SEL_POS));
            HalBachWriteReg(E_BACH_REG_BANK4, E_BACH_DMA4_SRC_CFG01, BACH_DMA_WR3_VALID_SEL_MSK,
                            (nClkRefSetting << BACH_DMA_WR3_VALID_SEL_POS));
            HalBachWriteReg(E_BACH_REG_BANK4, E_BACH_DMA4_SRC_CFG01, BACH_DMA_WR3_CH_MODE_MSK,
                            (nChNumSetting << BACH_DMA_WR3_CH_MODE_POS));
            break;

        case E_CHIP_AI_MCH_E:
            HalBachWriteReg(E_BACH_REG_BANK4, E_BACH_DMA5_SRC_CFG00, BACH_DMA_WR3_CH67_SEL_MSK,
                            (nChSel67 << BACH_DMA_WR3_CH67_SEL_POS));
            HalBachWriteReg(E_BACH_REG_BANK4, E_BACH_DMA5_SRC_CFG00, BACH_DMA_WR3_CH45_SEL_MSK,
                            (nChSel45 << BACH_DMA_WR3_CH45_SEL_POS));
            HalBachWriteReg(E_BACH_REG_BANK4, E_BACH_DMA5_SRC_CFG00, BACH_DMA_WR3_CH23_SEL_MSK,
                            (nChSel23 << BACH_DMA_WR3_CH23_SEL_POS));
            HalBachWriteReg(E_BACH_REG_BANK4, E_BACH_DMA5_SRC_CFG00, BACH_DMA_WR3_CH01_SEL_MSK,
                            (nChSel01 << BACH_DMA_WR3_CH01_SEL_POS));
            HalBachWriteReg(E_BACH_REG_BANK4, E_BACH_DMA5_SRC_CFG01, BACH_DMA_WR3_VALID_SEL_MSK,
                            (nClkRefSetting << BACH_DMA_WR3_VALID_SEL_POS));
            HalBachWriteReg(E_BACH_REG_BANK4, E_BACH_DMA5_SRC_CFG01, BACH_DMA_WR3_CH_MODE_MSK,
                            (nChNumSetting << BACH_DMA_WR3_CH_MODE_POS));
            break;

        default:
            ERRMSG("Func:%s, Line:%d eMchSet %d Fail !\n", __FUNCTION__, __LINE__, eMchSet);
            goto FAIL;
            break;
    }

    return AIO_OK;

FAIL:

    return AIO_NG;
}

static void _HalAudKeepDpgaGainCompatible(AudRate_e eRate)
{
    // SRC:
    // 48K: 0dB
    // Others: +1.5dB
    if (eRate == E_AUD_RATE_48K)
    {
        // SRC
        HalAudDpgaSetGainOffset(E_CHIP_DPGA_E, 0, 0);
        HalAudDpgaSetGainOffset(E_CHIP_DPGA_E, 0, 1);
    }
    else
    {
        // SRC
        HalAudDpgaSetGainOffset(E_CHIP_DPGA_E, -3, 0);
        HalAudDpgaSetGainOffset(E_CHIP_DPGA_E, -3, 1);
    }
}

static int _HalAudAoaSelectDpgaByIf(AO_CH_e eAoCh, int *dpga)
{
    switch (eAoCh)
    {
        case E_AO_CH_DAC_A_0:
        case E_AO_CH_DAC_B_0:
        case E_AO_CH_ECHO_A_0:
        case E_AO_CH_ECHO_A_1:
            *dpga = E_CHIP_DPGA_A;
            break;

        case E_AO_CH_HDMI_A_0:
        case E_AO_CH_HDMI_A_1:
            *dpga = E_CHIP_DPGA_I;
            break;

        case E_AO_CH_I2S_TX_A_0:
        case E_AO_CH_I2S_TX_A_1:
            *dpga = E_CHIP_DPGA_G;
            break;

        case E_AO_CH_I2S_TX_B_0:
        case E_AO_CH_I2S_TX_B_1:
            *dpga = E_CHIP_DPGA_H;
            break;

        default:
            ERRMSG("Func:%s, Line:%d DpgaGain can't support the set !\n", __FUNCTION__, __LINE__);
            return AIO_NG;
            break;
    }

    return AIO_OK;
}

static int _HalAudAobSelectDpgaByIf(AO_CH_e eAoCh, int *dpga)
{
    switch (eAoCh)

    {
        case E_AO_CH_HDMI_A_0:
        case E_AO_CH_HDMI_A_1:
            *dpga = E_CHIP_DPGA_I;
            break;

        case E_AO_CH_I2S_TX_A_0:
        case E_AO_CH_I2S_TX_A_1:
            *dpga = E_CHIP_DPGA_G;
            break;

        case E_AO_CH_I2S_TX_B_0:
        case E_AO_CH_I2S_TX_B_1:
            *dpga = E_CHIP_DPGA_H;
            break;

        default:
            ERRMSG("Func:%s, Line:%d DpgaGain can't support the set !\n", __FUNCTION__, __LINE__);
            return AIO_NG;
            break;
    }

    return AIO_OK;
}

static int _HalAudAoDirectSelectDpgaByIf(AO_CH_e eAoCh, int *dpga)
{
    switch (eAoCh)
    {
        case E_AO_CH_DAC_A_0:
        case E_AO_CH_DAC_B_0:
            *dpga = E_CHIP_DPGA_A;
            break;
        default:
            ERRMSG("Func:%s, Line:%d DpgaGain can't support the set !\n", __FUNCTION__, __LINE__);
            return AIO_NG;
            break;
    }

    return AIO_OK;
}

static int _HalAudAocSelectDpgaByIf(AO_CH_e eAoCh, int *dpga)
{
    switch (eAoCh)

    {
        case E_AO_CH_HDMI_A_0:
        case E_AO_CH_HDMI_A_1:
            *dpga = E_CHIP_DPGA_I;
            break;

        case E_AO_CH_I2S_TX_A_0:
        case E_AO_CH_I2S_TX_A_1:
            *dpga = E_CHIP_DPGA_G;
            break;

        case E_AO_CH_I2S_TX_B_0:
        case E_AO_CH_I2S_TX_B_1:
            *dpga = E_CHIP_DPGA_H;
            break;

        default:
            ERRMSG("Func:%s, Line:%d DpgaGain can't support the set !\n", __FUNCTION__, __LINE__);
            return AIO_NG;
            break;
    }

    return AIO_OK;
}

static void _HalAudPllSet(void)
{
    // aupll
    HalBachWriteRegByte(0x00111b12, 0xff, 0x00);
    HalBachWriteRegByte(0x00141d46, 0xff, 0x19);
    HalBachWriteRegByte(0x00141d4e, 0xff, 0xbc);
    HalBachWriteRegByte(0x00141d00, 0xff, 0x00);
    HalBachWriteRegByte(0x00141d4a, 0xff, 0xa7);
    HalBachWriteRegByte(0x00141d4c, 0xff, 0x80);
    HalBachWriteRegByte(0x00141d4d, 0xff, 0x81);
    // new add for U02
    HalBachWriteRegByte(0x00141d4b, 0xff, 0x40);
#if 1
    // upll
    HalBachWriteRegByte(0x00142044, 0xff, 0xc0);
    HalBachWriteRegByte(0x00142045, 0xff, 0x02);
    HalBachWriteRegByte(0x0014204e, 0xff, 0xbc);
    HalBachWriteRegByte(0x00142040, 0xff, 0x24);
    HalBachWriteRegByte(0x00142041, 0xff, 0x8f);
    HalBachWriteRegByte(0x00142042, 0xff, 0x2b);
    HalBachWriteRegByte(0x00142000, 0xff, 0x00);
    HalBachWriteRegByte(0x00142001, 0xff, 0x00);
    HalBachWriteRegByte(0x00103fd0, 0xff, 0x10);
#endif
}

static BOOL _HalAudDacSetMute(AO_CH_e eAoCh, BOOL bEnable)
{
    U16 pos, msk;

    switch (eAoCh)
    {
        case E_AO_CH_DAC_A_0:
            pos = REG_DAC_DIN_L_SEL_POS;
            msk = REG_DAC_DIN_L_SEL_MSK;
            break;
        case E_AO_CH_DAC_B_0:
            pos = REG_DAC_DIN_R_SEL_POS;
            msk = REG_DAC_DIN_R_SEL_MSK;
            break;

        default:
            ERRMSG("Function - %s # %d - AudDacSel_e = %d, error !] \n", __func__, __LINE__, (int)eAoCh);
            return FALSE;
    }
    if (bEnable)
    {
        HalBachWriteReg(E_BACH_REG_BANK1, E_BACH_MIX1_SEL, msk, 0 << pos);
    }
    else
    {
        HalBachWriteReg(E_BACH_REG_BANK1, E_BACH_MIX1_SEL, msk, 1 << pos);
    }
    return TRUE;
}

//------------------------------------------------------------------------------
BOOL HalAudDigMicBckMode(U8 uMode8K, U8 uMode16K, U8 uMode32K, U8 uMode48K)
{
    g_nDmicBckMode8K  = uMode8K;
    g_nDmicBckMode16K = uMode16K;
    g_nDmicBckMode32K = uMode32K;
    g_nDmicBckMode48K = uMode48K;

    return TRUE;
}

int HalAudMainInit(void)
{
    _HalAudPllInit();
    _HalAudSysInit();
    _HalAudDmicRegInit();
    _HalAudDmaFirstInit();
    _HalAudDmaWrMchInit();
    _HalAudDpgaInit();
    _HalAudIntEnable();

    _HalAudPllSet();

    return AIO_OK;
}

int HalAudMainDeInit(void)
{
    g_bAdcActive = 0;
    g_bDacActive = 0;

    return AIO_OK;
}

int HalAudSetAiPath(CHIP_AIO_DMA_e eAioDma, U8 nDmaCh, AI_CH_e eAiCh)
{
    int ret = AIO_OK;

    switch (eAioDma)
    {
        case E_CHIP_AIO_DMA_AI_A:
            ret |= _HalAudSetAiPathCoreDmaA(nDmaCh, eAiCh);
            break;

        case E_CHIP_AIO_DMA_AI_B:
            ret |= _HalAudSetAiPathCoreDmaB(nDmaCh, eAiCh);
            break;

        case E_CHIP_AIO_DMA_AI_C:
            ret |= _HalAudSetAiPathCoreDmaC(nDmaCh, eAiCh);
            break;

        case E_CHIP_AIO_DMA_AI_D:
            ret |= _HalAudSetAiPathCoreDmaD(nDmaCh, eAiCh);
            break;

        case E_CHIP_AIO_DMA_AI_E:
            ret |= _HalAudSetAiPathCoreDmaE(nDmaCh, eAiCh);
            break;

        default:
            ERRMSG("Func:%s, Line:%d eAioDma %d Fail !\n", __FUNCTION__, __LINE__, eAioDma);

            goto FAIL;
            break;
    }

    if (ret != AIO_OK)
    {
        ERRMSG("Func:%s, Line:%d eAioDma %d Fail !\n", __FUNCTION__, __LINE__, eAioDma);
        goto FAIL;
    }

    return AIO_OK;

FAIL:

    return AIO_NG;
}

int HalAudSetAoPath(CHIP_AIO_DMA_e eAioDma, U8 nDmaCh, AO_CH_e eAoCh)
{
    int ret = AIO_OK;

    switch (eAioDma)
    {
        case E_CHIP_AIO_DMA_AO_A:
            ret |= _HalAudSetAoPathCoreDmaA(nDmaCh, eAoCh);
            break;

        case E_CHIP_AIO_DMA_AO_B:
            ret |= _HalAudSetAoPathCoreDmaB(nDmaCh, eAoCh);
            break;

        case E_CHIP_AIO_DMA_AO_C:
            ret |= _HalAudSetAoPathCoreDmaC(nDmaCh, eAoCh);
            break;

        default:
            ERRMSG("Func:%s, Line:%d eAioDma %d Fail !\n", __FUNCTION__, __LINE__, eAioDma);
            goto FAIL;
            break;
    }

    if (ret != AIO_OK)
    {
        ERRMSG("Func:%s, Line:%d eAioDma %d Fail !\n", __FUNCTION__, __LINE__, eAioDma);
        goto FAIL;
    }

    return AIO_OK;

FAIL:

    return AIO_NG;
}

int HalAudAoDetach(AO_CH_e aoch)
{
    switch (aoch)
    {
        case E_AO_CH_DAC_A_0:
        case E_AO_CH_ECHO_A_0:
            HalBachWriteReg(E_BACH_REG_BANK1, E_BACH_MUX4_SEL, REG_SDM_DINL_SEL_MSK, (7 << REG_SDM_DINL_SEL_POS));
            break;
        case E_AO_CH_DAC_B_0:
        case E_AO_CH_ECHO_A_1:
            HalBachWriteReg(E_BACH_REG_BANK1, E_BACH_MUX4_SEL, REG_SDM_DINR_SEL_MSK, (7 << REG_SDM_DINR_SEL_POS));
            break;
        default:
            ERRMSG("HalAudAoDetach aoch[%d] not support now\n", aoch);
            goto FAIL;
            break;
    }

    return AIO_OK;
FAIL:
    return AIO_NG;
}

int HalAudSetDirectPath(AI_CH_e eAiCh, AO_CH_e eAoCh)
{
    int ret  = AIO_OK;
    int line = 0;

    switch (eAiCh)
    {
        case E_AI_CH_ADC_A_0:
            switch (eAoCh)
            {
                case E_AO_CH_DAC_A_0:
                    ret |= HalAudSetMux(E_AUD_MUX_DMAWR1, 1);
                    if (ret != AIO_OK)
                    {
                        line = __LINE__;
                        goto FAIL;
                    }

                    ret |= HalAudSetMux(E_AUD_MUX_MMC1, 0);
                    if (ret != AIO_OK)
                    {
                        line = __LINE__;
                        goto FAIL;
                    }

                    ret |= HalAudSetMux(E_AUD_MUX_SDM_DINL, 0);
                    if (ret != AIO_OK)
                    {
                        line = __LINE__;
                        goto FAIL;
                    }
                    break;

                case E_AO_CH_DAC_B_0:
                    ret |= HalAudSetMux(E_AUD_MUX_DMAWR1, 1);
                    if (ret != AIO_OK)
                    {
                        line = __LINE__;
                        goto FAIL;
                    }

                    ret |= HalAudSetMux(E_AUD_MUX_MMC1, 0);
                    if (ret != AIO_OK)
                    {
                        line = __LINE__;
                        goto FAIL;
                    }

                    ret |= HalAudSetMux(E_AUD_MUX_SDM_DINR, 1);
                    if (ret != AIO_OK)
                    {
                        line = __LINE__;
                        goto FAIL;
                    }
                    break;

                default:
                    line = __LINE__;
                    goto FAIL;
                    break;
            }
            break;
        case E_AI_CH_ADC_B_0:
            switch (eAoCh)
            {
                case E_AO_CH_DAC_A_0:
                    ret |= HalAudSetMux(E_AUD_MUX_DMAWR1, 1);
                    if (ret != AIO_OK)
                    {
                        line = __LINE__;
                        goto FAIL;
                    }

                    ret |= HalAudSetMux(E_AUD_MUX_MMC1, 0);
                    if (ret != AIO_OK)
                    {
                        line = __LINE__;
                        goto FAIL;
                    }

                    ret |= HalAudSetMux(E_AUD_MUX_SDM_DINL, 1);
                    if (ret != AIO_OK)
                    {
                        line = __LINE__;
                        goto FAIL;
                    }
                    break;

                case E_AO_CH_DAC_B_0:
                    ret |= HalAudSetMux(E_AUD_MUX_DMAWR1, 1);
                    if (ret != AIO_OK)
                    {
                        line = __LINE__;
                        goto FAIL;
                    }

                    ret |= HalAudSetMux(E_AUD_MUX_MMC1, 0);
                    if (ret != AIO_OK)
                    {
                        line = __LINE__;
                        goto FAIL;
                    }

                    ret |= HalAudSetMux(E_AUD_MUX_SDM_DINR, 0);
                    if (ret != AIO_OK)
                    {
                        line = __LINE__;
                        goto FAIL;
                    }
                    break;

                default:
                    line = __LINE__;
                    goto FAIL;
                    break;
            }

            break;
        default:
            line = __LINE__;
            goto FAIL;
            break;
    }

    //
    if (ret != AIO_OK)
    {
        line = __LINE__;
        goto FAIL;
    }

    return AIO_OK;

FAIL:
    ERRMSG("Func:%s, Line:%d eAiCh %d eAoCh %d Fail !\n", __FUNCTION__, line, eAiCh, eAoCh);

    return AIO_NG;
}

int HalAudSetMux(AudMux_e eMux, U8 nChoice)
{
    switch (eMux)
    {
        case E_AUD_MUX_MMC1:
            HalBachWriteReg(E_BACH_REG_BANK1, E_BACH_MUX0_SEL, REG_MMC1_SRC_SEL, (nChoice ? REG_MMC1_SRC_SEL : 0));
            break;

        case E_AUD_MUX_MMC2:
            HalBachWriteReg(E_BACH_REG_BANK1, E_BACH_MUX0_SEL, REG_MMC2_SRC_SEL, (nChoice ? REG_MMC2_SRC_SEL : 0));
            break;

        case E_AUD_MUX_DMARD1_TDM_SEL:
            HalBachWriteReg(E_BACH_REG_BANK2, E_BACH_DMA_SRC_CFG02, BACH_DMA_RD1_TDM_SEL_MSK,
                            nChoice << BACH_DMA_RD1_TDM_SEL_POS);
            break;

        case E_AUD_MUX_DMARD2_TDM_SEL:
            HalBachWriteReg(E_BACH_REG_BANK2, E_BACH_DMA_SRC_CFG02, BACH_DMA_RD2_TDM_SEL_MSK,
                            nChoice << BACH_DMA_RD2_TDM_SEL_POS);
            break;

        case E_AUD_MUX_DMAWR1: // combine two mux, now we only have SRC mode(0) & 32bit bypass mode(1)
            HalBachWriteReg(E_BACH_REG_BANK2, E_BACH_MUX1_SEL, MUX_ASRC_ADC_SEL, (nChoice ? MUX_ASRC_ADC_SEL : 0));
            break;

        case E_AUD_MUX_DMAWR1_MCH:
            HalBachWriteReg(E_BACH_REG_BANK2, E_BACH_DMA_SRC_CFG00, BACH_DMA_WR_NEW_MODE,
                            (nChoice ? BACH_DMA_WR_NEW_MODE : 0));
            break;

        case E_AUD_MUX_DMAWR2_MCH:
            HalBachWriteReg(E_BACH_REG_BANK2, E_BACH_DMA_SRC_CFG01, BACH_DMA_WR_NEW_MODE,
                            (nChoice ? BACH_DMA_WR_NEW_MODE : 0));
            break;

        case E_AUD_MUX_I2STDM_RX:
            HalBachWriteReg(E_BACH_REG_BANK2, E_BACH_I2S_CFG00, BACH_MUX2_SEL, (nChoice ? BACH_MUX2_SEL : 0));
            break;

        case E_AUD_MUX_TDM1_TX:
            HalBachWriteReg(E_BACH_REG_BANK2, E_BACH_I2S_CFG00, BACH_I2S_TDM_TX_MUX,
                            (nChoice ? BACH_I2S_TDM_TX_MUX : 0));
            break;

        case E_AUD_MUX_TDM2_TX:
            HalBachWriteReg(E_BACH_REG_BANK4, E_BACH_I2S1_CFG00, BACH_I2S_TDM2_TX_MUX,
                            (nChoice ? BACH_I2S_TDM2_TX_MUX : 0));
            break;

        case E_AUD_MUX_I2S_TX:
            HalBachWriteReg(E_BACH_REG_BANK2, E_BACH_AU_SYS_CTRL1, MUX_CODEC_TX_SEL_MSK,
                            nChoice << MUX_CODEC_TX_SEL_POS);
            break;

        case E_AUD_MUX_I2S_CFG_00:
            HalBachWriteReg(E_BACH_REG_BANK2, E_BACH_I2S_CFG00, BACH_DMA_RD2_PATH_MSK,
                            nChoice << BACH_DMA_RD2_PATH_POS);
            break;

        case E_AUD_MUX_I2S_1_CFG_00:
            HalBachWriteReg(E_BACH_REG_BANK4, E_BACH_I2S1_CFG00, BACH_DMA_RD2_PATH_MSK,
                            nChoice << BACH_DMA_RD2_PATH_POS);
            break;

        case E_AUD_MUX_SDM_DINL:
            HalBachWriteReg(E_BACH_REG_BANK1, E_BACH_MUX4_SEL, REG_SDM_DINL_SEL_MSK, nChoice << REG_SDM_DINL_SEL_POS);
            break;

        case E_AUD_MUX_SDM_DINR:
            HalBachWriteReg(E_BACH_REG_BANK1, E_BACH_MUX4_SEL, REG_SDM_DINR_SEL_MSK, nChoice << REG_SDM_DINR_SEL_POS);
            break;

        case E_AUD_MUX_AO_HDMI:
            HalBachWriteReg(E_BACH_REG_BANK1, E_BACH_AU2HDMI_CTRL, REG_AU2HDMI_MUX_SEL_MSK,
                            nChoice << REG_AU2HDMI_MUX_SEL_POS); // 0: RDMA1   1: RDMA2    2:RDMA3  3:SRC
            break;

        default:
            ERRMSG("Func:%s, Line:%d eMux %d Fail !\n", __FUNCTION__, __LINE__, eMux);
            goto FAIL;
            break;
    }

    return AIO_OK;

FAIL:

    return AIO_NG;
}

int HalAudSrcSetRate(AI_CH_e eAiCh, AudRate_e eRate)
{
    switch (eAiCh)
    {
        case E_AI_CH_ECHO_A_0:
        case E_AI_CH_ECHO_A_1:

            switch (eRate)
            {
                case E_AUD_RATE_8K:
                    HalBachWriteReg(E_BACH_REG_BANK1, E_BACH_SR0_SEL, REG_CIC_1_SEL_MSK, 0 << REG_CIC_1_SEL_POS);
                    break;

                case E_AUD_RATE_16K:
                    HalBachWriteReg(E_BACH_REG_BANK1, E_BACH_SR0_SEL, REG_CIC_1_SEL_MSK, 1 << REG_CIC_1_SEL_POS);
                    break;

                case E_AUD_RATE_32K:
                    HalBachWriteReg(E_BACH_REG_BANK1, E_BACH_SR0_SEL, REG_CIC_1_SEL_MSK, 2 << REG_CIC_1_SEL_POS);
                    break;

                case E_AUD_RATE_48K:
                    HalBachWriteReg(E_BACH_REG_BANK1, E_BACH_SR0_SEL, REG_CIC_1_SEL_MSK, 3 << REG_CIC_1_SEL_POS);
                    break;

                default:
                    ERRMSG("Func:%s, Line:%d eAiCh %d eRate %d Fail !\n", __FUNCTION__, __LINE__, eAiCh, eRate);
                    goto FAIL;
                    break;
            }

            break;

        default:
            ERRMSG("Func:%s, Line:%d eAiCh %d Fail !\n", __FUNCTION__, __LINE__, eAiCh);
            goto FAIL;
            break;
    }

    return AIO_OK;

FAIL:

    return AIO_NG;
}

int HalAudAtopOpen(CHIP_AIO_ATOP_e eAtop)
{
    int  ret           = AIO_OK;
    int  i             = 0;
    bool bIsAllAtopOff = TRUE;

    if (g_bAtopStatus[eAtop] == FALSE)
    {
        for (i = 0; i < E_CHIP_AIO_ATOP_TOTAL; i++)
        {
            if (g_bAtopStatus[i] == TRUE)
            {
                bIsAllAtopOff = FALSE;
                break;
            }
        }

        if (bIsAllAtopOff == TRUE)
        {
            ret |= _HalAudAtopEnableRef(TRUE);
            if (ret != AIO_OK)
            {
                ERRMSG("Func:%s, Line:%d AtopEnableRef Fail !\n", __FUNCTION__, __LINE__);
                goto FAIL;
            }
        }

        ret |= _HalAudAtopSwitch(eAtop, TRUE);
        if (ret != AIO_OK)
        {
            ERRMSG("Func:%s, Line:%d AtopSwitch Fail !\n", __FUNCTION__, __LINE__);
            goto FAIL;
        }

        g_bAtopStatus[eAtop] = TRUE;
    }

    return AIO_OK;

FAIL:

    return AIO_NG;
}

int HalAudAtopClose(CHIP_AIO_ATOP_e eAtop)
{
    int  ret           = AIO_OK;
    int  i             = 0;
    bool bIsAllAtopOff = TRUE;

    if (g_bAtopStatus[eAtop] == TRUE)
    {
        for (i = 0; i < E_CHIP_AIO_ATOP_TOTAL; i++)
        {
            if (g_bAtopStatus[i] == TRUE)
            {
                bIsAllAtopOff = FALSE;
                break;
            }
        }

        ret |= _HalAudAtopSwitch(eAtop, FALSE);
        if (ret != AIO_OK)
        {
            ERRMSG("Func:%s, Line:%d AtopSwitch Fail !\n", __FUNCTION__, __LINE__);
            goto FAIL;
        }

        if (bIsAllAtopOff == TRUE)
        {
            ret |= _HalAudAtopEnableRef(FALSE);
            if (ret != AIO_OK)
            {
                ERRMSG("Func:%s, Line:%d AtopEnableRef Fail !\n", __FUNCTION__, __LINE__);
                goto FAIL;
            }
        }

        g_bAtopStatus[eAtop] = FALSE;
    }

    return AIO_OK;

FAIL:

    return AIO_NG;
}

int HalAudAtopMicAmpGain(CHIP_AI_ADC_e eAdc, U16 nSel)
{
    U16 nRegMsk, nPos, nGain;
    U8  nOffset;

    switch (eAdc)
    {
        case E_CHIP_AI_ADC_A:
            nOffset = E_BACH_ANALOG_CTRL06;
            nRegMsk = REG_SEL_GAIN_INMUX_L_MSK;
            nPos    = REG_SEL_GAIN_INMUX_L_POS;
            break;

        case E_CHIP_AI_ADC_B:
            nOffset = E_BACH_ANALOG_CTRL06;
            nRegMsk = REG_SEL_GAIN_INMUX_R_MSK;
            nPos    = REG_SEL_GAIN_INMUX_R_POS;
            break;

        default:
            ERRMSG("Func:%s, Line:%d eAdc %d Fail !\n", __FUNCTION__, __LINE__, eAdc);
            goto FAIL;
            break;
    }

    if (nSel > 0xD)
    {
        ERRMSG("Func:%s, Line:%d eAdc %d nSel %d Fail !\n", __FUNCTION__, __LINE__, eAdc, nSel);
        goto FAIL;
    }

    nGain = nSel;
    HalBachWriteReg(E_BACH_REG_BANK5, nOffset, nRegMsk, (nGain << nPos));

    return AIO_OK;

FAIL:

    return AIO_NG;
}

int HalAudAtopSetAdcGain(CHIP_AI_ADC_e eAdc, U16 nSel)
{
    U16 nConfigValue, nRegMsk, nPos, nGain;
    U8  nOffset;

    switch (eAdc)
    {
        case E_CHIP_AI_ADC_A:
            nOffset      = E_BACH_ANALOG_CTRL08;
            nConfigValue = 0x0001;
            nRegMsk      = REG_SEL_MICGAIN_STG1_L_MSK;
            nPos         = REG_SEL_MICGAIN_STG1_L_POS;
            break;

        case E_CHIP_AI_ADC_B:
            nOffset      = E_BACH_ANALOG_CTRL08;
            nConfigValue = 0x0002;
            nRegMsk      = REG_SEL_MICGAIN_STG1_R_MSK;
            nPos         = REG_SEL_MICGAIN_STG1_R_POS;
            break;

        default:
            ERRMSG("Func:%s, Line:%d eAdc %d Fail !\n", __FUNCTION__, __LINE__, eAdc);
            goto FAIL;
            break;
    }

    if (nSel > 0x3)
    {
        ERRMSG("Func:%s, Line:%d eAdc %d nSel %d Fail !\n", __FUNCTION__, __LINE__, eAdc, nSel);
        goto FAIL;
    }

    nGain = nSel;

    HalBachWriteReg(E_BACH_REG_BANK5, nOffset, nRegMsk, (nGain << nPos));

    return AIO_OK;

FAIL:

    return AIO_NG;
}

int HalAudAtopSetAdcMux(CHIP_AI_ADC_e eAdc, CHIP_ADC_MUX_e eAdcMux)
{
    switch (eAdc)
    {
        case E_CHIP_AI_ADC_A:
            if (eAdcMux == E_CHIP_ADC_MUX_LINEIN)
            {
                HalBachWriteReg(E_BACH_REG_BANK5, E_BACH_ANALOG_CTRL05, REG_SEL_CH_INMUX_L_MSK,
                                0x0 << REG_SEL_CH_INMUX_L_POS);
            }
            else if (eAdcMux == E_CHIP_ADC_MUX_MICIN)
            {
                HalBachWriteReg(E_BACH_REG_BANK5, E_BACH_ANALOG_CTRL05, REG_SEL_CH_INMUX_L_MSK,
                                0x3 << REG_SEL_CH_INMUX_L_POS);
            }
            break;

        case E_CHIP_AI_ADC_B:
            if (eAdcMux == E_CHIP_ADC_MUX_LINEIN)
            {
                HalBachWriteReg(E_BACH_REG_BANK5, E_BACH_ANALOG_CTRL05, REG_SEL_CH_INMUX_R_MSK,
                                0x0 << REG_SEL_CH_INMUX_R_POS);
            }
            else if (eAdcMux == E_CHIP_ADC_MUX_MICIN)
            {
                HalBachWriteReg(E_BACH_REG_BANK5, E_BACH_ANALOG_CTRL05, REG_SEL_CH_INMUX_R_MSK,
                                0x3 << REG_SEL_CH_INMUX_R_POS);
            }
            break;

        default:
            ERRMSG("Func:%s, Line:%d eAdc %d Fail !\n", __FUNCTION__, __LINE__, eAdc);
            goto FAIL;
            break;
    }

    return AIO_OK;

FAIL:

    return AIO_NG;
}

int HalAudAtopSwap(BOOL bEn)
{
    if (bEn)
    {
        HalBachWriteReg(E_BACH_REG_BANK5, E_BACH_ANALOG_STAT03, REG_ADC_LR_SWAP, REG_ADC_LR_SWAP);
        HalBachWriteReg(E_BACH_REG_BANK5, E_BACH_ANALOG_STAT03, REG_ADC2_LR_SWAP, REG_ADC2_LR_SWAP);
    }
    else
    {
        HalBachWriteReg(E_BACH_REG_BANK5, E_BACH_ANALOG_STAT03, REG_ADC_LR_SWAP, ~REG_ADC_LR_SWAP);
        HalBachWriteReg(E_BACH_REG_BANK5, E_BACH_ANALOG_STAT03, REG_ADC2_LR_SWAP, ~REG_ADC2_LR_SWAP);
    }

    return AIO_OK;
}

int HalAudI2sSaveRxTdmWsPgm(BOOL bEnable)
{
    g_nI2sRxTdmWsPgm = (U8)bEnable;

    return AIO_OK;
}

int HalAudI2sSaveTdmWsPgm(AUDIO_TDM_e nTdm, BOOL bEnable)
{
    long *nI2sTdmWsPgm = NULL;

    switch (nTdm)
    {
        case E_AUDIO_TDM_RXA:
            nI2sTdmWsPgm = &g_nI2sTdmWsPgm[E_AUDIO_TDM_RXA];
            break;

        case E_AUDIO_TDM_RXB:
            nI2sTdmWsPgm = &g_nI2sTdmWsPgm[E_AUDIO_TDM_RXB];
            break;

        case E_AUDIO_TDM_RXC:
            nI2sTdmWsPgm = &g_nI2sTdmWsPgm[E_AUDIO_TDM_RXC];
            break;

        case E_AUDIO_TDM_RXD:
            nI2sTdmWsPgm = &g_nI2sTdmWsPgm[E_AUDIO_TDM_RXD];
            break;

        case E_AUDIO_TDM_TXA:
            nI2sTdmWsPgm = &g_nI2sTdmWsPgm[E_AUDIO_TDM_TXA];
            break;

        case E_AUDIO_TDM_TXB:
            nI2sTdmWsPgm = &g_nI2sTdmWsPgm[E_AUDIO_TDM_TXB];
            break;
        default:
            ERRMSG("Func:%s, Line:%d nTdm  %d Fail !\n", __FUNCTION__, __LINE__, nTdm);
            goto FAIL;
            break;
    }

    *nI2sTdmWsPgm = 0;

    *nI2sTdmWsPgm = bEnable;

    return AIO_OK;
FAIL:

    return AIO_NG;
}

int HalAudI2sGetRxTdmWsPgm(U8 *bEnable)
{
    *bEnable = g_nI2sRxTdmWsPgm;

    return AIO_OK;
}

long *HalAudI2sGetTdmWsPgm(void)
{
    return &g_nI2sTdmWsPgm[0];
}

int HalAudI2sSaveRxTdmWsWidth(U8 nWsWidth)
{
    g_nI2sRxTdmWsWidth = nWsWidth;

    return AIO_OK;
}

int HalAudI2sGetRxTdmWsWidth(U8 *nWsWidth)
{
    *nWsWidth = g_nI2sRxTdmWsWidth;

    return AIO_OK;
}

long *HalAudI2sGetTdmWsWidth(void)
{
    return &g_nI2sTdmWsWidth[0];
}

int HalAudI2sSaveRxTdmWsInv(BOOL bEnable)
{
    g_nI2sRxTdmWsInv = (U8)bEnable;

    return AIO_OK;
}

int HalAudI2sSaveTdmWsInv(AUDIO_TDM_e nTdm, BOOL bEnable)
{
    long *nI2sTdmWsInv = NULL;

    switch (nTdm)
    {
        case E_AUDIO_TDM_RXA:
            nI2sTdmWsInv = &g_nI2sTdmWsInv[E_AUDIO_TDM_RXA];
            break;

        case E_AUDIO_TDM_RXB:
            nI2sTdmWsInv = &g_nI2sTdmWsInv[E_AUDIO_TDM_RXB];
            break;

        case E_AUDIO_TDM_RXC:
            nI2sTdmWsInv = &g_nI2sTdmWsInv[E_AUDIO_TDM_RXC];
            break;

        case E_AUDIO_TDM_RXD:
            nI2sTdmWsInv = &g_nI2sTdmWsInv[E_AUDIO_TDM_RXD];
            break;

        case E_AUDIO_TDM_TXA:
            nI2sTdmWsInv = &g_nI2sTdmWsInv[E_AUDIO_TDM_TXA];
            break;

        case E_AUDIO_TDM_TXB:
            nI2sTdmWsInv = &g_nI2sTdmWsInv[E_AUDIO_TDM_TXB];
            break;

        default:
            ERRMSG("Func:%s, Line:%d nTdm  %d Fail !\n", __FUNCTION__, __LINE__, nTdm);
            goto FAIL;
            break;
    }

    *nI2sTdmWsInv = 0;

    *nI2sTdmWsInv = bEnable;

    return AIO_OK;
FAIL:

    return AIO_NG;
}

int HalAudI2sSaveTdmBckInv(AUDIO_TDM_e nTdm, BOOL bEnable)
{
    long *nI2sTdmBckInv = NULL;

    switch (nTdm)
    {
        case E_AUDIO_TDM_RXA:
            nI2sTdmBckInv = &g_nI2sTdmBckInv[E_AUDIO_TDM_RXA];
            break;

        case E_AUDIO_TDM_RXB:
            nI2sTdmBckInv = &g_nI2sTdmBckInv[E_AUDIO_TDM_RXB];
            break;

        case E_AUDIO_TDM_RXC:
            nI2sTdmBckInv = &g_nI2sTdmBckInv[E_AUDIO_TDM_RXC];
            break;

        case E_AUDIO_TDM_RXD:
            nI2sTdmBckInv = &g_nI2sTdmBckInv[E_AUDIO_TDM_RXD];
            break;

        case E_AUDIO_TDM_TXA:
            nI2sTdmBckInv = &g_nI2sTdmBckInv[E_AUDIO_TDM_TXA];
            break;

        case E_AUDIO_TDM_TXB:
            nI2sTdmBckInv = &g_nI2sTdmBckInv[E_AUDIO_TDM_TXB];
            break;

        default:
            ERRMSG("Func:%s, Line:%d nTdm  %d Fail !\n", __FUNCTION__, __LINE__, nTdm);
            goto FAIL;
            break;
    }

    *nI2sTdmBckInv = 0;

    *nI2sTdmBckInv = bEnable;

    return AIO_OK;
FAIL:

    return AIO_NG;
}

long *HalAudI2sGetTdmBckInv(void)
{
    return &g_nI2sTdmBckInv[0];
}

long *HalAudI2sGetRxMode(void)
{
    return &g_nI2sRxMode;
}

int HalAudI2sGetRxTdmWsInv(U8 *bEnable)
{
    *bEnable = g_nI2sRxTdmWsInv;

    return AIO_OK;
}

long *HalAudI2sGetTdmWsInv(void)
{
    return &g_nI2sTdmWsInv[0];
}

int HalAudI2sSaveRxTdmChSwap(U8 nSwap_0_2, U8 nSwap_0_4)
{
    g_nI2sRxTdmChSwap = 0;

    if (nSwap_0_2 != 0)
    {
        g_nI2sRxTdmChSwap |= 0x01;
    }

    if (nSwap_0_4 != 0)
    {
        g_nI2sRxTdmChSwap |= 0x02;
    }

    return AIO_OK;
}

int HalAudI2sGetRxTdmChSwap(U8 *nSwap)
{
    *nSwap = g_nI2sRxTdmChSwap;

    return AIO_OK;
}

long *HalAudI2sGetTdmChSwap(void)
{
    return &g_nI2sTdmChSwap[0];
}

int HalAudI2sSaveTxTdmWsPgm(BOOL bEnable)
{
    g_nI2sTxTdmWsPgm = (U8)bEnable;

    return AIO_OK;
}

int HalAudI2sGetTxTdmWsPgm(U8 *bEnable)
{
    *bEnable = g_nI2sTxTdmWsPgm;

    return AIO_OK;
}

int HalAudI2sSaveTxTdmWsWidth(U8 nWsWidth)
{
    g_nI2sTxTdmWsWidth = nWsWidth;

    return AIO_OK;
}

int HalAudI2sSaveTdmWsWidth(AUDIO_TDM_e nTdm, U8 nWsWidth)
{
    long *nI2sTdmWsWidth = NULL;

    switch (nTdm)
    {
        case E_AUDIO_TDM_RXA:
            nI2sTdmWsWidth = &g_nI2sTdmWsWidth[E_AUDIO_TDM_RXA];
            break;

        case E_AUDIO_TDM_RXB:
            nI2sTdmWsWidth = &g_nI2sTdmWsWidth[E_AUDIO_TDM_RXB];
            break;

        case E_AUDIO_TDM_RXC:
            nI2sTdmWsWidth = &g_nI2sTdmWsWidth[E_AUDIO_TDM_RXC];
            break;

        case E_AUDIO_TDM_RXD:
            nI2sTdmWsWidth = &g_nI2sTdmWsWidth[E_AUDIO_TDM_RXD];
            break;

        case E_AUDIO_TDM_TXA:
            nI2sTdmWsWidth = &g_nI2sTdmWsWidth[E_AUDIO_TDM_TXA];
            break;

        case E_AUDIO_TDM_TXB:
            nI2sTdmWsWidth = &g_nI2sTdmWsWidth[E_AUDIO_TDM_TXB];
            break;

        default:
            ERRMSG("Func:%s, Line:%d nTdm  %d Fail !\n", __FUNCTION__, __LINE__, nTdm);
            goto FAIL;
            break;
    }

    *nI2sTdmWsWidth = 0;

    *nI2sTdmWsWidth = nWsWidth;

    return AIO_OK;
FAIL:

    return AIO_NG;
}

int HalAudI2sGetTxTdmWsWidth(U8 *nWsWidth)
{
    *nWsWidth = g_nI2sTxTdmWsWidth;

    return AIO_OK;
}

int HalAudI2sSaveTxTdmWsInv(BOOL bEnable)
{
    g_nI2sTxTdmWsInv = (U8)bEnable;

    return AIO_OK;
}

int HalAudI2sGetTxTdmWsInv(U8 *bEnable)
{
    *bEnable = g_nI2sTxTdmWsInv;

    return AIO_OK;
}

int HalAudI2sSaveTxTdmChSwap(U8 nSwap_0_2, U8 nSwap_0_4)
{
    g_nI2sTxTdmChSwap = 0;

    if (nSwap_0_2 != 0)
    {
        g_nI2sTxTdmChSwap |= 0x01;
    }

    if (nSwap_0_4 != 0)
    {
        g_nI2sTxTdmChSwap |= 0x02;
    }

    return AIO_OK;
}

int HalAudI2sSetRxMode(U8 mode)
{
    g_nI2sRxMode = mode;
    return AIO_OK;
}

int HalAudI2sSaveTdmChSwap(AUDIO_TDM_e nTdm, U8 nSwap_0_2, U8 nSwap_0_4, U8 nSwap_L_R)
{
    long *nI2sTdmChSwap = NULL;

    switch (nTdm)
    {
        case E_AUDIO_TDM_RXA:
            nI2sTdmChSwap = &g_nI2sTdmChSwap[E_AUDIO_TDM_RXA];
            break;

        case E_AUDIO_TDM_RXB:
            nI2sTdmChSwap = &g_nI2sTdmChSwap[E_AUDIO_TDM_RXB];
            break;

        case E_AUDIO_TDM_RXC:
            nI2sTdmChSwap = &g_nI2sTdmChSwap[E_AUDIO_TDM_RXC];
            break;

        case E_AUDIO_TDM_RXD:
            nI2sTdmChSwap = &g_nI2sTdmChSwap[E_AUDIO_TDM_RXD];
            break;

        case E_AUDIO_TDM_TXA:
            nI2sTdmChSwap = &g_nI2sTdmChSwap[E_AUDIO_TDM_TXA];
            break;

        case E_AUDIO_TDM_TXB:
            nI2sTdmChSwap = &g_nI2sTdmChSwap[E_AUDIO_TDM_TXB];
            break;
        default:
            ERRMSG("Func:%s, Line:%d nTdm  %d Fail !\n", __FUNCTION__, __LINE__, nTdm);
            goto FAIL;
            break;
    }

    *nI2sTdmChSwap = 0;

    if (nSwap_0_2 != 0)
    {
        *nI2sTdmChSwap |= 0x02;
    }

    if (nSwap_0_4 != 0)
    {
        *nI2sTdmChSwap |= 0x01;
    }

    if (nSwap_L_R != 0)
    {
        *nI2sTdmChSwap |= 0x04;
    }

    return AIO_OK;
FAIL:

    return AIO_NG;
}

int HalAudI2sGetTxTdmChSwap(U8 *nSwap)
{
    *nSwap = g_nI2sTxTdmChSwap;

    return AIO_OK;
}

int HalAudI2sSaveTxTdmActiveSlot(U8 nActiveSlot)
{
    g_nI2sTxTdmActiveSlot = nActiveSlot;

    return AIO_OK;
}

int HalAudI2sGetTxTdmActiveSlot(U8 *nActiveSlot)
{
    *nActiveSlot = g_nI2sTxTdmActiveSlot;

    return AIO_OK;
}

int HalAudI2sSetTdmDetails(CHIP_AIO_I2S_e eAioI2s)
{
    switch (eAioI2s)
    {
        case E_CHIP_AIO_I2S_RX_A:
            HalBachWriteReg(E_BACH_REG_BANK2, E_BACH_I2S_TDM_CFG00, BACH_I2S_RX_WS_FMT,
                            (g_nI2sTdmWsPgm[E_AUDIO_TDM_RXA] ? BACH_I2S_RX_WS_FMT : 0));
            HalBachWriteReg(E_BACH_REG_BANK2, E_BACH_I2S_TDM_CFG00, BACH_I2S_RX_WS_INV,
                            (g_nI2sTdmWsInv[E_AUDIO_TDM_RXA] ? BACH_I2S_RX_WS_INV : 0));
            HalBachWriteReg(E_BACH_REG_BANK2, E_BACH_I2S_TDM_CFG01, BACH_I2S_RX_WS_WDTH_MSK,
                            (g_nI2sTdmWsWidth[E_AUDIO_TDM_RXA] << BACH_I2S_RX_WS_WDTH_POS));
            HalBachWriteReg(E_BACH_REG_BANK2, E_BACH_I2S_TDM_CFG02, BACH_I2S_RX_CH_SWAP_MSK,
                            (g_nI2sTdmChSwap[E_AUDIO_TDM_RXA] << BACH_I2S_RX_CH_SWAP_POS));
            HalBachWriteReg(E_BACH_REG_BANK2, E_BACH_I2S_CFG00, BACH_INV_CLK_RX_BCK_MSK,
                            (g_nI2sTdmBckInv[E_AUDIO_TDM_RXA] << BACH_INV_CLK_RX_BCK_POS));
            break;

        case E_CHIP_AIO_I2S_RX_B:
            HalBachWriteReg(E_BACH_REG_BANK4, E_BACH_I2S_TDM1_CFG00, BACH_I2S_RX_WS_FMT,
                            (g_nI2sTdmWsPgm[E_AUDIO_TDM_RXB] ? BACH_I2S_RX_WS_FMT : 0));
            HalBachWriteReg(E_BACH_REG_BANK4, E_BACH_I2S_TDM1_CFG00, BACH_I2S_RX_WS_INV,
                            (g_nI2sTdmWsInv[E_AUDIO_TDM_RXB] ? BACH_I2S_RX_WS_INV : 0));
            HalBachWriteReg(E_BACH_REG_BANK4, E_BACH_I2S_TDM1_CFG01, BACH_I2S_RX_WS_WDTH_MSK,
                            (g_nI2sTdmWsWidth[E_AUDIO_TDM_RXB] << BACH_I2S_RX_WS_WDTH_POS));
            HalBachWriteReg(E_BACH_REG_BANK4, E_BACH_I2S_TDM1_CFG02, BACH_I2S_RX_CH_SWAP_MSK,
                            (g_nI2sTdmChSwap[E_AUDIO_TDM_RXB] << BACH_I2S_RX_CH_SWAP_POS));
            HalBachWriteReg(E_BACH_REG_BANK4, E_BACH_I2S1_CFG00, BACH_INV_CLK_RX1_BCK_MSK,
                            (g_nI2sTdmBckInv[E_AUDIO_TDM_RXB] << BACH_INV_CLK_RX1_BCK_POS));
            break;

        case E_CHIP_AIO_I2S_RX_C:
            if (g_nI2sRxMode)
            {
                HalBachWriteReg(E_BACH_REG_BANK2, E_BACH_I2S_TDM_CFG00, BACH_I2S_RX_WS_FMT,
                                (g_nI2sTdmWsPgm[E_AUDIO_TDM_RXC] ? BACH_I2S_RX_WS_FMT : 0));
                HalBachWriteReg(E_BACH_REG_BANK2, E_BACH_I2S_TDM_CFG00, BACH_I2S_RX_WS_INV,
                                (g_nI2sTdmWsInv[E_AUDIO_TDM_RXC] ? BACH_I2S_RX_WS_INV : 0));
                HalBachWriteReg(E_BACH_REG_BANK2, E_BACH_I2S_TDM_CFG01, BACH_I2S_RX_WS_WDTH_MSK,
                                (g_nI2sTdmWsWidth[E_AUDIO_TDM_RXC] << BACH_I2S_RX_WS_WDTH_POS));
                HalBachWriteReg(E_BACH_REG_BANK4, E_BACH_I2S_TDM2_CFG02, BACH_I2S_RX_CH_SWAP_MSK,
                                (g_nI2sTdmChSwap[E_AUDIO_TDM_RXC] << BACH_I2S_RX_CH_SWAP_POS));
                HalBachWriteReg(E_BACH_REG_BANK2, E_BACH_I2S_CFG00, BACH_INV_CLK_RX_BCK_MSK,
                                (g_nI2sTdmBckInv[E_AUDIO_TDM_RXC] << BACH_INV_CLK_RX_BCK_POS));
            }
            else
            {
                HalBachWriteReg(E_BACH_REG_BANK4, E_BACH_I2S_TDM2_CFG00, BACH_I2S_RX_WS_FMT,
                                (g_nI2sRxTdmWsPgm ? BACH_I2S_RX_WS_FMT : 0));
                HalBachWriteReg(E_BACH_REG_BANK4, E_BACH_I2S_TDM2_CFG00, BACH_I2S_RX_WS_INV,
                                (g_nI2sRxTdmWsInv ? BACH_I2S_RX_WS_INV : 0));
                HalBachWriteReg(E_BACH_REG_BANK4, E_BACH_I2S_TDM2_CFG01, BACH_I2S_RX_WS_WDTH_MSK,
                                (g_nI2sRxTdmWsWidth << BACH_I2S_RX_WS_WDTH_POS));
                HalBachWriteReg(E_BACH_REG_BANK4, E_BACH_I2S_TDM2_CFG02, BACH_I2S_RX_CH_SWAP_MSK,
                                (g_nI2sRxTdmChSwap << BACH_I2S_RX_CH_SWAP_POS));
            }
            break;

        case E_CHIP_AIO_I2S_RX_D:
            if (g_nI2sRxMode)
            {
                HalBachWriteReg(E_BACH_REG_BANK4, E_BACH_I2S_TDM1_CFG00, BACH_I2S_RX_WS_FMT,
                                (g_nI2sTdmWsPgm[E_AUDIO_TDM_RXD] ? BACH_I2S_RX_WS_FMT : 0));
                HalBachWriteReg(E_BACH_REG_BANK4, E_BACH_I2S_TDM1_CFG00, BACH_I2S_RX_WS_INV,
                                (g_nI2sTdmWsInv[E_AUDIO_TDM_RXD] ? BACH_I2S_RX_WS_INV : 0));
                HalBachWriteReg(E_BACH_REG_BANK4, E_BACH_I2S_TDM1_CFG01, BACH_I2S_RX_WS_WDTH_MSK,
                                (g_nI2sTdmWsWidth[E_AUDIO_TDM_RXD] << BACH_I2S_RX_WS_WDTH_POS));
                HalBachWriteReg(E_BACH_REG_BANK4, E_BACH_I2S_TDM3_CFG02, BACH_I2S_RX_CH_SWAP_MSK,
                                (g_nI2sTdmChSwap[E_AUDIO_TDM_RXD] << BACH_I2S_RX_CH_SWAP_POS));
                HalBachWriteReg(E_BACH_REG_BANK4, E_BACH_I2S1_CFG00, BACH_INV_CLK_RX1_BCK_MSK,
                                (g_nI2sTdmBckInv[E_AUDIO_TDM_RXD] << BACH_INV_CLK_RX1_BCK_POS));
            }
            else
            {
                HalBachWriteReg(E_BACH_REG_BANK4, E_BACH_I2S_TDM3_CFG00, BACH_I2S_RX_WS_FMT,
                                (g_nI2sRxTdmWsPgm ? BACH_I2S_RX_WS_FMT : 0));
                HalBachWriteReg(E_BACH_REG_BANK4, E_BACH_I2S_TDM3_CFG00, BACH_I2S_RX_WS_INV,
                                (g_nI2sRxTdmWsInv ? BACH_I2S_RX_WS_INV : 0));
                HalBachWriteReg(E_BACH_REG_BANK4, E_BACH_I2S_TDM3_CFG01, BACH_I2S_RX_WS_WDTH_MSK,
                                (g_nI2sRxTdmWsWidth << BACH_I2S_RX_WS_WDTH_POS));
                HalBachWriteReg(E_BACH_REG_BANK4, E_BACH_I2S_TDM3_CFG02, BACH_I2S_RX_CH_SWAP_MSK,
                                (g_nI2sRxTdmChSwap << BACH_I2S_RX_CH_SWAP_POS));
            }
            break;

        case E_CHIP_AIO_I2S_TX_A:
            HalBachWriteReg(E_BACH_REG_BANK2, E_BACH_I2S_TDM_CFG05, BACH_I2S_TX_WS_FMT,
                            (g_nI2sTdmWsPgm[E_AUDIO_TDM_TXA] ? BACH_I2S_TX_WS_FMT : 0));
            HalBachWriteReg(E_BACH_REG_BANK2, E_BACH_I2S_TDM_CFG05, BACH_I2S_TX_WS_INV,
                            (g_nI2sTdmWsInv[E_AUDIO_TDM_TXA] ? BACH_I2S_TX_WS_INV : 0));
            HalBachWriteReg(E_BACH_REG_BANK2, E_BACH_I2S_TDM_CFG06, BACH_I2S_TX_WS_WDTH_MSK,
                            (g_nI2sTdmWsWidth[E_AUDIO_TDM_TXA] << BACH_I2S_TX_WS_WDTH_POS));
            HalBachWriteReg(E_BACH_REG_BANK2, E_BACH_I2S_TDM_CFG06, BACH_I2S_TX_CH_SWAP_MSK,
                            (g_nI2sTdmChSwap[E_AUDIO_TDM_TXA] << BACH_I2S_TX_CH_SWAP_POS));
            HalBachWriteReg(E_BACH_REG_BANK2, E_BACH_I2S_TDM_CFG06, BACH_I2S_TX_ACT_SLOT_MSK,
                            (g_nI2sTxTdmActiveSlot << BACH_I2S_TX_ACT_SLOT_POS));
            HalBachWriteReg(E_BACH_REG_BANK2, E_BACH_I2S_CFG00, BACH_INV_CLK_TX_BCK_MSK,
                            (g_nI2sTdmBckInv[E_AUDIO_TDM_TXA] << BACH_INV_CLK_TX_BCK_POS));
            break;

        case E_CHIP_AIO_I2S_TX_B:
            HalBachWriteReg(E_BACH_REG_BANK4, E_BACH_I2S_TDM1_CFG05, BACH_I2S_TX_WS_FMT,
                            (g_nI2sTdmWsPgm[E_AUDIO_TDM_TXB] ? BACH_I2S_TX_WS_FMT : 0));
            HalBachWriteReg(E_BACH_REG_BANK4, E_BACH_I2S_TDM1_CFG05, BACH_I2S_TX_WS_INV,
                            (g_nI2sTdmWsInv[E_AUDIO_TDM_TXB] ? BACH_I2S_TX_WS_INV : 0));
            HalBachWriteReg(E_BACH_REG_BANK4, E_BACH_I2S_TDM1_CFG06, BACH_I2S_TX_WS_WDTH_MSK,
                            (g_nI2sTdmWsWidth[E_AUDIO_TDM_TXB] << BACH_I2S_TX_WS_WDTH_POS));
            HalBachWriteReg(E_BACH_REG_BANK4, E_BACH_I2S_TDM1_CFG06, BACH_I2S_TX_CH_SWAP_MSK,
                            (g_nI2sTdmChSwap[E_AUDIO_TDM_TXB] << BACH_I2S_TX_CH_SWAP_POS));
            HalBachWriteReg(E_BACH_REG_BANK4, E_BACH_I2S_TDM1_CFG06, BACH_I2S_TX_ACT_SLOT_MSK,
                            (g_nI2sTxTdmActiveSlot << BACH_I2S_TX_ACT_SLOT_POS));
            HalBachWriteReg(E_BACH_REG_BANK4, E_BACH_I2S1_CFG00, BACH_INV_CLK_TX1_BCK_MSK,
                            (g_nI2sTdmBckInv[E_AUDIO_TDM_TXB] << BACH_INV_CLK_TX1_BCK_POS));
            break;

        default:
            ERRMSG("Func:%s, Line:%d eAioI2s %d Fail !\n", __FUNCTION__, __LINE__, eAioI2s);
            goto FAIL;
            break;
    }

    return AIO_OK;

FAIL:

    return AIO_NG;
}

int HalAudI2sSetRate(CHIP_AIO_I2S_e eAioI2s, AudRate_e eRate)
{
    g_aI2sCfg[eAioI2s].eRate = eRate;

    return _HalAudI2sSetBck(eAioI2s, _HalAudBckCalculate(g_aI2sCfg[eAioI2s].eRate, g_aI2sCfg[eAioI2s].enI2sWidth,
                                                         g_aI2sCfg[eAioI2s].nChannelNum));
}

#if 1
int HalAudI2sSetClkRef(CHIP_AIO_I2S_e eAioI2s, AudI2sClkRef_e eI2sClkRef)
{
    U16            nClkRefSetting = 0;
    U16            msk, pos;
    CHIP_AIO_I2S_e i;

    for (i = E_CHIP_AIO_I2S_RX_START; i <= E_CHIP_AIO_I2S_RX_END; i++)
    {
        switch (i)
        {
            case E_CHIP_AIO_I2S_RX_A:
                msk = BACH_I2S_TDM_RX_MSK;
                pos = BACH_I2S_TDM_RX_POS;
                break;
            case E_CHIP_AIO_I2S_RX_B:
                msk = BACH_I2S_TDM_RX1_MSK;
                pos = BACH_I2S_TDM_RX1_POS;
                break;
            case E_CHIP_AIO_I2S_RX_C:
                msk = BACH_I2S_TDM_RX2_MSK;
                pos = BACH_I2S_TDM_RX2_POS;
                break;
            case E_CHIP_AIO_I2S_RX_D:
                msk = BACH_I2S_TDM_RX3_MSK;
                pos = BACH_I2S_TDM_RX3_POS;
                break;
            default:
                ERRMSG("Func:%s, Line:%d eAioI2s %d eI2sClkRef %d Fail !\n", __FUNCTION__, __LINE__, eAioI2s,
                       eI2sClkRef);
                goto FAIL;
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
                ERRMSG("Func:%s, Line:%d eAioI2s %d eI2sClkRef %d Fail !\n", __FUNCTION__, __LINE__, eAioI2s,
                       eI2sClkRef);
                goto FAIL;
                break;
        }

        HalBachWriteReg(E_BACH_REG_BANK2, E_BACH_DMA_SRC_CFG02, msk, (nClkRefSetting << pos)); // 0x15039A[1:0]
    }

    return AIO_OK;

FAIL:

    return AIO_NG;
}
#endif

int HalAudI2sSetTdmMode(CHIP_AIO_I2S_e eAioI2s, AudI2sMode_e enI2sMode)
{
    U16 nTdmMode;

    switch (enI2sMode)
    {
        case E_AUD_I2S_MODE_I2S:
            nTdmMode = 0;
            break;
        case E_AUD_I2S_MODE_TDM:
            nTdmMode = 1;
            break;
        default:
            ERRMSG("Func:%s, Line:%d enI2sMode %d Fail !\n", __FUNCTION__, __LINE__, enI2sMode);
            goto FAIL;
            break;
    }
    switch (eAioI2s)
    {
        case E_CHIP_AIO_I2S_RX_A:
            HalBachWriteReg(E_BACH_REG_BANK2, E_BACH_I2S_TDM_CFG00, BACH_I2S_RX_TDM_MODE,
                            (nTdmMode ? BACH_I2S_RX_TDM_MODE : 0));
            break;

        case E_CHIP_AIO_I2S_RX_B:
            HalBachWriteReg(E_BACH_REG_BANK4, E_BACH_I2S_TDM1_CFG00, BACH_I2S_RX_TDM_MODE,
                            (nTdmMode ? BACH_I2S_RX_TDM_MODE : 0));
            break;

        case E_CHIP_AIO_I2S_RX_C:
            if (g_nI2sRxMode)
            {
                HalBachWriteReg(E_BACH_REG_BANK2, E_BACH_I2S_TDM_CFG00, BACH_I2S_RX_TDM_MODE,
                                (nTdmMode ? BACH_I2S_RX_TDM_MODE : 0));
            }
            else
            {
                HalBachWriteReg(E_BACH_REG_BANK4, E_BACH_I2S_TDM2_CFG00, BACH_I2S_RX_TDM_MODE,
                                (nTdmMode ? BACH_I2S_RX_TDM_MODE : 0));
            }
            break;

        case E_CHIP_AIO_I2S_RX_D:
            if (g_nI2sRxMode)
            {
                HalBachWriteReg(E_BACH_REG_BANK4, E_BACH_I2S_TDM1_CFG00, BACH_I2S_RX_TDM_MODE,
                                (nTdmMode ? BACH_I2S_RX_TDM_MODE : 0));
            }
            else
            {
                HalBachWriteReg(E_BACH_REG_BANK4, E_BACH_I2S_TDM3_CFG00, BACH_I2S_RX_TDM_MODE,
                                (nTdmMode ? BACH_I2S_RX_TDM_MODE : 0));
            }
            break;

        case E_CHIP_AIO_I2S_TX_A:
            HalBachWriteReg(E_BACH_REG_BANK2, E_BACH_I2S_TDM_CFG05, BACH_I2S_TX_TDM_MODE,
                            (nTdmMode ? BACH_I2S_TX_TDM_MODE : 0));
            break;

        case E_CHIP_AIO_I2S_TX_B:
            HalBachWriteReg(E_BACH_REG_BANK4, E_BACH_I2S_TDM1_CFG05, BACH_I2S_TX_TDM_MODE,
                            (nTdmMode ? BACH_I2S_TX_TDM_MODE : 0));
            break;

        default:
            ERRMSG("Func:%s, Line:%d eAioI2s %d Fail !\n", __FUNCTION__, __LINE__, eAioI2s);
            goto FAIL;
            break;
    }

    g_aI2sCfg[eAioI2s].enI2sMode = enI2sMode;

    return AIO_OK;

FAIL:

    return AIO_NG;
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
            ERRMSG("Func:%s, Line:%d eMsMode %d Fail !\n", __FUNCTION__, __LINE__, eMsMode);
            goto FAIL;
            break;
    }

    switch (eAioI2s)
    {
        case E_CHIP_AIO_I2S_RX_A:
            HalBachWriteReg(E_BACH_REG_BANK2, E_BACH_I2S_TDM_CFG00, BACH_I2S_RX_MS_MODE,
                            (nMsMode ? BACH_I2S_RX_MS_MODE : 0)); // 150360[14]
            break;

        case E_CHIP_AIO_I2S_RX_B:
            HalBachWriteReg(E_BACH_REG_BANK4, E_BACH_I2S_TDM1_CFG00, BACH_I2S_RX_MS_MODE,
                            (nMsMode ? BACH_I2S_RX_MS_MODE : 0));
            break;

        case E_CHIP_AIO_I2S_RX_C:
            if (g_nI2sRxMode)
            {
                HalBachWriteReg(E_BACH_REG_BANK2, E_BACH_I2S_TDM_CFG00, BACH_I2S_RX_MS_MODE,
                                (nMsMode ? BACH_I2S_RX_MS_MODE : 0));
            }
            else
            {
                HalBachWriteReg(E_BACH_REG_BANK4, E_BACH_I2S_TDM2_CFG00, BACH_I2S_RX_MS_MODE,
                                (nMsMode ? BACH_I2S_RX_MS_MODE : 0));
            }
            break;

        case E_CHIP_AIO_I2S_RX_D:
            if (g_nI2sRxMode)
            {
                HalBachWriteReg(E_BACH_REG_BANK4, E_BACH_I2S_TDM1_CFG00, BACH_I2S_RX_MS_MODE,
                                (nMsMode ? BACH_I2S_RX_MS_MODE : 0));
            }
            else
            {
                HalBachWriteReg(E_BACH_REG_BANK4, E_BACH_I2S_TDM3_CFG00, BACH_I2S_RX_MS_MODE,
                                (nMsMode ? BACH_I2S_RX_MS_MODE : 0));
            }
            break;

        case E_CHIP_AIO_I2S_TX_A:
            HalBachWriteReg(E_BACH_REG_BANK2, E_BACH_I2S_TDM_CFG05, BACH_I2S_TX_MS_MODE,
                            (nMsMode ? BACH_I2S_TX_MS_MODE : 0));
            break;

        case E_CHIP_AIO_I2S_TX_B:
            HalBachWriteReg(E_BACH_REG_BANK4, E_BACH_I2S_TDM1_CFG05, BACH_I2S_TX_MS_MODE,
                            (nMsMode ? BACH_I2S_TX_MS_MODE : 0));
            break;

        default:
            ERRMSG("Func:%s, Line:%d eAioI2s %d Fail !\n", __FUNCTION__, __LINE__, eAioI2s);
            goto FAIL;
            break;
    }

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
            ERRMSG("Func:%s, Line:%d enI2sFmt %d Fail !\n", __FUNCTION__, __LINE__, enI2sFmt);
            goto FAIL;
            break;
    }

    switch (eAioI2s)
    {
        case E_CHIP_AIO_I2S_RX_A:
            HalBachWriteReg(E_BACH_REG_BANK2, E_BACH_I2S_TDM_CFG00, BACH_I2S_RX_FMT, (nSel ? BACH_I2S_RX_FMT : 0));
            break;

        case E_CHIP_AIO_I2S_RX_B:
            HalBachWriteReg(E_BACH_REG_BANK4, E_BACH_I2S_TDM1_CFG00, BACH_I2S_RX_FMT, (nSel ? BACH_I2S_RX_FMT : 0));
            break;

        case E_CHIP_AIO_I2S_RX_C:
            if (g_nI2sRxMode)
            {
                HalBachWriteReg(E_BACH_REG_BANK2, E_BACH_I2S_TDM_CFG00, BACH_I2S_RX_FMT,
                                (nSel ? BACH_I2S_RX_FMT : 0)); // 1503A0[6]
            }
            else
            {
                HalBachWriteReg(E_BACH_REG_BANK4, E_BACH_I2S_TDM2_CFG00, BACH_I2S_RX_FMT, (nSel ? BACH_I2S_RX_FMT : 0));
            }
            break;

        case E_CHIP_AIO_I2S_RX_D:
            if (g_nI2sRxMode)
            {
                HalBachWriteReg(E_BACH_REG_BANK4, E_BACH_I2S_TDM1_CFG00, BACH_I2S_RX_FMT, (nSel ? BACH_I2S_RX_FMT : 0));
            }
            else
            {
                HalBachWriteReg(E_BACH_REG_BANK4, E_BACH_I2S_TDM3_CFG00, BACH_I2S_RX_FMT, (nSel ? BACH_I2S_RX_FMT : 0));
            }
            break;

        case E_CHIP_AIO_I2S_TX_A:
            HalBachWriteReg(E_BACH_REG_BANK2, E_BACH_I2S_TDM_CFG05, BACH_I2S_TX_FMT, (nSel ? BACH_I2S_TX_FMT : 0));
            break;

        case E_CHIP_AIO_I2S_TX_B:
            HalBachWriteReg(E_BACH_REG_BANK4, E_BACH_I2S_TDM1_CFG05, BACH_I2S_TX_FMT, (nSel ? BACH_I2S_TX_FMT : 0));
            break;

        default:
            ERRMSG("Func:%s, Line:%d eAioI2s %d Fail !\n", __FUNCTION__, __LINE__, eAioI2s);
            goto FAIL;
            break;
    }

    g_aI2sCfg[eAioI2s].eFormat = enI2sFmt;

    return AIO_OK;

FAIL:

    return AIO_NG;
}

int HalAudI2sSetWidth(CHIP_AIO_I2S_e eAioI2s, AudBitWidth_e enI2sWidth)
{
    U16 nSel;

    switch (enI2sWidth)
    {
        case E_AUD_BITWIDTH_16:
            nSel = 0;
            break;
        case E_AUD_BITWIDTH_32:
            nSel = 1;
            break;
        default:
            ERRMSG("Func:%s, Line:%d enI2sWidth %d Fail !\n", __FUNCTION__, __LINE__, enI2sWidth);
            goto FAIL;
            break;
    }

    switch (eAioI2s)
    {
        case E_CHIP_AIO_I2S_RX_A:
            HalBachWriteReg(E_BACH_REG_BANK2, E_BACH_I2S_TDM_CFG00, BACH_I2S_RX_ENC_WIDTH,
                            (nSel ? BACH_I2S_RX_ENC_WIDTH : 0));
            break;

        case E_CHIP_AIO_I2S_RX_B:
            HalBachWriteReg(E_BACH_REG_BANK4, E_BACH_I2S_TDM1_CFG00, BACH_I2S_RX_ENC_WIDTH,
                            (nSel ? BACH_I2S_RX_ENC_WIDTH : 0));
            break;

        case E_CHIP_AIO_I2S_RX_C:
            if (g_nI2sRxMode)
            {
                HalBachWriteReg(E_BACH_REG_BANK2, E_BACH_I2S_TDM_CFG00, BACH_I2S_RX_ENC_WIDTH,
                                (nSel ? BACH_I2S_RX_ENC_WIDTH : 0));
            }
            else
            {
                HalBachWriteReg(E_BACH_REG_BANK4, E_BACH_I2S_TDM2_CFG00, BACH_I2S_RX_ENC_WIDTH,
                                (nSel ? BACH_I2S_RX_ENC_WIDTH : 0));
            }
            break;

        case E_CHIP_AIO_I2S_RX_D:
            if (g_nI2sRxMode)
            {
                HalBachWriteReg(E_BACH_REG_BANK4, E_BACH_I2S_TDM1_CFG00, BACH_I2S_RX_ENC_WIDTH,
                                (nSel ? BACH_I2S_RX_ENC_WIDTH : 0));
            }
            else
            {
                HalBachWriteReg(E_BACH_REG_BANK4, E_BACH_I2S_TDM3_CFG00, BACH_I2S_RX_ENC_WIDTH,
                                (nSel ? BACH_I2S_RX_ENC_WIDTH : 0));
            }
            break;

        case E_CHIP_AIO_I2S_TX_A:
            HalBachWriteReg(E_BACH_REG_BANK2, E_BACH_I2S_TDM_CFG05, BACH_I2S_TX_ENC_WIDTH,
                            (nSel ? BACH_I2S_TX_ENC_WIDTH : 0));
            break;

        case E_CHIP_AIO_I2S_TX_B:
            HalBachWriteReg(E_BACH_REG_BANK4, E_BACH_I2S_TDM1_CFG05, BACH_I2S_TX_ENC_WIDTH,
                            (nSel ? BACH_I2S_TX_ENC_WIDTH : 0));
            break;

        default:
            ERRMSG("Func:%s, Line:%d eAioI2s %d Fail !\n", __FUNCTION__, __LINE__, eAioI2s);
            goto FAIL;
            break;
    }

    g_aI2sCfg[eAioI2s].enI2sWidth = enI2sWidth;

    return AIO_OK;

FAIL:

    return AIO_NG;
}

int HalAudI2sSetChannel(CHIP_AIO_I2S_e eAioI2s, U16 nChannel)
{
    if ((nChannel != 2) && (nChannel != 4) && (nChannel != 8))
    {
        ERRMSG("Func:%s, Line:%d nChannel %d Fail !\n", __FUNCTION__, __LINE__, nChannel);
        goto FAIL;
    }

    if (nChannel == 2)
    {
        g_aI2sCfg[eAioI2s].nChannelNum = nChannel;
        goto SUCCESS;
    }
    else
    {
        switch (eAioI2s)
        {
            case E_CHIP_AIO_I2S_RX_A:
                HalBachWriteReg(E_BACH_REG_BANK2, E_BACH_I2S_TDM_CFG00, BACH_I2S_RX_CHLEN,
                                (nChannel == 8 ? BACH_I2S_RX_CHLEN : 0));
                break;

            case E_CHIP_AIO_I2S_RX_B:
                HalBachWriteReg(E_BACH_REG_BANK4, E_BACH_I2S_TDM1_CFG00, BACH_I2S_RX_CHLEN,
                                (nChannel == 8 ? BACH_I2S_RX_CHLEN : 0));
                break;

            case E_CHIP_AIO_I2S_RX_C:
                if (g_nI2sRxMode)
                {
                    HalBachWriteReg(E_BACH_REG_BANK2, E_BACH_I2S_TDM_CFG00, BACH_I2S_RX_CHLEN,
                                    (nChannel == 8 ? BACH_I2S_RX_CHLEN : 0));
                }
                else
                {
                    HalBachWriteReg(E_BACH_REG_BANK4, E_BACH_I2S_TDM2_CFG00, BACH_I2S_RX_CHLEN,
                                    (nChannel == 8 ? BACH_I2S_RX_CHLEN : 0));
                }
                break;

            case E_CHIP_AIO_I2S_RX_D:
                if (g_nI2sRxMode)
                {
                    HalBachWriteReg(E_BACH_REG_BANK4, E_BACH_I2S_TDM1_CFG00, BACH_I2S_RX_CHLEN,
                                    (nChannel == 8 ? BACH_I2S_RX_CHLEN : 0));
                }
                else
                {
                    HalBachWriteReg(E_BACH_REG_BANK4, E_BACH_I2S_TDM3_CFG00, BACH_I2S_RX_CHLEN,
                                    (nChannel == 8 ? BACH_I2S_RX_CHLEN : 0));
                }
                break;

            case E_CHIP_AIO_I2S_TX_A:
                HalBachWriteReg(E_BACH_REG_BANK2, E_BACH_I2S_TDM_CFG05, BACH_I2S_TX_CHLEN,
                                (nChannel == 8 ? BACH_I2S_TX_CHLEN : 0));
                break;

            case E_CHIP_AIO_I2S_TX_B:
                HalBachWriteReg(E_BACH_REG_BANK4, E_BACH_I2S_TDM1_CFG05, BACH_I2S_TX_CHLEN,
                                (nChannel == 8 ? BACH_I2S_TX_CHLEN : 0));
                break;

            default:
                ERRMSG("Func:%s, Line:%d eAioI2s %d Fail !\n", __FUNCTION__, __LINE__, eAioI2s);
                goto FAIL;
                break;
        }

        g_aI2sCfg[eAioI2s].nChannelNum = nChannel;
    }
SUCCESS:

    return AIO_OK;

FAIL:

    return AIO_NG;
}

int HalAudI2sSetWireMode(CHIP_AIO_I2S_e eAioI2s, AudWireMode_e eWireMode)
{
    if (g_nI2sRxMode)
    {
        HalBachWriteReg(E_BACH_REG_BANK4, E_BACH_I2S_TDM3_CFG00, BACH_I2S_RX_SHARE_MODE, BACH_I2S_RX_SHARE_MODE);
        HalBachWriteReg(E_BACH_REG_BANK4, E_BACH_I2S_TDM2_CFG00, BACH_I2S_RX_SHARE_MODE, BACH_I2S_RX_SHARE_MODE);
    }
    else
    {
        HalBachWriteReg(E_BACH_REG_BANK4, E_BACH_I2S_TDM3_CFG00, BACH_I2S_RX_SHARE_MODE, 0);
        HalBachWriteReg(E_BACH_REG_BANK4, E_BACH_I2S_TDM2_CFG00, BACH_I2S_RX_SHARE_MODE, 0);
    }
    if (eAioI2s == E_CHIP_AIO_I2S_TX_A)
    {
        switch (eWireMode)
        {
            case E_AUD_I2S_WIRE_4:
                HalBachWriteReg(E_BACH_REG_BANK2, E_BACH_I2S_TDM_CFG05, BACH_I2S_TX_4WIRE_MODE, BACH_I2S_TX_4WIRE_MODE);
                break;

            case E_AUD_I2S_WIRE_6:
                HalBachWriteReg(E_BACH_REG_BANK2, E_BACH_I2S_TDM_CFG05, BACH_I2S_TX_4WIRE_MODE, 0);
                break;

            default:
                ERRMSG("Func:%s, Line:%d eAioI2s %d eWireMode %d Fail !\n", __FUNCTION__, __LINE__, eAioI2s, eWireMode);
                goto FAIL;
                break;
        }

        g_aI2sCfg[eAioI2s].eWireMode = eWireMode;
    }
    else if (eAioI2s == E_CHIP_AIO_I2S_TX_B)
    {
        switch (eWireMode)
        {
            case E_AUD_I2S_WIRE_4:
                HalBachWriteReg(E_BACH_REG_BANK4, E_BACH_I2S_TDM1_CFG05, BACH_I2S_TX_4WIRE_MODE,
                                BACH_I2S_TX_4WIRE_MODE);
                break;

            case E_AUD_I2S_WIRE_6:
                HalBachWriteReg(E_BACH_REG_BANK4, E_BACH_I2S_TDM1_CFG05, BACH_I2S_TX_4WIRE_MODE, 0);
                break;

            default:
                ERRMSG("Func:%s, Line:%d eAioI2s %d eWireMode %d Fail !\n", __FUNCTION__, __LINE__, eAioI2s, eWireMode);
                goto FAIL;
                break;
        }
    }
    else
    {
        g_aI2sCfg[eAioI2s].eWireMode = E_AUD_I2S_WIRE_NULL;
    }

    return AIO_OK;

FAIL:

    return AIO_NG;
}

int HalAudI2sSetTdmSlotConfig(CHIP_AIO_I2S_e eAioI2s, U16 nSlotMsk, AudTdmChnMap_e eMap)
{
    switch (eAioI2s)
    {
        case E_CHIP_AIO_I2S_RX_A:
            break;

        case E_CHIP_AIO_I2S_TX_A:
            HalBachWriteReg(E_BACH_REG_BANK2, E_BACH_I2S_TDM_CFG06, BACH_I2S_TX_ACT_SLOT_MSK,
                            (nSlotMsk << BACH_I2S_TX_ACT_SLOT_POS));
            HalBachWriteReg(E_BACH_REG_BANK2, E_BACH_I2S_TDM_CFG06, BACH_I2S_TX_CH_SWAP_MSK,
                            (eMap << BACH_I2S_TX_CH_SWAP_POS));
            break;

        default:
            ERRMSG("Func:%s, Line:%d eAioI2s %d Fail !\n", __FUNCTION__, __LINE__, eAioI2s);
            goto FAIL;
            break;
    }

    return AIO_OK;

FAIL:

    return AIO_NG;
}

int HalAudI2sEnableMck(CHIP_AIO_I2S_e eAioI2s, BOOL bEn)
{
    U16 enTimeGen = BACH_ENABLE_CLK_NF_SYNTH_REF | BACH_CODEC_BCK_EN_TIME_GEN;
    U16 bank;

    if ((bEn == TRUE) && (g_aI2sCfg[eAioI2s].eMck == E_AUD_I2S_MCK_NULL))
    {
        // do nothing
        return AIO_OK;
    }

    switch (eAioI2s)
    {
        case E_CHIP_AIO_I2S_RX_A:
            g_bI2MckActive[E_CHIP_AIO_I2S_RX_A] = TRUE;
            break;
        case E_CHIP_AIO_I2S_TX_A:
            g_bI2MckActive[E_CHIP_AIO_I2S_TX_A] = TRUE;
            break;
        case E_CHIP_AIO_I2S_RX_C:
            g_bI2MckActive[E_CHIP_AIO_I2S_RX_C] = TRUE;
            break;
        case E_CHIP_AIO_I2S_RX_B:
            g_bI2MckActive[E_CHIP_AIO_I2S_RX_B] = TRUE;
            break;
        case E_CHIP_AIO_I2S_TX_B:
            g_bI2MckActive[E_CHIP_AIO_I2S_TX_B] = TRUE;
            break;
        case E_CHIP_AIO_I2S_RX_D:
            g_bI2MckActive[E_CHIP_AIO_I2S_RX_D] = TRUE;
            break;
        default:
            ERRMSG("Func:%s, Line:%d eAioI2s %d Fail !\n", __FUNCTION__, __LINE__, eAioI2s);
            goto FAIL;
            break;
    }

    switch (eAioI2s)
    {
        case E_CHIP_AIO_I2S_TX_A:
        case E_CHIP_AIO_I2S_RX_A:
        case E_CHIP_AIO_I2S_RX_C:
            bank = E_BACH_REG_BANK2;
            break;
        case E_CHIP_AIO_I2S_TX_B:
        case E_CHIP_AIO_I2S_RX_B:
        case E_CHIP_AIO_I2S_RX_D:
            bank = E_BACH_REG_BANK4;
            break;
        default:
            ERRMSG("Func:%s, Line:%d eAioI2s %d Fail !\n", __FUNCTION__, __LINE__, eAioI2s);
            goto FAIL;
    }

    if (bEn)
    {
        HalBachWriteReg(bank, E_BACH_NF_SYNTH_MCK00, enTimeGen | BACH_CODEC_BCK_EN_SYNTH_TRIG, enTimeGen);
        HalBachWriteReg(bank, E_BACH_NF_SYNTH_MCK00, enTimeGen | BACH_CODEC_BCK_EN_SYNTH_TRIG,
                        enTimeGen | BACH_CODEC_BCK_EN_SYNTH_TRIG);
        HalBachWriteReg(bank, E_BACH_NF_SYNTH_MCK00, enTimeGen | BACH_CODEC_BCK_EN_SYNTH_TRIG, enTimeGen);
    }
    else
    {
        switch (eAioI2s)
        {
            case E_CHIP_AIO_I2S_RX_A:
                g_bI2MckActive[E_CHIP_AIO_I2S_RX_A] = FALSE;
                break;
            case E_CHIP_AIO_I2S_TX_A:
                g_bI2MckActive[E_CHIP_AIO_I2S_TX_A] = FALSE;
                break;
            case E_CHIP_AIO_I2S_RX_B:
                g_bI2MckActive[E_CHIP_AIO_I2S_RX_B] = FALSE;
                break;
            case E_CHIP_AIO_I2S_TX_B:
                g_bI2MckActive[E_CHIP_AIO_I2S_TX_B] = FALSE;
                break;
            case E_CHIP_AIO_I2S_RX_C:
                g_bI2MckActive[E_CHIP_AIO_I2S_RX_C] = FALSE;
                break;
            case E_CHIP_AIO_I2S_RX_D:
                g_bI2MckActive[E_CHIP_AIO_I2S_RX_D] = FALSE;
                break;
            default:
                ERRMSG("Func:%s, Line:%d eAioI2s %d Fail !\n", __FUNCTION__, __LINE__, eAioI2s);
                goto FAIL;
                break;
        }
        if (bank == E_BACH_REG_BANK2)
        {
            if ((g_bI2MckActive[E_CHIP_AIO_I2S_RX_A] == FALSE) && (g_bI2MckActive[E_CHIP_AIO_I2S_TX_A] == FALSE)
                && (g_bI2MckActive[E_CHIP_AIO_I2S_RX_C]))
            {
                HalBachWriteReg(E_BACH_REG_BANK2, E_BACH_NF_SYNTH_MCK00, enTimeGen | BACH_CODEC_BCK_EN_SYNTH_TRIG, 0);
            }
        }
        else if (bank == E_BACH_REG_BANK4)
        {
            if ((g_bI2MckActive[E_CHIP_AIO_I2S_RX_B] == FALSE) && (g_bI2MckActive[E_CHIP_AIO_I2S_TX_B] == FALSE)
                && (g_bI2MckActive[E_CHIP_AIO_I2S_RX_D]))
            {
                HalBachWriteReg(E_BACH_REG_BANK4, E_BACH_NF_SYNTH_MCK00, enTimeGen | BACH_CODEC_BCK_EN_SYNTH_TRIG, 0);
            }
        }
        else
        {
            ERRMSG("Func:%s, Line:%d bank %f Fail !\n", __FUNCTION__, __LINE__, bank);
            goto FAIL;
        }
    }

    return AIO_OK;

FAIL:

    return AIO_NG;
}

int HalAudI2sSetMck(CHIP_AIO_I2S_e eAioI2s, AudI2sMck_e eMck)
{
    int ret = AIO_OK;
    U16 bank;

    if (eMck < E_AUD_I2S_MCK_NULL || eMck > E_AUD_I2S_MCK_48M)
    {
        ERRMSG("Func:%s, Line:%d eAioI2s %d eMck %d Fail !\n", __FUNCTION__, __LINE__, eAioI2s, eMck);
        goto FAIL;
    }

    switch (eAioI2s)
    {
        case E_CHIP_AIO_I2S_TX_A:
        case E_CHIP_AIO_I2S_RX_A:
        case E_CHIP_AIO_I2S_RX_C:
            bank = E_BACH_REG_BANK2;
            break;
        case E_CHIP_AIO_I2S_TX_B:
        case E_CHIP_AIO_I2S_RX_B:
        case E_CHIP_AIO_I2S_RX_D:
            bank = E_BACH_REG_BANK4;
            break;
        default:
            ERRMSG("Func:%s, Line:%d eAioI2s %d eMck %d Fail !\n", __FUNCTION__, __LINE__, eAioI2s, eMck);
            goto FAIL;
    }

    switch (eMck)
    {
        case E_AUD_I2S_MCK_48M:
            HalBachWriteReg(bank, E_BACH_I2S_CFG00, BACH_MCK_SEL_MSK, 0 << BACH_MCK_SEL_POS);
            break;
        case E_AUD_I2S_MCK_24M:
            HalBachWriteReg(bank, E_BACH_I2S_CFG00, BACH_MCK_SEL_MSK, 1 << BACH_MCK_SEL_POS);
            break;
        default:
            HalBachWriteReg(bank, E_BACH_I2S_CFG00, BACH_MCK_SEL_MSK, 2 << BACH_MCK_SEL_POS);
            ret |= _HalAudMckCalculate(eAioI2s, eMck);
            if (ret != AIO_OK)
            {
                ERRMSG("Func:%s, Line:%d eAioI2s %d eMck %d Fail !\n", __FUNCTION__, __LINE__, eAioI2s, eMck);
                goto FAIL;
            }
            break;
    }

    g_aI2sCfg[eAioI2s].eMck = eMck;

    return AIO_OK;

FAIL:

    return AIO_NG;
}

BOOL HalAudSetHpf(AudHpfDev_e eHfpDev, U8 level)
{
    switch (eHfpDev)
    {
        case E_AUD_HPF_ADC1_DMIC_2CH:
            g_nAdc1HpfLevel = level;
            break;
        case E_AUD_HPF_DMIC_4CH:
            g_nDmicHpfLevel = level;
            break;
        default:
            ERRMSG("Function - %s # %d - hpfdev = %d, max = %d, error !] \n", __func__, __LINE__, eHfpDev,
                   E_AUD_HPF_DEV_NUM - 1);
            return FALSE;
    }

    return TRUE;
}

// for I2S master
int HalAudI2sEnableBck(CHIP_AIO_I2S_e eAioI2s, BOOL bEn)
{
    U16 enTimeGen = BACH_ENABLE_CLK_NF_SYNTH_REF | BACH_CODEC_BCK_EN_TIME_GEN;

    if (bEn)
    {
        switch (eAioI2s)
        {
            case E_CHIP_AIO_I2S_RX_A:
            case E_CHIP_AIO_I2S_RX_C:
                HalBachWriteReg(E_BACH_REG_BANK2, E_BACH_NF_SYNTH_RX_BCK00, enTimeGen | BACH_CODEC_BCK_EN_SYNTH_TRIG,
                                enTimeGen);
                HalBachWriteReg(E_BACH_REG_BANK2, E_BACH_NF_SYNTH_RX_BCK00, enTimeGen | BACH_CODEC_BCK_EN_SYNTH_TRIG,
                                enTimeGen | BACH_CODEC_BCK_EN_SYNTH_TRIG);
                HalBachWriteReg(E_BACH_REG_BANK2, E_BACH_NF_SYNTH_RX_BCK00, enTimeGen | BACH_CODEC_BCK_EN_SYNTH_TRIG,
                                enTimeGen);
                break;
            case E_CHIP_AIO_I2S_RX_B:
            case E_CHIP_AIO_I2S_RX_D:
                HalBachWriteReg(E_BACH_REG_BANK4, E_BACH_NF_SYNTH_RX_BCK00, enTimeGen | BACH_CODEC_BCK_EN_SYNTH_TRIG,
                                enTimeGen);
                HalBachWriteReg(E_BACH_REG_BANK4, E_BACH_NF_SYNTH_RX_BCK00, enTimeGen | BACH_CODEC_BCK_EN_SYNTH_TRIG,
                                enTimeGen | BACH_CODEC_BCK_EN_SYNTH_TRIG);
                HalBachWriteReg(E_BACH_REG_BANK4, E_BACH_NF_SYNTH_RX_BCK00, enTimeGen | BACH_CODEC_BCK_EN_SYNTH_TRIG,
                                enTimeGen);

                break;
            case E_CHIP_AIO_I2S_TX_A:
                HalBachWriteReg(E_BACH_REG_BANK2, E_BACH_NF_SYNTH_TX_BCK00, enTimeGen | BACH_CODEC_BCK_EN_SYNTH_TRIG,
                                enTimeGen);
                HalBachWriteReg(E_BACH_REG_BANK2, E_BACH_NF_SYNTH_TX_BCK00, enTimeGen | BACH_CODEC_BCK_EN_SYNTH_TRIG,
                                enTimeGen | BACH_CODEC_BCK_EN_SYNTH_TRIG);
                HalBachWriteReg(E_BACH_REG_BANK2, E_BACH_NF_SYNTH_TX_BCK00, enTimeGen | BACH_CODEC_BCK_EN_SYNTH_TRIG,
                                enTimeGen);
                break;
            case E_CHIP_AIO_I2S_TX_B:
                HalBachWriteReg(E_BACH_REG_BANK4, E_BACH_NF_SYNTH_TX_BCK00, enTimeGen | BACH_CODEC_BCK_EN_SYNTH_TRIG,
                                enTimeGen);
                HalBachWriteReg(E_BACH_REG_BANK4, E_BACH_NF_SYNTH_TX_BCK00, enTimeGen | BACH_CODEC_BCK_EN_SYNTH_TRIG,
                                enTimeGen | BACH_CODEC_BCK_EN_SYNTH_TRIG);
                HalBachWriteReg(E_BACH_REG_BANK4, E_BACH_NF_SYNTH_TX_BCK00, enTimeGen | BACH_CODEC_BCK_EN_SYNTH_TRIG,
                                enTimeGen);
                break;
            default:
                ERRMSG("Func:%s, Line:%d eAioI2s %d Fail !\n", __FUNCTION__, __LINE__, eAioI2s);
                goto FAIL;
                break;
        }
    }
    else
    {
        switch (eAioI2s)
        {
            case E_CHIP_AIO_I2S_RX_A:
            case E_CHIP_AIO_I2S_RX_C:
                HalBachWriteReg(E_BACH_REG_BANK2, E_BACH_NF_SYNTH_RX_BCK00, enTimeGen | BACH_CODEC_BCK_EN_SYNTH_TRIG,
                                0);
                break;
            case E_CHIP_AIO_I2S_RX_B:
            case E_CHIP_AIO_I2S_RX_D:
                HalBachWriteReg(E_BACH_REG_BANK4, E_BACH_NF_SYNTH_RX_BCK00, enTimeGen | BACH_CODEC_BCK_EN_SYNTH_TRIG,
                                0);
                break;
            case E_CHIP_AIO_I2S_TX_A:
                HalBachWriteReg(E_BACH_REG_BANK2, E_BACH_NF_SYNTH_TX_BCK00, enTimeGen | BACH_CODEC_BCK_EN_SYNTH_TRIG,
                                0);
                break;
            case E_CHIP_AIO_I2S_TX_B:
                HalBachWriteReg(E_BACH_REG_BANK4, E_BACH_NF_SYNTH_TX_BCK00, enTimeGen | BACH_CODEC_BCK_EN_SYNTH_TRIG,
                                0);
                break;
            default:
                ERRMSG("Func:%s, Line:%d eAioI2s %d Fail !\n", __FUNCTION__, __LINE__, eAioI2s);
                goto FAIL;
                break;
        }
    }

    return AIO_OK;

FAIL:

    return AIO_NG;
}

int HalAudDmaSetRate(CHIP_AIO_DMA_e eAioDma, AudRate_e eRate)
{
    switch (eAioDma)
    {
        case E_CHIP_AIO_DMA_AI_A:
            switch (eRate)
            {
                case E_AUD_RATE_8K:
                    HalBachWriteReg(E_BACH_REG_BANK1, E_BACH_SR0_SEL, REG_WRITER_SEL_MSK, 0 << REG_WRITER_SEL_POS);
                    HalBachWriteReg(E_BACH_REG_BANK1, E_BACH_SR0_SEL, REG_CIC_3_SEL_MSK, 0 << REG_CIC_3_SEL_POS);
                    break;

                case E_AUD_RATE_16K:
                    HalBachWriteReg(E_BACH_REG_BANK1, E_BACH_SR0_SEL, REG_WRITER_SEL_MSK, 1 << REG_WRITER_SEL_POS);
                    HalBachWriteReg(E_BACH_REG_BANK1, E_BACH_SR0_SEL, REG_CIC_3_SEL_MSK, 1 << REG_CIC_3_SEL_POS);
                    break;

                case E_AUD_RATE_32K:
                    HalBachWriteReg(E_BACH_REG_BANK1, E_BACH_SR0_SEL, REG_WRITER_SEL_MSK, 2 << REG_WRITER_SEL_POS);
                    HalBachWriteReg(E_BACH_REG_BANK1, E_BACH_SR0_SEL, REG_CIC_3_SEL_MSK, 2 << REG_CIC_3_SEL_POS);
                    break;

                case E_AUD_RATE_48K:
                    HalBachWriteReg(E_BACH_REG_BANK1, E_BACH_SR0_SEL, REG_WRITER_SEL_MSK, 3 << REG_WRITER_SEL_POS);
                    HalBachWriteReg(E_BACH_REG_BANK1, E_BACH_SR0_SEL, REG_CIC_3_SEL_MSK, 3 << REG_CIC_3_SEL_POS);
                    break;

                default:
                    ERRMSG("Func:%s, Line:%d eAioDma %d eRate %d Fail !\n", __FUNCTION__, __LINE__, eAioDma, eRate);
                    goto FAIL;
            }

            _HalAudKeepDpgaGainCompatible(eRate);

            break;

        case E_CHIP_AIO_DMA_AO_A:
            switch (eRate)
            {
                case E_AUD_RATE_8K:
                    HalBachWriteReg(E_BACH_REG_BANK1, E_BACH_SR0_SEL, REG_SRC1_SEL_MSK, 0 << REG_SRC1_SEL_POS);
                    break;

                case E_AUD_RATE_11K:
                    HalBachWriteReg(E_BACH_REG_BANK1, E_BACH_SR0_SEL, REG_SRC1_SEL_MSK, 1 << REG_SRC1_SEL_POS);
                    break;

                case E_AUD_RATE_12K:
                    HalBachWriteReg(E_BACH_REG_BANK1, E_BACH_SR0_SEL, REG_SRC1_SEL_MSK, 2 << REG_SRC1_SEL_POS);
                    break;

                case E_AUD_RATE_16K:
                    HalBachWriteReg(E_BACH_REG_BANK1, E_BACH_SR0_SEL, REG_SRC1_SEL_MSK, 3 << REG_SRC1_SEL_POS);
                    break;

                case E_AUD_RATE_22K:
                    HalBachWriteReg(E_BACH_REG_BANK1, E_BACH_SR0_SEL, REG_SRC1_SEL_MSK, 4 << REG_SRC1_SEL_POS);
                    break;

                case E_AUD_RATE_24K:
                    HalBachWriteReg(E_BACH_REG_BANK1, E_BACH_SR0_SEL, REG_SRC1_SEL_MSK, 5 << REG_SRC1_SEL_POS);
                    break;

                case E_AUD_RATE_32K:
                    HalBachWriteReg(E_BACH_REG_BANK1, E_BACH_SR0_SEL, REG_SRC1_SEL_MSK, 6 << REG_SRC1_SEL_POS);
                    break;

                case E_AUD_RATE_44K:
                    HalBachWriteReg(E_BACH_REG_BANK1, E_BACH_SR0_SEL, REG_SRC1_SEL_MSK, 7 << REG_SRC1_SEL_POS);
                    break;

                case E_AUD_RATE_48K:
                    HalBachWriteReg(E_BACH_REG_BANK1, E_BACH_SR0_SEL, REG_SRC1_SEL_MSK, 8 << REG_SRC1_SEL_POS);
                    break;

                default:
                    ERRMSG("Func:%s, Line:%d eAioDma %d eRate %d Fail !\n", __FUNCTION__, __LINE__, eAioDma, eRate);
                    goto FAIL;
            }
            break;

        case E_CHIP_AIO_DMA_AI_B:
            switch (eRate)
            {
                case E_AUD_RATE_8K:
                    HalBachWriteReg(E_BACH_REG_BANK1, E_BACH_SR0_SEL, REG_CODEC_SEL_AU2_MSK,
                                    0 << REG_CODEC_SEL_AU2_POS);
                    HalBachWriteReg(E_BACH_REG_BANK1, E_BACH_SR0_SEL, REG_CIC_3_SEL_MSK, 0 << REG_CIC_3_SEL_POS);
                    break;

                case E_AUD_RATE_16K:
                    HalBachWriteReg(E_BACH_REG_BANK1, E_BACH_SR0_SEL, REG_CODEC_SEL_AU2_MSK,
                                    1 << REG_CODEC_SEL_AU2_POS);
                    HalBachWriteReg(E_BACH_REG_BANK1, E_BACH_SR0_SEL, REG_CIC_3_SEL_MSK, 1 << REG_CIC_3_SEL_POS);
                    break;

                case E_AUD_RATE_32K:
                    HalBachWriteReg(E_BACH_REG_BANK1, E_BACH_SR0_SEL, REG_CODEC_SEL_AU2_MSK,
                                    2 << REG_CODEC_SEL_AU2_POS);
                    HalBachWriteReg(E_BACH_REG_BANK1, E_BACH_SR0_SEL, REG_CIC_3_SEL_MSK, 2 << REG_CIC_3_SEL_POS);
                    break;

                case E_AUD_RATE_48K:
                    HalBachWriteReg(E_BACH_REG_BANK1, E_BACH_SR0_SEL, REG_CODEC_SEL_AU2_MSK,
                                    3 << REG_CODEC_SEL_AU2_POS);
                    HalBachWriteReg(E_BACH_REG_BANK1, E_BACH_SR0_SEL, REG_CIC_3_SEL_MSK, 3 << REG_CIC_3_SEL_POS);
                    break;

                default:
                    ERRMSG("Func:%s, Line:%d eAioDma %d eRate %d Fail !\n", __FUNCTION__, __LINE__, eAioDma, eRate);
                    goto FAIL;
            }

            _HalAudKeepDpgaGainCompatible(eRate);

            break;

        case E_CHIP_AIO_DMA_AO_B:
            switch (eRate)
            {
                case E_AUD_RATE_8K:
                    HalBachWriteReg(E_BACH_REG_BANK1, E_BACH_SR0_SEL, REG_SRC2_SEL_MSK, 0 << REG_SRC2_SEL_POS);
                    break;

                case E_AUD_RATE_11K:
                    HalBachWriteReg(E_BACH_REG_BANK1, E_BACH_SR0_SEL, REG_SRC2_SEL_MSK, 1 << REG_SRC2_SEL_POS);
                    break;

                case E_AUD_RATE_12K:
                    HalBachWriteReg(E_BACH_REG_BANK1, E_BACH_SR0_SEL, REG_SRC2_SEL_MSK, 2 << REG_SRC2_SEL_POS);
                    break;

                case E_AUD_RATE_16K:
                    HalBachWriteReg(E_BACH_REG_BANK1, E_BACH_SR0_SEL, REG_SRC2_SEL_MSK, 3 << REG_SRC2_SEL_POS);
                    break;

                case E_AUD_RATE_22K:
                    HalBachWriteReg(E_BACH_REG_BANK1, E_BACH_SR0_SEL, REG_SRC2_SEL_MSK, 4 << REG_SRC2_SEL_POS);
                    break;

                case E_AUD_RATE_24K:
                    HalBachWriteReg(E_BACH_REG_BANK1, E_BACH_SR0_SEL, REG_SRC2_SEL_MSK, 5 << REG_SRC2_SEL_POS);
                    break;

                case E_AUD_RATE_32K:
                    HalBachWriteReg(E_BACH_REG_BANK1, E_BACH_SR0_SEL, REG_SRC2_SEL_MSK, 6 << REG_SRC2_SEL_POS);
                    break;

                case E_AUD_RATE_44K:
                    HalBachWriteReg(E_BACH_REG_BANK1, E_BACH_SR0_SEL, REG_SRC2_SEL_MSK, 7 << REG_SRC2_SEL_POS);
                    break;

                case E_AUD_RATE_48K:
                    HalBachWriteReg(E_BACH_REG_BANK1, E_BACH_SR0_SEL, REG_SRC2_SEL_MSK, 8 << REG_SRC2_SEL_POS);
                    break;

                default:
                    ERRMSG("Func:%s, Line:%d eAioDma %d eRate %d Fail !\n", __FUNCTION__, __LINE__, eAioDma, eRate);
                    goto FAIL;
            }
            break;
            // need check
        case E_CHIP_AIO_DMA_AI_C:
            switch (eRate)
            {
                case E_AUD_RATE_8K:
                    HalBachWriteReg(E_BACH_REG_BANK1, E_BACH_SR0_SEL_AU2, REG_CIC_3_SEL_AU2_MSK,
                                    0 << REG_CIC_3_SEL_AU2_POS);
                    HalBachWriteReg(E_BACH_REG_BANK1, E_BACH_SR0_SEL_AU2, REG_CODEC_SEL_AU2_MSK,
                                    0 << REG_CODEC_SEL_AU2_POS);
                    break;

                case E_AUD_RATE_16K:
                    HalBachWriteReg(E_BACH_REG_BANK1, E_BACH_SR0_SEL_AU2, REG_CIC_3_SEL_AU2_MSK,
                                    1 << REG_CIC_3_SEL_AU2_POS);
                    HalBachWriteReg(E_BACH_REG_BANK1, E_BACH_SR0_SEL_AU2, REG_CODEC_SEL_AU2_MSK,
                                    1 << REG_CODEC_SEL_AU2_POS);
                    break;

                case E_AUD_RATE_32K:
                    HalBachWriteReg(E_BACH_REG_BANK1, E_BACH_SR0_SEL_AU2, REG_CIC_3_SEL_AU2_MSK,
                                    2 << REG_CIC_3_SEL_AU2_POS);
                    HalBachWriteReg(E_BACH_REG_BANK1, E_BACH_SR0_SEL_AU2, REG_CODEC_SEL_AU2_MSK,
                                    2 << REG_CODEC_SEL_AU2_POS);
                    break;

                case E_AUD_RATE_48K:
                    HalBachWriteReg(E_BACH_REG_BANK1, E_BACH_SR0_SEL_AU2, REG_CIC_3_SEL_AU2_MSK,
                                    3 << REG_CIC_3_SEL_AU2_POS);
                    HalBachWriteReg(E_BACH_REG_BANK1, E_BACH_SR0_SEL_AU2, REG_CODEC_SEL_AU2_MSK,
                                    3 << REG_CODEC_SEL_AU2_POS);
                    break;

                default:
                    ERRMSG("Func:%s, Line:%d eAioDma %d eRate %d Fail !\n", __FUNCTION__, __LINE__, eAioDma, eRate);
                    goto FAIL;
            }

            _HalAudKeepDpgaGainCompatible(eRate);

            break;

        case E_CHIP_AIO_DMA_AO_C:
            switch (eRate)
            {
                case E_AUD_RATE_8K:
                    HalBachWriteReg(E_BACH_REG_BANK4, E_BACH_DMA3_SRC_CFG, BACH_RDMA3_SAMPLE_RATE_MSK,
                                    0 << BACH_RDMA3_SAMPLE_RATE_POS);
                    break;

                case E_AUD_RATE_11K:
                    HalBachWriteReg(E_BACH_REG_BANK4, E_BACH_DMA3_SRC_CFG, BACH_RDMA3_SAMPLE_RATE_MSK,
                                    1 << BACH_RDMA3_SAMPLE_RATE_POS);
                    break;

                case E_AUD_RATE_12K:
                    HalBachWriteReg(E_BACH_REG_BANK4, E_BACH_DMA3_SRC_CFG, BACH_RDMA3_SAMPLE_RATE_MSK,
                                    2 << BACH_RDMA3_SAMPLE_RATE_POS);
                    break;

                case E_AUD_RATE_16K:
                    HalBachWriteReg(E_BACH_REG_BANK4, E_BACH_DMA3_SRC_CFG, BACH_RDMA3_SAMPLE_RATE_MSK,
                                    3 << BACH_RDMA3_SAMPLE_RATE_POS);
                    break;

                case E_AUD_RATE_22K:
                    HalBachWriteReg(E_BACH_REG_BANK4, E_BACH_DMA3_SRC_CFG, BACH_RDMA3_SAMPLE_RATE_MSK,
                                    4 << BACH_RDMA3_SAMPLE_RATE_POS);
                    break;

                case E_AUD_RATE_24K:
                    HalBachWriteReg(E_BACH_REG_BANK4, E_BACH_DMA3_SRC_CFG, BACH_RDMA3_SAMPLE_RATE_MSK,
                                    5 << BACH_RDMA3_SAMPLE_RATE_POS);
                    break;

                case E_AUD_RATE_32K:
                    HalBachWriteReg(E_BACH_REG_BANK4, E_BACH_DMA3_SRC_CFG, BACH_RDMA3_SAMPLE_RATE_MSK,
                                    6 << BACH_RDMA3_SAMPLE_RATE_POS);
                    break;

                case E_AUD_RATE_44K:
                    HalBachWriteReg(E_BACH_REG_BANK4, E_BACH_DMA3_SRC_CFG, BACH_RDMA3_SAMPLE_RATE_MSK,
                                    7 << BACH_RDMA3_SAMPLE_RATE_POS);
                    break;

                case E_AUD_RATE_48K:
                    HalBachWriteReg(E_BACH_REG_BANK4, E_BACH_DMA3_SRC_CFG, BACH_RDMA3_SAMPLE_RATE_MSK,
                                    8 << BACH_RDMA3_SAMPLE_RATE_POS);
                    break;

                default:
                    ERRMSG("Func:%s, Line:%d eAioDma %d eRate %d Fail !\n", __FUNCTION__, __LINE__, eAioDma, eRate);
                    goto FAIL;
            }
            break;

        case E_CHIP_AIO_DMA_AI_D:
            switch (eRate)
            {
                case E_AUD_RATE_8K:
                    HalBachWriteReg(E_BACH_REG_BANK1, E_BACH_SR0_SEL_AU2, REG_CIC_3_SEL_AU2_MSK,
                                    0 << REG_CIC_3_SEL_AU2_POS);
                    HalBachWriteReg(E_BACH_REG_BANK1, E_BACH_SR0_SEL_AU2, REG_CODEC_SEL_AU2_MSK,
                                    0 << REG_CODEC_SEL_AU2_POS);
                    break;

                case E_AUD_RATE_16K:
                    HalBachWriteReg(E_BACH_REG_BANK1, E_BACH_SR0_SEL_AU2, REG_CIC_3_SEL_AU2_MSK,
                                    1 << REG_CIC_3_SEL_AU2_POS);
                    HalBachWriteReg(E_BACH_REG_BANK1, E_BACH_SR0_SEL_AU2, REG_CODEC_SEL_AU2_MSK,
                                    1 << REG_CODEC_SEL_AU2_POS);
                    break;

                case E_AUD_RATE_32K:
                    HalBachWriteReg(E_BACH_REG_BANK1, E_BACH_SR0_SEL_AU2, REG_CIC_3_SEL_AU2_MSK,
                                    2 << REG_CIC_3_SEL_AU2_POS);
                    HalBachWriteReg(E_BACH_REG_BANK1, E_BACH_SR0_SEL_AU2, REG_CODEC_SEL_AU2_MSK,
                                    2 << REG_CODEC_SEL_AU2_POS);
                    break;

                case E_AUD_RATE_48K:
                    HalBachWriteReg(E_BACH_REG_BANK1, E_BACH_SR0_SEL_AU2, REG_CIC_3_SEL_AU2_MSK,
                                    3 << REG_CIC_3_SEL_AU2_POS);
                    HalBachWriteReg(E_BACH_REG_BANK1, E_BACH_SR0_SEL_AU2, REG_CODEC_SEL_AU2_MSK,
                                    3 << REG_CODEC_SEL_AU2_POS);
                    break;

                default:
                    ERRMSG("Func:%s, Line:%d eAioDma %d eRate %d Fail !\n", __FUNCTION__, __LINE__, eAioDma, eRate);
                    goto FAIL;
            }

            _HalAudKeepDpgaGainCompatible(eRate);

            break;

        case E_CHIP_AIO_DMA_AI_E:
            switch (eRate)
            {
                case E_AUD_RATE_8K:
                    HalBachWriteReg(E_BACH_REG_BANK1, E_BACH_SR0_SEL, REG_WRITER_SEL_MSK, 0 << REG_WRITER_SEL_POS);
                    HalBachWriteReg(E_BACH_REG_BANK1, E_BACH_SR0_SEL, REG_CIC_3_SEL_MSK, 0 << REG_CIC_3_SEL_POS);
                    break;

                case E_AUD_RATE_16K:
                    HalBachWriteReg(E_BACH_REG_BANK1, E_BACH_SR0_SEL, REG_WRITER_SEL_MSK, 1 << REG_WRITER_SEL_POS);
                    HalBachWriteReg(E_BACH_REG_BANK1, E_BACH_SR0_SEL, REG_CIC_3_SEL_MSK, 1 << REG_CIC_3_SEL_POS);
                    break;

                case E_AUD_RATE_32K:
                    HalBachWriteReg(E_BACH_REG_BANK1, E_BACH_SR0_SEL, REG_WRITER_SEL_MSK, 2 << REG_WRITER_SEL_POS);
                    HalBachWriteReg(E_BACH_REG_BANK1, E_BACH_SR0_SEL, REG_CIC_3_SEL_MSK, 2 << REG_CIC_3_SEL_POS);
                    break;

                case E_AUD_RATE_48K:
                    HalBachWriteReg(E_BACH_REG_BANK1, E_BACH_SR0_SEL, REG_WRITER_SEL_MSK, 3 << REG_WRITER_SEL_POS);
                    HalBachWriteReg(E_BACH_REG_BANK1, E_BACH_SR0_SEL, REG_CIC_3_SEL_MSK, 3 << REG_CIC_3_SEL_POS);
                    break;

                default:
                    ERRMSG("Func:%s, Line:%d eAioDma %d eRate %d Fail !\n", __FUNCTION__, __LINE__, eAioDma, eRate);
                    goto FAIL;
            }

            _HalAudKeepDpgaGainCompatible(eRate);

            break;

        default:
            ERRMSG("Func:%s, Line:%d eAioDma %d eRate %d Fail !\n", __FUNCTION__, __LINE__, eAioDma, eRate);
            goto FAIL;
    }

    return AIO_OK;

FAIL:

    return AIO_NG;
}

int HalAudDmaGetRate(CHIP_AIO_DMA_e eAioDma, AudRate_e *eRate, U32 *nRate)
{
    ERRMSG("Func:%s, Line:%d - have NOT support yet, warning !\n", __FUNCTION__, __LINE__);

    return AIO_OK;
}

int HalAudDmaReset(CHIP_AIO_DMA_e eAioDma)
{
    _Msleep(4);
    return _HalAudDmaReInit(eAioDma);
}

int HalAudDmaEnable(CHIP_AIO_DMA_e eAioDma, BOOL bEnable)
{
    switch (eAioDma)
    {
        case E_CHIP_AIO_DMA_AO_A:
            if (bEnable)
            {
                HalBachWriteReg(E_BACH_REG_BANK1, E_BACH_DMA1_CTRL_0, REG_ENABLE,
                                REG_ENABLE); // reader prefetch enable, it should be enabled before reader enable
                _UDelay(10);
                HalBachWriteReg(E_BACH_REG_BANK1, E_BACH_DMA1_CTRL_1, REG_RD_ENABLE, REG_RD_ENABLE);
            }
            else
            {
                HalBachWriteReg(E_BACH_REG_BANK1, E_BACH_DMA1_CTRL_1, REG_RD_ENABLE, 0);
                HalBachWriteReg(E_BACH_REG_BANK1, E_BACH_DMA1_CTRL_0, REG_ENABLE,
                                0); // reader prefetch enable, it has to be disabled before dma init
            }
            break;

        case E_CHIP_AIO_DMA_AO_B:
            if (bEnable)
            {
                HalBachWriteReg(E_BACH_REG_BANK1, E_BACH_DMA2_CTRL_0, REG_ENABLE,
                                REG_ENABLE); // reader prefetch enable, it should be enabled before reader enable
                _UDelay(10);
                HalBachWriteReg(E_BACH_REG_BANK1, E_BACH_DMA2_CTRL_1, REG_RD_ENABLE, REG_RD_ENABLE);
            }
            else
            {
                HalBachWriteReg(E_BACH_REG_BANK1, E_BACH_DMA2_CTRL_1, REG_RD_ENABLE, 0);
                HalBachWriteReg(E_BACH_REG_BANK1, E_BACH_DMA2_CTRL_0, REG_ENABLE,
                                0); // reader prefetch enable, it has to be disabled before dma init
            }
            break;

        case E_CHIP_AIO_DMA_AO_C:
            if (bEnable)
            {
                HalBachWriteReg(E_BACH_REG_BANK4, E_BACH_DMA3_CTRL_0, REG_ENABLE,
                                REG_ENABLE); // reader prefetch enable, it should be enabled before reader enable
                _UDelay(10);
                HalBachWriteReg(E_BACH_REG_BANK4, E_BACH_DMA3_CTRL_1, REG_RD_ENABLE, REG_RD_ENABLE);
            }
            else
            {
                HalBachWriteReg(E_BACH_REG_BANK4, E_BACH_DMA3_CTRL_1, REG_RD_ENABLE, 0);
                HalBachWriteReg(E_BACH_REG_BANK4, E_BACH_DMA3_CTRL_0, REG_ENABLE,
                                0); // reader prefetch enable, it has to be disabled before dma init
            }
            break;

        case E_CHIP_AIO_DMA_AI_A:
            HalBachWriteReg(E_BACH_REG_BANK1, E_BACH_DMA1_CTRL_9, REG_WR_ENABLE, (bEnable ? REG_WR_ENABLE : 0));
            break;

        case E_CHIP_AIO_DMA_AI_B:
            HalBachWriteReg(E_BACH_REG_BANK1, E_BACH_DMA2_CTRL_9, REG_WR_ENABLE, (bEnable ? REG_WR_ENABLE : 0));
            break;

        case E_CHIP_AIO_DMA_AI_C:
            HalBachWriteReg(E_BACH_REG_BANK4, E_BACH_DMA3_CTRL_9, REG_WR_ENABLE, (bEnable ? REG_WR_ENABLE : 0));
            break;

        case E_CHIP_AIO_DMA_AI_D:
            HalBachWriteReg(E_BACH_REG_BANK4, E_BACH_DMA4_CTRL_9, REG_WR_ENABLE, (bEnable ? REG_WR_ENABLE : 0));
            break;

        case E_CHIP_AIO_DMA_AI_E:
            HalBachWriteReg(E_BACH_REG_BANK4, E_BACH_DMA5_CTRL_9, REG_WR_ENABLE, (bEnable ? REG_WR_ENABLE : 0));
            break;

        default:
            ERRMSG("Func:%s, Line:%d eAioDma %d bEnable %d Fail !\n", __FUNCTION__, __LINE__, eAioDma, bEnable);
            goto FAIL;
            break;
    }

    return AIO_OK;

FAIL:

    return AIO_NG;
}

int HalAudDmaPause(CHIP_AIO_DMA_e eAioDma)
{
    return HalAudDmaEnable(eAioDma, FALSE);
}

int HalAudDmaResume(CHIP_AIO_DMA_e eAioDma)
{
    return HalAudDmaEnable(eAioDma, TRUE);
}

int HalAudDmaIntEnable(CHIP_AIO_DMA_e eAioDma, BOOL bDatatrigger, BOOL bDataboundary)
{
    int ret = AIO_OK;

    if (eAioDma < E_CHIP_AIO_DMA_AO_A)
    {
        ret |= _HalAudDmaWrIntEnable(eAioDma, bDatatrigger, bDataboundary);
    }
    else
    {
        ret |= _HalAudDmaRdIntEnable(eAioDma, bDatatrigger, bDataboundary);
    }

    if (ret != AIO_OK)
    {
        ERRMSG("Func:%s, Line:%d eAioDma %d bDatatrigger %d bDataboundary %d Fail !\n", __FUNCTION__, __LINE__, eAioDma,
               bDatatrigger, bDataboundary);
        goto FAIL;
    }

    return AIO_OK;

FAIL:

    return AIO_NG;
}

int HalAudDmaGetInt(CHIP_AIO_DMA_e eAioDma, BOOL *bDatatrigger, BOOL *bDataboundary)
{
    int ret = AIO_OK;

    if (eAioDma < E_CHIP_AIO_DMA_AO_A)
    {
        ret |= _HalAudDmaGetWrInt(eAioDma, bDatatrigger, bDataboundary);
    }
    else
    {
        ret |= _HalAudDmaGetRdInt(eAioDma, bDatatrigger, bDataboundary);
    }

    if (ret != AIO_OK)
    {
        ERRMSG("Func:%s, Line:%d eAioDma %d Fail !\n", __FUNCTION__, __LINE__, eAioDma);
        goto FAIL;
    }

    return AIO_OK;

FAIL:

    return AIO_NG;
}

int HalAudDmaClearInt(CHIP_AIO_DMA_e eAioDma)
{
    switch (eAioDma)
    {
        case E_CHIP_AIO_DMA_AI_A:
            // DMA writer1 full flag clear / DMA writer1 local buffer full flag clear
            HalBachWriteReg(E_BACH_REG_BANK1, E_BACH_DMA1_CTRL_0, REG_WR_FULL_FLAG_CLR, REG_WR_FULL_FLAG_CLR);
            HalBachWriteReg(E_BACH_REG_BANK1, E_BACH_DMA1_CTRL_0, REG_WR_FULL_FLAG_CLR, 0);
            break;

        case E_CHIP_AIO_DMA_AO_A:
            // DMA reader1 empty flag clear / DMA reader1 local buffer empty flag clear
            HalBachWriteReg(E_BACH_REG_BANK1, E_BACH_DMA1_CTRL_0, REG_RD_EMPTY_FLAG_CLR, REG_RD_EMPTY_FLAG_CLR);
            HalBachWriteReg(E_BACH_REG_BANK1, E_BACH_DMA1_CTRL_0, REG_RD_EMPTY_FLAG_CLR, 0);
            break;

        case E_CHIP_AIO_DMA_AI_B:
            // DMA writer2 full flag clear / DMA writer2 local buffer full flag clear
            HalBachWriteReg(E_BACH_REG_BANK1, E_BACH_DMA2_CTRL_0, REG_WR_FULL_FLAG_CLR, REG_WR_FULL_FLAG_CLR);
            HalBachWriteReg(E_BACH_REG_BANK1, E_BACH_DMA2_CTRL_0, REG_WR_FULL_FLAG_CLR, 0);
            break;

        case E_CHIP_AIO_DMA_AO_B:
            // DMA reader2 empty flag clear / DMA reader2 local buffer empty flag clear
            HalBachWriteReg(E_BACH_REG_BANK1, E_BACH_DMA2_CTRL_0, REG_RD_EMPTY_FLAG_CLR, REG_RD_EMPTY_FLAG_CLR);
            HalBachWriteReg(E_BACH_REG_BANK1, E_BACH_DMA2_CTRL_0, REG_RD_EMPTY_FLAG_CLR, 0);
            break;

        case E_CHIP_AIO_DMA_AI_C:
            // DMA writer3 full flag clear / DMA writer3 local buffer full flag clear
            HalBachWriteReg(E_BACH_REG_BANK4, E_BACH_DMA3_CTRL_0, REG_WR_FULL_FLAG_CLR, REG_WR_FULL_FLAG_CLR);
            HalBachWriteReg(E_BACH_REG_BANK4, E_BACH_DMA3_CTRL_0, REG_WR_FULL_FLAG_CLR, 0);
            break;

        case E_CHIP_AIO_DMA_AO_C:
            // DMA reader3 empty flag clear / DMA reader3 local buffer empty flag clear
            HalBachWriteReg(E_BACH_REG_BANK4, E_BACH_DMA3_CTRL_0, REG_RD_EMPTY_FLAG_CLR, REG_RD_EMPTY_FLAG_CLR);
            HalBachWriteReg(E_BACH_REG_BANK4, E_BACH_DMA3_CTRL_0, REG_RD_EMPTY_FLAG_CLR, 0);
            break;

        case E_CHIP_AIO_DMA_AI_D:
            // DMA writer3 full flag clear / DMA writer3 local buffer full flag clear
            HalBachWriteReg(E_BACH_REG_BANK4, E_BACH_DMA4_CTRL_0, REG_WR_FULL_FLAG_CLR, REG_WR_FULL_FLAG_CLR);
            HalBachWriteReg(E_BACH_REG_BANK4, E_BACH_DMA4_CTRL_0, REG_WR_FULL_FLAG_CLR, 0);
            break;

        case E_CHIP_AIO_DMA_AI_E:
            // DMA writer3 full flag clear / DMA writer3 local buffer full flag clear
            HalBachWriteReg(E_BACH_REG_BANK4, E_BACH_DMA5_CTRL_0, REG_WR_FULL_FLAG_CLR, REG_WR_FULL_FLAG_CLR);
            HalBachWriteReg(E_BACH_REG_BANK4, E_BACH_DMA5_CTRL_0, REG_WR_FULL_FLAG_CLR, 0);
            break;

        default:
            ERRMSG("Func:%s, Line:%d eAioDma %d Fail !\n", __FUNCTION__, __LINE__, eAioDma);
            goto FAIL;
            break;
    }

    return AIO_OK;

FAIL:

    return AIO_NG;
}

U32 HalAudDmaGetLevelCnt(CHIP_AIO_DMA_e eAioDma)
{
    U32 nRawMiuUnitLevelCnt;

    //
    nRawMiuUnitLevelCnt = _HalAudDmaGetRawMiuUnitLevelCnt(eAioDma);
    //
    if (eAioDma < E_CHIP_AIO_DMA_AO_A) // dma writer
    {
        nRawMiuUnitLevelCnt =
            ((nRawMiuUnitLevelCnt > DMA_LOCALBUF_LINE) ? (nRawMiuUnitLevelCnt - DMA_LOCALBUF_LINE)
                                                       : 0); // level count contains the local buffer data size
    }
    else // dma reader
    {
        nRawMiuUnitLevelCnt = ((nRawMiuUnitLevelCnt <= DMA_EMPTY_THD) ? 0 : nRawMiuUnitLevelCnt);
    }

    return (nRawMiuUnitLevelCnt * MIU_WORD_BYTE_SIZE);
}

U32 HalAudDmaGetRawLevelCnt(CHIP_AIO_DMA_e eAioDma)
{
    U32 nRawMiuUnitLevelCnt;

    //
    nRawMiuUnitLevelCnt = _HalAudDmaGetRawMiuUnitLevelCnt(eAioDma);

    return (nRawMiuUnitLevelCnt * MIU_WORD_BYTE_SIZE);
}

U32 HalAudDmaTrigLevelCnt(CHIP_AIO_DMA_e eAioDma, U32 nDataSize)
{
    U16 nConfigValue = 0;
    nConfigValue     = (U16)((nDataSize / MIU_WORD_BYTE_SIZE) & REG_WR_SIZE_MSK);
    nDataSize        = nConfigValue * MIU_WORD_BYTE_SIZE;

    if (nConfigValue > 0)
    {
        switch (eAioDma)
        {
            case E_CHIP_AIO_DMA_AI_A:
                HalBachWriteReg(E_BACH_REG_BANK1, E_BACH_DMA1_CTRL_12, 0xFFFF, nConfigValue);
                nConfigValue = HalBachReadReg(E_BACH_REG_BANK1, E_BACH_DMA1_CTRL_9);
                if (nConfigValue & REG_WR_TRIG)
                {
                    HalBachWriteReg(E_BACH_REG_BANK1, E_BACH_DMA1_CTRL_9, REG_WR_TRIG, 0);
                }
                else
                {
                    HalBachWriteReg(E_BACH_REG_BANK1, E_BACH_DMA1_CTRL_9, REG_WR_TRIG, REG_WR_TRIG);
                }
                break;

            case E_CHIP_AIO_DMA_AO_A:
                HalBachWriteReg(E_BACH_REG_BANK1, E_BACH_DMA1_CTRL_4, 0xFFFF, nConfigValue);
                nConfigValue = HalBachReadReg(E_BACH_REG_BANK1, E_BACH_DMA1_CTRL_1);
                if (nConfigValue & REG_RD_TRIG)
                {
                    HalBachWriteReg(E_BACH_REG_BANK1, E_BACH_DMA1_CTRL_1, REG_RD_TRIG, 0);
                }
                else
                {
                    HalBachWriteReg(E_BACH_REG_BANK1, E_BACH_DMA1_CTRL_1, REG_RD_TRIG, REG_RD_TRIG);
                }
                break;

            case E_CHIP_AIO_DMA_AI_B:
                HalBachWriteReg(E_BACH_REG_BANK1, E_BACH_DMA2_CTRL_12, 0xFFFF, nConfigValue);
                nConfigValue = HalBachReadReg(E_BACH_REG_BANK1, E_BACH_DMA2_CTRL_9);
                if (nConfigValue & REG_WR_TRIG)
                {
                    HalBachWriteReg(E_BACH_REG_BANK1, E_BACH_DMA2_CTRL_9, REG_WR_TRIG, 0);
                }
                else
                {
                    HalBachWriteReg(E_BACH_REG_BANK1, E_BACH_DMA2_CTRL_9, REG_WR_TRIG, REG_WR_TRIG);
                }
                break;

            case E_CHIP_AIO_DMA_AO_B:
                HalBachWriteReg(E_BACH_REG_BANK1, E_BACH_DMA2_CTRL_4, 0xFFFF, nConfigValue);
                nConfigValue = HalBachReadReg(E_BACH_REG_BANK1, E_BACH_DMA2_CTRL_1);
                if (nConfigValue & REG_RD_TRIG)
                {
                    HalBachWriteReg(E_BACH_REG_BANK1, E_BACH_DMA2_CTRL_1, REG_RD_TRIG, 0);
                }
                else
                {
                    HalBachWriteReg(E_BACH_REG_BANK1, E_BACH_DMA2_CTRL_1, REG_RD_TRIG, REG_RD_TRIG);
                }
                break;

            case E_CHIP_AIO_DMA_AI_C:
                HalBachWriteReg(E_BACH_REG_BANK4, E_BACH_DMA3_CTRL_12, 0xFFFF, nConfigValue);
                nConfigValue = HalBachReadReg(E_BACH_REG_BANK4, E_BACH_DMA3_CTRL_9);
                if (nConfigValue & REG_WR_TRIG)
                {
                    HalBachWriteReg(E_BACH_REG_BANK4, E_BACH_DMA3_CTRL_9, REG_WR_TRIG, 0);
                }
                else
                {
                    HalBachWriteReg(E_BACH_REG_BANK4, E_BACH_DMA3_CTRL_9, REG_WR_TRIG, REG_WR_TRIG);
                }
                break;

            case E_CHIP_AIO_DMA_AO_C:
                HalBachWriteReg(E_BACH_REG_BANK4, E_BACH_DMA3_CTRL_4, 0xFFFF, nConfigValue);
                nConfigValue = HalBachReadReg(E_BACH_REG_BANK4, E_BACH_DMA3_CTRL_1);
                if (nConfigValue & REG_RD_TRIG)
                {
                    HalBachWriteReg(E_BACH_REG_BANK4, E_BACH_DMA3_CTRL_1, REG_RD_TRIG, 0);
                }
                else
                {
                    HalBachWriteReg(E_BACH_REG_BANK4, E_BACH_DMA3_CTRL_1, REG_RD_TRIG, REG_RD_TRIG);
                }
                break;

            case E_CHIP_AIO_DMA_AI_D:
                HalBachWriteReg(E_BACH_REG_BANK4, E_BACH_DMA4_CTRL_02, 0xFFFF, nConfigValue);
                nConfigValue = HalBachReadReg(E_BACH_REG_BANK4, E_BACH_DMA4_CTRL_9);
                if (nConfigValue & REG_WR_TRIG)
                {
                    HalBachWriteReg(E_BACH_REG_BANK4, E_BACH_DMA4_CTRL_9, REG_WR_TRIG, 0);
                }
                else
                {
                    HalBachWriteReg(E_BACH_REG_BANK4, E_BACH_DMA4_CTRL_9, REG_WR_TRIG, REG_WR_TRIG);
                }
                break;

            case E_CHIP_AIO_DMA_AI_E:
                HalBachWriteReg(E_BACH_REG_BANK4, E_BACH_DMA5_CTRL_12, 0xFFFF, nConfigValue);
                nConfigValue = HalBachReadReg(E_BACH_REG_BANK4, E_BACH_DMA5_CTRL_9);
                if (nConfigValue & REG_WR_TRIG)
                {
                    HalBachWriteReg(E_BACH_REG_BANK4, E_BACH_DMA5_CTRL_9, REG_WR_TRIG, 0);
                }
                else
                {
                    HalBachWriteReg(E_BACH_REG_BANK4, E_BACH_DMA5_CTRL_9, REG_WR_TRIG, REG_WR_TRIG);
                }
                break;

            default:
                ERRMSG("Func:%s, Line:%d eAioDma %d Fail !\n", __FUNCTION__, __LINE__, eAioDma);
                return 0;
        }

        return nDataSize;
    }

    return 0;
}

int HalAudDmaSetPhyAddr(CHIP_AIO_DMA_e eAioDma, long nBufAddrOffset, U32 nBufSize)
{
    int ret = AIO_OK;
    U16 nMiuAddrLo, nMiuAddrHi, nMiuAddrExtra, nMiuSize;

    if ((nBufSize / MIU_WORD_BYTE_SIZE) > ((1 << 16) - 1))
    {
        ERRMSG("Func:%s, Line:%d eAioDma %d, BufSize %d BufSize overflow ! Fail !] \n", __FUNCTION__, __LINE__, eAioDma,
               nBufSize);
        goto FAIL;
    }

    if ((nBufSize % MIU_WORD_BYTE_SIZE) != 0)
    {
        ERRMSG("Func:%s, Line:%d eAioDma %d, BufSize %d Buf size must be multiple of MIU_WORD_BYTE_SIZE ! Fail !] \n",
               __FUNCTION__, __LINE__, eAioDma, nBufSize);
        goto FAIL;
    }

    if ((nBufAddrOffset % MIU_WORD_BYTE_SIZE) != 0)
    {
        ERRMSG(
            "Func:%s, Line:%d eAioDma %d, nBufAddrOffset %d Buf addr must be multiple of MIU_WORD_BYTE_SIZE ! Fail !] "
            "\n",
            __FUNCTION__, __LINE__, eAioDma, nBufAddrOffset);
        goto FAIL;
    }

    ret |= _HalAudDmaSetMiuSel(eAioDma, nBufAddrOffset);
    if (ret != AIO_OK)
    {
        ERRMSG("Func:%s, Line:%d eAioDma %d Fail !\n", __FUNCTION__, __LINE__, eAioDma);
        goto FAIL;
    }
    // flow need check
    switch (eAioDma)
    {
        case E_CHIP_AIO_DMA_AI_A:
            nMiuAddrLo = (U16)((nBufAddrOffset / MIU_WORD_BYTE_SIZE) & REG_WR_BASE_ADDR_LO_MSK);
            nMiuAddrHi =
                (U16)(((nBufAddrOffset / MIU_WORD_BYTE_SIZE) >> REG_WR_BASE_ADDR_HI_OFFSET) & REG_WR_BASE_ADDR_HI_MSK);
            nMiuAddrExtra = (U16)((nBufAddrOffset / MIU_WORD_BYTE_SIZE) >> REG_WR_BASE_ADDR_ADDITION_OFFSET);
            nMiuSize      = (U16)((nBufSize / MIU_WORD_BYTE_SIZE) & REG_WR_BUFF_SIZE_MSK);

            HalBachWriteReg(E_BACH_REG_BANK1, E_BACH_DMA1_CTRL_9, REG_WR_BASE_ADDR_LO_MSK, nMiuAddrLo);
            HalBachWriteReg(E_BACH_REG_BANK1, E_BACH_DMA1_CTRL_10, REG_WR_BASE_ADDR_HI_MSK, nMiuAddrHi);
            HalBachWriteReg(E_BACH_REG_BANK1, E_BACH_DMA_TEST_CTRL_18, REG_WR1_BASE_ADDR_ADDITION_MSK,
                            nMiuAddrExtra << REG_WR1_BASE_ADDR_ADDITION_POS);
            HalBachWriteReg(E_BACH_REG_BANK1, E_BACH_DMA1_CTRL_11, 0xFFFF, nMiuSize);
            break;

        case E_CHIP_AIO_DMA_AO_A:
            nMiuAddrLo = (U16)((nBufAddrOffset / MIU_WORD_BYTE_SIZE) & REG_RD_BASE_ADDR_LO_MSK);
            nMiuAddrHi =
                (U16)(((nBufAddrOffset / MIU_WORD_BYTE_SIZE) >> REG_RD_BASE_ADDR_HI_OFFSET) & REG_RD_BASE_ADDR_HI_MSK);
            nMiuSize      = (U16)((nBufSize / MIU_WORD_BYTE_SIZE) & REG_RD_BUFF_SIZE_MSK);
            nMiuAddrExtra = (U16)((nBufAddrOffset / MIU_WORD_BYTE_SIZE) >> REG_RD_BASE_ADDR_ADDITION_OFFSET);

            HalBachWriteReg(E_BACH_REG_BANK1, E_BACH_DMA1_CTRL_1, REG_RD_BASE_ADDR_LO_MSK, nMiuAddrLo);
            HalBachWriteReg(E_BACH_REG_BANK1, E_BACH_DMA1_CTRL_2, REG_RD_BASE_ADDR_HI_MSK, nMiuAddrHi);
            HalBachWriteReg(E_BACH_REG_BANK1, E_BACH_DMA_TEST_CTRL_18, REG_RD1_BASE_ADDR_ADDITION_MSK,
                            nMiuAddrExtra << REG_RD1_BASE_ADDR_ADDITION_POS);
            HalBachWriteReg(E_BACH_REG_BANK1, E_BACH_DMA1_CTRL_3, 0xFFFF, nMiuSize);
            break;

        case E_CHIP_AIO_DMA_AI_B:
            nMiuAddrLo = (U16)((nBufAddrOffset / MIU_WORD_BYTE_SIZE) & REG_WR_BASE_ADDR_LO_MSK);
            nMiuAddrHi =
                (U16)(((nBufAddrOffset / MIU_WORD_BYTE_SIZE) >> REG_WR_BASE_ADDR_HI_OFFSET) & REG_WR_BASE_ADDR_HI_MSK);
            nMiuAddrExtra = (U16)((nBufAddrOffset / MIU_WORD_BYTE_SIZE) >> REG_WR_BASE_ADDR_ADDITION_OFFSET);
            nMiuSize      = (U16)((nBufSize / MIU_WORD_BYTE_SIZE) & REG_WR_BUFF_SIZE_MSK);

            HalBachWriteReg(E_BACH_REG_BANK1, E_BACH_DMA2_CTRL_9, REG_WR_BASE_ADDR_LO_MSK, nMiuAddrLo);
            HalBachWriteReg(E_BACH_REG_BANK1, E_BACH_DMA2_CTRL_10, REG_WR_BASE_ADDR_HI_MSK, nMiuAddrHi);
            HalBachWriteReg(E_BACH_REG_BANK1, E_BACH_DMA_TEST_CTRL_18, REG_WR2_BASE_ADDR_ADDITION_MSK,
                            nMiuAddrExtra << REG_WR2_BASE_ADDR_ADDITION_POS);
            HalBachWriteReg(E_BACH_REG_BANK1, E_BACH_DMA2_CTRL_11, 0xFFFF, nMiuSize);
            break;

        case E_CHIP_AIO_DMA_AO_B:
            nMiuAddrLo = (U16)((nBufAddrOffset / MIU_WORD_BYTE_SIZE) & REG_RD_BASE_ADDR_LO_MSK);
            nMiuAddrHi =
                (U16)(((nBufAddrOffset / MIU_WORD_BYTE_SIZE) >> REG_RD_BASE_ADDR_HI_OFFSET) & REG_RD_BASE_ADDR_HI_MSK);
            nMiuAddrExtra = (U16)((nBufAddrOffset / MIU_WORD_BYTE_SIZE) >> REG_RD_BASE_ADDR_ADDITION_OFFSET);
            nMiuSize      = (U16)((nBufSize / MIU_WORD_BYTE_SIZE) & REG_RD_BUFF_SIZE_MSK);

            HalBachWriteReg(E_BACH_REG_BANK1, E_BACH_DMA2_CTRL_1, REG_RD_BASE_ADDR_LO_MSK, nMiuAddrLo);
            HalBachWriteReg(E_BACH_REG_BANK1, E_BACH_DMA2_CTRL_2, REG_RD_BASE_ADDR_HI_MSK, nMiuAddrHi);
            HalBachWriteReg(E_BACH_REG_BANK1, E_BACH_DMA_TEST_CTRL_18, REG_RD2_BASE_ADDR_ADDITION_MSK,
                            nMiuAddrExtra << REG_RD2_BASE_ADDR_ADDITION_POS);
            HalBachWriteReg(E_BACH_REG_BANK1, E_BACH_DMA2_CTRL_3, 0xFFFF, nMiuSize);
            break;

        case E_CHIP_AIO_DMA_AI_C:
            nMiuAddrLo = (U16)((nBufAddrOffset / MIU_WORD_BYTE_SIZE) & REG_WR_BASE_ADDR_LO_MSK);
            nMiuAddrHi =
                (U16)(((nBufAddrOffset / MIU_WORD_BYTE_SIZE) >> REG_WR_BASE_ADDR_HI_OFFSET) & REG_WR_BASE_ADDR_HI_MSK);
            nMiuAddrExtra = (U16)((nBufAddrOffset / MIU_WORD_BYTE_SIZE) >> REG_WR_BASE_ADDR_ADDITION_OFFSET);
            nMiuSize      = (U16)((nBufSize / MIU_WORD_BYTE_SIZE) & REG_WR_BUFF_SIZE_MSK);

            HalBachWriteReg(E_BACH_REG_BANK4, E_BACH_DMA3_CTRL_9, REG_WR_BASE_ADDR_LO_MSK, nMiuAddrLo);
            HalBachWriteReg(E_BACH_REG_BANK4, E_BACH_DMA3_CTRL_10, REG_WR_BASE_ADDR_HI_MSK, nMiuAddrHi);
            HalBachWriteReg(E_BACH_REG_BANK4, E_BACH_DMA3_CTRL_16, REG_WR_BASE_ADDR_ADD_MSK,
                            nMiuAddrExtra << REG_WR_BASE_ADDR_ADD_POS);
            HalBachWriteReg(E_BACH_REG_BANK4, E_BACH_DMA3_CTRL_11, 0xFFFF, nMiuSize);
            break;

        case E_CHIP_AIO_DMA_AO_C:
            nMiuAddrLo = (U16)((nBufAddrOffset / MIU_WORD_BYTE_SIZE) & REG_RD_BASE_ADDR_LO_MSK);
            nMiuAddrHi =
                (U16)(((nBufAddrOffset / MIU_WORD_BYTE_SIZE) >> REG_RD_BASE_ADDR_HI_OFFSET) & REG_RD_BASE_ADDR_HI_MSK);
            nMiuAddrExtra = (U16)((nBufAddrOffset / MIU_WORD_BYTE_SIZE) >> REG_RD_BASE_ADDR_ADDITION_OFFSET);
            nMiuSize      = (U16)((nBufSize / MIU_WORD_BYTE_SIZE) & REG_RD_BUFF_SIZE_MSK);

            HalBachWriteReg(E_BACH_REG_BANK4, E_BACH_DMA3_CTRL_1, REG_RD_BASE_ADDR_LO_MSK, nMiuAddrLo);
            HalBachWriteReg(E_BACH_REG_BANK4, E_BACH_DMA3_CTRL_2, REG_RD_BASE_ADDR_HI_MSK, nMiuAddrHi);
            HalBachWriteReg(E_BACH_REG_BANK4, E_BACH_DMA3_CTRL_16, REG_RD_BASE_ADDR_ADD_MSK,
                            nMiuAddrExtra << REG_RD_BASE_ADDR_ADD_POS);
            HalBachWriteReg(E_BACH_REG_BANK4, E_BACH_DMA3_CTRL_3, 0xFFFF, nMiuSize);
            break;

        case E_CHIP_AIO_DMA_AI_D:
            nMiuAddrLo = (U16)((nBufAddrOffset / MIU_WORD_BYTE_SIZE) & REG_WR_BASE_ADDR_LO_MSK);
            nMiuAddrHi =
                (U16)(((nBufAddrOffset / MIU_WORD_BYTE_SIZE) >> REG_WR_BASE_ADDR_HI_OFFSET) & REG_WR_BASE_ADDR_HI_MSK);
            nMiuAddrExtra = (U16)((nBufAddrOffset / MIU_WORD_BYTE_SIZE) >> REG_WR_BASE_ADDR_ADDITION_OFFSET);
            nMiuSize      = (U16)((nBufSize / MIU_WORD_BYTE_SIZE) & REG_WR_BUFF_SIZE_MSK);

            HalBachWriteReg(E_BACH_REG_BANK4, E_BACH_DMA4_CTRL_9, REG_WR_BASE_ADDR_LO_MSK, nMiuAddrLo);
            HalBachWriteReg(E_BACH_REG_BANK4, E_BACH_DMA4_CTRL_00, REG_WR_BASE_ADDR_HI_MSK, nMiuAddrHi);
            HalBachWriteReg(E_BACH_REG_BANK4, E_BACH_DMA4_CTRL_06, REG_WR_BASE_ADDR_ADD_MSK,
                            nMiuAddrExtra << REG_WR_BASE_ADDR_ADD_POS);
            HalBachWriteReg(E_BACH_REG_BANK4, E_BACH_DMA4_CTRL_01, 0xFFFF, nMiuSize);
            break;

        case E_CHIP_AIO_DMA_AI_E:
            nMiuAddrLo = (U16)((nBufAddrOffset / MIU_WORD_BYTE_SIZE) & REG_WR_BASE_ADDR_LO_MSK);
            nMiuAddrHi =
                (U16)(((nBufAddrOffset / MIU_WORD_BYTE_SIZE) >> REG_WR_BASE_ADDR_HI_OFFSET) & REG_WR_BASE_ADDR_HI_MSK);
            nMiuAddrExtra = (U16)((nBufAddrOffset / MIU_WORD_BYTE_SIZE) >> REG_WR_BASE_ADDR_ADDITION_OFFSET);
            nMiuSize      = (U16)((nBufSize / MIU_WORD_BYTE_SIZE) & REG_WR_BUFF_SIZE_MSK);

            HalBachWriteReg(E_BACH_REG_BANK4, E_BACH_DMA5_CTRL_9, REG_WR_BASE_ADDR_LO_MSK, nMiuAddrLo);
            HalBachWriteReg(E_BACH_REG_BANK4, E_BACH_DMA5_CTRL_10, REG_WR_BASE_ADDR_HI_MSK, nMiuAddrHi);
            HalBachWriteReg(E_BACH_REG_BANK4, E_BACH_DMA5_CTRL_16, REG_WR_BASE_ADDR_ADD_MSK,
                            nMiuAddrExtra << REG_WR_BASE_ADDR_ADD_POS);
            HalBachWriteReg(E_BACH_REG_BANK4, E_BACH_DMA5_CTRL_11, 0xFFFF, nMiuSize);
            break;

        default:
            ERRMSG("Func:%s, Line:%d eAioDma %d Fail !\n", __FUNCTION__, __LINE__, eAioDma);
            goto FAIL;
            break;
    }

    return AIO_OK;

FAIL:

    return AIO_NG;
}

int HalAudDmaAiSetThreshold(CHIP_AIO_DMA_e eAioDma, U32 nOverrunTh)
{
    U16 nMiuOverrunTh;

    if ((nOverrunTh < MIU_WORD_BYTE_SIZE) || (nOverrunTh > (0xFFFFUL * MIU_WORD_BYTE_SIZE)))
    {
        ERRMSG("Func:%s, Line:%d eAioDma %d nOverrunTh %d Fail !\n", __FUNCTION__, __LINE__, eAioDma, nOverrunTh);
        goto FAIL;
    }

    switch (eAioDma)
    {
        case E_CHIP_AIO_DMA_AI_A:
            nMiuOverrunTh = (U16)((nOverrunTh / MIU_WORD_BYTE_SIZE) & REG_WR_OVERRUN_TH_MSK);
            HalBachWriteReg(E_BACH_REG_BANK1, E_BACH_DMA1_CTRL_13, 0xFFFF, nMiuOverrunTh);
            break;

        case E_CHIP_AIO_DMA_AI_B:
            nMiuOverrunTh = (U16)((nOverrunTh / MIU_WORD_BYTE_SIZE) & REG_WR_OVERRUN_TH_MSK);
            HalBachWriteReg(E_BACH_REG_BANK1, E_BACH_DMA2_CTRL_13, 0xFFFF, nMiuOverrunTh);
            break;

        case E_CHIP_AIO_DMA_AI_C:
            nMiuOverrunTh = (U16)((nOverrunTh / MIU_WORD_BYTE_SIZE) & REG_WR_OVERRUN_TH_MSK);
            HalBachWriteReg(E_BACH_REG_BANK4, E_BACH_DMA3_CTRL_13, 0xFFFF, nMiuOverrunTh);
            break;

        case E_CHIP_AIO_DMA_AI_D:
            nMiuOverrunTh = (U16)((nOverrunTh / MIU_WORD_BYTE_SIZE) & REG_WR_OVERRUN_TH_MSK);
            HalBachWriteReg(E_BACH_REG_BANK4, E_BACH_DMA4_CTRL_03, 0xFFFF, nMiuOverrunTh);
            break;

        case E_CHIP_AIO_DMA_AI_E:
            nMiuOverrunTh = (U16)((nOverrunTh / MIU_WORD_BYTE_SIZE) & REG_WR_OVERRUN_TH_MSK);
            HalBachWriteReg(E_BACH_REG_BANK4, E_BACH_DMA5_CTRL_13, 0xFFFF, nMiuOverrunTh);
            break;

        default:
            ERRMSG("Func:%s, Line:%d eAioDma %d Fail !\n", __FUNCTION__, __LINE__, eAioDma);
            goto FAIL;
            break;
    }

    return AIO_OK;

FAIL:

    return AIO_NG;
}

int HalAudDmaAoSetThreshold(CHIP_AIO_DMA_e eAioDma, U32 nUnderrunTh)
{
    U16 nMiuUnderrunTh;

    if ((nUnderrunTh < MIU_WORD_BYTE_SIZE) || (nUnderrunTh > (0xFFFFUL * MIU_WORD_BYTE_SIZE)))
    {
        ERRMSG("Func:%s, Line:%d eAioDma %d nUnderrunTh %d Fail !\n", __FUNCTION__, __LINE__, eAioDma, nUnderrunTh);
        goto FAIL;
    }

    switch (eAioDma)
    {
        case E_CHIP_AIO_DMA_AO_A:
            nMiuUnderrunTh = (U16)((nUnderrunTh / MIU_WORD_BYTE_SIZE) & REG_RD_UNDERRUN_TH_MSK);
            HalBachWriteReg(E_BACH_REG_BANK1, E_BACH_DMA1_CTRL_6, 0xFFFF, nMiuUnderrunTh);
            break;

        case E_CHIP_AIO_DMA_AO_B:
            nMiuUnderrunTh = (U16)((nUnderrunTh / MIU_WORD_BYTE_SIZE) & REG_RD_UNDERRUN_TH_MSK);
            HalBachWriteReg(E_BACH_REG_BANK1, E_BACH_DMA2_CTRL_6, 0xFFFF, nMiuUnderrunTh);
            break;

        case E_CHIP_AIO_DMA_AO_C:
            nMiuUnderrunTh = (U16)((nUnderrunTh / MIU_WORD_BYTE_SIZE) & REG_RD_UNDERRUN_TH_MSK);
            HalBachWriteReg(E_BACH_REG_BANK4, E_BACH_DMA3_CTRL_6, 0xFFFF, nMiuUnderrunTh);
            break;

        default:
            ERRMSG("Func:%s, Line:%d eAioDma %d Fail !\n", __FUNCTION__, __LINE__, eAioDma);
            goto FAIL;
            break;
    }

    return AIO_OK;

FAIL:

    return AIO_NG;
}

int HalAudDmaGetFlags(CHIP_AIO_DMA_e eAioDma, BOOL *pbDatatrigger, BOOL *pbDataboundary, BOOL *pbLocalData)
{
    int ret = AIO_OK;

    if (eAioDma < E_CHIP_AIO_DMA_AO_A)
    {
        ret |= _HalAudDmaWrGetFlags(eAioDma, pbDatatrigger, pbDataboundary, pbLocalData);
    }
    else
    {
        ret |= _HalAudDmaRdGetFlags(eAioDma, pbDatatrigger, pbDataboundary, pbLocalData);
    }

    if (ret != AIO_OK)
    {
        ERRMSG("Func:%s, Line:%d eAioDma %d Fail !\n", __FUNCTION__, __LINE__, eAioDma);
        goto FAIL;
    }

    return AIO_OK;

FAIL:

    return AIO_NG;
}

int HalAudAiSetClkRefAndMch(AI_ATTACH_t *aiAttach)
{
    int            i     = 0;
    int            ret   = AIO_OK;
    BOOL           valid = FALSE;
    AI_DMA_e       aiDma = 0;
    AI_IF_e        aiIf  = 0;
    EN_CHIP_AI_MCH eMchSet;
    U16            nChNum                              = 0;
    AudMchClkRef_e eMchClkRef                          = E_AUD_MCH_CLK_REF_NONE;
    AudMchSel_e    mchSel[E_MHAL_AI_DMA_CH_SLOT_TOTAL] = {0};

    //
    aiDma = aiAttach->enAiDma;

    //
    switch (aiDma)
    {
        case E_MHAL_AI_DMA_A:
            eMchSet = E_CHIP_AI_MCH_A;
            break;

        case E_MHAL_AI_DMA_B:
            eMchSet = E_CHIP_AI_MCH_B;
            break;

        case E_MHAL_AI_DMA_C:
            eMchSet = E_CHIP_AI_MCH_C;
            break;

        case E_MHAL_AI_DMA_D:
            eMchSet = E_CHIP_AI_MCH_D;
            break;

        case E_MHAL_AI_DMA_E:
            eMchSet = E_CHIP_AI_MCH_E;
            break;

        default:
            ERRMSG("Func:%s, Line:%d aiDma %d Fail !\n", __FUNCTION__, __LINE__, aiDma);
            goto FAIL;
            break;
    }

    //
    for (i = 0; i < E_MHAL_AI_DMA_CH_SLOT_TOTAL; i++)
    {
        aiIf      = aiAttach->eAiIf[i];
        valid     = FALSE;
        mchSel[i] = E_AUD_MCH_SEL_NULL;

        if (aiIf != E_MHAL_AI_IF_NONE)
        {
            switch (aiIf)
            {
                case E_MHAL_AI_IF_ADC_A_0_B_0:
                    mchSel[i] = E_AUD_MCH_SEL_AMIC01;
                    if (eMchClkRef
                        != E_AUD_MCH_CLK_REF_I2S_TDM_RX) // If DMA has attached I2S, just using I2S ref clock.
                    {
                        eMchClkRef = E_AUD_MCH_CLK_REF_ADC;
                    }
                    valid = TRUE;
                    break;

                case E_MHAL_AI_IF_ADC_C_0_D_0:
                    mchSel[i] = E_AUD_MCH_SEL_AMIC23;
                    if (eMchClkRef
                        != E_AUD_MCH_CLK_REF_I2S_TDM_RX) // If DMA has attached I2S, just using I2S ref clock.
                    {
                        eMchClkRef = E_AUD_MCH_CLK_REF_ADC;
                    }
                    valid = TRUE;
                    break;

                case E_MHAL_AI_IF_DMIC_A_0_1:
                    mchSel[i] = E_AUD_MCH_SEL_DMIC01;
                    if (eMchClkRef
                        != E_AUD_MCH_CLK_REF_I2S_TDM_RX) // If DMA has attached I2S, just using I2S ref clock.
                    {
                        eMchClkRef = E_AUD_MCH_CLK_REF_DMIC;
                    }
                    valid = TRUE;
                    break;

                case E_MHAL_AI_IF_DMIC_A_2_3:
                    mchSel[i] = E_AUD_MCH_SEL_DMIC23;
                    if (eMchClkRef
                        != E_AUD_MCH_CLK_REF_I2S_TDM_RX) // If DMA has attached I2S, just using I2S ref clock.
                    {
                        eMchClkRef = E_AUD_MCH_CLK_REF_DMIC;
                    }
                    valid = TRUE;
                    break;

                case E_MHAL_AI_IF_ECHO_A_0_1:
                    mchSel[i] = E_AUD_MCH_SEL_SRC;
                    if (eMchClkRef
                        != E_AUD_MCH_CLK_REF_I2S_TDM_RX) // If DMA has attached I2S, just using I2S ref clock.
                    {
                        eMchClkRef = E_AUD_MCH_CLK_REF_SRC;
                    }
                    valid = TRUE;
                    break;

                case E_MHAL_AI_IF_I2S_RX_A_0_1:
                    mchSel[i]  = E_AUD_MCH_SEL_I2S_A_RX01;
                    eMchClkRef = E_AUD_MCH_CLK_REF_I2S_TDM_RX;
                    valid      = TRUE;
                    break;

                case E_MHAL_AI_IF_I2S_RX_A_2_3:
                    mchSel[i]  = E_AUD_MCH_SEL_I2S_A_RX23;
                    eMchClkRef = E_AUD_MCH_CLK_REF_I2S_TDM_RX;
                    valid      = TRUE;
                    break;

                case E_MHAL_AI_IF_I2S_RX_A_4_5:
                    mchSel[i]  = E_AUD_MCH_SEL_I2S_A_RX45;
                    eMchClkRef = E_AUD_MCH_CLK_REF_I2S_TDM_RX;
                    valid      = TRUE;
                    break;

                case E_MHAL_AI_IF_I2S_RX_A_6_7:
                    mchSel[i]  = E_AUD_MCH_SEL_I2S_A_RX67;
                    eMchClkRef = E_AUD_MCH_CLK_REF_I2S_TDM_RX;
                    valid      = TRUE;
                    break;

                case E_MHAL_AI_IF_I2S_RX_B_0_1:
                    mchSel[i]  = E_AUD_MCH_SEL_I2S_B_RX01;
                    eMchClkRef = E_AUD_MCH_CLK_REF_I2S_TDM_RX;
                    valid      = TRUE;
                    break;

                case E_MHAL_AI_IF_I2S_RX_B_2_3:
                    mchSel[i]  = E_AUD_MCH_SEL_I2S_B_RX23;
                    eMchClkRef = E_AUD_MCH_CLK_REF_I2S_TDM_RX;
                    valid      = TRUE;
                    break;

                case E_MHAL_AI_IF_I2S_RX_B_4_5:
                    mchSel[i]  = E_AUD_MCH_SEL_I2S_B_RX45;
                    eMchClkRef = E_AUD_MCH_CLK_REF_I2S_TDM_RX;
                    valid      = TRUE;
                    break;

                case E_MHAL_AI_IF_I2S_RX_B_6_7:
                    mchSel[i]  = E_AUD_MCH_SEL_I2S_B_RX67;
                    eMchClkRef = E_AUD_MCH_CLK_REF_I2S_TDM_RX;
                    valid      = TRUE;
                    break;

                case E_MHAL_AI_IF_I2S_RX_C_0_1:
                    mchSel[i]  = E_AUD_MCH_SEL_I2S_C_RX01;
                    eMchClkRef = E_AUD_MCH_CLK_REF_I2S_TDM_RX;
                    valid      = TRUE;
                    break;

                case E_MHAL_AI_IF_I2S_RX_C_2_3:
                    mchSel[i]  = E_AUD_MCH_SEL_I2S_C_RX23;
                    eMchClkRef = E_AUD_MCH_CLK_REF_I2S_TDM_RX;
                    valid      = TRUE;
                    break;

                case E_MHAL_AI_IF_I2S_RX_C_4_5:
                    mchSel[i]  = E_AUD_MCH_SEL_I2S_C_RX45;
                    eMchClkRef = E_AUD_MCH_CLK_REF_I2S_TDM_RX;
                    valid      = TRUE;
                    break;

                case E_MHAL_AI_IF_I2S_RX_C_6_7:
                    mchSel[i]  = E_AUD_MCH_SEL_I2S_C_RX67;
                    eMchClkRef = E_AUD_MCH_CLK_REF_I2S_TDM_RX;
                    valid      = TRUE;
                    break;

                case E_MHAL_AI_IF_I2S_RX_D_0_1:
                    mchSel[i]  = E_AUD_MCH_SEL_I2S_D_RX01;
                    eMchClkRef = E_AUD_MCH_CLK_REF_I2S_TDM_RX;
                    valid      = TRUE;
                    break;

                case E_MHAL_AI_IF_I2S_RX_D_2_3:
                    mchSel[i]  = E_AUD_MCH_SEL_I2S_D_RX23;
                    eMchClkRef = E_AUD_MCH_CLK_REF_I2S_TDM_RX;
                    valid      = TRUE;
                    break;

                case E_MHAL_AI_IF_I2S_RX_D_4_5:
                    mchSel[i]  = E_AUD_MCH_SEL_I2S_D_RX45;
                    eMchClkRef = E_AUD_MCH_CLK_REF_I2S_TDM_RX;
                    valid      = TRUE;
                    break;

                case E_MHAL_AI_IF_I2S_RX_D_6_7:
                    mchSel[i]  = E_AUD_MCH_SEL_I2S_D_RX67;
                    eMchClkRef = E_AUD_MCH_CLK_REF_I2S_TDM_RX;
                    valid      = TRUE;
                    break;

                default:
                    break;
            }

            //
            if (valid == TRUE)
            {
                nChNum = (i + 1) * AI_DMA_CH_NUM_PER_SLOT;
            }
            else
            {
                ERRMSG("Func:%s, Line:%d aiIf %d Fail !\n", __FUNCTION__, __LINE__, aiIf);
                goto FAIL;
            }
        }
    }

    //
    if (nChNum <= 1)
    {
        nChNum = 1;
    }
    else if (nChNum <= 2)
    {
        nChNum = 2;
    }
    else if (nChNum <= 4)
    {
        nChNum = 4;
    }
    else // else if (nChNum <= 8)
    {
        nChNum = 8;
    }

    //
    ret |= _HalAudDmaWrConfigMchCore(eMchSet, nChNum, eMchClkRef, mchSel);
    if (ret != AIO_OK)
    {
        ERRMSG("Func:%s, Line:%d aiDma %d Fail !\n", __FUNCTION__, __LINE__, aiDma);
        goto FAIL;
    }

    return AIO_OK;

FAIL:

    return AIO_NG;
}

int HalAudAoSetClkRef(AO_ATTACH_t *aoAttach)
{
    int           i         = 0;
    AO_DMA_e      aoDma     = 0;
    AO_IF_e       aoIf      = 0;
    AudAoClkRef_e eAoClkRef = E_AUD_AO_CLK_REF_NONE;

    //
    aoDma = aoAttach->enAoDma;

    // Default
    eAoClkRef = E_AUD_AO_CLK_REF_A;

    // If exist I2S, just use I2S clk ref.
    for (i = 0; i < E_MHAL_AO_DMA_CH_SLOT_TOTAL; i++)
    {
        aoIf = aoAttach->eAoIf[i];

        if ((aoIf & E_MHAL_AO_IF_I2S_TX_A_0) || (aoIf & E_MHAL_AO_IF_I2S_TX_A_1))
        {
            eAoClkRef = E_AUD_AO_CLK_REF_I2S_TDM_TX_A;
        }

        if ((aoIf & E_MHAL_AO_IF_I2S_TX_B_0) || (aoIf & E_MHAL_AO_IF_I2S_TX_B_1))
        {
            eAoClkRef = E_AUD_AO_CLK_REF_I2S_TDM_TX_B;
        }
    }

    switch (aoDma)
    {
        case E_MHAL_AO_DMA_A:

            if (eAoClkRef == E_AUD_AO_CLK_REF_I2S_TDM_TX_A)
            {
                HalBachWriteReg(E_BACH_REG_BANK2, E_BACH_DMA_SRC_CFG02, BACH_DMA_RD1_SEL, (1 ? BACH_DMA_RD1_SEL : 0));
            }
            else if (eAoClkRef == E_AUD_AO_CLK_REF_I2S_TDM_TX_B)
            {
                HalBachWriteReg(E_BACH_REG_BANK2, E_BACH_DMA_SRC_CFG02, BACH_DMA_RD1_SEL, (1 ? BACH_DMA_RD1_SEL : 0));
            }
            else
            {
                HalBachWriteReg(E_BACH_REG_BANK2, E_BACH_DMA_SRC_CFG02, BACH_DMA_RD1_SEL, (0 ? BACH_DMA_RD1_SEL : 0));
            }
            break;

        case E_MHAL_AO_DMA_B:

            if (eAoClkRef == E_AUD_AO_CLK_REF_I2S_TDM_TX_A)
            {
                HalBachWriteReg(E_BACH_REG_BANK2, E_BACH_DMA_SRC_CFG02, BACH_DMA_RD2_SEL, (1 ? BACH_DMA_RD2_SEL : 0));
            }
            else if (eAoClkRef == E_AUD_AO_CLK_REF_I2S_TDM_TX_B)
            {
                HalBachWriteReg(E_BACH_REG_BANK2, E_BACH_DMA_SRC_CFG02, BACH_DMA_RD2_SEL, (1 ? BACH_DMA_RD2_SEL : 0));
            }
            else
            {
                HalBachWriteReg(E_BACH_REG_BANK2, E_BACH_DMA_SRC_CFG02, BACH_DMA_RD2_SEL, (0 ? BACH_DMA_RD2_SEL : 0));
            }
            break;

        case E_MHAL_AO_DMA_C:

            if (eAoClkRef == E_AUD_AO_CLK_REF_I2S_TDM_TX_A)
            {
                HalBachWriteReg(E_BACH_REG_BANK4, E_BACH_DMA3_SRC_CFG, BACH_RDMA3_VALID_SEL_MSK,
                                (1 ? 1 << BACH_RDMA3_VALID_SEL_POS : 0));
            }
            else if (eAoClkRef == E_AUD_AO_CLK_REF_I2S_TDM_TX_B)
            {
                HalBachWriteReg(E_BACH_REG_BANK4, E_BACH_DMA3_SRC_CFG, BACH_RDMA3_VALID_SEL_MSK,
                                (2 ? 2 << BACH_RDMA3_VALID_SEL_POS : 0));
            }
            else
            {
                HalBachWriteReg(E_BACH_REG_BANK4, E_BACH_DMA3_SRC_CFG, BACH_RDMA3_VALID_SEL_MSK,
                                (0 ? 0 << BACH_RDMA3_VALID_SEL_POS : 0));
            }
            break;

        default:
            ERRMSG("Func:%s, Line:%d aoDma %d Fail !\n", __FUNCTION__, __LINE__, aoDma);
            goto FAIL;
            break;
    }

    return AIO_OK;

FAIL:

    return AIO_NG;
}

int HalAudAoSetClkUnRef(AO_ATTACH_t *aoAttach)
{
    int           i           = 0;
    AO_DMA_e      aoDma       = 0;
    AO_IF_e       aoIf        = 0;
    AudAoClkRef_e eAoClkUnRef = E_AUD_AO_CLK_REF_NONE;

    //
    aoDma = aoAttach->enAoDma;

    // Default
    eAoClkUnRef = E_AUD_AO_CLK_REF_A;

    for (i = 0; i < E_MHAL_AO_DMA_CH_SLOT_TOTAL; i++)
    {
        aoIf = aoAttach->eAoIf[i];
        if ((aoIf & E_MHAL_AO_IF_I2S_TX_A_0) || (aoIf & E_MHAL_AO_IF_I2S_TX_A_1))
        {
            eAoClkUnRef = E_AUD_AO_CLK_REF_I2S_TDM_TX_A;
        }

        if ((aoIf & E_MHAL_AO_IF_I2S_TX_B_0) || (aoIf & E_MHAL_AO_IF_I2S_TX_B_1))
        {
            eAoClkUnRef = E_AUD_AO_CLK_REF_I2S_TDM_TX_B;
        }
    }
    // Just I2S Clk ref need to un ref clk
    if (eAoClkUnRef == E_AUD_AO_CLK_REF_I2S_TDM_TX_A || eAoClkUnRef == E_AUD_AO_CLK_REF_I2S_TDM_TX_B)
    {
        switch (aoDma)
        {
            case E_MHAL_AO_DMA_A:
                HalBachWriteReg(E_BACH_REG_BANK2, E_BACH_DMA_SRC_CFG02, BACH_DMA_RD1_SEL, (0 ? BACH_DMA_RD1_SEL : 0));
                break;

            case E_MHAL_AO_DMA_B:
                HalBachWriteReg(E_BACH_REG_BANK2, E_BACH_DMA_SRC_CFG02, BACH_DMA_RD2_SEL, (0 ? BACH_DMA_RD2_SEL : 0));
                break;

            case E_MHAL_AO_DMA_C:
                HalBachWriteReg(E_BACH_REG_BANK4, E_BACH_DMA3_SRC_CFG, BACH_RDMA3_VALID_SEL_MSK,
                                (0 ? 0 << BACH_RDMA3_VALID_SEL_POS : 0));
                break;

            default:
                ERRMSG("Func:%s, Line:%d aoDma %d Fail !\n", __FUNCTION__, __LINE__, aoDma);
                goto FAIL;
                break;
        }
    }

    return AIO_OK;

FAIL:

    return AIO_NG;
}

int HalAudDmaMonoEnable(CHIP_AIO_DMA_e eAioDma, BOOL bEn, BOOL bMonoCopyEn)
{
    U16 nValue = 0;

    switch (eAioDma)
    {
        case E_CHIP_AIO_DMA_AI_A:
            HalBachWriteReg(E_BACH_REG_BANK1, E_BACH_DMA_TEST_CTRL7, REG_DMA1_WR_MONO, (bEn ? REG_DMA1_WR_MONO : 0));
            break;

        case E_CHIP_AIO_DMA_AO_A:
            if (bEn)
            {
                nValue = nValue | REG_DMA1_RD_MONO;
            }

            if (bMonoCopyEn)
            {
                nValue = nValue | REG_DMA1_RD_MONO_COPY;
            }

            HalBachWriteReg(E_BACH_REG_BANK1, E_BACH_DMA_TEST_CTRL7, REG_DMA1_RD_MONO | REG_DMA1_RD_MONO_COPY, nValue);
            break;

        case E_CHIP_AIO_DMA_AI_B:
            HalBachWriteReg(E_BACH_REG_BANK1, E_BACH_DMA_TEST_CTRL7, REG_DMA2_WR_MONO, (bEn ? REG_DMA2_WR_MONO : 0));
            break;

        case E_CHIP_AIO_DMA_AO_B:
            if (bEn)
            {
                nValue = nValue | REG_DMA2_RD_MONO;
            }

            if (bMonoCopyEn)
            {
                nValue = nValue | REG_DMA2_RD_MONO_COPY;
            }

            HalBachWriteReg(E_BACH_REG_BANK1, E_BACH_DMA_TEST_CTRL7, REG_DMA2_RD_MONO | REG_DMA2_RD_MONO_COPY, nValue);
            break;

        case E_CHIP_AIO_DMA_AI_C:
            HalBachWriteReg(E_BACH_REG_BANK4, E_BACH_DMA3_CTRL_17, REG_DMA3_WR_MONO, (bEn ? REG_DMA3_WR_MONO : 0));
            break;

        case E_CHIP_AIO_DMA_AO_C:
            if (bEn)
            {
                nValue = nValue | REG_DMA3_RD_MONO;
            }

            if (bMonoCopyEn)
            {
                nValue = nValue | REG_DMA3_RD_MONO_COPY;
            }

            HalBachWriteReg(E_BACH_REG_BANK4, E_BACH_DMA3_CTRL_17, REG_DMA3_RD_MONO | REG_DMA3_RD_MONO_COPY, nValue);
            break;

        case E_CHIP_AIO_DMA_AI_D:
            HalBachWriteReg(E_BACH_REG_BANK4, E_BACH_DMA4_CTRL_07, REG_DMA3_WR_MONO, (bEn ? REG_DMA3_WR_MONO : 0));
            break;

        case E_CHIP_AIO_DMA_AI_E:
            HalBachWriteReg(E_BACH_REG_BANK4, E_BACH_DMA4_CTRL_07, REG_DMA3_WR_MONO, (bEn ? REG_DMA3_WR_MONO : 0));
            break;

        default:
            ERRMSG("Func:%s, Line:%d eAioDma %d Fail !\n", __FUNCTION__, __LINE__, eAioDma);
            goto FAIL;
            break;
    }

    return AIO_OK;

FAIL:

    return AIO_NG;
}

int HalAudDpgaCtrl(EN_CHIP_DPGA eDpga, BOOL bEnable, BOOL bMute, BOOL bFade)
{
    U8            nAddr = 0;
    U16           nConfigValue;
    BachRegBank_e nBank;

    switch (eDpga)
    {
        case E_CHIP_DPGA_A:
            nBank = E_BACH_REG_BANK1;
            nAddr = E_BACH_MMC1_DPGA_CFG1;
            break;

        case E_CHIP_DPGA_C:
            nBank = E_BACH_REG_BANK1;
            nAddr = E_BACH_ADC_DPGA_CFG1;
            break;

        case E_CHIP_DPGA_D:
            nBank = E_BACH_REG_BANK1;
            nAddr = E_BACH_AEC1_DPGA_CFG1;
            break;

        case E_CHIP_DPGA_E:
            nBank = E_BACH_REG_BANK1;
            nAddr = E_BACH_MMCDEC1_DPGA_CFG1;
            break;

        case E_CHIP_DPGA_B:
            nBank = E_BACH_REG_BANK1;
            nAddr = E_BACH_MMC2_DPGA_CFG1;
            break;

        case E_CHIP_DPGA_F:
            nBank = E_BACH_REG_BANK1;
            nAddr = E_BACH_MMCDEC2_DPGA_CFG1;
            break;

        case E_CHIP_DPGA_G:
            nBank = E_BACH_REG_BANK2;
            nAddr = E_BACH_I2S_TDM_DPGA_CFG1;
            break;

        case E_CHIP_DPGA_H:
            nBank = E_BACH_REG_BANK4;
            nAddr = E_BACH_I2S_TDM1_DPGA_CFG1;
            break;

        case E_CHIP_DPGA_I:
            nBank = E_BACH_REG_BANK1;
            nAddr = E_BACH_AU2HDMI_DPGA_CFG1;
            break;

        default:
            ERRMSG("Func:%s, Line:%d eDpga %d Fail !\n", __FUNCTION__, __LINE__, eDpga);
            goto FAIL;
            break;
    }

    nConfigValue = 0;
    if (bEnable)
    {
        nConfigValue |= DPGA_EN;
    }
    if (bMute)
    {
        nConfigValue |= MUTE_2_ZERO;
    }
    if (bFade)
    {
        nConfigValue |= FADING_EN;
    }

    HalBachWriteReg(nBank, nAddr, (DPGA_EN | MUTE_2_ZERO | FADING_EN), nConfigValue);
    // Step Size for fading function, 4: 16 sample, 5: 32 sample, 6: 64 sample
    HalBachWriteReg(nBank, nAddr, STEP_MSK, 6 << STEP_POS);

    return AIO_OK;

FAIL:

    return AIO_NG;
}

int HalAudDpgaSetGain(EN_CHIP_DPGA eDpga, S8 gain, S8 ch)
{
    int           ret = AIO_OK;
    U8            nAddr;
    U8            nGainIdx;
    U16           nRegMsk, nValue;
    BachRegBank_e nBank;

    ret |= _HalAudDpgaCalGain(gain, &nGainIdx);
    if (ret != AIO_OK)
    {
        ERRMSG("Func:%s, Line:%d eDpga %d Fail !\n", __FUNCTION__, __LINE__, eDpga);
        goto FAIL;
    }

    switch (eDpga)
    {
        case E_CHIP_DPGA_A:
            nBank   = E_BACH_REG_BANK1;
            nAddr   = E_BACH_MMC1_DPGA_CFG2;
            nRegMsk = (REG_GAIN_R_MSK | REG_GAIN_L_MSK);
            nValue  = HalBachReadReg(nBank, nAddr);

            if (ch == 0)
            {
                nValue = nValue & (0xFF << REG_GAIN_R_POS);
                nValue = nValue | (nGainIdx << REG_GAIN_L_POS);
            }
            else if (ch == 1)
            {
                nValue = nValue & (0xFF << REG_GAIN_L_POS);
                nValue = nValue | (nGainIdx << REG_GAIN_R_POS);
            }
            else
            {
                ERRMSG("Func:%s, Line:%d eDpga %d ch = %d Fail !\n", __FUNCTION__, __LINE__, eDpga, ch);
                goto FAIL;
            }

            break;

        case E_CHIP_DPGA_C:
            nBank   = E_BACH_REG_BANK1;
            nAddr   = E_BACH_ADC_DPGA_CFG2;
            nRegMsk = (REG_GAIN_L_MSK);
            nValue  = HalBachReadReg(nBank, nAddr);

            if (ch == 0)
            {
                nValue = nValue & (0xFF << REG_GAIN_R_POS);
                nValue = nValue | (nGainIdx << REG_GAIN_L_POS);
            }
            else
            {
                ERRMSG("Func:%s, Line:%d eDpga %d ch = %d Fail !\n", __FUNCTION__, __LINE__, eDpga, ch);
                goto FAIL;
            }

            break;

        case E_CHIP_DPGA_D:
            nBank   = E_BACH_REG_BANK1;
            nAddr   = E_BACH_AEC1_DPGA_CFG2;
            nRegMsk = (REG_GAIN_L_MSK);
            nValue  = HalBachReadReg(nBank, nAddr);

            if (ch == 0)
            {
                nValue = nValue & (0xFF << REG_GAIN_R_POS);
                nValue = nValue | (nGainIdx << REG_GAIN_L_POS);
            }
            else
            {
                ERRMSG("Func:%s, Line:%d eDpga %d ch = %d Fail !\n", __FUNCTION__, __LINE__, eDpga, ch);
                goto FAIL;
            }

            break;

        case E_CHIP_DPGA_E:
            nBank   = E_BACH_REG_BANK1;
            nAddr   = E_BACH_MMCDEC1_DPGA_CFG2;
            nRegMsk = (REG_GAIN_R_MSK | REG_GAIN_L_MSK);
            nValue  = HalBachReadReg(nBank, nAddr);

            if (ch == 0)
            {
                nValue = nValue & (0xFF << REG_GAIN_R_POS);
                nValue = nValue | (nGainIdx << REG_GAIN_L_POS);
            }
            else if (ch == 1)
            {
                nValue = nValue & (0xFF << REG_GAIN_L_POS);
                nValue = nValue | (nGainIdx << REG_GAIN_R_POS);
            }
            else
            {
                ERRMSG("Func:%s, Line:%d eDpga %d ch = %d Fail !\n", __FUNCTION__, __LINE__, eDpga, ch);
                goto FAIL;
            }

            break;

        case E_CHIP_DPGA_B:
            nBank   = E_BACH_REG_BANK1;
            nAddr   = E_BACH_MMC2_DPGA_CFG2;
            nRegMsk = (REG_GAIN_R_MSK | REG_GAIN_L_MSK);
            nValue  = HalBachReadReg(nBank, nAddr);

            if (ch == 0)
            {
                nValue = nValue & (0xFF << REG_GAIN_R_POS);
                nValue = nValue | (nGainIdx << REG_GAIN_L_POS);
            }
            else if (ch == 1)
            {
                nValue = nValue & (0xFF << REG_GAIN_L_POS);
                nValue = nValue | (nGainIdx << REG_GAIN_R_POS);
            }
            else
            {
                ERRMSG("Func:%s, Line:%d eDpga %d ch = %d Fail !\n", __FUNCTION__, __LINE__, eDpga, ch);
                goto FAIL;
            }

            break;

        case E_CHIP_DPGA_F:
            nBank   = E_BACH_REG_BANK1;
            nAddr   = E_BACH_MMCDEC2_DPGA_CFG2;
            nRegMsk = (REG_GAIN_R_MSK | REG_GAIN_L_MSK);
            nValue  = HalBachReadReg(nBank, nAddr);

            if (ch == 0)
            {
                nValue = nValue & (0xFF << REG_GAIN_R_POS);
                nValue = nValue | (nGainIdx << REG_GAIN_L_POS);
            }
            else if (ch == 1)
            {
                nValue = nValue & (0xFF << REG_GAIN_L_POS);
                nValue = nValue | (nGainIdx << REG_GAIN_R_POS);
            }
            else
            {
                ERRMSG("Func:%s, Line:%d eDpga %d ch = %d Fail !\n", __FUNCTION__, __LINE__, eDpga, ch);
                goto FAIL;
            }

            break;

        case E_CHIP_DPGA_G:
            nBank   = E_BACH_REG_BANK2;
            nAddr   = E_BACH_I2S_TDM_DPGA_CFG2;
            nRegMsk = (REG_GAIN_R_MSK | REG_GAIN_L_MSK);
            nValue  = HalBachReadReg(nBank, nAddr);

            if (ch == 0)
            {
                nValue = nValue & (0xFF << REG_GAIN_R_POS);
                nValue = nValue | (nGainIdx << REG_GAIN_L_POS);
            }
            else if (ch == 1)
            {
                nValue = nValue & (0xFF << REG_GAIN_L_POS);
                nValue = nValue | (nGainIdx << REG_GAIN_R_POS);
            }
            else
            {
                ERRMSG("Func:%s, Line:%d eDpga %d ch = %d Fail !\n", __FUNCTION__, __LINE__, eDpga, ch);
                goto FAIL;
            }

            break;

        case E_CHIP_DPGA_H:
            nBank   = E_BACH_REG_BANK4;
            nAddr   = E_BACH_I2S_TDM1_DPGA_CFG2;
            nRegMsk = (REG_GAIN_R_MSK | REG_GAIN_L_MSK);
            nValue  = HalBachReadReg(nBank, nAddr);

            if (ch == 0)
            {
                nValue = nValue & (0xFF << REG_GAIN_R_POS);
                nValue = nValue | (nGainIdx << REG_GAIN_L_POS);
            }
            else if (ch == 1)
            {
                nValue = nValue & (0xFF << REG_GAIN_L_POS);
                nValue = nValue | (nGainIdx << REG_GAIN_R_POS);
            }
            else
            {
                ERRMSG("Func:%s, Line:%d eDpga %d ch = %d Fail !\n", __FUNCTION__, __LINE__, eDpga, ch);
                goto FAIL;
            }

            break;

        case E_CHIP_DPGA_I:
            nBank   = E_BACH_REG_BANK1;
            nAddr   = E_BACH_AU2HDMI_DPGA_CFG2;
            nRegMsk = (REG_GAIN_R_MSK | REG_GAIN_L_MSK);
            nValue  = HalBachReadReg(nBank, nAddr);

            if (ch == 0)
            {
                nValue = nValue & (0xFF << REG_GAIN_R_POS);
                nValue = nValue | (nGainIdx << REG_GAIN_L_POS);
            }
            else if (ch == 1)
            {
                nValue = nValue & (0xFF << REG_GAIN_L_POS);
                nValue = nValue | (nGainIdx << REG_GAIN_R_POS);
            }
            else
            {
                ERRMSG("Func:%s, Line:%d eDpga %d ch = %d Fail !\n", __FUNCTION__, __LINE__, eDpga, ch);
                goto FAIL;
            }

            break;

        default:
            ERRMSG("Func:%s, Line:%d eDpga %d Fail !\n", __FUNCTION__, __LINE__, eDpga);
            goto FAIL;
            break;
    }

    HalBachWriteReg(nBank, nAddr, nRegMsk, nValue);

    return AIO_OK;

FAIL:

    return AIO_NG;
}

int HalAudDpgaSetGainOffset(EN_CHIP_DPGA eDpga, S8 gainRegValue, S8 ch)
{
    U8            nAddr;
    U8            nGainIdx;
    U16           nRegMsk, nValue;
    S8            nGain;
    BachRegBank_e nBank;

    //
    // 5bits: 15 ~ -16 (0xF ~ 0x10)
    // -7.5dB ~ 8dB

    nGain = gainRegValue;
    if (nGain > 15)
    {
        nGain = 15;
    }
    else if (nGain < -16)
    {
        nGain = -16;
    }

    nGainIdx = (U8)nGain;

    switch (eDpga)
    {
        case E_CHIP_DPGA_A:
            nBank   = E_BACH_REG_BANK1;
            nAddr   = E_BACH_MMC1_DPGA_CFG1;
            nRegMsk = (REG_OFFSET_R_MSK | REG_OFFSET_L_MSK);
            nValue  = HalBachReadReg(nBank, nAddr);

            if (ch == 0)
            {
                nValue = nValue & (0x1F << REG_OFFSET_R_POS);
                nValue = nValue | (nGainIdx << REG_OFFSET_L_POS);
            }
            else if (ch == 1)
            {
                nValue = nValue & (0x1F << REG_OFFSET_L_POS);
                nValue = nValue | (nGainIdx << REG_OFFSET_R_POS);
            }
            else
            {
                ERRMSG("Func:%s, Line:%d eDpga %d ch = %d Fail !\n", __FUNCTION__, __LINE__, eDpga, ch);
                goto FAIL;
            }

            break;

        case E_CHIP_DPGA_C:
            nBank   = E_BACH_REG_BANK1;
            nAddr   = E_BACH_ADC_DPGA_CFG1;
            nRegMsk = (REG_OFFSET_L_MSK);
            nValue  = HalBachReadReg(nBank, nAddr);

            if (ch == 0)
            {
                nValue = nValue & (0x1F << REG_OFFSET_R_POS);
                nValue = nValue | (nGainIdx << REG_OFFSET_L_POS);
            }
            else
            {
                ERRMSG("Func:%s, Line:%d eDpga %d ch = %d Fail !\n", __FUNCTION__, __LINE__, eDpga, ch);
                goto FAIL;
            }

            break;

        case E_CHIP_DPGA_D:
            nBank   = E_BACH_REG_BANK1;
            nAddr   = E_BACH_AEC1_DPGA_CFG1;
            nRegMsk = (REG_OFFSET_L_MSK);
            nValue  = HalBachReadReg(nBank, nAddr);

            if (ch == 0)
            {
                nValue = nValue & (0x1F << REG_OFFSET_R_POS);
                nValue = nValue | (nGainIdx << REG_OFFSET_L_POS);
            }
            else
            {
                ERRMSG("Func:%s, Line:%d eDpga %d ch = %d Fail !\n", __FUNCTION__, __LINE__, eDpga, ch);
                goto FAIL;
            }

            break;

        case E_CHIP_DPGA_E:
            nBank   = E_BACH_REG_BANK1;
            nAddr   = E_BACH_MMCDEC1_DPGA_CFG1;
            nRegMsk = (REG_OFFSET_R_MSK | REG_OFFSET_L_MSK);
            nValue  = HalBachReadReg(nBank, nAddr);

            if (ch == 0)
            {
                nValue = nValue & (0x1F << REG_OFFSET_R_POS);
                nValue = nValue | (nGainIdx << REG_OFFSET_L_POS);
            }
            else if (ch == 1)
            {
                nValue = nValue & (0x1F << REG_OFFSET_L_POS);
                nValue = nValue | (nGainIdx << REG_OFFSET_R_POS);
            }
            else
            {
                ERRMSG("Func:%s, Line:%d eDpga %d ch = %d Fail !\n", __FUNCTION__, __LINE__, eDpga, ch);
                goto FAIL;
            }

            break;

        case E_CHIP_DPGA_B:
            nBank   = E_BACH_REG_BANK1;
            nAddr   = E_BACH_MMC2_DPGA_CFG1;
            nRegMsk = (REG_OFFSET_R_MSK | REG_OFFSET_L_MSK);
            nValue  = HalBachReadReg(nBank, nAddr);

            if (ch == 0)
            {
                nValue = nValue & (0x1F << REG_OFFSET_R_POS);
                nValue = nValue | (nGainIdx << REG_OFFSET_L_POS);
            }
            else if (ch == 1)
            {
                nValue = nValue & (0x1F << REG_OFFSET_L_POS);
                nValue = nValue | (nGainIdx << REG_OFFSET_R_POS);
            }
            else
            {
                ERRMSG("Func:%s, Line:%d eDpga %d ch = %d Fail !\n", __FUNCTION__, __LINE__, eDpga, ch);
                goto FAIL;
            }

            break;

        case E_CHIP_DPGA_F:
            nBank   = E_BACH_REG_BANK1;
            nAddr   = E_BACH_MMCDEC2_DPGA_CFG1;
            nRegMsk = (REG_OFFSET_R_MSK | REG_OFFSET_L_MSK);
            nValue  = HalBachReadReg(nBank, nAddr);

            if (ch == 0)
            {
                nValue = nValue & (0x1F << REG_OFFSET_R_POS);
                nValue = nValue | (nGainIdx << REG_OFFSET_L_POS);
            }
            else if (ch == 1)
            {
                nValue = nValue & (0x1F << REG_OFFSET_L_POS);
                nValue = nValue | (nGainIdx << REG_OFFSET_R_POS);
            }
            else
            {
                ERRMSG("Func:%s, Line:%d eDpga %d ch = %d Fail !\n", __FUNCTION__, __LINE__, eDpga, ch);
                goto FAIL;
            }

            break;

        case E_CHIP_DPGA_G:
            nBank   = E_BACH_REG_BANK2;
            nAddr   = E_BACH_I2S_TDM_DPGA_CFG1;
            nRegMsk = (REG_OFFSET_R_MSK | REG_OFFSET_L_MSK);
            nValue  = HalBachReadReg(nBank, nAddr);

            if (ch == 0)
            {
                nValue = nValue & (0x1F << REG_OFFSET_R_POS);
                nValue = nValue | (nGainIdx << REG_OFFSET_L_POS);
            }
            else if (ch == 1)
            {
                nValue = nValue & (0x1F << REG_OFFSET_L_POS);
                nValue = nValue | (nGainIdx << REG_OFFSET_R_POS);
            }
            else
            {
                ERRMSG("Func:%s, Line:%d eDpga %d ch = %d Fail !\n", __FUNCTION__, __LINE__, eDpga, ch);
                goto FAIL;
            }

            break;

        case E_CHIP_DPGA_H:
            nBank   = E_BACH_REG_BANK4;
            nAddr   = E_BACH_I2S_TDM1_DPGA_CFG1;
            nRegMsk = (REG_OFFSET_R_MSK | REG_OFFSET_L_MSK);
            nValue  = HalBachReadReg(nBank, nAddr);

            if (ch == 0)
            {
                nValue = nValue & (0x1F << REG_OFFSET_R_POS);
                nValue = nValue | (nGainIdx << REG_OFFSET_L_POS);
            }
            else if (ch == 1)
            {
                nValue = nValue & (0x1F << REG_OFFSET_L_POS);
                nValue = nValue | (nGainIdx << REG_OFFSET_R_POS);
            }
            else
            {
                ERRMSG("Func:%s, Line:%d eDpga %d ch = %d Fail !\n", __FUNCTION__, __LINE__, eDpga, ch);
                goto FAIL;
            }

            break;

        case E_CHIP_DPGA_I:
            nBank   = E_BACH_REG_BANK1;
            nAddr   = E_BACH_AU2HDMI_DPGA_CFG1;
            nRegMsk = (REG_OFFSET_R_MSK | REG_OFFSET_L_MSK);
            nValue  = HalBachReadReg(nBank, nAddr);

            if (ch == 0)
            {
                nValue = nValue & (0x1F << REG_OFFSET_R_POS);
                nValue = nValue | (nGainIdx << REG_OFFSET_L_POS);
            }
            else if (ch == 1)
            {
                nValue = nValue & (0x1F << REG_OFFSET_L_POS);
                nValue = nValue | (nGainIdx << REG_OFFSET_R_POS);
            }
            else
            {
                ERRMSG("Func:%s, Line:%d eDpga %d ch = %d Fail !\n", __FUNCTION__, __LINE__, eDpga, ch);
                goto FAIL;
            }

            break;

        default:
            ERRMSG("Func:%s, Line:%d eDpga %d Fail !\n", __FUNCTION__, __LINE__, eDpga);
            goto FAIL;
            break;
    }

    HalBachWriteReg(nBank, nAddr, nRegMsk, nValue);

    return AIO_OK;

FAIL:

    return AIO_NG;
}

int HalAudDpgaSetGainFading(EN_CHIP_DPGA eDpga, U8 nFading, S8 ch)
{
    U8            nAddr;
    U16           nRegMsk, nValue;
    U8            nFadingEn = 1, nFadingValue = 0;
    BachRegBank_e nBank;

    // 0:OFF    1~7:reg value 0~6
    if (nFading > 7)
    {
        ERRMSG("Func:%s, Line:%d eDpga %d nFading %d ch = %d Fail !\n", __FUNCTION__, __LINE__, eDpga, nFading, ch);
        goto FAIL;
    }

    if (nFading == 0) // OFF
    {
        nFadingEn    = 0;
        nFadingValue = 0;
    }
    else
    {
        nFadingEn    = 1;
        nFadingValue = nFading - 1;
    }

    switch (eDpga)
    {
        case E_CHIP_DPGA_A:
            nBank   = E_BACH_REG_BANK1;
            nAddr   = E_BACH_MMC1_DPGA_CFG1;
            nRegMsk = (STEP_MSK | FADING_EN);

            break;

        case E_CHIP_DPGA_B:
            nBank   = E_BACH_REG_BANK1;
            nAddr   = E_BACH_MMC2_DPGA_CFG1;
            nRegMsk = (STEP_MSK | FADING_EN);

            break;

        case E_CHIP_DPGA_G:
            nBank   = E_BACH_REG_BANK2;
            nAddr   = E_BACH_I2S_TDM_DPGA_CFG1;
            nRegMsk = (STEP_MSK | FADING_EN);

            break;

        case E_CHIP_DPGA_H:
            nBank   = E_BACH_REG_BANK4;
            nAddr   = E_BACH_I2S_TDM1_DPGA_CFG1;
            nRegMsk = (STEP_MSK | FADING_EN);

            break;

        case E_CHIP_DPGA_I:
            nBank   = E_BACH_REG_BANK1;
            nAddr   = E_BACH_AU2HDMI_DPGA_CFG1;
            nRegMsk = (STEP_MSK | FADING_EN);

            break;

        case E_CHIP_DPGA_D:
        case E_CHIP_DPGA_E:
        case E_CHIP_DPGA_F:
            ERRMSG("Func:%s, Line:%d eDpga %d nFading %d ch = %d don't support this DPGA!\n", __FUNCTION__, __LINE__,
                   eDpga, nFading, ch);
            goto FAIL;
            break;

        default:
            ERRMSG("Func:%s, Line:%d eDpga %d unable to find this DPGA !\n", __FUNCTION__, __LINE__, eDpga);
            goto FAIL;
            break;
    }

    nValue = HalBachReadReg(nBank, nAddr);

    if ((ch == 0) || (ch == 1))
    {
        nValue = nValue & (~nRegMsk);
        nValue = nValue | (nFadingValue << STEP_POS) | (nFadingEn << 1);
    }
    else
    {
        ERRMSG("Func:%s, Line:%d eDpga %d nFading %d ch = %d Fail !\n", __FUNCTION__, __LINE__, eDpga, nFading, ch);
        goto FAIL;
    }

    HalBachWriteReg(nBank, nAddr, nRegMsk, nValue);

    return AIO_OK;

FAIL:

    return AIO_NG;
}

int HalAudDigMicEnable(CHIP_AI_DMIC_e eDmic, BOOL bEn)
{
    switch (eDmic)
    {
        case E_CHIP_AI_DMIC_A:
            if (bEn)
            {
                HalBachWriteReg(E_BACH_REG_BANK3, E_BACH_VREC_CTRL02, REG_SEL_RECORD_SRC, 0);
            }
            else
            {
                // todo later
            }
            break;

        default:
            ERRMSG("Func:%s, Line:%d eDmic %d Fail !\n", __FUNCTION__, __LINE__, eDmic);
            goto FAIL;
            break;
    }

    return AIO_OK;

FAIL:

    return AIO_NG;
}

int HalAudDigMicSetChannel(CHIP_AI_DMIC_e eDmic, U16 nCh)
{
    U16 nConfigValue;

    if (nCh == 1)
    {
        nConfigValue = 0;
    }
    else if (nCh == 2)
    {
        nConfigValue = 1;
    }
    else if (nCh == 4)
    {
        nConfigValue = 2;
    }
    else if (nCh == 8)
    {
        nConfigValue = 3;
    }
    else
    {
        ERRMSG("Func:%s, Line:%d eDmic %d nCh %d Fail !\n", __FUNCTION__, __LINE__, eDmic, nCh);
        goto FAIL;
    }
    switch (eDmic)
    {
        case E_CHIP_AI_DMIC_A:
            // DMIC channel
            HalBachWriteReg(E_BACH_REG_BANK3, E_BACH_VREC_CTRL01, REG_CHN_MODE_DMIC_MSK,
                            (nConfigValue << REG_CHN_MODE_DMIC_POS)); // 1504 02 [5:4]
            break;

        default:
            ERRMSG("Func:%s, Line:%d eDmic %d nCh %d Fail !\n", __FUNCTION__, __LINE__, eDmic, nCh);
            goto FAIL;
            break;
    }

    return AIO_OK;

FAIL:

    return AIO_NG;
}

int HalAudDigMicSetRate(CHIP_AI_DMIC_e eDmic, AudRate_e eRate)
{
    int i = 0;

    switch (eDmic)
    {
        case E_CHIP_AI_DMIC_A:
            g_eDmicSmpRat[E_CHIP_AI_DMIC_A] = eRate;

            //
            if ((g_nDmicBckMode8K < 1) || (g_nDmicBckMode8K > 7))
            {
                ERRMSG("Function - %s - Bck mode 8K = %d, Out of range 1~7 !] \n", __func__, (int)g_nDmicBckMode8K);
                return FALSE;
            }
            if ((g_nDmicBckMode16K < 8) || (g_nDmicBckMode16K > 14))
            {
                ERRMSG("Function - %s - Bck mode 16K = %d, Out of range 8~14 !] \n", __func__, (int)g_nDmicBckMode16K);
                return FALSE;
            }
            if ((g_nDmicBckMode32K < 15) || (g_nDmicBckMode32K > 15))
            {
                ERRMSG("Function - %s - Bck mode 32K = %d, Out of range 15~15 !] \n", __func__, (int)g_nDmicBckMode32K);
                return FALSE;
            }
            if ((g_nDmicBckMode48K < 16) || (g_nDmicBckMode48K > 16))
            {
                ERRMSG("Function - %s - Bck mode 48K = %d, Out of range 16~16 !] \n", __func__, (int)g_nDmicBckMode48K);
                return FALSE;
            }

            // for gain compensation
            for (i = 0; i < 4; i++)
            {
                HalAudDigMicSetGain(eDmic, g_nDmicGain[E_CHIP_AI_DMIC_A][i], i);
            }

            switch (eRate)
            {
                case E_AUD_RATE_8K:
                    HalBachWriteReg(E_BACH_REG_BANK3, E_BACH_VREC_CTRL04, REG_FS_SEL_MSK,
                                    g_nDmicBckMode8K << REG_FS_SEL_POS);
                    HalBachWriteReg(E_BACH_REG_BANK3, E_BACH_CIC_CTRL01, REG_DEC_FILTER,
                                    REG_DEC_FILTER); // 1: 3stage, 0: 5stage
                    HalBachWriteReg(E_BACH_REG_BANK3, E_BACH_CIC_CTRL02, REG_DEC_FILTER, REG_DEC_FILTER);
                    break;

                case E_AUD_RATE_16K:
                    HalBachWriteReg(E_BACH_REG_BANK3, E_BACH_VREC_CTRL04, REG_FS_SEL_MSK,
                                    g_nDmicBckMode16K << REG_FS_SEL_POS);
                    HalBachWriteReg(E_BACH_REG_BANK3, E_BACH_CIC_CTRL01, REG_DEC_FILTER, REG_DEC_FILTER);
                    HalBachWriteReg(E_BACH_REG_BANK3, E_BACH_CIC_CTRL02, REG_DEC_FILTER, REG_DEC_FILTER);
                    break;

                case E_AUD_RATE_32K:
                    HalBachWriteReg(E_BACH_REG_BANK3, E_BACH_VREC_CTRL04, REG_FS_SEL_MSK,
                                    g_nDmicBckMode32K << REG_FS_SEL_POS);
                    HalBachWriteReg(E_BACH_REG_BANK3, E_BACH_CIC_CTRL01, REG_DEC_FILTER, 0);
                    HalBachWriteReg(E_BACH_REG_BANK3, E_BACH_CIC_CTRL02, REG_DEC_FILTER, 0);
                    break;

                case E_AUD_RATE_48K:
                    HalBachWriteReg(E_BACH_REG_BANK3, E_BACH_VREC_CTRL04, REG_FS_SEL_MSK,
                                    g_nDmicBckMode48K << REG_FS_SEL_POS);
                    HalBachWriteReg(E_BACH_REG_BANK3, E_BACH_CIC_CTRL01, REG_DEC_FILTER, 0);
                    HalBachWriteReg(E_BACH_REG_BANK3, E_BACH_CIC_CTRL02, REG_DEC_FILTER, 0);
                    break;

                default:
                    ERRMSG("Func:%s, Line:%d eDmic %d eRate %d Fail !\n", __FUNCTION__, __LINE__, eDmic, eRate);
                    goto FAIL;
                    break;
            }
            break;

        default:
            ERRMSG("Func:%s, Line:%d eDmic %d eRate %d Fail !\n", __FUNCTION__, __LINE__, eDmic, eRate);
            goto FAIL;
            break;
    }

    return AIO_OK;

FAIL:

    return AIO_NG;
}

int HalAudDigMicSetGain(CHIP_AI_DMIC_e eDmic, S8 s8Gain, S8 ch)
{
    U16 nGain, nAddr, nRegMsk, nPos;

    switch (eDmic)
    {
        case E_CHIP_AI_DMIC_A:

            if (s8Gain > 6)
            {
                ERRMSG("Func:%s, Line:%d eDmic %d s8Gain %d ch %d Fail !\n", __FUNCTION__, __LINE__, eDmic, s8Gain, ch);
                goto FAIL;
            }

            break;

        default:
            ERRMSG("Func:%s, Line:%d eDmic %d s8Gain %d ch %d Fail !\n", __FUNCTION__, __LINE__, eDmic, s8Gain, ch);
            goto FAIL;
            break;
    }

    g_nDmicGain[eDmic][ch] = s8Gain;

    if (g_eDmicSmpRat[eDmic] == E_AUD_RATE_48K || g_eDmicSmpRat[eDmic] == E_AUD_RATE_32K)
    {
        nGain = (U16)s8Gain + 1;
    }
    else
    {
        nGain = (U16)s8Gain;
    }

    switch (eDmic)
    {
        case E_CHIP_AI_DMIC_A:

            if (ch == 0)
            {
                nAddr   = E_BACH_CIC_CTRL01;
                nRegMsk = REG_CIC_GAIN_L_MSK;
                nPos    = REG_CIC_GAIN_L_POS;
            }
            else if (ch == 1)
            {
                nAddr   = E_BACH_CIC_CTRL01;
                nRegMsk = REG_CIC_GAIN_R_MSK;
                nPos    = REG_CIC_GAIN_R_POS;
            }
            else if (ch == 2)
            {
                nAddr   = E_BACH_CIC_CTRL02;
                nRegMsk = REG_CIC_GAIN_L_MSK;
                nPos    = REG_CIC_GAIN_L_POS;
            }
            else if (ch == 3)
            {
                nAddr   = E_BACH_CIC_CTRL02;
                nRegMsk = REG_CIC_GAIN_R_MSK;
                nPos    = REG_CIC_GAIN_R_POS;
            }
            else
            {
                ERRMSG("Func:%s, Line:%d eDmic %d s8Gain %d ch %d Fail !\n", __FUNCTION__, __LINE__, eDmic, s8Gain, ch);
                goto FAIL;
            }

            HalBachWriteReg(E_BACH_REG_BANK3, nAddr, nRegMsk, nGain << nPos);

            break;

        default:
            ERRMSG("Func:%s, Line:%d eDmic %d s8Gain %d ch %d Fail !\n", __FUNCTION__, __LINE__, eDmic, s8Gain, ch);
            goto FAIL;
            break;
    }

    return AIO_OK;

FAIL:

    return AIO_NG;
}

int HalAudDmaSineGenEnable(SINE_GEN_e enSineGen, BOOL bEn)
{
    switch (enSineGen)
    {
        case E_MHAL_SINEGEN_AO_DMA_A:
            HalBachWriteReg(E_BACH_REG_BANK1, E_BACH_DMA_TEST_CTRL5, REG_SINE_GEN_EN, (bEn ? REG_SINE_GEN_EN : 0));
            HalBachWriteReg(E_BACH_REG_BANK1, E_BACH_DMA_TEST_CTRL5, REG_SINE_GEN_L | REG_SINE_GEN_R,
                            REG_SINE_GEN_L | REG_SINE_GEN_R);
            HalBachWriteReg(E_BACH_REG_BANK1, E_BACH_DMA_TEST_CTRL5, REG_SINE_GEN_RD_WR, 0);
            break;

        case E_MHAL_SINEGEN_AO_DMA_B:
            HalBachWriteReg(E_BACH_REG_BANK1, E_BACH_DMA_TEST_CTRL6, REG_SINE_GEN_EN, (bEn ? REG_SINE_GEN_EN : 0));
            HalBachWriteReg(E_BACH_REG_BANK1, E_BACH_DMA_TEST_CTRL6, REG_SINE_GEN_L | REG_SINE_GEN_R,
                            REG_SINE_GEN_L | REG_SINE_GEN_R);
            HalBachWriteReg(E_BACH_REG_BANK1, E_BACH_DMA_TEST_CTRL6, REG_SINE_GEN_RD_WR, 0);
            break;

        case E_MHAL_SINEGEN_AI_DMA_A:
            HalBachWriteReg(E_BACH_REG_BANK1, E_BACH_DMA_TEST_CTRL5, REG_SINE_GEN_EN, (bEn ? REG_SINE_GEN_EN : 0));
            HalBachWriteReg(E_BACH_REG_BANK1, E_BACH_DMA_TEST_CTRL5, REG_SINE_GEN_L | REG_SINE_GEN_R,
                            REG_SINE_GEN_L | REG_SINE_GEN_R);
            HalBachWriteReg(E_BACH_REG_BANK1, E_BACH_DMA_TEST_CTRL5, REG_SINE_GEN_RD_WR, REG_SINE_GEN_RD_WR);
            break;

        case E_MHAL_SINEGEN_AI_DMA_B:
            HalBachWriteReg(E_BACH_REG_BANK1, E_BACH_DMA_TEST_CTRL6, REG_SINE_GEN_EN, (bEn ? REG_SINE_GEN_EN : 0));
            HalBachWriteReg(E_BACH_REG_BANK1, E_BACH_DMA_TEST_CTRL6, REG_SINE_GEN_L | REG_SINE_GEN_R,
                            REG_SINE_GEN_L | REG_SINE_GEN_R);
            HalBachWriteReg(E_BACH_REG_BANK1, E_BACH_DMA_TEST_CTRL6, REG_SINE_GEN_RD_WR, REG_SINE_GEN_RD_WR);
            break;

        case E_MHAL_SINEGEN_AI_IF_DMIC_A:
            HalBachWriteReg(E_BACH_REG_BANK3, E_BACH_VREC_CTRL03, REG_DMIC_SIN_EN, (bEn ? REG_DMIC_SIN_EN : 0));
            HalBachWriteReg(E_BACH_REG_BANK3, E_BACH_VREC_CTRL03, REG_SIN_PATH_SEL_LR_MSK, 0);
            HalBachWriteReg(E_BACH_REG_BANK3, E_BACH_VREC_CTRL03, REG_SIN_PATH_SEL_MSK, (2 << REG_SIN_PATH_SEL_POS));
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

int HalAudDmaSineGenSetting(SINE_GEN_e enSineGen, U8 u8Freq, S8 s8Gain)
{
    int ret = AIO_OK;

    if (u8Freq > 15) // 4 bits setting for gain
    {
        ERRMSG("Func:%s, Line:%d enSineGen %d u8Freq %d s8Gain %d Fail !\n", __FUNCTION__, __LINE__, enSineGen, u8Freq,
               s8Gain);
        goto FAIL;
    }

    if (s8Gain > 15) // 4 bits setting for gain
    {
        ERRMSG("Func:%s, Line:%d enSineGen %d u8Freq %d s8Gain %d Fail !\n", __FUNCTION__, __LINE__, enSineGen, u8Freq,
               s8Gain);
        goto FAIL;
    }

    ret |= HalAudDmaSineGenSetFreq(enSineGen, u8Freq);
    if (ret != AIO_OK)
    {
        ERRMSG("Func:%s, Line:%d enSineGen %d u8Freq %d s8Gain %d Fail !\n", __FUNCTION__, __LINE__, enSineGen, u8Freq,
               s8Gain);
        goto FAIL;
    }

    ret |= HalAudDmaSineGenSetGain(enSineGen, s8Gain);
    if (ret != AIO_OK)
    {
        ERRMSG("Func:%s, Line:%d enSineGen %d u8Freq %d s8Gain %d Fail !\n", __FUNCTION__, __LINE__, enSineGen, u8Freq,
               s8Gain);
        goto FAIL;
    }

    return AIO_OK;

FAIL:

    return AIO_NG;
}

int HalAudDmaSineGenSetFreq(SINE_GEN_e enSineGen, U8 u8Freq)
{
    if (u8Freq > 15) // 4 bits setting for gain
    {
        ERRMSG("Func:%s, Line:%d enSineGen %d u8Freq %d Fail !\n", __FUNCTION__, __LINE__, enSineGen, u8Freq);
        goto FAIL;
    }

    switch (enSineGen)
    {
        case E_MHAL_SINEGEN_AO_DMA_A:
            HalBachWriteReg(E_BACH_REG_BANK1, E_BACH_DMA_TEST_CTRL5, REG_SINE_GEN_FREQ_MSK,
                            u8Freq << REG_SINE_GEN_FREQ_POS);
            break;

        case E_MHAL_SINEGEN_AO_DMA_B:
            HalBachWriteReg(E_BACH_REG_BANK1, E_BACH_DMA_TEST_CTRL6, REG_SINE_GEN_FREQ_MSK,
                            u8Freq << REG_SINE_GEN_FREQ_POS);
            break;

        case E_MHAL_SINEGEN_AI_DMA_A:
            HalBachWriteReg(E_BACH_REG_BANK1, E_BACH_DMA_TEST_CTRL5, REG_SINE_GEN_FREQ_MSK,
                            u8Freq << REG_SINE_GEN_FREQ_POS);
            break;

        case E_MHAL_SINEGEN_AI_DMA_B:
            HalBachWriteReg(E_BACH_REG_BANK1, E_BACH_DMA_TEST_CTRL6, REG_SINE_GEN_FREQ_MSK,
                            u8Freq << REG_SINE_GEN_FREQ_POS);
            break;

        case E_MHAL_SINEGEN_AI_IF_DMIC_A:
            HalBachWriteReg(E_BACH_REG_BANK3, E_BACH_VREC_CTRL03, REG_SIN_FREQ_SEL_MSK, u8Freq << REG_SIN_FREQ_SEL_POS);
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

int HalAudDmaSineGenSetGain(SINE_GEN_e enSineGen, S8 s8Gain)
{
    if (s8Gain > 15) // 4 bits setting for gain
    {
        ERRMSG("Func:%s, Line:%d enSineGen %d s8Gain %d Fail !\n", __FUNCTION__, __LINE__, enSineGen, s8Gain);
        goto FAIL;
    }

    switch (enSineGen)
    {
        case E_MHAL_SINEGEN_AO_DMA_A:
            HalBachWriteReg(E_BACH_REG_BANK1, E_BACH_DMA_TEST_CTRL5, REG_SINE_GEN_GAIN_MSK,
                            s8Gain << REG_SINE_GEN_GAIN_POS);
            break;

        case E_MHAL_SINEGEN_AO_DMA_B:
            HalBachWriteReg(E_BACH_REG_BANK1, E_BACH_DMA_TEST_CTRL6, REG_SINE_GEN_GAIN_MSK,
                            s8Gain << REG_SINE_GEN_GAIN_POS);
            break;

        case E_MHAL_SINEGEN_AI_DMA_A:
            HalBachWriteReg(E_BACH_REG_BANK1, E_BACH_DMA_TEST_CTRL5, REG_SINE_GEN_GAIN_MSK,
                            s8Gain << REG_SINE_GEN_GAIN_POS);
            break;

        case E_MHAL_SINEGEN_AI_DMA_B:
            HalBachWriteReg(E_BACH_REG_BANK1, E_BACH_DMA_TEST_CTRL6, REG_SINE_GEN_GAIN_MSK,
                            s8Gain << REG_SINE_GEN_GAIN_POS);
            break;

        case E_MHAL_SINEGEN_AI_IF_DMIC_A:
            HalBachWriteReg(E_BACH_REG_BANK3, E_BACH_VREC_CTRL03, REG_SIN_GAIN_MSK, s8Gain << REG_SIN_GAIN_POS);
            break;

        default:
            ERRMSG("Func:%s, Line:%d enSineGen %d s8Gain %d Fail !\n", __FUNCTION__, __LINE__, enSineGen, s8Gain);
            goto FAIL;
            break;
    }

    return AIO_OK;

FAIL:

    return AIO_NG;
}

int HalAudDmaSineGenGetEnable(SINE_GEN_e enSineGen, BOOL *pbEn)
{
    u16 nValue = 0;

    switch (enSineGen)
    {
        case E_MHAL_SINEGEN_AO_DMA_A:

            nValue = HalBachReadReg(E_BACH_REG_BANK1, E_BACH_DMA_TEST_CTRL5);

            if (((nValue & REG_SINE_GEN_EN) == REG_SINE_GEN_EN)
                && ((nValue & (REG_SINE_GEN_L | REG_SINE_GEN_R)) == (REG_SINE_GEN_L | REG_SINE_GEN_R))
                && ((nValue & REG_SINE_GEN_RD_WR) == 0))
            {
                *pbEn = TRUE;
            }
            else
            {
                *pbEn = FALSE;
            }

            break;

        case E_MHAL_SINEGEN_AO_DMA_B:

            nValue = HalBachReadReg(E_BACH_REG_BANK1, E_BACH_DMA_TEST_CTRL6);

            if (((nValue & REG_SINE_GEN_EN) == REG_SINE_GEN_EN)
                && ((nValue & (REG_SINE_GEN_L | REG_SINE_GEN_R)) == (REG_SINE_GEN_L | REG_SINE_GEN_R))
                && ((nValue & REG_SINE_GEN_RD_WR) == 0))
            {
                *pbEn = TRUE;
            }
            else
            {
                *pbEn = FALSE;
            }

            break;

        case E_MHAL_SINEGEN_AI_DMA_A:

            nValue = HalBachReadReg(E_BACH_REG_BANK1, E_BACH_DMA_TEST_CTRL5);

            if (((nValue & REG_SINE_GEN_EN) == REG_SINE_GEN_EN)
                && ((nValue & (REG_SINE_GEN_L | REG_SINE_GEN_R)) == (REG_SINE_GEN_L | REG_SINE_GEN_R))
                && ((nValue & REG_SINE_GEN_RD_WR) == REG_SINE_GEN_RD_WR))
            {
                *pbEn = TRUE;
            }
            else
            {
                *pbEn = FALSE;
            }

            break;

        case E_MHAL_SINEGEN_AI_DMA_B:

            nValue = HalBachReadReg(E_BACH_REG_BANK1, E_BACH_DMA_TEST_CTRL6);

            if (((nValue & REG_SINE_GEN_EN) == REG_SINE_GEN_EN)
                && ((nValue & (REG_SINE_GEN_L | REG_SINE_GEN_R)) == (REG_SINE_GEN_L | REG_SINE_GEN_R))
                && ((nValue & REG_SINE_GEN_RD_WR) == REG_SINE_GEN_RD_WR))
            {
                *pbEn = TRUE;
            }
            else
            {
                *pbEn = FALSE;
            }

            break;

        case E_MHAL_SINEGEN_AI_IF_DMIC_A:

            nValue = HalBachReadReg(E_BACH_REG_BANK3, E_BACH_VREC_CTRL03);

            if (((nValue & REG_DMIC_SIN_EN) == REG_DMIC_SIN_EN) && ((nValue & REG_SIN_PATH_SEL_LR_MSK) == 0)
                && ((nValue & REG_SIN_PATH_SEL_MSK) == (2 << REG_SIN_PATH_SEL_POS)))
            {
                *pbEn = TRUE;
            }
            else
            {
                *pbEn = FALSE;
            }

            break;

        default:
            ERRMSG("Func:%s, Line:%d enSineGen %d Fail !\n", __FUNCTION__, __LINE__, enSineGen);
            goto FAIL;
            break;
    }

    return AIO_OK;

FAIL:

    return AIO_NG;
}

int HalAudDmaSineGenGetSetting(SINE_GEN_e enSineGen, U8 *pu8Freq, S8 *ps8Gain)
{
    u16 nValue = 0;

    switch (enSineGen)
    {
        case E_MHAL_SINEGEN_AO_DMA_A:

            nValue = HalBachReadReg(E_BACH_REG_BANK1, E_BACH_DMA_TEST_CTRL5);

            *ps8Gain = (nValue & REG_SINE_GEN_GAIN_MSK) >> REG_SINE_GEN_GAIN_POS;
            *pu8Freq = (nValue & REG_SINE_GEN_FREQ_MSK) >> REG_SINE_GEN_FREQ_POS;

            break;

        case E_MHAL_SINEGEN_AO_DMA_B:

            nValue = HalBachReadReg(E_BACH_REG_BANK1, E_BACH_DMA_TEST_CTRL6);

            *ps8Gain = (nValue & REG_SINE_GEN_GAIN_MSK) >> REG_SINE_GEN_GAIN_POS;
            *pu8Freq = (nValue & REG_SINE_GEN_FREQ_MSK) >> REG_SINE_GEN_FREQ_POS;

            break;

        case E_MHAL_SINEGEN_AI_DMA_A:

            nValue = HalBachReadReg(E_BACH_REG_BANK1, E_BACH_DMA_TEST_CTRL5);

            *ps8Gain = (nValue & REG_SINE_GEN_GAIN_MSK) >> REG_SINE_GEN_GAIN_POS;
            *pu8Freq = (nValue & REG_SINE_GEN_FREQ_MSK) >> REG_SINE_GEN_FREQ_POS;

            break;

        case E_MHAL_SINEGEN_AI_DMA_B:

            nValue = HalBachReadReg(E_BACH_REG_BANK1, E_BACH_DMA_TEST_CTRL6);

            *ps8Gain = (nValue & REG_SINE_GEN_GAIN_MSK) >> REG_SINE_GEN_GAIN_POS;
            *pu8Freq = (nValue & REG_SINE_GEN_FREQ_MSK) >> REG_SINE_GEN_FREQ_POS;

            break;

        case E_MHAL_SINEGEN_AI_IF_DMIC_A:

            nValue = HalBachReadReg(E_BACH_REG_BANK3, E_BACH_VREC_CTRL03);

            *ps8Gain = (nValue & REG_SIN_GAIN_MSK) >> REG_SIN_GAIN_POS;
            *pu8Freq = (nValue & REG_SIN_FREQ_SEL_MSK) >> REG_SIN_FREQ_SEL_POS;

            break;

        default:
            ERRMSG("Func:%s, Line:%d enSineGen %d Fail !\n", __FUNCTION__, __LINE__, enSineGen);
            goto FAIL;
            break;
    }

    return AIO_OK;

FAIL:

    return AIO_NG;
}

int HalAudHdmiEanble(CHIP_AO_HDMI_e eHdmi, BOOL bEn)
{
    switch (eHdmi)
    {
        case E_CHIP_AO_HDMI_A:
            if (bEn)
            {
                HalBachWriteReg(E_BACH_REG_BANK1, E_BACH_AU2HDMI_CTRL, REG_AU2HDMI_EN, REG_AU2HDMI_EN);
                HalBachWriteReg(E_BACH_REG_BANK1, E_BACH_AU2HDMI_REF_FS, REG_AU2HDMI_1K_MSK,
                                0 << REG_AU2HDMI_1K_POS); // 1KHz
            }
            else
            {
                HalBachWriteReg(E_BACH_REG_BANK1, E_BACH_AU2HDMI_CTRL, REG_AU2HDMI_EN, 0);
            }
            break;

        default:
            ERRMSG("Func:%s, Line:%d eHdmi %d Fail !\n", __FUNCTION__, __LINE__, eHdmi);
            goto FAIL;
            break;
    }

    return AIO_OK;

FAIL:

    return AIO_NG;
}

int HalAudAiSetGain(AI_CH_e eAiCh, S16 s16Gain)
{
    int ret     = AIO_OK;
    int ch      = 0;
    int v       = 0;
    U16 adcGain = 0;
    U16 preGain = 0;
    int line    = 0;

    //
    if ((s16Gain >= CHIP_ADC_GAIN_STEP_TOTAL) || (s16Gain < 0))
    {
        ERRMSG("Func:%s, Line:%d eAiCh %d s16Gain %d Fail !\n", __FUNCTION__, __LINE__, eAiCh, s16Gain);
        goto FAIL;
    }

    //
    preGain = g_aMicInCombineGainTable[s16Gain][0];
    adcGain = g_aMicInCombineGainTable[s16Gain][1];

    //
    switch (eAiCh)
    {
        case E_AI_CH_ADC_A_0:
        case E_AI_CH_ADC_B_0:

            if (eAiCh == E_AI_CH_ADC_A_0)
            {
                v = CHIP_AI_ADC_IDX_BY_DEV(E_AI_DEV_ADC_A);
            }
            else if (eAiCh == E_AI_CH_ADC_B_0)
            {
                v = CHIP_AI_ADC_IDX_BY_DEV(E_AI_DEV_ADC_B);
            }
            else
            {
                line = __LINE__;
                goto FAIL;
            }

            if (!CHIP_AI_ADC_VALID(v))
            {
                line = __LINE__;
                goto FAIL;
            }

            ret |= HalAudAtopSetAdcGain((CHIP_AI_ADC_e)v, adcGain); // 0~3:0dB, 1:6dB, 2:12dB, 3:18dB
            if (ret != AIO_OK)
            {
                line = __LINE__;
                goto FAIL;
            }

            ret |= HalAudAtopMicAmpGain((CHIP_AI_ADC_e)v, preGain); // 0~D : -6dB~39dB
            if (ret != AIO_OK)
            {
                line = __LINE__;
                goto FAIL;
            }

            break;

        case E_AI_CH_ADC_C_0:
            break;

        case E_AI_CH_ADC_D_0:
            break;

        case E_AI_CH_DMIC_A_0:
        case E_AI_CH_DMIC_A_1:
        case E_AI_CH_DMIC_A_2:
        case E_AI_CH_DMIC_A_3:

            if (eAiCh == E_AI_CH_DMIC_A_0)
            {
                ch = 0;
            }
            else if (eAiCh == E_AI_CH_DMIC_A_1)
            {
                ch = 1;
            }
            else if (eAiCh == E_AI_CH_DMIC_A_2)
            {
                ch = 2;
            }
            else if (eAiCh == E_AI_CH_DMIC_A_3)
            {
                ch = 3;
            }
            else
            {
                line = __LINE__;
                goto FAIL;
            }

            v = CHIP_AI_DMIC_IDX_BY_DEV(E_AI_DEV_DMIC_A);
            if (!CHIP_AI_DMIC_VALID(v))
            {
                line = __LINE__;
                goto FAIL;
            }

            ret |= HalAudDigMicSetGain((CHIP_AI_DMIC_e)v, (S8)s16Gain, (S8)ch);
            if (ret != AIO_OK)
            {
                line = __LINE__;
                goto FAIL;
            }

            break;

        case E_AI_CH_ECHO_A_0:
            break;

        case E_AI_CH_ECHO_A_1:
            break;

        case E_AI_CH_I2S_RX_A_0:
        case E_AI_CH_I2S_RX_A_1:
        case E_AI_CH_I2S_RX_A_2:
        case E_AI_CH_I2S_RX_A_3:
        case E_AI_CH_I2S_RX_A_4:
        case E_AI_CH_I2S_RX_A_5:
        case E_AI_CH_I2S_RX_A_6:
        case E_AI_CH_I2S_RX_A_7:
            break;

        case E_AI_CH_I2S_RX_B_0:
        case E_AI_CH_I2S_RX_B_1:
        case E_AI_CH_I2S_RX_B_2:
        case E_AI_CH_I2S_RX_B_3:
        case E_AI_CH_I2S_RX_B_4:
        case E_AI_CH_I2S_RX_B_5:
        case E_AI_CH_I2S_RX_B_6:
        case E_AI_CH_I2S_RX_B_7:
            break;

        case E_AI_CH_I2S_RX_C_0:
        case E_AI_CH_I2S_RX_C_1:
        case E_AI_CH_I2S_RX_C_2:
        case E_AI_CH_I2S_RX_C_3:
        case E_AI_CH_I2S_RX_C_4:
        case E_AI_CH_I2S_RX_C_5:
        case E_AI_CH_I2S_RX_C_6:
        case E_AI_CH_I2S_RX_C_7:
            break;

        case E_AI_CH_I2S_RX_D_0:
        case E_AI_CH_I2S_RX_D_1:
        case E_AI_CH_I2S_RX_D_2:
        case E_AI_CH_I2S_RX_D_3:
        case E_AI_CH_I2S_RX_D_4:
        case E_AI_CH_I2S_RX_D_5:
        case E_AI_CH_I2S_RX_D_6:
        case E_AI_CH_I2S_RX_D_7:
            break;

        default:
            line = __LINE__;
            goto FAIL;
            break;
    }

    return AIO_OK;

FAIL:
    ERRMSG("Func:%s, Line:%d eAiCh %d Gain %d fail\n", __func__, line, eAiCh, s16Gain);

    return AIO_NG;
}

int HalAudAoSetGain(AO_CH_e eAoCh, S16 s16Gain)
{
    //    int ret = AIO_OK;

    return AIO_OK;

    // FAIL:

    return AIO_NG;
}

int HalAudAiSetIfMute(AI_CH_e eAiCh, BOOL bEn)
{
    return AIO_OK;
}

int HalAudAoSetIfMute(AO_CH_e eAoCh, BOOL bEn)
{
    switch (eAoCh)
    {
        case E_AO_CH_DAC_A_0:
        case E_AO_CH_DAC_B_0:
            _HalAudDacSetMute(eAoCh, bEn);
            break;
        default:
            ERRMSG("Func:%s, Line:%d eAoCh %d not support\n", __func__, __LINE__, eAoCh);
            return AIO_NG;
    }
    return AIO_OK;
}

int HalAudAiSetIfSampleRate(AI_IF_e eAiIf, u32 nSampleRate)
{
    AudRate_e eRate = 0;

    switch (nSampleRate)
    {
        case 8000:
            eRate = E_AUD_RATE_8K;
            break;
        case 11000:
            eRate = E_AUD_RATE_11K;
            break;
        case 12000:
            eRate = E_AUD_RATE_12K;
            break;
        case 16000:
            eRate = E_AUD_RATE_16K;
            break;
        case 22000:
            eRate = E_AUD_RATE_22K;
            break;
        case 24000:
            eRate = E_AUD_RATE_24K;
            break;
        case 32000:
            eRate = E_AUD_RATE_32K;
            break;
        case 44000:
            eRate = E_AUD_RATE_44K;
            break;
        case 48000:
            eRate = E_AUD_RATE_48K;
            break;
        default:
            ERRMSG("Func:%s, Line:%d eAiIf %d nSampleRate %d Fail !\n", __FUNCTION__, __LINE__, eAiIf, nSampleRate);
            goto FAIL;
            break;
    }

    //
    switch (eAiIf)
    {
        case E_MHAL_AI_IF_ADC_A_0_B_0:

            switch (eRate)
            {
                case E_AUD_RATE_8K:
                    HalBachWriteReg(E_BACH_REG_BANK1, E_BACH_SR0_SEL, REG_CIC_3_SEL_MSK, 0 << REG_CIC_3_SEL_POS);
                    break;

                case E_AUD_RATE_16K:
                    HalBachWriteReg(E_BACH_REG_BANK1, E_BACH_SR0_SEL, REG_CIC_3_SEL_MSK, 1 << REG_CIC_3_SEL_POS);
                    break;

                case E_AUD_RATE_32K:
                    HalBachWriteReg(E_BACH_REG_BANK1, E_BACH_SR0_SEL, REG_CIC_3_SEL_MSK, 2 << REG_CIC_3_SEL_POS);
                    break;

                case E_AUD_RATE_48K:
                    HalBachWriteReg(E_BACH_REG_BANK1, E_BACH_SR0_SEL, REG_CIC_3_SEL_MSK, 3 << REG_CIC_3_SEL_POS);
                    break;

                default:
                    ERRMSG("Func:%s, Line:%d eAiIf %d eRate %d Fail !\n", __FUNCTION__, __LINE__, eAiIf, eRate);
                    goto FAIL;
            }

            break;

        case E_MHAL_AI_IF_ADC_C_0_D_0:

            switch (eRate)
            {
                case E_AUD_RATE_8K:
                    HalBachWriteReg(E_BACH_REG_BANK1, E_BACH_SR0_SEL_AU2, REG_CIC_3_SEL_AU2_MSK,
                                    0 << REG_CIC_3_SEL_AU2_POS);
                    break;

                case E_AUD_RATE_16K:
                    HalBachWriteReg(E_BACH_REG_BANK1, E_BACH_SR0_SEL_AU2, REG_CIC_3_SEL_AU2_MSK,
                                    1 << REG_CIC_3_SEL_AU2_POS);
                    break;

                case E_AUD_RATE_32K:
                    HalBachWriteReg(E_BACH_REG_BANK1, E_BACH_SR0_SEL_AU2, REG_CIC_3_SEL_AU2_MSK,
                                    2 << REG_CIC_3_SEL_AU2_POS);
                    break;

                case E_AUD_RATE_48K:
                    HalBachWriteReg(E_BACH_REG_BANK1, E_BACH_SR0_SEL_AU2, REG_CIC_3_SEL_AU2_MSK,
                                    3 << REG_CIC_3_SEL_AU2_POS);
                    break;

                default:
                    ERRMSG("Func:%s, Line:%d eAiIf %d eRate %d Fail !\n", __FUNCTION__, __LINE__, eAiIf, eRate);
                    goto FAIL;
            }

            break;

        case E_MHAL_AI_IF_DMIC_A_0_1:
            break;

        case E_MHAL_AI_IF_DMIC_A_2_3:
            break;

        case E_MHAL_AI_IF_HDMI_A_0_1:
            break;

        case E_MHAL_AI_IF_ECHO_A_0_1:
            break;

        case E_MHAL_AI_IF_I2S_RX_A_0_1:
        case E_MHAL_AI_IF_I2S_RX_A_2_3:
        case E_MHAL_AI_IF_I2S_RX_A_4_5:
        case E_MHAL_AI_IF_I2S_RX_A_6_7:
        case E_MHAL_AI_IF_I2S_RX_A_8_9:
        case E_MHAL_AI_IF_I2S_RX_A_10_11:
        case E_MHAL_AI_IF_I2S_RX_A_12_13:
        case E_MHAL_AI_IF_I2S_RX_A_14_15:
            break;

        case E_MHAL_AI_IF_I2S_RX_B_0_1:
        case E_MHAL_AI_IF_I2S_RX_B_2_3:
        case E_MHAL_AI_IF_I2S_RX_B_4_5:
        case E_MHAL_AI_IF_I2S_RX_B_6_7:
        case E_MHAL_AI_IF_I2S_RX_B_8_9:
        case E_MHAL_AI_IF_I2S_RX_B_10_11:
        case E_MHAL_AI_IF_I2S_RX_B_12_13:
        case E_MHAL_AI_IF_I2S_RX_B_14_15:
            break;

        case E_MHAL_AI_IF_I2S_RX_C_0_1:
        case E_MHAL_AI_IF_I2S_RX_C_2_3:
        case E_MHAL_AI_IF_I2S_RX_C_4_5:
        case E_MHAL_AI_IF_I2S_RX_C_6_7:
        case E_MHAL_AI_IF_I2S_RX_C_8_9:
        case E_MHAL_AI_IF_I2S_RX_C_10_11:
        case E_MHAL_AI_IF_I2S_RX_C_12_13:
        case E_MHAL_AI_IF_I2S_RX_C_14_15:
            break;

        case E_MHAL_AI_IF_I2S_RX_D_0_1:
        case E_MHAL_AI_IF_I2S_RX_D_2_3:
        case E_MHAL_AI_IF_I2S_RX_D_4_5:
        case E_MHAL_AI_IF_I2S_RX_D_6_7:
        case E_MHAL_AI_IF_I2S_RX_D_8_9:
        case E_MHAL_AI_IF_I2S_RX_D_10_11:
        case E_MHAL_AI_IF_I2S_RX_D_12_13:
        case E_MHAL_AI_IF_I2S_RX_D_14_15:
            break;

        default:
            break;
    }

    return AIO_OK;

FAIL:

    return AIO_NG;
}

int HalAudAoSetIfSampleRate(AO_IF_e eAoIf, u32 nSampleRate)
{
    int       ret   = AIO_OK;
    AudRate_e eRate = 0;

    switch (nSampleRate)
    {
        case 8000:
            eRate = E_AUD_RATE_8K;
            break;
        case 11000:
            eRate = E_AUD_RATE_11K;
            break;
        case 12000:
            eRate = E_AUD_RATE_12K;
            break;
        case 16000:
            eRate = E_AUD_RATE_16K;
            break;
        case 22000:
            eRate = E_AUD_RATE_22K;
            break;
        case 24000:
            eRate = E_AUD_RATE_24K;
            break;
        case 32000:
            eRate = E_AUD_RATE_32K;
            break;
        case 44000:
            eRate = E_AUD_RATE_44K;
            break;
        case 48000:
            eRate = E_AUD_RATE_48K;
            break;
        default:
            ERRMSG("Func:%s, Line:%d eAoIf %d nSampleRate %d Fail !\n", __FUNCTION__, __LINE__, eAoIf, nSampleRate);
            goto FAIL;
            break;
    }

    //
    if ((eAoIf & E_MHAL_AO_IF_DAC_A_0) || (eAoIf & E_MHAL_AO_IF_DAC_B_0))
    {
        ret |= HalAudDmaSetRate(E_CHIP_AIO_DMA_AO_A, eRate);
        if (ret != AIO_OK)
        {
            ERRMSG("Func:%s, Line:%d eAoIf %d eRate %d Fail !\n", __FUNCTION__, __LINE__, eAoIf, eRate);
            goto FAIL;
        }
    }

    if ((eAoIf & E_MHAL_AO_IF_DAC_C_0) || (eAoIf & E_MHAL_AO_IF_DAC_D_0))
    {
        ret |= HalAudDmaSetRate(E_CHIP_AIO_DMA_AO_B, eRate);
        if (ret != AIO_OK)
        {
            ERRMSG("Func:%s, Line:%d eAoIf %d eRate %d Fail !\n", __FUNCTION__, __LINE__, eAoIf, eRate);
            goto FAIL;
        }
    }

    if (eAoIf & E_MHAL_AO_IF_HDMI_A_0)
    {
    }

    if (eAoIf & E_MHAL_AO_IF_HDMI_A_1)
    {
    }

    if (eAoIf & E_MHAL_AO_IF_ECHO_A_0)
    {
    }

    if (eAoIf & E_MHAL_AO_IF_ECHO_A_1)
    {
    }

    if (eAoIf & E_MHAL_AO_IF_I2S_TX_A_0)
    {
    }

    if (eAoIf & E_MHAL_AO_IF_I2S_TX_A_1)
    {
    }

    if (eAoIf & E_MHAL_AO_IF_I2S_TX_B_0)
    {
    }

    if (eAoIf & E_MHAL_AO_IF_I2S_TX_B_1)
    {
    }

    return AIO_OK;

FAIL:

    return AIO_NG;
}

int HalAudAiSetDpgaGain(CHIP_AIO_DMA_e eAioDma, U8 nDmaCh, AI_CH_e eAiCh, S16 s16Gain)
{
    int ret    = AIO_OK;
    int dpgaCh = 0;
    int dpga   = 0;

    // enAiDma may be used in new chip
    // nDmaCh may be used in new chip

    switch (eAiCh)
    {
        case E_AI_CH_ADC_A_0:
            dpga   = E_CHIP_DPGA_C;
            dpgaCh = 0;
            break;

        case E_AI_CH_ADC_B_0:
            dpga   = E_CHIP_DPGA_D;
            dpgaCh = 0;
            break;

        case E_AI_CH_ADC_C_0:
            dpga   = E_CHIP_DPGA_F;
            dpgaCh = 0;
            break;

        case E_AI_CH_ADC_D_0:
            dpga   = E_CHIP_DPGA_F;
            dpgaCh = 1;
            break;

        case E_AI_CH_ECHO_A_0:
            dpga   = E_CHIP_DPGA_E;
            dpgaCh = 0;
            break;

        case E_AI_CH_ECHO_A_1:
            dpga   = E_CHIP_DPGA_E;
            dpgaCh = 1;
            break;

        default:
            ERRMSG("Func:%s, Line:%d eAioDma %d nDmaCh %d eAiCh %d s16Gain %d Fail !\n", __FUNCTION__, __LINE__,
                   eAioDma, nDmaCh, eAiCh, s16Gain);
            goto FAIL;
            break;
    }
    if (s16Gain != BACH_DPGA_GAIN_MIN_DB)
    {
        g_nDpgaGain[(EN_CHIP_DPGA)dpga][dpgaCh] = (S8)s16Gain;
    }
    ret |= HalAudDpgaSetGain((EN_CHIP_DPGA)dpga, (S8)s16Gain, (S8)dpgaCh);
    if (ret != AIO_OK)
    {
        ERRMSG("Func:%s, Line:%d eAioDma %d nDmaCh %d eAiCh %d s16Gain %d Fail !\n", __FUNCTION__, __LINE__, eAioDma,
               nDmaCh, eAiCh, s16Gain);
        goto FAIL;
    }

    return AIO_OK;

FAIL:

    return AIO_NG;
}

int HalAudAoSetDpgaGain(CHIP_AIO_DMA_e eAioDma, U8 nDmaCh, AO_CH_e eAoCh, S16 s16Gain, U8 nFading)
{
    int ret    = AIO_OK;
    int dpgaCh = 0;
    int dpga   = 0;

    switch (eAioDma)
    {
        case E_CHIP_AIO_DMA_AO_A:
            ret = _HalAudAoaSelectDpgaByIf(eAoCh, &dpga);
            if (ret != AIO_OK)
            {
                goto FAIL;
            }
            break;
        case E_CHIP_AIO_DMA_AO_B:
            ret = _HalAudAobSelectDpgaByIf(eAoCh, &dpga);
            if (ret != AIO_OK)
            {
                goto FAIL;
            }
            break;
        case E_CHIP_AIO_DMA_AO_C:
            ret = _HalAudAocSelectDpgaByIf(eAoCh, &dpga);
            if (ret != AIO_OK)
            {
                goto FAIL;
            }
            break;
        case E_CHIP_AIO_DMA_AO_DIRECT_A:
            ret = _HalAudAoDirectSelectDpgaByIf(eAoCh, &dpga);
            if (ret != AIO_OK)
            {
                goto FAIL;
            }
            break;
        default:
            ERRMSG("Func:%s, Line:%d eAioDma %d nDmaCh %d eAoCh %d s16Gain %d Fail !\n", __FUNCTION__, __LINE__,
                   eAioDma, nDmaCh, eAoCh, s16Gain);
            goto FAIL;
            break;
    }

    dpgaCh = ((nDmaCh % 2) == 0) ? 0 : 1;

    ret |= HalAudDpgaSetGainFading((EN_CHIP_DPGA)dpga, nFading, dpgaCh);
    if (ret != AIO_OK)
    {
        ERRMSG("Func:%s, Line:%d eAioDma %d nDmaCh %d eAoCh %d s16Gain %d Set Gain Fading Fail !\n", __FUNCTION__,
               __LINE__, eAioDma, nDmaCh, eAoCh, s16Gain);
        goto FAIL;
    }
    if (s16Gain != BACH_DPGA_GAIN_MIN_DB)
    {
        g_nDpgaGain[(EN_CHIP_DPGA)dpga][dpgaCh] = (S8)s16Gain;
    }
    ret |= HalAudDpgaSetGain((EN_CHIP_DPGA)dpga, (S8)s16Gain, dpgaCh);
    if (ret != AIO_OK)
    {
        ERRMSG("Func:%s, Line:%d eAioDma %d nDmaCh %d eAoCh %d s16Gain %d Set Gain Fail !\n", __FUNCTION__, __LINE__,
               eAioDma, nDmaCh, eAoCh, s16Gain);
        goto FAIL;
    }

    return AIO_OK;

FAIL:

    return AIO_NG;
}

int HalAudAiSetDpgaMute(CHIP_AIO_DMA_e eAioDma, U8 nDmaCh, AI_CH_e eAiCh, BOOL bEn)
{
    int ret    = AIO_OK;
    int dpgaCh = 0;
    int dpga   = 0;

    // enAiDma may be used in new chip
    // nDmaCh may be used in new chip

    switch (eAiCh)
    {
        case E_AI_CH_ADC_A_0:
            dpga   = E_CHIP_DPGA_C;
            dpgaCh = 0;
            break;

        case E_AI_CH_ADC_B_0:
            dpga   = E_CHIP_DPGA_D;
            dpgaCh = 0;
            break;

        case E_AI_CH_ADC_C_0:
            dpga   = E_CHIP_DPGA_F;
            dpgaCh = 0;
            break;

        case E_AI_CH_ADC_D_0:
            dpga   = E_CHIP_DPGA_F;
            dpgaCh = 1;
            break;

        case E_AI_CH_ECHO_A_0:
            dpga   = E_CHIP_DPGA_E;
            dpgaCh = 0;
            break;

        case E_AI_CH_ECHO_A_1:
            dpga   = E_CHIP_DPGA_E;
            dpgaCh = 1;
            break;

        default:
            ERRMSG("Func:%s, Line:%d eAioDma %d nDmaCh %d eAiCh %d Fail !\n", __FUNCTION__, __LINE__, eAioDma, nDmaCh,
                   eAiCh);
            goto FAIL;
            break;
    }

    if (bEn)
    {
        ret |= HalAudDpgaSetGain((EN_CHIP_DPGA)dpga, BACH_DPGA_GAIN_MIN_DB, (S8)dpgaCh);
        if (ret != AIO_OK)
        {
            ERRMSG("Func:%s, Line:%d eAioDma %d nDmaCh %d eAiCh %d Fail !\n", __FUNCTION__, __LINE__, eAioDma, nDmaCh,
                   eAiCh);
            goto FAIL;
        }
    }
    else
    {
        ret |= HalAudDpgaSetGain((EN_CHIP_DPGA)dpga, g_nDpgaGain[(EN_CHIP_DPGA)dpga][dpgaCh], (S8)dpgaCh);
        if (ret != AIO_OK)
        {
            ERRMSG("Func:%s, Line:%d eAioDma %d nDmaCh %d eAiCh %d Fail !\n", __FUNCTION__, __LINE__, eAioDma, nDmaCh,
                   eAiCh);
            goto FAIL;
        }
    }

    return AIO_OK;

FAIL:

    return AIO_NG;
}

int HalAudAoSetDpgaMute(CHIP_AIO_DMA_e eAioDma, U8 nDmaCh, AO_CH_e eAoCh, BOOL bEn, U8 nFading)
{
    int ret    = AIO_OK;
    int dpgaCh = 0;
    int dpga   = 0;

    switch (eAioDma)
    {
        case E_CHIP_AIO_DMA_AO_A:
            ret = _HalAudAoaSelectDpgaByIf(eAoCh, &dpga);
            if (ret != AIO_OK)
            {
                goto FAIL;
            }
            break;
        case E_CHIP_AIO_DMA_AO_B:
            ret = _HalAudAobSelectDpgaByIf(eAoCh, &dpga);
            if (ret != AIO_OK)
            {
                goto FAIL;
            }
            break;
        case E_CHIP_AIO_DMA_AO_C:
            ret = _HalAudAocSelectDpgaByIf(eAoCh, &dpga);
            if (ret != AIO_OK)
            {
                goto FAIL;
            }
            break;
        case E_CHIP_AIO_DMA_AO_DIRECT_A:
            ret = _HalAudAoDirectSelectDpgaByIf(eAoCh, &dpga);
            if (ret != AIO_OK)
            {
                goto FAIL;
            }
            break;
        default:
            ERRMSG("Func:%s, Line:%d eAioDma %d nDmaCh %d eAoCh %d Fail !\n", __FUNCTION__, __LINE__, eAioDma, nDmaCh,
                   eAoCh);
            goto FAIL;
            break;
    }

    dpgaCh = ((nDmaCh % 2) == 0) ? 0 : 1;

    if (bEn)
    {
        HalAudAoSetDpgaGain(eAioDma, nDmaCh, eAoCh, BACH_DPGA_GAIN_MIN_DB, nFading);
    }
    else
    {
        HalAudAoSetDpgaGain(eAioDma, nDmaCh, eAoCh, g_nDpgaGain[(EN_CHIP_DPGA)dpga][dpgaCh], nFading);
    }

    return AIO_OK;

FAIL:

    return AIO_NG;
}

int HalAudDirectSetDpgaGain(AI_CH_e eAiCh, AO_CH_e eAoCh, S16 s16Gain)
{
    int ret    = AIO_OK;
    int dpgaCh = 0;
    int dpga   = 0;

    switch (eAiCh)
    {
        case E_AI_CH_ADC_C_0:

            if (eAoCh == E_AO_CH_DAC_A_0)
            {
                dpga   = E_CHIP_DPGA_B;
                dpgaCh = 0;
            }
            else
            {
                ERRMSG("Func:%s, Line:%d eAiCh %d eAoCh %d s16Gain %d Fail !\n", __FUNCTION__, __LINE__, eAiCh, eAoCh,
                       s16Gain);
                goto FAIL;
            }

            break;

        default:
            ERRMSG("Func:%s, Line:%d eAiCh %d eAoCh %d s16Gain %d Fail !\n", __FUNCTION__, __LINE__, eAiCh, eAoCh,
                   s16Gain);
            goto FAIL;
            break;
    }

    //
    ret |= HalAudDpgaSetGain((EN_CHIP_DPGA)dpga, (S8)s16Gain, (S8)dpgaCh);
    if (ret != AIO_OK)
    {
        ERRMSG("Func:%s, Line:%d eAiCh %d eAoCh %d s16Gain %d Fail !\n", __FUNCTION__, __LINE__, eAiCh, eAoCh, s16Gain);
        goto FAIL;
    }

    return AIO_OK;

FAIL:

    return AIO_NG;
}

int HalAudConfigDigMicParam(CHIP_AI_DMIC_e eChipDmic, DigMicParam_t *ptDigMicParam)
{
    int       ret = AIO_OK;
    AudRate_e eSampleRate;
    u8        nChNum = 0;

    switch (ptDigMicParam->nSampleRate)
    {
        case 8000:
            eSampleRate = E_AUD_RATE_8K;
            break;
        case 16000:
            eSampleRate = E_AUD_RATE_16K;
            break;
        case 32000:
            eSampleRate = E_AUD_RATE_32K;
            break;
        case 48000:
            eSampleRate = E_AUD_RATE_48K;
            break;
        default:
            ERRMSG("Func:%s, Line:%d eChipDmic %d Fail !\n", __FUNCTION__, __LINE__, eChipDmic);
            goto FAIL;
            break;
    }

    nChNum = ptDigMicParam->nChNum;

    ret |= HalAudDigMicSetRate(eChipDmic, eSampleRate);
    if (ret != AIO_OK)
    {
        ERRMSG("Func:%s, Line:%d eChipDmic %d eSampleRate %d Fail !\n", __FUNCTION__, __LINE__, eChipDmic, eSampleRate);
        goto FAIL;
    }

    ret |= HalAudDigMicSetChannel(eChipDmic, nChNum);
    if (ret != AIO_OK)
    {
        ERRMSG("Func:%s, Line:%d eChipDmic %d nChNum %d Fail !\n", __FUNCTION__, __LINE__, eChipDmic, nChNum);
        goto FAIL;
    }

    return AIO_OK;

FAIL:

    return AIO_NG;
}

int HalAudConfigDigMicSampleRate(CHIP_AI_DMIC_e eChipDmic, u32 nSampleRate)
{
    int       ret = AIO_OK;
    AudRate_e eSampleRate;

    switch (nSampleRate)
    {
        case 8000:
            eSampleRate = E_AUD_RATE_8K;
            break;
        case 16000:
            eSampleRate = E_AUD_RATE_16K;
            break;
        case 32000:
            eSampleRate = E_AUD_RATE_32K;
            break;
        case 48000:
            eSampleRate = E_AUD_RATE_48K;
            break;
        default:
            ERRMSG("Func:%s, Line:%d eChipDmic %d Fail !\n", __FUNCTION__, __LINE__, eChipDmic);
            goto FAIL;
            break;
    }

    ret |= HalAudDigMicSetRate(eChipDmic, eSampleRate);
    if (ret != AIO_OK)
    {
        ERRMSG("Func:%s, Line:%d eChipDmic %d eSampleRate %d Fail !\n", __FUNCTION__, __LINE__, eChipDmic, eSampleRate);
        goto FAIL;
    }

    return AIO_OK;

FAIL:

    return AIO_NG;
}

int HalAudConfigDigMicChNum(CHIP_AI_DMIC_e eChipDmic, u8 nChNum)
{
    int ret = AIO_OK;

    if (nChNum > 4)
    {
        ERRMSG("Func:%s, Line:%d eChipDmic %d nChNum %d Fail !\n", __FUNCTION__, __LINE__, eChipDmic, nChNum);
        goto FAIL;
    }

    ret |= HalAudDigMicSetChannel(eChipDmic, nChNum);
    if (ret != AIO_OK)
    {
        ERRMSG("Func:%s, Line:%d eChipDmic %d nChNum %d Fail !\n", __FUNCTION__, __LINE__, eChipDmic, nChNum);
        goto FAIL;
    }

    return AIO_OK;

FAIL:

    return AIO_NG;
}

int HalAudConfigSrcParam(AI_CH_e eAiCh, SrcParam_t *ptSrcParam)
{
    int       ret = AIO_OK;
    AudRate_e eSampleRate;

    switch (eAiCh)
    {
        case E_AI_CH_ECHO_A_0:
            break;

        case E_AI_CH_ECHO_A_1:
            break;

        default:
            ERRMSG("Func:%s, Line:%d eAiCh %d Fail !\n", __FUNCTION__, __LINE__, eAiCh);
            goto FAIL;
            break;
    }

    switch (ptSrcParam->nSampleRate)
    {
        case 8000:
            eSampleRate = E_AUD_RATE_8K;
            break;
        case 16000:
            eSampleRate = E_AUD_RATE_16K;
            break;
        case 32000:
            eSampleRate = E_AUD_RATE_32K;
            break;
        case 48000:
            eSampleRate = E_AUD_RATE_48K;
            break;
        default:
            ERRMSG("Func:%s, Line:%d eAiCh %d ptSrcParam->nSampleRate %d Fail !\n", __FUNCTION__, __LINE__, eAiCh,
                   ptSrcParam->nSampleRate);
            goto FAIL;
            break;
    }

    ret |= HalAudSrcSetRate(eAiCh, eSampleRate);
    if (ret != AIO_OK)
    {
        ERRMSG("Func:%s, Line:%d eAiCh %d Fail !\n", __FUNCTION__, __LINE__, eAiCh);
        goto FAIL;
    }

    return AIO_OK;

FAIL:

    return AIO_NG;
}

int HalAudAoIfMultiAttachAction(AO_CH_e eAoCh, U8 nAttachCount)
{
    int ret = AIO_OK;

    switch (eAoCh)
    {
        case E_AO_CH_DAC_A_0:

            if (nAttachCount == 0)
            {
                // Recover DAC mux to default setting
                ret |= HalAudSetMux(E_AUD_MUX_SDM_DINL, 0);
                if (ret != AIO_OK)
                {
                    ERRMSG("Func:%s, Line:%d eAoCh %d nAttachCount %d Fail !\n", __FUNCTION__, __LINE__, eAoCh,
                           nAttachCount);
                    goto FAIL;
                }
            }

            break;

        case E_AO_CH_DAC_B_0:

            if (nAttachCount == 0)
            {
                // Recover DAC mux to default setting
                ret |= HalAudSetMux(E_AUD_MUX_SDM_DINR, 0);
                if (ret != AIO_OK)
                {
                    ERRMSG("Func:%s, Line:%d eAoCh %d nAttachCount %d Fail !\n", __FUNCTION__, __LINE__, eAoCh,
                           nAttachCount);
                    goto FAIL;
                }
            }

            break;

        default:

            break;
    }

    return AIO_OK;

FAIL:

    return AIO_NG;
}

int HalAudAoDmaSetRefClk(CHIP_AIO_DMA_e eAioDma, BOOL bIsFromI2S)
{
    switch (eAioDma)
    {
        case E_CHIP_AIO_DMA_AO_A:

            if (bIsFromI2S)
            {
            }
            else
            {
            }

            break;

        case E_CHIP_AIO_DMA_AO_B:

            if (bIsFromI2S)
            {
            }
            else
            {
            }

            break;

        default:
            ERRMSG("Func:%s, Line:%d eAioDma %d bIsFromI2S %d Fail !\n", __FUNCTION__, __LINE__, eAioDma, bIsFromI2S);
            goto FAIL;
            break;
    }

    return AIO_OK;

FAIL:

    return AIO_NG;
}
