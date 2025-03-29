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

#ifndef __PTREE_MOD_VIF_H__
#define __PTREE_MOD_VIF_H__

#include "ptree_sur_sys.h"
#include "mi_sys_datatype.h"

typedef struct PTREE_SUR_VIF_Info_s
{
    PTREE_SUR_SYS_Info_t base;
    int                  s32SensorId;
    int                  s32HdrType;
    int                  s32HdrExposureMask;
    int                  s32WorkMode;
    unsigned int         u32StitchMask;
} PTREE_SUR_VIF_Info_t;

typedef struct PTREE_SUR_VIF_OutInfo_s
{
    PTREE_SUR_SYS_OutInfo_t base;
    unsigned int            u32CropX;
    unsigned int            u32CropY;
    unsigned int            u32CropW;
    unsigned int            u32CropH;
    unsigned int            u32Width;
    unsigned int            u32Height;
    int                     s32IsUseSnrFmt;
    int                     s32CompressMode;
    MI_SYS_PixelFormat_e    eOutFmt;
} PTREE_SUR_VIF_OutInfo_t;

#endif //__PTREE_MOD_VIF_H__
