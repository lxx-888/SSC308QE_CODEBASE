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
#include "ssos_mem.h"
#include "iniparser.h"
#include "ptree_log.h"
#include "ptree_db.h"
#include "ptree_db_ini.h"

#define DB_DEBUG           0

typedef struct PTREE_DB_INI_Ins_s
{
    dictionary *         iniDictionary;
    int                  rootInfoCnt;
    PTREE_DB_RootInfo_t *rootInfo;
} PTREE_DB_INI_Ins_t;

typedef struct PTREE_DB_INI_SecObj_s
{
    char *              secName;
    PTREE_DB_INI_Ins_t *iniIns;
} PTREE_DB_INI_SecObj_t;

typedef struct PTREE_DB_INI_KeyObj_s
{
    PTREE_DB_INI_SecObj_t *secObj;
    const char *           subSec;
} PTREE_DB_INI_KeyObj_t;

static void *_PTREE_DB_INI_CreateIns(const char *file)
{
    int                 i = 0;
    char                sec[64];
    const char *        blockName = NULL;
    PTREE_DB_INI_Ins_t *ins       = NULL;

    ins = (PTREE_DB_INI_Ins_t *)SSOS_MEM_Alloc(sizeof(PTREE_DB_INI_Ins_t));
    if (!ins)
    {
        PTREE_ERR("Alloc buf error!");
        return NULL;
    }
    ins->iniDictionary = iniparser_load(file);
    if (!ins->iniDictionary)
    {
        PTREE_ERR("Get file error !");
        SSOS_MEM_Free(ins);
        return NULL;
    }
    ins->rootInfoCnt = iniparser_getint(ins->iniDictionary, "ROOT:COUNT", 0);
    if (ins->rootInfoCnt)
    {
        ins->rootInfo = (PTREE_DB_RootInfo_t *)SSOS_MEM_Alloc(sizeof(PTREE_DB_RootInfo_t) * ins->rootInfoCnt);
        if (!ins->rootInfo)
        {
            PTREE_ERR("Buf error!");
            iniparser_freedict(ins->iniDictionary);
            SSOS_MEM_Free(ins);
            return NULL;
        }
        for (i = 0; i < ins->rootInfoCnt; i++)
        {
            snprintf(sec, 64, "ROOT:NAME_%d", i);
            blockName = iniparser_getstring(ins->iniDictionary, sec, NULL);
            if (!blockName)
            {
                PTREE_ERR("Get Block name error in load_file!");
                SSOS_MEM_Free(ins->rootInfo);
                iniparser_freedict(ins->iniDictionary);
                SSOS_MEM_Free(ins);
                return NULL;
            }
            snprintf(ins->rootInfo[i].entrySection, 32, "%s", blockName);
            snprintf(sec, 64, "ROOT:NAME_%d_CHIP_ID", i);
            ins->rootInfo[i].chipId = iniparser_getunsignedint(ins->iniDictionary, sec, 0);
            snprintf(sec, 64, "ROOT:NAME_%d_DELAY", i);
            ins->rootInfo[i].bDelay = (unsigned char)iniparser_getunsignedint(ins->iniDictionary, sec, 0);
#if DB_DEBUG
            PTREE_DBG("Load file root sec: %s, chip %d, b_delay %d", ins->root_info[i].entry_section,
                ins->root_info[i].chip_id, ins->root_info[i].b_delay);
#endif
        }
    }
    return (void *)ins;
}
static int _PTREE_DB_INI_DestroyIns(void *ins)
{
    PTREE_DB_INI_Ins_t *iniIns = (PTREE_DB_INI_Ins_t *)ins;

    if (!iniIns)
    {
        PTREE_ERR("NULL of ins");
        return -1;
    }
    if (!iniIns->iniDictionary)
    {
        PTREE_ERR("Iniparser is NULL!");
        return -1;
    }
    iniparser_freedict(iniIns->iniDictionary);
    return 0;
}
static int _PTREE_DB_INI_GetRoot(void *ins, int loopId, PTREE_DB_RootInfo_t *info)
{
    PTREE_DB_INI_Ins_t *iniIns = (PTREE_DB_INI_Ins_t *)ins;

    if (!iniIns)
    {
        PTREE_ERR("NULL of ins");
        return -1;
    }
    if (loopId >= iniIns->rootInfoCnt)
    {
        PTREE_ERR("Root block cnt is 0");
        return -1;
    }
    if (!iniIns->rootInfo)
    {
        PTREE_ERR("Root info is null");
        return -1;
    }
    memcpy(info, &iniIns->rootInfo[loopId], sizeof(PTREE_DB_RootInfo_t));
#if DB_DEBUG
    PTREE_DBG("Get root sec: %s, chip %d, b_delay %d", info->entry_section, info->chip_id, info->b_delay);
#endif
    return (loopId + 1 == iniIns->rootInfoCnt) ? DB_ROOT_LOOP_MAX : (loopId + 1);
}

static void *_PTREE_DB_INI_CreateObj(void *ins, const char *key)
{
    PTREE_DB_INI_SecObj_t *secObj = NULL;

    secObj = (PTREE_DB_INI_SecObj_t *)SSOS_MEM_Alloc(sizeof(PTREE_DB_INI_SecObj_t));
    if (!secObj)
    {
        PTREE_ERR("Alloc error!");
        return NULL;
    }
    secObj->secName = SSOS_MEM_Alloc(strlen(key) + 1);
    if (!secObj->secName)
    {
        PTREE_ERR("Alloc error!");
        SSOS_MEM_Free(secObj);
        return NULL;
    }
    strcpy(secObj->secName, key);
    secObj->iniIns = (PTREE_DB_INI_Ins_t *)ins;
    return (void *)secObj;
}
static int _PTREE_DB_INI_DestroyObj(void *secObj)
{
    PTREE_DB_INI_SecObj_t *iniSecObj = (PTREE_DB_INI_SecObj_t *)secObj;
    if (!iniSecObj || !iniSecObj->secName)
    {
        PTREE_ERR("NULL pointer!");
        return -1;
    }
    SSOS_MEM_Free(iniSecObj->secName);
    SSOS_MEM_Free(iniSecObj);
    return 0;
}
static void *_PTREE_DB_INI_ProcessKey(void *secObj, const char *key)
{
    char                   sec[64];
    const char *           subSec    = NULL;
    PTREE_DB_INI_KeyObj_t *iniKeyObj = NULL;
    PTREE_DB_INI_SecObj_t *iniSecObj = (PTREE_DB_INI_SecObj_t *)secObj;

    if (!iniSecObj || !iniSecObj->iniIns || !iniSecObj->iniIns->iniDictionary)
    {
        PTREE_ERR("Obj error!");
        return NULL;
    }
    snprintf(sec, 64, "%s:%s", iniSecObj->secName, key);
    subSec = iniparser_getstring(iniSecObj->iniIns->iniDictionary, sec, NULL);
    if (!subSec || strlen(subSec) >= 32)
    {
        PTREE_ERR("Process key %s error!", key);
        return NULL;
    }
    iniKeyObj = (PTREE_DB_INI_KeyObj_t *)SSOS_MEM_Alloc(sizeof(PTREE_DB_INI_KeyObj_t));
    if (!iniKeyObj)
    {
        PTREE_ERR("Alloc error!");
        return NULL;
    }
    iniKeyObj->secObj = iniSecObj;
    iniKeyObj->subSec = subSec;
#if DB_DEBUG
    PTREE_DBG("Process key end sec %s", sec);
#endif
    return (void *)iniKeyObj;
}
static int _PTREE_DB_INI_InvalidKey(void *keyObj)
{
    PTREE_DB_INI_KeyObj_t *iniKeyObj = (PTREE_DB_INI_KeyObj_t *)keyObj;

    if (!iniKeyObj || !iniKeyObj->secObj || !iniKeyObj->subSec)
    {
        PTREE_ERR("Obj error!");
        return -1;
    }
#if DB_DEBUG
    PTREE_DBG("Invalid key end sec %s", iniKeyObj->subSec);
#endif
    SSOS_MEM_Free(iniKeyObj);
    return 0;
}
static int _PTREE_DB_INI_GetInt(void *keyObj, const char *key)
{
    char                   sec[64];
    int                    retVal    = 0;
    PTREE_DB_INI_KeyObj_t *iniKeyObj = (PTREE_DB_INI_KeyObj_t *)keyObj;

    if (!iniKeyObj || !iniKeyObj->secObj || !iniKeyObj->subSec)
    {
        PTREE_ERR("Obj error!");
        return -1;
    }
    if (!iniKeyObj->secObj->iniIns || !iniKeyObj->secObj->iniIns->iniDictionary)
    {
        PTREE_ERR("Obj dictionary error!");
        return -1;
    }
    snprintf(sec, 64, "%s:%s", iniKeyObj->subSec, key);
    retVal = iniparser_getint(iniKeyObj->secObj->iniIns->iniDictionary, sec, 0);
#if DB_DEBUG
    PTREE_DBG("Get int val %s=%d", key, retVal);
#endif
    return retVal;
}
static long long _PTREE_DB_INI_GetLongLong(void *keyObj, const char *key)
{
    char                   sec[64];
    const char *           retStr    = NULL;
    PTREE_DB_INI_KeyObj_t *iniKeyObj = (PTREE_DB_INI_KeyObj_t *)keyObj;

    if (!iniKeyObj || !iniKeyObj->secObj || !iniKeyObj->subSec)
    {
        PTREE_ERR("Obj error!");
        return -1;
    }
    if (!!iniKeyObj->secObj->iniIns || !iniKeyObj->secObj->iniIns->iniDictionary)
    {
        PTREE_ERR("Obj dictionary error!");
        return -1;
    }
    snprintf(sec, 64, "%s:%s", iniKeyObj->subSec, key);
    retStr = iniparser_getstring(iniKeyObj->secObj->iniIns->iniDictionary, sec, "");
#if DB_DEBUG
    PTREE_DBG("Get long val %s=%s", key, retStr);
#endif
    return SSOS_IO_Atol(retStr);
}
static const char *_PTREE_DB_INI_GetStr(void *keyObj, const char *key)
{
    char                   sec[64];
    const char *           retStr    = NULL;
    PTREE_DB_INI_KeyObj_t *iniKeyObj = (PTREE_DB_INI_KeyObj_t *)keyObj;

    if (!iniKeyObj || !iniKeyObj->secObj || !iniKeyObj->subSec)
    {
        PTREE_ERR("Obj error!");
        return NULL;
    }
    if (!!iniKeyObj->secObj->iniIns || !iniKeyObj->secObj->iniIns->iniDictionary)
    {
        PTREE_ERR("Obj dictionary error!");
        return NULL;
    }
    snprintf(sec, 64, "%s:%s", iniKeyObj->subSec, key);
    retStr = iniparser_getstring(iniKeyObj->secObj->iniIns->iniDictionary, sec, "");
#if DB_DEBUG
    PTREE_DBG("Get str val %s=%s", key, retStr);
#endif
    return retStr;
}
void *PTREE_DB_INI_Init(const char *file)
{
    PTREE_DB_Opt_t opt;

    memset(&opt, 0, sizeof(PTREE_DB_Opt_t));
    opt.createIns  = _PTREE_DB_INI_CreateIns;
    opt.destroyIns = _PTREE_DB_INI_DestroyIns;
    opt.getRoot    = _PTREE_DB_INI_GetRoot;
    opt.createObj  = _PTREE_DB_INI_CreateObj;
    opt.destroyObj = _PTREE_DB_INI_DestroyObj;
    opt.processKey = _PTREE_DB_INI_ProcessKey;
    opt.invalidKey = _PTREE_DB_INI_InvalidKey;
    opt.getInt     = _PTREE_DB_INI_GetInt;
    opt.getLong    = _PTREE_DB_INI_GetLongLong;
    opt.getStr     = _PTREE_DB_INI_GetStr;
    return PTREE_DB_Init(file, &opt);
}
int PTREE_DB_INI_Deinit(void *ins)
{
    return PTREE_DB_Deinit(ins);
}
