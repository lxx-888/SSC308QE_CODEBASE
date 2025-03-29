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
#ifndef __AMIGOS_MODULE_AEC_H__
#define __AMIGOS_MODULE_AEC_H__

#include <cstdio>
#include <cstring>
#include "amigos_log.h"
#include "mi_ai_datatype.h"
#include "mi_aio_datatype.h"
#include "AudioAecProcess.h"
#include "amigos_surface_aec.h"
#include "amigos_module_base.h"
#include "amigos_module_aio.h"
#include "ss_enum_cast.hpp"

class AmigosModuleAec : public AmigosSurfaceAec, public AmigosModuleBase
{
public:
    explicit AmigosModuleAec(const std::string &strInSection);
    ~AmigosModuleAec() override;
    unsigned int GetModId() const override;
    unsigned int GetInputType(unsigned int port) const override;
    unsigned int GetOutputType(unsigned int port) const override;
    int AecReset(AudioAecInit &aec_init, AudioAecConfig &aec_config);
    inline int SetByPass(bool bypass)
    {
        m_bypass = bypass;
        return 0;
    };

protected:
    void _Init() override;
    void _Deinit() override;
    int  _DataReceiver(unsigned int inPortId, stream_packet_obj &packet) override;
    void _StartOut(unsigned int outPortId) override;
    void _StopOut(unsigned int outPortId) override;

private:
    bool _IsPostReader(unsigned int inPortId) override;
    stream_packet_info _GetStreamInfo(unsigned int outPortId) override;
    stream_packet_obj _DequeueFromInside(unsigned int outPortId, unsigned int ms) override;
    bool _checkAecPacket(stream_packet_obj packet)
    {
        if (nullptr == m_handle || 0 == packet->es_aud_i.packet_info[0].size
            || 0 == packet->es_aud_i.packet_info[1].size || nullptr == packet->es_aud.packet_data[0].data
            || nullptr == packet->es_aud.packet_data[1].data)
        {
            return false;
        }
        return true;
    };
    void           *m_handle;
    char           *m_workingBuffer;
    int             m_iPeriodSize;
    bool            m_bypass;
    ss_linker_base *m_linker;
    pthread_mutex_t stAecLock;
    pthread_mutex_t stPacketLock;
};

#endif
