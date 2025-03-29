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
#include "amigos_module_ao.h"
#include "mi_ao.h"
#include "mi_common_datatype.h"

int SetPause(vector<string> &in_strs)
{
    AmigosModuleAo *pMyClass = GET_PRIVATE_CLASS_OBJ(AmigosModuleAo, AmigosModuleMiBase);

    if(MI_SUCCESS != MI_AO_Pause((MI_AUDIO_DEV)pMyClass->GetDevId()))
    {
        sslog.lv_set(PRINT_LV_ERROR) << "Set AO Pause Fail" << std::endl << PRINT_COLOR_END;
        return -1;
    }

    return 0;
}

int SetResume(vector<string> &in_strs)
{
    AmigosModuleAo *pMyClass = GET_PRIVATE_CLASS_OBJ(AmigosModuleAo, AmigosModuleMiBase);

    if(MI_SUCCESS != MI_AO_Resume((MI_AUDIO_DEV)pMyClass->GetDevId()))
    {
        sslog.lv_set(PRINT_LV_ERROR) << "Set AO Resume Fail" << std::endl << PRINT_COLOR_END;
        return -1;
    }

    return 0;
}

int SetVolume(vector<string> &in_strs)
{
    AmigosModuleAo *pMyClass = GET_PRIVATE_CLASS_OBJ(AmigosModuleAo, AmigosModuleMiBase);
    signed short Volume = ss_cmd_atoi(in_strs[1].c_str());
    unsigned char Fading = ss_cmd_atoi(in_strs[2].c_str());

    if(Volume < -508 || Volume > 512)
    {
        sslog.lv_set(PRINT_LV_ERROR) << "Volume " << (int)Volume << "Over Range" << std::endl << PRINT_COLOR_END;
        return -1;
    }

    if(Fading < 0 || Fading > 7)
    {
        sslog.lv_set(PRINT_LV_ERROR) << "fading " << (int)Fading << "Over Range" << std::endl << PRINT_COLOR_END;
        return -1;
    }

    if(MI_SUCCESS != MI_AO_SetVolume((MI_AUDIO_DEV)pMyClass->GetDevId(), Volume, Volume, (MI_AO_GainFading_e)Fading))
    {
        sslog.lv_set(PRINT_LV_ERROR) << "Set AO Volume Fail" << std::endl << PRINT_COLOR_END;
        return -1;
    }

    return 0;
}

int SetAoMute(vector < string > & in_strs)
{
    AmigosModuleAo *pMyClass = GET_PRIVATE_CLASS_OBJ(AmigosModuleAo, AmigosModuleMiBase);
    unsigned char bmute = ss_cmd_atoi(in_strs[1].c_str());

    if(MI_SUCCESS != MI_AO_SetMute((MI_AUDIO_DEV)pMyClass->GetDevId(), bmute, bmute, E_MI_AO_GAIN_FADING_OFF))
    {
        sslog.lv_set(PRINT_LV_ERROR) << "Set AO Mute Mode/Unmute Fail" << std::endl << PRINT_COLOR_END;
        return -1;
    }

    return 0;
}

int SetAoIfMute(vector < string > & in_strs)
{
    AmigosModuleAo *pMyClass = GET_PRIVATE_CLASS_OBJ(AmigosModuleAo, AmigosModuleMiBase);
    unsigned char bmute = ss_cmd_atoi(in_strs[1].c_str());

    if(E_MI_AO_IF_DAC_AB != (pMyClass->enAoIf & E_MI_AO_IF_DAC_AB))
    {
        sslog.lv_set(PRINT_LV_ERROR) << "IF " << pMyClass->enAoIf << " not supprot set mute" << std::endl << PRINT_COLOR_END;
        return -1;
    }

    if(MI_SUCCESS != MI_AO_SetIfMute(E_MI_AO_IF_DAC_AB, bmute, bmute))
    {
        sslog.lv_set(PRINT_LV_ERROR) << "Set AO If Mute Mode/Unmute Fail" << std::endl << PRINT_COLOR_END;
        return -1;
    }

    return 0;
}
MOD_CMDS(AmiCmdAo)
{
    ADD_CMD("ao_pause_play", SetPause, 0);
    ADD_CMD_HELP("ao_pause_play", "No argument", "Set Ao Pause!");
    ADD_CMD("ao_resume_play", SetResume, 0);
    ADD_CMD_HELP("ao_resume_play", "No argument", "Set Ao Resume!");
    ADD_CMD("ao_set_volume", SetVolume, 2);
    ADD_CMD_HELP("ao_set_volume", "[volume] [fading]", "Set Ao volume, range [-60, 30]!");
    ADD_CMD("ao_set_mute", SetAoMute, 1);
    ADD_CMD_HELP("ao_set_mute", "[bmute]", "1 means mute, 0 means unmute!");
    ADD_CMD("ao_set_if_mute", SetAoIfMute, 1);
    ADD_CMD_HELP("ao_set_if_mute", "[bmute]", "1 means mute, 0 means unmute!");
}
