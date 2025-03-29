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

#include "mi_common.h"
#include "mi_common_datatype.h"
#include "ptree_log.h"
#include "ssos_mem.h"
#include "ptree_mod.h"
#include "ptree_cmd.h"
#include "ptree_sur_ai.h"
#include "ssos_list.h"
#include "mi_ai.h"
#include "mi_ai_datatype.h"
#include "ptree_maker.h"

int PTREE_CMD_AI_StrCmdGetIfGain(PTREE_MOD_Obj_t *pstModObj, const PTREE_CMD_Args_t *pstCmd, unsigned int paraCnt)
{
    PTREE_SUR_AI_Info_t *info = NULL;
    signed short         s16LeftIfGain, s16RightIfGain;

    PTREE_DBG("Mod %s, Sec %s, Dev %d, Chn %d, In %d, Out %d", pstModObj->info->typeName, pstModObj->info->sectionName,
              pstModObj->info->devId, pstModObj->info->chnId, pstModObj->info->inCnt, pstModObj->info->outCnt);
    PTREE_DBG("Cmd Str %s, ParaCnt %d", (const char *)pstCmd->cmdId, paraCnt);

    info = CONTAINER_OF(pstModObj->info, PTREE_SUR_AI_Info_t, base.base);
    if (MI_SUCCESS != MI_AI_GetIfGain(info->enAiIf[0], &s16LeftIfGain, &s16RightIfGain))
    {
        PTREE_ERR("Get interface%d gain failed", info->enAiIf[0]);
        return -1;
    }
    PTREE_DBG("Get interface%d gain successfully left gain %d, rights gain %d", info->enAiIf[0], s16LeftIfGain,
              s16RightIfGain);
    return 0;
}

int PTREE_CMD_AI_StrCmdSetIfGain(PTREE_MOD_Obj_t *pstModObj, const PTREE_CMD_Args_t *pstCmd, unsigned int paraCnt)
{
    PTREE_SUR_AI_Info_t *info           = NULL;
    signed short         s16LeftIfGain  = SSOS_IO_Atoi((const char *)pstCmd->cmdPara[0]);
    signed short         s16RightIfGain = SSOS_IO_Atoi((const char *)pstCmd->cmdPara[1]);

    PTREE_DBG("Mod %s, Sec %s, Dev %d, Chn %d, In %d, Out %d", pstModObj->info->typeName, pstModObj->info->sectionName,
              pstModObj->info->devId, pstModObj->info->chnId, pstModObj->info->inCnt, pstModObj->info->outCnt);
    PTREE_DBG("Cmd Str %s, ParaCnt %d, p0->%s, p1->%s", (const char *)pstCmd->cmdId, paraCnt,
              (const char *)pstCmd->cmdPara[0], (const char *)pstCmd->cmdPara[1]);

    info = CONTAINER_OF(pstModObj->info, PTREE_SUR_AI_Info_t, base.base);
    if (MI_SUCCESS != MI_AI_SetIfGain(info->enAiIf[0], s16LeftIfGain, s16RightIfGain))
    {
        PTREE_ERR("Set interface%d gain fail", info->enAiIf[0]);
        return -1;
    }
    PTREE_DBG("Set interface gain successfully, s16RightIfGain %d, s16LeftIfGain %d", s16RightIfGain, s16LeftIfGain);
    return 0;
}

int PTREE_CMD_AI_StrCmdSetIfMute(PTREE_MOD_Obj_t *pstModObj, const PTREE_CMD_Args_t *pstCmd, unsigned int paraCnt)
{
    PTREE_SUR_AI_Info_t *info           = NULL;
    signed short         s16LeftIfGain  = SSOS_IO_Atoi((const char *)pstCmd->cmdPara[0]);
    signed short         s16RightIfGain = SSOS_IO_Atoi((const char *)pstCmd->cmdPara[1]);

    PTREE_DBG("Mod %s, Sec %s, Dev %d, Chn %d, In %d, Out %d", pstModObj->info->typeName, pstModObj->info->sectionName,
              pstModObj->info->devId, pstModObj->info->chnId, pstModObj->info->inCnt, pstModObj->info->outCnt);
    PTREE_DBG("Cmd Str %s, ParaCnt %d, p0->%s, p1->%s", (const char *)pstCmd->cmdId, paraCnt,
              (const char *)pstCmd->cmdPara[0], (const char *)pstCmd->cmdPara[1]);

    info = CONTAINER_OF(pstModObj->info, PTREE_SUR_AI_Info_t, base.base);
    if (MI_SUCCESS != MI_AI_SetIfMute(info->enAiIf[0], s16LeftIfGain, s16RightIfGain))
    {
        PTREE_ERR("Set interface%d mute fail", info->enAiIf[0]);
        return -1;
    }
    PTREE_DBG("Set interface mute successfully");
    return 0;
}

int PTREE_CMD_AI_StrCmdSetMute(PTREE_MOD_Obj_t *pstModObj, const PTREE_CMD_Args_t *pstCmd, unsigned int paraCnt)
{
    PTREE_SUR_AI_Info_t *info                                 = NULL;
    unsigned int         bmute                                = SSOS_IO_Atoi((const char *)pstCmd->cmdPara[0]);
    unsigned char        arrymute[E_MI_AUDIO_SOUND_MODE_16CH] = {0};
    unsigned int         i;

    PTREE_DBG("Mod %s, Sec %s, Dev %d, Chn %d, In %d, Out %d", pstModObj->info->typeName, pstModObj->info->sectionName,
              pstModObj->info->devId, pstModObj->info->chnId, pstModObj->info->inCnt, pstModObj->info->outCnt);
    PTREE_DBG("Cmd Str %s, ParaCnt %d, p0->%s", (const char *)pstCmd->cmdId, paraCnt, (const char *)pstCmd->cmdPara[0]);

    info = CONTAINER_OF(pstModObj->info, PTREE_SUR_AI_Info_t, base.base);
    for (i = 0; i < info->eAiSoundMode; i++)
    {
        arrymute[i] = bmute;
    }

    if (MI_SUCCESS != MI_AI_SetMute((MI_AUDIO_DEV)info->base.base.devId, 0, arrymute, i))
    {
        PTREE_ERR("Set Mute Fail");
        return -1;
    }
    return 0;
}

int PTREE_CMD_AI_StrCmdSetGain(PTREE_MOD_Obj_t *pstModObj, const PTREE_CMD_Args_t *pstCmd, unsigned int paraCnt)
{
    PTREE_SUR_AI_Info_t *info                                 = NULL;
    signed short         gain                                 = SSOS_IO_Atoi((const char *)pstCmd->cmdPara[0]);
    signed short         arrygain[E_MI_AUDIO_SOUND_MODE_16CH] = {0};
    unsigned int         i;

    PTREE_DBG("Mod %s, Sec %s, Dev %d, Chn %d, In %d, Out %d", pstModObj->info->typeName, pstModObj->info->sectionName,
              pstModObj->info->devId, pstModObj->info->chnId, pstModObj->info->inCnt, pstModObj->info->outCnt);
    PTREE_DBG("Cmd Str %s, ParaCnt %d, p0->%s", (const char *)pstCmd->cmdId, paraCnt, (const char *)pstCmd->cmdPara[0]);

    info = CONTAINER_OF(pstModObj->info, PTREE_SUR_AI_Info_t, base.base);
    if (E_MI_AI_IF_ADC_AB != info->enAiIf[0])
    {
        PTREE_ERR("set_gain only support E_MI_AI_IF_ADC_AB(1), you used if %d", info->enAiIf[0]);
        return -1;
    }
    for (i = 0; i < info->eAiSoundMode; i++)
    {
        arrygain[i] = gain;
    }
    if (MI_SUCCESS != MI_AI_SetGain((MI_AUDIO_DEV)info->base.base.devId, 0, arrygain, i))
    {
        PTREE_ERR("Set gain fail");
        return -1;
    }
    return 0;
}

PTREE_MAKER_CMD_INIT(AI, PTREE_CMD_StrCompare, PTREE_CMD_StrHashVal,
                     {(unsigned long)"ai_get_if_gain", PTREE_CMD_AI_StrCmdGetIfGain, 0},
                     {(unsigned long)"ai_set_if_gain", PTREE_CMD_AI_StrCmdSetIfGain, 2},
                     {(unsigned long)"ai_set_if_mute", PTREE_CMD_AI_StrCmdSetIfMute, 2},
                     {(unsigned long)"ai_set_mute", PTREE_CMD_AI_StrCmdSetMute, 1},
                     {(unsigned long)"ai_set_gain", PTREE_CMD_AI_StrCmdSetGain, 1});
