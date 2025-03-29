/*
 * drv_audio.h - Sigmastar
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

#ifndef __DRV_AUDIO_H__
#define __DRV_AUDIO_H__

// -------------------------------------------------------------------------------
typedef struct
{
    u8 * pDmaBuf;
    long nPhysDmaAddr;
    u32  nBufferSize;
    u16  nChannels;
    u16  nBitWidth;
    u32  nSampleRate;
    u32  nPeriodSize;
    u16  nInterleaved;
    u8   bIsOnlyEvenCh;
    void *private;

} DmaParam_t;

// -------------------------------------------------------------------------------
extern int  DrvAudInit(void);
extern int  DrvAudDeInit(void);
extern int  DrvAudConfigI2s(CHIP_AIO_I2S_e eAioI2s, AudI2sCfg_t *ptI2sCfg);
extern int  DrvAudEnableI2s(CHIP_AIO_I2S_e eAioI2s, BOOL bEn);
extern int  DrvAudSetAiPath(CHIP_AIO_DMA_e eAioDma, U8 nDmaCh, AI_CH_e eAiCh);
extern int  DrvAudSetAoPath(CHIP_AIO_DMA_e eAioDma, U8 nDmaCh, AO_CH_e eAoCh);
extern int  DrvAudSetDirectPath(AI_CH_e eAiCh, AO_CH_e eAoCh);
extern int  DrvAudDetachAo(AO_CH_e aoch);
extern int  DrvAudSetMux(AudMux_e eMux, u8 nChoice);
extern int  DrvAudMonoEnable(CHIP_AIO_DMA_e eAioDma, BOOL bEn, BOOL bMonoCopyEn);
extern int  DrvAudEnableAtop(CHIP_AIO_ATOP_e eAtop, BOOL bEn);
extern int  DrvAudAiSetClkRefAndMch(AI_ATTACH_t *aiAttach);
extern int  DrvAudAoSetClkRef(AO_ATTACH_t *aoAttach);
extern int  DrvAudAoSetClkUnRef(AO_ATTACH_t *aoAttach);
extern int  DrvAudConfigDmaParam(CHIP_AIO_DMA_e eAioDma, DmaParam_t *ptParam);
extern u32  DrvAudGetDmaSampleRate(CHIP_AIO_DMA_e eAioDma);
extern int  DrvAudOpenDma(CHIP_AIO_DMA_e eAioDma);
extern int  DrvAudPrepareDma(CHIP_AIO_DMA_e eAioDma);
extern int  DrvAudStartDma(CHIP_AIO_DMA_e eAioDma);
extern int  DrvAudStopDma(CHIP_AIO_DMA_e eAioDma);
extern int  DrvAudPauseDma(CHIP_AIO_DMA_e eAioDma);
extern int  DrvAudResumeDma(CHIP_AIO_DMA_e eAioDma);
extern int  DrvAudCloseDma(CHIP_AIO_DMA_e eAioDma);
extern BOOL DrvAudIsXrun(CHIP_AIO_DMA_e eAioDma);
extern s32  DrvAudWritePcm(CHIP_AIO_DMA_e eAioDma, void *pWrBuffer, u32 nSize);
extern s32  DrvAudReadPcm(CHIP_AIO_DMA_e eAioDma, void *pRdBuffer, u32 nSize);
extern int  DrvAudSetAdcGain(CHIP_AI_ADC_e eAdc, U16 nSel);
extern int  DrvAudSetMicAmpGain(CHIP_AI_ADC_e eAdc, U16 nSel);
extern int  DrvAudAdcSetMux(CHIP_AI_ADC_e eAdc, CHIP_ADC_MUX_e eAdcMux);
extern int  DrvAudDpgaSetGain(EN_CHIP_DPGA eDpga, S8 s8Gain, S8 s8Ch);
extern int  DrvAudDpgaSetGainFading(EN_CHIP_DPGA eDpga, U8 nFading, S8 s8Ch);
extern int  DrvAudConfigDigMicParam(CHIP_AI_DMIC_e eChipDmic, DigMicParam_t *ptDigMicParam);
extern int  DrvAudConfigDigMicSampleRate(CHIP_AI_DMIC_e eChipDmic, u32 nSampleRate);
extern int  DrvAudConfigDigMicChNum(CHIP_AI_DMIC_e eChipDmic, u8 nChNum);
extern int  DrvAudDigMicEnable(CHIP_AI_DMIC_e eDmic, BOOL bEn);
extern int  DrvAudSetDigMicGain(CHIP_AI_DMIC_e eDmic, S8 s8Gain, S8 s8Ch);
extern int  DrvAudGetBufCurrDataLen(CHIP_AIO_DMA_e eAioDma, u32 *len);
extern int  DrvAudConfigSrcParam(AI_CH_e eAiCh, SrcParam_t *ptSrcParam);
extern int  DrvAudEnableHdmi(CHIP_AO_HDMI_e eHdmi, BOOL bEn);
extern int  DrvAudAiIfSetGain(AI_CH_e eAiCh, S16 s16Gain);
extern int  DrvAudAoIfSetGain(AO_CH_e eAoCh, S16 s16Gain);
extern int  DrvAudAiIfSetMute(AI_CH_e eAiCh, BOOL bEn);
extern int  DrvAudAoIfSetMute(AO_CH_e eAoCh, BOOL bEn);
extern int  DrvAudAiSetDpgaGain(CHIP_AIO_DMA_e eAioDma, U8 nDmaCh, AI_CH_e eAiCh, S16 s16Gain);
extern int  DrvAudAoSetDpgaGain(CHIP_AIO_DMA_e eAioDma, U8 nDmaCh, AO_CH_e eAoCh, S16 s16Gain, U8 nFading);
extern int  DrvAudAiSetDpgaMute(CHIP_AIO_DMA_e eAioDma, U8 nDmaCh, AI_CH_e eAiCh, BOOL bEn);
extern int  DrvAudAoSetDpgaMute(CHIP_AIO_DMA_e eAioDma, U8 nDmaCh, AO_CH_e eAoCh, BOOL bEn, U8 nFading);
extern int  DrvAudDirectSetDpgaGain(AI_CH_e eAiCh, AO_CH_e eAoCh, S16 s16Gain);
extern int  DrvAudAoIfMultiAttachAction(AO_CH_e eAoCh, U8 nAttachCount);
extern int  DrvAudSineGenEnable(SINE_GEN_e enSineGen, BOOL bEn);
extern int  DrvAudSineGenSetting(SINE_GEN_e enSineGen, U8 u8Freq, S8 s8Gain);
extern int  DrvAudAiSetIfSampleRate(AI_IF_e eAiIf, u32 nSampleRate);
extern int  DrvAudAoSetIfSampleRate(AO_IF_e eAoIf, u32 nSampleRate);

#endif //__DRV_AUDIO_H__
