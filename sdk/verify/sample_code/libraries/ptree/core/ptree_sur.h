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

#ifndef __PTREE_SUR_H__
#define __PTREE_SUR_H__

#include "parena.h"
#include "ptree_obj.h"
#include "ptree_db.h"

typedef struct PTREE_SUR_Info_s    PTREE_SUR_Info_t;
typedef struct PTREE_SUR_InInfo_s  PTREE_SUR_InInfo_t;
typedef struct PTREE_SUR_OutInfo_s PTREE_SUR_OutInfo_t;

typedef struct PTREE_SUR_Obj_s  PTREE_SUR_Obj_t;
typedef struct PTREE_SUR_Ops_s  PTREE_SUR_Ops_t;
typedef struct PTREE_SUR_Hook_s PTREE_SUR_Hook_t;

struct PTREE_SUR_Info_s
{
    char          sectionName[32];
    char          typeName[32];
    unsigned int  modId;
    unsigned int  devId;
    unsigned int  chnId;
    unsigned char inCnt;
    unsigned char outCnt;
    unsigned char preload;
    unsigned char initStage;
};
struct PTREE_SUR_InInfo_s
{
    unsigned char  port;
    unsigned short fps;
    char           prevSection[32];
    unsigned int   prevOutId;
    int            curTagOffset;
    int            prevTagOffset;
};
struct PTREE_SUR_OutInfo_s
{
    unsigned char  port;
    unsigned short fps;
    int            curTagOffset;
};

struct PTREE_SUR_Hook_s
{
    void (*destruct)(PTREE_SUR_Obj_t *sur);
    void (*free)(PTREE_SUR_Obj_t *sur);
};
struct PTREE_SUR_Ops_s
{
    PARENA_Tag_t *(*occupyInfo)(PTREE_SUR_Obj_t *sur, void *pArena);
    PARENA_Tag_t *(*occupyInInfo)(PTREE_SUR_Obj_t *sur, void *pArena);
    PARENA_Tag_t *(*occupyOutInfo)(PTREE_SUR_Obj_t *sur, void *pArena);
    int (*loadDb)(PTREE_SUR_Obj_t *sur, PTREE_SUR_Info_t *info, PTREE_DB_Obj_t db);
    int (*loadInDb)(PTREE_SUR_Obj_t *sur, PTREE_SUR_InInfo_t *info, PTREE_DB_Obj_t db);
    int (*loadOutDb)(PTREE_SUR_Obj_t *sur, PTREE_SUR_OutInfo_t *info, PTREE_DB_Obj_t db);
};
struct PTREE_SUR_Obj_s
{
    PTREE_OBJ_Obj_t         obj;
    const PTREE_SUR_Ops_t * ops;
    const PTREE_SUR_Hook_t *hook;
    /* Global enterance to let module access other section. */
    void *dbins;
};

int PTREE_SUR_Init(PTREE_SUR_Obj_t *sur, const PTREE_SUR_Ops_t *ops);

void PTREE_SUR_Register(PTREE_SUR_Obj_t *sur, const PTREE_SUR_Hook_t *hook);

PARENA_Tag_t *PTREE_SUR_LoadDb(PTREE_SUR_Obj_t *sur, const char *section, const char *type, PTREE_DB_Obj_t db,
                               void *pArena);

void PTREE_SUR_Del(PTREE_SUR_Obj_t *sur);

#endif /* ifndef __PTREE_SUR_H__ */
