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

#include "ss_packet.h"
#include <new>

stream_packet_obj stream_packet_base::convert(stream_packet_obj &packet, enum stream_type type)
{
    return (packet->en_type == type) ? packet : packet->do_convert(packet, type);
}
stream_packet_obj stream_packet_base::dup(stream_packet_obj &old_packet)
{
    auto new_packet = old_packet->dup();
    return new_packet ? new_packet : old_packet;
}
stream_packet_base::stream_packet_base()
{
    this->en_type = EN_STREAM_TYPE_NONE;
    memset(&this->es_aud_i, 0, sizeof(es_audio_info));
    memset(&this->es_vid_i, 0, sizeof(es_video_info));
    memset(&this->raw_vid_i, 0, sizeof(raw_video_info));
    memset(&this->meta_data_i, 0, sizeof(meta_data_info));
    memset(&this->es_aud, 0, sizeof(es_audio_packet));
    memset(&this->es_vid, 0, sizeof(es_video_packet));
    memset(&this->raw_vid, 0, sizeof(raw_video_packet));
    memset(&this->meta_data, 0, sizeof(meta_data_packet));
}
stream_packet_base::~stream_packet_base() {}
stream_packet_obj stream_packet_base::do_convert(stream_packet_obj &self, enum stream_type type)
{
    return nullptr;
}
stream_packet_obj stream_packet_base::dup()
{
    return nullptr;
}

stream_packet::stream_packet(const stream_packet_info &packet_info) : stream_packet_base()
{
    this->en_type = packet_info.en_type;
    switch (packet_info.en_type)
    {
        case EN_RAW_FRAME_DATA:
        {
            this->raw_vid_i = packet_info.raw_vid_i;
            for (unsigned int i = 0; i < this->raw_vid_i.plane_num; ++i)
            {
                const raw_video_info::raw_info &raw_i = this->raw_vid_i.plane_info[i];
                raw_video_packet::raw_data     &raw   = this->raw_vid.plane_data[i];
                stream_packet_info::raw_data_stride(raw_i, raw.stride);
                stream_packet_info::raw_data_size(raw_i, raw.stride, raw.size);
                raw.data[0] = new (std::nothrow)char[raw.size[0] + raw.size[1] + raw.size[2]];
                assert(raw.data[0]);
                raw.data[1] = raw.size[1] ? raw.data[0] + raw.size[0] : nullptr;
                raw.data[2] = raw.size[2] ? raw.data[1] + raw.size[1] : nullptr;
            }
        }
        break;
        case EN_VIDEO_CODEC_DATA:
        {
            this->es_vid_i = packet_info.es_vid_i;
            for (unsigned int i = 0; i < this->es_vid_i.packet_count; ++i)
            {
                this->es_vid.packet_data[i].data = new (std::nothrow)char[this->es_vid_i.packet_info[i].size];
                assert(this->es_vid.packet_data[i].data);
            }
        }
        break;
        case EN_AUDIO_CODEC_DATA:
        {
            this->es_aud_i = packet_info.es_aud_i;
            for (unsigned int i = 0; i < this->es_aud_i.packet_count; ++i)
            {
                this->es_aud.packet_data[i].data = new (std::nothrow)char[this->es_aud_i.packet_info[i].size];
                assert(this->es_aud.packet_data[i].data);
            }
        }
        break;
        case EN_USER_META_DATA:
        {
            this->meta_data_i = packet_info.meta_data_i;
            this->meta_data.data = new (std::nothrow)char[this->meta_data_i.size];
            assert(this->meta_data.data);
        }
        break;
        default:
        break;
    }
}
stream_packet::~stream_packet()
{
    switch (this->en_type)
    {
        case EN_RAW_FRAME_DATA:
        {
            for (unsigned int i = 0; i < this->raw_vid_i.plane_num; ++i)
            {
                delete[] this->raw_vid.plane_data[i].data[0];
            }
        }
        break;
        case EN_VIDEO_CODEC_DATA:
        {
            for (unsigned int i = 0; i < this->es_vid_i.packet_count; i++)
            {
                delete[] this->es_vid.packet_data[i].data;
            }
        }
        break;
        case EN_AUDIO_CODEC_DATA:
        {
            for (unsigned int i = 0; i < this->es_aud_i.packet_count; i++)
            {
                delete[] this->es_aud.packet_data[i].data;
            }
        }
        break;
        case EN_USER_META_DATA:
        {
            delete[] this->meta_data.data;
        }
        break;
        default:
            return;
    }
}

stream_packet_clone::stream_packet_clone(const stream_packet_base &packet)
    : stream_packet_base(packet)
{
    switch (packet.en_type)
    {
        case EN_RAW_FRAME_DATA:
        {
            for (unsigned int i = 0; i < this->raw_vid_i.plane_num; ++i)
            {
                raw_video_packet::raw_data &raw = this->raw_vid.plane_data[i];
                raw.data[0] = new (std::nothrow)char[raw.size[0] + raw.size[1] + raw.size[2]];
                assert(raw.data[0]);
                memcpy(raw.data[0], packet.raw_vid.plane_data[i].data[0], raw.size[0]);
                raw.data[1] = raw.size[1] ?
                    (char *)memcpy(raw.data[0] + raw.size[0], packet.raw_vid.plane_data[i].data[1], raw.size[1])
                    : nullptr;
                raw.data[2] = raw.size[2] ?
                    (char *)memcpy(raw.data[1] + raw.size[1], packet.raw_vid.plane_data[i].data[2], raw.size[2])
                    : nullptr;
            }
        }
        break;
        case EN_VIDEO_CODEC_DATA:
        {
            for (unsigned int i = 0; i < this->es_vid_i.packet_count; ++i)
            {
                this->es_vid.packet_data[i].data = new (std::nothrow)char[this->es_vid_i.packet_info[i].size];
                assert(this->es_vid.packet_data[i].data);
                memcpy(this->es_vid.packet_data[i].data, packet.es_vid.packet_data[i].data, this->es_vid_i.packet_info[i].size);
            }
        }
        break;
        case EN_AUDIO_CODEC_DATA:
        {
            for (unsigned int i = 0; i < this->es_aud_i.packet_count; ++i)
            {
                this->es_aud.packet_data[i].data = new (std::nothrow)char[this->es_aud_i.packet_info[i].size];
                assert(this->es_aud.packet_data[i].data);
                memcpy(this->es_aud.packet_data[i].data, packet.es_aud.packet_data[i].data, this->es_aud_i.packet_info[i].size);
            }
        }
        break;
        case EN_USER_META_DATA:
        {
            this->meta_data_i = packet.meta_data_i;
            this->meta_data.data = new (std::nothrow)char[this->meta_data_i.size];
            assert(this->meta_data.data);
            memcpy(this->meta_data.data, packet.meta_data.data, this->meta_data_i.size);
        }
        break;
        default:
        break;
    }
}

stream_packet_clone::~stream_packet_clone()
{
    switch (this->en_type)
    {
        case EN_RAW_FRAME_DATA:
        {
            for (unsigned int i = 0; i < this->raw_vid_i.plane_num; ++i)
            {
                delete[] this->raw_vid.plane_data[i].data[0];
            }
        }
        break;
        case EN_VIDEO_CODEC_DATA:
        {
            for (unsigned int i = 0; i < this->es_vid_i.packet_count; i++)
            {
                delete[] this->es_vid.packet_data[i].data;
            }
        }
        break;
        case EN_AUDIO_CODEC_DATA:
        {
            for (unsigned int i = 0; i < this->es_aud_i.packet_count; i++)
            {
                delete[] this->es_aud.packet_data[i].data;
            }
        }
        break;
        case EN_USER_META_DATA:
        {
            delete[] this->meta_data.data;
        }
        break;
        default:
            return;
    }
}

void stream_packet_clone::update_time_stamp()
{
    //return nothing to skip gettimeofday.
}

void stream_packet_info::raw_data_size(const raw_video_info::raw_info &raw_i, const unsigned int stride[3],
                                       unsigned int size[3])
{
    switch (raw_i.fmt)
    {
        case RAW_FORMAT_YUV420SP:
        case RAW_FORMAT_YUV420SP_NV21:
        {
            size[0] = raw_i.height * stride[0];
            size[1] = raw_i.height * stride[1] / 2;
            size[2] = 0;
        }
        break;
        case RAW_FORMAT_YUV422SP:
        {
            size[0] = raw_i.height * stride[0];
            size[1] = raw_i.height * stride[1];
            size[2] = 0;
        }
        break;
        default:
        {
            size[0] = raw_i.height * stride[0];
            size[1] = 0;
            size[2] = 0;
        }
        break;
    }
}
void stream_packet_info::raw_data_stride(const raw_video_info::raw_info &raw_i, unsigned int stride[3])
{
    switch (raw_i.fmt)
    {
        case RAW_FORMAT_YUV422SP:
        case RAW_FORMAT_YUV420SP:
        case RAW_FORMAT_YUV420SP_NV21:
        {
            stride[0] = raw_i.width;
            stride[1] = raw_i.width;
            stride[2] = 0;
        }
        break;
        case RAW_FORMAT_YUV422_YUYV:
        case RAW_FORMAT_YUV422_UYVY:
        case RAW_FORMAT_YUV422_VYUY:
        case RAW_FORMAT_YUV422_YVYU:
        case RAW_FORMAT_RGB565:
        case RAW_FORMAT_ARGB1555:
        case RAW_FORMAT_ARGB4444:
        {
            stride[0] = raw_i.width * 2;
            stride[1] = 0;
            stride[2] = 0;
        }
        break;
        case RAW_FORMAT_RGB888:
        case RAW_FORMAT_BGR888:
        {
            stride[0] = raw_i.width * 3;
            stride[1] = 0;
            stride[2] = 0;
        }
        break;
        case RAW_FORMAT_ARGB8888:
        case RAW_FORMAT_ABGR8888:
        case RAW_FORMAT_BGRA8888:
        {
            stride[0] = raw_i.width * 4;
            stride[1] = 0;
            stride[2] = 0;
        }
        break;
        case RAW_FORMAT_I2:
        {
            stride[0] = ALIGN_UP(raw_i.width, 4) / 4;
            stride[1] = 0;
            stride[2] = 0;
        }
        break;
        case RAW_FORMAT_I4:
        {
            stride[0] = ALIGN_UP(raw_i.width, 2) / 2;
            stride[1] = 0;
            stride[2] = 0;
        }
        break;
        case RAW_FORMAT_I8:
        {
            stride[0] = raw_i.width;
            stride[1] = 0;
            stride[2] = 0;
        }
        break;
        case RAW_FORMAT_BAYER_BASE ... RAW_FORMAT_BAYER_NUM:
        {
            if (RAW_PRECISION_10BPP == (raw_i.fmt - RAW_FORMAT_BAYER_BASE) / RAW_PIXEL_BAYERID_MAX)
            {
                stride[0] = raw_i.width * 5 / 4;
            }
            else if (RAW_PRECISION_12BPP == (raw_i.fmt - RAW_FORMAT_BAYER_BASE) / RAW_PIXEL_BAYERID_MAX)
            {
                stride[0] = raw_i.width * 3 / 2;
            }
            else if (RAW_PRECISION_14BPP == (raw_i.fmt - RAW_FORMAT_BAYER_BASE) / RAW_PIXEL_BAYERID_MAX
                     || RAW_PRECISION_16BPP == (raw_i.fmt - RAW_FORMAT_BAYER_BASE) / RAW_PIXEL_BAYERID_MAX)
            {
                stride[0] = raw_i.width * 2;
            }
            else
            {
                stride[0] = raw_i.width;
            }
            stride[1] = 0;
            stride[2] = 0;
        }
        break;
        default:
        {
            stride[0] = raw_i.width;
            stride[1] = 0;
            stride[2] = 0;
        }
        break;
    }
}
int stream_packet_info::raw_convert_vpa_packet(stream_packet_obj &self, stream_packet_obj &pa, stream_packet_obj &va)
{
    stream_packet_info info;
    if (self->en_type == EN_RAW_FRAME_DATA)
    {
        va = self;
        pa = stream_packet_base::convert(self, EN_RAW_FRAME_DATA_PA);
        return pa ? 0 : -1;
    }
    if (self->en_type == EN_RAW_FRAME_DATA_PA)
    {
        pa = self;
        va = stream_packet_base::convert(self, EN_RAW_FRAME_DATA);
        return va ? 0 : -1;
    }
    return -1;
}
