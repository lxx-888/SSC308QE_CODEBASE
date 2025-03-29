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
#ifndef __AMIGOS_MODULE_SAE_H__
#define __AMIGOS_MODULE_SAE_H__

#include <cstdio>
#include <cstring>
#include "mi_common_datatype.h"
#include "mi_isp_ae.h"
#include "amigos_log.h"
#include "amigos_surface_sae.h"
#include "ss_enum_cast.hpp"

class AmigosModuleSae : public AmigosSurfaceSae, public AmigosModuleBase
{
public:
    explicit AmigosModuleSae(const std::string &strInSection);
    ~AmigosModuleSae() override;
    unsigned int GetModId() const override;
    unsigned int GetInputType(unsigned int port) const override;
    unsigned int GetOutputType(unsigned int port) const override;

protected:
    void _Init() override;
    void _Deinit() override;
    int  _DataReceiver(unsigned int inPortId, stream_packet_obj &packet) override;
    void _StartIn(unsigned int inPortId) override;
    void _StopIn(unsigned int inPortId) override;

private:
    //bool _IsPostReader(unsigned int inPortId) override;
    //stream_packet_info _GetStreamInfo(unsigned int outPortId) override;
    //stream_packet_obj _DequeueFromInside(unsigned int outPortId, unsigned int ms) override;
};

#endif


