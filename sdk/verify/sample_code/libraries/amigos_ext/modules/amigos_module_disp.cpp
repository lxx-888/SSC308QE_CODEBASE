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

#include <vector>
#include <string>

#include "mi_sys.h"
#ifdef INTERFACE_HDMI
#include "mi_hdmi.h"
#endif
#include "mi_disp.h"
#include "amigos_module_init.h"
#include "amigos_module_disp.h"
#include "ss_enum_cast.hpp"

#define MAKE_YUYV_VALUE(y,u,v)  ((y) << 24) | ((u) << 16) | ((y) << 8) | (v)
#define YUYV_BLACK              MAKE_YUYV_VALUE(0,128,128)
#define YUYV_WHITE              MAKE_YUYV_VALUE(255,128,128)
#define YUYV_RED                MAKE_YUYV_VALUE(76,84,255)
#define YUYV_GREEN              MAKE_YUYV_VALUE(149,43,21)
#define YUYV_BLUE               MAKE_YUYV_VALUE(29,225,107)

SS_ENUM_CAST_STR(MI_DISP_Interface_e,
{
    { E_MI_DISP_INTF_TTL, "ttl" },
    { E_MI_DISP_INTF_MCU, "mcu" },
    { E_MI_DISP_INTF_SRGB, "srgb" },
    { E_MI_DISP_INTF_BT1120, "bt1120" },
    { E_MI_DISP_INTF_BT656, "bt656" },
    { E_MI_DISP_INTF_MIPIDSI, "mipi" },
    { E_MI_DISP_INTF_CVBS, "cvbs" },
    { E_MI_DISP_INTF_VGA, "vga" },
});

SS_ENUM_CAST_STR(MI_DISP_OutputTiming_e,
{
    { E_MI_DISP_OUTPUT_USER, "user" },
    { E_MI_DISP_OUTPUT_NTSC, "ntsc" },
    { E_MI_DISP_OUTPUT_PAL, "pal" },
    { E_MI_DISP_OUTPUT_720P30, "720p30" },
    { E_MI_DISP_OUTPUT_720P50, "720p50" },
    { E_MI_DISP_OUTPUT_720P60, "720p60" },
    { E_MI_DISP_OUTPUT_1024x768_60, "1024x768p60" },
    { E_MI_DISP_OUTPUT_1280x1024_60, "1280x1024p60" },
    { E_MI_DISP_OUTPUT_1366x768_60, "1366x768p60" },
    { E_MI_DISP_OUTPUT_1080P24, "1080p24" },
    { E_MI_DISP_OUTPUT_1080P30, "1080p30" },
    { E_MI_DISP_OUTPUT_1080P50, "1080p50" },
    { E_MI_DISP_OUTPUT_1080P60, "1080p60" },
    { E_MI_DISP_OUTPUT_3840x2160_30, "3840x2160p30" },
    { E_MI_DISP_OUTPUT_3840x2160_60, "3840x2160p60" },
});

SS_ENUM_CAST_STR(int,
{
    { YUYV_BLACK, "black" },
    { YUYV_WHITE, "white" },
    { YUYV_RED, "red" },
    { YUYV_GREEN, "green" },
    { YUYV_BLUE, "blue" },
});


AmigosModuleDisp::AmigosModuleDisp(const std::string &strInSection)
    : AmigosSurfaceDisp(strInSection), AmigosModuleMiBase(this)
{
}
AmigosModuleDisp::~AmigosModuleDisp()
{
}
unsigned int AmigosModuleDisp::GetModId() const
{
    return E_MI_MODULE_ID_DISP;
}
unsigned int AmigosModuleDisp::GetInputType(unsigned int port) const
{
    return E_MOD_PORT_TYPE_KERNEL | E_MOD_PORT_TYPE_USER;
}
unsigned int AmigosModuleDisp::GetOutputType(unsigned int port) const
{
    return 0;
}
#if INTERFACE_HDMI
static int HdmiCb(MI_HDMI_DeviceId_e eHdmi, MI_HDMI_EventType_e Event, void *pEventParam, void *pUsrParam)
{
    switch (Event)
    {
        case E_MI_HDMI_EVENT_HOTPLUG:
            AMIGOS_INFO("E_MI_HDMI_EVENT_HOTPLUG.\n");
            break;
        case E_MI_HDMI_EVENT_NO_PLUG:
            AMIGOS_INFO("E_MI_HDMI_EVENT_NO_PLUG.\n");
            break;
        default:
            AMIGOS_ERR("Unsupport event.\n");
            break;
    }

    return MI_SUCCESS;
}
#endif

void AmigosModuleDisp::_Init()
{
    MI_U8 u8LayerId = 0;
    MI_U8 u8LayerPortId = 0;
    MI_DISP_PubAttr_t stPubAttr;
    MI_DISP_VideoLayerAttr_t stLayerAttr;
    MI_DISP_InputPortAttr_t stInputPortAttr;
    MI_DISP_RotateConfig_t stRotateConfig;
    std::map<unsigned int, DispLayerInfo>::iterator itMapLayerInfo;

    //pub attr
    memset(&stPubAttr, 0, sizeof(MI_DISP_PubAttr_t));
    memset(&stLayerAttr, 0, sizeof(MI_DISP_VideoLayerAttr_t));
    memset(&stInputPortAttr, 0, sizeof(MI_DISP_InputPortAttr_t));
    stPubAttr.u32BgColor = ss_enum_cast<int>::from_str(stDispInfo.strBackGroundColor);
    stPubAttr.eIntfSync = ss_enum_cast<MI_DISP_OutputTiming_e>::from_str(stDispInfo.strOutTiming);
    if (stDispInfo.strDevType == "panel")
    {
        stPubAttr.eIntfType = ss_enum_cast<MI_DISP_Interface_e>::from_str(stDispInfo.strPnlLinkType);
    }
    else if (stDispInfo.strDevType == "vga")
    {
        stPubAttr.eIntfType = E_MI_DISP_INTF_VGA;
    }
    else if (stDispInfo.strDevType == "cvbs_out")
    {
        stPubAttr.eIntfType = E_MI_DISP_INTF_CVBS;
    }
    else if (stDispInfo.strDevType == "hdmi")
    {
#ifdef INTERFACE_HDMI
        MI_HDMI_InitParam_t stInitParam;
        MI_HDMI_Attr_t stAttr;

        stInitParam.pCallBackArgs = NULL;
        stInitParam.pfnHdmiEventCallback = HdmiCb;
        MI_HDMI_Init(&stInitParam);
        MI_HDMI_Open(E_MI_HDMI_ID_0);

        memset(&stAttr, 0, sizeof(MI_HDMI_Attr_t));
        stAttr.stEnInfoFrame.bEnableAudInfoFrame  = FALSE;
        stAttr.stEnInfoFrame.bEnableAviInfoFrame  = FALSE;
        stAttr.stEnInfoFrame.bEnableSpdInfoFrame  = FALSE;
        stAttr.stAudioAttr.bEnableAudio = TRUE;
        stAttr.stAudioAttr.bIsMultiChannel = 0;
        stAttr.stAudioAttr.eBitDepth = E_MI_HDMI_BIT_DEPTH_16;
        stAttr.stAudioAttr.eCodeType = E_MI_HDMI_ACODE_PCM;
        stAttr.stAudioAttr.eSampleRate = E_MI_HDMI_AUDIO_SAMPLERATE_48K;
        stAttr.stVideoAttr.bEnableVideo = TRUE;
        stAttr.stVideoAttr.eColorType = E_MI_HDMI_COLOR_TYPE_RGB444;//default color type
        stAttr.stVideoAttr.eDeepColorMode = E_MI_HDMI_DEEP_COLOR_MAX;
        switch (stPubAttr.eIntfSync)
        {
            case E_MI_DISP_OUTPUT_NTSC:
            case E_MI_DISP_OUTPUT_480P60:
                stAttr.stVideoAttr.eTimingType = E_MI_HDMI_TIMING_480_60P;
                break;
            case E_MI_DISP_OUTPUT_PAL:
            case E_MI_DISP_OUTPUT_576P50:
                stAttr.stVideoAttr.eTimingType = E_MI_HDMI_TIMING_576_50P;
                break;
            case E_MI_DISP_OUTPUT_720P50:
                stAttr.stVideoAttr.eTimingType = E_MI_HDMI_TIMING_720_50P;
                break;
            case E_MI_DISP_OUTPUT_720P60:
                stAttr.stVideoAttr.eTimingType = E_MI_HDMI_TIMING_720_60P;
                break;
            case E_MI_DISP_OUTPUT_1080P60:
                stAttr.stVideoAttr.eTimingType = E_MI_HDMI_TIMING_1080_60P;
                break;
            case E_MI_DISP_OUTPUT_3840x2160_30:
                stAttr.stVideoAttr.eTimingType = E_MI_HDMI_TIMING_4K2K_30P;
                break;
            case E_MI_DISP_OUTPUT_3840x2160_60:
                stAttr.stVideoAttr.eTimingType = E_MI_HDMI_TIMING_4K2K_60P;
                break;
            default:
                stAttr.stVideoAttr.eTimingType = E_MI_HDMI_TIMING_1080_60P;
                break;
        }
        stAttr.stVideoAttr.eOutputMode = E_MI_HDMI_OUTPUT_MODE_HDMI;
        MI_HDMI_SetAttr(E_MI_HDMI_ID_0, &stAttr);
        MI_HDMI_Start(E_MI_HDMI_ID_0);
        stPubAttr.eIntfType = E_MI_DISP_INTF_HDMI;
#endif
    }
    else
    {
        AMIGOS_ERR("Not support current device type!\n");
        return;
    }
    //set disp pub
    MI_DISP_SetPubAttr((MI_DISP_DEV)stModInfo.devId,  &stPubAttr);
    MI_DISP_Enable((MI_DISP_DEV)stModInfo.devId);

    for (itMapLayerInfo = mapLayerInfo.begin(); itMapLayerInfo != mapLayerInfo.end(); ++itMapLayerInfo)
    {
        u8LayerId = itMapLayerInfo->second.uintId;
        // set layer
        MI_DISP_BindVideoLayer((MI_DISP_LAYER)u8LayerId, (MI_DISP_DEV)stModInfo.devId);
        stLayerAttr.ePixFormat = E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420;
        stLayerAttr.stVidLayerSize.u16Width = itMapLayerInfo->second.uintWidth;
        stLayerAttr.stVidLayerSize.u16Height = itMapLayerInfo->second.uintHeight;
        stLayerAttr.stVidLayerDispWin.u16Width = itMapLayerInfo->second.uintDispWidth;
        stLayerAttr.stVidLayerDispWin.u16Height = itMapLayerInfo->second.uintDispHeight;
        stLayerAttr.stVidLayerDispWin.u16X = itMapLayerInfo->second.uintDispXpos;
        stLayerAttr.stVidLayerDispWin.u16Y = itMapLayerInfo->second.uintDispYpos;
        MI_DISP_SetVideoLayerAttr((MI_DISP_LAYER)u8LayerId, &stLayerAttr);
        MI_DISP_EnableVideoLayer((MI_DISP_LAYER)u8LayerId);
        //rotate
        stRotateConfig.eRotateMode = (MI_DISP_RotateMode_e)itMapLayerInfo->second.uintRot;
        MI_DISP_SetVideoLayerRotateMode((MI_DISP_LAYER)u8LayerId, &stRotateConfig);
    }
    for (unsigned int i = 0; i < mapDispInInfo.size(); i++)
    {
        u8LayerId = mapDispInInfo[i].uintLayerId;
        u8LayerPortId = mapDispInInfo[i].uintLayerPortId;
        stInputPortAttr.u16SrcWidth = mapDispInInfo[i].uintSrcWidth;
        stInputPortAttr.u16SrcHeight = mapDispInInfo[i].uintSrcHeight;
        stInputPortAttr.stDispWin.u16X = mapDispInInfo[i].uintDstXpos;
        stInputPortAttr.stDispWin.u16Y = mapDispInInfo[i].uintDstYpos;
        stInputPortAttr.stDispWin.u16Width = mapDispInInfo[i].uintDstWidth;
        stInputPortAttr.stDispWin.u16Height = mapDispInInfo[i].uintDstHeight;
        MI_DISP_SetInputPortAttr(u8LayerId, u8LayerPortId, &stInputPortAttr);
    }
}
void AmigosModuleDisp::_DirectBind(unsigned int inPortId, unsigned int prevOutPortId, const AmigosModuleBase *pPrev)
{
    // disp inputport enable should eariler than bind
    MI_U8 u8LayerId = 0;
    MI_U8 u8LayerPortId = 0;
    u8LayerId = mapDispInInfo[inPortId].uintLayerId;
    u8LayerPortId = mapDispInInfo[inPortId].uintLayerPortId;
    //enable inputport
    MI_DISP_EnableInputPort(u8LayerId, u8LayerPortId);
    MI_DISP_SetInputPortSyncMode(u8LayerId, u8LayerPortId, E_MI_DISP_SYNC_MODE_FREE_RUN);

    MI_SYS_ChnPort_t  stSrcChnPort;
    MI_SYS_ChnPort_t  stDstChnPort;

    AmigosSurfaceBase::ModPortInInfo stInPortInfo;
    if (!this->GetPortInInfo(inPortId, stInPortInfo))
    {
        AMIGOS_ERR("Get curr mod port in info error\n");
        return;
    }
    AmigosSurfaceBase::ModPortOutInfo stPrevOutPortInfo;
    if (!pPrev->GetSurface()->GetPortOutInfo(prevOutPortId, stPrevOutPortInfo))
    {
        AMIGOS_ERR("Get prev mod port out info error\n");
        return;
    }

    memset(&stSrcChnPort, 0, sizeof(MI_SYS_ChnPort_t));
    memset(&stDstChnPort, 0, sizeof(MI_SYS_ChnPort_t));

    const AmigosSurfaceBase::ModInfo &stSrcModInfo = pPrev->GetSurface()->GetModInfo();
    stSrcChnPort.eModId    = (MI_ModuleId_e)pPrev->GetModId();
    stSrcChnPort.u32DevId  = stSrcModInfo.devId;
    stSrcChnPort.u32ChnId  = stSrcModInfo.chnId;
    stSrcChnPort.u32PortId = prevOutPortId;

    const AmigosSurfaceBase::ModInfo &stDstModInfo = this->GetModInfo();
    stDstChnPort.eModId    = (MI_ModuleId_e)this->GetModId();
    stDstChnPort.u32DevId  = stDstModInfo.devId;
    stDstChnPort.u32ChnId  = mapDispInInfo[inPortId].uintSysChn;
    stDstChnPort.u32PortId = 0;

    // Bind
    if (MI_SUCCESS
        != MI_SYS_BindChnPort2(0, &stSrcChnPort, &stDstChnPort, stPrevOutPortInfo.curFrmRate, stInPortInfo.curFrmRate,
                               (MI_SYS_BindType_e)stInPortInfo.bindType, stInPortInfo.bindPara))
    {
        AMIGOS_ERR("MI_SYS_BindChnPort2 failed\n");
    }
    AMIGOS_INFO("Bind %d-%d-%d-%d -> %d-%d-%d-%d\n", stSrcChnPort.eModId, stSrcChnPort.u32DevId, stSrcChnPort.u32ChnId,
                stSrcChnPort.u32PortId, stDstChnPort.eModId, stDstChnPort.u32DevId, stDstChnPort.u32ChnId,
                stDstChnPort.u32PortId);
}
void AmigosModuleDisp::_DirectUnBind(unsigned int inPortId, unsigned int prevOutPortId, const AmigosModuleBase *pPrev)
{
    // UnBind
    MI_SYS_ChnPort_t stSrcChnPort;
    MI_SYS_ChnPort_t stDstChnPort;

    memset(&stSrcChnPort, 0, sizeof(MI_SYS_ChnPort_t));
    memset(&stDstChnPort, 0, sizeof(MI_SYS_ChnPort_t));

    const AmigosSurfaceBase::ModInfo &stSrcModInfo = pPrev->GetSurface()->GetModInfo();
    stSrcChnPort.eModId    = (MI_ModuleId_e)pPrev->GetModId();
    stSrcChnPort.u32DevId  = stSrcModInfo.devId;
    stSrcChnPort.u32ChnId  = stSrcModInfo.chnId;
    stSrcChnPort.u32PortId = prevOutPortId;

    const AmigosSurfaceBase::ModInfo &stDstModInfo = this->GetModInfo();
    stDstChnPort.eModId    = (MI_ModuleId_e)this->GetModId();
    stDstChnPort.u32DevId  = stDstModInfo.devId;
    stDstChnPort.u32ChnId  = mapDispInInfo[inPortId].uintSysChn;
    stDstChnPort.u32PortId = inPortId;

    if (MI_SUCCESS != MI_SYS_UnBindChnPort(0, &stSrcChnPort, &stDstChnPort))
    {
        AMIGOS_ERR("MI_SYS_UnBindChnPort failed\n");
    }
    AMIGOS_INFO("UnBind %d-%d-%d-%d -> %d-%d-%d-%d\n", stSrcChnPort.eModId, stSrcChnPort.u32DevId,
                stSrcChnPort.u32ChnId, stSrcChnPort.u32PortId, stDstChnPort.eModId, stDstChnPort.u32DevId,
                stDstChnPort.u32ChnId, stDstChnPort.u32PortId);

    // dispable inputport
    MI_U8 u8LayerId = 0;
    MI_U8 u8LayerPortId = 0;
    u8LayerId = mapDispInInfo[inPortId].uintLayerId;
    u8LayerPortId = mapDispInInfo[inPortId].uintLayerPortId;
    MI_DISP_DisableInputPort(u8LayerId, u8LayerPortId);
}
void AmigosModuleDisp::_Deinit()
{
    MI_U8 u8LayerId = 0;
    std::map<unsigned int, DispLayerInfo>::iterator itMapLayerInfo;

    for (itMapLayerInfo = mapLayerInfo.begin(); itMapLayerInfo != mapLayerInfo.end(); ++itMapLayerInfo)
    {
        u8LayerId = itMapLayerInfo->second.uintId;
        MI_DISP_DisableVideoLayer(u8LayerId);
        MI_DISP_UnBindVideoLayer(u8LayerId, (MI_DISP_DEV)stModInfo.devId);
    }
    MI_DISP_Disable((MI_DISP_DEV)stModInfo.devId);
    if (stDispInfo.strDevType == "hdmi")
    {
#ifdef INTERFACE_HDMI
        MI_HDMI_Stop(E_MI_HDMI_ID_0);
        MI_HDMI_Close(E_MI_HDMI_ID_0);
        MI_HDMI_DeInit();
#endif
    }
}

AMIGOS_MODULE_INIT("DISP", AmigosModuleDisp);
