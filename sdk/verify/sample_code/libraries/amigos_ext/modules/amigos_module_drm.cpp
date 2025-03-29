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
#include <fcntl.h>
#include <vector>
#include <unistd.h>
#include <string>
#include <map>

#include "xf86drmMode.h"
#include "xf86drm.h"
#include "drm_fourcc.h"

#include "amigos_module_drm.h"

#define ALIGN_BACK(x, a)            (((x) / (a)) * (a))

enum DrmPlaneId
{
    GOP_UI_ID = 31,
    GOP_CURSOR_ID = 40,
    MOPG_ID0 = 47,
    MOPG_ID1 = 53,
    MOPG_ID2 = 59,
    MOPG_ID3 = 65,
    MOPG_ID4 = 71,
    MOPG_ID5 = 77,
    MOPG_ID6 = 83,
    MOPG_ID7 = 89,
    MOPG_ID8 = 95,
    MOPG_ID9 = 101,
    MOPG_ID10 = 107,
    MOPG_ID11 = 113,
    MOPS_ID = 119,
};

AmigosModuleDrm::AmigosModuleDrm(const std::string &strInSection) : AmigosSurfaceDrm(strInSection)
{
}
AmigosModuleDrm::~AmigosModuleDrm()
{
}

typedef struct drm_mode_property_ids {
    uint32_t CRTC_ID;
    uint32_t FENCE_ID;
    uint32_t ACTIVE;
    uint32_t MODE_ID;
} drm_mode_property_ids_t;

typedef struct DrmModeConfiction_s
{
    int fd;
    int out_fence;
    unsigned int crtc_id;
    unsigned int conn_id;
    unsigned int width;
    unsigned int height;
    unsigned int blob_id;
    drm_mode_property_ids_t Prop_ids;
    drmModeRes *pDrmModeRes;
    drmModeConnectorPtr Connector;
}DrmModeConfiction_t;
static DrmModeConfiction_t stDrmCfg;

static std::map<AmigosDmaBuf*, unsigned int> g_DmaBufMaps;

static int get_property_id(int fd, drmModeObjectProperties* props, const char* name) {
    int id = 0;
    drmModePropertyPtr property;
    int found = 0;

    for (unsigned int i = 0; !found && i < props->count_props; ++i) {
        property = drmModeGetProperty(fd, props->props[i]);
        if (!strcmp(property->name, name)) {
            id = property->prop_id;
            found = 1;
        }
        drmModeFreeProperty(property);
    }

    return id;
}

static inline int sync_wait(int fd, int timeout)
{
    struct timeval time;
    fd_set fs_read;
    int ret;
    time.tv_sec = timeout / 1000;
    time.tv_usec = timeout % 1000 * 1000;

    FD_ZERO(&fs_read);
    FD_SET(fd, &fs_read);

    ret = select(fd + 1, &fs_read, NULL, NULL, &time);
    if(ret <= 0)
    {
        return -1;
    }
    else
    {
        return 0;
    }
}

unsigned int AddDmabufToFB(AmigosDmaBuf* dmabuf)
{
    int ret = -1;
    unsigned int pitches[4] = {0}, offsets[4] = {0}, gem_handles[4] = {0};
    unsigned int width = 0, height = 0, fb_id = 0, drm_fmt = 0;

    width = ALIGN_BACK(std::min(dmabuf->width, stDrmCfg.width), 4);
    height = ALIGN_BACK(std::min(dmabuf->height, stDrmCfg.height), 4);

    //TODO only support nv12 now
    pitches[0] = ALIGN_BACK(width, 4);
    pitches[1] = ALIGN_BACK(width, 4);
    offsets[0] = 0;
    offsets[1] = pitches[0] * height;
    drm_fmt = DRM_FORMAT_NV12;

    ret = drmPrimeFDToHandle(stDrmCfg.fd, dmabuf->fds[0], &gem_handles[0]);
    if (ret) {
        AMIGOS_ERR("drmPrimeFDToHandle failed, ret=%d dma_buffer_fd=%d\n", ret, dmabuf->fds[0]);
        return -1;
    }
    gem_handles[1] = gem_handles[0];

    ret = drmModeAddFB2(stDrmCfg.fd, width, height, drm_fmt, gem_handles, pitches, offsets, &fb_id, 0);
    AMIGOS_INFO("add fb:\n w=%d,h=%d,fmt=%d\n", width, height, drm_fmt);
    for(int i = 0;i < 4; i++)
    {
        AMIGOS_INFO("%d : handle = %d  pitches = %d  offset = %d\n", i, gem_handles[i], pitches[i], offsets[i]);
    }

    if (ret)
    {
        AMIGOS_ERR("drmModeAddFB2 failed, ret=%d fb_id=%d\n", ret, fb_id);
        return -1;
    }
    return fb_id;
}


static int add_plane_property(drmModeAtomicReqPtr req, int plane_id, const char *name, int val)
{
    drmModeObjectProperties* pModeProps;
    int prop_id;

    pModeProps = drmModeObjectGetProperties(stDrmCfg.fd, plane_id, DRM_MODE_OBJECT_PLANE);
    if (!pModeProps) {
        AMIGOS_ERR("Get properties error,plane_id=%d \n",plane_id);
        return  -1;
    }
    prop_id = get_property_id(stDrmCfg.fd, pModeProps, name);
    drmModeAtomicAddProperty(req, plane_id, prop_id, val);
    //AMIGOS_INFO("add plane_id = %d property %s %d = %d\n", plane_id, name, prop_id, val);
}

static int atomic_set_plane(int plane_id, int fb_id, int is_realtime) {
    int ret;
    drmModeAtomicReqPtr req;

    req = drmModeAtomicAlloc();
    if (!req) {
        AMIGOS_ERR("drmModeAtomicAlloc failed \n");
        return -1;
    }

    add_plane_property(req, plane_id, "FB_ID", fb_id);
    add_plane_property(req, plane_id, "CRTC_ID", stDrmCfg.crtc_id);
    add_plane_property(req, plane_id, "CRTC_X", 0);
    add_plane_property(req, plane_id, "CRTC_Y", 0);
    add_plane_property(req, plane_id, "CRTC_W", stDrmCfg.width);
    add_plane_property(req, plane_id, "CRTC_H", stDrmCfg.height);
    add_plane_property(req, plane_id, "SRC_X", 0);
    add_plane_property(req, plane_id, "SRC_Y", 0);
    add_plane_property(req, plane_id, "SRC_W", stDrmCfg.width << 16);
    add_plane_property(req, plane_id, "SRC_H", stDrmCfg.height << 16);
    if(is_realtime)
    {
        add_plane_property(req, plane_id, "realtime_mode", 1);
    }

    drmModeAtomicAddProperty(req, stDrmCfg.crtc_id, stDrmCfg.Prop_ids.FENCE_ID, (uint64_t)&stDrmCfg.out_fence);//use this flag,must be close out_fence

    ret = drmModeAtomicCommit(stDrmCfg.fd, req, DRM_MODE_ATOMIC_ALLOW_MODESET, NULL);
    if(ret != 0)
    {
        AMIGOS_ERR("drmModeAtomicCommit failed ret=%d \n",ret);
    }
    drmModeAtomicFree(req);

    ret = sync_wait(stDrmCfg.out_fence, 16);
    if(ret != 0)
    {
        AMIGOS_ERR("waring:maybe drop one drm frame, ret=%d out_fence=%d\n", ret, stDrmCfg.out_fence);
    }
    close(stDrmCfg.out_fence);
    return 0;
}

void AmigosModuleDrm::_Init()
{
    int ret;
    drmModePlaneRes *pDrmPlaneRes;
    drmModeObjectProperties* pModeProps;
    drmModeAtomicReqPtr req;

    stDrmCfg.fd = open("/dev/dri/card0", O_RDWR | O_CLOEXEC);
    if(stDrmCfg.fd < 0)
    {
        AMIGOS_ERR("Open dri/card0 fail \n");
        return ;
    }
    
    ret = drmSetClientCap(stDrmCfg.fd, DRM_CLIENT_CAP_ATOMIC, 1);
    if(ret != 0)
    {
        AMIGOS_ERR("drmSetClientCap DRM_CLIENT_CAP_ATOMIC ret=%d  \n",ret);
    }
    drmSetClientCap(stDrmCfg.fd, DRM_CLIENT_CAP_UNIVERSAL_PLANES, 1);

    stDrmCfg.pDrmModeRes = drmModeGetResources(stDrmCfg.fd);
    stDrmCfg.crtc_id = stDrmCfg.pDrmModeRes->crtcs[0];
    stDrmCfg.conn_id = stDrmCfg.pDrmModeRes->connectors[0];

    stDrmCfg.Connector = drmModeGetConnector(stDrmCfg.fd, stDrmCfg.conn_id);
    stDrmCfg.width = stDrmCfg.Connector->modes[0].hdisplay;
    stDrmCfg.height = stDrmCfg.Connector->modes[0].vdisplay;

    //get crtc property
    pModeProps = drmModeObjectGetProperties(stDrmCfg.fd, stDrmCfg.crtc_id, DRM_MODE_OBJECT_CRTC);
    if (!pModeProps) {
        AMIGOS_ERR("Get properties error,crtc_id=%d \n",stDrmCfg.crtc_id);
        return  ;
    }
    stDrmCfg.Prop_ids.FENCE_ID = get_property_id(stDrmCfg.fd, pModeProps, "OUT_FENCE_PTR");
    stDrmCfg.Prop_ids.ACTIVE = get_property_id(stDrmCfg.fd, pModeProps, "ACTIVE");
    stDrmCfg.Prop_ids.MODE_ID = get_property_id(stDrmCfg.fd, pModeProps, "MODE_ID");
    drmModeFreeObjectProperties(pModeProps);

    //get CRTC_ID
    pModeProps = drmModeObjectGetProperties(stDrmCfg.fd, stDrmCfg.conn_id, DRM_MODE_OBJECT_CONNECTOR);
    stDrmCfg.Prop_ids.CRTC_ID = get_property_id(stDrmCfg.fd, pModeProps, "CRTC_ID");
    drmModeFreeObjectProperties(pModeProps);

    //create property olob
    drmModeCreatePropertyBlob(stDrmCfg.fd, &stDrmCfg.Connector->modes[0],sizeof(stDrmCfg.Connector->modes[0]), &stDrmCfg.blob_id);

    //set crtc property
    req = drmModeAtomicAlloc();
    if (!req) {
        AMIGOS_ERR("drmModeAtomicAlloc failed \n");
        return  ;
    }
    drmModeAtomicAddProperty(req, stDrmCfg.crtc_id, stDrmCfg.Prop_ids.ACTIVE, 1);
    drmModeAtomicAddProperty(req, stDrmCfg.crtc_id, stDrmCfg.Prop_ids.MODE_ID, stDrmCfg.blob_id);
    drmModeAtomicAddProperty(req, stDrmCfg.conn_id, 20, stDrmCfg.crtc_id);

    ret = drmModeAtomicCommit(stDrmCfg.fd, req, DRM_MODE_ATOMIC_ALLOW_MODESET, NULL);
    if(ret != 0)
    {
        AMIGOS_ERR("drmModeAtomicCommit failed ret=%d \n",ret);
        return  ;
    }
    drmModeAtomicFree(req);
}

void AmigosModuleDrm::_Deinit()
{
    drmModeFreeConnector(stDrmCfg.Connector);
    drmModeFreeResources(stDrmCfg.pDrmModeRes);
    close(stDrmCfg.fd);
}

void AmigosModuleDrm::BindBlock(const stModInputInfo_t & stIn)
{
    stModDesc_t stPreDesc = {0};
    MI_SYS_ChnPort_t stChnOutputPort;
    AmigosModuleBase *pPrevClass = dynamic_cast<AmigosModuleBase *>(stIn.stPrev.pClass);
    assert(pPrevClass);
    pPrevClass->GetModDesc(stPreDesc);
    AMIGOS_INFO("Bind: name %s modid %d chn %d dev %d port %d fps %d\n", stModDesc.modName.c_str(), stModDesc.modId, stModDesc.chnId, stModDesc.devId, stIn.curPortId, stIn.curFrmRate);
    AMIGOS_INFO("Pre : name %s modid %d chn %d dev %d port %d fps %d\n", stPreDesc.modName.c_str(), stPreDesc.modId, stPreDesc.chnId, stPreDesc.devId, stIn.stPrev.portId, stIn.stPrev.frmRate);
    
    if(stIn.bindType == E_MI_SYS_BIND_TYPE_REALTIME)
    {
        //drm need only a buf when realtime bind hvp
        AmigosDmaBuf *pOutputBuf = _alloc_dmabuf(stDmaBufCfg.DmaBufWidth, stDmaBufCfg.DmaBufHeight, (E_VIDEO_RAW_FORMAT)stDmaBufCfg.DmaBufFmt);
        if(!pOutputBuf)
        {
            return;
        }
        atomic_set_plane(MOPS_ID, AddDmabufToFB(pOutputBuf), 1);
    }
    else
    {
        //_Init Prev Module's DmaBufAllocator
        stChnOutputPort.eModId = (MI_ModuleId_e)stPreDesc.modId;
        stChnOutputPort.u32DevId = (MI_U32)stPreDesc.devId;
        stChnOutputPort.u32ChnId = (MI_U32)stPreDesc.chnId;
        stChnOutputPort.u32PortId = (MI_U32)stIn.stPrev.portId;
        AMIGOS_INFO("Create DMA buf Allocator for %s\n", stPreDesc.modName.c_str());
        MI_SYS_SetChnOutputPortDepth(0, &stChnOutputPort, 0, 4);
        MI_SYS_CreateChnOutputPortDmabufCusAllocator(&stChnOutputPort);
        //TODO move to incoming
        for(int i = 0; i < 3; i++)
        {
            AmigosDmaBuf *pOutputBuf = _alloc_dmabuf(stDmaBufCfg.DmaBufWidth, stDmaBufCfg.DmaBufHeight, (E_VIDEO_RAW_FORMAT)stDmaBufCfg.DmaBufFmt);
            if(pOutputBuf == NULL)
            {
                return ;
            }
            pOutputBuf->pclass = (AmigosBase*)pPrevClass;
            g_DmaBufMaps[pOutputBuf] = AddDmabufToFB(pOutputBuf);
            pPrevClass->DmaBufQueuePush(pOutputBuf, OUTPUT_DAMBUF_QUEUE);
        }
        CreateReceiver(stIn.curPortId, DataReceiver, this);
        StartReceiver(stIn.curPortId);

        //create SenderMonitor to update drm fb
        struct ss_thread_attr stSsThreadAttr = {0};
        memset(&stSsThreadAttr, 0, sizeof(struct ss_thread_attr));
        stSsThreadAttr.do_signal = NULL;
        stSsThreadAttr.do_monitor = SenderMonitor;
        stSsThreadAttr.in_buf.buf = this;
        stSsThreadAttr.in_buf.size = 0;
        ss_thread_open(GetOutPortIdentifyStr(0).c_str(), &stSsThreadAttr);
        ss_thread_start_monitor(GetOutPortIdentifyStr(0).c_str());
    }

}

void AmigosModuleDrm::UnBindBlock(const stModInputInfo_t & stIn)
{
    stModDesc_t stPreDesc = {0};
    MI_SYS_ChnPort_t stChnOutputPort;
    AmigosModuleBase *pPrevClass = dynamic_cast<AmigosModuleBase *>(stIn.stPrev.pClass);
    assert(pPrevClass);
    pPrevClass->GetModDesc(stPreDesc);

    AMIGOS_INFO("UnBind: name %s modid %d chn %d dev %d port %d fps %d\n", stModDesc.modName.c_str(), stModDesc.modId, stModDesc.chnId, stModDesc.devId, stIn.curPortId, stIn.curFrmRate);
    AMIGOS_INFO("Pre   : name %s modid %d chn %d dev %d port %d fps %d\n", stPreDesc.modName.c_str(), stPreDesc.modId, stPreDesc.chnId, stPreDesc.devId, stIn.stPrev.portId, stIn.stPrev.frmRate);

    if(stIn.bindType == E_MI_SYS_BIND_TYPE_REALTIME)
    {

    }
    else
    {
        StopReceiver(stIn.curPortId);
        DestroyReceiver(stIn.curPortId);

        stChnOutputPort.eModId = (MI_ModuleId_e)stPreDesc.modId;
        stChnOutputPort.u32DevId = (MI_U32)stPreDesc.devId;
        stChnOutputPort.u32ChnId = (MI_U32)stPreDesc.chnId;
        stChnOutputPort.u32PortId = (MI_U32)stIn.stPrev.portId;
        MI_SYS_DestroyChnOutputPortDmabufCusAllocator(&stChnOutputPort);

        //close SenderMonitor
        ss_thread_close(GetOutPortIdentifyStr(0).c_str());
    }
    

}

int AmigosModuleDrm::DestroySender(unsigned int outPortId)
{
    return 0;
}
int AmigosModuleDrm::CreateSender(unsigned int outPortId)
{
    return 0;
}

void * AmigosModuleDrm::SenderMonitor(struct ss_thread_buffer *pstBuf)
{
    AmigosDmaBuf *pInputBuf = NULL;
    unsigned int fb_id = 0;
    int ret;

    AmigosModuleDrm *pSendClass = (AmigosModuleDrm*)pstBuf->buf;
    pInputBuf = pSendClass->DmaBufQueuePop(INPUT_DAMBUF_QUEUE);
    if(pInputBuf == NULL)
    {
        usleep(10*1000);
        return NULL;
    }

    std::map<AmigosDmaBuf*, unsigned int>::iterator it = g_DmaBufMaps.find(pInputBuf);
    fb_id = it->second;

    ret = atomic_set_plane(MOPG_ID0,fb_id, 0);
    if (ret) {
        AMIGOS_ERR("AtomicCommit failed, ret=%d out_fence=%d\n", ret, stDrmCfg.out_fence);
    }
    AmigosModuleBase *srcClass = dynamic_cast<AmigosModuleBase*>(pInputBuf->pclass);
    srcClass->DmaBufQueuePush(pInputBuf, OUTPUT_DAMBUF_QUEUE);
    return NULL;
}



void AmigosModuleDrm::DataReceiver(void *pData, unsigned int dataSize, void *pUsrData,  unsigned char portId, DeliveryCopyFp fpCopy)
{
    AmigosDmaBuf *buf = (AmigosDmaBuf*)pData;
    AmigosModuleDrm *DrmClass = (AmigosModuleDrm *)pUsrData;
    DrmClass->DmaBufQueuePush(buf, INPUT_DAMBUF_QUEUE);
}

AMIGOS_MODULE_INIT("DRM", AmigosModuleDrm);
