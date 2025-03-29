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

#ifndef __AMIGOS_MODULE_VENC_H__
#define __AMIGOS_MODULE_VENC_H__

#include <string>
#include "mi_venc.h"
#include "mi_venc_datatype.h"
#include "amigos_module_mi_base.h"
#include "amigos_surface_venc.h"

class AmigosModuleVenc: public AmigosSurfaceVenc, public AmigosModuleMiBase
{
        friend void *VencReader(struct ss_thread_buffer *thread_buf);
    private:
        struct VencReaderDesc
        {
            LinkerGroup      *linker;
            void             *threadHandle;
            AmigosModuleVenc *pModule;
            stream_packet_obj packet;
            VencReaderDesc() : linker(nullptr), threadHandle(nullptr), pModule(nullptr), packet(nullptr)
            {}
        };
    public:
        explicit AmigosModuleVenc(const std::string &strSection);
        ~AmigosModuleVenc() override;

    protected:
        void _Init() override;
        void _Deinit() override;
        void _ResourceInit() override;
        void _ResourceDeinit() override;

        void _StartOut(unsigned int outPortId) override;
        void _StopOut(unsigned int outPortId) override;

        int _Connected(unsigned int outPortId, unsigned int ref) override;
        int _Disconnected(unsigned int outPortId, unsigned int ref) override;

        bool _NeedResetStreamTraverse(unsigned int outPortId, unsigned int bindType,
                                      unsigned int &inPortId) override;
        void _ResetStreamOut(unsigned int outPortId, unsigned int width, unsigned int height) override;

        int _EnqueueOut(unsigned int outPortId, stream_packet_obj &packet) override;
        int _DequeueOut(unsigned int outPortId, stream_packet_obj &packet) override;
        stream_packet_info _GetStreamInfo(unsigned int outPortId) override;
        stream_packet_obj _DequeueFromInside(unsigned int outPortId, unsigned int ms) override;
#ifdef CONFIG_MI_SYSCALL_L2R_DESTROY_SUPPORT
        bool _NeedMarkDeinitOnRtos() override;
        bool _NeedMarkUnbindOnRtos(unsigned int inPortId) override;
        bool _NeedMarkStopOutOnRtos(unsigned int outPortId) override;
#endif
        stream_packet_obj _OutPacket(unsigned int ms);
    public:
        unsigned int GetModId() const override;
        unsigned int GetInputType(unsigned int port) const override;
        unsigned int GetOutputType(unsigned int port) const override;
    public:
        void SetRefParam();
        void SetDeBreathParam();
        void SetRoiParam(int lable);

private:
        std::map<unsigned int, VencReaderDesc> mapReaderDesc;
};
#endif
