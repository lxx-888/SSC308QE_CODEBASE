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
#include <cstdio>
#include "amigos_log.h"
#include "amigos_module_init.h"
#include "amigos_module_audiodecode.h"
#include "audio_code.hpp"
#include "ss_enum_cast.hpp"

static void _GetPacketSize(stream_packet_info &info);
AmigosModuleAudioDecode::AmigosModuleAudioDecode(const std::string &strInSection)
    : AmigosSurfaceAudioDecode(strInSection),
      AmigosModuleBase(this),
      sampleRate(0),
      chnMode(0),
      linker(nullptr),
      codeType(E_G711U),
      adapter(nullptr)
{
}
AmigosModuleAudioDecode::~AmigosModuleAudioDecode() {}
unsigned int AmigosModuleAudioDecode::GetModId() const
{
    return uintExtModId;
}
unsigned int AmigosModuleAudioDecode::GetInputType(unsigned int) const
{
    return E_MOD_PORT_TYPE_USER;
}
unsigned int AmigosModuleAudioDecode::GetOutputType(unsigned int) const
{
    return E_MOD_PORT_TYPE_USER;
}
void AmigosModuleAudioDecode::_Init()
{
    codeType = ss_enum_cast<EN_CODE_TYPE>::from_str(this->stCodeInfo.type);
    if (codeType == E_G711U)
    {
        adapter = new G711U();
    }
    else if (codeType == E_G711A)
    {
        adapter = new G711A();
    }
    else if(codeType == E_G726_16 ||
            codeType == E_G726_24 ||
            codeType == E_G726_32 ||
            codeType == E_G726_40)
    {
        adapter = new G726(static_cast<g726_type_e>(codeType));
    }
}
void AmigosModuleAudioDecode::_Deinit()
{
    if (adapter)
    {
        delete adapter;
        adapter = nullptr;
    }
}
void AmigosModuleAudioDecode::_Start()
{
    stream_packet_info  info;
    auto it = mapPortIn.begin();
    if (mapPortIn.end() == it)
    {
        AMIGOS_ERR("has no input port!, start failed!\n");
        return;
    }
    info = it->second.get_packet_info();
    if (info.en_type != EN_AUDIO_CODEC_DATA)
    {
        AMIGOS_ERR("data type is not audio");
        return;
    }
    sampleRate = info.es_aud_i.sample_rate;
    chnMode    = info.es_aud_i.channels;
}
void AmigosModuleAudioDecode::_Stop() {}
int AmigosModuleAudioDecode::_Connected(unsigned int outPortId, unsigned int ref)
{
    if (ref > 0)
    {
        return 0;
    }
    if(outPortId != 0)
    {
        return -1;
    }
    auto iter = this->mapPortOut.find(outPortId);
    if(iter == this->mapPortOut.end())
    {
        AMIGOS_ERR("Base port %d is not found\n", outPortId);
        return -1;
    }
    linker = &iter->second.positive;
    packer = &iter->second.outPacker;
    return 0;
}
int AmigosModuleAudioDecode::_Disconnected(unsigned int outPortId, unsigned int ref)
{
    if (ref > 0)
    {
        return 0;
    }
    if(outPortId != 0)
    {
        return -1;
    }
    auto iter = this->mapPortOut.find(outPortId);
    if(iter == this->mapPortOut.end())
    {
        AMIGOS_ERR("Base port %d is not found\n", outPortId);
        return -1;
    }
    linker = nullptr;
    return 0;
}
int AmigosModuleAudioDecode::_DataReceiver(unsigned int inPortId, stream_packet_obj &packet)
{
    if (!linker)
    {
        AMIGOS_ERR("Linker error!\n");
        return -1;
    }
    stream_packet_info packet_info;
    packet_info.en_type  = EN_AUDIO_CODEC_DATA;
    packet_info.es_aud_i = packet->es_aud_i;
    _GetPacketSize(packet_info);
    packet_info.es_aud_i.fmt = ES_STREAM_PCM;
    packet_info.es_aud_i.sample_width = 16;
    auto expacket = packer->make(packet_info);
    if (!expacket)
    {
        return -1;
    }
    adapter->decode((short *)expacket->es_aud.packet_data[0].data,
            (unsigned char *)packet->es_aud.packet_data[0].data, packet->es_aud_i.packet_info[0].size);
    return linker->enqueue(expacket);
}
stream_packet_info AmigosModuleAudioDecode::_GetStreamInfo(unsigned int outPortId)
{
    stream_packet_info streamInfo;
    streamInfo.en_type = EN_AUDIO_CODEC_DATA;
    streamInfo.es_aud_i.fmt = ES_STREAM_PCM;
    streamInfo.es_aud_i.channels = chnMode;
    streamInfo.es_aud_i.sample_rate = sampleRate;
    streamInfo.es_aud_i.sample_width = 16;
    return streamInfo;
}
static void _GetPacketSize(stream_packet_info &info)
{
    if (info.es_aud_i.fmt == ES_STREAM_G726_16)
    {
        info.es_aud_i.packet_info[0].size = (unsigned int)(128000. / (2 * 8000.)
            * info.es_aud_i.packet_info[0].size);
    }
    else if (info.es_aud_i.fmt == ES_STREAM_G726_24)
    {
        info.es_aud_i.packet_info[0].size = (unsigned int)(128000. / (3 * 8000.)
            * info.es_aud_i.packet_info[0].size);
    }
    else if (info.es_aud_i.fmt == ES_STREAM_G726_32)
    {
        info.es_aud_i.packet_info[0].size = (unsigned int)(128000. / (4 * 8000.)
            * info.es_aud_i.packet_info[0].size);
    }
    else if (info.es_aud_i.fmt == ES_STREAM_G726_40)
    {
        info.es_aud_i.packet_info[0].size = (unsigned int)(128000. / (5 * 8000.)
            * info.es_aud_i.packet_info[0].size);
    }
    else //g711
    {
        info.es_aud_i.packet_info[0].size *= 2;
    }
}
AMIGOS_MODULE_INIT("AUDIODECODE", AmigosModuleAudioDecode);
