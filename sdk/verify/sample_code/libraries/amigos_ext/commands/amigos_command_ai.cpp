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
#include "ss_cmd_base.h"
#include "amigos_module_ai.h"
#include "mi_ai.h"

int GetIfGain(vector<string> &in_strs)
{
    AmigosModuleAi *pMyClass = GET_PRIVATE_CLASS_OBJ(AmigosModuleAi, AmigosModuleMiBase);
    MI_S16 s16LeftIfGain, s8RightIfGain;

    if(MI_SUCCESS != MI_AI_GetIfGain(pMyClass->GetAiIf(0), &s16LeftIfGain, &s8RightIfGain))
    {
        sslog.lv_set(PRINT_LV_ERROR) << "Get If" << pMyClass->GetAiIf(0) << "Gain fail" << std::endl << PRINT_COLOR_END;
        return -1;
    }
    else
    {
        sslog << "Get If" << pMyClass->GetAiIf(0) << " Left Gain: " << (short)s16LeftIfGain << ", Right Gain: " << (short)s8RightIfGain << std::endl;
    }

    return 0;
}

int SetIfGain(vector<string> &in_strs)
{
    AmigosModuleAi *pMyClass = GET_PRIVATE_CLASS_OBJ(AmigosModuleAi, AmigosModuleMiBase);
    MI_S16 s16LeftIfGain = ss_cmd_atoi(in_strs[1].c_str());
    MI_S16 s16RightIfGain = ss_cmd_atoi(in_strs[2].c_str());

    if(MI_SUCCESS != MI_AI_SetIfGain(pMyClass->GetAiIf(0), s16LeftIfGain, s16RightIfGain))
    {
        sslog.lv_set(PRINT_LV_ERROR) << "Set If" << pMyClass->GetAiIf(0) << "Gain fail" << std::endl << PRINT_COLOR_END;
        return -1;
    }

    return 0;
}

int SetAiIfMute(vector<string> &in_strs)
{
    AmigosModuleAi *pMyClass = GET_PRIVATE_CLASS_OBJ(AmigosModuleAi, AmigosModuleMiBase);
    bool bLiftIfMute = ss_cmd_atoi(in_strs[1].c_str());
    bool bRightIfMute = ss_cmd_atoi(in_strs[2].c_str());

    if(MI_SUCCESS != MI_AI_SetIfMute(pMyClass->GetAiIf(0), bLiftIfMute, bRightIfMute))
    {
        sslog.lv_set(PRINT_LV_ERROR) << "Set IF Mute Fail" << std::endl << PRINT_COLOR_END;
        return -1;
    }

    return 0;
}

int SetAiMute(vector<string> &in_strs)
{
    AmigosModuleAi *pMyClass = GET_PRIVATE_CLASS_OBJ(AmigosModuleAi, AmigosModuleMiBase);
    unsigned char bmute = ss_cmd_atoi(in_strs[1].c_str());
    unsigned char arrymute[pMyClass->GetAiSoundMod()] = {0};

    for(unsigned int i = 0; i < pMyClass->GetAiSoundMod(); i++)
    {
        arrymute[i] = bmute;
    }

    if(MI_SUCCESS != MI_AI_SetMute((MI_AUDIO_DEV)pMyClass->GetDevId(), 0, arrymute, sizeof(arrymute)))
    {
        sslog.lv_set(PRINT_LV_ERROR) << "Set Mute Fail" << std::endl << PRINT_COLOR_END;
        return -1;
    }

    return 0;
}

int SetGain(vector<string> &in_strs)
{
    AmigosModuleAi *pMyClass = GET_PRIVATE_CLASS_OBJ(AmigosModuleAi, AmigosModuleMiBase);
    signed short Gain = ss_cmd_atoi(in_strs[1].c_str());
    signed short arrygain[pMyClass->GetAiSoundMod()] = {0};
    unsigned int i = 0;
    if(E_MI_AI_IF_ADC_AB != pMyClass->GetAiIf(0))
    {
        sslog.lv_set(PRINT_LV_ERROR) << "set_gain only support E_MI_AI_IF_ADC_AB(1), you used if " << pMyClass->GetAiIf(0) << std::endl << PRINT_COLOR_END;
        return -1;
    }

    for(i = 0; i < pMyClass->GetAiSoundMod(); i++)
    {
        arrygain[i] = Gain;
    }

    if(MI_SUCCESS != MI_AI_SetGain((MI_AUDIO_DEV)pMyClass->GetDevId(), 0, arrygain, i))
    {
        sslog.lv_set(PRINT_LV_ERROR) << "Set Gain Fail" << std::endl << PRINT_COLOR_END;
        return -1;
    }

    return 0;
}
MOD_CMDS(AmiCmdAi)
{
    ADD_CMD("ai_set_if_gain", SetIfGain, 2);
    ADD_CMD_HELP("ai_set_if_gain", "[LeftIfGain] [RightIfGain]",
                 "ADC_AB/ADC_CD value rang [0, 19];DMIC_A_01/DMIC_A_23 value rang [0,6]!");
    ADD_CMD("ai_get_if_gain", GetIfGain, 0);
    ADD_CMD_HELP("ai_get_if_gain", "No argument", "Get interface gain value!");
    ADD_CMD("ai_set_if_mute", SetAiIfMute, 2);
    ADD_CMD_HELP("ai_set_if_mute", "[LeftIfMute] [RightIfMute]", "1 means mute, 0 means unmute!");
    ADD_CMD("ai_set_mute", SetAiMute, 1);
    ADD_CMD_HELP("ai_set_mute", "[bMute]", "1 means mute, 0 means unmute!");
    ADD_CMD("ai_set_gain", SetGain, 1);
    ADD_CMD_HELP("ai_set_gain", "[Gain]", "Gain value range [-60, 30]");
}
