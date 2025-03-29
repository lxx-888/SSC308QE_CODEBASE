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
#include "ptree_api_vif.h"
#include "mi_vif.h"

int PTREE_API_VIF_SetShutterGain(PTREE_MOD_Obj_t *pstModObj, const PTREE_CMD_Args_t *pstCmd, unsigned int paraCnt)
{
    PTREE_API_VIF_SetShutterGain_t *pstShutterGain = (PTREE_API_VIF_SetShutterGain_t *)pstCmd->cmdPara[0];

    (void)paraCnt;
    if (pstModObj->status < E_PTREE_MOD_STATUS_ACTIVATE)
    {
        PTREE_ERR("Mod %s, Sec %s, Dev %d, Chn %d, Module should init first", pstModObj->info->typeName,
                  pstModObj->info->sectionName, pstModObj->info->devId, pstModObj->info->chnId);
        return -1;
    }
    return MI_VIF_CustFunction(pstModObj->info->devId, E_MI_VIF_CUSTCMD_SHUTTER_GAIN_SET,
                               sizeof(MI_VIF_ShutterGainParams_t), (MI_VIF_ShutterGainParams_t *)pstShutterGain);
}
PTREE_MAKER_API_INIT(VIF, NULL, NULL, {E_PTREE_API_VIF_CMD_SET_SHUTTER_GAIN, PTREE_API_VIF_SetShutterGain, 1})
