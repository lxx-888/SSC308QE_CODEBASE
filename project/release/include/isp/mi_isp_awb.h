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

#ifndef _MI_ISP_AWB_H_
#define _MI_ISP_AWB_H_

#include "mi_isp_awb_datatype.h"

#ifdef __cplusplus
extern "C"
{
#endif

    /************************************* AWB API START *************************************/
    MI_S32 MI_ISP_AWB_QueryInfo(MI_U32 DevId, MI_U32 Channel, MI_ISP_AWB_QueryInfoType_t *data);
    MI_S32 MI_ISP_AWB_SetAttr(MI_U32 DevId, MI_U32 Channel, MI_ISP_AWB_AttrType_t *data);
    MI_S32 MI_ISP_AWB_GetAttr(MI_U32 DevId, MI_U32 Channel, MI_ISP_AWB_AttrType_t *data);
    MI_S32 MI_ISP_AWB_SetAttrEx(MI_U32 DevId, MI_U32 Channel, MI_ISP_AWB_AttrExType_t *data);
    MI_S32 MI_ISP_AWB_GetAttrEx(MI_U32 DevId, MI_U32 Channel, MI_ISP_AWB_AttrExType_t *data);
    MI_S32 MI_ISP_AWB_SetMultiLsAttr(MI_U32 DevId, MI_U32 Channel, MI_ISP_AWB_MultiLsType_t *data);
    MI_S32 MI_ISP_AWB_GetMultiLsAttr(MI_U32 DevId, MI_U32 Channel, MI_ISP_AWB_MultiLsType_t *data);
    MI_S32 MI_ISP_AWB_SetCtWeight(
        MI_U32 DevId, MI_U32 Channel,
        MI_ISP_AWB_CtWeightType_t *data); // Not works now, please use MI_ISP_AWB_SetAttr to set LvWeight indstead.
    MI_S32 MI_ISP_AWB_GetCtWeight(
        MI_U32 DevId, MI_U32 Channel,
        MI_ISP_AWB_CtWeightType_t *data); // Not works now, please use MI_ISP_AWB_GetAttr to set LvWeight indstead.
    MI_S32 MI_ISP_AWB_SetCtCaliAttr(MI_U32 DevId, MI_U32 Channel, MI_ISP_AWB_CtCaliType_t *data);
    MI_S32 MI_ISP_AWB_GetCtCaliAttr(MI_U32 DevId, MI_U32 Channel, MI_ISP_AWB_CtCaliType_t *data);
    MI_S32 MI_ISP_AWB_GetCurCtCaliAttr(MI_U32 DevId, MI_U32 Channel, MI_ISP_AWB_CurCtCaliType_t *data);
    MI_S32 MI_ISP_AWB_GetCtStats(MI_U32 DevId, MI_U32 Channel, MI_ISP_AWB_CtStatisticsType_t *data);
    MI_S32 MI_ISP_AWB_SetCtMwbAttr(MI_U32 DevId, MI_U32 Channel, MI_ISP_AWB_CtmwbParam_t *data);
    MI_S32 MI_ISP_AWB_GetCtMwbAttr(MI_U32 DevId, MI_U32 Channel, MI_ISP_AWB_CtmwbParam_t *data);
    MI_S32 MI_ISP_AWB_SetDayNightDetection(MI_U32 DevId, MI_U32 Channel, MI_ISP_AWB_DaynightDetectionType_t *data);
    MI_S32 MI_ISP_AWB_GetDayNightDetection(MI_U32 DevId, MI_U32 Channel, MI_ISP_AWB_DaynightDetectionType_t *data);
    MI_S32 MI_ISP_AWB_QueryDayNightInfo(MI_U32 DevId, MI_U32 Channel, MI_ISP_AWB_DaynightInfoType_t *data);
    MI_S32 MI_ISP_AWB_SetStabilizer(MI_U32 DevId, MI_U32 Channel, MI_ISP_AWB_StabilizerType_t *data);
    MI_S32 MI_ISP_AWB_GetStabilizer(MI_U32 DevId, MI_U32 Channel, MI_ISP_AWB_StabilizerType_t *data);
    MI_S32 MI_ISP_AWB_SetSpecialCase(MI_U32 DevId, MI_U32 Channel, MI_ISP_AWB_SpecialCaseType_t *data);
    MI_S32 MI_ISP_AWB_GetSpecialCase(MI_U32 DevId, MI_U32 Channel, MI_ISP_AWB_SpecialCaseType_t *data);
    MI_S32 MI_ISP_AWB_QuerySpecialCaseInfo(MI_U32 DevId, MI_U32 Channel, MI_ISP_AWB_SpecialCaseInfoType_t *data);
    MI_S32 MI_ISP_AWB_SetStatisFilter(MI_U32 DevId, MI_U32 Channel, MI_ISP_AWB_StatisFilterType_t *data);
    MI_S32 MI_ISP_AWB_GetStatisFilter(MI_U32 DevId, MI_U32 Channel, MI_ISP_AWB_StatisFilterType_t *data);
    MI_S32 MI_ISP_AWB_GetVersionInfo(MI_U32 DevId, MI_U32 Channel, MI_ISP_AWB_VerInfoType_t *data);
    MI_S32 MI_ISP_AWB_SetStatisNr(MI_U32 DevId, MI_U32 Channel, MI_ISP_AWB_StatisNrType_t *data);
    MI_S32 MI_ISP_AWB_GetStatisNr(MI_U32 DevId, MI_U32 Channel, MI_ISP_AWB_StatisNrType_t *data);
    MI_S32 MI_ISP_AWB_UpdateAIAwbStatis(MI_U32 DevId, MI_U32 Channel, MI_ISP_AWB_AIAwbStatisType_t *data);
    MI_S32 MI_ISP_AWB_SetAIAwbAdj(MI_U32 DevId, MI_U32 Channel, MI_ISP_AWB_AIAwbAdjType_t *data);
    MI_S32 MI_ISP_AWB_GetAIAwbAdj(MI_U32 DevId, MI_U32 Channel, MI_ISP_AWB_AIAwbAdjType_t *data);
    MI_S32 MI_ISP_AWB_QueryAIAwbInfo(MI_U32 DevId, MI_U32 Channel, MI_ISP_AWB_AIAwbInfoType_t *data);
    MI_S32 MI_ISP_AWB_SetFaceDetectInfo(MI_U32 DevId, MI_U32 Channel, MI_ISP_AWB_FdInfoType_t *data);
    MI_S32 MI_ISP_AWB_SetFDAWBParam(MI_U32 DevId, MI_U32 Channel, MI_ISP_AWB_FdAwbParam_t *data);
    MI_S32 MI_ISP_AWB_GetFDAWBParam(MI_U32 DevId, MI_U32 Channel, MI_ISP_AWB_FdAwbParam_t *data);
    MI_S32 MI_ISP_AWB_GetFDAWBInfo(MI_U32 DevId, MI_U32 Channel, MI_ISP_AWB_FdAwbInfoType_t *data);
    MI_S32 MI_ISP_AWB_SetFewStatisStrategy(MI_U32 DevId, MI_U32 Channel, MI_ISP_AWB_FwstStrategyParam_t *data);
    MI_S32 MI_ISP_AWB_GetFewStatisStrategy(MI_U32 DevId, MI_U32 Channel, MI_ISP_AWB_FwstStrategyParam_t *data);
    MI_S32 MI_ISP_AWB_SetRunPeriod(MI_U32 DevId, MI_U32 Channel, MI_ISP_AWB_RunPeriodParam_t *data);
    MI_S32 MI_ISP_AWB_GetRunPeriod(MI_U32 DevId, MI_U32 Channel, MI_ISP_AWB_RunPeriodParam_t *data);
    MI_S32 MI_ISP_AWB_SetSceneInd(MI_U32 DevId, MI_U32 Channel, MI_ISP_AWB_SceneIndType_t *data);
    MI_S32 MI_ISP_AWB_GetSceneInd(MI_U32 DevId, MI_U32 Channel, MI_ISP_AWB_SceneIndType_t *data);
    MI_S32 MI_ISP_AWB_SetSceneAdj(MI_U32 DevId, MI_U32 Channel, MI_ISP_AWB_SceneAdjType_t *data);
    MI_S32 MI_ISP_AWB_GetSceneAdj(MI_U32 DevId, MI_U32 Channel, MI_ISP_AWB_SceneAdjType_t *data);
    MI_S32 MI_ISP_AWB_QuerySceneCurLevelInfo(MI_U32 DevId, MI_U32 Channel, MI_ISP_AWB_SceneCurLevelType_t *data);
    /************************************* AWB API END   *************************************/

    /************************************* LOAD API START*************************************/
    MI_S32 MI_ISP_AWB_SetAttrCall(MI_U32 DevId, MI_U32 Channel, MI_U8 *param_ary[], MI_U8 param_num);
    MI_S32 MI_ISP_AWB_SetAttrExCall(MI_U32 DevId, MI_U32 Channel, MI_U8 *param_ary[], MI_U8 param_num);
    MI_S32 MI_ISP_AWB_SetMultiLsAttrCall(MI_U32 DevId, MI_U32 Channel, MI_U8 *param_ary[], MI_U8 param_num);
    MI_S32 MI_ISP_AWB_SetCtCaliAttrCall(MI_U32 DevId, MI_U32 Channel, MI_U8 *param_ary[], MI_U8 param_num);
    MI_S32 MI_ISP_AWB_SetCtMwbAttrCall(MI_U32 DevId, MI_U32 Channel, MI_U8 *param_ary[], MI_U8 param_num);
    MI_S32 MI_ISP_AWB_SetStabilizerCall(MI_U32 DevId, MI_U32 Channel, MI_U8 *param_ary[], MI_U8 param_num);
    MI_S32 MI_ISP_AWB_SetSceneAdjCall(MI_U32 DevId, MI_U32 Channel, MI_U8 *param_ary[], MI_U8 param_num);
    /************************************* LOAD API END  *************************************/

#ifdef __cplusplus
} // end of extern C
#endif

#endif //_MI_ISP_AWB_H_
