/*
 * drv_disp_irq.c- Sigmastar
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

#define _DRV_DISP_IRQ_C_

#include "mi_common_datatype.h"
#include "drv_disp_os.h"
#include "hal_disp_include.h"
#include "disp_debug.h"
#include "mhal_common.h"
#include "mhal_cmdq.h"
//#include "mhal_disp_datatype.h"
#include "drv_disp_ctx.h"
//-------------------------------------------------------------------------------------------------
//  Include Files
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//  Defines & Macro
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//  structure
//-------------------------------------------------------------------------------------------------
typedef struct
{
    MI_U32 u32IrqNum;
    MI_U8  bInit;
    void * pvMetaDate;
} DRV_DISP_IRQ_Config_t;
//-------------------------------------------------------------------------------------------------
//  Variable
//-------------------------------------------------------------------------------------------------
DRV_DISP_IRQ_Config_t     g_stDispIrqCfg[HAL_DISP_IRQ_ID_MAX] = HAL_DISP_IRQ_CFG;
HAL_DISP_IRQ_StatusHist_t g_stDispIrqHist;
//-------------------------------------------------------------------------------------------
//  Local Functions
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//  Global Functions
//-------------------------------------------------------------------------------------------------
void DRV_DISP_IRQ_Init(void)
{
    CamOsMemset(&g_stDispIrqHist, 0, sizeof(HAL_DISP_IRQ_StatusHist_t));
    HAL_DISP_IRQ_Init();
}

void DRV_DISP_IRQ_SetIsrNum(MI_U32 u32DevId, MI_U32 u32IsrNum)
{
    if (u32DevId >= HAL_DISP_IRQ_ID_MAX)
    {
        DISP_ERR("%s %d, DevId too big: %d\n", __FUNCTION__, __LINE__, u32DevId);
        return;
    }
    DRV_DISP_IRQ_Init();
    g_stDispIrqCfg[u32DevId].bInit     = 1;
    g_stDispIrqCfg[u32DevId].u32IrqNum = u32IsrNum;
    DISP_DBG(DISP_DBG_LEVEL_HAL, "%s %d, DevId:%d, bInit:%d, IrqNum=%d\n", __FUNCTION__, __LINE__, u32DevId,
             g_stDispIrqCfg[u32DevId].bInit, g_stDispIrqCfg[u32DevId].u32IrqNum);
}

MI_U8 DRV_DISP_IRQ_GetIsrNumByDevId(MI_U32 u32DevId, MI_U32 *pu32IsrNum)
{
    MI_U8                         bRet = 1;
    HAL_DISP_IRQ_IoctlConfig_t    stIrqIoctlCfg;
    DRV_DISP_CTX_Config_t         stDispCtxCfg;
    DRV_DISP_CTX_Contain_t        stDispCtxContain;
    DRV_DISP_CTX_DeviceContain_t *pstDevContain;
    MI_U8                         u8Idx;

    // Create a Fake Ctx
    stDispCtxCfg.enCtxType     = E_DRV_DISP_CTX_TYPE_DEVICE;
    stDispCtxCfg.u32ContainIdx = u32DevId;
    stDispCtxCfg.pstCtxContain = &stDispCtxContain;

    pstDevContain = CamOsMemAlloc(sizeof(DRV_DISP_CTX_DeviceContain_t));

    pstDevContain->u32DevId                  = u32DevId;
    stDispCtxContain.pstDevContain[u32DevId] = pstDevContain;

    stIrqIoctlCfg.enIoctlType  = E_HAL_DISP_IRQ_IOCTL_GET_ID;
    stIrqIoctlCfg.enIrqType    = E_HAL_DISP_IRQ_TYPE_NONE;
    stIrqIoctlCfg.enLcdIrqType = E_HAL_DISP_IRQ_TYPE_LCD_NONE;
    stIrqIoctlCfg.pDispCtx     = &stDispCtxCfg;
    stIrqIoctlCfg.pParam       = (void *)&u8Idx;
    HAL_DISP_IRQ_IoCtl(&stIrqIoctlCfg);

    if (g_stDispIrqCfg[u8Idx].bInit)
    {
        *pu32IsrNum = g_stDispIrqCfg[u8Idx].u32IrqNum;
        DISP_DBG(DISP_DBG_LEVEL_IRQ, "%s %d, DevId:%d, Idx:%d, IrqNum=%d\n", __FUNCTION__, __LINE__, u32DevId, u8Idx,
                 *pu32IsrNum);
    }
    else
    {
        bRet = 0;
        DISP_ERR("%s %d, Irq Idx %d, not init\n", __FUNCTION__, __LINE__, u8Idx);
    }

    CamOsMemRelease(pstDevContain);

    return bRet;
}

MI_U8 DRV_DISP_IRQ_GetIsrNum(void *pDevCtx, MI_U32 *pu32IsrNum)
{
    MI_U8                      bRet       = 1;
    DRV_DISP_CTX_Config_t *    pstDispCtx = (DRV_DISP_CTX_Config_t *)pDevCtx;
    HAL_DISP_IRQ_IoctlConfig_t stIrqIoctlCfg;
    MI_U8                      u8Idx;

    if (pstDispCtx->enCtxType == E_DRV_DISP_CTX_TYPE_DEVICE)
    {
        stIrqIoctlCfg.enIoctlType  = E_HAL_DISP_IRQ_IOCTL_GET_ID;
        stIrqIoctlCfg.enIrqType    = E_HAL_DISP_IRQ_TYPE_NONE;
        stIrqIoctlCfg.enLcdIrqType = E_HAL_DISP_IRQ_TYPE_LCD_NONE;
        stIrqIoctlCfg.pDispCtx     = pDevCtx;
        stIrqIoctlCfg.pParam       = (void *)&u8Idx;
        HAL_DISP_IRQ_IoCtl(&stIrqIoctlCfg);

        if (g_stDispIrqCfg[u8Idx].bInit)
        {
            *pu32IsrNum = g_stDispIrqCfg[u8Idx].u32IrqNum;
            DISP_DBG(DISP_DBG_LEVEL_IRQ, "%s %d, ContainId=%d, DevId:%d, Idx:%d, IrqNum=%d\n", __FUNCTION__, __LINE__,
                     pstDispCtx->u32ContainIdx, HAL_DISP_GetDeviceId(pstDispCtx, 0), u8Idx, *pu32IsrNum);
        }
        else
        {
            bRet = 0;
            DISP_ERR("%s %d, Irq Idx %d, not init\n", __FUNCTION__, __LINE__, u8Idx);
        }
    }
    else if (pstDispCtx->enCtxType == E_DRV_DISP_CTX_TYPE_DMA)
    {
        stIrqIoctlCfg.enIoctlType  = E_HAL_DISP_IRQ_IOCTL_DMA_GET_ID;
        stIrqIoctlCfg.enIrqType    = E_HAL_DISP_IRQ_TYPE_NONE;
        stIrqIoctlCfg.enLcdIrqType = E_HAL_DISP_IRQ_TYPE_LCD_NONE;
        stIrqIoctlCfg.pDispCtx     = pDevCtx;
        stIrqIoctlCfg.pParam       = (void *)&u8Idx;

        if (HAL_DISP_IRQ_IoCtl(&stIrqIoctlCfg) == 0)
        {
            DISP_ERR("%s %d, Irq Ioctl(%d) Fail\n", __FUNCTION__, __LINE__, stIrqIoctlCfg.enIoctlType);
            *pu32IsrNum = 0;
        }
        else
        {
            if (g_stDispIrqCfg[u8Idx].bInit)
            {
                *pu32IsrNum = g_stDispIrqCfg[u8Idx].u32IrqNum;
                DISP_DBG(DISP_DBG_LEVEL_IRQ_DMA, "%s %d, ContainId=%d, DmaId:%d, Idx:%d, IrqNum=%d\n", __FUNCTION__,
                         __LINE__, pstDispCtx->u32ContainIdx,
                         pstDispCtx->pstCtxContain->pstDmaContain[pstDispCtx->u32ContainIdx]->u32DmaId, u8Idx,
                         *pu32IsrNum);
            }
            else
            {
                bRet = 0;
                DISP_ERR("%s %d, Irq Idx %d, not init\n", __FUNCTION__, __LINE__, u8Idx);
            }
        }
    }
    else
    {
        bRet = 0;
        DISP_ERR("%s %d, CtxType=%s, Wrong\n", __FUNCTION__, __LINE__, PARSING_CTX_TYPE(pstDispCtx->enCtxType));
    }
    return bRet;
}

MI_U8 DRV_DISP_IRQ_Enable(void *pDevCtx, MI_U32 u32DevIrq, MI_U8 bEnable)
{
    MI_U8                      bRet = 1;
    HAL_DISP_IRQ_IoctlConfig_t stIrqIoctlCfg;
    DRV_DISP_CTX_Config_t *    pstDispCtx = (DRV_DISP_CTX_Config_t *)pDevCtx;
    // DRV_DISP_CTX_DeviceContain_t *pstDevContain = NULL;
    MI_U8 u8Idx;

    if (pstDispCtx->enCtxType == E_DRV_DISP_CTX_TYPE_DEVICE)
    {
        // pstDevContain             = pstDispCtx->pstCtxContain->pstDevContain[pstDispCtx->u32ContainIdx];
        stIrqIoctlCfg.enIoctlType  = E_HAL_DISP_IRQ_IOCTL_GET_ID;
        stIrqIoctlCfg.enIrqType    = E_HAL_DISP_IRQ_TYPE_NONE;
        stIrqIoctlCfg.enLcdIrqType = E_HAL_DISP_IRQ_TYPE_LCD_NONE;
        stIrqIoctlCfg.pDispCtx     = pDevCtx;
        stIrqIoctlCfg.pParam       = (void *)&u8Idx;
        if (HAL_DISP_IRQ_IoCtl(&stIrqIoctlCfg))
        {
            if (g_stDispIrqCfg[u8Idx].u32IrqNum == u32DevIrq)
            {
                stIrqIoctlCfg.enIoctlType  = E_HAL_DISP_IRQ_IOCTL_ENABLE;
                stIrqIoctlCfg.enIrqType    = E_HAL_DISP_IRQ_TYPE_INPORT_VSYNC;
                stIrqIoctlCfg.enLcdIrqType = E_HAL_DISP_IRQ_TYPE_LCD_NONE;
                stIrqIoctlCfg.pDispCtx     = pDevCtx;
                stIrqIoctlCfg.pParam       = (void *)&bEnable;
                if (!DISP_IN_STR_STATUS(pstDispCtx))
                {
                    g_stDispIrqCfg[u8Idx].pvMetaDate = (bEnable) ? pDevCtx : NULL;
                }
                HAL_DISP_IRQ_IoCtl(&stIrqIoctlCfg);
            }
            else
            {
                bRet = 0;
                DISP_DBG(DISP_DBG_LEVEL_IRQ, "%s %d, IrqNum not match %d != %d\n", __FUNCTION__, __LINE__, u32DevIrq,
                         g_stDispIrqCfg[u8Idx].u32IrqNum);
            }
        }
    }
    else if (pstDispCtx->enCtxType == E_DRV_DISP_CTX_TYPE_DMA)
    {
        stIrqIoctlCfg.enIoctlType  = E_HAL_DISP_IRQ_IOCTL_DMA_GET_ID;
        stIrqIoctlCfg.enIrqType    = E_HAL_DISP_IRQ_TYPE_NONE;
        stIrqIoctlCfg.enLcdIrqType = E_HAL_DISP_IRQ_TYPE_LCD_NONE;
        stIrqIoctlCfg.pDispCtx     = pDevCtx;
        stIrqIoctlCfg.pParam       = (void *)&u8Idx;
        if (HAL_DISP_IRQ_IoCtl(&stIrqIoctlCfg))
        {
            if (g_stDispIrqCfg[u8Idx].u32IrqNum == u32DevIrq)
            {
                stIrqIoctlCfg.enIoctlType  = E_HAL_DISP_IRQ_IOCTL_DMA_ENABLE;
                stIrqIoctlCfg.enIrqType    = E_HAL_DISP_IRQ_TYPE_DMA;
                stIrqIoctlCfg.enLcdIrqType = E_HAL_DISP_IRQ_TYPE_LCD_NONE;
                stIrqIoctlCfg.pDispCtx     = pDevCtx;
                stIrqIoctlCfg.pParam       = (void *)&bEnable;
                HAL_DISP_IRQ_IoCtl(&stIrqIoctlCfg);
                if (!DISP_IN_STR_STATUS(pstDispCtx))
                {
                    g_stDispIrqCfg[u8Idx].pvMetaDate = (bEnable) ? pDevCtx : NULL;
                }
            }
            else
            {
                bRet = 0;
                DISP_DBG(DISP_DBG_LEVEL_IRQ, "%s %d, IrqNum not match %d != %d\n", __FUNCTION__, __LINE__, u32DevIrq,
                         g_stDispIrqCfg[u8Idx].u32IrqNum);
            }
        }
    }
    else
    {
        bRet = 0;
        DISP_ERR("%s %d, CtxType=%s, Wrong\n", __FUNCTION__, __LINE__, PARSING_CTX_TYPE(pstDispCtx->enCtxType));
    }
    return bRet;
}

MI_U8 DRV_DISP_IRQ_GetFlag(void *pDevCtx, MI_DISP_IMPL_MhalIRQFlag_t *pstIrqFlag)
{
    MI_U8                      bRet = 1;
    MI_U32                     u32IrqFlag;
    HAL_DISP_IRQ_IoctlConfig_t stIrqIoctlCfg;
    DRV_DISP_CTX_Config_t *    pstDispCtx = (DRV_DISP_CTX_Config_t *)pDevCtx;

    if (pstDispCtx->enCtxType == E_DRV_DISP_CTX_TYPE_DEVICE)
    {
        stIrqIoctlCfg.enIoctlType  = E_HAL_DISP_IRQ_IOCTL_GET_FLAG;
        stIrqIoctlCfg.enIrqType    = E_HAL_DISP_IRQ_TYPE_INPORT_VSYNC;
        stIrqIoctlCfg.enLcdIrqType = E_HAL_DISP_IRQ_TYPE_LCD_NONE;
        stIrqIoctlCfg.pDispCtx     = pDevCtx;
        stIrqIoctlCfg.pParam       = (void *)&u32IrqFlag;
        HAL_DISP_IRQ_IoCtl(&stIrqIoctlCfg);
        pstIrqFlag->u32IrqFlag = u32IrqFlag;
        pstIrqFlag->u32IrqMask = E_HAL_DISP_IRQ_TYPE_INPORT_VSYNC;
    }
    else if (pstDispCtx->enCtxType == E_DRV_DISP_CTX_TYPE_DMA)
    {
        stIrqIoctlCfg.enIoctlType  = E_HAL_DISP_IRQ_IOCTL_DMA_GET_FLAG;
        stIrqIoctlCfg.enIrqType    = E_HAL_DISP_IRQ_TYPE_DMA;
        stIrqIoctlCfg.enLcdIrqType = E_HAL_DISP_IRQ_TYPE_LCD_NONE;
        stIrqIoctlCfg.pDispCtx     = pDevCtx;
        stIrqIoctlCfg.pParam       = (void *)&u32IrqFlag;
        HAL_DISP_IRQ_IoCtl(&stIrqIoctlCfg);
        pstIrqFlag->u32IrqFlag = u32IrqFlag;
        pstIrqFlag->u32IrqMask = E_HAL_DISP_IRQ_TYPE_DMA;
    }
    else
    {
        bRet = 0;
        DISP_ERR("%s %d, CtxType=%s, Wrong\n", __FUNCTION__, __LINE__, PARSING_CTX_TYPE(pstDispCtx->enCtxType));
    }
    return bRet;
}

MI_U8 DRV_DISP_IRQ_Clear(void *pDevCtx, void *pData)
{
    MI_U8                       bRet = 1;
    HAL_DISP_IRQ_IoctlConfig_t  stIrqIoctlCfg;
    MI_DISP_IMPL_MhalIRQFlag_t *pstIrqFlag = (MI_DISP_IMPL_MhalIRQFlag_t *)pData;
    DRV_DISP_CTX_Config_t *     pstDispCtx = (DRV_DISP_CTX_Config_t *)pDevCtx;

    if (pstDispCtx->enCtxType == E_DRV_DISP_CTX_TYPE_DEVICE)
    {
        stIrqIoctlCfg.enIoctlType  = E_HAL_DISP_IRQ_IOCTL_CLEAR;
        stIrqIoctlCfg.enIrqType    = pstIrqFlag->u32IrqFlag & pstIrqFlag->u32IrqMask;
        stIrqIoctlCfg.enLcdIrqType = E_HAL_DISP_IRQ_TYPE_LCD_NONE;
        stIrqIoctlCfg.pDispCtx     = pDevCtx;
        stIrqIoctlCfg.pParam       = NULL;
        HAL_DISP_IRQ_IoCtl(&stIrqIoctlCfg);
    }
    else if (pstDispCtx->enCtxType == E_DRV_DISP_CTX_TYPE_DMA)
    {
        stIrqIoctlCfg.enIoctlType  = E_HAL_DISP_IRQ_IOCTL_DMA_CLEAR;
        stIrqIoctlCfg.enIrqType    = pstIrqFlag->u32IrqFlag & pstIrqFlag->u32IrqMask;
        stIrqIoctlCfg.enLcdIrqType = E_HAL_DISP_IRQ_TYPE_LCD_NONE;
        stIrqIoctlCfg.pDispCtx     = pDevCtx;
        stIrqIoctlCfg.pParam       = NULL;
        HAL_DISP_IRQ_IoCtl(&stIrqIoctlCfg);
    }
    else
    {
        bRet = 0;
        DISP_ERR("%s %d, CtxType=%s, Wrong\n", __FUNCTION__, __LINE__, PARSING_CTX_TYPE(pstDispCtx->enCtxType));
    }
    return bRet;
}

MI_U8 DRV_DISP_IRQ_GetLcdIsrNum(void *pDevCtx, MI_U32 *pu32IsrNum)
{
    MI_U8                      bRet       = 1;
    DRV_DISP_CTX_Config_t *    pstDispCtx = (DRV_DISP_CTX_Config_t *)pDevCtx;
    HAL_DISP_IRQ_IoctlConfig_t stIrqIoctlCfg;
    MI_U8                      u8Idx;

    if (pstDispCtx->enCtxType == E_DRV_DISP_CTX_TYPE_DEVICE)
    {
        stIrqIoctlCfg.enIoctlType  = E_HAL_DISP_IRQ_IOCTL_LCD_GET_ID;
        stIrqIoctlCfg.enIrqType    = E_HAL_DISP_IRQ_TYPE_NONE;
        stIrqIoctlCfg.enLcdIrqType = E_HAL_DISP_IRQ_TYPE_LCD_NONE;
        stIrqIoctlCfg.pDispCtx     = pDevCtx;
        stIrqIoctlCfg.pParam       = (void *)&u8Idx;
        HAL_DISP_IRQ_IoCtl(&stIrqIoctlCfg);

        if (g_stDispIrqCfg[u8Idx].bInit)
        {
            *pu32IsrNum = g_stDispIrqCfg[u8Idx].u32IrqNum;
            DISP_DBG(DISP_DBG_LEVEL_IRQ, "%s %d, ContainId=%d, DevId:%d, Idx:%d, IrqNum=%d\n", __FUNCTION__, __LINE__,
                     pstDispCtx->u32ContainIdx, HAL_DISP_GetDeviceId(pstDispCtx, 0), u8Idx, *pu32IsrNum);
        }
        else
        {
            bRet = 0;
            DISP_ERR("%s %d, Irq Idx %d, not init\n", __FUNCTION__, __LINE__, u8Idx);
        }
    }
    else
    {
        bRet = 0;
        DISP_ERR("%s %d, CtxType=%s, Wrong\n", __FUNCTION__, __LINE__, PARSING_CTX_TYPE(pstDispCtx->enCtxType));
    }

    return bRet;
}

MI_U8 DRV_DISP_IRQ_EnableLcd(void *pDevCtx, MI_U32 u32DevIrq, MI_U8 bEnable)
{
    MI_U8                      bRet = 1;
    HAL_DISP_IRQ_IoctlConfig_t stIrqIoctlCfg;
    DRV_DISP_CTX_Config_t *    pstDispCtx = (DRV_DISP_CTX_Config_t *)pDevCtx;
    // DRV_DISP_CTX_DeviceContain_t *pstDevContain = NULL;
    MI_U8 u8Idx;

    if (pstDispCtx->enCtxType == E_DRV_DISP_CTX_TYPE_DEVICE)
    {
        // pstDevContain             = pstDispCtx->pstCtxContain->pstDevContain[pstDispCtx->u32ContainIdx];
        stIrqIoctlCfg.enIoctlType  = E_HAL_DISP_IRQ_IOCTL_LCD_GET_ID;
        stIrqIoctlCfg.enIrqType    = E_HAL_DISP_IRQ_TYPE_NONE;
        stIrqIoctlCfg.enLcdIrqType = E_HAL_DISP_IRQ_TYPE_LCD_NONE;
        stIrqIoctlCfg.pDispCtx     = pDevCtx;
        stIrqIoctlCfg.pParam       = (void *)&u8Idx;
        HAL_DISP_IRQ_IoCtl(&stIrqIoctlCfg);

        if (g_stDispIrqCfg[u8Idx].u32IrqNum == u32DevIrq)
        {
            stIrqIoctlCfg.enIoctlType  = E_HAL_DISP_IRQ_IOCTL_LCD_ENABLE;
            stIrqIoctlCfg.enIrqType    = E_HAL_DISP_IRQ_TYPE_NONE;
            stIrqIoctlCfg.enLcdIrqType = E_HAL_DISP_IRQ_TYPE_LCD_FRM_END | E_HAL_DISP_IRQ_TYPE_LCD_FLM;
            stIrqIoctlCfg.pDispCtx     = pDevCtx;
            stIrqIoctlCfg.pParam       = (void *)&bEnable;
            HAL_DISP_IRQ_IoCtl(&stIrqIoctlCfg);
            if (!DISP_IN_STR_STATUS(pstDispCtx))
            {
                g_stDispIrqCfg[u8Idx].pvMetaDate = (bEnable) ? pDevCtx : NULL;
            }
            DISP_DBG(DISP_DBG_LEVEL_IRQ, "%s %d, ContainId=%d, DevId:%d, En_Idx:%d\n", __FUNCTION__, __LINE__,
                     pstDispCtx->u32ContainIdx, HAL_DISP_GetDeviceId(pstDispCtx, 0), u8Idx);
        }
        else
        {
            bRet = 0;
            DISP_DBG(DISP_DBG_LEVEL_IRQ, "%s %d, IrqNum not match %d != %d\n", __FUNCTION__, __LINE__, u32DevIrq,
                     g_stDispIrqCfg[u8Idx].u32IrqNum);
        }
    }
    else
    {
        bRet = 0;
        DISP_DBG(DISP_DBG_LEVEL_IRQ, "%s %d, CtxType=%s, Wrong\n", __FUNCTION__, __LINE__,
                 PARSING_CTX_TYPE(pstDispCtx->enCtxType));
    }

    return bRet;
}

MI_U8 DRV_DISP_IRQ_GetLcdFlag(void *pDevCtx, MI_DISP_IMPL_MhalIRQFlag_t *pstIrqFlag)
{
    MI_U8                      bRet = 1;
    MI_U32                     u32IrqFlag;
    HAL_DISP_IRQ_IoctlConfig_t stIrqIoctlCfg;
    DRV_DISP_CTX_Config_t *    pstDispCtx = (DRV_DISP_CTX_Config_t *)pDevCtx;

    if (pstDispCtx->enCtxType == E_DRV_DISP_CTX_TYPE_DEVICE)
    {
        stIrqIoctlCfg.enIoctlType  = E_HAL_DISP_IRQ_IOCTL_LCD_GET_FLAG;
        stIrqIoctlCfg.enIrqType    = E_HAL_DISP_IRQ_TYPE_NONE;
        stIrqIoctlCfg.enLcdIrqType = E_HAL_DISP_IRQ_TYPE_LCD_FRM_END;
        stIrqIoctlCfg.pDispCtx     = pDevCtx;
        stIrqIoctlCfg.pParam       = (void *)&u32IrqFlag;
        HAL_DISP_IRQ_IoCtl(&stIrqIoctlCfg);
        pstIrqFlag->u32IrqFlag = u32IrqFlag;
        pstIrqFlag->u32IrqMask = E_HAL_DISP_IRQ_TYPE_LCD_FRM_END;
    }
    else
    {
        bRet = 0;
        DISP_ERR("%s %d, CtxType=%s, Wrong\n", __FUNCTION__, __LINE__, PARSING_CTX_TYPE(pstDispCtx->enCtxType));
    }

    return bRet;
}

MI_U8 DRV_DISP_IRQ_ClearLcd(void *pDevCtx, void *pData)
{
    MI_U8                       bRet = 1;
    HAL_DISP_IRQ_IoctlConfig_t  stIrqIoctlCfg;
    MI_DISP_IMPL_MhalIRQFlag_t *pstIrqFlag = (MI_DISP_IMPL_MhalIRQFlag_t *)pData;
    DRV_DISP_CTX_Config_t *     pstDispCtx = (DRV_DISP_CTX_Config_t *)pDevCtx;

    if (pstDispCtx->enCtxType == E_DRV_DISP_CTX_TYPE_DEVICE)
    {
        stIrqIoctlCfg.enIoctlType  = E_HAL_DISP_IRQ_IOCTL_LCD_CLEAR;
        stIrqIoctlCfg.enIrqType    = E_HAL_DISP_IRQ_TYPE_NONE;
        stIrqIoctlCfg.enLcdIrqType = pstIrqFlag->u32IrqFlag & pstIrqFlag->u32IrqMask;
        stIrqIoctlCfg.pDispCtx     = pDevCtx;
        stIrqIoctlCfg.pParam       = NULL;
        HAL_DISP_IRQ_IoCtl(&stIrqIoctlCfg);
    }
    else
    {
        bRet = 0;
        DISP_ERR("%s %d, CtxType=%s, Wrong\n", __FUNCTION__, __LINE__, PARSING_CTX_TYPE(pstDispCtx->enCtxType));
    }

    return bRet;
}

//------------------------------------------------------------------------------
// Internal Isr
//------------------------------------------------------------------------------

#if DISP_TIMEZONE_ISR_SUPPORT
static irqreturn_t _DRV_DISP_IRQ_TimeZoneCb(int eIntNum, void *pstDevParam)
{
    MI_U32                        u32Flag[HAL_DISP_DEVICE_MAX];
    HAL_DISP_IRQ_IoctlConfig_t    stIrqIoctlCfg[HAL_DISP_DEVICE_MAX];
    DRV_DISP_CTX_Config_t *       pstDispCtxCfg = (DRV_DISP_CTX_Config_t *)pstDevParam;
    DRV_DISP_CTX_DeviceContain_t *pstDevContain =
        pstDispCtxCfg->pstCtxContain->pstDevContain[pstDispCtxCfg->u32ContainIdx];
    MI_U32 u32DevId = pstDevContain->u32DevId;

    stIrqIoctlCfg[u32DevId].enIoctlType  = E_HAL_DISP_IRQ_IOCTL_TIMEZONE_GET_FLAG;
    stIrqIoctlCfg[u32DevId].enIrqType    = E_HAL_DISP_IRQ_TYPE_TIMEZONE;
    stIrqIoctlCfg[u32DevId].enLcdIrqType = E_HAL_DISP_IRQ_TYPE_LCD_NONE;
    stIrqIoctlCfg[u32DevId].pDispCtx     = pstDevParam;
    stIrqIoctlCfg[u32DevId].pParam       = (void *)&u32Flag[u32DevId];
    HAL_DISP_IRQ_IoCtl(&stIrqIoctlCfg[u32DevId]);

    stIrqIoctlCfg[u32DevId].enIoctlType  = E_HAL_DISP_IRQ_IOCTL_TIMEZONE_CLEAR;
    stIrqIoctlCfg[u32DevId].enIrqType    = u32Flag[u32DevId];
    stIrqIoctlCfg[u32DevId].enLcdIrqType = E_HAL_DISP_IRQ_TYPE_LCD_NONE;
    stIrqIoctlCfg[u32DevId].pDispCtx     = pstDevParam;
    stIrqIoctlCfg[u32DevId].pParam       = NULL;
    HAL_DISP_IRQ_IoCtl(&stIrqIoctlCfg[u32DevId]);

    return IRQ_HANDLED;
}
#endif

static MI_U8 _DRV_DISP_IRQ_CreateTimeZoneIsr(DRV_DISP_CTX_Config_t *pDispCtx)
{
#if DISP_TIMEZONE_ISR_SUPPORT
    MI_S32                        s32IrqRet;
    MI_U32                        u32IrqNum;
    MI_U8                         bSupported;
    HAL_DISP_IRQ_IoctlConfig_t    stIrqIoctlCfg;
    DRV_DISP_CTX_DeviceContain_t *pstDevContain = NULL;

    stIrqIoctlCfg.enIoctlType  = E_HAL_DISP_IRQ_IOCTL_TIMEZONE_SUPPORTED;
    stIrqIoctlCfg.enIrqType    = E_HAL_DISP_IRQ_TYPE_NONE;
    stIrqIoctlCfg.enLcdIrqType = E_HAL_DISP_IRQ_TYPE_LCD_NONE;
    stIrqIoctlCfg.pDispCtx     = (void *)pDispCtx;
    stIrqIoctlCfg.pParam       = (void *)&bSupported;
    HAL_DISP_IRQ_IoCtl(&stIrqIoctlCfg);

    if (bSupported)
    {
        MI_U8 u8DeviceIdx;
        MI_U8 bEnable;

        stIrqIoctlCfg.enIoctlType  = E_HAL_DISP_IRQ_IOCTL_TIMEZONE_GET_ID;
        stIrqIoctlCfg.enIrqType    = E_HAL_DISP_IRQ_TYPE_NONE;
        stIrqIoctlCfg.enLcdIrqType = E_HAL_DISP_IRQ_TYPE_LCD_NONE;
        stIrqIoctlCfg.pDispCtx     = (void *)pDispCtx;
        stIrqIoctlCfg.pParam       = (void *)&u8DeviceIdx;
        HAL_DISP_IRQ_IoCtl(&stIrqIoctlCfg);
        if (u8DeviceIdx == HAL_DISP_IRQ_ID_MAX)
        {
            DISP_ERR("Get Irq Num Fail\n");
            return 0;
        }
        u32IrqNum = g_stDispIrqCfg[u8DeviceIdx].u32IrqNum;

        pstDevContain = pDispCtx->pstCtxContain->pstDevContain[pDispCtx->u32ContainIdx];

        if (g_stDispIrqCfg[u8DeviceIdx].pvMetaDate == 0
            || (DISP_IN_STR_STATUS(pDispCtx) && g_stDispIrqCfg[u8DeviceIdx].pvMetaDate))
        {
            if (!DISP_IN_STR_STATUS(pDispCtx))
            {
                g_stDispIrqCfg[u8DeviceIdx].pvMetaDate = pDispCtx;
            }
            s32IrqRet = request_irq(u32IrqNum, _DRV_DISP_IRQ_TimeZoneCb, (IRQF_ONESHOT | IRQF_SHARED),
                                    (pstDevContain->u32DevId == HAL_DISP_DEVICE_ID_0)   ? "mdisp_interisr0"
                                    : (pstDevContain->u32DevId == HAL_DISP_DEVICE_ID_1) ? "mdisp_interisr1"
                                                                                        : "mdisp_interisr2",
                                    (void *)g_stDispIrqCfg[u8DeviceIdx].pvMetaDate);
            if (0 == s32IrqRet)
            {
                DISP_DBG(DISP_DBG_LEVEL_IRQ_TIMEZONE, "%s %d, IrqNum=%d\n", __FUNCTION__, __LINE__, u32IrqNum);
            }
            else
            {
                DISP_ERR("Attach Irq Fail\n");
                return 0;
            }

            disable_irq(u32IrqNum);
            enable_irq(u32IrqNum);
        }
        bEnable                    = 1;
        stIrqIoctlCfg.enIoctlType  = E_HAL_DISP_IRQ_IOCTL_TIMEZONE_ENABLE;
        stIrqIoctlCfg.enIrqType    = E_HAL_DISP_IRQ_TYPE_TIMEZONE;
        stIrqIoctlCfg.enLcdIrqType = E_HAL_DISP_IRQ_TYPE_LCD_NONE;
        stIrqIoctlCfg.pDispCtx     = (void *)pDispCtx;
        stIrqIoctlCfg.pParam       = (void *)&bEnable;
        HAL_DISP_IRQ_IoCtl(&stIrqIoctlCfg);
    }
    else
    {
        DISP_DBG(DISP_DBG_LEVEL_IRQ_TIMEZONE, "%s %d, Not Support\n", __FUNCTION__, __LINE__);
    }
#endif

    return 1;
}

#if DISP_VGA_HPD_ISR_SUPPORT
static irqreturn_t _DRV_DISP_IRQ_VgaHpdIsrCb(int eIntNum, void *pstDevParam)
{
    MI_U32                     u32Flag;
    HAL_DISP_IRQ_IoctlConfig_t stIrqIoctlCfg;

    stIrqIoctlCfg.enIoctlType  = E_HAL_DISP_IRQ_IOCTL_VGA_HPD_GET_FLAG;
    stIrqIoctlCfg.enIrqType    = E_HAL_DISP_IRQ_TYPE_VGA_HPD_ON_OFF;
    stIrqIoctlCfg.enLcdIrqType = E_HAL_DISP_IRQ_TYPE_LCD_NONE;
    stIrqIoctlCfg.pDispCtx     = pstDevParam;
    stIrqIoctlCfg.pParam       = (void *)&u32Flag;
    HAL_DISP_IRQ_IoCtl(&stIrqIoctlCfg);

    stIrqIoctlCfg.enIoctlType  = E_HAL_DISP_IRQ_IOCTL_VGA_HPD_CLEAR;
    stIrqIoctlCfg.enIrqType    = u32Flag;
    stIrqIoctlCfg.enLcdIrqType = E_HAL_DISP_IRQ_TYPE_LCD_NONE;
    stIrqIoctlCfg.pDispCtx     = pstDevParam;
    stIrqIoctlCfg.pParam       = NULL;
    HAL_DISP_IRQ_IoCtl(&stIrqIoctlCfg);

    DISP_DBG(DISP_DBG_LEVEL_IRQ_VGA_HPD, "%s %d VgaHpd:%s\n", __FUNCTION__, __LINE__,
             (u32Flag & E_HAL_DISP_IRQ_TYPE_VGA_HPD_ON) ? "ON" : "OFF");

    return IRQ_HANDLED;
}
#endif

static MI_U8 _DRV_DISP_IRQ_CreateVgaHpdIsr(DRV_DISP_CTX_Config_t *pDispCtx)
{
#if DISP_VGA_HPD_ISR_SUPPORT
    MI_S32                     s32IrqRet;
    MI_U32                     u32IrqNum;
    MI_U8                      bSupported;
    HAL_DISP_IRQ_IoctlConfig_t stIrqIoctlCfg;

    stIrqIoctlCfg.enIoctlType  = E_HAL_DISP_IRQ_IOCTL_VGA_HPD_SUPPORTED;
    stIrqIoctlCfg.enIrqType    = E_HAL_DISP_IRQ_TYPE_NONE;
    stIrqIoctlCfg.enLcdIrqType = E_HAL_DISP_IRQ_TYPE_LCD_NONE;
    stIrqIoctlCfg.pDispCtx     = (void *)pDispCtx;
    stIrqIoctlCfg.pParam       = (void *)&bSupported;
    HAL_DISP_IRQ_IoCtl(&stIrqIoctlCfg);

    if (bSupported)
    {
        MI_U8 u8DeviceIdx;
        MI_U8 bEnable;

        stIrqIoctlCfg.enIoctlType  = E_HAL_DISP_IRQ_IOCTL_VGA_HPD_GET_ID;
        stIrqIoctlCfg.enIrqType    = E_HAL_DISP_IRQ_TYPE_NONE;
        stIrqIoctlCfg.enLcdIrqType = E_HAL_DISP_IRQ_TYPE_LCD_NONE;
        stIrqIoctlCfg.pDispCtx     = (void *)pDispCtx;
        stIrqIoctlCfg.pParam       = (void *)&u8DeviceIdx;
        HAL_DISP_IRQ_IoCtl(&stIrqIoctlCfg);

        u32IrqNum = g_stDispIrqCfg[u8DeviceIdx].u32IrqNum;

        if (g_stDispIrqCfg[u8DeviceIdx].pvMetaDate == 0
            || (DISP_IN_STR_STATUS(pDispCtx) && g_stDispIrqCfg[u8DeviceIdx].pvMetaDate))
        {
            if (!DISP_IN_STR_STATUS(pDispCtx))
            {
                g_stDispIrqCfg[u8DeviceIdx].pvMetaDate = pDispCtx;
            }
            s32IrqRet = request_irq(u32IrqNum, _DRV_DISP_IRQ_VgaHpdIsrCb, (IRQF_ONESHOT | IRQF_SHARED),
                                    "mdisp_vgahpdisr", (void *)g_stDispIrqCfg[u8DeviceIdx].pvMetaDate);
            if (s32IrqRet)
            {
                DISP_ERR("Attach Irq Fail\n");
                return 0;
            }
            else
            {
                DISP_DBG(DISP_DBG_LEVEL_IRQ_VGA_HPD, "%s %d, IrqNum=%d\n", __FUNCTION__, __LINE__, u32IrqNum);
            }

            disable_irq(u32IrqNum);
            enable_irq(u32IrqNum);
        }
        bEnable                    = 1;
        stIrqIoctlCfg.enIoctlType  = E_HAL_DISP_IRQ_IOCTL_VGA_HPD_ENABLE;
        stIrqIoctlCfg.enIrqType    = E_HAL_DISP_IRQ_TYPE_VGA_HPD_ON_OFF;
        stIrqIoctlCfg.enLcdIrqType = E_HAL_DISP_IRQ_TYPE_LCD_NONE;
        stIrqIoctlCfg.pDispCtx     = (void *)pDispCtx;
        stIrqIoctlCfg.pParam       = (void *)&bEnable;
        HAL_DISP_IRQ_IoCtl(&stIrqIoctlCfg);
    }
    else
    {
        DISP_DBG(DISP_DBG_LEVEL_IRQ_VGA_HPD, "%s %d, Not Support\n", __FUNCTION__, __LINE__);
    }
#endif
    return 1;
}

static MI_U8 _DRV_DISP_IRQ_DestroyTimeZoneIsr(DRV_DISP_CTX_Config_t *pDispCtx)
{
#if DISP_TIMEZONE_ISR_SUPPORT
    MI_U32                     u32IrqNum;
    MI_U8                      bSupported;
    HAL_DISP_IRQ_IoctlConfig_t stIrqIoctlCfg;

    stIrqIoctlCfg.enIoctlType  = E_HAL_DISP_IRQ_IOCTL_TIMEZONE_SUPPORTED;
    stIrqIoctlCfg.enIrqType    = E_HAL_DISP_IRQ_TYPE_NONE;
    stIrqIoctlCfg.enLcdIrqType = E_HAL_DISP_IRQ_TYPE_LCD_NONE;
    stIrqIoctlCfg.pDispCtx     = (void *)pDispCtx;
    stIrqIoctlCfg.pParam       = (void *)&bSupported;
    HAL_DISP_IRQ_IoCtl(&stIrqIoctlCfg);

    if (bSupported)
    {
        MI_U8 u8DeviceIdx;
        MI_U8 bEnable;

        stIrqIoctlCfg.enIoctlType  = E_HAL_DISP_IRQ_IOCTL_TIMEZONE_GET_ID;
        stIrqIoctlCfg.enIrqType    = E_HAL_DISP_IRQ_TYPE_NONE;
        stIrqIoctlCfg.enLcdIrqType = E_HAL_DISP_IRQ_TYPE_LCD_NONE;
        stIrqIoctlCfg.pDispCtx     = (void *)pDispCtx;
        stIrqIoctlCfg.pParam       = (void *)&u8DeviceIdx;
        HAL_DISP_IRQ_IoCtl(&stIrqIoctlCfg);
        if (u8DeviceIdx == HAL_DISP_IRQ_ID_MAX)
        {
            DISP_ERR("Get Irq Num Fail\n");
            return 0;
        }
        if (g_stDispIrqCfg[u8DeviceIdx].pvMetaDate)
        {
            u32IrqNum = g_stDispIrqCfg[u8DeviceIdx].u32IrqNum;
            disable_irq(u32IrqNum);
            free_irq(u32IrqNum, (void *)g_stDispIrqCfg[u8DeviceIdx].pvMetaDate);
        }
        bEnable = 0;
        if (!DISP_IN_STR_STATUS(pDispCtx))
        {
            g_stDispIrqCfg[u8DeviceIdx].pvMetaDate = 0;
        }
        stIrqIoctlCfg.enIoctlType  = E_HAL_DISP_IRQ_IOCTL_TIMEZONE_ENABLE;
        stIrqIoctlCfg.enIrqType    = E_HAL_DISP_IRQ_TYPE_TIMEZONE;
        stIrqIoctlCfg.enLcdIrqType = E_HAL_DISP_IRQ_TYPE_LCD_NONE;
        stIrqIoctlCfg.pDispCtx     = (void *)pDispCtx;
        stIrqIoctlCfg.pParam       = (void *)&bEnable;
        HAL_DISP_IRQ_IoCtl(&stIrqIoctlCfg);
    }
    else
    {
        DISP_DBG(DISP_DBG_LEVEL_IRQ_TIMEZONE, "%s %d, Not Support\n", __FUNCTION__, __LINE__);
    }
#endif
    return 1;
}

static MI_U8 _DRV_DISP_IRQ_DestroyVgaHpdIsr(DRV_DISP_CTX_Config_t *pDispCtx)
{
#if DISP_VGA_HPD_ISR_SUPPORT
    MI_U32                     u32IrqNum;
    MI_U8                      bSupported;
    HAL_DISP_IRQ_IoctlConfig_t stIrqIoctlCfg;

    stIrqIoctlCfg.enIoctlType  = E_HAL_DISP_IRQ_IOCTL_VGA_HPD_SUPPORTED;
    stIrqIoctlCfg.enIrqType    = E_HAL_DISP_IRQ_TYPE_NONE;
    stIrqIoctlCfg.enLcdIrqType = E_HAL_DISP_IRQ_TYPE_LCD_NONE;
    stIrqIoctlCfg.pDispCtx     = (void *)pDispCtx;
    stIrqIoctlCfg.pParam       = (void *)&bSupported;
    HAL_DISP_IRQ_IoCtl(&stIrqIoctlCfg);

    if (bSupported)
    {
        MI_U8 u8DeviceIdx;
        MI_U8 bEnable;

        stIrqIoctlCfg.enIoctlType  = E_HAL_DISP_IRQ_IOCTL_VGA_HPD_GET_ID;
        stIrqIoctlCfg.enIrqType    = E_HAL_DISP_IRQ_TYPE_NONE;
        stIrqIoctlCfg.enLcdIrqType = E_HAL_DISP_IRQ_TYPE_LCD_NONE;
        stIrqIoctlCfg.pDispCtx     = (void *)pDispCtx;
        stIrqIoctlCfg.pParam       = (void *)&u8DeviceIdx;
        HAL_DISP_IRQ_IoCtl(&stIrqIoctlCfg);

        if (g_stDispIrqCfg[u8DeviceIdx].pvMetaDate)
        {
            u32IrqNum = g_stDispIrqCfg[u8DeviceIdx].u32IrqNum;
            disable_irq(u32IrqNum);
            free_irq(u32IrqNum, (void *)g_stDispIrqCfg[u8DeviceIdx].pvMetaDate);
        }
        bEnable = 0;
        if (!DISP_IN_STR_STATUS(pDispCtx))
        {
            g_stDispIrqCfg[u8DeviceIdx].pvMetaDate = 0;
        }
        stIrqIoctlCfg.enIoctlType  = E_HAL_DISP_IRQ_IOCTL_VGA_HPD_ENABLE;
        stIrqIoctlCfg.enIrqType    = E_HAL_DISP_IRQ_TYPE_VGA_HPD_ON_OFF;
        stIrqIoctlCfg.enLcdIrqType = E_HAL_DISP_IRQ_TYPE_LCD_NONE;
        stIrqIoctlCfg.pDispCtx     = (void *)pDispCtx;
        stIrqIoctlCfg.pParam       = (void *)&bEnable;
        HAL_DISP_IRQ_IoCtl(&stIrqIoctlCfg);
    }
    else
    {
        DISP_DBG(DISP_DBG_LEVEL_IRQ_VGA_HPD, "%s %d, Not Support\n", __FUNCTION__, __LINE__);
    }
#endif
    return 1;
}

MI_U8 DRV_DISP_IRQ_CreateInternalIsr(void *pDispCtx)
{
    MI_U8 bRet = 1;

    bRet &= _DRV_DISP_IRQ_CreateTimeZoneIsr((DRV_DISP_CTX_Config_t *)pDispCtx);
    bRet &= _DRV_DISP_IRQ_CreateVgaHpdIsr((DRV_DISP_CTX_Config_t *)pDispCtx);
    return bRet;
}

MI_U8 DRV_DISP_IRQ_DestroyInternalIsr(void *pDispCtx)
{
    MI_U8 bRet = 1;

    bRet &= _DRV_DISP_IRQ_DestroyTimeZoneIsr((DRV_DISP_CTX_Config_t *)pDispCtx);
    bRet &= _DRV_DISP_IRQ_DestroyVgaHpdIsr((DRV_DISP_CTX_Config_t *)pDispCtx);
    return bRet;
}

MI_U8 DRV_DISP_IRQ_Str(DRV_DISP_CTX_Config_t *pCtx, MI_U8 bResume)
{
    MI_U8  bRet = 1;
    MI_U32 u32DevId;
    MI_U8  u8Idx;

    pCtx->enCtxType = E_DRV_DISP_CTX_TYPE_DEVICE;
    for (u32DevId = 0; u32DevId < HAL_DISP_DEVICE_MAX; u32DevId++)
    {
        pCtx->u32ContainIdx = u32DevId;
        DISP_DBG(DISP_DBG_LEVEL_IO, "%s %d,   IRQ Handle:%d DevId:%d\n", __FUNCTION__, __LINE__, bResume, u32DevId);
        if (pCtx->pstCtxContain->bDevContainUsed[u32DevId] == 0)
        {
            continue;
        }
        if (bResume)
        {
            DRV_DISP_IRQ_CreateInternalIsr(pCtx);
        }
        else
        {
            DRV_DISP_IRQ_DestroyInternalIsr(pCtx);
        }
    }
    for (u8Idx = 0; u8Idx < HAL_DISP_IRQ_ID_MAX; u8Idx++)
    {
        if (g_stDispIrqCfg[u8Idx].pvMetaDate)
        {
            DRV_DISP_IRQ_Enable(g_stDispIrqCfg[u8Idx].pvMetaDate, g_stDispIrqCfg[u8Idx].u32IrqNum, bResume);
            DRV_DISP_IRQ_EnableLcd(g_stDispIrqCfg[u8Idx].pvMetaDate, g_stDispIrqCfg[u8Idx].u32IrqNum, bResume);
        }
    }
    return bRet;
}

MI_U16 DRV_DISP_IRQ_GetIrqCount(void)
{
    return HAL_DISP_IRQ_ID_MAX;
}
