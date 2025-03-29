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

#ifndef __PTREE_SUR_DISP_H__
#define __PTREE_SUR_DISP_H__
#include "ptree_sur_sys.h"

#define PTREE_SUR_DISP_LAYER_MAX (16)

typedef struct PTREE_SUR_DISP_LayerInfo_s
{
    unsigned int uintId;
    unsigned int uintRot;
    unsigned int uintWidth;
    unsigned int uintHeight;
    unsigned int uintDispWidth;
    unsigned int uintDispHeight;
    unsigned int uintDispXpos;
    unsigned int uintDispYpos;
} PTREE_SUR_DISP_LayerInfo_t;

typedef struct PTREE_SUR_DISP_Info_s
{
    PTREE_SUR_SYS_Info_t       base;
    char                       chDevType[16];     // 0: panel 1: hdmi: 2: vga 3: cvbs out
    char                       chPnlLinkType[16]; // 0: mipi 11: ttl
    char                       chOutTiming[16];
    char                       chBackGroundColor[16];
    unsigned int               uintLayerCount;
    PTREE_SUR_DISP_LayerInfo_t stDispLayerInfo[PTREE_SUR_DISP_LAYER_MAX];
} PTREE_SUR_DISP_Info_t;

typedef struct PTREE_SUR_DISP_InInfo_s
{
    PTREE_SUR_SYS_InInfo_t base;
    unsigned int           uintSrcWidth;
    unsigned int           uintSrcHeight;
    unsigned int           uintDstWidth;
    unsigned int           uintDstHeight;
    unsigned int           uintDstXpos;
    unsigned int           uintDstYpos;
    unsigned int           uintSysChn;
    unsigned int           uintLayId;
    unsigned int           uintLayPortId;
} PTREE_SUR_DISP_InInfo_t;

#endif //__PTREE_MOD_DISP_H__
