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
#include "amigos_module_osd.h"

static int HelloWorld(vector<string> &in_strs)
{
    AmigosModuleOsd *pMyClass = GET_PRIVATE_CLASS_OBJ(AmigosModuleOsd, AmigosModuleBase);
    sslog << "MY MOD ID IS: " << pMyClass->GetModId() << std::endl;
    return 0;
}

MOD_CMDS(AmiCmdOsd)
{
    ADD_CMD("hello_world", HelloWorld, 0);
    ADD_CMD_HELP("hello_world", "No argument", "Test!");
}
