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
#include "amigos_module_aec.h"

static int AecReset(vector<string> &in_strs)
{
    AmigosModuleAec *pMyClass = GET_PRIVATE_CLASS_OBJ(AmigosModuleAec, AmigosModuleBase);
    if(pMyClass == nullptr)
    {
        sslog.lv_set(PRINT_LV_ERROR) << "Aec Class has not access, address " << pMyClass << std::endl << PRINT_COLOR_END;
        return -1;
    }

    AudioAecInit aec_init;
    AudioAecConfig aec_config;
    memset(&aec_init, 0, sizeof(AudioAecInit));
    memset(&aec_config, 0, sizeof(AudioAecConfig));

    aec_init.farend_channel = aec_init.nearend_channel = ss_cmd_atoi(in_strs[1].c_str());
    aec_init.point_number = ss_cmd_atoi(in_strs[2].c_str());
    aec_init.sample_rate = (IAA_AEC_SAMPLE_RATE)ss_cmd_atoi(in_strs[3].c_str());

    aec_config.comfort_noise_enable = (IAA_AEC_BOOL)ss_cmd_atoi(in_strs[4].c_str());
    aec_config.delay_sample = (short)ss_cmd_atoi(in_strs[5].c_str());

    return pMyClass->AecReset(aec_init, aec_config);
}

static int SetAecBypass(vector<string> &in_strs)
{
    AmigosModuleAec *pMyClass = GET_PRIVATE_CLASS_OBJ(AmigosModuleAec, AmigosModuleBase);
    if(pMyClass == nullptr)
    {
        sslog.lv_set(PRINT_LV_ERROR) << "Aec Class has not access, address " << pMyClass << std::endl << PRINT_COLOR_END;
        return -1;
    }
    bool bypass = ss_cmd_atoi(in_strs[1].c_str());
    return pMyClass->SetByPass(bypass);
}
MOD_CMDS(AmiCmdAec)
{
    ADD_CMD("aec_reset", AecReset, 5);
    ADD_CMD_HELP("aec_reset", "[channel] [point_number] [sample_rate] [noise_enable] [delay_sample]",
                 "eg: aec_reset 1 128 8000 1 0");
    ADD_CMD("aec_bypass", SetAecBypass, 1);
    ADD_CMD_HELP("aec_bypass", "[bypass enable]",
                 "eg: aec_bypass 1");
}
