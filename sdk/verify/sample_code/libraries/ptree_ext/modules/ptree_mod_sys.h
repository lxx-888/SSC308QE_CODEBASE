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
#ifndef __PTREE_MOD_SYS_H__
#define __PTREE_MOD_SYS_H__

#include "mi_common_datatype.h"
#include "mi_sys_datatype.h"
#include "ptree_mod.h"
#include "ptree_packet_raw.h"

typedef struct PTREE_MOD_SYS_Hook_s    PTREE_MOD_SYS_Hook_t;
typedef struct PTREE_MOD_SYS_InHook_s  PTREE_MOD_SYS_InHook_t;
typedef struct PTREE_MOD_SYS_OutHook_s PTREE_MOD_SYS_OutHook_t;

typedef struct PTREE_MOD_SYS_Ops_s    PTREE_MOD_SYS_Ops_t;
typedef struct PTREE_MOD_SYS_InOps_s  PTREE_MOD_SYS_InOps_t;
typedef struct PTREE_MOD_SYS_OutOps_s PTREE_MOD_SYS_OutOps_t;

typedef struct PTREE_MOD_SYS_Obj_s    PTREE_MOD_SYS_Obj_t;
typedef struct PTREE_MOD_SYS_InObj_s  PTREE_MOD_SYS_InObj_t;
typedef struct PTREE_MOD_SYS_OutObj_s PTREE_MOD_SYS_OutObj_t;

struct PTREE_MOD_SYS_Hook_s
{
    void (*destruct)(PTREE_MOD_SYS_Obj_t *sysMod);
    void (*free)(PTREE_MOD_SYS_Obj_t *sysMod);
};
struct PTREE_MOD_SYS_Ops_s
{
    int (*init)(PTREE_MOD_SYS_Obj_t *sysMod);
    int (*deinit)(PTREE_MOD_SYS_Obj_t *sysMod);
    int (*prepare)(PTREE_MOD_SYS_Obj_t *sysMod);
    int (*unprepare)(PTREE_MOD_SYS_Obj_t *sysMod);
    int (*start)(PTREE_MOD_SYS_Obj_t *sysMod);
    int (*stop)(PTREE_MOD_SYS_Obj_t *sysMod);

    PTREE_MOD_InObj_t *(*createModIn)(PTREE_MOD_Obj_t *mod, unsigned int loopId);
    PTREE_MOD_OutObj_t *(*createModOut)(PTREE_MOD_Obj_t *mod, unsigned int loopId);
};
struct PTREE_MOD_SYS_Obj_s
{
    const PTREE_MOD_SYS_Ops_t * ops;
    const PTREE_MOD_SYS_Hook_t *hook;
    PTREE_MOD_Obj_t             base;
    MI_ModuleId_e               modId;
};

struct PTREE_MOD_SYS_InHook_s
{
    void (*destruct)(PTREE_MOD_SYS_InObj_t *modIn);
    void (*free)(PTREE_MOD_SYS_InObj_t *modIn);
};
struct PTREE_MOD_SYS_InOps_s
{
    int (*start)(PTREE_MOD_SYS_InObj_t *modIn);
    int (*stop)(PTREE_MOD_SYS_InObj_t *modIn);

    int (*linked)(PTREE_MOD_SYS_InObj_t *modIn, unsigned int ref);
    int (*unlinked)(PTREE_MOD_SYS_InObj_t *modIn, unsigned int ref);

    int (*directBind)(PTREE_MOD_SYS_InObj_t *modIn);
    int (*directUnbind)(PTREE_MOD_SYS_InObj_t *modIn);
};
struct PTREE_MOD_SYS_InObj_s
{
    const PTREE_MOD_SYS_InOps_t * ops;
    const PTREE_MOD_SYS_InHook_t *hook;
    PTREE_MOD_InObj_t             base;
    PTREE_MOD_SYS_Obj_t *         thisSysMod;
};

struct PTREE_MOD_SYS_OutHook_s
{
    void (*destruct)(PTREE_MOD_SYS_OutObj_t *modOut);
    void (*free)(PTREE_MOD_SYS_OutObj_t *modOut);
};
struct PTREE_MOD_SYS_OutOps_s
{
    int (*start)(PTREE_MOD_SYS_OutObj_t *modOut);
    int (*stop)(PTREE_MOD_SYS_OutObj_t *modOut);

    void (*getPacketInfo)(PTREE_MOD_SYS_OutObj_t *modOut, PTREE_PACKET_RAW_RawInfo_t *rawInfo);

    PTREE_MOD_SYS_InObj_t *(*resetStreamTraverse)(PTREE_MOD_SYS_OutObj_t *modOut);
    int (*resetStreamOut)(PTREE_MOD_SYS_OutObj_t *modOut, unsigned int width, unsigned int height);
};
struct PTREE_MOD_SYS_OutObj_s
{
    const PTREE_MOD_SYS_OutOps_t * ops;
    const PTREE_MOD_SYS_OutHook_t *hook;
    PTREE_MOD_OutObj_t             base;
    void *                         taskHandle;
    PTREE_MOD_SYS_Obj_t *          thisSysMod;
};

int  PTREE_MOD_SYS_ObjInit(PTREE_MOD_SYS_Obj_t *mod, const PTREE_MOD_SYS_Ops_t *ops, PARENA_Tag_t *tag,
                           MI_ModuleId_e modId);
int  PTREE_MOD_SYS_InObjInit(PTREE_MOD_SYS_InObj_t *modIn, const PTREE_MOD_SYS_InOps_t *ops, PTREE_MOD_SYS_Obj_t *mod,
                             unsigned int loopId);
int  PTREE_MOD_SYS_OutObjInit(PTREE_MOD_SYS_OutObj_t *modOut, const PTREE_MOD_SYS_OutOps_t *ops,
                              PTREE_MOD_SYS_Obj_t *mod, unsigned int loopId);
void PTREE_MOD_SYS_ObjRegister(PTREE_MOD_SYS_Obj_t *mod, const PTREE_MOD_SYS_Hook_t *hook);
void PTREE_MOD_SYS_InObjRegister(PTREE_MOD_SYS_InObj_t *modIn, const PTREE_MOD_SYS_InHook_t *hook);
void PTREE_MOD_SYS_OutObjRegister(PTREE_MOD_SYS_OutObj_t *modOut, const PTREE_MOD_SYS_OutHook_t *hook);
enum PTREE_PACKET_RAW_VideoFmt_e PTREE_MOD_SYS_SysFmtToPtreeFmt(MI_SYS_PixelFormat_e fmt);
MI_SYS_PixelFormat_e             PTREE_MOD_SYS_PtreeFmtToSysFmt(enum PTREE_PACKET_RAW_VideoFmt_e fmt);

#endif //__PTREE_MOD_SYS_H__
