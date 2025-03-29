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
#include "amigos_module_aec.h"

AmigosModuleAec::AmigosModuleAec(const std::string &strInSection)
    : AmigosSurfaceAec(strInSection),
      AmigosModuleBase(this),
      m_handle(nullptr),
      m_workingBuffer(nullptr),
      m_iPeriodSize(0),
      m_bypass(false),
      m_linker(nullptr),
      stAecLock(PTHREAD_MUTEX_INITIALIZER),
      stPacketLock(PTHREAD_MUTEX_INITIALIZER)
{
}
AmigosModuleAec::~AmigosModuleAec() {}
unsigned int AmigosModuleAec::GetModId() const
{
    return uintExtModId;
}
unsigned int AmigosModuleAec::GetInputType(unsigned int port) const
{
    return E_MOD_PORT_TYPE_USER;
}
unsigned int AmigosModuleAec::GetOutputType(unsigned int port) const
{
    return E_MOD_PORT_TYPE_USER;
}

void AmigosModuleAec::_Init()
{
    unsigned int   workingBufferSize;
    ALGO_AEC_RET   ret;
    AudioAecInit   aecInit;
    AudioAecConfig aecConfig;

    aecInit.point_number   = this->stAecInfo.uintPeriodSize;
    aecInit.farend_channel = aecInit.nearend_channel = this->stAecInfo.uintSoundMode;

    aecInit.sample_rate = static_cast<IAA_AEC_SAMPLE_RATE>(this->stAecInfo.uintSampleRate);

    aecConfig.delay_sample         = this->stAecInfo.intDelaySample;
    aecConfig.comfort_noise_enable = static_cast<IAA_AEC_BOOL>(this->stAecInfo.NoiseEnable);
    memcpy(aecConfig.suppression_mode_freq, this->stAecInfo.uintSupFreqArray, sizeof(int) * 6);
    memcpy(aecConfig.suppression_mode_intensity, this->stAecInfo.uintSupIntensityArray, sizeof(int) * 7);
    m_iPeriodSize = aecInit.point_number * aecInit.nearend_channel;

    workingBufferSize = IaaAec_GetBufferSize();
    m_workingBuffer   = new char[workingBufferSize];

    m_handle = IaaAec_Init(m_workingBuffer, &aecInit);
    if (m_handle == nullptr)
    {
        AMIGOS_ERR("AEC init error\n");
        delete[] m_workingBuffer;
        m_workingBuffer = nullptr;
        return;
    }

    ret = IaaAec_Config(m_handle, &(aecConfig));
    if (ret != ALGO_AEC_RET_SUCCESS)
    {
        AMIGOS_ERR("Error occured in AEC config function \n");
        IaaAec_Free(m_handle);
        m_handle = NULL;
        delete[] m_workingBuffer;
        m_workingBuffer = nullptr;
        return;
    }
}

void AmigosModuleAec::_Deinit()
{
    if (nullptr != m_handle)
    {
        IaaAec_Free(m_handle);
        m_handle = nullptr;
    }

    if (nullptr != m_workingBuffer)
    {
        delete[] m_workingBuffer;
        m_workingBuffer = nullptr;
    }
}

void AmigosModuleAec::_StartOut(unsigned int outPortId)
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
    ss_auto_lock autoLock(stAecLock);
    m_linker = &iter->second.positive;
}

void AmigosModuleAec::_StopOut(unsigned int outPortId)
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
    ss_auto_lock autoLock(stAecLock);
    m_linker = nullptr;
}

int AmigosModuleAec::_DataReceiver(unsigned int inPortId, stream_packet_obj &packet)
{
    if (!_checkAecPacket(packet))
    {
        AMIGOS_ERR("AEC hanle or data size is 0\n");
        return 0;
    }
    ss_auto_lock autoLock(stPacketLock);
    if (!m_linker)
    {
        return -1;
    }
    if (m_bypass)
    {
        return m_linker->enqueue(packet);
    }
    int          iSet = 0, iLen = packet->es_aud_i.packet_info[0].size;
    ALGO_AEC_RET ret;
    do
    {
        ret = IaaAec_Run(m_handle,
                        (short *)((char *)packet->es_aud.packet_data[0].data + m_iPeriodSize * iSet * sizeof(short)),
                        (short *)((char *)packet->es_aud.packet_data[1].data + m_iPeriodSize * iSet * sizeof(short)));

        if (ALGO_AEC_RET_SUCCESS != ret)
        {
            AMIGOS_ERR("Error occured in AEC run function, ret error: 0x%x\n", ret);
            return 0;
        }

        iSet++;
    } while (iLen - int(m_iPeriodSize * iSet * sizeof(short)) > 0);
    return m_linker->enqueue(packet);
}

int AmigosModuleAec::AecReset(AudioAecInit &aec_init, AudioAecConfig &aec_config)
{
    ALGO_AEC_RET ret;

    ss_auto_lock autoLock(stPacketLock);
    if (nullptr == m_workingBuffer)
    {
        return -1;
    }

    m_handle = IaaAec_Reset(m_workingBuffer, &aec_init);

    if (m_handle == nullptr)
    {
        AMIGOS_ERR("AEC reset error\n");
        delete[] m_workingBuffer;
        m_workingBuffer = nullptr;
        return -1;
    }

    memcpy(aec_config.suppression_mode_freq, this->stAecInfo.uintSupFreqArray, sizeof(int) * 6);
    memcpy(aec_config.suppression_mode_intensity, this->stAecInfo.uintSupIntensityArray, sizeof(int) * 7);
    m_iPeriodSize = aec_init.point_number * aec_init.nearend_channel;

    ret = IaaAec_Config(m_handle, &(aec_config));

    if (ret != ALGO_AEC_RET_SUCCESS)
    {
        AMIGOS_ERR("Error occured in AEC config function \n");
        delete[] m_workingBuffer;
        m_workingBuffer = nullptr;
        return -1;
    }

    AMIGOS_INFO("AEC reset sucessed!\n");
    return 0;
}

stream_packet_info AmigosModuleAec::_GetStreamInfo(unsigned int outPortId)
{
    stream_packet_info streamInfo;
    streamInfo.en_type               = EN_AUDIO_CODEC_DATA;
    streamInfo.es_aud_i.fmt          = ES_STREAM_PCM;
    streamInfo.es_aud_i.channels     = this->stAecInfo.uintSoundMode;
    streamInfo.es_aud_i.sample_rate  = this->stAecInfo.uintSampleRate;
    streamInfo.es_aud_i.sample_width = 16;
    return streamInfo;
}

stream_packet_obj AmigosModuleAec::_DequeueFromInside(unsigned int outPortId, unsigned int ms)
{
    unsigned int inPortId = outPortId;
    auto itin = this->mapPortIn.find(inPortId);
    if (itin == this->mapPortIn.end())
    {
        AMIGOS_ERR("Input port %d is no exist\n", inPortId);
        return nullptr;
    }
    auto packet =  itin->second.positive.empty() ? nullptr : itin->second.positive.dequeue(ms);
    if (!packet || !_checkAecPacket(packet))
    {
        AMIGOS_WRN("AEC hanle or data size is 0\n");
        return nullptr;
    }
    ss_auto_lock autoLock(stPacketLock);
    int          iSet = 0, iLen = packet->es_aud_i.packet_info[0].size;
    ALGO_AEC_RET ret;
    do
    {
        ret = IaaAec_Run(m_handle,
                        (short *)((char *)packet->es_aud.packet_data[0].data + m_iPeriodSize * iSet * sizeof(short)),
                        (short *)((char *)packet->es_aud.packet_data[1].data + m_iPeriodSize * iSet * sizeof(short)));

        if (ALGO_AEC_RET_SUCCESS != ret)
        {
            AMIGOS_ERR("Error occured in AEC run function, ret error: 0x%x\n", ret);
            return nullptr;
        }

        iSet++;
    } while (iLen - int(m_iPeriodSize * iSet * sizeof(short)) > 0);
    return packet;
}
bool AmigosModuleAec::_IsPostReader(unsigned int inPortId)
{
    unsigned int outPortId = inPortId;
    auto itPortOut = this->mapPortOut.find(outPortId);
    if (itPortOut == this->mapPortOut.end())
    {
        AMIGOS_ERR("Output port %d is no exist\n", outPortId);
        return false;
    }
    return itPortOut->second.positive.empty();
}
AMIGOS_MODULE_INIT("AEC", AmigosModuleAec);
