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

/*
 * The full name of 'ptree' is 'Pipeline tree', which use the idea of 'Amigos'
 * for reference and auther is 'pedro.peng' from Sigmastar.
 */
#include "ptree_log.h"
#include "ptree_api_aestable.h"
#include "ptree_mod.h"
#include "ptree_sur_aestable.h"
#include "ptree_mod_aestable.h"
#include "ssos_list.h"
#include "ptree_maker.h"

/**
 * @brief  set aestable run mode on surface
 * @note
 * @param[out]  pstCmd->cmdPara[0]: (PTREE_API_AESTABLE_RunModeParam_t *) modify aestable runmode parameter.
 * @retval 0: Success
 *        -1: fail
 */
static int _PTREE_API_AESTABLE_GetRunMode(PTREE_MOD_Obj_t *pstModObj, const PTREE_CMD_Args_t *pstCmd,
                                          unsigned int paraCnt)
{
    PTREE_SUR_AESTABLE_Info_t *        info  = NULL;
    PTREE_API_AESTABLE_RunModeParam_t *param = NULL;

    (void)paraCnt;
    info        = CONTAINER_OF(pstModObj->info, PTREE_SUR_AESTABLE_Info_t, base);
    param       = (PTREE_API_AESTABLE_RunModeParam_t *)pstCmd->cmdPara[0];
    param->mode = (enum PTREE_API_AESTABLE_RunMode_e)info->runMode;
    if (info->runMode == E_PTREE_SUR_AESTABLE_RUN_MODE_SHOT)
    {
        param->captureParam.caputreCount  = info->captureCount;
        param->captureParam.stableCount   = info->stableCount;
        param->captureParam.usingLowPower = info->usingLowPower;
        return 0;
    }
    if (info->runMode == E_PTREE_SUR_AESTABLE_RUN_MODE_RECORD)
    {
        param->recordParam.stableCount = info->stableCount;
        return 0;
    }
    PTREE_ERR("Error in run mode config, mode=%d\n", info->runMode);
    return -1;
}

/**
 * @brief  set aestable run mode on surface
 * @note
 * @param[in]  pstCmd->cmdPara[0]: (PTREE_API_AESTABLE_RunModeParam_t *) modify aestable runmode parameter.
 * @retval 0: Success
 *        -1: fail
 */
static int _PTREE_API_AESTABLE_SetRunMode(PTREE_MOD_Obj_t *pstModObj, const PTREE_CMD_Args_t *pstCmd,
                                          unsigned int paraCnt)
{
    PTREE_SUR_AESTABLE_Info_t *        info  = NULL;
    PTREE_API_AESTABLE_RunModeParam_t *param = NULL;

    (void)paraCnt;
    info          = CONTAINER_OF(pstModObj->info, PTREE_SUR_AESTABLE_Info_t, base);
    param         = (PTREE_API_AESTABLE_RunModeParam_t *)pstCmd->cmdPara[0];
    info->runMode = (enum PTREE_SUR_AESTABLE_RunMode_e)param->mode;
    if (info->runMode == E_PTREE_SUR_AESTABLE_RUN_MODE_SHOT)
    {
        info->captureCount  = param->captureParam.caputreCount;
        info->stableCount   = param->captureParam.stableCount;
        info->usingLowPower = param->captureParam.usingLowPower;
        return 0;
    }
    if (info->runMode == E_PTREE_SUR_AESTABLE_RUN_MODE_RECORD)
    {
        info->stableCount   = param->recordParam.stableCount;
        info->captureCount  = 0;
        info->usingLowPower = 0;
        return 0;
    }
    PTREE_ERR("Error in run mode config, mode=%d\n", info->runMode);
    return -1;
}

/**
 * @brief  set aestable stable mode on surface
 * @note
 * @param[in]  pstCmd->cmdPara[0]: (PTREE_API_AESTABLE_StartingMode_e) modify aestable stblemode.
 * @retval 0: Success
 *        -1: fail
 */
static int _PTREE_API_AESTABLE_SetStartMode(PTREE_MOD_Obj_t *pstModObj, const PTREE_CMD_Args_t *pstCmd,
                                            unsigned int paraCnt)
{
    PTREE_SUR_AESTABLE_Info_t *info = NULL;
    info                            = CONTAINER_OF(pstModObj->info, PTREE_SUR_AESTABLE_Info_t, base);
    info->startMode                 = (enum PTREE_API_AESTABLE_StartMode_e)pstCmd->cmdPara[0];
    (void)paraCnt;
    return 0;
}

PTREE_MAKER_API_INIT(AESTABLE, NULL, NULL,
                     {E_PTREE_API_AESTABLE_CMD_SET_START_MODE, _PTREE_API_AESTABLE_SetStartMode, 1},
                     {E_PTREE_API_AESTABLE_CMD_GET_RUN_MODE, _PTREE_API_AESTABLE_GetRunMode, 1},
                     {E_PTREE_API_AESTABLE_CMD_SET_RUN_MODE, _PTREE_API_AESTABLE_SetRunMode, 1});
