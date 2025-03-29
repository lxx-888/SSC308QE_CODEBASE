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

#include <stdio.h>
#include <unistd.h>
#include "amigos_env.h"
#include "amigos_module_init.h"
#include "amigos_module_uac.h"
#include "ss_packet.h"
#include "ss_uac_datatype.h"

class AmigosUacStreamPacket : public stream_packet_base
{
public:
    AmigosUacStreamPacket(stream_packet_info &packet_info, SS_UAC_Frame_t *stUacFrame)
    {
        this->en_type = EN_AUDIO_CODEC_DATA;
        this->es_aud_i.sample_rate = packet_info.es_aud_i.sample_rate;
        this->es_aud_i.channels = packet_info.es_aud_i.channels;
        this->es_aud_i.fmt = packet_info.es_aud_i.fmt;
        this->es_aud_i.packet_count = 1;
        this->es_aud_i.sample_width = stUacFrame->bitwith;
        this->es_aud_i.packet_info[0].size = stUacFrame->length;
        this->es_aud.packet_data[0].data = (char*)stUacFrame->data;
    }
    virtual ~AmigosUacStreamPacket()
    {
    }
    stream_packet_obj dup()
    {
        stream_packet_info packet_info;
        packet_info.en_type  = EN_AUDIO_CODEC_DATA;
        packet_info.es_aud_i = this->es_aud_i;
        auto new_pack = stream_packet_base::make<stream_packet>(packet_info);
        assert(new_pack);
        for (unsigned int i = 0; i < new_pack->es_aud_i.packet_count; ++i)
        {
            memcpy(new_pack->es_aud.packet_data[i].data, this->es_aud.packet_data[i].data,
                new_pack->es_aud_i.packet_info[i].size);
        }
        return new_pack;
    }
};
AmigosModuleUac::AmigosModuleUac(const std::string &strInSection)
    : AmigosSurfaceUac(strInSection),
      AmigosModuleBase(this),
      handle(nullptr),
      srcLinker(nullptr),
      dstLinker(nullptr),
      srcLinkerLock(PTHREAD_RWLOCK_INITIALIZER),
      dstLinkerLock(PTHREAD_RWLOCK_INITIALIZER)
{
}
AmigosModuleUac::~AmigosModuleUac()
{
}
unsigned int AmigosModuleUac::GetModId() const
{
    return uintExtModId;
}
unsigned int AmigosModuleUac::GetInputType(unsigned int port) const
{
    return E_MOD_PORT_TYPE_USER;
}
unsigned int AmigosModuleUac::GetOutputType(unsigned int port) const
{
    return E_MOD_PORT_TYPE_USER;
}
void AmigosModuleUac::_StartIn(unsigned int inPortId)
{
    auto iter = this->mapPortIn.find(inPortId);
    if (iter == this->mapPortIn.end())
    {
        AMILOG_ERR << "Base inport " << inPortId << " is not found!" << COLOR_ENDL;
        return;
    }
    ss_auto_rwlock Lock(srcLinkerLock);
    srcLinker = &iter->second.positive;
}
void AmigosModuleUac::_StopIn(unsigned int inPortId)
{
    auto iter = this->mapPortIn.find(inPortId);
    if (iter == this->mapPortIn.end())
    {
        AMILOG_ERR << "Base inport " << inPortId << " is not found!" << COLOR_ENDL;
        return;
    }
    ss_auto_rwlock Lock(srcLinkerLock);
    srcLinker = nullptr;
}
void AmigosModuleUac::_StartOut(unsigned int outPortId)
{
    auto iter = this->mapPortOut.find(outPortId);
    if (iter == this->mapPortOut.end())
    {
        AMILOG_ERR << "Base outport " << outPortId << " is not found!" << COLOR_ENDL;
        return;
    }
    ss_auto_rwlock Lock(dstLinkerLock);
    info.en_type = EN_AUDIO_CODEC_DATA;
    info.es_aud_i.sample_rate = ((SS_UAC_Device_t*)handle)->config[0]->pcm_config.rate;
    info.es_aud_i.channels = ((SS_UAC_Device_t*)handle)->config[0]->pcm_config.channels;
    info.es_aud_i.fmt = ES_STREAM_PCM;
    info.es_aud_i.packet_count = 1;
    dstLinker = &iter->second.positive;
}
void AmigosModuleUac::_StopOut(unsigned int outPortId)
{
    ss_auto_rwlock Lock(dstLinkerLock);
    dstLinker = nullptr;
}
int AmigosModuleUac::_InConnect(unsigned int inPortId)
{
    SS_UAC_StartDev(handle);
    SaveEnvDevPath();
    return 0;
}
int AmigosModuleUac::_InDisconnect(unsigned int inPortId)
{
    int ret = SS_UAC_SUCCESS;
    ClearEnvDevPath();
    ret = SS_UAC_StoptDev(handle);
    if (SS_UAC_SUCCESS != ret)
    {
        AMILOG_ERR << "stop failed!" << COLOR_ENDL;
    }
    return 0;
}
int AmigosModuleUac::_Connected(unsigned int outPortId, unsigned int ref)
{
    // if uac is only ao mode, it need to start dev, otherwise start dev will in _InConnect
    if (!this->stUacInfo.inMode)
    {
        SS_UAC_StartDev(handle);
        SaveEnvDevPath();
    }
    return 0;
}
int AmigosModuleUac::_Disconnected(unsigned int outPortId, unsigned int ref)
{
    if (!this->stUacInfo.inMode)
    {
        int ret = SS_UAC_SUCCESS;
        ClearEnvDevPath();
        ret = SS_UAC_StoptDev(handle);
        if (SS_UAC_SUCCESS != ret)
        {
            AMILOG_ERR << "stop failed!" << COLOR_ENDL;
        }
    }
    return 0;
}
void AmigosModuleUac::_Init()
{
    SS_UAC_Device_t *pstDevice = nullptr;
    SS_UAC_OPS_t opsAo = {AoInit, TakeBuffer, AoDeinit, nullptr};
    SS_UAC_OPS_t opsAi = {AiInit, FillBuffer, AiDeinit, AiSetVolume};
    int ret = SS_UAC_SUCCESS;
    int mode = 0;
    if (this->stUacInfo.inMode)
    {
        mode |= AS_IN_MODE;
    }
    if (this->stUacInfo.outMode)
    {
        mode |= AS_OUT_MODE;
    }
    ret = SS_UAC_AllocStream(this, &handle);
    if (SS_UAC_SUCCESS != ret)
    {
        AMILOG_ERR << "alloc stream failed!" << COLOR_ENDL;
        return;
    }

    pstDevice = (SS_UAC_Device_t *)handle;
    pstDevice->mode = mode;
    // ai attr settings
    if (this->stUacInfo.inMode)
    {
        pstDevice->opsAi = opsAi;
        pstDevice->config[0]->pcm_config.rate = this->stUacInfo.inSampleRate;
        pstDevice->config[0]->pcm_config.channels = this->stUacInfo.inSndMode;
        pstDevice->config[0]->volume.s32Volume = -EINVAL;
    }
    // ao attr settings
    if (this->stUacInfo.outMode)
    {
        pstDevice->opsAo = opsAo;
        pstDevice->config[1]->pcm_config.rate = this->stUacInfo.outSampleRate;
        pstDevice->config[1]->pcm_config.channels = this->stUacInfo.outSndMode;
    }
}
void AmigosModuleUac::_Deinit()
{
    int ret = SS_UAC_SUCCESS;
    ret = SS_UAC_FreeStream(handle);
    if (SS_UAC_SUCCESS != ret)
    {
        AMILOG_ERR << "free stream failed!" << COLOR_ENDL;
    }
    handle = nullptr;
}
void AmigosModuleUac::SaveEnvDevPath()
{
    std::stringstream ss, aud;
    ss << "UAC_" << this->stModInfo.devId << "_" << this->stModInfo.chnId;
    aud << "audio" << this->stModInfo.devId;
    this->env.Ext("UAC_PREVIEW_WINDOWS")[ss.str()] = aud.str();
}
void AmigosModuleUac::ClearEnvDevPath()
{
    std::stringstream ss;
    ss << "UAC_" << this->stModInfo.devId << "_" << this->stModInfo.chnId;
    this->env.Ext("UAC_PREVIEW_WINDOWS")[ss.str()] = "";
}
int AmigosModuleUac::FillBuffer(void *uac, SS_UAC_Frame_t *stUacFrame)
{
    AmigosModuleUac *Obj = (AmigosModuleUac*)((SS_UAC_Device_t*)uac)->private_data;
    if (!Obj)
    {
        AMILOG_ERR << "Object is Null" << COLOR_ENDL;
        return -1;
    }
    ss_auto_rdlock Lock(Obj->srcLinkerLock);
    if (!Obj->srcLinker)
    {
        AMILOG_WRN << "Linker is Null" << COLOR_ENDL;
        return -1;
    }
    auto packet = Obj->srcLinker->dequeue(100);
    if (packet == nullptr)
    {
        return 0;
    }
    memcpy(stUacFrame->data, packet->es_aud.packet_data[0].data, packet->es_aud_i.packet_info[0].size);
    stUacFrame->length = packet->es_aud_i.packet_info[0].size;
    return 0;
}

int AmigosModuleUac::TakeBuffer(void *uac, SS_UAC_Frame_t *stUacFrame)
{
    AmigosModuleUac *Obj = (AmigosModuleUac*)((SS_UAC_Device_t*)uac)->private_data;
    if (!Obj)
    {
        AMILOG_ERR << "Object is Null" << COLOR_ENDL;
        return -1;
    }
    ss_auto_rdlock Lock(Obj->dstLinkerLock);
    if (!Obj->dstLinker)
    {
        AMILOG_WRN << "Linker is Null" << COLOR_ENDL;
        return -1;
    }

    stream_packet_obj packet = stream_packet_base::make<AmigosUacStreamPacket>(Obj->info, stUacFrame);
    if (!packet)
    {
        return -1;
    }
    Obj->dstLinker->enqueue(packet);
    return 0;
}
AMIGOS_MODULE_INIT("UAC", AmigosModuleUac);
