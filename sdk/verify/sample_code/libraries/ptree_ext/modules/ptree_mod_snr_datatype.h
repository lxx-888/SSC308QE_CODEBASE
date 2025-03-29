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

#ifndef __PTREE_MOD_SNR_DATATYPE_H__
#define __PTREE_MOD_SNR_DATATYPE_H__
#include "mi_sensor.h"

typedef struct PTREE_MOD_SNR_DATATYPE_SensorDrvInfo_s
{
    MI_SNR_PADInfo_t   stPadInfo;
    MI_SNR_PlaneInfo_t stSnrPlaneInfo;
    MI_BOOL            bHdrEnable;
} PTREE_MOD_SNR_DATATYPE_SensorDrvInfo_t;

PTREE_MOD_SNR_DATATYPE_SensorDrvInfo_t *PTREE_MOD_SNR_DATATYPE_GetSensorInfo(MI_U32 u32SnrId);
#endif //__PTREE_MOD_SNR_DATATYPE_H__
