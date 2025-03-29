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

#include "mi_sys.h"
#include "ssos_def.h"
#include "ptree_log.h"
#include "ssos_mem.h"
#include "ssos_list.h"
#include "ptree_obj.h"
#include "ptree_packer.h"
#include "ptree_packet.h"
#include "ptree_packet_raw.h"
#include "ptree_mma_packet.h"

#define CHECK_POINTER(_ptr, _err)            \
    (                                        \
        {                                    \
            if (!(_ptr))                     \
            {                                \
                PTREE_ERR("%s NULL", #_ptr); \
                _err;                        \
            }                                \
        })

static int _PTREE_MMA_PACKET_InfoEqual(const PTREE_PACKET_Info_t *self, const PTREE_PACKET_Info_t *other);

static PTREE_PACKET_Obj_t *_PTREE_MMA_PACKET_RawPaConvert(PTREE_PACKET_Obj_t *packet, const char *type);
static void                _PTREE_MMA_PACKET_RawPaUpdateTimeStamp(PTREE_PACKET_Obj_t *packet);

static void _PTREE_MMA_PACKET_RawPaDestruct(PTREE_PACKET_Obj_t *packet);
static void _PTREE_MMA_PACKET_RawPaFree(PTREE_PACKET_Obj_t *packet);

static PTREE_PACKET_Obj_t *_PTREE_MMA_PACKET_RawVaConvert(PTREE_PACKET_Obj_t *packet, const char *type);
static void                _PTREE_MMA_PACKET_RawVaUpdateTimeStamp(PTREE_PACKET_Obj_t *packet);

static void _PTREE_MMA_PACKET_RawVaDestruct(PTREE_PACKET_Obj_t *packet);
static void _PTREE_MMA_PACKET_RawVaFree(PTREE_PACKET_Obj_t *packet);

static PTREE_PACKET_Obj_t *_PTREE_MMA_PACKET_PackerMake(PTREE_PACKER_Obj_t *packer, const PTREE_PACKET_Info_t *info);

static const PTREE_PACKET_InfoOps_t G_PTREE_MMA_PACKET_INFO_OPS = {
    .equal = _PTREE_MMA_PACKET_InfoEqual,
};

static const PTREE_PACKET_InfoHook_t G_PTREE_MMA_PACKET_INFO_HOOK = {};

static const PTREE_PACKET_ObjOps_t G_PTREE_MMA_PACKET_RAW_PA_OPS = {
    .convert         = _PTREE_MMA_PACKET_RawPaConvert,
    .updateTimeStamp = _PTREE_MMA_PACKET_RawPaUpdateTimeStamp,
};
static const PTREE_PACKET_ObjHook_t G_PTREE_MMA_PACKET_RAW_PA_HOOK = {
    .destruct = _PTREE_MMA_PACKET_RawPaDestruct,
};
static const PTREE_PACKET_ObjHook_t G_PTREE_MMA_PACKET_RAW_PA_HOOK_DYN = {
    .destruct = _PTREE_MMA_PACKET_RawPaDestruct,
    .free     = _PTREE_MMA_PACKET_RawPaFree,
};

static const PTREE_PACKET_ObjOps_t G_PTREE_MMA_PACKET_RAW_VA_OPS = {
    .convert         = _PTREE_MMA_PACKET_RawVaConvert,
    .updateTimeStamp = _PTREE_MMA_PACKET_RawVaUpdateTimeStamp,
};
static const PTREE_PACKET_ObjHook_t G_PTREE_MMA_PACKET_RAW_VA_HOOK = {
    .destruct = _PTREE_MMA_PACKET_RawVaDestruct,
};
static const PTREE_PACKET_ObjHook_t G_PTREE_MMA_PACKET_RAW_VA_HOOK_DYN = {
    .destruct = _PTREE_MMA_PACKET_RawVaDestruct,
    .free     = _PTREE_MMA_PACKET_RawVaFree,
};

static const PTREE_PACKER_Ops_t G_PTREE_MMA_PACKER_OPS = {
    .make = _PTREE_MMA_PACKET_PackerMake,
};
static const PTREE_PACKER_Hook_t G_PTREE_MMA_PACKER_HOOK = {};

PTREE_PACKET_INFO_TYPE_DEFINE(raw_pa);

PTREE_PACKET_TYPE_DEFINE(sys_mma);

static int _PTREE_MMA_PACKET_InfoEqual(const PTREE_PACKET_Info_t *self, const PTREE_PACKET_Info_t *other)
{
    PTREE_PACKET_RAW_Info_t *selfRawInfo  = CONTAINER_OF(self, PTREE_PACKET_RAW_Info_t, base);
    PTREE_PACKET_RAW_Info_t *otherRawInfo = CONTAINER_OF(other, PTREE_PACKET_RAW_Info_t, base);
    return !memcmp(&selfRawInfo->rawInfo, &otherRawInfo->rawInfo, sizeof(PTREE_PACKET_RAW_RawInfo_t));
}

static PTREE_PACKET_Obj_t *_PTREE_MMA_PACKET_RawPaConvert(PTREE_PACKET_Obj_t *packet, const char *type)
{
    PTREE_MMA_PACKET_RawPa_t *rawPa = CONTAINER_OF(packet, PTREE_MMA_PACKET_RawPa_t, base);
    if (PTREE_PACKET_INFO_TYPE(raw) == type)
    {
        return PTREE_MMA_PACKET_RawVaNew(rawPa);
    }
    PTREE_ERR("Can't convert to %s", type);
    return NULL;
}

static void _PTREE_MMA_PACKET_RawPaUpdateTimeStamp(PTREE_PACKET_Obj_t *packet)
{
    PTREE_MMA_PACKET_RawPa_t *rawPa = CONTAINER_OF(packet, PTREE_MMA_PACKET_RawPa_t, base);
    if (!rawPa->ops || !rawPa->ops->updateTimeStamp)
    {
        PTREE_PACKET_AutoUpdateTimeStamp(packet);
        return;
    }
    rawPa->ops->updateTimeStamp(packet);
}

static void _PTREE_MMA_PACKET_RawPaDestruct(PTREE_PACKET_Obj_t *packet)
{
    PTREE_MMA_PACKET_RawPa_t *rawPa = CONTAINER_OF(packet, PTREE_MMA_PACKET_RawPa_t, base);
    if (rawPa->rawData.phy[0])
    {
        MI_SYS_MMA_Free(0, rawPa->rawData.phy[0]);
        rawPa->rawData.phy[0] = 0;
        rawPa->rawData.phy[1] = 0;
    }
    rawPa->rawData.size[0]   = 0;
    rawPa->rawData.size[1]   = 0;
    rawPa->rawData.stride[0] = 0;
    rawPa->rawData.stride[1] = 0;
}

static void _PTREE_MMA_PACKET_RawPaFree(PTREE_PACKET_Obj_t *packet)
{
    PTREE_MMA_PACKET_RawPa_t *rawPa = CONTAINER_OF(packet, PTREE_MMA_PACKET_RawPa_t, base);
    SSOS_MEM_Free(rawPa);
}

static PTREE_PACKET_Obj_t *_PTREE_MMA_PACKET_RawVaConvert(PTREE_PACKET_Obj_t *packet, const char *type)
{
    PTREE_MMA_PACKET_RawVa_t *rawVa = CONTAINER_OF(packet, PTREE_MMA_PACKET_RawVa_t, base.base);
    if (PTREE_PACKET_INFO_TYPE(raw_pa) == type)
    {
        MI_SYS_FlushInvCache(rawVa->base.rawData.data[0], rawVa->base.rawData.size[0] + rawVa->base.rawData.size[1]);
        return PTREE_PACKET_Dup(rawVa->rawPa);
    }
    PTREE_ERR("Can't convert to %s", type);
    return NULL;
}

static void _PTREE_MMA_PACKET_RawVaUpdateTimeStamp(PTREE_PACKET_Obj_t *packet)
{
    PTREE_MMA_PACKET_RawVa_t *rawVa = CONTAINER_OF(packet, PTREE_MMA_PACKET_RawVa_t, base.base);
    PTREE_PACKET_SetTimeStamp(packet, PTREE_PACKET_GetTimeStamp(rawVa->rawPa));
}

static void _PTREE_MMA_PACKET_RawVaDestruct(PTREE_PACKET_Obj_t *packet)
{
    PTREE_MMA_PACKET_RawVa_t *rawVa = CONTAINER_OF(packet, PTREE_MMA_PACKET_RawVa_t, base.base);
    if (rawVa->base.rawData.data[0])
    {
        MI_SYS_Munmap(rawVa->base.rawData.data[0], rawVa->base.rawData.size[0] + rawVa->base.rawData.size[1]);
        rawVa->base.rawData.data[0] = NULL;
        rawVa->base.rawData.data[1] = NULL;
    }
    rawVa->base.rawData.stride[0] = 0;
    rawVa->base.rawData.stride[1] = 0;
    rawVa->base.rawData.size[0]   = 0;
    rawVa->base.rawData.size[1]   = 0;
    PTREE_PACKET_Del(rawVa->rawPa);
}

static void _PTREE_MMA_PACKET_RawVaFree(PTREE_PACKET_Obj_t *packet)
{
    PTREE_MMA_PACKET_RawVa_t *rawVa = CONTAINER_OF(packet, PTREE_MMA_PACKET_RawVa_t, base.base);
    SSOS_MEM_Free(rawVa);
}

static PTREE_PACKET_Obj_t *_PTREE_MMA_PACKET_PackerMake(PTREE_PACKER_Obj_t *packer, const PTREE_PACKET_Info_t *info)
{
    (void)packer;
    if (PTREE_PACKET_InfoLikely(info, PTREE_PACKET_INFO_TYPE(raw_pa)))
    {
        PTREE_PACKET_RAW_Info_t *rawInfo = CONTAINER_OF(info, PTREE_PACKET_RAW_Info_t, base);
        return PTREE_MMA_PACKET_RawPaNormalNew(&rawInfo->rawInfo);
    }
    if (PTREE_PACKET_InfoLikely(info, PTREE_PACKET_INFO_TYPE(raw)))
    {
        PTREE_PACKET_RAW_Info_t *rawInfo     = CONTAINER_OF(info, PTREE_PACKET_RAW_Info_t, base);
        PTREE_PACKET_Obj_t *     rawPaPacket = PTREE_MMA_PACKET_RawPaNormalNew(&rawInfo->rawInfo);
        PTREE_PACKET_Obj_t *     rawVaPacket = NULL;
        if (!rawPaPacket)
        {
            return NULL;
        }
        rawVaPacket = PTREE_MMA_PACKET_RawVaNew(CONTAINER_OF(rawPaPacket, PTREE_MMA_PACKET_RawPa_t, base));
        PTREE_PACKET_Del(rawPaPacket);
        return rawVaPacket;
    }
    PTREE_ERR("packet info type %s is not support\n", info->type);
    return NULL;
}

static int _PTREE_MMA_PACKET_RawPaNormalInit(PTREE_MMA_PACKET_RawPa_t *rawPa, const PTREE_PACKET_RAW_RawInfo_t *rawInfo)
{
    int ret;
    ret = PTREE_MMA_PACKET_RawPaInit(rawPa, NULL, PTREE_PACKET_TYPE(sys_mma));
    if (ret != SSOS_DEF_OK)
    {
        PTREE_ERR("PTREE_MMA_PACKET_RawPaInit Failed");
        return ret;
    }

    rawPa->info.rawInfo = *rawInfo;
    PTREE_PACKET_RAW_NormalStride(&rawPa->info.rawInfo, rawPa->rawData.stride);
    PTREE_PACKET_RAW_NormalSize(&rawPa->info.rawInfo, rawPa->rawData.stride, rawPa->rawData.size);

    if (MI_SUCCESS
        != MI_SYS_MMA_Alloc(0, NULL, rawPa->rawData.size[0] + rawPa->rawData.size[1], &rawPa->rawData.phy[0]))
    {
        PTREE_ERR("MI_SYS_MMA_Alloc failed");
        PTREE_PACKET_Del(&rawPa->base);
        return SSOS_DEF_ENOMEM;
    }

    if (rawPa->rawData.size[1])
    {
        rawPa->rawData.phy[1] = rawPa->rawData.phy[0] + rawPa->rawData.size[0];
    }
    return ret;
}

static int _PTREE_MMA_PACKET_RawVaInit(PTREE_MMA_PACKET_RawVa_t *rawVa, PTREE_MMA_PACKET_RawPa_t *rawPa)
{
    int ret;
    ret = PTREE_PACKET_RAW_Init(&rawVa->base, &G_PTREE_MMA_PACKET_RAW_VA_OPS, PTREE_PACKET_TYPE(sys_mma));
    if (ret != SSOS_DEF_OK)
    {
        PTREE_ERR("PTREE_MMA_PACKET_RawVaInit Failed");
        return ret;
    }

    rawVa->rawPa = PTREE_PACKET_Dup(&rawPa->base);
    if (!rawVa->rawPa)
    {
        PTREE_ERR("PTREE_PACKET_Dup failed");
        PTREE_PACKET_Del(&rawVa->base.base);
        return SSOS_DEF_FAIL;
    }
    rawVa->base.info.rawInfo      = rawPa->info.rawInfo;
    rawVa->base.rawData.stride[0] = rawPa->rawData.stride[0];
    rawVa->base.rawData.stride[1] = rawPa->rawData.stride[1];
    rawVa->base.rawData.size[0]   = rawPa->rawData.size[0];
    rawVa->base.rawData.size[1]   = rawPa->rawData.size[1];

    if (MI_SUCCESS
        != MI_SYS_Mmap(rawPa->rawData.phy[0], rawPa->rawData.size[0] + rawPa->rawData.size[1],
                       (void **)&rawVa->base.rawData.data[0], TRUE))
    {
        PTREE_ERR("MI_SYS_MMA_Alloc failed");
        PTREE_PACKET_Del(&rawVa->base.base);
        return SSOS_DEF_ENOMEM;
    }

    if (rawVa->base.rawData.size[1])
    {
        rawVa->base.rawData.data[1] = rawVa->base.rawData.data[0] + rawVa->base.rawData.size[0];
    }
    return ret;
}

int PTREE_MMA_PACKET_RawPaInfoInit(PTREE_PACKET_RAW_Info_t *packetRawInfo, const PTREE_PACKET_RAW_RawInfo_t *rawInfo)
{
    int ret = SSOS_DEF_OK;
    CHECK_POINTER(packetRawInfo, return SSOS_DEF_EINVAL);
    CHECK_POINTER(rawInfo, return SSOS_DEF_EINVAL);
    ret = PTREE_PACKET_InfoInit(&packetRawInfo->base, &G_PTREE_MMA_PACKET_INFO_OPS, PTREE_PACKET_INFO_TYPE(raw_pa));
    if (ret != SSOS_DEF_OK)
    {
        PTREE_ERR("PTREE_PACKET_InfoInit Failed");
        return ret;
    }
    packetRawInfo->rawInfo = *rawInfo;
    PTREE_PACKET_InfoRegister(&packetRawInfo->base, &G_PTREE_MMA_PACKET_INFO_HOOK);
    return ret;
}

int PTREE_MMA_PACKET_RawPaInit(PTREE_MMA_PACKET_RawPa_t *rawPa, const PTREE_MMA_PACKET_RawPaOps_t *rawPaOps,
                               const char *type)
{
    int ret;
    CHECK_POINTER(rawPa, return SSOS_DEF_EINVAL);
    CHECK_POINTER(type, return SSOS_DEF_EINVAL);
    ret = PTREE_PACKET_InfoInit(&rawPa->info.base, &G_PTREE_MMA_PACKET_INFO_OPS, PTREE_PACKET_INFO_TYPE(raw_pa));
    if (ret != SSOS_DEF_OK)
    {
        PTREE_ERR("PTREE_PACKET_InfoInit Failed");
        return ret;
    }
    rawPa->ops = rawPaOps;
    return PTREE_PACKET_Init(&rawPa->base, &G_PTREE_MMA_PACKET_RAW_PA_OPS, &rawPa->info.base, type);
}

void PTREE_MMA_PACKET_RawPaCopy(PTREE_MMA_PACKET_RawPa_t *rawPaDst, const PTREE_MMA_PACKET_RawPa_t *rawPaSrc)
{
    CHECK_POINTER(rawPaDst, return );
    CHECK_POINTER(rawPaSrc, return );
    if (rawPaDst->rawData.phy[0])
    {
        MI_SYS_MemcpyPa(0, rawPaDst->rawData.phy[0], rawPaSrc->rawData.phy[0], rawPaDst->rawData.size[0]);
    }
    if (rawPaDst->rawData.phy[1])
    {
        MI_SYS_MemcpyPa(0, rawPaDst->rawData.phy[1], rawPaSrc->rawData.phy[1], rawPaDst->rawData.size[1]);
    }
}

int PTREE_MMA_PACKET_RawPaNormalInit(PTREE_MMA_PACKET_RawPa_t *rawPa, const PTREE_PACKET_RAW_RawInfo_t *rawInfo)
{
    int ret;
    CHECK_POINTER(rawPa, return SSOS_DEF_EINVAL);
    CHECK_POINTER(rawInfo, return SSOS_DEF_EINVAL);
    ret = _PTREE_MMA_PACKET_RawPaNormalInit(rawPa, rawInfo);
    if (ret != SSOS_DEF_OK)
    {
        return ret;
    }
    PTREE_PACKET_Register(&rawPa->base, &G_PTREE_MMA_PACKET_RAW_PA_HOOK);
    return ret;
}

PTREE_PACKET_Obj_t *PTREE_MMA_PACKET_RawPaNormalNew(const PTREE_PACKET_RAW_RawInfo_t *rawInfo)
{
    PTREE_MMA_PACKET_RawPa_t *rawPa = NULL;
    CHECK_POINTER(rawInfo, return NULL);
    rawPa = SSOS_MEM_Alloc(sizeof(PTREE_MMA_PACKET_RawPa_t));
    if (!rawPa)
    {
        PTREE_ERR("Alloc err");
        return NULL;
    }
    memset(rawPa, 0, sizeof(PTREE_MMA_PACKET_RawPa_t));
    if (SSOS_DEF_OK != _PTREE_MMA_PACKET_RawPaNormalInit(rawPa, rawInfo))
    {
        SSOS_MEM_Free(rawPa);
        return NULL;
    }
    PTREE_PACKET_Register(&rawPa->base, &G_PTREE_MMA_PACKET_RAW_PA_HOOK_DYN);
    return &rawPa->base;
}

int PTREE_MMA_PACKET_RawVaInit(PTREE_MMA_PACKET_RawVa_t *rawVa, PTREE_MMA_PACKET_RawPa_t *rawPa)
{
    int ret;
    CHECK_POINTER(rawVa, return SSOS_DEF_EINVAL);
    CHECK_POINTER(rawPa, return SSOS_DEF_EINVAL);
    ret = _PTREE_MMA_PACKET_RawVaInit(rawVa, rawPa);
    if (ret != SSOS_DEF_OK)
    {
        return ret;
    }
    PTREE_PACKET_Register(&rawVa->base.base, &G_PTREE_MMA_PACKET_RAW_VA_HOOK);
    return ret;
}

PTREE_PACKET_Obj_t *PTREE_MMA_PACKET_RawVaNew(PTREE_MMA_PACKET_RawPa_t *rawPa)
{
    PTREE_MMA_PACKET_RawVa_t *rawVa = NULL;
    CHECK_POINTER(rawPa, return NULL);
    rawVa = SSOS_MEM_Alloc(sizeof(PTREE_MMA_PACKET_RawVa_t));
    if (!rawVa)
    {
        PTREE_ERR("Alloc err\n");
        return NULL;
    }
    memset(rawVa, 0, sizeof(PTREE_MMA_PACKET_RawVa_t));
    if (SSOS_DEF_OK != _PTREE_MMA_PACKET_RawVaInit(rawVa, rawPa))
    {
        SSOS_MEM_Free(rawVa);
        return NULL;
    }
    PTREE_PACKET_Register(&rawVa->base.base, &G_PTREE_MMA_PACKET_RAW_VA_HOOK_DYN);
    return &rawVa->base.base;
}

int PTREE_MMA_PACKET_PackerInit(PTREE_MMA_PACKET_Packer_t *mmaPacker)
{
    int ret;

    ret = PTREE_PACKER_Init(&mmaPacker->base, &G_PTREE_MMA_PACKER_OPS);
    if (ret != SSOS_DEF_OK)
    {
        return ret;
    }

    PTREE_PACKER_Register(&mmaPacker->base, &G_PTREE_MMA_PACKER_HOOK);
    return SSOS_DEF_OK;
}
