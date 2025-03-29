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

#ifndef __PTREE_DB_H__
#define __PTREE_DB_H__

#define DB_ROOT_LOOP_MAX   64
#define DB_ROOT_LOOP_START (0)
#define DB_ROOT_LOOP_END   (DB_ROOT_LOOP_MAX)

typedef void *PTREE_DB_Obj_t;

typedef struct PTREE_DB_RootInfo_s
{
    char          entrySection[32];
    unsigned int  chipId;
    unsigned char bDelay;
} PTREE_DB_RootInfo_t;

typedef struct PTREE_DB_Opt_s
{
    void *(*createIns)(const char *file);
    int (*destroyIns)(void *ins);
    int (*getRoot)(void *ins, int loopId, PTREE_DB_RootInfo_t *info);
    void *(*createObj)(void *ins, const char *key);
    int (*destroyObj)(void *secObj);
    void *(*processKey)(void *secObj, const char *key);
    int (*invalidKey)(void *keyObj);
    int (*getInt)(void *keyObj, const char *key);
    long long (*getLongLong)(void *keyObj, const char *key);
    const char *(*getStr)(void *keyObj, const char *key);
} PTREE_DB_Opt_t;

void *         PTREE_DB_Init(const char *fileIns, const PTREE_DB_Opt_t *opt);
int            PTREE_DB_Deinit(void *handle);
int            PTREE_DB_GetRoot(void *handle, int loopId, PTREE_DB_RootInfo_t *info);
PTREE_DB_Obj_t PTREE_DB_CreateObj(void *handle, const char *section);
int            PTREE_DB_DestroyObj(PTREE_DB_Obj_t obj);
int            PTREE_DB_ProcessKey(PTREE_DB_Obj_t obj, const char *key);
int            PTREE_DB_ProcessBack(PTREE_DB_Obj_t obj);
int            PTREE_DB_ProcessEnd(PTREE_DB_Obj_t obj);
int            PTREE_DB_ProcessIn(PTREE_DB_Obj_t obj, int loopId);
int            PTREE_DB_ProcessOut(PTREE_DB_Obj_t obj, int loopId);
int            PTREE_DB_GetInt(PTREE_DB_Obj_t obj, const char *key);
long long      PTREE_DB_GetLongLong(PTREE_DB_Obj_t obj, const char *key);
const char *   PTREE_DB_GetStr(PTREE_DB_Obj_t obj, const char *key);

#endif //__PTREE_DB_H__
