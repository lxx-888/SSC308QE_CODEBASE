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

#ifndef __PTREE_PACKER_H__
#define __PTREE_PACKER_H__

#include "ptree_obj.h"
#include "ptree_packet.h"

typedef struct PTREE_PACKER_Obj_s  PTREE_PACKER_Obj_t;
typedef struct PTREE_PACKER_Ops_s  PTREE_PACKER_Ops_t;
typedef struct PTREE_PACKER_Hook_s PTREE_PACKER_Hook_t;

struct PTREE_PACKER_Ops_s
{
    PTREE_PACKET_Obj_t *(*make)(PTREE_PACKER_Obj_t *packer, const PTREE_PACKET_Info_t *info);
};

struct PTREE_PACKER_Hook_s
{
    void (*destruct)(PTREE_PACKER_Obj_t *packer);
    void (*free)(PTREE_PACKER_Obj_t *packer);
};

struct PTREE_PACKER_Obj_s
{
    PTREE_OBJ_Obj_t            obj;
    const PTREE_PACKER_Ops_t * ops;
    const PTREE_PACKER_Hook_t *hook;
};

int PTREE_PACKER_Init(PTREE_PACKER_Obj_t *packer, const PTREE_PACKER_Ops_t *ops);

void PTREE_PACKER_Register(PTREE_PACKER_Obj_t *packer, const PTREE_PACKER_Hook_t *hook);

PTREE_PACKER_Obj_t *PTREE_PACKER_Dup(PTREE_PACKER_Obj_t *packer);

PTREE_PACKET_Obj_t *PTREE_PACKER_Make(PTREE_PACKER_Obj_t *packer, const PTREE_PACKET_Info_t *info);

void PTREE_PACKER_Del(PTREE_PACKER_Obj_t *packer);

int PTREE_PACKER_NormalInit(PTREE_PACKER_Obj_t *packer);

PTREE_PACKER_Obj_t *PTREE_PACKER_NormalNew(void);

#endif /* ifndef __PTREE_PACKER_H__ */
