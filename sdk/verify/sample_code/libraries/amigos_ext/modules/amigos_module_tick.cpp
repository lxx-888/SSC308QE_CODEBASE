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
#include <stdio.h>
#include <pthread.h>
#include <vector>
#include <string>
#include <map>
#include "amigos_module_init.h"
#include "amigos_module_tick.h"
#include "ss_packet.h"

#define ADD_TIMER(__dst_time, __src_time, __diff_time)                     \
        do                                                                 \
        {                                                                  \
            __dst_time.tv_sec  = __src_time.tv_sec + __diff_time.tv_sec;   \
            __dst_time.tv_nsec = __src_time.tv_nsec + __diff_time.tv_nsec; \
            if (__dst_time.tv_nsec > 1000000000)                           \
            {                                                              \
                __dst_time.tv_nsec %= 1000000000;                          \
                __dst_time.tv_sec++;                                       \
            }                                                              \
        } while(0)

SS_ENUM_CAST_STR(AmigosModuleTick::TickEvent::EN_CONTROL_FPS_TYPE,
{
    { AmigosModuleTick::TickEvent::EN_CONTROL_FPS_BLOCK, "block" },
    { AmigosModuleTick::TickEvent::EN_CONTROL_FPS_RATE, "rate" },
    { AmigosModuleTick::TickEvent::EN_CONTROL_FPS_DEST, "dest" },
});

class LinkerTickInput : public ss_linker_base
{
public:
    LinkerTickInput(ss_linker_base *linker, AmigosModuleTick::TickEvent *tick)
        :holdLinker(linker), tickTimer(tick)
    {
    }
    virtual ~LinkerTickInput()
    {
    }
    int enqueue(stream_packet_obj &packet) override
    {
        if (tickTimer->Wait())
        {
            return holdLinker->enqueue(packet);
        }
        return -1;
    }
    stream_packet_obj dequeue(unsigned int ms)
    {
        return holdLinker->dequeue(ms);
    }
private:
    ss_linker_base *holdLinker;
    AmigosModuleTick::TickEvent *tickTimer;
};

AmigosModuleTick::LinkerTickOutput::LinkerTickOutput(InPortInfo *inInfo, TickEvent *tick)
    : inInfo(inInfo), tickTimer(tick)
{
}
AmigosModuleTick::LinkerTickOutput::~LinkerTickOutput()
{
}
int AmigosModuleTick::LinkerTickOutput::enqueue(stream_packet_obj &packet)
{
    return inInfo->positive.enqueue(packet);
}
stream_packet_obj AmigosModuleTick::LinkerTickOutput::dequeue(unsigned int ms)
{
    if (!tickTimer->Wait())
    {
        return nullptr;
    }
    return inInfo->positive.dequeue(ms);
}
AmigosModuleTick::LinkerTickOutputFifo::LinkerTickOutputFifo(InPortInfo *inInfo, TickEvent *tick)
    : inInfo(inInfo), tickTimer(tick)
{
    linkerMutex = PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP;
}
AmigosModuleTick::LinkerTickOutputFifo::~LinkerTickOutputFifo()
{
}
int AmigosModuleTick::LinkerTickOutputFifo::enqueue(stream_packet_obj &packet)
{
    auto newPacket = stream_packet_base::dup(packet);
    assert(newPacket);
    LinkerAsyncNegative *holdLinker = static_cast<LinkerAsyncNegative *>(inInfo->negative);
    if (!holdLinker)
    {
        AMIGOS_ERR("Input negative linker is nullptr!\n");
        return -1;
    }
    auto packetIn = holdLinker->WaitPacket();
    int ret = newPacket->compare_copy(packetIn);
    if (ret == -1)
    {
        AMIGOS_ERR("Packet compare error!\n");
        return -1;
    }
    ss_auto_lock lock(linkerMutex);
    this->packetList.push_back(newPacket);
    return 0;
}
stream_packet_obj AmigosModuleTick::LinkerTickOutputFifo::dequeue(unsigned int ms)
{
    if (!tickTimer->Wait())
    {
        return nullptr;
    }
    if (this->packetList.empty())
    {
        LinkerAsyncNegative *holdLinker = static_cast<LinkerAsyncNegative *>(inInfo->negative);
        return holdLinker ? holdLinker->WaitPacket(ms) : nullptr;
    }
    ss_auto_lock lock(linkerMutex);
    auto packetOut = this->packetList.front();
    packetList.pop_front();
    return packetOut;
}

AmigosModuleTick::TickEvent::TickEvent()
    : enCtlFpsType(EN_CONTROL_FPS_BLOCK)
    , uiSrcFps(0)
    , uiDstFps(0)
    , uiPacketNum(0)
{
    memset(&stDiffTime, 0, sizeof(struct timespec));
    memset(&stStartTime, 0, sizeof(struct timespec));
    eventMutex = PTHREAD_MUTEX_INITIALIZER;
    pthread_condattr_init(&eventCondAttr);
    pthread_condattr_setclock(&eventCondAttr, CLOCK_MONOTONIC);
    pthread_cond_init(&eventCond, &eventCondAttr);
}
AmigosModuleTick::TickEvent::~TickEvent()
{
    pthread_cond_destroy(&eventCond);
    pthread_condattr_destroy(&eventCondAttr);
}
void AmigosModuleTick::TickEvent::Notify()
{
    pthread_mutex_lock(&eventMutex);
    pthread_cond_signal(&eventCond);
    pthread_mutex_unlock(&eventMutex);
}
void AmigosModuleTick::TickEvent::SetTimer(long sec, long nsec)
{
    pthread_mutex_lock(&eventMutex);
    stDiffTime.tv_sec = sec;
    stDiffTime.tv_nsec = nsec;
    clock_gettime(CLOCK_MONOTONIC, &stStartTime);
    pthread_cond_signal(&eventCond);
    pthread_mutex_unlock(&eventMutex);
}
bool AmigosModuleTick::TickEvent::CalcPacketNum()
{
    uiPacketNum += uiDstFps;
    if (uiPacketNum < uiSrcFps)
    {
        return false;
    }
    uiPacketNum -= uiSrcFps;
    return true;
}
bool AmigosModuleTick::TickEvent::CalcPacketTime()
{
    struct timespec stCurrTime;
    struct timespec stSetTime;
    if (!stDiffTime.tv_sec && !stDiffTime.tv_nsec)
    {
        return true;
    }
    clock_gettime(CLOCK_MONOTONIC, &stCurrTime);
    if (!stStartTime.tv_sec && !stStartTime.tv_nsec)
    {
        stStartTime = stCurrTime;
    }
    ADD_TIMER(stSetTime, stStartTime, stDiffTime);
    if (stCurrTime.tv_sec > stSetTime.tv_sec || (stCurrTime.tv_sec == stSetTime.tv_sec && stCurrTime.tv_nsec >= stSetTime.tv_nsec))
    {
        stStartTime = stSetTime;
        return true;
    }
    return false;
}
bool AmigosModuleTick::TickEvent::Wait()
{
    if (EN_CONTROL_FPS_RATE == enCtlFpsType)
    {
        return CalcPacketNum();
    }
    if (EN_CONTROL_FPS_DEST == enCtlFpsType)
    {
        return CalcPacketTime();
    }
    struct timespec stCurrTime;
    struct timespec stSetTime;
    pthread_mutex_lock(&eventMutex);
    if (stDiffTime.tv_sec == -1)
    {
        pthread_mutex_unlock(&eventMutex);
        usleep(10 * 1000);
        return true;
    }
    if  (!stDiffTime.tv_sec && !stDiffTime.tv_nsec)
    {
        pthread_mutex_unlock(&eventMutex);
        return true;
    }
    clock_gettime(CLOCK_MONOTONIC, &stCurrTime);
    if (!stStartTime.tv_sec && !stStartTime.tv_nsec)
    {
        stStartTime = stCurrTime;
    }
    ADD_TIMER(stSetTime, stStartTime, stDiffTime);
    if (stCurrTime.tv_sec > stSetTime.tv_sec
        || ((stCurrTime.tv_sec == stSetTime.tv_sec) && (stCurrTime.tv_nsec >= stSetTime.tv_nsec)))
    {
        AMIGOS_WRN("Take timer exceeded! current time sec: %ld nsec:%ld, next sec: %ld nsec: %ld, step sec: %ld nsec: %ld\n",
                   stCurrTime.tv_sec, stCurrTime.tv_nsec, stSetTime.tv_sec, stSetTime.tv_nsec, stDiffTime.tv_sec, stDiffTime.tv_nsec);
        stStartTime = stCurrTime;
        ADD_TIMER(stSetTime, stStartTime, stDiffTime);
    }
    int ret = pthread_cond_timedwait(&eventCond, &eventMutex, &stSetTime);
    if (ETIMEDOUT == ret)
    {
        stStartTime = stSetTime;
    }
    pthread_mutex_unlock(&eventMutex);
    return true;
}

AmigosSurfaceTick::~AmigosSurfaceTick() {}


AmigosModuleTick::AmigosModuleTick(const std::string &strInSection)
    : AmigosSurfaceTick(strInSection), AmigosModuleBase(this)
{
}
AmigosModuleTick::~AmigosModuleTick()
{
}
unsigned int AmigosModuleTick::GetModId() const
{
    return uintExtModId;
}
unsigned int AmigosModuleTick::GetInputType(unsigned int port) const
{
    return E_MOD_PORT_TYPE_USER;
}
unsigned int AmigosModuleTick::GetOutputType(unsigned int port) const
{
    return E_MOD_PORT_TYPE_USER;
}
void AmigosModuleTick::SetBlockTime(long sec, long nsec)
{
    this->tick.SetFrcType(TickEvent::EN_CONTROL_FPS_BLOCK);
    this->tick.SetTimer(sec, nsec);
    AMIGOS_INFO("tick set block type sec: %ld, nsec: %ld\n", sec, nsec);
}
void AmigosModuleTick::SetRateFps(unsigned int srcFps, unsigned int inFps)
{
    this->tick.SetFrcType(TickEvent::EN_CONTROL_FPS_RATE);
    this->tick.SetSrcFps(srcFps);
    this->tick.SetDstFps(inFps);
    AMIGOS_INFO("tick set rate type src fps: %d, dst fps: %d.\n", srcFps, inFps);
}
void AmigosModuleTick::SetDestFps(unsigned int dstFps)
{
    if (0 == dstFps)
    {
        AMIGOS_WRN("tick set dest type fps: %d failed, not support this fps.\n", dstFps);
        return;
    }
    this->tick.SetFrcType(TickEvent::EN_CONTROL_FPS_DEST);
    this->tick.SetTimer(0, 1000000000 / dstFps);
    AMIGOS_INFO("tick set dest type fps: %d.\n", dstFps);
}
void AmigosModuleTick::_Init()
{
    auto itIn = this->mapPortIn.begin();
    if (itIn == this->mapPortIn.end())
    {
        AMILOG_ERR << "Not set in port " << COLOR_ENDL;
        return;
    }
    auto itOut = this->mapPortOut.begin();
    if (itOut == this->mapPortOut.end())
    {
        AMILOG_ERR << "Not found out port 0" << COLOR_ENDL;
        return;
    }
    ModPortOutInfo stOutPortInfo, stPrevOut;
    ModPortInInfo stInPortInfo;
    this->GetSurface()->GetPortOutInfo(itOut->first, stOutPortInfo);
    this->GetSurface()->GetPortInInfo(itIn->first, stInPortInfo);
    itIn->second.pPrev->GetSurface()->GetPortOutInfo(itIn->second.prevOutPortId, stPrevOut);
    auto type = ss_enum_cast<TickEvent::EN_CONTROL_FPS_TYPE>::from_str(stTickInfo.strFrcType);
    if (TickEvent::EN_CONTROL_FPS_RATE == type)
    {
        SetRateFps(stPrevOut.curFrmRate, stInPortInfo.curFrmRate);
        return;
    }
    if (TickEvent::EN_CONTROL_FPS_DEST == type)
    {
        SetDestFps(stOutPortInfo.curFrmRate);
        return;
    }
    SetBlockTime(stTickInfo.longTimerSec, stTickInfo.longTimerNSec);
}
void AmigosModuleTick::_Deinit()
{
}
int AmigosModuleTick::_InConnect(unsigned int inPortId)
{
    auto itIn = this->mapPortIn.find(inPortId);
    if (itIn == this->mapPortIn.end())
    {
        AMILOG_ERR << "Not found in port " << inPortId << COLOR_ENDL;
        return -1;
    }
    auto itOut = this->mapPortOut.find(0);
    if (itOut == this->mapPortOut.end())
    {
        AMILOG_ERR << "Not found out port 0" << COLOR_ENDL;
        return -1;
    }
    bool receivable = (itOut->second.positive.empty());
    bool sync       = (stTickInfo.strMode == "push");
    if (receivable && sync)
    {
        AMIGOS_ERR("======== Tick input port %d is sync mode and it's next is post reader.\n", inPortId);
        AMIGOS_ERR("======== Stream pipeline will slow\n", inPortId);
        AMIGOS_ERR("======== Modify %s -> IN_%d -> MODE to fifo/pull\n", this->GetModIdStr().c_str(), inPortId);
        assert(0);
    }
    if (!receivable && !sync)
    {
        AMIGOS_ERR("======== Tick input port %d is not sync mode and it's next is not post reader.\n", inPortId);
        AMIGOS_ERR("======== Stream pipeline will pending\n", inPortId);
        AMIGOS_ERR("======== Modify %s -> IN_%d -> MODE to push\n", this->GetModIdStr().c_str(), inPortId);
        assert(0);
    }
    return 0;
}
stream_packet_info AmigosModuleTick::_GetStreamInfo(unsigned int outPortId)
{
    auto it = mapPortIn.find(0);
    if (mapPortIn.end() == it)
    {
        AMIGOS_ERR("Tick chn has no this input(0) port!!!\n");
        return stream_packet_info();
    }
    return it->second.get_packet_info();
}
int AmigosModuleTick::_ConnectedTransfer(unsigned int outPortId)
{
    auto iter = this->mapPortIn.find(0);
    if (iter == this->mapPortIn.end())
    {
        AMILOG_ERR << "Not found in port 0" << COLOR_ENDL;
        return -1;
    }
    iter->second.access();
    return 0;
}
int AmigosModuleTick::_DisconnectedTransfer(unsigned int outPortId)
{
    auto iter = this->mapPortIn.find(0);
    if (iter == this->mapPortIn.end())
    {
        AMILOG_ERR << "Not found in port 0" << COLOR_ENDL;
        return -1;
    }
    iter->second.leave();
    return 0;
}
ss_linker_base *AmigosModuleTick::_CreateInputNegativeLinker(unsigned int inPortId)
{
    auto iter = this->mapPortOut.find(0);
    if (iter == this->mapPortOut.end())
    {
        AMILOG_ERR << "Not found out port 0" << COLOR_ENDL;
        return NULL;
    }
    if (stTickInfo.strMode == "pull")
    {
        return new LinkerBypass(&iter->second.positive);
    }
    if (stTickInfo.strMode == "push")
    {
        return new LinkerTickInput(&iter->second.positive, &tick);
    }
    if (stTickInfo.strMode == "fifo")
    {
        return new LinkerAsyncNegative(false, 8);
    }
    AMILOG_ERR << "Err in tick's mode:" << stTickInfo.strMode << COLOR_ENDL;
    return nullptr;
}

ss_linker_base *AmigosModuleTick::_CreateOutputNegativeLinker(unsigned int outPortId)
{
    auto iter = this->mapPortIn.find(0);
    if (iter == this->mapPortIn.end())
    {
        AMILOG_ERR << "Not found in port 0" << COLOR_ENDL;
        return nullptr;
    }
    if (stTickInfo.strMode == "pull")
    {
        if (iter->second.positive.empty())
        {
            AMIGOS_INFO("Output negative should not be null!\n");
            return nullptr;
        }
        return new LinkerTickOutput(&iter->second, &tick);
    }
    if (stTickInfo.strMode == "fifo")
    {
        return new LinkerTickOutputFifo(&iter->second, &tick);
    }
    return nullptr;
}
stream_packer *AmigosModuleTick::_CreateInputStreamPacker(unsigned int inPortId, bool &bFast)
{
    auto iter = this->mapPortOut.find(0);
    if (iter == this->mapPortOut.end())
    {
        AMILOG_ERR << "Not found out port 0" << COLOR_ENDL;
        return NULL;
    }
    return new StreamPackerBypass(&iter->second.outPacker);
}
bool AmigosModuleTick::_IsPostReader(unsigned int inPortId)
{
    if (stTickInfo.strMode == "push" || stTickInfo.strMode == "fifo")
    {
        return false;
    }
    if (stTickInfo.strMode == "pull")
    {
        return true;
    }
    return false;
}
AMIGOS_MODULE_INIT("TICK", AmigosModuleTick);
