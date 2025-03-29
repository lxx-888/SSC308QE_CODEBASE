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

#ifndef __PTREE_PACKET_H__
#define __PTREE_PACKET_H__

#include "ptree_obj.h"
#include "ssos_thread.h"

#define PTREE_PACKET_INFO_TYPE(_type)                        \
    (                                                        \
        {                                                    \
            extern const char *__ptreePacketInfoType##_type; \
            __ptreePacketInfoType##_type;                    \
        })
#define PTREE_PACKET_INFO_TYPE_DEFINE(_type) const char *__ptreePacketInfoType##_type = #_type

#define PTREE_PACKET_TYPE(_type)                         \
    (                                                    \
        {                                                \
            extern const char *__ptreePacketType##_type; \
            __ptreePacketType##_type;                    \
        })
#define PTREE_PACKET_TYPE_DEFINE(_type) const char *__ptreePacketType##_type = #_type

typedef struct PTREE_PACKET_InfoHook_s  PTREE_PACKET_InfoHook_t;
typedef struct PTREE_PACKET_InfoOps_s   PTREE_PACKET_InfoOps_t;
typedef struct PTREE_PACKET_Info_s      PTREE_PACKET_Info_t;
typedef struct PTREE_PACKET_ObjHook_s   PTREE_PACKET_ObjHook_t;
typedef struct PTREE_PACKET_ObjOps_s    PTREE_PACKET_ObjOps_t;
typedef struct PTREE_PACKET_Obj_s       PTREE_PACKET_Obj_t;
typedef struct PTREE_PACKET_TimeStamp_s PTREE_PACKET_TimeStamp_t;

struct PTREE_PACKET_InfoHook_s
{
    void (*destruct)(PTREE_PACKET_Info_t *info);
    void (*free)(PTREE_PACKET_Info_t *info);
};
struct PTREE_PACKET_InfoOps_s
{
    int (*equal)(const PTREE_PACKET_Info_t *self, const PTREE_PACKET_Info_t *other);
};
struct PTREE_PACKET_Info_s
{
    PTREE_OBJ_Obj_t                obj;
    const PTREE_PACKET_InfoOps_t * ops;
    const PTREE_PACKET_InfoHook_t *hook;
    const char *                   type;
};

struct PTREE_PACKET_ObjHook_s
{
    void (*destruct)(PTREE_PACKET_Obj_t *packet);
    void (*free)(PTREE_PACKET_Obj_t *packet);
};
struct PTREE_PACKET_ObjOps_s
{
    PTREE_PACKET_Obj_t *(*dup)(PTREE_PACKET_Obj_t *packet);
    PTREE_PACKET_Obj_t *(*convert)(PTREE_PACKET_Obj_t *packet, const char *type);
    void (*updateTimeStamp)(PTREE_PACKET_Obj_t *packet);
};
struct PTREE_PACKET_TimeStamp_s
{
    unsigned long tvSec;
    unsigned long tvUSec;
};
struct PTREE_PACKET_Obj_s
{
    const char *                  type;
    PTREE_OBJ_Obj_t               obj;
    const PTREE_PACKET_ObjOps_t * ops;
    const PTREE_PACKET_ObjHook_t *hook;
    PTREE_PACKET_Info_t *         info;
    PTREE_PACKET_TimeStamp_t      stamp;
};

int PTREE_PACKET_Init(PTREE_PACKET_Obj_t *packet, const PTREE_PACKET_ObjOps_t *ops, PTREE_PACKET_Info_t *info,
                      const char *type);

void PTREE_PACKET_Register(PTREE_PACKET_Obj_t *packet, const PTREE_PACKET_ObjHook_t *hook);

void PTREE_PACKET_SetTimeStamp(PTREE_PACKET_Obj_t *packet, const PTREE_PACKET_TimeStamp_t *stamp);

const PTREE_PACKET_TimeStamp_t *PTREE_PACKET_GetTimeStamp(PTREE_PACKET_Obj_t *packet);

void PTREE_PACKET_AutoUpdateTimeStamp(PTREE_PACKET_Obj_t *packet);

int PTREE_PACKET_Likely(const PTREE_PACKET_Obj_t *packet, const char *type);

PTREE_PACKET_Obj_t *PTREE_PACKET_Dup(PTREE_PACKET_Obj_t *packet);

PTREE_PACKET_Obj_t *PTREE_PACKET_Convert(PTREE_PACKET_Obj_t *packet, const char *type);

void PTREE_PACKET_Del(PTREE_PACKET_Obj_t *packet);

int PTREE_PACKET_InfoInit(PTREE_PACKET_Info_t *info, const PTREE_PACKET_InfoOps_t *ops, const char *type);

void PTREE_PACKET_InfoRegister(PTREE_PACKET_Info_t *info, const PTREE_PACKET_InfoHook_t *hook);

int PTREE_PACKET_InfoLikely(const PTREE_PACKET_Info_t *info, const char *type);

PTREE_PACKET_Info_t *PTREE_PACKET_InfoDup(PTREE_PACKET_Info_t *info);

int PTREE_PACKET_InfoEqual(const PTREE_PACKET_Info_t *selfInfo, const PTREE_PACKET_Info_t *otherInfo);

void PTREE_PACKET_InfoDel(PTREE_PACKET_Info_t *info);

#endif /* ifndef __PTREE_PACKET_H__ */
