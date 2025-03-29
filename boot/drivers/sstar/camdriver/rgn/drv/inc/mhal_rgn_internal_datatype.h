/*
 * mhal_rgn_internal_datatype.h - Sigmastar
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

#ifndef __MHAL_RGN_INTERNAL_DATATYPE_H__
#define __MHAL_RGN_INTERNAL_DATATYPE_H__

#include "mhal_rgn_datatype.h"

struct MHAL_RGN_Dev_s;
struct MHAL_RGN_Cmdq_s;
struct MHAL_RGN_GopDev_s;
struct MHAL_RGN_CoverDev_s;
struct MHAL_RGN_FrameDev_s;
struct MHAL_RGN_ColorInvertDev_s;
struct MHAL_RGN_IfcdDev_s;

typedef enum
{
    E_MHAL_RESET_MD_HW_INIT   = 0x1,
    E_MHAL_RESET_MD_SW_INIT   = 0x2,
    E_MHAL_RESET_MD_REG_SYNC  = 0x4,
    E_MHAL_RESET_MD_SRAM_SYNC = 0x8,
    E_MHAL_RESET_MD_MAX       = 0xf,
} MHAL_RGN_ResetMode_e;

typedef enum
{
    E_HAL_RGN_CMDQ_MD_NORMAL,
    E_HAL_RGN_CMDQ_MD_MEM,
    E_HAL_RGN_CMDQ_MD_MAX,
} MHAL_RGN_CmdqModeType_e;

typedef struct MHAL_RGN_CmdqOps_s
{
    void (*fpWrite2Byte)(const struct MHAL_RGN_Cmdq_s *pstRgnCmdq, MS_U32 u32Reg, MS_U16 u16Val);
    void (*fpWrite2ByteMsk)(const struct MHAL_RGN_Cmdq_s *pstRgnCmdq, MS_U32 u32Reg, MS_U16 u16Val, MS_U16 u16Mask);
    void (*fpRegUpdate)(const struct MHAL_RGN_Cmdq_s *pstRgnCmdq);
} MHAL_RGN_CmdqOps_t;

typedef struct MHAL_RGN_Cmdq_s
{
    MHAL_RGN_CmdqModeType_e    eCmdqType;
    MHAL_CMDQ_CmdqInterface_t *pstCmdqInf;
    const MHAL_RGN_CmdqOps_t * pstOps;
} MHAL_RGN_Cmdq_t;

struct MHAL_RGN_Mux_s;

typedef struct MHAL_RGN_MuxOps_s
{
    MS_S32 (*fpActivate)(const struct MHAL_RGN_Mux_s *pstRgnMux, const struct MHAL_RGN_Dev_s *pstRgnDev);
    MS_S32 (*fpDeactivate)(const struct MHAL_RGN_Mux_s *pstRgnMux, const struct MHAL_RGN_Dev_s *pstRgnDev);
} MHAL_RGN_MuxOps_t;

typedef struct MHAL_RGN_Mux_s
{
    MHAL_RGN_TargetType_e eTargetType;
    MS_U32                u32Id;
    MS_U32                u32RefCnt;
} MHAL_RGN_Mux_t;

typedef struct MHAL_RGN_DevOps_s
{
    MS_S32 (*fpReset)(const struct MHAL_RGN_Dev_s *pstRgnDev, MHAL_RGN_ResetMode_e eRstMode);
    MS_S32 (*fpProcess)(const struct MHAL_RGN_Dev_s *pstRgnDev);
    MS_S32 (*fpActivate)(const struct MHAL_RGN_Dev_s *pstRgnDev);
    MS_S32 (*fpDeactivate)(const struct MHAL_RGN_Dev_s *pstRgnDev);
    MS_S32 (*fpSetResolution)(const struct MHAL_RGN_Dev_s *pstRgnDev, const MHAL_RGN_ResolutionConfig_t *);
} MHAL_RGN_DevOps_t;

typedef struct MHAL_RGN_Dev_s
{
    MHAL_RGN_DevAttr_t       stRgnDevAttr;
    const MHAL_RGN_DevOps_t *pstOps;
    MHAL_RGN_Cmdq_t *        pstRgnCmdq;
    MHAL_RGN_Mux_t *         pstRgnMux;
} MHAL_RGN_Dev_t;

typedef struct MHAL_RGN_GopOps_s
{
    MS_S32 (*fpSetPalette)(const struct MHAL_RGN_GopDev_s *, const MHAL_RGN_GopPaletteConfig_t *);
    MS_S32 (*fpSetPaletteTbl)(const struct MHAL_RGN_GopDev_s *, const MHAL_RGN_GopPaletteTblConfig_t *);
    MS_S32 (*fpSetBaseWindow)(const struct MHAL_RGN_GopDev_s *, const MHAL_RGN_GopStretchWinConfig_t *);
    MS_S32 (*fpSetColorKey)(const struct MHAL_RGN_GopDev_s *, const MHAL_RGN_GopColorKeyConfig_t *);
    MS_S32 (*fpSetAlphaZeroOpaque)(const struct MHAL_RGN_GopDev_s *, const MHAL_RGN_GopAlphaZeroOpaqueConfig_t *);
    MS_S32 (*fpSetOutputFormat)(const struct MHAL_RGN_GopDev_s *, const MHAL_RGN_GopOutputFmtConfig_t *);
    MS_S32 (*fpSetContrast)(const struct MHAL_RGN_GopDev_s *, const MHAL_RGN_GopContrastConfig_t *);
    MS_S32 (*fpSetBrightness)(const struct MHAL_RGN_GopDev_s *, const MHAL_RGN_GopBrightnessConfig_t *);
    MS_S32 (*fpSetEncMode)(const struct MHAL_RGN_GopDev_s *, const MHAL_RGN_GopEncFormatConfig_t *);
    MS_S32 (*fpSetDmaThreahold)(const struct MHAL_RGN_GopDev_s *, const MHAL_RGN_GopDmaThreaholdConfig_t *);
    MS_S32 (*fpGwinSetPixelFormat)(const struct MHAL_RGN_GopDev_s *, const MHAL_RGN_GopGwinPixelFmtConfig_t *);
    MS_S32 (*fpGwinSetPaletteIndex)(const struct MHAL_RGN_GopDev_s *, const MHAL_RGN_GopGwinPaletteIdxConfig_t *);
    MS_S32 (*fpGwinSetWindow)(const struct MHAL_RGN_GopDev_s *, const MHAL_RGN_GopGwinWinConfig_t *);
    MS_S32 (*fpGwinSetBuffer)(const struct MHAL_RGN_GopDev_s *, const MHAL_RGN_GopGwinBufferConfig_t *);
    MS_S32 (*fpGwinSetOnOff)(const struct MHAL_RGN_GopDev_s *, const MHAL_RGN_GopGwinOnOffConfig_t *);
    MS_S32 (*fpGwinSetAlphaType)(const struct MHAL_RGN_GopDev_s *, const MHAL_RGN_GopGwinAlphaConfig_t *);
    MS_S32 (*fpSetArgb1555AlphaVal)(const struct MHAL_RGN_GopDev_s *, const MHAL_RGN_GopGwinArgb1555AlphaConfig_t *);
    MS_S32 (*fpSetBlend)(const struct MHAL_RGN_GopDev_s *, const MHAL_RGN_GopBlendConfig_t *);
} MHAL_RGN_GopOps_t;

typedef struct MHAL_RGN_GopDev_s
{
    MHAL_RGN_Dev_t           stRgnDev;
    const MHAL_RGN_GopOps_t *pstOps;
} MHAL_RGN_GopDev_t;

typedef struct MHAL_RGN_CoverOps_s
{
    MS_S32 (*fpSetPalette)(const struct MHAL_RGN_CoverDev_s *, const MHAL_RGN_CoverPaletteConfig_t *);
    MS_S32 (*fpSetWinBuffer)(const struct MHAL_RGN_CoverDev_s *, const MHAL_RGN_BufferConfig_t *);
    MS_S32 (*fpSetColor)(const struct MHAL_RGN_CoverDev_s *, const MHAL_RGN_CoverColorConfig_t *);
    MS_S32 (*fpSetBlock)(const struct MHAL_RGN_CoverDev_s *, const MHAL_RGN_CoverBlockConfig_t *);
    MS_S32 (*fpSetMap)(const struct MHAL_RGN_CoverDev_s *, const MHAL_RGN_CoverMapConfig_t *);
    MS_S32 (*fpSetWindow)(const struct MHAL_RGN_CoverDev_s *, const MHAL_RGN_CoverWindowConfig_t *);
    MS_S32 (*fpSetOnOff)(const struct MHAL_RGN_CoverDev_s *, const MHAL_RGN_CoverOnOffConfig_t *);
} MHAL_RGN_CoverOps_t;

typedef struct MHAL_RGN_CoverDev_s
{
    MHAL_RGN_Dev_t             stRgnDev;
    const MHAL_RGN_CoverOps_t *pstOps;
} MHAL_RGN_CoverDev_t;

typedef struct MHAL_RGN_FrameOps_s
{
    MS_S32 (*fpSetPalette)(const struct MHAL_RGN_FrameDev_s *, const MHAL_RGN_FramePaletteConfig_t *);
    MS_S32 (*fpSetWinBuffer)(const struct MHAL_RGN_FrameDev_s *, const MHAL_RGN_BufferConfig_t *);
    MS_S32 (*fpSetBorder)(const struct MHAL_RGN_FrameDev_s *, const MHAL_RGN_FrameBorderConfig_t *);
    MS_S32 (*fpSetColor)(const struct MHAL_RGN_FrameDev_s *, const MHAL_RGN_FrameColorConfig_t *);
    MS_S32 (*fpSetWindow)(const struct MHAL_RGN_FrameDev_s *, const MHAL_RGN_FrameWindowConfig_t *);
    MS_S32 (*fpSetOnOff)(const struct MHAL_RGN_FrameDev_s *, const MHAL_RGN_FrameOnOffConfig_t *);
} MHAL_RGN_FrameOps_t;

typedef struct MHAL_RGN_FrameDev_s
{
    MHAL_RGN_Dev_t             stRgnDev;
    const MHAL_RGN_FrameOps_t *pstOps;
} MHAL_RGN_FrameDev_t;

typedef struct MHAL_RGN_ColorInvertOps_s
{
    MS_S32 (*fpSetCalMode)(const struct MHAL_RGN_ColorInvertDev_s *, const MHAL_RGN_ColorInvertCalModeConfig_t *);
    MS_S32(*fpSetStroageMode)
    (const struct MHAL_RGN_ColorInvertDev_s *, const MHAL_RGN_ColorInvertStorageModeConfig_t *);
    MS_S32 (*fpSetBlock)(const struct MHAL_RGN_ColorInvertDev_s *, const MHAL_RGN_ColorInvertBlockSizeConfig_t *);
    MS_S32 (*fpSetThreshold)(const struct MHAL_RGN_ColorInvertDev_s *, const MHAL_RGN_ColorInvertThresholdConfig_t *);
    MS_S32 (*fpSetBuffer)(const struct MHAL_RGN_ColorInvertDev_s *, const MHAL_RGN_ColorInvertBufferConfig_t *);
    MS_S32 (*fpSetOnOff)(const struct MHAL_RGN_ColorInvertDev_s *, const MHAL_RGN_ColorInvertOnOffConfig_t *);
} MHAL_RGN_ColorInvertOps_t;

typedef struct MHAL_RGN_ColorInvertDev_s
{
    MHAL_RGN_Dev_t                   stRgnDev;
    const MHAL_RGN_ColorInvertOps_t *pstOps;
} MHAL_RGN_ColorInvertDev_t;

typedef struct MHAL_RGN_IfcdOps_s
{
    MS_S32 (*fpSetAttr)(const struct MHAL_RGN_IfcdDev_s *, const MHAL_RGN_IfcdAttrConfig_t *);
    MS_S32 (*fpSetBuffer)(const struct MHAL_RGN_IfcdDev_s *, const MHAL_RGN_IfcdBufferConfig_t *);
    MS_S32 (*fpSetOnOff)(const struct MHAL_RGN_IfcdDev_s *, const MHAL_RGN_IfcdOnOffConfig_t *);
} MHAL_RGN_IfcdOps_t;

typedef struct MHAL_RGN_IfcdDev_s
{
    MHAL_RGN_Dev_t            stRgnDev;
    const MHAL_RGN_IfcdOps_t *pstOps;
} MHAL_RGN_IfcdDev_t;

#endif /* ifndef __MHAL_RGN_INTERNAL_DATATYPE_H__ */
