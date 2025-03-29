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
#include "amigos_module_scl.h"
#include "mi_scl.h"

static int SclRot(vector<string> &in_strs)
{
    AmigosModuleScl *pMyClass = GET_PRIVATE_CLASS_OBJ(AmigosModuleScl, AmigosModuleBase);
    AmigosSurfaceScl *pMySurface = dynamic_cast<AmigosSurfaceScl*>(pMyClass->GetSurface());
    sslog << "MY MOD ID IS: " << pMyClass->GetModId() << std::endl;
    const AmigosSurfaceBase::ModInfo &stInfo = pMyClass->GetModInfo();
    int dev = stInfo.devId;
    int chn = stInfo.chnId;
    int rot = ss_cmd_atoi(in_strs[1].c_str());
    MI_SCL_ChnParam_t stSclChnParam;
    AmigosSurfaceScl::SclInfo info;

    pMySurface->GetInfo(info);

    MI_SCL_GetChnParam((MI_SCL_DEV)dev, chn, &stSclChnParam);
    if(90 == rot)
        stSclChnParam.eRot = E_MI_SYS_ROTATE_90;
    else if(180 == rot)
        stSclChnParam.eRot = E_MI_SYS_ROTATE_180;
    else if(270 == rot)
        stSclChnParam.eRot = E_MI_SYS_ROTATE_270;
    else if(0 == rot)
        stSclChnParam.eRot = E_MI_SYS_ROTATE_NONE;

    MI_SCL_SetChnParam((MI_SCL_DEV)dev, chn, &stSclChnParam);
    ss_print(PRINT_LV_TRACE,"set scl dev: %d, chn: %d, rot :%d \n",dev,chn,rot);

    info.uintRotation = stSclChnParam.eRot;
    pMySurface->SetInfo(info);
    return 0;
}

static int SclInputCrop(vector<string> &in_strs)
{
    AmigosModuleScl *pMyClass = GET_PRIVATE_CLASS_OBJ(AmigosModuleScl, AmigosModuleBase);
    AmigosSurfaceScl *pMySurface = dynamic_cast<AmigosSurfaceScl*>(pMyClass->GetSurface());
    sslog << "MY MOD ID IS: " << pMyClass->GetModId() << std::endl;
    const AmigosSurfaceBase::ModInfo &stInfo = pMyClass->GetModInfo();
    int dev = stInfo.devId;
    int chn = stInfo.chnId;
    int x = ss_cmd_atoi(in_strs[1].c_str());
    int y = ss_cmd_atoi(in_strs[2].c_str());
    int w = ss_cmd_atoi(in_strs[3].c_str());
    int h = ss_cmd_atoi(in_strs[4].c_str());
    MI_SYS_WindowRect_t   stCapRect;
    AmigosSurfaceScl::SclInInfo info;

    pMySurface->GetInInfo(0,info);

    MI_SCL_GetInputPortCrop((MI_SCL_DEV)dev, (MI_SCL_CHANNEL)chn, &stCapRect);
    stCapRect.u16X = x;
    stCapRect.u16Y = y;
    stCapRect.u16Width = w;
    stCapRect.u16Height = h;
    MI_SCL_SetInputPortCrop((MI_SCL_DEV)dev, (MI_SCL_CHANNEL)chn, &stCapRect);
    ss_print(PRINT_LV_TRACE,"set scl dev: %d, chn: %d, crop(%d,%d,%d,%d) \n",dev,chn,x,y,w,h);

    info.uintSclInCropX = x;
    info.uintSclInCropY = y;
    info.uintSclInWidth = w;
    info.uintSclInHeight = h;
    pMySurface->SetInInfo(0,info);
    return 0;
}

static int SclOutputCrop(vector<string> &in_strs)
{
    AmigosModuleScl *pMyClass = GET_PRIVATE_CLASS_OBJ(AmigosModuleScl, AmigosModuleBase);
    AmigosSurfaceScl *pMySurface = dynamic_cast<AmigosSurfaceScl*>(pMyClass->GetSurface());
    sslog << "MY MOD ID IS: " << pMyClass->GetModId() << std::endl;
    const AmigosSurfaceBase::ModInfo &stInfo = pMyClass->GetModInfo();
    int dev = stInfo.devId;
    int chn = stInfo.chnId;
    int port = ss_cmd_atoi(in_strs[1].c_str());
    int x = ss_cmd_atoi(in_strs[2].c_str());
    int y = ss_cmd_atoi(in_strs[3].c_str());
    int w = ss_cmd_atoi(in_strs[4].c_str());
    int h = ss_cmd_atoi(in_strs[5].c_str());
    MI_SCL_OutPortParam_t stSclOutputParam;
    AmigosSurfaceScl::SclOutInfo info;

    pMySurface->GetOutInfo(port,info);

    MI_SCL_GetOutputPortParam((MI_SCL_DEV)dev, (MI_SCL_CHANNEL)chn, (MI_SCL_PORT)port, &stSclOutputParam);
    stSclOutputParam.stSCLOutCropRect.u16X = x;
    stSclOutputParam.stSCLOutCropRect.u16Y = y;
    stSclOutputParam.stSCLOutCropRect.u16Width = w;
    stSclOutputParam.stSCLOutCropRect.u16Height = h;
    MI_SCL_SetOutputPortParam((MI_SCL_DEV)dev, (MI_SCL_CHANNEL)chn, (MI_SCL_PORT)port, &stSclOutputParam);
    ss_print(PRINT_LV_TRACE,"set scl dev: %d, chn: %d, port :%d, crop(%d,%d,%d,%d) \n",dev,chn,port,x,y,w,h);

    info.uintSclOutCropX = x;
    info.uintSclOutCropY = y;
    info.uintSclOutCropW = w;
    info.uintSclOutCropH = h;
    pMySurface->SetOutInfo(port,info);
    return 0;
}

static int SclOutputRes(vector<string> &in_strs)
{
    AmigosModuleScl *pMyClass = GET_PRIVATE_CLASS_OBJ(AmigosModuleScl, AmigosModuleBase);
    AmigosSurfaceScl *pMySurface = dynamic_cast<AmigosSurfaceScl*>(pMyClass->GetSurface());
    sslog << "MY MOD ID IS: " << pMyClass->GetModId() << std::endl;
    const AmigosSurfaceBase::ModInfo &stInfo = pMyClass->GetModInfo();
    int dev = stInfo.devId;
    int chn = stInfo.chnId;
    int port = ss_cmd_atoi(in_strs[1].c_str());
    int w = ss_cmd_atoi(in_strs[2].c_str());
    int h = ss_cmd_atoi(in_strs[3].c_str());
    MI_SCL_OutPortParam_t stSclOutputParam;
    AmigosSurfaceScl::SclOutInfo info;

    pMySurface->GetOutInfo(port,info);

    MI_SCL_GetOutputPortParam((MI_SCL_DEV)dev, (MI_SCL_CHANNEL)chn, (MI_SCL_PORT)port, &stSclOutputParam);
    stSclOutputParam.stSCLOutputSize.u16Width = w;
    stSclOutputParam.stSCLOutputSize.u16Height = h;
    MI_SCL_SetOutputPortParam((MI_SCL_DEV)dev, (MI_SCL_CHANNEL)chn, (MI_SCL_PORT)port, &stSclOutputParam);
    ss_print(PRINT_LV_TRACE,"set scl dev: %d, chn: %d, port :%d, size(%d,%d) \n",dev,chn,port,w,h);

    info.uintSclOutWidth = w;
    info.uintSclOutHeight = h;
    pMySurface->SetOutInfo(port,info);
    return 0;
}

static int SclMirrorFlip(vector<string> &in_strs)
{
    AmigosModuleScl *pMyClass = GET_PRIVATE_CLASS_OBJ(AmigosModuleScl, AmigosModuleBase);
    AmigosSurfaceScl *pMySurface = dynamic_cast<AmigosSurfaceScl*>(pMyClass->GetSurface());
    sslog << "MY MOD ID IS: " << pMyClass->GetModId() << std::endl;
    const AmigosSurfaceBase::ModInfo &stInfo = pMyClass->GetModInfo();
    int dev = stInfo.devId;
    int chn = stInfo.chnId;
    int port = ss_cmd_atoi(in_strs[1].c_str());
    bool mirror = ss_cmd_atoi(in_strs[2].c_str());
    bool flip = ss_cmd_atoi(in_strs[3].c_str());
    MI_SCL_OutPortParam_t stSclOutputParam;
    AmigosSurfaceScl::SclOutInfo info;

    pMySurface->GetOutInfo(port,info);

    MI_SCL_GetOutputPortParam((MI_SCL_DEV)dev, (MI_SCL_CHANNEL)chn, (MI_SCL_PORT)port, &stSclOutputParam);
    stSclOutputParam.bMirror = mirror;
    stSclOutputParam.bFlip = flip;
    MI_SCL_SetOutputPortParam((MI_SCL_DEV)dev, (MI_SCL_CHANNEL)chn, (MI_SCL_PORT)port, &stSclOutputParam);
    ss_print(PRINT_LV_TRACE,"set scl dev: %d, chn: %d, port :%d, mirror:%d, flip:%d \n",dev,chn,port,mirror,flip);


    info.uintIsMirror = mirror;
    info.uintIsFlip = flip;
    pMySurface->SetOutInfo(port,info);
    return 0;
}

MOD_CMDS(AmiCmdScl)
{
    ADD_CMD("scl_rot", SclRot, 1);
    ADD_CMD_HELP("scl_rot", "[rot(0,90,180,270)]", "example : scl_rot 90");
    ADD_CMD("scl_input_crop", SclInputCrop, 4);
    ADD_CMD_HELP("scl_input_crop", "[x] [y] [w] [h]", "example : scl_input_crop 0 0 640 360");
    ADD_CMD("scl_output_crop", SclOutputCrop, 5);
    ADD_CMD_HELP("scl_output_crop", "[port] [x] [y] [w] [h]", "example : scl_output_crop 0 0 0 640 360");
    ADD_CMD("scl_output_res", SclOutputRes, 3);
    ADD_CMD_HELP("scl_output_res", "[port] [w] [h]", "example : scl_output_res 0 1920 1080");
    ADD_CMD("scl_mirror_flip", SclMirrorFlip, 3);
    ADD_CMD_HELP("scl_mirror_flip", "[port] [mirror] [flip]", "example : scl_mirror_flip 0 1 1");
}
