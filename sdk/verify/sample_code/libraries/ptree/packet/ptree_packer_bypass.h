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

#ifndef __PTREE_PACKER_BYPASS_H__
#define __PTREE_PACKER_BYPASS_H__

#include "ptree_packer.h"

typedef struct PTREE_PACKER_BYPASS_Obj_s PTREE_PACKER_BYPASS_Obj_t;

struct PTREE_PACKER_BYPASS_Obj_s
{
    PTREE_PACKER_Obj_t  base;
    PTREE_PACKER_Obj_t *targetPacker;
};

int PTREE_PACKER_BYPASS_Init(PTREE_PACKER_BYPASS_Obj_t *packerBypass);

PTREE_PACKER_Obj_t *PTREE_PACKER_BYPASS_New(void);

void PTREE_PACKER_BYPASS_SetTarget(PTREE_PACKER_BYPASS_Obj_t *packerBypass, PTREE_PACKER_Obj_t *targetPacker);

#endif /* ifndef __PTREE_PACKER_BYPASS_H__ */
