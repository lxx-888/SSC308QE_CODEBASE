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

#include "mi_common_datatype.h"
#include "algo_hseg_api.h"
#include "ssos_def.h"
#include "ssos_io.h"
#include "ptree_log.h"
#include "ssos_mem.h"
#include "ptree_mod.h"
#include "ssos_task.h"
#include "ssos_thread.h"
#include "ptree_packer.h"
#include "ptree_mma_packet.h"
#include "ptree_linker.h"
#include "ptree_plinker_group.h"
#include "ptree_nlinker_async.h"
#include "ptree_nlinker_out.h"
#include "ptree_mod_sys.h"
#include "ptree_sur_hseg.h"
#include "ptree_mod_hseg.h"
#include "ptree_maker.h"
#include "ssos_time.h"

#define PTREE_MOD_HSEG_TASK_NAME_LEN (32)

#define HSEG_IN_LOOP_ORI  0
#define HSEG_IN_LOOP_REP  1
#define HSEG_IN_LOOP_SEND 2

PTREE_ENUM_DEFINE(ALGO_HsegBgbMode_e, {E_ALOG_BGBLUR_MODE_BLUR, "blur"}, {E_ALGO_BGBLUR_MODE_REPLACE, "replace"}, )

PTREE_ENUM_DEFINE(ALGO_BgBlurMaskOp_e, {E_ALGO_BGB_MASK_OP_DILATE, "dilate"}, {E_ALGO_BGB_MASK_OP_NONE, "none"},
                  {E_ALGO_BGB_MASK_OP_ERODE, "erode"}, )

typedef struct PTREE_MOD_HSEG_OutObj_s   PTREE_MOD_HSEG_OutObj_t;
typedef struct PTREE_MOD_HSEG_InObj_s    PTREE_MOD_HSEG_InObj_t;
typedef struct PTREE_MOD_HSEG_DlaInObj_s PTREE_MOD_HSEG_DlaInObj_t;
typedef struct PTREE_MOD_HSEG_Obj_s      PTREE_MOD_HSEG_Obj_t;

struct PTREE_MOD_HSEG_DlaInObj_s
{
    PTREE_MOD_InObj_t         base;
    PTREE_NLINKER_ASYNC_Obj_t asyncLinker;
    PTREE_MMA_PACKET_Packer_t packer;
    void *                    taskHandle;
};
struct PTREE_MOD_HSEG_InObj_s
{
    PTREE_MOD_InObj_t         base;
    PTREE_NLINKER_ASYNC_Obj_t asyncLinker;
    PTREE_MMA_PACKET_Packer_t packer;
};
struct PTREE_MOD_HSEG_OutObj_s
{
    PTREE_MOD_OutObj_t      base;
    PTREE_NLINKER_OUT_Obj_t outLinker;
    PTREE_PACKET_Obj_t *    oriPacket;
    PTREE_PACKET_Obj_t *    repPacket;
    void *                  taskHandle;
};
struct PTREE_MOD_HSEG_Obj_s
{
    PTREE_MOD_SYS_Obj_t   base;
    void *                hsegHandle;
    ALGO_HsegBgBlurCtrl_t stCtrl;
    SSOS_THREAD_Mutex_t  mutexCtrl;

    PTREE_MOD_HSEG_DlaInObj_t dlaIn;

    PTREE_MOD_HSEG_InObj_t oriIn;
    PTREE_MOD_HSEG_InObj_t repIn;

    PTREE_MOD_HSEG_OutObj_t modOut;
};

static PTREE_PACKET_Obj_t *_PTREE_MOD_HSEG_KickPacket(PTREE_PACKER_Obj_t *packer, PTREE_MOD_HSEG_Obj_t *hsegMod,
                                                      int ms);

static int _PTREE_MOD_HSEG_OutLinkerEnqueue(PTREE_NLINKER_OUT_Obj_t *nlinkerOut, PTREE_PACKET_Obj_t *packet);
static PTREE_PACKET_Obj_t *_PTREE_MOD_HSEG_OutLinkerDequeueFromInside(PTREE_NLINKER_OUT_Obj_t *nlinkerOut, int ms);
static int _PTREE_MOD_HSEG_OutLinkerDequeueOut(PTREE_NLINKER_OUT_Obj_t *nlinkerOut, PTREE_PACKET_Obj_t *packet);

static int                 _PTREE_MOD_HSEG_InGetType(PTREE_MOD_InObj_t *modIn);
static int                 _PTREE_MOD_HSEG_InUnlinked(PTREE_MOD_InObj_t *modIn, unsigned int ref);
static PTREE_LINKER_Obj_t *_PTREE_MOD_HSEG_InCreateNLinker(PTREE_MOD_InObj_t *modIn);
static PTREE_PACKER_Obj_t *_PTREE_MOD_HSEG_InCreatePacker(PTREE_MOD_InObj_t *modIn, int *isFast);

static int                 _PTREE_MOD_HSEG_DlaInGetType(PTREE_MOD_InObj_t *modIn);
static int                 _PTREE_MOD_HSEG_DlaInLinked(PTREE_MOD_InObj_t *modIn, unsigned int ref);
static int                 _PTREE_MOD_HSEG_DlaInUnlinked(PTREE_MOD_InObj_t *modIn, unsigned int ref);
static PTREE_LINKER_Obj_t *_PTREE_MOD_HSEG_DlaInCreateNLinker(PTREE_MOD_InObj_t *modIn);
static PTREE_PACKER_Obj_t *_PTREE_MOD_HSEG_DlaInCreatePacker(PTREE_MOD_InObj_t *modIn, int *isFast);

static int                  _PTREE_MOD_HSEG_OutGetType(PTREE_MOD_OutObj_t *modOut);
static PTREE_LINKER_Obj_t * _PTREE_MOD_HSEG_OutCreateNLinker(PTREE_MOD_OutObj_t *modOut);
static PTREE_PACKET_Info_t *_PTREE_MOD_HSEG_OutGetPacketInfo(PTREE_MOD_OutObj_t *modOut);
static int                  _PTREE_MOD_HSEG_OutLinked(PTREE_MOD_OutObj_t *modOut, unsigned int ref);
static int                  _PTREE_MOD_HSEG_OutUnlinked(PTREE_MOD_OutObj_t *modOut, unsigned int ref);

static int                 _PTREE_MOD_HSEG_Init(PTREE_MOD_SYS_Obj_t *sysMod);
static int                 _PTREE_MOD_HSEG_Deinit(PTREE_MOD_SYS_Obj_t *sysMod);
static PTREE_MOD_InObj_t * _PTREE_MOD_HSEG_CreateModIn(PTREE_MOD_Obj_t *mod, unsigned int loopId);
static PTREE_MOD_OutObj_t *_PTREE_MOD_HSEG_CreateModOut(PTREE_MOD_Obj_t *mod, unsigned int loopId);
static void                _PTREE_MOD_HSEG_Destruct(PTREE_MOD_SYS_Obj_t *sysMod);
static void                _PTREE_MOD_HSEG_Free(PTREE_MOD_SYS_Obj_t *sysMod);
static PTREE_MOD_InObj_t * _PTREE_MOD_HSEG_ResetStreamTraverse(PTREE_MOD_OutObj_t *modOut);

static const PTREE_NLINKER_ASYNC_Hook_t G_PTREE_MOD_HSEG_IN_LINKER_HOOK = {};

static const PTREE_NLINKER_OUT_Ops_t G_PTREE_MOD_HSEG_OUT_LINKER_OPS = {
    .enqueue           = _PTREE_MOD_HSEG_OutLinkerEnqueue,
    .dequeueOut        = _PTREE_MOD_HSEG_OutLinkerDequeueOut,
    .dequeueFromInside = _PTREE_MOD_HSEG_OutLinkerDequeueFromInside,
};

static const PTREE_NLINKER_OUT_Hook_t G_PTREE_MOD_HSEG_OUT_LINKER_HOOK = {};
static const PTREE_MOD_InOps_t        G_PTREE_MOD_HSEG_IN_OPS          = {
    .getType       = _PTREE_MOD_HSEG_InGetType,
    .unlinked      = _PTREE_MOD_HSEG_InUnlinked,
    .createNLinker = _PTREE_MOD_HSEG_InCreateNLinker,
    .createPacker  = _PTREE_MOD_HSEG_InCreatePacker,
};
static const PTREE_MOD_InHook_t G_PTREE_MOD_HSEG_IN_HOOK = {};

static const PTREE_MOD_InOps_t G_PTREE_MOD_HSEG_DLA_IN_OPS = {
    .getType       = _PTREE_MOD_HSEG_DlaInGetType,
    .linked        = _PTREE_MOD_HSEG_DlaInLinked,
    .unlinked      = _PTREE_MOD_HSEG_DlaInUnlinked,
    .createNLinker = _PTREE_MOD_HSEG_DlaInCreateNLinker,
    .createPacker  = _PTREE_MOD_HSEG_DlaInCreatePacker,
};
static const PTREE_MOD_InHook_t G_PTREE_MOD_HSEG_DLA_IN_HOOK = {};

static const PTREE_MOD_OutOps_t G_PTREE_MOD_HSEG_OUT_OPS = {
    .linked              = _PTREE_MOD_HSEG_OutLinked,
    .unlinked            = _PTREE_MOD_HSEG_OutUnlinked,
    .getType             = _PTREE_MOD_HSEG_OutGetType,
    .createNLinker       = _PTREE_MOD_HSEG_OutCreateNLinker,
    .getPacketInfo       = _PTREE_MOD_HSEG_OutGetPacketInfo,
    .resetStreamTraverse = _PTREE_MOD_HSEG_ResetStreamTraverse,
};
static const PTREE_MOD_OutHook_t G_PTREE_MOD_HSEG_OUT_HOOK = {};

static const PTREE_MOD_SYS_Ops_t G_PTREE_MOD_HSEG_SYS_OPS = {
    .init         = _PTREE_MOD_HSEG_Init,
    .deinit       = _PTREE_MOD_HSEG_Deinit,
    .createModIn  = _PTREE_MOD_HSEG_CreateModIn,
    .createModOut = _PTREE_MOD_HSEG_CreateModOut,
};
static const PTREE_MOD_SYS_Hook_t G_PTREE_MOD_HSEG_SYS_HOOK = {
    .destruct = _PTREE_MOD_HSEG_Destruct,
    .free     = _PTREE_MOD_HSEG_Free,
};

static PTREE_PACKET_Obj_t *_PTREE_MOD_HSEG_KickPacket(PTREE_PACKER_Obj_t *packer, PTREE_MOD_HSEG_Obj_t *hsegMod, int ms)
{
    AlgoHsegInputInfo_t srcOri;
    AlgoHsegInputInfo_t srcRep;
    AlgoHsegInputInfo_t hsegInputInfo;

    PTREE_PACKET_Obj_t *oriRawPaPacket = NULL;
    PTREE_PACKET_Obj_t *oriRawVaPacket = NULL;
    PTREE_PACKET_Obj_t *repRawPaPacket = NULL;
    PTREE_PACKET_Obj_t *repRawVaPacket = NULL;
    PTREE_PACKET_Obj_t *outRawPaPacket = NULL;
    PTREE_PACKET_Obj_t *outRawVaPacket = NULL;

    PTREE_MMA_PACKET_RawPa_t *oriRawPa = NULL;
    PTREE_PACKET_RAW_Obj_t *  oriRawVa = NULL;
    PTREE_MMA_PACKET_RawPa_t *repRawPa = NULL;
    PTREE_PACKET_RAW_Obj_t *  repRawVa = NULL;
    PTREE_MMA_PACKET_RawPa_t *outRawPa = NULL;
    PTREE_PACKET_RAW_Obj_t *  outRawVa = NULL;

    int     ret  = 0;
    MI_BOOL flag = TRUE;

    if (!hsegMod->hsegHandle)
    {
        SSOS_TIME_MSleep(ms);
        PTREE_ERR("Hseg handle error!");
        return NULL;
    }

    if (!hsegMod->modOut.oriPacket
        && !(hsegMod->modOut.oriPacket = PTREE_NLINKER_ASYNC_WaitPacket(&hsegMod->oriIn.asyncLinker, ms)))
    {
        return NULL;
    }

    if (!hsegMod->modOut.repPacket
        && !(hsegMod->modOut.repPacket = PTREE_NLINKER_ASYNC_WaitPacket(&hsegMod->repIn.asyncLinker, ms)))
    {
        return NULL;
    }

    if (!(oriRawPaPacket = PTREE_PACKET_Convert(hsegMod->modOut.oriPacket, PTREE_PACKET_INFO_TYPE(raw_pa))))
    {
        PTREE_ERR("Convert ori packet to raw_pa from %s failed.", hsegMod->modOut.oriPacket->info->type);
        goto ERR_ORI_PA_CVT;
    }
    if (!(oriRawVaPacket = PTREE_PACKET_Convert(hsegMod->modOut.oriPacket, PTREE_PACKET_INFO_TYPE(raw))))
    {
        PTREE_ERR("Convert ori packet to raw from %s failed.", hsegMod->modOut.oriPacket->info->type);
        goto ERR_ORI_VA_CVT;
    }
    if (!(repRawPaPacket = PTREE_PACKET_Convert(hsegMod->modOut.repPacket, PTREE_PACKET_INFO_TYPE(raw_pa))))
    {
        PTREE_ERR("Convert rep packet to raw_pa from %s failed.", hsegMod->modOut.repPacket->info->type);
        goto ERR_REP_PA_CVT;
    }
    if (!(repRawVaPacket = PTREE_PACKET_Convert(hsegMod->modOut.repPacket, PTREE_PACKET_INFO_TYPE(raw))))
    {
        PTREE_ERR("Convert rep packet to raw from %s failed.", hsegMod->modOut.repPacket->info->type);
        goto ERR_REP_VA_CVT;
    }

    oriRawPa = CONTAINER_OF(oriRawPaPacket, PTREE_MMA_PACKET_RawPa_t, base);
    oriRawVa = CONTAINER_OF(oriRawVaPacket, PTREE_PACKET_RAW_Obj_t, base);
    repRawPa = CONTAINER_OF(repRawPaPacket, PTREE_MMA_PACKET_RawPa_t, base);
    repRawVa = CONTAINER_OF(repRawVaPacket, PTREE_PACKET_RAW_Obj_t, base);

    if (oriRawPa->info.rawInfo.fmt != repRawPa->info.rawInfo.fmt)
    {
        PTREE_ERR("Input format error : (ori: %d, rep: %d).", oriRawPa->info.rawInfo.fmt, repRawPa->info.rawInfo.fmt);
        goto ERR_ORI_REP_FMT_DIFF;
    }

    if (!(outRawPaPacket = PTREE_PACKER_Make(packer, &oriRawPa->info.base)))
    {
        PTREE_ERR("Get out port buffer failed.");
        goto ERR_OUT_PA_MAKE;
    }
    PTREE_PACKET_SetTimeStamp(outRawPaPacket, PTREE_PACKET_GetTimeStamp(oriRawPaPacket));
    if (!(outRawVaPacket = PTREE_PACKET_Convert(outRawPaPacket, PTREE_PACKET_INFO_TYPE(raw))))
    {
        PTREE_ERR("Convert out packet to raw from %s raw_pa.");
        goto ERR_OUT_VA_CVT;
    }

    outRawPa = CONTAINER_OF(outRawPaPacket, PTREE_MMA_PACKET_RawPa_t, base);
    outRawVa = CONTAINER_OF(outRawVaPacket, PTREE_PACKET_RAW_Obj_t, base);

    memset(&srcOri, 0, sizeof(AlgoHsegInputInfo_t));
    memset(&srcRep, 0, sizeof(AlgoHsegInputInfo_t));
    memset(&hsegInputInfo, 0, sizeof(AlgoHsegInputInfo_t));

    srcOri.bufsize            = oriRawPa->rawData.size[0] + oriRawPa->rawData.size[1];
    srcOri.phy_tensor_addr[0] = oriRawPa->rawData.phy[0];
    srcOri.phy_tensor_addr[1] = oriRawPa->rawData.phy[1];
    srcOri.pt_tensor_data[0]  = oriRawVa->rawData.data[0];
    srcOri.pt_tensor_data[1]  = oriRawVa->rawData.data[1];
    srcOri.width              = oriRawPa->info.rawInfo.width;
    srcOri.height             = oriRawPa->info.rawInfo.height;
    srcOri.data_type =
        (E_PTREE_PACKET_RAW_FORMAT_YUV422_YUYV == oriRawPa->info.rawInfo.fmt) ? E_ALGO_YUV422_YUYV : E_ALOG_YUV420SP;

    srcRep.bufsize            = repRawPa->rawData.size[0] + repRawPa->rawData.size[1];
    srcRep.phy_tensor_addr[0] = repRawPa->rawData.phy[0];
    srcRep.phy_tensor_addr[1] = repRawPa->rawData.phy[1];
    srcRep.pt_tensor_data[0]  = repRawVa->rawData.data[0];
    srcRep.pt_tensor_data[1]  = repRawVa->rawData.data[1];
    srcRep.width              = repRawPa->info.rawInfo.width;
    srcRep.height             = repRawPa->info.rawInfo.height;
    srcRep.data_type          = srcOri.data_type;

    hsegInputInfo.phy_tensor_addr[0] = outRawPa->rawData.phy[0];
    hsegInputInfo.phy_tensor_addr[1] = outRawPa->rawData.phy[1];
    hsegInputInfo.pt_tensor_data[0]  = outRawVa->rawData.data[0];
    hsegInputInfo.pt_tensor_data[1]  = outRawVa->rawData.data[1];
    hsegInputInfo.width              = srcOri.width;
    hsegInputInfo.height             = srcOri.height;
    hsegInputInfo.data_type          = srcOri.data_type;

    ret = ALGO_HSEG_SegmentAndBlurBackgroud(hsegMod->hsegHandle, &srcOri, &srcRep, &hsegInputInfo, hsegMod->stCtrl,
                                            &flag);
    if (0 != ret || !flag)
    {
        PTREE_WRN("ALGO_HSEG_SegmentAndBlurBackgroud run failed! ret: %d, flag: %d", ret, flag);
        goto ERR_SEG_AND_BG_BLUR;
    }

    PTREE_PACKET_Del(outRawVaPacket);

    PTREE_PACKET_Del(hsegMod->modOut.oriPacket);
    hsegMod->modOut.oriPacket = NULL;
    PTREE_PACKET_Del(hsegMod->modOut.repPacket);
    hsegMod->modOut.repPacket = NULL;

    goto SEG_AND_BG_BLUR_SUCCESS;

ERR_SEG_AND_BG_BLUR:
ERR_OUT_VA_CVT:
    PTREE_PACKET_Del(outRawVaPacket);
    PTREE_PACKET_Del(outRawPaPacket);
    outRawPaPacket = NULL;
SEG_AND_BG_BLUR_SUCCESS:
ERR_OUT_PA_MAKE:
ERR_ORI_REP_FMT_DIFF:
    PTREE_PACKET_Del(repRawVaPacket);
ERR_REP_VA_CVT:
    PTREE_PACKET_Del(repRawPaPacket);
ERR_REP_PA_CVT:
    PTREE_PACKET_Del(oriRawVaPacket);
ERR_ORI_VA_CVT:
    PTREE_PACKET_Del(oriRawPaPacket);
ERR_ORI_PA_CVT:
    return outRawPaPacket;
}

static void *_PTREE_MOD_HSEG_SendDlaTask(struct SSOS_TASK_Buffer_s *pstBuf)
{
    int ret = 0;
    int i   = 0;

    AlgoHsegInputInfo_t hsegInputInfo;

    PTREE_PACKET_Obj_t *      packet      = NULL;
    PTREE_PACKET_Obj_t *      rawVaPacket = NULL;
    PTREE_PACKET_Obj_t *      rawPaPacket = NULL;
    PTREE_PACKET_RAW_Obj_t *  rawVa       = NULL;
    PTREE_MMA_PACKET_RawPa_t *rawPa       = NULL;

    PTREE_MOD_InObj_t *     modIn     = (PTREE_MOD_InObj_t *)pstBuf->buf;
    PTREE_MOD_HSEG_InObj_t *hsegModIn = CONTAINER_OF(modIn, PTREE_MOD_HSEG_InObj_t, base);
    PTREE_MOD_HSEG_Obj_t *  hsegMod   = CONTAINER_OF(modIn->thisMod, PTREE_MOD_HSEG_Obj_t, base.base);

    memset(&hsegInputInfo, 0, sizeof(AlgoHsegInputInfo_t));

    if (!(packet = PTREE_NLINKER_ASYNC_WaitPacket(&hsegModIn->asyncLinker, 100)))
    {
        return NULL;
    }
    if (!(rawPaPacket = PTREE_PACKET_Convert(packet, PTREE_PACKET_INFO_TYPE(raw_pa))))
    {
        PTREE_ERR("Convert packet to raw_pa from %s failed.", packet->info->type);
        goto ERR_PA_CVT;
    }
    if (!(rawVaPacket = PTREE_PACKET_Convert(packet, PTREE_PACKET_INFO_TYPE(raw))))
    {
        PTREE_ERR("Convert packet to raw from %s failed.", packet->info->type);
        goto ERR_VA_CVT;
    }

    rawPa = CONTAINER_OF(rawPaPacket, PTREE_MMA_PACKET_RawPa_t, base);
    rawVa = CONTAINER_OF(rawVaPacket, PTREE_PACKET_RAW_Obj_t, base);
    for (i = 0; i < 2 && i < MAX_INPUT_NUM; i++)
    {
        hsegInputInfo.bufsize += rawPa->rawData.size[i];
        hsegInputInfo.pt_tensor_data[i]  = rawVa->rawData.data[i];
        hsegInputInfo.phy_tensor_addr[i] = rawPa->rawData.phy[i];
    }
    hsegInputInfo.width  = rawPa->info.rawInfo.width;
    hsegInputInfo.height = rawPa->info.rawInfo.height;
    hsegInputInfo.data_type =
        E_PTREE_PACKET_RAW_FORMAT_YUV422_YUYV == rawPa->info.rawInfo.fmt ? E_ALGO_YUV422_YUYV : E_ALOG_YUV420SP;
    ret = ALGO_HSEG_SendInput(hsegMod->hsegHandle, &hsegInputInfo);
    if (0 != ret)
    {
        PTREE_ERR("ALGO_HSEG_SendInput run failed! ret: %d", ret);
    }

    PTREE_PACKET_Del(rawVaPacket);
ERR_VA_CVT:
    PTREE_PACKET_Del(rawPaPacket);
ERR_PA_CVT:
    PTREE_PACKET_Del(packet);
    return NULL;
}
static void *_PTREE_MOD_HSEG_DoBgBlurTask(struct SSOS_TASK_Buffer_s *pstBuf)
{
    PTREE_PACKET_Obj_t *  packet  = NULL;
    PTREE_MOD_OutObj_t *  modOut  = (PTREE_MOD_OutObj_t *)pstBuf->buf;
    PTREE_MOD_HSEG_Obj_t *hsegMod = CONTAINER_OF(modOut, PTREE_MOD_HSEG_Obj_t, modOut.base);
    packet                        = _PTREE_MOD_HSEG_KickPacket(&modOut->packer.base, hsegMod, 10);
    if (NULL != packet)
    {
        PTREE_LINKER_Enqueue(&modOut->plinker.base, packet);
        PTREE_PACKET_Del(packet);
    }
    return NULL;
}

static int _PTREE_MOD_HSEG_OutLinkerEnqueue(PTREE_NLINKER_OUT_Obj_t *nlinkerOut, PTREE_PACKET_Obj_t *packet)
{
    (void)nlinkerOut;
    (void)packet;
    return SSOS_DEF_OK;
}
static PTREE_PACKET_Obj_t *_PTREE_MOD_HSEG_OutLinkerDequeueFromInside(PTREE_NLINKER_OUT_Obj_t *nlinkerOut, int ms)
{
    PTREE_MMA_PACKET_Packer_t mmaPacker;
    PTREE_MOD_HSEG_OutObj_t * hsegModOut = CONTAINER_OF(nlinkerOut, PTREE_MOD_HSEG_OutObj_t, outLinker);
    PTREE_MOD_HSEG_Obj_t *    hsegMod    = CONTAINER_OF(hsegModOut, PTREE_MOD_HSEG_Obj_t, modOut);

    memset(&mmaPacker, 0, sizeof(PTREE_MMA_PACKET_Packer_t));
    if (SSOS_DEF_OK != PTREE_MMA_PACKET_PackerInit(&mmaPacker))
    {
        return NULL;
    }

    return _PTREE_MOD_HSEG_KickPacket(&mmaPacker.base, hsegMod, ms);
}
static int _PTREE_MOD_HSEG_OutLinkerDequeueOut(PTREE_NLINKER_OUT_Obj_t *nlinkerOut, PTREE_PACKET_Obj_t *packet)
{
    (void)nlinkerOut;
    (void)packet;
    return SSOS_DEF_OK;
}

static int _PTREE_MOD_HSEG_InGetType(PTREE_MOD_InObj_t *modIn)
{
    (void)modIn;
    return E_PTREE_MOD_BIND_TYPE_USER;
}
static int _PTREE_MOD_HSEG_InUnlinked(PTREE_MOD_InObj_t *modIn, unsigned int ref)
{
    PTREE_MOD_HSEG_InObj_t *hsegModIn = CONTAINER_OF(modIn, PTREE_MOD_HSEG_InObj_t, base);
    if (ref)
    {
        return SSOS_DEF_OK;
    }
    PTREE_NLINKER_ASYNC_Clear(&hsegModIn->asyncLinker);
    return SSOS_DEF_OK;
}
static PTREE_LINKER_Obj_t *_PTREE_MOD_HSEG_InCreateNLinker(PTREE_MOD_InObj_t *modIn)
{
    PTREE_MOD_HSEG_InObj_t *hsegModIn = CONTAINER_OF(modIn, PTREE_MOD_HSEG_InObj_t, base);
    return PTREE_LINKER_Dup(&hsegModIn->asyncLinker.base);
}
static PTREE_PACKER_Obj_t *_PTREE_MOD_HSEG_InCreatePacker(PTREE_MOD_InObj_t *modIn, int *isFast)
{
    PTREE_MOD_HSEG_InObj_t *hsegModIn = CONTAINER_OF(modIn, PTREE_MOD_HSEG_InObj_t, base);
    *isFast                           = SSOS_DEF_FALSE;
    return PTREE_PACKER_Dup(&hsegModIn->packer.base);
}

static int _PTREE_MOD_HSEG_DlaInGetType(PTREE_MOD_InObj_t *modIn)
{
    (void)modIn;
    return E_PTREE_MOD_BIND_TYPE_USER;
}
static int _PTREE_MOD_HSEG_DlaInLinked(PTREE_MOD_InObj_t *modIn, unsigned int ref)
{
    PTREE_MOD_HSEG_DlaInObj_t *hsegDlaIn = CONTAINER_OF(modIn, PTREE_MOD_HSEG_DlaInObj_t, base);
    SSOS_TASK_Attr_t          taskAttr;
    char                       taskName[PTREE_MOD_HSEG_TASK_NAME_LEN] = "";

    if (ref)
    {
        return SSOS_DEF_OK;
    }

    memset(&taskAttr, 0, sizeof(SSOS_TASK_Attr_t));
    taskAttr.threadAttr.name = PTREE_MOD_InKeyStr(&hsegDlaIn->base, taskName, PTREE_MOD_HSEG_TASK_NAME_LEN);
    taskAttr.inBuf.buf       = (void *)modIn;
    taskAttr.doMonitor       = _PTREE_MOD_HSEG_SendDlaTask;

    hsegDlaIn->taskHandle = SSOS_TASK_Open(&taskAttr);
    if (!hsegDlaIn->taskHandle)
    {
        PTREE_ERR("Monitor(%s) return error!", taskName);
        return SSOS_DEF_FAIL;
    }
    SSOS_TASK_StartMonitor(hsegDlaIn->taskHandle);
    return SSOS_DEF_OK;
}
static int _PTREE_MOD_HSEG_DlaInUnlinked(PTREE_MOD_InObj_t *modIn, unsigned int ref)
{
    PTREE_MOD_HSEG_DlaInObj_t *hsegDlaIn = CONTAINER_OF(modIn, PTREE_MOD_HSEG_DlaInObj_t, base);
    if (ref)
    {
        return SSOS_DEF_OK;
    }
    if (!hsegDlaIn->taskHandle)
    {
        return SSOS_DEF_OK;
    }
    SSOS_TASK_Stop(hsegDlaIn->taskHandle);
    SSOS_TASK_Close(hsegDlaIn->taskHandle);
    PTREE_NLINKER_ASYNC_Clear(&hsegDlaIn->asyncLinker);
    hsegDlaIn->taskHandle = NULL;
    return SSOS_DEF_OK;
}
static PTREE_LINKER_Obj_t *_PTREE_MOD_HSEG_DlaInCreateNLinker(PTREE_MOD_InObj_t *modIn)
{
    PTREE_MOD_HSEG_DlaInObj_t *hsegDlaIn = CONTAINER_OF(modIn, PTREE_MOD_HSEG_DlaInObj_t, base);
    return PTREE_LINKER_Dup(&hsegDlaIn->asyncLinker.base);
}
static PTREE_PACKER_Obj_t *_PTREE_MOD_HSEG_DlaInCreatePacker(PTREE_MOD_InObj_t *modIn, int *isFast)
{
    PTREE_MOD_HSEG_DlaInObj_t *hsegDlaIn = CONTAINER_OF(modIn, PTREE_MOD_HSEG_DlaInObj_t, base);
    *isFast                              = SSOS_DEF_FALSE;
    return PTREE_PACKER_Dup(&hsegDlaIn->packer.base);
}
static int _PTREE_MOD_HSEG_OutGetType(PTREE_MOD_OutObj_t *modOut)
{
    (void)modOut;
    return E_PTREE_MOD_BIND_TYPE_USER;
}
static PTREE_LINKER_Obj_t *_PTREE_MOD_HSEG_OutCreateNLinker(PTREE_MOD_OutObj_t *modOut)
{
    PTREE_MOD_HSEG_OutObj_t *hsegModOut = CONTAINER_OF(modOut, PTREE_MOD_HSEG_OutObj_t, base);
    return PTREE_LINKER_Dup(&hsegModOut->outLinker.base);
}
static PTREE_PACKET_Info_t *_PTREE_MOD_HSEG_OutGetPacketInfo(PTREE_MOD_OutObj_t *modOut)
{
    PTREE_MOD_HSEG_Obj_t *hsegMod = CONTAINER_OF(modOut, PTREE_MOD_HSEG_Obj_t, modOut.base);
    return PTREE_MESSAGE_GetPacketInfo(&hsegMod->oriIn.base.message);
}
static int _PTREE_MOD_HSEG_OutLinked(PTREE_MOD_OutObj_t *modOut, unsigned int ref)
{
    PTREE_MOD_HSEG_OutObj_t *hsegModOut = CONTAINER_OF(modOut, PTREE_MOD_HSEG_OutObj_t, base);
    SSOS_TASK_Attr_t        taskAttr;
    char                     taskName[PTREE_MOD_HSEG_TASK_NAME_LEN] = "";

    if (ref > 0)
    {
        return SSOS_DEF_OK;
    }

    if (PTREE_PLINKER_GROUP_Empty(&modOut->plinker) || modOut->nlinker)
    {
        return SSOS_DEF_OK;
    }
    memset(&taskAttr, 0, sizeof(SSOS_TASK_Attr_t));
    PTREE_MOD_OutKeyStr(&hsegModOut->base, taskName, PTREE_MOD_HSEG_TASK_NAME_LEN);
    taskAttr.threadAttr.name = taskName;
    taskAttr.inBuf.buf       = (void *)modOut;
    taskAttr.doMonitor       = _PTREE_MOD_HSEG_DoBgBlurTask;

    hsegModOut->taskHandle = SSOS_TASK_Open(&taskAttr);
    if (!hsegModOut->taskHandle)
    {
        PTREE_ERR("Monitor(%s) return error!", taskName);
        return SSOS_DEF_FAIL;
    }
    SSOS_TASK_StartMonitor(hsegModOut->taskHandle);
    PTREE_DBG("Hseg bg blur task start");
    return SSOS_DEF_OK;
}
static int _PTREE_MOD_HSEG_OutUnlinked(PTREE_MOD_OutObj_t *modOut, unsigned int ref)
{
    PTREE_MOD_HSEG_OutObj_t *hsegModOut = CONTAINER_OF(modOut, PTREE_MOD_HSEG_OutObj_t, base);
    if (ref > 0)
    {
        return SSOS_DEF_OK;
    }
    if (!hsegModOut->taskHandle)
    {
        return SSOS_DEF_OK;
    }
    SSOS_TASK_Stop(hsegModOut->taskHandle);
    SSOS_TASK_Close(hsegModOut->taskHandle);
    hsegModOut->taskHandle = NULL;
    PTREE_DBG("Hseg bg blur task stop");
    return SSOS_DEF_OK;
}

static int _PTREE_MOD_HSEG_Init(PTREE_MOD_SYS_Obj_t *sysMod)
{
    int                    ret = 0;
    InitHsegParam_t        initParam;
    PTREE_MOD_HSEG_Obj_t * hsegMod  = NULL;
    PTREE_SUR_HSEG_Info_t *hsegInfo = NULL;

    hsegMod  = CONTAINER_OF(sysMod, PTREE_MOD_HSEG_Obj_t, base);
    hsegInfo = CONTAINER_OF(sysMod->base.info, PTREE_SUR_HSEG_Info_t, base.base);

    if (strlen(hsegInfo->chIpuPath) + 1 > IPU_MAX_LENGTH || strlen(hsegInfo->chModelPath) + 1 > MODEL_MAX_LENGTH)
    {
        PTREE_ERR("hseg init failed! ipu path or model path is too long, ipu path: %s,  model Path: %s",
                  hsegInfo->chIpuPath, hsegInfo->chModelPath);
        return -1;
    }
    memset(&initParam, 0, sizeof(InitHsegParam_t));
    memcpy(initParam.ipu_firware_bin, hsegInfo->chIpuPath, strlen(hsegInfo->chIpuPath) + 1);
    memcpy(initParam.seg_model_path, hsegInfo->chModelPath, strlen(hsegInfo->chModelPath) + 1);
    ret = ALGO_HSEG_CreateHandle(&hsegMod->hsegHandle);
    if (0 != ret || !hsegMod->hsegHandle)
    {
        PTREE_ERR("create handle failed! ret: %d", ret);
        return -1;
    }
    ret = ALGO_HSEG_Init(hsegMod->hsegHandle, initParam);
    if (0 != ret)
    {
        PTREE_ERR("create handle failed! ret: %d", ret);
        ALGO_HSEG_ReleaseHandle(hsegMod->hsegHandle);
        hsegMod->hsegHandle = NULL;
        return -1;
    }
    return ret;
}
static int _PTREE_MOD_HSEG_Deinit(PTREE_MOD_SYS_Obj_t *sysMod)
{
    PTREE_MOD_HSEG_Obj_t *hsegMod = NULL;

    hsegMod = CONTAINER_OF(sysMod, PTREE_MOD_HSEG_Obj_t, base);
    if (hsegMod->hsegHandle)
    {
        ALGO_HSEG_DeInit(hsegMod->hsegHandle);
        ALGO_HSEG_ReleaseHandle(hsegMod->hsegHandle);
        hsegMod->hsegHandle = NULL;
    }

    return SSOS_DEF_OK;
}
static PTREE_MOD_InObj_t *_PTREE_MOD_HSEG_CreateModIn(PTREE_MOD_Obj_t *mod, unsigned int loopId)
{
    PTREE_MOD_HSEG_Obj_t *hsegMod = CONTAINER_OF(mod, PTREE_MOD_HSEG_Obj_t, base.base);

    switch (loopId)
    {
        case HSEG_IN_LOOP_ORI:
            return PTREE_MOD_InObjDup(&hsegMod->oriIn.base);
        case HSEG_IN_LOOP_REP:
            return PTREE_MOD_InObjDup(&hsegMod->repIn.base);
        case HSEG_IN_LOOP_SEND:
            return PTREE_MOD_InObjDup(&hsegMod->dlaIn.base);
        default:
            PTREE_ERR("loopId %d is error", loopId);
            break;
    }
    return NULL;
}
static PTREE_MOD_OutObj_t *_PTREE_MOD_HSEG_CreateModOut(PTREE_MOD_Obj_t *mod, unsigned int loopId)
{
    PTREE_MOD_HSEG_Obj_t *hsegMod = CONTAINER_OF(mod, PTREE_MOD_HSEG_Obj_t, base.base);
    (void)loopId;
    return PTREE_MOD_OutObjDup(&hsegMod->modOut.base);
}
static void _PTREE_MOD_HSEG_Destruct(PTREE_MOD_SYS_Obj_t *sysMod)
{
    PTREE_MOD_HSEG_Obj_t *hsegMod = CONTAINER_OF(sysMod, PTREE_MOD_HSEG_Obj_t, base);

    SSOS_THREAD_MutexDeinit(&hsegMod->mutexCtrl);
    PTREE_PACKER_Del(&hsegMod->dlaIn.packer.base);
    PTREE_PACKER_Del(&hsegMod->repIn.packer.base);
    PTREE_PACKER_Del(&hsegMod->oriIn.packer.base);
    PTREE_LINKER_Del(&hsegMod->modOut.outLinker.base);
    PTREE_LINKER_Del(&hsegMod->dlaIn.asyncLinker.base);
    PTREE_LINKER_Del(&hsegMod->repIn.asyncLinker.base);
    PTREE_LINKER_Del(&hsegMod->oriIn.asyncLinker.base);
    PTREE_MOD_OutObjDel(&hsegMod->modOut.base);
    PTREE_MOD_InObjDel(&hsegMod->dlaIn.base);
    PTREE_MOD_InObjDel(&hsegMod->repIn.base);
    PTREE_MOD_InObjDel(&hsegMod->oriIn.base);
}
static void _PTREE_MOD_HSEG_Free(PTREE_MOD_SYS_Obj_t *sysMod)
{
    PTREE_MOD_HSEG_Obj_t *hsegMod = CONTAINER_OF(sysMod, PTREE_MOD_HSEG_Obj_t, base);
    SSOS_MEM_Free(hsegMod);
}
static PTREE_MOD_InObj_t *_PTREE_MOD_HSEG_ResetStreamTraverse(PTREE_MOD_OutObj_t *modOut)
{
    if (modOut->thisMod->info->inCnt <= HSEG_IN_LOOP_ORI)
    {
        return NULL;
    }
    return modOut->thisMod->arrModIn[HSEG_IN_LOOP_ORI];
}
PTREE_MOD_Obj_t *PTREE_MOD_HSEG_New(PARENA_Tag_t *tag)
{
    PTREE_MOD_HSEG_Obj_t * hsegMod  = NULL;
    PTREE_SUR_HSEG_Info_t *hsegInfo = NULL;

    hsegMod = SSOS_MEM_Alloc(sizeof(PTREE_MOD_HSEG_Obj_t));
    if (!hsegMod)
    {
        PTREE_ERR("Alloc err");
        goto ERR_MEM_ALLOC;
    }
    memset(hsegMod, 0, sizeof(PTREE_MOD_HSEG_Obj_t));
    if (SSOS_DEF_OK != PTREE_MOD_SYS_ObjInit(&hsegMod->base, &G_PTREE_MOD_HSEG_SYS_OPS, tag, E_MI_MODULE_ID_MAX))
    {
        goto ERR_MOD_SYS_INIT;
    }

    hsegInfo = CONTAINER_OF(hsegMod->base.base.info, PTREE_SUR_HSEG_Info_t, base.base);

    if (hsegMod->base.base.info->inCnt != 3)
    {
        PTREE_ERR("hseg mod input count need 3 but %d", hsegMod->base.base.info->inCnt);
        goto ERR_IO_COUNT;
    }
    if (hsegMod->base.base.info->outCnt != 1)
    {
        PTREE_ERR("hseg mod output count need 1 but %d", hsegMod->base.base.info->outCnt);
        goto ERR_IO_COUNT;
    }

    if (SSOS_DEF_OK
        != PTREE_MOD_InObjInit(&hsegMod->oriIn.base, &G_PTREE_MOD_HSEG_IN_OPS, &hsegMod->base.base, HSEG_IN_LOOP_ORI))
    {
        goto ERR_MOD_ORI_IN_INIT;
    }
    if (SSOS_DEF_OK
        != PTREE_MOD_InObjInit(&hsegMod->repIn.base, &G_PTREE_MOD_HSEG_IN_OPS, &hsegMod->base.base, HSEG_IN_LOOP_REP))
    {
        goto ERR_MOD_REP_IN_INIT;
    }
    if (SSOS_DEF_OK
        != PTREE_MOD_InObjInit(&hsegMod->dlaIn.base, &G_PTREE_MOD_HSEG_DLA_IN_OPS, &hsegMod->base.base,
                               HSEG_IN_LOOP_SEND))
    {
        goto ERR_MOD_DLA_IN_INIT;
    }
    if (SSOS_DEF_OK != PTREE_MOD_OutObjInit(&hsegMod->modOut.base, &G_PTREE_MOD_HSEG_OUT_OPS, &hsegMod->base.base, 0))
    {
        goto ERR_MOD_OUT_INIT;
    }
    if (SSOS_DEF_OK != PTREE_NLINKER_ASYNC_Init(&hsegMod->oriIn.asyncLinker, 1, 100))
    {
        goto ERR_ORI_IN_LINKER_INIT;
    }
    if (SSOS_DEF_OK != PTREE_NLINKER_ASYNC_Init(&hsegMod->repIn.asyncLinker, 1, 100))
    {
        goto ERR_REP_IN_LINKER_INIT;
    }
    if (SSOS_DEF_OK != PTREE_NLINKER_ASYNC_Init(&hsegMod->dlaIn.asyncLinker, 1, 100))
    {
        goto ERR_DLA_IN_LINKER_INIT;
    }
    if (SSOS_DEF_OK != PTREE_NLINKER_OUT_Init(&hsegMod->modOut.outLinker, &G_PTREE_MOD_HSEG_OUT_LINKER_OPS))
    {
        goto ERR_OUT_LINKER_INIT;
    }

    if (SSOS_DEF_OK != PTREE_MMA_PACKET_PackerInit(&hsegMod->oriIn.packer))
    {
        goto ERR_ORI_IN_PACKER_INIT;
    }
    if (SSOS_DEF_OK != PTREE_MMA_PACKET_PackerInit(&hsegMod->repIn.packer))
    {
        goto ERR_REP_IN_PACKER_INIT;
    }
    if (SSOS_DEF_OK != PTREE_MMA_PACKET_PackerInit(&hsegMod->dlaIn.packer))
    {
        goto ERR_DLA_IN_PACKER_INIT;
    }

    PTREE_NLINKER_ASYNC_Register(&hsegMod->oriIn.asyncLinker, &G_PTREE_MOD_HSEG_IN_LINKER_HOOK);
    PTREE_NLINKER_ASYNC_Register(&hsegMod->repIn.asyncLinker, &G_PTREE_MOD_HSEG_IN_LINKER_HOOK);
    PTREE_NLINKER_ASYNC_Register(&hsegMod->dlaIn.asyncLinker, &G_PTREE_MOD_HSEG_IN_LINKER_HOOK);
    PTREE_NLINKER_OUT_Register(&hsegMod->modOut.outLinker, &G_PTREE_MOD_HSEG_OUT_LINKER_HOOK);

    PTREE_MOD_InObjRegister(&hsegMod->oriIn.base, &G_PTREE_MOD_HSEG_IN_HOOK);
    PTREE_MOD_InObjRegister(&hsegMod->repIn.base, &G_PTREE_MOD_HSEG_IN_HOOK);
    PTREE_MOD_InObjRegister(&hsegMod->dlaIn.base, &G_PTREE_MOD_HSEG_DLA_IN_HOOK);
    PTREE_MOD_OutObjRegister(&hsegMod->modOut.base, &G_PTREE_MOD_HSEG_OUT_HOOK);

    hsegMod->stCtrl.bgblur_mode    = PTREE_ENUM_FROM_STR(ALGO_HsegBgbMode_e, hsegInfo->chHsegMode);
    hsegMod->stCtrl.mask_thredhold = (MI_U8)hsegInfo->uiMaskThr;
    hsegMod->stCtrl.blur_level     = (MI_U8)hsegInfo->uiBlurLv;
    hsegMod->stCtrl.scaling_stage  = (MI_U8)hsegInfo->uiScalingStage;
    hsegMod->stCtrl.maskOp         = PTREE_ENUM_FROM_STR(ALGO_BgBlurMaskOp_e, hsegInfo->chMaskOp);
    SSOS_THREAD_MutexInit(&hsegMod->mutexCtrl);

    PTREE_MOD_SYS_ObjRegister(&hsegMod->base, &G_PTREE_MOD_HSEG_SYS_HOOK);
    return &hsegMod->base.base;

    PTREE_PACKER_Del(&hsegMod->dlaIn.packer.base);
ERR_DLA_IN_PACKER_INIT:
    PTREE_PACKER_Del(&hsegMod->repIn.packer.base);
ERR_REP_IN_PACKER_INIT:
    PTREE_PACKER_Del(&hsegMod->oriIn.packer.base);
ERR_ORI_IN_PACKER_INIT:
    PTREE_LINKER_Del(&hsegMod->modOut.outLinker.base);
ERR_OUT_LINKER_INIT:
    PTREE_LINKER_Del(&hsegMod->dlaIn.asyncLinker.base);
ERR_DLA_IN_LINKER_INIT:
    PTREE_LINKER_Del(&hsegMod->repIn.asyncLinker.base);
ERR_REP_IN_LINKER_INIT:
    PTREE_LINKER_Del(&hsegMod->oriIn.asyncLinker.base);
ERR_ORI_IN_LINKER_INIT:
    PTREE_MOD_OutObjDel(&hsegMod->modOut.base);
ERR_MOD_OUT_INIT:
    PTREE_MOD_InObjDel(&hsegMod->dlaIn.base);
ERR_MOD_DLA_IN_INIT:
    PTREE_MOD_InObjDel(&hsegMod->repIn.base);
ERR_MOD_REP_IN_INIT:
    PTREE_MOD_InObjDel(&hsegMod->oriIn.base);
ERR_MOD_ORI_IN_INIT:
ERR_IO_COUNT:
    PTREE_MOD_ObjDel(&hsegMod->base.base);
ERR_MOD_SYS_INIT:
    SSOS_MEM_Free(hsegMod);
ERR_MEM_ALLOC:
    return NULL;
}

void PTREE_MOD_HSEG_SetCtrlParam(PTREE_MOD_Obj_t *mod, const char *pchParam, PTREE_MOD_HSEG_ParamTypeE_t type)
{
    PTREE_MOD_HSEG_Obj_t *hsegMod = CONTAINER_OF(mod, PTREE_MOD_HSEG_Obj_t, base.base);
    SSOS_THREAD_MutexLock(&hsegMod->mutexCtrl);
    switch (type)
    {
        case E_PTREE_MOD_HSEG_PARAM_TYPE_MODE:
            hsegMod->stCtrl.bgblur_mode = PTREE_ENUM_FROM_STR(ALGO_HsegBgbMode_e, pchParam);
            break;
        case E_PTREE_MOD_HSEG_PARAM_TYPE_OP:
            hsegMod->stCtrl.maskOp = PTREE_ENUM_FROM_STR(ALGO_BgBlurMaskOp_e, pchParam);
            break;
        case E_PTREE_MOD_HSEG_PARAM_TYPE_THR:
            hsegMod->stCtrl.mask_thredhold = (MI_U8)SSOS_IO_Atoi(pchParam);
            break;
        case E_PTREE_MOD_HSEG_PARAM_TYPE_LV:
            hsegMod->stCtrl.blur_level = (MI_U8)SSOS_IO_Atoi(pchParam);
            break;
        case E_PTREE_MOD_HSEG_PARAM_TYPE_STAGE:
            hsegMod->stCtrl.scaling_stage = (MI_U8)SSOS_IO_Atoi(pchParam);
            break;
        default:
            PTREE_ERR("reset ctrl param failed! type is : %d", type);
            break;
    }
    SSOS_THREAD_MutexUnlock(&hsegMod->mutexCtrl);
    PTREE_DBG("reset ctrl param type : %d, param: %s", type, pchParam);
}

PTREE_MAKER_MOD_INIT(HSEG, PTREE_MOD_HSEG_New);
