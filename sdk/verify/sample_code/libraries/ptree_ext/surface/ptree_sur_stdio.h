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

#ifndef __PTREE_SUR_STDIO_H__
#define __PTREE_SUR_STDIO_H__

#include "ptree_sur.h"
#include "ptree_packet.h"
#include "ptree_packet_raw.h"

enum PTREE_SUR_STDIO_OutMode_e
{
    E_PTREE_SUR_STDIO_OUT_MODE_RAW_VIDEO,
    E_PTREE_SUR_STDIO_OUT_MODE_META_RGN_FRAME,
};

enum PTREE_SUR_STDIO_MetaRgnRectAreaLayout_e
{
    E_PTREE_SUR_STDIO_META_RGN_FRAME_LAYOUT_GRID,
    E_PTREE_SUR_STDIO_META_RGN_FRAME_LAYOUT_STACK,
    E_PTREE_SUR_STDIO_META_RGN_FRAME_LAYOUT_PAGODA,
    E_PTREE_SUR_STDIO_META_RGN_FRAME_LAYOUT_RANDOM,
};
typedef struct PTREE_SUR_STDIO_MetaRgnRectAreaInfo_s
{
    enum PTREE_SUR_STDIO_MetaRgnRectAreaLayout_e layout;
    unsigned int                                 number;
} PTREE_SUR_STDIO_MetaRgnRectAreaInfo_t;

typedef struct PTREE_SUR_STDIO_RawVideoInfo_s
{
    PTREE_PACKET_RAW_RawInfo_t rawInfo;
    unsigned char              rawData[2];
} PTREE_SUR_STDIO_RawVideoInfo_t;

typedef struct PTREE_SUR_STDIO_TypeInfo_s
{
    unsigned char                  isUserTrigger;
    enum PTREE_SUR_STDIO_OutMode_e mode;
    union
    {
        PTREE_SUR_STDIO_MetaRgnRectAreaInfo_t metaRgnFrameInfo;
        PTREE_SUR_STDIO_RawVideoInfo_t        rawVideoInfo;
    };
} PTREE_SUR_STDIO_TypeInfo_t;

typedef struct PTREE_SUR_STDIO_OutInfo_s
{
    PTREE_SUR_OutInfo_t        base;
    PTREE_SUR_STDIO_TypeInfo_t info;
} PTREE_SUR_STDIO_OutInfo_t;

typedef struct PTREE_SUR_STDIO_InInfo_s
{
    PTREE_SUR_OutInfo_t base;
    unsigned char       isPostReader;
} PTREE_SUR_STDIO_InInfo_t;

#endif /* ifndef __PTREE_SUR_STDIO_H__ */
