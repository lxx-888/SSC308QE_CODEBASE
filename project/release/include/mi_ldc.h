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

#ifndef __MI_LDC_H__
#define __MI_LDC_H__

#include "mi_ldc_datatype.h"

#define LDC_MAJOR_VERSION   4
#define LDC_SUB_VERSION     2
#define MACRO_TO_STR(macro) #macro
#define LDC_VERSION_STR(major_version, sub_version)                                                                 \
    (                                                                                                               \
        {                                                                                                           \
            char *tmp =                                                                                             \
                sub_version / 100  ? "mi_ldc_version_" MACRO_TO_STR(major_version) "." MACRO_TO_STR(sub_version)    \
                : sub_version / 10 ? "mi_ldc_version_" MACRO_TO_STR(major_version) ".0" MACRO_TO_STR(sub_version)   \
                                   : "mi_ldc_version_" MACRO_TO_STR(major_version) ".00" MACRO_TO_STR(sub_version); \
            tmp;                                                                                                    \
        })
#define MI_LDC_API_VERSION LDC_VERSION_STR(LDC_MAJOR_VERSION, LDC_SUB_VERSION)

#ifdef __cplusplus
extern "C"
{
#endif

    MI_S32 MI_LDC_CreateDevice(MI_LDC_DEV devId, MI_LDC_DevAttr_t *pstDevAttr);
    MI_S32 MI_LDC_DestroyDevice(MI_LDC_DEV devId);

    MI_S32 MI_LDC_CreateChannel(MI_LDC_DEV devId, MI_LDC_CHN chnId, MI_LDC_ChnAttr_t *pstChnAttr);
    MI_S32 MI_LDC_DestroyChannel(MI_LDC_DEV devId, MI_LDC_CHN chnId);

    MI_S32 MI_LDC_StartChannel(MI_LDC_DEV devId, MI_LDC_CHN chnId);
    MI_S32 MI_LDC_StopChannel(MI_LDC_DEV devId, MI_LDC_CHN chnId);

    MI_S32 MI_LDC_AttachToChn(MI_LDC_DEV devId, MI_LDC_CHN chnId, MI_SYS_ChnPort_t *pstChnPort);
    MI_S32 MI_LDC_DetachFromChn(MI_LDC_DEV devId, MI_LDC_CHN chnId, MI_SYS_ChnPort_t *pstChnPort);

    MI_S32 MI_LDC_GetRegionBorderMappedPointCnt(MI_LDC_DEV devId, MI_LDC_CHN chnId, MI_U32 u32RegionId, MI_U32 *pu32PointCnt);
    MI_S32 MI_LDC_GetRegionBorderMappedPoints(MI_LDC_DEV devId, MI_LDC_CHN chnId, MI_U32 u32RegionId, MI_LDC_Point_t *pstPoints);

    MI_S32 MI_LDC_SetChnLDCAttr(MI_LDC_DEV devId, MI_LDC_CHN chnId, MI_LDC_ChnLDCAttr_t *pstChnLDCAttr);
    MI_S32 MI_LDC_GetChnLDCAttr(MI_LDC_DEV devId, MI_LDC_CHN chnId, MI_LDC_ChnLDCAttr_t *pstChnLDCAttr);

    MI_S32 MI_LDC_SetChnDISAttr(MI_LDC_DEV devId, MI_LDC_CHN chnId, MI_LDC_ChnDISAttr_t *pstChnDISAttr);
    MI_S32 MI_LDC_GetChnDISAttr(MI_LDC_DEV devId, MI_LDC_CHN chnId, MI_LDC_ChnDISAttr_t *pstChnDISAttr);

    MI_S32 MI_LDC_SetChnPMFAttr(MI_LDC_DEV devId, MI_LDC_CHN chnId, MI_LDC_ChnPMFAttr_t *pstChnPMFAttr);
    MI_S32 MI_LDC_GetChnPMFAttr(MI_LDC_DEV devId, MI_LDC_CHN chnId, MI_LDC_ChnPMFAttr_t *pstChnPMFAttr);

    MI_S32 MI_LDC_SetChnLDCHorAttr(MI_LDC_DEV devId, MI_LDC_CHN chnId, MI_LDC_ChnLDCHorAttr_t *pstChnLDCHorAttr);
    MI_S32 MI_LDC_GetChnLDCHorAttr(MI_LDC_DEV devId, MI_LDC_CHN chnId, MI_LDC_ChnLDCHorAttr_t *pstChnLDCHorAttr);

    MI_S32 MI_LDC_SetChnStitchAttr(MI_LDC_DEV devId, MI_LDC_CHN chnId, MI_LDC_ChnStitchAttr_t *pstChnStitchAttr);
    MI_S32 MI_LDC_GetChnStitchAttr(MI_LDC_DEV devId, MI_LDC_CHN chnId, MI_LDC_ChnStitchAttr_t *pstChnStitchAttr);

    MI_S32 MI_LDC_SetChnCustomAttr(MI_LDC_DEV devId, MI_LDC_CHN chnId, MI_LDC_ChnCustomAttr_t *pstChnCustomAttr);
    MI_S32 MI_LDC_GetChnCustomAttr(MI_LDC_DEV devId, MI_LDC_CHN chnId, MI_LDC_ChnCustomAttr_t *pstChnCustomAttr);

    MI_S32 MI_LDC_SetChnDPUAttr(MI_LDC_DEV devId, MI_LDC_CHN chnId, MI_LDC_ChnDPUAttr_t *pstChnDPUAttr);
    MI_S32 MI_LDC_GetChnDPUAttr(MI_LDC_DEV devId, MI_LDC_CHN chnId, MI_LDC_ChnDPUAttr_t *pstChnDPUAttr);

    MI_S32 MI_LDC_SetChnNIRAttr(MI_LDC_DEV devId, MI_LDC_CHN chnId, MI_LDC_ChnNIRAttr_t *pstChnNIRAttr);
    MI_S32 MI_LDC_GetChnNIRAttr(MI_LDC_DEV devId, MI_LDC_CHN chnId, MI_LDC_ChnNIRAttr_t *pstChnNIRAttr);

    MI_S32 MI_LDC_SetInputPortAttr(MI_LDC_DEV devId, MI_LDC_CHN chnId, MI_LDC_InputPortAttr_t *pstInputAttr);
    MI_S32 MI_LDC_GetInputPortAttr(MI_LDC_DEV devId, MI_LDC_CHN chnId, MI_LDC_InputPortAttr_t *pstInputAttr);
    MI_S32 MI_LDC_SetOutputPortAttr(MI_LDC_DEV devId, MI_LDC_CHN chnId, MI_LDC_OutputPortAttr_t *pstOutputAttr);
    MI_S32 MI_LDC_GetOutputPortAttr(MI_LDC_DEV devId, MI_LDC_CHN chnId, MI_LDC_OutputPortAttr_t *pstOutputAttr);

    MI_S32 MI_LDC_DoLutDirectTask(MI_LDC_DEV devId, MI_LDC_CHN chnId, MI_LDC_LutTaskAttr_t *pstAttr);

    MI_S32 MI_LDC_GetDisplacementMapSize(MI_LDC_DEV devId, MI_LDC_CHN chnId, MI_LDC_DispMapConf_t *pstDispMapConf, MI_LDC_DispMapSize_t *pstDispMapSize);
    MI_S32 MI_LDC_GenerateDisplacementMap(MI_LDC_DEV devId, MI_LDC_CHN chnId, MI_LDC_DispMapConf_t *pstDispMapConf, MI_LDC_DispMapInfo_t *pstDispMapInfo);
    MI_S32 MI_LDC_QueryMappingPoint(MI_LDC_DEV devId, MI_LDC_CHN chnId, MI_LDC_DispMapSize_t *pstDispMapSize, MI_LDC_DispMapInfo_t *pstDispMapInfo, MI_LDC_Point_t *pstOriPoint,
                                                     MI_LDC_Point_t *pstMapPoint);

    MI_S32 MI_LDC_CalibIMUBaseDrift(MI_LDC_DEV devId, MI_LDC_CHN chnId, MI_U32 u32IMUPart, MI_U32 u32TimeMs);
    MI_S32 MI_LDC_SetIMUDriftPara(MI_LDC_DEV devId, MI_LDC_CHN chnId, MI_LDC_IMUDrift_t *pstIMUDrift);
    MI_S32 MI_LDC_GetIMUDriftPara(MI_LDC_DEV devId, MI_LDC_CHN chnId, MI_LDC_IMUDrift_t *pstIMUDrift);

#ifdef __cplusplus
}
#endif

#endif ///_MI_LDC_H_
