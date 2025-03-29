/*
 * proxyfs_server.c- Sigmastar
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
#include <linux/kernel.h>
#include <linux/vmalloc.h>
#include <linux/of.h>
#include <linux/device.h>
#include <linux/module.h>
#include <linux/version.h>
#include <linux/fs.h>
#include <asm/uaccess.h>
#include <asm/io.h>
#include "ms_platform.h"
#include "cam_inter_os.h"
#include "cam_os_wrapper.h"

#define PROXYFS_PATHNAME_MAX 256

#define INTEROS_SC_R2L_PROXYFS_OPEN  0xF1010000
#define INTEROS_SC_R2L_PROXYFS_CLOSE 0xF1010001
#define INTEROS_SC_R2L_PROXYFS_READ  0xF1010002
#define INTEROS_SC_R2L_PROXYFS_WRITE 0xF1010003
#define INTEROS_SC_R2L_PROXYFS_LSEEK 0xF1010004

// flags defined same as CAM_FS_WRAPPER
#define PROXY_FS_O_RDONLY 0x00010001
#define PROXY_FS_O_WRONLY 0x00020002
#define PROXY_FS_O_RDWR   0x00040004
#define PROXY_FS_O_CREAT  0x00080008
#define PROXY_FS_O_TRUNC  0x02000200
#define PROXY_FS_O_APPEND 0x04000400

#define PROXY_FS_SEEK_SET 0xFF000000 /* seek relative to beginning of file */
#define PROXY_FS_SEEK_CUR 0xFF000001 /* seek relative to current file position */
#define PROXY_FS_SEEK_END 0xFF000002 /* seek relative to end of file */

typedef struct
{
    u32 u32Arg0;
    u32 u32Arg1;
    u32 u32Arg2;
    u32 u32Ret;
} __attribute__((packed, aligned(ARCH_DMA_MINALIGN))) ProxyFSArgs_t;

static u32 proxyfs_server_open(u32 arg0, u32 arg1, u32 arg2, u32 arg3)
{
    struct file *  ptFp = NULL;
    mm_segment_t   tFs;
    char *         pathname;
    int            flags  = 0;
    ProxyFSArgs_t *ptArgs = NULL;

    if (arg2 != sizeof(ProxyFSArgs_t))
    {
        printk("ProxyFs(S): struct size not match\n");
        return -1;
    }

    ptArgs = (ProxyFSArgs_t *)ioremap_cache(arg1, arg2);
    if (!ptArgs)
    {
        printk("ProxyFs(S): map args struct fail\n");
        return -1;
    }

    CamOsMemInvalidate((void *)ptArgs, arg2);

    pathname = (char *)ioremap_cache(ptArgs->u32Arg0, PROXYFS_PATHNAME_MAX);
    if (!pathname)
    {
        printk("ProxyFs(S): map pathname fail\n");
        return -1;
    }

    CamOsMemInvalidate((void *)pathname, PROXYFS_PATHNAME_MAX);

    tFs = get_fs();
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 1, 0)
    set_fs(KERNEL_DS);
#else
    set_fs(get_ds());
#endif

    if (ptArgs->u32Arg1 & PROXY_FS_O_RDONLY)
        flags |= O_RDONLY;
    if (ptArgs->u32Arg1 & PROXY_FS_O_WRONLY)
        flags |= O_WRONLY;
    if (ptArgs->u32Arg1 & PROXY_FS_O_RDWR)
        flags |= O_RDWR;
    if (ptArgs->u32Arg1 & PROXY_FS_O_CREAT)
        flags |= O_CREAT;
    if (ptArgs->u32Arg1 & PROXY_FS_O_APPEND)
        flags |= O_APPEND;
    if (ptArgs->u32Arg1 & PROXY_FS_O_TRUNC)
        flags |= O_TRUNC;

    ptFp = filp_open(pathname, flags, ptArgs->u32Arg2);
    set_fs(tFs);

    iounmap(pathname);

    ptArgs->u32Ret = IS_ERR(ptFp) ? 0 : (u32)ptFp;
    CamOsMemFlush(ptArgs, arg2);
    iounmap(ptArgs);

    return 0;
}

static u32 proxyfs_server_close(u32 arg0, u32 arg1, u32 arg2, u32 arg3)
{
    if (arg1)
        return (u32)filp_close((struct file *)arg1, NULL);
    else
        return -1;
}

static u32 proxyfs_server_read(u32 arg0, u32 arg1, u32 arg2, u32 arg3)
{
    struct file *ptFp = (struct file *)arg1;
    mm_segment_t tFs;
    loff_t       tPos;
    s32          nRet;
    void *       read_buf = NULL;

    if (!arg1)
    {
        return -1;
    }

    read_buf = (char *)ioremap_cache(arg2, arg3);
    if (!read_buf)
    {
        printk("ProxyFs(S): map read buf fail\n");
        return -1;
    }

    tFs = get_fs();
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 1, 0)
    set_fs(KERNEL_DS);
#else
    set_fs(get_ds());
#endif
    tPos        = ptFp->f_pos;
    nRet        = vfs_read(ptFp, (void *)read_buf, arg3, &tPos);
    ptFp->f_pos = tPos;
    set_fs(tFs);

    CamOsMemFlush((void *)read_buf, CAM_OS_ALIGN_UP(arg3, cache_line_size()));
    iounmap(read_buf);

    return nRet;
}

static u32 proxyfs_server_write(u32 arg0, u32 arg1, u32 arg2, u32 arg3)
{
    struct file *ptFp = (struct file *)arg1;
    mm_segment_t tFs;
    loff_t       tPos;
    s32          nRet;
    void *       write_buf = NULL;

    if (!arg1)
    {
        return -1;
    }

    write_buf = (char *)ioremap_cache(arg2, arg3);
    if (!write_buf)
    {
        printk("ProxyFs(S): map write buf fail\n");
        return -1;
    }
    CamOsMemInvalidate((void *)write_buf, CAM_OS_ALIGN_UP(arg3, cache_line_size()));

    tFs = get_fs();
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 1, 0)
    set_fs(KERNEL_DS);
#else
    set_fs(get_ds());
#endif
    tPos        = ptFp->f_pos;
    nRet        = vfs_write(ptFp, (void *)write_buf, arg3, &tPos);
    ptFp->f_pos = tPos;
    set_fs(tFs);

    iounmap(write_buf);

    return nRet;
}

static u32 proxyfs_server_lseek(u32 arg0, u32 arg1, u32 arg2, u32 arg3)
{
    int whence = 0;

    if (arg1)
    {
        if (arg3 == PROXY_FS_SEEK_SET)
            whence = SEEK_SET;
        else if (arg3 == PROXY_FS_SEEK_CUR)
            whence = SEEK_CUR;
        else if (arg3 == PROXY_FS_SEEK_END)
            whence = SEEK_END;

        return vfs_llseek((struct file *)arg1, arg2, whence);
    }
    else
    {
        return -1;
    }
}

int proxyfs_server_init(void)
{
    s32 reg_ret = 0;

    reg_ret |= CamInterOsSignalReg(INTEROS_SC_R2L_PROXYFS_OPEN, proxyfs_server_open, "proxyfs_open");
    reg_ret |= CamInterOsSignalReg(INTEROS_SC_R2L_PROXYFS_CLOSE, proxyfs_server_close, "proxyfs_close");
    reg_ret |= CamInterOsSignalReg(INTEROS_SC_R2L_PROXYFS_READ, proxyfs_server_read, "proxyfs_read");
    reg_ret |= CamInterOsSignalReg(INTEROS_SC_R2L_PROXYFS_WRITE, proxyfs_server_write, "proxyfs_write");
    reg_ret |= CamInterOsSignalReg(INTEROS_SC_R2L_PROXYFS_LSEEK, proxyfs_server_lseek, "proxyfs_lseek");

    if (reg_ret != 0)
    {
        printk("ProxyFs(S): register signal callback fail\n");
    }

    return reg_ret;
}

late_initcall(proxyfs_server_init);
