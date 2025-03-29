/* SigmaStar trade secret */
/* Copyright (c) [2019~2022] SigmaStar Technology.
All rights reserved.

Unless otherwise stipulated in writing, any and all information contained
herein regardless in any format shall remain the sole proprietary of
SigmaStar and be kept in strict confidence
(SigmaStar Confidential Information) by the recipient.
Any unauthorized act including without limitation unauthorized disclosure,
copying, use, reproduction, sale, distribution, modification, disassembling,
reverse engineering and compiling of the contents of SigmaStar Confidential
Information is unlawful and strictly prohibited. SigmaStar hereby reserves the
rights to any and all damages, losses, costs and expenses resulting therefrom.
*/

#ifndef __MI_GDL_H__
#define __MI_GDL_H__
#include "mi_common.h"


#ifdef __cplusplus
extern "C"
{
#endif


typedef enum
{
    MI_GDL_SUCCESS_E = 0x0,
    MI_GDL_ERR_INVALID_PARA_E,
} MI_GDL_ErrorType_e;

typedef struct
{
    MI_S32 x;
    MI_S32 y;
} MI_GDL_RectCoord_t;

typedef struct
{
    MI_FLOAT phi;
    MI_FLOAT theta;
} MI_GDL_SphCoord_t;

typedef struct
{
    MI_FLOAT fx;
    MI_FLOAT fy;
    MI_FLOAT cx;
    MI_FLOAT cy;
} MI_GDL_Intr;

typedef struct
{
    MI_FLOAT R[3][3];
} MI_GDL_Wac2PtzMatrix_t;


typedef struct
{
    void *data;
    MI_U16 stride;
    MI_U16 width;
    MI_U16 height;
} MI_GDL_Image_t;

MI_GDL_ErrorType_e MI_GDL_GenWac2PtzMatrix(MI_GDL_Intr *pstWacIntr, MI_U16 u16CoordNum, MI_GDL_RectCoord_t *pstWacCoord, MI_GDL_SphCoord_t *pstPtzCoord, MI_GDL_Wac2PtzMatrix_t *pstMatrix);

MI_GDL_ErrorType_e MI_GDL_MapWac2Ptz(MI_GDL_Intr *pstWacIntr, MI_GDL_Wac2PtzMatrix_t *pstMatrix, MI_GDL_RectCoord_t *pstWacPoint, MI_GDL_SphCoord_t *pstPtzPoint);

MI_GDL_ErrorType_e MI_GDL_MapPtz2Wac(MI_GDL_Intr *pstWacIntr, MI_GDL_Wac2PtzMatrix_t *pstMatrix, MI_GDL_SphCoord_t *pstPtzPoint, MI_GDL_RectCoord_t *pstWacPoint);

MI_GDL_ErrorType_e MI_GDL_MapPtzCenter2Wac(MI_GDL_Image_t *pstWacImg, MI_GDL_Image_t *pstPtzImg, MI_GDL_RectCoord_t *pstWacPoint);

#ifdef __cplusplus
}
#endif

#endif
