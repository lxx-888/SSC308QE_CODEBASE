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

#include <string>

#include "amigos_module_init.h"
#include "amigos_module_audioalgo.h"
#include "amigos_module_aio.h"
#include "ss_packet.h"
#include "ss_thread.h"

SS_ENUM_CAST_STR(IAA_HPF_FREQ, {
                                   {AUDIO_HPF_FREQ_80, "80"},
                                   {AUDIO_HPF_FREQ_120, "120"},
                                   {AUDIO_HPF_FREQ_150, "150"},
                                   {AUDIO_HPF_FREQ_500, "500"},
                                   {AUDIO_HPF_FREQ_BUTT, "BUTT"},
                               });

void *AlgoReceiverMonitor(struct ss_thread_buffer *thread_buf);

AmigosModuleAudioAlgo::AmigosModuleAudioAlgo(const std::string &strInSection)
    : AmigosSurfaceAudioAlgo(strInSection),
      AmigosModuleBase(this),
      m_linker(nullptr),
      stAlgoLock(PTHREAD_MUTEX_INITIALIZER)
{
}
AmigosModuleAudioAlgo::~AmigosModuleAudioAlgo() {}
unsigned int AmigosModuleAudioAlgo::GetModId() const
{
    return uintExtModId;
}
unsigned int AmigosModuleAudioAlgo::GetInputType(unsigned int port) const
{
    return E_MOD_PORT_TYPE_USER;
}
unsigned int AmigosModuleAudioAlgo::GetOutputType(unsigned int port) const
{
    return E_MOD_PORT_TYPE_USER;
}

void AmigosModuleAudioAlgo::_Init()
{
    ss_linker_audioalgo::Audio_Algorith_t m_stAudioAlgo;

    memset(&m_stAudioAlgo, 0, sizeof(m_stAudioAlgo));
    m_stAudioAlgo.enSampleRate   = (MI_AUDIO_SampleRate_e)this->stAlgoInfo.uintSampleRate;
    m_stAudioAlgo.enSoundMode    = ss_enum_cast<MI_AUDIO_SoundMode_e>::from_str(this->stAlgoInfo.strSoundMode);
    m_stAudioAlgo.u32PeriodSize  = this->stAlgoInfo.uintPeriodSize;
    ss_linker_audioalgo *pLinker = nullptr;

    if (this->stAlgoInfo.stApcInfo.onoff)
    {
        // anr attr
        ss_linker_apc::ApcParm stApcInfo;
        stApcInfo.stAnrCfg.bEnable      = this->stAlgoInfo.stAnrInfo.onoff;
        stApcInfo.stAnrCfg.user_mode    = this->stAlgoInfo.stAnrInfo.uintUserMode;
        stApcInfo.stAnrCfg.smooth_level = this->stAlgoInfo.stAnrInfo.uintSmoothLevel;
        stApcInfo.stAnrCfg.speed        = this->stAlgoInfo.stAnrInfo.uintSpeed;

        // eq/hpf attr
        stApcInfo.stEqCfg.bEnable     = this->stAlgoInfo.stEqInfo.onoff;
        stApcInfo.stEqCfg.user_mode   = this->stAlgoInfo.stEqInfo.uintUserMode;
        stApcInfo.stHpfCfg.user_mode  = this->stAlgoInfo.stHpfInfo.uintUserMode;
        stApcInfo.stHpfCfg.CutoffFreq = ss_enum_cast<IAA_HPF_FREQ>::from_str(this->stAlgoInfo.stHpfInfo.strCutoffFreq);

        // agc attr
        stApcInfo.stAgcCfg.bEnable           = this->stAlgoInfo.stAgcInfo.onoff;
        stApcInfo.stAgcCfg.user_mode         = this->stAlgoInfo.stAgcInfo.uintUserMode;
        stApcInfo.stAgcCfg.DropGainMax       = this->stAlgoInfo.stAgcInfo.uintDropGainMax;
        stApcInfo.stAgcCfg.AttackTime        = this->stAlgoInfo.stAgcInfo.uintAttackTime;
        stApcInfo.stAgcCfg.ReleaseTime       = this->stAlgoInfo.stAgcInfo.uintReleaseTime;
        stApcInfo.stAgcCfg.DropGainThreshold = this->stAlgoInfo.stAgcInfo.intDropGainThreshold;

        try
        {
            pLinker = new ss_linker_apc(m_stAudioAlgo, stApcInfo);
        }
        catch (std::bad_alloc &)
        {
            AMIGOS_ERR("Create Apc error\n");
            return;
        }

        m_velinker.push_back(pLinker);
        m_maplinker["apc"] = pLinker;
    }
    else
    {
        if (this->stAlgoInfo.stAnrInfo.onoff)
        {
            ss_linker_anr::AnrParm stAnrInfo;
            stAnrInfo.bEnable      = this->stAlgoInfo.stAnrInfo.onoff;
            stAnrInfo.user_mode    = this->stAlgoInfo.stAnrInfo.uintUserMode;
            stAnrInfo.smooth_level = this->stAlgoInfo.stAnrInfo.uintSmoothLevel;
            stAnrInfo.speed        = this->stAlgoInfo.stAnrInfo.uintSpeed;

            try
            {
                pLinker = new ss_linker_anr(m_stAudioAlgo, stAnrInfo);
            }
            catch (std::bad_alloc &)
            {
                AMIGOS_ERR("Create Anr error\n");
                return;
            }

            m_velinker.push_back(pLinker);
            m_maplinker["anr"] = pLinker;
        }

        if (this->stAlgoInfo.stEqInfo.onoff)
        {
            ss_linker_eq::EqParm  stEqInfo;
            ss_linker_eq::HpfParm stHpfInfo;
            stEqInfo.bEnable     = this->stAlgoInfo.stEqInfo.onoff;
            stEqInfo.user_mode   = this->stAlgoInfo.stEqInfo.uintUserMode;
            stHpfInfo.user_mode  = this->stAlgoInfo.stHpfInfo.uintUserMode;
            stHpfInfo.CutoffFreq = ss_enum_cast<IAA_HPF_FREQ>::from_str(this->stAlgoInfo.stHpfInfo.strCutoffFreq);

            try
            {
                pLinker = new ss_linker_eq(m_stAudioAlgo, stEqInfo, stHpfInfo);
            }
            catch (std::bad_alloc &)
            {
                AMIGOS_ERR("Create eq error\n");
                return;
            }

            m_velinker.push_back(pLinker);
            m_maplinker["eq"] = pLinker;
        }

        if (this->stAlgoInfo.stAgcInfo.onoff)
        {
            ss_linker_agc::AgcParm stAgcInfo;
            stAgcInfo.bEnable           = this->stAlgoInfo.stAgcInfo.onoff;
            stAgcInfo.user_mode         = this->stAlgoInfo.stAgcInfo.uintUserMode;
            stAgcInfo.DropGainMax       = this->stAlgoInfo.stAgcInfo.uintDropGainMax;
            stAgcInfo.AttackTime        = this->stAlgoInfo.stAgcInfo.uintAttackTime;
            stAgcInfo.ReleaseTime       = this->stAlgoInfo.stAgcInfo.uintReleaseTime;
            stAgcInfo.DropGainThreshold = this->stAlgoInfo.stAgcInfo.uintReleaseTime;

            try
            {
                pLinker = new ss_linker_agc(m_stAudioAlgo, stAgcInfo);
            }
            catch (std::bad_alloc &)
            {
                AMIGOS_ERR("Create Agc error\n");
                return;
            }

            m_velinker.push_back(pLinker);
            m_maplinker["agc"] = pLinker;
        }
    }

    if (this->stAlgoInfo.stSedInfo.onoff)
    {
        ss_linker_sed::SedParm stSedInfo;
        stSedInfo.bEnable            = this->stAlgoInfo.stSedInfo.onoff;
        stSedInfo.smooth_length      = this->stAlgoInfo.stSedInfo.intSmoothLength;
        stSedInfo.path               = this->stAlgoInfo.stSedInfo.strModelPath;
        stSedInfo.detect_mode        = this->stAlgoInfo.stSedInfo.intDetectMode;
        stSedInfo.vad_threshold      = this->stAlgoInfo.stSedInfo.fVadTheshold;
        stSedInfo.event_threshold[0] = this->stAlgoInfo.stSedInfo.fEventTheshold[0];
        stSedInfo.event_threshold[1] = this->stAlgoInfo.stSedInfo.fEventTheshold[1];
        try
        {
            pLinker = new ss_linker_sed(m_stAudioAlgo, stSedInfo);
        }
        catch (std::bad_alloc &)
        {
            AMIGOS_ERR("Create Sed error\n");
            return;
        }

        m_velinker.push_back(pLinker);
        m_maplinker["sed"] = pLinker;
    }

    if (this->stAlgoInfo.stSrcInfo.onoff)
    {
        ss_linker_src::SrcParm stSrcInfo;
        stSrcInfo.order         = static_cast<SrcFilterOrder>(this->stAlgoInfo.stSrcInfo.uintorder);
        stSrcInfo.OutSampleRate = static_cast<MI_AUDIO_SampleRate_e>(this->stAlgoInfo.stSrcInfo.uintOutSampleRate);

        try
        {
            pLinker = new ss_linker_src(m_stAudioAlgo, stSrcInfo);
        }
        catch (std::bad_alloc &)
        {
            AMIGOS_ERR("Create Src error\n");
            return;
        }

        m_velinker.push_back(pLinker);
        m_maplinker["src"] = pLinker;
    }
}
void AmigosModuleAudioAlgo::_Deinit()
{
    for (auto &it : m_velinker)
    {
        if (it != nullptr)
        {
            delete it;
            it = nullptr;
        }
    }

    m_velinker.clear();
}

void AmigosModuleAudioAlgo::_StartIn(unsigned int inPortId)
{
    if (inPortId != 0)
    {
        AMIGOS_ERR("Audio Algo port%d error", inPortId);
        return;
    }

    if (!stAlgoInfo.bSyncMode)
    {
        CreateReceiver(inPortId);
    }
}

void AmigosModuleAudioAlgo::_StopIn(unsigned int inPortId)
{
    if (inPortId != 0)
    {
        AMIGOS_ERR("Audio Algo port%d error", inPortId);
        return;
    }

    if (!stAlgoInfo.bSyncMode)
    {
        DestoryReceiver(inPortId);
    }
}

int AmigosModuleAudioAlgo::_Connected(unsigned int outPortId, unsigned int ref)
{
    if (ref > 0)
    {
        return 0;
    }
    if (outPortId != 0)
    {
        return -1;
    }
    auto iter = this->mapPortOut.find(outPortId);
    if (iter == this->mapPortOut.end())
    {
        AMIGOS_ERR("Base port %d is not found\n", outPortId);
        return -1;
    }
    ss_auto_lock autoLock(stAlgoLock);
    m_linker = &iter->second.positive;
    for (auto i = m_velinker.size(); i > 0; --i)
    {
        m_velinker[i - 1]->SetDstLinker(m_linker);
        m_linker = m_velinker[i - 1];
    }
    return 0;
}
int AmigosModuleAudioAlgo::_Disconnected(unsigned int outPortId, unsigned int ref)
{
    if (ref > 0)
    {
        return 0;
    }
    if (outPortId != 0)
    {
        return -1;
    }
    auto iter = this->mapPortOut.find(outPortId);
    if (iter == this->mapPortOut.end())
    {
        AMIGOS_ERR("Base port %d is not found\n", outPortId);
        return -1;
    }
    ss_auto_lock autoLock(stAlgoLock);
    m_linker = NULL;
    return 0;
}

int AmigosModuleAudioAlgo::_DataReceiver(unsigned int inPortId, stream_packet_obj &packet)
{
    ss_auto_lock autoLock(stAlgoLock);
    if (!m_linker)
    {
        AMIGOS_ERR("Linker error!\n");
        return -1;
    }
    return m_linker->enqueue(packet);
}

ss_linker_base *AmigosModuleAudioAlgo::_CreateInputNegativeLinker(unsigned int inPortId)
{
    if (stAlgoInfo.bSyncMode)
    {
        return new LinkerSyncNegative(inPortId, this);
    }
    return new LinkerAsyncNegative(false, 8);
}

void AmigosModuleAudioAlgo::CreateReceiver(unsigned int inPortId)
{
    auto iter = this->mapPortIn.find(inPortId);

    if (iter == this->mapPortIn.end())
    {
        AMIGOS_ERR("Base port %d is not found\n", inPortId);
        return;
    }
    this->writerDesc.linker     = dynamic_cast<LinkerAsyncNegative *>(iter->second.negative);
    this->writerDesc.pThisClass = this;
    assert(this->writerDesc.linker);

    ss_thread_attr ss_attr;
    memset(&ss_attr, 0, sizeof(ss_thread_attr));
    ss_attr.do_signal          = NULL;
    ss_attr.do_monitor         = AlgoReceiverMonitor;
    ss_attr.monitor_cycle_sec  = 0;
    ss_attr.monitor_cycle_nsec = 0;
    ss_attr.is_reset_timer     = 0;
    ss_attr.in_buf.buf         = (&this->writerDesc);
    ss_attr.in_buf.size        = 0;
    snprintf(ss_attr.thread_name, 128, "%s", this->GetInPortIdStr(inPortId).c_str());
    this->writerDesc.threadHandle = ss_thread_open(&ss_attr);
    if (!this->writerDesc.threadHandle)
    {
        AMIGOS_ERR("Monitor return error!\n");
        return;
    }

    ss_thread_start_monitor(this->writerDesc.threadHandle);
    AMIGOS_INFO("Algo writer %d start\n", inPortId);
    return;
}

void AmigosModuleAudioAlgo::DestoryReceiver(unsigned int inPortId)
{
    if (!this->writerDesc.linker && 0 != inPortId)
    {
        return;
    }
    ss_thread_stop(this->writerDesc.threadHandle);
    ss_thread_close(this->writerDesc.threadHandle);
    this->writerDesc.linker       = nullptr;
    this->writerDesc.pThisClass   = nullptr;
    this->writerDesc.threadHandle = nullptr;
}

void *AlgoReceiverMonitor(struct ss_thread_buffer *thread_buf)
{
    AmigosModuleAudioAlgo::AlgoWriterDesc *desc = (AmigosModuleAudioAlgo::AlgoWriterDesc *)thread_buf->buf;

    auto packet = desc->linker->WaitPacket(-1);
    if (packet == nullptr)
    {
        // Time out!
        return NULL;
    }
    if (packet->en_type != EN_AUDIO_CODEC_DATA)
    {
        return NULL;
    }
    assert(desc->pThisClass);
    ss_auto_lock autoLock(desc->pThisClass->stAlgoLock);
    if (!desc->pThisClass->m_linker)
    {
        AMIGOS_ERR("Linker is null\n");
        return NULL;
    }
    desc->pThisClass->m_linker->enqueue(packet);
    return NULL;
}
stream_packet_info AmigosModuleAudioAlgo::_GetStreamInfo(unsigned int outPortId)
{
    stream_packet_info streamInfo;
    streamInfo.en_type               = EN_AUDIO_CODEC_DATA;
    streamInfo.es_aud_i.fmt          = ES_STREAM_PCM;
    streamInfo.es_aud_i.channels     = ss_enum_cast<MI_AUDIO_SoundMode_e>::from_str(this->stAlgoInfo.strSoundMode);
    streamInfo.es_aud_i.sample_rate  = this->stAlgoInfo.uintSampleRate;
    streamInfo.es_aud_i.sample_width = 16;
    return streamInfo;
}
AMIGOS_MODULE_INIT("AUDIOALGO", AmigosModuleAudioAlgo);
