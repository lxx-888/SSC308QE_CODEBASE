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

#ifndef __AMIGOS_MODULE_AUDIODECODE_H__
#define __AMIGOS_MODULE_AUDIODECODE_H__

#include "amigos_surface_audiodecode.h"
#include "amigos_module_base.h"
#include "audio_code.hpp"
#include "ss_connector.h"
#define ALIGN_2xUP(x)               (((x+1) / 2) * 2)

class AmigosModuleAudioDecode : public AmigosSurfaceAudioDecode, public AmigosModuleBase
{
public:
    explicit AmigosModuleAudioDecode(const std::string &strInSection);
    ~AmigosModuleAudioDecode() override;
    unsigned int GetModId() const override;
    unsigned int GetInputType(unsigned int port) const override;
    unsigned int GetOutputType(unsigned int port) const override;
protected:
    void _Init() override;
    void _Deinit() override;
    void _Start() override;
    void _Stop() override;
    int  _DataReceiver(unsigned int inPortId, stream_packet_obj &packet) override;
    int  _Connected(unsigned int outPortId, unsigned int ref) override;
    int  _Disconnected(unsigned int outPortId, unsigned int ref) override;
private:
    stream_packet_info _GetStreamInfo(unsigned int outPortId) override;
    unsigned int    sampleRate;
    unsigned int    chnMode;
    ss_linker_base *linker;
    EN_CODE_TYPE    codeType;
    AudioCode      *adapter;
    stream_packer  *packer;
};
#endif
