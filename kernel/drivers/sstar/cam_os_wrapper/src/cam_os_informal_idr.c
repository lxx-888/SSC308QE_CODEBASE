/*
 * cam_os_informal_idr.c- Sigmastar
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
/// @file      cam_os_informal_idr.c
/// @brief     Cam OS Informal IDR Source File for Linux User Space and RTK.
///            It's Not A Standard IDR Algorithm.
///////////////////////////////////////////////////////////////////////////////

#if defined(CAM_OS_RTK) || defined(CAM_OS_LINUX_USER)
#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include "cam_os_wrapper.h"
#include "cam_os_util_bitmap.h"

#define IDR_ENTRY_NUM 0x4000

typedef struct
{
    void **         ppEntryPtr;
    unsigned long * pBitmap;
    unsigned long   entry_num;
    unsigned long   next;
    CamOsSpinlock_t lock;
} CamOsInformalIdr_t, *pCamOsInformalIdr;

CamOsRet_e _CamOsIdrInit(CamOsIdr_t *ptIdr, u32 nEntryNum)
{
    CamOsRet_e          eRet         = CAM_OS_OK;
    CamOsInformalIdr_t *pInformalIdr = CamOsMemCallocAtomic(sizeof(CamOsInformalIdr_t), 1);

    if (pInformalIdr)
    {
        pInformalIdr->entry_num = (nEntryNum) ? nEntryNum : IDR_ENTRY_NUM;
        pInformalIdr->next      = 0;

        if (NULL == (pInformalIdr->ppEntryPtr = CamOsMemCalloc(sizeof(void *) * pInformalIdr->entry_num, 1)))
            eRet = CAM_OS_ALLOCMEM_FAIL;

        if (NULL
            == (pInformalIdr->pBitmap =
                    CamOsMemCalloc(sizeof(unsigned long) * CAM_OS_BITS_TO_LONGS(pInformalIdr->entry_num), 1)))
        {
            CamOsMemRelease(pInformalIdr->ppEntryPtr);
            pInformalIdr->ppEntryPtr = NULL;
            eRet                     = CAM_OS_ALLOCMEM_FAIL;
        }

        CamOsSpinInit(&pInformalIdr->lock);
        *ptIdr = pInformalIdr;
    }
    else
    {
        eRet = CAM_OS_FAIL;
    }

    return eRet;
}

void _CamOsIdrDestroy(CamOsIdr_t *ptIdr)
{
    CamOsInformalIdr_t *pInformalIdr = (CamOsInformalIdr_t *)*ptIdr;

    if (pInformalIdr)
    {
        if (pInformalIdr->ppEntryPtr)
            CamOsMemRelease(pInformalIdr->ppEntryPtr);

        if (pInformalIdr->pBitmap)
            CamOsMemRelease(pInformalIdr->pBitmap);

        CamOsSpinDeinit(&pInformalIdr->lock);
        CamOsMemRelease(*ptIdr);
        *ptIdr = NULL;
    }
}

s32 _CamOsIdrAlloc(CamOsIdr_t *ptIdr, void *pDataPtr, s32 nStart, s32 nEnd)
{
    CamOsInformalIdr_t *pInformalIdr = (CamOsInformalIdr_t *)*ptIdr;
    s32                 nEmptyID     = -1;
    s32                 end          = nEnd;

    if (!pInformalIdr)
    {
        CamOsCallStack();
        CamOsPanic("IDR may not initialized before use\n");
    }

    if (pInformalIdr && pDataPtr && pInformalIdr->ppEntryPtr)
    {
        if (nEnd < nStart || nEnd <= 0 || nEnd > pInformalIdr->entry_num)
            end = pInformalIdr->entry_num;

        CamOsSpinLockIrqSave(&pInformalIdr->lock);

        nEmptyID = CAM_OS_FIND_NEXT_ZERO_BIT(pInformalIdr->pBitmap, pInformalIdr->entry_num, nStart);

        if (nEmptyID >= nStart && nEmptyID < end)
        {
            pInformalIdr->ppEntryPtr[nEmptyID] = pDataPtr;
            CAM_OS_SET_BIT(nEmptyID, pInformalIdr->pBitmap);
            pInformalIdr->next = nEmptyID + 1;
        }
        else
        {
            nEmptyID = -1;
        }

        CamOsSpinUnlockIrqRestore(&pInformalIdr->lock);
    }

    return nEmptyID;
}

s32 _CamOsIdrAllocCyclic(CamOsIdr_t *ptIdr, void *pDataPtr, s32 nStart, s32 nEnd)
{
    CamOsInformalIdr_t *pInformalIdr = (CamOsInformalIdr_t *)*ptIdr;
    s32                 nEmptyID     = _CamOsIdrAlloc(ptIdr, pDataPtr, CAM_OS_MAX(pInformalIdr->next, nStart), nEnd);

    if (nEmptyID < 0)
    {
        nEmptyID = _CamOsIdrAlloc(ptIdr, pDataPtr, nStart, nEnd);
    }

    return nEmptyID;
}

void _CamOsIdrRemove(CamOsIdr_t *ptIdr, s32 nId)
{
    CamOsInformalIdr_t *pInformalIdr = (CamOsInformalIdr_t *)*ptIdr;

    if (!pInformalIdr)
    {
        CamOsCallStack();
        CamOsPanic("IDR may not initialized before use\n");
    }

    if (pInformalIdr && pInformalIdr->ppEntryPtr)
    {
        CamOsSpinLockIrqSave(&pInformalIdr->lock);
        pInformalIdr->ppEntryPtr[nId] = NULL;
        CAM_OS_CLEAR_BIT(nId, pInformalIdr->pBitmap);
        CamOsSpinUnlockIrqRestore(&pInformalIdr->lock);
    }
}

void *_CamOsIdrFind(CamOsIdr_t *ptIdr, s32 nId)
{
    CamOsInformalIdr_t *pInformalIdr = (CamOsInformalIdr_t *)*ptIdr;
    void *              ret          = NULL;

    if (!pInformalIdr)
    {
        CamOsCallStack();
        CamOsPanic("IDR may not initialized before use\n");
    }

    if (pInformalIdr && pInformalIdr->ppEntryPtr)
    {
        CamOsSpinLockIrqSave(&pInformalIdr->lock);
        if (CAM_OS_TEST_BIT(nId, pInformalIdr->pBitmap))
        {
            ret = pInformalIdr->ppEntryPtr[nId];
        }
        CamOsSpinUnlockIrqRestore(&pInformalIdr->lock);
    }

    return ret;
}
#endif
