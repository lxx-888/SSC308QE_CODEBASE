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

#ifndef __AMIGOS_MODULE_AUDIOALGO_H__
#define __AMIGOS_MODULE_AUDIOALGO_H__

#include "audio_algo.hpp"
#include "amigos_surface_audioalgo.h"
#include "amigos_module_base.h"
#include "ss_packet.h"

class AmigosModuleAudioAlgo : public AmigosSurfaceAudioAlgo, public AmigosModuleBase
{
    friend void *AlgoReceiverMonitor(struct ss_thread_buffer *thread_buf);

public:
    explicit AmigosModuleAudioAlgo(const std::string &strInSection);
    ~AmigosModuleAudioAlgo() override;
    unsigned int           GetModId() const override;
    unsigned int           GetInputType(unsigned int port) const override;
    unsigned int           GetOutputType(unsigned int port) const override;
    inline ss_linker_base *GetLinker(std::string str)
    {
        return m_maplinker[str];
    };

protected:
    void _Init() override;
    void _Deinit() override;
    int  _DataReceiver(unsigned int inPortId, stream_packet_obj &packet) override;
    void _StartIn(unsigned int inPortId) override;
    void _StopIn(unsigned int inPortId) override;
    int  _Connected(unsigned int outPortId, unsigned int ref) override;
    int  _Disconnected(unsigned int outPortId, unsigned int ref) override;

private:
    void               CreateReceiver(unsigned int inPortId);
    void               DestoryReceiver(unsigned int inPortId);
    stream_packet_info _GetStreamInfo(unsigned int outPortId) override;
    ss_linker_base    *_CreateInputNegativeLinker(unsigned int inPortId) override;
    struct AlgoWriterDesc
    {
        LinkerAsyncNegative   *linker;
        AmigosModuleAudioAlgo *pThisClass;
        void                  *threadHandle;
        AlgoWriterDesc() : linker(nullptr), pThisClass(nullptr), threadHandle(nullptr) {}
    } writerDesc;
    std::vector<ss_linker_audioalgo *>      m_velinker;
    std::map<std::string, ss_linker_base *> m_maplinker;
    ss_linker_base                         *m_linker;
    pthread_mutex_t                         stAlgoLock;
};
#endif
