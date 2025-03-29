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
#include "amigos_log.h"
#include "amigos_module_init.h"
#include "amigos_module_audioencode.h"
#include "audio_code.hpp"
#include "ss_packet.h"

static void _GetPacketSize(stream_packet_info &info);
static es_audio_fmt _CovertFmt(EN_CODE_TYPE type);
static unsigned int _CovertSampleWidth(EN_CODE_TYPE type);
AmigosModuleAudioEncode::AmigosModuleAudioEncode(const std::string &strInSection)
    : AmigosSurfaceAudioEncode(strInSection),
      AmigosModuleBase(this),
      sampleRate(0),
      chnMode(0),
      linker(nullptr),
      codeType(E_G711U),
      adapter(nullptr)
{
}

AmigosModuleAudioEncode::~AmigosModuleAudioEncode() {}
unsigned int AmigosModuleAudioEncode::GetModId() const
{
    return uintExtModId;
}
unsigned int AmigosModuleAudioEncode::GetInputType(unsigned int) const
{
    return E_MOD_PORT_TYPE_USER;
}
unsigned int AmigosModuleAudioEncode::GetOutputType(unsigned int) const
{
    return E_MOD_PORT_TYPE_USER;
}
void AmigosModuleAudioEncode::_Init()
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
void AmigosModuleAudioEncode::_Deinit()
{
    if (adapter)
    {
        delete adapter;
        adapter = nullptr;
    }
}
void AmigosModuleAudioEncode::_Start()
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
void AmigosModuleAudioEncode::_Stop() {}
int AmigosModuleAudioEncode::_Connected(unsigned int outPortId, unsigned int ref)
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
int AmigosModuleAudioEncode::_Disconnected(unsigned int outPortId, unsigned int ref)
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
int AmigosModuleAudioEncode::_DataReceiver(unsigned int inPortId, stream_packet_obj &packet)
{
    if (!linker)
    {
        AMIGOS_ERR("Linker error!\n");
        return -1;
    }
    stream_packet_info packet_info;
    packet_info.en_type = EN_AUDIO_CODEC_DATA;
    packet_info.es_aud_i = packet->es_aud_i;
    packet_info.es_aud_i.fmt = _CovertFmt(codeType);
    _GetPacketSize(packet_info);
    packet_info.es_aud_i.sample_width = _CovertSampleWidth(codeType);
    auto expacket = packer->make(packet_info);
    if (!expacket)
    {
        return -1;
    }
    adapter->encode((unsigned char *)expacket->es_aud.packet_data[0].data,
            (short *)packet->es_aud.packet_data[0].data, packet->es_aud_i.packet_info[0].size);
    return linker->enqueue(expacket);
    return 0;
}
stream_packet_info AmigosModuleAudioEncode::_GetStreamInfo(unsigned int outPortId)
{
    stream_packet_info streamInfo;
    streamInfo.en_type = EN_AUDIO_CODEC_DATA;
    streamInfo.es_aud_i.fmt = _CovertFmt(codeType);
    streamInfo.es_aud_i.channels = chnMode;
    streamInfo.es_aud_i.sample_rate = sampleRate;
    streamInfo.es_aud_i.sample_width = _CovertSampleWidth(codeType);
    return streamInfo;
}
static void _GetPacketSize(stream_packet_info &info)
{
    if (info.es_aud_i.fmt == ES_STREAM_G726_16)
    {
        info.es_aud_i.packet_info[0].size = (unsigned int)((2 * 8000.) / 128000.
            * info.es_aud_i.packet_info[0].size);
    }
    else if (info.es_aud_i.fmt == ES_STREAM_G726_24)
    {
        info.es_aud_i.packet_info[0].size = (unsigned int)((3 * 8000.) / 128000.
            * info.es_aud_i.packet_info[0].size);
    }
    else if (info.es_aud_i.fmt == ES_STREAM_G726_32)
    {
        info.es_aud_i.packet_info[0].size = (unsigned int)((4 * 8000.) / 128000.
            * info.es_aud_i.packet_info[0].size);
    }
    else if (info.es_aud_i.fmt == ES_STREAM_G726_40)
    {
        info.es_aud_i.packet_info[0].size = (unsigned int)((5 * 8000.) / 128000.
            * info.es_aud_i.packet_info[0].size);
    }
    else //g711
    {
        info.es_aud_i.packet_info[0].size = info.es_aud_i.packet_info[0].size / 2;
    }
}
static es_audio_fmt _CovertFmt(EN_CODE_TYPE type)
{
    switch (type)
    {
        case E_G711U:
            return ES_STREAM_G711U;
        case E_G711A:
            return ES_STREAM_G711A;
        case E_G726_16:
            return ES_STREAM_G726_16;
        case E_G726_24:
            return ES_STREAM_G726_24;
        case E_G726_32:
            return ES_STREAM_G726_32;
        case E_G726_40:
            return ES_STREAM_G726_40;
        case E_AAC:
            return ES_STREAM_AAC;
        default:
            return ES_STREAM_PCM;
    }
}
static unsigned int _CovertSampleWidth(EN_CODE_TYPE type)
{
    switch (type)
    {
        case E_G711U:
            return 8;
        case E_G711A:
            return 8;
        case E_G726_16:
            return 2;
        case E_G726_24:
            return 3;
        case E_G726_32:
            return 4;
        case E_G726_40:
            return 5;
        default:
            return 16;
    }
}
AMIGOS_MODULE_INIT("AUDIOENCODE", AmigosModuleAudioEncode);
