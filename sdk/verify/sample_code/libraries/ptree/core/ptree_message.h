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

#ifndef __PTREE_MESSAGE_H__
#define __PTREE_MESSAGE_H__

#include "ptree_obj.h"
#include "ptree_packet.h"
#include "ssos_thread.h"

typedef struct PTREE_MESSAGE_Obj_s  PTREE_MESSAGE_Obj_t;
typedef struct PTREE_MESSAGE_Ops_s  PTREE_MESSAGE_Ops_t;
typedef struct PTREE_MESSAGE_Hook_s PTREE_MESSAGE_Hook_t;

struct PTREE_MESSAGE_Hook_s
{
    void (*destruct)(PTREE_MESSAGE_Obj_t *message);
    void (*free)(PTREE_MESSAGE_Obj_t *message);
};

struct PTREE_MESSAGE_Ops_s
{
    void (*connected)(PTREE_MESSAGE_Obj_t *message, unsigned int ref);
    void (*disconnected)(PTREE_MESSAGE_Obj_t *message, unsigned int ref);
    void (*suspend)(PTREE_MESSAGE_Obj_t *message); /* Only used to stop connected monitor */
    void (*resume)(PTREE_MESSAGE_Obj_t *message);  /* Only used to start connected monitor */
    PTREE_PACKET_Info_t *(*getPacketInfo)(PTREE_MESSAGE_Obj_t *message);
};

struct PTREE_MESSAGE_Obj_s
{
    const PTREE_MESSAGE_Ops_t * ops;
    const PTREE_MESSAGE_Hook_t *hook;
    PTREE_OBJ_Obj_t             obj;
    SSOS_THREAD_RwLock_t        lock;
    unsigned int                status;
    unsigned int                refCount;
    unsigned char               bSuspend;
};

int PTREE_MESSAGE_Init(PTREE_MESSAGE_Obj_t *message, const PTREE_MESSAGE_Ops_t *ops);

void PTREE_MESSAGE_Register(PTREE_MESSAGE_Obj_t *message, const PTREE_MESSAGE_Hook_t *hook);

PTREE_MESSAGE_Obj_t *PTREE_MESSAGE_Dup(PTREE_MESSAGE_Obj_t *message);

int PTREE_MESSAGE_Check(PTREE_MESSAGE_Obj_t *message);

int PTREE_MESSAGE_Access(PTREE_MESSAGE_Obj_t *message);

int PTREE_MESSAGE_Leave(PTREE_MESSAGE_Obj_t *message);

int PTREE_MESSAGE_ConnectIn(PTREE_MESSAGE_Obj_t *message);

int PTREE_MESSAGE_DisconnectIn(PTREE_MESSAGE_Obj_t *message);

int PTREE_MESSAGE_Suspend(PTREE_MESSAGE_Obj_t *message);

int PTREE_MESSAGE_Resume(PTREE_MESSAGE_Obj_t *message);

PTREE_PACKET_Info_t *PTREE_MESSAGE_GetPacketInfo(PTREE_MESSAGE_Obj_t *message);

void PTREE_MESSAGE_Del(PTREE_MESSAGE_Obj_t *message);

#endif /* ifndef __PTREE_MESSAGE_H__ */
