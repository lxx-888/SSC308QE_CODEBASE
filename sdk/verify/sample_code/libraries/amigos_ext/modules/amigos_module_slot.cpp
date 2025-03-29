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

#include "mi_sys.h"
#include "amigos_module_init.h"
#include "amigos_module_slot.h"

AmigosModuleSlot::AmigosModuleSlot(const std::string &strInSection)
    : AmigosSurfaceSlot(strInSection), AmigosModuleBase(this)
{
}
AmigosModuleSlot::~AmigosModuleSlot()
{
}
unsigned int AmigosModuleSlot::GetModId() const
{
    return stSlotInfo.uintDstBindMod;
}
unsigned int AmigosModuleSlot::GetInputType(unsigned int port) const
{
    return E_MOD_PORT_TYPE_KERNEL;
}
unsigned int AmigosModuleSlot::GetOutputType(unsigned int port) const
{
    return E_MOD_PORT_TYPE_KERNEL;
}
void AmigosModuleSlot::_Init()
{
}
void AmigosModuleSlot::_Deinit()
{
}
void AmigosModuleSlot::_DirectBind(unsigned int inPortId, unsigned int prevOutPortId, const AmigosModuleBase *pPrev)
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

    stSrcChnPort.eModId    = (MI_ModuleId_e)pPrev->GetModId();
    if (stSrcChnPort.eModId == uintExtModId)
    {
        AMIGOS_ERR("Prev mod is not mi module.\n");
        return;
    }
    const AmigosSurfaceBase::ModInfo &stSrcModInfo = pPrev->GetSurface()->GetModInfo();
    stSrcChnPort.u32DevId  = stSrcModInfo.devId;
    stSrcChnPort.u32ChnId  = stSrcModInfo.chnId;
    stSrcChnPort.u32PortId = prevOutPortId;

    const AmigosSurfaceBase::ModInfo &stDstModInfo = this->GetModInfo();
    stDstChnPort.eModId    = (MI_ModuleId_e)(MI_ModuleId_e)stSlotInfo.uintDstBindMod;
    stDstChnPort.u32DevId  = stDstModInfo.devId;
    if (stDstChnPort.eModId == E_MI_MODULE_ID_DISP || stDstChnPort.eModId == E_MI_MODULE_ID_VDISP)
    {
        stDstChnPort.u32ChnId  = inPortId;
        stDstChnPort.u32PortId = 0;
    }
    else
    {
        stDstChnPort.u32ChnId  = stDstModInfo.chnId;
        stDstChnPort.u32PortId = inPortId;
    }

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
void AmigosModuleSlot::_DirectUnBind(unsigned int inPortId, unsigned int prevOutPortId, const AmigosModuleBase *pPrev)
{
    // UnBind
    MI_SYS_ChnPort_t stSrcChnPort;
    MI_SYS_ChnPort_t stDstChnPort;

    memset(&stSrcChnPort, 0, sizeof(MI_SYS_ChnPort_t));
    memset(&stDstChnPort, 0, sizeof(MI_SYS_ChnPort_t));

    stSrcChnPort.eModId = (MI_ModuleId_e)pPrev->GetModId();
    if (stSrcChnPort.eModId == uintExtModId)
    {
        AMIGOS_ERR("Prev mod is not mi module.\n");
        return;
    }

    const AmigosSurfaceBase::ModInfo &stSrcModInfo = pPrev->GetSurface()->GetModInfo();
    stSrcChnPort.u32DevId  = stSrcModInfo.devId;
    stSrcChnPort.u32ChnId  = stSrcModInfo.chnId;
    stSrcChnPort.u32PortId = prevOutPortId;

    const AmigosSurfaceBase::ModInfo &stDstModInfo = this->GetModInfo();
    stDstChnPort.eModId    = (MI_ModuleId_e)(MI_ModuleId_e)stSlotInfo.uintDstBindMod;
    stDstChnPort.u32DevId  = stDstModInfo.devId;
    if (stDstChnPort.eModId == E_MI_MODULE_ID_DISP || stDstChnPort.eModId == E_MI_MODULE_ID_VDISP)
    {
        stDstChnPort.u32ChnId  = inPortId;
        stDstChnPort.u32PortId = 0;
    }
    else
    {
        stDstChnPort.u32ChnId  = stDstModInfo.chnId;
        stDstChnPort.u32PortId = inPortId;
    }
    if (MI_SUCCESS != MI_SYS_UnBindChnPort(0, &stSrcChnPort, &stDstChnPort))
    {
        AMIGOS_ERR("MI_SYS_UnBindChnPort failed\n");
    }
    AMIGOS_INFO("UnBind %d-%d-%d-%d -> %d-%d-%d-%d\n", stSrcChnPort.eModId, stSrcChnPort.u32DevId,
                stSrcChnPort.u32ChnId, stSrcChnPort.u32PortId, stDstChnPort.eModId, stDstChnPort.u32DevId,
                stDstChnPort.u32ChnId, stDstChnPort.u32PortId);
}
AMIGOS_MODULE_INIT("SLOT", AmigosModuleSlot);
