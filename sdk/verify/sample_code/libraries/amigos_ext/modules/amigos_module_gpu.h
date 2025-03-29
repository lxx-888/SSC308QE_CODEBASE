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

#ifndef __AMIGOS_MODULE_GPU_H__
#define __AMIGOS_MODULE_GPU_H__

#include "amigos_module_base.h"
#include "amigos_surface_gpu.h"
#include <pthread.h>
#include <queue>
#include "gles_ldc.h"

class AmigosModuleGpu: public AmigosSurfaceGpu<AmigosModuleBase>
{
    public:
        explicit AmigosModuleGpu(const std::string &strInSection);
        ~AmigosModuleGpu() override;
    private:
        void _Init() override;
        void _Deinit() override;
        void BindBlock(const stModInputInfo_t & stIn);
        void UnBindBlock(const stModInputInfo_t & stIn);
        virtual int CreateSender(unsigned int outPortId);
        virtual int DestroySender(unsigned int outPortId);
        static void *Do_Gpu_DeInit(struct ss_thread_buffer *buf, struct ss_thread_user_data *data);
        static void DataReceiver(void *pData, unsigned int dataSize, void *pUsrData,  unsigned char portId, DeliveryCopyFp fpCopy);
        static void *SenderMonitor(struct ss_thread_buffer *pstBuf);
        static int GpuInit(AmigosModuleGpu *Sender);
        GLES_LDC_MODULE* Module = NULL;
        std::queue<GLES_DMABUF_INFO*> DmaBufQueue;
};

#endif //__AMIGOS_MODULE_GPU_H__

