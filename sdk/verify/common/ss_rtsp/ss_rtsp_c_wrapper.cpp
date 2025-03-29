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
#include <stdio.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <map>
#include <string>
#include <assert.h>

#include "ss_rtsp.h"
#include "ss_rtsp_c_wrapper.h"

class ss_rtsp_c_wrapper :  public ss_rtsp
{
    public:
        ss_rtsp_c_wrapper() {}
        virtual ~ss_rtsp_c_wrapper() {}
        std::map<std::string, struct rtsp_connection_fp> video_connect;
        std::map<std::string, struct rtsp_connection_fp> audio_connect;
    private:
        void connect_video_stream(const std::string &url);
        void disconnect_video_stream(const std::string &url);
        void connect_audio_stream(const std::string &url);
        void disconnect_audio_stream(const std::string &url);
};

void ss_rtsp_c_wrapper::connect_video_stream(const std::string &url)
{
    auto iter = video_connect.find(url);

    if (iter == video_connect.end())
    {
        std::cout << "Not found: " << url << std::endl;
        return;
    }
    assert(iter->second.connect);

    iter->second.connect(url.c_str(), iter->second.user_data);
}

void ss_rtsp_c_wrapper::disconnect_video_stream(const std::string &url)
{
    auto iter = video_connect.find(url);

    if (iter == video_connect.end())
    {
        std::cout << "Not found: " << url << std::endl;
        return;
    }
    assert(iter->second.disconnect);

    iter->second.disconnect(url.c_str(), iter->second.user_data);
}

void ss_rtsp_c_wrapper::connect_audio_stream(const std::string &url)
{
    auto iter = audio_connect.find(url);

    if (iter == audio_connect.end())
    {
        std::cout << "Not found: " << url << std::endl;
        return;
    }
    assert(iter->second.connect);

    iter->second.connect(url.c_str(), iter->second.user_data);
}

void ss_rtsp_c_wrapper::disconnect_audio_stream(const std::string &url)
{
    auto iter = audio_connect.find(url);

    if (iter == audio_connect.end())
    {
        std::cout << "Not found: " << url << std::endl;
        return;
    }
    assert(iter->second.disconnect);

    iter->second.disconnect(url.c_str(), iter->second.user_data);
}

ss_rtsp_handle ss_rtsp_server_create(void)
{
    ss_rtsp_c_wrapper *ss_rtsp_srv = new ss_rtsp_c_wrapper();
    if(ss_rtsp_srv == NULL)
    {
        printf("new ss_rtsp error\n");
        return NULL;
    }

    return (ss_rtsp_handle)ss_rtsp_srv;
}

int ss_rtsp_server_start(ss_rtsp_handle handle)
{
    int ret = 0;
    ss_rtsp_c_wrapper *ss_rtsp_srv = (ss_rtsp_c_wrapper *)handle;

    ret = ss_rtsp_srv->start_server();

    return ret;
}

int ss_rtsp_server_stop(ss_rtsp_handle handle)
{
    int ret = 0;
    ss_rtsp_c_wrapper *ss_rtsp_srv = (ss_rtsp_c_wrapper *)handle;

    ret = ss_rtsp_srv->stop_server();

    return ret;
}

void ss_rtsp_server_destroy(ss_rtsp_handle handle)
{
    ss_rtsp_c_wrapper *ss_rtsp_srv = (ss_rtsp_c_wrapper *)handle;

    if(ss_rtsp_srv == NULL)
    {
        printf("destroy ss_rtsp error, handle null\n");
        return ;
    }

    delete ss_rtsp_srv;
    ss_rtsp_srv = NULL;
}

/*URL Configuration*/
int ss_rtsp_server_add_video_url(ss_rtsp_handle handle, const char *url, const struct rtsp_video_info *info, struct rtsp_connection_fp fp)
{
    int ret = 0;

    ss_rtsp_c_wrapper *ss_rtsp_srv = (ss_rtsp_c_wrapper *)handle;

    ret = ss_rtsp_srv->add_video_server_url(url, *info);
    if(ret < 0)
    {
        return -1;
    }

    ss_rtsp_srv->video_connect[url] = fp;

    return ret;
}

int ss_rtsp_server_add_audio_url(ss_rtsp_handle handle, const char *url, const struct rtsp_audio_info *info, struct rtsp_connection_fp fp)
{
    int ret = 0;

    ss_rtsp_c_wrapper *ss_rtsp_srv = (ss_rtsp_c_wrapper *)handle;

    ret = ss_rtsp_srv->add_audio_server_url(url, *info);
    if(ret < 0)
    {
        return -1;
    }

    ss_rtsp_srv->audio_connect[url] = fp;

    return ret;
}

int ss_rtsp_server_del_video_url(ss_rtsp_handle handle, const char *url)
{
    int ret = 0;

    ss_rtsp_c_wrapper *ss_rtsp_srv = (ss_rtsp_c_wrapper *)handle;

    ret = ss_rtsp_srv->del_video_server_url(url);
    if(ret < 0)
    {
        return -1;
    }

    auto iter = ss_rtsp_srv->video_connect.find(url);

    if (iter == ss_rtsp_srv->video_connect.end())
    {
        std::cout << "Not found: " << url << std::endl;
        return -1;
    }
    ss_rtsp_srv->video_connect.erase(iter);

    return 0;
}

int ss_rtsp_server_del_audio_url(ss_rtsp_handle handle, const char *url)
{
    int ret = 0;

    ss_rtsp_c_wrapper *ss_rtsp_srv = (ss_rtsp_c_wrapper *)handle;

    ret = ss_rtsp_srv->del_audio_server_url(url);
    if(ret < 0)
    {
        return -1;
    }

    auto iter = ss_rtsp_srv->audio_connect.find(url);

    if (iter == ss_rtsp_srv->audio_connect.end())
    {
        std::cout << "Not found: " << url << std::endl;
        return -1;
    }
    ss_rtsp_srv->audio_connect.erase(iter);

    return 0;
}

/*Server Pool Configuration*/
void *ss_rtsp_server_get_video_pool(ss_rtsp_handle handle, const char *url)
{
    ss_rtsp_c_wrapper *ss_rtsp_srv = (ss_rtsp_c_wrapper *)handle;

    return ss_rtsp_srv->get_video_pool_handle(url);
}

void *ss_rtsp_server_get_audio_pool(ss_rtsp_handle handle, const char *url)
{
    ss_rtsp_c_wrapper *ss_rtsp_srv = (ss_rtsp_c_wrapper *)handle;

    return ss_rtsp_srv->get_audio_pool_handle(url);
}


void ss_rtsp_server_config_pool(ss_rtsp_handle handle, void *pool_handle, const struct rtsp_pool_config *config)
{
    ss_rtsp_c_wrapper *ss_rtsp_srv = (ss_rtsp_c_wrapper *)handle;

    ss_rtsp_srv->config_pool(pool_handle, *config);
}

int ss_rtsp_server_check_pool(ss_rtsp_handle handle, void *pool_handle)
{
    ss_rtsp_c_wrapper *ss_rtsp_srv = (ss_rtsp_c_wrapper *)handle;

    if(ss_rtsp_srv->check_pool_package(pool_handle))
    {
        return -1;
    }
    return 0;
}

void ss_rtsp_server_send_package(ss_rtsp_handle handle, void *pool_handle, const struct rtsp_frame_packet *frame_package)
{
    ss_rtsp_c_wrapper *ss_rtsp_srv = (ss_rtsp_c_wrapper *)handle;
    ss_rtsp_srv->send_pool_package(pool_handle, *frame_package);
}

const char *ss_rtsp_server_get_url_prefix(ss_rtsp_handle handle, void *pool_handle)
{
    ss_rtsp_c_wrapper *ss_rtsp_srv = (ss_rtsp_c_wrapper *)handle;

    return ss_rtsp_srv->get_url_prefix(pool_handle);
}


