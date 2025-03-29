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

#ifndef __PTREE_NLINKER_OUT_H__
#define __PTREE_NLINKER_OUT_H__

#include "ptree_linker.h"
#include "ssos_list.h"
#include "ptree_packet.h"
#include "ssos_thread.h"

typedef struct PTREE_NLINKER_OUT_Obj_s  PTREE_NLINKER_OUT_Obj_t;
typedef struct PTREE_NLINKER_OUT_Ops_s  PTREE_NLINKER_OUT_Ops_t;
typedef struct PTREE_NLINKER_OUT_Hook_s PTREE_NLINKER_OUT_Hook_t;

struct PTREE_NLINKER_OUT_Hook_s
{
    void (*destruct)(PTREE_NLINKER_OUT_Obj_t *nlinkerOut);
    void (*free)(PTREE_NLINKER_OUT_Obj_t *nlinkerOut);
};

struct PTREE_NLINKER_OUT_Ops_s
{
    int (*enqueue)(PTREE_NLINKER_OUT_Obj_t *nlinkerOut, PTREE_PACKET_Obj_t *packet);
    PTREE_PACKET_Obj_t *(*dequeueFromInside)(PTREE_NLINKER_OUT_Obj_t *nlinkerOut, int ms);
    int (*dequeueOut)(PTREE_NLINKER_OUT_Obj_t *nlinkerOut, PTREE_PACKET_Obj_t *packet);
};

struct PTREE_NLINKER_OUT_Obj_s
{
    const PTREE_NLINKER_OUT_Ops_t * ops;
    const PTREE_NLINKER_OUT_Hook_t *hook;
    PTREE_LINKER_Obj_t              base;
    struct SSOS_LIST_Head_s         packetList;
    SSOS_THREAD_Mutex_t             mutex;
};

int PTREE_NLINKER_OUT_Init(PTREE_NLINKER_OUT_Obj_t *nlinkerOut, const PTREE_NLINKER_OUT_Ops_t *ops);

void PTREE_NLINKER_OUT_Register(PTREE_NLINKER_OUT_Obj_t *nlinkerOut, const PTREE_NLINKER_OUT_Hook_t *hook);

#endif /* ifndef __PTREE_NLINKER_OUT_H__ */
