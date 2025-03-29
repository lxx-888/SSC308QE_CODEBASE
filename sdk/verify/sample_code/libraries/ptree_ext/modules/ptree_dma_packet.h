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

#ifndef __PTREE_DMA_PACKET_H__
#define __PTREE_DMA_PACKET_H__

#include "mi_sys_datatype.h"
#include "ptree_packet.h"

typedef struct PTREE_DMA_PACKET_Info_s PTREE_DMA_PACKET_Info_t;
typedef struct PTREE_DMA_PACKET_Obj_s  PTREE_DMA_PACKET_Obj_t;

struct PTREE_DMA_PACKET_Info_s
{
    PTREE_PACKET_Info_t base;
    MI_SYS_DmaBufInfo_t dmaBufInfo;
};

struct PTREE_DMA_PACKET_Obj_s
{
    PTREE_PACKET_Obj_t      base;
    PTREE_DMA_PACKET_Info_t info;
};

int                  PTREE_DMA_PACKET_InfoInit(PTREE_DMA_PACKET_Info_t *dmaInfo, const MI_SYS_DmaBufInfo_t *dmaBufInfo);
PTREE_PACKET_Info_t *PTREE_DMA_PACKET_InfoNew(const MI_SYS_DmaBufInfo_t *dmaBufInfo);
int                  PTREE_DMA_PACKET_Init(PTREE_DMA_PACKET_Obj_t *dma, const PTREE_PACKET_ObjOps_t *ops,
                                           const MI_SYS_DmaBufInfo_t *dmaBufInfo);
PTREE_PACKET_Obj_t * PTREE_DMA_PACKET_NormalNew(const MI_SYS_DmaBufInfo_t *dmaBufInfo);

#endif /* ifndef __PTREE_DMA_PACKET_H__ */
