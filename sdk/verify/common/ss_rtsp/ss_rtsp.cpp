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
#include <string.h>
#include <pthread.h>
#include <assert.h>
#include <time.h>

#include <set>
#include "ss_rtsp.h"

#include "BasicUsageEnvironment.hh"
#include "liveMedia.hh"
#include "Live555RTSPServer.hh"

#define RTSP_LISTEN_PORT        554

#define add_time(_ts, _sec, _nsec)     \
    do                                 \
    {                                  \
        _ts.tv_sec += _sec;            \
        _ts.tv_nsec += _nsec;          \
        if (_ts.tv_nsec > 1000000000)  \
        {                              \
            _ts.tv_nsec %= 1000000000; \
            _ts.tv_sec++;              \
        }                              \
    } while(0);

class ss_rtsp_event
{
    public:
        explicit ss_rtsp_event(struct ss_rtsp::frame_package_lock &lock)
            : event_mutex(lock.frame_mutex), event_cond(lock.frame_cond)
        {
            pthread_mutex_lock(&event_mutex);
        }
        virtual ~ss_rtsp_event()
        {
            pthread_mutex_unlock(&event_mutex);

        }
        void notify_frame()
        {
            pthread_cond_signal(&event_cond);
        }
        void wait_frame(unsigned int wait_time_ms = 0xFFFFFFFF)
        {
            if (wait_time_ms == 0xFFFFFFFF)
            {
                pthread_cond_wait(&event_cond, &event_mutex);
            }
            else
            {
                struct timespec out_time;
                clock_gettime(CLOCK_MONOTONIC, &out_time);
                add_time(out_time, (wait_time_ms / 1000), (wait_time_ms % 1000) * 1000000);
                pthread_cond_timedwait(&event_cond, &event_mutex, &out_time);
            }
        }
    private:
        pthread_mutex_t    &event_mutex;
        pthread_cond_t     &event_cond;
};

class rtsp_packet_pool;
class rtsp_object_pool_base
{
public:
    rtsp_object_pool_base();
    virtual ~rtsp_object_pool_base();
    void insert(rtsp_packet_pool *p);
    virtual rtsp_packet normal_make(const struct rtsp_frame_packet &packet) = 0;
    std::set<rtsp_packet_pool *> pool;
};

class rtsp_packet_pool
{
public:
    explicit rtsp_packet_pool(ss_rtsp &rtsp, rtsp_object_pool_base &base, const std::string &name);
    virtual ~rtsp_packet_pool();
    void insert_frame(rtsp_packet &obj);
    void stop_frame();
    bool is_frame_end();
    unsigned int packet_size();
    void get_one_slice(std::vector<struct rtsp_es_slice_data> &slice, struct timeval &time_stamp);
    void put_one_slice();
protected:
    ss_rtsp               &this_rtsp;
    rtsp_object_pool_base &obj_pool;
    std::string           stream_name;
private:
    std::list<rtsp_packet> packet_pool;
    unsigned int           frame_off;
    bool                   frame_stop;
    bool                   b_sync_time;
    struct timeval         time_diff;
};

template <typename T>
class rtsp_object_pool : public rtsp_object_pool_base
{
public:
    static rtsp_object_pool_base &get_instance(void **handle)
    {
        rtsp_object_pool<T> *obj = (rtsp_object_pool<T> *)*handle;
        if (!obj)
        {
            obj = new rtsp_object_pool<T>;
            assert(obj);
            *handle = (void *)obj;
        }
        return *obj;
    }
private:
    rtsp_packet normal_make(const struct rtsp_frame_packet &packet) final
    {
        return ss_rtsp_packet::make<ss_rtsp_normal_packet<T>>(packet);
    }
};

class video_packet_pool : public rtsp_packet_pool
{
public:
    explicit video_packet_pool(ss_rtsp &rtsp, rtsp_object_pool_base &base, const std::string &name)
        : rtsp_packet_pool(rtsp, base, name)
    {
    }
    ~video_packet_pool() final
    {
        this_rtsp.disconnect_video_stream(stream_name);
    }
};

class audio_packet_pool : public rtsp_packet_pool
{
public:
    explicit audio_packet_pool(ss_rtsp &rtsp, rtsp_object_pool_base &base, const std::string &name)
        : rtsp_packet_pool(rtsp, base, name)
    {
    }
    ~audio_packet_pool() final
    {
        this_rtsp.disconnect_audio_stream(stream_name);
    }
};

rtsp_object_pool_base::rtsp_object_pool_base()
{
    std::cout << "Construct Object pool " << this << std::endl;
}

rtsp_object_pool_base::~rtsp_object_pool_base()
{
    std::cout << "Destruct Object pool " << this << std::endl;
}

void rtsp_object_pool_base::insert(rtsp_packet_pool *p)
{
    pool.insert(p);
    std::cout << "Object pool size: " << pool.size() << std::endl;
}

rtsp_packet_pool::rtsp_packet_pool(ss_rtsp &rtsp, rtsp_object_pool_base &base, const std::string &name)
    : this_rtsp(rtsp), obj_pool(base), stream_name(name), frame_off(0), frame_stop(false), b_sync_time(false)
{
    std::cout << "Open packet pool : " << this << std::endl;
}

rtsp_packet_pool::~rtsp_packet_pool()
{
    ss_rtsp_event auto_lock(this_rtsp.frame_lock);
    auto iter = obj_pool.pool.find(this);
    assert(iter != obj_pool.pool.end());
    obj_pool.pool.erase(iter);
    std::cout << "Close packet pool : " << this << ", object pool size: " << obj_pool.pool.size() << std::endl;
}

void rtsp_packet_pool::insert_frame(rtsp_packet &obj)
{
    if (!frame_stop)
    {
        if (!b_sync_time)
        {
            struct timeval now;
            gettimeofday(&now, NULL);
            time_diff.tv_sec  = now.tv_sec  - obj->time_stamp.tv_sec;
            time_diff.tv_usec = now.tv_usec - obj->time_stamp.tv_usec;
            std::cout << "Time stamp time diff: " << "SEC: " << time_diff.tv_sec << ", USEC: " << time_diff.tv_usec << std::endl;
            b_sync_time = true;
        }
        this->packet_pool.push_back(obj);
    }
}

void rtsp_packet_pool::stop_frame()
{
    frame_stop = true;
}

bool rtsp_packet_pool::is_frame_end()
{
    return frame_stop;
}

unsigned int rtsp_packet_pool::packet_size()
{
    return this->packet_pool.size();
}

void rtsp_packet_pool::get_one_slice(std::vector<struct rtsp_es_slice_data> &slice, struct timeval &time_stamp)
{
    unsigned int i;
    if (this->frame_stop || !packet_pool.size())
    {
        return;
    }
    rtsp_packet &obj = packet_pool.front();
    if (obj->frame.size() <= this->frame_off)
    {
        std::cout << "ERR: Frame offset " << obj->frame.size() << " > offset(" << this->frame_off << ')' << std::endl;
        return;
    }
    slice.push_back(obj->frame[frame_off]);
    for (i = frame_off + 1; i < obj->frame.size() && !obj->frame[i].b_nalu_start; i++)
    {
        slice.push_back(obj->frame[i]);
    }
    frame_off          = i;
    time_stamp.tv_sec  = obj->time_stamp.tv_sec + time_diff.tv_sec;
    time_stamp.tv_usec = obj->time_stamp.tv_usec + time_diff.tv_usec;
    if (time_stamp.tv_usec >= 1000000)
    {
        time_stamp.tv_usec %= 1000000;
        time_stamp.tv_sec++;
    }
    else if (time_stamp.tv_usec < 0)
    {
        time_stamp.tv_usec = 1000000 + time_stamp.tv_usec;
        time_stamp.tv_sec--;
    }
    return;
}

void rtsp_packet_pool::put_one_slice()
{
    if (!packet_pool.size() || frame_stop)
    {
        return;
    }
    rtsp_packet &obj = packet_pool.front();
    if (obj->frame.size() == frame_off)
    {
        packet_pool.pop_front();
        frame_off = 0;
    }
}

ss_rtsp::ss_rtsp() : live555_srv(NULL)
{
    pthread_mutex_init(&frame_lock.frame_mutex, NULL);
    pthread_condattr_init(&frame_lock.frame_cond_attr);
    pthread_condattr_setclock(&frame_lock.frame_cond_attr, CLOCK_MONOTONIC);
    pthread_cond_init(&frame_lock.frame_cond, &frame_lock.frame_cond_attr);
}

ss_rtsp::~ss_rtsp()
{
    pthread_condattr_destroy(&frame_lock.frame_cond_attr);
    pthread_cond_destroy(&frame_lock.frame_cond);
    pthread_mutex_destroy(&frame_lock.frame_mutex);
}

int ss_rtsp::add_audio_server_url(const std::string &url, const struct rtsp_audio_info &info)
{
    ss_rtsp_event auto_lock(frame_lock);
    auto iter = rtsp_input.find(url);
    if (iter == rtsp_input.end())
    {
        memset(&rtsp_input[url], 0, sizeof(struct rtsp_input_info));
    }
    else if (iter->second.audio_input.info.format != RTSP_ES_FMT_AUDIO_NONE)
    {
        std::cout << "URL " << url << " Audio url had been added before." << std::endl;
        return -1;
    }
    rtsp_input[url].audio_input.info = info;
    return 0;
}

int ss_rtsp::del_audio_server_url(const std::string &url)
{
    ss_rtsp_event auto_lock(frame_lock);
    auto iter = rtsp_input.find(url);
    if (iter == rtsp_input.end())
    {
        std::cout << "Not find " << url << std::endl;
        return -1;
    }
    if (iter->second.media_session)
    {
        std::cout << "AUDIO URL: " << url << ", It should stop server first." << std::endl;
        return -1;
    }
    if (iter->second.audio_input.info.format == RTSP_ES_FMT_AUDIO_NONE)
    {
        std::cout << "AUDIO URL " << url << " Did not add before." << std::endl;
        return -1;
    }
    iter->second.audio_input.info.format = RTSP_ES_FMT_AUDIO_NONE;
    return 0;
}

int ss_rtsp::add_video_server_url(const std::string &url, const struct rtsp_video_info &info)
{
    ss_rtsp_event auto_lock(frame_lock);

    auto iter = rtsp_input.find(url);
    if (iter == rtsp_input.end())
    {
        memset(&rtsp_input[url], 0, sizeof(struct rtsp_input_info));
    }
    else if (iter->second.video_input.info.format != RTSP_ES_FMT_VIDEO_NONE)
    {
        std::cout << "URL " << url << " Video url had been added before." << std::endl;
        return -1;
    }
    rtsp_input[url].video_input.info = info;
    return 0;
}

int ss_rtsp::del_video_server_url(const std::string &url)
{
    ss_rtsp_event auto_lock(frame_lock);
    auto iter = rtsp_input.find(url);
    if (iter == rtsp_input.end())
    {
        std::cout << "Not find " << url << std::endl;
        return -1;
    }
    if (iter->second.media_session)
    {
        std::cout << "VIDEO URL: " << url << ", It should stop server first." << std::endl;
        return -1;
    }
    if (iter->second.video_input.info.format == RTSP_ES_FMT_VIDEO_NONE)
    {
        std::cout << "VIDEO URL " << url << " Did not add before." << std::endl;
        return -1;
    }
    iter->second.video_input.info.format = RTSP_ES_FMT_VIDEO_NONE;
    return 0;
}

void *ss_rtsp::get_video_pool_handle(const std::string &url)
{
    ss_rtsp_event auto_lock(frame_lock);
    auto iter = rtsp_input.find(url);
    if (iter == rtsp_input.end())
    {
        std::cout << "Not found url " << url << std::endl;
        return NULL;
    }
    return &iter->second.video_input.frame_package;
}
void *ss_rtsp::get_audio_pool_handle(const std::string &url)
{
    ss_rtsp_event auto_lock(frame_lock);
    auto iter = rtsp_input.find(url);
    if (iter == rtsp_input.end())
    {
        std::cout << "Not found url " << url << std::endl;
        return NULL;
    }
    return &iter->second.audio_input.frame_package;
}
const char *ss_rtsp::get_url_prefix(void *handle)
{
    struct frame_package_head *frame_package = (frame_package_head *)handle;

    ss_rtsp_event auto_lock(frame_lock);
    if (!frame_package || !frame_package->frame_url_prefix)
    {
        std::cout << "handle or url prefix is NULL!!!" << std::endl;
        return "";
    }
    return frame_package->frame_url_prefix;
}
int ss_rtsp::all_url(std::vector<std::string> &url_array)
{
    ss_rtsp_event auto_lock(frame_lock);
    for (auto &it : rtsp_input)
    {
        if (it.second.url_prefix)
        {
            url_array.push_back(*it.second.url_prefix);
        }
    }
    return 0;
}
void ss_rtsp::config_pool(void *handle, const struct rtsp_pool_config &config)
{
    struct frame_package_head *frame_package = (frame_package_head *)handle;

    ss_rtsp_event auto_lock(frame_lock);
    if (!frame_package)
    {
        std::cout << "handle NULL!!!" << std::endl;
        return;
    }
    frame_package->frame_pool_config = config;
}

int ss_rtsp::check_pool_package(void *handle)
{
    struct frame_package_head *frame_package = (frame_package_head *)handle;
    rtsp_object_pool_base *obj_pool = nullptr;

    ss_rtsp_event lock(frame_lock);
    obj_pool = (rtsp_object_pool_base *)frame_package->frame_obj_pool;
    if (!obj_pool)
    {
        std::cout << "obj pool is empty" << std::endl;
        return -1;
    }
    for (auto &it : obj_pool->pool)
    {
        if (it->packet_size() >= frame_package->frame_pool_config.depth)
        {
            return -1;
        }
    }
    return 0;
}

void ss_rtsp::send_pool_package(void *handle, const struct rtsp_frame_packet &frame)
{
    struct frame_package_head *frame_package = (frame_package_head *)handle;

    if (!frame_package)
    {
        std::cout << "handle NULL!!!" << std::endl;
        return;
    }
    ss_rtsp_event event(frame_lock);
    rtsp_object_pool_base *obj_pool = (rtsp_object_pool_base *)frame_package->frame_obj_pool;
    if (!obj_pool)
    {
        std::cout << "obj pool is empty" << std::endl;
        return;
    }
    if (!obj_pool->pool.size())
    {
        std::cout << "ss_rtsp: ref=0" << std::endl;
        return;
    }
    rtsp_packet packet = obj_pool->normal_make(frame);
    for (auto &it : obj_pool->pool)
    {
        it->insert_frame(packet);
    }
    event.notify_frame();
    return;
}

void ss_rtsp::send_pool_package(void *handle, rtsp_packet &obj)
{
    struct frame_package_head *frame_package = (frame_package_head *)handle;

    if (!frame_package)
    {
        std::cout << "handle NULL!!!" << std::endl;
        return;
    }
    ss_rtsp_event event(frame_lock);
    rtsp_object_pool_base *obj_pool = (rtsp_object_pool_base *)frame_package->frame_obj_pool;
    if (!obj_pool)
    {
        std::cout << "obj pool is empty" << std::endl;
        return;
    }
    if (!obj_pool->pool.size())
    {
        std::cout << "ss_rtsp: ref=0" << std::endl;
        return;
    }
    for (auto &it : obj_pool->pool)
    {
        it->insert_frame(obj);
    }
    event.notify_frame();
    return;
}

int ss_rtsp::start_server(unsigned int maxBufDiv)
{

    if (!rtsp_input.size())
    {
        std::cout << "NO URL." << std::endl;
        return -1;
    }
    if (live555_srv)
    {
        std::cout << "Server start twice." << std::endl;
        return -1;
    }
    live555_srv = new Live555RTSPServer();
    assert(live555_srv);
#if 0
    live555_srv->addUserRecord("admin", "888888");
#endif
    int listenPort = RTSP_LISTEN_PORT;
    int ret = ((Live555RTSPServer *)live555_srv)->SetRTSPServerPort(listenPort);
    while(ret < 0)
    {
        listenPort++;
        if (listenPort > 65535)
        {
            std::cout << "Failed to create RTSP server: " << ((Live555RTSPServer *)live555_srv)->getResultMsg() << std::endl;
            delete (Live555RTSPServer *)live555_srv;
            live555_srv = NULL;
            return -1;
        }
        ret = ((Live555RTSPServer *)live555_srv)->SetRTSPServerPort(listenPort);
    }
    for (auto iter = rtsp_input.begin(); iter != rtsp_input.end(); ++iter)
    {
        if (iter->second.video_input.info.format == RTSP_ES_FMT_VIDEO_NONE
            && iter->second.audio_input.info.format == RTSP_ES_FMT_AUDIO_NONE)
        {
            continue;
        }

        unsigned int maxBufSize = iter->second.video_input.info.width * iter->second.video_input.info.height * 10 / maxBufDiv;
        std::cout << "video width: " << iter->second.video_input.info.width
                  << ", height: " << iter->second.video_input.info.height << ", Max buf size: 0x" << std::hex << maxBufSize << std::dec << std::endl;
        if (!maxBufSize)
        {
            std::cout << "Warning: buffer is 0, using default 1024*1024" << std::endl;
            maxBufSize = 1024 * 1024;
        }
        ((Live555RTSPServer *)live555_srv)->createServerMediaSession((ServerMediaSession *&)iter->second.media_session, iter->first.c_str());
        if (!iter->second.media_session)
        {
            std::cout << "SS_RTSP ERROR: Create media_session fail!!!" << std::endl;
            continue;
        }

        iter->second.url_prefix = new (std::nothrow)std::string(((Live555RTSPServer *)live555_srv)->rtspURLPrefix() + iter->first);
        assert(iter->second.url_prefix);
        std::cout << "=================URL===================" << std::endl;
        std::cout << *iter->second.url_prefix << std::endl;
        std::cout << "=================URL===================" << std::endl;

        if (iter->second.video_input.info.format == RTSP_ES_FMT_VIDEO_H264)
        {
            iter->second.video_input.session =
                WW_H264VideoFileServerMediaSubsession::createNew(*(((Live555RTSPServer *)live555_srv)->GetUsageEnvironmentObj()), iter->first.c_str(),
                                                                 open_video_stream, read_stream, close_stream, iter->second.video_input.info.frame_rate,
                                                                 this, 0, True, False, True, maxBufSize);
            ((Live555RTSPServer *)live555_srv)->addSubsession((ServerMediaSession *)iter->second.media_session, (ServerMediaSubsession *)iter->second.video_input.session);
            iter->second.video_input.frame_package.frame_url_prefix = iter->second.url_prefix->c_str();
            std::cout << "Create Rtsp H264 Session FPS: " << iter->second.video_input.info.frame_rate << std::endl;
        }
        else if (iter->second.video_input.info.format == RTSP_ES_FMT_VIDEO_H265)
        {
            iter->second.video_input.session=
                WW_H265VideoFileServerMediaSubsession::createNew(*(((Live555RTSPServer *)live555_srv)->GetUsageEnvironmentObj()), iter->first.c_str(),
                                                                 open_video_stream, read_stream, close_stream, iter->second.video_input.info.frame_rate,
                                                                 this, 0, True, False, True, maxBufSize);
            ((Live555RTSPServer *)live555_srv)->addSubsession((ServerMediaSession *)iter->second.media_session, (ServerMediaSubsession *)iter->second.video_input.session);
            iter->second.video_input.frame_package.frame_url_prefix = iter->second.url_prefix->c_str();
            std::cout << "Create Rtsp H265 Session, FPS: " << iter->second.video_input.info.frame_rate << std::endl;
        }
        else if (iter->second.video_input.info.format == RTSP_ES_FMT_VIDEO_JPEG)
        {
            iter->second.video_input.session =
                WW_JPEGVideoFileServerMediaSubsession::createNew(*(((Live555RTSPServer *)live555_srv)->GetUsageEnvironmentObj()), iter->first.c_str(),
                                                                 open_video_stream, read_stream, close_stream, iter->second.video_input.info.frame_rate,
                                                                 this, 0, True, False, maxBufSize);
            ((Live555RTSPServer *)live555_srv)->addSubsession((ServerMediaSession *)iter->second.media_session, (ServerMediaSubsession *)iter->second.video_input.session);
            iter->second.video_input.frame_package.frame_url_prefix = iter->second.url_prefix->c_str();
            std::cout << "Create Rtsp JPEG Session FPS: " << iter->second.video_input.info.frame_rate << std::endl;
        }
        else if (iter->second.video_input.info.format == RTSP_ES_FMT_VIDEO_AV1)
        {
            iter->second.video_input.session =
                WW_AV1VideoFileServerMediaSubsession::createNew(*(((Live555RTSPServer *)live555_srv)->GetUsageEnvironmentObj()), iter->first.c_str(),
                                                                 open_video_stream, read_stream, close_stream, iter->second.video_input.info.frame_rate,
                                                                 this, 0, True, False, maxBufSize);
            ((Live555RTSPServer *)live555_srv)->addSubsession((ServerMediaSession *)iter->second.media_session, (ServerMediaSubsession *)iter->second.video_input.session);
            iter->second.video_input.frame_package.frame_url_prefix = iter->second.url_prefix->c_str();
            std::cout << "Create Rtsp AV1 Session FPS: " << iter->second.video_input.info.frame_rate << std::endl;
        }
        if (iter->second.audio_input.info.format == RTSP_ES_FMT_AUDIO_PCM)
        {
            iter->second.audio_input.session =
                WW_WAVAudioFileServerMediaSubsession::createNew(*(((Live555RTSPServer *)live555_srv)->GetUsageEnvironmentObj()), iter->first.c_str(), WWA_PCM,
                                                                iter->second.audio_input.info.sample_rate, iter->second.audio_input.info.channels,
                                                                iter->second.audio_input.info.sample_width, open_audio_stream, read_stream, close_stream, this, 0, True, False, 256 * 1024);
            ((Live555RTSPServer *)live555_srv)->addSubsession((ServerMediaSession *)iter->second.media_session, (ServerMediaSubsession *)iter->second.audio_input.session);
            iter->second.audio_input.frame_package.frame_url_prefix = iter->second.url_prefix->c_str();
            printf("Create Rtsp Audio Session\n");
        }
        if (iter->second.audio_input.info.format == RTSP_ES_FMT_AUDIO_PCMU)
        {
            iter->second.audio_input.session =
                WW_WAVAudioFileServerMediaSubsession::createNew(*(((Live555RTSPServer *)live555_srv)->GetUsageEnvironmentObj()), iter->first.c_str(), WWA_PCMU,
                                                                iter->second.audio_input.info.sample_rate, iter->second.audio_input.info.channels,
                                                                iter->second.audio_input.info.sample_width, open_audio_stream, read_stream, close_stream, this, 0, True, False, 256 * 1024);
            ((Live555RTSPServer *)live555_srv)->addSubsession((ServerMediaSession *)iter->second.media_session, (ServerMediaSubsession *)iter->second.audio_input.session);
            iter->second.audio_input.frame_package.frame_url_prefix = iter->second.url_prefix->c_str();
            printf("Create Rtsp Audio Session\n");
        }
        if (iter->second.audio_input.info.format == RTSP_ES_FMT_AUDIO_PCMA)
        {
            iter->second.audio_input.session =
                WW_WAVAudioFileServerMediaSubsession::createNew(*(((Live555RTSPServer *)live555_srv)->GetUsageEnvironmentObj()), iter->first.c_str(), WWA_PCMA,
                                                                iter->second.audio_input.info.sample_rate, iter->second.audio_input.info.channels,
                                                                iter->second.audio_input.info.sample_width, open_audio_stream, read_stream, close_stream, this, 0, True, False, 256 * 1024);
            ((Live555RTSPServer *)live555_srv)->addSubsession((ServerMediaSession *)iter->second.media_session, (ServerMediaSubsession *)iter->second.audio_input.session);
            iter->second.audio_input.frame_package.frame_url_prefix = iter->second.url_prefix->c_str();
            printf("Create Rtsp Audio Session\n");
        }
        if (iter->second.audio_input.info.format == RTSP_ES_FMT_AUDIO_G726)
        {
            iter->second.audio_input.session =
                WW_WAVAudioFileServerMediaSubsession::createNew(*(((Live555RTSPServer *)live555_srv)->GetUsageEnvironmentObj()), iter->first.c_str(), WWA_G726,
                                                                iter->second.audio_input.info.sample_rate, iter->second.audio_input.info.channels,
                                                                iter->second.audio_input.info.sample_width, open_audio_stream, read_stream, close_stream, this, 0, True, False, 256 * 1024);
            ((Live555RTSPServer *)live555_srv)->addSubsession((ServerMediaSession *)iter->second.media_session, (ServerMediaSubsession *)iter->second.audio_input.session);
            iter->second.audio_input.frame_package.frame_url_prefix = iter->second.url_prefix->c_str();
            printf("Create Rtsp Audio Session\n");
        }
        ((Live555RTSPServer *)live555_srv)->addServerMediaSession((ServerMediaSession *)iter->second.media_session);
    }
    ((Live555RTSPServer *)live555_srv)->Start(99);
    return 0;
}

int ss_rtsp::stop_server()
{
    if (!live555_srv)
    {
        std::cout << "SRV is NULL" << std::endl;
        return -1;
    }
    end_frame_object_pool();
    ((Live555RTSPServer *)live555_srv)->Join();
    for (auto iter = rtsp_input.begin(); iter != rtsp_input.end(); ++iter)
    {
        if (!iter->second.media_session)
        {
            continue;
        }
        ((Live555RTSPServer *)live555_srv)->deleteServerMediaSession((ServerMediaSession *)iter->second.media_session);
        iter->second.media_session = NULL;
        iter->second.video_input.session = NULL;
        iter->second.audio_input.session = NULL;
        delete iter->second.url_prefix;
        iter->second.url_prefix = NULL;
    }
    delete (Live555RTSPServer *)live555_srv;
    live555_srv = NULL;
    clear_frame_object_pool();

    return 0;
}

void ss_rtsp::end_frame_object_pool(void)
{
    rtsp_object_pool_base *obj_pool = nullptr;

    ss_rtsp_event event(frame_lock);
    for(auto iter = rtsp_input.begin(); iter != rtsp_input.end(); iter++)
    {
        if (iter->second.video_input.info.format != RTSP_ES_FMT_VIDEO_NONE
            && iter->second.video_input.frame_package.frame_obj_pool)
        {
            obj_pool = (rtsp_object_pool_base *)iter->second.video_input.frame_package.frame_obj_pool;
            for (auto &it : obj_pool->pool)
            {
                it->stop_frame();
            }
            event.notify_frame();
        }
        if (iter->second.audio_input.info.format != RTSP_ES_FMT_AUDIO_NONE
            && iter->second.audio_input.frame_package.frame_obj_pool)
        {
            obj_pool = (rtsp_object_pool_base *)iter->second.audio_input.frame_package.frame_obj_pool;
            for (auto &it : obj_pool->pool)
            {
                it->stop_frame();
            }
            event.notify_frame();
        }
        std::cout << "SRV: " << iter->first << " Exit!" << std::endl;
    }
}

static inline bool check_wait_frame(rtsp_object_pool_base *obj_pool)
{
    if (!obj_pool)
    {
        return true;
    }
    for (auto &it : obj_pool->pool)
    {
        if (it->packet_size() || it->is_frame_end())
        {
            return false;
        }
    }
    return true;
}

bool ss_rtsp::check_wait_frame_object_pool(void)
{
    rtsp_object_pool_base *obj_pool = nullptr;

    for (auto iter = rtsp_input.begin(); iter != rtsp_input.end(); iter++)
    {
        if (iter->second.video_input.info.format != RTSP_ES_FMT_VIDEO_NONE)
        {
            obj_pool = (rtsp_object_pool_base *)iter->second.video_input.frame_package.frame_obj_pool;
            if (!check_wait_frame(obj_pool))
            {
                return false;
            }
        }
        if (iter->second.audio_input.info.format != RTSP_ES_FMT_AUDIO_NONE)
        {
            obj_pool = (rtsp_object_pool_base *)iter->second.audio_input.frame_package.frame_obj_pool;
            if (!check_wait_frame(obj_pool))
            {
                return false;
            }
        }
    }
    return true;
}

static inline void clear_object_pool(struct ss_rtsp::frame_package_head &package_head)
{
    if (package_head.frame_obj_pool)
    {
        rtsp_object_pool_base *obj_pool = (rtsp_object_pool_base *)package_head.frame_obj_pool;
        // should do copy here with using 'auto it'.
        for (auto &it : obj_pool->pool)
        {
            delete it;
        }
        delete obj_pool;
        package_head.frame_obj_pool = NULL;
    }
}

void ss_rtsp::clear_frame_object_pool(void)
{
    ss_rtsp_event auto_lock(frame_lock);

    for (auto iter = rtsp_input.begin(); iter != rtsp_input.end(); ++iter)
    {
        if (iter->second.video_input.info.format != RTSP_ES_FMT_VIDEO_NONE)
        {
            clear_object_pool(iter->second.video_input.frame_package);
        }
        if (iter->second.audio_input.info.format != RTSP_ES_FMT_AUDIO_NONE)
        {
            clear_object_pool(iter->second.audio_input.frame_package);
        }
    }
}

static rtsp_object_pool_base &get_video_obj_pool(int format, void **ins)
{
    switch (format)
    {
    case RTSP_ES_FMT_VIDEO_H264:
        return rtsp_object_pool<rtsp_h264_data_parser>::get_instance(ins);
    case RTSP_ES_FMT_VIDEO_H265:
        return rtsp_object_pool<rtsp_h265_data_parser>::get_instance(ins);
    default:
        return rtsp_object_pool<rtsp_data_parser>::get_instance(ins);
    }
}

void *ss_rtsp::open_video_stream(char const * stream_name, void * arg)
{
    ss_rtsp *this_class = (ss_rtsp *)arg;

    assert(this_class);
    auto iter = this_class->rtsp_input.find(stream_name);
    if (iter == this_class->rtsp_input.end())
    {
        std::cout << "Not found URL " << stream_name << std::endl;
        return NULL;
    }
    struct frame_package_head &frame_package = iter->second.video_input.frame_package;
    rtsp_object_pool_base &obj_pool = get_video_obj_pool(iter->second.video_input.info.format, &frame_package.frame_obj_pool);
    video_packet_pool *pool = new video_packet_pool(*this_class, obj_pool, stream_name);
    assert(pool);
    obj_pool.insert(pool);
    this_class->connect_video_stream(stream_name);
    printf("open video \"%s\" success, %p\n", stream_name, pool);
    return (void *)pool;
}

void *ss_rtsp::open_audio_stream(char const * stream_name, void * arg)
{
    ss_rtsp *this_class = (ss_rtsp *)arg;

    assert(this_class);
    auto iter = this_class->rtsp_input.find(stream_name);
    if (iter == this_class->rtsp_input.end())
    {
        std::cout << "Not found URL " << stream_name << std::endl;
        return NULL;
    }
    struct frame_package_head &frame_package = iter->second.audio_input.frame_package;
    rtsp_object_pool_base &obj_pool = rtsp_object_pool<rtsp_data_parser>::get_instance(&frame_package.frame_obj_pool);
    audio_packet_pool *pool = new audio_packet_pool(*this_class, obj_pool, stream_name);
    assert(pool);
    obj_pool.insert(pool);
    this_class->connect_audio_stream(stream_name);
    printf("open audio \"%s\" success, %p\n", stream_name, pool);
    return (void *)pool;
}

int ss_rtsp::read_stream(void *handle, unsigned char *out_buf, int len, struct timeval *time_stamp, void *arg)
{
    std::vector<struct rtsp_es_slice_data> slice;
    rtsp_packet_pool *pool  = (rtsp_packet_pool *)handle;
    ss_rtsp *this_class     = (ss_rtsp *)arg;
    unsigned int copy_shift = 0;

    assert(pool);
    assert(this_class);
    assert(time_stamp);
    ss_rtsp_event event(this_class->frame_lock);
    if (this_class->check_wait_frame_object_pool())
    {
        event.wait_frame();
    }
    pool->get_one_slice(slice, *time_stamp);
    for (auto &it : slice)
    {
        if (copy_shift + it.size > (unsigned int)len)
        {
            printf("Data max! copy size %d total size %d\n", copy_shift + it.size, len);
            copy_shift = 0;
            break;
        }
        memcpy(out_buf + copy_shift, it.data, it.size);
        copy_shift += it.size;
    }
    pool->put_one_slice();
    return copy_shift;
}

int ss_rtsp::close_stream(void *handle, void *arg)
{
    rtsp_packet_pool *pool  = (rtsp_packet_pool *)handle;

    assert(pool);
    delete pool;
    return 0;
}
