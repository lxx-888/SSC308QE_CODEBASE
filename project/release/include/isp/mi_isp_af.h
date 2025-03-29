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

#ifndef _MI_ISP_AF_H_
#define _MI_ISP_AF_H_

#include "mi_isp_af_datatype.h"
#include "mi_isp_iq_datatype.h"

#ifdef __cplusplus
extern "C"
{
#endif

    /************************************* AF  API START *************************************/
    MI_S32 MI_ISP_AF_SetAdjustIII(MI_U32 DevId, MI_U32 Channel, MI_ISP_AF_AdjustIIIType_t *data);
    MI_S32 MI_ISP_AF_GetAdjustIII(MI_U32 DevId, MI_U32 Channel, MI_ISP_AF_AdjustIIIType_t *data);
    MI_S32 MI_ISP_AF_SetFDAEInfo(MI_U32 DevId, MI_U32 Channel, MI_ISP_AF_FDAEInfoType_t *data);
    MI_S32 MI_ISP_AF_GetFDAEInfo(MI_U32 DevId, MI_U32 Channel, MI_ISP_AF_FDAEInfoType_t *data);
    MI_S32 MI_ISP_AF_SetAWBInfo(MI_U32 DevId, MI_U32 Channel, MI_ISP_AF_AWBInfoType_t *data);
    MI_S32 MI_ISP_AF_GetAWBInfo(MI_U32 DevId, MI_U32 Channel, MI_ISP_AF_AWBInfoType_t *data);
    MI_S32 MI_ISP_AF_SetBackUpPosition(MI_U32 DevId, MI_U32 Channel, MI_ISP_AF_BackUpPositionType_t *data);
    MI_S32 MI_ISP_AF_GetBackUpPosition(MI_U32 DevId, MI_U32 Channel, MI_ISP_AF_BackUpPositionType_t *data);
    MI_S32 MI_ISP_AF_SetOffset(MI_U32 DevId, MI_U32 Channel, MI_ISP_AF_OffsetType_t *data);
    MI_S32 MI_ISP_AF_GetOffset(MI_U32 DevId, MI_U32 Channel, MI_ISP_AF_OffsetType_t *data);
    MI_S32 MI_ISP_AF_SetAdjustII(MI_U32 DevId, MI_U32 Channel, MI_ISP_AF_AdjustIIType_t *data);
    MI_S32 MI_ISP_AF_GetAdjustII(MI_U32 DevId, MI_U32 Channel, MI_ISP_AF_AdjustIIType_t *data);
    MI_S32 MI_ISP_AF_SetStartVCMPos(MI_U32 DevId, MI_U32 Channel, MI_ISP_AF_StartVCMPosType_t *data);
    MI_S32 MI_ISP_AF_GetStartVCMPos(MI_U32 DevId, MI_U32 Channel, MI_ISP_AF_StartVCMPosType_t *data);
    MI_S32 MI_ISP_AF_SetStatus(MI_U32 DevId, MI_U32 Channel, MI_ISP_AF_StatusType_t *data);
    MI_S32 MI_ISP_AF_GetStatus(MI_U32 DevId, MI_U32 Channel, MI_ISP_AF_StatusType_t *data);
    MI_S32 MI_ISP_AF_GetAFVerInfo(MI_U32 DevId, MI_U32 Channel, MI_ISP_AF_AFVerInfoType_t *data);
    MI_S32 MI_ISP_AF_SetFDAFInfo(MI_U32 DevId, MI_U32 Channel, MI_ISP_AF_FDAFInfoType_t *data);
    MI_S32 MI_ISP_AF_GetFDAFInfo(MI_U32 DevId, MI_U32 Channel, MI_ISP_AF_FDAFInfoType_t *data);
    MI_S32 MI_ISP_AF_SetFDAF(MI_U32 DevId, MI_U32 Channel, MI_ISP_AF_FDAFType_t *data);
    MI_S32 MI_ISP_AF_GetFDAF(MI_U32 DevId, MI_U32 Channel, MI_ISP_AF_FDAFType_t *data);
    MI_S32 MI_ISP_AF_SetDetectFlatZone(MI_U32 DevId, MI_U32 Channel, MI_ISP_AF_DetectFlatZoneType_t *data);
    MI_S32 MI_ISP_AF_GetDetectFlatZone(MI_U32 DevId, MI_U32 Channel, MI_ISP_AF_DetectFlatZoneType_t *data);
    MI_S32 MI_ISP_AF_SetAdjust(MI_U32 DevId, MI_U32 Channel, MI_ISP_AF_AdjustType_t *data);
    MI_S32 MI_ISP_AF_GetAdjust(MI_U32 DevId, MI_U32 Channel, MI_ISP_AF_AdjustType_t *data);
    MI_S32 MI_ISP_AF_QueryInfo(MI_U32 DevId, MI_U32 Channel, MI_ISP_AF_QueryInfoType_t *data);
    MI_S32 MI_ISP_AF_SetHwWin(MI_U32 DevId, MI_U32 Channel, MI_ISP_AF_HwWinType_t *data);
    MI_S32 MI_ISP_AF_GetHwWin(MI_U32 DevId, MI_U32 Channel, MI_ISP_AF_HwWinType_t *data);
    MI_S32 MI_ISP_AF_SetHwFilterAttr(MI_U32 DevId, MI_U32 Channel, MI_ISP_AF_HwFilterAttrType_t *data);
    MI_S32 MI_ISP_AF_GetHwFilterAttr(MI_U32 DevId, MI_U32 Channel, MI_ISP_AF_HwFilterAttrType_t *data);
    MI_S32 MI_ISP_AF_SetHwFilterSq(MI_U32 DevId, MI_U32 Channel, MI_ISP_AF_HwFilterSqType_t *data);
    MI_S32 MI_ISP_AF_GetHwFilterSq(MI_U32 DevId, MI_U32 Channel, MI_ISP_AF_HwFilterSqType_t *data);
    MI_S32 MI_ISP_AF_SetHwBnr(MI_U32 DevId, MI_U32 Channel, MI_ISP_AF_HwBnrType_t *data);
    MI_S32 MI_ISP_AF_GetHwBnr(MI_U32 DevId, MI_U32 Channel, MI_ISP_AF_HwBnrType_t *data);
    MI_S32 MI_ISP_AF_SetHwYParam(MI_U32 DevId, MI_U32 Channel, MI_ISP_AF_HwYParamType_t *data);
    MI_S32 MI_ISP_AF_GetHwYParam(MI_U32 DevId, MI_U32 Channel, MI_ISP_AF_HwYParamType_t *data);
    MI_S32 MI_ISP_AF_SetHwSource(MI_U32 DevId, MI_U32 Channel, MI_ISP_AF_HwSourceType_e *data);
    MI_S32 MI_ISP_AF_GetHwSource(MI_U32 DevId, MI_U32 Channel, MI_ISP_AF_HwSourceType_e *data);
    MI_S32 MI_ISP_AF_SetHwPreFilter(MI_U32 DevId, MI_U32 Channel, MI_ISP_AF_HwPrefilterType_t *data);
    MI_S32 MI_ISP_AF_GetHwPreFilter(MI_U32 DevId, MI_U32 Channel, MI_ISP_AF_HwPrefilterType_t *data);
    MI_S32 MI_ISP_AF_SetHwYMap(MI_U32 DevId, MI_U32 Channel, MI_ISP_AF_HwYMapType_t *data);
    MI_S32 MI_ISP_AF_GetHwYMap(MI_U32 DevId, MI_U32 Channel, MI_ISP_AF_HwYMapType_t *data);
    MI_S32 MI_ISP_AF_SetHwLdg(MI_U32 DevId, MI_U32 Channel, MI_ISP_AF_HwLdgType_t *data);
    MI_S32 MI_ISP_AF_GetHwLdg(MI_U32 DevId, MI_U32 Channel, MI_ISP_AF_HwLdgType_t *data);
    MI_S32 MI_ISP_AF_SetHwPeakMode(MI_U32 DevId, MI_U32 Channel, MI_ISP_AF_HwPeakModeType_t *data);
    MI_S32 MI_ISP_AF_GetHwPeakMode(MI_U32 DevId, MI_U32 Channel, MI_ISP_AF_HwPeakModeType_t *data);
    MI_S32 MI_ISP_AF_SetAttr(MI_U32 DevId, MI_U32 Channel, MI_ISP_AF_AttrType_t *data);
    MI_S32 MI_ISP_AF_GetAttr(MI_U32 DevId, MI_U32 Channel, MI_ISP_AF_AttrType_t *data);
    MI_S32 MI_ISP_AF_SetMotor(MI_U32 DevId, MI_U32 Channel, MI_ISP_AF_MotorType_t *data);
    MI_S32 MI_ISP_AF_GetMotor(MI_U32 DevId, MI_U32 Channel, MI_ISP_AF_MotorType_t *data);
    MI_S32 MI_ISP_AF_SetAccWeight(MI_U32 DevId, MI_U32 Channel, MI_ISP_AF_AccWeightType_t *data);
    MI_S32 MI_ISP_AF_GetAccWeight(MI_U32 DevId, MI_U32 Channel, MI_ISP_AF_AccWeightType_t *data);
    MI_S32 MI_ISP_AF_SetOneShot(MI_U32 DevId, MI_U32 Channel, MI_ISP_AF_OneShotType_t *data);
    MI_S32 MI_ISP_AF_GetOneShot(MI_U32 DevId, MI_U32 Channel, MI_ISP_AF_OneShotType_t *data);
    MI_S32 MI_ISP_AF_SetSceneChange(MI_U32 DevId, MI_U32 Channel, MI_ISP_AF_SceneChangeType_t *data);
    MI_S32 MI_ISP_AF_GetSceneChange(MI_U32 DevId, MI_U32 Channel, MI_ISP_AF_SceneChangeType_t *data);
    MI_S32 MI_ISP_AF_SetSearchStart(MI_U32 DevId, MI_U32 Channel, MI_ISP_AF_SearchStartType_t *data);
    MI_S32 MI_ISP_AF_GetSearchStart(MI_U32 DevId, MI_U32 Channel, MI_ISP_AF_SearchStartType_t *data);
    MI_S32 MI_ISP_AF_SetSearch(MI_U32 DevId, MI_U32 Channel, MI_ISP_AF_SearchType_t *data);
    MI_S32 MI_ISP_AF_GetSearch(MI_U32 DevId, MI_U32 Channel, MI_ISP_AF_SearchType_t *data);
    MI_S32 MI_ISP_AF_GetVersionInfo(MI_U32 DevId, MI_U32 Channel, MI_ISP_AF_VerInfoType_t *data);
    /************************************* AF  API END   *************************************/

    /************************************* LOAD API START*************************************/
    MI_S32 MI_ISP_AF_SetAdjustIIICall(MI_U32 DevId, MI_U32 Channel, MI_U8 *param_ary[], MI_U8 param_num);
    MI_S32 MI_ISP_AF_SetFDAEInfoCall(MI_U32 DevId, MI_U32 Channel, MI_U8 *param_ary[], MI_U8 param_num);
    MI_S32 MI_ISP_AF_SetAWBInfoCall(MI_U32 DevId, MI_U32 Channel, MI_U8 *param_ary[], MI_U8 param_num);
    MI_S32 MI_ISP_AF_SetBackUpPositionCall(MI_U32 DevId, MI_U32 Channel, MI_U8 *param_ary[], MI_U8 param_num);
    MI_S32 MI_ISP_AF_SetOffsetCall(MI_U32 DevId, MI_U32 Channel, MI_U8 *param_ary[], MI_U8 param_num);
    MI_S32 MI_ISP_AF_SetAdjustIICall(MI_U32 DevId, MI_U32 Channel, MI_U8 *param_ary[], MI_U8 param_num);
    MI_S32 MI_ISP_AF_SetStartVCMPosCall(MI_U32 DevId, MI_U32 Channel, MI_U8 *param_ary[], MI_U8 param_num);
    MI_S32 MI_ISP_AF_SetFDAFCall(MI_U32 DevId, MI_U32 Channel, MI_U8 *param_ary[], MI_U8 param_num);
    MI_S32 MI_ISP_AF_SetDetectFlatZoneCall(MI_U32 DevId, MI_U32 Channel, MI_U8 *param_ary[], MI_U8 param_num);
    MI_S32 MI_ISP_AF_SetAdjustCall(MI_U32 DevId, MI_U32 Channel, MI_U8 *param_ary[], MI_U8 param_num);
    MI_S32 MI_ISP_AF_SetHwWinCall(MI_U32 DevId, MI_U32 Channel, MI_U8 *param_ary[], MI_U8 param_num);
    MI_S32 MI_ISP_AF_SetHwFilterAttrCall(MI_U32 DevId, MI_U32 Channel, MI_U8 *param_ary[], MI_U8 param_num);
    MI_S32 MI_ISP_AF_SetHwFilterSqCall(MI_U32 DevId, MI_U32 Channel, MI_U8 *param_ary[], MI_U8 param_num);
    MI_S32 MI_ISP_AF_SetHwBnrCall(MI_U32 DevId, MI_U32 Channel, MI_U8 *param_ary[], MI_U8 param_num);
    MI_S32 MI_ISP_AF_SetHwYParamCall(MI_U32 DevId, MI_U32 Channel, MI_U8 *param_ary[], MI_U8 param_num);
    MI_S32 MI_ISP_AF_SetHwSourceCall(MI_U32 DevId, MI_U32 Channel, MI_U8 *param_ary[], MI_U8 param_num);
    MI_S32 MI_ISP_AF_SetHwPreFilterCall(MI_U32 DevId, MI_U32 Channel, MI_U8 *param_ary[], MI_U8 param_num);
    MI_S32 MI_ISP_AF_SetHwYMapCall(MI_U32 DevId, MI_U32 Channel, MI_U8 *param_ary[], MI_U8 param_num);
    MI_S32 MI_ISP_AF_SetHwLdgCall(MI_U32 DevId, MI_U32 Channel, MI_U8 *param_ary[], MI_U8 param_num);
    MI_S32 MI_ISP_AF_SetHwPeakModeCall(MI_U32 DevId, MI_U32 Channel, MI_U8 *param_ary[], MI_U8 param_num);
    MI_S32 MI_ISP_AF_SetAttrCall(MI_U32 DevId, MI_U32 Channel, MI_U8 *param_ary[], MI_U8 param_num);
    MI_S32 MI_ISP_AF_SetMotorCall(MI_U32 DevId, MI_U32 Channel, MI_U8 *param_ary[], MI_U8 param_num);
    MI_S32 MI_ISP_AF_SetAccWeightCall(MI_U32 DevId, MI_U32 Channel, MI_U8 *param_ary[], MI_U8 param_num);
    MI_S32 MI_ISP_AF_SetOneShotCall(MI_U32 DevId, MI_U32 Channel, MI_U8 *param_ary[], MI_U8 param_num);
    MI_S32 MI_ISP_AF_SetSceneChangeCall(MI_U32 DevId, MI_U32 Channel, MI_U8 *param_ary[], MI_U8 param_num);
    MI_S32 MI_ISP_AF_SetSearchStartCall(MI_U32 DevId, MI_U32 Channel, MI_U8 *param_ary[], MI_U8 param_num);
    MI_S32 MI_ISP_AF_SetSearchCall(MI_U32 DevId, MI_U32 Channel, MI_U8 *param_ary[], MI_U8 param_num);
    /************************************* LOAD API END  *************************************/

#ifdef __cplusplus
} // end of extern C
#endif

#endif //_MI_ISP_AF_H_
