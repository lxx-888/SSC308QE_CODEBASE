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
#ifndef __SS_RING_CONNECTOR__
#define __SS_RING_CONNECTOR__

#include <cassert>
#include <map>
#include <memory>
#include "ss_auto_lock.h"
#include "ss_handle.h"
#include "ss_packet.h"
#include "ss_ring_allocator.h"
#include "ss_connector.h"

struct ring_stream_header
{
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
        struct es_audio_packet  es_aud;
        struct es_video_packet  es_vid;
        struct raw_video_packet raw_vid;
        struct meta_data_packet meta_data;
    };
    ring_stream_header & operator=(stream_packet_obj &packet)
    {
        this->en_type = packet->en_type;
        switch (this->en_type) 
        {
            case EN_RAW_FRAME_DATA:
            {
                this->raw_vid_i = packet->raw_vid_i;
            }
            break;
            case EN_VIDEO_CODEC_DATA:
            {
                this->es_vid_i = packet->es_vid_i;
            }
            break;
            case EN_AUDIO_CODEC_DATA:
            {
                this->es_aud_i = packet->es_aud_i;
            }
            break;
            case EN_USER_META_DATA:
            {
                this->meta_data_i = packet->meta_data_i;
            }
            break;
            default:
            break;
        }
        return *this;
    }
    ring_stream_header& operator=(const stream_packet_info &packet)
    {
        this->en_type = packet.en_type;
        switch (this->en_type) 
        {
            case EN_RAW_FRAME_DATA:
            {
                this->raw_vid_i = packet.raw_vid_i;
            }
            break;
            case EN_VIDEO_CODEC_DATA:
            {
                this->es_vid_i = packet.es_vid_i;
            }
            break;
            case EN_AUDIO_CODEC_DATA:
            {
                this->es_aud_i = packet.es_aud_i;
            }
            break;
            case EN_USER_META_DATA:
            {
                this->meta_data_i = packet.meta_data_i;
            }
            break;
            default:
            break;
        }
        return *this;
    }
};
class ring_stream_packet : public stream_packet_base
{
public:
    ring_stream_packet(const ring_stream_header &packet) : stream_packet_base()
    {
        this->en_type = packet.en_type;
        switch (this->en_type) 
        {
            case EN_RAW_FRAME_DATA:
            {
                this->raw_vid_i = packet.raw_vid_i;
                this->raw_vid   = packet.raw_vid;
            }
            break;
            case EN_VIDEO_CODEC_DATA:
            {
                this->es_vid_i = packet.es_vid_i;
                this->es_vid   = packet.es_vid;
            }
            break;
            case EN_AUDIO_CODEC_DATA:
            {
                this->es_aud_i = packet.es_aud_i;
                this->es_aud   = packet.es_aud;
            }
            break;
            case EN_USER_META_DATA:
            {
                this->meta_data_i = packet.meta_data_i;
                this->meta_data   = packet.meta_data;
            }
            break;
            default:
                return;
        }
    }
    virtual ~ring_stream_packet() {}
private:
    stream_packet_obj dup() const
    {
        return stream_packet_base::make<stream_packet_clone>(*this);
    }
};

class ss_ring_connector : public ss_connector, protected ss_ring_allocator
{
    public:
        ss_ring_connector()
        {
        }
        explicit ss_ring_connector(unsigned int size)
        {
            char *virt = new char[size];
            assert(virt);
            init_pool(size, virt);
        }
        virtual ~ss_ring_connector() {}
        virtual int come(stream_packet_obj &packet) override
        {
            ring_addr start = 0, end = 0;
            ring_stream_header *packet_ring = NULL;

            ss_auto_lock lock(connector_mutex);

            //Prepare free ring buffer to fill packet header data.
            start = prepare_buf(sizeof(ring_stream_header));
            if (start == (ring_addr)-1)
                return -1;

            //Translate ring addr to system addr.
            from_ring_buf((char *&)packet_ring, start);

            //Copy the stream header to ring.
            *packet_ring = packet;

            //Prepare stream data by header.
            end = prepare_ring_packet(*packet_ring);
            if (end == (ring_addr)-1)
            {
                deactive_buf();
                return -1;
            }
            //Copy stream data to the prepared ring buffer.
            copy_to_ring_packet(*packet_ring, *packet);

            //Active ring buffer, can get from function 'grab'.
            active_buf();

            //Wake up 'grab'.
            wake_up_cond();

            return 0;
        }
        virtual stream_packet_obj getbuf(const stream_packet_info &packet_info) override
        {
            ring_addr start = 0, end = 0;
            struct ring_stream_header *packet_ring = NULL;

            ss_auto_lock lock(connector_mutex);

            //Prepare free ring buffer to fill packet header data.
            start = prepare_buf(sizeof(ring_stream_header));
            if (start == (ring_addr)-1)
                return nullptr;

            //Translate ring addr to system addr.
            from_ring_buf((char *&)packet_ring, start);

            //Copy the stream header to ring.
            *packet_ring = packet_info;

            //Prepare stream data by header.
            end = prepare_ring_packet(*packet_ring);
            if (end == (ring_addr)-1)
            {
                deactive_buf();
                return nullptr;
            }

            //Translate data address from ring to system.
            ring_stream_header packet = *packet_ring;
            from_ring_packet(packet);

            auto new_pack = stream_packet_base::make<ring_stream_packet>(packet);
            assert(new_pack);
            new_pack->set_priv(end);
            return new_pack;
        }
        virtual void update(stream_packet_obj &packet) override
        {
            if (packet == nullptr)
            {
                return;
            }
            ss_auto_lock lock(connector_mutex);
            if (packet->get_priv() != prepare_buf(0))
            {
                deactive_buf();
                cout << "Update order error!" << endl;
                return;
            }
            active_buf();
            //Wake up 'grab'.
            wake_up_cond();
        }
        virtual stream_packet_obj grab(int delay_ms = 100) override
        {
            int wait_ret = 0;
            ss_auto_lock lock(connector_mutex);
            do
            {
                ring_stream_header packet_ring;
                if (copy_from_ring((char *)&packet_ring, sizeof(ring_stream_header)) != -1)
                {
                    ring_addr end =  from_ring_packet(packet_ring);
                    auto new_pack = stream_packet_base::make<ring_stream_packet>(packet_ring);
                    assert(new_pack);
                    new_pack->set_priv(end);
                    return new_pack;
                }
                wait_ret = wait_cond(delay_ms);
            }while (wait_ret != ETIMEDOUT);
            return nullptr;
        }
        virtual void back(stream_packet_obj &packet) override
        {
            if (packet == nullptr)
            {
                return;
            }
            ss_auto_lock lock(connector_mutex);
            flip_buf(packet->get_priv());
        }
    protected:
        ring_addr prepare_ring_packet(ring_stream_header &packet)
        {
            switch (packet.en_type)
            {
                case EN_RAW_FRAME_DATA:
                {
                    for (unsigned int i = 0; i < packet.raw_vid_i.plane_num; ++i)
                    {
                        const raw_video_info::raw_info &raw_i = packet.raw_vid_i.plane_info[i];
                        raw_video_packet::raw_data     &raw   = packet.raw_vid.plane_data[i];
                        stream_packet_info::raw_data_stride(raw_i, raw.stride);
                        stream_packet_info::raw_data_size(raw_i, raw.stride, raw.size);
                        raw.data[0] = (char *)prepare_buf(raw.size[0] + raw.size[1] + raw.size[2]);
                        if (raw.data[0] == (char *)-1)
                        {
                            return (ring_addr)-1;
                        }
                        raw.data[1] = raw.size[1] ? raw.data[0] + raw.size[0] : nullptr;
                        raw.data[2] = raw.size[2] ? raw.data[1] + raw.size[1] : nullptr;
                    }
                }
                break;
                case EN_VIDEO_CODEC_DATA:
                {
                    for (unsigned int i = 0; i < packet.es_vid_i.packet_count; i++)
                    {
                        packet.es_vid.packet_data[i].data = (char *)prepare_buf(packet.es_vid_i.packet_info[i].size);
                        if (packet.es_vid.packet_data[i].data == (char *)-1)
                            return (ring_addr)-1;
                    }
                }
                break;
                case EN_AUDIO_CODEC_DATA:
                {
                    for (unsigned int i = 0; i < packet.es_aud_i.packet_count; i++)
                    {
                        packet.es_aud.packet_data[i].data = (char *)prepare_buf(packet.es_aud_i.packet_info[i].size);
                        if (packet.es_aud.packet_data[i].data == (char *)-1)
                            return (ring_addr)-1;
                    }
                }
                break;
                case EN_USER_META_DATA:
                {
                    packet.meta_data.data = (char *)prepare_buf(packet.meta_data_i.size);
                    if (packet.meta_data.data == (char *)-1)
                        return (ring_addr)-1;
                }
                break;
                default:
                    cout << "FMT " << packet.en_type << " ERROR!" << endl;
                    return (ring_addr)-1;
            }
            return prepare_buf(0);
        }
        ring_addr from_ring_packet(ring_stream_header &packet)
        {
            ring_addr tail = 0;
            switch (packet.en_type)
            {
                case EN_RAW_FRAME_DATA:
                {
                    for (unsigned int i = 0; i < packet.raw_vid_i.plane_num; ++i)
                    {
                        raw_video_packet::raw_data &raw = packet.raw_vid.plane_data[i];

                        tail = (ring_addr)raw.data[0] + raw.size[0] + raw.size[1] + raw.size[2];
                        if (raw.size[0])
                        {
                            from_ring_buf(raw.data[0], (ring_addr)raw.data[0]);
                        }
                        if (raw.size[1])
                        {
                            from_ring_buf(raw.data[1], (ring_addr)raw.data[1]);
                        }
                        if (raw.size[2])
                        {
                            from_ring_buf(raw.data[2], (ring_addr)raw.data[2]);
                        }
                    }
                }
                break;
                case EN_VIDEO_CODEC_DATA:
                {
                    for (unsigned int i = 0; i < packet.es_vid_i.packet_count; i++)
                    {
                        tail = (ring_addr)packet.es_vid.packet_data[i].data + packet.es_vid_i.packet_info[i].size;
                        from_ring_buf(packet.es_vid.packet_data[i].data, (ring_addr)packet.es_vid.packet_data[i].data);
                    }
                }
                break;
                case EN_AUDIO_CODEC_DATA:
                {
                    for (unsigned int i = 0; i < packet.es_aud_i.packet_count; i++)
                    {
                        tail = (ring_addr)packet.es_aud.packet_data[i].data + packet.es_aud_i.packet_info[i].size;
                        from_ring_buf(packet.es_aud.packet_data[i].data, (ring_addr)packet.es_aud.packet_data[i].data);
                    }
                }
                break;
                case EN_USER_META_DATA:
                {
                    tail = (ring_addr)packet.meta_data.data + packet.meta_data_i.size;
                    from_ring_buf(packet.meta_data.data, (ring_addr)packet.meta_data.data);
                }
                break;
                default:
                    cout << "FMT " << packet.en_type << " ERROR!" << endl;
                    return -1;
            }
            return tail;
        }
        int copy_to_ring_packet(ring_stream_header &ring_packet, const stream_packet_base &packet)
        {
            switch (packet.en_type)
            {
                case EN_RAW_FRAME_DATA:
                {
                    for (unsigned int i = 0; i < packet.raw_vid_i.plane_num; ++i)
                    {
                        if (ring_packet.raw_vid.plane_data[i].size[0])
                        {
                            copy_to_ring((ring_addr)ring_packet.raw_vid.plane_data[i].data[0],
                                         packet.raw_vid.plane_data[i].data[0],
                                         ring_packet.raw_vid.plane_data[i].size[0]);
                        }
                        if (ring_packet.raw_vid.plane_data[i].size[1])
                        {
                            copy_to_ring((ring_addr)ring_packet.raw_vid.plane_data[i].data[1],
                                         packet.raw_vid.plane_data[i].data[1],
                                         ring_packet.raw_vid.plane_data[i].size[1]);
                        }
                        if (ring_packet.raw_vid.plane_data[i].size[2])
                        {
                            copy_to_ring((ring_addr)ring_packet.raw_vid.plane_data[i].data[2],
                                         packet.raw_vid.plane_data[i].data[2],
                                         ring_packet.raw_vid.plane_data[i].size[2]);
                        }
                    }
                }
                break;
                case EN_VIDEO_CODEC_DATA:
                {
                    for (unsigned int i = 0; i < packet.es_vid_i.packet_count; i++)
                    {
                        copy_to_ring((ring_addr)ring_packet.es_vid.packet_data[i].data, packet.es_vid.packet_data[i].data, packet.es_vid_i.packet_info[i].size);
                    }
                }
                break;
                case EN_AUDIO_CODEC_DATA:
                {
                    for (unsigned int i = 0; i < packet.es_aud_i.packet_count; i++)
                    {
                        copy_to_ring((ring_addr)ring_packet.es_aud.packet_data[i].data, packet.es_aud.packet_data[i].data, packet.es_aud_i.packet_info[i].size);
                    }
                }
                break;
                case EN_USER_META_DATA:
                {
                    copy_to_ring((ring_addr)ring_packet.meta_data.data, packet.meta_data.data, packet.meta_data_i.size);
                }
                break;
                default:
                    cout << "FMT " << packet.en_type << " ERROR!" << endl;
                    return -1;
            }
            return 0;
        }
    private:
        std::map<void *, ring_addr> map_packet_ring_addr;
};
#endif
