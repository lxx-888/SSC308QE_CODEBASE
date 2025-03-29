/* SigmaStar trade secret */
/* Copyright (c) [2021~2022] SigmaStar Technology.
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
#ifndef _MI_DPU_H_
#define _MI_DPU_H_

#include "mi_dpu_datatype.h"

#define DPU_MAJOR_VERSION   0
#define DPU_SUB_VERSION     1
#define MACRO_TO_STR(macro) #macro
#define DPU_VERSION_STR(major_version, sub_version)                                                                 \
    (                                                                                                               \
        {                                                                                                           \
            char *tmp =                                                                                             \
                sub_version / 100  ? "mi_dpu_version_" MACRO_TO_STR(major_version) "." MACRO_TO_STR(sub_version)    \
                : sub_version / 10 ? "mi_dpu_version_" MACRO_TO_STR(major_version) ".0" MACRO_TO_STR(sub_version)   \
                                   : "mi_dpu_version_" MACRO_TO_STR(major_version) ".00" MACRO_TO_STR(sub_version); \
            tmp;                                                                                                    \
        })
#define MI_DPU_API_VERSION DPU_VERSION_STR(DPU_MAJOR_VERSION, DPU_SUB_VERSION)

#ifdef __cplusplus
extern "C"
{
#endif

    MI_S32 MI_DPU_CreateDevice(MI_DPU_DEV DevId, MI_DPU_DevAttr_t *pstDevAttr);
    MI_S32 MI_DPU_DestroyDevice(MI_DPU_DEV DevId);

    MI_S32 MI_DPU_CreateChannel(MI_DPU_DEV DevId, MI_DPU_CHANNEL ChnId, MI_DPU_ChnAttr_t *pstChAttr);
    MI_S32 MI_DPU_DestroyChannel(MI_DPU_DEV DevId, MI_DPU_CHANNEL ChnId);

    MI_S32 MI_DPU_SetChnParam(MI_DPU_DEV DevId, MI_DPU_CHANNEL ChnId, MI_DPU_ChnParam_t *pstChnParam);
    MI_S32 MI_DPU_GetChnParam(MI_DPU_DEV DevId, MI_DPU_CHANNEL ChnId, MI_DPU_ChnParam_t *pstChnParam);

    MI_S32 MI_DPU_StartChannel(MI_DPU_DEV DevId, MI_DPU_CHANNEL ChnId);
    MI_S32 MI_DPU_StopChannel(MI_DPU_DEV DevId, MI_DPU_CHANNEL ChnId);

#ifdef __cplusplus
}
#endif

#endif
