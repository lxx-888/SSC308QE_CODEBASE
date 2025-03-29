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
#include <sys/select.h>
#include <cstring>
#include <unistd.h>

#include "amigos_module_init.h"
#include "amigos_module_sync.h"
#include "ss_packet.h"
#include "ss_thread.h"

AmigosModuleSync::AmigosModuleSync(const std::string &strSection)
    : AmigosSurfaceSync(strSection), AmigosModuleBase(this), threadHandle(nullptr)
{
}

AmigosModuleSync::~AmigosModuleSync()
{
}

void AmigosModuleSync::_Init()
{
}

void AmigosModuleSync::_Deinit()
{
}

unsigned int AmigosModuleSync::GetModId() const
{
    return uintExtModId;
}
unsigned int AmigosModuleSync::GetInputType(unsigned int port) const
{
    return E_MOD_PORT_TYPE_USER;
}
unsigned int AmigosModuleSync::GetOutputType(unsigned int port) const
{
    return E_MOD_PORT_TYPE_USER;
}

void AmigosModuleSync::_Start()
{
    ss_thread_attr ss_attr;
    memset(&ss_attr, 0, sizeof(ss_thread_attr));
    ss_attr.do_monitor         = SyncReader;
    ss_attr.do_signal          = NULL;
    ss_attr.monitor_cycle_sec  = 0;
    ss_attr.monitor_cycle_nsec = 0;
    ss_attr.is_reset_timer     = 0;
    ss_attr.in_buf.buf         = this;
    ss_attr.in_buf.size        = 0;
    snprintf(ss_attr.thread_name, 128, "%s", this->GetModIdStr().c_str());
    threadHandle = ss_thread_open(&ss_attr);
    if (!threadHandle)
    {
        AMIGOS_ERR("Monitor return error!\n");
       return;
    }
    ss_thread_start_monitor(threadHandle);
    AMIGOS_INFO("Sync reader %s start\n", this->GetModIdStr().c_str());
}
void AmigosModuleSync::_Stop()
{
    ss_thread_close(threadHandle);
    threadHandle = nullptr;
    AMIGOS_INFO("Sync reader %s exit\n", this->GetModIdStr().c_str());
}

stream_packer *AmigosModuleSync::_CreateInputStreamPacker(unsigned int inPortId, bool &bFast)
{
    bFast = false;
    auto it = mapSyncInInfo.find(inPortId);
    if (it == mapSyncInInfo.end())
    {
        AMIGOS_ERR("Not found port %d in surface.\n", inPortId);
        return NULL;
    }
    auto iterPortOut = this->mapPortOut.find(it->second);
    if (iterPortOut == this->mapPortOut.end())
    {
        AMIGOS_ERR("Port %d has not base sturcture.\n", it->second);
        return NULL;
    }
    return new StreamPackerBypass(&iterPortOut->second.outPacker);
}

stream_packet_info AmigosModuleSync::_GetStreamInfo(unsigned int outPortId)
{
    for (auto &it : mapSyncInInfo)
    {
        if (it.second == outPortId)
        {
            auto iter = mapPortIn.find(it.first);
            if (iter == mapPortIn.end())
            {
                AMIGOS_ERR("Port %d has not base sturcture.\n", it.first);
            }
            return iter->second.get_packet_info();
        }
    }
    return stream_packet_info();
}

bool AmigosModuleSync::_NeedResetStreamTraverse(unsigned int outPortId, unsigned int bindType,
                                                unsigned int &inPortId)
{
    for (auto &it : mapSyncInInfo)
    {
        if (it.second == outPortId)
        {
            auto iter = mapPortIn.find(it.first);
            if (iter == mapPortIn.end())
            {
                AMIGOS_ERR("Port %d has not base sturcture.\n", it.first);
                return false;
            }
            inPortId = it.first;
            return true;
        }
    }
    return false;
}

int AmigosModuleSync::_ConnectedTransfer(unsigned int outPortId)
{
    for (auto &it : mapSyncInInfo)
    {
        if (it.second == outPortId)
        {
            auto iter = mapPortIn.find(it.first);
            if (iter == mapPortIn.end())
            {
                AMIGOS_ERR("Port %d has not base sturcture.\n", it.first);
                return -1;
            }
            iter->second.access();
            break;
        }
    }
    return 0;
}

int AmigosModuleSync::_DisconnectedTransfer(unsigned int outPortId)
{
    for (auto &it : mapSyncInInfo)
    {
        if (it.second == outPortId)
        {
            auto iter = mapPortIn.find(it.first);
            if (iter == mapPortIn.end())
            {
                AMIGOS_ERR("Port %d has not base sturcture.\n", it.first);
                return -1;
            }
            iter->second.leave();
            break;
        }
    }
    return 0;
}
bool AmigosModuleSync::_IsPostReader(unsigned int inPortId)
{
    return true;
}
bool AmigosModuleSync::_IsDelayConnected(unsigned int uintInPort)
{
    // The post module of SYNC is not set as 'post reader', so it depended the '_Connected' function to do ss_message's
    // access.
    return true;
}

void *AmigosModuleSync::SyncReader(struct ss_thread_buffer *thread_buf)
{
    AmigosModuleSync* pReader = (AmigosModuleSync*)thread_buf->buf;
    bool bTransfer = false;

    assert(pReader);
    for (auto &it : pReader->mapSyncInInfo)
    {
        auto itIn = pReader->mapPortIn.find(it.first);
        assert(pReader->mapPortIn.end() != itIn);
        auto itOut = pReader->mapPortOut.find(it.second);
        assert(pReader->mapPortOut.end() != itOut);

        if (!itOut->second.connection_check())
        {
            continue;
        }
        auto packet = itIn->second.positive.dequeue(1000);
        if (packet == nullptr)
        {
            //AMIGOS_ERR("syncing failed! can`t get data from inport %d\n", it.first);
            continue;
        }
        if (!bTransfer)
        {
            bTransfer = true;
        }
        itOut->second.positive.enqueue(packet);
    }
    if (!bTransfer)
    {
        usleep(20 * 1000);
    }
    return nullptr;
}
AMIGOS_MODULE_INIT("SYNC", AmigosModuleSync);

