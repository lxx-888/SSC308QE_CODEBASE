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
#include "amigos_module_ive.h"

static int ShowModuleFunction(vector<string> &in_strs)
{
    sslog.color_set(PRINT_COLOR_GREEN, PRINT_MODE_HIGHTLIGHT)
            << "this module use for ive" << COLOR_ENDL;
    return 0;
}

static int set_bgblur_mode(vector<string> &in_strs)
{
    AmigosModuleIve *pMyClass  = GET_PRIVATE_CLASS_OBJ(AmigosModuleIve, AmigosModuleMiBase);
    if ("replace" != in_strs[1] && "blur" != in_strs[1] && "mosaic" != in_strs[1] && "blur_mosaic" != in_strs[1])
    {
        AMIGOS_ERR("set mode: %s failed! only suport replace/blur/mosaic/blur_mosaic\n", in_strs[1].c_str());
        return -1;
    }
    pMyClass->SetCtrlParam(in_strs[1], AmigosModuleIve::LinkerBgBlurRepBgNegative::E_MOD_IVE_PARAM_TYPE_MODE);
    return 0;
}
static int set_bgblur_maskop(vector<string> &in_strs)
{
    AmigosModuleIve *pMyClass  = GET_PRIVATE_CLASS_OBJ(AmigosModuleIve, AmigosModuleMiBase);
    if ("none" != in_strs[1] && "dilate" != in_strs[1] && "erode" != in_strs[1])
    {
        AMIGOS_ERR("set mask op: %s failed! only suport none/dilate/erode\n", in_strs[1].c_str());
        return -1;
    }
    pMyClass->SetCtrlParam(in_strs[1], AmigosModuleIve::LinkerBgBlurRepBgNegative::E_MOD_IVE_PARAM_TYPE_OP);
    return 0;
}
static int set_bgblur_thredhold(vector<string> &in_strs)
{
    AmigosModuleIve *pMyClass  = GET_PRIVATE_CLASS_OBJ(AmigosModuleIve, AmigosModuleMiBase);
    int iParam = ss_cmd_atoi(in_strs[1].c_str());
    if (iParam < 0 || iParam > 255)
    {
        AMIGOS_ERR("set thredhold: %d failed! out of rang [0, 255]\n", in_strs[1].c_str());
        return -1;
    }
    pMyClass->SetCtrlParam(in_strs[1], AmigosModuleIve::LinkerBgBlurRepBgNegative::E_MOD_IVE_PARAM_TYPE_THR);
    return 0;
}
static int set_bgblur_level(vector<string> &in_strs)
{
    AmigosModuleIve *pMyClass  = GET_PRIVATE_CLASS_OBJ(AmigosModuleIve, AmigosModuleMiBase);
    int iParam = ss_cmd_atoi(in_strs[1].c_str());
    if (iParam < 0 || iParam > 255)
    {
        AMIGOS_ERR("set level: %d failed! out of rang [0, 255]\n", in_strs[1].c_str());
        return -1;
    }
    pMyClass->SetCtrlParam(in_strs[1], AmigosModuleIve::LinkerBgBlurRepBgNegative::E_MOD_IVE_PARAM_TYPE_LV);
    return 0;
}
static int set_bgblur_stage(vector<string> &in_strs)
{
    AmigosModuleIve *pMyClass  = GET_PRIVATE_CLASS_OBJ(AmigosModuleIve, AmigosModuleMiBase);
    int iParam = ss_cmd_atoi(in_strs[1].c_str());
    if (iParam < 1 || iParam > 15)
    {
        AMIGOS_ERR("set scaling stage: %d failed! out of rang [1, 15]\n", in_strs[1].c_str());
        return -1;
    }
    pMyClass->SetCtrlParam(in_strs[1], AmigosModuleIve::LinkerBgBlurRepBgNegative::E_MOD_IVE_PARAM_TYPE_STAGE);
    return 0;
}
static int set_bgblur_saturation_level(vector<string> &in_strs)
{
    AmigosModuleIve *pMyClass  = GET_PRIVATE_CLASS_OBJ(AmigosModuleIve, AmigosModuleMiBase);
    int iParam = ss_cmd_atoi(in_strs[1].c_str());
    if (iParam < 0 || iParam > 128)
    {
        AMIGOS_ERR("set saturation level: %d failed! out of rang [0, 128]\n", in_strs[1].c_str());
        return -1;
    }
    pMyClass->SetCtrlParam(in_strs[1], AmigosModuleIve::LinkerBgBlurRepBgNegative::E_MOD_IVE_PARAM_TYPE_SATURATIONLV);
    return 0;
}
static int set_bgblur_mosaic_size(vector<string> &in_strs)
{
    AmigosModuleIve *pMyClass  = GET_PRIVATE_CLASS_OBJ(AmigosModuleIve, AmigosModuleMiBase);
    int iParam = ss_cmd_atoi(in_strs[1].c_str());
    if (0 != iParam % 2 || iParam < 2 || iParam > 10)
    {
        AMIGOS_ERR("set mosaic size: %d failed! out of rang [2, 4, 6, 8, 10]\n", in_strs[1].c_str());
        return -1;
    }
    pMyClass->SetCtrlParam(in_strs[1], AmigosModuleIve::LinkerBgBlurRepBgNegative::E_MOD_IVE_PARAM_TYPE_MOSAICSIZE);
    return 0;
}
static int debug_ive_onoff(vector<string> &in_strs)
{
    AmigosModuleIve *pMyClass  = GET_PRIVATE_CLASS_OBJ(AmigosModuleIve, AmigosModuleMiBase);
    if ("on" != in_strs[1] && "off" != in_strs[1])
    {
        AMIGOS_ERR("set ive debug: %s failed! only suport on or off\n", in_strs[1].c_str());
        return -1;
    }
    pMyClass->DebugIveOnOff(in_strs[1]);
    return 0;
}


MOD_CMDS(AmiCmdIve)
{
    ADD_CMD("doWhat", ShowModuleFunction, 0);
    ADD_CMD_HELP("doWhat", "No argument", "what can i do.");
    ADD_CMD("set_bgblur_mode", set_bgblur_mode, 1);
    ADD_CMD_HELP("set_bgblur_mode", "[val]", "Set bgblur mode such as replace(only Souffle)/blur/mosaic(only iFord)/blur_mosaic(only iFord)", "[val]: A string in [replace/blur/mosaic/blur_mosaic]");
    ADD_CMD("set_bgblur_maskop", set_bgblur_maskop, 1);
    ADD_CMD_HELP("set_bgblur_maskop", "[val]", "Set ive preset", "[val]: A string in [none/dilate/erode]");
    ADD_CMD("set_bgblur_thredhold", set_bgblur_thredhold, 1);
    ADD_CMD_HELP("set_bgblur_thredhold", "[val]", "Set bgblur image thredhold param", "[val]: A int in [0, 255]");
    ADD_CMD("set_bgblur_level", set_bgblur_level, 1);
    ADD_CMD_HELP("set_bgblur_level", "[val]", "Set bgblur level", "[val]: A int in [0, 255]");
    ADD_CMD("set_bgblur_stage", set_bgblur_stage, 1);
    ADD_CMD_HELP("set_bgblur_stage", "[val]", "Set bgblur scaling stage", "[val]: A int in [0, 15]");
    ADD_CMD("set_bgblur_saturation_level", set_bgblur_saturation_level, 1);
    ADD_CMD_HELP("set_bgblur_saturation_level", "[val]", "Set bgblur saturation level(only on chip iford)", "[val]: A int in [0, 128]");
    ADD_CMD("set_bgblur_mosaic_size", set_bgblur_mosaic_size, 1);
    ADD_CMD_HELP("set_bgblur_mosaic_size", "[val]", "Set bgblur mosaic size(only on chip iford)", "[val]: A int in [2, 4, 6, 8, 10]");
    ADD_CMD("debug_ive_onoff", debug_ive_onoff, 1);
    ADD_CMD_HELP("debug_ive_onoff", "[val]", "debug for run or close ive", "[val]: A string in [on/off]");
}
