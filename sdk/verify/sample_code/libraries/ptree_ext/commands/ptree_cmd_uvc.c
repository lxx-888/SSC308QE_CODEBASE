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
#include "ssos_mem.h"
#include "ptree_log.h"
#include "ptree_mod.h"
#include "ptree_cmd.h"
#include "ptree_maker.h"

int PTREE_CMD_UVC_CmdTest(PTREE_MOD_Obj_t *pstModObj, const PTREE_CMD_Args_t *pstCmd, unsigned int paraCnt)
{
    PTREE_DBG("Mod %s, Sec %s, Dev %d, Chn %d, In %d, Out %d", pstModObj->info->typeName, pstModObj->info->sectionName,
              pstModObj->info->devId, pstModObj->info->chnId, pstModObj->info->inCnt, pstModObj->info->outCnt);
    PTREE_DBG("Cmd Id %ld, ParaCnt %d, p0->%ld, p1->%ld, p2-%ld", pstCmd->cmdId, paraCnt, pstCmd->cmdPara[0],
              pstCmd->cmdPara[1], pstCmd->cmdPara[2]);
    return 0;
}
// PTREE_MAKER_CMD_INIT(UVC, PTREE_CMD_StrCompare, PTREE_CMD_StrHashVal, {});
PTREE_MAKER_CMD_INIT(UVC, NULL, NULL, {0, PTREE_CMD_UVC_CmdTest, 3});
