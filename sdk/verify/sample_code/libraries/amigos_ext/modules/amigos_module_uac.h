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

#ifndef __AMIGOS_MODULE_UAC_H__
#define __AMIGOS_MODULE_UAC_H__

#include "amigos_module_base.h"
#include "amigos_surface_uac.h"
#include "ss_packet.h"
#include "ss_uac.h"

class AmigosModuleUac: public AmigosSurfaceUac, public AmigosModuleBase
{
    public:
        explicit AmigosModuleUac(const std::string &strInSection);
        ~AmigosModuleUac() override;
        unsigned int GetModId() const override;
        unsigned int GetInputType(unsigned int port) const override;
        unsigned int GetOutputType(unsigned int port) const override;
    protected:
        inline bool _IsPostReader(unsigned int) override
        {
            return true;
        }
    private:
        void ClearEnvDevPath();
        void SaveEnvDevPath();
        void _Init() override;
        void _Deinit() override;
        void _StartOut(unsigned int outPortId) override;
        void _StopOut(unsigned int outPortId) override;
        void _StartIn(unsigned int inPortId) override;
        void _StopIn(unsigned int inPortId) override;

        int _Connected(unsigned int outPortId, unsigned int ref) override;
        int _Disconnected(unsigned int outPortId, unsigned int ref) override;

        int _InConnect(unsigned int inPortId) override;
        int _InDisconnect(unsigned int inPortId) override;

        static int AiInit(void *uac, unsigned int uintAiSampleRate, unsigned char chn)
        {
            return 0;
        }
        static int FillBuffer(void *uac, SS_UAC_Frame_t *stUacFrame);
        static int AiDeinit(void *uac)
        {
            return 0;
        }
        static int AiSetVolume(SS_UAC_Volume_t stVolume)
        {
            return 0;
        }
        static int AoInit(void *uac, unsigned int uintAoSampleRate, unsigned char chn)
        {
            return 0;
        }
        static int TakeBuffer(void *uac, SS_UAC_Frame_t *stUacFrame);
        static int AoDeinit(void *uac)
        {
            return 0;
        }
        SS_UAC_Handle_h handle;
        ss_linker_base *srcLinker;
        ss_linker_base *dstLinker;
        pthread_rwlock_t srcLinkerLock;
        pthread_rwlock_t dstLinkerLock;
        stream_packet_info info;
};
#endif

