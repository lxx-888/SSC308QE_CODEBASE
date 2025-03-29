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
#include "amigos_module_audioalgo.h"


static int ApcReset(vector<string> &in_strs)
{
    AmigosModuleAudioAlgo *pMyClass = GET_PRIVATE_CLASS_OBJ(AmigosModuleAudioAlgo, AmigosModuleBase);
    ss_linker_apc *AlgoClass = dynamic_cast<ss_linker_apc*>(pMyClass->GetLinker("apc"));

    if(AlgoClass == nullptr)
    {
        sslog.lv_set(PRINT_LV_ERROR) << "Apc Class has not access, address " << AlgoClass << std::endl << PRINT_COLOR_END;
        return -1;
    }

    AudioProcessInit audio_process_init;
    AudioApcBufferConfig apc_switch;
    memset(&audio_process_init, 0, sizeof(audio_process_init));
    memset(&apc_switch, 0, sizeof(apc_switch));
    audio_process_init.channel = ss_cmd_atoi(in_strs[1].c_str());
    audio_process_init.point_number = ss_cmd_atoi(in_strs[2].c_str());
    audio_process_init.sample_rate = (IAA_APC_SAMPLE_RATE)ss_cmd_atoi(in_strs[3].c_str());

    apc_switch.anr_enable = ss_cmd_atoi(in_strs[4].c_str());
    apc_switch.eq_enable = ss_cmd_atoi(in_strs[5].c_str());
    apc_switch.agc_enable = ss_cmd_atoi(in_strs[6].c_str());

    return AlgoClass->Reset(audio_process_init, apc_switch);
}

static int AlgoByPass(vector<string> &in_strs)
{
    AmigosModuleAudioAlgo *pMyClass = GET_PRIVATE_CLASS_OBJ(AmigosModuleAudioAlgo, AmigosModuleBase);
    ss_linker_audioalgo *AlgoClass = dynamic_cast<ss_linker_audioalgo*>(pMyClass->GetLinker(in_strs[1]));

    if(AlgoClass == nullptr)
    {
        sslog.lv_set(PRINT_LV_ERROR) << "Audio Class has not access, address " << AlgoClass << std::endl << PRINT_COLOR_END;
        return -1;
    }

    bool bypass = ss_cmd_atoi(in_strs[2].c_str());
    AlgoClass->SetBypass(bypass);
    return 0;
}
MOD_CMDS(AmiCmdAudioAlgo)
{
    ADD_CMD("apc_reset", ApcReset, 6);
    ADD_CMD_HELP("apc_reset", "[channel] [point_number] [sample_rate] [anr_enable] [eq_enable] [agc_enable]",
                 "eg: apc_reset 1 128 8000 1 1 1");

    ADD_CMD("algo_bypass", AlgoByPass, 2);
    ADD_CMD_HELP("algo_bypass", "[algo] [bypass]",
                 "[algo]: sed, apc, anr, eq, agc, src",
                 "[bypass] 0 mean processed, 1 mean bypass",
                 "has audio data been processed through the algo algo");
}

