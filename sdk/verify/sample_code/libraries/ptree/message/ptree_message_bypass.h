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

#ifndef __PTREE_MESSAGE_BYPASS_H__
#define __PTREE_MESSAGE_BYPASS_H__

#include "ptree_message.h"

typedef struct PTREE_MESSAGE_BYPASS_Obj_s PTREE_MESSAGE_BYPASS_Obj_t;

struct PTREE_MESSAGE_BYPASS_Obj_s
{
    PTREE_MESSAGE_Obj_t  base;
    PTREE_MESSAGE_Obj_t *targetMessage;
};

int PTREE_MESSAGE_BYPASS_Init(PTREE_MESSAGE_BYPASS_Obj_t *messageBypass);

void PTREE_MESSAGE_BYPASS_SetTarget(PTREE_MESSAGE_BYPASS_Obj_t *messageBypass, PTREE_MESSAGE_Obj_t *targetMessage);

#endif /* ifndef __PTREE_MESSAGE_BYPASS_H__ */
