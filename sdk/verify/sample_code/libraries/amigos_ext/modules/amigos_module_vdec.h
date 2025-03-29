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
#ifndef __AMIGOS_MODULE_VDEC_H__
#define __AMIGOS_MODULE_VDEC_H__

#include "amigos_module_base.h"
#include "amigos_module_mi_base.h"
#include "amigos_surface_vdec.h"

class AmigosModuleVdec: public AmigosSurfaceVdec, public AmigosModuleMiBase
{
    struct VdecWriterDesc
    {
        enum es_video_fmt fmt;
        VdecWriterDesc() : fmt(ES_STREAM_H265) {}
    };

public:
    explicit AmigosModuleVdec(const std::string &strSection);
    ~AmigosModuleVdec() override;

public:
    unsigned int GetModId() const override;
    unsigned int GetInputType(unsigned int port) const override;
    unsigned int GetOutputType(unsigned int port) const override;

private:
    void _Init() override;
    void _Deinit() override;
    int  _DataReceiver(unsigned int inPortId, stream_packet_obj &packet) override;
    void _StartIn(unsigned int inPortId) override;
    void _StopIn(unsigned int inPortId) override;
    void _ResetStreamOut(unsigned int outPortId, unsigned int width, unsigned int height) override;
    ss_linker_base *_CreateInputNegativeLinker(unsigned int inPortId) override;
    stream_packer  *_CreateInputStreamPacker(unsigned int inPortId, bool &bFast) override;
    int _SendStream2Hw(const timeval &timeStamp, char *data, unsigned int size);
    VdecWriterDesc stWriterDesc;
    AmigosSurfaceVdec *pVdecSurface;
};
#endif

