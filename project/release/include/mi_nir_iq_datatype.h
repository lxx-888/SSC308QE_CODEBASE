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

#ifndef _MI_NIR_IQ_DATATYPE_H_
#define _MI_NIR_IQ_DATATYPE_H_

#include "mi_nir_datatype.h"

#define MI_NIR_AUTO_NUM           16
#define BLEND_SAT_X_NUM           8
#define BLEND_SAT_Y_NUM           9
#define CONTRAST_X_NUM            8
#define CONTRAST_Y_NUM            9
#define SAT_X_NUM                 8
#define SAT_Y_NUM                 9
#define WEIGHT_X_NUM              13
#define WEIGHT_Y_NUM              14
#define WEIGHT_SAT_X_NUM          8
#define WEIGHT_SAT_Y_NUM          9
#define WEIGHT_VISY_X_NUM         8
#define WEIGHT_VISY_Y_NUM         9
#define WEIGHT_NIRY_X_NUM         8
#define WEIGHT_NIRY_Y_NUM         9
#define WEIGHT_GAP_CONTRAST_X_NUM 14
#define WEIGHT_GAP_CONTRAST_Y_NUM 15
#define WEIGHT_GAP_LUMA_X_NUM     14
#define WEIGHT_GAP_LUMA_Y_NUM     15
#define COLOR_RECOVERY_X_NUM      8
#define COLOR_RECOVERY_Y_NUM      9

typedef enum __attribute__((aligned(4)))
{
    E_API_BLEND_XNR = 0,
    E_API_BLEND_SAT,
    E_API_CONTRAST,
    E_API_SATURATION,
    E_API_WEIGHT,
    E_API_BWEIGHT_BY_SAT,
    E_API_DWEIGHT_BY_SAT,
    E_API_BWEIGHT_BY_VISY,
    E_API_DWEIGHT_BY_VISY,
    E_API_BWEIGHT_BY_NIRY,
    E_API_DWEIGHT_BY_NIRY,
    E_API_BWEIGHT_BY_GAP_CONTRAST,
    E_API_DWEIGHT_BY_GAP_CONTRAST,
    E_API_BWEIGHT_BY_GAP_LUMA,
    E_API_DWEIGHT_BY_GAP_LUMA,
    E_API_BYPASS_MODE,
    E_API_COLOR_RECOVERY,
    E_API_MAX, // for para reset
} MI_NIR_IQ_ApiId_e;

typedef enum
{
    E_NIR_IQ_OP_TYP_AUTO   = 0,
    E_NIR_IQ_OP_TYP_MANUAL = !E_NIR_IQ_OP_TYP_AUTO,
    E_NIRNIR_IQ_OP_TYP_MODE_MAX,
} MI_NIR_IQ_OpType_e;

typedef struct MI_NIR_IQ_BlendingXnr_s
{
    MI_U8 u8SatTh;
    MI_U8 u8YTh;
    MI_U8 u8XnrStr;
} MI_NIR_IQ_BlendingXnr_t;

typedef struct MI_NIR_IQ_BlendingXnrAutoAttr_s
{
    MI_NIR_IQ_BlendingXnr_t stBlendingXnr[MI_NIR_AUTO_NUM];
} MI_NIR_IQ_BlendingXnrAutoAttr_t;

typedef struct MI_NIR_IQ_BlendingXnrManualAttr_s
{
    MI_NIR_IQ_BlendingXnr_t stBlendingXnr;
} MI_NIR_IQ_BlendingXnrManualAttr_t;

typedef struct MI_NIR_IQ_BlendingXnrAttr_s
{
    MI_NIR_IQ_OpType_e                enOpType;
    MI_NIR_IQ_BlendingXnrAutoAttr_t   stAuto;
    MI_NIR_IQ_BlendingXnrManualAttr_t stManual;
} MI_NIR_IQ_BlendingXnrAttr_t;

typedef struct MI_NIR_IQ_BlendingSaturationGain_s
{
    MI_U8 u8YGainSft; // 0~255
    MI_U8 u8MaxUVRatioSft;
} MI_NIR_IQ_BlendingSaturationGain_t;

typedef struct MI_NIR_IQ_BlendingSaturationCurve_s
{
    MI_U16 u16CurveX[BLEND_SAT_X_NUM];
    MI_U16 u16CurveY[BLEND_SAT_Y_NUM];
} MI_NIR_IQ_BlendingSaturationCurve_t;

typedef struct MI_NIR_IQ_BlendingSaturationParam_s
{
    union
    {
        MI_NIR_IQ_BlendingSaturationGain_t  stBlendingSatGain;
        MI_NIR_IQ_BlendingSaturationCurve_t stBlendingSatLut;
    };
} MI_NIR_IQ_BlendingSaturationParam_t;

typedef struct MI_NIR_IQ_BlendingSaturationAutoAttr_s
{
    MI_NIR_IQ_BlendingSaturationParam_t stParaAPI[MI_NIR_AUTO_NUM];
} MI_NIR_IQ_BlendingSaturationAutoAttr_t;

typedef struct MI_NIR_IQ_BlendingSaturationManualAttr_s
{
    MI_NIR_IQ_BlendingSaturationParam_t stParaAPI;
} MI_NIR_IQ_BlendingSaturationManualAttr_t;

typedef struct MI_NIR_IQ_BlendingSaturationAttr_s
{
    MI_NIR_IQ_OpType_e                       enOpType;
    MI_NIR_IQ_BlendingSaturationAutoAttr_t   stAuto;
    MI_NIR_IQ_BlendingSaturationManualAttr_t stManual;
} MI_NIR_IQ_BlendingSaturationAttr_t;

typedef struct MI_NIR_IQ_ContrastParam_s
{
    MI_U16 u16ContrastThd;
    MI_U16 u16ContrastThdExConf;
    MI_U16 u16CurveX[CONTRAST_X_NUM];
    MI_U16 u16CurveY[CONTRAST_Y_NUM];
} MI_NIR_IQ_ContrastParam_t;

typedef struct MI_NIR_IQ_ContrastAutoAttr_s
{
    MI_NIR_IQ_ContrastParam_t stParaAPI[MI_NIR_AUTO_NUM];
} MI_NIR_IQ_ContrastAutoAttr_t;

typedef struct MI_NIR_IQ_ContrastManualAttr_s
{
    MI_NIR_IQ_ContrastParam_t stParaAPI;
} MI_NIR_IQ_ContrastManualAttr_t;

typedef struct MI_NIR_IQ_ContrastAttr_s
{
    MI_NIR_IQ_OpType_e             enOpType;
    MI_NIR_IQ_ContrastAutoAttr_t   stAuto;
    MI_NIR_IQ_ContrastManualAttr_t stManual;
} MI_NIR_IQ_ContrastAttr_t;

typedef struct MI_NIR_IQ_SaturationParam_s
{
    MI_U16 u16SatConf;
    MI_U16 u16CurveX[SAT_X_NUM];
    MI_U16 u16CurveY[SAT_Y_NUM];
} MI_NIR_IQ_SaturationParam_t;

typedef struct MI_NIR_IQ_SaturationAutoAttr_s
{
    MI_NIR_IQ_SaturationParam_t stParaAPI[MI_NIR_AUTO_NUM];
} MI_NIR_IQ_SaturationAutoAttr_t;

typedef struct MI_NIR_IQ_SaturationManualAttr_s
{
    MI_NIR_IQ_SaturationParam_t stParaAPI;
} MI_NIR_IQ_SaturationManualAttr_t;

typedef struct MI_NIR_IQ_SaturationAttr_s
{
    MI_NIR_IQ_OpType_e               enOpType;
    MI_NIR_IQ_SaturationAutoAttr_t   stAuto;
    MI_NIR_IQ_SaturationManualAttr_t stManual;
} MI_NIR_IQ_SaturationAttr_t;

typedef struct MI_NIR_IQ_WeightParam_s
{
    MI_U8  u8BsRatioCus;
    MI_U8  u8DtRatioCus;
    MI_U16 u16VisCurveX[WEIGHT_X_NUM];
    MI_U16 u16VisCurveY[WEIGHT_Y_NUM];
    MI_U16 u16NirCurveX[WEIGHT_X_NUM];
    MI_U16 u16NirCurveY[WEIGHT_Y_NUM];
} MI_NIR_IQ_WeightParam_t;

typedef struct MI_NIR_IQ_WeightAutoAttr_s
{
    MI_NIR_IQ_WeightParam_t stParaAPI[MI_NIR_AUTO_NUM];
} MI_NIR_IQ_WeightAutoAttr_t;

typedef struct MI_NIR_IQ_WeightManualAttr_s
{
    MI_NIR_IQ_WeightParam_t stParaAPI;
} MI_NIR_IQ_WeightManualAttr_t;

typedef struct MI_NIR_IQ_WeightAttr_s
{
    MI_NIR_IQ_OpType_e           enOpType;
    MI_NIR_IQ_WeightAutoAttr_t   stAuto;
    MI_NIR_IQ_WeightManualAttr_t stManual;
} MI_NIR_IQ_WeightAttr_t;

typedef struct MI_NIR_IQ_WeightBySaturationParam_s
{
    MI_U16 u16CurveX[WEIGHT_SAT_X_NUM];
    MI_S16 s16CurveY[WEIGHT_SAT_Y_NUM];
} MI_NIR_IQ_WeightBySaturationParam_t;

typedef struct MI_NIR_IQ_WeightBySaturationAutoAttr_s
{
    MI_NIR_IQ_WeightBySaturationParam_t stParaAPI[MI_NIR_AUTO_NUM];
} MI_NIR_IQ_WeightBySaturationAutoAttr_t;

typedef struct MI_NIR_IQ_WeightBySaturationManualAttr_s
{
    MI_NIR_IQ_WeightBySaturationParam_t stParaAPI;
} MI_NIR_IQ_WeightBySaturationManualAttr_t;

typedef struct MI_NIR_IQ_WeightBySaturationAttr_s
{
    MI_NIR_IQ_OpType_e                       enOpType;
    MI_NIR_IQ_WeightBySaturationAutoAttr_t   stAuto;
    MI_NIR_IQ_WeightBySaturationManualAttr_t stManual;
} MI_NIR_IQ_WeightBySaturationAttr_t;

typedef struct MI_NIR_IQ_WeightByVisYParam_s
{
    MI_U16 u16CurveX[WEIGHT_VISY_X_NUM];
    MI_S16 s16CurveY[WEIGHT_VISY_Y_NUM];
} MI_NIR_IQ_WeightByVisYParam_t;

typedef struct MI_NIR_IQ_WeightByVisYtAutoAttr_s
{
    MI_NIR_IQ_WeightByVisYParam_t stParaAPI[MI_NIR_AUTO_NUM];
} MI_NIR_IQ_WeightByVisYtAutoAttr_t;

typedef struct MI_NIR_IQ_WeightByVisYManualAttr_s
{
    MI_NIR_IQ_WeightByVisYParam_t stParaAPI;
} MI_NIR_IQ_WeightByVisYManualAttr_t;

typedef struct MI_NIR_IQ_WeightByVisYAttr_s
{
    MI_NIR_IQ_OpType_e                 enOpType;
    MI_NIR_IQ_WeightByVisYtAutoAttr_t  stAuto;
    MI_NIR_IQ_WeightByVisYManualAttr_t stManual;
} MI_NIR_IQ_WeightByVisYAttr_t;

typedef struct MI_NIR_IQ_WeightByNirYParam_s
{
    MI_U16 u16CurveX[WEIGHT_NIRY_X_NUM];
    MI_S16 s16CurveY[WEIGHT_NIRY_Y_NUM];
} MI_NIR_IQ_WeightByNirYParam_t;

typedef struct MI_NIR_IQ_WeightByNirYAutoAttr_s
{
    MI_NIR_IQ_WeightByNirYParam_t stParaAPI[MI_NIR_AUTO_NUM];
} MI_NIR_IQ_WeightByNirYAutoAttr_t;

typedef struct MI_NIR_IQ_WeightByNirYManualAttr_s
{
    MI_NIR_IQ_WeightByNirYParam_t stParaAPI;
} MI_NIR_IQ_WeightByNirYManualAttr_t;

typedef struct MI_NIR_IQ_WeightByNirYAttr_s
{
    MI_NIR_IQ_OpType_e                 enOpType;
    MI_NIR_IQ_WeightByNirYAutoAttr_t   stAuto;
    MI_NIR_IQ_WeightByNirYManualAttr_t stManual;
} MI_NIR_IQ_WeightByNirYAttr_t;

typedef struct MI_NIR_IQ_WeightByGapContrastParam_s
{
    MI_U16 u16CurveX[WEIGHT_GAP_CONTRAST_X_NUM];
    MI_S16 s16CurveY[WEIGHT_GAP_CONTRAST_Y_NUM];
} MI_NIR_IQ_WeightByGapContrastParam_t;

typedef struct MI_NIR_IQ_WeightByGapContrastAutoAttr_s
{
    MI_NIR_IQ_WeightByGapContrastParam_t stParaAPI[MI_NIR_AUTO_NUM];
} MI_NIR_IQ_WeightByGapContrastAutoAttr_t;

typedef struct MI_NIR_IQ_WeightByGapContrastManualAttr_s
{
    MI_NIR_IQ_WeightByGapContrastParam_t stParaAPI;
} MI_NIR_IQ_WeightByGapContrastManualAttr_t;

typedef struct MI_NIR_IQ_WeightByGapContrastAttr_s
{
    MI_NIR_IQ_OpType_e                        enOpType;
    MI_NIR_IQ_WeightByGapContrastAutoAttr_t   stAuto;
    MI_NIR_IQ_WeightByGapContrastManualAttr_t stManual;
} MI_NIR_IQ_WeightByGapContrastAttr_t;

typedef struct MI_NIR_IQ_WeightByGapLumaParam_s
{
    MI_U16 u16CurveX[WEIGHT_GAP_LUMA_X_NUM];
    MI_S16 s16CurveY[WEIGHT_GAP_LUMA_Y_NUM];
} MI_NIR_IQ_WeightByGapLumaParam_t;

typedef struct MI_NIR_IQ_WeightByGapLumaAutoAttr_s
{
    MI_NIR_IQ_WeightByGapLumaParam_t stParaAPI[MI_NIR_AUTO_NUM];
} MI_NIR_IQ_WeightByGapLumaAutoAttr_t;

typedef struct MI_NIR_IQ_WeightByGapLumaManualAttr_s
{
    MI_NIR_IQ_WeightByGapLumaParam_t stParaAPI;
} MI_NIR_IQ_WeightByGapLumaManualAttr_t;

typedef struct MI_NIR_IQ_WeightByGapLumaAttr_s
{
    MI_NIR_IQ_OpType_e                    enOpType;
    MI_NIR_IQ_WeightByGapLumaAutoAttr_t   stAuto;
    MI_NIR_IQ_WeightByGapLumaManualAttr_t stManual;
} MI_NIR_IQ_WeightByGapLumaAttr_t;

typedef struct MI_NIR_IQ_ColorRecoveryParam_s
{
    MI_U16 u16CurveX[COLOR_RECOVERY_X_NUM];
    MI_U16 u16CurveY[COLOR_RECOVERY_Y_NUM];
} MI_NIR_IQ_ColorRecoveryParam_t;

typedef struct MI_NIR_IQ_ColorRecoveryAutoAttr_s
{
    MI_NIR_IQ_ColorRecoveryParam_t stParaAPI[MI_NIR_AUTO_NUM];
} MI_NIR_IQ_ColorRecoveryAutoAttr_t;

typedef struct MI_NIR_IQ_ColorRecoveryManualAttr_s
{
    MI_NIR_IQ_ColorRecoveryParam_t stParaAPI;
} MI_NIR_IQ_ColorRecoveryManualAttr_t;

typedef struct MI_NIR_IQ_ColorRecoveryAttr_s
{
    MI_NIR_IQ_OpType_e                  enOpType;
    MI_NIR_IQ_ColorRecoveryAutoAttr_t   stAuto;
    MI_NIR_IQ_ColorRecoveryManualAttr_t stManual;
} MI_NIR_IQ_ColorRecoveryAttr_t;

#endif /* _MI_NIR_IQ_DATATYPE_H_ */
