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
#include "ptree_mod.h"
#include "ptree_maker.h"
#include "ptree_api_isp.h"
#include "mi_isp.h"
#include "isp/mi_isp_cus3a_api.h"

static int _PTREE_API_ISP_Cus3ASetAeParam(PTREE_MOD_Obj_t *pstModObj, const PTREE_CMD_Args_t *pstCmd,
                                          unsigned int paraCnt)
{
    PTREE_API_ISP_Cus3ASetAeParam_t *pstAeParam = (PTREE_API_ISP_Cus3ASetAeParam_t *)pstCmd->cmdPara[0];

    (void)paraCnt;
    if (!pstAeParam)
    {
        PTREE_ERR("cmd null!");
        return -1;
    }
    if (pstModObj->status < E_PTREE_MOD_STATUS_ACTIVATE)
    {
        PTREE_ERR("Mod %s, Sec %s, Dev %d, Chn %d, Module should init first", pstModObj->info->typeName,
                  pstModObj->info->sectionName, pstModObj->info->devId, pstModObj->info->chnId);
        return -1;
    }
    return MI_ISP_CUS3A_SetAeParam(pstModObj->info->devId, pstModObj->info->chnId, (CusAEResult_t *)pstAeParam);
}

PTREE_MAKER_API_INIT(ISP, NULL, NULL, {E_PTREE_API_ISP_CMD_CUS3A_SET_AE_PARAM, _PTREE_API_ISP_Cus3ASetAeParam, 1})
