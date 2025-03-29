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
#include <sys/mman.h>
#include <unistd.h>
#include <vector>
#include <string>

#include "amigos_module_gpu.h"
#include "mi_sys.h"
#include <fcntl.h>
#include <sys/ioctl.h>
#include "amigos_module_base.h"


AmigosModuleGpu::AmigosModuleGpu(const std::string &strInSection) : AmigosSurfaceGpu(strInSection)
{
}
AmigosModuleGpu::~AmigosModuleGpu()
{
}

void AmigosModuleGpu::_Init()
{
    stModOutputInfo_t stOutInfo;

    GetOutputPortInfo(0, stOutInfo);
    stOutInfo.width = stOutRes.u32ResWidth;
    stOutInfo.height = stOutRes.u32ResHeight;
    stOutInfo.fmt = E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420;
    UpdateOutputPortInfo(0, stOutInfo);
}

void AmigosModuleGpu::_Deinit()
{
}

void AmigosModuleGpu::BindBlock(const stModInputInfo_t &stIn)
{
    stModDesc_t stPreDesc = {0};
    MI_SYS_ChnPort_t stChnOutputPort;
    AmigosModuleBase *pPrevClass = dynamic_cast<AmigosModuleBase *>(stIn.stPrev.pClass);
    assert(pPrevClass);
    pPrevClass->GetModDesc(stPreDesc);
    AMIGOS_INFO("Bind: name %s modid %d chn %d dev %d port %d fps %d\n", stModDesc.modName.c_str(), stModDesc.modId, stModDesc.chnId, stModDesc.devId, stIn.curPortId, stIn.curFrmRate);
    AMIGOS_INFO("Pre : name %s modid %d chn %d dev %d port %d fps %d\n", stPreDesc.modName.c_str(), stPreDesc.modId, stPreDesc.chnId, stPreDesc.devId, stIn.stPrev.portId, stIn.stPrev.frmRate);

    //_Init Prev Module's DmaBufAllocator
    stChnOutputPort.eModId = (MI_ModuleId_e)stPreDesc.modId;
    stChnOutputPort.u32DevId = (MI_U32)stPreDesc.devId;
    stChnOutputPort.u32ChnId = (MI_U32)stPreDesc.chnId;
    stChnOutputPort.u32PortId = (MI_U32)stIn.stPrev.portId;
    AMIGOS_INFO("Create DMA buf Allocator for %s\n", stPreDesc.modName.c_str());
    MI_SYS_SetChnOutputPortDepth(0, &stChnOutputPort, 0, 4);
    MI_SYS_CreateChnOutputPortDmabufCusAllocator(&stChnOutputPort);
    AllocDmabufQueue(stChnOutputPort.u32PortId, pPrevClass, OUTPUT_DAMBUF_QUEUE);

    CreateReceiver(stIn.curPortId, DataReceiver, this);
    StartReceiver(stIn.curPortId);
}

void AmigosModuleGpu::UnBindBlock(const stModInputInfo_t &stIn)
{
    stModDesc_t stPreDesc = {0};
    MI_SYS_ChnPort_t stChnOutputPort;
    AmigosModuleBase *pPrevClass = dynamic_cast<AmigosModuleBase *>(stIn.stPrev.pClass);
    assert(pPrevClass);
    pPrevClass->GetModDesc(stPreDesc);

    StopReceiver(stIn.curPortId);
    DestroyReceiver(stIn.curPortId);

    AMIGOS_INFO("UnBind: name %s modid %d chn %d dev %d port %d fps %d\n", stModDesc.modName.c_str(), stModDesc.modId, stModDesc.chnId, stModDesc.devId, stIn.curPortId, stIn.curFrmRate);
    AMIGOS_INFO("Pre   : name %s modid %d chn %d dev %d port %d fps %d\n", stPreDesc.modName.c_str(), stPreDesc.modId, stPreDesc.chnId, stPreDesc.devId, stIn.stPrev.portId, stIn.stPrev.frmRate);

    stChnOutputPort.eModId = (MI_ModuleId_e)stPreDesc.modId;
    stChnOutputPort.u32DevId = (MI_U32)stPreDesc.devId;
    stChnOutputPort.u32ChnId = (MI_U32)stPreDesc.chnId;
    stChnOutputPort.u32PortId = (MI_U32)stIn.stPrev.portId;
    MI_SYS_DestroyChnOutputPortDmabufCusAllocator(&stChnOutputPort);
}

//deinit must be process in SenderMonitor thread
void *AmigosModuleGpu::Do_Gpu_DeInit(struct ss_thread_buffer *pstBuf, struct ss_thread_user_data *data)
{
    stReceiverDesc_t *pReceiver = (stReceiverDesc_t *)pstBuf->buf;
    AmigosModuleGpu *pSendClass = dynamic_cast<AmigosModuleGpu *>(pReceiver->pBaseClass);
    gles_ldc_deinit(pSendClass->Module);
    free(pSendClass->Module);

    return NULL;
}

int AmigosModuleGpu::CreateSender(unsigned int outPortId)
{
    struct ss_thread_attr stSsThreadAttr = {0};
    memset(&stSsThreadAttr, 0, sizeof(struct ss_thread_attr));
    stSsThreadAttr.do_signal = Do_Gpu_DeInit;
    stSsThreadAttr.do_monitor = SenderMonitor;
    stSsThreadAttr.in_buf.buf = (void *)&(mapRecevier[outPortId]);
    stSsThreadAttr.in_buf.size = 0;

    ss_thread_open(GetOutPortIdentifyStr(outPortId).c_str(), &stSsThreadAttr);

    return 0;
}

int AmigosModuleGpu::DestroySender(unsigned int outPortId)
{
    ss_thread_send(GetOutPortIdentifyStr(outPortId).c_str(), NULL);
    ss_thread_close(GetOutPortIdentifyStr(outPortId).c_str());
    return 0;
}

int AmigosModuleGpu::GpuInit(AmigosModuleGpu *Sender)
{
    MAP_INFO info = {0};
    unsigned int mapSize = 0;
    float *mapX = NULL, *mapY = NULL;
    FILE *px = NULL, *py = NULL;

    memset(&info, 0x00, sizeof(MAP_INFO));
    info.inResolutionW = Sender->stInRes.u32ResWidth;
    info.inResolutionH = Sender->stInRes.u32ResHeight;
    info.outResolutionW = Sender->stOutRes.u32ResWidth;
    info.outResolutionH = Sender->stOutRes.u32ResHeight;
    info.mapGridX = Sender->stBinInfo.u32MapGridX;
    info.mapGridY = Sender->stBinInfo.u32MapGridY;
    info.gridSize = Sender->stBinInfo.u32GridSize;

    mapSize = info.mapGridX * info.mapGridY;

    Sender->Module = (GLES_LDC_MODULE *)malloc(sizeof(GLES_LDC_MODULE));
    memset(Sender->Module, 0, sizeof(GLES_LDC_MODULE));

    mapX = (float*)malloc(sizeof(float) * mapSize);
    mapY = (float*)malloc(sizeof(float) * mapSize);

    px = fopen(Sender->stBinInfo.Map_X_Path, "r+");
    py = fopen(Sender->stBinInfo.Map_Y_Path, "r+");

    fread(mapX, sizeof(float), mapSize, px);
    fread(mapY, sizeof(float), mapSize, py);

    fclose(px);
    fclose(py);
    AMIGOS_INFO("info = {%d,%d,%d,%d,%d,%d,%d}\n",info.inResolutionW,info.inResolutionH,info.outResolutionW,info.outResolutionH,info.mapGridX,info.mapGridY,info.gridSize);
    if(gles_ldc_init((GLES_LDC_MODULE *)Sender->Module, mapX, mapY, &info, 3) != 0)
    {
        AMIGOS_ERR("Gpu _Init fail");
    }

    free(mapX);
    free(mapY);
    return 0;
}

void * AmigosModuleGpu::SenderMonitor(struct ss_thread_buffer *pstBuf)
{
    stStreamData_t stStreamData;
    GLES_DMABUF_INFO* renderInfo = NULL;
    AmigosDmaBuf *pInputBuf = NULL;
    stReceiverDesc_t *pReceiver = (stReceiverDesc_t *)pstBuf->buf;
    AmigosModuleGpu *pSendClass = dynamic_cast<AmigosModuleGpu *>(pReceiver->pBaseClass);

    if(!pSendClass->Module)
    {
        //note:do gpu init/deinit and gpu get buf in one thread
        GpuInit(pSendClass);
    }

    pInputBuf = pSendClass->DmaBufQueuePop(INPUT_DAMBUF_QUEUE);
    if(pInputBuf == NULL)
    {
        usleep(1000);
        return NULL;
    }
    //gles_ldc_input_texture_buffer((GLES_LDC_MODULE *)pSendClass->Module, &pInputBuf->buf);
    renderInfo = gles_ldc_get_render_buffer((GLES_LDC_MODULE *)pSendClass->Module); // the consistent tid to wait render
    if (renderInfo == NULL)
    {
        AMIGOS_ERR("gles_ldc_get_render_buffer: get render buffer failed\n");
        return NULL;
    }

    stStreamData.stInfo.eStreamType = E_STREAM_VIDEO_DMABUF_DATA;
    memset(&stStreamData.stDmaBufInfo, 0, sizeof(MI_SYS_DmaBufInfo_t));
    stStreamData.stDmaBufInfo.s32Fd[0]          = renderInfo->fds[0];
    stStreamData.stDmaBufInfo.s32Fd[1]          = renderInfo->fds[1];
    stStreamData.stDmaBufInfo.u16Width          = renderInfo->width;
    stStreamData.stDmaBufInfo.u16Height         = renderInfo->height;
    stStreamData.stDmaBufInfo.u32Stride[0]      = renderInfo->stride[0];
    stStreamData.stDmaBufInfo.u32Stride[1]      = renderInfo->stride[1];
    stStreamData.stDmaBufInfo.bEndOfStream      = FALSE;
    stStreamData.stDmaBufInfo.u32SequenceNumber = 1;
    stStreamData.stDmaBufInfo.eFormat           = E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420;
    MI_SYS_GetCurPts(0, &stStreamData.stDmaBufInfo.u64Pts);

    pSendClass->Send(pReceiver->uintPort, &stStreamData, sizeof(stStreamData_t), NULL);

    gles_ldc_put_render_buffer((GLES_LDC_MODULE *)pSendClass->Module, renderInfo);
    AmigosModuleBase *srcClass = dynamic_cast<AmigosModuleBase*>(pInputBuf->pclass);
    srcClass->DmaBufQueuePush(pInputBuf, OUTPUT_DAMBUF_QUEUE);
    return NULL;
}

void AmigosModuleGpu::DataReceiver(void *pData, unsigned int dataSize, void *pUsrData, unsigned char portId, DeliveryCopyFp fpCopy)
{
    AmigosDmaBuf *buf = (AmigosDmaBuf*)pData;
    GLES_DMABUF_INFO *texurebuf = (GLES_DMABUF_INFO *)buf;
    AmigosModuleGpu *GpuClass = (AmigosModuleGpu *)pUsrData;
    GpuClass->DmaBufQueuePush(buf, INPUT_DAMBUF_QUEUE);
    gles_ldc_input_texture_buffer((GLES_LDC_MODULE *)GpuClass->Module, texurebuf);
}

AMIGOS_MODULE_INIT("GPU", AmigosModuleGpu);
