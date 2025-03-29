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

#ifndef __AMIGOS_MODULE_TICK_H__
#define __AMIGOS_MODULE_TICK_H__

#include <atomic>
#include "amigos_module_base.h"
#include "amigos_surface_tick.h"
#include "ss_packet.h"

class AmigosModuleTick: public AmigosSurfaceTick, public AmigosModuleBase
{
    public:
        explicit AmigosModuleTick(const std::string &strInSection);
        ~AmigosModuleTick() override;
        unsigned int GetModId() const override;
        unsigned int GetInputType(unsigned int port) const override;
        unsigned int GetOutputType(unsigned int port) const override;
        void SetBlockTime(long sec, long nsec);
        void SetRateFps(unsigned int srcFps, unsigned int inFps);
        void SetDestFps(unsigned int dstFps);
        class TickEvent
        {
        public:
            typedef enum
            {
                EN_CONTROL_FPS_BLOCK,
                EN_CONTROL_FPS_RATE,
                EN_CONTROL_FPS_DEST
            } EN_CONTROL_FPS_TYPE;
            TickEvent();
            virtual ~TickEvent();
            void Notify();
            void SetTimer(long sec, long nsec);
            bool Wait();
            bool CalcPacketNum();
            bool CalcPacketTime();
            void SetFrcType(EN_CONTROL_FPS_TYPE type)
            {
                enCtlFpsType = type;
            };
            void SetSrcFps(unsigned int fps)
            {
                uiSrcFps = fps;
            };
            void SetDstFps(unsigned int fps)
            {
                uiDstFps = fps;
            };
        private:
            struct timespec    stDiffTime;
            struct timespec    stStartTime;
            pthread_mutex_t    eventMutex;
            pthread_cond_t     eventCond;
            pthread_condattr_t eventCondAttr;
            std::atomic<EN_CONTROL_FPS_TYPE> enCtlFpsType;
            std::atomic<unsigned int>        uiSrcFps;
            std::atomic<unsigned int>        uiDstFps;
            unsigned int                     uiPacketNum;
        };
        class LinkerTickOutput : public ss_linker_base
        {
        public:
            explicit LinkerTickOutput(InPortInfo *inInfo, TickEvent *tick);
            ~LinkerTickOutput() override;
        private:
            int enqueue(stream_packet_obj &packet) override;
            stream_packet_obj dequeue(unsigned int ms);
            InPortInfo *inInfo;
            TickEvent  *tickTimer;
        };
        class LinkerTickOutputFifo: public ss_linker_base
        {
        public:
            explicit LinkerTickOutputFifo(InPortInfo *inInfo, TickEvent *tick);
            ~LinkerTickOutputFifo() override;
        private:
            int enqueue(stream_packet_obj &packet) override;
            stream_packet_obj dequeue(unsigned int ms) override;
            InPortInfo *inInfo;
            TickEvent  *tickTimer;
            std::list<stream_packet_obj> packetList;
            pthread_mutex_t linkerMutex;
        };
    private:
        void _Init() override;
        void _Deinit() override;
        int _ConnectedTransfer(unsigned int outPortId) override;
        int _DisconnectedTransfer(unsigned int outPortId) override;
        int _InConnect(unsigned int inPortId) override;
        stream_packet_info _GetStreamInfo(unsigned int outPortId) override;
        ss_linker_base *_CreateInputNegativeLinker(unsigned int inPortId) override;
        ss_linker_base *_CreateOutputNegativeLinker(unsigned int outPortId) override;
        stream_packer *_CreateInputStreamPacker(unsigned int inPortId, bool &bFast) override;
        bool _IsPostReader(unsigned int inPortId) override;
        TickEvent tick;
};
#endif

