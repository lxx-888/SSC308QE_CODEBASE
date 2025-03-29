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
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "mi_venc.h"
#include "st_common_rtsp_video.h"

#define RTSP_SERVER_MAX 32
#define BUF_POOL_MAX 60

typedef struct ST_RtspInfo_s
{
    int refcnt;
    void *pool_handle;
    ss_rtsp_handle rtsphandle;
    pthread_mutex_t Outputmutex;
    pthread_t pThread;
    MI_BOOL bThreadExit;
    MI_U32 Dev;
    MI_U32 Chn;
    char url[16];
    struct rtsp_connection_fp fp;
    MI_BOOL bSaveFile;
    MI_U16 u16SaveNum;
    char u8SaveFilePath[256];
}ST_RtspInfo_t;

static ST_RtspInfo_t gstRtspInfo[RTSP_SERVER_MAX];

void *venc_read_stream(void *args)
{
    int ret;
    MI_S32 s32Ret = MI_SUCCESS;
    MI_VENC_Stream_t stStream;
    ST_RtspInfo_t *pstRtspInfo = (ST_RtspInfo_t *)args;
    MI_VENC_ChnStat_t stStat;
    MI_VENC_ChnAttr_t stVencAttr;
    MI_VENC_DEV VencDev = pstRtspInfo->Dev;
    MI_VENC_CHN VencChn = pstRtspInfo->Chn;
    MI_S32 s32Fd = -1;
    struct timeval tv;
    fd_set read_fds;
    FILE *pSaveFile = NULL;

    memset(&stVencAttr, 0x0, sizeof(MI_VENC_ChnAttr_t));
    s32Ret = MI_VENC_GetChnAttr(VencDev, VencChn, &stVencAttr);
    if(s32Ret != MI_SUCCESS)
    {
        printf("channel %d, not valid \n", VencChn);
        return NULL;
    }

    s32Fd = MI_VENC_GetFd(VencDev, VencChn);
    if (s32Fd < 0)
    {
        printf("GET Venc Fd Failed , Venc Dev = %d Chn = %d\n", VencDev, VencChn);
        return NULL;
    }

    printf("[%s] Venc Fd = %d, Venc Dev = %d Chn = %d\n ", __FUNCTION__, s32Fd, VencDev, VencChn);

    while (!pstRtspInfo->bThreadExit)
    {
        FD_ZERO(&read_fds);
        FD_SET(s32Fd, &read_fds);

        tv.tv_sec = 0;
        tv.tv_usec = 500 * 1000;

        s32Ret = select(s32Fd + 1, &read_fds, NULL, NULL, &tv);
        if (s32Ret < 0)
        {
            printf("VideoEncoder Stream_Task%d select failed\n", VencChn);
            usleep(10 * 1000);
            continue;
        }
        else if (0 == s32Ret)
        {
            usleep(10 * 1000);
            continue;
        }
        else
        {
            FD_CLR(s32Fd, &read_fds);

            memset(&stStream, 0, sizeof(stStream));
            memset(&stStat, 0, sizeof(MI_VENC_ChnStat_t));
            s32Ret = MI_VENC_Query(VencDev, VencChn, &stStat);
            if (MI_SUCCESS != s32Ret || stStat.u32CurPacks == 0)
            {
                usleep(10 * 1000);
                continue;
            }

            stStream.pstPack = (MI_VENC_Pack_t*)malloc(sizeof(MI_VENC_Pack_t)*stStat.u32CurPacks);
            if (NULL == stStream.pstPack)
            {
                usleep(10 * 1000);
                continue;
            }
            stStream.u32PackCount = stStat.u32CurPacks;
            pthread_mutex_lock(&pstRtspInfo->Outputmutex);
            if(pstRtspInfo->refcnt > 0)
            {
                //check pool
                ret = ss_rtsp_server_check_pool(pstRtspInfo->rtsphandle, pstRtspInfo->pool_handle);
                if(ret != 0)
                {
                    printf("Buf pool is full");
                    continue;
                }

                s32Ret = MI_VENC_GetStream(VencDev, VencChn, &stStream, FALSE);
                if (MI_SUCCESS == s32Ret)
                {
                    struct rtsp_frame_packet frame_package;
                    struct timeval stamp;
                    memset(&frame_package, 0, sizeof(struct rtsp_frame_packet));
                    memset(&stamp, 0, sizeof(struct timeval));
                    frame_package.packet_count = stStream.u32PackCount;
                    for (unsigned int i = 0; i < stStream.u32PackCount; i++)
                    {
                        frame_package.packet_data[i].data =  (char*)stStream.pstPack[i].pu8Addr;
                        frame_package.packet_data[i].size = stStream.pstPack[i].u32Len;
                        if(pstRtspInfo->bSaveFile)
                        {
                            if (MI_SUCCESS
                                != ST_Common_WriteFile(pstRtspInfo->u8SaveFilePath, &pSaveFile,
                                                    stStream.pstPack[i].pu8Addr + stStream.pstPack[i].u32Offset,
                                                    stStream.pstPack[i].u32Len - stStream.pstPack[i].u32Offset,
                                                    &pstRtspInfo->u16SaveNum, 0))
                            {
                                printf("wirtefile %s fail\n", pstRtspInfo->u8SaveFilePath);
                            }
                            if (pstRtspInfo->u16SaveNum == 0)
                            {
                                pstRtspInfo->bSaveFile=FALSE;
                                printf("wirtefile %s done \n",pstRtspInfo->u8SaveFilePath);
                            }
                        }
                    }

                    frame_package.stamp.tv_sec  = stStream.pstPack[0].u64PTS / 1000000;
                    frame_package.stamp.tv_usec = stStream.pstPack[0].u64PTS % 1000000;

                    ss_rtsp_server_send_package(pstRtspInfo->rtsphandle, pstRtspInfo->pool_handle, &frame_package);

                    MI_VENC_ReleaseStream(VencDev, VencChn, &stStream);
                }
            }
            pthread_mutex_unlock(&pstRtspInfo->Outputmutex);
            free(stStream.pstPack);
            stStream.pstPack = NULL;
        }
    }

    MI_VENC_CloseFd(VencDev, VencChn);
    FD_ZERO(&read_fds);

    return NULL;
}


void venc_open_stream(const char *url, void *user_data)
{
    ST_RtspInfo_t *pstRtspInfo = (ST_RtspInfo_t *)user_data;

    if(MI_SUCCESS != MI_VENC_RequestIdr(pstRtspInfo->Dev, pstRtspInfo->Chn, TRUE))
    {
        printf("dev%d chn%d open stream fail\n", pstRtspInfo->Dev, pstRtspInfo->Chn);
    }

    pthread_mutex_lock(&pstRtspInfo->Outputmutex);
    pstRtspInfo->refcnt++;
    pthread_mutex_unlock(&pstRtspInfo->Outputmutex);

    printf("venc_open_stream success dev %d, chn %d\n", pstRtspInfo->Dev, pstRtspInfo->Chn);
}

void venc_close_stream(const char *url, void *user_data)
{
    ST_RtspInfo_t *pstRtspInfo = (ST_RtspInfo_t *)user_data;

    pthread_mutex_lock(&pstRtspInfo->Outputmutex);
    pstRtspInfo->refcnt--;
    pthread_mutex_unlock(&pstRtspInfo->Outputmutex);

    printf("venc_close_stream success, dev %d, chn %d\n",  pstRtspInfo->Dev, pstRtspInfo->Chn);
}

MI_S32 ST_Common_RtspServerStartVideo(ST_VideoStreamInfo_t *pstStreamAttr)
{
    int ret = 0;
    struct rtsp_video_info info;
    struct rtsp_pool_config config;
    ST_RtspInfo_t *pstRtspInfo = NULL;

    if(pstStreamAttr == NULL)
    {
        printf("pointer null\n");
        return -1;
    }

    pstRtspInfo = &gstRtspInfo[pstStreamAttr->rtspIndex];

    //create rtsp handle
    pstRtspInfo->rtsphandle = ss_rtsp_server_create();
    if(pstRtspInfo->rtsphandle == NULL)
    {
        return -1;
    }

    //add video url
    memset(&info, 0x00, sizeof(struct rtsp_video_info));
    if(pstStreamAttr->eType == E_MI_VENC_MODTYPE_H264E)
    {
        info.format = RTSP_ES_FMT_VIDEO_H264;
    }
    else if(pstStreamAttr->eType == E_MI_VENC_MODTYPE_H265E)
    {
        info.format = RTSP_ES_FMT_VIDEO_H265;
    }
    else if(pstStreamAttr->eType == E_MI_VENC_MODTYPE_JPEGE)
    {
        info.format = RTSP_ES_FMT_VIDEO_JPEG;
    }
    else if(pstStreamAttr->eType == E_MI_VENC_MODTYPE_AV1)
    //if rtsp use AV1, need to use ffmpeg tool to paly
    //(in path: project/tools/av1X86Player/ffmpeg_av1)
    {
        info.format = RTSP_ES_FMT_VIDEO_AV1;
    }
    else
    {
        printf("venc type %d not support\n", pstStreamAttr->eType);
        return -1;
    }
    info.width = pstStreamAttr->u32Width;
    info.height = pstStreamAttr->u32Height;
    sprintf(pstRtspInfo->url, "66%d%d", pstStreamAttr->VencDev, pstStreamAttr->VencChn);
    info.frame_rate = pstStreamAttr->u32FrameRate;

    printf("rtsp venc dev%d, chn %d, type %d, url %s\n", pstStreamAttr->VencDev,
        pstStreamAttr->VencChn, pstStreamAttr->eType, pstRtspInfo->url);

    pstRtspInfo->Dev = pstStreamAttr->VencDev;
    pstRtspInfo->Chn = pstStreamAttr->VencChn;

    //set connect/disconnect callbackfun
    pstRtspInfo->fp.connect = venc_open_stream;
    pstRtspInfo->fp.disconnect = venc_close_stream;
    pstRtspInfo->fp.user_data  = (void *)pstRtspInfo;

    ret = ss_rtsp_server_add_video_url(pstRtspInfo->rtsphandle, pstRtspInfo->url, &info, pstRtspInfo->fp);
    if(ret != 0)
    {
        return -1;
    }

    //start server
    ret = ss_rtsp_server_start(pstRtspInfo->rtsphandle);
    if(ret != 0)
    {
        return -1;
    }

    //get pool handle
    pstRtspInfo->pool_handle = ss_rtsp_server_get_video_pool(pstRtspInfo->rtsphandle, pstRtspInfo->url);

    //config pool
    memset(&config, 0x00, sizeof(struct rtsp_pool_config));
    config.depth = BUF_POOL_MAX;
    ss_rtsp_server_config_pool(pstRtspInfo->rtsphandle, pstRtspInfo->pool_handle, &config);

    pstRtspInfo->refcnt = 0;

    //satrt thread read stream
    pthread_mutex_init(&pstRtspInfo->Outputmutex, NULL);
    pstRtspInfo->bThreadExit = FALSE;
    if (pstStreamAttr->bSaveFile && pstStreamAttr->u16SaveNum !=0 && pstStreamAttr->u8SaveFilePath != NULL)
    {
        // printf("pstStreamAttr->u8SaveFilePath %s\n",pstStreamAttr->u8SaveFilePath);
        pstRtspInfo->bSaveFile=pstStreamAttr->bSaveFile;
        pstRtspInfo->u16SaveNum=pstStreamAttr->u16SaveNum;
        if(pstStreamAttr->eType == E_MI_VENC_MODTYPE_H264E)
        {
            sprintf(pstRtspInfo->u8SaveFilePath, "%s.h264", pstStreamAttr->u8SaveFilePath);
        }
        else if(pstStreamAttr->eType == E_MI_VENC_MODTYPE_H265E)
        {
            sprintf(pstRtspInfo->u8SaveFilePath, "%s.h265", pstStreamAttr->u8SaveFilePath);
        }
        else if(pstStreamAttr->eType == E_MI_VENC_MODTYPE_JPEGE)
        {
            sprintf(pstRtspInfo->u8SaveFilePath, "%s.jpg", pstStreamAttr->u8SaveFilePath);
        }
        else if(pstStreamAttr->eType == E_MI_VENC_MODTYPE_AV1)
        {
            sprintf(pstRtspInfo->u8SaveFilePath, "%s.av1", pstStreamAttr->u8SaveFilePath);
        }
        else
        {
            printf("venc type %d not support\n", pstStreamAttr->eType);
            return -1;
        }
    }

    pthread_create(&pstRtspInfo->pThread, NULL, venc_read_stream, (void *)pstRtspInfo);

    return ret;
}

MI_S32 ST_Common_RtspServerStopVideo(ST_VideoStreamInfo_t *pstStreamAttr)
{
    int ret = 0;
    ST_RtspInfo_t *pstRtspInfo = &gstRtspInfo[pstStreamAttr->rtspIndex];

    if(pstStreamAttr == NULL)
    {
        printf("pointer null\n");
        return -1;
    }

    if(!pstRtspInfo->rtsphandle)
    {
        printf("stop rtsp error, rtspHandle Null\n");
        return -1;
    }
    //stop thread
    pstRtspInfo->bThreadExit = TRUE;
    pthread_join(pstRtspInfo->pThread, NULL);
    pthread_mutex_destroy(&pstRtspInfo->Outputmutex);

    ret = ss_rtsp_server_stop(pstRtspInfo->rtsphandle);
    if(ret != 0)
    {
        return -1;
    }

    ret = ss_rtsp_server_del_video_url(pstRtspInfo->rtsphandle, pstRtspInfo->url);
    if(ret != 0)
    {
        return -1;
    }

    pstRtspInfo->fp.connect    = NULL;
    pstRtspInfo->fp.disconnect = NULL;
    pstRtspInfo->fp.user_data  = NULL;

    //destroy server
    ss_rtsp_server_destroy(pstRtspInfo->rtsphandle);

    return 0;
}

char * ST_Common_RtspServerGetUrl(unsigned int rtspIndex)
{
    ST_RtspInfo_t *pstRtspInfo = &gstRtspInfo[rtspIndex];

    if(!pstRtspInfo->rtsphandle)
    {
        printf("rtsp index %d error, rtspHandle Null\n", rtspIndex);
        return NULL;
    }

    return (char *)ss_rtsp_server_get_url_prefix(pstRtspInfo->rtsphandle, pstRtspInfo->pool_handle);
}

