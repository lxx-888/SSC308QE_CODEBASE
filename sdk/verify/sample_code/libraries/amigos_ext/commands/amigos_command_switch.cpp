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

#include "amigos_module_switch.h"
#include "ss_cmd_base.h"
#include "ss_log.h"

static int do_switch(vector<string> &in_strs)
{
    AmigosModuleSwitch *pMyClass = GET_PRIVATE_CLASS_OBJ(AmigosModuleSwitch, AmigosModuleBase);
    unsigned int src_port = ss_cmd_atoi(in_strs[1].c_str());
    int dst_port = ss_cmd_atoi(in_strs[2].c_str());
    if (!pMyClass->DoSwitch(src_port, dst_port))
    {
        ss_print(PRINT_LV_ERROR, "do_switch %d -> %d is failed\n", src_port, dst_port);
        return -1;
    }
    return 0;
}

MOD_CMDS(AmiCmdSwitch)
{
    ADD_CMD("do_switch", do_switch, 2);
    ADD_CMD_HELP("do_switch", "[src_port] [dst_port]",
            "Do switch action, link src_port to dst_port",
            "[src_port] : input port id of switch module",
            "[dst_port] : Dst ouput port id of switch module, -1 means no link");
}
