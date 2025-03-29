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

#ifndef __AMIGOS_MODULE_PARES_H__
#define __AMIGOS_MODULE_PARES_H__

#include <string>
#include "amigos_module_mi_base.h"
#include "amigos_surface_pares.h"
#include "ss_packet.h"

class AmigosModulePares : public AmigosSurfacePares, public AmigosModuleMiBase
{
public:
    explicit AmigosModulePares(const std::string &strInSection);
    ~AmigosModulePares() override;
private:
    unsigned int GetModId() const override;
    unsigned int GetInputType(unsigned int port) const override;
    unsigned int GetOutputType(unsigned int port) const override;
    bool _IsPostReader(unsigned int inPortId) override;
    int _DataReceiver(unsigned int inPortId, stream_packet_obj &packet) override;
    int _Connected(unsigned int outPortId, unsigned int ref);
    int _Disconnected(unsigned int outPortId, unsigned int ref);
    void _StartIn(unsigned int inPortId) override;
    void _StopIn(unsigned int inPortId) override;
    void _StartOut(unsigned int outPortId) override;
    void _StopOut(unsigned int outPortId) override;
    int _EnqueueOut(unsigned int outPortId, stream_packet_obj &packet) override;
    int _DequeueOut(unsigned int outPortId, stream_packet_obj &packet) override;
    stream_packet_obj _DequeueFromInside(unsigned int outPortId, unsigned int ms) override;
    ss_linker_base *_CreateInputNegativeLinker(unsigned int inPortId) override;
    stream_packer *_CreateInputStreamPacker(unsigned int inPortId, bool &bFast) override;
    stream_packet_info _GetStreamInfo(unsigned int outPortId);
    void _Init() override;
    void _Deinit() override;
};

#endif /* __AMIGOS_MODULE_PARES_H__ */
