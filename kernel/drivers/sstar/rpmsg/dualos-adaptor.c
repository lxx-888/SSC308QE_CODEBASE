/*
 * dualos-adaptor.c- Sigmastar
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
#include <linux/module.h>
#include <linux/device.h>
#include <linux/major.h>
#include <linux/fs.h>
#include <linux/arm-smccc.h>
#include <linux/slab.h>
#include <linux/kernel.h>
#include <linux/uaccess.h>
#include <linux/version.h>
#include <linux/string.h>
#include <linux/mm.h>
#include <linux/mman.h>
#include <linux/semaphore.h>
#include <linux/atomic.h>
#include <asm/pgtable.h>
#include <asm/io.h>
#include <linux/seq_file.h>
#include <linux/proc_fs.h>
#include <linux/poll.h>
#include <linux/skbuff.h>
#include <linux/delay.h>
#include <linux/kthread.h>
#include <linux/platform_device.h>
#include "drv_dualos.h"
#include "cam_inter_os.h"
#include "ms_platform.h"
#include "rpmsg_dualos.h"

#include "mi_common_datatype.h"
#include "mi_sys_datatype.h"
#include "sys_ioctl.h"
#include "mi_sys_buf_mgr.h"
#include "mi_sys_internal.h"
#include "mi_sys_impl.h"

#define __MI_DEVICE_PROC             0xfffffffful
#define __MI_DEVICE_PROC_IO          0xfffffffeul
#define __MI_DEVICE_PROC_CONT        0xfffffffdul
#define __MI_DEVICE_ADDR_QUERY       0xfffffffcul
#define __MI_DEVICE_MMA_INFO         0xfffffffbul
#define __MI_DEVICE_PROC_POLL        0xfffffffaul
#define __MI_DEVICE_CONNECT          0
#define __MI_DEVICE_DISCONNECT       1
#define __MI_DEVICE_QUERY            2
#define __MI_DEVICE_POLL_CREATE      3
#define __MI_DEVICE_POLL_RELEASE     4
#define __MI_DEVICE_POLL_STATE       5
#define __MI_DEVICE_POLL_START       6
#define INTEROS_SC_L2R_MI_MMA        (0xffffff87)
#define INTEROS_SC_R2L_MI_MMA        (0xffffff88)
#define INTEROS_SC_R2L_MI_NOTIFY     (0xffffff89)
#define INTEROS_SC_R2L_DUMPBUF_DATA  0xfffffff0ul
#define INTEROS_SC_R2L_DUMPBUF_OPEN  0xfffffff1ul
#define INTEROS_SC_R2L_DUMPBUF_CLOSE 0xfffffff2ul
#define INTEROS_SC_R2L_DUMPBUF_WRITE 0xfffffff3ul
#define INTEROS_SC_R2L_PROC_DEVADD   0xfffffff4ul
#define INTEROS_SC_R2L_PROC_DEVDEL   0xfffffff5ul
#define __MI_SYS_BUF_HANDLE_QUERY    0xfffffff6ul

#define REMOTE_ADAPTOR_MANAGER_EPT_ADDR \
    (EPT_ADDR_PREFIX(EPT_TYPE_SIGMASTAR, EPT_SOC_DEFAULT, EPT_OS_RTOS) | EPT_CLASS(EPT_CLASS_RESERVED) | 0x401)

#define INTEROS_TIMEOUT           (HZ * 5) // 5s
#define TX_TIMEOUT                (1000)   // 1s
#define MAX_PROC_LIST_ENTRY_SIZE  200
#define MAX_ENTRY_PATH_LEN        62
#define MAX_DUMPBUF_DATA_PATHNAME 128

#define DATA_CACHE_ENTRY_SIZE 64 // 64 bytes per data cache

static DEFINE_MUTEX(device_mtx);

#define MI_PROC_FAKE_ID    (E_MI_MODULE_ID_MAX + 1)
#define MI_PROC_DEV_ID_MAX (10)

#define MI_CLI_FAKE_ID (E_MI_MODULE_ID_MAX + 2)

#define ALIGN_UP(x, align) (((x) + ((align)-1)) & ~((align)-1))

#define MMA_MAP_HASH_BITS 8
static CAM_OS_DEFINE_HASHTABLE(g_mma_map_addr_hash, MMA_MAP_HASH_BITS);
CamOsTsem_t g_mma_map_hash_semphore;

typedef struct mma_map_record_s
{
    struct CamOsHListNode_t hentry;
    s32                     pid;
    int                     mod_id;
    void *                  mma_cache_vaddr;
    void *                  mma_nocache_vaddr;
} mma_map_record_t;

struct dualos_adaptor_node
{
    struct rpmsg_endpoint *ept;

    spinlock_t          queue_lock;
    struct sk_buff_head queue;

    wait_queue_head_t wq;
    void *            mma_cache_vaddr;
    void *            mma_nocache_vaddr;
    int               bcache;

    int mod_id;
    int dev_id;
    int chn_id;

    int          remote_addr;
    unsigned int index;

    struct mutex mutex;

    void *private;
};

#define LINUX_CTX_DATA                        \
    struct                                    \
    {                                         \
        union                                 \
        {                                     \
            int           pid;                \
            unsigned long cmd;                \
            unsigned long poll_handle;        \
        };                                    \
        void *curr_cache_vaddrbase;           \
        void *curr_nocache_vaddrbase;         \
        long  idx;                            \
        char  entry_path[MAX_ENTRY_PATH_LEN]; \
        union                                 \
        {                                     \
            unsigned long mma_base;           \
            unsigned long log_base;           \
        };                                    \
        unsigned int arg_size;                \
    };

typedef struct
{
    LINUX_CTX_DATA
} linux_ctx_nopadding __attribute__((aligned(DATA_CACHE_ENTRY_SIZE)));

typedef struct
{
    LINUX_CTX_DATA
    char __pad[DATA_CACHE_ENTRY_SIZE - ((sizeof(linux_ctx_nopadding)) % DATA_CACHE_ENTRY_SIZE)];
} linux_ctx __attribute__((aligned(DATA_CACHE_ENTRY_SIZE)));

struct proc_EntryNode
{
    unsigned char dir;
    char          depth;
    char          entry_path[MAX_ENTRY_PATH_LEN];
};

struct proc_EntryList
{
    int                   Length;
    struct proc_EntryNode eNode[MAX_PROC_LIST_ENTRY_SIZE];
} __attribute__((aligned(DATA_CACHE_ENTRY_SIZE)));

struct proc_buffer
{
    void *buf;
    int   size;
    int   used;
};

typedef struct DRV_PROC_DumpBuf_data_s
{
    char   pathname[MAX_DUMPBUF_DATA_PATHNAME];
    MI_U32 u32BufAddr;
    MI_U32 u32BufSize;
} DRV_PROC_DumpBuf_data_t;

#define POLL_FILE_MAX 64
typedef enum
{
    E_MI_COMMON_POLL_NOT_READY         = (0x0),
    E_MI_COMMON_FRAME_READY_FOR_READ   = (0x1 << 0),
    E_MI_COMMON_BUFFER_READY_FOR_WRITE = (0x1 << 1),
} MI_COMMON_PollFlag_e;

static struct mutex proc_mutex;

static int disable_os_adaptor = 0;
#if 1
static int __init disable_os_adaptor_func(char *str)
{
    disable_os_adaptor = simple_strtol(str, NULL, 10);
    return 0;
}
// early_param("disable_os_adaptor", disable_os_adaptor_func);
early_param("disable_rtos", disable_os_adaptor_func);
#else
module_param(disable_os_adaptor, int, 0644);
MODULE_PARM_DESC(disable_os_adaptor, "Disable Linux-RTOS Adaptor");
#endif

extern void recode_timestamp(int mark, const char *name);

static int dualos_adaptor_interos(struct dualos_adaptor_node *anode, unsigned int arg0, unsigned int arg1,
                                  unsigned int arg2, unsigned int arg3);

static int dualos_adaptor_interos_to(struct dualos_adaptor_node *anode, int dst, unsigned int arg0, unsigned int arg1,
                                     unsigned int arg2, unsigned int arg3, struct dualos_interos_datagram *data);

static struct dualos_adaptor_node *dualos_adaptor_node_alloc(int mod_id, int dev_id, int chn_id);

static void dualos_adaptor_node_release(struct dualos_adaptor_node *anode);

static int dualos_adaptor_cb(struct rpmsg_device *rpdev, void *data, int len, void *priv, u32 src)
{
    struct dualos_adaptor_node *anode = priv;
    struct sk_buff *            skb;

    skb = alloc_skb(len, GFP_ATOMIC);
    if (!skb)
        return -ENOMEM;

    memcpy(skb_put(skb, len), data, len);

    spin_lock(&anode->queue_lock);
    skb_queue_tail(&anode->queue, skb);
    spin_unlock(&anode->queue_lock);

    wake_up_all(&anode->wq);
    return 0;
}

/*
 * from ARM Architecture Reference Manual
 *            ARMv7-A and ARMv7-R edition
 * B3.18.6 Cache maintenance operations, functional group, VMSA
 * Table B3-49 Cache and branch predictor maintenance operations, VMSA
 */
static void flush_cache_area(void *ptr, int len)
{
    const unsigned long cache_line_size = 64;
    unsigned long       iter, end;
    iter = (unsigned long)ptr, end = (unsigned long)ptr + len;
    iter = iter / cache_line_size * cache_line_size;
    end  = end / cache_line_size * cache_line_size;
    asm __volatile__("dsb st" ::: "memory"); /* data sync barrier for store */
    while (iter <= end)
    {
        // asm __volatile__("mcr p15, 0, %0, c7, c11, 1"::"r"(iter):"memory"); /* DCCMVAC: flush to PoU (aka last level
        // cache) */
        asm __volatile__("mcr p15, 0, %0, c7, c10, 1" ::"r"(iter)
                         : "memory"); /* DCCMVAU: flush to PoC (aka main memory) */
        iter += cache_line_size;
    }
}
static void invalid_cache_area(void *ptr, int len)
{
    const unsigned long cache_line_size = 64;
    unsigned long       iter, end;
    iter = (unsigned long)ptr, end = (unsigned long)ptr + len;
    iter = iter / cache_line_size * cache_line_size;
    end  = end / cache_line_size * cache_line_size;
    while (iter <= end)
    {
        asm __volatile__("mcr p15, 0, %0, c7, c6, 1" ::"r"(iter) : "memory"); /* DCIMVAC: invalidate to PoC */
        iter += cache_line_size;
    }
}
static void flush_and_invalid_cache_area(void *ptr, int len)
{
    const unsigned long cache_line_size = 64;
    unsigned long       iter, end;
    iter = (unsigned long)ptr, end = (unsigned long)ptr + len;
    iter = iter / cache_line_size * cache_line_size;
    end  = end / cache_line_size * cache_line_size;
    asm __volatile__("dsb st" ::: "memory"); /* data sync barrier for store */
    while (iter <= end)
    {
        asm __volatile__("mcr p15, 0, %0, c7, c14, 1" ::"r"(iter)
                         : "memory"); /* DCCIMVAC: flush & invalid to PoC (aka main memory) */
        iter += cache_line_size;
    }
}

static struct class *        device_class;
static struct device *       device_list[E_MI_MODULE_ID_MAX + 1];
static struct device *       device_cli;
static struct proc_EntryList proc_list                                         = {0};
static int                   proc_node[E_MI_MODULE_ID_MAX][MI_PROC_DEV_ID_MAX] = {0};
static int                   device_chrdev[E_MI_MODULE_ID_MAX]                 = {0};
static int                   device_major, poll_major;
static struct resource *     rtk_res;

static struct dualos_adaptor_node *module_anodes[E_MI_MODULE_ID_MAX];
static DEFINE_MUTEX(module_anodes_mutex);

static struct task_struct *poll_manager_task = NULL;

static u32 dualos_adaptor_poll_notify(u32 arg0, u32 arg1, u32 arg2, u32 arg3);

static int dualos_adaptor_poll_manager(void *arg)
{
    struct dualos_interos_datagram *resp;
    struct sk_buff *                skb;
    unsigned long                   flags;
    struct dualos_adaptor_node *    anode = arg;
    DEFINE_WAIT(wait);

    if (!anode)
        return 0;

    while (1)
    {
        spin_lock_irqsave(&anode->queue_lock, flags);

        if (skb_queue_empty(&anode->queue))
        {
            prepare_to_wait(&anode->wq, &wait, TASK_UNINTERRUPTIBLE);
            spin_unlock_irqrestore(&anode->queue_lock, flags);

            schedule();
            finish_wait(&anode->wq, &wait);
            spin_lock_irqsave(&anode->queue_lock, flags);
        }

        skb = skb_dequeue(&anode->queue);
        spin_unlock_irqrestore(&anode->queue_lock, flags);

        if (!skb)
        {
            continue;
        }

        resp = (struct dualos_interos_datagram *)skb->data;
        dualos_adaptor_poll_notify(resp->arg0, resp->arg1, resp->arg2, resp->arg3);
        kfree_skb(skb);
    }

    return 0;
}

static unsigned long mma_base = 0x25500000;
static unsigned long mma_size = 0x02700000;

/*
  example:
  os_adaptor=mma_base=0x25500000,mma_size=0x02700000
  os_adaptor=mma_base=0x21F00000,mma_size=0x01D00000
*/
static bool parse_os_adaptor_config(char *cmdline, unsigned long *mma_base, unsigned long *mma_size)
{
    char *option;

    if (cmdline == NULL)
        goto INVALID_OS_ADAPTOR_CONFIG;

    option = strstr(cmdline, "mma_size=");
    if (option == NULL)
        goto INVALID_OS_ADAPTOR_CONFIG;
    option = strstr(cmdline, "mma_base=");
    if (option == NULL)
        goto INVALID_OS_ADAPTOR_CONFIG;
    sscanf(option, "mma_base=%lx,mma_size=%lx", mma_base, mma_size);

    return true;

INVALID_OS_ADAPTOR_CONFIG:

    return false;
}

int __init setup_dualos_adaptor(char *cmdline)
{
    if (!parse_os_adaptor_config(cmdline, &mma_base, &mma_size))
    {
        printk(KERN_ERR "error: os_adaptor args invalid\n");
    }
    return 0;
}
early_param("os_adaptor", setup_dualos_adaptor);

static atomic_t ctx_cost[sizeof(unsigned long) * 8] = {};
static atomic_t ctx_freq                            = {};

static struct proc_dir_entry *debug_tools;
// static struct proc_dir_entry *proc_root;

struct debug_tool
{
    struct proc_dir_entry *entry;
    void *                 obj;
    ssize_t (*write)(void *obj, const char **args, int count);
    ssize_t (*read)(void *obj, struct seq_file *q, void *v);
};
struct debug_tool_freq
{
    struct debug_tool dt;
    int               interval;
};
struct debug_tool_info
{
    struct debug_tool dt;
    const char *      version;
};
static ssize_t ctx_cost_erase(void *obj, const char **args, int count)
{
    atomic_t *cost = ctx_cost;
    int       i;

    for (i = 0; i < sizeof(unsigned long) * 8; ++i)
    {
        atomic_set(&cost[i], 0);
    }
    return 0;
}
static ssize_t ctx_cost_hist(void *obj, struct seq_file *q, void *v)
{
    atomic_t *    cost = ctx_cost;
    int           i;
    unsigned char buf[64];
    unsigned long rval;
    for (i = 0; i < sizeof(unsigned long) * 8; ++i)
    {
        rval = snprintf(buf, sizeof(buf), "CTX|%02d:%d\n", i, atomic_read(&cost[i]));
        seq_write(q, buf, rval);
    }
    return 0;
}
static ssize_t ctx_cost_freq_setup(void *obj, const char **args, int count)
{
    struct debug_tool_freq *dtf = obj;
    if (count == 1)
    {
        int interval;

        if (kstrtoint((const char *)args[0], 0, &interval))
            return -EFAULT;
        if (interval < 1)
            return -EFAULT;
        dtf->interval = interval;
        printk("freq watch interval=%dms\n", dtf->interval);
        return count;
    }
    return -EINVAL;
}
static ssize_t ctx_cost_freq(void *obj, struct seq_file *q, void *v)
{
    atomic_t *              freq = &ctx_freq;
    struct debug_tool_freq *dtf  = obj;

    atomic_xchg(freq, 0);
    while (schedule_timeout_interruptible(msecs_to_jiffies(dtf->interval)) == 0)
    {
        char          buf[64] = {0};
        unsigned long rval;
        unsigned long tmp;
        tmp  = atomic_xchg(freq, 0);
        rval = snprintf(buf, sizeof(buf), "CTX_FREQ:%8lu\n", tmp);
        seq_write(q, buf, rval);
        break;
    }
    return 0;
}

static ssize_t compile_version_info(void *obj, struct seq_file *q, void *v)
{
    struct debug_tool_info *dti     = obj;
    char                    buf[64] = {0};
    unsigned long           rval;
    rval = snprintf(buf, sizeof(buf), "version string:%s\n", dti->version);
    seq_write(q, buf, rval);
    return 0;
}

static struct debug_tool syscall_cost_column = {
    .write = ctx_cost_erase,
    .read  = ctx_cost_hist,
};
static struct debug_tool_freq syscall_freq_linear = {
    {
        .obj   = &syscall_freq_linear,
        .write = ctx_cost_freq_setup,
        .read  = ctx_cost_freq,
    },
    .interval = 1000,
};
static struct debug_tool_info info_tool = {
    {
        .obj  = &info_tool,
        .read = compile_version_info,
    },
    .version = "version",
};

static unsigned int time_log2(ktime_t start, ktime_t end)
{
    unsigned int  idx = 0;
    unsigned long us  = ktime_to_us(ktime_sub(end, start));
    us                = us >> 1;
    while (us)
    {
        idx = idx + 1;
        us  = us >> 1;
    }
    return idx;
}

typedef struct
{
    wait_queue_head_t           stPollHead;
    unsigned long               poll_handle;
    struct list_head            list;
    struct dualos_adaptor_node *anode;
} MI_COMMON_PollFileWrapper_t;
static LIST_HEAD(poll_task);
static spinlock_t poll_task_lock;

static int alkaid_poll_addr;

static u32 dualos_adaptor_poll_notify(u32 arg0, u32 arg1, u32 arg2, u32 arg3)
{
    MI_COMMON_PollFileWrapper_t *f;

    spin_lock(&poll_task_lock);
    list_for_each_entry(f, &poll_task, list)
    {
        if (f->poll_handle == arg2)
        {
            wake_up(&f->stPollHead);
            break;
        }
    }
    spin_unlock(&poll_task_lock);

    return true;
}

static unsigned long alkaid_poll_wrapper(struct dualos_adaptor_node *anode, MI_COMMON_PollFileWrapper_t *f, int cmd)
{
    unsigned long res = 0;
    linux_ctx     ctx = {
        .poll_handle = 0,
        .idx         = -1,
    };

    ctx.poll_handle = f ? f->poll_handle : 0;
    flush_cache_area(&ctx, sizeof(ctx));
    Chip_Flush_MIU_Pipe();
    res = dualos_adaptor_interos(anode, E_MI_MODULE_ID_MAX, __pa((long)&ctx), -1, cmd);

    return res;
}

static unsigned long alkaid_poll_init(void)
{
    struct dualos_adaptor_node *anode;
    int                         remote_addr;
    int                         dst = REMOTE_ADAPTOR_MANAGER_EPT_ADDR;
    int                         res;

    spin_lock_init(&poll_task_lock);
    anode = dualos_adaptor_node_alloc(E_MI_MODULE_ID_MAX, 0xf, 0xff);
    if (IS_ERR_VALUE(anode))
        return (int)anode;

    remote_addr = dualos_adaptor_interos_to(anode, dst, __MI_DEVICE_ADDR_QUERY, anode->mod_id, anode->dev_id,
                                            anode->chn_id, NULL);
    if (remote_addr < 0)
    {
        pr_err(
            "Adaptor node (%d,%d,%d,%d) get remote addr "
            "error %d\n",
            anode->mod_id, anode->dev_id, anode->chn_id, anode->remote_addr, remote_addr);
        res = -EINVAL;
        goto err;
    }
    pr_info("Adaptor node (%d,%d,%d,%d) get remote addr 0x%x\n", anode->mod_id, anode->dev_id, anode->chn_id,
            anode->remote_addr, remote_addr);
    anode->remote_addr = remote_addr;
    alkaid_poll_addr   = remote_addr;

    res = alkaid_poll_wrapper(anode, NULL, __MI_DEVICE_POLL_CREATE);
    if (res)
        goto err;

    poll_manager_task = kthread_create(dualos_adaptor_poll_manager, (void *)anode, "adaptor_poll");
    set_user_nice(poll_manager_task, MIN_NICE);
    wake_up_process(poll_manager_task);
    return res;
err:
    dualos_adaptor_node_release(anode);
    return res;
}

static int MI_PollAdp_Open(struct inode *inode, struct file *filp)
{
    MI_COMMON_PollFileWrapper_t *fw = kmalloc(sizeof(MI_COMMON_PollFileWrapper_t), GFP_KERNEL);
    if (fw)
    {
        struct dualos_adaptor_node *anode;

        anode = dualos_adaptor_node_alloc(E_MI_MODULE_ID_MAX, 0xf, 0xff);
        if (IS_ERR_VALUE(anode))
        {
            kfree(fw);
            return (int)anode;
        }
        anode->remote_addr = alkaid_poll_addr;

        fw->anode = anode;
        init_waitqueue_head(&fw->stPollHead);
        fw->poll_handle = 0;
        INIT_LIST_HEAD(&fw->list);
        filp->private_data = fw;
        return 0;
    }
    return -ENOMEM;
}

static int MI_PollAdp_Release(struct inode *inode, struct file *filp)
{
    MI_COMMON_PollFileWrapper_t *f     = filp->private_data;
    struct dualos_adaptor_node * anode = f->anode;

    spin_lock(&poll_task_lock);
    list_del(&f->list);
    spin_unlock(&poll_task_lock);

    alkaid_poll_wrapper(anode, f, __MI_DEVICE_POLL_RELEASE);
    if (f->anode)
        dualos_adaptor_node_release(f->anode);
    kfree(f);

    return 0;
}

static long MI_PollAdp_Ioctl(struct file *filp, unsigned int cmd, unsigned long ptr)
{
    MI_COMMON_PollFileWrapper_t *f          = filp->private_data;
    struct dualos_adaptor_node * anode      = f->anode;
    unsigned long                handle_bkp = f->poll_handle;
    MI_COMMON_PollFileWrapper_t *tmp;

    if (cmd != 0 || ptr == 0)
    {
        CamOsPrintf("AdpPoll ioctrl is invalid: 0x%x\n", cmd);
        return -1;
    }
    f->poll_handle = ptr;
    if (alkaid_poll_wrapper(anode, f, __MI_DEVICE_POLL_START))
    {
        f->poll_handle = handle_bkp;
        return -1;
    }
    spin_lock(&poll_task_lock);
    list_for_each_entry(tmp, &poll_task, list)
    {
        if (f == tmp)
        {
            spin_unlock(&poll_task_lock);
            return 0;
        }
    }
    list_add(&f->list, &poll_task);
    spin_unlock(&poll_task_lock);
    return 0;
}

static unsigned int MI_PollAdp_Poll(struct file *filp, poll_table *wait)
{
    MI_COMMON_PollFileWrapper_t *f          = filp->private_data;
    unsigned int                 req_events = poll_requested_events(wait);
    unsigned int                 mask       = 0;
    unsigned long                ret        = 0;
    struct dualos_adaptor_node * anode      = f->anode;

    poll_wait(filp, &f->stPollHead, wait);
    ret = alkaid_poll_wrapper(anode, f, __MI_DEVICE_POLL_STATE);

    if (ret & E_MI_COMMON_FRAME_READY_FOR_READ)
        mask |= POLLIN;
    if (ret & E_MI_COMMON_BUFFER_READY_FOR_WRITE)
        mask |= POLLOUT;

    return req_events & mask;
}

static int dualos_adaptor_parseModuleIdByName(char *name)
{
    if (strstr(name, "mi_ive"))
        return E_MI_MODULE_ID_IVE;
    else if (strstr(name, "mi_vdf"))
        return E_MI_MODULE_ID_VDF;
    else if (strstr(name, "mi_venc"))
        return E_MI_MODULE_ID_VENC;
    else if (strstr(name, "mi_rgn"))
        return E_MI_MODULE_ID_RGN;
    else if (strstr(name, "mi_ai"))
        return E_MI_MODULE_ID_AI;
    else if (strstr(name, "mi_ao"))
        return E_MI_MODULE_ID_AO;
    else if (strstr(name, "mi_vif"))
        return E_MI_MODULE_ID_VIF;
    else if (strstr(name, "mi_divp"))
        return E_MI_MODULE_ID_DIVP;
    else if (strstr(name, "mi_vpe"))
        return E_MI_MODULE_ID_VPE;
    else if (strstr(name, "mi_isp"))
        return E_MI_MODULE_ID_ISP;
    else if (strstr(name, "mi_scl"))
        return E_MI_MODULE_ID_SCL;
    else if (strstr(name, "mi_vdec"))
        return E_MI_MODULE_ID_VDEC;
    else if (strstr(name, "mi_sys") || strstr(name, "mi_common"))
        return E_MI_MODULE_ID_SYS;
    else if (strstr(name, "mi_fb"))
        return E_MI_MODULE_ID_FB;
    else if (strstr(name, "mi_hdmi"))
        return E_MI_MODULE_ID_HDMI;
    else if (strstr(name, "mi_gfx"))
        return E_MI_MODULE_ID_GFX;
    else if (strstr(name, "mi_vdisp"))
        return E_MI_MODULE_ID_VDISP;
    else if (strstr(name, "mi_disp"))
        return E_MI_MODULE_ID_DISP;
    else if (strstr(name, "mi_os"))
        return E_MI_MODULE_ID_OS;
    else if (strstr(name, "mi_iae"))
        return E_MI_MODULE_ID_IAE;
    else if (strstr(name, "mi_md"))
        return E_MI_MODULE_ID_MD;
    else if (strstr(name, "mi_od"))
        return E_MI_MODULE_ID_OD;
    else if (strstr(name, "mi_shadow"))
        return E_MI_MODULE_ID_SHADOW;
    else if (strstr(name, "mi_warp"))
        return E_MI_MODULE_ID_WARP;
    else if (strstr(name, "mi_uac"))
        return E_MI_MODULE_ID_UAC;
    else if (strstr(name, "mi_ldc"))
        return E_MI_MODULE_ID_LDC;
    else if (strstr(name, "mi_sd"))
        return E_MI_MODULE_ID_SD;
    else if (strstr(name, "mi_panel"))
        return E_MI_MODULE_ID_PANEL;
    else if (strstr(name, "mi_cipher"))
        return E_MI_MODULE_ID_CIPHER;
    else if (strstr(name, "mi_sensor"))
        return E_MI_MODULE_ID_SNR;
    else if (strstr(name, "mi_wlan"))
        return E_MI_MODULE_ID_WLAN;
    else if (strstr(name, "mi_ipu"))
        return E_MI_MODULE_ID_IPU;
    else if (strstr(name, "mi_mipitx"))
        return E_MI_MODULE_ID_MIPITX;
    else if (strstr(name, "mi_gyro"))
        return E_MI_MODULE_ID_GYRO;
    else if (strstr(name, "mi_jpd"))
        return E_MI_MODULE_ID_JPD;
    else if (strstr(name, "mi_wbc"))
        return E_MI_MODULE_ID_WBC;
    else if (strstr(name, "mi_dsp"))
        return E_MI_MODULE_ID_DSP;
    else if (strstr(name, "mi_pcie"))
        return E_MI_MODULE_ID_PCIE;
    else if (strstr(name, "mi_dummy"))
        return E_MI_MODULE_ID_DUMMY;
    else if (strstr(name, "mi_nir"))
        return E_MI_MODULE_ID_NIR;
    else if (strstr(name, "mi_dpu"))
        return E_MI_MODULE_ID_DPU;
    else
    {
        // printk("Not MI Device\n");
        return -1;
    }
}

struct stream_ring_ctrl
{
    union
    {
        struct
        {
            unsigned int wpos : 14; // max 8k
            unsigned int epos : 14;
            unsigned int resv : 4;
        };
        unsigned int info; // must ==  unsigned int
    };
    unsigned short rpos; // only local os use
};
static ssize_t proc_read(struct seq_file *q, void *v)
{
    struct dualos_adaptor_node *anode   = q->private;
    struct proc_EntryNode *     pd      = (struct proc_EntryNode *)anode->private;
    int                         res     = 0;
    int                         modid   = -1;
    unsigned long               logpage = get_zeroed_page(GFP_KERNEL);
    linux_ctx                   ctx     = {.log_base = __pa(logpage), .idx = -1};
    struct proc_buffer          pb      = {0};
    struct stream_ring_ctrl     ctrl    = {};
    pb.buf                              = (void *)logpage;

    modid = dualos_adaptor_parseModuleIdByName(pd->entry_path);
    strcpy(ctx.entry_path, pd->entry_path);
    pb.size = PAGE_SIZE;
    flush_cache_area((void *)logpage, PAGE_SIZE);
    flush_cache_area(&ctx, sizeof(ctx));
    Chip_Flush_MIU_Pipe();
    invalid_cache_area((void *)logpage, PAGE_SIZE);

    mutex_lock(&proc_mutex);
    res = dualos_adaptor_interos(anode, modid, __pa((long)&ctx), -1, __MI_DEVICE_PROC_IO);
    do
    {
        res = dualos_adaptor_interos(anode, modid, ctrl.info, -1, __MI_DEVICE_PROC_POLL);
        if (res == -2)
        {
            pr_err("dualos-adaptor: proc_poll timeout:0x%x!\n", res);
            break;
        }
        if (res < 0)
        {
            pr_err("dualos-adaptor: proc_poll err:0x%x!\n", res);
            break;
        }
        // recive done
        if (res == 0)
        {
            // printk("read over\n");
            break;
        }
        // recived some logs
        ctrl.info = res;
        if (ctrl.wpos > ctrl.rpos)
        {
            res = ctrl.wpos - ctrl.rpos;
            seq_write(q, pb.buf + ctrl.rpos, res);
        }
        else if (ctrl.wpos < ctrl.rpos)
        {
            res = ctrl.epos - ctrl.rpos;
            seq_write(q, pb.buf + ctrl.rpos, res);
            seq_write(q, pb.buf + 0, ctrl.wpos);
            res += ctrl.wpos;
        }
        if (res > pb.size)
        {
            pr_err("dualos-adaptor: proc_poll invalid value:0x%x > 0x%x!\n", res, pb.size);
        }
        ctrl.rpos = ctrl.wpos; // read over update rpos
        invalid_cache_area((void *)logpage, PAGE_SIZE);
    } while (1);
    mutex_unlock(&proc_mutex);

    free_page(logpage);
    return 0;
}

static ssize_t proc_write(struct file *file, const char __user *user_buf, size_t count, loff_t *ppos)
{
    struct seq_file *           q     = file->private_data;
    struct dualos_adaptor_node *anode = q->private;
    struct proc_EntryNode *     pd    = anode->private;
    int                         res   = 0;
    int                         modid = -1;
    char *                      kbuf;
    unsigned long               logpage = get_zeroed_page(GFP_KERNEL);
    linux_ctx                   ctx     = {.log_base = __pa(logpage), .idx = -1};
    struct proc_buffer          pb      = {0};
    pb.buf                              = (void *)logpage;

    kbuf = memdup_user_nul(user_buf, count);
    if (!kbuf)
        return -ENOMEM;

    ctx.cmd = __pa(kbuf);
    modid   = dualos_adaptor_parseModuleIdByName(pd->entry_path);
    strcpy(ctx.entry_path, pd->entry_path);
    pb.size = PAGE_SIZE;
    flush_cache_area((void *)logpage, PAGE_SIZE);
    flush_cache_area(kbuf, count);
    flush_cache_area(&ctx, sizeof(ctx));
    Chip_Flush_MIU_Pipe();
    invalid_cache_area((void *)logpage, PAGE_SIZE);

    mutex_lock(&proc_mutex);
    res = dualos_adaptor_interos(anode, modid, __pa((long)&ctx), -1, __MI_DEVICE_PROC_IO);
    while (-2 == dualos_adaptor_interos(anode, modid, 0, -1, __MI_DEVICE_PROC_CONT))
    {
    };
    mutex_unlock(&proc_mutex);

    free_page(logpage);
    kfree(kbuf);
    return count;
}

static int proc_close(struct inode *inode, struct file *file)
{
    struct seq_file *           q     = file->private_data;
    struct dualos_adaptor_node *anode = q->private;

    if (anode)
    {
        dualos_adaptor_node_release(anode);
        q->private = NULL;
    }
    return single_release(inode, file);
}

static int proc_open(struct inode *inode, struct file *file)
{
    int                         remote_addr;
    struct dualos_adaptor_node *anode;
    int                         dst = REMOTE_ADAPTOR_MANAGER_EPT_ADDR;

    anode = dualos_adaptor_node_alloc(MI_PROC_FAKE_ID, 0xf, 0xff);
    if (IS_ERR_VALUE(anode))
        return (int)anode;

    remote_addr = dualos_adaptor_interos_to(anode, dst, __MI_DEVICE_ADDR_QUERY, anode->mod_id, anode->dev_id,
                                            anode->chn_id, NULL);
    if (remote_addr < 0)
    {
        pr_err(
            "Adaptor node (%d,%d,%d,%d) get remote addr "
            "error %d\n",
            anode->mod_id, anode->dev_id, anode->chn_id, anode->remote_addr, remote_addr);
        dualos_adaptor_node_release(anode);
        return -EINVAL;
    }
    pr_info("Adaptor node (%d,%d,%d,%d) get remote addr 0x%x\n", anode->mod_id, anode->dev_id, anode->chn_id,
            anode->remote_addr, remote_addr);
    anode->remote_addr = remote_addr;

    anode->private = PDE_DATA(inode);
    return single_open(file, proc_read, anode);
}

static const struct proc_ops g_FetchProcOps = {
    .proc_open    = proc_open,
    .proc_read    = seq_read,
    .proc_write   = proc_write,
    .proc_lseek   = seq_lseek,
    .proc_release = proc_close,
};

static int fetch_proc(struct dualos_adaptor_node *anode, int id, const char *name)
{
    unsigned long          res = 0;
    struct proc_EntryList *pd  = kzalloc(sizeof(struct proc_EntryList), GFP_KERNEL); //(struct proc_Entry*)zpage;
    int                    remote_addr;
    int                    dst = REMOTE_ADAPTOR_MANAGER_EPT_ADDR;

    flush_cache_area((void *)pd, sizeof(struct proc_EntryList));
    invalid_cache_area(pd, sizeof(struct proc_EntryList));
    remote_addr = dualos_adaptor_interos_to(anode, dst, __MI_DEVICE_ADDR_QUERY, id, anode->dev_id, anode->chn_id, NULL);
    if (remote_addr < 0)
    {
        kfree(pd);
        memset(&proc_list, 0, sizeof(struct proc_EntryList));
        return remote_addr;
    }

    res = dualos_adaptor_interos_to(anode, remote_addr, id, __pa(pd), -1, __MI_DEVICE_PROC, NULL);
    if (res == -ETIMEDOUT)
        goto err;

    if (res && id == MI_PROC_FAKE_ID)
    {
        int  i                             = 0;
        char proc_path[MAX_ENTRY_PATH_LEN] = {0};
        memcpy(&proc_list, pd, sizeof(struct proc_EntryList));
        for (i = 0; i < pd->Length; i++)
        {
            memset(proc_path, 0, sizeof(proc_path));
            if (pd->eNode[i].dir)
            {
                /* dir skip start and end '/' */
                // snprintf(proc_path, strlen(pd->eNode[i].entry_path)-1, "%s", pd->eNode[i].entry_path+1);
                strncpy(proc_path, pd->eNode[i].entry_path + 1, strlen(pd->eNode[i].entry_path) - 2);
                proc_mkdir(proc_path, NULL);
            }
            else
            {
                /* not dir skip start '/' */
                int modId = -1;
                // snprintf(proc_path, strlen(pd->eNode[i].entry_path), "%s", pd->eNode[i].entry_path+1);
                strncpy(proc_path, pd->eNode[i].entry_path + 1, strlen(pd->eNode[i].entry_path) - 1);
                proc_create_data(proc_path, 0640, NULL, &g_FetchProcOps, &proc_list.eNode[i]);
                modId = dualos_adaptor_parseModuleIdByName(proc_path);
                if (modId != -1)
                {
                    int devId = -1;
                    int n     = strlen(pd->eNode[i].entry_path);
                    /* mi_modules chrdev */
                    device_chrdev[modId] = 1;
                    /* mi_modules procdev */
                    while (n > 1 && pd->eNode[i].entry_path[n - 1] >= '0' && pd->eNode[i].entry_path[n - 1] <= '9')
                    {
                        n--;
                    }
                    if (n != strlen(pd->eNode[i].entry_path))
                        devId = simple_strtoul(pd->eNode[i].entry_path + n, NULL, 0);
                    if (devId != -1 && devId < MI_PROC_DEV_ID_MAX && proc_node[modId][devId] == 0)
                        proc_node[modId][devId] = 1;
                }
            }
        }
    }
err:
    kfree(pd);
    return res;
}

static const char *dev_list[] = {
    "ive",    /* 0 */
    "vdf",    /* 1 */
    "venc",   /* 2 */
    "rgn",    /* 3 */
    "ai",     /* 4 */
    "ao",     /* 5 */
    "vif",    /* 6 */
    "vpe",    /* 7 */
    "vdec",   /* 8 */
    "sys",    /* 9 */
    "fb",     /* 10 */
    "hdmi",   /* 11 */
    "divp",   /* 12 */
    "gfx",    /* 13 */
    "vdisp",  /* 14 */
    "disp",   /* 15 */
    "os",     /* 16 */
    "iae",    /* 17 */
    "md",     /* 18 */
    "od",     /* 19 */
    "shadow", /* 20 */
    "warp",   /* 21 */
    "uac",    /* 22 */
    "ldc",    /* 23 */
    "sd",     /* 24 */
    "panel",  /* 25 */
    "cipher", /* 26 */
    "sensor", /* 27 */
    "wlan",   /* 28 */
    "ipu",    /* 29 */
    "mipitx", /* 30 */
    "gyro",   /* 31 */
    "jpd",    /* 32 */
    "isp",    /* 33 */
    "scl",    /* 34 */
    "wbc",    /* 35 */
    "dsp",    /* 36 */
    "pcie",   /* 37 */
    "dummy",  /* 38 */
    "nir",    /* 39 */
    "dpu",    /* 40 */
};

static const struct file_operations pfops = {
    .owner          = THIS_MODULE,
    .open           = MI_PollAdp_Open,
    .release        = MI_PollAdp_Release,
    .unlocked_ioctl = MI_PollAdp_Ioctl,
    .poll           = MI_PollAdp_Poll,
    .llseek         = noop_llseek,
};

static int dualos_adaptor_sendto(struct dualos_adaptor_node *anode, int dst, char *buffer, int size, int timeout)
{
    int res;

    do
    {
        res = rpmsg_sendto(anode->ept, buffer, size, dst);
        if (!res)
            break;

        if (res && res != -ENOMEM)
        {
            pr_err(
                "Adaptor node (%d,%d,%d,%d) rpmsg_trysendto error %d, "
                "remote_addr=%d\n",
                anode->mod_id, anode->dev_id, anode->chn_id, anode->remote_addr, res, dst);
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
            "Adaptor node (%d,%d,%d,%d) rpmsg_trysendto timeout %d, "
            "remote_addr=%d\n",
            anode->mod_id, anode->dev_id, anode->chn_id, anode->remote_addr, res, dst);
    }
    return res;
}

static int dualos_adaptor_interos_to(struct dualos_adaptor_node *anode, int dst, unsigned int arg0, unsigned int arg1,
                                     unsigned int arg2, unsigned int arg3, struct dualos_interos_datagram *data)
{
    struct dualos_interos_datagram *resp;
    struct dualos_interos_datagram  req;
    struct sk_buff *                skb;
    unsigned long                   flags;
    int                             res;
    DEFINE_WAIT(wait);

    mutex_lock(&anode->mutex);
    memset(&req, 0, sizeof(req));
    req.header.version = SSTAR_RPMSG_DUALOS_VERSION;
    req.header.index   = anode->index++;
    req.arg0           = arg0;
    req.arg1           = arg1;
    req.arg2           = arg2;
    req.arg3           = arg3;

    res = dualos_adaptor_sendto(anode, dst, (char *)&req, sizeof(req), TX_TIMEOUT);
    if (res)
    {
        mutex_unlock(&anode->mutex);
        return res;
    }

    while (1)
    {
        spin_lock_irqsave(&anode->queue_lock, flags);

        if (skb_queue_empty(&anode->queue))
        {
            prepare_to_wait(&anode->wq, &wait, TASK_UNINTERRUPTIBLE);
            spin_unlock_irqrestore(&anode->queue_lock, flags);

            res = schedule_timeout(INTEROS_TIMEOUT);
            finish_wait(&anode->wq, &wait);
            spin_lock_irqsave(&anode->queue_lock, flags);
        }
        skb = skb_dequeue(&anode->queue);
        spin_unlock_irqrestore(&anode->queue_lock, flags);

        if (!skb)
        {
            if (res == 0)
            {
                res = -ETIMEDOUT;
                goto err;
            }
            continue;
        }

        resp = (struct dualos_interos_datagram *)skb->data;
        if (resp->header.index != req.header.index)
        {
            pr_err(
                "Adaptor node (%d,%d,%d,%d): index not match(%d-%d) "
                "req.arg0=0x%x,req.arg1=0x%x,req.arg2=0x%x,"
                "req.arg3=0x%x,resp.arg0=0x%x,resp.arg1=0x%x"
                "resp.arg2=0x%x,resp.arg3=0x%x\n",
                anode->mod_id, anode->dev_id, anode->chn_id, anode->remote_addr, resp->header.index, req.header.index,
                req.arg0, req.arg1, req.arg2, req.arg3, resp->arg0, resp->arg1, resp->arg2, resp->arg3);
            kfree_skb(skb);
            continue;
        }
        break;
    }

    res = resp->arg0;
    if (data)
        memcpy(data, resp, sizeof(*data));
    mutex_unlock(&anode->mutex);
    kfree_skb(skb);
    return res;
err:
    mutex_unlock(&anode->mutex);
    pr_err(
        "Adaptor node (%d,%d,%d,%d) interos error %d, "
        "remote_addr=%d, index=0x%x, arg0=0x%x, "
        "arg1=0x%x, arg2=0x%x, arg3=0x%x\n",
        anode->mod_id, anode->dev_id, anode->chn_id, anode->remote_addr, res, dst, req.header.index, req.arg0, req.arg1,
        req.arg2, req.arg3);
    return res;
}

static int dualos_adaptor_interos(struct dualos_adaptor_node *anode, unsigned int arg0, unsigned int arg1,
                                  unsigned int arg2, unsigned int arg3)
{
    return dualos_adaptor_interos_to(anode, anode->remote_addr, arg0, arg1, arg2, arg3, NULL);
}

static struct dualos_adaptor_node *dualos_adaptor_node_alloc(int mod_id, int dev_id, int chn_id)
{
    struct dualos_adaptor_node *anode;
    struct rpmsg_channel_info   info;
    int                         res;

    anode = kzalloc(sizeof(*anode), GFP_KERNEL);
    if (!anode)
    {
        pr_err("dualos_adaptor_node_alloc: out of memory!\n");
        return (void *)-ENOMEM;
    }

    anode->mod_id      = mod_id;
    anode->dev_id      = dev_id;
    anode->chn_id      = chn_id;
    anode->remote_addr = RPMSG_ADDR_ANY;

    init_waitqueue_head(&anode->wq);
    spin_lock_init(&anode->queue_lock);
    skb_queue_head_init(&anode->queue);
    mutex_init(&anode->mutex);

    snprintf(info.name, sizeof(info.name), "MI_EPT");
    info.src = RPMSG_ADDR_ANY;
    info.dst = RPMSG_ADDR_ANY;

#if defined(CONFIG_ARCH_INFINITY7) && defined(CONFIG_SSTAR_CA7_VIRTIO)
    anode->ept = dualos_rpmsg_create_ept(RPMsg_Device_CA7, 0, dualos_adaptor_cb, anode, info);
#elif defined(CONFIG_SSTAR_LH_RTOS_VIRTIO)
    anode->ept = dualos_rpmsg_create_ept(RPMsg_Device_LH_RTOS, 0, dualos_adaptor_cb, anode, info);
#endif
    if (anode->ept == NULL)
    {
        pr_err("Adaptor node (%d,%d,%d,%d) dualos_rpmsg_create_ept error\n", anode->mod_id, anode->dev_id,
               anode->chn_id, anode->remote_addr);
        res = -EINVAL;
        goto err_create_ept;
    }
    return anode;
err_create_ept:
    kfree(anode);
    return (void *)res;
}

static void dualos_adaptor_node_release(struct dualos_adaptor_node *anode)
{
    if (!anode)
        return;

    if (anode->ept)
        dualos_rpmsg_destroy_ept(anode->ept);
    skb_queue_purge(&anode->queue);
    kfree(anode);
    return;
}

static struct dualos_adaptor_node *MI_DEVICE_Open_ANode(int id, bool kapi)
{
    struct dualos_adaptor_node *anode;
    int                         ret;
    int                         remote_addr;
    linux_ctx                   ctx = {
        .idx = -1,
    };
    int dst = REMOTE_ADAPTOR_MANAGER_EPT_ADDR;

    if (kapi)
        ctx.pid = -1;
    else
        ctx.pid = current->tgid;
    anode = dualos_adaptor_node_alloc(id, 0xf, 0xff);
    if (IS_ERR_VALUE(anode))
        return anode;

    remote_addr = dualos_adaptor_interos_to(anode, dst, __MI_DEVICE_ADDR_QUERY, anode->mod_id, anode->dev_id,
                                            anode->chn_id, NULL);
    if (remote_addr < 0)
    {
        pr_err(
            "Adaptor node (%d,%d,%d,%d) get remote addr "
            "error %d\n",
            anode->mod_id, anode->dev_id, anode->chn_id, anode->remote_addr, remote_addr);
        dualos_adaptor_node_release(anode);
        return ERR_PTR(-EINVAL);
    }
    pr_info("Adaptor node (%d,%d,%d,%d) get remote addr 0x%x\n", anode->mod_id, anode->dev_id, anode->chn_id,
            anode->remote_addr, remote_addr);
    anode->remote_addr = remote_addr;

    if (anode->mod_id != MI_CLI_FAKE_ID)
    {
        flush_cache_area(&ctx, sizeof(ctx));
        Chip_Flush_MIU_Pipe();
        ret = dualos_adaptor_interos(anode, id, __pa((long)&ctx), -1, __MI_DEVICE_CONNECT);
        if (ret < 0)
        {
            dualos_adaptor_node_release(anode);
            return ERR_PTR(ret);
        }
    }
    return anode;
}

static int MI_DEVICE_Open(struct inode *inode, struct file *filp)
{
    int                         id            = iminor(inode);
    struct dualos_adaptor_node *anode         = NULL;
    void *                      vaddr         = NULL;
    int                         res           = 0;
    mma_map_record_t *          pMmaMapRecord = NULL;

    mutex_lock(&device_mtx);

    anode = MI_DEVICE_Open_ANode(id, false);
    if (IS_ERR_VALUE(anode))
    {
        mutex_unlock(&device_mtx);
        return PTR_ERR(anode);
    }

    CamOsTsemDown(&g_mma_map_hash_semphore);
    CAM_OS_HASH_FOR_EACH_POSSIBLE(g_mma_map_addr_hash, pMmaMapRecord, hentry, current->tgid)
    {
        if (pMmaMapRecord && pMmaMapRecord->pid == current->tgid)
        {
            anode->mma_cache_vaddr   = pMmaMapRecord->mma_cache_vaddr;
            anode->mma_nocache_vaddr = pMmaMapRecord->mma_nocache_vaddr;
            filp->private_data       = (void *)anode;
            CamOsTsemUp(&g_mma_map_hash_semphore);
            goto ok;
        }
    }

    if (id != E_MI_MODULE_ID_SYS && id != MI_CLI_FAKE_ID)
    {
        res = -1;
        pr_err("Err: need call MI_SYS_Init fisrt in this thread tgid %d befort open mod_id %d\n", current->tgid,
               anode->mod_id);
        goto err;
    }

    anode->bcache      = 0;
    filp->private_data = (void *)anode;
    /* map mma nocache range */
    vaddr = (void *)vm_mmap(filp, 0, mma_size, PROT_READ | PROT_WRITE, MAP_SHARED, rtk_res->start);
    if (IS_ERR_VALUE(vaddr))
    {
        res = PTR_ERR(vaddr);
        pr_err("Adaptor node (%d,%d,%d,%d) failed to vm_mmap nocache %x\n", anode->mod_id, anode->dev_id, anode->chn_id,
               anode->remote_addr, (unsigned int)vaddr);
        goto err;
    }
    anode->mma_nocache_vaddr = vaddr;

    anode->bcache      = 1;
    filp->private_data = (void *)anode;
    /* map mma cache range */
    vaddr = (void *)vm_mmap(filp, 0, mma_size, PROT_READ | PROT_WRITE, MAP_SHARED, rtk_res->start);
    if (IS_ERR_VALUE(vaddr))
    {
        res = PTR_ERR(vaddr);
        pr_err("Adaptor node (%d,%d,%d,%d) failed to vm_mmap cache %x\n", anode->mod_id, anode->dev_id, anode->chn_id,
               anode->remote_addr, (unsigned int)vaddr);
        goto err;
    }
    anode->mma_cache_vaddr = vaddr;

    pMmaMapRecord = CamOsMemAlloc(sizeof(*pMmaMapRecord));
    if (!pMmaMapRecord)
    {
        CamOsTsemUp(&g_mma_map_hash_semphore);
        goto err;
    }

    pMmaMapRecord->mma_cache_vaddr   = anode->mma_cache_vaddr;
    pMmaMapRecord->mma_nocache_vaddr = anode->mma_nocache_vaddr;
    pMmaMapRecord->pid               = current->tgid;
    pMmaMapRecord->mod_id            = id;
    CAM_OS_HASH_ADD(g_mma_map_addr_hash, &pMmaMapRecord->hentry, current->tgid);

    CamOsTsemUp(&g_mma_map_hash_semphore);
ok:

    mutex_unlock(&device_mtx);
    return res;
err:
    dualos_adaptor_node_release(anode);
    filp->private_data = NULL;
    mutex_unlock(&device_mtx);
    return res;
}

static void MI_DEVICE_Release_ANode(struct dualos_adaptor_node *anode, bool kapi)
{
    int       id  = anode->mod_id;
    int       ret = 0;
    linux_ctx ctx = {
        .idx = -1,
    };

    if (kapi)
        ctx.pid = -1;
    else
        ctx.pid = current->tgid;

    if (anode->mod_id != MI_CLI_FAKE_ID)
    {
        flush_cache_area(&ctx, sizeof(ctx));
        Chip_Flush_MIU_Pipe();
        ret = dualos_adaptor_interos(anode, id, __pa((long)&ctx), -1, __MI_DEVICE_DISCONNECT);
        if (ret)
        {
            pr_err("__MI_DEVICE_DISCONNECT (%d,%d) returns error %d\n", id, ctx.pid, ret);
        }
    }
    dualos_adaptor_node_release(anode);
    return;
}

static int MI_DEVICE_Release(struct inode *inode, struct file *filp)
{
    struct dualos_adaptor_node *anode         = NULL;
    struct CamOsHListNode_t *   pos           = NULL;
    mma_map_record_t *          pMmaMapRecord = NULL;
    int                         res           = 0;
    int                         id            = 0;

    mutex_lock(&device_mtx);

    anode = filp->private_data;
    id    = anode->mod_id;
    MI_DEVICE_Release_ANode(anode, false);
    filp->private_data = NULL;

    if (id == E_MI_MODULE_ID_SYS || id == MI_CLI_FAKE_ID)
    {
        CamOsTsemDown(&g_mma_map_hash_semphore);
        CAM_OS_HASH_FOR_EACH_POSSIBLE_SAFE(g_mma_map_addr_hash, pMmaMapRecord, pos, hentry, current->tgid)
        {
            if (pMmaMapRecord && pMmaMapRecord->pid == current->tgid && id == pMmaMapRecord->mod_id)
            {
                CamOsMemFromUserModeUnmap(pMmaMapRecord->mma_cache_vaddr, mma_size);
                CamOsMemFromUserModeUnmap(pMmaMapRecord->mma_nocache_vaddr, mma_size);
                CAM_OS_HASH_DEL(&pMmaMapRecord->hentry);
                CamOsMemRelease(pMmaMapRecord);
                break;
            }
        }
        CamOsTsemUp(&g_mma_map_hash_semphore);
    }

    mutex_unlock(&device_mtx);
    return res;
}

static unsigned long vir2phy(struct task_struct *curr, void *ptr)
{
    unsigned long addr = (unsigned long)ptr;
    pgd_t *       pgd  = pgd_offset(curr->mm, addr);
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 61)
    p4d_t *p4d = p4d_offset(pgd, addr);
    pud_t *pud = pud_offset(p4d, addr);
#else
    pud_t *pud = pud_offset(pgd, addr);
#endif
    pmd_t *pmd = pmd_offset(pud, addr);
    pte_t *pte = pmd_page_vaddr(*pmd) + pte_index(addr);
    return __pfn_to_phys(pte_pfn(*pte)) + (addr & ~PAGE_MASK);
}

static int MI_MMA_Is_Va_Invalid(struct dualos_adaptor_node *anode, void *pVirtualAddress, int u32BufSize)
{
    if ((u32BufSize > 0)
        && ((pVirtualAddress >= anode->mma_cache_vaddr
             && pVirtualAddress + u32BufSize <= anode->mma_cache_vaddr + mma_size)
            || (pVirtualAddress >= anode->mma_nocache_vaddr
                && pVirtualAddress + u32BufSize <= anode->mma_nocache_vaddr + mma_size)))
    {
        return 1;
    }

    return 0;
}

static void MI_MMA_Flush(struct dualos_adaptor_node *anode, MI_SYS_BufInfo_t stBufInfo)
{
    void *pVirtualAddress = NULL;
    int   u32BufSize      = 0;
    if (E_MI_SYS_BUFDATA_RAW == stBufInfo.eBufType)
    {
        pVirtualAddress = stBufInfo.stRawData.pVirAddr;
        u32BufSize      = stBufInfo.stRawData.u32BufSize;
    }
    else if (E_MI_SYS_BUFDATA_FRAME == stBufInfo.eBufType)
    {
        pVirtualAddress = stBufInfo.stFrameData.pVirAddr[0];
        u32BufSize      = stBufInfo.stFrameData.u32BufSize;
    }
    else if (E_MI_SYS_BUFDATA_META == stBufInfo.eBufType)
    {
        pVirtualAddress = stBufInfo.stMetaData.pVirAddr;
        u32BufSize      = stBufInfo.stMetaData.u32Size;
    }
    else if (E_MI_SYS_BUFDATA_MULTIPLANE == stBufInfo.eBufType)
    {
        int i = 0;
        for (i = 0; i < stBufInfo.stFrameDataMultiPlane.u8SubPlaneNum; i++)
        {
            pVirtualAddress = stBufInfo.stFrameDataMultiPlane.stSubPlanes[i].pVirAddr[0];
            u32BufSize      = stBufInfo.stFrameDataMultiPlane.stSubPlanes[i].u32BufSize;
            if (MI_MMA_Is_Va_Invalid(anode, pVirtualAddress, u32BufSize))
            {
                CamOsMemFlush(pVirtualAddress, CAM_OS_ALIGN_UP(u32BufSize, CamOsGetCacheLineSize()));
            }
        }
        return;
    }
    if (MI_MMA_Is_Va_Invalid(anode, pVirtualAddress, u32BufSize))
    {
        CamOsMemFlush(pVirtualAddress, CAM_OS_ALIGN_UP(u32BufSize, CamOsGetCacheLineSize()));
    }
    return;
}

static void MI_MMA_Invalidate(struct dualos_adaptor_node *anode, MI_SYS_BufInfo_t stBufInfo)
{
    void *pVirtualAddress = NULL;
    int   u32BufSize      = 0;
    if (E_MI_SYS_BUFDATA_RAW == stBufInfo.eBufType)
    {
        pVirtualAddress = stBufInfo.stRawData.pVirAddr;
        u32BufSize      = stBufInfo.stRawData.u32BufSize;
    }
    else if (E_MI_SYS_BUFDATA_FRAME == stBufInfo.eBufType)
    {
        pVirtualAddress = stBufInfo.stFrameData.pVirAddr[0];
        u32BufSize      = stBufInfo.stFrameData.u32BufSize;
    }
    else if (E_MI_SYS_BUFDATA_META == stBufInfo.eBufType)
    {
        pVirtualAddress = stBufInfo.stMetaData.pVirAddr;
        u32BufSize      = stBufInfo.stMetaData.u32Size;
    }
    else if (E_MI_SYS_BUFDATA_MULTIPLANE == stBufInfo.eBufType)
    {
        int i = 0;
        for (i = 0; i < stBufInfo.stFrameDataMultiPlane.u8SubPlaneNum; i++)
        {
            pVirtualAddress = stBufInfo.stFrameDataMultiPlane.stSubPlanes[i].pVirAddr[0];
            u32BufSize      = stBufInfo.stFrameDataMultiPlane.stSubPlanes[i].u32BufSize;
            if (MI_MMA_Is_Va_Invalid(anode, pVirtualAddress, u32BufSize))
            {
                invalid_cache_area(pVirtualAddress, CAM_OS_ALIGN_UP(u32BufSize, CamOsGetCacheLineSize()));
            }
        }
        return;
    }
    if (MI_MMA_Is_Va_Invalid(anode, pVirtualAddress, u32BufSize))
    {
        invalid_cache_area(pVirtualAddress, CAM_OS_ALIGN_UP(u32BufSize, CamOsGetCacheLineSize()));
    }
    return;
}

static long do_MI_DEVICE_Ioctl(struct dualos_adaptor_node *anode, unsigned int cmd, unsigned long ptr, bool kapi)
{
    int  id   = anode->mod_id;
    long rval = -EIO;

    if (_IOC_TYPE(cmd) == 'i' || _IOC_TYPE(cmd) == 'd')
    {
        unsigned long res = 0;
        atomic_t *    cost;
        atomic_t *    freq;
        ktime_t       t1, t2;
        t1 = ktime_get();

        if (ptr)
        {
            linux_ctx ctx = {.curr_cache_vaddrbase   = anode->mma_cache_vaddr,
                             .curr_nocache_vaddrbase = anode->mma_nocache_vaddr,
                             .idx                    = -1,
                             .mma_base               = mma_base};
            struct
            {
                unsigned short     socid;
                int                len;
                unsigned long long ptr;
            } tr;
            void *           arg       = NULL;
            int              len_align = 0;
            MI_SYS_BufInfo_t stBufInfo = {0};

            if (kapi)
                ctx.pid = -1;
            else
                ctx.pid = current->tgid;

            if (kapi)
                memcpy((char *)&tr, (void *)ptr, sizeof(tr));
            else if (copy_from_user((char *)&tr, (void *)ptr, sizeof(tr)))
                return -EFAULT;

            if (tr.len > _IOC_SIZE(cmd))
            {
                printk(KERN_ERR "write cmd(0x%08x) overflow!", cmd);
                return -EINVAL;
            }

            if (tr.len > 4096)
            {
                printk(KERN_WARNING "write cmd(0x%08x) Send Big Data size(%d)!", cmd, tr.len);
            }

            if (_IOC_DIR(cmd) & _IOC_WRITE)
            {
                if (tr.len == 0)
                {
                    printk(KERN_ERR "write cmd(0x%08x) send null data!", cmd);
                    return -EINVAL;
                }

                len_align = ALIGN_UP(tr.len, DATA_CACHE_ENTRY_SIZE);

                if (kapi)
                    arg = kmemdup((void *)(long)tr.ptr, tr.len, GFP_KERNEL);
                else
                    arg = kzalloc(len_align, GFP_KERNEL);
                if (!arg)
                    return -ENOMEM;

                if (kapi)
                {
                    // NOP
                }
                else if (copy_from_user(arg, (void *)(long)tr.ptr, len_align))
                {
                    kfree(arg);
                    printk(KERN_ERR "copy cmd(0x%08x) overflow!", cmd);
                    return -EFAULT;
                }
                if (_IOC_DIR(cmd) & _IOC_READ)
                {
                    flush_and_invalid_cache_area(arg, len_align);
                }
                else
                {
                    flush_cache_area(arg, len_align);
                }
            }
            else if (_IOC_DIR(cmd) & _IOC_READ)
            {
                len_align = ALIGN_UP(tr.len + sizeof(long), DATA_CACHE_ENTRY_SIZE);
                arg       = kzalloc(len_align, GFP_KERNEL);
                if (!arg)
                    return -ENOMEM;
                invalid_cache_area(arg, len_align);
            }
            else
            {
                printk(KERN_ERR "send a buffer to cmd(0x%08x) with_IOC_TYPE_NONE!\n", cmd);
                return -EINVAL;
            }
            ctx.arg_size = _IOC_SIZE(cmd);
            flush_cache_area(&ctx, sizeof(ctx));
            Chip_Flush_MIU_Pipe();

            if (id == E_MI_MODULE_ID_SYS && current->mm != NULL)
            {
                switch (_IOC_NR(cmd))
                {
                    case E_MI_SYS_FLUSH_INV_CACHE:
                    {
                        void *pVirtualAddress =
                            (void *)(MI_U32)(((MI_SYS_FlushInvCache_t *)(unsigned long)arg)->p64VirtualAddress);
                        int u32Size = ((MI_SYS_FlushInvCache_t *)(unsigned long)arg)->length;
                        if (MI_MMA_Is_Va_Invalid(anode, pVirtualAddress, u32Size))
                        {
                            CamOsMemFlush(pVirtualAddress, u32Size);
                        }
                    }
                    break;
                    case E_MI_SYS_CMD_CHN_INPUT_PORT_PUT_BUF:
                    {
                        MI_SYS_ChnInputPortPutBuf_t *pstChnPortPutBuf =
                            (MI_SYS_ChnInputPortPutBuf_t *)(unsigned long)arg;
                        MI_MMA_Flush(anode, pstChnPortPutBuf->stBufInfo);
                    }
                    break;
                    case E_MI_SYS_CMD_CHN_OUTPUT_PORT_PUT_BUF:
                    {
                        MI_SYS_BUF_HANDLE *pHandle = (MI_SYS_BUF_HANDLE *)arg;

                        flush_and_invalid_cache_area(&stBufInfo, sizeof(MI_SYS_BufInfo_t));
                        if (dualos_adaptor_interos_to(anode, REMOTE_ADAPTOR_MANAGER_EPT_ADDR, __MI_SYS_BUF_HANDLE_QUERY,
                                                      (unsigned int)*pHandle, __pa((long)&stBufInfo), 0, NULL)
                            == 0)
                        {
                            invalid_cache_area(&stBufInfo, sizeof(MI_SYS_BufInfo_t));
                            MI_MMA_Flush(anode, stBufInfo);
                        }
                    }
                    break;
                    default:
                        break;
                }
            }

            res = dualos_adaptor_interos(anode, id, __pa((long)&ctx), cmd, __pa((unsigned long)arg));
            if (res == -1)
            {
                // pr_err("mi_device_ioctl2:ctx.arg_size:0x%x,0x%x,0x%llx,0x%x -> 0x%lx\n", ctx.arg_size, cmd,
                //        __pa((unsigned long)arg), anode->index, res);
            }
            rval = res;
            if (_IOC_DIR(cmd) & _IOC_READ)
            {
                invalid_cache_area(arg, len_align);
                if (kapi)
                    memcpy((char *)(long)tr.ptr, arg, tr.len);
                else if (copy_to_user((char *)(long)tr.ptr, arg, tr.len))
                {
                    kfree(arg);
                    return -EFAULT;
                }
            }

            if (id == E_MI_MODULE_ID_SYS && 0 == res && current->mm != NULL)
            {
                switch (_IOC_NR(cmd))
                {
                    case E_MI_SYS_CMD_CHN_INPUT_PORT_GET_BUF:
                    {
                        MI_SYS_ChnInputPortGetBuf_t *pstChnPortGetBuf =
                            (MI_SYS_ChnInputPortGetBuf_t *)(unsigned long)arg;
                        MI_MMA_Invalidate(anode, pstChnPortGetBuf->stBufInfo);
                    }
                    break;
                    case E_MI_SYS_CMD_CHN_OUTPUT_PORT_GET_BUF:
                    {
                        MI_SYS_ChnOutputPortGetBuf_t *pstChnPortGetBuf =
                            (MI_SYS_ChnOutputPortGetBuf_t *)(unsigned long)arg;
                        MI_MMA_Invalidate(anode, pstChnPortGetBuf->stBufInfo);
                    }
                    break;
                    case E_MI_SYS_CMD_MMAP:
                    {
                        void *   pVirtualAddress;
                        int      u32Size;
                        MI_PTR64 u64StartVa, u64EndVa;

                        u64StartVa = (MI_PTR64)(MI_VIRT)(((MI_SYS_Mmap_t *)(unsigned long)arg)->p64VirtualAddress);
                        u64EndVa   = u64StartVa + ((MI_SYS_Mmap_t *)(unsigned long)arg)->u32Size;
                        if (u64StartVa % CamOsGetCacheLineSize())
                        {
                            u64StartVa = CAM_OS_ALIGN_DOWN(u64StartVa, CamOsGetCacheLineSize());
                        }
                        if (u64EndVa % CamOsGetCacheLineSize())
                        {
                            u64EndVa = CAM_OS_ALIGN_UP(u64EndVa, CamOsGetCacheLineSize());
                        }

                        pVirtualAddress = (void *)(MI_U32)u64StartVa;
                        u32Size         = u64EndVa - u64StartVa;

                        if (MI_MMA_Is_Va_Invalid(anode, pVirtualAddress, u32Size))
                        {
                            invalid_cache_area(pVirtualAddress, u32Size);
                        }
                    }
                    break;
                    default:
                        break;
                }
            }

            kfree(arg);
        }
        else
        {
            linux_ctx ctx = {.idx = -1, .pid = current->tgid};
            ctx.arg_size  = _IOC_SIZE(cmd);
            flush_cache_area(&ctx, sizeof(ctx));
            res  = dualos_adaptor_interos(anode, id, __pa((long)&ctx), cmd, 0);
            rval = res;
        }
        cost = ctx_cost;
        freq = &ctx_freq;

        t2 = ktime_get();
        atomic_inc(freq);
        atomic_inc(cost + time_log2(t1, t2));
    }
    else
    {
        unsigned long *vir = anode->mma_cache_vaddr + cmd;
        unsigned long  uval;

        get_user(uval, vir);
        printk("uva:%p,phy:%lx,off=%x,uval=%lx\n", vir, vir2phy(current, vir), cmd, uval);
        rval = 0;
    }

    return rval;
}

long MI_DEVICE_Ioctl_KAPI(int module_id, unsigned int cmd, unsigned long ptr)
{
    struct dualos_adaptor_node *anode;

    if (module_id < 0 || module_id >= E_MI_MODULE_ID_MAX || disable_os_adaptor)
    {
        return 0;
    }

    mutex_lock(&module_anodes_mutex);
    anode = module_anodes[module_id];
    if (!anode)
    {
        anode = MI_DEVICE_Open_ANode(module_id, true);
        if (IS_ERR_VALUE(anode))
        {
            mutex_unlock(&module_anodes_mutex);
            return PTR_ERR(anode);
        }
        module_anodes[module_id] = anode;
    }
    mutex_unlock(&module_anodes_mutex);

    return do_MI_DEVICE_Ioctl(anode, cmd, ptr, true);
}
EXPORT_SYMBOL(MI_DEVICE_Ioctl_KAPI);

static long MI_DEVICE_Ioctl(struct file *filp, unsigned int cmd, unsigned long ptr)
{
    struct dualos_adaptor_node *anode;

    anode = filp->private_data;

    return do_MI_DEVICE_Ioctl(anode, cmd, ptr, false);
}

static int MI_DEVICE_Mmap(struct file *file, struct vm_area_struct *vma)
{
    static const struct vm_operations_struct vma_ops = {};
    size_t                                   size    = vma->vm_end - vma->vm_start;
    struct dualos_adaptor_node *             anode;
    anode = file->private_data;

    if (anode->bcache)
    {
        vma->vm_page_prot = vm_get_page_prot(vma->vm_flags);
    }
    else
    {
        vma->vm_page_prot = pgprot_writecombine(vma->vm_page_prot);
    }
    vma->vm_ops = &vma_ops;

    /* Remap-pfn-range will mark the range VM_IO */
    if (remap_pfn_range(vma, vma->vm_start, vma->vm_pgoff, size, vma->vm_page_prot))
    {
        return -EAGAIN;
    }
    return 0;
}
static const struct file_operations fops = {
    .owner          = THIS_MODULE,
    .open           = MI_DEVICE_Open,
    .release        = MI_DEVICE_Release,
    .unlocked_ioctl = MI_DEVICE_Ioctl,
    .mmap           = MI_DEVICE_Mmap,
    .llseek         = noop_llseek,
};
module_param(mma_base, ulong, 0644);
module_param(mma_size, ulong, 0644);

static ssize_t debug_tool_read(struct seq_file *q, void *v)
{
    struct debug_tool *dt = q->private;
    if (dt->read)
        return dt->read(dt->obj, q, v);
    return -EIO;
}

static ssize_t debug_tool_write(struct file *file, const char __user *user_buf, size_t count, loff_t *ppos)
{
    const char *args[32] = {NULL};
    int         c        = 0;
    char *      p, tc, *kbuf;

    struct seq_file *  q  = file->private_data;
    struct debug_tool *dt = q->private;

    if (!dt->write)
        return -EIO;

    kbuf = memdup_user_nul(user_buf, count);
    if (!kbuf)
        return -ENOMEM;

    for (p = kbuf, tc = '\0'; tc != '\n' && (c < 32); ++p)
    {
        p += strspn(p, " \t\r\f\v");
        if (*p == '\n')
            break;
        args[c++] = p;
        p += strcspn(p, " \t\n\r\f\v");
        tc = *p;
        *p = '\0';
    }

    if (c < 32)
    {
        dt->write(dt->obj, args, c);
        kfree(kbuf);
        return count;
    }
    kfree(kbuf);
    return -EINVAL;
}

static int debug_tool_open(struct inode *inode, struct file *file)
{
    return single_open(file, debug_tool_read, PDE_DATA(inode));
}

static const struct proc_ops g_DebugtoolProcOps = {
    .proc_open    = debug_tool_open,
    .proc_read    = seq_read,
    .proc_write   = debug_tool_write,
    .proc_lseek   = seq_lseek,
    .proc_release = single_release,
};

static bool debug_tool_create(const char *name, struct debug_tool *dt)
{
    dt->entry = proc_create_data(name, 0640, debug_tools, &g_DebugtoolProcOps, dt);
    if (!dt->entry)
    {
        pr_err("failed	 to  create  procfs  file  %s.\n", name);
        return false;
    }
    return true;
}

static void debug_tool_delete(struct debug_tool *dt)
{
    proc_remove(dt->entry);
}

static u32 proc_dumpbuf_data(u32 arg0, u32 arg1, u32 arg2, u32 arg3)
{
    struct file *ptFp   = NULL;
    char *       pu8Buf = NULL;
    u32          u32Len = arg3;
    mm_segment_t tFs;
    loff_t       tPos = 0;
    char *       pathname;

    pathname = (char *)ioremap_cache(arg1, MAX_DUMPBUF_DATA_PATHNAME);
    if (!pathname)
    {
        printk("proc_dumpbuf_data(S): map pathname fail\n");
        return -1;
    }

    CamOsMemInvalidate((void *)pathname, MAX_DUMPBUF_DATA_PATHNAME);

    if (strlen(pathname) == 0)
        sprintf(pathname, "/tmp/dumpbuf_data_%ld.yuv", jiffies);
    tFs = get_fs();
    set_fs(get_fs());
    ptFp = filp_open(pathname, O_RDWR | O_CREAT, 0777);

    pu8Buf = (char *)CamOsMemMap((ss_phys_addr_t)arg2, u32Len, false);
    if (!pu8Buf)
    {
        printk(KERN_ERR "proc_dumpbuf_data(S): map write buf fail\n");
        iounmap(pathname);
        filp_close(ptFp, NULL);
        return -1;
    }

    if (vfs_write(ptFp, pu8Buf, u32Len, &tPos) != u32Len)
    {
        printk(KERN_ERR "fwrite %s failed\n", pathname);
    }
    else
    {
        printk("dump file(%s) v1 ok ..............[len:%d]\r\n", pathname, arg3);
    }
    set_fs(tFs);
    CamOsMemInvalidate((void *)pu8Buf, u32Len);
    CamOsMemUnmap(pu8Buf, u32Len);
    iounmap(pathname);
    filp_close(ptFp, NULL);
    return 0;
}

static char         aDumpSeqFileName[MAX_DUMPBUF_DATA_PATHNAME];
static struct file *pDumpSeqFp;
static mm_segment_t tDumpSeqFs;
CamOsTsem_t         gtDumpSeqSem;
static u32          proc_dumpOpenFile(u32 arg0, u32 arg1, u32 arg2, u32 arg3)
{
    char *pathname;
    CamOsTsemInit(&gtDumpSeqSem, 1);
    pathname = (char *)ioremap_cache(arg1, MAX_DUMPBUF_DATA_PATHNAME);
    if (!pathname)
    {
        printk("proc_dumpOpenFile(S): map pathname fail\r\n");
        return -1;
    }

    CamOsMemInvalidate((void *)pathname, MAX_DUMPBUF_DATA_PATHNAME);
    if (strlen((char *)aDumpSeqFileName) == 0)
    {
        strcpy((char *)aDumpSeqFileName, pathname);
        tDumpSeqFs = get_fs();
        set_fs(get_fs());
        pDumpSeqFp = filp_open(pathname, O_RDWR | O_CREAT, 0777);
        if (!pDumpSeqFp)
        {
            printk("proc_dumpOpenFile(S): open file fail\r\n");
            iounmap(pathname);
            return -1;
        }
    }
    // printk("proc_dumpOpenFile: %s ok\r\n", aDumpSeqFileName);
    iounmap(pathname);
    return 0;
}

static u32 proc_dumpCloseFile(u32 arg0, u32 arg1, u32 arg2, u32 arg3)
{
    char *pathname;

    pathname = (char *)ioremap_cache(arg1, MAX_DUMPBUF_DATA_PATHNAME);
    if (!pathname)
    {
        printk("proc_dumpCloseFile(S): map pathname fail\r\n");
        return -1;
    }

    CamOsMemInvalidate((void *)pathname, MAX_DUMPBUF_DATA_PATHNAME);
    if (strcmp((char *)aDumpSeqFileName, pathname) == 0)
    {
        set_fs(tDumpSeqFs);
        filp_close(pDumpSeqFp, NULL);
    }
    // printk("proc_dumpCloseFile: %s ok\r\n", aDumpSeqFileName);
    iounmap(pathname);
    memset(aDumpSeqFileName, 0, sizeof(aDumpSeqFileName));
    CamOsTsemDeinit(&gtDumpSeqSem);
    return 0;
}

static u32 proc_dumpWriteFile(u32 arg0, u32 arg1, u32 arg2, u32 arg3)
{
    char *pathname;
    char *pu8Buf = NULL;
    u32   u32Len = arg3;

    CamOsTsemDown(&gtDumpSeqSem);
    pathname = (char *)ioremap_cache(arg1, MAX_DUMPBUF_DATA_PATHNAME);
    if (!pathname)
    {
        printk("proc_dumpWriteFile(S): map pathname fail\r\n");
        return -1;
    }

    CamOsMemInvalidate((void *)pathname, MAX_DUMPBUF_DATA_PATHNAME);
    if (strcmp((char *)aDumpSeqFileName, pathname) == 0)
    {
        pu8Buf = (char *)CamOsMemMap((ss_phys_addr_t)arg2, u32Len, false);
        if (!pu8Buf)
        {
            printk(KERN_ERR "proc_dumpWriteFile(S): map write buf fail\r\n");
            iounmap(pathname);
            return -1;
        }

        if (vfs_write(pDumpSeqFp, pu8Buf, u32Len, &pDumpSeqFp->f_pos) != u32Len)
        {
            printk(KERN_ERR "fwrite %s failed\n", aDumpSeqFileName);
        }
        else
        {
            // printk("dump file(%s) v1 ok ..............[len:%d]\r\n", aDumpSeqFileName, u32Len);
        }
    }
    CamOsMemInvalidate((void *)pu8Buf, u32Len);
    CamOsMemUnmap(pu8Buf, u32Len);
    iounmap(pathname);
    CamOsTsemUp(&gtDumpSeqSem);
    return 0;
}

static u32 proc_adddev(u32 arg0, u32 arg1, u32 arg2, u32 arg3)
{
    if (proc_node[arg1][arg2] == 0)
    {
        int                    i                              = 0;
        char                   entry_name[MAX_ENTRY_PATH_LEN] = {0};
        struct proc_EntryNode *entry_node                     = NULL;

        if (arg1 == E_MI_MODULE_ID_VDF)
            snprintf(entry_name, sizeof(entry_name), "/mi_modules/mi_%s/mi_%s%d", dev_list[E_MI_MODULE_ID_SHADOW],
                     dev_list[arg1], arg2);
        else if (arg1 == E_MI_MODULE_ID_WBC)
            snprintf(entry_name, sizeof(entry_name), "/mi_modules/mi_%s/mi_%s%d", dev_list[E_MI_MODULE_ID_DISP],
                     dev_list[arg1], arg2);
        else if (arg1 == E_MI_MODULE_ID_OS)
            snprintf(entry_name, sizeof(entry_name), "/mi_modules/mi_%s/mi_%s%d", dev_list[E_MI_MODULE_ID_DUMMY],
                     dev_list[arg1], arg2);
        else
            snprintf(entry_name, sizeof(entry_name), "/mi_modules/mi_%s/mi_%s%d", dev_list[arg1], dev_list[arg1], arg2);
        for (i = 0; i < MAX_PROC_LIST_ENTRY_SIZE; i++)
        {
            if (strlen(proc_list.eNode[i].entry_path) == 0)
                break;
            if (strcmp(proc_list.eNode[i].entry_path, entry_name) == 0)
            {
                proc_node[arg1][arg2] = 1;
                return 0;
            }
        }

        if (i == MAX_PROC_LIST_ENTRY_SIZE)
        {
            pr_err("adaptor proc entry full!!!\n");
            return -1;
        }
        entry_node      = &proc_list.eNode[i]; // unused node
        entry_node->dir = 0;
        strcpy(entry_node->entry_path, entry_name);
        proc_create_data(entry_name + 1, 0640, NULL, &g_FetchProcOps, entry_node);
        proc_node[arg1][arg2] = 1;
    }
    return 0;
}

static u32 proc_deldev(u32 arg0, u32 arg1, u32 arg2, u32 arg3)
{
    if (proc_node[arg1][arg2] == 1)
    {
        int  i                              = 0;
        char entry_name[MAX_ENTRY_PATH_LEN] = {0};

        if (arg1 == E_MI_MODULE_ID_VDF)
            snprintf(entry_name, sizeof(entry_name), "/mi_modules/mi_%s/mi_%s%d", dev_list[E_MI_MODULE_ID_SHADOW],
                     dev_list[arg1], arg2);
        else if (arg1 == E_MI_MODULE_ID_WBC)
            snprintf(entry_name, sizeof(entry_name), "/mi_modules/mi_%s/mi_%s%d", dev_list[E_MI_MODULE_ID_DISP],
                     dev_list[arg1], arg2);
        else if (arg1 == E_MI_MODULE_ID_OS)
            snprintf(entry_name, sizeof(entry_name), "/mi_modules/mi_%s/mi_%s%d", dev_list[E_MI_MODULE_ID_DUMMY],
                     dev_list[arg1], arg2);
        else
            snprintf(entry_name, sizeof(entry_name), "/mi_modules/mi_%s/mi_%s%d", dev_list[arg1], dev_list[arg1], arg2);
        for (i = 0; i < MAX_PROC_LIST_ENTRY_SIZE; i++)
        {
            if (strlen(proc_list.eNode[i].entry_path) == 0)
                continue;
            if (strcmp(proc_list.eNode[i].entry_path, entry_name) == 0)
            {
                memset(proc_list.eNode[i].entry_path, 0, MAX_ENTRY_PATH_LEN); // clear node
                remove_proc_entry(entry_name + 1, NULL);
                break;
            }
        }
        proc_node[arg1][arg2] = 0;
    }
    return 0;
}

static int __init linux_dualos_adaptor_init(void)
{
    int                            err = 0, i;
    struct dualos_adaptor_node *   anode;
    struct dualos_interos_datagram resp;
    int                            dst     = REMOTE_ADAPTOR_MANAGER_EPT_ADDR;
    int                            timeout = 5000;

    if (disable_os_adaptor)
        return 0;

    if (NULL == driver_find("sstar-lh-rtos-virtio", &platform_bus_type))
    {
        pr_err("[adaptor] %s: sstar-lh-rtos-virtio driver not found, disable os adaptor\n", __FUNCTION__);
        disable_os_adaptor = 1;
        return 0;
    }

    pr_info("[adaptor] wait adaptor announce...\n");
#if defined(CONFIG_SSTAR_LH_RTOS_VIRTIO)
    if (dualos_rpmsg_wait_remote_device(RPMsg_Device_LH_RTOS, 0, timeout))
    {
        pr_err("[adaptor] wait for remote adaptor announce timeout!");
        disable_os_adaptor = 1;
        return 0;
    }
#else
    while (timeout--)
    {
        if (dualos_rpmsg_get_remote_adaptor_status(EPT_OS_RTOS))
            break;
        if (timeout == 0)
        {
            pr_err("[adaptor] wait for remote adaptor announce timeout!");
            disable_os_adaptor = 1;
            return 0;
        }
        msleep(1);
    }
#endif
#ifdef CONFIG_SS_PROFILING_TIME
    recode_timestamp(__LINE__, "adaptor_announce_recv");
#endif
    printk("[adaptor] adaptor announce received!\n");

    anode = dualos_adaptor_node_alloc(0x0, 0xf, 0xff);
    if (IS_ERR_VALUE(anode))
        return PTR_ERR(anode);

    err =
        dualos_adaptor_interos_to(anode, dst, __MI_DEVICE_MMA_INFO, anode->mod_id, anode->dev_id, anode->chn_id, &resp);
    if (err < 0)
    {
        pr_err(
            "[adaptor] Adaptor node (%d,%d,%d,%d) get mma info "
            "error %d\n",
            anode->mod_id, anode->dev_id, anode->chn_id, anode->remote_addr, err);
        return err;
    }

    mma_base = resp.arg1;
    mma_size = resp.arg2;
    printk(
        "[adaptor] Adaptor node (%d,%d,%d,%d) get mma info mma_base=0x%lx,"
        "mma_size=0x%lx\n",
        anode->mod_id, anode->dev_id, anode->chn_id, anode->remote_addr, mma_base, mma_size);

    if (E_MI_MODULE_ID_MAX < sizeof(dev_list) / sizeof(*dev_list))
    {
        return -EINVAL;
    }
    err          = -EIO;
    device_major = register_chrdev(0, "dualos-adaptor", &fops);
    if (device_major <= 0)
        goto fail_register_chrdev;

    device_class = class_create(THIS_MODULE, "dualos-adaptor");
    err          = PTR_ERR(device_class);
    if (IS_ERR(device_class))
        goto fail_class_create;

    mutex_init(&proc_mutex);
    fetch_proc(anode, MI_PROC_FAKE_ID, NULL);

    for (i = 0; i < E_MI_MODULE_ID_MAX; ++i)
    {
        if (device_chrdev[i])
        {
            device_list[i] =
                device_create(device_class, NULL, MKDEV(device_major, i), device_list + i, "mi_%s", dev_list[i]);
        }
    }
    poll_major = register_chrdev(0, "poll-dev", &pfops);
    if (poll_major <= 0)
        goto fail_register_chrdev2;

    device_list[E_MI_MODULE_ID_MAX] =
        device_create(device_class, NULL, MKDEV(poll_major, 0), device_list + E_MI_MODULE_ID_MAX, "mi_poll");

    if (device_list[E_MI_MODULE_ID_MAX] == NULL)
        goto fail_create_poll_dev;

    device_cli = device_create(device_class, NULL, MKDEV(device_major, MI_CLI_FAKE_ID), device_cli, "mi_cli");
    if (device_cli == NULL)
    {
        printk("create mi_cli dev failed\n");
        goto fail_create_cli_dev;
    }

    CamOsTsemInit(&g_mma_map_hash_semphore, 1);
    rtk_res = request_mem_region(mma_base, mma_size, "mma");
    if (!rtk_res)
    {
        goto fail_mem_req;
    }
    printk("#map req:(0x%llx,0x%llx)\n", (u64)rtk_res->start, (u64)resource_size(rtk_res));
    debug_tools = proc_mkdir("adaptor-debug-tools", NULL);
    if (!debug_tools)
        goto fail_create_debug_tools;

    if (!debug_tool_create("syscall_cost", &syscall_cost_column))
        goto fail_create_syscall_cost;

    if (!debug_tool_create("syscall_freq", &syscall_freq_linear.dt))
        goto fail_create_syscall_freq;

    if (!debug_tool_create("info_tool", &info_tool.dt))
        goto fail_create_info_tool;

    printk("register notify\n");

    alkaid_poll_init();
    printk("poll init dev\n");

    CamInterOsSignalReg(INTEROS_SC_R2L_DUMPBUF_DATA, proc_dumpbuf_data, "dump buffer");
    CamInterOsSignalReg(INTEROS_SC_R2L_DUMPBUF_OPEN, proc_dumpOpenFile, "dump openfile");
    CamInterOsSignalReg(INTEROS_SC_R2L_DUMPBUF_CLOSE, proc_dumpCloseFile, "dump closefile");
    CamInterOsSignalReg(INTEROS_SC_R2L_DUMPBUF_WRITE, proc_dumpWriteFile, "dump writefile");
    CamInterOsSignalReg(INTEROS_SC_R2L_PROC_DEVADD, proc_adddev, "proc adddev");
    CamInterOsSignalReg(INTEROS_SC_R2L_PROC_DEVDEL, proc_deldev, "proc deldev");

    dualos_adaptor_node_release(anode);
    printk("linux-adaptor init success!(%s)\n", __TIME__);
    return 0;

fail_create_info_tool:
    pr_err("create info tool failed!\n");
    debug_tool_delete(&syscall_freq_linear.dt);
fail_create_syscall_freq:
    pr_err("create syscall freq analyzer failed!\n");
    debug_tool_delete(&syscall_cost_column);
fail_create_syscall_cost:
    pr_err("create syscall cost analyzer failed!\n");
    proc_remove(debug_tools);
fail_create_debug_tools:
    pr_err("proc mkdir failed!\n");
    release_mem_region(rtk_res->start, mma_size);
fail_mem_req:
    pr_err("request mem failed\n");
    device_destroy(device_class, MKDEV(device_major, MI_CLI_FAKE_ID));
fail_create_cli_dev:
    pr_err("create mi_cli dev failed\n");
    device_destroy(device_class, MKDEV(poll_major, 0));
fail_create_poll_dev:
    pr_err("create poll dev failed\n");
    unregister_chrdev(poll_major, "poll-dev");
fail_register_chrdev2:
    pr_err("unable to get mi device\n");
    for (i = 0; i < E_MI_MODULE_ID_MAX; ++i)
    {
        if (device_list[i])
            device_destroy(device_class, MKDEV(device_major, i));
    }
    unregister_chrdev(device_major, "dualos-adaptor");
fail_class_create:
    pr_err("fail create class\n");
fail_register_chrdev:
    pr_err("unable to get mi device\n");
    class_destroy(device_class);
    dualos_adaptor_node_release(anode);
    return err;
}
#ifdef CONFIG_DEFERRED_LINUX_DUALOS_ADAPTOR_INIT
deferred_module_init(linux_dualos_adaptor_init)
// module_init(linux_dualos_adaptor_init)
#else
module_init(linux_dualos_adaptor_init)
#endif

    static void __exit linux_adaptor_exit(void)
{
    int i;
    debug_tool_delete(&info_tool.dt);
    debug_tool_delete(&syscall_freq_linear.dt);
    debug_tool_delete(&syscall_cost_column);
    proc_remove(debug_tools);
    release_mem_region(rtk_res->start, mma_size);
    device_destroy(device_class, MKDEV(poll_major, 0));
    unregister_chrdev(poll_major, "poll-dev");
    for (i = 0; i < E_MI_MODULE_ID_MAX; ++i)
    {
        if (device_list[i])
            device_destroy(device_class, MKDEV(device_major, i));
    }
    unregister_chrdev(device_major, "dualos-adaptor");
    class_destroy(device_class);
    CamOsTsemDeinit(&g_mma_map_hash_semphore);
}
module_exit(linux_adaptor_exit)

    MODULE_LICENSE("GPL v2");
