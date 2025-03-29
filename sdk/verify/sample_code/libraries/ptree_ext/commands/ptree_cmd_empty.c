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

#include "ssos_mem.h"
#include "ptree_log.h"
#include "ptree_mod.h"
#include "ptree_cmd.h"
#include "ptree_maker.h"

int PTREE_CMD_EMPTY_CmdTest0(PTREE_MOD_Obj_t *pstModObj, const PTREE_CMD_Args_t *pstCmd, unsigned int paraCnt)
{
    PTREE_DBG("Mod %s, Sec %s, Dev %d, Chn %d, In %d, Out %d", pstModObj->info->typeName, pstModObj->info->sectionName,
              pstModObj->info->devId, pstModObj->info->chnId, pstModObj->info->inCnt, pstModObj->info->outCnt);
    PTREE_DBG("Cmd Id %ld, ParaCnt %d, p0->%ld, p1->%ld, p2-%ld", pstCmd->cmdId, paraCnt, pstCmd->cmdPara[0],
              pstCmd->cmdPara[1], pstCmd->cmdPara[2]);
    return 0;
}
int PTREE_CMD_EMPTY_CmdTest1(PTREE_MOD_Obj_t *pstModObj, const PTREE_CMD_Args_t *pstCmd, unsigned int paraCnt)
{
    PTREE_DBG("Mod %s, Sec %s, Dev %d, Chn %d, In %d, Out %d", pstModObj->info->typeName, pstModObj->info->sectionName,
              pstModObj->info->devId, pstModObj->info->chnId, pstModObj->info->inCnt, pstModObj->info->outCnt);
    PTREE_DBG("Cmd Id %ld, ParaCnt %d, p0->%ld, p1->%ld, p2-%ld", pstCmd->cmdId, paraCnt, pstCmd->cmdPara[0],
              pstCmd->cmdPara[1], pstCmd->cmdPara[2]);
    return 1;
}
int PTREE_CMD_EMPTY_CmdTest2(PTREE_MOD_Obj_t *pstModObj, const PTREE_CMD_Args_t *pstCmd, unsigned int paraCnt)
{
    PTREE_DBG("Mod %s, Sec %s, Dev %d, Chn %d, In %d, Out %d", pstModObj->info->typeName, pstModObj->info->sectionName,
              pstModObj->info->devId, pstModObj->info->chnId, pstModObj->info->inCnt, pstModObj->info->outCnt);
    PTREE_DBG("Cmd Id %ld, ParaCnt %d", pstCmd->cmdId, paraCnt);
    return 1;
}
int PTREE_CMD_EMPTY_StrCmdTest0(PTREE_MOD_Obj_t *pstModObj, const PTREE_CMD_Args_t *pstCmd, unsigned int paraCnt)
{
    PTREE_DBG("Mod %s, Sec %s, Dev %d, Chn %d, In %d, Out %d", pstModObj->info->typeName, pstModObj->info->sectionName,
              pstModObj->info->devId, pstModObj->info->chnId, pstModObj->info->inCnt, pstModObj->info->outCnt);
    PTREE_DBG("Cmd Str %s, ParaCnt %d, p0->%s, p1->%s, p2-%s", (const char *)pstCmd->cmdId, paraCnt,
              (const char *)pstCmd->cmdPara[0], (const char *)pstCmd->cmdPara[1], (const char *)pstCmd->cmdPara[2]);
    return 0;
}
int PTREE_CMD_EMPTY_StrCmdTest1(PTREE_MOD_Obj_t *pstModObj, const PTREE_CMD_Args_t *pstCmd, unsigned int paraCnt)
{
    PTREE_DBG("Mod %s, Sec %s, Dev %d, Chn %d, In %d, Out %d", pstModObj->info->typeName, pstModObj->info->sectionName,
              pstModObj->info->devId, pstModObj->info->chnId, pstModObj->info->inCnt, pstModObj->info->outCnt);
    PTREE_DBG("Cmd Str %s, ParaCnt %d, p0->%s, p1->%s, p2-%s, p3->%s", (const char *)pstCmd->cmdId, paraCnt,
              (const char *)pstCmd->cmdPara[0], (const char *)pstCmd->cmdPara[1], (const char *)pstCmd->cmdPara[2],
              (const char *)pstCmd->cmdPara[3]);
    return 1;
}
int PTREE_CMD_EMPTY_StrCmdTest2(PTREE_MOD_Obj_t *pstModObj, const PTREE_CMD_Args_t *pstCmd, unsigned int paraCnt)
{
    PTREE_DBG("Mod %s, Sec %s, Dev %d, Chn %d, In %d, Out %d", pstModObj->info->typeName, pstModObj->info->sectionName,
              pstModObj->info->devId, pstModObj->info->chnId, pstModObj->info->inCnt, pstModObj->info->outCnt);
    PTREE_DBG("Cmd Str %s, ParaCnt %d", (const char *)pstCmd->cmdId, paraCnt);
    return 1;
}

PTREE_MAKER_CMD_INIT(CMD_TEST, NULL, NULL, {0, PTREE_CMD_EMPTY_CmdTest0, 3}, {1, PTREE_CMD_EMPTY_CmdTest1, 3},
                     {2, PTREE_CMD_EMPTY_CmdTest2, 0})

PTREE_MAKER_CMD_INIT(STR_CMD_TEST, PTREE_CMD_StrCompare, PTREE_CMD_StrHashVal,
                     {(unsigned long)"cmd0", PTREE_CMD_EMPTY_StrCmdTest0, 3},
                     {(unsigned long)"cmd1", PTREE_CMD_EMPTY_StrCmdTest1, 4},
                     {(unsigned long)"cmd2", PTREE_CMD_EMPTY_StrCmdTest2, 0})

PTREE_MAKER_CMD_INIT(EMPTY, PTREE_CMD_StrCompare, PTREE_CMD_StrHashVal,
                     {(unsigned long)"cmd0", PTREE_CMD_EMPTY_StrCmdTest0, 3},
                     {(unsigned long)"cmd1", PTREE_CMD_EMPTY_StrCmdTest1, 4},
                     {(unsigned long)"cmd2", PTREE_CMD_EMPTY_StrCmdTest2, 0})

PTREE_MAKER_CMD_ADD(EMPTY0, {(unsigned long)"cmd3", PTREE_CMD_EMPTY_StrCmdTest0, 3},
                    {(unsigned long)"cmd4", PTREE_CMD_EMPTY_StrCmdTest1, 4},
                    {(unsigned long)"cmd5", PTREE_CMD_EMPTY_StrCmdTest2, 0})

PTREE_MAKER_CMD_ADD(EMPTY1, {(unsigned long)"cmd6", PTREE_CMD_EMPTY_StrCmdTest0, 3},
                    {(unsigned long)"cmd7", PTREE_CMD_EMPTY_StrCmdTest1, 4},
                    {(unsigned long)"cmd8", PTREE_CMD_EMPTY_StrCmdTest2, 0})

PTREE_MAKER_CMD_ADD(EMPTY2, {(unsigned long)"cmd9", PTREE_CMD_EMPTY_StrCmdTest0, 3},
                    {(unsigned long)"cmd10", PTREE_CMD_EMPTY_StrCmdTest1, 4},
                    {(unsigned long)"cmd11", PTREE_CMD_EMPTY_StrCmdTest2, 0})

PTREE_MAKER_CMD_ADD(EMPTY3, {(unsigned long)"cmd12", PTREE_CMD_EMPTY_StrCmdTest0, 3},
                    {(unsigned long)"cmd13", PTREE_CMD_EMPTY_StrCmdTest1, 4},
                    {(unsigned long)"cmd14", PTREE_CMD_EMPTY_StrCmdTest2, 0})

PTREE_MAKER_CMD_ADD(EMPTY4, {(unsigned long)"cmd15", PTREE_CMD_EMPTY_StrCmdTest0, 3},
                    {(unsigned long)"cmd16", PTREE_CMD_EMPTY_StrCmdTest1, 4},
                    {(unsigned long)"cmd17", PTREE_CMD_EMPTY_StrCmdTest2, 0})
