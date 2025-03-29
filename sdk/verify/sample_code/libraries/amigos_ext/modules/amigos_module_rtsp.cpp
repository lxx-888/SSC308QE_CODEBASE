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
#include <string.h>
#include <pthread.h>
#include <unistd.h>

#include <cassert>
#include <map>
#include <memory>
#include <string>

#include "amigos_env.h"
#include "amigos_module_base.h"
#include "amigos_module_init.h"
#include "amigos_surface_rtsp.h"
#include "mi_venc.h"
#include "mi_common.h"
#include "ss_linker.h"
#include "ss_packet.h"
#include "ss_rtsp.h"
#include "ss_auto_lock.h"
#include "amigos_module_rtsp.h"
#include "ss_enum_cast.hpp"
#include "amigos_module_aio.h"

#if !defined(__BIONIC__) && defined(CONFIG_ONVIF)
#include "onvif_server.h"
#endif

#define RTSP_LISTEN_PORT   554

#define VIDEO_ES_STRAM_FMT_2_RTSP(__rtsp, __stream) do { \
    switch (__stream)                                    \
    {                                                    \
        case ES_STREAM_H264:                             \
            __rtsp = RTSP_ES_FMT_VIDEO_H264;             \
            break;                                       \
        case ES_STREAM_H265:                             \
            __rtsp = RTSP_ES_FMT_VIDEO_H265;             \
            break;                                       \
        case ES_STREAM_JPEG:                             \
            __rtsp = RTSP_ES_FMT_VIDEO_JPEG;             \
            break;                                       \
        case ES_STREAM_AV1:                              \
            __rtsp = RTSP_ES_FMT_VIDEO_AV1;              \
            break;                                       \
        case ES_STREAM_VP9:                              \
            __rtsp = RTSP_ES_FMT_VIDEO_VP9;              \
            break;                                       \
        default:                                         \
            AMIGOS_ERR("VID FMT : %d Err!\n", __stream); \
            __rtsp = RTSP_ES_FMT_VIDEO_NONE;             \
            break;                                       \
    }                                                    \
} while(0)

#define AUDIO_ES_STRAM_FMT_2_RTSP(__rtsp, __stream) do { \
    switch (__stream)                                    \
    {                                                    \
        case ES_STREAM_PCM:                              \
            __rtsp = RTSP_ES_FMT_AUDIO_PCM;              \
            break;                                       \
        case ES_STREAM_AAC:                              \
            __rtsp = RTSP_ES_FMT_AUDIO_AAC;              \
            break;                                       \
        case ES_STREAM_G711U:                            \
            __rtsp = RTSP_ES_FMT_AUDIO_PCMU;             \
            break;                                       \
        case ES_STREAM_G711A:                            \
            __rtsp = RTSP_ES_FMT_AUDIO_PCMA;             \
            break;                                       \
        case ES_STREAM_G726_16:                          \
        case ES_STREAM_G726_24:                          \
        case ES_STREAM_G726_32:                          \
        case ES_STREAM_G726_40:                          \
            __rtsp = RTSP_ES_FMT_AUDIO_G726;             \
            break;                                       \
        default:                                         \
            AMIGOS_ERR("AUD FMT : %d Err!\n", __stream); \
            __rtsp = RTSP_ES_FMT_AUDIO_NONE;             \
            break;                                       \
    }                                                    \
} while(0)



typedef struct stRtspClientEvent_s
{
    unsigned char ucharCmd; //0: open url, 1: while loop
    const char *pstrUrl;
} stRtspClientEvent_t;


template <typename T>
class rtsp_stream_in_packet : public ss_rtsp_packet
{
public:
    explicit rtsp_stream_in_packet(const struct rtsp_frame_packet &packet, stream_packet_obj obj)
        : ss_rtsp_packet(obj->get_time_stamp()), store_obj(obj)
    {
        T parser;
        parser.parse_nalu(packet, this->frame);
    }
    ~rtsp_stream_in_packet() final
    {
    }
private:
    stream_packet_obj store_obj;
};

class rtsp_stream_out_packet : public stream_packet_base
{
public:
    rtsp_stream_out_packet(const struct rtsp_video_output &video_output) : stream_packet_base()
    {
        this->en_type = EN_VIDEO_CODEC_DATA;
        if (video_output.info.format == RTSP_ES_FMT_VIDEO_H264)
        {
            this->es_vid_i.fmt = ES_STREAM_H264;
        }
        else if (video_output.info.format == RTSP_ES_FMT_VIDEO_H265)
        {
            this->es_vid_i.fmt = ES_STREAM_H265;
        }
        else if (video_output.info.format == RTSP_ES_FMT_VIDEO_JPEG)
        {
            this->es_vid_i.fmt = ES_STREAM_JPEG;
        }
        else if (video_output.info.format == RTSP_ES_FMT_VIDEO_AV1)
        {
            this->es_vid_i.fmt = ES_STREAM_AV1;
        }
        else if (video_output.info.format == RTSP_ES_FMT_VIDEO_VP9)
        {
            this->es_vid_i.fmt = ES_STREAM_VP9;
        }
        else
        {
            AMIGOS_ERR("Not support current video fmt.\n");
            throw err_buf();
        }
        this->es_vid_i.width        = video_output.info.width;
        this->es_vid_i.height       = video_output.info.height;
        this->es_vid_i.b_head       = false;
        this->es_vid_i.packet_count = video_output.frame_package.packet_count;
        this->set_time_stamp(video_output.frame_package.stamp);
        for (unsigned int i = 0; i < this->es_vid_i.packet_count; i++)
        {
            this->es_vid_i.packet_info[i].size  = video_output.frame_package.packet_data[i].size;
            this->es_vid_i.packet_info[i].b_end = true;
            this->es_vid.packet_data[i].data    = video_output.frame_package.packet_data[i].data;
        }
    }
    rtsp_stream_out_packet(const struct rtsp_audio_output &audio_output) : stream_packet_base()
    {
        this->en_type = EN_AUDIO_CODEC_DATA;
        if (audio_output.info.format == RTSP_ES_FMT_AUDIO_PCM)
        {
            this->es_aud_i.fmt = ES_STREAM_PCM;
        }
        else if (audio_output.info.format == RTSP_ES_FMT_AUDIO_AAC)
        {
            this->es_aud_i.fmt = ES_STREAM_AAC;
        }
        else
        {
            AMIGOS_ERR("Not support current audio fmt.\n");
            throw err_buf();
        }
        this->es_aud_i.sample_rate  = audio_output.info.sample_rate;
        this->es_aud_i.sample_width = audio_output.info.sample_width;
        this->es_aud_i.channels     = audio_output.info.channels;
        this->es_aud_i.packet_count   = audio_output.frame_package.packet_count;
        for (unsigned int i = 0; i < this->es_aud_i.packet_count; i++)
        {
            this->es_aud_i.packet_info[i].size = audio_output.frame_package.packet_data[i].size;
            this->es_aud.packet_data[i].data   = audio_output.frame_package.packet_data[i].data;
        }
    }
    virtual ~rtsp_stream_out_packet() {}
    stream_packet_obj dup() override
    {
        stream_packet_info packet_info;
        packet_info.en_type  = this->en_type;
        packet_info.es_vid_i = this->es_vid_i;
        auto new_pack = stream_packet_base::make<stream_packet>(packet_info);
        assert(new_pack);
        if (new_pack->en_type == EN_VIDEO_CODEC_DATA)
        {
            for (unsigned int i = 0; i < new_pack->es_vid_i.packet_count; ++i)
            {
                memcpy(new_pack->es_vid.packet_data[i].data, this->es_vid.packet_data[i].data,
                       new_pack->es_vid_i.packet_info[i].size);
            }
        }
        else if (new_pack->en_type == EN_AUDIO_CODEC_DATA)
        {
            for (unsigned int i = 0; i < new_pack->es_aud_i.packet_count; ++i)
            {
                memcpy(new_pack->es_aud.packet_data[i].data, this->es_aud.packet_data[i].data,
                       new_pack->es_aud_i.packet_info[i].size);
            }
        }
        return new_pack;
    }
};

AmigosModuleRtsp::AmigosModuleRtsp(const std::string &strSection)
    : AmigosSurfaceRtsp(strSection), AmigosModuleBase(this)
{
}
AmigosModuleRtsp::~AmigosModuleRtsp() {}

void AmigosModuleRtsp::_Init()
{
}
void AmigosModuleRtsp::_StartOut(unsigned int outPortId)
{
    auto iter = this->mapPortOut.find(outPortId);
    if (iter == this->mapPortOut.end())
    {
        AMIGOS_ERR("Base out port %d not exist\n", outPortId);
        return;
    }
    auto iterOutInfo = mapPortToCfg.find(outPortId);
    if (iterOutInfo == mapPortToCfg.end())
    {
        AMIGOS_ERR("Base out port info %d not exist\n", outPortId);
        return;
    }
    struct RtspOutDesc &desc = this->mapReaderDesc[iterOutInfo->second.url];
    if (iterOutInfo->second.portType == "video" && !desc.video_linker)
    {
        desc.video_linker = &iter->second.positive;
    }
    else if (iterOutInfo->second.portType == "audio" && !desc.audio_linker)
    {
        desc.audio_linker = &iter->second.positive;
    }
    else
    {
        AMIGOS_ERR("Base out port %d type error, URL: %s\n", outPortId, iterOutInfo->second.url.c_str());
        AMIGOS_ERR("RTSP only support single video and audio for the same URL.\n");
        return;
    }
}

void AmigosModuleRtsp::_StopOut(unsigned int outPortId)
{
    auto iterOutInfo = mapPortToCfg.find(outPortId);
    if (iterOutInfo == mapPortToCfg.end())
    {
        AMIGOS_ERR("Base out port info %d not exist\n", outPortId);
        return;
    }
    auto iter = this->mapReaderDesc.find(iterOutInfo->second.url);
    if (this->mapReaderDesc.end() == iter)
    {
        AMIGOS_ERR("Reader not found\n");
        return;
    }
    if (iterOutInfo->second.portType == "video")
    {
        iter->second.video_linker = NULL;
    }
    else if (iterOutInfo->second.portType == "audio")
    {
        iter->second.audio_linker = NULL;
    }
    else
    {
        AMIGOS_ERR("Base out port %d type error\n", outPortId);
        return;
    }
    if (!iter->second.video_linker && !iter->second.audio_linker)
    {
        if (iter->second.output_ref)
        {
            AMIGOS_ERR("Output ref did not clear!\n");
            return;
        }
        this->mapReaderDesc.erase(iter);
    }
}
void AmigosModuleRtsp::_Start()
{
    if (mapModInputInfo.size())
    {
        start_server();
        SaveEnvUrlPrefix();
        if (bOpenOnvif)
        {
#if !defined(__BIONIC__) && defined(CONFIG_ONVIF)
            struct MST_ONVIF_Descs descs;
            memset(&descs, 0, sizeof(struct MST_ONVIF_Descs));
            descs.count = this->mapRtspInInfo.size();
            for (auto &it : this->mapRtspInInfo)
            {
                strncpy(descs.descs[it.first].name, it.second.url.c_str(), MST_ONVIF_NAME_MAX_LEN - 1);
            }
            MST_ONVIF_Init();
            MST_ONVIF_SetDescs(&descs);
            MST_ONVIF_StartTask();
#else
            AMIGOS_INFO("Not support onvif on bionic!\n");
#endif
        }
    }
}
void AmigosModuleRtsp::_Stop()
{
    if (mapModInputInfo.size())
    {
        if (bOpenOnvif)
        {
#if !defined(__BIONIC__) && defined(CONFIG_ONVIF)
            MST_ONVIF_StopTask();
#else
            AMIGOS_INFO("Not support onvif on bionic!\n");
#endif
        }
        ClearEnvUrlPrefix();
        stop_server();
    }
}
void AmigosModuleRtsp::_Deinit()
{
}

void AmigosModuleRtsp::SaveEnvUrlPrefix()
{
    unsigned int urlId = 0;
    std::stringstream ss;
    for (auto &it : this->mapWriterDesc)
    {
        ss << "URL_" << this->stModInfo.devId << "_" << this->stModInfo.chnId << "_" << urlId++;
        if (it.second.video_pool_handle)
        {
            this->env.Ext("RTSP_PREVIEW_WINDOWS")[ss.str()] = get_url_prefix(it.second.video_pool_handle);
        }
        else if (it.second.audio_pool_handle)
        {
            this->env.Ext("RTSP_PREVIEW_WINDOWS")[ss.str()] = get_url_prefix(it.second.audio_pool_handle);
        }
        ss.str("");
    }
}

void AmigosModuleRtsp::ClearEnvUrlPrefix()
{
    unsigned int urlId = 0;
    std::stringstream ss;
    for (auto it : this->mapWriterDesc)
    {
        ss << "URL_" << this->stModInfo.devId << "_" << this->stModInfo.chnId << "_" << urlId++;
        this->env.Ext("RTSP_PREVIEW_WINDOWS")[ss.str()] = "";
        ss.str("");
    }
}

int AmigosModuleRtsp::_DataReceiver(unsigned int inPortId, stream_packet_obj &obj)
{
    struct RtspInInfo stInInfo;
    if (!this->GetRtspInInfo(inPortId, stInInfo))
    {
        AMIGOS_ERR("Can not find the url, port : %d\n", inPortId);
        return -1;
    }
    char array_data[8];
    memset(array_data, 0, sizeof(char) * 8);
    auto iter = this->mapWriterDesc.find(stInInfo.url);
    if (iter == this->mapWriterDesc.end())
    {
        AMIGOS_ERR("Can not find desc by url %s\n", stInInfo.url.c_str());
        return -1;
    }
    stream_packet_obj packet = stream_packet_base::dup(obj);
    if (!packet)
    {
        AMIGOS_ERR("Can not dup input buffer! url %s\n", stInInfo.url.c_str());
        return -1;
    }
    switch (packet->en_type)
    {
        case EN_VIDEO_CODEC_DATA:
        {
            rtsp_packet in_packet;
            if (!iter->second.video_pool_handle)
            {
                //AMIGOS_ERR("[%s]Not found video pool handle!\n", stInInfo.url.c_str());
                return -1;
            }
            if (0 != this->check_pool_package(iter->second.video_pool_handle))
            {
                return LinkerRtspServer::RETRY_LATER;
            }
            struct rtsp_frame_packet frame_package;
            frame_package.packet_count = packet->es_vid_i.packet_count;
            for (unsigned int i = 0; i < packet->es_vid_i.packet_count; i++)
            {
                frame_package.packet_data[i].b_end = packet->es_vid_i.packet_info[i].b_end;
                frame_package.packet_data[i].size  = packet->es_vid_i.packet_info[i].size;
                frame_package.packet_data[i].data  = packet->es_vid.packet_data[i].data;
            }
            switch (packet->es_vid_i.fmt)
            {
                case ES_STREAM_H264:
                    in_packet = ss_rtsp_packet::make<rtsp_stream_in_packet<rtsp_h264_data_parser>>(frame_package, packet);
                    break;
                case ES_STREAM_H265:
                    in_packet = ss_rtsp_packet::make<rtsp_stream_in_packet<rtsp_h265_data_parser>>(frame_package, packet);
                    break;
                case ES_STREAM_JPEG:
                case ES_STREAM_AV1:
                case ES_STREAM_VP9:
                    in_packet = ss_rtsp_packet::make<rtsp_stream_in_packet<rtsp_data_parser>>(frame_package, packet);
                    break;
                default:
                    AMIGOS_ERR("Not support fmt %d\n", packet->es_vid_i.fmt);
                    return -1;
                break;
            }
            this->send_pool_package(iter->second.video_pool_handle, in_packet);
        }
        break;
        case EN_AUDIO_CODEC_DATA:
        {
            if (!iter->second.audio_pool_handle)
            {
                //AMIGOS_ERR("[%s]Not found audio pool handle!\n", stInInFo.url.c_str());
                return -1;
            }
            if (0 != this->check_pool_package(iter->second.audio_pool_handle))
            {
                return LinkerRtspServer::RETRY_LATER;
            }
            struct rtsp_frame_packet frame_package;
            frame_package.packet_count = packet->es_aud_i.packet_count;
            for (unsigned int i = 0; i < packet->es_aud_i.packet_count; i++)
            {
                frame_package.packet_data[i].b_end = true;
                frame_package.packet_data[i].size  = packet->es_aud_i.packet_info[i].size;
                frame_package.packet_data[i].data  = packet->es_aud.packet_data[i].data;
            }
            rtsp_packet in_packet = ss_rtsp_packet::make<rtsp_stream_in_packet<rtsp_data_parser>>(frame_package, packet);
            this->send_pool_package(iter->second.audio_pool_handle, in_packet);
        }
        break;
        default:
        {
            AMIGOS_ERR("Not support type %d\n", packet->en_type);
            return -1;
        }
    }
    return 0;
}

void AmigosModuleRtsp::_StartIn(unsigned int inPortId)
{
    struct RtspInInfo  stInInfo;
    stream_packet_info streamInfo;

    if (!this->GetRtspInInfo(inPortId, stInInfo))
    {
        AMIGOS_ERR("Surface error.\n");
        return;
    }
    auto iter = this->mapPortIn.find(inPortId);
    if (iter == this->mapPortIn.end())
    {
        AMIGOS_ERR("Port %d not found\n", inPortId);
        return;
    }
    streamInfo = iter->second.get_packet_info();
    if (streamInfo.en_type != EN_VIDEO_CODEC_DATA && streamInfo.en_type != EN_AUDIO_CODEC_DATA)
    {
        AMIGOS_ERR("Prev mod error, type: %d\n", streamInfo.en_type);
        return;
    }
    auto &desc = this->mapWriterDesc[stInInfo.url];
    AMIGOS_INFO("INPORT %d, STREAM : %s DEPTH_IN %d.\n", inPortId, stInInfo.url.c_str(), stInInfo.depth);
    if (streamInfo.en_type == EN_VIDEO_CODEC_DATA)
    {
        rtsp_video_info info;
        rtsp_pool_config pool_config;

        memset(&info, 0, sizeof(rtsp_video_info));
        VIDEO_ES_STRAM_FMT_2_RTSP(info.format, streamInfo.es_vid_i.fmt);
        info.frame_rate = 1000;
        info.width      = streamInfo.es_vid_i.width;
        info.height     = streamInfo.es_vid_i.height;
        AMIGOS_INFO("RTSP VID: format      : %d\n", streamInfo.es_vid_i.fmt);
        AMIGOS_INFO("RTSP VID: width       : %d\n", streamInfo.es_vid_i.width);
        AMIGOS_INFO("RTSP VID: height      : %d\n", streamInfo.es_vid_i.height);
        int ret = this->add_video_server_url(stInInfo.url, info);
        if (ret == -1)
        {
            AMILOG_ERR << "Add video server url error, URL:" << stInInfo.url << COLOR_ENDL;
            return;
        }
        desc.video_pool_handle = this->get_video_pool_handle(stInInfo.url);
        assert(desc.video_pool_handle);
        desc.video_in_message = &iter->second;
        pool_config.depth = stInInfo.depth;
        this->config_pool(desc.video_pool_handle, pool_config);
    }
    else if (streamInfo.en_type == EN_AUDIO_CODEC_DATA)
    {
        rtsp_audio_info info;
        rtsp_pool_config pool_config;
        memset(&info, 0, sizeof(rtsp_audio_info));
        AUDIO_ES_STRAM_FMT_2_RTSP(info.format, streamInfo.es_aud_i.fmt);
        info.channels     = streamInfo.es_aud_i.channels;
        info.sample_rate  = streamInfo.es_aud_i.sample_rate;
        info.sample_width = streamInfo.es_aud_i.sample_width;
        AMIGOS_INFO("RTSP AUD: sample rate  : %d\n", streamInfo.es_aud_i.sample_rate);
        AMIGOS_INFO("RTSP AUD: sample width : %d\n", streamInfo.es_aud_i.sample_width);
        AMIGOS_INFO("RTSP AUD: channels     : %d\n", streamInfo.es_aud_i.channels);
        AMIGOS_INFO("RTSP AUD: fmt          : %d\n", streamInfo.es_aud_i.fmt);
        int ret = this->add_audio_server_url(stInInfo.url, info);
        if (ret == -1)
        {
            AMILOG_ERR << "Add audio server url error, URL:" << stInInfo.url<< COLOR_ENDL;
            return;
        }
        desc.audio_pool_handle = this->get_audio_pool_handle(stInInfo.url);
        assert(desc.audio_pool_handle);
        desc.audio_in_message = &iter->second;
        pool_config.depth = stInInfo.depth;
        this->config_pool(desc.audio_pool_handle, pool_config);
    }
}

void AmigosModuleRtsp::_StopIn(unsigned int inPortId)
{
    struct RtspInInfo  stInInfo;
    stream_packet_info streamInfo;

    if (!this->GetRtspInInfo(inPortId, stInInfo))
    {
        AMIGOS_ERR("Surface error.\n");
        return;
    }
    auto iter = this->mapPortIn.find(inPortId);
    if (iter == this->mapPortIn.end())
    {
        AMIGOS_ERR("Port %d not found\n", inPortId);
        return;
    }
    streamInfo = iter->second.get_packet_info();
    if (streamInfo.en_type != EN_VIDEO_CODEC_DATA && streamInfo.en_type != EN_AUDIO_CODEC_DATA)
    {
        AMIGOS_ERR("Prev mod error, type: %d\n", streamInfo.en_type);
        return;
    }
    auto writerIter = this->mapWriterDesc.find(stInInfo.url);
    if (writerIter == this->mapWriterDesc.end())
    {
        AMIGOS_ERR("PORT %d URL %s, Writer not found\n", inPortId, stInInfo.url.c_str());
        return;
    }
    if (streamInfo.en_type == EN_VIDEO_CODEC_DATA)
    {
        int ret = this->del_video_server_url(stInInfo.url);
        if (ret == -1)
        {
            AMILOG_ERR << "Del video server url error!, URL:" << stInInfo.url << COLOR_ENDL;
            return;
        }
        writerIter->second.video_pool_handle = NULL;
    }
    else if (streamInfo.en_type == EN_AUDIO_CODEC_DATA)
    {
        int ret = this->del_audio_server_url(stInInfo.url);
        if (ret == -1)
        {
            AMILOG_ERR << "Del audio server url error!, URL:" << stInInfo.url << COLOR_ENDL;
            return;
        }
        writerIter->second.audio_pool_handle = NULL;
    }
    if (!writerIter->second.video_pool_handle && !writerIter->second.audio_pool_handle)
    {
        this->mapWriterDesc.erase(writerIter);
    }
}

int AmigosModuleRtsp::_Connected(unsigned int outPortId, unsigned int ref)
{
    if (ref > 0)
    {
        return 0;
    }
    auto iterOutInfo = mapPortToCfg.find(outPortId);
    if (iterOutInfo == mapPortToCfg.end())
    {
        AMIGOS_ERR("Base out port info %d not exist\n", outPortId);
        return -1;
    }
    auto iter = this->mapReaderDesc.find(iterOutInfo->second.url);
    if (this->mapReaderDesc.end() == iter)
    {
        AMIGOS_ERR("Reader not found\n");
        return -1;
    }
    if (!iter->second.output_ref)
    {
        ss_rtsp_client::play(iterOutInfo->second.url, iterOutInfo->second.userName, iterOutInfo->second.passwd);
    }
    iter->second.output_ref++;
    AMIGOS_INFO("Ref=%d\n", iter->second.output_ref);
    return 0;
}

int AmigosModuleRtsp::_Disconnected(unsigned int outPortId, unsigned int ref)
{
    if (ref > 0)
    {
        return 0;
    }
    auto iterOutInfo = mapPortToCfg.find(outPortId);
    if (iterOutInfo == mapPortToCfg.end())
    {
        AMIGOS_ERR("Base out port info %d not exist\n", outPortId);
        return -1;
    }
    auto iter = this->mapReaderDesc.find(iterOutInfo->second.url);
    if (this->mapReaderDesc.end() == iter)
    {
        AMIGOS_ERR("Reader not found\n");
        return -1;
    }
    if (!iter->second.output_ref)
    {
        AMIGOS_ERR("Ref=0 error!\n");
        return -1;
    }
    iter->second.output_ref--;
    if (!iter->second.output_ref)
    {
        ss_rtsp_client::stop(iterOutInfo->second.url);
    }
    AMIGOS_INFO("Ref=%d\n", iter->second.output_ref);
    return 0;
}
unsigned int AmigosModuleRtsp::GetModId() const
{
    return uintExtModId;
}
unsigned int AmigosModuleRtsp::GetInputType(unsigned int port) const
{
    return E_MOD_PORT_TYPE_USER;
}
unsigned int AmigosModuleRtsp::GetOutputType(unsigned int port) const
{
    return E_MOD_PORT_TYPE_USER;
}

// ss_rtsp api.
void AmigosModuleRtsp::connect_video_stream(const std::string &url)
{
    auto iter = this->mapWriterDesc.find(url);
    if (iter == this->mapWriterDesc.end())
    {
        AMIGOS_ERR("Not found url %s\n", url.c_str());
        return;
    }
    if (!iter->second.video_in_message)
    {
        AMIGOS_ERR("[%s]Not found video connector!\n", url.c_str());
        return;
    }
    iter->second.video_in_message->access();
    AMIGOS_INFO("Vidoe stream %s connect\n", url.c_str());
}
void AmigosModuleRtsp::disconnect_video_stream(const std::string &url)
{
    auto iter = this->mapWriterDesc.find(url);
    if (iter == this->mapWriterDesc.end())
    {
        AMIGOS_ERR("Not found url %s\n", url.c_str());
        return;
    }
    if (!iter->second.video_in_message)
    {
        AMIGOS_ERR("[%s]Not found video connector!\n", url.c_str());
        return;
    }
    iter->second.video_in_message->leave();
    AMIGOS_INFO("Vidoe stream %s disconnect\n", url.c_str());
}
void AmigosModuleRtsp::connect_audio_stream(const std::string &url)
{
    auto iter = this->mapWriterDesc.find(url);
    if (iter == this->mapWriterDesc.end())
    {
        AMIGOS_ERR("Not found url %s\n", url.c_str());
        return;
    }
    if (!iter->second.audio_in_message)
    {
        AMIGOS_ERR("[%s]Not found audio connector!\n", url.c_str());
        return;
    }
    iter->second.audio_in_message->access();
    AMIGOS_INFO("Audio stream %s connect\n", url.c_str());
}
void AmigosModuleRtsp::disconnect_audio_stream(const std::string &url)
{
    auto iter = this->mapWriterDesc.find(url);
    if (iter == this->mapWriterDesc.end())
    {
        AMIGOS_ERR("Not found url %s\n", url.c_str());
        return;
    }
    if (!iter->second.audio_in_message)
    {
        AMIGOS_ERR("[%s]Not found audio connector!\n", url.c_str());
        return;
    }
    iter->second.audio_in_message->leave();
    AMIGOS_INFO("Audio stream %s disconnect\n", url.c_str());
}

// ss_rtsp_client api.
int AmigosModuleRtsp::recv_video_package(const std::string &url, const struct rtsp_video_output &video_output,
                                          unsigned int frame_id)
{
    stream_packet_obj packet = stream_packet_base::make<rtsp_stream_out_packet>(video_output);
    auto iter = this->mapReaderDesc.find(url);
    if (iter == this->mapReaderDesc.end())
    {
        AMIGOS_ERR("Video not found: %s, Fid: %d, Fmt: %d\n", url.c_str(), frame_id, video_output.info.format);
        return -1;
    }
    if (!iter->second.video_linker)
    {
        AMIGOS_ERR("Video connector not found: %s, Fid: %d, Fmt: %d\n", url.c_str(), frame_id, video_output.info.format);
        return -1;
    }
    return iter->second.video_linker->enqueue(packet);
}
int AmigosModuleRtsp::recv_audio_package(const std::string &url, const struct rtsp_audio_output &audio_output,
                                          unsigned int frame_id)
{
    stream_packet_obj packet = stream_packet_base::make<rtsp_stream_out_packet>(audio_output);
    auto iter = this->mapReaderDesc.find(url);
    if (iter == this->mapReaderDesc.end())
    {
        AMIGOS_ERR("Audio not found: %s, Fid: %d, Fmt: %d\n", url.c_str(), frame_id, audio_output.info.format);
        return -1;
    }
    if (!iter->second.audio_linker)
    {
        AMIGOS_ERR("Audio connector not found: %s, Fid: %d, Fmt: %d\n", url.c_str(), frame_id, audio_output.info.format);
        return -1;
    }
    return iter->second.audio_linker->enqueue(packet);
}
ss_linker_base *AmigosModuleRtsp::_CreateInputNegativeLinker(unsigned int inPortId)
{
    struct RtspInInfo stInInfo;
    auto iter = this->mapModInputInfo.find(inPortId);
    if (iter == this->mapModInputInfo.end())
    {
        AMIGOS_ERR("Port %d not found\n", inPortId);
        return nullptr;
    }
    if (!this->GetRtspInInfo(inPortId, stInInfo))
    {
        AMIGOS_ERR("Can not find the url, port : %d\n", inPortId);
        return nullptr;
    }
    if (0 == iter->second.curFrmRate)
    {
        AMIGOS_ERR("set input port %d fps 0, not permit, set default 30fps\n", inPortId);
        return new LinkerRtspServer(1000000 / 30 * stInInfo.depth, inPortId, this);
    }
    return new LinkerRtspServer(1000000 / iter->second.curFrmRate * stInInfo.depth, inPortId, this);
}
stream_packet_info AmigosModuleRtsp::_GetStreamInfo(unsigned int outPortId)
{
    struct stream_packet_info streamInfo;
    auto iterOutInfo = mapPortToCfg.find(outPortId);
    if (iterOutInfo->second.portType == "video")
    {
        streamInfo.en_type         = EN_VIDEO_CODEC_DATA;
        streamInfo.es_vid_i.fmt    = ss_enum_cast<es_video_fmt>::from_str(iterOutInfo->second.fmt);
        streamInfo.es_vid_i.width  = iterOutInfo->second.width;
        streamInfo.es_vid_i.height = iterOutInfo->second.height;
    }
    else if (iterOutInfo->second.portType == "audio")
    {
        streamInfo.en_type               = EN_AUDIO_CODEC_DATA;
        streamInfo.es_aud_i.fmt          = ss_enum_cast<es_audio_fmt>::from_str(iterOutInfo->second.fmt);
        streamInfo.es_aud_i.channels     = ss_enum_cast<MI_AUDIO_SoundMode_e>::from_str(iterOutInfo->second.channel);
        streamInfo.es_aud_i.sample_rate  = iterOutInfo->second.samplerate;
        streamInfo.es_aud_i.sample_width = 16;
    }
    return streamInfo;
}
AMIGOS_MODULE_INIT("RTSP", AmigosModuleRtsp);
