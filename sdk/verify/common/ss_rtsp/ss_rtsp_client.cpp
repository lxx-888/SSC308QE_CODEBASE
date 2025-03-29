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
#include <assert.h>
#include <pthread.h>
#include <sys/types.h>
#include "ss_rtsp.h"

#include "BasicUsageEnvironment.hh"
#include "liveMedia.hh"
#include "Live555RTSPServer.hh"

// Define a class to hold per-stream state that we maintain throughout each stream's lifetime:

#define RTSP_CLIENT_VERBOSITY_LEVEL 1 // by default, print verbose output from each "RTSPClient"
// By default, we request that the server stream its data using RTP/UDP.
// If, instead, you want to request that the server stream via RTP-over-TCP, change the following to True:
#define REQUEST_STREAMING_OVER_TCP False

static void continueAfterDESCRIBE(RTSPClient* rtspClient, int resultCode, char* resultString);
static void continueAfterSETUP(RTSPClient* rtspClient, int resultCode, char* resultString);
static void continueAfterPLAY(RTSPClient* rtspClient, int resultCode, char* resultString);
static void streamTimerHandler(void* clientData);
static RTSPClient* openURL(UsageEnvironment& env, char const* progName, char const* rtspURL, Authenticator* pAuth = NULL);
static void shutdownStream(RTSPClient* rtspClient, int exitCode = 1);
static UsageEnvironment& operator<<(UsageEnvironment& env, const RTSPClient& rtspClient);
static UsageEnvironment& operator<<(UsageEnvironment& env, const MediaSubsession& subsession);

// SigmaStar coding here


class ss_rtsp_client_env;
class TopUsageEnvironment : public BasicUsageEnvironment{
    public:
        static TopUsageEnvironment* createNew(ss_rtsp_client_env &rtspRef, TaskScheduler& taskScheduler);
    protected:
        TopUsageEnvironment(ss_rtsp_client_env &rtspRef, TaskScheduler& taskScheduler)
            : BasicUsageEnvironment(taskScheduler), rtspReferance(rtspRef) {
        };
        virtual ~TopUsageEnvironment(){};
    public:
        ss_rtsp_client_env &rtspReferance;
};

class ss_rtsp_client_env
{
    public:
        explicit ss_rtsp_client_env(ss_rtsp_client &client, const std::string &url, const std::string &userName, const std::string &pwd,
                unsigned int recv_buffer_size) : client_obj(client), recv_buffer_size(recv_buffer_size)
        {
            Authenticator auth;
            scheduler = BasicTaskScheduler::createNew();
            env = TopUsageEnvironment::createNew(*this, *scheduler);
            assert(scheduler);
            assert(env);
            client_url = url;
            event_loop = 0;
            if (!userName.empty())
            {
                auth.setUsernameAndPassword(userName.c_str(), pwd.c_str());
                rtsp_client = openURL(*env, "ss_rtsp_client", client_url.c_str(), &auth);
            }
            else
            {
                rtsp_client = openURL(*env, "ss_rtsp_client", client_url.c_str());
            }
            if (rtsp_client)
            {
                pthread_create(&tid, NULL, thread_main, this);
            }
        }
        virtual ~ss_rtsp_client_env()
        {
            void *ret_val = NULL;
            if (rtsp_client)
            {
                event_loop = 1;
                pthread_join(tid, &ret_val);
            }

            shutdownStream(rtsp_client);
            rtsp_client->envir() << *rtsp_client<< "Closing the stream.\n";
            Medium::close(rtsp_client);

            if (env->reclaim())
            {
                delete scheduler;
                scheduler = NULL;
                env = NULL;
            }
        }
        ss_rtsp_client &get_obj()
        {
            return client_obj;
        }
        std::string &get_url()
        {
            return client_url;
        }
        unsigned int get_buffer_size()
        {
            return recv_buffer_size;
        }
    private:
        static void *thread_main(void *arg)
        {
            ss_rtsp_client_env *this_class = (ss_rtsp_client_env *)arg;
            assert(this_class);

            this_class->env->taskScheduler().doEventLoop(&this_class->event_loop);
            return NULL;
        }
        BasicTaskScheduler *scheduler;
        TopUsageEnvironment *env;
        pthread_t tid;
        ss_rtsp_client &client_obj;
        std::string client_url;
        unsigned int recv_buffer_size;
        char event_loop;
        RTSPClient *rtsp_client;
};

class FrameSender
{
public:
    explicit FrameSender(ss_rtsp_client &client, const std::string &url, u_int8_t *buf, u_int32_t size)
        : rtspClient(client), url(url), bufBase(buf), bufBaseSize(size), frameId(0)
    {
    }
    virtual ~FrameSender()
    {
    }
    virtual u_int32_t inputBuffer(u_int32_t size, const struct timeval &timeStamp) = 0;
protected:
    ss_rtsp_client &rtspClient;
    std::string     url;
    u_int8_t *      bufBase;
    u_int32_t       bufBaseSize;
    unsigned int    frameId;
};

class AudioAacFrameSender : public FrameSender
{
public:
    explicit AudioAacFrameSender(ss_rtsp_client &client, const std::string &url, u_int8_t *buf,
                                    u_int32_t size, unsigned int sampleRate, unsigned int channels)
        : FrameSender(client, url, buf, size), sampleRate(sampleRate), channels(channels)
    {
    }
    ~AudioAacFrameSender() final
    {
    }
private:
    u_int32_t inputBuffer(u_int32_t frameSize, const struct timeval &timeStamp) final
    {
        struct rtsp_audio_output audio_output;
        audio_output.info.sample_rate                   = sampleRate;
        audio_output.info.channels                      = channels;
        audio_output.info.format                        = RTSP_ES_FMT_AUDIO_AAC;
        audio_output.info.sample_width                  = 16;
        audio_output.frame_package.packet_count         = 1;
        audio_output.frame_package.packet_data[0].b_end = true;
        audio_output.frame_package.packet_data[0].data  = (char *)this->bufBase;
        audio_output.frame_package.packet_data[0].size  = frameSize;
        audio_output.frame_package.stamp                = timeStamp;
        rtspClient.recv_audio_package(this->url, audio_output, this->frameId++);
        return 0;
    }
    unsigned int sampleRate;
    unsigned int channels;
};

class AudioPcmL16FrameSender : public FrameSender
{
public:
    explicit AudioPcmL16FrameSender(ss_rtsp_client &client, const std::string &url, u_int8_t *buf,
                                    u_int32_t size, unsigned int sampleRate, unsigned int channels)
        : FrameSender(client, url, buf, size), sampleRate(sampleRate), channels(channels)
    {
        std::cout << "RTSP-CLIENT: " << "PCM L16, SampleRate: " << sampleRate << ", Channels: " << channels << std::endl;
    }
    ~AudioPcmL16FrameSender() final
    {
    }
private:
    u_int32_t inputBuffer(u_int32_t frameSize, const struct timeval &timeStamp) final
    {
        struct rtsp_audio_output audio_output;
        pcmS16beToS16le(bufBase, frameSize);
        audio_output.info.sample_rate                   = sampleRate;
        audio_output.info.channels                      = channels;
        audio_output.info.format                        = RTSP_ES_FMT_AUDIO_PCM;
        audio_output.info.sample_width                  = 16;
        audio_output.frame_package.packet_count         = 1;
        audio_output.frame_package.packet_data[0].b_end = true;
        audio_output.frame_package.packet_data[0].data  = (char *)this->bufBase;
        audio_output.frame_package.packet_data[0].size  = frameSize;
        audio_output.frame_package.stamp                = timeStamp;
        //std::cout << "PCM Send buffer size: " << audio_output.frame_package.packet_data[0].size << std::endl;
        rtspClient.recv_audio_package(this->url, audio_output, this->frameId++);
        return 0;
    }
    void pcmS16beToS16le(u_int8_t *buf, u_int32_t size)
    {
        u_int32_t i = 0;

        for (i = 0; i < size; i += 2)
        {
            buf[i] ^= buf[i + 1];
            buf[i + 1] ^= buf[i];
            buf[i] ^= buf[i + 1];
        }
    }
    unsigned int sampleRate;
    unsigned int channels;
};

class AudioPcmuFrameSender : public FrameSender
{
public:
    explicit AudioPcmuFrameSender(ss_rtsp_client &client, const std::string &url, u_int8_t *buf,
                                    u_int32_t size, unsigned int sampleRate, unsigned int channels)
        : FrameSender(client, url, buf, size), sampleRate(sampleRate), channels(channels)
    {
        std::cout << "RTSP-CLIENT: " << "PCM L16, SampleRate: " << sampleRate << ", Channels: " << channels << std::endl;
    }
    ~AudioPcmuFrameSender() final
    {
    }
private:
    u_int32_t inputBuffer(u_int32_t frameSize, const struct timeval &timeStamp) final
    {
        struct rtsp_audio_output audio_output;
        pcmS16beToS16le(bufBase, frameSize);
        audio_output.info.sample_rate                   = sampleRate;
        audio_output.info.channels                      = channels;
        audio_output.info.format                        = RTSP_ES_FMT_AUDIO_PCMU;
        audio_output.info.sample_width                  = 8;
        audio_output.frame_package.packet_count         = 1;
        audio_output.frame_package.packet_data[0].b_end = true;
        audio_output.frame_package.packet_data[0].data  = (char *)this->bufBase;
        audio_output.frame_package.packet_data[0].size  = frameSize;
        audio_output.frame_package.stamp                = timeStamp;
        //std::cout << "PCM Send buffer size: " << audio_output.frame_package.packet_data[0].size << std::endl;
        rtspClient.recv_audio_package(this->url, audio_output, this->frameId++);
        return 0;
    }
    void pcmS16beToS16le(u_int8_t *buf, u_int32_t size)
    {
        u_int32_t i = 0;

        for (i = 0; i < size; i += 2)
        {
            buf[i] ^= buf[i + 1];
            buf[i + 1] ^= buf[i];
            buf[i] ^= buf[i + 1];
        }
    }
    unsigned int sampleRate;
    unsigned int channels;
};

class AudioPcmaFrameSender : public FrameSender
{
public:
    explicit AudioPcmaFrameSender(ss_rtsp_client &client, const std::string &url, u_int8_t *buf,
                                    u_int32_t size, unsigned int sampleRate, unsigned int channels)
        : FrameSender(client, url, buf, size), sampleRate(sampleRate), channels(channels)
    {
        std::cout << "RTSP-CLIENT: " << "PCM L16, SampleRate: " << sampleRate << ", Channels: " << channels << std::endl;
    }
    ~AudioPcmaFrameSender() final
    {
    }
private:
    u_int32_t inputBuffer(u_int32_t frameSize, const struct timeval &timeStamp) final
    {
        struct rtsp_audio_output audio_output;
        pcmS16beToS16le(bufBase, frameSize);
        audio_output.info.sample_rate                   = sampleRate;
        audio_output.info.channels                      = channels;
        audio_output.info.format                        = RTSP_ES_FMT_AUDIO_PCMA;
        audio_output.info.sample_width                  = 8;
        audio_output.frame_package.packet_count         = 1;
        audio_output.frame_package.packet_data[0].b_end = true;
        audio_output.frame_package.packet_data[0].data  = (char *)this->bufBase;
        audio_output.frame_package.packet_data[0].size  = frameSize;
        audio_output.frame_package.stamp                = timeStamp;
        //std::cout << "PCM Send buffer size: " << audio_output.frame_package.packet_data[0].size << std::endl;
        rtspClient.recv_audio_package(this->url, audio_output, this->frameId++);
        return 0;
    }
    void pcmS16beToS16le(u_int8_t *buf, u_int32_t size)
    {
        u_int32_t i = 0;

        for (i = 0; i < size; i += 2)
        {
            buf[i] ^= buf[i + 1];
            buf[i + 1] ^= buf[i];
            buf[i] ^= buf[i + 1];
        }
    }
    unsigned int sampleRate;
    unsigned int channels;
};

class AudioG726FrameSender : public FrameSender
{
public:
    explicit AudioG726FrameSender(ss_rtsp_client &client, const std::string &url, u_int8_t *buf,
                                    u_int32_t size, unsigned int sampleRate, unsigned int channels)
        : FrameSender(client, url, buf, size), sampleRate(sampleRate), channels(channels)
    {
        std::cout << "RTSP-CLIENT: " << "PCM L16, SampleRate: " << sampleRate << ", Channels: " << channels << std::endl;
    }
    ~AudioG726FrameSender() final
    {
    }
private:
    u_int32_t inputBuffer(u_int32_t frameSize, const struct timeval &timeStamp) final
    {
        struct rtsp_audio_output audio_output;
        pcmS16beToS16le(bufBase, frameSize);
        audio_output.info.sample_rate                   = sampleRate;
        audio_output.info.channels                      = channels;
        audio_output.info.format                        = RTSP_ES_FMT_AUDIO_G726;
        audio_output.info.sample_width                  = 4;
        audio_output.frame_package.packet_count         = 1;
        audio_output.frame_package.packet_data[0].b_end = true;
        audio_output.frame_package.packet_data[0].data  = (char *)this->bufBase;
        audio_output.frame_package.packet_data[0].size  = frameSize;
        audio_output.frame_package.stamp                = timeStamp;
        //std::cout << "PCM Send buffer size: " << audio_output.frame_package.packet_data[0].size << std::endl;
        rtspClient.recv_audio_package(this->url, audio_output, this->frameId++);
        return 0;
    }
    void pcmS16beToS16le(u_int8_t *buf, u_int32_t size)
    {
        u_int32_t i = 0;

        for (i = 0; i < size; i += 2)
        {
            buf[i] ^= buf[i + 1];
            buf[i + 1] ^= buf[i];
            buf[i] ^= buf[i + 1];
        }
    }
    unsigned int sampleRate;
    unsigned int channels;
};

class VideoFrameSender : public FrameSender
{
public:
    enum
    {
        SS_RTSP_CLIENT_RECV_FRAME0_IDLE = 0,
        SS_RTSP_CLIENT_RECV_FRAME0_START,
        SS_RTSP_CLIENT_RECV_FRAME1_START
    };
    explicit VideoFrameSender(ss_rtsp_client &client, const std::string &url, u_int8_t *buf,
                             u_int32_t size, u_int32_t width, u_int32_t height, u_int32_t fps)
        : FrameSender(client, url, buf, size), recvState(SS_RTSP_CLIENT_RECV_FRAME0_IDLE),
        videoWidth(width), videoHeight(height), frameFps(fps), startCode{0x0, 0x0, 0x0, 0x1}
    {
        u_int8_t *charHolder = this->bufBase;
        assert(size > this->startCodeSize);
        bAfterChecking = false;
        bufShift       = this->startCodeSize;
        charHolder[0]  = this->startCode[0];
        charHolder[1]  = this->startCode[1];
        charHolder[2]  = this->startCode[2];
        charHolder[3]  = this->startCode[3];
    }
    virtual ~VideoFrameSender()
    {
    }
    const static u_int8_t startCodeSize = 4;
protected:
    virtual u_int32_t getFormat() = 0;
    virtual bool isFrameStart() = 0;
    virtual bool isFrameEnd() = 0;
    virtual bool isFramePrefix() = 0;
    virtual bool checkFrameSize(u_int32_t frameSize) = 0;
    bool checkBeginning()
    {
        if (this->bAfterChecking)
        {
            return true;
        }
        this->bAfterChecking = this->isFramePrefix();
        return this->bAfterChecking;
    }
    bool bAfterChecking;
    u_int32_t bufShift;
private:
    u_int32_t continueGetFrame()
    {
        u_int8_t *charHolder = nullptr;
        if (this->bufShift + this->startCodeSize >= this->bufBaseSize)
        {
            std::cout << "RTSP-CLIENT vdeo buffer not enough! URL: "
                << this->url << ", current max size: " << this->bufBaseSize << std::endl;
            this->bufShift = this->startCodeSize;
            return this->bufShift;
        }
        charHolder      = this->bufBase + this->bufShift;
        charHolder[0]   = this->startCode[0];
        charHolder[1]   = this->startCode[1];
        charHolder[2]   = this->startCode[2];
        charHolder[3]   = this->startCode[3];
        this->bufShift += this->startCodeSize;
        return this->bufShift;
    }
    void recvFrame(const struct timeval &timeStamp)
    {
        struct rtsp_video_output video_output;
        video_output.info.frame_rate                    = this->frameFps;
        video_output.info.format                        = this->getFormat();
        video_output.info.width                         = this->videoWidth;
        video_output.info.height                        = this->videoHeight;
        video_output.frame_package.packet_count         = 1;
        video_output.frame_package.packet_data[0].b_end = true;
        video_output.frame_package.packet_data[0].data  = (char *)this->bufBase;
        video_output.frame_package.packet_data[0].size  = this->bufShift - this->startCodeSize;
        video_output.frame_package.stamp                = timeStamp;
        if (rtspClient.recv_video_package(this->url, video_output, this->frameId++) != 0)
        {
            this->bAfterChecking = false; // check idr enterance again.
        }
        //std::cout << "Detect frameEnd Send buffer size: " << video_output.frame_package.packet_data[0].size << std::endl;
    }
    u_int32_t processFrame(u_int32_t frameSize)
    {
        this->bufShift += frameSize;
        return this->bufShift;
    }
    u_int32_t resetFrame()
    {
        this->bufShift = this->startCodeSize;
        return this->bufShift;
    }
    u_int32_t moveFrame(u_int32_t frameSize)
    {
        memmove(this->bufBase + this->startCodeSize, this->bufBase + this->bufShift, frameSize);
        this->bufShift   = this->startCodeSize + frameSize;
        return this->bufShift;
    }
    u_int32_t inputBuffer(u_int32_t frameSize, const struct timeval &timeStamp) final
    {
        if (!this->checkFrameSize(frameSize))
        {
            std::cout << "RTSP-CLIENT video frame size check error!, size " << frameSize << std::endl;
            return this->resetFrame();
        }
        if (!this->checkBeginning())
        {
            // For h264/h265 if stream doesn't start from sps or pps, then drop this frame.
            return this->bufShift;
        }
        switch (this->recvState)
        {
            case SS_RTSP_CLIENT_RECV_FRAME0_IDLE:
                if (this->isFrameEnd())
                {
                    std::cout << "ss_rtsp_client warning : " << this->url << " hasn't received any available data!!" << std::endl;
                    this->bAfterChecking = false; // check idr enterance again.
                    return this->resetFrame();
                }
                if (this->isFrameStart())
                {
                    this->recvState = SS_RTSP_CLIENT_RECV_FRAME0_START;
                }
                this->processFrame(frameSize);
                break;
            case SS_RTSP_CLIENT_RECV_FRAME0_START:
                if (this->isFrameEnd())
                {
                    this->recvState = SS_RTSP_CLIENT_RECV_FRAME0_IDLE;
                    this->recvFrame(timeStamp);
                    return this->resetFrame();
                }
                if (this->isFramePrefix())
                {
                    /* Received pps/vps/sps, and change state. */
                    this->recvFrame(timeStamp);
                    this->moveFrame(frameSize);
                    this->recvState = SS_RTSP_CLIENT_RECV_FRAME1_START;
                    break;
                }
                if (this->isFrameStart())
                {
                    this->recvFrame(timeStamp);
                    this->moveFrame(frameSize);
                    break;
                }
                this->processFrame(frameSize);
                break;
            case SS_RTSP_CLIENT_RECV_FRAME1_START:
                if (this->isFrameStart())
                {
                    this->recvState = SS_RTSP_CLIENT_RECV_FRAME0_START;
                }
                /* After received vps/pps/sps, maybe process multi-slice or sei. */
                this->processFrame(frameSize);
                break;
            default:
                std::cout << "Error st:" << this->recvState << std::endl;
                break;
        }
        return this->continueGetFrame();
    }
    u_int32_t recvState;
    u_int32_t videoWidth;
    u_int32_t videoHeight;
    u_int32_t frameFps;
    u_int8_t  startCode[4];
};

class JpegVideoFrameSender : public VideoFrameSender
{
public:
    explicit JpegVideoFrameSender(ss_rtsp_client &client, const std::string &url, u_int8_t *buf,
                                  u_int32_t size, u_int32_t width, u_int32_t height, u_int32_t fps)
                                  : VideoFrameSender(client, url, buf, size, width, height, fps)
    {
        std::cout << "RTSP-CLIENT: " << "JPEG , Width: " << width << ", Height: " << height << ", Fps: " << fps << std::endl;
    }
    ~JpegVideoFrameSender() final
    {
    }
private:
    virtual bool checkFrameSize(u_int32_t frameSize)
    {
        return (frameSize >= 6);
    }
    virtual bool isFrameStart()
    {
        u_int8_t *charHolder = this->bufBase + this->bufShift;
        return (charHolder[5] == 0xD8);
    }
    virtual bool isFrameEnd()
    {
        return false;
    }
    virtual bool isFramePrefix()
    {
        return false;
    }
    virtual u_int32_t getFormat()
    {
        return RTSP_ES_FMT_VIDEO_JPEG;
    }
};

class H264VideoFrameSender : public VideoFrameSender
{
public:
    explicit H264VideoFrameSender(ss_rtsp_client &client, const std::string &url, u_int8_t *buf,
                                  u_int32_t size, u_int32_t width, u_int32_t height, u_int32_t fps)
                                  : VideoFrameSender(client, url, buf, size, width, height, fps)
    {
        std::cout << "RTSP-CLIENT: " << "H264, Width: " << width << ", Height: " << height << ", Fps: " << fps << std::endl;
    }
    ~H264VideoFrameSender() final
    {
    }
private:
    virtual bool checkFrameSize(u_int32_t frameSize)
    {
        return (frameSize >= 4);
    }
    virtual bool isFrameStart()
    {
        u_int8_t *charHolder = this->bufBase + this->bufShift;
        u_int8_t naluType = charHolder[0] & 0x1F;
        return (naluType >= 1 && naluType <= 5) && (charHolder[1] & 0x80);
    }
    virtual bool isFrameEnd()
    {
        u_int8_t *charHolder = this->bufBase + this->bufShift;
        return CHECK_H264_FRAME_END_PAYLOAD(charHolder);
    }
    virtual bool isFramePrefix()
    {
        u_int8_t *charHolder = this->bufBase + this->bufShift;
        u_int8_t naluType = charHolder[0] & 0x1F;
        return (naluType == 7 || naluType == 8); //check sps pps
    }
    virtual u_int32_t getFormat()
    {
        return RTSP_ES_FMT_VIDEO_H264;
    }
    bool bAfterChecking;
};

class H265VideoFrameSender : public VideoFrameSender
{
public:
    explicit H265VideoFrameSender(ss_rtsp_client &client, const std::string &url, u_int8_t *buf,
                                  u_int32_t size, u_int32_t width, u_int32_t height, u_int32_t fps)
                                  : VideoFrameSender(client, url, buf, size, width, height, fps)
    {
        std::cout << "RTSP-CLIENT: " << "H265, Width: " << width << ", Height: " << height << ", Fps: " << fps << std::endl;
    }
    ~H265VideoFrameSender() final
    {
    }
private:
    virtual bool checkFrameSize(u_int32_t frameSize)
    {
        return (frameSize >= 4);
    }
    virtual bool isFrameStart()
    {
        u_int8_t *charHolder = this->bufBase + this->bufShift;
        u_int8_t naluType = (charHolder[0] & 0x7E) >> 1;
        return (naluType <= 31 && (charHolder[2] & 0x80));
    }
    virtual bool isFrameEnd()
    {
        u_int8_t *charHolder = this->bufBase + this->bufShift;
        return CHECK_H265_FRAME_END_PAYLOAD(charHolder);
    }
    virtual bool isFramePrefix()
    {
        u_int8_t *charHolder = this->bufBase + this->bufShift;
        u_int8_t naluType    = (charHolder[0] & 0x7E) >> 1;
        return (32 == naluType || 33 == naluType || 34 == naluType);
    }
    virtual u_int32_t getFormat()
    {
        return RTSP_ES_FMT_VIDEO_H265;
    }
};

// SigmaStar coding end.

class StreamClientState {
public:
    StreamClientState();
    virtual ~StreamClientState();

public:
    MediaSubsessionIterator* iter;
    MediaSession* session;
    MediaSubsession* subsession;
    TaskToken streamTimerTask;
    double duration;
};

// If you're streaming just a single stream (i.e., just from a single URL, once), then you can define and use just a single
// "StreamClientState" structure, as a global variable in your application.  However, because - in this demo application - we're
// showing how to play multiple streams, concurrently, we can't do that.  Instead, we have to have a separate "StreamClientState"
// structure for each "RTSPClient".  To do this, we subclass "RTSPClient", and add a "StreamClientState" field to the subclass:

class ourRTSPClient: public RTSPClient {
public:
    static ourRTSPClient* createNew(UsageEnvironment& env, char const* rtspURL,
				  int verbosityLevel = 0,
				  char const* applicationName = NULL,
				  portNumBits tunnelOverHTTPPortNum = 0);

protected:
    ourRTSPClient(UsageEnvironment& env, char const* rtspURL,
		int verbosityLevel, char const* applicationName, portNumBits tunnelOverHTTPPortNum);
    // called only by createNew();
    virtual ~ourRTSPClient();

public:
    StreamClientState scs;
};

// Define a data sink (a subclass of "MediaSink") to receive the data for each subsession (i.e., each audio or video 'substream').
// In practice, this might be a class (or a chain of classes) that decodes and then renders the incoming audio or video.
// Or it might be a "FileSink", for outputting the received data into a file (as is done by the "openRTSP" application).
// In this example code, however, we define a simple 'dummy' sink that receives incoming data, but does nothing with it.

class DummySink: public MediaSink {
public:
    static DummySink* createNew(UsageEnvironment& env,
                  MediaSubsession& subsession, // identifies the kind of data that's being received
                  char const* streamId = NULL); // identifies the stream itself (optional)

private:
    DummySink(UsageEnvironment& env, MediaSubsession& subsession, char const* streamId);
    // called only by "createNew()"
    virtual ~DummySink();

    static void afterGettingFrame(void* clientData, unsigned frameSize,
                                unsigned numTruncatedBytes,
                  struct timeval presentationTime,
                                unsigned durationInMicroseconds);
    void afterGettingFrame(unsigned frameSize, unsigned numTruncatedBytes,
                  struct timeval presentationTime, unsigned durationInMicroseconds);

private:
    // redefined virtual functions:
    virtual Boolean continuePlaying();

private:
    u_int8_t* fReceiveBuffer;
    unsigned int fReceiveBufferSize;
    u_int32_t u32MultiSliceOffset;
    MediaSubsession& fSubsession;
    char* fStreamId;

    // Sigmastar frame sender.
    FrameSender *fFrameSender;

};

// A function that outputs a string that identifies each stream (for debugging output).  Modify this if you wish:
static UsageEnvironment& operator<<(UsageEnvironment& env, const RTSPClient& rtspClient) {
  return env << "[URL:\"" << rtspClient.url() << "\"]: ";
}

// A function that outputs a string that identifies each subsession (for debugging output).  Modify this if you wish:
static UsageEnvironment& operator<<(UsageEnvironment& env, const MediaSubsession& subsession) {
  return env << subsession.mediumName() << "/" << subsession.codecName();
}

TopUsageEnvironment* TopUsageEnvironment::createNew(ss_rtsp_client_env &rtspRef, TaskScheduler& taskScheduler) {
    return new TopUsageEnvironment(rtspRef, taskScheduler);
}

static void shutdownStream(RTSPClient* rtspClient, int exitCode) {
  StreamClientState& scs = ((ourRTSPClient*)rtspClient)->scs; // alias

  // First, check whether any subsessions have still to be closed:
  if (scs.session != NULL) {
    Boolean someSubsessionsWereActive = False;
    MediaSubsessionIterator iter(*scs.session);
    MediaSubsession* subsession;

    while ((subsession = iter.next()) != NULL) {
      if (subsession->sink != NULL) {
	Medium::close(subsession->sink);
	subsession->sink = NULL;

	if (subsession->rtcpInstance() != NULL) {
	  subsession->rtcpInstance()->setByeHandler(NULL, NULL); // in case the server sends a RTCP "BYE" while handling "TEARDOWN"
	}

	someSubsessionsWereActive = True;
      }
    }

    if (someSubsessionsWereActive) {
      // Send a RTSP "TEARDOWN" command, to tell the server to shutdown the stream.
      // Don't bother handling the response to the "TEARDOWN".
      rtspClient->sendTeardownCommand(*scs.session, NULL);
    }
  }
}
// Implementation of the other event handlers:

static void subsessionAfterPlaying(void* clientData) {
  MediaSubsession* subsession = (MediaSubsession*)clientData;
  RTSPClient* rtspClient = (RTSPClient*)(subsession->miscPtr);

  // Begin by closing this subsession's stream:
  Medium::close(subsession->sink);
  subsession->sink = NULL;

  // Next, check whether *all* subsessions' streams have now been closed:
  MediaSession& session = subsession->parentSession();
  MediaSubsessionIterator iter(session);
  while ((subsession = iter.next()) != NULL) {
    if (subsession->sink != NULL) return; // this subsession is still active
  }

  // All subsessions' streams have now been closed, so shutdown the client:
  shutdownStream(rtspClient);
}

static void subsessionByeHandler(void* clientData, char const* reason) {
  MediaSubsession* subsession = (MediaSubsession*)clientData;
  RTSPClient* rtspClient = (RTSPClient*)subsession->miscPtr;
  UsageEnvironment& env = rtspClient->envir(); // alias

  env << *rtspClient << "Received RTCP \"BYE\"";
  if (reason != NULL) {
    env << " (reason:\"" << reason << "\")";
    delete[] reason;
  }
  env << " on \"" << *subsession << "\" subsession\n";

  // Now act as if the subsession had closed:
  subsessionAfterPlaying(subsession);
}

static void setupNextSubsession(RTSPClient* rtspClient) {
  UsageEnvironment& env = rtspClient->envir(); // alias
  StreamClientState& scs = ((ourRTSPClient*)rtspClient)->scs; // alias

  scs.subsession = scs.iter->next();
  if (scs.subsession != NULL) {
    if (!scs.subsession->initiate()) {
      env << *rtspClient << "Failed to initiate the \"" << *scs.subsession << "\" subsession: " << env.getResultMsg() << "\n";
      setupNextSubsession(rtspClient); // give up on this subsession; go to the next one
    } else {
      env << *rtspClient << "Initiated the \"" << *scs.subsession << "\" subsession (";
      if (scs.subsession->rtcpIsMuxed()) {
	env << "client port " << scs.subsession->clientPortNum();
      } else {
	env << "client ports " << scs.subsession->clientPortNum() << "-" << scs.subsession->clientPortNum()+1;
      }
      env << ")\n";

      // Continue setting up this subsession, by sending a RTSP "SETUP" command:
      rtspClient->sendSetupCommand(*scs.subsession, continueAfterSETUP, False, REQUEST_STREAMING_OVER_TCP);
    }
    return;
  }

  // We've finished setting up all of the subsessions.  Now, send a RTSP "PLAY" command to start the streaming:
  if (scs.session->absStartTime() != NULL) {
    // Special case: The stream is indexed by 'absolute' time, so send an appropriate "PLAY" command:
    rtspClient->sendPlayCommand(*scs.session, continueAfterPLAY, scs.session->absStartTime(), scs.session->absEndTime());
  } else {
    scs.duration = scs.session->playEndTime() - scs.session->playStartTime();
    rtspClient->sendPlayCommand(*scs.session, continueAfterPLAY);
  }
}

static void continueAfterSETUP(RTSPClient* rtspClient, int resultCode, char* resultString) {
  do {
    UsageEnvironment& env = rtspClient->envir(); // alias
    StreamClientState& scs = ((ourRTSPClient*)rtspClient)->scs; // alias

    if (resultCode != 0) {
      env << *rtspClient << "Failed to set up the \"" << *scs.subsession << "\" subsession: " << resultString << "\n";
      break;
    }

    env << *rtspClient << "Set up the \"" << *scs.subsession << "\" subsession (";
    if (scs.subsession->rtcpIsMuxed()) {
      env << "client port " << scs.subsession->clientPortNum();
    } else {
      env << "client ports " << scs.subsession->clientPortNum() << "-" << scs.subsession->clientPortNum()+1;
    }
    env << ")\n";

    // Having successfully setup the subsession, create a data sink for it, and call "startPlaying()" on it.
    // (This will prepare the data sink to receive data; the actual flow of data from the client won't start happening until later,
    // after we've sent a RTSP "PLAY" command.)

    scs.subsession->sink = DummySink::createNew(env, *scs.subsession, rtspClient->url());
      // perhaps use your own custom "MediaSink" subclass instead
    if (scs.subsession->sink == NULL) {
      env << *rtspClient << "Failed to create a data sink for the \"" << *scs.subsession
	  << "\" subsession: " << env.getResultMsg() << "\n";
      break;
    }

    env << *rtspClient << "Created a data sink for the \"" << *scs.subsession << "\" subsession\n";
    scs.subsession->miscPtr = rtspClient; // a hack to let subsession handler functions get the "RTSPClient" from the subsession
    scs.subsession->sink->startPlaying(*(scs.subsession->readSource()),
				       subsessionAfterPlaying, scs.subsession);
    // Also set a handler to be called if a RTCP "BYE" arrives for this subsession:
    if (scs.subsession->rtcpInstance() != NULL) {
      scs.subsession->rtcpInstance()->setByeWithReasonHandler(subsessionByeHandler, scs.subsession);
    }
  } while (0);
  delete[] resultString;

  // Set up the next subsession, if any:
  setupNextSubsession(rtspClient);
}

// Implementation of the RTSP 'response handlers':
static void continueAfterDESCRIBE(RTSPClient* rtspClient, int resultCode, char* resultString) {
  do {
    UsageEnvironment& env = rtspClient->envir(); // alias
    StreamClientState& scs = ((ourRTSPClient*)rtspClient)->scs; // alias

    if (resultCode != 0) {
      env << *rtspClient << "Failed to get a SDP description: " << resultString << "\n";
      delete[] resultString;
      break;
    }

    char* const sdpDescription = resultString;
    env << *rtspClient << "Got a SDP description:\n" << sdpDescription << "\n";

    // Create a media session object from this SDP description:
    scs.session = MediaSession::createNew(env, sdpDescription);
    delete[] sdpDescription; // because we don't need it anymore
    if (scs.session == NULL) {
      env << *rtspClient << "Failed to create a MediaSession object from the SDP description: " << env.getResultMsg() << "\n";
      break;
    } else if (!scs.session->hasSubsessions()) {
      env << *rtspClient << "This session has no media subsessions (i.e., no \"m=\" lines)\n";
      break;
    }

    // Then, create and set up our data source objects for the session.  We do this by iterating over the session's 'subsessions',
    // calling "MediaSubsession::initiate()", and then sending a RTSP "SETUP" command, on each one.
    // (Each 'subsession' will have its own data source.)
    scs.iter = new MediaSubsessionIterator(*scs.session);
    setupNextSubsession(rtspClient);
    return;
  } while (0);

  // An unrecoverable error occurred with this stream.
  shutdownStream(rtspClient);
}

static RTSPClient* openURL(UsageEnvironment& env, char const* progName, char const* rtspURL, Authenticator* pAuth) {
    // Begin by creating a "RTSPClient" object.  Note that there is a separate "RTSPClient" object for each stream that we wish
    // to receive (even if more than stream uses the same "rtsp://" URL).
    RTSPClient* rtspClient = ourRTSPClient::createNew(env, rtspURL, RTSP_CLIENT_VERBOSITY_LEVEL, progName);
    if (rtspClient == NULL) {
        env << "Failed to create a RTSP client for URL \"" << rtspURL << "\": " << env.getResultMsg() << "\n";
        return NULL;
    }

    // Next, send a RTSP "DESCRIBE" command, to get a SDP description for the stream.
    // Note that this command - like all RTSP commands - is sent asynchronously; we do not block, waiting for a response.
    // Instead, the following function call returns immediately, and we handle the RTSP response later, from within the event loop:
    rtspClient->sendDescribeCommand(continueAfterDESCRIBE, pAuth);

    return rtspClient;
}

// Implementation of "ourRTSPClient":

ourRTSPClient* ourRTSPClient::createNew(UsageEnvironment& env, char const* rtspURL,
					int verbosityLevel, char const* applicationName, portNumBits tunnelOverHTTPPortNum) {
  return new ourRTSPClient(env, rtspURL, verbosityLevel, applicationName, tunnelOverHTTPPortNum);
}

ourRTSPClient::ourRTSPClient(UsageEnvironment& env, char const* rtspURL,
			     int verbosityLevel, char const* applicationName, portNumBits tunnelOverHTTPPortNum)
  : RTSPClient(env,rtspURL, verbosityLevel, applicationName, tunnelOverHTTPPortNum, -1) {
}

ourRTSPClient::~ourRTSPClient() {
}

static void continueAfterPLAY(RTSPClient* rtspClient, int resultCode, char* resultString) {
  Boolean success = False;

  do {
    UsageEnvironment& env = rtspClient->envir(); // alias
    StreamClientState& scs = ((ourRTSPClient*)rtspClient)->scs; // alias

    if (resultCode != 0) {
      env << *rtspClient << "Failed to start playing session: " << resultString << "\n";
      break;
    }

    // Set a timer to be handled at the end of the stream's expected duration (if the stream does not already signal its end
    // using a RTCP "BYE").  This is optional.  If, instead, you want to keep the stream active - e.g., so you can later
    // 'seek' back within it and do another RTSP "PLAY" - then you can omit this code.
    // (Alternatively, if you don't want to receive the entire stream, you could set this timer for some shorter value.)
    if (scs.duration > 0) {
      unsigned const delaySlop = 2; // number of seconds extra to delay, after the stream's expected duration.  (This is optional.)
      scs.duration += delaySlop;
      unsigned uSecsToDelay = (unsigned)(scs.duration*1000000);
      scs.streamTimerTask = env.taskScheduler().scheduleDelayedTask(uSecsToDelay, (TaskFunc*)streamTimerHandler, rtspClient);
    }

    env << *rtspClient << "Started playing session";
    if (scs.duration > 0) {
      env << " (for up to " << scs.duration << " seconds)";
    }
    env << "...\n";

    success = True;
  } while (0);
  delete[] resultString;

  if (!success) {
    // An unrecoverable error occurred with this stream.
    shutdownStream(rtspClient);
  }
}

static void streamTimerHandler(void* clientData) {
  ourRTSPClient* rtspClient = (ourRTSPClient*)clientData;
  StreamClientState& scs = rtspClient->scs; // alias

  scs.streamTimerTask = NULL;

  // Shut down the stream:
  shutdownStream(rtspClient);
}

// Implementation of "StreamClientState":

StreamClientState::StreamClientState()
  : iter(NULL), session(NULL), subsession(NULL), streamTimerTask(NULL), duration(0.0) {
}

StreamClientState::~StreamClientState() {
  delete iter;
  if (session != NULL) {
    // We also need to delete "session", and unschedule "streamTimerTask" (if set)
    UsageEnvironment& env = session->envir(); // alias

    env.taskScheduler().unscheduleDelayedTask(streamTimerTask);
    Medium::close(session);
  }
}

// Implementation of "DummySink":

// Even though we're not going to be doing anything with the incoming data, we still need to receive it.
// Define the size of the buffer that we'll use:
#define DUMMY_SINK_RECEIVE_BUFFER_SIZE 0x200000

DummySink* DummySink::createNew(UsageEnvironment& env, MediaSubsession& subsession, char const* streamId) {
  return new DummySink(env, subsession, streamId);
}

DummySink::DummySink(UsageEnvironment& env, MediaSubsession& subsession, char const* streamId)
    : MediaSink(env), fSubsession(subsession){
    fStreamId = strDup(streamId);

    std::string tmpMediaStr;
    std::string tmpCodecStr;
    TopUsageEnvironment *pEnv = (dynamic_cast<TopUsageEnvironment *>(&env));
    if (!pEnv)
    {
        std::cout << "TopUsageEnvironment is null" << std::endl;
        return;
    }
    fFrameSender        = nullptr;
    fReceiveBuffer      = nullptr;
    u32MultiSliceOffset = 0;
    fReceiveBufferSize  = pEnv->rtspReferance.get_buffer_size();
    fReceiveBuffer      = new u_int8_t[fReceiveBufferSize];
    if (!fReceiveBuffer)
    {
        std::cout << "RTSP-CLIENT Recv buffer alloc fail!" << std::endl;
        return;
    }
    tmpMediaStr    = fSubsession.mediumName();
    tmpCodecStr    = fSubsession.codecName();
    if (tmpMediaStr == "audio")
    {
        unsigned int sampleRate = (unsigned int)fSubsession.rtpTimestampFrequency();
        unsigned int channels   = (unsigned int)fSubsession.numChannels();
        if (tmpCodecStr == "L16")
        {
            fFrameSender = new AudioPcmL16FrameSender(pEnv->rtspReferance.get_obj(), pEnv->rtspReferance.get_url(),
                                                      fReceiveBuffer, fReceiveBufferSize, sampleRate, channels);
        }
        else if (tmpCodecStr == "PCMU")
        {
            fFrameSender = new AudioPcmuFrameSender(pEnv->rtspReferance.get_obj(), pEnv->rtspReferance.get_url(),
                                                      fReceiveBuffer, fReceiveBufferSize, sampleRate, channels);
        }
        else if (tmpCodecStr == "PCMA")
        {
            fFrameSender = new AudioPcmaFrameSender(pEnv->rtspReferance.get_obj(), pEnv->rtspReferance.get_url(),
                                                      fReceiveBuffer, fReceiveBufferSize, sampleRate, channels);
        }
        else if (tmpCodecStr == "G726-32")
        {
            fFrameSender = new AudioG726FrameSender(pEnv->rtspReferance.get_obj(), pEnv->rtspReferance.get_url(),
                                                      fReceiveBuffer, fReceiveBufferSize, sampleRate, channels);
        }
        else if (tmpCodecStr == "MPEG4-GENERIC")
        {
            fFrameSender = new AudioAacFrameSender(pEnv->rtspReferance.get_obj(), pEnv->rtspReferance.get_url(),
                                                   fReceiveBuffer, fReceiveBufferSize, sampleRate, channels);
        }
        else
        {
            printf("not support audio format [%s]!\n", tmpCodecStr.c_str());
            return;
        }
    }
    else if (tmpMediaStr == "video")
    {
        u_int32_t width     = fSubsession.videoWidth();
        u_int32_t height    = fSubsession.videoHeight();
        u_int32_t fps       = fSubsession.videoFPS();
        u32MultiSliceOffset = VideoFrameSender::startCodeSize;
        if (tmpCodecStr == "H264")
        {
            fFrameSender = new H264VideoFrameSender(pEnv->rtspReferance.get_obj(), pEnv->rtspReferance.get_url(),
                                                   fReceiveBuffer, fReceiveBufferSize, width, height, fps);
        }
        else if (tmpCodecStr == "H265")
        {
            fFrameSender = new H265VideoFrameSender(pEnv->rtspReferance.get_obj(), pEnv->rtspReferance.get_url(),
                                                   fReceiveBuffer, fReceiveBufferSize, width, height, fps);
        }
        else if (tmpCodecStr == "JPEG")
        {
            fFrameSender = new JpegVideoFrameSender(pEnv->rtspReferance.get_obj(), pEnv->rtspReferance.get_url(),
                                                   fReceiveBuffer, fReceiveBufferSize, width, height, fps);
        }
        else
        {
            printf("not support video format [%s]!\n", tmpCodecStr.c_str());
            return;
        }
    }
}

DummySink::~DummySink() {
    if (fReceiveBuffer)
    {
        delete[] fReceiveBuffer;
    }
    if (fStreamId)
    {
        delete[] fStreamId;
    }
    if (fFrameSender)
    {
        delete fFrameSender;
    }
}

void DummySink::afterGettingFrame(void* clientData, unsigned frameSize, unsigned numTruncatedBytes,
				  struct timeval presentationTime, unsigned durationInMicroseconds) {
  DummySink* sink = (DummySink*)clientData;
  sink->afterGettingFrame(frameSize, numTruncatedBytes, presentationTime, durationInMicroseconds);
}

// If you don't want to see debugging output for each received frame, then comment out the following line:
#define DEBUG_PRINT_EACH_RECEIVED_FRAME 1

void DummySink::afterGettingFrame(unsigned frameSize, unsigned numTruncatedBytes,
				  struct timeval presentationTime, unsigned /*durationInMicroseconds*/) {
    // We've just received a frame of data.  (Optionally) print out information about it:
#if 0//DEBUG_PRINT_EACH_RECEIVED_FRAME
    if (fStreamId != NULL) envir() << "Stream \"" << fStreamId << "\"; ";
    envir() << fSubsession.mediumName() << "/" << fSubsession.codecName() << ":\tReceived " << frameSize << " bytes";
    if (numTruncatedBytes > 0) envir() << " (with " << numTruncatedBytes << " bytes truncated)";
    char uSecsStr[6+1]; // used to output the 'microseconds' part of the presentation time
    sprintf(uSecsStr, "%06u", (unsigned)presentationTime.tv_usec);
    envir() << ".\tPresentation time: " << (int)presentationTime.tv_sec << "." << uSecsStr;
    if (fSubsession.rtpSource() != NULL && !fSubsession.rtpSource()->hasBeenSynchronizedUsingRTCP()) {
        envir() << "!"; // mark the debugging output to indicate that this presentation time is not RTCP-synchronized
    }
#ifdef DEBUG_PRINT_NPT
    envir() << "\tNPT: " << fSubsession.getNormalPlayTime(presentationTime);
#endif
    envir() << "\n";
#endif
    if (fFrameSender)
    {
        u32MultiSliceOffset = fFrameSender->inputBuffer(frameSize, presentationTime);
    }
    else
    {
        std::cout << "RTSP-CLIENT ERR: Frame sender is nullptr" << std::endl;
    }
    // Then continue, to request the next frame of data:
    continuePlaying();
}

Boolean DummySink::continuePlaying() {
  if (fSource == NULL) return False; // sanity check (should not happen)

  // Request the next frame of data from our input source.  "afterGettingFrame()" will get called later, when it arrives:
  fSource->getNextFrame(fReceiveBuffer + u32MultiSliceOffset, fReceiveBufferSize- u32MultiSliceOffset,
                        afterGettingFrame, this,
                        onSourceClosure, this);
  return True;
}

ss_rtsp_client::ss_rtsp_client()
{
}

ss_rtsp_client::~ss_rtsp_client()
{
    for (auto iter = rtsp_output.begin(); iter != rtsp_output.end(); ++iter)
    {
        stop(iter->first);
    }
    rtsp_output.clear();
}

int ss_rtsp_client::play(const std::string& url, const std::string& userName, const std::string& pwd,
                         unsigned int width, unsigned int height, unsigned int bufDiv)
{
    auto iter = rtsp_output.find(url);

    if (iter != rtsp_output.end())
    {
        return -1;
    }

    if (!bufDiv)
    {
        bufDiv = 10;
    }
    unsigned int recv_buffer_size = width * height * 10 / bufDiv;
    if (!recv_buffer_size)
    {
        recv_buffer_size = DUMMY_SINK_RECEIVE_BUFFER_SIZE;
    }
    rtsp_output[url] = (void *)new ss_rtsp_client_env(*this, url, userName, pwd, recv_buffer_size);
    assert(rtsp_output[url]);
    return 0;
}

int ss_rtsp_client::stop(const std::string &url)
{
    auto iter = rtsp_output.find(url);

    if (iter == rtsp_output.end())
    {
        return -1;
    }
    ss_rtsp_client_env *this_env = (ss_rtsp_client_env *)iter->second;
    assert(this_env);
    delete this_env;
    iter->second = NULL;
    rtsp_output.erase(iter);
    return 0;
}
