/* SigmaStar trade secret */
/* Copyright (c) [2019~2020] SigmaStar Technology.
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

#ifndef __PTREE_SUR_RGN_H__
#define __PTREE_SUR_RGN_H__

#include "ptree_sur_sys.h"
#include "mi_rgn_datatype.h"

#define PTREE_SUR_RGN_ATTACH_MAX (8)

enum PTREE_SUR_RGN_InMode_e
{
    E_PTREE_SUR_RGN_IN_MODE_FRAME,
    E_PTREE_SUR_RGN_IN_MODE_OSD_FRAME,
    E_PTREE_SUR_RGN_IN_MODE_OSD_DOT_MATRIX,
};

enum PTREE_SUR_RGN_Thickness_e
{
    E_PTREE_SUR_RGN_THICKNESS_THIN,
    E_PTREE_SUR_RGN_THICKNESS_NORMAL,
    E_PTREE_SUR_RGN_THICKNESS_THICK,
};

enum PTREE_SUR_RGN_Size_e
{
    E_PTREE_SUR_RGN_SIZE_TINY,
    E_PTREE_SUR_RGN_SIZE_SMALL,
    E_PTREE_SUR_RGN_SIZE_NORMAL,
    E_PTREE_SUR_RGN_SIZE_LARGE,
    E_PTREE_SUR_RGN_SIZE_HUGE,
};

typedef struct PTREE_SUR_RGN_FrameInfo_s
{
    unsigned int                   color;
    enum PTREE_SUR_RGN_Thickness_e thickness;
} PTREE_SUR_RGN_FrameInfo_t;

typedef struct PTREE_SUR_RGN_OsdFrameInfo_s
{
    MI_RGN_PixelFormat_e           ePixelFormat;
    unsigned int                   color;
    enum PTREE_SUR_RGN_Thickness_e thickness;
} PTREE_SUR_RGN_OsdFrameInfo_t;

typedef struct PTREE_SUR_RGN_OsdDotMatrixInfo_s
{
    MI_RGN_PixelFormat_e      ePixelFormat;
    unsigned int              color;
    enum PTREE_SUR_RGN_Size_e size;
} PTREE_SUR_RGN_OsdDotMatrixInfo_t;

typedef struct PTREE_SUR_RGN_TypeInfo_s
{
    union
    {
        PTREE_SUR_RGN_FrameInfo_t        stFrameInfo;
        PTREE_SUR_RGN_OsdFrameInfo_t     stOsdFrameInfo;
        PTREE_SUR_RGN_OsdDotMatrixInfo_t stOsdDotMatrixInfo;
    };
} PTREE_SUR_RGN_TypeInfo_t;

typedef struct PTREE_SUR_RGN_AttachInfo_s
{
    MI_RGN_ChnPort_t stChnPort;
    unsigned int     timingW;
    unsigned int     timingH;
} PTREE_SUR_RGN_AttachInfo_t;

typedef struct PTREE_SUR_RGN_Info_s
{
    PTREE_SUR_SYS_Info_t       base;
    unsigned int               attachCnt;
    PTREE_SUR_RGN_AttachInfo_t astAttachInfo[PTREE_SUR_RGN_ATTACH_MAX];
} PTREE_SUR_RGN_Info_t;

typedef struct PTREE_SUR_RGN_InInfo_s
{
    PTREE_SUR_SYS_InInfo_t      base;
    enum PTREE_SUR_RGN_InMode_e mode;
    PTREE_SUR_RGN_TypeInfo_t    info;
} PTREE_SUR_RGN_InInfo_t;

#endif /* ifndef __PTREE_SUR_RGN_H__ */
