/*
 * mhal_rgn_datatype.h - Sigmastar
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

#ifndef _MHAL_RGN_DATATYPE_H_
#define _MHAL_RGN_DATATYPE_H_

//#include "cam_os_wrapper.h"

//======================================================================================================================
// macro
//======================================================================================================================
#define MHAL_RGN_DBG_LV_MHAL      (0x0001)
#define MHAL_RGN_DBG_LV_HAL_IF    (0x0002)
#define MHAL_RGN_DBG_LV_HAL_GOP   (0x0004)
#define MHAL_RGN_DBG_LV_HAL_COVER (0x0008)
#define MHAL_RGN_DBG_LV_HAL_FRAME (0x0010)
#define MHAL_RGN_DBG_LV_HAL_CI    (0x0020)
#define MHAL_RGN_DBG_LV_HAL_IFCD  (0x0040)
#define MHAL_RGN_DBG_LV_REG       (0x0080)

//======================================================================================================================
// enum
//======================================================================================================================

/// @brief RGN Dev type
typedef enum
{
    E_MHAL_RGN_DEV_TYPE_GOP,   ///> Dev type for GOP
    E_MHAL_RGN_DEV_TYPE_COVER, ///> Dev type for COVER
    E_MHAL_RGN_DEV_TYPE_FRAME, ///> Dev type for FRAME
    E_MHAL_RGN_DEV_TYPE_CI,    ///> Dev type for Color Invert
    E_MHAL_RGN_DEV_TYPE_IFCD,  ///> Dev type for IFCD
    E_MHAL_RGN_DEV_TYPE_MAX,   ///> MAX number of Dev type
} MHAL_RGN_DevType_e;

/// @brief RGN TARGET type
typedef enum
{
    E_MHAL_RGN_TARGET_TYPE_SCL,        ///< TARGET type for SCL
    E_MHAL_RGN_TARGET_TYPE_SCL_RDMA,   ///< TARGET type for SCL RDMA
    E_MHAL_RGN_TARGET_TYPE_SCL_ISP,    ///< TARGET type for SCL ISP Path
    E_MHAL_RGN_TARGET_TYPE_DISP_UI,    ///< TARGET type for DISP UI
    E_MHAL_RGN_TARGET_TYPE_DISP_CUR_0, ///< TARGET type for DISP CURSOR0
    E_MHAL_RGN_TARGET_TYPE_DISP_CUR_1, ///< TARGET type for DISP CURSOR1
    E_MHAL_RGN_TARGET_TYPE_LDC,        ///< TARGET type for LDC
    E_MHAL_RGN_TARGET_TYPE_VENC,       ///< TARGET type for VENC
    E_MHAL_RGN_TARGET_TYPE_JPE,        ///< TARGET type for JPE
    E_MHAL_RGN_TARGET_TYPE_MAX,        ///< Max number of TARGET TYPE
} MHAL_RGN_TargetType_e;

/// @brief RGN GOP alpha bleding type
typedef enum
{
    E_MHAL_GOP_GWIN_ALPHA_CONSTANT, ///< Contstant alpha blending
    E_MHAL_GOP_GWIN_ALPHA_PIXEL,    ///< Pixel alpha blending
    E_MHAL_GOP_GWIN_ALPHA_NUM,      ///< Number of gop alpha type
} MHAL_RGN_GopGwinAlphaType_e;

/// @brief RGN GOP ARGB1555 alpha bleding type
typedef enum
{
    E_MHAL_GOP_GWIN_ARGB1555_ALPHA0,   ///< ARGB1555 pixel alpha = 0
    E_MHAL_GOP_GWIN_ARGB1555_ALPHA1,   ///< ARGB1555 pixel alpha = 1
    E_MHAL_GOP_GWIN_ARGB1555_ALPHA_MAX ///< Max number of ARGB1555 pixel alpha type
} MHAL_RGN_GopGwinArgb1555AlpahType_e;

/// @brief RGN GOP Blend Type
typedef enum
{
    E_MHAL_GOP_BLEND_SRC_ALPHA_DEST_ALPHA,       ///< colorOut = colorSrc*alphaSrc + colorDst * ( 1 - alphaSrc )
    E_MHAL_GOP_BLEND_SRC_ALPHA_PALNE_DEST_PLANE, ///< colorOut = colorSrc*alphaSrc*planeAlphaSrc + colorDst * ( 1 -
                                                 ///< (alphaSrc * planeAlphaSrc) )
    E_MHAL_GOP_BLEND_SRC_DEST_ALPHA,             ///< colorOut = colorSrc + colorDst * ( 1 - alphaSrc)
    E_MHAL_GOP_BLEND_SRC_PLANE_DEST_ALPHA_PLANE, ///< colorOut = colorSrc*planeAlphaSrc + colorDst * ( 1 - (alphaSrc *
                                                 ///< planeAlphaSrc) )
    E_MHAL_GOP_BLEND_NUM,
} MHAL_RGN_GopBlendType_e;

/// @brief RGN GOP output color format
typedef enum
{
    E_MHAL_GOP_OUT_FMT_RGB, ///< RGB output color
    E_MHAL_GOP_OUT_FMT_YUV, ///< YUV output color
    E_MHAL_GOP_OUT_FMT_NUM, ///< Number of output color format
} MHAL_RGN_GopOutFmtType_e;

/// @brief RGN GOP pixel format of source image
typedef enum
{
    E_MHAL_GOP_PIXEL_FORMAT_ARGB1555 = 0, ///< ARGB1555
    E_MHAL_GOP_PIXEL_FORMAT_ARGB4444,     ///< ARGB4444
    E_MHAL_GOP_PIXEL_FORMAT_I2,           ///< I2, palette format
    E_MHAL_GOP_PIXEL_FORMAT_I4,           ///< I4, palette format
    E_MHAL_GOP_PIXEL_FORMAT_I8,           ///< I8, palette foramt
    E_MHAL_GOP_PIXEL_FORMAT_RGB565,       ///< RGB565
    E_MHAL_GOP_PIXEL_FORMAT_ARGB8888,     ///< ARGB8888
    E_MHAL_GOP_PIXEL_FORMAT_UV8Y8,        ///< YUV422
    E_MHAL_GOP_PIXEL_FORMAT_ABGR8888,     ///< ABGR8888
    E_MHAL_GOP_PIXEL_FORMAT_ABGR1555,     ///< ABGR1555
    E_MHAL_GOP_PIXEL_FORMAT_ABGR4444,     ///< ABGR4444
    E_MHAL_GOP_PIXEL_FORMAT_BGR565,       ///< BGR565
    E_MHAL_GOP_PIXEL_FORMAT_MAX           ///< Max number of gop pixel format
} MHAL_RGN_GopPixelFormat_e;

/// @brief RGN GOP trigger mode for VEN and JPE
typedef enum
{
    E_MHAL_GOP_ENC_MD_VENC_H264, ///< Trigger mode for VENC_H264
    E_MHAL_GOP_ENC_MD_VENC_H265, ///< Trigger mode for VENC_H265
    E_MHAL_GOP_ENC_MD_VENC_AV1,  ///< Trigger mode for AV1
    E_MHAL_GOP_ENC_MD_JPE_420,   ///< Trigger mode for JPE_420
    E_MHAL_GOP_ENC_MD_JPE_422,   ///< Trigger mode for JPE_422
    E_MHAL_GOP_ENC_MD_MAX,       ///< Max number for trigger mode
} MHAL_RGN_GopEncMode_e;

/// @brief RGN COVER block size
typedef enum
{
    E_MHAL_RGN_COVER_BLOCK_SIZE_4,   ///< COVER block size 4x4
    E_MHAL_RGN_COVER_BLOCK_SIZE_8,   ///< COVER block size 8x8
    E_MHAL_RGN_COVER_BLOCK_SIZE_16,  ///< COVER block size 16x16
    E_MHAL_RGN_COVER_BLOCK_SIZE_32,  ///< COVER block size 32x32
    E_MHAL_RGN_COVER_BLOCK_SIZE_64,  ///< COVER block size 64x64
    E_MHAL_RGN_COVER_BLOCK_SIZE_128, ///< COVER block size 128x128
    E_MHAL_RGN_COVER_BLOCK_SIZE_256, ///< COVER block size 256x256
    E_MHAL_RGN_COVER_BLOCK_SIZE_MAX, ///< Max number COVER block size
} MHAL_RGN_CoverBlockSize_e;

/// @brief RGN COVER color mode
typedef enum
{
    E_MHAL_RGN_COVER_COLOR_MODE_MOSAIC,  ///< Cover color use data from video.
    E_MHAL_RGN_COVER_COLOR_MODE_PALETTE, ///< Cover color use palettee
    E_MHAL_RGN_COVER_COLOR_MODE_YUV,     ///< Cover color use yuv
    E_MHAL_RGN_COVER_COLOR_MODE_MAX,     ///< Max number COVER color mode
} MHAL_RGN_CoverColorMode_e;

/// @brief RGN COVER map block size
typedef enum
{
    E_MHAL_RGN_COVER_MAP_BLOCK_SIZE_1,   ///< Map block size 1x1 (pixel)
    E_MHAL_RGN_COVER_MAP_BLOCK_SIZE_2,   ///< Map block size 2x2 (pixel)
    E_MHAL_RGN_COVER_MAP_BLOCK_SIZE_4,   ///< Map block size 4x4 (pixel)
    E_MHAL_RGN_COVER_MAP_BLOCK_SIZE_8,   ///< Map block size 8x8 (pixel)
    E_MHAL_RGN_COVER_MAP_BLOCK_SIZE_MAX, ///< Max number map Block size
} MHAL_RGN_CoverMapBlockSize_e;

/// @brief RGN COLOR INVERT cal mode
typedef enum
{
    E_MHAL_RGN_COLOR_INVERT_CAL_MODE_Y_ONLY, ///< Color invert cal y only
    E_MHAL_RGN_COLOR_INVERT_CAL_MODE_YUYV,   ///< Color invert cal yuyv
    E_MHAL_RGN_COLOR_INVERT_CAL_MODE_MAX,    ///< Max number for COLOR INVERT Cal mode
} MHAL_RGN_ColorInvertCalMode_e;

/// @brief RGN COVER INVERT storage mode
typedef enum
{
    E_MHAL_RGN_COLOR_INVERT_STORAGE_MODE_CURR_ONLY,    ///< val = curr
    E_MHAL_RGN_COLOR_INVERT_STORAGE_MODE_CURR_1_PRE_1, ///< val = curr * (1/2) + pre * (1/2)
    E_MHAL_RGN_COLOR_INVERT_STORAGE_MODE_CURR_1_PRE_3, ///< val = curr * (1/4) + pre * (3/4)
    E_MHAL_RGN_COLOR_INVERT_STORAGE_MODE_CURR_1_PRE_7, ///< val = curr * (1/8) + pre * (7/8)
    E_MHAL_RGN_COLOR_INVERT_STORAGE_MODE_MAX,          ///< Max number for Storage mode
} MHAL_RGN_ColorInvertStorageMode_e;

/// @brief RGN COVER INVERT block size
typedef enum
{
    E_MHAL_RGN_COLOR_INVERT_BLOCK_SIZE_32,  ///< CI block size 32
    E_MHAL_RGN_COLOR_INVERT_BLOCK_SIZE_64,  ///< CI block size 64
    E_MHAL_RGN_COLOR_INVERT_BLOCK_SIZE_128, ///< CI block size 128
    E_MHAL_RGN_COLOR_INVERT_BLOCK_SIZE_256, ///< CI block size 256
    E_MHAL_RGN_COLOR_INVERT_BLOCK_SIZE_MAX, ///< Max number of block size
} MHAL_RGN_ColorInvertBlockSize_e;

//======================================================================================================================
// struct
//======================================================================================================================

/// @brief RGN Chip capability ops
typedef struct MHAL_RGN_ChipCapOps_s
{
    MS_U32 (*fpGetGopGwinCnt)(MHAL_RGN_TargetType_e eTargetType);
    MS_U32 (*fpGetGopWidthAlign)(MHAL_RGN_GopPixelFormat_e ePixelFormat);
    MS_U32 (*fpGetGopXPosAlign)(MHAL_RGN_GopPixelFormat_e ePixelFormat);
    MS_U32 (*fpGetGopPitchAlign)(void);

    MS_BOOL (*fpIsSupportGopFmt)(MHAL_RGN_TargetType_e eTargetType, MHAL_RGN_GopPixelFormat_e ePixelFormat);
    MS_BOOL (*fpIsSupportGopColorkey)(MHAL_RGN_TargetType_e eTargetType);
    MS_BOOL (*fpIsSupportGopOverlap)(MHAL_RGN_TargetType_e eTargetType);
    MS_BOOL (*fpIsSupportGopXOverlap)(MHAL_RGN_TargetType_e eTargetType);

    MS_U32 (*fpGetDevGroupId)(MHAL_RGN_DevType_e eDevType, MHAL_RGN_TargetType_e eTargetType);

    MS_BOOL (*fpIsSupportCoverBlockSize)(MHAL_RGN_TargetType_e eTargetType, MHAL_RGN_CoverBlockSize_e eBlockSize);
    MS_BOOL (*fpIsSupportCoverDiffBlock)(MHAL_RGN_TargetType_e eTargetType);
    MS_BOOL (*fpIsSupportCoverColorMode)(MHAL_RGN_TargetType_e eTargetType, MHAL_RGN_CoverColorMode_e eColorMode);
    MS_BOOL (*fpIsSupportCoverMap)(MHAL_RGN_TargetType_e eTargetType);
    MS_BOOL(*fpIsSupportColorInvertBlockSize)
    (MHAL_RGN_TargetType_e eTargetType, MHAL_RGN_ColorInvertBlockSize_e eBlockSize);

    MS_U32 (*fpGetCoverHAlign)(MHAL_RGN_TargetType_e eTargetType, MHAL_RGN_CoverBlockSize_e eBlockSize);
    MS_U32 (*fpGetCoverVAlign)(MHAL_RGN_TargetType_e eTargetType, MHAL_RGN_CoverBlockSize_e eBlockSize);

    MS_U32 (*fpGetCoverWinCnt)(MHAL_RGN_TargetType_e eTargetType);
    MS_U32 (*fpGetFrameWinCnt)(MHAL_RGN_TargetType_e eTargetType);

    MS_U32 (*fpGetCoverWinBufferSize)(MHAL_RGN_TargetType_e eTargetType);
    MS_U32 (*fpGetFrameWinBufferSize)(MHAL_RGN_TargetType_e eTargetType);

    MS_U32 (*fpGetFrameMaxBorderSize)(void);
    MS_U32 (*fpGetFrameHAlign)(MHAL_RGN_TargetType_e eTargetType);
    MS_U32 (*fpGetFrameVAlign)(MHAL_RGN_TargetType_e eTargetType);

    MS_U32 (*fpGetColorInvertPitchAlign)(void);
    MS_U32 (*fpGetCoverMapPitchAlign)(void);

    MS_U32 (*fpGetCoverPaletteNum)(void);
    MS_U32 (*fpGetFramePaletteNum)(void);

    MS_BOOL (*fpIsSupportIfcd)(MHAL_RGN_TargetType_e eTargetType);

    MS_U32 (*fpGetMaxCmdqNum)(MHAL_RGN_TargetType_e eTargetType);
} MHAL_RGN_ChipCapOps_t;

/// @brief RGN Dev attr
typedef struct
{
    MHAL_RGN_DevType_e    eDevType;    ///< Rgn dev type
    MHAL_RGN_TargetType_e eTargetType; ///> Rgn target module type
    MS_U32                u32Id;       ///> Rgn target module dev id
} MHAL_RGN_DevAttr_t;

/// @brief RGN GOP CMDQ configuration
typedef struct
{
    MHAL_CMDQ_CmdqInterface_t *pstCmdqInf; ///< interface point of cmdq
} MHAL_RGN_CmdqConfig_t;

/// @brief RGN Win buffer configuation
typedef struct
{
    MS_PHY phyAddr; ///< Physical address
    void * pAddr;   ///< Virtual address
} MHAL_RGN_BufferConfig_t;

/// @brief RGN Resolution configuation
typedef struct MHAL_RGN_ResolutionConfig_s
{
    MS_U32 u32Width;  ///< Resolution width
    MS_U32 u32Height; ///< Resolution height
} MHAL_RGN_ResolutionConfig_t;

/// @brief RGN GOP window configuration
typedef struct MHAL_RGN_WindowConfig_s
{
    MS_U32 u32X;      ///< Horizontal start
    MS_U32 u32Y;      ///< Vertical start
    MS_U32 u32Width;  ///< Horizontal size
    MS_U32 u32Height; ///< Vertical size
} MHAL_RGN_WindowConfig_t;

/// @brief RGN GOP Palette contain for each entry
typedef struct
{
    MS_U8 u8A; ///< Color alpha
    MS_U8 u8R; ///< Color R
    MS_U8 u8G; ///< Color G
    MS_U8 u8B; ///< Color B
} MHAL_RGN_GopPaletteEntry_t;

typedef struct
{
    MS_U8 u8Y;
    MS_U8 u8U;
    MS_U8 u8V;
} MHAL_RGN_YUVPaletteEntry_t;

/// @brief RGN GOP Palette configuration
typedef struct
{
    MS_U8                      u8Idx;   ///< Index of palette
    MHAL_RGN_GopPaletteEntry_t stEntry; ///< Data for u8Idx
} MHAL_RGN_GopPaletteConfig_t;

/// @brief RGN GOP Palette talbe configuration
typedef struct
{
    MS_U32                      u32EntryNum; ///< Total entry number of palette table
    MHAL_RGN_GopPaletteEntry_t *pstEntry;    ///< Data for whole palette table
} MHAL_RGN_GopPaletteTblConfig_t;

/// @brief RGN GOP stretch windows configuration
typedef struct
{
    MHAL_RGN_WindowConfig_t stSrcWinCfg;  ///< Source windows configuration
    MHAL_RGN_WindowConfig_t stDestWinCfg; ///< Destintaion window configuration
} MHAL_RGN_GopStretchWinConfig_t;

/// @brief RGN GOP colorkey configuration
typedef struct
{
    MS_BOOL bEn; ///< 1:enable colorkey, 0:disable color key
    MS_U8   u8R; ///< Color R
    MS_U8   u8G; ///< Color G
    MS_U8   u8B; ///< Color B
} MHAL_RGN_GopColorKeyConfig_t;

/// @brief RGN GOP invert alpha vlaue configuration
typedef struct
{
    MS_BOOL                   bEn;          ///< 1:enable, 0:disable
    MS_BOOL                   bConstAlpha;  ///< 1: constant alpha, 0:pixel alpha
    MHAL_RGN_GopPixelFormat_e ePixelFormat; ///< Pixel format of souce image
} MHAL_RGN_GopAlphaZeroOpaqueConfig_t;

/// @brief RGN GOP output color fomrat configuration
typedef struct
{
    MHAL_RGN_GopOutFmtType_e eFmt; ///< Enum of output fomat
} MHAL_RGN_GopOutputFmtConfig_t;

/// @brief RGN GOP trigger configuration for VENC/JPE
typedef struct
{
    MHAL_RGN_GopEncMode_e eEncMode; ///< Enum of vnec/jpe trigger mode
} MHAL_RGN_GopEncFormatConfig_t;

/// @brief RGN GOP brighntess configuration
typedef struct
{
    MS_S16 s16Brightness; ///< Value of brightness,  -256 ~ 255
} MHAL_RGN_GopBrightnessConfig_t;

/// @brief RGN GOP contrast configuration
typedef struct
{
    MS_U8 u8Gain_R; ///< Value of Gain R, 0x00 ~ 0x3F
    MS_U8 u8Gain_G; ///< Value of Gain G, 0x00 ~ 0x3F
    MS_U8 u8Gain_B; ///< Value of Gain B, 0x00 ~ 0x3F
} MHAL_RGN_GopContrastConfig_t;

/// @brief RGN GOP Dma Threshold configuration
typedef struct
{
    MS_U32 u32Val;
} MHAL_RGN_GopDmaThreaholdConfig_t;

/// @brief RGN GOP Gwin pixel format configuration
typedef struct
{
    MS_U32                    u32GwinId;    ///< GWIN ID
    MHAL_RGN_GopPixelFormat_e ePixelFormat; ///< enum type of pixel format
} MHAL_RGN_GopGwinPixelFmtConfig_t;

/// @brief RGN GOP Gwin palette index configuration
typedef struct
{
    MS_U32 u32GwinId;
    MS_U8  u8PaletteIdx;
} MHAL_RGN_GopGwinPaletteIdxConfig_t;

/// @brief RGN GOP Gwin window configuration
typedef struct
{
    MS_U32                  u32GwinId; ///< GWIN ID
    MS_U32                  u32Stride; ///< Memory Stride of Gwin
    MHAL_RGN_WindowConfig_t stWinCfg;  ///< Gwin window configuration
} MHAL_RGN_GopGwinWinConfig_t;

/// @brief RGN GOP Gwin buffer configuration
typedef struct
{
    MS_U32 u32GwinId;  ///< GWIN ID
    MS_U16 u16Xoffset; ///< Value of X offset
    MS_PHY phyAddr;    ///< Physical base address of buffer
} MHAL_RGN_GopGwinBufferConfig_t;

/// @brief RGN GOP Gwin OnOff configuration
typedef struct
{
    MS_U32  u32GwinId; ///< GWIN ID
    MS_BOOL bEn;       ///< 1: enable Gwin, 0: disable Gwin
} MHAL_RGN_GopGwinOnOffConfig_t;

/// @brief RGN GOP Gwin alpha blending configuration
typedef struct
{
    MS_U32                      u32GwinId;       ///< GWIN ID
    MHAL_RGN_GopGwinAlphaType_e eAlphaType;      ///< enum of alpha type
    MS_U8                       u8ConstAlphaVal; ///< Value of constant alpha
} MHAL_RGN_GopGwinAlphaConfig_t;

/// @brief RGN GOP blending configuration
typedef struct
{
    MHAL_RGN_GopBlendType_e eBlendType;    ///< enum of blend type
    u16                     u16PlaneAlpha; ///< Value of plane alpha
} MHAL_RGN_GopBlendConfig_t;

/// @brief RGN GOP Gwin ARGB1555 alpha blending configuration
typedef struct
{
    MS_U32                              u32GwinId;  ///< GWIN ID
    MHAL_RGN_GopGwinArgb1555AlpahType_e eAlphaType; ///< enum of argb1555 type
    MS_U8                               u8AlphaVal; ///< value of argb1555 alpha0 or alpha1
} MHAL_RGN_GopGwinArgb1555AlphaConfig_t;

/// @brief RGN IFCD attribute confiuration
typedef struct
{
    MS_BOOL                   bHalfBlock;      ///< 1:enable half block, 0:disable half block
    MS_BOOL                   bBlockSplit;     ///< 1:enable block split, 0:disable block split
    MS_BOOL                   bColorTransform; ///< 1: enable color transform: 0: disable color transform
    MHAL_RGN_GopPixelFormat_e ePixelFmt;       ///< pixel format of source buffer
    MS_U16                    u16Width;        ///< width of IFCD  image
    MS_U16                    u16Height;       ///< height of IFCD image
} MHAL_RGN_IfcdAttrConfig_t;

/// @brief RGN IFCD buffer confiuration
typedef struct
{
    MS_PHY phyHeaderAddr; ///< IFCD header address
    MS_PHY phyImiAddr;    ///< IMI address(sram address)
} MHAL_RGN_IfcdBufferConfig_t;

/// @brief RGN IFCD on/off configuration
typedef struct
{
    MS_BOOL bEn; ///< 1:enable, 0:disable
} MHAL_RGN_IfcdOnOffConfig_t;

/// @brief RGN COVER Palette table configuration
typedef struct
{
    MS_U8                      u8Idx;   ///< Palette index
    MHAL_RGN_YUVPaletteEntry_t stEntry; ///< Data oF u8Idx palette
} MHAL_RGN_CoverPaletteConfig_t;

/// @brief RGN COVER Color configuation
typedef struct
{
    MS_U32                    u32WinId;   ///< COVER Win ID
    MHAL_RGN_CoverColorMode_e eColorMode; ///< COVER Color mode (pixel / palette)
    union
    {
        MS_U8  u8PaletteIdx; ///< If E_MHAL_RGN_COVER_COLOR_MODE_COVER: use this palette idx
        MS_U32 u32Color;     ///< Color value Y U V
    };
} MHAL_RGN_CoverColorConfig_t;

/// @brief RGN COVER Win block size configuration
typedef struct
{
    MS_U32                    u32WinId;   ///< COVER Win ID
    MHAL_RGN_CoverBlockSize_e eBlockSize; ///< block size configuration
} MHAL_RGN_CoverBlockConfig_t;

/// @brief RGN COVER Map configuation
typedef struct
{
    MS_PHY                       phyAddr;    ///< Map buffer phyAddr
    MHAL_RGN_CoverMapBlockSize_e eBlockSize; ///< block size of map
    MS_BOOL                      bEn;        ///< enable map
} MHAL_RGN_CoverMapConfig_t;

/// @brief RGN COVER Window configuration
typedef struct
{
    MS_U32                  u32WinId; ///< COVER Win ID
    MHAL_RGN_WindowConfig_t stWinCfg; ///< windows configuration
} MHAL_RGN_CoverWindowConfig_t;

/// @brief RGN COVER Win OnOff configuration
typedef struct
{
    MS_U32  u32WinId; ///< COVER Win ID
    MS_BOOL bEn;      ///< 1:enable, 0:disable
} MHAL_RGN_CoverOnOffConfig_t;

/// @brief RGN FRAME Palette table configuration
typedef struct
{
    MS_U8                      u8Idx;   ///< Palette index
    MHAL_RGN_YUVPaletteEntry_t stEntry; ///< Data oF u8Idx palette
} MHAL_RGN_FramePaletteConfig_t;

/// @brief RGN FRAME color index configuration
typedef struct
{
    MS_U32 u32WinId; ///< FRAME Win ID
    MS_U8  u8Idx;    ///< FRAME color index
} MHAL_RGN_FrameColorConfig_t;

/// @brief RGN FRAME window configuration
typedef struct
{
    MS_U32                  u32WinId; ///< FRAME Win ID
    MHAL_RGN_WindowConfig_t stWinCfg; ///< FRAME window configuration
} MHAL_RGN_FrameWindowConfig_t;

/// @brief RGN FRAME border configuration
typedef struct
{
    MS_U32 u32WinId; ///< FRAME Win ID
    MS_U32 u32Size;  ///< FRAME border size, border width = u32Size * 2 / 2
} MHAL_RGN_FrameBorderConfig_t;

/// @brief RGN FRAME OnOff configuration
typedef struct
{
    MS_U32  u32WinId; ///< FRAME Win ID
    MS_BOOL bEn;      ///< FRAME OnOff
} MHAL_RGN_FrameOnOffConfig_t;

/// @brief RGN COLOR INVERT cal mode configuration
typedef struct
{
    MHAL_RGN_ColorInvertCalMode_e eCalMode; ///< COLOR INVERT cal mode
} MHAL_RGN_ColorInvertCalModeConfig_t;

/// @brief RGN COLOR INVERT storage mode configuration
typedef struct
{
    MHAL_RGN_ColorInvertStorageMode_e eStorageMode; ///< storage mode
    MS_BOOL bEnPreRead; ///< 1:enable, 0:disable, if bEnPreRead == 0: eStorageMode = 0; Disable bEnPreRead when first
                        ///< frame and enable it begin second frame
} MHAL_RGN_ColorInvertStorageModeConfig_t;

/// @brief RGN COLOR INVERT block size configuration
typedef struct
{
    MHAL_RGN_ColorInvertBlockSize_e eBlockSizeH; ///< block size H
    MHAL_RGN_ColorInvertBlockSize_e eBlockSizeV; ///< block size V
} MHAL_RGN_ColorInvertBlockSizeConfig_t;

/// @brief RGN COLOR INVERT Threshold configuration
/// |------l-------h------|
/// |000000xxxxxxxx1111111|
/// 0: inv_bit = 0, x: inv_bit = last_inv_bit, 1: inv_bit = 1
typedef struct
{
    MS_U8 u8ThresholdHigh; ///< threahold high
    MS_U8 u8ThresholdLow;  ///< threahold low
} MHAL_RGN_ColorInvertThresholdConfig_t;

/// @brief RGN COLOR INVERT Dram configuration
/// packet format: pre_val [7:1] , inv_bit [1:0]
typedef struct
{
    MS_PHY phyRAddr; ///< CI read pre_val and inv_bit from phyRAddr
    MS_PHY phyWAddr; ///< CI write pre_val and inv_bit to phyWAddr
} MHAL_RGN_ColorInvertBufferConfig_t;

/// @brief RGN COLOR INVERT OnOff configuration
typedef struct
{
    MS_BOOL bEn; ///< COLOR INVERT En
} MHAL_RGN_ColorInvertOnOffConfig_t;

#endif //_MHAL_RGN_DATATYPE_H_
