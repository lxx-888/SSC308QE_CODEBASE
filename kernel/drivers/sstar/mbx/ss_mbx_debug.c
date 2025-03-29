/*
 * ss_mbx_debug.c - Sigmastar
 *
 * Copyright (c) [2019~2022] SigmaStar Technology.
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

#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/mm.h>
#include "mbx_protocol.h"
#include "ms_msys.h"
#include "cam_os_wrapper.h"
#include <linux/slab.h>
#include "ss_mbx_impl.h"
#include "mbx_platform.h"
#include "hal_bdma.h"
#include "ss_mbx_debug.h"
#include "MsTypes.h"

#define PM_RTOS_ENTRY_ROOT    "pm_rtos"
#define PM_RTOS_ENTRY_NORMAL  "pm_rtos/log"
#define PM_RTOS_ENTRY_CRASH   "pm_rtos/crash_log"
#define PM_RTOS_STATUE        (RIU_RD_2BYTE(MBX_REG_BASE, MBX_REG_PM_EXCEPTION_ADDR))
#define PM_INFO_VERSION       0x0200
#define PM_INFO_HEAD_MAGICNUM 0x3c5a3c5a
#define PM_INFO_TAIL_MAGICNUM 0x3a5c3a5c

#define PM_RTOS_BUFSIZE_SIZE(size) (size & 0x7fffffff)
#define PM_RTOS_BUFSIZE_TYPE(size) (size >> 31)
#define ALIGN_UP(x, align)         (((x) + ((align)-1)) & ~((align)-1))

// reg addr = 0xfd000000 + bank*200 + offset*4
#define RIU_RD_2BYTE(base, offset)      (*(volatile unsigned short *)(base + offset))
#define RIU_WR_2BYTE(base, offset, val) *(volatile unsigned short *)(base + offset) = (val)
#define RIU_WR_2BYTE_MASK(base, offset, val, mask) \
    RIU_WR_2BYTE(base, (offset), ((RIU_RD_2BYTE(base, offset) & ~(mask)) | ((val) & (mask))));

// The total size of this structure must be kept 64 bytes with reserved bytes
typedef struct
{
    MS_U32 u32MaggicHead; // NUST be 0x3c5a3c5a

    MS_U32 u32VersionId;

    MS_U16 u32Size;
    MS_U8  u8HasDead;
    MS_U8  u8Resv0;

    // Bit 31 as 0 is for IMI, 1 is for PSRAM, the other lower bits are buffer's length
    MS_U32 u32NormalLogBufTypeSize;

    MS_U32 u32NormalLogBufAddr;

    MS_U16 u16ReadPoint;
    MS_U16 u16WritePoint;

    // Bit 31 as 0 is for IMI, 1 is for PSRAM, the other lower bits are buffer's length
    MS_U32 u32CrashLogBufTypeSize;

    MS_U32 u32CrashLogBufAddr;

    MS_U32 u32CrashLogRealSize;

    // reserved bytes for extension
    MS_U8 u8Reserved[24];

    MS_U32 u32MaggicTail; // NUST be 0x3a5c3a5c
} MBX_PM_Info;

typedef struct mbx_dbg_info_s
{
    MBX_PM_Info *  pstPmInfo;
    CamOsMutex_t   lock;
    void *         pBufVirtAddr;
    ss_phys_addr_t u64BufPhyAddr;
    u32            u32BufSize;
} mbx_dbg_info_t;

extern int msys_dma_copy_general(u8 u8DmaCh, int path_sel, MSYS_DMA_COPY *cfg);

static struct proc_dir_entry *g_pstPmRtosEntry, *g_pstPmRtosEntryNormal, *g_pstPmRtosEntryCrash;
static mbx_dbg_info_t         g_MbxDbgInfo;

static int SS_Mbx_PmDbg_ReadInfo(MBX_PM_Info *pstMbxPmInfo)
{
    u16           u16MbxPmInfoSize;
    u32           u32MbxPmInfoSrcAddr;
    MSYS_DMA_COPY stDmaCpy;
    s32           ret;

    // get MbxPminfo address in pmrtos
    u16MbxPmInfoSize    = RIU_RD_2BYTE(MBX_REG_BASE, MBX_REG_PM_BUFSIZE_ADDR) & MBX_REG_PM_BUFSIZE_MASK;
    u32MbxPmInfoSrcAddr = (RIU_RD_2BYTE(MBX_REG_BASE, MBX_REG_PM_BUFADDR_H_ADDR) << 16)
                          | (RIU_RD_2BYTE(MBX_REG_BASE, MBX_REG_PM_BUFADDR_L_ADDR));

    if (u16MbxPmInfoSize != sizeof(MBX_PM_Info))
    {
        pr_err("error, PmInfoSize :%u < %u\n", u16MbxPmInfoSize, sizeof(MBX_PM_Info));
        return -1;
    }

    // trigger bdma to copy pminfo
    stDmaCpy.length      = u16MbxPmInfoSize;
    stDmaCpy.phyaddr_src = u32MbxPmInfoSrcAddr;
    stDmaCpy.phyaddr_dst = CamOsMemPhysToMiu(CamOsMemVirtToPhys(pstMbxPmInfo));

    // bit15 is memory type
    if (RIU_RD_2BYTE(MBX_REG_BASE, MBX_REG_PM_BUFSIZE_ADDR) >> 15)
    {
        ret = msys_dma_copy_from_psram(&stDmaCpy);
    }
    else
    {
        ret = msys_dma_copy_from_pmimi(&stDmaCpy);
    }
    if (ret != 0)
    {
        pr_err("bdma cpy normal log fail\n");
        return -1;
    }

    // must invalid the struct to make sure cpu read data from dram but not cache
    CamOsMemInvalidate(pstMbxPmInfo, u16MbxPmInfoSize);

    if (pstMbxPmInfo->u32MaggicHead != PM_INFO_HEAD_MAGICNUM || pstMbxPmInfo->u32MaggicTail != PM_INFO_TAIL_MAGICNUM)
    {
        pr_err("BUG!pminfo magic check fail 0x%x,0x%x\n", pstMbxPmInfo->u32MaggicHead, pstMbxPmInfo->u32MaggicTail);
        return -1;
    }

    if (pstMbxPmInfo->u32VersionId != PM_INFO_VERSION)
    {
        pr_err("pminfo version check fail rtos :0x%x != linux:0x%x\n", pstMbxPmInfo->u32VersionId, PM_INFO_VERSION);
        return -1;
    }

    pr_info("normal buf address = 0x%x, size = 0x%x, type:%u\n crash buf address = 0x%x, size = 0x%x,type:%u\n",
            pstMbxPmInfo->u32NormalLogBufAddr, PM_RTOS_BUFSIZE_SIZE(pstMbxPmInfo->u32NormalLogBufTypeSize),
            PM_RTOS_BUFSIZE_TYPE(pstMbxPmInfo->u32NormalLogBufTypeSize), pstMbxPmInfo->u32CrashLogBufAddr,
            PM_RTOS_BUFSIZE_SIZE(pstMbxPmInfo->u32CrashLogBufTypeSize),
            PM_RTOS_BUFSIZE_TYPE(pstMbxPmInfo->u32CrashLogBufTypeSize));

    return 0;
}

// alloc new buf for seq_file->buf to show on terminal
static void *SS_Mbx_PmDbg_AllocSeqFileBuf(struct seq_file *m, u32 u32Size)
{
    void *new_buf;

    // new_buf will be free in seq_release()
    new_buf = CamOsMemCalloc(sizeof(char), u32Size);
    if (!new_buf)
    {
        return NULL;
    }

    // replace seq_file->buf
    if (m->buf)
    {
        kvfree(m->buf);
    }
    m->buf   = new_buf;
    m->size  = u32Size;
    m->count = 0;

    return new_buf;
}

// call this when we are sure that pmrtos is crash
static void SS_Mbx_PmDbg_DumpCrash(struct seq_file *m)
{
    void *        output = NULL;
    MSYS_DMA_COPY stDmaCpy;
    u16           write, read;
    s32           ret;
    u32           u32NormalSize, u32CrashSize, u32Total;

    // must read pminfo here as dump normal log may cause pmrtos crash
    if (SS_Mbx_PmDbg_ReadInfo(g_MbxDbgInfo.pstPmInfo) != 0)
    {
        pr_err("get mbx pm info fail\n");
        return;
    }

    u32NormalSize = PM_RTOS_BUFSIZE_SIZE(g_MbxDbgInfo.pstPmInfo->u32NormalLogBufTypeSize);
    u32CrashSize  = PM_RTOS_BUFSIZE_SIZE(g_MbxDbgInfo.pstPmInfo->u32CrashLogBufTypeSize);

    // 0. alloc 64 + u32NormalSize + u32CrashSize buf to store crash log, 64 byte is for tips log
    u32Total = 64 + u32NormalSize + u32CrashSize;
    output   = SS_Mbx_PmDbg_AllocSeqFileBuf(m, u32Total);
    if (!output)
    {
        pr_err("GetSeqFileBuf fail %d\n", u32Total);
        return;
    }

    // 1. copy normal log
    stDmaCpy.length      = u32NormalSize;
    stDmaCpy.phyaddr_src = g_MbxDbgInfo.pstPmInfo->u32NormalLogBufAddr;
    stDmaCpy.phyaddr_dst = CamOsMemPhysToMiu(g_MbxDbgInfo.u64BufPhyAddr);

    if (PM_RTOS_BUFSIZE_TYPE(g_MbxDbgInfo.pstPmInfo->u32NormalLogBufTypeSize))
    {
        ret = msys_dma_copy_from_psram(&stDmaCpy);
    }
    else
    {
        ret = msys_dma_copy_from_pmimi(&stDmaCpy);
    }

    if (ret != 0)
    {
        pr_err("bdma cpy normal log fail ret = %d\n", ret);
        return;
    }

    m->count += sprintf(output, "normal log:\n");

    // read/write pointer contain log
    read  = g_MbxDbgInfo.pstPmInfo->u16ReadPoint;
    write = g_MbxDbgInfo.pstPmInfo->u16WritePoint;

    if (read < write)
    {
        CamOsMemcpy(output + m->count, g_MbxDbgInfo.pBufVirtAddr + read, write - read + 1);
        m->count += write - read + 1;
    }
    else
    {
        // eg:<789|456> first copy 456,then copy 789
        // copy read to end of buf
        CamOsMemcpy(output + m->count, g_MbxDbgInfo.pBufVirtAddr + read, u32NormalSize - read);
        m->count += u32NormalSize - read;
        //  copy start to write
        CamOsMemcpy(output + m->count, g_MbxDbgInfo.pBufVirtAddr, write + 1);
        m->count += write + 1;
    }

    // 2. copy crash log
    stDmaCpy.length      = u32CrashSize;
    stDmaCpy.phyaddr_src = g_MbxDbgInfo.pstPmInfo->u32CrashLogBufAddr;
    stDmaCpy.phyaddr_dst = CamOsMemPhysToMiu(CamOsMemVirtToPhys(g_MbxDbgInfo.pBufVirtAddr));

    if (PM_RTOS_BUFSIZE_TYPE(g_MbxDbgInfo.pstPmInfo->u32CrashLogBufTypeSize))
    {
        ret = msys_dma_copy_from_psram(&stDmaCpy);
    }
    else
    {
        ret = msys_dma_copy_from_pmimi(&stDmaCpy);
    }
    if (ret != 0)
    {
        pr_err("bdma cpy normal log fail\n");
        return;
    }

    m->count += sprintf(output + m->count, "crash log:\n");
    CamOsMemcpy(output + m->count, g_MbxDbgInfo.pBufVirtAddr, g_MbxDbgInfo.pstPmInfo->u32CrashLogRealSize);
    m->count += g_MbxDbgInfo.pstPmInfo->u32CrashLogRealSize;
}

static void SS_Mbx_PmDbg_DumpNormal(struct seq_file *m)
{
    SS_Mbx_Msg_t stMbxMsg;
    int          ret = E_SS_MBX_RET_OK;
    char *       output;
    u16          size;
    u32          u32BufDramAddr = CamOsMemPhysToMiu(g_MbxDbgInfo.u64BufPhyAddr);

    // send dump normal log request
    CamOsMemset(&stMbxMsg, 0x0, sizeof(stMbxMsg));
    stMbxMsg.u8MsgClass       = E_MBX_CLASS_DUMP_LOG;
    stMbxMsg.u8ParameterCount = 3;
    stMbxMsg.u16Parameters[0] = u32BufDramAddr >> 16;
    stMbxMsg.u16Parameters[1] = u32BufDramAddr & 0xffff;
    stMbxMsg.u16Parameters[2] = g_MbxDbgInfo.u32BufSize;
    stMbxMsg.eDirect          = E_SS_MBX_DIRECT_ARM_TO_CM4;

    ret = SS_Mailbox_IMPL_SendMsg(&stMbxMsg);
    if (ret != E_SS_MBX_RET_OK)
    {
        pr_err("send mbx msg fail %d\n", ret);
        return SS_Mbx_PmDbg_DumpCrash(m);
    }

    CamOsMemset(&stMbxMsg, 0x0, sizeof(stMbxMsg));

    ret = SS_Mailbox_IMPL_RecvMsg(E_MBX_CLASS_DUMP_LOG, &stMbxMsg, 1000);
    if (ret != E_SS_MBX_RET_OK)
    {
        pr_err("recv mbx msg fail %d\n", ret);
        return SS_Mbx_PmDbg_DumpCrash(m);
    }

    if (stMbxMsg.u8ParameterCount != 1)
    {
        pr_err("msg para cnt error %d != %d", stMbxMsg.u8ParameterCount, 1);
        return;
    }

    size = stMbxMsg.u16Parameters[0];
    if (0 == size)
    {
        pr_err("pm rtos has no new log\n");
        return;
    }

    // alloc size+1 to avoid overflow
    output = SS_Mbx_PmDbg_AllocSeqFileBuf(m, size + 1);
    if (!output)
    {
        pr_err("GetSeqFileBuf fail %d", size + 1);
        return;
    }

    CamOsMemcpy(output, g_MbxDbgInfo.pBufVirtAddr, size);
    m->count = size;
}

static int SS_Mbx_PmDbg_ShowNormal(struct seq_file *m, void *v)
{
    CamOsMutexLock(&g_MbxDbgInfo.lock);

    // 1. check pm rtos status first
    if (SS_Mbx_PmDbg_ReadInfo(g_MbxDbgInfo.pstPmInfo) != 0)
    {
        CamOsMutexUnlock(&g_MbxDbgInfo.lock);
        pr_err("get mbx pm info fail\n");
        return 0;
    }

    // 2. dump log according to status
    if (PM_RTOS_STATUE != 0 || g_MbxDbgInfo.pstPmInfo->u8HasDead)
    {
        // pm rtos is crash
        SS_Mbx_PmDbg_DumpCrash(m);
    }
    else
    {
        SS_Mbx_PmDbg_DumpNormal(m);
    }

    CamOsMutexUnlock(&g_MbxDbgInfo.lock);

    return 0;
}

static int SS_Mbx_PmDbg_ShowCrash(struct seq_file *m, void *v)
{
    CamOsMutexLock(&g_MbxDbgInfo.lock);

    SS_Mbx_PmDbg_DumpCrash(m);

    CamOsMutexUnlock(&g_MbxDbgInfo.lock);

    return 0;
}

static int SS_Mbx_PmDbg_OpenNormal(struct inode *inode, struct file *file)
{
    return single_open(file, SS_Mbx_PmDbg_ShowNormal, NULL);
}

static int SS_Mbx_PmDbg_OpenCrash(struct inode *inode, struct file *file)
{
    return single_open(file, SS_Mbx_PmDbg_ShowCrash, NULL);
}

static const struct proc_ops PmRtos_NormalProc_operations = {
    .proc_open    = SS_Mbx_PmDbg_OpenNormal,
    .proc_read    = seq_read,
    .proc_lseek   = seq_lseek,
    .proc_release = seq_release,
};

static const struct proc_ops PmRtos_CrashProc_operations = {
    .proc_open    = SS_Mbx_PmDbg_OpenCrash,
    .proc_read    = seq_read,
    .proc_lseek   = seq_lseek,
    .proc_release = seq_release,
};

int SS_Mailbox_Debug_Init(void)
{
    int ret;
    u32 max_size;

    ret = SS_Mailbox_IMPL_Init();
    if (ret != E_SS_MBX_RET_OK)
    {
        return ret;
    }
    ret = SS_Mailbox_IMPL_Enable(E_MBX_CLASS_DUMP_LOG);
    if (ret != E_SS_MBX_RET_OK)
    {
        SS_Mailbox_IMPL_Deinit();
        return ret;
    }

    g_pstPmRtosEntry = proc_mkdir(PM_RTOS_ENTRY_ROOT, NULL);
    if (!g_pstPmRtosEntry)
    {
        pr_err("create /proc/%s fail\n", PM_RTOS_ENTRY_ROOT);
        goto FAIL;
    }
    g_pstPmRtosEntryNormal = proc_create(PM_RTOS_ENTRY_NORMAL, 0x0444, NULL, &PmRtos_NormalProc_operations);
    if (!g_pstPmRtosEntryNormal)
    {
        pr_err("create /proc/%s fail\n", PM_RTOS_ENTRY_NORMAL);
        goto FAIL;
    }

    g_pstPmRtosEntryCrash = proc_create(PM_RTOS_ENTRY_CRASH, 0x0444, NULL, &PmRtos_CrashProc_operations);
    if (!g_pstPmRtosEntryNormal)
    {
        pr_err("create /proc/%s fail\n", PM_RTOS_ENTRY_CRASH);
        goto FAIL;
    }

    // use dynamic alloc to make sure MBX_PM_Info is 64 byte aligned as we need to fill MBX_PM_Info by BDMA
    g_MbxDbgInfo.pstPmInfo = CamOsMemAlloc(sizeof(MBX_PM_Info));
    if (!g_MbxDbgInfo.pstPmInfo)
    {
        pr_err("fail to alloc pminfo\n");
        goto FAIL;
    }
    CamOsMemset(g_MbxDbgInfo.pstPmInfo, 0, sizeof(MBX_PM_Info));
    CamOsMemFlush(g_MbxDbgInfo.pstPmInfo, sizeof(MBX_PM_Info));

    if (SS_Mbx_PmDbg_ReadInfo(g_MbxDbgInfo.pstPmInfo) != 0)
    {
        pr_err("get mbx pm info fail,dump log disable\n");
        goto FAIL;
    }

    max_size = PM_RTOS_BUFSIZE_SIZE(g_MbxDbgInfo.pstPmInfo->u32NormalLogBufTypeSize)
                       > PM_RTOS_BUFSIZE_SIZE(g_MbxDbgInfo.pstPmInfo->u32CrashLogBufTypeSize)
                   ? PM_RTOS_BUFSIZE_SIZE(g_MbxDbgInfo.pstPmInfo->u32NormalLogBufTypeSize)
                   : PM_RTOS_BUFSIZE_SIZE(g_MbxDbgInfo.pstPmInfo->u32CrashLogBufTypeSize);

    if (max_size <= 0)
    {
        pr_err("pmrtos not use log buf mode\n");
        goto FAIL;
    }
    // max_size align up to 64byte to suit for bdma transfer
    max_size = ALIGN_UP(max_size, 64);

    g_MbxDbgInfo.u64BufPhyAddr = CamOsContiguousMemAlloc(max_size);
    if (NULL == g_MbxDbgInfo.u64BufPhyAddr)
    {
        pr_err("alloc log buf fail size:%u\n", max_size);
        goto FAIL;
    }
    g_MbxDbgInfo.pBufVirtAddr = CamOsMemMap(g_MbxDbgInfo.u64BufPhyAddr, max_size, FALSE);
    if (NULL == g_MbxDbgInfo.pBufVirtAddr)
    {
        pr_err("map log buf fail phy:0x%llx\n", g_MbxDbgInfo.u64BufPhyAddr);
        goto FAIL;
    }
    g_MbxDbgInfo.u32BufSize = max_size;

    CamOsMutexInit(&g_MbxDbgInfo.lock);
    pr_info("%s phyaddr = 0x%llx,size 0x%x\n", __FUNCTION__, g_MbxDbgInfo.u64BufPhyAddr, g_MbxDbgInfo.u32BufSize);

    return 0;

FAIL:
    if (g_MbxDbgInfo.pstPmInfo)
    {
        CamOsMemRelease(g_MbxDbgInfo.pstPmInfo);
        g_MbxDbgInfo.pstPmInfo = NULL;
    }
    if (g_pstPmRtosEntryCrash)
    {
        proc_remove(g_pstPmRtosEntryCrash);
        g_pstPmRtosEntryCrash = NULL;
    }
    if (g_pstPmRtosEntryNormal)
    {
        proc_remove(g_pstPmRtosEntryNormal);
        g_pstPmRtosEntryNormal = NULL;
    }
    if (g_pstPmRtosEntry)
    {
        proc_remove(g_pstPmRtosEntry);
        g_pstPmRtosEntry = NULL;
    }
    if (g_MbxDbgInfo.u64BufPhyAddr)
    {
        CamOsContiguousMemRelease(g_MbxDbgInfo.u64BufPhyAddr);
        g_MbxDbgInfo.u64BufPhyAddr = NULL;
    }

    SS_Mailbox_IMPL_Disable(E_MBX_CLASS_DUMP_LOG);
    SS_Mailbox_IMPL_Deinit();

    return -1;
}

int SS_Mailbox_Debug_Deinit(void)
{
    if (g_MbxDbgInfo.pBufVirtAddr)
    {
        CamOsMemUnmap(g_MbxDbgInfo.pBufVirtAddr, g_MbxDbgInfo.u32BufSize);
        CamOsContiguousMemRelease(g_MbxDbgInfo.u64BufPhyAddr);
        CamOsMemRelease(g_MbxDbgInfo.pstPmInfo);
        CamOsMutexDestroy(&g_MbxDbgInfo.lock);
        CamOsMemset(&g_MbxDbgInfo, 0x0, sizeof(g_MbxDbgInfo));

        // remove the proc
        if (g_pstPmRtosEntryNormal)
        {
            proc_remove(g_pstPmRtosEntryNormal);
            g_pstPmRtosEntryNormal = NULL;
        }
        if (g_pstPmRtosEntryCrash)
        {
            proc_remove(g_pstPmRtosEntryCrash);
            g_pstPmRtosEntryCrash = NULL;
        }
        if (g_pstPmRtosEntry)
        {
            proc_remove(g_pstPmRtosEntry);
            g_pstPmRtosEntry = NULL;
        }

        SS_Mailbox_IMPL_Disable(E_MBX_CLASS_DUMP_LOG);
        SS_Mailbox_IMPL_Deinit();
    }

    return 0;
}
