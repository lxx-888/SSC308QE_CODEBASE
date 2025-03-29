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

#ifndef __PTREE_LINKER_BYPASS_H__
#define __PTREE_LINKER_BYPASS_H__

#include "ptree_linker.h"
#include "ssos_thread.h"

typedef struct PTREE_LINKER_BYPASS_Obj_s PTREE_LINKER_BYPASS_Obj_t;

struct PTREE_LINKER_BYPASS_Obj_s
{
    PTREE_LINKER_Obj_t   base;
    PTREE_LINKER_Obj_t * targetLinker;
    SSOS_THREAD_RwLock_t lock;
};

int PTREE_LINKER_BYPASS_Init(PTREE_LINKER_BYPASS_Obj_t *linkerBypass);

void PTREE_LINKER_BYPASS_SetTarget(PTREE_LINKER_BYPASS_Obj_t *linkerBypass, PTREE_LINKER_Obj_t *targetLinker);

#endif /* ifndef __PTREE_LINKER_BYPASS_H__ */
