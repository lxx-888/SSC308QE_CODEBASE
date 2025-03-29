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

#ifndef __PTREE_PACKET_VIDEO_H__
#define __PTREE_PACKET_VIDEO_H__

#include "ptree_enum.h"
#include "ptree_packet.h"

#define PTREE_PACKET_VIDEO_SLICE_COUNT_MAX (16)

typedef struct PTREE_PACKET_VIDEO_PacketInfo_s PTREE_PACKET_VIDEO_PacketInfo_t;
typedef struct PTREE_PACKET_VIDEO_PacketData_s PTREE_PACKET_VIDEO_PacketData_t;

typedef struct PTREE_PACKET_VIDEO_Info_s PTREE_PACKET_VIDEO_Info_t;
typedef struct PTREE_PACKET_VIDEO_Obj_s  PTREE_PACKET_VIDEO_Obj_t;

enum PTREE_PACKET_VIDEO_Fmt_e
{
    E_PTREE_PACKET_VIDEO_STREAM_H264,
    E_PTREE_PACKET_VIDEO_STREAM_H265,
    E_PTREE_PACKET_VIDEO_STREAM_JPEG,
    E_PTREE_PACKET_VIDEO_STREAM_AV1,
    E_PTREE_PACKET_VIDEO_STREAM_VP9
};

struct PTREE_PACKET_VIDEO_PacketInfo_s
{
    enum PTREE_PACKET_VIDEO_Fmt_e fmt;
    unsigned int                  width;
    unsigned int                  height;
    unsigned char                 bHead;
    unsigned int                  pktCount;
    struct
    {
        unsigned int  size;
        unsigned char bEnd;
    } pktInfo[PTREE_PACKET_VIDEO_SLICE_COUNT_MAX];
};

struct PTREE_PACKET_VIDEO_PacketData_s
{
    struct
    {
        char* data;
    } pktData[PTREE_PACKET_VIDEO_SLICE_COUNT_MAX];
};

struct PTREE_PACKET_VIDEO_Info_s
{
    PTREE_PACKET_Info_t             base;
    PTREE_PACKET_VIDEO_PacketInfo_t packetInfo;
};

struct PTREE_PACKET_VIDEO_Obj_s
{
    PTREE_PACKET_Obj_t              base;
    PTREE_PACKET_VIDEO_Info_t       info;
    PTREE_PACKET_VIDEO_PacketData_t packetData;
};

int PTREE_PACKET_VIDEO_Init(PTREE_PACKET_VIDEO_Obj_t* packetVideo, const PTREE_PACKET_ObjOps_t* ops, const char* type);

int PTREE_PACKET_VIDEO_InfoInit(PTREE_PACKET_VIDEO_Info_t*             packetVideo,
                                const PTREE_PACKET_VIDEO_PacketInfo_t* packetInfo);

PTREE_PACKET_Info_t* PTREE_PACKET_VIDEO_InfoNew(const PTREE_PACKET_VIDEO_PacketInfo_t* packetInfo);

int PTREE_PACKET_VIDEO_Copy(PTREE_PACKET_VIDEO_Obj_t* packetVideoDst, const PTREE_PACKET_VIDEO_Obj_t* packetVideoSrc);

PTREE_PACKET_Obj_t* PTREE_PACKET_VIDEO_NormalNew(const PTREE_PACKET_VIDEO_PacketInfo_t* packetInfo);

int PTREE_PACKET_VIDEO_NormalInit(PTREE_PACKET_VIDEO_Obj_t*              packetVideo,
                                  const PTREE_PACKET_VIDEO_PacketInfo_t* packetInfo);

PTREE_PACKET_Obj_t* PTREE_PACKET_VIDEO_NormalNewClone(const PTREE_PACKET_VIDEO_Obj_t* packetRawSrc);

int PTREE_PACKET_VIDEO_NormalClone(PTREE_PACKET_VIDEO_Obj_t*       packetVideo,
                                   const PTREE_PACKET_VIDEO_Obj_t* packetVideoSrc);

PTREE_ENUM_DECLARE(PTREE_PACKET_VIDEO_Fmt_e);

#endif /* ifndef __PTREE_PACKET_VIDEO_H__ */
