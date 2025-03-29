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
#include <assert.h>

#include <vector>
#include <string>

#include "amigos_surface_base.h"
#include "mi_sys_datatype.h"
#include "mi_vif.h"
#include "mi_sys.h"
#include "amigos_module_init.h"
#include "amigos_module_vif.h"
#include "ss_enum_cast.hpp"

std::map<unsigned int, unsigned int> AmigosModuleVif::mapGroupEnable;

AmigosModuleVif::AmigosModuleVif(const std::string &strInSection)
    : AmigosSurfaceVif(strInSection), AmigosModuleMiBase(this)
{
}
AmigosModuleVif::~AmigosModuleVif()
{
}
unsigned int AmigosModuleVif::GetModId() const
{
    return E_MI_MODULE_ID_VIF;
}
unsigned int AmigosModuleVif::GetInputType(unsigned int port) const
{
    return 0;
}
unsigned int AmigosModuleVif::GetOutputType(unsigned int port) const
{
    return E_MOD_PORT_TYPE_KERNEL | E_MOD_PORT_TYPE_USER;
}
void AmigosModuleVif::_ResourceInit()
{
#if INTERFACE_SENSOR
    MI_U32 u32CapWidth = 0, u32CapHeight = 0;
    AmigosModuleMiBase::stSensorDrvInfo_t stSnrDrvInfo;
    std::map<unsigned int, VifOutInfo>::iterator   itMapVifOut;
    std::map<unsigned int, unsigned int>::iterator itMapGrp;
    unsigned int uintGrpId = 0;
    memset(&stSnrDrvInfo, 0x0, sizeof(stSensorDrvInfo_t));
    GetSensorInfo(stSnrDrvInfo, (unsigned int)stVifInfo.intSensorId);
    u32CapWidth = stSnrDrvInfo.stSnrPlaneInfo.stCapRect.u16Width;
    u32CapHeight = stSnrDrvInfo.stSnrPlaneInfo.stCapRect.u16Height;
    uintGrpId = stModInfo.devId / 4;
    itMapGrp = mapGroupEnable.find(uintGrpId);
    if (itMapGrp == mapGroupEnable.end())
    {
        AMIGOS_INFO("Create Group %d\n", uintGrpId);
        mapGroupEnable[uintGrpId] = 0;
    }
    mapGroupEnable[uintGrpId]++;
    for (itMapVifOut = mapVifOutInfo.begin(); itMapVifOut != mapVifOutInfo.end(); itMapVifOut++)
    {
        if (itMapVifOut->second.intIsUseSnrFmt)
        {
            u32CapWidth = (MI_U16)(((unsigned int)itMapVifOut->second.intCropW && (unsigned int)itMapVifOut->second.intCropW < u32CapWidth)
                            ? (unsigned int)itMapVifOut->second.intCropW : u32CapWidth);
            u32CapHeight = (MI_U16)(((unsigned int)itMapVifOut->second.intCropH && (unsigned int)itMapVifOut->second.intCropH < u32CapHeight)
                            ? (unsigned int)itMapVifOut->second.intCropH : u32CapHeight);
        }
        else
        {
            u32CapWidth = (MI_U16)(((unsigned int)itMapVifOut->second.intWidth && (unsigned int)itMapVifOut->second.intWidth < u32CapWidth)
                            ? (unsigned int)itMapVifOut->second.intWidth : u32CapWidth);
            u32CapHeight = (MI_U16)(((unsigned int)itMapVifOut->second.intHeight && (unsigned int)itMapVifOut->second.intHeight < u32CapHeight)
                            ? (unsigned int)itMapVifOut->second.intHeight : u32CapHeight);
        }
    }
#endif
}
void AmigosModuleVif::_ResourceDeinit()
{
    unsigned int uintGrpId = stModInfo.devId / 4;
    auto itMapGrp = mapGroupEnable.find(uintGrpId);
    if (itMapGrp != mapGroupEnable.end())
    {
        itMapGrp->second--;
        if (!itMapGrp->second)
        {
            mapGroupEnable.erase(itMapGrp);
        }
    }
}

void AmigosModuleVif::_Init()
{
#if INTERFACE_SENSOR
    MI_VIF_DevAttr_t stDevAttr;
    MI_VIF_GroupAttr_t stGroupAttr;
    AmigosModuleMiBase::stSensorDrvInfo_t stSnrDrvInfo;
    std::map<unsigned int, unsigned int>::iterator itMapGrp;
    unsigned int uintGrpId = 0;

    memset(&stSnrDrvInfo, 0x0, sizeof(stSensorDrvInfo_t));
    memset(&stDevAttr, 0, sizeof(MI_VIF_DevAttr_t));

    GetSensorInfo(stSnrDrvInfo, (unsigned int)stVifInfo.intSensorId);
    uintGrpId = stModInfo.devId / 4;
    itMapGrp = mapGroupEnable.find(uintGrpId);
    if (itMapGrp == mapGroupEnable.end())
    {
        memset(&stGroupAttr, 0, sizeof(MI_VIF_GroupAttr_t));
        stGroupAttr.eIntfMode = (MI_VIF_IntfMode_e)stSnrDrvInfo.stPadInfo.eIntfMode;
        stGroupAttr.eWorkMode = (MI_VIF_WorkMode_e)stVifInfo.intWorkMode;
        stGroupAttr.eHDRType = (MI_VIF_HDRType_e)stVifInfo.intHdrType;
        stGroupAttr.eHDRFusionTpye = (MI_VIF_HDRFusionType_e)stSnrDrvInfo.stPadInfo.eHDRFusionType;
        stGroupAttr.u8HDRExposureMask = stVifInfo.intHdrExposureMask;

        switch (stSnrDrvInfo.stPadInfo.eIntfMode)
        {
        case E_MI_VIF_MODE_BT656:
        {
            stGroupAttr.eClkEdge = (MI_VIF_ClkEdge_e)stSnrDrvInfo.stPadInfo.unIntfAttr.stBt656Attr.eClkEdge;
        }
        break;
        case E_MI_VIF_MODE_MIPI:
        case E_MI_VIF_MODE_DIGITAL_CAMERA:
        case E_MI_VIF_MODE_BT1120_STANDARD:
        case E_MI_VIF_MODE_LVDS:
        {
            stGroupAttr.eClkEdge = E_MI_VIF_CLK_EDGE_DOUBLE;
        }
        break;
        default:
            assert(0);
        }
        stGroupAttr.u32GroupStitchMask = stVifInfo.uintStitchMask;
        MI_VIF_CreateDevGroup((MI_VIF_GROUP)uintGrpId, &stGroupAttr);
        AMIGOS_INFO("Create Group %d\n", uintGrpId);
        mapGroupEnable[uintGrpId] = 0;
    }
    mapGroupEnable[uintGrpId]++;

    stDevAttr.stInputRect.u16X = stSnrDrvInfo.stSnrPlaneInfo.stCapRect.u16X;
    stDevAttr.stInputRect.u16Y = stSnrDrvInfo.stSnrPlaneInfo.stCapRect.u16Y;
    stDevAttr.stInputRect.u16Width = stSnrDrvInfo.stSnrPlaneInfo.stCapRect.u16Width;
    stDevAttr.stInputRect.u16Height = stSnrDrvInfo.stSnrPlaneInfo.stCapRect.u16Height;
    AMIGOS_INFO("u16X:%d u16Y:%d u16Width:%d u16Height:%d\n", stDevAttr.stInputRect.u16X, stDevAttr.stInputRect.u16Y,
              stDevAttr.stInputRect.u16Width, stDevAttr.stInputRect.u16Height);
    if(stSnrDrvInfo.stSnrPlaneInfo.eBayerId >= E_MI_SYS_PIXEL_BAYERID_MAX)
    {
        stDevAttr.eInputPixel = (MI_SYS_PixelFormat_e)stSnrDrvInfo.stSnrPlaneInfo.ePixel;
    }
    else
    {
        stDevAttr.eInputPixel = (MI_SYS_PixelFormat_e)RGB_BAYER_PIXEL(stSnrDrvInfo.stSnrPlaneInfo.ePixPrecision, stSnrDrvInfo.stSnrPlaneInfo.eBayerId);
    }
    MI_VIF_SetDevAttr((MI_VIF_DEV)stModInfo.devId, &stDevAttr);
    MI_VIF_EnableDev((MI_VIF_DEV)stModInfo.devId);
#endif
}
void AmigosModuleVif::_Deinit()
{
    std::map<unsigned int, unsigned int>::iterator itMapGrp;
    unsigned int uintGrpId = 0;

    MI_VIF_DisableDev((MI_VIF_DEV)stModInfo.devId);
    uintGrpId = stModInfo.devId / 4;
    itMapGrp = mapGroupEnable.find(uintGrpId);
    if (itMapGrp != mapGroupEnable.end())
    {
        itMapGrp->second--;
        if (!itMapGrp->second)
        {
            MI_VIF_DestroyDevGroup((MI_VIF_GROUP)uintGrpId);
            AMIGOS_INFO("Destroy Group %d\n", uintGrpId);
            mapGroupEnable.erase(itMapGrp);
        }
    }
}
void AmigosModuleVif::_StartMiOut(unsigned int outPortId)
{
    std::map<unsigned int, VifOutInfo>::iterator itMapVifOut;
    MI_VIF_OutputPortAttr_t stVifPortInfo;
    MI_SYS_PixelFormat_e ePixFormat;
    MI_U32 u32CapWidth = 0, u32CapHeight = 0;

    itMapVifOut = mapVifOutInfo.find(outPortId);
    if (itMapVifOut == mapVifOutInfo.end())
    {
        AMIGOS_ERR("Can not find vif output port %d info!\n", outPortId);
        return;
    }
    AmigosModuleMiBase::stSensorDrvInfo_t stSnrDrvInfo;
    GetSensorInfo(stSnrDrvInfo, (unsigned int)stVifInfo.intSensorId);
    u32CapWidth = stSnrDrvInfo.stSnrPlaneInfo.stCapRect.u16Width;
    u32CapHeight = stSnrDrvInfo.stSnrPlaneInfo.stCapRect.u16Height;
    memset(&stVifPortInfo, 0, sizeof(MI_VIF_OutputPortAttr_t));
    stVifPortInfo.stCapRect.u16X = (MI_U16)((unsigned int)itMapVifOut->second.intCropX < u32CapWidth
                                            ? (unsigned int)itMapVifOut->second.intCropX : stSnrDrvInfo.stSnrPlaneInfo.stCapRect.u16X);
    stVifPortInfo.stCapRect.u16Y = (MI_U16)((unsigned int)itMapVifOut->second.intCropY < u32CapHeight
                                            ? (unsigned int)itMapVifOut->second.intCropX : stSnrDrvInfo.stSnrPlaneInfo.stCapRect.u16Y);
    stVifPortInfo.stCapRect.u16Width = (MI_U16)(((unsigned int)itMapVifOut->second.intCropW && (unsigned int)itMapVifOut->second.intCropW < u32CapWidth)
                                                ? (unsigned int)itMapVifOut->second.intCropW : u32CapWidth);
    stVifPortInfo.stCapRect.u16Height = (MI_U16)(((unsigned int)itMapVifOut->second.intCropH && (unsigned int)itMapVifOut->second.intCropH < u32CapHeight)
                                                 ? (unsigned int)itMapVifOut->second.intCropH : u32CapHeight);
    if (itMapVifOut->second.intIsUseSnrFmt)
    {
        if(stSnrDrvInfo.stSnrPlaneInfo.eBayerId == E_MI_SYS_PIXEL_BAYERID_MAX)
        {
            ePixFormat = (MI_SYS_PixelFormat_e)stSnrDrvInfo.stSnrPlaneInfo.ePixel;
        }
        else
        {
            ePixFormat = (MI_SYS_PixelFormat_e)RGB_BAYER_PIXEL(stSnrDrvInfo.stSnrPlaneInfo.ePixPrecision, stSnrDrvInfo.stSnrPlaneInfo.eBayerId);
        }
        stVifPortInfo.stDestSize.u16Width = (MI_U16)stVifPortInfo.stCapRect.u16Width;
        stVifPortInfo.stDestSize.u16Height = (MI_U16)stVifPortInfo.stCapRect.u16Height;
    }
    else
    {
        if("bayer" == itMapVifOut->second.strOutFmt)
        {
            ePixFormat = ss_enum_cast<MI_SYS_PixelFormat_e>::from_str_bayer(itMapVifOut->second.strOutFmt, \
                                                                            itMapVifOut->second.strBayerId,itMapVifOut->second.strPrecision);
        }
        else
        {
            ePixFormat = ss_enum_cast<MI_SYS_PixelFormat_e>::from_str(itMapVifOut->second.strOutFmt);
        }
        stVifPortInfo.stDestSize.u16Width = (MI_U16)(((unsigned int)itMapVifOut->second.intWidth && (unsigned int)itMapVifOut->second.intWidth < u32CapWidth)
                                                     ? (unsigned int)itMapVifOut->second.intWidth : u32CapWidth);
        stVifPortInfo.stDestSize.u16Height = (MI_U16)(((unsigned int)itMapVifOut->second.intHeight && (unsigned int)itMapVifOut->second.intHeight < u32CapHeight)
                                                      ? (unsigned int)itMapVifOut->second.intHeight : u32CapHeight);
    }
    stVifPortInfo.ePixFormat = ePixFormat;//E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420;
    stVifPortInfo.eFrameRate = E_MI_VIF_FRAMERATE_FULL;
    stVifPortInfo.eCompressMode = (MI_SYS_CompressMode_e)itMapVifOut->second.intCompressMode;
    AMIGOS_INFO("VIF Output %d Crop: X: %d, Y: %d, W: %d, H: %d\n", itMapVifOut->first,
                stVifPortInfo.stCapRect.u16X, stVifPortInfo.stCapRect.u16Y, stVifPortInfo.stCapRect.u16Width,
                stVifPortInfo.stCapRect.u16Height);
    MI_VIF_SetOutputPortAttr((MI_VIF_DEV)stModInfo.devId, itMapVifOut->first, &stVifPortInfo);
    MI_VIF_EnableOutputPort((MI_VIF_DEV)stModInfo.devId, itMapVifOut->first);
}
void AmigosModuleVif::_StopMiOut(unsigned int outPortId)
{
    MI_VIF_DisableOutputPort((MI_VIF_DEV)stModInfo.devId, outPortId);
}
stream_packet_info AmigosModuleVif::_GetStreamInfo(unsigned int outPortId)
{
    stream_packet_info streamInfo;
    auto itMapVifOut = this->mapVifOutInfo.find(outPortId);
    if (itMapVifOut == this->mapVifOutInfo.end())
    {
        AMIGOS_ERR("Can not find vif output port %d info!\n", outPortId);
        return streamInfo;
    }
    streamInfo.en_type = ss_enum_cast<stream_type>::from_str(itMapVifOut->second.strOutType);
    streamInfo.raw_vid_i.plane_num = 1;
    streamInfo.raw_vid_i.plane_info[0].width = itMapVifOut->second.intCropW;
    streamInfo.raw_vid_i.plane_info[0].height = itMapVifOut->second.intCropH;
    return streamInfo;
}
#ifdef CONFIG_MI_SYSCALL_L2R_DESTROY_SUPPORT
bool AmigosModuleVif::_NeedMarkDeinitOnRtos()
{
    return true;
}
bool AmigosModuleVif::_NeedMarkUnbindOnRtos(unsigned int inPortId)
{
    return true;
}
bool AmigosModuleVif::_NeedMarkStopOutOnRtos(unsigned int outPortId)
{
    return true;
}
#endif

AMIGOS_MODULE_INIT("VIF", AmigosModuleVif);
