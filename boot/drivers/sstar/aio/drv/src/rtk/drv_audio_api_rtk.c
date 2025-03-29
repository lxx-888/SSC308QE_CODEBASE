/*
 * drv_audio_api_rtk.c - Sigmastar
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

//
#include "cam_os_wrapper.h"
#include "sys_MsWrapper_cus_os_int_ctrl.h"
#include "hal_int_ctrl_pub.h"
#include "drv_gpio_io.h"

// Hal
#include "hal_audio_common.h"
#include "hal_audio_sys.h"
#include "hal_audio_config.h"
#include "hal_audio_types.h"
#include "hal_audio_api.h"
#include "hal_audio_reg.h"

// Drv
#include "drv_audio_dbg.h"
#include "drv_audio_api.h"
#include "drv_audio_pri.h"

// ------------------------
// DTS
// ------------------------
#include "rtk_audio_dts_st.h"

#include "rtk_bach_board_cfg.c"

// ------------------------
// IRQ
// ------------------------
// Follow Linux's enum
enum IRQRETURN_e
{
    IRQ_NONE        = (0 << 0),
    IRQ_HANDLED     = (1 << 0),
    IRQ_WAKE_THREAD = (1 << 1),
};

typedef enum IRQRETURN_e irqreturn_t;

//
typedef struct
{
    BOOL  IsActive;
    void *dev_id;

} RtkIrqInfo_t;

typedef struct
{
    int             rtkIrqReqCnt;
    RtkIrqInfo_t    rtkIrqInfo[E_AUD_DMA_NUM];
    CamOsSpinlock_t rtkIrqLock;

} RtkIrqHandle_t;

// ------------------------
// Default Value
// ------------------------
#define AUD_DEF_IRQ_ID MS_INT_NUM_IRQ_BACH
#define AUD_DEF_AMP_GPIO               \
    {                                  \
        PAD_UNKNOWN, 0, PAD_UNKNOWN, 0 \
    }

// ------------------------
// Variable
// ------------------------
static unsigned int           g_infinityIrqId       = AUD_DEF_IRQ_ID;
static U32                    g_aAmpGpio[4]         = AUD_DEF_AMP_GPIO;
static DMA_STOP_CHANNEL_CB_FN g_pDmaStopChannelCbFn = NULL;

// irq
static RtkIrqHandle_t g_tRtkIrqHandle;

// ------------------------
// Static declaration
// ------------------------
static void        _AudDmaIrqRtk(u32 irq, void *pri);
static irqreturn_t _AudDmaIrqRtkCore(int irq, void *dev_id);

static void _AudDmaIrqRtk(u32 irq, void *pri)
{
    int i = 0;

    CamOsSpinLockIrqSave(&g_tRtkIrqHandle.rtkIrqLock);

    for (i = 0; i < E_AUD_DMA_NUM; i++)
    {
        if ((g_tRtkIrqHandle.rtkIrqInfo[i].IsActive == TRUE) && (g_tRtkIrqHandle.rtkIrqInfo[i].dev_id != NULL))
        {
            _AudDmaIrqRtkCore(0, g_tRtkIrqHandle.rtkIrqInfo[i].dev_id);
        }
    }

    CamOsSpinUnlockIrqRestore(&g_tRtkIrqHandle.rtkIrqLock);
}

static irqreturn_t _AudDmaIrqRtkCore(int irq, void *dev_id)
{
    PcmRuntimeData_t *ptRuntimeData = (PcmRuntimeData_t *)dev_id;
    PcmDmaData_t *    ptDmaData     = (PcmDmaData_t *)ptRuntimeData->pPrivateData;
    // unsigned long nFlags;
    BOOL bTrigger = 0, bBoundary = 0, bLocalData = 0;
    BOOL bTriggerInt = 0, bBoundaryInt = 0; //,bLocalDataInt;

    CamOsSpinLockIrqSave(&ptDmaData->tLock);

    HalAudDmaGetFlags(ptDmaData->nChnId, &bTrigger, &bBoundary, &bLocalData);
    HalAudDmaGetInt(ptDmaData->nChnId, &bTriggerInt, &bBoundaryInt);

    if (((bTrigger == FALSE) || (bTriggerInt == FALSE)) && ((bBoundary == FALSE) || (bBoundaryInt == FALSE))
        //&& ( ( bLocalData == FALSE ) || ( bLocalDataInt == FALSE ) )
    )
    {
        CamOsSpinUnlockIrqRestore(&ptDmaData->tLock);
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
                CamOsSpinUnlockIrqRestore(&ptDmaData->tLock);

                CamOsTsemUp(&ptRuntimeData->tsleep);
                CamOsSpinLockIrqSave(&ptDmaData->tLock);
                AUD_PRINTF(IRQ_LEVEL, "UNDER: DMA Id = %d, LC = %u\n", ptDmaData->nChnId,
                           HalAudDmaGetLevelCnt(ptDmaData->nChnId));
            }
            else
            {
                AUD_PRINTF(ERROR_LEVEL, "AO UNKNOWN interrupt %d %d %d\n", bTrigger, bBoundary, bLocalData);
            }
        }
    }
    else if ((E_PCM_STREAM_CAPTURE == ptRuntimeData->eStream) && (ptRuntimeData->nStatus == SND_PCM_STATE_RUNNING))
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
                AUD_PRINTF(ERROR_LEVEL, "AI UNKNOWN interrupt %d %d %d\n", bTrigger, bBoundary, bLocalData);
        }
    }

    AUD_PRINTF(IRQ_LEVEL, "Function - %s, DMA Id = %d, cur state = %d\n", __func__, ptDmaData->nChnId,
               ptRuntimeData->nState);
    AUD_PRINTF(IRQ_LEVEL, "bTrigger = %d   bTriggerInt = %d\r\n", bTrigger, bTriggerInt);
    AUD_PRINTF(IRQ_LEVEL, "bBoundary = %d  bBoundaryInt = %d\r\n", bBoundary, bBoundaryInt);
    // AUD_PRINTF(IRQ_LEVEL, "bLocalData = %d bLocalDataInt = %d\r\n",bLocalData,bLocalDataInt);

    CamOsSpinUnlockIrqRestore(&ptDmaData->tLock);

    return IRQ_HANDLED;
}

void DrvAudApiDtsInit(void)
{
    int         ret, i                   = 0;
    S32         nPadMuxMode, nKeepI2sClk = 0;
    S32         nKeepAdcPowerOn  = 0;
    S32         nKeepDacPowerOn  = 0;
    S32         nI2sPcmMode      = 0;
    const char *dtsCompatibleStr = NULL, *dtsAmpGpioStr = NULL, *dtsKeepI2sClkStr = NULL;
    const char *dtsKeepAdcPowerOn = NULL, *dtsKeepDacPowerOn = NULL, *dtsI2sPcmModeStr = NULL;
    // ------------------------
    // Get Pre info.
    // ------------------------
    if (card.dts->audio_dtsCompatibleStr != NULL)
    {
        dtsCompatibleStr = card.dts->audio_dtsCompatibleStr;
    }
    else
    {
        AUD_PRINTF(ERROR_LEVEL, "%s, audio_dtsCompatibleStr = NULL !\n", __FUNCTION__);
        return;
    }

    if (card.dts->audio_dtsAmpGpioStr != NULL)
    {
        dtsAmpGpioStr = card.dts->audio_dtsAmpGpioStr;
    }
    else
    {
        AUD_PRINTF(ERROR_LEVEL, "%s, audio_dtsAmpGpioStr = NULL !\n", __FUNCTION__);
        return;
    }

    if (card.dts->audio_dtsKeepI2sClkStr != NULL)
    {
        dtsKeepI2sClkStr = card.dts->audio_dtsKeepI2sClkStr;
    }
    else
    {
        AUD_PRINTF(ERROR_LEVEL, "%s, audio_dtsKeepI2sClkStr = NULL !\n", __FUNCTION__);
        return;
    }
    if (card.dts->audio_dtsKeepAdcPowerOn != NULL)
    {
        dtsKeepAdcPowerOn = card.dts->audio_dtsKeepAdcPowerOn;
    }
    else
    {
        AUD_PRINTF(ERROR_LEVEL, "%s, audio_dtsKeepAdcPowerOn = NULL !\n", __FUNCTION__);
        return;
    }
    if (card.dts->audio_dtsKeepDacPowerOn != NULL)
    {
        dtsKeepDacPowerOn = card.dts->audio_dtsKeepDacPowerOn;
    }
    else
    {
        AUD_PRINTF(ERROR_LEVEL, "%s, audio_dtsKeepDacPowerOn = NULL !\n", __FUNCTION__);
        return;
    }
    if (card.dts->audio_dts_i2s_pcm_str != NULL)
    {
        dtsI2sPcmModeStr = card.dts->audio_dts_i2s_pcm_str;
    }
    else
    {
        AUD_PRINTF(ERROR_LEVEL, "%s, audio_dts_i2s_pcm_str = NULL !\n", __FUNCTION__);
        return;
    }
    //
    if (AudRtkDtsFindCompatibleNode(dtsCompatibleStr) == TRUE)
    {
        // IRQ
        g_infinityIrqId = AudRtkDtsGetIrqId();

        //  enable for line-out
        if (AudRtkDtsGetGpio(dtsAmpGpioStr, g_aAmpGpio) != TRUE)
        {
            AUD_PRINTF(ERROR_LEVEL, "%s, Failed to gpio_request amp-gpio !\n", __FUNCTION__);
            AUD_PRINTF(ERROR_LEVEL, "%s, amp-gpio chn0 will use default:%d %d !\n", __FUNCTION__, g_aAmpGpio[0],
                       g_aAmpGpio[1]);
            AUD_PRINTF(ERROR_LEVEL, "%s, amp-gpio chn1 will use default:%d %d !\n", __FUNCTION__, g_aAmpGpio[2],
                       g_aAmpGpio[3]);
        }
        else
        {
            if (g_aAmpGpio[0] == PAD_UNKNOWN)
            {
                // Do nothing
                AUD_PRINTF(DEBUG_LEVEL, "AmpGpio chn0 = PAD_UNKNOWN, don't do CamGpioRequest !\n");
            }
            else
            {
                ret = camdriver_gpio_request(NULL, g_aAmpGpio[0]);
                if (ret)
                {
                    AUD_PRINTF(ERROR_LEVEL, "%s, Failed to gpio_request amp-gpio chn0 !\n", __FUNCTION__);
                }
            }

            if (g_aAmpGpio[2] == PAD_UNKNOWN)
            {
                // Do nothing
                AUD_PRINTF(DEBUG_LEVEL, "AmpGpio chn1 = PAD_UNKNOWN, don't do CamGpioRequest !\n");
            }
            else
            {
                ret = camdriver_gpio_request(NULL, g_aAmpGpio[2]);
                if (ret)
                {
                    AUD_PRINTF(ERROR_LEVEL, "%s, Failed to gpio_request amp-gpio chn1 !\n", __FUNCTION__);
                }
            }
        }

        for (i = 0; i < card.dts->num_padmux; i++)
        {
            if (AudRtkDtsGetPadMux(card.dts->audio_dts_padmux_list[i].str, &nPadMuxMode) == TRUE)
            {
                card.dts->audio_dts_padmux_list[i].fp(nPadMuxMode);
            }
            else
            {
                AUD_PRINTF(ERROR_LEVEL, "%s, Failed to get %s !\n", __FUNCTION__,
                           card.dts->audio_dts_padmux_list[i].str);
                AUD_PRINTF(ERROR_LEVEL, "%s, %s will use default !\n", __FUNCTION__,
                           card.dts->audio_dts_padmux_list[i].str);
            }
        }

        if (AudRtkDtsGetKeepI2sClk(dtsKeepI2sClkStr, &nKeepI2sClk) == TRUE)
        {
            HalAudI2sSetKeepClk(nKeepI2sClk);
        }
        else
        {
            AUD_PRINTF(ERROR_LEVEL, "%s, Failed to get %s attr !\n", __FUNCTION__, dtsKeepI2sClkStr);
        }

        if (AudRtkDtsGetKeepAdcPowerOn(dtsKeepAdcPowerOn, &nKeepAdcPowerOn) == TRUE)
        {
            HalAudKeepAdcPowerOn((U8)nKeepAdcPowerOn);
        }
        else
        {
            AUD_PRINTF(ERROR_LEVEL, "%s, Failed to get %s attr !\n", __FUNCTION__, dtsKeepAdcPowerOn);
        }

        if (AudRtkDtsGetKeepDacPowerOn(dtsKeepDacPowerOn, &nKeepDacPowerOn) == TRUE)
        {
            HalAudKeepDacPowerOn((U8)nKeepDacPowerOn);
        }
        else
        {
            AUD_PRINTF(ERROR_LEVEL, "%s, Failed to get %s attr !\n", __FUNCTION__, dtsKeepDacPowerOn);
        }

        if (AudRtkDtsGetI2sPcmMode(dtsI2sPcmModeStr, &nI2sPcmMode) == TRUE)
        {
            HalAudI2sSetPcmMode((U8)nI2sPcmMode);
        }
        else
        {
            // AUD_PRINTF(ERROR_LEVEL,"%s, Failed to get %s attr !\n",__FUNCTION__, dtsI2sPcmModeStr);
        }
    }
    else
    {
        AUD_PRINTF(ERROR_LEVEL, "%s, Failed to find device node !\n", __FUNCTION__);
        AUD_PRINTF(ERROR_LEVEL, "IRQ ID will use default:%d !\n", g_infinityIrqId);
        AUD_PRINTF(ERROR_LEVEL, "amp-gpio chn0 will use default:%d %d !\n", g_aAmpGpio[0], g_aAmpGpio[1]);
        AUD_PRINTF(ERROR_LEVEL, "amp-gpio chn1 will use default:%d %d !\n", g_aAmpGpio[2], g_aAmpGpio[3]);
        AUD_PRINTF(ERROR_LEVEL, "All padmux will use default !\n");
    }

    AUD_PRINTF(DEBUG_LEVEL, "g_infinityIrqId = %d !\n", g_infinityIrqId);
    AUD_PRINTF(DEBUG_LEVEL, " g_aAmpGpio[0] = %d, g_aAmpGpio[1] = %d !\n", g_aAmpGpio[0], g_aAmpGpio[1]);
    AUD_PRINTF(DEBUG_LEVEL, " g_aAmpGpio[2] = %d, g_aAmpGpio[3] = %d !\n", g_aAmpGpio[2], g_aAmpGpio[3]);
}

void DrvAudApiIrqInit(void)
{
    int              i          = 0;
    MsIntInitParam_u uInitParam = {{0}};

    // Lock for IRQ handle
    CamOsSpinInit(&g_tRtkIrqHandle.rtkIrqLock);

    g_tRtkIrqHandle.rtkIrqReqCnt = 0;

    for (i = 0; i < E_AUD_DMA_NUM; i++)
    {
        g_tRtkIrqHandle.rtkIrqInfo[i].IsActive = FALSE;
        g_tRtkIrqHandle.rtkIrqInfo[i].dev_id   = NULL;
    }

    // Register IRQ
    uInitParam.intc.eMap      = INTC_MAP_IRQ;
    uInitParam.intc.ePriority = INTC_PRIORITY_7;
    uInitParam.intc.pfnIsr    = _AudDmaIrqRtk;

    MsInitInterrupt(&uInitParam, g_infinityIrqId);
}

void DrvAudApiIrqSetDmaStopCb(DMA_STOP_CHANNEL_CB_FN pfnCb)
{
    g_pDmaStopChannelCbFn = pfnCb;
}

BOOL DrvAudApiIrqRequest(const char *name, void *dev_id)
{
    PcmRuntimeData_t *ptRuntimeData = (PcmRuntimeData_t *)dev_id;
    PcmDmaData_t *    ptDmaData     = (PcmDmaData_t *)ptRuntimeData->pPrivateData;

    if (ptDmaData->nChnId >= E_AUD_DMA_NUM)
    {
        return FALSE;
    }

    CamOsSpinLockIrqSave(&g_tRtkIrqHandle.rtkIrqLock);

    g_tRtkIrqHandle.rtkIrqInfo[ptDmaData->nChnId].IsActive = TRUE;
    g_tRtkIrqHandle.rtkIrqInfo[ptDmaData->nChnId].dev_id   = dev_id;

    if (g_tRtkIrqHandle.rtkIrqReqCnt == 0)
    {
        MsUnmaskInterrupt(g_infinityIrqId);
    }

    g_tRtkIrqHandle.rtkIrqReqCnt++;

    CamOsSpinUnlockIrqRestore(&g_tRtkIrqHandle.rtkIrqLock);
    return TRUE;
}

BOOL DrvAudApiIrqFree(void *dev_id)
{
    PcmRuntimeData_t *ptRuntimeData = (PcmRuntimeData_t *)dev_id;
    PcmDmaData_t *    ptDmaData     = (PcmDmaData_t *)ptRuntimeData->pPrivateData;

    if (ptDmaData->nChnId >= E_AUD_DMA_NUM)
    {
        return FALSE;
    }

    CamOsSpinLockIrqSave(&g_tRtkIrqHandle.rtkIrqLock);

    g_tRtkIrqHandle.rtkIrqInfo[ptDmaData->nChnId].IsActive = FALSE;
    g_tRtkIrqHandle.rtkIrqInfo[ptDmaData->nChnId].dev_id   = NULL;

    if (g_tRtkIrqHandle.rtkIrqReqCnt > 0)
    {
        g_tRtkIrqHandle.rtkIrqReqCnt--;

        if (g_tRtkIrqHandle.rtkIrqReqCnt == 0)
        {
            MsMaskInterrupt(g_infinityIrqId);
        }
    }
    else
    {
        CamOsSpinUnlockIrqRestore(&g_tRtkIrqHandle.rtkIrqLock);
        return FALSE;
    }

    CamOsSpinUnlockIrqRestore(&g_tRtkIrqHandle.rtkIrqLock);
    return TRUE;
}

BOOL DrvAudApiAoAmpEnable(BOOL bEnable, S8 s8Ch)
{
    if ((g_aAmpGpio[0] == PAD_UNKNOWN) && (g_aAmpGpio[2] == PAD_UNKNOWN))
    {
        // Do nothing
        AUD_PRINTF(DEBUG_LEVEL, "AmpGpio chn0 & chn1 = PAD_UNKNOWN in %s\n", __FUNCTION__);
        return TRUE;
    }

    if (bEnable == TRUE)
    {
        // enable gpio for line-out, should after atop enable
        if (s8Ch == 0)
        {
            if (g_aAmpGpio[0] != PAD_UNKNOWN)
                camdriver_gpio_direction_output(NULL, g_aAmpGpio[0], g_aAmpGpio[1]);
        }
        else if (s8Ch == 1)
        {
            if (g_aAmpGpio[2] != PAD_UNKNOWN)
                camdriver_gpio_direction_output(NULL, g_aAmpGpio[2], g_aAmpGpio[3]);
        }
        else
        {
            AUD_PRINTF(ERROR_LEVEL, "AmpGpio channel error, in %s\n", __FUNCTION__);
        }

#if 0   // QFN128
        // PAD_PM_GPIO0
        HalBachWriteReg2Byte(0xF00, 0x0003, 0x0002); // Output High
#elif 0 // QFN88 (Board must be modify manually)
        // PAD_SAR_GPIO1(79)
        HalBachWriteReg2Byte(0x1422, 0x3F3F, 0x0000); // Switch to GPIO mode + Output enable
        HalBachWriteReg2Byte(0x1423, 0x0002, 0x0000); // Output
        HalBachWriteReg2Byte(0x1424, 0x0002, 0x0002); // High
#else   // Cocoa
        // PAD_PM_LED1(77)
        HalBachWriteReg2Byte(0xF96, 0x0003, 0x0002); // Output High
#endif
    }
    else
    {
        // disable gpio for line-out, should before atop disable
        if (s8Ch == 0)
        {
            if (g_aAmpGpio[0] != PAD_UNKNOWN)
                camdriver_gpio_direction_output(NULL, g_aAmpGpio[0], !(g_aAmpGpio[1]));
        }
        else if (s8Ch == 1)
        {
            if (g_aAmpGpio[2] != PAD_UNKNOWN)
                camdriver_gpio_direction_output(NULL, g_aAmpGpio[2], !(g_aAmpGpio[3]));
        }
        else
        {
            AUD_PRINTF(ERROR_LEVEL, "AmpGpio channel error, in %s\n", __FUNCTION__);
        }

#if 0   // QFN128
        // PAD_PM_GPIO0
        HalBachWriteReg2Byte(0xF00, 0x0003, 0x0000); // Output Low
#elif 0 // QFN88 (Board must be modify manually)
        // PAD_SAR_GPIO1(79)
        HalBachWriteReg2Byte(0x1422, 0x3F3F, 0x0000); // Switch to GPIO mode + Output enable
        HalBachWriteReg2Byte(0x1423, 0x0002, 0x0000); // Output
        HalBachWriteReg2Byte(0x1424, 0x0002, 0x0000); // Low
#else   // Cocoa
        // PAD_PM_LED1(77)
        HalBachWriteReg2Byte(0xF96, 0x0003, 0x0000); // Output Low
#endif
    }

    return TRUE;
}

BOOL DrvAudApiAoAmpCtrlInit(void)
{
    return TRUE;
}

u32 DrvAudApiIoAddress(u32 addr)
{
    return addr;
}
