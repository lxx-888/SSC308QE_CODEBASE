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

#ifndef __PTREE_SUR_LDC_H__
#define __PTREE_SUR_LDC_H__

#include "ptree_sur_sys.h"
#include "mi_ldc_datatype.h"

#define ST_MAX_PTREE_SUR_LDC_REGION_NUM LDC_MAX_REGION_NUM

typedef struct PTREE_SUR_LDC_LdcRegionSPara_s
{
    unsigned int cropMode;
    unsigned int pan;
    unsigned int tilt;
    unsigned int zoomV;
    unsigned int zoomH;
    unsigned int inRadius;
    unsigned int outRadius;
    unsigned int focalRatio;
    unsigned int distortionRatio;
    unsigned int outRot;
    unsigned int rot;
} PTREE_SUR_LDC_LdcRegionSPara_t;

typedef struct PTREE_SUR_LDC_Map2binPara_s
{
    unsigned int grid;
    char         mapX[16];
    char         mapY[16];
} PTREE_SUR_LDC_Map2binPara_t;

typedef struct PTREE_SUR_LDC_LdcRegion_s
{
    unsigned int                   regionMode;
    unsigned short                 x;
    unsigned short                 y;
    unsigned short                 width;
    unsigned short                 height;
    PTREE_SUR_LDC_LdcRegionSPara_t para;
    PTREE_SUR_LDC_Map2binPara_t    map2bin;
} PTREE_SUR_LDC_LdcRegion_t;

typedef struct PTREE_SUR_LDC_LdcModeCfg_s
{
    unsigned int              enBgColor;
    unsigned int              bgColor;
    unsigned int              mountMode;
    unsigned int              regionNum;
    unsigned int              centerXOff;
    unsigned int              centerYOff;
    unsigned int              fisheyeRadius;
    PTREE_SUR_LDC_LdcRegion_t region[ST_MAX_PTREE_SUR_LDC_REGION_NUM];
} PTREE_SUR_LDC_LdcModeCfg_t;

typedef struct PTREE_SUR_LDC_LutModeCfg_s
{
    unsigned short width;
    unsigned short height;
    char           tableX[16];
    char           tableY[16];
    char           tableW[16];
} PTREE_SUR_LDC_LutModeCfg_t;

typedef struct PTREE_SUR_LDC_DisModeCfg_s
{
    unsigned int disMode;
    unsigned int userSliceNum;
    unsigned int focalLengthX;
    unsigned int focalLengthY;
    unsigned int sceneType;
    unsigned int motionLevel;
    unsigned int cropRatio;
    unsigned int rotationMatrix[LDC_MAXTRIX_NUM];
} PTREE_SUR_LDC_DisModeCfg_t;

typedef struct PTREE_SUR_LDC_PmfModeCfg_s
{
    char pmfCoef[16];
} PTREE_SUR_LDC_PmfModeCfg_t;

typedef struct PTREE_SUR_LDC_StitchModeCfg_s
{
    unsigned int projType;
    int          distance;
} PTREE_SUR_LDC_StitchModeCfg_t;

typedef struct PTREE_SUR_LDC_NirModeCfg_s
{
    int distance;
} PTREE_SUR_LDC_NirModeCfg_t;

typedef struct PTREE_SUR_LDC_DpuModeCfg_s
{
    int distance;
} PTREE_SUR_LDC_DpuModeCfg_t;

typedef struct PTREE_SUR_LDC_LdcHorizontalModeCfg_s
{
    int distortionRatio;
} PTREE_SUR_LDC_LdcHorizontalModeCfg_t;

typedef struct PTREE_SUR_LDC_CalibInfo_s
{
    char         path[64];
    unsigned int len;
} PTREE_SUR_LDC_CalibInfo_t;

typedef struct PTREE_SUR_LDC_Info_s
{
    PTREE_SUR_SYS_Info_t                 base;
    unsigned int                         workMode;
    PTREE_SUR_LDC_CalibInfo_t            calib;
    PTREE_SUR_LDC_LdcModeCfg_t           ldcCfg;
    PTREE_SUR_LDC_LutModeCfg_t           lutCfg;
    PTREE_SUR_LDC_DisModeCfg_t           disCfg;
    PTREE_SUR_LDC_PmfModeCfg_t           pmfCfg;
    PTREE_SUR_LDC_StitchModeCfg_t        stitchCfg;
    PTREE_SUR_LDC_NirModeCfg_t           nirCfg;
    PTREE_SUR_LDC_DpuModeCfg_t           dpuCfg;
    PTREE_SUR_LDC_LdcHorizontalModeCfg_t ldcHorizontalCfg;
} PTREE_SUR_LDC_Info_t;

typedef struct PTREE_SUR_LDC_InInfo_s
{
    PTREE_SUR_SYS_InInfo_t base;
    unsigned short         width;
    unsigned short         height;
} PTREE_SUR_LDC_InInfo_t;

typedef struct PTREE_SUR_LDC_OutInfo_s
{
    PTREE_SUR_SYS_OutInfo_t base;
    unsigned short          width;
    unsigned short          height;
    unsigned int            videoType;
    unsigned int            videoFormat;
} PTREE_SUR_LDC_OutInfo_t;

#endif //__PTREE_MOD_LDC_H__
