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

#ifndef __AMIGOS_MODULE_AI_H__
#define __AMIGOS_MODULE_AI_H__

#include "amigos_module_mi_base.h"
#include "amigos_surface_ai.h"
#include "mi_ai_datatype.h"

class AmigosModuleAi: public AmigosSurfaceAi, public AmigosModuleMiBase
{
    friend int SetIfGain(std::vector<std::string> &in_strs);
    friend int GetIfGain(std::vector<std::string> &in_strs);
    friend int SetAiIfMute(std::vector<std::string> &in_strs);
    friend int SetAiMute(std::vector<std::string> &in_strs);
    friend int SetGain(std::vector<std::string> &in_strs);
public:
    explicit AmigosModuleAi(const std::string &strInSection);
    ~AmigosModuleAi() override;
    unsigned int GetModId() const override;
    unsigned int GetInputType(unsigned int port) const override;
    unsigned int GetOutputType(unsigned int port) const override;
protected:
    void _Init() override;
    void _Deinit() override;
    void _ResourceInit() override;
    void _ResourceDeinit() override;
#ifdef CONFIG_MI_SYSCALL_L2R_DESTROY_SUPPORT
    bool _NeedMarkDeinitOnRtos() override;
#endif
    inline virtual unsigned int GetDevId() const
    {
        return this->stModInfo.devId;
    };
    virtual unsigned int GetAiSoundMod() const;
    virtual MI_AI_If_e GetAiIf(unsigned int index);
    int _EnqueueOut(unsigned int outPortId, stream_packet_obj &packet) override;
    int _DequeueOut(unsigned int outPortId, stream_packet_obj &packet) override;
    stream_packet_obj _DequeueFromInside(unsigned int outPortId, unsigned int ms) override;
    stream_packet_info _GetStreamInfo(unsigned int outPortId) override;
    MI_AI_If_e enAiIf[MI_AI_MAX_CHN_NUM / 2]; //ai max interface num is MI_AI_MAX_CHN_NUM / 2
private:
    int _Connected(unsigned int outPortId, unsigned int ref) override;
    int _Disconnected(unsigned int outPortId, unsigned int ref) override;
    static void *AiReader(struct ss_thread_buffer *thread_buf);
    struct AiReaderDesc
    {
        LinkerGroup    *linker;
        unsigned int   outPortId;
        AmigosModuleAi *pModule;
        int            fd;
        void           *threadHandle;
        AiReaderDesc() : linker(nullptr), outPortId(0), pModule(nullptr), fd(-1), threadHandle(nullptr){}
    } readerDesc;
    bool bEcho;
    es_audio_info packet_info;
};
#endif

