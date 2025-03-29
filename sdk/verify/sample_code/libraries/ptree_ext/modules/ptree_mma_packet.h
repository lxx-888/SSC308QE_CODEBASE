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

#ifndef __PTREE_MMA_PACKET_H__
#define __PTREE_MMA_PACKET_H__

#include "mi_common_datatype.h"
#include "mi_sys_datatype.h"
#include "ptree_obj.h"
#include "ptree_packer.h"
#include "ptree_packet.h"
#include "ptree_packet_raw.h"
#include "ptree_packet_video.h"

typedef struct PTREE_MMA_PACKET_RawData_s  PTREE_MMA_PACKET_RawData_t;
typedef struct PTREE_MMA_PACKET_RawPa_s    PTREE_MMA_PACKET_RawPa_t;
typedef struct PTREE_MMA_PACKET_RawPaOps_s PTREE_MMA_PACKET_RawPaOps_t;
typedef struct PTREE_MMA_PACKET_RawVa_s    PTREE_MMA_PACKET_RawVa_t;
typedef struct PTREE_MMA_PACKET_VideoPa_s  PTREE_MMA_PACKET_VideoPa_t;

typedef struct PTREE_MMA_PACKET_Packer_s PTREE_MMA_PACKET_Packer_t;

struct PTREE_MMA_PACKET_VideoPa_s
{
    PTREE_PACKET_VIDEO_Obj_t base;
    MI_PHY                   pa[PTREE_PACKET_VIDEO_SLICE_COUNT_MAX];
};

struct PTREE_MMA_PACKET_RawData_s
{
    MI_PHY       phy[PTREE_PACKET_RAW_PLANE_NUM];
    unsigned int stride[PTREE_PACKET_RAW_PLANE_NUM];
    unsigned int size[PTREE_PACKET_RAW_PLANE_NUM];
};

struct PTREE_MMA_PACKET_RawPa_s
{
    PTREE_PACKET_Obj_t                 base;
    const PTREE_MMA_PACKET_RawPaOps_t *ops;
    PTREE_PACKET_RAW_Info_t            info;
    PTREE_MMA_PACKET_RawData_t         rawData;
};

struct PTREE_MMA_PACKET_RawPaOps_s
{
    void (*updateTimeStamp)(PTREE_PACKET_Obj_t *packet);
};

struct PTREE_MMA_PACKET_RawVa_s
{
    PTREE_PACKET_RAW_Obj_t base;
    PTREE_PACKET_Obj_t *   rawPa;
};

struct PTREE_MMA_PACKET_Packer_s
{
    PTREE_PACKER_Obj_t base;
};

int PTREE_MMA_PACKET_RawPaInfoInit(PTREE_PACKET_RAW_Info_t *packetRawInfo, const PTREE_PACKET_RAW_RawInfo_t *rawInfo);

int PTREE_MMA_PACKET_RawPaInit(PTREE_MMA_PACKET_RawPa_t *rawPa, const PTREE_MMA_PACKET_RawPaOps_t *rawPaOps,
                               const char *type);

void PTREE_MMA_PACKET_RawPaCopy(PTREE_MMA_PACKET_RawPa_t *rawPaDst, const PTREE_MMA_PACKET_RawPa_t *rawPaSrc);

int PTREE_MMA_PACKET_RawPaNormalInit(PTREE_MMA_PACKET_RawPa_t *rawPa, const PTREE_PACKET_RAW_RawInfo_t *rawInfo);

PTREE_PACKET_Obj_t *PTREE_MMA_PACKET_RawPaNormalNew(const PTREE_PACKET_RAW_RawInfo_t *rawInfo);

int PTREE_MMA_PACKET_RawVaInit(PTREE_MMA_PACKET_RawVa_t *rawVa, PTREE_MMA_PACKET_RawPa_t *rawPa);

PTREE_PACKET_Obj_t *PTREE_MMA_PACKET_RawVaNew(PTREE_MMA_PACKET_RawPa_t *rawPa);

int PTREE_MMA_PACKET_PackerInit(PTREE_MMA_PACKET_Packer_t *mmaPacker);

#endif /* ifndef __PTREE_MMA_PACKET_H__ */
