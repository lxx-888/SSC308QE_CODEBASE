/*
 * cam_fs_wrapper.c- Sigmastar
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
/// @file      cam_fs_wrapper.c
/// @brief     Cam FS Wrapper Source File for
///            1. RTK OS
///            2. Linux User Space
///            3. Linux Kernel Space
///////////////////////////////////////////////////////////////////////////////

#if defined(__KERNEL__)
#define CAM_OS_LINUX_KERNEL
#endif

#ifdef CAM_OS_RTK
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <fcntl.h>
#include "drv_flash_os_impl.h"
#include "cam_os_wrapper.h"
#include "cam_fs_wrapper.h"
#if defined(CONFIG_LWFS_SUPPORT)
#include "lwfs.h"
#endif
#if defined(CONFIG_PROXYFS_SUPPORT)
#include "proxyfs_client.h"
#endif
#if defined(CONFIG_LITTLEFS_SUPPORT)
#include "littlefs.h"
#endif
#if defined(CONFIG_FIRMWAREFS_SUPPORT)
#include "firmwarefs.h"
#endif

#define MNT_PATH_MAX 32

typedef struct
{
    struct CamOsListHead_t tList;
    char                   mnt_path[MNT_PATH_MAX];
    CamFsFmt_e             fmt;
    void *                 handle;
} CamFsMntPoint_t, *pCamFsMntPoint_t;

typedef struct
{
    CamFsFmt_e fmt;
    void *     fd;
} CamFsFdRtk_t, *pCamFsFdRtk_t;

static CamFsMntPoint_t _gtMntPointList      = {0};
static u32             _gMntPointListInited = 0;
static CamOsMutex_t    gMntPointListLock    = {0};

#elif defined(CAM_OS_LINUX_USER)
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include "cam_os_wrapper.h"
#include "cam_fs_wrapper.h"

#elif defined(CAM_OS_LINUX_KERNEL)
#include <linux/module.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/unistd.h>
#include <linux/sched.h>
#include <linux/fs.h>
#include <linux/file.h>
#include <linux/mm.h>
#include <linux/version.h>
#include <asm/uaccess.h>
#include "cam_os_wrapper.h"
#include "cam_fs_wrapper.h"
#endif

#define TO_STR_NATIVE(e)   #e
#define MACRO_TO_STRING(e) TO_STR_NATIVE(e)
char cam_fs_wrapper_version_string[] = MACRO_TO_STRING(SIGMASTAR_MODULE_VERSION) " wrapper." CAM_FS_WRAPPER_VERSION;

#define LWFS_MNT_PATH "/mnt/"
#define MIN(a, b)     ((a) < (b) ? (a) : (b))

CamFsRet_e CamFsMount(CamFsFmt_e fmt, const char *szPartName, const char *szMntPath)
{
#ifdef CAM_OS_RTK
    void *                  mnt_ret = NULL;
    struct CamOsListHead_t *ptPos, *ptQ;
    CamFsMntPoint_t *       pNewMntPoint = NULL, *pMntPoint = NULL;
    u32                     mnt_path_busy = 0;
    CamFsRet_e              eRet          = CAM_FS_FAIL;
    u8                      u8MntPathLen  = strlen(szMntPath);
    char                    TmpMntPath[MNT_PATH_MAX];
    u8                      u8TmpMntPathLen   = 0;
    u8                      u8MntPointPathLen = 0;

    if (u8MntPathLen > (MNT_PATH_MAX - 2))
    {
        CamOsPrintf("%s: mount path too long (%s)\n", __FUNCTION__, szMntPath);
        goto cam_os_mount_err;
    }

    strncpy(TmpMntPath, szMntPath, sizeof(TmpMntPath) - 1);
    if (!((u8MntPathLen == 1 && szMntPath[0] == '/') || szMntPath[u8MntPathLen - 1] == '/'))
    {
        TmpMntPath[u8MntPathLen] = '/';
    }
    u8TmpMntPathLen = strlen(TmpMntPath);

    CamOsMutexLock(&gMntPointListLock);
    if (!_gMntPointListInited)
    {
        memset(&_gtMntPointList, 0, sizeof(_gtMntPointList));
        CAM_OS_INIT_LIST_HEAD(&_gtMntPointList.tList);

        _gMntPointListInited = 1;
    }

    CAM_OS_LIST_FOR_EACH_SAFE(ptPos, ptQ, &_gtMntPointList.tList)
    {
        pMntPoint         = CAM_OS_LIST_ENTRY(ptPos, CamFsMntPoint_t, tList);
        u8MntPointPathLen = strlen(pMntPoint->mnt_path);

        if (!strncmp(pMntPoint->mnt_path, TmpMntPath, MIN(u8TmpMntPathLen, u8MntPointPathLen))
            && (u8TmpMntPathLen >= u8MntPointPathLen))
        {
            CamOsPrintf("%s: device or resource busy (%s)\n", __FUNCTION__, szMntPath);
            mnt_path_busy = 1;
            break;
        }
    }
    CamOsMutexUnlock(&gMntPointListLock);

    if (mnt_path_busy)
        goto cam_os_mount_err;

    switch (fmt)
    {
#if defined(CONFIG_LWFS_SUPPORT)
        case CAM_FS_FMT_LWFS:
            mnt_ret = lwfs_mount((char *)szPartName, (char *)szMntPath);
            break;
#endif
#if defined(CONFIG_LITTLEFS_SUPPORT)
        case CAM_FS_FMT_LITTLEFS:
            mnt_ret = littlefs_mount((char *)szPartName, (char *)szMntPath);
            break;
#endif
#if defined(CONFIG_PROXYFS_SUPPORT)
        case CAM_FS_FMT_PROXYFS:
            mnt_ret = (void *)1;
            break;
#endif
#if defined(CONFIG_FIRMWAREFS_SUPPORT)
        case CAM_FS_FMT_FIRMWAREFS:
            mnt_ret = firmwarefs_mount((char *)szPartName, (char *)szMntPath);
            break;
#endif
        default:
            CamOsPrintf("%s: unsupported filesystem id (0x%x)\n", __FUNCTION__, (u32)fmt);
            break;
    }

    if (mnt_ret)
    {
        pNewMntPoint = (CamFsMntPoint_t *)CamOsMemCalloc(sizeof(CamFsMntPoint_t), 1);
        if (pNewMntPoint)
        {
            strncpy(pNewMntPoint->mnt_path, TmpMntPath, sizeof(pNewMntPoint->mnt_path) - 1);
            pNewMntPoint->fmt    = fmt;
            pNewMntPoint->handle = mnt_ret;
            CamOsMutexLock(&gMntPointListLock);
            CAM_OS_LIST_ADD_TAIL(&(pNewMntPoint->tList), &_gtMntPointList.tList);
            CamOsMutexUnlock(&gMntPointListLock);
            eRet = CAM_FS_OK;
        }
    }

    return eRet;

cam_os_mount_err:
    if (pNewMntPoint)
        CamOsMemRelease(pNewMntPoint);

    return eRet;
#elif defined(CAM_OS_LINUX_USER)
    return CAM_FS_OK;
#elif defined(CAM_OS_LINUX_KERNEL)
    return CAM_FS_OK;
#endif
}

CamFsRet_e CamFsUnmount(const char *szMntPath)
{
#ifdef CAM_OS_RTK
    struct CamOsListHead_t *ptPos, *ptQ;
    CamFsMntPoint_t *       pMntPoint    = NULL;
    CamFsRet_e              eRet         = CAM_FS_FAIL;
    u8                      u8MntPathLen = strlen(szMntPath);
    char                    TmpMntPath[MNT_PATH_MAX];

    strncpy(TmpMntPath, szMntPath, sizeof(TmpMntPath) - 1);
    if (!((u8MntPathLen == 1 && szMntPath[0] == '/') || szMntPath[u8MntPathLen - 1] == '/'))
    {
        TmpMntPath[u8MntPathLen] = '/';
    }

    CamOsMutexLock(&gMntPointListLock);
    if (!_gMntPointListInited)
    {
        memset(&_gtMntPointList, 0, sizeof(_gtMntPointList));
        CAM_OS_INIT_LIST_HEAD(&_gtMntPointList.tList);

        _gMntPointListInited = 1;
    }

    CAM_OS_LIST_FOR_EACH_SAFE(ptPos, ptQ, &_gtMntPointList.tList)
    {
        pMntPoint = CAM_OS_LIST_ENTRY(ptPos, CamFsMntPoint_t, tList);

        if (!strncmp(pMntPoint->mnt_path, TmpMntPath, sizeof(pMntPoint->mnt_path)))
        {
            CAM_OS_LIST_DEL(ptPos);
            switch (pMntPoint->fmt)
            {
#if defined(CONFIG_LWFS_SUPPORT)
                case CAM_FS_FMT_LWFS:
                    lwfs_unmount(pMntPoint->handle);
                    break;
#endif
#if defined(CONFIG_LITTLEFS_SUPPORT)
                case CAM_FS_FMT_LITTLEFS:
                    littlefs_unmount(pMntPoint->handle);
                    break;
#endif
#if defined(CONFIG_PROXYFS_SUPPORT)
                case CAM_FS_FMT_PROXYFS:
                    break;
#endif
#if defined(CONFIG_FIRMWAREFS_SUPPORT)
                case CAM_FS_FMT_FIRMWAREFS:
                    firmwarefs_unmount(pMntPoint->handle);
                    break;
#endif
                default:
                    CamOsPrintf("%s: unsupported filesystem id (0x%x)\n", __FUNCTION__, (u32)pMntPoint->fmt);
                    break;
            }
            CamOsMemRelease(pMntPoint);
        }
    }
    CamOsMutexUnlock(&gMntPointListLock);

    return eRet;
#elif defined(CAM_OS_LINUX_USER)
    return CAM_FS_OK;
#elif defined(CAM_OS_LINUX_KERNEL)
    return CAM_FS_OK;
#endif
}

CamFsRet_e CamFsOpen(CamFsFd *ptFd, const char *szPath, u32 nFlag, u32 nMode)
{
#ifdef CAM_OS_RTK
    int                     osFlags = 0;
    CamFsFdRtk_t *          ptFdRtk;
    void *                  open_ret = NULL;
    char *                  ch       = NULL;
    struct CamOsListHead_t *ptPos, *ptQ;
    CamFsMntPoint_t *       pMntPoint       = NULL;
    CamFsMntPoint_t *       pTmpMntPoint    = NULL;
    u8                      u8MntPathLen    = MNT_PATH_MAX;
    u8                      u8TmpMntPathLen = 0;
    CamFsRet_e              eRet            = CAM_FS_FAIL;

    if (!strlen(szPath))
    {
        CamOsPrintf(KERN_ERR "CamFsOpen with invalidate filename\n");
        return CAM_FS_FAIL;
    }

    if ((nFlag & 0xFFFF) != (nFlag >> 16))
    {
        CamOsPrintf(KERN_ERR "CamFsOpen with invalidate flags, please use CAM_FS_O_ flag\n");
        return CAM_FS_FAIL;
    }

    CamOsMutexLock(&gMntPointListLock);
    if (!_gMntPointListInited)
    {
        memset(&_gtMntPointList, 0, sizeof(_gtMntPointList));
        CAM_OS_INIT_LIST_HEAD(&_gtMntPointList.tList);

        _gMntPointListInited = 1;
    }

    CAM_OS_LIST_FOR_EACH_SAFE(ptPos, ptQ, &_gtMntPointList.tList)
    {
        pTmpMntPoint    = CAM_OS_LIST_ENTRY(ptPos, CamFsMntPoint_t, tList);
        u8TmpMntPathLen = strlen(pTmpMntPoint->mnt_path);

        if (!strncmp(pTmpMntPoint->mnt_path, szPath, u8TmpMntPathLen))
        {
            if (u8TmpMntPathLen < u8MntPathLen)
            {
                pMntPoint    = pTmpMntPoint;
                u8MntPathLen = u8TmpMntPathLen;
            }
        }
    }

    if (pMntPoint == NULL)
    {
        CamOsPrintf("%s: no mount point found (%s)\n", __FUNCTION__, szPath);
        goto cam_os_open_err;
    }

    ch = (char *)szPath + strlen(pMntPoint->mnt_path);
    while (*ch == '/')
    {
        ch++;
    }

    switch (pMntPoint->fmt)
    {
#if defined(CONFIG_LWFS_SUPPORT)
        case CAM_FS_FMT_LWFS:
            if (nFlag & CAM_FS_O_RDONLY)
                osFlags |= O_RDONLY;
            if (nFlag & CAM_FS_O_WRONLY)
                osFlags |= O_WRONLY;
            if (nFlag & CAM_FS_O_RDWR)
                osFlags |= O_RDWR;
            if (nFlag & CAM_FS_O_CREAT)
                osFlags |= O_CREAT;
            if (nFlag & CAM_FS_O_APPEND)
                osFlags |= O_APPEND;
            if (nFlag & CAM_FS_O_TRUNC)
                osFlags |= O_TRUNC;

            open_ret = lwfs_open(pMntPoint->handle, ch, osFlags, nMode);
            break;
#endif
#if defined(CONFIG_LITTLEFS_SUPPORT)
        case CAM_FS_FMT_LITTLEFS:
            if (nFlag & CAM_FS_O_RDONLY)
                osFlags |= O_RDONLY;
            if (nFlag & CAM_FS_O_WRONLY)
                osFlags |= O_WRONLY;
            if (nFlag & CAM_FS_O_RDWR)
                osFlags |= O_RDWR;
            if (nFlag & CAM_FS_O_CREAT)
                osFlags |= O_CREAT;
            if (nFlag & CAM_FS_O_APPEND)
                osFlags |= O_APPEND;
            if (nFlag & CAM_FS_O_TRUNC)
                osFlags |= O_TRUNC;

            open_ret = littlefs_open(pMntPoint->handle, ch, osFlags, nMode);
            break;
#endif
#if defined(CONFIG_PROXYFS_SUPPORT)
        case CAM_FS_FMT_PROXYFS:
            if (nFlag & CAM_FS_O_RDONLY)
                osFlags |= PROXY_FS_O_RDONLY;
            if (nFlag & CAM_FS_O_WRONLY)
                osFlags |= PROXY_FS_O_WRONLY;
            if (nFlag & CAM_FS_O_RDWR)
                osFlags |= PROXY_FS_O_RDWR;
            if (nFlag & CAM_FS_O_CREAT)
                osFlags |= PROXY_FS_O_CREAT;
            if (nFlag & CAM_FS_O_APPEND)
                osFlags |= PROXY_FS_O_APPEND;
            if (nFlag & CAM_FS_O_TRUNC)
                osFlags |= PROXY_FS_O_TRUNC;

            open_ret = proxyfs_client_open(NULL, (char *)szPath, osFlags, nMode);
            break;
#endif
#if defined(CONFIG_FIRMWAREFS_SUPPORT)
        case CAM_FS_FMT_FIRMWAREFS:
            if (nFlag & CAM_FS_O_RDONLY)
                osFlags |= O_RDONLY;
            if (nFlag & CAM_FS_O_WRONLY)
                osFlags |= O_WRONLY;
            if (nFlag & CAM_FS_O_RDWR)
                osFlags |= O_RDWR;
            if (nFlag & CAM_FS_O_CREAT)
                osFlags |= O_CREAT;
            if (nFlag & CAM_FS_O_APPEND)
                osFlags |= O_APPEND;
            if (nFlag & CAM_FS_O_TRUNC)
                osFlags |= O_TRUNC;

            open_ret = firmwarefs_open(pMntPoint->handle, ch, osFlags, nMode);
            break;
#endif
        default:
            osFlags = osFlags; // avoid unused variable warning.
            CamOsPrintf("%s: unsupported filesystem id (0x%x)\n", __FUNCTION__, (u32)pMntPoint->fmt);
            break;
    }

    if (open_ret)
    {
        ptFdRtk      = (CamFsFdRtk_t *)CamOsMemCalloc(1, sizeof(CamFsFdRtk_t));
        ptFdRtk->fmt = pMntPoint->fmt;
        ptFdRtk->fd  = open_ret;
        *ptFd        = (CamFsFd *)ptFdRtk;
        eRet         = CAM_FS_OK;
    }

cam_os_open_err:
    CamOsMutexUnlock(&gMntPointListLock);

    return eRet;
#elif defined(CAM_OS_LINUX_USER)
    int open_ret = 0;
    int osFlags  = 0;
    if (!strlen(szPath))
    {
        CamOsPrintf("CamFsOpen with invalidate filename\n");
        return CAM_FS_FAIL;
    }

    if (nFlag == 0 || (nFlag & 0xFFFF) != (nFlag >> 16))
    {
        CamOsPrintf("CamFsOpen with invalidate flags, please use CAM_FS_O_ flag\n");
        return CAM_FS_FAIL;
    }
    else
    {
        if (nFlag & CAM_FS_O_RDONLY)
            osFlags |= O_RDONLY;

        if (nFlag & CAM_FS_O_WRONLY)
            osFlags |= O_WRONLY;

        if (nFlag & CAM_FS_O_RDWR)
            osFlags |= O_RDWR;

        if (nFlag & CAM_FS_O_CREAT)
            osFlags |= O_CREAT;

        if (nFlag & CAM_FS_O_APPEND)
            osFlags |= O_APPEND;

        if (nFlag & CAM_FS_O_TRUNC)
            osFlags |= O_TRUNC;
    }

    open_ret = open(szPath, osFlags, nMode);
    if (open_ret >= 0)
    {
        *ptFd = (CamFsFd *)(intptr_t)open_ret;
        return CAM_FS_OK;
    }
    else
    {
        return CAM_FS_FAIL;
    }
#elif defined(CAM_OS_LINUX_KERNEL)
    struct file *ptFp = NULL;
    mm_segment_t tFs;
    int          osFlags = 0;

    if (!strlen(szPath))
    {
        CamOsPrintf(KERN_ERR "CamFsOpen with invalidate filename\n");
        return CAM_FS_FAIL;
    }

    if (nFlag == 0 || (nFlag & 0xFFFF) != (nFlag >> 16))
    {
        CamOsPrintf(KERN_ERR "CamFsOpen with invalidate flags, please use CAM_FS_O_ flag\n");
        return CAM_FS_FAIL;
    }
    else
    {
        if (nFlag & CAM_FS_O_RDONLY)
            osFlags |= O_RDONLY;

        if (nFlag & CAM_FS_O_WRONLY)
            osFlags |= O_WRONLY;

        if (nFlag & CAM_FS_O_RDWR)
            osFlags |= O_RDWR;

        if (nFlag & CAM_FS_O_CREAT)
            osFlags |= O_CREAT;

        if (nFlag & CAM_FS_O_APPEND)
            osFlags |= O_APPEND;

        if (nFlag & CAM_FS_O_TRUNC)
            osFlags |= O_TRUNC;
    }

    tFs = get_fs();
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 1, 0)
    set_fs(KERNEL_DS);
#else
    set_fs(get_ds());
#endif

    ptFp = filp_open(szPath, osFlags, nMode);
    set_fs(tFs);

    if (IS_ERR(ptFp))
    {
        *ptFd = NULL;
        return CAM_FS_FAIL;
    }
    else
    {
        *ptFd = (CamFsFd)ptFp;
        return CAM_FS_OK;
    }
#endif
}

CamFsRet_e CamFsClose(CamFsFd tFd)
{
#ifdef CAM_OS_RTK
    CamFsFdRtk_t *ptFdRtk = (CamFsFdRtk_t *)tFd;

    if (ptFdRtk == NULL)
    {
        return CAM_FS_FAIL;
    }

    switch (ptFdRtk->fmt)
    {
#if defined(CONFIG_LWFS_SUPPORT)
        case CAM_FS_FMT_LWFS:
            lwfs_close(ptFdRtk->fd);
            break;
#endif
#if defined(CONFIG_LITTLEFS_SUPPORT)
        case CAM_FS_FMT_LITTLEFS:
            littlefs_close(ptFdRtk->fd);
            break;
#endif
#if defined(CONFIG_PROXYFS_SUPPORT)
        case CAM_FS_FMT_PROXYFS:
            proxyfs_client_close(ptFdRtk->fd);
            break;
#endif
#if defined(CONFIG_FIRMWAREFS_SUPPORT)
        case CAM_FS_FMT_FIRMWAREFS:
            firmwarefs_close(ptFdRtk->fd);
            break;
#endif
        default:
            CamOsPrintf("%s: unsupported filesystem id (0x%x)\n", __FUNCTION__, (u32)ptFdRtk->fmt);
            break;
    }

    CamOsMemRelease(tFd);

    return CAM_FS_OK;
#elif defined(CAM_OS_LINUX_USER)
    if (!close((int)(intptr_t)tFd))
        return CAM_FS_OK;
    else
        return CAM_FS_FAIL;
#elif defined(CAM_OS_LINUX_KERNEL)
    struct file *ptFp = (struct file *)tFd;

    if (ptFp)
    {
        return (!filp_close(ptFp, NULL)) ? CAM_FS_OK : CAM_FS_FAIL;
    }
    else
    {
        return CAM_FS_FAIL;
    }
#endif
}

s32 CamFsRead(CamFsFd tFd, void *pBuf, u32 nCount)
{
#ifdef CAM_OS_RTK
    CamFsFdRtk_t *ptFdRtk = (CamFsFdRtk_t *)tFd;
    s32           nRet    = 0;

    if (ptFdRtk == NULL)
    {
        return nRet;
    }

    switch (ptFdRtk->fmt)
    {
#if defined(CONFIG_LWFS_SUPPORT)
        case CAM_FS_FMT_LWFS:
            nRet = lwfs_read(ptFdRtk->fd, (void *)pBuf, nCount);
            break;
#endif
#if defined(CONFIG_LITTLEFS_SUPPORT)
        case CAM_FS_FMT_LITTLEFS:
            nRet = littlefs_read(ptFdRtk->fd, (void *)pBuf, nCount);
            break;
#endif
#if defined(CONFIG_PROXYFS_SUPPORT)
        case CAM_FS_FMT_PROXYFS:
            nRet = proxyfs_client_read(ptFdRtk->fd, (void *)pBuf, nCount);
            break;
#endif
#if defined(CONFIG_FIRMWAREFS_SUPPORT)
        case CAM_FS_FMT_FIRMWAREFS:
            nRet = firmwarefs_read(ptFdRtk->fd, (void *)pBuf, nCount);
            break;
#endif
        default:
            CamOsPrintf("%s: unsupported filesystem id (0x%x)\n", __FUNCTION__, (u32)ptFdRtk->fmt);
            nRet = 0;
            break;
    }

    return nRet;
#elif defined(CAM_OS_LINUX_USER)
    return read((int)(intptr_t)tFd, pBuf, nCount);
#elif defined(CAM_OS_LINUX_KERNEL)
    struct file *ptFp = (struct file *)tFd;
    mm_segment_t tFs;
    loff_t       tPos;
    s32          nRet;

    if (ptFp)
    {
        tFs = get_fs();
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 1, 0)
        set_fs(KERNEL_DS);
#else
        set_fs(get_ds());
#endif
        tPos        = ptFp->f_pos;
        nRet        = kernel_read(ptFp, pBuf, nCount, &tPos);
        ptFp->f_pos = tPos;
        set_fs(tFs);
        return nRet;
    }
    else
    {
        return -1;
    }
#endif
}

s32 CamFsWrite(CamFsFd tFd, const void *pBuf, u32 nCount)
{
#ifdef CAM_OS_RTK
    CamFsFdRtk_t *ptFdRtk = (CamFsFdRtk_t *)tFd;
    s32           nRet    = 0;

    if (ptFdRtk == NULL)
    {
        return nRet;
    }

    switch (ptFdRtk->fmt)
    {
#if defined(CONFIG_LWFS_SUPPORT)
        case CAM_FS_FMT_LWFS:
            nRet = lwfs_write(ptFdRtk->fd, (void *)pBuf, nCount);
            break;
#endif
#if defined(CONFIG_LITTLEFS_SUPPORT)
        case CAM_FS_FMT_LITTLEFS:
            nRet = littlefs_write(ptFdRtk->fd, (void *)pBuf, nCount);
            break;
#endif
#if defined(CONFIG_PROXYFS_SUPPORT)
        case CAM_FS_FMT_PROXYFS:
            nRet = proxyfs_client_write(ptFdRtk->fd, (void *)pBuf, nCount);
            break;
#endif
#if defined(CONFIG_FIRMWAREFS_SUPPORT)
        case CAM_FS_FMT_FIRMWAREFS:
            nRet = firmwarefs_write(ptFdRtk->fd, (void *)pBuf, nCount);
            break;
#endif
        default:
            CamOsPrintf("%s: unsupported filesystem id (0x%x)\n", __FUNCTION__, (u32)ptFdRtk->fmt);
            nRet = 0;
            break;
    }

    return nRet;
#elif defined(CAM_OS_LINUX_USER)
    return write((int)(intptr_t)tFd, pBuf, nCount);
#elif defined(CAM_OS_LINUX_KERNEL)
    struct file *ptFp = (struct file *)tFd;
    mm_segment_t tFs;
    loff_t       tPos;
    s32          nRet;

    if (ptFp)
    {
        tFs = get_fs();
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 1, 0)
        set_fs(KERNEL_DS);
#else
        set_fs(get_ds());
#endif
        tPos        = ptFp->f_pos;
        nRet        = kernel_write(ptFp, pBuf, nCount, &tPos);
        ptFp->f_pos = tPos;
        set_fs(tFs);
        return nRet;
    }
    else
    {
        return -1;
    }
#endif
}

s32 CamFsSeek(CamFsFd tFd, u32 nOffset, u32 nWhence)
{
#ifdef CAM_OS_RTK
    int           osWhence = 0;
    CamFsFdRtk_t *ptFdRtk  = (CamFsFdRtk_t *)tFd;
    s32           nRet     = 0;

    if (!((nWhence == CAM_FS_SEEK_SET) || (nWhence == CAM_FS_SEEK_CUR) || (nWhence == CAM_FS_SEEK_END)))
    {
        CamOsPrintf(KERN_ERR "CamFsSeek with invalidate nWhence, please use CAM_FS_SEEK_ flag\n");
        return CAM_FS_FAIL;
    }

    if (ptFdRtk == NULL)
    {
        return nRet;
    }

    switch (ptFdRtk->fmt)
    {
#if defined(CONFIG_LWFS_SUPPORT)
        case CAM_FS_FMT_LWFS:
            if (nWhence == CAM_FS_SEEK_SET)
                osWhence = SEEK_SET;
            else if (nWhence == CAM_FS_SEEK_CUR)
                osWhence = SEEK_CUR;
            else if (nWhence == CAM_FS_SEEK_END)
                osWhence = SEEK_END;

            nRet = lwfs_lseek(ptFdRtk->fd, nOffset, osWhence);
            break;
#endif
#if defined(CONFIG_LITTLEFS_SUPPORT)
        case CAM_FS_FMT_LITTLEFS:
            if (nWhence == CAM_FS_SEEK_SET)
                osWhence = SEEK_SET;
            else if (nWhence == CAM_FS_SEEK_CUR)
                osWhence = SEEK_CUR;
            else if (nWhence == CAM_FS_SEEK_END)
                osWhence = SEEK_END;

            nRet = littlefs_lseek(ptFdRtk->fd, nOffset, osWhence);
            break;
#endif
#if defined(CONFIG_PROXYFS_SUPPORT)
        case CAM_FS_FMT_PROXYFS:
            if (nWhence == CAM_FS_SEEK_SET)
                osWhence = PROXY_FS_SEEK_SET;
            else if (nWhence == CAM_FS_SEEK_CUR)
                osWhence = PROXY_FS_SEEK_CUR;
            else if (nWhence == CAM_FS_SEEK_END)
                osWhence = PROXY_FS_SEEK_END;

            nRet = proxyfs_client_lseek(ptFdRtk->fd, nOffset, osWhence);
            break;
#endif
#if defined(CONFIG_FIRMWAREFS_SUPPORT)
        case CAM_FS_FMT_FIRMWAREFS:
            if (nWhence == CAM_FS_SEEK_SET)
                osWhence = SEEK_SET;
            else if (nWhence == CAM_FS_SEEK_CUR)
                osWhence = SEEK_CUR;
            else if (nWhence == CAM_FS_SEEK_END)
                osWhence = SEEK_END;

            nRet = firmwarefs_lseek(ptFdRtk->fd, nOffset, osWhence);
            break;
#endif
        default:
            CamOsPrintf("%s: unsupported filesystem id (0x%x)\n", __FUNCTION__, (u32)ptFdRtk->fmt);
            osWhence = osWhence; // avoid "set but not used" warning
            nRet     = -1;
            break;
    }

    return nRet;
#elif defined(CAM_OS_LINUX_USER)
    int osWhence = 0;

    if (nWhence == CAM_FS_SEEK_SET)
    {
        osWhence = SEEK_SET;
    }
    else if (nWhence == CAM_FS_SEEK_CUR)
    {
        osWhence = SEEK_CUR;
    }
    else if (nWhence == CAM_FS_SEEK_END)
    {
        osWhence = SEEK_END;
    }
    else
    {
        CamOsPrintf("CamFsSeek with invalidate nWhence, please use CAM_FS_SEEK_ flag\n");
        return CAM_FS_FAIL;
    }

    return lseek((int)(intptr_t)tFd, nOffset, osWhence);
#elif defined(CAM_OS_LINUX_KERNEL)
    struct file *ptFp     = (struct file *)tFd;
    int          osWhence = 0;

    if (nWhence == CAM_FS_SEEK_SET)
    {
        osWhence = SEEK_SET;
    }
    else if (nWhence == CAM_FS_SEEK_CUR)
    {
        osWhence = SEEK_CUR;
    }
    else if (nWhence == CAM_FS_SEEK_END)
    {
        osWhence = SEEK_END;
    }
    else
    {
        CamOsPrintf(KERN_ERR "CamFsSeek with invalidate nWhence, please use CAM_FS_SEEK_ flag\n");
        return CAM_FS_FAIL;
    }

    return vfs_llseek(ptFp, nOffset, osWhence);
#endif
}

#ifdef CAM_OS_RTK
static void CamFsInit(void)
{
    CamOsMutexInit(&gMntPointListLock);
}
rtos_subsys_initcall(CamFsInit)
#endif
