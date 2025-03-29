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

#ifndef __SS_RTSP_C_WRAPPER_H__
#define __SS_RTSP_C_WRAPPER_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "ss_rtsp_datatype.h"
typedef void * ss_rtsp_handle;

struct rtsp_connection_fp
{
    void (*connect)(const char *url, void *user_data);
    void (*disconnect)(const char *url, void *user_data);
    void *user_data;
};

/*Server Configuration*/
ss_rtsp_handle ss_rtsp_server_create(void);
int ss_rtsp_server_start(ss_rtsp_handle handle);
int ss_rtsp_server_stop(ss_rtsp_handle handle);
void ss_rtsp_server_destroy(ss_rtsp_handle handle);

/*URL Configuration*/
int ss_rtsp_server_add_video_url(ss_rtsp_handle handle, const char *url, const struct rtsp_video_info *info, struct rtsp_connection_fp fp);
int ss_rtsp_server_add_audio_url(ss_rtsp_handle handle, const char *url, const struct rtsp_audio_info *info, struct rtsp_connection_fp fp);
int ss_rtsp_server_del_video_url(ss_rtsp_handle handle, const char *url);
int ss_rtsp_server_del_audio_url(ss_rtsp_handle handle, const char *url);

const char * ss_rtsp_server_get_url_prefix(ss_rtsp_handle handle, void *pool_handle);

/*Server Pool Configuration*/
void *ss_rtsp_server_get_video_pool(ss_rtsp_handle handle, const char *url);
void *ss_rtsp_server_get_audio_pool(ss_rtsp_handle handle, const char *url);
void ss_rtsp_server_config_pool(ss_rtsp_handle handle, void *pool_handle, const struct rtsp_pool_config *config);
int ss_rtsp_server_check_pool(ss_rtsp_handle handle, void *pool_handle);
void ss_rtsp_server_send_package(ss_rtsp_handle handle, void *pool_handle, const struct rtsp_frame_packet *frame_package);

typedef void *(*recv_video_pkg)(const char *, const struct rtsp_video_output *, unsigned int);
typedef void *(*recv_audio_pkg)(const char *, const struct rtsp_audio_output *, unsigned int);

/*Rtsp Client Configuration*/
ss_rtsp_handle ss_rtsp_client_create(recv_video_pkg recv_video, recv_audio_pkg recv_audio);
void ss_rtsp_clent_destroy(ss_rtsp_handle handle);

int ss_rtsp_clent_play(ss_rtsp_handle handle, const char *url, const char *user, const char *pwd);
int ss_rtsp_clent_stop(ss_rtsp_handle handle, const char *url);

#ifdef __cplusplus
}
#endif

#endif //__SS_RTSP_C_WRAPPER_H__
