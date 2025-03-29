/*
 * hal_disp_color.h- Sigmastar
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
#ifndef _HAL_DISP_COLOR_H_
#define _HAL_DISP_COLOR_H_

//-------------------------------------------------------------------------------------------------
//  Defines & Macro
//-------------------------------------------------------------------------------------------------
#define HAL_DISP_COLOR_CSC_NUM 4

#define HAL_DISP_COLOR_CSC_ID 0

#define ENABLE_CBCR 1

//-------------------------------------------------------------------------------------------------
//  structure & Enum
//-------------------------------------------------------------------------------------------------
typedef enum
{
    E_HAL_DISP_COLOR_YUV_2_RGB_MATRIX_BYPASS,
    E_HAL_DISP_COLOR_YUV_2_RGB_MATRIX_HDTV,
    E_HAL_DISP_COLOR_YUV_2_RGB_MATRIX_SDTV,
    E_HAL_DISP_COLOR_YUV_2_RGB_MATRIX_USER,
    E_HAL_DISP_COLOR_YUV_2_RGB_MATRIX_MAX,
} HAL_DISP_COLOR_YuvToRgbMatrixType_e;

typedef enum
{
    E_HAL_DISP_COLOR_CSC_ID_0,
    E_HAL_DISP_COLOR_CSC_ID_NUM,
} HAL_DISP_COLOR_CscIdType_e;

typedef struct
{
    MI_S16 *tSrgbMatrix;
    MI_U8   bColorCorrectMatrixUpdate;

    MI_U8                               u8ACEConfig;
    HAL_DISP_COLOR_YuvToRgbMatrixType_e enMatrixType;

    MI_U8 u8VideoRCon;
    MI_U8 u8VideoGCon;
    MI_U8 u8VideoBCon;
    MI_U8 u8VideoContrast;
    MI_U8 u8VideoSaturation;

    MI_U8 u8BrightnessR;
    MI_U8 u8BrightnessG;
    MI_U8 u8BrightnessB;

#if (ENABLE_CBCR)
    MI_U8 u8VideoCb;
    MI_U8 u8VideoCr;
#endif

    MI_U8 u8VideoHue;

    MI_S16 sYuvToRGBMatrix[3][3];
    MI_S16 sVideoSatHueMatrix[3][3];
    MI_S16 sVideoContrastMatrix[3][3];
    MI_S16 sColorCorrrectMatrix[3][3];

    // For PC
    MI_U8  u8PCRCon;
    MI_U8  u8PCGCon;
    MI_U8  u8PCBCon;
    MI_U8  u8PCContrast;
    MI_S16 sPCConRGBMatrix[3][3];
} HAL_DISP_COLOR_Config_t;
//-------------------------------------------------------------------------------------------------
//  Prototype
//-------------------------------------------------------------------------------------------------

#ifdef _HAL_DISP_COLOR_C_
#define INTERFACE
#else
#define INTERFACE extern
#endif

INTERFACE MI_U8 HAL_DISP_COLOR_InitVar(void *pCtx);
INTERFACE void  HAL_DISP_COLOR_SetColorMatrixEn(MI_U8 u8Id, MI_U8 bEn, MI_U32 u32DevId, void *pstDispCtx);
INTERFACE void  HAL_DISP_COLOR_SetColorMatrixMd(MI_U8 u8Id, HAL_DISP_COLOR_YuvToRgbMatrixType_e enMatrixType,
                                                MI_U32 u32DevId, void *pstDispCtx);
INTERFACE MI_U8 HAL_DISP_COLOR_SetColorCorrectMatrix(MI_U8 u8Id, MI_S16 *psColorCorrectMatrix, void *pCtx);
INTERFACE MI_U8 HAL_DISP_COLOR_SeletYuvToRgbMatrix(MI_U8 u8Id, HAL_DISP_COLOR_YuvToRgbMatrixType_e enType,
                                                   MI_S16 *psYuv2RgbMatrix, void *pCtx);
INTERFACE MI_U8 HAL_DISP_COLOR_AdjustBrightness(MI_U8 u8Id, MI_U8 u8BrightnessR, MI_U8 u8BrightnessG,
                                                MI_U8 u8BrightnessB, void *pCtx);
INTERFACE MI_U8 HAL_DISP_COLOR_AdjustHCS(MI_U8 u8Id, MI_U8 u8Hue, MI_U8 u8Saturation, MI_U8 u8Contrast, void *pCtx);
INTERFACE MI_U8 HAL_DISP_COLOR_AdjustVideoRGB(MI_U8 u8Id, MI_U8 u8RCon, MI_U8 u8GCon, MI_U8 u8BCon, void *pCtx);

#undef INTERFACE
#endif
