/*
 * hal_disp_irq.c- Sigmastar
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
#define _HAL_DISP_IRQ_C_

//-------------------------------------------------------------------------------------------------
//  Include Files
//-------------------------------------------------------------------------------------------------
#include "mi_disp.h"
#include "mi_disp_impl_datatype.h"
#include "drv_disp_os.h"
#include "hal_disp_include.h"
#include "disp_debug.h"
#include "drv_disp_ctx.h"
//-------------------------------------------------------------------------------------------------
//  Defines & Macro
//-------------------------------------------------------------------------------------------------

#define HAL_DISP_IRQ_SC_INT7_MASK_REG  REG_DISP_TTL_68_L
#define HAL_DISP_IRQ_SC_INT7_FLAG_REG  REG_DISP_TTL_6B_L
#define HAL_DISP_IRQ_SC_INT7_CLEAR_REG REG_DISP_TTL_6A_L

//-------------------------------------------------------------------------------------------------
//  structure
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//  Variable
//-------------------------------------------------------------------------------------------------
extern MI_DISP_IMPL_MhalDeviceConfig_t g_stInterCfg[HAL_DISP_DEVICE_MAX];

//-------------------------------------------------------------------------------------------------
//  Local Functions
//-------------------------------------------------------------------------------------------------
static __inline void _HAL_DISP_IRQ_EnableIsr(HAL_DISP_IRQ_Type_e enType, MI_U8 *pbEn, MI_U32 u32DevId)
{
    MI_U16 u16Val, u16Msk;
    MI_U8  bEnable = *pbEn;
    UNUSED(u32DevId);

    u16Val = 0;
    u16Msk = 0;

    DISP_DBG(DISP_DBG_LEVEL_IRQ, "%s %d, Type:%x, En:%d\n", __FUNCTION__, __LINE__, enType, bEnable);
    if (enType & E_HAL_DISP_IRQ_TYPE_INPORT_VSYNC)
    {
        u16Val |= bEnable ? 0 : HAL_DISP_IRQ_TTL_VSYNC_BIT;
        u16Msk |= HAL_DISP_IRQ_TTL_VSYNC_MSK;
    }

    HAL_DISP_UTILITY_W2BYTEMSKDirect(HAL_DISP_IRQ_SC_INT7_MASK_REG, u16Val, u16Msk);
}

static __inline void _HAL_DISP_IRQ_GetFlag(HAL_DISP_IRQ_Type_e enType, MI_U32 *pu32Flag, MI_U32 u32DevId)
{
    MI_U16 u16Val, u16IrqMsk;
    UNUSED(u32DevId);

    *pu32Flag = 0;

    u16Val    = R2BYTE(HAL_DISP_IRQ_SC_INT7_FLAG_REG);
    u16IrqMsk = R2BYTE(HAL_DISP_IRQ_SC_INT7_MASK_REG);

    if (enType & E_HAL_DISP_IRQ_TYPE_INPORT_VSYNC)
    {
        *pu32Flag |= (u16Val & HAL_DISP_IRQ_TTL_VSYNC_MSK) ? E_HAL_DISP_IRQ_TYPE_INPORT_VSYNC : 0;
    }

    DISP_DBG(DISP_DBG_LEVEL_IRQ, "%s %d, enType:%x, Flag:%x (%04x %04x)\n", __FUNCTION__, __LINE__, enType, *pu32Flag,
             u16IrqMsk, u16Val);
    DISP_DBG_VAL(u16IrqMsk);
    DISP_DBG_VAL(u16Val);
}

static __inline void _HAL_DISP_IRQ_Clear(HAL_DISP_IRQ_Type_e enType, MI_U32 u32DevId)
{
    MI_U16 u16Val, u16Msk;
    MI_U64 u64Time = DRV_DISP_OS_GetSystemTimeStamp();

    DISP_DBG(DISP_DBG_LEVEL_IRQ, "%s %d, enType:%x\n", __FUNCTION__, __LINE__, enType);

    u16Val = 0;
    u16Msk = 0;
    if (enType & E_HAL_DISP_IRQ_TYPE_INPORT_VSYNC)
    {
        u16Val |= HAL_DISP_IRQ_TTL_VSYNC_BIT;
        u16Msk |= HAL_DISP_IRQ_TTL_VSYNC_MSK;
        DISP_STATISTIC_VAL(g_stDispIrqHist.stWorkingHist.stDevHist[u32DevId].u32VsCnt++);
        DISP_STATISTIC_VAL(g_stDispIrqHist.stWorkingHist.stDevHist[u32DevId].u64TimeStamp = u64Time);
    }

    // Clear, 1->0
    HAL_DISP_UTILITY_W2BYTEMSKDirect(HAL_DISP_IRQ_SC_INT7_CLEAR_REG, u16Val, u16Msk);
    HAL_DISP_UTILITY_W2BYTEMSKDirect(HAL_DISP_IRQ_SC_INT7_CLEAR_REG, 0, u16Msk);
    DISP_DBG_VAL(u64Time);
}

static __inline void _HAL_DISP_IRQ_GetId(MI_U8 *pu8Id, MI_U32 u32DevId)
{
    UNUSED(u32DevId);
    *pu8Id = HAL_DISP_IRQ_ID_DEVICE_0;
}

void HAL_DISP_IRQ_SetDevIntClr(MI_U32 u32DevId, MI_U16 val)
{
    MI_U32 u32ClrReg;
    UNUSED(u32DevId);

    u32ClrReg = HAL_DISP_IRQ_SC_INT7_CLEAR_REG;
    if (u32ClrReg)
    {
        HAL_DISP_UTILITY_W2BYTEMSKDirect(u32ClrReg, val, 0xFC00);
        HAL_DISP_UTILITY_W2BYTEMSKDirect(u32ClrReg + 2, DISP_BIT0, DISP_BIT0);
    }
}

//-------------------------------------------------------------------------------------------------
//  Global Functions
//-------------------------------------------------------------------------------------------------
MI_U8 HAL_DISP_IRQ_IoCtl(HAL_DISP_IRQ_IoctlConfig_t *pCfg)
{
    MI_U8                         bRet          = 1;
    DRV_DISP_CTX_Config_t *       pstDispCtx    = NULL;
    DRV_DISP_CTX_DeviceContain_t *pstDevContain = NULL;

    pstDispCtx = (DRV_DISP_CTX_Config_t *)pCfg->pDispCtx;

    if (pstDispCtx == NULL)
    {
        DISP_ERR("%s %d, Null DispCtx\n", __FUNCTION__, __LINE__);
        return 0;
    }

    if (pstDispCtx->enCtxType == E_DRV_DISP_CTX_TYPE_DEVICE)
    {
        pstDevContain = pstDispCtx->pstCtxContain->pstDevContain[pstDispCtx->u32ContainIdx];

        DISP_DBG(DISP_DBG_LEVEL_IRQ, "%s %d, ContainId:%d, DevId:%d, Ioctl:%s, IrqType:%s\n", __FUNCTION__, __LINE__,
                 pstDispCtx->u32ContainIdx, pstDevContain->u32DevId, PARSING_HAL_IRQ_IOCTL(pCfg->enIoctlType),
                 PARSING_HAL_IRQ_TYPE(pCfg->enIrqType));
    }

    switch (pCfg->enIoctlType)
    {
        case E_HAL_DISP_IRQ_IOCTL_ENABLE:
            _HAL_DISP_IRQ_EnableIsr(pCfg->enIrqType, (MI_U8 *)pCfg->pParam, pstDevContain->u32DevId);
            break;

        case E_HAL_DISP_IRQ_IOCTL_GET_FLAG:
            _HAL_DISP_IRQ_GetFlag(pCfg->enIrqType, (MI_U32 *)pCfg->pParam, pstDevContain->u32DevId);
            break;

        case E_HAL_DISP_IRQ_IOCTL_CLEAR:
            _HAL_DISP_IRQ_Clear(pCfg->enIrqType, pstDevContain->u32DevId);
            break;

        case E_HAL_DISP_IRQ_IOCTL_GET_ID:
            _HAL_DISP_IRQ_GetId((MI_U8 *)pCfg->pParam, pstDevContain->u32DevId);
            break;

        case E_HAL_DISP_IRQ_IOCTL_LCD_ENABLE:
        case E_HAL_DISP_IRQ_IOCTL_LCD_GET_FLAG:
        case E_HAL_DISP_IRQ_IOCTL_LCD_CLEAR:
        case E_HAL_DISP_IRQ_IOCTL_LCD_GET_ID:
        default:
            bRet = FALSE;
            DISP_ERR("%s %d, UnKnown Irq Iocl:%d\n", __FUNCTION__, __LINE__, pCfg->enIoctlType);
            break;
    }
    return bRet;
}
void HAL_DISP_IRQ_Init(void) {}
