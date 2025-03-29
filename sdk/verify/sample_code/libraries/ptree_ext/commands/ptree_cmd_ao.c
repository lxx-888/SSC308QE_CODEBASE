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
#include "ptree_sur_ao.h"
#include "ssos_list.h"
#include "mi_ao.h"
#include "mi_ao_datatype.h"
#include "ptree_maker.h"

int PTREE_CMD_AO_StrCmdSetPause(PTREE_MOD_Obj_t *pstModObj, const PTREE_CMD_Args_t *pstCmd, unsigned int paraCnt)
{
    int dev = pstModObj->info->devId;

    PTREE_DBG("Mod %s, Sec %s, Dev %d, Chn %d, In %d, Out %d", pstModObj->info->typeName, pstModObj->info->sectionName,
              pstModObj->info->devId, pstModObj->info->chnId, pstModObj->info->inCnt, pstModObj->info->outCnt);
    PTREE_DBG("Cmd Str %s, ParaCnt %d", (const char *)pstCmd->cmdId, paraCnt);

    if (MI_SUCCESS != MI_AO_Pause((MI_AUDIO_DEV)dev))
    {
        PTREE_ERR("Set Ao Pause Fail");
        return -1;
    }
    return 0;
}

int PTREE_CMD_AO_StrCmdSetResume(PTREE_MOD_Obj_t *pstModObj, const PTREE_CMD_Args_t *pstCmd, unsigned int paraCnt)
{
    int dev = pstModObj->info->devId;

    PTREE_DBG("Mod %s, Sec %s, Dev %d, Chn %d, In %d, Out %d", pstModObj->info->typeName, pstModObj->info->sectionName,
              pstModObj->info->devId, pstModObj->info->chnId, pstModObj->info->inCnt, pstModObj->info->outCnt);
    PTREE_DBG("Cmd Str %s, ParaCnt %d", (const char *)pstCmd->cmdId, paraCnt);

    if (MI_SUCCESS != MI_AO_Resume((MI_AUDIO_DEV)dev))
    {
        PTREE_ERR("Set Ao Resume Fail");
        return -1;
    }
    return 0;
}

int PTREE_CMD_AO_StrCmdSetVolume(PTREE_MOD_Obj_t *pstModObj, const PTREE_CMD_Args_t *pstCmd, unsigned int paraCnt)
{
    int          dev    = pstModObj->info->devId;
    signed short volume = SSOS_IO_Atoi((const char *)pstCmd->cmdPara[0]);
    signed short fading = SSOS_IO_Atoi((const char *)pstCmd->cmdPara[1]);

    PTREE_DBG("Mod %s, Sec %s, Dev %d, Chn %d, In %d, Out %d", pstModObj->info->typeName, pstModObj->info->sectionName,
              pstModObj->info->devId, pstModObj->info->chnId, pstModObj->info->inCnt, pstModObj->info->outCnt);
    PTREE_DBG("Cmd Str %s, ParaCnt %d, p0->%s, p1->%s", (const char *)pstCmd->cmdId, paraCnt,
              (const char *)pstCmd->cmdPara[0], (const char *)pstCmd->cmdPara[1]);

    if (volume < -508 || volume > 512)
    {
        PTREE_ERR("volume %d is over range", volume);
        return -1;
    }

    if (fading < 0 || fading > 7)
    {
        PTREE_ERR("fading %d is over range", fading);
        return -1;
    }

    if (MI_SUCCESS != MI_AO_SetVolume((MI_AUDIO_DEV)dev, volume, volume, (MI_AO_GainFading_e)fading))
    {
        PTREE_ERR("set ao volume fail", fading);
        return -1;
    }

    return 0;
}

int PTREE_CMD_AO_StrCmdSetMute(PTREE_MOD_Obj_t *pstModObj, const PTREE_CMD_Args_t *pstCmd, unsigned int paraCnt)
{
    int           dev  = pstModObj->info->devId;
    unsigned char mute = SSOS_IO_Atoi((const char *)pstCmd->cmdPara[0]);

    PTREE_DBG("Mod %s, Sec %s, Dev %d, Chn %d, In %d, Out %d", pstModObj->info->typeName, pstModObj->info->sectionName,
              pstModObj->info->devId, pstModObj->info->chnId, pstModObj->info->inCnt, pstModObj->info->outCnt);
    PTREE_DBG("Cmd Str %s, ParaCnt %d, p0->%s", (const char *)pstCmd->cmdId, paraCnt, (const char *)pstCmd->cmdPara[0]);

    if (MI_SUCCESS != MI_AO_SetMute((MI_AUDIO_DEV)dev, mute, mute, E_MI_AO_GAIN_FADING_OFF))
    {
        PTREE_ERR("Set Ao %s Fail", mute ? "mute" : "unmute");
        return -1;
    }
    return 0;
}

int PTREE_CMD_AO_StrCmdSetIfMute(PTREE_MOD_Obj_t *pstModObj, const PTREE_CMD_Args_t *pstCmd, unsigned int paraCnt)
{
    PTREE_SUR_AO_Info_t *info = NULL;
    unsigned char        mute = SSOS_IO_Atoi((const char *)pstCmd->cmdPara[0]);

    PTREE_DBG("Mod %s, Sec %s, Dev %d, Chn %d, In %d, Out %d", pstModObj->info->typeName, pstModObj->info->sectionName,
              pstModObj->info->devId, pstModObj->info->chnId, pstModObj->info->inCnt, pstModObj->info->outCnt);
    PTREE_DBG("Cmd Str %s, ParaCnt %d, p0->%s", (const char *)pstCmd->cmdId, paraCnt, (const char *)pstCmd->cmdPara[0]);

    info = CONTAINER_OF(pstModObj->info, PTREE_SUR_AO_Info_t, base.base);
    if (E_MI_AO_IF_DAC_AB != (info->enAoIf & E_MI_AO_IF_DAC_AB))
    {
        return -1;
    }

    if (MI_SUCCESS != MI_AO_SetIfMute(E_MI_AO_IF_DAC_AB, mute, mute))
    {
        PTREE_ERR("Set Ao If %s Fail", mute ? "mute" : "unmute");
        return -1;
    }
    return 0;
}
PTREE_MAKER_CMD_INIT(AO, PTREE_CMD_StrCompare, PTREE_CMD_StrHashVal,
                     {(unsigned long)"ao_pause", PTREE_CMD_AO_StrCmdSetPause, 0},
                     {(unsigned long)"ao_resume", PTREE_CMD_AO_StrCmdSetResume, 0},
                     {(unsigned long)"ao_set_volume", PTREE_CMD_AO_StrCmdSetVolume, 2},
                     {(unsigned long)"ao_set_mute", PTREE_CMD_AO_StrCmdSetMute, 1},
                     {(unsigned long)"ao_set_if_mute", PTREE_CMD_AO_StrCmdSetIfMute, 1})
