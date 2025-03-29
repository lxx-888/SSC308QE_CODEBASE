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

#include "ssos_def.h"
#include "ssos_io.h"
#include "ssos_list.h"
#include "ptree_log.h"
#include "ssos_mem.h"
#include "ptree_enum.h"
#include "ptree_sur.h"
#include "ptree_maker.h"

static void                   _PTREE_SUR_EMPTY_Free(PTREE_SUR_Obj_t *sur);
static const PTREE_SUR_Ops_t  G_PTREE_SUR_EMPTY_OPS  = {};
static const PTREE_SUR_Hook_t G_PTREE_SUR_EMPTY_HOOK = {
    .free = _PTREE_SUR_EMPTY_Free,
};
static void _PTREE_SUR_EMPTY_Free(PTREE_SUR_Obj_t *sur)
{
    SSOS_MEM_Free(sur);
}

PTREE_SUR_Obj_t *PTREE_SUR_EMPTY_New(void)
{
    PTREE_SUR_Obj_t *sur = SSOS_MEM_Alloc(sizeof(PTREE_SUR_Obj_t));
    if (!sur)
    {
        PTREE_ERR("Alloc err");
        return NULL;
    }
    memset(sur, 0, sizeof(PTREE_SUR_Obj_t));
    if (SSOS_DEF_OK != PTREE_SUR_Init(sur, &G_PTREE_SUR_EMPTY_OPS))
    {
        SSOS_MEM_Free(sur);
        return NULL;
    }
    PTREE_SUR_Register(sur, &G_PTREE_SUR_EMPTY_HOOK);
    return sur;
}

PTREE_MAKER_SUR_INIT(EMPTY, PTREE_SUR_EMPTY_New);
PTREE_MAKER_SUR_INIT(CMD_TEST, PTREE_SUR_EMPTY_New);
PTREE_MAKER_SUR_INIT(STR_CMD_TEST, PTREE_SUR_EMPTY_New);
