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
#ifndef __PTREE_SUR_PASS_H__
#define __PTREE_SUR_PASS_H__
#include "parena.h"
#include "ptree_sur.h"

typedef struct PTREE_SUR_PASS_InInfo_s
{
    PTREE_SUR_InInfo_t base;
    char               modName[32];
    unsigned int       devId;
    unsigned int       chnId;
    unsigned int       secPortId;
} PTREE_SUR_PASS_InInfo_t;

typedef struct PTREE_SUR_PASS_OutInfo_s
{
    PTREE_SUR_OutInfo_t base;
    unsigned char       headIdx;
    int                 rootOffset[PARENA_ROOT_COUNT_MAX];
    unsigned int        secPortId;
    char                arenaBuffer[4096];
} PTREE_SUR_PASS_OutInfo_t;

#define PTREE_PASS_MOD_HASH_SIZE 32

#endif //__PTREE_SUR_PASS_H__
