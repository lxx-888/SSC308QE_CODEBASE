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

#ifndef __AMIGOS_MODULE_SCL_STRETCH_H__
#define __AMIGOS_MODULE_SCL_STRETCH_H__

#include <map>
#include "amigos_env.h"
#include "amigos_module_mi_base.h"
#include "amigos_module_scl_base.h"
#include "amigos_surface_scl_stretch.h"
#include "mi_scl_datatype.h"

class AmigosModuleSclStretch : public AmigosSurfaceSclStretch, public AmigosModuleSclBase
{
private:
    static void *SclStretchReader(struct ss_thread_buffer *thread_buf);
private:
    struct SclStretchReaderDesc
    {
        LinkerGroup            *linker;
        stream_packer          *packer;
        AmigosModuleSclStretch *pModule;
        unsigned int           outPortId;
        void                   *threadHandle;
        SclStretchReaderDesc() : linker(nullptr), pModule(nullptr), outPortId(0), threadHandle(nullptr) {}
    };
public:
    explicit AmigosModuleSclStretch(const std::string &strSection);
    ~AmigosModuleSclStretch() override;
    unsigned int GetModId() const override;
    unsigned int GetInputType(unsigned int port) const override;
    unsigned int GetOutputType(unsigned int port) const override;
private:
    void _Init() override;
    void _Deinit() override;
    int _InConnect(unsigned int inPortId) override;
    int _InDisconnect(unsigned int inPortId) override;
    stream_packet_obj _DequeueFromInside(unsigned int outPortId, unsigned int ms) override;
    int _FillPacket(unsigned int outPortId, stream_packet_obj &packet);
    ss_linker_base *_CreateInputNegativeLinker(unsigned int inPortId) override;
    stream_packer *_CreateInputStreamPacker(unsigned int inPortId, bool &bFast) override;
private:
    std::map<unsigned int, SclStretchReaderDesc> mapReaderDesc;
    MI_PHY                                       phyOsdBuf;
};
#endif
