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

#ifndef __PTREE_SUR_SNR_H__
#define __PTREE_SUR_SNR_H__

#include "ptree_sur_sys.h"

typedef struct PTREE_SUR_SNR_Info_s
{
    PTREE_SUR_SYS_Info_t base;
    int                  s32SensorId;
    int                  s32SensorRes;
    int                  s32HdrType;
    int                  s32SensorFps;
    int                  s32SensorMirror;
    int                  s32SensorFlip;
} PTREE_SUR_SNR_Info_t;

#endif //__PTREE_SUR_SNR_H__
