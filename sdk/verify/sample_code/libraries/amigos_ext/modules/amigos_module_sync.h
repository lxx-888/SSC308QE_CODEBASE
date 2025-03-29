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
#pragma once

#include "amigos_module_base.h"
#include "amigos_surface_sync.h"


class AmigosModuleSync final: public AmigosSurfaceSync, public AmigosModuleBase
{
public:
    explicit AmigosModuleSync(const std::string &strSection);
    ~AmigosModuleSync() override;

public:
    unsigned int GetModId() const override;
    unsigned int GetInputType(unsigned int port) const override;
    unsigned int GetOutputType(unsigned int port) const override;

private:
    void _Init() override;
    void _Deinit() override;
    void _Start() override;
    void _Stop() override;
    bool _IsDelayConnected(unsigned int uintInPort) override;
    bool _IsPostReader(unsigned int inPortId) override;
    int _ConnectedTransfer(unsigned int outPortId) override;
    int _DisconnectedTransfer(unsigned int outPortId) override;
    bool _NeedResetStreamTraverse(unsigned int outPortId, unsigned int bindType, unsigned int &inPortId) override;
    stream_packet_info _GetStreamInfo(unsigned int outPortId) override;
    stream_packer *_CreateInputStreamPacker(unsigned int inPortId, bool &bFast) override;

private:
    static void *SyncReader(struct ss_thread_buffer *thread_buf);
    void *threadHandle;
};


