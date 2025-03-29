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

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <cassert>
#include <vector>
#include <map>
#include "ss_connector.h"
#include "ss_packet.h"
#include "ss_thread.h"
#include "ss_cmd_base.h"
#include "ss_ring_connector.h"

#define NALU_PACKET_SIZE            256*1024
#define PACKET_SIZE_ALIGNMENT_BYTE 1024
#define MI_U32VALUE(pu8Data, index) (pu8Data[index]<<24)|(pu8Data[index+1]<<16)|(pu8Data[index+2]<<8)|(pu8Data[index+3])

static int create_system_ring_connector(vector<string> &in_strs)
{
    const string &handle = in_strs[1];
    int size = ss_cmd_atoi(in_strs[2].c_str());

    ss_ring_connector *connector = new ss_ring_connector(size);
    ss_handle::install(handle, connector);
    return 0;
}
static int destroy_system_ring_connector(vector<string> &in_strs)
{
    const string &handle = in_strs[1];

    ss_ring_connector *connector = dynamic_cast <ss_ring_connector *>(ss_ring_connector::get(handle));
    if (!connector)
    {
        ss_print(PRINT_COLOR_RED, PRINT_MODE_HIGHTLIGHT, "Connector handle error!\n");
        return -1;
    }
    if (ss_handle::destroy(handle) == -1)
    {
        ss_print(PRINT_COLOR_RED, PRINT_MODE_HIGHTLIGHT, "Connector destroy error!\n");
        return -1;
    }
    return 0;
}
struct wave_format
{
    signed short format_tag;
    signed short channels;
    unsigned int samples_per_sec;
    unsigned int avg_bytes_per_sec;
    signed short block_align;
    signed short bits_per_sample;
};
struct wave_file_header
{
    char riff[4];
    unsigned int riff_len;
    char wave[4];
    char fmt[4];
    unsigned int fmt_len;
    struct wave_format wave_info;
    char data[4];
    unsigned int data_len;
};
struct file_reader_desc
{
    int read_fd;
    char file_path[256];
    bool b_play_loop;
    void *thread_handle;
    long long offset;
    ss_connector *connector;
    ss_thread_monitor do_monitor;
    int monitor_fps;
    int monitor_delay_ms;
    unsigned int last_time_sec;
    unsigned int last_time_nsec;
    unsigned int gap_time_ns;
    unsigned int gap_data_size;
    enum stream_type en_type;
    union
    {
        struct es_audio_info es_aud_i;
        struct es_video_info es_vid_i;
        struct raw_video_info raw_vid_i;
    };
};

struct file_writer_desc
{
    int write_fd;
    int b_header;
    int grab_time_out;
    void *thread_handle;
    ss_connector *connector;
};

static std::map<std::string, struct file_reader_desc> map_reader;

static int fill_wave_header(struct wave_file_header *p_head, unsigned int sample_rate, unsigned int channels, unsigned int size)
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

    p_head->wave_info.channels = channels;
    p_head->wave_info.format_tag = 0x1;
    p_head->wave_info.bits_per_sample = 16; /* 16bit */
    p_head->wave_info.samples_per_sec = sample_rate;
    p_head->wave_info.avg_bytes_per_sec =
        (p_head->wave_info.bits_per_sample * sample_rate * channels) / 8;
    p_head->wave_info.block_align = 1024;
    p_head->data[0] = 'd';
    p_head->data[1] = 'a';
    p_head->data[2] = 't';
    p_head->data[3] = 'a';

    p_head->data_len = p_head->data_len + size;
    p_head->riff_len = p_head->data_len + sizeof(struct wave_file_header) - 8;
    return 0;
}
static void *video_es_file_reader(struct ss_thread_buffer *thread_buf)
{
    struct file_reader_desc *p_desc = NULL;
    stream_packet_info es_packet_info;
    unsigned char header[16] = {0};
    unsigned int header_check = 0;
    int ret = 0;

    assert(thread_buf);
    assert(thread_buf->size == sizeof(struct file_reader_desc));
    p_desc = (struct file_reader_desc *)thread_buf->buf;
    assert(p_desc);
    assert(p_desc->connector);
    if (p_desc->connector->approch() == -1)
    {
        return NULL;
    }
    memset(header, 0, 16);
    //u32Pos = lseek(readfp, 0, SEEK_CUR);
    ret = read(p_desc->read_fd, header, 16);
    if (ret <= 0)
    {
        if (p_desc->b_play_loop)
        {
            lseek(p_desc->read_fd, 0, SEEK_SET);
        }
        return NULL;
    }
    header_check = *(unsigned int*)header;
    if (header_check != 0x1)
    {
        cout << "FILE: " << p_desc->file_path << " ES Header error!" << endl;
        return NULL;
    }
    es_packet_info.en_type                       = p_desc->en_type;
    es_packet_info.es_vid_i.b_head               = false;
    es_packet_info.es_vid_i                      = p_desc->es_vid_i;
    es_packet_info.es_vid_i.packet_count         = 1;
    es_packet_info.es_vid_i.packet_info[0].b_end = true;
    es_packet_info.es_vid_i.packet_info[0].size  = MI_U32VALUE(header, 4);
    auto es_packet = p_desc->connector->getbuf(es_packet_info);
    if (es_packet == nullptr)
    {
        lseek(p_desc->read_fd, -16, SEEK_CUR);
        return NULL;
    }
    assert(es_packet->es_vid.packet_data[0].data);
    assert(es_packet->es_vid_i.packet_info[0].size);
    ret = read(p_desc->read_fd, es_packet->es_vid.packet_data[0].data, es_packet->es_vid_i.packet_info[0].size);
    if (ret <= 0)
    {
        if (p_desc->b_play_loop)
        {
            lseek(p_desc->read_fd, 0, SEEK_SET);
        }
    }
    p_desc->connector->update(es_packet);

    return NULL;
}

static void read_raw_file(int fd, const raw_video_info &info, raw_video_packet &data)
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
                off_t curr = lseek(fd, 0, SEEK_CUR);
                off_t end  = lseek(fd, 0, SEEK_END);
                if (end - curr < (off_t)raw.size[0])
                {
                    curr = 0;
                }
                lseek(fd, curr, SEEK_SET);
                if (raw.size[0] != (unsigned int)read(fd, raw.data[0], raw.size[0]))
                {
                    ss_print(PRINT_COLOR_RED, PRINT_MODE_HIGHTLIGHT, "[%d] Bad file!\n", __LINE__);
                }
            }
            break;
            case RAW_FORMAT_YUV420SP:
            case RAW_FORMAT_YUV420SP_NV21:
            {
                off_t curr = lseek(fd, 0, SEEK_CUR);
                off_t end  = lseek(fd, 0, SEEK_END);
                if (end - curr < (off_t)(raw.size[0] + raw.size[1]))
                {
                    curr = 0;
                }
                lseek(fd, curr, SEEK_SET);
                if (raw.size[0] != (unsigned int)read(fd, raw.data[0], raw.size[0])
                    || raw.size[1] != (unsigned int)read(fd, raw.data[1], raw.size[1]))
                {
                    ss_print(PRINT_COLOR_RED, PRINT_MODE_HIGHTLIGHT, "[%d] Bad file!\n", __LINE__);
                }
            }
            default:
                break;
        }
    }
}
static void *yuv_file_reader(struct ss_thread_buffer *thread_buf)
{
    struct file_reader_desc *p_desc = NULL;
    stream_packet_info yuv_packet_info{};

    assert(thread_buf);
    assert(thread_buf->size == sizeof(struct file_reader_desc));
    p_desc = (struct file_reader_desc *)thread_buf->buf;
    assert(p_desc);
    assert(p_desc->connector);
    if (p_desc->connector->approch() == -1)
    {
        cout << "YUV File APPROCH Error, Retry layer" << endl;
        return NULL;
    }

    if (p_desc->en_type != EN_RAW_FRAME_DATA)
    {
        return NULL;
    }

    yuv_packet_info.en_type   = p_desc->en_type;
    yuv_packet_info.raw_vid_i = p_desc->raw_vid_i;

    auto yuv_packet = p_desc->connector->getbuf(yuv_packet_info);
    if (yuv_packet == nullptr)
    {
        return NULL;
    }
    read_raw_file(p_desc->read_fd, yuv_packet->raw_vid_i, yuv_packet->raw_vid);
    p_desc->connector->update(yuv_packet);

    return NULL;
}
static unsigned int es_audio_loop_file_size(struct file_reader_desc *p_desc)
{
    struct timespec cur_time;
    unsigned int cycle_ms = 10;
    unsigned int ret_size  = 0;
    unsigned int cur_sec   = 0;
    unsigned int cur_nsec  = 0;

    clock_gettime(CLOCK_MONOTONIC, &cur_time);
    /*Value < 1000000000, 32bit can handle this.*/
    cur_sec  = cur_time.tv_sec;
    cur_nsec = cur_time.tv_nsec;
    if (p_desc->last_time_nsec || p_desc->last_time_sec)
    {
        unsigned int time_ns = 0;
        time_ns = (cur_sec - p_desc->last_time_sec) * 1000000000 + cur_nsec - p_desc->last_time_nsec;
        cycle_ms = time_ns / 10000000 * 10;
        p_desc->gap_time_ns += (time_ns % 10000000);
        if (p_desc->gap_time_ns >= 10000000)
        {
            p_desc->gap_time_ns -= 10000000;
            cycle_ms += 10;
        }
        // std::cout << "TimeNs: " << curSec << ","<< curNsec
        //          << " DiffNs: " << std::dec << time_ns << " GapNs: "<< pFrdDesc->gapTimeNs << " DataMs: " << uCycleMs << std::endl;
    }
    p_desc->last_time_sec = cur_time.tv_sec;
    p_desc->last_time_nsec = cur_time.tv_nsec;
    ret_size  = p_desc->es_aud_i.sample_rate * p_desc->es_aud_i.channels
        * ((p_desc->es_aud_i.sample_width * 120) / 8) * cycle_ms / 120000;
    p_desc->gap_data_size += ret_size % (PACKET_SIZE_ALIGNMENT_BYTE);
    /* Audio data algogrithem only accept byte alignment of pcm data.*/
    ret_size = ret_size / (PACKET_SIZE_ALIGNMENT_BYTE) * (PACKET_SIZE_ALIGNMENT_BYTE);
    if (p_desc->gap_data_size >= PACKET_SIZE_ALIGNMENT_BYTE)
    {
        p_desc->gap_data_size -= PACKET_SIZE_ALIGNMENT_BYTE;
        ret_size += PACKET_SIZE_ALIGNMENT_BYTE;
    }
    return ret_size;
}

static void * es_audio_file_reader(struct ss_thread_buffer *thread_buf)
{
    struct file_reader_desc *p_desc = NULL;
    stream_packet_info es_packet_info;
    int ret = 0;
    int size;

    assert(thread_buf);
    assert(thread_buf->size == sizeof(struct file_reader_desc));
    p_desc = (struct file_reader_desc *)thread_buf->buf;
    assert(p_desc);

    es_packet_info.en_type               = p_desc->en_type;
    es_packet_info.es_aud_i              = p_desc->es_aud_i;
    es_packet_info.es_aud_i.packet_count = 1;
    size = es_audio_loop_file_size(p_desc);
    if (!size)
    {
        return NULL;
    }
    es_packet_info.es_aud_i.packet_info[0].size = size;
    assert(p_desc->connector);
    auto es_packet = p_desc->connector->getbuf(es_packet_info);
    if (es_packet == nullptr)
    {
        ss_print(PRINT_COLOR_RED, PRINT_MODE_HIGHTLIGHT, "[%d] Getbuf error!\n", __LINE__);
        return NULL;
    }
    assert(es_packet->es_vid.packet_data[0].data);
    assert(es_packet->es_vid_i.packet_info[0].size);
    ret = read(p_desc->read_fd, es_packet->es_vid.packet_data[0].data, es_packet->es_vid_i.packet_info[0].size);
    if (ret != (int)es_packet->es_vid_i.packet_info[0].size)
    {
        if ((p_desc->b_play_loop) && (p_desc->es_aud_i.fmt == ES_STREAM_WAV))
        {
            lseek(p_desc->read_fd, sizeof(struct wave_file_header), SEEK_SET);
        }
        else if ((p_desc->b_play_loop) && (p_desc->es_aud_i.fmt == ES_STREAM_PCM))
        {
            lseek(p_desc->read_fd, 0, SEEK_SET);
        }
        else
        {
            ss_print(PRINT_COLOR_RED, PRINT_MODE_HIGHTLIGHT, "[%d] The file has no data!\n", __LINE__);
            return NULL;
        }
        read(p_desc->read_fd, es_packet->es_vid.packet_data[0].data, es_packet->es_vid_i.packet_info[0].size);
    }
    p_desc->connector->update(es_packet);

    return NULL;
}
static void *video_es_file_writer_thread(struct ss_thread_buffer *thread_buf)
{
    struct file_writer_desc *p_w_desc = NULL;

    assert(thread_buf);
    assert(thread_buf->size == sizeof(struct file_writer_desc));
    p_w_desc = (struct file_writer_desc *)thread_buf->buf;
    assert(p_w_desc);
    auto packet = p_w_desc->connector->grab(p_w_desc->grab_time_out);
    if (packet == nullptr)
    {
        cout << "Grab time out!" << endl;
        return NULL;
    }
    if (packet->en_type != EN_VIDEO_CODEC_DATA)
    {
        cout << "Not es stream!" << endl;
        p_w_desc->connector->back(packet);
        return NULL;
    }
    if (p_w_desc->b_header)
    {
        if (!packet->es_vid_i.b_head)
        {
            unsigned char header[16];
            unsigned int data_size = 0;
            for (unsigned int i = 0; i < packet->es_vid_i.packet_count; i++)
            {
                data_size += packet->es_vid_i.packet_info[i].size;
            }
            memset(header, 0, 16);
            header[0] = 0x1;
            header[4] = ((data_size) >> 24) & 0xFF;
            header[5] = ((data_size) >> 16) & 0xFF;
            header[6] = ((data_size) >> 8)& 0xFF;
            header[7] = (data_size) & 0xFF;
            write(p_w_desc->write_fd, header, 16);
        }
    }
    for (unsigned int i = 0; i < packet->es_vid_i.packet_count; i++)
    {
        write(p_w_desc->write_fd, packet->es_vid.packet_data[i].data, packet->es_vid_i.packet_info[i].size);
    }
    p_w_desc->connector->back(packet);
    return NULL;
}
static int connect_file_reader(void *user, unsigned int ref)
{
    if (ref > 0)
    {
        return 0;
    }
    struct file_reader_desc *p_desc = (struct file_reader_desc *)user;
    assert(p_desc);
    assert(p_desc->connector);
    const string &handle = p_desc->connector->get_handle();
    printf("[CONNECTOR]: CONNECT!! handle : %s\n", handle.c_str());
    p_desc->read_fd = open(p_desc->file_path, O_RDONLY);
    if (p_desc->read_fd < 0)
    {
        printf("File: %s open error!\n", p_desc->file_path);
        return -1;
    }
    if (p_desc->en_type == EN_AUDIO_CODEC_DATA && p_desc->es_aud_i.fmt == ES_STREAM_WAV)
    {
        struct wave_file_header wav_header;
        memset(&wav_header, 0, sizeof(struct wave_file_header));
        read(p_desc->read_fd, &wav_header, sizeof(struct wave_file_header));
        p_desc->es_aud_i.sample_rate = wav_header.wave_info.samples_per_sec;
        p_desc->es_aud_i.channels    = wav_header.wave_info.channels;
    }
    struct ss_thread_attr ss_attr;
    memset(&ss_attr, 0, sizeof(struct ss_thread_attr));
    ss_attr.do_signal          = NULL;
    ss_attr.do_monitor         = p_desc->do_monitor;
    ss_attr.is_reset_timer     = 0;
    ss_attr.in_buf.buf         = (void *)(p_desc);
    ss_attr.in_buf.size        = sizeof(struct file_reader_desc);
    ss_attr.monitor_cycle_sec  = 0;
    ss_attr.monitor_cycle_nsec = (p_desc->monitor_fps ? 1000000000 / p_desc->monitor_fps :
        (p_desc->monitor_delay_ms ? p_desc->monitor_delay_ms * 1000000 : 33333333));
    snprintf(ss_attr.thread_name, 32, "file_reader%s", handle.c_str());
    p_desc->thread_handle = ss_thread_open(&ss_attr);
    if (!p_desc->thread_handle)
    {
        printf("Monitor return error!\n");
        close(p_desc->read_fd);
        return -1;
    }
    if (p_desc->offset)
    {
        lseek(p_desc->read_fd, p_desc->offset, SEEK_SET);
    }
    return ss_thread_start_monitor(p_desc->thread_handle);
}
static int disconnect_file_reader(void *user, unsigned int ref)
{
    if (ref > 0)
    {
        return 0;
    }
    struct file_reader_desc *p_desc = (struct file_reader_desc *)user;
    assert(p_desc);
    assert(p_desc->connector);
    const string &handle = p_desc->connector->get_handle();
    printf("[CONNECTOR]: DISCONNECT!! handle : %s\n", handle.c_str());
    if(!p_desc->thread_handle)
    {
        printf("Handle %s thread_handle error!\n", handle.c_str());
        return -1;
    }
    ss_thread_stop(p_desc->thread_handle);
    int ret = ss_thread_close(p_desc->thread_handle);
    if (ret == -1)
    {
        printf("Handle %s close error!\n", handle.c_str());
        return -1;
    }
    p_desc->offset = lseek(p_desc->read_fd, 0, SEEK_CUR);
    return close(p_desc->read_fd);
}
static int create_video_es_file_connector_reader(vector<string> &in_strs)
{
    struct file_reader_desc desc;
    const string &handle = in_strs[1];
    if (access(in_strs[2].c_str(), F_OK) == -1)
    {
        ss_print(PRINT_COLOR_RED, PRINT_MODE_HIGHTLIGHT, "File: %s assess error!\n", in_strs[2].c_str());
        return -1;
    }
    memset(&desc, 0, sizeof(struct file_reader_desc));
    desc.connector = dynamic_cast <ss_connector *>(ss_connector::get(handle));
    if (!desc.connector)
    {
        ss_print(PRINT_COLOR_RED, PRINT_MODE_HIGHTLIGHT, "Connector handle error!\n");
        return -1;
    }
    desc.en_type = EN_VIDEO_CODEC_DATA;
    if (in_strs[4] == "h264")
        desc.es_vid_i.fmt = ES_STREAM_H264;
    else if (in_strs[4] == "h265")
        desc.es_vid_i.fmt = ES_STREAM_H265;
    else if (in_strs[4] == "jpeg")
        desc.es_vid_i.fmt = ES_STREAM_JPEG;
    else
    {
        ss_print(PRINT_COLOR_RED, PRINT_MODE_HIGHTLIGHT, "Format: %s error!\n", in_strs[4].c_str());
        return -1;
    }
    snprintf(desc.file_path, 256, "%s", in_strs[2].c_str());
    desc.monitor_fps     = ss_cmd_atoi(in_strs[3].c_str());
    desc.es_vid_i.width  = ss_cmd_atoi(in_strs[5].c_str());
    desc.es_vid_i.height = ss_cmd_atoi(in_strs[6].c_str());
    desc.b_play_loop     = ss_cmd_atoi(in_strs[7].c_str());
    desc.do_monitor      = video_es_file_reader;
    map_reader[handle]   = desc;
    desc.connector->connect_in(connect_file_reader, disconnect_file_reader,
                               (void *)(&map_reader[handle]));
    return 0;
}
static int create_video_yuv_file_connector_reader(vector<string> &in_strs)
{
    struct file_reader_desc desc;
    const string &handle = in_strs[1];
    if (access(in_strs[2].c_str(), F_OK) == -1)
    {
        ss_print(PRINT_COLOR_RED, PRINT_MODE_HIGHTLIGHT, "File: %s assess error!\n", in_strs[2].c_str());
        return -1;
    }
    memset(&desc, 0, sizeof(struct file_reader_desc));
    desc.connector = dynamic_cast <ss_connector *>(ss_connector::get(handle));
    if (!desc.connector)
    {
        ss_print(PRINT_COLOR_RED, PRINT_MODE_HIGHTLIGHT, "Connector handle error!\n");
        close(desc.read_fd);
        return -1;
    }
    desc.en_type = EN_RAW_FRAME_DATA;
    desc.raw_vid_i.plane_num = 1;
    if (in_strs[4] == "yuv420")
    {
        desc.raw_vid_i.plane_info[0].fmt = RAW_FORMAT_YUV420SP;
    }
    else if (in_strs[4] == "yuv422")
    {
        desc.raw_vid_i.plane_info[0].fmt = RAW_FORMAT_YUV422_YUYV;
    }
    else
    {
        ss_print(PRINT_COLOR_RED, PRINT_MODE_HIGHTLIGHT, "Format: %s error!\n", in_strs[4].c_str());
        return -1;
    }
    snprintf(desc.file_path, 256, "%s", in_strs[2].c_str());
    desc.monitor_fps                            = ss_cmd_atoi(in_strs[3].c_str());
    desc.raw_vid_i.plane_info[0].width  = ss_cmd_atoi(in_strs[5].c_str());
    desc.raw_vid_i.plane_info[0].height = ss_cmd_atoi(in_strs[6].c_str());
    desc.b_play_loop                    = ss_cmd_atoi(in_strs[7].c_str());
    desc.do_monitor                     = yuv_file_reader;
    map_reader[handle]                  = desc;
    desc.connector->connect_in(connect_file_reader, disconnect_file_reader,
                               (void *)(&map_reader[handle]));
    return 0;
}
static int create_audio_es_file_connector_reader(vector<string> &in_strs)
{
    struct file_reader_desc desc;
    const string &handle = in_strs[1];
    if (access(in_strs[2].c_str(), F_OK) == -1)
    {
        ss_print(PRINT_COLOR_RED, PRINT_MODE_HIGHTLIGHT, "File: %s assess error!\n", in_strs[2].c_str());
        return -1;
    }
    memset(&desc, 0, sizeof(struct file_reader_desc));
    desc.connector = dynamic_cast <ss_connector *>(ss_connector::get(handle));
    if (!desc.connector)
    {
        ss_print(PRINT_COLOR_RED, PRINT_MODE_HIGHTLIGHT, "Connector handle error!\n");
        close(desc.read_fd);
        return -1;
    }
    ss_print(PRINT_COLOR_GREEN, PRINT_MODE_HIGHTLIGHT, "Format: %s.\n", in_strs[4].c_str());
    desc.en_type               = EN_AUDIO_CODEC_DATA;
    if (in_strs[4] == "wav")
    {
        desc.es_aud_i.fmt = ES_STREAM_WAV;
    }
    else if (in_strs[4] == "pcm")
    {
        desc.es_aud_i.sample_rate = ss_cmd_atoi(in_strs[5].c_str());
        desc.es_aud_i.channels    = ss_cmd_atoi(in_strs[6].c_str());
        desc.es_aud_i.fmt         = ES_STREAM_PCM;
    }
    else
    {
        ss_print(PRINT_COLOR_RED, PRINT_MODE_HIGHTLIGHT, "Format: %s error!\n", in_strs[4].c_str());
        return -1;
    }
    desc.es_aud_i.sample_width = 16;
    snprintf(desc.file_path, 256, "%s", in_strs[2].c_str());
    desc.monitor_delay_ms      = ss_cmd_atoi(in_strs[3].c_str());
    desc.b_play_loop           = ss_cmd_atoi(in_strs[7].c_str());
    desc.do_monitor            = es_audio_file_reader;
    map_reader[handle]         = desc;
    desc.connector->connect_in(connect_file_reader, disconnect_file_reader,
                               (void *)(&map_reader[handle]));
    return 0;
}
static int destroy_file_connector_reader(vector<string> &in_strs)
{
    const string &handle = in_strs[1];
    auto it = map_reader.find(handle);
    if (it == map_reader.end())
    {
        ss_print(PRINT_COLOR_RED, PRINT_MODE_HIGHTLIGHT, "Not found this handle %s!\n", handle.c_str());
        return -1;
    }
    it->second.connector->disconnect_in();
    map_reader.erase(it);
    return 0;
}
static int video_es_file_writer(stream_packet_obj &packet, void *user)
{
    struct file_writer_desc *desc = (struct file_writer_desc *)user;

    if (!desc)
    {
        cout << "W desc error!" << endl;
        return -1;
    }
    if (packet->en_type != EN_VIDEO_CODEC_DATA)
    {
        cout << "Format error!" << " is: " << packet->en_type << endl;
        return -1;
    }
    if (desc->b_header)
    {
        if (!packet->es_vid_i.b_head)
        {
            unsigned char header[16];
            unsigned int data_size = 0;
            for (unsigned int i = 0; i < packet->es_vid_i.packet_count; i++)
            {
                data_size += packet->es_vid_i.packet_info[i].size;
            }
            memset(header, 0, 16);
            header[0] = 0x1;
            header[4] = ((data_size) >> 24) & 0xFF;
            header[5] = ((data_size) >> 16) & 0xFF;
            header[6] = ((data_size) >> 8)& 0xFF;
            header[7] = (data_size) & 0xFF;
            write(desc->write_fd, header, 16);
        }
    }
    for (unsigned int i = 0; i < packet->es_vid_i.packet_count; i++)
    {
        //char *p = new char[packet->es_vid.packet_data[i].size];
        //assert(p);
        //memcpy(p, packet->es_vid.packet_data[i].data, packet->es_vid.packet_data[i].size);
        //write(desc->write_fd, p, packet->es_vid.packet_data[i].size);
        //delete p;
        write(desc->write_fd, packet->es_vid.packet_data[i].data, packet->es_vid_i.packet_info[i].size);
    }

    return 0;
}
static map<string, struct file_writer_desc> writer_descs;
static int create_video_es_file_connector_writer(vector<string> &in_strs)
{
    const string &handle = in_strs[1];
    struct file_writer_desc w_desc;
    if (writer_descs.find(handle) != writer_descs.end())
    {
        return -1;
    }
    memset(&w_desc, 0, sizeof(struct file_writer_desc));
    w_desc.b_header = ss_cmd_atoi(in_strs[4].c_str());
    w_desc.connector = dynamic_cast <ss_connector *>(ss_connector::get(handle));
    if (!w_desc.connector)
    {
        ss_print(PRINT_COLOR_RED, PRINT_MODE_HIGHTLIGHT, "Connector handle error!\n");
        return -1;
    }
    w_desc.write_fd = open(in_strs[2].c_str(), O_WRONLY|O_CREAT|O_TRUNC, 0777);
    if (w_desc.write_fd < 0)
    {
        ss_print(PRINT_COLOR_RED, PRINT_MODE_HIGHTLIGHT, "Open %s error!\n", in_strs[2].c_str());
        return -1;
    }
    writer_descs[handle] = w_desc;
    w_desc.connector->set_packet_to(in_strs[3], video_es_file_writer, NULL, (void *)(long)&writer_descs[handle]);
    w_desc.connector->access();
    return 0;
}
static int create_video_es_file_connector_writer_thread(vector<string> &in_strs)
{
    const string &handle = in_strs[1];
    struct file_writer_desc w_desc;
    struct ss_thread_attr ss_attr;

    if (writer_descs.find(handle) != writer_descs.end())
    {
        return -1;
    }
    memset(&w_desc, 0, sizeof(struct file_writer_desc));
    w_desc.grab_time_out = ss_cmd_atoi(in_strs[3].c_str());
    w_desc.b_header = ss_cmd_atoi(in_strs[4].c_str());
    w_desc.connector = dynamic_cast <ss_connector *>(ss_connector::get(handle));
    if (!w_desc.connector)
    {
        ss_print(PRINT_COLOR_RED, PRINT_MODE_HIGHTLIGHT, "Connector handle error!\n");
        return -1;
    }
    w_desc.write_fd = open(in_strs[2].c_str(), O_WRONLY|O_CREAT|O_TRUNC, 0777);
    if (w_desc.write_fd < 0)
    {
        ss_print(PRINT_COLOR_RED, PRINT_MODE_HIGHTLIGHT, "Open %s error!\n", in_strs[2].c_str());
        return -1;
    }
    memset(&ss_attr, 0, sizeof(struct ss_thread_attr));
    ss_attr.do_signal = NULL;
    ss_attr.do_monitor = video_es_file_writer_thread;
    ss_attr.monitor_cycle_sec = 0;
    ss_attr.monitor_cycle_nsec = 0;
    ss_attr.is_reset_timer = 0;
    ss_attr.in_buf.buf = (void *)&(w_desc);
    ss_attr.in_buf.size = sizeof(struct file_writer_desc);
    snprintf(ss_attr.thread_name, 32, "video_es_file_writer_thread%s", handle.c_str());
    w_desc.thread_handle = ss_thread_open(&ss_attr);
    if (!w_desc.thread_handle)
    {
        close(w_desc.write_fd);
        ss_print(PRINT_COLOR_RED, PRINT_MODE_HIGHTLIGHT, "Monitor return error!\n");
        return -1;
    }
    ss_thread_start_monitor(w_desc.thread_handle);
    writer_descs[handle] = w_desc;
    w_desc.connector->access();
    return 0;
}
static int destroy_video_es_file_connector_writer_thread(vector<string> &in_strs)
{
    const string &handle = in_strs[1];
    struct file_writer_desc desc;
    int ret = 0;

    auto iter = writer_descs.find(handle);
    if (iter == writer_descs.end())
    {
        return -1;
    }
    memset(&desc, 0, sizeof(struct file_writer_desc));
    ret = ss_thread_get_buffer(iter->second.thread_handle, (void *)&desc);
    if (ret == -1)
    {
        ss_print(PRINT_COLOR_RED, PRINT_MODE_HIGHTLIGHT, "Handle %s get buffer error!\n", handle.c_str());
        return -1;
    }
    ret = ss_thread_close(iter->second.thread_handle);
    if (ret == -1)
    {
        ss_print(PRINT_COLOR_RED, PRINT_MODE_HIGHTLIGHT, "Handle %s close error!\n", handle.c_str());
        return -1;
    }
    iter->second.connector->leave();
    writer_descs.erase(iter);
    return close(desc.write_fd);
}
static int grab_connector_to_connector_step(vector<string> &in_strs)
{
    const string &src_handle = in_strs[1];
    const string &dst_handle = in_strs[2];
    unsigned int grab_time_out = ss_cmd_atoi(in_strs[3].c_str());
    ss_connector *src_connector = NULL;
    ss_connector *dst_connector = NULL;
    int retry_count = 100;

    src_connector = dynamic_cast <ss_connector *>(ss_ring_connector::get(src_handle));
    if (!src_connector)
    {
        ss_print(PRINT_COLOR_RED, PRINT_MODE_HIGHTLIGHT, "Connector handle error!\n");
        return -1;
    }
    dst_connector = dynamic_cast <ss_connector *>(ss_connector::get(dst_handle));
    if (!dst_connector)
    {
        ss_print(PRINT_COLOR_RED, PRINT_MODE_HIGHTLIGHT, "Connector handle error!\n");
        return -1;
    }
    while (retry_count)
    {
        auto packet = src_connector->grab(grab_time_out);
        if (packet == nullptr)
        {
            retry_count--;
            continue;
        }
        dst_connector->come(packet);
        src_connector->back(packet);
        return 0;
    }
    ss_print(PRINT_COLOR_RED, PRINT_MODE_HIGHTLIGHT, "GRAB time out!\n");
    return -1;
}
static int audio_es_file_writer(stream_packet_obj &packet, void *user)
{
    struct file_writer_desc *desc = (struct file_writer_desc *)user;

    if (!desc)
    {
        cout << "W desc error!" << endl;
        return -1;
    }
    if (packet->en_type != EN_AUDIO_CODEC_DATA)
    {
        cout << "Format error!" << " is: " << packet->en_type << endl;
        return -1;
    }
    for (unsigned int i = 0; i < packet->es_aud_i.packet_count; i++)
    {
        if (desc->b_header)
        {
            struct wave_file_header wav_header;
            memset(&wav_header, 0, sizeof(struct wave_file_header));
            lseek(desc->write_fd, 0, SEEK_SET);
            read(desc->write_fd, &wav_header, sizeof(struct wave_file_header));
            fill_wave_header(&wav_header, packet->es_aud_i.sample_rate, packet->es_aud_i.channels,
                             packet->es_aud_i.packet_info[i].size);
            lseek(desc->write_fd, 0, SEEK_SET);
            write(desc->write_fd, &wav_header, sizeof(struct wave_file_header));
            lseek(desc->write_fd, 0, SEEK_END);
        }
        write(desc->write_fd, packet->es_aud.packet_data[i].data, packet->es_aud_i.packet_info[i].size);
    }

    return 0;
}
static int create_audio_es_file_connector_writer(vector<string> &in_strs)
{
    const string &handle = in_strs[1];
    struct file_writer_desc w_desc;

    if (writer_descs.find(handle) != writer_descs.end())
    {
        return -1;
    }
    memset(&w_desc, 0, sizeof(struct file_writer_desc));
    w_desc.b_header = ss_cmd_atoi(in_strs[4].c_str());
    w_desc.connector = dynamic_cast <ss_connector *>(ss_connector::get(handle));
    if (!w_desc.connector)
    {
        ss_print(PRINT_COLOR_RED, PRINT_MODE_HIGHTLIGHT, "Connector handle error!\n");
        return -1;
    }
    w_desc.write_fd = open(in_strs[2].c_str(), O_WRONLY|O_CREAT|O_TRUNC, 0777);
    if (w_desc.write_fd < 0)
    {
        ss_print(PRINT_COLOR_RED, PRINT_MODE_HIGHTLIGHT, "Open %s error!\n", in_strs[2].c_str());
        return -1;
    }
    writer_descs[handle] = w_desc;
    w_desc.connector->set_packet_to(in_strs[3], audio_es_file_writer, NULL, (void *)(long)&writer_descs[handle]);
    w_desc.connector->access();
    return 0;
}
static int destroy_es_file_connector_writer(vector<string> &in_strs)
{
    const string &handle = in_strs[1];
    auto iter = writer_descs.find(handle);
    if (iter == writer_descs.end())
    {
        return -1;
    }
    if (!iter->second.connector)
    {
        ss_print(PRINT_COLOR_RED, PRINT_MODE_HIGHTLIGHT, "Connector handle error!\n");
        return -1;
    }
    iter->second.connector->set_packet_to(in_strs[2], NULL, NULL, NULL);
    iter->second.connector->leave();
    close(iter->second.write_fd);
    writer_descs.erase(iter);
    return 0;
}
MOD_CMDS(ring_connector) {
    ADD_CMD("create_system_ring_connector", create_system_ring_connector, 2);
    ADD_CMD_HELP("create_system_ring_connector", "[handle] [size]", "Create a system ring connector, and ringbuffer alloc by system.");
    ADD_CMD("destroy_system_ring_connector", destroy_system_ring_connector, 1);
    ADD_CMD_HELP("destroy_system_ring_connector", "[handle]", "Destroy a system ring handler.");
    ADD_CMD("create_video_es_file_connector_reader", create_video_es_file_connector_reader, 7);
    ADD_CMD_HELP("create_video_es_file_connector_reader", "[handle] [file_path] [fps] [enType(jpeg, h264, h265)] [w] [h] [b_loop]", "Create a es file reader for connector.");
    ADD_CMD("create_video_yuv_file_connector_reader", create_video_yuv_file_connector_reader, 7);
    ADD_CMD_HELP("create_video_yuv_file_connector_reader", "[handle] [file_path] [fps] [enType(yuv422, yuv420)] [w] [h] [b_loop]", "Create a yuv file reader for connector.");
    ADD_CMD("destroy_video_yuv_file_connector_reader", destroy_file_connector_reader, 1);
    ADD_CMD_HELP("destroy_video_yuv_file_connector_reader", "[handle]", "Destroy a yuv file reader for connector.");
    ADD_CMD("destroy_video_es_file_connector_reader", destroy_file_connector_reader, 1);
    ADD_CMD_HELP("destroy_video_es_file_connector_reader", "[handle]", "Destroy a file reader for connector.");
    ADD_CMD("create_video_es_file_connector_writer", create_video_es_file_connector_writer, 4);
    ADD_CMD_HELP("create_video_es_file_connector_writer", "[handle] [file_path] [refer_id] [b_add_header]", "Create a file writer for connector out.");
    ADD_CMD("destroy_video_es_file_connector_writer", destroy_es_file_connector_writer, 2);
    ADD_CMD_HELP("destroy_video_es_file_connector_writer", "[handle] [refer_id]", "Destroy a file writer for connector out.");
    ADD_CMD("create_video_es_file_connector_writer_thread", create_video_es_file_connector_writer_thread, 4);
    ADD_CMD_HELP("create_video_es_file_connector_writer_thread", "[handle] [file_path] [time_out] [b_add_header]", "Create a file writer for connector out.");
    ADD_CMD("destroy_video_es_file_connector_writer_thread", destroy_video_es_file_connector_writer_thread, 1);
    ADD_CMD_HELP("destroy_video_es_file_connector_writer_thread", "[handle]", "Destroy a file writer for connector out.");
    ADD_CMD("grab_connector_to_connector_step", grab_connector_to_connector_step, 3);
    ADD_CMD_HELP("grab_connector_to_connector_step", "[src_handle] [dst_handle] [time_out]", "Grab a stream packet from a connector to another connector.");
    ADD_CMD("create_audio_es_file_connector_reader", create_audio_es_file_connector_reader, 7);
    ADD_CMD_HELP("create_audio_es_file_connector_reader", "[handle] [file_path] [monitor_delay_ms] [enType:wav/pcm] [sample_rate] [channels] [b_loop]", "%s\n%s\n%s",
                 "Create a audio file reader for connector.",
                 "[sample_rate] If it is in [enType:wav] format, this parameter is invalid.",
                 "[channels] If it is in [enType:wav] format, this parameter is invalid.");
    ADD_CMD("destroy_audio_es_file_connector_reader", destroy_file_connector_reader, 1);
    ADD_CMD_HELP("destroy_audio_es_file_connector_reader", "[handle]", "Destroy a audio file reader for connector.");
    ADD_CMD("create_audio_es_file_connector_writer", create_audio_es_file_connector_writer, 4);
    ADD_CMD_HELP("create_audio_es_file_connector_writer", "[handle] [file_path] [refer_id] [b_add_header]", "Create a file writer for connector out.");
    ADD_CMD("destroy_audio_es_file_connector_writer", destroy_es_file_connector_writer, 2); /* Different commands call the same function. */
    ADD_CMD_HELP("destroy_audio_es_file_connector_writer", "[handle] [refer_id]", "Destroy a file writer for connector out.");
}
