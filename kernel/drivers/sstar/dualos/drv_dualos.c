/*
 * drv_dualos.c- Sigmastar
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
/*
 * drv_dualos.c
 *   ipc rsq with LH/RTOS
 */
#include <linux/init.h>
#include <linux/printk.h>
#include <linux/ioport.h>
#include <linux/slab.h>
#include <linux/semaphore.h>
#include <linux/delay.h>
#include <linux/ctype.h>
#include <linux/module.h>
#include <linux/io.h>
#include <linux/platform_device.h>
#include <asm/compiler.h>
#include <asm/io.h>
#include "linux/arm-smccc.h"
#include "ms_platform.h"
#include "registers.h"
#include "drv_dualos.h"
#include "interos_call.h"
#include "cam_inter_os.h"
#include "rsq.h"
#include "sw_sem.h"
#include "lock.h"

/* proc */
#include <linux/fs.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/uaccess.h>
#include <linux/version.h>
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 12, 0)
#include <linux/sched/task.h>
#endif
#include <linux/list.h>

#if defined(CONFIG_SSTAR_RPMSG)
#include <linux/skbuff.h>
#include "sstar_rpmsg.h"
#include "rpmsg_dualos.h"

#define INTEROS_TIMEOUT (HZ * 5) // 5s
#define TX_TIMEOUT      (1000)   // 1s

#define INTEROS_SC_L2R_LOG_BUF_SIZE       0xF1040000
#define INTEROS_SC_L2R_LOG_BUF_GET        0xF1040001
#define INTEROS_SC_R2L_CLI_EXEC_DONE      0xF1040002
#define INTEROS_SC_L2R_CRASH_LOG_BUF_SIZE 0xF1040003
#define INTEROS_SC_L2R_CRASH_LOG_BUF_ADDR 0xF1040004

struct dualos_interos_node
{
    struct rpmsg_endpoint *ept;

    spinlock_t          queue_lock;
    struct sk_buff_head queue;

    wait_queue_head_t wq;

    int          remote_addr;
    unsigned int index;

    struct mutex mutex;
    void *private;
};

static struct dualos_interos_node *default_dnode;

struct dualos_interos_node *dualos_interos_node_alloc(int local_addr, int remote_addr);
void                        dualos_interos_node_release(struct dualos_interos_node *dnode);

static DEFINE_MUTEX(_gRtkinfoWriteLock);
struct semaphore gRtkinfoWriteSem = __SEMAPHORE_INITIALIZER(gRtkinfoWriteSem, 0);

extern void recode_timestamp(int mark, const char *name);

int dualos_interos_recv(struct dualos_interos_node *dnode, struct dualos_interos_datagram *data, unsigned int *src,
                        signed long timeout);
int dualos_interos_call_to(struct dualos_interos_node *dnode, int dst, unsigned int arg0, unsigned int arg1,
                           unsigned int arg2, unsigned int arg3, struct dualos_interos_datagram *resp);
int dualos_interos_call(struct dualos_interos_node *dnode, unsigned int arg0, unsigned int arg1, unsigned int arg2,
                        unsigned int arg3, struct dualos_interos_datagram *resp);

static int dualos_interos_cb(struct rpmsg_device *rpdev, void *data, int len, void *priv, u32 src)
{
    struct dualos_interos_node *dnode = priv;
    struct sk_buff *            skb;

    skb = alloc_skb(len + sizeof(src), GFP_ATOMIC);
    if (!skb)
        return -ENOMEM;

    memcpy(skb_put(skb, len), data, len);
    memcpy(skb_put(skb, sizeof(src)), &src, sizeof(src));

    spin_lock(&dnode->queue_lock);
    skb_queue_tail(&dnode->queue, skb);
    spin_unlock(&dnode->queue_lock);

    wake_up_all(&dnode->wq);
    return 0;
}

static int dualos_interos_sendto(struct dualos_interos_node *dnode, int dst, char *buffer, int size, int timeout)
{
    int res;

    do
    {
        res = rpmsg_sendto(dnode->ept, buffer, size, dst);
        if (!res)
            break;

        if (res && res != -ENOMEM)
        {
            pr_err(
                "dualos node (%d) rpmsg_trysendto error %d, "
                "remote_addr=%d\n",
                dnode->remote_addr, res, dst);
            goto err;
        }
        if (timeout <= 0)
            break;
        msleep(1);
    } while (timeout--);
err:
    if (timeout <= 0)
    {
        pr_err(
            "dualos node (%d) rpmsg_trysendto timeout %d, "
            "remote_addr=%d\n",
            dnode->remote_addr, res, dst);
    }
    return res;
}

int dualos_interos_call_to(struct dualos_interos_node *dnode, int dst, unsigned int arg0, unsigned int arg1,
                           unsigned int arg2, unsigned int arg3, struct dualos_interos_datagram *resp)
{
    struct dualos_interos_datagram req;
    int                            ret;

    mutex_lock(&dnode->mutex);
    memset(&req, 0, sizeof(req));
    req.header.version = SSTAR_RPMSG_DUALOS_VERSION;
    req.header.index   = dnode->index++;
    req.arg0           = arg0;
    req.arg1           = arg1;
    req.arg2           = arg2;
    req.arg3           = arg3;

    ret = dualos_interos_sendto(dnode, dst, (char *)&req, sizeof(req), TX_TIMEOUT);
    if (ret)
    {
        mutex_unlock(&dnode->mutex);
        return ret;
    }

    while (1)
    {
        ret = dualos_interos_recv(dnode, resp, NULL, INTEROS_TIMEOUT);
        if (ret == -EAGAIN)
            continue;
        else if (ret)
            goto timeout;

        if (resp->header.index != req.header.index)
        {
            pr_err(
                "dualos node (%d): index not match(%d-%d) "
                "req.arg0=0x%x,req.arg1=0x%x,req.arg2=0x%x,"
                "req.arg3=0x%x,resp.arg0=0x%x,resp.arg1=0x%x"
                "resp.arg2=0x%x,resp.arg3=0x%x\n",
                dnode->remote_addr, resp->header.index, req.header.index, req.arg0, req.arg1, req.arg2, req.arg3,
                resp->arg0, resp->arg1, resp->arg2, resp->arg3);
            continue;
        }
        break;
    }

    mutex_unlock(&dnode->mutex);
    return ret;
timeout:
    mutex_unlock(&dnode->mutex);
    pr_err(
        "dualos node (%d) interos error %d, "
        "remote_addr=%d, index=0x%x, arg0=0x%x, "
        "arg1=0x%x, arg2=0x%x, arg3=0x%x\n",
        dnode->remote_addr, ret, dst, req.header.index, req.arg0, req.arg1, req.arg2, req.arg3);
    return ret;
}

int dualos_interos_call(struct dualos_interos_node *dnode, unsigned int arg0, unsigned int arg1, unsigned int arg2,
                        unsigned int arg3, struct dualos_interos_datagram *resp)
{
    return dualos_interos_call_to(dnode, dnode->remote_addr, arg0, arg1, arg2, arg3, resp);
}

struct dualos_interos_node *dualos_interos_node_alloc(int local_addr, int remote_addr)
{
    struct dualos_interos_node *dnode;
    struct rpmsg_channel_info   info;
    long                        res;

    dnode = kzalloc(sizeof(*dnode), GFP_KERNEL);
    if (!dnode)
    {
        pr_err("dualos_interos_node_alloc: out of memory!\n");
        return (void *)-ENOMEM;
    }
    dnode->remote_addr = remote_addr;

    init_waitqueue_head(&dnode->wq);
    spin_lock_init(&dnode->queue_lock);
    skb_queue_head_init(&dnode->queue);
    mutex_init(&dnode->mutex);

    snprintf(info.name, sizeof(info.name), "MI_EPT");
    info.src = local_addr;
    info.dst = RPMSG_ADDR_ANY;
#if defined(CONFIG_ARCH_INFINITY7) && defined(CONFIG_SSTAR_CA7_VIRTIO)
    dnode->ept = dualos_rpmsg_create_ept(RPMsg_Device_CA7, 0, dualos_interos_cb, dnode, info);
#elif defined(CONFIG_SSTAR_LH_RTOS_VIRTIO)
    dnode->ept = dualos_rpmsg_create_ept(RPMsg_Device_LH_RTOS, 0, dualos_interos_cb, dnode, info);
#endif
    if (dnode->ept == NULL)
    {
        pr_err("Interos node (%d, %d) dualos_rpmsg_create_ept error\n", local_addr, remote_addr);
        res = -EINVAL;
        goto err_create_ept;
    }
    return dnode;
err_create_ept:
    kfree(dnode);
    return (void *)res;
}

void dualos_interos_node_release(struct dualos_interos_node *dnode)
{
    if (!dnode)
        return;

    if (dnode->ept)
        dualos_rpmsg_destroy_ept(dnode->ept);
    skb_queue_purge(&dnode->queue);
    kfree(dnode);
    return;
}
#endif

static int disable_rtos = 0;
#if 1
static int __init disable_rtos_func(char *str)
{
    disable_rtos = simple_strtol(str, NULL, 10);
    return 0;
}
early_param("disable_rtos", disable_rtos_func);
#else
module_param(disable_rtos, int, 0644);
MODULE_PARM_DESC(disable_rtos, "Disable RTOS IPC");
#endif

static struct file *file_open(const char *path, int flags, int rights);
static void         file_close(struct file *file);
static int          file_write(struct file *file, unsigned long long offset, unsigned char *data, unsigned int size);
static int          file_read(struct file *file, unsigned long long offset, unsigned char *data, unsigned int size);
static int          file_sync(struct file *file);
/* */
struct sstream
{
    reservoir_t *    rvr;
    struct resource *res;
    unsigned int     id;
    void *private;
};

#define SHARE_SIZE (0x1000)
static rtkinfo_t *    _rtklin  = NULL;
static struct sstream _sstr[8] = {{0}};

static struct resource *   _rtkres;
struct semaphore           _interos_call_req = __SEMAPHORE_INITIALIZER(_interos_call_req, 0);
static struct task_struct *_interos_call_pid;
#ifdef CONFIG_SS_AMP
struct semaphore _interos_call_mbox     = __SEMAPHORE_INITIALIZER(_interos_call_mbox, 1);
struct semaphore _interos_call_resp     = __SEMAPHORE_INITIALIZER(_interos_call_resp, 0);
static int       _signal_1st_using_mbox = 1;
#endif
#if ENABLE_NBLK_CALL
struct semaphore           _interos_nblk_call_req = __SEMAPHORE_INITIALIZER(_interos_nblk_call_req, 0);
static struct task_struct *_interos_nblk_call_pid;
#endif

struct rsqcb
{
    struct list_head list;
    int (*rsqproc)(void *param, void *buf, int size, slot_t *slot);
    void *private;
};

LIST_HEAD(_rsqcblst);
struct rsqcb rec_rsq;
struct rsqrec
{
    struct rsqcb cb;
    char         file[32];
    reservoir_t *rvr;
    unsigned int frms;
    struct file *fp;
    long long    off;
};

#if defined(CONFIG_LH_RTOS) && defined(CONFIG_SMP)
static reroute_smc_info_t _reroute_smc_info = {.mbox_sem = __SEMAPHORE_INITIALIZER(_reroute_smc_info.mbox_sem, 1),
                                               .resp_sem = __SEMAPHORE_INITIALIZER(_reroute_smc_info.resp_sem, 0)};
#endif

typedef struct
{
    u32 id;
    u32 (*func)(u32 arg0, u32 arg1, u32 arg2, u32 arg3);
#ifdef __ADV_HYP_PROF__
    u64  count;
    char name[SYSCALL_NAME_LEN_MAX];
#endif
} stInterosSyscallEntry;

#define SYSCALL_ENTRY_MAX 64
static stInterosSyscallEntry gInterosSyscallEntry[SYSCALL_ENTRY_MAX] = {{0}};
static CamOsMutex_t          gSyscallEntryLock;
static int                   gRtosTxLogBufSize = 0;

static int c_logshow(struct seq_file *m, void *v)
{
    void *new_buf;

    if (get_rtkinfo() && get_rtkinfo()->has_dead)
    {
        printk(KERN_ERR "Rtos has dead-1\n");
        return 0;
    }

    // Get rtos buffer size
    if (!gRtosTxLogBufSize)
    {
        gRtosTxLogBufSize = CamInterOsSignal(INTEROS_SC_L2R_LOG_BUF_SIZE, 0, 0, 0);
    }

    if ((new_buf = kzalloc(gRtosTxLogBufSize, GFP_KERNEL)))
    {
        kfree(m->buf);
        m->size = gRtosTxLogBufSize;
        m->buf  = new_buf;
        CamOsMemInvalidate(m->buf, m->size);
    }
    else
    {
        seq_printf(m, "%s: allocate continuous memory fail\n", __FUNCTION__);
        return 0;
    }

    m->count = CamInterOsSignal(INTEROS_SC_L2R_LOG_BUF_GET, (unsigned int)virt_to_phys(m->buf), 0, 0);
    CamOsMemInvalidate(m->buf, m->size);

    return 0;
}

static void *rtos_crash_log_buf_va = NULL;
static int   c_show_crashlog(struct seq_file *m, void *v)
{
    if (get_rtkinfo() && get_rtkinfo()->crash_log_buf_addr && get_rtkinfo()->crash_log_buf_size)
    {
        if (!rtos_crash_log_buf_va)
        {
            void *new_buf = kzalloc(get_rtkinfo()->crash_log_buf_size, GFP_KERNEL);
            kfree(m->buf);
            m->size = get_rtkinfo()->crash_log_buf_size;
            m->buf  = new_buf;
            CamOsMemInvalidate(m->buf, m->size);
            rtos_crash_log_buf_va =
                CamOsMemMap(get_rtkinfo()->crash_log_buf_addr, get_rtkinfo()->crash_log_buf_size, 1);
            memcpy((unsigned char *)m->buf, (unsigned char *)rtos_crash_log_buf_va, get_rtkinfo()->crash_log_buf_size);
            m->count                                 = get_rtkinfo()->crash_log_buf_size - 1;
            *((unsigned char *)m->buf + m->size - 1) = '\0';
        }
    }

    return 0;
}

static void c_stop_crashlog(struct seq_file *m, void *v)
{
    if (rtos_crash_log_buf_va)
    {
        CamOsMemUnmap(rtos_crash_log_buf_va, get_rtkinfo()->crash_log_buf_size);
        rtos_crash_log_buf_va = NULL;
    }
}

static void *c_start(struct seq_file *m, loff_t *pos)
{
    return *pos < 1 ? (void *)1 : NULL;
}

static void *c_next(struct seq_file *m, void *v, loff_t *pos)
{
    ++*pos;
    return NULL;
}

static void c_stop(struct seq_file *m, void *v) {}

/************************* RTOS Core Dump Remote START *************************/

//#define RTOS_CORE_DUMP_REMOTE_DEBUG

#ifdef RTOS_CORE_DUMP_REMOTE_DEBUG
#define RTOS_CORE_DUMP_REMOTE_PRINTF(fmt, args...) \
    printk(KERN_EMERG "[RtosCrDmpRmt][%u] %s: " fmt, geStage, __func__, ##args)
#else
#define RTOS_CORE_DUMP_REMOTE_PRINTF(fmt, args...)
#endif

#define CORE_DUMP_REMOTE_FIFO_SIZE (PAGE_SIZE)

typedef struct
{
    unsigned long stackTopOffset;
    unsigned long stackBotOffset;
    unsigned long taskIdOffset;
    unsigned long archGRegArrayOffset;
    unsigned int  r0OffInStackAndVfpRegSize;
    unsigned long vfpRefArrayOffset;
    unsigned long fpscrOffset;
} TcbMap;

typedef struct
{
    bool           isCoreDumpOtherCtxVfp;
    bool           isConfigVfpSupport;
    unsigned short taskCnt;
    unsigned int   loadNum;
    unsigned int   ctxNum;
    unsigned int   pid;
    unsigned short signalType;
    unsigned char  coreDumpLv;
    unsigned char  vfpStateInHwTask;
    unsigned long  memDumpStart;
    unsigned long  memDumpEnd;
    unsigned long  curTaskStackTop;
    unsigned long  curTaskStackBot;
    unsigned int   curTaskId;
    unsigned long  curArchGRegAddr;
    unsigned long  curArchSp;
    unsigned long  curArchLr;
    unsigned long  curArchPc;
    unsigned long  curArchCpsr;
    unsigned long  curVfpRefAddr;
    unsigned int   curFpscr;
    unsigned int   aVfpInHwRef[64];
    unsigned int   tcbSize;
    TcbMap         stTcbMap;
    unsigned long  aTcbAddr[0];
} CoreDumpRemote; // This should be sync with the one in sys_dbg.c in RTOS

typedef enum
{
    E_CORE_DUMP_STAGE_PREPARE,
    E_CORE_DUMP_STAGE_START,
    E_CORE_DUMP_STAGE_DONE,
} CORE_DUMP_STAGE;

typedef struct
{
    unsigned char *pBuf;
    unsigned int   size;
    unsigned int   offset;
} CrDmpRmtFifo;

typedef struct __attribute__((packed))
{
    u8  identMagic[4];
    u8  identClass;
    u8  identData;
    u8  identVersion;
    u8  identOsAbi;
    u8  identAbiVersion;
    u8  identPad[7];
    u16 type;
    u16 machine;
    u32 version;
    u32 entry;
    u32 programHdrOffset;
    u32 sectionHdrOffset;
    u32 flags;
    u16 hdrSize;
    u16 programHdrSize;
    u16 programHdrNum;
    u16 sectionHdrSize;
    u16 sectionHdrNum;
    u16 sectionHdrTblIdx;
} SS_CORE_ELF_HEADER_t;

typedef struct __attribute__((packed))
{
    u32 type;
    u32 offset;
    u32 va;
    u32 pa;
    u32 fileSize;
    u32 memSize;
    u32 flags;
    u32 align;
} SS_CORE_ELF_PROGRAM_t;

typedef struct __attribute__((packed))
{
    u32  nameSize;
    u32  noteSize;
    u32  type;
    char name[8];
} SS_CORE_ELF_NOTE_HEADER_t;

typedef struct __attribute__((packed))
{
    u32 signalNum;
    u32 code;
    u32 errno;
} SS_CORE_ELF_SIGINFO_t;

typedef struct __attribute__((packed))
{
    u32 reg[18]; // r0 ~ r12, sp, lr, pc, cpsr, fpscr
} SS_CORE_ELF_REGSET_ARM_t;

typedef struct __attribute__((packed))
{
    SS_CORE_ELF_SIGINFO_t    sigInfo;
    u32                      currSig;
    u32                      sigPend;
    u32                      sigHold;
    u32                      pid;
    u32                      ppid;
    u32                      pgrp;
    u32                      sid;
    u64                      utime;
    u64                      stime;
    u64                      cutime;
    u64                      cstime;
    SS_CORE_ELF_REGSET_ARM_t regset;
    u32                      fpValid;
} SS_CORE_ELF_PRSTATUS_t;

typedef struct __attribute__((packed))
{
    u32 type;
    u32 value;
} SS_CORE_ELF_AUXV_PAIR_t;

typedef struct __attribute__((packed))
{
    u64 fpregs[32];
    u32 fpscr;
} SS_CORE_ELF_FPREGSETARM_VFP_t;

struct semaphore gRtkCrDmpRmtShowSem = __SEMAPHORE_INITIALIZER(gRtkCrDmpRmtShowSem, 0);
struct semaphore gRtkCrDmpRmtFifoSem = __SEMAPHORE_INITIALIZER(gRtkCrDmpRmtFifoSem, 0);

static CoreDumpRemote *gpstCrDmpRmtVa = NULL;
static CORE_DUMP_STAGE geStage        = E_CORE_DUMP_STAGE_PREPARE;

static CrDmpRmtFifo      gstCrDmpRmtFifo;
static struct timespec64 ts_start;

static unsigned long _getCrDmpRmtSize(CoreDumpRemote *pstCrDmpRmt)
{
    int           i;
    unsigned long totalSize = 0;
#ifdef RTOS_CORE_DUMP_REMOTE_DEBUG
    unsigned int *pBufVa = NULL;
#endif

    if (!pstCrDmpRmt)
        return 0;

#ifdef RTOS_CORE_DUMP_REMOTE_DEBUG
    RTOS_CORE_DUMP_REMOTE_PRINTF("coreDumpLv = 0x%x\n", pstCrDmpRmt->coreDumpLv);
    RTOS_CORE_DUMP_REMOTE_PRINTF("isCoreDumpOtherCtxVfp = 0x%x\n", pstCrDmpRmt->isCoreDumpOtherCtxVfp);
    RTOS_CORE_DUMP_REMOTE_PRINTF("isConfigVfpSupport = 0x%x\n", pstCrDmpRmt->isConfigVfpSupport);
    RTOS_CORE_DUMP_REMOTE_PRINTF("loadNum = 0x%x\n", pstCrDmpRmt->loadNum);
    RTOS_CORE_DUMP_REMOTE_PRINTF("pid = 0x%x\n", pstCrDmpRmt->pid);
    RTOS_CORE_DUMP_REMOTE_PRINTF("signalType = 0x%x\n", pstCrDmpRmt->signalType);
    RTOS_CORE_DUMP_REMOTE_PRINTF("Rtos MEM = 0x%lx ~ 0x%lx, size = 0x%lx\n", pstCrDmpRmt->memDumpStart,
                                 pstCrDmpRmt->memDumpEnd, pstCrDmpRmt->memDumpEnd - pstCrDmpRmt->memDumpStart);
    RTOS_CORE_DUMP_REMOTE_PRINTF("Task Cnt = %u\n", pstCrDmpRmt->taskCnt);
    RTOS_CORE_DUMP_REMOTE_PRINTF("Task 0 = 0x%lX ~ 0x%lX, size = 0x%lx\n", pstCrDmpRmt->curTaskStackTop,
                                 pstCrDmpRmt->curTaskStackBot,
                                 pstCrDmpRmt->curTaskStackBot - pstCrDmpRmt->curTaskStackTop);
    RTOS_CORE_DUMP_REMOTE_PRINTF("\ttask id = 0x%x\n", pstCrDmpRmt->curTaskId);

    pBufVa = (unsigned int *)CamOsMemMap(pstCrDmpRmt->curArchGRegAddr, sizeof(unsigned int) * 13, 1);
    if (!pBufVa)
    {
        printk(KERN_EMERG "[%d] %s: CamOsMemMap() fail\n", __LINE__, __func__);
        return 0;
    }
    RTOS_CORE_DUMP_REMOTE_PRINTF("\tarchGReg = \n");
    RTOS_CORE_DUMP_REMOTE_PRINTF("\t0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x", *(pBufVa + 0),
                                 *(pBufVa + 1), *(pBufVa + 2), *(pBufVa + 3), *(pBufVa + 4), *(pBufVa + 5),
                                 *(pBufVa + 6), *(pBufVa + 7));
    RTOS_CORE_DUMP_REMOTE_PRINTF("\t0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08lx 0x%08lx 0x%08lx", *(pBufVa + 8),
                                 *(pBufVa + 9), *(pBufVa + 10), *(pBufVa + 11), *(pBufVa + 12), pstCrDmpRmt->curArchSp,
                                 pstCrDmpRmt->curArchLr, pstCrDmpRmt->curArchPc);
    RTOS_CORE_DUMP_REMOTE_PRINTF("\t0x%08lx", pstCrDmpRmt->curArchCpsr);
    CamOsMemUnmap(pBufVa, sizeof(unsigned int) * 13);

    if (!pstCrDmpRmt->vfpStateInHwTask)
    {
        RTOS_CORE_DUMP_REMOTE_PRINTF("\taVfpInHwRef[64] =\n");
        for (i = 0; i < 64; i += 16)
        {
            RTOS_CORE_DUMP_REMOTE_PRINTF(
                "\t0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x\n",
                pstCrDmpRmt->aVfpInHwRef[i + 0], pstCrDmpRmt->aVfpInHwRef[i + 1], pstCrDmpRmt->aVfpInHwRef[i + 2],
                pstCrDmpRmt->aVfpInHwRef[i + 3], pstCrDmpRmt->aVfpInHwRef[i + 4], pstCrDmpRmt->aVfpInHwRef[i + 5],
                pstCrDmpRmt->aVfpInHwRef[i + 6], pstCrDmpRmt->aVfpInHwRef[i + 7], pstCrDmpRmt->aVfpInHwRef[i + 8],
                pstCrDmpRmt->aVfpInHwRef[i + 9], pstCrDmpRmt->aVfpInHwRef[i + 10], pstCrDmpRmt->aVfpInHwRef[i + 11],
                pstCrDmpRmt->aVfpInHwRef[i + 12], pstCrDmpRmt->aVfpInHwRef[i + 13], pstCrDmpRmt->aVfpInHwRef[i + 14],
                pstCrDmpRmt->aVfpInHwRef[i + 15]);
        }
    }
    else
    {
        pBufVa = (unsigned int *)CamOsMemMap(pstCrDmpRmt->curVfpRefAddr, sizeof(unsigned int) * 64, 1);
        if (!pBufVa)
        {
            printk(KERN_EMERG "[%d] %s: CamOsMemMap() fail\n", __LINE__, __func__);
            return 0;
        }
        RTOS_CORE_DUMP_REMOTE_PRINTF("\tvfpRef = \n");
        for (i = 0; i < 64; i += 8)
        {
            RTOS_CORE_DUMP_REMOTE_PRINTF("\t0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x\n",
                                         *(pBufVa + i + 0), *(pBufVa + i + 1), *(pBufVa + i + 2), *(pBufVa + i + 3),
                                         *(pBufVa + i + 4), *(pBufVa + i + 5), *(pBufVa + i + 6), *(pBufVa + i + 7));
        }
        CamOsMemUnmap(pBufVa, sizeof(unsigned int) * 64);
    }
    RTOS_CORE_DUMP_REMOTE_PRINTF("\tfpscr = 0x%x\n", pstCrDmpRmt->curFpscr);

    for (i = 0; i < pstCrDmpRmt->taskCnt - 1; i++)
    {
        u8 *          pTcbVa   = CamOsMemMap(pstCrDmpRmt->aTcbAddr[i], pstCrDmpRmt->tcbSize, 1);
        unsigned long stackTop = 0, stackBot = 0, archGRegArrayAddr = 0;
        unsigned int  taskId = 0, fpscr = 0, j = 0;

        if (!pTcbVa)
        {
            printk(KERN_EMERG "[%d] %s: CamOsMemMap() fail\n", __LINE__, __func__);
            return 0;
        }

        stackTop          = *((unsigned long *)(pTcbVa + pstCrDmpRmt->stTcbMap.stackTopOffset));
        stackBot          = *((unsigned long *)(pTcbVa + pstCrDmpRmt->stTcbMap.stackBotOffset));
        taskId            = *((unsigned int *)(pTcbVa + pstCrDmpRmt->stTcbMap.taskIdOffset));
        archGRegArrayAddr = (unsigned long)(*((void **)(pTcbVa + pstCrDmpRmt->stTcbMap.archGRegArrayOffset)))
                            + pstCrDmpRmt->stTcbMap.r0OffInStackAndVfpRegSize;
        fpscr = *((unsigned int *)(pTcbVa + pstCrDmpRmt->stTcbMap.fpscrOffset));

        RTOS_CORE_DUMP_REMOTE_PRINTF("Task %d = 0x%lX ~ 0x%lX, size = 0x%lx\n", i + 1, stackTop, stackBot,
                                     stackBot - stackTop);
        RTOS_CORE_DUMP_REMOTE_PRINTF("\ttask id = 0x%x\n", taskId);

        pBufVa = (unsigned int *)CamOsMemMap(archGRegArrayAddr, sizeof(unsigned int) * 16, 1);
        if (!pBufVa)
        {
            printk(KERN_EMERG "[%d] %s: CamOsMemMap() fail\n", __LINE__, __func__);
            CamOsMemUnmap(pTcbVa, pstCrDmpRmt->tcbSize);
            return 0;
        }
        RTOS_CORE_DUMP_REMOTE_PRINTF("\tarchGReg = \n");
        RTOS_CORE_DUMP_REMOTE_PRINTF("\t0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x", *(pBufVa + 0),
                                     *(pBufVa + 1), *(pBufVa + 2), *(pBufVa + 3), *(pBufVa + 4), *(pBufVa + 5),
                                     *(pBufVa + 6), *(pBufVa + 7));
        RTOS_CORE_DUMP_REMOTE_PRINTF("\t0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08lx 0x%08x 0x%08x", *(pBufVa + 8),
                                     *(pBufVa + 9), *(pBufVa + 10), *(pBufVa + 11), *(pBufVa + 12),
                                     archGRegArrayAddr + sizeof(u32) * 16, *(pBufVa + 13), *(pBufVa + 14));
        RTOS_CORE_DUMP_REMOTE_PRINTF("\t0x%08x", *(pBufVa + 15));
        CamOsMemUnmap(pBufVa, sizeof(unsigned int) * 16);

        if (pstCrDmpRmt->vfpStateInHwTask == i + 1)
        {
            RTOS_CORE_DUMP_REMOTE_PRINTF("\taVfpInHwRef[64] =\n");
            for (i = 0; i < 64; i += 16)
            {
                RTOS_CORE_DUMP_REMOTE_PRINTF(
                    "\t0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x\n",
                    pstCrDmpRmt->aVfpInHwRef[i + 0], pstCrDmpRmt->aVfpInHwRef[i + 1], pstCrDmpRmt->aVfpInHwRef[i + 2],
                    pstCrDmpRmt->aVfpInHwRef[i + 3], pstCrDmpRmt->aVfpInHwRef[i + 4], pstCrDmpRmt->aVfpInHwRef[i + 5],
                    pstCrDmpRmt->aVfpInHwRef[i + 6], pstCrDmpRmt->aVfpInHwRef[i + 7], pstCrDmpRmt->aVfpInHwRef[i + 8],
                    pstCrDmpRmt->aVfpInHwRef[i + 9], pstCrDmpRmt->aVfpInHwRef[i + 10], pstCrDmpRmt->aVfpInHwRef[i + 11],
                    pstCrDmpRmt->aVfpInHwRef[i + 12], pstCrDmpRmt->aVfpInHwRef[i + 13],
                    pstCrDmpRmt->aVfpInHwRef[i + 14], pstCrDmpRmt->aVfpInHwRef[i + 15]);
            }
        }
        else
        {
            unsigned int *vfpRefArrayAddr = (unsigned int *)(pTcbVa + pstCrDmpRmt->stTcbMap.vfpRefArrayOffset);
            RTOS_CORE_DUMP_REMOTE_PRINTF("\tvfpRef = \n");
            for (j = 0; j < 64; j += 8)
            {
                RTOS_CORE_DUMP_REMOTE_PRINTF("\t0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x\n",
                                             *(vfpRefArrayAddr + j + 0), *(vfpRefArrayAddr + j + 1),
                                             *(vfpRefArrayAddr + j + 2), *(vfpRefArrayAddr + j + 3),
                                             *(vfpRefArrayAddr + j + 4), *(vfpRefArrayAddr + j + 5),
                                             *(vfpRefArrayAddr + j + 6), *(vfpRefArrayAddr + j + 7));
            }
        }
        RTOS_CORE_DUMP_REMOTE_PRINTF("\tfpscr = 0x%x\n", fpscr);
        CamOsMemUnmap(pTcbVa, pstCrDmpRmt->tcbSize);
    }
#endif

    if (pstCrDmpRmt->coreDumpLv != 3)
    {
        totalSize += pstCrDmpRmt->curTaskStackBot - pstCrDmpRmt->curTaskStackTop;

        for (i = 0; i < pstCrDmpRmt->taskCnt - 1; i++)
        {
            u8 *          pTcbVa   = CamOsMemMap(pstCrDmpRmt->aTcbAddr[i], pstCrDmpRmt->tcbSize, 1);
            unsigned long stackTop = 0, stackBot = 0;

            if (!pTcbVa)
            {
                printk(KERN_EMERG "[%d] %s: CamOsMemMap() fail\n", __LINE__, __func__);
                return 0;
            }

            stackTop = *((unsigned long *)(pTcbVa + pstCrDmpRmt->stTcbMap.stackTopOffset));
            stackBot = *((unsigned long *)(pTcbVa + pstCrDmpRmt->stTcbMap.stackBotOffset));
            totalSize += stackBot - stackTop;
            CamOsMemUnmap(pTcbVa, pstCrDmpRmt->tcbSize);
        }
    }

    RTOS_CORE_DUMP_REMOTE_PRINTF("Stack size = 0x%lx\n", totalSize);

    totalSize += pstCrDmpRmt->memDumpEnd - pstCrDmpRmt->memDumpStart;
    RTOS_CORE_DUMP_REMOTE_PRINTF("MEM size = 0x%lx, total size = 0x%lx\n",
                                 pstCrDmpRmt->memDumpEnd - pstCrDmpRmt->memDumpStart, totalSize);

    totalSize += sizeof(SS_CORE_ELF_HEADER_t) + (sizeof(SS_CORE_ELF_PROGRAM_t) << 1);
    if (pstCrDmpRmt->coreDumpLv != 3)
        totalSize += sizeof(SS_CORE_ELF_PROGRAM_t) * pstCrDmpRmt->taskCnt;
    totalSize += sizeof(SS_CORE_ELF_NOTE_HEADER_t) + sizeof(SS_CORE_ELF_PRSTATUS_t);
    totalSize += sizeof(SS_CORE_ELF_NOTE_HEADER_t) + (sizeof(SS_CORE_ELF_AUXV_PAIR_t) << 1);
    totalSize += sizeof(SS_CORE_ELF_NOTE_HEADER_t) + sizeof(SS_CORE_ELF_FPREGSETARM_VFP_t);

    if (pstCrDmpRmt->coreDumpLv != 1)
    {
        totalSize += (sizeof(SS_CORE_ELF_NOTE_HEADER_t) + sizeof(SS_CORE_ELF_PRSTATUS_t)) * (pstCrDmpRmt->taskCnt - 1);
        if (pstCrDmpRmt->isConfigVfpSupport && pstCrDmpRmt->isCoreDumpOtherCtxVfp)
            totalSize += (sizeof(SS_CORE_ELF_NOTE_HEADER_t) + sizeof(SS_CORE_ELF_FPREGSETARM_VFP_t))
                         * (pstCrDmpRmt->taskCnt - 1);
    }

    RTOS_CORE_DUMP_REMOTE_PRINTF("Total Size = 0x%lx\n", totalSize);

    return totalSize;
}

static void _ElfFillHeader(SS_CORE_ELF_HEADER_t *hdr, u32 progNum)
{
    hdr->identMagic[0]   = 0x7F; // Magic
    hdr->identMagic[1]   = 0x45; // Magic
    hdr->identMagic[2]   = 0x4C; // Magic
    hdr->identMagic[3]   = 0x46; // Magic
    hdr->identClass      = 1;    // ELF32
    hdr->identData       = 1;    // 2's complement, little endian
    hdr->identVersion    = 1;    // 1 (current)
    hdr->identOsAbi      = 0;    // UNIX - System V
    hdr->identAbiVersion = 0;    // version 0
    memset(hdr->identPad, 0, sizeof(hdr->identPad));
    hdr->type             = 0x0004; // CORE
    hdr->machine          = 0x0028; // ARM
    hdr->version          = 1;      // version 1
    hdr->entry            = 0;
    hdr->programHdrOffset = sizeof(SS_CORE_ELF_HEADER_t);
    hdr->sectionHdrOffset = 0;
    hdr->flags            = 0;
    hdr->hdrSize          = sizeof(SS_CORE_ELF_HEADER_t);
    hdr->programHdrSize   = sizeof(SS_CORE_ELF_PROGRAM_t);
    hdr->programHdrNum    = progNum;
    hdr->sectionHdrSize   = 0;
    hdr->sectionHdrNum    = 0;
    hdr->sectionHdrTblIdx = 0;
}

static void _ElfFillProgramNote(SS_CORE_ELF_PROGRAM_t *prog, u32 offset, u32 size)
{
    prog->type     = 0x4; // Type NOTE
    prog->offset   = offset;
    prog->va       = 0;
    prog->pa       = 0;
    prog->fileSize = size;
    prog->memSize  = 0;
    prog->flags    = 0;
    prog->align    = 0;
}

static void _ElfFillProgramLoad(SS_CORE_ELF_PROGRAM_t *prog, u32 offset, u32 va, u32 size)
{
    prog->type     = 0x1; // Type LOAD
    prog->offset   = offset;
    prog->va       = va;
    prog->pa       = 0;
    prog->fileSize = size;
    prog->memSize  = size;
    prog->flags    = 0x06; // R+W
    prog->align    = 0;
}

static void _ElfFillNotePrstatus(SS_CORE_ELF_NOTE_HEADER_t *noteHdr, SS_CORE_ELF_PRSTATUS_t *prstatus, u32 signalType,
                                 u32 pid, u32 *regArray, u32 dfsr, u32 ifsr, u32 dfar, u32 ifar, u32 fpValid)
{
    snprintf(noteHdr->name, sizeof(noteHdr->name), "%s", "CORE");
    noteHdr->nameSize = strlen(noteHdr->name) + 1;
    noteHdr->noteSize = sizeof(SS_CORE_ELF_PRSTATUS_t);
    noteHdr->type     = 0x1;

    prstatus->sigInfo.signalNum = signalType; // SIGSEGV
    prstatus->sigInfo.code      = 0;
    prstatus->sigInfo.errno     = 0;
    prstatus->currSig           = signalType; // SIGSEGV
    prstatus->sigPend           = 0;
    prstatus->sigHold           = 0;
    prstatus->pid               = pid;
    prstatus->ppid              = 0;
    prstatus->pgrp              = pid;
    prstatus->sid               = 0;
    prstatus->utime             = 0;
    prstatus->stime             = 0;
    prstatus->cutime            = 0;
    prstatus->cstime            = 0;
    memcpy(prstatus->regset.reg, regArray, sizeof(unsigned int) * 13);
    prstatus->regset.reg[13] = dfsr;
    prstatus->regset.reg[14] = ifsr;
    prstatus->regset.reg[15] = dfar;
    prstatus->regset.reg[16] = ifar;
    prstatus->regset.reg[17] = 0;
    prstatus->fpValid        = fpValid;
}

static void _ElfFillNoteAUXV(SS_CORE_ELF_NOTE_HEADER_t *noteHdr, u32 pairNum)
{
    snprintf(noteHdr->name, sizeof(noteHdr->name), "%s", "CORE");
    noteHdr->nameSize = strlen(noteHdr->name) + 1;
    noteHdr->noteSize = sizeof(SS_CORE_ELF_AUXV_PAIR_t) * pairNum;
    noteHdr->type     = 0x6;
}

static void _ElfFillNoteArmVfp(SS_CORE_ELF_NOTE_HEADER_t *noteHdr, SS_CORE_ELF_FPREGSETARM_VFP_t *vfp, u32 *regArray,
                               u32 fpscr)
{
    snprintf(noteHdr->name, sizeof(noteHdr->name), "%s", "LINUX");
    noteHdr->nameSize = strlen(noteHdr->name) + 1;
    noteHdr->noteSize = sizeof(SS_CORE_ELF_FPREGSETARM_VFP_t);
    noteHdr->type     = 0x400;

    memcpy(vfp->fpregs, regArray, sizeof(vfp->fpregs));
    vfp->fpscr = fpscr;
}

static u32 _coreDumpRemoteWriteBuf(u8 *pBuf, u32 totalSize)
{
    u32 restSize = totalSize;

    if (gstCrDmpRmtFifo.size > 0 && gstCrDmpRmtFifo.size < CORE_DUMP_REMOTE_FIFO_SIZE)
    {
        if (totalSize + gstCrDmpRmtFifo.size >= CORE_DUMP_REMOTE_FIFO_SIZE)
        {
            memcpy(gstCrDmpRmtFifo.pBuf + gstCrDmpRmtFifo.size, pBuf,
                   CORE_DUMP_REMOTE_FIFO_SIZE - gstCrDmpRmtFifo.size);
            restSize -= CORE_DUMP_REMOTE_FIFO_SIZE - gstCrDmpRmtFifo.size;
            RTOS_CORE_DUMP_REMOTE_PRINTF("Write 0x%lX bytes, rest 0x%x, READY\n",
                                         CORE_DUMP_REMOTE_FIFO_SIZE - gstCrDmpRmtFifo.size, restSize);
            gstCrDmpRmtFifo.size = CORE_DUMP_REMOTE_FIFO_SIZE;
            up(&gRtkCrDmpRmtShowSem);
            down(&gRtkCrDmpRmtFifoSem);
        }
        else
        {
            memcpy(gstCrDmpRmtFifo.pBuf + gstCrDmpRmtFifo.size, pBuf, totalSize);
            RTOS_CORE_DUMP_REMOTE_PRINTF("Write 0x%X bytes, rest 0\n", totalSize);
            gstCrDmpRmtFifo.size += totalSize;
            restSize = 0;
        }
    }

    while (restSize)
    {
        if (restSize >= CORE_DUMP_REMOTE_FIFO_SIZE)
        {
            memcpy(gstCrDmpRmtFifo.pBuf, pBuf + totalSize - restSize, CORE_DUMP_REMOTE_FIFO_SIZE);
            restSize -= CORE_DUMP_REMOTE_FIFO_SIZE;
            RTOS_CORE_DUMP_REMOTE_PRINTF("Write 0x%lX bytes, rest 0x%x, READY\n", CORE_DUMP_REMOTE_FIFO_SIZE, restSize);
            gstCrDmpRmtFifo.size = CORE_DUMP_REMOTE_FIFO_SIZE;
            up(&gRtkCrDmpRmtShowSem);
            down(&gRtkCrDmpRmtFifoSem);
        }
        else
        {
            memcpy(gstCrDmpRmtFifo.pBuf, pBuf + totalSize - restSize, restSize);
            gstCrDmpRmtFifo.size = restSize;
            RTOS_CORE_DUMP_REMOTE_PRINTF("Write 0x%X bytes, rest 0\n", restSize);
            break;
        }
    }
    return totalSize;
}

static void _coreDumpRemoteFlushBuf(void)
{
    RTOS_CORE_DUMP_REMOTE_PRINTF("READY\n");
    up(&gRtkCrDmpRmtShowSem);
}

static int _tfunc_fill_fifo(void *arg)
{
    CoreDumpRemote *              pstCrDmpRmt = (CoreDumpRemote *)arg;
    unsigned long                 memSize     = 0;
    int                           i;
    u8 *                          pBufVa = NULL;
    u32                           size = 0, totalSize = 0;
    SS_CORE_ELF_HEADER_t          stElfHdr       = {0};
    SS_CORE_ELF_PROGRAM_t         stProg         = {0};
    SS_CORE_ELF_NOTE_HEADER_t     stNoteHdr      = {0};
    SS_CORE_ELF_PRSTATUS_t        stPrStatus     = {0};
    SS_CORE_ELF_AUXV_PAIR_t       astAuxvPair[2] = {0};
    SS_CORE_ELF_FPREGSETARM_VFP_t stArmvfp       = {0};

    _ElfFillHeader(&stElfHdr, pstCrDmpRmt->loadNum + 1);
    totalSize += _coreDumpRemoteWriteBuf((u8 *)&stElfHdr, sizeof(SS_CORE_ELF_HEADER_t));

    size = sizeof(SS_CORE_ELF_NOTE_HEADER_t) + (sizeof(SS_CORE_ELF_AUXV_PAIR_t) << 1)
           + (sizeof(SS_CORE_ELF_NOTE_HEADER_t) + sizeof(SS_CORE_ELF_PRSTATUS_t)) * pstCrDmpRmt->ctxNum;
    if (pstCrDmpRmt->isCoreDumpOtherCtxVfp)
        size += (sizeof(SS_CORE_ELF_NOTE_HEADER_t) + sizeof(SS_CORE_ELF_FPREGSETARM_VFP_t)) * pstCrDmpRmt->ctxNum;
    else
        size += sizeof(SS_CORE_ELF_NOTE_HEADER_t) + sizeof(SS_CORE_ELF_FPREGSETARM_VFP_t);

    _ElfFillProgramNote(&stProg, sizeof(SS_CORE_ELF_HEADER_t) + stElfHdr.programHdrNum * sizeof(SS_CORE_ELF_PROGRAM_t),
                        size);
    totalSize += _coreDumpRemoteWriteBuf((u8 *)&stProg, sizeof(SS_CORE_ELF_PROGRAM_t));

    _ElfFillProgramLoad(&stProg, stProg.offset + stProg.fileSize, pstCrDmpRmt->memDumpStart,
                        pstCrDmpRmt->memDumpEnd - pstCrDmpRmt->memDumpStart);
    totalSize += _coreDumpRemoteWriteBuf((u8 *)&stProg, sizeof(SS_CORE_ELF_PROGRAM_t));

    if (pstCrDmpRmt->coreDumpLv != 3)
    {
        memSize = pstCrDmpRmt->curTaskStackBot - pstCrDmpRmt->curTaskStackTop;
        _ElfFillProgramLoad(&stProg, stProg.offset + stProg.fileSize, pstCrDmpRmt->curTaskStackTop, memSize);
        totalSize += _coreDumpRemoteWriteBuf((u8 *)&stProg, sizeof(SS_CORE_ELF_PROGRAM_t));
        for (i = 0; i < pstCrDmpRmt->taskCnt - 1; i++)
        {
            pBufVa = CamOsMemMap(pstCrDmpRmt->aTcbAddr[i], pstCrDmpRmt->tcbSize, 1);
            if (!pBufVa)
            {
                printk(KERN_EMERG "[%d] %s: CamOsMemMap() fail\n", __LINE__, __func__);
                return 0;
            }
            memSize = *((unsigned long *)(pBufVa + pstCrDmpRmt->stTcbMap.stackBotOffset))
                      - *((unsigned long *)(pBufVa + pstCrDmpRmt->stTcbMap.stackTopOffset));
            _ElfFillProgramLoad(&stProg, stProg.offset + stProg.fileSize,
                                *((u32 *)(pBufVa + pstCrDmpRmt->stTcbMap.stackTopOffset)), memSize);
            totalSize += _coreDumpRemoteWriteBuf((u8 *)&stProg, sizeof(SS_CORE_ELF_PROGRAM_t));
            CamOsMemUnmap(pBufVa, pstCrDmpRmt->tcbSize);
        }
    }

    pBufVa = CamOsMemMap(pstCrDmpRmt->curArchGRegAddr, sizeof(u32) * 13, 1);
    if (!pBufVa)
    {
        printk(KERN_EMERG "[%d] %s: CamOsMemMap() fail\n", __LINE__, __func__);
        return 0;
    }
    _ElfFillNotePrstatus(&stNoteHdr, &stPrStatus, pstCrDmpRmt->signalType, pstCrDmpRmt->pid, (u32 *)pBufVa,
                         pstCrDmpRmt->curArchSp, pstCrDmpRmt->curArchLr, pstCrDmpRmt->curArchPc,
                         pstCrDmpRmt->curArchCpsr, 1);
    totalSize += _coreDumpRemoteWriteBuf((u8 *)&stNoteHdr, sizeof(SS_CORE_ELF_NOTE_HEADER_t));
    totalSize += _coreDumpRemoteWriteBuf((u8 *)&stPrStatus, sizeof(SS_CORE_ELF_PRSTATUS_t));
    CamOsMemUnmap(pBufVa, sizeof(u32) * 13);

    astAuxvPair[0].type  = 0x10;       // AT_HWCAP
    astAuxvPair[0].value = 0x003FB8D6; // HWCAP value
    astAuxvPair[1].type  = 0x0;        // AT_NULL
    astAuxvPair[1].value = 0x0;        // end of vector
    _ElfFillNoteAUXV(&stNoteHdr, sizeof(astAuxvPair) / sizeof(SS_CORE_ELF_AUXV_PAIR_t));
    totalSize += _coreDumpRemoteWriteBuf((u8 *)&stNoteHdr, sizeof(SS_CORE_ELF_NOTE_HEADER_t));
    totalSize += _coreDumpRemoteWriteBuf((u8 *)&astAuxvPair, sizeof(SS_CORE_ELF_AUXV_PAIR_t) << 1);

    if (!pstCrDmpRmt->vfpStateInHwTask)
        _ElfFillNoteArmVfp(&stNoteHdr, &stArmvfp, pstCrDmpRmt->aVfpInHwRef, pstCrDmpRmt->curFpscr);
    else
    {
        pBufVa = CamOsMemMap(pstCrDmpRmt->curVfpRefAddr, sizeof(u32) << 6, 1);
        if (!pBufVa)
        {
            printk(KERN_EMERG "[%d] %s: CamOsMemMap() fail\n", __LINE__, __func__);
            return 0;
        }
        _ElfFillNoteArmVfp(&stNoteHdr, &stArmvfp, (u32 *)pBufVa, pstCrDmpRmt->curFpscr);
        CamOsMemUnmap(pBufVa, sizeof(u32) << 6);
    }
    totalSize += _coreDumpRemoteWriteBuf((u8 *)&stNoteHdr, sizeof(SS_CORE_ELF_NOTE_HEADER_t));
    totalSize += _coreDumpRemoteWriteBuf((u8 *)&stArmvfp, sizeof(SS_CORE_ELF_FPREGSETARM_VFP_t));

    if (pstCrDmpRmt->coreDumpLv != 1)
    {
        for (i = 0; i < pstCrDmpRmt->taskCnt - 1; i++)
        {
            u8 *          pTcbVa = CamOsMemMap(pstCrDmpRmt->aTcbAddr[i], pstCrDmpRmt->tcbSize, 1);
            unsigned int  taskId = 0, fpscr = 0, *pVfpRefArray = NULL;
            unsigned long archGRegArrayAddr = 0;

            if (!pTcbVa)
            {
                printk(KERN_EMERG "[%d] %s: CamOsMemMap() fail\n", __LINE__, __func__);
                return 0;
            }
            taskId            = *((unsigned int *)(pTcbVa + pstCrDmpRmt->stTcbMap.taskIdOffset));
            fpscr             = *((unsigned int *)(pTcbVa + pstCrDmpRmt->stTcbMap.fpscrOffset));
            pVfpRefArray      = (u32 *)(pTcbVa + pstCrDmpRmt->stTcbMap.vfpRefArrayOffset);
            archGRegArrayAddr = (unsigned long)(*((void **)(pTcbVa + pstCrDmpRmt->stTcbMap.archGRegArrayOffset)))
                                + pstCrDmpRmt->stTcbMap.r0OffInStackAndVfpRegSize;

            pBufVa = CamOsMemMap(archGRegArrayAddr, sizeof(unsigned int) * 16, 1);
            if (!pBufVa)
            {
                printk(KERN_EMERG "[%d] %s: CamOsMemMap() fail\n", __LINE__, __func__);
                CamOsMemUnmap(pTcbVa, pstCrDmpRmt->tcbSize);
                return 0;
            }
            _ElfFillNotePrstatus(&stNoteHdr, &stPrStatus, pstCrDmpRmt->signalType, taskId, (u32 *)pBufVa,
                                 archGRegArrayAddr + sizeof(u32) * 16, *((u32 *)pBufVa + 13), *((u32 *)pBufVa + 14),
                                 *((u32 *)pBufVa + 15), pstCrDmpRmt->isCoreDumpOtherCtxVfp);
            CamOsMemUnmap(pBufVa, sizeof(u32) * 16);
            totalSize += _coreDumpRemoteWriteBuf((u8 *)&stNoteHdr, sizeof(SS_CORE_ELF_NOTE_HEADER_t));
            totalSize += _coreDumpRemoteWriteBuf((u8 *)&stPrStatus, sizeof(SS_CORE_ELF_PRSTATUS_t));

            if (pstCrDmpRmt->isConfigVfpSupport && pstCrDmpRmt->isCoreDumpOtherCtxVfp)
            {
                if (pstCrDmpRmt->vfpStateInHwTask == i + 1)
                    _ElfFillNoteArmVfp(&stNoteHdr, &stArmvfp, pstCrDmpRmt->aVfpInHwRef, fpscr);
                else
                    _ElfFillNoteArmVfp(&stNoteHdr, &stArmvfp, pVfpRefArray, fpscr);
                totalSize += _coreDumpRemoteWriteBuf((u8 *)&stNoteHdr, sizeof(SS_CORE_ELF_NOTE_HEADER_t));
                totalSize += _coreDumpRemoteWriteBuf((u8 *)&stArmvfp, sizeof(SS_CORE_ELF_FPREGSETARM_VFP_t));
            }
            CamOsMemUnmap(pTcbVa, pstCrDmpRmt->tcbSize);
        }
    }
    RTOS_CORE_DUMP_REMOTE_PRINTF("Write HEAD 0x%X bytes\n", totalSize);

    // RTOS memory
    memSize = pstCrDmpRmt->memDumpEnd - pstCrDmpRmt->memDumpStart;
    RTOS_CORE_DUMP_REMOTE_PRINTF("Write MEM 0x%lX bytes\n", memSize);
    pBufVa = CamOsMemMap(pstCrDmpRmt->memDumpStart, memSize, 1);
    if (!pBufVa)
    {
        printk(KERN_EMERG "[%d] %s: CamOsMemMap() fail\n", __LINE__, __func__);
        return 0;
    }
    totalSize += _coreDumpRemoteWriteBuf(pBufVa, memSize);
    CamOsMemUnmap(pBufVa, memSize);

    // Stack
    if (pstCrDmpRmt->coreDumpLv != 3)
    {
        memSize = pstCrDmpRmt->curTaskStackBot - pstCrDmpRmt->curTaskStackTop;
        RTOS_CORE_DUMP_REMOTE_PRINTF("Write Stack 0 0x%lX bytes\n", memSize);
        pBufVa = CamOsMemMap(pstCrDmpRmt->curTaskStackTop, memSize, 1);
        if (!pBufVa)
        {
            printk(KERN_EMERG "[%d] %s: CamOsMemMap() fail\n", __LINE__, __func__);
            return 0;
        }
        totalSize += _coreDumpRemoteWriteBuf(pBufVa, memSize);
        CamOsMemUnmap(pBufVa, memSize);
        for (i = 0; i < pstCrDmpRmt->taskCnt - 1; i++)
        {
            u8 *          pTcbVa = CamOsMemMap(pstCrDmpRmt->aTcbAddr[i], pstCrDmpRmt->tcbSize, 1);
            unsigned long stackTop, stackBot;

            if (!pTcbVa)
            {
                printk(KERN_EMERG "[%d] %s: CamOsMemMap() fail\n", __LINE__, __func__);
                return 0;
            }
            stackTop = *((unsigned long *)(pTcbVa + pstCrDmpRmt->stTcbMap.stackTopOffset));
            stackBot = *((unsigned long *)(pTcbVa + pstCrDmpRmt->stTcbMap.stackBotOffset));

            memSize = stackBot - stackTop;
            RTOS_CORE_DUMP_REMOTE_PRINTF("Write Stack %u 0x%lX bytes\n", i + 1, memSize);
            pBufVa = CamOsMemMap(stackTop, memSize, 1);
            if (!pBufVa)
            {
                printk(KERN_EMERG "[%d] %s: CamOsMemMap() fail\n", __LINE__, __func__);
                CamOsMemUnmap(pTcbVa, pstCrDmpRmt->tcbSize);
                return 0;
            }
            totalSize += _coreDumpRemoteWriteBuf(pBufVa, memSize);
            CamOsMemUnmap(pBufVa, memSize);
            CamOsMemUnmap(pTcbVa, pstCrDmpRmt->tcbSize);
        }
    }

    _coreDumpRemoteFlushBuf();

    RTOS_CORE_DUMP_REMOTE_PRINTF("Done. TotalSize = 0x%x\n", totalSize);
    return 0;
}

static void *c_start_crdmprmt(struct seq_file *m, loff_t *pos)
{
    static struct task_struct *pKth = NULL;

    RTOS_CORE_DUMP_REMOTE_PRINTF("pos = 0x%llx\n", *pos);

    if (*pos == 0)
    {
        if (geStage == E_CORE_DUMP_STAGE_PREPARE)
        {
            ktime_get_ts64(&ts_start);
            geStage = E_CORE_DUMP_STAGE_START;
            memset(&gstCrDmpRmtFifo, 0, sizeof(CrDmpRmtFifo));
            gstCrDmpRmtFifo.pBuf = kzalloc(CORE_DUMP_REMOTE_FIFO_SIZE, GFP_KERNEL);
            if (get_rtkinfo() && get_rtkinfo()->core_dump_remote_addr && get_rtkinfo()->core_dump_remote_size)
            {
                gpstCrDmpRmtVa =
                    CamOsMemMap(get_rtkinfo()->core_dump_remote_addr, get_rtkinfo()->core_dump_remote_size, 1);

                m->private = (void *)_getCrDmpRmtSize(gpstCrDmpRmtVa);

                pKth = kthread_create(_tfunc_fill_fifo, (void *)gpstCrDmpRmtVa, "kthread_RtosCrDmpRmt");
                if (pKth)
                    wake_up_process(pKth);
                else
                {
                    geStage = E_CORE_DUMP_STAGE_DONE;
                    printk(KERN_EMERG "kthread_RtosCrDmpRmt could not be created\n");
                    return NULL;
                }
                return SEQ_START_TOKEN;
            }
        }
        else if (geStage == E_CORE_DUMP_STAGE_START)
        {
            geStage = E_CORE_DUMP_STAGE_DONE;
            return NULL;
        }
    }

    return (void *)((unsigned long)*pos);
}

static int c_show_crdmprmt(struct seq_file *m, void *v)
{
    unsigned short size = 0;
    RTOS_CORE_DUMP_REMOTE_PRINTF("v = 0x%lx\n", (unsigned long)v);

    if (v == SEQ_START_TOKEN)
        return 0;

    size = gstCrDmpRmtFifo.size > PAGE_SIZE ? PAGE_SIZE : gstCrDmpRmtFifo.size;
    seq_write(m, gstCrDmpRmtFifo.pBuf + gstCrDmpRmtFifo.offset, size);

    return 0;
}

static void *c_next_crdmprmt(struct seq_file *m, void *v, loff_t *pos)
{
    RTOS_CORE_DUMP_REMOTE_PRINTF("v = 0x%lx, pos = 0x%llx\n", (unsigned long)v, *pos);

    if (v == SEQ_START_TOKEN)
    {
        down(&gRtkCrDmpRmtShowSem);
        *pos = (unsigned long)m->private;
    }
    else
    {
        *pos = ((unsigned long)v) > PAGE_SIZE ? ((unsigned long)v - PAGE_SIZE) : 0;
        if (*pos == 0)
            return (void *)((unsigned long)*pos);

        RTOS_CORE_DUMP_REMOTE_PRINTF("rest 0x%llx\n", *pos);

        gstCrDmpRmtFifo.size -= PAGE_SIZE;
        gstCrDmpRmtFifo.offset += PAGE_SIZE;
        if (!gstCrDmpRmtFifo.size)
        {
            gstCrDmpRmtFifo.offset = 0;
            up(&gRtkCrDmpRmtFifoSem);
            down(&gRtkCrDmpRmtShowSem);
        }
    }

    return (void *)((unsigned long)*pos);
}

static void c_stop_crdmprmt(struct seq_file *m, void *v)
{
    RTOS_CORE_DUMP_REMOTE_PRINTF("v = 0x%lx\n", (unsigned long)v);
    if (geStage == E_CORE_DUMP_STAGE_DONE)
    {
        struct timespec64 ts_end, ts_delta;
        if (gpstCrDmpRmtVa)
        {
            CamOsMemUnmap(gpstCrDmpRmtVa, get_rtkinfo()->core_dump_remote_size);
            gpstCrDmpRmtVa = NULL;
        }
        kfree(gstCrDmpRmtFifo.pBuf);
        up(&gRtkCrDmpRmtShowSem);
        up(&gRtkCrDmpRmtFifoSem);
        ktime_get_ts64(&ts_end);
        ts_delta = timespec64_sub(ts_end, ts_start);
        RTOS_CORE_DUMP_REMOTE_PRINTF("Take %lld ms\n", timespec64_to_ns(&ts_delta) / 1000000);
    }
    return;
}

const struct seq_operations crdmprmt_op = {
    .start = c_start_crdmprmt, .next = c_next_crdmprmt, .stop = c_stop_crdmprmt, .show = c_show_crdmprmt};

static int rtkcrdmprmt_open(struct inode *inode, struct file *file)
{
    int              res = -ENOMEM;
    struct seq_file *seq;

    res = seq_open(file, &crdmprmt_op);
    if (res)
        return res;
    seq     = file->private_data;
    geStage = E_CORE_DUMP_STAGE_PREPARE;
    return res;
}

static ssize_t rtkcrdmprmt_write(struct file *file, const char __user *pUserBuf, size_t userDataLen, loff_t *pos)
{
    unsigned long long coreDumpRemoteLv = 0;
    int                ret              = 0;

    if ((ret = kstrtoull_from_user(pUserBuf, userDataLen, 10, &coreDumpRemoteLv)))
        return ret;

    CamInterOsSignal(INTEROS_SC_L2R_SET_COREDUMP_LV, (unsigned int)coreDumpRemoteLv, 0, 0);

    return userDataLen;
}

static const struct proc_ops proc_rtkcrdmprmt_operations = {
    .proc_open    = rtkcrdmprmt_open,
    .proc_read    = seq_read,
    .proc_lseek   = seq_lseek,
    .proc_write   = rtkcrdmprmt_write,
    .proc_release = seq_release,
};

/************************* RTOS Core Dump Remote END *************************/

const struct seq_operations rsqlog_op   = {.start = c_start, .next = c_next, .stop = c_stop, .show = c_logshow};
const struct seq_operations crashlog_op = {
    .start = c_start, .next = c_next, .stop = c_stop_crashlog, .show = c_show_crashlog};

static int rsqinfo_open(struct inode *inode, struct file *file)
{
    int              res = -ENOMEM;
    int              i;
    struct seq_file *seq;

    res = seq_open(file, NULL);
    if (res)
        return res;
    seq = file->private_data;
    for (i = 0; i < sizeof(_sstr) / sizeof(struct sstream); i++)
    {
        if (!_sstr[i].rvr)
            continue;
        if (!strncmp(file->f_path.dentry->d_iname, _sstr[i].rvr->name, sizeof(_sstr[i].rvr->name)))
        {
            seq->private = _sstr[i].rvr;
            break;
        }
    }
    if (i == (sizeof(_sstr) / sizeof(struct sstream)))
        return -ENOMEM;
    seq->op = _sstr[i].private;
    return res;
}

static ssize_t rsqinfo_write(struct file *file, const char __user *buf, size_t len, loff_t *pos)
{
    char         code[32];
    reservoir_t *rv;

    rv = (reservoir_t *)((struct seq_file *)file->private_data)->private;
    if (!rv)
        return len;
    if (copy_from_user(code, buf, 32))
        return -EFAULT;
    if (strncmp(rv->name, "log", 3) == 0)
    {
        signal_rtos(INTEROS_SC_L2R_RTK_LOG, 0, 0, 0);
    }

    return len;
}

#define TTM(s) (((s) + 3000) / 6000)
static u64 _spent = 0, _lifet = 0;
static u64 _spent_hyp = 0, _spent_sc = 0;
static int c_show_rtk(struct seq_file *m, void *v)
{
    rtkinfo_t *rtk;
    int        s;
    u64        cs, cl;
    int        sh, sc;
    u64        ch, cc;

    rtk = m->private;
    if (!rtk)
    {
        seq_printf(m, "not avaliable\n");
        return 0;
    }

#ifdef CONFIG_SS_AMP
    /* in AMP, use an IPC signal to trigger CPU statistics update */
    signal_rtos(INTEROS_SC_L2R_AMP_RTK_CPU_USAGE, 0, 0, 0);
#endif

    /* reset cpu usage after present */
    cs = rtk->spent;
    cl = rtk->lifet;
    ch = rtk->spent_hyp;
    cc = rtk->spent_sc;

    s = (int)div64_u64((cs - _spent) * 1000, (cl - _lifet));
    if (ch && cc) // It means __ADV_HYP_PROF__ enabled in rtos
    {
        sh = (int)div64_u64((ch - _spent_hyp) * 1000, (cl - _lifet));
        sc = (int)div64_u64((cc - _spent_sc) * 1000, (cl - _lifet));
    }

    seq_printf(m, "RTOS: %s\n", rtk->version);

    if (ch && cc) // It means __ADV_HYP_PROF__ enabled in rtos
    {
        seq_printf(m, "\tcpu usage(hyp/sc/rtos): %u.%u%% (%llu/%llu)\n", s / 10, s % 10, cs - _spent, cl - _lifet);
        seq_printf(m, "\tcpu usage(hyp): %u.%u%% (%llu/%llu)\n", sh / 10, sh % 10, ch - _spent_hyp, cl - _lifet);
        seq_printf(m, "\tcpu usage(sc): %u.%u%% (%llu/%llu)\n", sc / 10, sc % 10, cc - _spent_sc, cl - _lifet);
    }
    else
    {
        seq_printf(m, "\tcpu usage: %u.%u%% (%llu/%llu)\n", s / 10, s % 10, cs - _spent, cl - _lifet);
    }
    seq_printf(m, "\tttff(isp): %u ms\n", TTM(rtk->ttff_isp));
    seq_printf(m, "\tttff(scl): %u ms\n", TTM(rtk->ttff_scl));
    seq_printf(m, "\tttff(mfe): %u ms\n", TTM(rtk->ttff_mfe));
    seq_printf(m, "\tload ns  : %u ms\n", TTM(rtk->ldns_ts));
    seq_printf(m, "\tfiq count: %u\n", rtk->fiq_cnt);
    seq_printf(m, "\tsyscall  : %llu\n", rtk->syscall_cnt);
    seq_printf(m, "\tstatus   : %s\n", rtk->has_dead ? "DEAD" : "ALIVE");
    seq_printf(m, "\tcrash_log_buf_addr : %lx\n", rtk->crash_log_buf_addr);
    seq_printf(m, "\tcrash_log_buf_size : %lx\n", rtk->crash_log_buf_size);
    _spent = cs;
    _lifet = cl;
    if (ch && cc) // It means __ADV_HYP_PROF__ enabled in rtk
    {
        _spent_hyp = ch;
        _spent_sc  = cc;
    }

    return 0;
}

static u32 _RtosCliExecDoneCb(u32 arg0, u32 ret, u32 unused0, u32 unused1)
{
    if (ret != 0)
    {
        printk("echo cli to RTOS execute error\n");
    }

    up(&gRtkinfoWriteSem);

    return 0;
}

static ssize_t rtkinfo_write(struct file *file, const char __user *buf, size_t len, loff_t *pos)
{
    char       code[128] = {0};
    rtkinfo_t *rtk;

    if (len > sizeof(code) - 1)
    {
        printk(KERN_ERR "command len is to long!\n");
        return len;
    }

    rtk = (rtkinfo_t *)((struct seq_file *)file->private_data)->private;
    if (!rtk || !len)
    {
        return len;
    }
    if (rtk->has_dead)
    {
        printk(KERN_ERR "Rtos has dead-2\n");
        return -EFAULT;
    }
    if (copy_from_user(code, buf, len))
    {
        return -EFAULT;
    }
    code[len] = '\0';
    // rtkinfo simple command parser
    // cli -
    if (!strncmp(code, "cli", 3))
    {
        char *ptr;
        // strip return and space in tail and head.
        ptr = strrchr(code, 0x0a);
        if (ptr)
        {
            *ptr = '\0';
        }
        for (ptr = code + 4; *ptr == ' ' || *ptr == '\t'; ptr++)
            ;

        mutex_lock(&_gRtkinfoWriteLock);
        // copy command to share buffer and send to S
        strncpy(rtk->sbox, ptr, sizeof(rtk->sbox) - 1);

        /* rtk->sbox is non-cacheable buffer, do MiuPipeFlush to
         * ensure that the data is synced to the drma*/
        CamOsMiuPipeFlush();

        signal_rtos(INTEROS_SC_L2R_RTK_CLI, (u32)(unsigned long)rtk, (u32)rtk->diff, 0);
        down(&gRtkinfoWriteSem);
        mutex_unlock(&_gRtkinfoWriteLock);
    }
    else if (!strncmp(code, "assert", 6))
    {
        signal_rtos(INTEROS_SC_L2R_RTK_ASSERT, 0, 0, 0);
    }
    return len;
}

const struct seq_operations rtkinfo_op = {.start = c_start, .next = c_next, .stop = c_stop, .show = c_show_rtk};

static int rtkinfo_open(struct inode *inode, struct file *file)
{
    int              res = -ENOMEM;
    struct seq_file *seq;

    res = seq_open(file, &rtkinfo_op);
    if (res)
        return res;
    seq          = file->private_data;
    seq->private = (void *)_rtklin;
    return res;
}

#if defined(CONFIG_LINUX_ON_SS_HYPERVISOR)
static unsigned int measure_interval_ms = 1000;
unsigned long       signal_hyp(u32 type, u32 arg1, u32 arg2, u32 arg3)
{
    struct arm_smccc_res res;

    arm_smccc_hvc(type, arg1, arg2, arg3, 0, 0, 0, 0, &res);
    return res.a0;
}

unsigned long signal_hyp_vm_died(void)
{
    return signal_hyp(HYPERCALL_FID_VM_STATUS, HYPERCALL_SUBCMD_VM_STATUS_SET_DIED, 0, 0);
}

static unsigned long    hyp_cpu_num           = 0;
static unsigned long    hyp_vm_num            = 0;
static unsigned long    hyp_cpu_stat_buf_addr = 0;
static struct resource *_hyp_cpu_stat_buf_res;
static int              c_show_hyp(struct seq_file *m, void *v)
{
    unsigned long total_count;
    u64 *         hyp_cpu_stat_buf = NULL;
    int           core_id          = 0;
    int           vm_id            = 0;
    int           percentage       = 0;
    int           idle_avg         = 0;

    // get total CPU number
    if (!hyp_cpu_num)
        hyp_cpu_num = signal_hyp(HYPERCALL_FID_VM_QUERY, HYPERCALL_SUBCMD_VM_QUERY_CPU_NUM, 0, 0);

    if (!hyp_vm_num)
        hyp_vm_num = signal_hyp(HYPERCALL_FID_VM_QUERY, HYPERCALL_SUBCMD_VM_QUERY_VM_NUM, 0, 0);

    // start stat
    signal_hyp(HYPERCALL_FID_SHOW_INFO, HYPERCALL_SUBCMD_SHOW_INFO_CPU_STAT, 1, 0);

    if (!hyp_cpu_stat_buf_addr)
    {
        hyp_cpu_stat_buf_addr = signal_hyp(HYPERCALL_FID_SHOW_INFO, HYPERCALL_SUBCMD_SHOW_INFO_CPU_STAT, 2, 0);
        _hyp_cpu_stat_buf_res = request_mem_region(hyp_cpu_stat_buf_addr, sizeof(u64) * hyp_cpu_num * 3, "hypcpustat");
    }

    CamOsMsSleep(measure_interval_ms);
    // stop stat
    total_count = signal_hyp(HYPERCALL_FID_SHOW_INFO, HYPERCALL_SUBCMD_SHOW_INFO_CPU_STAT, 0, 0);
    // get stat buf addr
    hyp_cpu_stat_buf = ioremap_wc(_hyp_cpu_stat_buf_res->start, resource_size(_hyp_cpu_stat_buf_res));
    if (!hyp_cpu_stat_buf)
    {
        printk(KERN_ERR "%s: Failed to map hyp CPU stat buffer\n", __FUNCTION__);
        return -ENOMEM;
    }

    seq_printf(m, "========== CPU Usage ==========\n");
    seq_printf(m, "\tIDLE    ");
    for (vm_id = 0; vm_id < hyp_vm_num; vm_id++)
        seq_printf(m, "VM%d     ", vm_id);
    seq_printf(m, "\n");
    for (core_id = 0; core_id < hyp_cpu_num; core_id++)
    {
        percentage =
            (int)div64_u64(*(hyp_cpu_stat_buf + ((hyp_vm_num + 1) * core_id) + hyp_vm_num) * 10000, total_count);
        seq_printf(m, "CPU%d\t%u.%02u%%", core_id, percentage / 100, percentage % 100);
        idle_avg += percentage;
        for (vm_id = 0; vm_id < hyp_vm_num; vm_id++)
        {
            percentage =
                (int)div64_u64(*(hyp_cpu_stat_buf + ((hyp_vm_num + 1) * core_id) + vm_id) * 10000, total_count);
            seq_printf(m, "\t%u.%02u%%", percentage / 100, percentage % 100);
        }

        seq_printf(m, "\n");
    }

    idle_avg /= hyp_cpu_num;
    seq_printf(m, "AVG\t%u.%02u%%\n", idle_avg / 100, idle_avg % 100);

    iounmap(hyp_cpu_stat_buf);

    return 0;
}

static ssize_t hyp_info_write(struct file *file, const char __user *buf, size_t len, loff_t *pos)
{
    char code[128] = {0};

    if (len > sizeof(code))
    {
        printk(KERN_ERR "command len is to long!\n");
        return len;
    }

    if (!len)
    {
        return len;
    }

    if (copy_from_user(code, buf, len))
    {
        return -EFAULT;
    }
    code[len - 1] = '\0';

    if (!strncmp(code, "intrstat", 8))
    {
        signal_hyp(HYPERCALL_FID_SHOW_INFO, HYPERCALL_SUBCMD_SHOW_INFO_INT_STAT, 0, 0);
    }
    else if (!strncmp(code, "vmstat", 6))
    {
        signal_hyp(HYPERCALL_FID_SHOW_INFO, HYPERCALL_SUBCMD_SHOW_INFO_VM_STAT, 0, 0);
    }
    else if (!strncmp(code, "esrstat", 7))
    {
        signal_hyp(HYPERCALL_FID_SHOW_INFO, HYPERCALL_SUBCMD_SHOW_INFO_ESR_STAT, 0, 0);
    }
    else if (!strncmp(code, "vgicstat", 8))
    {
        signal_hyp(HYPERCALL_FID_SHOW_INFO, HYPERCALL_SUBCMD_SHOW_INFO_VGIC_STAT, 0, 0);
    }
    else if (!strncmp(code, "boot_done", 9))
    {
        signal_hyp(HYPERCALL_FID_VM_STATUS, HYPERCALL_SUBCMD_VM_STATUS_SET_READY, 0, 0);
    }
    else if (!strncmp(code, "interval", 8))
    {
        measure_interval_ms = CamOsStrtoul(code + 9, NULL, 10);
        if (measure_interval_ms < 20)
            measure_interval_ms = 20;
        else if (measure_interval_ms > 120000)
            measure_interval_ms = 120000;
    }

    return len;
}

const struct seq_operations hyp_info_op = {.start = c_start, .next = c_next, .stop = c_stop, .show = c_show_hyp};

static int hyp_info_open(struct inode *inode, struct file *file)
{
    int              res = -ENOMEM;
    struct seq_file *seq;

    res = seq_open(file, &hyp_info_op);
    if (res)
        return res;
    seq = file->private_data;
    // seq->private = (void *)_rtklin;
    return res;
}
#endif

static int rtklog_open(struct inode *inode, struct file *file)
{
    return seq_open(file, &rsqlog_op);
}

static int rtkcrashlog_open(struct inode *inode, struct file *file)
{
    int              res = -ENOMEM;
    struct seq_file *seq;

    res = seq_open(file, &crashlog_op);
    if (res)
        return res;
    seq = file->private_data;
    return res;
}

static const struct proc_ops proc_rsqinfo_operations = {
    .proc_open    = rsqinfo_open,
    .proc_read    = seq_read,
    .proc_write   = rsqinfo_write,
    .proc_lseek   = seq_lseek,
    .proc_release = seq_release,
};

static const struct proc_ops proc_rtkinfo_operations = {
    .proc_open    = rtkinfo_open,
    .proc_read    = seq_read,
    .proc_write   = rtkinfo_write,
    .proc_lseek   = seq_lseek,
    .proc_release = seq_release,
};

#if defined(CONFIG_LINUX_ON_SS_HYPERVISOR)
static const struct proc_ops proc_hyp_info_operations = {
    .proc_open    = hyp_info_open,
    .proc_read    = seq_read,
    .proc_write   = hyp_info_write,
    .proc_lseek   = seq_lseek,
    .proc_release = seq_release,
};
#endif

static const struct proc_ops proc_rtklog_operations = {
    .proc_open    = rtklog_open,
    .proc_read    = seq_read,
    .proc_lseek   = seq_lseek,
    .proc_release = seq_release,
};

static const struct proc_ops proc_rtkcrashlog_operations = {
    .proc_open    = rtkcrashlog_open,
    .proc_read    = seq_read,
    .proc_lseek   = seq_lseek,
    .proc_release = seq_release,
};

unsigned long do_interos_call(u32 arg0, u32 arg1, u32 arg2, u32 arg3)
{
    u32 ret = 0;
    u32 i;

#ifdef CONFIG_SS_SWTOE
    if (arg0 == INTEROS_SC_L2R_SWTOE)
    {
        extern int drv_swtoe_cb_hdl(u32 arg0, u32 arg1, u32 arg2, u32 arg3);
        drv_swtoe_cb_hdl(type, arg1, arg2, arg3);
        return ret;
    }
#endif

    CamOsMutexLock(&gSyscallEntryLock);
    for (i = 0; i < SYSCALL_ENTRY_MAX; i++)
    {
        if (gInterosSyscallEntry[i].id == arg0 && gInterosSyscallEntry[i].func)
        {
            ret = gInterosSyscallEntry[i].func(arg0, arg1, arg2, arg3);
#ifdef __ADV_HYP_PROF__
            gInterosSyscallEntry[i].count++;
#endif
        }
    }
    CamOsMutexUnlock(&gSyscallEntryLock);

    return ret;
}

#if defined(CONFIG_SSTAR_RPMSG)
int dualos_interos_recv(struct dualos_interos_node *dnode, struct dualos_interos_datagram *data, unsigned int *src,
                        signed long timeout)
{
    struct dualos_interos_datagram *resp;
    struct sk_buff *                skb;
    unsigned long                   flags;
    signed long                     ret = timeout;
    DEFINE_WAIT(wait);

    spin_lock_irqsave(&dnode->queue_lock, flags);
    if (skb_queue_empty(&dnode->queue))
    {
        prepare_to_wait(&dnode->wq, &wait, TASK_INTERRUPTIBLE);
        spin_unlock_irqrestore(&dnode->queue_lock, flags);

        ret = schedule_timeout(timeout);
        finish_wait(&dnode->wq, &wait);
        spin_lock_irqsave(&dnode->queue_lock, flags);
    }
    skb = skb_dequeue(&dnode->queue);
    spin_unlock_irqrestore(&dnode->queue_lock, flags);

    if (!skb)
    {
        if (ret == 0)
            return -ETIMEDOUT;
        return -EAGAIN;
    }
    resp = (struct dualos_interos_datagram *)skb->data;
    if (data)
        memcpy(data, resp, sizeof(*data));
    if (src)
        *src = *((unsigned int *)((unsigned char *)resp + sizeof(*resp)));
    kfree_skb(skb);
    return 0;
}

static int interos_call_receiver(void *unused)
{
    struct dualos_interos_datagram req;
    struct dualos_interos_datagram resp;
    int                            ret;
    unsigned int                   src;
    struct dualos_interos_node *   dnode;
    int                            addr =
        EPT_ADDR_PREFIX(EPT_TYPE_SIGMASTAR, EPT_SOC_DEFAULT, EPT_OS_LINUX) | EPT_CLASS(EPT_CLASS_RESERVED) | 0x31;

    dnode = dualos_interos_node_alloc(addr, RPMSG_ADDR_ANY);
    if (IS_ERR_VALUE(dnode))
    {
        pr_err("interos_call_receiver: dualos_interos_node_alloc return %p\n", dnode);
        return PTR_ERR(dnode);
    }

    while (1)
    {
        ret = dualos_interos_recv(dnode, &req, &src, INTEROS_TIMEOUT);
        if (ret != 0)
            continue;

        memset(&resp, 0, sizeof(resp));
        resp.header.version = SSTAR_RPMSG_DUALOS_VERSION;
        resp.header.index   = req.header.index;
        resp.arg0           = do_interos_call(req.arg0, req.arg1, req.arg2, req.arg3);

        ret = dualos_interos_sendto(dnode, src, (char *)&resp, sizeof(resp), TX_TIMEOUT);
        if (ret)
        {
            pr_err(
                "interos_call_receiver: index=0x%x,"
                "req.arg0=0x%x,req.arg1=0x%x,req.arg2=0x%x,"
                "req.arg3=0x%x,"
                "resp.arg0=0x%x,resp.arg1=0x%x,resp.arg2=0x%x,"
                "resp.arg3=0x%x => %d\n",
                req.header.index, req.arg0, req.arg1, req.arg2, req.arg3, resp.arg0, resp.arg1, resp.arg2, resp.arg3,
                ret);
        }
    }

    return 0;
}

static int interos_nblk_call_receiver(void *unused)
{
    struct dualos_interos_datagram req;
    int                            ret;
    struct dualos_interos_node *   dnode;
    int                            addr =
        EPT_ADDR_PREFIX(EPT_TYPE_SIGMASTAR, EPT_SOC_DEFAULT, EPT_OS_LINUX) | EPT_CLASS(EPT_CLASS_RESERVED) | 0x32;

    dnode = dualos_interos_node_alloc(addr, RPMSG_ADDR_ANY);
    if (IS_ERR_VALUE(dnode))
    {
        pr_err("interos_nblk_call_receiver: dualos_interos_node_alloc return %p\n", dnode);
        return PTR_ERR(dnode);
    }

    while (1)
    {
        ret = dualos_interos_recv(dnode, &req, NULL, INTEROS_TIMEOUT);
        if (ret)
            continue;

        do_interos_call(req.arg0, req.arg1, req.arg2, req.arg3);
    }

    return 0;
}
#endif

static int epoch_init(void)
{
#ifndef CONFIG_DISABLE_DUALOS_NODE
    char proc[32];
#endif
    long               share;
    struct rlink_head *rl;

#if defined(CONFIG_LINUX_ON_SS_HYPERVISOR)
    proc_mkdir("hyp", NULL);
    snprintf(proc, sizeof(proc), "hyp/info");
    proc_create(proc, 0, NULL, &proc_hyp_info_operations);
#endif

    CamOsMutexInit(&gSyscallEntryLock);

    if (disable_rtos)
    {
        return 0;
    }

    if (NULL == driver_find("sstar-lh-rtos-virtio", &platform_bus_type))
    {
        pr_err("[dualos] %s: sstar-lh-rtos-virtio driver not found, disable rtos\n", __FUNCTION__);
        disable_rtos = 1;
        return 0;
    }
    pr_info("[dualos] wait dualos announce...");
#if defined(CONFIG_SSTAR_RPMSG)
#if defined(CONFIG_ARCH_INFINITY7) && defined(CONFIG_SSTAR_CA7_VIRTIO)
    if (dualos_rpmsg_wait_remote_device(RPMsg_Device_CA7, 0, 5000))
    {
        pr_err("[dualos] wait for remote dualos announce timeout!");
        disable_rtos = 1;
        return 0;
    }
#endif
#if defined(CONFIG_SSTAR_LH_RTOS_VIRTIO)
    if (dualos_rpmsg_wait_remote_device(RPMsg_Device_LH_RTOS, 0, 5000))
    {
        pr_err("[dualos] wait for remote dualos announce timeout!");
        disable_rtos = 1;
        return 0;
    }
#endif
#ifdef CONFIG_SS_PROFILING_TIME
    recode_timestamp(__LINE__, "dualos_announce_recv");
#endif
    pr_info("[dualos] dualos announce received!");

    default_dnode = dualos_interos_node_alloc(RPMSG_ADDR_ANY, RPMSG_ADDR_ANY);
    if (IS_ERR_VALUE(default_dnode))
    {
        int ret = PTR_ERR(default_dnode);

        default_dnode = NULL;
        panic("[dualos] dualos_interos_node_alloc return %d\n", ret);
        return ret;
    }
    default_dnode->remote_addr =
        EPT_ADDR_PREFIX(EPT_TYPE_SIGMASTAR, EPT_SOC_DEFAULT, EPT_OS_RTOS) | EPT_CLASS(EPT_CLASS_RESERVED) | 0x30;
#endif

#ifndef CONFIG_DISABLE_DUALOS_NODE
    proc_mkdir("dualos", NULL);
#endif

    /* get RTOS info */
    share = (long)signal_rtos(INTEROS_SC_L2R_HANDSHAKE, (u32)0, (u32)0, (u32)0);
    if (share == (long)-1)
    {
        panic("[dualos] failed to get share memory (0x%lx)\n", share);
        return 0;
    }

#ifdef CONFIG_SS_AMP
    _signal_1st_using_mbox = 0;
#endif
#if defined(CONFIG_ARCH_INFINITY7) && defined(CONFIG_ENABLE_CA7)
    share = share + (CONFIG_CA7_MEM_PHY_ADDR - MIU0_LOW_BASE);
#endif
    _rtkres = request_mem_region(share, SHARE_SIZE, "dualos");
    _rtklin = (rtkinfo_t *)ioremap_wc(_rtkres->start, resource_size(_rtkres));

#if defined(CONFIG_SS_AMP) || (defined(CONFIG_LH_RTOS) && defined(CONFIG_SMP))
    intercore_sem_init((unsigned long)_rtklin + SHARE_SIZE - SW_SEM_LOCK_TOTAL_NUM * sizeof(intercore_sem_t));
#endif

    if (INTEROS_CALL_SHMEM_PARAM_SIZE < sizeof(interos_call_args_t))
    {
        printk(KERN_ERR "[dualos] Error!! interos shmem param address\n");
        *(int *)0 = 0;
    }

    if (!_rtklin || _rtklin->size != sizeof(*_rtklin) || _rtklin->verid != RSQ_VERSION_ID)
    {
#ifndef CONFIG_DISABLE_DUALOS_NODE
        proc_create("dualos/version_not_match", 0, NULL, &proc_rtkinfo_operations);
#endif
        printk(KERN_ERR "[dualos] Error!! RTOS version not match\n");
        _rtklin = NULL;
        return 0;
    }
#ifndef CONFIG_DISABLE_DUALOS_NODE
    snprintf(proc, sizeof(proc), "dualos/%s", _rtklin->name);
    proc_create(proc, 0, NULL, &proc_rtkinfo_operations);
    snprintf(proc, sizeof(proc), "dualos/log");
    proc_create(proc, 0x0444, NULL, &proc_rtklog_operations);
    if (get_rtkinfo())
    {
        if (get_rtkinfo()->crash_log_buf_addr && get_rtkinfo()->crash_log_buf_size)
        {
            snprintf(proc, sizeof(proc), "dualos/crashlog");
            proc_create(proc, 0x0444, NULL, &proc_rtkcrashlog_operations);
        }
        if (get_rtkinfo()->core_dump_remote_addr && get_rtkinfo()->core_dump_remote_size)
        {
            snprintf(proc, sizeof(proc), "dualos/coredump");
            proc_create(proc, 0x0444, NULL, &proc_rtkcrdmprmt_operations);
        }
    }

    CamInterOsSignalReg(INTEROS_SC_R2L_CLI_EXEC_DONE, _RtosCliExecDoneCb, "RtosCliExecDoneCb");
#endif
    /* */
    for (rl = &(_rtklin->root); rl->nphys && rl->nsize;)
    {
        struct sstream sst;

#if defined(CONFIG_ARCH_INFINITY7) && defined(CONFIG_ENABLE_CA7)
        sst.res = request_mem_region((long)rl->nphys + (CONFIG_CA7_MEM_PHY_ADDR - MIU0_LOW_BASE), rl->nsize, "rsq");
#else
        sst.res = request_mem_region((long)rl->nphys, rl->nsize, "rsq");
#endif
        sst.rvr = (reservoir_t *)ioremap_wc(sst.res->start, resource_size(sst.res));
        rename_region(sst.res, sst.rvr->name);
        init_rsq_rcvr(sst.rvr);
#ifndef CONFIG_DISABLE_DUALOS_NODE
        snprintf(proc, sizeof(proc), "dualos/%s", sst.rvr->name);
        proc_create(proc, 0, NULL, &proc_rsqinfo_operations);
#endif
        sst.id = sst.rvr->iid;
        if (strncmp(sst.rvr->name, "log", 3) == 0)
            sst.private = (void *)&rsqlog_op; // set log read op
        _sstr[sst.id] = sst;
        rl            = &(sst.rvr->link);
    }
    _rtklin->diff = (u64)share - (u64)(unsigned long)_rtklin; // offset for address transfer

    /* create a thread to get interos call */
    //_interos_call_pid = kernel_thread(interos_call_receiver, NULL, CLONE_FS | CLONE_FILES);
    _interos_call_pid = kthread_run(interos_call_receiver, NULL, "interos");

#if ENABLE_NBLK_CALL
    //_interos_nblk_call_pid = kernel_thread(interos_nblk_call_receiver, NULL, CLONE_FS | CLONE_FILES);

    _interos_nblk_call_pid = kthread_run(interos_nblk_call_receiver, NULL, "interos_nblk");

#endif

    /* for debug */
    signal_rtos(INTEROS_SC_L2R_RSQ_INIT, (u32)(unsigned long)_rtklin, (u32)_rtklin->diff, 0);
    return 0;
}

/*
 * kerenl file I/O
 */
struct file *file_open(const char *path, int flags, int rights)
{
    struct file *filp = NULL;
    mm_segment_t oldfs;
    int          err = 0;

    oldfs = get_fs();
    set_fs(KERNEL_DS);
    filp = filp_open(path, flags, rights);
    set_fs(oldfs);
    if (IS_ERR(filp))
    {
        err = PTR_ERR(filp);
        return NULL;
    }
    return filp;
}

void file_close(struct file *file)
{
    filp_close(file, NULL);
}

int file_read(struct file *file, unsigned long long offset, unsigned char *data, unsigned int size)
{
    mm_segment_t oldfs;
    int          ret;

    oldfs = get_fs();
    set_fs(KERNEL_DS);

    ret = vfs_read(file, data, size, &offset);

    set_fs(oldfs);
    return ret;
}

int file_write(struct file *file, unsigned long long offset, unsigned char *data, unsigned int size)
{
    mm_segment_t oldfs;
    int          ret;

    oldfs = get_fs();
    set_fs(KERNEL_DS);

    ret = vfs_write(file, data, size, &offset);

    set_fs(oldfs);
    return ret;
}

int file_sync(struct file *file)
{
    vfs_fsync(file, 0);
    return 0;
}

rtkinfo_t *get_rtkinfo(void)
{
    return _rtklin;
}
EXPORT_SYMBOL(get_rtkinfo);

bool dualos_rtos_exist(void)
{
    return (_rtklin == NULL) ? FALSE : TRUE;
}
EXPORT_SYMBOL(dualos_rtos_exist);

#if defined(CONFIG_SSTAR_RPMSG)
unsigned long signal_rtos(u32 type, u32 arg1, u32 arg2, u32 arg3)
{
    struct dualos_interos_datagram resp;
    int                            ret;

    if (get_rtkinfo() && get_rtkinfo()->has_dead)
    {
        printk(KERN_ERR "Rtos has dead-3\n");
        return (unsigned long)-1;
    }

    if (default_dnode == NULL)
        return (unsigned long)-1;

    ret = dualos_interos_call(default_dnode, type, arg1, arg2, arg3, &resp);
    if (ret < 0)
        return (unsigned long)-1;
    return resp.arg0;
}
#else
unsigned long signal_rtos(u32 type, u32 arg1, u32 arg2, u32 arg3)
{
    return 0;
}
#endif

s32 CamInterOsSignalReg(u32 id, void *func, const char *name)
{
    u32 i;
    s32 ret = -1;

    CamOsMutexLock(&gSyscallEntryLock);
    for (i = 0; i < SYSCALL_ENTRY_MAX; i++)
    {
        if (!gInterosSyscallEntry[i].id && !gInterosSyscallEntry[i].func)
        {
            gInterosSyscallEntry[i].id   = id;
            gInterosSyscallEntry[i].func = func;
#ifdef __ADV_HYP_PROF__
            gInterosSyscallEntry[i].count = 0;
            snprintf(gInterosSyscallEntry[i].name, SYSCALL_NAME_LEN_MAX, "%s", name);
#endif
            ret = 0;
            break;
        }
    }

    CamOsMutexUnlock(&gSyscallEntryLock);
    return ret;
}
EXPORT_SYMBOL(CamInterOsSignalReg);

s32 CamInterOsSignalDereg(u32 id, void *func)
{
    u32 i;
    s32 ret = -1;

    CamOsMutexLock(&gSyscallEntryLock);
    for (i = 0; i < SYSCALL_ENTRY_MAX; i++)
    {
        if (gInterosSyscallEntry[i].id == id && gInterosSyscallEntry[i].func == func)
        {
            gInterosSyscallEntry[i].id   = 0;
            gInterosSyscallEntry[i].func = NULL;
#ifdef __ADV_HYP_PROF__
            gInterosSyscallEntry[i].count = 0;
            memset(gInterosSyscallEntry[i].name, 0, SYSCALL_NAME_LEN_MAX);
#endif
            ret = 0;
            break;
        }
    }

    CamOsMutexUnlock(&gSyscallEntryLock);
    return ret;
}

int CamInterOsSignal(unsigned int id, unsigned int arg0, unsigned int arg1, unsigned int arg2)
{
    return signal_rtos(id, arg0, arg1, arg2);
}

EXPORT_SYMBOL(CamInterOsSignal);

int CamInterOsSignalAsync(unsigned int id, unsigned int arg0, unsigned int arg1, unsigned int arg2)
{
    printk(KERN_ERR "CamInterOsSignalAsync not supported in linux\n");
    return 0;
}

unsigned int CamInterOsRequestLock(void)
{
    return intercore_sem_lock_request();
}

void CamInterOsFreeLock(unsigned int sem_id)
{
    if (sem_id)
        intercore_sem_lock_free(sem_id);

    return;
}

void CamInterOsLock(unsigned int *lock, unsigned int sem_id)
{
#if defined(CONFIG_SS_AMP) // Multi-core AMP
    unsigned long cpu_sr = 0;
    local_irq_save(cpu_sr);
    *lock = cpu_sr;
    intercore_sem_lock(sem_id);
#elif defined(CONFIG_LH_RTOS)
#ifdef CONFIG_SMP // Multi-core SMPLH
    unsigned long cpu_sr = 0;
    if (get_cpu() == 0)
    {
        local_fiq_disable();
    }
    local_irq_save(cpu_sr);
    *lock = cpu_sr;
    intercore_sem_lock(sem_id);
#else             // Single-core LH
    local_fiq_disable();
#endif
#endif
}

void CamInterOsUnlock(unsigned int *lock, unsigned int sem_id)
{
#if defined(CONFIG_SS_AMP) // Multi-core AMP
    unsigned long cpu_sr = 0;
    intercore_sem_unlock(sem_id);
    cpu_sr = *lock;
    local_irq_restore(cpu_sr);
#elif defined(CONFIG_LH_RTOS)
#ifdef CONFIG_SMP // Multi-core SMPLH
    unsigned long cpu_sr = 0;
    intercore_sem_unlock(sem_id);
    cpu_sr = *lock;
    local_irq_restore(cpu_sr);
    if (smp_processor_id() == 0)
    {
        local_fiq_enable();
        put_cpu();
    }
#else             // Single-core LH
    local_fiq_enable();
#endif
#endif
}

fs_initcall(epoch_init);
