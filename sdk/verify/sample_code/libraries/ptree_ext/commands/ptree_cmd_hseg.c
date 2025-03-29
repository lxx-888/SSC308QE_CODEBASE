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

#ifdef __KERNEL__

#else /* LINUX USER */

#include "mi_common.h"
#include "mi_common_datatype.h"
#include "ptree_log.h"
#include "ssos_mem.h"
#include "ptree_mod.h"
#include "ptree_cmd.h"
#include "ptree_mod_hseg.h"
#include "ssos_list.h"
#include "ptree_maker.h"

int PTREE_CMD_HSEG_SetMode(PTREE_MOD_Obj_t *pstModObj, const PTREE_CMD_Args_t *pstCmd, unsigned int paraCnt)
{
    UNUSED(paraCnt);
    if (strcmp("replace", (const char *)pstCmd->cmdPara[0]) && strcmp("blur", (const char *)pstCmd->cmdPara[0]))
    {
        PTREE_ERR("set mode: %s failed! only suport replace or blur", (const char *)pstCmd->cmdPara[0]);
        return -1;
    }
    PTREE_MOD_HSEG_SetCtrlParam((void *)pstModObj, (const char *)pstCmd->cmdPara[0], E_PTREE_MOD_HSEG_PARAM_TYPE_MODE);
    return 0;
}
int PTREE_CMD_HSEG_SetMaskOp(PTREE_MOD_Obj_t *pstModObj, const PTREE_CMD_Args_t *pstCmd, unsigned int paraCnt)
{
    UNUSED(paraCnt);
    if (strcmp("none", (const char *)pstCmd->cmdPara[0]) && strcmp("dilate", (const char *)pstCmd->cmdPara[0])
        && strcmp("erode", (const char *)pstCmd->cmdPara[0]))
    {
        PTREE_ERR("set mask op: %s failed! only suport none/dilate/erode", (const char *)pstCmd->cmdPara[0]);
        return -1;
    }
    PTREE_MOD_HSEG_SetCtrlParam((void *)pstModObj, (const char *)pstCmd->cmdPara[0], E_PTREE_MOD_HSEG_PARAM_TYPE_OP);
    return 0;
}
int PTREE_CMD_HSEG_SetThredHold(PTREE_MOD_Obj_t *pstModObj, const PTREE_CMD_Args_t *pstCmd, unsigned int paraCnt)
{
    UNUSED(paraCnt);
    int iParam = SSOS_IO_Atoi((const char *)pstCmd->cmdPara[0]);
    if (iParam < 0 || iParam > 255)
    {
        PTREE_ERR("set thredhold: %d failed! out of rang [0, 255]", iParam);
        return -1;
    }
    PTREE_MOD_HSEG_SetCtrlParam((void *)pstModObj, (const char *)pstCmd->cmdPara[0], E_PTREE_MOD_HSEG_PARAM_TYPE_THR);
    return 0;
}
int PTREE_CMD_HSEG_SetLevel(PTREE_MOD_Obj_t *pstModObj, const PTREE_CMD_Args_t *pstCmd, unsigned int paraCnt)
{
    UNUSED(paraCnt);
    int iParam = SSOS_IO_Atoi((const char *)pstCmd->cmdPara[0]);
    if (iParam < 0 || iParam > 255)
    {
        PTREE_ERR("set level: %d failed! out of rang [0, 255]", iParam);
        return -1;
    }
    PTREE_MOD_HSEG_SetCtrlParam((void *)pstModObj, (const char *)pstCmd->cmdPara[0], E_PTREE_MOD_HSEG_PARAM_TYPE_LV);
    return 0;
}
int PTREE_CMD_HSEG_SetStage(PTREE_MOD_Obj_t *pstModObj, const PTREE_CMD_Args_t *pstCmd, unsigned int paraCnt)
{
    UNUSED(paraCnt);
    int iParam = SSOS_IO_Atoi((const char *)pstCmd->cmdPara[0]);
    if (iParam < 1 || iParam > 15)
    {
        PTREE_ERR("set scaling stage: %d failed! out of rang [1, 15]", iParam);
        return -1;
    }
    PTREE_MOD_HSEG_SetCtrlParam((void *)pstModObj, (const char *)pstCmd->cmdPara[0], E_PTREE_MOD_HSEG_PARAM_TYPE_STAGE);
    return 0;
}

PTREE_MAKER_CMD_INIT(HSEG, PTREE_CMD_StrCompare, PTREE_CMD_StrHashVal,
                     {(unsigned long)"set_mode", PTREE_CMD_HSEG_SetMode, 1},
                     {(unsigned long)"set_maskop", PTREE_CMD_HSEG_SetMaskOp, 1},
                     {(unsigned long)"set_thredhold", PTREE_CMD_HSEG_SetThredHold, 1},
                     {(unsigned long)"set_level", PTREE_CMD_HSEG_SetLevel, 1},
                     {(unsigned long)"set_stage", PTREE_CMD_HSEG_SetStage, 1})

#endif
