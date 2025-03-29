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
#include "cjson.h"
#include "ssos_mem.h"
#include "ptree_db.h"
#include "ptree_db_json.h"
#include "ptree_log.h"
#include "ssos_io.h"
#include "ssos_mem.h"
#include "ssos_list.h"

#define PTREE_DB_JSON_DEBUG (0)

typedef struct PTREE_DB_JSON_Ins_s
{
    char * srcBuf;
    cJSON *cJsonObj;
} PTREE_DB_JSON_Ins_t;

static void *_PTREE_DB_JSON_CreateIns(const char *file)
{
    SSOS_IO_File_t       fd;
    off_t                len     = 0;
    unsigned long        readLen = 0;
    PTREE_DB_JSON_Ins_t *ins     = NULL;

    fd = SSOS_IO_FileOpen(file, SSOS_IO_O_RDONLY, 0644);
    if (!fd)
    {
        PTREE_ERR("not exist file:%s", file);
        return NULL;
    }

    len = SSOS_IO_FileSeek(fd, 0, SSOS_IO_SEEK_END);
    if (len <= 0)
    {
        PTREE_ERR("file size error:%ld", len);
        SSOS_IO_FileClose(fd);
        return NULL;
    }

    SSOS_IO_FileSeek(fd, 0, SSOS_IO_SEEK_SET);
    ins = (PTREE_DB_JSON_Ins_t *)SSOS_MEM_Alloc(sizeof(PTREE_DB_JSON_Ins_t));
    if (!ins)
    {
        PTREE_ERR("Alloc json db ins err");
        SSOS_IO_FileClose(fd);
        return NULL;
    }
    ins->srcBuf = (char *)SSOS_MEM_Alloc(len + 1);
    if (ins->srcBuf == NULL)
    {
        PTREE_ERR("malloc %ld fail", len + 1);
        SSOS_MEM_Free(ins);
        SSOS_IO_FileClose(fd);
        return NULL;
    }

    readLen = SSOS_IO_FileRead(fd, ins->srcBuf, len);
    if (readLen < len)
    {
        PTREE_ERR("file read fail expect:%ld, but:%ld", len, readLen);
        SSOS_MEM_Free(ins->srcBuf);
        SSOS_MEM_Free(ins);
        SSOS_IO_FileClose(fd);
        return NULL;
    }
    ins->srcBuf[len] = '\0';
    SSOS_IO_FileClose(fd);
    ins->cJsonObj = cJSON_Parse(ins->srcBuf);
    if (!ins->cJsonObj)
    {
        PTREE_ERR("Parse err");
        SSOS_MEM_Free(ins->srcBuf);
        SSOS_MEM_Free(ins);
        return NULL;
    }
    return (void *)ins;
}
static int _PTREE_DB_JSON_DestroyIns(void *ins)
{
    PTREE_DB_JSON_Ins_t *jsonIns = (PTREE_DB_JSON_Ins_t *)ins;
    if (!jsonIns)
    {
        PTREE_ERR("Null pointer!");
        return -1;
    }
    cJSON_Delete(jsonIns->cJsonObj);
    SSOS_MEM_Free(jsonIns->srcBuf);
    SSOS_MEM_Free(jsonIns);
    return 0;
}
static int _PTREE_DB_JSON_GetRoot(void *ins, int loopId, PTREE_DB_RootInfo_t *info)
{
    int                  ret     = -1;
    int                  size    = 0;
    PTREE_DB_JSON_Ins_t *jsonIns = (PTREE_DB_JSON_Ins_t *)ins;
    cJSON *              obj     = NULL;
    cJSON *              root    = NULL;
    cJSON *              item    = NULL;
    cJSON *              key     = NULL;

    if (!jsonIns)
    {
        PTREE_ERR("Instance err!");
        goto err_ret;
    }
    if (loopId < DB_ROOT_LOOP_START || !info)
    {
        PTREE_ERR("Param err");
        goto err_ret;
    }
    obj  = jsonIns->cJsonObj;
    root = cJSON_GetObjectItem(obj, "ROOT");
    if (!root || !cJSON_IsArray(root))
    {
        PTREE_ERR("ROOT is not array");
        goto err_ret;
    }

    size = cJSON_GetArraySize(root);
    if (size <= 0 || loopId >= size)
    {
        PTREE_ERR("Out of range [%d/%d]", loopId, size);
        goto err_ret;
    }

    item = cJSON_GetArrayItem(root, loopId);
    if (!item || !cJSON_IsObject(item))
    {
        PTREE_ERR("root[%d] is not obj", loopId);
        goto err_ret;
    }

    key = cJSON_GetObjectItem(item, "NAME");
    if (key)
    {
        if (cJSON_IsString(key))
        {
            strncpy(info->entrySection, cJSON_GetStringValue(key),
                    sizeof(info->entrySection) / sizeof(info->entrySection[0]) - 1);
        }
        else
        {
            PTREE_ERR("NAME type err");
        }
    }
    else
    {
        PTREE_ERR("NAME is not found");
    }
    key = cJSON_GetObjectItem(item, "DELAY");
    if (key)
    {
        if (cJSON_IsString(key))
        {
            info->bDelay = SSOS_IO_Atoi(cJSON_GetStringValue(key));
        }
        else
        {
            PTREE_ERR("DELAY type err");
        }
    }
    else
    {
        PTREE_ERR("DELAY is not found");
    }
    key = cJSON_GetObjectItem(item, "CHIP");
    if (key)
    {
        if (cJSON_IsString(key))
        {
            info->chipId = SSOS_IO_Atoi(cJSON_GetStringValue(key));
        }
        else
        {
            PTREE_ERR("CHIP type err");
        }
    }
    else
    {
        PTREE_ERR("CHIP is not found");
    }

    ret = loopId + 1;
    if (ret == size)
    {
#if PTREE_DB_JSON_DEBUG
        PTREE_DBG("Loop end");
#endif
        ret = DB_ROOT_LOOP_END;
    }
err_ret:
    return ret;
}
static void *_PTREE_DB_JSON_CreateObj(void *ins, const char *key)
{
    PTREE_DB_JSON_Ins_t *jsonIns = (PTREE_DB_JSON_Ins_t *)ins;
    cJSON *              newObj  = NULL;
    newObj                       = cJSON_GetObjectItem(jsonIns->cJsonObj, key);
    if (!newObj || !cJSON_IsObject(newObj))
    {
        PTREE_ERR("%s is not found", key);
        return NULL;
    }
    return (void *)newObj;
}
static int _PTREE_DB_JSON_DestroyObj(void *secObj)
{
    (void)secObj;
    return 0;
}
static void *_PTREE_DB_JSON_ProcessKey(void *secObj, const char *key)
{
    cJSON *newObj = NULL;

    if (!secObj || !key)
    {
        PTREE_ERR("Param err");
        return NULL;
    }
    newObj = cJSON_GetObjectItem((cJSON *)secObj, key);
    if (!newObj)
    {
        PTREE_ERR("%s is not found", key);
        return NULL;
    }
    return (void *)newObj;
}
static int _PTREE_DB_JSON_InvalidKey(void *keyObj)
{
    (void)keyObj;
    return 0;
}
static int _PTREE_DB_JSON_GetInt(void *keyObj, const char *key)
{
    cJSON *strObj = NULL;
    char * str    = NULL;
    int    val    = 0;

    if (!keyObj || !key)
    {
        PTREE_ERR("Param err");
        return -1;
    }
    strObj = cJSON_GetObjectItem((cJSON *)keyObj, key);
    if (!strObj)
    {
        PTREE_ERR("%s is not found", key);
        return -1;
    }
    str = cJSON_GetStringValue(strObj);
    if (!str)
    {
        PTREE_ERR("%s is not found", key);
        return -1;
    }
    val = SSOS_IO_Atoi(str);
#if PTREE_DB_JSON_DEBUG
    PTREE_DBG("%s = %d", key, val);
#endif
    return val;
}
static long long _PTREE_DB_JSON_GetLongLong(void *keyObj, const char *key)
{
    cJSON *strObj = NULL;
    char * str    = NULL;
    long   val    = 0;

    if (!keyObj || !key)
    {
        PTREE_ERR("Param err");
        return -1;
    }
    strObj = cJSON_GetObjectItem((cJSON *)keyObj, key);
    if (!strObj)
    {
        PTREE_ERR("%s is not found", key);
        return -1;
    }
    str = cJSON_GetStringValue(strObj);
    if (!str)
    {
        PTREE_ERR("%s is not found", key);
        return -1;
    }
    val = SSOS_IO_Atoll(str);
#if PTREE_DB_JSON_DEBUG
    PTREE_DBG("%s = %ld", key, val);
#endif
    return val;
}
static const char *_PTREE_DB_JSON_GetStr(void *keyObj, const char *key)
{
    cJSON *strObj = NULL;
    char * str    = NULL;

    if (!keyObj || !key)
    {
        PTREE_ERR("Param err");
        return NULL;
    }

    strObj = cJSON_GetObjectItem((cJSON *)keyObj, key);
    if (!strObj)
    {
        PTREE_ERR("%s is not found", key);
        return NULL;
    }
    str = cJSON_GetStringValue(strObj);
    if (!str)
    {
        PTREE_ERR("%s is not found", key);
        return NULL;
    }
#if PTREE_DB_JSON_DEBUG
    PTREE_DBG("%s = %s", key, str);
#endif
    return str;
}
void *PTREE_DB_JSON_Init(const char *file)
{
    PTREE_DB_Opt_t opt;

    memset(&opt, 0, sizeof(PTREE_DB_Opt_t));
    opt.createIns   = _PTREE_DB_JSON_CreateIns;
    opt.destroyIns  = _PTREE_DB_JSON_DestroyIns;
    opt.getRoot     = _PTREE_DB_JSON_GetRoot;
    opt.createObj   = _PTREE_DB_JSON_CreateObj;
    opt.destroyObj  = _PTREE_DB_JSON_DestroyObj;
    opt.processKey  = _PTREE_DB_JSON_ProcessKey;
    opt.invalidKey  = _PTREE_DB_JSON_InvalidKey;
    opt.getInt      = _PTREE_DB_JSON_GetInt;
    opt.getLongLong = _PTREE_DB_JSON_GetLongLong;
    opt.getStr      = _PTREE_DB_JSON_GetStr;
    return PTREE_DB_Init(file, &opt);
}
int PTREE_DB_JSON_Deinit(void *ins)
{
    return PTREE_DB_Deinit(ins);
}
