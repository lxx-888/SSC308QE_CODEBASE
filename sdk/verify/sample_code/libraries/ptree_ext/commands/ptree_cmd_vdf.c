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
#include "ssos_mem.h"
#include "ptree_log.h"
#include "ptree_mod.h"
#include "ptree_cmd.h"
#include "ptree_sur_vdf.h"
#include "ssos_list.h"
#include "mi_vdf.h"
#include "ptree_maker.h"

int PTREE_CMD_VDF_SetMdDynamicParam(PTREE_MOD_Obj_t *pstModObj, const PTREE_CMD_Args_t *pstCmd, unsigned int paraCnt)
{
    UNUSED(paraCnt);
    MI_VDF_ChnAttr_t stMdAttr;

    if (MI_SUCCESS != MI_VDF_GetChnAttr(pstModObj->info->chnId, &stMdAttr))
    {
        PTREE_DBG("set Md param failed! Get MD chn(%d) attr failed!", pstModObj->info->chnId);
        return -1;
    }
    stMdAttr.stMdAttr.stMdDynamicParamsIn.sensitivity = SSOS_IO_Atoi((const char *)pstCmd->cmdPara[0]);
    stMdAttr.stMdAttr.stMdDynamicParamsIn.learn_rate  = SSOS_IO_Atoi((const char *)pstCmd->cmdPara[1]);
    stMdAttr.stMdAttr.stMdDynamicParamsIn.md_thr      = SSOS_IO_Atoi((const char *)pstCmd->cmdPara[2]);
    if (MI_SUCCESS != MI_VDF_SetChnAttr(pstModObj->info->chnId, &stMdAttr))
    {
        PTREE_DBG("Set MD chn(%d) attr failed!", pstModObj->info->chnId);
        return -1;
    }
    return 0;
}

int PTREE_CMD_VDF_SetOdParam(PTREE_MOD_Obj_t *pstModObj, const PTREE_CMD_Args_t *pstCmd, unsigned int paraCnt)
{
    UNUSED(paraCnt);
    MI_VDF_ChnAttr_t stOdAttr;

    if (MI_SUCCESS != MI_VDF_GetChnAttr(pstModObj->info->chnId, &stOdAttr))
    {
        PTREE_DBG("set Md param failed! Get OD chn(%d) attr failed!", pstModObj->info->chnId);
        return -1;
    }
    if (E_MI_VDF_WORK_MODE_OD != stOdAttr.enWorkMode)
    {
        PTREE_DBG("set od param failed! this mod(%d) is not MD(%d)", stOdAttr.enWorkMode, E_MI_VDF_WORK_MODE_OD);
        return -1;
    }
    stOdAttr.stOdAttr.stOdDynamicParamsIn.thd_tamper     = SSOS_IO_Atoi((const char *)pstCmd->cmdPara[0]);
    stOdAttr.stOdAttr.stOdDynamicParamsIn.tamper_blk_thd = SSOS_IO_Atoi((const char *)pstCmd->cmdPara[1]);
    stOdAttr.stOdAttr.stOdDynamicParamsIn.min_duration   = SSOS_IO_Atoi((const char *)pstCmd->cmdPara[2]);
    if (MI_SUCCESS != MI_VDF_SetChnAttr(pstModObj->info->chnId, &stOdAttr))
    {
        PTREE_DBG("Set OD chn(%d) attr failed!", pstModObj->info->chnId);
        return -1;
    }
    return 0;
}

PTREE_MAKER_CMD_INIT(VDF, PTREE_CMD_StrCompare, PTREE_CMD_StrHashVal,
                     {(unsigned long)"set_md", PTREE_CMD_VDF_SetMdDynamicParam, 3},
                     {(unsigned long)"set_od", PTREE_CMD_VDF_SetOdParam, 3})
#endif
