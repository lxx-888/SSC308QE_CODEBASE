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
#include <assert.h>
#include <memory>
#include <vector>

#include "ss_connector.h"
#include "ss_cmd_base.h"
#include "ss_packet.h"
#include "ss_rtsp.h"

struct ss_cmd_rtsp_in_desc
{
    class ss_cmd_rtsp  *this_class;
    class ss_connector *connector;
    void               *pool_handle;
    std::string        str_ref;
    ss_cmd_rtsp_in_desc ()
    {
        this_class  = NULL;
        connector   = NULL;
        pool_handle = NULL;
    }
};

struct ss_cmd_rtsp_out_desc
{
    unsigned int    output_ref;
    ss_connector *  vid_connector;
    ss_connector *  aud_connector;
    std::string     url;
    ss_cmd_rtsp *   this_class;
    ss_cmd_rtsp_out_desc() : output_ref(0) , vid_connector(nullptr), aud_connector(nullptr)
    {
    }
};

class rtsp_stream_packet : public stream_packet_base
{
public:
    rtsp_stream_packet(const struct rtsp_video_output &video_output) : stream_packet_base()
    {
        this->en_type = EN_VIDEO_CODEC_DATA;
        if (video_output.info.format == RTSP_ES_FMT_VIDEO_H264)
        {
            this->es_vid_i.fmt = ES_STREAM_H264;
        }
        else if (video_output.info.format == RTSP_ES_FMT_VIDEO_H265)
        {
            this->es_vid_i.fmt = ES_STREAM_H265;
        }
        else if (video_output.info.format == RTSP_ES_FMT_VIDEO_JPEG)
        {
            this->es_vid_i.fmt = ES_STREAM_JPEG;
        }
        else if (video_output.info.format == RTSP_ES_FMT_VIDEO_AV1)
        {
            this->es_vid_i.fmt = ES_STREAM_AV1;
        }
        else if (video_output.info.format == RTSP_ES_FMT_VIDEO_VP9)
        {
            this->es_vid_i.fmt = ES_STREAM_VP9;
        }
        else
        {
            std::cout << "Not support current video fmt." << std::endl;
            throw err_buf();
        }
        this->es_vid_i.width        = video_output.info.width;
        this->es_vid_i.height       = video_output.info.height;
        this->es_vid_i.b_head       = false;
        this->es_vid_i.packet_count = video_output.frame_package.packet_count;
        for (unsigned int i = 0; i < this->es_vid_i.packet_count; i++)
        {
            this->es_vid_i.packet_info[i].b_end = video_output.frame_package.packet_data[i].b_end;
            this->es_vid_i.packet_info[i].size  = video_output.frame_package.packet_data[i].size;
            this->es_vid.packet_data[i].data    = video_output.frame_package.packet_data[i].data;
        }
        this->set_time_stamp(video_output.frame_package.stamp);
    }
    rtsp_stream_packet(const struct rtsp_audio_output &audio_output) : stream_packet_base()
    {
        this->en_type = EN_AUDIO_CODEC_DATA;
        if (audio_output.info.format == RTSP_ES_FMT_AUDIO_PCM)
        {
            this->es_aud_i.fmt = ES_STREAM_PCM;
        }
        else if (audio_output.info.format == RTSP_ES_FMT_AUDIO_AAC)
        {
            this->es_aud_i.fmt = ES_STREAM_AAC;
        }
        else
        {
            std::cout << "Not support current audio fmt." << std::endl;
            throw err_buf();
        }
        this->es_aud_i.sample_rate  = audio_output.info.sample_rate;
        this->es_aud_i.sample_width = audio_output.info.sample_width;
        this->es_aud_i.channels     = audio_output.info.channels;
        this->es_aud_i.packet_count = audio_output.frame_package.packet_count;
        for (unsigned int i = 0; i < this->es_aud_i.packet_count; i++)
        {
            this->es_aud_i.packet_info[i].size  = audio_output.frame_package.packet_data[i].size;
            this->es_aud.packet_data[i].data    = audio_output.frame_package.packet_data[i].data;
        }
        this->set_time_stamp(audio_output.frame_package.stamp);
    }
    virtual ~rtsp_stream_packet() {}
    virtual stream_packet_obj dup() const
    {
        return stream_packet_base::make<stream_packet_clone>(*this);
    }
private:
    void update_time_stamp() override final
    {
    }
};

class ss_cmd_rtsp : public ss_handle, private ss_rtsp, private ss_rtsp_client
{
public:
    ss_cmd_rtsp() : video_server_depth(0), audio_server_depth(0)
    {
    }
    virtual ~ss_cmd_rtsp()
    {
        for (auto iter = video_in_path_desc.begin(); iter != video_in_path_desc.end(); ++iter)
        {
            if (iter->second.connector)
            {
                iter->second.connector->set_packet_to(iter->second.str_ref, NULL, NULL, NULL);
            }
        }
        for (auto iter = audio_in_path_desc.begin(); iter != audio_in_path_desc.end(); ++iter)
        {
            if (iter->second.connector)
            {
                iter->second.connector->set_packet_to(iter->second.str_ref, NULL, NULL, NULL);
            }
        }
    }
    static int create(const string &handle)
    {
        ss_cmd_rtsp *cmd_rtsp = new ss_cmd_rtsp();
        assert(cmd_rtsp);
        if (!ss_handle::install(handle, cmd_rtsp))
        {
            delete cmd_rtsp;
            return -1;
        }
        return 0;
    }
    int config_video_server(const std::string &url, const struct rtsp_video_info &info)
    {
        return add_video_server_url(url, info);
    }
    int config_audio_server(const std::string &url, const struct rtsp_audio_info &info)
    {
        return add_audio_server_url(url, info);
    }
    int bind_video_in_connector(const std::string &url, int depth, ss_connector *connector, const std::string &str_ref)
    {
        struct rtsp_pool_config pool_config;
        struct ss_cmd_rtsp_in_desc desc;
        memset(&pool_config, 0, sizeof(struct rtsp_pool_config));
        desc.pool_handle = get_video_pool_handle(url);
        desc.connector = connector;
        desc.str_ref = str_ref;
        desc.this_class = this;
        video_in_path_desc[url] = desc;
        video_server_depth = pool_config.depth = depth;
        config_pool(desc.pool_handle, pool_config);
        connector->set_packet_to(str_ref, es_file_writer, es_file_check, &video_in_path_desc[url]);
        return 0;
    }
    int unbind_video_in_connector(std::string &url)
    {
        auto iter = video_in_path_desc.find(url);
        if (iter == video_in_path_desc.end())
        {
            std::cout << "VIDEO CONNECTOR INIT ERROR" << std::endl;
            return -1;
        }
        assert(iter->second.connector);
        iter->second.connector->set_packet_to(iter->second.str_ref, NULL, NULL, NULL);
        video_in_path_desc.erase(iter);
        return 0;
    }
    int bind_audio_in_connector(const std::string &url, int depth, ss_connector *connector, const std::string &str_ref)
    {
        struct rtsp_pool_config pool_config;
        struct ss_cmd_rtsp_in_desc desc;
        memset(&pool_config, 0, sizeof(struct rtsp_pool_config));
        desc.pool_handle = get_audio_pool_handle(url);
        desc.connector = connector;
        desc.str_ref = str_ref;
        desc.this_class = this;
        audio_in_path_desc[url] = desc;
        audio_server_depth = pool_config.depth = depth;
        config_pool(desc.pool_handle, pool_config);
        connector->set_packet_to(str_ref, es_file_writer, es_file_check, &audio_in_path_desc[url]);
        return 0;
    }
    int unbind_audio_in_connector(std::string &url)
    {
        auto iter = audio_in_path_desc.find(url);
        if (iter == audio_in_path_desc.end())
        {
            std::cout << "AUDIO CONNECTOR INIT ERROR" << std::endl;
            return -1;
        }
        assert(iter->second.connector);
        iter->second.connector->set_packet_to(iter->second.str_ref, NULL, NULL, NULL);
        audio_in_path_desc.erase(iter);
        return 0;
    }
    int bind_video_out_connector(const std::string &url, ss_connector *connector)
    {
        out_path_desc[url].vid_connector = connector;
        out_path_desc[url].url           = url;
        out_path_desc[url].this_class    = this;
        return 0;
    }
    int unbind_video_out_connector(const std::string &url)
    {
        auto iter = out_path_desc.find(url);
        if (iter == out_path_desc.end())
        {
            std::cout << "VIDEO CONNECTOR INIT ERROR" << std::endl;
            return -1;
        }
        assert(iter->second.vid_connector);
        iter->second.vid_connector = nullptr;
        if (!iter->second.vid_connector && !iter->second.aud_connector)
        {
            out_path_desc.erase(iter);
        }
        return 0;
    }
    int bind_audio_out_connector(const std::string &url, ss_connector *connector)
    {
        out_path_desc[url].aud_connector = connector;
        out_path_desc[url].url           = url;
        out_path_desc[url].this_class    = this;
        return 0;
    }
    int unbind_audio_out_connector(const std::string &url)
    {
        auto iter = out_path_desc.find(url);
        if (iter == out_path_desc.end())
        {
            std::cout << "VIDEO CONNECTOR INIT ERROR" << std::endl;
            return -1;
        }
        assert(iter->second.aud_connector);
        iter->second.aud_connector = nullptr;
        if (!iter->second.aud_connector && !iter->second.aud_connector)
        {
            out_path_desc.erase(iter);
        }
        return 0;
    }
    int play_rtsp_client(const std::string &url)
    {
        auto it = out_path_desc.find(url);
        if (it == out_path_desc.end())
        {
            std::cout << "URL: " << url << " not found!" << std::endl;
            return -1;
        }
        if (it->second.vid_connector)
        {
            it->second.vid_connector->connect_in(connect_rtsp_reader, disconnect_rtsp_reader,
                                                 (void *)&out_path_desc[url]);
        }
        if (it->second.aud_connector)
        {
            it->second.aud_connector->connect_in(connect_rtsp_reader, disconnect_rtsp_reader,
                                                 (void *)&out_path_desc[url]);
        }
        return 0;
    }
    int stop_rtsp_clent(const std::string &url)
    {
        auto it = out_path_desc.find(url);
        if (it == out_path_desc.end())
        {
            std::cout << "Can not found played url: " << url << '!' << std::endl;
            return -1;
        }
        if (it->second.vid_connector)
        {
            it->second.vid_connector->disconnect_in();
        }
        if (it->second.aud_connector)
        {
            it->second.aud_connector->disconnect_in();
        }
        return 0;
    }
    int start_rtsp_server()
    {
        return ss_rtsp::start_server(10);
    }
    int stop_rtsp_server()
    {
        return ss_rtsp::stop_server();
    }
    int dump_rtsp_server_url(std::vector<string> &urls)
    {
        return ss_rtsp::all_url(urls);
    }
private:
    void connect_video_stream(const std::string &url)
    {
        auto iter = video_in_path_desc.find(url);

        if (iter == video_in_path_desc.end())
        {
            std::cout << "Not found: " << url << std::endl;
            return;
        }
        assert(iter->second.connector);
        iter->second.connector->access();
    }
    void disconnect_video_stream(const std::string &url)
    {
        auto iter = video_in_path_desc.find(url);

        if (iter == video_in_path_desc.end())
        {
            std::cout << "Not found: " << url << std::endl;
            return;
        }
        assert(iter->second.connector);
        iter->second.connector->leave();
    }
    void connect_audio_stream(const std::string &url)
    {
        auto iter = audio_in_path_desc.find(url);

        if (iter == audio_in_path_desc.end())
        {
            std::cout << "Not found: " << url << std::endl;
            return;
        }
        assert(iter->second.connector);
        iter->second.connector->access();
    }
    void disconnect_audio_stream(const std::string &url)
    {
        auto iter = audio_in_path_desc.find(url);

        if (iter == audio_in_path_desc.end())
        {
            std::cout << "Not found: " << url << std::endl;
            return;
        }
        assert(iter->second.connector);
        iter->second.connector->leave();
    }
    const char *video_fmt_to_string(unsigned int fmt)
    {
        if (fmt == RTSP_ES_FMT_VIDEO_H264)
            return "H264";
        if (fmt == RTSP_ES_FMT_VIDEO_H265)
            return "H265";
        if (fmt == RTSP_ES_FMT_VIDEO_JPEG)
            return "JPEG";
        if (fmt == RTSP_ES_FMT_VIDEO_AV1)
            return "AV1";
        if (fmt == RTSP_ES_FMT_VIDEO_VP9)
            return "VP9";
        return "Not support";
    }
    const char *audio_fmt_to_string(unsigned int fmt)
    {
        if (fmt == RTSP_ES_FMT_AUDIO_PCM)
            return "PCM";
        if (fmt == RTSP_ES_FMT_AUDIO_AAC)
            return "AAC";
        return "Not support";
    }
    int recv_video_package(const std::string &url, const struct rtsp_video_output &video_output, unsigned int frame_id) override
    {
        auto iter = out_path_desc.find(url);

        if (iter == out_path_desc.end())
        {
            std::cout << "Video Not found: " << url << " Fid: " << frame_id << " Fmt: " << video_fmt_to_string(video_output.info.format) << std::endl;
            return -1;
        }
        if (iter->second.vid_connector->approch() == -1)
        {
            return -1;
        }
        stream_packet_obj packet = stream_packet_base::make<rtsp_stream_packet>(video_output);
        return iter->second.vid_connector->come(packet);
    }
    int recv_audio_package(const std::string &url, const struct rtsp_audio_output &audio_output, unsigned int frame_id) override
    {
        auto iter = out_path_desc.find(url);

        if (iter == out_path_desc.end())
        {
            std::cout << "Audio Not found: " << url << " Fid: " << frame_id << " Fmt: " << audio_fmt_to_string(audio_output.info.format) << std::endl;
            return -1;
        }
        if (iter->second.vid_connector->approch() == -1)
        {
            return -1;
        }
        stream_packet_obj packet = stream_packet_base::make<rtsp_stream_packet>(audio_output);
        return iter->second.aud_connector->come(packet);
    }
    static int es_file_check(void *user)
    {
        struct ss_cmd_rtsp_in_desc *desc = (struct ss_cmd_rtsp_in_desc *)user;
        assert(desc);
        assert(desc->this_class);
        int ret = desc->this_class->check_pool_package(desc->pool_handle);
        if (ret != 0)
        {
            std::cout << "Retry es frame. Maybe client did not close or the network environment is bad." << std::endl;
        }
        return ret;
    }
    static int es_file_writer(stream_packet_obj &packet, void *user)
    {
        struct ss_cmd_rtsp_in_desc *desc = (struct ss_cmd_rtsp_in_desc *)user;
        struct rtsp_frame_packet frame_package;
        assert(desc);
        assert(desc->this_class);
        switch (packet->en_type)
        {
            case EN_VIDEO_CODEC_DATA:
                {
                    if (packet->es_vid_i.fmt == ES_STREAM_H264
                        || packet->es_vid_i.fmt == ES_STREAM_H265
                        || packet->es_vid_i.fmt == ES_STREAM_JPEG
                        || packet->es_vid_i.fmt == ES_STREAM_AV1
                        || packet->es_vid_i.fmt == ES_STREAM_VP9)
                    {
                        frame_package.packet_count = packet->es_vid_i.packet_count;
                        for (unsigned int i = 0; i < packet->es_vid_i.packet_count; i++)
                        {
                            frame_package.packet_data[i].b_end = packet->es_vid_i.packet_info[i].b_end;
                            frame_package.packet_data[i].size  = packet->es_vid_i.packet_info[i].size;
                            frame_package.packet_data[i].data  = packet->es_vid.packet_data[i].data;
                        }
                    }
                    else
                {
                        std::cout << "Fmt error!" << std::endl;
                        return -1;
                    }
                }
                break;
            case EN_AUDIO_CODEC_DATA:
                {
                    frame_package.packet_count = packet->es_aud_i.packet_count;
                    for (unsigned int i = 0; i < packet->es_aud_i.packet_count; i++)
                    {
                        frame_package.packet_data[i].b_end = true;
                        frame_package.packet_data[i].size  = packet->es_aud_i.packet_info[i].size;
                        frame_package.packet_data[i].data  = packet->es_aud.packet_data[i].data;
                    }
                }
                break;
            default:
                std::cout << "Fmt error!" << std::endl;
                return -1;
        }
        frame_package.stamp = packet->get_time_stamp();
        desc->this_class->send_pool_package(desc->pool_handle, frame_package);
        return 0;
    }
    static int connect_rtsp_reader(void *user, unsigned int ref)
    {
        if (ref > 0)
        {
            return 0;
        }
        struct ss_cmd_rtsp_out_desc *desc = (struct ss_cmd_rtsp_out_desc *)user;
        if (!desc || !desc->this_class)
        {
            std::cout << "Desc out or this class is NULL!" << std::endl;
            return -1;
        }
        if (!desc->output_ref)
        {
            size_t strStart = desc->url.find_first_of('<');
            size_t strEnd = desc->url.find('>');
            std::string strUser = "";
            std::string strPwd = "";
            std::string strUrl = desc->url;
            if (strStart != std::string::npos && strEnd != std::string::npos && strEnd > strStart)
            {
                strUser = desc->url.substr(strStart + 1, strEnd - strStart - 1);
                std::istringstream iss(strUser);
                std::getline(iss, strUser, ':');
                std::getline(iss, strPwd);
                strUrl = desc->url.substr(0, strStart);
            }
            desc->this_class->play(strUrl, strUser, strPwd);
        }
        desc->output_ref++;
        std::cout << "Ref=" << desc->output_ref << std::endl;
        return 0;
    }
    static int disconnect_rtsp_reader(void *user, unsigned int ref)
    {
        if (ref > 0)
        {
            return 0;
        }
        struct ss_cmd_rtsp_out_desc *desc = (struct ss_cmd_rtsp_out_desc *)user;
        if (!desc || !desc->this_class)
        {
            std::cout << "Desc out or this class is NULL!" << std::endl;
            return -1;
        }
        if (!desc->output_ref)
        {
            std::cout << "Ref=0 error, Rtsp client did not play." << std::endl;
            return -1;
        }
        desc->output_ref--;
        if (!desc->output_ref)
        {
            desc->this_class->stop(desc->url);
        }
        std::cout << "Ref=" << desc->output_ref << std::endl;
        return 0;
    }
    std::map<std::string, struct ss_cmd_rtsp_in_desc>  video_in_path_desc;
    std::map<std::string, struct ss_cmd_rtsp_in_desc>  audio_in_path_desc;
    std::map<std::string, struct ss_cmd_rtsp_out_desc> out_path_desc;
    int video_server_depth;
    int audio_server_depth;
};

static inline unsigned int str_2_format_enum(const std::string &str)
{
    if (str == "h264")
        return RTSP_ES_FMT_VIDEO_H264;
    if (str == "h265")
        return RTSP_ES_FMT_VIDEO_H265;
    if (str == "jpeg")
        return RTSP_ES_FMT_VIDEO_JPEG;
    if (str == "pcm")
        return RTSP_ES_FMT_AUDIO_PCM;
    if (str == "aac")
        return RTSP_ES_FMT_AUDIO_AAC;
    return RTSP_ES_FMT_VIDEO_H264;
}

static int rtsp_create(vector<string> &in_strs)
{
    const string &handle = in_strs[1];
    return ss_cmd_rtsp::create(handle);
}

static int rtsp_destroy(vector<string> &in_strs)
{
    const string &handle = in_strs[1];
    return ss_handle::destroy(handle);
}

static int rtsp_add_video_server_url(vector<string> &in_strs)
{
    struct rtsp_video_info info;
    const string &handle = in_strs[1];
    unsigned int format    = str_2_format_enum(in_strs[3]);
    unsigned int fps       = ss_cmd_atoi(in_strs[4].c_str());
    unsigned int width     = ss_cmd_atoi(in_strs[5].c_str());
    unsigned int height    = ss_cmd_atoi(in_strs[6].c_str());
    ss_cmd_rtsp  *cmd_rtsp = dynamic_cast <ss_cmd_rtsp *>(ss_cmd_rtsp::get(handle));
    if (!cmd_rtsp)
    {
        ss_print(PRINT_COLOR_RED, PRINT_MODE_HIGHTLIGHT, "Connector handle error!\n");
        return -1;
    }
    memset(&info, 0, sizeof(struct rtsp_video_info));
    info.format     = format;
    info.frame_rate = fps;
    info.width      = width;
    info.height     = height;
    return cmd_rtsp->config_video_server(in_strs[2], info);
}

static int rtsp_add_audio_server_url(vector<string> &in_strs)
{
    struct rtsp_audio_info info;
    const string &handle = in_strs[1];
    unsigned int  format    = str_2_format_enum(in_strs[3]);
    unsigned int  channels     = ss_cmd_atoi(in_strs[4].c_str());
    unsigned int  sample_rate  = ss_cmd_atoi(in_strs[5].c_str());
    unsigned char sample_width = ss_cmd_atoi(in_strs[6].c_str());
    ss_cmd_rtsp   *cmd_rtsp = dynamic_cast <ss_cmd_rtsp *>(ss_cmd_rtsp::get(handle));
    if (!cmd_rtsp)
    {
        ss_print(PRINT_COLOR_RED, PRINT_MODE_HIGHTLIGHT, "Connector handle error!\n");
        return -1;
    }
    memset(&info, 0, sizeof(struct rtsp_audio_info));
    info.format       = format;
    info.channels     = channels;
    info.sample_rate  = sample_rate;
    info.sample_width = sample_width;
    return cmd_rtsp->config_audio_server(in_strs[2], info);
}

static int rtsp_create_video_connector_writer(vector<string> &in_strs)
{
    const string &handle = in_strs[1];
    const string &handle_rtsp = in_strs[3];

    ss_connector *connector = dynamic_cast <ss_connector *>(ss_connector::get(handle));
    if (!connector)
    {
        ss_print(PRINT_COLOR_RED, PRINT_MODE_HIGHTLIGHT, "Connector handle error!\n");
        return -1;
    }
    ss_cmd_rtsp *cmd_rtsp = dynamic_cast <ss_cmd_rtsp *>(ss_cmd_rtsp::get(handle_rtsp));
    if (!cmd_rtsp)
    {
        ss_print(PRINT_COLOR_RED, PRINT_MODE_HIGHTLIGHT, "Connector handle error!\n");
        return -1;
    }
    int depth = ss_cmd_atoi(in_strs[5].c_str());
    if (!depth)
    {
        ss_print(PRINT_COLOR_RED, PRINT_MODE_HIGHTLIGHT, "depth error!\n");
        return -1;
    }
    return cmd_rtsp->bind_video_in_connector(in_strs[4], depth, connector, in_strs[2]);
}

static int rtsp_destroy_video_connector_writer(vector<string> &in_strs)
{
    const string &handle = in_strs[1];
    ss_cmd_rtsp *cmd_rtsp    = dynamic_cast <ss_cmd_rtsp *>(ss_cmd_rtsp::get(handle));
    if (!cmd_rtsp)
    {
        ss_print(PRINT_COLOR_RED, PRINT_MODE_HIGHTLIGHT, "Connector handle error!\n");
        return -1;
    }
    return cmd_rtsp->unbind_video_in_connector(in_strs[2]);
}

static int rtsp_create_audio_connector_writer(vector<string> &in_strs)
{
    const string &handle = in_strs[1];
    const string &handle_rtsp = in_strs[3];

    ss_connector *connector = dynamic_cast <ss_connector *>(ss_connector::get(handle));
    if (!connector)
    {
        ss_print(PRINT_COLOR_RED, PRINT_MODE_HIGHTLIGHT, "Connector handle error!\n");
        return -1;
    }
    ss_cmd_rtsp *cmd_rtsp = dynamic_cast <ss_cmd_rtsp *>(ss_cmd_rtsp::get(handle_rtsp));
    if (!cmd_rtsp)
    {
        ss_print(PRINT_COLOR_RED, PRINT_MODE_HIGHTLIGHT, "Connector handle error!\n");
        return -1;
    }
    int depth = ss_cmd_atoi(in_strs[5].c_str());
    if (!depth)
    {
        ss_print(PRINT_COLOR_RED, PRINT_MODE_HIGHTLIGHT, "depth error!\n");
        return -1;
    }
    return cmd_rtsp->bind_audio_in_connector(in_strs[4], depth, connector, in_strs[2]);
}

static int rtsp_destroy_audio_connector_writer(vector<string> &in_strs)
{
    const string &handle = in_strs[1];
    ss_cmd_rtsp *cmd_rtsp    = dynamic_cast <ss_cmd_rtsp *>(ss_cmd_rtsp::get(handle));
    if (!cmd_rtsp)
    {
        ss_print(PRINT_COLOR_RED, PRINT_MODE_HIGHTLIGHT, "Connector handle error!\n");
        return -1;
    }
    return cmd_rtsp->unbind_audio_in_connector(in_strs[2]);
}

static int rtsp_start_server(vector<string> &in_strs)
{
    const string &handle = in_strs[1];
    ss_cmd_rtsp *cmd_rtsp = dynamic_cast <ss_cmd_rtsp *>(ss_cmd_rtsp::get(handle));
    if (!cmd_rtsp)
    {
        ss_print(PRINT_COLOR_RED, PRINT_MODE_HIGHTLIGHT, "Connector handle error!\n");
        return -1;
    }
    return cmd_rtsp->start_rtsp_server();
}

static int rtsp_all_url(vector<string> &in_strs)
{
    const string &handle = in_strs[1];
    ss_cmd_rtsp *cmd_rtsp = dynamic_cast <ss_cmd_rtsp *>(ss_cmd_rtsp::get(handle));
    if (!cmd_rtsp)
    {
        ss_print(PRINT_COLOR_RED, PRINT_MODE_HIGHTLIGHT, "Connector handle error!\n");
        return -1;
    }
    std::vector<std::string> urls;
    cmd_rtsp->dump_rtsp_server_url(urls);
    for (auto &it : urls)
    {
        sslog << it << std::endl;
    }
    return 0;
}

static int rtsp_stop_server(vector<string> &in_strs)
{
    const string &handle = in_strs[1];
    ss_cmd_rtsp *cmd_rtsp = dynamic_cast <ss_cmd_rtsp *>(ss_cmd_rtsp::get(handle));
    if (!cmd_rtsp)
    {
        ss_print(PRINT_COLOR_RED, PRINT_MODE_HIGHTLIGHT, "Connector handle error!\n");
        return -1;
    }
    return cmd_rtsp->stop_rtsp_server();
}
static int rtsp_play(vector<string> &in_strs)
{
    const string &handle = in_strs[1];
    ss_cmd_rtsp *cmd_rtsp = dynamic_cast <ss_cmd_rtsp *>(ss_cmd_rtsp::get(handle));
    if (!cmd_rtsp)
    {
        ss_print(PRINT_COLOR_RED, PRINT_MODE_HIGHTLIGHT, "Connector handle error!\n");
        return -1;
    }
    cmd_rtsp->play_rtsp_client(in_strs[2]);
    return 0;
}
static int rtsp_stop(vector<string> &in_strs)
{
    const string &handle = in_strs[1];
    ss_cmd_rtsp *cmd_rtsp = dynamic_cast <ss_cmd_rtsp *>(ss_cmd_rtsp::get(handle));
    if (!cmd_rtsp)
    {
        ss_print(PRINT_COLOR_RED, PRINT_MODE_HIGHTLIGHT, "Connector handle error!\n");
        return -1;
    }
    cmd_rtsp->stop_rtsp_clent(in_strs[2]);
    return 0;
}

int rtsp_create_video_connector_reader(vector<string> &in_strs)
{
    const string &handle = in_strs[1];
    const string &handle_rtsp = in_strs[2];

    ss_connector *connector = dynamic_cast <ss_connector *>(ss_connector::get(handle));
    if (!connector)
    {
        ss_print(PRINT_COLOR_RED, PRINT_MODE_HIGHTLIGHT, "Connector handle error!\n");
        return -1;
    }
    ss_cmd_rtsp *cmd_rtsp = dynamic_cast <ss_cmd_rtsp *>(ss_cmd_rtsp::get(handle_rtsp));
    if (!cmd_rtsp)
    {
        ss_print(PRINT_COLOR_RED, PRINT_MODE_HIGHTLIGHT, "Connector handle error!\n");
        return -1;
    }
    return cmd_rtsp->bind_video_out_connector(in_strs[3], connector);
}

int rtsp_destroy_video_connector_reader(vector<string> &in_strs)
{
    const string &handle = in_strs[1];
    ss_cmd_rtsp *cmd_rtsp = dynamic_cast <ss_cmd_rtsp *>(ss_cmd_rtsp::get(handle));
    if (!cmd_rtsp)
    {
        ss_print(PRINT_COLOR_RED, PRINT_MODE_HIGHTLIGHT, "Connector handle error!\n");
        return -1;
    }
    return cmd_rtsp->unbind_video_out_connector(in_strs[2]);
}

int rtsp_create_audio_connector_reader(vector<string> &in_strs)
{
    const string &handle = in_strs[1];
    string handle_rtsp = in_strs[2];

    ss_connector *connector = dynamic_cast <ss_connector *>(ss_connector::get(handle));
    if (!connector)
    {
        ss_print(PRINT_COLOR_RED, PRINT_MODE_HIGHTLIGHT, "Connector handle error!\n");
        return -1;
    }
    ss_cmd_rtsp *cmd_rtsp = dynamic_cast <ss_cmd_rtsp *>(ss_cmd_rtsp::get(handle_rtsp));
    if (!cmd_rtsp)
    {
        ss_print(PRINT_COLOR_RED, PRINT_MODE_HIGHTLIGHT, "Connector handle error!\n");
        return -1;
    }
    return cmd_rtsp->bind_audio_out_connector(in_strs[3], connector);
}

int rtsp_destroy_audio_connector_reader(vector<string> &in_strs)
{
    const string &handle = in_strs[1];
    ss_cmd_rtsp *cmd_rtsp = dynamic_cast <ss_cmd_rtsp *>(ss_cmd_rtsp::get(handle));
    if (!cmd_rtsp)
    {
        ss_print(PRINT_COLOR_RED, PRINT_MODE_HIGHTLIGHT, "Connector handle error!\n");
        return -1;
    }
    return cmd_rtsp->unbind_audio_out_connector(in_strs[2]);
}

MOD_CMDS(rtsp) {
    ADD_CMD("rtsp_create", rtsp_create, 1);
    ADD_CMD_HELP("rtsp_create", "[handle]", "Create rtsp instance by handle id.");
    ADD_CMD("rtsp_destroy", rtsp_destroy, 1);
    ADD_CMD_HELP("rtsp_destroy", "[handle]", "Destroy rtsp instance by handle id.");
    ADD_CMD("rtsp_add_video_server_url", rtsp_add_video_server_url, 6);
    ADD_CMD_HELP("rtsp_add_video_server_url", "[handle] [url] [format] [fps] [width] [height]",
                 "%s\n%s\n%s\n%s\n%s\n%s\n%s",
                 "Set rtsp's videp path url and set video config.",
                 "[handle] Rtsp instance's handle.",
                 "[url] Rtsp's url string",
                 "[format] Video format about: 'h264', 'h265', 'jpeg', 'av1', 'vp9'",
                 "[fps] Video frame rate."
                 "[width] Video frame width",
                 "[height] Video frame height");
    ADD_CMD("rtsp_add_audio_server_url", rtsp_add_audio_server_url, 6);
    ADD_CMD_HELP("rtsp_add_audio_server_url", "[handle] [url] [format] [channel] [bit rate] [bit width]",
                 "%s\n%s\n%s\n%s\n%s\n%s\n%s",
                 "Set rtsp's audio path url and set audio config.",
                 "[handle] Rtsp instance's handle.",
                 "[url] Rtsp's url string",
                 "[format] Video format about: 'pcm', 'aac'",
                 "[channel] Aduio source's channel count.",
                 "[bit rate] Aduio source's sample rate.",
                 "[bit withc] Bit count per sample.");
    ADD_CMD("rtsp_create_video_connector_writer", rtsp_create_video_connector_writer, 5);
    ADD_CMD_HELP("rtsp_create_video_connector_writer",
                 "[connector handle] [refer id] [rtsp handle] [url] [pool_depth]",
                 "Create a connector's writer for rtsp video path.");
    ADD_CMD("rtsp_destroy_video_connector_writer", rtsp_destroy_video_connector_writer, 2);
    ADD_CMD_HELP("rtsp_destroy_video_connector_writer", "[rtsp handle] [url]",
                 "Destroy rtsp's video path connector writer.");
    ADD_CMD("rtsp_create_audio_connector_writer", rtsp_create_audio_connector_writer, 5);
    ADD_CMD_HELP("rtsp_create_audio_connector_writer",
                 "[connector handle] [refer id] [rtsp handle] [url] [pool_depth]",
                 "Create a connector's writer for rtsp audio path.");
    ADD_CMD("rtsp_destroy_audio_connector_writer", rtsp_destroy_audio_connector_writer, 2);
    ADD_CMD_HELP("rtsp_destroy_audio_connector_writer", "[rtsp handle] [url]",
                 "Destroy rtsp's audio path connector writer.");
    ADD_CMD("rtsp_start_server", rtsp_start_server, 1);
    ADD_CMD_HELP("rtsp_start_server", "[rtsp handle]", "Start rtsp server for all added url.");
    ADD_CMD("rtsp_stop_server", rtsp_stop_server, 1);
    ADD_CMD_HELP("rtsp_stop_server", "[rtsp handle]", "Stop rtsp server for all added url.");
    ADD_CMD("rtsp_all_url", rtsp_all_url, 1);
    ADD_CMD_HELP("rtsp_all_url", "[rtsp handle]", "Get all rtsp url.");
    ADD_CMD("rtsp_play", rtsp_play, 2);
    ADD_CMD_HELP("rtsp_play", "[rtsp handle] [url]", "Play rtsp client video by the specified url.");
    ADD_CMD("rtsp_stop", rtsp_stop, 2);
    ADD_CMD_HELP("rtsp_stop", "[rtsp handle] [url]", "Stop playing rtsp client video by the specified url.");
    ADD_CMD("rtsp_create_video_connector_reader", rtsp_create_video_connector_reader, 3);
    ADD_CMD_HELP("rtsp_create_video_connector_reader", "[connector handle] [rtsp handle] [url]",
                 "Create a video connector's reader for rtsp client out..");
    ADD_CMD("rtsp_destroy_video_connector_reader", rtsp_destroy_video_connector_reader, 2);
    ADD_CMD_HELP("rtsp_destroy_video_connector_reader", "[rtsp handle] [url]",
                 "Destroy a video connector's reader for rtsp client out..");
    ADD_CMD("rtsp_create_audio_connector_reader", rtsp_create_audio_connector_reader, 3);
    ADD_CMD_HELP("rtsp_create_audio_connector_reader", "[connector handle] [rtsp handle] [url]",
                 "Create an audio connector's reader for rtsp client out..");
    ADD_CMD("rtsp_destroy_audio_connector_reader", rtsp_destroy_audio_connector_reader, 2);
    ADD_CMD_HELP("rtsp_destroy_audio_connector_reader", "[rtsp handle] [url]",
                 "Destroy an audio connector's reader for rtsp client out..");
}
