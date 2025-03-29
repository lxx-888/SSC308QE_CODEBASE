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
#ifndef __MI_AO_H__
#define __MI_AO_H__

#include "mi_aio_datatype.h"
#include "mi_ao_datatype.h"

#define AO_MAJOR_VERSION    3
#define AO_SUB_VERSION      56
#define MACRO_TO_STR(macro) #macro
#define AO_VERSION_STR(major_version, sub_version)                                                                     \
    (                                                                                                                  \
        {                                                                                                              \
            char *tmp = sub_version / 100 ? "mi_ao_version_" MACRO_TO_STR(major_version) "." MACRO_TO_STR(sub_version) \
                        : sub_version / 10                                                                             \
                            ? "mi_ao_version_" MACRO_TO_STR(major_version) ".0" MACRO_TO_STR(sub_version)              \
                            : "mi_ao_version_" MACRO_TO_STR(major_version) ".00" MACRO_TO_STR(sub_version);            \
            tmp;                                                                                                       \
        })
#define MI_AO_API_VERSION AO_VERSION_STR(AO_MAJOR_VERSION, AO_SUB_VERSION)

#ifdef __cplusplus
extern "C"
{
#endif

//=============================================================================
// Include files
//=============================================================================

//=============================================================================
// Extern definition
//=============================================================================

//=============================================================================
// Macro definition
//=============================================================================

//=============================================================================
// Data type definition
//=============================================================================

//=============================================================================
// Variable definition
//=============================================================================

//=============================================================================
// Global function definition
//=============================================================================
MI_S32 MI_AO_Open(MI_AUDIO_DEV AoDevId, const MI_AO_Attr_t *pstAttr);
MI_S32 MI_AO_GetAttr(MI_AUDIO_DEV AoDevId, MI_AO_Attr_t *pstAttr);
MI_S32 MI_AO_OpenWithCfgFile(MI_AUDIO_DEV AoDevId, const char *pCfgPath);
MI_S32 MI_AO_Close(MI_AUDIO_DEV AoDevId);

MI_S32 MI_AO_AttachIf(MI_AUDIO_DEV AoDevId, MI_AO_If_e enAoIfs, MI_U32 u32AudioDelay);
MI_S32 MI_AO_DetachIf(MI_AUDIO_DEV AoDevId, MI_AO_If_e enAoIfs);

MI_S32 MI_AO_SetChannelMode(MI_AUDIO_DEV AoDevId, MI_AO_ChannelMode_e enChannelMode);

MI_S32 MI_AO_Write(MI_AUDIO_DEV AoDevId, const void *pvBuffer, MI_U32 u32Byte, MI_U64 u64Pts, MI_S32 s32TimeoutMs);

MI_S32 MI_AO_Start(MI_AUDIO_DEV AoDevId);
MI_S32 MI_AO_Stop(MI_AUDIO_DEV AoDevId);
MI_S32 MI_AO_Pause(MI_AUDIO_DEV AoDevId);
MI_S32 MI_AO_Resume(MI_AUDIO_DEV AoDevId);

MI_S32 MI_AO_SetVolume(MI_AUDIO_DEV AoDevId, MI_S16 s16LeftVolume, MI_S16 s16RightVolume,
                       MI_AO_GainFading_e enFading);
MI_S32 MI_AO_GetVolume(MI_AUDIO_DEV AoDevId, MI_S16 *ps16LeftVolume, MI_S16 *ps16RightVolume);
MI_S32 MI_AO_SetMute(MI_AUDIO_DEV AoDevId, MI_BOOL bLeftMute, MI_BOOL bRightMute, MI_AO_GainFading_e enFading);
MI_S32 MI_AO_GetMute(MI_AUDIO_DEV AoDevId, MI_BOOL *pbLefMute, MI_BOOL *pbRightMute);
MI_S32 MI_AO_SetIfVolume(MI_AO_If_e enAoIf, MI_S16 s16LeftVolume, MI_S16 s16RightVolume);
MI_S32 MI_AO_GetIfVolume(MI_AO_If_e enAoIf, MI_S16 *ps16LeftVolume, MI_S16 *ps16RightVolume);
MI_S32 MI_AO_SetIfMute(MI_AO_If_e enAoIf, MI_BOOL bLeftMute, MI_BOOL bRightMute);
MI_S32 MI_AO_GetIfMute(MI_AO_If_e enAoIf, MI_BOOL *pbLeftMute, MI_BOOL *pbRightMute);

MI_S32 MI_AO_SetI2SConfig(MI_AO_If_e enAoI2SIf, const MI_AUDIO_I2sConfig_t *pstConfig);
MI_S32 MI_AO_GetI2SConfig(MI_AO_If_e enAoI2SIf, MI_AUDIO_I2sConfig_t *pstConfig);

// Use in A/V Sync
MI_S32 MI_AO_AdjustSpeed(MI_AUDIO_DEV AoDevId, MI_S32 s32Speed);
MI_S32 MI_AO_GetTimestamp(MI_AUDIO_DEV AoDevId, MI_U32 *pu32Remaining,
                          MI_U64 *pu64TStamp); /* Returns remaining frames in DMA and corresponding timestamp. */
MI_S32 MI_AO_GetLatency(MI_AUDIO_DEV AoDevId, MI_U32 *pu32Latency);

MI_S32 MI_AO_InitDev(MI_AO_InitParam_t *pstInitParam);
MI_S32 MI_AO_DeInitDev(void);
MI_S32 MI_AO_Dup(MI_AUDIO_DEV AoDevId);

#ifdef __cplusplus
}
#endif

#endif ///__MI_AO_H__
