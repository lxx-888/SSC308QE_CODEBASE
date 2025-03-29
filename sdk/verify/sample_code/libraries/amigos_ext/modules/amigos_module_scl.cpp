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

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <string>
#include "amigos_module_init.h"
#include "amigos_module_mi_base.h"
#include "amigos_module_scl_base.h"
#include "amigos_surface_base.h"
#include "amigos_surface_scl.h"
#include "mi_common_datatype.h"
#include "mi_scl.h"
#include "amigos_module_scl.h"

AmigosModuleScl::AmigosModuleScl(const std::string &strSection)
    : AmigosSurfaceScl(strSection), AmigosModuleSclBase(this)
{
}
AmigosModuleScl::~AmigosModuleScl() {}

unsigned int AmigosModuleScl::GetModId() const
{
    return E_MI_MODULE_ID_SCL;
}
unsigned int AmigosModuleScl::GetInputType(unsigned int port) const
{
    return E_MOD_PORT_TYPE_KERNEL | E_MOD_PORT_TYPE_USER;
}
unsigned int AmigosModuleScl::GetOutputType(unsigned int port) const
{
    return E_MOD_PORT_TYPE_KERNEL | E_MOD_PORT_TYPE_USER;
}
void AmigosModuleScl::_ResourceInit()
{
    this->_CreateDeviceResource(this->stModInfo.devId);
    for (auto itMapSclOut = mapSclOut.begin(); itMapSclOut != mapSclOut.end(); itMapSclOut++)
    {
        this->env.Out(itMapSclOut->first)["WIDTH"]  = std::to_string(itMapSclOut->second.uintSclOutWidth);
        this->env.Out(itMapSclOut->first)["HEIGHT"] = std::to_string(itMapSclOut->second.uintSclOutHeight);
    }
}
void AmigosModuleScl::_ResourceDeinit()
{
    this->_DestroyDeviceResource(this->stModInfo.devId);
}

void AmigosModuleScl::_Init()
{
    MI_SCL_DevAttr_t      stSclDevAttr;
    MI_SCL_ChannelAttr_t  stSclChnAttr;
    MI_SCL_ChnParam_t     stSclChnParam;
    MI_SYS_WindowRect_t   stCapRect;

    memset(&stSclDevAttr, 0x0, sizeof(MI_SCL_DevAttr_t));
    memset(&stSclChnAttr, 0x0, sizeof(MI_SCL_ChannelAttr_t));
    memset(&stSclChnParam, 0x0, sizeof(MI_SCL_ChnParam_t));

    AMILOG_INFO << std::to_string(this->GetModId()) << " " << std::to_string(this->stModInfo.devId) << " "
                << std::to_string(this->stModInfo.chnId) << std::endl;
    stSclDevAttr.u32NeedUseHWOutPortMask = this->stSclInfo.uintHwPortMode;
    this->_CreateDevice(this->stModInfo.devId, stSclDevAttr);
    MI_SCL_CreateChannel((MI_SCL_DEV)this->stModInfo.devId, (MI_SCL_CHANNEL)this->stModInfo.chnId, &stSclChnAttr);
    stSclChnParam.eRot = (MI_SYS_Rotate_e)stSclInfo.uintRotation;
    MI_SCL_SetChnParam((MI_SCL_DEV)this->stModInfo.devId, (MI_SCL_CHANNEL)this->stModInfo.chnId, &stSclChnParam);
    for (auto itMapSclIn = mapSclIn.begin(); itMapSclIn != mapSclIn.end(); itMapSclIn++)
    {
        AmigosSurfaceBase::ModPortInInfo stInInfo;
        this->GetSurface()->GetPortInInfo(itMapSclIn->first, stInInfo);
        if (stInInfo.bindType != E_MI_SYS_BIND_TYPE_FRAME_BASE)
        {
            //Scl device 0 and device 2 is for realtime mode use.
            continue;
        }
        stCapRect.u16X = itMapSclIn->second.uintSclInCropX;
        stCapRect.u16Y = itMapSclIn->second.uintSclInCropY;
        stCapRect.u16Width = itMapSclIn->second.uintSclInWidth;
        stCapRect.u16Height = itMapSclIn->second.uintSclInHeight;
        if (!stCapRect.u16Width || !stCapRect.u16Height)
        {
            continue;
        }
        MI_SCL_SetInputPortCrop((MI_SCL_DEV)this->stModInfo.devId, (MI_SCL_CHANNEL)this->stModInfo.chnId, &stCapRect);
    }
}
void AmigosModuleScl::_ResetStreamOut(unsigned int outPortId, unsigned int width, unsigned int height)
{
    auto it = this->mapPortOut.find(outPortId);
    if (it == this->mapPortOut.end())
    {
        AMILOG_ERR << "can not find outPortId " << outPortId << COLOR_ENDL;
        return;
    }
    auto itMapSclOut = mapSclOut.find(outPortId);
    if (itMapSclOut == mapSclOut.end())
    {
        AMIGOS_ERR("Can not find scl output port %d info!\n", outPortId);
        return;
    }
    if (it->second.bStart)
    {
        MI_SCL_OutPortParam_t stSclOutputParam;
        memset(&stSclOutputParam, 0, sizeof(MI_SCL_OutPortParam_t));
        MI_SCL_GetOutputPortParam((MI_SCL_DEV)stModInfo.devId, (MI_SCL_CHANNEL)stModInfo.chnId, (MI_SCL_PORT)outPortId, &stSclOutputParam);
        itMapSclOut->second.uintSclOutWidth = stSclOutputParam.stSCLOutputSize.u16Width = width;
        itMapSclOut->second.uintSclOutHeight = stSclOutputParam.stSCLOutputSize.u16Height = height;
        MI_SCL_SetOutputPortParam((MI_SCL_DEV)stModInfo.devId, (MI_SCL_CHANNEL)stModInfo.chnId, (MI_SCL_PORT)outPortId, &stSclOutputParam);
        this->env.Out(outPortId)["WIDTH"]  = std::to_string(width);
        this->env.Out(outPortId)["HEIGHT"] = std::to_string(height);
        return;
    }
    itMapSclOut->second.uintSclOutWidth = width;
    itMapSclOut->second.uintSclOutHeight = height;
    this->env.Out(outPortId)["WIDTH"]  = std::to_string(width);
    this->env.Out(outPortId)["HEIGHT"] = std::to_string(height);
}
void AmigosModuleScl::_Deinit()
{
    MI_SCL_DestroyChannel((MI_SCL_DEV)this->stModInfo.devId, (MI_SCL_CHANNEL)this->stModInfo.chnId);
    this->_DestroyDevice(this->stModInfo.devId);
}

void AmigosModuleScl::_Start()
{
    MI_SCL_StartChannel((MI_SCL_DEV)this->stModInfo.devId, (MI_SCL_CHANNEL)this->stModInfo.chnId);
}

void AmigosModuleScl::_Stop()
{
    MI_SCL_StopChannel((MI_SCL_DEV)this->stModInfo.devId, (MI_SCL_CHANNEL)this->stModInfo.chnId);
}

void AmigosModuleScl::_StartMiOut(unsigned int outPortId)
{
    MI_SCL_OutPortParam_t stSclOutputParam;
    auto itMapSclOut = mapSclOut.find(outPortId);
    if (itMapSclOut == mapSclOut.end())
    {
        AMIGOS_ERR("Can not find scl output port %d info!\n", outPortId);
        return;
    }
    memset(&stSclOutputParam, 0, sizeof(MI_SCL_OutPortParam_t));
    stSclOutputParam.bMirror = (MI_BOOL)itMapSclOut->second.uintIsMirror;
    stSclOutputParam.bFlip = (MI_BOOL)itMapSclOut->second.uintIsFlip;
    stSclOutputParam.stSCLOutCropRect.u16X = itMapSclOut->second.uintSclOutCropX;
    stSclOutputParam.stSCLOutCropRect.u16Y = itMapSclOut->second.uintSclOutCropY;
    stSclOutputParam.stSCLOutCropRect.u16Width = itMapSclOut->second.uintSclOutCropW;
    stSclOutputParam.stSCLOutCropRect.u16Height = itMapSclOut->second.uintSclOutCropH;;
    stSclOutputParam.ePixelFormat =
        ss_enum_cast<MI_SYS_PixelFormat_e>::from_str(itMapSclOut->second.strOutFmt);
    stSclOutputParam.stSCLOutputSize.u16Width = (MI_U16)itMapSclOut->second.uintSclOutWidth;
    stSclOutputParam.stSCLOutputSize.u16Height = (MI_U16)itMapSclOut->second.uintSclOutHeight;
    stSclOutputParam.eCompressMode = (MI_SYS_CompressMode_e)itMapSclOut->second.uintCompressMode;

    MI_SCL_SetOutputPortParam((MI_SCL_DEV)this->stModInfo.devId, (MI_SCL_CHANNEL)this->stModInfo.chnId, (MI_SCL_PORT)outPortId, &stSclOutputParam);
    MI_SCL_EnableOutputPort((MI_SCL_DEV)this->stModInfo.devId, (MI_SCL_CHANNEL)this->stModInfo.chnId, (MI_SCL_PORT)outPortId);
    this->env.Out(itMapSclOut->first)["WIDTH"] = std::to_string(itMapSclOut->second.uintSclOutWidth);
    this->env.Out(itMapSclOut->first)["HEIGHT"] = std::to_string(itMapSclOut->second.uintSclOutHeight);
    AMIGOS_INFO("SCL ENABLE dev%d chn %d port %d\n", stModInfo.devId, stModInfo.chnId, outPortId);
}

void AmigosModuleScl::_StopMiOut(unsigned int outPortId)
{
    AMIGOS_INFO("SCL DISABLE dev%d chn %d port %d\n", stModInfo.devId, stModInfo.chnId, outPortId);
    MI_SCL_DisableOutputPort((MI_SCL_DEV)this->stModInfo.devId, (MI_SCL_CHANNEL)this->stModInfo.chnId, (MI_SCL_PORT)outPortId);
}
stream_packet_info AmigosModuleScl::_GetStreamInfo(unsigned int outPortId)
{
    stream_packet_info streamInfo;
    auto itMapSclOut = this->mapSclOut.find(outPortId);
    if (itMapSclOut == this->mapSclOut.end())
    {
        AMIGOS_ERR("Can not find scl output port %d info!\n", outPortId);
        return streamInfo;
    }
    streamInfo.en_type = ss_enum_cast<stream_type>::from_str(itMapSclOut->second.strOutType);
    streamInfo.raw_vid_i.plane_num = 1;
    streamInfo.raw_vid_i.plane_info[0].fmt    = ss_enum_cast<raw_video_fmt>::from_str(itMapSclOut->second.strOutFmt);
    streamInfo.raw_vid_i.plane_info[0].width  = (MI_U16)itMapSclOut->second.uintSclOutWidth;
    streamInfo.raw_vid_i.plane_info[0].height = (MI_U16)itMapSclOut->second.uintSclOutHeight;
   return streamInfo;
}
#ifdef CONFIG_MI_SYSCALL_L2R_DESTROY_SUPPORT
bool AmigosModuleScl::_NeedMarkDeinitOnRtos()
{
    return true;
}
bool AmigosModuleScl::_NeedMarkStopOnRtos()
{
    return true;
}
bool AmigosModuleScl::_NeedMarkUnbindOnRtos(unsigned int inPortId)
{
    return true;
}
bool AmigosModuleScl::_NeedMarkStopOutOnRtos(unsigned int outPortId)
{
    return true;
}
#endif

AMIGOS_MODULE_INIT("SCL", AmigosModuleScl);
