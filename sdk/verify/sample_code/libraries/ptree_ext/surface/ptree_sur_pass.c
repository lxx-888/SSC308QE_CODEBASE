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
#include "parena.h"
#include "ptree_db.h"
#include "ssos_def.h"
#include "ssos_io.h"
#include "ssos_list.h"
#include "ptree_log.h"
#include "ssos_mem.h"
#include "ptree_enum.h"
#include "ptree_sur.h"
#include "ptree_maker.h"
#include "ptree_sur_sys.h"
#include "ptree_sur_pass.h"

typedef struct PTREE_SUR_PASS_Node_s
{
    struct SSOS_LIST_Head_s stHashEntry;
    PARENA_Tag_t *          tag;
} PTREE_SUR_PASS_Node_t;

static PARENA_Tag_t *_PTREE_SUR_PASS_OccupyInInfo(PTREE_SUR_Obj_t *sur, void *pArena);
static PARENA_Tag_t *_PTREE_SUR_PASS_OccupyOutInfo(PTREE_SUR_Obj_t *sur, void *pArena);
static int           _PTREE_SUR_PASS_LoadInDb(PTREE_SUR_Obj_t *sur, PTREE_SUR_InInfo_t *info, PTREE_DB_Obj_t db);
static int           _PTREE_SUR_PASS_LoadOutDb(PTREE_SUR_Obj_t *sur, PTREE_SUR_OutInfo_t *info, PTREE_DB_Obj_t db);
static void          _PTREE_SUR_PASS_Free(PTREE_SUR_Obj_t *sur);
static PTREE_SUR_PASS_Node_t *_PTREE_SUR_PASS_ImplementDb(PTREE_SUR_Obj_t *sur, const char *section, void *pArena,
                                                          struct SSOS_LIST_Head_s modHash[]);

static const PTREE_SUR_Ops_t G_PTREE_SUR_PASS_OPS = {
    .occupyInInfo  = _PTREE_SUR_PASS_OccupyInInfo,
    .occupyOutInfo = _PTREE_SUR_PASS_OccupyOutInfo,
    .loadInDb      = _PTREE_SUR_PASS_LoadInDb,
    .loadOutDb     = _PTREE_SUR_PASS_LoadOutDb,
};

static const PTREE_SUR_Hook_t G_PTREE_SUR_PASS_HOOK = {
    .free = _PTREE_SUR_PASS_Free,
};

static void _PTREE_SUR_PASS_Free(PTREE_SUR_Obj_t *sur)
{
    SSOS_MEM_Free(sur);
}

static PARENA_Tag_t *_PTREE_SUR_PASS_OccupyInInfo(PTREE_SUR_Obj_t *sur, void *pArena)
{
    (void)sur;
    return PARENA_GET(pArena, PTREE_SUR_PASS_InInfo_t, base, PTREE_SUR_InInfo_t);
}

static PARENA_Tag_t *_PTREE_SUR_PASS_OccupyOutInfo(PTREE_SUR_Obj_t *sur, void *pArena)
{
    (void)sur;
    return PARENA_GET(pArena, PTREE_SUR_PASS_OutInfo_t, base, PTREE_SUR_OutInfo_t);
}

static int _PTREE_SUR_PASS_LoadInDb(PTREE_SUR_Obj_t *sur, PTREE_SUR_InInfo_t *info, PTREE_DB_Obj_t db)
{
    PTREE_SUR_PASS_InInfo_t *passInInfo = CONTAINER_OF(info, PTREE_SUR_PASS_InInfo_t, base);
    PTREE_DB_Obj_t *         internalDb = NULL;
    const char *             modName    = NULL;
    char                     secName[32];

    if (!sur || !sur->dbins)
    {
        PTREE_ERR("Null pointer");
        return -1;
    }
    snprintf(secName, 32, "%s", PTREE_DB_GetStr(db, "TAIL"));
    internalDb = PTREE_DB_CreateObj(sur->dbins, secName);
    if (!internalDb)
    {
        PTREE_ERR("DB create error section: %s", secName);
        return -1;
    }
    modName = PTREE_DB_GetStr(internalDb, "MOD");
    if (!modName)
    {
        PTREE_DB_DestroyObj(internalDb);
        PTREE_ERR("DB get 'MOD' error sectio: %s", secName);
        return -1;
    }
    snprintf(passInInfo->modName, 32, "%s", modName);
    passInInfo->devId = PTREE_DB_GetInt(internalDb, "DEV");
    passInInfo->chnId = PTREE_DB_GetInt(internalDb, "CHN");
    PTREE_DB_DestroyObj(internalDb);
    passInInfo->secPortId = (unsigned int)PTREE_DB_GetInt(db, "TAIL_IN_ID");
    return SSOS_DEF_OK;
}

static int _PTREE_SUR_PASS_ClearHashData(struct SSOS_LIST_Head_s modHash[])
{
    PTREE_SUR_PASS_Node_t *pos = NULL, *posN = NULL;
    int                    i;
    for (i = 0; i < PTREE_PASS_MOD_HASH_SIZE; i++)
    {
        SSOS_LIST_FOR_EACH_ENTRY_SAFE(pos, posN, &modHash[i], stHashEntry)
        {
            SSOS_LIST_Del(&pos->stHashEntry);
            SSOS_MEM_Free(pos);
        }
    }
    return 0;
}

static int _PTREE_SUR_PASS_LoadOutDb(PTREE_SUR_Obj_t *sur, PTREE_SUR_OutInfo_t *info, PTREE_DB_Obj_t db)
{
    PTREE_SUR_PASS_OutInfo_t *passOutInfo = CONTAINER_OF(info, PTREE_SUR_PASS_OutInfo_t, base);
    unsigned int              rootCnt, i;
    int                       ret     = 0;
    PTREE_SUR_PASS_Node_t *   pstNode = NULL;
    void *                    pArena  = NULL;
    const char *              headSec = NULL;

    LIST_HEAD_ARRAY(modHash, PTREE_PASS_MOD_HASH_SIZE);

    ret = ARENA_MapUnused(&pArena, passOutInfo->arenaBuffer, 4096);
    if (ret != 0)
    {
        PTREE_ERR("Arena map error!");
        return ret;
    }
    rootCnt = (unsigned int)PTREE_DB_GetInt(db, "ROOT_CNT");
    if (rootCnt == (unsigned int)-1)
    {
        _PTREE_SUR_PASS_ClearHashData(modHash);
        ARENA_Unmap(pArena);
        PTREE_ERR("SUB_ROOT_CNT Get fail!");
        return -1;
    }
    if (rootCnt >= PARENA_ROOT_COUNT_MAX)
    {
        _PTREE_SUR_PASS_ClearHashData(modHash);
        ARENA_Unmap(pArena);
        PTREE_ERR("SUB_ROOT_CNT too much!");
        return -1;
    }
    passOutInfo->headIdx = PARENA_ROOT_COUNT_MAX;
    headSec              = PTREE_DB_GetStr(db, "HEAD");
    for (i = 0; i < rootCnt; i++)
    {
        char        tmpKey[32];
        const char *rootSec = NULL;
        snprintf(tmpKey, 32, "ROOT_%d", i);
        PTREE_DB_ProcessKey(db, tmpKey);
        rootSec = PTREE_DB_GetStr(db, "SEC_NAME");
        pstNode = _PTREE_SUR_PASS_ImplementDb(sur, rootSec, pArena, modHash);
        if (!pstNode)
        {
            _PTREE_SUR_PASS_ClearHashData(modHash);
            ARENA_Unmap(pArena);
            PTREE_DB_ProcessBack(db);
            PTREE_ERR("Implement sub root module error!");
            return -1;
        }
        passOutInfo->rootOffset[i] = ARENA_Offset(pArena, (void *)pstNode->tag);
        if (!strcmp(headSec, rootSec))
        {
            passOutInfo->headIdx = i;
        }
        PTREE_DB_ProcessBack(db);
    }
    ARENA_Unmap(pArena);
    passOutInfo->secPortId = (unsigned int)PTREE_DB_GetInt(db, "HEAD_OUT_ID");
    /* Mark -1 to indicate that the loop of root offset is end. */
    passOutInfo->rootOffset[i] = -1;
    _PTREE_SUR_PASS_ClearHashData(modHash);
    return SSOS_DEF_OK;
}

static inline int _PTREE_SUR_PASS_ModueDescHash(const char *modName, unsigned int u32DevId, unsigned u32ChnId)
{
    char str[64];

    snprintf(str, 64, "%s_%u_%u", modName, u32DevId, u32ChnId);
    return SSOS_HASH_Val(str, PTREE_PASS_MOD_HASH_SIZE);
}

static inline PTREE_SUR_PASS_Node_t *_PTREE_SUR_PASS_FindNodeInHash(struct SSOS_LIST_Head_s *hash, const char *modName,
                                                                    unsigned int dev, unsigned int chn)
{
    PTREE_SUR_PASS_Node_t *pstPos  = NULL;
    PTREE_SUR_Info_t *     pstInfo = NULL;

    SSOS_LIST_FOR_EACH_ENTRY(pstPos, hash, stHashEntry)
    {
        pstInfo = (PTREE_SUR_Info_t *)PARENA_USE_BASE(pstPos->tag);
        if (!strcmp(pstInfo->typeName, modName) && pstInfo->devId == dev && pstInfo->chnId == chn)
        {
            return pstPos;
        }
    }
    return NULL;
}

static PTREE_SUR_PASS_Node_t *_PTREE_SUR_PASS_ImplementDb(PTREE_SUR_Obj_t *sur, const char *section, void *arena,
                                                          struct SSOS_LIST_Head_s hash[])
{
    unsigned char          i           = 0;
    PTREE_DB_Obj_t         db          = NULL;
    PTREE_MAKER_Obj_t *    pstMaker    = NULL;
    PTREE_SUR_Obj_t *      pstSur      = NULL;
    PTREE_SUR_PASS_Node_t *pstNode     = NULL;
    PTREE_SUR_PASS_Node_t *pstPrevNode = NULL;
    PARENA_Tag_t *         pstTag      = NULL;

    const char * pModStr    = NULL;
    unsigned int u32HashVal = 0;
    unsigned int u32DevId   = 0;
    unsigned int u32ChnId   = 0;

    PTREE_SUR_Info_t *  pstInfo   = NULL;
    PTREE_SUR_InInfo_t *pstInInfo = NULL;
    PARENA_Tag_t *      inTag     = NULL;

    PTREE_SUR_Info_t *pstPrevInfo = NULL;

    if (!sur || !sur->dbins || !section || !arena || !hash)
    {
        PTREE_ERR("Null pointer or section error");
        return NULL;
    }

    db = PTREE_DB_CreateObj(sur->dbins, section);
    if (!db)
    {
        PTREE_ERR("DB create error section: %s", section);
        return NULL;
    }

    pModStr = PTREE_DB_GetStr(db, "MOD");
    if (!pModStr)
    {
        PTREE_DB_DestroyObj(db);
        PTREE_ERR("DB get 'MOD' error sectio: %s", section);
        return NULL;
    }
    u32DevId = PTREE_DB_GetInt(db, "DEV");
    u32ChnId = PTREE_DB_GetInt(db, "CHN");

    u32HashVal = _PTREE_SUR_PASS_ModueDescHash(pModStr, u32DevId, u32ChnId);
    pstNode    = _PTREE_SUR_PASS_FindNodeInHash(&hash[u32HashVal], pModStr, u32DevId, u32ChnId);
    if (pstNode)
    {
        PTREE_DB_DestroyObj(db);
        return pstNode;
    }

    pstMaker = PTREE_MAKER_Get(pModStr);
    if (!pstMaker)
    {
        PTREE_DB_DestroyObj(db);
        PTREE_ERR("Get %s maker err.", pModStr);
        return NULL;
    }

    pstSur = PTREE_MAKER_MakeSur(pstMaker);
    if (!pstSur)
    {
        PTREE_DB_DestroyObj(db);
        PTREE_ERR("Make %s sur err.", pModStr);
        return NULL;
    }
    pstSur->dbins = sur->dbins;

    pstTag = PTREE_SUR_LoadDb(pstSur, section, pModStr, db, arena);
    if (!pstTag)
    {
        PTREE_MAKER_DelSur(pstSur);
        PTREE_DB_DestroyObj(db);
        PTREE_ERR("Load db error.");
        return NULL;
    }
    if (pstTag->u32Magic != __PARENA_MAGIC_NUM__)
    {
        PTREE_MAKER_DelSur(pstSur);
        PTREE_DB_DestroyObj(db);
        PTREE_ERR("Tag u32Magic num error.");
        return NULL;
    }

    pstNode = (PTREE_SUR_PASS_Node_t *)SSOS_MEM_Alloc(sizeof(PTREE_SUR_PASS_Node_t));
    if (!pstNode)
    {
        PTREE_MAKER_DelSur(pstSur);
        PTREE_DB_DestroyObj(db);
        PTREE_ERR("Node malloc error, while implementing %s!", section);
        return NULL;
    }
    pstNode->tag = pstTag;
    pstInfo      = (PTREE_SUR_Info_t *)PARENA_USE_BASE(pstTag);

    PTREE_MAKER_DelSur(pstSur);
    PTREE_DB_DestroyObj(db);
    SSOS_LIST_AddTail(&pstNode->stHashEntry, &hash[u32HashVal]);
    if (pstInfo->inCnt)
    {
        inTag = PARENA_USE_NEXT(pstTag);
    }
    for (i = 0; i < pstInfo->inCnt; i++)
    {
        pstInInfo = (PTREE_SUR_InInfo_t *)PARENA_USE_BASE(inTag);
        if (!strlen(pstInInfo->prevSection))
        {
            continue;
        }
        pstPrevNode = _PTREE_SUR_PASS_ImplementDb(sur, pstInInfo->prevSection, arena, hash);
        if (!pstPrevNode)
        {
            PTREE_ERR("Prev node is null");
            goto ERROR_IMPLEMENT;
        }
        pstPrevInfo = (PTREE_SUR_Info_t *)PARENA_USE_BASE(pstPrevNode->tag);
        if (pstPrevInfo->outCnt <= pstInInfo->prevOutId)
        {
            PTREE_ERR("Prev output cnt %d <= loop id %d", pstPrevInfo->outCnt, pstInInfo->prevOutId);
            goto ERROR_IMPLEMENT;
        }
        pstInInfo->prevTagOffset = PARENA_RELATIVE_OFFSET(pstInInfo, pstPrevNode->tag);
        inTag                    = (PARENA_Tag_t *)(inTag->pData + inTag->u32Size);
    }
    return pstNode;
ERROR_IMPLEMENT:
    SSOS_LIST_Del(&pstNode->stHashEntry);
    SSOS_MEM_Free(pstNode);
    return NULL;
}

PTREE_SUR_Obj_t *PTREE_SUR_PASS_New(void)
{
    PTREE_SUR_Obj_t *sur = SSOS_MEM_Alloc(sizeof(PTREE_SUR_Obj_t));
    if (!sur)
    {
        PTREE_ERR("Alloc err");
        return NULL;
    }
    memset(sur, 0, sizeof(PTREE_SUR_Obj_t));
    if (SSOS_DEF_OK != PTREE_SUR_Init(sur, &G_PTREE_SUR_PASS_OPS))
    {
        SSOS_MEM_Free(sur);
        return NULL;
    }
    PTREE_SUR_Register(sur, &G_PTREE_SUR_PASS_HOOK);
    return sur;
}
PTREE_MAKER_SUR_INIT(PASS, PTREE_SUR_PASS_New);
