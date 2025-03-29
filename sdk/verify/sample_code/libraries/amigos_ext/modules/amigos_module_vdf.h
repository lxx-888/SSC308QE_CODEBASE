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
#include <mutex>
#include "amigos_module_mi_base.h"
#include "amigos_surface_vdf.h"
#include "amigos_module_rgn_metadata_define.h"
#include "ss_packet.h"
#include "mi_vdf_datatype.h"


class AmigosModuleVdf final: public AmigosSurfaceVdf, public AmigosModuleMiBase
{
public:
    explicit AmigosModuleVdf(const std::string &strSection);
    ~AmigosModuleVdf() override;

public:
    unsigned int GetModId() const override;
    unsigned int GetInputType(unsigned int port) const override;
    unsigned int GetOutputType(unsigned int port) const override;
    void _StartIn(unsigned int inPortId) override {};
    void _StopIn(unsigned int inPortId) override {};
    void SendDetectResult(stream_packet_obj & stPacket, bool bSendforce = false);
    bool GetOutPortApproch();

protected:
    void _Init() override;
    void _Deinit() override;
    int _Connected(unsigned int outPortId, unsigned int ref)override;
    int _Disconnected(unsigned int outPortId, unsigned int ref)override;
    void _StartOut(unsigned int outPortId) override {};
    void _StopOut(unsigned int outPortId) override {};
    void _Prepare()override;
    void _Unprepare()override;

private:
    void _CalMdMap(MI_VDF_MdAttr_t &mdAttr);
    void _CalOdMap(MI_VDF_OdAttr_t &odAttr);
    void _GetMdResult(MI_MD_Result_t &mdResult, AmigosRgnMetaDataMap *map);
    void _GetOdResult(MI_OD_Result_t &odResult, AmigosRgnMetaDataMap *map);

private:
    static void *VdfReader(struct ss_thread_buffer *thread_buf);
    static std::map<unsigned int, unsigned int> mapVdfMode;
    static unsigned int                         vdfInitCount;
    bool               bLastSend;
    bool               bVgFirstSend;
    unsigned int       inportWith;
    unsigned int       inportHeight;
    unsigned int       streamMapWidth, streamMapHeight;
    unsigned int       vdfMapStartX, vdfMapStartY;
    unsigned int       vdfMapEndX, vdfMapEndY;
    stream_packet_obj  vdfPacketInfoObj;
    void               *threadHandle;
};


