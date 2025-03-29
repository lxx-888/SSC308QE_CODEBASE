/*
 * hal_disp_picture.h- Sigmastar
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
#ifndef _HAL_DISP_PICTURE_H_
#define _HAL_DISP_PICTURE_H_

//-------------------------------------------------------------------------------------------------
//  Defines & Macro
//-------------------------------------------------------------------------------------------------
#define HAL_DISP_PICTURE_SHARPNESS_NUM 2

//-------------------------------------------------------------------------------------------------
//  structure & Enum
//-------------------------------------------------------------------------------------------------
typedef enum
{
    E_HAL_DISP_PICTURE_CONTRAST = 0,
    E_HAL_DISP_PICTURE_R_BRIGHTNESS,
    E_HAL_DISP_PICTURE_G_BRIGHTNESS,
    E_HAL_DISP_PICTURE_B_BRIGHTNESS,
    E_HAL_DISP_PICTURE_SATURATION,
    E_HAL_DISP_PICTURE_SHARPNESS,
    E_HAL_DISP_PICTURE_SHARPNESS1,
    E_HAL_DISP_PICTURE_HUE,
    E_HAL_DISP_PICTURE_NUM,
} HAL_DISP_PICTURE_Type_e;

typedef struct
{
    MI_U16 u16OSD_0;
    MI_U16 u16OSD_25;
    MI_U16 u16OSD_50;
    MI_U16 u16OSD_75;
    MI_U16 u16OSD_100;
} HAL_DISP_PICTURE_NonLinearCurveType_t;

typedef struct
{
    HAL_DISP_PICTURE_NonLinearCurveType_t stDispDevice[E_HAL_DISP_PICTURE_NUM];
} HAL_DISP_PICTURE_NonLinearCurveConfig_t;

typedef struct
{
    MI_U16 u16Hue;
    MI_U16 u16Saturation;
    MI_U16 u16Contrast;
    MI_U16 u16BrightnessR;
    MI_U16 u16BrightnessG;
    MI_U16 u16BrightnessB;
    MI_U16 u16Sharpness[HAL_DISP_PICTURE_SHARPNESS_NUM];
} HAL_DISP_PICTURE_Config_t;

typedef struct
{
    MI_U8  bUpdate;
    MI_U16 u16GainR;
    MI_U16 u16GainG;
    MI_U16 u16GainB;
    MI_U16 u16BrightnessR;
    MI_U16 u16BrightnessG;
    MI_U16 u16BrightnessB;
    MI_U16 u16Hue;
    MI_U16 u16Saturation;
    MI_U16 u16Contrast;
    MI_U16 u16Sharpness;
    MI_U16 u16Sharpness1;
    MI_S16 as16Coef[9];
} HAL_DISP_PICTURE_PqConfig_t;
//-------------------------------------------------------------------------------------------------
//  Prototype
//-------------------------------------------------------------------------------------------------

#ifdef _HAL_DISP_PICTURE_C_
#define INTERFACE
#else
#define INTERFACE extern
#endif

INTERFACE void  HAL_DISP_PICTURE_SetConfig(MI_DISP_CscMatrix_e eCscMatrix, HAL_DISP_PICTURE_Config_t *pstPictureCfg,
                                           void *pCtx);
INTERFACE MI_U8 HAL_DISP_PICTURE_TransNonLinear(void *pCtx, MI_DISP_Csc_t *pstCsc, MI_U32 *pu32Sharpness,
                                                HAL_DISP_PICTURE_Config_t *pstPictureCfg);
INTERFACE void  HAL_DISP_PICTURE_SetPqConfig(MI_U32 u32DevId, HAL_DISP_PICTURE_PqConfig_t *pstPqInCfg);
INTERFACE MI_U8 HAL_DISP_PICTURE_IsPqUpdate(MI_U32 u32DevId);
INTERFACE void  HAL_DISP_PICTURE_ApplyPqConfig(void *pCtx);

#undef INTERFACE
#endif
