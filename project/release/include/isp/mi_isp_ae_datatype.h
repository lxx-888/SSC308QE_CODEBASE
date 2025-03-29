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

#ifndef _MI_ISP_AE_DATATYPE_H_
#define _MI_ISP_AE_DATATYPE_H_

#ifdef __USE_USERSPACE_3A__
#include "mi_common.h"
#endif

#define MI_ISP_AE_STAT_BLOCK     128 * 90
#define MI_ISP_AE_HIST_BIN       128
#define MI_ISP_AE_RGBIR_HIST_BIN 256
#define MI_ISP_AE_LUT_32         32
#define MI_ISP_AE_LUT_16         16
#define MI_ISP_AE_LUT_4          4
#define MI_ISP_AE_SCENE_NUM       16
#define MI_ISP_AE_SCENE_ADJ_NUM    8
#define MI_ISP_AE_SCENE_PARAM_NUM 16

typedef enum __attribute__((aligned(4)))
{
    E_SS_AE_FALSE = 0,
    E_SS_AE_TRUE  = !E_SS_AE_FALSE,
    E_SS_AE_BOOL_MAX
} MI_ISP_AE_Bool_e;

typedef enum __attribute__((aligned(4)))
{
    E_SS_AE_OP_TYP_AUTO   = 0,
    E_SS_AE_OP_TYP_MANUAL = !E_SS_AE_OP_TYP_AUTO,
    E_SS_AE_OP_TYP_MODE_MAX
} MI_ISP_AE_OpType_e;

typedef enum __attribute__((aligned(4)))
{
    E_SS_AE_STATE_NORMAL = 0,
    E_SS_AE_STATE_PAUSE  = 1,
    E_SS_AE_STATE_MAX
} MI_ISP_AE_SmStateType_e;

typedef enum __attribute__((aligned(4)))
{
    E_SS_AE_MODE_A,  // auto
    E_SS_AE_MODE_AV, // aperture priority
    E_SS_AE_MODE_SV,
    E_SS_AE_MODE_TV, // shutter priority
    E_SS_AE_MODE_M,  // manual mode
    E_SS_AE_MODE_MAX
} MI_ISP_AE_ModeType_e;

typedef enum __attribute__((aligned(4)))
{
    E_SS_AE_WEIGHT_AVERAGE = 0,
    E_SS_AE_WEIGHT_CENTER,
    E_SS_AE_WEIGHT_SPOT,
    E_SS_AE_WEIGHT_MAX
} MI_ISP_AE_WinWeightModeType_e;

typedef enum __attribute__((aligned(4)))
{
    E_SS_AE_FLICKER_TYPE_DISABLE = 0,
    E_SS_AE_FLICKER_TYPE_60HZ    = 1,
    E_SS_AE_FLICKER_TYPE_50HZ    = 2,
    E_SS_AE_FLICKER_TYPE_AUTO    = 3,
    // E_SS_AE_FLICKER_TYPE_DETECT_60HZ = 4,
    // E_SS_AE_FLICKER_TYPE_DETECT_50HZ = 5,
    E_SS_AE_FLICKER_TYPE_MAX
} MI_ISP_AE_FlickerType_e;

typedef enum __attribute__((aligned(4)))
{
    E_SS_AE_FLICKER_TYPE_DETECT_60HZ = 0,
    E_SS_AE_FLICKER_TYPE_DETECT_50HZ = 1,
    E_SS_AE_FLICKER_TYPE_DETECT_MAX
} MI_ISP_AE_FlickerDetectType_e;

typedef enum __attribute__((aligned(4)))
{
    E_SS_AE_HDR_COMB_MODE_ML = 0,
#if 0 // IFORD NOT SUPPORT
    E_SS_AE_HDR_COMB_MODE_SM = 1,
#endif
    E_SS_AE_HDR_COMB_MODE_MAX
} MI_ISP_AE_HdrComb_ModeType_e;

// typedef struct MI_ISP_AE_FlickerInfoType_s
//{
//     MI_ISP_AE_Bool_e bIsEffective;
//     MI_ISP_AE_FlickerType_e eFlickerType;
//     MI_U16 u16Score;
// } MI_ISP_AE_FlickerInfoType_t;

typedef struct MI_ISP_AE_FlickerExType_s
{
    MI_ISP_AE_Bool_e              bEnable;          // 0 ~ 1
    MI_ISP_AE_OpType_e            enOpType;         // M_AUTO ~ (M_MODMAX-1)
    MI_U8                         u8AmpSensitivity; // 1 ~ 100
    MI_U8                         u8ScoreThd;       // 1 ~ 100
    MI_U8                         uRefreshCycles;   // 1 ~ 10
    MI_U8                         u8ValidTimesThd;  // 1 ~ 10
    MI_ISP_AE_FlickerDetectType_e eFlickerType;     // 60Hz = 0, 50Hz = 1
} MI_ISP_AE_FlickerExType_t;

typedef struct MI_ISP_AE_FlickerExInfoType_s
{
    MI_ISP_AE_Bool_e              bIsEffective;
    MI_ISP_AE_FlickerDetectType_e eFlickerType;
    MI_U16                        u16Score;
} MI_ISP_AE_FlickerExInfoType_t;

typedef enum __attribute__((aligned(4)))
{
    E_SS_AE_STRATEGY_BRIGHTTONE,
    E_SS_AE_STRATEGY_DARKTONE,
    E_SS_AE_STRATEGY_AUTO,
    E_SS_AE_STRATEGY_MAX
} MI_ISP_AE_StrategyType_e;

typedef struct MI_ISP_AE_HistWeightYType_s
{
    MI_U32 u32LumY;
    MI_U32 u32AvgY;
    MI_U32 u32Hits[MI_ISP_AE_HIST_BIN];
} MI_ISP_AE_HistWeightYType_t;

typedef struct MI_ISP_AE_ExpoValueType_s
{
    MI_U32 u32FNx10;
    MI_U32 u32SensorGain;
    MI_U32 u32ISPGain;
    MI_U32 u32US;
} MI_ISP_AE_ExpoValueType_t;

typedef struct MI_ISP_AE_ExpoInfoType_s
{
    MI_ISP_AE_Bool_e            bIsStable;
    MI_ISP_AE_Bool_e            bIsReachBoundary;
    MI_ISP_AE_ExpoValueType_t   stExpoValueLong;
    MI_ISP_AE_ExpoValueType_t   stExpoValueShort;
    MI_ISP_AE_HistWeightYType_t stHistWeightY;
    MI_U32                      u32LVx10;
    MI_S32                      s32BV;
    MI_U32                      u32SceneTarget;
} MI_ISP_AE_ExpoInfoType_t;

typedef struct MI_ISP_AE_VeryShortExpoInfoType_s
{
    MI_ISP_AE_ExpoValueType_t   stExpoValueVeryShort;
} MI_ISP_AE_VeryShortExpoInfoType_t;

typedef struct MI_ISP_AE_EvCompType_s
{
    MI_S32 s32EV;
    MI_U32 u32Grad;
} MI_ISP_AE_EvCompType_t;

typedef struct MI_ISP_AE_IntpLutType_s
{
    MI_U16 u16NumOfPoints;
    MI_S32 u32Y[MI_ISP_AE_LUT_16];
    MI_S32 u32X[MI_ISP_AE_LUT_16];
} MI_ISP_AE_IntpLutType_t;

typedef struct MI_ISP_AE_ConvSpeedParam_s
{
    MI_U32 u32SpeedX[MI_ISP_AE_LUT_4];
    MI_U32 u32SpeedY[MI_ISP_AE_LUT_4];
} MI_ISP_AE_ConvSpeedParam_t;

typedef struct MI_ISP_AE_ConvSpeedParam_EX_s
{
    MI_U32 u32SpeedX[MI_ISP_AE_LUT_16];
    MI_U32 u32SpeedY[MI_ISP_AE_LUT_16];
} MI_ISP_AE_ConvSpeedParam_EX_t;

typedef struct MI_ISP_AE_ConvThdParam_s
{
    MI_U32 u32InThd;
    MI_U32 u32OutThd;
} MI_ISP_AE_ConvThdParam_t;

typedef struct MI_ISP_AE_ConvConditonType_s
{
    MI_ISP_AE_ConvThdParam_t   stConvThrd;
    MI_ISP_AE_ConvSpeedParam_t stConvSpeed;
} MI_ISP_AE_ConvConditonType_t;

typedef struct MI_ISP_AE_ConvSpeed_EX_s
{
    MI_ISP_AE_Bool_e              bEnable;
    MI_ISP_AE_ConvSpeedParam_EX_t stConvSpeedEX;
} MI_ISP_AE_ConvSpeed_EX_t;

typedef struct MI_ISP_AE_ExpoLimitType_s
{
    MI_U32 u32MinShutterUS;
    MI_U32 u32MaxShutterUS;
    MI_U32 u32MinFNx10;
    MI_U32 u32MaxFNx10;
    MI_U32 u32MinSensorGain;
    MI_U32 u32MinISPGain;
    MI_U32 u32MaxSensorGain;
    MI_U32 u32MaxISPGain;
} MI_ISP_AE_ExpoLimitType_t;

typedef struct MI_ISP_AE_ExpoPointParam_s
{
    MI_U32 u32FNumx10;
    MI_U32 u32Shutter;
    MI_U32 u32TotalGain;
    MI_U32 u32SensorGain;
} MI_ISP_AE_ExpoPointParam_t;

typedef struct MI_ISP_AE_ExpoTableType_s
{
    MI_U32                     u32NumPoints;
    MI_ISP_AE_ExpoPointParam_t stExpoTbl[MI_ISP_AE_LUT_16]; // LV from High to Low
} MI_ISP_AE_ExpoTableType_t;

typedef struct MI_ISP_AE_WinWeightParam_s
{
    MI_U8 u8AverageTbl[MI_ISP_AE_LUT_32 * MI_ISP_AE_LUT_32];
    MI_U8 u8CenterTbl[MI_ISP_AE_LUT_32 * MI_ISP_AE_LUT_32];
    MI_U8 u8SpotTbl[MI_ISP_AE_LUT_32 * MI_ISP_AE_LUT_32];
} MI_ISP_AE_WinWeightParam_t;

typedef struct MI_ISP_AE_WinWeightType_s
{
    MI_ISP_AE_WinWeightModeType_e eTypeID;
    MI_ISP_AE_WinWeightParam_t    stParaAPI;
} MI_ISP_AE_WinWeightType_t;

typedef struct MI_ISP_AE_StrategyType_s
{
    MI_ISP_AE_StrategyType_e eAEStrategyMode;
    MI_U32                   u32Weighting;
    MI_ISP_AE_IntpLutType_t  stUpperOffset;
    MI_ISP_AE_IntpLutType_t  stLowerOffset;
    MI_U32                   u32BrightToneStrength;
    MI_U32                   u32BrightToneSensitivity;
    MI_U32                   u32DarkToneStrength;
    MI_U32                   u32DarkToneSensitivity;
    MI_U32                   u32AutoStrength;
    MI_U32                   u32AutoSensitivity;
} MI_ISP_AE_StrategyType_t;

typedef enum __attribute__((aligned(4)))
{
    E_SS_AE_STRATEGY_EX_BRIGHTTONE,
    E_SS_AE_STRATEGY_EX_DARKTONE,
    E_SS_AE_STRATEGY_EX_HDRAUTO,
} MI_ISP_AE_StrategyExPriority_e;

typedef enum __attribute__((aligned(4)))
{
    E_SS_AE_STRATEGY_EX_COUNT,
    E_SS_AE_STRATEGY_EX_TARGET,
} MI_ISP_AE_StrategyExMode_e;

typedef struct MI_ISP_AE_StrategyExType_s
{
    MI_ISP_AE_Bool_e               bEnable;
    MI_ISP_AE_StrategyExMode_e     eMode;
    MI_ISP_AE_StrategyExPriority_e ePriority;
    MI_U16                         u16BT_NodeNum;
    MI_S32                         u32BT_NodeBV[MI_ISP_AE_LUT_16];
    MI_U32                         u32BT_ThdY[MI_ISP_AE_LUT_16];
    MI_U32                         u32BT_Percentx10[MI_ISP_AE_LUT_16];
    MI_U32                         u32BT_TargetYx10[MI_ISP_AE_LUT_16];
    MI_U32                         u32BT_MaxOffsetDown[MI_ISP_AE_LUT_16];
    MI_U16                         u16DT_NodeNum;
    MI_S32                         u32DT_NodeBV[MI_ISP_AE_LUT_16];
    MI_U32                         u32DT_ThdY[MI_ISP_AE_LUT_16];
    MI_U32                         u32DT_Percentx10[MI_ISP_AE_LUT_16];
    MI_U32                         u32DT_TargetYx10[MI_ISP_AE_LUT_16];
    MI_U32                         u32DT_MaxOffsetUp[MI_ISP_AE_LUT_16];
} MI_ISP_AE_StrategyExType_t;

typedef struct MI_ISP_AE_StrategyExAdvType_s
{
    MI_S16 u16WeightRatio1;
    MI_S16 u16WeightRatio2;
} MI_ISP_AE_StrategyExAdvType_t;

typedef struct MI_ISP_AE_StrategyExInfoType_s
{
    MI_U16 u16GMBlendRatio;
    MI_U16 u16UpperLimitTargetx10;
    MI_U16 u16LowerLimitTargetx10;
    MI_U16 u16BTCntPcntx10;
    MI_U16 u16DTCntPcntx10;
    MI_U16 u16BTYx10;
    MI_U16 u16DTYx10;
} MI_ISP_AE_StrategyExInfoType_t;

typedef struct MI_ISP_AE_RgbirAeType_s
{
    MI_ISP_AE_Bool_e bEnable;
    MI_U16           u16MaxYWithIR;
    MI_U16           u16MinISPGainCompRatio;
} MI_ISP_AE_RgbirAeType_t;

typedef struct MI_ISP_AE_HdrType_s
{
    MI_ISP_AE_IntpLutType_t stAeHdrRatio;
} MI_ISP_AE_HdrType_t;

typedef struct MI_ISP_AE_StabilizerExType_s
{
    MI_U8           u8StableCnt;
    MI_U8           u8StableTh;
    MI_U8           u8DetectMotionSensitivity;
} MI_ISP_AE_StabilizerExType_t;

typedef struct MI_ISP_AE_StabilizerType_s
{
    MI_ISP_AE_Bool_e bEnable;
    MI_U16           u16DiffThd;
    MI_U16           u16Percent;
} MI_ISP_AE_StabilizerType_t;

typedef struct MI_ISP_AE_DaynightInfoType_s
{
    MI_ISP_AE_Bool_e bD2N;
    MI_ISP_AE_Bool_e bN2D;
    MI_U32           u32N2D_VsbLtScore;
} MI_ISP_AE_DaynightInfoType_t;

typedef struct MI_ISP_AE_DaynightDetectionType_s
{
    MI_ISP_AE_Bool_e bEnable;
    MI_S32           s32D2N_BvThd;
    MI_U32           u32N2D_VsbLtScoreThd;
} MI_ISP_AE_DaynightDetectionType_t;

typedef struct MI_ISP_AE_PowerLineType_s
{
    MI_ISP_AE_Bool_e bEnable;
    MI_U16           u16Ratio;
} MI_ISP_AE_PowerLineType_t;

typedef struct MI_ISP_AE_LumaWgtType_s
{
    MI_ISP_AE_Bool_e        bEnable;
    MI_ISP_AE_IntpLutType_t WeightLuma;
    MI_ISP_AE_IntpLutType_t WeightSatCnt;
    MI_U16                  u2SatCntThd;
} MI_ISP_AE_LumaWgtType_t;

typedef struct MI_ISP_AE_HdrDynamicRatioType_s
{
    MI_ISP_AE_OpType_e      enOpType;               // M_AUTO ~ (M_MODMAX-1)
    MI_ISP_AE_IntpLutType_t ExpoRatio;              // 1024 ~ 524288
    MI_ISP_AE_IntpLutType_t ExpoRatioOffset;        // 1024 ~ 524288
    MI_U16                  u2ExpoRatioSensitivity; //    1 ~     99
    MI_U16                  u2ExpoRatioTolerance;   //    1 ~    100
    MI_U16                  u2ExpoRatio_NodeNum;
    MI_U32                  u4ExpoRatio_Min[MI_ISP_AE_LUT_16];
    MI_U32                  u4ExpoRatio_Max[MI_ISP_AE_LUT_16];
    MI_S32                  u4ExpoRatio_NodeBV[MI_ISP_AE_LUT_16];
    MI_ISP_AE_OpType_e      enOpTypeShortExpTab;    // M_AUTO ~ (M_MODMAX-1)
    MI_U32                  u4ShortShutterMax;
} MI_ISP_AE_HdrDynamicRatioType_t;

typedef struct MI_ISP_AE_ExpoTableMode_s
{
    MI_ISP_AE_OpType_e      enOpType;               // M_AUTO ~ (M_MODMAX-1)
} MI_ISP_AE_ExpoTableMode_t;

typedef struct MI_ISP_AE_QueryHdrDynamicRatioInfoType_s
{
    MI_U32                  u4ExpoRatio[E_SS_AE_HDR_COMB_MODE_MAX];        // 1024 ~ 524288
    MI_ISP_AE_Bool_e        IsHDRRatioChange[E_SS_AE_HDR_COMB_MODE_MAX];   // 0 ~ 1
} MI_ISP_AE_QueryHdrDynamicRatioInfoType_t;

typedef struct MI_ISP_AE_PowerLineInfoType_s
{
    MI_S16 s2Direction;
    MI_U16 u2IsEffective;
    MI_U32 u4PreShutter;
} MI_ISP_AE_PowerLineInfoType_t;

typedef struct MI_ISP_AE_VerInfoType_s
{
    MI_U32  u32ReleaseDate;
    MI_U32  u32ReportID;
    MI_U8   u8Major;
    MI_U8   u8Minor;
    MI_U8   u8TestVer;
    MI_U8   u8Reserve;
} MI_ISP_AE_VerInfoType_t;

typedef enum __attribute__((aligned(4)))
{
    E_SS_AE_SM_BACKLIGHT      = 0,
    E_SS_AE_SM_FRONTLIGHTFACE = 1,
    E_SS_AE_SM_MIXLIGHT       = 2,
    E_SS_AE_SM_NUM
} MI_ISP_AE_Scene_e;

typedef enum __attribute__((aligned(4)))
{
    E_SS_AE_SM_ADJ_BYPASS      = 0,
    E_SS_AE_SM_ADJ_EXPOSURE    = 1,
    E_SS_AE_SM_ADJ_NUM
} MI_ISP_AE_SceneAdj_e;

typedef struct MI_ISP_AE_SceneAdjType_s
{
    MI_ISP_AE_Bool_e bEnable;
    MI_U16           u16ConvSpeed;
    MI_U16           u16AdjItem[MI_ISP_AE_SCENE_NUM][MI_ISP_AE_SCENE_ADJ_NUM];
    MI_U16           u16Level[MI_ISP_AE_SCENE_NUM][MI_ISP_AE_SCENE_ADJ_NUM];
} MI_ISP_AE_SceneAdjType_t;

typedef struct MI_ISP_AE_SceneIndType_s
{
    MI_U16 u16Flag[MI_ISP_AE_SCENE_NUM];
} MI_ISP_AE_SceneIndType_t;

typedef struct MI_ISP_AE_SceneCurLevelType_s
{
    MI_U16 u16Level[MI_ISP_AE_SCENE_PARAM_NUM];
} MI_ISP_AE_SceneCurLevelType_t;

typedef struct MI_ISP_AE_FDCorParam_s
{
    MI_U16 u16MinX;
    MI_U16 u16MinY;
    MI_U16 u16MaxX;
    MI_U16 u16MaxY;
    MI_U8  u8Brightness;
    MI_U32 u32Area;
} MI_ISP_AE_FDCorParam_t;

typedef struct MI_ISP_AE_FDInfoType_s
{
    MI_U16                  u16FDNum;
    MI_U32                  u32FDInitTarget;
    MI_ISP_AE_FDCorParam_t  stFDCor[4];
} MI_ISP_AE_FDInfoType_t;

typedef struct MI_ISP_AE_FDParamType_s
{
    MI_ISP_AE_Bool_e  bEnable;
    MI_U8             u8FacetargetY;
    MI_U8             u8Tolerance;
    MI_U8             u8Speed;
    MI_U8             u8FaceChangePcnt;
    MI_U8             u8EnvChangePcnt;
    MI_U16            u16Upper;
    MI_U16            u16Lower;
} MI_ISP_AE_FDParamType_t;

typedef struct MI_ISP_AE_FDParamEXType_s
{
    MI_U16 u16DetectEnvChangeRatio;
    MI_U16 u16DetectOverExposureDiff;
    MI_U16 u16DetectBlackDiff;
    MI_U16 u16ConvSpeedX[4];
    MI_U16 u16ConvSpeedY[4];
    MI_U16 u16UnStableArea[4];
    MI_U8  u8StableXTh_UnStableArea;
    MI_U8  u8StableYTh_UnStableArea;
    MI_U8  u8StableXTh;
    MI_U8  u8StableYTh;
    MI_U8  u8StableAreaTh;
    MI_U8  u8StableCnt;
    MI_U8  u8FaceToNoFaceSpeed;
    MI_U8  u8CompensationArea;
    MI_U8  u8CompensationRatio;
    MI_U8  u8ExcludeFaceRatio;
} MI_ISP_AE_FDParamEXType_t;

typedef struct MI_ISP_AE_FDEXInfoType_s
{
    MI_U8                   uIsFaceEffective;
    MI_U8                   uIsReachFDMinMaxBoundary;
    MI_U8                   uIsAFStable;
    MI_U16                  u16FDNum;
    MI_ISP_AE_FDCorParam_t  stFDCor[4];
} MI_ISP_AE_FDEXInfoType_t;

typedef struct MI_ISP_AE_FDEXParamType_s
{
    MI_ISP_AE_Bool_e    bEnable;
    MI_ISP_AE_Bool_e    bEnDbgMsg;
    MI_ISP_AE_Bool_e    bIgnoreAFStable;
    MI_U8               u8FDCalCycleNum;
    MI_U16              u16FDStableCountThr;
    MI_U16              u16FDUnstableCountThr;
    MI_U16              u16FDFastTriggerCountThr;
    MI_U16              u16FDLumaNodeNum;
    MI_S32              u32FDLumaTarget[MI_ISP_AE_LUT_16];
    MI_S32              u32FDLumaTolerance[MI_ISP_AE_LUT_16];
    MI_S32              u32FDSceneTargetMin[MI_ISP_AE_LUT_16];
    MI_S32              u32FDSceneTargetMax[MI_ISP_AE_LUT_16];
    MI_S32              u32FDLumaNodeBV[MI_ISP_AE_LUT_16];
    MI_U16              u16FDConvRatioNum;
    MI_S32              u32FDConvRatioX[MI_ISP_AE_LUT_16];
    MI_S32              u32FDConvRatioY[MI_ISP_AE_LUT_16];
} MI_ISP_AE_FDEXParamType_t;

#endif //_MI_ISP_AE_DATATYPE_H_
