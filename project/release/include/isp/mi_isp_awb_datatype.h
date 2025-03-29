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

#ifndef _MI_ISP_AWB_DATATYPE_H_
#define _MI_ISP_AWB_DATATYPE_H_

#ifdef __USE_USERSPACE_3A__
#include "mi_common.h"
#endif

#define MI_ISP_AWB_LV_CT_TBL_NUM  18
#define MI_ISP_AWB_CT_TBL_NUM     10
#define MI_ISP_AWB_WEIGHT_WIN_NUM 81
#define MI_ISP_AWB_EX_LIGHT_CNT   4
#define MI_ISP_AWB_MULTILS_CNT    9
#define MI_ISP_AWB_CT_CALI_CNT    40
#define MI_ISP_AWB_STAT_BLOCK     128 * 90
#define MI_ISP_AWB_CHN_CNT        3
#define MI_ISP_AWB_WEIGHT_WIN_NUM        81
#define MI_ISP_AWB_SPECIAL_CASE_NUM      4
#define MI_ISP_AWB_SPECIAL_ZONE_NUM      8
#define MI_ISP_AWB_SPECIAL_NODE_NUM      4
#define MI_ISP_AWB_STATISFILTER_NODE_NUM 4
#define MI_ISP_AWB_STATISNR_NODE_NUM     4
#define MI_ISP_AWB_AIAWB_ADJ_NUM         16
#define MI_ISP_AWB_MAX_AI_STATIS_NUM     28
#define MI_ISP_FDAWB_SKIN_NUM            8
#define MI_ISP_FDAWB_Gray_NUM            10
#define MI_ISP_FDAWB_NODE_NUM            4
#define MI_ISP_AWB_SCENE_NUM             16
#define MI_ISP_AWB_SCENE_ADJ_NUM         8
#define MI_ISP_AWB_SCENE_PARAM_NUM       16

typedef enum __attribute__((aligned(4)))
{
    E_SS_AWB_FALSE = 0,
    E_SS_AWB_TRUE  = !E_SS_AWB_FALSE,
    E_SS_AWB_BOOL_MAX
} MI_ISP_AWB_Bool_e;

typedef enum __attribute__((aligned(4)))
{
    E_SS_AWB_OP_TYP_AUTO   = 0,
    E_SS_AWB_OP_TYP_MANUAL = !E_SS_AWB_OP_TYP_AUTO,
    E_SS_AWB_OP_TYP_MODE_MAX
} MI_ISP_AWB_OpType_e;

typedef enum __attribute__((aligned(4)))
{
    E_SS_AWB_STATE_NORMAL = 0,
    E_SS_AWB_STATE_PAUSE  = 1,
    E_SS_AWB_STATE_MAX
} MI_ISP_AWB_SmStateType_e;

typedef struct MI_ISP_AWB_QueryInfoType_s
{
    MI_ISP_AWB_Bool_e bIsStable;
    MI_U16            u16Rgain;
    MI_U16            u16Grgain;
    MI_U16            u16Gbgain;
    MI_U16            u16Bgain;
    MI_U16            u16ColorTemp;
    MI_U8             u8WPInd;
    MI_ISP_AWB_Bool_e bMultiLSDetected;
    MI_U8             u8FirstLSInd;
    MI_U8             u8SecondLSInd;
} MI_ISP_AWB_QueryInfoType_t;

typedef enum __attribute__((aligned(4)))
{
    E_SS_AWB_MODE_AUTO,
    E_SS_AWB_MODE_MANUAL,
    E_SS_AWB_MODE_CTMANUAL,
    E_SS_AWB_MODE_MAX
} MI_ISP_AWB_ModeType_e;

typedef enum __attribute__((aligned(4)))
{
    E_SS_AWB_ALG_GRAYWORLD = 0,
    E_SS_AWB_ALG_NORMAL    = 1,
    E_SS_AWB_ALG_BALANCE   = 2,
    E_SS_AWB_ALG_FOCUS     = 3,
    E_SS_AWB_ALG_MAX       = 0xffffffff
} MI_ISP_AWB_AlgoType_e;

typedef enum __attribute__((aligned(4)))
{
    E_SS_AWB_ADV_DEFAULT = 0,
    E_SS_AWB_ADV_ADVANCE = 1,
    E_SS_AWB_ADV_MAX     = 0xffffffff
} MI_ISP_AWB_AdvType_e;

typedef struct MI_ISP_AWB_CtLimitParam_s
{
    MI_U16 u16MaxRgain; // RW, Maximum  RGain, Range: [0, 8191]
    MI_U16 u16MinRgain; // RW, Miniimum RGain, Range: [0, 8191]
    MI_U16 u16MaxBgain; // RW, Maximum  BGain, Range: [0, 8191]
    MI_U16 u16MinBgain; // RW, Miniimum BGain, Range: [0, 8191]
} MI_ISP_AWB_CtLimitParam_t;

typedef struct MI_ISP_AWB_CtWeightParam_s
{
    MI_U16 u16Weight[MI_ISP_AWB_CT_TBL_NUM]; // RW, Light CT Weight, Range: [1, 255]
} MI_ISP_AWB_CtWeightParam_t;

typedef struct MI_ISP_AWB_CtRatioParam_s
{
    MI_U16 u16Ratio[MI_ISP_AWB_CT_TBL_NUM]; // RW, CT Prefer Ratio, Range: [1, 255]
} MI_ISP_AWB_CtRatioParam_t;

typedef struct MI_ISP_AWB_AttrParam_s
{
    MI_U8                      u8Speed;      // RW, AWB converging speed, Range: [0x1, 0x64]
    MI_U8                      u8ConvInThd;  // RW, AWB converging threshold, Range:[0, 255], Recommended: [32]
    MI_U8                      u8ConvOutThd; // RW, AWB converging threshold, Range:[0, 255], Recommended: [64]
    MI_ISP_AWB_AlgoType_e      eAlgType;     // RW, AWB 0:GrayWorld, 1:Normal, 2:Balance 3,Focus
    MI_ISP_AWB_AdvType_e       eAdvType;     // RW, AWB advance mode type
    MI_U8                      u8RGStrength; // RW, AWB adjust RG ratio, Range:[0, 255]
    MI_U8                      u8BGStrength; // RW, AWB adjust BG ratio, Range:[0, 255]
    MI_ISP_AWB_CtLimitParam_t  stCTLimit;    // RW, AWB limitation when envirnoment ct is out of boundary
    MI_ISP_AWB_CtWeightParam_t stLvWeight[MI_ISP_AWB_LV_CT_TBL_NUM];     // RW, AWB Lv Ct Weight, Range: [0, 255]
    MI_ISP_AWB_CtRatioParam_t  stPreferRRatio[MI_ISP_AWB_LV_CT_TBL_NUM]; // RW, AWB prefer R gain, Range: [0, 255]
    MI_ISP_AWB_CtRatioParam_t  stPreferBRatio[MI_ISP_AWB_LV_CT_TBL_NUM]; // RW, AWB prefer B gain, Range: [0, 255]
    MI_U16                     u16WpWeight[MI_ISP_AWB_CT_TBL_NUM];
    MI_U32                     u32WeightWin[MI_ISP_AWB_WEIGHT_WIN_NUM]; // RW, AWB region gain, Range: [0, 16]
} MI_ISP_AWB_AttrParam_t;

typedef struct MI_ISP_AWB_MwbAttrParam_s
{
    MI_U16 u16Rgain;  // RW, Multiplier for R  color channel, Range: [0, 0x2000]
    MI_U16 u16Grgain; // RW, Multiplier for Gr color channel, Range: [0, 0x2000]
    MI_U16 u16Gbgain; // RW, Multiplier for Gb color channel, Range: [0, 0x2000]
    MI_U16 u16Bgain;  // RW, Multiplier for B  color channel, Range: [0, 0x2000]
} MI_ISP_AWB_MwbAttrParam_t;

typedef struct MI_ISP_AWB_AttrType_s
{
    MI_ISP_AWB_SmStateType_e  eState;
    MI_ISP_AWB_ModeType_e     eOpType;
    MI_ISP_AWB_MwbAttrParam_t stManualParaAPI;
    MI_ISP_AWB_AttrParam_t    stAutoParaAPI;
} MI_ISP_AWB_AttrType_t;

typedef struct MI_ISP_AWB_AttrExtraLightsourceParam_s
{
    MI_U16            u16WhiteRgain; // RW, RGain of white Point Location , Range: [256, 4095]
    MI_U16            u16WhiteBgain; // RW, RGain of white Point Location , Range: [256, 4095]
    MI_U8             u8AreaSize;    // RW, Light Area Size , Range: [1, 32]
    MI_ISP_AWB_Bool_e bExclude;      // RW, Include or exclude Uaer light Area, 0: include, 1:exclude
} MI_ISP_AWB_AttrExtraLightsourceParam_t;

typedef struct MI_ISP_AWB_AttrExType_s
{
    MI_ISP_AWB_Bool_e                      bExtraLightEn;
    MI_ISP_AWB_AttrExtraLightsourceParam_t stLightInfo[MI_ISP_AWB_EX_LIGHT_CNT];
} MI_ISP_AWB_AttrExType_t;

typedef struct MI_ISP_AWB_MultilsLsType_s
{
    MI_ISP_AWB_Bool_e bEnable;
    MI_U8             u8Sensitive;
    MI_U8             u8CaliStrength;
    MI_U16            u16CcmForLow[MI_ISP_AWB_MULTILS_CNT];
    MI_U16            u16CcmForHigh[MI_ISP_AWB_MULTILS_CNT];
} MI_ISP_AWB_MultiLsType_t;

typedef struct MI_ISP_AWB_CtWeightType_s
{
    MI_U16                     u16LvIndex;
    MI_ISP_AWB_CtWeightParam_t stParaAPI;
} MI_ISP_AWB_CtWeightType_t;

typedef struct MI_ISP_AWB_CtCaliType_s
{
    MI_U16 u16StartIdx;                         // RW, Light area start index, Range: [0, u2EndIdx]
    MI_U16 u16EndIdx;                           // RW, Light area end index, Range: [u2StartIdx, 9]
    MI_U16 u16CtParams[MI_ISP_AWB_CT_CALI_CNT]; // RW, Color temperature of calibration paramters , Range: [1, 1000]
} MI_ISP_AWB_CtCaliType_t;

typedef struct MI_ISP_AWB_CurCtCaliType_s
{
    MI_U16 u16CtParams[MI_ISP_AWB_CT_CALI_CNT]; // RW, Color temperature of calibration paramters , Range: [1, 1000]
} MI_ISP_AWB_CurCtCaliType_t;

typedef struct MI_ISP_AWB_CtStatisticsType_s
{
    MI_U16 u16Width;                              // RW, Effective range
    MI_U16 u16Height;                             // RW, Effective range
    MI_U16 u16StatisX[MI_ISP_AWB_STAT_BLOCK / 2]; // RW, Color Temperature Curve Domain Statistics X, max is 64x90
    MI_U16 u16StatisY[MI_ISP_AWB_STAT_BLOCK / 2]; // RW, Color Temperature Curve Domain Statistics Y, max is 64x90
} MI_ISP_AWB_CtStatisticsType_t;

typedef struct MI_ISP_AWB_HwStatisticsType_s
{
    MI_U8 u8AwbBuffer[MI_ISP_AWB_STAT_BLOCK * MI_ISP_AWB_CHN_CNT]; // 128 * 90 * 3
} MI_ISP_AWB_HwStatisticsType_t;

typedef struct MI_ISP_AWB_CtmwbParam_s
{
    MI_U32 u32CT;
} MI_ISP_AWB_CtmwbParam_t;

typedef struct MI_ISP_AWB_StabilizerType_s
{
    MI_ISP_AWB_Bool_e bEnable;
    MI_U16            u16GlbGainThd;
    MI_U16            u16CountThd;
    MI_U16            u16ForceTriGainThd;
} MI_ISP_AWB_StabilizerType_t;

typedef struct MI_ISP_AWB_DaynightInfoType_s
{
    MI_ISP_AWB_Bool_e bD2N;
    MI_ISP_AWB_Bool_e bN2D;
    MI_U32            u32N2D_VsbLtScore;
} MI_ISP_AWB_DaynightInfoType_t;

typedef struct MI_ISP_AWB_DaynightDetectionType_s
{
    MI_ISP_AWB_Bool_e bEnable;
    MI_S32            s32D2N_BvThd;
    MI_U32            u32N2D_VsbLtScoreThd;
} MI_ISP_AWB_DaynightDetectionType_t;

typedef enum __attribute__ ((aligned (4)))
{
    E_SS_AWB_SPECIAL_WEIGHTCTRL  = 0,
    E_SS_AWB_SPECIAL_PREFER_CT   = 1,
    E_SS_AWB_SPECIAL_PREFER_GAIN = 2
} MI_ISP_AWB_SpecialMode_e;

typedef struct MI_ISP_AWB_SpecialCaseParam_s
{
    MI_U8  u8Group1_ZoneNum;
    MI_U16 u16Group1_CenterX[MI_ISP_AWB_SPECIAL_ZONE_NUM];
    MI_U16 u16Group1_CenterY[MI_ISP_AWB_SPECIAL_ZONE_NUM];
    MI_U8  u8Group1_Radius[MI_ISP_AWB_SPECIAL_ZONE_NUM];
    MI_U8  u8Group1_CntLutNodeNum;
    MI_S32 s32Group1_CntLutX[MI_ISP_AWB_SPECIAL_NODE_NUM];
    MI_S32 s32Group1_CntLutY[MI_ISP_AWB_SPECIAL_NODE_NUM];
    MI_U8  u8Group2_ZoneNum;
    MI_U16 u16Group2_CenterX[MI_ISP_AWB_SPECIAL_ZONE_NUM];
    MI_U16 u16Group2_CenterY[MI_ISP_AWB_SPECIAL_ZONE_NUM];
    MI_U8  u8Group2_Radius[MI_ISP_AWB_SPECIAL_ZONE_NUM];
    MI_U8  u8Group2_CntLutNodeNum;
    MI_S32 s32Group2_CntLutX[MI_ISP_AWB_SPECIAL_NODE_NUM];
    MI_S32 s32Group2_CntLutY[MI_ISP_AWB_SPECIAL_NODE_NUM];
    MI_U8  u8BvLutNodeNum;
    MI_S32 s32BvLutX[MI_ISP_AWB_SPECIAL_NODE_NUM];
    MI_S32 s32BvLutY[MI_ISP_AWB_SPECIAL_NODE_NUM];
    MI_ISP_AWB_SpecialMode_e eMode;
    MI_U16 u16Weight[MI_ISP_AWB_CT_TBL_NUM];
    MI_U32 u32PreferCT;
    MI_U16 u16PreferRgain;
    MI_U16 u16PreferBgain;
} MI_ISP_AWB_SpecialCaseParam_t;

typedef struct MI_ISP_AWB_SpecialCaseType_s
{
    MI_U32 u32CaseNum;
    MI_ISP_AWB_SpecialCaseParam_t  stCase[MI_ISP_AWB_SPECIAL_CASE_NUM];
} MI_ISP_AWB_SpecialCaseType_t;

typedef struct MI_ISP_AWB_SpecialCaseInfoType_s
{
    MI_U32 u32Group1Cnt[MI_ISP_AWB_SPECIAL_CASE_NUM];
    MI_U32 u32Group1CntRatio[MI_ISP_AWB_SPECIAL_CASE_NUM];
    MI_U32 u32Group2Cnt[MI_ISP_AWB_SPECIAL_CASE_NUM];
    MI_U32 u32Group2CntRatio[MI_ISP_AWB_SPECIAL_CASE_NUM];
    MI_U32 u32BvRatio[MI_ISP_AWB_SPECIAL_CASE_NUM];
    MI_U32 u32CaseRatio[MI_ISP_AWB_SPECIAL_CASE_NUM];
    MI_U16 u16CaseWeight[MI_ISP_AWB_SPECIAL_CASE_NUM][MI_ISP_AWB_CT_TBL_NUM];
    MI_U16 u16SpecialWeight[MI_ISP_AWB_CT_TBL_NUM];
    MI_U16 u16SpecialRgain[MI_ISP_AWB_SPECIAL_CASE_NUM];
    MI_U16 u16SpecialBgain[MI_ISP_AWB_SPECIAL_CASE_NUM];
} MI_ISP_AWB_SpecialCaseInfoType_t;

typedef struct MI_ISP_AWB_StatisFilterType_s
{
    MI_U32 u32NodeNum;
    MI_S32 s32LutX_BV[MI_ISP_AWB_STATISFILTER_NODE_NUM];
    MI_S32 s32LutY_HighThd[MI_ISP_AWB_STATISFILTER_NODE_NUM];
    MI_S32 s32LutY_LowThd[MI_ISP_AWB_STATISFILTER_NODE_NUM];
} MI_ISP_AWB_StatisFilterType_t;

typedef struct MI_ISP_AWB_VerInfoType_s
{
    MI_U32  u32ReleaseDate;
    MI_U32  u32ReportID;
    MI_U8   u8Major;
    MI_U8   u8Minor;
    MI_U8   u8TestVer;
    MI_U8   u8Reserve;
} MI_ISP_AWB_VerInfoType_t;

typedef struct MI_ISP_AWB_StatisNrType_s
{
    MI_U32 u4NodeNum;
    MI_S32 s4LutX_BV[MI_ISP_AWB_STATISNR_NODE_NUM];
    MI_S32 s4LutY_SfLvl[MI_ISP_AWB_STATISNR_NODE_NUM]; //0~4
    MI_S32 s4LutY_TfLvl[MI_ISP_AWB_STATISNR_NODE_NUM]; //0~63
} MI_ISP_AWB_StatisNrType_t;

typedef struct MI_ISP_AWB_AIAwbStatisType_s
{
    MI_U32 u32R[MI_ISP_AWB_MAX_AI_STATIS_NUM];
    MI_U32 u32G[MI_ISP_AWB_MAX_AI_STATIS_NUM];
    MI_U32 u32B[MI_ISP_AWB_MAX_AI_STATIS_NUM];
} MI_ISP_AWB_AIAwbStatisType_t;

typedef struct MI_ISP_AWB_AIAwbAdjType_s
{
    MI_U32 u32NodeNum;
    MI_S32 s32LutX_BV[MI_ISP_AWB_AIAWB_ADJ_NUM];
    MI_S32 s32LutY_AIAwbWeight[MI_ISP_AWB_AIAWB_ADJ_NUM];
} MI_ISP_AWB_AIAwbAdjType_t;

typedef struct MI_ISP_AWB_AIAwbInfoType_s
{
    MI_U16 u2CurWeight; //0~64
    MI_U16 u16NormalRgain;
    MI_U16 u16NormalGgain;
    MI_U16 u16NormalBgain;
    MI_U16 u16AIRgain;
    MI_U16 u16AIGgain;
    MI_U16 u16AIBgain;
} MI_ISP_AWB_AIAwbInfoType_t;

typedef struct MI_ISP_AWB_FdCorParam_s
{
    MI_U16 u16MinX;
    MI_U16 u16MinY;
    MI_U16 u16MaxX;
    MI_U16 u16MaxY;
    MI_U8  u8Brightness;
    MI_U16 u32Area;
} MI_ISP_AWB_FdCorParam_t;

typedef struct MI_ISP_AWB_FdInfoType_s
{
    MI_U16 u16FDNum;
    MI_ISP_AWB_FdCorParam_t stFDCor[4];
} MI_ISP_AWB_FdInfoType_t;

typedef enum __attribute__ ((aligned (4)))
{
    E_SS_FDAWB_REFER_TO_SKIN,
    E_SS_FDAWB_ASSIGN_SKIN_COLOR
} MI_ISP_AWB_FdAwb_Mode_e;

typedef struct MI_ISP_AWB_FdAwbParam_s
{
    MI_ISP_AWB_Bool_e bEnable;
    MI_ISP_AWB_FdAwb_Mode_e eMode;
    MI_U8  u8SkinAreaNum; //Range: 0 ~ 10
    MI_U8  u8SkinAreaCntInThd;
    MI_U16 u16SkinAreaCntOutThd;
    MI_U16 u16SkinAreaCT[MI_ISP_FDAWB_SKIN_NUM];
    MI_U8  u8SkinAreaCenterX[MI_ISP_FDAWB_SKIN_NUM];
    MI_U8  u8SkinAreaCenterY[MI_ISP_FDAWB_SKIN_NUM];
    MI_U8  u8SkinAreaRadius[MI_ISP_FDAWB_SKIN_NUM];
    MI_U8  u8SkinAreaSkipRadius[MI_ISP_FDAWB_SKIN_NUM];
    MI_U16 u16SkinUnStbCntThd;
    MI_U8  u8RefGrayNum[MI_ISP_FDAWB_SKIN_NUM];
    MI_U8  u8RefGrayCenterX[MI_ISP_FDAWB_SKIN_NUM][MI_ISP_FDAWB_Gray_NUM];
    MI_U8  u8RefGrayCenterY[MI_ISP_FDAWB_SKIN_NUM][MI_ISP_FDAWB_Gray_NUM];
    MI_U8  u8RefGrayRadius[MI_ISP_FDAWB_SKIN_NUM][MI_ISP_FDAWB_Gray_NUM];
    MI_U8  u8AsnSkinX[MI_ISP_FDAWB_SKIN_NUM];
    MI_U8  u8AsnSkinY[MI_ISP_FDAWB_SKIN_NUM];
    MI_U8  u8AsnSkinStbLvl;
    MI_ISP_AWB_Bool_e bFwstWbFromSkin;
    MI_U8  u8Period;
    MI_U16 u16FdRoiMin;
    MI_U8  u8NodeNum;//Range: 1~4
    MI_S32 s32Bv[MI_ISP_FDAWB_NODE_NUM];
    MI_S32 s32ConvSpeed[MI_ISP_FDAWB_NODE_NUM];
    MI_S32 s32ConvInThd[MI_ISP_FDAWB_NODE_NUM];
    MI_S32 s32ConvOutThd[MI_ISP_FDAWB_NODE_NUM];
    MI_S32 s32FdLumaDiffThd[MI_ISP_FDAWB_NODE_NUM];
    MI_S32 s32FdLumaStbCntThd[MI_ISP_FDAWB_NODE_NUM];
    MI_S32 s32FdRoiDiffThd[MI_ISP_FDAWB_NODE_NUM];
    MI_S32 s32FdRoiStbCntThd[MI_ISP_FDAWB_NODE_NUM];
} MI_ISP_AWB_FdAwbParam_t;

typedef struct MI_ISP_AWB_FdAwbInfoType_s
{
    MI_U8  u8FaceNum;
    MI_U8  u8FaceCor[4]; //MinX, MinY, MaxX, MaxY
    MI_ISP_AWB_Bool_e bEffective;
    MI_U32 u32EffectiveCnt;
    MI_U32 u32FaceAvgX;
    MI_U32 u32FaceAvgY;
    MI_U16 u16FaceSkinInd;
    MI_U32 u32FaceTarRgain;
    MI_U32 u32FaceTarBgain;
} MI_ISP_AWB_FdAwbInfoType_t;

typedef enum __attribute__ ((aligned (4)))
{
    E_SS_AWB_FWST_KEEP           = 0,
    E_SS_AWB_FWST_MIX_PREFERGAIN = 1,
    E_SS_AWB_FWST_MIX_GRAYWORLD  = 2,
} MI_ISP_AWB_Fwst_AlgoType_e;

typedef struct MI_ISP_AWB_FwstStrategyParam_s
{
    MI_ISP_AWB_Fwst_AlgoType_e eAlgType;
    MI_U32 u32CntThd;
    MI_U32 u32SmoothWidth;
    MI_U32 u32PreferCT;
} MI_ISP_AWB_FwstStrategyParam_t;

typedef enum __attribute__((aligned(4)))
{
    E_SS_AWB_SM_BACKLIGHT      = 1,
    E_SS_AWB_SM_FRONTLIGHTFACE = 0,
    E_SS_AWB_SM_MIXLIGHT       = 2,
    E_SS_AWB_SM_NUM
} MI_ISP_AWB_Scene_e;

typedef enum __attribute__((aligned(4)))
{
    E_SS_AWB_SM_ADJ_BYPASS      = 0,
    E_SS_AWB_SM_ADJ_WT20000K    = 1,
    E_SS_AWB_SM_ADJ_WT15000K    = 2,
    E_SS_AWB_SM_ADJ_WT10000K    = 3,
    E_SS_AWB_SM_ADJ_WT6500K     = 4,
    E_SS_AWB_SM_ADJ_WT5000K     = 5,
    E_SS_AWB_SM_ADJ_WT4000K     = 6,
    E_SS_AWB_SM_ADJ_WT3000K     = 7,
    E_SS_AWB_SM_ADJ_WT2300K     = 8,
    E_SS_AWB_SM_ADJ_WT1500K     = 9,
    E_SS_AWB_SM_ADJ_WT1000K     = 10,
    E_SS_AWB_SM_ADJ_NUM
} MI_ISP_AWB_SceneAdj_e;

typedef struct MI_ISP_AWB_SceneAdjType_s
{
    MI_ISP_AWB_Bool_e bEnable;
    MI_U16            u16ConvSpeed;
    MI_U16            u16AdjItem[MI_ISP_AWB_SCENE_NUM][MI_ISP_AWB_SCENE_ADJ_NUM];
    MI_U16            u16Level[MI_ISP_AWB_SCENE_NUM][MI_ISP_AWB_SCENE_ADJ_NUM];
} MI_ISP_AWB_SceneAdjType_t;

typedef struct MI_ISP_AWB_SceneIndType_s
{
    MI_U16 u16Flag[MI_ISP_AWB_SCENE_NUM];
} MI_ISP_AWB_SceneIndType_t;

typedef struct MI_ISP_AWB_SceneCurLevelType_s
{
    MI_U16 u16Level[MI_ISP_AWB_SCENE_PARAM_NUM];
} MI_ISP_AWB_SceneCurLevelType_t;

typedef struct MI_ISP_AWB_RunPeriodParam_s
{
    MI_U8 u8Period;
} MI_ISP_AWB_RunPeriodParam_t;

#endif //_MI_ISP_AWB_DATATYPE_H_
