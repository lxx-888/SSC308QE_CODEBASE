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
#include "ptree_sur_tick.h"
#include "ptree_mod_tick.h"
#include "ptree_maker.h"

int PTREE_CMD_TICK_SetBlockTime(PTREE_MOD_Obj_t *pstModObj, const PTREE_CMD_Args_t *pstCmd, unsigned int paraCnt)
{
    unsigned int sec  = 0;
    unsigned int nsec = 0;

    sec  = SSOS_IO_Atoi((char *)pstCmd->cmdPara[0]);
    nsec = SSOS_IO_Atoi((const char *)pstCmd->cmdPara[1]);

    PTREE_MOD_TICK_SetBlockTime(pstModObj, sec, nsec);

    PTREE_DBG("SetBlockTime  sec :%d ,nsec %d, paraCnt %d\n", sec, nsec, paraCnt);
    return 0;
}

int PTREE_CMD_TICK_SetRateFps(PTREE_MOD_Obj_t *pstModObj, const PTREE_CMD_Args_t *pstCmd, unsigned int paraCnt)
{
    unsigned int srcFps = 0;
    unsigned int dstFps = 0;

    srcFps = SSOS_IO_Atoi((char *)pstCmd->cmdPara[0]);
    dstFps = SSOS_IO_Atoi((char *)pstCmd->cmdPara[1]);

    PTREE_MOD_TICK_SetRateFps(pstModObj, srcFps, dstFps);

    PTREE_DBG("SetRateFps  srcFps :%d ,dstFps %d, paraCnt %d\n", srcFps, dstFps, paraCnt);
    return 0;
}

int PTREE_CMD_TICK_SetDestFps(PTREE_MOD_Obj_t *pstModObj, const PTREE_CMD_Args_t *pstCmd, unsigned int paraCnt)
{
    unsigned int dstFps = 0;

    dstFps = SSOS_IO_Atoi((const char *)pstCmd->cmdPara[0]);

    PTREE_MOD_TICK_SetDestFps(pstModObj, dstFps);

    PTREE_DBG("SetDestFps  dstFps :%d, paraCnt %d\n", dstFps, paraCnt);
    return 0;
}

int PTREE_CMD_TICK_SetLeftFrameWrite(PTREE_MOD_Obj_t *pstModObj, const PTREE_CMD_Args_t *pstCmd, unsigned int paraCnt)
{
    unsigned int left = 0;

    left = SSOS_IO_Atoi((const char *)pstCmd->cmdPara[0]);

    PTREE_MOD_TICK_SetLeftFrame(pstModObj, left);

    PTREE_DBG("SetLeftFrameWrite  left :%d , paraCnt %d\n", left, paraCnt);
    return 0;
}

int PTREE_CMD_TICK_WaitFrame(PTREE_MOD_Obj_t *pstModObj, const PTREE_CMD_Args_t *pstCmd, unsigned int paraCnt)
{
    unsigned int timeOut = 0;
    unsigned int ret     = 0;

    timeOut = SSOS_IO_Atoi((const char *)pstCmd->cmdPara[0]);

    ret = PTREE_MOD_TICK_WaitFrame(pstModObj, timeOut);
    if (!ret)
    {
        PTREE_ERR("Wait timeout or error!, timeOut :%d, paraCnt %d\n", timeOut, paraCnt);
        return -1;
    }
    return 0;
}

PTREE_MAKER_CMD_INIT(TICK, PTREE_CMD_StrCompare, PTREE_CMD_StrHashVal,
                     {(unsigned long)"set_block_time", PTREE_CMD_TICK_SetBlockTime, 2},
                     {(unsigned long)"set_rate_fps", PTREE_CMD_TICK_SetRateFps, 2},
                     {(unsigned long)"set_dest_fps", PTREE_CMD_TICK_SetDestFps, 1},
                     {(unsigned long)"set_left_frame_write", PTREE_CMD_TICK_SetLeftFrameWrite, 1},
                     {(unsigned long)"wait_frame", PTREE_CMD_TICK_WaitFrame, 1})
