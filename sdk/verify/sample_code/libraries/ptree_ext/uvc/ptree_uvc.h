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
#ifndef __PTREE_UVC__
#define __PTREE_UVC__

#include "mi_common_datatype.h"
#include "uvc_video.h"

#define MAX_VS_IF (CONFIG_USB_GADGET_UVC_STREAM_NUM)
#ifndef ALIGN_UP
#define ALIGN_UP(x, align) (((x) + ((align) - 1)) & ~((align) - 1))
#endif

#ifndef ALIGN_DOWN
#define ALIGN_DOWN(x, align) (((x) / (align)) * (align))
#endif

enum msg_type
{
    UVC_STOP_CAPTURE,
    UVC_START_CAPTURE,
    UVC_SUSPEND,
};
struct uvc_app_msg
{
    enum msg_type type;
    CamOsTsem_t  *pblocking_call_sem;
};
typedef struct uvc_ops
{
    int (*uvc_init)(void *);
    int (*uvc_deinit)(void *);
    int (*start_caputre)(void *);
    int (*fill_buffer)(void *, char **, MI_PHY *, u32 *, u32);
    int (*finish_buffer)(void *);
    int (*stop_caputre)(void *);
} uvcOps_t;
typedef struct Stream_Attr_S
{
    char            pszStreamName[32];
    unsigned char   bForceIdr;
    Stream_Params_t stream_params;
} StreamAttr_t;
typedef struct uvc_app_para
{
    unsigned int  uvc_idx;
    CamOsThread   videoIntfThread;
    char          threadName[32];
    CamOsMsgQueue viMsgQueue;
    unsigned int  viMsgQueueTimeout;
#if defined(CONFIG_USB_GADGET_UVC_INTERRUPT_EP_SUPPORT)
    CamOsThread videoCtrlThread;
    CamOsTsem_t videoCtrlSem;
    u32         vcSemWaitTimeout;
    u8          vcThreadShouldStop;
#endif

    unsigned char      stream_init;
    unsigned char      uvc_status;
    unsigned char      flag_resume;
    StreamAttr_t       attr;
    void              *puvc_frm_buf;
    unsigned long long uvc_frm_dma;
    void              *packet;
    void              *covertPtk;

#if defined(CONFIG_CUS3A_SUPPORT)
    // CUS3A_ALGO_STATUS_t cus_3a_status;
#endif

    // ST_USBUVC_IMPL_PTZ_t ptz_impl_handle;
    // PCAM_USB_ZOOM zoom_cur;
    // PCAM_USB_PANTILT pan_tilt_cur;

#if (VID_LATENCY_MEASURE)
    UVC_LATENCY_MEASUREMENT_t uvc_latency_para;
#endif
    uvcOps_t ops;
} ST_UVC_ARGS;

/* call back function */
void        uvc_app_stop_capture(unsigned char uvc_idx);
signed char uvc_app_start_capture(unsigned char uvc_idx);
signed char uvc_app_process_vc_data(unsigned char idx, struct usb_request_data *data);
void        uvc_app_set_config(unsigned char config);
void        uvc_app_suspend(unsigned char idx);
void        uvc_app_resume(unsigned char uvc_idx);
void        uvc_app_usb_reset(unsigned char uvc_idx);
void        uvc_app_speed_negotiate(unsigned char speed, unsigned char interface);
signed int  ST_UVC_SwitchIQBinByChannel(unsigned int u32Ch, unsigned char u8Mod);
signed int  ST_UVC_CfgSNRHDRSetting(unsigned char SensorId, unsigned char SnrHDREnable);
signed int  ST_UVC_SetVideoMute(unsigned int uvc_idx, unsigned int enMute);

int PTREE_UVC_Init(ST_UVC_ARGS *);
int PTREE_UVC_Deinit(int);

#endif
