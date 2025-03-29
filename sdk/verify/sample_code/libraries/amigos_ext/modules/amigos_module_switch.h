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

#ifndef __AMIGOS_MODULE_SWITCH_H__
#define __AMIGOS_MODULE_SWITCH_H__

#include <pthread.h>
#include <map>
#include "amigos_module_base.h"
#include "amigos_surface_switch.h"
#include "ss_auto_lock.h"
#include "ss_linker.h"
#include "ss_packet.h"

class AmigosModuleSwitch : public AmigosSurfaceSwitch, public AmigosModuleBase
{
public:
    class SwitchInLinker : public ss_linker_base
    {
    public:
        SwitchInLinker();
        ~SwitchInLinker() override;
        int enqueue(stream_packet_obj &obj) override;
        stream_packet_obj dequeue(unsigned int time_out_ms) override;
        void setTarget(ss_linker_base *targetLinker);
    private:
        pthread_rwlock_t rwlock;
        ss_linker_base *targetLinker;
    };
public:
    explicit AmigosModuleSwitch(const std::string &strInSection);
    ~AmigosModuleSwitch() override;

    unsigned int GetModId() const override;
    unsigned int GetInputType(unsigned int port) const override;
    unsigned int GetOutputType(unsigned int port) const override;

    bool DoSwitch(unsigned int inPortId, int dstPort);

private:
    void _Init() override;
    void _Deinit() override;
    void _StartIn(unsigned int inPortId) override;
    void _StopIn(unsigned int inPortId) override;
    int _ConnectedTransfer(unsigned int outPortId) override;
    int _DisconnectedTransfer(unsigned int outPortId) override;
    ss_linker_base *_CreateInputNegativeLinker(unsigned int inPortId) override;
    void _DestroyInputNegativeLinker(unsigned int inPortId) override;
    bool _IsDelayConnected(unsigned int) override
    {
        return true;
    }
    stream_packet_info _GetStreamInfo(unsigned int outPortId) override;

private:
    std::map<unsigned int, SwitchInLinker> mapSwitchInLinker;
    std::map<unsigned int, int>            mapSwitchIn;
    std::map<unsigned int, int>            mapSwitchOut;
};

#endif /* __AMIGOS_MODULE_SWITCH_H__ */
