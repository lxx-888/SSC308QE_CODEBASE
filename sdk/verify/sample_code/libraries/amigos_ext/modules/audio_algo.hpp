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
#ifndef _AUDIO_ALGO_H__
#define _AUDIO_ALGO_H__

#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <cassert>
#include <memory>

#include "mi_ai_datatype.h"
#include "mi_aio_datatype.h"
#include "AudioAecProcess.h"
#include "AudioProcess.h"
#include "AudioSedProcess.h"
#include "AudioSRCProcess.h"
#include "ss_linker.h"
#include "ss_auto_lock.h"
#include "amigos_log.h"
#include "ss_packet.h"

#define MIN(a, b) ((a) < (b) ? (a) : (b))
class ss_linker_audioalgo : public ss_linker_base
{
public:
    struct Audio_Algorith_t
    {
        MI_AUDIO_SoundMode_e  enSoundMode;
        MI_AUDIO_SampleRate_e enSampleRate;
        MI_U32                u32PeriodSize;
    };

public:
    ss_linker_audioalgo()
        : m_handle(nullptr), m_workingBuffer(nullptr), m_iPeriodSize(0), m_bypass(false), dstLinker(nullptr){};
    virtual ~ss_linker_audioalgo(){};
    virtual int enqueue(stream_packet_obj &packet) override
    {
        if (!m_bypass)
        {
            Run(packet);
        }
        if (!dstLinker)
        {
            AMIGOS_ERR("Dst linker is null!\n");
            return -1;
        }
        return dstLinker->enqueue(packet);
    };
    void SetBypass(bool bypass)
    {
        m_bypass = bypass;
    }
    void SetDstLinker(ss_linker_base *linker)
    {
        dstLinker = linker;
    }
    virtual int Run(stream_packet_obj &packet)
    {
        return 0;
    }
    virtual int Run(stream_packet_obj &packet, stream_packet_obj &expacket)
    {
        return 0;
    }

protected:
    void           *m_handle;
    char           *m_workingBuffer;
    int             m_iPeriodSize;
    bool            m_bypass;
    ss_linker_base *dstLinker;

private:
    stream_packet_obj dequeue(unsigned int delay_ms = 100) override
    {
        return nullptr;
    };
};

class ss_linker_anr : public ss_linker_audioalgo
{
public:
    struct AnrParm
    {
        bool         bEnable;
        unsigned int user_mode;
        unsigned int smooth_level;
        unsigned int speed;
    } stAnrParm;

public:
    explicit ss_linker_anr(Audio_Algorith_t &stAudioAlogo, AnrParm &stAnrCfg)
    {
        unsigned int     workingBufferSize;
        AudioProcessInit stInitParam;
        AudioAnrConfig   stAnrConfig;

        memset(&stInitParam, 0, sizeof(stInitParam));
        memset(&stAnrConfig, 0, sizeof(stAnrConfig));
        int intensity_band[6]    = {3, 24, 40, 64, 80, 128};
        int intensity[7]         = {30, 30, 30, 30, 30, 30, 30};
        stInitParam.point_number = stAudioAlogo.u32PeriodSize;

        if (E_MI_AUDIO_SOUND_MODE_MONO == stAudioAlogo.enSoundMode)
        {
            stInitParam.channel = 1;
        }
        else
        {
            stInitParam.channel = 2;
        }

        m_iPeriodSize = stInitParam.point_number * stInitParam.channel;

        switch (stAudioAlogo.enSampleRate)
        {
            case E_MI_AUDIO_SAMPLE_RATE_8000:
                stInitParam.sample_rate = IAA_APC_SAMPLE_RATE_8000;
                break;
            case E_MI_AUDIO_SAMPLE_RATE_16000:
                stInitParam.sample_rate = IAA_APC_SAMPLE_RATE_16000;
                break;
            case E_MI_AUDIO_SAMPLE_RATE_32000:
                stInitParam.sample_rate = IAA_APC_SAMPLE_RATE_32000;
                break;
            case E_MI_AUDIO_SAMPLE_RATE_48000:
                stInitParam.sample_rate = IAA_APC_SAMPLE_RATE_48000;
                break;
            default:
                AMIGOS_ERR("apc not support samplerate is: %d\n", stAudioAlogo.enSampleRate);
                return;
        }

        workingBufferSize = IaaAnr_GetBufferSize();
        m_workingBuffer   = new char[workingBufferSize];
        m_handle          = IaaAnr_Init((char *)m_workingBuffer, &stInitParam);

        if (nullptr == m_handle)
        {
            AMIGOS_ERR("ANR init error\n");
            return;
        }

        /******ANR Config*******/
        stAnrConfig.anr_enable      = stAnrCfg.bEnable;
        stAnrConfig.user_mode       = stAnrCfg.user_mode;
        stAnrConfig.anr_filter_mode = 4;
        memcpy(stAnrConfig.anr_intensity_band, intensity_band, 6 * sizeof(int));
        memcpy(stAnrConfig.anr_intensity, intensity, 7 * sizeof(int));
        stAnrConfig.anr_smooth_level   = stAnrCfg.smooth_level;
        stAnrConfig.anr_converge_speed = (NR_CONVERGE_SPEED)stAnrCfg.speed;

        int ret = IaaAnr_Config(m_handle, &stAnrConfig);
        if (ret)
        {
            IaaAnr_Free(m_handle);
            m_handle = NULL;
            AMIGOS_ERR("ANR Config Error!");
            return;
        }

        AMIGOS_INFO("ANR init succeed\n");
    };
    ~ss_linker_anr()
    {
        if (nullptr != m_handle)
        {
            IaaAnr_Free(m_handle);
            m_handle = nullptr;
        }

        if (nullptr != m_workingBuffer)
        {
            delete[] m_workingBuffer;
            m_workingBuffer = nullptr;
        }

        AMIGOS_INFO("ANR Uninit succeed\n");
    };
    int Run(stream_packet_obj &packet) override
    {
        if (nullptr == m_handle || 0 == packet->es_aud_i.packet_info[0].size
            || nullptr == packet->es_aud.packet_data[0].data)
        {
            AMIGOS_ERR("APC handle or data size is 0\n");
            return 0;
        }

        int iSet = 0, iLen = packet->es_aud_i.packet_info[0].size;
        int ret = 0;

        while (iLen - int(m_iPeriodSize * iSet * sizeof(short)) >= m_iPeriodSize)
        {
            ret = IaaAnr_Run(m_handle,
                             (short *)(packet->es_aud.packet_data[0].data + m_iPeriodSize * iSet * sizeof(short)));

            if (ret < 0)
            {
                AMIGOS_ERR("Error occured in ANR run function \n");
                return 0;
            }

            iSet++;
        }

        return 0;
    };

private:
};

class ss_linker_eq : public ss_linker_audioalgo
{
public:
    struct EqParm
    {
        bool         bEnable;
        unsigned int user_mode;
    };
    struct HpfParm
    {
        unsigned int user_mode;
        unsigned int CutoffFreq;
    };

public:
    explicit ss_linker_eq(Audio_Algorith_t &stAudioAlogo, EqParm &stEqCfg, HpfParm &stHpfCfg)
    {
        unsigned int     workingBufferSize;
        AudioProcessInit stInitParam;
        AudioEqConfig    stEqConfig;
        AudioHpfConfig   stHpfConfig;

        memset(&stInitParam, 0, sizeof(AudioProcessInit));
        memset(&stEqConfig, 0, sizeof(AudioEqConfig));
        memset(&stHpfConfig, 0, sizeof(AudioHpfConfig));
        short eq_table[129] = {1, 1};
        std::fill(eq_table + 2, eq_table + 127, -5);

        stInitParam.point_number = stAudioAlogo.u32PeriodSize;

        if (E_MI_AUDIO_SOUND_MODE_MONO == stAudioAlogo.enSoundMode)
        {
            stInitParam.channel = 1;
        }
        else
        {
            stInitParam.channel = 2;
        }

        m_iPeriodSize = stInitParam.point_number * stInitParam.channel;

        switch (stAudioAlogo.enSampleRate)
        {
            case E_MI_AUDIO_SAMPLE_RATE_8000:
                stInitParam.sample_rate = IAA_APC_SAMPLE_RATE_8000;
                break;
            case E_MI_AUDIO_SAMPLE_RATE_16000:
                stInitParam.sample_rate = IAA_APC_SAMPLE_RATE_16000;
                break;
            case E_MI_AUDIO_SAMPLE_RATE_32000:
                stInitParam.sample_rate = IAA_APC_SAMPLE_RATE_32000;
                break;
            case E_MI_AUDIO_SAMPLE_RATE_48000:
                stInitParam.sample_rate = IAA_APC_SAMPLE_RATE_48000;
                break;
            default:
                AMIGOS_ERR("apc not support samplerate is: %d\n", stAudioAlogo.enSampleRate);
                return;
        }

        workingBufferSize = IaaEq_GetBufferSize();
        m_workingBuffer   = new char[workingBufferSize];
        m_handle          = IaaEq_Init((char *)m_workingBuffer, &stInitParam);

        if (nullptr == m_handle)
        {
            AMIGOS_ERR("EQ init error\n");
            return;
        }

        /******EQ Config********/
        stEqConfig.eq_enable = stEqCfg.bEnable;
        stEqConfig.user_mode = stEqCfg.user_mode;
        memcpy(stEqConfig.eq_gain_db, eq_table, _EQ_BAND_NUM * sizeof(short));
        /******HPF Config********/
        stHpfConfig.hpf_enable       = stEqCfg.bEnable;
        stHpfConfig.user_mode        = stHpfCfg.user_mode;
        stHpfConfig.cutoff_frequency = (IAA_HPF_FREQ)stHpfCfg.CutoffFreq;

        int ret = IaaEq_Config(m_handle, &stHpfConfig, &stEqConfig);
        if (ret)
        {
            AMIGOS_ERR("eq & hpf Config Error!");
            return;
        }

        AMIGOS_INFO("eq & hpf init succeed\n");
    };
    ~ss_linker_eq()
    {
        if (nullptr != m_handle)
        {
            IaaEq_Free(m_handle);
            m_handle = nullptr;
        }

        if (nullptr != m_workingBuffer)
        {
            delete[] m_workingBuffer;
            m_workingBuffer = nullptr;
        }

        AMIGOS_INFO("eq Uninit succeed\n");
    };
    int Run(stream_packet_obj &packet) override
    {
        if (nullptr == m_handle || 0 == packet->es_aud_i.packet_info[0].size
            || nullptr == packet->es_aud.packet_data[0].data)
        {
            AMIGOS_ERR("EQ handle or data size is 0\n");
            return 0;
        }

        int iSet = 0, iLen = packet->es_aud_i.packet_info[0].size;
        int ret = 0;

        while (iLen - int(m_iPeriodSize * iSet * sizeof(short)) >= m_iPeriodSize)
        {
            ret = IaaEq_Run(m_handle,
                            (short *)(packet->es_aud.packet_data[0].data + m_iPeriodSize * iSet * sizeof(short)));

            if (ret < 0)
            {
                AMIGOS_ERR("Error occured in ANR run function \n");
                return 0;
            }

            iSet++;
        }

        return 0;
    };

private:
};

class ss_linker_agc : public ss_linker_audioalgo
{
public:
    struct AgcParm
    {
        bool         bEnable;
        unsigned int user_mode;
        unsigned int DropGainMax;
        unsigned int AttackTime;
        unsigned int ReleaseTime;
        int          DropGainThreshold;
    };

public:
    explicit ss_linker_agc(Audio_Algorith_t &stAudioAlogo, AgcParm &stAgcCfg)
    {
        unsigned int     workingBufferSize;
        AudioProcessInit stInitParam;
        AudioAgcConfig   stAgcConfig;
        memset(&stInitParam, 0, sizeof(stInitParam));
        memset(&stAgcConfig, 0, sizeof(stAgcConfig));
        short compression_ratio_input[_AGC_CR_NUM]         = {-65, -55, -48, -25, -18, -12, 0};
        short compression_ratio_output[_AGC_CR_NUM]        = {-65, -50, -27, -12, -1, -1, -1};
        int   freqBand[_AGC_FREQ_BAND_NUM]                 = {1000, 6000, 8000};
        int   compressionRatioArrayLowInput[_AGC_CR_NUM]   = {-65, -55, -48, -25, -18, -12, 0};
        int   compressionRatioArrayLowOutput[_AGC_CR_NUM]  = {-65, -50, -27, -12, -1, -1, -1};
        int   compressionRatioArrayMidInput[_AGC_CR_NUM]   = {-65, -55, -48, -25, -18, -12, 0};
        int   compressionRatioArrayMidOutput[_AGC_CR_NUM]  = {-65, -50, -27, -12, -1, -1, -1};
        int   compressionRatioArrayHighInput[_AGC_CR_NUM]  = {-65, -55, -48, -25, -18, -12, 0};
        int   compressionRatioArrayHighOutput[_AGC_CR_NUM] = {-65, -50, -27, -12, -1, -1, -1};
        stInitParam.point_number                           = stAudioAlogo.u32PeriodSize;

        if (E_MI_AUDIO_SOUND_MODE_MONO == stAudioAlogo.enSoundMode)
        {
            stInitParam.channel = 1;
        }
        else
        {
            stInitParam.channel = 2;
        }

        m_iPeriodSize = stInitParam.point_number * stInitParam.channel;

        switch (stAudioAlogo.enSampleRate)
        {
            case E_MI_AUDIO_SAMPLE_RATE_8000:
                stInitParam.sample_rate = IAA_APC_SAMPLE_RATE_8000;
                break;
            case E_MI_AUDIO_SAMPLE_RATE_16000:
                stInitParam.sample_rate = IAA_APC_SAMPLE_RATE_16000;
                break;
            case E_MI_AUDIO_SAMPLE_RATE_32000:
                stInitParam.sample_rate = IAA_APC_SAMPLE_RATE_32000;
                break;
            case E_MI_AUDIO_SAMPLE_RATE_48000:
                stInitParam.sample_rate = IAA_APC_SAMPLE_RATE_48000;
                break;
            default:
                AMIGOS_ERR("apc not support samplerate is: %d\n", stAudioAlogo.enSampleRate);
                return;
        }

        workingBufferSize = IaaAgc_GetBufferSize();
        m_workingBuffer   = new char[workingBufferSize];
        m_handle          = IaaAgc_Init((char *)m_workingBuffer, &stInitParam);

        if (nullptr == m_handle)
        {
            AMIGOS_ERR("AGC init error\n");
            return;
        }

        /******AGC Config********/
        stAgcConfig.agc_enable          = stAgcCfg.bEnable;
        stAgcConfig.user_mode           = stAgcCfg.user_mode;
        stAgcConfig.gain_info.gain_max  = 40;
        stAgcConfig.gain_info.gain_min  = -10;
        stAgcConfig.gain_info.gain_init = 12;
        stAgcConfig.gain_step           = 1;
        stAgcConfig.drop_gain_max       = stAgcCfg.DropGainMax;
        stAgcConfig.attack_time         = stAgcCfg.AttackTime;
        stAgcConfig.release_time        = stAgcCfg.ReleaseTime;
        stAgcConfig.noise_gate_db       = -80;
        memcpy(stAgcConfig.compression_ratio_input, compression_ratio_input, _AGC_CR_NUM * sizeof(short));
        memcpy(stAgcConfig.compression_ratio_output, compression_ratio_output, _AGC_CR_NUM * sizeof(short));
        stAgcConfig.noise_gate_attenuation_db = 0;
        stAgcConfig.drop_gain_threshold       = stAgcCfg.DropGainThreshold;

        int ret = IaaAgc_Config(m_handle, &stAgcConfig);
        if (ret)
        {
            IaaAgc_Free(m_handle);
            m_handle = nullptr;
            AMIGOS_ERR("Config Error!");
            return;
        }

        IaaAgc_SetAgcFreqBand(m_handle, freqBand);
        IaaAgc_SetLowFreqCompressionRatioCurve(m_handle, compressionRatioArrayLowInput, compressionRatioArrayLowOutput);
        IaaAgc_SetMidFreqCompressionRatioCurve(m_handle, compressionRatioArrayMidInput, compressionRatioArrayMidOutput);
        IaaAgc_SetHighFreqCompressionRatioCurve(m_handle, compressionRatioArrayHighInput,
                                                compressionRatioArrayHighOutput);
        AMIGOS_INFO("AGC init succeed\n");
    };
    ~ss_linker_agc()
    {
        if (nullptr != m_handle)
        {
            IaaAgc_Free(m_handle);
            m_handle = nullptr;
        }

        if (nullptr != m_workingBuffer)
        {
            delete[] m_workingBuffer;
            m_workingBuffer = nullptr;
        }

        AMIGOS_INFO("AGC Uninit succeed\n");
    };
    int Run(stream_packet_obj &packet) override
    {
        if (nullptr == m_handle || 0 == packet->es_aud_i.packet_info[0].size
            || nullptr == packet->es_aud.packet_data[0].data)
        {
            AMIGOS_ERR("AGC handle or data size is 0\n");
            return 0;
        }

        int iSet = 0, iLen = packet->es_aud_i.packet_info[0].size;
        int ret = 0;

        while (iLen - int(m_iPeriodSize * iSet * sizeof(short)) >= m_iPeriodSize)
        {
            ret = IaaAgc_Run(m_handle,
                             (short *)(packet->es_aud.packet_data[0].data + m_iPeriodSize * iSet * sizeof(short)));

            if (ret < 0)
            {
                AMIGOS_ERR("Error occured in ANR run function \n");
                return 0;
            }

            iSet++;
        }

        return 0;
    };

private:
};

class ss_linker_apc : public ss_linker_audioalgo
{
public:
    struct ApcParm
    {
        ss_linker_anr::AnrParm stAnrCfg;
        ss_linker_eq::EqParm   stEqCfg;
        ss_linker_eq::HpfParm  stHpfCfg;
        ss_linker_agc::AgcParm stAgcCfg;
    } stApcParm;

public:
    explicit ss_linker_apc(Audio_Algorith_t &stAudioAlogo, ApcParm &stApcCfg) : stApcLock(PTHREAD_MUTEX_INITIALIZER)
    {
        unsigned int         workingBufferSize;
        AudioApcBufferConfig stApcAttr;
        AudioProcessInit     stInitParam;
        AudioAnrConfig       stAnrConfig;
        AudioEqConfig        stEqConfig;
        AudioHpfConfig       stHpfConfig;
        AudioAgcConfig       stAgcConfig;

        memset(&stApcAttr, 0, sizeof(AudioApcBufferConfig));
        memset(&stInitParam, 0, sizeof(AudioProcessInit));
        memset(&stAnrConfig, 0, sizeof(AudioAnrConfig));
        memset(&stEqConfig, 0, sizeof(AudioEqConfig));
        memset(&stHpfConfig, 0, sizeof(AudioHpfConfig));
        memset(&stAgcConfig, 0, sizeof(AudioAgcConfig));

        int   intensity_band[6]                     = {3, 24, 40, 64, 80, 128};
        int   intensity[7]                          = {30, 30, 30, 30, 30, 30, 30};
        short compression_ratio_input[_AGC_CR_NUM]  = {-65, -55, -48, -25, -18, -12, 0};
        short compression_ratio_output[_AGC_CR_NUM] = {-65, -50, -40, -20, -10, -5, -3};
        short eq_table[129]                         = {1, 1};
        std::fill(eq_table + 2, eq_table + 127, -5);

        stApcAttr.anr_enable     = stApcCfg.stAnrCfg.bEnable;
        stApcAttr.eq_enable      = stApcCfg.stEqCfg.bEnable;
        stApcAttr.agc_enable     = stApcCfg.stAgcCfg.bEnable;
        stInitParam.point_number = stAudioAlogo.u32PeriodSize;

        if (E_MI_AUDIO_SOUND_MODE_MONO == stAudioAlogo.enSoundMode)
        {
            stInitParam.channel = 1;
        }
        else
        {
            stInitParam.channel = 2;
        }

        m_iPeriodSize = stInitParam.point_number * stInitParam.channel;

        switch (stAudioAlogo.enSampleRate)
        {
            case E_MI_AUDIO_SAMPLE_RATE_8000:
                stInitParam.sample_rate = IAA_APC_SAMPLE_RATE_8000;
                break;
            case E_MI_AUDIO_SAMPLE_RATE_16000:
                stInitParam.sample_rate = IAA_APC_SAMPLE_RATE_16000;
                break;
            case E_MI_AUDIO_SAMPLE_RATE_32000:
                stInitParam.sample_rate = IAA_APC_SAMPLE_RATE_32000;
                break;
            case E_MI_AUDIO_SAMPLE_RATE_48000:
                stInitParam.sample_rate = IAA_APC_SAMPLE_RATE_48000;
                break;
            default:
                AMIGOS_ERR("apc not support samplerate is: %d\n", stAudioAlogo.enSampleRate);
                return;
        }

        workingBufferSize = IaaApc_GetBufferSize(&stApcAttr);
        m_workingBuffer   = new char[workingBufferSize];
        m_handle          = IaaApc_Init(m_workingBuffer, &stInitParam, &stApcAttr);

        if (nullptr == m_handle)
        {
            AMIGOS_ERR("APC init error\n");
            return;
        }

        /******ANR Config*******/
        stAnrConfig.anr_enable      = stApcAttr.anr_enable;
        stAnrConfig.user_mode       = stApcCfg.stAnrCfg.user_mode;
        stAnrConfig.anr_filter_mode = 0;
        memcpy(stAnrConfig.anr_intensity_band, intensity_band, 6 * sizeof(int));
        memcpy(stAnrConfig.anr_intensity, intensity, 7 * sizeof(int));
        stAnrConfig.anr_smooth_level   = stApcCfg.stAnrCfg.smooth_level;
        stAnrConfig.anr_converge_speed = (NR_CONVERGE_SPEED)stApcCfg.stAnrCfg.speed;
        /******EQ Config********/
        stEqConfig.eq_enable = stApcAttr.eq_enable;
        stEqConfig.user_mode = stApcCfg.stEqCfg.user_mode;
        memcpy(stEqConfig.eq_gain_db, eq_table, _EQ_BAND_NUM * sizeof(short));
        /******HPF Config********/
        stHpfConfig.hpf_enable       = stApcAttr.eq_enable;
        stHpfConfig.user_mode        = stApcCfg.stEqCfg.user_mode;
        stHpfConfig.cutoff_frequency = (IAA_HPF_FREQ)stApcCfg.stHpfCfg.CutoffFreq;
        /******AGC Config********/
        stAgcConfig.agc_enable          = stApcAttr.agc_enable;
        stAgcConfig.user_mode           = stApcCfg.stAgcCfg.user_mode;
        stAgcConfig.gain_info.gain_max  = 60;
        stAgcConfig.gain_info.gain_min  = -5;
        stAgcConfig.gain_info.gain_init = 0;
        stAgcConfig.drop_gain_max       = stApcCfg.stAgcCfg.DropGainMax;
        stAgcConfig.gain_step           = 3;
        stAgcConfig.attack_time         = stApcCfg.stAgcCfg.AttackTime;
        stAgcConfig.release_time        = stApcCfg.stAgcCfg.ReleaseTime;
        stAgcConfig.noise_gate_db       = -50;
        memcpy(stAgcConfig.compression_ratio_input, compression_ratio_input, _AGC_CR_NUM * sizeof(short));
        memcpy(stAgcConfig.compression_ratio_output, compression_ratio_output, _AGC_CR_NUM * sizeof(short));
        stAgcConfig.noise_gate_attenuation_db = 0;
        stAgcConfig.drop_gain_threshold       = stApcCfg.stAgcCfg.DropGainThreshold;

        int freqBand[_AGC_FREQ_BAND_NUM]                 = {3000, 6000, 8000};
        int compressionRatioArrayLowInput[_AGC_CR_NUM]   = {-80, -60, -40, -20, 0, 0, 0};
        int compressionRatioArrayLowOutput[_AGC_CR_NUM]  = {-5, -5, -5, -5, -5, -5, -5};
        int compressionRatioArrayMidInput[_AGC_CR_NUM]   = {-80, -60, -40, -20, 0, 0, 0};
        int compressionRatioArrayMidOutput[_AGC_CR_NUM]  = {-80, -60, -40, -20, 0, 0, 0};
        int compressionRatioArrayHighInput[_AGC_CR_NUM]  = {-80, -60, -40, -20, 0, 0, 0};
        int compressionRatioArrayHighOutput[_AGC_CR_NUM] = {-80, -60, -40, -20, 0, 0, 0};

        int ret = IaaApc_Config(m_handle, &stAnrConfig, &stEqConfig, &stHpfConfig, NULL, NULL, &stAgcConfig);
        if (ret)
        {
            IaaApc_Free(m_handle);
            m_handle = nullptr;
            AMIGOS_ERR("apc Config Error!");
            return;
        }
        if (stAgcConfig.agc_enable && stAgcConfig.user_mode == 2)
        {
            IaaApc_SetAgcFreqBand(m_handle, freqBand);
            IaaApc_SetLowFreqCompressionRatioCurve(m_handle, compressionRatioArrayLowInput,
                                                   compressionRatioArrayLowOutput);
            IaaApc_SetMidFreqCompressionRatioCurve(m_handle, compressionRatioArrayMidInput,
                                                   compressionRatioArrayMidOutput);
            IaaApc_SetHighFreqCompressionRatioCurve(m_handle, compressionRatioArrayHighInput,
                                                    compressionRatioArrayHighOutput);
        }

        AMIGOS_INFO("APC init succeed\n");
    };
    ~ss_linker_apc()
    {
        if (nullptr != m_handle)
        {
            IaaApc_Free(m_handle);
            m_handle = nullptr;
        }

        if (nullptr != m_workingBuffer)
        {
            delete[] m_workingBuffer;
            m_workingBuffer = nullptr;
        }

        AMIGOS_INFO("APC Uninit succeed\n");
    };
    int Run(stream_packet_obj &packet) override
    {
        ss_auto_lock autoLock(stApcLock);
        if (nullptr == m_handle || 0 == packet->es_aud_i.packet_info[0].size
            || nullptr == packet->es_aud.packet_data[0].data)
        {
            AMIGOS_ERR("APC handle or data size is 0\n");
            return 0;
        }

        int iSet = 0, iLen = packet->es_aud_i.packet_info[0].size;
        int ret = 0;


        while (iLen - int(m_iPeriodSize * iSet * sizeof(short)) >= m_iPeriodSize)
        {
            ret = IaaApc_Run(
                m_handle, (short *)((char *)packet->es_aud.packet_data[0].data + m_iPeriodSize * iSet * sizeof(short)));

            if (ret < 0)
            {
                AMIGOS_ERR("Error occured in APC run function \n");
                return 0;
            }

            iSet++;
        }

        return 0;
    };
    int Reset(AudioProcessInit &audio_process_init, AudioApcBufferConfig &apc_switch)
    {
        ALGO_APC_RET ret;

        ss_auto_lock autoLock(stApcLock);
        if (nullptr == m_workingBuffer || nullptr == m_handle)
        {
            AMIGOS_ERR("AEC handle is nullptr, please cheack apc linker is create\n");
            return -1;
        }

        AudioProcessInit old_audio_process_init;
        AudioAnrConfig   anr_config;
        AudioEqConfig    eq_config;
        AudioHpfConfig   hpf_config;
        AudioAgcConfig   agc_config;
        ret = IaaApc_GetConfig(m_handle, &old_audio_process_init, (apc_switch.anr_enable ? &anr_config : nullptr),
                               (apc_switch.eq_enable ? (&eq_config) : (nullptr)),
                               (apc_switch.eq_enable ? (&hpf_config) : (nullptr)), nullptr, nullptr,
                               (apc_switch.agc_enable ? &agc_config : nullptr));

        if (ALGO_APC_RET_SUCCESS != ret)
        {
            AMIGOS_ERR("APC get Config error, err code 0x%x\n", ret);
            return -1;
        }

        m_handle = IaaApc_Reset(m_workingBuffer, &audio_process_init, &apc_switch);

        if (nullptr == m_handle)
        {
            AMIGOS_ERR("APC reset error\n");
            delete[] m_workingBuffer;
            m_workingBuffer = nullptr;
            return -1;
        }

        int   intensity_band[6]                     = {3, 24, 40, 64, 80, 128};
        int   intensity[7]                          = {30, 30, 30, 30, 30, 30, 30};
        short compression_ratio_input[_AGC_CR_NUM]  = {-65, -55, -48, -25, -18, -12, 0};
        short compression_ratio_output[_AGC_CR_NUM] = {-65, -50, -40, -20, -10, -5, -3};
        short eq_table[129];
        std::fill(eq_table, eq_table + 129, 20);

        /******ANR Config*******/
        anr_config.anr_enable      = apc_switch.anr_enable;
        anr_config.user_mode       = 1;
        anr_config.anr_filter_mode = 0;
        memcpy(anr_config.anr_intensity_band, intensity_band, 6 * sizeof(int));
        memcpy(anr_config.anr_intensity, intensity, 7 * sizeof(int));
        anr_config.anr_smooth_level   = 10;
        anr_config.anr_converge_speed = NR_SPEED_HIGH;
        /******EQ Config********/
        eq_config.eq_enable = apc_switch.eq_enable;
        eq_config.user_mode = 1;
        memcpy(eq_config.eq_gain_db, eq_table, _EQ_BAND_NUM * sizeof(short));
        /******HPF Config********/
        hpf_config.hpf_enable       = apc_switch.eq_enable;
        hpf_config.user_mode        = 1;
        hpf_config.cutoff_frequency = AUDIO_HPF_FREQ_150;
        /******AGC Config********/
        agc_config.agc_enable          = apc_switch.agc_enable;
        agc_config.user_mode           = 1;
        agc_config.gain_info.gain_max  = 60;
        agc_config.gain_info.gain_min  = -20;
        agc_config.gain_info.gain_init = 0;
        agc_config.drop_gain_max       = 36;
        agc_config.gain_step           = 3;
        agc_config.attack_time         = 1;
        agc_config.release_time        = 1;
        agc_config.noise_gate_db       = -50;
        memcpy(agc_config.compression_ratio_input, compression_ratio_input, _AGC_CR_NUM * sizeof(short));
        memcpy(agc_config.compression_ratio_output, compression_ratio_output, _AGC_CR_NUM * sizeof(short));
        agc_config.noise_gate_attenuation_db = 0;
        agc_config.drop_gain_threshold       = -5;

        int freqBand[_AGC_FREQ_BAND_NUM]                 = {3000, 6000, 8000};
        int compressionRatioArrayLowInput[_AGC_CR_NUM]   = {-80, -60, -40, -20, 0, 0, 0};
        int compressionRatioArrayLowOutput[_AGC_CR_NUM]  = {-5, -5, -5, -5, -5, -5, -5};
        int compressionRatioArrayMidInput[_AGC_CR_NUM]   = {-80, -60, -40, -20, 0, 0, 0};
        int compressionRatioArrayMidOutput[_AGC_CR_NUM]  = {-80, -60, -40, -20, 0, 0, 0};
        int compressionRatioArrayHighInput[_AGC_CR_NUM]  = {-80, -60, -40, -20, 0, 0, 0};
        int compressionRatioArrayHighOutput[_AGC_CR_NUM] = {-80, -60, -40, -20, 0, 0, 0};

        ret = IaaApc_Config(m_handle, &anr_config, &eq_config, &hpf_config, nullptr, nullptr, &agc_config);
        if (ALGO_APC_RET_SUCCESS != ret)
        {
            AMIGOS_ERR("APC set Config error, err code 0x%x\n", ret);
            return -1;
        }
        if (agc_config.agc_enable && agc_config.user_mode == 2)
        {
            IaaApc_SetAgcFreqBand(m_handle, freqBand);
            IaaApc_SetLowFreqCompressionRatioCurve(m_handle, compressionRatioArrayLowInput,
                                                   compressionRatioArrayLowOutput);
            IaaApc_SetMidFreqCompressionRatioCurve(m_handle, compressionRatioArrayMidInput,
                                                   compressionRatioArrayMidOutput);
            IaaApc_SetHighFreqCompressionRatioCurve(m_handle, compressionRatioArrayHighInput,
                                                    compressionRatioArrayHighOutput);
        }

        AMIGOS_INFO("APC reset sucessed!\n");
        return 0;
    };

private:
    pthread_mutex_t stApcLock;
};

class ss_linker_sed : public ss_linker_audioalgo
{
public:
    struct SedParm
    {
        bool        bEnable;
        std::string path;
        int         detect_mode;
        int         smooth_length;
        float       vad_threshold;
        float       event_threshold[SED_MAX_EVENT_NUM];
    };

public:
    explicit ss_linker_sed(Audio_Algorith_t &stAudioAlogo, SedParm &param)
    {
        AudioSedInit_t   sedInit;
        AudioSedConfig_t sedConfig;
        memset(&sedInit, 0, sizeof(sedInit));
        memset(&sedConfig, 0, sizeof(sedConfig));

        if (stAudioAlogo.enSampleRate != E_MI_AUDIO_SAMPLE_RATE_16000)
        {
            AMIGOS_ERR("Sed Only support 16k");
            return;
        }

        sedInit.sampleRate = 16000;
        sedInit.bitWidth   = 16;
        // ipuMaxSize set 0, IPU will set the size based on the model size
        sedInit.ipuMaxSize = 0;
        memcpy(sedInit.modelPath, param.path.c_str(), MIN(param.path.length(), SED_MAX_FILE_PATH_LEN));

        sedConfig.detectMode   = (AudioSedMode_e)param.detect_mode;
        sedConfig.vadThreshold = param.smooth_length;
        sedConfig.vadThreshold = param.vad_threshold;
        for (int i = 0; i < SED_MAX_EVENT_NUM; i++)
        {
            sedConfig.eventThreshold[i] = param.event_threshold[i];
        }
        m_handle = IaaSed_Init(&sedInit);

        if (nullptr == m_handle)
        {
            AMIGOS_ERR("SED init error\n");
            return;
        }

        if (IaaSed_SetConfig(m_handle, &sedConfig) != 0)
        {
            AMIGOS_ERR("Sed Config Error!");
            IaaSed_Free(m_handle);
            m_handle = nullptr;
            return;
        }
        AMIGOS_INFO("SED init success\n");
    };
    ~ss_linker_sed()
    {
        if (nullptr != m_handle)
        {
            IaaSed_Free(m_handle);
            m_handle = nullptr;
        }

        AMIGOS_INFO("SED Uninit success\n");
        return;
    };
    int Run(stream_packet_obj &packet) override
    {
        if (nullptr == m_handle || 0 == packet->es_aud_i.packet_info[0].size
            || nullptr == packet->es_aud.packet_data[0].data)
        {
            return -1;
        }

        int input_len   = 0;
        int detect_flag = 0;
        int event_idx   = 0;
        int iSet        = 0;
        int ret = 0, iLen = packet->es_aud_i.packet_info[0].size;
        IaaSed_GetInputSamples(m_handle, &input_len);
        while (iLen - input_len * iSet >= input_len)
        {
            ret =
                IaaSed_LoadData(m_handle, (short *)packet->es_aud.packet_data[0].data + input_len * iSet, &detect_flag);
            if (ret != 0)
            {
                AMIGOS_ERR("load data err, errcode: %d\n", ret);
            }
            if (detect_flag)
            {
                ret = IaaSed_Run(m_handle, &event_idx);
                if (ret != 0)
                {
                    AMIGOS_ERR("sed run err, errcode: %d\n", ret);
                }
                // -------------------------------------------------------------------
                //    mode       |    function        |    description               |
                //  sed_tbs.img  |     babycry        | 0:negaitve,1:babycry         |
                //  sed_tcs.img  |     cough          | 0:negaitve,1:cough           |
                //  sed_tcbs.img | babycry & cought   | 0:negaitve,1:cough,2:babycry |
                // -------------------------------------------------------------------
                if (event_idx == 2)
                {
                    AMIGOS_INFO("Detected babycry.....\n");
                }
            }
            iSet++;
        }
        return 0;
    };

private:
};

class ss_linker_src : public ss_linker_audioalgo
{
public:
    struct SrcParm
    {
        SrcFilterOrder        order;
        MI_AUDIO_SampleRate_e OutSampleRate;
    };

public:
    explicit ss_linker_src(Audio_Algorith_t &stAudioAlogo, SrcParm &stSrcCfg)
        : m_channel(0), m_OutSampleRate(E_MI_AUDIO_SAMPLE_RATE_8000), m_NumTimes(1)
    {
        unsigned int     workingBufferSize;
        SRCStructProcess stSrcAttr;
        memset(&stSrcAttr, 0, sizeof(SRCStructProcess));

        if (E_MI_AUDIO_SOUND_MODE_MONO == stAudioAlogo.enSoundMode)
        {
            stSrcAttr.channel = 1;
        }
        else
        {
            stSrcAttr.channel = 2;
        }

        stSrcAttr.order        = stSrcCfg.order;
        stSrcAttr.point_number = stAudioAlogo.u32PeriodSize;
        m_iPeriodSize          = stSrcAttr.point_number * stSrcAttr.channel * 2; // 2 is from bitwidth(16) / 8
        m_channel              = stSrcAttr.channel;
        m_OutSampleRate        = stSrcCfg.OutSampleRate;

        switch (stAudioAlogo.enSampleRate)
        {
            case E_MI_AUDIO_SAMPLE_RATE_8000:
                stSrcAttr.WaveIn_srate = SRATE_8K;

                if (E_MI_AUDIO_SAMPLE_RATE_16000 == stSrcCfg.OutSampleRate)
                {
                    stSrcAttr.mode = SRC_8k_to_16k;
                    m_NumTimes     = 2;
                }
                else if (E_MI_AUDIO_SAMPLE_RATE_32000 == stSrcCfg.OutSampleRate)
                {
                    stSrcAttr.mode = SRC_8k_to_32k;
                    m_NumTimes     = 4;
                }
                else if (E_MI_AUDIO_SAMPLE_RATE_48000 == stSrcCfg.OutSampleRate)
                {
                    stSrcAttr.mode = SRC_8k_to_48k;
                    m_NumTimes     = 6;
                }
                else
                {
                    AMIGOS_ERR("not support src samplerate, src samplerate: %d, dst: %d\n", stAudioAlogo.enSampleRate,
                               stSrcCfg.OutSampleRate);
                    return;
                }

                break;

            case E_MI_AUDIO_SAMPLE_RATE_16000:
                stSrcAttr.WaveIn_srate = SRATE_16K;

                if (E_MI_AUDIO_SAMPLE_RATE_8000 == stSrcCfg.OutSampleRate)
                {
                    stSrcAttr.mode = SRC_16k_to_8k;
                }
                else if (E_MI_AUDIO_SAMPLE_RATE_32000 == stSrcCfg.OutSampleRate)
                {
                    stSrcAttr.mode = SRC_16k_to_32k;
                    m_NumTimes     = 2;
                }
                else if (E_MI_AUDIO_SAMPLE_RATE_48000 == stSrcCfg.OutSampleRate)
                {
                    stSrcAttr.mode = SRC_16k_to_48k;
                    m_NumTimes     = 3;
                }
                else
                {
                    AMIGOS_ERR("not support src samplerate, src samplerate: %d, dst: %d\n", stAudioAlogo.enSampleRate,
                               stSrcCfg.OutSampleRate);
                    return;
                }

                break;

            case E_MI_AUDIO_SAMPLE_RATE_32000:
                stSrcAttr.WaveIn_srate = SRATE_32K;

                if (E_MI_AUDIO_SAMPLE_RATE_8000 == stSrcCfg.OutSampleRate)
                {
                    stSrcAttr.mode = SRC_32k_to_8k;
                }
                else if (E_MI_AUDIO_SAMPLE_RATE_16000 == stSrcCfg.OutSampleRate)
                {
                    stSrcAttr.mode = SRC_32k_to_16k;
                }
                else if (E_MI_AUDIO_SAMPLE_RATE_48000 == stSrcCfg.OutSampleRate)
                {
                    stSrcAttr.mode = SRC_32k_to_48k;
                    m_NumTimes     = 2;
                }
                else
                {
                    AMIGOS_ERR("not support src samplerate, src samplerate: %d, dst: %d\n", stAudioAlogo.enSampleRate,
                               stSrcCfg.OutSampleRate);
                    return;
                }

                break;

            case E_MI_AUDIO_SAMPLE_RATE_48000:
                stSrcAttr.WaveIn_srate = SRATE_48K;

                if (E_MI_AUDIO_SAMPLE_RATE_8000 == stSrcCfg.OutSampleRate)
                {
                    stSrcAttr.mode = SRC_48k_to_8k;
                }
                else if (E_MI_AUDIO_SAMPLE_RATE_16000 == stSrcCfg.OutSampleRate)
                {
                    stSrcAttr.mode = SRC_48k_to_16k;
                }
                else if (E_MI_AUDIO_SAMPLE_RATE_32000 == stSrcCfg.OutSampleRate)
                {
                    stSrcAttr.mode = SRC_48k_to_32k;
                }
                else
                {
                    AMIGOS_ERR("not support src samplerate, src samplerate: %d, dst: %d\n", stAudioAlogo.enSampleRate,
                               stSrcCfg.OutSampleRate);
                    return;
                }

                break;

            default:
                AMIGOS_ERR("aec only support 8k/16k, but ai type is: %d\n", stAudioAlogo.enSampleRate);
                return;
        }

        workingBufferSize = IaaSrc_GetBufferSize(stSrcAttr.mode);
        m_workingBuffer   = new char[workingBufferSize];
        m_handle          = IaaSrc_Init((char *)m_workingBuffer, &stSrcAttr);

        if (nullptr == m_handle)
        {
            AMIGOS_ERR("SRC init error\n");
            return;
        }

        AMIGOS_INFO("SRC init succeed\n");
    };

    ~ss_linker_src()
    {
        if (nullptr != m_handle)
        {
            IaaSrc_Release(m_handle);
            m_handle = nullptr;
        }

        if (nullptr != m_workingBuffer)
        {
            delete[] m_workingBuffer;
            m_workingBuffer = nullptr;
        }

        AMIGOS_INFO("SRC Uninit succeed\n");
    };
    int enqueue(stream_packet_obj &packet) override
    {
        stream_packet_info packet_info;
        packet_info.en_type  = EN_AUDIO_CODEC_DATA;
        packet_info.es_aud_i = packet->es_aud_i;
        packet_info.es_aud_i.packet_info[0].size *= m_NumTimes;
        stream_packet_obj expacket = stream_packet_base::make<stream_packet>(packet_info);
        assert(expacket);
        expacket->es_aud_i.packet_info[0].size = packet->es_aud_i.packet_info[0].size;
        if (!dstLinker)
        {
            AMIGOS_ERR("Dst linker is null!\n");
            return -1;
        }
        if (!m_bypass)
        {
            Run(packet, expacket);
            return dstLinker->enqueue(expacket);
        }
        return dstLinker->enqueue(packet);
    };
    int Run(stream_packet_obj &packet, stream_packet_obj &expacket) override
    {
        if (nullptr == m_handle || 0 == packet->es_aud_i.packet_info[0].size
            || nullptr == packet->es_aud.packet_data[0].data)
        {
            return -1;
        }

        int OutputSampleNum;
        int iSet = 0, iLen = packet->es_aud_i.packet_info[0].size;
        expacket->es_aud_i.packet_info[0].size = 0;
        ALGO_SRC_RET ret;

        while (iLen - (m_iPeriodSize * iSet) >= m_iPeriodSize)
        {
            ret = IaaSrc_Run(m_handle, (short *)(packet->es_aud.packet_data[0].data + m_iPeriodSize * iSet),
                             (short *)(expacket->es_aud.packet_data[0].data + expacket->es_aud_i.packet_info[0].size),
                             &OutputSampleNum);

            if (0 > ret)
            {
                AMIGOS_ERR("Error occured in Src run function, ret error: 0x%x\n", ret);
                return -1;
            }

            expacket->es_aud_i.packet_info[0].size += m_channel * OutputSampleNum * sizeof(short);
            iSet++;
        }

        expacket->es_aud_i.sample_rate = m_OutSampleRate;
        return 0;
    };

private:
    unsigned              m_channel;
    MI_AUDIO_SampleRate_e m_OutSampleRate;
    unsigned int          m_NumTimes;
};

#endif
