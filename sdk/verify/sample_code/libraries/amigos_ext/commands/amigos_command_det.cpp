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
#include "amigos_module_det.h"
#include "ss_log.h"

static int set_threshold(vector<string> &in_strs)
{
    AmigosModuleDet *pMyClass  = GET_PRIVATE_CLASS_OBJ(AmigosModuleDet, AmigosModuleBase);
    unsigned int     threshold = ss_cmd_atoi(in_strs[0].c_str());
    if (threshold >= 1000)
    {
        ss_print(PRINT_LV_ERROR, "threshold need in [0, 1000), but %d\n", threshold);
        return -1;
    }
    if (!pMyClass->SetThreshold(threshold))
    {
        ss_print(PRINT_LV_ERROR, "set_threshold %d failed\n", threshold);
        return -1;
    }
    ss_print(PRINT_LV_TRACE, "set_threshold %d success\n", threshold);
    return 0;
}

MOD_CMDS(AmiCmdDet)
{
    ADD_CMD("set_threshold", set_threshold, 1);
    ADD_CMD_HELP("set_threshold", "[val]", "Set threshold of algorithm", "[val]: A integer in [1, 1000)");
}
