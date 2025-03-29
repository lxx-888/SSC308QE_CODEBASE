/*
 * hal_audio_api.h - Sigmastar
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

#ifndef __HAL_AUD_API_H__
#define __HAL_AUD_API_H__

typedef long PcmFrames_t;

typedef struct
{
    U32  nSrcMuxIndex;
    U32  nSrcMixerIndex;
    bool nSrcUse;
} SrcPathAttr_t;

typedef struct
{
    u8 *          pDmaBuf;
    long          nPhysDmaAddr;
    u32           nBufferSize;
    u16           nChannels;
    u16           nBitWidth;
    AudRate_e     nSampleRate;
    u32           nPeriodSize;
    u32           nPeriodCnt;
    u16           nInterleaved;
    u8            bIsOnlyEvenCh;
    SrcFrom_e     nClockRef;
    SrcPathAttr_t nLeftPath;
    SrcPathAttr_t nRightPath;
    bool          nUseSrc;
} DmaPar_t;

typedef enum
{
    E_DMA_STATUS_INIT,
    E_DMA_STATUS_SETUP,
    E_DMA_STATUS_OPEN,
    E_DMA_STATUS_PREPARED,
    E_DMA_STATUS_RUNNING,
    E_DMA_STATUS_XRUN,
    E_DMA_STATUS_PAUSED,
} DmaStatus_e;

typedef struct
{
    u32 nSampleRate;
    u8  nChNum;

} DigMicParam_t;

typedef struct
{
    u32 nSampleRate;
} SrcParam_t;

typedef struct
{
    u16             nChnId;    /* Channel ID */
    ss_miu_addr_t   tDmaAddr;  /* physical bus address (not accessible from main CPU) */
    u32             nBufBytes; /* Size of the DMA transfer */
    PcmFrames_t     tBufSize;  /* Size of the DMA transfer */
    PcmFrames_t     tPeriodSize;
    CamOsSpinlock_t tLock;
} PcmDmaData_t;

typedef struct PcmRuntimeData_s
{
    CamOsSpinlock_t tPcmLock;
    void *          pPrivateData;
    u16             nFrameBits;
    u16             nHwFrameBits;
    DmaStatus_e     nStatus;
} PcmRuntimeData_t;

extern int HalAudMainInit(void);
extern int HalAudMainDeInit(void);
extern int HalAudAoDetach(CHIP_AIO_DMA_e eAioDma, AO_CH_e aoch);
/****************** PLL **************************/
extern void HalAudPllEnable(S8 parent_clk_index, S8 clk_index, BOOL bEn, S64 nFreq);
/****************** ATOP **************************/
extern int  HalAudAtopInit(void);
extern int  HalAudAtopEnable(CHIP_AIO_ATOP_e eAtop, BOOL bEnable);
extern int  HalAudAtopSetAdcMux(CHIP_AI_ADC_e eAdc, CHIP_ADC_MUX_e eAdcMux);
extern int  HalAudAtopSwap(BOOL bEn);
extern int  HalAudAtopAiSetSampleRate(AI_IF_e eAiIf, U32 nSampleRate);
extern int  HalAudAtopAiSetGain(AI_CH_e eAiCh, S16 s16Gain);
extern int  HalAudAtopAoSetMute(AO_CH_e eAoCh, BOOL bEnable);
extern void HalAudAtopDeInit(void);
/****************** DMA **************************/
extern int  HalAudDmaInit(void);
extern int  HalAudDmaStart(CHIP_AIO_DMA_e eAioDma);
extern int  HalAudDmaStop(CHIP_AIO_DMA_e eAioDma);
extern int  HalAudDmaResume(CHIP_AIO_DMA_e eAioDma);
extern int  HalAudDmaPause(CHIP_AIO_DMA_e eAioDma);
extern int  HalAudDmaPrepare(CHIP_AIO_DMA_e eAioDma);
extern int  HalAudDmaClose(CHIP_AIO_DMA_e eAioDma);
extern int  HalAudDmaConfig(CHIP_AIO_DMA_e eAioDma, DmaPar_t *ptParam);
extern int  HalAudDmaOpen(CHIP_AIO_DMA_e eAioDma);
extern int  HalAudDmaTrigPcm(CHIP_AIO_DMA_e eAioDma, U32 nFrames);
extern U32  HalAudDmaGetLevelCnt(CHIP_AIO_DMA_e eAioDma);
extern int  HalAudDmaSetChannelMode(CHIP_AIO_DMA_e enAoDma, AudAoChMode_e enChMode);
extern void HalAudDmaSetOnlyEvenCh(CHIP_AIO_DMA_e eAioDma, BOOL bIsOnlyEvenCh);
extern int  HalAudDmaReset(CHIP_AIO_DMA_e eAioDma);
extern int  HalAudDmaEnable(CHIP_AIO_DMA_e eAioDma, BOOL bEnable);
extern int  HalAudDmaSetRate(CHIP_AIO_DMA_e eAioDma, AudRate_e eRate);
extern int  HalAudDmaIntEnable(CHIP_AIO_DMA_e eAioDma, BOOL bDatatrigger, BOOL bDataboundary, BOOL bDataTranslate);
extern int  HalAudDmaIntLocalEnable(CHIP_AIO_DMA_e eAioDma, BOOL bLocalfifo);
extern int  HalAudDmaGetInt(CHIP_AIO_DMA_e eAioDma, BOOL *bDatatrigger, BOOL *bDataboundary, BOOL *bDataLocal);
extern int  HalAudDmaClearInt(CHIP_AIO_DMA_e eAioDma);
extern void HalAudDmaClearIrqStatus(CHIP_AIO_DMA_e eAioDma);
extern int  HalAudDmaSetPhyAddr(CHIP_AIO_DMA_e eAioDma, U64 nBufAddrOffset, U32 nBufSize);
extern int  HalAudDmaAiSetThreshold(CHIP_AIO_DMA_e eAioDma, U32 nOverrunTh);
extern int  HalAudDmaAoSetThreshold(CHIP_AIO_DMA_e eAioDma, U32 nUnderrunTh);
extern int  HalAudDmaAoSetTransThreshold(CHIP_AIO_DMA_e eAioDma, U32 nPeriodCnt);
extern int  HalAudDmaAiSetTransThreshold(CHIP_AIO_DMA_e eAioDma, U32 nPeriodCnt);
extern int  HalAudDmaGetFlags(CHIP_AIO_DMA_e eAioDma, BOOL *pbDatatrigger, BOOL *pbDataboundary, BOOL *pbLocalData,
                              BOOL *pbTransmit);
extern int  HalAudDmaMonoEnable(CHIP_AIO_DMA_e eAioDma, BOOL bEn, BOOL bMonoCopyEn);
extern int  HalAudDmaWrConfigMchCore(EN_CHIP_AI_MCH eMchSet, U16 nChNum, AudMchClkRef_e eMchClkRef,
                                     AudMchSel_e *mch_sel);
extern U32  HalAudDmaGetHwAddr(CHIP_AIO_DMA_e eAioDma);
extern int  HalAudDmaLocalDebug(CHIP_AIO_DMA_e eAioDma, U32 time);
extern U32  HalAudDmaPrepareRestart(CHIP_AIO_DMA_e eAioDma);
extern CHIP_AO_DMA_e HalAudDmaGetAoEnum(CHIP_AIO_DMA_e eAioDma, U8 nDmaCh);
extern void          HalAudDmaDeInit(void);
/****************** I2S **************************/
extern int  HalAudI2sSetBckClkSrc(CHIP_AIO_I2S_e eAioI2s, CLK_SRC_e clk_src);
extern int  HalAudI2sSetMckClkSrc(CHIP_AIO_I2S_e eAioI2s, CLK_SRC_e clk_src);
extern int  HalAudI2sSetTdmDetails(CHIP_AIO_I2S_e eAioI2s);
extern int  HalAudI2sSetRate(CHIP_AIO_I2S_e eAioI2s, AudRate_e eRate);
extern int  HalAudI2sSetMckDivAndEnable(CHIP_AIO_I2S_e eAioI2s);
extern int  HalAudI2sSetClkRef(CHIP_AIO_I2S_e eAioI2s, AudI2sClkRef_e eI2sClkRef);
extern int  HalAudI2sSetTdmMode(CHIP_AIO_I2S_e eAioI2s, AudI2sMode_e enI2sMode);
extern int  HalAudI2sSetMsMode(CHIP_AIO_I2S_e eAioI2s, AudI2sMsMode_e eMsMode);
extern int  HalAudI2sSetFmt(CHIP_AIO_I2S_e eAioI2s, AudI2sFmt_e enI2sFmt);
extern int  HalAudI2sSetWidth(CHIP_AIO_I2S_e eAioI2s, AudBitWidth_e enI2sWidth);
extern int  HalAudI2sSetChannel(CHIP_AIO_I2S_e eAioI2s, U16 nChannel);
extern int  HalAudI2sSetWireMode(CHIP_AIO_I2S_e eAioI2s, AudWireMode_e eWireMode);
extern int  HalAudI2sSetTdmSlotConfig(CHIP_AIO_I2S_e eAioI2s, U16 nSlotMsk, AudTdmChnMap_e eMap);
extern int  HalAudI2sEnableMck(CHIP_AIO_I2S_e eAioI2s, BOOL bEn);
extern int  HalAudI2sSetMck(MHAL_MCLK_ID_e eMclkID, AudI2sMck_e eMck, BOOL bEn);
extern int  HalAudI2sEnableBck(CHIP_AIO_I2S_e eAioI2s, BOOL bEn);
extern int  HalAudI2sEnable(CHIP_AIO_I2S_e eAioI2s, BOOL bEn);
extern BOOL HalAudI2sGetEnable(CHIP_AIO_I2S_e eAioI2s);
extern void HalAudI2sSetDriving(U8 level);
extern int  HalAudI2sTxDataMute(AUDIO_TDM_e nTdm, BOOL bEnable);
extern int  HalAudI2sUpdateCfg(CHIP_AIO_I2S_e eAioI2s, AudI2sCfg_t *ptI2sCfg);
extern int  HalAudI2sConfig(CHIP_AIO_I2S_e eAioI2s, AudI2sCfg_t *ptI2sCfg);
extern int  HalAudI2sGetChannel(CHIP_AIO_I2S_e eAioI2s);

/****************** DMIC **************************/
extern int HalAudDmicInit(void);
extern int HalAudDmicEnable(CHIP_AI_DMIC_e eDmic, BOOL bEn, AudMchSel_e eSel);
extern int HalAudDmicSetChannel(CHIP_AI_DMIC_e eDmic, U16 nCh);
extern int HalAudDmicSetRate(CHIP_AI_DMIC_e eDmic, U32 eRate);

/****************** SINEGEN **************************/
extern int HalAudSineGenEnable(SINE_GEN_e enSineGen, BOOL bEn);
extern int HalAudSineGenSetting(SINE_GEN_e enSineGen, BOOL bEnable, U8 u8Freq, S8 s8Gain);
extern int HalAudSineGenSetFreq(SINE_GEN_e enSineGen, U8 u8Freq);
extern int HalAudSineGenSetGain(SINE_GEN_e enSineGen, S8 s8Gain);
extern int HalAudSineGenGetEnable(SINE_GEN_e enSineGen, BOOL *pbEn);
extern int HalAudSineGenGetSetting(SINE_GEN_e enSineGen, U8 *pu8Freq, S8 *ps8Gain);

/***************** SRC ******************************/
extern int  HalAudSrcConfig(AI_CH_e eAiCh, u32 nSampleRate);
extern int  HalAudSrcSetRate(AI_CH_e eAiCh, AudRate_e eRate);
extern void HalAudSrcMixStatusUpdate(void);
extern int  HalAudSrcConfigInputClock(u32 nSampleRate);
extern int  HalAudSrcSetOutSampleRate(AudMchSel_e Src, u32 nSampleRate);
extern void HalAudSrcGetAoMixInfo(int nMixId, int *from, int *to);
extern int  HalAudSrcSetAoPath(CHIP_AIO_DMA_e eAioDma, U8 nDmaCh, AO_CH_e eAoCh, BOOL nUseSrc);
extern int  HalAudSrcCreatePath(CHIP_AIO_DMA_e eAioDma, U8 nDmaCh, AO_CH_e eAoCh, AI_CH_e eAiCh);
extern int  HalAudSrcMonoDmaSelChannel(CHIP_AIO_DMA_e eAioDma, AudAoChMode_e enChMode);
extern int  HalAudSrcStereoDmaSelChannel(CHIP_AIO_DMA_e eAioDma, AudAoChMode_e enChMode);

/***************** SPDIF ******************************/
extern int HalAudSpdifEnable(CHIP_AI_SPDIF_e eSpdif, BOOL bEn);
extern int HalAudSpdifSetRate(CHIP_AI_SPDIF_e eSpdif, U32 nSampleRate);
extern int HalAudSpdifSetPadmux(CHIP_AI_SPDIF_e eSpdif);

/***************** MISC ******************************/
extern int  HalAudAiSetGain(AI_CH_e eAiCh, S16 s16Gain);
extern int  HalAudAoSetGain(AO_CH_e eAoCh, S16 s16Gain);
extern int  HalAudAoSetIfSampleRate(AO_IF_e eAoIf, u32 nSampleRate);
extern int  HalAudAoSetSideToneDpga(CHIP_AIO_DMA_e eAioDma, U8 nDmaCh, U32 nVolume, U8 nFading);
extern int  HalAudDirectSetDpgaGain(AI_CH_e eAiCh, AO_CH_e eAoCh, S16 s16Gain);
extern int  HalAudAoIfMultiAttachAction(AO_CH_e eAoCh, U8 nAttachCount);
extern BOOL HalAudSetHpf(AudHpfDev_e eHfpDev, BOOL bEnable, U8 level);
extern int  HalAudAoPathSampleSet(AO_IF_e eAoIf, CHIP_AIO_DMA_e eAioDma, u32 eDmaRate, u32 eIfRate);
extern int  HalAudRuntimePowerCtl(BOOL bEn);
extern void HalAudKeepDpgaGainCompatible(AudRate_e eRate);
extern int  HalAudGetAoPathInfo(int nAoId, bool lsLeft, int *nMixId, int *nMuxId, bool *nSrcUse, int *nDmaSampleRate,
                                int *nCache, int *nWrPos, int *nSetCache, int *nLevelCnt);
extern int  HalAudAiSetClkRefAndMch(AI_ATTACH_t *aiAttach);
extern int  HalAudAoSetClkRef(AO_ATTACH_t *aoAttach);
extern int  HalAudAoSetClkUnRef(AO_ATTACH_t *aoAttach);
extern int  HalAudSetAudioDelay(CHIP_AIO_DMA_e eAioDma, BOOL bCtl);
extern int  HalAudDpgaSetGain(EN_CHIP_DPGA eDpga, S8 gain, S8 ch);
extern int  HalAudDpgaSetGainOffset(EN_CHIP_DPGA eDpga, S8 gainRegValue, S8 ch);
extern int  HalAudDpgaSetGainFading(EN_CHIP_DPGA eDpga, U8 nFading, S8 ch);
extern int  HalAudSetAiPath(CHIP_AIO_DMA_e eAioDma, U8 nDmaCh, AI_CH_e eAiCh);
extern int  HalAudSetDirectPath(AI_CH_e eAiCh, AO_CH_e eAoCh);
extern int  HalAudSetMux(AudMux_e eMux, U8 nChoice);
extern void HalAudDirectDmaRateSet(int nSampleRate);
extern int  HalAudSetBachPLLandMclkFreq(CLK_SRC_e src_clk, U32 nBachPll);

#endif //__HAL_AUD_API_H__
