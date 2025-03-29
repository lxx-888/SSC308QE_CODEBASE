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

#ifndef __PTREE_SUR_SYS_H__
#define __PTREE_SUR_SYS_H__

#include "ptree_sur.h"
#include "mi_sys_datatype.h"
#include "ptree_enum.h"

typedef struct PTREE_SUR_SYS_Info_s    PTREE_SUR_SYS_Info_t;
typedef struct PTREE_SUR_SYS_InInfo_s  PTREE_SUR_SYS_InInfo_t;
typedef struct PTREE_SUR_SYS_OutInfo_s PTREE_SUR_SYS_OutInfo_t;

typedef struct PTREE_SUR_SYS_Hook_s PTREE_SUR_SYS_Hook_t;
typedef struct PTREE_SUR_SYS_Ops_s  PTREE_SUR_SYS_Ops_t;
typedef struct PTREE_SUR_SYS_Obj_s  PTREE_SUR_SYS_Obj_t;

struct PTREE_SUR_SYS_Info_s
{
    PTREE_SUR_Info_t base;
};
struct PTREE_SUR_SYS_InInfo_s
{
    PTREE_SUR_InInfo_t base;
    unsigned int       bindType;
    unsigned int       bindParam;
};
struct PTREE_SUR_SYS_OutInfo_s
{
    PTREE_SUR_OutInfo_t base;
    unsigned char       userFrc;
    unsigned char       depthEn;
    unsigned int        depthUser;
    unsigned int        depthTotal;
    unsigned char       extEn;
    unsigned int        extHAlign;
    unsigned int        extVAlign;
    unsigned int        extChromaAlign;
    unsigned int        extCompressAlign;
    unsigned int        extExtraSize;
    unsigned int        extClearPadding;
};

struct PTREE_SUR_SYS_Hook_s
{
    void (*destruct)(PTREE_SUR_SYS_Obj_t *sysSur);
    void (*free)(PTREE_SUR_SYS_Obj_t *sysSur);
};

struct PTREE_SUR_SYS_Ops_s
{
    PARENA_Tag_t *(*occupyInfo)(PTREE_SUR_SYS_Obj_t *sysSur, void *pArena);
    PARENA_Tag_t *(*occupyInInfo)(PTREE_SUR_SYS_Obj_t *sysSur, void *pArena);
    PARENA_Tag_t *(*occupyOutInfo)(PTREE_SUR_SYS_Obj_t *sysSur, void *pArena);
#ifndef USING_BINARY
    int (*loadDb)(PTREE_SUR_SYS_Obj_t *sysSur, PTREE_SUR_SYS_Info_t *sysInfo, PTREE_DB_Obj_t db);
    int (*loadInDb)(PTREE_SUR_SYS_Obj_t *sysSur, PTREE_SUR_SYS_InInfo_t *sysInInfo, PTREE_DB_Obj_t db);
    int (*loadOutDb)(PTREE_SUR_SYS_Obj_t *sysSur, PTREE_SUR_SYS_OutInfo_t *sysOutInfo, PTREE_DB_Obj_t db);
#endif
};

struct PTREE_SUR_SYS_Obj_s
{
    PTREE_SUR_Obj_t             base;
    const PTREE_SUR_SYS_Ops_t * ops;
    const PTREE_SUR_SYS_Hook_t *hook;
};

int PTREE_SUR_SYS_Init(PTREE_SUR_SYS_Obj_t *sysSur, const PTREE_SUR_SYS_Ops_t *ops);

void PTREE_SUR_SYS_Register(PTREE_SUR_SYS_Obj_t *sysSur, const PTREE_SUR_SYS_Hook_t *hook);

PTREE_ENUM_DECLARE(MI_SYS_PixelFormat_e)
PTREE_ENUM_DECLARE(MI_SYS_BayerId_e)
PTREE_ENUM_DECLARE(MI_SYS_DataPrecision_e)

#endif /* ifndef __PTREE_SUR_SYS_H__ */
