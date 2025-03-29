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

#ifndef __PTREE_SUR_VENC_H__
#define __PTREE_SUR_VENC_H__

#include "ptree_sur_sys.h"
#include "ptree_packet.h"
#include "ptree_packet_video.h"
#include "mi_venc_datatype.h"

#define PTREE_SUR_VENC_DEV_NUM (16)

typedef struct PTREE_SUR_VENC_VbrCfg_s
{
    unsigned int u32Gop;
    unsigned int u32MinQp;
    unsigned int u32MaxQp;
    unsigned int u32BitRate;
} PTREE_SUR_VENC_VbrCfg_t;

typedef struct PTREE_SUR_VENC_CbrCfg_s
{
    unsigned int u32Gop;
    unsigned int u32BitRate;
} PTREE_SUR_VENC_CbrCfg_t;

typedef struct PTREE_SUR_VENC_FixQpCfg_s
{
    unsigned int u32Gop;
    unsigned int u32IQp;
    unsigned int u32PQp;
    unsigned int u32Qfactor;
} PTREE_SUR_VENC_FixQpCfg_t;

typedef struct PTREE_SUR_VENC_AvbrCfg_s
{
    unsigned int u32Gop;
    unsigned int u32MinQp;
    unsigned int u32MaxQp;
    unsigned int u32BitRate;
} PTREE_SUR_VENC_AvbrCfg_t;

typedef struct PTREE_SUR_VENC_Info_s
{
    PTREE_SUR_SYS_Info_t base;
    unsigned int         u32MaxWidth;
    unsigned int         u32MaxHeight;
    unsigned int         u32MaxStreamCnt;
    unsigned int         u32Width;
    unsigned int         u32Height;
    unsigned char        u8StreamCntEnable;
    unsigned int         u32RoiNum;
    union
    {
        PTREE_SUR_VENC_VbrCfg_t   stVbrCfg;
        PTREE_SUR_VENC_CbrCfg_t   stCbrCfg;
        PTREE_SUR_VENC_FixQpCfg_t stFixQpCfg;
        PTREE_SUR_VENC_AvbrCfg_t  stAvbrCfg;
    };
    unsigned int                  u32EncodeFps;
    unsigned int                  u32MultiSlice;
    unsigned int                  u32SliceRowCnt;
    enum PTREE_PACKET_VIDEO_Fmt_e pEncodeType;
    char                          pRcMode[16];
    unsigned char                 bYuvEnable;
    unsigned int                  u32YuvWidth;
    unsigned int                  u32YuvHeight;
} PTREE_SUR_VENC_Info_t;

#endif //__PTREE_MOD_VENC_H__
