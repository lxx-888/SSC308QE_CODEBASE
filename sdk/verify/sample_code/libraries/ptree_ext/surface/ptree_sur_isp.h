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

#ifndef __PTREE_SUR_ISP_H__
#define __PTREE_SUR_ISP_H__

#include "ptree_sur_sys.h"

#define ST_MAX_PTREE_SUR_ISP_TABLE_NUM (8)

typedef struct PTREE_SUR_ISP_TableParam_s
{
    unsigned char snrId;
    unsigned int  tableX;
    unsigned int  tableY;
    unsigned int  tableW;
    unsigned int  tableH;

} PTREE_SUR_ISP_TableParam_t;

typedef struct PTREE_SUR_ISP_ZoomParam_s
{
    unsigned char              u8FromEntryIndex;
    unsigned char              u8ToEntryIndex;
    unsigned char              u8TableNum;
    PTREE_SUR_ISP_TableParam_t stTable[ST_MAX_PTREE_SUR_ISP_TABLE_NUM];

} PTREE_SUR_ISP_ZoomParam_t;

typedef struct PTREE_SUR_ISP_SubChnIqParam_s
{
    unsigned char dev;
    unsigned char chn;
    char          apiFile[64];

} PTREE_SUR_ISP_SubChnIqParam_t;

typedef struct PTREE_SUR_ISP_EarlyInitParam_s
{
    unsigned int   u32Revision;
    unsigned short u16SnrEarlyFps;
    unsigned short u16SnrEarlyFlicker;
    unsigned int   u32SnrEarlyShutter;
    unsigned int   u32SnrEarlyGainX1024;
    unsigned int   u32SnrEarlyDGain;
    unsigned short u16SnrEarlyAwbRGain;
    unsigned short u16SnrEarlyAwbGGain;
    unsigned short u16SnrEarlyAwbBGain;
    /******will add all parameters as required******/
} PTREE_SUR_ISP_EarlyInitParam_t;

typedef struct PTREE_SUR_ISP_LdcParam_s
{
    unsigned int centerX;
    unsigned int centerY;
    unsigned int alpha;
    unsigned int beta;
    unsigned int cropL;
    unsigned int cropR;
} PTREE_SUR_ISP_LdcParam_t;

typedef struct PTREE_SUR_ISP_OverLapParam_s
{
    char chOverlap[16];
} PTREE_SUR_ISP_OverLapParam_t;

typedef struct PTREE_SUR_ISP_Info_s
{
    PTREE_SUR_SYS_Info_t           base;
    char                           apiBinpath[64];
    unsigned int                   u32HdrType;
    unsigned int                   u32HdrFusionType;
    unsigned int                   u32HdrExposureMask;
    unsigned int                   u32SnrMask;
    unsigned int                   u32Rotation;
    unsigned int                   u32level3dnr;
    unsigned int                   u32Sync3aType;
    unsigned int                   u32StitchMask;
    unsigned char                  u8Mirror;
    unsigned char                  u8Flip;
    unsigned char                  u8CustIqEn;
    unsigned char                  u8ZoomEn;
    unsigned char                  u8SubChnIqEn;
    unsigned char                  u8IspLdcEn;
    unsigned char                  u8MutichnEn;
    unsigned char                  u8IspOverlapEn;
    PTREE_SUR_ISP_EarlyInitParam_t stEarlyInitParam;
    PTREE_SUR_ISP_ZoomParam_t      stZoomParam;
    PTREE_SUR_ISP_SubChnIqParam_t  stSubChnIqParam;
    PTREE_SUR_ISP_LdcParam_t       stIspLdcParam;
    PTREE_SUR_ISP_OverLapParam_t   stOverLapParam;
} PTREE_SUR_ISP_Info_t;

typedef struct PTREE_SUR_ISP_InInfo_s
{
    PTREE_SUR_SYS_InInfo_t base;
    unsigned short         u16CropX;
    unsigned short         u16CropY;
    unsigned short         u16CropW;
    unsigned short         u16CropH;
} PTREE_SUR_ISP_InInfo_t;

typedef struct PTREE_SUR_ISP_OutInfo_s
{
    PTREE_SUR_SYS_OutInfo_t base;
    unsigned short          u16CropX;
    unsigned short          u16CropY;
    unsigned short          u16CropW;
    unsigned short          u16CropH;
    unsigned int            u32CompressMode;
    unsigned int            u32VideoType;
    unsigned int            u32VideoFormat;
} PTREE_SUR_ISP_OutInfo_t;

#endif //__PTREE_MOD_ISP_H__
