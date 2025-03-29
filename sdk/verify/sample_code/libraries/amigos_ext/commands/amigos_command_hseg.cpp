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
#include "amigos_module_hseg.h"

static int ShowModuleFunction(vector<string> &in_strs)
{
    sslog.color_set(PRINT_COLOR_GREEN, PRINT_MODE_HIGHTLIGHT)
            << "this module use for bgblur" << COLOR_ENDL;
    return 0;
}

static int set_mode(vector<string> &in_strs)
{
    AmigosModuleHseg *pMyClass  = GET_PRIVATE_CLASS_OBJ(AmigosModuleHseg, AmigosModuleBase);
    if ("replace" != in_strs[1] && "blur" != in_strs[1])
    {
        AMIGOS_ERR("set mode: %s failed! only suport replace or blur\n", in_strs[1].c_str());
        return -1;
    }
#if defined(LINUX_FLOW_ON_DUAL_OS)
    string strCmd = "set_mode " + in_strs[1];
    if (pMyClass->IsPreload())
    {
        pMyClass->DoDualOsModCli(strCmd);
        return 0;
    }
#endif
    pMyClass->SetCtrlParam(in_strs[1], AmigosModuleHseg::E_MOD_HSEG_PARAM_TYPE_MODE);
    return 0;
}
static int set_maskop(vector<string> &in_strs)
{
    AmigosModuleHseg *pMyClass  = GET_PRIVATE_CLASS_OBJ(AmigosModuleHseg, AmigosModuleBase);
    if ("none" != in_strs[1] && "dilate" != in_strs[1] && "erode" != in_strs[1])
    {
        AMIGOS_ERR("set mask op: %s failed! only suport none/dilate/erode\n", in_strs[1].c_str());
        return -1;
    }
#if defined(LINUX_FLOW_ON_DUAL_OS)
    string strCmd = "set_maskop " + in_strs[1];
    if (pMyClass->IsPreload())
    {
        pMyClass->DoDualOsModCli(strCmd);
        return 0;
    }
#endif
    pMyClass->SetCtrlParam(in_strs[1], AmigosModuleHseg::E_MOD_HSEG_PARAM_TYPE_OP);
    return 0;
}
static int set_thredhold(vector<string> &in_strs)
{
    AmigosModuleHseg *pMyClass  = GET_PRIVATE_CLASS_OBJ(AmigosModuleHseg, AmigosModuleBase);
    int iParam = ss_cmd_atoi(in_strs[1].c_str());
    if (iParam < 0 || iParam > 255)
    {
        AMIGOS_ERR("set thredhold: %d failed! out of rang [0, 255]\n", in_strs[1].c_str());
        return -1;
    }
#if defined(LINUX_FLOW_ON_DUAL_OS)
    string strCmd = "set_thredhold " + in_strs[1];
    if (pMyClass->IsPreload())
    {
        pMyClass->DoDualOsModCli(strCmd);
        return 0;
    }
#endif
    pMyClass->SetCtrlParam(in_strs[1], AmigosModuleHseg::E_MOD_HSEG_PARAM_TYPE_THR);
    return 0;
}
static int set_level(vector<string> &in_strs)
{
    AmigosModuleHseg *pMyClass  = GET_PRIVATE_CLASS_OBJ(AmigosModuleHseg, AmigosModuleBase);
    int iParam = ss_cmd_atoi(in_strs[1].c_str());
    if (iParam < 0 || iParam > 255)
    {
        AMIGOS_ERR("set level: %d failed! out of rang [0, 255]\n", in_strs[1].c_str());
        return -1;
    }
#if defined(LINUX_FLOW_ON_DUAL_OS)
    string strCmd = "set_level " + in_strs[1];
    if (pMyClass->IsPreload())
    {
        pMyClass->DoDualOsModCli(strCmd);
        return 0;
    }
#endif
    pMyClass->SetCtrlParam(in_strs[1], AmigosModuleHseg::E_MOD_HSEG_PARAM_TYPE_LV);
    return 0;
}
static int set_stage(vector<string> &in_strs)
{
    AmigosModuleHseg *pMyClass  = GET_PRIVATE_CLASS_OBJ(AmigosModuleHseg, AmigosModuleBase);
    int iParam = ss_cmd_atoi(in_strs[1].c_str());
    if (iParam < 1 || iParam > 15)
    {
        AMIGOS_ERR("set scaling stage: %d failed! out of rang [1, 15]\n", in_strs[1].c_str());
        return -1;
    }
#if defined(LINUX_FLOW_ON_DUAL_OS)
    string strCmd = "set_stage " + in_strs[1];
    if (pMyClass->IsPreload())
    {
        pMyClass->DoDualOsModCli(strCmd);
        return 0;
    }
#endif
    pMyClass->SetCtrlParam(in_strs[1], AmigosModuleHseg::E_MOD_HSEG_PARAM_TYPE_STAGE);
    return 0;
}

MOD_CMDS(AmiCmdHseg)
{
    ADD_CMD("doWhat", ShowModuleFunction, 0);
    ADD_CMD_HELP("doWhat", "No argument", "what can i do.");
    ADD_CMD("set_mode", set_mode, 1);
    ADD_CMD_HELP("set_mode", "[val]", "Set hseg mode such as replace or blur", "[val]: A string in [replace/blur]");
    ADD_CMD("set_maskop", set_maskop, 1);
    ADD_CMD_HELP("set_maskop", "[val]", "Set hseg preset", "[val]: A string in [none/dilate/erode]");
    ADD_CMD("set_thredhold", set_thredhold, 1);
    ADD_CMD_HELP("set_thredhold", "[val]", "Set image thredhold param", "[val]: A int in [0, 255]");
    ADD_CMD("set_level", set_level, 1);
    ADD_CMD_HELP("set_level", "[val]", "Set hseg level", "[val]: A int in [0, 255]");
    ADD_CMD("set_stage", set_stage, 1);
    ADD_CMD_HELP("set_stage", "[val]", "Set scaling stage", "[val]: A int in [0, 15]");
}
