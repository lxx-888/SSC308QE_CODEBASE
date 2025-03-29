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
#ifndef _MI_NIR_H_
#define _MI_NIR_H_

#include "mi_nir_datatype.h"

#define NIR_MAJOR_VERSION   0
#define NIR_SUB_VERSION     1
#define MACRO_TO_STR(macro) #macro
#define NIR_VERSION_STR(major_version, sub_version)                                                                 \
    (                                                                                                               \
        {                                                                                                           \
            char *tmp =                                                                                             \
                sub_version / 100  ? "mi_nir_version_" MACRO_TO_STR(major_version) "." MACRO_TO_STR(sub_version)    \
                : sub_version / 10 ? "mi_nir_version_" MACRO_TO_STR(major_version) ".0" MACRO_TO_STR(sub_version)   \
                                   : "mi_nir_version_" MACRO_TO_STR(major_version) ".00" MACRO_TO_STR(sub_version); \
            tmp;                                                                                                    \
        })
#define MI_NIR_API_VERSION NIR_VERSION_STR(NIR_MAJOR_VERSION, NIR_SUB_VERSION)

#ifdef __cplusplus
extern "C"
{
#endif

    MI_S32 MI_NIR_CreateDevice(MI_NIR_DEV DevId, MI_NIR_DevAttr_t *pstDevAttr);
    MI_S32 MI_NIR_DestroyDevice(MI_NIR_DEV DevId);

    MI_S32 MI_NIR_CreateChannel(MI_NIR_DEV DevId, MI_NIR_CHANNEL ChnId, MI_NIR_ChannelAttr_t *pstChAttr);
    MI_S32 MI_NIR_DestroyChannel(MI_NIR_DEV DevId, MI_NIR_CHANNEL ChnId);

    MI_S32 MI_NIR_SetChnParam(MI_NIR_DEV DevId, MI_NIR_CHANNEL ChnId, MI_NIR_ChnParam_t *pstChnParam);
    MI_S32 MI_NIR_GetChnParam(MI_NIR_DEV DevId, MI_NIR_CHANNEL ChnId, MI_NIR_ChnParam_t *pstChnParam);

    MI_S32 MI_NIR_StartChannel(MI_NIR_DEV DevId, MI_NIR_CHANNEL ChnId);
    MI_S32 MI_NIR_StopChannel(MI_NIR_DEV DevId, MI_NIR_CHANNEL ChnId);

#ifdef __cplusplus
}
#endif

#endif
