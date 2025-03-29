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
#include "amigos_module_isp.h"
#include "mi_isp.h"
#include "mi_isp_cus3a_api.h"

static int IspRot(vector<string> &in_strs)
{
    AmigosModuleIsp *pMyClass = GET_PRIVATE_CLASS_OBJ(AmigosModuleIsp, AmigosModuleBase);
    AmigosSurfaceIsp *pMySurface = dynamic_cast<AmigosSurfaceIsp*>(pMyClass->GetSurface());
    sslog << "MY MOD ID IS: " << pMyClass->GetModId() << std::endl;
    const AmigosSurfaceBase::ModInfo &stInfo = pMyClass->GetModInfo();
    int dev = stInfo.devId;
    int chn = stInfo.chnId;
    int rot = ss_cmd_atoi(in_strs[1].c_str());
    MI_ISP_ChnParam_t stChannelIspParam;
    AmigosSurfaceIsp::IspInfo info;

    pMySurface->GetInfo(info);

    MI_ISP_StopChannel(dev,chn);
    MI_ISP_GetChnParam((MI_ISP_DEV)dev, chn, &stChannelIspParam);
    if(90 == rot)
        stChannelIspParam.eRot = E_MI_SYS_ROTATE_90;
    else if(180 == rot)
        stChannelIspParam.eRot = E_MI_SYS_ROTATE_180;
    else if(270 == rot)
        stChannelIspParam.eRot = E_MI_SYS_ROTATE_270;
    else if(0 == rot)
        stChannelIspParam.eRot = E_MI_SYS_ROTATE_NONE;
    MI_ISP_SetChnParam((MI_ISP_DEV)dev, chn, &stChannelIspParam);
    MI_ISP_StartChannel(dev,chn);
    ss_print(PRINT_LV_TRACE,"set isp dev: %d, chn: %d, rot :%d \n",dev,chn,rot);

    info.uintRotation = stChannelIspParam.eRot;
    pMySurface->SetInfo(info);
    return 0;
}

static int IspMirrorFlip(vector<string> &in_strs)
{
    AmigosModuleIsp *pMyClass = GET_PRIVATE_CLASS_OBJ(AmigosModuleIsp, AmigosModuleBase);
    AmigosSurfaceIsp *pMySurface = dynamic_cast<AmigosSurfaceIsp*>(pMyClass->GetSurface());
    sslog << "MY MOD ID IS: " << pMyClass->GetModId() << std::endl;
    const AmigosSurfaceBase::ModInfo &stInfo = pMyClass->GetModInfo();
    int dev = stInfo.devId;
    int chn = stInfo.chnId;
    bool mirror = ss_cmd_atoi(in_strs[1].c_str());
    bool flip = ss_cmd_atoi(in_strs[2].c_str());
    MI_ISP_ChnParam_t stChannelIspParam;
    AmigosSurfaceIsp::IspInfo info;

    pMySurface->GetInfo(info);

    MI_ISP_StopChannel(dev, chn);
    MI_ISP_GetChnParam((MI_ISP_DEV)dev, chn, &stChannelIspParam);
    stChannelIspParam.bMirror = mirror;
    stChannelIspParam.bFlip = flip;
    MI_ISP_SetChnParam((MI_ISP_DEV)dev, chn, &stChannelIspParam);
    MI_ISP_StartChannel(dev,chn);
    ss_print(PRINT_LV_TRACE,"set isp dev: %d, chn: %d, mirror :%d, flip: %d \n",dev,chn,mirror,flip);

    info.uintIsMirror = mirror;
    info.uintIsFlip = flip;
    pMySurface->SetInfo(info);
    return 0;
}

static int IspInputCrop(vector<string> &in_strs)
{
    AmigosModuleIsp *pMyClass = GET_PRIVATE_CLASS_OBJ(AmigosModuleIsp, AmigosModuleBase);
    AmigosSurfaceIsp *pMySurface = dynamic_cast<AmigosSurfaceIsp*>(pMyClass->GetSurface());
    sslog << "MY MOD ID IS: " << pMyClass->GetModId() << std::endl;
    const AmigosSurfaceBase::ModInfo &stInfo = pMyClass->GetModInfo();
    int dev = stInfo.devId;
    int chn = stInfo.chnId;
    int x = ss_cmd_atoi(in_strs[1].c_str());
    int y = ss_cmd_atoi(in_strs[2].c_str());
    int w = ss_cmd_atoi(in_strs[3].c_str());
    int h = ss_cmd_atoi(in_strs[4].c_str());
    MI_SYS_WindowRect_t stCapRect;
    AmigosSurfaceIsp::IspInInfo info;

    pMySurface->GetInInfo(0,info);

    stCapRect.u16X = x;
    stCapRect.u16Y = y;
    stCapRect.u16Width = w;
    stCapRect.u16Height = h;
    MI_ISP_SetInputPortCrop((MI_ISP_DEV)dev, chn, &stCapRect);
    ss_print(PRINT_LV_TRACE,"set isp dev: %d, chn: %d, input crop(%d,%d,%d,%d) \n",dev,chn,x,y,w,h);

    info.uintIspInCropX = x;
    info.uintIspInCropY = y;
    info.uintIspInWidth = w;
    info.uintIspInHeight = h;
    pMySurface->SetInInfo(0,info);
    return 0;
}

static int IspOutputCrop(vector<string> &in_strs)
{
    AmigosModuleIsp *pMyClass = GET_PRIVATE_CLASS_OBJ(AmigosModuleIsp, AmigosModuleBase);
    AmigosSurfaceIsp *pMySurface = dynamic_cast<AmigosSurfaceIsp*>(pMyClass->GetSurface());
    sslog << "MY MOD ID IS: " << pMyClass->GetModId() << std::endl;
    const AmigosSurfaceBase::ModInfo &stInfo = pMyClass->GetModInfo();
    int dev = stInfo.devId;
    int chn = stInfo.chnId;
    int port = ss_cmd_atoi(in_strs[1].c_str());
    int x = ss_cmd_atoi(in_strs[2].c_str());
    int y = ss_cmd_atoi(in_strs[3].c_str());
    int w = ss_cmd_atoi(in_strs[4].c_str());
    int h = ss_cmd_atoi(in_strs[5].c_str());
    MI_ISP_OutPortParam_t stIspOutputParam;
    AmigosSurfaceIsp::IspOutInfo info;

    pMySurface->GetOutInfo(port,info);

    MI_ISP_GetOutputPortParam((MI_ISP_DEV)dev, (MI_ISP_CHANNEL)chn, port, &stIspOutputParam);
    stIspOutputParam.stCropRect.u16X = x;
    stIspOutputParam.stCropRect.u16Y = y;
    stIspOutputParam.stCropRect.u16Width = w;
    stIspOutputParam.stCropRect.u16Height = h;
    MI_ISP_SetOutputPortParam((MI_ISP_DEV)dev, (MI_ISP_CHANNEL)chn, port, &stIspOutputParam);
    ss_print(PRINT_LV_TRACE,"set isp dev: %d, chn: %d, port:%d, output crop(%d,%d,%d,%d) \n",dev,chn,port,x,y,w,h);

    info.uintIspOutCropX = x;
    info.uintIspOutCropY = y;
    info.uintIspOutCropW = w;
    info.uintIspOutCropH = h;
    pMySurface->SetOutInfo(port,info);
    return 0;
}

static int Isp3dnr(vector<string> &in_strs)
{
    AmigosModuleIsp *pMyClass = GET_PRIVATE_CLASS_OBJ(AmigosModuleIsp, AmigosModuleBase);
    AmigosSurfaceIsp *pMySurface = dynamic_cast<AmigosSurfaceIsp*>(pMyClass->GetSurface());
    sslog << "MY MOD ID IS: " << pMyClass->GetModId() << std::endl;
    const AmigosSurfaceBase::ModInfo &stInfo = pMyClass->GetModInfo();
    int dev = stInfo.devId;
    int chn = stInfo.chnId;
    int level_3dnr = ss_cmd_atoi(in_strs[1].c_str());
    MI_ISP_ChnParam_t stChannelIspParam;
    AmigosSurfaceIsp::IspInfo info;

    pMySurface->GetInfo(info);

    MI_ISP_StopChannel(dev,chn);
    MI_ISP_GetChnParam((MI_ISP_DEV)dev, chn, &stChannelIspParam);
    stChannelIspParam.e3DNRLevel = (MI_ISP_3DNR_Level_e)level_3dnr;
    MI_ISP_SetChnParam((MI_ISP_DEV)dev, chn, &stChannelIspParam);
    MI_ISP_StartChannel(dev,chn);
    ss_print(PRINT_LV_TRACE,"set isp dev: %d, chn: %d, 3dnr :%d \n",dev,chn,level_3dnr);

    info.uint3dNrLevel = level_3dnr;
    pMySurface->SetInfo(info);
    return 0;
}

static int IspHdr(vector<string> &in_strs)
{
    AmigosModuleIsp *pMyClass = GET_PRIVATE_CLASS_OBJ(AmigosModuleIsp, AmigosModuleBase);
    AmigosSurfaceIsp *pMySurface = dynamic_cast<AmigosSurfaceIsp*>(pMyClass->GetSurface());
    sslog << "MY MOD ID IS: " << pMyClass->GetModId() << std::endl;
    const AmigosSurfaceBase::ModInfo &stInfo = pMyClass->GetModInfo();
    int dev = stInfo.devId;
    int chn = stInfo.chnId;
    MI_ISP_HDRType_e hdr = ss_enum_cast<MI_ISP_HDRType_e>::from_str(in_strs[1].c_str());
    MI_ISP_HDRFusionType_e hdr_fusion = ss_enum_cast<MI_ISP_HDRFusionType_e>::from_str(in_strs[2].c_str());
    int hdr_exposure_mask = ss_cmd_atoi(in_strs[3].c_str());
    MI_ISP_ChnParam_t stChannelIspParam;
    AmigosSurfaceIsp::IspInfo info;

    pMySurface->GetInfo(info);

    MI_ISP_StopChannel(dev,chn);
    MI_ISP_GetChnParam((MI_ISP_DEV)dev, chn, &stChannelIspParam);
    stChannelIspParam.eHDRType = (MI_ISP_HDRType_e)hdr;
    stChannelIspParam.eHDRFusionType = (MI_ISP_HDRFusionType_e)hdr_fusion;
    stChannelIspParam.u16HDRExposureMask = hdr_exposure_mask;
    MI_ISP_SetChnParam((MI_ISP_DEV)dev, chn, &stChannelIspParam);
    MI_ISP_StartChannel(dev,chn);
    ss_print(PRINT_LV_TRACE,"set isp dev: %d, chn: %d, hdr :%d, fusion: %d, expo mask: %d\n",dev,chn,hdr,hdr_fusion,hdr_exposure_mask);

    info.uintHdrType = hdr;
    info.uintHdrFusionType = hdr_fusion;
    info.uintHdrExposureMask = hdr_exposure_mask;
    pMySurface->SetInfo(info);
    return 0;
}

static int IspSkipFrame(vector<string> &in_strs)
{
    AmigosModuleIsp *pMyClass = GET_PRIVATE_CLASS_OBJ(AmigosModuleIsp, AmigosModuleBase);
    sslog << "MY MOD ID IS: " << pMyClass->GetModId() << std::endl;
    const AmigosSurfaceBase::ModInfo &stInfo = pMyClass->GetModInfo();
    int dev = stInfo.devId;
    int chn = stInfo.chnId;
    int frame = ss_cmd_atoi(in_strs[1].c_str());
    MI_ISP_SkipFrame(dev, chn,frame);
    ss_print(PRINT_LV_TRACE,"set isp dev: %d, chn: %d, skip frame :%d \n",dev,chn,frame);
    return 0;
}

static int IspApiBin(vector<string> &in_strs)
{
    AmigosModuleIsp *pMyClass = GET_PRIVATE_CLASS_OBJ(AmigosModuleIsp, AmigosModuleBase);
    AmigosSurfaceIsp *pMySurface = dynamic_cast<AmigosSurfaceIsp*>(pMyClass->GetSurface());
    sslog << "MY MOD ID IS: " << pMyClass->GetModId() << std::endl;
    const AmigosSurfaceBase::ModInfo &stInfo = pMyClass->GetModInfo();
    int dev = stInfo.devId;
    int ch = stInfo.chnId;
    const char * pIqBinPath = in_strs[1].c_str();
    AmigosSurfaceIsp::IspInfo info;

    pMySurface->GetInfo(info);

    ss_print(PRINT_LV_TRACE,"%s:%d pIqBinPath:%s\n", __func__, __LINE__, pIqBinPath);
    //return MI_ISP_IQ_ApiCmdLoadBinFile(dev, ch, (MI_U8 *)pIqBinPath, 1234);
    MI_ISP_ApiCmdLoadBinFile(dev, ch, (char *)pIqBinPath, 1234);

    info.apiFileName = in_strs[1];
    pMySurface->SetInfo(info);
    return 0;
}

static int IspLdc(vector<string> &in_strs)
{
    AmigosModuleIsp *pMyClass = GET_PRIVATE_CLASS_OBJ(AmigosModuleIsp, AmigosModuleBase);
    AmigosSurfaceIsp *pMySurface = dynamic_cast<AmigosSurfaceIsp*>(pMyClass->GetSurface());
    sslog << "MY MOD ID IS: " << pMyClass->GetModId() << std::endl;
    const AmigosSurfaceBase::ModInfo &stInfo = pMyClass->GetModInfo();
    int dev = stInfo.devId;
    int ch = stInfo.chnId;
    int centerX = ss_cmd_atoi(in_strs[1].c_str());
    int centerY = ss_cmd_atoi(in_strs[2].c_str());
    int alpha = ss_cmd_atoi(in_strs[3].c_str());
    int beta = ss_cmd_atoi(in_strs[4].c_str());
    int cropL = ss_cmd_atoi(in_strs[5].c_str());
    int cropR = ss_cmd_atoi(in_strs[6].c_str());
    MI_ISP_LdcAttr_t stLdcAttr;
    AmigosSurfaceIsp::IspInfo info;

    pMySurface->GetInfo(info);

    stLdcAttr.u32CenterXOffset = centerX;
    stLdcAttr.u32CenterYOffset = centerY;
    stLdcAttr.s32Alpha = alpha;
    stLdcAttr.s32Beta = beta;
    stLdcAttr.u32CropLeft = cropL;
    stLdcAttr.u32CropRight = cropR;
    MI_ISP_SetLdcAttr(dev, ch, &stLdcAttr);
    ss_print(PRINT_LV_TRACE,"set isp dev: %d, chn: %d ldc centerX/Y(%d,%d), alpha/beta(%d,%d),cropL/R(%d,%d)\n", \
        dev,ch,centerX,centerY,alpha,beta,cropL,cropR);

    info.IspLdcParam.CenterX = centerX;
    info.IspLdcParam.CenterY = centerY;
    info.IspLdcParam.Alpha = alpha;
    info.IspLdcParam.Beta = beta;
    info.IspLdcParam.CropL = cropL;
    info.IspLdcParam.CropR = cropR;
    pMySurface->SetInfo(info);
    return 0;
}

MOD_CMDS(AmiCmdIsp)
{
    ADD_CMD("isp_rot", IspRot, 1);
    ADD_CMD_HELP("isp_rot", "[rot]","[rot]:90,180,270,0 example : isp_rot 90");
    ADD_CMD("isp_mirror_flip", IspMirrorFlip, 2);
    ADD_CMD_HELP("isp_mirror_flip", "[mirror] [flip]", "example : isp_mirror_flip 0 0");
    ADD_CMD("isp_input_crop", IspInputCrop, 4);
    ADD_CMD_HELP("isp_input_crop", "[x] [y] [w] [h]", "example : isp_input_crop 0 0 640 360");
    ADD_CMD("isp_output_crop", IspOutputCrop, 5);
    ADD_CMD_HELP("isp_output_crop", "[port] [x] [y] [w] [h]", "example : isp_output_crop 0 0 0 640 360");
    ADD_CMD("isp_3dnr", Isp3dnr, 1);
    ADD_CMD_HELP("isp_3dnr", "[3dnr_level(0~2)]", "example : isp_3dnr 1");
    ADD_CMD("isp_hdr", IspHdr, 3);
    ADD_CMD_HELP("isp_hdr", "[hdr(off/vc/dol)] [FusionType(off/2to1/3to1)] [ExposureMask(0:off 1:S 2:M 4:L)]", "example : isp_hdr dol 2to1 5");
    ADD_CMD("isp_skip_frame", IspSkipFrame, 1);
    ADD_CMD_HELP("isp_skip_frame", "[num]", "example : isp_skip_frame 5");
    ADD_CMD("isp_api_bin", IspApiBin, 1);
    ADD_CMD_HELP("isp_api_bin", "[file]", "[file]: bin file path");
    ADD_CMD("isp_ldc", IspLdc, 6);
    ADD_CMD_HELP("isp_ldc", "[centerX] [centerY] [alpha] [beta] [cropL] [cropR]", "example : isp_ldc 10094 5765 614 438 0 0");
}
