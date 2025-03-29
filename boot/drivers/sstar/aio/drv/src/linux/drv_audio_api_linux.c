/*
 * drv_audio_api_linux.c - Sigmastar
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

#include "ms_platform.h"

//
#include "cam_os_wrapper.h"

//
#include "cam_sysfs.h"
#include "cam_clkgen.h"
#include "drv_gpio_io.h"

//
#include <linux/interrupt.h>
#include <linux/of_irq.h>
#include <linux/gpio.h>
#include <linux/clk.h>

//
#include "gpio.h"
#include "padmux.h"
#include "drv_gpio.h"
#include "drv_padmux.h"

//

// Hal
#include "hal_audio_common.h"
#include "hal_audio_sys.h"
#include "hal_audio_config.h"
#include "hal_audio_types.h"
#include "hal_audio_api.h"
#include "hal_audio_os_api.h"

// Drv
#include "drv_audio_dbg.h"
#include "drv_audio_api.h"
#include "drv_audio_pri.h"

#ifdef CONFIG_CAM_CLK
#include "drv_camclk_Api.h"
#endif

// ------------------------
// Variable
// ------------------------
static unsigned int           g_infinityIrqId       = 0;
static DMA_STOP_CHANNEL_CB_FN g_pDmaStopChannelCbFn = NULL;

// ------------------------
// Static declaration
// ------------------------
static irqreturn_t _AudDmaIrq(int irq, void *dev_id);

static irqreturn_t _AudDmaIrq(int irq, void *dev_id)
{
    PcmRuntimeData_t *ptRuntimeData = (PcmRuntimeData_t *)dev_id;
    PcmDmaData_t *    ptDmaData     = (PcmDmaData_t *)ptRuntimeData->pPrivateData;
    // unsigned long nFlags;
    BOOL bTrigger = 0, bBoundary = 0, bLocalData = 0;
    BOOL bTriggerInt = 0, bBoundaryInt = 0; //,bLocalDataInt;

    // AUD_PRINTF(IRQ_LEVEL, "Function - %s, DMA Id = %d, pre state = %d\n", __func__, ptDmaData->nChnId,
    // ptRuntimeData->nState);
    CamOsSpinLockIrqSave(&ptDmaData->tLock);

    HalAudDmaGetFlags(ptDmaData->nChnId, &bTrigger, &bBoundary, &bLocalData);
    HalAudDmaGetInt(ptDmaData->nChnId, &bTriggerInt, &bBoundaryInt);

    if (((bTrigger == FALSE) || (bTriggerInt == FALSE)) && ((bBoundary == FALSE) || (bBoundaryInt == FALSE))
        //&& ( ( bLocalData == FALSE ) || ( bLocalDataInt == FALSE ) )
    )
    {
        // CamOsSpinUnlockIrqRestore(&ptDmaData->tLock);
        return IRQ_NONE;
    }

    if ((E_PCM_STREAM_PLAYBACK == ptRuntimeData->eStream) && ptRuntimeData->nStatus == SND_PCM_STATE_RUNNING)
    {
        if (ptRuntimeData->nState != DMA_EMPTY)
        {
            if (bBoundary == TRUE)
            {
                HalAudDmaIntEnable(ptDmaData->nChnId, FALSE, FALSE);
                g_pDmaStopChannelCbFn(ptDmaData->nChnId);
                ptRuntimeData->nStatus = SND_PCM_STATE_XRUN;
                ptRuntimeData->nState  = DMA_EMPTY;

                AUD_PRINTF(IRQ_LEVEL, "EMPTY: DMA Id = %d\n", ptDmaData->nChnId);
            }
            else if ((ptRuntimeData->nState != DMA_UNDERRUN) && bTrigger == TRUE)
            {
                HalAudDmaIntEnable(ptDmaData->nChnId, FALSE, TRUE);

                ptRuntimeData->nState = DMA_UNDERRUN;
                // CamOsSpinUnlockIrqRestore(&ptDmaData->tLock);

                CamOsTsemUp(&ptRuntimeData->tsleep);
                // CamOsSpinLockIrqSave(&ptDmaData->tLock);
                AUD_PRINTF(IRQ_LEVEL, "UNDER: DMA Id = %d, LC = %u\n", ptDmaData->nChnId,
                           HalAudDmaGetLevelCnt(ptDmaData->nChnId));
            }
            else
            {
                AUD_PRINTF(ERROR_LEVEL, "[%d] AO UNKNOWN flags %d, %d, %d\n", __LINE__, bTrigger, bBoundary,
                           bLocalData);
                AUD_PRINTF(ERROR_LEVEL, "[%d] AO UNKNOWN interrupt %d, %d\n", __LINE__, bTriggerInt, bBoundaryInt);
            }
        }
        else
        {
            if (bBoundary == TRUE)
            {
                HalAudDmaIntEnable(ptDmaData->nChnId, FALSE, FALSE);
                g_pDmaStopChannelCbFn(ptDmaData->nChnId);
                ptRuntimeData->nStatus = SND_PCM_STATE_XRUN;
                ptRuntimeData->nState  = DMA_EMPTY;

                AUD_PRINTF(IRQ_LEVEL, "EMPTY: DMA Id = %d\n", ptDmaData->nChnId);
            }
            else
            {
                AUD_PRINTF(ERROR_LEVEL, "[%d] AO UNKNOWN flags %d, %d, %d\n", __LINE__, bTrigger, bBoundary,
                           bLocalData);
                AUD_PRINTF(ERROR_LEVEL, "[%d] AO UNKNOWN interrupt %d, %d\n", __LINE__, bTriggerInt, bBoundaryInt);
            }
        }
    }
    else if ((E_PCM_STREAM_CAPTURE == ptRuntimeData->eStream)
             && (ptRuntimeData->nStatus == SND_PCM_STATE_RUNNING)) // CAPTURE device
    {
        if (ptRuntimeData->nState != DMA_FULL)
        {
            if (bBoundary == TRUE)
            {
                HalAudDmaIntEnable(ptDmaData->nChnId, FALSE, FALSE);
                g_pDmaStopChannelCbFn(ptDmaData->nChnId);
                ptRuntimeData->nStatus = SND_PCM_STATE_XRUN;

                AUD_PRINTF(IRQ_LEVEL, "FULL: DMA Id = %d\n", ptDmaData->nChnId);

                ptRuntimeData->nState = DMA_FULL;
            }
            else if ((ptRuntimeData->nState != DMA_OVERRUN) && bTrigger == TRUE)
            {
                HalAudDmaIntEnable(ptDmaData->nChnId, FALSE, TRUE);

                ptRuntimeData->nState = DMA_OVERRUN;
                CamOsSpinUnlockIrqRestore(&ptDmaData->tLock);

                CamOsTsemUp(&ptRuntimeData->tsleep);
                CamOsSpinLockIrqSave(&ptDmaData->tLock);

                AUD_PRINTF(IRQ_LEVEL, "OVER: DMA Id = %d, LC = %u\n", ptDmaData->nChnId,
                           HalAudDmaGetLevelCnt(ptDmaData->nChnId));
            }
            else
            {
                AUD_PRINTF(ERROR_LEVEL, "[%d] AI UNKNOWN flags %d, %d, %d\n", __LINE__, bTrigger, bBoundary,
                           bLocalData);
                AUD_PRINTF(ERROR_LEVEL, "[%d] AI UNKNOWN interrupt %d, %d\n", __LINE__, bTriggerInt, bBoundaryInt);
            }
        }
        else
        {
            if (bBoundary == TRUE)
            {
                HalAudDmaIntEnable(ptDmaData->nChnId, FALSE, FALSE);
                g_pDmaStopChannelCbFn(ptDmaData->nChnId);
                ptRuntimeData->nStatus = SND_PCM_STATE_XRUN;

                AUD_PRINTF(IRQ_LEVEL, "FULL: DMA Id = %d\n", ptDmaData->nChnId);

                ptRuntimeData->nState = DMA_FULL;
            }
            else
            {
                AUD_PRINTF(ERROR_LEVEL, "[%d] AI UNKNOWN flags %d, %d, %d\n", __LINE__, bTrigger, bBoundary,
                           bLocalData);
                AUD_PRINTF(ERROR_LEVEL, "[%d] AI UNKNOWN interrupt %d, %d\n", __LINE__, bTriggerInt, bBoundaryInt);
            }
        }
    }
    else
    {
        AUD_PRINTF(ERROR_LEVEL, "[%s] Status != RUNNING\n", __func__);
        AUD_PRINTF(ERROR_LEVEL, "DMA Id = %d, Current state = %d\n", ptDmaData->nChnId, ptRuntimeData->nState);
        AUD_PRINTF(ERROR_LEVEL, "bTrigger = %d   bTriggerInt = %d\r\n", bTrigger, bTriggerInt);
        AUD_PRINTF(ERROR_LEVEL, "bBoundary = %d  bBoundaryInt = %d\r\n", bBoundary, bBoundaryInt);

        HalAudDmaIntEnable(ptDmaData->nChnId, FALSE, FALSE);
    }

    AUD_PRINTF(IRQ_LEVEL, "Function - %s, DMA Id = %d, cur state = %d\n", __func__, ptDmaData->nChnId,
               ptRuntimeData->nState);
    AUD_PRINTF(IRQ_LEVEL, "bTrigger = %d   bTriggerInt = %d\r\n", bTrigger, bTriggerInt);
    AUD_PRINTF(IRQ_LEVEL, "bBoundary = %d  bBoundaryInt = %d\r\n", bBoundary, bBoundaryInt);
    // AUD_PRINTF(IRQ_LEVEL, "bLocalData = %d bLocalDataInt = %d\r\n",bLocalData,bLocalDataInt);

    CamOsSpinUnlockIrqRestore(&ptDmaData->tLock);

    return IRQ_HANDLED;
}

int DrvAudApiIrqInit(void)
{
    return HalAudApiGetIrqId(&g_infinityIrqId);
}

void DrvAudApiIrqSetDmaStopCb(DMA_STOP_CHANNEL_CB_FN pfnCb)
{
    g_pDmaStopChannelCbFn = pfnCb;
}

int DrvAudApiIrqRequest(const char *name, void *dev_id)
{
    s32 err = 0;

    AUD_PRINTF(TRACE_LEVEL, "IRQ_ID = 0x%x\n", g_infinityIrqId);

    err = request_irq(g_infinityIrqId, _AudDmaIrq, IRQF_SHARED, name, dev_id);
    if (err)
    {
        AUD_PRINTF(ERROR_LEVEL, "%s request irq err=%d\n", __FUNCTION__, err);
        goto FAIL;
    }

    return AIO_OK;

FAIL:

    return AIO_NG;
}

BOOL DrvAudApiIrqFree(void *dev_id)
{
    free_irq(g_infinityIrqId, dev_id);

    return TRUE;
}

u64 DrvAudApiIoAddress(u32 addr)
{
    return (u64)IO_ADDRESS(addr);
}
