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

#ifndef __AMIGOS_MODULE_FILE_H__
#define __AMIGOS_MODULE_FILE_H__

#include <pthread.h>
#include <map>
#include <string>
#include "ss_filter.h"
#include "ss_linker.h"
#include "amigos_module_base.h"
#include "amigos_surface_file.h"
class AmigosModuleFile : public AmigosSurfaceFile, public AmigosModuleBase
{
public:
    struct FileReaderDesc
    {
        FILE                      *readFp;
        LinkerGroup               *linkerOut;
        stream_packer             *packer;
        unsigned int              lastTimeSec;
        unsigned int              lastTimeNsec;
        unsigned int              gapTimeNs;
        unsigned int              gapDataSize;
        int frameCntLimit;
        void                      *threadHandle;
        stream_packet_obj         packet;
        struct stream_packet_info info;
        ss_video_packet_filter    video_filter;
        FileReaderDesc()
            : readFp(nullptr), linkerOut(nullptr), packer(nullptr), lastTimeSec(0), lastTimeNsec(0), gapTimeNs(0),
              gapDataSize(0),frameCntLimit(-1),threadHandle(nullptr), packet(nullptr)
        {}
    };
    struct FileWriterDesc
    {
        FILE *writeFp;
        int frameCntLimit;
        int frameSkipCount;
        bool bHead;
        pthread_mutex_t    frame_mutex;
        pthread_cond_t     frame_cond;
        pthread_condattr_t frame_cond_attr;
        ss_video_packet_filter video_filter;
        FileWriterDesc() : writeFp(nullptr), frameCntLimit(-1), frameSkipCount(0), bHead(false)
        {
            pthread_mutex_init(&frame_mutex, NULL);
            pthread_condattr_init(&frame_cond_attr);
            pthread_condattr_setclock(&frame_cond_attr, CLOCK_MONOTONIC);
            pthread_cond_init(&frame_cond, &frame_cond_attr);
        }
        ~FileWriterDesc()
        {
            pthread_condattr_destroy(&frame_cond_attr);
            pthread_cond_destroy(&frame_cond);
            pthread_mutex_destroy(&frame_mutex);
        }
    };
    explicit AmigosModuleFile(const std::string &strSection);
    ~AmigosModuleFile() override;

    unsigned int GetModId() const override;
    unsigned int GetInputType(unsigned int port) const override;
    unsigned int GetOutputType(unsigned int port) const override;
    // Set the left frames.
    void SetLeftFrame(unsigned int inPortId, unsigned int frameCnt);
    // Wait frame count down until zero, return false if it is timeout or error.
    bool WaitFrame(unsigned int inPortId, unsigned int waitMs);

protected:
    void _Init() override;
    void _Deinit() override;

    int _DataReceiver(unsigned int inPortId, stream_packet_obj &packet) override;

    void _StartIn(unsigned int inPortId) override;
    void _StopIn(unsigned int inPortId) override;

    void _StartOut(unsigned int outPortId) override;
    void _StopOut(unsigned int outPortId) override;

    int _Connected(unsigned int outPortId, unsigned int ref) override;
    int _Disconnected(unsigned int outPortId, unsigned int ref) override;
private:
    int _EnqueueOut(unsigned int outPortId, stream_packet_obj &packet) override;
    int _DequeueOut(unsigned int outPortId, stream_packet_obj &packet) override;
    stream_packet_info _GetStreamInfo(unsigned int outPortId) override;
    stream_packet_obj _DequeueFromInside(unsigned int outPortId, unsigned int ms) override;
    stream_packet_obj _KickPacket(stream_packer *packer, unsigned int outPortId);

    void _SetFileReaderDesc(unsigned int outPortId);
    std::map<unsigned int, struct FileWriterDesc> mapFileWriterDesc;
    std::map<unsigned int, struct FileReaderDesc> mapFileReaderDesc;
};
#endif
