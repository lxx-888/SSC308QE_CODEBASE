/*
 * hal_disp_utility.c- Sigmastar
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
#define __HAL_DISP_UTILITY_C__

//-------------------------------------------------------------------------------------------------
//  Include Files
//-------------------------------------------------------------------------------------------------
#include "mi_common_datatype.h"
#include "mhal_cmdq.h"
#include "drv_disp_os.h"
#include "disp_debug.h"
#include "hal_disp_chip.h"
#include "hal_disp_reg.h"
#include "hal_disp_util.h"

//-------------------------------------------------------------------------------------------------
//  Defines & Macro
//-------------------------------------------------------------------------------------------------
#define RIU_GET_ADDR(addr) ((void *)(g_pvDispUtilityRiuBaseVir + ((addr) << 1)))

//-------------------------------------------------------------------------------------------------
//  Variable
//-------------------------------------------------------------------------------------------------
MI_U32                           g_u32DispUtilityCmdqInitFlag = 0;
HAL_DISP_UTILITY_CmdqContext_t   g_stDispUtilityCmdqCtx[HAL_DISP_UTILITY_CMDQ_NUM];
MI_DISP_IMPL_MhalRegAccessType_e g_eDispRegAccessMode[HAL_DISP_UTILITY_CMDQ_NUM] = {DISP_REG_ACCESS_MODE};
static void *                    g_pvDispUtilityRiuBaseVir                       = NULL;

//-------------------------------------------------------------------------------------------------
//  Local Functions
//-------------------------------------------------------------------------------------------------
static void _HAL_DISP_UTILITY_SetInitFlagByIdx(MI_U32 u32CtxId, MI_U8 bEnable)
{
    if (bEnable)
    {
        g_u32DispUtilityCmdqInitFlag |= (1 << u32CtxId);
    }
    else
    {
        g_u32DispUtilityCmdqInitFlag &= ~(1 << u32CtxId);
    }
}

static MI_U8 _HAL_DISP_UTILITY_GetInitFlagByIdx(MI_U32 u32CtxId)
{
    return (g_u32DispUtilityCmdqInitFlag & (1 << u32CtxId)) ? 1 : 0;
}

static void *_HAL_DISP_UTILITY_GetDirectCmdqBuffer(HAL_DISP_UTILITY_CmdqContext_t *pstCmdqCtx)
{
    return pstCmdqCtx->pvDirectCmdqBuffer;
}

static void *_HAL_DISP_UTILITY_GetDirectCmdqCnt(HAL_DISP_UTILITY_CmdqContext_t *pstCmdqCtx)
{
    return (void *)&pstCmdqCtx->u32DirectCmdqCnt;
}

static void *_HAL_DISP_UTILITY_GetCmdqCmdBuffer(HAL_DISP_UTILITY_CmdqContext_t *pstCmdqCtx)
{
    return pstCmdqCtx->pvCmdqCmdBuffer;
}

static void *_HAL_DISP_UTILITY_GetCmdqCmdCnt(HAL_DISP_UTILITY_CmdqContext_t *pstCmdqCtx)
{
    return (void *)&pstCmdqCtx->u32CmdqCmdCnt;
}

static MI_U8 _HAL_DISP_UTILITY_AddWriteRegCmdqMaskMulti(MHAL_CMDQ_CmdqInterface_t *pstCmdqInf, void *pvCmdqBuffer,
                                                        MI_U32 u32CmdqCmdCnt)
{
    MI_U8                      bRet         = 1;
    MI_S32                     s32CmdqRet   = 0;
    MI_U32                     u32CmdqState = 0;
    MI_U32                     i            = 0;
    HAL_DISP_UTILITY_CmdReg_t *pstCmdReg    = NULL;

    if (pstCmdqInf && pstCmdqInf->MHAL_CMDQ_ReadStatusCmdq && pvCmdqBuffer && u32CmdqCmdCnt)
    {
        DISP_DBG(DISP_DBG_LEVEL_UTILITY_FLIP, "%s %d, Cnt:%d, Buff:%px\n", __FUNCTION__, __LINE__, u32CmdqCmdCnt,
                 pvCmdqBuffer);

        for (i = 0; i < u32CmdqCmdCnt; i++)
        {
            pstCmdReg = (HAL_DISP_UTILITY_CmdReg_t *)(pvCmdqBuffer + (i * sizeof(HAL_DISP_UTILITY_CmdReg_t)));
            DISP_DBG(DISP_DBG_LEVEL_UTILITY_FLIP, "%s %d, Addr [0x%04x%02x] Val [0x%04x] Mask [0x%04x]\n", __FUNCTION__,
                     __LINE__, (pstCmdReg->u32Addr & 0xFFFF00) >> 8, (pstCmdReg->u32Addr & 0xFF) >> 1,
                     pstCmdReg->u16Data, pstCmdReg->u16Mask);
            DISP_DBG(DISP_DBG_LEVEL_UTILITY_FLIP_HWRD, "wriu -w 0x%04x%02x 0x%04hx \n", (pstCmdReg->u32Addr >> 8),
                     (pstCmdReg->u32Addr & 0xFF),
                     ((pstCmdReg->u16Data & pstCmdReg->u16Mask)
                      | HAL_DISP_UTILITY_R2BYTEMaskDirect(pstCmdReg->u32Addr, ~pstCmdReg->u16Mask)));
        }

        while (0 == (s32CmdqRet = pstCmdqInf->MHAL_CMDQ_CheckBufAvailable(pstCmdqInf, u32CmdqCmdCnt + 10)))
        {
            s32CmdqRet = pstCmdqInf->MHAL_CMDQ_ReadStatusCmdq(pstCmdqInf, &u32CmdqState);
            if (s32CmdqRet)
            {
                // DISP_ERR("%s %d, MHAL_CMDQ_ReadStatusCmdq Error\n", __FUNCTION__, __LINE__);
            }

            if ((u32CmdqState & MHAL_CMDQ_ERROR_STATUS) != 0)
            {
                DISP_ERR("%s %d, Cmdq Status Error\n", __FUNCTION__, __LINE__);
                pstCmdqInf->MHAL_CMDQ_CmdqResetEngine(pstCmdqInf);
            }
        }

        s32CmdqRet = pstCmdqInf->MHAL_CMDQ_WriteRegCmdqMaskMulti(
            pstCmdqInf, (MHAL_CMDQ_MultiCmdBufMask_t *)pvCmdqBuffer, u32CmdqCmdCnt);
        if (s32CmdqRet < u32CmdqCmdCnt)
        {
            DISP_ERR("%s %d, DirectCmdqTbl Cmdq Write Multi Fail, Ret [%d] Cnt [%d]\n", __FUNCTION__, __LINE__,
                     s32CmdqRet, u32CmdqCmdCnt);
        }
    }
    else
    {
        bRet = 0;
        DISP_ERR("%s %d, pstCmdqInf:%px, CmdqCnt:%d, Buffer:%px\n", __FUNCTION__, __LINE__, pstCmdqInf, u32CmdqCmdCnt,
                 pvCmdqBuffer);
    }
    DISP_DBG_VAL(pstCmdReg);
    return bRet;
}

static void _HAL_DISP_UTILITY_DirectCmdqWriteWithCmdqCmd(void *pvCtxIn)
{
    MI_U32                          i                 = 0;
    HAL_DISP_UTILITY_CmdqCmd_t *    pstCmdqCmdPre     = NULL;
    HAL_DISP_UTILITY_CmdqCmd_t *    pstCmdqCmdCur     = NULL;
    HAL_DISP_UTILITY_CmdReg_t *     pvCmdqBuffer      = NULL;
    HAL_DISP_UTILITY_CmdqContext_t *pstCmdqCtx        = (HAL_DISP_UTILITY_CmdqContext_t *)pvCtxIn;
    MHAL_CMDQ_CmdqInterface_t *     pstCmdqInf        = NULL;
    MI_U32 *                        pu32DirectCmdqCnt = NULL;
    void *                          pvDirctCmdqBuffer = NULL;
    MI_U32 *                        pu32CmdqCmdCnt    = NULL;
    void *                          pvCmdqCmdBuffer   = NULL;
    MI_U32                          u32CmdqBufferCnt  = 0;

    pstCmdqInf = pstCmdqCtx->pvCmdqInf;

    pvDirctCmdqBuffer = _HAL_DISP_UTILITY_GetDirectCmdqBuffer(pstCmdqCtx);
    pu32DirectCmdqCnt = _HAL_DISP_UTILITY_GetDirectCmdqCnt(pstCmdqCtx);

    pvCmdqCmdBuffer = _HAL_DISP_UTILITY_GetCmdqCmdBuffer(pstCmdqCtx);
    pu32CmdqCmdCnt  = _HAL_DISP_UTILITY_GetCmdqCmdCnt(pstCmdqCtx);

    DISP_DBG(DISP_DBG_LEVEL_UTILITY_FLIP, "%s %d, CmdqCmdCnt:%d, DirectCmdqCnt:%d\n", __FUNCTION__, __LINE__,
             *pu32CmdqCmdCnt, *pu32DirectCmdqCnt);

    for (i = 0; i < *pu32CmdqCmdCnt; i++)
    {
        pstCmdqCmdPre =
            i > 0 ? (HAL_DISP_UTILITY_CmdqCmd_t *)(pvCmdqCmdBuffer + ((i - 1) * sizeof(HAL_DISP_UTILITY_CmdqCmd_t)))
                  : NULL;

        pstCmdqCmdCur = (HAL_DISP_UTILITY_CmdqCmd_t *)(pvCmdqCmdBuffer + (i * sizeof(HAL_DISP_UTILITY_CmdqCmd_t)));

        if (pstCmdqCmdPre)
        {
            u32CmdqBufferCnt = pstCmdqCmdCur->u32CmdqBufIdx - pstCmdqCmdPre->u32CmdqBufIdx;
            pvCmdqBuffer =
                (HAL_DISP_UTILITY_CmdReg_t *)(pvDirctCmdqBuffer
                                              + (pstCmdqCmdPre->u32CmdqBufIdx * sizeof(HAL_DISP_UTILITY_CmdReg_t)));
        }
        else
        {
            u32CmdqBufferCnt = pstCmdqCmdCur->u32CmdqBufIdx;
            pvCmdqBuffer     = (HAL_DISP_UTILITY_CmdReg_t *)pvDirctCmdqBuffer;
        }

        if (u32CmdqBufferCnt)
        {
            _HAL_DISP_UTILITY_AddWriteRegCmdqMaskMulti(pstCmdqInf, (void *)pvCmdqBuffer, u32CmdqBufferCnt);
        }

        if (pstCmdqCmdCur->u32Addr == HAL_DISP_UTILITY_CMDQ_DELAY_CMD)
        {
            DISP_DBG(DISP_DBG_LEVEL_UTILITY_FLIP, "%s %d, DelayCmd Time [%d]\n", __FUNCTION__, __LINE__,
                     pstCmdqCmdCur->u32Time);
            HAL_DISP_UTILITY_AddDelayCmd(pvCtxIn, pstCmdqCmdCur->u32Time);
        }
        else if (pstCmdqCmdCur->u32Addr == HAL_DISP_UTILITY_CMDQ_WAITDONE_CMD)
        {
            DISP_DBG(DISP_DBG_LEVEL_UTILITY_FLIP, "%s %d, WaitDone\n", __FUNCTION__, __LINE__);
            HAL_DISP_UTILITY_AddWaitCmd(pvCtxIn);
        }
        else
        {
            DISP_DBG(DISP_DBG_LEVEL_UTILITY_FLIP, "%s %d, PollCmd [0x%04x%02x] Val [0x%04x] Mask [0x%04x] Time [%x]\n",
                     __FUNCTION__, __LINE__, (pstCmdqCmdCur->u32Addr & 0xFFFF00) >> 8,
                     (pstCmdqCmdCur->u32Addr & 0xFF) >> 1, pstCmdqCmdCur->u16Data, pstCmdqCmdCur->u16Mask,
                     pstCmdqCmdCur->u32Time);

            HAL_DISP_UTILITY_PollWait((void *)pstCmdqInf, pstCmdqCmdCur->u32Addr, pstCmdqCmdCur->u16Data,
                                      pstCmdqCmdCur->u16Mask, pstCmdqCmdCur->u32Time, pstCmdqCmdCur->bPollEq);
        }
    }

    u32CmdqBufferCnt = *pu32DirectCmdqCnt - pstCmdqCmdCur->u32CmdqBufIdx;
    pvCmdqBuffer     = (HAL_DISP_UTILITY_CmdReg_t *)(pvDirctCmdqBuffer
                                                 + (pstCmdqCmdCur->u32CmdqBufIdx * sizeof(HAL_DISP_UTILITY_CmdReg_t)));

    if (u32CmdqBufferCnt)
    {
        _HAL_DISP_UTILITY_AddWriteRegCmdqMaskMulti(pstCmdqInf, pvCmdqBuffer, u32CmdqBufferCnt);
    }
}
//-------------------------------------------------------------------------------------------------
//  Global Functions
//-------------------------------------------------------------------------------------------------

MI_U8 HAL_DISP_UTILITY_Init(MI_U32 u32UtilityId)
{
    MI_U8 bRet = 1;

    if (u32UtilityId >= HAL_DISP_UTILITY_CMDQ_NUM)
    {
        DISP_ERR("%s %d, UtiId(%d) is bigger than CMDQ NUM (%d)\n", __FUNCTION__, __LINE__, u32UtilityId,
                 HAL_DISP_UTILITY_CMDQ_NUM);
        bRet = 0;
    }
    else
    {
        if (!_HAL_DISP_UTILITY_GetInitFlagByIdx(u32UtilityId))
        {
            g_stDispUtilityCmdqCtx[u32UtilityId].s32UtilityId = u32UtilityId;

            g_stDispUtilityCmdqCtx[u32UtilityId].u16RegFlipCnt  = 0;
            g_stDispUtilityCmdqCtx[u32UtilityId].u16WaitDoneCnt = 0;

            g_stDispUtilityCmdqCtx[u32UtilityId].u32DirectCmdqCnt = 0;
            g_stDispUtilityCmdqCtx[u32UtilityId].pvDirectCmdqBuffer =
                CamOsMemAlloc(sizeof(HAL_DISP_UTILITY_CmdReg_t) * HAL_DISP_UTILITY_DIRECT_CMDQ_CNT);

            if (g_stDispUtilityCmdqCtx[u32UtilityId].pvDirectCmdqBuffer == NULL)
            {
                bRet = 0;
                DISP_ERR("%s %d, Cmdq:%d, DirectCmdqBUffer Memallc Failn", __FUNCTION__, __LINE__, u32UtilityId);
            }

            if (DRV_DISP_OS_CreateMutex(&g_stDispUtilityCmdqCtx[u32UtilityId].stMutxCfg) == 0)
            {
                bRet = 0;
                DISP_ERR("%s %d, CreateMutex Fail\n", __FUNCTION__, __LINE__);
            }

            g_stDispUtilityCmdqCtx[u32UtilityId].u32CmdqCmdCnt = 0;
            g_stDispUtilityCmdqCtx[u32UtilityId].pvCmdqCmdBuffer =
                CamOsMemAlloc(sizeof(HAL_DISP_UTILITY_CmdqCmd_t) * HAL_DISP_UTILITY_CMDQ_CMD_CNT);

            if (g_stDispUtilityCmdqCtx[u32UtilityId].pvCmdqCmdBuffer == NULL)
            {
                bRet = 0;
                DISP_ERR("%s %d, Cmdq:%d, PollCmdq Buffer Memallc Failn", __FUNCTION__, __LINE__, u32UtilityId);
            }

            _HAL_DISP_UTILITY_SetInitFlagByIdx(u32UtilityId, 1);
        }
        else
        {
            DISP_DBG(DISP_DBG_LEVEL_UTILITY_CMDQ, "%s %d, CmdqCtx (%d) already Init\n", __FUNCTION__, __LINE__,
                     u32UtilityId);
            return 1;
        }
    }
    return bRet;
}

MI_U8 HAL_DISP_UTILITY_DeInit(MI_U32 u32CtxId)
{
    if (_HAL_DISP_UTILITY_GetInitFlagByIdx(u32CtxId))
    {
        g_stDispUtilityCmdqCtx[u32CtxId].s32UtilityId   = -1;
        g_stDispUtilityCmdqCtx[u32CtxId].pvCmdqInf      = NULL;
        g_stDispUtilityCmdqCtx[u32CtxId].u16RegFlipCnt  = 0;
        g_stDispUtilityCmdqCtx[u32CtxId].u16WaitDoneCnt = 0;
        g_stDispUtilityCmdqCtx[u32CtxId].u32CmdqCmdCnt  = 0;
        CamOsMemRelease(g_stDispUtilityCmdqCtx[u32CtxId].pvCmdqCmdBuffer);

        g_stDispUtilityCmdqCtx[u32CtxId].u32DirectCmdqCnt = 0;
        CamOsMemRelease(g_stDispUtilityCmdqCtx[u32CtxId].pvDirectCmdqBuffer);

        _HAL_DISP_UTILITY_SetInitFlagByIdx(u32CtxId, 0);

        DRV_DISP_OS_DestroyMutex(&g_stDispUtilityCmdqCtx[u32CtxId].stMutxCfg);
    }

    return 1;
}

MI_U8 HAL_DISP_UTILITY_GetCmdqContext(void **pvCtx, MI_U32 u32CtxId)
{
    MI_U8 bRet = 1;
    if (g_u32DispUtilityCmdqInitFlag)
    {
        *pvCtx = (void *)&g_stDispUtilityCmdqCtx[u32CtxId];
    }
    else
    {
        *pvCtx = NULL;
        bRet   = 0;
        DISP_ERR("%s %d, No Disp Utility Init\n", __FUNCTION__, __LINE__);
    }
    return bRet;
}

void HAL_DISP_UTILITY_SetCmdqInf(void *pCmdqInf, MI_U32 u32CtxId)
{
    g_stDispUtilityCmdqCtx[u32CtxId].pvCmdqInf = pCmdqInf;
}

MI_U16 HAL_DISP_UTILITY_R2BYTEDirect(MI_U32 u32Reg) // NOLINT
{
    MI_U16 u16RetVal = 0;

    u16RetVal = R2BYTE(u32Reg);
    return u16RetVal;
}

MI_U16 HAL_DISP_UTILITY_R2BYTEMaskDirect(MI_U32 u32Reg, MI_U16 u16Mask) // NOLINT
{
    MI_U16 u16RetVal = 0;

    u16RetVal = R2BYTEMSK(u32Reg, u16Mask);
    return u16RetVal;
}

void HAL_DISP_UTILITY_W2BYTEDirect(MI_U32 u32Reg, MI_U16 u16Val) // NOLINT
{
    W2BYTE(u32Reg, u16Val);
    DISP_DBG(DISP_DBG_LEVEL_UTILITY_DW_HWRD, "wriu -w 0x%04x%02x 0x%04hx \n", (u32Reg >> 8), (u32Reg & 0xFF), u16Val);
}

void HAL_DISP_UTILITY_W2BYTE(MI_U32 u32Reg, MI_U16 u16Val, void *pvCtxIn)
{
    HAL_DISP_UTILITY_CmdqContext_t *pstCmdqCtx = (HAL_DISP_UTILITY_CmdqContext_t *)pvCtxIn;

    if (pstCmdqCtx && HAL_DISP_UTILITY_GetRegAccessMode((MI_U32)pstCmdqCtx->s32UtilityId) == E_MI_DISP_REG_ACCESS_CMDQ)
    {
        HAL_DISP_UTILITY_W2BYTEMSKDirectCmdq(pvCtxIn, u32Reg, u16Val, 0xFFFF);
    }
    else
    {
        HAL_DISP_UTILITY_W2BYTEDirect(u32Reg, u16Val);
    }
}

void HAL_DISP_UTILITY_W2BYTEMSKDirect(MI_U32 u32Reg, MI_U16 u16Val, MI_U16 u16Mask) // NOLINT
{
    W2BYTEMSK(u32Reg, u16Val, u16Mask);
    DISP_DBG(DISP_DBG_LEVEL_UTILITY_DW_HWRD, "wriu -w 0x%04x%02x 0x%04hx \n", (u32Reg >> 8), (u32Reg & 0xFF),
             ((u16Val & u16Mask) | HAL_DISP_UTILITY_R2BYTEMaskDirect(u32Reg, ~u16Mask)));
}

void HAL_DISP_UTILITY_W2BYTEMSK(MI_U32 u32Reg, MI_U16 u16Val, MI_U16 u16Mask, void *pvCtxIn)
{
    HAL_DISP_UTILITY_CmdqContext_t *pstCmdqCtx = (HAL_DISP_UTILITY_CmdqContext_t *)pvCtxIn;

    if (pstCmdqCtx && HAL_DISP_UTILITY_GetRegAccessMode((MI_U32)pstCmdqCtx->s32UtilityId) == E_MI_DISP_REG_ACCESS_CMDQ)
    {
        HAL_DISP_UTILITY_W2BYTEMSKDirectCmdq(pvCtxIn, u32Reg, u16Val, u16Mask);
    }
    else
    {
        HAL_DISP_UTILITY_W2BYTEMSKDirect(u32Reg, u16Val, u16Mask);
    }
}

void HAL_DISP_UTILITY_AddW2BYTEMSK(MI_U32 u32Reg, MI_U16 u16Val, MI_U16 u16Mask, void *pvCtxIn)
{
    HAL_DISP_UTILITY_CmdqContext_t *pstCmdqCtx = (HAL_DISP_UTILITY_CmdqContext_t *)pvCtxIn;
    MHAL_CMDQ_CmdqInterface_t *     pstCmdqInf = NULL;
    HAL_DISP_UTILITY_CmdReg_t       stCmdReg;

    if (pstCmdqCtx)
    {
        pstCmdqInf = pstCmdqCtx->pvCmdqInf;
    }

    stCmdReg.u32Addr = u32Reg;
    stCmdReg.u16Data = u16Val;
    stCmdReg.u16Mask = u16Mask;

    DISP_DBG(DISP_DBG_LEVEL_UTILITY_FLIP, "%s %d, Addr [0x%04x%02x] Val [0x%04x] Mask [0x%04x]\n", __FUNCTION__,
             __LINE__, (stCmdReg.u32Addr & 0xFFFF00) >> 8, (stCmdReg.u32Addr & 0xFF) >> 1, stCmdReg.u16Data,
             stCmdReg.u16Mask);

    if (pstCmdqInf && pstCmdqInf->MHAL_CMDQ_WriteRegCmdqMaskMulti)
    {
        _HAL_DISP_UTILITY_AddWriteRegCmdqMaskMulti(pstCmdqInf, (void *)&stCmdReg, 1);
    }
    else
    {
        W2BYTEMSK(u32Reg, u16Val, u16Mask);
    }
}

void HAL_DISP_UTILITY_W2BYTEMSKDirectCmdq(void *pvCtxIn, MI_U32 u32Reg, MI_U16 u16Val, MI_U16 u16Mask)
{
    HAL_DISP_UTILITY_CmdReg_t *     pstCmdReg   = NULL;
    MI_U32 *                        pu32TblCnt  = NULL;
    void *                          pvTblBuffer = NULL;
    HAL_DISP_UTILITY_CmdqContext_t *pstCmdqCtx  = (HAL_DISP_UTILITY_CmdqContext_t *)pvCtxIn;

    if (HAL_DISP_UTILITY_GetRegAccessMode((MI_U32)pstCmdqCtx->s32UtilityId) == E_MI_DISP_REG_ACCESS_CMDQ)
    {
        DRV_DISP_OS_ObtainMutex(&pstCmdqCtx->stMutxCfg);

        pvTblBuffer = _HAL_DISP_UTILITY_GetDirectCmdqBuffer(pstCmdqCtx);
        pu32TblCnt  = _HAL_DISP_UTILITY_GetDirectCmdqCnt(pstCmdqCtx);

        if (*pu32TblCnt >= HAL_DISP_UTILITY_DIRECT_CMDQ_CNT)
        {
            DISP_ERR("%s %d, Cnt [%d] over max [%d] \n", __FUNCTION__, __LINE__, *pu32TblCnt,
                     HAL_DISP_UTILITY_DIRECT_CMDQ_CNT);
        }
        else
        {
            pstCmdReg =
                (HAL_DISP_UTILITY_CmdReg_t *)(pvTblBuffer + ((*pu32TblCnt) * sizeof(HAL_DISP_UTILITY_CmdReg_t)));
            pstCmdReg->u32Addr = u32Reg;
            pstCmdReg->u16Mask = u16Mask;
            pstCmdReg->u16Data = u16Val;
            (*pu32TblCnt)++;
        }

        DRV_DISP_OS_ReleaseMutex(&pstCmdqCtx->stMutxCfg);
    }
    else
    {
        W2BYTEMSK(u32Reg, u16Val, u16Mask);
    }
}
void HAL_DISP_UTILITY_KickoffCmdq(void *pvCtxIn)
{
    MHAL_CMDQ_CmdqInterface_t *     pstCmdqInf = NULL;
    HAL_DISP_UTILITY_CmdqContext_t *pstCmdqCtx = pvCtxIn;

    pstCmdqInf = pstCmdqCtx->pvCmdqInf;
    if (pstCmdqInf && pstCmdqInf->MHAL_CMDQ_KickOffCmdq)
    {
        pstCmdqInf->MHAL_CMDQ_KickOffCmdq(pstCmdqInf);
    }
}
void HAL_DISP_UTILITY_W2BYTEMSKDirectCmdqWrite(void *pvCtxIn)
{
    MI_U32                          i                 = 0;
    MI_U32                          j                 = 0;
    HAL_DISP_UTILITY_CmdReg_t *     pstCmdReg         = 0;
    HAL_DISP_UTILITY_CmdqContext_t *pstCmdqCtx        = (HAL_DISP_UTILITY_CmdqContext_t *)pvCtxIn;
    HAL_DISP_UTILITY_CmdqCmd_t *    pstCmdqCmdCur     = NULL;
    MHAL_CMDQ_CmdqInterface_t *     pstCmdqInf        = NULL;
    MI_U32 *                        pu32DirectCmdqCnt = NULL;
    void *                          pvDirctCmdqBuffer = NULL;
    MI_U32 *                        pu32CmdqCmdCnt    = NULL;
    void *                          pvCmdqCmdBuffer   = NULL;

    if (pstCmdqCtx)
    {
        pstCmdqInf = pstCmdqCtx->pvCmdqInf;
    }

    DRV_DISP_OS_ObtainMutex(&pstCmdqCtx->stMutxCfg);

    pvDirctCmdqBuffer = _HAL_DISP_UTILITY_GetDirectCmdqBuffer(pstCmdqCtx);
    pu32DirectCmdqCnt = _HAL_DISP_UTILITY_GetDirectCmdqCnt(pstCmdqCtx);

    pvCmdqCmdBuffer = _HAL_DISP_UTILITY_GetCmdqCmdBuffer(pstCmdqCtx);
    pu32CmdqCmdCnt  = _HAL_DISP_UTILITY_GetCmdqCmdCnt(pstCmdqCtx);

    if (pvDirctCmdqBuffer && *pu32DirectCmdqCnt > 0)
    {
        if (pstCmdqInf && pstCmdqInf->MHAL_CMDQ_WriteRegCmdqMaskMulti)
        {
            DISP_DBG(DISP_DBG_LEVEL_UTILITY_FLIP, "%s %d, Use Cmdq_%d \n", __FUNCTION__, __LINE__,
                     pstCmdqCtx->s32UtilityId);

            if (pvCmdqCmdBuffer && *pu32CmdqCmdCnt)
            {
                _HAL_DISP_UTILITY_DirectCmdqWriteWithCmdqCmd(pvCtxIn);
            }
            else
            {
                _HAL_DISP_UTILITY_AddWriteRegCmdqMaskMulti(pstCmdqInf, pvDirctCmdqBuffer, *pu32DirectCmdqCnt);
            }
        }
        else
        {
            DISP_DBG(DISP_DBG_LEVEL_UTILITY_FLIP, "%s %d, Use RIU\n", __FUNCTION__, __LINE__);

            for (i = 0, j = 0; i < *pu32DirectCmdqCnt; i++)
            {
                if (*pu32CmdqCmdCnt)
                {
                    pstCmdqCmdCur =
                        (HAL_DISP_UTILITY_CmdqCmd_t *)(pvCmdqCmdBuffer + (j * sizeof(HAL_DISP_UTILITY_CmdqCmd_t)));

                    if (pstCmdqCmdCur->u32CmdqBufIdx == i)
                    {
                        if (pstCmdqCmdCur->u32Addr == HAL_DISP_UTILITY_CMDQ_DELAY_CMD)
                        {
                            DISP_DBG(DISP_DBG_LEVEL_UTILITY_FLIP, "%s %d, DelayCmd [%d]\n", __FUNCTION__, __LINE__,
                                     pstCmdqCmdCur->u32Time);
                            HAL_DISP_UTILITY_AddDelayCmd(pvCtxIn, pstCmdqCmdCur->u32Time);
                        }
                        else if (pstCmdqCmdCur->u32Addr == HAL_DISP_UTILITY_CMDQ_WAITDONE_CMD)
                        {
                            DISP_DBG(DISP_DBG_LEVEL_UTILITY_FLIP, "%s %d, WaitDone\n", __FUNCTION__, __LINE__);
                            HAL_DISP_UTILITY_AddWaitCmd(pvCtxIn);
                        }
                        else
                        {
                            DISP_DBG(DISP_DBG_LEVEL_UTILITY_FLIP,
                                     "%s %d, Poll [0x%04x%02x] Val [0x%04x] Mask [0x%04x]\n", __FUNCTION__, __LINE__,
                                     (pstCmdqCmdCur->u32Addr & 0xFFFF00) >> 8, (pstCmdqCmdCur->u32Addr & 0xFF) >> 1,
                                     pstCmdqCmdCur->u16Data, pstCmdqCmdCur->u16Mask);

                            HAL_DISP_UTILITY_PollWait(pstCmdqInf, pstCmdqCmdCur->u32Addr, pstCmdqCmdCur->u16Data,
                                                      pstCmdqCmdCur->u16Mask, pstCmdqCmdCur->u32Time,
                                                      pstCmdqCmdCur->bPollEq);
                        }
                        j++;
                    }
                }

                pstCmdReg = (HAL_DISP_UTILITY_CmdReg_t *)(pvDirctCmdqBuffer + (i * sizeof(HAL_DISP_UTILITY_CmdReg_t)));
                DISP_DBG(DISP_DBG_LEVEL_UTILITY_FLIP, "%s %d, Addr [0x%04x%02x] Val [0x%04x] Mask [0x%04x]\n",
                         __FUNCTION__, __LINE__, (pstCmdReg->u32Addr & 0xFFFF00) >> 8, (pstCmdReg->u32Addr & 0xFF) >> 1,
                         pstCmdReg->u16Data, pstCmdReg->u16Mask);

                DISP_DBG(DISP_DBG_LEVEL_UTILITY_FLIP_HWRD, "wriu -w 0x%04x%02x 0x%04hx \n", (pstCmdReg->u32Addr >> 8),
                         (pstCmdReg->u32Addr & 0xFF),
                         ((pstCmdReg->u16Data & pstCmdReg->u16Mask)
                          | HAL_DISP_UTILITY_R2BYTEMaskDirect(pstCmdReg->u32Addr, ~pstCmdReg->u16Mask)));
                W2BYTEMSK(pstCmdReg->u32Addr, pstCmdReg->u16Data, pstCmdReg->u16Mask);
            }
        }

        (*pu32DirectCmdqCnt) = 0;
        (*pu32CmdqCmdCnt)    = 0;
    }

    DRV_DISP_OS_ReleaseMutex(&pstCmdqCtx->stMutxCfg);
}

void HAL_DISP_UTILITY_WriteDelayCmd(MI_U32 u32PollingTime, void *pvCtxIn)
{
    HAL_DISP_UTILITY_CmdqCmd_t *    pstCmdqCmd        = NULL;
    MI_U32 *                        pu32DirectCmdqCnt = NULL;
    MI_U32 *                        pu32CmdqCmdCnt    = NULL;
    void *                          pvTblBuffer       = NULL;
    HAL_DISP_UTILITY_CmdqContext_t *pstCmdqCtx        = (HAL_DISP_UTILITY_CmdqContext_t *)pvCtxIn;

    pvTblBuffer       = _HAL_DISP_UTILITY_GetCmdqCmdBuffer(pstCmdqCtx);
    pu32DirectCmdqCnt = _HAL_DISP_UTILITY_GetDirectCmdqCnt(pstCmdqCtx);
    pu32CmdqCmdCnt    = _HAL_DISP_UTILITY_GetCmdqCmdCnt(pstCmdqCtx);

    if (HAL_DISP_UTILITY_GetRegAccessMode((MI_U32)pstCmdqCtx->s32UtilityId) == E_MI_DISP_REG_ACCESS_CMDQ)
    {
        if (*pu32CmdqCmdCnt >= HAL_DISP_UTILITY_CMDQ_CMD_CNT)
        {
            DISP_ERR("%s %d, CmdqPollCmd Cnt [%d] over max [%d] \n", __FUNCTION__, __LINE__, *pu32CmdqCmdCnt,
                     HAL_DISP_UTILITY_CMDQ_CMD_CNT);

            return;
        }

        pstCmdqCmd =
            (HAL_DISP_UTILITY_CmdqCmd_t *)(pvTblBuffer + ((*pu32CmdqCmdCnt) * sizeof(HAL_DISP_UTILITY_CmdqCmd_t)));
        pstCmdqCmd->u32CmdqBufIdx = *pu32DirectCmdqCnt;
        pstCmdqCmd->u32Addr       = HAL_DISP_UTILITY_CMDQ_DELAY_CMD;
        pstCmdqCmd->u16Mask       = 0xFFFF;
        pstCmdqCmd->u16Data       = 0xFFFF;
        pstCmdqCmd->u32Time       = u32PollingTime;
        pstCmdqCmd->bPollEq       = 0;
        (*pu32CmdqCmdCnt)++;
    }
}

MI_U8 HAL_DISP_UTILITY_PollWait(void *pvCmdqInf, MI_U32 u32Reg, MI_U16 u16Val, MI_U16 u16Mask, MI_U32 u32PollTime,
                                MI_U8 bPollEq)

{
    MHAL_CMDQ_CmdqInterface_t *pstCmdqInf = (MHAL_CMDQ_CmdqInterface_t *)pvCmdqInf;
    MI_S32                     s32CmdqRet = 0;
    MI_U32                     u32WaitCnt = 0;
    MI_U8                      bRet       = 1;

    DISP_DBG(DISP_DBG_LEVEL_UTILITY_FLIP, "%s %d, Poll [0x%04x%02x] Val [0x%04x] Mask [0x%04x] Time:%d, PollEq:%d\n",
             __FUNCTION__, __LINE__, (u32Reg & 0xFFFF00) >> 8, (u32Reg & 0xFF) >> 1, u16Val, u16Mask, u32PollTime,
             bPollEq);

    if (pstCmdqInf == NULL || pstCmdqInf->MHAL_CMDQ_CmdqPollRegBits == NULL)
    {
        while ((bPollEq && (HAL_DISP_UTILITY_R2BYTEMaskDirect(u32Reg, u16Mask) != (u16Val)))
               || (!bPollEq && (HAL_DISP_UTILITY_R2BYTEMaskDirect(u32Reg, u16Mask) == (u16Val))))
        {
            if (++u32WaitCnt > 2000)
            {
                return 1;
            }
            CamOsUsSleep(((u32PollTime / (1000 * 2000)) ? (u32PollTime / (1000 * 2000)) : 1)); // Wait time ns/1000
        }
    }
    else
    {
        if (u32PollTime)
        {
            s32CmdqRet =
                pstCmdqInf->MHAL_CMDQ_CmdqPollRegBits_ByTime(pstCmdqInf, u32Reg, u16Val, u16Mask, bPollEq, u32PollTime);
            if (s32CmdqRet != 0 /*MHAL_SUCCESS*/)
            {
                bRet = 0;
                DISP_ERR("%s %d, CmdqPollRegBits_ByTime [%u] Fail\n", __FUNCTION__, __LINE__, u32PollTime);
            }
        }
        else
        {
            s32CmdqRet = pstCmdqInf->MHAL_CMDQ_CmdqPollRegBits(pstCmdqInf, u32Reg, u16Val, u16Mask, bPollEq);
            if (s32CmdqRet != 0 /*MHAL_SUCCESS*/)
            {
                bRet = 0;
                DISP_ERR("%s %d, MHAL_CMDQ_CmdqPollRegBits Fail\n", __FUNCTION__, __LINE__);
            }
        }
    }
    return bRet;
}

MI_U8 HAL_DISP_UTILITY_AddWaitCmd(void *pvCtxIn)
{
    HAL_DISP_UTILITY_CmdqContext_t *pstCmdqCtx  = (HAL_DISP_UTILITY_CmdqContext_t *)pvCtxIn;
    MHAL_CMDQ_CmdqInterface_t *     pstCmdqInf  = NULL;
    MHAL_CMDQ_EventId_e             enCmdqEvent = 0;
    MI_S32                          s32CmdqRet  = 0;
    MI_U8                           bRet        = 1;

    if (HAL_DISP_UTILITY_GetRegAccessMode((MI_U32)pstCmdqCtx->s32UtilityId) == E_MI_DISP_REG_ACCESS_CMDQ)
    {
        if (pstCmdqCtx)
        {
            pstCmdqInf = pstCmdqCtx->pvCmdqInf;
        }

        if (pstCmdqInf && pstCmdqInf->MHAL_CMDQ_CmdqAddWaitEventCmd)
        {
            enCmdqEvent = HAL_DISP_UTILITY_CMDQDEV_WAIT_DONE_EVENT(pstCmdqCtx->s32UtilityId);
            s32CmdqRet  = pstCmdqInf->MHAL_CMDQ_CmdqAddWaitEventCmd(pstCmdqInf, enCmdqEvent);
            if (s32CmdqRet != 0 /*MHAL_SUCCESS*/)
            {
                bRet = 0;
                DISP_ERR("%s %d, CmdqAddWaitEventCmd Fail\n", __FUNCTION__, __LINE__);
            }
            else
            {
                DISP_DBG(DISP_DBG_LEVEL_UTILITY_FLIP, "%s %d\n", __FUNCTION__, __LINE__);
            }
        }
        else
        {
            DISP_DBG(DISP_DBG_LEVEL_UTILITY_FLIP, "%s %d, NULL CmdqInf\n", __FUNCTION__, __LINE__);
        }
    }
    return bRet;
}

MI_U8 HAL_DISP_UTILITY_AddDelayCmd(void *pvCtxIn, MI_U32 u32PollingTime)
{
    HAL_DISP_UTILITY_CmdqContext_t *pstCmdqCtx = (HAL_DISP_UTILITY_CmdqContext_t *)pvCtxIn;
    MHAL_CMDQ_CmdqInterface_t *     pstCmdqInf = NULL;
    MI_S32                          s32CmdqRet = 0;
    MI_U8                           bRet       = 1;

    if (pstCmdqCtx)
    {
        pstCmdqInf = pstCmdqCtx->pvCmdqInf;
    }

    if (pstCmdqInf && pstCmdqInf->MHAL_CMDQ_CmdDelay)
    {
        s32CmdqRet = pstCmdqInf->MHAL_CMDQ_CmdDelay(pstCmdqInf, 1000 * u32PollingTime);
        if (s32CmdqRet != 0 /*MHAL_SUCCESS*/)
        {
            bRet = 0;
            DISP_ERR("%s %d, CmdDelay Fail\r\n", __FUNCTION__, __LINE__);
        }
    }
    else
    {
        // if need
        if (u32PollingTime > 500)
        {
            CamOsUsDelay(u32PollingTime);
        }
    }

    return bRet;
}

void HalDispUtilityReadBankCpyToBuffer(u32 u32Bank, void *pBuf)
{
    void *pvBankaddr;

    if (g_u32DispUtilityCmdqInitFlag)
    {
        pvBankaddr = RIU_GET_ADDR(u32Bank);
        CamOsMemcpy((void *)pBuf, pvBankaddr, DISP_BANK_SIZE);
    }
}

void HAL_DISP_UTILITY_SetRegAccessMode(MI_U32 u32Id, MI_U32 u32Mode)
{
    g_eDispRegAccessMode[u32Id] = u32Mode;
}

MI_U32 HAL_DISP_UTILITY_GetRegAccessMode(MI_U32 u32Id)
{
    return (MI_U32)g_eDispRegAccessMode[u32Id];
}
