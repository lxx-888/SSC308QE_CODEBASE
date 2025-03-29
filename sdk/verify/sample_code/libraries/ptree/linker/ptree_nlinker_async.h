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

#ifndef __PTREE_NLINKER_ASYNC_H__
#define __PTREE_NLINKER_ASYNC_H__

#include "ptree_linker.h"
#include "ssos_list.h"
#include "ptree_packet.h"
#include "ssos_thread.h"

typedef struct PTREE_NLINKER_ASYNC_Obj_s  PTREE_NLINKER_ASYNC_Obj_t;
typedef struct PTREE_NLINKER_ASYNC_Ops_s  PTREE_NLINKER_ASYNC_Ops_t;
typedef struct PTREE_NLINKER_ASYNC_Hook_s PTREE_NLINKER_ASYNC_Hook_t;

struct PTREE_NLINKER_ASYNC_Hook_s
{
    void (*destruct)(PTREE_NLINKER_ASYNC_Obj_t *nlinkerAsync);
    void (*free)(PTREE_NLINKER_ASYNC_Obj_t *nlinkerAsync);
};

struct PTREE_NLINKER_ASYNC_Obj_s
{
    const PTREE_NLINKER_ASYNC_Ops_t * ops;
    const PTREE_NLINKER_ASYNC_Hook_t *hook;
    PTREE_LINKER_Obj_t                base;
    struct SSOS_LIST_Head_s           packetList;
    unsigned int                      packetCount;
    SSOS_THREAD_Mutex_t               mutex;
    SSOS_THREAD_Cond_t                cond;
    int                               bDropMsg;
    unsigned int                      packetDepth;
};

int PTREE_NLINKER_ASYNC_Init(PTREE_NLINKER_ASYNC_Obj_t *nlinkerAsync, int bDropMsg, unsigned int packetDepth);

void PTREE_NLINKER_ASYNC_Register(PTREE_NLINKER_ASYNC_Obj_t *nlinkerAsync, const PTREE_NLINKER_ASYNC_Hook_t *hook);

PTREE_PACKET_Obj_t *PTREE_NLINKER_ASYNC_WaitPacket(PTREE_NLINKER_ASYNC_Obj_t *nlinkerAsync, int ms);

void PTREE_NLINKER_ASYNC_Clear(PTREE_NLINKER_ASYNC_Obj_t *nlinkerAsync);

#endif /* ifndef __PTREE_NLINKER_ASYNC_H__ */
