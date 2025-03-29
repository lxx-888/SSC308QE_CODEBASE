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

typedef struct
{
    u32 nSampleRate;
    u8  nChNum;

} DigMicParam_t;

typedef struct
{
    u32 nSampleRate;

} SrcParam_t;

extern int   HalAudMainInit(void);
extern int   HalAudMainDeInit(void);
extern int   HalAudSetAiPath(CHIP_AIO_DMA_e eAioDma, U8 nDmaCh, AI_CH_e eAiCh);
extern int   HalAudSetAoPath(CHIP_AIO_DMA_e eAioDma, U8 nDmaCh, AO_CH_e eAoCh);
extern int   HalAudSetDirectPath(AI_CH_e eAiCh, AO_CH_e eAoCh);
extern int   HalAudAoDetach(AO_CH_e aoch);
extern int   HalAudSetMux(AudMux_e eMux, U8 nChoice);
extern int   HalAudSrcSetRate(AI_CH_e eAiCh, AudRate_e eRate);
extern int   HalAudAtopOpen(CHIP_AIO_ATOP_e eAtop);
extern int   HalAudAtopClose(CHIP_AIO_ATOP_e eAtop);
extern int   HalAudAtopMicAmpGain(CHIP_AI_ADC_e eAdc, U16 nSel);
extern int   HalAudAtopSetAdcGain(CHIP_AI_ADC_e eAdc, U16 nSel);
extern int   HalAudAtopSetAdcMux(CHIP_AI_ADC_e eAdc, CHIP_ADC_MUX_e eAdcMux);
extern int   HalAudAtopSwap(BOOL bEn);
extern int   HalAudI2sSaveRxTdmWsPgm(BOOL bEnable);
extern int   HalAudI2sSaveTdmWsPgm(AUDIO_TDM_e nTdm, BOOL bEnable);
extern int   HalAudI2sSaveRxTdmWsWidth(U8 nWsWidth);
extern int   HalAudI2sSaveRxTdmWsInv(BOOL bEnable);
extern int   HalAudI2sSaveTdmWsInv(AUDIO_TDM_e nTdm, BOOL bEnable);
extern int   HalAudI2sSaveTdmBckInv(AUDIO_TDM_e nTdm, BOOL bEnable);
extern int   HalAudI2sSaveRxTdmChSwap(U8 nSwap_0_2, U8 nSwap_0_4);
extern int   HalAudI2sSaveTdmChSwap(AUDIO_TDM_e nTdm, U8 nSwap_0_2, U8 nSwap_0_4, U8 nSwap_L_R);
extern int   HalAudI2sSetRxMode(U8 mode);
extern int   HalAudI2sSaveTxTdmWsPgm(BOOL bEnable);
extern int   HalAudI2sSaveTxTdmWsWidth(U8 nWsWidth);
extern int   HalAudI2sSaveTdmWsWidth(AUDIO_TDM_e nTdm, U8 nWsWidth);
extern int   HalAudI2sSaveTxTdmWsInv(BOOL bEnable);
extern int   HalAudI2sSaveTxTdmChSwap(U8 nSwap_0_2, U8 nSwap_0_4);
extern int   HalAudI2sSaveTxTdmActiveSlot(U8 nActiveSlot);
extern int   HalAudI2sGetRxTdmWsPgm(U8 *bEnable);
extern long *HalAudI2sGetTdmWsPgm(void);
extern int   HalAudI2sGetRxTdmWsWidth(U8 *nWsWidth);
extern long *HalAudI2sGetTdmWsWidth(void);
extern int   HalAudI2sGetRxTdmWsInv(U8 *bEnable);
extern long *HalAudI2sGetTdmWsInv(void);
extern long *HalAudI2sGetTdmBckInv(void);
extern long *HalAudI2sGetRxMode(void);
extern int   HalAudI2sGetRxTdmChSwap(U8 *nSwap);
extern long *HalAudI2sGetTdmChSwap(void);
extern int   HalAudI2sGetTxTdmWsPgm(U8 *bEnable);
extern int   HalAudI2sGetTxTdmWsWidth(U8 *nWsWidth);
extern int   HalAudI2sGetTxTdmWsInv(U8 *bEnable);
extern int   HalAudI2sGetTxTdmChSwap(U8 *nSwap);
extern int   HalAudI2sGetTxTdmActiveSlot(U8 *nActiveSlot);
extern int   HalAudI2sSetTdmDetails(CHIP_AIO_I2S_e eAioI2s);
extern int   HalAudI2sSetRate(CHIP_AIO_I2S_e eAioI2s, AudRate_e eRate);
extern int   HalAudI2sSetClkRef(CHIP_AIO_I2S_e eAioI2s, AudI2sClkRef_e eI2sClkRef);
extern int   HalAudI2sSetTdmMode(CHIP_AIO_I2S_e eAioI2s, AudI2sMode_e enI2sMode);
extern int   HalAudI2sSetMsMode(CHIP_AIO_I2S_e eAioI2s, AudI2sMsMode_e eMsMode);
extern int   HalAudI2sSetFmt(CHIP_AIO_I2S_e eAioI2s, AudI2sFmt_e enI2sFmt);
extern int   HalAudI2sSetWidth(CHIP_AIO_I2S_e eAioI2s, AudBitWidth_e enI2sWidth);
extern int   HalAudI2sSetChannel(CHIP_AIO_I2S_e eAioI2s, U16 nChannel);
extern int   HalAudI2sSetWireMode(CHIP_AIO_I2S_e eAioI2s, AudWireMode_e eWireMode);
extern int   HalAudI2sSetTdmSlotConfig(CHIP_AIO_I2S_e eAioI2s, U16 nSlotMsk, AudTdmChnMap_e eMap);
extern int   HalAudI2sEnableMck(CHIP_AIO_I2S_e eAioI2s, BOOL bEn);
extern int   HalAudI2sSetMck(CHIP_AIO_I2S_e eAioI2s, AudI2sMck_e eMck);
extern int   HalAudI2sEnableBck(CHIP_AIO_I2S_e eAioI2s, BOOL bEn);
extern int   HalAudDmaSetRate(CHIP_AIO_DMA_e eAioDma, AudRate_e eRate);
extern int   HalAudDmaGetRate(CHIP_AIO_DMA_e eAioDma, AudRate_e *eRate, U32 *nRate);
extern int   HalAudDmaReset(CHIP_AIO_DMA_e eAioDma);
extern int   HalAudDmaEnable(CHIP_AIO_DMA_e eAioDma, BOOL bEnable);
extern int   HalAudDmaPause(CHIP_AIO_DMA_e eAioDma);
extern int   HalAudDmaResume(CHIP_AIO_DMA_e eAioDma);
extern int   HalAudDmaIntEnable(CHIP_AIO_DMA_e eAioDma, BOOL bDatatrigger, BOOL bDataboundary);
extern int   HalAudDmaGetInt(CHIP_AIO_DMA_e eAioDma, BOOL *bDatatrigger, BOOL *bDataboundary);
extern int   HalAudDmaClearInt(CHIP_AIO_DMA_e eAioDma);
extern U32   HalAudDmaGetLevelCnt(CHIP_AIO_DMA_e eAioDma);
extern U32   HalAudDmaGetRawLevelCnt(CHIP_AIO_DMA_e eAioDma);
extern U32   HalAudDmaTrigLevelCnt(CHIP_AIO_DMA_e eAioDma, U32 nDataSize);
extern int   HalAudDmaSetPhyAddr(CHIP_AIO_DMA_e eAioDma, long nBufAddrOffset, U32 nBufSize);
extern int   HalAudDmaAiSetThreshold(CHIP_AIO_DMA_e eAioDma, U32 nOverrunTh);
extern int   HalAudDmaAoSetThreshold(CHIP_AIO_DMA_e eAioDma, U32 nUnderrunTh);
extern int   HalAudDmaGetFlags(CHIP_AIO_DMA_e eAioDma, BOOL *pbDatatrigger, BOOL *pbDataboundary, BOOL *pbLocalData);
extern int   HalAudAiSetClkRefAndMch(AI_ATTACH_t *aiAttach);
extern int   HalAudAoSetClkRef(AO_ATTACH_t *aoAttach);
extern int   HalAudAoSetClkUnRef(AO_ATTACH_t *aoAttach);
extern int   HalAudDmaMonoEnable(CHIP_AIO_DMA_e eAioDma, BOOL bEn, BOOL bMonoCopyEn);
extern int   HalAudDpgaCtrl(EN_CHIP_DPGA eDpga, BOOL bEnable, BOOL bMute, BOOL bFade);
extern int   HalAudDpgaSetGain(EN_CHIP_DPGA eDpga, S8 gain, S8 ch);
extern int   HalAudDpgaSetGainOffset(EN_CHIP_DPGA eDpga, S8 gainRegValue, S8 ch);
extern int   HalAudDpgaSetGainFading(EN_CHIP_DPGA eDpga, U8 nFading, S8 ch);
extern int   HalAudDigMicEnable(CHIP_AI_DMIC_e eDmic, BOOL bEn);
extern int   HalAudDigMicSetChannel(CHIP_AI_DMIC_e eDmic, U16 nCh);
extern int   HalAudDigMicSetRate(CHIP_AI_DMIC_e eDmic, AudRate_e eRate);
extern int   HalAudDigMicSetGain(CHIP_AI_DMIC_e eDmic, S8 s8Gain, S8 ch);
extern int   HalAudDmaSineGenEnable(SINE_GEN_e enSineGen, BOOL bEn);
extern int   HalAudDmaSineGenSetting(SINE_GEN_e enSineGen, U8 u8Freq, S8 s8Gain);
extern int   HalAudDmaSineGenSetFreq(SINE_GEN_e enSineGen, U8 u8Freq);
extern int   HalAudDmaSineGenSetGain(SINE_GEN_e enSineGen, S8 s8Gain);
extern int   HalAudDmaSineGenGetEnable(SINE_GEN_e enSineGen, BOOL *pbEn);
extern int   HalAudDmaSineGenGetSetting(SINE_GEN_e enSineGen, U8 *pu8Freq, S8 *ps8Gain);
extern int   HalAudHdmiEanble(CHIP_AO_HDMI_e eHdmi, BOOL bEn);
extern int   HalAudAiSetGain(AI_CH_e eAiCh, S16 s16Gain);
extern int   HalAudAoSetGain(AO_CH_e eAoCh, S16 s16Gain);
extern int   HalAudAiSetIfMute(AI_CH_e eAiCh, BOOL bEn);
extern int   HalAudAoSetIfMute(AO_CH_e eAoCh, BOOL bEn);
extern int   HalAudAiSetIfSampleRate(AI_IF_e eAiIf, u32 nSampleRate);
extern int   HalAudAoSetIfSampleRate(AO_IF_e eAoIf, u32 nSampleRate);
extern int   HalAudAiSetDpgaGain(CHIP_AIO_DMA_e eAioDma, U8 nDmaCh, AI_CH_e eAiCh, S16 s16Gain);
extern int   HalAudAoSetDpgaGain(CHIP_AIO_DMA_e eAioDma, U8 nDmaCh, AO_CH_e eAoCh, S16 s16Gain, U8 nFading);
extern int   HalAudAiSetDpgaMute(CHIP_AIO_DMA_e eAioDma, U8 nDmaCh, AI_CH_e eAiCh, BOOL bEn);
extern int   HalAudAoSetDpgaMute(CHIP_AIO_DMA_e eAioDma, U8 nDmaCh, AO_CH_e eAoCh, BOOL bEn, U8 nFading);
extern int   HalAudDirectSetDpgaGain(AI_CH_e eAiCh, AO_CH_e eAoCh, S16 s16Gain);
extern int   HalAudConfigDigMicParam(CHIP_AI_DMIC_e eChipDmic, DigMicParam_t *ptDigMicParam);
extern int   HalAudConfigDigMicSampleRate(CHIP_AI_DMIC_e eChipDmic, u32 nSampleRate);
extern int   HalAudConfigDigMicChNum(CHIP_AI_DMIC_e eChipDmic, u8 nChNum);
extern int   HalAudConfigSrcParam(AI_CH_e eAiCh, SrcParam_t *ptSrcParam);
extern int   HalAudAoIfMultiAttachAction(AO_CH_e eAoCh, U8 nAttachCount);
extern int   HalAudAoDmaSetRefClk(CHIP_AIO_DMA_e eAioDma, BOOL bIsFromI2S);
extern BOOL  HalAudSetHpf(AudHpfDev_e eHfpDev, U8 level);
extern BOOL  HalAudDigMicBckMode(U8 uMode8K, U8 uMode16K, U8 uMode32K, U8 uMode48K);

#endif //__HAL_AUD_API_H__
