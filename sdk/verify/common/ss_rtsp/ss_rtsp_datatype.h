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

#include <pthread.h>

#ifndef __SS_RTSP_DATATYPE_H__
#define __SS_RTSP_DATATYPE_H__

#define ES_PACKET_COUNT_MAX 16

//We use NAL_UNIT_RESERVED_30 for slice end
#define H264_FRAME_END_PAYLOAD(__declare) char __declare[4] = {0x1E, 0x00, 0x53, 0x53}

#define H265_FRAME_END_PAYLOAD(__declare) char __declare[4] = {0x3C, 0x00, 0x53, 0x53}

#define CHECK_H264_FRAME_END_PAYLOAD(__buf) (((__buf)[3] == 0x53) && ((__buf)[2] == 0x53) \
                                            && ((__buf)[1] == 0x00) && ((__buf)[0] == 0x1E))

#define CHECK_H265_FRAME_END_PAYLOAD(__buf) (((__buf)[3] == 0x53) && ((__buf)[2] == 0x53) \
                                            && ((__buf)[1] == 0x00) && ((__buf)[0] == 0x3C))

enum rtsp_vid_es_fmt
{
    RTSP_ES_FMT_VIDEO_NONE = 0x0,
    RTSP_ES_FMT_VIDEO_H264 = 0x1,
    RTSP_ES_FMT_VIDEO_H265,
    RTSP_ES_FMT_VIDEO_JPEG,
    RTSP_ES_FMT_VIDEO_AV1,
    RTSP_ES_FMT_VIDEO_VP9
};

enum rtsp_aud_es_fmt
{
    RTSP_ES_FMT_AUDIO_NONE = 0x0,
    RTSP_ES_FMT_AUDIO_PCM  = 0x1,
    RTSP_ES_FMT_AUDIO_AAC,
    RTSP_ES_FMT_AUDIO_PCMU,
    RTSP_ES_FMT_AUDIO_PCMA,
    RTSP_ES_FMT_AUDIO_G726,
};

struct rtsp_video_info
{
    unsigned int format;
    unsigned int frame_rate;
    unsigned int width;
    unsigned int height;
};

struct rtsp_audio_info
{
    unsigned int  format;
    unsigned int  channels;
    unsigned int  sample_rate;  //44000/48000/44100
    unsigned char sample_width; //8/10/16 bit
};

struct rtsp_pool_config
{
    unsigned int depth;
};

struct rtsp_frame_packet_data
{
    unsigned char b_end;
    unsigned int  size;
    char *        data;
};

struct rtsp_frame_packet
{
    unsigned int                  packet_count;
    struct rtsp_frame_packet_data packet_data[ES_PACKET_COUNT_MAX];
    struct timeval                stamp;
};

struct rtsp_audio_output
{
    struct rtsp_audio_info   info;
    struct rtsp_frame_packet frame_package;
};

struct rtsp_video_output
{
    struct rtsp_video_info   info;
    struct rtsp_frame_packet frame_package;
};

#endif
