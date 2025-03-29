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
#include "ms_platform.h"
#include "hal_audio_common.h"
#include "hal_audio_sys.h"
#include "hal_audio_config.h"
//#include "hal_audio_reg.h"
#include "hal_audio_types.h"
#include "hal_audio_api.h"
#include "hal_audio_os_api.h"

#include "mhal_common.h"
#include "mhal_audio.h"
#include "mhal_audio_datatype.h"

#include "audio_proc.h"

// -------------------------------------------------------------------------------
#define DMIC_CH_MAX             (8)
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
static CamOsMutex_t    g_tAudInitMutex;
static volatile MS_S16 g_nInitialed = 0;
static DEFINE_MUTEX(g_AudioAttachmutex);
static DEFINE_MUTEX(g_AudioDetachmutex);

static MHAL_AI_Dma_Status_t    g_pAiDmaStatusList[E_MHAL_AI_DMA_TOTAL];
static MHAL_AO_Dma_Status_t    g_pAoDmaStatusList[E_MHAL_AO_DMA_TOTAL];
static MHAL_AI_DigMic_Status_t g_pAiDigMicStatusList[AI_DEV_DMIC_TOTAL];
static MHAL_AO_If_Ch_Status_t  g_pAoIfStatusList[E_AO_CH_TOTAL];
static MHAL_AI_If_Ch_Status_t  g_pAiIfStatusList[E_AI_CH_TOTAL];
// -------------------------------------------------------------------------------
static int _MhalAudioAiAttachCore(AI_DMA_e enAiDma, MS_U8 nAiDmaChSlotNum, AI_IF_e eAiIf);
static int _MhalAudioAiDettachCore(AI_DMA_e enAiDma, MS_U8 nAiDmaChSlotNum, AI_IF_e eAiIf);
static int _MhalAudioAoDettachCore(AO_DMA_e enAoDma, MS_U8 nAoDmaChSlotNum, AO_IF_e eAoIf);
static int _MhalAudioAiDirectAttachCore(AI_DMA_e enAiDma, MS_U8 nAiDmaChSlotNum, AI_IF_e eAiIf);
static int _MhalAudioAoDirectAttachCore(AO_DMA_e enAoDma, MS_U8 nAoDmaChSlotNum, AO_IF_e eAoIf);
static int _MhalAudioAiDirectDettachCore(AI_DMA_e enAiDma, MS_U8 nAiDmaChSlotNum, AI_IF_e eAiIf);
static int _MhalAudioAoDirectDettachCore(AO_DMA_e enAoDma, MS_U8 nAoDmaChSlotNum, AO_IF_e eAoIf);
static int _MhalAudioDirectAttachCore(MHAL_Direct_Attach_t *pDirectAttach);
static int _MhalAudioDirectDettachCore(AO_DMA_e enAoDma, MHAL_Direct_Attach_t *pDirectAttach);
static int _MhalAudioAiConfigI2s(AI_DEV_e eAiDev, MHAL_AUDIO_I2sCfg_t *pstI2sConfig);
static int _MhalAudioAoConfigI2s(AO_DEV_e eAoDev, MHAL_AUDIO_I2sCfg_t *pstI2sConfig);
static int _MhalAudioAiIfEnableByStatus(AI_DMA_e enAiDma, AI_IF_e eAiIf, BOOL bEn);
static int _MhalAudioAoIfEnableByStatus(AO_DMA_e enAoDma, AO_IF_e eAoIf, BOOL bEn);
static int _MhalAudioAiIfToDevCh(MHAL_AI_IF_e eAiIf, AI_CH_e *peAiCh1, AI_CH_e *peAiCh2);
static int _MhalAudioAoIfToDevCh(MHAL_AO_IF_e eAoIf, AO_CH_e *peAoCh, MS_U8 *pnGetNum);
static int _MhalAudioAiCheckIfDoSettingByStatus(MHAL_AI_IF_e eAiIf, BOOL bEn, BOOL *pbIsDoSetting);
static int _MhalAudioAoCheckIfDoSettingByStatus(MHAL_AO_IF_e eAoIf, BOOL bEn, BOOL *pbIsDoSetting);
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
            ret |= HalAudSetAiPath(eAioDma, nDmaCh1, E_AI_CH_ADC_A_0);
            ret |= HalAudSetAiPath(eAioDma, nDmaCh2, E_AI_CH_ADC_B_0);
            break;

        case E_MHAL_AI_IF_ADC_C_0_D_0:
            ret |= HalAudSetAiPath(eAioDma, nDmaCh1, E_AI_CH_ADC_C_0);
            ret |= HalAudSetAiPath(eAioDma, nDmaCh2, E_AI_CH_ADC_D_0);
            break;

        case E_MHAL_AI_IF_DMIC_A_0_1:
            ret |= HalAudSetAiPath(eAioDma, nDmaCh1, E_AI_CH_DMIC_A_0);
            ret |= HalAudSetAiPath(eAioDma, nDmaCh2, E_AI_CH_DMIC_A_1);
            break;

        case E_MHAL_AI_IF_DMIC_A_2_3:
            ret |= HalAudSetAiPath(eAioDma, nDmaCh1, E_AI_CH_DMIC_A_2);
            ret |= HalAudSetAiPath(eAioDma, nDmaCh2, E_AI_CH_DMIC_A_3);
            break;

        case E_MHAL_AI_IF_DMIC_A_4_5:
            ret |= HalAudSetAiPath(eAioDma, nDmaCh1, E_AI_CH_DMIC_A_4);
            ret |= HalAudSetAiPath(eAioDma, nDmaCh2, E_AI_CH_DMIC_A_5);
            break;

        case E_MHAL_AI_IF_DMIC_A_6_7:
            ret |= HalAudSetAiPath(eAioDma, nDmaCh1, E_AI_CH_DMIC_A_6);
            ret |= HalAudSetAiPath(eAioDma, nDmaCh2, E_AI_CH_DMIC_A_7);
            break;

        case E_MHAL_AI_IF_ECHO_A_0_1:
            ret |= HalAudSetAiPath(eAioDma, nDmaCh1, E_AI_CH_ECHO_A_0);
            ret |= HalAudSetAiPath(eAioDma, nDmaCh2, E_AI_CH_ECHO_A_1);
            break;

        case E_MHAL_AI_IF_I2S_RX_A_0_1:
            ret |= HalAudSetAiPath(eAioDma, nDmaCh1, E_AI_CH_I2S_RX_A_0);
            ret |= HalAudSetAiPath(eAioDma, nDmaCh2, E_AI_CH_I2S_RX_A_1);
            break;

        case E_MHAL_AI_IF_I2S_RX_A_2_3:
            ret |= HalAudSetAiPath(eAioDma, nDmaCh1, E_AI_CH_I2S_RX_A_2);
            ret |= HalAudSetAiPath(eAioDma, nDmaCh2, E_AI_CH_I2S_RX_A_3);
            break;

        case E_MHAL_AI_IF_I2S_RX_A_4_5:
            ret |= HalAudSetAiPath(eAioDma, nDmaCh1, E_AI_CH_I2S_RX_A_4);
            ret |= HalAudSetAiPath(eAioDma, nDmaCh2, E_AI_CH_I2S_RX_A_5);
            break;

        case E_MHAL_AI_IF_I2S_RX_A_6_7:
            ret |= HalAudSetAiPath(eAioDma, nDmaCh1, E_AI_CH_I2S_RX_A_6);
            ret |= HalAudSetAiPath(eAioDma, nDmaCh2, E_AI_CH_I2S_RX_A_7);
            break;

        case E_MHAL_AI_IF_I2S_RX_A_8_9:
            ret |= HalAudSetAiPath(eAioDma, nDmaCh1, E_AI_CH_I2S_RX_A_8);
            ret |= HalAudSetAiPath(eAioDma, nDmaCh2, E_AI_CH_I2S_RX_A_9);
            break;

        case E_MHAL_AI_IF_I2S_RX_A_10_11:
            ret |= HalAudSetAiPath(eAioDma, nDmaCh1, E_AI_CH_I2S_RX_A_10);
            ret |= HalAudSetAiPath(eAioDma, nDmaCh2, E_AI_CH_I2S_RX_A_11);
            break;

        case E_MHAL_AI_IF_I2S_RX_A_12_13:
            ret |= HalAudSetAiPath(eAioDma, nDmaCh1, E_AI_CH_I2S_RX_A_12);
            ret |= HalAudSetAiPath(eAioDma, nDmaCh2, E_AI_CH_I2S_RX_A_13);
            break;

        case E_MHAL_AI_IF_I2S_RX_A_14_15:
            ret |= HalAudSetAiPath(eAioDma, nDmaCh1, E_AI_CH_I2S_RX_A_14);
            ret |= HalAudSetAiPath(eAioDma, nDmaCh2, E_AI_CH_I2S_RX_A_15);
            break;

        case E_MHAL_AI_IF_I2S_RX_B_0_1:
            ret |= HalAudSetAiPath(eAioDma, nDmaCh1, E_AI_CH_I2S_RX_B_0);
            ret |= HalAudSetAiPath(eAioDma, nDmaCh2, E_AI_CH_I2S_RX_B_1);
            break;

        case E_MHAL_AI_IF_I2S_RX_B_2_3:
            ret |= HalAudSetAiPath(eAioDma, nDmaCh1, E_AI_CH_I2S_RX_B_2);
            ret |= HalAudSetAiPath(eAioDma, nDmaCh2, E_AI_CH_I2S_RX_B_3);
            break;

        case E_MHAL_AI_IF_I2S_RX_B_4_5:
            ret |= HalAudSetAiPath(eAioDma, nDmaCh1, E_AI_CH_I2S_RX_B_4);
            ret |= HalAudSetAiPath(eAioDma, nDmaCh2, E_AI_CH_I2S_RX_B_5);
            break;

        case E_MHAL_AI_IF_I2S_RX_B_6_7:
            ret |= HalAudSetAiPath(eAioDma, nDmaCh1, E_AI_CH_I2S_RX_B_6);
            ret |= HalAudSetAiPath(eAioDma, nDmaCh2, E_AI_CH_I2S_RX_B_7);
            break;

        case E_MHAL_AI_IF_I2S_RX_B_8_9:
            ret |= HalAudSetAiPath(eAioDma, nDmaCh1, E_AI_CH_I2S_RX_B_8);
            ret |= HalAudSetAiPath(eAioDma, nDmaCh2, E_AI_CH_I2S_RX_B_9);
            break;

        case E_MHAL_AI_IF_I2S_RX_B_10_11:
            ret |= HalAudSetAiPath(eAioDma, nDmaCh1, E_AI_CH_I2S_RX_B_10);
            ret |= HalAudSetAiPath(eAioDma, nDmaCh2, E_AI_CH_I2S_RX_B_11);
            break;

        case E_MHAL_AI_IF_I2S_RX_B_12_13:
            ret |= HalAudSetAiPath(eAioDma, nDmaCh1, E_AI_CH_I2S_RX_B_12);
            ret |= HalAudSetAiPath(eAioDma, nDmaCh2, E_AI_CH_I2S_RX_B_13);
            break;

        case E_MHAL_AI_IF_I2S_RX_B_14_15:
            ret |= HalAudSetAiPath(eAioDma, nDmaCh1, E_AI_CH_I2S_RX_B_14);
            ret |= HalAudSetAiPath(eAioDma, nDmaCh2, E_AI_CH_I2S_RX_B_15);
            break;

        case E_MHAL_AI_IF_I2S_RX_C_0_1:
            ret |= HalAudSetAiPath(eAioDma, nDmaCh1, E_AI_CH_I2S_RX_C_0);
            ret |= HalAudSetAiPath(eAioDma, nDmaCh2, E_AI_CH_I2S_RX_C_1);
            break;

        case E_MHAL_AI_IF_I2S_RX_C_2_3:
            ret |= HalAudSetAiPath(eAioDma, nDmaCh1, E_AI_CH_I2S_RX_C_2);
            ret |= HalAudSetAiPath(eAioDma, nDmaCh2, E_AI_CH_I2S_RX_C_3);
            break;

        case E_MHAL_AI_IF_I2S_RX_C_4_5:
            ret |= HalAudSetAiPath(eAioDma, nDmaCh1, E_AI_CH_I2S_RX_C_4);
            ret |= HalAudSetAiPath(eAioDma, nDmaCh2, E_AI_CH_I2S_RX_C_5);
            break;

        case E_MHAL_AI_IF_I2S_RX_C_6_7:
            ret |= HalAudSetAiPath(eAioDma, nDmaCh1, E_AI_CH_I2S_RX_C_6);
            ret |= HalAudSetAiPath(eAioDma, nDmaCh2, E_AI_CH_I2S_RX_C_7);
            break;

        case E_MHAL_AI_IF_I2S_RX_C_8_9:
            ret |= HalAudSetAiPath(eAioDma, nDmaCh1, E_AI_CH_I2S_RX_C_8);
            ret |= HalAudSetAiPath(eAioDma, nDmaCh2, E_AI_CH_I2S_RX_C_9);
            break;

        case E_MHAL_AI_IF_I2S_RX_C_10_11:
            ret |= HalAudSetAiPath(eAioDma, nDmaCh1, E_AI_CH_I2S_RX_C_10);
            ret |= HalAudSetAiPath(eAioDma, nDmaCh2, E_AI_CH_I2S_RX_C_11);
            break;

        case E_MHAL_AI_IF_I2S_RX_C_12_13:
            ret |= HalAudSetAiPath(eAioDma, nDmaCh1, E_AI_CH_I2S_RX_C_12);
            ret |= HalAudSetAiPath(eAioDma, nDmaCh2, E_AI_CH_I2S_RX_C_13);
            break;

        case E_MHAL_AI_IF_I2S_RX_C_14_15:
            ret |= HalAudSetAiPath(eAioDma, nDmaCh1, E_AI_CH_I2S_RX_C_14);
            ret |= HalAudSetAiPath(eAioDma, nDmaCh2, E_AI_CH_I2S_RX_C_15);
            break;

        case E_MHAL_AI_IF_I2S_RX_D_0_1:
            ret |= HalAudSetAiPath(eAioDma, nDmaCh1, E_AI_CH_I2S_RX_D_0);
            ret |= HalAudSetAiPath(eAioDma, nDmaCh2, E_AI_CH_I2S_RX_D_1);
            break;

        case E_MHAL_AI_IF_I2S_RX_D_2_3:
            ret |= HalAudSetAiPath(eAioDma, nDmaCh1, E_AI_CH_I2S_RX_D_2);
            ret |= HalAudSetAiPath(eAioDma, nDmaCh2, E_AI_CH_I2S_RX_D_3);
            break;

        case E_MHAL_AI_IF_I2S_RX_D_4_5:
            ret |= HalAudSetAiPath(eAioDma, nDmaCh1, E_AI_CH_I2S_RX_D_4);
            ret |= HalAudSetAiPath(eAioDma, nDmaCh2, E_AI_CH_I2S_RX_D_5);
            break;

        case E_MHAL_AI_IF_I2S_RX_D_6_7:
            ret |= HalAudSetAiPath(eAioDma, nDmaCh1, E_AI_CH_I2S_RX_D_6);
            ret |= HalAudSetAiPath(eAioDma, nDmaCh2, E_AI_CH_I2S_RX_D_7);
            break;

        case E_MHAL_AI_IF_I2S_RX_D_8_9:
            ret |= HalAudSetAiPath(eAioDma, nDmaCh1, E_AI_CH_I2S_RX_D_8);
            ret |= HalAudSetAiPath(eAioDma, nDmaCh2, E_AI_CH_I2S_RX_D_9);
            break;

        case E_MHAL_AI_IF_I2S_RX_D_10_11:
            ret |= HalAudSetAiPath(eAioDma, nDmaCh1, E_AI_CH_I2S_RX_D_10);
            ret |= HalAudSetAiPath(eAioDma, nDmaCh2, E_AI_CH_I2S_RX_D_11);
            break;

        case E_MHAL_AI_IF_I2S_RX_D_12_13:
            ret |= HalAudSetAiPath(eAioDma, nDmaCh1, E_AI_CH_I2S_RX_D_12);
            ret |= HalAudSetAiPath(eAioDma, nDmaCh2, E_AI_CH_I2S_RX_D_13);
            break;

        case E_MHAL_AI_IF_I2S_RX_D_14_15:
            ret |= HalAudSetAiPath(eAioDma, nDmaCh1, E_AI_CH_I2S_RX_D_14);
            ret |= HalAudSetAiPath(eAioDma, nDmaCh2, E_AI_CH_I2S_RX_D_15);
            break;

        case E_MHAL_AI_IF_SPDIF_RX_A_0_1:
            ret |= HalAudSetAiPath(eAioDma, nDmaCh1, E_AI_CH_SPDIF_RX_A_0);
            ret |= HalAudSetAiPath(eAioDma, nDmaCh2, E_AI_CH_SPDIF_RX_A_1);
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
    ERRMSG("[%d]enAiDma[%d] nAiDmaChSlotNum[%d] eAiIf[%d]error\n", line, enAiDma, nAiDmaChSlotNum, eAiIf);
    return AIO_NG;
}

static int _MhalAudioAoAttachCore(AO_DMA_e enAoDma, MS_U8 nAoDmaChSlotNum, AO_IF_e eAoIf, BOOL nUseSrc)
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
        ret |= HalAudSrcSetAoPath(eAioDma, nDmaCh, E_AO_CH_DAC_A_0, nUseSrc);
    }

    if (eAoIf & E_MHAL_AO_IF_DAC_B_0)
    {
        ret |= HalAudSrcSetAoPath(eAioDma, nDmaCh, E_AO_CH_DAC_B_0, nUseSrc);
    }

    if (eAoIf & E_MHAL_AO_IF_ECHO_A_0)
    {
        ret |= HalAudSrcSetAoPath(eAioDma, nDmaCh, E_AO_CH_ECHO_A_0, nUseSrc);
    }

    if (eAoIf & E_MHAL_AO_IF_ECHO_A_1)
    {
        ret |= HalAudSrcSetAoPath(eAioDma, nDmaCh, E_AO_CH_ECHO_A_1, nUseSrc);
    }

    if (eAoIf & E_MHAL_AO_IF_I2S_TX_A_0)
    {
        ret |= HalAudSrcSetAoPath(eAioDma, nDmaCh, E_AO_CH_I2S_TX_A_0, nUseSrc);
    }

    if (eAoIf & E_MHAL_AO_IF_I2S_TX_A_1)
    {
        ret |= HalAudSrcSetAoPath(eAioDma, nDmaCh, E_AO_CH_I2S_TX_A_1, nUseSrc);
    }

    if (eAoIf & E_MHAL_AO_IF_I2S_TX_B_0)
    {
        ret |= HalAudSrcSetAoPath(eAioDma, nDmaCh, E_AO_CH_I2S_TX_B_0, nUseSrc);
    }

    if (eAoIf & E_MHAL_AO_IF_I2S_TX_B_1)
    {
        ret |= HalAudSrcSetAoPath(eAioDma, nDmaCh, E_AO_CH_I2S_TX_B_1, nUseSrc);
    }

    if (eAoIf & E_MHAL_AO_IF_I2S_TX_C_0)
    {
        ret |= HalAudSrcSetAoPath(eAioDma, nDmaCh, E_AO_CH_I2S_TX_C_0, nUseSrc);
    }

    if (eAoIf & E_MHAL_AO_IF_I2S_TX_C_1)
    {
        ret |= HalAudSrcSetAoPath(eAioDma, nDmaCh, E_AO_CH_I2S_TX_C_1, nUseSrc);
    }

    //
    if (ret != AIO_OK)
    {
        line = __LINE__;
        goto FAIL;
    }

    return AIO_OK;
FAIL:
    ERRMSG("[%d] enAoDma[%d] nAoDmaChSlotNum[%d] eAoIf[%d]error\n", line, enAoDma, nAoDmaChSlotNum, eAoIf);
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
    int            ret     = AIO_OK;
    U8             nDmaCh  = 0;
    CHIP_AIO_DMA_e eAioDma = 0;

    //
    nDmaCh = nAoDmaChSlotNum;

    eAioDma = CHIP_AIO_DMA_IDX_BY_AO_DMA(enAoDma);
    if (!CHIP_AIO_DMA_IDX_VALID(eAioDma))
    {
        goto FAIL;
    }

    //
    if (eAoIf & E_MHAL_AO_IF_DAC_A_0)
    {
        ret |= HalAudAoDetach(eAioDma, E_AO_CH_DAC_A_0);
    }

    if (eAoIf & E_MHAL_AO_IF_DAC_B_0)
    {
        ret |= HalAudAoDetach(eAioDma, E_AO_CH_DAC_B_0);
    }

    if (eAoIf & E_MHAL_AO_IF_ECHO_A_0)
    {
        ret |= HalAudAoDetach(eAioDma, E_AO_CH_ECHO_A_0);
    }

    if (eAoIf & E_MHAL_AO_IF_ECHO_A_1)
    {
        ret |= HalAudAoDetach(eAioDma, E_AO_CH_ECHO_A_1);
    }

    if (eAoIf & E_MHAL_AO_IF_I2S_TX_A_0)
    {
        ret |= HalAudAoDetach(eAioDma, E_AO_CH_I2S_TX_A_0);
    }

    if (eAoIf & E_MHAL_AO_IF_I2S_TX_A_1)
    {
        ret |= HalAudAoDetach(eAioDma, E_AO_CH_I2S_TX_A_1);
    }

    if (eAoIf & E_MHAL_AO_IF_I2S_TX_B_0)
    {
        ret |= HalAudAoDetach(eAioDma, E_AO_CH_I2S_TX_B_0);
    }

    if (eAoIf & E_MHAL_AO_IF_I2S_TX_B_1)
    {
        ret |= HalAudAoDetach(eAioDma, E_AO_CH_I2S_TX_B_1);
    }

    if (eAoIf & E_MHAL_AO_IF_I2S_TX_C_0)
    {
        ret |= HalAudAoDetach(eAioDma, E_AO_CH_I2S_TX_C_0);
    }

    if (eAoIf & E_MHAL_AO_IF_I2S_TX_C_1)
    {
        ret |= HalAudAoDetach(eAioDma, E_AO_CH_I2S_TX_C_1);
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
    ERRMSG("[%d] enAiDma[%d] nAiDmaChSlotNum[%d] eAiIf[%d]error\n", line, enAiDma, nAiDmaChSlotNum, eAiIf);
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
    ERRMSG("[%d] enAoDma[%d] nAoDmaChSlotNum[%d] eAoIf[%d]error\n", line, enAoDma, nAoDmaChSlotNum, eAoIf);
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
    ret |= _MhalAudioDirectDettachCore(enAoDma, &pDirectAttach);
    if (ret != AIO_OK)
    {
        line = __LINE__;
        goto FAIL;
    }

    return AIO_OK;

FAIL:
    ERRMSG("[%d] enAiDma[%d] nAiDmaChSlotNum[%d] eAiIf[%d]error\n", line, enAiDma, nAiDmaChSlotNum, eAiIf);
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
    ret |= _MhalAudioDirectDettachCore(enAoDma, &pDirectAttach);
    if (ret != AIO_OK)
    {
        line = __LINE__;
        goto FAIL;
    }

    return AIO_OK;

FAIL:
    ERRMSG("[%d] enAoDma[%d] nAoDmaChSlotNum[%d] eAoIf[%d]error\n", line, enAoDma, nAoDmaChSlotNum, eAoIf);
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
        ret |= HalAudSetDirectPath(eAiCh1, aeAoCh1[i]);
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
        ret |= HalAudSetDirectPath(eAiCh2, aeAoCh2[i]);
        if (ret != AIO_OK)
        {
            line = __LINE__;
            goto FAIL;
        }
    }

    return AIO_OK;

FAIL:
    ERRMSG("[%d] error\n", line);
    return AIO_NG;
}

static int _MhalAudioDirectDettachCore(AO_DMA_e enAoDma, MHAL_Direct_Attach_t *pDirectAttach)
{
    int ret = AIO_OK;
    int i   = 0;
    // AI_CH_e eAiCh1, eAiCh2;
    AO_CH_e        aeAoCh1[AO_IF_IN_ONE_DMA_CH_MAX], aeAoCh2[AO_IF_IN_ONE_DMA_CH_MAX];
    MS_U8          nGetNum1 = 0, nGetNum2 = 0;
    int            line    = 0;
    CHIP_AIO_DMA_e eAioDma = 0;

    eAioDma = CHIP_AIO_DMA_IDX_BY_AO_DMA(enAoDma);
    if (!CHIP_AIO_DMA_IDX_VALID(eAioDma))
    {
        goto FAIL;
    }

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
        ret |= HalAudAoDetach(E_CHIP_AIO_DMA_AO_DIRECT_A, aeAoCh1[i]);
        if (ret != AIO_OK)
        {
            line = __LINE__;
            goto FAIL;
        }
    }

    //
    for (i = 0; i < nGetNum2; i++)
    {
        ret |= HalAudAoDetach(E_CHIP_AIO_DMA_AO_DIRECT_A, aeAoCh2[i]);
        if (ret != AIO_OK)
        {
            line = __LINE__;
            goto FAIL;
        }
    }

    return AIO_OK;

FAIL:
    ERRMSG("[%d] error\n", line);
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

    memset(&tI2sCfg, 0, sizeof(AudI2sCfg_t));
    //
    tI2sCfg.enI2sWidth = HalAudApiBitWidthToEnum(pstI2sConfig->u16Width);
    if (E_AUD_BITWIDTH_NULL == tI2sCfg.enI2sWidth)
    {
        line = __LINE__;
        goto FAIL;
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
            // Use default config
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
            // Use default config
            break;
    }

    tI2sCfg.eRate = HalAudApiRateToEnum(pstI2sConfig->commonCfg.u32Rate);
    if (E_AUD_RATE_NULL == tI2sCfg.eRate)
    {
        line = __LINE__;
        goto FAIL;
    }

    switch (pstI2sConfig->enMode)
    {
        case E_MHAL_AUDIO_MODE_TDM_MASTER:
            tI2sCfg.eMsMode = E_AUD_I2S_MSMODE_MASTER;
            break;
        case E_MHAL_AUDIO_MODE_TDM_SLAVE:
            tI2sCfg.eMsMode = E_AUD_I2S_MSMODE_SLAVE;
            break;
        default:
            line = __LINE__;
            break;
    }

    tI2sCfg.nChannelNum = pstI2sConfig->u16Channels;

    //
    ret |= HalAudI2sConfig(eI2s, &tI2sCfg);

    if (ret != AIO_OK)
    {
        line = __LINE__;
        goto FAIL;
    }

    return AIO_OK;

FAIL:
    ERRMSG("[%d] eAiDev %d error\n", line, eAiDev);
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

    memset(&tI2sCfg, 0, sizeof(AudI2sCfg_t));
    //

    tI2sCfg.enI2sWidth = HalAudApiBitWidthToEnum(pstI2sConfig->u16Width);
    if (E_AUD_BITWIDTH_NULL == tI2sCfg.enI2sWidth)
    {
        line = __LINE__;
        goto FAIL;
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
            line = __LINE__;
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
            break;
    }

    tI2sCfg.eRate = HalAudApiRateToEnum(pstI2sConfig->commonCfg.u32Rate);
    if (E_AUD_RATE_NULL == tI2sCfg.eRate)
    {
        line = __LINE__;
        goto FAIL;
    }

    switch (pstI2sConfig->enMode)
    {
        case E_MHAL_AUDIO_MODE_TDM_MASTER:
            tI2sCfg.eMsMode = E_AUD_I2S_MSMODE_MASTER;
            break;
        case E_MHAL_AUDIO_MODE_TDM_SLAVE:
            tI2sCfg.eMsMode = E_AUD_I2S_MSMODE_SLAVE;
            break;
        default:
            line = __LINE__;
            break;
    }

    tI2sCfg.nChannelNum = pstI2sConfig->u16Channels;

    ret |= HalAudI2sConfig(eI2s, &tI2sCfg);

    if (ret != AIO_OK)
    {
        line = __LINE__;
        goto FAIL;
    }

    return AIO_OK;

FAIL:
    ERRMSG("[%d] eAoDev %d error\n", line, eAoDev);
    return AIO_NG;
}

static int _MhalAudioAiIfEnableByStatus(AI_DMA_e enAiDma, AI_IF_e eAiIf, BOOL bEn)
{
    int     ret          = AIO_OK;
    int     v            = 0;
    AI_IF_e aiIf         = 0;
    int     eDmicSel     = 0;
    BOOL    bIsDoSetting = FALSE;
    int     line         = 0;

    aiIf = eAiIf;

    if (!bEn)
    {
        int     i               = 0;
        BOOL    bIsAnyDmaAttach = 0;
        AI_CH_e eAiCh_1, eAiCh_2;
        ret |= _MhalAudioAiIfToDevCh(eAiIf, &eAiCh_1, &eAiCh_2);
        for (i = 0; i < E_MHAL_AI_DMA_TOTAL; i++)
        {
            if (g_pAiDmaStatusList[i].bIsAttached && i != enAiDma)
            {
                bIsAnyDmaAttach = true;
                break;
            }
        }
        if (!bIsAnyDmaAttach)
        {
            g_pAiIfStatusList[eAiCh_1].ncAttachedCount = 0;
            g_pAiIfStatusList[eAiCh_2].ncAttachedCount = 0;
        }
    }

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

                ret |= HalAudAtopEnable((CHIP_AIO_ATOP_e)v, bEn);
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

                ret |= HalAudAtopEnable((CHIP_AIO_ATOP_e)v, bEn);
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

                ret |= HalAudAtopEnable((CHIP_AIO_ATOP_e)v, bEn);
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

                ret |= HalAudAtopEnable((CHIP_AIO_ATOP_e)v, bEn);
                if (ret != AIO_OK)
                {
                    line = __LINE__;
                    goto FAIL;
                }
            }

            break;

        case E_MHAL_AI_IF_DMIC_A_0_1:
        case E_MHAL_AI_IF_DMIC_A_2_3:
        case E_MHAL_AI_IF_DMIC_A_4_5:
        case E_MHAL_AI_IF_DMIC_A_6_7:

            ret |= _MhalAudioAiCheckIfDoSettingByStatus(aiIf, bEn, &bIsDoSetting);
            if (ret != AIO_OK)
            {
                line = __LINE__;
                goto FAIL;
            }

            if (bIsDoSetting == TRUE)
            {
                v = CHIP_AI_DMIC_IDX_BY_DEV(E_AI_DEV_DMIC_A);

                if (aiIf == E_MHAL_AI_IF_DMIC_A_0_1)
                {
                    eDmicSel = E_AUD_MCH_SEL_DMIC01;
                }
                else if (aiIf == E_MHAL_AI_IF_DMIC_A_2_3)
                {
                    eDmicSel = E_AUD_MCH_SEL_DMIC23;
                }
                else if (aiIf == E_MHAL_AI_IF_DMIC_A_4_5)
                {
                    eDmicSel = E_AUD_MCH_SEL_DMIC45;
                }
                else if (aiIf == E_MHAL_AI_IF_DMIC_A_6_7)
                {
                    eDmicSel = E_AUD_MCH_SEL_DMIC67;
                }
                ret |= HalAudDmicEnable((CHIP_AI_DMIC_e)v, bEn, (AudMchSel_e)eDmicSel);
                if (ret != AIO_OK)
                {
                    line = __LINE__;
                    goto FAIL;
                }
            }

            break;

        case E_MHAL_AI_IF_SPDIF_RX_A_0_1:
            ret |= _MhalAudioAiCheckIfDoSettingByStatus(aiIf, bEn, &bIsDoSetting);
            if (ret != AIO_OK)
            {
                line = __LINE__;
                goto FAIL;
            }

            if (bIsDoSetting == TRUE)
            {
                ret |= HalAudSpdifEnable(E_CHIP_AI_SPDIF_A, bEn);
                if (ret != AIO_OK)
                {
                    line = __LINE__;
                    goto FAIL;
                }
            }
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

                ret |= HalAudI2sEnable((CHIP_AIO_I2S_e)v, bEn);
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

                ret |= HalAudI2sEnable((CHIP_AIO_I2S_e)v, bEn);
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

                ret |= HalAudI2sEnable((CHIP_AIO_I2S_e)v, bEn);
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

                ret |= HalAudI2sEnable((CHIP_AIO_I2S_e)v, bEn);
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
    ERRMSG("[%d] enAiDma %d eAiIf[%d] bEn[%d] error\n", line, enAiDma, eAiIf, bEn);
    return AIO_NG;
}

static int _MhalAudioAoIfEnableI2sByStatus(AO_IF_e eAoIf, AO_DEV_e eAoDev, BOOL bEn)
{
    int  ret          = AIO_OK;
    int  v            = 0;
    BOOL bIsDoSetting = FALSE;
    ret |= _MhalAudioAoCheckIfDoSettingByStatus(eAoIf, bEn, &bIsDoSetting);
    if (ret != AIO_OK)
    {
        goto FAIL;
    }

    if (bIsDoSetting == TRUE)
    {
        v = CHIP_AIO_I2S_IDX_BY_AO_DEV_I2S_TX(eAoDev);
        if (!CHIP_AIO_I2S_IDX_VALID(v))
        {
            goto FAIL;
        }

        ret |= HalAudI2sEnable((CHIP_AIO_I2S_e)v, bEn);
        if (ret != AIO_OK)
        {
            goto FAIL;
        }
    }
    return AIO_OK;
FAIL:
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

    HalAudSrcMixStatusUpdate();

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
            v = CHIP_AIO_ATOP_IDX_BY_AO_DEV_DAC(E_AO_DEV_DAC_A); // v = 1  v = 2
            if (!CHIP_AIO_ATOP_IDX_VALID(v))
            {
                printk("v = %d\n", v);
                line = __LINE__;
                goto FAIL;
            }

            ret |= HalAudAtopEnable((CHIP_AIO_ATOP_e)v, bEn);
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

            ret |= HalAudAtopEnable((CHIP_AIO_ATOP_e)v, bEn);
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
        ret |= _MhalAudioAoIfEnableI2sByStatus(E_MHAL_AO_IF_I2S_TX_A_0, E_AO_DEV_I2S_TX_A, bEn);
        if (ret != AIO_OK)
        {
            line = __LINE__;
            goto FAIL;
        }
    }

    if (aoIf & E_MHAL_AO_IF_I2S_TX_A_1)
    {
        ret |= _MhalAudioAoIfEnableI2sByStatus(E_MHAL_AO_IF_I2S_TX_A_1, E_AO_DEV_I2S_TX_A, bEn);
        if (ret != AIO_OK)
        {
            line = __LINE__;
            goto FAIL;
        }
    }

    if (aoIf & E_MHAL_AO_IF_I2S_TX_B_0)
    {
        ret |= _MhalAudioAoIfEnableI2sByStatus(E_MHAL_AO_IF_I2S_TX_B_0, E_AO_DEV_I2S_TX_B, bEn);
        if (ret != AIO_OK)
        {
            line = __LINE__;
            goto FAIL;
        }
    }

    if (aoIf & E_MHAL_AO_IF_I2S_TX_B_1)
    {
        ret |= _MhalAudioAoIfEnableI2sByStatus(E_MHAL_AO_IF_I2S_TX_B_1, E_AO_DEV_I2S_TX_B, bEn);
        if (ret != AIO_OK)
        {
            line = __LINE__;
            goto FAIL;
        }
    }

    if (aoIf & E_MHAL_AO_IF_I2S_TX_C_0)
    {
        ret |= _MhalAudioAoIfEnableI2sByStatus(E_MHAL_AO_IF_I2S_TX_C_0, E_AO_DEV_I2S_TX_C, bEn);
        if (ret != AIO_OK)
        {
            line = __LINE__;
            goto FAIL;
        }
    }

    if (aoIf & E_MHAL_AO_IF_I2S_TX_C_1)
    {
        ret |= _MhalAudioAoIfEnableI2sByStatus(E_MHAL_AO_IF_I2S_TX_C_1, E_AO_DEV_I2S_TX_C, bEn);
        if (ret != AIO_OK)
        {
            line = __LINE__;
            goto FAIL;
        }
    }

    return AIO_OK;

FAIL:
    ERRMSG("[%d] enAoDma %d eAoIf[%d] bEn[%d] error\n", line, enAoDma, eAoIf, bEn);
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

        case E_MHAL_AI_IF_DMIC_A_4_5:
            *peAiCh1 = E_AI_CH_DMIC_A_4;
            *peAiCh2 = E_AI_CH_DMIC_A_5;
            break;

        case E_MHAL_AI_IF_DMIC_A_6_7:
            *peAiCh1 = E_AI_CH_DMIC_A_6;
            *peAiCh2 = E_AI_CH_DMIC_A_7;
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

        case E_MHAL_AI_IF_SPDIF_RX_A_0_1:
            *peAiCh1 = E_AI_CH_SPDIF_RX_A_0;
            *peAiCh2 = E_AI_CH_SPDIF_RX_A_1;
            break;

        default:
            ERRMSG("eAiIf[%d] error\n", eAiIf);
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

    if (eAoIf & E_MHAL_AO_IF_I2S_TX_C_0)
    {
        if (i < AO_IF_IN_ONE_DMA_CH_MAX)
        {
            peAoCh[i] = E_AO_CH_I2S_TX_C_0;
            i++;
        }
        else
        {
            line = __LINE__;
            goto FAIL;
        }
    }

    if (eAoIf & E_MHAL_AO_IF_I2S_TX_C_1)
    {
        if (i < AO_IF_IN_ONE_DMA_CH_MAX)
        {
            peAoCh[i] = E_AO_CH_I2S_TX_C_1;
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
    ERRMSG("[%d] eAoIf[%d] error\n", line, eAoIf);
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
            eAiChEnd   = E_AI_CH_DMIC_A_1;
            break;

        case E_MHAL_AI_IF_DMIC_A_2_3:
            eAiChStart = E_AI_CH_DMIC_A_2;
            eAiChEnd   = E_AI_CH_DMIC_A_3;
            break;

        case E_MHAL_AI_IF_DMIC_A_4_5:
            eAiChStart = E_AI_CH_DMIC_A_4;
            eAiChEnd   = E_AI_CH_DMIC_A_5;
            break;

        case E_MHAL_AI_IF_DMIC_A_6_7:
            eAiChStart = E_AI_CH_DMIC_A_6;
            eAiChEnd   = E_AI_CH_DMIC_A_7;
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

        case E_MHAL_AI_IF_SPDIF_RX_A_0_1:
            eAiChStart = E_AI_CH_SPDIF_RX_A_0;
            eAiChEnd   = E_AI_CH_SPDIF_RX_A_1;
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

        case E_MHAL_AO_IF_ECHO_A_0:
            eAoChStart = E_AO_CH_ECHO_A_0;
            eAoChEnd   = E_AO_CH_ECHO_A_1;
            break;

        case E_MHAL_AO_IF_ECHO_A_1:
            eAoChStart = E_AO_CH_ECHO_A_0;
            eAoChEnd   = E_AO_CH_ECHO_A_1;
            break;

        case E_MHAL_AO_IF_I2S_TX_A_0:
        case E_MHAL_AO_IF_I2S_TX_A_1:
            eAoChStart = E_AO_CH_I2S_TX_A_0;
            eAoChEnd   = E_AO_CH_I2S_TX_A_1;
            break;

        case E_MHAL_AO_IF_I2S_TX_B_0:
        case E_MHAL_AO_IF_I2S_TX_B_1:
            eAoChStart = E_AO_CH_I2S_TX_B_0;
            eAoChEnd   = E_AO_CH_I2S_TX_B_1;
            break;

        case E_MHAL_AO_IF_I2S_TX_C_0:
        case E_MHAL_AO_IF_I2S_TX_C_1:
            eAoChStart = E_AO_CH_I2S_TX_C_0;
            eAoChEnd   = E_AO_CH_I2S_TX_C_1;
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
                // printk("g_pAiDigMicStatusList[%d].anDmicChAttachedCount[%d] = %d, nDmicChNum = %d\n", i, j,
                //        g_pAiDigMicStatusList[i].anDmicChAttachedCount[j], nDmicChNum);
            }

            //
            if (bIsDoSetting == TRUE)
            {
                eChipDmic = i;

                ret |= HalAudDmicSetChannel(eChipDmic, nDmicChNum);
                if (ret != AIO_OK)
                {
                    ERRMSG("eChipDmic %d error\n", eChipDmic);
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
    u8 nChn     = 0;

    switch (eAiIf)
    {
        case E_MHAL_AI_IF_DMIC_A_0_1:
            nDmicIdx = (E_AI_DEV_DMIC_A - E_AI_DEV_DMIC_START);
            nChn     = 0;
            break;
        case E_MHAL_AI_IF_DMIC_A_2_3:
            nDmicIdx = (E_AI_DEV_DMIC_A - E_AI_DEV_DMIC_START);
            nChn     = 2;
            break;
        case E_MHAL_AI_IF_DMIC_A_4_5:
            nDmicIdx = (E_AI_DEV_DMIC_A - E_AI_DEV_DMIC_START);
            nChn     = 4;
            break;
        case E_MHAL_AI_IF_DMIC_A_6_7:
            nDmicIdx = (E_AI_DEV_DMIC_A - E_AI_DEV_DMIC_START);
            nChn     = 6;
            break;
        default:
            return AIO_OK;
    }

    if (bEn)
    {
        if (g_pAiDigMicStatusList[nDmicIdx].anDmicChAttachedCount[nChn] < DMIC_CH_MAX * E_MHAL_AI_DMA_TOTAL)
        {
            g_pAiDigMicStatusList[nDmicIdx].anDmicChAttachedCount[nChn]++;
        }
        if (g_pAiDigMicStatusList[nDmicIdx].anDmicChAttachedCount[nChn + 1] < DMIC_CH_MAX * E_MHAL_AI_DMA_TOTAL)
        {
            g_pAiDigMicStatusList[nDmicIdx].anDmicChAttachedCount[nChn + 1]++;
        }
    }
    else
    {
        if (g_pAiDigMicStatusList[nDmicIdx].anDmicChAttachedCount[nChn] != 0)
        {
            g_pAiDigMicStatusList[nDmicIdx].anDmicChAttachedCount[nChn]--;
        }
        else
        {
            // ERRMSG("eAiIf[%d] nDmicIdx[%d] DmicCh[%d] error\n", eAiIf, nDmicIdx, nChn);
            goto FAIL;
        }

        if (g_pAiDigMicStatusList[nDmicIdx].anDmicChAttachedCount[nChn + 1] != 0)
        {
            g_pAiDigMicStatusList[nDmicIdx].anDmicChAttachedCount[nChn + 1]--;
        }
        else
        {
            // ERRMSG("eAiIf[%d] nDmicIdx[%d] DmicCh[%d] error\n", eAiIf, nDmicIdx, nChn);
            goto FAIL;
        }
    }

    pbIsDigMicNeedUpdate[nDmicIdx] = TRUE;

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
        if (g_pAiIfStatusList[eAiCh1].ncAttachedCount == 0 || g_pAiIfStatusList[eAiCh2].ncAttachedCount == 0)
        {
            goto SUCCESS;
        }

        g_pAiIfStatusList[eAiCh1].ncAttachedCount--;
        g_pAiIfStatusList[eAiCh2].ncAttachedCount--;
    }

SUCCESS:

    return AIO_OK;

FAIL:
    ERRMSG("[%d] eAiIf %d eAiCh1 %d eAiCh2 %d bIsAttach %d error\n", line, eAiIf, eAiCh1, eAiCh2, bIsAttach);
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
                ret |= HalAudAoIfMultiAttachAction(aeAoCh[i], g_pAoIfStatusList[aeAoCh[i]].ncAttachedCount);
                if (ret != AIO_OK)
                {
                    line = __LINE__;
                    goto FAIL;
                }
            }
            else if (g_pAoIfStatusList[aeAoCh[i]].ncAttachedCount > 2)
            {
                ERRMSG("g_pAoIfStatusList[%d].ncAttachedCount = %d > 2\n", aeAoCh[i],
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

                ret |= HalAudAoIfMultiAttachAction(aeAoCh[i], g_pAoIfStatusList[aeAoCh[i]].ncAttachedCount);
                if (ret != AIO_OK)
                {
                    line = __LINE__;
                    goto FAIL;
                }
            }
            else if (g_pAoIfStatusList[aeAoCh[i]].ncAttachedCount > 2)
            {
                ERRMSG("g_pAoIfStatusList[%d].ncAttachedCount = %d > 2\n", aeAoCh[i],
                       g_pAoIfStatusList[aeAoCh[i]].ncAttachedCount);
                line = __LINE__;
                goto FAIL;
            }
        }
    }

SUCCESS:

    return AIO_OK;

FAIL:
    ERRMSG("eAoIf[%d] error\n", eAoIf);
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
            ERRMSG("aiDma direct %d error\n", aiDma);
            line = __LINE__;
            goto FAIL;
        }
    }
    else
    {
        if (!CHIP_AI_DMA_VALID(aiDma))
        {
            ERRMSG("aiDma[%d] error\n", aiDma);
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
                // line = __LINE__;
                // goto FAIL;
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
    ERRMSG("[%d] enAiDma %d error\n", line, enAiDma);
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
    if (aoDma >= E_MHAL_AO_DMA_TOTAL)
    {
        line = __LINE__;
        goto FAIL;
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
    ERRMSG("[%d] enAoDma %d error\n", line, enAoDma);
    return AIO_NG;
}

// -------------------------------------------------------------------------------
MS_S32 mhal_audio_init(void *pdata)
{
    int ret  = AIO_OK;
    int line = 0;

    DBGMSG(AUDIO_DBG_LEVEL_PATH, "start init g_nInitialed = %d\n", g_nInitialed);

    CamOsMutexLock(&g_tAudInitMutex);

    if (g_nInitialed == 0)
    {
        DBGMSG(AUDIO_DBG_LEVEL_TEST, "start init\n");

        //
        ret |= HalAudMainInit();
        if (ret != AIO_OK)
        {
            line = __LINE__;
            CamOsMutexUnlock(&g_tAudInitMutex);
            goto FAIL;
        }

        //
        ret |= AudioProcInit();
        if (ret != AIO_OK)
        {
            line = __LINE__;
            CamOsMutexUnlock(&g_tAudInitMutex);
            goto FAIL;
        }
    }

    g_nInitialed++;

    CamOsMutexUnlock(&g_tAudInitMutex);

    return MHAL_SUCCESS;

FAIL:
    ERRMSG("error\n");
    return MHAL_FAILURE;
}

MS_S32 mhal_audio_deinit(void)
{
    int ret  = AIO_OK;
    int line = 0;

    DBGMSG(AUDIO_DBG_LEVEL_PATH, "start deinit g_nInitialed = %d\n", g_nInitialed);

    CamOsMutexLock(&g_tAudInitMutex);

    if (g_nInitialed == 1)
    {
        memset(g_pAoDmaStatusList, 0, sizeof(MHAL_AO_Dma_Status_t) * E_MHAL_AO_DMA_TOTAL);
        memset(g_pAiDmaStatusList, 0, sizeof(MHAL_AI_Dma_Status_t) * E_MHAL_AI_DMA_TOTAL);
        memset(g_pAoIfStatusList, 0, sizeof(MHAL_AO_If_Ch_Status_t) * E_AO_CH_TOTAL);
        memset(g_pAiIfStatusList, 0, sizeof(MHAL_AI_If_Ch_Status_t) * E_AI_CH_TOTAL);
        //
        ret |= AudioProcDeInit();
        if (ret != AIO_OK)
        {
            line = __LINE__;
            CamOsMutexUnlock(&g_tAudInitMutex);
            goto FAIL;
        }

        //
        ret |= HalAudMainDeInit();
        if (ret != AIO_OK)
        {
            line = __LINE__;
            CamOsMutexUnlock(&g_tAudInitMutex);
            goto FAIL;
        }

        DBGMSG(AUDIO_DBG_LEVEL_PATH, "mhal_audio_deinit \n");
    }

    g_nInitialed--;

    CamOsMutexUnlock(&g_tAudInitMutex);

    return MHAL_SUCCESS;

FAIL:
    ERRMSG("error\n");
    return MHAL_FAILURE;
}

MS_S32 mhal_audio_ai_if_setting(MHAL_AI_IF_e enAiIf, void *pAiSetting)
{
    int ret      = AIO_OK;
    int v        = 0;
    int line     = 0;
    int eDmicSel = 0;

    //
    DBGMSG(AUDIO_DBG_LEVEL_PATH, "enAiIf = %d\n", enAiIf);
    switch (enAiIf)
    {
        case E_MHAL_AI_IF_ADC_A_0_B_0:
        case E_MHAL_AI_IF_ADC_C_0_D_0:
            ret |= HalAudAtopAiSetSampleRate(enAiIf, ((MHAL_AUDIO_CommonCfg_t *)pAiSetting)->u32Rate);
            break;

        case E_MHAL_AI_IF_DMIC_A_0_1:
        case E_MHAL_AI_IF_DMIC_A_2_3:
        case E_MHAL_AI_IF_DMIC_A_4_5:
        case E_MHAL_AI_IF_DMIC_A_6_7:
            ret |= HalAudDmicSetRate(E_CHIP_AI_DMIC_A, ((MHAL_AUDIO_CommonCfg_t *)pAiSetting)->u32Rate);

            v = CHIP_AI_DMIC_IDX_BY_DEV(E_AI_DEV_DMIC_A);

            if (enAiIf == E_MHAL_AI_IF_DMIC_A_0_1)
            {
                eDmicSel = E_AUD_MCH_SEL_DMIC01;
            }
            else if (enAiIf == E_MHAL_AI_IF_DMIC_A_2_3)
            {
                eDmicSel = E_AUD_MCH_SEL_DMIC23;
            }
            else if (enAiIf == E_MHAL_AI_IF_DMIC_A_4_5)
            {
                eDmicSel = E_AUD_MCH_SEL_DMIC45;
            }
            else if (enAiIf == E_MHAL_AI_IF_DMIC_A_6_7)
            {
                eDmicSel = E_AUD_MCH_SEL_DMIC67;
            }
            ret |= HalAudDmicEnable((CHIP_AI_DMIC_e)v, true, eDmicSel);
            if (ret != AIO_OK)
            {
                line = __LINE__;
                goto FAIL;
            }
            break;

        case E_MHAL_AI_IF_ECHO_A_0_1:
            ret |= HalAudSrcConfig(E_AI_CH_ECHO_A_0, ((MHAL_AUDIO_CommonCfg_t *)pAiSetting)->u32Rate);
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

        case E_MHAL_AI_IF_SPDIF_RX_A_0_1:
            ret |= HalAudSpdifSetRate(E_CHIP_AI_SPDIF_A, ((MHAL_AUDIO_CommonCfg_t *)pAiSetting)->u32Rate);
            ret |= HalAudSpdifEnable(E_CHIP_AI_SPDIF_A, true);
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
    ERRMSG("[%d] error enAiIf is %d\n", line, enAiIf);
    return MHAL_FAILURE;
}

MS_S32 mhal_audio_ao_if_setting(MHAL_AO_IF_e enAoIf, void *pAoSetting, MHAL_AO_PATH_RATE_t *aoAttribute)
{
    int            ret     = AIO_OK;
    int            line    = 0;
    CHIP_AIO_DMA_e eAioDma = 0;

    //
    eAioDma = CHIP_AIO_DMA_IDX_BY_AO_DMA(aoAttribute->nDma);
    if (!CHIP_AIO_DMA_IDX_VALID(eAioDma))
    {
        line = __LINE__;
        goto FAIL;
    }

    // AO_CH_e eAoCh
    switch (enAoIf)
    {
        case E_MHAL_AO_IF_DAC_A_0:
        case E_MHAL_AO_IF_DAC_B_0:
        case E_MHAL_AO_IF_ECHO_A_0:
        case E_MHAL_AO_IF_ECHO_A_1:
            ret |= HalAudAoPathSampleSet(enAoIf, eAioDma, aoAttribute->nDmaSampleRate, aoAttribute->nIfSampleRate);
            break;

        case E_MHAL_AO_IF_I2S_TX_A_0:
        case E_MHAL_AO_IF_I2S_TX_A_1:
            ret |= _MhalAudioAoConfigI2s(E_AO_DEV_I2S_TX_A, (MHAL_AUDIO_I2sCfg_t *)pAoSetting);
            ret |= HalAudAoPathSampleSet(enAoIf, eAioDma, aoAttribute->nDmaSampleRate, aoAttribute->nIfSampleRate);
            break;

        case E_MHAL_AO_IF_I2S_TX_B_0:
        case E_MHAL_AO_IF_I2S_TX_B_1:
            ret |= _MhalAudioAoConfigI2s(E_AO_DEV_I2S_TX_B, (MHAL_AUDIO_I2sCfg_t *)pAoSetting);
            ret |= HalAudAoPathSampleSet(enAoIf, eAioDma, aoAttribute->nDmaSampleRate, aoAttribute->nIfSampleRate);
            break;

        case E_MHAL_AO_IF_I2S_TX_C_0:
        case E_MHAL_AO_IF_I2S_TX_C_1:
            ret |= _MhalAudioAoConfigI2s(E_AO_DEV_I2S_TX_C, (MHAL_AUDIO_I2sCfg_t *)pAoSetting);
            ret |= HalAudAoPathSampleSet(enAoIf, eAioDma, aoAttribute->nDmaSampleRate, aoAttribute->nIfSampleRate);
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
    ERRMSG("[%d] error enAoIf is %d\n", line, enAoIf);
    return MHAL_FAILURE;
}

MS_S32 mhal_audio_ai_if_gain(MHAL_AI_IF_e enAiIf, MS_S8 s8LeftIfGain, MS_S8 s8RightIfGain)
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
    ret |= HalAudAiSetGain(eAiCh1, (S16)s8LeftIfGain);
    if (ret != AIO_OK)
    {
        line = __LINE__;
        goto FAIL;
    }

    ret |= HalAudAiSetGain(eAiCh2, (S16)s8RightIfGain);
    if (ret != AIO_OK)
    {
        line = __LINE__;
        goto FAIL;
    }

    return MHAL_SUCCESS;

FAIL:
    ERRMSG("[%d] enAiIf[%d] s8LeftIfGain[%d] s8RightIfGain[%d] error\n", line, enAiIf, s8LeftIfGain, s8RightIfGain);
    return MHAL_FAILURE;
}

MS_S32 mhal_audio_ao_if_gain(MHAL_AO_IF_e enAoIf, MS_S8 s8IfGain)
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
        ret |= HalAudAoSetGain(aeAoCh[i], (S16)s8IfGain);
        if (ret != AIO_OK)
        {
            line = __LINE__;
            goto FAIL;
        }
    }

    return MHAL_SUCCESS;

FAIL:
    ERRMSG("[%d] enAoIf %d s8IfGain[%d] error\n", line, enAoIf, s8IfGain);
    return MHAL_FAILURE;
}

MS_S32 mhal_audio_ai_config(MHAL_AI_Dma_e enAiDma, MHAL_AUDIO_PcmCfg_t *pstDmaConfig)
{
    int            ret = AIO_OK;
    DmaPar_t       tParam;
    CHIP_AIO_DMA_e eAioDma = 0;
    AI_DMA_e       aiDma   = 0;
    int            line    = 0;

    //
    aiDma = enAiDma;

    //
    if (aiDma >= E_MHAL_AI_DMA_TOTAL)
    {
        ERRMSG("aiDma[%d] error\n", aiDma);
        line = __LINE__;
        goto FAIL;
    }

    //
    g_pAiDmaStatusList[aiDma].nSampleRate = pstDmaConfig->u32Rate;

    //
    if (aiDma >= AI_DMA_DIRECT_START)
    {
        ret = HalAudSrcConfigInputClock(pstDmaConfig->u32Rate);
        if (ret != AIO_OK)
        {
            line = __LINE__;
            goto FAIL;
        }
    }
    else
    {
        eAioDma = CHIP_AIO_DMA_IDX_BY_AI_DMA(aiDma);

        //
        switch (pstDmaConfig->u16Width)
        {
            case 16:
            case 24:
            case 32:
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
        tParam.nPeriodCnt    = pstDmaConfig->u32PeriodCnt;
        tParam.nBitWidth     = pstDmaConfig->u16Width;
        tParam.nSampleRate   = pstDmaConfig->u32Rate;
        tParam.nInterleaved  = pstDmaConfig->bInterleaved;

        if ((tParam.nChannels != 1) && (tParam.nChannels != 2) && (tParam.nChannels != 4) && (tParam.nChannels != 8)
            && (tParam.nChannels != 16))
        {
            ERRMSG("enAiDma[%d] AttachedChNum[%d] not support\n", enAiDma, tParam.nChannels);
            line = __LINE__;
            goto FAIL;
        }

        if (tParam.bIsOnlyEvenCh == TRUE)
        {
            ret |= HalAudDmaMonoEnable(eAioDma, TRUE, FALSE);
            if (ret != AIO_OK)
            {
                line = __LINE__;
                goto FAIL;
            }
        }
        else
        {
            ret |= HalAudDmaMonoEnable(eAioDma, FALSE, FALSE);
            if (ret != AIO_OK)
            {
                line = __LINE__;
                goto FAIL;
            }
        }

        // Dma
        ret |= HalAudDmaConfig(eAioDma, &tParam);
        if (ret != AIO_OK)
        {
            line = __LINE__;
            goto FAIL;
        }
    }

    return MHAL_SUCCESS;

FAIL:
    ERRMSG("[%d] enAiDma[%d] error\n", line, enAiDma);
    return MHAL_FAILURE;
}

MS_S32 mhal_audio_ao_config(MHAL_AO_Dma_e enAoDma, MHAL_AUDIO_PcmCfg_t *pstDmaConfig)
{
    int ret = AIO_OK;
    // int i = 0;
    DmaPar_t       tParam;
    CHIP_AIO_DMA_e eAioDma = 0;
    AO_DMA_e       aoDma   = 0;
    int            line    = 0;

    //
    aoDma = enAoDma;

    //
    if (aoDma >= E_MHAL_AO_DMA_TOTAL)
    {
        ERRMSG("aoDma[%d] error\n", aoDma);
        line = __LINE__;
        goto FAIL;
    }

    //
    g_pAoDmaStatusList[aoDma].nSampleRate = pstDmaConfig->u32Rate;

    //
    if (aoDma >= AO_DMA_DIRECT_START)
    {
        HalAudDirectDmaRateSet(pstDmaConfig->u32Rate);
        eAioDma = CHIP_AIO_DMA_IDX_BY_AO_DMA(aoDma);
        HalAudDmaSetOnlyEvenCh(eAioDma, pstDmaConfig->u8IsOnlyEvenCh);
    }
    else
    {
        eAioDma = CHIP_AIO_DMA_IDX_BY_AO_DMA(aoDma);
        //
        switch (pstDmaConfig->u16Width)
        {
            case 16:
            case 24:
            case 32:
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
        tParam.nBitWidth     = pstDmaConfig->u16Width;
        tParam.nSampleRate   = pstDmaConfig->u32Rate;
        tParam.nPeriodSize   = pstDmaConfig->u32PeriodSize;
        tParam.nPeriodCnt    = pstDmaConfig->u32PeriodCnt;
        tParam.nInterleaved  = pstDmaConfig->bInterleaved;

        //
        if (tParam.bIsOnlyEvenCh == TRUE)
        {
            if (pstDmaConfig->enChMode == E_MHAL_AO_CH_MODE_DOUBLE_MONO)
            {
                ret |= HalAudDmaMonoEnable(eAioDma, TRUE, TRUE);
            }
            else
            {
                ret |= HalAudDmaMonoEnable(eAioDma, TRUE, FALSE);
            }

            if (ret != AIO_OK)
            {
                line = __LINE__;
                goto FAIL;
            }
        }
        else
        {
            ret |= HalAudDmaMonoEnable(eAioDma, FALSE, FALSE);
            if (ret != AIO_OK)
            {
                line = __LINE__;
                goto FAIL;
            }
        }

        //
        ret |= HalAudDmaConfig(eAioDma, &tParam);
        if (ret != AIO_OK)
        {
            line = __LINE__;
            goto FAIL;
        }
    }

    return MHAL_SUCCESS;

FAIL:
    ERRMSG("[%d] aiDma %d error\n", line, aoDma);
    return MHAL_FAILURE;
}

MS_S32 mhal_audio_ai_open(MHAL_AI_Dma_e enAiDma)
{
    int            ret     = AIO_OK;
    CHIP_AIO_DMA_e eAioDma = 0;
    AI_DMA_e       aiDma   = 0;
    AI_IF_e        eAiIf   = 0;
    int            line    = 0;

    eAioDma = CHIP_AIO_DMA_IDX_BY_AI_DMA(enAiDma);

    //
    aiDma = enAiDma;

    //
    if (aiDma >= E_MHAL_AI_DMA_TOTAL)
    {
        ERRMSG("aiDma[%d] error\n", aiDma);
        line = __LINE__;
        goto FAIL;
    }

    //
    if (aiDma >= AI_DMA_DIRECT_START)
    {
        eAiIf = g_pAiDmaStatusList[aiDma].eAiIf[0];

        ret |= HalAudAtopAiSetSampleRate(eAiIf, g_pAiDmaStatusList[aiDma].nSampleRate);
        if (ret != AIO_OK)
        {
            line = __LINE__;
            goto FAIL;
        }
    }
    else
    {
        eAioDma = CHIP_AIO_DMA_IDX_BY_AI_DMA(aiDma);

        // Open
        ret |= HalAudDmaOpen(eAioDma);
        if (ret != AIO_OK)
        {
            line = __LINE__;
            goto FAIL;
        }

        // Prepare
        ret |= HalAudDmaPrepare(eAioDma);
        if (ret != AIO_OK)
        {
            line = __LINE__;
            goto FAIL;
        }
    }

    return MHAL_SUCCESS;

FAIL:
    ERRMSG("[%d] enAiDma %d error\n", line, enAiDma);
    return MHAL_FAILURE;
}

MS_S32 mhal_audio_ao_open(MHAL_AO_Dma_e enAoDma)
{
    int            ret     = AIO_OK;
    CHIP_AIO_DMA_e eAioDma = 0;
    AO_DMA_e       aoDma   = 0;
    AO_IF_e        eAoIf   = 0;
    int            line    = 0;
    //
    aoDma = enAoDma;

    if (aoDma >= E_MHAL_AO_DMA_TOTAL)
    {
        line = __LINE__;
        goto FAIL;
    }

    //
    if (aoDma >= AO_DMA_DIRECT_START)
    {
        eAoIf = g_pAoDmaStatusList[aoDma].eAoIf[0];

        ret |= HalAudAoSetIfSampleRate(eAoIf, g_pAoDmaStatusList[aoDma].nSampleRate);
        if (ret != AIO_OK)
        {
            line = __LINE__;
            goto FAIL;
        }
    }
    else
    {
        eAioDma = CHIP_AIO_DMA_IDX_BY_AO_DMA(aoDma);

        // Open
        ret |= HalAudDmaOpen(eAioDma);
        if (ret != AIO_OK)
        {
            line = __LINE__;
            goto FAIL;
        }

        // Prepare
        ret |= HalAudDmaPrepare(eAioDma);
        if (ret != AIO_OK)
        {
            line = __LINE__;
            goto FAIL;
        }
    }

    return MHAL_SUCCESS;

FAIL:
    ERRMSG("[%d] enAoDma[%d] error\n", line, enAoDma);
    return MHAL_FAILURE;
}

MS_S32 mhal_audio_ai_attach(MHAL_AI_Attach_t *pAiAttach)
{
    int            ret                                    = AIO_OK;
    int            i                                      = 0;
    AI_DMA_e       aiDma                                  = 0;
    AI_IF_e        aiIf                                   = 0;
    BOOL           aIsDigMicNeedUpdate[AI_DEV_DMIC_TOTAL] = {0};
    CHIP_AIO_DMA_e eAioDma;

    AI_ATTACH_t aiAttach;
    int         line = 0;

    //
    aiDma   = pAiAttach->enAiDma;
    eAioDma = CHIP_AIO_DMA_IDX_BY_AI_DMA(aiDma);

    if (aiDma >= E_MHAL_AI_DMA_TOTAL)
    {
        line = __LINE__;
        goto FAIL;
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
        // Mch
        aiAttach.enAiDma = aiDma;

        for (i = 0; i < E_MHAL_AI_DMA_CH_SLOT_TOTAL; i++)
        {
            aiAttach.eAiIf[i] = g_pAiDmaStatusList[aiDma].eAiIf[i];
        }

        ret |= HalAudAiSetClkRefAndMch(&aiAttach);
        if (ret != AIO_OK)
        {
            line = __LINE__;
            goto FAIL;
        }
    }

    return MHAL_SUCCESS;

FAIL:
    ERRMSG("[%d] enAiDma[%d] error\n", line, aiDma);
    return MHAL_FAILURE;
}

MS_S32 mhal_audio_ai_detach(MHAL_AI_Attach_t *pAiAttach)
{
    int         ret                                    = AIO_OK;
    int         i                                      = 0;
    AI_DMA_e    aiDma                                  = 0;
    AI_IF_e     aiIf                                   = 0;
    BOOL        aIsDigMicNeedUpdate[AI_DEV_DMIC_TOTAL] = {0};
    int         line                                   = 0;
    BOOL        aIsMchNeedUpdate                       = FALSE;
    AI_ATTACH_t aiAttach;

    //
    aiDma = pAiAttach->enAiDma;

    if (aiDma >= E_MHAL_AI_DMA_TOTAL)
    {
        line = __LINE__;
        goto FAIL;
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
                aiIf = g_pAiDmaStatusList[aiDma].eAiIf[i];
                ret |= _MhalAudioAiDirectDettachCore(aiDma, i, aiIf);
                if (ret != AIO_OK)
                {
                    line = __LINE__;
                    goto FAIL;
                }
            }
            else
            {
                aiIf = g_pAiDmaStatusList[aiDma].eAiIf[i];
                if (aiIf == E_MHAL_AI_IF_NONE)
                {
                    return MHAL_SUCCESS;
                }
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

            //
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
    if (aiDma >= AI_DMA_DIRECT_START)
    {
        // Do nothing
    }
    else
    {
        // Dmic
        ret |= _MhalAudioAiDmicChNumCfg(aIsDigMicNeedUpdate);
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
            if (aiAttach.eAiIf[i] != 0)
            {
                aIsMchNeedUpdate = TRUE;
            }
        }

        if (aIsMchNeedUpdate)
        {
            ret |= HalAudAiSetClkRefAndMch(&aiAttach);
            if (ret != AIO_OK)
            {
                line = __LINE__;
                goto FAIL;
            }
        }
    }
    return MHAL_SUCCESS;

FAIL:
    ERRMSG("error line[%d]\n", line);
    return MHAL_FAILURE;
}

MS_S32 mhal_audio_ao_attach(MHAL_AO_Attach_t *pAoAttach)
{
    int         ret      = AIO_OK;
    int         i        = 0;
    AO_DMA_e    aoDma    = 0;
    AO_IF_e     aoIf     = 0;
    MS_U8       nAoDmaCh = 0;
    AO_ATTACH_t aoAttach;
    int         line    = 0;
    BOOL        nUseSrc = 0;
    //
    mutex_lock(&g_AudioAttachmutex);

    aoDma   = pAoAttach->enAoDma;
    nUseSrc = pAoAttach->nUseSrc;

    if (aoDma >= E_MHAL_AO_DMA_TOTAL)
    {
        line = __LINE__;
        goto FAIL;
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
            ret |= _MhalAudioAoAttachCore(aoDma, nAoDmaCh, aoIf, nUseSrc);
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

    aoAttach.enAoDma = aoDma;

    for (i = 0; i < E_MHAL_AO_DMA_CH_SLOT_TOTAL; i++)
    {
        aoAttach.eAoIf[i] = g_pAoDmaStatusList[aoDma].eAoIf[i];
    }

    ret |= HalAudAoSetClkRef(&aoAttach);
    if (ret != AIO_OK)
    {
        line = __LINE__;
        goto FAIL;
    }

    mutex_unlock(&g_AudioAttachmutex);
    return MHAL_SUCCESS;

FAIL:
    mutex_unlock(&g_AudioAttachmutex);
    ERRMSG("[%d] aoDma[%d] error\n", line, aoDma);
    return MHAL_FAILURE;
}

MS_S32 mhal_audio_ao_detach(MHAL_AO_Attach_t *pAoAttach)
{
    int         ret      = AIO_OK;
    AO_DMA_e    aoDma    = 0;
    AO_IF_e     aoIf     = 0;
    MS_U8       nAoDmaCh = 0;
    int         line     = 0;
    int         bSetting = 0;
    int         i        = 0;
    AO_ATTACH_t aoAttach;

    mutex_lock(&g_AudioDetachmutex);
    //
    aoDma = pAoAttach->enAoDma;

    if (aoDma >= E_MHAL_AO_DMA_TOTAL)
    {
        line = __LINE__;
        goto FAIL;
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
            HalAudAoSetClkUnRef(&aoAttach);
        }
    }

    mutex_unlock(&g_AudioDetachmutex);
    return MHAL_SUCCESS;

FAIL:
    mutex_unlock(&g_AudioDetachmutex);
    ERRMSG("[%d] aoDma[%d] error\n", line, aoDma);
    return MHAL_FAILURE;
}

MS_S32 mhal_audio_ai_close(MHAL_AI_Dma_e enAiDma)
{
    int            ret     = AIO_OK;
    CHIP_AIO_DMA_e eAioDma = 0;
    AI_DMA_e       aiDma   = 0;
    int            line    = 0;

    //
    aiDma = enAiDma;

    //
    if (aiDma >= E_MHAL_AI_DMA_TOTAL)
    {
        line = __LINE__;
        goto FAIL;
    }

    //
    if (aiDma >= AI_DMA_DIRECT_START)
    {
    }
    else
    {
        eAioDma = CHIP_AIO_DMA_IDX_BY_AI_DMA(aiDma);

        // Close
        ret |= HalAudDmaClose(eAioDma);
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
    ERRMSG("[%d] enAiDma[%d] error\n", line, enAiDma);
    return MHAL_FAILURE;
}

MS_S32 mhal_audio_ao_close(MHAL_AO_Dma_e enAoDma)
{
    int            ret     = AIO_OK;
    CHIP_AIO_DMA_e eAioDma = 0;
    AO_DMA_e       aoDma   = 0;
    int            line    = 0;

    //
    aoDma = enAoDma;

    //
    if (aoDma >= E_MHAL_AO_DMA_TOTAL)
    {
        line = __LINE__;
        goto FAIL;
    }

    //
    if (aoDma >= AO_DMA_DIRECT_START)
    {
    }
    else
    {
        eAioDma = CHIP_AIO_DMA_IDX_BY_AO_DMA(aoDma);

        // Close
        ret |= HalAudDmaClose(eAioDma);
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
    ERRMSG("[%d] enAoDma[%d] error\n", line, enAoDma);
    return MHAL_FAILURE;
}

MS_S32 mhal_audio_ai_start(MHAL_AI_Dma_e enAiDma)
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
        ERRMSG("enAiDma [%d] bIsAttached FALSE\n", enAiDma);
        line = __LINE__;
        goto FAIL;
    }

    eAioDma = CHIP_AIO_DMA_IDX_BY_AI_DMA(aiDma);

    // Start
    ret |= HalAudDmaStart(eAioDma);
    if (ret != AIO_OK)
    {
        line = __LINE__;
        goto FAIL;
    }

    return MHAL_SUCCESS;

FAIL:
    ERRMSG("[%d] enAiDma[%d] error\n", line, enAiDma);
    return MHAL_FAILURE;
}

MS_S32 mhal_audio_ao_start(MHAL_AO_Dma_e enAoDma)
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
        ERRMSG("enAoDma[%d] bIsAttached FALSE\n", enAoDma);
        line = __LINE__;
        goto FAIL;
    }

    eAioDma = CHIP_AIO_DMA_IDX_BY_AO_DMA(aoDma);

    // Start
    ret |= HalAudDmaStart(eAioDma);
    if (ret != AIO_OK)
    {
        line = __LINE__;
        goto FAIL;
    }

    return MHAL_SUCCESS;

FAIL:
    ERRMSG("[%d] enAoDma[%d] error\n", line, enAoDma);
    return MHAL_FAILURE;
}

MS_S32 mhal_audio_ai_stop(MHAL_AI_Dma_e enAiDma)
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
        ERRMSG("enAiDma[%d] bIsAttached FALSE\n", enAiDma);
        line = __LINE__;
        goto FAIL;
    }

    eAioDma = CHIP_AIO_DMA_IDX_BY_AI_DMA(aiDma);

    // Stop
    ret |= HalAudDmaStop(eAioDma);
    if (ret != AIO_OK)
    {
        line = __LINE__;
        goto FAIL;
    }

    return MHAL_SUCCESS;

FAIL:
    ERRMSG("[%d] enAiDma[%d] error\n", line, enAiDma);
    return MHAL_FAILURE;
}

MS_S32 mhal_audio_ao_stop(MHAL_AO_Dma_e enAoDma)
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

    eAioDma = CHIP_AIO_DMA_IDX_BY_AO_DMA(aoDma);

    // Stop
    ret |= HalAudDmaStop(eAioDma);
    if (ret != AIO_OK)
    {
        line = __LINE__;
        goto FAIL;
    }

    return MHAL_SUCCESS;

FAIL:
    ERRMSG("[%d] enAoDma[%d] error\n", line, enAoDma);
    return MHAL_FAILURE;
}

MS_S32 mhal_audio_ai_read_trig(MHAL_AI_Dma_e enAiDma, MS_U32 u32Frames)
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
        return MHAL_FAILURE;
    }

    //
    if (g_pAiDmaStatusList[aiDma].bIsAttached == FALSE)
    {
        ERRMSG("enAiDma[%d] bIsAttached = FALSE\n", enAiDma);
        line = __LINE__;
        return MHAL_FAILURE;
    }

    eAioDma = CHIP_AIO_DMA_IDX_BY_AI_DMA(aiDma);

    // Read
    s32Err = HalAudDmaTrigPcm(eAioDma, u32Frames);

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

SUCCESS:
    return s32Err;
}

MS_S32 mhal_audio_ao_write_trig(MHAL_AO_Dma_e enAoDma, MS_U32 u32Frames)
{
    // int ret = AIO_OK;
    CHIP_AIO_DMA_e eAioDma = 0;
    AO_DMA_e       aoDma   = 0;
    MS_S32         s32Err;

    aoDma = enAoDma;

    if (!CHIP_AO_DMA_VALID(aoDma))
    {
        return MHAL_FAILURE;
    }

    eAioDma = CHIP_AIO_DMA_IDX_BY_AO_DMA(aoDma);

    // Write
    s32Err = HalAudDmaTrigPcm(eAioDma, u32Frames);

    if (s32Err >= 0)
    {
        goto SUCCESS;
    }
    else
    {
        if (s32Err == -EAGAIN) // Non-block mode, Resource temporarily unavailable.
        {
            ERRMSG("MHAL_FAILURE_NO_RESOURCE\n");
            return MHAL_FAILURE_NO_RESOURCE;
        }

        return MHAL_FAILURE;
    }

SUCCESS:
    return s32Err;
}

MS_S32 mhal_audio_ai_preparetorestart(MHAL_AI_Dma_e enAiDma)
{
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
        ERRMSG("enAiDma[%d], bIsAttached = FALSE\n", enAiDma);
        line = __LINE__;
        goto FAIL;
    }

    eAioDma = CHIP_AIO_DMA_IDX_BY_AI_DMA(aiDma);

    //
    HalAudDmaPrepareRestart(eAioDma);

    return MHAL_SUCCESS;

FAIL:
    ERRMSG("[%d] enAiDma[%d] error\n", line, enAiDma);
    return MHAL_FAILURE;
}

MS_S32 mhal_audio_ao_preparetorestart(MHAL_AO_Dma_e enAoDma)
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

    eAioDma = CHIP_AIO_DMA_IDX_BY_AO_DMA(aoDma);

    //
    ret |= HalAudDmaStop(eAioDma);
    if (ret != AIO_OK)
    {
        line = __LINE__;
        goto FAIL;
    }

    return MHAL_SUCCESS;

FAIL:
    ERRMSG("[%d] enAoDma[%d] error\n", line, enAoDma);
    return MHAL_FAILURE;
}

MS_S32 mhal_audio_ai_get_curr_datalen(MHAL_AI_Dma_e enAiDma, MS_U32 *len)
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
        ERRMSG("enAiDma[%d] bIsAttached = FALSE\n", enAiDma);
        line = __LINE__;
        goto FAIL;
    }

    eAioDma = CHIP_AIO_DMA_IDX_BY_AI_DMA(aiDma);

    *len = HalAudDmaGetLevelCnt(eAioDma);
    if (ret != AIO_OK)
    {
        line = __LINE__;
        goto FAIL;
    }

    return MHAL_SUCCESS;

FAIL:
    ERRMSG("[%d] enAiDma[%d] error\n", line, enAiDma);
    return MHAL_FAILURE;
}

MS_S32 mhal_audio_ao_get_curr_datalen(MHAL_AO_Dma_e enAoDma, MS_U32 *len)
{
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

    eAioDma = CHIP_AIO_DMA_IDX_BY_AO_DMA(aoDma);

    *len = HalAudDmaGetLevelCnt(eAioDma);

    return MHAL_SUCCESS;

FAIL:
    ERRMSG("[%d] enAoDma[%d] error\n", line, enAoDma);
    return MHAL_FAILURE;
}

MS_BOOL mhal_audio_ai_is_dmafree(MHAL_AI_Dma_e enAiDma)
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

MS_BOOL mhal_audio_ao_is_dmafree(MHAL_AO_Dma_e enAoDma)
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

MS_S32 mhal_audio_ao_pause(MHAL_AO_Dma_e enAoDma)
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
        ERRMSG("enAoDma[%d] bIsAttached = FALSE\n", enAoDma);
        line = __LINE__;
        goto FAIL;
    }

    eAioDma = CHIP_AIO_DMA_IDX_BY_AO_DMA(aoDma);

    // Stop
    ret |= HalAudDmaPause(eAioDma);
    if (ret != AIO_OK)
    {
        line = __LINE__;
        goto FAIL;
    }

    return MHAL_SUCCESS;

FAIL:
    ERRMSG("[%d] enAoDma[%d] error\n", line, enAoDma);
    return MHAL_FAILURE;
}

MS_S32 mhal_audio_ao_resume(MHAL_AO_Dma_e enAoDma)
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
        ERRMSG("enAoDma[%d] bIsAttached = FALSE\n", enAoDma);
        line = __LINE__;
        goto FAIL;
    }

    eAioDma = CHIP_AIO_DMA_IDX_BY_AO_DMA(aoDma);

    // Resume
    ret |= HalAudDmaResume(eAioDma);
    if (ret != AIO_OK)
    {
        line = __LINE__;
        goto FAIL;
    }

    return MHAL_SUCCESS;

FAIL:
    ERRMSG("[%d] enAoDma[%d] error\n", line, enAoDma);
    return MHAL_FAILURE;
}

MS_S32 mhal_audio_ai_resume(MHAL_AI_Dma_e enAiDma)
{
    int            ret     = AIO_OK;
    CHIP_AIO_DMA_e eAioDma = 0;
    AO_DMA_e       aiDma   = 0;
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
        ERRMSG("enAiDma[%d] bIsAttached = FALSE\n", enAiDma);
        line = __LINE__;
        goto FAIL;
    }

    eAioDma = CHIP_AIO_DMA_IDX_BY_AI_DMA(aiDma);

    // Resume
    ret |= HalAudDmaResume(eAioDma);
    if (ret != AIO_OK)
    {
        line = __LINE__;
        goto FAIL;
    }

    return MHAL_SUCCESS;

FAIL:
    ERRMSG("[%d] enAiDma[%d] error\n", line, enAiDma);
    return MHAL_FAILURE;
}

MS_S32 mhal_audio_ai_pause(MHAL_AI_Dma_e enAiDma)
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
        ERRMSG("enAiDma[%d] bIsAttached = FALSE\n", enAiDma);
        line = __LINE__;
        goto FAIL;
    }

    eAioDma = CHIP_AIO_DMA_IDX_BY_AI_DMA(aiDma);

    // Resume
    ret |= HalAudDmaPause(eAioDma);
    if (ret != AIO_OK)
    {
        line = __LINE__;
        goto FAIL;
    }

    return MHAL_SUCCESS;

FAIL:
    ERRMSG("[%d] enAiDma[%d] error\n", line, enAiDma);
    return MHAL_FAILURE;
}

MS_S32 mhal_audio_ai_interrupt_get(MHAL_AI_Dma_e enAiDma, struct sstar_interupt_status *status,
                                   struct sstar_interupt_flag *flag)
{
    AI_DMA_e       aiDma   = 0;
    CHIP_AIO_DMA_e eAioDma = 0;
    int            line    = 0;

    aiDma = enAiDma;

    if (!CHIP_AI_DMA_VALID(aiDma))
    {
        line = __LINE__;
        goto FAIL;
    }

    eAioDma = CHIP_AIO_DMA_IDX_BY_AI_DMA(aiDma);

    HalAudDmaGetFlags(eAioDma, &flag->trigger_flag, &flag->boundary_flag, &flag->local_data_flag, &flag->transmit_flag);
    HalAudDmaGetInt(eAioDma, &status->trigger_int, &status->boundary_int, &status->local_data_int);

    return MHAL_SUCCESS;
FAIL:
    return MHAL_FAILURE;
}

MS_S32 mhal_audio_ai_interrupt_set(MHAL_AI_Dma_e enAiDma, struct sstar_interupt_en *irq_en)
{
    AI_DMA_e       aiDma   = 0;
    CHIP_AIO_DMA_e eAioDma = 0;
    int            line    = 0;

    aiDma = enAiDma;

    if (!CHIP_AI_DMA_VALID(aiDma))
    {
        line = __LINE__;
        goto FAIL;
    }

    eAioDma = CHIP_AIO_DMA_IDX_BY_AI_DMA(aiDma);

    HalAudDmaIntEnable(eAioDma, irq_en->trigger_en, irq_en->boundary_en, irq_en->transmit_en);

    return MHAL_SUCCESS;
FAIL:
    return MHAL_FAILURE;
}

MS_S32 mhal_audio_ao_interrupt_get(MHAL_AO_Dma_e enAoDma, struct sstar_interupt_status *status,
                                   struct sstar_interupt_flag *flag)
{
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

    eAioDma = CHIP_AIO_DMA_IDX_BY_AO_DMA(aoDma);

    HalAudDmaGetFlags(eAioDma, &flag->trigger_flag, &flag->boundary_flag, &flag->local_data_flag, &flag->transmit_flag);
    HalAudDmaGetInt(eAioDma, &status->trigger_int, &status->boundary_int, &status->local_data_int);

    return MHAL_SUCCESS;
FAIL:
    return MHAL_FAILURE;
}

MS_S32 mhal_audio_ao_interrupt_set(MHAL_AO_Dma_e enAoDma, struct sstar_interupt_en *irq_en)
{
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

    eAioDma = CHIP_AIO_DMA_IDX_BY_AO_DMA(aoDma);

    HalAudDmaIntEnable(eAioDma, irq_en->trigger_en, irq_en->boundary_en, irq_en->transmit_en);

    return MHAL_SUCCESS;
FAIL:
    return MHAL_FAILURE;
}

MS_S32 mhal_audio_ao_set_channel_mode(MHAL_AO_Dma_e enAoDma, MHAL_AO_ChMode_e enChMode)
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

    eAioDma = CHIP_AIO_DMA_IDX_BY_AO_DMA(aoDma);

    ret |= HalAudDmaSetChannelMode(eAioDma, (AudAoChMode_e)enChMode);
    if (ret != AIO_OK)
    {
        line = __LINE__;
        goto FAIL;
    }

    return MHAL_SUCCESS;

FAIL:
    ERRMSG("[%d] Ao[%d] SetChannelMode[%d] error\n", line, enAoDma, enChMode);
    return MHAL_FAILURE;
}

MS_S32 mhal_audio_set_audio_delay(MHAL_AO_Dma_e enAoDma, MS_BOOL bCtl)
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

    eAioDma = CHIP_AIO_DMA_IDX_BY_AO_DMA(aoDma);

    ret |= HalAudSetAudioDelay(eAioDma, bCtl);
    if (ret != AIO_OK)
    {
        line = __LINE__;
        goto FAIL;
    }

    return MHAL_SUCCESS;

FAIL:
    ERRMSG("[%d] Ao[%d] set_audio_delay[%d] error\n", line, enAoDma, bCtl);
    return MHAL_FAILURE;
}

MS_S32 mhal_audio_mclk_setting(MHAL_MCLK_ID_e mclk_id, int mclk_freq, MS_BOOL bEnable)
{
    AudI2sMck_e eMck = HalAudApiMckToEnum(mclk_freq);
    if (HalAudI2sSetMck(mclk_id, eMck, bEnable))
    {
        goto FAIL;
    }

    return MHAL_SUCCESS;
FAIL:
    return MHAL_FAILURE;
}

void mhal_audio_module_init(void)
{
    CamOsMutexInit(&g_tAudInitMutex);
}

void mhal_audio_module_deinit(void)
{
    CamOsMutexDestroy(&g_tAudInitMutex);
}

MS_S32 mhal_audio_runtime_power_ctl(MS_BOOL bEnable)
{
    int ret  = AIO_OK;
    int line = 0;

    ret |= HalAudRuntimePowerCtl(bEnable);
    if (ret != AIO_OK)
    {
        line = __LINE__;
        goto FAIL;
    }

    return MHAL_SUCCESS;

FAIL:
    ERRMSG("[%d]mhal_audio_runtime_power_ctl[%d] error\n", line, bEnable);
    return MHAL_FAILURE;
}

MS_S32 mhal_audio_ao_sidetone_regulate(MHAL_AO_Dma_e enAoDma, MS_U8 u8ChId, int nVolume,
                                       MHAL_AUDIO_GainFading_e enFading)
{
    int            ret     = AIO_OK;
    AO_DMA_e       aoDma   = 0;
    CHIP_AIO_DMA_e eAioDma = 0;
    int            line    = 0;

    //
    aoDma = enAoDma;

    if (!CHIP_AO_DMA_VALID(aoDma))
    {
        line = __LINE__;
        goto FAIL;
    }

    if (g_pAoDmaStatusList[aoDma].bIsAttached == FALSE)
    {
        ERRMSG("aoDma[%d], bIsAttached FALSE\n", aoDma);
        line = __LINE__;
        goto FAIL;
    }

    //
    eAioDma = CHIP_AIO_DMA_IDX_BY_AO_DMA(aoDma);

    ret |= HalAudAoSetSideToneDpga(eAioDma, u8ChId, nVolume, enFading);
    if (ret != AIO_OK)
    {
        line = __LINE__;
        goto FAIL;
    }

    return MHAL_SUCCESS;

FAIL:
    ERRMSG("[%d] enAoDma %d u8ChId[%d] sidetone_regulate  error\n", line, enAoDma, u8ChId);
    return MHAL_FAILURE;
}

MS_S32 mhal_audio_amp_state_set(int nEnable, MS_U8 u8ChId)
{
    int ret  = AIO_OK;
    int line = 0;

    ret |= HalAudApiAoAmpEnable(nEnable, u8ChId);
    if (ret != AIO_OK)
    {
        line = __LINE__;
        goto FAIL;
    }

    return MHAL_SUCCESS;

FAIL:
    ERRMSG("[%d]  mhal_audio_amp_state_set[%d] error\n", line, nEnable);
    return MHAL_FAILURE;
}

MS_S32 mhal_audio_amp_state_get(int *nEnable, MS_U8 u8ChId)
{
    int ret  = AIO_OK;
    int line = 0;

    ret |= HalAudApiAoAmpStateGet(nEnable, u8ChId);
    if (ret != AIO_OK)
    {
        line = __LINE__;
        goto FAIL;
    }

    return MHAL_SUCCESS;
FAIL:
    ERRMSG("[%d]  mhal_audio_amp_state_get error\n", line);
    return MHAL_FAILURE;
}

MS_S32 mhal_audio_local_fifo_irq(struct snd_pcm_substream *substream, int dma_id)
{
    CHIP_AIO_DMA_e eAioDma;
    U32            nHwAddr, nApplPtr, nLevelCnt, nDiff, nBufferBytes, nDiffCmp;
    if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK)
    {
        eAioDma = CHIP_AIO_DMA_IDX_BY_AO_DMA(dma_id);
    }
    else
    {
        eAioDma = CHIP_AIO_DMA_IDX_BY_AI_DMA(dma_id);
    }
    switch (eAioDma)
    {
        case E_CHIP_AIO_DMA_AO_A:
        case E_CHIP_AIO_DMA_AO_B:
        case E_CHIP_AIO_DMA_AO_C:
            nDiffCmp = 32;
            break;
        case E_CHIP_AIO_DMA_AI_A:
        case E_CHIP_AIO_DMA_AI_B:
            nDiffCmp = 0;
            break;
        default:
            return MHAL_SUCCESS;
    }
    nHwAddr   = HalAudDmaGetHwAddr(eAioDma);
    nLevelCnt = HalAudDmaGetLevelCnt(eAioDma);
    nApplPtr =
        frames_to_bytes(substream->runtime, substream->runtime->control->appl_ptr % substream->runtime->buffer_size);
    nBufferBytes = frames_to_bytes(substream->runtime, substream->runtime->buffer_size);
    DBGMSG(AUDIO_DBG_LEVEL_IRQ, "[%s]local fifo empty swaddr[%ld] hwaddr[%d] level[%d]\n",
           (substream->stream == SNDRV_PCM_STREAM_PLAYBACK) ? ("playback") : ("capture"), nApplPtr, nHwAddr, nLevelCnt);
    if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK)
    {
        if (nHwAddr > nApplPtr)
        {
            nDiff = nBufferBytes - (nHwAddr - nApplPtr) - nLevelCnt;
        }
        else
        {
            nDiff = nApplPtr - nHwAddr - nLevelCnt;
        }
    }
    else
    {
        if (nHwAddr >= nApplPtr)
        {
            nDiff = nHwAddr - nApplPtr - nLevelCnt;
        }
        else
        {
            nDiff = nBufferBytes - (nApplPtr - nHwAddr) - nLevelCnt;
        }
    }
    if (nDiff != nDiffCmp)
    {
        ERRMSG("Dma[%d] swptr[%d] hwptr[%d] level[%d] nDiff[%d]\n", eAioDma, nApplPtr, nHwAddr, nLevelCnt, nDiff);
    }
    HalAudDmaIntLocalEnable(eAioDma, 0);

    return MHAL_SUCCESS;
}

MS_U32 mhal_audio_get_dts_value(DTS_CONFIG_KEY_e key)
{
    return HalAudApiGetDtsValue(key);
}

MS_U32 mhal_audio_clear_int(struct snd_pcm_substream *substream, int dma_id)
{
    CHIP_AIO_DMA_e eAioDma;
    if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK)
    {
        eAioDma = CHIP_AIO_DMA_IDX_BY_AO_DMA(dma_id);
    }
    else
    {
        eAioDma = CHIP_AIO_DMA_IDX_BY_AI_DMA(dma_id);
    }

    return HalAudDmaClearInt(eAioDma);
}
