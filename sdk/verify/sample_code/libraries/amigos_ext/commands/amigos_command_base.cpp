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
#include "amigos_command.h"
#include "amigos_module_base.h"

static int WhoAmI(vector<string> &in_strs)
{
    AmigosModuleBase *pBaseClass = (AmigosModuleBase *)ss_cmd_base::get_private();

    if (!pBaseClass)
    {
        sslog.lv_set(PRINT_LV_ERROR) << "Class NULL" << COLOR_ENDL;
        return -1;
    }
    const AmigosSurfaceBase::ModInfo &stInfo = pBaseClass->GetSurface()->GetModInfo();
    sslog.color_set(PRINT_COLOR_GREEN, PRINT_MODE_HIGHTLIGHT)
        << "SEC:" << stInfo.sectionName << ", MOD:" << pBaseClass->GetModId()
        << ", DEV:" << stInfo.devId << ", CHN:" << stInfo.chnId << COLOR_ENDL;
    return 0;
}

static int LeaveObj(vector<string> &in_strs)
{
    ss_cmd_base::set_ext_data(NULL, NULL);
    for (auto it = ss_cmd_base::get_data().begin(); it != ss_cmd_base::get_data().end(); ++it)
    {
        sslog.store_tab() << it->first;
        sslog.store_tab() << "/";
    }
    return 0;
}

static int DumpHelpSelf(vector<string> &in_strs)
{
    std::string modName;
    AmigosModuleBase *pBaseClass = (AmigosModuleBase *)ss_cmd_base::get_private();

    if (!pBaseClass)
    {
        sslog.lv_set(PRINT_LV_ERROR) << "Class NULL" << COLOR_ENDL;
        return -1;
    }
    modName = pBaseClass->GetSurface()->GetDbIns()->GetMod<std::string>("MOD");
    AmigosCommand *pDstClassCmd = AmigosCommand::GetObj(modName);
    if (!pDstClassCmd)
    {
        sslog.color_set(PRINT_COLOR_RED, PRINT_MODE_HIGHTLIGHT) << "Mod ["<< modName << "] cmd did not create!"<< std::endl << PRINT_COLOR_END;
        return -1;
    }
    const std::set<mod_init_func> &setInitFunc = pDstClassCmd->GetModFunc();
    for (auto it = setInitFunc.begin(); it != setInitFunc.end(); it++)
    {
        map<string, struct base_command_data> cmdData;
        (*it)(cmdData);
        for (auto itCmd = cmdData.begin(); itCmd != cmdData.end(); ++itCmd)
        {
            sslog << itCmd->second.help;
        }
    }
    return 0;
}

MOD_CMDS(AmiCmdBase)
{
    ADD_CMD("dump_help_self", DumpHelpSelf, 0);
    ADD_CMD_HELP("dump_help_self", "No argument", "Dump help commands in current object.");
    ADD_CMD("leave_obj", LeaveObj, 0);
    ADD_CMD_HELP("leave_obj", "No argument", "Leave current object.");
    ADD_CMD("who_am_i", WhoAmI, 0);
}
