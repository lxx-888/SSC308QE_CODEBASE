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

#ifndef __SS_RTSP_H__
#define __SS_RTSP_H__

#include <sys/time.h>
#include <map>
#include <list>
#include <string>
#include <vector>
#include <iostream>

#include "ss_rtsp_datatype.h"
#include "ss_rtsp_packet.h"

class ss_rtsp
{
    public:
        struct frame_package_head
        {
            struct rtsp_pool_config frame_pool_config;
            const char *            frame_url_prefix;
            void *                  frame_obj_pool;
        };

        struct frame_package_lock
        {
            pthread_mutex_t    frame_mutex;
            pthread_cond_t     frame_cond;
            pthread_condattr_t frame_cond_attr;
        };

        struct rtsp_video_input
        {
            struct rtsp_video_info    info;
            struct frame_package_head frame_package;
            void                      *session;
        };

        struct rtsp_audio_input
        {
            struct rtsp_audio_info    info;
            struct frame_package_head frame_package;
            void                      *session;
        };

        struct rtsp_input_info
        {
            struct rtsp_video_input video_input;
            struct rtsp_audio_input audio_input;
            std::string             *url_prefix;
            void                    *media_session;
        };

        explicit ss_rtsp();
        virtual ~ss_rtsp();
        int add_video_server_url(const std::string &url, const struct rtsp_video_info &info);
        int add_audio_server_url(const std::string &url, const struct rtsp_audio_info &info);
        int del_video_server_url(const std::string &url);
        int del_audio_server_url(const std::string &url);
        void *get_video_pool_handle(const std::string &url);
        void *get_audio_pool_handle(const std::string &url);

        const char *get_url_prefix(void *handle);
        void config_pool(void *handle, const struct rtsp_pool_config &config);
        int check_pool_package(void *handle);
        void send_pool_package(void *handle, rtsp_packet &obj);
        void send_pool_package(void *handle, const struct rtsp_frame_packet &frame_package);

        // the less maxBufDiv the more buffer size, max is w * h * 10 if maxBufDiv = 0
        int start_server(unsigned int maxBufDiv = 20);
        int stop_server();

        int all_url(std::vector<std::string> &url_array);

        virtual void connect_audio_stream(const std::string &url) = 0;
        virtual void disconnect_audio_stream(const std::string &url) = 0;
        virtual void connect_video_stream(const std::string &url) = 0;
        virtual void disconnect_video_stream(const std::string &url) = 0;

        struct frame_package_lock frame_lock;

    private:
        void end_frame_object_pool(void);
        bool check_wait_frame_object_pool(void);
        void clear_frame_object_pool(void);

        static void *open_video_stream(char const * stream_name, void * arg);
        static void *open_audio_stream(char const * stream_name, void * arg);
        static int read_stream(void *handle, unsigned char *out_buf,
                                int len, struct timeval *time_stamp, void *arg);
        static int close_stream(void *handle, void *arg);

        std::map<std::string, struct rtsp_input_info> rtsp_input;
        void *live555_srv;
};

class ss_rtsp_client
{
    public:
        explicit ss_rtsp_client();
        virtual ~ss_rtsp_client();
        int play(const std::string &url, const std::string &userName, const std::string &pwd, unsigned int width = 0,
                 unsigned int height = 0, unsigned int bufDiv = 20);
        int stop(const std::string &url);

        virtual int recv_video_package(const std::string &url, const struct rtsp_video_output &video_output, unsigned int frame_id) = 0;
        virtual int recv_audio_package(const std::string &url, const struct rtsp_audio_output &audio_output, unsigned int frame_id) = 0;
    private:
        std::map<std::string, void *> rtsp_output;
};

#endif //__SS_RTSP_H__

