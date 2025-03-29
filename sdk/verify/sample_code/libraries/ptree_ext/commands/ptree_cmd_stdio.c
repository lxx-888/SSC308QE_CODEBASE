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

#include "ptree_cmd.h"
#include "ssos_def.h"
#include "ssos_mem.h"
#include "ptree_mod.h"
#include "ssos_io.h"
#include "ptree_maker.h"
#include "ptree_mod_stdio.h"

static int _PTREE_CMD_STDIO_UserTirgger(PTREE_MOD_Obj_t *mod, const PTREE_CMD_Args_t *args, unsigned int argc)
{
    PTREE_MOD_OutObj_t *modOut = NULL;
    unsigned int        port   = 0;
    unsigned int        count  = 0;
    unsigned int        i      = 0;

    (void)argc;

    port  = SSOS_IO_Atoi((char *)args->cmdPara[0]);
    count = SSOS_IO_Atoi((char *)args->cmdPara[1]);

    modOut = PTREE_MOD_GetOutObjByPort(mod, port);
    if (!modOut)
    {
        return SSOS_DEF_FAIL;
    }

    for (i = 0; i < count; ++i)
    {
        PTREE_MOD_STDIO_UserTirgger(modOut);
    }
    return 0;
}

PTREE_MAKER_CMD_INIT(STDIO, NULL, NULL,
                     {(unsigned long)"user_trigger", _PTREE_CMD_STDIO_UserTirgger, 2}, /* user_trigger <port> <count> */
);
