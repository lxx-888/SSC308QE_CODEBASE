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
#include "ptree_uvc.h"
#include "usb_class_ac_vc.h"
#include "composite.h"
#include "ssos_task.h"
#include "ssos_mem.h"
#include "ptree_log.h"
#include "ssos_time.h"
#include "mi_sys.h"
#include "ssos_thread.h"
#include "uvc_video.h"

static ST_UVC_ARGS         *g_uvcAppPara[MAX_VS_IF];
static int                  g_uvcNum = 0;
static SSOS_THREAD_Mutex_t g_dataMutex;

static struct uvc_user_ops uvc_app_ops = {
    .process_vc_req  = uvc_video_process_vc_req,
    .process_vs_req  = uvc_video_process_vs_req,
    .process_vc_data = uvc_app_process_vc_data,
    .process_vs_data = uvc_video_process_vs_data,

    .stop_stream  = uvc_app_stop_capture,
    .start_stream = uvc_app_start_capture,
    .set_config   = uvc_app_set_config,
    .suspend      = uvc_app_suspend,
    .resume       = uvc_app_resume,
    .reset        = uvc_app_usb_reset,

    .clear_format = uvc_video_clear_format,
    .add_format   = uvc_video_add_format,
    .xfer_cfg     = uvc_video_xfer_cfg,

    .init_req_param   = uvc_video_init_req_param,
    .deinit_req_param = uvc_video_deinit_req_param,
    .speed_negotiate  = uvc_app_speed_negotiate,
};

static signed char _PTREE_UVC_AppMsgEnqueue(CamOsMsgQueue uvc_app_mq, enum msg_type type, unsigned int timeout,
                                            unsigned char blocking_call, unsigned int blocking_timeout)
{
    struct uvc_app_msg *pmsg;
    CamOsTsem_t         blocking_call_sem = {0};

    pmsg = SSOS_MEM_Alloc(sizeof(*pmsg));
    if (pmsg == NULL)
    {
        return -1;
    }
    CamOsMemset((void *)pmsg, 0x0, sizeof(*pmsg));
    pmsg->type = type;
    if (blocking_call)
    {
        if (SSOS_DEF_OK != CamOsTsemInit(&blocking_call_sem, 0))
        {
            SSOS_MEM_Free(pmsg);
            PTREE_ERR("uvc app sem fail! type:%d\n", type);
            return -1;
        }
        pmsg->pblocking_call_sem = &blocking_call_sem;
    }

    if (SSOS_DEF_OK != CamOsMsgQueueEnqueue(uvc_app_mq, pmsg, timeout))
    {
        SSOS_MEM_Free(pmsg);
        PTREE_ERR("uvc app mq enq fail! type:%d\n", type);
        return -1;
    }

    if (blocking_call)
    {
        int ret = SSOS_DEF_OK;
        do
        {
            ret = CamOsTsemTimedDown(&blocking_call_sem, blocking_timeout);
            if (ret == SSOS_DEF_ETIMEOUT)
            {
                PTREE_WRN("uvc app msg %d timeout!\n", type);
            }
        } while (ret == SSOS_DEF_ETIMEOUT);
        if (SSOS_DEF_OK != CamOsTsemDeinit(&blocking_call_sem))
        {
            return -1;
        }
    }

    return 0;
}

static unsigned int _PTREE_UVC_AppVideoBufferAllocate(ST_UVC_ARGS *puvc_para)
{
    signed int   ret             = -1;
    unsigned int bufSizeRequired = 0;
    unsigned int xfer_sz, hdr_sz;

    if (puvc_para == NULL)
    {
        PTREE_ERR("param is not ready\n");
        return SSOS_DEF_FAIL;
    }
    if (puvc_para->attr.stream_params.maxframesize == 0)
    {
        PTREE_ERR("param is not ready\n");
        return SSOS_DEF_FAIL;
    }

    xfer_sz         = sstar_usbd_uvc_bytes_per_intvl(puvc_para->uvc_idx);
    hdr_sz          = sizeof(struct uvc_payload_header);
    bufSizeRequired = ALIGN_UP(puvc_para->attr.stream_params.maxframesize
                                   + ((puvc_para->attr.stream_params.maxframesize / (xfer_sz - hdr_sz) + 1) * hdr_sz),
                               1024);

    switch (puvc_para->attr.stream_params.fcc)
    {
        case V4L2_PIX_FMT_YUYV:
            // NOP
            break;

        case V4L2_PIX_FMT_NV12:
            // NOP
            break;

        case V4L2_PIX_FMT_MJPG:
        case V4L2_PIX_FMT_H264:
        case V4L2_PIX_FMT_H265:
            /* memory alloc */
            ret = MI_SYS_MMA_Alloc(0, NULL, bufSizeRequired, &(puvc_para->uvc_frm_dma));
            if (ret != MI_SUCCESS)
            {
                PTREE_ERR("alloc mma failed\n");
                return SSOS_DEF_FAIL;
            }
            ret = MI_SYS_Mmap(puvc_para->uvc_frm_dma, bufSizeRequired, &(puvc_para->puvc_frm_buf), TRUE);
            if (ret != MI_SUCCESS)
            {
                MI_SYS_MMA_Free(0, puvc_para->uvc_frm_dma);
                PTREE_ERR("MI_SYS_Mmap failed !!!!\r\n");
            }
            break;

        default:
            break;
    }

    return bufSizeRequired;
}

static void _PTREE_UVC_AppVideoBufferFree(ST_UVC_ARGS *puvc_para, unsigned int bufSizeRequired)
{
    signed int ret = -1;

    switch (puvc_para->attr.stream_params.fcc)
    {
        case V4L2_PIX_FMT_YUYV:
            // NOP
            break;

        case V4L2_PIX_FMT_NV12:
            // NOP
            break;

        case V4L2_PIX_FMT_MJPG:
        case V4L2_PIX_FMT_H264:
        case V4L2_PIX_FMT_H265:
            ret = MI_SYS_Munmap(puvc_para->puvc_frm_buf, bufSizeRequired);
            if (ret != SSOS_DEF_OK)
            {
                PTREE_ERR("%s, failed\n", __func__);
                return;
            }
            ret = MI_SYS_MMA_Free(0, puvc_para->uvc_frm_dma);
            if (ret != SSOS_DEF_OK)
            {
                PTREE_ERR("%s, failed\n", __func__);
                return;
            }
            break;

        default:
            break;
    }
}

static void *PTREE_UVC_VideoProcessTask(SSOS_TASK_Buffer_t *data)
{
    int                          ret             = -1;
    unsigned int                 bufSizeRequired = 0;
    unsigned int                 size            = 0;
    struct uvc_app_msg          *pmsg            = NULL;
    char __attribute__((unused)) strFlag         = 0;
    CamOsTimespec_t              tTvStart = {0}, tTvEnd = {0};
    CamOsTsem_t                 *pblockingCallSemReceived = NULL;
    char                        *frameBuf                 = NULL;
    MI_PHY                       frameDma                 = 0;

    ST_UVC_ARGS *pArgs = (ST_UVC_ARGS *)data->buf;

    while (SSOS_DEF_OK != CamOsThreadShouldStop())
    {
        if (CamOsMsgQueueDequeue(pArgs->viMsgQueue, (void *)(&pmsg), pArgs->viMsgQueueTimeout) == SSOS_DEF_OK)
        {
            if (pmsg->type == UVC_SUSPEND)
            {
                PTREE_DBG("received msg %s\n", "UVC_SUSPEND");
                strFlag            = 1;
                pArgs->flag_resume = 0;
                CamOsGetMonotonicTime(&tTvStart);
                CamOsGetMonotonicTime(&tTvEnd);
            }
            else
            {
                PTREE_DBG("vid:%d %s => %s\n", pArgs->uvc_idx,
                          pArgs->uvc_status == UVC_STOP_CAPTURE ? "STOP_CAPTURE" : "START_CAPTURE",
                          pmsg->type == UVC_STOP_CAPTURE ? "STOP_CAPTURE" : "START_CAPTURE");

                pArgs->uvc_status = (unsigned char)pmsg->type;
            }

            pblockingCallSemReceived = pmsg->pblocking_call_sem;
            SSOS_MEM_Free(pmsg);
        }

        if (pArgs->uvc_status == UVC_START_CAPTURE)
        {
            strFlag = 0;

            if (pArgs->stream_init == 0)
            {
                PTREE_DBG(
                    "Webcam-UVC: start preview %d, "
                    "info(format:%c%c%c%c, "
                    "Width:%lu, High:%lu)\n",
                    (unsigned char)(pArgs->uvc_idx), (unsigned char)(pArgs->attr.stream_params.fcc),
                    (unsigned char)(pArgs->attr.stream_params.fcc >> 8),
                    (unsigned char)(pArgs->attr.stream_params.fcc >> 16),
                    (unsigned char)(pArgs->attr.stream_params.fcc >> 24), pArgs->attr.stream_params.width,
                    pArgs->attr.stream_params.height);
                if (!pArgs->ops.start_caputre)
                {
                    PTREE_ERR("start_caputre is not ready!\n");
                    return NULL;
                }
                ret = pArgs->ops.start_caputre(pArgs);
                if (ret != SSOS_DEF_OK)
                {
                    PTREE_ERR("Webcam-UVC: start failed \n");
                    continue;
                }

                pArgs->stream_init = 1;
                bufSizeRequired    = _PTREE_UVC_AppVideoBufferAllocate(pArgs);
            }

            if (!pArgs->ops.fill_buffer)
            {
                PTREE_ERR("fill buffer func is null\n");
                return NULL;
            }
            if (pblockingCallSemReceived != NULL)
            {
                CamOsTsemUp(pblockingCallSemReceived);
                pblockingCallSemReceived = NULL;
            }
            ret                      = pArgs->ops.fill_buffer(pArgs, &frameBuf, &frameDma, &size, bufSizeRequired);
            pArgs->viMsgQueueTimeout = (SSOS_DEF_OK == ret) ? 0 : 1;
            if (ret == SSOS_DEF_ENOMEM)
            {
                SSOS_TIME_Sleep(1);
                continue;
            }
            if (ret == SSOS_DEF_FAIL || !frameDma || !frameBuf)
            {
                pArgs->ops.finish_buffer(pArgs);
                SSOS_TIME_Sleep(1);
                continue;
            }
            sstar_usbd_uvc_send_frame(pArgs->uvc_idx, frameBuf, frameDma, size);
            pArgs->ops.finish_buffer(pArgs);
        }
        else
        {
            if (pArgs->stream_init == 1)
            {
                // int ret = 0;
                // PCAM_USB_SetAttrEnable(pArgs->uvc_idx, 0);
                if (!pArgs->ops.stop_caputre)
                {
                    PTREE_ERR("stop capture fail\n");
                    return NULL;
                }
                ret = pArgs->ops.stop_caputre(pArgs);
                if (ret != SSOS_DEF_OK)
                {
                    PTREE_ERR("UVC_StopCapture failed \n");
                }

                pArgs->stream_init = 0;
                _PTREE_UVC_AppVideoBufferFree(pArgs, bufSizeRequired);
            }

            pArgs->viMsgQueueTimeout = 1;
        }

        // CamOsUsSleep(1);
        if (pblockingCallSemReceived != NULL)
        {
            CamOsTsemUp(pblockingCallSemReceived);
            pblockingCallSemReceived = NULL;
        }
    }
    return NULL;
}

#if defined(CONFIG_USB_GADGET_UVC_INTERRUPT_EP_SUPPORT)
static void *PTREE_UVC_VideoControlTask(PTREE_TASK_Buffer_t *data)
{
    u32 status_size = 0;

    ST_UVC_ARGS                                              *pArgs;
    __attribute__((__aligned__(64))) struct uvc_status_packet status_pkt;
    CamOsRet_e                                                eRet;

    pArgs = (ST_UVC_ARGS *)data->buf;

    while (CAM_OS_OK != CamOsThreadShouldStop())
    {
        eRet = CamOsTsemTimedDown(&pArgs->videoCtrlSem, pArgs->vcSemWaitTimeout);

        if (pArgs->vcThreadShouldStop != 0)
            break;

        if (eRet == CAM_OS_OK)
        {
            status_size = uvc_video_get_status_packet(pArgs->uvc_idx, &status_pkt);
            if (status_size > 0)
            {
                sstar_usbd_uvc_send_status(pArgs->uvc_idx, &status_pkt, status_size);
            }
        }
        else if (eRet == CAM_OS_TIMEOUT)
        {
            // NOP
        }
    }

    return NULL;
}
#endif
int PTREE_UVC_Init(ST_UVC_ARGS *param)
{
    SSOS_TASK_Attr_t         taskAttr;
    int                       vid_idx = 0;
    struct usb_composite_info info    = {0};

    vid_idx = param->uvc_idx;
    SSOS_ASSERT(param->ops.uvc_init);
    param->ops.uvc_init(NULL);
    g_uvcAppPara[vid_idx] = param;
    SSOS_THREAD_MutexInit(&g_dataMutex);

    usb_class_ac_vc_app_init();
    if (CamOsMsgQueueCreate(&(g_uvcAppPara[vid_idx]->viMsgQueue), 10) != SSOS_DEF_OK)
    {
        PTREE_ERR("Msg queue Create Failed\n");
        return SSOS_DEF_FAIL;
    }
    // Create Thread
    CamOsMemset(&taskAttr, 0, sizeof(SSOS_TASK_Attr_t));
    taskAttr.doMonitor                     = PTREE_UVC_VideoProcessTask;
    taskAttr.inBuf.buf                     = (void *)g_uvcAppPara[vid_idx];
    taskAttr.threadAttr.name               = (char *)g_uvcAppPara[vid_idx]->threadName;
    taskAttr.threadAttr.priority           = 97;
    taskAttr.threadAttr.stackSize          = 16 * 1024;
    g_uvcAppPara[vid_idx]->videoIntfThread = SSOS_TASK_Open(&taskAttr);
    if (!g_uvcAppPara[vid_idx]->videoIntfThread)
    {
        PTREE_ERR("Create thread %s failed\n", taskAttr.threadAttr.name);
        return -1;
    }
    SSOS_TASK_StartMonitor(g_uvcAppPara[vid_idx]->videoIntfThread);

    PTREE_DBG("thread %s start.\n", g_uvcAppPara[vid_idx]->threadName);
#if defined(CONFIG_USB_GADGET_UVC_INTERRUPT_EP_SUPPORT)
    PTREE_TASK_Attr_t taskCtrlAttr;
    CamOsMemset(&taskCtrlAttr, 0, sizeof(PTREE_TASK_Attr_t));
    g_uvcAppPara[vid_idx]->vcSemWaitTimeout   = 1000;
    g_uvcAppPara[vid_idx]->vcThreadShouldStop = 0;
    CamOsTsemInit(&g_uvcAppPara[vid_idx]->videoCtrlSem, 0);
    taskCtrlAttr.doMonitor                 = PTREE_UVC_VideoControlTask;
    taskCtrlAttr.inBuf.buf                 = (void *)g_uvcAppPara[vid_idx];
    taskCtrlAttr.threadAttr.name           = (char *)g_uvcAppPara[vid_idx]->threadName;
    taskCtrlAttr.threadAttr.priority       = 50;
    taskCtrlAttr.threadAttr.stackSize      = 6 * 1024;
    g_uvcAppPara[vid_idx]->videoCtrlThread = SSOS_TASK_Open(&taskCtrlAttr);
    if (!g_uvcAppPara[vid_idx]->videoCtrlThread)
    {
        PTREE_ERR("Create thread %s failed\n", taskCtrlAttr.threadAttr.name);
        return -1;
    }
    SSOS_TASK_StartMonitor(g_uvcAppPara[vid_idx]->videoCtrlThread);
#endif

    info.uvc_enable[g_uvcNum] = 1;
    SSOS_THREAD_MutexLock(&g_dataMutex);
    g_uvcNum++;
    SSOS_THREAD_MutexUnlock(&g_dataMutex);
    // add compsite init
    // composite init must be after all uvc device start
    if (g_uvcNum == CONFIG_USB_GADGET_UVC_STREAM_NUM)
    {
        info.uvc_ops = &uvc_app_ops;
        sstar_usb_composite_init(&info);
    }
    return SSOS_DEF_OK;
}

int PTREE_UVC_Deinit(int vid_idx)
{
    ST_UVC_ARGS *pArgs;

    // add composite deinit, composite deinit before pipeline deinit
    SSOS_THREAD_MutexLock(&g_dataMutex);
    g_uvcNum--;
    SSOS_THREAD_MutexUnlock(&g_dataMutex);
    if (g_uvcNum == 0)
    {
        sstar_usb_composite_deinit();
    }
    pArgs = g_uvcAppPara[vid_idx];
    SSOS_ASSERT(pArgs->ops.uvc_deinit);
    pArgs->ops.uvc_deinit(pArgs);
    SSOS_TASK_Stop(pArgs->videoIntfThread);
    SSOS_TASK_Close(pArgs->videoIntfThread);
    pArgs->videoIntfThread = NULL;
    CamOsMsgQueueDestroy(pArgs->viMsgQueue);
#if defined(CONFIG_USB_GADGET_UVC_INTERRUPT_EP_SUPPORT)
    SSOS_TASK_Stop(pArgs->videoCtrlThread);
    SSOS_TASK_Close(pArgs->videoCtrlThread);
    pArgs->videoCtrlThread = NULL;
    CamOsTsemUp(&pArgs->videoCtrlSem);
    CamOsTsemDeinit(&pArgs->videoCtrlSem);
#endif
    PTREE_DBG("%s stop.\n", pArgs->threadName);
    return SSOS_DEF_OK;
}
/* callback from usb driver */
void uvc_app_stop_capture(unsigned char uvc_idx)
{
    ST_UVC_ARGS *pArgs;

    pArgs = g_uvcAppPara[uvc_idx];
    if (0 != _PTREE_UVC_AppMsgEnqueue(pArgs->viMsgQueue, UVC_STOP_CAPTURE, 1, 1, 5000))
    {
        PTREE_ERR("msg:stop capture, enauqe fail\n");
    }
    // Fixed: Preview would be failed after run CV TD9.4/TD9.9 and exit app.
    uvc_video_clear_stream_params(uvc_idx);
}

signed char uvc_app_start_capture(unsigned char uvc_idx)
{
    Stream_Params_t *pstream_params = NULL;
    ST_UVC_ARGS     *pArgs;

    pArgs = g_uvcAppPara[uvc_idx];
    uvc_video_get_stream_params(uvc_idx, &pArgs->attr.stream_params);
    pstream_params = &pArgs->attr.stream_params;
    // CV tool sends set_alt without UVC probe/commit in advance. (Configured State:TD9.4 & TD9.9)
    if (pstream_params == NULL || pstream_params->width == 0 || pstream_params->height == 0
        || pstream_params->frameRate == 0 || pstream_params->maxframesize == 0)
    {
        return 1;
    }
    if (0 != _PTREE_UVC_AppMsgEnqueue(pArgs->viMsgQueue, UVC_START_CAPTURE, 1, 1, 1000))
    {
        PTREE_ERR("msg:start capture, enauqe fail\n");
        return 1;
    }
    return 0;
}

signed char uvc_app_process_vc_data(unsigned char idx, struct usb_request_data *data)
{
    char ret;

    ret = uvc_video_process_vc_data(idx, data);

#if defined(CONFIG_USB_GADGET_UVC_INTERRUPT_EP_SUPPORT)
    if (uvc_video_get_status_updated(idx) != 0)
        CamOsTsemUp(&g_uvcAppPara[idx]->videoCtrlSem);
#endif

    return ret;
}

void uvc_app_set_config(unsigned char config)
{
    return;
}

void uvc_app_suspend(unsigned char idx)
{
    ST_UVC_ARGS *pArgs;

    pArgs = g_uvcAppPara[idx];
    if (0 != _PTREE_UVC_AppMsgEnqueue(pArgs->viMsgQueue, UVC_SUSPEND, 10, 0, 0))
    {
        PTREE_ERR("msg:uvc suspend, enauqe fail\n");
    }
}

void uvc_app_resume(unsigned char uvc_idx)
{
    ST_UVC_ARGS *pArgs;

    pArgs              = g_uvcAppPara[uvc_idx];
    pArgs->flag_resume = 1;
}

void uvc_app_usb_reset(unsigned char uvc_idx)
{
    ST_UVC_ARGS *pArgs;

    pArgs              = g_uvcAppPara[uvc_idx];
    pArgs->flag_resume = 1;
}

void uvc_app_speed_negotiate(unsigned char speed, unsigned char interface)
{
    uvc_video_speed_negotiate(speed, interface);
}

signed int ST_UVC_SwitchIQBinByChannel(unsigned int u32Ch, unsigned char u8Mod)
{
    return 0;
}
signed int ST_UVC_CfgSNRHDRSetting(unsigned char SensorId, unsigned char SnrHDREnable)
{
    return 0;
}
signed int ST_UVC_SetVideoMute(unsigned int uvc_idx, unsigned int enMute)
{
    return 0;
}

signed int ST_UVC_SetSensrFpsEx(MI_U32 ep_id, MI_U32 u32FrmRateNum, MI_U32 u32FrmRateDenom)
{
    return 0;
}
void usb_uvc_GetPrevwSts(u8 *pIsPreviewEnable)
{
    *pIsPreviewEnable = 0;
}
