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

#ifndef __PTREE_MOD_H__
#define __PTREE_MOD_H__

#include "ptree_obj.h"
#include "ptree_message.h"
#include "ptree_message_bypass.h"
#include "ptree_linker.h"
#include "ptree_linker_bypass.h"
#include "ptree_packet.h"
#include "ptree_plinker_group.h"
#include "ptree_packer_group.h"
#include "ptree_sur.h"

typedef struct PTREE_MOD_Hook_s    PTREE_MOD_Hook_t;
typedef struct PTREE_MOD_InHook_s  PTREE_MOD_InHook_t;
typedef struct PTREE_MOD_OutHook_s PTREE_MOD_OutHook_t;

typedef struct PTREE_MOD_Ops_s    PTREE_MOD_Ops_t;
typedef struct PTREE_MOD_InOps_s  PTREE_MOD_InOps_t;
typedef struct PTREE_MOD_OutOps_s PTREE_MOD_OutOps_t;

typedef struct PTREE_MOD_Obj_s    PTREE_MOD_Obj_t;
typedef struct PTREE_MOD_InObj_s  PTREE_MOD_InObj_t;
typedef struct PTREE_MOD_OutObj_s PTREE_MOD_OutObj_t;

enum PTREE_MOD_BindType_e
{
    E_PTREE_MOD_BIND_TYPE_NONE   = 0x0,
    E_PTREE_MOD_BIND_TYPE_KERNEL = 0x1,
    E_PTREE_MOD_BIND_TYPE_USER   = 0x2
};

enum PTREE_MOD_Status_e
{
    E_PTREE_MOD_STATUS_CONSTRUCT, /* Module instance has been built. */
    E_PTREE_MOD_STATUS_ACTIVATE,  /* Module has been initialized. */
    E_PTREE_MOD_STATUS_PREPARE,   /* Module has been prepared. */
    E_PTREE_MOD_STATUS_FINISH     /* Module has been started. */
};

struct PTREE_MOD_Hook_s
{
    void (*destruct)(PTREE_MOD_Obj_t *mod);
    void (*free)(PTREE_MOD_Obj_t *mod);
};
struct PTREE_MOD_Ops_s
{
    int (*init)(PTREE_MOD_Obj_t *mod);
    int (*deinit)(PTREE_MOD_Obj_t *mod);
    int (*prepare)(PTREE_MOD_Obj_t *mod);
    int (*unprepare)(PTREE_MOD_Obj_t *mod);
    int (*start)(PTREE_MOD_Obj_t *mod);
    int (*stop)(PTREE_MOD_Obj_t *mod);

    PTREE_MOD_InObj_t *(*createModIn)(PTREE_MOD_Obj_t *mod, unsigned int loopId);
    PTREE_MOD_OutObj_t *(*createModOut)(PTREE_MOD_Obj_t *mod, unsigned int loopId);
};
struct PTREE_MOD_Obj_s
{
    const PTREE_MOD_Ops_t * ops;
    const PTREE_MOD_Hook_t *hook;
    PTREE_OBJ_Obj_t         obj;
    PARENA_Tag_t *          tag;
    PTREE_SUR_Info_t *      info;
    PTREE_MOD_InObj_t **    arrModIn;
    PTREE_MOD_OutObj_t **   arrModOut;

    enum PTREE_MOD_Status_e status;
    /* Mark the resource that release had been done before. */
    unsigned int markRelease : 1;
};

struct PTREE_MOD_InHook_s
{
    void (*destruct)(PTREE_MOD_InObj_t *modIn);
    void (*free)(PTREE_MOD_InObj_t *modIn);
};
struct PTREE_MOD_InOps_s
{
    int (*start)(PTREE_MOD_InObj_t *modIn);
    int (*stop)(PTREE_MOD_InObj_t *modIn);

    int (*directBind)(PTREE_MOD_InObj_t *modIn);
    int (*directUnbind)(PTREE_MOD_InObj_t *modIn);

    int (*getType)(PTREE_MOD_InObj_t *modIn);

    int (*isPostReader)(PTREE_MOD_InObj_t *modIn);
    int (*isDelayLink)(PTREE_MOD_InObj_t *modIn);

    PTREE_LINKER_Obj_t *(*createNLinker)(PTREE_MOD_InObj_t *modIn);
    PTREE_PACKER_Obj_t *(*createPacker)(PTREE_MOD_InObj_t *modIn, int *isFast);

    int (*linked)(PTREE_MOD_InObj_t *modIn, unsigned int ref);
    int (*unlinked)(PTREE_MOD_InObj_t *modIn, unsigned int ref);
    int (*suspend)(PTREE_MOD_InObj_t *modIn);
    int (*resume)(PTREE_MOD_InObj_t *modIn);
};
struct PTREE_MOD_InObj_s
{
    const PTREE_MOD_InOps_t * ops;
    const PTREE_MOD_InHook_t *hook;
    PTREE_OBJ_Obj_t           obj;
    PTREE_SUR_InInfo_t *      info;
    PTREE_MOD_Obj_t *         thisMod;
    PTREE_MOD_OutObj_t *      prevModOut;
    PTREE_MOD_OutObj_t *      delayModOut;

    PTREE_MESSAGE_Obj_t       message;
    PTREE_PACKER_Obj_t *      packer;
    PTREE_LINKER_BYPASS_Obj_t plinker;
    PTREE_LINKER_Obj_t *      nlinker;

    unsigned int hasStart : 1;
    unsigned int hasBind : 1;
    /* Mark the resource that release had been done before. */
    unsigned int markRelease : 1;
    unsigned int markUnbind : 1;
};

struct PTREE_MOD_OutHook_s
{
    void (*destruct)(PTREE_MOD_OutObj_t *modOut);
    void (*free)(PTREE_MOD_OutObj_t *modOut);
};
struct PTREE_MOD_OutOps_s
{
    int (*start)(PTREE_MOD_OutObj_t *modOut);
    int (*stop)(PTREE_MOD_OutObj_t *modOut);

    int (*getType)(PTREE_MOD_OutObj_t *modOut);

    PTREE_LINKER_Obj_t *(*createNLinker)(PTREE_MOD_OutObj_t *modOut);

    PTREE_PACKET_Info_t *(*getPacketInfo)(PTREE_MOD_OutObj_t *modOut);

    int (*linked)(PTREE_MOD_OutObj_t *modOut, unsigned int ref);
    int (*unlinked)(PTREE_MOD_OutObj_t *modOut, unsigned int ref);
    int (*suspend)(PTREE_MOD_OutObj_t *modOut);
    int (*resume)(PTREE_MOD_OutObj_t *modOut);
    int (*linkedTransfer)(PTREE_MOD_OutObj_t *modOut);
    int (*unlinkedTransfer)(PTREE_MOD_OutObj_t *modOut);
    int (*suspendTransfer)(PTREE_MOD_OutObj_t *modOut);
    int (*resumeTransfer)(PTREE_MOD_OutObj_t *modOut);

    PTREE_MOD_InObj_t *(*resetStreamTraverse)(PTREE_MOD_OutObj_t *modOut);
    int (*resetStreamOut)(PTREE_MOD_OutObj_t *modOut, unsigned int width, unsigned int height);

    int (*createDelayPass)(PTREE_MOD_OutObj_t *modOut, PTREE_MOD_InObj_t *postModIn);
    int (*destroyDelayPass)(PTREE_MOD_OutObj_t *modOut, PTREE_MOD_InObj_t *postModIn);
    int (*initDelayPass)(PTREE_MOD_OutObj_t *modOut);
    int (*deinitDelayPass)(PTREE_MOD_OutObj_t *modOut);
    int (*bindDelayPass)(PTREE_MOD_OutObj_t *modOut);
    int (*unbindDelayPass)(PTREE_MOD_OutObj_t *modOut);
    int (*startDelayPass)(PTREE_MOD_OutObj_t *modOut);
    int (*stopDelayPass)(PTREE_MOD_OutObj_t *modOut);
};
struct PTREE_MOD_OutObj_s
{
    const PTREE_MOD_OutOps_t * ops;
    const PTREE_MOD_OutHook_t *hook;
    PTREE_OBJ_Obj_t            obj;
    PTREE_SUR_OutInfo_t *      info;
    PTREE_MOD_Obj_t *          thisMod;

    PTREE_MESSAGE_Obj_t       message;
    PTREE_PACKER_GROUP_Obj_t  packer;
    PTREE_PLINKER_GROUP_Obj_t plinker;
    PTREE_LINKER_Obj_t *      nlinker;

    unsigned int bindCount;
    unsigned int hasStart : 1;
    /* Mark the resource that release had been done before. */
    unsigned int markRelease : 1;
};

unsigned long PTREE_MOD_GetTimer(void);

int PTREE_MOD_ObjInit(PTREE_MOD_Obj_t *mod, const PTREE_MOD_Ops_t *ops, PARENA_Tag_t *tag);
int PTREE_MOD_InObjInit(PTREE_MOD_InObj_t *modIn, const PTREE_MOD_InOps_t *ops, PTREE_MOD_Obj_t *mod,
                        unsigned int loopId);
int PTREE_MOD_OutObjInit(PTREE_MOD_OutObj_t *modOut, const PTREE_MOD_OutOps_t *ops, PTREE_MOD_Obj_t *mod,
                         unsigned int loopId);

PTREE_MOD_Obj_t *   PTREE_MOD_ObjDup(PTREE_MOD_Obj_t *mod);
PTREE_MOD_InObj_t * PTREE_MOD_InObjDup(PTREE_MOD_InObj_t *modIn);
PTREE_MOD_OutObj_t *PTREE_MOD_OutObjDup(PTREE_MOD_OutObj_t *modOut);

void PTREE_MOD_ObjRegister(PTREE_MOD_Obj_t *mod, const PTREE_MOD_Hook_t *hook);
void PTREE_MOD_InObjRegister(PTREE_MOD_InObj_t *modIn, const PTREE_MOD_InHook_t *hook);
void PTREE_MOD_OutObjRegister(PTREE_MOD_OutObj_t *modOut, const PTREE_MOD_OutHook_t *hook);

void PTREE_MOD_ObjDel(PTREE_MOD_Obj_t *mod);
void PTREE_MOD_InObjDel(PTREE_MOD_InObj_t *modIn);
void PTREE_MOD_OutObjDel(PTREE_MOD_OutObj_t *modOut);

void PTREE_MOD_CreateIo(PTREE_MOD_Obj_t *mod);

int PTREE_MOD_Init(PTREE_MOD_Obj_t *mod);
int PTREE_MOD_Deinit(PTREE_MOD_Obj_t *mod);

int PTREE_MOD_Prepare(PTREE_MOD_Obj_t *mod);
int PTREE_MOD_Unprepare(PTREE_MOD_Obj_t *mod);

int PTREE_MOD_Start(PTREE_MOD_Obj_t *mod);
int PTREE_MOD_Stop(PTREE_MOD_Obj_t *mod);

int PTREE_MOD_StartIn(PTREE_MOD_InObj_t *modIn);
int PTREE_MOD_StopIn(PTREE_MOD_InObj_t *modIn);

int PTREE_MOD_StartOut(PTREE_MOD_OutObj_t *modOut);
int PTREE_MOD_StopOut(PTREE_MOD_OutObj_t *modOut);

int PTREE_MOD_Link(PTREE_MOD_InObj_t *modIn, PTREE_MOD_OutObj_t *prevModOut);
int PTREE_MOD_Unlink(PTREE_MOD_InObj_t *modIn);

int PTREE_MOD_Bind(PTREE_MOD_InObj_t *modIn);
int PTREE_MOD_Unbind(PTREE_MOD_InObj_t *modIn);

/* Protected functions */

char *PTREE_MOD_KeyStr(const PTREE_MOD_Obj_t *mod, char *str, unsigned long len);
char *PTREE_MOD_InKeyStr(const PTREE_MOD_InObj_t *modIn, char *str, unsigned long len);
char *PTREE_MOD_OutKeyStr(const PTREE_MOD_OutObj_t *modOut, char *str, unsigned long len);

PTREE_MOD_InObj_t * PTREE_MOD_GetInObjByPort(const PTREE_MOD_Obj_t *pstModObj, unsigned char portId);
PTREE_MOD_OutObj_t *PTREE_MOD_GetOutObjByPort(const PTREE_MOD_Obj_t *pstModObj, unsigned char portId);

int PTREE_MOD_ResetStream(PTREE_MOD_InObj_t *modIn, unsigned int width, unsigned int height);

int PTREE_MOD_CreateDelayPass(PTREE_MOD_InObj_t *modIn);
int PTREE_MOD_DestroyDelayPass(PTREE_MOD_InObj_t *modIn);
int PTREE_MOD_InitDelayPass(PTREE_MOD_InObj_t *modIn);
int PTREE_MOD_DeinitDelayPass(PTREE_MOD_InObj_t *modIn);
int PTREE_MOD_BindDelayPass(PTREE_MOD_InObj_t *modIn);
int PTREE_MOD_UnbindDelayPass(PTREE_MOD_InObj_t *modIn);
int PTREE_MOD_StartDelayPass(PTREE_MOD_InObj_t *modIn);
int PTREE_MOD_StopDelayPass(PTREE_MOD_InObj_t *modIn);

/* Patch to simulate customer's resource release. */
int PTREE_MOD_MarkStop(PTREE_MOD_Obj_t *mod);
int PTREE_MOD_MarkUnprepare(PTREE_MOD_Obj_t *mod);
int PTREE_MOD_MarkDeinit(PTREE_MOD_Obj_t *mod);
int PTREE_MOD_MarkStopIn(PTREE_MOD_InObj_t *modIn);
int PTREE_MOD_MarkStopOut(PTREE_MOD_OutObj_t *modOut);
int PTREE_MOD_MarkUnbind(PTREE_MOD_InObj_t *modIn);
#endif /* __PTREE_MOD_H__ */
