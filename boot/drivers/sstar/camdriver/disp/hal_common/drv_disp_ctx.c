/*
 * drv_disp_ctx.c- Sigmastar
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

#define _DRV_DISP_CTX_C_

#include "mi_common_datatype.h"
#include "drv_disp_os.h"
#include "hal_disp_include.h"
#include "disp_debug.h"
#include "drv_disp_irq.h"
#include "drv_disp_ctx.h"
#include "drv_disp_if.h"
//-------------------------------------------------------------------------------------------------
//  Include Files
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//  Defines & Macro
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//  structure
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//  Variable
//-------------------------------------------------------------------------------------------------
DRV_DISP_CTX_Contain_t g_stDispCtxContainTbl[HAL_DISP_CTX_MAX_INST];
MI_U8                  g_bDispCtxInit = 0;

DRV_DISP_CTX_Config_t *g_pastCurDispDevCtx[HAL_DISP_DEVICE_MAX];
DRV_DISP_CTX_Config_t *g_pastCurVidLayerCtx[HAL_DISP_VIDLAYER_MAX];
DRV_DISP_CTX_Config_t *g_pastCurInputPortCtx[HAL_DISP_INPUTPORT_MAX];
DRV_DISP_CTX_Config_t *g_pastCurDmaCtx[HAL_DISP_DMA_MAX];
//-------------------------------------------------------------------------------------------------
//  Local Functions
//-------------------------------------------------------------------------------------------------
MI_U32 DRV_DISP_CTX_GetDeviceContainIdx(MI_U32 u32DevId)
{
    MI_U32 u32ContainIdx = 0;
    MI_U16 i, j = 0;

    for (i = 0; i < HAL_DISP_CTX_MAX_INST; i++)
    {
        if (u32DevId < HAL_DISP_DEVICE_MAX)
        {
            for (j = 0; j < HAL_DISP_DEVICE_MAX; j++)
            {
                if (g_stDispCtxContainTbl[i].bDevContainUsed[j] == 1)
                {
                    if (g_stDispCtxContainTbl[i].pstDevContain[j]->u32DevId == u32DevId)
                    {
                        u32ContainIdx = j;
                        break;
                    }
                }
            }
        }
    }
    return u32ContainIdx;
}

static MI_U8 _DRV_DISP_CTX_BindInputPortWidthVidLayer(DRV_DISP_CTX_Config_t *pstVidLayerCtx, MI_U16 u16InputPortCtxId,
                                                      MI_U16 u16InputPortId)
{
    MI_U8                             bRet                = 1;
    DRV_DISP_CTX_VideoLayerContain_t *pstVidLayerContain  = NULL;
    DRV_DISP_CTX_InputPortContain_t * pstInputPortContain = NULL;

    if (pstVidLayerCtx->enCtxType == E_DRV_DISP_CTX_TYPE_VIDLAYER && u16InputPortCtxId != HAL_DISP_INPUTPORT_MAX
        && u16InputPortId < HAL_DISP_INPUTPORT_NUM)
    {
        pstVidLayerContain = pstVidLayerCtx->pstCtxContain->pstVidLayerContain[pstVidLayerCtx->u32ContainIdx];

        pstInputPortContain = pstVidLayerCtx->pstCtxContain->pstInputPortContain[u16InputPortCtxId];

        pstVidLayerContain->pstInputPortContain[u16InputPortId] = pstInputPortContain;
        pstInputPortContain->pstVidLayerContain                 = pstVidLayerContain;

        DISP_DBG(DISP_DBG_LEVEL_CTX,
                 "%s %d, VideContainId=%d, VidLayerType=%d,(%px), InputPortContainId:%d, InputPortId:%d,(%px)\n",
                 __FUNCTION__, __LINE__, pstVidLayerCtx->u32ContainIdx, pstVidLayerContain->eVidLayerType,
                 pstVidLayerContain, u16InputPortCtxId, u16InputPortId, pstInputPortContain);
    }
    else
    {
        bRet = 0;
        DISP_ERR("%s %d, Ctx Type is Err %s, \n", __FUNCTION__, __LINE__, PARSING_CTX_TYPE(pstVidLayerCtx->enCtxType));
    }
    return bRet;
}

static MI_U8 _DRV_DISP_CTX_UnBindInputPortWidthVidLayer(DRV_DISP_CTX_Config_t *pstInputPortCtx)
{
    MI_U8                             bRet                = 1;
    DRV_DISP_CTX_VideoLayerContain_t *pstVidLayerContain  = NULL;
    DRV_DISP_CTX_InputPortContain_t * pstInputPortContain = NULL;

    if (pstInputPortCtx->enCtxType == E_DRV_DISP_CTX_TYPE_INPUTPORT)
    {
        pstInputPortContain = pstInputPortCtx->pstCtxContain->pstInputPortContain[pstInputPortCtx->u32ContainIdx];
        pstVidLayerContain  = pstInputPortContain->pstVidLayerContain;

        pstVidLayerContain->pstInputPortContain[pstInputPortContain->u32PortId]                                 = NULL;
        pstInputPortCtx->pstCtxContain->pstInputPortContain[pstInputPortCtx->u32ContainIdx]->pstVidLayerContain = NULL;

        DISP_DBG(DISP_DBG_LEVEL_CTX, "%s %d, VidLayerType=%d, InputPortContainId:%d, InputPortId:%d\n", __FUNCTION__,
                 __LINE__, pstVidLayerContain->eVidLayerType, pstInputPortCtx->u32ContainIdx,
                 pstInputPortContain->u32PortId);
    }
    else
    {
        bRet = 0;
        DISP_ERR("%s %d, Ctx Type is Err %s, \n", __FUNCTION__, __LINE__, PARSING_CTX_TYPE(pstInputPortCtx->enCtxType));
    }
    return bRet;
}

static MI_U8 _DRV_DISP_CTX_InitDeviceContain(DRV_DISP_CTX_Contain_t *pDispContain)
{
    MI_U16 i;
    MI_U8  bRet = 1;

    for (i = 0; i < HAL_DISP_DEVICE_MAX; i++)
    {
        pDispContain->bDevContainUsed[i] = 0;
        pDispContain->pstDevContain[i]   = CamOsMemAlloc(sizeof(DRV_DISP_CTX_DeviceContain_t));
        if (pDispContain->pstDevContain[i] == NULL)
        {
            bRet = 0;
            DISP_ERR("%s %d, Alloc Device Contain Fail\n", __FUNCTION__, __LINE__);
            break;
        }
        CamOsMemset(pDispContain->pstDevContain[i], 0, sizeof(DRV_DISP_CTX_DeviceContain_t));
    }
    return bRet;
}

static void _DRV_DISP_CTX_DeInitDeviceContain(DRV_DISP_CTX_Contain_t *pDispContain)
{
    MI_U16 i;

    for (i = 0; i < HAL_DISP_DEVICE_MAX; i++)
    {
        if (pDispContain->pstDevContain[i])
        {
            CamOsMemRelease(pDispContain->pstDevContain[i]);
            pDispContain->pstDevContain[i]   = NULL;
            pDispContain->bDevContainUsed[i] = 0;
        }
    }
}

static MI_U8 _DRV_DISP_CTX_InitVidLayerContain(DRV_DISP_CTX_Contain_t *pDispContain)
{
    MI_U16 i;
    MI_U8  bRet = 1;

    for (i = 0; i < HAL_DISP_VIDLAYER_MAX; i++)
    {
        pDispContain->bVidLayerContainUsed[i] = 0;
        pDispContain->pstVidLayerContain[i]   = CamOsMemAlloc(sizeof(DRV_DISP_CTX_VideoLayerContain_t));
        if (pDispContain->pstVidLayerContain[i] == NULL)
        {
            bRet = 0;
            DISP_ERR("%s %d, Alloc VidLayer Contain Fail\n", __FUNCTION__, __LINE__);
            break;
        }
        CamOsMemset(pDispContain->pstVidLayerContain[i], 0, sizeof(DRV_DISP_CTX_VideoLayerContain_t));
    }
    return bRet;
}
static void _DRV_DISP_CTX_DeInitVidLayerContain(DRV_DISP_CTX_Contain_t *pDispContain)
{
    MI_U16 i;

    for (i = 0; i < HAL_DISP_VIDLAYER_MAX; i++)
    {
        if (pDispContain->pstVidLayerContain[i])
        {
            CamOsMemRelease(pDispContain->pstVidLayerContain[i]);
            pDispContain->pstVidLayerContain[i]   = NULL;
            pDispContain->bVidLayerContainUsed[i] = 0;
        }
    }
}

static MI_U8 _DRV_DISP_CTX_InitInputPortContain(DRV_DISP_CTX_Contain_t *pstDispContain)
{
    MI_U16 i;
    MI_U8  bRet = 1;

    for (i = 0; i < HAL_DISP_INPUTPORT_MAX; i++)
    {
        pstDispContain->bInputPortContainUsed[i] = 0;
        pstDispContain->pstInputPortContain[i]   = CamOsMemAlloc(sizeof(DRV_DISP_CTX_InputPortContain_t));
        if (pstDispContain->pstInputPortContain[i] == NULL)
        {
            bRet = 0;
            DISP_ERR("%s %d, Alloc InputPort Contain Fail\n", __FUNCTION__, __LINE__);
            break;
        }
        CamOsMemset(pstDispContain->pstInputPortContain[i], 0, sizeof(DRV_DISP_CTX_InputPortContain_t));
    }
    return bRet;
}

static void _DRV_DISP_CTX_DeInitInputPortContain(DRV_DISP_CTX_Contain_t *pstDispCtxContain)
{
    MI_U16 i;

    for (i = 0; i < HAL_DISP_INPUTPORT_MAX; i++)
    {
        if (pstDispCtxContain->pstInputPortContain[i])
        {
            CamOsMemRelease(pstDispCtxContain->pstInputPortContain[i]);
            pstDispCtxContain->pstInputPortContain[i]   = NULL;
            pstDispCtxContain->bInputPortContainUsed[i] = 0;
        }
    }
}

static MI_U8 _DRV_DISP_CTX_InitDmaContain(DRV_DISP_CTX_Contain_t *pDispContain)
{
    MI_U16 i;
    MI_U8  bRet = 1;

    for (i = 0; i < HAL_DISP_DMA_MAX; i++)
    {
        pDispContain->bDmaContainUsed[i] = 0;
        pDispContain->pstDmaContain[i]   = CamOsMemAlloc(sizeof(DRV_DISP_CTX_DmaContain_t));
        if (pDispContain->pstDmaContain[i] == NULL)
        {
            bRet = 0;
            DISP_ERR("%s %d, Alloc Dma Contain Fail\n", __FUNCTION__, __LINE__);
            break;
        }
        CamOsMemset(pDispContain->pstDmaContain[i], 0, sizeof(DRV_DISP_CTX_DmaContain_t));
    }
    return bRet;
}

static void _DRV_DISP_CTX_DeInitDmaContain(DRV_DISP_CTX_Contain_t *pDispCtxContain)
{
    MI_U16 i;

    for (i = 0; i < HAL_DISP_DMA_MAX; i++)
    {
        if (pDispCtxContain->pstDmaContain[i])
        {
            CamOsMemRelease(pDispCtxContain->pstDmaContain[i]);
            pDispCtxContain->pstDmaContain[i]         = NULL;
            pDispCtxContain->bInputPortContainUsed[i] = 0;
        }
    }
}

static MI_U8 _DRV_DISP_CTX_InitHalHwContain(DRV_DISP_CTX_Contain_t *pDispContain)
{
    MI_U8 bRet = 1;
    if (sizeof(HAL_DISP_HwContain_t) != 0)
    {
        pDispContain->pstHalHwContain = CamOsMemAlloc(sizeof(HAL_DISP_HwContain_t));
        if (pDispContain->pstHalHwContain == NULL)
        {
            bRet = 0;
            DISP_ERR("%s %d, Alloc Hw Contain Fail\n", __FUNCTION__, __LINE__);
        }
        CamOsMemset(pDispContain->pstHalHwContain, 0, sizeof(HAL_DISP_HwContain_t));
    }
    return bRet;
}

static void _DRV_DISP_CTX_DeInitHalHwContain(DRV_DISP_CTX_Contain_t *pDispCtxContain)
{
    if (pDispCtxContain->pstHalHwContain)
    {
        CamOsMemRelease(pDispCtxContain->pstHalHwContain);
        pDispCtxContain->pstHalHwContain = NULL;
    }
}

static MI_U8 _DRV_DISP_CTX_AllocDevContain(DRV_DISP_CTX_AllocConfig_t *pstAllocCfg, DRV_DISP_CTX_Config_t **pCtx)
{
    MI_U16                 i, j, u16EmptyIdx, u16DevContainId;
    MI_U8                  bRet       = 1;
    DRV_DISP_CTX_Config_t *pstDispCtx = NULL;

    u16EmptyIdx     = HAL_DISP_CTX_MAX_INST;
    u16DevContainId = HAL_DISP_DEVICE_MAX;

    for (i = 0; i < HAL_DISP_CTX_MAX_INST; i++)
    {
        for (j = 0; j < HAL_DISP_DEVICE_MAX; j++)
        {
            if (g_stDispCtxContainTbl[i].bDevContainUsed[j] == 0)
            {
                u16EmptyIdx     = i;
                u16DevContainId = j;
                break;
            }
        }
    }

    if (u16EmptyIdx != HAL_DISP_CTX_MAX_INST && u16DevContainId != HAL_DISP_DEVICE_MAX)
    {
        pstDispCtx = CamOsMemAlloc(sizeof(DRV_DISP_CTX_Config_t));

        if (pstDispCtx)
        {
            CamOsMemset(pstDispCtx, 0, sizeof(DRV_DISP_CTX_Config_t));
            pstDispCtx->enCtxType     = E_DRV_DISP_CTX_TYPE_DEVICE;
            pstDispCtx->u32CtxIdx     = u16EmptyIdx;
            pstDispCtx->u32ContainIdx = u16DevContainId;
            pstDispCtx->pstCtxContain = &g_stDispCtxContainTbl[u16EmptyIdx];

            CamOsMemset(pstDispCtx->pstCtxContain->pstDevContain[u16DevContainId], 0,
                        sizeof(DRV_DISP_CTX_DeviceContain_t));

            pstDispCtx->pstCtxContain->bDevContainUsed[u16DevContainId]         = 1;
            pstDispCtx->pstCtxContain->pstDevContain[u16DevContainId]->u32DevId = pstAllocCfg->u32DevId;
            pstDispCtx->pstCtxContain->stMemAllcCfg.alloc                       = pstAllocCfg->stMemAllcCfg.alloc;
            pstDispCtx->pstCtxContain->stMemAllcCfg.free                        = pstAllocCfg->stMemAllcCfg.free;
            pstDispCtx->pstCtxContain->pstDevContain[u16DevContainId]->stListHead.next =
                &pstDispCtx->pstCtxContain->pstDevContain[u16DevContainId]->stListTail;
            pstDispCtx->pstCtxContain->pstDevContain[u16DevContainId]->stListTail.prev =
                &pstDispCtx->pstCtxContain->pstDevContain[u16DevContainId]->stListHead;
            *pCtx = pstDispCtx;
        }
        else
        {
            bRet  = 0;
            *pCtx = NULL;
            DISP_ERR("%s %d, Alloc Ctx Fail\n", __FUNCTION__, __LINE__);
        }
    }
    else
    {
        bRet  = 0;
        *pCtx = NULL;
        DISP_ERR("%s %d, Alloc Dev Contain Fail\n", __FUNCTION__, __LINE__);
    }

    return bRet;
}

static MI_U8 _DRV_DISP_CTX_FreeDevContain(DRV_DISP_CTX_Config_t *pCtx)
{
    DRV_DISP_CTX_DeviceContain_t *pstDevContain = NULL;
    MI_U8                         bRet          = 1;

    if (pCtx->enCtxType == E_DRV_DISP_CTX_TYPE_DEVICE)
    {
        pstDevContain = pCtx->pstCtxContain->pstDevContain[pCtx->u32ContainIdx];
        CamOsMemset(pstDevContain, 0, sizeof(DRV_DISP_CTX_DeviceContain_t));
        pCtx->pstCtxContain->bDevContainUsed[pCtx->u32ContainIdx] = 0;
        CamOsMemRelease(pCtx);
    }
    else
    {
        bRet = 0;
        DISP_ERR("%s %d, Ctx Type Err, %s\n", __FUNCTION__, __LINE__, PARSING_CTX_TYPE(pCtx->enCtxType));
    }
    return bRet;
}

static MI_U8 _DRV_DISP_CTX_AllocVidLayerContain(DRV_DISP_CTX_AllocConfig_t *pstAllocCfg, DRV_DISP_CTX_Config_t **pCtx)
{
    MI_U16                 i, j, u16EmptyIdx, u16VidLayerContainId;
    DRV_DISP_CTX_Config_t *pstDispCtx = NULL;
    MI_U8                  bRet       = 1;

    u16EmptyIdx          = HAL_DISP_CTX_MAX_INST;
    u16VidLayerContainId = HAL_DISP_VIDLAYER_MAX;

    for (i = 0; i < HAL_DISP_CTX_MAX_INST; i++)
    {
        for (j = 0; j < HAL_DISP_VIDLAYER_MAX; j++)
        {
            if (g_stDispCtxContainTbl[i].bVidLayerContainUsed[j] == 0)
            {
                u16EmptyIdx          = i;
                u16VidLayerContainId = j;
                break;
            }
        }
    }

    if (u16EmptyIdx != HAL_DISP_CTX_MAX_INST && u16VidLayerContainId != HAL_DISP_VIDLAYER_MAX)
    {
        pstDispCtx = CamOsMemAlloc(sizeof(DRV_DISP_CTX_Config_t));

        if (pstDispCtx)
        {
            CamOsMemset(pstDispCtx, 0, sizeof(DRV_DISP_CTX_Config_t));
            pstDispCtx->enCtxType     = E_DRV_DISP_CTX_TYPE_VIDLAYER;
            pstDispCtx->u32CtxIdx     = u16EmptyIdx;
            pstDispCtx->u32ContainIdx = u16VidLayerContainId;
            pstDispCtx->pstCtxContain = &g_stDispCtxContainTbl[u16EmptyIdx];

            CamOsMemset(pstDispCtx->pstCtxContain->pstVidLayerContain[u16VidLayerContainId], 0,
                        sizeof(DRV_DISP_CTX_VideoLayerContain_t));
            pstDispCtx->pstCtxContain->bVidLayerContainUsed[u16VidLayerContainId] = 1;
            pstDispCtx->pstCtxContain->pstVidLayerContain[u16VidLayerContainId]->eVidLayerType =
                (MI_DISP_IMPL_MhalVideoLayerType_e)pstAllocCfg->u32DevId;
            pstDispCtx->pstCtxContain->pstVidLayerContain[u16VidLayerContainId]->u32BindDevId    = HAL_DISP_DEVICE_MAX;
            pstDispCtx->pstCtxContain->pstVidLayerContain[u16VidLayerContainId]->stVidList.entry = pstDispCtx;

            *pCtx = pstDispCtx;
        }
        else
        {
            *pCtx = NULL;
            bRet  = 0;
            DISP_ERR("%s %d, Alloc Ctx Fail\n", __FUNCTION__, __LINE__);
        }
    }
    else
    {
        *pCtx = NULL;
        bRet  = 0;
        DISP_ERR("%s %d, Alloc VidLayer Contain Fail\n", __FUNCTION__, __LINE__);
    }
    return bRet;
}

static MI_U8 _DRV_DISP_CTX_FreeVidLayerContain(DRV_DISP_CTX_Config_t *pCtx)
{
    DRV_DISP_CTX_VideoLayerContain_t *pstVidLayerContain = NULL;
    MI_U8                             bRet               = 1;

    if (pCtx->enCtxType == E_DRV_DISP_CTX_TYPE_VIDLAYER)
    {
        pstVidLayerContain = pCtx->pstCtxContain->pstVidLayerContain[pCtx->u32ContainIdx];
        if (pstVidLayerContain->pstDevCtx)
        {
            pCtx->u8bCpuDirectAccess = 1;
            DRV_DISP_IF_VideoLayerUnBind(pCtx, NULL);
            pCtx->u8bCpuDirectAccess = 0;
            DISP_ERR("%s %d, Vid Ctx Free Error,Not Unbind Device  %s\n", __FUNCTION__, __LINE__,
                     PARSING_CTX_TYPE(pCtx->enCtxType));
        }
        CamOsMemset(pstVidLayerContain, 0, sizeof(DRV_DISP_CTX_VideoLayerContain_t));
        pCtx->pstCtxContain->bVidLayerContainUsed[pCtx->u32ContainIdx] = 0;
        CamOsMemRelease(pCtx);
    }
    else
    {
        bRet = 0;
        DISP_ERR("%s %d, Ctx Type Err, %s\n", __FUNCTION__, __LINE__, PARSING_CTX_TYPE(pCtx->enCtxType));
    }
    return bRet;
}

static MI_U8 _DRV_DISP_CTX_AllocInputPortContain(DRV_DISP_CTX_AllocConfig_t *pstAllocCfg, DRV_DISP_CTX_Config_t **pCtx)
{
    MI_U16                 i, j, u16EmptyIdx, u16InputPortContainId;
    DRV_DISP_CTX_Config_t *pstDispCtx = NULL;
    MI_U8                  bRet       = 1;

    u16EmptyIdx           = HAL_DISP_CTX_MAX_INST;
    u16InputPortContainId = HAL_DISP_INPUTPORT_MAX;

    for (i = 0; i < HAL_DISP_CTX_MAX_INST; i++)
    {
        for (j = 0; j < HAL_DISP_INPUTPORT_MAX; j++)
        {
            if (g_stDispCtxContainTbl[i].bInputPortContainUsed[j] == 0)
            {
                u16EmptyIdx           = i;
                u16InputPortContainId = j;
                break;
            }
        }
    }

    if (u16EmptyIdx != HAL_DISP_CTX_MAX_INST && u16InputPortContainId != HAL_DISP_INPUTPORT_MAX)
    {
        pstDispCtx = CamOsMemAlloc(sizeof(DRV_DISP_CTX_Config_t));

        if (pstDispCtx)
        {
            CamOsMemset(pstDispCtx, 0, sizeof(DRV_DISP_CTX_Config_t));
            pstDispCtx->enCtxType     = E_DRV_DISP_CTX_TYPE_INPUTPORT;
            pstDispCtx->u32CtxIdx     = u16EmptyIdx;
            pstDispCtx->u32ContainIdx = u16InputPortContainId;
            pstDispCtx->pstCtxContain = &g_stDispCtxContainTbl[u16EmptyIdx];

            CamOsMemset(pstDispCtx->pstCtxContain->pstInputPortContain[u16InputPortContainId], 0,
                        sizeof(DRV_DISP_CTX_InputPortContain_t));

            if (_DRV_DISP_CTX_BindInputPortWidthVidLayer(pstAllocCfg->pstBindCtx, u16InputPortContainId,
                                                         pstAllocCfg->u32DevId))
            {
                pstDispCtx->pstCtxContain->bInputPortContainUsed[u16InputPortContainId] = 1;
                pstDispCtx->pstCtxContain->pstInputPortContain[u16InputPortContainId]->u32PortId =
                    pstAllocCfg->u32DevId;
                pstDispCtx->pstCtxContain->pstInputPortContain[u16InputPortContainId]->bUsedInputPort = 1;
                *pCtx                                                                                 = pstDispCtx;
            }
            else
            {
                CamOsMemRelease(pstDispCtx);
                *pCtx = NULL;
                bRet  = 0;
                DISP_ERR("%s %d, Bind Fail\n", __FUNCTION__, __LINE__);
            }
        }
        else
        {
            *pCtx = NULL;
            bRet  = 0;
            DISP_ERR("%s %d, Alloc Ctx Fail\n", __FUNCTION__, __LINE__);
        }
    }
    else
    {
        *pCtx = NULL;
        bRet  = 0;
        DISP_ERR("%s %d, Alloc InputPort Contain Fail\n", __FUNCTION__, __LINE__);
    }
    return bRet;
}

static MI_U8 _DRV_DISP_CTX_FreeInputPortContain(DRV_DISP_CTX_Config_t *pCtx)
{
    DRV_DISP_CTX_InputPortContain_t *pstInputPortContain = NULL;
    MI_U8                            bRet                = 1;

    if (pCtx->enCtxType == E_DRV_DISP_CTX_TYPE_INPUTPORT)
    {
        pstInputPortContain = pCtx->pstCtxContain->pstInputPortContain[pCtx->u32ContainIdx];
        if (pstInputPortContain->bEnInPort)
        {
            pCtx->u8bCpuDirectAccess = 1;
            DRV_DISP_IF_InputPortEnable(pCtx, 0);
            pCtx->u8bCpuDirectAccess = 0;
            DISP_ERR("%s %d, Inport Ctx Free Error,Not Disable  %s\n", __FUNCTION__, __LINE__,
                     PARSING_CTX_TYPE(pCtx->enCtxType));
        }
        if (_DRV_DISP_CTX_UnBindInputPortWidthVidLayer(pCtx))
        {
            CamOsMemset(pstInputPortContain, 0, sizeof(DRV_DISP_CTX_InputPortContain_t));
            pCtx->pstCtxContain->bInputPortContainUsed[pCtx->u32ContainIdx] = 0;
            CamOsMemRelease(pCtx);
        }
        else
        {
            bRet = 0;
            DISP_ERR("%s %d, UnBind Fail\n", __FUNCTION__, __LINE__);
        }
    }
    else
    {
        bRet = 0;
        DISP_ERR("%s %d, Ctx Type Err, %s\n", __FUNCTION__, __LINE__, PARSING_CTX_TYPE(pCtx->enCtxType));
    }
    return bRet;
}

static MI_U8 _DRV_DISP_CTX_AllocDmaContain(DRV_DISP_CTX_AllocConfig_t *pstAllocCfg, DRV_DISP_CTX_Config_t **pCtx)
{
    MI_U16                 i, j, u16EmptyIdx, u16DmaContainId;
    DRV_DISP_CTX_Config_t *pstDispCtx = NULL;
    MI_U8                  bRet       = 1;

    u16EmptyIdx     = HAL_DISP_CTX_MAX_INST;
    u16DmaContainId = HAL_DISP_DMA_MAX;

    for (i = 0; i < HAL_DISP_CTX_MAX_INST; i++)
    {
        for (j = 0; j < HAL_DISP_DMA_MAX; j++)
        {
            if (g_stDispCtxContainTbl[i].bDmaContainUsed[j] == 0)
            {
                u16EmptyIdx     = i;
                u16DmaContainId = j;
                break;
            }
        }
    }

    if (u16EmptyIdx != HAL_DISP_CTX_MAX_INST && u16DmaContainId != HAL_DISP_DMA_MAX)
    {
        pstDispCtx = CamOsMemAlloc(sizeof(DRV_DISP_CTX_Config_t));

        if (pstDispCtx)
        {
            CamOsMemset(pstDispCtx, 0, sizeof(DRV_DISP_CTX_Config_t));
            pstDispCtx->enCtxType     = E_DRV_DISP_CTX_TYPE_DMA;
            pstDispCtx->u32CtxIdx     = u16EmptyIdx;
            pstDispCtx->u32ContainIdx = u16DmaContainId;
            pstDispCtx->pstCtxContain = &g_stDispCtxContainTbl[u16EmptyIdx];

            CamOsMemset(pstDispCtx->pstCtxContain->pstDmaContain[u16DmaContainId], 0,
                        sizeof(DRV_DISP_CTX_DmaContain_t));
            pstDispCtx->pstCtxContain->bDmaContainUsed[u16DmaContainId]         = 1;
            pstDispCtx->pstCtxContain->pstDmaContain[u16DmaContainId]->u32DmaId = pstAllocCfg->u32DevId;
            *pCtx                                                               = pstDispCtx;
        }
        else
        {
            *pCtx = NULL;
            bRet  = 0;
            DISP_ERR("%s %d, Alloc Dma Ctx Fail\n", __FUNCTION__, __LINE__);
        }
    }
    else
    {
        *pCtx = NULL;
        bRet  = 0;
        DISP_ERR("%s %d, Alloc Dma Contain Fail\n", __FUNCTION__, __LINE__);
    }
    return bRet;
}

static MI_U8 _DRV_DISP_CTX_FreeDmaContain(DRV_DISP_CTX_Config_t *pCtx)
{
    DRV_DISP_CTX_DmaContain_t *pstDmaContain = NULL;
    MI_U8                      bRet          = 1;

    if (pCtx->enCtxType == E_DRV_DISP_CTX_TYPE_DMA)
    {
        pstDmaContain = pCtx->pstCtxContain->pstDmaContain[pCtx->u32ContainIdx];
        if (pstDmaContain->pSrcDevContain)
        {
            DISP_ERR("%s %d, Dma Ctx Free Waring,Not Unbind Device  %s\n", __FUNCTION__, __LINE__,
                     PARSING_CTX_TYPE(pCtx->enCtxType));
        }
        CamOsMemset(pstDmaContain, 0, sizeof(DRV_DISP_CTX_DmaContain_t));
        pCtx->pstCtxContain->bDmaContainUsed[pCtx->u32ContainIdx] = 0;
        CamOsMemRelease(pCtx);
    }
    else
    {
        bRet = 0;
        DISP_ERR("%s %d, Ctx Type Err, %s\n", __FUNCTION__, __LINE__, PARSING_CTX_TYPE(pCtx->enCtxType));
    }
    return bRet;
}

//-------------------------------------------------------------------------------------------------
//  Global Functions
//-------------------------------------------------------------------------------------------------
// use in head/tail list case.
void DRV_DISP_CTX_ListAddList(DRV_DISP_CTX_List_t *pstNew, DRV_DISP_CTX_List_t *pstPrev)
{
    DRV_DISP_CTX_List_t *pstNextEntry = NULL;

    DISP_DBG(DISP_DBG_LEVEL_CTX, "%s %d, pstPrev:%px,%px,%px pstNew:%px,%px,%px\n", __FUNCTION__, __LINE__, pstPrev,
             pstNew->prev, pstPrev->next, pstNew, pstNew->prev, pstNew->next);
    pstNextEntry       = pstPrev->next;
    pstPrev->next      = pstNew;
    pstNew->prev       = pstPrev;
    pstNew->next       = pstNextEntry;
    pstNextEntry->prev = pstNew;
}

void DRV_DISP_CTX_ListDelList(DRV_DISP_CTX_List_t *pstDel)
{
    DRV_DISP_CTX_List_t *pstNextEntry = NULL;

    pstNextEntry       = pstDel->prev;
    pstNextEntry->next = pstDel->next;
    pstDel->next->prev = pstNextEntry;
    pstDel->prev       = NULL;
    pstDel->next       = NULL;
}

MI_U8 DRV_DISP_CTX_Init(void)
{
    MI_U16 i;
    MI_U8  bRet = 1;

    if (g_bDispCtxInit == 1)
    {
        return 1;
    }

    DRV_DISP_IRQ_Init();

    CamOsMemset(&g_stDispCtxContainTbl, 0, sizeof(DRV_DISP_CTX_Config_t) * HAL_DISP_CTX_MAX_INST);

    HAL_DISP_IF_Init();

    for (i = 0; i < HAL_DISP_DEVICE_MAX; i++)
    {
        g_pastCurDispDevCtx[i] = NULL;
    }

    for (i = 0; i < HAL_DISP_VIDLAYER_MAX; i++)
    {
        g_pastCurVidLayerCtx[i] = NULL;
    }

    for (i = 0; i < HAL_DISP_INPUTPORT_MAX; i++)
    {
        g_pastCurInputPortCtx[i] = NULL;
    }

    for (i = 0; i < HAL_DISP_DMA_MAX; i++)
    {
        g_pastCurDmaCtx[i] = NULL;
    }
    for (i = 0; i < HAL_DISP_CTX_MAX_INST; i++)
    {
        // Device
        if (_DRV_DISP_CTX_InitDeviceContain(&g_stDispCtxContainTbl[i]) == FALSE)
        {
            _DRV_DISP_CTX_DeInitDeviceContain(&g_stDispCtxContainTbl[i]);
            bRet = 0;
            break;
        }

        // VideoLayer
        if (_DRV_DISP_CTX_InitVidLayerContain(&g_stDispCtxContainTbl[i]) == FALSE)
        {
            _DRV_DISP_CTX_DeInitVidLayerContain(&g_stDispCtxContainTbl[i]);
            _DRV_DISP_CTX_DeInitDeviceContain(&g_stDispCtxContainTbl[i]);
            bRet = 0;
            break;
        }

        // InputPort
        if (_DRV_DISP_CTX_InitInputPortContain(&g_stDispCtxContainTbl[i]) == FALSE)
        {
            _DRV_DISP_CTX_DeInitInputPortContain(&g_stDispCtxContainTbl[i]);
            _DRV_DISP_CTX_DeInitVidLayerContain(&g_stDispCtxContainTbl[i]);
            _DRV_DISP_CTX_DeInitDeviceContain(&g_stDispCtxContainTbl[i]);
            bRet = 0;
            break;
        }

        // Dma
        if (_DRV_DISP_CTX_InitDmaContain(&g_stDispCtxContainTbl[i]) == FALSE)
        {
            _DRV_DISP_CTX_DeInitDmaContain(&g_stDispCtxContainTbl[i]);
            _DRV_DISP_CTX_DeInitInputPortContain(&g_stDispCtxContainTbl[i]);
            _DRV_DISP_CTX_DeInitVidLayerContain(&g_stDispCtxContainTbl[i]);
            _DRV_DISP_CTX_DeInitDeviceContain(&g_stDispCtxContainTbl[i]);
            bRet = 0;
            break;
        }

        // HalHw
        if (_DRV_DISP_CTX_InitHalHwContain(&g_stDispCtxContainTbl[i]) == FALSE)
        {
            _DRV_DISP_CTX_DeInitHalHwContain(&g_stDispCtxContainTbl[i]);
            _DRV_DISP_CTX_DeInitDmaContain(&g_stDispCtxContainTbl[i]);
            _DRV_DISP_CTX_DeInitInputPortContain(&g_stDispCtxContainTbl[i]);
            _DRV_DISP_CTX_DeInitVidLayerContain(&g_stDispCtxContainTbl[i]);
            _DRV_DISP_CTX_DeInitDeviceContain(&g_stDispCtxContainTbl[i]);
            bRet = 0;
            break;
        }
        // MemAlloc
        g_stDispCtxContainTbl[i].stMemAllcCfg.alloc = NULL;
        g_stDispCtxContainTbl[i].stMemAllcCfg.free  = NULL;
        g_stDispCtxContainTbl[i].stStr.u8bSuspendEn = 0;
    }

    g_bDispCtxInit = 1;
    return bRet;
}

MI_U8 DRV_DISP_CTX_DeInit(void)
{
    MI_U16 i;
    MI_U8  bRet = 1;

    if (g_bDispCtxInit == 0)
    {
        bRet = 0;
        DISP_ERR("%s %d, Ctx not Init\n", __FUNCTION__, __LINE__);
    }
    else
    {
        for (i = 0; i < HAL_DISP_CTX_MAX_INST; i++)
        {
            _DRV_DISP_CTX_DeInitHalHwContain(&g_stDispCtxContainTbl[i]);
            _DRV_DISP_CTX_DeInitDmaContain(&g_stDispCtxContainTbl[i]);
            _DRV_DISP_CTX_DeInitInputPortContain(&g_stDispCtxContainTbl[i]);
            _DRV_DISP_CTX_DeInitVidLayerContain(&g_stDispCtxContainTbl[i]);
            _DRV_DISP_CTX_DeInitDeviceContain(&g_stDispCtxContainTbl[i]);
            g_stDispCtxContainTbl[i].stMemAllcCfg.alloc = NULL;
            g_stDispCtxContainTbl[i].stMemAllcCfg.free  = NULL;
            g_stDispCtxContainTbl[i].stStr.u8bSuspendEn = 0;
        }

        for (i = 0; i < HAL_DISP_DEVICE_MAX; i++)
        {
            g_pastCurDispDevCtx[i] = NULL;
        }

        for (i = 0; i < HAL_DISP_VIDLAYER_MAX; i++)
        {
            g_pastCurVidLayerCtx[i] = NULL;
        }

        for (i = 0; i < HAL_DISP_INPUTPORT_MAX; i++)
        {
            g_pastCurInputPortCtx[i] = NULL;
        }

        for (i = 0; i < HAL_DISP_DMA_MAX; i++)
        {
            g_pastCurDmaCtx[i] = NULL;
        }

        HAL_DISP_IF_DeInit();
        g_bDispCtxInit = 0;
        DISP_DBG(DISP_DBG_LEVEL_CTX, "%s %d\n", __FUNCTION__, __LINE__);
    }

    return bRet;
}

MI_U8 DRV_DISP_CTX_Allocate(DRV_DISP_CTX_AllocConfig_t *pstAllocCfg, DRV_DISP_CTX_Config_t **pCtx)
{
    MI_U8 bRet = 1;

    if (pstAllocCfg->enType == E_DRV_DISP_CTX_TYPE_DEVICE)
    {
        bRet = _DRV_DISP_CTX_AllocDevContain(pstAllocCfg, pCtx);
    }
    else if (pstAllocCfg->enType == E_DRV_DISP_CTX_TYPE_VIDLAYER)
    {
        bRet = _DRV_DISP_CTX_AllocVidLayerContain(pstAllocCfg, pCtx);
    }
    else if (pstAllocCfg->enType == E_DRV_DISP_CTX_TYPE_INPUTPORT)
    {
        bRet = _DRV_DISP_CTX_AllocInputPortContain(pstAllocCfg, pCtx);
    }
    else if (pstAllocCfg->enType == E_DRV_DISP_CTX_TYPE_DMA)
    {
        bRet = _DRV_DISP_CTX_AllocDmaContain(pstAllocCfg, pCtx);
    }
    else
    {
        bRet = 0;
        DISP_ERR("%s %d, Alloc Type is Err %s\n", __FUNCTION__, __LINE__, PARSING_CTX_TYPE(pstAllocCfg->enType));
    }
    DISP_DBG(DISP_DBG_LEVEL_CTX, "%s %d, CtxType:%s bCreate:%hhd\n", __FUNCTION__, __LINE__,
             PARSING_CTX_TYPE(pstAllocCfg->enType), bRet);
    return bRet;
}

MI_U8 DRV_DISP_CTX_Free(DRV_DISP_CTX_Config_t *pCtx)
{
    MI_U8 bRet = 1;

    DISP_DBG(DISP_DBG_LEVEL_CTX, "%s %d, CtxType=%s, ContainIdx:%d\n", __FUNCTION__, __LINE__,
             PARSING_CTX_TYPE(pCtx->enCtxType), pCtx->u32ContainIdx);
    if (pCtx->enCtxType == E_DRV_DISP_CTX_TYPE_DEVICE)
    {
        bRet = _DRV_DISP_CTX_FreeDevContain(pCtx);
    }
    else if (pCtx->enCtxType == E_DRV_DISP_CTX_TYPE_VIDLAYER)
    {
        bRet = _DRV_DISP_CTX_FreeVidLayerContain(pCtx);
    }
    else if (pCtx->enCtxType == E_DRV_DISP_CTX_TYPE_INPUTPORT)
    {
        bRet = _DRV_DISP_CTX_FreeInputPortContain(pCtx);
    }
    else if (pCtx->enCtxType == E_DRV_DISP_CTX_TYPE_DMA)
    {
        bRet = _DRV_DISP_CTX_FreeDmaContain(pCtx);
    }
    else
    {
        bRet = 0;
        DISP_ERR("%s %d, Alloc Type is Err %s\n", __FUNCTION__, __LINE__, PARSING_CTX_TYPE(pCtx->enCtxType));
    }
    return bRet;
}
MI_U8 DRV_DISP_CTX_IsLastDeviceCtx(DRV_DISP_CTX_Config_t *pCtx)
{
    MI_U16 i, j;
    MI_U8  bContainUse = 0;

    for (i = 0; i < HAL_DISP_CTX_MAX_INST; i++)
    {
        for (j = 0; j < HAL_DISP_DEVICE_MAX; j++)
        {
            bContainUse += g_stDispCtxContainTbl[i].bDevContainUsed[j];
        }
    }
    return (bContainUse == 1) ? 1 : 0;
}
MI_U8 DRV_DISP_CTX_IsAllFree(void)
{
    MI_U16 i, j;
    MI_U8  bContainUse = 0;

    for (i = 0; i < HAL_DISP_CTX_MAX_INST; i++)
    {
        for (j = 0; j < HAL_DISP_DEVICE_MAX; j++)
        {
            bContainUse |= g_stDispCtxContainTbl[i].bDevContainUsed[j];
        }

        for (j = 0; j < HAL_DISP_VIDLAYER_MAX; j++)
        {
            bContainUse |= g_stDispCtxContainTbl[i].bVidLayerContainUsed[j];
        }

        for (j = 0; j < HAL_DISP_INPUTPORT_MAX; j++)
        {
            bContainUse |= g_stDispCtxContainTbl[i].bInputPortContainUsed[j];
        }

        for (j = 0; j < HAL_DISP_DMA_MAX; j++)
        {
            bContainUse |= g_stDispCtxContainTbl[i].bDmaContainUsed[j];
        }
    }

    return bContainUse ? 0 : 1;
}

MI_U8 DRV_DISP_CTX_SetCurCtx(DRV_DISP_CTX_Config_t *pCtx, MI_U32 u32Idx)
{
    MI_U8 bRet = TRUE;

    if (pCtx->enCtxType == E_DRV_DISP_CTX_TYPE_DEVICE && u32Idx < HAL_DISP_DEVICE_MAX)
    {
        g_pastCurDispDevCtx[u32Idx] = pCtx;
    }
    else if (pCtx->enCtxType == E_DRV_DISP_CTX_TYPE_VIDLAYER && u32Idx < HAL_DISP_VIDLAYER_MAX)
    {
        g_pastCurVidLayerCtx[u32Idx] = pCtx;
    }
    else if (pCtx->enCtxType == E_DRV_DISP_CTX_TYPE_INPUTPORT && u32Idx < HAL_DISP_INPUTPORT_MAX)
    {
        g_pastCurInputPortCtx[u32Idx] = pCtx;
    }
    else if (pCtx->enCtxType == E_DRV_DISP_CTX_TYPE_DMA && u32Idx < HAL_DISP_DMA_MAX)
    {
        g_pastCurDmaCtx[u32Idx] = pCtx;
    }
    else
    {
        bRet = 0;
        DISP_ERR("%s %d, SetCurCtx Type is Err %s\n", __FUNCTION__, __LINE__, PARSING_CTX_TYPE(pCtx->enCtxType));
    }
    return bRet;
}
MI_U8 DRV_DISP_CTX_GetDeviceCurCtx(MI_U32 u32DeviceId, DRV_DISP_CTX_Config_t **pCtx)
{
    MI_U8  bRet          = TRUE;
    MI_U32 u32ContainIdx = 0;

    if (u32DeviceId < HAL_DISP_DEVICE_MAX)
    {
        u32ContainIdx = DRV_DISP_CTX_GetDeviceContainIdx(u32DeviceId);
        *pCtx         = g_pastCurDispDevCtx[u32ContainIdx];
    }
    else
    {
        bRet  = FALSE;
        *pCtx = NULL;
        DISP_ERR("%s %d, GetCurCtx Fail, ContainIdx=%d DeviceId=%d \n", __FUNCTION__, __LINE__, u32ContainIdx,
                 u32DeviceId);
    }
    return bRet;
}
