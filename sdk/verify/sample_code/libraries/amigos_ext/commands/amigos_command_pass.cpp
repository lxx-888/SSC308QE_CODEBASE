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
#include "amigos_module_pass.h"

static int CreateDelayPass(vector<string> &in_strs)
{
    AmigosModulePass *pMyClass = GET_PRIVATE_CLASS_OBJ(AmigosModulePass, AmigosModuleBase);
    unsigned int portId  = ss_cmd_atoi(in_strs[1].c_str());
    return pMyClass->BuildDelayPass(portId);
}
static int DestroyDelayPass(vector<string> &in_strs)
{
    AmigosModulePass *pMyClass = GET_PRIVATE_CLASS_OBJ(AmigosModulePass, AmigosModuleBase);
    unsigned int portId  = ss_cmd_atoi(in_strs[1].c_str());
    return pMyClass->DeleteDelayPass(portId);
}

static int InitDelayPass(vector<string> &in_strs)
{
    AmigosModulePass *pMyClass = GET_PRIVATE_CLASS_OBJ(AmigosModulePass, AmigosModuleBase);
    unsigned int portId  = ss_cmd_atoi(in_strs[1].c_str());
    unsigned int rootIdx = ss_cmd_atoi(in_strs[2].c_str());

    return pMyClass->InitDelayPass(portId, rootIdx);
}

static int DeinitDelayPass(vector<string> &in_strs)
{
    AmigosModulePass *pMyClass = GET_PRIVATE_CLASS_OBJ(AmigosModulePass, AmigosModuleBase);
    unsigned int portId  = ss_cmd_atoi(in_strs[1].c_str());
    unsigned int rootIdx = ss_cmd_atoi(in_strs[2].c_str());

    return pMyClass->DeinitDelayPass(portId, rootIdx);
}

static int BindDelayPass(vector<string> &in_strs)
{
    AmigosModulePass *pMyClass = GET_PRIVATE_CLASS_OBJ(AmigosModulePass, AmigosModuleBase);
    unsigned int portId  = ss_cmd_atoi(in_strs[1].c_str());
    unsigned int rootIdx = ss_cmd_atoi(in_strs[2].c_str());

    return pMyClass->BindDelayPass(portId, rootIdx);
}

static int UnbindDelayPass(vector<string> &in_strs)
{
    AmigosModulePass *pMyClass = GET_PRIVATE_CLASS_OBJ(AmigosModulePass, AmigosModuleBase);
    unsigned int portId  = ss_cmd_atoi(in_strs[1].c_str());
    unsigned int rootIdx = ss_cmd_atoi(in_strs[2].c_str());

    return pMyClass->UnbindDelayPass(portId, rootIdx);
}

static int StartDelayPass(vector<string> &in_strs)
{
    AmigosModulePass *pMyClass = GET_PRIVATE_CLASS_OBJ(AmigosModulePass, AmigosModuleBase);
    unsigned int portId  = ss_cmd_atoi(in_strs[1].c_str());
    unsigned int rootIdx = ss_cmd_atoi(in_strs[2].c_str());

    return pMyClass->StartDelayPass(portId, rootIdx);
}

static int StopDelayPass(vector<string> &in_strs)
{
    AmigosModulePass *pMyClass = GET_PRIVATE_CLASS_OBJ(AmigosModulePass, AmigosModuleBase);
    unsigned int portId  = ss_cmd_atoi(in_strs[1].c_str());
    unsigned int rootIdx = ss_cmd_atoi(in_strs[2].c_str());

    return pMyClass->StopDelayPass(portId, rootIdx);
}
static int DisplayModules(vector<string> &in_strs)
{
    std::set<AmigosModuleBase *> allObjects;
    AmigosModulePass *pMyClass = GET_PRIVATE_CLASS_OBJ(AmigosModulePass, AmigosModuleBase);

    allObjects = pMyClass->GetAllObjectsInPass();
    for (auto &it : allObjects)
    {
        std::string sec = it->GetSurface()->GetModInfo().sectionName;
        sslog.color_set(PRINT_COLOR_GREEN, PRINT_MODE_HIGHTLIGHT) << "Section ["<< sec << "] in PASS" << COLOR_ENDL;
    }
    return 0;
}

MOD_CMDS(AmiCmdPass)
{
    ADD_CMD("create", CreateDelayPass, 1);
    ADD_CMD_HELP("create", "[output port]", "Create modules in 'PASS'.!");
    ADD_CMD("destroy", DestroyDelayPass, 1);
    ADD_CMD_HELP("destroy", "[output port]", "Destroy modules in 'PASS'.!");
    ADD_CMD_HELP("init", "[output port] [root idx]", "Trigger init flow in 'PASS'.!");
    ADD_CMD("init", InitDelayPass, 2);
    ADD_CMD_HELP("init", "[output port] [root idx]", "Trigger init flow in 'PASS'.!");
    ADD_CMD("deinit", DeinitDelayPass, 2);
    ADD_CMD_HELP("deinit", "[output port] [root idx]", "Trigger deinit flow in 'PASS'.!");
    ADD_CMD("bind", BindDelayPass, 2);
    ADD_CMD_HELP("bind", "[output port] [root idx]", "Trigger bind flow in 'PASS'.!");
    ADD_CMD("unbind", UnbindDelayPass, 2);
    ADD_CMD_HELP("unbind", "[output port] [root idx]", "Trigger unbind flow in 'PASS'.!");
    ADD_CMD("start", StartDelayPass, 2);
    ADD_CMD_HELP("start", "[output port] [root idx]", "Trigger start flow in 'PASS'.!");
    ADD_CMD("stop", StopDelayPass, 2);
    ADD_CMD_HELP("stop", "[output port] [root idx]", "Trigger stop flow in 'PASS'.!");
    ADD_CMD("display_modules", DisplayModules, 0);
    ADD_CMD_HELP("display_modules", "No argument", "Display all section name in 'PASS'");
}
