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

#include "ptree_log.h"
#include "ssos_mem.h"
#include "ptree_mod.h"
#include "ptree_cmd.h"
#include "ptree_sur_file.h"
#include "ptree_mod_file.h"
#include "ptree_maker.h"

int PTREE_CMD_FILE_SetLeftFrameWrite(PTREE_MOD_Obj_t *pstModObj, const PTREE_CMD_Args_t *pstCmd, unsigned int paraCnt)
{
    PTREE_MOD_InObj_t *modIn = NULL;
    unsigned int       port  = 0;
    unsigned int       left  = 0;

    port = SSOS_IO_Atoi((char *)pstCmd->cmdPara[0]);
    left = SSOS_IO_Atoi((const char *)pstCmd->cmdPara[1]);

    modIn = PTREE_MOD_GetInObjByPort(pstModObj, port);
    if (!modIn)
    {
        return SSOS_DEF_FAIL;
    }

    PTREE_MOD_FILE_SetLeftFrame(modIn, left);

    PTREE_DBG("SetLeftFrameWrite  left :%d ,paraCnt %d\n", left, paraCnt);
    return 0;
}

int PTREE_CMD_FILE_WaitFrameWrite(PTREE_MOD_Obj_t *pstModObj, const PTREE_CMD_Args_t *pstCmd, unsigned int paraCnt)
{
    PTREE_MOD_InObj_t *modIn   = NULL;
    unsigned int       port    = 0;
    unsigned int       timeOut = 0;
    unsigned int       ret     = 0;

    port    = SSOS_IO_Atoi((char *)pstCmd->cmdPara[0]);
    timeOut = SSOS_IO_Atoi((const char *)pstCmd->cmdPara[1]);

    modIn = PTREE_MOD_GetInObjByPort(pstModObj, port);
    if (!modIn)
    {
        return SSOS_DEF_FAIL;
    }

    ret = PTREE_MOD_FILE_WaitFrame(modIn, timeOut);
    if (!ret)
    {
        PTREE_ERR("Wait timeout or error!, timeOut :%d,paraCnt %d\n", timeOut, paraCnt);
        return -1;
    }
    return 0;
}

PTREE_MAKER_CMD_INIT(FILE, PTREE_CMD_StrCompare, PTREE_CMD_StrHashVal,
                     {(unsigned long)"set_left_frame_write", PTREE_CMD_FILE_SetLeftFrameWrite, 2},
                     {(unsigned long)"wait_frame_write", PTREE_CMD_FILE_WaitFrameWrite, 2})