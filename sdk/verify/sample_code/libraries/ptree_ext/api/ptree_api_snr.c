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

#include "ssos_list.h"
#include "ptree_log.h"
#include "ptree_mod.h"
#include "ptree_maker.h"
#include "ptree_sur_snr.h"
#include "ptree_api_snr.h"

static int _PTREE_API_SNR_SetConfig(PTREE_MOD_Obj_t *pstModObj, const PTREE_CMD_Args_t *pstCmd, unsigned int paraCnt)
{
    PTREE_SUR_SNR_Info_t *  snrInfo   = NULL;
    PTREE_API_SNR_Config_t *snrConfig = NULL;

    (void)paraCnt;
    snrConfig = (PTREE_API_SNR_Config_t *)pstCmd->cmdPara[0];
    if (!snrConfig)
    {
        PTREE_ERR("Null cmd");
        return -1;
    }
    if (pstModObj->status >= E_PTREE_MOD_STATUS_FINISH)
    {
        PTREE_ERR("Mod %s, Sec %s, Dev %d, Chn %d, Module status %d error", pstModObj->info->typeName,
                  pstModObj->info->sectionName, pstModObj->info->devId, pstModObj->info->chnId, pstModObj->status);
        return -1;
    }
    snrInfo                  = CONTAINER_OF(pstModObj->info, PTREE_SUR_SNR_Info_t, base.base);
    snrInfo->s32SensorId     = snrConfig->id;
    snrInfo->s32SensorRes    = snrConfig->res;
    snrInfo->s32SensorFps    = snrConfig->fps;
    snrInfo->s32SensorMirror = snrConfig->isMirror;
    snrInfo->s32SensorFlip   = snrConfig->isFlip;
    snrInfo->s32HdrType      = snrConfig->hdrType;
    return 0;
}
static int _PTREE_API_SNR_GetConfig(PTREE_MOD_Obj_t *pstModObj, const PTREE_CMD_Args_t *pstCmd, unsigned int paraCnt)
{
    PTREE_SUR_SNR_Info_t *  snrInfo   = NULL;
    PTREE_API_SNR_Config_t *snrConfig = NULL;

    (void)paraCnt;
    snrConfig = (PTREE_API_SNR_Config_t *)pstCmd->cmdPara[0];
    if (!snrConfig)
    {
        PTREE_ERR("Null cmd");
        return -1;
    }
    snrInfo             = CONTAINER_OF(pstModObj->info, PTREE_SUR_SNR_Info_t, base.base);
    snrConfig->id       = snrInfo->s32SensorId;
    snrConfig->res      = snrInfo->s32SensorRes;
    snrConfig->fps      = snrInfo->s32SensorFps;
    snrConfig->isMirror = snrInfo->s32SensorMirror;
    snrConfig->isFlip   = snrInfo->s32SensorFlip;
    snrConfig->hdrType  = snrInfo->s32HdrType;
    return 0;
}
PTREE_MAKER_API_INIT(SNR, NULL, NULL, {E_PTREE_API_SNR_CMD_SET_CONFIG, _PTREE_API_SNR_SetConfig, 1},
                     {E_PTREE_API_SNR_CMD_GET_CONFIG, _PTREE_API_SNR_GetConfig, 1})
