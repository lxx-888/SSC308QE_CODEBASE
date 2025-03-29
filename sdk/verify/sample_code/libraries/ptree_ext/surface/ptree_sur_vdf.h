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

#ifndef __PTREE_SUR_VDF_H__
#define __PTREE_SUR_VDF_H__

#include "ptree_sur_sys.h"
#include "mi_vdf_datatype.h"

typedef enum PTREE_SUR_VDF_OdMotionSensitivelyS_e
{
    E_PTREE_SUR_VDF_OD_MOTIONSENSITIVELY_MIN    = 0,
    E_PTREE_SUR_VDF_OD_MOTIONSENSITIVELY_LOW    = 1,
    E_PTREE_SUR_VDF_OD_MOTIONSENSITIVELY_MIDDLE = 2,
    E_PTREE_SUR_VDF_OD_MOTIONSENSITIVELY_HIGH   = 3,
    E_PTREE_SUR_VDF_OD_MOTIONSENSITIVELY_MAX    = 4
} PTREE_SUR_VDF_OdMotionSensitivelyE_t;

typedef struct PTREE_SUR_VDF_Point_s
{
    unsigned int u32x;
    unsigned int u32y;
} PTREE_SUR_VDF_Point_t; // 定义点坐标

typedef struct PTREE_SUR_VDF_MdAttr_s
{
    unsigned int                 u32Sensitivity;
    unsigned int                 u32Thr;
    unsigned int                 u32ObjNumMax;
    unsigned int                 u32LearnRate;
    unsigned int                 u32PointNum;
    struct PTREE_SUR_VDF_Point_s stPnt[8];
} PTREE_SUR_VDF_MdAttr_t;

typedef struct PTREE_SUR_VDF_OdAttr_s
{
    unsigned int                 u32PointNum;
    struct PTREE_SUR_VDF_Point_s stPnt[8];
} PTREE_SUR_VDF_OdAttr_t;

typedef struct PTREE_SUR_VDF_Info_s
{
    PTREE_SUR_SYS_Info_t base;
    MI_VDF_WorkMode_e    enVdfMode;      //"md", "od", "vg"
    MDALG_MODE_e         enAlgMode;      // md: 0x0 == fg, 1 == sad, 2 == framediff; vg: 0 == gate, 1 == Reg
    MDMB_MODE_e          enMdMbMode;     // 0 == 4x4, 1 == 8x8, 2 == 16x16
    MDSAD_OUT_CTRL_e     enMdSadOutMode; // 0 == 8bit, 1 == 16bit
    ODWindow_e           enOdWindows;    // suport 1x1, 2x2, 3x3
    char strSensitivity[16]; // od: 0:low, 1：middle, 2：hight, vg: 0 == min, 1 == low, 2 == middle, 3 == high, 4 == max
    PTREE_SUR_VDF_OdMotionSensitivelyE_t enMotionSensitivity; // od 0 == min, 1 == low, 2 == middle, 3 == high, 4 == max
    union
    {
        struct PTREE_SUR_VDF_MdAttr_s stMdAttr;
        struct PTREE_SUR_VDF_OdAttr_s stOdAttr;
    };
} PTREE_SUR_VDF_Info_t;

#endif //__PTREE_SUR_VDF_H__
