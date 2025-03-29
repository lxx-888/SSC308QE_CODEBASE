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
#include "amigos_module_vdec.h"
#include "mi_vdec.h"

#define VDEC_ALIGN_2xUP(x)               (((x+1) / 2) * 2)
#define VDEC_ALIGN_16xUP(x)              (((x+15) / 16) * 16)

static int SetPause(vector<string> &in_strs)
{
    AmigosModuleVdec *pMyClass = GET_PRIVATE_CLASS_OBJ(AmigosModuleVdec, AmigosModuleBase);
    sslog << "MY MOD ID IS: " << pMyClass->GetModId() << std::endl;
    const AmigosSurfaceBase::ModInfo &stInfo = pMyClass->GetModInfo();
    int dev = stInfo.devId;
    int chn = stInfo.chnId;

    if (MI_SUCCESS != MI_VDEC_PauseChn(dev, chn))
    {
        ss_print(PRINT_LV_TRACE, "MI_VDEC_PauseChn failed,chn:%d\n", chn);
        return 0;
    }
    return 0;
}

static int SetRefresh(vector<string> &in_strs)
{
    AmigosModuleVdec *pMyClass = GET_PRIVATE_CLASS_OBJ(AmigosModuleVdec, AmigosModuleBase);
    sslog << "MY MOD ID IS: " << pMyClass->GetModId() << std::endl;
    const AmigosSurfaceBase::ModInfo &stInfo = pMyClass->GetModInfo();
    int dev = stInfo.devId;
    int chn = stInfo.chnId;

    if (MI_SUCCESS != MI_VDEC_RefreshChn(dev, chn))
    {
        ss_print(PRINT_LV_TRACE, "MI_VDEC_RefreshChn failed,chn:%d\n", chn);
        return 0;
    }
    return 0;
}

static int SetResume(vector<string> &in_strs)
{
    AmigosModuleVdec *pMyClass = GET_PRIVATE_CLASS_OBJ(AmigosModuleVdec, AmigosModuleBase);
    sslog << "MY MOD ID IS: " << pMyClass->GetModId() << std::endl;
    const AmigosSurfaceBase::ModInfo &stInfo = pMyClass->GetModInfo();
    int dev = stInfo.devId;
    int chn = stInfo.chnId;

    if (MI_SUCCESS != MI_VDEC_ResumeChn(dev, chn))
    {
        ss_print(PRINT_LV_TRACE, "MI_VDEC_ResumeChn failed,chn:%d\n", chn);
        return 0;
    }
    return 0;
}

static int SetCrop(vector<string> &in_strs)
{
    AmigosModuleVdec *pMyClass = GET_PRIVATE_CLASS_OBJ(AmigosModuleVdec, AmigosModuleBase);
    sslog << "MY MOD ID IS: " << pMyClass->GetModId() << std::endl;
    const AmigosSurfaceBase::ModInfo &stInfo = pMyClass->GetModInfo();
    MI_VDEC_CropCfg_t stCropCfg;
    int               dev   = stInfo.devId;
    int               chn   = stInfo.chnId;
    int               CropX = ss_cmd_atoi(in_strs[1].c_str());
    int               CropY = ss_cmd_atoi(in_strs[2].c_str());
    int               CropW = ss_cmd_atoi(in_strs[3].c_str());
    int               CropH = ss_cmd_atoi(in_strs[4].c_str());

    stCropCfg.bEnable          = TRUE;
    stCropCfg.stRect.u16X      = VDEC_ALIGN_16xUP(CropX);
    stCropCfg.stRect.u16Y      = VDEC_ALIGN_2xUP(CropY);
    stCropCfg.stRect.u16Width  = VDEC_ALIGN_16xUP(CropW);
    stCropCfg.stRect.u16Height = VDEC_ALIGN_2xUP(CropH);
    if (MI_SUCCESS != MI_VDEC_SetDestCrop(dev, chn, &stCropCfg))
    {
        ss_print(PRINT_LV_TRACE,"MI_VDEC_SetDestCrop failed, chn: %d\n", chn);
    }

    return 0;
}

static int SetScaling(vector<string> &in_strs)
{
    AmigosModuleVdec *pMyClass = GET_PRIVATE_CLASS_OBJ(AmigosModuleVdec, AmigosModuleBase);
    sslog << "MY MOD ID IS: " << pMyClass->GetModId() << std::endl;
    const AmigosSurfaceBase::ModInfo &stInfo = pMyClass->GetModInfo();
    int               dev   = stInfo.devId;
    int               chn   = stInfo.chnId;
    int               u16Width = ss_cmd_atoi(in_strs[1].c_str());
    int               u16Height = ss_cmd_atoi(in_strs[2].c_str());

    MI_VDEC_OutputPortAttr_t stOutputPortAttr;
    memset(&stOutputPortAttr, 0, sizeof(MI_VDEC_OutputPortAttr_t));
    stOutputPortAttr.u16Width  = u16Width;
    stOutputPortAttr.u16Height = u16Height;
    MI_VDEC_SetOutputPortAttr(dev, chn, &stOutputPortAttr);

    return 0;
}

MOD_CMDS(AmiCmdVdec)
{
    ADD_CMD("set_pause", SetPause, 0);
    ADD_CMD_HELP("set_pause", "No argument", "set pause");

    ADD_CMD("set_refresh", SetRefresh, 0);
    ADD_CMD_HELP("set_refresh", "No argument", "set refresh");

    ADD_CMD("set_resume", SetResume, 0);
    ADD_CMD_HELP("set_resume", "No argument", "set resume");

    ADD_CMD("set_crop", SetCrop, 4);
    ADD_CMD_HELP("set_crop", "[CropX],[CropY],[CropW],[CropH]",
                 "set crop",
                 "[CropX] : crop coord x",
                 "[CropX] : crop coord y",
                 "[CropW] : crop width",
                 "[CropH] : crop hight");

    ADD_CMD("set_scaling", SetScaling, 2);
    ADD_CMD_HELP("set_scaling", "[Width] [Height]",
                 "set scaling",
                 "[Width] : Scaling Width ",
                 "[Height] : Scaling Height");
}
