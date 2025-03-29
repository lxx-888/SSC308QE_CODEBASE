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

#ifdef __KERNEL__

/* IF build in kernel mode, no thing to do */

#else /* no kernel mode */

#include "mi_common_datatype.h"
#include "mi_ipu_datatype.h"
#include "sstar_det_api.h"
#include "ssos_def.h"
#include "ssos_io.h"
#include "ptree_log.h"
#include "ssos_mem.h"
#include "ssos_list.h"
#include "ptree_packet.h"
#include "ptree_packer.h"
#include "ptree_mma_packet.h"
#include "ptree_linker.h"
#include "ptree_mod_sys.h"
#include "ptree_sur_det.h"
#include "ptree_mod_det.h"
#include "ptree_rgn_packet.h"
#include "ptree_packet_raw.h"
#include "ptree_maker.h"

typedef struct PTREE_MOD_DET_Obj_s PTREE_MOD_DET_Obj_t;
struct PTREE_MOD_DET_Obj_s
{
    PTREE_MOD_SYS_Obj_t       base;
    PTREE_MOD_InObj_t         modIn;
    PTREE_MOD_OutObj_t        modOut;
    PTREE_LINKER_Obj_t        inLinker;
    PTREE_MMA_PACKET_Packer_t packer;
    Box_t                     boxes[MAX_DET_OBJECT];
    void *                    detectHandle;
};

static int                 _PTREE_MOD_DET_InLinkerEnqueue(PTREE_LINKER_Obj_t *linker, PTREE_PACKET_Obj_t *packet);
static PTREE_PACKET_Obj_t *_PTREE_MOD_DET_InLinkerDequeue(PTREE_LINKER_Obj_t *linker, int ms);

static int                 _PTREE_MOD_DET_InGetType(PTREE_MOD_InObj_t *modIn);
static PTREE_LINKER_Obj_t *_PTREE_MOD_DET_InCreateNLinker(PTREE_MOD_InObj_t *modIn);
static PTREE_PACKER_Obj_t *_PTREE_MOD_DET_InCreatePacker(PTREE_MOD_InObj_t *modIn, int *isFast);

static int _PTREE_MOD_DET_OutGetType(PTREE_MOD_OutObj_t *modOut);

static int                 _PTREE_MOD_DET_Init(PTREE_MOD_SYS_Obj_t *sysMod);
static int                 _PTREE_MOD_DET_Deinit(PTREE_MOD_SYS_Obj_t *sysMod);
static PTREE_MOD_InObj_t * _PTREE_MOD_DET_CreateModIn(PTREE_MOD_Obj_t *mod, unsigned int loopId);
static PTREE_MOD_OutObj_t *_PTREE_MOD_DET_CreateModOut(PTREE_MOD_Obj_t *mod, unsigned int loopId);
static void                _PTREE_MOD_DET_Destruct(PTREE_MOD_SYS_Obj_t *sysMod);
static void                _PTREE_MOD_DET_Free(PTREE_MOD_SYS_Obj_t *sysMod);

static const PTREE_LINKER_Ops_t G_PTREE_MOD_DET_IN_LINKER_OPS = {
    .enqueue = _PTREE_MOD_DET_InLinkerEnqueue,
    .dequeue = _PTREE_MOD_DET_InLinkerDequeue,
};
static const PTREE_LINKER_Hook_t G_PTREE_MOD_DET_IN_LINKER_HOOK = {};

static const PTREE_MOD_InOps_t G_PTREE_MOD_DET_IN_OPS = {
    .getType       = _PTREE_MOD_DET_InGetType,
    .createNLinker = _PTREE_MOD_DET_InCreateNLinker,
    .createPacker  = _PTREE_MOD_DET_InCreatePacker,
};
static const PTREE_MOD_InHook_t G_PTREE_MOD_DET_IN_HOOK = {};

static const PTREE_MOD_OutOps_t G_PTREE_MOD_DET_OUT_OPS = {
    .getType = _PTREE_MOD_DET_OutGetType,
};
static const PTREE_MOD_OutHook_t G_PTREE_MOD_DET_OUT_HOOK = {};

static const PTREE_MOD_SYS_Ops_t G_PTREE_MOD_DET_SYS_OPS = {
    .init         = _PTREE_MOD_DET_Init,
    .deinit       = _PTREE_MOD_DET_Deinit,
    .createModIn  = _PTREE_MOD_DET_CreateModIn,
    .createModOut = _PTREE_MOD_DET_CreateModOut,
};
static const PTREE_MOD_SYS_Hook_t G_PTREE_MOD_DET_SYS_HOOK = {
    .destruct = _PTREE_MOD_DET_Destruct,
    .free     = _PTREE_MOD_DET_Free,
};

static int _PTREE_MOD_DET_InLinkerEnqueue(PTREE_LINKER_Obj_t *linker, PTREE_PACKET_Obj_t *packet)
{
    PTREE_MOD_DET_Obj_t *     detMod      = CONTAINER_OF(linker, PTREE_MOD_DET_Obj_t, inLinker);
    PTREE_PACKET_Obj_t *      rawPaPacket = NULL;
    PTREE_PACKET_Obj_t *      rawVaPacket = NULL;
    PTREE_MMA_PACKET_RawPa_t *rawPa       = NULL;
    PTREE_PACKET_RAW_Obj_t *  rawVa       = NULL;

    ALGO_Input_t              algoInput;
    MI_S32                    boxNum = 0;
    PTREE_PACKET_Obj_t *      sendPacket;
    PTREE_RGN_PACKET_Rects_t *rects;
    int                       i = 0;

    if (!detMod->detectHandle)
    {
        return SSOS_DEF_FAIL;
    }

    rawPaPacket = PTREE_PACKET_Convert(packet, PTREE_PACKET_INFO_TYPE(raw_pa));
    if (!rawPaPacket)
    {
        PTREE_ERR("packet info type %s is not support", packet->info->type);
        goto ERR_PA_CVT;
    }
    rawVaPacket = PTREE_PACKET_Convert(packet, PTREE_PACKET_INFO_TYPE(raw));
    if (!rawVaPacket)
    {
        PTREE_ERR("packet info type %s is not support", packet->info->type);
        goto ERR_VA_CVT;
    }

    rawPa = CONTAINER_OF(rawPaPacket, PTREE_MMA_PACKET_RawPa_t, base);
    rawVa = CONTAINER_OF(rawVaPacket, PTREE_PACKET_RAW_Obj_t, base);

    memset(&algoInput, 0, sizeof(ALGO_Input_t));

    algoInput.buf_size   = rawVa->rawData.size[0] + rawVa->rawData.size[1];
    algoInput.p_vir_addr = rawVa->rawData.data[0];
    algoInput.phy_addr   = rawPa->rawData.phy[0];
    algoInput.pts        = 0;

    if (MI_SUCCESS != ALGO_DET_Run(detMod->detectHandle, &algoInput, detMod->boxes, &boxNum))
    {
        goto ERR_RET;
    }
    if (boxNum < 0)
    {
        goto ERR_RET;
    }

    sendPacket = PTREE_RGN_PACKET_RectsNew(boxNum);
    if (!sendPacket)
    {
        goto ERR_RET;
    }
    rects = CONTAINER_OF(sendPacket, PTREE_RGN_PACKET_Rects_t, base);

    for (i = 0; i < boxNum; ++i)
    {
        rects->rects[i].x = detMod->boxes[i].x;
        rects->rects[i].y = detMod->boxes[i].y;
        rects->rects[i].w = detMod->boxes[i].width;
        rects->rects[i].h = detMod->boxes[i].height;
    }

    PTREE_LINKER_Enqueue(&detMod->modOut.plinker.base, sendPacket);
    PTREE_PACKET_Del(sendPacket);
    PTREE_PACKET_Del(rawVaPacket);
    PTREE_PACKET_Del(rawPaPacket);
    return SSOS_DEF_OK;

ERR_RET:
    PTREE_PACKET_Del(rawVaPacket);
ERR_VA_CVT:
    PTREE_PACKET_Del(rawPaPacket);
ERR_PA_CVT:
    return SSOS_DEF_FAIL;
}
static PTREE_PACKET_Obj_t *_PTREE_MOD_DET_InLinkerDequeue(PTREE_LINKER_Obj_t *linker, int ms)
{
    (void)linker;
    (void)ms;
    return NULL;
}

static int _PTREE_MOD_DET_InGetType(PTREE_MOD_InObj_t *modIn)
{
    (void)modIn;
    return E_PTREE_MOD_BIND_TYPE_USER;
}
static PTREE_LINKER_Obj_t *_PTREE_MOD_DET_InCreateNLinker(PTREE_MOD_InObj_t *modIn)
{
    PTREE_MOD_DET_Obj_t *detMod = CONTAINER_OF(modIn, PTREE_MOD_DET_Obj_t, modIn);
    return PTREE_LINKER_Dup(&detMod->inLinker);
}
static PTREE_PACKER_Obj_t *_PTREE_MOD_DET_InCreatePacker(PTREE_MOD_InObj_t *modIn, int *isFast)
{
    PTREE_MOD_DET_Obj_t *detMod = CONTAINER_OF(modIn, PTREE_MOD_DET_Obj_t, modIn);
    *isFast                     = SSOS_DEF_FALSE;
    return PTREE_PACKER_Dup(&detMod->packer.base);
}

static int _PTREE_MOD_DET_OutGetType(PTREE_MOD_OutObj_t *modOut)
{
    (void)modOut;
    return E_PTREE_MOD_BIND_TYPE_USER;
}

static int _PTREE_MOD_DET_Init(PTREE_MOD_SYS_Obj_t *sysMod)
{
    PTREE_MOD_DET_Obj_t * detMod  = CONTAINER_OF(sysMod, PTREE_MOD_DET_Obj_t, base);
    PTREE_SUR_DET_Info_t *detInfo = CONTAINER_OF(sysMod->base.info, PTREE_SUR_DET_Info_t, base.base);
    DetectionInfo_t       detectInfo;
    if (MI_SUCCESS != ALGO_DET_CreateHandle(&detMod->detectHandle))
    {
        PTREE_ERR("ALGO_DET_CreateHandle Fail");
        goto ERR_CREATE;
    }
    memset(&detectInfo, 0, sizeof(DetectionInfo_t));
    detectInfo.disp_size.width  = PTREE_RGN_PACKET_COORDINATE_MAX_W;
    detectInfo.disp_size.height = PTREE_RGN_PACKET_COORDINATE_MAX_H;
    detectInfo.threshold        = detInfo->threshold / 1000.0;
    strncpy(detectInfo.ipu_firmware_path, detInfo->fwPath, MAX_DET_STRLEN - 1);
    strncpy(detectInfo.model, detInfo->modelPath, MAX_DET_STRLEN - 1);
    if (MI_SUCCESS != ALGO_DET_InitHandle(detMod->detectHandle, &detectInfo))
    {
        PTREE_ERR("ALGO_DET_InitHandle Fail");
        goto ERR_INIT;
    }
    return SSOS_DEF_OK;

ERR_INIT:
    ALGO_DET_ReleaseHandle(detMod->detectHandle);
    detMod->detectHandle = NULL;
ERR_CREATE:
    return SSOS_DEF_FAIL;
}

static int _PTREE_MOD_DET_Deinit(PTREE_MOD_SYS_Obj_t *sysMod)
{
    PTREE_MOD_DET_Obj_t *detMod = CONTAINER_OF(sysMod, PTREE_MOD_DET_Obj_t, base);
    if (detMod->detectHandle)
    {
        ALGO_DET_DeinitHandle(detMod->detectHandle);
        ALGO_DET_ReleaseHandle(detMod->detectHandle);
        detMod->detectHandle = NULL;
    }
    return SSOS_DEF_OK;
}
static PTREE_MOD_InObj_t *_PTREE_MOD_DET_CreateModIn(PTREE_MOD_Obj_t *mod, unsigned int loopId)
{
    PTREE_MOD_DET_Obj_t *detMod = CONTAINER_OF(mod, PTREE_MOD_DET_Obj_t, base.base);
    (void)loopId;
    return PTREE_MOD_InObjDup(&detMod->modIn);
}
static PTREE_MOD_OutObj_t *_PTREE_MOD_DET_CreateModOut(PTREE_MOD_Obj_t *mod, unsigned int loopId)
{
    PTREE_MOD_DET_Obj_t *detMod = CONTAINER_OF(mod, PTREE_MOD_DET_Obj_t, base.base);
    (void)loopId;
    return PTREE_MOD_OutObjDup(&detMod->modOut);
}
static void _PTREE_MOD_DET_Destruct(PTREE_MOD_SYS_Obj_t *sysMod)
{
    PTREE_MOD_DET_Obj_t *detMod = CONTAINER_OF(sysMod, PTREE_MOD_DET_Obj_t, base);
    PTREE_PACKER_Del(&detMod->packer.base);
    PTREE_LINKER_Del(&detMod->inLinker);
    PTREE_MOD_OutObjDel(&detMod->modOut);
    PTREE_MOD_InObjDel(&detMod->modIn);
}
static void _PTREE_MOD_DET_Free(PTREE_MOD_SYS_Obj_t *sysMod)
{
    PTREE_MOD_DET_Obj_t *detMod = CONTAINER_OF(sysMod, PTREE_MOD_DET_Obj_t, base);
    SSOS_MEM_Free(detMod);
}
PTREE_MOD_Obj_t *PTREE_MOD_DET_New(PARENA_Tag_t *tag)
{
    PTREE_MOD_DET_Obj_t *detMod = NULL;

    detMod = SSOS_MEM_Alloc(sizeof(PTREE_MOD_DET_Obj_t));
    if (!detMod)
    {
        PTREE_ERR("Alloc err");
        goto ERR_MEM_ALLOC;
    }
    memset(detMod, 0, sizeof(PTREE_MOD_DET_Obj_t));
    if (SSOS_DEF_OK != PTREE_MOD_SYS_ObjInit(&detMod->base, &G_PTREE_MOD_DET_SYS_OPS, tag, E_MI_MODULE_ID_MAX))
    {
        goto ERR_MOD_SYS_INIT;
    }

    if (detMod->base.base.info->inCnt != 1)
    {
        PTREE_ERR("Det mod input count need 1 but %d", detMod->base.base.info->inCnt);
        goto ERR_IO_COUNT;
    }
    if (detMod->base.base.info->outCnt != 1)
    {
        PTREE_ERR("Det mod output count need 1 but %d", detMod->base.base.info->outCnt);
        goto ERR_IO_COUNT;
    }

    if (SSOS_DEF_OK != PTREE_MOD_InObjInit(&detMod->modIn, &G_PTREE_MOD_DET_IN_OPS, &detMod->base.base, 0))
    {
        goto ERR_MOD_IN_INIT;
    }

    if (SSOS_DEF_OK != PTREE_MOD_OutObjInit(&detMod->modOut, &G_PTREE_MOD_DET_OUT_OPS, &detMod->base.base, 0))
    {
        goto ERR_MOD_OUT_INIT;
    }

    if (SSOS_DEF_OK != PTREE_LINKER_Init(&detMod->inLinker, &G_PTREE_MOD_DET_IN_LINKER_OPS))
    {
        goto ERR_IN_LINKER_INIT;
    }

    if (SSOS_DEF_OK != PTREE_MMA_PACKET_PackerInit(&detMod->packer))
    {
        goto ERR_PACKER_INIT;
    }

    PTREE_MOD_InObjRegister(&detMod->modIn, &G_PTREE_MOD_DET_IN_HOOK);
    PTREE_MOD_OutObjRegister(&detMod->modOut, &G_PTREE_MOD_DET_OUT_HOOK);
    PTREE_LINKER_Register(&detMod->inLinker, &G_PTREE_MOD_DET_IN_LINKER_HOOK);
    PTREE_MOD_SYS_ObjRegister(&detMod->base, &G_PTREE_MOD_DET_SYS_HOOK);
    return &detMod->base.base;

ERR_PACKER_INIT:
    PTREE_LINKER_Del(&detMod->inLinker);
ERR_IN_LINKER_INIT:
    PTREE_MOD_OutObjDel(&detMod->modOut);
ERR_MOD_OUT_INIT:
    PTREE_MOD_InObjDel(&detMod->modIn);
ERR_MOD_IN_INIT:
ERR_IO_COUNT:
    PTREE_MOD_ObjDel(&detMod->base.base);
ERR_MOD_SYS_INIT:
    SSOS_MEM_Free(detMod);
ERR_MEM_ALLOC:
    return NULL;
}

PTREE_MAKER_MOD_INIT(DET, PTREE_MOD_DET_New);
#endif
