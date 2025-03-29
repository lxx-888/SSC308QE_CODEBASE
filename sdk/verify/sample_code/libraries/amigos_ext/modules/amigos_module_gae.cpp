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
#include "ss_packet.h"
#include "amigos_log.h"
#include "amigos_module_init.h"
#include "amigos_module_gae.h"

AmigosModuleGae::AmigosModuleGae(const std::string &strInSection)
    : AmigosSurfaceGae(strInSection),
      AmigosModuleBase(this),
      m_linker(nullptr)
{
}
AmigosModuleGae::~AmigosModuleGae() {}
unsigned int AmigosModuleGae::GetModId() const
{
    return uintExtModId;
}
unsigned int AmigosModuleGae::GetInputType(unsigned int port) const
{
    return E_MOD_PORT_TYPE_USER;
}
unsigned int AmigosModuleGae::GetOutputType(unsigned int port) const
{
    return E_MOD_PORT_TYPE_USER;
}

void AmigosModuleGae::_Init()
{
    return;
}

void AmigosModuleGae::_Deinit()
{
    return;
}

void AmigosModuleGae::_StartOut(unsigned int outPortId)
{
    if (outPortId != 0)
    {
        return;
    }
    auto iter = this->mapPortOut.find(outPortId);
    if (iter == this->mapPortOut.end())
    {
        AMIGOS_ERR("Base port %d is not found\n", outPortId);
        return;
    }
    m_linker = &iter->second.positive;
}

void AmigosModuleGae::_StopOut(unsigned int outPortId)
{
    if (outPortId != 0)
    {
        return;
    }
    auto iter = this->mapPortOut.find(outPortId);
    if (iter == this->mapPortOut.end())
    {
        AMIGOS_ERR("Base port %d is not found\n", outPortId);
        return;
    }
    m_linker = nullptr;
}

int AmigosModuleGae::_DataReceiver(unsigned int inPortId, stream_packet_obj &packet)
{
    if (!m_linker)
    {
        return -1;
    }
    stream_packet_info packet_info;
    packet_info.en_type              = EN_USER_META_DATA;
    packet_info.meta_data_i.reserved = 0;
    packet_info.meta_data_i.size     = sizeof(struct MI_ISP_AE_ExpoInfoType_s);
    stream_packet_obj stAEExpoInfoPacket         = stream_packet_base::make<stream_packet>(packet_info);
    if (!stAEExpoInfoPacket)
    {
        AMIGOS_ERR("Packet make failed\n");
        return -1;
    }
    struct MI_ISP_AE_ExpoInfoType_s *pstAEExpoInfo = (struct MI_ISP_AE_ExpoInfoType_s*)stAEExpoInfoPacket->meta_data.data;
    if(MI_SUCCESS != MI_ISP_AE_QueryExposureInfo(this->stGaeInfo.intIspDev,this->stGaeInfo.intIspChn, pstAEExpoInfo))
    {
        AMIGOS_ERR("MI_ISP_AE_QueryExposureInfo failed\n");
        return -1;
    }
    AMIGOS_INFO("ispgain=%u,sensorgain=%u,shutter=%u\n",pstAEExpoInfo->stExpoValueLong.u32ISPGain,pstAEExpoInfo->stExpoValueLong.u32SensorGain,pstAEExpoInfo->stExpoValueLong.u32US);
    return m_linker->enqueue(stAEExpoInfoPacket);
}

AMIGOS_MODULE_INIT("GAE", AmigosModuleGae);

