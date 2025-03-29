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

#include "mi_common_datatype.h"
#include "st_common.h"
#include "st_common_aov.h"
#include <fcntl.h>
#include <pthread.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <linux/rtc.h>
#include <unistd.h>
#include "platform.h"
#include "mi_isp_awb.h"
#include "mi_isp_ae.h"
#include <sys/mman.h>
#include "mdrv_msys_io.h"
#include "mdrv_msys_io_st.h"
#include "mdrv_verchk.h"
#include <sys/personality.h>
#include "light_misc_control_api.h"

static MI_U8 *u8pMapBase;
static int    u32MsysFd = 0;
static int    u32MemFd  = 0;

const static ST_Common_AovLightTable_t g_stLightTable[2][10] = {
    {
        // 100 TIG
        //{   1, 33311, 6623,1024}, // 1LUX
        {1, 30000, 262144, 1024}, // 1LUX
        {3, 30000, 8331, 1024},   // 3LUX
        {7, 30000, 4128, 1024},   // 7LUX
        {15, 10000, 5875, 1024},  // 15LUX
                                  //{  30, 12889, 1025,1024}, // 30LUX
        {30, 10000, 3040, 1024},  // 30LUX
                                  //{  63,  6400, 1025,1025}, // 63LUX
        {63, 10000, 1508, 1025},  // 63LUX
        {132, 7158, 1025, 1024},  // 132LUX
        {265, 3579, 1025, 1038},  // 265LUX
        {511, 1790, 1025, 1042},  // 511LUX
        {1638, 870, 1025, 1067},  // 1638LUX
    },
    {
        // 400 TIG
        {7, 33311, 6281, 1024},   // 7LUX
        {16, 33311, 3220, 1024},  // 16LUX
        {33, 33311, 1587, 1024},  // 33LUX
        {68, 25536, 1025, 1024},  // 68LUX
        {138, 12867, 1025, 1024}, // 138LUX
        {287, 6423, 1025, 1024},  // 287LUX
        {590, 3112, 1025, 1025},  // 590LUX
        {1182, 1578, 1025, 1025}, // 1182LUX
        {1638, 778, 1025, 1035},  // 1638LUX
        {1638, 778, 1025, 1038},
    }};

MI_S32 __PrintToKmsg(char *buf)
{
    static MI_S32 fdKmsg = -1;

    if (fdKmsg == -1)
    {
        fdKmsg = open("/dev/kmsg", O_WRONLY);
    }

    write(fdKmsg, buf, strlen(buf) + 1);

    return MI_SUCCESS;
}

MI_S32 __WriteFile(const char *path, const char *str)
{
    MI_S32 s32Ret = MI_SUCCESS;
    MI_S32 fd;

    fd = open(path, O_RDWR);
    if (fd >= 0)
    {
        s32Ret = write(fd, str, strlen(str));
        if (s32Ret < 0)
        {
            printf("%s write fail\n", path);
            close(fd);
            return -1;
        }
        close(fd);
    }
    else
    {
        printf("%s open fail\n", path);
        return -1;
    }

    return MI_SUCCESS;
}

MI_S32 __PowerOff(ST_SysoffType_e stSysoffType, MI_BOOL bAutoTest)
{
    MI_S32 s32Ret = MI_SUCCESS;

    if (stSysoffType == E_ST_SYSOFF_REBOOT)
    {
        printf("system will reboot now\n");
        CHECK_AOV_RESULT(system("reboot -f"), s32Ret, EXIT);
    }
    else if (stSysoffType == E_ST_SYSOFF_POWEROFF)
    {
        if (TRUE == bAutoTest)
        {
            s32Ret = __WriteFile(ALARM_DEVICE_NAME, "4");
            if (s32Ret != MI_SUCCESS)
            {
                return -1;
            }
        }
        printf("system will power off now\n");
        CHECK_AOV_RESULT(system("poweroff -f"), s32Ret, EXIT);
    }

EXIT:
    return s32Ret;
}

MI_S32 __AovStreamQueueAdd(ST_Common_AovStreamHandle_t *pstStreamHandle, ST_Common_AovStreamNode_t *pstVencNewNode)
{
    ST_CHECK_POINTER(pstStreamHandle);
    ST_CHECK_POINTER(pstVencNewNode);

    pthread_mutex_lock(&(pstStreamHandle->ListMutex));

    list_add_tail(&(pstVencNewNode->list), &(pstStreamHandle->stStreamNode.list));
    pstStreamHandle->u32ListCnt++;

    for (MI_U32 i = 0; i < pstVencNewNode->stStream.u32PackCount; i++)
    {
        pstStreamHandle->u32StreamSize += pstVencNewNode->stStream.pstPack[i].u32Len;
    }

    pthread_mutex_unlock(&(pstStreamHandle->ListMutex));

    return MI_SUCCESS;
}

MI_S32 __AovStreamQueueDelete(ST_Common_AovStreamHandle_t *pstStreamHandle, ST_Common_AovStreamNode_t *pstVencDeletNode)
{
    ST_CHECK_POINTER(pstStreamHandle);
    ST_CHECK_POINTER(pstVencDeletNode);

    pthread_mutex_lock(&(pstStreamHandle->ListMutex));

    list_del(&pstVencDeletNode->list);
    pstStreamHandle->u32ListCnt--;

    for (MI_U32 i = 0; i < pstVencDeletNode->stStream.u32PackCount; i++)
    {
        pstStreamHandle->u32StreamSize -= pstVencDeletNode->stStream.pstPack[i].u32Len;
    }

    pthread_mutex_unlock(&(pstStreamHandle->ListMutex));

    return MI_SUCCESS;
}

MI_BOOL __AovStreamQueueIsEmpty(ST_Common_AovStreamHandle_t *pstStreamHandle)
{
    ST_CHECK_POINTER(pstStreamHandle);

    MI_BOOL bRet;

    pthread_mutex_lock(&(pstStreamHandle->ListMutex));

    bRet = list_empty(&(pstStreamHandle->stStreamNode.list));

    pthread_mutex_unlock(&(pstStreamHandle->ListMutex));

    return bRet;
}

MI_BOOL __AovStreamQueueHeadNodeGet(ST_Common_AovStreamHandle_t *pstStreamHandle,
                                    ST_Common_AovStreamNode_t ** ppstVencIterateNode)
{
    ST_CHECK_POINTER(pstStreamHandle);

    MI_BOOL bRet;

    pthread_mutex_lock(&(pstStreamHandle->ListMutex));

    bRet = list_empty(&pstStreamHandle->stStreamNode.list);
    if (bRet == TRUE)
    {
        bRet = FALSE;
    }
    else
    {
        *ppstVencIterateNode = list_entry((pstStreamHandle->stStreamNode.list.next), ST_Common_AovStreamNode_t, list);
        bRet                 = TRUE;
    }

    pthread_mutex_unlock(&(pstStreamHandle->ListMutex));

    return bRet;
}

MI_S32 __AovDropVencStream(ST_Common_AovHandle_t *pstAovHandle, MI_U32 u32Index)
{
    ST_CHECK_POINTER(pstAovHandle);

    MI_S32            s32Ret       = MI_SUCCESS;
    MI_S32            s32DropCount = 0;
    MI_VENC_ChnStat_t stStat;
    MI_VENC_Stream_t  stStream;

    do
    {
        s32Ret = MI_VENC_Query(pstAovHandle->stAovPipeAttr.stChnPortVenc[u32Index].u32DevId,
                               pstAovHandle->stAovPipeAttr.stChnPortVenc[u32Index].u32ChnId, &stStat);
        if (s32Ret != MI_SUCCESS)
        {
            ST_INFO("MI_VENC_Query failed\n");
            return s32Ret;
        }

        if (0 == stStat.u32LeftStreamFrames)
        {
            break;
        }
        s32DropCount++;

        stStream.pstPack = (MI_VENC_Pack_t *)malloc(sizeof(MI_VENC_Pack_t) * stStat.u32CurPacks);
        if (NULL == stStream.pstPack)
        {
            ST_INFO("malloc MI_VENC_Pack_t failed\n");

            return E_MI_ERR_FAILED;
        }

        stStream.u32PackCount = stStat.u32CurPacks;

        s32Ret = MI_VENC_GetStream(pstAovHandle->stAovPipeAttr.stChnPortVenc[u32Index].u32DevId,
                                   pstAovHandle->stAovPipeAttr.stChnPortVenc[u32Index].u32ChnId, &stStream, -1);

        if (MI_SUCCESS != s32Ret)
        {
            ST_INFO("MI_VENC_GetStream failed\n");
            free(stStream.pstPack);

            return s32Ret;
        }

        s32Ret = MI_VENC_ReleaseStream(pstAovHandle->stAovPipeAttr.stChnPortVenc[u32Index].u32DevId,
                                       pstAovHandle->stAovPipeAttr.stChnPortVenc[u32Index].u32ChnId, &stStream);
        if (MI_SUCCESS != s32Ret)
        {
            printf("MI_VENC_ReleaseStream failed\n");
            free(stStream.pstPack);

            return s32Ret;
        }

        free(stStream.pstPack);

    } while (stStat.u32LeftStreamFrames > 0);

    if(s32DropCount)
    {
        printf("\033[33mDrop num %d ungeted venc stream while release & write stream too low\n\033[0m", s32DropCount);
    }

    return s32Ret;
}

MI_S32 __AovGetVencStream(ST_Common_AovHandle_t *pstAovHandle, ST_Common_AovStreamHandle_t *pstStreamHandle,
                          ST_Common_AovStreamNode_t **ppstVencNewNode, MI_U32 u32Fps, MI_U32 u32Index)
{
    ST_CHECK_POINTER(pstAovHandle);
    ST_CHECK_POINTER(pstStreamHandle);
    ST_CHECK_POINTER(ppstVencNewNode);

    MI_S32            s32Ret = MI_SUCCESS;
    MI_S32            s32VencFd;
    MI_VENC_ChnStat_t stStat;
    fd_set            read_fds;
    struct timeval    TimeoutVal;

    TimeoutVal.tv_sec  = pstAovHandle->bStressTest ? STRESS_TIMEOUT_S_THREAD : 2;
    TimeoutVal.tv_usec = 0;

    s32VencFd = MI_VENC_GetFd(pstAovHandle->stAovPipeAttr.stChnPortVenc[u32Index].u32DevId,
                              pstAovHandle->stAovPipeAttr.stChnPortVenc[u32Index].u32ChnId);

    do
    {
        FD_ZERO(&read_fds);
        FD_SET(s32VencFd, &read_fds);

        s32Ret = select(s32VencFd + 1, &read_fds, NULL, NULL, &TimeoutVal);

        if (s32Ret < 0)
        {
            ST_INFO("select err\n");
            break;
        }
        else if (0 == s32Ret)
        {
            ST_INFO("__AovGetVencStream timeout\n");

            MI_VENC_Query(pstAovHandle->stAovPipeAttr.stChnPortVenc[u32Index].u32DevId,
                          pstAovHandle->stAovPipeAttr.stChnPortVenc[u32Index].u32ChnId, &stStat);

            printf("u32CurPacks = %d\nu32LeftStreamFrames = %d\nu32LeftStreamBytes = %d\nu32LeftPics = %d\n",
                   stStat.u32CurPacks, stStat.u32LeftStreamFrames, stStat.u32LeftStreamBytes, stStat.u32LeftPics);
            if (FALSE == pstAovHandle->stAutoTestParam.bAutoTest)
            {
                continue;
            }
            else
            {
                s32Ret = -1;
                break;
            }
        }
        else
        {
            if (FD_ISSET(s32VencFd, &read_fds))
            {
                do
                {
                    s32Ret = MI_VENC_Query(pstAovHandle->stAovPipeAttr.stChnPortVenc[u32Index].u32DevId,
                                           pstAovHandle->stAovPipeAttr.stChnPortVenc[u32Index].u32ChnId, &stStat);
                    if (s32Ret != MI_SUCCESS)
                    {
                        ST_INFO("MI_VENC_Query failed\n");
                        return s32Ret;
                    }

                    if (0 == stStat.u32CurPacks)
                    {
                        continue;
                    }

                    if (0 == stStat.u32LeftStreamFrames)
                    {
                        break;
                    }

                    *ppstVencNewNode = (ST_Common_AovStreamNode_t *)malloc(sizeof(ST_Common_AovStreamNode_t));
                    if (NULL == *ppstVencNewNode)
                    {
                        ST_INFO("malloc ST_Common_AovStreamNode_t failed\n");
                        return E_MI_ERR_FAILED;
                    }

                    (*ppstVencNewNode)->stStream.pstPack =
                        (MI_VENC_Pack_t *)malloc(sizeof(MI_VENC_Pack_t) * stStat.u32CurPacks);
                    if (NULL == (*ppstVencNewNode)->stStream.pstPack)
                    {
                        ST_INFO("malloc MI_VENC_Pack_t failed\n");
                        free(*ppstVencNewNode);
                        return E_MI_ERR_FAILED;
                    }

                    (*ppstVencNewNode)->stStream.u32PackCount = stStat.u32CurPacks;

                    s32Ret = MI_VENC_GetStream(pstAovHandle->stAovPipeAttr.stChnPortVenc[u32Index].u32DevId,
                                               pstAovHandle->stAovPipeAttr.stChnPortVenc[u32Index].u32ChnId,
                                               &((*ppstVencNewNode)->stStream), -1);

                    if (MI_SUCCESS != s32Ret)
                    {
                        ST_INFO("MI_VENC_GetStream failed\n");
                        free((*ppstVencNewNode)->stStream.pstPack);
                        free(*ppstVencNewNode);
                        return s32Ret;
                    }

                    (*ppstVencNewNode)->u32Fps   = u32Fps;
                    (*ppstVencNewNode)->u32Index = u32Index;

                    __AovStreamQueueAdd(pstStreamHandle, *ppstVencNewNode);

                } while ((stStat.u32LeftStreamFrames > 0)
                         && (pstStreamHandle->u32ListCnt < pstAovHandle->stAovParam.u32WriteStreamTriggerNum));

                break;
            }
        }

    } while (1);

    return s32Ret;
}

MI_S32 __AovWriteVencStream(ST_Common_AovHandle_t *pstAovHandle, ST_Common_AovStreamNode_t *pstVencNode, FILE *fpout)
{
    ST_CHECK_POINTER(pstAovHandle);
    ST_CHECK_POINTER(pstVencNode);

    for (MI_U32 i = 0; i < pstVencNode->stStream.u32PackCount; i++)
    {
        fwrite(pstVencNode->stStream.pstPack[i].pu8Addr + pstVencNode->stStream.pstPack[i].u32Offset, 1,
               pstVencNode->stStream.pstPack[i].u32Len - pstVencNode->stStream.pstPack[i].u32Offset, fpout);
    }

    return MI_SUCCESS;
}

MI_S32 __AovReleaseVencStream(ST_Common_AovHandle_t *pstAovHandle, ST_Common_AovStreamNode_t *pstVencNode,
                              MI_U32 u32Index)
{
    ST_CHECK_POINTER(pstAovHandle);
    ST_CHECK_POINTER(pstVencNode);
    ST_CHECK_POINTER(&pstVencNode->stStream);

    MI_S32 s32Ret = MI_SUCCESS;

    s32Ret =
        MI_VENC_ReleaseStream(pstAovHandle->stAovPipeAttr.stChnPortVenc[u32Index].u32DevId,
                              pstAovHandle->stAovPipeAttr.stChnPortVenc[u32Index].u32ChnId, &(pstVencNode->stStream));
    if (MI_SUCCESS != s32Ret)
    {
        printf("MI_VENC_ReleaseStream failed\n");
        free(pstVencNode->stStream.pstPack);
        free(pstVencNode);

        return s32Ret;
    }

    free(pstVencNode->stStream.pstPack);
    free(pstVencNode);

    return s32Ret;
}

ST_WakeupType_e __ST_Common_AovWakeupMcuCheck(void)
{
    ST_WakeupType_e eWakeupType = -1;
    MI_S32          s32ExportFD = 0;
    MI_S32          s32ValueFD  = 0;
    char            u8PirGpioPath[50];
    char            u8WifiGpioPath[50];
    char            u8GpioStr[20]   = {0};
    MI_S32          s32GpioState[2] = {0};
    char            u8RdBuf[20]     = {0};

    s32ExportFD = open(GPIO_EXPORT_NAME, O_WRONLY);
    if (s32ExportFD >= 0)
    {
        sprintf(u8GpioStr, "%d", GPIO_PIR_EVENT);
        write(s32ExportFD, u8GpioStr, 4);
        memset(u8GpioStr, 0x0, sizeof(u8GpioStr));

        sprintf(u8GpioStr, "%d", GPIO_WIFI_EVENT);
        write(s32ExportFD, u8GpioStr, 4);

        close(s32ExportFD);
    }
    else
    {
        ST_INFO(GPIO_EXPORT_NAME "open fail\n");
        return -1;
    }

    sprintf(u8PirGpioPath, "/sys/class/gpio/gpio%d/value", GPIO_PIR_EVENT);
    sprintf(u8WifiGpioPath, "/sys/class/gpio/gpio%d/value", GPIO_WIFI_EVENT);
    s32ValueFD = open(u8PirGpioPath, O_RDONLY);
    if (s32ValueFD >= 0)
    {
        if (read(s32ValueFD, u8RdBuf, sizeof(u8RdBuf)) < 0)
        {
            printf("Read GPIO failed. ");
            close(s32ValueFD);
            return -1;
        }
        else
        {
            s32GpioState[0] = (int)atoi(u8RdBuf);
            memset(u8RdBuf, 0x0, sizeof(u8RdBuf));
            close(s32ValueFD);
        }
    }
    else
    {
        printf("%s open failed\n", u8PirGpioPath);
        return -1;
    }

    s32ValueFD = open(u8WifiGpioPath, O_RDONLY);
    if (s32ValueFD >= 0)
    {
        if (read(s32ValueFD, u8RdBuf, sizeof(u8RdBuf)) < 0)
        {
            printf("Read GPIO failed. ");
            close(s32ValueFD);
            return -1;
        }
        else
        {
            s32GpioState[1] = (int)atoi(u8RdBuf);
            memset(u8RdBuf, 0x0, sizeof(u8RdBuf));
            close(s32ValueFD);
        }
    }
    else
    {
        printf("%s open failed\n", u8PirGpioPath);
        return -1;
    }

    printf("gpio state %d%d\n", s32GpioState[0], s32GpioState[1]);
    if (s32GpioState[0] && !s32GpioState[1])
    {
        eWakeupType = E_ST_WAKEUP_PIR;
        printf("By pir\n");
    }
    else if (s32GpioState[1] && !s32GpioState[0])
    {
        eWakeupType = E_ST_WAKEUP_PREVIEW;
        printf("By user preview\n");
    }
    else if (s32GpioState[1] && s32GpioState[0])
    {
        printf("detect pir/user preview err\n");
    }
    else
    {
        eWakeupType = E_ST_WAKEUP_TIMER;
        printf("By timer\n");
    }
    return eWakeupType;
}

MI_S32 __RtcOpen(int *pfd)
{
    *pfd = open(RTC_DEVICE_NAME, O_RDWR);
    if (*pfd < 0)
    {
        printf(RTC_DEVICE_NAME "open fail\n");
        return -1;
    }

    return MI_SUCCESS;
}

MI_S32 __RtcClose(MI_S32 fd)
{
    MI_S32 s32Ret;

    s32Ret = close(fd);
    if (s32Ret < 0)
    {
        printf(RTC_DEVICE_NAME "close fail\n");
        return -1;
    }

    return MI_SUCCESS;
}

MI_S32 __RtcSetDefaultTime(MI_S32 fd)
{
    struct rtc_time stRtcTimeTmp;
    stRtcTimeTmp.tm_year = 2024;
    stRtcTimeTmp.tm_mon  = 1;
    stRtcTimeTmp.tm_mday = 1;
    stRtcTimeTmp.tm_hour = 0;
    stRtcTimeTmp.tm_min  = 0;
    stRtcTimeTmp.tm_sec  = 0;

    stRtcTimeTmp.tm_year = stRtcTimeTmp.tm_year - 1900;
    stRtcTimeTmp.tm_mon  = stRtcTimeTmp.tm_mon - 1;
    stRtcTimeTmp.tm_mday = stRtcTimeTmp.tm_mday;
    stRtcTimeTmp.tm_hour = stRtcTimeTmp.tm_hour;
    stRtcTimeTmp.tm_min  = stRtcTimeTmp.tm_min;
    stRtcTimeTmp.tm_sec  = stRtcTimeTmp.tm_sec;

    if (ioctl(fd, RTC_SET_TIME, &stRtcTimeTmp))
    {
        printf("RTC_SET_TIME time is error\n");
        return -1;
    }

    return MI_SUCCESS;
}

MI_S32 __RtcReadTime(struct rtc_time *pstRtcTime, MI_S32 fd)
{
    if (ioctl(fd, RTC_RD_TIME, pstRtcTime))
    {
        printf("RTC_RD_TIME ioctl fail\n");
        return -1;
    }

    pstRtcTime->tm_year = pstRtcTime->tm_year + 1900;
    pstRtcTime->tm_mon  = pstRtcTime->tm_mon + 1;

    return MI_SUCCESS;
}

MI_S32 __AovAttachTimestamp(ST_Common_AovHandle_t *pstAovHandle)
{
    MI_S32          s32Ret = MI_SUCCESS;
    struct rtc_time stRtcTimeTmp;

    s32Ret = __RtcReadTime(&stRtcTimeTmp, pstAovHandle->stAovPipeAttr.fdRtc);
    if (s32Ret != MI_SUCCESS)
    {
        printf("__RtcReadTime failed\n");
        return -1;
    }

    sprintf(pstAovHandle->stAovPipeAttr.stDrawTextAttr.text, "@Sigmastar\n\n%d-%d %d:%d:%d\n", stRtcTimeTmp.tm_mday,
            stRtcTimeTmp.tm_mon, stRtcTimeTmp.tm_hour, stRtcTimeTmp.tm_min, stRtcTimeTmp.tm_sec);
    PRINT_AOV_DEBUG("OSD = %d-%d %d:%d:%d\n", stRtcTimeTmp.tm_mday, stRtcTimeTmp.tm_mon, stRtcTimeTmp.tm_hour,
                    stRtcTimeTmp.tm_min, stRtcTimeTmp.tm_sec);
#if AOV_DEBUG
    __PrintToKmsg(pstAovHandle->stAovPipeAttr.stDrawTextAttr.text);
#endif

    s32Ret = OsdDrawTextCanvas(&pstAovHandle->stAovPipeAttr.stDrawTextAttr);
    if (s32Ret != MI_SUCCESS)
    {
        printf("OsdDrawTextCanvas failed\n");
        return -1;
    }

    return MI_SUCCESS;
}

MI_S32 __AovAttachFrame(ST_Common_AovHandle_t *pstAovHandle, Box_t astBoxes[MAX_DET_OBJECT], MI_S32 s32DetectedBoxsNum)
{
    ST_CHECK_POINTER(pstAovHandle);

    MI_S32 s32Ret = MI_SUCCESS;

    for (MI_U32 i = 0; i < pstAovHandle->stAovPipeAttr.u32MaxFrameHandleNum; i++)
    {
        if ((i < s32DetectedBoxsNum) && (DET_TARGETID == astBoxes[i].class_id))
        {
            pstAovHandle->stAovPipeAttr.stChnPortFrameParam.stFrameChnPort.u32Color = 0XFF00;

            MI_S32 x = ((astBoxes[i].x) * 8192) / pstAovHandle->stAovParam.stPreviewSize.u16Width;
            MI_S32 y = ((astBoxes[i].y) * 8192) / pstAovHandle->stAovParam.stPreviewSize.u16Height;
            MI_S32 w = ((astBoxes[i].width) * 8192) / pstAovHandle->stAovParam.stPreviewSize.u16Width;
            MI_S32 h = ((astBoxes[i].height) * 8192) / pstAovHandle->stAovParam.stPreviewSize.u16Height;

            pstAovHandle->stAovPipeAttr.stChnPortFrameParam.stFrameChnPort.stRect.s32X      = x;
            pstAovHandle->stAovPipeAttr.stChnPortFrameParam.stFrameChnPort.stRect.s32Y      = y;
            pstAovHandle->stAovPipeAttr.stChnPortFrameParam.stFrameChnPort.stRect.u32Width  = w;
            pstAovHandle->stAovPipeAttr.stChnPortFrameParam.stFrameChnPort.stRect.u32Height = h;

            pstAovHandle->stAovPipeAttr.stChnPortFrameParam.bShow = TRUE;
        }
        else
        {
            pstAovHandle->stAovPipeAttr.stChnPortFrameParam.bShow = FALSE;
        }

        s32Ret = MI_RGN_SetDisplayAttr(0, pstAovHandle->stAovPipeAttr.ahFrame[i],
                                       &(pstAovHandle->stAovPipeAttr.stChnPortRgnAttached[0]),
                                       &(pstAovHandle->stAovPipeAttr.stChnPortFrameParam));
    }

    return s32Ret;
}

MI_S32 __AudioAddWaveHeader(WaveFileHeader_t *tWavHead, SoundMode_e enSoundMode, SampleRate_e enSampleRate,
                            MI_U32 u32Len)
{
    tWavHead->chRIFF[0] = 'R';
    tWavHead->chRIFF[1] = 'I';
    tWavHead->chRIFF[2] = 'F';
    tWavHead->chRIFF[3] = 'F';

    tWavHead->chWAVE[0] = 'W';
    tWavHead->chWAVE[1] = 'A';
    tWavHead->chWAVE[2] = 'V';
    tWavHead->chWAVE[3] = 'E';

    tWavHead->chFMT[0] = 'f';
    tWavHead->chFMT[1] = 'm';
    tWavHead->chFMT[2] = 't';
    tWavHead->chFMT[3] = 0x20;
    tWavHead->dwFMTLen = 0x10;

    if (enSoundMode == E_SOUND_MODE_MONO)
    {
        tWavHead->wave.wChannels = 0x01;
    }
    else if (enSoundMode == E_SOUND_MODE_STEREO)
    {
        tWavHead->wave.wChannels = 0x02;
    }

    tWavHead->wave.wFormatTag      = 0x1;
    tWavHead->wave.wBitsPerSample  = 16; // 16bit
    tWavHead->wave.dwSamplesPerSec = enSampleRate;
    tWavHead->wave.dwAvgBytesPerSec =
        (tWavHead->wave.wBitsPerSample * tWavHead->wave.dwSamplesPerSec * tWavHead->wave.wChannels) / 8;
    tWavHead->wave.wBlockAlign = 1024;
    tWavHead->chDATA[0]        = 'd';
    tWavHead->chDATA[1]        = 'a';
    tWavHead->chDATA[2]        = 't';
    tWavHead->chDATA[3]        = 'a';
    tWavHead->dwDATALen        = u32Len;
    tWavHead->dwRIFFLen        = u32Len + sizeof(WaveFileHeader_t) - 8;

    return MI_SUCCESS;
}

MI_U32 __AudioDumpAIData(FILE *AiChnFd, MI_U32 *u32MicDumpSize, MI_AUDIO_DEV AiDevId, MI_U8 u8ChnGrpId)
{
    MI_AI_Data_t stMicFrame;
    MI_AI_Data_t stEchoFrame;
    MI_S32       s32Ret;

    // while ((nowRead.tv_sec - baseRead.tv_sec) < 10)
    {
        memset(&stMicFrame, 0, sizeof(MI_AI_Data_t));
        memset(&stEchoFrame, 0, sizeof(MI_AI_Data_t));

        s32Ret = MI_AI_Read(AiDevId, u8ChnGrpId, &stMicFrame, &stEchoFrame, -1);

        if (MI_SUCCESS == s32Ret)
        {
            /* save mic file data */
            fwrite(stMicFrame.apvBuffer[0], 1, stMicFrame.u32Byte[0], AiChnFd);
            *u32MicDumpSize += stMicFrame.u32Byte[0];
            s32Ret = MI_AI_ReleaseData(AiDevId, u8ChnGrpId, &stMicFrame, &stEchoFrame);
            if (s32Ret != MI_SUCCESS)
            {
                printf("%s:%d MI_AI_ReleaseData s32Ret:%d\n", __FUNCTION__, __LINE__, s32Ret);
            }
        }
        else
        {
            printf("Failed to get frame from Ai Device %d ChnGrp %d, error:0x%x\n", AiDevId, u8ChnGrpId, s32Ret);
        }
    }

    return s32Ret;
}

MI_U32 __AudioPlayAoData(MI_U32 playfd, char *pTmpBuf, MI_U32 writeBufferSize, MI_AUDIO_DEV AoDevId)
{
    MI_U32         s32Ret;
    MI_S32         s32ReadSize;
    static MI_BOOL bAoExit = FALSE;

    // play audio form input file
    if (FALSE == bAoExit)
    {
        s32ReadSize = read(playfd, pTmpBuf, writeBufferSize);
        if (s32ReadSize != writeBufferSize)
        {
            lseek(playfd, sizeof(WaveFileHeader_t), SEEK_SET);
            s32ReadSize = read(playfd, pTmpBuf, writeBufferSize);
            // bAoExit     = TRUE;
            MI_AO_Stop(AoDevId);
        }

        s32Ret = MI_AO_Write(AoDevId, pTmpBuf, s32ReadSize, 0, 300);
        if (s32Ret != MI_SUCCESS)
        {
            printf("Failed to call MI_AO_Write of Ao DevICE %d , error is 0x%x.\n", AoDevId, s32Ret);
        }
    }

    return MI_SUCCESS;
}

MI_S32 __SetLowFpsAWBParam(ST_Common_AovHandle_t *pstAovHandle)
{
    MI_S32                      s32Ret = MI_SUCCESS;
    MI_ISP_AWB_RunPeriodParam_t stRunPeriodParam;
    MI_ISP_AWB_AttrType_t       stAwbAttr;
    MI_S32                      u32IspDevId = pstAovHandle->stAovPipeAttr.stChnPortIsp[0].u32DevId;
    MI_S32                      u32IspChnId = pstAovHandle->stAovPipeAttr.stChnPortIsp[0].u32ChnId;

    memset(&stRunPeriodParam, 0x00, sizeof(MI_ISP_AWB_RunPeriodParam_t));
    memset(&stAwbAttr, 0x0, sizeof(MI_ISP_AWB_AttrType_t));

    stRunPeriodParam.u8Period = 1;
    CHECK_AOV_RESULT(MI_ISP_AWB_SetRunPeriod(u32IspDevId, u32IspChnId, &stRunPeriodParam), s32Ret, EXIT);

    CHECK_AOV_RESULT(MI_ISP_AWB_GetAttr(u32IspDevId, u32IspChnId, &stAwbAttr), s32Ret, EXIT);
    stAwbAttr.stAutoParaAPI.u8Speed = 80; // max
    CHECK_AOV_RESULT(MI_ISP_AWB_SetAttr(u32IspDevId, u32IspChnId, &stAwbAttr), s32Ret, EXIT);

EXIT:
    return s32Ret;
}

MI_S32 __SetHightFpsAWBParam(ST_Common_AovHandle_t *pstAovHandle)
{
    MI_S32                      s32Ret = MI_SUCCESS;
    MI_ISP_AWB_RunPeriodParam_t stRunPeriodParam;
    MI_ISP_AWB_AttrType_t       stAwbAttr;
    MI_S32                      u32IspDevId = pstAovHandle->stAovPipeAttr.stChnPortIsp[0].u32DevId;
    MI_S32                      u32IspChnId = pstAovHandle->stAovPipeAttr.stChnPortIsp[0].u32ChnId;

    memset(&stRunPeriodParam, 0x00, sizeof(MI_ISP_AWB_RunPeriodParam_t));
    memset(&stAwbAttr, 0x0, sizeof(MI_ISP_AWB_AttrType_t));

    stRunPeriodParam.u8Period = 3;
    CHECK_AOV_RESULT(MI_ISP_AWB_SetRunPeriod(u32IspDevId, u32IspChnId, &stRunPeriodParam), s32Ret, EXIT);

    CHECK_AOV_RESULT(MI_ISP_AWB_GetAttr(u32IspDevId, u32IspChnId, &stAwbAttr), s32Ret, EXIT);
    stAwbAttr.stAutoParaAPI.u8Speed = 20;
    CHECK_AOV_RESULT(MI_ISP_AWB_SetAttr(u32IspDevId, u32IspChnId, &stAwbAttr), s32Ret, EXIT);

EXIT:
    return s32Ret;
}

MI_S32 __TurnOnIRLed(ST_Common_AovHandle_t *pstAovHandle)
{
    MI_S32 s32Ret = MI_SUCCESS;

    if (pstAovHandle->stLightMiscCtlParam.stAovLightCtlParam.stLightCtlAttr.controlType != E_CONTROL_TYPE_LONG_TERM_ON)
    {
        pstAovHandle->stLightMiscCtlParam.stAovLightCtlParam.stLightCtlAttr.controlType = E_CONTROL_TYPE_LONG_TERM_ON;

        Dev_Light_Misc_Device_Set_Attr(pstAovHandle->stLightMiscCtlParam.fd,
                                       &pstAovHandle->stLightMiscCtlParam.stAovLightCtlParam.stLightCtlAttr);

        printf("switch ir led state to E_CONTROL_TYPE_LONG_TERM_ON\n");
    }

    return s32Ret;
}

MI_S32 __TurnOffIRLed(ST_Common_AovHandle_t *pstAovHandle)
{
    MI_S32 s32Ret = MI_SUCCESS;

    if (pstAovHandle->stLightMiscCtlParam.stAovLightCtlParam.stLightCtlAttr.controlType != E_CONTROL_TYPE_LONG_TERM_OFF)
    {
        pstAovHandle->stLightMiscCtlParam.stAovLightCtlParam.stLightCtlAttr.controlType = E_CONTROL_TYPE_LONG_TERM_OFF;

        Dev_Light_Misc_Device_Set_Attr(pstAovHandle->stLightMiscCtlParam.fd,
                                       &pstAovHandle->stLightMiscCtlParam.stAovLightCtlParam.stLightCtlAttr);

        printf("switch ir led state to E_CONTROL_TYPE_LONG_TERM_OFF\n");
    }

    return s32Ret;
}

MI_S32 __TurnMultiFrameIRLed(ST_Common_AovHandle_t *pstAovHandle)
{
    MI_S32 s32Ret = MI_SUCCESS;

    if (pstAovHandle->stLightMiscCtlParam.stAovLightCtlParam.stLightCtlAttr.controlType != E_CONTROL_TYPE_MULTI_FRAME)
    {
        pstAovHandle->stLightMiscCtlParam.stAovLightCtlParam.stLightCtlAttr.controlType = E_CONTROL_TYPE_MULTI_FRAME;

        Dev_Light_Misc_Device_Set_Attr(pstAovHandle->stLightMiscCtlParam.fd,
                                       &pstAovHandle->stLightMiscCtlParam.stAovLightCtlParam.stLightCtlAttr);

        printf("switch ir led state to E_CONTROL_TYPE_MULTI_FRAME\n");
    }

    return s32Ret;
}

MI_S32 __TurnOnIRCut(ST_Common_AovHandle_t *pstAovHandle)
{
    MI_S32 s32Ret = MI_SUCCESS;

    if (pstAovHandle->stLightMiscCtlParam.stAovLightCtlParam.eSwitchState != E_SWITCH_STATE_ON)
    {
        pstAovHandle->stLightMiscCtlParam.stAovLightCtlParam.eSwitchState = E_SWITCH_STATE_ON;

        Dev_Light_Misc_Device_Set_Ircut(pstAovHandle->stLightMiscCtlParam.fd,
                                        pstAovHandle->stLightMiscCtlParam.stAovLightCtlParam.eSwitchState);

        printf("switch ir cut state to E_SWITCH_STATE_ON\n");

        usleep(1000 * 100);

        Dev_Light_Misc_Device_Set_Ircut(pstAovHandle->stLightMiscCtlParam.fd, E_SWITCH_STATE_KEEP);

        printf("switch ir cut state to E_SWITCH_STATE_KEEP(ON)\n");
    }

    return s32Ret;
}

MI_S32 __TurnOffIRCut(ST_Common_AovHandle_t *pstAovHandle)
{
    MI_S32 s32Ret = MI_SUCCESS;

    if (pstAovHandle->stLightMiscCtlParam.stAovLightCtlParam.eSwitchState != E_SWITCH_STATE_OFF)
    {
        pstAovHandle->stLightMiscCtlParam.stAovLightCtlParam.eSwitchState = E_SWITCH_STATE_OFF;

        Dev_Light_Misc_Device_Set_Ircut(pstAovHandle->stLightMiscCtlParam.fd,
                                        pstAovHandle->stLightMiscCtlParam.stAovLightCtlParam.eSwitchState);

        printf("switch ir cut state to E_SWITCH_STATE_OFF\n");

        usleep(1000 * 100);

        Dev_Light_Misc_Device_Set_Ircut(pstAovHandle->stLightMiscCtlParam.fd, E_SWITCH_STATE_KEEP);

        printf("switch ir cut state to E_SWITCH_STATE_KEEP(OFF)\n");
    }

    return s32Ret;
}

MI_S32 __SetAEManualMode(ST_Common_AovHandle_t *pstAovHandle, MI_ISP_AE_ExpoValueType_t *data)
{
    MI_S32               s32Ret      = MI_SUCCESS;
    MI_ISP_AE_ModeType_e eAEMode     = E_SS_AE_MODE_M;
    MI_S32               u32IspDevId = pstAovHandle->stAovPipeAttr.stChnPortIsp[0].u32DevId;
    MI_S32               u32IspChnId = pstAovHandle->stAovPipeAttr.stChnPortIsp[0].u32ChnId;

    printf("Set AE manual mode\n");

    MI_ISP_AE_SetManualExpo(u32IspDevId, u32IspChnId, data);
    MI_ISP_AE_SetExpoMode(u32IspDevId, u32IspChnId, &eAEMode);

    pstAovHandle->stAovSWLightSensorParam.bFlagTurnIRLedCut = FALSE;

    return s32Ret;
}

MI_S32 __UpdataLightParam(ST_Common_AovHandle_t *pstAovHandle)
{
    int                        lux        = -1;
    MI_U8                      index      = 0;
    MI_U8                      i          = 0;
    MI_U16                     min        = 0xFFFF;
    MI_S32                     s32TigMode = pstAovHandle->stLightMiscCtlParam.stAovHWLightSensorParam.s32TigMode;
    MI_VIF_ShutterGainParams_t stSnrParam;

    lux = Dev_Light_Misc_Device_Get_LightSensor_Value(pstAovHandle->stLightMiscCtlParam.fd);
    if (lux == ERR_HW_GETLUX_FAILED)
    {
        printf("get lux failed\n");
        return -1;
    }
    else if (lux == ERR_HW_GETLUX_NOUPDATE)
    {
        printf("the lux unupdate\n");
        return -1;
    }

    if ((DIFF(lux, pstAovHandle->stLightMiscCtlParam.stAovHWLightSensorParam.lastLux)
         < pstAovHandle->stLightMiscCtlParam.stAovHWLightSensorParam.u32DiffLux))
    {
        pstAovHandle->stLightMiscCtlParam.stAovHWLightSensorParam.bUpdateLight = FALSE;
        goto EXIT;
    }

    printf("Update light param while Lux diff large\n");
    for (i = 0; i < (sizeof(g_stLightTable[0]) / sizeof(g_stLightTable[0][0])); i++)
    {
        if (DIFF(lux, g_stLightTable[s32TigMode][i].lux) < min)
        {
            min   = DIFF(lux, g_stLightTable[s32TigMode][i].lux);
            index = i;
        }
    }

    stSnrParam.u32ShutterTimeUs = g_stLightTable[s32TigMode][index].u32Shutter;
    stSnrParam.u32AeGain        = g_stLightTable[s32TigMode][index].u32Sensorgain;
    for (MI_U32 i = 0; i < pstAovHandle->stAovPipeAttr.u32SensorNum; i++)
    {
        MI_VIF_CustFunction(pstAovHandle->stAovPipeAttr.stVifParam[i].u32VifDevId,
                                E_MI_VIF_CUSTCMD_SHUTTER_GAIN_SET, sizeof(MI_VIF_ShutterGainParams_t),
                               &stSnrParam);
    }
    pstAovHandle->stLightMiscCtlParam.stAovHWLightSensorParam.u8LastIndex  = index;
    pstAovHandle->stLightMiscCtlParam.stAovHWLightSensorParam.bUpdateLight = TRUE;

EXIT:
    printf(
        "table_lux:%d, shutter:%d, gain:%d, cur_lux:%d, LastLux:%d, tigmode:%d\n",
        g_stLightTable[s32TigMode][pstAovHandle->stLightMiscCtlParam.stAovHWLightSensorParam.u8LastIndex].lux,
        g_stLightTable[s32TigMode][pstAovHandle->stLightMiscCtlParam.stAovHWLightSensorParam.u8LastIndex].u32Shutter,
        g_stLightTable[s32TigMode][pstAovHandle->stLightMiscCtlParam.stAovHWLightSensorParam.u8LastIndex].u32Sensorgain,
        lux, pstAovHandle->stLightMiscCtlParam.stAovHWLightSensorParam.lastLux, s32TigMode);

    pstAovHandle->stLightMiscCtlParam.stAovHWLightSensorParam.lastLux = lux;

    return MI_SUCCESS;
}

MI_U32 __riu_init(void)
{
    MI_U8 *        map_base = NULL;
    MSYS_MMIO_INFO info;
    memset(&info, 0x0, sizeof(MSYS_MMIO_INFO));

    /*  RIU Base mapping */
    FILL_VERCHK_TYPE(info, info.VerChk_Version, info.VerChk_Size, IOCTL_MSYS_VERSION);

    u32MsysFd = open("/dev/msys", O_RDWR | O_SYNC);
    if (u32MsysFd == -1)
    {
        printf("can't open /dev/msys\n");
        return -1;
    }
    u32MemFd = open("/dev/mem", O_RDWR | O_SYNC);
    if (u32MemFd == -1)
    {
        printf("can't open /dev/mem\n");
        close(u32MsysFd);
        return -1;
    }

    if (0 != ioctl(u32MsysFd, IOCTL_MSYS_GET_RIU_MAP, &info))
    {
        printf("DMEM request failed!!\n");
        close(u32MsysFd);
        close(u32MemFd);
        return -1;
    }

    map_base = mmap(NULL, info.size, PROT_READ | PROT_WRITE, MAP_SHARED, u32MemFd, info.addr);
    if (map_base == 0)
    {
        printf("mmap failed. NULL pointer!\n");
        close(u32MsysFd);
        close(u32MemFd);
        return -1;
    }
    u8pMapBase = map_base;
    return 0;
}

void __riu_close(void)
{
    /* Close application */
    if (u8pMapBase != NULL)
    {
        munmap(u8pMapBase, 0xff);
    }

    if (u32MsysFd != -1)
    {
        close(u32MsysFd);
    }

    if (u32MemFd != -1)
    {
        close(u32MemFd);
    }
}

void __riu_write(MI_U32 u32bank, MI_U32 u32offset, MI_U32 u32val)
{
    if (u32bank != 0x34) // only support rtc bank write
    {
        printf("error: no support bank! \n");
        return;
    }
    MI_U32 u32addr;
    printf("BANK:0x%04X 16bit-offset 0x%02X\n", u32bank, u32offset);
    u32addr            = (MI_U32)(u8pMapBase + u32bank * 0x200 + u32offset * 4);
    *(MI_U16 *)u32addr = u32val;
    u32val             = *(MI_U16 *)u32addr;
    printf("0x%04X\n", (MI_U16)u32val);
}

MI_U32 __riu_read(MI_U32 u32bank, MI_U32 u32offset)
{
    MI_U32 u32addr;
    MI_U32 u32val;
    printf("BANK:0x%04X 16bit-offset 0x%02X\n", u32bank, u32offset);
    u32addr = (MI_U32)(u8pMapBase + u32bank * 0x200 + u32offset * 4);
    u32val  = *(MI_U16 *)u32addr;
    printf("0x%04X\n", (unsigned short)u32val);

    return u32val;
}

void __riu_w(MI_U32 u32bank, MI_U32 u32offset, u16 u32val)
{
    __riu_init();
    __riu_write(u32bank, u32offset, u32val);
    __riu_close();
    usleep(8 * 1000); // must be delay if continuous write
}

// just for debug: check freeze timeout
MI_S32 __FreezeTimeoutCheck(void)
{
    MI_S32         s32Ret                         = MI_SUCCESS;
    static MI_BOOL bFirstSet                      = TRUE;
    static char    au8CurrentFailedFreezeCnt[128] = {'\0'};
    static char    au8LastFailedFreezeCnt[128]    = {'\0'};
    MI_S32         fd;

    if (TRUE == bFirstSet)
    {
        bFirstSet = FALSE;

        s32Ret = __WriteFile(PM_FREEZE_TIMEOUT, PM_FREEZE_EXPECT_TIME);

        fd = open(FAILED_FREEZE, O_RDONLY);
        if (fd >= 0)
        {
            lseek(fd, 0, SEEK_SET);

            read(fd, au8CurrentFailedFreezeCnt, sizeof(au8CurrentFailedFreezeCnt));

            lseek(fd, 0, SEEK_SET);

            close(fd);

            memcpy(au8LastFailedFreezeCnt, au8CurrentFailedFreezeCnt, sizeof(au8LastFailedFreezeCnt));
        }
        else
        {
            ST_INFO(FAILED_FREEZE "open fail\n");
            return -1;
        }
    }
    else
    {
        fd = open(FAILED_FREEZE, O_RDONLY);
        if (fd >= 0)
        {
            lseek(fd, 0, SEEK_SET);

            read(fd, au8CurrentFailedFreezeCnt, sizeof(au8CurrentFailedFreezeCnt));

            lseek(fd, 0, SEEK_SET);

            close(fd);

            s32Ret = strncmp(au8LastFailedFreezeCnt, au8CurrentFailedFreezeCnt, sizeof(au8CurrentFailedFreezeCnt));
            if (MI_SUCCESS != s32Ret)
            {
                ST_INFO("Failed_freeze increased\n");
                memcpy(au8LastFailedFreezeCnt, au8CurrentFailedFreezeCnt, sizeof(au8LastFailedFreezeCnt));
            }
        }
        else
        {
            ST_INFO(FAILED_FREEZE "open fail\n");
            return -1;
        }
    }

    return s32Ret;
}


MI_BOOL __AovStressChkMnt(void)
{
    bool bResult = FALSE;

    FILE *fp = popen("mount | grep '/mnt'", "r");
    if (fp == NULL)
    {
        perror("popen");
        exit(EXIT_FAILURE);
    }

    char buffer[1024];
    while (fgets(buffer, sizeof(buffer), fp) != NULL)
    {
        if (strstr(buffer, "nfs") || strstr(buffer, "cifs"))
        {
            bResult = TRUE;
            break;
        }
    }
    pclose(fp);
    return bResult;
}

MI_BOOL __AovStressChkFile(const char *pFile)
{
    bool bResult = (access(pFile, F_OK) != 0) ? FALSE : TRUE;
    if (!bResult) fprintf(stderr, "Error checking file[%s]: %s\n", pFile, strerror(errno));

    return bResult;
}

MI_U32 __AovStressGetFileSize(const char *pFile)
{
    struct stat file_stat;

    if (stat(pFile, &file_stat) != 0)
    {
        perror("Error getting file status");
        return -1;
    }

    return file_stat.st_size;
}

MI_S32 __AovStressExecSysCmd(const char *pCmd)
{
    MI_S32 s32Ret = system(pCmd);

    if (s32Ret != 0)
    {
        fprintf(stderr, "Error executing command: %s\n", pCmd);
        return s32Ret;
    }
    printf("%s:'%s'\n", __FUNCTION__, pCmd);

    return s32Ret;
}

MI_S32 __AovStressCpBackupLog(const char *pSrcFile, const char *pDestDir)
{
    MI_S32 s32Ret = MI_SUCCESS;
    char cmd[128];
    char fname[32];
    char fpath[64];
    time_t t = time(NULL);
    struct tm *tm = localtime(&t);

    strftime(fname, sizeof(fname), "kmsg_%Y%m%d%H%M%S.txt", tm);
    snprintf(cmd, sizeof(cmd), "cp %s %s/%s", pSrcFile, pDestDir, fname);
    s32Ret = __AovStressExecSysCmd(cmd);

    snprintf(fpath, sizeof(fpath), "%s/%s", pDestDir, fname);
    printf("backup kmsg file(%s) size: %d bytes\n", fpath, __AovStressGetFileSize(fpath));

    return s32Ret;
}


MI_BOOL __AovStressProcessSta(const char *pProcessName)
{
    //FIX ME
    return FALSE;
}

MI_S32 __AovStressRestartBkgProcess(const char *pProcessName, const char *pProcKmsg, const char *pKmsgFile)
{
    MI_S32 s32Ret = MI_SUCCESS;

    if (!__AovStressProcessSta(pProcessName))
    {
        char cmd[256];
        memset(cmd, 0x0, sizeof(cmd));
        snprintf(cmd, sizeof(cmd), "rm -rf %s", pKmsgFile);
        s32Ret = __AovStressExecSysCmd(cmd);
        if (s32Ret != MI_SUCCESS)
        {
            printf("delete %s fail!\n", pKmsgFile);
            s32Ret = -1;
        }
        memset(cmd, 0x0, sizeof(cmd));
        snprintf(cmd, sizeof(cmd), "%s > %s &", pProcKmsg, pKmsgFile);
        s32Ret &= __AovStressExecSysCmd(cmd);
    }
    else
    {
        printf("Background process is already running.\n");
        s32Ret = -1;
    }

    return s32Ret;
}

pid_t __AovStressFindPid(const char *pProcessName)
{
    char cmd[256];

    memset(cmd, 0x0, sizeof(cmd));
    snprintf(cmd, sizeof(cmd), "pgrep -f '%s'", pProcessName);

    FILE *fp = popen(cmd, "r");
    if (fp == NULL)
    {
        perror("popen");
        return -1;
    }
    pid_t pgid;
    if (fscanf(fp, "%d", &pgid) != 1)
    {
        perror("Failed to find pgid");
        pgid = -1;
    }
    pclose(fp);
    printf("%s:pgid=%d\n", __FUNCTION__, pgid);
    return pgid;
}

MI_S32 ST_Common_AovStressBackupKmsg(const char *pProcessName, const char *pProcKmsg, const char *pSrcFile, const char *pDestDir)
{
    MI_S32 s32Ret = MI_SUCCESS;
    MI_U32 u32FileSize = 0;

    if (!__AovStressChkFile(pSrcFile))
    {
        printf("No src file:%s.\n", pSrcFile);
        return -1;
    }

    u32FileSize = __AovStressGetFileSize(pSrcFile);
    if (u32FileSize < STRESS_KMSG_BACKUP_SIZE)
    {
        printf("%s: Not yet at backup size, continue... cur/bak(bytes) = %d/%d\n", pSrcFile, u32FileSize, STRESS_KMSG_BACKUP_SIZE);
        return -1;
    }

    if (!__AovStressChkMnt())
    {
        printf("No /mnt path, The [%s] will fill up and eventually crash out of memory.\n", pSrcFile);
        return -1;
    }

    int retry = STRESS_KILL_PROCESS_RETRY_CNT + 1;
    pid_t pgid = __AovStressFindPid(pProcessName);
    if (pgid == -1) {
        fprintf(stderr, "Failed to find process group ID\n");
        return -1;
    }

    kill(pgid, SIGTERM);
    while ((kill(pgid, 0) == 0) && --retry)
    {
        sleep(1);
        printf("kill pgid[%d] ...\n", pgid);
        kill(pgid, SIGKILL);
    }

    if (retry < (STRESS_KILL_PROCESS_RETRY_CNT) && retry > 0)
    {
        fprintf(stderr, "errno: %d\n", errno);
        printf("kill pgid[%d] succ! retry:%d\n", pgid, STRESS_KILL_PROCESS_RETRY_CNT - retry);
    }
    else if (retry <= 0)
    {
        fprintf(stderr, "errno: %d\n", errno);
        printf("kill pgid[%d] fail! retry:%d\n", pgid, STRESS_KILL_PROCESS_RETRY_CNT - retry);
    }
    else
    {
        fprintf(stderr, "errno: %d\n", errno);
        printf("kill pgid[%d] succ!\n", pgid);
    }

    s32Ret = __AovStressCpBackupLog(pSrcFile, pDestDir);
    if (s32Ret != MI_SUCCESS)
    {
        return -1;
    }

    s32Ret = __AovStressRestartBkgProcess(pProcessName, pProcKmsg, pSrcFile);
    if (s32Ret != MI_SUCCESS)
    {
        return -1;
    }

    return s32Ret;
}

MI_S32 ST_Common_AovStressExecSysCmd(const char *pSystemCmd)
{
    return __AovStressExecSysCmd(pSystemCmd);
}

MI_S32 ST_Common_AovSetSuspendTime(MI_U32 u32SuspendSec)
{
    MI_S32 s32Ret = MI_SUCCESS;
    char   u8SuspendParam[128];

    sprintf(u8SuspendParam, "%d %d", u32SuspendSec, ALARM_THRESHOLD_VALUE);
    s32Ret = __WriteFile(ALARM_THRESHOLD_DEVICE_NAME, u8SuspendParam);
    if (s32Ret != MI_SUCCESS)
    {
        return -1;
    }
    return MI_SUCCESS;
}

MI_S32 ST_Common_AovEnterSuspend(ST_Common_AovHandle_t *pstAovHandle)
{
    MI_S32        s32Ret      = MI_SUCCESS;
    static MI_U32 u32StrTimes = 0;

    if (FALSE == pstAovHandle->stAutoTestParam.bAutoTest)
    {
        s32Ret = __WriteFile(POWER_STATE, "mem");
        printf("\033[34m=>\n\033[0m");
        if (TRUE == pstAovHandle->bStressTest)
        {
            if (s32Ret != MI_SUCCESS)
            {
                printf("Enter suspend fail, will retry after 5s!");
                sleep(5);
                s32Ret = __WriteFile(POWER_STATE, "mem");
                if (s32Ret != MI_SUCCESS)
                {
                    printf("Retry enter suspend fail!");
                    return s32Ret;
                }
            }
        }
        else
        {
            if (s32Ret != MI_SUCCESS)
            {
                return s32Ret;
            }
        }
    }
    else if (TRUE == pstAovHandle->stAutoTestParam.bAutoTest)
    {
        if ('a' == *pstAovHandle->stAutoTestParam.au8KeyBoardInput)
        {
            memset(pstAovHandle->stAutoTestParam.au8KeyBoardInput, '*',
                   sizeof(pstAovHandle->stAutoTestParam.au8KeyBoardInput));

            u32StrTimes = 6;
        }

        __PrintToKmsg("[AUTO TEST]PM: app resume");
        if (u32StrTimes > 0)
        {
            u32StrTimes--;

            printf("\033[34m==>\n\033[0m");
            s32Ret = __WriteFile(POWER_STATE, "mem");
            printf("\033[0m");
            if (s32Ret != MI_SUCCESS)
            {
                return s32Ret;
            }
        }
        else
        {
            sleep(1);

            ST_Common_SNRSleepParam_t stSNRSleepParam;
            stSNRSleepParam.bSleepEnable = FALSE;
            for (MI_U32 i = 0; i < pstAovHandle->stAovPipeAttr.u32SensorNum; i++)
            {
                MI_VIF_CustFunction(pstAovHandle->stAovPipeAttr.stVifParam[i].u32VifDevId,
                                    E_MI_VIF_CUSTCMD_SLEEPPARAM_SET, sizeof(ST_Common_SNRSleepParam_t),
                                    &stSNRSleepParam);
            }

            pstAovHandle->stAovPipeAttr.eCurrentFps = E_ST_FPS_HIGH;
        }
    }

    __FreezeTimeoutCheck();
    ST_Common_AovSetQos(pstAovHandle);
    return MI_SUCCESS;
}

MI_S32 ST_Common_AovEnterLowPowerMode(ST_Common_AovHandle_t *pstAovHandle)
{
    ST_CHECK_POINTER(pstAovHandle);

    MI_S32 s32Ret = MI_SUCCESS;

    printf("Will enter the low power mode after next boot\n");

    s32Ret = __WriteFile(RTCPWC_FOR_OS, "1");
    if (s32Ret != MI_SUCCESS)
    {
        return -1;
    }

    s32Ret = __PowerOff(E_ST_SYSOFF_POWEROFF, pstAovHandle->stAutoTestParam.bAutoTest);

    return s32Ret;
}

MI_S32 ST_Common_AovEnterNormalPowerMode()
{
    MI_S32 s32Ret = MI_SUCCESS;

    printf("Will enter the normal power mode after next boot\n");

    s32Ret = __WriteFile(RTCPWC_FOR_OS, "3");
    if (s32Ret != MI_SUCCESS)
    {
        return -1;
    }

    s32Ret = __PowerOff(E_ST_SYSOFF_REBOOT, FALSE);

    return s32Ret;
}

ST_WakeupType_e ST_Common_AovWakeupCheck(ST_Common_AovHandle_t *pstAovHandle)
{
    ST_CHECK_POINTER(pstAovHandle);

    ST_WakeupType_e eWakeupType = -1;
    MI_S32          fd;
    char            au8WakeupEvent[128] = {'\0'};
    MI_BOOL         bAutoTest           = pstAovHandle->stAutoTestParam.bAutoTest;
    MI_BOOL         bUsingMcu           = pstAovHandle->bUsingMcu;

    if (TRUE == bAutoTest)
    {
        printf("When testing, default wake up type is the timer\n");
        eWakeupType = E_ST_WAKEUP_TIMER;
    }
    else
    {
        if (!bUsingMcu)
        {
            fd = open(WAKEUP_EVENT_FILE_NAME, O_RDONLY);
            if (fd >= 0)
            {
                lseek(fd, 0, SEEK_SET);

                read(fd, au8WakeupEvent, (sizeof(au8WakeupEvent) - 1));

                au8WakeupEvent[127] = '\0';

                lseek(fd, 0, SEEK_SET);

                close(fd);

                if (NULL != strstr(au8WakeupEvent, WAKEUP_EVENT_TIMER))
                {
                    printf("Wake up by timer\n");
                    eWakeupType = E_ST_WAKEUP_TIMER;
                }
                else if (NULL != strstr(au8WakeupEvent, WAKEUP_EVENT_PIR))
                {
                    printf("Wake up by PIR\n");
                    eWakeupType = E_ST_WAKEUP_PIR;
                }
                else if (NULL != strstr(au8WakeupEvent, WAKEUP_EVENT_PREVIEW))
                {
                    printf("Wake up by user preview\n");
                    eWakeupType = E_ST_WAKEUP_PREVIEW;
                }
                else
                {
                    ST_INFO("Wakeup type check fail\n");
                    return -1;
                }
            }
            else
            {
                ST_INFO(WAKEUP_EVENT_FILE_NAME "open fail\n");
                return -1;
            }
        }
        else
        {
            eWakeupType = __ST_Common_AovWakeupMcuCheck();
        }
    }

    return eWakeupType;
}

ST_BatteryLevel_e ST_Common_AovBatteryLevelCheck(MI_U8 *pu8SimulateCmd)
{
    // The provided API does not have a specific implementation.
    // Customers are required to complete the implementation of this API based on their specific needs.

    ST_CHECK_POINTER(pu8SimulateCmd);

    ST_OSType_e       eOSType;
    ST_BatteryLevel_e eBatteryLevel;

    eOSType = ST_Common_AovOSCheck();

    if (E_ST_OS_PURELINUX == eOSType)
    {
        eBatteryLevel = E_ST_BATTERYLEVEL_NORMAL;

        if ('c' == *pu8SimulateCmd)
        {
            printf("The battery level is low\n");
            eBatteryLevel   = E_ST_BATTERYLEVEL_LOW;
            *pu8SimulateCmd = '*';
        }
    }
    else
    {
        eBatteryLevel = E_ST_BATTERYLEVEL_LOW;

        if ('c' == *pu8SimulateCmd)
        {
            printf("The battery level is normal\n");
            eBatteryLevel   = E_ST_BATTERYLEVEL_NORMAL;
            *pu8SimulateCmd = '*';
        }
    }

    return eBatteryLevel;
}

ST_RemoteStatus_e ST_Common_AovRemoteStatusCheck(MI_U8 *pu8SimulateCmd)
{
    // The provided API does not have a specific implementation.
    // Customers are required to complete the implementation of this API based on their specific needs.

    ST_CHECK_POINTER(pu8SimulateCmd);

    ST_RemoteStatus_e eRemoteStatus = E_ST_REMOTE_CONNECTTING;

    if ('d' == *pu8SimulateCmd)
    {
        printf("The remote status was disconnected\n");
        eRemoteStatus   = E_ST_REMOTE_DISCONNECT;
        *pu8SimulateCmd = '*';
    }

    return eRemoteStatus;
}

MI_S32 ST_Common_AovSetQos(ST_Common_AovHandle_t *pstAovHandle)
{
    MI_S32 s32Ret = MI_SUCCESS;

    if (pstAovHandle->stAovPipeAttr.u32SensorNum == 2)
    {
        s32Ret = system("echo qos r VENC0_CODEC 2 > /sys/devices/system/miu/miu0/client");
        if (s32Ret != MI_SUCCESS)
        {
            printf("echo Qos Value error!\n");
        }
    }

    return s32Ret;
}

MI_S32 ST_Common_AovStreamCreate(ST_Common_AovStreamHandle_t *pstStreamHandle)
{
    ST_CHECK_POINTER(pstStreamHandle);

    pstStreamHandle->u32ListCnt    = 0;
    pstStreamHandle->u32StreamSize = 0;
    INIT_LIST_HEAD(&(pstStreamHandle->stStreamNode.list));
    pthread_mutex_init(&pstStreamHandle->ListMutex, NULL);

    return MI_SUCCESS;
}

MI_S32 ST_Common_AovStreamProduce(ST_Common_AovHandle_t *pstAovHandle, ST_Common_AovStreamHandle_t *pstStreamHandle)
{
    ST_CHECK_POINTER(pstStreamHandle);
    ST_CHECK_POINTER(pstAovHandle);

    MI_S32                     s32Ret = MI_SUCCESS;
    ST_Common_AovStreamNode_t *pstVencNewNode;
    MI_U32                     u32CurrentSNRFps;

    if (E_ST_FPS_LOW == pstAovHandle->stAovPipeAttr.eCurrentFps)
    {
        u32CurrentSNRFps = FPS_LOW;
    }
    else
    {
        u32CurrentSNRFps = FPS_HIGH;
    }

    if (pstStreamHandle->u32ListCnt >= pstAovHandle->stAovParam.u32WriteStreamTriggerNum)
    {
        printf("\033[33mConsume stream is too slow, causing the stream produce to pause\n\033[0m");

        while (pstStreamHandle->u32ListCnt > pstAovHandle->stAovParam.u32WriteStreamTriggerNum)
        {
            usleep(10 * 1000);
        }
    }

    if (TRUE == pstAovHandle->stFastAEParam.bEnableFastAE
            && pstAovHandle->stAovPipeAttr.eCurrentFps == E_ST_FPS_LOW)
    {
        ST_Common_FastAE_Run(&pstAovHandle->stFastAEParam.bIsDoFastAE);
    }

    for (MI_U32 i = 0; i < pstAovHandle->stAovPipeAttr.u32SensorNum; i++)
    {
        s32Ret = __AovGetVencStream(pstAovHandle, pstStreamHandle, &pstVencNewNode, u32CurrentSNRFps, i);
        if (s32Ret != MI_SUCCESS)
        {
            return s32Ret;
        }
    }

    return s32Ret;
}

MI_S32 ST_Common_AovStreamConsume(ST_Common_AovHandle_t *pstAovHandle, ST_Common_AovStreamHandle_t *pstStreamHandle)
{
    ST_CHECK_POINTER(pstAovHandle);
    ST_CHECK_POINTER(pstStreamHandle);

    MI_S32                     s32Ret = MI_SUCCESS;
    ST_Common_AovStreamNode_t *pstTmpStreamNode;
    static MI_U32              gu32FileSeq    = 0;
    static MI_U32              gu32CurrentFps = 1;
    static MI_U32              gu32LastFps    = 1;
    char                       u8DumpFileName[512];
    MI_U32                     u32SensorIndex = 0;
    FILE *                     fpoutSensor[AOV_MAX_SENSOR_NUM] = {};

    if (__AovStreamQueueIsEmpty(pstStreamHandle))
    {
        usleep(THREAD_SLEEP_TIME_US);
        return MI_SUCCESS;
    }

    __AovStreamQueueHeadNodeGet(pstStreamHandle, &pstTmpStreamNode);

    gu32CurrentFps = pstTmpStreamNode->u32Fps;
    sprintf(u8DumpFileName, "%s/%d_snr%d_st_%dfps.es", pstAovHandle->stAovParam.au8StreamDumpPath,
            (pstAovHandle->bStressTest ? 0 : gu32FileSeq),
            u32SensorIndex,
            (pstAovHandle->bStressTest ? 0 : pstTmpStreamNode->u32Fps));

    //FIX ME: Switching AOV to High Fps makes ES files unopenable as the first frame saved is not an I-frame. **Need to refactor the ES save logic**.
    //For stress tests, save to one ES file.
    if (FALSE == pstAovHandle->bDemonstrate)
    {
        if (gu32CurrentFps != gu32LastFps) gu32FileSeq++;

        for (MI_U32 i = 0; i < pstAovHandle->stAovPipeAttr.u32SensorNum; i++)
        {
            sprintf(u8DumpFileName, "%s/%d_snr%d_st_%dfps.es", pstAovHandle->stAovParam.au8StreamDumpPath,
                (pstAovHandle->bStressTest ? 0 : gu32FileSeq),
                (u32SensorIndex + i),
                (pstAovHandle->bStressTest ? 0 : pstTmpStreamNode->u32Fps));
            if (fpoutSensor[i] != NULL)
            {
                fclose(fpoutSensor[i]);
                fpoutSensor[i] = NULL;
            }
            fpoutSensor[i] = fopen(u8DumpFileName, "ab");
            if (fpoutSensor[i] == NULL)
            {
                printf("fopen erro\n");
                return -1;
            }
        }
    }

    while (TRUE == __AovStreamQueueHeadNodeGet(pstStreamHandle, &pstTmpStreamNode))
    {
        gu32CurrentFps = pstTmpStreamNode->u32Fps;

        if (FALSE == pstAovHandle->bDemonstrate)
        {
            if (gu32CurrentFps != gu32LastFps)
            {
                sprintf(u8DumpFileName, "%s/%d_snr%d_st_%dfps.es", pstAovHandle->stAovParam.au8StreamDumpPath,
                        (pstAovHandle->bStressTest ? 0 : gu32FileSeq), u32SensorIndex, (pstAovHandle->bStressTest ? 0 : pstTmpStreamNode->u32Fps));

                if (access(u8DumpFileName, F_OK) == -1)
                {
                    gu32FileSeq++;

                    for (MI_U32 i = 0; i < pstAovHandle->stAovPipeAttr.u32SensorNum; i++)
                    {
                        sprintf(u8DumpFileName, "%s/%d_snr%d_st_%dfps.es", pstAovHandle->stAovParam.au8StreamDumpPath,
                                (pstAovHandle->bStressTest ? 0 : gu32FileSeq), u32SensorIndex + i, (pstAovHandle->bStressTest ? 0 : pstTmpStreamNode->u32Fps));
                        if (fpoutSensor[i] != NULL)
                        {
                            fclose(fpoutSensor[i]);
                            fpoutSensor[i] = NULL;
                        }
                        fpoutSensor[i] = fopen(u8DumpFileName, "ab");
                        if (fpoutSensor[i] == NULL)
                        {
                            printf("fopen erro\n");
                            return -1;
                        }
                    }
                }
            }
        }

        CHECK_AOV_RESULT(__AovStreamQueueDelete(pstStreamHandle, pstTmpStreamNode), s32Ret, EXIT);

        if (FALSE == pstAovHandle->bDemonstrate)
        {
            if (pstTmpStreamNode->u32Index == SENSOR_0)
            {
                CHECK_AOV_RESULT(__AovWriteVencStream(pstAovHandle, pstTmpStreamNode, fpoutSensor[0]), s32Ret, EXIT);
                // Forcibly synchronize data to disk
                s32Ret = fsync(fileno(fpoutSensor[0])) != 0;
                if (s32Ret != 0)
                {
                    perror("Error syncing to disk");
                    fclose(fpoutSensor[0]);
                    fpoutSensor[0] = NULL;
                    return s32Ret;
                }
            }
            else if (pstTmpStreamNode->u32Index == SENSOR_1)
            {
                CHECK_AOV_RESULT(__AovWriteVencStream(pstAovHandle, pstTmpStreamNode, fpoutSensor[1]), s32Ret, EXIT);
                // Forcibly synchronize data to disk
                s32Ret = fsync(fileno(fpoutSensor[1])) != 0;
                if (s32Ret != 0)
                {
                    perror("Error syncing to disk");
                    fclose(fpoutSensor[1]);
                    fpoutSensor[1] = NULL;
                    return s32Ret;
                }
            }
        }
        CHECK_AOV_RESULT(__AovReleaseVencStream(pstAovHandle, pstTmpStreamNode, pstTmpStreamNode->u32Index), s32Ret,
                         EXIT);

        gu32LastFps = gu32CurrentFps;
    }

EXIT:
    if (FALSE == pstAovHandle->bDemonstrate)
    {
        for (MI_U32 i = 0; i < pstAovHandle->stAovPipeAttr.u32SensorNum; i++)
        {
            if (fpoutSensor[i] != NULL)
            {
                fclose(fpoutSensor[i]);
                fpoutSensor[i] = NULL;
            }
        }
    }

    return s32Ret;
}

MI_S32 ST_Common_AovDetect(ST_Common_AovHandle_t *pstAovHandle)
{
    ST_CHECK_POINTER(pstAovHandle);

    MI_S32         s32Ret = MI_SUCCESS;
    static MI_BOOL bFirst = TRUE;
    static MI_U32  gu32UnTargetFrameCnt;
    Box_t          astBoxes[MAX_DET_OBJECT];
    MI_S32         s32DetectedBoxsNum = 0;
    MI_S32         s32TargetBoxsNum   = 0;
    struct timeval stTimeoutVal;

    stTimeoutVal.tv_sec  = pstAovHandle->bStressTest ? STRESS_TIMEOUT_S_THREAD : 2;
    stTimeoutVal.tv_usec = 0;

    if (TRUE == bFirst)
    {
        bFirst               = FALSE;
        gu32UnTargetFrameCnt = pstAovHandle->stAovParam.u32DelayConfirmFrameNum;

        pstAovHandle->stAovPipeAttr.pstDetBufHandle =
            (ST_Common_DetBufHandle_t *)malloc(sizeof(ST_Common_DetBufHandle_t));
        memset(pstAovHandle->stAovPipeAttr.pstDetBufHandle, 0x0, sizeof(ST_Common_DetBufHandle_t));

        CHECK_AOV_RESULT(ST_Common_GetOutputBufInfo(pstAovHandle->stAovPipeAttr.stChnPortSclDetect[0],
                                                    &(pstAovHandle->stAovPipeAttr.pstDetBufHandle->stDetBufInfo),
                                                    &(pstAovHandle->stAovPipeAttr.pstDetBufHandle->hDetHandle),
                                                    &stTimeoutVal),
                         s32Ret, EXIT);
    }
    else
    {
        CHECK_AOV_RESULT(ST_Common_DET_GetTargetData(&(pstAovHandle->stAovPipeAttr.pDetHandle),
                                         &(pstAovHandle->stAovPipeAttr.pstDetBufHandle->stDetBufInfo),
                                         astBoxes, &s32DetectedBoxsNum),
             s32Ret, EXIT);

        CHECK_AOV_RESULT(ST_Common_PutOutputBufInfo(&(pstAovHandle->stAovPipeAttr.pstDetBufHandle->hDetHandle)), s32Ret,
                         EXIT);

        free(pstAovHandle->stAovPipeAttr.pstDetBufHandle);
        pstAovHandle->stAovPipeAttr.pstDetBufHandle = NULL;

        pstAovHandle->stAovPipeAttr.pstDetBufHandle =
            (ST_Common_DetBufHandle_t *)malloc(sizeof(ST_Common_DetBufHandle_t));
        memset(pstAovHandle->stAovPipeAttr.pstDetBufHandle, 0x0, sizeof(ST_Common_DetBufHandle_t));
        CHECK_AOV_RESULT(ST_Common_GetOutputBufInfo(pstAovHandle->stAovPipeAttr.stChnPortSclDetect[0],
                                                    &(pstAovHandle->stAovPipeAttr.pstDetBufHandle->stDetBufInfo),
                                                    &(pstAovHandle->stAovPipeAttr.pstDetBufHandle->hDetHandle),
                                                    &stTimeoutVal),
                         s32Ret, EXIT);

        CHECK_AOV_RESULT(__AovAttachFrame(pstAovHandle, astBoxes, s32DetectedBoxsNum), s32Ret, EXIT);

        for (MI_U32 i = 0; i < s32DetectedBoxsNum; i++)
        {
            if (DET_TARGETID == astBoxes[i].class_id)
            {
                s32TargetBoxsNum++;
            }
        }

        if (s32TargetBoxsNum > 0)
        {
            gu32UnTargetFrameCnt            = 0;
            pstAovHandle->eCurrentDetResult = E_ST_DET_DETECTED;
        }
        else
        {
            gu32UnTargetFrameCnt++;

            pstAovHandle->eCurrentDetResult = E_ST_DET_DELAYCONFIRM;

            if (gu32UnTargetFrameCnt >= pstAovHandle->stAovParam.u32DelayConfirmFrameNum)
            {
                gu32UnTargetFrameCnt            = pstAovHandle->stAovParam.u32DelayConfirmFrameNum;
                pstAovHandle->eCurrentDetResult = E_ST_DET_UNDETECTED;
            }
        }
    }

EXIT:
    return s32Ret;
}

MI_S32 ST_Common_AovDoAudio(ST_Common_AovHandle_t *pstAovHandle)
{
    ST_CHECK_POINTER(pstAovHandle);

    MI_S32           s32Ret       = MI_SUCCESS;
    static MI_S32    gAiFileCount = 0;
    char             u8AiFileName[512];
    MI_U32           u32MicDumpSize = 0;
    WaveFileHeader_t stAiWavHead    = {0};
    FILE *           fAi;
    WaveFileHeader_t stAoWavHeader;
    MI_U32           playfd;
    char *           pTmpBuf         = NULL;
    MI_U32           writeBufferSize = AO_WRITE_SIZE;

    /************************************************
    init audio
    *************************************************/
    MI_AO_Attr_t     stAoSetAttr;
    MI_SYS_ChnPort_t stSrcChnPort;
    MI_AI_Attr_t     stAiDevAttr;
    MI_AUDIO_DEV     stAoDevId = pstAovHandle->stAovPipeAttr.stAudioParam.stAoDevId;
    memset(&stAoSetAttr, 0x0, sizeof(MI_AO_Attr_t));
    memset(&stAiDevAttr, 0x0, sizeof(MI_AI_Attr_t));
    ST_Common_GetAiDefaultDevAttr(&stAiDevAttr);
    ST_Common_AiOpenDev(pstAovHandle->stAovPipeAttr.stAudioParam.stAiDevId, &stAiDevAttr);
    ST_Common_AiAttachIf(0, pstAovHandle->stAovPipeAttr.stAudioParam.stAiChnGrpId,
                         pstAovHandle->stAovPipeAttr.stAudioParam.enAiIf, 2);
    if (pstAovHandle->stAovPipeAttr.stAudioParam.bAoEnable)
    {
        ST_Common_GetAoDefaultDevAttr(&stAoSetAttr);
        STCHECKRESULT(ST_Common_AoOpenDev(stAoDevId, &stAoSetAttr));
        STCHECKRESULT(ST_Common_AoAttachIf(stAoDevId, pstAovHandle->stAovPipeAttr.stAudioParam.aenAoIf));
    }
    memset(&stSrcChnPort, 0x00, sizeof(MI_SYS_ChnPort_t));
    stSrcChnPort.eModId    = E_MI_MODULE_ID_AI;
    stSrcChnPort.u32DevId  = pstAovHandle->stAovPipeAttr.stAudioParam.stAiDevId;
    stSrcChnPort.u32ChnId  = pstAovHandle->stAovPipeAttr.stAudioParam.stAiChnGrpId;
    stSrcChnPort.u32PortId = 0;
    STCHECKRESULT(MI_SYS_SetChnOutputPortDepth(0, &stSrcChnPort, 3, 5));

    sprintf(u8AiFileName, "%s/%dst_ai_dump.wav", pstAovHandle->stAovPipeAttr.stAudioParam.au8AiFile, gAiFileCount++);
    fAi = fopen((char *)u8AiFileName, "w+");
    if (NULL == fAi)
    {
        printf("Failed to open output file [%s].\n", u8AiFileName);
        s32Ret = -1;
        goto EXIT;
    }

    // write header format
    CHECK_AOV_RESULT(__AudioAddWaveHeader(&stAiWavHead, E_SOUND_MODE_MONO, E_MI_AUDIO_SAMPLE_RATE_8000, u32MicDumpSize),
                     s32Ret, EXITAI);
    s32Ret = fwrite(&stAiWavHead, sizeof(WaveFileHeader_t), 1, fAi);
    if (s32Ret != 1)
    {
        printf("Failed to write dump wav head.\n");
        s32Ret = -1;
        goto EXITAI;
    }

    if (pstAovHandle->stAovPipeAttr.stAudioParam.bAoEnable)
    {
        // open play file
        playfd = open(pstAovHandle->stAovPipeAttr.stAudioParam.au8AoFile, O_RDONLY, 0666);
        if (playfd < 0)
        {
            printf("Failed to open input file error \n");
            s32Ret = -1;
            goto EXITAI;
        }

        // read header
        memset(&stAoWavHeader, 0, sizeof(WaveFileHeader_t));
        s32Ret = read(playfd, &stAoWavHeader, sizeof(WaveFileHeader_t));
        if (s32Ret < 0)
        {
            printf("Failed to read wav header.\n");
            s32Ret = -1;
            goto EXITAO;
        }

        pTmpBuf = malloc(writeBufferSize);
        if (NULL == pTmpBuf)
        {
            printf("Failed to alloc data buffer of file.\n");
            s32Ret = -1;
            goto EXITAO;
        }
        memset(pTmpBuf, 0, sizeof(writeBufferSize));
    }

    ST_Common_AiEnableChnGroup(pstAovHandle->stAovPipeAttr.stAudioParam.stAiDevId,
                               pstAovHandle->stAovPipeAttr.stAudioParam.stAiChnGrpId);

    while (pstAovHandle->stAovPipeAttr.stAudioParam.bAudioRun)
    {
        CHECK_AOV_RESULT(__AudioDumpAIData(fAi, &u32MicDumpSize, pstAovHandle->stAovPipeAttr.stAudioParam.stAiDevId,
                                           pstAovHandle->stAovPipeAttr.stAudioParam.stAiChnGrpId),
                         s32Ret, EXITAO);
        if (pstAovHandle->stAovPipeAttr.stAudioParam.bAoEnable)
        {
            CHECK_AOV_RESULT(
                __AudioPlayAoData(playfd, pTmpBuf, writeBufferSize, pstAovHandle->stAovPipeAttr.stAudioParam.stAoDevId),
                s32Ret, EXITAO);
        }
    }

    /************************************************
    deinit audio
    *************************************************/
    ST_Common_AiDisableChnGroup(pstAovHandle->stAovPipeAttr.stAudioParam.stAiDevId,
                                pstAovHandle->stAovPipeAttr.stAudioParam.stAiChnGrpId);
    ST_Common_AiCloseDev(pstAovHandle->stAovPipeAttr.stAudioParam.stAiDevId);
    if (pstAovHandle->stAovPipeAttr.stAudioParam.bAoEnable)
    {
        ST_Common_AoCloseDev(pstAovHandle->stAovPipeAttr.stAudioParam.stAoDevId);
    }

    // write data size in header
    CHECK_AOV_RESULT(__AudioAddWaveHeader(&stAiWavHead, E_SOUND_MODE_MONO, E_MI_AUDIO_SAMPLE_RATE_8000, u32MicDumpSize),
                     s32Ret, EXITAO);
    fseek(fAi, 0, SEEK_SET);
    s32Ret = fwrite(&stAiWavHead, sizeof(WaveFileHeader_t), 1, fAi);
    if (s32Ret != 1)
    {
        printf("Failed to write wav head.\n");
        s32Ret = -1;
        goto EXITAO;
    }
    s32Ret = MI_SUCCESS;
EXITAO:
    if (pstAovHandle->stAovPipeAttr.stAudioParam.bAoEnable)
    {
        close(playfd);
        if (pTmpBuf)
        {
            free(pTmpBuf);
        }
    }
EXITAI:
    fclose(fAi);
EXIT:
    return s32Ret;
}

MI_S32 ST_Common_AovISPAdjust_HWLightSensor(ST_Common_AovHandle_t *pstAovHandle)
{
    ST_CHECK_POINTER(pstAovHandle);

    MI_S32                       s32Ret = MI_SUCCESS;
    MI_ISP_AE_ExpoInfoType_t     stAeExpoInfo;
    MI_ISP_IQ_ColorToGrayType_t  stColorToGray;
    MI_S32                       u32IspDevId = pstAovHandle->stAovPipeAttr.stChnPortIsp[0].u32DevId;
    MI_S32                       u32IspChnId = pstAovHandle->stAovPipeAttr.stChnPortIsp[0].u32ChnId;
    MI_ISP_IQ_DaynightInfoType_t stDayNightInfo;

    pstAovHandle->stAovPipeAttr.eLastLight = pstAovHandle->stAovPipeAttr.eCurrentLight;

    memset(&stColorToGray, 0x0, sizeof(MI_ISP_IQ_ColorToGrayType_t));
    memset(&stAeExpoInfo, 0x00, sizeof(MI_ISP_AE_ExpoInfoType_t));
    memset(&stDayNightInfo, 0x0, sizeof(MI_ISP_IQ_DaynightInfoType_t));

    if (pstAovHandle->stAovPipeAttr.eCurrentFps == E_ST_FPS_LOW)
    {
        __UpdataLightParam(pstAovHandle);
        if (pstAovHandle->stLightMiscCtlParam.stAovHWLightSensorParam.lastLux > DAYNIGHT_LUX_THRESHOLD)
        {
            pstAovHandle->stAovPipeAttr.eCurrentLight = E_ST_LIGHT_BRIGHT;
        }
        else
        {
            pstAovHandle->stAovPipeAttr.eCurrentLight = E_ST_LIGHT_DARK;
        }
    }
    else
    {
        CHECK_AOV_RESULT(MI_ISP_AE_QueryExposureInfo(u32IspDevId, u32IspChnId, &stAeExpoInfo), s32Ret, EXIT);
        printf("LumY      = %-10d, SceneTarget = %-5d, bStable     = %-5d, bIsReachBoundary = %d\n",
               stAeExpoInfo.stHistWeightY.u32LumY, stAeExpoInfo.u32SceneTarget, stAeExpoInfo.bIsStable,
               stAeExpoInfo.bIsReachBoundary);

        if (stAeExpoInfo.s32BV > DAYNIGHT_BV_THRESHOLD)
        {
            pstAovHandle->stAovPipeAttr.eCurrentLight = E_ST_LIGHT_BRIGHT;
        }
        else
        {
            pstAovHandle->stAovPipeAttr.eCurrentLight = E_ST_LIGHT_DARK;
        }
    }

    // Switch different Iqbin、3A param according to different fps & ae
    if (((pstAovHandle->stAovPipeAttr.eCurrentLight != pstAovHandle->stAovPipeAttr.eLastLight)
         || (pstAovHandle->stAovPipeAttr.eCurrentFps != pstAovHandle->stAovPipeAttr.eLastFps))
        && (FALSE == pstAovHandle->stAutoTestParam.bAutoTest))
    {
        if (E_ST_LIGHT_BRIGHT == pstAovHandle->stAovPipeAttr.eCurrentLight)
        {
            if (strlen(pstAovHandle->stAovPipeAttr.au8IqApiBinBrightPath) != 0)
            {
                ST_Common_IspSetIqBin(u32IspDevId, u32IspChnId, pstAovHandle->stAovPipeAttr.au8IqApiBinBrightPath);
            }

            stColorToGray.bEnable = FALSE;
            MI_ISP_IQ_SetColorToGray(u32IspDevId, u32IspChnId, &stColorToGray);

            __TurnOnIRCut(pstAovHandle);

            __TurnOffIRLed(pstAovHandle);

            if (E_ST_FPS_HIGH == pstAovHandle->stAovPipeAttr.eCurrentFps)
            {
                __SetHightFpsAWBParam(pstAovHandle);
            }
            else if (E_ST_FPS_LOW == pstAovHandle->stAovPipeAttr.eCurrentFps)
            {
                __SetLowFpsAWBParam(pstAovHandle);
            }
        }
        else if (E_ST_LIGHT_DARK == pstAovHandle->stAovPipeAttr.eCurrentLight)
        {
            if (strlen(pstAovHandle->stAovPipeAttr.au8IqApiBinDarkPath) != 0)
            {
                ST_Common_IspSetIqBin(u32IspDevId, u32IspChnId, pstAovHandle->stAovPipeAttr.au8IqApiBinDarkPath);
            }

            stColorToGray.bEnable = TRUE;
            MI_ISP_IQ_SetColorToGray(u32IspDevId, u32IspChnId, &stColorToGray);

            __TurnOffIRCut(pstAovHandle);

            if (E_ST_FPS_HIGH == pstAovHandle->stAovPipeAttr.eCurrentFps)
            {
                __SetHightFpsAWBParam(pstAovHandle);

                __TurnOnIRLed(pstAovHandle);
            }
            else if (E_ST_FPS_LOW == pstAovHandle->stAovPipeAttr.eCurrentFps)
            {
                __SetLowFpsAWBParam(pstAovHandle);

                __TurnMultiFrameIRLed(pstAovHandle);
            }
        }
    }

EXIT:
    return s32Ret;
}

MI_S32 ST_Common_AovISPAdjust_SWLightSensor(ST_Common_AovHandle_t *pstAovHandle)
{
    ST_CHECK_POINTER(pstAovHandle);

    MI_S32                      s32Ret = MI_SUCCESS;
    MI_ISP_AE_ExpoInfoType_t    stAeExpoInfo;
    MI_ISP_IQ_ColorToGrayType_t stColorToGray;
    MI_S32                      u32IspDevId = pstAovHandle->stAovPipeAttr.stChnPortIsp[0].u32DevId;
    MI_S32                      u32IspChnId = pstAovHandle->stAovPipeAttr.stChnPortIsp[0].u32ChnId;

    static MI_U32 u32CurrentAECnt = 0;
    static MI_U32 u32LastAECnt    = 0;

    MI_ISP_IQ_DaynightInfoType_t stDayNightInfo;

    pstAovHandle->stAovPipeAttr.eLastLight = pstAovHandle->stAovPipeAttr.eCurrentLight;

    memset(&stColorToGray, 0x0, sizeof(MI_ISP_IQ_ColorToGrayType_t));
    memset(&stAeExpoInfo, 0x00, sizeof(MI_ISP_AE_ExpoInfoType_t));
    memset(&stDayNightInfo, 0x0, sizeof(MI_ISP_IQ_DaynightInfoType_t));

    // Wait AE update done
    u32LastAECnt = u32CurrentAECnt;
    printf("LastAECnt = %d\n", u32LastAECnt);
    while (1)
    {
        MI_ISP_CUS3A_GetDoAeCount(u32IspDevId, u32IspChnId, &u32CurrentAECnt);
        if (u32CurrentAECnt != u32LastAECnt)
        {
            printf("CurrentAECnt = %d\n", u32CurrentAECnt);
            break;
        }
        usleep(1000 * 1);
    }

    CHECK_AOV_RESULT(MI_ISP_AE_QueryExposureInfo(u32IspDevId, u32IspChnId, &stAeExpoInfo), s32Ret, EXIT);
    printf("LumY      = %-10d, SceneTarget = %-5d, bStable     = %-5d, bIsReachBoundary = %d\n",
           stAeExpoInfo.stHistWeightY.u32LumY, stAeExpoInfo.u32SceneTarget, stAeExpoInfo.bIsStable,
           stAeExpoInfo.bIsReachBoundary);

    pstAovHandle->stAovSWLightSensorParam.bLastRegardedStable =
        pstAovHandle->stAovSWLightSensorParam.bCurrentRegardedStable;

    if (E_ST_FPS_LOW == pstAovHandle->stAovPipeAttr.eCurrentFps)
    {
        if (FALSE == pstAovHandle->stAovSWLightSensorParam.bFlagTurnIRLedCut)
        {
            if (TRUE == stAeExpoInfo.bIsStable)
            {
                if (pstAovHandle->stAovSWLightSensorParam.u32StableFramesBeforeTurnIr < STABLEFRAMESBEFORETURNIR)
                {
                    pstAovHandle->stAovSWLightSensorParam.u32StableFramesBeforeTurnIr++;
                    printf("u32StableFramesBeforeTurnIr = %d\n",
                           pstAovHandle->stAovSWLightSensorParam.u32StableFramesBeforeTurnIr);
                }

                if ((pstAovHandle->stAovSWLightSensorParam.u32StableFramesBeforeTurnIr >= STABLEFRAMESBEFORETURNIR))
                {
                    CHECK_AOV_RESULT(MI_ISP_IQ_QueryDayNightInfo(u32IspDevId, u32IspChnId, &stDayNightInfo), s32Ret,
                                     EXIT);

                    if (E_ST_LIGHT_BRIGHT == pstAovHandle->stAovPipeAttr.eCurrentLight)
                    {
                        if (TRUE == stDayNightInfo.bD2N)
                        {
                            printf("Check BV: stDayNightInfo.bD2N = TRUE\n");
                            pstAovHandle->stAovPipeAttr.eCurrentLight = E_ST_LIGHT_DARK;

                            if (strlen(pstAovHandle->stAovPipeAttr.au8IqApiBinDarkPath) != 0)
                            {
                                ST_Common_IspSetIqBin(u32IspDevId, u32IspChnId,
                                                      pstAovHandle->stAovPipeAttr.au8IqApiBinDarkPath);
                            }

                            stColorToGray.bEnable = TRUE;
                            MI_ISP_IQ_SetColorToGray(u32IspDevId, u32IspChnId, &stColorToGray);

                            __TurnOffIRCut(pstAovHandle);

                            __TurnOnIRLed(pstAovHandle);

                            pstAovHandle->stAovSWLightSensorParam.bFlagTurnIRLedCut = TRUE;
                        }
                        else
                        {
                            pstAovHandle->stAovSWLightSensorParam.bCurrentRegardedStable = TRUE;

                            if (FALSE == pstAovHandle->stAovSWLightSensorParam.bLastRegardedStable)
                            {
                                __SetAEManualMode(pstAovHandle, &stAeExpoInfo.stExpoValueLong);
                            }

                            if (E_ST_LIGHT_DARK == pstAovHandle->stAovPipeAttr.eCurrentLight)
                            {
                                __TurnMultiFrameIRLed(pstAovHandle);
                            }
                        }
                    }
                    else
                    {
                        if (TRUE == stDayNightInfo.bN2D)
                        {
                            printf("Check AWB: stDayNightInfo.bN2D = TRUE\n");
                            pstAovHandle->stAovPipeAttr.eCurrentLight = E_ST_LIGHT_BRIGHT;

                            if (strlen(pstAovHandle->stAovPipeAttr.au8IqApiBinBrightPath) != 0)
                            {
                                ST_Common_IspSetIqBin(u32IspDevId, u32IspChnId,
                                                      pstAovHandle->stAovPipeAttr.au8IqApiBinBrightPath);
                            }

                            stColorToGray.bEnable = FALSE;
                            MI_ISP_IQ_SetColorToGray(u32IspDevId, u32IspChnId, &stColorToGray);

                            __TurnOnIRCut(pstAovHandle);

                            __TurnOffIRLed(pstAovHandle);

                            pstAovHandle->stAovSWLightSensorParam.bFlagTurnIRLedCut = TRUE;
                        }
                        else
                        {
                            pstAovHandle->stAovSWLightSensorParam.bCurrentRegardedStable = TRUE;

                            if (FALSE == pstAovHandle->stAovSWLightSensorParam.bLastRegardedStable)
                            {
                                __SetAEManualMode(pstAovHandle, &stAeExpoInfo.stExpoValueLong);
                            }

                            if (E_ST_LIGHT_DARK == pstAovHandle->stAovPipeAttr.eCurrentLight)
                            {
                                __TurnMultiFrameIRLed(pstAovHandle);
                            }
                        }
                    }
                }
            }
            else
            {
                pstAovHandle->stAovSWLightSensorParam.bCurrentRegardedStable = FALSE;

                if (E_ST_LIGHT_DARK == pstAovHandle->stAovPipeAttr.eCurrentLight)
                {
                    __TurnOnIRLed(pstAovHandle);
                }

                if (TRUE == pstAovHandle->stAovSWLightSensorParam.bLastRegardedStable)
                {
                    printf("Set AE auto mode & fast mode\n");
                    MI_ISP_AE_ModeType_e eAEMode = E_SS_AE_MODE_A;
                    MI_ISP_AE_Bool_e     eAEBool = E_SS_AE_TRUE;
                    MI_ISP_AE_SetFastMode(u32IspDevId, u32IspChnId, &eAEBool);
                    MI_ISP_AE_SetExpoMode(u32IspDevId, u32IspChnId, &eAEMode);

                    pstAovHandle->stAovSWLightSensorParam.u32StableFramesBeforeTurnIr = 0;
                    pstAovHandle->stAovSWLightSensorParam.u32StableFramesAfterTurnIr  = 0;
                }
            }
        }
        else
        {
            if (TRUE == stAeExpoInfo.bIsStable)
            {
                if (pstAovHandle->stAovSWLightSensorParam.u32StableFramesBeforeTurnIr < STABLEFRAMESBEFORETURNIR)
                {
                    pstAovHandle->stAovSWLightSensorParam.u32StableFramesBeforeTurnIr++;
                    printf("u32StableFramesBeforeTurnIr = %d\n",
                           pstAovHandle->stAovSWLightSensorParam.u32StableFramesBeforeTurnIr);
                }
                else
                {
                    pstAovHandle->stAovSWLightSensorParam.u32StableFramesAfterTurnIr++;
                    printf("u32StableFramesAfterTurnIr = %d\n",
                           pstAovHandle->stAovSWLightSensorParam.u32StableFramesAfterTurnIr);
                }

                if ((pstAovHandle->stAovSWLightSensorParam.u32StableFramesBeforeTurnIr >= STABLEFRAMESBEFORETURNIR)
                    && (pstAovHandle->stAovSWLightSensorParam.u32StableFramesAfterTurnIr >= STABLEFRAMESAFTERTURNIR))
                {
                    pstAovHandle->stAovSWLightSensorParam.bCurrentRegardedStable = TRUE;

                    if (FALSE == pstAovHandle->stAovSWLightSensorParam.bLastRegardedStable)
                    {
                        __SetAEManualMode(pstAovHandle, &stAeExpoInfo.stExpoValueLong);

                        pstAovHandle->stAovSWLightSensorParam.bFlagTurnIRLedCut = FALSE;
                    }

                    if (E_ST_LIGHT_DARK == pstAovHandle->stAovPipeAttr.eCurrentLight)
                    {
                        __TurnMultiFrameIRLed(pstAovHandle);
                    }
                }
            }
            else
            {
                pstAovHandle->stAovSWLightSensorParam.u32StableFramesAfterTurnIr = 0;
            }
        }
    }
    else
    {
        if (E_ST_LIGHT_DARK == pstAovHandle->stAovPipeAttr.eCurrentLight)
        {
            __TurnOnIRLed(pstAovHandle);
        }
    }

EXIT:
    return s32Ret;
}

MI_S32 ST_Common_AovSetLowFps(ST_Common_AovHandle_t *pstAovHandle)
{
    ST_CHECK_POINTER(pstAovHandle);

    MI_S32           s32Ret    = MI_SUCCESS;
    MI_BOOL          bDropFlag = FALSE;
    MI_SYS_ChnPort_t stSrcChnPort;

    if (E_ST_FPS_HIGH == pstAovHandle->stAovPipeAttr.eCurrentFps)
    {
        bDropFlag = TRUE;
    }

    // Drop ungeted stream while the frame rate is switched
    if (TRUE == bDropFlag)
    {
        for (MI_U32 i = 0; i < pstAovHandle->stAovPipeAttr.u32SensorNum; i++)
        {
            MI_SCL_StopChannel(pstAovHandle->stAovPipeAttr.stChnPortSclPreview[i].u32DevId,
                               pstAovHandle->stAovPipeAttr.stChnPortSclPreview[i].u32ChnId);
            __AovDropVencStream(pstAovHandle, i);
        }
    }

    pstAovHandle->stAovPipeAttr.eLastFps    = pstAovHandle->stAovPipeAttr.eCurrentFps;
    pstAovHandle->stAovPipeAttr.eCurrentFps = E_ST_FPS_LOW;

    if (TRUE == pstAovHandle->stAovSWLightSensorParam.bEnableSWLightSensor)
    {
        ST_Common_AovISPAdjust_SWLightSensor(pstAovHandle);
    }

    if (TRUE == pstAovHandle->stAovSWLightSensorParam.bCurrentRegardedStable)
    {
        if ((E_ST_FPS_HIGH == pstAovHandle->stAovPipeAttr.eLastFps)
            || (FALSE == pstAovHandle->stAovSWLightSensorParam.bLastRegardedStable))
        {
            __SetLowFpsAWBParam(pstAovHandle);
            for (MI_U32 i = 0; i < pstAovHandle->stAovPipeAttr.u32SensorNum; i++)
            {
                // Change the port depth according to different fps
                memset(&stSrcChnPort, 0x00, sizeof(MI_SYS_ChnPort_t));
                stSrcChnPort.eModId    = pstAovHandle->stAovPipeAttr.stChnPortSclDetect[i].eModId;
                stSrcChnPort.u32DevId  = pstAovHandle->stAovPipeAttr.stChnPortSclDetect[i].u32DevId;
                stSrcChnPort.u32ChnId  = pstAovHandle->stAovPipeAttr.stChnPortSclDetect[i].u32ChnId;
                stSrcChnPort.u32PortId = pstAovHandle->stAovPipeAttr.stChnPortSclDetect[i].u32PortId;
                CHECK_AOV_RESULT(MI_SYS_SetChnOutputPortDepth(0, &stSrcChnPort, 2, 2), s32Ret, EXIT);
            }
        }
        // Configure Sensor to enable sleep mode & consecutive frames cnt
        pstAovHandle->stSNRSleepParam.bSleepEnable           = TRUE;
        pstAovHandle->stSNRSleepParam.u32FrameCntBeforeSleep = 1;
        for (MI_U32 i = 0; i < pstAovHandle->stAovPipeAttr.u32SensorNum; i++)
        {
            MI_VIF_CustFunction(pstAovHandle->stAovPipeAttr.stVifParam[i].u32VifDevId,
                                    E_MI_VIF_CUSTCMD_SLEEPPARAM_SET, sizeof(ST_Common_SNRSleepParam_t),
                                    &pstAovHandle->stSNRSleepParam);
        }
    }

    if (pstAovHandle->stAovPipeAttr.eCurrentFps != pstAovHandle->stAovPipeAttr.eLastFps)
    {
        for (MI_U32 i = 0; i < pstAovHandle->stAovPipeAttr.u32SensorNum; i++)
        {
            CHECK_AOV_RESULT(MI_VENC_RequestIdr(pstAovHandle->stAovPipeAttr.stChnPortVenc[i].u32DevId,
                                                pstAovHandle->stAovPipeAttr.stChnPortVenc[i].u32ChnId, 0),
                             s32Ret, EXIT);
        }
    }

    if (TRUE == bDropFlag)
    {
        for (MI_U32 i = 0; i < pstAovHandle->stAovPipeAttr.u32SensorNum; i++)
        {
            MI_SCL_StartChannel(pstAovHandle->stAovPipeAttr.stChnPortSclPreview[i].u32DevId,
                                pstAovHandle->stAovPipeAttr.stChnPortSclPreview[i].u32ChnId);
        }
    }

EXIT:
    return s32Ret;
}

MI_S32 ST_Common_AovSetHighFps(ST_Common_AovHandle_t *pstAovHandle)
{
    ST_CHECK_POINTER(pstAovHandle);

    MI_S32           s32Ret = MI_SUCCESS;
    MI_SYS_ChnPort_t stSrcChnPort;

    pstAovHandle->stAovPipeAttr.eLastFps    = pstAovHandle->stAovPipeAttr.eCurrentFps;
    pstAovHandle->stAovPipeAttr.eCurrentFps = E_ST_FPS_HIGH;

    if (TRUE == pstAovHandle->stAovSWLightSensorParam.bEnableSWLightSensor)
    {
        ST_Common_AovISPAdjust_SWLightSensor(pstAovHandle);
    }

    if (E_ST_FPS_LOW == pstAovHandle->stAovPipeAttr.eLastFps)
    {
        __SetHightFpsAWBParam(pstAovHandle);

        // Configure Sensor to disable sleep mode
        pstAovHandle->stSNRSleepParam.bSleepEnable = FALSE;
        for (MI_U32 i = 0; i < pstAovHandle->stAovPipeAttr.u32SensorNum; i++)
        {
            MI_VIF_CustFunction(pstAovHandle->stAovPipeAttr.stVifParam[i].u32VifDevId, E_MI_VIF_CUSTCMD_SLEEPPARAM_SET,
                                sizeof(ST_Common_SNRSleepParam_t), &pstAovHandle->stSNRSleepParam);

            // Change the port depth according to different fps
            memset(&stSrcChnPort, 0x00, sizeof(MI_SYS_ChnPort_t));
            stSrcChnPort.eModId    = pstAovHandle->stAovPipeAttr.stChnPortSclDetect[i].eModId;
            stSrcChnPort.u32DevId  = pstAovHandle->stAovPipeAttr.stChnPortSclDetect[i].u32DevId;
            stSrcChnPort.u32ChnId  = pstAovHandle->stAovPipeAttr.stChnPortSclDetect[i].u32ChnId;
            stSrcChnPort.u32PortId = pstAovHandle->stAovPipeAttr.stChnPortSclDetect[i].u32PortId;
            CHECK_AOV_RESULT(MI_SYS_SetChnOutputPortDepth(0, &stSrcChnPort, 2, 4), s32Ret, EXIT);
        }
    }

    if (pstAovHandle->stAovPipeAttr.eCurrentFps != pstAovHandle->stAovPipeAttr.eLastFps)
    {
        for (MI_U32 i = 0; i < pstAovHandle->stAovPipeAttr.u32SensorNum; i++)
        {
            CHECK_AOV_RESULT(MI_VENC_RequestIdr(pstAovHandle->stAovPipeAttr.stChnPortVenc[i].u32DevId,
                                                pstAovHandle->stAovPipeAttr.stChnPortVenc[i].u32ChnId, 0),
                             s32Ret, EXIT);
        }
    }

EXIT:
    return s32Ret;
}

ST_OSType_e ST_Common_AovOSCheck()
{
    ST_OSType_e eOSType;

    if (MI_SUCCESS == access(IS_DUALOS, F_OK))
    {
        eOSType = E_ST_OS_DUALOS;
    }
    else
    {
        eOSType = E_ST_OS_PURELINUX;
    }

    return eOSType;
}

MI_S32 ST_Common_AovGetDefaultAttr(ST_Common_AovHandle_t *pstAovHandle)
{
    ST_CHECK_POINTER(pstAovHandle);

    MI_SNR_PlaneInfo_t stPlaneInfo;

    pstAovHandle->stAovPipeAttr.stChnPortIsp[0].eModId    = E_MI_MODULE_ID_ISP;
    pstAovHandle->stAovPipeAttr.stChnPortIsp[0].u32DevId  = 0;
    pstAovHandle->stAovPipeAttr.stChnPortIsp[0].u32ChnId  = 0;
    pstAovHandle->stAovPipeAttr.stChnPortIsp[0].u32PortId = 0;

    pstAovHandle->stAovPipeAttr.stChnPortSclPreview[0].eModId    = E_MI_MODULE_ID_SCL;
    pstAovHandle->stAovPipeAttr.stChnPortSclPreview[0].u32DevId  = 0;
    pstAovHandle->stAovPipeAttr.stChnPortSclPreview[0].u32ChnId  = 0;
    pstAovHandle->stAovPipeAttr.stChnPortSclPreview[0].u32PortId = 0;

    pstAovHandle->stAovPipeAttr.stChnPortSclDetect[0].eModId    = E_MI_MODULE_ID_SCL;
    pstAovHandle->stAovPipeAttr.stChnPortSclDetect[0].u32DevId  = 0;
    pstAovHandle->stAovPipeAttr.stChnPortSclDetect[0].u32ChnId  = 0;
    pstAovHandle->stAovPipeAttr.stChnPortSclDetect[0].u32PortId = 1;

    pstAovHandle->stAovPipeAttr.stChnPortVenc[0].eModId    = E_MI_MODULE_ID_VENC;
    pstAovHandle->stAovPipeAttr.stChnPortVenc[0].u32DevId  = 0;
    pstAovHandle->stAovPipeAttr.stChnPortVenc[0].u32ChnId  = 0;
    pstAovHandle->stAovPipeAttr.stChnPortVenc[0].u32PortId = 0;

    pstAovHandle->stAovPipeAttr.stChnPortRgnAttached[0].eModId    = E_MI_MODULE_ID_VENC;
    pstAovHandle->stAovPipeAttr.stChnPortRgnAttached[0].s32DevId  = 0;
    pstAovHandle->stAovPipeAttr.stChnPortRgnAttached[0].s32ChnId  = 0;
    pstAovHandle->stAovPipeAttr.stChnPortRgnAttached[0].s32PortId = 0;

    pstAovHandle->stAovPipeAttr.stVifParam[0].u8SensorIndex = 0xFF;
    pstAovHandle->stAovPipeAttr.stVifParam[0].u32SnrPadId   = 0;
    pstAovHandle->stAovPipeAttr.stVifParam[0].u32VifDevId   = 0;
    pstAovHandle->stAovPipeAttr.stVifParam[0].u32VifGroupId = 0;
    pstAovHandle->stAovPipeAttr.stVifParam[0].u32VifPortId  = 0;

    if (pstAovHandle->stAovPipeAttr.u32SensorNum == 2)
    {
        pstAovHandle->stAovPipeAttr.stChnPortIsp[1].eModId    = E_MI_MODULE_ID_ISP;
        pstAovHandle->stAovPipeAttr.stChnPortIsp[1].u32DevId  = 0;
        pstAovHandle->stAovPipeAttr.stChnPortIsp[1].u32ChnId  = 1;
        pstAovHandle->stAovPipeAttr.stChnPortIsp[1].u32PortId = 0;

        pstAovHandle->stAovPipeAttr.stChnPortSclPreview[1].eModId    = E_MI_MODULE_ID_SCL;
        pstAovHandle->stAovPipeAttr.stChnPortSclPreview[1].u32DevId  = 0;
        pstAovHandle->stAovPipeAttr.stChnPortSclPreview[1].u32ChnId  = 1;
        pstAovHandle->stAovPipeAttr.stChnPortSclPreview[1].u32PortId = 0;

        pstAovHandle->stAovPipeAttr.stChnPortSclDetect[1].eModId    = E_MI_MODULE_ID_SCL;
        pstAovHandle->stAovPipeAttr.stChnPortSclDetect[1].u32DevId  = 0;
        pstAovHandle->stAovPipeAttr.stChnPortSclDetect[1].u32ChnId  = 1;
        pstAovHandle->stAovPipeAttr.stChnPortSclDetect[1].u32PortId = 1;

        pstAovHandle->stAovPipeAttr.stChnPortVenc[1].eModId    = E_MI_MODULE_ID_VENC;
        pstAovHandle->stAovPipeAttr.stChnPortVenc[1].u32DevId  = 0;
        pstAovHandle->stAovPipeAttr.stChnPortVenc[1].u32ChnId  = 1;
        pstAovHandle->stAovPipeAttr.stChnPortVenc[1].u32PortId = 0;

        pstAovHandle->stAovPipeAttr.stChnPortRgnAttached[1].eModId    = E_MI_MODULE_ID_VENC;
        pstAovHandle->stAovPipeAttr.stChnPortRgnAttached[1].s32DevId  = 0;
        pstAovHandle->stAovPipeAttr.stChnPortRgnAttached[1].s32ChnId  = 1;
        pstAovHandle->stAovPipeAttr.stChnPortRgnAttached[1].s32PortId = 0;

        pstAovHandle->stAovPipeAttr.stVifParam[1].u8SensorIndex = 0xFF;
        pstAovHandle->stAovPipeAttr.stVifParam[1].u32SnrPadId   = 2;
        pstAovHandle->stAovPipeAttr.stVifParam[1].u32VifDevId   = 4;
        pstAovHandle->stAovPipeAttr.stVifParam[1].u32VifGroupId = 1;
        pstAovHandle->stAovPipeAttr.stVifParam[1].u32VifPortId  = 0;
    }
    pstAovHandle->stAovParam.u32SuspendTime = 1; // Second

    pstAovHandle->stAovPipeAttr.stAudioParam.stAiDevId    = 0;
    pstAovHandle->stAovPipeAttr.stAudioParam.stAoDevId    = 0;
    pstAovHandle->stAovPipeAttr.stAudioParam.stAiChnGrpId = 0;

    pstAovHandle->eCurrentWakeupType = E_ST_WAKEUP_TIMER;
    pstAovHandle->eLastWakeupType    = E_ST_WAKEUP_TIMER;
    pstAovHandle->eCurrentDetResult  = E_ST_DET_UNDETECTED;
    pstAovHandle->eLastDetResult     = E_ST_DET_UNDETECTED;

    pstAovHandle->stAovPipeAttr.eCurrentFps = E_ST_FPS_HIGH;
    pstAovHandle->stAovPipeAttr.eLastFps    = E_ST_FPS_HIGH;

    pstAovHandle->stAovPipeAttr.eCurrentLight = E_ST_LIGHT_BRIGHT;
    pstAovHandle->stAovPipeAttr.eLastLight    = E_ST_LIGHT_BRIGHT;

    pstAovHandle->stAovPipeAttr.pstDetBufHandle = NULL;

    memset(&pstAovHandle->stSNRSleepParam, 0x00, sizeof(ST_Common_SNRSleepParam_t));

    strcpy(pstAovHandle->stAovParam.au8StreamDumpPath, ".");

    // For auto test
    pstAovHandle->stAutoTestParam.bAutoTest = FALSE;
    memset(&pstAovHandle->stAutoTestParam.stTargetDetected, 0x00, sizeof(struct timeval));
    memset(&pstAovHandle->stAutoTestParam.stTargetBufferGet, 0x00, sizeof(struct timeval));
    memset(pstAovHandle->stAutoTestParam.au8KeyBoardInput, '*', sizeof(pstAovHandle->stAutoTestParam.au8KeyBoardInput));

    // For Aux Conv
    pstAovHandle->stAovSWLightSensorParam.bEnableSWLightSensor        = FALSE;
    pstAovHandle->stAovSWLightSensorParam.u32StableFramesBeforeTurnIr = 0;
    pstAovHandle->stAovSWLightSensorParam.u32StableFramesAfterTurnIr  = 0;
    pstAovHandle->stAovSWLightSensorParam.bCurrentRegardedStable      = TRUE;
    pstAovHandle->stAovSWLightSensorParam.bLastRegardedStable         = TRUE;
    pstAovHandle->stAovSWLightSensorParam.bKeep                       = FALSE;
    pstAovHandle->stAovSWLightSensorParam.bFlagTurnIRLedCut           = FALSE;

    // For light misc
    memset(&pstAovHandle->stLightMiscCtlParam, 0x00, sizeof(ST_Common_AovLightMiscCtlParam_t));
    // For HW light sensor
    pstAovHandle->stLightMiscCtlParam.stAovHWLightSensorParam.bEnableHWLightSensor = FALSE;
    pstAovHandle->stLightMiscCtlParam.stAovHWLightSensorParam.lastLux              = 0;
    pstAovHandle->stLightMiscCtlParam.stAovHWLightSensorParam.u32DiffLux           = 30;
    pstAovHandle->stLightMiscCtlParam.stAovHWLightSensorParam.s32TigMode           = 0;
    pstAovHandle->stLightMiscCtlParam.stAovHWLightSensorParam.u8LastIndex          = 0;
    pstAovHandle->stLightMiscCtlParam.stAovHWLightSensorParam.bUpdateLight         = FALSE;
    // For light ctl
    pstAovHandle->stLightMiscCtlParam.stAovLightCtlParam.stLightCtlAttr.controlType     = E_CONTROL_TYPE_LONG_TERM_OFF;
    pstAovHandle->stLightMiscCtlParam.stAovLightCtlParam.stLightCtlAttr.lightType       = E_LIGHT_TYPE_IR;
    pstAovHandle->stLightMiscCtlParam.stAovLightCtlParam.stLightCtlAttr.lightIntensity  = 0;
    pstAovHandle->stLightMiscCtlParam.stAovLightCtlParam.stLightCtlAttr.delayOpenTimeMs = 0;
    pstAovHandle->stLightMiscCtlParam.stAovLightCtlParam.eSwitchState                   = E_SWITCH_STATE_KEEP;

    // For fast ae
    pstAovHandle->stFastAEParam.bEnableFastAE = FALSE;
    pstAovHandle->stFastAEParam.bIsDoFastAE   = FALSE;

    // For demonstrate
    pstAovHandle->bDemonstrate = FALSE;

    // For stresstest
    pstAovHandle->bStressTest = FALSE;

    STCHECKRESULT(ST_Common_Sys_Init());

    // Get Plane Info
    memset(&stPlaneInfo, 0x00, sizeof(MI_SNR_PlaneInfo_t));
    STCHECKRESULT(MI_SNR_GetPlaneInfo(pstAovHandle->stAovPipeAttr.stVifParam[0].u32SnrPadId, 0, &stPlaneInfo));
    pstAovHandle->stAovParam.stPreviewSize.u16Width  = stPlaneInfo.stCapRect.u16Width;
    pstAovHandle->stAovParam.stPreviewSize.u16Height = stPlaneInfo.stCapRect.u16Height;

    // Set OSD Attr
    ST_Common_GetRgnDefaultOsdAttr(&(pstAovHandle->stAovPipeAttr.stChnPortOsdParam));
    pstAovHandle->stAovPipeAttr.stChnPortOsdParam.u32Layer                  = 0;
    pstAovHandle->stAovPipeAttr.stChnPortOsdParam.stOsdChnPort.u8PaletteIdx = 0;
    pstAovHandle->stAovPipeAttr.stChnPortOsdParam.stOsdChnPort.stPoint.u32X = 0;
    pstAovHandle->stAovPipeAttr.stChnPortOsdParam.stOsdChnPort.stPoint.u32Y = 0;
    pstAovHandle->stAovPipeAttr.stChnPortOsdParam.bShow                     = TRUE;

    pstAovHandle->stAovPipeAttr.u32MaxFrameHandleNum  = MAX_FRAME_HANDLE;
    pstAovHandle->stAovPipeAttr.hOsd                  = MAX_FRAME_HANDLE + 1;
    pstAovHandle->stAovPipeAttr.eVencType             = E_MI_VENC_MODTYPE_H264E;
    pstAovHandle->stAovParam.u32DelayConfirmFrameNum  = 10;
    pstAovHandle->stAovParam.u32WriteStreamTriggerNum = 10 * pstAovHandle->stAovPipeAttr.u32SensorNum;
    pstAovHandle->stAovPipeAttr.u32MaxStreamBufferNum =
        pstAovHandle->stAovParam.u32WriteStreamTriggerNum + VENC_STREAM_POOL_BUFFER;
    pstAovHandle->stAovPipeAttr.u32MaxStreamBufferSize = 0;

    // Set Frame Attr
    ST_Common_GetRgnDefaultFrameAttr((&pstAovHandle->stAovPipeAttr.stChnPortFrameParam));
    pstAovHandle->stAovPipeAttr.stChnPortFrameParam.bShow = FALSE;
    pstAovHandle->stAovPipeAttr.stChnPortFrameParam.stFrameChnPort.u8Thickness =
        pstAovHandle->stAovParam.stPreviewSize.u16Height * 0.005;

    for (MI_U32 i = 0; i < pstAovHandle->stAovPipeAttr.u32MaxFrameHandleNum; i++)
    {
        pstAovHandle->stAovPipeAttr.ahFrame[i] = i;
    }

    // Set Font Attr
    memset(&(pstAovHandle->stAovPipeAttr.stDrawTextAttr), 0x0, sizeof(ST_Common_OsdDrawText_Attr_t));
    pstAovHandle->stAovPipeAttr.stDrawTextAttr.color     = COLOR_OF_RED_ARGB1555;
    pstAovHandle->stAovPipeAttr.stDrawTextAttr.ePixelFmt = E_MI_RGN_PIXEL_FORMAT_ARGB1555;
    pstAovHandle->stAovPipeAttr.stDrawTextAttr.eFontType = SS_FONT_32x32;
    pstAovHandle->stAovPipeAttr.stDrawTextAttr.rot       = FONT_ROT_NONE;
    pstAovHandle->stAovPipeAttr.stDrawTextAttr.u32X      = 0;
    pstAovHandle->stAovPipeAttr.stDrawTextAttr.u32Y      = 0;
    pstAovHandle->stAovPipeAttr.stDrawTextAttr.handle    = pstAovHandle->stAovPipeAttr.hOsd;

    // Set Audio Attr
    pstAovHandle->stAovPipeAttr.stAudioParam.bEnable    = FALSE;
    pstAovHandle->stAovPipeAttr.stAudioParam.bAoEnable  = FALSE;
    pstAovHandle->stAovPipeAttr.stAudioParam.enAiIf[0]  = E_MI_AI_IF_ADC_AB;
    pstAovHandle->stAovPipeAttr.stAudioParam.enAiIf[1]  = E_MI_AI_IF_ECHO_A;
    pstAovHandle->stAovPipeAttr.stAudioParam.aenAoIf[0] = E_MI_AO_IF_DAC_AB;
    strcpy(pstAovHandle->stAovPipeAttr.stAudioParam.au8AiFile, ".");
    strcpy(pstAovHandle->stAovPipeAttr.stAudioParam.au8AoFile, "./ao.wav");

    return MI_SUCCESS;
}

MI_S32 ST_Common_AovPipeInit(ST_Common_AovHandle_t *pstAovHandle)
{
    ST_CHECK_POINTER(pstAovHandle);

    MI_SYS_ChnPort_t stSrcChnPort;

    MI_U8  u8SensorRes     = 0;
    MI_U32 u32SnrPadId     = 0;
    MI_U32 u32VifGroupId   = 0;
    MI_U32 u32VifDevId     = 0;
    MI_U32 u32IspDevId     = 0;
    MI_U32 u32IspChnId     = 0;
    MI_U32 u32SclDevId     = 0;
    MI_U32 u32SclChnId     = 0;
    MI_U32 u32SclDetPortId = 0;
    MI_U32 u32VencDevId    = 0;
    MI_U32 u32VencChnId    = 0;
    MI_U32 i               = 0;

    /************************************************
    set rtc alarm
    *************************************************/
    ST_Common_AovSetSuspendTime(pstAovHandle->stAovParam.u32SuspendTime);

    /************************************************
    Enter suspend, ensure that the first wake-up source is timer
    *************************************************/
    if (FALSE == pstAovHandle->stAutoTestParam.bAutoTest)
    {
        STCHECKRESULT(ST_Common_AovEnterSuspend(pstAovHandle));
    }

    /************************************************
    Confirm OS
    *************************************************/
    if (E_ST_OS_PURELINUX != ST_Common_AovOSCheck())
    {
        ST_INFO("Does not match the expected OS");
        return -1;
    }

    /************************************************
    init SYS
    *************************************************/
    STCHECKRESULT(ST_Common_Sys_Init());

    /************************************************
    init sensor/vif
    *************************************************/
    MI_VIF_GroupAttr_t stVifGroupAttr;
    MI_VIF_DevAttr_t   stVifDevAttr;

    if ((TRUE == pstAovHandle->stLightMiscCtlParam.stAovHWLightSensorParam.bEnableHWLightSensor)
        || (TRUE == pstAovHandle->stAovSWLightSensorParam.bEnableSWLightSensor))
    {
        pstAovHandle->stLightMiscCtlParam.fd = Dev_Light_Misc_Device_GetFd();
        if (pstAovHandle->stLightMiscCtlParam.fd < 0)
        {
            ST_ERR("Get light fd failed");
            return -1;
        }

        Dev_Light_Misc_Device_Set_Attr(pstAovHandle->stLightMiscCtlParam.fd,
                                       &pstAovHandle->stLightMiscCtlParam.stAovLightCtlParam.stLightCtlAttr);

        pstAovHandle->stLightMiscCtlParam.stAovHWLightSensorParam.s32TigMode =
            Dev_Light_Misc_Device_Get_TIGMode(pstAovHandle->stLightMiscCtlParam.fd);

        STCHECKRESULT(
            Dev_Light_Misc_Device_Get_Attr(pstAovHandle->stLightMiscCtlParam.fd,
                                           &(pstAovHandle->stLightMiscCtlParam.stAovLightCtlParam.stLightCtlAttr)));
    }

    for (i = 0; i < pstAovHandle->stAovPipeAttr.u32SensorNum; i++)
    {
        u8SensorRes   = pstAovHandle->stAovPipeAttr.stVifParam[i].u8SensorIndex;
        u32SnrPadId   = pstAovHandle->stAovPipeAttr.stVifParam[i].u32SnrPadId;
        u32VifGroupId = pstAovHandle->stAovPipeAttr.stVifParam[i].u32VifGroupId;
        u32VifDevId   = pstAovHandle->stAovPipeAttr.stVifParam[i].u32VifDevId;
        printf("sensor %d  %d_%d_%d_%d \n", i, u8SensorRes, u32SnrPadId, u32VifGroupId, u32VifDevId);
        STCHECKRESULT(ST_Common_SensorInit(u32SnrPadId, FALSE, u8SensorRes, FPS_HIGH));
        ST_Common_GetVifDefaultGrouptAttr(&stVifGroupAttr);
        STCHECKRESULT(ST_Common_VifCreateDevGroup(u32VifGroupId, &stVifGroupAttr));
        ST_Common_GetVifDefaultDevAttr(&stVifDevAttr);
        STCHECKRESULT(ST_Common_VifEnableDev(u32VifDevId, &stVifDevAttr));

        if ((TRUE == pstAovHandle->stLightMiscCtlParam.stAovHWLightSensorParam.bEnableHWLightSensor)
            || (TRUE == pstAovHandle->stAovSWLightSensorParam.bEnableSWLightSensor))
        {
            STCHECKRESULT(Dev_Light_Misc_Device_Init(pstAovHandle->stLightMiscCtlParam.fd, u32VifDevId));
        }
    }

    /************************************************
    init det
    *************************************************/
    DetectionInfo_t stDetectionInfo;

    memset(&stDetectionInfo, 0x0, sizeof(DetectionInfo_t));
    memcpy(stDetectionInfo.model, pstAovHandle->stAovParam.au8DetectModelPath, MAX_DET_STRLEN);
    stDetectionInfo.threshold        = DET_THRESHOLD;
    stDetectionInfo.disp_size.width  = pstAovHandle->stAovParam.stPreviewSize.u16Width;
    stDetectionInfo.disp_size.height = pstAovHandle->stAovParam.stPreviewSize.u16Height;

    STCHECKRESULT(ST_Common_DET_Init(&(pstAovHandle->stAovPipeAttr.pDetHandle), &stDetectionInfo));
    STCHECKRESULT(ST_Common_DET_SetDefaultParam(&(pstAovHandle->stAovPipeAttr.pDetHandle)));
    STCHECKRESULT(ST_Common_DET_GetModelAttr(&(pstAovHandle->stAovPipeAttr.pDetHandle),
                                             &(pstAovHandle->stAovPipeAttr.stDetModelAttr)));

    /************************************************
     init isp
     *************************************************/
    MI_ISP_DevAttr_t         stIspDevAttr;
    MI_ISP_ChannelAttr_t     stIspChnAttr;
    MI_ISP_ChnParam_t        stIspChnParam;
    MI_SYS_WindowRect_t      stIspInputCrop;
    MI_ISP_BindSnrId_e       u32SensorBindId;
    MI_S32                   s32TigMode  = pstAovHandle->stLightMiscCtlParam.stAovHWLightSensorParam.s32TigMode;
    MI_U8                    u8LastIndex = pstAovHandle->stLightMiscCtlParam.stAovHWLightSensorParam.u8LastIndex;
    MI_ISP_AE_ExpoInfoType_t stAeExpoInfo;
    CusAeOpMode_e            eCusAeOpMode;

    memset(&stAeExpoInfo, 0x00, sizeof(MI_ISP_AE_ExpoInfoType_t));
    ST_Common_GetIspDefaultDevAttr(&stIspDevAttr);
    STCHECKRESULT(ST_Common_IspCreateDevice(u32IspDevId, &stIspDevAttr));
    for (i = 0; i < pstAovHandle->stAovPipeAttr.u32SensorNum; i++)
    {
        u32IspDevId = pstAovHandle->stAovPipeAttr.stChnPortIsp[i].u32DevId;
        u32IspChnId = pstAovHandle->stAovPipeAttr.stChnPortIsp[i].u32ChnId;
        ST_Common_GetIspDefaultChnAttr(&stIspChnAttr, &stIspInputCrop, &stIspChnParam);
        if (i == 0)
        {
            u32SensorBindId = E_MI_ISP_SENSOR0;
        }
        else
        {
            u32SensorBindId = E_MI_ISP_SENSOR2;
        }
        stIspChnParam.e3DNRLevel = E_MI_ISP_3DNR_LEVEL2;
        ST_Common_GetIspBindSensorIdByPad(pstAovHandle->stAovPipeAttr.stVifParam[i].u32SnrPadId, &u32SensorBindId);

        if (pstAovHandle->stLightMiscCtlParam.stAovHWLightSensorParam.bEnableHWLightSensor == TRUE)
        {
            MasterEarlyInitParam_t stIspEarlyInitParam;
            memset(&stIspEarlyInitParam, 0x0, sizeof(MasterEarlyInitParam_t));
            MI_U32 u32Shutter = g_stLightTable[s32TigMode][u8LastIndex].u32Shutter;
            MI_U32 u32AeGain  = g_stLightTable[s32TigMode][u8LastIndex].u32Sensorgain;
            MI_U32 u32IspGain = g_stLightTable[s32TigMode][u8LastIndex].u32Ispgain;

            if (u32Shutter != 0 && u32AeGain != 0 && u32IspGain != 0)
            {
                stIspEarlyInitParam.u16SnrEarlyFps             = 1;
                stIspEarlyInitParam.u16SnrEarlyFlicker         = 0;
                stIspEarlyInitParam.u32SnrEarlyShutter         = u32Shutter;
                stIspEarlyInitParam.u32SnrEarlyGainX1024       = u32AeGain;
                stIspEarlyInitParam.u32SnrEarlyDGain           = u32IspGain;
                stIspEarlyInitParam.u32SnrEarlyShutterShort    = 30;
                stIspEarlyInitParam.u32SnrEarlyGainX1024Short  = 1024;
                stIspEarlyInitParam.u32SnrEarlyDGainShort      = 1024;
                stIspEarlyInitParam.u16SnrEarlyAwbRGain        = 1024;
                stIspEarlyInitParam.u16SnrEarlyAwbBGain        = 2263;
                stIspEarlyInitParam.u16SnrEarlyAwbGGain        = 2134;
                stIspEarlyInitParam.u32SnrEarlyShutterMedium   = 30;
                stIspEarlyInitParam.u32SnrEarlyGainX1024Medium = 1024;
                stIspEarlyInitParam.u32SnrEarlyDGainMedium     = 1024;

                ST_INFO("sensor shutter %d, sensor gain %d, isp gain %d \n", u32Shutter, u32AeGain, u32IspGain);

                stIspChnAttr.stIspCustIqParam.stVersion.u32Revision = EARLYINIT_PARAM_TYPE_MASTER;
                stIspChnAttr.stIspCustIqParam.stVersion.u32Size     = sizeof(MasterEarlyInitParam_t);
                memcpy(stIspChnAttr.stIspCustIqParam.stVersion.u8Data, &stIspEarlyInitParam,
                       sizeof(MasterEarlyInitParam_t));
            }
        }

        stIspChnAttr.u32SensorBindId = u32SensorBindId;
        STCHECKRESULT(ST_Common_IspStartChn(u32IspDevId, u32IspChnId, &stIspChnAttr, &stIspInputCrop, &stIspChnParam));

        // Adjust the position of AE statistics (for fast ae)
        CusAESource_e eAeSource = AE_SOURCE_FROM_SE_ALSC_AF_HDR;
        MI_ISP_CUS3A_SetAESource(u32IspDevId, u32IspChnId, &eAeSource);

        STCHECKRESULT(MI_ISP_AE_QueryExposureInfo(u32IspDevId, u32IspChnId, &stAeExpoInfo));
        printf("LumY      = %-10d, SceneTarget = %-5d, bStable     = %-5d, bIsReachBoundary = %d\n",
               stAeExpoInfo.stHistWeightY.u32LumY, stAeExpoInfo.u32SceneTarget, stAeExpoInfo.bIsStable,
               stAeExpoInfo.bIsReachBoundary);

        if (pstAovHandle->stLightMiscCtlParam.stAovHWLightSensorParam.bEnableHWLightSensor == TRUE)
        {
            STCHECKRESULT(__UpdataLightParam(pstAovHandle));

            if (FALSE
                == pstAovHandle->stLightMiscCtlParam.stAovHWLightSensorParam
                       .bUpdateLight) // first time light retry one time
            {
                STCHECKRESULT(__UpdataLightParam(pstAovHandle));
            }

            if (pstAovHandle->stLightMiscCtlParam.stAovHWLightSensorParam.lastLux > 10)
            {
                pstAovHandle->stAovPipeAttr.eCurrentLight = E_ST_LIGHT_BRIGHT;
            }
            else
            {
                pstAovHandle->stAovPipeAttr.eCurrentLight = E_ST_LIGHT_DARK;
            }
        }
        else
        {
            pstAovHandle->stAovPipeAttr.eCurrentLight = E_ST_LIGHT_BRIGHT;
        }

        if (pstAovHandle->stAovPipeAttr.eCurrentLight == E_ST_LIGHT_DARK)
        {
            if (strlen(pstAovHandle->stAovPipeAttr.au8IqApiBinDarkPath) != 0)
            {
                ST_Common_IspSetIqBin(u32IspDevId, u32IspChnId, pstAovHandle->stAovPipeAttr.au8IqApiBinDarkPath);
            }

            if ((TRUE == pstAovHandle->stLightMiscCtlParam.stAovHWLightSensorParam.bEnableHWLightSensor)
                || (TRUE == pstAovHandle->stAovSWLightSensorParam.bEnableSWLightSensor))
            {
                __TurnOffIRCut(pstAovHandle);
                __TurnOnIRLed(pstAovHandle);
            }
        }
        else
        {
            if (strlen(pstAovHandle->stAovPipeAttr.au8IqApiBinBrightPath) != 0)
            {
                ST_Common_IspSetIqBin(u32IspDevId, u32IspChnId, pstAovHandle->stAovPipeAttr.au8IqApiBinBrightPath);
            }

            if ((TRUE == pstAovHandle->stLightMiscCtlParam.stAovHWLightSensorParam.bEnableHWLightSensor)
                || (TRUE == pstAovHandle->stAovSWLightSensorParam.bEnableSWLightSensor))
            {
                __TurnOnIRCut(pstAovHandle);
                __TurnOffIRLed(pstAovHandle);
            }
        }
    }

    // Config to aov sensor AE mode (for fast ae)
    eCusAeOpMode = AE_OP_MODE_AOV_SENSOR_AE;
    MI_ISP_CUS3A_SetAeOpMode(u32IspDevId, u32IspChnId, &eCusAeOpMode);

    /************************************************
    init scl
    *************************************************/
    MI_SCL_DevAttr_t     stSclDevAttr;
    MI_SCL_ChannelAttr_t stSclChnAttr;
    MI_SCL_ChnParam_t    stSclChnParam;
    MI_SYS_WindowRect_t  stSclInputCrop;

    for (i = 0; i < pstAovHandle->stAovPipeAttr.u32SensorNum; i++)
    {
        u32SclDevId     = pstAovHandle->stAovPipeAttr.stChnPortSclPreview[i].u32DevId;
        u32SclChnId     = pstAovHandle->stAovPipeAttr.stChnPortSclPreview[i].u32ChnId;
        u32SclDetPortId = pstAovHandle->stAovPipeAttr.stChnPortSclDetect[i].u32PortId;
        ST_Common_GetSclDefaultDevAttr(&stSclDevAttr);
        STCHECKRESULT(ST_Common_SclCreateDevice(u32SclDevId, &stSclDevAttr));

        ST_Common_GetSclDefaultChnAttr(&stSclChnAttr, &stSclInputCrop, &stSclChnParam);
        STCHECKRESULT(ST_Common_SclStartChn(u32SclDevId, u32SclChnId, &stSclChnAttr, &stSclInputCrop, &stSclChnParam));

        memset(&stSrcChnPort, 0x00, sizeof(MI_SYS_ChnPort_t));
        stSrcChnPort.eModId    = E_MI_MODULE_ID_SCL;
        stSrcChnPort.u32DevId  = u32SclDevId;
        stSrcChnPort.u32ChnId  = u32SclChnId;
        stSrcChnPort.u32PortId = u32SclDetPortId;
        STCHECKRESULT(MI_SYS_SetChnOutputPortDepth(0, &stSrcChnPort, 2, 2));
    }

    /************************************************
    init venc
    *************************************************/
    MI_VENC_InitParam_t         stVencInitParam;
    MI_VENC_ChnAttr_t           stVencChnAttr;
    MI_VENC_InputSourceConfig_t stVencSourceCfg;
    MI_VENC_ModType_e           eType;

    for (i = 0; i < pstAovHandle->stAovPipeAttr.u32SensorNum; i++)
    {
        u32VencDevId = pstAovHandle->stAovPipeAttr.stChnPortVenc[i].u32DevId;
        u32VencChnId = pstAovHandle->stAovPipeAttr.stChnPortVenc[i].u32ChnId;

        ST_Common_GetVencDefaultDevAttr(&stVencInitParam);
        STCHECKRESULT(ST_Common_VencCreateDev(u32VencDevId, &stVencInitParam));

        if (E_MI_VENC_MODTYPE_H265E == pstAovHandle->stAovPipeAttr.eVencType)
        {
            eType = E_MI_VENC_MODTYPE_H265E;
            ST_Common_GetVencDefaultChnAttr(eType, &stVencChnAttr, &stVencSourceCfg);
            stVencSourceCfg.eInputSrcBufferMode             = E_MI_VENC_INPUT_MODE_RING_UNIFIED_DMA;
            stVencChnAttr.stVeAttr.stAttrH265e.u32PicWidth  = pstAovHandle->stAovParam.stPreviewSize.u16Width;
            stVencChnAttr.stVeAttr.stAttrH265e.u32PicHeight = pstAovHandle->stAovParam.stPreviewSize.u16Height;
            stVencChnAttr.stRcAttr.stAttrH265Cbr.u32BitRate =
                stVencChnAttr.stVeAttr.stAttrH265e.u32PicWidth * stVencChnAttr.stVeAttr.stAttrH265e.u32PicHeight * 1.3;
            stVencChnAttr.stRcAttr.stAttrH265Cbr.u32Gop           = PIPE_VENC_GOP;
            stVencChnAttr.stRcAttr.stAttrH265Cbr.u32SrcFrmRateNum = FPS_HIGH;
            stVencChnAttr.stVeAttr.stAttrH265e.u32BufSize =
                ((stVencChnAttr.stRcAttr.stAttrH265Cbr.u32BitRate / 8) / FPS_HIGH)
                * (pstAovHandle->stAovPipeAttr.u32MaxStreamBufferNum);
        }
        else
        {
            eType = E_MI_VENC_MODTYPE_H264E;
            ST_Common_GetVencDefaultChnAttr(eType, &stVencChnAttr, &stVencSourceCfg);
            stVencSourceCfg.eInputSrcBufferMode             = E_MI_VENC_INPUT_MODE_RING_UNIFIED_DMA;
            stVencChnAttr.stVeAttr.stAttrH264e.u32PicWidth  = pstAovHandle->stAovParam.stPreviewSize.u16Width;
            stVencChnAttr.stVeAttr.stAttrH264e.u32PicHeight = pstAovHandle->stAovParam.stPreviewSize.u16Height;
            stVencChnAttr.stRcAttr.stAttrH264Cbr.u32BitRate =
                stVencChnAttr.stVeAttr.stAttrH264e.u32PicWidth * stVencChnAttr.stVeAttr.stAttrH264e.u32PicHeight * 1.3;
            stVencChnAttr.stRcAttr.stAttrH264Cbr.u32Gop           = PIPE_VENC_GOP;
            stVencChnAttr.stRcAttr.stAttrH264Cbr.u32SrcFrmRateNum = FPS_HIGH;
            stVencChnAttr.stVeAttr.stAttrH264e.u32BufSize =
                ((stVencChnAttr.stRcAttr.stAttrH264Cbr.u32BitRate / 8) / FPS_HIGH)
                * (pstAovHandle->stAovPipeAttr.u32MaxStreamBufferNum);
        }
        pstAovHandle->stAovPipeAttr.u32MaxStreamBufferSize = stVencChnAttr.stVeAttr.stAttrH265e.u32BufSize;
        printf("Venc max stream buffer = %d \n", pstAovHandle->stAovPipeAttr.u32MaxStreamBufferSize);

        STCHECKRESULT(ST_Common_VencStartChn(u32VencDevId, u32VencChnId, &stVencChnAttr, &stVencSourceCfg));

        MI_VENC_SetMaxStreamCnt(u32VencDevId, u32VencChnId, pstAovHandle->stAovPipeAttr.u32MaxStreamBufferNum);
    }
    /************************************************
    init rgn
    *************************************************/
    MI_RGN_PaletteTable_t stPaletteTable;
    MI_RGN_Attr_t         stRgnOsdAttr;
    MI_RGN_Attr_t         stRgnFrameAttr;

    ST_Common_GetRgnDefaultInitAttr(&stPaletteTable);
    STCHECKRESULT(ST_Common_RgnInit(0, &stPaletteTable));

    // Creat OSD hOsd
    ST_Common_GetRgnDefaultCreateAttr(&stRgnOsdAttr);
    stRgnOsdAttr.stOsdInitParam.ePixelFmt        = E_MI_RGN_PIXEL_FORMAT_ARGB1555;
    stRgnOsdAttr.stOsdInitParam.stSize.u32Height = 150;
    stRgnOsdAttr.stOsdInitParam.stSize.u32Width  = 500;
    STCHECKRESULT(ST_Common_RgnCreate(0, pstAovHandle->stAovPipeAttr.hOsd, &stRgnOsdAttr));
    for (MI_U32 i = 0; i < pstAovHandle->stAovPipeAttr.u32SensorNum; i++)
    {
        STCHECKRESULT(ST_Common_RgnAttachChn(0, pstAovHandle->stAovPipeAttr.hOsd,
                                             &(pstAovHandle->stAovPipeAttr.stChnPortRgnAttached[i]),
                                             &(pstAovHandle->stAovPipeAttr.stChnPortOsdParam)));
    }

    // Creat Frame ghFrame
    ST_Common_GetRgnDefaultCreateAttr(&stRgnFrameAttr);
    stRgnFrameAttr.eType = E_MI_RGN_TYPE_FRAME;
    for (i = 0; i < pstAovHandle->stAovPipeAttr.u32MaxFrameHandleNum; i++)
    {
        STCHECKRESULT(ST_Common_RgnCreate(0, pstAovHandle->stAovPipeAttr.ahFrame[i], &stRgnFrameAttr));

        /************************************************
        attch handle Frame to venc
        *************************************************/
        STCHECKRESULT(ST_Common_RgnAttachChn(0, pstAovHandle->stAovPipeAttr.ahFrame[i],
                                             &(pstAovHandle->stAovPipeAttr.stChnPortRgnAttached[0]),
                                             (&pstAovHandle->stAovPipeAttr.stChnPortFrameParam)));
    }

    // Init fastAE
    if (TRUE == pstAovHandle->stFastAEParam.bEnableFastAE)
    {
        ST_Pipeline_Param stPipelineParam;
        MI_U32            u32StreamOutCnt = 1;
        MI_U32            u32DropCnt      = 0;
        MI_U32            u32RunThreshold = 90;

        memset(&stPipelineParam, 0x00, sizeof(ST_Pipeline_Param));

        stPipelineParam.u32SnrPadId  = pstAovHandle->stAovPipeAttr.stVifParam[0].u32SnrPadId;
        stPipelineParam.u32SensorFps = FPS_HIGH;
        stPipelineParam.u32VifDevId  = pstAovHandle->stAovPipeAttr.stVifParam[0].u32VifDevId;
        stPipelineParam.u32VifPortId = pstAovHandle->stAovPipeAttr.stVifParam[0].u32VifPortId;
        stPipelineParam.u32IspDevId  = pstAovHandle->stAovPipeAttr.stChnPortIsp[0].u32DevId;
        stPipelineParam.u32IspChnId  = pstAovHandle->stAovPipeAttr.stChnPortIsp[0].u32ChnId;
        stPipelineParam.u32SclDevId  = pstAovHandle->stAovPipeAttr.stChnPortSclPreview[0].u32DevId;
        stPipelineParam.u32SclChnId  = pstAovHandle->stAovPipeAttr.stChnPortSclPreview[0].u32ChnId;
        stPipelineParam.u32VencDevId = pstAovHandle->stAovPipeAttr.stChnPortVenc[0].u32DevId;
        stPipelineParam.u32VencChnId = pstAovHandle->stAovPipeAttr.stChnPortVenc[0].u32ChnId;

        ST_Common_FastAE_InitPipelineParam(&stPipelineParam);

        ST_Common_FastAE_InitIspConvParam(u32StreamOutCnt, u32DropCnt, u32RunThreshold);
    }
    /************************************************
    open rtc
    *************************************************/
    __RtcOpen(&pstAovHandle->stAovPipeAttr.fdRtc);
    __RtcSetDefaultTime(pstAovHandle->stAovPipeAttr.fdRtc);

    return MI_SUCCESS;
}

MI_S32 ST_Common_AovPipeStart(ST_Common_AovHandle_t *pstAovHandle)
{
    ST_CHECK_POINTER(pstAovHandle);

    MI_VIF_OutputPortAttr_t stVifPortAttr;
    MI_SYS_ChnPort_t        stSrcChnPort;
    MI_SYS_ChnPort_t        stDstChnPort;
    MI_U32                  u32SrcFrmrate;
    MI_U32                  u32DstFrmrate;
    MI_SYS_BindType_e       eBindType;
    MI_U32                  u32BindParam;

    for (MI_U32 i = 0; i < pstAovHandle->stAovPipeAttr.u32SensorNum; i++)
    {
        MI_U32 u32VifDevId  = pstAovHandle->stAovPipeAttr.stVifParam[i].u32VifDevId;
        MI_U32 u32VifPortId = pstAovHandle->stAovPipeAttr.stVifParam[i].u32VifPortId;

        MI_U32 u32IspDevId  = pstAovHandle->stAovPipeAttr.stChnPortIsp[i].u32DevId;
        MI_U32 u32IspChnId  = pstAovHandle->stAovPipeAttr.stChnPortIsp[i].u32ChnId;
        MI_U32 u32IspPortId = pstAovHandle->stAovPipeAttr.stChnPortIsp[i].u32PortId;

        MI_U32 u32SclDevId         = pstAovHandle->stAovPipeAttr.stChnPortSclPreview[i].u32DevId;
        MI_U32 u32SclChnId         = pstAovHandle->stAovPipeAttr.stChnPortSclPreview[i].u32ChnId;
        MI_U32 u32SclPreviewPortId = pstAovHandle->stAovPipeAttr.stChnPortSclPreview[i].u32PortId;
        MI_U32 u32SclDetPortId     = pstAovHandle->stAovPipeAttr.stChnPortSclDetect[i].u32PortId;

        MI_U32 u32VencDevId = pstAovHandle->stAovPipeAttr.stChnPortVenc[i].u32DevId;
        MI_U32 u32VencChnId = pstAovHandle->stAovPipeAttr.stChnPortVenc[i].u32ChnId;

        /************************************************
        start isp
        *************************************************/
        MI_ISP_OutPortParam_t stIspOutPortParam;
        ST_Common_GetIspDefaultPortAttr(&stIspOutPortParam);
        STCHECKRESULT(ST_Common_IspEnablePort(u32IspDevId, u32IspChnId, u32IspPortId, &stIspOutPortParam));

        /************************************************
        start scl
        *************************************************/
        MI_SCL_OutPortParam_t stSclOutPortPreviewParam;
        MI_SCL_OutPortParam_t stSclOutPortDetParam;

        ST_Common_GetSclDefaultPortAttr(&stSclOutPortPreviewParam);
        ST_Common_GetSclDefaultPortAttr(&stSclOutPortDetParam);

        stSclOutPortPreviewParam.stSCLOutputSize.u16Width  = pstAovHandle->stAovParam.stPreviewSize.u16Width;
        stSclOutPortPreviewParam.stSCLOutputSize.u16Height = pstAovHandle->stAovParam.stPreviewSize.u16Height;
        STCHECKRESULT(
            ST_Common_SclEnablePort(u32SclDevId, u32SclChnId, u32SclPreviewPortId, &stSclOutPortPreviewParam));

        stSclOutPortDetParam.stSCLOutputSize.u16Width  = pstAovHandle->stAovPipeAttr.stDetModelAttr.width;
        stSclOutPortDetParam.stSCLOutputSize.u16Height = pstAovHandle->stAovPipeAttr.stDetModelAttr.height;
        STCHECKRESULT(ST_Common_SclEnablePort(u32SclDevId, u32SclChnId, u32SclDetPortId, &stSclOutPortDetParam));

        /************************************************
        start vif
        *************************************************/
        ST_Common_GetVifDefaultPortAttr(&stVifPortAttr);
        STCHECKRESULT(ST_Common_VifEnablePort(u32VifDevId, u32VifPortId, &stVifPortAttr));

        /************************************************
        bind vif->isp
        *************************************************/
        memset(&stSrcChnPort, 0x00, sizeof(MI_SYS_ChnPort_t));
        memset(&stDstChnPort, 0x00, sizeof(MI_SYS_ChnPort_t));
        stSrcChnPort.eModId    = E_MI_MODULE_ID_VIF;
        stSrcChnPort.u32DevId  = u32VifDevId;
        stSrcChnPort.u32ChnId  = 0;
        stSrcChnPort.u32PortId = u32VifPortId;
        stDstChnPort.eModId    = E_MI_MODULE_ID_ISP;
        stDstChnPort.u32DevId  = u32IspDevId;
        stDstChnPort.u32ChnId  = u32IspChnId;
        stDstChnPort.u32PortId = 0;
        u32SrcFrmrate          = 15;
        u32DstFrmrate          = 15;
        eBindType = E_MI_SYS_BIND_TYPE_FRAME_BASE;
        if(pstAovHandle->bRealTimeEnable)
        {
            eBindType = E_MI_SYS_BIND_TYPE_REALTIME;
        }
        u32BindParam = 0;
        STCHECKRESULT(MI_SYS_BindChnPort2(0, &stSrcChnPort, &stDstChnPort, u32SrcFrmrate, u32DstFrmrate, eBindType,
                                          u32BindParam));
        /************************************************
        bind isp->scl
        *************************************************/
        memset(&stSrcChnPort, 0x00, sizeof(MI_SYS_ChnPort_t));
        memset(&stDstChnPort, 0x00, sizeof(MI_SYS_ChnPort_t));
        stSrcChnPort.eModId    = E_MI_MODULE_ID_ISP;
        stSrcChnPort.u32DevId  = u32IspDevId;
        stSrcChnPort.u32ChnId  = u32IspChnId;
        stSrcChnPort.u32PortId = u32IspPortId;
        stDstChnPort.eModId    = E_MI_MODULE_ID_SCL;
        stDstChnPort.u32DevId  = u32SclDevId;
        stDstChnPort.u32ChnId  = u32SclChnId;
        stDstChnPort.u32PortId = 0;
        u32SrcFrmrate          = 15;
        u32DstFrmrate          = 15;
        eBindType              = E_MI_SYS_BIND_TYPE_REALTIME;
        u32BindParam           = 0;
        STCHECKRESULT(MI_SYS_BindChnPort2(0, &stSrcChnPort, &stDstChnPort, u32SrcFrmrate, u32DstFrmrate, eBindType,
                                          u32BindParam));

        // Set SCL output pool buffer
        MI_SYS_GlobalPrivPoolConfig_t stConfig;
        memset(&stConfig, 0x00, sizeof(MI_SYS_GlobalPrivPoolConfig_t));
        stConfig.bCreate                                         = TRUE;
        stConfig.eConfigType                                     = E_MI_SYS_PER_DEV_PRIVATE_RING_POOL;
        stConfig.uConfig.stpreDevPrivRingPoolConfig.eModule      = E_MI_MODULE_ID_SCL;
        stConfig.uConfig.stpreDevPrivRingPoolConfig.u32Devid     = u32SclDevId;
        stConfig.uConfig.stpreDevPrivRingPoolConfig.u16MaxWidth  = pstAovHandle->stAovParam.stPreviewSize.u16Width;
        stConfig.uConfig.stpreDevPrivRingPoolConfig.u16MaxHeight = pstAovHandle->stAovParam.stPreviewSize.u16Height;
        stConfig.uConfig.stpreDevPrivRingPoolConfig.u16RingLine  = pstAovHandle->stAovParam.stPreviewSize.u16Height * 2;
        MI_SYS_ConfigPrivateMMAPool(0, &stConfig);

        /************************************************
        bind scl->venc
        *************************************************/
        memset(&stSrcChnPort, 0x00, sizeof(MI_SYS_ChnPort_t));
        memset(&stDstChnPort, 0x00, sizeof(MI_SYS_ChnPort_t));
        stSrcChnPort.eModId    = E_MI_MODULE_ID_SCL;
        stSrcChnPort.u32DevId  = u32SclDevId;
        stSrcChnPort.u32ChnId  = u32SclChnId;
        stSrcChnPort.u32PortId = u32SclPreviewPortId;
        stDstChnPort.eModId    = E_MI_MODULE_ID_VENC;
        stDstChnPort.u32DevId  = u32VencDevId;
        stDstChnPort.u32ChnId  = u32VencChnId;
        stDstChnPort.u32PortId = 0;
        u32SrcFrmrate          = 15;
        u32DstFrmrate          = 15;
        eBindType              = E_MI_SYS_BIND_TYPE_HW_RING;
        u32BindParam           = 0;
        STCHECKRESULT(MI_SYS_BindChnPort2(0, &stSrcChnPort, &stDstChnPort, u32SrcFrmrate, u32DstFrmrate, eBindType,
                                          u32BindParam));
    }

    return MI_SUCCESS;
}

MI_S32 ST_Common_AovGpio_Set(char *pGpio, char *pDirection, char *pLevel)
{
    MI_S32 s32ExportFD    = 0;
    MI_S32 s32DirectionFD = 0;
    MI_S32 s32ValueFD     = 0;
    char   u8DirectionPath[128];
    char   u8ValuePath[128];
    s32ExportFD = open(GPIO_EXPORT_NAME, O_WRONLY);
    if (s32ExportFD >= 0)
    {
        write(s32ExportFD, pGpio, 4);
        close(s32ExportFD);
    }
    else
    {
        ST_INFO(GPIO_EXPORT_NAME "open fail\n");
        return -1;
    }

    sprintf(u8DirectionPath, "/sys/class/gpio/gpio%s/direction", pGpio);
    s32DirectionFD = open(u8DirectionPath, O_WRONLY);
    if (s32DirectionFD >= 0)
    {
        write(s32DirectionFD, pDirection, 5);
        close(s32DirectionFD);
    }
    else
    {
        ST_INFO(u8DirectionPath, "open fail\n");
        return -1;
    }

    sprintf(u8ValuePath, "/sys/class/gpio/gpio%s/value", pGpio);
    s32ValueFD = open(u8ValuePath, O_WRONLY);
    if (s32ValueFD >= 0)
    {
        write(s32ValueFD, pLevel, 2);
        close(s32ValueFD);
    }
    else
    {
        ST_INFO(u8ValuePath, "open fail\n");
        return -1;
    }

    return MI_SUCCESS;
}

MI_S32 ST_Common_AovMcuGpioInit(ST_Common_AovHandle_t *pstAovHandle)
{
    ST_CHECK_POINTER(pstAovHandle);

    if (pstAovHandle->bUsingMcu)
    {
        ST_Common_AovGpio_Set(GPIO_EVENT_DONE, "out", "1");
        ST_Common_AovGpio_Set(GPIO_PIR_FLAG, "in", "1");
        ST_Common_AovGpio_Set(GPIO_USER_PREVIEW_FLAG, "in", "1");
        ST_Common_AovGpio_Set(GPIO_SUSPEND_NOTICE, "high", "1");
    }

    return MI_SUCCESS;
}

MI_S32 ST_Common_AovPipeStop(ST_Common_AovHandle_t *pstAovHandle)
{
    ST_CHECK_POINTER(pstAovHandle);

    MI_SYS_ChnPort_t stSrcChnPort;
    MI_SYS_ChnPort_t stDstChnPort;

    MI_U32 i;

    for (i = 0; i < pstAovHandle->stAovPipeAttr.u32MaxFrameHandleNum; i++)
    {
        /************************************************
        detach rgn->venc
        *************************************************/
        STCHECKRESULT(ST_Common_RgnDetachChn(0, pstAovHandle->stAovPipeAttr.ahFrame[i],
                                             &(pstAovHandle->stAovPipeAttr.stChnPortRgnAttached[0])));
    }

    for (i = 0; i < pstAovHandle->stAovPipeAttr.u32SensorNum; i++)
    {
        STCHECKRESULT(ST_Common_RgnDetachChn(0, pstAovHandle->stAovPipeAttr.hOsd,
                                             &(pstAovHandle->stAovPipeAttr.stChnPortRgnAttached[i])));
    }

    for (i = 0; i < pstAovHandle->stAovPipeAttr.u32SensorNum; i++)
    {
        MI_U32 u32VifDevId  = pstAovHandle->stAovPipeAttr.stVifParam[i].u32VifDevId;
        MI_U32 u32VifPortId = pstAovHandle->stAovPipeAttr.stVifParam[i].u32VifPortId;

        MI_U32 u32IspDevId  = pstAovHandle->stAovPipeAttr.stChnPortIsp[i].u32DevId;
        MI_U32 u32IspChnId  = pstAovHandle->stAovPipeAttr.stChnPortIsp[i].u32ChnId;
        MI_U32 u32IspPortId = pstAovHandle->stAovPipeAttr.stChnPortIsp[i].u32PortId;

        MI_U32 u32SclDevId         = pstAovHandle->stAovPipeAttr.stChnPortSclPreview[i].u32DevId;
        MI_U32 u32SclChnId         = pstAovHandle->stAovPipeAttr.stChnPortSclPreview[i].u32ChnId;
        MI_U32 u32SclPreviewPortId = pstAovHandle->stAovPipeAttr.stChnPortSclPreview[i].u32PortId;
        MI_U32 u32SclDetPortId     = pstAovHandle->stAovPipeAttr.stChnPortSclDetect[i].u32PortId;

        MI_U32 u32VencDevId = pstAovHandle->stAovPipeAttr.stChnPortVenc[i].u32DevId;
        MI_U32 u32VencChnId = pstAovHandle->stAovPipeAttr.stChnPortVenc[i].u32ChnId;

        /************************************************
        unbind scl->venc
        *************************************************/
        memset(&stSrcChnPort, 0x00, sizeof(MI_SYS_ChnPort_t));
        memset(&stDstChnPort, 0x00, sizeof(MI_SYS_ChnPort_t));
        stSrcChnPort.eModId    = E_MI_MODULE_ID_SCL;
        stSrcChnPort.u32DevId  = u32SclDevId;
        stSrcChnPort.u32ChnId  = u32SclChnId;
        stSrcChnPort.u32PortId = u32SclPreviewPortId;
        stDstChnPort.eModId    = E_MI_MODULE_ID_VENC;
        stDstChnPort.u32DevId  = u32VencDevId;
        stDstChnPort.u32ChnId  = u32VencChnId;
        stDstChnPort.u32PortId = 0;
        STCHECKRESULT(MI_SYS_UnBindChnPort(0, &stSrcChnPort, &stDstChnPort));

        /************************************************
        stop venc
        *************************************************/
        STCHECKRESULT(ST_Common_VencStopChn(u32VencDevId, u32VencChnId));

        /************************************************
        unbind isp->scl
        *************************************************/
        memset(&stSrcChnPort, 0x00, sizeof(MI_SYS_ChnPort_t));
        memset(&stDstChnPort, 0x00, sizeof(MI_SYS_ChnPort_t));
        stSrcChnPort.eModId    = E_MI_MODULE_ID_ISP;
        stSrcChnPort.u32DevId  = u32IspDevId;
        stSrcChnPort.u32ChnId  = u32IspChnId;
        stSrcChnPort.u32PortId = u32IspPortId;
        stDstChnPort.eModId    = E_MI_MODULE_ID_SCL;
        stDstChnPort.u32DevId  = u32SclDevId;
        stDstChnPort.u32ChnId  = u32SclChnId;
        stDstChnPort.u32PortId = 0;
        STCHECKRESULT(MI_SYS_UnBindChnPort(0, &stSrcChnPort, &stDstChnPort));

        /************************************************
        stop scl
        *************************************************/
        STCHECKRESULT(ST_Common_SclDisablePort(u32SclDevId, u32SclChnId, u32SclDetPortId));
        STCHECKRESULT(ST_Common_SclDisablePort(u32SclDevId, u32SclChnId, u32SclPreviewPortId));
        STCHECKRESULT(ST_Common_SclStopChn(u32SclDevId, u32SclChnId));

        /************************************************
        unbind vif->isp
        *************************************************/
        memset(&stSrcChnPort, 0x00, sizeof(MI_SYS_ChnPort_t));
        memset(&stDstChnPort, 0x00, sizeof(MI_SYS_ChnPort_t));
        stSrcChnPort.eModId    = E_MI_MODULE_ID_VIF;
        stSrcChnPort.u32DevId  = u32VifDevId;
        stSrcChnPort.u32ChnId  = 0;
        stSrcChnPort.u32PortId = u32VifPortId;
        stDstChnPort.eModId    = E_MI_MODULE_ID_ISP;
        stDstChnPort.u32DevId  = u32IspDevId;
        stDstChnPort.u32ChnId  = u32IspChnId;
        stDstChnPort.u32PortId = 0;
        STCHECKRESULT(MI_SYS_UnBindChnPort(0, &stSrcChnPort, &stDstChnPort));

        /************************************************
        stop isp
        *************************************************/
        STCHECKRESULT(ST_Common_IspDisablePort(u32IspDevId, u32IspChnId, u32IspPortId));
        STCHECKRESULT(ST_Common_IspStopChn(u32IspDevId, u32IspChnId));

        /************************************************
        stop vif/sensor
        *************************************************/
        STCHECKRESULT(ST_Common_VifDisablePort(u32VifDevId, u32VifPortId));
        STCHECKRESULT(ST_Common_VifDisableDev(u32VifDevId));

    }
    return MI_SUCCESS;
}

MI_S32 ST_Common_AovPipeDeInit(ST_Common_AovHandle_t *pstAovHandle)
{
    ST_CHECK_POINTER(pstAovHandle);

    MI_U32 u32SnrPadId   = pstAovHandle->stAovPipeAttr.stVifParam[0].u32SnrPadId;
    MI_U32 u32VifGroupId = pstAovHandle->stAovPipeAttr.stVifParam[0].u32VifGroupId;
    MI_U32 u32VifDevId   = pstAovHandle->stAovPipeAttr.stVifParam[0].u32VifDevId;
    MI_U32 u32IspDevId   = pstAovHandle->stAovPipeAttr.stChnPortIsp[0].u32DevId;
    MI_U32 u32SclDevId   = pstAovHandle->stAovPipeAttr.stChnPortSclPreview[0].u32DevId;
    MI_U32 u32VencDevId  = pstAovHandle->stAovPipeAttr.stChnPortVenc[0].u32DevId;
    MI_U32 i             = 0;

    // Release the unreturned buffer handle
    if (NULL != pstAovHandle->stAovPipeAttr.pstDetBufHandle)
    {
        STCHECKRESULT(ST_Common_PutOutputBufInfo(&(pstAovHandle->stAovPipeAttr.pstDetBufHandle->hDetHandle)));
        free(pstAovHandle->stAovPipeAttr.pstDetBufHandle);
        pstAovHandle->stAovPipeAttr.pstDetBufHandle = NULL;
    }
    for (i = 0; i < pstAovHandle->stAovPipeAttr.u32MaxFrameHandleNum; i++)
    {
        /************************************************
        deinit rgn->venc
        *************************************************/
        STCHECKRESULT(ST_Common_RgnDestroy(0, pstAovHandle->stAovPipeAttr.ahFrame[i]));
    }

    STCHECKRESULT(ST_Common_RgnDestroy(0, pstAovHandle->stAovPipeAttr.hOsd));
    STCHECKRESULT(ST_Common_RgnDeInit(0));

    /************************************************
    deinit venc
    *************************************************/
    STCHECKRESULT(ST_Common_VencDestroyDev(u32VencDevId));

    /************************************************
    deinit scl
    *************************************************/
    STCHECKRESULT(ST_Common_SclDestroyDevice(u32SclDevId));

    /************************************************
    deinit det
    *************************************************/
    STCHECKRESULT(ST_Common_DET_DeInit(&(pstAovHandle->stAovPipeAttr.pDetHandle)));

    /************************************************
    deinit isp
    *************************************************/
    STCHECKRESULT(ST_Common_IspDestroyDevice(u32IspDevId));

    for (i = 0; i < pstAovHandle->stAovPipeAttr.u32SensorNum; i++)
    {
        u32SnrPadId   = pstAovHandle->stAovPipeAttr.stVifParam[i].u32SnrPadId;
        u32VifGroupId = pstAovHandle->stAovPipeAttr.stVifParam[i].u32VifGroupId;
        u32VifDevId   = pstAovHandle->stAovPipeAttr.stVifParam[i].u32VifDevId;

        /************************************************
        deinit vif/sensor
        *************************************************/
        STCHECKRESULT(ST_Common_VifDestroyDevGroup(u32VifGroupId));
        STCHECKRESULT(ST_Common_SensorDeInit(u32SnrPadId));
    }
    if ((pstAovHandle->stLightMiscCtlParam.stAovHWLightSensorParam.bEnableHWLightSensor == TRUE)
        || (pstAovHandle->stLightMiscCtlParam.stAovHWLightSensorParam.bEnableHWLightSensor == TRUE))

    {
        __TurnOnIRCut(pstAovHandle);
        __TurnOffIRLed(pstAovHandle);
        STCHECKRESULT(Dev_Light_Misc_Device_DeInit(pstAovHandle->stLightMiscCtlParam.fd, u32VifDevId));
        STCHECKRESULT(Dev_Light_Misc_Device_CloseFd(pstAovHandle->stLightMiscCtlParam.fd));
    }
    /************************************************
    close rtc
    *************************************************/
    __RtcClose(pstAovHandle->stAovPipeAttr.fdRtc);

    /************************************************
    sys exit
    *************************************************/
    STCHECKRESULT(ST_Common_Sys_Exit());

    return MI_SUCCESS;
}
