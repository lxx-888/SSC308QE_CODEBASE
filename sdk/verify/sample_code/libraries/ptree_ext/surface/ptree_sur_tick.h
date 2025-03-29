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

#ifndef __PTREE_SUR_TICK_H__
#define __PTREE_SUR_TICK_H__

#include "ptree_sur.h"

typedef struct PTREE_SUR_TICK_Info_s
{
    PTREE_SUR_Info_t base;
    unsigned int     sec;
    unsigned int     nSec;
    int              frameCntLimit;
    char             frcType[16];
    char             ioMode[16];
} PTREE_SUR_TICK_Info_t;

#endif //__PTREE_MOD_TICK_H__
