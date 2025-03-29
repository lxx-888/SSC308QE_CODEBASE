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
#include "ssos_list.h"
#include "ptree_log.h"
#include "ptree_maker.h"
#include "ssos_mem.h"
#include "ptree_mma_packet.h"
#include "ptree_packet.h"
#include "ptree_uvc.h"

typedef struct PTREE_MOD_UVC_Obj_s   PTREE_MOD_UVC_Obj_t;
typedef struct PTREE_MOD_UVC_InObj_s PTREE_MOD_UVC_InObj_t;

struct PTREE_MOD_UVC_Obj_s
{
    PTREE_MOD_Obj_t base;
    int             inport;
    ST_UVC_ARGS     param;
};
struct PTREE_MOD_UVC_InObj_s
{
    PTREE_MOD_InObj_t base;
};

static int  _PTREE_MOD_UVC_InGetType(PTREE_MOD_InObj_t *modIn);
static int  _PTREE_MOD_UVC_InIsPostReader(PTREE_MOD_InObj_t *modIn);
static void _PTREE_MOD_UVC_InFree(PTREE_MOD_InObj_t *modIn);

static int                _PTREE_MOD_UVC_Init(PTREE_MOD_Obj_t *mod);
static int                _PTREE_MOD_UVC_Deinit(PTREE_MOD_Obj_t *mod);
static int                _PTREE_MOD_UVC_Start(PTREE_MOD_Obj_t *mod);
static int                _PTREE_MOD_UVC_Stop(PTREE_MOD_Obj_t *mod);
static void               _PTREE_MOD_UVC_Free(PTREE_MOD_Obj_t *mod);
static PTREE_MOD_InObj_t *_PTREE_MOD_UVC_CreateModIn(PTREE_MOD_Obj_t *mod, unsigned int loopId);

static const PTREE_MOD_Ops_t G_PTREE_MOD_UVC_OPS = {
    .init        = _PTREE_MOD_UVC_Init,
    .deinit      = _PTREE_MOD_UVC_Deinit,
    .start       = _PTREE_MOD_UVC_Start,
    .stop        = _PTREE_MOD_UVC_Stop,
    .createModIn = _PTREE_MOD_UVC_CreateModIn,
};

static const PTREE_MOD_Hook_t G_PTREE_MOD_UVC_HOOK = {
    .free = _PTREE_MOD_UVC_Free,
};

static const PTREE_MOD_InOps_t G_PTREE_MOD_UVC_IN_OPS = {
    .getType      = _PTREE_MOD_UVC_InGetType,
    .isPostReader = _PTREE_MOD_UVC_InIsPostReader,
};

static const PTREE_MOD_InHook_t G_PTREE_MOD_UVC_IN_HOOK = {
    .destruct = NULL,
    .free     = _PTREE_MOD_UVC_InFree,
};

// callback func
static int _Uvc_Init(void *p)
{
    (void)p;
    return 0;
}
static int _Uvc_Deinit(void *p)
{
    (void)p;
    return 0;
}
static int _Start_Capture(void *pArgs)
{
    int                  id     = 0;
    ST_UVC_ARGS         *param  = (ST_UVC_ARGS *)pArgs;
    PTREE_MOD_UVC_Obj_t *uvcMod = CONTAINER_OF(param, PTREE_MOD_UVC_Obj_t, param);
    unsigned int         width  = param->attr.stream_params.width;
    unsigned int         height = param->attr.stream_params.height;
    PTREE_MOD_InObj_t   *inMod  = NULL;
    switch (param->attr.stream_params.fcc)
    {
        case V4L2_PIX_FMT_H264:
            id = 0;
            break;
        case V4L2_PIX_FMT_H265:
            id = 1;
            break;
        case V4L2_PIX_FMT_MJPG:
            id = 2;
            break;
        case V4L2_PIX_FMT_NV12:
            id = 3;
            break;
        case V4L2_PIX_FMT_YUYV:
            id = 4;
            break;
        default:
            id = -1;
            PTREE_ERR("inport %d, not support\n", id);
            return -1;
    }
    inMod = PTREE_MOD_GetInObjByPort(&uvcMod->base, id);
    PTREE_MOD_CreateDelayPass(inMod);
    PTREE_MOD_InitDelayPass(inMod);
    PTREE_MOD_BindDelayPass(inMod);
    PTREE_MOD_ResetStream(inMod, width, height);
    PTREE_MOD_StartDelayPass(inMod);
    PTREE_MESSAGE_Access(&inMod->message);
    uvcMod->inport = id;
    return 0;
}
static int _Fill_Buffer(void *pArgs, char **buf, MI_PHY *phy, u32 *size, u32 vsize)
{
    PTREE_MOD_UVC_Obj_t *uvcMod   = CONTAINER_OF((ST_UVC_ARGS *)pArgs, PTREE_MOD_UVC_Obj_t, param);
    PTREE_PACKET_Obj_t  *packet   = NULL;
    int                  id       = uvcMod->inport;
    char                *frameBuf = NULL;
    MI_PHY               frameDma;
    u32                  allSize         = 0;
    u32                  bufSizeRequired = 0;
    ST_UVC_ARGS         *param           = (ST_UVC_ARGS *)pArgs;

    packet           = PTREE_LINKER_Dequeue(&uvcMod->base.arrModIn[id]->plinker.base, 100);
    param->packet    = packet;
    param->covertPtk = NULL;
    if (!packet)
    {
        return SSOS_DEF_ENOMEM;
    }
    if (uvcMod->param.attr.stream_params.fcc == V4L2_PIX_FMT_NV12
        || uvcMod->param.attr.stream_params.fcc == V4L2_PIX_FMT_YUYV)
    {
        PTREE_MMA_PACKET_RawPa_t *rawPa    = CONTAINER_OF(packet, PTREE_MMA_PACKET_RawPa_t, base);
        PTREE_PACKET_Obj_t       *packetva = PTREE_PACKET_Convert(packet, PTREE_PACKET_INFO_TYPE(raw));
        param->covertPtk                   = packetva;
        if (!packetva)
        {
            return SSOS_DEF_ENOMEM;
        }
        PTREE_PACKET_RAW_Obj_t *rawVa = CONTAINER_OF(packetva, PTREE_PACKET_RAW_Obj_t, base);
        frameBuf                      = rawVa->rawData.data[0];
        allSize                       = rawVa->rawData.size[0] + rawVa->rawData.size[1];
        frameDma                      = rawPa->rawData.phy[0];

        *buf  = frameBuf;
        *phy  = frameDma;
        *size = allSize;
        return SSOS_DEF_OK;
    }
    PTREE_MMA_PACKET_VideoPa_t *uvcPacket = CONTAINER_OF(packet, PTREE_MMA_PACKET_VideoPa_t, base.base);
    for (int i = 0; i < uvcPacket->base.info.packetInfo.pktCount; i++)
    {
        bufSizeRequired += uvcPacket->base.info.packetInfo.pktInfo[i].size;
    }
    if (bufSizeRequired > vsize)
    {
        PTREE_ERR("size exceed max !!!!, size_max: %d, realbufSize:%d\n",
                  (param->attr.stream_params.width * param->attr.stream_params.height * 3), bufSizeRequired);
        return SSOS_DEF_FAIL;
    }
    if (uvcPacket->base.info.packetInfo.pktCount > 1)
    {
        frameBuf = param->puvc_frm_buf;
        frameDma = param->uvc_frm_dma;
        for (int i = 0; i < uvcPacket->base.info.packetInfo.pktCount; i++)
        {
            CamOsMemcpy(frameBuf + allSize, uvcPacket->base.packetData.pktData[i].data,
                        uvcPacket->base.info.packetInfo.pktInfo[i].size);
            allSize += uvcPacket->base.info.packetInfo.pktInfo[i].size;
        }
    }
    else
    {
        allSize  = uvcPacket->base.info.packetInfo.pktInfo[0].size;
        frameBuf = uvcPacket->base.packetData.pktData[0].data;
        frameDma = uvcPacket->pa[0];
    }
    *buf  = frameBuf;
    *phy  = frameDma;
    *size = allSize;
    return SSOS_DEF_OK;
}
static int _Finish_Buffer(void *pArgs)
{
    ST_UVC_ARGS *param = (ST_UVC_ARGS *)pArgs;
    if (param->packet != NULL)
    {
        PTREE_PACKET_Del((PTREE_PACKET_Obj_t *)param->packet);
    }
    if (param->covertPtk != NULL)
    {
        PTREE_PACKET_Del((PTREE_PACKET_Obj_t *)param->covertPtk);
    }
    return SSOS_DEF_OK;
}
static int _Stop_Capture(void *pArgs)
{
    PTREE_MOD_UVC_Obj_t *uvcMod = CONTAINER_OF((ST_UVC_ARGS *)pArgs, PTREE_MOD_UVC_Obj_t, param);
    int                  id     = uvcMod->inport;
    PTREE_MOD_InObj_t   *inMod  = PTREE_MOD_GetInObjByPort(&uvcMod->base, id);
    PTREE_MOD_StopDelayPass(inMod);
    PTREE_MOD_UnbindDelayPass(inMod);
    PTREE_MOD_DeinitDelayPass(inMod);
    PTREE_MOD_DestroyDelayPass(inMod);
    PTREE_MESSAGE_Leave(&inMod->message);
    return SSOS_DEF_OK;
}
// callback func end
static int _PTREE_MOD_UVC_InGetType(PTREE_MOD_InObj_t *modIn)
{
    (void)modIn;
    return E_PTREE_MOD_BIND_TYPE_USER;
}
static int _PTREE_MOD_UVC_InIsPostReader(PTREE_MOD_InObj_t *modIn)
{
    (void)modIn;
    return SSOS_DEF_TRUE;
}
static void _PTREE_MOD_UVC_InFree(PTREE_MOD_InObj_t *modIn)
{
    SSOS_MEM_Free(CONTAINER_OF(modIn, PTREE_MOD_UVC_InObj_t, base));
}

static PTREE_MOD_InObj_t *_PTREE_MOD_UVC_InNew(PTREE_MOD_Obj_t *mod, unsigned int loopId)
{
    PTREE_MOD_UVC_InObj_t *uvcModIn = NULL;
    uvcModIn                        = SSOS_MEM_Alloc(sizeof(PTREE_MOD_UVC_InObj_t));
    if (!uvcModIn)
    {
        PTREE_ERR("Alloc err\n");
        goto ERR_MEM_ALLOC;
    }
    memset(uvcModIn, 0, sizeof(PTREE_MOD_UVC_InObj_t));
    if (SSOS_DEF_OK != PTREE_MOD_InObjInit(&uvcModIn->base, &G_PTREE_MOD_UVC_IN_OPS, mod, loopId))
    {
        goto ERR_MOD_IN_INIT;
    }
    PTREE_MOD_InObjRegister(&uvcModIn->base, &G_PTREE_MOD_UVC_IN_HOOK);
    return &uvcModIn->base;
ERR_MOD_IN_INIT:
    SSOS_MEM_Free(uvcModIn);
ERR_MEM_ALLOC:
    return NULL;
}

static int _PTREE_MOD_UVC_Init(PTREE_MOD_Obj_t *mod)
{
    (void)mod;
    return SSOS_DEF_OK;
}
static int _PTREE_MOD_UVC_Deinit(PTREE_MOD_Obj_t *mod)
{
    (void)mod;
    return SSOS_DEF_OK;
}
static int _PTREE_MOD_UVC_Start(PTREE_MOD_Obj_t *mod)
{
    PTREE_MOD_UVC_Obj_t *uvcMod = CONTAINER_OF(mod, PTREE_MOD_UVC_Obj_t, base);
    int                  ret    = 0;

    uvcOps_t ops = {
        _Uvc_Init, _Uvc_Deinit, _Start_Capture, _Fill_Buffer, _Finish_Buffer, _Stop_Capture,
    };
    uvcMod->param.uvc_idx = uvcMod->base.info->devId;
    CamOsSprintf(uvcMod->param.threadName, "uvc%d thread", uvcMod->base.info->devId);
    uvcMod->param.ops               = ops;
    uvcMod->param.viMsgQueue        = SSOS_MEM_Alloc(sizeof(CamOsMsgQueue));
    uvcMod->param.viMsgQueueTimeout = 1;
    uvcMod->param.uvc_status        = UVC_STOP_CAPTURE;
    ret                             = PTREE_UVC_Init(&uvcMod->param);
    if (ret != SSOS_DEF_OK)
    {
        PTREE_ERR("init failed\n");
        return SSOS_DEF_FAIL;
    }
    return SSOS_DEF_OK;
}
static int _PTREE_MOD_UVC_Stop(PTREE_MOD_Obj_t *mod)
{
    PTREE_UVC_Deinit(mod->info->devId);
    return SSOS_DEF_OK;
}
static void _PTREE_MOD_UVC_Free(PTREE_MOD_Obj_t *mod)
{
    SSOS_MEM_Free(CONTAINER_OF(mod, PTREE_MOD_UVC_Obj_t, base));
}
static PTREE_MOD_InObj_t *_PTREE_MOD_UVC_CreateModIn(PTREE_MOD_Obj_t *mod, unsigned int loopId)
{
    return _PTREE_MOD_UVC_InNew(mod, loopId);
}
PTREE_MOD_Obj_t *PTREE_MOD_UVC_New(PARENA_Tag_t *tag)
{
    PTREE_MOD_UVC_Obj_t *uvcMod = NULL;

    uvcMod = SSOS_MEM_Alloc(sizeof(PTREE_MOD_UVC_Obj_t));
    if (!uvcMod)
    {
        PTREE_ERR("Alloc err\n");
        return NULL;
    }
    memset(uvcMod, 0, sizeof(PTREE_MOD_UVC_Obj_t));
    if (SSOS_DEF_OK != PTREE_MOD_ObjInit(&uvcMod->base, &G_PTREE_MOD_UVC_OPS, tag))
    {
        SSOS_MEM_Free(uvcMod);
        return NULL;
    }
    PTREE_MOD_ObjRegister(&uvcMod->base, &G_PTREE_MOD_UVC_HOOK);
    return &uvcMod->base;
}

PTREE_MAKER_MOD_INIT(UVC, PTREE_MOD_UVC_New);
