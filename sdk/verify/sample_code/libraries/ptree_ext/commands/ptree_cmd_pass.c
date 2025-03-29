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
#include "ptree_mod_pass.h"
#include "ptree_maker.h"

int PTREE_CMD_PASS_CreateDelayPass(PTREE_MOD_Obj_t *pstModObj, const PTREE_CMD_Args_t *pstCmd, unsigned int paraCnt)
{
    unsigned int        portId = (unsigned int)SSOS_IO_Atoi((const char *)pstCmd->cmdPara[0]);
    PTREE_MOD_OutObj_t *modOut = PTREE_MOD_GetOutObjByPort(pstModObj, portId);
    (void)paraCnt;
    return PTREE_MOD_PASS_BuildDelayPass(modOut);
}
int PTREE_CMD_PASS_DestroyDelayPass(PTREE_MOD_Obj_t *pstModObj, const PTREE_CMD_Args_t *pstCmd, unsigned int paraCnt)
{
    unsigned int        portId = (unsigned int)SSOS_IO_Atoi((const char *)pstCmd->cmdPara[0]);
    PTREE_MOD_OutObj_t *modOut = PTREE_MOD_GetOutObjByPort(pstModObj, portId);
    (void)paraCnt;
    return PTREE_MOD_PASS_DeleteDelayPass(modOut);
}
int PTREE_CMD_PASS_InitDelayPass(PTREE_MOD_Obj_t *pstModObj, const PTREE_CMD_Args_t *pstCmd, unsigned int paraCnt)
{
    unsigned int        portId  = (unsigned int)SSOS_IO_Atoi((const char *)pstCmd->cmdPara[0]);
    unsigned int        rootIdx = (unsigned int)SSOS_IO_Atoi((const char *)pstCmd->cmdPara[1]);
    PTREE_MOD_OutObj_t *modOut  = PTREE_MOD_GetOutObjByPort(pstModObj, portId);
    (void)paraCnt;
    return PTREE_MOD_PASS_InitDelayPass(modOut, rootIdx);
}
int PTREE_CMD_PASS_DeinitDelayPass(PTREE_MOD_Obj_t *pstModObj, const PTREE_CMD_Args_t *pstCmd, unsigned int paraCnt)
{
    unsigned int        portId  = (unsigned int)SSOS_IO_Atoi((const char *)pstCmd->cmdPara[0]);
    unsigned int        rootIdx = (unsigned int)SSOS_IO_Atoi((const char *)pstCmd->cmdPara[1]);
    PTREE_MOD_OutObj_t *modOut  = PTREE_MOD_GetOutObjByPort(pstModObj, portId);
    (void)paraCnt;
    return PTREE_MOD_PASS_DeinitDelayPass(modOut, rootIdx);
}
int PTREE_CMD_PASS_BindDelayPass(PTREE_MOD_Obj_t *pstModObj, const PTREE_CMD_Args_t *pstCmd, unsigned int paraCnt)
{
    unsigned int        portId  = (unsigned int)SSOS_IO_Atoi((const char *)pstCmd->cmdPara[0]);
    unsigned int        rootIdx = (unsigned int)SSOS_IO_Atoi((const char *)pstCmd->cmdPara[1]);
    PTREE_MOD_OutObj_t *modOut  = PTREE_MOD_GetOutObjByPort(pstModObj, portId);
    (void)paraCnt;
    return PTREE_MOD_PASS_BindDelayPass(modOut, rootIdx);
}
int PTREE_CMD_PASS_UnbindDelayPass(PTREE_MOD_Obj_t *pstModObj, const PTREE_CMD_Args_t *pstCmd, unsigned int paraCnt)
{
    unsigned int        portId  = (unsigned int)SSOS_IO_Atoi((const char *)pstCmd->cmdPara[0]);
    unsigned int        rootIdx = (unsigned int)SSOS_IO_Atoi((const char *)pstCmd->cmdPara[1]);
    PTREE_MOD_OutObj_t *modOut  = PTREE_MOD_GetOutObjByPort(pstModObj, portId);
    (void)paraCnt;
    return PTREE_MOD_PASS_UnbindDelayPass(modOut, rootIdx);
}
int PTREE_CMD_PASS_StartDelayPass(PTREE_MOD_Obj_t *pstModObj, const PTREE_CMD_Args_t *pstCmd, unsigned int paraCnt)
{
    unsigned int        portId  = (unsigned int)SSOS_IO_Atoi((const char *)pstCmd->cmdPara[0]);
    unsigned int        rootIdx = (unsigned int)SSOS_IO_Atoi((const char *)pstCmd->cmdPara[1]);
    PTREE_MOD_OutObj_t *modOut  = PTREE_MOD_GetOutObjByPort(pstModObj, portId);
    (void)paraCnt;
    return PTREE_MOD_PASS_StartDelayPass(modOut, rootIdx);
}
int PTREE_CMD_PASS_StopDelayPass(PTREE_MOD_Obj_t *pstModObj, const PTREE_CMD_Args_t *pstCmd, unsigned int paraCnt)
{
    unsigned int        portId  = (unsigned int)SSOS_IO_Atoi((const char *)pstCmd->cmdPara[0]);
    unsigned int        rootIdx = (unsigned int)SSOS_IO_Atoi((const char *)pstCmd->cmdPara[1]);
    PTREE_MOD_OutObj_t *modOut  = PTREE_MOD_GetOutObjByPort(pstModObj, portId);
    (void)paraCnt;
    return PTREE_MOD_PASS_StopDelayPass(modOut, rootIdx);
}
PTREE_MAKER_CMD_INIT(PASS, PTREE_CMD_StrCompare, PTREE_CMD_StrHashVal,
                     {(unsigned long)"create", PTREE_CMD_PASS_CreateDelayPass, 1},
                     {(unsigned long)"destroy", PTREE_CMD_PASS_DestroyDelayPass, 1},
                     {(unsigned long)"init", PTREE_CMD_PASS_InitDelayPass, 2},
                     {(unsigned long)"deinit", PTREE_CMD_PASS_DeinitDelayPass, 2},
                     {(unsigned long)"bind", PTREE_CMD_PASS_BindDelayPass, 2},
                     {(unsigned long)"unbind", PTREE_CMD_PASS_UnbindDelayPass, 2},
                     {(unsigned long)"start", PTREE_CMD_PASS_StartDelayPass, 2},
                     {(unsigned long)"stop", PTREE_CMD_PASS_StopDelayPass, 2})
