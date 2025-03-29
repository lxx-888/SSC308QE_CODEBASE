/*
 * mhal_audio.c - Sigmastar
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

// -------------------------------------------------------------------------------
#include "hal_audio_common.h"
#include "hal_audio_sys.h"
#include "hal_audio_config.h"
//#include "hal_audio_reg.h"
#include "hal_audio_types.h"
#include "hal_audio_api.h"

#include "mhal_common.h"
#include "mhal_audio.h"
#include "mhal_audio_datatype.h"

#include "drv_audio.h"
#include "drv_audio_dbg.h"
#include "audio_proc.h"

// -------------------------------------------------------------------------------
#define DMIC_CH_MAX             (4)
#define I2S_RX_CH_MAX           (8)
#define AO_IF_IN_ONE_DMA_CH_MAX (16)
#define AI_DMA_DIRECT_START     (E_MHAL_AI_DMA_DIRECT_A)
#define AO_DMA_DIRECT_START     (E_MHAL_AO_DMA_DIRECT_A)

// -------------------------------------------------------------------------------
typedef struct
{
    MHAL_AI_IF_e eAiIf[E_MHAL_AI_DMA_CH_SLOT_TOTAL];
    MS_U8        nAttachedChNum;
    MS_BOOL      bIsAttached;
    MS_U32       nSampleRate;

} MHAL_AI_Dma_Status_t;

typedef struct
{
    MHAL_AO_IF_e eAoIf[E_MHAL_AO_DMA_CH_SLOT_TOTAL];
    MS_U8        nAttachedChNum;
    MS_BOOL      bIsAttached;
    MS_U32       nSampleRate;

} MHAL_AO_Dma_Status_t;

typedef struct
{
    MS_U8 anDmicChAttachedCount[DMIC_CH_MAX];

} MHAL_AI_DigMic_Status_t;

typedef struct
{
    MS_U8 ncAttachedCount;

} MHAL_AI_If_Ch_Status_t;

typedef struct
{
    MS_U8 ncAttachedCount;

} MHAL_AO_If_Ch_Status_t;

typedef struct
{
    MHAL_AI_IF_e eAiIf;
    MHAL_AO_IF_e eAoIf[2];

} MHAL_Direct_Attach_t;

// -------------------------------------------------------------------------------
// static CamOsMutex_t    g_tAudInitMutex = {{0}};
static volatile MS_S16 g_nInitialed = 0;

static MHAL_AI_Dma_Status_t    g_pAiDmaStatusList[E_MHAL_AI_DMA_TOTAL];
static MHAL_AO_Dma_Status_t    g_pAoDmaStatusList[E_MHAL_AO_DMA_TOTAL];
static MHAL_AI_DigMic_Status_t g_pAiDigMicStatusList[AI_DEV_DMIC_TOTAL];
static MHAL_AO_If_Ch_Status_t  g_pAoIfStatusList[E_AO_CH_TOTAL];
static MHAL_AI_If_Ch_Status_t  g_pAiIfStatusList[E_AI_CH_TOTAL];

// -------------------------------------------------------------------------------
static int _MhalAudioAiAttachCore(AI_DMA_e enAiDma, MS_U8 nAiDmaChSlotNum, AI_IF_e eAiIf);
static int _MhalAudioAoAttachCore(AO_DMA_e enAoDma, MS_U8 nAoDmaChSlotNum, AO_IF_e eAoIf);
static int _MhalAudioAiDettachCore(AI_DMA_e enAiDma, MS_U8 nAiDmaChSlotNum, AI_IF_e eAiIf);
static int _MhalAudioAoDettachCore(AO_DMA_e enAoDma, MS_U8 nAoDmaChSlotNum, AO_IF_e eAoIf);
static int _MhalAudioAiDirectAttachCore(AI_DMA_e enAiDma, MS_U8 nAiDmaChSlotNum, AI_IF_e eAiIf);
static int _MhalAudioAoDirectAttachCore(AO_DMA_e enAoDma, MS_U8 nAoDmaChSlotNum, AO_IF_e eAoIf);
static int _MhalAudioAiDirectDettachCore(AI_DMA_e enAiDma, MS_U8 nAiDmaChSlotNum, AI_IF_e eAiIf);
static int _MhalAudioAoDirectDettachCore(AO_DMA_e enAoDma, MS_U8 nAoDmaChSlotNum, AO_IF_e eAoIf);
static int _MhalAudioDirectAttachCore(MHAL_Direct_Attach_t *pDirectAttach);
static int _MhalAudioDirectDettachCore(MHAL_Direct_Attach_t *pDirectAttach);
static int _MhalAudioAiConfigI2s(AI_DEV_e eAiDev, MHAL_AUDIO_I2sCfg_t *pstI2sConfig);
static int _MhalAudioAoConfigI2s(AO_DEV_e eAoDev, MHAL_AUDIO_I2sCfg_t *pstI2sConfig);
static int _MhalAudioAiIfEnableByStatus(AI_DMA_e enAiDma, AI_IF_e eAiIf, BOOL bEn);
static int _MhalAudioAoIfEnableByStatus(AO_DMA_e enAoDma, AO_IF_e eAoIf, BOOL bEn);
static int _MhalAudioAiIfToDevCh(MHAL_AI_IF_e eAiIf, AI_CH_e *peAiCh1, AI_CH_e *peAiCh2);
static int _MhalAudioAoIfToDevCh(MHAL_AO_IF_e eAoIf, AO_CH_e *peAoCh, MS_U8 *pnGetNum);
static int _MhalAudioAiCheckIfDoSettingByStatus(MHAL_AI_IF_e eAiIf, BOOL bEn, BOOL *pbIsDoSetting);
static int _MhalAudioAoCheckIfDoSettingByStatus(MHAL_AO_IF_e eAoIf, BOOL bEn, BOOL *pbIsDoSetting);
static int _MhalAudioAiEchoCfgByDma(MHAL_AI_Dma_e enAiDma);
static int _MhalAudioAiDmicSampleRateCfgByDma(MHAL_AI_Dma_e enAiDma, BOOL *pbIsDigMicNeedUpdate);
static int _MhalAudioAiDmicChNumCfg(BOOL *pbIsDigMicNeedUpdate);
static int _MhalAudioAiDmicStatusUpdate(AI_IF_e eAiIf, BOOL bEn, BOOL *pbIsDigMicNeedUpdate);
static int _MhalAudioAiIfStatusUpdateAndAction(AI_IF_e eAiIf, BOOL bIsAttach);
static int _MhalAudioAoIfStatusUpdateAndAction(AO_IF_e eAoIf, BOOL bIsAttach);
static int _MhalAudioAiDetachDma(MHAL_AI_Dma_e enAiDma);
static int _MhalAudioAoDetachDma(MHAL_AO_Dma_e enAoDma);

// -------------------------------------------------------------------------------
static int _MhalAudioAiAttachCore(AI_DMA_e enAiDma, MS_U8 nAiDmaChSlotNum, AI_IF_e eAiIf)
{
    int            ret     = AIO_OK;
    U8             nDmaCh1 = 0, nDmaCh2 = 0;
    CHIP_AIO_DMA_e eAioDma = 0;
    int            line    = 0;

    eAioDma = CHIP_AIO_DMA_IDX_BY_AI_DMA(enAiDma);
    if (!CHIP_AIO_DMA_IDX_VALID(eAioDma))
    {
        line = __LINE__;
        goto FAIL;
    }

    //
    nDmaCh1 = ((nAiDmaChSlotNum * AI_DMA_CH_NUM_PER_SLOT) + 0);
    nDmaCh2 = ((nAiDmaChSlotNum * AI_DMA_CH_NUM_PER_SLOT) + 1);

    //
    switch (eAiIf)
    {
        case E_MHAL_AI_IF_ADC_A_0_B_0:
            ret |= DrvAudSetAiPath(eAioDma, nDmaCh1, E_AI_CH_ADC_A_0);
            ret |= DrvAudSetAiPath(eAioDma, nDmaCh2, E_AI_CH_ADC_B_0);
            break;

        case E_MHAL_AI_IF_ADC_C_0_D_0:
            ret |= DrvAudSetAiPath(eAioDma, nDmaCh1, E_AI_CH_ADC_C_0);
            ret |= DrvAudSetAiPath(eAioDma, nDmaCh2, E_AI_CH_ADC_D_0);
            break;

        case E_MHAL_AI_IF_DMIC_A_0_1:
            ret |= DrvAudSetAiPath(eAioDma, nDmaCh1, E_AI_CH_DMIC_A_0);
            ret |= DrvAudSetAiPath(eAioDma, nDmaCh2, E_AI_CH_DMIC_A_1);
            break;

        case E_MHAL_AI_IF_DMIC_A_2_3:
            ret |= DrvAudSetAiPath(eAioDma, nDmaCh1, E_AI_CH_DMIC_A_2);
            ret |= DrvAudSetAiPath(eAioDma, nDmaCh2, E_AI_CH_DMIC_A_3);
            break;

        case E_MHAL_AI_IF_HDMI_A_0_1:
            ret |= DrvAudSetAiPath(eAioDma, nDmaCh1, E_AI_CH_HDMI_A_0);
            ret |= DrvAudSetAiPath(eAioDma, nDmaCh2, E_AI_CH_HDMI_A_1);
            break;

        case E_MHAL_AI_IF_ECHO_A_0_1:
            ret |= DrvAudSetAiPath(eAioDma, nDmaCh1, E_AI_CH_ECHO_A_0);
            ret |= DrvAudSetAiPath(eAioDma, nDmaCh2, E_AI_CH_ECHO_A_1);
            break;

        case E_MHAL_AI_IF_I2S_RX_A_0_1:
            ret |= DrvAudSetAiPath(eAioDma, nDmaCh1, E_AI_CH_I2S_RX_A_0);
            ret |= DrvAudSetAiPath(eAioDma, nDmaCh2, E_AI_CH_I2S_RX_A_1);
            break;

        case E_MHAL_AI_IF_I2S_RX_A_2_3:
            ret |= DrvAudSetAiPath(eAioDma, nDmaCh1, E_AI_CH_I2S_RX_A_2);
            ret |= DrvAudSetAiPath(eAioDma, nDmaCh2, E_AI_CH_I2S_RX_A_3);
            break;

        case E_MHAL_AI_IF_I2S_RX_A_4_5:
            ret |= DrvAudSetAiPath(eAioDma, nDmaCh1, E_AI_CH_I2S_RX_A_4);
            ret |= DrvAudSetAiPath(eAioDma, nDmaCh2, E_AI_CH_I2S_RX_A_5);
            break;

        case E_MHAL_AI_IF_I2S_RX_A_6_7:
            ret |= DrvAudSetAiPath(eAioDma, nDmaCh1, E_AI_CH_I2S_RX_A_6);
            ret |= DrvAudSetAiPath(eAioDma, nDmaCh2, E_AI_CH_I2S_RX_A_7);
            break;

        case E_MHAL_AI_IF_I2S_RX_A_8_9:
            ret |= DrvAudSetAiPath(eAioDma, nDmaCh1, E_AI_CH_I2S_RX_A_8);
            ret |= DrvAudSetAiPath(eAioDma, nDmaCh2, E_AI_CH_I2S_RX_A_9);
            break;

        case E_MHAL_AI_IF_I2S_RX_A_10_11:
            ret |= DrvAudSetAiPath(eAioDma, nDmaCh1, E_AI_CH_I2S_RX_A_10);
            ret |= DrvAudSetAiPath(eAioDma, nDmaCh2, E_AI_CH_I2S_RX_A_11);
            break;

        case E_MHAL_AI_IF_I2S_RX_A_12_13:
            ret |= DrvAudSetAiPath(eAioDma, nDmaCh1, E_AI_CH_I2S_RX_A_12);
            ret |= DrvAudSetAiPath(eAioDma, nDmaCh2, E_AI_CH_I2S_RX_A_13);
            break;

        case E_MHAL_AI_IF_I2S_RX_A_14_15:
            ret |= DrvAudSetAiPath(eAioDma, nDmaCh1, E_AI_CH_I2S_RX_A_14);
            ret |= DrvAudSetAiPath(eAioDma, nDmaCh2, E_AI_CH_I2S_RX_A_15);
            break;

        case E_MHAL_AI_IF_I2S_RX_B_0_1:
            ret |= DrvAudSetAiPath(eAioDma, nDmaCh1, E_AI_CH_I2S_RX_B_0);
            ret |= DrvAudSetAiPath(eAioDma, nDmaCh2, E_AI_CH_I2S_RX_B_1);
            break;

        case E_MHAL_AI_IF_I2S_RX_B_2_3:
            ret |= DrvAudSetAiPath(eAioDma, nDmaCh1, E_AI_CH_I2S_RX_B_2);
            ret |= DrvAudSetAiPath(eAioDma, nDmaCh2, E_AI_CH_I2S_RX_B_3);
            break;

        case E_MHAL_AI_IF_I2S_RX_B_4_5:
            ret |= DrvAudSetAiPath(eAioDma, nDmaCh1, E_AI_CH_I2S_RX_B_4);
            ret |= DrvAudSetAiPath(eAioDma, nDmaCh2, E_AI_CH_I2S_RX_B_5);
            break;

        case E_MHAL_AI_IF_I2S_RX_B_6_7:
            ret |= DrvAudSetAiPath(eAioDma, nDmaCh1, E_AI_CH_I2S_RX_B_6);
            ret |= DrvAudSetAiPath(eAioDma, nDmaCh2, E_AI_CH_I2S_RX_B_7);
            break;

        case E_MHAL_AI_IF_I2S_RX_B_8_9:
            ret |= DrvAudSetAiPath(eAioDma, nDmaCh1, E_AI_CH_I2S_RX_B_8);
            ret |= DrvAudSetAiPath(eAioDma, nDmaCh2, E_AI_CH_I2S_RX_B_9);
            break;

        case E_MHAL_AI_IF_I2S_RX_B_10_11:
            ret |= DrvAudSetAiPath(eAioDma, nDmaCh1, E_AI_CH_I2S_RX_B_10);
            ret |= DrvAudSetAiPath(eAioDma, nDmaCh2, E_AI_CH_I2S_RX_B_11);
            break;

        case E_MHAL_AI_IF_I2S_RX_B_12_13:
            ret |= DrvAudSetAiPath(eAioDma, nDmaCh1, E_AI_CH_I2S_RX_B_12);
            ret |= DrvAudSetAiPath(eAioDma, nDmaCh2, E_AI_CH_I2S_RX_B_13);
            break;

        case E_MHAL_AI_IF_I2S_RX_B_14_15:
            ret |= DrvAudSetAiPath(eAioDma, nDmaCh1, E_AI_CH_I2S_RX_B_14);
            ret |= DrvAudSetAiPath(eAioDma, nDmaCh2, E_AI_CH_I2S_RX_B_15);
            break;

        case E_MHAL_AI_IF_I2S_RX_C_0_1:
            ret |= DrvAudSetAiPath(eAioDma, nDmaCh1, E_AI_CH_I2S_RX_C_0);
            ret |= DrvAudSetAiPath(eAioDma, nDmaCh2, E_AI_CH_I2S_RX_C_1);
            break;

        case E_MHAL_AI_IF_I2S_RX_C_2_3:
            ret |= DrvAudSetAiPath(eAioDma, nDmaCh1, E_AI_CH_I2S_RX_C_2);
            ret |= DrvAudSetAiPath(eAioDma, nDmaCh2, E_AI_CH_I2S_RX_C_3);
            break;

        case E_MHAL_AI_IF_I2S_RX_C_4_5:
            ret |= DrvAudSetAiPath(eAioDma, nDmaCh1, E_AI_CH_I2S_RX_C_4);
            ret |= DrvAudSetAiPath(eAioDma, nDmaCh2, E_AI_CH_I2S_RX_C_5);
            break;

        case E_MHAL_AI_IF_I2S_RX_C_6_7:
            ret |= DrvAudSetAiPath(eAioDma, nDmaCh1, E_AI_CH_I2S_RX_C_6);
            ret |= DrvAudSetAiPath(eAioDma, nDmaCh2, E_AI_CH_I2S_RX_C_7);
            break;

        case E_MHAL_AI_IF_I2S_RX_C_8_9:
            ret |= DrvAudSetAiPath(eAioDma, nDmaCh1, E_AI_CH_I2S_RX_C_8);
            ret |= DrvAudSetAiPath(eAioDma, nDmaCh2, E_AI_CH_I2S_RX_C_9);
            break;

        case E_MHAL_AI_IF_I2S_RX_C_10_11:
            ret |= DrvAudSetAiPath(eAioDma, nDmaCh1, E_AI_CH_I2S_RX_C_10);
            ret |= DrvAudSetAiPath(eAioDma, nDmaCh2, E_AI_CH_I2S_RX_C_11);
            break;

        case E_MHAL_AI_IF_I2S_RX_C_12_13:
            ret |= DrvAudSetAiPath(eAioDma, nDmaCh1, E_AI_CH_I2S_RX_C_12);
            ret |= DrvAudSetAiPath(eAioDma, nDmaCh2, E_AI_CH_I2S_RX_C_13);
            break;

        case E_MHAL_AI_IF_I2S_RX_C_14_15:
            ret |= DrvAudSetAiPath(eAioDma, nDmaCh1, E_AI_CH_I2S_RX_C_14);
            ret |= DrvAudSetAiPath(eAioDma, nDmaCh2, E_AI_CH_I2S_RX_C_15);
            break;

        case E_MHAL_AI_IF_I2S_RX_D_0_1:
            ret |= DrvAudSetAiPath(eAioDma, nDmaCh1, E_AI_CH_I2S_RX_D_0);
            ret |= DrvAudSetAiPath(eAioDma, nDmaCh2, E_AI_CH_I2S_RX_D_1);
            break;

        case E_MHAL_AI_IF_I2S_RX_D_2_3:
            ret |= DrvAudSetAiPath(eAioDma, nDmaCh1, E_AI_CH_I2S_RX_D_2);
            ret |= DrvAudSetAiPath(eAioDma, nDmaCh2, E_AI_CH_I2S_RX_D_3);
            break;

        case E_MHAL_AI_IF_I2S_RX_D_4_5:
            ret |= DrvAudSetAiPath(eAioDma, nDmaCh1, E_AI_CH_I2S_RX_D_4);
            ret |= DrvAudSetAiPath(eAioDma, nDmaCh2, E_AI_CH_I2S_RX_D_5);
            break;

        case E_MHAL_AI_IF_I2S_RX_D_6_7:
            ret |= DrvAudSetAiPath(eAioDma, nDmaCh1, E_AI_CH_I2S_RX_D_6);
            ret |= DrvAudSetAiPath(eAioDma, nDmaCh2, E_AI_CH_I2S_RX_D_7);
            break;

        case E_MHAL_AI_IF_I2S_RX_D_8_9:
            ret |= DrvAudSetAiPath(eAioDma, nDmaCh1, E_AI_CH_I2S_RX_D_8);
            ret |= DrvAudSetAiPath(eAioDma, nDmaCh2, E_AI_CH_I2S_RX_D_9);
            break;

        case E_MHAL_AI_IF_I2S_RX_D_10_11:
            ret |= DrvAudSetAiPath(eAioDma, nDmaCh1, E_AI_CH_I2S_RX_D_10);
            ret |= DrvAudSetAiPath(eAioDma, nDmaCh2, E_AI_CH_I2S_RX_D_11);
            break;

        case E_MHAL_AI_IF_I2S_RX_D_12_13:
            ret |= DrvAudSetAiPath(eAioDma, nDmaCh1, E_AI_CH_I2S_RX_D_12);
            ret |= DrvAudSetAiPath(eAioDma, nDmaCh2, E_AI_CH_I2S_RX_D_13);
            break;

        case E_MHAL_AI_IF_I2S_RX_D_14_15:
            ret |= DrvAudSetAiPath(eAioDma, nDmaCh1, E_AI_CH_I2S_RX_D_14);
            ret |= DrvAudSetAiPath(eAioDma, nDmaCh2, E_AI_CH_I2S_RX_D_15);
            break;

        default:
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
    printf("%s[%d] enAiDma[%d] nAiDmaChSlotNum[%d] eAiIf[%d]error\n", __FUNCTION__, line, enAiDma, nAiDmaChSlotNum,
           eAiIf);
    return AIO_NG;
}

static int _MhalAudioAoAttachCore(AO_DMA_e enAoDma, MS_U8 nAoDmaChSlotNum, AO_IF_e eAoIf)
{
    int            ret     = AIO_OK;
    U8             nDmaCh  = 0;
    CHIP_AIO_DMA_e eAioDma = 0;
    int            line    = 0;

    eAioDma = CHIP_AIO_DMA_IDX_BY_AO_DMA(enAoDma);
    if (!CHIP_AIO_DMA_IDX_VALID(eAioDma))
    {
        line = __LINE__;
        goto FAIL;
    }

    //
    nDmaCh = nAoDmaChSlotNum;

    //
    if (eAoIf & E_MHAL_AO_IF_DAC_A_0)
    {
        ret |= DrvAudSetAoPath(eAioDma, nDmaCh, E_AO_CH_DAC_A_0);
    }

    if (eAoIf & E_MHAL_AO_IF_DAC_B_0)
    {
        ret |= DrvAudSetAoPath(eAioDma, nDmaCh, E_AO_CH_DAC_B_0);
    }

    if (eAoIf & E_MHAL_AO_IF_DAC_C_0)
    {
        ret |= DrvAudSetAoPath(eAioDma, nDmaCh, E_AO_CH_DAC_C_0);
    }

    if (eAoIf & E_MHAL_AO_IF_DAC_D_0)
    {
        ret |= DrvAudSetAoPath(eAioDma, nDmaCh, E_AO_CH_DAC_D_0);
    }

    if (eAoIf & E_MHAL_AO_IF_HDMI_A_0)
    {
        ret |= DrvAudSetAoPath(eAioDma, nDmaCh, E_AO_CH_HDMI_A_0);
    }

    if (eAoIf & E_MHAL_AO_IF_HDMI_A_1)
    {
        ret |= DrvAudSetAoPath(eAioDma, nDmaCh, E_AO_CH_HDMI_A_1);
    }

    if (eAoIf & E_MHAL_AO_IF_ECHO_A_0)
    {
        ret |= DrvAudSetAoPath(eAioDma, nDmaCh, E_AO_CH_ECHO_A_0);
    }

    if (eAoIf & E_MHAL_AO_IF_ECHO_A_1)
    {
        ret |= DrvAudSetAoPath(eAioDma, nDmaCh, E_AO_CH_ECHO_A_1);
    }

    if (eAoIf & E_MHAL_AO_IF_I2S_TX_A_0)
    {
        ret |= DrvAudSetAoPath(eAioDma, nDmaCh, E_AO_CH_I2S_TX_A_0);
    }

    if (eAoIf & E_MHAL_AO_IF_I2S_TX_A_1)
    {
        ret |= DrvAudSetAoPath(eAioDma, nDmaCh, E_AO_CH_I2S_TX_A_1);
    }

    if (eAoIf & E_MHAL_AO_IF_I2S_TX_B_0)
    {
        ret |= DrvAudSetAoPath(eAioDma, nDmaCh, E_AO_CH_I2S_TX_B_0);
    }

    if (eAoIf & E_MHAL_AO_IF_I2S_TX_B_1)
    {
        ret |= DrvAudSetAoPath(eAioDma, nDmaCh, E_AO_CH_I2S_TX_B_1);
    }

    //
    if (ret != AIO_OK)
    {
        line = __LINE__;
        goto FAIL;
    }

    return AIO_OK;

FAIL:
    printf("%s[%d] enAoDma[%d] nAoDmaChSlotNum[%d] eAoIf[%d]error\n", __FUNCTION__, line, enAoDma, nAoDmaChSlotNum,
           eAoIf);
    return AIO_NG;
}

static int _MhalAudioAiDettachCore(AI_DMA_e enAiDma, MS_U8 nAiDmaChSlotNum, AI_IF_e eAiIf)
{
    int ret    = AIO_OK;
    U8  nDmaCh = 0;

    //
    nDmaCh = nAiDmaChSlotNum;

    //
    if (ret != AIO_OK)
    {
        goto FAIL;
    }

    return AIO_OK;

FAIL:

    return AIO_NG;
}

static int _MhalAudioAoDettachCore(AO_DMA_e enAoDma, MS_U8 nAoDmaChSlotNum, AO_IF_e eAoIf)
{
    int ret    = AIO_OK;
    U8  nDmaCh = 0;

    //
    nDmaCh = nAoDmaChSlotNum;

    //
    if (eAoIf & E_MHAL_AO_IF_DAC_A_0)
    {
        return DrvAudDetachAo(E_AO_CH_DAC_A_0);
    }

    if (eAoIf & E_MHAL_AO_IF_DAC_B_0)
    {
        return DrvAudDetachAo(E_AO_CH_DAC_B_0);
    }

    if (eAoIf & E_MHAL_AO_IF_DAC_C_0)
    {
        // ret |= DrvAudSetAoPath(eAioDma, nDmaCh, E_AO_CH_DAC_C_0);
    }

    if (eAoIf & E_MHAL_AO_IF_DAC_D_0)
    {
        // ret |= DrvAudSetAoPath(eAioDma, nDmaCh, E_AO_CH_DAC_D_0);
    }

    if (eAoIf & E_MHAL_AO_IF_ECHO_A_0)
    {
        return DrvAudDetachAo(E_AO_CH_ECHO_A_0);
    }

    if (eAoIf & E_MHAL_AO_IF_ECHO_A_1)
    {
        return DrvAudDetachAo(E_AO_CH_ECHO_A_1);
    }

    if (eAoIf & E_MHAL_AO_IF_I2S_TX_A_0)
    {
        // ret |= DrvAudSetAoPath(eAioDma, nDmaCh, E_AO_CH_I2S_TX_A_0);
    }

    if (eAoIf & E_MHAL_AO_IF_I2S_TX_A_1)
    {
        // ret |= DrvAudSetAoPath(eAioDma, nDmaCh, E_AO_CH_I2S_TX_A_1);
    }

    if (eAoIf & E_MHAL_AO_IF_I2S_TX_B_0)
    {
        // ret |= DrvAudSetAoPath(eAioDma, nDmaCh, E_AO_CH_I2S_TX_B_0);
    }

    if (eAoIf & E_MHAL_AO_IF_I2S_TX_B_1)
    {
        // ret |= DrvAudSetAoPath(eAioDma, nDmaCh, E_AO_CH_I2S_TX_B_1);
    }

    //
    if (ret != AIO_OK)
    {
        goto FAIL;
    }

    return AIO_OK;

FAIL:

    return AIO_NG;
}

static int _MhalAudioAiDirectAttachCore(AI_DMA_e enAiDma, MS_U8 nAiDmaChSlotNum, AI_IF_e eAiIf)
{
    int                  ret     = AIO_OK;
    AO_DMA_e             enAoDma = 0;
    U8                   nDmaCh1 = 0, nDmaCh2 = 0;
    AO_IF_e              eAoIf1 = 0, eAoIf2 = 0;
    MHAL_Direct_Attach_t pDirectAttach;
    int                  line = 0;

    if (enAiDma < AI_DMA_DIRECT_START)
    {
        line = __LINE__;
        goto FAIL;
    }

    // Direct DMA is only 2 channels
    if (nAiDmaChSlotNum != 0)
    {
        line = __LINE__;
        goto FAIL;
    }

    nDmaCh1 = 0;
    nDmaCh2 = 1;

    //
    switch (enAiDma)
    {
        case E_MHAL_AI_DMA_DIRECT_A:
            enAoDma = E_MHAL_AO_DMA_DIRECT_A;
            break;

        case E_MHAL_AI_DMA_DIRECT_B:
            enAoDma = E_MHAL_AO_DMA_DIRECT_B;
            break;

        default:
            line = __LINE__;
            goto FAIL;
            break;
    }

    eAoIf1 = g_pAoDmaStatusList[enAoDma].eAoIf[nDmaCh1];
    eAoIf2 = g_pAoDmaStatusList[enAoDma].eAoIf[nDmaCh2];

    if (eAiIf != E_MHAL_AI_IF_NONE)
    {
        if ((eAoIf1 != E_MHAL_AO_IF_NONE) && (eAoIf2 != E_MHAL_AO_IF_NONE))
        {
            pDirectAttach.eAiIf    = eAiIf;
            pDirectAttach.eAoIf[0] = eAoIf1;
            pDirectAttach.eAoIf[1] = eAoIf2;

            //
            ret |= _MhalAudioDirectAttachCore(&pDirectAttach);
            if (ret != AIO_OK)
            {
                line = __LINE__;
                goto FAIL;
            }
        }
    }

    return AIO_OK;

FAIL:
    printf("%s[%d] enAiDma[%d] nAiDmaChSlotNum[%d] eAiIf[%d]error\n", __FUNCTION__, line, enAiDma, nAiDmaChSlotNum,
           eAiIf);
    return AIO_NG;
}

static int _MhalAudioAoDirectAttachCore(AO_DMA_e enAoDma, MS_U8 nAoDmaChSlotNum, AO_IF_e eAoIf)
{
    int                  ret     = AIO_OK;
    AI_DMA_e             enAiDma = 0;
    U8                   nDmaCh1 = 0, nDmaCh2 = 0;
    AI_IF_e              eAiIf  = 0;
    AO_IF_e              eAoIf1 = 0, eAoIf2 = 0;
    MHAL_Direct_Attach_t pDirectAttach;
    int                  line = 0;

    if (enAoDma < AO_DMA_DIRECT_START)
    {
        line = __LINE__;
        goto FAIL;
    }

    // Direct DMA is only 2 channels
    if (nAoDmaChSlotNum >= 2)
    {
        line = __LINE__;
        goto FAIL;
    }

    nDmaCh1 = 0;
    nDmaCh2 = 1;

    //
    switch (enAoDma)
    {
        case E_MHAL_AO_DMA_DIRECT_A:
            enAiDma = E_MHAL_AI_DMA_DIRECT_A;
            break;

        case E_MHAL_AO_DMA_DIRECT_B:
            enAiDma = E_MHAL_AI_DMA_DIRECT_B;
            break;

        default:
            line = __LINE__;
            goto FAIL;
            break;
    }

    //
    if (nAoDmaChSlotNum == 0)
    {
        eAoIf1 = eAoIf;
        eAoIf2 = g_pAoDmaStatusList[enAoDma].eAoIf[nDmaCh2];
    }
    else
    {
        eAoIf1 = g_pAoDmaStatusList[enAoDma].eAoIf[nDmaCh1];
        eAoIf2 = eAoIf;
    }

    eAiIf = g_pAiDmaStatusList[enAiDma].eAiIf[0];

    if (eAiIf != E_MHAL_AI_IF_NONE)
    {
        if ((eAoIf1 != E_MHAL_AO_IF_NONE) && (eAoIf2 != E_MHAL_AO_IF_NONE))
        {
            pDirectAttach.eAiIf    = eAiIf;
            pDirectAttach.eAoIf[0] = eAoIf1;
            pDirectAttach.eAoIf[1] = eAoIf2;

            //
            ret |= _MhalAudioDirectAttachCore(&pDirectAttach);
            if (ret != AIO_OK)
            {
                line = __LINE__;
                goto FAIL;
            }
        }
    }

    return AIO_OK;

FAIL:
    printf("%s[%d] enAoDma[%d] nAoDmaChSlotNum[%d] eAoIf[%d]error\n", __FUNCTION__, line, enAoDma, nAoDmaChSlotNum,
           eAoIf);
    return AIO_NG;
}

static int _MhalAudioAiDirectDettachCore(AI_DMA_e enAiDma, MS_U8 nAiDmaChSlotNum, AI_IF_e eAiIf)
{
    int                  ret     = AIO_OK;
    AO_DMA_e             enAoDma = 0;
    U8                   nDmaCh1 = 0, nDmaCh2 = 0;
    AO_IF_e              eAoIf1 = 0, eAoIf2 = 0;
    MHAL_Direct_Attach_t pDirectAttach;
    int                  line = 0;

    if (enAiDma < AI_DMA_DIRECT_START)
    {
        line = __LINE__;
        goto FAIL;
    }

    // Direct DMA is only 2 channels
    if (nAiDmaChSlotNum != 0)
    {
        line = __LINE__;
        goto FAIL;
    }

    nDmaCh1 = 0;
    nDmaCh2 = 1;

    //
    switch (enAiDma)
    {
        case E_MHAL_AI_DMA_DIRECT_A:
            enAoDma = E_MHAL_AO_DMA_DIRECT_A;
            break;

        case E_MHAL_AI_DMA_DIRECT_B:
            enAoDma = E_MHAL_AO_DMA_DIRECT_B;
            break;

        default:
            line = __LINE__;
            goto FAIL;
            break;
    }

    eAoIf1 = g_pAoDmaStatusList[enAoDma].eAoIf[nDmaCh1];
    eAoIf2 = g_pAoDmaStatusList[enAoDma].eAoIf[nDmaCh2];

    //
    pDirectAttach.eAiIf    = eAiIf;
    pDirectAttach.eAoIf[0] = eAoIf1;
    pDirectAttach.eAoIf[1] = eAoIf2;

    //
    ret |= _MhalAudioDirectDettachCore(&pDirectAttach);
    if (ret != AIO_OK)
    {
        line = __LINE__;
        goto FAIL;
    }

    return AIO_OK;

FAIL:
    printf("%s[%d] enAiDma[%d] nAiDmaChSlotNum[%d] eAiIf[%d]error\n", __FUNCTION__, line, enAiDma, nAiDmaChSlotNum,
           eAiIf);
    return AIO_NG;
}

static int _MhalAudioAoDirectDettachCore(AO_DMA_e enAoDma, MS_U8 nAoDmaChSlotNum, AO_IF_e eAoIf)
{
    int                  ret     = AIO_OK;
    AI_DMA_e             enAiDma = 0;
    U8                   nDmaCh1 = 0, nDmaCh2 = 0;
    AI_IF_e              eAiIf  = 0;
    AO_IF_e              eAoIf1 = 0, eAoIf2 = 0;
    MHAL_Direct_Attach_t pDirectAttach;
    int                  line = 0;

    if (enAoDma < AO_DMA_DIRECT_START)
    {
        line = __LINE__;
        goto FAIL;
    }

    // Direct DMA is only 2 channels
    if (nAoDmaChSlotNum >= 2)
    {
        line = __LINE__;
        goto FAIL;
    }

    nDmaCh1 = 0;
    nDmaCh2 = 1;

    //
    switch (enAoDma)
    {
        case E_MHAL_AO_DMA_DIRECT_A:
            enAiDma = E_MHAL_AI_DMA_DIRECT_A;
            break;

        case E_MHAL_AO_DMA_DIRECT_B:
            enAiDma = E_MHAL_AI_DMA_DIRECT_B;
            break;

        default:
            line = __LINE__;
            goto FAIL;
            break;
    }

    //
    if (nAoDmaChSlotNum == 0)
    {
        eAoIf1 = eAoIf;
        eAoIf2 = g_pAoDmaStatusList[enAoDma].eAoIf[nDmaCh2];
    }
    else
    {
        eAoIf1 = g_pAoDmaStatusList[enAoDma].eAoIf[nDmaCh1];
        eAoIf2 = eAoIf;
    }

    eAiIf = g_pAiDmaStatusList[enAiDma].eAiIf[0];

    //
    pDirectAttach.eAiIf    = eAiIf;
    pDirectAttach.eAoIf[0] = eAoIf1;
    pDirectAttach.eAoIf[1] = eAoIf2;

    //
    ret |= _MhalAudioDirectDettachCore(&pDirectAttach);
    if (ret != AIO_OK)
    {
        line = __LINE__;
        goto FAIL;
    }

    return AIO_OK;

FAIL:
    printf("%s[%d] enAoDma[%d] nAoDmaChSlotNum[%d] eAoIf[%d]error\n", __FUNCTION__, line, enAoDma, nAoDmaChSlotNum,
           eAoIf);
    return AIO_NG;
}

static int _MhalAudioDirectAttachCore(MHAL_Direct_Attach_t *pDirectAttach)
{
    int     ret = AIO_OK;
    int     i   = 0;
    AI_CH_e eAiCh1, eAiCh2;
    AO_CH_e aeAoCh1[AO_IF_IN_ONE_DMA_CH_MAX], aeAoCh2[AO_IF_IN_ONE_DMA_CH_MAX];
    MS_U8   nGetNum1 = 0, nGetNum2 = 0;
    int     line = 0;

    //
    ret |= _MhalAudioAiIfToDevCh(pDirectAttach->eAiIf, &eAiCh1, &eAiCh2);
    ret |= _MhalAudioAoIfToDevCh(pDirectAttach->eAoIf[0], aeAoCh1, &nGetNum1);
    ret |= _MhalAudioAoIfToDevCh(pDirectAttach->eAoIf[1], aeAoCh2, &nGetNum2);
    if (ret != AIO_OK)
    {
        line = __LINE__;
        goto FAIL;
    }

    //
    for (i = 0; i < nGetNum1; i++)
    {
        //
        ret |= DrvAudSetDirectPath(eAiCh1, aeAoCh1[i]);
        if (ret != AIO_OK)
        {
            line = __LINE__;
            goto FAIL;
        }
    }

    //
    for (i = 0; i < nGetNum2; i++)
    {
        //
        ret |= DrvAudSetDirectPath(eAiCh2, aeAoCh2[i]);
        if (ret != AIO_OK)
        {
            line = __LINE__;
            goto FAIL;
        }
    }

    return AIO_OK;

FAIL:
    printf("%s[%d] error\n", __FUNCTION__, line);
    return AIO_NG;
}

static int _MhalAudioDirectDettachCore(MHAL_Direct_Attach_t *pDirectAttach)
{
    int ret = AIO_OK;
    int i   = 0;
    // AI_CH_e eAiCh1, eAiCh2;
    AO_CH_e aeAoCh1[AO_IF_IN_ONE_DMA_CH_MAX], aeAoCh2[AO_IF_IN_ONE_DMA_CH_MAX];
    MS_U8   nGetNum1 = 0, nGetNum2 = 0;
    int     line = 0;

    //
    // ret |= _MhalAudioAiIfToDevCh(pDirectAttach->eAiIf, &eAiCh1, &eAiCh2);
    ret |= _MhalAudioAoIfToDevCh(pDirectAttach->eAoIf[0], aeAoCh1, &nGetNum1);
    ret |= _MhalAudioAoIfToDevCh(pDirectAttach->eAoIf[1], aeAoCh2, &nGetNum2);
    if (ret != AIO_OK)
    {
        line = __LINE__;
        goto FAIL;
    }

    //
    for (i = 0; i < nGetNum1; i++)
    {
        //
        // ret |= DrvAud UnSet DirectPath(eAiCh1, aeAoCh1[i]);
        if (ret != AIO_OK)
        {
            line = __LINE__;
            goto FAIL;
        }
    }

    //
    for (i = 0; i < nGetNum2; i++)
    {
        //
        // ret |= DrvAud UnSet DirectPath(eAiCh2, aeAoCh2[i]);
        if (ret != AIO_OK)
        {
            line = __LINE__;
            goto FAIL;
        }
    }

    return AIO_OK;

FAIL:
    printf("%s[%d] error\n", __FUNCTION__, line);
    return AIO_NG;
}

static int _MhalAudioAiConfigI2s(AI_DEV_e eAiDev, MHAL_AUDIO_I2sCfg_t *pstI2sConfig)
{
    int            ret = AIO_OK;
    AudI2sCfg_t    tI2sCfg;
    CHIP_AIO_I2S_e eI2s;
    int            line = 0;

    //
    if (!CHIP_AI_I2S_RX_VALID(CHIP_AI_I2S_RX_IDX_BY_DEV(eAiDev)))
    {
        line = __LINE__;
        goto FAIL;
    }

    eI2s = CHIP_AIO_I2S_IDX_BY_AI_DEV_I2S_RX(eAiDev);
    if (!CHIP_AIO_I2S_IDX_VALID(eI2s))
    {
        line = __LINE__;
        goto FAIL;
    }

    //
    switch (pstI2sConfig->enWidth)
    {
        case E_MHAL_AUDIO_BITWIDTH_16:
            tI2sCfg.enI2sWidth = E_AUD_BITWIDTH_16;
            break;
        case E_MHAL_AUDIO_BITWIDTH_32:
            tI2sCfg.enI2sWidth = E_AUD_BITWIDTH_32;
            break;
        default:
            printf("in %s, bitwidth %d  error\n", __FUNCTION__, pstI2sConfig->enWidth);
            line = __LINE__;
            goto FAIL;
            break;
    }

    switch (pstI2sConfig->enFmt)
    {
        case E_MHAL_AUDIO_I2S_FMT_I2S:
            tI2sCfg.eFormat = E_AUD_I2S_FMT_I2S;
            break;
        case E_MHAL_AUDIO_I2S_FMT_LEFT_JUSTIFY:
            tI2sCfg.eFormat = E_AUD_I2S_FMT_LEFT_JUSTIFY;
            break;
        default:
            printf("in %s, I2S format %d  error\n", __FUNCTION__, pstI2sConfig->enFmt);
            line = __LINE__;
            goto FAIL;
            break;
    }

    switch (pstI2sConfig->en4WireMode)
    {
        case E_MHAL_AUDIO_4WIRE_ON:
            tI2sCfg.eWireMode = E_AUD_I2S_WIRE_4;
            break;
        case E_MHAL_AUDIO_4WIRE_OFF:
            tI2sCfg.eWireMode = E_AUD_I2S_WIRE_6;
            break;
        default:
            printf("in %s, I2S wire mode %d  error\n", __FUNCTION__, pstI2sConfig->en4WireMode);
            line = __LINE__;
            goto FAIL;
            break;
    }

    switch (pstI2sConfig->enMck)
    {
        case E_MHAL_AUDIO_MCK_NULL:
            tI2sCfg.eMck = E_AUD_I2S_MCK_NULL;
            break;
        case E_MHAL_AUDIO_MCK_12_288M:
            tI2sCfg.eMck = E_AUD_I2S_MCK_12_288M;
            break;
        case E_MHAL_AUDIO_MCK_16_384M:
            tI2sCfg.eMck = E_AUD_I2S_MCK_16_384M;
            break;
        case E_MHAL_AUDIO_MCK_18_432M:
            tI2sCfg.eMck = E_AUD_I2S_MCK_18_432M;
            break;
        case E_MHAL_AUDIO_MCK_24_576M:
            tI2sCfg.eMck = E_AUD_I2S_MCK_24_576M;
            break;
        case E_MHAL_AUDIO_MCK_24M:
            tI2sCfg.eMck = E_AUD_I2S_MCK_24M;
            break;
        case E_MHAL_AUDIO_MCK_48M:
            tI2sCfg.eMck = E_AUD_I2S_MCK_48M;
            break;
        default:
            printf("in %s, I2S Mck %d  error\n", __FUNCTION__, pstI2sConfig->enMck);
            line = __LINE__;
            goto FAIL;
            break;
    }

    switch (pstI2sConfig->enRate)
    {
        case E_MHAL_AUDIO_RATE_8K:
            tI2sCfg.eRate = E_AUD_RATE_8K;
            break;
        case E_MHAL_AUDIO_RATE_16K:
            tI2sCfg.eRate = E_AUD_RATE_16K;
            break;
        case E_MHAL_AUDIO_RATE_32K:
            tI2sCfg.eRate = E_AUD_RATE_32K;
            break;
        case E_MHAL_AUDIO_RATE_48K:
            tI2sCfg.eRate = E_AUD_RATE_48K;
            break;
        case E_MHAL_AUDIO_RATE_96K:
            tI2sCfg.eRate = E_AUD_RATE_96K;
            break;
        case E_MHAL_AUDIO_RATE_192K:
            tI2sCfg.eRate = E_AUD_RATE_192K;
            break;
        default:
            printf("in %s, I2S Sample Rate %d  error\n", __FUNCTION__, pstI2sConfig->enRate);
            line = __LINE__;
            goto FAIL;
            break;
    }

    //
    switch (pstI2sConfig->enMode)
    {
        case E_MHAL_AUDIO_MODE_I2S_MASTER:
            tI2sCfg.enI2sMode = E_AUD_I2S_MODE_I2S;
            tI2sCfg.eMsMode   = E_AUD_I2S_MSMODE_MASTER;
            break;
        case E_MHAL_AUDIO_MODE_I2S_SLAVE:
            tI2sCfg.enI2sMode = E_AUD_I2S_MODE_I2S;
            tI2sCfg.eMsMode   = E_AUD_I2S_MSMODE_SLAVE;
            break;
        case E_MHAL_AUDIO_MODE_TDM_MASTER:
            tI2sCfg.enI2sMode = E_AUD_I2S_MODE_TDM;
            tI2sCfg.eMsMode   = E_AUD_I2S_MSMODE_MASTER;
            if (pstI2sConfig->u16Channels != 4 && pstI2sConfig->u16Channels != 8)
            {
                printf("in %s, channel %u  error\n", __FUNCTION__, pstI2sConfig->u16Channels);
                line = __LINE__;
                goto FAIL;
            }
            break;
        case E_MHAL_AUDIO_MODE_TDM_SLAVE:
            tI2sCfg.enI2sMode = E_AUD_I2S_MODE_TDM;
            tI2sCfg.eMsMode   = E_AUD_I2S_MSMODE_SLAVE;
            if (pstI2sConfig->u16Channels != 4 && pstI2sConfig->u16Channels != 8)
            {
                printf("in %s, channel %u  error\n", __FUNCTION__, pstI2sConfig->u16Channels);
                line = __LINE__;
                goto FAIL;
            }
            break;
        default:
            goto FAIL;
            break;
    }

    //
    if ((pstI2sConfig->u16Channels % 2) != 0)
    {
        printf("in %s, pstI2sConfig->u16Channels = %d  error\n", __FUNCTION__, pstI2sConfig->u16Channels);
        line = __LINE__;
        goto FAIL;
    }
    else
    {
        tI2sCfg.nChannelNum = pstI2sConfig->u16Channels;
    }

    //
    ret |= DrvAudConfigI2s(eI2s, &tI2sCfg);

    if (ret != AIO_OK)
    {
        line = __LINE__;
        goto FAIL;
    }

    return AIO_OK;

FAIL:
    printf("%s[%d] eAiDev %d error\n", __FUNCTION__, line, eAiDev);
    return AIO_NG;
}

static int _MhalAudioAoConfigI2s(AO_DEV_e eAoDev, MHAL_AUDIO_I2sCfg_t *pstI2sConfig)
{
    int            ret = AIO_OK;
    AudI2sCfg_t    tI2sCfg;
    CHIP_AIO_I2S_e eI2s;
    int            line = 0;

    //
    if (!CHIP_AO_I2S_TX_VALID(CHIP_AO_I2S_TX_IDX_BY_DEV(eAoDev)))
    {
        line = __LINE__;
        goto FAIL;
    }

    eI2s = CHIP_AIO_I2S_IDX_BY_AO_DEV_I2S_TX(eAoDev);
    if (!CHIP_AIO_I2S_IDX_VALID(eI2s))
    {
        line = __LINE__;
        goto FAIL;
    }

    //
    switch (pstI2sConfig->enWidth)
    {
        case E_MHAL_AUDIO_BITWIDTH_16:
            tI2sCfg.enI2sWidth = E_AUD_BITWIDTH_16;
            break;
        case E_MHAL_AUDIO_BITWIDTH_32:
            tI2sCfg.enI2sWidth = E_AUD_BITWIDTH_32;
            break;
        default:
            printf("in %s, bitwidth %d  error\n", __FUNCTION__, pstI2sConfig->enWidth);
            line = __LINE__;
            goto FAIL;
            break;
    }

    switch (pstI2sConfig->enFmt)
    {
        case E_MHAL_AUDIO_I2S_FMT_I2S:
            tI2sCfg.eFormat = E_AUD_I2S_FMT_I2S;
            break;
        case E_MHAL_AUDIO_I2S_FMT_LEFT_JUSTIFY:
            tI2sCfg.eFormat = E_AUD_I2S_FMT_LEFT_JUSTIFY;
            break;
        default:
            printf("in %s, I2S format %d  error\n", __FUNCTION__, pstI2sConfig->enFmt);
            line = __LINE__;
            goto FAIL;
            break;
    }

    switch (pstI2sConfig->en4WireMode)
    {
        case E_MHAL_AUDIO_4WIRE_ON:
            tI2sCfg.eWireMode = E_AUD_I2S_WIRE_4;
            break;
        case E_MHAL_AUDIO_4WIRE_OFF:
            tI2sCfg.eWireMode = E_AUD_I2S_WIRE_6;
            break;
        default:
            printf("in %s, I2S wire mode %d  error\n", __FUNCTION__, pstI2sConfig->en4WireMode);
            line = __LINE__;
            goto FAIL;
            break;
    }

    switch (pstI2sConfig->enRate)
    {
        case E_MHAL_AUDIO_RATE_8K:
            tI2sCfg.eRate = E_AUD_RATE_8K;
            break;
        case E_MHAL_AUDIO_RATE_16K:
            tI2sCfg.eRate = E_AUD_RATE_16K;
            break;
        case E_MHAL_AUDIO_RATE_32K:
            tI2sCfg.eRate = E_AUD_RATE_32K;
            break;
        case E_MHAL_AUDIO_RATE_44K:
            tI2sCfg.eRate = E_AUD_RATE_44K;
            break;
        case E_MHAL_AUDIO_RATE_48K:
            tI2sCfg.eRate = E_AUD_RATE_48K;
            break;
        case E_MHAL_AUDIO_RATE_96K:
            tI2sCfg.eRate = E_AUD_RATE_96K;
            break;
        case E_MHAL_AUDIO_RATE_192K:
            tI2sCfg.eRate = E_AUD_RATE_192K;
            break;
        default:
            printf("in %s, I2S Sample Rate %d  error\n", __FUNCTION__, pstI2sConfig->enRate);
            line = __LINE__;
            goto FAIL;
            break;
    }

    //
    switch (pstI2sConfig->enMode)
    {
        case E_MHAL_AUDIO_MODE_I2S_MASTER:
            tI2sCfg.enI2sMode = E_AUD_I2S_MODE_I2S;
            tI2sCfg.eMsMode   = E_AUD_I2S_MSMODE_MASTER;
            break;
        case E_MHAL_AUDIO_MODE_I2S_SLAVE:
            tI2sCfg.enI2sMode = E_AUD_I2S_MODE_I2S;
            tI2sCfg.eMsMode   = E_AUD_I2S_MSMODE_SLAVE;
            break;
        case E_MHAL_AUDIO_MODE_TDM_MASTER:
            tI2sCfg.enI2sMode = E_AUD_I2S_MODE_TDM;
            tI2sCfg.eMsMode   = E_AUD_I2S_MSMODE_MASTER;
            if (pstI2sConfig->u16Channels != 4 && pstI2sConfig->u16Channels != 8)
            {
                printf("in %s, channel %u  error\n", __FUNCTION__, pstI2sConfig->u16Channels);
                line = __LINE__;
                goto FAIL;
            }
            break;
        case E_MHAL_AUDIO_MODE_TDM_SLAVE:
            tI2sCfg.enI2sMode = E_AUD_I2S_MODE_TDM;
            tI2sCfg.eMsMode   = E_AUD_I2S_MSMODE_SLAVE;
            if (pstI2sConfig->u16Channels != 4 && pstI2sConfig->u16Channels != 8)
            {
                printf("in %s, channel %u  error\n", __FUNCTION__, pstI2sConfig->u16Channels);
                line = __LINE__;
                goto FAIL;
            }
            break;
        default:
            goto FAIL;
            break;
    }

    //
    if ((pstI2sConfig->u16Channels % 2) != 0)
    {
        printf("in %s, pstI2sConfig->u16Channels = %d  error\n", __FUNCTION__, pstI2sConfig->u16Channels);
        line = __LINE__;
        goto FAIL;
    }
    else
    {
        tI2sCfg.nChannelNum = pstI2sConfig->u16Channels;
    }

    //
    switch (pstI2sConfig->enMck)
    {
        case E_MHAL_AUDIO_MCK_NULL:
            tI2sCfg.eMck = E_AUD_I2S_MCK_NULL;
            break;
        case E_MHAL_AUDIO_MCK_12_288M:
            tI2sCfg.eMck = E_AUD_I2S_MCK_12_288M;
            break;
        case E_MHAL_AUDIO_MCK_16_384M:
            tI2sCfg.eMck = E_AUD_I2S_MCK_16_384M;
            break;
        case E_MHAL_AUDIO_MCK_18_432M:
            tI2sCfg.eMck = E_AUD_I2S_MCK_18_432M;
            break;
        case E_MHAL_AUDIO_MCK_24_576M:
            tI2sCfg.eMck = E_AUD_I2S_MCK_24_576M;
            break;
        case E_MHAL_AUDIO_MCK_24M:
            tI2sCfg.eMck = E_AUD_I2S_MCK_24M;
            break;
        case E_MHAL_AUDIO_MCK_48M:
            tI2sCfg.eMck = E_AUD_I2S_MCK_48M;
            break;
        default:
            printf("in %s, I2S Mck %d  error\n", __FUNCTION__, pstI2sConfig->enMck);
            line = __LINE__;
            goto FAIL;
            break;
    }

    //
    ret |= DrvAudConfigI2s(eI2s, &tI2sCfg);

    if (ret != AIO_OK)
    {
        line = __LINE__;
        goto FAIL;
    }

    return AIO_OK;

FAIL:
    printf("%s[%d] eAoDev %d error\n", __FUNCTION__, line, eAoDev);
    return AIO_NG;
}

static int _MhalAudioAiIfEnableByStatus(AI_DMA_e enAiDma, AI_IF_e eAiIf, BOOL bEn)
{
    int     ret          = AIO_OK;
    int     v            = 0;
    AI_IF_e aiIf         = 0;
    BOOL    bIsDoSetting = FALSE;
    int     line         = 0;

    aiIf = eAiIf;

    //
    switch (aiIf)
    {
        case E_MHAL_AI_IF_ADC_A_0_B_0:

            ret |= _MhalAudioAiCheckIfDoSettingByStatus(aiIf, bEn, &bIsDoSetting);
            if (ret != AIO_OK)
            {
                line = __LINE__;
                goto FAIL;
            }

            if (bIsDoSetting == TRUE)
            {
                v = CHIP_AIO_ATOP_IDX_BY_AI_DEV_ADC(E_AI_DEV_ADC_A);
                if (!CHIP_AIO_ATOP_IDX_VALID(v))
                {
                    line = __LINE__;
                    goto FAIL;
                }

                ret |= DrvAudEnableAtop((CHIP_AIO_ATOP_e)v, bEn);
                if (ret != AIO_OK)
                {
                    line = __LINE__;
                    goto FAIL;
                }

                v = CHIP_AIO_ATOP_IDX_BY_AI_DEV_ADC(E_AI_DEV_ADC_B);
                if (!CHIP_AIO_ATOP_IDX_VALID(v))
                {
                    line = __LINE__;
                    goto FAIL;
                }

                ret |= DrvAudEnableAtop((CHIP_AIO_ATOP_e)v, bEn);
                if (ret != AIO_OK)
                {
                    line = __LINE__;
                    goto FAIL;
                }
            }

            break;

        case E_MHAL_AI_IF_ADC_C_0_D_0:

            ret |= _MhalAudioAiCheckIfDoSettingByStatus(aiIf, bEn, &bIsDoSetting);
            if (ret != AIO_OK)
            {
                line = __LINE__;
                goto FAIL;
            }

            if (bIsDoSetting == TRUE)
            {
                v = CHIP_AIO_ATOP_IDX_BY_AI_DEV_ADC(E_AI_DEV_ADC_C);
                if (!CHIP_AIO_ATOP_IDX_VALID(v))
                {
                    line = __LINE__;
                    goto FAIL;
                }

                ret |= DrvAudEnableAtop((CHIP_AIO_ATOP_e)v, bEn);
                if (ret != AIO_OK)
                {
                    line = __LINE__;
                    goto FAIL;
                }

                v = CHIP_AIO_ATOP_IDX_BY_AI_DEV_ADC(E_AI_DEV_ADC_D);
                if (!CHIP_AIO_ATOP_IDX_VALID(v))
                {
                    line = __LINE__;
                    goto FAIL;
                }

                ret |= DrvAudEnableAtop((CHIP_AIO_ATOP_e)v, bEn);
                if (ret != AIO_OK)
                {
                    line = __LINE__;
                    goto FAIL;
                }
            }

            break;

        case E_MHAL_AI_IF_DMIC_A_0_1:
        case E_MHAL_AI_IF_DMIC_A_2_3:

            ret |= _MhalAudioAiCheckIfDoSettingByStatus(aiIf, bEn, &bIsDoSetting);
            if (ret != AIO_OK)
            {
                line = __LINE__;
                goto FAIL;
            }

            if (bIsDoSetting == TRUE)
            {
                v = CHIP_AI_DMIC_IDX_BY_DEV(E_AI_DEV_DMIC_A);
                if (!CHIP_AI_DMIC_VALID(v))
                {
                    line = __LINE__;
                    goto FAIL;
                }

                ret |= DrvAudDigMicEnable((CHIP_AI_DMIC_e)v, bEn);
                if (ret != AIO_OK)
                {
                    line = __LINE__;
                    goto FAIL;
                }
            }

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

            ret |= _MhalAudioAiCheckIfDoSettingByStatus(aiIf, bEn, &bIsDoSetting);
            if (ret != AIO_OK)
            {
                line = __LINE__;
                goto FAIL;
            }

            if (bIsDoSetting == TRUE)
            {
                v = CHIP_AIO_I2S_IDX_BY_AI_DEV_I2S_RX(E_AI_DEV_I2S_RX_A);
                if (!CHIP_AIO_I2S_IDX_VALID(v))
                {
                    line = __LINE__;
                    goto FAIL;
                }

                ret |= DrvAudEnableI2s((CHIP_AIO_I2S_e)v, bEn);
                if (ret != AIO_OK)
                {
                    line = __LINE__;
                    goto FAIL;
                }
            }

            break;

        case E_MHAL_AI_IF_I2S_RX_B_0_1:
        case E_MHAL_AI_IF_I2S_RX_B_2_3:
        case E_MHAL_AI_IF_I2S_RX_B_4_5:
        case E_MHAL_AI_IF_I2S_RX_B_6_7:
        case E_MHAL_AI_IF_I2S_RX_B_8_9:
        case E_MHAL_AI_IF_I2S_RX_B_10_11:
        case E_MHAL_AI_IF_I2S_RX_B_12_13:
        case E_MHAL_AI_IF_I2S_RX_B_14_15:

            ret |= _MhalAudioAiCheckIfDoSettingByStatus(aiIf, bEn, &bIsDoSetting);
            if (ret != AIO_OK)
            {
                line = __LINE__;
                goto FAIL;
            }

            if (bIsDoSetting == TRUE)
            {
                v = CHIP_AIO_I2S_IDX_BY_AI_DEV_I2S_RX(E_AI_DEV_I2S_RX_B);
                if (!CHIP_AIO_I2S_IDX_VALID(v))
                {
                    line = __LINE__;
                    goto FAIL;
                }

                ret |= DrvAudEnableI2s((CHIP_AIO_I2S_e)v, bEn);
                if (ret != AIO_OK)
                {
                    line = __LINE__;
                    goto FAIL;
                }
            }

            break;

        case E_MHAL_AI_IF_I2S_RX_C_0_1:
        case E_MHAL_AI_IF_I2S_RX_C_2_3:
        case E_MHAL_AI_IF_I2S_RX_C_4_5:
        case E_MHAL_AI_IF_I2S_RX_C_6_7:
        case E_MHAL_AI_IF_I2S_RX_C_8_9:
        case E_MHAL_AI_IF_I2S_RX_C_10_11:
        case E_MHAL_AI_IF_I2S_RX_C_12_13:
        case E_MHAL_AI_IF_I2S_RX_C_14_15:

            ret |= _MhalAudioAiCheckIfDoSettingByStatus(aiIf, bEn, &bIsDoSetting);
            if (ret != AIO_OK)
            {
                line = __LINE__;
                goto FAIL;
            }

            if (bIsDoSetting == TRUE)
            {
                v = CHIP_AIO_I2S_IDX_BY_AI_DEV_I2S_RX(E_AI_DEV_I2S_RX_C);
                if (!CHIP_AIO_I2S_IDX_VALID(v))
                {
                    line = __LINE__;
                    goto FAIL;
                }

                ret |= DrvAudEnableI2s((CHIP_AIO_I2S_e)v, bEn);
                if (ret != AIO_OK)
                {
                    line = __LINE__;
                    goto FAIL;
                }
            }

            break;

        case E_MHAL_AI_IF_I2S_RX_D_0_1:
        case E_MHAL_AI_IF_I2S_RX_D_2_3:
        case E_MHAL_AI_IF_I2S_RX_D_4_5:
        case E_MHAL_AI_IF_I2S_RX_D_6_7:
        case E_MHAL_AI_IF_I2S_RX_D_8_9:
        case E_MHAL_AI_IF_I2S_RX_D_10_11:
        case E_MHAL_AI_IF_I2S_RX_D_12_13:
        case E_MHAL_AI_IF_I2S_RX_D_14_15:

            ret |= _MhalAudioAiCheckIfDoSettingByStatus(aiIf, bEn, &bIsDoSetting);
            if (ret != AIO_OK)
            {
                line = __LINE__;
                goto FAIL;
            }

            if (bIsDoSetting == TRUE)
            {
                v = CHIP_AIO_I2S_IDX_BY_AI_DEV_I2S_RX(E_AI_DEV_I2S_RX_D);
                if (!CHIP_AIO_I2S_IDX_VALID(v))
                {
                    line = __LINE__;
                    goto FAIL;
                }

                ret |= DrvAudEnableI2s((CHIP_AIO_I2S_e)v, bEn);
                if (ret != AIO_OK)
                {
                    line = __LINE__;
                    goto FAIL;
                }
            }

            break;

        default:
            break;
    }

    return AIO_OK;

FAIL:
    printf("%s[%d] enAiDma %d eAiIf[%d] bEn[%d] error\n", __FUNCTION__, line, enAiDma, eAiIf, bEn);
    return AIO_NG;
}

static int _MhalAudioAoIfEnableByStatus(AO_DMA_e enAoDma, AO_IF_e eAoIf, BOOL bEn)
{
    int     ret          = AIO_OK;
    int     v            = 0;
    AO_IF_e aoIf         = 0;
    BOOL    bIsDoSetting = FALSE;
    int     line         = 0;

    aoIf = eAoIf;

    if (aoIf & E_MHAL_AO_IF_DAC_A_0)
    {
        ret |= _MhalAudioAoCheckIfDoSettingByStatus(E_MHAL_AO_IF_DAC_A_0, bEn, &bIsDoSetting);
        if (ret != AIO_OK)
        {
            line = __LINE__;
            goto FAIL;
        }

        if (bIsDoSetting == TRUE)
        {
            v = CHIP_AIO_ATOP_IDX_BY_AO_DEV_DAC(E_AO_DEV_DAC_A);
            if (!CHIP_AIO_ATOP_IDX_VALID(v))
            {
                line = __LINE__;
                goto FAIL;
            }

            ret |= DrvAudEnableAtop((CHIP_AIO_ATOP_e)v, bEn);
            if (ret != AIO_OK)
            {
                line = __LINE__;
                goto FAIL;
            }
        }
    }

    if (aoIf & E_MHAL_AO_IF_DAC_B_0)
    {
        ret |= _MhalAudioAoCheckIfDoSettingByStatus(E_MHAL_AO_IF_DAC_B_0, bEn, &bIsDoSetting);
        if (ret != AIO_OK)
        {
            line = __LINE__;
            goto FAIL;
        }

        if (bIsDoSetting == TRUE)
        {
            v = CHIP_AIO_ATOP_IDX_BY_AO_DEV_DAC(E_AO_DEV_DAC_B);
            if (!CHIP_AIO_ATOP_IDX_VALID(v))
            {
                line = __LINE__;
                goto FAIL;
            }

            ret |= DrvAudEnableAtop((CHIP_AIO_ATOP_e)v, bEn);
            if (ret != AIO_OK)
            {
                line = __LINE__;
                goto FAIL;
            }
        }
    }

    if (aoIf & E_MHAL_AO_IF_DAC_C_0)
    {
        ret |= _MhalAudioAoCheckIfDoSettingByStatus(E_MHAL_AO_IF_DAC_C_0, bEn, &bIsDoSetting);
        if (ret != AIO_OK)
        {
            line = __LINE__;
            goto FAIL;
        }

        if (bIsDoSetting == TRUE)
        {
            v = CHIP_AIO_ATOP_IDX_BY_AO_DEV_DAC(E_AO_DEV_DAC_C);
            if (!CHIP_AIO_ATOP_IDX_VALID(v))
            {
                line = __LINE__;
                goto FAIL;
            }

            ret |= DrvAudEnableAtop((CHIP_AIO_ATOP_e)v, bEn);
            if (ret != AIO_OK)
            {
                line = __LINE__;
                goto FAIL;
            }
        }
    }

    if (aoIf & E_MHAL_AO_IF_DAC_D_0)
    {
        ret |= _MhalAudioAoCheckIfDoSettingByStatus(E_MHAL_AO_IF_DAC_D_0, bEn, &bIsDoSetting);
        if (ret != AIO_OK)
        {
            line = __LINE__;
            goto FAIL;
        }

        if (bIsDoSetting == TRUE)
        {
            v = CHIP_AIO_ATOP_IDX_BY_AO_DEV_DAC(E_AO_DEV_DAC_D);
            if (!CHIP_AIO_ATOP_IDX_VALID(v))
            {
                line = __LINE__;
                goto FAIL;
            }

            ret |= DrvAudEnableAtop((CHIP_AIO_ATOP_e)v, bEn);
            if (ret != AIO_OK)
            {
                line = __LINE__;
                goto FAIL;
            }
        }
    }

    if (aoIf & E_MHAL_AO_IF_HDMI_A_0)
    {
        ret |= _MhalAudioAoCheckIfDoSettingByStatus(E_MHAL_AO_IF_HDMI_A_0, bEn, &bIsDoSetting);
        if (ret != AIO_OK)
        {
            line = __LINE__;
            goto FAIL;
        }

        if (bIsDoSetting == TRUE)
        {
            v = CHIP_AO_HDMI_IDX_BY_DEV(E_AO_DEV_HDMI_A);
            if (!CHIP_AO_HDMI_VALID(v))
            {
                line = __LINE__;
                goto FAIL;
            }

            ret |= DrvAudEnableHdmi((CHIP_AO_HDMI_e)v, bEn);
            if (ret != AIO_OK)
            {
                line = __LINE__;
                goto FAIL;
            }
        }
    }

    if (aoIf & E_MHAL_AO_IF_HDMI_A_1)
    {
        ret |= _MhalAudioAoCheckIfDoSettingByStatus(E_MHAL_AO_IF_HDMI_A_1, bEn, &bIsDoSetting);
        if (ret != AIO_OK)
        {
            line = __LINE__;
            goto FAIL;
        }

        if (bIsDoSetting == TRUE)
        {
            v = CHIP_AO_HDMI_IDX_BY_DEV(E_AO_DEV_HDMI_A);
            if (!CHIP_AO_HDMI_VALID(v))
            {
                line = __LINE__;
                goto FAIL;
            }

            ret |= DrvAudEnableHdmi((CHIP_AO_HDMI_e)v, bEn);
            if (ret != AIO_OK)
            {
                line = __LINE__;
                goto FAIL;
            }
        }
    }

    if (aoIf & E_MHAL_AO_IF_ECHO_A_0)
    {
        //
    }

    if (aoIf & E_MHAL_AO_IF_ECHO_A_1)
    {
        //
    }

    if (aoIf & E_MHAL_AO_IF_I2S_TX_A_0)
    {
        ret |= _MhalAudioAoCheckIfDoSettingByStatus(E_MHAL_AO_IF_I2S_TX_A_0, bEn, &bIsDoSetting);
        if (ret != AIO_OK)
        {
            line = __LINE__;
            goto FAIL;
        }

        if (bIsDoSetting == TRUE)
        {
            v = CHIP_AIO_I2S_IDX_BY_AO_DEV_I2S_TX(E_AO_DEV_I2S_TX_A);
            if (!CHIP_AIO_I2S_IDX_VALID(v))
            {
                line = __LINE__;
                goto FAIL;
            }

            ret |= DrvAudEnableI2s((CHIP_AIO_I2S_e)v, bEn);
            if (ret != AIO_OK)
            {
                line = __LINE__;
                goto FAIL;
            }
        }
    }

    if (aoIf & E_MHAL_AO_IF_I2S_TX_A_1)
    {
        ret |= _MhalAudioAoCheckIfDoSettingByStatus(E_MHAL_AO_IF_I2S_TX_A_1, bEn, &bIsDoSetting);
        if (ret != AIO_OK)
        {
            line = __LINE__;
            goto FAIL;
        }

        if (bIsDoSetting == TRUE)
        {
            v = CHIP_AIO_I2S_IDX_BY_AO_DEV_I2S_TX(E_AO_DEV_I2S_TX_A);
            if (!CHIP_AIO_I2S_IDX_VALID(v))
            {
                line = __LINE__;
                goto FAIL;
            }

            ret |= DrvAudEnableI2s((CHIP_AIO_I2S_e)v, bEn);
            if (ret != AIO_OK)
            {
                line = __LINE__;
                goto FAIL;
            }
        }
    }

    if (aoIf & E_MHAL_AO_IF_I2S_TX_B_0)
    {
        ret |= _MhalAudioAoCheckIfDoSettingByStatus(E_MHAL_AO_IF_I2S_TX_B_0, bEn, &bIsDoSetting);
        if (ret != AIO_OK)
        {
            line = __LINE__;
            goto FAIL;
        }

        if (bIsDoSetting == TRUE)
        {
            v = CHIP_AIO_I2S_IDX_BY_AO_DEV_I2S_TX(E_AO_DEV_I2S_TX_B);
            if (!CHIP_AIO_I2S_IDX_VALID(v))
            {
                line = __LINE__;
                goto FAIL;
            }

            ret |= DrvAudEnableI2s((CHIP_AIO_I2S_e)v, bEn);
            if (ret != AIO_OK)
            {
                line = __LINE__;
                goto FAIL;
            }
        }
    }

    if (aoIf & E_MHAL_AO_IF_I2S_TX_B_1)
    {
        ret |= _MhalAudioAoCheckIfDoSettingByStatus(E_MHAL_AO_IF_I2S_TX_B_1, bEn, &bIsDoSetting);
        if (ret != AIO_OK)
        {
            line = __LINE__;
            goto FAIL;
        }

        if (bIsDoSetting == TRUE)
        {
            v = CHIP_AIO_I2S_IDX_BY_AO_DEV_I2S_TX(E_AO_DEV_I2S_TX_B);
            if (!CHIP_AIO_I2S_IDX_VALID(v))
            {
                line = __LINE__;
                goto FAIL;
            }

            ret |= DrvAudEnableI2s((CHIP_AIO_I2S_e)v, bEn);
            if (ret != AIO_OK)
            {
                line = __LINE__;
                goto FAIL;
            }
        }
    }

    return AIO_OK;

FAIL:
    printf("%s[%d] enAoDma %d eAoIf[%d] bEn[%d] error\n", __FUNCTION__, line, enAoDma, eAoIf, bEn);
    return AIO_NG;
}

static int _MhalAudioAiIfToDevCh(MHAL_AI_IF_e eAiIf, AI_CH_e *peAiCh1, AI_CH_e *peAiCh2)
{
    switch (eAiIf)
    {
        case E_MHAL_AI_IF_ADC_A_0_B_0:
            *peAiCh1 = E_AI_CH_ADC_A_0;
            *peAiCh2 = E_AI_CH_ADC_B_0;
            break;

        case E_MHAL_AI_IF_ADC_C_0_D_0:
            *peAiCh1 = E_AI_CH_ADC_C_0;
            *peAiCh2 = E_AI_CH_ADC_D_0;
            break;

        case E_MHAL_AI_IF_DMIC_A_0_1:
            *peAiCh1 = E_AI_CH_DMIC_A_0;
            *peAiCh2 = E_AI_CH_DMIC_A_1;
            break;

        case E_MHAL_AI_IF_DMIC_A_2_3:
            *peAiCh1 = E_AI_CH_DMIC_A_2;
            *peAiCh2 = E_AI_CH_DMIC_A_3;
            break;

        case E_MHAL_AI_IF_HDMI_A_0_1:
            *peAiCh1 = E_AI_CH_HDMI_A_0;
            *peAiCh2 = E_AI_CH_HDMI_A_1;
            break;

        case E_MHAL_AI_IF_ECHO_A_0_1:
            *peAiCh1 = E_AI_CH_ECHO_A_0;
            *peAiCh2 = E_AI_CH_ECHO_A_1;
            break;

        case E_MHAL_AI_IF_I2S_RX_A_0_1:
            *peAiCh1 = E_AI_CH_I2S_RX_A_0;
            *peAiCh2 = E_AI_CH_I2S_RX_A_1;
            break;

        case E_MHAL_AI_IF_I2S_RX_A_2_3:
            *peAiCh1 = E_AI_CH_I2S_RX_A_2;
            *peAiCh2 = E_AI_CH_I2S_RX_A_3;
            break;

        case E_MHAL_AI_IF_I2S_RX_A_4_5:
            *peAiCh1 = E_AI_CH_I2S_RX_A_4;
            *peAiCh2 = E_AI_CH_I2S_RX_A_5;
            break;

        case E_MHAL_AI_IF_I2S_RX_A_6_7:
            *peAiCh1 = E_AI_CH_I2S_RX_A_6;
            *peAiCh2 = E_AI_CH_I2S_RX_A_7;
            break;

        case E_MHAL_AI_IF_I2S_RX_A_8_9:
            *peAiCh1 = E_AI_CH_I2S_RX_A_8;
            *peAiCh2 = E_AI_CH_I2S_RX_A_9;
            break;

        case E_MHAL_AI_IF_I2S_RX_A_10_11:
            *peAiCh1 = E_AI_CH_I2S_RX_A_10;
            *peAiCh2 = E_AI_CH_I2S_RX_A_11;
            break;

        case E_MHAL_AI_IF_I2S_RX_A_12_13:
            *peAiCh1 = E_AI_CH_I2S_RX_A_12;
            *peAiCh2 = E_AI_CH_I2S_RX_A_13;
            break;

        case E_MHAL_AI_IF_I2S_RX_A_14_15:
            *peAiCh1 = E_AI_CH_I2S_RX_A_14;
            *peAiCh2 = E_AI_CH_I2S_RX_A_15;
            break;

        case E_MHAL_AI_IF_I2S_RX_B_0_1:
            *peAiCh1 = E_AI_CH_I2S_RX_B_0;
            *peAiCh2 = E_AI_CH_I2S_RX_B_1;
            break;

        case E_MHAL_AI_IF_I2S_RX_B_2_3:
            *peAiCh1 = E_AI_CH_I2S_RX_B_2;
            *peAiCh2 = E_AI_CH_I2S_RX_B_3;
            break;

        case E_MHAL_AI_IF_I2S_RX_B_4_5:
            *peAiCh1 = E_AI_CH_I2S_RX_B_4;
            *peAiCh2 = E_AI_CH_I2S_RX_B_5;
            break;

        case E_MHAL_AI_IF_I2S_RX_B_6_7:
            *peAiCh1 = E_AI_CH_I2S_RX_B_6;
            *peAiCh2 = E_AI_CH_I2S_RX_B_7;
            break;

        case E_MHAL_AI_IF_I2S_RX_B_8_9:
            *peAiCh1 = E_AI_CH_I2S_RX_B_8;
            *peAiCh2 = E_AI_CH_I2S_RX_B_9;
            break;

        case E_MHAL_AI_IF_I2S_RX_B_10_11:
            *peAiCh1 = E_AI_CH_I2S_RX_B_10;
            *peAiCh2 = E_AI_CH_I2S_RX_B_11;
            break;

        case E_MHAL_AI_IF_I2S_RX_B_12_13:
            *peAiCh1 = E_AI_CH_I2S_RX_B_12;
            *peAiCh2 = E_AI_CH_I2S_RX_B_13;
            break;

        case E_MHAL_AI_IF_I2S_RX_B_14_15:
            *peAiCh1 = E_AI_CH_I2S_RX_B_14;
            *peAiCh2 = E_AI_CH_I2S_RX_B_15;
            break;

        case E_MHAL_AI_IF_I2S_RX_C_0_1:
            *peAiCh1 = E_AI_CH_I2S_RX_C_0;
            *peAiCh2 = E_AI_CH_I2S_RX_C_1;
            break;

        case E_MHAL_AI_IF_I2S_RX_C_2_3:
            *peAiCh1 = E_AI_CH_I2S_RX_C_2;
            *peAiCh2 = E_AI_CH_I2S_RX_C_3;
            break;

        case E_MHAL_AI_IF_I2S_RX_C_4_5:
            *peAiCh1 = E_AI_CH_I2S_RX_C_4;
            *peAiCh2 = E_AI_CH_I2S_RX_C_5;
            break;

        case E_MHAL_AI_IF_I2S_RX_C_6_7:
            *peAiCh1 = E_AI_CH_I2S_RX_C_6;
            *peAiCh2 = E_AI_CH_I2S_RX_C_7;
            break;

        case E_MHAL_AI_IF_I2S_RX_C_8_9:
            *peAiCh1 = E_AI_CH_I2S_RX_C_8;
            *peAiCh2 = E_AI_CH_I2S_RX_C_9;
            break;

        case E_MHAL_AI_IF_I2S_RX_C_10_11:
            *peAiCh1 = E_AI_CH_I2S_RX_C_10;
            *peAiCh2 = E_AI_CH_I2S_RX_C_11;
            break;

        case E_MHAL_AI_IF_I2S_RX_C_12_13:
            *peAiCh1 = E_AI_CH_I2S_RX_C_12;
            *peAiCh2 = E_AI_CH_I2S_RX_C_13;
            break;

        case E_MHAL_AI_IF_I2S_RX_C_14_15:
            *peAiCh1 = E_AI_CH_I2S_RX_C_14;
            *peAiCh2 = E_AI_CH_I2S_RX_C_15;
            break;

        case E_MHAL_AI_IF_I2S_RX_D_0_1:
            *peAiCh1 = E_AI_CH_I2S_RX_D_0;
            *peAiCh2 = E_AI_CH_I2S_RX_D_1;
            break;

        case E_MHAL_AI_IF_I2S_RX_D_2_3:
            *peAiCh1 = E_AI_CH_I2S_RX_D_2;
            *peAiCh2 = E_AI_CH_I2S_RX_D_3;
            break;

        case E_MHAL_AI_IF_I2S_RX_D_4_5:
            *peAiCh1 = E_AI_CH_I2S_RX_D_4;
            *peAiCh2 = E_AI_CH_I2S_RX_D_5;
            break;

        case E_MHAL_AI_IF_I2S_RX_D_6_7:
            *peAiCh1 = E_AI_CH_I2S_RX_D_6;
            *peAiCh2 = E_AI_CH_I2S_RX_D_7;
            break;

        case E_MHAL_AI_IF_I2S_RX_D_8_9:
            *peAiCh1 = E_AI_CH_I2S_RX_D_8;
            *peAiCh2 = E_AI_CH_I2S_RX_D_9;
            break;

        case E_MHAL_AI_IF_I2S_RX_D_10_11:
            *peAiCh1 = E_AI_CH_I2S_RX_D_10;
            *peAiCh2 = E_AI_CH_I2S_RX_D_11;
            break;

        case E_MHAL_AI_IF_I2S_RX_D_12_13:
            *peAiCh1 = E_AI_CH_I2S_RX_D_12;
            *peAiCh2 = E_AI_CH_I2S_RX_D_13;
            break;

        case E_MHAL_AI_IF_I2S_RX_D_14_15:
            *peAiCh1 = E_AI_CH_I2S_RX_D_14;
            *peAiCh2 = E_AI_CH_I2S_RX_D_15;
            break;

        default:
            printf("%s[%d] eAiIf[%d] error\n", __FUNCTION__, __LINE__, eAiIf);
            goto FAIL;
            break;
    }

    return AIO_OK;

FAIL:

    return AIO_NG;
}

static int _MhalAudioAoIfToDevCh(MHAL_AO_IF_e eAoIf, AO_CH_e *peAoCh, MS_U8 *pnGetNum)
{
    int i    = 0;
    int line = 0;

    if (eAoIf & E_MHAL_AO_IF_DAC_A_0)
    {
        if (i < AO_IF_IN_ONE_DMA_CH_MAX)
        {
            peAoCh[i] = E_AO_CH_DAC_A_0;
            i++;
        }
        else
        {
            line = __LINE__;
            goto FAIL;
        }
    }

    if (eAoIf & E_MHAL_AO_IF_DAC_B_0)
    {
        if (i < AO_IF_IN_ONE_DMA_CH_MAX)
        {
            peAoCh[i] = E_AO_CH_DAC_B_0;
            i++;
        }
        else
        {
            line = __LINE__;
            goto FAIL;
        }
    }

    if (eAoIf & E_MHAL_AO_IF_DAC_C_0)
    {
        if (i < AO_IF_IN_ONE_DMA_CH_MAX)
        {
            peAoCh[i] = E_AO_CH_DAC_C_0;
            i++;
        }
        else
        {
            line = __LINE__;
            goto FAIL;
        }
    }

    if (eAoIf & E_MHAL_AO_IF_DAC_D_0)
    {
        if (i < AO_IF_IN_ONE_DMA_CH_MAX)
        {
            peAoCh[i] = E_AO_CH_DAC_D_0;
            i++;
        }
        else
        {
            line = __LINE__;
            goto FAIL;
        }
    }

    if (eAoIf & E_MHAL_AO_IF_HDMI_A_0)
    {
        if (i < AO_IF_IN_ONE_DMA_CH_MAX)
        {
            peAoCh[i] = E_AO_CH_HDMI_A_0;
            i++;
        }
        else
        {
            line = __LINE__;
            goto FAIL;
        }
    }

    if (eAoIf & E_MHAL_AO_IF_HDMI_A_1)
    {
        if (i < AO_IF_IN_ONE_DMA_CH_MAX)
        {
            peAoCh[i] = E_AO_CH_HDMI_A_1;
            i++;
        }
        else
        {
            line = __LINE__;
            goto FAIL;
        }
    }

    if (eAoIf & E_MHAL_AO_IF_ECHO_A_0)
    {
        if (i < AO_IF_IN_ONE_DMA_CH_MAX)
        {
            peAoCh[i] = E_AO_CH_ECHO_A_0;
            i++;
        }
        else
        {
            line = __LINE__;
            goto FAIL;
        }
    }

    if (eAoIf & E_MHAL_AO_IF_ECHO_A_1)
    {
        if (i < AO_IF_IN_ONE_DMA_CH_MAX)
        {
            peAoCh[i] = E_AO_CH_ECHO_A_1;
            i++;
        }
        else
        {
            line = __LINE__;
            goto FAIL;
        }
    }

    if (eAoIf & E_MHAL_AO_IF_I2S_TX_A_0)
    {
        if (i < AO_IF_IN_ONE_DMA_CH_MAX)
        {
            peAoCh[i] = E_AO_CH_I2S_TX_A_0;
            i++;
        }
        else
        {
            line = __LINE__;
            goto FAIL;
        }
    }

    if (eAoIf & E_MHAL_AO_IF_I2S_TX_A_1)
    {
        if (i < AO_IF_IN_ONE_DMA_CH_MAX)
        {
            peAoCh[i] = E_AO_CH_I2S_TX_A_1;
            i++;
        }
        else
        {
            line = __LINE__;
            goto FAIL;
        }
    }

    if (eAoIf & E_MHAL_AO_IF_I2S_TX_B_0)
    {
        if (i < AO_IF_IN_ONE_DMA_CH_MAX)
        {
            peAoCh[i] = E_AO_CH_I2S_TX_B_0;
            i++;
        }
        else
        {
            line = __LINE__;
            goto FAIL;
        }
    }

    if (eAoIf & E_MHAL_AO_IF_I2S_TX_B_1)
    {
        if (i < AO_IF_IN_ONE_DMA_CH_MAX)
        {
            peAoCh[i] = E_AO_CH_I2S_TX_B_1;
            i++;
        }
        else
        {
            line = __LINE__;
            goto FAIL;
        }
    }

    //
    *pnGetNum = (MS_U8)i;

    return AIO_OK;

FAIL:
    printf("%s[%d] eAoIf %d error\n", __FUNCTION__, line, eAoIf);
    return AIO_NG;
}

static int _MhalAudioAiCheckIfDoSettingByStatus(MHAL_AI_IF_e eAiIf, BOOL bEn, BOOL *pbIsDoSetting)
{
    int     i          = 0;
    AI_CH_e eAiChStart = 0, eAiChEnd = 0;
    BOOL    bIsDoSetting = FALSE;

    //
    switch (eAiIf)
    {
        case E_MHAL_AI_IF_ADC_A_0_B_0:
            eAiChStart = E_AI_CH_ADC_A_0;
            eAiChEnd   = E_AI_CH_ADC_B_0;
            break;

        case E_MHAL_AI_IF_ADC_C_0_D_0:
            eAiChStart = E_AI_CH_ADC_C_0;
            eAiChEnd   = E_AI_CH_ADC_D_0;
            break;

        case E_MHAL_AI_IF_DMIC_A_0_1:
            eAiChStart = E_AI_CH_DMIC_A_0;
            eAiChEnd   = E_AI_CH_DMIC_A_3;
            break;

        case E_MHAL_AI_IF_DMIC_A_2_3:
            eAiChStart = E_AI_CH_DMIC_A_0;
            eAiChEnd   = E_AI_CH_DMIC_A_3;
            break;

        case E_MHAL_AI_IF_HDMI_A_0_1:
            eAiChStart = E_AI_CH_HDMI_A_0;
            eAiChEnd   = E_AI_CH_HDMI_A_1;
            break;

        case E_MHAL_AI_IF_ECHO_A_0_1:
            eAiChStart = E_AI_CH_ECHO_A_0;
            eAiChEnd   = E_AI_CH_ECHO_A_1;
            break;

        case E_MHAL_AI_IF_I2S_RX_A_0_1:
        case E_MHAL_AI_IF_I2S_RX_A_2_3:
        case E_MHAL_AI_IF_I2S_RX_A_4_5:
        case E_MHAL_AI_IF_I2S_RX_A_6_7:
        case E_MHAL_AI_IF_I2S_RX_A_8_9:
        case E_MHAL_AI_IF_I2S_RX_A_10_11:
        case E_MHAL_AI_IF_I2S_RX_A_12_13:
        case E_MHAL_AI_IF_I2S_RX_A_14_15:
            eAiChStart = E_AI_CH_I2S_RX_A_0;
            eAiChEnd   = E_AI_CH_I2S_RX_A_15;
            break;

        case E_MHAL_AI_IF_I2S_RX_B_0_1:
        case E_MHAL_AI_IF_I2S_RX_B_2_3:
        case E_MHAL_AI_IF_I2S_RX_B_4_5:
        case E_MHAL_AI_IF_I2S_RX_B_6_7:
        case E_MHAL_AI_IF_I2S_RX_B_8_9:
        case E_MHAL_AI_IF_I2S_RX_B_10_11:
        case E_MHAL_AI_IF_I2S_RX_B_12_13:
        case E_MHAL_AI_IF_I2S_RX_B_14_15:
            eAiChStart = E_AI_CH_I2S_RX_B_0;
            eAiChEnd   = E_AI_CH_I2S_RX_B_15;
            break;

        case E_MHAL_AI_IF_I2S_RX_C_0_1:
        case E_MHAL_AI_IF_I2S_RX_C_2_3:
        case E_MHAL_AI_IF_I2S_RX_C_4_5:
        case E_MHAL_AI_IF_I2S_RX_C_6_7:
        case E_MHAL_AI_IF_I2S_RX_C_8_9:
        case E_MHAL_AI_IF_I2S_RX_C_10_11:
        case E_MHAL_AI_IF_I2S_RX_C_12_13:
        case E_MHAL_AI_IF_I2S_RX_C_14_15:
            eAiChStart = E_AI_CH_I2S_RX_C_0;
            eAiChEnd   = E_AI_CH_I2S_RX_C_15;
            break;

        case E_MHAL_AI_IF_I2S_RX_D_0_1:
        case E_MHAL_AI_IF_I2S_RX_D_2_3:
        case E_MHAL_AI_IF_I2S_RX_D_4_5:
        case E_MHAL_AI_IF_I2S_RX_D_6_7:
        case E_MHAL_AI_IF_I2S_RX_D_8_9:
        case E_MHAL_AI_IF_I2S_RX_D_10_11:
        case E_MHAL_AI_IF_I2S_RX_D_12_13:
        case E_MHAL_AI_IF_I2S_RX_D_14_15:
            eAiChStart = E_AI_CH_I2S_RX_D_0;
            eAiChEnd   = E_AI_CH_I2S_RX_D_15;
            break;

        default:
            break;
    }

    //
    if (bEn)
    {
        bIsDoSetting = FALSE;

        for (i = eAiChStart; i <= eAiChEnd; i++)
        {
            if (g_pAiIfStatusList[i].ncAttachedCount != 0)
            {
                bIsDoSetting = TRUE;
                break;
            }
        }
    }
    else
    {
        bIsDoSetting = TRUE;

        for (i = eAiChStart; i <= eAiChEnd; i++)
        {
            if (g_pAiIfStatusList[i].ncAttachedCount != 0)
            {
                bIsDoSetting = FALSE;
                break;
            }
        }
    }

    //
    *pbIsDoSetting = bIsDoSetting;

    return AIO_OK;
}

static int _MhalAudioAoCheckIfDoSettingByStatus(MHAL_AO_IF_e eAoIf, BOOL bEn, BOOL *pbIsDoSetting)
{
    int     i          = 0;
    AO_CH_e eAoChStart = 0, eAoChEnd = 0;
    BOOL    bIsDoSetting = FALSE;

    // Note:
    // In this function, eAoIf only accept "single" IF

    //
    switch (eAoIf)
    {
        case E_MHAL_AO_IF_DAC_A_0:
            eAoChStart = E_AO_CH_DAC_A_0;
            eAoChEnd   = E_AO_CH_DAC_B_0;
            break;

        case E_MHAL_AO_IF_DAC_B_0:
            eAoChStart = E_AO_CH_DAC_A_0;
            eAoChEnd   = E_AO_CH_DAC_B_0;
            break;

        case E_MHAL_AO_IF_DAC_C_0:
            eAoChStart = E_AO_CH_DAC_C_0;
            eAoChEnd   = E_AO_CH_DAC_D_0;
            break;

        case E_MHAL_AO_IF_DAC_D_0:
            eAoChStart = E_AO_CH_DAC_C_0;
            eAoChEnd   = E_AO_CH_DAC_D_0;
            break;

        case E_MHAL_AO_IF_HDMI_A_0:
            eAoChStart = E_AO_CH_HDMI_A_0;
            eAoChEnd   = E_AO_CH_HDMI_A_1;
            break;

        case E_MHAL_AO_IF_HDMI_A_1:
            eAoChStart = E_AO_CH_HDMI_A_0;
            eAoChEnd   = E_AO_CH_HDMI_A_1;
            break;

        case E_MHAL_AO_IF_ECHO_A_0:
            eAoChStart = E_AO_CH_ECHO_A_0;
            eAoChEnd   = E_AO_CH_ECHO_A_1;
            break;

        case E_MHAL_AO_IF_ECHO_A_1:
            eAoChStart = E_AO_CH_ECHO_A_0;
            eAoChEnd   = E_AO_CH_ECHO_A_1;
            break;

        case E_MHAL_AO_IF_I2S_TX_A_0:
            eAoChStart = E_AO_CH_I2S_TX_A_0;
            eAoChEnd   = E_AO_CH_I2S_TX_A_1;
            break;

        case E_MHAL_AO_IF_I2S_TX_A_1:
            eAoChStart = E_AO_CH_I2S_TX_A_0;
            eAoChEnd   = E_AO_CH_I2S_TX_A_1;
            break;

        case E_MHAL_AO_IF_I2S_TX_B_0:
            eAoChStart = E_AO_CH_I2S_TX_B_0;
            eAoChEnd   = E_AO_CH_I2S_TX_B_1;
            break;

        case E_MHAL_AO_IF_I2S_TX_B_1:
            eAoChStart = E_AO_CH_I2S_TX_B_0;
            eAoChEnd   = E_AO_CH_I2S_TX_B_1;
            break;

        default:
            break;
    }

    //
    if (bEn)
    {
        bIsDoSetting = FALSE;

        for (i = eAoChStart; i <= eAoChEnd; i++)
        {
            if (g_pAoIfStatusList[i].ncAttachedCount != 0)
            {
                bIsDoSetting = TRUE;
                break;
            }
        }
    }
    else
    {
        bIsDoSetting = TRUE;

        for (i = eAoChStart; i <= eAoChEnd; i++)
        {
            if (g_pAoIfStatusList[i].ncAttachedCount != 0)
            {
                bIsDoSetting = FALSE;
                break;
            }
        }
    }

    //
    *pbIsDoSetting = bIsDoSetting;

    return AIO_OK;
}

static int _MhalAudioAiEchoCfgByDma(MHAL_AI_Dma_e enAiDma)
{
    int            ret     = AIO_OK;
    int            i       = 0;
    AI_IF_e        aiIf    = 0;
    CHIP_AIO_DMA_e eAioDma = 0;
    SrcParam_t     ptSrcParam;
    AI_CH_e        eAiCh1, eAiCh2;
    BOOL           bIsDoSetting = FALSE;
    int            line         = 0;

    if (!CHIP_AI_DMA_VALID(enAiDma))
    {
        line = __LINE__;
        goto FAIL;
    }

    //
    eAioDma = CHIP_AIO_DMA_IDX_BY_AI_DMA(enAiDma);
    if (!CHIP_AIO_DMA_IDX_VALID(eAioDma))
    {
        line = __LINE__;
        goto FAIL;
    }

    //
    ptSrcParam.nSampleRate = DrvAudGetDmaSampleRate(eAioDma);

    //
    for (i = 0; i < E_MHAL_AI_DMA_CH_SLOT_TOTAL; i++)
    {
        bIsDoSetting = FALSE;

        aiIf = g_pAiDmaStatusList[enAiDma].eAiIf[i];

        switch (aiIf)
        {
            case E_MHAL_AI_IF_ECHO_A_0_1:
                bIsDoSetting = TRUE;
                break;

            default:
                continue;
                break;
        }

        if (bIsDoSetting == TRUE)
        {
            ret |= _MhalAudioAiIfToDevCh(aiIf, &eAiCh1, &eAiCh2);
            if (ret != AIO_OK)
            {
                line = __LINE__;
                goto FAIL;
            }

            ret |= DrvAudConfigSrcParam(eAiCh1, &ptSrcParam);
            ret |= DrvAudConfigSrcParam(eAiCh2, &ptSrcParam);
            if (ret != AIO_OK)
            {
                line = __LINE__;
                goto FAIL;
            }
        }
    }

    return AIO_OK;

FAIL:
    printf("%s[%d] enAiDma %d error\n", __FUNCTION__, line, enAiDma);
    return AIO_NG;
}

static int _MhalAudioAiDmicSampleRateCfgByDma(MHAL_AI_Dma_e enAiDma, BOOL *pbIsDigMicNeedUpdate)
{
    int            ret       = AIO_OK;
    int            i         = 0;
    CHIP_AI_DMIC_e eChipDmic = 0;
    int            line      = 0;

    //
    for (i = 0; i < AI_DEV_DMIC_TOTAL; i++)
    {
        if (pbIsDigMicNeedUpdate[i] == TRUE)
        {
            eChipDmic = i;
            ret |= DrvAudConfigDigMicSampleRate(eChipDmic, g_pAiDmaStatusList[enAiDma].nSampleRate);
            if (ret != AIO_OK)
            {
                line = __LINE__;
                goto FAIL;
            }
        }
    }

    return AIO_OK;

FAIL:
    printf("%s[%d] enAiDma %d error\n", __FUNCTION__, line, enAiDma);
    return AIO_NG;
}

static int _MhalAudioAiDmicChNumCfg(BOOL *pbIsDigMicNeedUpdate)
{
    int            ret = AIO_OK;
    int            i = 0, j = 0;
    CHIP_AI_DMIC_e eChipDmic    = 0;
    u8             nDmicChNum   = 0;
    BOOL           bIsDoSetting = FALSE;

    //
    for (i = 0; i < AI_DEV_DMIC_TOTAL; i++)
    {
        bIsDoSetting = FALSE;

        if (pbIsDigMicNeedUpdate[i] == TRUE)
        {
            for (j = 0; j < DMIC_CH_MAX; j++)
            {
                if (g_pAiDigMicStatusList[i].anDmicChAttachedCount[j] != 0)
                {
                    nDmicChNum   = j + 1;
                    bIsDoSetting = TRUE;
                }
            }

            //
            if ((nDmicChNum == 1) || (nDmicChNum == 2))
            {
                //
            }
            else if (nDmicChNum <= 4)
            {
                nDmicChNum = 4;
            }
            else
            {
                nDmicChNum = 8;
            }

            //
            if (bIsDoSetting == TRUE)
            {
                eChipDmic = i;

                ret |= DrvAudConfigDigMicChNum(eChipDmic, nDmicChNum);
                if (ret != AIO_OK)
                {
                    printf("%s[%d] eChipDmic %d error\n", __FUNCTION__, __LINE__, eChipDmic);
                    goto FAIL;
                }
            }
        }
    }

    return AIO_OK;

FAIL:

    return AIO_NG;
}

static int _MhalAudioAiDmicStatusUpdate(AI_IF_e eAiIf, BOOL bEn, BOOL *pbIsDigMicNeedUpdate)
{
    u8 nDmicIdx = 0;

    switch (eAiIf)
    {
        case E_MHAL_AI_IF_DMIC_A_0_1:

            nDmicIdx = (E_AI_DEV_DMIC_A - E_AI_DEV_DMIC_START);

            if (bEn)
            {
                g_pAiDigMicStatusList[nDmicIdx].anDmicChAttachedCount[0]++;
                g_pAiDigMicStatusList[nDmicIdx].anDmicChAttachedCount[1]++;
            }
            else
            {
                if (g_pAiDigMicStatusList[nDmicIdx].anDmicChAttachedCount[0] != 0)
                {
                    g_pAiDigMicStatusList[nDmicIdx].anDmicChAttachedCount[0]--;
                }
                else
                {
                    printf("%s[%d] eAiIf %d nDmicIdx %d DmicCh 0 error\n", __FUNCTION__, __LINE__, eAiIf, nDmicIdx);
                    goto FAIL;
                }

                if (g_pAiDigMicStatusList[nDmicIdx].anDmicChAttachedCount[1] != 0)
                {
                    g_pAiDigMicStatusList[nDmicIdx].anDmicChAttachedCount[1]--;
                }
                else
                {
                    printf("%s[%d] eAiIf %d nDmicIdx %d DmicCh 1 error\n", __FUNCTION__, __LINE__, eAiIf, nDmicIdx);
                    goto FAIL;
                }
            }

            pbIsDigMicNeedUpdate[nDmicIdx] = TRUE;

            break;

        case E_MHAL_AI_IF_DMIC_A_2_3:

            nDmicIdx = (E_AI_DEV_DMIC_A - E_AI_DEV_DMIC_START);

            if (bEn)
            {
                g_pAiDigMicStatusList[nDmicIdx].anDmicChAttachedCount[2]++;
                g_pAiDigMicStatusList[nDmicIdx].anDmicChAttachedCount[3]++;
            }
            else
            {
                if (g_pAiDigMicStatusList[nDmicIdx].anDmicChAttachedCount[2] != 0)
                {
                    g_pAiDigMicStatusList[nDmicIdx].anDmicChAttachedCount[2]--;
                }
                else
                {
                    printf("%s[%d] eAiIf %d nDmicIdx %d DmicCh 2 error\n", __FUNCTION__, __LINE__, eAiIf, nDmicIdx);
                    goto FAIL;
                }

                if (g_pAiDigMicStatusList[nDmicIdx].anDmicChAttachedCount[3] != 0)
                {
                    g_pAiDigMicStatusList[nDmicIdx].anDmicChAttachedCount[3]--;
                }
                else
                {
                    printf("%s[%d] eAiIf %d nDmicIdx %d DmicCh 3 error\n", __FUNCTION__, __LINE__, eAiIf, nDmicIdx);
                    goto FAIL;
                }
            }

            pbIsDigMicNeedUpdate[nDmicIdx] = TRUE;

            break;

        default:
            break;
    }

    return AIO_OK;

FAIL:

    return AIO_NG;
}

static int _MhalAudioAiIfStatusUpdateAndAction(AI_IF_e eAiIf, BOOL bIsAttach)
{
    int     ret = AIO_OK;
    AI_CH_e eAiCh1, eAiCh2;
    int     line = 0;

    //
    if (eAiIf == E_MHAL_AI_IF_NONE)
    {
        goto SUCCESS;
    }

    //
    ret |= _MhalAudioAiIfToDevCh(eAiIf, &eAiCh1, &eAiCh2);
    if (ret != AIO_OK)
    {
        line = __LINE__;
        goto FAIL;
    }

    if (bIsAttach == TRUE)
    {
        g_pAiIfStatusList[eAiCh1].ncAttachedCount++;
        g_pAiIfStatusList[eAiCh2].ncAttachedCount++;
    }
    else
    {
        if (g_pAiIfStatusList[eAiCh1].ncAttachedCount == 0)
        {
            line = __LINE__;
            goto FAIL;
        }

        if (g_pAiIfStatusList[eAiCh2].ncAttachedCount == 0)
        {
            line = __LINE__;
            goto FAIL;
        }

        g_pAiIfStatusList[eAiCh1].ncAttachedCount--;
        g_pAiIfStatusList[eAiCh2].ncAttachedCount--;
    }

SUCCESS:

    return AIO_OK;

FAIL:
    printf("%s[%d] eAoIf %d eAiCh1 %d eAiCh2 %d bIsAttach %d error\n", __FUNCTION__, line, eAiIf, eAiCh1, eAiCh2,
           bIsAttach);
    return AIO_NG;
}

static int _MhalAudioAoIfStatusUpdateAndAction(AO_IF_e eAoIf, BOOL bIsAttach)
{
    int     ret = AIO_OK;
    int     i   = 0;
    AO_CH_e aeAoCh[AO_IF_IN_ONE_DMA_CH_MAX];
    MS_U8   nGetNum = 0;
    int     line    = 0;

    //
    if (eAoIf == E_MHAL_AO_IF_NONE)
    {
        goto SUCCESS;
    }

    ret |= _MhalAudioAoIfToDevCh(eAoIf, aeAoCh, &nGetNum);
    if (ret != AIO_OK)
    {
        line = __LINE__;
        goto FAIL;
    }

    if (bIsAttach == TRUE)
    {
        for (i = 0; i < nGetNum; i++)
        {
            g_pAoIfStatusList[aeAoCh[i]].ncAttachedCount++;

            if (g_pAoIfStatusList[aeAoCh[i]].ncAttachedCount == 2)
            {
                ret |= DrvAudAoIfMultiAttachAction(aeAoCh[i], g_pAoIfStatusList[aeAoCh[i]].ncAttachedCount);
                if (ret != AIO_OK)
                {
                    line = __LINE__;
                    goto FAIL;
                }
            }
            else if (g_pAoIfStatusList[aeAoCh[i]].ncAttachedCount > 2)
            {
                printf("%s, g_pAoIfStatusList[%d].ncAttachedCount = %d > 2\n", __FUNCTION__, aeAoCh[i],
                       g_pAoIfStatusList[aeAoCh[i]].ncAttachedCount);
                line = __LINE__;
                goto FAIL;
            }
        }
    }
    else
    {
        for (i = 0; i < nGetNum; i++)
        {
            g_pAoIfStatusList[aeAoCh[i]].ncAttachedCount--;

            if ((g_pAoIfStatusList[aeAoCh[i]].ncAttachedCount == 0)
                || (g_pAoIfStatusList[aeAoCh[i]].ncAttachedCount == 1))
            {
                // Do disable mux (mixer) or mute by eAoCh -> DEV ?

                ret |= DrvAudAoIfMultiAttachAction(aeAoCh[i], g_pAoIfStatusList[aeAoCh[i]].ncAttachedCount);
                if (ret != AIO_OK)
                {
                    line = __LINE__;
                    goto FAIL;
                }
            }
            else if (g_pAoIfStatusList[aeAoCh[i]].ncAttachedCount > 2)
            {
                printf("%s, g_pAoIfStatusList[%d].ncAttachedCount = %d > 2\n", __FUNCTION__, aeAoCh[i],
                       g_pAoIfStatusList[aeAoCh[i]].ncAttachedCount);
                line = __LINE__;
                goto FAIL;
            }
        }
    }

SUCCESS:

    return AIO_OK;

FAIL:
    printf("%s[%d] eAoIf %d error\n", __FUNCTION__, line, eAoIf);
    return AIO_NG;
}

static int _MhalAudioAiDetachDma(MHAL_AI_Dma_e enAiDma)
{
    int      ret                                    = AIO_OK;
    int      i                                      = 0;
    AI_DMA_e aiDma                                  = 0;
    AI_IF_e  aiIf                                   = 0;
    BOOL     aIsDigMicNeedUpdate[AI_DEV_DMIC_TOTAL] = {0};
    int      line                                   = 0;

    //
    aiDma = enAiDma;

    //
    if (aiDma >= AI_DMA_DIRECT_START)
    {
        if (aiDma >= E_MHAL_AI_DMA_TOTAL)
        {
            printf("%s[%d] aiDma direct %d error\n", __FUNCTION__, __LINE__, aiDma);
            line = __LINE__;
            goto FAIL;
        }
    }
    else
    {
        if (!CHIP_AI_DMA_VALID(aiDma))
        {
            printf("%s[%d] aiDma %d error\n", __FUNCTION__, __LINE__, aiDma);
            line = __LINE__;
            goto FAIL;
        }
    }

    //
    for (i = 0; i < E_MHAL_AI_DMA_CH_SLOT_TOTAL; i++)
    {
        aiIf = (g_pAiDmaStatusList[aiDma].eAiIf[i]);

        if (aiIf != E_MHAL_AI_IF_NONE)
        {
            //
            if (aiDma >= AI_DMA_DIRECT_START)
            {
                ret |= _MhalAudioAiDirectDettachCore(aiDma, i, aiIf);
                if (ret != AIO_OK)
                {
                    line = __LINE__;
                    goto FAIL;
                }
            }
            else
            {
                ret |= _MhalAudioAiDettachCore(aiDma, i, aiIf);
                if (ret != AIO_OK)
                {
                    line = __LINE__;
                    goto FAIL;
                }
            }

            // AI DMIC Status Update
            ret |= _MhalAudioAiDmicStatusUpdate(aiIf, FALSE, aIsDigMicNeedUpdate);
            if (ret != AIO_OK)
            {
                line = __LINE__;
                goto FAIL;
            }

            ret |= _MhalAudioAiIfStatusUpdateAndAction(aiIf, FALSE);
            if (ret != AIO_OK)
            {
                line = __LINE__;
                goto FAIL;
            }

            //
            ret |= _MhalAudioAiIfEnableByStatus(aiDma, aiIf, FALSE);
            if (ret != AIO_OK)
            {
                line = __LINE__;
                goto FAIL;
            }

            //
            g_pAiDmaStatusList[aiDma].eAiIf[i] = E_MHAL_AI_IF_NONE;
        }
    }

    //
    g_pAiDmaStatusList[aiDma].nAttachedChNum = 0;
    g_pAiDmaStatusList[aiDma].bIsAttached    = FALSE;

    //
    if (aiDma >= AI_DMA_DIRECT_START)
    {
        // Do nothing
    }
    else
    {
        // Dmic
        ret |= _MhalAudioAiDmicChNumCfg(aIsDigMicNeedUpdate); // After updating _MhalAudioAiDmicStatusUpdate()
        if (ret != AIO_OK)
        {
            line = __LINE__;
            goto FAIL;
        }
    }

    return AIO_OK;

FAIL:
    printf("%s[%d] enAiDma %d error\n", __FUNCTION__, line, enAiDma);
    return AIO_NG;
}

static int _MhalAudioAoDetachDma(MHAL_AO_Dma_e enAoDma)
{
    int      ret   = AIO_OK;
    int      i     = 0;
    AO_DMA_e aoDma = 0;
    AO_IF_e  aoIf  = 0;
    int      line  = 0;

    //
    aoDma = enAoDma;

    //
    if (aoDma >= AO_DMA_DIRECT_START)
    {
        if (aoDma >= E_MHAL_AO_DMA_TOTAL)
        {
            printf("%s[%d] aoDma direct %d error\n", __FUNCTION__, __LINE__, aoDma);
            line = __LINE__;
            goto FAIL;
        }
    }
    else
    {
        if (!CHIP_AO_DMA_VALID(aoDma))
        {
            printf("%s[%d] aoDma %d error\n", __FUNCTION__, __LINE__, aoDma);
            line = __LINE__;
            goto FAIL;
        }
    }

    //
    for (i = 0; i < E_MHAL_AO_DMA_CH_SLOT_TOTAL; i++)
    {
        aoIf = (g_pAoDmaStatusList[aoDma].eAoIf[i]);

        if (aoIf != E_MHAL_AO_IF_NONE)
        {
            //
            if (aoDma >= AO_DMA_DIRECT_START)
            {
                ret |= _MhalAudioAoDirectDettachCore(aoDma, i, aoIf);
            }
            else
            {
                ret |= _MhalAudioAoDettachCore(aoDma, i, aoIf);
            }

            if (ret != AIO_OK)
            {
                line = __LINE__;
                goto FAIL;
            }

            ret |= _MhalAudioAoIfStatusUpdateAndAction(aoIf, FALSE);
            if (ret != AIO_OK)
            {
                line = __LINE__;
                goto FAIL;
            }

            ret |= _MhalAudioAoIfEnableByStatus(aoDma, aoIf, FALSE);
            if (ret != AIO_OK)
            {
                line = __LINE__;
                goto FAIL;
            }

            //
            g_pAoDmaStatusList[aoDma].eAoIf[i] = E_MHAL_AO_IF_NONE;
        }
    }

    g_pAoDmaStatusList[aoDma].nAttachedChNum = 0;
    g_pAoDmaStatusList[aoDma].bIsAttached    = FALSE;

    return AIO_OK;

FAIL:
    printf("%s[%d] enAoDma %d error\n", __FUNCTION__, line, enAoDma);
    return AIO_NG;
}

// -------------------------------------------------------------------------------
MS_S32 MHAL_AUDIO_Init(void *pdata)
{
    int ret  = AIO_OK;
    int line = 0;

    AUD_PRINTF(MHAL_LEVEL, "in %s\n", __FUNCTION__);

    ret |= DrvAudInit();
    if (ret != AIO_OK)
    {
        line = __LINE__;

        goto FAIL;
    }
    AUD_PRINTF(MHAL_LEVEL, "in %s %d\n", __FUNCTION__, __LINE__);

    return MHAL_SUCCESS;

FAIL:
    printf("%s[%d] error\n", __FUNCTION__, line);
    return MHAL_FAILURE;
}

MS_S32 MHAL_AUDIO_DeInit(void)
{
    int ret  = AIO_OK;
    int line = 0;

    AUD_PRINTF(MHAL_LEVEL, "in %s\n", __FUNCTION__);

    // CamOsMutexLock(&g_tAudInitMutex);

    if (g_nInitialed == 1)
    {
        //
        ret |= AudioProcDeInit();
        if (ret != AIO_OK)
        {
            line = __LINE__;
            // CamOsMutexUnlock(&g_tAudInitMutex);
            goto FAIL;
        }

        //
        ret |= DrvAudDeInit();
        if (ret != AIO_OK)
        {
            line = __LINE__;
            // CamOsMutexUnlock(&g_tAudInitMutex);
            goto FAIL;
        }

        AUD_PRINTF(MHAL_LEVEL, "MHAL_AUDIO_DeInit \n", __FUNCTION__);
    }

    g_nInitialed--;

    // CamOsMutexUnlock(&g_tAudInitMutex);

    return MHAL_SUCCESS;

FAIL:
    printf("%s[%d] error\n", __FUNCTION__, line);
    return MHAL_FAILURE;
}

MS_S32 MHAL_AUDIO_AI_If_Setting(MHAL_AI_IF_e enAiIf, void *pAiSetting)
{
    int ret  = AIO_OK;
    int line = 0;

    //
    switch (enAiIf)
    {
        case E_MHAL_AI_IF_ADC_A_0_B_0:
            break;

        case E_MHAL_AI_IF_ADC_C_0_D_0:
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
            ret |= _MhalAudioAiConfigI2s(E_AI_DEV_I2S_RX_A, (MHAL_AUDIO_I2sCfg_t *)pAiSetting);
            break;

        case E_MHAL_AI_IF_I2S_RX_B_0_1:
        case E_MHAL_AI_IF_I2S_RX_B_2_3:
        case E_MHAL_AI_IF_I2S_RX_B_4_5:
        case E_MHAL_AI_IF_I2S_RX_B_6_7:
        case E_MHAL_AI_IF_I2S_RX_B_8_9:
        case E_MHAL_AI_IF_I2S_RX_B_10_11:
        case E_MHAL_AI_IF_I2S_RX_B_12_13:
        case E_MHAL_AI_IF_I2S_RX_B_14_15:
            ret |= _MhalAudioAiConfigI2s(E_AI_DEV_I2S_RX_B, (MHAL_AUDIO_I2sCfg_t *)pAiSetting);
            break;

        case E_MHAL_AI_IF_I2S_RX_C_0_1:
        case E_MHAL_AI_IF_I2S_RX_C_2_3:
        case E_MHAL_AI_IF_I2S_RX_C_4_5:
        case E_MHAL_AI_IF_I2S_RX_C_6_7:
        case E_MHAL_AI_IF_I2S_RX_C_8_9:
        case E_MHAL_AI_IF_I2S_RX_C_10_11:
        case E_MHAL_AI_IF_I2S_RX_C_12_13:
        case E_MHAL_AI_IF_I2S_RX_C_14_15:
            ret |= _MhalAudioAiConfigI2s(E_AI_DEV_I2S_RX_C, (MHAL_AUDIO_I2sCfg_t *)pAiSetting);
            break;

        case E_MHAL_AI_IF_I2S_RX_D_0_1:
        case E_MHAL_AI_IF_I2S_RX_D_2_3:
        case E_MHAL_AI_IF_I2S_RX_D_4_5:
        case E_MHAL_AI_IF_I2S_RX_D_6_7:
        case E_MHAL_AI_IF_I2S_RX_D_8_9:
        case E_MHAL_AI_IF_I2S_RX_D_10_11:
        case E_MHAL_AI_IF_I2S_RX_D_12_13:
        case E_MHAL_AI_IF_I2S_RX_D_14_15:
            ret |= _MhalAudioAiConfigI2s(E_AI_DEV_I2S_RX_D, (MHAL_AUDIO_I2sCfg_t *)pAiSetting);
            break;

        default:
            line = __LINE__;
            goto FAIL;
            break;
    }

    if (ret != AIO_OK)
    {
        line = __LINE__;
        goto FAIL;
    }

    return MHAL_SUCCESS;

FAIL:
    printf("%s[%d] error enAiIf is %d\n", __FUNCTION__, line, enAiIf);
    return MHAL_FAILURE;
}

MS_S32 MHAL_AUDIO_AO_If_Setting(MHAL_AO_IF_e enAoIf, void *pAoSetting)
{
    int ret  = AIO_OK;
    int line = 0;

    //
    switch (enAoIf)
    {
        case E_MHAL_AO_IF_DAC_A_0:
            break;

        case E_MHAL_AO_IF_DAC_B_0:
            break;

        case E_MHAL_AO_IF_DAC_C_0:
            break;

        case E_MHAL_AO_IF_DAC_D_0:
            break;

        case E_MHAL_AO_IF_HDMI_A_0:
            break;

        case E_MHAL_AO_IF_HDMI_A_1:
            break;

        case E_MHAL_AO_IF_ECHO_A_0:
            break;

        case E_MHAL_AO_IF_ECHO_A_1:
            break;

        case E_MHAL_AO_IF_I2S_TX_A_0:
        case E_MHAL_AO_IF_I2S_TX_A_1:
            ret |= _MhalAudioAoConfigI2s(E_AO_DEV_I2S_TX_A, (MHAL_AUDIO_I2sCfg_t *)pAoSetting);
            break;

        case E_MHAL_AO_IF_I2S_TX_B_0:
        case E_MHAL_AO_IF_I2S_TX_B_1:
            ret |= _MhalAudioAoConfigI2s(E_AO_DEV_I2S_TX_B, (MHAL_AUDIO_I2sCfg_t *)pAoSetting);
            break;

        default:
            line = __LINE__;
            goto FAIL;
            break;
    }

    if (ret != AIO_OK)
    {
        line = __LINE__;
        goto FAIL;
    }

    return MHAL_SUCCESS;

FAIL:
    printf("%s[%d] error enAoIf is %d\n", __FUNCTION__, line, enAoIf);
    return MHAL_FAILURE;
}

MS_S32 MHAL_AUDIO_AI_If_Gain(MHAL_AI_IF_e enAiIf, MS_S8 s8LeftIfGain, MS_S8 s8RightIfGain)
{
    int     ret    = AIO_OK;
    AI_IF_e aiIf   = 0;
    AI_CH_e eAiCh1 = 0, eAiCh2 = 0;
    int     line = 0;

    //
    aiIf = enAiIf;

    //
    ret |= _MhalAudioAiIfToDevCh(aiIf, &eAiCh1, &eAiCh2);
    if (ret != AIO_OK)
    {
        line = __LINE__;
        goto FAIL;
    }
    ret |= DrvAudAiIfSetGain(eAiCh1, (S16)s8LeftIfGain);
    if (ret != AIO_OK)
    {
        line = __LINE__;
        goto FAIL;
    }

    ret |= DrvAudAiIfSetGain(eAiCh2, (S16)s8RightIfGain);
    if (ret != AIO_OK)
    {
        line = __LINE__;
        goto FAIL;
    }

    return MHAL_SUCCESS;

FAIL:
    printf("%s[%d] enAiIf %d s8LeftIfGain[%d] s8RightIfGain error\n", __FUNCTION__, line, enAiIf, s8LeftIfGain);
    return MHAL_FAILURE;
}

MS_S32 MHAL_AUDIO_AO_If_Gain(MHAL_AO_IF_e enAoIf, MS_S8 s8IfGain)
{
    int     ret  = AIO_OK;
    int     i    = 0;
    AO_IF_e aoIf = 0;
    AO_CH_e aeAoCh[AO_IF_IN_ONE_DMA_CH_MAX];
    MS_U8   nGetNum = 0;
    int     line    = 0;

    //
    aoIf = enAoIf;

    //
    ret |= _MhalAudioAoIfToDevCh(aoIf, aeAoCh, &nGetNum);
    if (ret != AIO_OK)
    {
        line = __LINE__;
        goto FAIL;
    }

    // nGetNum is supposed to be 1
    for (i = 0; i < nGetNum; i++)
    {
        ret |= DrvAudAoIfSetGain(aeAoCh[i], (S16)s8IfGain);
        if (ret != AIO_OK)
        {
            line = __LINE__;
            goto FAIL;
        }
    }

    return MHAL_SUCCESS;

FAIL:
    printf("%s[%d] enAoIf %d s8IfGain[%d] error\n", __FUNCTION__, line, enAoIf, s8IfGain);
    return MHAL_FAILURE;
}

MS_S32 MHAL_AUDIO_AI_If_Mute(MHAL_AI_IF_e enAiIf, MS_BOOL bLeftEnable, MS_BOOL bRightEnable)
{
    int     ret    = AIO_OK;
    AI_IF_e aiIf   = 0;
    AI_CH_e eAiCh1 = 0, eAiCh2 = 0;
    int     line = 0;

    //
    aiIf = enAiIf;

    //
    ret |= _MhalAudioAiIfToDevCh(aiIf, &eAiCh1, &eAiCh2);
    if (ret != AIO_OK)
    {
        line = __LINE__;
        goto FAIL;
    }

    ret |= DrvAudAiIfSetMute(eAiCh1, bLeftEnable);
    if (ret != AIO_OK)
    {
        line = __LINE__;
        goto FAIL;
    }

    ret |= DrvAudAiIfSetMute(eAiCh2, bRightEnable);
    if (ret != AIO_OK)
    {
        line = __LINE__;
        goto FAIL;
    }

    return MHAL_SUCCESS;

FAIL:
    printf("%s[%d] enAiIf %d error\n", __FUNCTION__, line, enAiIf);
    return MHAL_FAILURE;
}

MS_S32 MHAL_AUDIO_AO_If_Mute(MHAL_AO_IF_e enAoIf, MS_BOOL bEnable)
{
    int     ret  = AIO_OK;
    int     i    = 0;
    AO_IF_e aoIf = 0;
    AO_CH_e aeAoCh[AO_IF_IN_ONE_DMA_CH_MAX];
    MS_U8   nGetNum = 0;
    int     line    = 0;

    //
    aoIf = enAoIf;

    //
    ret |= _MhalAudioAoIfToDevCh(aoIf, aeAoCh, &nGetNum);
    if (ret != AIO_OK)
    {
        line = __LINE__;
        goto FAIL;
    }

    // nGetNum is supposed to be 1
    for (i = 0; i < nGetNum; i++)
    {
        ret |= DrvAudAoIfSetMute(aeAoCh[i], bEnable);
        if (ret != AIO_OK)
        {
            line = __LINE__;
            goto FAIL;
        }
    }

    return MHAL_SUCCESS;

FAIL:
    printf("%s[%d] enAoIf %d error\n", __FUNCTION__, line, enAoIf);
    return MHAL_FAILURE;
}

MS_S32 MHAL_AUDIO_AI_Dpga_Gain(MHAL_AI_Dma_e enAiDma, MS_U8 u8ChId, MS_S8 s8Gain)
{
    int                 ret         = AIO_OK;
    AI_DMA_e            aiDma       = 0;
    AI_IF_e             aiIf        = 0;
    MHAL_AI_DmaChSlot_e aiDmaChSlot = 0;
    AI_CH_e             eAiCh1 = 0, eAiCh2 = 0, eAiCh = 0;
    CHIP_AIO_DMA_e      eAioDma = 0;
    int                 line    = 0;

    //
    aiDma = enAiDma;

    if (!CHIP_AI_DMA_VALID(aiDma))
    {
        line = __LINE__;
        goto FAIL;
    }

    if (g_pAiDmaStatusList[aiDma].bIsAttached == FALSE)
    {
        printf("%s, aiDma = %d, bIsAttached = FALSE\n", __FUNCTION__, aiDma);
        line = __LINE__;
        goto FAIL;
    }

    if (u8ChId >= 8)
    {
        line = __LINE__;
        goto FAIL;
    }

    aiDmaChSlot = u8ChId / AI_DMA_CH_NUM_PER_SLOT;

    aiIf = g_pAiDmaStatusList[aiDma].eAiIf[aiDmaChSlot];

    //
    ret |= _MhalAudioAiIfToDevCh(aiIf, &eAiCh1, &eAiCh2);
    if (ret != AIO_OK)
    {
        line = __LINE__;
        goto FAIL;
    }

    if (u8ChId % 2)
    {
        eAiCh = eAiCh2;
    }
    else
    {
        eAiCh = eAiCh1;
    }

    //
    eAioDma = CHIP_AIO_DMA_IDX_BY_AI_DMA(aiDma);
    if (!CHIP_AIO_DMA_IDX_VALID(eAioDma))
    {
        line = __LINE__;
        goto FAIL;
    }

    //
    ret |= DrvAudAiSetDpgaGain(eAioDma, u8ChId, eAiCh, (S16)s8Gain);

    if (ret != AIO_OK)
    {
        line = __LINE__;
        goto FAIL;
    }

    return MHAL_SUCCESS;

FAIL:
    printf("%s[%d] enAiDma[%d] u8ChId[%d] s8Gain[%d] error \n", __FUNCTION__, line, enAiDma, u8ChId, s8Gain);
    return MHAL_FAILURE;
}

MS_S32 MHAL_AUDIO_AO_Dpga_Gain(MHAL_AO_Dma_e enAoDma, MS_U8 u8ChId, MS_S8 s8Gain, MHAL_AUDIO_GainFading_e enFading)
{
    int                 ret         = AIO_OK;
    int                 i           = 0;
    AO_DMA_e            aoDma       = 0;
    AO_IF_e             aoIf        = 0;
    MHAL_AO_DmaChSlot_e aoDmaChSlot = 0;
    CHIP_AIO_DMA_e      eAioDma     = 0;
    AO_CH_e             aeAoCh[AO_IF_IN_ONE_DMA_CH_MAX];
    MS_U8               nGetNum = 0;
    int                 line    = 0;

    //
    aoDma = enAoDma;

    if (!CHIP_AO_DMA_VALID(aoDma))
    {
        line = __LINE__;
        goto FAIL;
    }

    if (g_pAoDmaStatusList[aoDma].bIsAttached == FALSE)
    {
        printf("%s, aoDma = %d, bIsAttached = FALSE\n", __FUNCTION__, aoDma);
        line = __LINE__;
        goto FAIL;
    }

    if (u8ChId >= 2)
    {
        line = __LINE__;
        goto FAIL;
    }

    aoDmaChSlot = u8ChId / AO_DMA_CH_NUM_PER_SLOT;

    aoIf = g_pAoDmaStatusList[aoDma].eAoIf[aoDmaChSlot];

    //
    eAioDma = CHIP_AIO_DMA_IDX_BY_AO_DMA(aoDma);
    if (!CHIP_AIO_DMA_IDX_VALID(eAioDma))
    {
        line = __LINE__;
        goto FAIL;
    }

    //
    ret |= _MhalAudioAoIfToDevCh(aoIf, aeAoCh, &nGetNum);
    if (ret != AIO_OK)
    {
        line = __LINE__;
        goto FAIL;
    }

    for (i = 0; i < nGetNum; i++)
    {
        ret |= DrvAudAoSetDpgaGain(eAioDma, u8ChId, aeAoCh[i], (S16)s8Gain, (U8)enFading);
        if (ret != AIO_OK)
        {
            line = __LINE__;
            goto FAIL;
        }
    }

    return MHAL_SUCCESS;

FAIL:
    printf("%s[%d] enAoDma %d u8ChId[%d] s8Gain[%d] enFading[%d] error\n", __FUNCTION__, line, enAoDma, u8ChId, s8Gain,
           enFading);
    return MHAL_FAILURE;
}

MS_S32 MHAL_AUDIO_AI_Dpga_Mute(MHAL_AI_Dma_e enAiDma, MS_U8 u8ChId, MS_BOOL bEnable)
{
    int                 ret         = AIO_OK;
    AI_DMA_e            aiDma       = 0;
    AI_IF_e             aiIf        = 0;
    MHAL_AI_DmaChSlot_e aiDmaChSlot = 0;
    AI_CH_e             eAiCh1 = 0, eAiCh2 = 0, eAiCh = 0;
    CHIP_AIO_DMA_e      eAioDma = 0;
    int                 line    = 0;

    //
    aiDma = enAiDma;

    if (!CHIP_AI_DMA_VALID(aiDma))
    {
        line = __LINE__;
        goto FAIL;
    }

    if (g_pAiDmaStatusList[aiDma].bIsAttached == FALSE)
    {
        printf("%s, aiDma = %d, bIsAttached = FALSE\n", __FUNCTION__, aiDma);
        line = __LINE__;
        goto FAIL;
    }

    if (u8ChId >= 8)
    {
        line = __LINE__;
        goto FAIL;
    }

    aiDmaChSlot = u8ChId / AI_DMA_CH_NUM_PER_SLOT;

    aiIf = g_pAiDmaStatusList[aiDma].eAiIf[aiDmaChSlot];

    //
    ret |= _MhalAudioAiIfToDevCh(aiIf, &eAiCh1, &eAiCh2);
    if (ret != AIO_OK)
    {
        line = __LINE__;
        goto FAIL;
    }

    if (u8ChId % 2)
    {
        eAiCh = eAiCh2;
    }
    else
    {
        eAiCh = eAiCh1;
    }

    //
    eAioDma = CHIP_AIO_DMA_IDX_BY_AI_DMA(aiDma);
    if (!CHIP_AIO_DMA_IDX_VALID(eAioDma))
    {
        line = __LINE__;
        goto FAIL;
    }

    //
    ret |= DrvAudAiSetDpgaMute(eAioDma, u8ChId, eAiCh, bEnable);

    if (ret != AIO_OK)
    {
        line = __LINE__;
        goto FAIL;
    }

    return MHAL_SUCCESS;

FAIL:
    printf("%s[%d] enAiDma %d u8ChId[%d] bEnable[%d] error\n", __FUNCTION__, line, enAiDma, u8ChId, bEnable);
    return MHAL_FAILURE;
}

MS_S32 MHAL_AUDIO_AO_Dpga_Mute(MHAL_AO_Dma_e enAoDma, MS_U8 u8ChId, MS_BOOL bEnable, MHAL_AUDIO_GainFading_e enFading)
{
    int                 ret         = AIO_OK;
    int                 i           = 0;
    AO_DMA_e            aoDma       = 0;
    AO_IF_e             aoIf        = 0;
    MHAL_AO_DmaChSlot_e aoDmaChSlot = 0;
    CHIP_AIO_DMA_e      eAioDma     = 0;
    AO_CH_e             aeAoCh[AO_IF_IN_ONE_DMA_CH_MAX];
    MS_U8               nGetNum = 0;
    int                 line    = 0;

    //
    aoDma = enAoDma;

    if (!CHIP_AO_DMA_VALID(aoDma))
    {
        line = __LINE__;
        goto FAIL;
    }

    if (g_pAoDmaStatusList[aoDma].bIsAttached == FALSE)
    {
        printf("%s, aoDma = %d, bIsAttached = FALSE\n", __FUNCTION__, aoDma);
        line = __LINE__;
        goto FAIL;
    }

    if (u8ChId >= 2)
    {
        line = __LINE__;
        goto FAIL;
    }

    aoDmaChSlot = u8ChId / AO_DMA_CH_NUM_PER_SLOT;

    aoIf = g_pAoDmaStatusList[aoDma].eAoIf[aoDmaChSlot];

    //
    eAioDma = CHIP_AIO_DMA_IDX_BY_AO_DMA(aoDma);
    if (!CHIP_AIO_DMA_IDX_VALID(eAioDma))
    {
        line = __LINE__;
        goto FAIL;
    }

    //
    ret |= _MhalAudioAoIfToDevCh(aoIf, aeAoCh, &nGetNum);
    if (ret != AIO_OK)
    {
        line = __LINE__;
        goto FAIL;
    }

    for (i = 0; i < nGetNum; i++)
    {
        ret |= DrvAudAoSetDpgaMute(eAioDma, u8ChId, aeAoCh[i], bEnable, enFading);
        if (ret != AIO_OK)
        {
            line = __LINE__;
            goto FAIL;
        }
    }

    return MHAL_SUCCESS;

FAIL:
    printf("%s[%d] enAoDma %d u8ChId[%d] bEnable[%d] error\n", __FUNCTION__, line, enAoDma, u8ChId, bEnable);
    return MHAL_FAILURE;
}

MS_S32 MHAL_AUDIO_AI_Config(MHAL_AI_Dma_e enAiDma, MHAL_AUDIO_PcmCfg_t *pstDmaConfig)
{
    int            ret = AIO_OK;
    DmaParam_t     tParam;
    MS_U16         nBitWidth;
    CHIP_AIO_DMA_e eAioDma = 0;
    AI_DMA_e       aiDma   = 0;
    int            line    = 0;

    //
    aiDma = enAiDma;

    //
    if (aiDma >= AI_DMA_DIRECT_START)
    {
        if (aiDma >= E_MHAL_AI_DMA_TOTAL)
        {
            printf("%s[%d] aiDma direct %d error\n", __FUNCTION__, __LINE__, aiDma);
            line = __LINE__;
            goto FAIL;
        }
    }
    else
    {
        if (!CHIP_AI_DMA_VALID(aiDma))
        {
            printf("%s[%d] aiDma %d error\n", __FUNCTION__, __LINE__, aiDma);
            line = __LINE__;
            goto FAIL;
        }
    }

    //
    g_pAiDmaStatusList[aiDma].nSampleRate = pstDmaConfig->enRate;

    //
    if (aiDma >= AI_DMA_DIRECT_START)
    {
    }
    else
    {
        eAioDma = CHIP_AIO_DMA_IDX_BY_AI_DMA(aiDma);
        if (!CHIP_AIO_DMA_IDX_VALID(eAioDma))
        {
            line = __LINE__;
            goto FAIL;
        }

        //
        switch (pstDmaConfig->enWidth)
        {
            case E_MHAL_AUDIO_BITWIDTH_16:
                nBitWidth = 16;
                break;
            case E_MHAL_AUDIO_BITWIDTH_32:
                nBitWidth = 32;
                break;
            default:
                line = __LINE__;
                goto FAIL;
        }

        //
        tParam.pDmaBuf       = pstDmaConfig->pu8DmaArea;
        tParam.bIsOnlyEvenCh = pstDmaConfig->u8IsOnlyEvenCh;
        tParam.nPhysDmaAddr  = pstDmaConfig->phyDmaAddr;
        tParam.nBufferSize   = pstDmaConfig->u32BufferSize;
        tParam.nChannels     = pstDmaConfig->u16Channels;
        tParam.nPeriodSize   = pstDmaConfig->u32PeriodSize;
        tParam.nBitWidth     = nBitWidth;
        tParam.nSampleRate   = pstDmaConfig->enRate;
        tParam.nInterleaved  = pstDmaConfig->bInterleaved;
        tParam.private       = NULL;

        if ((tParam.nChannels != 1) && (tParam.nChannels != 2) && (tParam.nChannels != 4) && (tParam.nChannels != 8))
        {
            printf("%s, enAiDma = %d, AttachedChNum %d not support\n", __FUNCTION__, enAiDma, tParam.nChannels);
            line = __LINE__;
            goto FAIL;
        }

        if (tParam.bIsOnlyEvenCh == TRUE)
        {
            ret |= DrvAudMonoEnable(eAioDma, TRUE, FALSE);
            if (ret != AIO_OK)
            {
                line = __LINE__;
                goto FAIL;
            }
        }
        else
        {
            ret |= DrvAudMonoEnable(eAioDma, FALSE, FALSE);
            if (ret != AIO_OK)
            {
                line = __LINE__;
                goto FAIL;
            }
        }

        // Dma
        ret |= DrvAudConfigDmaParam(eAioDma, &tParam);
        if (ret != AIO_OK)
        {
            line = __LINE__;
            goto FAIL;
        }
    }

    return MHAL_SUCCESS;

FAIL:
    printf("%s[%d] enAiDma[%d] error\n", __FUNCTION__, line, enAiDma);
    return MHAL_FAILURE;
}

MS_S32 MHAL_AUDIO_AO_Config(MHAL_AO_Dma_e enAoDma, MHAL_AUDIO_PcmCfg_t *pstDmaConfig)
{
    int ret = AIO_OK;
    // int i = 0;
    DmaParam_t     tParam;
    MS_U16         nBitWidth;
    CHIP_AIO_DMA_e eAioDma = 0;
    AO_DMA_e       aoDma   = 0;
    int            line    = 0;

    //
    aoDma = enAoDma;

    //
    if (aoDma >= AO_DMA_DIRECT_START)
    {
        if (aoDma >= E_MHAL_AO_DMA_TOTAL)
        {
            printf("%s[%d] aoDma direct %d error\n", __FUNCTION__, __LINE__, aoDma);
            line = __LINE__;
            goto FAIL;
        }
    }
    else
    {
        if (!CHIP_AO_DMA_VALID(aoDma))
        {
            printf("%s[%d] aoDma %d error\n", __FUNCTION__, __LINE__, aoDma);
            line = __LINE__;
            goto FAIL;
        }
    }

    //
    g_pAoDmaStatusList[aoDma].nSampleRate = pstDmaConfig->enRate;

    //
    if (aoDma >= AO_DMA_DIRECT_START)
    {
    }
    else
    {
        eAioDma = CHIP_AIO_DMA_IDX_BY_AO_DMA(aoDma);
        if (!CHIP_AIO_DMA_IDX_VALID(eAioDma))
        {
            line = __LINE__;
            goto FAIL;
        }

        //
        switch (pstDmaConfig->enWidth)
        {
            case E_MHAL_AUDIO_BITWIDTH_16:
                nBitWidth = 16;
                break;
            case E_MHAL_AUDIO_BITWIDTH_32:
                nBitWidth = 32;
                break;
            default:
                return E_MHAL_ERR_ILLEGAL_PARAM;
        }

        //
        tParam.pDmaBuf       = pstDmaConfig->pu8DmaArea;
        tParam.bIsOnlyEvenCh = pstDmaConfig->u8IsOnlyEvenCh;
        tParam.nPhysDmaAddr  = pstDmaConfig->phyDmaAddr; //!!!MIU_OFFSET
        tParam.nBufferSize   = pstDmaConfig->u32BufferSize;
        tParam.nChannels     = pstDmaConfig->u16Channels;
        tParam.nBitWidth     = nBitWidth;
        tParam.nSampleRate   = pstDmaConfig->enRate;
        tParam.nPeriodSize   = pstDmaConfig->u32PeriodSize;
        tParam.nInterleaved  = pstDmaConfig->bInterleaved;

        //
        if (tParam.nChannels == 1)
        {
            if (pstDmaConfig->enChMode == E_MHAL_AO_CH_MODE_DOUBLE_MONO)
            {
                ret |= DrvAudMonoEnable(eAioDma, TRUE, TRUE);
            }
            else
            {
                ret |= DrvAudMonoEnable(eAioDma, TRUE, FALSE);
            }

            if (ret != AIO_OK)
            {
                line = __LINE__;
                goto FAIL;
            }
        }
        else
        {
            ret |= DrvAudMonoEnable(eAioDma, FALSE, FALSE);
            if (ret != AIO_OK)
            {
                line = __LINE__;
                goto FAIL;
            }
        }

        //
        ret |= DrvAudConfigDmaParam(eAioDma, &tParam);
        if (ret != AIO_OK)
        {
            line = __LINE__;
            goto FAIL;
        }
    }

    return MHAL_SUCCESS;

FAIL:
    printf("%s[%d] aiDma %d error\n", __FUNCTION__, line, aoDma);
    return MHAL_FAILURE;
}

MS_S32 MHAL_AUDIO_AI_Open(MHAL_AI_Dma_e enAiDma)
{
    int            ret     = AIO_OK;
    CHIP_AIO_DMA_e eAioDma = 0;
    AI_DMA_e       aiDma   = 0;
    AI_IF_e        eAiIf   = 0;
    int            line    = 0;

    //
    aiDma = enAiDma;

    //
    if (aiDma >= AI_DMA_DIRECT_START)
    {
        if (aiDma >= E_MHAL_AI_DMA_TOTAL)
        {
            printf("%s[%d] aiDma direct %d error\n", __FUNCTION__, __LINE__, aiDma);
            line = __LINE__;
            goto FAIL;
        }
    }
    else
    {
        if (!CHIP_AI_DMA_VALID(aiDma))
        {
            printf("%s[%d] aiDma %d error\n", __FUNCTION__, __LINE__, aiDma);
            line = __LINE__;
            goto FAIL;
        }
    }

    //
    if (aiDma >= AI_DMA_DIRECT_START)
    {
        eAiIf = g_pAiDmaStatusList[aiDma].eAiIf[0];

        ret |= DrvAudAiSetIfSampleRate(eAiIf, g_pAiDmaStatusList[aiDma].nSampleRate);
        if (ret != AIO_OK)
        {
            line = __LINE__;
            goto FAIL;
        }
    }
    else
    {
        eAioDma = CHIP_AIO_DMA_IDX_BY_AI_DMA(aiDma);
        if (!CHIP_AIO_DMA_IDX_VALID(eAioDma))
        {
            line = __LINE__;
            goto FAIL;
        }

        // Open
        ret |= DrvAudOpenDma(eAioDma);
        if (ret != AIO_OK)
        {
            line = __LINE__;
            goto FAIL;
        }

        // Prepare
        ret |= DrvAudPrepareDma(eAioDma);
        if (ret != AIO_OK)
        {
            line = __LINE__;
            goto FAIL;
        }
    }

    return MHAL_SUCCESS;

FAIL:
    printf("%s[%d] enAiDma %d error\n", __FUNCTION__, line, enAiDma);
    return MHAL_FAILURE;
}

MS_S32 MHAL_AUDIO_AO_Open(MHAL_AO_Dma_e enAoDma)
{
    int            ret     = AIO_OK;
    CHIP_AIO_DMA_e eAioDma = 0;
    AO_DMA_e       aoDma   = 0;
    AO_IF_e        eAoIf   = 0;
    int            line    = 0;
    //
    aoDma = enAoDma;

    //
    if (aoDma >= AO_DMA_DIRECT_START)
    {
        if (aoDma >= E_MHAL_AO_DMA_TOTAL)
        {
            printf("%s[%d] aoDma direct %d error\n", __FUNCTION__, __LINE__, aoDma);
            line = __LINE__;
            goto FAIL;
        }
    }
    else
    {
        if (!CHIP_AO_DMA_VALID(aoDma))
        {
            printf("%s[%d] aoDma %d error\n", __FUNCTION__, __LINE__, aoDma);
            line = __LINE__;
            goto FAIL;
        }
    }

    //
    if (aoDma >= AO_DMA_DIRECT_START)
    {
        eAoIf = g_pAoDmaStatusList[aoDma].eAoIf[0];

        ret |= DrvAudAoSetIfSampleRate(eAoIf, g_pAoDmaStatusList[aoDma].nSampleRate);
        if (ret != AIO_OK)
        {
            line = __LINE__;
            goto FAIL;
        }
    }
    else
    {
        eAioDma = CHIP_AIO_DMA_IDX_BY_AO_DMA(aoDma);
        if (!CHIP_AIO_DMA_IDX_VALID(eAioDma))
        {
            line = __LINE__;
            goto FAIL;
        }

        // Open
        ret |= DrvAudOpenDma(eAioDma);
        if (ret != AIO_OK)
        {
            line = __LINE__;
            goto FAIL;
        }

        // Prepare
        ret |= DrvAudPrepareDma(eAioDma);
        if (ret != AIO_OK)
        {
            line = __LINE__;
            goto FAIL;
        }
    }

    return MHAL_SUCCESS;

FAIL:
    printf("%s[%d] enAoDma %d error\n", __FUNCTION__, line, enAoDma);
    return MHAL_FAILURE;
}

MS_S32 MHAL_AUDIO_AI_Attach(MHAL_AI_Attach_t *pAiAttach)
{
    int      ret                                    = AIO_OK;
    int      i                                      = 0;
    AI_DMA_e aiDma                                  = 0;
    AI_IF_e  aiIf                                   = 0;
    BOOL     aIsDigMicNeedUpdate[AI_DEV_DMIC_TOTAL] = {0};

    AI_ATTACH_t aiAttach;
    int         line = 0;

    //
    aiDma = pAiAttach->enAiDma;

    if (aiDma >= AI_DMA_DIRECT_START)
    {
        if (aiDma >= E_MHAL_AI_DMA_TOTAL)
        {
            printf("%s[%d] aiDma direct %d error\n", __FUNCTION__, __LINE__, aiDma);
            line = __LINE__;
            goto FAIL;
        }
    }
    else
    {
        if (!CHIP_AI_DMA_VALID(aiDma))
        {
            printf("%s[%d] aiDma %d error\n", __FUNCTION__, __LINE__, aiDma);
            line = __LINE__;
            goto FAIL;
        }
    }

    //
    for (i = 0; i < E_MHAL_AI_DMA_CH_SLOT_TOTAL; i++)
    {
        aiIf = pAiAttach->aenAiIf[i];

        if (aiIf != E_MHAL_AI_IF_NONE)
        {
            //
            if (aiDma >= AI_DMA_DIRECT_START)
            {
                ret |= _MhalAudioAiDirectAttachCore(aiDma, i, aiIf);
                if (ret != AIO_OK)
                {
                    line = __LINE__;
                    goto FAIL;
                }
            }
            else
            {
                ret |= _MhalAudioAiAttachCore(aiDma, i, aiIf);
                if (ret != AIO_OK)
                {
                    line = __LINE__;
                    goto FAIL;
                }
            }

            // AI DMIC Status Update
            ret |= _MhalAudioAiDmicStatusUpdate(aiIf, TRUE, aIsDigMicNeedUpdate);
            if (ret != AIO_OK)
            {
                line = __LINE__;
                goto FAIL;
            }

            //
            ret |= _MhalAudioAiIfStatusUpdateAndAction(aiIf, TRUE);
            if (ret != AIO_OK)
            {
                line = __LINE__;
                goto FAIL;
            }

            //
            ret |= _MhalAudioAiIfEnableByStatus(aiDma, aiIf, TRUE);
            if (ret != AIO_OK)
            {
                line = __LINE__;
                goto FAIL;
            }

            //
            g_pAiDmaStatusList[aiDma].nAttachedChNum = (i + 1) * AI_DMA_CH_NUM_PER_SLOT;

            g_pAiDmaStatusList[aiDma].bIsAttached = TRUE;

            g_pAiDmaStatusList[aiDma].eAiIf[i] = aiIf;
        }
    }

    // Dmic
    ret |= _MhalAudioAiDmicSampleRateCfgByDma(aiDma, aIsDigMicNeedUpdate);
    if (ret != AIO_OK)
    {
        line = __LINE__;
        goto FAIL;
    }

    ret |= _MhalAudioAiDmicChNumCfg(aIsDigMicNeedUpdate); // After updating _MhalAudioAiDmicStatusUpdate()
    if (ret != AIO_OK)
    {
        line = __LINE__;
        goto FAIL;
    }

    //
    if (aiDma >= AI_DMA_DIRECT_START)
    {
        // Do nothing so far
    }
    else
    {
        // Src
        ret |= _MhalAudioAiEchoCfgByDma(aiDma);
        if (ret != AIO_OK)
        {
            line = __LINE__;
            goto FAIL;
        }

        // Mch
        aiAttach.enAiDma = aiDma;

        for (i = 0; i < E_MHAL_AI_DMA_CH_SLOT_TOTAL; i++)
        {
            aiAttach.eAiIf[i] = g_pAiDmaStatusList[aiDma].eAiIf[i];
        }

        ret |= DrvAudAiSetClkRefAndMch(&aiAttach);
        if (ret != AIO_OK)
        {
            line = __LINE__;
            goto FAIL;
        }
    }

    return MHAL_SUCCESS;

FAIL:
    printf("%s[%d] error\n", __FUNCTION__, line);
    return MHAL_FAILURE;
}

MS_S32 MHAL_AUDIO_AO_Attach(MHAL_AO_Attach_t *pAoAttach)
{
    int         ret      = AIO_OK;
    int         i        = 0;
    AO_DMA_e    aoDma    = 0;
    AO_IF_e     aoIf     = 0;
    MS_U8       nAoDmaCh = 0;
    AO_ATTACH_t aoAttach;
    int         line = 0;
    //
    aoDma = pAoAttach->enAoDma;

    if (aoDma >= AO_DMA_DIRECT_START)
    {
        if (aoDma >= E_MHAL_AO_DMA_TOTAL)
        {
            printf("%s[%d] aoDma direct %d error\n", __FUNCTION__, __LINE__, aoDma);
            line = __LINE__;
            goto FAIL;
        }
    }
    else
    {
        if (!CHIP_AO_DMA_VALID(aoDma))
        {
            printf("%s[%d] aoDma %d error\n", __FUNCTION__, __LINE__, aoDma);
            line = __LINE__;
            goto FAIL;
        }
    }

    //
    nAoDmaCh = pAoAttach->nAoDmaCh;
    aoIf     = pAoAttach->enAoIf;
    aoIf &= ~(g_pAoDmaStatusList[aoDma].eAoIf[nAoDmaCh]);

    //
    if (aoIf != E_MHAL_AO_IF_NONE)
    {
        if (aoDma >= AO_DMA_DIRECT_START)
        {
            ret |= _MhalAudioAoDirectAttachCore(aoDma, nAoDmaCh, aoIf);
        }
        else
        {
            ret |= _MhalAudioAoAttachCore(aoDma, nAoDmaCh, aoIf);
        }

        if (ret != AIO_OK)
        {
            line = __LINE__;
            goto FAIL;
        }

        ret |= _MhalAudioAoIfStatusUpdateAndAction(aoIf, TRUE);
        if (ret != AIO_OK)
        {
            line = __LINE__;
            goto FAIL;
        }

        //
        ret |= _MhalAudioAoIfEnableByStatus(aoDma, aoIf, TRUE);
        if (ret != AIO_OK)
        {
            line = __LINE__;
            goto FAIL;
        }

        //
        if (g_pAoDmaStatusList[aoDma].eAoIf[nAoDmaCh] == E_MHAL_AO_IF_NONE)
        {
            g_pAoDmaStatusList[aoDma].nAttachedChNum = g_pAoDmaStatusList[aoDma].nAttachedChNum + 1;

            g_pAoDmaStatusList[aoDma].bIsAttached = TRUE;
        }

        g_pAoDmaStatusList[aoDma].eAoIf[nAoDmaCh] |= aoIf;
    }

    if (aoDma >= AO_DMA_DIRECT_START)
    {
        // Do nothing
    }
    else
    {
        //
        aoAttach.enAoDma = aoDma;

        for (i = 0; i < E_MHAL_AO_DMA_CH_SLOT_TOTAL; i++)
        {
            aoAttach.eAoIf[i] = g_pAoDmaStatusList[aoDma].eAoIf[i];
        }

        ret |= DrvAudAoSetClkRef(&aoAttach);
        if (ret != AIO_OK)
        {
            line = __LINE__;
            goto FAIL;
        }
    }

    return MHAL_SUCCESS;

FAIL:
    printf("%s[%d] error\n", __FUNCTION__, line);
    return MHAL_FAILURE;
}

MS_S32 MHAL_AUDIO_AO_Detach(MHAL_AO_Attach_t *pAoAttach)
{
    int         ret      = AIO_OK;
    AO_DMA_e    aoDma    = 0;
    AO_IF_e     aoIf     = 0;
    MS_U8       nAoDmaCh = 0;
    int         line     = 0;
    int         bSetting = 0;
    int         i        = 0;
    AO_ATTACH_t aoAttach;

    //
    aoDma = pAoAttach->enAoDma;

    if (aoDma >= AO_DMA_DIRECT_START)
    {
    }
    else
    {
        if (!CHIP_AO_DMA_VALID(aoDma))
        {
            printf("%s[%d] aoDma %d error\n", __FUNCTION__, __LINE__, aoDma);
            line = __LINE__;
            goto FAIL;
        }
    }

    //
    nAoDmaCh = pAoAttach->nAoDmaCh;

    aoIf = pAoAttach->enAoIf;

    aoIf &= (g_pAoDmaStatusList[aoDma].eAoIf[nAoDmaCh]);

    if (aoIf != E_MHAL_AO_IF_NONE)
    {
        //
        if (aoDma >= AO_DMA_DIRECT_START)
        {
            ret |= _MhalAudioAoDirectDettachCore(aoDma, nAoDmaCh, aoIf);
        }
        else
        {
            ret |= _MhalAudioAoDettachCore(aoDma, nAoDmaCh, aoIf);
        }

        if (ret != AIO_OK)
        {
            line = __LINE__;
            goto FAIL;
        }

        ret |= _MhalAudioAoIfStatusUpdateAndAction(aoIf, FALSE);
        if (ret != AIO_OK)
        {
            line = __LINE__;
            goto FAIL;
        }

        //
        ret |= _MhalAudioAoIfEnableByStatus(aoDma, aoIf, FALSE);
        if (ret != AIO_OK)
        {
            line = __LINE__;
            goto FAIL;
        }

        //
        g_pAoDmaStatusList[aoDma].eAoIf[nAoDmaCh] &= ~aoIf;

        if (g_pAoDmaStatusList[aoDma].eAoIf[nAoDmaCh] == E_MHAL_AO_IF_NONE)
        {
            if (g_pAoDmaStatusList[aoDma].nAttachedChNum > 0)
            {
                g_pAoDmaStatusList[aoDma].nAttachedChNum = g_pAoDmaStatusList[aoDma].nAttachedChNum - 1;

                if (g_pAoDmaStatusList[aoDma].nAttachedChNum == 0)
                {
                    g_pAoDmaStatusList[aoDma].bIsAttached = FALSE;
                }
            }
            else
            {
                // It should not be here.
                line = __LINE__;
                goto FAIL;
            }
        }

        aoAttach.enAoDma = aoDma;

        for (i = 0; i < E_MHAL_AO_DMA_CH_SLOT_TOTAL; i++)
        {
            aoAttach.eAoIf[i] = aoIf;
        }
        _MhalAudioAoCheckIfDoSettingByStatus(aoIf, FALSE, &bSetting);
        if (bSetting)
        {
            DrvAudAoSetClkUnRef(&aoAttach);
        }
    }

    return MHAL_SUCCESS;

FAIL:
    printf("%s[%d] error\n", __FUNCTION__, line);
    return MHAL_FAILURE;
}

MS_S32 MHAL_AUDIO_AI_Close(MHAL_AI_Dma_e enAiDma)
{
    int            ret     = AIO_OK;
    CHIP_AIO_DMA_e eAioDma = 0;
    AI_DMA_e       aiDma   = 0;
    int            line    = 0;

    //
    aiDma = enAiDma;

    //
    if (aiDma >= AI_DMA_DIRECT_START)
    {
        if (aiDma >= E_MHAL_AI_DMA_TOTAL)
        {
            printf("%s[%d] aiDma direct %d error\n", __FUNCTION__, __LINE__, aiDma);
            line = __LINE__;
            goto FAIL;
        }
    }
    else
    {
        if (!CHIP_AI_DMA_VALID(aiDma))
        {
            printf("%s[%d] aiDma %d error\n", __FUNCTION__, __LINE__, aiDma);
            line = __LINE__;
            goto FAIL;
        }
    }

    //
    if (aiDma >= AI_DMA_DIRECT_START)
    {
    }
    else
    {
        eAioDma = CHIP_AIO_DMA_IDX_BY_AI_DMA(aiDma);
        if (!CHIP_AIO_DMA_IDX_VALID(eAioDma))
        {
            line = __LINE__;
            goto FAIL;
        }

        // Close
        ret |= DrvAudCloseDma(eAioDma);
        if (ret != AIO_OK)
        {
            line = __LINE__;
            goto FAIL;
        }
    }

    //
    ret |= _MhalAudioAiDetachDma(aiDma);
    if (ret != AIO_OK)
    {
        line = __LINE__;
        goto FAIL;
    }

    return MHAL_SUCCESS;

FAIL:
    printf("%s[%d] enAiDma[%d] error\n", __FUNCTION__, line, enAiDma);
    return MHAL_FAILURE;
}

MS_S32 MHAL_AUDIO_AO_Close(MHAL_AO_Dma_e enAoDma)
{
    int            ret     = AIO_OK;
    CHIP_AIO_DMA_e eAioDma = 0;
    AO_DMA_e       aoDma   = 0;
    int            line    = 0;

    //
    aoDma = enAoDma;

    //
    if (aoDma >= AO_DMA_DIRECT_START)
    {
        if (aoDma >= E_MHAL_AO_DMA_TOTAL)
        {
            printf("%s[%d] aoDma direct %d error\n", __FUNCTION__, __LINE__, aoDma);
            line = __LINE__;
            goto FAIL;
        }
    }
    else
    {
        if (!CHIP_AO_DMA_VALID(aoDma))
        {
            printf("%s[%d] aoDma %d error\n", __FUNCTION__, __LINE__, aoDma);
            line = __LINE__;
            goto FAIL;
        }
    }

    //
    if (aoDma >= AO_DMA_DIRECT_START)
    {
    }
    else
    {
        eAioDma = CHIP_AIO_DMA_IDX_BY_AO_DMA(aoDma);
        if (!CHIP_AIO_DMA_IDX_VALID(eAioDma))
        {
            line = __LINE__;
            goto FAIL;
        }

        // Close
        ret |= DrvAudCloseDma(eAioDma);
        if (ret != AIO_OK)
        {
            line = __LINE__;
            goto FAIL;
        }
    }

    //
    ret |= _MhalAudioAoDetachDma(aoDma);
    if (ret != AIO_OK)
    {
        line = __LINE__;
        goto FAIL;
    }

    return MHAL_SUCCESS;

FAIL:
    printf("%s[%d] enAoDma[%d] error\n", __FUNCTION__, line, enAoDma);
    return MHAL_FAILURE;
}

MS_S32 MHAL_AUDIO_AI_Start(MHAL_AI_Dma_e enAiDma)
{
    int            ret     = AIO_OK;
    CHIP_AIO_DMA_e eAioDma = 0;
    AI_DMA_e       aiDma   = 0;
    int            line    = 0;

    //
    aiDma = enAiDma;

    if (!CHIP_AI_DMA_VALID(aiDma))
    {
        line = __LINE__;
        goto FAIL;
    }

    //
    if (g_pAiDmaStatusList[aiDma].bIsAttached == FALSE)
    {
        printf("%s, enAiDma = %d, bIsAttached = FALSE\n", __FUNCTION__, enAiDma);
        line = __LINE__;
        goto FAIL;
    }

    eAioDma = CHIP_AIO_DMA_IDX_BY_AI_DMA(aiDma);
    if (!CHIP_AIO_DMA_IDX_VALID(eAioDma))
    {
        line = __LINE__;
        goto FAIL;
    }

    // Start
    ret |= DrvAudStartDma(eAioDma);
    if (ret != AIO_OK)
    {
        line = __LINE__;
        goto FAIL;
    }

    return MHAL_SUCCESS;

FAIL:
    printf("%s[%d] enAiDma[%d] error\n", __FUNCTION__, line, enAiDma);
    return MHAL_FAILURE;
}

MS_S32 MHAL_AUDIO_AO_Start(MHAL_AO_Dma_e enAoDma)
{
    int            ret     = AIO_OK;
    CHIP_AIO_DMA_e eAioDma = 0;
    AO_DMA_e       aoDma   = 0;
    int            line    = 0;

    //
    aoDma = enAoDma;

    if (!CHIP_AO_DMA_VALID(aoDma))
    {
        line = __LINE__;
        goto FAIL;
    }

    //
    if (g_pAoDmaStatusList[aoDma].bIsAttached == FALSE)
    {
        printf("%s, enAoDma = %d, bIsAttached = FALSE\n", __FUNCTION__, enAoDma);
        line = __LINE__;
        goto FAIL;
    }

    eAioDma = CHIP_AIO_DMA_IDX_BY_AO_DMA(aoDma);
    if (!CHIP_AIO_DMA_IDX_VALID(eAioDma))
    {
        line = __LINE__;
        goto FAIL;
    }

    // Start
    ret |= DrvAudStartDma(eAioDma);
    if (ret != AIO_OK)
    {
        line = __LINE__;
        goto FAIL;
    }

    return MHAL_SUCCESS;

FAIL:
    printf("%s[%d] enAoDma[%d] error\n", __FUNCTION__, line, enAoDma);
    return MHAL_FAILURE;
}

MS_S32 MHAL_AUDIO_AI_Stop(MHAL_AI_Dma_e enAiDma)
{
    int            ret     = AIO_OK;
    CHIP_AIO_DMA_e eAioDma = 0;
    AI_DMA_e       aiDma   = 0;
    int            line    = 0;

    //
    aiDma = enAiDma;

    if (!CHIP_AI_DMA_VALID(aiDma))
    {
        line = __LINE__;
        goto FAIL;
    }

    //
    if (g_pAiDmaStatusList[aiDma].bIsAttached == FALSE)
    {
        printf("%s, enAiDma = %d, bIsAttached = FALSE\n", __FUNCTION__, enAiDma);
        line = __LINE__;
        goto FAIL;
    }

    eAioDma = CHIP_AIO_DMA_IDX_BY_AI_DMA(aiDma);
    if (!CHIP_AIO_DMA_IDX_VALID(eAioDma))
    {
        line = __LINE__;
        goto FAIL;
    }

    // Stop
    ret |= DrvAudStopDma(eAioDma);
    if (ret != AIO_OK)
    {
        line = __LINE__;
        goto FAIL;
    }

    return MHAL_SUCCESS;

FAIL:
    printf("%s[%d] enAiDma[%d] error\n", __FUNCTION__, line, enAiDma);
    return MHAL_FAILURE;
}

MS_S32 MHAL_AUDIO_AO_Stop(MHAL_AO_Dma_e enAoDma)
{
    int            ret     = AIO_OK;
    CHIP_AIO_DMA_e eAioDma = 0;
    AO_DMA_e       aoDma   = 0;
    int            line    = 0;

    //
    aoDma = enAoDma;

    if (!CHIP_AO_DMA_VALID(aoDma))
    {
        line = __LINE__;
        goto FAIL;
    }

#if 0
    //
    if (g_pAoDmaStatusList[aoDma].bIsAttached == FALSE)
    {
       printf( "%s, enAoDma = %d, bIsAttached = FALSE\n", __FUNCTION__, enAoDma);
        line = __LINE__;
        goto FAIL;
    }
#endif

    eAioDma = CHIP_AIO_DMA_IDX_BY_AO_DMA(aoDma);
    if (!CHIP_AIO_DMA_IDX_VALID(eAioDma))
    {
        line = __LINE__;
        goto FAIL;
    }
    // Stop
    ret |= DrvAudStopDma(eAioDma);
    if (ret != AIO_OK)
    {
        line = __LINE__;
        goto FAIL;
    }

    return MHAL_SUCCESS;

FAIL:
    printf("%s[%d] enAoDma[%d] error\n", __FUNCTION__, line, enAoDma);
    return MHAL_FAILURE;
}

MS_S32 MHAL_AUDIO_AI_Read_Data(MHAL_AI_Dma_e enAiDma, VOID *pRdBuffer, MS_U32 u32Size)
{
    // int ret = AIO_OK;
    CHIP_AIO_DMA_e eAioDma = 0;
    AI_DMA_e       aiDma   = 0;
    MS_S32         s32Err;
    int            line = 0;

    //
    aiDma = enAiDma;

    if (!CHIP_AI_DMA_VALID(aiDma))
    {
        line = __LINE__;
        goto FAIL;
    }

    //
    if (g_pAiDmaStatusList[aiDma].bIsAttached == FALSE)
    {
        printf("%s, enAiDma = %d, bIsAttached = FALSE\n", __FUNCTION__, enAiDma);
        line = __LINE__;
        goto FAIL;
    }

    eAioDma = CHIP_AIO_DMA_IDX_BY_AI_DMA(aiDma);
    if (!CHIP_AIO_DMA_IDX_VALID(eAioDma))
    {
        line = __LINE__;
        goto FAIL;
    }

    // Read
    s32Err = DrvAudReadPcm(eAioDma, pRdBuffer, u32Size);

    if (s32Err >= 0)
    {
        goto SUCCESS;
    }
    else
    {
        if (s32Err == -EAGAIN) // Non-block mode, Resource temporarily unavailable.
        {
            return MHAL_FAILURE_NO_RESOURCE;
        }

        return MHAL_FAILURE;
    }

    // return MHAL_SUCCESS;
SUCCESS:
    return s32Err;

FAIL:
    printf("%s[%d] enAiDma[%d] u32Size[%d] s32TimeoutMs[%d]error\n", __FUNCTION__, line, enAiDma, u32Size);
    return MHAL_FAILURE;
}

MS_S32 MHAL_AUDIO_AO_Write_Data(MHAL_AO_Dma_e enAoDma, VOID *pWrBuffer, MS_U32 u32Size)
{
    // int ret = AIO_OK;
    CHIP_AIO_DMA_e eAioDma = 0;
    AO_DMA_e       aoDma   = 0;
    MS_S32         s32Err;
    int            line = 0;

    //
    aoDma = enAoDma;

    if (!CHIP_AO_DMA_VALID(aoDma))
    {
        line = __LINE__;
        goto FAIL;
    }

//
#if 0
    if (g_pAoDmaStatusList[aoDma].bIsAttached == FALSE)
    {
       printf( "%s, enAoDma = %d, bIsAttached = FALSE\n", __FUNCTION__, enAoDma);
        line = __LINE__;
        goto FAIL;
    }
#endif

    eAioDma = CHIP_AIO_DMA_IDX_BY_AO_DMA(aoDma);
    if (!CHIP_AIO_DMA_IDX_VALID(eAioDma))
    {
        line = __LINE__;
        goto FAIL;
    }

    // Write
    s32Err = DrvAudWritePcm(eAioDma, pWrBuffer, u32Size);

    if (s32Err >= 0)
    {
        goto SUCCESS;
    }
    else
    {
        if (s32Err == -EAGAIN) // Non-block mode, Resource temporarily unavailable.
        {
            return MHAL_FAILURE_NO_RESOURCE;
        }

        return MHAL_FAILURE;
    }

    // return MHAL_SUCCESS;
SUCCESS:
    return s32Err;

FAIL:
    printf("%s[%d] enAoDma[%d] u32Size[%d] s32TimeoutMs[%d]error\n", __FUNCTION__, line, enAoDma, u32Size);
    return MHAL_FAILURE;
}

MS_BOOL MHAL_AUDIO_AI_IsXrun(MHAL_AI_Dma_e enAiDma)
{
    int            ret     = AIO_OK;
    CHIP_AIO_DMA_e eAioDma = 0;
    AI_DMA_e       aiDma   = 0;

    //
    aiDma = enAiDma;

    if (!CHIP_AI_DMA_VALID(aiDma))
    {
        printf("%s, enAiDma = %d, is not valid\n", __FUNCTION__, enAiDma);
        return FALSE;
    }

    //
    if (g_pAiDmaStatusList[aiDma].bIsAttached == FALSE)
    {
        printf("%s, enAiDma = %d, bIsAttached = FALSE\n", __FUNCTION__, enAiDma);
        // todo Show Error
        return FALSE;
    }

    eAioDma = CHIP_AIO_DMA_IDX_BY_AI_DMA(aiDma);
    if (!CHIP_AIO_DMA_IDX_VALID(eAioDma))
    {
        // todo Show Error
    }

    //
    if (DrvAudIsXrun(eAioDma) ? TRUE : FALSE)
    {
        ret |= DrvAudPrepareDma(eAioDma);
        if (ret != AIO_OK)
        {
            // todo Show Error
        }

        return TRUE;
    }

    return FALSE;
}

MS_BOOL MHAL_AUDIO_AO_IsXrun(MHAL_AO_Dma_e enAoDma)
{
    int            ret     = AIO_OK;
    CHIP_AIO_DMA_e eAioDma = 0;
    AO_DMA_e       aoDma   = 0;

    //
    aoDma = enAoDma;

    if (!CHIP_AO_DMA_VALID(aoDma))
    {
        printf("%s, enAiDma = %d, is not valid\n", __FUNCTION__, enAoDma);
        return FALSE;
    }

#if 0
    //
    if (g_pAoDmaStatusList[aoDma].bIsAttached == FALSE)
    {
       printf( "%s, enAoDma = %d, bIsAttached = FALSE\n", __FUNCTION__, enAoDma);
        // todo Show Error
        return FALSE;
    }
#endif

    eAioDma = CHIP_AIO_DMA_IDX_BY_AO_DMA(enAoDma);
    if (!CHIP_AIO_DMA_IDX_VALID(eAioDma))
    {
        // todo Show Error
    }

    //
    if (DrvAudIsXrun(eAioDma) ? TRUE : FALSE)
    {
        ret |= DrvAudPrepareDma(eAioDma);
        if (ret != AIO_OK)
        {
            // todo Show Error
        }

        return TRUE;
    }

    return FALSE;
}

MS_S32 MHAL_AUDIO_AI_PrepareToRestart(MHAL_AI_Dma_e enAiDma)
{
    int            ret     = AIO_OK;
    CHIP_AIO_DMA_e eAioDma = 0;
    AI_DMA_e       aiDma   = 0;
    int            line    = 0;

    //
    aiDma = enAiDma;

    if (!CHIP_AI_DMA_VALID(aiDma))
    {
        line = __LINE__;
        goto FAIL;
    }

    //
    if (g_pAiDmaStatusList[aiDma].bIsAttached == FALSE)
    {
        printf("%s, enAiDma = %d, bIsAttached = FALSE\n", __FUNCTION__, enAiDma);
        line = __LINE__;
        goto FAIL;
    }

    eAioDma = CHIP_AIO_DMA_IDX_BY_AI_DMA(aiDma);
    if (!CHIP_AIO_DMA_IDX_VALID(eAioDma))
    {
        line = __LINE__;
        goto FAIL;
    }

    //
    ret |= DrvAudStopDma(eAioDma);
    if (ret != AIO_OK)
    {
        line = __LINE__;
        goto FAIL;
    }

    ret |= DrvAudPrepareDma(eAioDma);
    if (ret != AIO_OK)
    {
        line = __LINE__;
        goto FAIL;
    }

    return MHAL_SUCCESS;

FAIL:
    printf("%s[%d] enAiDma[%d] error\n", __FUNCTION__, line, enAiDma);
    return MHAL_FAILURE;
}

MS_S32 MHAL_AUDIO_AO_PrepareToRestart(MHAL_AO_Dma_e enAoDma)
{
    int            ret     = AIO_OK;
    CHIP_AIO_DMA_e eAioDma = 0;
    AO_DMA_e       aoDma   = 0;
    int            line    = 0;

    //
    aoDma = enAoDma;

    if (!CHIP_AO_DMA_VALID(aoDma))
    {
        line = __LINE__;
        goto FAIL;
    }
#if 0
    //
    if (g_pAoDmaStatusList[aoDma].bIsAttached == FALSE)
    {
       printf( "%s, enAoDma = %d, bIsAttached = FALSE\n", __FUNCTION__, enAoDma);
        line = __LINE__;
        goto FAIL;
    }
#endif
    eAioDma = CHIP_AIO_DMA_IDX_BY_AO_DMA(aoDma);
    if (!CHIP_AIO_DMA_IDX_VALID(eAioDma))
    {
        line = __LINE__;
        goto FAIL;
    }

    //
    ret |= DrvAudStopDma(eAioDma);
    if (ret != AIO_OK)
    {
        line = __LINE__;
        goto FAIL;
    }

    ret |= DrvAudPrepareDma(eAioDma);
    if (ret != AIO_OK)
    {
        line = __LINE__;
        goto FAIL;
    }

    return MHAL_SUCCESS;

FAIL:
    printf("%s[%d] enAoDma[%d] error\n", __FUNCTION__, line, enAoDma);
    return MHAL_FAILURE;
}

MS_S32 MHAL_AUDIO_AI_GetCurrDataLen(MHAL_AI_Dma_e enAiDma, MS_U32 *len)
{
    int            ret     = AIO_OK;
    CHIP_AIO_DMA_e eAioDma = 0;
    AI_DMA_e       aiDma   = 0;
    int            line    = 0;

    //
    aiDma = enAiDma;

    if (!CHIP_AI_DMA_VALID(aiDma))
    {
        line = __LINE__;
        goto FAIL;
    }

    //
    if (g_pAiDmaStatusList[aiDma].bIsAttached == FALSE)
    {
        printf("%s, enAiDma = %d, bIsAttached = FALSE\n", __FUNCTION__, enAiDma);
        line = __LINE__;
        goto FAIL;
    }

    eAioDma = CHIP_AIO_DMA_IDX_BY_AI_DMA(aiDma);
    if (!CHIP_AIO_DMA_IDX_VALID(eAioDma))
    {
        line = __LINE__;
        goto FAIL;
    }

    ret |= DrvAudGetBufCurrDataLen(eAioDma, len);
    if (ret != AIO_OK)
    {
        line = __LINE__;
        goto FAIL;
    }

    return MHAL_SUCCESS;

FAIL:
    printf("%s[%d] enAiDma[%d] error\n", __FUNCTION__, line, enAiDma);
    return MHAL_FAILURE;
}

MS_S32 MHAL_AUDIO_AO_GetCurrDataLen(MHAL_AO_Dma_e enAoDma, MS_U32 *len)
{
    int            ret     = AIO_OK;
    CHIP_AIO_DMA_e eAioDma = 0;
    AO_DMA_e       aoDma   = 0;
    int            line    = 0;

    //
    aoDma = enAoDma;

    if (!CHIP_AO_DMA_VALID(aoDma))
    {
        line = __LINE__;
        goto FAIL;
    }

#if 0
    //
    if (g_pAoDmaStatusList[aoDma].bIsAttached == FALSE)
    {
       printf( "%s, enAoDma = %d, bIsAttached = FALSE\n", __FUNCTION__, enAoDma);
        line = __LINE__;
        goto FAIL;
    }
#endif

    eAioDma = CHIP_AIO_DMA_IDX_BY_AO_DMA(aoDma);
    if (!CHIP_AIO_DMA_IDX_VALID(eAioDma))
    {
        line = __LINE__;
        goto FAIL;
    }

    ret |= DrvAudGetBufCurrDataLen(eAioDma, len);
    if (ret != AIO_OK)
    {
        line = __LINE__;
        goto FAIL;
    }

    return MHAL_SUCCESS;

FAIL:
    printf("%s[%d] enAoDma[%d] error\n", __FUNCTION__, line, enAoDma);
    return MHAL_FAILURE;
}

MS_BOOL MHAL_AUDIO_AI_IsDmaFree(MHAL_AI_Dma_e enAiDma)
{
    AI_DMA_e aiDma = 0;

    //
    aiDma = enAiDma;

    if (!CHIP_AI_DMA_VALID(aiDma))
    {
        // todo Show Error
        return FALSE;
    }

    if (g_pAiDmaStatusList[aiDma].bIsAttached == FALSE)
    {
        // todo Show Error
        return FALSE;
    }

    return TRUE;
}

MS_BOOL MHAL_AUDIO_AO_IsDmaFree(MHAL_AO_Dma_e enAoDma)
{
    AO_DMA_e aoDma = 0;

    //
    aoDma = enAoDma;

    if (!CHIP_AO_DMA_VALID(aoDma))
    {
        // todo Show Error
        return FALSE;
    }

    if (g_pAoDmaStatusList[aoDma].bIsAttached == FALSE)
    {
        // todo Show Error
        return FALSE;
    }

    return TRUE;
}

MS_S32 MHAL_AUDIO_AO_Pause(MHAL_AO_Dma_e enAoDma)
{
    int            ret     = AIO_OK;
    CHIP_AIO_DMA_e eAioDma = 0;
    AO_DMA_e       aoDma   = 0;
    int            line    = 0;

    //
    aoDma = enAoDma;

    if (!CHIP_AO_DMA_VALID(aoDma))
    {
        line = __LINE__;
        goto FAIL;
    }

    //
    if (g_pAoDmaStatusList[aoDma].bIsAttached == FALSE)
    {
        printf("%s, enAoDma = %d, bIsAttached = FALSE\n", __FUNCTION__, enAoDma);
        line = __LINE__;
        goto FAIL;
    }

    eAioDma = CHIP_AIO_DMA_IDX_BY_AO_DMA(aoDma);
    if (!CHIP_AIO_DMA_IDX_VALID(eAioDma))
    {
        line = __LINE__;
        goto FAIL;
    }

    // Stop
    ret |= DrvAudPauseDma(eAioDma);
    if (ret != AIO_OK)
    {
        line = __LINE__;
        goto FAIL;
    }

    return MHAL_SUCCESS;

FAIL:
    printf("%s[%d] enAoDma[%d] error\n", __FUNCTION__, line, enAoDma);
    return MHAL_FAILURE;
}

MS_S32 MHAL_AUDIO_AO_Resume(MHAL_AO_Dma_e enAoDma)
{
    int            ret     = AIO_OK;
    CHIP_AIO_DMA_e eAioDma = 0;
    AO_DMA_e       aoDma   = 0;
    int            line    = 0;

    //
    aoDma = enAoDma;

    if (!CHIP_AO_DMA_VALID(aoDma))
    {
        line = __LINE__;
        goto FAIL;
    }

    //
    if (g_pAoDmaStatusList[aoDma].bIsAttached == FALSE)
    {
        printf("%s, enAoDma = %d, bIsAttached = FALSE\n", __FUNCTION__, enAoDma);
        line = __LINE__;
        goto FAIL;
    }

    eAioDma = CHIP_AIO_DMA_IDX_BY_AO_DMA(aoDma);
    if (!CHIP_AIO_DMA_IDX_VALID(eAioDma))
    {
        line = __LINE__;
        goto FAIL;
    }

    // Stop
    ret |= DrvAudResumeDma(eAioDma);
    if (ret != AIO_OK)
    {
        line = __LINE__;
        goto FAIL;
    }

    return MHAL_SUCCESS;

FAIL:
    printf("%s[%d] enAoDma[%d] error\n", __FUNCTION__, line, enAoDma);
    return MHAL_FAILURE;
}

MS_S32 MHAL_AUDIO_SineGen_Start(MHAL_SineGen_e enSineGen, MHAL_SineGen_Cfg_t *pstSineGenConfig)
{
    int                 ret    = AIO_OK;
    MHAL_SineGen_Freq_e enFreq = 0;
    MS_S8               s8Gain = 0;
    int                 line   = 0;

    enFreq = pstSineGenConfig->enFreq;
    s8Gain = pstSineGenConfig->s8Gain;

    ret |= DrvAudSineGenSetting(enSineGen, (U8)enFreq, s8Gain);
    if (ret != AIO_OK)
    {
        line = __LINE__;
        goto FAIL;
    }

    ret |= DrvAudSineGenEnable(enSineGen, TRUE);
    if (ret != AIO_OK)
    {
        line = __LINE__;
        goto FAIL;
    }

    return MHAL_SUCCESS;

FAIL:
    printf("%s[%d] enSineGen[%d] error\n", __FUNCTION__, line, enSineGen);
    return MHAL_FAILURE;
}

MS_S32 MHAL_AUDIO_SineGen_Stop(MHAL_SineGen_e enSineGen)
{
    int ret  = AIO_OK;
    int line = 0;

    ret |= DrvAudSineGenEnable(enSineGen, FALSE);
    if (ret != AIO_OK)
    {
        line = __LINE__;
        goto FAIL;
    }

    return MHAL_SUCCESS;

FAIL:
    printf("%s[%d] enSineGen[%d] error\n", __FUNCTION__, line, enSineGen);
    return MHAL_FAILURE;
}

void MHAL_AUDIO_Module_Init(void)
{
    // CamOsMutexInit(&g_tAudInitMutex);
}

void MHAL_AUDIO_Module_DeInit(void)
{
    //  CamOsMutexDestroy(&g_tAudInitMutex);
}

#if 0 // TEMP
switch (eAoIf)
{
    case E_MHAL_AO_IF_DAC_A_0:
        break;

    case E_MHAL_AO_IF_DAC_B_0:
        break;

    case E_MHAL_AO_IF_DAC_C_0:
        break;

    case E_MHAL_AO_IF_DAC_D_0:
        break;

    case E_MHAL_AO_IF_HDMI_A_0:
        break;

    case E_MHAL_AO_IF_HDMI_A_1:
        break;

    case E_MHAL_AO_IF_ECHO_A_0:
        break;

    case E_MHAL_AO_IF_ECHO_A_1:
        break;

    case E_MHAL_AO_IF_I2S_TX_A_0:
        break;

    case E_MHAL_AO_IF_I2S_TX_A_1:
        break;

    case E_MHAL_AO_IF_I2S_TX_B_0:
        break;

    case E_MHAL_AO_IF_I2S_TX_B_1:
        break;

    case E_MHAL_AO_IF_NONE:
        break;

    default:
        goto FAIL;
        break;
}

switch (eAiCh)
{
    case E_AI_CH_ADC_A_0:
        break;

    case E_AI_CH_ADC_B_0:
        break;

    case E_AI_CH_ADC_C_0:
        break;

    case E_AI_CH_ADC_D_0:
        break;

    case E_AI_CH_DMIC_A_0:
        break;

    case E_AI_CH_DMIC_A_1:
        break;

    case E_AI_CH_DMIC_A_2:
        break;

    case E_AI_CH_DMIC_A_3:
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
        goto FAIL;
        break;
}

switch (eAoCh)
{
    case E_AO_CH_DAC_A_0:
        break;

    case E_AO_CH_DAC_B_0:
        break;

    case E_AO_CH_DAC_C_0:
        break;

    case E_AO_CH_DAC_D_0:
        break;

    case E_AO_CH_HDMI_A_0:
        break;

    case E_AO_CH_HDMI_A_1:
        break;

    case E_AO_CH_ECHO_A_0:
        break;

    case E_AO_CH_ECHO_A_1:
        break;

    case E_AO_CH_I2S_TX_A_0:
    case E_AO_CH_I2S_TX_A_1:

        break;

    case E_AO_CH_I2S_TX_B_0:
    case E_AO_CH_I2S_TX_B_1:

        break;

    default:
        goto FAIL;
        break;
}




switch (aiIf)
{
    case E_MHAL_AI_IF_ADC_A_0_B_0:
        break;

    case E_MHAL_AI_IF_ADC_C_0_D_0:
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
        break;

    case E_MHAL_AI_IF_I2S_RX_A_2_3:
        break;

    case E_MHAL_AI_IF_I2S_RX_A_4_5:
        break;

    case E_MHAL_AI_IF_I2S_RX_A_6_7:
        break;

    case E_MHAL_AI_IF_I2S_RX_A_8_9:
        break;

    case E_MHAL_AI_IF_I2S_RX_A_10_11:
        break;

    case E_MHAL_AI_IF_I2S_RX_A_12_13:
        break;

    case E_MHAL_AI_IF_I2S_RX_A_14_15:
        break;

    case E_MHAL_AI_IF_I2S_RX_B_0_1:
        break;

    case E_MHAL_AI_IF_I2S_RX_B_2_3:
        break;

    case E_MHAL_AI_IF_I2S_RX_B_4_5:
        break;

    case E_MHAL_AI_IF_I2S_RX_B_6_7:
        break;

    case E_MHAL_AI_IF_I2S_RX_B_8_9:
        break;

    case E_MHAL_AI_IF_I2S_RX_B_10_11:
        break;

    case E_MHAL_AI_IF_I2S_RX_B_12_13:
        break;

    case E_MHAL_AI_IF_I2S_RX_B_14_15:
        break;

    case E_MHAL_AI_IF_I2S_RX_C_0_1:
        break;

    case E_MHAL_AI_IF_I2S_RX_C_2_3:
        break;

    case E_MHAL_AI_IF_I2S_RX_C_4_5:
        break;

    case E_MHAL_AI_IF_I2S_RX_C_6_7:
        break;

    case E_MHAL_AI_IF_I2S_RX_C_8_9:
        break;

    case E_MHAL_AI_IF_I2S_RX_C_10_11:
        break;

    case E_MHAL_AI_IF_I2S_RX_C_12_13:
        break;

    case E_MHAL_AI_IF_I2S_RX_C_14_15:
        break;

    case E_MHAL_AI_IF_I2S_RX_D_0_1:
        break;

    case E_MHAL_AI_IF_I2S_RX_D_2_3:
        break;

    case E_MHAL_AI_IF_I2S_RX_D_4_5:
        break;

    case E_MHAL_AI_IF_I2S_RX_D_6_7:
        break;

    case E_MHAL_AI_IF_I2S_RX_D_8_9:
        break;

    case E_MHAL_AI_IF_I2S_RX_D_10_11:
        break;

    case E_MHAL_AI_IF_I2S_RX_D_12_13:
        break;

    case E_MHAL_AI_IF_I2S_RX_D_14_15:
        break;

    default:
        break;
}

#if 0
    switch (enAiIf)
    {
        case E_MHAL_AI_IF_ADC_A_0_B_0:
            break;

        case E_MHAL_AI_IF_ADC_C_0_D_0:
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
            goto FAIL;
            break;
    }
#endif

#endif
