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

#ifndef __PTREE_PACKER_GROUP_H__
#define __PTREE_PACKER_GROUP_H__

#include "ssos_list.h"
#include "ssos_thread.h"
#include "ptree_packer.h"

typedef struct PTREE_PACKER_GROUP_Obj_s PTREE_PACKER_GROUP_Obj_t;

enum PTREE_PACKER_GROUP_Type_e
{
    E_PTREE_PACKER_GROUP_TYPE_FAST,
    E_PTREE_PACKER_GROUP_TYPE_SLOW,
};

struct PTREE_PACKER_GROUP_Obj_s
{
    PTREE_PACKER_Obj_t      base;
    SSOS_THREAD_RwLock_t    lock;
    struct SSOS_LIST_Head_s fastPacker;
    struct SSOS_LIST_Head_s slowPacker;
};

int PTREE_PACKER_GROUP_Init(PTREE_PACKER_GROUP_Obj_t *packerGroup);

int PTREE_PACKER_GROUP_AddPacker(PTREE_PACKER_GROUP_Obj_t *packerGroup, PTREE_PACKER_Obj_t *packer,
                                 enum PTREE_PACKER_GROUP_Type_e type);

int PTREE_PACKER_GROUP_DelPacker(PTREE_PACKER_GROUP_Obj_t *packerGroup, PTREE_PACKER_Obj_t *packer);

#endif /* ifndef __PTREE_PACKER_GROUP_H__ */
