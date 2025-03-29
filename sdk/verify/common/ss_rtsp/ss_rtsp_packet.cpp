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
#include <iostream>
#include "ss_rtsp_packet.h"

// We use special value to mark frame end in order to make client decode faster.
static H264_FRAME_END_PAYLOAD(g_h264_frame_end_flag);
static H265_FRAME_END_PAYLOAD(g_h265_frame_end_flag);

rtsp_data_parser::rtsp_data_parser()
{
}
rtsp_data_parser::~rtsp_data_parser()
{
}
int rtsp_data_parser::parse_nalu(const struct rtsp_frame_packet &es_packet, std::vector<struct rtsp_es_slice_data> &frame)
{
    for (unsigned int i = 0; i < es_packet.packet_count; i++)
    {
        struct rtsp_es_slice_data slice;
        slice.b_nalu_start = (i == 0);
        slice.data         = es_packet.packet_data[i].data;
        slice.size         = es_packet.packet_data[i].size;
        frame.push_back(slice);
    }
    return 0;
}
int rtsp_h264or5_data_parser::parse_nalu(const struct rtsp_frame_packet &es_packet, std::vector<struct rtsp_es_slice_data> &frame)
{
    const char          *bs = NULL;
    const char          *ls = NULL;
    int               state = 0;
    bool    find_final_nalu = false;

    if (!es_packet.packet_count)
    {
        return -1;
    }
    for (unsigned int i = 0; i < es_packet.packet_count; i++)
    {
        struct rtsp_es_slice_data slice;
        unsigned int start_code_sz = 0;

        slice.data         = es_packet.packet_data[i].data;
        slice.size         = es_packet.packet_data[i].size;
        slice.b_nalu_start = false;
        bs                 = slice.data;
        ls                 = slice.data + slice.size - 1;
        if (NULL == bs || NULL == ls)
        {
            return -1;
        }
        while (bs < ls && !find_final_nalu)
        {
            switch (state)
            {
            case 0:
            case 1:
                state = (0x00 == *bs) ? (state + 1) : 0;
                break;
            case 2:
                if (*bs == 0x01)
                {
                    state         = 4;
                    start_code_sz = 3;
                    break;
                }
                state = (0x00 == *bs) ? (state + 1) : 0;
                break;
            case 3:
                state = (0x01 == *bs) ? (state + 1) : 0;
                start_code_sz = 4;
                break;
            case 4:
                if (bs - start_code_sz > slice.data)
                {
                    slice.size = bs - start_code_sz - slice.data;
                    frame.push_back(slice);
                    //std::cout << std::hex << (int)*slice.data << std::dec << ", size: " << slice.size << std::endl;
                }
                slice.data         = (char *)bs;
                slice.size         = ls - bs + 1;
                slice.b_nalu_start = true;
                find_final_nalu    = this->check_final_nalu_type(*bs);
                state = 0;
                break;
            }
            bs++;
        }
        //std::cout << std::hex << (int)*slice.data << std::dec << ", size: " << slice.size << std::endl;
        frame.push_back(slice);
        if (es_packet.packet_data[i].b_end)
        {
            this->add_frame_end_flag(frame);
        }
    }
    return 0;
}
bool rtsp_h264_data_parser::check_final_nalu_type(char type)
{
    enum AVCNalUnitType
    {
        AVC_NAL_UNIT_NONIDR = 1,
        AVC_NAL_UNIT_IDR    = 5,
        AVC_NAL_UNIT_SEI,   //6
        AVC_NAL_UNIT_SPS,   //7
        AVC_NAL_UNIT_PPS,   //8
        AVC_NAL_UNIT_AUD,   //9
    };
    unsigned char nalu_type = (unsigned char)type & 0x1F;
    return (AVC_NAL_UNIT_IDR == nalu_type || AVC_NAL_UNIT_NONIDR == nalu_type);
}
void rtsp_h264_data_parser::add_frame_end_flag(std::vector<struct rtsp_es_slice_data> &frame)
{
    struct rtsp_es_slice_data slice;

    slice.data         = g_h264_frame_end_flag;
    slice.b_nalu_start = true;
    slice.size         = 4;
    frame.push_back(slice);
}
bool rtsp_h265_data_parser::check_final_nalu_type(char type)
{
    enum HEVCNalUnitType
    {
        HEVC_NAL_UNIT_CODED_SLICE_TRAIL_N = 0,   // 0
        HEVC_NAL_UNIT_CODED_SLICE_TRAIL_R,       // 1

        HEVC_NAL_UNIT_CODED_SLICE_TSA_N,         // 2
        HEVC_NAL_UNIT_CODED_SLICE_TLA,           // 3    Current name in the spec: TSA_R

        HEVC_NAL_UNIT_CODED_SLICE_STSA_N,        // 4
        HEVC_NAL_UNIT_CODED_SLICE_STSA_R,        // 5

        HEVC_NAL_UNIT_CODED_SLICE_RADL_N,        // 6
        HEVC_NAL_UNIT_CODED_SLICE_DLP,           // 7    Current name in the spec: RADL_R

        HEVC_NAL_UNIT_CODED_SLICE_RASL_N,        // 8
        HEVC_NAL_UNIT_CODED_SLICE_TFD,           // 9    Current name in the spec: RASL_R

        HEVC_NAL_UNIT_CODED_SLICE_BLA = 16,      // 16   Current name in the spec: BLA_W_LP
        HEVC_NAL_UNIT_CODED_SLICE_BLANT,         // 17   Current name in the spec: BLA_W_DLP
        HEVC_NAL_UNIT_CODED_SLICE_BLA_N_LP,      // 18
        HEVC_NAL_UNIT_CODED_SLICE_IDR,           // 19   Current name in the spec: IDR_W_DLP
        HEVC_NAL_UNIT_CODED_SLICE_IDR_N_LP,      // 20
        HEVC_NAL_UNIT_CODED_SLICE_CRA,           // 21
        HEVC_NAL_UNIT_RESERVED_22,
        HEVC_NAL_UNIT_RESERVED_23,

        HEVC_NAL_UNIT_VPS = 32,                  // 32
        HEVC_NAL_UNIT_SPS,                       // 33
        HEVC_NAL_UNIT_PPS,                       // 34
        HEVC_NAL_UNIT_ACCESS_UNIT_DELIMITER,     // 35
        HEVC_NAL_UNIT_EOS,                       // 36
        HEVC_NAL_UNIT_EOB,                       // 37
        HEVC_NAL_UNIT_FILLER_DATA,               // 38
        HEVC_NAL_UNIT_SEI,                       // 39   Prefix SEI
        HEVC_NAL_UNIT_SEI_SUFFIX,                // 40   Suffix SEI
    };
    unsigned char nalu_type = ((unsigned char)type >> 1) & 0x3f;
    return (nalu_type <= HEVC_NAL_UNIT_CODED_SLICE_CRA);
}
void rtsp_h265_data_parser::add_frame_end_flag(std::vector<struct rtsp_es_slice_data> &frame)
{
    struct rtsp_es_slice_data slice;

    slice.data         = g_h265_frame_end_flag;
    slice.b_nalu_start = true;
    slice.size         = 4;
    frame.push_back(slice);
}
ss_rtsp_packet::ss_rtsp_packet(const struct timeval &stamp)
    : time_stamp(stamp)
{
}

ss_rtsp_packet::~ss_rtsp_packet()
{
}

