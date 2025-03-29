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

#include "ss_filter.h"
#include <iostream>

enum AVCNalUnitType
{
    AVC_NAL_UNIT_NONIDR = 1,
    AVC_NAL_UNIT_IDR    = 5,
    AVC_NAL_UNIT_SEI, // 6
    AVC_NAL_UNIT_SPS, // 7
    AVC_NAL_UNIT_PPS, // 8
    AVC_NAL_UNIT_AUD, // 9
};

enum HEVCNalUnitType
{
    HEVC_NAL_UNIT_CODED_SLICE_TRAIL_N = 0, // 0
    HEVC_NAL_UNIT_CODED_SLICE_TRAIL_R,     // 1

    HEVC_NAL_UNIT_CODED_SLICE_TSA_N, // 2
    HEVC_NAL_UNIT_CODED_SLICE_TLA,   // 3    Current name in the spec: TSA_R

    HEVC_NAL_UNIT_CODED_SLICE_STSA_N, // 4
    HEVC_NAL_UNIT_CODED_SLICE_STSA_R, // 5

    HEVC_NAL_UNIT_CODED_SLICE_RADL_N, // 6
    HEVC_NAL_UNIT_CODED_SLICE_DLP,    // 7    Current name in the spec: RADL_R

    HEVC_NAL_UNIT_CODED_SLICE_RASL_N, // 8
    HEVC_NAL_UNIT_CODED_SLICE_TFD,    // 9    Current name in the spec: RASL_R

    HEVC_NAL_UNIT_CODED_SLICE_BLA = 16, // 16   Current name in the spec: BLA_W_LP
    HEVC_NAL_UNIT_CODED_SLICE_BLANT,    // 17   Current name in the spec: BLA_W_DLP
    HEVC_NAL_UNIT_CODED_SLICE_BLA_N_LP, // 18
    HEVC_NAL_UNIT_CODED_SLICE_IDR,      // 19   Current name in the spec: IDR_W_DLP
    HEVC_NAL_UNIT_CODED_SLICE_IDR_N_LP, // 20
    HEVC_NAL_UNIT_CODED_SLICE_CRA,      // 21
    HEVC_NAL_UNIT_RESERVED_22,
    HEVC_NAL_UNIT_RESERVED_23,

    HEVC_NAL_UNIT_VPS = 32,              // 32
    HEVC_NAL_UNIT_SPS,                   // 33
    HEVC_NAL_UNIT_PPS,                   // 34
    HEVC_NAL_UNIT_ACCESS_UNIT_DELIMITER, // 35
    HEVC_NAL_UNIT_EOS,                   // 36
    HEVC_NAL_UNIT_EOB,                   // 37
    HEVC_NAL_UNIT_FILLER_DATA,           // 38
    HEVC_NAL_UNIT_SEI,                   // 39   Prefix SEI
    HEVC_NAL_UNIT_SEI_SUFFIX,            // 40   Suffix SEI
};

static unsigned char CheckNalType(es_video_fmt fmt, const char *data)
{
    unsigned char naluType = 0;

    switch (fmt)
    {
        case ES_STREAM_H264:
            naluType = (unsigned char)*data & 0x1F;
            return (AVC_NAL_UNIT_SPS == naluType || AVC_NAL_UNIT_PPS == naluType);
            break;
        case ES_STREAM_H265:
            naluType = ((unsigned char)*data >> 1) & 0x3f;
            return (HEVC_NAL_UNIT_VPS == naluType || HEVC_NAL_UNIT_SPS == naluType || HEVC_NAL_UNIT_PPS == naluType);
            break;
        default:
            std::cout << "Error es fmt " << fmt << std::endl;
            break;
    }
    return 0;
}

static bool FindNal(const es_video_info &info, const es_video_packet &data)
{
    const char *bs    = NULL;
    const char *ls    = NULL;
    int         state = 0;

    for (unsigned int i = 0; i < info.packet_count; i++)
    {
        bs = data.packet_data[i].data;
        ls = data.packet_data[i].data + info.packet_info[i].size - 1;
        if (NULL == bs || NULL == ls)
        {
            return false;
        }
        while (bs < ls)
        {
            switch (state)
            {
                case 0:
                case 1:
                {
                    state = 0x00 == *bs ? state + 1 : 0;
                }
                break;

                case 2:
                {
                    state = 0x01 == *bs ? 4 : 0x00 == *bs ? 3 : 0;
                }
                break;

                case 3:
                {
                    state = 0x01 == *bs ? 4 : 0;
                }
                break;

                case 4:
                {
                    if (CheckNalType(info.fmt, bs))
                    {
                        return true;
                    }
                    state = 0;
                }
                break;
            }
            bs++;
        }
    }
    return false;
}

ss_video_packet_filter::ss_video_packet_filter() : skip_count(0), has_once_idr(false) {}
ss_video_packet_filter::~ss_video_packet_filter() {}
bool ss_video_packet_filter::check(stream_packet_obj &packet)
{
    if (packet->en_type != EN_VIDEO_CODEC_DATA)
    {
        return true;
    }
    if (packet->es_vid_i.fmt != ES_STREAM_H264 && packet->es_vid_i.fmt != ES_STREAM_H265)
    {
        return true;
    }
    if (this->has_once_idr)
    {
        return true;
    }
    if (FindNal(packet->es_vid_i, packet->es_vid))
    {
        this->has_once_idr = true;
        std::cout << "Find the first valid video packet data, " << this->skip_count << " packet data has been skiped."
                  << std::endl;
        return true;
    }
    ++this->skip_count;
    return false;
}
void ss_video_packet_filter::reset(void)
{
    this->has_once_idr = false;
    this->skip_count   = 0;
    std::cout << "Reset video packet filter" << std::endl;
}
