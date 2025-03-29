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

#ifndef __PTREE_LINKER_H__
#define __PTREE_LINKER_H__

#include "ptree_obj.h"
#include "ptree_packet.h"

typedef struct PTREE_LINKER_Obj_s  PTREE_LINKER_Obj_t;
typedef struct PTREE_LINKER_Ops_s  PTREE_LINKER_Ops_t;
typedef struct PTREE_LINKER_Hook_s PTREE_LINKER_Hook_t;

struct PTREE_LINKER_Ops_s
{
    int (*enqueue)(PTREE_LINKER_Obj_t *linker, PTREE_PACKET_Obj_t *packet);
    PTREE_PACKET_Obj_t *(*dequeue)(PTREE_LINKER_Obj_t *linker, int ms);
};

struct PTREE_LINKER_Hook_s
{
    void (*destruct)(PTREE_LINKER_Obj_t *linker);
    void (*free)(PTREE_LINKER_Obj_t *linker);
};

struct PTREE_LINKER_Obj_s
{
    const PTREE_LINKER_Ops_t * ops;
    const PTREE_LINKER_Hook_t *hook;
    PTREE_OBJ_Obj_t            obj;
};

int PTREE_LINKER_Init(PTREE_LINKER_Obj_t *linker, const PTREE_LINKER_Ops_t *ops);

void PTREE_LINKER_Register(PTREE_LINKER_Obj_t *linker, const PTREE_LINKER_Hook_t *hook);

PTREE_LINKER_Obj_t *PTREE_LINKER_Dup(PTREE_LINKER_Obj_t *linker);

int PTREE_LINKER_Enqueue(PTREE_LINKER_Obj_t *linker, PTREE_PACKET_Obj_t *packet);

PTREE_PACKET_Obj_t *PTREE_LINKER_Dequeue(PTREE_LINKER_Obj_t *linker, int ms);

void PTREE_LINKER_Del(PTREE_LINKER_Obj_t *linker);

#endif /* ifndef __PTREE_LINKER_H__ */
