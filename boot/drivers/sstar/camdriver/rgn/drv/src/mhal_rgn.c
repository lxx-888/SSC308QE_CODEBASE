/*
 * mhal_rgn.c - Sigmastar
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

#define __MHAL_RGN_C__
//-------------------------------------------------------------------------------------------------
//  Include Files
//-------------------------------------------------------------------------------------------------
#include "cam_os_wrapper.h"
#include "mhal_cmdq.h"
#include "drv_rgn_os_header.h"
#include "mhal_rgn_internal_datatype.h"
#include "mhal_rgn_datatype.h"
#include "mhal_rgn.h"

#include "hal_rgn_if.h"

#include "rgn_debug.h"
//-------------------------------------------------------------------------------------------------
//  Defines & Macro
//-------------------------------------------------------------------------------------------------
#define RGN_CHECK_CALLBACK(_pDev, _pOps, _pFunc, _errcode) \
    do                                                     \
    {                                                      \
        if (!_pDev || !_pDev->_pOps)                       \
        {                                                  \
            RGN_ERR("Dev not create.");                    \
            {                                              \
                _errcode;                                  \
            }                                              \
        }                                                  \
        if (!_pDev->_pOps->_pFunc)                         \
        {                                                  \
            {                                              \
                _errcode;                                  \
            }                                              \
        }                                                  \
    } while (0)

#define MHAL_RGN_INFO(_fmt, _args...) RGN_INFO(MHAL_RGN_DBG_LV_MHAL, _fmt, ##_args)

//-------------------------------------------------------------------------------------------------
//  structure
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//  Variable
//-------------------------------------------------------------------------------------------------
MS_U32 g_u32DebugLevel;

//-------------------------------------------------------------------------------------------------
// Functions
//-------------------------------------------------------------------------------------------------
void MHAL_RGN_SetDebugLevel(MS_U32 u32DbgLv)
{
    g_u32DebugLevel = u32DbgLv;
}

MS_S32 MHAL_RGN_GetChipCapOps(MHAL_RGN_ChipCapOps_t *pstChipCapOps)
{
    return HAL_RGN_IF_GetChipCapOps(pstChipCapOps);
}

MS_S32 MHAL_RGN_Create(const MHAL_RGN_DevAttr_t *pstDevAttr, void **pCtx)
{
    *pCtx = HAL_RGN_IF_Create(pstDevAttr);
    MHAL_RGN_INFO("(%d, %d, %d) -> %px", pstDevAttr->eDevType, pstDevAttr->eTargetType, pstDevAttr->u32Id, *pCtx);
    return *pCtx != NULL ? MHAL_SUCCESS : MHAL_FAILURE;
}

MS_S32 MHAL_RGN_Destory(const void *pCtx)
{
    HAL_RGN_IF_Destory(pCtx);
    MHAL_RGN_INFO("%px", pCtx);
    return MHAL_SUCCESS;
}

MS_S32 MHAL_RGN_Active(const void *pCtx)
{
    RGN_CHECK(pCtx, return MHAL_FAILURE, "Dev not create");
    MHAL_RGN_INFO("%px", pCtx);
    return HAL_RGN_IF_Active((MHAL_RGN_Dev_t *)pCtx);
}

MS_S32 MHAL_RGN_Deactive(const void *pCtx)
{
    RGN_CHECK(pCtx, return MHAL_FAILURE, "Dev not create");
    MHAL_RGN_INFO("%px", pCtx);
    return HAL_RGN_IF_Deactive((MHAL_RGN_Dev_t *)pCtx);
}

MS_S32 MHAL_RGN_SetCmdq(const void *pCtx, const MHAL_RGN_CmdqConfig_t *pstCmdqCfg)
{
    MHAL_RGN_Dev_t *pstRgnDev = (MHAL_RGN_Dev_t *)pCtx;
    RGN_CHECK(pstRgnDev && pstRgnDev->pstRgnCmdq, return MHAL_FAILURE, "Dev not create");
    pstRgnDev->pstRgnCmdq->pstCmdqInf = pstCmdqCfg->pstCmdqInf;
    MHAL_RGN_INFO("%px: %px", pCtx, pstCmdqCfg->pstCmdqInf);
    return MHAL_SUCCESS;
}

MS_S32 MHAL_RGN_Resume(const void *pCtx)
{
    MHAL_RGN_Dev_t *pstRgnDev = (MHAL_RGN_Dev_t *)pCtx;
    RGN_CHECK(pCtx, return MHAL_FAILURE, "Dev not create");
    MHAL_RGN_INFO("%px", pCtx);
    return pstRgnDev->pstOps->fpReset(pstRgnDev, E_MHAL_RESET_MD_REG_SYNC | E_MHAL_RESET_MD_SRAM_SYNC);
}

MS_S32 MHAL_RGN_Suspend(const void *pCtx)
{
    return MHAL_SUCCESS;
}

MS_S32 MHAL_RGN_SetResolution(const char *pCtx, const MHAL_RGN_ResolutionConfig_t *pstResolutionCfg)
{
    MHAL_RGN_Dev_t *pstRgnDev = (MHAL_RGN_Dev_t *)(pCtx);
    RGN_CHECK_CALLBACK(pstRgnDev, pstOps, fpSetResolution, return MHAL_FAILURE);
    MHAL_RGN_INFO("%px: %dx%d", pCtx, pstResolutionCfg->u32Width, pstResolutionCfg->u32Height);
    return pstRgnDev->pstOps->fpSetResolution(pstRgnDev, pstResolutionCfg);
}

MS_S32 MHAL_RGN_Process(const void *pCtx)
{
    MHAL_RGN_Dev_t *pstRgnDev = (MHAL_RGN_Dev_t *)(pCtx);
    RGN_CHECK_CALLBACK(pstRgnDev, pstOps, fpProcess, return MHAL_FAILURE);
    MHAL_RGN_INFO("%px", pCtx);
    return pstRgnDev->pstOps->fpProcess(pstRgnDev);
}

MS_S32 MHAL_RGN_GopSetPalette(const void *pCtx, const MHAL_RGN_GopPaletteConfig_t *pstPaletteCfg)
{
    MHAL_RGN_GopDev_t *pstGopDev = CAM_OS_CONTAINER_OF(pCtx, MHAL_RGN_GopDev_t, stRgnDev);
    RGN_CHECK_CALLBACK(pstGopDev, pstOps, fpSetPalette, return MHAL_FAILURE);
    MHAL_RGN_INFO("%px: [%2x]=0x%02x%02x%02x%02x", pCtx, pstPaletteCfg->u8Idx, pstPaletteCfg->stEntry.u8A,
                  pstPaletteCfg->stEntry.u8R, pstPaletteCfg->stEntry.u8G, pstPaletteCfg->stEntry.u8B);
    return pstGopDev->pstOps->fpSetPalette(pstGopDev, pstPaletteCfg);
}

MS_S32 MHAL_RGN_GopSetPaletteTbl(const void *pCtx, const MHAL_RGN_GopPaletteTblConfig_t *pstPaletteTblCfg)
{
    MHAL_RGN_GopDev_t *pstGopDev = CAM_OS_CONTAINER_OF(pCtx, MHAL_RGN_GopDev_t, stRgnDev);
    RGN_CHECK_CALLBACK(pstGopDev, pstOps, fpSetPaletteTbl, return MHAL_FAILURE);
    MHAL_RGN_INFO("%px: %px, %d", pCtx, pstPaletteTblCfg->pstEntry, pstPaletteTblCfg->u32EntryNum);
    return pstGopDev->pstOps->fpSetPaletteTbl(pstGopDev, pstPaletteTblCfg);
}

MS_S32 MHAL_RGN_GopSetBaseWindow(const void *pCtx, const MHAL_RGN_GopStretchWinConfig_t *pstStretchWinCfg)
{
    MHAL_RGN_GopDev_t *pstGopDev = CAM_OS_CONTAINER_OF(pCtx, MHAL_RGN_GopDev_t, stRgnDev);
    RGN_CHECK_CALLBACK(pstGopDev, pstOps, fpSetBaseWindow, return MHAL_FAILURE);
    MHAL_RGN_INFO("%px: (%d, %d, %dx%d)->(%d, %d, %dx%d)", pCtx, pstStretchWinCfg->stSrcWinCfg.u32X,
                  pstStretchWinCfg->stSrcWinCfg.u32Y, pstStretchWinCfg->stSrcWinCfg.u32Width,
                  pstStretchWinCfg->stSrcWinCfg.u32Height, pstStretchWinCfg->stDestWinCfg.u32X,
                  pstStretchWinCfg->stDestWinCfg.u32Y, pstStretchWinCfg->stDestWinCfg.u32Width,
                  pstStretchWinCfg->stDestWinCfg.u32Height);
    return pstGopDev->pstOps->fpSetBaseWindow(pstGopDev, pstStretchWinCfg);
}

MS_S32 MHAL_RGN_GopSetColorKey(const void *pCtx, const MHAL_RGN_GopColorKeyConfig_t *pstColorKeyCfg)
{
    MHAL_RGN_GopDev_t *pstGopDev = CAM_OS_CONTAINER_OF(pCtx, MHAL_RGN_GopDev_t, stRgnDev);
    RGN_CHECK_CALLBACK(pstGopDev, pstOps, fpSetColorKey, return MHAL_FAILURE);
    MHAL_RGN_INFO("%lx: %d, 0x%02x%02x%02x", pCtx, pstColorKeyCfg->bEn, pstColorKeyCfg->bEn, pstColorKeyCfg->u8R,
                  pstColorKeyCfg->u8G, pstColorKeyCfg->u8B);
    return pstGopDev->pstOps->fpSetColorKey(pstGopDev, pstColorKeyCfg);
}

MS_S32 MHAL_RGN_GopSetAlphaZeroOpaque(const void *                               pCtx,
                                      const MHAL_RGN_GopAlphaZeroOpaqueConfig_t *pstAlphaZeroOpaqueCfg)
{
    MHAL_RGN_GopDev_t *pstGopDev = CAM_OS_CONTAINER_OF(pCtx, MHAL_RGN_GopDev_t, stRgnDev);
    RGN_CHECK_CALLBACK(pstGopDev, pstOps, fpSetAlphaZeroOpaque, return MHAL_FAILURE);
    MHAL_RGN_INFO("%px: %d, %d, %d", pCtx, pstAlphaZeroOpaqueCfg->ePixelFormat, pstAlphaZeroOpaqueCfg->bConstAlpha,
                  pstAlphaZeroOpaqueCfg->bEn);
    return pstGopDev->pstOps->fpSetAlphaZeroOpaque(pstGopDev, pstAlphaZeroOpaqueCfg);
}

MS_S32 MHAL_RGN_GopSetOutputFormat(const void *pCtx, const MHAL_RGN_GopOutputFmtConfig_t *pstFmtCfg)
{
    MHAL_RGN_GopDev_t *pstGopDev = CAM_OS_CONTAINER_OF(pCtx, MHAL_RGN_GopDev_t, stRgnDev);
    RGN_CHECK_CALLBACK(pstGopDev, pstOps, fpSetOutputFormat, return MHAL_FAILURE);
    MHAL_RGN_INFO("%px: %d", pCtx, pstFmtCfg->eFmt);
    return pstGopDev->pstOps->fpSetOutputFormat(pstGopDev, pstFmtCfg);
}

MS_S32 MHAL_RGN_GopSetContrast(const void *pCtx, const MHAL_RGN_GopContrastConfig_t *pstContrastCfg)
{
    MHAL_RGN_GopDev_t *pstGopDev = CAM_OS_CONTAINER_OF(pCtx, MHAL_RGN_GopDev_t, stRgnDev);
    RGN_CHECK_CALLBACK(pstGopDev, pstOps, fpSetContrast, return MHAL_FAILURE);
    MHAL_RGN_INFO("%px: %02x,%02x,%02x", pCtx, pstContrastCfg->u8Gain_R, pstContrastCfg->u8Gain_G,
                  pstContrastCfg->u8Gain_B);
    return pstGopDev->pstOps->fpSetContrast(pstGopDev, pstContrastCfg);
}

MS_S32 MHAL_RGN_GopSetBrightness(const void *pCtx, const MHAL_RGN_GopBrightnessConfig_t *pstBirghtnessCfg)
{
    MHAL_RGN_GopDev_t *pstGopDev = CAM_OS_CONTAINER_OF(pCtx, MHAL_RGN_GopDev_t, stRgnDev);
    RGN_CHECK_CALLBACK(pstGopDev, pstOps, fpSetBrightness, return MHAL_FAILURE);
    MHAL_RGN_INFO("%px: %d", pCtx, pstBirghtnessCfg->s16Brightness);
    return pstGopDev->pstOps->fpSetBrightness(pstGopDev, pstBirghtnessCfg);
}

MS_S32 MHAL_RGN_GopSetEncMode(const void *pCtx, const MHAL_RGN_GopEncFormatConfig_t *pstEncFmtCfg)
{
    MHAL_RGN_GopDev_t *pstGopDev = CAM_OS_CONTAINER_OF(pCtx, MHAL_RGN_GopDev_t, stRgnDev);
    RGN_CHECK_CALLBACK(pstGopDev, pstOps, fpSetEncMode, return MHAL_FAILURE);
    MHAL_RGN_INFO("%px: %d", pCtx, pstEncFmtCfg->eEncMode);
    return pstGopDev->pstOps->fpSetEncMode(pstGopDev, pstEncFmtCfg);
}

MS_S32 MHAL_RGN_GopSetDmaThreahold(const void *pCtx, const MHAL_RGN_GopDmaThreaholdConfig_t *pstDmaThrdCfg)
{
    MHAL_RGN_GopDev_t *pstGopDev = CAM_OS_CONTAINER_OF(pCtx, MHAL_RGN_GopDev_t, stRgnDev);
    RGN_CHECK_CALLBACK(pstGopDev, pstOps, fpSetDmaThreahold, return MHAL_FAILURE);
    MHAL_RGN_INFO("%px: %d", pCtx, pstDmaThrdCfg->u32Val);
    return pstGopDev->pstOps->fpSetDmaThreahold(pstGopDev, pstDmaThrdCfg);
}

MS_S32 MHAL_RGN_GopGwinSetPixelFormat(const void *pCtx, const MHAL_RGN_GopGwinPixelFmtConfig_t *pstPixelFmtCfg)
{
    MHAL_RGN_GopDev_t *pstGopDev = CAM_OS_CONTAINER_OF(pCtx, MHAL_RGN_GopDev_t, stRgnDev);
    RGN_CHECK_CALLBACK(pstGopDev, pstOps, fpGwinSetPixelFormat, return MHAL_FAILURE);
    MHAL_RGN_INFO("%px: [%2d]=%d", pCtx, pstPixelFmtCfg->u32GwinId, pstPixelFmtCfg->ePixelFormat);
    return pstGopDev->pstOps->fpGwinSetPixelFormat(pstGopDev, pstPixelFmtCfg);
}

MS_S32 MHAL_RGN_GopGwinSetPaletteIndex(const void *pCtx, const MHAL_RGN_GopGwinPaletteIdxConfig_t *pstPltIdxCfg)
{
    MHAL_RGN_GopDev_t *pstGopDev = CAM_OS_CONTAINER_OF(pCtx, MHAL_RGN_GopDev_t, stRgnDev);
    RGN_CHECK_CALLBACK(pstGopDev, pstOps, fpGwinSetPaletteIndex, return MHAL_FAILURE);
    MHAL_RGN_INFO("%px: [%2d]=%d", pCtx, pstPltIdxCfg->u32GwinId, pstPltIdxCfg->u8PaletteIdx);
    return pstGopDev->pstOps->fpGwinSetPaletteIndex(pstGopDev, pstPltIdxCfg);
}

MS_S32 MHAL_RGN_GopGwinSetWindow(const void *pCtx, const MHAL_RGN_GopGwinWinConfig_t *pstWinCfg)
{
    MHAL_RGN_GopDev_t *pstGopDev = CAM_OS_CONTAINER_OF(pCtx, MHAL_RGN_GopDev_t, stRgnDev);
    RGN_CHECK_CALLBACK(pstGopDev, pstOps, fpGwinSetWindow, return MHAL_FAILURE);
    MHAL_RGN_INFO("%px: [%2d]=(%d, %d, %dx%d, %d)", pCtx, pstWinCfg->u32GwinId, pstWinCfg->stWinCfg.u32X,
                  pstWinCfg->stWinCfg.u32Y, pstWinCfg->stWinCfg.u32Width, pstWinCfg->stWinCfg.u32Height,
                  pstWinCfg->u32Stride);
    return pstGopDev->pstOps->fpGwinSetWindow(pstGopDev, pstWinCfg);
}

MS_S32 MHAL_RGN_GopGwinSetBuffer(const void *pCtx, const MHAL_RGN_GopGwinBufferConfig_t *pstBufferCfg)
{
    MHAL_RGN_GopDev_t *pstGopDev = CAM_OS_CONTAINER_OF(pCtx, MHAL_RGN_GopDev_t, stRgnDev);
    RGN_CHECK_CALLBACK(pstGopDev, pstOps, fpGwinSetBuffer, return MHAL_FAILURE);
    MHAL_RGN_INFO("%px: [%2d]=(%lx,%d)", pCtx, pstBufferCfg->u32GwinId, pstBufferCfg->phyAddr,
                  pstBufferCfg->u16Xoffset);
    return pstGopDev->pstOps->fpGwinSetBuffer(pstGopDev, pstBufferCfg);
}

MS_S32 MHAL_RGN_GopGwinSetOnOff(const void *pCtx, const MHAL_RGN_GopGwinOnOffConfig_t *pstOnOffCfg)
{
    MHAL_RGN_GopDev_t *pstGopDev = CAM_OS_CONTAINER_OF(pCtx, MHAL_RGN_GopDev_t, stRgnDev);
    RGN_CHECK_CALLBACK(pstGopDev, pstOps, fpGwinSetOnOff, return MHAL_FAILURE);
    MHAL_RGN_INFO("%px: [%2d]=(%d)", pCtx, pstOnOffCfg->u32GwinId, pstOnOffCfg->bEn);
    return pstGopDev->pstOps->fpGwinSetOnOff(pstGopDev, pstOnOffCfg);
}

MS_S32 MHAL_RGN_GopGwinSetAlphaType(const void *pCtx, const MHAL_RGN_GopGwinAlphaConfig_t *pstAlphaCfg)
{
    MHAL_RGN_GopDev_t *pstGopDev = CAM_OS_CONTAINER_OF(pCtx, MHAL_RGN_GopDev_t, stRgnDev);
    RGN_CHECK_CALLBACK(pstGopDev, pstOps, fpGwinSetAlphaType, return MHAL_FAILURE);
    MHAL_RGN_INFO("%px: [%2d]=(%d, 0x%02x)", pCtx, pstAlphaCfg->u32GwinId, pstAlphaCfg->eAlphaType,
                  pstAlphaCfg->u8ConstAlphaVal);
    return pstGopDev->pstOps->fpGwinSetAlphaType(pstGopDev, pstAlphaCfg);
}

MS_S32 MHAL_RGN_GopSetArgb1555AlphaVal(const void *pCtx, const MHAL_RGN_GopGwinArgb1555AlphaConfig_t *pstAlphaCfg)
{
    MHAL_RGN_GopDev_t *pstGopDev = CAM_OS_CONTAINER_OF(pCtx, MHAL_RGN_GopDev_t, stRgnDev);
    RGN_CHECK_CALLBACK(pstGopDev, pstOps, fpSetArgb1555AlphaVal, return MHAL_FAILURE);
    MHAL_RGN_INFO("%px: [%2d]=(%d, 0x%02x)", pCtx, pstAlphaCfg->u32GwinId, pstAlphaCfg->eAlphaType,
                  pstAlphaCfg->u8AlphaVal);
    return pstGopDev->pstOps->fpSetArgb1555AlphaVal(pstGopDev, pstAlphaCfg);
}

MS_S32 MHAL_RGN_GopSetBlend(const void *pCtx, const MHAL_RGN_GopBlendConfig_t *pstBlendCfg)
{
    MHAL_RGN_GopDev_t *pstGopDev = CAM_OS_CONTAINER_OF(pCtx, MHAL_RGN_GopDev_t, stRgnDev);
    RGN_CHECK_CALLBACK(pstGopDev, pstOps, fpSetBlend, return MHAL_FAILURE);
    MHAL_RGN_INFO("%px: %d %d", pCtx, pstBlendCfg->eBlendType, pstBlendCfg->u16PlaneAlpha);
    return pstGopDev->pstOps->fpSetBlend(pstGopDev, pstBlendCfg);
}

MS_S32 MHAL_RGN_CoverSetPalette(const void *pCtx, const MHAL_RGN_CoverPaletteConfig_t *pstPaletteCfg)
{
    MHAL_RGN_CoverDev_t *pstCoverDev = CAM_OS_CONTAINER_OF(pCtx, MHAL_RGN_CoverDev_t, stRgnDev);
    RGN_CHECK_CALLBACK(pstCoverDev, pstOps, fpSetPalette, return MHAL_FAILURE);
    MHAL_RGN_INFO("%px: [%d]-(%02x, %02x, %02x)", pCtx, pstPaletteCfg->u8Idx, pstPaletteCfg->stEntry.u8Y,
                  pstPaletteCfg->stEntry.u8U, pstPaletteCfg->stEntry.u8V);
    return pstCoverDev->pstOps->fpSetPalette(pstCoverDev, pstPaletteCfg);
}

MS_S32 MHAL_RGN_CoverSetWindowBuffer(const void *pCtx, const MHAL_RGN_BufferConfig_t *pstBufferCfg)
{
    MHAL_RGN_CoverDev_t *pstCoverDev = CAM_OS_CONTAINER_OF(pCtx, MHAL_RGN_CoverDev_t, stRgnDev);
    RGN_CHECK_CALLBACK(pstCoverDev, pstOps, fpSetWinBuffer, return MHAL_FAILURE);
    MHAL_RGN_INFO("%px: 0x%lx, 0x%lx", pCtx, pstBufferCfg->phyAddr, pstBufferCfg->pAddr);
    return pstCoverDev->pstOps->fpSetWinBuffer(pstCoverDev, pstBufferCfg);
}

MS_S32 MHAL_RGN_CoverSetColor(const void *pCtx, const MHAL_RGN_CoverColorConfig_t *pstColorCfg)
{
    MHAL_RGN_CoverDev_t *pstCoverDev = CAM_OS_CONTAINER_OF(pCtx, MHAL_RGN_CoverDev_t, stRgnDev);
    RGN_CHECK_CALLBACK(pstCoverDev, pstOps, fpSetColor, return MHAL_FAILURE);
    MHAL_RGN_INFO("%px: [%2d]=%d, 0x%x", pCtx, pstColorCfg->u32WinId, pstColorCfg->eColorMode, pstColorCfg->u32Color);
    return pstCoverDev->pstOps->fpSetColor(pstCoverDev, pstColorCfg);
}

MS_S32 MHAL_RGN_CoverSetBlock(const void *pCtx, const MHAL_RGN_CoverBlockConfig_t *pstBlockCfg)
{
    MHAL_RGN_CoverDev_t *pstCoverDev = CAM_OS_CONTAINER_OF(pCtx, MHAL_RGN_CoverDev_t, stRgnDev);
    RGN_CHECK_CALLBACK(pstCoverDev, pstOps, fpSetBlock, return MHAL_FAILURE);
    MHAL_RGN_INFO("%px: [%2d]=%d", pCtx, pstBlockCfg->u32WinId, pstBlockCfg->eBlockSize);
    return pstCoverDev->pstOps->fpSetBlock(pstCoverDev, pstBlockCfg);
}

MS_S32 MHAL_RGN_CoverSetMap(const void *pCtx, const MHAL_RGN_CoverMapConfig_t *pstMapCfg)
{
    MHAL_RGN_CoverDev_t *pstCoverDev = CAM_OS_CONTAINER_OF(pCtx, MHAL_RGN_CoverDev_t, stRgnDev);
    RGN_CHECK_CALLBACK(pstCoverDev, pstOps, fpSetMap, return MHAL_FAILURE);
    MHAL_RGN_INFO("%px: %d, %lx, %d", pCtx, pstMapCfg->bEn, pstMapCfg->phyAddr, pstMapCfg->eBlockSize);
    return pstCoverDev->pstOps->fpSetMap(pstCoverDev, pstMapCfg);
}

MS_S32 MHAL_RGN_CoverSetWindow(const void *pCtx, const MHAL_RGN_CoverWindowConfig_t *pstWinCfg)
{
    MHAL_RGN_CoverDev_t *pstCoverDev = CAM_OS_CONTAINER_OF(pCtx, MHAL_RGN_CoverDev_t, stRgnDev);
    RGN_CHECK_CALLBACK(pstCoverDev, pstOps, fpSetWindow, return MHAL_FAILURE);
    MHAL_RGN_INFO("%px: [%d]=%d, %d, %dx%d", pCtx, pstWinCfg->u32WinId, pstWinCfg->stWinCfg.u32X,
                  pstWinCfg->stWinCfg.u32Y, pstWinCfg->stWinCfg.u32Width, pstWinCfg->stWinCfg.u32Height);
    return pstCoverDev->pstOps->fpSetWindow(pstCoverDev, pstWinCfg);
}

MS_S32 MHAL_RGN_CoverSetOnOff(const void *pCtx, const MHAL_RGN_CoverOnOffConfig_t *pstOnOffCfg)
{
    MHAL_RGN_CoverDev_t *pstCoverDev = CAM_OS_CONTAINER_OF(pCtx, MHAL_RGN_CoverDev_t, stRgnDev);
    RGN_CHECK_CALLBACK(pstCoverDev, pstOps, fpSetOnOff, return MHAL_FAILURE);
    MHAL_RGN_INFO("%px: [%d]-%d", pCtx, pstOnOffCfg->u32WinId, pstOnOffCfg->bEn);
    return pstCoverDev->pstOps->fpSetOnOff(pstCoverDev, pstOnOffCfg);
}

MS_S32 MHAL_RGN_FrameSetPalette(const void *pCtx, const MHAL_RGN_FramePaletteConfig_t *pstPaletteCfg)
{
    MHAL_RGN_FrameDev_t *pstFrameDev = CAM_OS_CONTAINER_OF(pCtx, MHAL_RGN_FrameDev_t, stRgnDev);
    RGN_CHECK_CALLBACK(pstFrameDev, pstOps, fpSetPalette, return MHAL_FAILURE);
    MHAL_RGN_INFO("%px: [%d]-(%02x, %02x, %02x)", pCtx, pstPaletteCfg->u8Idx, pstPaletteCfg->stEntry.u8Y,
                  pstPaletteCfg->stEntry.u8U, pstPaletteCfg->stEntry.u8V);
    return pstFrameDev->pstOps->fpSetPalette(pstFrameDev, pstPaletteCfg);
}

MS_S32 MHAL_RGN_FrameSetWindowBuffer(const void *pCtx, const MHAL_RGN_BufferConfig_t *pstBufferCfg)
{
    MHAL_RGN_FrameDev_t *pstFrameDev = CAM_OS_CONTAINER_OF(pCtx, MHAL_RGN_FrameDev_t, stRgnDev);
    RGN_CHECK_CALLBACK(pstFrameDev, pstOps, fpSetWinBuffer, return MHAL_FAILURE);
    MHAL_RGN_INFO("%px: 0x%lx, 0x%lx", pCtx, pstBufferCfg->phyAddr, pstBufferCfg->pAddr);
    return pstFrameDev->pstOps->fpSetWinBuffer(pstFrameDev, pstBufferCfg);
}

MS_S32 MHAL_RGN_FrameSetBorder(const void *pCtx, const MHAL_RGN_FrameBorderConfig_t *pstBorderCfg)
{
    MHAL_RGN_FrameDev_t *pstFrameDev = CAM_OS_CONTAINER_OF(pCtx, MHAL_RGN_FrameDev_t, stRgnDev);
    RGN_CHECK_CALLBACK(pstFrameDev, pstOps, fpSetBorder, return MHAL_FAILURE);
    MHAL_RGN_INFO("%px: [%2d]=%d", pCtx, pstBorderCfg->u32WinId, pstBorderCfg->u32Size);
    return pstFrameDev->pstOps->fpSetBorder(pstFrameDev, pstBorderCfg);
}

MS_S32 MHAL_RGN_FrameSetColor(const void *pCtx, const MHAL_RGN_FrameColorConfig_t *pstColorCfg)
{
    MHAL_RGN_FrameDev_t *pstFrameDev = CAM_OS_CONTAINER_OF(pCtx, MHAL_RGN_FrameDev_t, stRgnDev);
    RGN_CHECK_CALLBACK(pstFrameDev, pstOps, fpSetColor, return MHAL_FAILURE);
    MHAL_RGN_INFO("%px: [%2d]=%d", pCtx, pstColorCfg->u32WinId, pstColorCfg->u8Idx);
    return pstFrameDev->pstOps->fpSetColor(pstFrameDev, pstColorCfg);
}

MS_S32 MHAL_RGN_FrameSetWindow(const void *pCtx, const MHAL_RGN_FrameWindowConfig_t *pstWinCfg)
{
    MHAL_RGN_FrameDev_t *pstFrameDev = CAM_OS_CONTAINER_OF(pCtx, MHAL_RGN_FrameDev_t, stRgnDev);
    RGN_CHECK_CALLBACK(pstFrameDev, pstOps, fpSetWindow, return MHAL_FAILURE);
    MHAL_RGN_INFO("%px: [%d]=%d, %d, %dx%d", pCtx, pstWinCfg->u32WinId, pstWinCfg->stWinCfg.u32X,
                  pstWinCfg->stWinCfg.u32Y, pstWinCfg->stWinCfg.u32Width, pstWinCfg->stWinCfg.u32Height);
    return pstFrameDev->pstOps->fpSetWindow(pstFrameDev, pstWinCfg);
}

MS_S32 MHAL_RGN_FrameSetOnOff(const void *pCtx, const MHAL_RGN_FrameOnOffConfig_t *pstOnOffCfg)
{
    MHAL_RGN_FrameDev_t *pstFrameDev = CAM_OS_CONTAINER_OF(pCtx, MHAL_RGN_FrameDev_t, stRgnDev);
    RGN_CHECK_CALLBACK(pstFrameDev, pstOps, fpSetOnOff, return MHAL_FAILURE);
    MHAL_RGN_INFO("%px: [%d]-%d", pCtx, pstOnOffCfg->u32WinId, pstOnOffCfg->bEn);
    return pstFrameDev->pstOps->fpSetOnOff(pstFrameDev, pstOnOffCfg);
}

MS_S32 MHAL_RGN_ColorInvertSetCalMode(const void *pCtx, const MHAL_RGN_ColorInvertCalModeConfig_t *pstCalModeCfg)
{
    MHAL_RGN_ColorInvertDev_t *pstColorInvertDev = CAM_OS_CONTAINER_OF(pCtx, MHAL_RGN_ColorInvertDev_t, stRgnDev);
    RGN_CHECK_CALLBACK(pstColorInvertDev, pstOps, fpSetCalMode, return MHAL_FAILURE);
    MHAL_RGN_INFO("%px: %d", pCtx, pstCalModeCfg->eCalMode);
    return pstColorInvertDev->pstOps->fpSetCalMode(pCtx, pstCalModeCfg);
}

MS_S32 MHAL_RGN_ColorInvertSetStroageMode(const void *                                   pCtx,
                                          const MHAL_RGN_ColorInvertStorageModeConfig_t *pstStorageModeCfg)
{
    MHAL_RGN_ColorInvertDev_t *pstColorInvertDev = CAM_OS_CONTAINER_OF(pCtx, MHAL_RGN_ColorInvertDev_t, stRgnDev);
    RGN_CHECK_CALLBACK(pstColorInvertDev, pstOps, fpSetStroageMode, return MHAL_FAILURE);
    MHAL_RGN_INFO("%px: %d, %d", pCtx, pstStorageModeCfg->eStorageMode, pstStorageModeCfg->bEnPreRead);
    return pstColorInvertDev->pstOps->fpSetStroageMode(pCtx, pstStorageModeCfg);
}

MS_S32 MHAL_RGN_ColorInvertSetBlock(const void *pCtx, const MHAL_RGN_ColorInvertBlockSizeConfig_t *pstBlockCfg)
{
    MHAL_RGN_ColorInvertDev_t *pstColorInvertDev = CAM_OS_CONTAINER_OF(pCtx, MHAL_RGN_ColorInvertDev_t, stRgnDev);
    RGN_CHECK_CALLBACK(pstColorInvertDev, pstOps, fpSetBlock, return MHAL_FAILURE);
    MHAL_RGN_INFO("%px: %dx%d", pCtx, pstBlockCfg->eBlockSizeH, pstBlockCfg->eBlockSizeV);
    return pstColorInvertDev->pstOps->fpSetBlock(pCtx, pstBlockCfg);
}

MS_S32 MHAL_RGN_ColorInvertSetThreshold(const void *pCtx, const MHAL_RGN_ColorInvertThresholdConfig_t *pstThresholdCfg)
{
    MHAL_RGN_ColorInvertDev_t *pstColorInvertDev = CAM_OS_CONTAINER_OF(pCtx, MHAL_RGN_ColorInvertDev_t, stRgnDev);
    RGN_CHECK_CALLBACK(pstColorInvertDev, pstOps, fpSetThreshold, return MHAL_FAILURE);
    MHAL_RGN_INFO("%px: %d ~ %d", pCtx, pstThresholdCfg->u8ThresholdLow, pstThresholdCfg->u8ThresholdHigh);
    return pstColorInvertDev->pstOps->fpSetThreshold(pCtx, pstThresholdCfg);
}

MS_S32 MHAL_RGN_ColorInvertSetBuffer(const void *pCtx, const MHAL_RGN_ColorInvertBufferConfig_t *pstBufferCfg)
{
    MHAL_RGN_ColorInvertDev_t *pstColorInvertDev = CAM_OS_CONTAINER_OF(pCtx, MHAL_RGN_ColorInvertDev_t, stRgnDev);
    RGN_CHECK_CALLBACK(pstColorInvertDev, pstOps, fpSetBuffer, return MHAL_FAILURE);
    MHAL_RGN_INFO("%px: r(%lx), w(%lx)", pCtx, pstBufferCfg->phyRAddr, pstBufferCfg->phyWAddr);
    return pstColorInvertDev->pstOps->fpSetBuffer(pCtx, pstBufferCfg);
}

MS_S32 MHAL_RGN_ColorInvertSetOnOff(const void *pCtx, const MHAL_RGN_ColorInvertOnOffConfig_t *pstOnOffCfg)
{
    MHAL_RGN_ColorInvertDev_t *pstColorInvertDev = CAM_OS_CONTAINER_OF(pCtx, MHAL_RGN_ColorInvertDev_t, stRgnDev);
    RGN_CHECK_CALLBACK(pstColorInvertDev, pstOps, fpSetOnOff, return MHAL_FAILURE);
    MHAL_RGN_INFO("%px: %d", pCtx, pstOnOffCfg->bEn);
    return pstColorInvertDev->pstOps->fpSetOnOff(pCtx, pstOnOffCfg);
}

MS_S32 MHAL_RGN_IfcdSetAttr(const void *pCtx, const MHAL_RGN_IfcdAttrConfig_t *pstAttrCfg)
{
    MHAL_RGN_IfcdDev_t *pstIfcdDev = CAM_OS_CONTAINER_OF(pCtx, MHAL_RGN_IfcdDev_t, stRgnDev);
    RGN_CHECK_CALLBACK(pstIfcdDev, pstOps, fpSetAttr, return MHAL_FAILURE);
    MHAL_RGN_INFO("%px: %d, %d, %d, %d, %dx%d", pCtx, pstAttrCfg->bHalfBlock, pstAttrCfg->bBlockSplit,
                  pstAttrCfg->bColorTransform, pstAttrCfg->ePixelFmt, pstAttrCfg->u16Width, pstAttrCfg->u16Height);
    return pstIfcdDev->pstOps->fpSetAttr(pCtx, pstAttrCfg);
}

MS_S32 MHAL_RGN_IfcdSetBuffer(const void *pCtx, const MHAL_RGN_IfcdBufferConfig_t *pstBufferCfg)
{
    MHAL_RGN_IfcdDev_t *pstIfcdDev = CAM_OS_CONTAINER_OF(pCtx, MHAL_RGN_IfcdDev_t, stRgnDev);
    RGN_CHECK_CALLBACK(pstIfcdDev, pstOps, fpSetBuffer, return MHAL_FAILURE);
    MHAL_RGN_INFO("%px: %lx, %lx", pCtx, pstBufferCfg->phyHeaderAddr, pstBufferCfg->phyImiAddr);
    return pstIfcdDev->pstOps->fpSetBuffer(pCtx, pstBufferCfg);
}

MS_S32 MHAL_RGN_IfcdSetOnOff(const void *pCtx, const MHAL_RGN_IfcdOnOffConfig_t *pstOnOffCfg)
{
    MHAL_RGN_IfcdDev_t *pstIfcdDev = CAM_OS_CONTAINER_OF(pCtx, MHAL_RGN_IfcdDev_t, stRgnDev);
    RGN_CHECK_CALLBACK(pstIfcdDev, pstOps, fpSetOnOff, return MHAL_FAILURE);
    MHAL_RGN_INFO("%px: %d", pCtx, pstOnOffCfg->bEn);
    return pstIfcdDev->pstOps->fpSetOnOff(pCtx, pstOnOffCfg);
}
