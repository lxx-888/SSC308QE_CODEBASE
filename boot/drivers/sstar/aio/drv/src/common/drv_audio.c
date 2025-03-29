/*
 * drv_audio.c - Sigmastar
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

#include "cam_os_wrapper.h"

// Hal
#include "hal_audio_common.h"
#include "hal_audio_sys.h"
#include "hal_audio_config.h"
#include "hal_audio_reg.h"
#include "hal_audio_types.h"
#include "hal_audio_api.h"
#include "hal_audio_os_api.h"

// Drv
#include "drv_audio.h"
#include "drv_audio_dbg.h"
#include "drv_audio_api.h"
#include "drv_audio_pri.h"

// -------------------------------------------------------------------------------
#ifndef max
#define max(x, y)                          \
    (                                      \
        {                                  \
            typeof(x) _max1 = (x);         \
            typeof(y) _max2 = (y);         \
            (void)(&_max1 == &_max2);      \
            _max1 > _max2 ? _max1 : _max2; \
        })
#endif

// -------------------------------------------------------------------------------
PcmRuntimeData_t g_aRuntimeData[E_CHIP_AIO_DMA_TOTAL];
DmaParam_t       g_aDmaParam[E_CHIP_AIO_DMA_TOTAL];
static BOOL      g_bI2sMaster[E_CHIP_AIO_I2S_TOTAL];

// -------------------------------------------------------------------------------
/**
 * bytes_to_frames - Unit conversion of the size from bytes to frames
 * @runtime: PCM runtime instance
 * @size: size in bytes
 */
static inline PcmFrames_t _BytesToFrames(PcmRuntimeData_t *pRtd, u32 size)
{
    return size / (pRtd->nFrameBits >> 3);
}

static inline PcmFrames_t _BytesToHwFrames(PcmRuntimeData_t *pRtd, u32 size)
{
    return size / (pRtd->nHwFrameBits >> 3);
}

/**
 * frames_to_bytes - Unit conversion of the size from frames to bytes
 * @runtime: PCM runtime instance
 * @size: size in frames
 */
static inline u32 _FramesToBytes(PcmRuntimeData_t *pRtd, PcmFrames_t size)
{
    return size * (pRtd->nFrameBits >> 3);
}

static inline u32 _HwFramesToBytes(PcmRuntimeData_t *pRtd, PcmFrames_t size)
{
    return size * (pRtd->nHwFrameBits >> 3);
}

/**
 * _AudPcmPlaybackAvail - Get the available (writable) space for playback
 */
static inline PcmFrames_t _AudPcmPlaybackAvail(PcmDmaData_t *pDmaData)
{
    // unsigned long nFlags;
    PcmFrames_t tAvail;
    u32         nCurDmaLevelCount = 0;

    // CamOsSpinLockIrqSave(&pDmaData->tLock);
    nCurDmaLevelCount = HalAudDmaGetLevelCnt(pDmaData->nChnId);

    AUD_PRINTF(LC_LEVEL, " AO --- Curr LC = %u\r\n", nCurDmaLevelCount);

    tAvail = pDmaData->tBufSize
             - _BytesToHwFrames(&g_aRuntimeData[pDmaData->nChnId],
                                nCurDmaLevelCount + g_aRuntimeData[pDmaData->nChnId].nRemainCount);

    // CamOsSpinUnlockIrqRestore(&pDmaData->tLock);

    return tAvail;
}

/**
 * _AudPcmCaptureAvail - Get the available (readable) space for capture
 */
static inline PcmFrames_t _AudPcmCaptureAvail(PcmDmaData_t *pDmaData)
{
    // unsigned long nFlags;
    PcmFrames_t tAvail;
    u32         nCurDmaLevelCount = 0;

    // CamOsSpinLockIrqSave(&pDmaData->tLock);
    nCurDmaLevelCount = HalAudDmaGetLevelCnt(pDmaData->nChnId);

    AUD_PRINTF(LC_LEVEL, " AI - Curr LC = %u\r\n", nCurDmaLevelCount);

    tAvail = _BytesToHwFrames(&g_aRuntimeData[pDmaData->nChnId],
                              nCurDmaLevelCount - g_aRuntimeData[pDmaData->nChnId].nRemainCount);

    //  CamOsSpinUnlockIrqRestore(&pDmaData->tLock);

    // AUD_PRINTF(ERROR_LEVEL,"buf %ld, count %u,remain
    // %u\n",pDmaData->tBufSize,nCurDmaLevelCount,g_aRuntimeData[pDmaData->nChnId].nRemainCount);

    if (g_aRuntimeData[pDmaData->nChnId].nRemainCount > 0)
    {
        AUD_PRINTF(ERROR_LEVEL, "remain %u\n", g_aRuntimeData[pDmaData->nChnId].nRemainCount);
    }

    return tAvail;
}

/**
 * _AudPcmCaptureRawAvail - Get the actual record data length for capture
 */
static inline PcmFrames_t _AudPcmCaptureRawAvail(PcmDmaData_t *pDmaData)
{
    // unsigned long nFlags;
    PcmFrames_t tAvail;
    u32         nCurDmaLevelCount = 0;
    // CamOsSpinLockIrqSave(&pDmaData->tLock);

    nCurDmaLevelCount = HalAudDmaGetRawLevelCnt(pDmaData->nChnId);
    tAvail            = _BytesToHwFrames(&g_aRuntimeData[pDmaData->nChnId],
                                         nCurDmaLevelCount - g_aRuntimeData[pDmaData->nChnId].nRemainCount);

    // CamOsSpinUnlockIrqRestore(&pDmaData->tLock);

    return tAvail;
}

static int _WaitForAvail(PcmRuntimeData_t *ptRuntimeData, PcmFrames_t *ptAvail)
{
    PcmDmaData_t *ptDmaData   = (PcmDmaData_t *)(ptRuntimeData->pPrivateData);
    BOOL          bIsPlayback = (ptRuntimeData->eStream == E_PCM_STREAM_PLAYBACK);
    int           err         = 0;
    long          nWaitTime;
    CamOsRet_e    eRet;
    PcmFrames_t   tAvail;

    if (g_aDmaParam[ptDmaData->nChnId].nSampleRate)
    {
        nWaitTime = ptDmaData->tPeriodSize * 2 * 1000 / g_aDmaParam[ptDmaData->nChnId].nSampleRate;
    }
    else
    {
        nWaitTime = 5000;
    }

    nWaitTime = max((long)5000, nWaitTime); // min 5s wait
    // nWaitTime = msecs_to_jiffies(nWaitTime);

    for (;;)
    {
        /*
         * We need to check if space became available already
         * (and thus the wakeup happened already) first to close
         * the race of space already having become available.
         * This check must happen after been added to the waitqueue
         * and having current state be INTERRUPTIBLE.
         */
        if (bIsPlayback)
        {
            tAvail = _AudPcmPlaybackAvail(ptDmaData);
        }
        else
        {
            tAvail = _AudPcmCaptureAvail(ptDmaData);
        }

#if NO_EXHAUST_DMA_BUF
        if (tAvail >= (ptDmaData->tPeriodSize
                       - _BytesToHwFrames(&g_aRuntimeData[ptDmaData->nChnId], DMA_BUF_REMAINDER))) // any available data
        {
            break;
        }
#else
        if (tAvail >= ptDmaData->tPeriodSize) // any available data
        {
            break;
        }
#endif
        // CamOsSpinUnlockIrqRestore(&ptRuntimeData->tPcmLock);

        // eRet = CamOsTsemTimedDown(&ptRuntimeData->tsleep, nWaitTime);

        // CamOsSpinLockIrqSave(&ptRuntimeData->tPcmLock);

        switch (ptRuntimeData->nStatus)
        {
            case SND_PCM_STATE_XRUN:
                err = -EPIPE;
                goto _endloop;
            case SND_PCM_STATE_OPEN:
            case SND_PCM_STATE_SETUP:
                err = -EBADFD;
                goto _endloop;
            case SND_PCM_STATE_PAUSED:
                continue;
        }

        if (eRet != CAM_OS_OK)
        {
            switch (eRet)
            {
                case CAM_OS_TIMEOUT:
                    AUD_PRINTF(DEBUG_LEVEL, "TIMEOUT\n");
                    break;
                case CAM_OS_PARAM_ERR:
                    AUD_PRINTF(DEBUG_LEVEL, "PARAM_ERR\n");
                    break;
                default: // CAM_OS_FAIL
                    AUD_PRINTF(DEBUG_LEVEL, "Condition not init\n");
                    break;
            }

            err = -EIO;
            break;
        }
    }
_endloop:
    *ptAvail = tAvail;
    return err;
}

// writer HW frames should be the same as user's
static s32 _AudDmaWrCopyI16(struct PcmRuntimeData_s *ptRuntimeData, u32 nHwoff, u8 *pBuf, u32 nBufSize, u32 nPos,
                            PcmFrames_t tFrames)
{
    PcmDmaData_t *ptDmaData = (PcmDmaData_t *)ptRuntimeData->pPrivateData;
    u8 *          pHwbufPtr = ptDmaData->pDmaArea + _HwFramesToBytes(ptRuntimeData, nHwoff);
    u8 *          pBufPtr   = pBuf + _FramesToBytes(ptRuntimeData, nPos);

    memcpy(pBufPtr, pHwbufPtr, _FramesToBytes(ptRuntimeData, tFrames));
    return 0;
}

static s32 _AudDmaWrCopyI32(struct PcmRuntimeData_s *ptRuntimeData, u32 nHwoff, u8 *pBuf, u32 nBufSize, u32 nPos,
                            PcmFrames_t tFrames)
{
    PcmDmaData_t *ptDmaData = (PcmDmaData_t *)ptRuntimeData->pPrivateData;
    u8 *          pHwbufPtr = ptDmaData->pDmaArea + _HwFramesToBytes(ptRuntimeData, nHwoff);
    u8 *          pBufPtr   = pBuf + _FramesToBytes(ptRuntimeData, nPos);

    memcpy(pBufPtr, pHwbufPtr, _FramesToBytes(ptRuntimeData, tFrames));

    return 0;
}

// output noninterleave data, writer HW frames should be the same as user's
static s32 _AudDmaWrCopyN16(struct PcmRuntimeData_s *ptRuntimeData, u32 nHwoff, u8 *pBuf, u32 nBufSize, u32 nPos,
                            PcmFrames_t tFrames)
{
    PcmDmaData_t *ptDmaData = (PcmDmaData_t *)ptRuntimeData->pPrivateData;
    u16           nChannels = g_aDmaParam[ptDmaData->nChnId].nChannels;
    u32           i;
    s16 *         pnInSample, *pnOutSample;
    u32           nInterval = nBufSize / nChannels;

    pnInSample = (s16 *)(ptDmaData->pDmaArea + _HwFramesToBytes(ptRuntimeData, nHwoff));
    while (tFrames > 0)
    {
        for (i = 0; i < nChannels; i++)
        {
            pnOutSample  = (s16 *)(pBuf + nInterval * i) + nPos;
            *pnOutSample = *pnInSample;
            pnInSample++;
        }
        tFrames--;
        nPos++;
    }
    return 0;
}

static s32 _AudDmaWrCopyN32(struct PcmRuntimeData_s *ptRuntimeData, u32 nHwoff, u8 *pBuf, u32 nBufSize, u32 nPos,
                            PcmFrames_t tFrames)
{
    return 0;
}

static s32 _AudDmaRdCopyI16(struct PcmRuntimeData_s *ptRuntimeData, u32 nHwoff, u8 *pBuf, u32 nBufSize, u32 nPos,
                            PcmFrames_t tFrames)
{
    PcmDmaData_t *ptDmaData = (PcmDmaData_t *)ptRuntimeData->pPrivateData;
    u16           nChannels = g_aDmaParam[ptDmaData->nChnId].nChannels;
    s16 *         pnInSample;
    s16 *         pnOutSample;
    u32           i;
    s32           nSampleValue;

    invalidate_dcache_range((unsigned long)ptDmaData->pDmaArea + _HwFramesToBytes(ptRuntimeData, nHwoff),
                            (unsigned long)(ptDmaData->pDmaArea + _HwFramesToBytes(ptRuntimeData, nHwoff)
                                            + _HwFramesToBytes(ptRuntimeData, tFrames)));

    pnOutSample = (s16 *)(ptDmaData->pDmaArea + _HwFramesToBytes(ptRuntimeData, nHwoff)); // DMA的buff，硬件的写位置
    pnInSample = (s16 *)(pBuf + _FramesToBytes(ptRuntimeData, nPos));

    while (tFrames > 0)
    {
        for (i = 0; i < nChannels; i++)
        {
            nSampleValue = (s32)(*pnInSample);
            *pnOutSample = (s16)nSampleValue;
            pnOutSample++;
            pnInSample++;
        }
        tFrames--;
    }
#if 1
    flush_dcache_range((unsigned long)ptDmaData->pDmaArea + _HwFramesToBytes(ptRuntimeData, nHwoff),
                       (unsigned long)(ptDmaData->pDmaArea + _HwFramesToBytes(ptRuntimeData, nHwoff)
                                       + _HwFramesToBytes(ptRuntimeData, tFrames)));
#endif
    // printf("1111111111\n");

    return 0;
}

static s32 _AudDmaRdCopyI32(struct PcmRuntimeData_s *ptRuntimeData, u32 nHwoff, u8 *pBuf, u32 nBufSize, u32 nPos,
                            PcmFrames_t tFrames)
{
    return 0;
}

// input noninterleave data
static s32 _AudDmaRdCopyN16(struct PcmRuntimeData_s *ptRuntimeData, u32 nHwoff, u8 *pBuf, u32 nBufSize, u32 nPos,
                            PcmFrames_t tFrames)
{
    PcmDmaData_t *ptDmaData = (PcmDmaData_t *)ptRuntimeData->pPrivateData;
    u16           nChannels = g_aDmaParam[ptDmaData->nChnId].nChannels;
    s16 *         pnInSample;
    s16 *         pnOutSample;
    u32           nInterval = nBufSize / nChannels;
    u32           i;
    s32           nSampleValue;

    pnOutSample = (s16 *)(ptDmaData->pDmaArea + _HwFramesToBytes(ptRuntimeData, nHwoff));

    while (tFrames > 0)
    {
        for (i = 0; i < nChannels; i++)
        {
            pnInSample   = (s16 *)(pBuf + nInterval * i) + nPos;
            nSampleValue = (s32)(*pnInSample);
            *pnOutSample = (s16)nSampleValue;
            pnOutSample++;
        }
        tFrames--;
        nPos++;
    }

    return 0;
}

static s32 _AudDmaRdCopyN32(struct PcmRuntimeData_s *ptRuntimeData, u32 nHwoff, u8 *pBuf, u32 nBufSize, u32 nPos,
                            PcmFrames_t tFrames)
{
    return 0;
}

// just copy(not handle wrap-around), function call should check enough size,
static s32 _AudDmaKernelCopy(struct PcmRuntimeData_s *ptRuntimeData, u32 nHwoff, u8 *pBuf, u32 nBufSize, u32 nPos,
                             PcmFrames_t tFrames)
{
    PcmDmaData_t *ptDmaData = (PcmDmaData_t *)ptRuntimeData->pPrivateData;
    // unsigned long nFlags;
    u32 nLevelCount = 0;
    u32 nTrigCount;
    u32 nOldState = ptRuntimeData->nState;
    // u16 nRetry = 0;
    if (E_PCM_STREAM_PLAYBACK == ptRuntimeData->eStream)
    {
        AUD_PRINTF(DEBUG_LEVEL, "%s pos=%u, frames %lu\n", __FUNCTION__, nPos, tFrames);
        ptRuntimeData->copy(ptRuntimeData, nHwoff, pBuf, nBufSize, nPos, tFrames);

        // CamOsSpinLockIrqSave(&ptDmaData->tLock);
        if (nOldState != DMA_EMPTY && ptRuntimeData->nState == DMA_EMPTY)
        {
            // CamOsSpinUnlockIrqRestore(&ptDmaData->tLock);
            AUD_PRINTF(DEBUG_LEVEL, "%s state changing!!!\n", __FUNCTION__);
            return 0;
        }

        // CamOsMiuPipeFlush();

        nLevelCount = HalAudDmaGetLevelCnt(ptDmaData->nChnId);
        nTrigCount  = HalAudDmaTrigLevelCnt(ptDmaData->nChnId,
                                            (_HwFramesToBytes(ptRuntimeData, tFrames) + ptRuntimeData->nRemainCount));

        if (ptRuntimeData->nState == DMA_EMPTY || ptRuntimeData->nState == DMA_INIT)
        {
            // HalAudDmaClearInt(ptDmaData->nChnId);
            ptRuntimeData->nState = DMA_NORMAL;
        }
#if 0
        /* Be careful!! check level count updated*/
        while(nRetry<20)
        {
            if(HalAudDmaGetLevelCnt(ptDmaData->nChnId)>nLevelCount)
            {
                printk("Re = %u\r\n",nRetry);
                break;
            }
            nRetry++;
        }

        if(nRetry==20)
        {
            printk("nTrigCount = %u !\r\n",nTrigCount);
            //AUD_PRINTF(ERROR_LEVEL,"update level count too slow!!!!!!!!\n");
        }
#endif
        if (((nLevelCount + nTrigCount)
             >= (ptDmaData->nBufBytes - _HwFramesToBytes(ptRuntimeData, ptDmaData->tPeriodSize)))
            && ptRuntimeData->nStatus == SND_PCM_STATE_RUNNING)
        {
            udelay(1); // wait level count ready
            ptRuntimeData->nState = DMA_NORMAL;
            HalAudDmaIntEnable(ptDmaData->nChnId, TRUE, FALSE);
        }

        if ((nLevelCount + nTrigCount) > ptDmaData->nBufBytes)
        {
            AUD_PRINTF(ERROR_LEVEL, "l:%u, t:%u, size %u!!!\n", nLevelCount, nTrigCount,
                       _HwFramesToBytes(ptRuntimeData, tFrames));
        }

        ptRuntimeData->nRemainCount =
            (_HwFramesToBytes(ptRuntimeData, tFrames) + ptRuntimeData->nRemainCount) - nTrigCount;

        // CamOsSpinUnlockIrqRestore(&ptDmaData->tLock);
    }
    else if (E_PCM_STREAM_CAPTURE == ptRuntimeData->eStream)
    {
        AUD_PRINTF(DEBUG_LEVEL, "%s pos=%u, frames %lu\n", __FUNCTION__, nPos, tFrames);
        ptRuntimeData->copy(ptRuntimeData, nHwoff, pBuf, nBufSize, nPos, tFrames);

        // CamOsSpinLockIrqSave(&ptDmaData->tLock);
        if (nOldState != DMA_FULL && ptRuntimeData->nState == DMA_FULL)
        { //
          //  CamOsSpinUnlockIrqRestore(&ptDmaData->tLock);
            AUD_PRINTF(DEBUG_LEVEL, "%s state changing!!!\n", __FUNCTION__);
            return 0;
        }

        nLevelCount = HalAudDmaGetLevelCnt(ptDmaData->nChnId);
        nTrigCount  = HalAudDmaTrigLevelCnt(ptDmaData->nChnId,
                                            (_HwFramesToBytes(ptRuntimeData, tFrames) + ptRuntimeData->nRemainCount));
        if (ptRuntimeData->nState == DMA_FULL || ptRuntimeData->nState == DMA_INIT)
        {
            HalAudDmaClearInt(ptDmaData->nChnId);
            ptRuntimeData->nState = DMA_NORMAL;
        }

#if 0
        while(nRetry<20)
        {
            if(HalAudDmaGetLevelCnt(ptDmaData->nChnId)<nLevelCount)
                break;
            nRetry++;
        }

         if(nRetry==20)
            AUD_PRINTF(ERROR_LEVEL,"update level count too slow!!!!!!!!\n");
#endif

        if (((nLevelCount - nTrigCount) < _HwFramesToBytes(ptRuntimeData, ptDmaData->tPeriodSize))
            && g_aRuntimeData[ptDmaData->nChnId].nStatus == SND_PCM_STATE_RUNNING)
        {
            udelay(1); // wait level count ready
            ptRuntimeData->nState = DMA_NORMAL;
            HalAudDmaIntEnable(ptDmaData->nChnId, TRUE, FALSE);
        }

        if (nLevelCount < nTrigCount)
        {
            AUD_PRINTF(ERROR_LEVEL, "l:%u, t:%u, size %u!!!\n", nLevelCount, nTrigCount,
                       _HwFramesToBytes(ptRuntimeData, tFrames));
        }

        ptRuntimeData->nRemainCount =
            (_HwFramesToBytes(ptRuntimeData, tFrames) + ptRuntimeData->nRemainCount) - nTrigCount;
        if (ptRuntimeData->nRemainCount)
        {
            AUD_PRINTF(DEBUG_LEVEL, "%s remain count = %ul\n", __FUNCTION__, ptRuntimeData->nRemainCount);
        }

        // CamOsSpinUnlockIrqRestore(&ptDmaData->tLock);
    }
    // AUD_PRINTF(DEBUG_LEVEL,"%s chn=%d, frames size %d bytes, Hw pos %d , level count
    // %u\n",__FUNCTION__,ptDmaData->nChnId,_HwFramesToBytes(ptRuntimeData,tFrames),_HwFramesToBytes(ptRuntimeData,nHwoff),ptRuntimeData->nDmaLevelCount);
    AUD_PRINTF(DEBUG_LEVEL, "%s chn=%d, frames size %d bytes, Hw pos %d \n", __FUNCTION__, ptDmaData->nChnId,
               _HwFramesToBytes(ptRuntimeData, tFrames), _HwFramesToBytes(ptRuntimeData, nHwoff));
    return 0;
}

// -------------------------------------------------------------------------------
static int _DrvAudDmaSetRate(CHIP_AIO_DMA_e eAioDma, AudRate_e eRate);
static int _DrvAudDmaStartChannel(CHIP_AIO_DMA_e eAioDma);
static int _DrvAudDmaStopChannel(CHIP_AIO_DMA_e eAioDma);

// -------------------------------------------------------------------------------
static int _DrvAudDmaSetRate(CHIP_AIO_DMA_e eAioDma, AudRate_e eRate)
{
    int ret = AIO_OK;

    AUD_PRINTF(TRACE_LEVEL, "%s chn = %d, rate %d\n", __FUNCTION__, eAioDma, eRate);

    if (!CHIP_AIO_DMA_IDX_VALID(eAioDma))
    {
        AUD_PRINTF(ERROR_LEVEL, "%s eAioDma = %d \n", __FUNCTION__, eAioDma);
        goto FAIL;
    }

    ret |= HalAudDmaSetRate(eAioDma, eRate);
    if (ret != AIO_OK)
    {
        AUD_PRINTF(ERROR_LEVEL, "%s HalAudDmaSetRate eAioDma[%d] eRate[%d]\n", __FUNCTION__, eAioDma, eRate);
        goto FAIL;
    }

    return AIO_OK;

FAIL:

    return AIO_NG;
}

static int _DrvAudDmaStartChannel(CHIP_AIO_DMA_e eAioDma)
{
    int ret = AIO_OK;

    AUD_PRINTF(TRACE_LEVEL, "%s eAioDma = %d, level %u\n", __FUNCTION__, eAioDma, HalAudDmaGetLevelCnt(eAioDma));

    // Writer for device (CPU read from device).
    // Flush L3 before dma start.
    if (eAioDma < E_CHIP_AIO_DMA_AO_A)
    {
        // CamOsMiuPipeFlush();
    }

    //
    ret |= HalAudDmaClearInt(eAioDma);
    if (ret != AIO_OK)
    {
        AUD_PRINTF(ERROR_LEVEL, "%s HalAudDmaClearInt eAioDma[%d]\n", __FUNCTION__, eAioDma);
        goto FAIL;
    }

    ret |= HalAudDmaEnable(eAioDma, TRUE);
    if (ret != AIO_OK)
    {
        AUD_PRINTF(ERROR_LEVEL, "%s HalAudDmaEnable eAioDma[%d]\n", __FUNCTION__, eAioDma);
        goto FAIL;
    }

    return AIO_OK;

FAIL:

    return AIO_NG;
}

static int _DrvAudDmaStopChannel(CHIP_AIO_DMA_e eAioDma)
{
    int ret = AIO_OK;

    AUD_PRINTF(TRACE_LEVEL, "%s chn = %d\n", __FUNCTION__, eAioDma);

    //
    ret |= HalAudDmaEnable(eAioDma, FALSE);
    if (ret != AIO_OK)
    {
        goto FAIL;
    }

#if 0
    ret |= HalAudDmaReset(
        eAioDma); // Here & DrvAudPrepareDma() both need DmaReset, here is for clear level count when empty occur.
    if (ret != AIO_OK)
    {
        goto FAIL;
    }
#endif
    return AIO_OK;

FAIL:

    return AIO_NG;
}

// -------------------------------------------------------------------------------
// todo later all
int DrvAudInit(void)
{
    int ret = AIO_OK;
    u16 i;

    // Hal
    // HalBachSysSetBankBaseAddr(DrvAudApiIoAddress(0x1F000000));
    HalBachSysSetBankBaseAddr(0x1F000000);
    HalAudMainInit();

    // Software init
    for (i = 0; i < E_CHIP_AIO_DMA_TOTAL; i++)
    {
        memset(&g_aRuntimeData[i], 0, sizeof(PcmRuntimeData_t));
        g_aRuntimeData[i].nStatus = SND_PCM_STATE_INIT;
        //  CamOsSpinInit(&g_aRuntimeData[i].tPcmLock);
        // CamOsTsemInit(&g_aRuntimeData[i].tsleep, 0);
        if (i < E_CHIP_AIO_DMA_AO_A)
        {
            g_aRuntimeData[i].eStream = E_PCM_STREAM_CAPTURE;
        }
        else
        {
            g_aRuntimeData[i].eStream = E_PCM_STREAM_PLAYBACK;
        }
#if 0
        ret |= HalAudDmaIntEnable(i, FALSE, FALSE);
        if (ret != AIO_OK)
        {
            goto FAIL;
        }
#endif
    }

    return AIO_OK;

FAIL:
    return AIO_NG;
}

int DrvAudDeInit(void)
{
    return HalAudMainDeInit();
}

// To-Do : MISC I2s slave mode with MCLK
int DrvAudConfigI2s(CHIP_AIO_I2S_e eAioI2s, AudI2sCfg_t *ptI2sCfg)
{
    int ret = AIO_OK;

    //
    ret |= HalAudI2sSetWireMode(eAioI2s, ptI2sCfg->eWireMode);
    ret |= HalAudI2sSetFmt(eAioI2s, ptI2sCfg->eFormat);
    ret |= HalAudI2sSetTdmMode(eAioI2s, ptI2sCfg->enI2sMode);
    ret |= HalAudI2sSetMsMode(eAioI2s, ptI2sCfg->eMsMode);
    ret |= HalAudI2sSetWidth(eAioI2s, ptI2sCfg->enI2sWidth);
    ret |= HalAudI2sSetChannel(eAioI2s, ptI2sCfg->nChannelNum);
    ret |= HalAudI2sSetMck(eAioI2s, ptI2sCfg->eMck);
    ret |= HalAudI2sSetTdmDetails(eAioI2s);

    // refer from M6 old framework default set to E_AUD_I2S_CLK_REF_I2S_TDM_RX is necessary to config frome dts
    ret |= HalAudI2sSetClkRef(eAioI2s, E_AUD_I2S_CLK_REF_I2S_TDM_RX);

    // Set I2S Rate, must be after HalAudI2sSetWidth & HalAudI2sSetChannel, if you need to set bck, you need width and
    // channel info.
    if (ptI2sCfg->eMsMode == E_AUD_I2S_MSMODE_MASTER)
    {
        ret |= HalAudI2sSetRate(eAioI2s, ptI2sCfg->eRate);

        g_bI2sMaster[eAioI2s] = TRUE;
    }
    else
    {
        g_bI2sMaster[eAioI2s] = FALSE;
    }

    if (ret != AIO_OK)
    {
        goto FAIL;
    }

    return AIO_OK;

FAIL:

    return AIO_NG;
}

int DrvAudEnableI2s(CHIP_AIO_I2S_e eAioI2s, BOOL bEn)
{
    int ret = AIO_OK;

    AUD_PRINTF(TRACE_LEVEL, "%s eAioI2s = %d, bEn = %d\n", __FUNCTION__, eAioI2s, bEn);

    //
    if ((unsigned int)eAioI2s >= E_CHIP_AIO_I2S_TOTAL)
    {
        AUD_PRINTF(ERROR_LEVEL, "%s[%d] eAioI2s = %d\n", __FUNCTION__, __LINE__, eAioI2s);
        goto FAIL;
    }

    ret |= HalAudI2sEnableMck(eAioI2s, bEn);
    if (ret != AIO_OK)
    {
        goto FAIL;
    }

    if (g_bI2sMaster[eAioI2s] == TRUE)
    {
        ret |= HalAudI2sEnableBck(eAioI2s, bEn);
    }

    if (ret != AIO_OK)
    {
        goto FAIL;
    }

    return AIO_OK;

FAIL:

    return AIO_NG;
}

int DrvAudSetAiPath(CHIP_AIO_DMA_e eAioDma, U8 nDmaCh, AI_CH_e eAiCh)
{
    return HalAudSetAiPath(eAioDma, nDmaCh, eAiCh);
}

int DrvAudSetAoPath(CHIP_AIO_DMA_e eAioDma, U8 nDmaCh, AO_CH_e eAoCh)
{
    return HalAudSetAoPath(eAioDma, nDmaCh, eAoCh);
}

int DrvAudDetachAo(AO_CH_e aoch)
{
    return HalAudAoDetach(aoch);
}

int DrvAudSetDirectPath(AI_CH_e eAiCh, AO_CH_e eAoCh)
{
    return HalAudSetDirectPath(eAiCh, eAoCh);
}

// mux have to be configured before dma rate
int DrvAudSetMux(AudMux_e eMux, u8 nChoice)
{
    return HalAudSetMux(eMux, nChoice);
}

int DrvAudMonoEnable(CHIP_AIO_DMA_e eAioDma, BOOL bEn, BOOL bMonoCopyEn)
{
    return HalAudDmaMonoEnable(eAioDma, bEn, bMonoCopyEn);
}

int DrvAudEnableAtop(CHIP_AIO_ATOP_e eAtop, BOOL bEn)
{
    int ret = AIO_OK;

    //
    if (!CHIP_AIO_ATOP_IDX_VALID(eAtop))
    {
        AUD_PRINTF(ERROR_LEVEL, "%s[%d] eAtop = %d\n", __FUNCTION__, __LINE__, eAtop);
        goto FAIL;
    }

    //
    if (bEn)
    {
        ret |= HalAudAtopOpen(eAtop);
        if (ret != AIO_OK)
        {
            goto FAIL;
        }
    }
    else
    {
        ret |= HalAudAtopClose(eAtop);
        if (ret != AIO_OK)
        {
            goto FAIL;
        }
    }

    return AIO_OK;

FAIL:

    return AIO_NG;
}

int DrvAudAiSetClkRefAndMch(AI_ATTACH_t *aiAttach)
{
    return HalAudAiSetClkRefAndMch(aiAttach);
}

int DrvAudAoSetClkRef(AO_ATTACH_t *aoAttach)
{
    return HalAudAoSetClkRef(aoAttach);
}

int DrvAudAoSetClkUnRef(AO_ATTACH_t *aoAttach)
{
    return HalAudAoSetClkUnRef(aoAttach);
}

int DrvAudConfigDmaParam(CHIP_AIO_DMA_e eAioDma, DmaParam_t *ptParam)
{
    int ret  = AIO_OK;
    int line = 0;

    // const struct audio_info_mch *info_mch = (const struct audio_info_mch *)ptParam->private;
    u32 nPeriodSize = 0, nBufferSize = 0;
    // int nNumMchCh = 0;
    // int i = 0;
    // const int *mch_sel = NULL;
    AUD_PRINTF(TRACE_LEVEL, "%s chn = %d,status %d\n", __FUNCTION__, eAioDma, g_aRuntimeData[eAioDma].nStatus);
    if (g_aRuntimeData[eAioDma].nStatus != SND_PCM_STATE_INIT && g_aRuntimeData[eAioDma].nStatus != SND_PCM_STATE_SETUP)
    {
        AUD_PRINTF(ERROR_LEVEL, "%s error status %s\n", __FUNCTION__, g_aRuntimeData[eAioDma].nStatus);
        line = __LINE__;
        goto FAIL;
    }

    if (ptParam->nPeriodSize >= ptParam->nBufferSize)
    {
        line = __LINE__;
        goto FAIL;
    }

    nBufferSize = ptParam->nBufferSize;

    if (eAioDma >= E_CHIP_AIO_DMA_AO_A) // Reader
    {
        nPeriodSize = (ptParam->nPeriodSize);
    }
    else // Writer "overrun threshold" should add dma local buffer size !
    {
        nPeriodSize = ((ptParam->nPeriodSize) ? (ptParam->nPeriodSize + DMA_LOCALBUF_SIZE) : (ptParam->nPeriodSize));
    }

    //
    ret |= HalAudDmaSetPhyAddr(eAioDma, ptParam->nPhysDmaAddr, ptParam->nBufferSize);
    if (ret != AIO_OK)
    {
        line = __LINE__;
        goto FAIL;
    }

    //
    if (eAioDma >= E_CHIP_AIO_DMA_AO_A) // Reader
    {
        ret |= HalAudDmaAoSetThreshold(eAioDma, (nBufferSize - nPeriodSize));
    }
    else
    {
        ret |= HalAudDmaAiSetThreshold(eAioDma, nPeriodSize);
    }

    if (ret != AIO_OK)
    {
        line = __LINE__;
        goto FAIL;
    }

    //
    g_aRuntimeData[eAioDma].bEnableIrq = TRUE;

    memcpy(&g_aDmaParam[eAioDma], ptParam, sizeof(DmaParam_t));

    g_aRuntimeData[eAioDma].nFrameBits   = ptParam->nChannels * ptParam->nBitWidth;
    g_aRuntimeData[eAioDma].nHwFrameBits = g_aRuntimeData[eAioDma].nFrameBits;
    g_aRuntimeData[eAioDma].nStatus      = SND_PCM_STATE_SETUP;

    return AIO_OK;

FAIL:
    AUD_PRINTF(ERROR_LEVEL, "%s[%d] eAioDma %d error\n", __FUNCTION__, line, eAioDma);
    return AIO_NG;
}

u32 DrvAudGetDmaSampleRate(CHIP_AIO_DMA_e eAioDma)
{
    return g_aDmaParam[eAioDma].nSampleRate;
}

int DrvAudOpenDma(CHIP_AIO_DMA_e eAioDma)
{
    int           ret = AIO_OK;
    PcmDmaData_t *ptPcmData;
    u8            szName[20];
    AudRate_e     eSampleRate;

    AUD_PRINTF(TRACE_LEVEL, "%s\n", __FUNCTION__);

    if (g_aRuntimeData[eAioDma].nStatus != SND_PCM_STATE_SETUP)
    {
        AUD_PRINTF(ERROR_LEVEL, "%s[%d] status wrong\n", __FUNCTION__, __LINE__);
        goto FAIL;
    }

    ptPcmData = calloc(1, sizeof(PcmDmaData_t));
    if (ptPcmData == NULL)
    {
        AUD_PRINTF(ERROR_LEVEL, "%s[%d] insufficient resource\n", __FUNCTION__, __LINE__);
        goto FAIL;
    }

    snprintf((char *)szName, sizeof(szName), "aio_dma%d", (s16)eAioDma);
    memcpy(ptPcmData->szName, szName, strlen((char *)szName));
    ptPcmData->nChnId      = eAioDma;
    ptPcmData->pDmaArea    = g_aDmaParam[eAioDma].pDmaBuf;
    ptPcmData->tDmaAddr    = g_aDmaParam[eAioDma].nPhysDmaAddr;
    ptPcmData->nBufBytes   = g_aDmaParam[eAioDma].nBufferSize;
    ptPcmData->tBufSize    = _BytesToHwFrames(&g_aRuntimeData[eAioDma], g_aDmaParam[eAioDma].nBufferSize);
    ptPcmData->tPeriodSize = _BytesToHwFrames(&g_aRuntimeData[eAioDma], g_aDmaParam[eAioDma].nPeriodSize);
    ptPcmData->tApplPtr    = 0;

    memset(ptPcmData->pDmaArea, 0, ptPcmData->nBufBytes);
    // CamOsSpinInit(&ptPcmData->tLock);
    g_aRuntimeData[eAioDma].pPrivateData = ptPcmData;

    if (g_aRuntimeData[eAioDma].bEnableIrq)
    {
        // ret |= DrvAudApiIrqRequest((const char *)ptPcmData->szName, (void *)&g_aRuntimeData[eAioDma]);
        // if (ret != AIO_OK)
        // {
        //   goto FAIL;
        //}
    }

    switch (g_aDmaParam[eAioDma].nSampleRate)
    {
        case 8000:
            eSampleRate = E_AUD_RATE_8K;
            break;
        case 11000:
            eSampleRate = E_AUD_RATE_11K;
            break;
        case 12000:
            eSampleRate = E_AUD_RATE_12K;
            break;
        case 16000:
            eSampleRate = E_AUD_RATE_16K;
            break;
        case 22000:
            eSampleRate = E_AUD_RATE_22K;
            break;
        case 24000:
            eSampleRate = E_AUD_RATE_24K;
            break;
        case 32000:
            eSampleRate = E_AUD_RATE_32K;
            break;
        case 44000:
            eSampleRate = E_AUD_RATE_44K;
            break;
        case 48000:
            eSampleRate = E_AUD_RATE_48K;
            break;
        default:
            AUD_PRINTF(ERROR_LEVEL, "%s[%d] eSampleRate unknown !\n", __FUNCTION__, __LINE__);
            goto FAIL;
            break;
    }

    ret |= _DrvAudDmaSetRate(eAioDma, eSampleRate);
    if (ret != AIO_OK)
    {
        goto FAIL;
    }

    switch (g_aDmaParam[eAioDma].nBitWidth)
    {
        case 16:
            if (g_aDmaParam[eAioDma].nInterleaved)
            {
                if (g_aRuntimeData[eAioDma].eStream == E_PCM_STREAM_PLAYBACK)
                {
                    g_aRuntimeData[eAioDma].copy = _AudDmaRdCopyI16;
                }
                else
                {
                    g_aRuntimeData[eAioDma].copy = _AudDmaWrCopyI16;
                }
            }
            else
            {
                if (g_aRuntimeData[eAioDma].eStream == E_PCM_STREAM_PLAYBACK)
                {
                    g_aRuntimeData[eAioDma].copy = _AudDmaRdCopyN16;
                }
                else
                {
                    g_aRuntimeData[eAioDma].copy = _AudDmaWrCopyN16;
                }
            }
            break;
        case 32:
        default:
            if (g_aDmaParam[eAioDma].nInterleaved)
            {
                if (g_aRuntimeData[eAioDma].eStream == E_PCM_STREAM_PLAYBACK)
                {
                    g_aRuntimeData[eAioDma].copy = _AudDmaRdCopyI32;
                }
                else
                {
                    g_aRuntimeData[eAioDma].copy = _AudDmaWrCopyI32;
                }
            }
            else
            {
                if (g_aRuntimeData[eAioDma].eStream == E_PCM_STREAM_PLAYBACK)
                {
                    g_aRuntimeData[eAioDma].copy = _AudDmaRdCopyN32;
                }
                else
                {
                    g_aRuntimeData[eAioDma].copy = _AudDmaWrCopyN32;
                }
            }
            break;
    }

    g_aRuntimeData[eAioDma].nStatus = SND_PCM_STATE_OPEN;

    return AIO_OK;

FAIL:

    return AIO_NG;
}

int DrvAudPrepareDma(CHIP_AIO_DMA_e eAioDma)
{
    int           ret       = AIO_OK;
    PcmDmaData_t *ptPcmData = (PcmDmaData_t *)g_aRuntimeData[eAioDma].pPrivateData;

    AUD_PRINTF(TRACE_LEVEL, "in %s\n", __FUNCTION__);

    if (g_aRuntimeData[eAioDma].nStatus != SND_PCM_STATE_OPEN && g_aRuntimeData[eAioDma].nStatus != SND_PCM_STATE_XRUN)
    {
        AUD_PRINTF(ERROR_LEVEL, "%s[%d] status error!\n", __FUNCTION__, __LINE__);
        goto FAIL;
    }

    ret |= HalAudDmaReset(eAioDma);
    if (ret != AIO_OK)
    {
        goto FAIL;
    }

    ptPcmData->tApplPtr = 0;
    memset(ptPcmData->pDmaArea, 0, ptPcmData->nBufBytes);

    g_aRuntimeData[eAioDma].nRemainCount = 0;
    g_aRuntimeData[eAioDma].nState       = DMA_INIT;
    g_aRuntimeData[eAioDma].nStatus      = SND_PCM_STATE_PREPARED;

    return AIO_OK;

FAIL:

    return AIO_NG;
}

int DrvAudStartDma(CHIP_AIO_DMA_e eAioDma)
{
    int ret = AIO_OK;

    AUD_PRINTF(TRACE_LEVEL, "in %s\n", __FUNCTION__);

    if (eAioDma == E_CHIP_AIO_DMA_AI_DIRECT_A || eAioDma == E_CHIP_AIO_DMA_AO_DIRECT_A)
    {
        g_aRuntimeData[eAioDma].nStatus = SND_PCM_STATE_RUNNING;
        return AIO_OK;
    }

    if (g_aRuntimeData[eAioDma].nStatus != SND_PCM_STATE_PREPARED)
    {
        AUD_PRINTF(ERROR_LEVEL, "%s[%d] error status %d\n", __FUNCTION__, __LINE__, g_aRuntimeData[eAioDma].nStatus);
        goto FAIL;
    }

    ret |= _DrvAudDmaStartChannel(eAioDma);
    if (ret != AIO_OK)
    {
        goto FAIL;
    }

    g_aRuntimeData[eAioDma].nStatus = SND_PCM_STATE_RUNNING;

    if (g_aRuntimeData[eAioDma].bEnableIrq)
    {
        ret |= HalAudDmaIntEnable(eAioDma, TRUE, FALSE);
        if (ret != AIO_OK)
        {
            goto FAIL;
        }
    }

    return AIO_OK;

FAIL:

    return AIO_NG;
}

int DrvAudStopDma(CHIP_AIO_DMA_e eAioDma)
{
    int ret = AIO_OK;

    if (eAioDma == E_CHIP_AIO_DMA_AI_DIRECT_A || eAioDma == E_CHIP_AIO_DMA_AO_DIRECT_A)
    {
        g_aRuntimeData[eAioDma].nStatus = SND_PCM_STATE_OPEN;
        return AIO_OK;
    }

    if (g_aRuntimeData[eAioDma].bEnableIrq)
    {
        ret |= HalAudDmaIntEnable(eAioDma, FALSE, FALSE);
        if (ret != AIO_OK)
        {
            goto FAIL;
        }
    }

    ret |= _DrvAudDmaStopChannel(eAioDma);
    if (ret != AIO_OK)
    {
        goto FAIL;
    }

    g_aRuntimeData[eAioDma].nStatus = SND_PCM_STATE_OPEN;

    return AIO_OK;

FAIL:

    return AIO_NG;
}

int DrvAudPauseDma(CHIP_AIO_DMA_e eAioDma)
{
    int ret = AIO_OK;

    if (g_aRuntimeData[eAioDma].nStatus != SND_PCM_STATE_RUNNING)
    {
        goto FAIL;
    }

    ret |= HalAudDmaPause(eAioDma);
    if (ret != AIO_OK)
    {
        goto FAIL;
    }

    g_aRuntimeData[eAioDma].nStatus = SND_PCM_STATE_PAUSED;

    return AIO_OK;

FAIL:

    return AIO_NG;
}

int DrvAudResumeDma(CHIP_AIO_DMA_e eAioDma)
{
    int ret = AIO_OK;

    if (g_aRuntimeData[eAioDma].nStatus != SND_PCM_STATE_PAUSED)
    {
        goto FAIL;
    }

    ret |= HalAudDmaResume(eAioDma);
    if (ret != AIO_OK)
    {
        goto FAIL;
    }

    g_aRuntimeData[eAioDma].nStatus = SND_PCM_STATE_RUNNING;

    return AIO_OK;

FAIL:

    return AIO_NG;
}

int DrvAudCloseDma(CHIP_AIO_DMA_e eAioDma)
{
    AUD_PRINTF(TRACE_LEVEL, "in %s\n", __FUNCTION__);
    if (g_aRuntimeData[eAioDma].bEnableIrq)
    {
        // DrvAudApiIrqFree((void *)&g_aRuntimeData[eAioDma]);
    }

    if (g_aRuntimeData[eAioDma].pPrivateData)
    {
        free(g_aRuntimeData[eAioDma].pPrivateData);
        g_aRuntimeData[eAioDma].pPrivateData = NULL;
    }
    g_aRuntimeData[eAioDma].nStatus = SND_PCM_STATE_INIT;

    return AIO_OK;
}

BOOL DrvAudIsXrun(CHIP_AIO_DMA_e eAioDma)
{
    return (g_aRuntimeData[eAioDma].nStatus == SND_PCM_STATE_XRUN);
}

s32 DrvAudWritePcm(CHIP_AIO_DMA_e eAioDma, void *pWrBuffer, u32 nSize)
{
    PcmDmaData_t *ptDmaData = (PcmDmaData_t *)(g_aRuntimeData[eAioDma].pPrivateData);
    s32           nErr      = 0;
    PcmFrames_t   tAvail;
    PcmFrames_t   tFreeSize;
    PcmFrames_t   tWorkSize;
    PcmFrames_t   tOffset = 0;
    PcmFrames_t   tApplptr;
    PcmFrames_t   tBufSize;
    AUD_PRINTF(DEBUG_LEVEL, "%s chn=%d\n", __FUNCTION__, eAioDma);
    if (nSize == 0)
    {
        goto null_data;
    }
    else
    {
        tBufSize = _BytesToFrames(&g_aRuntimeData[eAioDma], nSize); //真实大小除以4
    }

    // CamOsSpinLockIrqSave(&g_aRuntimeData[eAioDma].tPcmLock);
    switch (g_aRuntimeData[eAioDma].nStatus)
    {
        case SND_PCM_STATE_PREPARED:
        case SND_PCM_STATE_RUNNING:
        case SND_PCM_STATE_PAUSED:
            break;
        case SND_PCM_STATE_XRUN:
            nErr = -EPIPE;
            goto _end_unlock;
        default:
            nErr = -EBADFD;
            goto _end_unlock;
    }

    tAvail = _AudPcmPlaybackAvail(ptDmaData);
    AUD_PRINTF(DEBUG_LEVEL, "%s tAvail %lu,tBufSize %lu\n", __FUNCTION__, tAvail, tBufSize);
    while (tBufSize > 0)
    {
        if (tAvail == 0)
        {
#if 0
            if (!bBlock)
            {
                nErr = -EAGAIN;
                goto _end_unlock;
            }
#endif
            AUD_PRINTF(DEBUG_LEVEL, "_WaitForAvail\n");
            nErr = _WaitForAvail(&g_aRuntimeData[eAioDma], &tAvail);
            if (nErr < 0)
            {
                goto _end_unlock;
            }
            AUD_PRINTF(DEBUG_LEVEL, "_WaitForAvail avail %lu\n", tAvail);
        }

        tWorkSize = (tBufSize > tAvail ? tAvail : tBufSize); //要写入的大小
        tFreeSize = ptDmaData->tBufSize - (ptDmaData->tApplPtr % ptDmaData->tBufSize);

        if (tWorkSize > tFreeSize) //不回环的长度
        {
            tWorkSize = tFreeSize;
        }

        // AUD_PRINTF(ERROR_LEVEL,"tOffset %ld, tWorkSize %ld, tAvail %ld\n",tOffset,tWorkSize,tAvail);

        if (tWorkSize == 0)
        {
            nErr = -EINVAL;
            goto _end_unlock;
        }

        tApplptr = ptDmaData->tApplPtr % ptDmaData->tBufSize; //现在硬件的实际位置

        // CamOsSpinUnlockIrqRestore(&g_aRuntimeData[eAioDma].tPcmLock);
        // if(tOffset)
        //     AUD_PRINTF(ERROR_LEVEL,"tOffset %ld!!!!!\n",tOffset);
        nErr = _AudDmaKernelCopy(&g_aRuntimeData[eAioDma], tApplptr, pWrBuffer, nSize, tOffset,
                                 tWorkSize); //当前硬件地址，输入buff,输入buff的真实大小，0，实际要往硬件写的地址长度
        // CamOsSpinLockIrqSave(&g_aRuntimeData[eAioDma].tPcmLock);

        if (nErr < 0)
        {
            goto _end_unlock;
        }

        switch (g_aRuntimeData[eAioDma].nStatus)
        {
            case SND_PCM_STATE_XRUN:
                nErr = -EPIPE;
                goto _end_unlock;

            default:
                break;
        }
        tApplptr += tWorkSize;
        if (tApplptr >= ptDmaData->tBufSize)
        {
            tApplptr -= ptDmaData->tBufSize;
        }
        ptDmaData->tApplPtr = tApplptr;

        tOffset += tWorkSize;
        tBufSize -= tWorkSize;
        tAvail -= tWorkSize;
    }

_end_unlock:

    // CamOsSpinUnlockIrqRestore(&g_aRuntimeData[eAioDma].tPcmLock);
    return nErr < 0 ? nErr : _FramesToBytes(&g_aRuntimeData[eAioDma], tOffset); // nSize;

null_data:
    return 0;
}

s32 DrvAudReadPcm(CHIP_AIO_DMA_e eAioDma, void *pRdBuffer, u32 nSize)
{
    PcmDmaData_t *ptDmaData = (PcmDmaData_t *)(g_aRuntimeData[eAioDma].pPrivateData);
    s32           nErr      = 0;
    PcmFrames_t   tAvail;
    PcmFrames_t   tFreeSize;
    PcmFrames_t   tWorkSize;
    PcmFrames_t   tOffset = 0;
    PcmFrames_t   tApplptr;
    PcmFrames_t   tBufSize;

    AUD_PRINTF(DEBUG_LEVEL, "%s chn=%d\n", __FUNCTION__, eAioDma);
    if (nSize == 0)
    {
        goto null_data;
    }
    else
    {
        tBufSize = _BytesToFrames(&g_aRuntimeData[eAioDma], nSize);
    }

    // CamOsSpinLockIrqSave(&g_aRuntimeData[eAioDma].tPcmLock);
    switch (g_aRuntimeData[eAioDma].nStatus)
    {
        case SND_PCM_STATE_PREPARED:
        case SND_PCM_STATE_RUNNING:
        case SND_PCM_STATE_PAUSED:
            break;
        case SND_PCM_STATE_XRUN:
            nErr = -EPIPE;
            goto _end_unlock;
        default:
            nErr = -EBADFD;
            goto _end_unlock;
    }

    tAvail = _AudPcmCaptureAvail(ptDmaData);
    AUD_PRINTF(DEBUG_LEVEL, "%s tAvail %lu,tBufSize %lu\n", __FUNCTION__, tAvail, tBufSize);
    while (tBufSize > 0)
    {
        if (tAvail == 0)
        {
#if 0
            if (!bBlock)
            {
                nErr = -EAGAIN;
                goto _end_unlock;
            }
#endif
            AUD_PRINTF(DEBUG_LEVEL, "_WaitForAvail\n");
            nErr = _WaitForAvail(&g_aRuntimeData[eAioDma], &tAvail);
            if (nErr < 0)
            {
                goto _end_unlock;
            }
            AUD_PRINTF(DEBUG_LEVEL, "_WaitForAvail avail %lu\n", tAvail);
        }

        tWorkSize = (tBufSize > tAvail ? tAvail : tBufSize);
        tFreeSize = ptDmaData->tBufSize - (ptDmaData->tApplPtr % ptDmaData->tBufSize);

        if (tWorkSize > tFreeSize)
        {
            tWorkSize = tFreeSize;
        }

        if (tWorkSize == 0)
        {
            nErr = -EINVAL;
            goto _end_unlock;
        }

        tApplptr = ptDmaData->tApplPtr % ptDmaData->tBufSize;

        // CamOsSpinUnlockIrqRestore(&g_aRuntimeData[eAioDma].tPcmLock);
        nErr = _AudDmaKernelCopy(&g_aRuntimeData[eAioDma], tApplptr, pRdBuffer, nSize, tOffset, tWorkSize);
        // CamOsSpinLockIrqSave(&g_aRuntimeData[eAioDma].tPcmLock);

        if (nErr < 0)
        {
            goto _end_unlock;
        }

        switch (g_aRuntimeData[eAioDma].nStatus)
        {
            case SND_PCM_STATE_XRUN:
                nErr = -EPIPE;
                goto _end_unlock;

            default:
                break;
        }
        tApplptr += tWorkSize;
        if (tApplptr >= ptDmaData->tBufSize)
        {
            tApplptr -= ptDmaData->tBufSize;
        }
        ptDmaData->tApplPtr = tApplptr;
        tOffset += tWorkSize;
        tBufSize -= tWorkSize;
        tAvail -= tWorkSize;
    }

_end_unlock:

    // CamOsSpinUnlockIrqRestore(&g_aRuntimeData[eAioDma].tPcmLock);
    return nErr < 0 ? nErr : _FramesToBytes(&g_aRuntimeData[eAioDma], tOffset); // nSize;

null_data:
    return 0;
}

int DrvAudSetAdcGain(CHIP_AI_ADC_e eAdc, U16 nSel)
{
    return HalAudAtopSetAdcGain(eAdc, nSel);
}

int DrvAudSetMicAmpGain(CHIP_AI_ADC_e eAdc, U16 nSel)
{
    return HalAudAtopMicAmpGain(eAdc, nSel);
}

int DrvAudAdcSetMux(CHIP_AI_ADC_e eAdc, CHIP_ADC_MUX_e eAdcMux)
{
    return HalAudAtopSetAdcMux(eAdc, eAdcMux);
}

int DrvAudDpgaSetGain(EN_CHIP_DPGA eDpga, S8 s8Gain, S8 s8Ch)
{
    return HalAudDpgaSetGain(eDpga, s8Gain, s8Ch);
}

int DrvAudDpgaSetGainFading(EN_CHIP_DPGA eDpga, U8 nFading, S8 s8Ch)
{
    return HalAudDpgaSetGainFading(eDpga, nFading, s8Ch);
}

int DrvAudConfigDigMicParam(CHIP_AI_DMIC_e eChipDmic, DigMicParam_t *ptDigMicParam)
{
    return HalAudConfigDigMicParam(eChipDmic, ptDigMicParam);
}

int DrvAudConfigDigMicSampleRate(CHIP_AI_DMIC_e eChipDmic, u32 nSampleRate)
{
    return HalAudConfigDigMicSampleRate(eChipDmic, nSampleRate);
}

int DrvAudConfigDigMicChNum(CHIP_AI_DMIC_e eChipDmic, u8 nChNum)
{
    return HalAudConfigDigMicChNum(eChipDmic, nChNum);
}

int DrvAudDigMicEnable(CHIP_AI_DMIC_e eDmic, BOOL bEn)
{
    return HalAudDigMicEnable(eDmic, bEn);
}

int DrvAudSetDigMicGain(CHIP_AI_DMIC_e eDmic, S8 s8Gain, S8 s8Ch)
{
    return HalAudDigMicSetGain(eDmic, s8Gain, s8Ch);
}

int DrvAudGetBufCurrDataLen(CHIP_AIO_DMA_e eAioDma, u32 *len)
{
    // unsigned long nFlags;
    u32           nCurDmaLevelCount = 0;
    PcmDmaData_t *ptDmaData         = (PcmDmaData_t *)(g_aRuntimeData[eAioDma].pPrivateData);

    // CamOsSpinLockIrqSave(&ptDmaData->tLock);
    nCurDmaLevelCount = HalAudDmaGetLevelCnt(ptDmaData->nChnId);
    // CamOsSpinUnlockIrqRestore(&ptDmaData->tLock);

    *len = nCurDmaLevelCount;

    return AIO_OK;
}

int DrvAudConfigSrcParam(AI_CH_e eAiCh, SrcParam_t *ptSrcParam)
{
    return HalAudConfigSrcParam(eAiCh, ptSrcParam);
}

int DrvAudEnableHdmi(CHIP_AO_HDMI_e eHdmi, BOOL bEn)
{
    return HalAudHdmiEanble(eHdmi, bEn);
}

int DrvAudAiIfSetGain(AI_CH_e eAiCh, S16 s16Gain)
{
    return HalAudAiSetGain(eAiCh, s16Gain);
}

int DrvAudAoIfSetGain(AO_CH_e eAoCh, S16 s16Gain)
{
    return HalAudAoSetGain(eAoCh, s16Gain);
}

int DrvAudAiIfSetMute(AI_CH_e eAiCh, BOOL bEn)
{
    return HalAudAiSetIfMute(eAiCh, bEn);
}

int DrvAudAoIfSetMute(AO_CH_e eAoCh, BOOL bEn)
{
    return HalAudAoSetIfMute(eAoCh, bEn);
}

int DrvAudAiSetDpgaGain(CHIP_AIO_DMA_e eAioDma, U8 nDmaCh, AI_CH_e eAiCh, S16 s16Gain)
{
    return HalAudAiSetDpgaGain(eAioDma, nDmaCh, eAiCh, s16Gain);
}

int DrvAudAoSetDpgaGain(CHIP_AIO_DMA_e eAioDma, U8 nDmaCh, AO_CH_e eAoCh, S16 s16Gain, U8 nFading)
{
    return HalAudAoSetDpgaGain(eAioDma, nDmaCh, eAoCh, s16Gain, nFading);
}

int DrvAudAiSetDpgaMute(CHIP_AIO_DMA_e eAioDma, U8 nDmaCh, AI_CH_e eAiCh, BOOL bEn)
{
    return HalAudAiSetDpgaMute(eAioDma, nDmaCh, eAiCh, bEn);
}

int DrvAudAoSetDpgaMute(CHIP_AIO_DMA_e eAioDma, U8 nDmaCh, AO_CH_e eAoCh, BOOL bEn, U8 nFading)
{
    return HalAudAoSetDpgaMute(eAioDma, nDmaCh, eAoCh, bEn, nFading);
}

int DrvAudDirectSetDpgaGain(AI_CH_e eAiCh, AO_CH_e eAoCh, S16 s16Gain)
{
    return HalAudDirectSetDpgaGain(eAiCh, eAoCh, s16Gain);
}

int DrvAudAoIfMultiAttachAction(AO_CH_e eAoCh, U8 nAttachCount)
{
    return HalAudAoIfMultiAttachAction(eAoCh, nAttachCount);
}

int DrvAudSineGenEnable(SINE_GEN_e enSineGen, BOOL bEn)
{
    return HalAudDmaSineGenEnable(enSineGen, bEn);
}

int DrvAudSineGenSetting(SINE_GEN_e enSineGen, U8 u8Freq, S8 s8Gain)
{
    return HalAudDmaSineGenSetting(enSineGen, u8Freq, s8Gain);
}

int DrvAudAiSetIfSampleRate(AI_IF_e eAiIf, u32 nSampleRate)
{
    return HalAudAiSetIfSampleRate(eAiIf, nSampleRate);
}

int DrvAudAoSetIfSampleRate(AO_IF_e eAoIf, u32 nSampleRate)
{
    return HalAudAoSetIfSampleRate(eAoIf, nSampleRate);
}
