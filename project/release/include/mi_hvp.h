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
#include "mi_common_datatype.h"
#include "mi_hvp_datatype.h"

#ifndef __MI_HVP_H__
#define __MI_HVP_H__
#define HVP_MAJOR_VERSION   3
#define HVP_SUB_VERSION     1
#define MACRO_TO_STR(macro) #macro
#define HVP_VERSION_STR(major_version, sub_version)                                                                 \
    (                                                                                                               \
        {                                                                                                           \
            char *tmp =                                                                                             \
                sub_version / 100  ? "mi_hvp_version_" MACRO_TO_STR(major_version) "." MACRO_TO_STR(sub_version)    \
                : sub_version / 10 ? "mi_hvp_version_" MACRO_TO_STR(major_version) ".0" MACRO_TO_STR(sub_version)   \
                                   : "mi_hvp_version_" MACRO_TO_STR(major_version) ".00" MACRO_TO_STR(sub_version); \
            tmp;                                                                                                    \
        })
#define MI_HVP_API_VERSION HVP_VERSION_STR(HVP_MAJOR_VERSION, HVP_SUB_VERSION)
#ifdef __cplusplus
extern "C"
{
#endif
    MI_S32 MI_HVP_CreateDevice(MI_HVP_DEV devId, const MI_HVP_DeviceAttr_t *pstDevAttr);
    MI_S32 MI_HVP_DestroyDevice(MI_HVP_DEV devId);
    MI_S32 MI_HVP_GetSourceSignalStatus(MI_HVP_DEV devId, MI_HVP_SignalStatus_e *penSignalStatus);
    MI_S32 MI_HVP_GetSourceSignalTiming(MI_HVP_DEV devId, MI_HVP_SignalTiming_t *pstSrcTiming);
    MI_S32 MI_HVP_GetResetEventFd(MI_HVP_DEV devId, MI_S32 *p32Fd);
    MI_S32 MI_HVP_CloseResetEventFd(MI_HVP_DEV devId, MI_S32 s32Fd);
    MI_S32 MI_HVP_GetResetEvent(MI_HVP_DEV devId, MI_BOOL *pbTrigger);
    MI_S32 MI_HVP_ClearResetEvent(MI_HVP_DEV devId);
    MI_S32 MI_HVP_CreateChannel(MI_HVP_DEV devId, MI_HVP_CHN chnId, const MI_HVP_ChannelAttr_t *pstChnAttr);
    MI_S32 MI_HVP_DestroyChannel(MI_HVP_DEV devId, MI_HVP_CHN chnId);
    MI_S32 MI_HVP_StartChannel(MI_HVP_DEV devId, MI_HVP_CHN chnId);
    MI_S32 MI_HVP_StopChannel(MI_HVP_DEV devId, MI_HVP_CHN chnId);
    MI_S32 MI_HVP_SetChannelParam(MI_HVP_DEV devId, MI_HVP_CHN chnId, const MI_HVP_ChannelParam_t *pstChnParam);
    MI_S32 MI_HVP_GetChannelParam(MI_HVP_DEV devId, MI_HVP_CHN chnId, MI_HVP_ChannelParam_t *pstChnParam);
    MI_S32 MI_HVP_SetVideoMute(MI_HVP_DEV devId, MI_HVP_CHN chnId, MI_BOOL bEnable, MI_U32 u32MuteColor);
#ifdef __cplusplus
}
#endif
#endif
