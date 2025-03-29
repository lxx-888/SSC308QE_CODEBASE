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

#include "arena.h"
#include "mi_venc_datatype.h"
#include "parena.h"
#include "ssos_def.h"
#include "ssos_hash.h"
#include "ssos_list.h"
#include "ptree_log.h"
#include "ssos_mem.h"
#include "ptree_mod.h"
#include "ptree_maker.h"
#include "ptree_packer.h"
#include "ptree_packet.h"
#include "ptree_sur.h"
#include "ptree_sur_pass.h"

#define CHECK_FIRST_HASH_SIZE 32

#define PRINT_PASS_RELINK(__forwardIn, __currentOut, __replacedOut)                                                   \
    do                                                                                                                \
    {                                                                                                                 \
        PARENA_Tag_t *    __tag  = NULL;                                                                              \
        PTREE_SUR_Info_t *__info = NULL;                                                                              \
        __tag                    = (PARENA_Tag_t *)PARENA_RELATIVE_ADDRESS(__forwardIn, (__forwardIn)->curTagOffset); \
        __info                   = (PTREE_SUR_Info_t *)PARENA_USE_BASE(__tag);                                        \
        PTREE_DBG(" [Forward Section] : %s", __info->sectionName);                                                    \
        PTREE_DBG(" [PORT]            : %d", (__forwardIn)->port);                                                    \
        __tag  = (PARENA_Tag_t *)PARENA_RELATIVE_ADDRESS(__currentOut, (__currentOut)->curTagOffset);                 \
        __info = (PTREE_SUR_Info_t *)PARENA_USE_BASE(__tag);                                                          \
        PTREE_DBG(" [Curr Section]    : %s", __info->sectionName);                                                    \
        PTREE_DBG(" [PORT]            : %d", (__currentOut)->port);                                                   \
        __tag  = (PARENA_Tag_t *)PARENA_RELATIVE_ADDRESS(__replacedOut, (__replacedOut)->curTagOffset);               \
        __info = (PTREE_SUR_Info_t *)PARENA_USE_BASE(__tag);                                                          \
        PTREE_DBG(" [Replace To]      : %s", __info->sectionName);                                                    \
        PTREE_DBG(" [PORT]            : %d", (__replacedOut)->port);                                                  \
    } while (0)

enum PTREE_MOD_PASS_ResourceState_e
{
    E_PTREE_MOD_PASS_RESOURCE_NONE   = 0,
    E_PTREE_MOD_PASS_RESOURCE_CREATE = 0x1,
    E_PTREE_MOD_PASS_RESOURCE_INIT   = 0x2,
    E_PTREE_MOD_PASS_RESOURCE_BIND   = 0x4,
    E_PTREE_MOD_PASS_RESOURCE_START  = 0x8,
};

typedef struct PTREE_MOD_PASS_Obj_s    PTREE_MOD_PASS_Obj_t;
typedef struct PTREE_MOD_PASS_InObj_s  PTREE_MOD_PASS_InObj_t;
typedef struct PTREE_MOD_PASS_OutObj_s PTREE_MOD_PASS_OutObj_t;

typedef struct PTREE_MOD_PASS_AccessNode_s           PTREE_MOD_PASS_AccessNode_t;
typedef struct PTREE_MOD_PASS_MembersObjNode_s       PTREE_MOD_PASS_MembersObjNode_t;
typedef struct PTREE_MOD_PASS_MemberObjOrderedNode_s PTREE_MOD_PASS_MemberObjOrderedNode_t;

struct PTREE_MOD_PASS_MembersObjNode_s
{
    struct SSOS_LIST_Head_s memberHash;
    PTREE_MOD_Obj_t *       pstModObj;
    PTREE_CMD_Obj_t *       pstCmdObj;
    unsigned char           bObjInEdge;
    unsigned int            objUseCnt;
    unsigned int            objBindCnt;
    unsigned int            objInitCnt;
    unsigned int            objStartCnt;
};

struct PTREE_MOD_PASS_AccessNode_s
{
    struct SSOS_LIST_Head_s hash;
    unsigned int            value;
};

struct PTREE_MOD_PASS_MemberObjOrderedNode_s
{
    struct SSOS_LIST_Head_s                 orderList;
    struct PTREE_MOD_PASS_MembersObjNode_s *memberObj;
};

struct PTREE_MOD_PASS_Obj_s
{
    PTREE_MOD_Obj_t         base;
    struct SSOS_LIST_Head_s memberHash[PTREE_PASS_MOD_HASH_SIZE];
};

struct PTREE_MOD_PASS_InObj_s
{
    PTREE_MOD_InObj_t base;
};

struct PTREE_MOD_PASS_OutObj_s
{
    PTREE_MOD_OutObj_t               base;
    PTREE_MOD_PASS_MembersObjNode_t *rootMembers[PARENA_ROOT_COUNT_MAX];
    unsigned int                     arrResSt[PARENA_ROOT_COUNT_MAX];
};

static int  _PTREE_MOD_PASS_InGetType(PTREE_MOD_InObj_t *modIn);
static int  _PTREE_MOD_PASS_IsDelayLink(PTREE_MOD_InObj_t *modIn);
static void _PTREE_MOD_PASS_InFree(PTREE_MOD_InObj_t *modIn);
static int  _PTREE_MOD_PASS_LinkedTransfer(PTREE_MOD_OutObj_t *modOut);
static int  _PTREE_MOD_PASS_UnlinkedTransfer(PTREE_MOD_OutObj_t *modOut);
static int  _PTREE_MOD_PASS_SuspendTransfer(PTREE_MOD_OutObj_t *modOut);
static int  _PTREE_MOD_PASS_ResumeTransfer(PTREE_MOD_OutObj_t *modOut);

static int  _PTREE_MOD_PASS_OutGetType(PTREE_MOD_OutObj_t *modOut);
static void _PTREE_MOD_PASS_OutFree(PTREE_MOD_OutObj_t *modOut);

static int                 _PTREE_MOD_PASS_Deinit(PTREE_MOD_Obj_t *mod);
static PTREE_MOD_InObj_t * _PTREE_MOD_PASS_CreateModIn(PTREE_MOD_Obj_t *mod, unsigned int loopId);
static PTREE_MOD_OutObj_t *_PTREE_MOD_PASS_CreateModOut(PTREE_MOD_Obj_t *mod, unsigned int loopId);
static void                _PTREE_MOD_PASS_Free(PTREE_MOD_Obj_t *mod);

static unsigned int _PTREE_MOD_PASS_GetHash(unsigned int value, unsigned int size);
static void         _PTREE_MOD_PASS_FreeHash(struct SSOS_LIST_Head_s *hash, unsigned int hashSize,
                                             void (*freeNode)(struct SSOS_LIST_Head_s *));

static int  _PTREE_MOD_PASS_AccessValue(struct SSOS_LIST_Head_s hash[CHECK_FIRST_HASH_SIZE], unsigned int value);
static int  _PTREE_MOD_PASS_LeaveValue(struct SSOS_LIST_Head_s hash[CHECK_FIRST_HASH_SIZE], unsigned int value);
static int  _PTREE_MOD_PASS_IsAccessFirst(struct SSOS_LIST_Head_s hash[CHECK_FIRST_HASH_SIZE], unsigned int value);
static void _PTREE_MOD_PASS_FreeAccessNode(struct SSOS_LIST_Head_s *pos);

static PTREE_MOD_PASS_MembersObjNode_t *_PTREE_MOD_PASS_FindMemberInHash(struct SSOS_LIST_Head_s *hashList,
                                                                         const char *modName, unsigned int dev,
                                                                         unsigned int chn);

static int _PTREE_MOD_PASS_SignatureMembers(PARENA_Tag_t *          tag,
                                            struct SSOS_LIST_Head_s memberHash[PTREE_PASS_MOD_HASH_SIZE],
                                            struct SSOS_LIST_Head_s checkFirstHash[CHECK_FIRST_HASH_SIZE]);

static PTREE_MOD_PASS_MembersObjNode_t *
_PTREE_MOD_PASS_ImplementBin(PTREE_MOD_PASS_Obj_t *passMod, PARENA_Tag_t *tag,
                             struct SSOS_LIST_Head_s checkFirstHash[CHECK_FIRST_HASH_SIZE]);

static int _PTREE_MOD_PASS_GetMemberObjAll(PTREE_MOD_PASS_OutObj_t *passOutObj, struct SSOS_LIST_Head_s *orderList);
static int _PTREE_MOD_PASS_GetMemberObjIdx(PTREE_MOD_PASS_OutObj_t *passOutObj, struct SSOS_LIST_Head_s *orderList,
                                           unsigned int rootIdx);

static int  _PTREE_MOD_PASS_CheckState(PTREE_MOD_PASS_OutObj_t *           passOutObj,
                                       enum PTREE_MOD_PASS_ResourceState_e targetSt);
static void _PTREE_MOD_PASS_UnmarkState(PTREE_MOD_PASS_OutObj_t *           passOutObj,
                                        enum PTREE_MOD_PASS_ResourceState_e targetSt);
static void _PTREE_MOD_PASS_UnmarkStateIdx(PTREE_MOD_PASS_OutObj_t *passOutObj, unsigned int rootIdx,
                                           enum PTREE_MOD_PASS_ResourceState_e targetSt);

static int _PTREE_MOD_PASS_DoCreateDelayPass(PTREE_MOD_PASS_OutObj_t *passOutObj);
static int _PTREE_MOD_PASS_DoDestroyDelayPass(PTREE_MOD_PASS_OutObj_t *passOutObj, struct SSOS_LIST_Head_s *orderList);
static int _PTREE_MOD_PASS_DoInitDelayPass(struct SSOS_LIST_Head_s *orderList);
static int _PTREE_MOD_PASS_DoDeinitDelayPass(struct SSOS_LIST_Head_s *orderList);
static int _PTREE_MOD_PASS_DoBindDelayPass(struct SSOS_LIST_Head_s *orderList);
static int _PTREE_MOD_PASS_DoUnbindDelayPass(struct SSOS_LIST_Head_s *orderList);
static int _PTREE_MOD_PASS_DoStartDelayPass(struct SSOS_LIST_Head_s *orderList,
                                            struct SSOS_LIST_Head_s  memberHash[PTREE_PASS_MOD_HASH_SIZE]);
static int _PTREE_MOD_PASS_DoStopDelayPass(struct SSOS_LIST_Head_s *orderList,
                                           struct SSOS_LIST_Head_s  memberHash[PTREE_PASS_MOD_HASH_SIZE]);

static int _PTREE_MOD_PASS_CreateDelayPass(PTREE_MOD_OutObj_t *modOut, PTREE_MOD_InObj_t *postModIn);
static int _PTREE_MOD_PASS_DestroyDelayPass(PTREE_MOD_OutObj_t *modOut, PTREE_MOD_InObj_t *postModIn);
static int _PTREE_MOD_PASS_InitDelayPass(PTREE_MOD_OutObj_t *modOut);
static int _PTREE_MOD_PASS_DeinitDelayPass(PTREE_MOD_OutObj_t *modOut);
static int _PTREE_MOD_PASS_BindDelayPass(PTREE_MOD_OutObj_t *modOut);
static int _PTREE_MOD_PASS_UnbindDelayPass(PTREE_MOD_OutObj_t *modOut);
static int _PTREE_MOD_PASS_StartDelayPass(PTREE_MOD_OutObj_t *modOut);
static int _PTREE_MOD_PASS_StopDelayPass(PTREE_MOD_OutObj_t *modOut);

static const PTREE_MOD_Ops_t G_PTREE_MOD_PASS_OPS = {
    .deinit       = _PTREE_MOD_PASS_Deinit,
    .createModIn  = _PTREE_MOD_PASS_CreateModIn,
    .createModOut = _PTREE_MOD_PASS_CreateModOut,
};

static const PTREE_MOD_Hook_t G_PTREE_MOD_PASS_HOOK = {
    .free = _PTREE_MOD_PASS_Free,
};

static const PTREE_MOD_InOps_t G_PTREE_MOD_PASS_IN_OPS = {
    .getType     = _PTREE_MOD_PASS_InGetType,
    .isDelayLink = _PTREE_MOD_PASS_IsDelayLink,
};

static const PTREE_MOD_InHook_t G_PTREE_MOD_PASS_IN_HOOK = {
    .free = _PTREE_MOD_PASS_InFree,
};

static const PTREE_MOD_OutOps_t G_PTREE_MOD_PASS_OUT_OPS = {
    .getType          = _PTREE_MOD_PASS_OutGetType,
    .linkedTransfer   = _PTREE_MOD_PASS_LinkedTransfer,
    .unlinkedTransfer = _PTREE_MOD_PASS_UnlinkedTransfer,
    .suspendTransfer  = _PTREE_MOD_PASS_SuspendTransfer,
    .resumeTransfer   = _PTREE_MOD_PASS_ResumeTransfer,
    .createDelayPass  = _PTREE_MOD_PASS_CreateDelayPass,
    .destroyDelayPass = _PTREE_MOD_PASS_DestroyDelayPass,
    .initDelayPass    = _PTREE_MOD_PASS_InitDelayPass,
    .deinitDelayPass  = _PTREE_MOD_PASS_DeinitDelayPass,
    .bindDelayPass    = _PTREE_MOD_PASS_BindDelayPass,
    .unbindDelayPass  = _PTREE_MOD_PASS_UnbindDelayPass,
    .startDelayPass   = _PTREE_MOD_PASS_StartDelayPass,
    .stopDelayPass    = _PTREE_MOD_PASS_StopDelayPass,
};

static const PTREE_MOD_OutHook_t G_PTREE_MOD_PASS_OUT_HOOK = {
    .free = _PTREE_MOD_PASS_OutFree,
};

static int _PTREE_MOD_PASS_InGetType(PTREE_MOD_InObj_t *modIn)
{
    (void)modIn;
    return E_PTREE_MOD_BIND_TYPE_NONE;
}

static int _PTREE_MOD_PASS_IsDelayLink(PTREE_MOD_InObj_t *modIn)
{
    /*
     * We redefine this virtual function to avoid some case that PASS has no output info after 'unlink' is
     * called in '_CreateDelayPass'.
     */
    (void)modIn;
    return 1;
}

static int _PTREE_MOD_PASS_LinkedTransfer(PTREE_MOD_OutObj_t *modOut)
{
    int                                    ret        = 0;
    PTREE_MOD_PASS_MemberObjOrderedNode_t *pos        = NULL;
    PTREE_MOD_PASS_MemberObjOrderedNode_t *posN       = NULL;
    PTREE_MOD_PASS_OutObj_t *              passOutObj = CONTAINER_OF(modOut, PTREE_MOD_PASS_OutObj_t, base);
    SSOS_LIST_HEAD(orderList);

    if (passOutObj->arrResSt[0] == E_PTREE_MOD_PASS_RESOURCE_NONE)
    {
        /* Internal pass haven't beed created, no need to transfer message to previous module. */
        return 0;
    }
    ret = _PTREE_MOD_PASS_GetMemberObjAll(passOutObj, &orderList);
    if (ret == SSOS_DEF_FAIL)
    {
        PTREE_ERR("Output: %d object error!", passOutObj->base.info->port);
        return ret;
    }
    SSOS_LIST_FOR_EACH_ENTRY_SAFE(pos, posN, &orderList, orderList)
    {
        unsigned char i = 0;
        if (pos->memberObj->bObjInEdge)
        {
            for (i = 0; i < pos->memberObj->pstModObj->info->inCnt; i++)
            {
                PTREE_MOD_InObj_t *modIn = pos->memberObj->pstModObj->arrModIn[i];
                PTREE_MESSAGE_Access(&modIn->message);
            }
            SSOS_LIST_Del(&pos->orderList);
            SSOS_MEM_Free(pos);
        }
    }
    return SSOS_DEF_OK;
}

static int _PTREE_MOD_PASS_UnlinkedTransfer(PTREE_MOD_OutObj_t *modOut)
{
    int                                    ret        = 0;
    PTREE_MOD_PASS_MemberObjOrderedNode_t *pos        = NULL;
    PTREE_MOD_PASS_MemberObjOrderedNode_t *posN       = NULL;
    PTREE_MOD_PASS_OutObj_t *              passOutObj = CONTAINER_OF(modOut, PTREE_MOD_PASS_OutObj_t, base);
    SSOS_LIST_HEAD(orderList);

    if (passOutObj->arrResSt[0] == E_PTREE_MOD_PASS_RESOURCE_NONE)
    {
        /* Internal pass haven't beed created, no need to transfer message to previous module. */
        return 0;
    }
    ret = _PTREE_MOD_PASS_GetMemberObjAll(passOutObj, &orderList);
    if (ret == SSOS_DEF_FAIL)
    {
        PTREE_ERR("Output: %d object error!", passOutObj->base.info->port);
        return ret;
    }
    SSOS_LIST_FOR_EACH_ENTRY_SAFE(pos, posN, &orderList, orderList)
    {
        unsigned char i = 0;
        if (pos->memberObj->bObjInEdge)
        {
            for (i = 0; i < pos->memberObj->pstModObj->info->inCnt; i++)
            {
                PTREE_MOD_InObj_t *modIn = pos->memberObj->pstModObj->arrModIn[i];
                PTREE_MESSAGE_Leave(&modIn->message);
            }
            SSOS_LIST_Del(&pos->orderList);
            SSOS_MEM_Free(pos);
        }
    }
    return SSOS_DEF_OK;
}

static int _PTREE_MOD_PASS_SuspendTransfer(PTREE_MOD_OutObj_t *modOut)
{
    int                                    ret        = 0;
    PTREE_MOD_PASS_MemberObjOrderedNode_t *pos        = NULL;
    PTREE_MOD_PASS_MemberObjOrderedNode_t *posN       = NULL;
    PTREE_MOD_PASS_OutObj_t *              passOutObj = CONTAINER_OF(modOut, PTREE_MOD_PASS_OutObj_t, base);
    SSOS_LIST_HEAD(orderList);

    if (passOutObj->arrResSt[0] == E_PTREE_MOD_PASS_RESOURCE_NONE)
    {
        /* Internal pass haven't beed created, no need to transfer message to previous module. */
        return 0;
    }
    ret = _PTREE_MOD_PASS_GetMemberObjAll(passOutObj, &orderList);
    if (ret == SSOS_DEF_FAIL)
    {
        PTREE_ERR("Output: %d object error!", passOutObj->base.info->port);
        return ret;
    }
    SSOS_LIST_FOR_EACH_ENTRY_SAFE(pos, posN, &orderList, orderList)
    {
        unsigned char i = 0;
        if (pos->memberObj->bObjInEdge)
        {
            for (i = 0; i < pos->memberObj->pstModObj->info->inCnt; i++)
            {
                PTREE_MOD_InObj_t *modIn = pos->memberObj->pstModObj->arrModIn[i];
                PTREE_MESSAGE_Suspend(&modIn->message);
            }
            SSOS_LIST_Del(&pos->orderList);
            SSOS_MEM_Free(pos);
        }
    }
    return SSOS_DEF_OK;
}

static int _PTREE_MOD_PASS_ResumeTransfer(PTREE_MOD_OutObj_t *modOut)
{
    int                                    ret        = 0;
    PTREE_MOD_PASS_MemberObjOrderedNode_t *pos        = NULL;
    PTREE_MOD_PASS_MemberObjOrderedNode_t *posN       = NULL;
    PTREE_MOD_PASS_OutObj_t *              passOutObj = CONTAINER_OF(modOut, PTREE_MOD_PASS_OutObj_t, base);
    SSOS_LIST_HEAD(orderList);

    if (passOutObj->arrResSt[0] == E_PTREE_MOD_PASS_RESOURCE_NONE)
    {
        /* Internal pass haven't beed created, no need to transfer message to previous module. */
        return 0;
    }
    ret = _PTREE_MOD_PASS_GetMemberObjAll(passOutObj, &orderList);
    if (ret == SSOS_DEF_FAIL)
    {
        PTREE_ERR("Output: %d object error!", passOutObj->base.info->port);
        return ret;
    }
    SSOS_LIST_FOR_EACH_ENTRY_SAFE(pos, posN, &orderList, orderList)
    {
        unsigned char i = 0;
        if (pos->memberObj->bObjInEdge)
        {
            for (i = 0; i < pos->memberObj->pstModObj->info->inCnt; i++)
            {
                PTREE_MOD_InObj_t *modIn = pos->memberObj->pstModObj->arrModIn[i];
                PTREE_MESSAGE_Resume(&modIn->message);
            }
            SSOS_LIST_Del(&pos->orderList);
            SSOS_MEM_Free(pos);
        }
    }
    return SSOS_DEF_OK;
}

static void _PTREE_MOD_PASS_InFree(PTREE_MOD_InObj_t *modIn)
{
    PTREE_MOD_PASS_InObj_t *passInObj = CONTAINER_OF(modIn, PTREE_MOD_PASS_InObj_t, base);
    SSOS_MEM_Free(passInObj);
}

static int _PTREE_MOD_PASS_Deinit(PTREE_MOD_Obj_t *mod)
{
    unsigned char i = 0;

    for (i = 0; i < mod->info->outCnt; i++)
    {
        PTREE_MOD_PASS_OutObj_t *passOut = CONTAINER_OF(mod->arrModOut[i], PTREE_MOD_PASS_OutObj_t, base);
        if (passOut->rootMembers[i])
        {
            PTREE_ERR("Resource PASS output %d doesn't clear!", passOut->base.info->port);
            return SSOS_DEF_FAIL;
        }
    }
    return SSOS_DEF_OK;
}

static PTREE_MOD_InObj_t *_PTREE_MOD_PASS_CreateModIn(PTREE_MOD_Obj_t *mod, unsigned int loopId)
{
    PTREE_MOD_PASS_InObj_t *passIn = (PTREE_MOD_PASS_InObj_t *)SSOS_MEM_Alloc(sizeof(PTREE_MOD_PASS_InObj_t));
    if (!passIn)
    {
        PTREE_ERR("Alloc err");
        return NULL;
    }
    memset(passIn, 0, sizeof(PTREE_MOD_PASS_InObj_t));
    if (SSOS_DEF_OK != PTREE_MOD_InObjInit(&passIn->base, &G_PTREE_MOD_PASS_IN_OPS, mod, loopId))
    {
        SSOS_MEM_Free(passIn);
        return NULL;
    }
    PTREE_MOD_InObjRegister(&passIn->base, &G_PTREE_MOD_PASS_IN_HOOK);
    return &passIn->base;
}

static PTREE_MOD_OutObj_t *_PTREE_MOD_PASS_CreateModOut(PTREE_MOD_Obj_t *mod, unsigned int loopId)
{
    PTREE_MOD_PASS_OutObj_t *passOut = (PTREE_MOD_PASS_OutObj_t *)SSOS_MEM_Alloc(sizeof(PTREE_MOD_PASS_OutObj_t));
    if (!passOut)
    {
        PTREE_ERR("Alloc err");
        return NULL;
    }
    memset(passOut, 0, sizeof(PTREE_MOD_PASS_OutObj_t));
    if (SSOS_DEF_OK != PTREE_MOD_OutObjInit(&passOut->base, &G_PTREE_MOD_PASS_OUT_OPS, mod, loopId))
    {
        SSOS_MEM_Free(passOut);
        return NULL;
    }
    PTREE_MOD_OutObjRegister(&passOut->base, &G_PTREE_MOD_PASS_OUT_HOOK);
    return &passOut->base;
}

static void _PTREE_MOD_PASS_Free(PTREE_MOD_Obj_t *mod)
{
    PTREE_MOD_PASS_Obj_t *passObj = CONTAINER_OF(mod, PTREE_MOD_PASS_Obj_t, base);
    SSOS_MEM_Free(passObj);
}

static int _PTREE_MOD_PASS_OutGetType(PTREE_MOD_OutObj_t *modOut)
{
    (void)modOut;
    return E_PTREE_MOD_BIND_TYPE_NONE;
}

static void _PTREE_MOD_PASS_OutFree(PTREE_MOD_OutObj_t *modOut)
{
    PTREE_MOD_PASS_OutObj_t *passOutObj = CONTAINER_OF(modOut, PTREE_MOD_PASS_OutObj_t, base);
    SSOS_MEM_Free(passOutObj);
}

static unsigned int _PTREE_MOD_PASS_GetHash(unsigned int value, unsigned int size)
{
    return value % size;
}

static void _PTREE_MOD_PASS_FreeHash(struct SSOS_LIST_Head_s *hash, unsigned int hashSize,
                                     void (*freeNode)(struct SSOS_LIST_Head_s *))
{
    unsigned int             i;
    struct SSOS_LIST_Head_s *pos, *posN;
    if (!freeNode)
    {
        PTREE_ERR("Null node");
        return;
    }
    for (i = 0; i < hashSize; i++)
    {
        SSOS_LIST_FOR_EACH_SAFE(pos, posN, &hash[i])
        {
            SSOS_LIST_Del(pos);
            freeNode(pos);
        }
    }
}

static int _PTREE_MOD_PASS_AccessValue(struct SSOS_LIST_Head_s hash[CHECK_FIRST_HASH_SIZE], unsigned int value)
{
    PTREE_MOD_PASS_AccessNode_t *node = NULL;
    unsigned int                 key  = _PTREE_MOD_PASS_GetHash(value, CHECK_FIRST_HASH_SIZE);
    node = (PTREE_MOD_PASS_AccessNode_t *)SSOS_MEM_Alloc(sizeof(PTREE_MOD_PASS_AccessNode_t));
    if (!node)
    {
        PTREE_ERR("Alloc fail!\n");
        return SSOS_DEF_FAIL;
    }
    node->value = value;
    SSOS_LIST_AddTail(&node->hash, &hash[key]);
    return SSOS_DEF_OK;
}

static int _PTREE_MOD_PASS_LeaveValue(struct SSOS_LIST_Head_s hash[CHECK_FIRST_HASH_SIZE], unsigned int value)
{
    PTREE_MOD_PASS_AccessNode_t *pos  = NULL;
    PTREE_MOD_PASS_AccessNode_t *posN = NULL;
    unsigned int                 key  = _PTREE_MOD_PASS_GetHash(value, CHECK_FIRST_HASH_SIZE);
    SSOS_LIST_FOR_EACH_ENTRY_SAFE(pos, posN, &hash[key], hash)
    {
        if (pos->value == value)
        {
            SSOS_LIST_Del(&pos->hash);
            SSOS_MEM_Free(pos);
            return 0;
        }
    }
    PTREE_ERR("Did not find key %d", key);
    return SSOS_DEF_FAIL;
}

static int _PTREE_MOD_PASS_IsAccessFirst(struct SSOS_LIST_Head_s hash[CHECK_FIRST_HASH_SIZE], unsigned int value)
{
    PTREE_MOD_PASS_AccessNode_t *pos;
    unsigned int                 key = _PTREE_MOD_PASS_GetHash(value, CHECK_FIRST_HASH_SIZE);
    SSOS_LIST_FOR_EACH_ENTRY(pos, &hash[key], hash)
    {
        if (pos->value == value)
        {
            return 0;
        }
    }
    return 1;
}

static void _PTREE_MOD_PASS_FreeAccessNode(struct SSOS_LIST_Head_s *pos)
{
    PTREE_MOD_PASS_AccessNode_t *node = CONTAINER_OF(pos, PTREE_MOD_PASS_AccessNode_t, hash);
    SSOS_MEM_Free(node);
}

static inline int _PTREE_MOD_PASS_MemberHash(const char *modName, unsigned int u32DevId, unsigned u32ChnId)
{
    char str[64];

    snprintf(str, 64, "%s_%u_%u", modName, u32DevId, u32ChnId);
    return SSOS_HASH_Val(str, PTREE_PASS_MOD_HASH_SIZE);
}

static PTREE_MOD_PASS_MembersObjNode_t *
_PTREE_MOD_PASS_FindMemberInHash(struct SSOS_LIST_Head_s *hashList, const char *modName, unsigned int dev, unsigned chn)
{
    PTREE_MOD_PASS_MembersObjNode_t *pos;
    SSOS_LIST_FOR_EACH_ENTRY(pos, hashList, memberHash)
    {
        PTREE_SUR_Info_t *info = pos->pstModObj->info;
        if (!strcmp(info->typeName, modName) && info->devId == dev && info->chnId == chn)
        {
            return pos;
        }
    }
    return NULL;
}

static int _PTREE_MOD_PASS_SignatureMembers(PARENA_Tag_t *          tag,
                                            struct SSOS_LIST_Head_s memberHash[PTREE_PASS_MOD_HASH_SIZE],
                                            struct SSOS_LIST_Head_s checkFirstHash[CHECK_FIRST_HASH_SIZE])
{
    unsigned int                     i       = 0;
    unsigned int                     hashVal = 0;
    PTREE_MOD_Obj_t *                mod     = NULL;
    PTREE_SUR_Info_t *               info    = NULL;
    PTREE_SUR_InInfo_t *             inInfo  = NULL;
    PTREE_MOD_PASS_MembersObjNode_t *node    = NULL;
    PARENA_Tag_t *                   prevTag = NULL;

    info    = (PTREE_SUR_Info_t *)PARENA_USE_BASE(tag);
    hashVal = _PTREE_MOD_PASS_MemberHash(info->typeName, info->devId, info->chnId);
    node    = _PTREE_MOD_PASS_FindMemberInHash(&memberHash[hashVal], info->typeName, info->devId, info->chnId);
    if (!node)
    {
        PTREE_ERR("Can not find the memberObj, sec %s", info->sectionName);
        return SSOS_DEF_FAIL;
    }
    if (!_PTREE_MOD_PASS_IsAccessFirst(checkFirstHash, tag->u32Offset))
    {
        /* Do nothing if traverse the samve member. */
        return SSOS_DEF_OK;
    }
    if (_PTREE_MOD_PASS_AccessValue(checkFirstHash, tag->u32Offset) == SSOS_DEF_FAIL)
    {
        PTREE_ERR("Access value error!");
        return SSOS_DEF_FAIL;
    }
    PTREE_DBG("LOOP: %s", info->sectionName);
    node->objUseCnt++;
    mod = node->pstModObj;
    for (i = 0; i < info->inCnt; i++)
    {
        inInfo = mod->arrModIn[i]->info;
        if (!strlen(inInfo->prevSection))
        {
            /* the tail module in 'PASS' has no previous section name. */
            continue;
        }
        prevTag = (PARENA_Tag_t *)PARENA_RELATIVE_ADDRESS(inInfo, inInfo->prevTagOffset);
        _PTREE_MOD_PASS_SignatureMembers(prevTag, memberHash, checkFirstHash);
    }
    return SSOS_DEF_OK;
}

static PTREE_MOD_PASS_MembersObjNode_t *
_PTREE_MOD_PASS_ImplementBin(PTREE_MOD_PASS_Obj_t *passMod, PARENA_Tag_t *tag,
                             struct SSOS_LIST_Head_s checkFirstHash[CHECK_FIRST_HASH_SIZE])
{
    unsigned int                     i        = 0;
    unsigned int                     hashVal  = 0;
    PTREE_SUR_Info_t *               info     = NULL;
    PTREE_SUR_InInfo_t *             inInfo   = NULL;
    PTREE_MOD_PASS_MembersObjNode_t *node     = NULL;
    PTREE_MOD_PASS_MembersObjNode_t *prevNode = NULL;
    PTREE_MAKER_Obj_t *              maker    = NULL;
    PTREE_MOD_Obj_t *                mod      = NULL;
    PTREE_CMD_Obj_t *                cmd      = NULL;
    PARENA_Tag_t *                   prevTag  = NULL;

    info    = (PTREE_SUR_Info_t *)PARENA_USE_BASE(tag);
    hashVal = _PTREE_MOD_PASS_MemberHash(info->typeName, info->devId, info->chnId);
    node    = _PTREE_MOD_PASS_FindMemberInHash(&passMod->memberHash[hashVal], info->typeName, info->devId, info->chnId);
    if (node)
    {
        if (_PTREE_MOD_PASS_IsAccessFirst(checkFirstHash, tag->u32Offset))
        {
            PTREE_DBG("LOOP: %s", info->sectionName);
            node->objUseCnt++;
            mod = node->pstModObj;
            for (i = 0; i < info->inCnt; i++)
            {
                inInfo  = mod->arrModIn[i]->info;
                prevTag = (PARENA_Tag_t *)PARENA_RELATIVE_ADDRESS(inInfo, inInfo->prevTagOffset);
                _PTREE_MOD_PASS_SignatureMembers(prevTag, passMod->memberHash, checkFirstHash);
            }
        }
        return node;
    }
    PTREE_DBG("NEW: %s", info->sectionName);
    maker = PTREE_MAKER_Get(info->typeName);
    if (!maker)
    {
        PTREE_ERR("Get %s maker err.", info->typeName);
        return NULL;
    }
    mod = PTREE_MAKER_MakeMod(maker, tag);
    if (!mod)
    {
        PTREE_ERR("object error.");
        return NULL;
    }
    PTREE_MOD_CreateIo(mod);
    cmd  = PTREE_MAKER_MakeCmd(maker);
    node = (PTREE_MOD_PASS_MembersObjNode_t *)SSOS_MEM_Alloc(sizeof(PTREE_MOD_PASS_MembersObjNode_t));
    if (!node)
    {
        PTREE_MAKER_DelMod(mod);
        PTREE_ERR("Alloc error!");
        return NULL;
    }
    memset(node, 0, sizeof(PTREE_MOD_PASS_MembersObjNode_t));
    node->pstModObj = mod;
    node->pstCmdObj = cmd;
    node->objUseCnt = 1;
    if (_PTREE_MOD_PASS_AccessValue(checkFirstHash, tag->u32Offset) == SSOS_DEF_FAIL)
    {
        PTREE_MAKER_DelMod(mod);
        SSOS_MEM_Free(node);
        PTREE_ERR("Access value error!");
        return NULL;
    }
    SSOS_LIST_AddTail(&node->memberHash, &passMod->memberHash[hashVal]);
    for (i = 0; i < info->inCnt; i++)
    {
        inInfo = mod->arrModIn[i]->info;
        if (!strlen(inInfo->prevSection))
        {
            /* the tail module in 'PASS' has no previous section name. */
            continue;
        }
        prevTag = (PARENA_Tag_t *)PARENA_RELATIVE_ADDRESS(inInfo, inInfo->prevTagOffset);
        if (prevTag->u32Magic != __PARENA_MAGIC_NUM__)
        {
            PTREE_ERR("Get prev maigc num error!, section: %s", info->sectionName);
            goto ERROR_IMPLEMENT;
        }
        prevNode = _PTREE_MOD_PASS_ImplementBin(passMod, prevTag, checkFirstHash);
        if (!prevNode)
        {
            goto ERROR_IMPLEMENT;
        }
        PTREE_MOD_Link(node->pstModObj->arrModIn[i], prevNode->pstModObj->arrModOut[inInfo->prevOutId]);
    }
    return node;
ERROR_IMPLEMENT:
    for (; i > 0; i--)
    {
        PTREE_MOD_Unlink(node->pstModObj->arrModIn[i - 1]);
    }
    _PTREE_MOD_PASS_LeaveValue(checkFirstHash, tag->u32Offset);
    PTREE_MAKER_DelMod(node->pstModObj);
    SSOS_LIST_Del(&node->memberHash);
    SSOS_MEM_Free(node);
    return NULL;
}

static int _PTREE_MOD_PASS_TraversePipeline(PTREE_MOD_PASS_MembersObjNode_t *node, struct SSOS_LIST_Head_s *orderList,
                                            struct SSOS_LIST_Head_s memberHash[PTREE_PASS_MOD_HASH_SIZE],
                                            struct SSOS_LIST_Head_s checkFirstHash[CHECK_FIRST_HASH_SIZE])
{
    unsigned int                           hashVal     = 0;
    unsigned int                           i           = 0;
    PTREE_SUR_Info_t *                     prevInfo    = NULL;
    PTREE_MOD_OutObj_t *                   prevModOut  = NULL;
    PTREE_MOD_PASS_MemberObjOrderedNode_t *orderedNode = NULL;
    PTREE_MOD_Obj_t *                      mod;

    mod = node->pstModObj;
    if (!_PTREE_MOD_PASS_IsAccessFirst(checkFirstHash, mod->tag->u32Offset))
    {
        /* Do nothing if traverse the samve member. */
        return SSOS_DEF_OK;
    }
    if (_PTREE_MOD_PASS_AccessValue(checkFirstHash, mod->tag->u32Offset) == SSOS_DEF_FAIL)
    {
        PTREE_ERR("Access value error!");
        return SSOS_DEF_FAIL;
    }
    orderedNode =
        (PTREE_MOD_PASS_MemberObjOrderedNode_t *)SSOS_MEM_Alloc(sizeof(PTREE_MOD_PASS_MemberObjOrderedNode_t));
    if (!orderedNode)
    {
        _PTREE_MOD_PASS_LeaveValue(checkFirstHash, mod->tag->u32Offset);
        return SSOS_DEF_FAIL;
    }
    orderedNode->memberObj = node;
    for (i = 0; i < mod->info->inCnt; i++)
    {
        prevModOut = mod->arrModIn[i]->prevModOut;
        prevInfo   = prevModOut->thisMod->info;
        hashVal    = _PTREE_MOD_PASS_MemberHash(prevInfo->typeName, prevInfo->devId, prevInfo->chnId);
        node       = _PTREE_MOD_PASS_FindMemberInHash(&memberHash[hashVal], prevInfo->typeName, prevInfo->devId,
                                                      prevInfo->chnId);
        if (!node)
        {
            /*
             * If module on the edge of pass input, it make memberHash not be found.
             */
            continue;
        }
        _PTREE_MOD_PASS_TraversePipeline(node, orderList, memberHash, checkFirstHash);
    }
    SSOS_LIST_AddTail(&orderedNode->orderList, orderList);
    return SSOS_DEF_OK;
}

static int _PTREE_MOD_PASS_GetMemberObjIdx(PTREE_MOD_PASS_OutObj_t *passOutObj, struct SSOS_LIST_Head_s *orderList,
                                           unsigned int rootIdx)
{
    PTREE_MOD_PASS_Obj_t *passObj = CONTAINER_OF(passOutObj->base.thisMod, PTREE_MOD_PASS_Obj_t, base);
    LIST_HEAD_ARRAY(checkFirstHash, CHECK_FIRST_HASH_SIZE);

    if (rootIdx >= PARENA_ROOT_COUNT_MAX || !passOutObj->rootMembers[rootIdx])
    {
        PTREE_ERR("ROOT IDX %d Loop error!", rootIdx);
        return SSOS_DEF_FAIL;
    }
    _PTREE_MOD_PASS_TraversePipeline(passOutObj->rootMembers[rootIdx], orderList, passObj->memberHash, checkFirstHash);
    _PTREE_MOD_PASS_FreeHash(checkFirstHash, CHECK_FIRST_HASH_SIZE, _PTREE_MOD_PASS_FreeAccessNode);
    return SSOS_LIST_Empty(orderList) ? SSOS_DEF_FAIL : SSOS_DEF_OK;
}

static int _PTREE_MOD_PASS_GetMemberObjAll(PTREE_MOD_PASS_OutObj_t *passOutObj, struct SSOS_LIST_Head_s *orderList)
{
    unsigned int          i       = 0;
    PTREE_MOD_PASS_Obj_t *passObj = CONTAINER_OF(passOutObj->base.thisMod, PTREE_MOD_PASS_Obj_t, base);
    LIST_HEAD_ARRAY(checkFirstHash, CHECK_FIRST_HASH_SIZE);

    for (i = 0; i < PARENA_ROOT_COUNT_MAX; i++)
    {
        if (!passOutObj->rootMembers[i])
        {
            break;
        }
        _PTREE_MOD_PASS_TraversePipeline(passOutObj->rootMembers[i], orderList, passObj->memberHash, checkFirstHash);
        _PTREE_MOD_PASS_FreeHash(checkFirstHash, CHECK_FIRST_HASH_SIZE, _PTREE_MOD_PASS_FreeAccessNode);
    }
    return SSOS_LIST_Empty(orderList) ? SSOS_DEF_FAIL : SSOS_DEF_OK;
}

static int _PTREE_MOD_PASS_CheckStateIdx(PTREE_MOD_PASS_OutObj_t *passOutObj, unsigned int rootIdx,
                                         enum PTREE_MOD_PASS_ResourceState_e targetSt)
{
    if (rootIdx >= PARENA_ROOT_COUNT_MAX || !passOutObj->rootMembers[rootIdx])
    {
        PTREE_ERR("rootIdx %d resource didn't exist.", rootIdx);
        return SSOS_DEF_FAIL;
    }
    if (!(passOutObj->arrResSt[rootIdx] & E_PTREE_MOD_PASS_RESOURCE_CREATE))
    {
        PTREE_ERR("PASS-OUT %d ROOT %d didn't creat!", passOutObj->base.info->port, rootIdx);
        return SSOS_DEF_FAIL;
    }
    if (!(passOutObj->arrResSt[rootIdx] & E_PTREE_MOD_PASS_RESOURCE_INIT)
        && (targetSt == E_PTREE_MOD_PASS_RESOURCE_BIND || targetSt == E_PTREE_MOD_PASS_RESOURCE_START))
    {
        PTREE_ERR("PASS-OUT %d ROOT %d must init first!", passOutObj->base.info->port, rootIdx);
        return SSOS_DEF_FAIL;
    }
    if (targetSt == E_PTREE_MOD_PASS_RESOURCE_INIT && (passOutObj->arrResSt[rootIdx] & targetSt))
    {
        PTREE_ERR("PASS-OUT %d ROOT %d init duplicated!", passOutObj->base.info->port, rootIdx);
        return SSOS_DEF_FAIL;
    }
    if (targetSt == E_PTREE_MOD_PASS_RESOURCE_BIND && (passOutObj->arrResSt[rootIdx] & targetSt))
    {
        PTREE_ERR("PASS-OUT %d ROOT %d bind duplicated!", passOutObj->base.info->port, rootIdx);
        return SSOS_DEF_FAIL;
    }
    if (targetSt == E_PTREE_MOD_PASS_RESOURCE_START && (passOutObj->arrResSt[rootIdx] & targetSt))
    {
        PTREE_ERR("PASS-OUT %d ROOT %d start duplicated!", passOutObj->base.info->port, rootIdx);
        return SSOS_DEF_FAIL;
    }
    return SSOS_DEF_OK;
}

static int _PTREE_MOD_PASS_CheckState(PTREE_MOD_PASS_OutObj_t *passOutObj, enum PTREE_MOD_PASS_ResourceState_e targetSt)
{
    int i = 0;
    for (i = 0; i < PARENA_ROOT_COUNT_MAX; i++)
    {
        if (!passOutObj->rootMembers[i])
        {
            break;
        }
        if (!(passOutObj->arrResSt[i] & E_PTREE_MOD_PASS_RESOURCE_CREATE))
        {
            PTREE_ERR("PASS-OUT %d didn't creat!", passOutObj->base.info->port);
            return SSOS_DEF_FAIL;
        }
        if (!(passOutObj->arrResSt[i] & E_PTREE_MOD_PASS_RESOURCE_INIT)
            && (targetSt == E_PTREE_MOD_PASS_RESOURCE_BIND || targetSt == E_PTREE_MOD_PASS_RESOURCE_START))
        {
            PTREE_ERR("PASS-OUT %d must init first!", passOutObj->base.info->port);
            return SSOS_DEF_FAIL;
        }
        if (targetSt == E_PTREE_MOD_PASS_RESOURCE_INIT && (passOutObj->arrResSt[i] & targetSt))
        {
            PTREE_ERR("PASS-OUT %d init duplicated!", passOutObj->base.info->port);
            return SSOS_DEF_FAIL;
        }
        if (targetSt == E_PTREE_MOD_PASS_RESOURCE_BIND && (passOutObj->arrResSt[i] & targetSt))
        {
            PTREE_ERR("PASS-OUT %d bind duplicated!", passOutObj->base.info->port);
            return SSOS_DEF_FAIL;
        }
        if (targetSt == E_PTREE_MOD_PASS_RESOURCE_START && (passOutObj->arrResSt[i] & targetSt))
        {
            PTREE_ERR("PASS-OUT %d start duplicated!", passOutObj->base.info->port);
            return SSOS_DEF_FAIL;
        }
    }
    return (i == 0) ? SSOS_DEF_FAIL : SSOS_DEF_OK;
}

static void _PTREE_MOD_PASS_MarkStateIdx(PTREE_MOD_PASS_OutObj_t *passOutObj, unsigned int rootIdx,
                                         enum PTREE_MOD_PASS_ResourceState_e targetSt)
{
    passOutObj->arrResSt[rootIdx] |= targetSt;
}

static void _PTREE_MOD_PASS_MarkState(PTREE_MOD_PASS_OutObj_t *passOutObj, enum PTREE_MOD_PASS_ResourceState_e targetSt)
{
    int i = 0;
    for (i = 0; i < PARENA_ROOT_COUNT_MAX; i++)
    {
        if (!passOutObj->rootMembers[i])
        {
            break;
        }
        passOutObj->arrResSt[i] |= targetSt;
    }
}

static void _PTREE_MOD_PASS_UnmarkStateIdx(PTREE_MOD_PASS_OutObj_t *passOutObj, unsigned int rootIdx,
                                           enum PTREE_MOD_PASS_ResourceState_e targetSt)
{
    passOutObj->arrResSt[rootIdx] &= (~(unsigned int)targetSt);
}

static void _PTREE_MOD_PASS_UnmarkState(PTREE_MOD_PASS_OutObj_t *           passOutObj,
                                        enum PTREE_MOD_PASS_ResourceState_e targetSt)
{
    int i = 0;
    for (i = 0; i < PARENA_ROOT_COUNT_MAX; i++)
    {
        passOutObj->arrResSt[i] &= (~(unsigned int)targetSt);
    }
}

static int _PTREE_MOD_PASS_DoCreateDelayPass(PTREE_MOD_PASS_OutObj_t *passOutObj)
{
    int                              i           = 0;
    int                              ret         = 0;
    unsigned int                     hashVal     = 0;
    void *                           arena       = NULL;
    PARENA_Tag_t *                   tag         = NULL;
    PTREE_MOD_Obj_t *                prevObj     = NULL;
    PTREE_MOD_OutObj_t *             prevOutObj  = NULL;
    PTREE_MOD_InObj_t *              memberInObj = NULL;
    PTREE_MOD_PASS_MembersObjNode_t *inMemberObj = NULL;
    PTREE_MOD_PASS_InObj_t *         passInObj   = NULL;
    PTREE_SUR_PASS_InInfo_t *        passInInfo  = NULL;
    PTREE_MOD_PASS_Obj_t *           passObj     = CONTAINER_OF(passOutObj->base.thisMod, PTREE_MOD_PASS_Obj_t, base);
    PTREE_SUR_PASS_OutInfo_t *       passOutInfo = CONTAINER_OF(passOutObj->base.info, PTREE_SUR_PASS_OutInfo_t, base);
    PTREE_SUR_Info_t *               info        = passObj->base.info;
    LIST_HEAD_ARRAY(checkFirstHash, CHECK_FIRST_HASH_SIZE);

    ret = ARENA_Map(&arena, passOutInfo->arenaBuffer, 4096);
    if (ret != 0)
    {
        PTREE_ERR("Map error!");
        return ret;
    }
    for (i = 0; i < PARENA_ROOT_COUNT_MAX && passOutInfo->rootOffset[i] != -1; i++)
    {
        if (passOutObj->arrResSt[i] != E_PTREE_MOD_PASS_RESOURCE_NONE)
        {
            PTREE_ERR("Fail: Resource create again!");
            ARENA_Unmap(arena);
            return SSOS_DEF_FAIL;
        }
        tag                        = (PARENA_Tag_t *)ARENA_Addr(arena, passOutInfo->rootOffset[i]);
        passOutObj->rootMembers[i] = _PTREE_MOD_PASS_ImplementBin(passObj, tag, checkFirstHash);
        _PTREE_MOD_PASS_FreeHash(checkFirstHash, CHECK_FIRST_HASH_SIZE, _PTREE_MOD_PASS_FreeAccessNode);
        if (!passOutObj->rootMembers[i])
        {
            PTREE_ERR("Fail: Resource create failed!");
            ARENA_Unmap(arena);
            return SSOS_DEF_FAIL;
        }
        passOutObj->arrResSt[i] |= E_PTREE_MOD_PASS_RESOURCE_CREATE;
    }
    ARENA_Unmap(arena);
    /* Unlink the edge of 'PASS' input and link to the module of 'PASS' input. */
    for (i = 0; i < info->inCnt; i++)
    {
        passInInfo  = CONTAINER_OF(passObj->base.arrModIn[i]->info, PTREE_SUR_PASS_InInfo_t, base);
        hashVal     = _PTREE_MOD_PASS_MemberHash(passInInfo->modName, passInInfo->devId, passInInfo->chnId);
        inMemberObj = _PTREE_MOD_PASS_FindMemberInHash(&passObj->memberHash[hashVal], passInInfo->modName,
                                                       passInInfo->devId, passInInfo->chnId);
        if (!inMemberObj)
        {
            continue;
        }
        passInObj = CONTAINER_OF(passObj->base.arrModIn[i], PTREE_MOD_PASS_InObj_t, base);
        if (!passInObj->base.prevModOut)
        {
            /* After other module had beed linked, it is normal that makes 'PASS' can not find input port. */
            continue;
        }
        prevOutObj  = passInObj->base.prevModOut;
        memberInObj = PTREE_MOD_GetInObjByPort(inMemberObj->pstModObj, passInInfo->secPortId);
        if (!memberInObj)
        {
            /* Just print error here and return shouble be successed. */
            PTREE_ERR("Member in port %d is null!", passInInfo->secPortId);
            continue;
        }
        inMemberObj->bObjInEdge = 1;
        prevObj                 = prevOutObj->thisMod;
        PTREE_MOD_Stop(prevObj);
        PTREE_MOD_StopOut(prevOutObj);
        PTREE_DBG("In link replace forward:");
        PRINT_PASS_RELINK(&passInInfo->base, prevOutObj->info, memberInObj->info);
        PTREE_MOD_StopIn(&passInObj->base);
        PTREE_MOD_Unbind(&passInObj->base);
        PTREE_MOD_Unlink(&passInObj->base);
        PTREE_MOD_Link(memberInObj, prevOutObj);
    }
    return SSOS_DEF_OK;
}

static int _PTREE_MOD_PASS_DoDestroyDelayPass(PTREE_MOD_PASS_OutObj_t *passOutObj, struct SSOS_LIST_Head_s *orderList)
{
    int                                    i           = 0;
    unsigned int                           hashVal     = 0;
    PTREE_MOD_Obj_t *                      prevObj     = NULL;
    PTREE_MOD_OutObj_t *                   prevOutObj  = NULL;
    PTREE_MOD_InObj_t *                    memberInObj = NULL;
    PTREE_MOD_PASS_MemberObjOrderedNode_t *pos         = NULL;
    PTREE_MOD_PASS_MemberObjOrderedNode_t *posN        = NULL;
    PTREE_MOD_PASS_MembersObjNode_t *      inMemberObj = NULL;
    PTREE_MOD_PASS_InObj_t *               passInObj   = NULL;
    PTREE_SUR_PASS_InInfo_t *              passInInfo  = NULL;
    PTREE_MOD_PASS_Obj_t *                 passObj = CONTAINER_OF(passOutObj->base.thisMod, PTREE_MOD_PASS_Obj_t, base);
    PTREE_SUR_Info_t *                     info    = passObj->base.info;

    SSOS_LIST_FOR_EACH_ENTRY_SAFE(pos, posN, orderList, orderList)
    {
        if (pos->memberObj->objUseCnt > 1)
        {
            pos->memberObj->objUseCnt--;
            SSOS_LIST_Del(&pos->orderList);
            SSOS_MEM_Free(pos);
            /* Just del and free the reference of memberObj, which make no influence. */
            continue;
        }
        /* Only keep the node with reference of cnt=0 to do memberObj destruction later. */
        pos->memberObj->objUseCnt = 0;
    }
    /* Unlink the edge module in 'PASS' input and link to 'PASS' of itself. */
    for (i = 0; i < info->inCnt; i++)
    {
        passInInfo  = CONTAINER_OF(passObj->base.arrModIn[i]->info, PTREE_SUR_PASS_InInfo_t, base);
        hashVal     = _PTREE_MOD_PASS_MemberHash(passInInfo->modName, passInInfo->devId, passInInfo->chnId);
        inMemberObj = _PTREE_MOD_PASS_FindMemberInHash(&passObj->memberHash[hashVal], passInInfo->modName,
                                                       passInInfo->devId, passInInfo->chnId);
        if (!inMemberObj)
        {
            PTREE_DBG("Obj is null\n");
            continue;
        }
        if (inMemberObj->objUseCnt > 0)
        {
            PTREE_DBG("Obj Cnt %d, sec: %s", inMemberObj->objUseCnt, inMemberObj->pstModObj->info->sectionName);
            continue;
        }
        if (!inMemberObj->pstModObj)
        {
            PTREE_ERR("Mod object is null");
            return SSOS_DEF_FAIL;
        }
        passInObj   = CONTAINER_OF(passObj->base.arrModIn[i], PTREE_MOD_PASS_InObj_t, base);
        memberInObj = PTREE_MOD_GetInObjByPort(inMemberObj->pstModObj, passInInfo->secPortId);
        if (!memberInObj || !memberInObj->prevModOut)
        {
            PTREE_ERR("Member in port %d is null!", passInInfo->secPortId);
            return SSOS_DEF_FAIL;
        }
        prevOutObj = memberInObj->prevModOut;
        prevObj    = prevOutObj->thisMod;
        PTREE_DBG("In unlink replace forward:");
        PRINT_PASS_RELINK(memberInObj->info, prevOutObj->info, passInObj->base.info);
        PTREE_MOD_Unlink(memberInObj);
        PTREE_MOD_Link(&passInObj->base, prevOutObj);
        PTREE_MOD_Bind(&passInObj->base);
        PTREE_MOD_StartIn(&passInObj->base);
        PTREE_MOD_StartOut(prevOutObj);
        PTREE_MOD_Start(prevObj);
    }
    SSOS_LIST_FOR_EACH_ENTRY_SAFE(pos, posN, orderList, orderList)
    {
        /* The reset nodes are all useCnt = 0. */
        for (i = 0; i < pos->memberObj->pstModObj->info->inCnt; i++)
        {
            memberInObj = pos->memberObj->pstModObj->arrModIn[i];
            PTREE_MOD_Unlink(memberInObj);
        }
        /*
         * 'Amigos' should do unload db here but 'ptree' does not care,
         * because the info buffer which is managed by 'arena' was loacated at pass's out info.
         */
        PTREE_DBG("FREE: %s", pos->memberObj->pstModObj->info->sectionName);
        PTREE_MAKER_DelMod(pos->memberObj->pstModObj);
        SSOS_LIST_Del(&pos->memberObj->memberHash);
        SSOS_LIST_Del(&pos->orderList);
        SSOS_MEM_Free(pos->memberObj);
        SSOS_MEM_Free(pos);
    }
    memset(passOutObj->rootMembers, 0, sizeof(PTREE_MOD_PASS_MembersObjNode_t *) * PARENA_ROOT_COUNT_MAX);
    memset(passOutObj->arrResSt, 0, sizeof(unsigned int) * PARENA_ROOT_COUNT_MAX);
    return SSOS_DEF_OK;
}

static int _PTREE_MOD_PASS_DoInitDelayPass(struct SSOS_LIST_Head_s *orderList)
{
    PTREE_MOD_PASS_MemberObjOrderedNode_t *pos  = NULL;
    PTREE_MOD_PASS_MemberObjOrderedNode_t *posN = NULL;
    SSOS_LIST_FOR_EACH_ENTRY_SAFE(pos, posN, orderList, orderList)
    {
        if (pos->memberObj->objInitCnt > 0)
        {
            pos->memberObj->objInitCnt++;
            PTREE_DBG("DELAY-PASS-SKIP] : current section: %s", pos->memberObj->pstModObj->info->sectionName);
            SSOS_LIST_Del(&pos->orderList);
            SSOS_MEM_Free(pos);
            continue;
        }
        PTREE_DBG("[DELAY-PASS-INIT] : current section: %s", pos->memberObj->pstModObj->info->sectionName);
        PTREE_MOD_Init(pos->memberObj->pstModObj);
        pos->memberObj->objInitCnt = 1;
        SSOS_LIST_Del(&pos->orderList);
        SSOS_MEM_Free(pos);
    }
    return SSOS_DEF_OK;
}

static int _PTREE_MOD_PASS_DoDeinitDelayPass(struct SSOS_LIST_Head_s *orderList)
{
    PTREE_MOD_PASS_MemberObjOrderedNode_t *pos  = NULL;
    PTREE_MOD_PASS_MemberObjOrderedNode_t *posN = NULL;

    SSOS_LIST_FOR_EACH_ENTRY(pos, orderList, orderList)
    {
        if (pos->memberObj->objStartCnt >= pos->memberObj->objInitCnt
            || pos->memberObj->objBindCnt >= pos->memberObj->objInitCnt || pos->memberObj->objInitCnt == 0)
        {
            PTREE_ERR(
                "[DELAY-PASS-ERR] : current section: %s, Didn't stop or unbind delay-pass or delay-pass hadn't init "
                "before.",
                pos->memberObj->pstModObj->info->sectionName);
            return SSOS_DEF_FAIL;
        }
    }
    SSOS_LIST_FOR_EACH_ENTRY_SAFE_REVERSE(pos, posN, orderList, orderList)
    {
        if (pos->memberObj->objInitCnt > 1)
        {
            PTREE_DBG("DELAY-PASS-SKIP] : current section: %s", pos->memberObj->pstModObj->info->sectionName);
            pos->memberObj->objInitCnt--;
            SSOS_LIST_Del(&pos->orderList);
            SSOS_MEM_Free(pos);
            continue;
        }
        PTREE_DBG("[DELAY-PASS-INIT] : current section: %s", pos->memberObj->pstModObj->info->sectionName);
        PTREE_MOD_Deinit(pos->memberObj->pstModObj);
        pos->memberObj->objInitCnt = 0;
        SSOS_LIST_Del(&pos->orderList);
        SSOS_MEM_Free(pos);
    }
    return SSOS_DEF_OK;
}

static int _PTREE_MOD_PASS_DoBindDelayPass(struct SSOS_LIST_Head_s *orderList)
{
    PTREE_MOD_PASS_MemberObjOrderedNode_t *pos  = NULL;
    PTREE_MOD_PASS_MemberObjOrderedNode_t *posN = NULL;

    SSOS_LIST_FOR_EACH_ENTRY_SAFE_REVERSE(pos, posN, orderList, orderList)
    {
        unsigned char      i     = 0;
        PTREE_MOD_InObj_t *modIn = NULL;
        if (pos->memberObj->objBindCnt > 0)
        {
            PTREE_DBG("DELAY-PASS-SKIP] : current section: %s", pos->memberObj->pstModObj->info->sectionName);
            pos->memberObj->objBindCnt++;
            SSOS_LIST_Del(&pos->orderList);
            SSOS_MEM_Free(pos);
            continue;
        }
        PTREE_DBG("[DELAY-PASS-BIND] : current section: %s", pos->memberObj->pstModObj->info->sectionName);
        for (i = 0; i < pos->memberObj->pstModObj->info->inCnt; i++)
        {
            modIn = pos->memberObj->pstModObj->arrModIn[i];
            PTREE_MOD_Bind(modIn);
        }
        pos->memberObj->objBindCnt = 1;
        SSOS_LIST_Del(&pos->orderList);
        SSOS_MEM_Free(pos);
    }
    return SSOS_DEF_OK;
}

static int _PTREE_MOD_PASS_DoUnbindDelayPass(struct SSOS_LIST_Head_s *orderList)
{
    PTREE_MOD_PASS_MemberObjOrderedNode_t *pos  = NULL;
    PTREE_MOD_PASS_MemberObjOrderedNode_t *posN = NULL;

    SSOS_LIST_FOR_EACH_ENTRY(pos, orderList, orderList)
    {
        if (pos->memberObj->objBindCnt == 0)
        {
            PTREE_ERR("[DELAY-PASS-UNBIND] : current section: %s did not bind.",
                      pos->memberObj->pstModObj->info->sectionName);
            return SSOS_DEF_FAIL;
        }
    }
    SSOS_LIST_FOR_EACH_ENTRY_SAFE(pos, posN, orderList, orderList)
    {
        unsigned char      i     = 0;
        PTREE_MOD_InObj_t *modIn = NULL;
        if (pos->memberObj->objBindCnt > 1)
        {
            PTREE_DBG("DELAY-PASS-SKIP] : current section: %s", pos->memberObj->pstModObj->info->sectionName);
            pos->memberObj->objBindCnt--;
            SSOS_LIST_Del(&pos->orderList);
            SSOS_MEM_Free(pos);
            continue;
        }
        PTREE_DBG("[DELAY-PASS-UNBIND] : current section: %s", pos->memberObj->pstModObj->info->sectionName);
        for (i = 0; i < pos->memberObj->pstModObj->info->inCnt; i++)
        {
            modIn = pos->memberObj->pstModObj->arrModIn[i];
            PTREE_MOD_Unbind(modIn);
        }
        pos->memberObj->objBindCnt = 0;
        SSOS_LIST_Del(&pos->orderList);
        SSOS_MEM_Free(pos);
    }
    return SSOS_DEF_OK;
}

static int _PTREE_MOD_PASS_DoStartDelayPass(struct SSOS_LIST_Head_s *orderList,
                                            struct SSOS_LIST_Head_s  memberHash[PTREE_PASS_MOD_HASH_SIZE])
{
    PTREE_MOD_PASS_MemberObjOrderedNode_t *pos  = NULL;
    PTREE_MOD_PASS_MemberObjOrderedNode_t *posN = NULL;

    SSOS_LIST_FOR_EACH_ENTRY_SAFE_REVERSE(pos, posN, orderList, orderList)
    {
        unsigned char i = 0;
        if (pos->memberObj->objStartCnt > 0)
        {
            PTREE_DBG("[DELAY-PASS-SKIP] : current section: %s", pos->memberObj->pstModObj->info->sectionName);
            pos->memberObj->objStartCnt++;
            SSOS_LIST_Del(&pos->orderList);
            SSOS_MEM_Free(pos);
            continue;
        }
        PTREE_MOD_Prepare(pos->memberObj->pstModObj);
        for (i = 0; i < pos->memberObj->pstModObj->info->outCnt; i++)
        {
            PTREE_MOD_OutObj_t *modOut = pos->memberObj->pstModObj->arrModOut[i];
            PTREE_MOD_StartOut(modOut);
        }
        for (i = 0; i < pos->memberObj->pstModObj->info->inCnt; i++)
        {
            PTREE_MOD_InObj_t *modIn = pos->memberObj->pstModObj->arrModIn[i];
            PTREE_MOD_StartIn(modIn);
        }
        PTREE_MOD_Start(pos->memberObj->pstModObj);
        pos->memberObj->objStartCnt = 1;
        PTREE_DBG("[DELAY-PASS-START-IO] : section: %s", pos->memberObj->pstModObj->info->sectionName);
        for (i = 0; i < pos->memberObj->pstModObj->info->inCnt; i++)
        {
            unsigned int                     hashVal    = 0;
            PTREE_MOD_PASS_MembersObjNode_t *node       = NULL;
            PTREE_MOD_InObj_t *              modIn      = pos->memberObj->pstModObj->arrModIn[i];
            PTREE_SUR_Info_t *               prevInfo   = modIn->prevModOut->thisMod->info;
            PTREE_MOD_OutObj_t *             prevModOut = modIn->prevModOut;

            hashVal = _PTREE_MOD_PASS_MemberHash(prevInfo->typeName, prevInfo->devId, prevInfo->chnId);
            node    = _PTREE_MOD_PASS_FindMemberInHash(&memberHash[hashVal], prevInfo->typeName, prevInfo->devId,
                                                       prevInfo->chnId);
            if (!node)
            {
                /* It make passMember not be found, if module on the edge of pass input.
                 *
                 * Example:
                 *       |-------------|
                 * |---| |PASS|-----|--|--Out0(), Module XX in 'PASS' had beed created by trigger on out0.
                 * |AA |-|----| XX  |  |
                 * |---| |    |-----|--|--Out1(), After out0 had been triggered, module 'XX' just run '_StartOut' only.
                 *       |-------------|
                 */
                PTREE_DBG("[DELAY-PASS-START-OUT] : section: %s PORT: %d", prevInfo->sectionName,
                          prevModOut->info->port);
                PTREE_MOD_StartOut(prevModOut);
                PTREE_MOD_Start(prevModOut->thisMod);
            }
            else if (node->objStartCnt > 0)
            {
                /* It should just only start this out, if previous module which is inside 'PASS' had been created
                 * before.
                 *
                 * Example:
                 * |-------------------|
                 * |PASS               |
                 * | |----|------------|--Out0(), Module AA in 'PASS' had beed created by trigger this out0.
                 * | |    |            |
                 * |-| AA |   |----|   |
                 * | |    |---| BB |---|--Out1(), After out0 triggered, module 'AA' just run '_StartOut' only.
                 * | |----|   |----|   |
                 * |-------------------|
                 */
                PTREE_DBG("[DELAY-PASS-START-OUT] : section: %s PORT: %d", prevInfo->sectionName,
                          prevModOut->info->port);
                PTREE_MOD_StartOut(prevModOut);
            }
        }
        SSOS_LIST_Del(&pos->orderList);
        SSOS_MEM_Free(pos);
    }
    return SSOS_DEF_OK;
}

static int _PTREE_MOD_PASS_DoStopDelayPass(struct SSOS_LIST_Head_s *orderList,
                                           struct SSOS_LIST_Head_s  memberHash[PTREE_PASS_MOD_HASH_SIZE])
{
    PTREE_MOD_PASS_MemberObjOrderedNode_t *pos  = NULL;
    PTREE_MOD_PASS_MemberObjOrderedNode_t *posN = NULL;

    SSOS_LIST_FOR_EACH_ENTRY(pos, orderList, orderList)
    {
        if (pos->memberObj->objStartCnt == 0)
        {
            PTREE_ERR("[DELAY-PASS-STOP-IO] : current section: %s, did not start.",
                      pos->memberObj->pstModObj->info->sectionName);
            return SSOS_DEF_FAIL;
        }
    }
    SSOS_LIST_FOR_EACH_ENTRY_SAFE_REVERSE(pos, posN, orderList, orderList)
    {
        unsigned char i = 0;
        if (pos->memberObj->objStartCnt > 1)
        {
            PTREE_DBG("[DELAY-PASS-SKIP] : current section: %s", pos->memberObj->pstModObj->info->sectionName);
            pos->memberObj->objStartCnt--;
            SSOS_LIST_Del(&pos->orderList);
            SSOS_MEM_Free(pos);
            continue;
        }
        PTREE_MOD_Stop(pos->memberObj->pstModObj);
        for (i = 0; i < pos->memberObj->pstModObj->info->inCnt; i++)
        {
            PTREE_MOD_InObj_t *modIn = pos->memberObj->pstModObj->arrModIn[i];
            PTREE_MOD_StopIn(modIn);
        }
        for (i = 0; i < pos->memberObj->pstModObj->info->outCnt; i++)
        {
            PTREE_MOD_OutObj_t *modOut = pos->memberObj->pstModObj->arrModOut[i];
            PTREE_MOD_StopOut(modOut);
        }
        PTREE_MOD_Unprepare(pos->memberObj->pstModObj);
        for (i = 0; i < pos->memberObj->pstModObj->info->inCnt; i++)
        {
            unsigned int                     hashVal    = 0;
            PTREE_MOD_PASS_MembersObjNode_t *node       = NULL;
            PTREE_MOD_InObj_t *              modIn      = pos->memberObj->pstModObj->arrModIn[i];
            PTREE_SUR_Info_t *               prevInfo   = modIn->prevModOut->thisMod->info;
            PTREE_MOD_OutObj_t *             prevModOut = modIn->prevModOut;

            hashVal = _PTREE_MOD_PASS_MemberHash(prevInfo->typeName, prevInfo->devId, prevInfo->chnId);
            node    = _PTREE_MOD_PASS_FindMemberInHash(&memberHash[hashVal], prevInfo->typeName, prevInfo->devId,
                                                       prevInfo->chnId);
            if (!node)
            {
                /* It make passMember not be found, if module on the edge of pass input. */
                PTREE_MOD_Stop(prevModOut->thisMod);
                PTREE_MOD_StopOut(prevModOut);
                PTREE_DBG("[DELAY-PASS-STOP-OUT] : section: %s PORT: %d", prevInfo->sectionName,
                          prevModOut->info->port);
            }
            else if (node->objStartCnt > 1)
            {
                /* It should just only start this out, if previous module which is inside 'PASS' had been created
                 * before. */
                PTREE_MOD_StopOut(prevModOut);
                PTREE_DBG("[DELAY-PASS-STOP-OUT] : section: %s PORT: %d", prevInfo->sectionName,
                          prevModOut->info->port);
            }
        }
        pos->memberObj->objStartCnt = 0;
        PTREE_DBG("[DELAY-PASS-STOP-IO] : section: %s", pos->memberObj->pstModObj->info->sectionName);
        SSOS_LIST_Del(&pos->orderList);
        SSOS_MEM_Free(pos);
    }
    return SSOS_DEF_OK;
}

static int _PTREE_MOD_PASS_CreateDelayPass(PTREE_MOD_OutObj_t *modOut, PTREE_MOD_InObj_t *postModIn)
{
    PTREE_MOD_OutObj_t *      outObj      = NULL;
    PTREE_MOD_PASS_OutObj_t * passOutObj  = CONTAINER_OF(modOut, PTREE_MOD_PASS_OutObj_t, base);
    PTREE_SUR_PASS_OutInfo_t *passOutInfo = CONTAINER_OF(modOut->info, PTREE_SUR_PASS_OutInfo_t, base);
    int                       ret         = 0;

    if (passOutInfo->headIdx >= PARENA_ROOT_COUNT_MAX)
    {
        PTREE_ERR("PASS-OUT %d No head out seciton!", passOutObj->base.info->port);
        return SSOS_DEF_FAIL;
    }
    ret = _PTREE_MOD_PASS_DoCreateDelayPass(passOutObj);
    if (ret != SSOS_DEF_OK)
    {
        PTREE_ERR("Create delay-pass failed! PORT: %d", modOut->info->port);
        return SSOS_DEF_FAIL;
    }
    outObj =
        PTREE_MOD_GetOutObjByPort(passOutObj->rootMembers[passOutInfo->headIdx]->pstModObj, passOutInfo->secPortId);
    /* Link the edge module in 'PASS' output. */
    PTREE_DBG("Out link Replace current :");
    PRINT_PASS_RELINK(postModIn->info, modOut->info, outObj->info);
    return PTREE_MOD_Link(postModIn, outObj);
}

static int _PTREE_MOD_PASS_DestroyDelayPass(PTREE_MOD_OutObj_t *modOut, PTREE_MOD_InObj_t *postModIn)
{
    int                                    ret         = 0;
    PTREE_MOD_PASS_MemberObjOrderedNode_t *pos         = NULL;
    PTREE_MOD_OutObj_t *                   outObj      = NULL;
    PTREE_MOD_PASS_OutObj_t *              passOutObj  = CONTAINER_OF(modOut, PTREE_MOD_PASS_OutObj_t, base);
    PTREE_SUR_PASS_OutInfo_t *             passOutInfo = CONTAINER_OF(modOut->info, PTREE_SUR_PASS_OutInfo_t, base);
    SSOS_LIST_HEAD(orderList);

    if (passOutInfo->headIdx >= PARENA_ROOT_COUNT_MAX)
    {
        PTREE_ERR("PASS-OUT %d No head out seciton!", passOutObj->base.info->port);
        return SSOS_DEF_FAIL;
    }
    if (passOutObj->arrResSt[passOutInfo->headIdx] == E_PTREE_MOD_PASS_RESOURCE_NONE)
    {
        PTREE_ERR("PASS-OUT %d Resouce didn't create.", passOutObj->base.info->port);
        return SSOS_DEF_FAIL;
    }
    if (!passOutObj->rootMembers[passOutInfo->headIdx] || !passOutObj->rootMembers[passOutInfo->headIdx]->pstModObj)
    {
        PTREE_ERR("PASS-OUT %d Head out resource error", passOutObj->base.info->port);
        return SSOS_DEF_FAIL;
    }
    ret = _PTREE_MOD_PASS_GetMemberObjAll(passOutObj, &orderList);
    if (ret == SSOS_DEF_FAIL)
    {
        PTREE_ERR("Output: %d object error!", passOutObj->base.info->port);
        return ret;
    }
    SSOS_LIST_FOR_EACH_ENTRY(pos, &orderList, orderList)
    {
        if (pos->memberObj->objStartCnt >= pos->memberObj->objUseCnt
            || pos->memberObj->objBindCnt >= pos->memberObj->objUseCnt
            || pos->memberObj->objInitCnt >= pos->memberObj->objUseCnt)
        {
            PTREE_ERR("[DELAY-PASS-ERR] : current section: %s, should stop and unbind and deinit delay-pass first",
                      pos->memberObj->pstModObj->info->sectionName);
            return SSOS_DEF_FAIL;
        }
    }
    outObj =
        PTREE_MOD_GetOutObjByPort(passOutObj->rootMembers[passOutInfo->headIdx]->pstModObj, passOutInfo->secPortId);
    /* Unlink the edge module in 'PASS' output. */
    PTREE_DBG("Out unlink replace current:");
    PRINT_PASS_RELINK(postModIn->info, outObj->info, modOut->info);
    PTREE_MOD_Unlink(postModIn);
    return _PTREE_MOD_PASS_DoDestroyDelayPass(passOutObj, &orderList);
}

static int _PTREE_MOD_PASS_InitDelayPass(PTREE_MOD_OutObj_t *modOut)
{
    int                      ret        = 0;
    PTREE_MOD_PASS_OutObj_t *passOutObj = CONTAINER_OF(modOut, PTREE_MOD_PASS_OutObj_t, base);
    SSOS_LIST_HEAD(orderList);

    ret = _PTREE_MOD_PASS_CheckState(passOutObj, E_PTREE_MOD_PASS_RESOURCE_INIT);
    if (ret != SSOS_DEF_OK)
    {
        return ret;
    }
    ret = _PTREE_MOD_PASS_GetMemberObjAll(passOutObj, &orderList);
    if (ret == SSOS_DEF_FAIL)
    {
        PTREE_ERR("Output: %d object error!", passOutObj->base.info->port);
        return ret;
    }
    ret = _PTREE_MOD_PASS_DoInitDelayPass(&orderList);
    if (ret == SSOS_DEF_OK)
    {
        _PTREE_MOD_PASS_MarkState(passOutObj, E_PTREE_MOD_PASS_RESOURCE_INIT);
    }
    return ret;
}

static int _PTREE_MOD_PASS_DeinitDelayPass(PTREE_MOD_OutObj_t *modOut)
{
    int                      ret        = 0;
    PTREE_MOD_PASS_OutObj_t *passOutObj = CONTAINER_OF(modOut, PTREE_MOD_PASS_OutObj_t, base);
    SSOS_LIST_HEAD(orderList);

    ret = _PTREE_MOD_PASS_GetMemberObjAll(passOutObj, &orderList);
    if (ret == SSOS_DEF_FAIL)
    {
        PTREE_ERR("Output: %d object error!", passOutObj->base.info->port);
        return ret;
    }
    ret = _PTREE_MOD_PASS_DoDeinitDelayPass(&orderList);
    if (ret == SSOS_DEF_OK)
    {
        _PTREE_MOD_PASS_UnmarkState(passOutObj, E_PTREE_MOD_PASS_RESOURCE_INIT);
    }
    return ret;
}

static int _PTREE_MOD_PASS_BindDelayPass(PTREE_MOD_OutObj_t *modOut)
{
    int                      ret        = 0;
    PTREE_MOD_PASS_OutObj_t *passOutObj = CONTAINER_OF(modOut, PTREE_MOD_PASS_OutObj_t, base);
    SSOS_LIST_HEAD(orderList);

    ret = _PTREE_MOD_PASS_CheckState(passOutObj, E_PTREE_MOD_PASS_RESOURCE_BIND);
    if (ret != SSOS_DEF_OK)
    {
        PTREE_ERR("PASS-OUT %d No reseource!", passOutObj->base.info->port);
        return ret;
    }
    ret = _PTREE_MOD_PASS_GetMemberObjAll(passOutObj, &orderList);
    if (ret != SSOS_DEF_OK)
    {
        PTREE_ERR("Output: %d object error!", passOutObj->base.info->port);
        return ret;
    }
    ret = _PTREE_MOD_PASS_DoBindDelayPass(&orderList);
    if (ret == SSOS_DEF_OK)
    {
        _PTREE_MOD_PASS_MarkState(passOutObj, E_PTREE_MOD_PASS_RESOURCE_BIND);
    }
    return ret;
}

static int _PTREE_MOD_PASS_UnbindDelayPass(PTREE_MOD_OutObj_t *modOut)
{
    int                      ret        = 0;
    PTREE_MOD_PASS_OutObj_t *passOutObj = CONTAINER_OF(modOut, PTREE_MOD_PASS_OutObj_t, base);
    SSOS_LIST_HEAD(orderList);

    ret = _PTREE_MOD_PASS_GetMemberObjAll(passOutObj, &orderList);
    if (ret != SSOS_DEF_OK)
    {
        PTREE_ERR("Output: %d object error!", passOutObj->base.info->port);
        return ret;
    }
    ret = _PTREE_MOD_PASS_DoUnbindDelayPass(&orderList);
    if (ret == SSOS_DEF_OK)
    {
        _PTREE_MOD_PASS_UnmarkState(passOutObj, E_PTREE_MOD_PASS_RESOURCE_BIND);
    }
    return ret;
}

static int _PTREE_MOD_PASS_StartDelayPass(PTREE_MOD_OutObj_t *modOut)
{
    int                       ret           = 0;
    PTREE_MOD_PASS_OutObj_t * passOutObj    = CONTAINER_OF(modOut, PTREE_MOD_PASS_OutObj_t, base);
    PTREE_MOD_PASS_Obj_t *    passObj       = CONTAINER_OF(passOutObj->base.thisMod, PTREE_MOD_PASS_Obj_t, base);
    PTREE_SUR_PASS_OutInfo_t *passOutInfo   = CONTAINER_OF(passOutObj->base.info, PTREE_SUR_PASS_OutInfo_t, base);
    PTREE_MOD_OutObj_t *      passMemOutObj = NULL;
    SSOS_LIST_HEAD(orderList);

    if (passOutInfo->headIdx >= PARENA_ROOT_COUNT_MAX)
    {
        PTREE_ERR("PASS-OUT %d No head out seciton!", passOutObj->base.info->port);
        return SSOS_DEF_FAIL;
    }
    ret = _PTREE_MOD_PASS_CheckState(passOutObj, E_PTREE_MOD_PASS_RESOURCE_START);
    if (ret != SSOS_DEF_OK)
    {
        return ret;
    }
    ret = _PTREE_MOD_PASS_GetMemberObjAll(passOutObj, &orderList);
    if (ret == SSOS_DEF_FAIL)
    {
        PTREE_ERR("Output: %d object error!", passOutObj->base.info->port);
        return ret;
    }
    ret = _PTREE_MOD_PASS_DoStartDelayPass(&orderList, passObj->memberHash);
    if (ret != SSOS_DEF_OK)
    {
        PTREE_ERR("Output: %d Start error!", modOut->info->port);
        return ret;
    }
    _PTREE_MOD_PASS_MarkState(passOutObj, E_PTREE_MOD_PASS_RESOURCE_START);
    /* It should just only start this out if module on the edge of pass output,
     * and if it was created by another pass before,
     *
     * Example:
     * |-------------|
     * |PASS|-----|--|--Out0(), Module XX in 'PASS' had beed created by trigger this out0.
     * |----| XX  |  |
     * |    |-----|--|--Out1(), After out0 triggered, module 'XX' just run '_StartOut' only.
     * |-------------|
     */
    passMemOutObj =
        PTREE_MOD_GetOutObjByPort(passOutObj->rootMembers[passOutInfo->headIdx]->pstModObj, passOutInfo->secPortId);
    return PTREE_MOD_StartOut(passMemOutObj);
}

static int _PTREE_MOD_PASS_StopDelayPass(PTREE_MOD_OutObj_t *modOut)
{
    int                       ret           = 0;
    PTREE_MOD_PASS_OutObj_t * passOutObj    = CONTAINER_OF(modOut, PTREE_MOD_PASS_OutObj_t, base);
    PTREE_MOD_PASS_Obj_t *    passObj       = CONTAINER_OF(passOutObj->base.thisMod, PTREE_MOD_PASS_Obj_t, base);
    PTREE_SUR_PASS_OutInfo_t *passOutInfo   = CONTAINER_OF(passOutObj->base.info, PTREE_SUR_PASS_OutInfo_t, base);
    PTREE_MOD_OutObj_t *      passMemOutObj = NULL;
    SSOS_LIST_HEAD(orderList);

    if (passOutInfo->headIdx >= PARENA_ROOT_COUNT_MAX)
    {
        PTREE_ERR("PASS-OUT %d No head out seciton!", passOutObj->base.info->port);
        return SSOS_DEF_FAIL;
    }
    ret = _PTREE_MOD_PASS_GetMemberObjAll(passOutObj, &orderList);
    if (ret != SSOS_DEF_OK)
    {
        PTREE_ERR("Output: %d object error!", passOutObj->base.info->port);
        return ret;
    }
    /* Stop the module on the edge of pass output. */
    passMemOutObj =
        PTREE_MOD_GetOutObjByPort(passOutObj->rootMembers[passOutInfo->headIdx]->pstModObj, passOutInfo->secPortId);
    PTREE_MOD_StopOut(passMemOutObj);
    ret = _PTREE_MOD_PASS_DoStopDelayPass(&orderList, passObj->memberHash);
    if (ret == SSOS_DEF_OK)
    {
        _PTREE_MOD_PASS_UnmarkState(passOutObj, E_PTREE_MOD_PASS_RESOURCE_START);
    }
    return ret;
}

int PTREE_MOD_PASS_BuildDelayPass(PTREE_MOD_OutObj_t *modOut)
{
    PTREE_MOD_PASS_OutObj_t *passOutObj = CONTAINER_OF(modOut, PTREE_MOD_PASS_OutObj_t, base);
    return _PTREE_MOD_PASS_DoCreateDelayPass(passOutObj);
}

int PTREE_MOD_PASS_DeleteDelayPass(PTREE_MOD_OutObj_t *modOut)
{
    int                                    ret        = 0;
    PTREE_MOD_PASS_MemberObjOrderedNode_t *pos        = NULL;
    PTREE_MOD_PASS_OutObj_t *              passOutObj = CONTAINER_OF(modOut, PTREE_MOD_PASS_OutObj_t, base);
    SSOS_LIST_HEAD(orderList);

    if (passOutObj->arrResSt[0] == E_PTREE_MOD_PASS_RESOURCE_NONE)
    {
        PTREE_ERR("PASS-OUT %d Resouce didn't create.", passOutObj->base.info->port);
        return SSOS_DEF_FAIL;
    }
    ret = _PTREE_MOD_PASS_GetMemberObjAll(passOutObj, &orderList);
    if (ret == SSOS_DEF_FAIL)
    {
        PTREE_ERR("Output: %d object error!", passOutObj->base.info->port);
        return ret;
    }
    SSOS_LIST_FOR_EACH_ENTRY(pos, &orderList, orderList)
    {
        if (!pos->memberObj)
        {
            PTREE_ERR("Member is null!");
            return SSOS_DEF_FAIL;
        }
        if (pos->memberObj->objStartCnt >= pos->memberObj->objUseCnt
            || pos->memberObj->objBindCnt >= pos->memberObj->objUseCnt
            || pos->memberObj->objInitCnt >= pos->memberObj->objUseCnt)
        {
            PTREE_ERR("[DELAY-PASS-ERR] : current section: %s should stop and unbind and deinit delay-pass first",
                      passOutObj->base.thisMod->info->sectionName);
            return SSOS_DEF_FAIL;
        }
    }
    return _PTREE_MOD_PASS_DoDestroyDelayPass(passOutObj, &orderList);
}

int PTREE_MOD_PASS_InitDelayPass(PTREE_MOD_OutObj_t *modOut, unsigned int rootIdx)
{
    int                      ret        = 0;
    PTREE_MOD_PASS_OutObj_t *passOutObj = CONTAINER_OF(modOut, PTREE_MOD_PASS_OutObj_t, base);
    SSOS_LIST_HEAD(orderList);

    ret = _PTREE_MOD_PASS_CheckStateIdx(passOutObj, rootIdx, E_PTREE_MOD_PASS_RESOURCE_INIT);
    if (ret != SSOS_DEF_OK)
    {
        PTREE_ERR("Output: %d no resource!", passOutObj->base.info->port);
        return ret;
    }
    ret = _PTREE_MOD_PASS_GetMemberObjIdx(passOutObj, &orderList, rootIdx);
    if (ret != SSOS_DEF_OK)
    {
        PTREE_ERR("Output: %d object error!", passOutObj->base.info->port);
        return ret;
    }
    ret = _PTREE_MOD_PASS_DoInitDelayPass(&orderList);
    if (ret == SSOS_DEF_OK)
    {
        _PTREE_MOD_PASS_MarkStateIdx(passOutObj, rootIdx, E_PTREE_MOD_PASS_RESOURCE_INIT);
    }
    return ret;
}

int PTREE_MOD_PASS_DeinitDelayPass(PTREE_MOD_OutObj_t *modOut, unsigned int rootIdx)
{
    int                      ret        = 0;
    PTREE_MOD_PASS_OutObj_t *passOutObj = CONTAINER_OF(modOut, PTREE_MOD_PASS_OutObj_t, base);
    SSOS_LIST_HEAD(orderList);

    ret = _PTREE_MOD_PASS_GetMemberObjIdx(passOutObj, &orderList, rootIdx);
    if (ret != SSOS_DEF_OK)
    {
        PTREE_ERR("Output: %d object error!", passOutObj->base.info->port);
        return ret;
    }
    ret = _PTREE_MOD_PASS_DoDeinitDelayPass(&orderList);
    if (ret == SSOS_DEF_OK)
    {
        _PTREE_MOD_PASS_UnmarkStateIdx(passOutObj, rootIdx, E_PTREE_MOD_PASS_RESOURCE_INIT);
    }
    return ret;
}

int PTREE_MOD_PASS_BindDelayPass(PTREE_MOD_OutObj_t *modOut, unsigned int rootIdx)
{
    int                      ret        = 0;
    PTREE_MOD_PASS_OutObj_t *passOutObj = CONTAINER_OF(modOut, PTREE_MOD_PASS_OutObj_t, base);
    SSOS_LIST_HEAD(orderList);

    ret = _PTREE_MOD_PASS_CheckStateIdx(passOutObj, rootIdx, E_PTREE_MOD_PASS_RESOURCE_BIND);
    if (ret != SSOS_DEF_OK)
    {
        return ret;
    }
    ret = _PTREE_MOD_PASS_GetMemberObjIdx(passOutObj, &orderList, rootIdx);
    if (ret != SSOS_DEF_OK)
    {
        PTREE_ERR("Output: %d object error!", passOutObj->base.info->port);
        return ret;
    }
    ret = _PTREE_MOD_PASS_DoBindDelayPass(&orderList);
    if (ret == SSOS_DEF_OK)
    {
        _PTREE_MOD_PASS_MarkStateIdx(passOutObj, rootIdx, E_PTREE_MOD_PASS_RESOURCE_BIND);
    }
    return ret;
}

int PTREE_MOD_PASS_UnbindDelayPass(PTREE_MOD_OutObj_t *modOut, unsigned int rootIdx)
{
    int                      ret        = 0;
    PTREE_MOD_PASS_OutObj_t *passOutObj = CONTAINER_OF(modOut, PTREE_MOD_PASS_OutObj_t, base);
    SSOS_LIST_HEAD(orderList);

    ret = _PTREE_MOD_PASS_GetMemberObjIdx(passOutObj, &orderList, rootIdx);
    if (ret != SSOS_DEF_OK)
    {
        PTREE_ERR("Output: %d object error!", passOutObj->base.info->port);
        return ret;
    }
    ret = _PTREE_MOD_PASS_DoUnbindDelayPass(&orderList);
    if (ret == SSOS_DEF_OK)
    {
        _PTREE_MOD_PASS_UnmarkStateIdx(passOutObj, rootIdx, E_PTREE_MOD_PASS_RESOURCE_BIND);
    }
    return ret;
}

int PTREE_MOD_PASS_StartDelayPass(PTREE_MOD_OutObj_t *modOut, unsigned int rootIdx)
{
    int                      ret        = 0;
    PTREE_MOD_PASS_OutObj_t *passOutObj = CONTAINER_OF(modOut, PTREE_MOD_PASS_OutObj_t, base);
    PTREE_MOD_PASS_Obj_t *   passObj    = CONTAINER_OF(passOutObj->base.thisMod, PTREE_MOD_PASS_Obj_t, base);
    SSOS_LIST_HEAD(orderList);

    ret = _PTREE_MOD_PASS_CheckStateIdx(passOutObj, rootIdx, E_PTREE_MOD_PASS_RESOURCE_START);
    if (ret != SSOS_DEF_OK)
    {
        return ret;
    }
    ret = _PTREE_MOD_PASS_GetMemberObjIdx(passOutObj, &orderList, rootIdx);
    if (ret != SSOS_DEF_OK)
    {
        PTREE_ERR("Output: %d object error!", passOutObj->base.info->port);
        return ret;
    }
    ret = _PTREE_MOD_PASS_DoStartDelayPass(&orderList, passObj->memberHash);
    if (ret == SSOS_DEF_OK)
    {
        _PTREE_MOD_PASS_MarkStateIdx(passOutObj, rootIdx, E_PTREE_MOD_PASS_RESOURCE_START);
    }
    return ret;
}

int PTREE_MOD_PASS_StopDelayPass(PTREE_MOD_OutObj_t *modOut, unsigned int rootIdx)
{
    int                      ret        = 0;
    PTREE_MOD_PASS_OutObj_t *passOutObj = CONTAINER_OF(modOut, PTREE_MOD_PASS_OutObj_t, base);
    PTREE_MOD_PASS_Obj_t *   passObj    = CONTAINER_OF(passOutObj->base.thisMod, PTREE_MOD_PASS_Obj_t, base);
    SSOS_LIST_HEAD(orderList);

    ret = _PTREE_MOD_PASS_GetMemberObjIdx(passOutObj, &orderList, rootIdx);
    if (ret != SSOS_DEF_OK)
    {
        PTREE_ERR("Output: %d object error!", passOutObj->base.info->port);
        return ret;
    }
    ret = _PTREE_MOD_PASS_DoStopDelayPass(&orderList, passObj->memberHash);
    if (ret == SSOS_DEF_OK)
    {
        _PTREE_MOD_PASS_UnmarkStateIdx(passOutObj, rootIdx, E_PTREE_MOD_PASS_RESOURCE_START);
    }
    return ret;
}

PTREE_MOD_Obj_t *PTREE_MOD_PASS_New(PARENA_Tag_t *tag)
{
    PTREE_MOD_PASS_Obj_t *passMod = NULL;

    passMod = (PTREE_MOD_PASS_Obj_t *)SSOS_MEM_Alloc(sizeof(PTREE_MOD_PASS_Obj_t));
    if (!passMod)
    {
        PTREE_ERR("Alloc err");
        return NULL;
    }
    memset(passMod, 0, sizeof(PTREE_MOD_PASS_Obj_t));
    SSOS_HASH_INIT(passMod->memberHash);
    if (SSOS_DEF_OK != PTREE_MOD_ObjInit(&passMod->base, &G_PTREE_MOD_PASS_OPS, tag))
    {
        SSOS_MEM_Free(passMod);
        return NULL;
    }
    PTREE_MOD_ObjRegister(&passMod->base, &G_PTREE_MOD_PASS_HOOK);
    return &passMod->base;
}
PTREE_MAKER_MOD_INIT(PASS, PTREE_MOD_PASS_New);
