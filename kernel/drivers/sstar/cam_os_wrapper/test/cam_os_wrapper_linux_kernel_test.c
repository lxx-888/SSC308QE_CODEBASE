/*
 * cam_os_wrapper_linux_kernel_test.c- Sigmastar
 *
 * Copyright (c) [2019~2020] SigmaStar Technology.
 *
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License version 2 for more details.
 *
 */

///////////////////////////////////////////////////////////////////////////////
/// @file      cam_os_wrapper_linux_kernel_test.c
/// @brief     Cam OS Wrapper Test Code for Linux Kernel Space
///////////////////////////////////////////////////////////////////////////////

#include <linux/module.h>
#include "cam_os_wrapper.h"

MODULE_AUTHOR("SSTAR");
MODULE_DESCRIPTION("SStar Kernel Wrapper Test");
MODULE_LICENSE("GPL");

// #define TEST_TIME_AND_SLEEP
// #define TEST_PHY_MAP_VIRT
// #define TEST_CACHE_ALLOC
// #define TEST_IDR
// #define TEST_THREAD
// #define TEST_THREAD_PRIORITY
// #define TEST_SEMAPHORE
// #define TEST_RW_SEMAPHORE
// #define TEST_MUTEX
// #define TEST_DIV64
// #define TEST_SYSTEM_TIME
// #define TEST_MEM_SIZE
// #define TEST_CHIP_ID
// #define TEST_ATOMIC_OPERATION
// #define TEST_BITMAP
// #define TEST_HASH
// #define TEST_TIMER
// #define TEST_SPINLOCK
// #define TEST_CONDITION
// #define TEST_CPU_RUNTIME
// #define TEST_WORK_QUEUE
// #define TEST_MEMCPY
// #define TEST_MEMMOVE
// #define TEST_MEMSET
// #define TEST_MEMCMP
// #define TEST_STRSTR
// #define TEST_STRNCPY
// #define TEST_STRNCMP
// #define TEST_STRNCASECMP
// #define TEST_STRLEN
// #define TEST_STRCAT
// #define TEST_STRSEP
// #define TEST_STRCHR
// #define TEST_STRRCHR
// #define TEST_QSORT

#ifdef TEST_TIME_AND_SLEEP
CamOsThread tTimeThread0;
static int  ThreadTestTimeSleep(void *pUserData)
{
    CamOsTimespec_t tTimeStart   = {0};
    CamOsTimespec_t tTimeEnd     = {0};
    u32             nTestCounter = 0;

    for (nTestCounter = 0; nTestCounter < 10; nTestCounter++)
    {
        if (CAM_OS_OK == CamOsThreadShouldStop())
            break;

        CamOsGetMonotonicTime(&tTimeStart);
        CamOsMsSleep(1000);
        CamOsGetMonotonicTime(&tTimeEnd);
        CamOsPrintf("%s  set sleep 1000 ms, real sleep %lld us\n", __FUNCTION__,
                    CamOsTimeDiff(&tTimeStart, &tTimeEnd, CAM_OS_TIME_DIFF_US));
    }

    for (nTestCounter = 0; nTestCounter < 10; nTestCounter++)
    {
        if (CAM_OS_OK == CamOsThreadShouldStop())
            break;

        CamOsGetMonotonicTime(&tTimeStart);
        CamOsMsSleep(100);
        CamOsGetMonotonicTime(&tTimeEnd);
        CamOsPrintf("%s  set sleep 100 ms, real sleep %lld us\n", __FUNCTION__,
                    CamOsTimeDiff(&tTimeStart, &tTimeEnd, CAM_OS_TIME_DIFF_US));
    }

    for (nTestCounter = 0; nTestCounter < 10; nTestCounter++)
    {
        if (CAM_OS_OK == CamOsThreadShouldStop())
            break;

        CamOsGetMonotonicTime(&tTimeStart);
        CamOsMsSleep(10);
        CamOsGetMonotonicTime(&tTimeEnd);
        CamOsPrintf("%s  set sleep 10 ms, real sleep %lld us\n", __FUNCTION__,
                    CamOsTimeDiff(&tTimeStart, &tTimeEnd, CAM_OS_TIME_DIFF_US));
    }

    for (nTestCounter = 0; nTestCounter < 10; nTestCounter++)
    {
        if (CAM_OS_OK == CamOsThreadShouldStop())
            break;

        CamOsGetMonotonicTime(&tTimeStart);
        CamOsMsSleep(1);
        CamOsGetMonotonicTime(&tTimeEnd);
        CamOsPrintf("%s  set sleep 1 ms, real sleep %lld us\n", __FUNCTION__,
                    CamOsTimeDiff(&tTimeStart, &tTimeEnd, CAM_OS_TIME_DIFF_US));
    }

    for (nTestCounter = 0; nTestCounter < 10; nTestCounter++)
    {
        if (CAM_OS_OK == CamOsThreadShouldStop())
            break;

        CamOsGetMonotonicTime(&tTimeStart);
        CamOsUsSleep(1000);
        CamOsGetMonotonicTime(&tTimeEnd);
        CamOsPrintf("%s  set sleep 1000 us, real sleep %lld us\n", __FUNCTION__,
                    CamOsTimeDiff(&tTimeStart, &tTimeEnd, CAM_OS_TIME_DIFF_US));
    }

    for (nTestCounter = 0; nTestCounter < 10; nTestCounter++)
    {
        if (CAM_OS_OK == CamOsThreadShouldStop())
            break;

        CamOsGetMonotonicTime(&tTimeStart);
        CamOsUsSleep(100);
        CamOsGetMonotonicTime(&tTimeEnd);
        CamOsPrintf("%s  set sleep 100 us, real sleep %lld us\n", __FUNCTION__,
                    CamOsTimeDiff(&tTimeStart, &tTimeEnd, CAM_OS_TIME_DIFF_US));
    }

    for (nTestCounter = 0; nTestCounter < 10; nTestCounter++)
    {
        if (CAM_OS_OK == CamOsThreadShouldStop())
            break;

        CamOsGetMonotonicTime(&tTimeStart);
        CamOsUsSleep(10);
        CamOsGetMonotonicTime(&tTimeEnd);
        CamOsPrintf("%s  set sleep 10 us, real sleep %lld us\n", __FUNCTION__,
                    CamOsTimeDiff(&tTimeStart, &tTimeEnd, CAM_OS_TIME_DIFF_US));
    }

    for (nTestCounter = 0; nTestCounter < 10; nTestCounter++)
    {
        if (CAM_OS_OK == CamOsThreadShouldStop())
            break;

        CamOsGetMonotonicTime(&tTimeStart);
        CamOsUsSleep(1);
        CamOsGetMonotonicTime(&tTimeEnd);
        CamOsPrintf("%s  set sleep 1 us, real sleep %lld us\n", __FUNCTION__,
                    CamOsTimeDiff(&tTimeStart, &tTimeEnd, CAM_OS_TIME_DIFF_US));
    }

    for (nTestCounter = 0; nTestCounter < 10; nTestCounter++)
    {
        if (CAM_OS_OK == CamOsThreadShouldStop())
            break;

        CamOsGetMonotonicTime(&tTimeStart);
        CamOsMsDelay(1);
        CamOsGetMonotonicTime(&tTimeEnd);
        CamOsPrintf("%s  set delay 1 ms, real delay %lld us\n", __FUNCTION__,
                    CamOsTimeDiff(&tTimeStart, &tTimeEnd, CAM_OS_TIME_DIFF_US));
    }

    for (nTestCounter = 0; nTestCounter < 10; nTestCounter++)
    {
        if (CAM_OS_OK == CamOsThreadShouldStop())
            break;

        CamOsGetMonotonicTime(&tTimeStart);
        CamOsUsDelay(1);
        CamOsGetMonotonicTime(&tTimeEnd);
        CamOsPrintf("%s  set delay 1 us, real delay %lld ns\n", __FUNCTION__,
                    CamOsTimeDiff(&tTimeStart, &tTimeEnd, CAM_OS_TIME_DIFF_NS));
    }

    while (1)
    {
        if (CAM_OS_OK == CamOsThreadShouldStop())
            break;

        CamOsMsSleep(1000);
    }

    CamOsPrintf("%s break\n", __FUNCTION__);

    return 0;
}

int KernelTestTimeAndSleep(void)
{
    CamOsPrintf("=================================\n");
    CamOsPrintf("Test time and sleep\n");
    CamOsPrintf("=================================\n");
    CamOsThreadCreate(&tTimeThread0, NULL, (void *)ThreadTestTimeSleep, NULL);
    CamOsMsSleep(15000);
    CamOsThreadStop(tTimeThread0);

    return 0;
}
#endif

#ifdef TEST_PHY_MAP_VIRT
int KernelTestPhyMapVirt(void)
{
    ss_phys_addr_t PhysAddr0 = 0;
    ss_phys_addr_t PhysAddr1 = 0;
    void *         pVirtPtr  = NULL;

#define TEST_PHY_MEM_SIZE 0x100000

    CamOsPrintf("=================================\n");
    CamOsPrintf("Test phy mem map to virtual addr\n");
    CamOsPrintf("=================================\n");

    PhysAddr0 = CamOsContiguousMemAlloc(TEST_PHY_MEM_SIZE);
    PhysAddr1 = PhysAddr0 + 0x123;

    pVirtPtr = CamOsMemMap(PhysAddr0, TEST_PHY_MEM_SIZE, 1);
    CamOsPrintf("0x%016llx(phys) map to 0x%016llx(virt), VirtToPhys: 0x%016llx\n", PhysAddr0, pVirtPtr,
                CamOsMemVirtToPhys(pVirtPtr));
    CamOsMemUnmap(pVirtPtr, TEST_PHY_MEM_SIZE);

    pVirtPtr = CamOsMemMap(PhysAddr1, TEST_PHY_MEM_SIZE, 1);
    CamOsPrintf("0x%016llx(phys) map to 0x%016llx(virt), VirtToPhys: 0x%016llx\n", PhysAddr1, pVirtPtr,
                CamOsMemVirtToPhys(pVirtPtr));
    CamOsMemUnmap(pVirtPtr, TEST_PHY_MEM_SIZE);

    CamOsContiguousMemRelease(PhysAddr0);

    return 0;
}
#endif

#ifdef TEST_CACHE_ALLOC
int KernelTestCacheAlloc(void)
{
    CamOsMemCache_t tMemCache;
    void *          pMemCacheObj1, *pMemCacheObj2, *pMemCacheObj3;

#define MEMORY_CACHE_OBJECT_SIZE 0x50

    CamOsPrintf("=================================\n");
    CamOsPrintf("Test memory cache\n");
    CamOsPrintf("=================================\n");
    CamOsPrintf("Test non-HW cache alignment mapping:\n");
    if (CAM_OS_OK == CamOsMemCacheCreate(&tMemCache, "MemCacheTest", MEMORY_CACHE_OBJECT_SIZE, 0))
    {
        pMemCacheObj1 = CamOsMemCacheAlloc(&tMemCache);
        CamOsPrintf("Object1 address: 0x%08X\n", pMemCacheObj1);
        pMemCacheObj2 = CamOsMemCacheAlloc(&tMemCache);
        CamOsPrintf("Object2 address: 0x%08X\n", pMemCacheObj2);
        pMemCacheObj3 = CamOsMemCacheAlloc(&tMemCache);
        CamOsPrintf("Object3 address: 0x%08X\n", pMemCacheObj3);
        CamOsMemCacheFree(&tMemCache, pMemCacheObj1);
        CamOsMemCacheFree(&tMemCache, pMemCacheObj2);
        CamOsMemCacheFree(&tMemCache, pMemCacheObj3);
        CamOsMemCacheDestroy(&tMemCache);
    }
    else
        CamOsPrintf("CamOsMemCacheCreate fail!\n");

    CamOsPrintf("Test HW cache alignment mapping:\n");
    if (CAM_OS_OK == CamOsMemCacheCreate(&tMemCache, "MemCacheTest", MEMORY_CACHE_OBJECT_SIZE, 1))
    {
        pMemCacheObj1 = CamOsMemCacheAlloc(&tMemCache);
        CamOsPrintf("Object1 address: 0x%08X\n", pMemCacheObj1);
        pMemCacheObj2 = CamOsMemCacheAlloc(&tMemCache);
        CamOsPrintf("Object2 address: 0x%08X\n", pMemCacheObj2);
        pMemCacheObj3 = CamOsMemCacheAlloc(&tMemCache);
        CamOsPrintf("Object3 address: 0x%08X\n", pMemCacheObj3);
        CamOsMemCacheFree(&tMemCache, pMemCacheObj1);
        CamOsMemCacheFree(&tMemCache, pMemCacheObj2);
        CamOsMemCacheFree(&tMemCache, pMemCacheObj3);
        CamOsMemCacheDestroy(&tMemCache);
    }
    else
        CamOsPrintf("CamOsMemCacheCreate fail!\n");

    return 0;
}
#endif

#ifdef TEST_IDR
int KernelTestIdr(void)
{
    CamOsIdr_t tIdr;
    uint32_t   nIdrData1 = 11111, nIdrData2 = 22222, nIdrData3 = 33333, *pnIdrDataPtr;
    int32_t    nIdrId1, nIdrId2, nIdrId3;

    CamOsPrintf("=================================\n");
    CamOsPrintf("Test IDR operation\n");
    CamOsPrintf("=================================\n");
    if (CAM_OS_OK == CamOsIdrInit(&tIdr))
    {
        CamOsPrintf("Alloc data1(=%u) in 100~200\n", nIdrData1);
        nIdrId1 = CamOsIdrAlloc(&tIdr, (void *)&nIdrData1, 100, 200);
        CamOsPrintf("Alloc data2(=%u) in 100~200\n", nIdrData2);
        nIdrId2 = CamOsIdrAlloc(&tIdr, (void *)&nIdrData2, 100, 200);
        CamOsPrintf("Alloc data3(=%u) in 500~\n", nIdrData3);
        nIdrId3      = CamOsIdrAlloc(&tIdr, (void *)&nIdrData3, 500, 0);
        pnIdrDataPtr = (uint32_t *)CamOsIdrFind(&tIdr, nIdrId1);
        CamOsPrintf("ID1 = %d, find data = %u\n", nIdrId1, *pnIdrDataPtr);
        pnIdrDataPtr = (uint32_t *)CamOsIdrFind(&tIdr, nIdrId2);
        CamOsPrintf("ID2 = %d, find data = %u\n", nIdrId2, *pnIdrDataPtr);
        pnIdrDataPtr = (uint32_t *)CamOsIdrFind(&tIdr, nIdrId3);
        CamOsPrintf("ID3 = %d, find data = %u\n", nIdrId3, *pnIdrDataPtr);

        CamOsPrintf("Remove ID3(=%d) ... ", nIdrId3);
        CamOsIdrRemove(&tIdr, nIdrId3);
        if (NULL != CamOsIdrFind(&tIdr, nIdrId3))
            CamOsPrintf("fail! find removed item\n");

        CamOsPrintf("IdrAlloc %d\n", CamOsIdrAlloc(&tIdr, (void *)&nIdrData3, 200, 220));             // get 200
        CamOsPrintf("IdrAlloc %d\n", CamOsIdrAlloc(&tIdr, (void *)&nIdrData3, 200, 220));             // get 201
        CamOsPrintf("IdrAlloc %d\n", CamOsIdrAlloc(&tIdr, (void *)&nIdrData3, 200, 220));             // get 202
        CamOsPrintf("IdrAlloc %d\n", CamOsIdrAlloc(&tIdr, (void *)&nIdrData3, 200, 220));             // get 203
        CamOsIdrRemove(&tIdr, 201);                                                                   // remove 201
        CamOsIdrRemove(&tIdr, 202);                                                                   // remove 202
        CamOsPrintf("IdrAlloc %d\n", CamOsIdrAlloc(&tIdr, (void *)&nIdrData3, 200, 220));             // get 201
        CamOsPrintf("IdrAlloc %d\n", CamOsIdrAlloc(&tIdr, (void *)&nIdrData3, 200, 220));             // get 202
        CamOsPrintf("IdrAlloc %d\n", CamOsIdrAlloc(&tIdr, (void *)&nIdrData3, 200, 220));             // get 204
        CamOsPrintf("IdrAlloc %d\n", CamOsIdrAllocCyclic(&tIdr, (void *)&nIdrData3, 200, 220));       // get 205
        CamOsPrintf("IdrAlloc %d\n", CamOsIdrAllocCyclic(&tIdr, (void *)&nIdrData3, 200, 220));       // get 206
        CamOsPrintf("IdrAlloc %d\n", CamOsIdrAllocCyclic(&tIdr, (void *)&nIdrData3, 200, 220));       // get 207
        CamOsPrintf("IdrAlloc %d\n", CamOsIdrAllocCyclic(&tIdr, (void *)&nIdrData3, 200, 220));       // get 208
        CamOsIdrRemove(&tIdr, 206);                                                                   // remove 206
        CamOsIdrRemove(&tIdr, 207);                                                                   // remove 207
        CamOsPrintf("IdrAllocCyclic %d\n", CamOsIdrAllocCyclic(&tIdr, (void *)&nIdrData3, 200, 220)); // get 208
        CamOsPrintf("IdrAllocCyclic %d\n", CamOsIdrAllocCyclic(&tIdr, (void *)&nIdrData3, 200, 220)); // get 209
        CamOsPrintf("IdrAllocCyclic %d\n", CamOsIdrAllocCyclic(&tIdr, (void *)&nIdrData3, 200, 220)); // get 210
        CamOsPrintf("IdrAllocCyclic %d\n", CamOsIdrAllocCyclic(&tIdr, (void *)&nIdrData3, 200, 220)); // get 211

        CamOsIdrDestroy(&tIdr);
    }
    else
        CamOsPrintf("CamOsIdrInit fail!\n");

    return 0;
}
#endif

#ifdef TEST_THREAD
CamOsAtomic_t      _gtThreadAtomic;
CamOsThreadAttrb_t tThreadAttr;
CamOsThread        tThread0, tThread1;
static int         ThreadTest1(void *pUserData)
{
    int32_t nTestCounter = 0;

    CamOsMsSleep(10);

    while (nTestCounter < 10000)
    {
        nTestCounter++;
        if ((nTestCounter % 500) == 0)
            CamOsPrintf("%s run...\n", __FUNCTION__);
    }

    CamOsAtomicIncReturn(&_gtThreadAtomic);
    while (CamOsAtomicRead(&_gtThreadAtomic) < 3)
        CamOsMsSleep(100);

    CamOsPrintf("%s free run end\n", __FUNCTION__);

    while (CAM_OS_OK != CamOsThreadShouldStop())
    {
        if (CamOsAtomicRead(&_gtThreadAtomic) == 3)
        {
            CamOsPrintf("%s enter CamOsThreadSchedule\n", __FUNCTION__);
            CamOsThreadSchedule(0, 2000);
            CamOsPrintf("%s leave CamOsThreadSchedule\n", __FUNCTION__);
            CamOsAtomicIncReturn(&_gtThreadAtomic);
        }

        CamOsPrintf("%s run...\n", __FUNCTION__);
        CamOsMsSleep(300);
    }

    CamOsPrintf("%s break\n", __FUNCTION__);

    return 0;
}

static int ThreadTest2(void *pUserData)
{
    int32_t nTestCounter = 0;

    CamOsMsSleep(10);

    while (nTestCounter < 10000)
    {
        nTestCounter++;
        if ((nTestCounter % 500) == 0)
            CamOsPrintf("%s run...\n", __FUNCTION__);
    }

    CamOsAtomicIncReturn(&_gtThreadAtomic);
    while (CamOsAtomicRead(&_gtThreadAtomic) < 3)
        CamOsMsSleep(100);

    CamOsPrintf("%s free run end\n", __FUNCTION__);

    while (CAM_OS_OK != CamOsThreadShouldStop())
    {
        if (CamOsAtomicRead(&_gtThreadAtomic) == 4)
        {
            CamOsPrintf("%s enter CamOsThreadSchedule\n", __FUNCTION__);
            CamOsThreadSchedule(1, CAM_OS_MAX_TIMEOUT);
            CamOsPrintf("%s leave CamOsThreadSchedule\n", __FUNCTION__);
            CamOsAtomicIncReturn(&_gtThreadAtomic);
        }
        CamOsPrintf("%s run...\n", __FUNCTION__);
        CamOsMsSleep(300);
    }

    CamOsPrintf("%s break\n", __FUNCTION__);

    return 0;
}

static int ThreadTest3(void *pUserData)
{
    while (CamOsAtomicRead(&_gtThreadAtomic) < 1)
    {
        CamOsPrintf("%s run...\n", __FUNCTION__);
        CamOsMsSleep(300);
    }

    CamOsPrintf("%s break\n", __FUNCTION__);

    return 0;
}

int KernelTestThread(void)
{
    CamOsPrintf("=================================\n");
    CamOsPrintf("Test thread\n");
    CamOsPrintf("=================================\n");
    CamOsAtomicSet(&_gtThreadAtomic, 0);
    CamOsPrintf("### Priority: ThreadTest1 < ThreadTest2\n");
    tThreadAttr.nPriority  = 1;
    tThreadAttr.nStackSize = 3072;
    CamOsThreadCreate(&tThread0, &tThreadAttr, (void *)ThreadTest1, NULL);
    tThreadAttr.nPriority  = 10;
    tThreadAttr.nStackSize = 3072;
    CamOsThreadCreate(&tThread1, &tThreadAttr, (void *)ThreadTest2, NULL);

    while (CamOsAtomicRead(&_gtThreadAtomic) < 2)
        CamOsMsSleep(100);

    CamOsMsSleep(1000);

    CamOsAtomicIncReturn(&_gtThreadAtomic);

    while (CamOsAtomicRead(&_gtThreadAtomic) < 4)
        CamOsMsSleep(100);

    CamOsMsSleep(2000);

    CamOsPrintf("### Wake up ThreadTest2\n");
    CamOsThreadWakeUp(tThread1);

    CamOsMsSleep(1000);

    CamOsThreadStop(tThread0);
    CamOsThreadStop(tThread1);

    CamOsPrintf("Test for CamOsThreadJoin (thread exit before CamOsThreadJoin)\n");
    CamOsAtomicSet(&_gtThreadAtomic, 1);
    tThreadAttr.nPriority  = 10;
    tThreadAttr.nStackSize = 3072;
    CamOsThreadCreate(&tThread0, &tThreadAttr, (void *)ThreadTest3, NULL);
    CamOsMsSleep(100);
    CamOsThreadJoin(tThread0);

    CamOsPrintf("Test for CamOsThreadJoin (thread exit after CamOsThreadJoin)\n");
    CamOsAtomicSet(&_gtThreadAtomic, 0);
    tThreadAttr.nPriority  = 10;
    tThreadAttr.nStackSize = 3072;
    CamOsThreadCreate(&tThread0, &tThreadAttr, (void *)ThreadTest3, NULL);
    CamOsMsSleep(100);
    CamOsAtomicSet(&_gtThreadAtomic, 1);
    CamOsThreadJoin(tThread0);

    return 0;
}
#endif

#ifdef TEST_THREAD_PRIORITY
#define THREAD_PRIORITY_TEST_NUM 100
CamOsThreadAttrb_t tThreadAttrPrio;
CamOsThread        tThreadPrio[THREAD_PRIORITY_TEST_NUM];
static int         ThreadTestPrio(void *pUserData)
{
    CamOsPrintf("%s create\n", __FUNCTION__);

    while (CAM_OS_OK != CamOsThreadShouldStop())
    {
        CamOsMsSleep(10);
    }

    CamOsPrintf("%s break\n", __FUNCTION__);

    return 0;
}

int KernelTestThreadPriority(void)
{
    uint32_t nThreadPrioCnt;
    char     szThreadName[16];

    CamOsPrintf("=================================\n");
    CamOsPrintf("Test thread Priority\n");
    CamOsPrintf("=================================\n");

    for (nThreadPrioCnt = 0; nThreadPrioCnt < THREAD_PRIORITY_TEST_NUM; nThreadPrioCnt++)
    {
        tThreadAttrPrio.nPriority  = nThreadPrioCnt;
        tThreadAttrPrio.nStackSize = 3072;
        CamOsSnprintf(szThreadName, sizeof(szThreadName), "ThreadPrio%d", nThreadPrioCnt);
        tThreadAttrPrio.szName = szThreadName;
        CamOsThreadCreate(&tThreadPrio[nThreadPrioCnt], &tThreadAttrPrio, (void *)ThreadTestPrio, NULL);
    }

    CamOsMsSleep(10000);

    for (nThreadPrioCnt = 0; nThreadPrioCnt < THREAD_PRIORITY_TEST_NUM; nThreadPrioCnt++)
    {
        CamOsThreadStop(tThreadPrio[nThreadPrioCnt]);
    }

    return 0;
}
#endif

#ifdef TEST_SEMAPHORE
static uint32_t _gTsemTestCnt = 0;
CamOsTsem_t     tSem;
CamOsThread     tTsemThread0, tTsemThread1;
static int      ThreadTestTsem1(void *pUserData)
{
    CamOsTsem_t *pSem = (CamOsTsem_t *)pUserData;
    CamOsRet_e   eRet;

    CamOsMsSleep(1000);

    _gTsemTestCnt++;

    CamOsPrintf("%s CamOsTsemUp\n", __FUNCTION__);
    CamOsTsemUp(pSem);

    CamOsPrintf("%s CamOsTsemDown start\n", __FUNCTION__);
    CamOsTsemDown(pSem);
    CamOsPrintf("%s CamOsTsemDown end\n", __FUNCTION__);

    if (_gTsemTestCnt != 2)
    {
        CamOsPrintf("%s: step 1 fail!(_gTsemTestCnt=%d)\n", __FUNCTION__, _gTsemTestCnt);
        return -1;
    }

    CamOsMsSleep(300);

    _gTsemTestCnt++;

    CamOsPrintf("%s CamOsTsemUp\n", __FUNCTION__);
    CamOsTsemUp(pSem);

    CamOsMsSleep(100);

    CamOsPrintf("%s CamOsTsemUp\n", __FUNCTION__);
    CamOsTsemUp(pSem);

    CamOsMsSleep(100);

    CamOsPrintf("%s CamOsTsemUp\n", __FUNCTION__);
    CamOsTsemUp(pSem);

    CamOsMsSleep(5000);

    _gTsemTestCnt++;

    CamOsPrintf("%s CamOsTsemUp\n", __FUNCTION__);
    CamOsTsemUp(pSem);

    CamOsPrintf("%s CamOsTsemTimedDown start\n", __FUNCTION__);
    eRet = CamOsTsemTimedDown(pSem, 3000);
    CamOsPrintf("%s CamOsTsemTimedDown end (%s)\n", __FUNCTION__, (eRet == CAM_OS_OK) ? "wakened" : "timeout");

    if (_gTsemTestCnt != 5)
    {
        CamOsPrintf("%s: step 2 fail!(_gTsemTestCnt=%d)\n", __FUNCTION__, _gTsemTestCnt);
        return -1;
    }

    CamOsMsSleep(1000);

    _gTsemTestCnt++;

    CamOsPrintf("%s CamOsTsemUp\n", __FUNCTION__);
    CamOsTsemUp(pSem);

    CamOsPrintf("%s CamOsTsemTimedDown start\n", __FUNCTION__);
    eRet = CamOsTsemTimedDown(pSem, 5000);
    CamOsPrintf("%s CamOsTsemTimedDown end (%s)\n", __FUNCTION__, (eRet == CAM_OS_OK) ? "wakened" : "timeout");

    if (eRet != CAM_OS_OK)
    {
        CamOsPrintf("%s: step 3 fail!(eRet=%d)\n", __FUNCTION__, eRet);
        return -1;
    }

    for (;;)
    {
        if (CAM_OS_OK == CamOsThreadShouldStop())
            break;
        CamOsMsSleep(100);
    }
    CamOsPrintf("%s break\n", __FUNCTION__);

    return 0;
}

static int ThreadTestTsem2(void *pUserData)
{
    CamOsTsem_t *pSem = (CamOsTsem_t *)pUserData;
    CamOsRet_e   eRet;

    CamOsPrintf("%s CamOsTsemDown start\n", __FUNCTION__);
    CamOsTsemDown(pSem);
    CamOsPrintf("%s CamOsTsemDown end\n", __FUNCTION__);

    if (_gTsemTestCnt != 1)
    {
        CamOsPrintf("%s: step 1 fail!(_gTsemTestCnt=%d)\n", __FUNCTION__, _gTsemTestCnt);
        return -1;
    }

    CamOsMsSleep(1000);

    _gTsemTestCnt++;

    CamOsPrintf("%s CamOsTsemUp\n", __FUNCTION__);
    CamOsTsemUp(pSem);

    CamOsMsSleep(100);

    CamOsPrintf("%s CamOsTsemDown start\n", __FUNCTION__);
    CamOsTsemDown(pSem);
    CamOsPrintf("%s CamOsTsemDown end\n", __FUNCTION__);

    if (_gTsemTestCnt != 3)
    {
        CamOsPrintf("%s: step 2 fail!(_gTsemTestCnt=%d)\n", __FUNCTION__, _gTsemTestCnt);
        return -1;
    }

    CamOsPrintf("%s CamOsTsemDown start\n", __FUNCTION__);
    CamOsTsemDown(pSem);
    CamOsPrintf("%s CamOsTsemDown end\n", __FUNCTION__);

    CamOsPrintf("%s CamOsTsemDown start\n", __FUNCTION__);
    CamOsTsemDown(pSem);
    CamOsPrintf("%s CamOsTsemDown end\n", __FUNCTION__);

    CamOsMsSleep(3000);

    CamOsPrintf("%s CamOsTsemDown start\n", __FUNCTION__);
    CamOsTsemDown(pSem);
    CamOsPrintf("%s CamOsTsemDown end\n", __FUNCTION__);

    if (_gTsemTestCnt != 4)
    {
        CamOsPrintf("%s: step 3 fail!(_gTsemTestCnt=%d)\n", __FUNCTION__, _gTsemTestCnt);
        return -1;
    }

    CamOsMsSleep(1000);

    _gTsemTestCnt++;

    CamOsPrintf("%s CamOsTsemUp\n", __FUNCTION__);
    CamOsTsemUp(pSem);

    CamOsMsSleep(100);

    CamOsPrintf("%s CamOsTsemTimedDown start\n", __FUNCTION__);
    eRet = CamOsTsemTimedDown(pSem, 3000);
    CamOsPrintf("%s CamOsTsemTimedDown end (%s)\n", __FUNCTION__, (eRet == CAM_OS_OK) ? "wakened" : "timeout");

    if (_gTsemTestCnt != 6)
    {
        CamOsPrintf("%s: step 4 fail!(_gTsemTestCnt=%d)\n", __FUNCTION__, _gTsemTestCnt);
        return -1;
    }

    CamOsPrintf("%s CamOsTsemTimedDown start\n", __FUNCTION__);
    eRet = CamOsTsemTimedDown(pSem, 3000);
    CamOsPrintf("%s CamOsTsemTimedDown end (%s)\n", __FUNCTION__, (eRet == CAM_OS_OK) ? "wakened" : "timeout");

    if (eRet != CAM_OS_TIMEOUT)
    {
        CamOsPrintf("%s: step 5 fail!(eRet=%d)\n", __FUNCTION__, eRet);
        return -1;
    }

    CamOsPrintf("%s CamOsTsemUp\n", __FUNCTION__);
    CamOsTsemUp(pSem);

    for (;;)
    {
        if (CAM_OS_OK == CamOsThreadShouldStop())
            break;
        CamOsMsSleep(100);
    }
    CamOsPrintf("%s break\n", __FUNCTION__);

    return 0;
}

int KernelTestSemaphore(void)
{
    CamOsPrintf("=================================\n");
    CamOsPrintf("Test semaphore\n");
    CamOsPrintf("=================================\n");
    CamOsTsemInit(&tSem, 0);
    CamOsThreadCreate(&tTsemThread0, NULL, (void *)ThreadTestTsem1, &tSem);
    CamOsThreadCreate(&tTsemThread1, NULL, (void *)ThreadTestTsem2, &tSem);

    CamOsMsSleep(15000);

    CamOsThreadStop(tTsemThread0);
    CamOsThreadStop(tTsemThread1);

    CamOsTsemDeinit(&tSem);

    return 0;
}
#endif

#ifdef TEST_RW_SEMAPHORE
static uint32_t _gRwsemTestCnt = 0;
CamOsRwsem_t    tCamOsRwsem;
CamOsThread     tRwsemThread0, tRwsemThread1, tRwsemThread2;

static int CamOsRwsemTestEntry0(void *pUserData)
{
    CamOsRwsem_t *tpRwsem = (CamOsRwsem_t *)pUserData;

    CamOsPrintf("%s CamOsRwsemDownRead start (%llu)\n", __FUNCTION__, CamOsGetTimeInMs());
    CamOsRwsemDownRead(tpRwsem);
    CamOsPrintf("%s CamOsRwsemDownRead end (%llu)\n", __FUNCTION__, CamOsGetTimeInMs());

    CamOsMsSleep(2000);

    if (_gRwsemTestCnt != 0)
    {
        CamOsPrintf("%s: step 1 fail!(_gRwsemTestCnt=%d) (%llu)\n", __FUNCTION__, _gRwsemTestCnt, CamOsGetTimeInMs());
        return -1;
    }

    CamOsPrintf("%s CamOsRwsemUpRead (%llu)\n", __FUNCTION__, CamOsGetTimeInMs());
    CamOsRwsemUpRead(tpRwsem);

    CamOsMsSleep(100);

    if (_gRwsemTestCnt != 1)
    {
        CamOsPrintf("%s: step 2 fail!(_gRwsemTestCnt=%d) (%llu)\n", __FUNCTION__, _gRwsemTestCnt, CamOsGetTimeInMs());
        return -1;
    }

    CamOsPrintf("%s CamOsRwsemDownWrite start (%llu)\n", __FUNCTION__, CamOsGetTimeInMs());
    CamOsRwsemDownWrite(tpRwsem);
    CamOsPrintf("%s CamOsRwsemDownWrite end (%llu)\n", __FUNCTION__, CamOsGetTimeInMs());

    CamOsMsSleep(1500);

    _gRwsemTestCnt++;

    CamOsPrintf("%s CamOsRwsemUpWrite (%llu)\n", __FUNCTION__, CamOsGetTimeInMs());
    CamOsRwsemUpWrite(tpRwsem);

    for (;;)
    {
        if (CAM_OS_OK == CamOsThreadShouldStop())
            break;
        CamOsMsSleep(100);
    }

    CamOsPrintf("%s break\n", __FUNCTION__);

    return 0;
}

static int CamOsRwsemTestEntry1(void *pUserData)
{
    CamOsRwsem_t *tpRwsem = (CamOsRwsem_t *)pUserData;
    CamOsRet_e    eRet;

    CamOsMsSleep(20);
    CamOsPrintf("%s CamOsRwsemTryDownRead start (%llu)\n", __FUNCTION__, CamOsGetTimeInMs());
    if (CAM_OS_OK != (eRet = CamOsRwsemTryDownRead(tpRwsem)))
    {
        CamOsPrintf("%s: step 1 fail!(eRet=%d) (%llu)\n", __FUNCTION__, eRet, CamOsGetTimeInMs());
        return -1;
    }
    CamOsPrintf("%s CamOsRwsemTryDownRead end (%llu)\n", __FUNCTION__, CamOsGetTimeInMs());

    CamOsMsSleep(1950);

    if (_gRwsemTestCnt != 0)
    {
        CamOsPrintf("%s: step 2 fail! (%llu)\n", __FUNCTION__, CamOsGetTimeInMs());
        return -1;
    }

    CamOsPrintf("%s CamOsRwsemUpRead (%llu)\n", __FUNCTION__, CamOsGetTimeInMs());
    CamOsRwsemUpRead(tpRwsem);

    CamOsMsSleep(100);

    if (_gRwsemTestCnt != 1)
    {
        CamOsPrintf("%s: step 3 fail! (%llu)\n", __FUNCTION__, CamOsGetTimeInMs());
        return -1;
    }

    CamOsMsSleep(1000);

    CamOsPrintf("%s CamOsRwsemDownWrite start (%llu)\n", __FUNCTION__, CamOsGetTimeInMs());
    CamOsRwsemDownWrite(tpRwsem);
    CamOsPrintf("%s CamOsRwsemDownWrite end (%llu)\n", __FUNCTION__, CamOsGetTimeInMs());

    if (_gRwsemTestCnt != 2)
    {
        CamOsPrintf("%s: step 4 fail!(_gRwsemTestCnt=%d) (%llu)\n", __FUNCTION__, _gRwsemTestCnt, CamOsGetTimeInMs());
        return -1;
    }

    _gRwsemTestCnt++;

    CamOsMsSleep(100);

    CamOsPrintf("%s CamOsRwsemUpWrite (%llu)\n", __FUNCTION__, CamOsGetTimeInMs());
    CamOsRwsemUpWrite(tpRwsem);

    for (;;)
    {
        if (CAM_OS_OK == CamOsThreadShouldStop())
            break;
        CamOsMsSleep(100);
    }

    CamOsPrintf("%s break\n", __FUNCTION__);

    return 0;
}

static int CamOsRwsemTestEntry2(void *pUserData)
{
    CamOsRwsem_t *tpRwsem = (CamOsRwsem_t *)pUserData;
    CamOsRet_e    eRet;

    CamOsMsSleep(50);

    CamOsPrintf("%s CamOsRwsemTryDownWrite start (%llu)\n", __FUNCTION__, CamOsGetTimeInMs());
    if (CAM_OS_RESOURCE_BUSY != (eRet = CamOsRwsemTryDownWrite(tpRwsem)))
    {
        CamOsPrintf("%s: step 1 fail!(eRet=%d) (%llu)\n", __FUNCTION__, eRet, CamOsGetTimeInMs());
        return -1;
    }
    CamOsPrintf("%s CamOsRwsemTryDownWrite end (%llu)\n", __FUNCTION__, CamOsGetTimeInMs());

    CamOsPrintf("%s CamOsRwsemDownWrite start (%llu)\n", __FUNCTION__, CamOsGetTimeInMs());
    CamOsRwsemDownWrite(tpRwsem);
    CamOsPrintf("%s CamOsRwsemDownWrite end (%llu)\n", __FUNCTION__, CamOsGetTimeInMs());

    _gRwsemTestCnt++;

    CamOsPrintf("%s CamOsRwsemUpWrite (%llu)\n", __FUNCTION__, CamOsGetTimeInMs());
    CamOsRwsemUpWrite(tpRwsem);

    CamOsMsSleep(200);

    CamOsPrintf("%s CamOsRwsemDownRead start (%llu)\n", __FUNCTION__, CamOsGetTimeInMs());
    CamOsRwsemDownRead(tpRwsem);
    CamOsPrintf("%s CamOsRwsemDownRead end (%llu)\n", __FUNCTION__, CamOsGetTimeInMs());

    if (_gRwsemTestCnt != 2 && _gRwsemTestCnt != 3)
    {
        CamOsPrintf("%s: step 2 fail!(_gRwsemTestCnt=%d) (%llu)\n", __FUNCTION__, _gRwsemTestCnt, CamOsGetTimeInMs());
        return -1;
    }

    CamOsPrintf("%s CamOsRwsemUpRead (%llu)\n", __FUNCTION__, CamOsGetTimeInMs());
    CamOsRwsemUpRead(tpRwsem);

    for (;;)
    {
        if (CAM_OS_OK == CamOsThreadShouldStop())
            break;
        CamOsMsSleep(100);
    }

    CamOsPrintf("%s break\n", __FUNCTION__);

    return 0;
}

int KernelTestRwSemaphore(void)
{
    CamOsPrintf("=================================\n");
    CamOsPrintf("Test rw semaphore\n");
    CamOsPrintf("=================================\n");
    CamOsRwsemInit(&tCamOsRwsem);
    CamOsThreadCreate(&tRwsemThread0, NULL, (void *)CamOsRwsemTestEntry0, &tCamOsRwsem);
    CamOsThreadCreate(&tRwsemThread1, NULL, (void *)CamOsRwsemTestEntry1, &tCamOsRwsem);
    CamOsThreadCreate(&tRwsemThread2, NULL, (void *)CamOsRwsemTestEntry2, &tCamOsRwsem);

    CamOsMsSleep(5000);
    CamOsThreadStop(tRwsemThread0);
    CamOsThreadStop(tRwsemThread1);
    CamOsThreadStop(tRwsemThread2);
    CamOsRwsemDeinit(&tCamOsRwsem);

    return 0;
}
#endif

#ifdef TEST_MUTEX
static uint32_t    _gMutexTestCnt = 0;
CamOsThreadAttrb_t tMutexThreadAttr;
CamOsMutex_t       tCamOsMutex;
CamOsThread        tMutexThread0, tMutexThread1, tMutexThread2;
static void        CamOsMutexTestEntry0(void *pUserData)
{
    CamOsMutex_t *tpMutex = (CamOsMutex_t *)pUserData;
    unsigned int  i       = 0;
    u32           tmpCnt  = 0;

    for (i = 0; i < 100; i++)
    {
        if (CAM_OS_OK == CamOsThreadShouldStop())
            break;
        CamOsMutexLock(tpMutex);
        _gMutexTestCnt++;
        tmpCnt = _gMutexTestCnt;
        CamOsPrintf("%s start count: %d\n\r", __FUNCTION__, _gMutexTestCnt);
        CamOsMsSleep(2);
        CamOsPrintf("%s end count: %d\n\r", __FUNCTION__, _gMutexTestCnt);
        if (_gMutexTestCnt != tmpCnt)
        {
            CamOsCallStack();
            CamOsPanic("Mutex UT fail!");
        }
        CamOsMutexUnlock(tpMutex);
        CamOsMsSleep(1);
    }

    for (;;)
    {
        if (CAM_OS_OK == CamOsThreadShouldStop())
            break;
        CamOsMsSleep(100);
    }
}

static void CamOsMutexTestEntry1(void *pUserData)
{
    CamOsMutex_t *tpMutex = (CamOsMutex_t *)pUserData;
    unsigned int  i       = 0;
    u32           tmpCnt  = 0;

    for (i = 0; i < 100; i++)
    {
        if (CAM_OS_OK == CamOsThreadShouldStop())
            break;
        CamOsMutexLock(tpMutex);
        _gMutexTestCnt++;
        tmpCnt = _gMutexTestCnt;
        CamOsPrintf("%s start count: %d\n\r", __FUNCTION__, _gMutexTestCnt);
        CamOsMsSleep(2);
        CamOsPrintf("%s end count: %d\n\r", __FUNCTION__, _gMutexTestCnt);
        if (_gMutexTestCnt != tmpCnt)
        {
            CamOsCallStack();
            CamOsPanic("Mutex UT fail!");
        }
        CamOsMutexUnlock(tpMutex);
        CamOsMsSleep(1);
    }

    for (;;)
    {
        if (CAM_OS_OK == CamOsThreadShouldStop())
            break;
        CamOsMsSleep(100);
    }
}

static void CamOsMutexTestEntry2(void *pUserData)
{
    CamOsMutex_t *tpMutex = (CamOsMutex_t *)pUserData;
    unsigned int  i       = 0;
    u32           tmpCnt  = 0;

    for (i = 0; i < 100; i++)
    {
        if (CAM_OS_OK == CamOsThreadShouldStop())
            break;
        CamOsMutexLock(tpMutex);
        _gMutexTestCnt++;
        tmpCnt = _gMutexTestCnt;
        CamOsPrintf("%s start count: %d\n\r", __FUNCTION__, _gMutexTestCnt);
        CamOsMsSleep(2);
        CamOsPrintf("%s end count: %d\n\r", __FUNCTION__, _gMutexTestCnt);
        if (_gMutexTestCnt != tmpCnt)
        {
            CamOsCallStack();
            CamOsPanic("Mutex UT fail!");
        }
        CamOsMutexUnlock(tpMutex);
        CamOsMsSleep(1);
    }

    for (;;)
    {
        if (CAM_OS_OK == CamOsThreadShouldStop())
            break;
        CamOsMsSleep(100);
    }
}

int KernelTestMutex(void)
{
    CamOsPrintf("=================================\n");
    CamOsPrintf("Test mutex\n");
    CamOsPrintf("=================================\n");
    CamOsMutexInit(&tCamOsMutex);
    tMutexThreadAttr.nPriority  = 1;
    tMutexThreadAttr.nStackSize = 3072;
    CamOsThreadCreate(&tMutexThread0, &tMutexThreadAttr, (void *)CamOsMutexTestEntry0, &tCamOsMutex);
    tMutexThreadAttr.nPriority  = 10;
    tMutexThreadAttr.nStackSize = 3072;
    CamOsThreadCreate(&tMutexThread1, &tMutexThreadAttr, (void *)CamOsMutexTestEntry1, &tCamOsMutex);
    tMutexThreadAttr.nPriority  = 20;
    tMutexThreadAttr.nStackSize = 3072;
    CamOsThreadCreate(&tMutexThread2, &tMutexThreadAttr, (void *)CamOsMutexTestEntry2, &tCamOsMutex);

    CamOsMsSleep(10000);
    CamOsThreadStop(tMutexThread0);
    CamOsThreadStop(tMutexThread1);
    CamOsThreadStop(tMutexThread2);
    CamOsMutexDestroy(&tCamOsMutex);

    return 0;
}
#endif

#ifdef TEST_DIV64
int KernelTestDiv64(void)
{
    uint64_t nDividendU64 = 0, nDivisorU64 = 0, nResultU64 = 0, nRemainderU64 = 0;
    int64_t  nDividendS64 = 0, nDivisorS64 = 0, nResultS64 = 0, nRemainderS64 = 0;

    CamOsPrintf("=================================\n");
    CamOsPrintf("Test 64-bit division\n");
    CamOsPrintf("=================================\n");
    nDividendU64 = 858993459978593;
    nDivisorU64  = 34358634759;
    nResultU64   = CamOsMathDivU64(nDividendU64, nDivisorU64, &nRemainderU64);
    CamOsPrintf("Dividend: %llu  Divisor: %llu  Result: %llu  Remainder: %llu\n", nDividendU64, nDivisorU64, nResultU64,
                nRemainderU64);

    nDividendS64 = -858993459978593;
    nDivisorS64  = 34358634759;
    nResultS64   = CamOsMathDivS64(nDividendS64, nDivisorS64, &nRemainderS64);
    CamOsPrintf("Dividend: %lld  Divisor: %lld  Result: %lld  Remainder: %lld\n", nDividendS64, nDivisorS64, nResultS64,
                nRemainderS64);

    nDividendS64 = 858993459978593;
    nDivisorS64  = -34358634759;
    nResultS64   = CamOsMathDivS64(nDividendS64, nDivisorS64, &nRemainderS64);
    CamOsPrintf("Dividend: %lld  Divisor: %lld  Result: %lld  Remainder: %lld\n", nDividendS64, nDivisorS64, nResultS64,
                nRemainderS64);

    nDividendS64 = -858993459978593;
    nDivisorS64  = -34358634759;
    nResultS64   = CamOsMathDivS64(nDividendS64, nDivisorS64, &nRemainderS64);
    CamOsPrintf("Dividend: %lld  Divisor: %lld  Result: %lld  Remainder: %lld\n", nDividendS64, nDivisorS64, nResultS64,
                nRemainderS64);

    return 0;
}
#endif

#ifdef TEST_SYSTEM_TIME
int KernelTestSystemTime(void)
{
    int32_t         nCnt = 0;
    CamOsTimespec_t tTs;
    struct tm       tTm;

    CamOsPrintf("=================================\n");
    CamOsPrintf("Test system time\n");
    CamOsPrintf("=================================\n");
    for (nCnt = 0; nCnt < 10; nCnt++)
    {
        CamOsGetTimeOfDay(&tTs);
        time64_to_tm((time64_t)(tTs.nSec), 0, &tTm);
        CamOsPrintf("RawSecond: %llu  ->  %d/%02d/%02d [%d]  %02d:%02d:%02d\n", tTs.nSec, tTm.tm_year + 1900,
                    tTm.tm_mon + 1, tTm.tm_mday, tTm.tm_wday, tTm.tm_hour, tTm.tm_min, tTm.tm_sec);

        tTs.nSec += 90000;
        CamOsSetTimeOfDay(&tTs);

        CamOsMsSleep(3000);
    }

    for (nCnt = 0; nCnt < 10; nCnt++)
    {
        CamOsGetTimeOfDay(&tTs);
        time64_to_tm((time64_t)(tTs.nSec), 0, &tTm);
        CamOsPrintf("RawSecond: %llu  ->  %d/%02d/%02d [%d]  %02d:%02d:%02d\n", tTs.nSec, tTm.tm_year + 1900,
                    tTm.tm_mon + 1, tTm.tm_mday, tTm.tm_wday, tTm.tm_hour, tTm.tm_min, tTm.tm_sec);

        tTs.nSec -= 90000;
        CamOsSetTimeOfDay(&tTs);

        CamOsMsSleep(3000);
    }

    return 0;
}
#endif

#ifdef TEST_MEM_SIZE
int KernelTestMemSize(void)
{
    CamOsMemSize_e  eMemSize;
    CamOsDramInfo_t Info = {0};

    CamOsPrintf("=================================\n");
    CamOsPrintf("Test memory size\n");
    CamOsPrintf("=================================\n");
    eMemSize = CamOsPhysMemSize();
    CamOsPrintf("System has %dMB physical memory\n", 1 << (uint32_t)eMemSize);
    CamOsDramInfo(&Info);
    CamOsPrintf("DRAM Info:  Size %lld    Type %d    Bus %d\n", Info.nBytes, Info.nType, Info.nBusWidth);

    return 0;
}
#endif

#ifdef TEST_CHIP_ID
int KernelTestChipId(void)
{
    CamOsPrintf("=================================\n");
    CamOsPrintf("Test chip ID\n");
    CamOsPrintf("=================================\n");
    CamOsPrintf("Chip ID: 0x%X    Revision: 0x%X\n", CamOsChipId(), CamOsChipRevision());

    return 0;
}
#endif

#ifdef TEST_ATOMIC_OPERATION
int KernelTestAtomicOperation(void)
{
    CamOsAtomic_t tAtomic;

    CamOsPrintf("=================================\n");
    CamOsPrintf("Test atomic operation\n");
    CamOsPrintf("=================================\n");
    CamOsPrintf("AtomicSet 10\n");
    CamOsAtomicSet(&tAtomic, 10);
    CamOsPrintf("AtomicRead                  : %d\n", CamOsAtomicRead(&tAtomic));
    CamOsPrintf("CamOsAtomicAddReturn (5)    : %d\n", CamOsAtomicAddReturn(&tAtomic, 5));
    CamOsPrintf("CamOsAtomicSubReturn (3)    : %d\n", CamOsAtomicSubReturn(&tAtomic, 3));
    CamOsPrintf("CamOsAtomicSubAndTest (2)   : %d\n", CamOsAtomicSubAndTest(&tAtomic, 2));
    CamOsPrintf("CamOsAtomicSubAndTest (10)  : %d\n", CamOsAtomicSubAndTest(&tAtomic, 10));
    CamOsPrintf("CamOsAtomicIncReturn        : %d\n", CamOsAtomicIncReturn(&tAtomic));
    CamOsPrintf("CamOsAtomicDecReturn        : %d\n", CamOsAtomicDecReturn(&tAtomic));
    CamOsPrintf("CamOsAtomicDecReturn        : %d\n", CamOsAtomicDecReturn(&tAtomic));
    CamOsPrintf("CamOsAtomicIncAndTest       : %d\n", CamOsAtomicIncAndTest(&tAtomic));
    CamOsPrintf("CamOsAtomicIncAndTest       : %d\n", CamOsAtomicIncAndTest(&tAtomic));
    CamOsPrintf("CamOsAtomicDecAndTest       : %d\n", CamOsAtomicDecAndTest(&tAtomic));
    CamOsPrintf("CamOsAtomicDecAndTest       : %d\n", CamOsAtomicDecAndTest(&tAtomic));
    CamOsPrintf("CamOsAtomicAddNegative (1)  : %d\n", CamOsAtomicAddNegative(&tAtomic, 1));
    CamOsPrintf("CamOsAtomicAddNegative (1)  : %d\n", CamOsAtomicAddNegative(&tAtomic, 1));
    CamOsPrintf("CamOsAtomicAddNegative (-3) : %d\n", CamOsAtomicAddNegative(&tAtomic, -3));

    return 0;
}
#endif

#ifdef TEST_BITMAP
int KernelTestBitmap(void)
{
#define BITMAP_BIT_NUM 128
    CAM_OS_DECLARE_BITMAP(aBitmap, BITMAP_BIT_NUM);

    CamOsPrintf("=================================\n");
    CamOsPrintf("Test bitmap operation\n");
    CamOsPrintf("=================================\n");
    CAM_OS_BITMAP_CLEAR(aBitmap);
    CamOsPrintf("Set bit 0, 1, 2, 37, 98\n");
    CAM_OS_SET_BIT(0, aBitmap);
    CAM_OS_SET_BIT(1, aBitmap);
    CAM_OS_SET_BIT(2, aBitmap);
    CAM_OS_SET_BIT(37, aBitmap);
    CAM_OS_SET_BIT(98, aBitmap);
    CamOsPrintf("\ttest bit 0:   %d\n", CAM_OS_TEST_BIT(0, aBitmap));
    CamOsPrintf("\ttest bit 1:   %d\n", CAM_OS_TEST_BIT(1, aBitmap));
    CamOsPrintf("\ttest bit 2:   %d\n", CAM_OS_TEST_BIT(2, aBitmap));
    CamOsPrintf("\ttest bit 3:   %d\n", CAM_OS_TEST_BIT(3, aBitmap));
    CamOsPrintf("\ttest bit 30:  %d\n", CAM_OS_TEST_BIT(30, aBitmap));
    CamOsPrintf("\ttest bit 37:  %d\n", CAM_OS_TEST_BIT(37, aBitmap));
    CamOsPrintf("\ttest bit 80:  %d\n", CAM_OS_TEST_BIT(80, aBitmap));
    CamOsPrintf("\ttest bit 98:  %d\n", CAM_OS_TEST_BIT(98, aBitmap));
    CamOsPrintf("\ttest bit 127: %d\n", CAM_OS_TEST_BIT(127, aBitmap));
    CamOsPrintf("\tfirst zero bit: %u\n", CAM_OS_FIND_FIRST_ZERO_BIT(aBitmap, BITMAP_BIT_NUM));
    CamOsPrintf("Clear bit 2, 98\n");
    CAM_OS_CLEAR_BIT(2, aBitmap);
    CAM_OS_CLEAR_BIT(98, aBitmap);
    CamOsPrintf("\ttest bit 2:   %d\n", CAM_OS_TEST_BIT(2, aBitmap));
    CamOsPrintf("\ttest bit 98:  %d\n", CAM_OS_TEST_BIT(98, aBitmap));
    CamOsPrintf("\tfirst zero bit: %u\n", CAM_OS_FIND_FIRST_ZERO_BIT(aBitmap, BITMAP_BIT_NUM));

    return 0;
}
#endif

#ifdef TEST_HASH
struct HashTableElement_t
{
    struct CamOsHListNode_t tHentry;
    uint32_t                nKey;
    uint32_t                nData;
};

int KernelTestHash(void)
{
    uint32_t nItemNum;
    CAM_OS_DEFINE_HASHTABLE(aHashTable, 8);
    struct HashTableElement_t tHListNode0, tHListNode1, tHListNode2, tHListNode3, tHListNode4, *ptHListNode;

    CamOsPrintf("=================================\n");
    CamOsPrintf("Test hash operation\n");
    CamOsPrintf("=================================\n");
    tHListNode0.nKey  = 102;
    tHListNode0.nData = 1021;
    tHListNode1.nKey  = 1872;
    tHListNode1.nData = 18721;
    tHListNode2.nKey  = 102;
    tHListNode2.nData = 1022;
    tHListNode3.nKey  = 1872;
    tHListNode3.nData = 18722;
    tHListNode4.nKey  = 102;
    tHListNode4.nData = 1023;
    CamOsPrintf("Add 3 items with key 102 and 2 items with key 1872.\n");
    CAM_OS_HASH_ADD(aHashTable, &tHListNode0.tHentry, tHListNode0.nKey);
    CAM_OS_HASH_ADD(aHashTable, &tHListNode1.tHentry, tHListNode1.nKey);
    CAM_OS_HASH_ADD(aHashTable, &tHListNode2.tHentry, tHListNode2.nKey);
    CAM_OS_HASH_ADD(aHashTable, &tHListNode3.tHentry, tHListNode3.nKey);
    CAM_OS_HASH_ADD(aHashTable, &tHListNode4.tHentry, tHListNode4.nKey);
    CamOsPrintf("Get items with key 102: \n");
    nItemNum = 0;
    CAM_OS_HASH_FOR_EACH_POSSIBLE(aHashTable, ptHListNode, tHentry, 102)
    {
        CamOsPrintf("\titem %u: data=%u\n", nItemNum, ptHListNode->nData);
        nItemNum++;
    }

    CamOsPrintf("Get items with key 1872: \n");
    nItemNum = 0;
    CAM_OS_HASH_FOR_EACH_POSSIBLE(aHashTable, ptHListNode, tHentry, 1872)
    {
        CamOsPrintf("\titem %u: data=%u\n", nItemNum, ptHListNode->nData);
        nItemNum++;
    }

    CamOsPrintf("Delete one items with key 1872.\n");
    CAM_OS_HASH_DEL(&tHListNode3.tHentry);
    CamOsPrintf("Get items with key 1872: \n");
    nItemNum = 0;
    CAM_OS_HASH_FOR_EACH_POSSIBLE(aHashTable, ptHListNode, tHentry, 1872)
    {
        CamOsPrintf("\titem %u: data=%u\n", nItemNum, ptHListNode->nData);
        nItemNum++;
    }

    return 0;
}
#endif

#ifdef TEST_TIMER
CamOsTimer_t  tTimer;
unsigned long nTimerMs;
u32           nTimerRetriggerCnt = 0;

static void _TimerCallback(void *nDataAddr)
{
    unsigned long *pnTimerMs = (unsigned long *)nDataAddr;

    nTimerRetriggerCnt++;
    CamOsPrintf("%s: timer ms=%lu (%llu)\n", __FUNCTION__, *pnTimerMs, CamOsGetTimeInMs());

    if (nTimerRetriggerCnt < 5)
    {
        CamOsPrintf("%s Call CamOsTimerAdd with %dms expires (%llu)\n", __FUNCTION__, nTimerMs, CamOsGetTimeInMs());
        CamOsTimerAdd(&tTimer, nTimerMs, (void *)&nTimerMs, _TimerCallback);
    }
}

int KernelTestTimer(void)
{
    CamOsPrintf("=================================\n");
    CamOsPrintf("Test timer operation\n");
    CamOsPrintf("=================================\n");
    CamOsPrintf("[Step 1] add timer to 2000ms, then sleep 6100ms ... \n");
    CamOsTimerInit(&tTimer);
    nTimerMs           = 2000;
    nTimerRetriggerCnt = 0;
    CamOsPrintf("%s Call CamOsTimerAdd with %dms expires (%llu)\n", __FUNCTION__, nTimerMs, CamOsGetTimeInMs());
    CamOsTimerAdd(&tTimer, nTimerMs, (void *)&nTimerMs, _TimerCallback);
    CamOsMsSleep(6100);
    if (CamOsTimerDelete(&tTimer))
    {
        CamOsPrintf("success!\n\n");
    }
    else
    {
        CamOsPrintf("fail!\n\n");
    }
    CamOsTimerDeinit(&tTimer);

    CamOsPrintf(
        "[Step 2] add timer to 1000ms, modify to 1500ms in 300ms, then "
        "sleep 3000ms ... \n");
    CamOsTimerInit(&tTimer);
    nTimerMs           = 1000;
    nTimerRetriggerCnt = 0;
    CamOsPrintf("%s Call CamOsTimerAdd with %dms expires (%llu)\n", __FUNCTION__, nTimerMs, CamOsGetTimeInMs());
    CamOsTimerAdd(&tTimer, nTimerMs, (void *)&nTimerMs, _TimerCallback);
    CamOsMsSleep(300);
    nTimerMs = 1500;
    CamOsPrintf("%s Call CamOsTimerModify with %dms expires (%llu)\n", __FUNCTION__, nTimerMs, CamOsGetTimeInMs());
    CamOsTimerModify(&tTimer, nTimerMs);
    CamOsMsSleep(3000);
    if (CamOsTimerDelete(&tTimer))
    {
        CamOsPrintf("success!\n\n");
    }
    else
    {
        CamOsPrintf("fail!\n\n");
    }
    CamOsTimerDeinit(&tTimer);

    CamOsPrintf("[Step 3] add timer to 3000ms, sleep 1000ms, then delete timer ... \n");
    CamOsTimerInit(&tTimer);
    nTimerMs           = 3000;
    nTimerRetriggerCnt = 0;
    CamOsPrintf("%s Call CamOsTimerAdd with %dms expires (%llu)\n", __FUNCTION__, nTimerMs, CamOsGetTimeInMs());
    CamOsTimerAdd(&tTimer, nTimerMs, (void *)&nTimerMs, _TimerCallback);
    CamOsMsSleep(1000);
    if (CamOsTimerDelete(&tTimer))
    {
        CamOsPrintf("success!\n\n");
    }
    else
    {
        CamOsPrintf("fail!\n\n");
    }
    CamOsTimerDeinit(&tTimer);

    CamOsPrintf("[Step 4] add timer to 500ms, sleep 4000ms, then delete timer ... \n");
    CamOsTimerInit(&tTimer);
    nTimerMs           = 500;
    nTimerRetriggerCnt = 0;
    CamOsPrintf("%s Call CamOsTimerAdd with %dms expires (%llu)\n", __FUNCTION__, nTimerMs, CamOsGetTimeInMs());
    CamOsTimerAdd(&tTimer, nTimerMs, (void *)&nTimerMs, _TimerCallback);
    CamOsMsSleep(4000);
    if (CamOsTimerDelete(&tTimer))
    {
        CamOsPrintf("fail!\n\n");
    }
    else
    {
        CamOsPrintf("success!\n\n");
    }
    CamOsTimerDeinit(&tTimer);

    return 0;
}
#endif

#ifdef TEST_SPINLOCK
static void _CamOsThreadTestSpinlock0(void *pUserdata)
{
    CamOsSpinlock_t *tpSpinlock = (CamOsSpinlock_t *)pUserdata;
    s32              i          = 0;

    CamOsSpinLock(tpSpinlock);
    for (i = 0; i < 500; i++)
    {
        CamOsPrintf("%s(%d)\n", __FUNCTION__, i);
        CamOsMsDelay(1);
    }
    CamOsSpinUnlock(tpSpinlock);

    CamOsMsSleep(1000);

    CamOsSpinLockIrqSave(tpSpinlock);
    for (i = 0; i < 500; i++)
    {
        CamOsPrintf("%s(%d)\n", __FUNCTION__, i);
        CamOsMsDelay(1);
    }
    CamOsSpinUnlockIrqRestore(tpSpinlock);

    for (;;)
    {
        if (CAM_OS_OK == CamOsThreadShouldStop())
            break;
        CamOsMsSleep(100);
    }
}

static void _CamOsThreadTestSpinlock1(void *pUserdata)
{
    CamOsSpinlock_t *tpSpinlock = (CamOsSpinlock_t *)pUserdata;
    s32              i          = 0;

    CamOsSpinLock(tpSpinlock);
    for (i = 0; i < 500; i++)
    {
        CamOsPrintf("\033[1;34m%s (%d)\033[m\n", __FUNCTION__, i);
        CamOsMsDelay(1);
    }
    CamOsSpinUnlock(tpSpinlock);

    CamOsMsSleep(1000);

    CamOsSpinLockIrqSave(tpSpinlock);
    for (i = 0; i < 500; i++)
    {
        CamOsPrintf("%s(%d)\n", __FUNCTION__, i);
        CamOsMsDelay(1);
    }
    CamOsSpinUnlockIrqRestore(tpSpinlock);

    for (;;)
    {
        if (CAM_OS_OK == CamOsThreadShouldStop())
            break;
        CamOsMsSleep(100);
    }
}

int KernelTestSpinlock(void)
{
    CamOsThread     SpinlockTaskHandle0, SpinlockTaskHandle1;
    CamOsSpinlock_t stSpinlock;

    CamOsPrintf("=================================\n");
    CamOsPrintf("Test Spinlock\n");
    CamOsPrintf("=================================\n");

    CamOsSpinInit(&stSpinlock);
    CamOsThreadCreate(&SpinlockTaskHandle0, NULL, (void *)_CamOsThreadTestSpinlock0, &stSpinlock);
    CamOsThreadCreate(&SpinlockTaskHandle1, NULL, (void *)_CamOsThreadTestSpinlock1, &stSpinlock);

    CamOsMsSleep(20000);

    CamOsThreadStop(SpinlockTaskHandle0);
    CamOsThreadStop(SpinlockTaskHandle1);
    CamOsSpinDeinit(&stSpinlock);

    return 0;
}
#endif

#ifdef TEST_CONDITION
CamOsThread      tConditionTestThread0, tConditionTestThread1;
CamOsCondition_t cond_handle0;
u32              test_cond     = 0;
u32              test_cond_cnt = 0;

static int ConditionThreadTest0(void *pUserData)
{
    CamOsRet_e ret;

    CamOsPrintf(
        "[%llu] Thread0: Condition wait start, wait test_cond==0, "
        "timeout=3000ms\n",
        CamOsGetTimeInMs());
    ret = CamOsConditionTimedWait(&cond_handle0, test_cond == 0, 3000);
    CamOsPrintf("[%llu] Thread0: Condition wait end (%s)\n", CamOsGetTimeInMs(),
                (ret == CAM_OS_TIMEOUT) ? "timeouted" : "");

    CamOsMsSleep(10);

    CamOsPrintf(
        "[%llu] Thread0: Condition wait start, wait test_cond==1, "
        "timeout=3000ms\n",
        CamOsGetTimeInMs());
    ret = CamOsConditionTimedWait(&cond_handle0, test_cond == 1, 3000);
    CamOsPrintf("[%llu] Thread0: Condition wait end (%s)\n", CamOsGetTimeInMs(),
                (ret == CAM_OS_TIMEOUT) ? "timeouted" : "");

    CamOsMsSleep(4000);

    CamOsPrintf("[%llu] Thread0: Condition wait start, wait test_cond==3\n", CamOsGetTimeInMs());
    CamOsConditionWait(&cond_handle0, test_cond == 3);
    CamOsPrintf("[%llu] Thread0: Condition wait end\n", CamOsGetTimeInMs());

    CamOsMsSleep(1000);

    CamOsPrintf("[%llu] Thread0: Condition wait start, wait test_cond==98\n", CamOsGetTimeInMs());
    CamOsConditionWait(&cond_handle0, test_cond == 98);
    CamOsPrintf("[%llu] Thread0: Condition wait end\n", CamOsGetTimeInMs());

    CamOsMsSleep(1500);

    CamOsPrintf(
        "[%llu] Thread0: Condition wait start, wait test_cond==99, "
        "timeout=3000ms\n",
        CamOsGetTimeInMs());
    ret = CamOsConditionTimedWait(&cond_handle0, test_cond == 99, 3000);
    CamOsPrintf("[%llu] Thread0: Condition wait end (%s)\n", CamOsGetTimeInMs(),
                (ret == CAM_OS_TIMEOUT) ? "timeouted" : "");
    CamOsMsSleep(1000);
    CamOsPrintf(
        "[%llu] Thread0: Condition wait start, wait test_cond==99, "
        "timeout=3000ms\n",
        CamOsGetTimeInMs());
    ret = CamOsConditionTimedWait(&cond_handle0, test_cond == 99, 3000);
    CamOsPrintf("[%llu] Thread0: Condition wait end (%s)\n", CamOsGetTimeInMs(),
                (ret == CAM_OS_TIMEOUT) ? "timeouted" : "");
    CamOsMsSleep(1000);
    CamOsPrintf(
        "[%llu] Thread0: Condition wait start, wait test_cond==99, "
        "timeout=3000ms\n",
        CamOsGetTimeInMs());
    ret = CamOsConditionTimedWait(&cond_handle0, test_cond == 99, 3000);
    CamOsPrintf("[%llu] Thread0: Condition wait end (%s)\n", CamOsGetTimeInMs(),
                (ret == CAM_OS_TIMEOUT) ? "timeouted" : "");
    CamOsMsSleep(1000);
    CamOsPrintf(
        "[%llu] Thread0: Condition wait start, wait test_cond==99, "
        "timeout=3000ms\n",
        CamOsGetTimeInMs());
    ret = CamOsConditionTimedWait(&cond_handle0, test_cond == 99, 3000);
    CamOsPrintf("[%llu] Thread0: Condition wait end (%s)\n", CamOsGetTimeInMs(),
                (ret == CAM_OS_TIMEOUT) ? "timeouted" : "");

    while (CAM_OS_OK != CamOsThreadShouldStop())
    {
        CamOsMsSleep(1);
    }

    return 0;
}

static int ConditionThreadTest1(void *pUserData)
{
    CamOsRet_e ret;

    CamOsPrintf(
        "[%llu] Thread1: Condition wait start, wait test_cond==0, "
        "timeout=3000ms\n",
        CamOsGetTimeInMs());
    ret = CamOsConditionTimedWait(&cond_handle0, test_cond == 0, 3000);
    CamOsPrintf("[%llu] Thread1: Condition wait end (%s)\n", CamOsGetTimeInMs(),
                (ret == CAM_OS_TIMEOUT) ? "timeouted" : "");

    CamOsMsSleep(10);

    CamOsPrintf(
        "[%llu] Thread1: Condition wait start, wait test_cond==2, "
        "timeout=3000ms\n",
        CamOsGetTimeInMs());
    ret = CamOsConditionTimedWait(&cond_handle0, test_cond == 2, 3000);
    CamOsPrintf("[%llu] Thread1: Condition wait end (%s)\n", CamOsGetTimeInMs(),
                (ret == CAM_OS_TIMEOUT) ? "timeouted" : "");

    CamOsMsSleep(2000);

    CamOsPrintf("[%llu] Thread1: Condition wait start, wait test_cond==3\n", CamOsGetTimeInMs());
    CamOsConditionWait(&cond_handle0, test_cond == 3);
    CamOsPrintf("[%llu] Thread1: Condition wait end\n", CamOsGetTimeInMs());

    CamOsMsSleep(1000);

    CamOsPrintf("[%llu] Thread1: Condition wait start, wait test_cond==98\n", CamOsGetTimeInMs());
    CamOsConditionWait(&cond_handle0, test_cond == 98);
    CamOsPrintf("[%llu] Thread1: Condition wait end\n", CamOsGetTimeInMs());

    CamOsMsSleep(1500);

    CamOsPrintf(
        "[%llu] Thread1: Condition wait start, wait test_cond==99, "
        "timeout=3000ms\n",
        CamOsGetTimeInMs());
    ret = CamOsConditionTimedWait(&cond_handle0, test_cond == 99, 3000);
    CamOsPrintf("[%llu] Thread1: Condition wait end (%s)\n", CamOsGetTimeInMs(),
                (ret == CAM_OS_TIMEOUT) ? "timeouted" : "");
    CamOsMsSleep(1000);
    CamOsPrintf(
        "[%llu] Thread1: Condition wait start, wait test_cond==99, "
        "timeout=3000ms\n",
        CamOsGetTimeInMs());
    ret = CamOsConditionTimedWait(&cond_handle0, test_cond == 99, 3000);
    CamOsPrintf("[%llu] Thread1: Condition wait end (%s)\n", CamOsGetTimeInMs(),
                (ret == CAM_OS_TIMEOUT) ? "timeouted" : "");
    CamOsMsSleep(1000);
    CamOsPrintf(
        "[%llu] Thread1: Condition wait start, wait test_cond==99, "
        "timeout=3000ms\n",
        CamOsGetTimeInMs());
    ret = CamOsConditionTimedWait(&cond_handle0, test_cond == 99, 3000);
    CamOsPrintf("[%llu] Thread1: Condition wait end (%s)\n", CamOsGetTimeInMs(),
                (ret == CAM_OS_TIMEOUT) ? "timeouted" : "");
    CamOsMsSleep(1000);
    CamOsPrintf(
        "[%llu] Thread1: Condition wait start, wait test_cond==99, "
        "timeout=3000ms\n",
        CamOsGetTimeInMs());
    ret = CamOsConditionTimedWait(&cond_handle0, test_cond == 99, 3000);
    CamOsPrintf("[%llu] Thread1: Condition wait end (%s)\n", CamOsGetTimeInMs(),
                (ret == CAM_OS_TIMEOUT) ? "timeouted" : "");

    while (CAM_OS_OK != CamOsThreadShouldStop())
    {
        CamOsMsSleep(1);
    }

    return 0;
}

int KernelTestCondition(void)
{
    CamOsConditionInit(&cond_handle0);

    test_cond = 0;
    CamOsPrintf("[%llu] %s (set test_cond to %d)\n", CamOsGetTimeInMs(), __FUNCTION__, test_cond);

    CamOsThreadCreate(&tConditionTestThread0, NULL, (void *)ConditionThreadTest0, NULL);
    CamOsThreadCreate(&tConditionTestThread1, NULL, (void *)ConditionThreadTest1, NULL);

    CamOsMsSleep(1000);

    test_cond = 1;
    CamOsPrintf("[%llu] %s wake up cond_handle0 (set test_cond to %d)\n", CamOsGetTimeInMs(), __FUNCTION__, test_cond);
    CamOsConditionWakeUpAll(&cond_handle0);

    CamOsMsSleep(7000);

    test_cond = 3;
    CamOsPrintf("[%llu] %s wake up cond_handle0 (set test_cond to %d)\n", CamOsGetTimeInMs(), __FUNCTION__, test_cond);
    CamOsConditionWakeUpAll(&cond_handle0);

    CamOsMsSleep(500);
    CamOsPrintf("============================================================\n");
    CamOsMsSleep(1500);

    CamOsPrintf("Test CamOsConditionWait: condition fail & never call WakeUpAll\n");
    for (test_cond_cnt = 0; test_cond_cnt < 3; test_cond_cnt++)
    {
        test_cond = 97;
        CamOsPrintf("[%llu] %s never wake up cond_handle0 (set test_cond to %d)\n", CamOsGetTimeInMs(), __FUNCTION__,
                    test_cond);
        CamOsMsSleep(1000);
    }
    CamOsPrintf("Test CamOsConditionWait: condition true & never call WakeUpAll\n");
    for (test_cond_cnt = 0; test_cond_cnt < 3; test_cond_cnt++)
    {
        test_cond = 98;
        CamOsPrintf("[%llu] %s never wake up cond_handle0 (set test_cond to %d)\n", CamOsGetTimeInMs(), __FUNCTION__,
                    test_cond);
        CamOsMsSleep(1000);
    }
    CamOsPrintf("Test CamOsConditionWait: condition fail & call WakeUpAll\n");
    for (test_cond_cnt = 0; test_cond_cnt < 3; test_cond_cnt++)
    {
        test_cond = 97;
        CamOsPrintf("[%llu] %s wake up cond_handle0 (set test_cond to %d)\n", CamOsGetTimeInMs(), __FUNCTION__,
                    test_cond);
        CamOsMsSleep(1000);
    }
    CamOsPrintf("Test CamOsConditionWait: condition true & call WakeUpAll\n");
    for (test_cond_cnt = 0; test_cond_cnt < 1; test_cond_cnt++)
    {
        test_cond = 98;
        CamOsPrintf("[%llu] %s wake up cond_handle0 (set test_cond to %d)\n", CamOsGetTimeInMs(), __FUNCTION__,
                    test_cond);
        CamOsConditionWakeUpAll(&cond_handle0);
        CamOsMsSleep(1000);
    }

    CamOsPrintf("============================================================\n");
    CamOsMsSleep(1000);

    CamOsPrintf("Test CamOsConditionTiedWait: condition fail & never call WakeUpAll\n");
    for (test_cond_cnt = 0; test_cond_cnt < 3; test_cond_cnt++)
    {
        test_cond = 97;
        CamOsPrintf("[%llu] %s never wake up cond_handle0 (set test_cond to %d)\n", CamOsGetTimeInMs(), __FUNCTION__,
                    test_cond);
        CamOsMsSleep(1000);
    }
    CamOsMsSleep(1000);
    CamOsPrintf("Test CamOsConditionTiedWait: condition true & never call WakeUpAll\n");
    for (test_cond_cnt = 0; test_cond_cnt < 3; test_cond_cnt++)
    {
        test_cond = 99;
        CamOsPrintf("[%llu] %s never wake up cond_handle0 (set test_cond to %d)\n", CamOsGetTimeInMs(), __FUNCTION__,
                    test_cond);
        CamOsMsSleep(1000);
    }
    test_cond = 97;
    CamOsMsSleep(1000);
    CamOsPrintf("Test CamOsConditionTiedWait: condition fail & call WakeUpAll\n");
    for (test_cond_cnt = 0; test_cond_cnt < 3; test_cond_cnt++)
    {
        test_cond = 97;
        CamOsPrintf("[%llu] %s wake up cond_handle0 (set test_cond to %d)\n", CamOsGetTimeInMs(), __FUNCTION__,
                    test_cond);
        CamOsMsSleep(1000);
    }
    CamOsMsSleep(1000);
    CamOsPrintf("Test CamOsConditionTiedWait: condition true & call WakeUpAll\n");
    for (test_cond_cnt = 0; test_cond_cnt < 1; test_cond_cnt++)
    {
        test_cond = 99;
        CamOsPrintf("[%llu] %s wake up cond_handle0 (set test_cond to %d)\n", CamOsGetTimeInMs(), __FUNCTION__,
                    test_cond);
        CamOsConditionWakeUpAll(&cond_handle0);
        CamOsMsSleep(1000);
    }

    CamOsMsSleep(1000);

    CamOsThreadStop(tConditionTestThread0);
    CamOsThreadStop(tConditionTestThread1);

    CamOsConditionDeinit(&cond_handle0);

    return 0;
}
#endif

#ifdef TEST_CPU_RUNTIME
int KernelTestCpuRunTime(void)
{
    u64 nStart, nEnd;
    u64 nTest;

    CamOsPrintf("=================================\n");
    CamOsPrintf("Test CPU RunTime\n");
    CamOsPrintf("=================================\n");
    nStart = CamOsGetTaskSchedRunTime(); // Dummy function call, make CamOsGetTaskSchedRunTime into cache
    nTest  = CamOsGetJiffies();
    nStart = CamOsGetTaskSchedRunTime();
    nTest += CamOsGetJiffies();
    nEnd = CamOsGetTaskSchedRunTime();
    CamOsPrintf("Time cost without sleep: %llu ns (end %llu ns, start %llu ns, nTest = %lu)\n", nEnd - nStart, nEnd,
                nStart, nTest);

    nStart = CamOsGetTaskSchedRunTime();
    nTest += CamOsGetJiffies();
    CamOsMsSleep(1000);
    nEnd = CamOsGetTaskSchedRunTime();
    CamOsPrintf("Time cost with sleep 1s: %llu ns (end %llu ns, start %llu ns, nTest = %lu)\n", nEnd - nStart, nEnd,
                nStart, nTest);

    return 0;
}

#endif

#ifdef TEST_WORK_QUEUE
void WorkQueueTestHandle(void *data)
{
    CamOsPrintf("%s execute job (%d)\n", __FUNCTION__, *(u32 *)data);
}

int KernelTestWorkQueue(void)
{
    CamOsWorkQueue     wq                    = NULL;
    CamOsThreadAttrb_t attr                  = {.nPriority = 95, .nStackSize = 4096, .szName = "cam_os_test_wq"};
    u32                nWorkQueuePrivData[8] = {0};

    CamOsPrintf("=================================\n");
    CamOsPrintf("Test Workqueue\n");
    CamOsPrintf("=================================\n");

    CamOsPrintf("Create workqueue named cam_os_test_wq\n");
    CamOsWorkQueueCreate(&wq, "cam_os_test_wq", 32);
    nWorkQueuePrivData[0] = 111;
    CamOsWorkQueueAdd(wq, WorkQueueTestHandle, &nWorkQueuePrivData[0], 0);
    nWorkQueuePrivData[1] = 222;
    CamOsWorkQueueAdd(wq, WorkQueueTestHandle, &nWorkQueuePrivData[1], 1000);
    nWorkQueuePrivData[2] = 333;
    CamOsWorkQueueAdd(wq, WorkQueueTestHandle, &nWorkQueuePrivData[2], 2000);
    nWorkQueuePrivData[3] = 444;
    CamOsWorkQueueAdd(wq, WorkQueueTestHandle, &nWorkQueuePrivData[3], 10000);
    CamOsMsSleep(5000);
    CamOsWorkQueueDestroy(wq);
    CamOsPrintf("Destroy workqueue named cam_os_test_wq\n");

    CamOsPrintf("Create workqueue named cam_os_test_wq with thread attribute\n");
    CamOsWorkQueueCreateExt(&wq, &attr, 32);
    nWorkQueuePrivData[4] = 555;
    CamOsWorkQueueAdd(wq, WorkQueueTestHandle, &nWorkQueuePrivData[4], 0);
    nWorkQueuePrivData[5] = 666;
    CamOsWorkQueueAdd(wq, WorkQueueTestHandle, &nWorkQueuePrivData[5], 1000);
    nWorkQueuePrivData[6] = 777;
    CamOsWorkQueueAdd(wq, WorkQueueTestHandle, &nWorkQueuePrivData[6], 2000);
    nWorkQueuePrivData[7] = 888;
    CamOsWorkQueueAdd(wq, WorkQueueTestHandle, &nWorkQueuePrivData[7], 10000);
    CamOsMsSleep(5000);
    CamOsWorkQueueDestroy(wq);
    CamOsPrintf("Destroy workqueue named cam_os_test_wq\n");

    return 0;
}
#endif

#ifdef TEST_MEMCPY
struct
{
    char name[40];
    int  age;
} person, person_copy;

int KernelTestMemcpy(void)
{
    char myname[] = "Pierre de Fermat";

    /* using memcpy to copy string: */
    CamOsMemcpy(person.name, myname, CamOsStrlen(myname) + 1);
    person.age = 46;

    /* using memcpy to copy structure: */
    CamOsMemcpy(&person_copy, &person, sizeof(person));

    CamOsPrintf("person_copy: %s, %d \n", person_copy.name, person_copy.age);

    return 0;
}
#endif

#ifdef TEST_MEMMOVE
int KernelTestMemmove(void)
{
    char str[] = "memmove can be very useful......";
    CamOsMemmove(str + 20, str + 15, 11);
    CamOsPrintf(str);
    return 0;
}
#endif

#ifdef TEST_MEMSET
int KernelTestMemset(void)
{
    char str[] = "almost every programmer should know memset!";
    CamOsMemset(str, '-', 6);
    CamOsPrintf(str);
    return 0;
}
#endif

#ifdef TEST_MEMCMP
int KernelTestMemcmp(void)
{
    char buffer1[] = "DWgaOtP12df0";
    char buffer2[] = "DWGAOTP12DF0";

    int n;

    n = CamOsMemcmp(buffer1, buffer2, sizeof(buffer1));

    if (n > 0)
        CamOsPrintf("'%s' is greater than '%s'.\n", buffer1, buffer2);
    else if (n < 0)
        CamOsPrintf("'%s' is less than '%s'.\n", buffer1, buffer2);
    else
        CamOsPrintf("'%s' is the same as '%s'.\n", buffer1, buffer2);

    return 0;
}
#endif

#ifdef TEST_STRSTR
int KernelTestStrstr(void)
{
    char  str[] = "This is a simple string";
    char *pch;
    pch = CamOsStrstr(str, "simple");
    if (pch != NULL)
        CamOsStrncpy(pch, "sample", 6);
    CamOsPrintf(str);
    return 0;
}
#endif

#ifdef TEST_STRNCPY
int KernelTestStrncpy(void)
{
    char str1[] = "To be or not to be";
    char str2[40];
    char str3[40];

    /* copy to sized buffer (overflow safe): */
    CamOsStrncpy(str2, str1, sizeof(str2));

    /* partial copy (only 5 chars): */
    CamOsStrncpy(str3, str2, 5);
    str3[5] = '\0'; /* null character manually added */

    CamOsPrintf(str1);
    CamOsPrintf(str2);
    CamOsPrintf(str3);

    return 0;
}
#endif

#ifdef TEST_STRNCMP
int KernelTestStrncmp(void)
{
    char str[][5] = {"R2D2", "C3PO", "R2A6"};
    int  n;
    CamOsPrintf("Looking for R2 astromech droids...");
    for (n = 0; n < 3; n++)
        if (CamOsStrncmp(str[n], "R2xx", 2) == 0)
        {
            CamOsPrintf("found %s\n", str[n]);
        }
    return 0;
}
#endif

#ifdef TEST_STRNCASECMP
int KernelTestStrncasecmp(void)
{
    char *str1 = "STRING ONE";
    char *str2 = "string TWO";
    int   result;

    result = CamOsStrncasecmp(str1, str2, 6);

    if (result == 0)
        CamOsPrintf("Strings compared equal.\n");
    else if (result < 0)
        CamOsPrintf("\"%s\" is less than \"%s\".\n", str1, str2);
    else
        CamOsPrintf("\"%s\" is greater than \"%s\".\n", str1, str2);

    return 0;
}
#endif

#ifdef TEST_STRLEN
int KernelTestStrlen(void)
{
    char str[]  = "This is CamOs UT";
    int  length = strlen(str);
    CamOsPrintf("Length of string is : %d", length);

    return 0;
}
#endif

#ifdef TEST_STRCAT
int KernelTestStrcat(void)
{
    char str[80];
    CamOsStrncpy(str, "these ", sizeof(str));
    CamOsStrcat(str, "strings ");
    CamOsStrcat(str, "are ");
    CamOsStrcat(str, "concatenated.");
    CamOsPrintf(str);
    return 0;
}
#endif

#ifdef TEST_STRSEP
int KernelTestStrsep(void)
{
    char  s_in[50] = "Today is a great day for programming";
    char  del[20]  = " ";
    char *in_Ptr   = s_in;
    char *o_Ptr;

    while (o_Ptr != NULL)
    {
        o_Ptr = CamOsStrsep(&in_Ptr, del);
        CamOsPrintf("%s\n\n", o_Ptr);
    }

    return 0;
}
#endif

#ifdef TEST_STRCHR
int KernelTestStrchr(void)
{
    char  str[] = "This is a sample string";
    char *pch;

    CamOsPrintf("Looking for the 's' character in \"%s\"...\n", str);
    pch = CamOsStrchr(str, 's');
    while (pch != NULL)
    {
        CamOsPrintf("found at %d\n", pch - str + 1);
        pch = CamOsStrchr(pch + 1, 's');
    }

    return 0;
}
#endif

#ifdef TEST_STRRCHR
int KernelTestStrrchr(void)
{
    char  str[] = "This is a sample string";
    char *pch;

    pch = CamOsStrrchr(str, 's');
    CamOsPrintf("Last occurence of 's' found at %d \n", pch - str + 1);

    return 0;
}
#endif

#ifdef TEST_QSORT
int g_qsortValues[] = {40, 10, 100, 90, 20, 25};

int QsortCompare(const void *a, const void *b)
{
    return (*(int *)a - *(int *)b);
}

int KernelTestQsort(void)
{
    int n;
    CamOsQsort(g_qsortValues, 6, sizeof(int), QsortCompare);
    for (n = 0; n < 6; n++)
        CamOsPrintf("%d ", g_qsortValues[n]);
    return 0;
}
#endif

static int __init KernelTestInit(void)
{
#ifdef TEST_TIME_AND_SLEEP
    KernelTestTimeAndSleep();
#endif

#ifdef TEST_PHY_MAP_VIRT
    KernelTestPhyMapVirt();
#endif

#ifdef TEST_CACHE_ALLOC
    KernelTestCacheAlloc();
#endif

#ifdef TEST_THREAD
    KernelTestThread();
#endif

#ifdef TEST_THREAD_PRIORITY
    KernelTestThreadPriority();
#endif

#ifdef TEST_SEMAPHORE
    KernelTestSemaphore();
#endif

#ifdef TEST_RW_SEMAPHORE
    KernelTestRwSemaphore();
#endif

#ifdef TEST_MUTEX
    KernelTestMutex();
#endif

#ifdef TEST_DIV64
    KernelTestDiv64();
#endif

#ifdef TEST_SYSTEM_TIME
    KernelTestSystemTime();
#endif

#ifdef TEST_MEM_SIZE
    KernelTestMemSize();
#endif

#ifdef TEST_CHIP_ID
    KernelTestChipId();
#endif

#ifdef TEST_ATOMIC_OPERATION
    KernelTestAtomicOperation();
#endif

#ifdef TEST_BITMAP
    KernelTestBitmap();
#endif

#ifdef TEST_HASH
    KernelTestHash();
#endif

#ifdef TEST_IDR
    KernelTestIdr();
#endif

#ifdef TEST_TIMER
    KernelTestTimer();
#endif

#ifdef TEST_SPINLOCK
    KernelTestSpinlock();
#endif

#ifdef TEST_CONDITION
    KernelTestCondition();
#endif

#ifdef TEST_CPU_RUNTIME
    KernelTestCpuRunTime();
#endif

#ifdef TEST_WORK_QUEUE
    KernelTestWorkQueue();
#endif

#ifdef TEST_MEMCPY
    KernelTestMemcpy();
#endif

#ifdef TEST_MEMMOVE
    KernelTestMemmove();
#endif

#ifdef TEST_MEMSET
    KernelTestMemset();
#endif

#ifdef TEST_MEMCMP
    KernelTestMemcmp();
#endif

#ifdef TEST_STRSTR
    KernelTestStrstr();
#endif

#ifdef TEST_STRNCPY
    KernelTestStrncpy();
#endif

#ifdef TEST_STRNCMP
    KernelTestStrncmp();
#endif

#ifdef TEST_STRNCASECMP
    KernelTestStrncasecmp();
#endif

#ifdef TEST_STRLEN
    KernelTestStrlen();
#endif

#ifdef TEST_STRCAT
    KernelTestStrcat();
#endif

#ifdef TEST_STRSEP
    KernelTestStrsep();
#endif

#ifdef TEST_STRCHR
    KernelTestStrchr();
#endif

#ifdef TEST_STRRCHR
    KernelTestStrrchr();
#endif

#ifdef TEST_QSORT
    KernelTestQsort();
#endif

    return 0;
}

static void __exit KernelTestExit(void)
{
    CamOsPrintf("Goodbye\n");
}

module_init(KernelTestInit);
module_exit(KernelTestExit);
