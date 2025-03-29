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
#ifndef __MI_AI_H__
#define __MI_AI_H__

#include "mi_aio_datatype.h"
#include "mi_ai_datatype.h"

#define AI_MAJOR_VERSION    3
#define AI_SUB_VERSION      55
#define MACRO_TO_STR(macro) #macro
#define AI_VERSION_STR(major_version, sub_version)                                                                     \
    (                                                                                                                  \
        {                                                                                                              \
            char *tmp = sub_version / 100 ? "mi_ai_version_" MACRO_TO_STR(major_version) "." MACRO_TO_STR(sub_version) \
                        : sub_version / 10                                                                             \
                            ? "mi_ai_version_" MACRO_TO_STR(major_version) ".0" MACRO_TO_STR(sub_version)              \
                            : "mi_ai_version_" MACRO_TO_STR(major_version) ".00" MACRO_TO_STR(sub_version);            \
            tmp;                                                                                                       \
        })
#define MI_AI_API_VERSION AI_VERSION_STR(AI_MAJOR_VERSION, AI_SUB_VERSION)

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
MI_S32 MI_AI_Open(MI_AUDIO_DEV AiDevId, const MI_AI_Attr_t *pstAttr);
MI_S32 MI_AI_GetAttr(MI_AUDIO_DEV AiDevId, MI_AI_Attr_t *pstAttr);
MI_S32 MI_AI_OpenWithCfgFile(MI_AUDIO_DEV AiDevId, const char *pCfgPath);
MI_S32 MI_AI_Close(MI_AUDIO_DEV AiDevId);

MI_S32 MI_AI_AttachIf(MI_AUDIO_DEV AiDevId, const MI_AI_If_e aenAiIfs[],
                      MI_U8 u8AiIfSize); /* u8AiIfSize < MI_AI_MAX_CHN_NUM/2 */

MI_S32 MI_AI_EnableChnGroup(MI_AUDIO_DEV AiDevId, MI_U8 u8ChnGrpIdx);
MI_S32 MI_AI_DisableChnGroup(MI_AUDIO_DEV AiDevId, MI_U8 u8ChnGrpIdx);

MI_S32 MI_AI_Read(MI_AUDIO_DEV AiDevId, MI_U8 u8ChnGrpIdx, MI_AI_Data_t *pstData, MI_AI_Data_t *pstEchoRefData,
                  MI_S32 s32TimeoutMs);
MI_S32 MI_AI_ReleaseData(MI_AUDIO_DEV AiDevId, MI_U8 u8ChnGrpIdx, MI_AI_Data_t *pstData,
                         MI_AI_Data_t *pstEchoRefData);

MI_S32 MI_AI_SetGain(MI_AUDIO_DEV AiDevId, MI_U8 u8ChnGrpIdx, const MI_S16 as16Gains[],
                     MI_U8 u8GainSize); /* u8GainSize == channel count in channel group */
MI_S32 MI_AI_GetGain(MI_AUDIO_DEV AiDevId, MI_U8 u8ChnGrpIdx, MI_S16 as16Gains[], MI_U8 *pu8GainSize);
MI_S32 MI_AI_SetMute(MI_AUDIO_DEV AiDevId, MI_U8 u8ChnGrpIdx, const MI_BOOL abMutes[], MI_U8 u8MuteSize);
MI_S32 MI_AI_GetMute(MI_AUDIO_DEV AiDevId, MI_U8 u8ChnGrpIdx, MI_BOOL abMutes[], MI_U8 *pu8MuteSize);
MI_S32 MI_AI_SetIfGain(MI_AI_If_e enAiIf, MI_S16 s16LeftIfGain,
                       MI_S16 s16RightIfGain); /* 0 ~ x depend on SOC */
MI_S32 MI_AI_GetIfGain(MI_AI_If_e enAiIf, MI_S16 *ps16LeftIfGain, MI_S16 *ps16RightIfGain);
MI_S32 MI_AI_SetIfMute(MI_AI_If_e enAiIf, MI_BOOL bLeftMute, MI_BOOL bRightMute);
MI_S32 MI_AI_GetIfMute(MI_AI_If_e enAiIf, MI_BOOL *pbLeftMute, MI_BOOL *pbRightMute);

MI_S32 MI_AI_SetI2SConfig(MI_AI_If_e enAiI2SIf, const MI_AUDIO_I2sConfig_t *pstConfig);
MI_S32 MI_AI_GetI2SConfig(MI_AI_If_e enAiI2SIf, MI_AUDIO_I2sConfig_t *pstConfig);

MI_S32 MI_AI_DupChnGroup(MI_AUDIO_DEV AiDevId, MI_U8 u8ChnGrpIdx);

MI_S32 MI_AI_InitDev(MI_AI_InitParam_t *pstInitParam);
MI_S32 MI_AI_DeInitDev(void);

#ifdef __cplusplus
}
#endif

#endif ///__MI_AI_H__
