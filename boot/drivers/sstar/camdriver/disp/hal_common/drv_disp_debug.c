/*
 * drv_disp_debug.c- Sigmastar
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

#define _DISP_DEBUG_C_
//-------------------------------------------------------------------------------------------------
//  Include Files
//-------------------------------------------------------------------------------------------------
#include "mi_common_datatype.h"
#include "drv_disp_os.h"
#include "hal_disp_include.h"
#include "disp_debug.h"

#include "mhal_cmdq.h"
#include "drv_disp_if.h"
#include "drv_disp_ctx.h"

//-------------------------------------------------------------------------------------------------
//  Defines & Macro
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
//  structure
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//  Variable
//-------------------------------------------------------------------------------------------------
extern MI_DISP_IMPL_MhalRegAccessType_e g_eDispRegAccessMode[HAL_DISP_UTILITY_CMDQ_NUM];

//-------------------------------------------------------------------------------------------------
//  Local Functions
//-------------------------------------------------------------------------------------------------
DRV_DISP_DEBUG_ProcfsOps_t g_stDispProcfsApi = {
    .SetClk            = DRV_DISP_DEBUG_SetClk,
    .GetClk            = DRV_DISP_DEBUG_GetClk,
    .SetDbgmgFlag      = DRV_DISP_DEBUG_SetDbgmgFlag,
    .GetDbgmgFlag      = DRV_DISP_DEBUG_GetDbgmgFlag,
    .SetFunc           = DRV_DISP_DEBUG_SetFunc,
    .GetFunc           = DRV_DISP_DEBUG_GetFunc,
    .SetTurnDrv        = DRV_DISP_DEBUG_SetTurnDrv,
    .GetTurnDrv        = DRV_DISP_DEBUG_GetTurnDrv,
    .GetIrqHist        = DRV_DISP_DEBUG_GetIrqHist,
    .GetDeviceInstance = DRV_DISP_DEBUG_GetDeviceInstance,
};

MI_BOOL DRV_DISP_DEBUG_GetHalProcfsOps(DRV_DISP_DEBUG_ProcfsOps_t **ppProcfsOps)
{
    *ppProcfsOps = &g_stDispProcfsApi;

    return TRUE;
}

//#if DISP_STATISTIC_EN
#if 0
void DRV_DISP_DEBUG_InPortStore(DRV_DISP_OS_StrConfig_t *pstStringCfg)
{
    DRV_DISP_CTX_Config_t stDispCtx;
    DRV_DISP_CTX_Config_t *pDevCtx;
    DRV_DISP_CTX_InputPortContain_t *pstInputPortContain;
    MI_DISP_IMPL_MhalRegFlipConfig_t stRegFlipCfg;
    MI_DISP_InputPortAttr_t stAttr;
    MI_U8 u8bSet = 0;
    MI_U32 i, j, k;
    MI_U32 u32Dev = 0;
    MI_U32 u32Vid = 0;
    MI_U32 u32PortId = 0;
    MI_U32 u32bEn = 0;
    MI_U32 u32x = 0;
    MI_U32 u32y = 0;
    MI_U32 u32w = 0;
    MI_U32 u32h = 0;

    HAL_DISP_IF_Init();

    if (pstStringCfg->argc == 5 && DRV_DISP_OS_StrCmp(pstStringCfg->argv[0], "en") == 0)
    {
        DRV_DISP_OS_StrToL(pstStringCfg->argv[1], 16, &u32Dev);
        DRV_DISP_OS_StrToL(pstStringCfg->argv[2], 16, &u32Vid);
        DRV_DISP_OS_StrToL(pstStringCfg->argv[3], 16, &u32PortId);
        DRV_DISP_OS_StrToL(pstStringCfg->argv[4], 16, &u32bEn);
    }
    else if (pstStringCfg->argc == 8 &&
        ((DRV_DISP_OS_StrCmp(pstStringCfg->argv[0], "disp") == 0)||
        (DRV_DISP_OS_StrCmp(pstStringCfg->argv[0], "crop") == 0)))
    {
        DRV_DISP_OS_StrToL(pstStringCfg->argv[1], 16, &u32Dev);
        DRV_DISP_OS_StrToL(pstStringCfg->argv[2], 16, &u32Vid);
        DRV_DISP_OS_StrToL(pstStringCfg->argv[3], 16, &u32PortId);
        DRV_DISP_OS_StrToL(pstStringCfg->argv[4], 10, &u32x);
        DRV_DISP_OS_StrToL(pstStringCfg->argv[5], 10, &u32y);
        DRV_DISP_OS_StrToL(pstStringCfg->argv[6], 10, &u32w);
        DRV_DISP_OS_StrToL(pstStringCfg->argv[7], 10, &u32h);
        stAttr.stDispWin.u16X = u32x;
        stAttr.stDispWin.u16Y = u32y;
        stAttr.stDispWin.u16Width = u32w;
        stAttr.stDispWin.u16Height = u32h;
    }
    else
    {
        DISP_MSG("en [dev] [vid] [port] [val]\n");
        DISP_MSG("disp [dev] [vid] [port] [x] [y] [w] [h]\n");
        DISP_MSG("crop [dev] [vid] [port] [x] [y] [w] [h]\n");
        return;
    }

    DRV_DISP_IF_DeviceGetInstance(HAL_DISP_MAPPING_DEVICEID_FROM_MI(0), (void **)&pDevCtx);
    if (pDevCtx && pDevCtx->pstCtxContain)
    {
        for (i = 0; i < HAL_DISP_VIDLAYER_MAX; i++)
        {
            if (pDevCtx->pstCtxContain->bVidLayerContainUsed[i])
            {
                if(pDevCtx->pstCtxContain->pstVidLayerContain[i] &&
                    pDevCtx->pstCtxContain->pstVidLayerContain[i]->u32BindDevId == u32Dev &&
                    pDevCtx->pstCtxContain->pstVidLayerContain[i]->u32VidLayerTypeId == u32Vid)
                {
                    for (j = 0; j < HAL_DISP_INPUTPORT_NUM; j++)
                    {
                        pstInputPortContain = pDevCtx->pstCtxContain->pstVidLayerContain[i]->pstInputPortContain[j];
                        if(pstInputPortContain &&
                            pstInputPortContain->bUsedInputPort &&
                            pstInputPortContain->u32PortId == u32PortId)
                        {
                            for (k = 0; k < HAL_DISP_INPUTPORT_MAX; k++)
                            {
                                if(pstInputPortContain == pDevCtx->pstCtxContain->pstInputPortContain[k])
                                {
                                    stDispCtx.enCtxType = E_DRV_DISP_CTX_TYPE_INPUTPORT;
                                    stDispCtx.pstCtxContain = pDevCtx->pstCtxContain;
                                    stDispCtx.u32ContainIdx = k;
                                    stDispCtx.u32CtxIdx = 0;
                                    stAttr.u16SrcHeight = pstInputPortContain->stAttr.u16SrcHeight;
                                    stAttr.u16SrcWidth = pstInputPortContain->stAttr.u16SrcWidth;
                                    u8bSet = 1;
                                    break;
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    if(u8bSet)
    {
        if(DRV_DISP_OS_StrCmp(pstStringCfg->argv[0], "en") == 0)
        {
            DRV_DISP_IF_InputPortEnable(&stDispCtx, u32bEn);
        }
        else if(DRV_DISP_OS_StrCmp(pstStringCfg->argv[0], "disp") == 0)
        {
            DRV_DISP_IF_InputPortSetAttr(&stDispCtx, &stAttr);
        }
        else if(DRV_DISP_OS_StrCmp(pstStringCfg->argv[0], "crop") == 0)
        {
            DRV_DISP_IF_InputPortSetCropAttr(&stDispCtx, &stAttr.stDispWin);
        }
        stRegFlipCfg.bEnable = 1;
        stRegFlipCfg.pCmdqInf = NULL;
        DRV_DISP_IF_SetRegFlipConfig(pDevCtx, &stRegFlipCfg);
    }
    else
    {
        DISP_MSG("en [dev] [vid] [port] [val]\n");
        DISP_MSG("disp [dev] [vid] [port] [x] [y] [w] [h]\n");
        DISP_MSG("crop [dev] [vid] [port] [x] [y] [w] [h]\n");
    }
}
MI_U32 DRV_DISP_DEBUG_InPortShow(MI_U8 *p8DstBuf)
{
    MI_U8 *                       p8SrcBuf = p8DstBuf;
    DRV_DISP_CTX_Config_t *       pDevCtx;
    DRV_DISP_CTX_DeviceContain_t *pDevCon;
    MI_U8 i;
    MI_U8 bDone = 1;
    static MI_U8                  u8Dev = 0;

    DRV_DISP_IF_DeviceGetInstance(HAL_DISP_MAPPING_DEVICEID_FROM_MI(0), (void **)&pDevCtx);
    if (pDevCtx && pDevCtx->pstCtxContain)
    {
        p8DstBuf += DISPDEBUG_SPRINTF(p8DstBuf, PAGE_SIZE, "-------DISP INPORT CFG -------\n");
        for (i = 0; i < HAL_DISP_DEVICE_MAX; i++)
        {
            if (pDevCtx->pstCtxContain->bDevContainUsed[i])
            {
                pDevCon = pDevCtx->pstCtxContain->pstDevContain[i];
                if(pDevCon && pDevCon->u32DevId == u8Dev)
                {
                    p8DstBuf += DISPDEBUG_SPRINTF(p8DstBuf, PAGE_SIZE, "===========DISP CFG Con %hhd DEV%d BindVid:%x===========\n",
                        i, pDevCon->u32DevId, pDevCon->eBindVideoLayer);
                    p8DstBuf += HalDispIpIfInportShow(p8DstBuf, pDevCon, &bDone, p8SrcBuf);
                    p8DstBuf += DISPDEBUG_SPRINTF(p8DstBuf, PAGE_SIZE, "===========DISP INPORT CFG ===========\n\n");
                    break;
                }
            }
            if(bDone == 0)
            {
                break;
            }
        }
        if(bDone)
        {
            u8Dev++;
            if(u8Dev == HAL_DISP_DEVICE_MAX)
            {
                u8Dev = 0;
            }
        }
    }
    return (MI_U32)(p8DstBuf - p8SrcBuf);
}
void DRV_DISP_DEBUG_VidLayerStore(DRV_DISP_OS_StrConfig_t *pstStringCfg)
{
    DRV_DISP_CTX_Config_t stDispCtx;
    DRV_DISP_CTX_Config_t *pDevCtx;
    MI_DISP_IMPL_MhalRegFlipConfig_t stRegFlipCfg;
    MI_U32 i;
    MI_U32 u32Dev = 0;
    MI_U32 u32Vid = 0;
    MI_U32 u32Val = 0;

    HAL_DISP_IF_Init();

    if (pstStringCfg->argc == 4 &&
        ((DRV_DISP_OS_StrCmp(pstStringCfg->argv[0], "en") == 0) ||
        (DRV_DISP_OS_StrCmp(pstStringCfg->argv[0], "pri") == 0)) )
    {
        DRV_DISP_OS_StrToL(pstStringCfg->argv[1], 16, &u32Dev);
        DRV_DISP_OS_StrToL(pstStringCfg->argv[2], 16, &u32Vid);
        DRV_DISP_OS_StrToL(pstStringCfg->argv[3], 16, &u32Val);
    }
    else
    {
        DISP_MSG("en [dev] [vid] [val]\n");
        DISP_MSG("pri [dev] [vid] [val]\n");
        return;
    }

    DRV_DISP_IF_DeviceGetInstance(HAL_DISP_MAPPING_DEVICEID_FROM_MI(0), (void **)&pDevCtx);
    if (pDevCtx && pDevCtx->pstCtxContain)
    {
        for (i = 0; i < HAL_DISP_VIDLAYER_MAX; i++)
        {
            if (pDevCtx->pstCtxContain->bVidLayerContainUsed[i])
            {
                if(pDevCtx->pstCtxContain->pstVidLayerContain[i] &&
                    pDevCtx->pstCtxContain->pstVidLayerContain[i]->u32BindDevId == u32Dev &&
                    pDevCtx->pstCtxContain->pstVidLayerContain[i]->u32VidLayerTypeId == u32Vid)
                {
                    stDispCtx.enCtxType = E_DRV_DISP_CTX_TYPE_VIDLAYER;
                    stDispCtx.pstCtxContain = pDevCtx->pstCtxContain;
                    stDispCtx.u32ContainIdx = i;
                    stDispCtx.u32CtxIdx = 0;
                }
            }
        }
    }
    if((DRV_DISP_OS_StrCmp(pstStringCfg->argv[0], "en") == 0))
    {
        DRV_DISP_IF_VideoLayerEnable(&stDispCtx, u32Val);
    }
    else if((DRV_DISP_OS_StrCmp(pstStringCfg->argv[0], "pri") == 0))
    {
        DRV_DISP_IF_VideoLayerSetPriority(&stDispCtx, u32Val);
    }
    stRegFlipCfg.bEnable = 1;
    stRegFlipCfg.pCmdqInf = NULL;
    DRV_DISP_IF_SetRegFlipConfig(pDevCtx, &stRegFlipCfg);
}
MI_U32 DRV_DISP_DEBUG_VidLayerShow(MI_U8 *p8DstBuf)
{
    MI_U8 *                       p8SrcBuf = p8DstBuf;
    DRV_DISP_CTX_Config_t *       pDevCtx;
    DRV_DISP_CTX_DeviceContain_t *pDevCon;
    MI_U8                         i;

    DRV_DISP_IF_DeviceGetInstance(HAL_DISP_MAPPING_DEVICEID_FROM_MI(0), (void **)&pDevCtx);
    if (pDevCtx && pDevCtx->pstCtxContain)
    {
        p8DstBuf += DISPDEBUG_SPRINTF(p8DstBuf, PAGE_SIZE, "-------DISP VIDLAYER CFG -------\n");
        for (i = 0; i < HAL_DISP_DEVICE_MAX; i++)
        {
            if (pDevCtx->pstCtxContain->bDevContainUsed[i])
            {
                pDevCon = pDevCtx->pstCtxContain->pstDevContain[i];
                if(pDevCon)
                {
                    p8DstBuf += DISPDEBUG_SPRINTF(p8DstBuf, PAGE_SIZE, "===========DISP CFG Con %hhd DEV%d BindVid:%x=========\n",
                        i, pDevCon->u32DevId, pDevCon->eBindVideoLayer);
                    p8DstBuf += HalDispIpIfVidLayerShow(p8DstBuf, pDevCon);
                    p8DstBuf += DISPDEBUG_SPRINTF(p8DstBuf, PAGE_SIZE, "===========DISP VIDLAYER CFG ===========\n\n");
                }
            }
        }
    }
    return (MI_U32)(p8DstBuf - p8SrcBuf);
}
#endif

MI_U8 DRV_DISP_DEBUG_SetClk(MI_U8 *p8ClkName, MI_U32 u32Enable, MI_U32 u32ClkRate, MI_U32 *pu32ClkRate)
{
    MI_U32                u32idx;
    DRV_DISP_CTX_Config_t stDispCtx;
    MI_U8 *               pbClkEn = NULL;
    MI_U32                u32i;
    MI_U8                 bRet = TRUE;

    if (E_HAL_DISP_CLK_NUM)
    {
        pbClkEn = CamOsMemAlloc(sizeof(MI_U8) * E_HAL_DISP_CLK_NUM);

        if (DRV_DISP_OS_StrCmp(p8ClkName, "clktree") == 0)
        {
            if (u32Enable)
            {
                for (u32i = 0; u32i < E_HAL_DISP_CLK_NUM; u32i++)
                {
                    pbClkEn[u32i] = HAL_DISP_CLK_GET_OFF_SETTING(u32i);
                }

                if (DRV_DISP_OS_SetClkOn(pbClkEn, pu32ClkRate, E_HAL_DISP_CLK_NUM) == 0)
                {
                    DISP_ERR("%s %d, Set Clk On Fail\n", __FUNCTION__, __LINE__);
                    bRet = FALSE;
                }
            }
            else
            {
                for (u32i = 0; u32i < E_HAL_DISP_CLK_NUM; u32i++)
                {
                    pbClkEn[u32i] = HAL_DISP_CLK_GET_ON_SETTING(u32i);
                }
                if (DRV_DISP_OS_SetClkOff(pbClkEn, pu32ClkRate, E_HAL_DISP_CLK_NUM) == 0)
                {
                    DISP_ERR("%s %d, Set Clk Off Fail\n", __FUNCTION__, __LINE__);
                    bRet = FALSE;
                }
            }
        }
        else
        {
            if (DRV_DISP_IF_GetClk((void *)&stDispCtx, pbClkEn, pu32ClkRate, E_HAL_DISP_CLK_NUM) == 0)
            {
                DISP_ERR("%s %d, Get Clk Fail\n", __FUNCTION__, __LINE__);
                bRet = FALSE;
            }
            else
            {
                for (u32idx = 0; u32idx < E_HAL_DISP_CLK_NUM; u32idx++)
                {
                    if (DRV_DISP_OS_StrCmp(p8ClkName, HAL_DISP_CLK_GET_NAME(u32idx)) == 0)
                    {
                        pbClkEn[u32idx] = u32Enable, pu32ClkRate[u32idx] = u32ClkRate;
                        if (DRV_DISP_IF_SetClk((void *)&stDispCtx, pbClkEn, pu32ClkRate, E_HAL_DISP_CLK_NUM) == 0)
                        {
                            DISP_ERR("%s %d, Set Clk Fail\n", __FUNCTION__, __LINE__);
                            bRet = FALSE;
                        }
                        break;
                    }
                }

                if (u32idx == E_HAL_DISP_CLK_NUM)
                {
                    DISP_ERR("%s %d, p8ClkName (%s) is not correct\n", __FUNCTION__, __LINE__, p8ClkName);
                    bRet = FALSE;
                }
            }
        }

        if (pbClkEn)
        {
            CamOsMemRelease(pbClkEn);
        }
    }
    return bRet;
}

MI_U8 DRV_DISP_DEBUG_GetClk(MI_U8 *pbClkEn, MI_U32 *pu32ClkRate)
{
    DRV_DISP_CTX_Config_t stDispCtx;
    MI_U8                 bRet = TRUE;

    if (DRV_DISP_IF_GetClk((void *)&stDispCtx, pbClkEn, pu32ClkRate, E_HAL_DISP_CLK_NUM) == 0)
    {
        DISP_ERR("%s %d, Get Clk Fail\n", __FUNCTION__, __LINE__);
        bRet = FALSE;
    }

    return bRet;
}

MI_U8 DRV_DISP_DEBUG_SetDbgmgFlag(MI_U32 u32Level)
{
    MI_U8 bRet = TRUE;

    DRV_DISP_IF_SetDbgLevel((void *)&u32Level);

    return bRet;
}

MI_U8 DRV_DISP_DEBUG_GetDbgmgFlag(void *m)
{
    MI_U8                     bRet = TRUE;
    MI_U8                     u8idx;
    void *                    pDevCtx;
    HAL_DISP_ST_QueryConfig_t stQueryCfg;
    HAL_DISP_ST_DbgmgConfig_t stDbgmgCfg;

    for (u8idx = 0; u8idx < HAL_DISP_DEVICE_MAX; u8idx++)
    {
        DRV_DISP_IF_DeviceGetInstance(HAL_DISP_MAPPING_DEVICEID_FROM_MI(u8idx), &pDevCtx);
        if (pDevCtx)
        {
            CamOsMemset(&stQueryCfg, 0, sizeof(HAL_DISP_ST_QueryConfig_t));
            CamOsMemset(&stDbgmgCfg, 0, sizeof(HAL_DISP_ST_DbgmgConfig_t));

            stQueryCfg.stInCfg.enQueryType = E_HAL_DISP_ST_QUERY_DBGMG_GET;
            stQueryCfg.stInCfg.u32CfgSize  = sizeof(HAL_DISP_ST_DbgmgConfig_t);
            stQueryCfg.stInCfg.pInCfg      = (void *)&stDbgmgCfg;

            stDbgmgCfg.pData = m;

            if (HAL_DISP_IF_Query(pDevCtx, &stQueryCfg))
            {
                if (stQueryCfg.stOutCfg.pSetFunc)
                {
                    stQueryCfg.stOutCfg.pSetFunc(pDevCtx, stQueryCfg.stInCfg.pInCfg);
                }
            }
        }
    }

    return bRet;
}

MI_U8 DRV_DISP_DEBUG_SetFunc(MI_U8 *p8FuncType, MI_U32 u32val, MI_U32 u32DevId)
{
    MI_DISP_IMPL_MhalDeviceConfig_t stDevCfg;
    MI_U8                           bRet = TRUE;

    CamOsMemset(&stDevCfg, 0, sizeof(MI_DISP_IMPL_MhalDeviceConfig_t));
    if (!HAL_DISP_IF_ParseFunc(p8FuncType, &stDevCfg, u32val))
    {
        DRV_DISP_IF_SetDeviceConfig(u32DevId, &stDevCfg);
    }

    return bRet;
}

MI_U8 DRV_DISP_DEBUG_GetFunc(MI_U8 u8DevIdx, void *pstDevCfg)
{
    MI_U8 bRet = TRUE;

    bRet = DRV_DISP_IF_GetDeviceConfig(u8DevIdx, (MI_DISP_IMPL_MhalDeviceConfig_t *)pstDevCfg);

    return bRet;
}

MI_U8 DRV_DISP_DEBUG_SetTurnDrv(MI_U32 u32Argc, MI_U8 *p8TrimName, MI_U16 *pu16Trim)
{
    DRV_DISP_CTX_Config_t          stDispCtx;
    HAL_DISP_ST_QueryConfig_t      stQueryCfg;
    HAL_DISP_ST_DrvTurningConfig_t stDriTrimCfg;
    MI_U8                          bRet = TRUE;

    CamOsMemset(&stDriTrimCfg, 0, sizeof(HAL_DISP_ST_DrvTurningConfig_t));
    CamOsMemset(&stQueryCfg, 0, sizeof(HAL_DISP_ST_QueryConfig_t));
    HAL_DISP_IF_Init();

    if (DRV_DISP_OS_StrCmp(p8TrimName, "vga_drv") == 0)
    {
        stDriTrimCfg.enType = E_HAL_DISP_ST_DRV_TURNING_RGB;
        if (u32Argc == 2)
        {
            pu16Trim[1] = pu16Trim[0];
            pu16Trim[2] = pu16Trim[0];
        }
    }
    else if (DRV_DISP_OS_StrCmp(p8TrimName, "cvbs_drv") == 0)
    {
        stQueryCfg.stInCfg.enQueryType = E_HAL_DISP_ST_QUERY_DRVTURNING_GET;
        stQueryCfg.stInCfg.u32CfgSize  = sizeof(HAL_DISP_ST_DrvTurningConfig_t);
        stQueryCfg.stInCfg.pInCfg      = (void *)&stDriTrimCfg;
        stDriTrimCfg.enType            = E_HAL_DISP_ST_DRV_TURNING_CVBS;
        if (HAL_DISP_IF_Query((void *)&stDispCtx, &stQueryCfg))
        {
            if (stQueryCfg.stOutCfg.pSetFunc)
            {
                stQueryCfg.stOutCfg.pSetFunc((void *)&stDispCtx, stQueryCfg.stInCfg.pInCfg);
            }
        }
    }

    stQueryCfg.stInCfg.enQueryType = E_HAL_DISP_ST_QUERY_DRVTURNING_SET;
    stQueryCfg.stInCfg.u32CfgSize  = sizeof(HAL_DISP_ST_DrvTurningConfig_t);
    stQueryCfg.stInCfg.pInCfg      = (void *)&stDriTrimCfg;

    if (HAL_DISP_IF_Query((void *)&stDispCtx, &stQueryCfg))
    {
        if (stQueryCfg.stOutCfg.pSetFunc)
        {
            stQueryCfg.stOutCfg.pSetFunc((void *)&stDispCtx, stQueryCfg.stInCfg.pInCfg);
        }
    }

    return bRet;
}

MI_U8 DRV_DISP_DEBUG_GetTurnDrv(MI_U16 *pu16VgaTrim, MI_U16 *pu16CvbsTrim)
{
    DRV_DISP_CTX_Config_t          stDispCtx;
    HAL_DISP_ST_QueryConfig_t      stQueryCfg;
    HAL_DISP_ST_DrvTurningConfig_t stDrivTrimCfg;
    MI_U8                          bRet = TRUE;

    CamOsMemset(&stDrivTrimCfg, 0, sizeof(HAL_DISP_ST_DrvTurningConfig_t));
    CamOsMemset(&stQueryCfg, 0, sizeof(HAL_DISP_ST_QueryConfig_t));
    HAL_DISP_IF_Init();

    stQueryCfg.stInCfg.enQueryType = E_HAL_DISP_ST_QUERY_DRVTURNING_GET;
    stQueryCfg.stInCfg.u32CfgSize  = sizeof(HAL_DISP_ST_DrvTurningConfig_t);
    stQueryCfg.stInCfg.pInCfg      = (void *)&stDrivTrimCfg;

    stDrivTrimCfg.enType = E_HAL_DISP_ST_DRV_TURNING_RGB;
    if (HAL_DISP_IF_Query((void *)&stDispCtx, &stQueryCfg))
    {
        if (stQueryCfg.stOutCfg.pSetFunc)
        {
            stQueryCfg.stOutCfg.pSetFunc((void *)&stDispCtx, stQueryCfg.stInCfg.pInCfg);
        }
    }

    CamOsMemset(&stDrivTrimCfg, 0, sizeof(HAL_DISP_ST_DrvTurningConfig_t));

    stDrivTrimCfg.enType = E_HAL_DISP_ST_DRV_TURNING_CVBS;
    if (HAL_DISP_IF_Query((void *)&stDispCtx, &stQueryCfg))
    {
        if (stQueryCfg.stOutCfg.pSetFunc)
        {
            stQueryCfg.stOutCfg.pSetFunc((void *)&stDispCtx, stQueryCfg.stInCfg.pInCfg);
        }
    }

    return bRet;
}

#if DISP_SYSFS_PQ_EN
void DRV_DISP_DEBUG_SetPq(DRV_DISP_OS_StrConfig_t *pstStrCfg)
{
    MI_U8 *                pFileName;
    MI_U32                 ret;
    DrvDispOsFileConfig_t  stReadFileCfg;
    MHAL_DISP_PqConfig_t   stPqCfg;
    MI_U32                 SrcId, Process, LoadSettingManual = 0;
    MI_U8                  bParamSet   = 1;
    MI_U32                 filesize    = 0;
    static MI_U32          u32filesize = 0;
    MI_U32                 u32DispPath = 0;
    static char *          u64PhyAddr  = NULL;
    DRV_DISP_CTX_Config_t *pstDispCtx;
    MI_U32                 u32SetSrcId, u32Process, u32LoadSetting;

    if (pstStrCfg->argc >= 2)
    {
        if (!DRV_DISP_OS_StrCmp(pstStrCfg->argv[1], "loadbin"))
        {
            pFileName = pstStrCfg->argv[2];

            DrvDispOsOpenFile(pFileName, O_RDONLY, 0, &stReadFileCfg);
            if (!stReadFileCfg.pFile)
            {
                DISP_ERR("Read PQ file name: %s failed \n", pFileName);
                bParamSet = 0;
            }
            else
            {
                stPqCfg.u32PqFlags = E_MHAL_DISP_PQ_FLAG_LOAD_BIN;
                filesize           = DrvDispOsGetFileSize(&stReadFileCfg);
                DISPUT_DBG("PQ file name: %s filesize = %d \n", pFileName, filesize);
                DrvDispOsCloseFile(&stReadFileCfg);
                if (NULL == u64PhyAddr)
                {
                    u64PhyAddr  = CamOsMemAlloc(filesize);
                    u32filesize = filesize;
                }
                else if (u64PhyAddr && filesize != u32filesize)
                {
                    CamOsMemRelease(u64PhyAddr);
                    u64PhyAddr  = CamOsMemAlloc(filesize);
                    u32filesize = filesize;
                }
                DrvDispOsOpenFile(pFileName, O_RDONLY, 0, &stReadFileCfg);
                ret = DrvDispOsReadFile(&stReadFileCfg, (MI_U8 *)u64PhyAddr, filesize);
                if (ret > 0)
                {
                    DISPUT_DBG("Read file size: %d bytes \n", ret);
                }
                DrvDispOsCloseFile(&stReadFileCfg);
            }
        }
        else if (!DRV_DISP_OS_StrCmp(pstStrCfg->argv[1], "bypass"))
        {
            ret                = DRV_DISP_OS_StrToL(pstStrCfg->argv[2], 10, &u32DispPath);
            stPqCfg.u32PqFlags = E_MHAL_DISP_PQ_FLAG_BYPASS;
        }
        else if (!DRV_DISP_OS_StrCmp(pstStrCfg->argv[1], "freebin"))
        {
            if (NULL == u64PhyAddr)
            {
                DISPUT_DBG("PQ Bin Not Load already\n");
                bParamSet = 0;
            }
            stPqCfg.u32PqFlags = E_MHAL_DISP_PQ_FLAG_FREE_BIN;
            DISPUT_DBG("PQ Bin Free Start\n");
        }
        else if (!DRV_DISP_OS_StrCmp(pstStrCfg->argv[1], "srcid"))
        {
            ret                = DRV_DISP_OS_StrToL(pstStrCfg->argv[2], 10, &SrcId);
            ret                = DRV_DISP_OS_StrToL(pstStrCfg->argv[3], 10, &u32DispPath);
            stPqCfg.u32PqFlags = E_MHAL_DISP_PQ_FLAG_SET_SRC_ID;
        }
        else if (!DRV_DISP_OS_StrCmp(pstStrCfg->argv[1], "process"))
        {
            ret                = DRV_DISP_OS_StrToL(pstStrCfg->argv[2], 10, &Process);
            ret                = DRV_DISP_OS_StrToL(pstStrCfg->argv[3], 10, &u32DispPath);
            stPqCfg.u32PqFlags = E_MHAL_DISP_PQ_FLAG_PROCESS;
        }
        else if (!DRV_DISP_OS_StrCmp(pstStrCfg->argv[1], "loadsetting"))
        {
            LoadSettingManual  = (!DRV_DISP_OS_StrCmp(pstStrCfg->argv[2], "manual")) ? 1 : 0;
            stPqCfg.u32PqFlags = E_MHAL_DISP_PQ_FLAG_SET_LOAD_SETTING_TYPE;
        }
        else
        {
            bParamSet = 0;
        }
    }
    else
    {
        bParamSet = 0;
    }
    if (!bParamSet)
    {
        DISP_ERR("pq loadbin [PQ FILE_NAME] \n");
        DISP_ERR("pq freebin \n");
        DISP_ERR("pq bypass [DispPath]\n");
        DISP_ERR("pq srcid [Src ID] [DispPath]\n");
        DISP_ERR("pq process [En] [DispPath]\n");
        DISP_ERR("pq loadsetting [auot/manual]\n");
    }
    else
    {
        DrvDispCtxGetDeviceCurCtx(u32DispPath, &pstDispCtx);
        if (stPqCfg.u32PqFlags == E_MHAL_DISP_PQ_FLAG_LOAD_BIN)
        {
            stPqCfg.pData       = (void *)(u64PhyAddr);
            stPqCfg.u32DataSize = filesize;
        }
        else if (stPqCfg.u32PqFlags == E_MHAL_DISP_PQ_FLAG_FREE_BIN)
        {
            stPqCfg.pData       = (void *)(u64PhyAddr);
            stPqCfg.u32DataSize = sizeof(void *);
        }
        else if (stPqCfg.u32PqFlags == E_MHAL_DISP_PQ_FLAG_BYPASS)
        {
            u32SetSrcId         = (u32DispPath & 0xFFFF);
            stPqCfg.pData       = &u32SetSrcId;
            stPqCfg.u32DataSize = sizeof(MI_U32);
        }
        else if (stPqCfg.u32PqFlags == E_MHAL_DISP_PQ_FLAG_SET_SRC_ID)
        {
            u32SetSrcId         = (u32DispPath & 0xFFFF) << 16 | (SrcId & 0xFFFF);
            stPqCfg.pData       = &u32SetSrcId;
            stPqCfg.u32DataSize = sizeof(MI_U32);
        }
        else if (stPqCfg.u32PqFlags == E_MHAL_DISP_PQ_FLAG_SET_LOAD_SETTING_TYPE)
        {
            u32LoadSetting      = LoadSettingManual;
            stPqCfg.pData       = &u32LoadSetting;
            stPqCfg.u32DataSize = sizeof(MI_U32);
        }
        else if (stPqCfg.u32PqFlags == E_MHAL_DISP_PQ_FLAG_PROCESS)
        {
            u32Process          = Process ? 1 : 0;
            stPqCfg.pData       = &u32Process;
            stPqCfg.u32DataSize = sizeof(MI_U32);
        }
        DrvDispIfDeviceSetPqConfig(pstDispCtx, &stPqCfg);
        if (stPqCfg.u32PqFlags == E_MHAL_DISP_PQ_FLAG_FREE_BIN)
        {
            CamOsMemRelease(u64PhyAddr);
            u64PhyAddr  = NULL;
            u32filesize = 0;
        }
    }
}

MI_U32 DRV_DISP_DEBUG_GetPqBinName(MI_U8 *p8DstBuf)
{
    HAL_DISP_ST_QueryConfig_t   stQueryCfg;
    MI_DISP_IMPL_MhalPqConfig_t stPqCfg;
    DRV_DISP_CTX_Config_t       stDispCtx;
    MI_U32                      u32RetSprintf = -1;
    MI_U8 *                     p8SrcBuf;

    CamOsMemset(&stPqCfg, 0, sizeof(MI_DISP_IMPL_MhalPqConfig_t));
    CamOsMemset(&stQueryCfg, 0, sizeof(HAL_DISP_ST_QueryConfig_t));

    stPqCfg.pData = (void *)p8DstBuf;

    stQueryCfg.stInCfg.enQueryType = E_HAL_DISP_ST_QUERY_PQ_GET;
    stQueryCfg.stInCfg.u32CfgSize  = sizeof(MI_DISP_IMPL_MhalPqConfig_t);
    stQueryCfg.stInCfg.pInCfg      = (void *)&stPqCfg;
    HAL_DISP_IF_Init();
    if (HAL_DISP_IF_Query((void *)&stDispCtx, &stQueryCfg))
    {
        if (stQueryCfg.stOutCfg.pSetFunc)
        {
            stQueryCfg.stOutCfg.pSetFunc((void *)&stDispCtx, stQueryCfg.stInCfg.pInCfg);
        }
    }
    u32RetSprintf = stPqCfg.u32DataSize;
    return u32RetSprintf;
}
#endif

MI_U8 DRV_DISP_DEBUG_GetIrqHist(void *pstDispIrqHist)
{
    MI_U8 bRet = TRUE;

    if (pstDispIrqHist)
    {
        CamOsMemcpy(pstDispIrqHist, &g_stDispIrqHist, sizeof(HAL_DISP_IRQ_StatusHist_t));
    }
    else
    {
        bRet = FALSE;
    }

    return bRet;
}
MI_U8 DRV_DISP_DEBUG_GetDeviceInstance(MI_U32 u32DeviceId, void **ppstDevCtx)
{
    return DRV_DISP_IF_DeviceGetInstance(u32DeviceId, ppstDevCtx);
}
MI_U32 DRV_DISP_DEBUG_GetCrc16(MI_U32 u32DmaId, MI_U64 *p64Crc16)
{
    MI_U64 u64Val = 0;

    u64Val    = g_stDispIrqHist.stWorkingStatus.stDmaStatus[u32DmaId].u32CRC16[1];
    *p64Crc16 = g_stDispIrqHist.stWorkingStatus.stDmaStatus[u32DmaId].u32CRC16[0] | (u64Val << 32);
    return 0;
}
MI_U32 DRV_DISP_DEBUG_GetAffCnt(MI_U32 u32Dev, MI_U32 u32Vid, MI_U32 *p32Cnt)
{
    MI_U32 u32Val = 0;

#if DISP_STATISTIC_EN
    if (u32Dev < HAL_DISP_DEVICE_MAX && u32Vid < HAL_DISP_VIDLAYER_CNT)
    {
        u32Val = g_stDispIrqHist.stWorkingHist.stDevHist[u32Dev].stVidLayerHist[u32Vid].u32FifoFullCnt;
    }
#endif
    *p32Cnt = u32Val;
    return 0;
}
void DRV_DISP_DEBUG_Str(void *pstDispCtx, MI_U8 bResume)
{
#ifdef DISP_STR_DEBUG_ON
    HAL_DISP_ST_StrConfig_t *pstStrCfg;
    DRV_DISP_CTX_Config_t *  pCtx = pstDispCtx;
    MI_U32                   u32idx, u32idy, u32idw;
    MI_U32                   u32Reg[]   = HAL_DISP_STR_TEST_BANK;
    MI_U32                   u32WAddr[] = HAL_DISP_STR_WHITE_LIST_ADDR;
    MI_U32                   u32Cnt     = 0;
    MI_U32                   u32CntW    = 0;
    MI_U8                    u8BankData[DISP_BANK_SIZE];
    MI_U8 *                  u8PriData;
    MI_U8                    u8Print;

    pstStrCfg = &pCtx->pstCtxContain->stStr;
    u32Cnt    = sizeof(u32Reg) / sizeof(MI_U32);
    u32CntW   = sizeof(u32WAddr) / sizeof(MI_U32);
    if (bResume)
    {
        if (pstStrCfg->pData)
        {
            for (u32idx = 0; u32idx < u32Cnt; u32idx++)
            {
                HalDispUtilityReadBankCpyToBuffer(u32Reg[u32idx], u8BankData);
                u8PriData = pstStrCfg->pData + DISP_BANK_SIZE * u32idx;
                for (u32idy = 0; u32idy < DISP_BANK_SIZE; u32idy += 4)
                {
                    if ((u8BankData[u32idy] | u8BankData[u32idy + 1] << 8)
                        != (u8PriData[u32idy] | u8PriData[u32idy + 1] << 8))
                    {
                        for (u32idw = 0; u32idw < u32CntW; u32idw++)
                        {
                            u8Print = 1;
                            if (u32WAddr[u32idw] == (u32Reg[u32idx] | u32idy / 2))
                            {
                                u8Print = 0;
                                break;
                            }
                        }
                        if (u8Print)
                        {
                            DISP_DBG(DISP_DBG_LEVEL_IO, "%s %d,   Bank:%x Addr:%x Val:%x vs %x\n", __FUNCTION__,
                                     __LINE__, u32Reg[u32idx], u32idy / 4,
                                     u8BankData[u32idy] | u8BankData[u32idy + 1] << 8,
                                     u8PriData[u32idy] | u8PriData[u32idy + 1] << 8);
                        }
                    }
                }
            }
            CamOsMemRelease(pstStrCfg->pData);
            pstStrCfg->pData = NULL;
        }
    }
    else
    {
        pstStrCfg->pData = CamOsMemAlloc(DISP_BANK_SIZE * u32Cnt);
        if (pstStrCfg->pData)
        {
            for (u32idx = 0; u32idx < u32Cnt; u32idx++)
            {
                HalDispUtilityReadBankCpyToBuffer(u32Reg[u32idx], pstStrCfg->pData + DISP_BANK_SIZE * u32idx);
            }
        }
    }
#endif
}
