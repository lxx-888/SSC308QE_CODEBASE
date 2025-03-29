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
#include "amigos_module_tick.h"

static int SetBlockTime(vector<string> &in_strs)
{
    AmigosModuleTick *pMyClass = GET_PRIVATE_CLASS_OBJ(AmigosModuleTick, AmigosModuleBase);
    if (!pMyClass)
    {
        ss_print(PRINT_LV_ERROR, "Not found this class's object\n");
        return -1;
    }
    pMyClass->SetBlockTime(ss_cmd_atoi(in_strs[1].c_str()), ss_cmd_atoi(in_strs[2].c_str()));
    return 0;
}
static int SetRateFps(vector<string> &in_strs)
{
    AmigosModuleTick *pMyClass = GET_PRIVATE_CLASS_OBJ(AmigosModuleTick, AmigosModuleBase);
    if (!pMyClass)
    {
        ss_print(PRINT_LV_ERROR, "Not found this class's object\n");
        return -1;
    }
    pMyClass->SetRateFps(ss_cmd_atoi(in_strs[1].c_str()), ss_cmd_atoi(in_strs[2].c_str()));
    return 0;
}
static int SetDestFps(vector<string> &in_strs)
{
    AmigosModuleTick *pMyClass = GET_PRIVATE_CLASS_OBJ(AmigosModuleTick, AmigosModuleBase);
    if (!pMyClass)
    {
        ss_print(PRINT_LV_ERROR, "Not found this class's object\n");
        return -1;
    }
    pMyClass->SetDestFps(ss_cmd_atoi(in_strs[1].c_str()));
    return 0;
}

MOD_CMDS(AmiCmdTick)
{
    ADD_CMD("set_block_time", SetBlockTime, 2);
    ADD_CMD_HELP("set_block_time", "[sec] [nsec]", "Set 'TICK' block mode's time.");
    ADD_CMD("set_rate_fps", SetRateFps, 2);
    ADD_CMD_HELP("set_rate_fps", "[srcfps] [destfps]", "Set 'TICK' rate fps for src and dest fps.");
    ADD_CMD("set_dest_fps", SetDestFps, 1);
    ADD_CMD_HELP("set_dest_fps", "[destfps]", "Set 'TICK' out fps.");
}
