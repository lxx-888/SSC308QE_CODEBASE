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

#ifndef __AMIGOS_MODULE_AO_H__
#define __AMIGOS_MODULE_AO_H__

#include "amigos_module_mi_base.h"
#include "amigos_surface_ao.h"
#include "mi_ao_datatype.h"
#include "ss_packet.h"

class AmigosModuleAo: public AmigosSurfaceAo, public AmigosModuleMiBase
{
    friend void *ReceiverMonitor(struct ss_thread_buffer *thread_buf);
    friend int SetAoIfMute(std::vector<std::string> & in_strs);
public:
    explicit AmigosModuleAo(const std::string &strInSection);
    ~AmigosModuleAo() override;
    int _InConnect(unsigned int inPortId) override;
    int _InDisconnect(unsigned int inPortId) override;
    unsigned int GetModId() const override;
    unsigned int GetInputType(unsigned int port) const override;
    unsigned int GetOutputType(unsigned int port) const override;
    inline virtual unsigned int GetDevId() const
    {
        return this->stModInfo.devId;
    };
protected:
    void _Init() override;
    void _Deinit() override;
    void _ResourceInit() override;
    void _ResourceDeinit() override;
    int _DataReceiver(unsigned int inPortId, stream_packet_obj &packet) override;
#ifdef CONFIG_MI_SYSCALL_L2R_DESTROY_SUPPORT
    bool _NeedMarkDeinitOnRtos() override;
#endif

private:
    stream_packer *_CreateInputStreamPacker(unsigned int inPortId, bool &bFast) override;
    ss_linker_base *_CreateInputNegativeLinker(unsigned int inPortId) override;
    void CreateReceiver(unsigned int outPortId);
    void DestoryReceiver(unsigned int inPortId);
    MI_AO_If_e enAoIf;
    struct AoWriterDesc
    {
        LinkerAsyncNegative *linker;
        unsigned int        devId;
        void                *threadHandle;
        AoWriterDesc() : linker(nullptr), devId(0), threadHandle(nullptr) {}
    } writerDesc;

};
#endif

