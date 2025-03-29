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
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <iostream>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "ss_auto_lock.h"
#include "ss_linker.h"
#include "ss_packet.h"
#include "ss_thread.h"
#include "amigos_module_init.h"
#include "amigos_module_file.h"
#include "ss_auto_lock.h"

#define U32VALUE(pu8Data, index) (pu8Data[index]<<24)|(pu8Data[index+1]<<16)|(pu8Data[index+2]<<8)|(pu8Data[index+3])
#define PACKET_SIZE_ALIGNMENT_BYTE 1024

struct wave_file_header
{
    char riff[4];
    unsigned int riff_len;
    char wave[4];
    char fmt[4];
    unsigned int fmt_len;
    struct wave_format
    {
        signed short format_tag;
        signed short channels;
        unsigned int samples_per_sec;
        unsigned int avg_bytes_per_sec;
        signed short block_align;
        signed short bits_per_sample;
    } wave_info;
    char data[4];
    unsigned int data_len;
};

static void ReadRawFile(FILE *fp, const raw_video_info &info, raw_video_packet &data);
static bool CheckEsVideoFile(FILE *fp, es_video_info &info, ss_video_packet_filter &video_filter);
static void ReadEsVideoFile(FILE *fp, const es_video_info &info, const es_video_packet &data);
static unsigned int ReadEsAudioFile(FILE *fp, bool b_skip_wav_header, const es_audio_info &info, es_audio_packet &data);

static void WriteRawFile(FILE *fp, const raw_video_info &info, const raw_video_packet &data);
static void WriteEsVideoFile(FILE *fp, bool b_add_head, const es_video_info &info, const es_video_packet &data);
static void WriteEsAudioFile(FILE *fp, const es_audio_info &info, const es_audio_packet &data);
static void WriteMetadataFile(FILE *fp, bool b_add_head, const meta_data_info &info, const meta_data_packet &data);

static void *RawFileReader(struct ss_thread_buffer *thread_buf);
static void *EsVideoFileReader(struct ss_thread_buffer *thread_buf);
static void *EsAudioFileReader(struct ss_thread_buffer *thread_buf);
static void *MetadataFileReader(struct ss_thread_buffer *thread_buf);

AmigosModuleFile::AmigosModuleFile(const std::string &strSection)
    : AmigosSurfaceFile(strSection), AmigosModuleBase(this)
{
}
AmigosModuleFile::~AmigosModuleFile()
{
}

void AmigosModuleFile::_Init()
{
}
void AmigosModuleFile::_Deinit()
{
}
void AmigosModuleFile::SetLeftFrame(unsigned int inPortId, unsigned int frameCnt)
{
    auto iter = this->mapFileWriterDesc.find(inPortId);
    if (iter == this->mapFileWriterDesc.end())
    {
        AMIGOS_ERR("File input port %d has not fd.\n", inPortId);
        return;
    }
    ss_auto_lock event(iter->second.frame_mutex, iter->second.frame_cond);
    iter->second.frameCntLimit = frameCnt;
}
bool AmigosModuleFile::WaitFrame(unsigned int inPortId, unsigned int waitMs)
{
    auto iter = this->mapFileWriterDesc.find(inPortId);
    if (iter == this->mapFileWriterDesc.end())
    {
        AMIGOS_ERR("File input port %d has not fd.\n", inPortId);
        return false;
    }
    ss_auto_lock event(iter->second.frame_mutex, iter->second.frame_cond);
    if (iter->second.frameCntLimit < 0)
    {
        if (!event.wait(waitMs))
        {
            // Wait timeout.
            return false;
        }
        return true;
    }
    while (iter->second.frameCntLimit)
    {
        if (!event.wait(waitMs))
        {
            // Wait timeout.
            return false;
        }
    }
    return true;
}
int AmigosModuleFile::_DataReceiver(unsigned int inPortId, stream_packet_obj &packet)
{
    auto iter = this->mapFileWriterDesc.find(inPortId);
    if (iter == this->mapFileWriterDesc.end())
    {
        AMIGOS_ERR("File input port %d has not fd.\n", inPortId);
        return -1;
    }
    ss_auto_lock event(iter->second.frame_mutex, iter->second.frame_cond);
    if (iter->second.frameCntLimit == 0)
    {
        return 0;
    }
    if (iter->second.frameCntLimit > 0)
    {
        iter->second.frameCntLimit--;
    }
    switch (packet->en_type)
    {
        case EN_RAW_FRAME_DATA_PA:
        {
            auto va = stream_packet_base::convert(packet, EN_RAW_FRAME_DATA);
            if (!va)
            {
                AMIGOS_ERR("Packet: %s PA to VA failed!\n", packet->get_type().c_str());
                return -1;
            }
            WriteRawFile(iter->second.writeFp, va->raw_vid_i, va->raw_vid);
            event.notify();
        }
        break;
        case EN_RAW_FRAME_DATA:
        {
            WriteRawFile(iter->second.writeFp, packet->raw_vid_i, packet->raw_vid);
            event.notify();
        }
        break;
        case EN_VIDEO_CODEC_DATA:
        {
            if (!iter->second.video_filter.check(packet))
            {
                break;
            }
            WriteEsVideoFile(iter->second.writeFp, iter->second.bHead, packet->es_vid_i, packet->es_vid);
            event.notify();
        }
        break;
        case EN_AUDIO_CODEC_DATA:
        {
            WriteEsAudioFile(iter->second.writeFp, packet->es_aud_i, packet->es_aud);
            event.notify();
        }
        break;
        case EN_USER_META_DATA:
        {
            WriteMetadataFile(iter->second.writeFp, true, packet->meta_data_i, packet->meta_data);
            event.notify();
        }
        break;
        default:
        break;
    }
    return 0;
}

void AmigosModuleFile::_StartIn(unsigned int inPortId)
{
    auto iter = this->mapInputWrFile.find(inPortId);
    if (iter == this->mapInputWrFile.end())
    {
        AMIGOS_ERR("File input port %d has not surface.\n", inPortId);
        return ;
    }
    stream_packet_info  info;
    auto it = mapPortIn.begin();
    if (mapPortIn.end() == it)
    {
        AMIGOS_ERR("has no input port!, startIn failed!\n");
        return;
    }
    info = it->second.get_packet_info();
    if (ES_STREAM_H264 == info.es_vid_i.fmt || ES_STREAM_H265 == info.es_vid_i.fmt)
    {
        AMIGOS_INFO("need set ES video state to check nal\n");
    }
    AmigosSurfaceFile::FileInInfo &stFileIn = iter->second;
    FILE* fp = fopen(stFileIn.fileName.c_str(), "w");
    if (fp == nullptr)
    {
        AMIGOS_ERR("File %s open failed\n", stFileIn.fileName.c_str());
        return ;
    }
    this->mapFileWriterDesc[inPortId].writeFp = fp;
    this->mapFileWriterDesc[inPortId].frameCntLimit = stFileIn.frameCntLimit;
    this->mapFileWriterDesc[inPortId].bHead         = stFileIn.bHead;
}
void AmigosModuleFile::_StopIn(unsigned int inPortId)
{
    auto iter = this->mapFileWriterDesc.find(inPortId);
    if (iter == this->mapFileWriterDesc.end())
    {
        return ;
    }
    iter->second.frameSkipCount = 0;
    fclose(iter->second.writeFp);
    this->mapFileWriterDesc.erase(iter);
}

void AmigosModuleFile::_SetFileReaderDesc(unsigned int outPortId)
{
    auto itReaderDesc = this->mapFileReaderDesc.find(outPortId);
    if (itReaderDesc != this->mapFileReaderDesc.end())
    {
        return;
    }
    auto iter = this->mapOutputRdFile.find(outPortId);
    if (iter == this->mapOutputRdFile.end())
    {
        AMIGOS_ERR("Output port %d has not surface.\n", outPortId);
        return;
    }
    AmigosSurfaceFile::FileOutInfo &stFileOut = iter->second;
    struct AmigosModuleFile::FileReaderDesc frdDesc;
    frdDesc.frameCntLimit = stFileOut.frameCntLimit;
    frdDesc.info.en_type = ss_enum_cast<stream_type>::from_str(stFileOut.strOutType);
    switch (frdDesc.info.en_type)
    {
    case EN_RAW_FRAME_DATA:
        {
            frdDesc.info.raw_vid_i.plane_num = stFileOut.intPlaneNum;
            for (unsigned int i = 0; i < frdDesc.info.raw_vid_i.plane_num; ++i)
            {
                if("bayer" == stFileOut.strOutFmt)
                {
                    frdDesc.info.raw_vid_i.plane_info[i].fmt = ss_enum_cast<raw_video_fmt>::from_str_bayer(stFileOut.strOutFmt,
                                                                                                            stFileOut.strBayerId,
                                                                                                            stFileOut.strPrecision);
                }
                else
                {
                    frdDesc.info.raw_vid_i.plane_info[i].fmt = ss_enum_cast<raw_video_fmt>::from_str(stFileOut.strOutFmt);
                }
                frdDesc.info.raw_vid_i.plane_info[i].width  = stFileOut.intFileOutWidth;
                frdDesc.info.raw_vid_i.plane_info[i].height = stFileOut.intFileOutHeight;
            }
        }
        break;
    case EN_VIDEO_CODEC_DATA:
        {
            frdDesc.info.es_vid_i.fmt    = ss_enum_cast<es_video_fmt>::from_str(stFileOut.strOutFmt);
            frdDesc.info.es_vid_i.width  = stFileOut.intFileOutWidth;
            frdDesc.info.es_vid_i.height = stFileOut.intFileOutHeight;
            frdDesc.info.es_vid_i.b_head = false;
        }
        break;
    case EN_AUDIO_CODEC_DATA:
        {
            wave_file_header wav_header;
            size_t  size = 0;
            memset(&wav_header, 0, sizeof(struct wave_file_header));
            FILE* fp = fopen(stFileOut.fileName.c_str(), "r");
            if (fp == nullptr)
            {
                AMIGOS_ERR("File %s open failed\n", stFileOut.fileName.c_str());
                return;
            }
            size = fread(&wav_header, 1, sizeof(struct wave_file_header), fp);
            if (size == sizeof(struct wave_file_header))
            {
               frdDesc.info.es_aud_i.sample_rate  = wav_header.wave_info.samples_per_sec;
               frdDesc.info.es_aud_i.sample_width = wav_header.wave_info.bits_per_sample;
               frdDesc.info.es_aud_i.channels     = wav_header.wave_info.channels;
               frdDesc.info.es_aud_i.fmt          = ss_enum_cast<es_audio_fmt>::from_str(stFileOut.strOutFmt);
               frdDesc.info.es_aud_i.packet_count = 1;
               frdDesc.gapDataSize  = 0;
               frdDesc.lastTimeSec  = 0;
               frdDesc.lastTimeNsec = 0;
               frdDesc.gapTimeNs    = 0;
            }
            fclose(fp);
        }
        break;
    case EN_USER_META_DATA:
        {
            //frdDesc.info.meta_data_i.size = 0x208; //get size from file header when call CheckMetadataFile;
        }
        break;
    default:
        {
            AMIGOS_ERR("File fmt %s not support\n", stFileOut.strOutType.c_str());
            return;
        }
        break;
    }
    mapFileReaderDesc[outPortId] = frdDesc;
}
void AmigosModuleFile::_StartOut(unsigned int outPortId)
{
    this->_SetFileReaderDesc(outPortId);
}

void AmigosModuleFile::_StopOut(unsigned int outPortId)
{
    auto iter = this->mapFileReaderDesc.find(outPortId);
    if (iter == this->mapFileReaderDesc.end())
    {
        AMIGOS_ERR("Port %d has not reader.\n", outPortId);
        return;
    }
    this->mapFileReaderDesc.erase(iter);
}

int AmigosModuleFile::_Connected(unsigned int outPortId, unsigned int ref)
{
    if (ref > 0)
    {
        return 0;
    }
    auto iterPortOut = this->mapPortOut.find(outPortId);
    if (iterPortOut == this->mapPortOut.end())
    {
        AMIGOS_ERR("Port %d has not base sturcture.\n", outPortId);
        return -1;
    }
    auto iterOut = this->mapOutputRdFile.find(outPortId);
    if (iterOut == this->mapOutputRdFile.end())
    {
        AMIGOS_ERR("Output port %d has not surface.\n", outPortId);
        return -1;
    }
    auto itDesc = this->mapFileReaderDesc.find(outPortId);
    if (itDesc == this->mapFileReaderDesc.end())
    {
        AMIGOS_ERR("Output port %d has not file desc.\n", outPortId);
        return -1;
    }
    AmigosSurfaceFile::FileOutInfo &stFileOut = iterOut->second;
    itDesc->second.linkerOut = &iterPortOut->second.positive;
    itDesc->second.packer    = &iterPortOut->second.outPacker;
    itDesc->second.readFp = fopen(stFileOut.fileName.c_str(), "r");
    if (itDesc->second.readFp == nullptr)
    {
        AMIGOS_ERR("File %s open failed\n", stFileOut.fileName.c_str());
        return -1;
    }
    if(iterPortOut->second.positive.empty())
    {
        AMIGOS_INFO("Post module from output port %d is doing post reader mode.\n", outPortId);
        return 0;
    }
    AmigosSurfaceBase::ModPortOutInfo stOut;
    this->GetSurface()->GetPortOutInfo(outPortId, stOut);
    struct ss_thread_attr ss_attr;
    memset(&ss_attr, 0, sizeof(struct ss_thread_attr));
    ss_attr.do_signal = NULL;
    if (stOut.curFrmRate == 1)
    {
        ss_attr.monitor_cycle_sec  = 1;
        ss_attr.monitor_cycle_nsec = 0;
    }
    else
    {
        ss_attr.monitor_cycle_sec  = 0;
        ss_attr.monitor_cycle_nsec = 1000000000 / stOut.curFrmRate;
    }
    ss_attr.is_reset_timer      = 0;
    ss_attr.in_buf.size         = 0;
    snprintf(ss_attr.thread_name, 128, "%s", this->GetOutPortIdStr(outPortId).c_str());
    switch (itDesc->second.info.en_type)
    {
    case EN_RAW_FRAME_DATA:
        ss_attr.do_monitor = RawFileReader;
        break;
    case EN_VIDEO_CODEC_DATA:
        ss_attr.do_monitor = EsVideoFileReader;
        break;
    case EN_AUDIO_CODEC_DATA:
        wave_file_header wav_header;
        memset(&wav_header, 0, sizeof(struct wave_file_header));
        fseek(itDesc->second.readFp, sizeof(struct wave_file_header), SEEK_SET);
        ss_attr.monitor_cycle_sec  = 0;
        ss_attr.do_monitor = EsAudioFileReader;
        break;
    case EN_USER_META_DATA:
        ss_attr.do_monitor = MetadataFileReader;
        break;
    default:
        AMIGOS_ERR("Not support in file out %d!\n", outPortId);
        fclose(itDesc->second.readFp);
        return -1;
    }
    ss_attr.in_buf.buf = (void *)&this->mapFileReaderDesc[outPortId];
    this->mapFileReaderDesc[outPortId].threadHandle = ss_thread_open(&ss_attr);
    if (!this->mapFileReaderDesc[outPortId].threadHandle)
    {
        AMIGOS_ERR("Monitor return error!\n");
        return -1;
    }
    ss_thread_start_monitor(this->mapFileReaderDesc[outPortId].threadHandle);
    AMIGOS_INFO("File reader [%s] start\n", this->GetOutPortIdStr(outPortId).c_str());
    return 0;
}

int AmigosModuleFile::_Disconnected(unsigned int outPortId, unsigned int ref)
{
    if (ref > 0)
    {
        return 0;
    }
    auto iter = mapFileReaderDesc.find(outPortId);
    if (iter == mapFileReaderDesc.end())
    {
        AMIGOS_ERR("Not found port %d\n", outPortId);
        return -1;
    }
    if (iter->second.linkerOut->empty())
    {
        AMIGOS_INFO("Post module is running post reader.%d\n", outPortId);
        return 0;
    }
    if (!iter->second.threadHandle)
    {
        AMIGOS_ERR("Monitor handle error!\n");
        return -1;
    }
    ss_thread_stop(iter->second.threadHandle);
    ss_thread_close(iter->second.threadHandle);
    fclose(iter->second.readFp);
    iter->second.readFp = nullptr;
    AMIGOS_INFO("File reader [%s] stop\n", this->GetOutPortIdStr(outPortId).c_str());

    return 0;
}
stream_packet_info AmigosModuleFile::_GetStreamInfo(unsigned int outPortId)
{
    this->_SetFileReaderDesc(outPortId);
    auto iter = mapFileReaderDesc.find(outPortId);
    if (iter == mapFileReaderDesc.end())
    {
        AMIGOS_ERR("Not found port %d\n", outPortId);
        return stream_packet_info();
    }
    return iter->second.info;
}
unsigned int AmigosModuleFile::GetModId() const
{
    return uintExtModId;
}
unsigned int AmigosModuleFile::GetInputType(unsigned int port) const
{
    return E_MOD_PORT_TYPE_USER;
}
unsigned int AmigosModuleFile::GetOutputType(unsigned int port) const
{
    return E_MOD_PORT_TYPE_USER;
}

static void WriteRawFile(FILE *fp, const raw_video_info &info, const raw_video_packet &data)
{
    for (unsigned int plane_idx = 0; plane_idx < info.plane_num; ++plane_idx)
    {
        if (data.plane_data[plane_idx].data[0])
        {
            fwrite(data.plane_data[plane_idx].data[0], 1, data.plane_data[plane_idx].size[0], fp);
        }
        if (data.plane_data[plane_idx].data[1])
        {
            fwrite(data.plane_data[plane_idx].data[1], 1, data.plane_data[plane_idx].size[1], fp);
        }
        if (data.plane_data[plane_idx].data[2])
        {
            fwrite(data.plane_data[plane_idx].data[2], 1, data.plane_data[plane_idx].size[2], fp);
        }
    }
}
static void WriteMetadataFile(FILE *fp, bool b_add_head, const meta_data_info &info, const meta_data_packet &data)
{
    if (b_add_head)
    {
        unsigned char header[16];
        unsigned int  data_size = info.size;
        memset(header, 0, 16);
        header[0] = 'M';
        header[1] = 'e';
        header[2] = 't';
        header[3] = 'a';
        header[4] = ((data_size) >> 24) & 0xFF;
        header[5] = ((data_size) >> 16) & 0xFF;
        header[6] = ((data_size) >> 8) & 0xFF;
        header[7] = (data_size)&0xFF;
        fwrite(header, 1, 16, fp);
    }
    fwrite(data.data, 1, info.size, fp);
}

static void WriteEsVideoFile(FILE *fp, bool b_add_head, const es_video_info &info, const es_video_packet &data)
{
    if (b_add_head)
    {
        unsigned char header[16];
        unsigned int  data_size = 0;
        for (unsigned int i = 0; i < info.packet_count; i++)
        {
            data_size += info.packet_info[i].size;
        }
        memset(header, 0, 16);
        header[0] = 0x1;
        header[4] = ((data_size) >> 24) & 0xFF;
        header[5] = ((data_size) >> 16) & 0xFF;
        header[6] = ((data_size) >> 8) & 0xFF;
        header[7] = (data_size)&0xFF;
        fwrite(header, 1, 16, fp);
    }
    for (unsigned int i = 0; i < info.packet_count; i++)
    {
        fwrite(data.packet_data[i].data, 1, info.packet_info[i].size, fp);
    }
}

static int fill_wave_header(struct wave_file_header *p_head, const es_audio_info &info)
{
    p_head->riff[0] = 'R';
    p_head->riff[1] = 'I';
    p_head->riff[2] = 'F';
    p_head->riff[3] = 'F';

    p_head->wave[0] = 'W';
    p_head->wave[1] = 'A';
    p_head->wave[2] = 'V';
    p_head->wave[3] = 'E';

    p_head->fmt[0] = 'f';
    p_head->fmt[1] = 'm';
    p_head->fmt[2] = 't';
    p_head->fmt[3] = 0x20;
    p_head->fmt_len = 0x10;

    if (info.fmt == ES_STREAM_G711A)
    {
        p_head->wave_info.format_tag = 0x6;
    }
    if (info.fmt == ES_STREAM_G711U)
    {
        p_head->wave_info.format_tag = 0x7;
    }
    if (info.fmt == ES_STREAM_G711A || info.fmt == ES_STREAM_G711U)
    {
        p_head->wave_info.bits_per_sample = 8;
        p_head->wave_info.block_align = (info.sample_rate * info.channels) / 8;
    }
    if (info.fmt == ES_STREAM_PCM)
    {
        p_head->wave_info.format_tag = 0x1;
        p_head->wave_info.bits_per_sample = 16;
        p_head->wave_info.block_align = 1024;
    }
    if (info.fmt == ES_STREAM_G726_16)
    {
        p_head->wave_info.format_tag = 0x45;
        p_head->wave_info.bits_per_sample = 2;
        p_head->wave_info.block_align = 2;
    }
    if (info.fmt == ES_STREAM_G726_24)
    {
        p_head->wave_info.format_tag = 0x45;
        p_head->wave_info.bits_per_sample = 3;
        p_head->wave_info.block_align = 3;
    }
    if (info.fmt == ES_STREAM_G726_32)
    {
        p_head->wave_info.format_tag = 0x45;
        p_head->wave_info.bits_per_sample = 4;
        p_head->wave_info.block_align = 4;
    }
    if (info.fmt == ES_STREAM_G726_40)
    {
        p_head->wave_info.format_tag = 0x45;
        p_head->wave_info.bits_per_sample = 5;
        p_head->wave_info.block_align = 5;
    }
    p_head->wave_info.channels = info.channels;
    p_head->wave_info.samples_per_sec = info.sample_rate;
    p_head->wave_info.avg_bytes_per_sec =
        (p_head->wave_info.bits_per_sample * info.sample_rate * info.channels) / 8;
    p_head->data[0] = 'd';
    p_head->data[1] = 'a';
    p_head->data[2] = 't';
    p_head->data[3] = 'a';

    p_head->data_len = p_head->data_len + info.packet_info[0].size;
    p_head->riff_len = p_head->data_len + sizeof(struct wave_file_header) - 8;
    return 0;
}

static void WriteEsAudioFile(FILE *fp, const es_audio_info &info, const es_audio_packet &data)
{
    for (unsigned int i = 0; i < info.packet_count; i++)
    {
        static wave_file_header wav_header;
        fill_wave_header(&wav_header, info);
        fseeko64(fp, 0, SEEK_SET);
        fwrite(&wav_header, 1, sizeof(struct wave_file_header), fp);
        fseeko64(fp, 0, SEEK_END);
        fwrite(data.packet_data[i].data, 1, info.packet_info[i].size, fp);
    }
}
static void ReadRawFile(FILE *fp, const raw_video_info &info, raw_video_packet &data)
{
    for (unsigned int i = 0; i < info.plane_num; ++i)
    {
        raw_video_packet::raw_data      raw   = data.plane_data[i];
        switch (info.plane_info[i].fmt)
        {
            case RAW_FORMAT_YUV422_YUYV:
            case RAW_FORMAT_YUV422_UYVY:
            case RAW_FORMAT_YUV422_YVYU:
            case RAW_FORMAT_YUV422_VYUY:
            case RAW_FORMAT_RGB888:
            case RAW_FORMAT_BGR888:
            case RAW_FORMAT_ARGB8888:
            case RAW_FORMAT_ABGR8888:
            case RAW_FORMAT_BGRA8888:
            case RAW_FORMAT_RGB565:
            case RAW_FORMAT_ARGB1555:
            case RAW_FORMAT_ARGB4444:
            case RAW_FORMAT_I2:
            case RAW_FORMAT_I4:
            case RAW_FORMAT_I8:
            case RAW_FORMAT_BAYER_BASE ... RAW_FORMAT_BAYER_NUM:
            {
                fseeko64(fp, 0, SEEK_CUR);
                long long curr = ftello64(fp);
                fseeko64(fp, 0, SEEK_END);
                long long end  = ftello64(fp);
                if (end - curr < (long)raw.size[0])
                {
                    curr = 0;
                }
                fseeko64(fp, curr, SEEK_SET);
                if (raw.size[0] != (unsigned int)fread(raw.data[0], 1, raw.size[0], fp))
                {
                    AMIGOS_ERR("Bad file,\n");
                }
            }
            break;
            case RAW_FORMAT_YUV420SP:
            case RAW_FORMAT_YUV420SP_NV21:
            {
                fseeko64(fp, 0, SEEK_CUR);
                long long curr = ftello64(fp);
                fseeko64(fp, 0, SEEK_END);
                long long end = ftello64(fp);
                if (end - curr < (long)(raw.size[0] + raw.size[1]))
                {
                    curr = 0;
                }
                fseeko64(fp, curr, SEEK_SET);
                if (raw.size[0] != (unsigned int)fread(raw.data[0], 1, raw.size[0], fp)
                    || raw.size[1] != (unsigned int)fread(raw.data[1], 1, raw.size[1], fp))
                {
                    AMIGOS_ERR("Bad file,\n");
                }
            }
            default:
                break;
        }
    }
}

static bool CheckMetadataFile(FILE *fp, meta_data_info &info)
{
    unsigned char header[16] = {0};
    fseeko64(fp, 0, SEEK_CUR);
    long long curr = ftello64(fp);
    fseeko64(fp, 0, SEEK_END);
    long long end = ftello64(fp);

    memset(header, 0, 16);
    if (end - curr < 16)
    {
        curr = 0;
    }
    fseeko64(fp, curr, SEEK_SET);
    if (16 != fread(header, 1, 16, fp))
    {
        AMIGOS_ERR("Bad file,\n");
        return false;
    }
    if (header[0] != 'M'  || header[1] !='e'  || header[2] != 't'  || header[3] !='a')
    {
        AMIGOS_ERR("Metadata File Header error!\n");
        return false;
    }
    info.size = U32VALUE(header, 4);
    return true;
}

static void ReadMetadataFile(FILE *fp, const meta_data_info &info, meta_data_packet &data)
{
    if (info.size != (unsigned int)fread(data.data, 1, info.size, fp))
    {
        AMIGOS_ERR("Bad file, sz %d address %p\n", info.size, data.data);
    }
}

static bool CheckEsVideoFile(FILE *fp, es_video_info &info, ss_video_packet_filter &video_filter)
{
    unsigned char header[16] = {0};
    fseeko64(fp, 0, SEEK_CUR);
    long long curr = ftello64(fp);
    fseeko64(fp, 0, SEEK_END);
    long long end = ftello64(fp);
    memset(header, 0, 16);
    if (end - curr < 16)
    {
        curr = 0;
        video_filter.reset();
    }
    fseeko64(fp, curr, SEEK_SET);
    if (16 != (int)fread(header, 1, 16, fp))
    {
        AMIGOS_ERR("Bad file,\n");
        return false;
    }
    if (*(unsigned int *)header != 0x1)
    {
        AMIGOS_ERR("ES File Header error!\n");
        return false;
    }
    info.packet_count         = 1;
    info.packet_info[0].b_end = true;
    info.packet_info[0].size  = U32VALUE(header, 4);
    return true;
}
static void ReadEsVideoFile(FILE *fp, const es_video_info &info, const es_video_packet &data)
{
    for (unsigned int i = 0; i < info.packet_count; ++i)
    {
        if (info.packet_info[i].size != (unsigned int)fread(data.packet_data[i].data, 1, info.packet_info[i].size, fp))
        {
            AMIGOS_ERR("Bad file, sz %d address %p\n", info.packet_info[i].size, data.packet_data[i].data);
            fseeko64(fp, 0, SEEK_SET);
            break;
        }
    }
}
static unsigned int ReadEsAudioFile(FILE *fp, bool b_skip_wav_header, const es_audio_info &info, es_audio_packet &data)
{
    unsigned int read_count = 0;
    for (unsigned int i = 0; i < info.packet_count; i++)
    {
        read_count = (unsigned int)fread(data.packet_data[i].data, 1, info.packet_info[i].size, fp);
        if (info.packet_info[i].size != read_count)
        {
            fseeko64(fp, (b_skip_wav_header) ? sizeof(struct wave_file_header): 0, SEEK_SET);
            break;
        }
    }
    return read_count;
}

static void *RawFileReader(struct ss_thread_buffer *thread_buf)
{
    struct AmigosModuleFile::FileReaderDesc *pFrdDesc
        = (struct AmigosModuleFile::FileReaderDesc *)thread_buf->buf;

    assert(pFrdDesc->info.en_type == EN_RAW_FRAME_DATA);
    if (pFrdDesc->frameCntLimit == 0)
    {
        AMIGOS_INFO("RawFile readcnt reach frameCntLimit [finish......]\n");
        ss_thread_stop(pFrdDesc->threadHandle);
        AMIGOS_INFO("File reader [%s] stop\n", pFrdDesc->threadHandle);
        return 0;
    }
    if (pFrdDesc->frameCntLimit > 0)
    {
        pFrdDesc->frameCntLimit--;
    }
    auto packet = pFrdDesc->packer->make(pFrdDesc->info);
    if (packet == nullptr)
    {
        return NULL;
    }
    ReadRawFile(pFrdDesc->readFp, packet->raw_vid_i, packet->raw_vid);
    pFrdDesc->linkerOut->enqueue(packet);
    return NULL;
}
static void *MetadataFileReader(struct ss_thread_buffer *thread_buf)
{
    struct AmigosModuleFile::FileReaderDesc *pFrdDesc
    = (struct AmigosModuleFile::FileReaderDesc *)thread_buf->buf;

    assert(pFrdDesc->info.en_type == EN_USER_META_DATA);
    if (pFrdDesc->frameCntLimit == 0)
    {
        AMIGOS_INFO("MetadataFile readcnt reach frameCntLimit [finish......]\n");
        ss_thread_stop(pFrdDesc->threadHandle);
        AMIGOS_INFO("File reader [%s] stop\n", pFrdDesc->threadHandle);
        return 0;
    }
    if (pFrdDesc->frameCntLimit > 0)
    {
        pFrdDesc->frameCntLimit--;
    }
    if (!CheckMetadataFile(pFrdDesc->readFp, pFrdDesc->info.meta_data_i))
    {
        AMIGOS_ERR("Check fail\n");
        return NULL;
    }
    auto packet = pFrdDesc->packer->make(pFrdDesc->info);
    if (packet == nullptr)
    {
        return NULL;
    }
    ReadMetadataFile(pFrdDesc->readFp, packet->meta_data_i, packet->meta_data);
    pFrdDesc->linkerOut->enqueue(packet);
    return NULL;
}

static unsigned int EsAudioLoopFileSize(struct AmigosModuleFile::FileReaderDesc *pFrdDesc)
{
    struct timespec cur_time;
    unsigned int uCycleMs = 10;
    unsigned int retSize  = 0;
    unsigned int curSec   = 0;
    unsigned int curNsec  = 0;

    clock_gettime(CLOCK_MONOTONIC, &cur_time);
    /*Value < 1000000000, 32bit can handle this.*/
    curSec  = cur_time.tv_sec;
    curNsec = cur_time.tv_nsec;
    if (pFrdDesc->lastTimeNsec || pFrdDesc->lastTimeSec)
    {
        unsigned int time_ns = 0;
        time_ns = (curSec - pFrdDesc->lastTimeSec) * 1000000000 + curNsec - pFrdDesc->lastTimeNsec;
        uCycleMs = time_ns / 10000000 * 10;
        pFrdDesc->gapTimeNs += (time_ns % 10000000);
        if (pFrdDesc->gapTimeNs >= 10000000)
        {
            pFrdDesc->gapTimeNs -= 10000000;
            uCycleMs += 10;
        }
        // std::cout << "TimeNs: " << curSec << ","<< curNsec
        //          << " DiffNs: " << std::dec << time_ns << " GapNs: "<< pFrdDesc->gapTimeNs << " DataMs: " << uCycleMs << std::endl;
    }
    pFrdDesc->lastTimeSec = cur_time.tv_sec;
    pFrdDesc->lastTimeNsec = cur_time.tv_nsec;
    retSize  = pFrdDesc->info.es_aud_i.sample_rate * pFrdDesc->info.es_aud_i.channels
        * ((pFrdDesc->info.es_aud_i.sample_width * 120) / 8) * uCycleMs / 120000;
    pFrdDesc->gapDataSize += retSize % (PACKET_SIZE_ALIGNMENT_BYTE);
    /* Audio data algogrithem only accept byte alignment of pcm data.*/
    retSize = retSize / (PACKET_SIZE_ALIGNMENT_BYTE) * (PACKET_SIZE_ALIGNMENT_BYTE);
    if (pFrdDesc->gapDataSize >= PACKET_SIZE_ALIGNMENT_BYTE)
    {
        pFrdDesc->gapDataSize -= PACKET_SIZE_ALIGNMENT_BYTE;
        retSize += PACKET_SIZE_ALIGNMENT_BYTE;
    }
    return retSize;
}

static void *EsAudioFileReader(struct ss_thread_buffer *thread_buf)
{
    struct AmigosModuleFile::FileReaderDesc *pFrdDesc
        = (struct AmigosModuleFile::FileReaderDesc *)thread_buf->buf;
    unsigned int size = 0;

    assert(pFrdDesc->info.en_type == EN_AUDIO_CODEC_DATA);
    if (pFrdDesc->frameCntLimit == 0)
    {
        AMIGOS_INFO("EsAudioFile readcnt reach frameCntLimit [finish......]\n");
        ss_thread_stop(pFrdDesc->threadHandle);
        AMIGOS_INFO("File reader [%s] stop\n", pFrdDesc->threadHandle);
        return 0;
    }
    if (pFrdDesc->frameCntLimit > 0)
    {
        pFrdDesc->frameCntLimit--;
    }
    size = EsAudioLoopFileSize(pFrdDesc);
    if (!size)
    {
        return NULL;
    }
    pFrdDesc->info.es_aud_i.packet_info[0].size = size;
    auto packet = pFrdDesc->packer->make(pFrdDesc->info);
    if (!packet)
    {
        return NULL;
    }
    if(ReadEsAudioFile(pFrdDesc->readFp, true, packet->es_aud_i, packet->es_aud) == 0)
    {
        return NULL;
    }
    pFrdDesc->linkerOut->enqueue(packet);
    return NULL;
}

static void *EsVideoFileReader(struct ss_thread_buffer *thread_buf)
{
    struct AmigosModuleFile::FileReaderDesc *pFrdDesc
        = (struct AmigosModuleFile::FileReaderDesc *)thread_buf->buf;

    assert(pFrdDesc->info.en_type == EN_VIDEO_CODEC_DATA);
    if (pFrdDesc->frameCntLimit == 0)
    {
        AMIGOS_INFO("EsVideoFile readcnt reach frameCntLimit [finish......]\n");
        ss_thread_stop(pFrdDesc->threadHandle);
        AMIGOS_INFO("File reader [%s] stop\n", pFrdDesc->threadHandle);
        return 0;
    }
    if (pFrdDesc->frameCntLimit > 0)
    {
        pFrdDesc->frameCntLimit--;
    }
    if (!pFrdDesc->packet)
    {
        if (!CheckEsVideoFile(pFrdDesc->readFp, pFrdDesc->info.es_vid_i, pFrdDesc->video_filter))
        {
            AMIGOS_ERR("Check fail\n");
            return NULL;
        }
        pFrdDesc->packet = pFrdDesc->packer->make(pFrdDesc->info);
        if (!pFrdDesc->packet)
        {
            fseeko64(pFrdDesc->readFp, 0, SEEK_CUR);
            long long curr = ftello64(pFrdDesc->readFp);
            if (curr >= 16)
            {
                fseeko64(pFrdDesc->readFp, curr - 16, SEEK_SET);
            }
            return NULL;
        }
        ReadEsVideoFile(pFrdDesc->readFp, pFrdDesc->packet->es_vid_i, pFrdDesc->packet->es_vid);
    }
    if (!pFrdDesc->video_filter.check(pFrdDesc->packet))
    {
        pFrdDesc->packet = nullptr;
        return NULL;
    }
    if (0 == pFrdDesc->linkerOut->enqueue(pFrdDesc->packet))
    {
        pFrdDesc->packet = nullptr;
        return NULL;
    }
    usleep(1000 * 10);
    return NULL;
}
int AmigosModuleFile::_EnqueueOut(unsigned int outPortId, stream_packet_obj &packet)
{
    return -1;
}
int AmigosModuleFile::_DequeueOut(unsigned int outPortId, stream_packet_obj &packet)
{
    return -1;
}
stream_packet_obj AmigosModuleFile::_DequeueFromInside(unsigned int outPortId, unsigned int ms)
{
    auto iterPortOut = this->mapPortOut.find(outPortId);
    if (iterPortOut == this->mapPortOut.end())
    {
        AMIGOS_ERR("Port %d has not base sturcture.\n", outPortId);
        return nullptr;
    }
    return _KickPacket(&iterPortOut->second.outPacker, outPortId);
}
stream_packet_obj AmigosModuleFile::_KickPacket(stream_packer *packer, unsigned int outPortId)
{
    auto iter = mapFileReaderDesc.find(outPortId);
    if (iter == mapFileReaderDesc.end())
    {
        AMIGOS_ERR("Reader fail\n");
        return nullptr;
    }
    struct FileReaderDesc &fileOutDesc = iter->second;
    if (fileOutDesc.frameCntLimit == 0)
    {
        AMIGOS_INFO("FILE readcnt reach frameCntLimit [finish......]\n");
        return 0;
    }
    if (fileOutDesc.frameCntLimit > 0)
    {
        fileOutDesc.frameCntLimit--;
    }
    auto type = fileOutDesc.info.en_type;
    if (type == EN_VIDEO_CODEC_DATA)
    {
        if (!CheckEsVideoFile(fileOutDesc.readFp, fileOutDesc.info.es_vid_i, fileOutDesc.video_filter))
        {
            AMIGOS_ERR("Check fail\n");
            return nullptr;
        }
        auto packet = packer->make(fileOutDesc.info);
        if (packet == nullptr)
        {
            fseeko64(fileOutDesc.readFp, 0, SEEK_CUR);
            long long curr = ftello64(fileOutDesc.readFp);
            if (curr >= 16)
            {
                fseeko64(fileOutDesc.readFp, curr - 16, SEEK_SET);
            }
            return nullptr;
        }
        ReadEsVideoFile(fileOutDesc.readFp, packet->es_vid_i, packet->es_vid);
        if (!fileOutDesc.video_filter.check(packet))
        {
            packet = nullptr;
            return packet;
        }
        return packet;
    }
    if (type == EN_AUDIO_CODEC_DATA)
    {
        unsigned int size = 0;
        size = EsAudioLoopFileSize(&fileOutDesc);
        if (!size)
        {
            return nullptr;
        }
        fileOutDesc.info.es_aud_i.packet_info[0].size = size;
        auto packet = packer->make(fileOutDesc.info);
        if (packet == nullptr)
        {
            AMIGOS_ERR("Get buf fail\n");
            return nullptr;
        }
        if(ReadEsAudioFile(fileOutDesc.readFp, true, packet->es_aud_i, packet->es_aud) == 0)
        {
            return nullptr;
        }
        return packet;
    }
    if (type == EN_RAW_FRAME_DATA)
    {
        auto packet = packer->make(fileOutDesc.info);
        if (packet == nullptr)
        {
            AMIGOS_ERR("Get buf fail\n");
            return nullptr;
        }
        ReadRawFile(fileOutDesc.readFp, packet->raw_vid_i, packet->raw_vid);
        return packet;
    }
    if (type == EN_USER_META_DATA)
    {
        if (!CheckMetadataFile(fileOutDesc.readFp, fileOutDesc.info.meta_data_i))
        {
            AMIGOS_ERR("Check fail\n");
            return NULL;
        }
        auto packet = packer->make(fileOutDesc.info);
        if (packet == nullptr)
        {
            AMIGOS_ERR("Get meta_data buf fail\n");
            return nullptr;
        }
        ReadMetadataFile(fileOutDesc.readFp, packet->meta_data_i, packet->meta_data);
        return packet;
    }
    AMIGOS_ERR("Not support fmt %d\n", fileOutDesc.info.en_type);
    return nullptr;
}
AMIGOS_MODULE_INIT("FILE", AmigosModuleFile);

