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
#include <unistd.h>
#include <string.h>
#include "mi_common.h"
#include "mi_sys.h"
#include "mi_hvp.h"
#include "mi_hdmirx.h"
#include "amigos_module_init.h"
#include "amigos_module_hvp.h"

AmigosModuleHvp::AmigosModuleHvp(const std::string &strInSection)
    : AmigosSurfaceHvp(strInSection), AmigosModuleMiBase(this)
{
}
AmigosModuleHvp::~AmigosModuleHvp()
{
}
unsigned int AmigosModuleHvp::GetModId() const
{
    return E_MI_MODULE_ID_HVP;
}
unsigned int AmigosModuleHvp::GetInputType(unsigned int port) const
{
    return 0;
}
unsigned int AmigosModuleHvp::GetOutputType(unsigned int port) const
{
    return E_MOD_PORT_TYPE_KERNEL;
}
void AmigosModuleHvp::_Init()
{
    MI_S32 s32Ret = -1;
    MI_HVP_DEV devId = stModInfo.devId;
    MI_HVP_CHN chnId = stModInfo.chnId;
    MI_HVP_DeviceAttr_t stDevAttr;
    MI_HVP_ChannelAttr_t stChnAttr;
    MI_HVP_ChannelParam_t stChnParam;
    MI_BOOL Trigger;
    MI_SYS_ChnPort_t OutputPort;

    memset(&stDevAttr, 0, sizeof(MI_HVP_DeviceAttr_t));
    stDevAttr.enSrcType = E_MI_HVP_SRC_TYPE_HDMI;
    s32Ret = MI_HVP_CreateDevice(devId, &stDevAttr);
    if(s32Ret != MI_SUCCESS)
    {
        AMIGOS_ERR("MI_HVP_CreateDevice err 0x%x\n", s32Ret);
        return;
    }

    memset(&stChnAttr, 0, sizeof(MI_HVP_ChannelAttr_t));
    stChnAttr.enFrcMode = (MI_HVP_FrcMode_e)stHvpConf.FrcMode;
    if (stChnAttr.enFrcMode == E_MI_HVP_FRC_MODE_RATIO || stChnAttr.enFrcMode == E_MI_HVP_FRC_MODE_LOCK_OUT)
    {
        //config pqbuf
        stChnAttr.stPqBufModeConfig.eDmaColor        = (MI_HVP_ColorFormat_e)stHvpConf.DMAColor;
        stChnAttr.stPqBufModeConfig.u16BufMaxCount   = stHvpConf.BufMaxCnt;
        stChnAttr.stPqBufModeConfig.u16BufMaxWidth   = stHvpConf.BufMaxWidth;
        stChnAttr.stPqBufModeConfig.u16BufMaxHeight  = stHvpConf.BufMaxHeight;
        stChnAttr.stPqBufModeConfig.eBufCompressMode = (MI_SYS_CompressMode_e)stHvpConf.CompressMode;
        stChnAttr.stPqBufModeConfig.eFieldType       = (MI_SYS_FieldType_e)stHvpConf.FieldType;
    }
    s32Ret = MI_HVP_CreateChannel(devId, chnId, &stChnAttr);

    memset(&stChnParam, 0, sizeof(MI_HVP_ChannelParam_t));
    MI_HVP_GetChannelParam(devId, chnId, &stChnParam);
    stChnParam.stChnSrcParam.enPixRepType     = E_MI_HVP_PIX_REP_TYPE_1X;
    stChnParam.stChnSrcParam.enInputColor     = (MI_HVP_ColorFormat_e)stHvpConf.InputColor;
    stChnParam.stChnSrcParam.enColorDepth     = (MI_HVP_ColorDepth_e)stHvpConf.ColorDepth;
    stChnParam.stChnSrcParam.stCropWin.u16X   = 0;
    stChnParam.stChnSrcParam.stCropWin.u16Y   = 0;
    stChnParam.stChnSrcParam.stCropWin.u16Width = stHvpConf.BufMaxWidth;
    stChnParam.stChnSrcParam.stCropWin.u16Height = stHvpConf.BufMaxHeight;

    stChnParam.stChnDstParam.u16Width  = stHvpConf.OutWidth;
    stChnParam.stChnDstParam.u16Height = stHvpConf.OutHeight;

    //DstCrop and DstWin work only for DISP
    stChnParam.stChnDstParam.enColor = (MI_HVP_ColorFormat_e)stHvpConf.DestColor;  //E_MI_HVP_COLOR_FORMAT_YUV444
    stChnParam.stChnDstParam.stCropWin.u16X      = 0;
    stChnParam.stChnDstParam.stCropWin.u16Y      = 0;
    stChnParam.stChnDstParam.stCropWin.u16Width  = stHvpConf.OutWidth;
    stChnParam.stChnDstParam.stCropWin.u16Height = stHvpConf.OutHeight;

    stChnParam.stChnDstParam.stDispWin.u16X      = 0;
    stChnParam.stChnDstParam.stDispWin.u16Y      = 0;
    stChnParam.stChnDstParam.stDispWin.u16Width  = stHvpConf.OutWidth;
    stChnParam.stChnDstParam.stDispWin.u16Height = stHvpConf.OutHeight;

    stChnParam.stChnDstParam.u16Fpsx100 = stHvpConf.Fpsx100;
    s32Ret = MI_HVP_SetChannelParam(devId, chnId, &stChnParam);

    OutputPort.eModId = E_MI_MODULE_ID_HVP;
    OutputPort.u32DevId = devId;
    OutputPort.u32ChnId = chnId;
    OutputPort.u32PortId = 0;
    MI_SYS_SetChnOutputPortDepth(0, &OutputPort, 1, 2);

    s32Ret = MI_HVP_StartChannel(devId, chnId);

    s32Ret = MI_HVP_GetResetEvent(devId, &Trigger);
    if(Trigger)
    {
        MI_HVP_ClearResetEvent(devId);
    }
}

void AmigosModuleHvp::_Deinit()
{
    MI_HVP_DEV devId = 0;
    MI_HVP_CHN chnId = 0;

    MI_HVP_StopChannel(devId, chnId);
    MI_HVP_DestroyChannel(devId, chnId);
    MI_HVP_DestroyDevice(devId);
}
AMIGOS_MODULE_INIT("HVP", AmigosModuleHvp);
