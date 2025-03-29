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

#ifndef __AMIGOS_MODULE_UVC_H__
#define __AMIGOS_MODULE_UVC_H__

#include "amigos_module_base.h"
#include "amigos_surface_uvc.h"
#include "ss_packet.h"
#include "ss_uvc.h"

class AmigosModuleUvc: public AmigosSurfaceUvc, public AmigosModuleBase
{
    public:
        explicit AmigosModuleUvc(const std::string &strInSection);
        ~AmigosModuleUvc() override;
        unsigned int GetModId() const override;
        unsigned int GetInputType(unsigned int port) const override;
        unsigned int GetOutputType(unsigned int port) const override;
    protected:
        inline bool _IsPostReader(unsigned int) override
        {
            return true;
        }
        virtual bool _DoesCustomerConnectIn(unsigned int) const
        {
            return true;
        }
    private:
        void _Init() override;
        void _Deinit() override;
        void _Start() override;
        void _Stop() override;
        void SaveEnvDevPath();
        void ClearEnvDevPath();
        static int StartCapture(void *uvc,Stream_Params_t format);
        static int StopCapture(void *uvc);
        static int MmapFillBuffer(void *uvc,SS_UVC_BufInfo_t *bufInfo);
        static int UserPtrFillBuffer(void *uvc,SS_UVC_BufInfo_t *bufInfo);
        static int UserPtrFinishBuffer(void *uvc,SS_UVC_BufInfo_t *bufInfo);
        static void ForceIdr(void *uvc);
        static int UvcInit(void *uvc)
        {
            return 0;
        };
        static int UvcDeinit(void *uvc)
        {
            return 0;
        };
        ss_linker_base *srcLinker;
        int curInport;
        SS_UVC_Handle_h handle;
        int streamType;
        int width;
        int height;
};
#endif

