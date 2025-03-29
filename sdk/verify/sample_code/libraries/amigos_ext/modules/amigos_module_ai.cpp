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
#include <sys/select.h>
#include <string.h>
#include <cassert>
#include <memory>

#include "amigos_log.h"
#include "mi_ai_datatype.h"
#include "mi_sys.h"
#include "mi_ai.h"
#include "amigos_module_init.h"
#include "amigos_module_ai.h"
#include "amigos_module_aio.h"
#include "ss_enum_cast.hpp"
#include "ss_packet.h"
#include "ss_thread.h"

SS_ENUM_CAST_STR(MI_AI_If_e,
{
    {E_MI_AI_IF_NONE,        "NONE"},
    {E_MI_AI_IF_ADC_AB,      "ADC_AB"},
    {E_MI_AI_IF_ADC_CD,      "ADC_CD"},
    {E_MI_AI_IF_DMIC_A_01,   "DMIC_A_01"},
    {E_MI_AI_IF_DMIC_A_23,   "DMIC_A_23"},
    {E_MI_AI_IF_I2S_A_01,    "I2S_A_01"},
    {E_MI_AI_IF_I2S_A_23,    "I2S_A_23"},
    {E_MI_AI_IF_I2S_A_45,    "I2S_A_45"},
    {E_MI_AI_IF_I2S_A_67,    "I2S_A_67"},
    {E_MI_AI_IF_I2S_A_89,    "I2S_A_89"},
    {E_MI_AI_IF_I2S_A_ab,    "I2S_A_ab"},
    {E_MI_AI_IF_I2S_A_cd,    "I2S_A_cd"},
    {E_MI_AI_IF_I2S_A_ef,    "I2S_A_ef"},
    {E_MI_AI_IF_I2S_B_01,    "I2S_B_01"},
    {E_MI_AI_IF_I2S_B_23,    "I2S_B_23"},
    {E_MI_AI_IF_I2S_B_45,    "I2S_B_45"},
    {E_MI_AI_IF_I2S_B_67,    "I2S_B_67"},
    {E_MI_AI_IF_I2S_B_89,    "I2S_B_89"},
    {E_MI_AI_IF_I2S_B_ab,    "I2S_B_ab"},
    {E_MI_AI_IF_I2S_B_cd,    "I2S_B_cd"},
    {E_MI_AI_IF_I2S_B_ef,    "I2S_B_ef"},
    {E_MI_AI_IF_I2S_C_01,    "I2S_C_01"},
    {E_MI_AI_IF_I2S_C_23,    "I2S_C_23"},
    {E_MI_AI_IF_I2S_C_45,    "I2S_C_45"},
    {E_MI_AI_IF_I2S_C_67,    "I2S_C_67"},
    {E_MI_AI_IF_I2S_C_89,    "I2S_C_89"},
    {E_MI_AI_IF_I2S_C_ab,    "I2S_C_ab"},
    {E_MI_AI_IF_I2S_C_cd,    "I2S_C_cd"},
    {E_MI_AI_IF_I2S_C_ef,    "I2S_C_ef"},
    {E_MI_AI_IF_I2S_D_01,    "I2S_D_01"},
    {E_MI_AI_IF_I2S_D_23,    "I2S_D_23"},
    {E_MI_AI_IF_I2S_D_45,    "I2S_D_45"},
    {E_MI_AI_IF_I2S_D_67,    "I2S_D_67"},
    {E_MI_AI_IF_I2S_D_89,    "I2S_D_89"},
    {E_MI_AI_IF_I2S_D_ab,    "I2S_D_ab"},
    {E_MI_AI_IF_I2S_D_cd,    "I2S_D_cd"},
    {E_MI_AI_IF_I2S_D_ef,    "I2S_D_ef"},
    {E_MI_AI_IF_DMIC_A_45,   "DMIC_A_45"},
    {E_MI_AI_IF_ECHO_A,      "ECHO_A"},
    {E_MI_AI_IF_HDMI_A,      "HDMI_A"},
});

class AmigosAiStreamPacket : public stream_packet_base
{
public:
    class AiNoBuf: public err_buf
    {
        virtual void show() const override;
    };
public:
    AmigosAiStreamPacket(int fd, unsigned int ms, unsigned int devId, unsigned int chn_group,
                         bool bEcho, const es_audio_info &es_aud_i)
        : stream_packet_base(), dupPacket(nullptr)
    {
        struct timeval tv;
        int ret;

        this->devId = devId;
        this->chn_group = chn_group;
        memset(&stAiChFrame, 0, sizeof(MI_AI_Data_t));
        memset(&stEchoFrame, 0, sizeof(MI_AI_Data_t));

        FD_ZERO(&read_fds);
        FD_SET(fd, &read_fds);
        tv.tv_sec = 0;
        tv.tv_usec = ms * 1000;
        ret = select(fd + 1, &read_fds, NULL, NULL, &tv);
        if(ret < 0)
        {
            AMIGOS_ERR("select error!\n");
            throw AiNoBuf();
        }
        if(0 == ret)
        {
            throw AiNoBuf();
        }
        ret = MI_AI_Read(devId, chn_group, &stAiChFrame, &stEchoFrame, -1);
        if(ret == MI_AI_ERR_NOBUF)
        {
            AMIGOS_ERR("dev%d, chn_group%d no buffer warning!\n", devId, chn_group);
            throw AiNoBuf();
        }
        else if(ret != MI_SUCCESS)
        {
            AMIGOS_ERR("dev%d, chn_group%d get frame error!\n", devId, chn_group);
            throw AiNoBuf();
        }

        this->en_type               = EN_AUDIO_CODEC_DATA;
        this->es_aud_i              = es_aud_i;
        this->es_aud_i.packet_count = 1;

        this->es_aud.packet_data[0].data   = (char *)stAiChFrame.apvBuffer[0];
        this->es_aud_i.packet_info[0].size = stAiChFrame.u32Byte[0];

        if (bEcho)
        {
            this->es_aud.packet_data[1].data   = (char *)stEchoFrame.apvBuffer[0];
            this->es_aud_i.packet_info[1].size = stEchoFrame.u32Byte[0];
        }
    }
    virtual ~AmigosAiStreamPacket()
    {
        if (!this->dupPacket)
        {
            MI_AI_ReleaseData(devId, chn_group, &stAiChFrame, &stEchoFrame);
        }
    }
    stream_packet_obj dup() override
    {
        if (this->dupPacket)
        {
            return nullptr;
        }
        this->dupPacket = stream_packet_base::make<stream_packet_clone>(*this);
        assert(this->dupPacket);
        this->en_type  = this->dupPacket->en_type;
        this->es_aud_i = this->dupPacket->es_aud_i;
        this->es_aud   = this->dupPacket->es_aud;

        MI_AI_ReleaseData(devId, chn_group, &stAiChFrame, &stEchoFrame);
        return nullptr;
    }
private:
    void update_time_stamp() final
    {
        struct timeval stamp;
        stamp.tv_sec  = stAiChFrame.u64Pts / 1000000;
        stamp.tv_usec = stAiChFrame.u64Pts % 1000000;
        this->set_time_stamp(stamp);
    }
    stream_packet_obj dupPacket;
    unsigned int devId;
    unsigned int chn_group;
    MI_AI_Data_t stAiChFrame;
    MI_AI_Data_t stEchoFrame;
    fd_set read_fds;
};
void AmigosAiStreamPacket::AiNoBuf::show() const
{
}
AmigosModuleAi::AmigosModuleAi(const std::string &strInSection)
    : AmigosSurfaceAi(strInSection), AmigosModuleMiBase(this), bEcho(false)
{
    for (int i = 0; i < MI_AI_MAX_CHN_NUM / 2; i++)
    {
        enAiIf[i] = E_MI_AI_IF_NONE;
    }
    memset(&packet_info, 0, sizeof(es_audio_info));
}
AmigosModuleAi::~AmigosModuleAi()
{
}
unsigned int AmigosModuleAi::GetModId() const
{
    return E_MI_MODULE_ID_AI;
}
unsigned int AmigosModuleAi::GetInputType(unsigned int port) const
{
    return 0;
}
unsigned int AmigosModuleAi::GetOutputType(unsigned int port) const
{
    return E_MOD_PORT_TYPE_USER;
}
unsigned int AmigosModuleAi::GetAiSoundMod() const
{
    return ss_enum_cast<MI_AUDIO_SoundMode_e>::from_str(this->stAiInfo.strSoundMode);
}
MI_AI_If_e AmigosModuleAi::GetAiIf(unsigned int index)
{
    return enAiIf[index];
}
void AmigosModuleAi::_ResourceInit()
{
    MI_AI_DupChnGroup(this->stModInfo.devId, 0);

    int countIF = 0;
    for (auto &iter : this->stAiInfo.mapInterface)
    {
        if (iter.second)
        {
            if (countIF >= MI_AI_MAX_CHN_NUM / 2)
            {
                break;
            }
            std::cout << iter.first << std::endl;
            enAiIf[countIF] = ss_enum_cast<MI_AI_If_e>::from_str(iter.first);
            // if echo is deteced, if rtos support aec, bEcho should be true for packet
            if (E_MI_AI_IF_ECHO_A == enAiIf[countIF])
            {
                bEcho = true;
            }
            countIF++;
        }
    }
}
void AmigosModuleAi::_ResourceDeinit()
{
    MI_AI_DisableChnGroup(this->stModInfo.devId, 0);
}
void AmigosModuleAi::_Init()
{
    MI_AI_Attr_t stAiSetAttr;
    unsigned char u8ChnGrpId = 0;

    //set ai attr
    memset(&stAiSetAttr, 0, sizeof(stAiSetAttr));
    stAiSetAttr.enFormat = ss_enum_cast<MI_AUDIO_Format_e>::from_str(this->stAiInfo.strFormat);
    stAiSetAttr.enSampleRate = static_cast<MI_AUDIO_SampleRate_e>(this->stAiInfo.uintSampleRate);
    stAiSetAttr.enSoundMode = ss_enum_cast<MI_AUDIO_SoundMode_e>::from_str(this->stAiInfo.strSoundMode);
    stAiSetAttr.u32PeriodSize = this->stAiInfo.uintPeriodSize;
    stAiSetAttr.bInterleaved = this->stAiInfo.intInterleaved;

    /*
     * ATTENTION:this section must be before preload skip, bEcho should be set first
     */
    int countIF = 0;
    for (auto &iter : this->stAiInfo.mapInterface)
    {
        if (iter.second)
        {
            if (countIF >= MI_AI_MAX_CHN_NUM / 2)
            {
                break;
            }
            enAiIf[countIF] = ss_enum_cast<MI_AI_If_e>::from_str(iter.first);
            // if echo is deteced, if rtos support aec, bEcho should be true for packet
            if (E_MI_AI_IF_ECHO_A == enAiIf[countIF])
            {
                bEcho = true;
            }
            countIF++;
        }
    }

    if (MI_SUCCESS != MI_AI_Open((MI_AUDIO_DEV) this->stModInfo.devId, &stAiSetAttr))
    {
        AMIGOS_ERR("AI dev%d create fail\n", this->stModInfo.devId);
        return;
    }

    for (int i = 0; i < countIF; i++)
    {
        if (enAiIf[i] > E_MI_AI_IF_DMIC_A_23 && enAiIf[i] < E_MI_AI_IF_ECHO_A)
        {
            MI_AUDIO_I2sConfig_t stAiI2sACfg;
            memset(&stAiI2sACfg, 0, sizeof(stAiI2sACfg));
            stAiI2sACfg.enMode = ss_enum_cast<MI_AUDIO_I2sMode_e>::from_str(this->stAiInfo.stAiI2sInfo.Mode);
            stAiI2sACfg.enFormat = ss_enum_cast<MI_AUDIO_I2sFormat_e>::from_str(this->stAiInfo.stAiI2sInfo.Format);
            stAiI2sACfg.enSampleRate = static_cast<MI_AUDIO_SampleRate_e>(this->stAiInfo.uintSampleRate);
            stAiI2sACfg.enMclk = ss_enum_cast<MI_AUDIO_I2sMclk_e>::from_str(this->stAiInfo.stAiI2sInfo.Mclk);
            stAiI2sACfg.bSyncClock = this->stAiInfo.stAiI2sInfo.SyncClock;
            stAiI2sACfg.u32TdmSlots = this->stAiInfo.stAiI2sInfo.TdmSlots;
            stAiI2sACfg.enBitWidth = ss_enum_cast<MI_AUDIO_I2sBitWidth_e>::from_str(this->stAiInfo.stAiI2sInfo.BitWidth);
            if(MI_SUCCESS != MI_AI_SetI2SConfig(enAiIf[i], &stAiI2sACfg))
            {
                AMIGOS_ERR("AI I2s set fail\n");
                goto out;
            }
            AMIGOS_INFO("Interface %d AI set I2S suecess!\n", enAiIf[i]);
        }
    }
    if(MI_SUCCESS != MI_AI_AttachIf((MI_AUDIO_DEV)this->stModInfo.devId, enAiIf, countIF))
    {
        AMIGOS_ERR("AI attach interface fail\n");
        goto out;
    }
    if(MI_SUCCESS != MI_AI_EnableChnGroup((MI_AUDIO_DEV) this->stModInfo.devId, u8ChnGrpId))
    {
        AMIGOS_ERR("AI Enable chn group%d fail\n", u8ChnGrpId);
        goto out;
    }

    for(int i = 0; i < countIF; i++)
    {
        if(enAiIf[i] > E_MI_AI_IF_DMIC_A_23)
        {
            continue;
        }
        if(MI_SUCCESS != MI_AI_SetIfGain(enAiIf[i], this->stAiInfo.intVolume, this->stAiInfo.intVolume))
        {
            AMIGOS_ERR("AI interface %d Set Gain fail\n", enAiIf[i]);
            goto out;
        }
    }
    // echo sholud be set echo gain, default 0
    if (bEcho)
    {
        signed short gain[2] = {0};
        if (MI_SUCCESS != MI_AI_SetGain((MI_AUDIO_DEV) this->stModInfo.devId, MI_AI_ECHO_CHN_GROUP_ID, gain, 2))
        {
            AMIGOS_ERR("echo gain set fail\n");
        }
    }

    return;

out:
    MI_AI_Close((MI_AUDIO_DEV) this->stModInfo.devId);
    return;
}
void AmigosModuleAi::_Deinit()
{
    if (MI_SUCCESS != MI_AI_DisableChnGroup(this->stModInfo.devId, 0))
    {
        AMIGOS_ERR("Disable Chn Group Err\n");
        return;
    }
    if (MI_SUCCESS != MI_AI_Close((MI_AUDIO_DEV) this->stModInfo.devId))
    {
        AMIGOS_ERR("Close AI Err\n");
    }
}
int AmigosModuleAi::_Connected(unsigned int outPortId, unsigned int ref)
{
    if(ref > 0)
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
    AmigosSurfaceBase::ModPortOutInfo surfaceOutInfo;
    if (!this->GetSurface()->GetPortOutInfo(outPortId, surfaceOutInfo))
    {
        AMIGOS_ERR("GetPortOutInfo failed\n");
        return -1;
    }
    MI_SYS_ChnPort_t stChnPort;
    const AmigosSurfaceBase::ModInfo &stModInfo = this->GetSurface()->GetModInfo();
    memset(&stChnPort, 0, sizeof(MI_SYS_ChnPort_t));
    stChnPort.eModId    = E_MI_MODULE_ID_AI;
    stChnPort.u32DevId  = stModInfo.devId;
    stChnPort.u32ChnId  = stModInfo.chnId;
    stChnPort.u32PortId = outPortId;
    // It go user bind.
    if (surfaceOutInfo.bUserFrc)
    {
        MI_SYS_SetChnOutputPortUserFrc(&stChnPort, -1, surfaceOutInfo.curFrmRate);
    }
    if (!surfaceOutInfo.bUseDepth)
    {
        // Set default user depth if output is not direct bind.
        if (MI_SUCCESS != MI_SYS_SetChnOutputPortDepth(0, &stChnPort, 3, 4))
        {
            AMIGOS_ERR("MI_SYS_SetChnOutputPortDepth failed\n");
        }
    }
    packet_info.fmt            = ES_STREAM_WAV;
    packet_info.sample_rate    = this->stAiInfo.uintSampleRate;
    packet_info.channels       = ss_enum_cast<MI_AUDIO_SoundMode_e>::from_str(this->stAiInfo.strSoundMode);
    int ret, fd;
    MI_SYS_ChnPort_t stChnOutputPort;
    stChnOutputPort.eModId    = E_MI_MODULE_ID_AI;
    stChnOutputPort.u32DevId  = this->stModInfo.devId;
    stChnOutputPort.u32ChnId  = this->stModInfo.chnId;
    stChnOutputPort.u32PortId = 0;
    ret = MI_SYS_GetFd(&stChnOutputPort, &fd);
    if(ret < 0)
    {
        AMIGOS_ERR("dev%d, chn%d get fd error!\n", this->stModInfo.devId, this->stModInfo.chnId);
        return -1;
    }
    this->readerDesc.linker    = &iter->second.positive;
    this->readerDesc.outPortId = outPortId;
    this->readerDesc.pModule   = this;
    this->readerDesc.fd        = fd;
    if (iter->second.positive.empty())
    {
        AMIGOS_INFO("No reader\n");
        return 0;
    }
    ss_thread_attr ss_attr;
    memset(&ss_attr, 0, sizeof(ss_thread_attr));
    ss_attr.do_signal          = NULL;
    ss_attr.do_monitor         = AiReader;
    ss_attr.monitor_cycle_sec  = 0;
    ss_attr.monitor_cycle_nsec = 0;
    ss_attr.is_reset_timer     = 0;
    ss_attr.in_buf.buf         = (&this->readerDesc);
    ss_attr.in_buf.size        = 0;
    snprintf(ss_attr.thread_name, 128, "%s", this->GetOutPortIdStr(outPortId).c_str());
    this->readerDesc.threadHandle = ss_thread_open(&ss_attr);
    if(!this->readerDesc.threadHandle)
    {
        struct AiReaderDesc tmp;
        std::swap(this->readerDesc, tmp);
        AMIGOS_ERR("Monitor return error!\n");
        MI_SYS_CloseFd(fd);
        return -1;
    }
    ss_thread_start_monitor(this->readerDesc.threadHandle);
    AMIGOS_INFO("Ai reader %p started\n", this->readerDesc.threadHandle);
    return 0;
}
int AmigosModuleAi::_Disconnected(unsigned int outPortId, unsigned int ref)
{
    if(ref > 0)
    {
        return 0;
    }
    if(outPortId != 0)
    {
        return -1;
    }
    if(this->readerDesc.linker->empty())
    {
        return 0;
    }
    if (!this->readerDesc.threadHandle)
    {
        AMIGOS_ERR("Reader %d handle is null!\n", outPortId);
        return -1;
    }
    ss_thread_stop(this->readerDesc.threadHandle);
    ss_thread_close(this->readerDesc.threadHandle);
    this->readerDesc.linker       = nullptr;
    this->readerDesc.threadHandle = nullptr;
    MI_SYS_CloseFd(this->readerDesc.fd);
    this->readerDesc.fd = -1;
    AMIGOS_INFO("Ai reader %p stopped\n", this->readerDesc.threadHandle);
    return 0;
}
int AmigosModuleAi::_EnqueueOut(unsigned int outPortId, stream_packet_obj &packet)
{
    // To do... audio enqueue from outside, mi sys's api maybe support.
    return 0;
}
int AmigosModuleAi::_DequeueOut(unsigned int outPortId, stream_packet_obj &packet)
{
    // To do... audio dequeue from outside, mi sys's api maybe support.
    return 0;
}
stream_packet_obj AmigosModuleAi::_DequeueFromInside(unsigned int outPortId, unsigned int ms)
{
    return stream_packet_base::make<AmigosAiStreamPacket>(this->readerDesc.fd, ms, this->stModInfo.devId,
                                                          this->stModInfo.chnId, this->bEcho,
                                                          this->packet_info);
}
void *AmigosModuleAi::AiReader(struct ss_thread_buffer *thread_buf)
{
    AmigosModuleAi::AiReaderDesc *desc = (AmigosModuleAi::AiReaderDesc *) thread_buf->buf;
    stream_packet_obj packet = stream_packet_base::make<AmigosAiStreamPacket>(desc->fd, 10, desc->pModule->stModInfo.devId,
                                                                              desc->pModule->stModInfo.chnId, desc->pModule->bEcho,
                                                                              desc->pModule->packet_info);
    if (!packet)
    {
        return NULL;
    }
    desc->linker->enqueue(packet);
    return NULL;
}

stream_packet_info AmigosModuleAi::_GetStreamInfo(unsigned int outPortId)
{
    stream_packet_info streamInfo;
    streamInfo.en_type               = EN_AUDIO_CODEC_DATA;
    streamInfo.es_aud_i.fmt          = ES_STREAM_PCM;
    streamInfo.es_aud_i.channels     = ss_enum_cast<MI_AUDIO_SoundMode_e>::from_str(this->stAiInfo.strSoundMode);
    streamInfo.es_aud_i.sample_rate  = this->stAiInfo.uintSampleRate;
    streamInfo.es_aud_i.sample_width = 16;
    return streamInfo;
}
#ifdef CONFIG_MI_SYSCALL_L2R_DESTROY_SUPPORT
bool AmigosModuleAi::_NeedMarkDeinitOnRtos()
{
    return true;
}
#endif

AMIGOS_MODULE_INIT("AI", AmigosModuleAi);
