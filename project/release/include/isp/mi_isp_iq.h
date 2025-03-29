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

#ifndef _MI_ISP_IQ_H_
#define _MI_ISP_IQ_H_

#include "mi_isp_iq_datatype.h"

#ifdef __cplusplus
extern "C"
{
#endif

#if 1
    /************************************* IQ  API START *************************************/
    MI_S32 MI_ISP_IQ_GetVersionInfo(MI_U32 DevId, MI_U32 Channel, MI_ISP_IQ_VersionInfoType_t *data);
    MI_S32 MI_ISP_IQ_SetVersionInfo(MI_U32 DevId, MI_U32 Channel, MI_ISP_IQ_VersionInfoType_t *data);
    MI_S32 MI_ISP_IQ_SetGroupHold(MI_U32 DevId, MI_U32 Channel, MI_ISP_IQ_Bool_e *data);
    MI_S32 MI_ISP_IQ_GetParaInitStatus(MI_U32 DevId, MI_U32 Channel, MI_ISP_IQ_ParamInitInfoType_t *data);
    MI_S32 MI_ISP_IQ_SetFastMode(MI_U32 DevId, MI_U32 Channel, MI_ISP_IQ_FastModeType_t *data); //[not support chip] macaron, pudding, ispahan, tiramisu, ikayaki, muffin, maruko
    MI_S32 MI_ISP_IQ_GetFastMode(MI_U32 DevId, MI_U32 Channel, MI_ISP_IQ_FastModeType_t *data); //[not support chip] macaron, pudding, ispahan, tiramisu, ikayaki, muffin, maruko
    MI_S32 MI_ISP_IQ_SetColorToGray(MI_U32 DevId, MI_U32 Channel, MI_ISP_IQ_ColorToGrayType_t *data);
    MI_S32 MI_ISP_IQ_GetColorToGray(MI_U32 DevId, MI_U32 Channel, MI_ISP_IQ_ColorToGrayType_t *data);
    MI_S32 MI_ISP_IQ_SetContrast(MI_U32 DevId, MI_U32 Channel, MI_ISP_IQ_ContrastType_t *data);
    MI_S32 MI_ISP_IQ_GetContrast(MI_U32 DevId, MI_U32 Channel, MI_ISP_IQ_ContrastType_t *data);
    MI_S32 MI_ISP_IQ_SetBrightness(MI_U32 DevId, MI_U32 Channel, MI_ISP_IQ_BrightnessType_t *data);
    MI_S32 MI_ISP_IQ_GetBrightness(MI_U32 DevId, MI_U32 Channel, MI_ISP_IQ_BrightnessType_t *data);
    MI_S32 MI_ISP_IQ_SetLightness(MI_U32 DevId, MI_U32 Channel, MI_ISP_IQ_LightnessType_t *data);
    MI_S32 MI_ISP_IQ_GetLightness(MI_U32 DevId, MI_U32 Channel, MI_ISP_IQ_LightnessType_t *data);
    MI_S32 MI_ISP_IQ_SetRgbGamma(MI_U32 DevId, MI_U32 Channel, MI_ISP_IQ_RgbGammaType_t *data);
    MI_S32 MI_ISP_IQ_GetRgbGamma(MI_U32 DevId, MI_U32 Channel, MI_ISP_IQ_RgbGammaType_t *data);
    MI_S32 MI_ISP_IQ_SetYuvGamma(MI_U32 DevId, MI_U32 Channel, MI_ISP_IQ_YuvGammaType_t *data);
    MI_S32 MI_ISP_IQ_GetYuvGamma(MI_U32 DevId, MI_U32 Channel, MI_ISP_IQ_YuvGammaType_t *data);
    MI_S32 MI_ISP_IQ_SetSaturation(MI_U32 DevId, MI_U32 Channel, MI_ISP_IQ_SaturationType_t *data);
    MI_S32 MI_ISP_IQ_GetSaturation(MI_U32 DevId, MI_U32 Channel, MI_ISP_IQ_SaturationType_t *data);
    MI_S32 MI_ISP_IQ_SetHue(MI_U32 DevId, MI_U32 Channel, MI_ISP_IQ_HueType_t *data);
    MI_S32 MI_ISP_IQ_GetHue(MI_U32 DevId, MI_U32 Channel, MI_ISP_IQ_HueType_t *data);
    MI_S32 MI_ISP_IQ_SetDefog(MI_U32 DevId, MI_U32 Channel, MI_ISP_IQ_DefogType_t *data);
    MI_S32 MI_ISP_IQ_GetDefog(MI_U32 DevId, MI_U32 Channel, MI_ISP_IQ_DefogType_t *data);
    MI_S32 MI_ISP_IQ_SetRgbMatrix(MI_U32 DevId, MI_U32 Channel, MI_ISP_IQ_RgbMatrixType_t *data);
    MI_S32 MI_ISP_IQ_GetRgbMatrix(MI_U32 DevId, MI_U32 Channel, MI_ISP_IQ_RgbMatrixType_t *data);
    MI_S32 MI_ISP_IQ_SetFalseColor(MI_U32 DevId, MI_U32 Channel, MI_ISP_IQ_FalseColorType_t *data);
    MI_S32 MI_ISP_IQ_GetFalseColor(MI_U32 DevId, MI_U32 Channel, MI_ISP_IQ_FalseColorType_t *data);
    MI_S32 MI_ISP_IQ_SetNr3d(MI_U32 DevId, MI_U32 Channel, MI_ISP_IQ_Nr3dType_t *data);
    MI_S32 MI_ISP_IQ_GetNr3d(MI_U32 DevId, MI_U32 Channel, MI_ISP_IQ_Nr3dType_t *data);
    MI_S32
    MI_ISP_IQ_SetNr3dP1(MI_U32 DevId, MI_U32 Channel,
                        MI_ISP_IQ_Nr3dP1Type_t *data); //[not support chip] macaron, pudding, ispahan, tiramisu, ikayaki, muffin, maruko
    MI_S32
    MI_ISP_IQ_GetNr3dP1(MI_U32 DevId, MI_U32 Channel,
                        MI_ISP_IQ_Nr3dP1Type_t *data); //[not support chip] macaron, pudding, ispahan, tiramisu, ikayaki, muffin, maruko
    MI_S32 MI_ISP_IQ_SetNrDeSpike(MI_U32 DevId, MI_U32 Channel, MI_ISP_IQ_NrDespikeType_t *data);
    MI_S32 MI_ISP_IQ_GetNrDeSpike(MI_U32 DevId, MI_U32 Channel, MI_ISP_IQ_NrDespikeType_t *data);
    MI_S32 MI_ISP_IQ_SetNrLuma(MI_U32 DevId, MI_U32 Channel, MI_ISP_IQ_NrLumaType_t *data);
    MI_S32 MI_ISP_IQ_GetNrLuma(MI_U32 DevId, MI_U32 Channel, MI_ISP_IQ_NrLumaType_t *data);
    MI_S32 MI_ISP_IQ_SetNrChroma(MI_U32 DevId, MI_U32 Channel, MI_ISP_IQ_NrChromaType_t *data);
    MI_S32 MI_ISP_IQ_GetNrChroma(MI_U32 DevId, MI_U32 Channel, MI_ISP_IQ_NrChromaType_t *data);
    MI_S32 MI_ISP_IQ_SetSharpness(MI_U32 DevId, MI_U32 Channel, MI_ISP_IQ_SharpnessType_t *data);
    MI_S32 MI_ISP_IQ_GetSharpness(MI_U32 DevId, MI_U32 Channel, MI_ISP_IQ_SharpnessType_t *data);
    MI_S32 MI_ISP_IQ_SetCrossTalk(MI_U32 DevId, MI_U32 Channel, MI_ISP_IQ_CrosstalkType_t *data);
    MI_S32 MI_ISP_IQ_GetCrossTalk(MI_U32 DevId, MI_U32 Channel, MI_ISP_IQ_CrosstalkType_t *data);
    MI_S32 MI_ISP_IQ_SetDobc(MI_U32 DevId, MI_U32 Channel, MI_ISP_IQ_DobcType_t *data);
    MI_S32 MI_ISP_IQ_GetDobc(MI_U32 DevId, MI_U32 Channel, MI_ISP_IQ_DobcType_t *data);
    MI_S32 MI_ISP_IQ_SetObc(MI_U32 DevId, MI_U32 Channel, MI_ISP_IQ_ObcType_t *data);
    MI_S32 MI_ISP_IQ_GetObc(MI_U32 DevId, MI_U32 Channel, MI_ISP_IQ_ObcType_t *data);
    MI_S32 MI_ISP_IQ_SetObcP1(MI_U32 DevId, MI_U32 Channel, MI_ISP_IQ_ObcType_t *data); //[not support chip] ikayaki
    MI_S32 MI_ISP_IQ_GetObcP1(MI_U32 DevId, MI_U32 Channel, MI_ISP_IQ_ObcType_t *data); //[not support chip] ikayaki
    MI_S32 MI_ISP_IQ_SetObcP2(MI_U32 DevId, MI_U32 Channel, MI_ISP_IQ_ObcType_t *data); //[not support chip] ikayaki
    MI_S32 MI_ISP_IQ_GetObcP2(MI_U32 DevId, MI_U32 Channel, MI_ISP_IQ_ObcType_t *data); //[not support chip] ikayaki
    MI_S32 MI_ISP_IQ_SetWdr(MI_U32 DevId, MI_U32 Channel, MI_ISP_IQ_WdrType_t *data);
    MI_S32 MI_ISP_IQ_GetWdr(MI_U32 DevId, MI_U32 Channel, MI_ISP_IQ_WdrType_t *data);
    MI_S32 MI_ISP_IQ_SetWdrLce(MI_U32 DevId, MI_U32 Channel, MI_ISP_IQ_WdrLceType_t *data);//[not support chip] maruko
    MI_S32 MI_ISP_IQ_GetWdrLce(MI_U32 DevId, MI_U32 Channel, MI_ISP_IQ_WdrLceType_t *data);//[not support chip] maruko
    MI_S32 MI_ISP_IQ_SetWdrLtm(MI_U32 DevId, MI_U32 Channel, MI_ISP_IQ_WdrLtmType_t *data);
    MI_S32 MI_ISP_IQ_GetWdrLtm(MI_U32 DevId, MI_U32 Channel, MI_ISP_IQ_WdrLtmType_t *data);
    MI_S32 MI_ISP_IQ_SetWdrNr(MI_U32 DevId, MI_U32 Channel, MI_ISP_IQ_WdrNrType_t *data);
    MI_S32 MI_ISP_IQ_GetWdrNr(MI_U32 DevId, MI_U32 Channel, MI_ISP_IQ_WdrNrType_t *data);
    MI_S32 MI_ISP_IQ_SetWdrCurveAdv(MI_U32 DevId, MI_U32 Channel, MI_ISP_IQ_WdrCurveAdvType_t *data); //[not support chip] pudding, ispahan, tiramisu, ikayaki, muffin, maruko
    MI_S32 MI_ISP_IQ_GetWdrCurveAdv(MI_U32 DevId, MI_U32 Channel, MI_ISP_IQ_WdrCurveAdvType_t *data); //[not support chip] pudding, ispahan, tiramisu, ikayaki, muffin, maruko
    MI_S32 MI_ISP_IQ_SetWdrCurveFull(MI_U32 DevId, MI_U32 Channel,
                                     MI_ISP_IQ_WdrCurveFullType_t *data); //[not support chip] tiramisu
    MI_S32 MI_ISP_IQ_GetWdrCurveFull(MI_U32 DevId, MI_U32 Channel,
                                     MI_ISP_IQ_WdrCurveFullType_t *data); //[not support chip] tiramisu
    MI_S32 MI_ISP_IQ_SetDynamicDp(MI_U32 DevId, MI_U32 Channel, MI_ISP_IQ_DynamicDpType_t *data);
    MI_S32 MI_ISP_IQ_GetDynamicDp(MI_U32 DevId, MI_U32 Channel, MI_ISP_IQ_DynamicDpType_t *data);
    MI_S32 MI_ISP_IQ_SetDynamicDpCluster(MI_U32 DevId, MI_U32 Channel, MI_ISP_IQ_DynamicDpClusterType_t *data);
    MI_S32 MI_ISP_IQ_GetDynamicDpCluster(MI_U32 DevId, MI_U32 Channel, MI_ISP_IQ_DynamicDpClusterType_t *data);
    MI_S32 MI_ISP_IQ_SetHsv(MI_U32 DevId, MI_U32 Channel, MI_ISP_IQ_HsvType_t *data);
    MI_S32 MI_ISP_IQ_GetHsv(MI_U32 DevId, MI_U32 Channel, MI_ISP_IQ_HsvType_t *data);
    MI_S32 MI_ISP_IQ_SetRgbir(MI_U32 DevId, MI_U32 Channel, MI_ISP_IQ_RgbirType_t *data);
    MI_S32 MI_ISP_IQ_GetRgbir(MI_U32 DevId, MI_U32 Channel, MI_ISP_IQ_RgbirType_t *data);
    MI_S32 MI_ISP_IQ_SetThermalIR(MI_U32 DevId, MI_U32 Channel, MI_ISP_IQ_ThermalIRType_t *data);
    MI_S32 MI_ISP_IQ_GetThermalIR(MI_U32 DevId, MI_U32 Channel, MI_ISP_IQ_ThermalIRType_t *data);
    MI_S32
    MI_ISP_IQ_SetFpn(MI_U32 DevId, MI_U32 Channel,
                     MI_ISP_IQ_FpnType_t *data); //[not support chip] macaron, pudding, ispahan, tiramisu, ikayaki, muffin
    MI_S32
    MI_ISP_IQ_GetFpn(MI_U32 DevId, MI_U32 Channel,
                     MI_ISP_IQ_FpnType_t *data); //[not support chip] macaron, pudding, ispahan, tiramisu, ikayaki, muffin
    MI_S32 MI_ISP_IQ_SetDarkFrame(MI_U32 DevId, MI_U32 Channel, MI_ISP_IQ_DarkFrameType_t *data);
    MI_S32 MI_ISP_IQ_GetDarkFrame(MI_U32 DevId, MI_U32 Channel, MI_ISP_IQ_DarkFrameType_t *data);
    MI_S32 MI_ISP_IQ_SetPfc(MI_U32 DevId, MI_U32 Channel, MI_ISP_IQ_PfcType_t *data); //[not support chip] macaron
    MI_S32 MI_ISP_IQ_GetPfc(MI_U32 DevId, MI_U32 Channel, MI_ISP_IQ_PfcType_t *data); //[not support chip] macaron
    MI_S32 MI_ISP_IQ_SetDemosaic(MI_U32 DevId, MI_U32 Channel, MI_ISP_IQ_DemosaicType_t *data);
    MI_S32 MI_ISP_IQ_GetDemosaic(MI_U32 DevId, MI_U32 Channel, MI_ISP_IQ_DemosaicType_t *data);
    MI_S32 MI_ISP_IQ_SetR2Y(MI_U32 DevId, MI_U32 Channel, MI_ISP_IQ_R2YType_t *data);
    MI_S32 MI_ISP_IQ_GetR2Y(MI_U32 DevId, MI_U32 Channel, MI_ISP_IQ_R2YType_t *data);
    MI_S32 MI_ISP_IQ_SetColorTrans(MI_U32 DevId, MI_U32 Channel, MI_ISP_IQ_ColorTransType_t *data);
    MI_S32 MI_ISP_IQ_GetColorTrans(MI_U32 DevId, MI_U32 Channel, MI_ISP_IQ_ColorTransType_t *data);
    MI_S32 MI_ISP_IQ_SetColorTrans_EX(MI_U32 DevId, MI_U32 Channel, MI_ISP_IQ_CTExType_t *data);
    MI_S32 MI_ISP_IQ_GetColorTrans_EX(MI_U32 DevId, MI_U32 Channel, MI_ISP_IQ_CTExType_t *data);
    MI_S32 MI_ISP_IQ_SetHdr(MI_U32 DevId, MI_U32 Channel, MI_ISP_IQ_HdrType_t *data); //[not support chip] ikayaki
    MI_S32 MI_ISP_IQ_GetHdr(MI_U32 DevId, MI_U32 Channel, MI_ISP_IQ_HdrType_t *data); //[not support chip] ikayaki
    MI_S32 MI_ISP_IQ_SetEffect(MI_U32 DevId, MI_U32 Channel, MI_ISP_IQ_EffectType_t *data); //[not support chip] macaron, pudding, ispahan, tiramisu, ikayaki, muffin, maruko
    MI_S32 MI_ISP_IQ_GetEffect(MI_U32 DevId, MI_U32 Channel, MI_ISP_IQ_EffectType_t *data); //[not support chip] macaron, pudding, ispahan, tiramisu, ikayaki, muffin, maruko
    MI_S32 MI_ISP_IQ_SetEnSysMcnrMemory(MI_U32 DevId, MI_U32 Channel, MI_ISP_IQ_SysMcnrMemoryType_t *data); //[not support chip] macaron, pudding, ispahan, tiramisu, ikayaki, muffin, maruko
    MI_S32 MI_ISP_IQ_GetEnSysMcnrMemory(MI_U32 DevId, MI_U32 Channel, MI_ISP_IQ_SysMcnrMemoryType_t *data); //[not support chip] macaron, pudding, ispahan, tiramisu, ikayaki, muffin, maruko
    MI_S32 MI_ISP_IQ_SetLsc(MI_U32 DevId, MI_U32 Channel, MI_ISP_IQ_LscType_t *data);
    MI_S32 MI_ISP_IQ_GetLsc(MI_U32 DevId, MI_U32 Channel, MI_ISP_IQ_LscType_t *data);
    MI_S32 MI_ISP_IQ_SetLscCtrl(MI_U32 DevId, MI_U32 Channel, MI_ISP_IQ_LscCtrlType_t *data);
    MI_S32 MI_ISP_IQ_GetLscCtrl(MI_U32 DevId, MI_U32 Channel, MI_ISP_IQ_LscCtrlType_t *data);
    MI_S32 MI_ISP_IQ_SetAlsc(MI_U32 DevId, MI_U32 Channel, MI_ISP_IQ_AlscType_t *data);
    MI_S32 MI_ISP_IQ_GetAlsc(MI_U32 DevId, MI_U32 Channel, MI_ISP_IQ_AlscType_t *data);
    MI_S32 MI_ISP_IQ_SetAlscCtrl(MI_U32 DevId, MI_U32 Channel, MI_ISP_IQ_AlscCtrlType_t *data);
    MI_S32 MI_ISP_IQ_GetAlscCtrl(MI_U32 DevId, MI_U32 Channel, MI_ISP_IQ_AlscCtrlType_t *data);
    MI_S32 MI_ISP_IQ_SetDarkShading(MI_U32 DevId, MI_U32 Channel, MI_ISP_IQ_DarkShadingType_t *data);
    MI_S32 MI_ISP_IQ_GetDarkShading(MI_U32 DevId, MI_U32 Channel, MI_ISP_IQ_DarkShadingType_t *data);
    MI_S32 MI_ISP_IQ_SetNrLumaAdv(MI_U32 DevId, MI_U32 Channel,
                                  MI_ISP_IQ_NrLumaAdvType_t *data); //[not support chip] macaron
    MI_S32 MI_ISP_IQ_GetNrLumaAdv(MI_U32 DevId, MI_U32 Channel,
                                  MI_ISP_IQ_NrLumaAdvType_t *data); //[not support chip] macaron
    MI_S32 MI_ISP_IQ_SetNrChromaAdv(MI_U32 DevId, MI_U32 Channel,
                                    MI_ISP_IQ_NrChromaAdvType_t *data); //[not support chip] macaron
    MI_S32 MI_ISP_IQ_GetNrChromaAdv(MI_U32 DevId, MI_U32 Channel,
                                    MI_ISP_IQ_NrChromaAdvType_t *data); //[not support chip] macaron
    MI_S32
    MI_ISP_IQ_SetNrChromaPre(MI_U32 DevId, MI_U32 Channel,
                             MI_ISP_IQ_NrChromaPreType_t *data); //[not support chip] macaron, pudding, ispahan, ikayaki
    MI_S32
    MI_ISP_IQ_GetNrChromaPre(MI_U32 DevId, MI_U32 Channel,
                             MI_ISP_IQ_NrChromaPreType_t *data); //[not support chip] macaron, pudding, ispahan, ikayaki
    MI_S32 MI_ISP_IQ_SetPfcEx(MI_U32 DevId, MI_U32 Channel,
                              MI_ISP_IQ_PfcExType_t *data); //[not support chip] macaron
    MI_S32 MI_ISP_IQ_GetPfcEx(MI_U32 DevId, MI_U32 Channel,
                              MI_ISP_IQ_PfcExType_t *data); //[not support chip] macaron
    MI_S32 MI_ISP_IQ_SetHdrEx(MI_U32 DevId, MI_U32 Channel,
                              MI_ISP_IQ_HdrExType_t *data); //[not support chip] macaron
    MI_S32 MI_ISP_IQ_GetHdrEx(MI_U32 DevId, MI_U32 Channel,
                              MI_ISP_IQ_HdrExType_t *data); //[not support chip] macaron
    MI_S32 MI_ISP_IQ_SetShpEx(MI_U32 DevId, MI_U32 Channel,
                              MI_ISP_IQ_SharpnessExType_t *data); //[not support chip] macaron
    MI_S32 MI_ISP_IQ_GetShpEx(MI_U32 DevId, MI_U32 Channel,
                              MI_ISP_IQ_SharpnessExType_t *data); //[not support chip] macaron
    MI_S32 MI_ISP_IQ_SetNr3dEx(MI_U32 DevId, MI_U32 Channel,
                               MI_ISP_IQ_Nr3dExType_t *data); //[not support chip] macaron
    MI_S32 MI_ISP_IQ_GetNr3dEx(MI_U32 DevId, MI_U32 Channel,
                               MI_ISP_IQ_Nr3dExType_t *data); //[not support chip] macaron
    MI_S32 MI_ISP_IQ_SetDummy(MI_U32 DevId, MI_U32 Channel, MI_ISP_IQ_DummyType_t *data);
    MI_S32 MI_ISP_IQ_GetDummy(MI_U32 DevId, MI_U32 Channel, MI_ISP_IQ_DummyType_t *data);
    MI_S32 MI_ISP_IQ_SetDummyEx(MI_U32 DevId, MI_U32 Channel, MI_ISP_IQ_DummyExType_t *data);
    MI_S32 MI_ISP_IQ_GetDummyEx(MI_U32 DevId, MI_U32 Channel, MI_ISP_IQ_DummyExType_t *data);
    MI_S32 MI_ISP_IQ_SetIqMode(MI_U32 DevId, MI_U32 Channel, MI_ISP_IQ_ParamMode_e *data);
    MI_S32 MI_ISP_IQ_GetIqMode(MI_U32 DevId, MI_U32 Channel, MI_ISP_IQ_ParamMode_e *data);
    MI_S32 MI_ISP_IQ_GetIqInd(MI_U32 DevId, MI_U32 Channel, MI_ISP_IQ_Index_e *data);
    MI_S32 MI_ISP_IQ_SetApiBypassMode(MI_U32 DevId, MI_U32 Channel, MI_ISP_IQ_ApiBypassType_t *data);
    MI_S32 MI_ISP_IQ_GetApiBypassMode(MI_U32 DevId, MI_U32 Channel, MI_ISP_IQ_ApiBypassType_t *data);
    MI_S32 MI_ISP_IQ_QueryCcmInfo(MI_U32 DevId, MI_U32 Channel, MI_ISP_IQ_CcmInfoType_t *data);
    MI_S32 MI_ISP_IQ_SetAdaptiveGamma(MI_U32 DevId, MI_U32 Channel, MI_ISP_IQ_AdaptiveGammaType_t *data);
    MI_S32 MI_ISP_IQ_GetAdaptiveGamma(MI_U32 DevId, MI_U32 Channel, MI_ISP_IQ_AdaptiveGammaType_t *data);
    MI_S32 MI_ISP_IQ_SetTemp(MI_U32 DevId, MI_U32 Channel, MI_ISP_IQ_TempType_t *data);
    MI_S32 MI_ISP_IQ_GetTemp(MI_U32 DevId, MI_U32 Channel, MI_ISP_IQ_TempType_t *data);
    MI_S32 MI_ISP_IQ_GetTempInfo(MI_U32 DevId, MI_U32 Channel, MI_ISP_IQ_TempInfoType_t *data);
    MI_S32 MI_ISP_IQ_SetROI(MI_U32 DevId, MI_U32 Channel, MI_ISP_IQ_ROIType_t *data);
    MI_S32 MI_ISP_IQ_GetROI(MI_U32 DevId, MI_U32 Channel, MI_ISP_IQ_ROIType_t *data);
    MI_S32 MI_ISP_IQ_SetDayNightDetection(MI_U32 DevId, MI_U32 Channel, MI_ISP_IQ_DaynightDetectionType_t *data);
    MI_S32 MI_ISP_IQ_GetDayNightDetection(MI_U32 DevId, MI_U32 Channel, MI_ISP_IQ_DaynightDetectionType_t *data);
    MI_S32 MI_ISP_IQ_QueryDayNightInfo(MI_U32 DevId, MI_U32 Channel, MI_ISP_IQ_DaynightInfoType_t *data);
    MI_S32 MI_ISP_IQ_SetCsa(MI_U32 DevId, MI_U32 Channel, MI_ISP_IQ_CsaType_t *data);
    MI_S32 MI_ISP_IQ_GetCsa(MI_U32 DevId, MI_U32 Channel, MI_ISP_IQ_CsaType_t *data);
    MI_S32 MI_ISP_IQ_SetYclpf(MI_U32 DevId, MI_U32 Channel, MI_ISP_IQ_YclpfType_t *data); //[not support chip] macaron, pudding, ispahan, tiramisu, ikayaki, muffin, maruko
    MI_S32 MI_ISP_IQ_GetYclpf(MI_U32 DevId, MI_U32 Channel, MI_ISP_IQ_YclpfType_t *data); //[not support chip] macaron, pudding, ispahan, tiramisu, ikayaki, muffin, maruko
    MI_S32 MI_ISP_IQ_SetAlscAdj(MI_U32 DevId, MI_U32 Channel, MI_ISP_IQ_AlscAdjType_t *data);
    MI_S32 MI_ISP_IQ_GetAlscAdj(MI_U32 DevId, MI_U32 Channel, MI_ISP_IQ_AlscAdjType_t *data);
    MI_S32 MI_ISP_IQ_SetWDRCurveAlignHDRtoLDR(MI_U32 DevId, MI_U32 Channel, MI_ISP_IQ_WDRCurveAlignHDRtoLDR_TYPE_t *data);
    MI_S32 MI_ISP_IQ_GetWDRCurveAlignHDRtoLDR(MI_U32 DevId, MI_U32 Channel, MI_ISP_IQ_WDRCurveAlignHDRtoLDR_TYPE_t *data);
    MI_S32 MI_ISP_IQ_SetWDRCurveAlignLDRtoHDR(MI_U32 DevId, MI_U32 Channel, MI_ISP_IQ_WDRCurveAlignLDRtoHDR_TYPE_t *data);
    MI_S32 MI_ISP_IQ_GetWDRCurveAlignLDRtoHDR(MI_U32 DevId, MI_U32 Channel, MI_ISP_IQ_WDRCurveAlignLDRtoHDR_TYPE_t *data);
    MI_S32 MI_ISP_IQ_SetSTITCH_LPF(MI_U32 DevId, MI_U32 Channel, MI_ISP_IQ_STITCH_LPF_TYPE_t *data);
    MI_S32 MI_ISP_IQ_GetSTITCH_LPF(MI_U32 DevId, MI_U32 Channel, MI_ISP_IQ_STITCH_LPF_TYPE_t *data);
    MI_S32 MI_ISP_IQ_SetWdrCurveParam(MI_U32 DevId, MI_U32 Channel, MI_ISP_IQ_WDRCurveParam_TYPE_t *data);
    MI_S32 MI_ISP_IQ_GetWdrCurveParam(MI_U32 DevId, MI_U32 Channel, MI_ISP_IQ_WDRCurveParam_TYPE_t *data);
    MI_S32 MI_ISP_IQ_SetAwbAlign(MI_U32 DevId, MI_U32 Channel, MI_ISP_IQ_AwbAlign_t *data);
    MI_S32 MI_ISP_IQ_GetAwbAlign(MI_U32 DevId, MI_U32 Channel, MI_ISP_IQ_AwbAlign_t *data);
    MI_S32 MI_ISP_IQ_SetSceneDecision(MI_U32 DevId, MI_U32 Channel, MI_ISP_IQ_SceneDecisionType_t *data);
    MI_S32 MI_ISP_IQ_GetSceneDecision(MI_U32 DevId, MI_U32 Channel, MI_ISP_IQ_SceneDecisionType_t *data);
    MI_S32 MI_ISP_IQ_SetSceneStatis(MI_U32 DevId, MI_U32 Channel, MI_ISP_IQ_SceneStatisType_t *data);
    MI_S32 MI_ISP_IQ_SetSceneAdj(MI_U32 DevId, MI_U32 Channel, MI_ISP_IQ_SceneAdjType_t *data);
    MI_S32 MI_ISP_IQ_GetSceneAdj(MI_U32 DevId, MI_U32 Channel, MI_ISP_IQ_SceneAdjType_t *data);
    MI_S32 MI_ISP_IQ_QuerySceneCurLevelInfo(MI_U32 DevId, MI_U32 Channel, MI_ISP_IQ_SceneCurLevelType_t *data);
    MI_S32 MI_ISP_IQ_QuerySceneInfo(MI_U32 DevId, MI_U32 Channel, MI_ISP_IQ_SceneInfoType_t *data);
    MI_S32 MI_ISP_IQ_SetDecompress(MI_U32 DevId, MI_U32 Channel, MI_ISP_IQ_DecompressType_t *data);
    MI_S32 MI_ISP_IQ_GetDecompress(MI_U32 DevId, MI_U32 Channel, MI_ISP_IQ_DecompressType_t *data);
    MI_S32 MI_ISP_IQ_SetAIBNRCtrl(MI_U32 DevId, MI_U32 Channel, MI_ISP_IQ_AiBnrCtrlType_t *data);
    MI_S32 MI_ISP_IQ_GetAIBNRCtrl(MI_U32 DevId, MI_U32 Channel, MI_ISP_IQ_AiBnrCtrlType_t *data);

    /************************************* IQ  API END   *************************************/

    /************************************* OTHER  API START *************************************/
    MI_S32 MI_ISP_OTHER_SetWDRCurveAlignHDRtoLDR(MI_U32 DevId, MI_U32 Channel, MI_ISP_IQ_WDRCurveAlignHDRtoLDR_TYPE_t *data);
    MI_S32 MI_ISP_OTHER_GetWDRCurveAlignHDRtoLDR(MI_U32 DevId, MI_U32 Channel, MI_ISP_IQ_WDRCurveAlignHDRtoLDR_TYPE_t *data);
    MI_S32 MI_ISP_OTHER_SetWDRCurveAlignLDRtoHDR(MI_U32 DevId, MI_U32 Channel, MI_ISP_IQ_WDRCurveAlignLDRtoHDR_TYPE_t *data);
    MI_S32 MI_ISP_OTHER_GetWDRCurveAlignLDRtoHDR(MI_U32 DevId, MI_U32 Channel, MI_ISP_IQ_WDRCurveAlignLDRtoHDR_TYPE_t *data);
    /************************************* OTHER  API END   *************************************/

    MI_S32 MI_ISP_IQ_SetAll(MI_U32 DevId, MI_U32 Channel, MI_U16 ApiId, MI_U32 ApiLen, MI_U8 *pApiBuf);
    MI_S32 MI_ISP_IQ_GetAll(MI_U32 DevId, MI_U32 Channel, MI_U16 ApiId, MI_U32 *ApiLen, MI_U8 *pApiBuf);

    /************************************* LOAD API START*************************************/
    MI_S32 MI_ISP_IQ_SetFpnCall(MI_U32 DevId, MI_U32 Channel, MI_U8 *param_ary[], MI_U8 param_num);
    MI_S32 MI_ISP_IQ_SetDarkFrameCall(MI_U32 DevId, MI_U32 Channel, MI_U8 *param_ary[], MI_U8 param_num);
    MI_S32 MI_ISP_IQ_SetDynamicDpCall(MI_U32 DevId, MI_U32 Channel, MI_U8 *param_ary[], MI_U8 param_num);
    MI_S32 MI_ISP_IQ_SetDynamicDpClusterCall(MI_U32 DevId, MI_U32 Channel, MI_U8 *param_ary[], MI_U8 param_num);
    MI_S32 MI_ISP_IQ_SetCrossTalkCall(MI_U32 DevId, MI_U32 Channel, MI_U8 *param_ary[], MI_U8 param_num);
    MI_S32 MI_ISP_IQ_SetFalseColorCall(MI_U32 DevId, MI_U32 Channel, MI_U8 *param_ary[], MI_U8 param_num);
    MI_S32 MI_ISP_IQ_SetDobcCall(MI_U32 DevId, MI_U32 Channel, MI_U8 *param_ary[], MI_U8 param_num);
    MI_S32 MI_ISP_IQ_SetObcCall(MI_U32 DevId, MI_U32 Channel, MI_U8 *param_ary[], MI_U8 param_num);
    MI_S32 MI_ISP_IQ_SetObcP1Call(MI_U32 DevId, MI_U32 Channel, MI_U8 *param_ary[], MI_U8 param_num);
    MI_S32 MI_ISP_IQ_SetObcP2Call(MI_U32 DevId, MI_U32 Channel, MI_U8 *param_ary[], MI_U8 param_num);
    MI_S32 MI_ISP_IQ_SetNrDeSpikeCall(MI_U32 DevId, MI_U32 Channel, MI_U8 *param_ary[], MI_U8 param_num);
    MI_S32 MI_ISP_IQ_SetNrLumaCall(MI_U32 DevId, MI_U32 Channel, MI_U8 *param_ary[], MI_U8 param_num);
    MI_S32 MI_ISP_IQ_SetNrChromaCall(MI_U32 DevId, MI_U32 Channel, MI_U8 *param_ary[], MI_U8 param_num);
    MI_S32 MI_ISP_IQ_SetNr3dCall(MI_U32 DevId, MI_U32 Channel, MI_U8 *param_ary[], MI_U8 param_num);
    MI_S32 MI_ISP_IQ_SetNr3dP1Call(MI_U32 DevId, MI_U32 Channel, MI_U8 *param_ary[], MI_U8 param_num);
    MI_S32 MI_ISP_IQ_SetRgbMatrixCall(MI_U32 DevId, MI_U32 Channel, MI_U8 *param_ary[], MI_U8 param_num);
    MI_S32 MI_ISP_IQ_SetHsvCall(MI_U32 DevId, MI_U32 Channel, MI_U8 *param_ary[], MI_U8 param_num);
    MI_S32 MI_ISP_IQ_SetRgbGammaCall(MI_U32 DevId, MI_U32 Channel, MI_U8 *param_ary[], MI_U8 param_num);
    MI_S32 MI_ISP_IQ_SetAdaptiveGammaCall(MI_U32 DevId, MI_U32 Channel, MI_U8 *param_ary[], MI_U8 param_num);
    MI_S32 MI_ISP_IQ_SetBrightnessCall(MI_U32 DevId, MI_U32 Channel, MI_U8 *param_ary[], MI_U8 param_num);
    MI_S32 MI_ISP_IQ_SetLightnessCall(MI_U32 DevId, MI_U32 Channel, MI_U8 *param_ary[], MI_U8 param_num);
    MI_S32 MI_ISP_IQ_SetContrastCall(MI_U32 DevId, MI_U32 Channel, MI_U8 *param_ary[], MI_U8 param_num);
    MI_S32 MI_ISP_IQ_SetColorToGrayCall(MI_U32 DevId, MI_U32 Channel, MI_U8 *param_ary[], MI_U8 param_num);
    MI_S32 MI_ISP_IQ_SetSharpnessCall(MI_U32 DevId, MI_U32 Channel, MI_U8 *param_ary[], MI_U8 param_num);
    MI_S32 MI_ISP_IQ_SetSaturationCall(MI_U32 DevId, MI_U32 Channel, MI_U8 *param_ary[], MI_U8 param_num);
    MI_S32 MI_ISP_IQ_SetPfcCall(MI_U32 DevId, MI_U32 Channel, MI_U8 *param_ary[], MI_U8 param_num);
    MI_S32 MI_ISP_IQ_SetYuvGammaCall(MI_U32 DevId, MI_U32 Channel, MI_U8 *param_ary[], MI_U8 param_num);
    MI_S32 MI_ISP_IQ_SetWdrCall(MI_U32 DevId, MI_U32 Channel, MI_U8 *param_ary[], MI_U8 param_num);
    MI_S32 MI_ISP_IQ_SetWdrLceCall(MI_U32 DevId, MI_U32 Channel, MI_U8 *param_ary[], MI_U8 param_num);
    MI_S32 MI_ISP_IQ_SetWdrLtmCall(MI_U32 DevId, MI_U32 Channel, MI_U8 *param_ary[], MI_U8 param_num);
    MI_S32 MI_ISP_IQ_SetWdrNrCall(MI_U32 DevId, MI_U32 Channel, MI_U8 *param_ary[], MI_U8 param_num);
    MI_S32 MI_ISP_IQ_SetWdrCurveAdvCall(MI_U32 DevId, MI_U32 Channel, MI_U8 *param_ary[], MI_U8 param_num);
    MI_S32 MI_ISP_IQ_SetWdrCurveFullCall(MI_U32 DevId, MI_U32 Channel, MI_U8 *param_ary[], MI_U8 param_num);
    MI_S32 MI_ISP_IQ_SetHdrCall(MI_U32 DevId, MI_U32 Channel, MI_U8 *param_ary[], MI_U8 param_num);
    MI_S32 MI_ISP_IQ_SetColorTransCall(MI_U32 DevId, MI_U32 Channel, MI_U8 *param_ary[], MI_U8 param_num);
    MI_S32 MI_ISP_IQ_SetColorTransEXCall(MI_U32 DevId, MI_U32 Channel, MI_U8 *param_ary[], MI_U8 param_num);
    MI_S32 MI_ISP_IQ_SetRgbirCall(MI_U32 DevId, MI_U32 Channel, MI_U8 *param_ary[], MI_U8 param_num);
    MI_S32 MI_ISP_IQ_SetLscCall(MI_U32 DevId, MI_U32 Channel, MI_U8 *param_ary[], MI_U8 param_num);
    MI_S32 MI_ISP_IQ_SetLscCtrlCall(MI_U32 DevId, MI_U32 Channel, MI_U8 *param_ary[], MI_U8 param_num);
    MI_S32 MI_ISP_IQ_SetAlscCall(MI_U32 DevId, MI_U32 Channel, MI_U8 *param_ary[], MI_U8 param_num);
    MI_S32 MI_ISP_IQ_SetAlscCtrlCall(MI_U32 DevId, MI_U32 Channel, MI_U8 *param_ary[], MI_U8 param_num);
	MI_S32 MI_ISP_IQ_SetDarkShadingCall(MI_U32 DevId, MI_U32 Channel, MI_U8 *param_ary[], MI_U8 param_num);
    MI_S32 MI_ISP_IQ_SetDefogCall(MI_U32 DevId, MI_U32 Channel, MI_U8 *param_ary[], MI_U8 param_num);
    MI_S32 MI_ISP_IQ_SetDeMosaicCall(MI_U32 DevId, MI_U32 Channel, MI_U8 *param_ary[], MI_U8 param_num);
    MI_S32 MI_ISP_IQ_SetR2YCall(MI_U32 DevId, MI_U32 Channel, MI_U8 *param_ary[], MI_U8 param_num);
    MI_S32 MI_ISP_IQ_SetNrLumaAdvCall(MI_U32 DevId, MI_U32 Channel, MI_U8 *param_ary[], MI_U8 param_num);
    MI_S32 MI_ISP_IQ_SetNrChromaAdvCall(MI_U32 DevId, MI_U32 Channel, MI_U8 *param_ary[], MI_U8 param_num);
    MI_S32 MI_ISP_IQ_SetNrChromaPreCall(MI_U32 DevId, MI_U32 Channel, MI_U8 *param_ary[], MI_U8 param_num);
    MI_S32 MI_ISP_IQ_SetPfcExCall(MI_U32 DevId, MI_U32 Channel, MI_U8 *param_ary[], MI_U8 param_num);
    MI_S32 MI_ISP_IQ_SetHdrExCall(MI_U32 DevId, MI_U32 Channel, MI_U8 *param_ary[], MI_U8 param_num);
    MI_S32 MI_ISP_IQ_SetShpExCall(MI_U32 DevId, MI_U32 Channel, MI_U8 *param_ary[], MI_U8 param_num);
    MI_S32 MI_ISP_IQ_SetNr3dExCall(MI_U32 DevId, MI_U32 Channel, MI_U8 *param_ary[], MI_U8 param_num);
    MI_S32 MI_ISP_IQ_SetDummyCall(MI_U32 DevId, MI_U32 Channel, MI_U8 *param_ary[], MI_U8 param_num);
    MI_S32 MI_ISP_IQ_SetDummyExCall(MI_U32 DevId, MI_U32 Channel, MI_U8 *param_ary[], MI_U8 param_num);
    MI_S32 MI_ISP_IQ_SetTempCall(MI_U32 DevId, MI_U32 Channel, MI_U8 *param_ary[], MI_U8 param_num);
    MI_S32 MI_ISP_IQ_SetROICall(MI_U32 DevId, MI_U32 Channel, MI_U8 *param_ary[], MI_U8 param_num);
    MI_S32 MI_ISP_IQ_SetVersionInfoCall(MI_U32 DevId, MI_U32 Channel, MI_U8 *param_ary[], MI_U8 param_num);
    MI_S32 MI_ISP_IQ_SetDayNightDetectionCall(MI_U32 DevId, MI_U32 Channel, MI_U8 *param_ary[], MI_U8 param_num);
    MI_S32 MI_ISP_IQ_SetCsaCall(MI_U32 DevId, MI_U32 Channel, MI_U8 *param_ary[], MI_U8 param_num);
    MI_S32 MI_ISP_IQ_SetYclpfCall(MI_U32 DevId, MI_U32 Channel, MI_U8 *param_ary[], MI_U8 param_num);
    MI_S32 MI_ISP_IQ_SetAlscAdjCall(MI_U32 DevId, MI_U32 Channel, MI_U8 *param_ary[], MI_U8 param_num);
    MI_S32 MI_ISP_IQ_SetThermalIRCall(MI_U32 DevId, MI_U32 Channel, MI_U8 *param_ary[], MI_U8 param_num);
    MI_S32 MI_ISP_IQ_SetStitchLpfCall(MI_U32 DevId, MI_U32 Channel, MI_U8 *param_ary[], MI_U8 param_num);
    MI_S32 MI_ISP_IQ_SetWdrCurveParamCall(MI_U32 DevId, MI_U32 Channel, MI_U8 *param_ary[], MI_U8 param_num);
    MI_S32 MI_ISP_IQ_SetFpnCtrlCall(MI_U32 DevId, MI_U32 Channel, MI_U8 *param_ary[], MI_U8 param_num);
    MI_S32 MI_ISP_IQ_SetGroupHoldCall(MI_U32 DevId, MI_U32 Channel, MI_U8 *param_ary[], MI_U8 param_num);
    MI_S32 MI_ISP_IQ_SetWDRCurveAlignHDRtoLDRCall(MI_U32 DevId, MI_U32 Channel, MI_U8 *param_ary[], MI_U8 param_num);
    MI_S32 MI_ISP_IQ_SetWDRCurveAlignLDRtoHDRCall(MI_U32 DevId, MI_U32 Channel, MI_U8 *param_ary[], MI_U8 param_num);
    MI_S32 MI_ISP_IQ_SetSceneDecisionCall(MI_U32 DevId, MI_U32 Channel, MI_U8 *param_ary[], MI_U8 param_num);
    MI_S32 MI_ISP_IQ_SetSceneAdjCall(MI_U32 DevId, MI_U32 Channel, MI_U8 *param_ary[], MI_U8 param_num);
    MI_S32 MI_ISP_IQ_SetDecompressCall(MI_U32 DevId, MI_U32 Channel, MI_U8 *param_ary[], MI_U8 param_num);
    MI_S32 MI_ISP_IQ_SetAiBnrCtrlCall(MI_U32 DevId, MI_U32 Channel, MI_U8 *param_ary[], MI_U8 param_num);

    /************************************* LOAD API END  *************************************/

#endif

#if 1

    typedef struct MI_ISP_IQ_ExcuteCmdArray_s
    {
        MI_U32 u32CmdType;
        MI_U16 u16APIID;
        MI_S32 (*callback)(MI_U32 DevId, MI_U32 Channel, MI_U8 *param_ary[], MI_U8 param_num);
    } MI_ISP_IQ_ExcuteCmdArray_t;

    typedef enum __attribute__((aligned(1)))
    {
        E_MI_ISP_CAMERA_CMD_SET = 0,
        E_MI_ISP_CAMERA_CMD_GET,
        E_MI_ISP_CAMERA_CMD_SET_MODE,
        E_MI_ISP_CAMERA_CMD_GET_MODE,
        E_MI_ISP_CAMERA_CMD_GET_PIC,
        E_MI_ISP_CAMERA_CMD_SET_API,
        E_MI_ISP_CAMERA_CMD_GET_API,
        E_MI_ISP_CAMERA_CMD_UPLOAD_FILE,   /* client upload file to server */
        E_MI_ISP_CAMERA_CMD_DOWNLOAD_FILE, /* client download file from server*/
    } MI_ISP_IQ_CameraExtCmdType_e;

    typedef struct MI_ISP_IQ_CmdHeader_s
    {
        MI_ISP_IQ_CameraExtCmdType_e CmdType;
        MI_S32                       CmdLen;
    } MI_ISP_IQ_CmdHeader_t;

    typedef struct MI_ISP_IQ_ApiParam_s
    {
        MI_U32                u32MagicKey;
        MI_ISP_IQ_CmdHeader_t sCmdheader;
        MI_U16                u16APIID;
        MI_U16                u16ParamNum;
    } MI_ISP_IQ_ApiParam_t;

    typedef struct MI_ISP_IQ_ApiBinFile_s
    {
        MI_U32 u32FileID;
        MI_U32 u32ISPVer;
        MI_U32 u32DataLen;
        MI_U32 u32Checksum;
        MI_U32 u32MagicKey;
        MI_U32 u32Reserved[3];
    } MI_ISP_IQ_ApiBinFile_t;

    typedef struct MI_ISP_IQ_ApiCaliFile_s
    {
        MI_U32 u32CaliVer;
        MI_U32 u32DataSize;
        MI_U32 u32Checksum;
        MI_U32 u32Reserved[2];
    } MI_ISP_IQ_ApiCaliFile_t;

    MI_U32 MI_ISP_IQ_ApiCrc32(MI_U8 *data, MI_U32 len);
    void   MI_ISP_IQ_ApiDumpParam(MI_ISP_IQ_ApiParam_t param);
    MI_S32 MI_ISP_IQ_ApiExecuteCmd(MI_U32 DevId, MI_U32 Channel, MI_U32 cmd_type, MI_U8 *param_buf);
    MI_S32 MI_ISP_IQ_ApiReset(MI_U32 DevId, MI_U32 Channel, MI_U16 APIID);
    MI_S32 MI_ISP_IQ_ApiDisableAll(MI_U32 DevId, MI_U32 Channel);
    MI_S32 MI_ISP_IQ_ApiCmdLoadBinFile(MI_U32 DevId, MI_U32 Channel, MI_U8 *bindata_buf, MI_U32 user_key);
    MI_S32 MI_ISP_IQ_ApiCmdLoadCaliData(MI_U32 DevId, MI_U32 Channel, MI_U8 *bindata_buf, MI_U32 BIN_BUF_MAX_LEN,
                                        MI_ISP_IQ_CaliItem_e eCaliItem);

#endif

#ifdef __cplusplus
} // end of extern C
#endif

#endif //_MI_ISP_IQ_H_
