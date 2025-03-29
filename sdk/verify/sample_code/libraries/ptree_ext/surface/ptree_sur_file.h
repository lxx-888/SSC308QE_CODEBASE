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

#ifndef __PTREE_SUR_FILE_H__
#define __PTREE_SUR_FILE_H__

#include "ptree_sur.h"

typedef struct PTREE_SUR_FILE_InInfo_s
{
    PTREE_SUR_InInfo_t base;
    char               fileName[256];
    int                frameCntLimit;
    unsigned char      bAddHead;
} PTREE_SUR_FILE_InInfo_t;

typedef struct PTREE_SUR_FILE_OutInfo_s
{
    PTREE_SUR_OutInfo_t base;
    char                fileName[256];
    char                outType[16];
    char                outFmt[16];
    char                bayerId[16];
    char                precision[16];
    unsigned short      videoWidth;
    unsigned short      videoHeight;
    unsigned char       planeNum;
    int                 frameCntLimit;
} PTREE_SUR_FILE_OutInfo_t;

#endif //__PTREE_MOD_FILE_H__
