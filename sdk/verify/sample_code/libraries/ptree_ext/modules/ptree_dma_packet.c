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

#include "ssos_def.h"
#include "ssos_list.h"
#include "ssos_mem.h"
#include "ptree_log.h"
#include "ptree_dma_packet.h"
#include "mi_sys_datatype.h"
#include "mi_sys.h"
#include "ptree_packet.h"

#define CHECK_POINTER(_ptr, _err)            \
    (                                        \
        {                                    \
            if (!(_ptr))                     \
            {                                \
                PTREE_ERR("%s NULL", #_ptr); \
                _err;                        \
            }                                \
        })

PTREE_PACKET_TYPE_DEFINE(sys_dma);
PTREE_PACKET_INFO_TYPE_DEFINE(sys_dma);

static int _PTREE_DMA_PACKET_InfoEqual(const PTREE_PACKET_Info_t *self, const PTREE_PACKET_Info_t *other);

static void _PTREE_DMA_PACKET_InfoFree(PTREE_PACKET_Info_t *info);

static void _PTREE_DMA_PACKET_Free(PTREE_PACKET_Obj_t *packet);

static const PTREE_PACKET_InfoOps_t G_PTREE_DMA_PACKET_INFO_OPS = {
    .equal = _PTREE_DMA_PACKET_InfoEqual,
};

static const PTREE_PACKET_InfoHook_t G_PTREE_DMA_PACKET_INFO_HOOK = {};

static const PTREE_PACKET_InfoHook_t G_PTREE_DMA_PACKET_INFO_DYN_HOOK = {
    .free = _PTREE_DMA_PACKET_InfoFree,
};

static const PTREE_PACKET_ObjOps_t G_PTREE_DMA_PACKET_NORMAL_OPS = {};

static const PTREE_PACKET_ObjHook_t G_PTREE_DMA_PACKET_DYN_HOOK = {
    .free = _PTREE_DMA_PACKET_Free,
};

static int _PTREE_DMA_PACKET_InfoEqual(const PTREE_PACKET_Info_t *self, const PTREE_PACKET_Info_t *other)
{
    PTREE_DMA_PACKET_Info_t *selfDmaInfo  = CONTAINER_OF(self, PTREE_DMA_PACKET_Info_t, base);
    PTREE_DMA_PACKET_Info_t *otherDmaInfo = CONTAINER_OF(other, PTREE_DMA_PACKET_Info_t, base);
    return 0 == memcmp(&selfDmaInfo->dmaBufInfo, &otherDmaInfo->dmaBufInfo, sizeof(MI_SYS_DmaBufInfo_t));
}

static void _PTREE_DMA_PACKET_InfoFree(PTREE_PACKET_Info_t *info)
{
    SSOS_MEM_Free(CONTAINER_OF(info, PTREE_DMA_PACKET_Info_t, base));
}

static void _PTREE_DMA_PACKET_Free(PTREE_PACKET_Obj_t *packet)
{
    SSOS_MEM_Free(CONTAINER_OF(packet, PTREE_DMA_PACKET_Obj_t, base));
}

static int _PTREE_DMA_PACKET_Init(PTREE_DMA_PACKET_Obj_t *dma, const PTREE_PACKET_ObjOps_t *ops,
                                  const MI_SYS_DmaBufInfo_t *dmaBufInfo)
{
    int ret = SSOS_DEF_OK;
    ret     = PTREE_DMA_PACKET_InfoInit(&dma->info, dmaBufInfo);
    if (ret != SSOS_DEF_OK)
    {
        PTREE_ERR("PTREE_DMA_PACKET_InfoInit failed");
        return ret;
    }
    ret = PTREE_PACKET_Init(&dma->base, ops, &dma->info.base, PTREE_PACKET_TYPE(sys_dma));
    if (ret != SSOS_DEF_OK)
    {
        PTREE_PACKET_InfoDel(&dma->info.base);
        PTREE_ERR("PTREE_PACKET_Init failed");
        return ret;
    }
    return ret;
}

int PTREE_DMA_PACKET_InfoInit(PTREE_DMA_PACKET_Info_t *dmaInfo, const MI_SYS_DmaBufInfo_t *dmaBufInfo)
{
    int ret = SSOS_DEF_OK;
    CHECK_POINTER(dmaInfo, return SSOS_DEF_EINVAL);
    CHECK_POINTER(dmaBufInfo, return SSOS_DEF_EINVAL);
    ret = PTREE_PACKET_InfoInit(&dmaInfo->base, &G_PTREE_DMA_PACKET_INFO_OPS, PTREE_PACKET_INFO_TYPE(sys_dma));
    if (ret != SSOS_DEF_OK)
    {
        PTREE_ERR("PTREE_PACKET_InfoInit failed");
        return ret;
    }
    dmaInfo->dmaBufInfo = *dmaBufInfo;
    PTREE_PACKET_InfoRegister(&dmaInfo->base, &G_PTREE_DMA_PACKET_INFO_HOOK);
    return SSOS_DEF_OK;
}
PTREE_PACKET_Info_t *PTREE_DMA_PACKET_InfoNew(const MI_SYS_DmaBufInfo_t *dmaBufInfo)
{
    PTREE_DMA_PACKET_Info_t *dmaInfo = NULL;
    CHECK_POINTER(dmaBufInfo, return NULL);
    dmaInfo = SSOS_MEM_Alloc(sizeof(PTREE_DMA_PACKET_Info_t));
    if (!dmaInfo)
    {
        PTREE_ERR("Alloc err");
        return NULL;
    }
    memset(dmaInfo, 0, sizeof(PTREE_DMA_PACKET_Info_t));
    if (SSOS_DEF_OK
        != PTREE_PACKET_InfoInit(&dmaInfo->base, &G_PTREE_DMA_PACKET_INFO_OPS, PTREE_PACKET_INFO_TYPE(sys_dma)))
    {
        PTREE_ERR("PTREE_PACKET_InfoInit failed");
        SSOS_MEM_Free(dmaInfo);
        return NULL;
    }
    dmaInfo->dmaBufInfo = *dmaBufInfo;
    PTREE_PACKET_InfoRegister(&dmaInfo->base, &G_PTREE_DMA_PACKET_INFO_DYN_HOOK);
    return &dmaInfo->base;
}
int PTREE_DMA_PACKET_Init(PTREE_DMA_PACKET_Obj_t *dma, const PTREE_PACKET_ObjOps_t *ops,
                          const MI_SYS_DmaBufInfo_t *dmaBufInfo)
{
    CHECK_POINTER(dma, return SSOS_DEF_EINVAL);
    CHECK_POINTER(ops, return SSOS_DEF_EINVAL);
    CHECK_POINTER(dmaBufInfo, return SSOS_DEF_EINVAL);

    return _PTREE_DMA_PACKET_Init(dma, ops, dmaBufInfo);
}
PTREE_PACKET_Obj_t *PTREE_DMA_PACKET_NormalNew(const MI_SYS_DmaBufInfo_t *dmaBufInfo)
{
    PTREE_DMA_PACKET_Obj_t *dma = NULL;

    CHECK_POINTER(dmaBufInfo, return NULL);

    dma = SSOS_MEM_Alloc(sizeof(PTREE_DMA_PACKET_Obj_t));
    if (!dma)
    {
        PTREE_ERR("Alloc err");
        return NULL;
    }
    memset(dma, 0, sizeof(PTREE_DMA_PACKET_Obj_t));

    if (SSOS_DEF_OK != _PTREE_DMA_PACKET_Init(dma, &G_PTREE_DMA_PACKET_NORMAL_OPS, dmaBufInfo))
    {
        return NULL;
    }
    PTREE_PACKET_Register(&dma->base, &G_PTREE_DMA_PACKET_DYN_HOOK);
    return &dma->base;
}
