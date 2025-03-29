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

#include "amigos_module_init.h"
#include "amigos_module_pares.h"
#include "amigos_surface_pares.h"
#include "mi_sys.h"
#include "ss_packet.h"

AmigosModulePares::AmigosModulePares(const std::string &strInSection)
    : AmigosSurfacePares(strInSection), AmigosModuleMiBase(this)
{
}
AmigosModulePares::~AmigosModulePares() {}
unsigned int AmigosModulePares::GetModId() const
{
    return this->uintExtModId;
}
unsigned int AmigosModulePares::GetInputType(unsigned int port) const
{
    return E_MOD_PORT_TYPE_USER;
}
unsigned int AmigosModulePares::GetOutputType(unsigned int port) const
{
    return E_MOD_PORT_TYPE_USER;
}
void AmigosModulePares::_Init() {}
void AmigosModulePares::_Deinit() {}
void AmigosModulePares::_StartIn(unsigned int inPortId)
{
    auto itIn = this->mapPortIn.find(inPortId);
    if (itIn == this->mapPortIn.end())
    {
        AMIGOS_ERR("Input port %d is no exist\n", inPortId);
        return;
    }
    auto itInPares = this->mapParesIn.find(inPortId);
    if (itInPares == this->mapParesIn.end())
    {
        AMIGOS_ERR("Can't find input %d\n", inPortId);
        return;
    }
    unsigned int outPortId = inPortId;
    auto itOut = this->mapPortOut.find(outPortId);
    if (itOut == this->mapPortOut.end())
    {
        AMIGOS_ERR("Output port %d is not exist\n", outPortId);
        return;
    }
    bool receivable = itOut->second.positive.empty();
    bool sync       = (itInPares->second.strMode == "push");
    if (receivable && sync)
    {
        AMIGOS_ERR("======== Pares input port %d is sync mode and it's next is post reader.\n", inPortId);
        AMIGOS_ERR("======== Stream pipeline will slow\n", inPortId);
        AMIGOS_ERR("======== Modify %s -> IN_%d -> MODE to fifo/pull\n", this->GetModIdStr().c_str(), inPortId);
        assert(0);
    }
    if (!receivable && !sync)
    {
        AMIGOS_ERR("======== Pares input port %d is not sync mode and it's next is not post reader.\n", inPortId);
        AMIGOS_ERR("======== Stream pipeline will pending\n", inPortId);
        AMIGOS_ERR("======== Modify %s -> IN_%d -> MODE to push\n", this->GetModIdStr().c_str(), inPortId);
        assert(0);
    }
}
stream_packet_info AmigosModulePares::_GetStreamInfo(unsigned int outPortId)
{
    unsigned int inPortId = outPortId;
    auto it = mapPortIn.find(inPortId);
    if (mapPortIn.end() == it)
    {
        AMIGOS_ERR("Pares chn has no this input(%d) port!!!\n", inPortId);
        return stream_packet_info();
    }
    return it->second.get_packet_info();
}
void AmigosModulePares::_StopIn(unsigned int inPortId)
{
    AMIGOS_INFO("PARES StopIn %d\n", inPortId);
}
void AmigosModulePares::_StartOut(unsigned int outPortId)
{
    AMIGOS_INFO("PARES StartOut %d\n", outPortId);
}
void AmigosModulePares::_StopOut(unsigned int outPortId)
{
    AMIGOS_INFO("PARES StopOut %d\n", outPortId);
}
int AmigosModulePares::_Connected(unsigned int outPortId, unsigned int ref)
{
    const AmigosSurfaceBase::ModInfo &modInfo = GetSurface()->GetModInfo();
    AMIGOS_INFO("Nothing to do with _Connected for %s\n", modInfo.modName.c_str());
    return 0;
}
int AmigosModulePares::_Disconnected(unsigned int outPortId, unsigned int ref)
{
    const AmigosSurfaceBase::ModInfo &modInfo = GetSurface()->GetModInfo();
    AMIGOS_INFO("Nothing to do with _Disconnected for %s\n", modInfo.modName.c_str());
    return 0;
}
bool AmigosModulePares::_IsPostReader(unsigned int inPortId)
{
    auto it = this->mapParesIn.find(inPortId);
    if (it == this->mapParesIn.end())
    {
        return false;
    }
    if (it->second.strMode == "push" || it->second.strMode == "fifo")
    {
        return false;
    }
    if (it->second.strMode == "pull")
    {
        return true;
    }
    return false;
}

int AmigosModulePares::_DataReceiver(unsigned int inPortId, stream_packet_obj &packet)
{
    unsigned int outPortId = inPortId;
    auto itOut = this->mapPortOut.find(outPortId);
    if (itOut == this->mapPortOut.end())
    {
        AMIGOS_ERR("Output port %d is not exist\n", outPortId);
        return 0;
    }
    return itOut->second.positive.enqueue(packet);
}

int AmigosModulePares::_EnqueueOut(unsigned int outPortId, stream_packet_obj &packet)
{
    unsigned int inPortId = outPortId;
    auto itin = this->mapPortIn.find(inPortId);
    if (itin == this->mapPortIn.end())
    {
        AMIGOS_ERR("Input port %d is no exist\n", inPortId);
        return -1;
    }
    if (itin->second.positive.empty())
    {
        AMIGOS_ERR("Input port %d hos not positive linker\n", inPortId);
        return -1;
    }
    if (packet->get_type() == "sys_mma")
    {
        return itin->second.positive.enqueue(packet);
    }
    AMIGOS_ERR("Not sys_mma buffer!\n");
    return -1;
}
int AmigosModulePares::_DequeueOut(unsigned int outPortId, stream_packet_obj &packet)
{
    unsigned int inPortId = outPortId;
    auto itin = this->mapPortIn.find(inPortId);
    if (itin == this->mapPortIn.end())
    {
        AMIGOS_ERR("Input port %d is no exist\n", inPortId);
        return -1;
    }
    if (itin->second.positive.empty())
    {
        AMIGOS_ERR("Input port %d has not positive linker\n", inPortId);
        return -1;
    }
    if (packet->get_type() == "sys_mma")
    {
        itin->second.positive.dequeue(100);
        return 0;
    }
    AMIGOS_ERR("Not sys_mma buffer!\n");
    return -1;
}

stream_packet_obj AmigosModulePares::_DequeueFromInside(unsigned int outPortId, unsigned int ms)
{
    unsigned int inPortId = outPortId;
    auto itin = this->mapPortIn.find(inPortId);
    if (itin == this->mapPortIn.end())
    {
        AMIGOS_ERR("Input port %d is no exist\n", inPortId);
        return nullptr;
    }
    if (itin->second.negative)
    {
        // fifo,push do this
        LinkerAsyncNegative *fifoLinker = static_cast<LinkerAsyncNegative *>(itin->second.negative);
        return fifoLinker->WaitPacket(ms);
    }
    return itin->second.positive.dequeue(ms);
    AMIGOS_ERR("Input port %d has not connector\n", inPortId);
    return nullptr;
}
ss_linker_base *AmigosModulePares::_CreateInputNegativeLinker(unsigned int inPortId)
{
    auto it = this->mapParesIn.find(inPortId);
    if (it == this->mapParesIn.end())
    {
        AMIGOS_ERR("Can't find input %d\n", inPortId);
        return nullptr;
    }
    if (it->second.strMode == "push")
    {
        return new LinkerSyncNegative(inPortId, this);
    }
    if (it->second.strMode == "fifo")
    {
        AMIGOS_INFO("set parser: %s, depth: %d, warning %d\n", this->GetModIdStr().c_str(), it->second.uiPacketDepth, (bool)it->second.uiDropPacketMsg);
        return new LinkerAsyncNegative((bool)it->second.uiDropPacketMsg, it->second.uiPacketDepth);
    }
    AMIGOS_ERR("Mode error %s\n", it->second.strMode);
    return nullptr;
}
stream_packer *AmigosModulePares::_CreateInputStreamPacker(unsigned int inPortId, bool &bFast)
{
    return new StreamPackerSysMma();
}
AMIGOS_MODULE_INIT("PARES", AmigosModulePares);
