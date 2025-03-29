/*
 * drv_disp_if.c- Sigmastar
 *
 * Copyright (c) [2019~2020] SigmaStar Technology.
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

#define _DRV_DISP_IF_C_

//-------------------------------------------------------------------------------------------------
//  Include Files
//-------------------------------------------------------------------------------------------------
#include "mi_common_datatype.h"
#include "drv_disp_os.h"
#include "hal_disp_include.h"
#include "disp_debug.h"
#include "mhal_cmdq.h"
#include "drv_disp_ctx.h"
#include "drv_disp_irq.h"
#include "drv_disp_if.h"
//-------------------------------------------------------------------------------------------------
//  Defines & Macro
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//  structure
//-------------------------------------------------------------------------------------------------
typedef struct
{
    MI_U8                            bValid;
    MI_DISP_OutputTiming_e           enDevTimingType;
    HAL_DISP_ST_DeviceTimingConfig_t stDevTiming;
} DRV_DISP_IF_TimingData_t;

typedef struct
{
    MI_U32                    u32DataSize;
    DRV_DISP_IF_TimingData_t *pstData;
} DRV_DISP_IF_TimingConfig_t;
//-------------------------------------------------------------------------------------------------
//  Variable
//-------------------------------------------------------------------------------------------------
DRV_DISP_IF_TimingConfig_t    g_stDispDevTimingCfg = {0, NULL};
extern DRV_DISP_CTX_Contain_t g_stDispCtxContainTbl[HAL_DISP_CTX_MAX_INST];

MI_U32 g_u32DispDbgLevel = 0;

//-------------------------------------------------------------------------------------------------
//  Local Functions
//-------------------------------------------------------------------------------------------------
static HAL_DISP_ST_DeviceTimingConfig_t *_DRV_DISP_IF_GetTimingFromTbl(MI_DISP_OutputTiming_e eOutputTiming)
{
    MI_U32                            i;
    HAL_DISP_ST_DeviceTimingConfig_t *pstHalDeviceTimingCfg = NULL;
    DRV_DISP_IF_TimingData_t *        pstTimingData         = NULL;

    for (i = 0; i < g_stDispDevTimingCfg.u32DataSize; i++)
    {
        pstTimingData = &g_stDispDevTimingCfg.pstData[i];
        if (pstTimingData->bValid && pstTimingData->enDevTimingType == eOutputTiming)
        {
            pstHalDeviceTimingCfg = &pstTimingData->stDevTiming;
            break;
        }
    }

    return pstHalDeviceTimingCfg;
}

static MI_U8 _DRV_DISP_IF_TransDeviceOutpuTimingInfoToHal(MI_DISP_IMPL_MhalDeviceTimingInfo_t *pstDeviceTimingInfo,
                                                          HAL_DISP_ST_DeviceTimingInfo_t *     pstHalDevTimingInfo)
{
    MI_U8                             bRet                  = 1;
    HAL_DISP_ST_DeviceTimingConfig_t *pstHalDeviceTimingCfg = NULL;
    MI_U32                            u32Tmp;

    pstHalDevTimingInfo->eTimeType = pstDeviceTimingInfo->eTimeType;

    if (pstHalDevTimingInfo->eTimeType == E_MI_DISP_OUTPUT_USER && pstDeviceTimingInfo->pstSyncInfo)
    {
        CamOsMemset(&pstHalDevTimingInfo->stDeviceTimingCfg, 0, sizeof(HAL_DISP_ST_DeviceTimingConfig_t));

        pstHalDevTimingInfo->stDeviceTimingCfg.u16HsyncWidth     = pstDeviceTimingInfo->pstSyncInfo->u16Hpw;
        pstHalDevTimingInfo->stDeviceTimingCfg.u16HsyncBackPorch = pstDeviceTimingInfo->pstSyncInfo->u16Hbb;
        pstHalDevTimingInfo->stDeviceTimingCfg.u16Hactive        = pstDeviceTimingInfo->pstSyncInfo->u16Hact;
        pstHalDevTimingInfo->stDeviceTimingCfg.u16Hstart         = pstDeviceTimingInfo->pstSyncInfo->u16HStart;
        pstHalDevTimingInfo->stDeviceTimingCfg.u16Htotal =
            pstDeviceTimingInfo->pstSyncInfo->u16Hpw + pstDeviceTimingInfo->pstSyncInfo->u16Hbb
            + pstDeviceTimingInfo->pstSyncInfo->u16Hact + pstDeviceTimingInfo->pstSyncInfo->u16Hfb;
        pstHalDevTimingInfo->stDeviceTimingCfg.u16VsyncWidth     = pstDeviceTimingInfo->pstSyncInfo->u16Vpw;
        pstHalDevTimingInfo->stDeviceTimingCfg.u16VsyncBackPorch = pstDeviceTimingInfo->pstSyncInfo->u16Vbb;
        pstHalDevTimingInfo->stDeviceTimingCfg.u16Vactive        = pstDeviceTimingInfo->pstSyncInfo->u16Vact;
        pstHalDevTimingInfo->stDeviceTimingCfg.u16Vstart         = pstDeviceTimingInfo->pstSyncInfo->u16VStart;
        pstHalDevTimingInfo->stDeviceTimingCfg.u16Vtotal =
            pstDeviceTimingInfo->pstSyncInfo->u16Vpw + pstDeviceTimingInfo->pstSyncInfo->u16Vbb
            + pstDeviceTimingInfo->pstSyncInfo->u16Vact + pstDeviceTimingInfo->pstSyncInfo->u16Vfb;
        pstHalDevTimingInfo->stDeviceTimingCfg.u16Fps = pstDeviceTimingInfo->pstSyncInfo->u32FrameRate;

        pstHalDevTimingInfo->stDeviceTimingCfg.u32Dclk = pstHalDevTimingInfo->stDeviceTimingCfg.u16Vtotal
                                                         * pstHalDevTimingInfo->stDeviceTimingCfg.u16Htotal
                                                         * pstHalDevTimingInfo->stDeviceTimingCfg.u16Fps;

        u32Tmp = 1000000UL / (MI_U32)pstHalDevTimingInfo->stDeviceTimingCfg.u16Fps;
        pstHalDevTimingInfo->stDeviceTimingCfg.u32VSyncPeriod =
            u32Tmp * (MI_U32)pstDeviceTimingInfo->pstSyncInfo->u16Vpw
            / (MI_U32)pstHalDevTimingInfo->stDeviceTimingCfg.u16Vtotal;
        pstHalDevTimingInfo->stDeviceTimingCfg.u32VBackPorchPeriod =
            u32Tmp * (MI_U32)pstDeviceTimingInfo->pstSyncInfo->u16Vbb
            / (MI_U32)pstHalDevTimingInfo->stDeviceTimingCfg.u16Vtotal;
        pstHalDevTimingInfo->stDeviceTimingCfg.u32VActivePeriod =
            u32Tmp * (MI_U32)pstDeviceTimingInfo->pstSyncInfo->u16Vact
            / (MI_U32)pstHalDevTimingInfo->stDeviceTimingCfg.u16Vtotal;
        pstHalDevTimingInfo->stDeviceTimingCfg.u32VFrontPorchPeriod =
            u32Tmp * (MI_U32)pstDeviceTimingInfo->pstSyncInfo->u16Vfb
            / (MI_U32)pstHalDevTimingInfo->stDeviceTimingCfg.u16Vtotal;
    }
    else
    {
        pstHalDeviceTimingCfg = _DRV_DISP_IF_GetTimingFromTbl(pstHalDevTimingInfo->eTimeType);
        if (pstHalDeviceTimingCfg)
        {
            CamOsMemcpy(&pstHalDevTimingInfo->stDeviceTimingCfg, pstHalDeviceTimingCfg,
                        sizeof(HAL_DISP_ST_DeviceTimingConfig_t));
        }
        else
        {
            bRet = 0;
            DISP_ERR("%s %d, Can't find Timing(%s) in Table\n", __FUNCTION__, __LINE__,
                     PARSING_HAL_TMING_ID(pstHalDevTimingInfo->eTimeType));
        }
    }
    return bRet;
}

static void _DRV_DISP_IF_TransDeviceCvbsParamToHal(MI_DISP_CvbsParam_t *      pstCvbsParam,
                                                   HAL_DISP_ST_DeviceParam_t *pstHalDevParam)
{
    CamOsMemcpy(&pstHalDevParam->stCsc, &pstCvbsParam->stCsc, sizeof(pstHalDevParam->stCsc));
    pstHalDevParam->bEnable      = TRUE;
    pstHalDevParam->u32Sharpness = pstCvbsParam->u32Sharpness;

    DISP_DBG(DISP_DBG_LEVEL_DRV, "%s %d, En:%d ,Csc:%s, Luma:%d, Hue:%d, Con:%d, Sat:%d, Sharp:%d\n", __FUNCTION__,
             __LINE__, pstHalDevParam->bEnable, PARSING_HAL_CSC_MATRIX(pstHalDevParam->stCsc.eCscMatrix),
             pstHalDevParam->stCsc.u32Luma, pstHalDevParam->stCsc.u32Hue, pstHalDevParam->stCsc.u32Contrast,
             pstHalDevParam->stCsc.u32Saturation, pstHalDevParam->u32Sharpness);
}

static void _DRV_DISP_IF_TransDeviceVgaParamToHal(MI_DISP_VgaParam_t *       pstVgaParam,
                                                  HAL_DISP_ST_DeviceParam_t *pstHalDevParam)
{
    CamOsMemcpy(&pstHalDevParam->stCsc, &pstVgaParam->stCsc, sizeof(pstHalDevParam->stCsc));
    pstHalDevParam->bEnable      = TRUE;
    pstHalDevParam->u32Gain      = pstVgaParam->u32Gain;
    pstHalDevParam->u32Sharpness = pstVgaParam->u32Sharpness;

    DISP_DBG(DISP_DBG_LEVEL_DRV, "%s %d, Csc:%s, Luma:%d, Hue:%d, Con:%d, Sat%d, Gain:%d Sharp:%d\n", __FUNCTION__,
             __LINE__, PARSING_HAL_CSC_MATRIX(pstHalDevParam->stCsc.eCscMatrix), pstHalDevParam->stCsc.u32Luma,
             pstHalDevParam->stCsc.u32Hue, pstHalDevParam->stCsc.u32Contrast, pstHalDevParam->stCsc.u32Saturation,
             pstHalDevParam->u32Gain, pstHalDevParam->u32Sharpness);
}

static void _DRV_DISP_IF_TransDeviceHdmiParamToHal(MI_DISP_HdmiParam_t *      pstHdmiParam,
                                                   HAL_DISP_ST_DeviceParam_t *pstHalDevParam)
{
    CamOsMemcpy(&pstHalDevParam->stCsc, &pstHdmiParam->stCsc, sizeof(pstHalDevParam->stCsc));
    pstHalDevParam->bEnable      = TRUE;
    pstHalDevParam->u32Sharpness = pstHdmiParam->u32Sharpness;

    DISP_DBG(DISP_DBG_LEVEL_DRV, "%s %d, Csc:%s, Luma:%d, Hue:%d, Con:%d, Sat:%d, Sharp:%d\n", __FUNCTION__, __LINE__,
             PARSING_HAL_CSC_MATRIX(pstHalDevParam->stCsc.eCscMatrix), pstHalDevParam->stCsc.u32Luma,
             pstHalDevParam->stCsc.u32Hue, pstHalDevParam->stCsc.u32Contrast, pstHalDevParam->stCsc.u32Saturation,
             pstHalDevParam->u32Sharpness);
}

static void _DRV_DISP_IF_TransDeviceLcdParamToHal(MI_DISP_LcdParam_t *       pstLcdParam,
                                                  HAL_DISP_ST_DeviceParam_t *pstHalDevParam)
{
    CamOsMemcpy(&pstHalDevParam->stCsc, &pstLcdParam->stCsc, sizeof(pstHalDevParam->stCsc));
    pstHalDevParam->bEnable      = TRUE;
    pstHalDevParam->u32Sharpness = pstLcdParam->u32Sharpness;

    DISP_DBG(DISP_DBG_LEVEL_DRV, "%s %d, Csc:%s, Luma:%d, Hue:%d, Con:%d, Sat%d, Sharp:%d\n", __FUNCTION__, __LINE__,
             PARSING_HAL_CSC_MATRIX(pstHalDevParam->stCsc.eCscMatrix), pstHalDevParam->stCsc.u32Luma,
             pstHalDevParam->stCsc.u32Hue, pstHalDevParam->stCsc.u32Contrast, pstHalDevParam->stCsc.u32Saturation,
             pstHalDevParam->u32Sharpness);
}

static void _DRV_DISP_IF_TransDeviceHdmiParamToMI(MI_DISP_HdmiParam_t *      pstHdmiParam,
                                                  HAL_DISP_ST_DeviceParam_t *pstHalDevParam)
{
    CamOsMemcpy(&pstHdmiParam->stCsc, &pstHalDevParam->stCsc, sizeof(pstHdmiParam->stCsc));
    pstHdmiParam->u32Sharpness = pstHalDevParam->u32Sharpness;

    DISP_DBG(DISP_DBG_LEVEL_DRV, "%s %d, Csc:%s, Luma:%d, Hue:%d, Con:%d, Sat:%d, Sharp:%d\n", __FUNCTION__, __LINE__,
             PARSING_HAL_CSC_MATRIX(pstHalDevParam->stCsc.eCscMatrix), pstHalDevParam->stCsc.u32Luma,
             pstHalDevParam->stCsc.u32Hue, pstHalDevParam->stCsc.u32Contrast, pstHalDevParam->stCsc.u32Saturation,
             pstHalDevParam->u32Sharpness);
}

static void _DRV_DISP_IF_TransDeviceCvbsParamToMI(MI_DISP_CvbsParam_t *      pstCvbsParam,
                                                  HAL_DISP_ST_DeviceParam_t *pstHalDevParam)
{
    CamOsMemcpy(&pstCvbsParam->stCsc, &pstHalDevParam->stCsc, sizeof(pstCvbsParam->stCsc));
    pstCvbsParam->u32Sharpness = pstHalDevParam->u32Sharpness;

    DISP_DBG(DISP_DBG_LEVEL_DRV, "%s %d, Csc:%s, Luma:%d, Hue:%d, Con:%d, Sat:%d, Sharp:%d\n", __FUNCTION__, __LINE__,
             PARSING_HAL_CSC_MATRIX(pstHalDevParam->stCsc.eCscMatrix), pstHalDevParam->stCsc.u32Luma,
             pstHalDevParam->stCsc.u32Hue, pstHalDevParam->stCsc.u32Contrast, pstHalDevParam->stCsc.u32Saturation,
             pstHalDevParam->u32Sharpness);
}

static void _DRV_DISP_IF_TransDeviceVgaParamToMI(MI_DISP_VgaParam_t *       pstVgaParam,
                                                 HAL_DISP_ST_DeviceParam_t *pstHalDevParam)
{
    CamOsMemcpy(&pstVgaParam->stCsc, &pstHalDevParam->stCsc, sizeof(pstVgaParam->stCsc));
    pstVgaParam->u32Gain      = pstHalDevParam->u32Gain;
    pstVgaParam->u32Sharpness = pstHalDevParam->u32Sharpness;

    DISP_DBG(DISP_DBG_LEVEL_DRV, "%s %d, Csc:%s, Luma:%d, Hue:%d, Con:%d, Sat%d, Gain:%d Sharp:%d\n", __FUNCTION__,
             __LINE__, PARSING_HAL_CSC_MATRIX(pstHalDevParam->stCsc.eCscMatrix), pstHalDevParam->stCsc.u32Luma,
             pstHalDevParam->stCsc.u32Hue, pstHalDevParam->stCsc.u32Contrast, pstHalDevParam->stCsc.u32Saturation,
             pstHalDevParam->u32Gain, pstHalDevParam->u32Sharpness);
}

static void _DRV_DISP_IF_TransDeviceLcdParamToMI(MI_DISP_LcdParam_t *       pstLcdParam,
                                                 HAL_DISP_ST_DeviceParam_t *pstHalDevParam)
{
    CamOsMemcpy(&pstLcdParam->stCsc, &pstHalDevParam->stCsc, sizeof(pstLcdParam->stCsc));
    pstLcdParam->u32Sharpness = pstHalDevParam->u32Sharpness;

    DISP_DBG(DISP_DBG_LEVEL_DRV, "%s %d, Csc:%s, Luma:%d, Hue:%d, Con:%d, Sat%d, Sharp:%d\n", __FUNCTION__, __LINE__,
             PARSING_HAL_CSC_MATRIX(pstHalDevParam->stCsc.eCscMatrix), pstHalDevParam->stCsc.u32Luma,
             pstHalDevParam->stCsc.u32Hue, pstHalDevParam->stCsc.u32Contrast, pstHalDevParam->stCsc.u32Saturation,
             pstHalDevParam->u32Sharpness);
}

static MI_U8 _DRV_DISP_IF_SetClkOn(void *pCtx)
{
    MI_U8   bRet               = 1;
    MI_U32 *pu32ClkDefaultRate = NULL;
    MI_U8 * pbClkDefaultEn     = NULL;
    MI_U32 *pu32ClkCurRate     = NULL;
    MI_U8 * pbClkCurEn         = NULL;
    MI_U8   i;

    if (E_HAL_DISP_CLK_NUM)
    {
        pu32ClkDefaultRate = CamOsMemAlloc(sizeof(MI_U32) * E_HAL_DISP_CLK_NUM);
        pbClkDefaultEn     = CamOsMemAlloc(sizeof(MI_U8) * E_HAL_DISP_CLK_NUM);

        pu32ClkCurRate = CamOsMemAlloc(sizeof(MI_U32) * E_HAL_DISP_CLK_NUM);
        pbClkCurEn     = CamOsMemAlloc(sizeof(MI_U8) * E_HAL_DISP_CLK_NUM);

        if (pu32ClkDefaultRate == NULL || pbClkDefaultEn == NULL || pu32ClkCurRate == NULL || pbClkCurEn == NULL)
        {
            bRet = 0;
            DISP_ERR("%s %d, Allocate Memory Fail\n", __FUNCTION__, __LINE__);
        }
        else
        {
            for (i = 0; i < E_HAL_DISP_CLK_NUM; i++)
            {
                pbClkDefaultEn[i]     = HAL_DISP_CLK_GET_ON_SETTING(i);
                pu32ClkDefaultRate[i] = HAL_DISP_CLK_GET_RATE_ON_SETTING(i);
            }
            DRV_DISP_IF_GetClk(pCtx, pbClkCurEn, pu32ClkCurRate, E_HAL_DISP_CLK_NUM);

            if (DRV_DISP_OS_SetClkOn(pbClkCurEn, pu32ClkDefaultRate, E_HAL_DISP_CLK_NUM) == 0)
            {
                if (DRV_DISP_IF_SetClk(pCtx, pbClkDefaultEn, pu32ClkDefaultRate, E_HAL_DISP_CLK_NUM) == 0)
                {
                    DISP_ERR("%s %d:: SetClk Fail\n", __FUNCTION__, __LINE__);
                    bRet = 0;
                }
                else
                {
                    bRet = 1;
                }
            }
            else
            {
                bRet = 1;
            }
        }
    }

    if (pu32ClkDefaultRate)
    {
        CamOsMemRelease(pu32ClkDefaultRate);
    }
    if (pu32ClkCurRate)
    {
        CamOsMemRelease(pu32ClkCurRate);
    }

    if (pbClkDefaultEn)
    {
        CamOsMemRelease(pbClkDefaultEn);
    }

    if (pbClkCurEn)
    {
        CamOsMemRelease(pbClkCurEn);
    }

    return bRet;
}

static MI_U8 _DRV_DISP_IF_SetClkOff(void *pCtx)
{
    MI_U8   bRet               = 1;
    MI_U32 *pu32ClkDefaultRate = NULL;
    MI_U8 * pbClkDefaultEn     = NULL;
    MI_U32 *pu32ClkCurRate     = NULL;
    MI_U8 * pbClkCurEn         = NULL;
    MI_U8   i;

    if (E_HAL_DISP_CLK_NUM)
    {
        pu32ClkDefaultRate = CamOsMemAlloc(sizeof(MI_U32) * E_HAL_DISP_CLK_NUM);
        pbClkDefaultEn     = CamOsMemAlloc(sizeof(MI_U8) * E_HAL_DISP_CLK_NUM);

        pu32ClkCurRate = CamOsMemAlloc(sizeof(MI_U32) * E_HAL_DISP_CLK_NUM);
        pbClkCurEn     = CamOsMemAlloc(sizeof(MI_U8) * E_HAL_DISP_CLK_NUM);

        if (pu32ClkDefaultRate == NULL || pbClkDefaultEn == NULL || pu32ClkCurRate == NULL || pbClkCurEn == NULL)
        {
            bRet = 0;
            DISP_ERR("%s %d, Allocate Memory Fail\n", __FUNCTION__, __LINE__);
        }
        else
        {
            for (i = 0; i < E_HAL_DISP_CLK_NUM; i++)
            {
                pbClkDefaultEn[i]     = HAL_DISP_CLK_GET_OFF_SETTING(i);
                pu32ClkDefaultRate[i] = HAL_DISP_CLK_GET_RATE_OFF_SETTING(i);
            }
            DRV_DISP_IF_GetClk(pCtx, pbClkCurEn, pu32ClkCurRate, E_HAL_DISP_CLK_NUM);

            if (DRV_DISP_OS_SetClkOff(pbClkCurEn, pu32ClkDefaultRate, E_HAL_DISP_CLK_NUM) == 0)
            {
                if (DRV_DISP_IF_SetClk(pCtx, pbClkDefaultEn, pu32ClkDefaultRate, E_HAL_DISP_CLK_NUM) == 0)
                {
                    DISP_ERR("%s %d:: SetClk Fail\n", __FUNCTION__, __LINE__);
                    bRet = 0;
                }
                else
                {
                    bRet = 1;
                }
            }
            else
            {
                bRet = 1;
            }
        }
    }

    if (pu32ClkDefaultRate)
    {
        CamOsMemRelease(pu32ClkDefaultRate);
    }
    if (pu32ClkCurRate)
    {
        CamOsMemRelease(pu32ClkCurRate);
    }

    if (pbClkDefaultEn)
    {
        CamOsMemRelease(pbClkDefaultEn);
    }

    if (pbClkCurEn)
    {
        CamOsMemRelease(pbClkCurEn);
    }

    return bRet;
}
static MI_U8 _DRV_DISP_IF_Presuspend(DRV_DISP_CTX_Config_t *pstDispCtxCfg)
{
    MI_U8                            u8bEn = 0;
    MI_U32                           u32Idx;
    MI_DISP_IMPL_MhalRegFlipConfig_t stRegFlipCfg;
    MI_U8                            u8Ret = 1;

    DRV_DISP_DEBUG_Str(pstDispCtxCfg, 0);
    pstDispCtxCfg->enCtxType = E_DRV_DISP_CTX_TYPE_INPUTPORT;
    for (u32Idx = 0; u32Idx < HAL_DISP_INPUTPORT_MAX; u32Idx++)
    {
        pstDispCtxCfg->u32ContainIdx = u32Idx;
        if (pstDispCtxCfg->pstCtxContain->bInputPortContainUsed[u32Idx])
        {
            if (pstDispCtxCfg->pstCtxContain->pstInputPortContain[u32Idx]->bEnInPort)
            {
                DRV_DISP_IF_InputPortEnable(pstDispCtxCfg, u8bEn);
            }
        }
    }
    pstDispCtxCfg->enCtxType = E_DRV_DISP_CTX_TYPE_DEVICE;
    for (u32Idx = 0; u32Idx < HAL_DISP_DEVICE_MAX; u32Idx++)
    {
        pstDispCtxCfg->u32ContainIdx = u32Idx;
        if (pstDispCtxCfg->pstCtxContain->bDevContainUsed[u32Idx])
        {
            if (pstDispCtxCfg->pstCtxContain->pstDevContain[u32Idx]->bEnable)
            {
                stRegFlipCfg.bEnable  = 1;
                stRegFlipCfg.pCmdqInf = NULL;
                DRV_DISP_IF_SetRegFlipConfig(pstDispCtxCfg, &stRegFlipCfg);
            }
        }
    }
    return u8Ret;
}

static MI_U8 _DRV_DISP_IF_ExecuteQueryWithoutCtx(HAL_DISP_ST_QueryConfig_t *pstQueryCfg)
{
    MI_U8                 bRet = 1;
    DRV_DISP_CTX_Config_t stFakeDispCtxCfg;

    CamOsMemset(&stFakeDispCtxCfg, 0, sizeof(DRV_DISP_CTX_Config_t));
    if (HAL_DISP_IF_Query(&stFakeDispCtxCfg, pstQueryCfg))
    {
        if (pstQueryCfg->stOutCfg.enQueryRet == E_HAL_DISP_ST_QUERY_RET_OK
            || pstQueryCfg->stOutCfg.enQueryRet == E_HAL_DISP_ST_QUERY_RET_NONEED)
        {
            if (pstQueryCfg->stOutCfg.pSetFunc)
            {
                pstQueryCfg->stOutCfg.pSetFunc(&stFakeDispCtxCfg, pstQueryCfg->stInCfg.pInCfg);
            }
        }
        else if (pstQueryCfg->stOutCfg.enQueryRet == E_HAL_DISP_ST_QUERY_RET_IMPLICIT_ERR)
        {
            bRet = 0;
        }
        else
        {
            bRet = 0;
            DISP_ERR("%s %d, Query:%s, Ret:%s\n", __FUNCTION__, __LINE__,
                     PARSING_HAL_QUERY_TYPE(pstQueryCfg->stInCfg.enQueryType),
                     PARSING_HAL_QUERY_RET(pstQueryCfg->stOutCfg.enQueryRet));
        }
    }
    else
    {
        bRet = 0;
        DISP_ERR("%s %d, Query Fail\n", __FUNCTION__, __LINE__);
    }
    return bRet;
}

static MI_U8 _DRV_DISP_IF_ExecuteQuery(void *pCtx, HAL_DISP_ST_QueryConfig_t *pstQueryCfg)
{
    MI_U8 bRet = 1;

    if (HAL_DISP_IF_Query(pCtx, pstQueryCfg))
    {
        if (pstQueryCfg->stOutCfg.enQueryRet == E_HAL_DISP_ST_QUERY_RET_OK
            || pstQueryCfg->stOutCfg.enQueryRet == E_HAL_DISP_ST_QUERY_RET_NONEED)
        {
            if (pstQueryCfg->stOutCfg.pSetFunc)
            {
                pstQueryCfg->stOutCfg.pSetFunc(pCtx, pstQueryCfg->stInCfg.pInCfg);
            }
        }
        else if (pstQueryCfg->stOutCfg.enQueryRet == E_HAL_DISP_ST_QUERY_RET_IMPLICIT_ERR)
        {
            bRet = 0;
        }
        else
        {
            bRet = 0;
            DISP_ERR("%s %d, Query:%s, Ret:%s\n", __FUNCTION__, __LINE__,
                     PARSING_HAL_QUERY_TYPE(pstQueryCfg->stInCfg.enQueryType),
                     PARSING_HAL_QUERY_RET(pstQueryCfg->stOutCfg.enQueryRet));
        }
    }
    else
    {
        bRet = 0;
        DISP_ERR("%s %d, Query Fail\n", __FUNCTION__, __LINE__);
    }
    return bRet;
}

//-------------------------------------------------------------------------------------------------
//  Global Functions
//-------------------------------------------------------------------------------------------------
MI_U8 DRV_DISP_IF_GetClk(void *pDevCtx, MI_U8 *pbEn, MI_U32 *pu32ClkRate, MI_U32 u32ClkNum)
{
    MI_U8                     bRet = 1;
    HAL_DISP_ST_QueryConfig_t stQueryCfg;
    HAL_DISP_ST_ClkConfig_t   stHalClkCfg;

    HAL_DISP_IF_Init();

    CamOsMemset(&stQueryCfg, 0, sizeof(HAL_DISP_ST_QueryConfig_t));
    CamOsMemset(&stHalClkCfg, 0, sizeof(HAL_DISP_ST_ClkConfig_t));
    stHalClkCfg.u32Num = u32ClkNum;

    stQueryCfg.stInCfg.enQueryType = E_HAL_DISP_ST_QUERY_CLK_GET;
    stQueryCfg.stInCfg.u32CfgSize  = sizeof(stHalClkCfg);
    stQueryCfg.stInCfg.pInCfg      = &stHalClkCfg;

    if (_DRV_DISP_IF_ExecuteQuery(pDevCtx, &stQueryCfg))
    {
        if (stHalClkCfg.u32Num == u32ClkNum)
        {
            CamOsMemcpy(pu32ClkRate, stHalClkCfg.u32Rate, sizeof(stHalClkCfg.u32Rate));
            CamOsMemcpy(pbEn, stHalClkCfg.bEn, sizeof(stHalClkCfg.bEn));
            bRet = 1;
        }
    }
    else
    {
        bRet = FALSE;
        DISP_ERR("%s %d, Get Clk Fail\n", __FUNCTION__, __LINE__);
    }
    return bRet;
}

MI_U8 DRV_DISP_IF_SetClk(void *pDevCtx, MI_U8 *pbEn, MI_U32 *pu32ClkRate, MI_U32 u32ClkNum)
{
    MI_U8                     bRet = 1;
    HAL_DISP_ST_QueryConfig_t stQueryCfg;
    HAL_DISP_ST_ClkConfig_t   stHalClkCfg;

    if (sizeof(stHalClkCfg.u32Rate) != sizeof(MI_U32) * u32ClkNum
        || sizeof(stHalClkCfg.bEn) != sizeof(MI_U8) * u32ClkNum)
    {
        bRet = 0;
        DISP_ERR("%s %d, Clk Num is not correct", __FUNCTION__, __LINE__);
    }
    else
    {
        HAL_DISP_IF_Init();

        CamOsMemset(&stQueryCfg, 0, sizeof(HAL_DISP_ST_QueryConfig_t));
        stQueryCfg.stInCfg.enQueryType = E_HAL_DISP_ST_QUERY_CLK_SET;
        stQueryCfg.stInCfg.u32CfgSize  = sizeof(HAL_DISP_ST_ClkConfig_t);
        stQueryCfg.stInCfg.pInCfg      = &stHalClkCfg;

        stHalClkCfg.u32Num = u32ClkNum;
        CamOsMemcpy(stHalClkCfg.u32Rate, pu32ClkRate, sizeof(MI_U32) * u32ClkNum);
        CamOsMemcpy(stHalClkCfg.bEn, pbEn, sizeof(MI_U8) * u32ClkNum);

        bRet = _DRV_DISP_IF_ExecuteQuery(pDevCtx, &stQueryCfg);
    }
    return bRet;
}

MI_U8 DRV_DISP_IF_InitPanelConfig(MI_DISP_IMPL_MhalPanelConfig_t *pstPanelConfig, MI_U8 u8Size)
{
    MI_U8                             i;
    MI_U8                             bRet = TRUE;
    MI_U32                            u32Mod, u32HttVtt, u32DClkhz;
    MI_U32                            u32Tmp;
    MI_U16                            u16FrontPorch;
    MI_U16                            u16FpsDot             = 0;
    MI_U16                            u16Fps                = 1;
    HAL_DISP_ST_DeviceTimingConfig_t *pstHalDeviceTimingCfg = NULL;
    DRV_DISP_IF_TimingData_t *        pstTimingData         = NULL;

    if (u8Size > E_MI_DISP_OUTPUT_MAX)
    {
        bRet = FALSE;
        DISP_ERR("%s %d, The size (%d) is bigger than %d\n", __FUNCTION__, __LINE__, u8Size, E_MI_DISP_OUTPUT_MAX);
    }
    else
    {
        DISP_DBG(DISP_DBG_LEVEL_DRV, "%s %d, u8Size:%d\n", __FUNCTION__, __LINE__, u8Size);

        if (g_stDispDevTimingCfg.pstData)
        {
            CamOsMemRelease(g_stDispDevTimingCfg.pstData);
            g_stDispDevTimingCfg.pstData     = NULL;
            g_stDispDevTimingCfg.u32DataSize = 0;
        }

        g_stDispDevTimingCfg.pstData     = CamOsMemAlloc(sizeof(DRV_DISP_IF_TimingData_t) * u8Size);
        g_stDispDevTimingCfg.u32DataSize = u8Size;

        if (g_stDispDevTimingCfg.pstData == NULL)
        {
            bRet = FALSE;
            DISP_ERR("%s %d, Allocate Memory Fail\n", __FUNCTION__, __LINE__);
        }
        else
        {
            for (i = 0; i < u8Size; i++)
            {
                if (!pstPanelConfig[i].bValid)
                {
                    continue;
                }
                pstTimingData                  = &g_stDispDevTimingCfg.pstData[i];
                pstTimingData->bValid          = pstPanelConfig[i].bValid;
                pstTimingData->enDevTimingType = pstPanelConfig[i].stPnlUniParamCfg.eDisplayTiming;

                pstHalDeviceTimingCfg                = &pstTimingData->stDevTiming;
                pstHalDeviceTimingCfg->u16HsyncWidth = pstPanelConfig[i].stPnlUniParamCfg.stTgnTimingInfo.u16HSyncWidth;
                pstHalDeviceTimingCfg->u16HsyncBackPorch =
                    pstPanelConfig[i].stPnlUniParamCfg.stTgnTimingInfo.u16HSyncBackPorch;
                pstHalDeviceTimingCfg->u16Hstart  = pstPanelConfig[i].stPnlUniParamCfg.stTgnTimingInfo.u16HStart;
                pstHalDeviceTimingCfg->u16Hactive = pstPanelConfig[i].stPnlUniParamCfg.stTgnTimingInfo.u16HActive;
                pstHalDeviceTimingCfg->u16Htotal  = pstPanelConfig[i].stPnlUniParamCfg.stTgnTimingInfo.u16HTotal;

                pstHalDeviceTimingCfg->u16VsyncWidth = pstPanelConfig[i].stPnlUniParamCfg.stTgnTimingInfo.u16VSyncWidth;
                pstHalDeviceTimingCfg->u16VsyncBackPorch =
                    pstPanelConfig[i].stPnlUniParamCfg.stTgnTimingInfo.u16VSyncBackPorch;
                pstHalDeviceTimingCfg->u16Vstart  = pstPanelConfig[i].stPnlUniParamCfg.stTgnTimingInfo.u16VStart;
                pstHalDeviceTimingCfg->u16Vactive = pstPanelConfig[i].stPnlUniParamCfg.stTgnTimingInfo.u16VActive;
                pstHalDeviceTimingCfg->u16Vtotal  = pstPanelConfig[i].stPnlUniParamCfg.stTgnTimingInfo.u16VTotal;
                pstHalDeviceTimingCfg->u32Dclk    = pstPanelConfig[i].stPnlUniParamCfg.stTgnTimingInfo.u32Dclk;

                u32HttVtt = ((MI_U32)pstPanelConfig[i].stPnlUniParamCfg.stTgnTimingInfo.u16HTotal
                             * (MI_U32)pstPanelConfig[i].stPnlUniParamCfg.stTgnTimingInfo.u16VTotal);
                if (u32HttVtt == 0)
                {
                    DISP_ERR("%s %d, Htt or Vtt is Empty\n", __FUNCTION__, __LINE__);
                    continue;
                }
                if (pstTimingData->enDevTimingType == E_MI_DISP_OUTPUT_USER)
                {
                    u32DClkhz = (pstPanelConfig[i].stPnlUniParamCfg.stTgnTimingInfo.u32Dclk);
                    if (u32DClkhz < (u32HttVtt / 100))
                    {
                        DISP_DBG(DISP_DBG_LEVEL_DRV, "%s %d, The Dclk (%d) is Too small than vtthtt %d\n", __FUNCTION__,
                                 __LINE__, u32DClkhz, u32HttVtt);
                        u32DClkhz *= HAL_DISP_1M;
                    }

                    u32Mod = u32DClkhz % u32HttVtt;

                    if (u32Mod > (u32HttVtt / 2))
                    {
                        pstHalDeviceTimingCfg->u16Fps = (u32DClkhz + u32HttVtt - 1) / u32HttVtt;
                    }
                    else
                    {
                        pstHalDeviceTimingCfg->u16Fps = u32DClkhz / u32HttVtt;
                    }
                }
                else
                {
                    u32DClkhz = u32HttVtt * u16Fps;
                    if (u16FpsDot)
                    {
                        u32DClkhz += ((u32HttVtt * u16FpsDot) / 100);
                        u16Fps++;
                    }
                    pstHalDeviceTimingCfg->u16Fps  = u16Fps;
                    pstHalDeviceTimingCfg->u32Dclk = u32DClkhz;
                }

                pstHalDeviceTimingCfg->u16SscSpan =
                    pstPanelConfig[i].stPnlUniParamCfg.stTgnSscInfo.u16SpreadSpectrumSpan;
                pstHalDeviceTimingCfg->u16SscStep =
                    pstPanelConfig[i].stPnlUniParamCfg.stTgnSscInfo.u16SpreadSpectrumStep;

                u32Tmp = 1000000UL / (MI_U32)pstHalDeviceTimingCfg->u16Fps;

                u16FrontPorch = (pstHalDeviceTimingCfg->u16Vtotal - pstHalDeviceTimingCfg->u16VsyncWidth
                                 - pstHalDeviceTimingCfg->u16VsyncBackPorch - pstHalDeviceTimingCfg->u16Vactive);

                pstHalDeviceTimingCfg->u32VSyncPeriod =
                    u32Tmp * (MI_U32)pstHalDeviceTimingCfg->u16VsyncWidth / (MI_U32)pstHalDeviceTimingCfg->u16Vtotal;
                pstHalDeviceTimingCfg->u32VBackPorchPeriod = u32Tmp * (MI_U32)pstHalDeviceTimingCfg->u16VsyncBackPorch
                                                             / (MI_U32)pstHalDeviceTimingCfg->u16Vtotal;
                pstHalDeviceTimingCfg->u32VActivePeriod =
                    u32Tmp * (MI_U32)pstHalDeviceTimingCfg->u16Vactive / (MI_U32)pstHalDeviceTimingCfg->u16Vtotal;
                pstHalDeviceTimingCfg->u32VFrontPorchPeriod =
                    u32Tmp * (MI_U32)u16FrontPorch / (MI_U32)pstHalDeviceTimingCfg->u16Vtotal;
            }
        }
    }
    return bRet;
}

MI_U8 DRV_DISP_IF_SetRegAccessConfig(void *pDevCtx, MI_DISP_IMPL_MhalRegAccessConfig_t *pstRegAccessCfg)
{
    MI_U8                     bRet = 1;
    HAL_DISP_ST_QueryConfig_t stQueryCfg;

    if (DRV_DISP_CTX_SetCurCtx((DRV_DISP_CTX_Config_t *)pDevCtx, ((DRV_DISP_CTX_Config_t *)pDevCtx)->u32ContainIdx)
        == FALSE)
    {
        bRet = FALSE;
    }
    else
    {
        CamOsMemset(&stQueryCfg, 0, sizeof(HAL_DISP_ST_QueryConfig_t));

        stQueryCfg.stInCfg.enQueryType = E_HAL_DISP_ST_QUERY_REG_ACCESS;
        stQueryCfg.stInCfg.u32CfgSize  = sizeof(MI_DISP_IMPL_MhalRegAccessConfig_t);
        stQueryCfg.stInCfg.pInCfg      = pstRegAccessCfg;

        bRet = _DRV_DISP_IF_ExecuteQuery(pDevCtx, &stQueryCfg);
    }
    return bRet;
}

MI_U8 DRV_DISP_IF_SetRegFlipConfig(void *pDevCtx, MI_DISP_IMPL_MhalRegFlipConfig_t *pstRegFlipCfg)
{
    MI_U8                     bRet = 1;
    HAL_DISP_ST_QueryConfig_t stQueryCfg;

    CamOsMemset(&stQueryCfg, 0, sizeof(HAL_DISP_ST_QueryConfig_t));

    stQueryCfg.stInCfg.enQueryType = E_HAL_DISP_ST_QUERY_REG_FLIP;
    stQueryCfg.stInCfg.u32CfgSize  = sizeof(MI_DISP_IMPL_MhalRegFlipConfig_t);
    stQueryCfg.stInCfg.pInCfg      = pstRegFlipCfg;

    bRet = _DRV_DISP_IF_ExecuteQuery(pDevCtx, &stQueryCfg);
    return bRet;
}

MI_U8 DRV_DISP_IF_SetRegWaitDoneConfig(void *pDevCtx, MI_DISP_IMPL_MhalRegWaitDoneConfig_t *pstRegWaitDoneCfg)
{
    MI_U8                     bRet = 1;
    HAL_DISP_ST_QueryConfig_t stQueryCfg;

    if (DRV_DISP_CTX_SetCurCtx((DRV_DISP_CTX_Config_t *)pDevCtx, ((DRV_DISP_CTX_Config_t *)pDevCtx)->u32ContainIdx)
        == FALSE)
    {
        bRet = FALSE;
    }
    else
    {
        CamOsMemset(&stQueryCfg, 0, sizeof(HAL_DISP_ST_QueryConfig_t));

        stQueryCfg.stInCfg.enQueryType = E_HAL_DISP_ST_QUERY_REG_WAITDONE;
        stQueryCfg.stInCfg.u32CfgSize  = sizeof(MI_DISP_IMPL_MhalRegWaitDoneConfig_t);
        stQueryCfg.stInCfg.pInCfg      = pstRegWaitDoneCfg;

        bRet = _DRV_DISP_IF_ExecuteQuery(pDevCtx, &stQueryCfg);
    }
    return bRet;
}

MI_U8 DRV_DISP_IF_SetCmdqInterfaceConfig(MI_U32 u32DevId, MI_DISP_IMPL_MhalCmdqInfConfig_t *pstCmdqInfCfg)
{
    MI_U8                       bRet = 1;
    HAL_DISP_ST_QueryConfig_t   stQueryCfg;
    HAL_DISP_ST_CmdqInfConfig_t stHalCmdqIfCfg;
    DRV_DISP_CTX_Config_t       stDispCtxCfg;

    HAL_DISP_IF_Init();

    CamOsMemset(&stQueryCfg, 0, sizeof(HAL_DISP_ST_QueryConfig_t));

    stQueryCfg.stInCfg.enQueryType = E_HAL_DISP_ST_QUERY_CMDQINF;
    stQueryCfg.stInCfg.u32CfgSize  = sizeof(HAL_DISP_ST_CmdqInfConfig_t);
    stQueryCfg.stInCfg.pInCfg      = (void *)&stHalCmdqIfCfg;

    stHalCmdqIfCfg.pCmdqInf = (void *)pstCmdqInfCfg->pCmdqInf;
    stHalCmdqIfCfg.u32DevId = u32DevId;

    bRet = _DRV_DISP_IF_ExecuteQuery(&stDispCtxCfg, &stQueryCfg);
    return bRet;
}

MI_U8 DRV_DISP_IF_DeviceCreateInstance(MI_DISP_IMPL_MhalMemAllocConfig_t *pstAlloc, MI_U32 u32DeviceId, void **pDevCtx)
{
    MI_U8                      bRet = TRUE;
    DRV_DISP_CTX_AllocConfig_t stCtxAllocCfg;
    HAL_DISP_ST_QueryConfig_t  stQueryCfg;
    DRV_DISP_CTX_Config_t *    pstDispCtx = NULL;

    DRV_DISP_CTX_Init();

    stCtxAllocCfg.enType             = E_DRV_DISP_CTX_TYPE_DEVICE;
    stCtxAllocCfg.u32DevId           = u32DeviceId;
    stCtxAllocCfg.pstBindCtx         = NULL;
    stCtxAllocCfg.stMemAllcCfg.alloc = pstAlloc->alloc;
    stCtxAllocCfg.stMemAllcCfg.free  = pstAlloc->free;

    if (DRV_DISP_CTX_Allocate(&stCtxAllocCfg, &pstDispCtx) == FALSE)
    {
        bRet     = 0;
        *pDevCtx = NULL;
        DISP_ERR("%s %d, CreateInstance Fail\n", __FUNCTION__, __LINE__);
    }
    else
    {
        *pDevCtx = (void *)pstDispCtx;

        if (_DRV_DISP_IF_SetClkOn((void *)pstDispCtx))
        {
            CamOsMemset(&stQueryCfg, 0, sizeof(HAL_DISP_ST_QueryConfig_t));
            stQueryCfg.stInCfg.enQueryType = E_HAL_DISP_ST_QUERY_DEVICE_INIT;
            stQueryCfg.stInCfg.pInCfg      = NULL;
            stQueryCfg.stInCfg.u32CfgSize  = 0;

            if (_DRV_DISP_IF_ExecuteQuery(pstDispCtx, &stQueryCfg))
            {
                if (DRV_DISP_CTX_SetCurCtx(pstDispCtx, pstDispCtx->u32ContainIdx) == FALSE)
                {
                    bRet = 0;
                }
                else
                {
                    DRV_DISP_IRQ_CreateInternalIsr((void *)pstDispCtx);
                }
            }
            else
            {
                DISP_ERR("%s %d:: Device Init Fail\n", __FUNCTION__, __LINE__);
                bRet = 0;
            }
        }
        else
        {
            bRet = 0;
            DISP_ERR("%s %d, Set Clk On Fail\n", __FUNCTION__, __LINE__);
        }
    }

    return bRet;
}

MI_U8 DRV_DISP_IF_DeviceDestroyInstance(void *pDevCtx)
{
    MI_U8                     bRet = TRUE;
    HAL_DISP_ST_QueryConfig_t stQueryCfg;
    DRV_DISP_CTX_Config_t *   pstDispCtx = (DRV_DISP_CTX_Config_t *)pDevCtx;

    if (DRV_DISP_CTX_SetCurCtx(pstDispCtx, pstDispCtx->u32ContainIdx) == FALSE)
    {
        bRet = FALSE;
    }
    else
    {
        if (DRV_DISP_IRQ_DestroyInternalIsr((void *)pstDispCtx))
        {
            if (DRV_DISP_CTX_IsLastDeviceCtx(pstDispCtx))
            {
                CamOsMemset(&stQueryCfg, 0, sizeof(HAL_DISP_ST_QueryConfig_t));
                stQueryCfg.stInCfg.enQueryType = E_HAL_DISP_ST_QUERY_DEVICE_DEINIT;
                stQueryCfg.stInCfg.pInCfg      = NULL;
                stQueryCfg.stInCfg.u32CfgSize  = 0;

                if (_DRV_DISP_IF_ExecuteQuery(pstDispCtx, &stQueryCfg))
                {
                    if (!_DRV_DISP_IF_SetClkOff(pDevCtx))
                    {
                        bRet = 0;
                        DISP_ERR("%s %d, Set Clk Off Fail\n", __FUNCTION__, __LINE__);
                    }
                }
            }
            if (!DRV_DISP_CTX_Free(pstDispCtx))
            {
                bRet = 0;
                DISP_ERR("%s %d, DestroyInstance Fail\n", __FUNCTION__, __LINE__);
            }
            if (DRV_DISP_CTX_IsAllFree())
            {
                DRV_DISP_CTX_DeInit();
            }
        }
    }
    return bRet;
}

MI_U8 DRV_DISP_IF_DeviceEnable(void *pDevCtx, MI_U8 bEnable)
{
    MI_U8                     bRet = 1;
    HAL_DISP_ST_QueryConfig_t stQueryCfg;

    if (DRV_DISP_CTX_SetCurCtx((DRV_DISP_CTX_Config_t *)pDevCtx, ((DRV_DISP_CTX_Config_t *)pDevCtx)->u32ContainIdx)
        == FALSE)
    {
        bRet = FALSE;
    }
    else
    {
        CamOsMemset(&stQueryCfg, 0, sizeof(HAL_DISP_ST_QueryConfig_t));

        stQueryCfg.stInCfg.enQueryType = E_HAL_DISP_ST_QUERY_DEVICE_ENABLE;
        stQueryCfg.stInCfg.u32CfgSize  = sizeof(MI_U8);
        stQueryCfg.stInCfg.pInCfg      = (void *)&bEnable;

        bRet = _DRV_DISP_IF_ExecuteQuery(pDevCtx, &stQueryCfg);
    }
    return bRet;
}

MI_U8 DRV_DISP_IF_DeviceSetBackGroundColor(void *pDevCtx, MI_U32 u32BgColor)
{
    MI_U8                     bRet = 1;
    HAL_DISP_ST_QueryConfig_t stQueryCfg;

    if (DRV_DISP_CTX_SetCurCtx((DRV_DISP_CTX_Config_t *)pDevCtx, ((DRV_DISP_CTX_Config_t *)pDevCtx)->u32ContainIdx)
        == FALSE)
    {
        bRet = FALSE;
    }
    else
    {
        CamOsMemset(&stQueryCfg, 0, sizeof(HAL_DISP_ST_QueryConfig_t));

        stQueryCfg.stInCfg.enQueryType = E_HAL_DISP_ST_QUERY_DEVICE_BACKGROUND_COLOR;
        stQueryCfg.stInCfg.u32CfgSize  = sizeof(MI_U32);
        stQueryCfg.stInCfg.pInCfg      = (void *)&u32BgColor;

        bRet = _DRV_DISP_IF_ExecuteQuery(pDevCtx, &stQueryCfg);
    }
    return bRet;
}

MI_U8 DRV_DISP_IF_DeviceAddOutInterface(void *pDevCtx, MI_U32 u32Interface)
{
    MI_U8                     bRet = 1;
    HAL_DISP_ST_QueryConfig_t stQueryCfg;

    if (DRV_DISP_CTX_SetCurCtx((DRV_DISP_CTX_Config_t *)pDevCtx, ((DRV_DISP_CTX_Config_t *)pDevCtx)->u32ContainIdx)
        == FALSE)
    {
        bRet = FALSE;
    }
    else
    {
        CamOsMemset(&stQueryCfg, 0, sizeof(HAL_DISP_ST_QueryConfig_t));

        stQueryCfg.stInCfg.enQueryType = E_HAL_DISP_ST_QUERY_DEVICE_INTERFACE;
        stQueryCfg.stInCfg.u32CfgSize  = sizeof(MI_U32);
        stQueryCfg.stInCfg.pInCfg      = (void *)&u32Interface;

        bRet = _DRV_DISP_IF_ExecuteQuery(pDevCtx, &stQueryCfg);
    }
    return bRet;
}

MI_U8 DRV_DISP_IF_DeviceSetOutputTiming(void *pDevCtx, MI_U32 u32Interface,
                                        MI_DISP_IMPL_MhalDeviceTimingInfo_t *pstTimingInfo)
{
    MI_U8                          bRet = 1;
    HAL_DISP_ST_QueryConfig_t      stQueryCfg;
    HAL_DISP_ST_DeviceTimingInfo_t stHalTimingIfo;

    if (DRV_DISP_CTX_SetCurCtx((DRV_DISP_CTX_Config_t *)pDevCtx, ((DRV_DISP_CTX_Config_t *)pDevCtx)->u32ContainIdx)
        == FALSE)
    {
        bRet = FALSE;
    }
    else
    {
        CamOsMemset(&stQueryCfg, 0, sizeof(HAL_DISP_ST_QueryConfig_t));

        stQueryCfg.stInCfg.enQueryType = E_HAL_DISP_ST_QUERY_DEVICE_OUTPUTTIMING;
        stQueryCfg.stInCfg.u32CfgSize  = sizeof(HAL_DISP_ST_DeviceTimingInfo_t);
        stQueryCfg.stInCfg.pInCfg      = (void *)&stHalTimingIfo;

        if (_DRV_DISP_IF_TransDeviceOutpuTimingInfoToHal(pstTimingInfo, &stHalTimingIfo))
        {
            bRet = _DRV_DISP_IF_ExecuteQuery(pDevCtx, &stQueryCfg);
        }
        else
        {
            bRet = 0;
        }
    }

    return bRet;
}

MI_U8 DRV_DISP_IF_DeviceSetColortemp(void *pDevCtx, MI_DISP_ColorTemperature_t *pstcolorTemp)
{
    MI_U8                     bRet = 1;
    HAL_DISP_ST_QueryConfig_t stQueryCfg;

    if (DRV_DISP_CTX_SetCurCtx((DRV_DISP_CTX_Config_t *)pDevCtx, ((DRV_DISP_CTX_Config_t *)pDevCtx)->u32ContainIdx)
        == FALSE)
    {
        bRet = FALSE;
    }
    else
    {
        CamOsMemset(&stQueryCfg, 0, sizeof(HAL_DISP_ST_QueryConfig_t));

        stQueryCfg.stInCfg.enQueryType = E_HAL_DISP_ST_QUERY_DEVICE_COLORTEMP;
        stQueryCfg.stInCfg.u32CfgSize  = sizeof(MI_DISP_ColorTemperature_t);
        stQueryCfg.stInCfg.pInCfg      = pstcolorTemp;

        bRet = _DRV_DISP_IF_ExecuteQuery(pDevCtx, &stQueryCfg);
    }
    return bRet;
}

MI_U8 DRV_DISP_IF_DeviceSetCvbsParam(void *pDevCtx, MI_DISP_CvbsParam_t *pstCvbsInfo)
{
    MI_U8                     bRet = 1;
    HAL_DISP_ST_QueryConfig_t stQueryCfg;
    HAL_DISP_ST_DeviceParam_t stDeviceParam;

    if (DRV_DISP_CTX_SetCurCtx((DRV_DISP_CTX_Config_t *)pDevCtx, ((DRV_DISP_CTX_Config_t *)pDevCtx)->u32ContainIdx)
        == FALSE)
    {
        bRet = FALSE;
    }
    else
    {
        CamOsMemset(&stQueryCfg, 0, sizeof(HAL_DISP_ST_QueryConfig_t));
        stQueryCfg.stInCfg.enQueryType = E_HAL_DISP_ST_QUERY_DEVICE_PARAM;
        stQueryCfg.stInCfg.u32CfgSize  = sizeof(HAL_DISP_ST_DeviceParam_t);
        stQueryCfg.stInCfg.pInCfg      = (void *)&stDeviceParam;

        CamOsMemset(&stDeviceParam, 0, sizeof(HAL_DISP_ST_DeviceParam_t));
        _DRV_DISP_IF_TransDeviceCvbsParamToHal(pstCvbsInfo, &stDeviceParam);

        bRet = _DRV_DISP_IF_ExecuteQuery(pDevCtx, &stQueryCfg);
    }
    return bRet;
}

MI_U8 DRV_DISP_IF_DeviceGetCvbsParam(void *pDevCtx, MI_DISP_CvbsParam_t *pstCvbsInfo)
{
    MI_U8                     bRet = 1;
    HAL_DISP_ST_QueryConfig_t stQueryCfg;
    HAL_DISP_ST_DeviceParam_t stDeviceParam;

    if (DRV_DISP_CTX_SetCurCtx((DRV_DISP_CTX_Config_t *)pDevCtx, ((DRV_DISP_CTX_Config_t *)pDevCtx)->u32ContainIdx)
        == FALSE)
    {
        bRet = FALSE;
    }
    else
    {
        CamOsMemset(&stQueryCfg, 0, sizeof(HAL_DISP_ST_QueryConfig_t));
        CamOsMemset(&stDeviceParam, 0, sizeof(HAL_DISP_ST_DeviceParam_t));

        stQueryCfg.stInCfg.enQueryType = E_HAL_DISP_ST_QUERY_DEVICE_PARAM_GET;
        stQueryCfg.stInCfg.u32CfgSize  = sizeof(HAL_DISP_ST_DeviceParam_t);
        stQueryCfg.stInCfg.pInCfg      = (void *)&stDeviceParam;

        if (_DRV_DISP_IF_ExecuteQuery(pDevCtx, &stQueryCfg))
        {
            _DRV_DISP_IF_TransDeviceCvbsParamToMI(pstCvbsInfo, &stDeviceParam);
        }
        else
        {
            bRet = 0;
        }
    }

    return bRet;
}

MI_U8 DRV_DISP_IF_DeviceSetHdmiParam(void *pDevCtx, MI_DISP_HdmiParam_t *pstHdmiInfo)
{
    MI_U8                     bRet = 1;
    HAL_DISP_ST_QueryConfig_t stQueryCfg;
    HAL_DISP_ST_DeviceParam_t stDeviceParam;

    if (DRV_DISP_CTX_SetCurCtx((DRV_DISP_CTX_Config_t *)pDevCtx, ((DRV_DISP_CTX_Config_t *)pDevCtx)->u32ContainIdx)
        == FALSE)
    {
        bRet = FALSE;
    }
    else
    {
        CamOsMemset(&stQueryCfg, 0, sizeof(HAL_DISP_ST_QueryConfig_t));
        CamOsMemset(&stDeviceParam, 0, sizeof(HAL_DISP_ST_DeviceParam_t));

        stQueryCfg.stInCfg.enQueryType = E_HAL_DISP_ST_QUERY_DEVICE_PARAM;
        stQueryCfg.stInCfg.u32CfgSize  = sizeof(HAL_DISP_ST_DeviceParam_t);
        stQueryCfg.stInCfg.pInCfg      = (void *)&stDeviceParam;

        _DRV_DISP_IF_TransDeviceHdmiParamToHal(pstHdmiInfo, &stDeviceParam);

        bRet = _DRV_DISP_IF_ExecuteQuery(pDevCtx, &stQueryCfg);
    }
    return bRet;
}

MI_U8 DRV_DISP_IF_DeviceGetHdmiParam(void *pDevCtx, MI_DISP_HdmiParam_t *pstHdmiInfo)
{
    MI_U8                     bRet = 1;
    HAL_DISP_ST_QueryConfig_t stQueryCfg;
    HAL_DISP_ST_DeviceParam_t stDeviceParam;

    if (DRV_DISP_CTX_SetCurCtx((DRV_DISP_CTX_Config_t *)pDevCtx, ((DRV_DISP_CTX_Config_t *)pDevCtx)->u32ContainIdx)
        == FALSE)
    {
        bRet = FALSE;
    }
    else
    {
        CamOsMemset(&stQueryCfg, 0, sizeof(HAL_DISP_ST_QueryConfig_t));
        CamOsMemset(&stDeviceParam, 0, sizeof(HAL_DISP_ST_DeviceParam_t));

        stQueryCfg.stInCfg.enQueryType = E_HAL_DISP_ST_QUERY_DEVICE_PARAM_GET;
        stQueryCfg.stInCfg.u32CfgSize  = sizeof(HAL_DISP_ST_DeviceParam_t);
        stQueryCfg.stInCfg.pInCfg      = (void *)&stDeviceParam;

        if (_DRV_DISP_IF_ExecuteQuery(pDevCtx, &stQueryCfg))
        {
            _DRV_DISP_IF_TransDeviceHdmiParamToMI(pstHdmiInfo, &stDeviceParam);
        }
        else
        {
            bRet = 0;
        }
    }
    return bRet;
}

MI_U8 DRV_DISP_IF_DeviceSetLcdParam(void *pDevCtx, MI_DISP_LcdParam_t *pstLcdInfo)
{
    MI_U8                     bRet = 1;
    HAL_DISP_ST_QueryConfig_t stQueryCfg;
    HAL_DISP_ST_DeviceParam_t stDeviceParam;

    if (DRV_DISP_CTX_SetCurCtx((DRV_DISP_CTX_Config_t *)pDevCtx, ((DRV_DISP_CTX_Config_t *)pDevCtx)->u32ContainIdx)
        == FALSE)
    {
        bRet = FALSE;
    }
    else
    {
        CamOsMemset(&stQueryCfg, 0, sizeof(HAL_DISP_ST_QueryConfig_t));
        CamOsMemset(&stDeviceParam, 0, sizeof(HAL_DISP_ST_DeviceParam_t));

        stQueryCfg.stInCfg.enQueryType = E_HAL_DISP_ST_QUERY_DEVICE_PARAM;
        stQueryCfg.stInCfg.u32CfgSize  = sizeof(HAL_DISP_ST_DeviceParam_t);
        stQueryCfg.stInCfg.pInCfg      = (void *)&stDeviceParam;

        _DRV_DISP_IF_TransDeviceLcdParamToHal(pstLcdInfo, &stDeviceParam);

        bRet = _DRV_DISP_IF_ExecuteQuery(pDevCtx, &stQueryCfg);
    }
    return bRet;
}

MI_U8 DRV_DISP_IF_DeviceGetLcdParam(void *pDevCtx, MI_DISP_LcdParam_t *pstLcdInfo)
{
    MI_U8                     bRet = 1;
    HAL_DISP_ST_QueryConfig_t stQueryCfg;
    HAL_DISP_ST_DeviceParam_t stDeviceParam;

    if (DRV_DISP_CTX_SetCurCtx((DRV_DISP_CTX_Config_t *)pDevCtx, ((DRV_DISP_CTX_Config_t *)pDevCtx)->u32ContainIdx)
        == FALSE)
    {
        bRet = FALSE;
    }
    else
    {
        CamOsMemset(&stQueryCfg, 0, sizeof(HAL_DISP_ST_QueryConfig_t));
        CamOsMemset(&stDeviceParam, 0, sizeof(HAL_DISP_ST_DeviceParam_t));

        stQueryCfg.stInCfg.enQueryType = E_HAL_DISP_ST_QUERY_DEVICE_PARAM_GET;
        stQueryCfg.stInCfg.u32CfgSize  = sizeof(HAL_DISP_ST_DeviceParam_t);
        stQueryCfg.stInCfg.pInCfg      = (void *)&stDeviceParam;

        if (_DRV_DISP_IF_ExecuteQuery(pDevCtx, &stQueryCfg))
        {
            _DRV_DISP_IF_TransDeviceLcdParamToMI(pstLcdInfo, &stDeviceParam);
        }
        else
        {
            bRet = 0;
        }
    }
    return bRet;
}

MI_U8 DRV_DISP_IF_DeviceSetVgaParam(void *pDevCtx, MI_DISP_VgaParam_t *pstVgaInfo)
{
    MI_U8                     bRet = 1;
    HAL_DISP_ST_QueryConfig_t stQueryCfg;
    HAL_DISP_ST_DeviceParam_t stDeviceParam;

    if (DRV_DISP_CTX_SetCurCtx((DRV_DISP_CTX_Config_t *)pDevCtx, ((DRV_DISP_CTX_Config_t *)pDevCtx)->u32ContainIdx)
        == FALSE)
    {
        bRet = FALSE;
    }
    else
    {
        CamOsMemset(&stQueryCfg, 0, sizeof(HAL_DISP_ST_QueryConfig_t));
        CamOsMemset(&stDeviceParam, 0, sizeof(HAL_DISP_ST_DeviceParam_t));

        stQueryCfg.stInCfg.enQueryType = E_HAL_DISP_ST_QUERY_DEVICE_PARAM;
        stQueryCfg.stInCfg.u32CfgSize  = sizeof(HAL_DISP_ST_DeviceParam_t);
        stQueryCfg.stInCfg.pInCfg      = (void *)&stDeviceParam;

        _DRV_DISP_IF_TransDeviceVgaParamToHal(pstVgaInfo, &stDeviceParam);

        bRet = _DRV_DISP_IF_ExecuteQuery(pDevCtx, &stQueryCfg);
    }
    return bRet;
}

MI_U8 DRV_DISP_IF_DeviceGetVgaParam(void *pDevCtx, MI_DISP_VgaParam_t *pstVgaInfo)
{
    MI_U8                     bRet = 1;
    HAL_DISP_ST_QueryConfig_t stQueryCfg;
    HAL_DISP_ST_DeviceParam_t stDeviceParam;

    if (DRV_DISP_CTX_SetCurCtx((DRV_DISP_CTX_Config_t *)pDevCtx, ((DRV_DISP_CTX_Config_t *)pDevCtx)->u32ContainIdx)
        == FALSE)
    {
        bRet = FALSE;
    }
    else
    {
        CamOsMemset(&stQueryCfg, 0, sizeof(HAL_DISP_ST_QueryConfig_t));
        CamOsMemset(&stDeviceParam, 0, sizeof(HAL_DISP_ST_DeviceParam_t));

        stQueryCfg.stInCfg.enQueryType = E_HAL_DISP_ST_QUERY_DEVICE_PARAM_GET;
        stQueryCfg.stInCfg.u32CfgSize  = sizeof(HAL_DISP_ST_DeviceParam_t);
        stQueryCfg.stInCfg.pInCfg      = (void *)&stDeviceParam;

        if (_DRV_DISP_IF_ExecuteQuery(pDevCtx, &stQueryCfg))
        {
            _DRV_DISP_IF_TransDeviceVgaParamToMI(pstVgaInfo, &stDeviceParam);
        }
        else
        {
            bRet = 0;
        }
    }
    return bRet;
}

MI_U8 DRV_DISP_IF_DeviceSetGammaParam(void *pDevCtx, MI_DISP_GammaParam_t *pstGammaInfo)
{
    MI_U8                     bRet = 1;
    HAL_DISP_ST_QueryConfig_t stQueryCfg;

    if (DRV_DISP_CTX_SetCurCtx((DRV_DISP_CTX_Config_t *)pDevCtx, ((DRV_DISP_CTX_Config_t *)pDevCtx)->u32ContainIdx)
        == FALSE)
    {
        bRet = FALSE;
    }
    else
    {
        CamOsMemset(&stQueryCfg, 0, sizeof(HAL_DISP_ST_QueryConfig_t));

        stQueryCfg.stInCfg.enQueryType = E_HAL_DISP_ST_QUERY_DEVICE_GAMMA_PARAM;
        stQueryCfg.stInCfg.u32CfgSize  = sizeof(MI_DISP_GammaParam_t);
        stQueryCfg.stInCfg.pInCfg      = pstGammaInfo;

        bRet = _DRV_DISP_IF_ExecuteQuery(pDevCtx, &stQueryCfg);
    }
    return bRet;
}

MI_U8 DRV_DISP_IF_DeviceAttach(void *pSrcDevCtx, void *pDstDevCtx)
{
    MI_U8                     bRet = 1;
    HAL_DISP_ST_QueryConfig_t stQueryCfg;

    if (DRV_DISP_CTX_SetCurCtx((DRV_DISP_CTX_Config_t *)pSrcDevCtx,
                               ((DRV_DISP_CTX_Config_t *)pSrcDevCtx)->u32ContainIdx)
            == FALSE
        || DRV_DISP_CTX_SetCurCtx((DRV_DISP_CTX_Config_t *)pDstDevCtx,
                                  ((DRV_DISP_CTX_Config_t *)pDstDevCtx)->u32ContainIdx)
               == FALSE)
    {
        bRet = FALSE;
    }
    else
    {
        CamOsMemset(&stQueryCfg, 0, sizeof(HAL_DISP_ST_QueryConfig_t));

        stQueryCfg.stInCfg.enQueryType = E_HAL_DISP_ST_QUERY_DEVICE_ATTACH;
        stQueryCfg.stInCfg.u32CfgSize  = sizeof(pDstDevCtx);
        stQueryCfg.stInCfg.pInCfg      = pDstDevCtx;

        bRet = _DRV_DISP_IF_ExecuteQuery(pSrcDevCtx, &stQueryCfg);
    }
    return bRet;
}

MI_U8 DRV_DISP_IF_DeviceDetach(void *pSrcDevCtx, void *pDstDevCtx)
{
    MI_U8                     bRet = 1;
    HAL_DISP_ST_QueryConfig_t stQueryCfg;

    if (DRV_DISP_CTX_SetCurCtx((DRV_DISP_CTX_Config_t *)pSrcDevCtx,
                               ((DRV_DISP_CTX_Config_t *)pSrcDevCtx)->u32ContainIdx)
            == FALSE
        || DRV_DISP_CTX_SetCurCtx((DRV_DISP_CTX_Config_t *)pDstDevCtx,
                                  ((DRV_DISP_CTX_Config_t *)pDstDevCtx)->u32ContainIdx)
               == FALSE)
    {
        bRet = FALSE;
    }
    else
    {
        CamOsMemset(&stQueryCfg, 0, sizeof(HAL_DISP_ST_QueryConfig_t));

        stQueryCfg.stInCfg.enQueryType = E_HAL_DISP_ST_QUERY_DEVICE_DETACH;
        stQueryCfg.stInCfg.u32CfgSize  = sizeof(pDstDevCtx);
        stQueryCfg.stInCfg.pInCfg      = pDstDevCtx;

        bRet = _DRV_DISP_IF_ExecuteQuery(pSrcDevCtx, &stQueryCfg);
    }
    return bRet;
}

MI_U8 DRV_DISP_IF_DeviceGetTimeZone(void *pDevCtx, MI_DISP_IMPL_MhalTimeZone_t *pstTimeZone)
{
    MI_U8                     bRet = 1;
    HAL_DISP_ST_QueryConfig_t stQueryCfg;

    if (DRV_DISP_CTX_SetCurCtx((DRV_DISP_CTX_Config_t *)pDevCtx, ((DRV_DISP_CTX_Config_t *)pDevCtx)->u32ContainIdx)
        == FALSE)
    {
        bRet = FALSE;
    }
    else
    {
        CamOsMemset(&stQueryCfg, 0, sizeof(HAL_DISP_ST_QueryConfig_t));

        stQueryCfg.stInCfg.enQueryType = E_HAL_DISP_ST_QUERY_DEVICE_TIME_ZONE;
        stQueryCfg.stInCfg.u32CfgSize  = sizeof(MI_DISP_IMPL_MhalTimeZone_t);
        stQueryCfg.stInCfg.pInCfg      = pstTimeZone;

        bRet = _DRV_DISP_IF_ExecuteQuery(pDevCtx, &stQueryCfg);

        if (bRet)
        {
        }
        else
        {
            pstTimeZone->enType = E_MI_DISP_TIMEZONE_UNKONWN;
        }
    }

    return bRet;
}

MI_U8 DRV_DISP_IF_DeviceSetTimeZoneConfig(void *pDevCtx, MI_DISP_IMPL_MhalTimeZoneConfig_t *pstTimeZoneCfg)
{
    MI_U8                     bRet = 1;
    HAL_DISP_ST_QueryConfig_t stQueryCfg;

    if (DRV_DISP_CTX_SetCurCtx((DRV_DISP_CTX_Config_t *)pDevCtx, ((DRV_DISP_CTX_Config_t *)pDevCtx)->u32ContainIdx)
        == FALSE)
    {
        bRet = FALSE;
    }
    else
    {
        CamOsMemset(&stQueryCfg, 0, sizeof(HAL_DISP_ST_QueryConfig_t));

        stQueryCfg.stInCfg.enQueryType = E_HAL_DISP_ST_QUERY_DEVICE_TIME_ZONE_CFG;
        stQueryCfg.stInCfg.u32CfgSize  = sizeof(MI_DISP_IMPL_MhalTimeZoneConfig_t);
        stQueryCfg.stInCfg.pInCfg      = pstTimeZoneCfg;

        bRet = _DRV_DISP_IF_ExecuteQuery(pDevCtx, &stQueryCfg);
    }
    return bRet;
}

MI_U8 DRV_DISP_IF_DeviceGetDisplayInfo(void *pDevCtx, MHAL_DISP_DisplayInfo_t *pstDisplayInfo)
{
    MI_U8                     bRet = 1;
    HAL_DISP_ST_QueryConfig_t stQueryCfg;

    CamOsMemset(&stQueryCfg, 0, sizeof(HAL_DISP_ST_QueryConfig_t));

    stQueryCfg.stInCfg.enQueryType = E_HAL_DISP_ST_QUERY_DEVICE_DISPLAY_INFO;
    stQueryCfg.stInCfg.u32CfgSize  = sizeof(MHAL_DISP_DisplayInfo_t);
    stQueryCfg.stInCfg.pInCfg      = pstDisplayInfo;

    bRet = _DRV_DISP_IF_ExecuteQuery(pDevCtx, &stQueryCfg);

    if (bRet)
    {
    }
    else
    {
        CamOsMemset(pstDisplayInfo, 0, sizeof(MHAL_DISP_DisplayInfo_t));
    }

    return bRet;
}

MI_U8 DRV_DISP_IF_DeviceGetInstance(MI_U32 u32DeviceId, void **pDevCtx)
{
    MI_U8                  bRet       = TRUE;
    DRV_DISP_CTX_Config_t *pstDispCtx = NULL;

    if (DRV_DISP_CTX_GetDeviceCurCtx(u32DeviceId, &pstDispCtx))
    {
        *pDevCtx = (void *)pstDispCtx;
    }
    else
    {
        bRet     = FALSE;
        *pDevCtx = NULL;
    }
    return bRet;
}

MI_U8 DRV_DISP_IF_DeviceSetPqConfig(void *pDevCtx, MI_DISP_IMPL_MhalPqConfig_t *pstPqCfg)
{
    MI_U8                     bRet = 1;
    HAL_DISP_ST_QueryConfig_t stQueryCfg;

    CamOsMemset(&stQueryCfg, 0, sizeof(HAL_DISP_ST_QueryConfig_t));

    stQueryCfg.stInCfg.enQueryType = E_HAL_DISP_ST_QUERY_PQ_SET;
    stQueryCfg.stInCfg.u32CfgSize  = sizeof(MI_DISP_IMPL_MhalPqConfig_t);
    stQueryCfg.stInCfg.pInCfg      = pstPqCfg;

    if (pDevCtx == NULL
        && (pstPqCfg->u32PqFlags == E_MI_DISP_PQ_FLAG_LOAD_BIN || pstPqCfg->u32PqFlags == E_MI_DISP_PQ_FLAG_FREE_BIN
            || pstPqCfg->u32PqFlags == E_MI_DISP_PQ_FLAG_SET_SRC_ID
            || pstPqCfg->u32PqFlags == E_MI_DISP_PQ_FLAG_SET_LOAD_SETTING_TYPE))
    {
        HAL_DISP_IF_Init();
        bRet = _DRV_DISP_IF_ExecuteQueryWithoutCtx(&stQueryCfg);
    }
    else
    {
        bRet = _DRV_DISP_IF_ExecuteQuery(pDevCtx, &stQueryCfg);
    }

    return bRet;
}

MI_U8 DRV_DISP_IF_DeviceGetHwCount(MI_U32 *pu32Count)
{
    MI_U8                      bRet = 1;
    HAL_DISP_ST_QueryConfig_t  stQueryCfg;
    HAL_DISP_ST_HwInfoConfig_t stHalHwInfoCfg;
    DRV_DISP_CTX_Config_t      stDispCtxCfg;

    HAL_DISP_IF_Init();

    CamOsMemset(&stQueryCfg, 0, sizeof(HAL_DISP_ST_QueryConfig_t));
    stQueryCfg.stInCfg.enQueryType = E_HAL_DISP_ST_QUERY_HW_INFO;
    stQueryCfg.stInCfg.u32CfgSize  = sizeof(HAL_DISP_ST_HwInfoConfig_t);
    stQueryCfg.stInCfg.pInCfg      = &stHalHwInfoCfg;

    CamOsMemset(&stHalHwInfoCfg, 0, sizeof(HAL_DISP_ST_HwInfoConfig_t));
    stHalHwInfoCfg.eType = E_HAL_DISP_ST_HW_INFO_DEVICE;

    if (_DRV_DISP_IF_ExecuteQuery(&stDispCtxCfg, &stQueryCfg))
    {
        *pu32Count = stHalHwInfoCfg.u32Count;
        bRet       = 1;
    }
    else
    {
        *pu32Count = 0;
        bRet       = 0;
    }
    return bRet;
}

MI_U8 DRV_DISP_IF_SetDeviceConfig(MI_U32 u32DevId, MI_DISP_IMPL_MhalDeviceConfig_t *pstDevCfg)
{
    HAL_DISP_ST_QueryConfig_t stQueryCfg;
    DRV_DISP_CTX_Config_t     stDispCtxCfg;
    DRV_DISP_CTX_Config_t *   pstDispCtx;
    MI_U8                     bRet;

    HAL_DISP_IF_Init();

    CamOsMemset(&stQueryCfg, 0, sizeof(HAL_DISP_ST_QueryConfig_t));

    stQueryCfg.stInCfg.enQueryType = E_HAL_DISP_ST_QUERY_INTERCFG_SET;
    stQueryCfg.stInCfg.u32CfgSize  = sizeof(MI_DISP_IMPL_MhalDeviceConfig_t);
    stQueryCfg.stInCfg.pInCfg      = pstDevCfg;

    stDispCtxCfg.u32ContainIdx = u32DevId;
    stDispCtxCfg.enCtxType     = E_DRV_DISP_CTX_TYPE_DEVICE;

    if (pstDevCfg->eType & E_MI_DISP_DEV_CFG_CSC_MD)
    {
        pstDevCfg->bCtx = 1;
    }
    if (pstDevCfg->bCtx)
    {
        DRV_DISP_IF_DeviceGetInstance(u32DevId, (void *)&pstDispCtx);
        stDispCtxCfg.pstCtxContain = (pstDispCtx) ? pstDispCtx->pstCtxContain : NULL;
    }
    else
    {
        stDispCtxCfg.pstCtxContain = NULL;
    }

    bRet = _DRV_DISP_IF_ExecuteQuery(&stDispCtxCfg, &stQueryCfg);
    return bRet;
}

MI_U8 DRV_DISP_IF_GetDeviceConfig(MI_U32 u32DevId, MI_DISP_IMPL_MhalDeviceConfig_t *pstDevCfg)
{
    HAL_DISP_ST_QueryConfig_t stQueryCfg;
    DRV_DISP_CTX_Config_t     stDispCtxCfg;
    MI_U8                     bRet = 1;

    HAL_DISP_IF_Init();

    CamOsMemset(&stQueryCfg, 0, sizeof(HAL_DISP_ST_QueryConfig_t));

    stQueryCfg.stInCfg.enQueryType = E_HAL_DISP_ST_QUERY_INTERCFG_GET;
    stQueryCfg.stInCfg.u32CfgSize  = sizeof(MI_DISP_IMPL_MhalDeviceConfig_t);
    stQueryCfg.stInCfg.pInCfg      = pstDevCfg;

    stDispCtxCfg.u32ContainIdx = u32DevId;
    stDispCtxCfg.enCtxType     = E_DRV_DISP_CTX_TYPE_DEVICE;
    stDispCtxCfg.pstCtxContain = NULL;

    if (_DRV_DISP_IF_ExecuteQuery(&stDispCtxCfg, &stQueryCfg))
    {
    }
    else
    {
        bRet = 0;
        DISP_ERR("%s %d Get Config Fail\n", __FUNCTION__, __LINE__);
    }
    return bRet;
}

MI_U8 DRV_DISP_IF_DeviceGetCapabilityConfig(MI_U32 u32DevId, MI_DISP_IMPL_MhalDeviceCapabilityConfig_t *pstDevCapCfg)
{
    MI_U8                                bRet = 1;
    HAL_DISP_ST_QueryConfig_t            stQueryCfg;
    HAL_DISP_ST_DeviceCapabilityConfig_t stHalDevCapCfg;
    DRV_DISP_CTX_Config_t                stDispCtxCfg;

    HAL_DISP_IF_Init();

    CamOsMemset(&stQueryCfg, 0, sizeof(HAL_DISP_ST_QueryConfig_t));
    stQueryCfg.stInCfg.enQueryType = E_HAL_DISP_ST_QUERY_DEVICE_CAPABILITY_GET;
    stQueryCfg.stInCfg.u32CfgSize  = sizeof(HAL_DISP_ST_DeviceCapabilityConfig_t);
    stQueryCfg.stInCfg.pInCfg      = &stHalDevCapCfg;

    stHalDevCapCfg.u32DevId = u32DevId;

    if (_DRV_DISP_IF_ExecuteQuery(&stDispCtxCfg, &stQueryCfg))
    {
        CamOsMemcpy(pstDevCapCfg, &stHalDevCapCfg.stDevCapCfg, sizeof(MI_DISP_IMPL_MhalDeviceCapabilityConfig_t));
        bRet = 1;
    }
    else
    {
        bRet = 0;
    }

    return bRet;
}

MI_U8 DRV_DISP_IF_DeviceGetInterfaceCapabilityConfig(MI_U32                                        u32Interface,
                                                     MI_DISP_IMPL_MhalInterfaceCapabilityConfig_t *pstInterfaceCapCfg)
{
    MI_U8                                   bRet = 1;
    HAL_DISP_ST_QueryConfig_t               stQueryCfg;
    HAL_DISP_ST_InterfaceCapabilityConfig_t stHalInterfaceCapCfg;
    DRV_DISP_CTX_Config_t                   stDispCtxCfg;

    HAL_DISP_IF_Init();

    CamOsMemset(&stQueryCfg, 0, sizeof(HAL_DISP_ST_QueryConfig_t));
    stQueryCfg.stInCfg.enQueryType = E_HAL_DISP_ST_QUERY_INTERFACE_CAPABILITY_GET;
    stQueryCfg.stInCfg.u32CfgSize  = sizeof(HAL_DISP_ST_InterfaceCapabilityConfig_t);
    stQueryCfg.stInCfg.pInCfg      = &stHalInterfaceCapCfg;

    stHalInterfaceCapCfg.u32Interface = u32Interface;

    if (_DRV_DISP_IF_ExecuteQuery(&stDispCtxCfg, &stQueryCfg))
    {
        CamOsMemcpy(pstInterfaceCapCfg, &stHalInterfaceCapCfg.stInterfaceCapCfg,
                    sizeof(MI_DISP_IMPL_MhalInterfaceCapabilityConfig_t));

        bRet = 1;
    }
    else
    {
        bRet = 0;
    }
    return bRet;
}

// VideoLayer
MI_U8 DRV_DISP_IF_VideoLayerCreateInstance(MI_DISP_IMPL_MhalMemAllocConfig_t *pstAlloc,
                                           MI_DISP_IMPL_MhalVideoLayerType_e eVidLayerType, void **pVidLayerCtx)
{
    MI_U8                      bRet = TRUE;
    DRV_DISP_CTX_AllocConfig_t stCtxAllocCfg;
    HAL_DISP_ST_QueryConfig_t  stQueryCfg;
    DRV_DISP_CTX_Config_t *    pstDispCtx = NULL;

    MI_U32 u32NewLayerId = HAL_DISP_MAPPING_VIDLAYERID_FROM_MI((MI_U32)eVidLayerType);

    DRV_DISP_CTX_Init();

    stCtxAllocCfg.enType             = E_DRV_DISP_CTX_TYPE_VIDLAYER;
    stCtxAllocCfg.u32DevId           = u32NewLayerId;
    stCtxAllocCfg.pstBindCtx         = NULL;
    stCtxAllocCfg.stMemAllcCfg.alloc = NULL;
    stCtxAllocCfg.stMemAllcCfg.free  = NULL;

    if (DRV_DISP_CTX_Allocate(&stCtxAllocCfg, &pstDispCtx) == FALSE)
    {
        bRet          = 0;
        *pVidLayerCtx = NULL;
        DISP_ERR("%s %d, CreateInstance Fail\n", __FUNCTION__, __LINE__);
    }
    else
    {
        *pVidLayerCtx = (void *)pstDispCtx;

        CamOsMemset(&stQueryCfg, 0, sizeof(HAL_DISP_ST_QueryConfig_t));
        stQueryCfg.stInCfg.enQueryType = E_HAL_DISP_ST_QUERY_VIDEOLAYER_INIT;
        stQueryCfg.stInCfg.pInCfg      = NULL;
        stQueryCfg.stInCfg.u32CfgSize  = 0;
        bRet                           = _DRV_DISP_IF_ExecuteQuery(pstDispCtx, &stQueryCfg);
        DISP_DBG(DISP_DBG_LEVEL_DRV, "%s %d,Create Inst VidId:%d (%s) Ret(%d) \n", __FUNCTION__, __LINE__,
                 u32NewLayerId, PARSING_VIDLAYER_TYPE(u32NewLayerId), bRet);
    }

    return bRet;
}

MI_U8 DRV_DISP_IF_VideoLayerDestoryInstance(void *pVidLayerCtx)
{
    MI_U8                  bRet       = TRUE;
    DRV_DISP_CTX_Config_t *pstDispCtx = (DRV_DISP_CTX_Config_t *)pVidLayerCtx;

    if (DRV_DISP_CTX_Free(pstDispCtx))
    {
        if (DRV_DISP_CTX_IsAllFree())
        {
            DRV_DISP_CTX_DeInit();
        }
    }
    else
    {
        bRet = 0;
        DISP_ERR("%s %d, DestroyInstance Fail\n", __FUNCTION__, __LINE__);
    }

    return bRet;
}

MI_U8 DRV_DISP_IF_VideoLayerEnable(void *pVidLayerCtx, MI_U8 bEnable)
{
    MI_U8                     bRet = 1;
    HAL_DISP_ST_QueryConfig_t stQueryCfg;

    CamOsMemset(&stQueryCfg, 0, sizeof(HAL_DISP_ST_QueryConfig_t));

    stQueryCfg.stInCfg.enQueryType = E_HAL_DISP_ST_QUERY_VIDEOLAYER_ENABLE;
    stQueryCfg.stInCfg.u32CfgSize  = sizeof(MI_U8);
    stQueryCfg.stInCfg.pInCfg      = &bEnable;

    bRet = _DRV_DISP_IF_ExecuteQuery(pVidLayerCtx, &stQueryCfg);

    return bRet;
}

MI_U8 DRV_DISP_IF_VideoLayerBind(void *pVidLayerCtx, void *pDevCtx)
{
    MI_U8                     bRet = 1;
    HAL_DISP_ST_QueryConfig_t stQueryCfg;

    CamOsMemset(&stQueryCfg, 0, sizeof(HAL_DISP_ST_QueryConfig_t));

    stQueryCfg.stInCfg.enQueryType = E_HAL_DISP_ST_QUERY_VIDEOLAYER_BIND;
    stQueryCfg.stInCfg.u32CfgSize  = sizeof(pDevCtx);
    stQueryCfg.stInCfg.pInCfg      = pDevCtx;

    bRet = _DRV_DISP_IF_ExecuteQuery(pVidLayerCtx, &stQueryCfg);

    return bRet;
}

MI_U8 DRV_DISP_IF_VideoLayerUnBind(void *pVidLayerCtx, void *pDevCtx)
{
    MI_U8                     bRet = 1;
    HAL_DISP_ST_QueryConfig_t stQueryCfg;

    CamOsMemset(&stQueryCfg, 0, sizeof(HAL_DISP_ST_QueryConfig_t));

    stQueryCfg.stInCfg.enQueryType = E_HAL_DISP_ST_QUERY_VIDEOLAYER_UNBIND;
    stQueryCfg.stInCfg.u32CfgSize  = sizeof(pDevCtx);
    stQueryCfg.stInCfg.pInCfg      = pDevCtx;

    bRet = _DRV_DISP_IF_ExecuteQuery(pVidLayerCtx, &stQueryCfg);

    return bRet;
}

MI_U8 DRV_DISP_IF_VideoLayerSetAttr(void *pVidLayerCtx, MI_DISP_VideoLayerAttr_t *pstAttr)
{
    MI_U8                     bRet = 1;
    HAL_DISP_ST_QueryConfig_t stQueryCfg;

    CamOsMemset(&stQueryCfg, 0, sizeof(HAL_DISP_ST_QueryConfig_t));

    stQueryCfg.stInCfg.enQueryType = E_HAL_DISP_ST_QUERY_VIDEOLAYER_ATTR;
    stQueryCfg.stInCfg.u32CfgSize  = sizeof(MI_DISP_VideoLayerAttr_t);
    stQueryCfg.stInCfg.pInCfg      = pstAttr;

    bRet = _DRV_DISP_IF_ExecuteQuery(pVidLayerCtx, &stQueryCfg);

    return bRet;
}

MI_U8 DRV_DISP_IF_VideoLayerBufferFire(void *pVidLayerCtx)
{
    MI_U8                     bRet = 1;
    HAL_DISP_ST_QueryConfig_t stQueryCfg;

    CamOsMemset(&stQueryCfg, 0, sizeof(HAL_DISP_ST_QueryConfig_t));

    stQueryCfg.stInCfg.enQueryType = E_HAL_DISP_ST_QUERY_VIDEOLAYER_BUFFER_FIRE;
    stQueryCfg.stInCfg.u32CfgSize  = 0;
    stQueryCfg.stInCfg.pInCfg      = NULL;

    bRet = _DRV_DISP_IF_ExecuteQuery(pVidLayerCtx, &stQueryCfg);

    return bRet;
}

MI_U8 DRV_DISP_IF_VideoLayerCheckBufferFired(void *pVidLayerCtx)
{
    MI_U8                     bRet = 1;
    HAL_DISP_ST_QueryConfig_t stQueryCfg;

    CamOsMemset(&stQueryCfg, 0, sizeof(HAL_DISP_ST_QueryConfig_t));

    stQueryCfg.stInCfg.enQueryType = E_HAL_DISP_ST_QUERY_VIDEOLAYER_CHECK_FIRE;
    stQueryCfg.stInCfg.u32CfgSize  = 0;
    stQueryCfg.stInCfg.pInCfg      = NULL;

    bRet = _DRV_DISP_IF_ExecuteQuery(pVidLayerCtx, &stQueryCfg);

    return bRet;
}

MI_U8 DRV_DISP_IF_VideoLayerSetCompress(void *pVidLayerCtx, MI_BOOL bCompress)
{
    MI_U8                     bRet = 1;
    HAL_DISP_ST_QueryConfig_t stQueryCfg;

    CamOsMemset(&stQueryCfg, 0, sizeof(HAL_DISP_ST_QueryConfig_t));

    stQueryCfg.stInCfg.enQueryType = E_HAL_DISP_ST_QUERY_VIDEOLAYER_COMPRESS;
    stQueryCfg.stInCfg.u32CfgSize  = sizeof(MI_BOOL);
    stQueryCfg.stInCfg.pInCfg      = &bCompress;

    bRet = _DRV_DISP_IF_ExecuteQuery(pVidLayerCtx, &stQueryCfg);

    return bRet;
}

MI_U8 DRV_DISP_IF_VideoLayerSetPriority(void *pVidLayerCtx, MI_U32 u32Priority)
{
    MI_U8                     bRet = 1;
    HAL_DISP_ST_QueryConfig_t stQueryCfg;

    CamOsMemset(&stQueryCfg, 0, sizeof(HAL_DISP_ST_QueryConfig_t));

    stQueryCfg.stInCfg.enQueryType = E_HAL_DISP_ST_QUERY_VIDEOLAYER_PRIORITY;
    stQueryCfg.stInCfg.u32CfgSize  = sizeof(MI_U32);
    stQueryCfg.stInCfg.pInCfg      = &u32Priority;

    bRet = _DRV_DISP_IF_ExecuteQuery(pVidLayerCtx, &stQueryCfg);

    return bRet;
}

MI_U8 DRV_DISP_IF_VideoLayerGetCapabilityConfig(MI_DISP_IMPL_MhalVideoLayerType_e              eVidLayerType,
                                                MI_DISP_IMPL_MhalVideoLayerCapabilityConfig_t *pstVidLayerCapCfg)
{
    MI_U8                                    bRet = 1;
    HAL_DISP_ST_QueryConfig_t                stQueryCfg;
    HAL_DISP_ST_VideoLayerCapabilityConfig_t stHalVidLayerCapCfg;
    DRV_DISP_CTX_Config_t                    stDispCtxCfg;

    HAL_DISP_IF_Init();

    CamOsMemset(&stQueryCfg, 0, sizeof(HAL_DISP_ST_QueryConfig_t));
    stQueryCfg.stInCfg.enQueryType = E_HAL_DISP_ST_QUERY_VIDLAYER_CAPABILITY_GET;
    stQueryCfg.stInCfg.u32CfgSize  = sizeof(HAL_DISP_ST_VideoLayerCapabilityConfig_t);
    stQueryCfg.stInCfg.pInCfg      = &stHalVidLayerCapCfg;

    CamOsMemset(&stHalVidLayerCapCfg, 0, sizeof(HAL_DISP_ST_VideoLayerCapabilityConfig_t));
    stHalVidLayerCapCfg.eVidLayerType            = eVidLayerType;
    stHalVidLayerCapCfg.stVidLayerCapCfg.u8DevId = pstVidLayerCapCfg->u8DevId;

    if (_DRV_DISP_IF_ExecuteQuery(&stDispCtxCfg, &stQueryCfg))
    {
        CamOsMemcpy(pstVidLayerCapCfg, &stHalVidLayerCapCfg.stVidLayerCapCfg,
                    sizeof(MI_DISP_IMPL_MhalVideoLayerCapabilityConfig_t));
        bRet = 1;
    }
    else
    {
        bRet = 0;
    }
    return bRet;
}

// InputPort
MI_U8 DRV_DISP_IF_InputPortCreateInstance(MI_DISP_IMPL_MhalMemAllocConfig_t *pstAlloc, void *pVidLayerCtx,
                                          MI_U32 u32PortId, void **pCtx)
{
    MI_U8                      bRet = TRUE;
    DRV_DISP_CTX_AllocConfig_t stCtxAllocCfg;
    HAL_DISP_ST_QueryConfig_t  stQueryCfg;
    DRV_DISP_CTX_Config_t *    pstDispCtx = NULL;

    DRV_DISP_CTX_Init();

    stCtxAllocCfg.enType             = E_DRV_DISP_CTX_TYPE_INPUTPORT;
    stCtxAllocCfg.u32DevId           = u32PortId;
    stCtxAllocCfg.pstBindCtx         = (DRV_DISP_CTX_Config_t *)pVidLayerCtx;
    stCtxAllocCfg.stMemAllcCfg.alloc = NULL;
    stCtxAllocCfg.stMemAllcCfg.free  = NULL;

    if (DRV_DISP_CTX_Allocate(&stCtxAllocCfg, &pstDispCtx) == FALSE)
    {
        bRet  = 0;
        *pCtx = NULL;
        DISP_ERR("%s %d, CreateInstance Fail\n", __FUNCTION__, __LINE__);
    }
    else
    {
        *pCtx = (void *)pstDispCtx;

        CamOsMemset(&stQueryCfg, 0, sizeof(HAL_DISP_ST_QueryConfig_t));
        stQueryCfg.stInCfg.enQueryType = E_HAL_DISP_ST_QUERY_INPUTPORT_INIT;
        stQueryCfg.stInCfg.pInCfg      = NULL;
        stQueryCfg.stInCfg.u32CfgSize  = 0;
        bRet                           = _DRV_DISP_IF_ExecuteQuery(pstDispCtx, &stQueryCfg);
    }

    return bRet;
}

MI_U8 DRV_DISP_IF_InputPortDestroyInstance(void *pInputPortCtx)
{
    MI_U8                  bRet       = TRUE;
    DRV_DISP_CTX_Config_t *pstDispCtx = (DRV_DISP_CTX_Config_t *)pInputPortCtx;

    if (DRV_DISP_CTX_Free(pstDispCtx))
    {
        if (DRV_DISP_CTX_IsAllFree())
        {
            DRV_DISP_CTX_DeInit();
        }
    }
    else
    {
        bRet = 0;
        DISP_ERR("%s %d, DestroyInstance Fail\n", __FUNCTION__, __LINE__);
    }

    return bRet;
}

MI_U8 DRV_DISP_IF_InputPortRotate(void *pInputPortCtx, MI_DISP_RotateConfig_t *pstRotateCfg)
{
    MI_U8                     bRet = 1;
    HAL_DISP_ST_QueryConfig_t stQueryCfg;

    CamOsMemset(&stQueryCfg, 0, sizeof(HAL_DISP_ST_QueryConfig_t));

    stQueryCfg.stInCfg.enQueryType = E_HAL_DISP_ST_QUERY_INPUTPORT_ROTATE;
    stQueryCfg.stInCfg.u32CfgSize  = sizeof(MI_DISP_RotateConfig_t);
    stQueryCfg.stInCfg.pInCfg      = pstRotateCfg;

    bRet = _DRV_DISP_IF_ExecuteQuery(pInputPortCtx, &stQueryCfg);

    return bRet;
}

MI_U8 DRV_DISP_IF_InputPortSetCropAttr(void *pInputPortCtx, MI_DISP_VidWinRect_t *pstCropAttr)
{
    MI_U8                     bRet = 1;
    HAL_DISP_ST_QueryConfig_t stQueryCfg;

    CamOsMemset(&stQueryCfg, 0, sizeof(HAL_DISP_ST_QueryConfig_t));

    stQueryCfg.stInCfg.enQueryType = E_HAL_DISP_ST_QUERY_INPUTPORT_CROP;
    stQueryCfg.stInCfg.u32CfgSize  = sizeof(MI_DISP_VidWinRect_t);
    stQueryCfg.stInCfg.pInCfg      = pstCropAttr;

    bRet = _DRV_DISP_IF_ExecuteQuery(pInputPortCtx, &stQueryCfg);

    return bRet;
}

MI_U8 DRV_DISP_IF_InputPortFlip(void *pInputPortCtx, MI_DISP_IMPL_MhalVideoFrameData_t *pstVideoFrameData)
{
    MI_U8                     bRet = 1;
    HAL_DISP_ST_QueryConfig_t stQueryCfg;

    CamOsMemset(&stQueryCfg, 0, sizeof(HAL_DISP_ST_QueryConfig_t));

    stQueryCfg.stInCfg.enQueryType = E_HAL_DISP_ST_QUERY_INPUTPORT_FLIP;
    stQueryCfg.stInCfg.u32CfgSize  = sizeof(MI_DISP_IMPL_MhalVideoFrameData_t);
    stQueryCfg.stInCfg.pInCfg      = pstVideoFrameData;

    bRet = _DRV_DISP_IF_ExecuteQuery(pInputPortCtx, &stQueryCfg);

    return bRet;
}

MI_U8 DRV_DISP_IF_InputPortSetRingBuffAttr(void *pInputPortCtx, MI_DISP_IMPL_MhalRingBufferAttr_t *pstRingBufAttr)
{
    MI_U8                     bRet = 1;
    HAL_DISP_ST_QueryConfig_t stQueryCfg;

    CamOsMemset(&stQueryCfg, 0, sizeof(HAL_DISP_ST_QueryConfig_t));

    stQueryCfg.stInCfg.enQueryType = E_HAL_DISP_ST_QUERY_INPUTPORT_RING_BUFF_ATTR;
    stQueryCfg.stInCfg.u32CfgSize  = sizeof(MI_DISP_IMPL_MhalRingBufferAttr_t);
    stQueryCfg.stInCfg.pInCfg      = pstRingBufAttr;

    bRet = _DRV_DISP_IF_ExecuteQuery(pInputPortCtx, &stQueryCfg);

    return bRet;
}

MI_U8 DRV_DISP_IF_InputPortEnable(void *pInputPortCtx, MI_U8 bEnable)
{
    MI_U8                     bRet = 1;
    HAL_DISP_ST_QueryConfig_t stQueryCfg;

    CamOsMemset(&stQueryCfg, 0, sizeof(HAL_DISP_ST_QueryConfig_t));

    stQueryCfg.stInCfg.enQueryType = E_HAL_DISP_ST_QUERY_INPUTPORT_ENABLE;
    stQueryCfg.stInCfg.u32CfgSize  = sizeof(MI_U8);
    stQueryCfg.stInCfg.pInCfg      = &bEnable;

    bRet = _DRV_DISP_IF_ExecuteQuery(pInputPortCtx, &stQueryCfg);

    return bRet;
}

MI_U8 DRV_DISP_IF_InputPortSetAttr(void *pInputPortCtx, MI_DISP_InputPortAttr_t *pstAttr)
{
    MI_U8                     bRet = 1;
    HAL_DISP_ST_QueryConfig_t stQueryCfg;

    CamOsMemset(&stQueryCfg, 0, sizeof(HAL_DISP_ST_QueryConfig_t));

    stQueryCfg.stInCfg.enQueryType = E_HAL_DISP_ST_QUERY_INPUTPORT_ATTR;
    stQueryCfg.stInCfg.u32CfgSize  = sizeof(MI_DISP_InputPortAttr_t);
    stQueryCfg.stInCfg.pInCfg      = pstAttr;

    bRet = _DRV_DISP_IF_ExecuteQuery(pInputPortCtx, &stQueryCfg);

    return bRet;
}

MI_U8 DRV_DISP_IF_InputPortShow(void *pInputPortCtx)
{
    MI_U8                     bRet = 1;
    HAL_DISP_ST_QueryConfig_t stQueryCfg;

    CamOsMemset(&stQueryCfg, 0, sizeof(HAL_DISP_ST_QueryConfig_t));

    stQueryCfg.stInCfg.enQueryType = E_HAL_DISP_ST_QUERY_INPUTPORT_SHOW;
    stQueryCfg.stInCfg.u32CfgSize  = 0;
    stQueryCfg.stInCfg.pInCfg      = NULL;

    bRet = _DRV_DISP_IF_ExecuteQuery(pInputPortCtx, &stQueryCfg);

    return bRet;
}

MI_U8 DRV_DISP_IF_InputPortHide(void *pInputPortCtx)
{
    MI_U8                     bRet = 1;
    HAL_DISP_ST_QueryConfig_t stQueryCfg;

    CamOsMemset(&stQueryCfg, 0, sizeof(HAL_DISP_ST_QueryConfig_t));

    stQueryCfg.stInCfg.enQueryType = E_HAL_DISP_ST_QUERY_INPUTPORT_HIDE;
    stQueryCfg.stInCfg.u32CfgSize  = 0;
    stQueryCfg.stInCfg.pInCfg      = NULL;

    bRet = _DRV_DISP_IF_ExecuteQuery(pInputPortCtx, &stQueryCfg);

    return bRet;
}

MI_U8 DRV_DISP_IF_InputPortAttrBegin(void *pVidLayerCtx)
{
    MI_U8                     bRet = 1;
    HAL_DISP_ST_QueryConfig_t stQueryCfg;

    CamOsMemset(&stQueryCfg, 0, sizeof(HAL_DISP_ST_QueryConfig_t));

    stQueryCfg.stInCfg.enQueryType = E_HAL_DISP_ST_QUERY_INPUTPORT_BEGIN;
    stQueryCfg.stInCfg.u32CfgSize  = 0;
    stQueryCfg.stInCfg.pInCfg      = NULL;

    bRet = _DRV_DISP_IF_ExecuteQuery(pVidLayerCtx, &stQueryCfg);

    return bRet;
}

MI_U8 DRV_DISP_IF_InputPortAttrEnd(void *pVidLayerCtx)
{
    MI_U8                     bRet = 1;
    HAL_DISP_ST_QueryConfig_t stQueryCfg;

    CamOsMemset(&stQueryCfg, 0, sizeof(HAL_DISP_ST_QueryConfig_t));

    stQueryCfg.stInCfg.enQueryType = E_HAL_DISP_ST_QUERY_INPUTPORT_END;
    stQueryCfg.stInCfg.u32CfgSize  = 0;
    stQueryCfg.stInCfg.pInCfg      = NULL;

    bRet = _DRV_DISP_IF_ExecuteQuery(pVidLayerCtx, &stQueryCfg);

    return bRet;
}

MI_U8 DRV_DISP_IF_InputPortSetImiAddr(void *pInputPortCtx, MI_DISP_IMPL_MhalVideoFrameData_t *pstImiAddr)
{
    MI_U8                     bRet = 1;
    HAL_DISP_ST_QueryConfig_t stQueryCfg;

    CamOsMemset(&stQueryCfg, 0, sizeof(HAL_DISP_ST_QueryConfig_t));

    stQueryCfg.stInCfg.enQueryType = E_HAL_DISP_ST_QUERY_INPUTPORT_IMIADDR;
    stQueryCfg.stInCfg.u32CfgSize  = sizeof(MI_DISP_IMPL_MhalVideoFrameData_t);
    stQueryCfg.stInCfg.pInCfg      = pstImiAddr;

    bRet = _DRV_DISP_IF_ExecuteQuery(pInputPortCtx, &stQueryCfg);

    return bRet;
}

MI_U8 DRV_DISP_IF_SetDbgLevel(void *p)
{
    g_u32DispDbgLevel = *((MI_U32 *)p);
    DISP_DBG(DISP_DBG_LEVEL_DRV, "%s %d, g_u32DispDbgLevel:%x\n", __FUNCTION__, __LINE__, g_u32DispDbgLevel);
    return TRUE;
}

MI_U8 DRV_DISP_IF_DmaCreateInstance(MI_DISP_IMPL_MhalMemAllocConfig_t *pstAlloc, MI_U32 u32DmaId, void **pDmaCtx)
{
    MI_U8                      bRet = TRUE;
    DRV_DISP_CTX_AllocConfig_t stCtxAllocCfg;
    HAL_DISP_ST_QueryConfig_t  stQueryCfg;
    DRV_DISP_CTX_Config_t *    pstDispCtx = NULL;

    DRV_DISP_CTX_Init();

    stCtxAllocCfg.enType             = E_DRV_DISP_CTX_TYPE_DMA;
    stCtxAllocCfg.u32DevId           = u32DmaId;
    stCtxAllocCfg.pstBindCtx         = NULL;
    stCtxAllocCfg.stMemAllcCfg.alloc = NULL;
    stCtxAllocCfg.stMemAllcCfg.free  = NULL;

    if (DRV_DISP_CTX_Allocate(&stCtxAllocCfg, &pstDispCtx) == FALSE)
    {
        bRet     = 0;
        *pDmaCtx = NULL;
        DISP_ERR("%s %d, CreateInstance Fail\n", __FUNCTION__, __LINE__);
    }
    else
    {
        *pDmaCtx = (void *)pstDispCtx;

        CamOsMemset(&stQueryCfg, 0, sizeof(HAL_DISP_ST_QueryConfig_t));
        stQueryCfg.stInCfg.enQueryType = E_HAL_DISP_ST_QUERY_DMA_INIT;
        stQueryCfg.stInCfg.pInCfg      = NULL;
        stQueryCfg.stInCfg.u32CfgSize  = 0;
        bRet                           = _DRV_DISP_IF_ExecuteQuery((void *)pstDispCtx, &stQueryCfg);
    }

    return bRet;
}

MI_U8 DRV_DISP_IF_DmaDestoryInstance(void *pDmaCtx)
{
    MI_U8                     bRet       = TRUE;
    DRV_DISP_CTX_Config_t *   pstDispCtx = (DRV_DISP_CTX_Config_t *)pDmaCtx;
    HAL_DISP_ST_QueryConfig_t stQueryCfg;

    CamOsMemset(&stQueryCfg, 0, sizeof(HAL_DISP_ST_QueryConfig_t));
    stQueryCfg.stInCfg.enQueryType = E_HAL_DISP_ST_QUERY_DMA_DEINIT;
    stQueryCfg.stInCfg.pInCfg      = NULL;
    stQueryCfg.stInCfg.u32CfgSize  = 0;

    if (_DRV_DISP_IF_ExecuteQuery(pstDispCtx, &stQueryCfg))
    {
        if (DRV_DISP_CTX_Free(pstDispCtx))
        {
            if (DRV_DISP_CTX_IsAllFree())
            {
                DRV_DISP_CTX_DeInit();
            }
        }
        else
        {
            bRet = 0;
            DISP_ERR("%s %d, DestroyInstance Fail\n", __FUNCTION__, __LINE__);
        }
    }
    else
    {
        bRet = 0;
        DISP_ERR("%s %d, Dma DeInit Fail\n", __FUNCTION__, __LINE__);
    }
    return bRet;
}

MI_U8 DRV_DISP_IF_DmaBind(void *pDmaCtx, MI_DISP_IMPL_MhalDmaBindConfig_t *pstDmaBindCfg)
{
    MI_U8                     bRet = 1;
    HAL_DISP_ST_QueryConfig_t stQueryCfg;

    CamOsMemset(&stQueryCfg, 0, sizeof(HAL_DISP_ST_QueryConfig_t));

    stQueryCfg.stInCfg.enQueryType = E_HAL_DISP_ST_QUERY_DMA_BIND;
    stQueryCfg.stInCfg.u32CfgSize  = sizeof(MI_DISP_IMPL_MhalDmaBindConfig_t);
    stQueryCfg.stInCfg.pInCfg      = pstDmaBindCfg;

    bRet = _DRV_DISP_IF_ExecuteQuery(pDmaCtx, &stQueryCfg);

    return bRet;
}

MI_U8 DRV_DISP_IF_DmaUnBind(void *pDmaCtx, MI_DISP_IMPL_MhalDmaBindConfig_t *pstDmaBindCfg)
{
    MI_U8                     bRet = 1;
    HAL_DISP_ST_QueryConfig_t stQueryCfg;

    CamOsMemset(&stQueryCfg, 0, sizeof(HAL_DISP_ST_QueryConfig_t));

    stQueryCfg.stInCfg.enQueryType = E_HAL_DISP_ST_QUERY_DMA_UNBIND;
    stQueryCfg.stInCfg.u32CfgSize  = sizeof(MI_DISP_IMPL_MhalDmaBindConfig_t);
    stQueryCfg.stInCfg.pInCfg      = pstDmaBindCfg;

    bRet = _DRV_DISP_IF_ExecuteQuery(pDmaCtx, &stQueryCfg);

    return bRet;
}

MI_U8 DRV_DISP_IF_DmaSetAttr(void *pDmaCtx, MI_DISP_IMPL_MhalDmaAttrConfig_t *pstDmaAttrCfg)
{
    MI_U8                     bRet = 1;
    HAL_DISP_ST_QueryConfig_t stQueryCfg;

    if (DRV_DISP_CTX_SetCurCtx((DRV_DISP_CTX_Config_t *)pDmaCtx, ((DRV_DISP_CTX_Config_t *)pDmaCtx)->u32ContainIdx)
        == FALSE)
    {
        bRet = FALSE;
    }
    else
    {
        CamOsMemset(&stQueryCfg, 0, sizeof(HAL_DISP_ST_QueryConfig_t));

        stQueryCfg.stInCfg.enQueryType = E_HAL_DISP_ST_QUERY_DMA_ATTR;
        stQueryCfg.stInCfg.u32CfgSize  = sizeof(MI_DISP_IMPL_MhalDmaAttrConfig_t);
        stQueryCfg.stInCfg.pInCfg      = pstDmaAttrCfg;

        bRet = _DRV_DISP_IF_ExecuteQuery(pDmaCtx, &stQueryCfg);
    }
    return bRet;
}

MI_U8 DRV_DISP_IF_DmaSetBufferAttr(void *pDmaCtx, MI_DISP_IMPL_MhalDmaBufferAttrConfig_t *pstDmaBufferAttrCfg)
{
    MI_U8                     bRet = 1;
    HAL_DISP_ST_QueryConfig_t stQueryCfg;

    if (DRV_DISP_CTX_SetCurCtx((DRV_DISP_CTX_Config_t *)pDmaCtx, ((DRV_DISP_CTX_Config_t *)pDmaCtx)->u32ContainIdx)
        == FALSE)
    {
        bRet = FALSE;
    }
    else
    {
        CamOsMemset(&stQueryCfg, 0, sizeof(HAL_DISP_ST_QueryConfig_t));

        stQueryCfg.stInCfg.enQueryType = E_HAL_DISP_ST_QUERY_DMA_BUFFERATTR;
        stQueryCfg.stInCfg.u32CfgSize  = sizeof(MI_DISP_IMPL_MhalDmaBufferAttrConfig_t);
        stQueryCfg.stInCfg.pInCfg      = pstDmaBufferAttrCfg;

        bRet = _DRV_DISP_IF_ExecuteQuery(pDmaCtx, &stQueryCfg);
    }
    return bRet;
}

MI_U8 DRV_DISP_IF_DmaGetHwCount(MI_U32 *pu32Count)
{
    MI_U8                      bRet = 1;
    HAL_DISP_ST_QueryConfig_t  stQueryCfg;
    HAL_DISP_ST_HwInfoConfig_t stHalHwInfoCfg;
    DRV_DISP_CTX_Config_t      stDispCtxCfg;

    HAL_DISP_IF_Init();

    CamOsMemset(&stQueryCfg, 0, sizeof(HAL_DISP_ST_QueryConfig_t));
    stQueryCfg.stInCfg.enQueryType = E_HAL_DISP_ST_QUERY_HW_INFO;
    stQueryCfg.stInCfg.u32CfgSize  = sizeof(HAL_DISP_ST_HwInfoConfig_t);
    stQueryCfg.stInCfg.pInCfg      = &stHalHwInfoCfg;

    CamOsMemset(&stHalHwInfoCfg, 0, sizeof(HAL_DISP_ST_HwInfoConfig_t));
    stHalHwInfoCfg.eType = E_HAL_DISP_ST_HW_INFO_DMA;

    if (_DRV_DISP_IF_ExecuteQuery(&stDispCtxCfg, &stQueryCfg))
    {
        *pu32Count = stHalHwInfoCfg.u32Count;
        bRet       = 1;
    }
    else
    {
        *pu32Count = 0;
        bRet       = 0;
    }
    return bRet;
}

MI_U8 DRV_DISP_IF_DmaGetCapabilityConfig(MI_U32 u32DmaId, MI_DISP_IMPL_MhalDmaCapabiliytConfig_t *pstDmaCapCfg)
{
    MI_U8                              bRet = 1;
    HAL_DISP_ST_QueryConfig_t          stQueryCfg;
    HAL_DISP_ST_DmaCapabilitytConfig_t stHalDmaCapCfg;
    DRV_DISP_CTX_Config_t              stDispCtxCfg;

    HAL_DISP_IF_Init();

    CamOsMemset(&stQueryCfg, 0, sizeof(HAL_DISP_ST_QueryConfig_t));
    stQueryCfg.stInCfg.enQueryType = E_HAL_DISP_ST_QUERY_DMA_CAPABILITY_GET;
    stQueryCfg.stInCfg.u32CfgSize  = sizeof(HAL_DISP_ST_DmaCapabilitytConfig_t);
    stQueryCfg.stInCfg.pInCfg      = &stHalDmaCapCfg;

    CamOsMemset(&stHalDmaCapCfg, 0, sizeof(HAL_DISP_ST_DmaCapabilitytConfig_t));
    stHalDmaCapCfg.u32DmaId = u32DmaId;

    if (_DRV_DISP_IF_ExecuteQuery(&stDispCtxCfg, &stQueryCfg))
    {
        CamOsMemcpy(pstDmaCapCfg, &stHalDmaCapCfg.stDmaCapCfg, sizeof(MI_DISP_IMPL_MhalDmaCapabiliytConfig_t));
        bRet = 1;
    }
    else
    {
        bRet = 0;
    }
    return bRet;
}

MI_U8 DRV_DISP_IF_ClkOn(void)
{
    MI_U8                       bRet = 1;
    HAL_DISP_ST_QueryConfig_t   stQueryCfg;
    HAL_DISP_ST_ClkInitConfig_t stHalClkInitCfg;
    DRV_DISP_CTX_Config_t       stDispCtxCfg;

    HAL_DISP_IF_Init();

    CamOsMemset(&stQueryCfg, 0, sizeof(HAL_DISP_ST_QueryConfig_t));
    stQueryCfg.stInCfg.enQueryType = E_HAL_DISP_ST_QUERY_CLK_INIT;
    stQueryCfg.stInCfg.u32CfgSize  = sizeof(HAL_DISP_ST_ClkInitConfig_t);
    stQueryCfg.stInCfg.pInCfg      = &stHalClkInitCfg;

    stHalClkInitCfg.bEn = 1;

    bRet = _DRV_DISP_IF_ExecuteQuery(&stDispCtxCfg, &stQueryCfg);
    return bRet;
}

MI_U8 DRV_DISP_IF_ClkOff(void)
{
    MI_U8                       bRet = 1;
    HAL_DISP_ST_QueryConfig_t   stQueryCfg;
    HAL_DISP_ST_ClkInitConfig_t stHalClkInitCfg;
    DRV_DISP_CTX_Config_t       stDispCtxCfg;

    HAL_DISP_IF_Init();

    CamOsMemset(&stQueryCfg, 0, sizeof(HAL_DISP_ST_QueryConfig_t));
    stQueryCfg.stInCfg.enQueryType = E_HAL_DISP_ST_QUERY_CLK_INIT;
    stQueryCfg.stInCfg.u32CfgSize  = sizeof(HAL_DISP_ST_ClkInitConfig_t);
    stQueryCfg.stInCfg.pInCfg      = &stHalClkInitCfg;

    stHalClkInitCfg.bEn = 0;

    bRet = _DRV_DISP_IF_ExecuteQuery(&stDispCtxCfg, &stQueryCfg);
    return bRet;
}

MI_U8 DRV_DISP_IF_Suspend(MI_DISP_IMPL_MhalPreSuspendConfig_t *pCfg)
{
    MI_U8                     bRet = 1;
    HAL_DISP_ST_QueryConfig_t stQueryCfg;
    DRV_DISP_CTX_Config_t     stDispCtxCfg;
    DRV_DISP_CTX_Config_t *   pstDispCtxCfg = &stDispCtxCfg;

    HAL_DISP_IF_Init();

    CamOsMemset(&stQueryCfg, 0, sizeof(HAL_DISP_ST_QueryConfig_t));
    CamOsMemset(pstDispCtxCfg, 0, sizeof(DRV_DISP_CTX_Config_t));
    stDispCtxCfg.pstCtxContain = g_stDispCtxContainTbl;
    if (pCfg->enPreSuspendType == E_MI_DISP_PRESUSPEND_GOING || pCfg->enPreSuspendType == E_MI_DISP_PRESUSPEND_REPENT)
    {
        stDispCtxCfg.pstCtxContain->stStr.enPreSuspendType = pCfg->enPreSuspendType;

        if (pCfg->enPreSuspendType == E_MI_DISP_PRESUSPEND_GOING)
        {
            pstDispCtxCfg->pstCtxContain->stStr.u8bSuspendEn = 1;
            bRet                                             = _DRV_DISP_IF_Presuspend(pstDispCtxCfg);
        }
        if (pCfg->enPreSuspendType == E_MI_DISP_PRESUSPEND_REPENT && !DISP_IN_STR_STATUS(pstDispCtxCfg))
        {
            DISP_ERR("%s %d, Repent W/O PreSuspend\n", __FUNCTION__, __LINE__);
            return 0;
        }
        if (bRet)
        {
            stDispCtxCfg.pstCtxContain->stStr.enPreSuspendType = E_MI_DISP_PRESUSPEND_DONE;
        }
        if (pCfg->enPreSuspendType == E_MI_DISP_PRESUSPEND_REPENT)
        {
            stDispCtxCfg.pstCtxContain->stStr.u8bSuspendEn     = 0;
            stDispCtxCfg.pstCtxContain->stStr.enPreSuspendType = E_MI_DISP_PRESUSPEND_IDLE;
            DRV_DISP_DEBUG_Str(pstDispCtxCfg, 1);
        }
    }
    if (pCfg->enPreSuspendType == E_MI_DISP_PRESUSPEND_DONE)
    {
        if (!DISP_IN_STR_STATUS(pstDispCtxCfg)
            || stDispCtxCfg.pstCtxContain->stStr.enPreSuspendType != E_MI_DISP_PRESUSPEND_DONE)
        {
            DISP_ERR("%s %d, Suspend W/O PreSuspend\n", __FUNCTION__, __LINE__);
            return 0;
        }
        stQueryCfg.stInCfg.enQueryType = E_HAL_DISP_ST_QUERY_SUSPEND;
        stQueryCfg.stInCfg.u32CfgSize  = sizeof(HAL_DISP_ST_StrConfig_t);
        stQueryCfg.stInCfg.pInCfg      = &stDispCtxCfg.pstCtxContain->stStr;
        bRet                           = _DRV_DISP_IF_ExecuteQuery(pstDispCtxCfg, &stQueryCfg);
        bRet                           = DRV_DISP_IRQ_Str(pstDispCtxCfg, 0);
        if (!_DRV_DISP_IF_SetClkOff(pstDispCtxCfg))
        {
            bRet = 0;
            DISP_ERR("%s %d, Set Clk Off Fail\n", __FUNCTION__, __LINE__);
        }
        if (bRet)
        {
            stDispCtxCfg.pstCtxContain->stStr.enPreSuspendType = E_MI_DISP_PRESUSPEND_IDLE;
        }
    }
    return bRet;
}

MI_U8 DRV_DISP_IF_Resume(void)
{
    MI_U8                     bRet = 1;
    HAL_DISP_ST_QueryConfig_t stQueryCfg;
    DRV_DISP_CTX_Config_t     stDispCtxCfg;

    HAL_DISP_IF_Init();

    CamOsMemset(&stQueryCfg, 0, sizeof(HAL_DISP_ST_QueryConfig_t));
    CamOsMemset(&stDispCtxCfg, 0, sizeof(DRV_DISP_CTX_Config_t));
    stDispCtxCfg.pstCtxContain = g_stDispCtxContainTbl;

    if (_DRV_DISP_IF_SetClkOn((void *)&stDispCtxCfg))
    {
        stQueryCfg.stInCfg.enQueryType = E_HAL_DISP_ST_QUERY_RESUME;
        stQueryCfg.stInCfg.u32CfgSize  = sizeof(HAL_DISP_ST_StrConfig_t);
        stQueryCfg.stInCfg.pInCfg      = &stDispCtxCfg.pstCtxContain->stStr;
        bRet                           = _DRV_DISP_IF_ExecuteQuery(&stDispCtxCfg, &stQueryCfg);
        DRV_DISP_IRQ_Str(&stDispCtxCfg, 1);
        DRV_DISP_DEBUG_Str(&stDispCtxCfg, 1);
    }
    stDispCtxCfg.pstCtxContain->stStr.u8bSuspendEn = 0;
    return bRet;
}

MI_U8 DRV_DISP_IF_SetPnlUnifiedParamConfig(void *pDevCtx, MI_DISP_IMPL_MhalPnlUnifiedParamConfig_t *pUdParamCfg)
{
    MI_U8                     bRet = 1;
    HAL_DISP_ST_QueryConfig_t stQueryCfg;

    if (DRV_DISP_CTX_SetCurCtx((DRV_DISP_CTX_Config_t *)pDevCtx, ((DRV_DISP_CTX_Config_t *)pDevCtx)->u32ContainIdx)
        == FALSE)
    {
        bRet = FALSE;
    }
    else
    {
        CamOsMemset(&stQueryCfg, 0, sizeof(HAL_DISP_ST_QueryConfig_t));

        stQueryCfg.stInCfg.enQueryType = E_HAL_DISP_ST_QUERY_PNL_UNIFIED_PARAM_SET;
        stQueryCfg.stInCfg.u32CfgSize  = sizeof(MI_DISP_IMPL_MhalPnlUnifiedParamConfig_t);
        stQueryCfg.stInCfg.pInCfg      = pUdParamCfg;

        bRet = _DRV_DISP_IF_ExecuteQuery(pDevCtx, &stQueryCfg);
    }
    return bRet;
}

MI_U8 DRV_DISP_IF_GetPnlUnifiedParamConfig(void *pDevCtx, MI_DISP_IMPL_MhalPnlUnifiedParamConfig_t *pUdParamCfg)
{
    MI_U8                     bRet = 1;
    HAL_DISP_ST_QueryConfig_t stQueryCfg;

    if (DRV_DISP_CTX_SetCurCtx((DRV_DISP_CTX_Config_t *)pDevCtx, ((DRV_DISP_CTX_Config_t *)pDevCtx)->u32ContainIdx)
        == FALSE)
    {
        bRet = FALSE;
    }
    else
    {
        CamOsMemset(&stQueryCfg, 0, sizeof(HAL_DISP_ST_QueryConfig_t));

        stQueryCfg.stInCfg.enQueryType = E_HAL_DISP_ST_QUERY_PNL_UNIFIED_PARAM_GET;
        stQueryCfg.stInCfg.u32CfgSize  = sizeof(MI_DISP_IMPL_MhalPnlUnifiedParamConfig_t);
        stQueryCfg.stInCfg.pInCfg      = pUdParamCfg;

        if (_DRV_DISP_IF_ExecuteQuery(pDevCtx, &stQueryCfg))
        {
        }
        else
        {
            bRet = 0;
        }
    }
    return bRet;
}

MI_U8 DRV_DISP_IF_SetPnlPowerConfig(void *pDevCtx, MI_DISP_IMPL_MhalPnlPowerConfig_t *pstPowerCfg)
{
    MI_U8                     bRet = 1;
    HAL_DISP_ST_QueryConfig_t stQueryCfg;

    if (DRV_DISP_CTX_SetCurCtx((DRV_DISP_CTX_Config_t *)pDevCtx, ((DRV_DISP_CTX_Config_t *)pDevCtx)->u32ContainIdx)
        == FALSE)
    {
        bRet = FALSE;
    }
    else
    {
        CamOsMemset(&stQueryCfg, 0, sizeof(HAL_DISP_ST_QueryConfig_t));

        stQueryCfg.stInCfg.enQueryType = E_HAL_DISP_ST_QUERY_PNL_POWER_SET;
        stQueryCfg.stInCfg.u32CfgSize  = sizeof(MI_DISP_IMPL_MhalPnlPowerConfig_t);
        stQueryCfg.stInCfg.pInCfg      = pstPowerCfg;

        bRet = _DRV_DISP_IF_ExecuteQuery(pDevCtx, &stQueryCfg);
    }
    return bRet;
}

MI_U8 DRV_DISP_IF_SetPnlMipiDsiWriteCmdConfig(void *                                       pDevCtx,
                                              MI_DISP_IMPL_MhalPnlMipiDsiWriteCmdConfig_t *pstMipiDsiWriteCmdCfg)
{
    MI_U8                     bRet = 1;
    HAL_DISP_ST_QueryConfig_t stQueryCfg;

    if (DRV_DISP_CTX_SetCurCtx((DRV_DISP_CTX_Config_t *)pDevCtx, ((DRV_DISP_CTX_Config_t *)pDevCtx)->u32ContainIdx)
        == FALSE)
    {
        bRet = FALSE;
    }
    else
    {
        CamOsMemset(&stQueryCfg, 0, sizeof(HAL_DISP_ST_QueryConfig_t));

        stQueryCfg.stInCfg.enQueryType = E_HAL_DISP_ST_QUERY_PNL_MIPIDSI_CMD_WRITE;
        stQueryCfg.stInCfg.u32CfgSize  = sizeof(MI_DISP_IMPL_MhalPnlMipiDsiWriteCmdConfig_t);
        stQueryCfg.stInCfg.pInCfg      = pstMipiDsiWriteCmdCfg;

        bRet = _DRV_DISP_IF_ExecuteQuery(pDevCtx, &stQueryCfg);
    }
    return bRet;
}

MI_U8 DRV_DISP_IF_SetPnlMipiDsiReadCmdConfig(void *                                      pDevCtx,
                                             MI_DISP_IMPL_MhalPnlMipiDsiReadCmdConfig_t *pstMipiDsiReadCmdCfg)
{
    MI_U8                     bRet = 1;
    HAL_DISP_ST_QueryConfig_t stQueryCfg;

    if (DRV_DISP_CTX_SetCurCtx((DRV_DISP_CTX_Config_t *)pDevCtx, ((DRV_DISP_CTX_Config_t *)pDevCtx)->u32ContainIdx)
        == FALSE)
    {
        bRet = FALSE;
    }
    else
    {
        CamOsMemset(&stQueryCfg, 0, sizeof(HAL_DISP_ST_QueryConfig_t));

        stQueryCfg.stInCfg.enQueryType = E_HAL_DISP_ST_QUERY_PNL_MIPIDSI_CMD_READ;
        stQueryCfg.stInCfg.u32CfgSize  = sizeof(MI_DISP_IMPL_MhalPnlMipiDsiReadCmdConfig_t);
        stQueryCfg.stInCfg.pInCfg      = pstMipiDsiReadCmdCfg;

        bRet = _DRV_DISP_IF_ExecuteQuery(pDevCtx, &stQueryCfg);
    }
    return bRet;
}

MI_U8 DRV_DISP_IF_GetFpllLockStatus(void *pDevCtx, MI_U8 *pu8LockStatus)
{
    MI_U8                     bRet = 1;
    MI_U8                     u8LockStatus;
    HAL_DISP_ST_QueryConfig_t stQueryCfg;

    HAL_DISP_IF_Init();

    CamOsMemset(&stQueryCfg, 0, sizeof(HAL_DISP_ST_QueryConfig_t));

    stQueryCfg.stInCfg.enQueryType = E_HAL_DISP_ST_QUERY_FPLL_LOCK_STATUS_GET;
    stQueryCfg.stInCfg.u32CfgSize  = sizeof(MI_U8);
    stQueryCfg.stInCfg.pInCfg      = &u8LockStatus;

    if (_DRV_DISP_IF_ExecuteQuery(pDevCtx, &stQueryCfg))
    {
        *pu8LockStatus = u8LockStatus;
    }
    else
    {
        bRet = 0;
        DISP_ERR("%s %d Get Fpll lock status Fail\n", __FUNCTION__, __LINE__);
    }

    return bRet;
}

//-------------------------------------------------------------------------------------------------
