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

#ifndef __SS_RTSP_PACKET_H__
#define __SS_RTSP_PACKET_H__

#include <string.h>
#include <assert.h>
#include <cassert>
#include <vector>
#include <memory>
#include <new>
#include <string>
#include <type_traits>
#include "ss_rtsp_datatype.h"

class ss_rtsp_packet;
using rtsp_packet = std::shared_ptr<ss_rtsp_packet>;

struct rtsp_es_slice_data
{
    unsigned char b_nalu_start;
    char         *data;
    unsigned int  size;
};
class rtsp_data_parser
{
public:
    rtsp_data_parser();
    virtual ~rtsp_data_parser();
    virtual int parse_nalu(const struct rtsp_frame_packet &packet, std::vector<struct rtsp_es_slice_data> &frame);
};
class rtsp_h264or5_data_parser : public rtsp_data_parser
{
public:
    virtual int parse_nalu(const struct rtsp_frame_packet &packet, std::vector<struct rtsp_es_slice_data> &frame) final;
private:
    virtual bool check_final_nalu_type(char type) = 0;
    virtual void add_frame_end_flag(std::vector<struct rtsp_es_slice_data> &frame) = 0;

};
class rtsp_h264_data_parser : public rtsp_h264or5_data_parser
{
private:
    bool check_final_nalu_type(char type) final;
    void add_frame_end_flag(std::vector<struct rtsp_es_slice_data> &frame) final;

};
class rtsp_h265_data_parser : public rtsp_h264or5_data_parser
{
private:
    bool check_final_nalu_type(char type) final;
    void add_frame_end_flag(std::vector<struct rtsp_es_slice_data> &frame) final;
};
class ss_rtsp_packet
{
public:
    template <typename T, typename... Args>
    static std::shared_ptr<T> make(Args&&... args)
    {
        static_assert(std::is_base_of<ss_rtsp_packet, T>::value, "T must be derived from ss_rtsp_packet");
        return std::make_shared<T>(std::forward<Args>(args)...);
    }
    std::vector<struct rtsp_es_slice_data> frame;
    struct timeval time_stamp;
    explicit ss_rtsp_packet(const struct timeval &stamp);
    virtual ~ss_rtsp_packet();
};

template <typename T>
class ss_rtsp_normal_packet: public ss_rtsp_packet
{
public:
    explicit ss_rtsp_normal_packet(const struct rtsp_frame_packet &packet)
        : ss_rtsp_packet(packet.stamp)
    {
        T parser;
        parser.parse_nalu(packet, this->frame);
        for (auto &it : frame)
        {
            char *tmp = it.data;
            it.data = new char[it.size];
            assert(it.data);
            memcpy(it.data, tmp, it.size);
        }
    }
    ~ss_rtsp_normal_packet()
    {
        for (auto &it : frame)
        {
            delete []it.data;
        }
    }
};
#endif //__SS_RTSP_PACKET_H__
