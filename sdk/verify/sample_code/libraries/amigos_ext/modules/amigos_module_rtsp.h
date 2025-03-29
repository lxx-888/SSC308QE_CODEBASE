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

#ifndef __AMIGOS_MODULE_RTSP_H__
#define __AMIGOS_MODULE_RTSP_H__

#include <unistd.h>
#include <map>
#include <string>

#include "ss_packet.h"
#include "ss_rtsp.h"

#include "amigos_module_base.h"
#include "amigos_surface_rtsp.h"

typedef struct stSsRtspDesc_s
{
    void                   *pPoolhandle;
    class AmigosModuleRtsp *thisClass;
    unsigned int            uintRefCnt;
    unsigned int            uintInPort;
} stSsRtspInDesc_t;

class AmigosModuleRtsp : public ss_rtsp, public ss_rtsp_client, public AmigosSurfaceRtsp, public AmigosModuleBase
{
public:
    struct RtspInDesc
    {
        ss_message *video_in_message;
        ss_message *audio_in_message;
        void       *video_pool_handle;
        void       *audio_pool_handle;
        RtspInDesc ()
            : video_in_message(nullptr), audio_in_message(nullptr), video_pool_handle(nullptr), audio_pool_handle(nullptr)
        {}
    };

    struct RtspOutDesc
    {
        ss_linker_base *video_linker;
        ss_linker_base *audio_linker;
        unsigned int   output_ref;
        RtspOutDesc () : video_linker(nullptr), audio_linker(nullptr), output_ref(0)
        {}
    };
    class LinkerRtspServer: public LinkerSyncNegative
    {
    public:
        LinkerRtspServer(unsigned int delay, unsigned inPortId, class AmigosModuleRtsp *thisModule)
            : LinkerSyncNegative(inPortId, thisModule), retryDelay(delay)
        {
        }
        ~LinkerRtspServer() final {}
    public:
        static const int RETRY_LATER = -1234;
    private:
        int enqueue(stream_packet_obj &packet) override final
        {
            int ret = LinkerSyncNegative::enqueue(packet);
            if (ret == RETRY_LATER)
            {
                AMILOG << "Retry es frame. Maybe client did not close or the network environment is bad." << std::endl;
                usleep(retryDelay);
                ret = LinkerSyncNegative::enqueue(packet);
            }
            return (ret == RETRY_LATER) ? -1 : ret;
        }
    private:
        unsigned int retryDelay;
    };
public:
    explicit AmigosModuleRtsp(const std::string &strSection);
    ~AmigosModuleRtsp() override;

    unsigned int GetModId() const override;
    unsigned int GetInputType(unsigned int port) const override;
    unsigned int GetOutputType(unsigned int port) const override;
protected:
    void _Init() override;
    void _Deinit() override;
    void _Start() override;
    void _Stop() override;

    void _StartOut(unsigned int outPortId) override;
    void _StopOut(unsigned int outPortId) override;

    int _DataReceiver(unsigned int inPortId, stream_packet_obj &packet) override;

    void _StartIn(unsigned int inPortId) override;
    void _StopIn(unsigned int inPortId) override;

    int _Connected(unsigned int outPortId, unsigned int ref) override;
    int _Disconnected(unsigned int outPortId, unsigned int ref) override;

    bool _IsDelayConnected(unsigned int) override
    {
        return true;
    }
    ss_linker_base *_CreateInputNegativeLinker(unsigned int inPortId) override;
    stream_packet_info _GetStreamInfo(unsigned int outPortId) override;
public:
    // ss_rtsp virtual api.
    void connect_video_stream(const std::string &url) override;
    void disconnect_video_stream(const std::string &url) override;
    void connect_audio_stream(const std::string &url) override;
    void disconnect_audio_stream(const std::string &url) override;

    // ss_rtsp_client virtual api.
    int recv_video_package(const std::string &url, const struct rtsp_video_output &video_output,
                                    unsigned int frame_id) override;
    int recv_audio_package(const std::string &url, const struct rtsp_audio_output &audio_output,
                                    unsigned int frame_id) override;
private:
    void SaveEnvUrlPrefix();
    void ClearEnvUrlPrefix();
    std::map<std::string, RtspInDesc>  mapWriterDesc;
    std::map<std::string, RtspOutDesc> mapReaderDesc;
};

#endif
