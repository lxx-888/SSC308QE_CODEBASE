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

#ifndef __PTREE_PACKET_RAW_H__
#define __PTREE_PACKET_RAW_H__

#include "ptree_enum.h"
#include "ptree_packet.h"

extern const char* gp_ptreePacketInfoTypeRaw;

#define PTREE_PACKET_RAW_PLANE_NUM (2)

typedef struct PTREE_PACKET_RAW_Info_s PTREE_PACKET_RAW_Info_t;
typedef struct PTREE_PACKET_RAW_Obj_s  PTREE_PACKET_RAW_Obj_t;

typedef struct PTREE_PACKET_RAW_RawInfo_s PTREE_PACKET_RAW_RawInfo_t;
typedef struct PTREE_PACKET_RAW_RawData_s PTREE_PACKET_RAW_RawData_t;

enum PTREE_PACKET_RAW_DataPrecision_e
{
    E_PTREE_PACKET_RAW_PRECISION_8BPP,
    E_PTREE_PACKET_RAW_PRECISION_10BPP,
    E_PTREE_PACKET_RAW_PRECISION_12BPP,
    E_PTREE_PACKET_RAW_PRECISION_14BPP,
    E_PTREE_PACKET_RAW_PRECISION_16BPP,
    E_PTREE_PACKET_RAW_PRECISION_MAX
};

enum PTREE_PACKET_RAW_BayerId_e
{
    E_PTREE_PACKET_RAW_PIXEL_BAYERID_RG,
    E_PTREE_PACKET_RAW_PIXEL_BAYERID_GR,
    E_PTREE_PACKET_RAW_PIXEL_BAYERID_BG,
    E_PTREE_PACKET_RAW_PIXEL_BAYERID_GB,
    E_PTREE_PACKET_RAW_PIXEL_RGBIR_R0,
    E_PTREE_PACKET_RAW_PIXEL_RGBIR_G0,
    E_PTREE_PACKET_RAW_PIXEL_RGBIR_B0,
    E_PTREE_PACKET_RAW_PIXEL_RGBIR_G1,
    E_PTREE_PACKET_RAW_PIXEL_RGBIR_G2,
    E_PTREE_PACKET_RAW_PIXEL_RGBIR_I0,
    E_PTREE_PACKET_RAW_PIXEL_RGBIR_G3,
    E_PTREE_PACKET_RAW_PIXEL_RGBIR_I1,
    E_PTREE_PACKET_RAW_PIXEL_BAYERID_MAX
};

enum PTREE_PACKET_RAW_VideoFmt_e
{
    E_PTREE_PACKET_RAW_FORMAT_YUV422_YUYV,
    E_PTREE_PACKET_RAW_FORMAT_YUV422_UYVY,
    E_PTREE_PACKET_RAW_FORMAT_YUV422_YVYU,
    E_PTREE_PACKET_RAW_FORMAT_YUV422_VYUY,
    E_PTREE_PACKET_RAW_FORMAT_YUV422SP,
    E_PTREE_PACKET_RAW_FORMAT_YUV420SP,
    E_PTREE_PACKET_RAW_FORMAT_YUV420SP_NV21,
    E_PTREE_PACKET_RAW_FORMAT_RGB888,
    E_PTREE_PACKET_RAW_FORMAT_BGR888,
    E_PTREE_PACKET_RAW_FORMAT_ARGB8888,
    E_PTREE_PACKET_RAW_FORMAT_ABGR8888,
    E_PTREE_PACKET_RAW_FORMAT_BGRA8888,
    E_PTREE_PACKET_RAW_FORMAT_RGB565,
    E_PTREE_PACKET_RAW_FORMAT_ARGB1555,
    E_PTREE_PACKET_RAW_FORMAT_ARGB4444,
    E_PTREE_PACKET_RAW_FORMAT_I2,
    E_PTREE_PACKET_RAW_FORMAT_I4,
    E_PTREE_PACKET_RAW_FORMAT_I8,
    E_PTREE_PACKET_RAW_FORMAT_BAYER_BASE,
    E_PTREE_PACKET_RAW_FORMAT_BAYER_NUM = E_PTREE_PACKET_RAW_FORMAT_BAYER_BASE
                                          + E_PTREE_PACKET_RAW_PIXEL_BAYERID_MAX * E_PTREE_PACKET_RAW_PRECISION_MAX - 1,
    E_PTREE_PACKET_RAW_FORMAT_MAX
};

#define PTREE_PACKET_RAW_TO_OTHER_RGB_BAYER_PIXEL(__pixel, __other_base) \
    ((__pixel)-E_PTREE_PACKET_RAW_FORMAT_BAYER_BASE + (__other_base))
#define PTREE_PACKET_RAW_FROM_RGB_BAYER_PIXEL(__bit_mode, __pixel) \
    (E_PTREE_PACKET_RAW_FORMAT_BAYER_BASE + (__bit_mode)*E_PTREE_PACKET_RAW_PRECISION_MAX + (__pixel))

struct PTREE_PACKET_RAW_RawInfo_s
{
    enum PTREE_PACKET_RAW_VideoFmt_e fmt;
    unsigned int                     width;
    unsigned int                     height;
};

struct PTREE_PACKET_RAW_RawData_s
{
    char*        data[PTREE_PACKET_RAW_PLANE_NUM];
    unsigned int stride[PTREE_PACKET_RAW_PLANE_NUM];
    unsigned int size[PTREE_PACKET_RAW_PLANE_NUM];
};

struct PTREE_PACKET_RAW_Info_s
{
    PTREE_PACKET_Info_t        base;
    PTREE_PACKET_RAW_RawInfo_t rawInfo;
};

struct PTREE_PACKET_RAW_Obj_s
{
    PTREE_PACKET_Obj_t         base;
    PTREE_PACKET_RAW_Info_t    info;
    PTREE_PACKET_RAW_RawData_t rawData;
};

int PTREE_PACKET_RAW_Init(PTREE_PACKET_RAW_Obj_t* packetRaw, const PTREE_PACKET_ObjOps_t* ops, const char* type);

int PTREE_PACKET_RAW_InfoInit(PTREE_PACKET_RAW_Info_t* packetRawInfo, const PTREE_PACKET_RAW_RawInfo_t* rawInfo);

PTREE_PACKET_Info_t* PTREE_PACKET_RAW_InfoNew(const PTREE_PACKET_RAW_RawInfo_t* rawInfo);

void PTREE_PACKET_RAW_Copy(PTREE_PACKET_RAW_Obj_t* packetRawDst, const PTREE_PACKET_RAW_Obj_t* packetRawSrc);

PTREE_PACKET_Obj_t* PTREE_PACKET_RAW_NormalNew(const PTREE_PACKET_RAW_RawInfo_t* rawInfo);

int PTREE_PACKET_RAW_NormalInit(PTREE_PACKET_RAW_Obj_t* packetRaw, const PTREE_PACKET_RAW_RawInfo_t* rawInfo);

PTREE_PACKET_Obj_t* PTREE_PACKET_RAW_NormalNewClone(const PTREE_PACKET_RAW_Obj_t* packetRawSrc);

int PTREE_PACKET_RAW_NormalClone(PTREE_PACKET_RAW_Obj_t* packetRaw, const PTREE_PACKET_RAW_Obj_t* packetRawSrc);

void PTREE_PACKET_RAW_NormalStride(const PTREE_PACKET_RAW_RawInfo_t* rawInfo,
                                   unsigned int                      stride[PTREE_PACKET_RAW_PLANE_NUM]);

void PTREE_PACKET_RAW_NormalSize(const PTREE_PACKET_RAW_RawInfo_t* rawInfo,
                                 const unsigned int                stride[PTREE_PACKET_RAW_PLANE_NUM],
                                 unsigned int                      size[PTREE_PACKET_RAW_PLANE_NUM]);

PTREE_ENUM_DECLARE(PTREE_PACKET_RAW_VideoFmt_e);

#endif /* ifndef __PTREE_PACKET_RAW_H__ */
