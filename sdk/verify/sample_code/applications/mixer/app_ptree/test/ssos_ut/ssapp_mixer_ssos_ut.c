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
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "ssos_time.h"
#include "ssos_io.h"
#include "ssos_mem.h"
#include "ssos_thread.h"

#define TEST(_func)                                                    \
    do                                                                 \
    {                                                                  \
        SSOS_IO_Printf(">>>>> %s ========================\n", #_func); \
        if (0 != _func())                                              \
        {                                                              \
            SSOS_IO_Printf("- Result: Failed\n");                      \
        }                                                              \
        else                                                           \
        {                                                              \
            SSOS_IO_Printf("- Result: Pass\n");                        \
        }                                                              \
        SSOS_IO_Printf("<<<<< %s ========================\n", #_func); \
    } while (0)

static int _SSAPP_MIXER_SSOS_UT_TestStringIo()
{
    char str[256];
    snprintf(str, 255, "Hello world! %u", 0xffffffff);
    SSOS_IO_Printf("%s\n", str);
    return 0;
}
static int _SSAPP_MIXER_SSOS_UT_TestFileIo()
{
    const char *   str  = "Hello world";
    SSOS_IO_File_t file = SSOS_IO_FileOpen("test.txt", O_WRONLY | O_CREAT, 0755);
    if (!file)
    {
        return -1;
    }
    SSOS_IO_FileSeek(file, 0, SEEK_END);
    if (0 > SSOS_IO_FileWrite(file, str, strlen(str) + 1))
    {
        return -1;
    }
    SSOS_IO_FileClose(file);
    file = SSOS_IO_FileOpen("test.txt", O_RDONLY, 0755);
    if (!file)
    {
        return -1;
    }
    char strOut[8] = "";
    SSOS_IO_FileSeek(file, 5, SEEK_SET);
    SSOS_IO_FileRead(file, strOut, 7);
    strOut[7] = 0;
    SSOS_IO_FileClose(file);
    SSOS_IO_Printf("%s -> %s\n", str, strOut);
    return 0;
}
static int _SSAPP_MIXER_SSOS_UT_TestMem()
{
    return 0;
}
static void *_SSAPP_MIXER_SSOS_UT_TestThreadHandle(void *arg)
{
    int val = (long unsigned int)arg;
    int i   = 0;
    for (i = 0; i < 10; ++i)
    {
        SSOS_IO_Printf("_TestThreadHandle %d, val = %d\n", i, val);
    }
    return NULL;
}
static int _SSAPP_MIXER_SSOS_UT_TestThread()
{
    SSOS_THREAD_Handle_t tHandle = NULL;
    SSOS_THREAD_Create(&tHandle, NULL, _SSAPP_MIXER_SSOS_UT_TestThreadHandle, (void *)100);
    SSOS_THREAD_Join(tHandle);
    return 0;
}
typedef struct SSAPP_MIXER_SSOS_UT_TestMutex_s
{
    SSOS_THREAD_Mutex_t mutex;
    int                 data;
} SSAPP_MIXER_SSOS_UT_TestMutex_t;
static SSAPP_MIXER_SSOS_UT_TestMutex_t g_testMutex = {0};

static void *_SSAPP_MIXER_SSOS_UT_TestMutexHandle(void *arg)
{
    int id = (long unsigned int)arg;
    while (1)
    {
        SSOS_THREAD_MutexLock(&g_testMutex.mutex);
        if (g_testMutex.data >= 10)
        {
            SSOS_THREAD_MutexUnlock(&g_testMutex.mutex);
            break;
        }
        SSOS_IO_Printf("_SSAPP_MIXER_SSOS_UT_TestMutexHandle[%d], data = %d\n", id, g_testMutex.data++);
        SSOS_THREAD_MutexUnlock(&g_testMutex.mutex);
        SSOS_TIME_MSleep(100);
    }
    return NULL;
}
static int _SSAPP_MIXER_SSOS_UT_TestMutex()
{
    int                  i          = 0;
    SSOS_THREAD_Handle_t tHandles[] = {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL};
    SSOS_THREAD_MutexInit(&g_testMutex.mutex);
    for (i = 0; i < sizeof(tHandles) / sizeof(tHandles[0]); ++i)
    {
        SSOS_THREAD_Create(&tHandles[i], NULL, _SSAPP_MIXER_SSOS_UT_TestMutexHandle, (void *)(unsigned long)i);
    }
    for (i = 0; i < sizeof(tHandles) / sizeof(tHandles[0]); ++i)
    {
        SSOS_THREAD_Join(tHandles[i]);
    }
    SSOS_THREAD_MutexDeinit(&g_testMutex.mutex);
    return 0;
}

typedef struct SSAPP_MIXER_SSOS_UT_TestRwLock_s
{
    SSOS_THREAD_RwLock_t lock;
    int                  data;
} SSAPP_MIXER_SSOS_UT_TestRwLock_t;

static SSAPP_MIXER_SSOS_UT_TestRwLock_t g_testRwLock = {0};

static void *_SSAPP_MIXER_SSOS_UT_TestRwLockReadHandle(void *arg)
{
    int id = (long unsigned int)arg;
    while (1)
    {
        SSOS_THREAD_ReadLock(&g_testRwLock.lock);
        if (g_testRwLock.data >= 5)
        {
            SSOS_THREAD_ReadUnLock(&g_testRwLock.lock);
            break;
        }
        SSOS_IO_Printf("_SSAPP_MIXER_SSOS_UT_TestRwLockReadHandle[%d], data = %d\n", id, g_testRwLock.data);
        SSOS_THREAD_ReadUnLock(&g_testRwLock.lock);
        SSOS_TIME_MSleep(100);
    }
    return NULL;
}
static void *_SSAPP_MIXER_SSOS_UT_TestRwLockWriteHandle(void *arg)
{
    int id = (long unsigned int)arg;
    while (1)
    {
        SSOS_THREAD_WriteLock(&g_testRwLock.lock);
        if (g_testRwLock.data >= 5)
        {
            SSOS_THREAD_WriteUnlock(&g_testRwLock.lock);
            break;
        }
        SSOS_IO_Printf("_SSAPP_MIXER_SSOS_UT_TestRwLockWriteHandle[%d], data = %d\n", id, g_testRwLock.data++);
        SSOS_THREAD_WriteUnlock(&g_testRwLock.lock);
        SSOS_TIME_MSleep(200);
    }
    return NULL;
}
static int _SSAPP_MIXER_SSOS_UT_TestRwLock()
{
    int                  i              = 0;
    SSOS_THREAD_Handle_t tReadHandles[] = {NULL, NULL, NULL};
    SSOS_THREAD_Handle_t tWriteHandle   = NULL;
    SSOS_THREAD_RwLockInit(&g_testRwLock.lock);
    SSOS_THREAD_Create(&tReadHandles[i], NULL, _SSAPP_MIXER_SSOS_UT_TestRwLockWriteHandle, (void *)(unsigned long)100);
    for (i = 0; i < sizeof(tReadHandles) / sizeof(tReadHandles[0]); ++i)
    {
        SSOS_THREAD_Create(&tReadHandles[i], NULL, _SSAPP_MIXER_SSOS_UT_TestRwLockReadHandle, (void *)(unsigned long)i);
    }
    for (i = 0; i < sizeof(tReadHandles) / sizeof(tReadHandles[0]); ++i)
    {
        SSOS_THREAD_Join(tReadHandles[i]);
    }
    SSOS_THREAD_Join(tWriteHandle);
    SSOS_THREAD_RwLockDeinit(&g_testRwLock.lock);
    return 0;
}

typedef struct SSAPP_MIXER_SSOS_UT_TestCond_s
{
    SSOS_THREAD_Cond_t cond;
    int                data;
} SSAPP_MIXER_SSOS_UT_TestCond_t;

static SSAPP_MIXER_SSOS_UT_TestCond_t g_testCond = {0};

static void *_SSAPP_MIXER_SSOS_UT_TestCondHandle(void *arg)
{
    int              id = (long unsigned int)arg;
    SSOS_TIME_Spec_t tv0, tv1;
    SSOS_IO_Printf("_SSAPP_MIXER_SSOS_UT_TestCondHandle[%d], is running\n", id);
    SSOS_TIME_Get(&tv0);
    SSOS_THREAD_COND_WAIT(&g_testCond.cond, g_testCond.data == 100);
    SSOS_TIME_Get(&tv1);
    SSOS_IO_Printf("_SSAPP_MIXER_SSOS_UT_TestCondHandle[%d], is stoping\n", id);
    SSOS_IO_Printf("%02llu:%03llu:%03llu:%03llu ~ %02llu:%03llu:%03llu:%03llu\n", tv0.tvSec, tv0.tvNSec / 1000000,
                   (tv0.tvNSec / 1000) % 1000, tv0.tvNSec % 1000, tv1.tvSec, tv1.tvNSec / 1000000,
                   (tv1.tvNSec / 1000) % 1000, tv1.tvNSec % 1000);
    return NULL;
}
static int _SSAPP_MIXER_SSOS_UT_TestCond()
{
    SSOS_THREAD_COND_INIT(&g_testCond.cond);
    SSOS_THREAD_Handle_t tHandle = NULL;
    SSOS_THREAD_Create(&tHandle, NULL, _SSAPP_MIXER_SSOS_UT_TestCondHandle, (void *)100);
    SSOS_TIME_MSleep(200);
    g_testCond.data = 100;
    SSOS_THREAD_COND_WAKE(&g_testCond.cond);
    SSOS_THREAD_Join(tHandle);
    SSOS_THREAD_COND_DEINIT(&g_testCond.cond);
    return 0;
}

typedef struct SSAPP_MIXER_SSOS_UT_TestCondTime_s
{
    SSOS_THREAD_Cond_t cond;
    int                data;
} SSAPP_MIXER_SSOS_UT_TestCondTime_t;
static SSAPP_MIXER_SSOS_UT_TestCondTime_t g_testCondTime = {0};
static void *                             _SSAPP_MIXER_SSOS_UT_TestCondTimeHandle(void *arg)
{
    int              id      = (long unsigned int)arg;
    SSOS_TIME_Spec_t timeOut = {.tvSec = 0, .tvNSec = 20 * 1000 * 1000};
    SSOS_TIME_Spec_t tv0, tv1;
    SSOS_IO_Printf("_SSAPP_MIXER_SSOS_UT_TestCondTimeHandle[%d], is running\n", id);
    SSOS_TIME_Get(&tv0);
    SSOS_THREAD_COND_TIME_WAIT(&g_testCondTime.cond, g_testCondTime.data == 100, timeOut);
    SSOS_TIME_Get(&tv1);
    SSOS_IO_Printf("_SSAPP_MIXER_SSOS_UT_TestCondTimeHandle[%d], is stoping\n", id);
    SSOS_IO_Printf("%02llu:%03llu:%03llu:%03llu ~ %02llu:%03llu:%03llu:%03llu\n", tv0.tvSec, tv0.tvNSec / 1000000,
                   (tv0.tvNSec / 1000) % 1000, tv0.tvNSec % 1000, tv1.tvSec, tv1.tvNSec / 1000000,
                   (tv1.tvNSec / 1000) % 1000, tv1.tvNSec % 1000);
    return NULL;
}
static int _SSAPP_MIXER_SSOS_UT_TestCondTime()
{
    SSOS_THREAD_COND_INIT(&g_testCondTime.cond);
    SSOS_THREAD_Handle_t tHandle = NULL;
    SSOS_THREAD_Create(&tHandle, NULL, _SSAPP_MIXER_SSOS_UT_TestCondTimeHandle, (void *)100);
    SSOS_TIME_MSleep(500);
    g_testCondTime.data = 100;
    SSOS_THREAD_COND_WAKE(&g_testCondTime.cond);
    SSOS_THREAD_Join(tHandle);
    SSOS_THREAD_COND_DEINIT(&g_testCondTime.cond);
    return 0;
}

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        SSOS_IO_Printf("Test all...\n");
        TEST(_SSAPP_MIXER_SSOS_UT_TestFileIo);
        TEST(_SSAPP_MIXER_SSOS_UT_TestStringIo);
        TEST(_SSAPP_MIXER_SSOS_UT_TestMem);
        TEST(_SSAPP_MIXER_SSOS_UT_TestThread);
        TEST(_SSAPP_MIXER_SSOS_UT_TestMutex);
        TEST(_SSAPP_MIXER_SSOS_UT_TestRwLock);
        TEST(_SSAPP_MIXER_SSOS_UT_TestCond);
        TEST(_SSAPP_MIXER_SSOS_UT_TestCondTime);
        return 0;
    }
    if (0 == strcmp(argv[1], "file"))
    {
        TEST(_SSAPP_MIXER_SSOS_UT_TestFileIo);
    }
    else if (0 == strcmp(argv[1], "string"))
    {
        TEST(_SSAPP_MIXER_SSOS_UT_TestStringIo);
    }
    else if (0 == strcmp(argv[1], "mem"))
    {
        TEST(_SSAPP_MIXER_SSOS_UT_TestMem);
    }
    else if (0 == strcmp(argv[1], "thread"))
    {
        TEST(_SSAPP_MIXER_SSOS_UT_TestThread);
    }
    else if (0 == strcmp(argv[1], "mutex"))
    {
        TEST(_SSAPP_MIXER_SSOS_UT_TestMutex);
    }
    else if (0 == strcmp(argv[1], "rwlock"))
    {
        TEST(_SSAPP_MIXER_SSOS_UT_TestRwLock);
    }
    else if (0 == strcmp(argv[1], "cond"))
    {
        TEST(_SSAPP_MIXER_SSOS_UT_TestCond);
    }
    else if (0 == strcmp(argv[1], "condtime"))
    {
        TEST(_SSAPP_MIXER_SSOS_UT_TestCondTime);
    }
    else
    {
        SSOS_IO_Printf("Not found %s\n", argv[1]);
    }
    return 0;
}
