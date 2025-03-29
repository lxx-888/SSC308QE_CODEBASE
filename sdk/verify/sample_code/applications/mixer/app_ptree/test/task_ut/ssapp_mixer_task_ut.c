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
#include "ssos_time.h"
#include "ssos_io.h"
#include "ssos_task.h"

#ifndef PTH_RET_CHK
#define PTH_RET_CHK(_pf_)                                                                             \
    (                                                                                                 \
        {                                                                                             \
            int r = _pf_;                                                                             \
            if ((r != 0) && (r != ETIMEDOUT))                                                         \
                SSOS_IO_Printf("[PTHREAD] %s: %d: %s: %s\n", __FILE__, __LINE__, #_pf_, strerror(r)); \
            r;                                                                                        \
        }) while (0)
#endif
static void _SSAPP_MIXER_TASK_UT_PrintTime(void)
{
    SSOS_TIME_Spec_t ts;
    unsigned int     ms;
    SSOS_TIME_Get(&ts);
    ms = (ts.tvNSec * 1000) + (ts.tvNSec / 1000000);
    if (ms == 0)
    {
        ms = 1;
    }
    SSOS_IO_Printf("Time is %u\n", ms);
}
static void *_SSAPP_MIXER_TASK_UT_DoMonitor(struct SSOS_TASK_Buffer_s *pstBuf)
{
    SSOS_IO_Printf("\n############Do monitor!!!##############\n");
    _SSAPP_MIXER_TASK_UT_PrintTime();
    SSOS_IO_Printf("Buffer size  %d\n", pstBuf->size);
    SSOS_IO_Printf("Buffer value %d\n", *(int *)pstBuf->buf);
    SSOS_IO_Printf("############END ###################\n");
    return NULL;
}
static void *_SSAPP_MIXER_TASK_UT_DoSignal(struct SSOS_TASK_Buffer_s *pstBuf, struct SSOS_TASK_UserData_s *data)
{
    SSOS_IO_Printf("\n##########Do DoSignal!!!#############\n");
    _SSAPP_MIXER_TASK_UT_PrintTime();
    SSOS_IO_Printf("Addr %lu\n", (unsigned long)data->data);
    SSOS_IO_Printf("size %d\n", data->size);
    SSOS_IO_Printf("%d\n", *(int *)data->data);
    SSOS_IO_Printf("Buffer size  %d\n", pstBuf->size);
    SSOS_IO_Printf("Buffer value %d\n", *(int *)pstBuf->buf);
    SSOS_IO_Printf("############END ###################\n");
    return NULL;
}
int main(void)
{
    struct SSOS_TASK_Attr_s     ptreeAttr;
    struct SSOS_TASK_UserData_s ptreeData;
    char                        pThreadName0[30] = {0};
    char                        pThreadName1[30] = {0};
    void *                      handle0          = NULL;
    void *                      handle1          = NULL;

    memset(&ptreeAttr, 0, sizeof(struct SSOS_TASK_Attr_s));
    memset(&ptreeData, 0, sizeof(struct SSOS_TASK_UserData_s));
    snprintf(pThreadName0, 30, "DO_SIG_MONITOR_ID%d", 3333);
    ptreeAttr.doSignal         = _SSAPP_MIXER_TASK_UT_DoSignal;
    ptreeAttr.doMonitor        = _SSAPP_MIXER_TASK_UT_DoMonitor;
    ptreeAttr.monitorCycleSec  = 1;
    ptreeAttr.monitorCycleNsec = 0;
    ptreeAttr.isResetTimer     = 0;
    int bbb                    = 6666;
    ptreeAttr.inBuf.buf        = (void *)&bbb;
    ptreeAttr.inBuf.size       = sizeof(int);
    ptreeAttr.threadAttr.name  = pThreadName0;
    handle0                    = SSOS_TASK_Open(&ptreeAttr);
    if (NULL == handle0)
    {
        SSOS_IO_Printf("thread open fail\n");
        return -1;
    }
    bbb                  = 7777;
    ptreeAttr.inBuf.buf  = (void *)&bbb;
    ptreeAttr.inBuf.size = sizeof(int);
    snprintf(pThreadName1, 30, "DO_SIG_MONITOR_ID%d", 4444);
    ptreeAttr.threadAttr.name = pThreadName1;
    handle1                   = SSOS_TASK_Open(&ptreeAttr);
    if (NULL == handle1)
    {
        SSOS_TASK_Close(handle0);
        SSOS_IO_Printf("thread open fail\n");
        return -1;
    }
    SSOS_IO_Printf("open thread done %s, %s\n", pThreadName0, pThreadName1);
    SSOS_TASK_StartMonitor(handle0);
    SSOS_TASK_StartMonitor(handle1);
    SSOS_IO_Printf("start thread %s, %s\n", pThreadName0, pThreadName1);
    int ddddd      = 9999;
    ptreeData.data = &ddddd;
    ptreeData.size = sizeof(int);
    SSOS_TASK_Send(handle0, &ptreeData);
    SSOS_IO_Printf("send thread %s\n", pThreadName0);
    SSOS_TASK_Send(handle0, &ptreeData);
    SSOS_IO_Printf("send thread %s\n", pThreadName1);
    SSOS_TASK_Send(handle1, &ptreeData);
    SSOS_IO_Printf("send thread %s\n", pThreadName0);
    SSOS_TASK_Send(handle0, &ptreeData);
    SSOS_IO_Printf("send thread %s\n", pThreadName1);
    SSOS_TASK_Send(handle1, &ptreeData);
    SSOS_IO_Printf("send thread %s\n", pThreadName0);
    SSOS_TASK_Send(handle0, &ptreeData);
    SSOS_TASK_Send(handle0, &ptreeData);
    SSOS_TIME_MSleep(10 * 1000);
    SSOS_TASK_Stop(handle0);
    SSOS_TASK_Stop(handle1);
    SSOS_IO_Printf("stop thread %s, %s\n", pThreadName0, pThreadName1);
    SSOS_TASK_ConfigTimer(handle0, 5, 0, 1);
    SSOS_TASK_ConfigTimer(handle1, 5, 0, 1);
    SSOS_IO_Printf("reconfig thread %s, %s\n", pThreadName0, pThreadName1);
    SSOS_TASK_OneShot(handle0);
    SSOS_TASK_OneShot(handle1);
    SSOS_IO_Printf("oneshot thread %s, %s\n", pThreadName0, pThreadName1);
    SSOS_TIME_MSleep(10 * 1000);
    SSOS_TASK_Stop(handle0);
    SSOS_TASK_Stop(handle1);
    SSOS_TASK_Close(handle0);
    SSOS_TASK_Close(handle1);
    SSOS_IO_Printf("stop and close thread %s, %s\n", pThreadName0, pThreadName1);
    return 0;
}
