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
#include "ptree_log.h"
#include "ssos_mem.h"
#include "ptree_mod.h"
#include "ptree_maker.h"
#include "ptree_packer.h"
#include "ptree_packet.h"
#include "ptree_packet_raw.h"
#include "ptree_packet_video.h"
#include "ptree_packet_audio.h"

static int                 _PTREE_MOD_EMPTY_InLinkerEnqueue(PTREE_LINKER_Obj_t *linker, PTREE_PACKET_Obj_t *packet);
static PTREE_PACKET_Obj_t *_PTREE_MOD_EMPTY_InLinkerDequeue(PTREE_LINKER_Obj_t *linker, int ms);
static void                _PTREE_MOD_EMPTY_InLinkerFree(PTREE_LINKER_Obj_t *linker);

static int                 _PTREE_MOD_EMPTY_InGetType(PTREE_MOD_InObj_t *modIn);
static PTREE_LINKER_Obj_t *_PTREE_MOD_EMPTY_InCreateNLinker(PTREE_MOD_InObj_t *modIn);
static PTREE_PACKER_Obj_t *_PTREE_MOD_EMPTY_InCreatePacker(PTREE_MOD_InObj_t *modIn, int *isFast);
static void                _PTREE_MOD_EMPTY_InFree(PTREE_MOD_InObj_t *modIn);

static int  _PTREE_MOD_EMPTY_OutGetType(PTREE_MOD_OutObj_t *modOut);
static void _PTREE_MOD_EMPTY_OutFree(PTREE_MOD_OutObj_t *modOut);

static PTREE_MOD_InObj_t * _PTREE_MOD_EMPTY_CreateModIn(PTREE_MOD_Obj_t *mod, unsigned int loopId);
static PTREE_MOD_OutObj_t *_PTREE_MOD_EMPTY_CreateModOut(PTREE_MOD_Obj_t *mod, unsigned int loopId);
static void                _PTREE_MOD_EMPTY_Free(PTREE_MOD_Obj_t *mod);

static const PTREE_LINKER_Ops_t G_PTREE_MOD_EMPTY_LINKER_OPS = {
    .enqueue = _PTREE_MOD_EMPTY_InLinkerEnqueue,
    .dequeue = _PTREE_MOD_EMPTY_InLinkerDequeue,
};
static const PTREE_LINKER_Hook_t G_PTREE_MOD_EMPTY_LINKER_HOOK = {
    .free = _PTREE_MOD_EMPTY_InLinkerFree,
};

static const PTREE_MOD_Ops_t G_PTREE_MOD_EMPTY_OPS = {
    .createModIn  = _PTREE_MOD_EMPTY_CreateModIn,
    .createModOut = _PTREE_MOD_EMPTY_CreateModOut,
};
static const PTREE_MOD_Hook_t G_PTREE_MOD_EMPTY_HOOK = {
    .free = _PTREE_MOD_EMPTY_Free,
};

static const PTREE_MOD_InOps_t G_PTREE_MOD_EMPTY_IN_OPS = {
    .getType       = _PTREE_MOD_EMPTY_InGetType,
    .createNLinker = _PTREE_MOD_EMPTY_InCreateNLinker,
    .createPacker  = _PTREE_MOD_EMPTY_InCreatePacker,
};
static const PTREE_MOD_InHook_t G_PTREE_MOD_EMPTY_IN_HOOK = {
    .free = _PTREE_MOD_EMPTY_InFree,
};

static const PTREE_MOD_OutOps_t G_PTREE_MOD_EMPTY_OUT_OPS = {
    .getType = _PTREE_MOD_EMPTY_OutGetType,
};
static const PTREE_MOD_OutHook_t G_PTREE_MOD_EMPTY_OUT_HOOK = {
    .free = _PTREE_MOD_EMPTY_OutFree,
};

static int _PTREE_MOD_EMPTY_InLinkerEnqueue(PTREE_LINKER_Obj_t *linker, PTREE_PACKET_Obj_t *packet)
{
    (void)linker;
    (void)packet;
    return SSOS_DEF_OK;
}
static PTREE_PACKET_Obj_t *_PTREE_MOD_EMPTY_InLinkerDequeue(PTREE_LINKER_Obj_t *linker, int ms)
{
    (void)linker;
    (void)ms;
    return NULL;
}
static void _PTREE_MOD_EMPTY_InLinkerFree(PTREE_LINKER_Obj_t *linker)
{
    SSOS_MEM_Free(linker);
}
static int _PTREE_MOD_EMPTY_InGetType(PTREE_MOD_InObj_t *modIn)
{
    (void)modIn;
    return E_PTREE_MOD_BIND_TYPE_USER;
}
static PTREE_LINKER_Obj_t *_PTREE_MOD_EMPTY_InCreateNLinker(PTREE_MOD_InObj_t *modIn)
{
    PTREE_LINKER_Obj_t *linker = SSOS_MEM_Alloc(sizeof(PTREE_LINKER_Obj_t));

    (void)modIn;
    if (!linker)
    {
        PTREE_ERR("Alloc err");
        return NULL;
    }
    memset(linker, 0, sizeof(PTREE_LINKER_Obj_t));

    if (SSOS_DEF_OK != PTREE_LINKER_Init(linker, &G_PTREE_MOD_EMPTY_LINKER_OPS))
    {
        SSOS_MEM_Free(linker);
        return NULL;
    }

    PTREE_LINKER_Register(linker, &G_PTREE_MOD_EMPTY_LINKER_HOOK);
    return linker;
}
static PTREE_PACKER_Obj_t *_PTREE_MOD_EMPTY_InCreatePacker(PTREE_MOD_InObj_t *modIn, int *isFast)
{
    (void)modIn;
    *isFast = SSOS_DEF_FALSE;
    return PTREE_PACKER_NormalNew();
}
static void _PTREE_MOD_EMPTY_InFree(PTREE_MOD_InObj_t *modIn)
{
    SSOS_MEM_Free(modIn);
}

static int _PTREE_MOD_EMPTY_OutGetType(PTREE_MOD_OutObj_t *modOut)
{
    (void)modOut;
    return E_PTREE_MOD_BIND_TYPE_USER;
}
static void _PTREE_MOD_EMPTY_OutFree(PTREE_MOD_OutObj_t *modOut)
{
    SSOS_MEM_Free(modOut);
}

static PTREE_MOD_InObj_t *_PTREE_MOD_EMPTY_CreateModIn(PTREE_MOD_Obj_t *mod, unsigned int loopId)
{
    PTREE_MOD_InObj_t *modIn = SSOS_MEM_Alloc(sizeof(PTREE_MOD_InObj_t));
    if (!modIn)
    {
        PTREE_ERR("Alloc err");
        return NULL;
    }
    memset(modIn, 0, sizeof(PTREE_MOD_InObj_t));
    if (SSOS_DEF_OK != PTREE_MOD_InObjInit(modIn, &G_PTREE_MOD_EMPTY_IN_OPS, mod, loopId))
    {
        SSOS_MEM_Free(modIn);
        return NULL;
    }
    PTREE_MOD_InObjRegister(modIn, &G_PTREE_MOD_EMPTY_IN_HOOK);
    return modIn;
}
static PTREE_MOD_OutObj_t *_PTREE_MOD_EMPTY_CreateModOut(PTREE_MOD_Obj_t *mod, unsigned int loopId)
{
    PTREE_MOD_OutObj_t *modOut = SSOS_MEM_Alloc(sizeof(PTREE_MOD_OutObj_t));
    if (!modOut)
    {
        PTREE_ERR("Alloc err");
        return NULL;
    }
    memset(modOut, 0, sizeof(PTREE_MOD_OutObj_t));
    if (SSOS_DEF_OK != PTREE_MOD_OutObjInit(modOut, &G_PTREE_MOD_EMPTY_OUT_OPS, mod, loopId))
    {
        SSOS_MEM_Free(modOut);
        return NULL;
    }
    PTREE_MOD_OutObjRegister(modOut, &G_PTREE_MOD_EMPTY_OUT_HOOK);
    return modOut;
}
static void _PTREE_MOD_EMPTY_Free(PTREE_MOD_Obj_t *mod)
{
    SSOS_MEM_Free(mod);
}

PTREE_MOD_Obj_t *PTREE_MOD_EMPTY_New(PARENA_Tag_t *tag)
{
    PTREE_MOD_Obj_t *mod = NULL;

    mod = SSOS_MEM_Alloc(sizeof(PTREE_MOD_Obj_t));
    if (!mod)
    {
        PTREE_ERR("Alloc err");
        return NULL;
    }
    memset(mod, 0, sizeof(PTREE_MOD_Obj_t));
    if (SSOS_DEF_OK != PTREE_MOD_ObjInit(mod, &G_PTREE_MOD_EMPTY_OPS, tag))
    {
        SSOS_MEM_Free(mod);
        return NULL;
    }
    PTREE_MOD_ObjRegister(mod, &G_PTREE_MOD_EMPTY_HOOK);
    return mod;
}

PTREE_MAKER_MOD_INIT(EMPTY, PTREE_MOD_EMPTY_New);
PTREE_MAKER_MOD_INIT(CMD_TEST, PTREE_MOD_EMPTY_New);
PTREE_MAKER_MOD_INIT(STR_CMD_TEST, PTREE_MOD_EMPTY_New);
