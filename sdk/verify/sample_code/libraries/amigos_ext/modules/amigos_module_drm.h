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
#ifndef __AMIGOS_MODULE_DRM_H__
#define __AMIGOS_MODULE_DRM_H__

#include "amigos_module_base.h"
#include "amigos_surface_drm.h"
#include <queue>

class AmigosModuleDrm: public AmigosSurfaceDrm<AmigosModuleBase>
{
    public:
        explicit AmigosModuleDrm(const std::string &strInSection);
        ~AmigosModuleDrm() override;
    private:
        void _Init() override;
        void _Deinit() override;
        virtual int CreateSender(unsigned int outPortId);
        virtual int DestroySender(unsigned int outPortId);
        static void *SenderMonitor(struct ss_thread_buffer *pstBuf);
        void BindBlock(const stModInputInfo_t & stIn);
        void UnBindBlock(const stModInputInfo_t & stIn);
        static void DataReceiver(void *pData, unsigned int dataSize, void *pUsrData,  unsigned char portId, DeliveryCopyFp fpCopy);
};
#endif
