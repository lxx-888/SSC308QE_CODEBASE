/*
 * drvSEM.c- Sigmastar
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
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/time64.h>
#include "ms_types.h"
#include "drvSEM.h"
#include "halSEM.h"

//-------------------------------------------------------------------------------------------------
//  Local Defines
//-------------------------------------------------------------------------------------------------
#define SEM_RESOURCE_ID SEM_ARM_NS_ID

static BOOL         _gbSEMInitialized = FALSE;
static struct mutex stMutex[E_SEM_MAX_NUM];

//-------------------------------------------------------------------------------------------------
//  Debug Functions
//-------------------------------------------------------------------------------------------------
#define SEM_PRINT pr_info
#define SEM_DEBUG pr_debug
#define SEM_ERROR pr_err
#define SEM_WARN  pr_warn

#define SEM_MUTEX_CREATE(_index)        MDrv_CreateMutex(_index)
#define SEM_MUTEX_LOCK(_index, _waitms) MDrv_ObtainMutex(_index, _waitms)
#define SEM_MUTEX_UNLOCK(_index)        MDrv_ReleaseMutex(_index)

//-------------------------------------------------------------------------------------------------
// Gloabal Functions
//-------------------------------------------------------------------------------------------------
static BOOL MDrv_CreateMutex(S32 s32MutexId)
{
    mutex_init(&stMutex[s32MutexId]);
    return TRUE;
}

static BOOL MDrv_ObtainMutex(S32 s32MutexId, U32 u32WaitMs)
{
    BOOL bRet = FALSE;

    if (u32WaitMs == MSOS_WAIT_FOREVER) // blocking wait
    {
        mutex_lock(&stMutex[s32MutexId]);
        bRet = TRUE;
    }
    else if (u32WaitMs == 0) // non-blocking
    {
        if (!mutex_trylock(&stMutex[s32MutexId]))
        {
            bRet = TRUE;
        }
    }
    else // blocking wait with timeout
    {
        mutex_lock(&stMutex[s32MutexId]);
        bRet = TRUE;
    }
    return bRet;
}

static BOOL MDrv_ReleaseMutex(S32 s32MutexId)
{
    mutex_unlock(&stMutex[s32MutexId]);
    return TRUE;
}

static U32 MDrv_GetSystemTime(void)
{
    struct timespec64 ts;

    ktime_get_raw_ts64(&ts);
    return ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
}

//-------------------------------------------------------------------------------------------------
/// Attempt to create mutex
/// @return TRUE : succeed
/// @return FALSE : fail
//-------------------------------------------------------------------------------------------------
BOOL MDrv_SEM_Init(void)
{
    S32 s32Index;

    if (_gbSEMInitialized)
        return TRUE;

    for (s32Index = 0; s32Index < E_SEM_MAX_NUM; s32Index++)
    {
        SEM_MUTEX_CREATE(s32Index);
        SEM_DEBUG("s32Index = %d\n", s32Index);
    }

    _gbSEMInitialized = TRUE;
    return TRUE;
}
EXPORT_SYMBOL(MDrv_SEM_Init);

//-------------------------------------------------------------------------------------------------
/// Attempt to get resource
/// @return TRUE : succeed
/// @return FALSE : fail
//-------------------------------------------------------------------------------------------------
BOOL MDrv_SEM_Get_Resource(U8 u8SemID, U16 u16ResId)
{
    if (!_gbSEMInitialized)
    {
        SEM_WARN("%s is called before init\n", __FUNCTION__);
        return FALSE;
    }

    return HAL_SEM_Get_Resource(u8SemID, u16ResId);
}
EXPORT_SYMBOL(MDrv_SEM_Get_Resource);

//-------------------------------------------------------------------------------------------------
/// Attempt to free resource
/// @return TRUE : succeed
/// @return FALSE : fail
//-------------------------------------------------------------------------------------------------
BOOL MDrv_SEM_Free_Resource(U8 u8SemID, U16 u16ResId)
{
    if (!_gbSEMInitialized)
    {
        SEM_WARN("%s is called before init\n", __FUNCTION__);
        return FALSE;
    }

    return HAL_SEM_Free_Resource(u8SemID, u16ResId);
}
EXPORT_SYMBOL(MDrv_SEM_Free_Resource);

//-------------------------------------------------------------------------------------------------
/// Attempt to reset resource
/// @return TRUE : succeed
/// @return FALSE : fail
//-------------------------------------------------------------------------------------------------
BOOL MDrv_SEM_Reset_Resource(U8 u8SemID)
{
    if (!_gbSEMInitialized)
    {
        SEM_WARN("%s is called before init\n", __FUNCTION__);
        return FALSE;
    }

    return HAL_SEM_Reset_Resource(u8SemID);
}
EXPORT_SYMBOL(MDrv_SEM_Reset_Resource);

//-------------------------------------------------------------------------------------------------
/// Attempt to get resource ID
/// @return TRUE : succeed
/// @return FALSE : fail
//-------------------------------------------------------------------------------------------------
BOOL MDrv_SEM_Get_ResourceID(U8 u8SemID, U16* pu16ResId)
{
    if (!_gbSEMInitialized)
    {
        SEM_WARN("%s is called before init\n", __FUNCTION__);
        return FALSE;
    }

    return HAL_SEM_Get_ResourceID(u8SemID, pu16ResId);
}
EXPORT_SYMBOL(MDrv_SEM_Get_ResourceID);

//-------------------------------------------------------------------------------------------------
/// Attempt to get semaphore number
/// @return TRUE : succeed
/// @return FALSE : fail
//-------------------------------------------------------------------------------------------------
U32 MDrv_SEM_Get_Num(void)
{
    return HAL_SEM_Get_Num();
}
EXPORT_SYMBOL(MDrv_SEM_Get_Num);

//-------------------------------------------------------------------------------------------------
/// Attempt to lock a hardware semaphore
/// @param  SemId       \b IN: hardware semaphore ID
/// @param  u32WaitMs   \b IN: 0 ~ SEM_WAIT_FOREVER: suspend time (ms) if the mutex is locked
/// @return TRUE : succeed
/// @return FALSE : fail
//-------------------------------------------------------------------------------------------------
BOOL MDrv_SEM_Lock(eSemId SemId, U32 u32WaitMs)
{
    S16  s16SemId;
    BOOL bRet;
    U32  u32SysOldTime, u32Interval;

    if (!_gbSEMInitialized)
    {
        SEM_WARN("%s is called before init\n", __FUNCTION__);
        return FALSE;
    }

    SEM_DEBUG("Lock SemId = %d\n", SemId);
    SEM_DEBUG("Lock u32WaitMs = %d\n", u32WaitMs);

    bRet          = FALSE;
    s16SemId      = HAL_SEM_GetSemId(SemId);
    u32SysOldTime = MDrv_GetSystemTime();
    u32Interval   = 0;

    if (s16SemId < 0)
    {
        SEM_ERROR("Lock SemId%d invalid\n", SemId);
        return FALSE;
    }

    SEM_DEBUG("Lock s16SemId = %d\n", s16SemId);

    /*blocking*/
    if (u32WaitMs == SEM_WAIT_FOREVER)
    {
        bRet = SEM_MUTEX_LOCK(s16SemId, MSOS_WAIT_FOREVER);

        if (bRet == FALSE)
        {
            SEM_ERROR("Obtain mutex %d failed\n", s16SemId);
        }
        else
        {
            do
            {
                bRet = HAL_SEM_Get_Resource((U8)s16SemId, SEM_RESOURCE_ID);
            } while (bRet != TRUE);

            if (bRet == FALSE)
            {
                SEM_MUTEX_UNLOCK(s16SemId);
                SEM_ERROR("Obtain hardware semaphore %d failed\n", s16SemId);
            }
        }
    }
    /*blocking with timeout*/
    else
    {
        bRet = SEM_MUTEX_LOCK(s16SemId, (u32WaitMs - u32Interval));

        if (bRet == FALSE)
        {
            SEM_ERROR("Obtain mutex %d failed\n", s16SemId);
        }
        else
        {
            do
            {
                bRet        = HAL_SEM_Get_Resource((U8)s16SemId, SEM_RESOURCE_ID);
                u32Interval = MDrv_GetSystemTime() - u32SysOldTime;
            } while ((bRet != TRUE) && (u32Interval < u32WaitMs));

            if (bRet == FALSE)
            {
                SEM_MUTEX_UNLOCK(s16SemId);
                SEM_ERROR("Obtain hardware semaphore %d failed, timeout=%d\n", s16SemId, u32WaitMs);
            }
        }
    }

    return bRet;
}
EXPORT_SYMBOL(MDrv_SEM_Lock);

//-------------------------------------------------------------------------------------------------
/// Attempt to unlock a hardware semaphore
/// @param  SemId       \b IN: hardware semaphore ID
/// @return TRUE : succeed
/// @return FALSE : fail
//-------------------------------------------------------------------------------------------------
BOOL MDrv_SEM_Unlock(eSemId SemId)
{
    S16  s16SemId;
    BOOL bRet;

    if (!_gbSEMInitialized)
    {
        SEM_WARN("%s is called before init\n", __FUNCTION__);
        return FALSE;
    }

    bRet = FALSE;
    SEM_DEBUG("Unlock SemId = %d\n", SemId);
    s16SemId = HAL_SEM_GetSemId(SemId);

    if (s16SemId < 0)
    {
        SEM_ERROR("Unlock SemId%d invalid\n", SemId);
        return FALSE;
    }

    SEM_DEBUG("Unlock s16SemId = %d\n", s16SemId);

    bRet = HAL_SEM_Free_Resource((U8)s16SemId, SEM_RESOURCE_ID);

    if (bRet == FALSE)
    {
        SEM_ERROR("Release hardware semaphore %d failed\n", s16SemId);
    }

    SEM_MUTEX_UNLOCK(s16SemId);

    return bRet;
}
EXPORT_SYMBOL(MDrv_SEM_Unlock);

//-------------------------------------------------------------------------------------------------
/// Attempt to delete a hardware semaphore
/// @param  SemId       \b IN: hardware semaphore ID
/// @return TRUE : succeed
/// @return FALSE : fail
//-------------------------------------------------------------------------------------------------
BOOL MDrv_SEM_Delete(eSemId SemId)
{
    S16  s16SemId;
    BOOL bRet;

    if (!_gbSEMInitialized)
    {
        SEM_WARN("%s is called before init\n", __FUNCTION__);
        return FALSE;
    }

    bRet = FALSE;
    SEM_DEBUG("Delete SemId = %d\n", SemId);
    s16SemId = HAL_SEM_GetSemId(SemId);

    if (s16SemId < 0)
    {
        return FALSE;
    }

    bRet = SEM_MUTEX_UNLOCK(s16SemId);

    if (bRet == FALSE)
    {
        SEM_ERROR("Release mutex %d failed\n", s16SemId);
    }
    else
    {
        bRet = HAL_SEM_Reset_Resource((U8)s16SemId);

        if (bRet == FALSE)
        {
            SEM_ERROR("Reset hardware semaphore %d failed\n", s16SemId);
        }
    }

    return bRet;
}
EXPORT_SYMBOL(MDrv_SEM_Delete);
