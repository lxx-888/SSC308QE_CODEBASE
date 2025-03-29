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

#ifndef __SS_PACKET_H__
#define __SS_PACKET_H__

#include <sys/time.h>
#include <cassert>
#include <cstring>
#include <memory>
#include <new>
#include <string>
#include <type_traits>
#define ES_SLICE_COUNT_MAX 16

//#define ALIGN_UP(x, align) (((x) + ((align) - 1)) & ~((align) - 1))
#define ALIGN_UP(x, align) ((((x) + ((align) - 1)) / (align)) * (align))

class stream_packet_base;
using stream_packet_obj = std::shared_ptr<stream_packet_base>;

enum es_video_fmt
{
    ES_STREAM_H264,
    ES_STREAM_H265,
    ES_STREAM_JPEG,
    ES_STREAM_AV1,
    ES_STREAM_VP9
};
enum es_audio_fmt
{
    ES_STREAM_PCM,
    ES_STREAM_AAC,
    ES_STREAM_WAV,
    ES_STREAM_G711U,
    ES_STREAM_G711A,
    ES_STREAM_G726_16,
    ES_STREAM_G726_24,
    ES_STREAM_G726_32,
    ES_STREAM_G726_40
};
enum raw_data_precision
{
    RAW_PRECISION_8BPP,
    RAW_PRECISION_10BPP,
    RAW_PRECISION_12BPP,
    RAW_PRECISION_14BPP,
    RAW_PRECISION_16BPP,
    RAW_PRECISION_MAX
};
enum raw_bayer_id
{
    RAW_PIXEL_BAYERID_RG,
    RAW_PIXEL_BAYERID_GR,
    RAW_PIXEL_BAYERID_BG,
    RAW_PIXEL_BAYERID_GB,
    RAW_PIXEL_RGBIR_R0,
    RAW_PIXEL_RGBIR_G0,
    RAW_PIXEL_RGBIR_B0,
    RAW_PIXEL_RGBIR_G1,
    RAW_PIXEL_RGBIR_G2,
    RAW_PIXEL_RGBIR_I0,
    RAW_PIXEL_RGBIR_G3,
    RAW_PIXEL_RGBIR_I1,
    RAW_PIXEL_BAYERID_MAX
};
enum raw_video_fmt
{
    RAW_FORMAT_YUV422_YUYV,
    RAW_FORMAT_YUV422_UYVY,
    RAW_FORMAT_YUV422_YVYU,
    RAW_FORMAT_YUV422_VYUY,
    RAW_FORMAT_YUV422SP,
    RAW_FORMAT_YUV420SP,
    RAW_FORMAT_YUV420SP_NV21,
    RAW_FORMAT_RGB888,
    RAW_FORMAT_BGR888,
    RAW_FORMAT_ARGB8888,
    RAW_FORMAT_ABGR8888,
    RAW_FORMAT_BGRA8888,
    RAW_FORMAT_RGB565,
    RAW_FORMAT_ARGB1555,
    RAW_FORMAT_ARGB4444,
    RAW_FORMAT_I2,
    RAW_FORMAT_I4,
    RAW_FORMAT_I8,
    RAW_FORMAT_BAYER_BASE,
    RAW_FORMAT_BAYER_NUM =
        RAW_FORMAT_BAYER_BASE + RAW_PIXEL_BAYERID_MAX * RAW_PRECISION_MAX -1,
    RAW_FORMAT_MAX
};

#define TO_OTHER_RGB_BAYER_PIXEL(__pixel, __other_base) \
    ((__pixel ) - RAW_FORMAT_BAYER_BASE + (__other_base))

#define FROM_RGB_BAYER_PIXEL(__bit_mode, __pixel) \
    (RAW_FORMAT_BAYER_BASE + (__bit_mode ) * RAW_PRECISION_MAX + (__pixel))

struct es_video_info
{
    struct es_info
    {
        unsigned int size;
        bool         b_end;
    };
    enum es_video_fmt fmt;
    unsigned int      width;
    unsigned int      height;
    bool              b_head;
    unsigned int      packet_count;
    es_info           packet_info[ES_SLICE_COUNT_MAX];
};

struct es_video_packet
{
    struct es_data
    {
        char *data;
    };
    struct es_data packet_data[ES_SLICE_COUNT_MAX];
};
struct es_audio_info
{
    struct es_info
    {
        unsigned int size;
    };
    enum es_audio_fmt fmt;
    unsigned int      sample_rate;
    unsigned int      sample_width;
    unsigned int      channels;
    unsigned int      packet_count;
    es_info           packet_info[ES_SLICE_COUNT_MAX];
};
struct es_audio_packet
{
    struct es_data
    {
        char *data;
    };
    struct es_data packet_data[ES_SLICE_COUNT_MAX];
};
#define RAW_FRAME_MULTI_PLANE_NUM_MAX (8)
struct raw_video_info
{
    struct raw_info
    {
        enum raw_video_fmt fmt;
        unsigned int       width;
        unsigned int       height;
    };
    unsigned char plane_num;
    raw_info      plane_info[RAW_FRAME_MULTI_PLANE_NUM_MAX];
};
struct raw_video_packet_pa
{
    struct raw_data_pa
    {
        unsigned long long phy[3];
        unsigned int       stride[3];
        unsigned int       size[3];
        void               *raw_data_handle;
    };
    unsigned long long private_handle;
    raw_data_pa        plane_data[RAW_FRAME_MULTI_PLANE_NUM_MAX];
};
struct raw_video_packet
{
    struct raw_data
    {
        char              *data[3];
        unsigned int       stride[3];
        unsigned int       size[3];
    };
    raw_data plane_data[RAW_FRAME_MULTI_PLANE_NUM_MAX];
};
struct meta_data_info
{
    unsigned int size;
    unsigned int reserved;
};
struct meta_data_packet
{
    char *data;
};
enum stream_type
{
    EN_RAW_FRAME_DATA,
    EN_RAW_FRAME_DATA_PA,
    EN_VIDEO_CODEC_DATA,
    EN_AUDIO_CODEC_DATA,
    EN_USER_META_DATA,
    EN_STREAM_TYPE_NONE
};
struct stream_packet_info
{
    enum stream_type en_type;
    union
    {
        struct es_audio_info es_aud_i;
        struct es_video_info es_vid_i;
        struct raw_video_info raw_vid_i;
        struct meta_data_info meta_data_i;
    };
    stream_packet_info() : en_type(EN_STREAM_TYPE_NONE)
    {
        memset(&this->es_aud_i, 0, sizeof(es_audio_info));
        memset(&this->es_vid_i, 0, sizeof(es_video_info));
        memset(&this->raw_vid_i, 0, sizeof(raw_video_info));
        memset(&this->meta_data_i, 0, sizeof(meta_data_info));
    }
    static void raw_data_size(const raw_video_info::raw_info &raw_i, const unsigned int stride[3],
                              unsigned int size[3]);
    static void raw_data_stride(const raw_video_info::raw_info &raw_i, unsigned int stride[3]);
    static int  raw_convert_vpa_packet(stream_packet_obj &self, stream_packet_obj &pa, stream_packet_obj &va);
};

class stream_packet_base
{
public:
    class err_buf
    {
    public:
        err_buf() {}
        virtual ~err_buf() {}
        virtual void show() const
        {
            assert(0);
        }
    };
public:
    enum stream_type en_type;

    union
    {
        struct es_audio_info  es_aud_i;
        struct es_video_info  es_vid_i;
        struct raw_video_info raw_vid_i;
        struct meta_data_info meta_data_i;
    };

    union
    {
        struct es_audio_packet     es_aud;
        struct es_video_packet     es_vid;
        struct raw_video_packet    raw_vid;    //shared with raw_vid_i
        struct raw_video_packet_pa raw_vid_pa; //shared with raw_vid_i
        struct meta_data_packet    meta_data;
    };

    virtual ~stream_packet_base();
    // If old_packet->dup() is nullptr, return old_packet
    static stream_packet_obj dup(stream_packet_obj &old_packet);
    static stream_packet_obj convert(stream_packet_obj &packet, enum stream_type type);
    template <typename T, typename... Args>
    static std::shared_ptr<T> make(Args&&... args)
    {
        static_assert(std::is_base_of<stream_packet_base, T>::value, "T must be derived from stream_packet_base");
        try
        {
            auto obj = std::make_shared<T>(std::forward<Args>(args)...);
            if (obj)
            {
                obj->auto_update_time_stamp();
            }
            return obj;
        }
        catch (const err_buf &e)
        {
            e.show();
            return nullptr;
        }
    }
    int compare(const stream_packet_obj &packet) const
    {
        if (packet->en_type != en_type)
        {
            return -1;
        }
        if (en_type == EN_AUDIO_CODEC_DATA)
        {
            if (es_aud_i.channels != packet->es_aud_i.channels || es_aud_i.fmt != packet->es_aud_i.fmt
                || es_aud_i.sample_rate != packet->es_aud_i.sample_rate || es_aud_i.sample_width != packet->es_aud_i.sample_width
                || es_aud_i.packet_count != packet->es_aud_i.packet_count)
            {
                return -1;
            }
            for (unsigned int i = 0; i < es_aud_i.packet_count; i++)
            {
                if (es_aud_i.packet_info[i].size != packet->es_aud_i.packet_info[i].size )
                {
                    return -1;
                }
            }
            return 0;
        }
        if (en_type == EN_VIDEO_CODEC_DATA)
        {
            if (es_vid_i.fmt != packet->es_vid_i.fmt || es_vid_i.width != packet->es_vid_i.width || es_vid_i.height != packet->es_vid_i.height
                || es_vid_i.b_head != packet->es_vid_i.b_head || es_vid_i.packet_count != packet->es_vid_i.packet_count)
            {
                return -1;
            }
            for (unsigned int i = 0; i < es_vid_i.packet_count; i++)
            {
                if (es_vid_i.packet_info[i].size != packet->es_vid_i.packet_info[i].size
                    || es_vid_i.packet_info[i].b_end != packet->es_vid_i.packet_info[i].b_end)
                {
                    return -1;
                }
            }
            return 0;
        }
        if (en_type == EN_RAW_FRAME_DATA)
        {
            if (raw_vid_i.plane_num != packet->raw_vid_i.plane_num)
            {
                return -1;
            }
            for (unsigned int i = 0; i < raw_vid_i.plane_num; i++)
            {
                if (raw_vid_i.plane_info[i].fmt != packet->raw_vid_i.plane_info[i].fmt
                    || raw_vid_i.plane_info[i].width != packet->raw_vid_i.plane_info[i].width
                    || raw_vid_i.plane_info[i].height != packet->raw_vid_i.plane_info[i].height)
                {
                    return -1;
                }
            }
            return 0;
        }
        if (en_type == EN_USER_META_DATA)
        {
            if (meta_data_i.size != packet->meta_data_i.size)
            {
                return -1;
            }
            return 0;
        }
        return -1;
    }
    int compare_copy(stream_packet_obj &packet)
    {
        if (packet->en_type != en_type)
        {
            return -1;
        }
        if (en_type == EN_AUDIO_CODEC_DATA)
        {
            if (es_aud_i.channels != packet->es_aud_i.channels || es_aud_i.fmt != packet->es_aud_i.fmt
                || es_aud_i.sample_rate != packet->es_aud_i.sample_rate || es_aud_i.sample_width != packet->es_aud_i.sample_width
                || es_aud_i.packet_count != packet->es_aud_i.packet_count)
            {
                return -1;
            }
            for (unsigned int i = 0; i < es_aud_i.packet_count; i++)
            {
                if (es_aud_i.packet_info[i].size != packet->es_aud_i.packet_info[i].size)
                {
                    return -1;
                }
                memcpy(es_aud.packet_data[i].data, packet->es_aud.packet_data[i].data, es_aud_i.packet_info[i].size);
            }
            return 0;
        }
        if (en_type == EN_VIDEO_CODEC_DATA)
        {
            if (es_vid_i.fmt != packet->es_vid_i.fmt || es_vid_i.width != packet->es_vid_i.width || es_vid_i.height != packet->es_vid_i.height
                || es_vid_i.b_head != packet->es_vid_i.b_head || es_vid_i.packet_count != packet->es_vid_i.packet_count)
            {
                return -1;
            }
            for (unsigned int i = 0; i < es_vid_i.packet_count; i++)
            {
                if (es_vid_i.packet_info[i].size != packet->es_vid_i.packet_info[i].size
                    || es_vid_i.packet_info[i].b_end != packet->es_vid_i.packet_info[i].b_end)
                {
                    return -1;
                }
                memcpy(es_vid.packet_data[i].data, packet->es_vid.packet_data[i].data, es_vid_i.packet_info[i].size);
            }
            return 0;
        }
        if (en_type == EN_RAW_FRAME_DATA)
        {
            if (raw_vid_i.plane_num != packet->raw_vid_i.plane_num)
            {
                return -1;
            }
            for (unsigned int i = 0; i < raw_vid_i.plane_num; i++)
            {
                if (raw_vid_i.plane_info[i].fmt != packet->raw_vid_i.plane_info[i].fmt
                    || raw_vid_i.plane_info[i].width != packet->raw_vid_i.plane_info[i].width
                    || raw_vid_i.plane_info[i].height != packet->raw_vid_i.plane_info[i].height)
                {
                    return -1;
                }
                memcpy(es_aud.packet_data[i].data, packet->es_aud.packet_data[i].data, es_aud_i.packet_info[i].size);
            }
            return 0;
        }
        if (en_type == EN_USER_META_DATA)
        {
            if (meta_data_i.size != packet->meta_data_i.size)
            {
                return -1;
            }
            memcpy(meta_data.data, packet->meta_data.data, meta_data_i.size);
            return 0;
        }
        return -1;
    }
    long get_priv() const
    {
        return private_data;
    }
    void set_priv(long priv)
    {
        private_data = priv;
    }
    const std::string &get_type()
    {
        return type_name;
    }
    const struct timeval &get_time_stamp()
    {
        return time_stamp;
    }
    void set_time_stamp(const struct timeval &stamp)
    {
        time_stamp = stamp;
    }
    void auto_update_time_stamp()
    {
        update_time_stamp();
    }
protected:
    stream_packet_base();
    virtual void update_time_stamp()
    {
        gettimeofday(&time_stamp, NULL);
    }
    std::string type_name;
private:
    // Default, dup return nullptr
    // If the memory of packet is not alloc by self, use deep copy overwrite it.
    virtual stream_packet_obj dup();
    virtual stream_packet_obj do_convert(stream_packet_obj &self, enum stream_type type);
    long private_data;
    struct timeval time_stamp;
};

class stream_packet : public stream_packet_base
{
public:
    stream_packet(const stream_packet_info &packet_info);
    virtual ~stream_packet() override;
};

class stream_packet_clone : public stream_packet_base
{
public:
    stream_packet_clone(const stream_packet_base &packet);
    virtual ~stream_packet_clone();
private:
    void update_time_stamp() final;
};

class stream_packer
{
public:
    stream_packer() {}
    virtual ~stream_packer() {}
    virtual stream_packet_obj make(const stream_packet_info &packet_info) = 0;
};

#endif /* __SS_PACKET_H__ */
