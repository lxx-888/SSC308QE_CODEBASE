/* SigmaStar trade secret */
/* Copyright (c) [2019~2022] SigmaStar Technology.
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
#include "ms_platform.h"
#include "rpmsg_dualos.h"
#include "drv_int_ctrl_pub_api.h"
#include "st_common_dualos_sample_rtos.h"
#include "application_selector.h"

static struct rpmsg_lite_instance *st_rpmsg_instance;
static CamOsThread                 rpmsg_dualos_sample_main_thread;
static CamOsThread                 rpmsg_dualos_sample_stop_thread;
static rpmsg_queue_handle          st_rpmsg_queue_handle;
static struct rpmsg_lite_endpoint *st_rpmsg_lite_endpoint;
static CallbackItem                g_dualos_sample_callbacktable[24];
static CamOsThread                 rpmsg_dualos_sample_demo_thread[24];
static FuncInputParam             *stFuncInputParam[24];
static unsigned int                u32Src;
static int                         s32NeedStopThread = -1;

static void __DualosSampleCreateModuleThread(int num, void *arg);

void ST_DUALOS_SAMPLE_RegisterCallback(const char *identifier, Dualos_Sample_CallbackFunc function)
{
    int i = 0;
    for (i = 0; i < sizeof(g_dualos_sample_callbacktable) / sizeof(g_dualos_sample_callbacktable[0]); i++)
    {
        if (g_dualos_sample_callbacktable[i].function == NULL)
        {
            strcpy(g_dualos_sample_callbacktable[i].identifier, identifier);
            g_dualos_sample_callbacktable[i].function = function;
            break;
        }
    }
}

static Dualos_Sample_CallbackFunc __DualosSampleGetCallbackFunction(const char *identifier)
{
    int i = 0;
    for (i = 0; i < sizeof(g_dualos_sample_callbacktable) / sizeof(g_dualos_sample_callbacktable[0]); i++)
    {
        if (strcmp(g_dualos_sample_callbacktable[i].identifier, identifier) == 0)
        {
            if (g_dualos_sample_callbacktable[i].function != NULL)
            {
                return g_dualos_sample_callbacktable[i].function;
            }
        }
    }
    CamOsPrintf("Callback for %s not found.\n", identifier);
    return NULL;
}

static int __DualosSampleStopModule(int num)
{
    stFuncInputParam[num]->use_flag = 0;
    memset(stFuncInputParam[num]->common, 0x0, 256);
    CamOsMemRelease(stFuncInputParam[num]->argv_demo);
    if (CamOsThreadStop(rpmsg_dualos_sample_demo_thread[num]) != CAM_OS_OK)
    {
        CamOsPrintf("dualos_sample_module %d stop failed ...\n", num);
        return -1;
    }
    // CamOsPrintf("dualos_sample_module %d stop ok ...\n", num);
    return 0;
}

static void *__DualosSampleModuleThread(void *arg)
{
    int                        send_data = -1;
    int                              ret = 0;
    char                   send_buf[256] = {0};
    FuncInputParam *stFuncInputParamTemp = (FuncInputParam *)arg;

    send_data = stFuncInputParamTemp->function(stFuncInputParamTemp->argc_demo, stFuncInputParamTemp->argv_demo);
    if (send_data == 0)
    {
        CamOsSprintf(send_buf, "%s pass", stFuncInputParamTemp->argv_demo[0]);
        ret = rpmsg_lite_send(st_rpmsg_instance, st_rpmsg_lite_endpoint, u32Src, send_buf, strlen(send_buf), 5 * 1000);
        if (ret != 0)
        {
            CamOsPrintf("rpmsg_send rpmsg_lite_send return %d\n", ret);
        }
    }
    else if (send_data == -1)
    {
        CamOsSprintf(send_buf, "%s fail", stFuncInputParamTemp->argv_demo[0]);
        ret = rpmsg_lite_send(st_rpmsg_instance, st_rpmsg_lite_endpoint, u32Src, send_buf, strlen(send_buf), 5 * 1000);
        if (ret != 0)
        {
            CamOsPrintf("rpmsg_send rpmsg_lite_send return %d\n", ret);
        }
    }
    else
    {
        CamOsPrintf("bsp call back return not find %d\n", ret);
    }
    s32NeedStopThread = stFuncInputParamTemp->index;
    return NULL;
}

static void *__DualosSampleModuleStopThread(void *arg)
{
    int s32Index = 0;
    while (1)
    {
        if(s32NeedStopThread != -1)
        {
            s32Index = s32NeedStopThread;
            s32NeedStopThread = -1;
            CamOsMsSleep(10);
            __DualosSampleStopModule(s32Index);
        }
        CamOsMsSleep(10);
    }

    return NULL;
}

static int __DualosSampleGetUnuseFuncInputParam(void)
{
    int s32Index = 0;
    int i = 0;
    for (i = 0; i < sizeof(stFuncInputParam) / sizeof(stFuncInputParam[0]); i++)
    {
        if(stFuncInputParam[i]->use_flag == 0)
        {
            stFuncInputParam[i]->use_flag = 1;
            s32Index = i;
            return s32Index;
        }
    }
    return -1;
}

static void __DualosSampleInitFuncInputParam(int argc, char **argv, Dualos_Sample_CallbackFunc function, int s32Index)
{

    stFuncInputParam[s32Index]->index = s32Index;
    stFuncInputParam[s32Index]->argc_demo = argc;
    stFuncInputParam[s32Index]->argv_demo = CamOsMemAlloc(argc * sizeof(char *));
    if (stFuncInputParam[s32Index]->argv_demo == NULL)
    {
        CamOsPrintf("Failed to allocate memory for arguments");
        return;
    }
    for (int j = 0;j < argc; j++)
    {
        stFuncInputParam[s32Index]->argv_demo[j] = argv[j];
        // CamOsPrintf("stFuncInputParam %s\n", stFuncInputParam[s32Index]->argv_demo[j]);
    }
    stFuncInputParam[s32Index]->function = function;

    __DualosSampleCreateModuleThread(s32Index, (void *)stFuncInputParam[s32Index]);
}

static unsigned int __RtosRpmsgInit(void)
{
    unsigned int u32EptAddr;

    CamOsPrintf("Running rpmsg_init ...\n");
    // get rpmsg instance
    while (1)
    {
        st_rpmsg_instance = rpmsg_dualos_get_instance(EPT_SOC_DEFAULT, EPT_OS_LINUX);
        if (st_rpmsg_instance)
            break;
        CamOsPrintf("Ru ...\n");
        CamOsMsSleep(1);
    }
    // create rpmsg queue
    st_rpmsg_queue_handle = rpmsg_queue_create(st_rpmsg_instance);
    if (st_rpmsg_queue_handle == NULL)
    {
        CamOsPrintf("dualos_sample: failed to create queue\n");
        return -1;
    }

    u32EptAddr = EPT_ADDR_PREFIX(EPT_TYPE_SIGMASTAR, EPT_SOC_DEFAULT, EPT_OS_RTOS) | EPT_CLASS(EPT_CLASS_RESERVED) | 0x31;
    st_rpmsg_lite_endpoint = rpmsg_lite_create_ept(st_rpmsg_instance, u32EptAddr, rpmsg_queue_rx_cb, st_rpmsg_queue_handle);
    if (st_rpmsg_lite_endpoint == NULL)
    {
        CamOsPrintf("dualos_sample: failed to create ept\n");
        rpmsg_queue_destroy(st_rpmsg_instance, st_rpmsg_queue_handle);
        return -1;
    }

    return 0;
}

static void *__DualosSampleMainThread(void *arg)
{
    int                        recv_len = 0;
    char                       buf_recv[256];
    char                      *token;
    int                        argc_demo = 0;
    char                      *argv_demo[32];
    int                     s32Index = 0;
    Dualos_Sample_CallbackFunc callback = NULL;

    if(__RtosRpmsgInit() != 0)
    {
        CamOsPrintf("rpmsg_init failed!\n");
        return NULL;
    }
    CamOsPrintf("rpmsg_init ok\n");

    for (int i = 0; i < sizeof(stFuncInputParam) / sizeof(stFuncInputParam[0]); i++)
    {
        stFuncInputParam[i] = CamOsMemAlloc(sizeof(FuncInputParam));
    }

    while (1)
    {
        recv_len  = 0;
        argc_demo = 0;
        memset(buf_recv, 0, sizeof(buf_recv));
        rpmsg_queue_recv(st_rpmsg_instance, st_rpmsg_queue_handle, (unsigned long*)&u32Src, (char *)buf_recv, 256, &recv_len, RL_BLOCK);
        if (recv_len > 0)
        {
            s32Index = __DualosSampleGetUnuseFuncInputParam();
            if(s32Index == -1)
            {
                CamOsPrintf("get index error run thread is full!\n");
                break;
            }

            memcpy(stFuncInputParam[s32Index]->common, buf_recv, recv_len);
            token = strtok(stFuncInputParam[s32Index]->common, " ");
            while (token != NULL)
            {
                argv_demo[argc_demo] = token;
                argc_demo++;
                token = strtok(NULL, " ");
            }

            callback = __DualosSampleGetCallbackFunction(argv_demo[0]);
            if (callback != NULL)
            {
                __DualosSampleInitFuncInputParam(argc_demo, argv_demo, callback, s32Index);
            }
            else
            {
                CamOsPrintf("MCU recv %s\n", stFuncInputParam[s32Index]);
                continue;
            }
        }
        CamOsMsSleep(10);
    }

    return NULL;
}

static void __DualosSampleCreateModuleThread(int num, void *arg)
{
    CamOsThreadAttrb_t attr_read = {.nStackSize = 20480};
    attr_read.szName             = "DualosSampleModuleThread";
    attr_read.nPriority          = 98;
    CamOsThreadCreate(&rpmsg_dualos_sample_demo_thread[num], &attr_read, __DualosSampleModuleThread, arg);
    // CamOsPrintf("dualos_sample_module %d create ok ...\n", num);
}

static void __DualosSampleCreateStopModuleThread(void)
{
    CamOsThreadAttrb_t attr_read = {.nStackSize = 2048};
    attr_read.szName             = "DualosSampleModuleStopThread";
    attr_read.nPriority          = 98;
    CamOsThreadCreate(&rpmsg_dualos_sample_stop_thread, &attr_read, __DualosSampleModuleStopThread, NULL);
}

static void __DualosSampleCreateMainThread(void)
{
    CamOsThreadAttrb_t attr_read = {.nStackSize = 20480};
    attr_read.szName             = "DualosSampleMainThread";
    attr_read.nPriority          = 98;
    CamOsThreadCreate(&rpmsg_dualos_sample_main_thread, &attr_read, __DualosSampleMainThread, NULL);
    CamOsPrintf("dualos_sample_demo create ok ...\n");
}

static int RtosAppMainEntry(int argc, char **argv)
{
    __DualosSampleCreateMainThread();
    __DualosSampleCreateStopModuleThread();
    if (application_selector_retreat() != 0)
    {
        CamOsPrintf("application selector stop failed.\n");
    }
    return 0;
}

rtos_application_selector_initcall(dualos_sample, RtosAppMainEntry);
