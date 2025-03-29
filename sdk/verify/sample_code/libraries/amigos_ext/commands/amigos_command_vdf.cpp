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
#include "amigos_module_vdf.h"
#include "mi_common_datatype.h"
#include "mi_vdf.h"

static int SetMdDynamicParam(vector<string> &in_strs)
{
    MI_VDF_ChnAttr_t stMdAttr;
    AmigosModuleVdf *pMyClass = GET_PRIVATE_CLASS_OBJ(AmigosModuleVdf, AmigosModuleBase);
    const AmigosSurfaceBase::ModInfo &stMdModInfo = pMyClass->GetModInfo();
    if(MI_SUCCESS != MI_VDF_GetChnAttr(stMdModInfo.chnId, &stMdAttr))
    {
        ss_print(PRINT_LV_ERROR, "set Md param failed! Get MD chn(%d) attr failed!\n", stMdModInfo.chnId);
        return -1;
    }
    stMdAttr.stMdAttr.stMdDynamicParamsIn.sensitivity = atoi(in_strs[1].c_str());
    stMdAttr.stMdAttr.stMdDynamicParamsIn.learn_rate = atoi(in_strs[2].c_str());
    stMdAttr.stMdAttr.stMdDynamicParamsIn.md_thr = atoi(in_strs[3].c_str());
    if(MI_SUCCESS != MI_VDF_SetChnAttr(stMdModInfo.chnId, &stMdAttr))
    {
        ss_print(PRINT_LV_ERROR, "Set MD chn(%d) attr failed!\n", stMdModInfo.chnId);
        return -1;
    }
    return 0;
}

static int SetOdParam(vector<string> &in_strs)
{
    MI_VDF_ChnAttr_t stOdAttr;
    AmigosModuleVdf *pMyClass = GET_PRIVATE_CLASS_OBJ(AmigosModuleVdf, AmigosModuleBase);
    const AmigosSurfaceBase::ModInfo &stMdModInfo = pMyClass->GetModInfo();
    if(MI_SUCCESS != MI_VDF_GetChnAttr(stMdModInfo.chnId, &stOdAttr))
    {
        ss_print(PRINT_LV_ERROR, "set Md param failed! Get OD chn(%d) attr failed!\n", stMdModInfo.chnId);
        return -1;
    }
    if(E_MI_VDF_WORK_MODE_OD != stOdAttr.enWorkMode)
    {
        ss_print(PRINT_LV_ERROR, "set od param failed! this mod(%d) is not MD(%d)\n", stOdAttr.enWorkMode, E_MI_VDF_WORK_MODE_OD);
        return -1;
    }
    stOdAttr.stOdAttr.stOdDynamicParamsIn.thd_tamper = atoi(in_strs[1].c_str());
    stOdAttr.stOdAttr.stOdDynamicParamsIn.tamper_blk_thd = atoi(in_strs[2].c_str());
    stOdAttr.stOdAttr.stOdDynamicParamsIn.min_duration   = atoi(in_strs[3].c_str());
    if(MI_SUCCESS != MI_VDF_SetChnAttr(stMdModInfo.chnId, &stOdAttr))
    {
        ss_print(PRINT_LV_ERROR, "Set OD chn(%d) attr failed!\n", stMdModInfo.chnId);
        return -1;
    }
    return 0;
}

MOD_CMDS(AmiCmdVdf)
{
    ADD_CMD("set_md", SetMdDynamicParam, 3);
    ADD_CMD_HELP("set_md", "[sensitivity] [learn_rate] [md_thr]",
                 "set_md is set md dynamic params",
                 "[sensitivity]: only FG mode valid, rand [10,20, ... 100]",
                 "[learn_rate]: FG mode meas times for pic turn to bg pic, rand [1000, 30000]; SAD mode means bg update weight ratio, rand [1, 255], suggest 128",
                 "[md_thr]: FG mode means alarm  by percentage of ccl, rand [0, 99], SAD mode means alram by pixel nums of ccl, rand [0, 255] ");
    ADD_CMD("set_od", SetOdParam, 3);
    ADD_CMD_HELP("set_od", "[thd_tamper] [tamper_blk_thd] [min_duration]",
                 "set_od is set od dynamic params",
                 "[thd_tamper]: alarm when cover area over (10 - [thd_tamper]) * 10 / 100 , rand [0, 10]",
                 "[tamper_blk_thd]: alarm when cover area over nums, rand [1, OD_WINDOW]",
                 "[min_duration]: alarm by diff pic nums");
}
