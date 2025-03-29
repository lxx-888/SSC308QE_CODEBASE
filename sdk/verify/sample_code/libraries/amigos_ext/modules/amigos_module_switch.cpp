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

#include <pthread.h>
#include "amigos_module_init.h"
#include "amigos_module_switch.h"
#include "amigos_module_base.h"
#include "amigos_surface_switch.h"
#include "amigos_log.h"
#include "ss_auto_lock.h"
#include "ss_packet.h"

AmigosModuleSwitch::AmigosModuleSwitch(const std::string &strInSection)
    : AmigosSurfaceSwitch(strInSection), AmigosModuleBase(this)
{
}
AmigosModuleSwitch::~AmigosModuleSwitch()
{
}

AmigosModuleSwitch::SwitchInLinker::SwitchInLinker() : targetLinker(nullptr)
{
    this->rwlock = PTHREAD_RWLOCK_INITIALIZER;
}

AmigosModuleSwitch::SwitchInLinker::~SwitchInLinker()
{

}

int AmigosModuleSwitch::SwitchInLinker::enqueue(stream_packet_obj &obj)
{
    ss_auto_rdlock(this->rwlock);
    if (!this->targetLinker)
    {
        return 0;
    }
    return this->targetLinker->enqueue(obj);
}
stream_packet_obj AmigosModuleSwitch::SwitchInLinker::dequeue(unsigned int time_out_ms)
{
    ss_auto_rdlock(this->rwlock);
    if (!this->targetLinker)
    {
        return 0;
    }
    return this->targetLinker->dequeue(time_out_ms);
}
void AmigosModuleSwitch::SwitchInLinker::setTarget(ss_linker_base *targetLinker)
{
    ss_auto_rwlock(this->rwlock);
    this->targetLinker = targetLinker;
}

unsigned int AmigosModuleSwitch::GetModId() const
{
    return uintExtModId;
}
unsigned int AmigosModuleSwitch::GetInputType(unsigned int) const
{
    return E_MOD_PORT_TYPE_USER;
}
unsigned int AmigosModuleSwitch::GetOutputType(unsigned int) const
{
    return E_MOD_PORT_TYPE_USER;
}
void AmigosModuleSwitch::_Init() {}
void AmigosModuleSwitch::_Deinit() {}

void AmigosModuleSwitch::_StartIn(unsigned int inPortId)
{
    auto itSwitchInInfo = this->mapSwitchInInfo.find(inPortId);
    if (itSwitchInInfo == this->mapSwitchInInfo.end())
    {
        AMIGOS_ERR("Cannot find input port %d InInfo\n", inPortId);
        return;
    }
    this->mapSwitchInLinker[inPortId] = SwitchInLinker();
    this->mapSwitchIn[inPortId]       = -1;
    this->DoSwitch(inPortId, itSwitchInInfo->second);
}
void AmigosModuleSwitch::_StopIn(unsigned int inPortId)
{
    auto itInLinker = this->mapSwitchInLinker.find(inPortId);
    if (itInLinker == this->mapSwitchInLinker.end())
    {
        return;
    }
    this->DoSwitch(inPortId, -1);
    this->mapSwitchInLinker.erase(itInLinker);
}
ss_linker_base *AmigosModuleSwitch::_CreateInputNegativeLinker(unsigned int inPortId)
{
    return &this->mapSwitchInLinker[inPortId];
}
void AmigosModuleSwitch::_DestroyInputNegativeLinker(unsigned int inPortId)
{
    auto iter = this->mapPortIn.find(inPortId);
    if (iter == this->mapPortIn.end())
    {
        AMIGOS_ERR("Cannot find input port %d InInfo\n", inPortId);
        return;
    }
    iter->second.negative = nullptr;
}

int AmigosModuleSwitch::_ConnectedTransfer(unsigned int outPortId)
{
    auto itOutDesc = this->mapSwitchOut.find(outPortId);
    if (itOutDesc == this->mapSwitchOut.end())
    {
        return 0;
    }
    if (itOutDesc->second < 0)
    {
        return 0;
    }
    auto itIn = this->mapPortIn.find(itOutDesc->second);
    if (itIn == this->mapPortIn.end())
    {
        return 0;
    }
    auto itSwitchIn = this->mapSwitchIn.find(itOutDesc->second);
    if (itSwitchIn == this->mapSwitchIn.end())
    {
        return 0;
    }
    if (itSwitchIn->second != static_cast<int>(outPortId))
    {
        return 0;
    }
    itIn->second.access();
    return 0;
}
int AmigosModuleSwitch::_DisconnectedTransfer(unsigned int outPortId)
{
    auto itOutDesc = this->mapSwitchOut.find(outPortId);
    if (itOutDesc == this->mapSwitchOut.end())
    {
        return 0;
    }
    if (itOutDesc->second < 0)
    {
        return 0;
    }
    auto itIn = this->mapPortIn.find(itOutDesc->second);
    if (itIn == this->mapPortIn.end())
    {
        return 0;
    }
    auto itSwitchIn = this->mapSwitchIn.find(itOutDesc->second);
    if (itSwitchIn == this->mapSwitchIn.end())
    {
        return 0;
    }
    if (itSwitchIn->second != static_cast<int>(outPortId))
    {
        return 0;
    }
    itIn->second.leave();
    return 0;
}

bool AmigosModuleSwitch::DoSwitch(unsigned int inPortId, int dstPort)
{
    auto itIn = this->mapPortIn.find(inPortId);
    if (itIn == this->mapPortIn.end())
    {
        AMIGOS_ERR("Input port %d is not exist\n", inPortId);
        return false;
    }

    auto itSwitchIn = this->mapSwitchIn.find(inPortId);
    if (itSwitchIn == this->mapSwitchIn.end())
    {
        AMIGOS_ERR("Input port %d is not exist\n", inPortId);
        return false;
    }

    auto itInLinker = this->mapSwitchInLinker.find(inPortId);
    if (itInLinker == this->mapSwitchInLinker.end())
    {
        AMIGOS_ERR("Input port %d is not exist\n", inPortId);
        return false;
    }

    if (itSwitchIn->second == dstPort)
    {
        AMIGOS_INFO("Same dstPort %d\n", dstPort);
        return true;
    }

    if (itSwitchIn->second >= 0
        && this->mapPortOut.find(itSwitchIn->second) != this->mapPortOut.end())
    {
        itInLinker->second.setTarget(NULL);
        itIn->second.leave();
        this->mapSwitchOut.erase(itSwitchIn->second);
    }

    if (dstPort >= 0)
    {
        auto itOut = this->mapPortOut.find(dstPort);
        if (itOut != this->mapPortOut.end())
        {
            auto itOutDesc = this->mapSwitchOut.find(dstPort);
            if (itOutDesc != this->mapSwitchOut.end())
            {
                this->DoSwitch(itOutDesc->second, -1);
            }
            this->mapSwitchOut[dstPort] = inPortId;
            itInLinker->second.setTarget(&itOut->second.positive);
            if (itOut->second.connection_check())
            {
                itIn->second.access();
            }
        }
    }

    AMIGOS_INFO("%s:IN_%d [%d] -> [%d]\n", this->stModInfo.sectionName.c_str(), inPortId, itSwitchIn->second, dstPort);
    itSwitchIn->second = dstPort;
    return true;
}
stream_packet_info AmigosModuleSwitch::_GetStreamInfo(unsigned int outPortId)
{
    auto itSwitchOut = this->mapSwitchOut.find(outPortId);
    if (itSwitchOut != this->mapSwitchOut.end())
    {
        auto itIn = this->mapPortIn.find(itSwitchOut->second);
        if (itIn != this->mapPortIn.end())
        {
            return itIn->second.get_packet_info();
        }
    }
    return stream_packet_info();
}
AMIGOS_MODULE_INIT("SWITCH", AmigosModuleSwitch);

