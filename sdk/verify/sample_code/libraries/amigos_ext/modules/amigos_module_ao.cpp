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
#include <unistd.h>
#include <iostream>

#include "amigos_module_init.h"
#include "amigos_module_ao.h"
#include "amigos_module_aio.h"
#include "mi_ao_datatype.h"
#include "mi_sys.h"
#include "mi_ao.h"
#include "amigos_module_init.h"
#include "ss_linker.h"
#include "ss_packet.h"
#include "ss_thread.h"

SS_ENUM_CAST_STR(MI_AO_If_e,
{
    {E_MI_AO_IF_NONE,        "NONE"},
    {E_MI_AO_IF_DAC_AB,      "DAC_AB"},
    {E_MI_AO_IF_DAC_CD,      "DAC_CD"},
    {E_MI_AO_IF_I2S_A,       "I2S_A"},
    {E_MI_AO_IF_I2S_B,       "I2S_B"},
    {E_MI_AO_IF_ECHO_A,      "ECHO_A"},
    {E_MI_AO_IF_HDMI_A,      "HDMI_A"},
});
SS_ENUM_CAST_STR(MI_AO_ChannelMode_e,
{
    {E_MI_AO_CHANNEL_MODE_STEREO,          "STEREO"},
    {E_MI_AO_CHANNEL_MODE_DOUBLE_MONO,     "DOUBLE_MONO"},
    {E_MI_AO_CHANNEL_MODE_DOUBLE_LEFT,     "DOUBLE_LEFT"},
    {E_MI_AO_CHANNEL_MODE_DOUBLE_RIGHT,    "DOUBLE_RIGHT"},
    {E_MI_AO_CHANNEL_MODE_EXCHANGE,        "EXCHANGE"},
    {E_MI_AO_CHANNEL_MODE_ONLY_LEFT,       "ONLY_LEFT"},
    {E_MI_AO_CHANNEL_MODE_ONLY_RIGHT,      "ONLY_RIGHT"},
});

void *ReceiverMonitor(struct ss_thread_buffer *thread_buf);

AmigosModuleAo::AmigosModuleAo(const std::string &strInSection)
    : AmigosSurfaceAo(strInSection), AmigosModuleMiBase(this), enAoIf(E_MI_AO_IF_NONE)
{
}
AmigosModuleAo::~AmigosModuleAo()
{
}
unsigned int AmigosModuleAo::GetModId() const
{
    return E_MI_MODULE_ID_AO;
}
unsigned int AmigosModuleAo::GetInputType(unsigned int port) const
{
    return E_MOD_PORT_TYPE_USER;
}
unsigned int AmigosModuleAo::GetOutputType(unsigned int port) const
{
    return 0;
}

void AmigosModuleAo::_ResourceInit()
{
    MI_AO_Dup(this->stModInfo.devId);
}
void AmigosModuleAo::_ResourceDeinit()
{
    MI_AO_Close(this->stModInfo.devId);
}

void AmigosModuleAo::_Init()
{
    MI_AO_Attr_t stAoSetAttr;

    memset(&stAoSetAttr, 0x0, sizeof(MI_AO_Attr_t));
    stAoSetAttr.enFormat = ss_enum_cast<MI_AUDIO_Format_e>::from_str(this->stAoInfo.strFormat);
    stAoSetAttr.enSoundMode = ss_enum_cast<MI_AUDIO_SoundMode_e>::from_str(this->stAoInfo.strSoundMode);
    stAoSetAttr.enSampleRate = static_cast<MI_AUDIO_SampleRate_e>(this->stAoInfo.uintSampleRate);
    stAoSetAttr.enChannelMode = ss_enum_cast<MI_AO_ChannelMode_e>::from_str(this->stAoInfo.strChannelMode);
    stAoSetAttr.u32PeriodSize = this->stAoInfo.uintPeriodSize;
    if (MI_SUCCESS != MI_AO_Open((MI_AUDIO_DEV) this->stModInfo.devId, &stAoSetAttr))
    {
        AMIGOS_ERR("AO dev%d create fail\n", this->stModInfo.devId);
        return;
    }

    int intAoIf = 0;
    for (auto &iter : this->stAoInfo.mapInterface)
    {
        if (iter.second)
        {
            intAoIf |= ss_enum_cast<MI_AO_If_e>::from_str(iter.first);
        }
    }
    enAoIf = static_cast<MI_AO_If_e>(intAoIf);

    if((enAoIf & E_MI_AO_IF_I2S_A) || (enAoIf & E_MI_AO_IF_I2S_B))
    {
        MI_AUDIO_I2sConfig_t stAoI2sACfg;
        memset(&stAoI2sACfg, 0x0, sizeof(stAoI2sACfg));
        stAoI2sACfg.enMode = ss_enum_cast<MI_AUDIO_I2sMode_e>::from_str(this->stAoInfo.stAoI2sInfo.Mode);
        stAoI2sACfg.enFormat = ss_enum_cast<MI_AUDIO_I2sFormat_e>::from_str(this->stAoInfo.stAoI2sInfo.Format);
        stAoI2sACfg.enSampleRate = static_cast<MI_AUDIO_SampleRate_e>(this->stAoInfo.uintSampleRate);
        stAoI2sACfg.enMclk = ss_enum_cast<MI_AUDIO_I2sMclk_e>::from_str(this->stAoInfo.stAoI2sInfo.Mclk);
        stAoI2sACfg.bSyncClock = this->stAoInfo.stAoI2sInfo.SyncClock;
        stAoI2sACfg.u32TdmSlots = this->stAoInfo.stAoI2sInfo.TdmSlots;
        stAoI2sACfg.enBitWidth = ss_enum_cast<MI_AUDIO_I2sBitWidth_e>::from_str(this->stAoInfo.stAoI2sInfo.BitWidth);
        if(enAoIf & E_MI_AO_IF_I2S_A)
        {
            if(MI_SUCCESS != MI_AO_SetI2SConfig(E_MI_AO_IF_I2S_A, &stAoI2sACfg))
            {
                AMIGOS_ERR("AO I2s set fail\n");
                goto out;
            }
        }
        if(enAoIf & E_MI_AO_IF_I2S_B)
        {
            if(MI_SUCCESS != MI_AO_SetI2SConfig(E_MI_AO_IF_I2S_B, &stAoI2sACfg))
            {
                AMIGOS_ERR("AO I2s set fail\n");
                goto out;
            }
        }
        AMIGOS_INFO("Interface 0x%x AO set I2S suecess!\n", enAoIf);
    }
    if(MI_SUCCESS != MI_AO_AttachIf((MI_AUDIO_DEV) this->stModInfo.devId, enAoIf, 0))
    {
        AMIGOS_ERR("AO dev%d attach interface fail\n", this->stModInfo.devId);
        goto out;
    }

    return;

out:
    MI_AO_Close((MI_AUDIO_DEV) this->stModInfo.devId);
    return;
}
void AmigosModuleAo::_Deinit()
{
    MI_AO_DetachIf((MI_AUDIO_DEV) this->stModInfo.devId, enAoIf);
    MI_AO_Close((MI_AUDIO_DEV) this->stModInfo.devId);
}
int AmigosModuleAo::_InConnect(unsigned int inPortId)
{
    if(inPortId != 0)
    {
        AMIGOS_ERR("AO port%d error", inPortId);
        return -1;
    }

    if(!stAoInfo.bSyncMode)
    {
        CreateReceiver(inPortId);
    }
    return 0;
}
int AmigosModuleAo::_InDisconnect(unsigned int inPortId)
{
    if(inPortId != 0)
    {
        AMIGOS_ERR("AO port%d error", inPortId);
        return -1;
    }

    if(!stAoInfo.bSyncMode)
    {
        DestoryReceiver(inPortId);
    }
    return 0;
}
int AmigosModuleAo::_DataReceiver(unsigned int inPortId, stream_packet_obj &packet)
{
    void *pvBuffer = NULL;
    unsigned int byte = 0;
    int send_retry = 0;
    int ret;

    if(packet->en_type != EN_AUDIO_CODEC_DATA)
    {
        return NULL;
    }

    for(unsigned int i = 0; i < packet->es_aud_i.packet_count; i++)
    {
        // ao dma size is 65536, so if buffer data oversize, drop it!
        if (packet->es_aud_i.packet_info[i].size > 65535)
        {
            AMIGOS_WRN("packet size over 65535\n");
            return 0;
        }
        pvBuffer = (void *) packet->es_aud.packet_data[i].data;
        byte = packet->es_aud_i.packet_info[i].size;
        send_retry = 20;

        do
        {
            ret = MI_AO_Write((MI_AUDIO_DEV) this->stModInfo.devId, pvBuffer, byte, 0, 20);

            if(MI_SUCCESS == ret)
            {
                break;
            }

            send_retry--;
        }
        while(send_retry);
    }

    return 0;
}
stream_packer *AmigosModuleAo::_CreateInputStreamPacker(unsigned int inPortId, bool &bFast)
{
    return AmigosModuleBase::_CreateInputStreamPacker(inPortId, bFast);
}
ss_linker_base *AmigosModuleAo::_CreateInputNegativeLinker(unsigned int inPortId)
{
    if(stAoInfo.bSyncMode)
    {
        return new LinkerSyncNegative(inPortId, this);
    }
    return new LinkerAsyncNegative(false, 32);
}

void AmigosModuleAo::CreateReceiver(unsigned int inPortId)
{
    auto iter = this->mapPortIn.find(inPortId);

    if(iter == this->mapPortIn.end())
    {
        AMIGOS_ERR("Base port %d is not found\n", inPortId);
        return;
    }

    this->writerDesc.devId  = this->stModInfo.devId;
    this->writerDesc.linker = dynamic_cast<LinkerAsyncNegative *>(iter->second.negative);
    assert(this->writerDesc.linker);

    ss_thread_attr ss_attr;
    memset(&ss_attr, 0, sizeof(ss_thread_attr));
    ss_attr.do_signal          = NULL;
    ss_attr.do_monitor         = ReceiverMonitor;
    ss_attr.monitor_cycle_sec  = 0;
    ss_attr.monitor_cycle_nsec = 0;
    ss_attr.is_reset_timer     = 0;
    ss_attr.in_buf.buf         = (&this->writerDesc);
    ss_attr.in_buf.size        = 0;
    snprintf(ss_attr.thread_name, 128, "%s", this->GetInPortIdStr(inPortId).c_str());
    this->writerDesc.threadHandle = ss_thread_open(&ss_attr);
    if(!this->writerDesc.threadHandle)
    {
        struct AoWriterDesc tmp;
        std::swap(this->writerDesc, tmp);
        AMIGOS_ERR("Monitor return error!\n");
        return;
    }
    ss_thread_start_monitor(this->writerDesc.threadHandle);
    AMIGOS_INFO("Ao writer %d start\n", inPortId);
}
void AmigosModuleAo::DestoryReceiver(unsigned int inPortId)
{
    if(!this->writerDesc.linker && 0 != inPortId)
    {
        return;
    }
    ss_thread_stop(this->writerDesc.threadHandle);
    ss_thread_close(this->writerDesc.threadHandle);
    this->writerDesc.threadHandle = nullptr;
    this->writerDesc.linker = nullptr;
}
#ifdef CONFIG_MI_SYSCALL_L2R_DESTROY_SUPPORT
bool AmigosModuleAo::_NeedMarkDeinitOnRtos()
{
    return true;
}
#endif

void *ReceiverMonitor(struct ss_thread_buffer *thread_buf)
{
    AmigosModuleAo::AoWriterDesc *desc = (AmigosModuleAo::AoWriterDesc *) thread_buf->buf;
    void *pvBuffer = NULL;
    unsigned int byte = 0;
    int send_retry = 0;
    int ret;

    auto packet = desc->linker->WaitPacket();
    if(packet == nullptr)
    {
        //Time out!
        return NULL;
    }
    if(packet->en_type != EN_AUDIO_CODEC_DATA)
    {
        return NULL;
    }
    for(unsigned int i = 0; i < packet->es_aud_i.packet_count; i++)
    {
        pvBuffer = (void *) packet->es_aud.packet_data[i].data;
        byte = packet->es_aud_i.packet_info[i].size;

        do
        {
            ret = MI_AO_Write((MI_AUDIO_DEV) desc->devId, pvBuffer, byte, 0, 20);

            if(MI_SUCCESS == ret)
            {
                break;
            }

            send_retry--;
        }
        while(send_retry);
    }
    return NULL;
}

AMIGOS_MODULE_INIT("AO", AmigosModuleAo);
