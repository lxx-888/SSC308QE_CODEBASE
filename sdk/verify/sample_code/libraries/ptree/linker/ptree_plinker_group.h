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

#ifndef __PTREE_PLINKER_GROUP_H__
#define __PTREE_PLINKER_GROUP_H__

#include "ssos_list.h"
#include "ssos_thread.h"
#include "ptree_linker.h"

typedef struct PTREE_PLINKER_GROUP_Obj_s PTREE_PLINKER_GROUP_Obj_t;

struct PTREE_PLINKER_GROUP_Obj_s
{
    PTREE_LINKER_Obj_t      base;
    SSOS_THREAD_RwLock_t    lock;
    struct SSOS_LIST_Head_s lstLinker;
};

int PTREE_PLINKER_GROUP_Init(PTREE_PLINKER_GROUP_Obj_t *plinkerGroup);

int PTREE_PLINKER_GROUP_AddLinker(PTREE_PLINKER_GROUP_Obj_t *plinkerGroup, PTREE_LINKER_Obj_t *linker);

int PTREE_PLINKER_GROUP_DelLinker(PTREE_PLINKER_GROUP_Obj_t *plinkerGroup, PTREE_LINKER_Obj_t *linker);

int PTREE_PLINKER_GROUP_Empty(PTREE_PLINKER_GROUP_Obj_t *plinkerGroup);

#endif /* ifndef __PTREE_PLINKER_GROUP_H__ */
