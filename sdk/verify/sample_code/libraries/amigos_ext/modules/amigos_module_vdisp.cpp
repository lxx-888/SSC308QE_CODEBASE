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
#include <mi_sys_datatype.h>
#include <mi_sys.h>
#include <mi_vdisp_datatype.h>
#include <mi_vdisp.h>
#include "amigos_module_init.h"
#include "amigos_module_vdisp.h"
#include "ss_enum_cast.hpp"

#define MAKE_YUYV_VALUE(y,u,v)  ((y) << 24) | ((u) << 16) | ((y) << 8) | (v)
#define YUYV_BLACK              MAKE_YUYV_VALUE(0,128,128)
#define YUYV_WHITE              MAKE_YUYV_VALUE(255,128,128)
#define YUYV_RED                MAKE_YUYV_VALUE(76,84,255)
#define YUYV_GREEN              MAKE_YUYV_VALUE(149,43,21)
#define YUYV_BLUE               MAKE_YUYV_VALUE(29,225,107)


SS_ENUM_CAST_STR(int,
{
    { YUYV_BLACK, "black" },
    { YUYV_WHITE, "white" },
    { YUYV_RED, "red" },
    { YUYV_GREEN, "green" },
    { YUYV_BLUE, "blue" },
});

typedef struct stSys_BindInfo_s
{
    MI_SYS_ChnPort_t stSrcChnPort;
    MI_SYS_ChnPort_t stDstChnPort;
    MI_U32 u32SrcFrmrate;
    MI_U32 u32DstFrmrate;
    MI_SYS_BindType_e eBindType;
    MI_U32 u32BindParam;
} stSys_BindInfo_T;
AmigosModuleVdisp::AmigosModuleVdisp(const std::string &strInSection)
    : AmigosSurfaceVdisp(strInSection), AmigosModuleMiBase(this)
{
}
AmigosModuleVdisp::~AmigosModuleVdisp()
{
}
unsigned int AmigosModuleVdisp::GetModId() const
{
    return E_MI_MODULE_ID_VDISP;
}
unsigned int AmigosModuleVdisp::GetInputType(unsigned int port) const
{
    return E_MOD_PORT_TYPE_KERNEL | E_MOD_PORT_TYPE_USER;
}
unsigned int AmigosModuleVdisp::GetOutputType(unsigned int port) const
{
    return E_MOD_PORT_TYPE_KERNEL | E_MOD_PORT_TYPE_USER;
}
void AmigosModuleVdisp::_Init()
{
    MI_VDISP_InputChnAttr_t stInputChnAttr;
    MI_VDISP_OutputPortAttr_t stOutputPortAttr;
    std::vector<VdispInputInfo>::iterator itVdispIn;
    std::vector<VdispOutputInfo>::iterator itVdispOut;
    MI_VDISP_Init();
    MI_VDISP_OpenDevice(stModInfo.devId);
    //set input port attr
    for (itVdispIn = vVdispInputInfo.begin(); itVdispIn != vVdispInputInfo.end(); itVdispIn++)
    {
        if (itVdispIn->intFreeRun == 1)
        {
            stInputChnAttr.s32IsFreeRun = TRUE;
        }
        else
        {
            stInputChnAttr.s32IsFreeRun = FALSE;
        }
        stInputChnAttr.u32OutX = itVdispIn->intVdispInX;
        stInputChnAttr.u32OutY = itVdispIn->intVdispInY;
        stInputChnAttr.u32OutWidth = itVdispIn->intVdispInWidth;
        stInputChnAttr.u32OutHeight = itVdispIn->intVdispInHeight;
        MI_VDISP_SetInputChannelAttr(stModInfo.devId, (MI_VDISP_CHN)itVdispIn->intChnId, &stInputChnAttr);
    }
    for (itVdispOut = vVdispOutputInfo.begin(); itVdispOut != vVdispOutputInfo.end(); itVdispOut++)
    {
        memset(&stOutputPortAttr, 0, sizeof(MI_VDISP_OutputPortAttr_t));
        stOutputPortAttr.u32FrmRate = itVdispOut->intVdispOutFrameRate;
        stOutputPortAttr.u32Height = itVdispOut->intVdispOutHeight;
        stOutputPortAttr.u32Width = itVdispOut->intVdispOutWidth;
        stOutputPortAttr.u64pts = itVdispOut->intVdispOutPts;
        stOutputPortAttr.ePixelFormat = ss_enum_cast<MI_SYS_PixelFormat_e>::from_str(itVdispOut->strVdispOutFormat);
        stOutputPortAttr.u32BgColor = ss_enum_cast<int>::from_str(itVdispOut->strVdispOutBkColor);
        MI_VDISP_SetOutputPortAttr(stModInfo.devId, 0, &stOutputPortAttr);
    }
    for (itVdispIn = vVdispInputInfo.begin(); itVdispIn != vVdispInputInfo.end(); itVdispIn++)
    {
        MI_VDISP_EnableInputChannel(stModInfo.devId, (MI_VDISP_CHN)itVdispIn->intChnId);
    }
    MI_VDISP_StartDev(stModInfo.devId);
}
void AmigosModuleVdisp::_DirectBind(unsigned int inPortId, unsigned int prevOutPortId, const AmigosModuleBase *pPrev)
{
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
    stDstChnPort.u32ChnId  = inPortId;
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
void AmigosModuleVdisp::_DirectUnBind(unsigned int inPortId, unsigned int prevOutPortId, const AmigosModuleBase *pPrev)
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
    stDstChnPort.u32ChnId  = inPortId;
    stDstChnPort.u32PortId = 0;
    if (MI_SUCCESS != MI_SYS_UnBindChnPort(0, &stSrcChnPort, &stDstChnPort))
    {
        AMIGOS_ERR("MI_SYS_UnBindChnPort failed\n");
    }
    AMIGOS_INFO("UnBind %d-%d-%d-%d -> %d-%d-%d-%d\n", stSrcChnPort.eModId, stSrcChnPort.u32DevId,
                stSrcChnPort.u32ChnId, stSrcChnPort.u32PortId, stDstChnPort.eModId, stDstChnPort.u32DevId,
                stDstChnPort.u32ChnId, stDstChnPort.u32PortId);
}
void AmigosModuleVdisp::_Deinit()
{
    std::vector<VdispInputInfo>::iterator itVdispIn;
    MI_VDISP_StopDev(stModInfo.devId);
    for (itVdispIn = vVdispInputInfo.begin(); itVdispIn != vVdispInputInfo.end(); itVdispIn++)
    {
        MI_VDISP_DisableInputChannel(stModInfo.devId, (MI_VDISP_CHN)(itVdispIn->intChnId));
    }
    MI_VDISP_CloseDevice(stModInfo.devId);
    MI_VDISP_Exit();
}
AMIGOS_MODULE_INIT("VDISP", AmigosModuleVdisp);
