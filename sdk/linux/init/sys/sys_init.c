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
#include <mi_common.h>
#include <mi_common_internal.h>
#include <mi_common_datatype.h>
#include <mi_sys.h>

#include <linux/module.h>
#include <linux/sched.h>
#include <linux/sched/debug.h>
#include <linux/mm.h>
#include <linux/file.h>
#include <linux/sched/task_stack.h>
#include <asm/pgtable.h>
#include <linux/of_platform.h>

#include <cam_proc_wrapper.h>
#include <cam_sysfs.h>

#include "common_print.h"
#include <linux/freezer.h>
#include <linux/suspend.h>
#include <cam_fs_wrapper.h>

extern pgd_t *msys_get_pgd_offset_k(unsigned long va);

void MI_SYS_SetFreezable(void)
{
#if defined(CONFIG_ENABLE_MI_NORMAL_STR)
    set_freezable();
#endif
}

MI_S32 MI_SYS_TryToFreeze(void)
{
#if defined(CONFIG_ENABLE_MI_NORMAL_STR)
    return CamTryToFreeze();
#else
    return MI_SUCCESS;
#endif
}

#if defined(CONFIG_ENABLE_MI_NORMAL_STR)
extern MI_S32 MI_SYS_PM_StrSuspendPrepare(void);
extern MI_S32 MI_SYS_PM_StrPostSuspend(void);

static MI_S32 MI_SYS_IMPL_PmNotify(struct notifier_block *b, unsigned long v, void *d)
{
    switch (v)
    {
        case PM_SUSPEND_PREPARE:
            if(MI_SUCCESS != MI_SYS_PM_StrSuspendPrepare())
            {
                DBG_ERR("PM notify suspend prepare failed\n");
                return NOTIFY_BAD;
            }
            break;
        case PM_POST_SUSPEND:
            MI_SYS_PM_StrPostSuspend();
            break;
        default:
            return NOTIFY_DONE;
    }

    return NOTIFY_OK;
}

static struct notifier_block mi_pm_notify_block = {
    .notifier_call = MI_SYS_IMPL_PmNotify,
};
#endif

MI_S32 MI_SYS_IMPL_InitPmNotify(void)
{
#if defined(CONFIG_ENABLE_MI_NORMAL_STR)
    return CamRegisterPmNotifier(&mi_pm_notify_block);
#else
    return MI_SUCCESS;
#endif
}

MI_S32 MI_SYS_IMPL_DeInitPmNotify(void)
{
#if defined(CONFIG_ENABLE_MI_NORMAL_STR)
    return CamUnRegisterPmNotifier(&mi_pm_notify_block);
#else
    return MI_SUCCESS;
#endif
}

MI_U32 MI_SYS_FreeReservedArea(ss_phys_addr_t start, ss_phys_addr_t end, int poison, const char *s)
{
    return free_reserved_area(__va(start), __va(end), poison, s);
}

MI_S32 MI_SYS_GetPid(void)
{
    if (current->mm)
        return current->tgid;
    return -1;
}

void * MI_SYS_Buf_GetCurUserMapCtx(void)
{
    return CURRENT_CTX;
}

void MI_SYS_ShowTask(struct task_struct *p)
{
    int           ppid;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 1, 0)
     unsigned long state = READ_ONCE(p->__state);
#else
     unsigned long state = p->state;
#endif

    if (!try_get_task_stack(p))
        return;

    printk(KERN_INFO "%-15.15s %c", p->comm, task_state_to_char(p));

    if (state == TASK_RUNNING)
        CamOsPrintf(KERN_CONT "  running task    ");

    ppid = 0;
    if (pid_alive(p))
    {
#ifndef CONFIG_PROVE_RCU
        ppid = task_pid_nr(rcu_dereference(p->real_parent));
#endif
    }

    CamOsPrintf(KERN_CONT "%5d %6d 0x%08lx\n", task_pid_nr(p), ppid, (unsigned long)task_thread_info(p)->flags);

    // print_worker_info(KERN_INFO, p);
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0)
        show_stack(p, NULL, KERN_DEFAULT);
#else
        show_stack(p, NULL);
#endif
    // put_task_stack(p);
}

void MI_SYS_ShowAllTask(void)
{
    struct task_struct *g, *p;

    printk(KERN_INFO "  task                PC stack   pid father\n");

    for_each_process_thread(g, p)
    {
        MI_SYS_ShowTask(p);
    }
}

#define CMD_LINE_LETH 512
char *parese_Cmdline(char *str)
{
    CamFsFd fd;
    int          ret  = 0;
    unsigned int n    = 0;
    char *       pCmdline = NULL;
    char         cmdline[CMD_LINE_LETH];
    static char *pCmd_Section = NULL;

    if (CamFsOpen(&fd, "/proc/cmdline", CAM_FS_O_RDONLY, 0644) != CAM_FS_OK)
    {
        CamOsPrintf("\33[0;36m read /proc/cmdline failed!\n");
        return NULL;
    }

    CamFsSeek(fd, 0, SEEK_SET);
    ret = CamFsRead(fd, cmdline, CMD_LINE_LETH - 1);
    CamFsClose(fd);
    if (ret == 0)
    {
        CamOsPrintf("\33[0;36m read /proc/cmdline failed!\n");
        return NULL;
    }

    cmdline[CMD_LINE_LETH - 1] = '\0';
    pCmdline                   = strstr(cmdline, str);
    if (NULL == pCmdline)
    {
        CamOsPrintf("\33[0;36m Can't find str:%s in cmdline failed!\n", str);
        return NULL;
    }
    pCmdline = strstr(pCmdline, "=");
    while (*(pCmdline + n) != '\0' && *(pCmdline + n) != ' ')
    {
        n++;
    }

    if (n < 1)
    {
        return NULL;
    }

    if (NULL == pCmd_Section)
    {
        pCmd_Section = CamOsMemAlloc(n * sizeof(char));
        if (NULL == pCmd_Section)
        {
            CamOsPrintf("\33[0;36m malloc fail.. str:%s in cmdline failed!\n", str);
            return NULL;
        }
    }
    pCmdline += 1;
    strncpy(pCmd_Section, pCmdline, n - 1);
    pCmd_Section[n - 1] = '\0';

    CamOsPrintf("function:%s,pCmd_Section:%s\n", __FUNCTION__, pCmd_Section);
    return pCmd_Section;
}

typedef void (*process_func)(void *private_data);

MI_S32 MI_SYS_IMPL_ProcessFilePrivate(MI_S32 s32Fd, process_func func)
{
    struct file *pFile = NULL;
    if (s32Fd < 0)
        return MI_ERR_SYS_ILLEGAL_PARAM;
    pFile = fget(s32Fd);
    if (!pFile)
        return MI_ERR_SYS_FAILED;
    if (!pFile->private_data)
    {
        fput(pFile);
        return MI_SUCCESS;
    }
    if (pFile->private_data)
    {
        func(pFile->private_data);
    }
    fput(pFile);
    return MI_SUCCESS;
}

extern int                        mstar_driver_boot_mmap_reserved_buffer_num; // NOLINT
extern struct MMA_BootArgs_Config mmap_reserved_config[MAX_MMA_AREAS];        // NOLINT
MI_S32 _MI_SYS_IMPL_FreeMMAPReservedArea(MI_U8 *pMMAPReserverAreaName)
{
    int i = 0;
    if (pMMAPReserverAreaName == NULL)
        return MI_ERR_SYS_NULL_PTR;
    for (; i < mstar_driver_boot_mmap_reserved_buffer_num; i++)
    {
        if (strcmp(mmap_reserved_config[i].name, (char *)pMMAPReserverAreaName) == 0)
        {
            if (mmap_reserved_config[i].reserved_start != 0)
            {
                free_reserved_area(__va(mmap_reserved_config[i].reserved_start),
                                   __va(mmap_reserved_config[i].reserved_start + mmap_reserved_config[i].size), 256,
                                   (char *)pMMAPReserverAreaName);
                mmap_reserved_config[i].reserved_start = 0;
            }
            return MI_SUCCESS;
        }
    }
    return MI_ERR_SYS_UNEXIST;
}

MI_BOOL MI_SYS_Is_Va_Invalid(void *va, bool bUserVa)
{
    pgd_t *pgd = NULL;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,10,61)
    p4d_t *p4d = NULL;
#endif
    pud_t *pud = NULL;
    pmd_t *pmd = NULL;
    pte_t *pte = NULL;
    struct task_struct *tsk = current;

    if (bUserVa)
    {
        if (va == NULL || tsk->mm == NULL)
        {
            goto EXIT;
        }
        pgd = pgd_offset(tsk->mm,(MI_VIRT)va); // get Page Global Directory
    }
    else
    {
        if (va == NULL)
        {
            goto EXIT;
        }
        pgd = msys_get_pgd_offset_k((MI_VIRT)va); // get Page Global Directory
    }

    if (pgd_none(*pgd) || pgd_bad(*pgd))
    {
        DBG_INFO("pgd is NULL!\n");
        goto EXIT;
    }

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,10,61)
    /* The additional page table level -- p4d -- is enable to support 5 level paging */
    p4d = p4d_offset(pgd, (MI_VIRT)va); // get p4d
    if (p4d_none(*p4d) || p4d_bad(*p4d))
    {
        DBG_INFO("p4d is NULL!\n");
        goto EXIT;
    }

    pud = pud_offset(p4d,(MI_VIRT)va);
#else
    pud = pud_offset(pgd,(MI_VIRT)va);  // get Page Upper Directory
#endif
    if (pud_none(*pud) || pud_bad(*pud))
    {
        DBG_INFO("pud is NULL!\n");
        goto EXIT;
    }

    pmd = pmd_offset(pud,(MI_VIRT)va);  // get Page Middle Directory
    if (pmd_none(*pmd) || pmd_bad(*pmd))
    {
        DBG_INFO("pmd is NULL!\n");
        goto EXIT;
    }

    if (PageHighMem(pfn_to_page(pmd_val(*pmd) >> PAGE_SHIFT)))
    {
        DBG_INFO("It's high memory!\n");
    }

    pte = pte_offset_map(pmd, (MI_VIRT)va);  // get Page Table Entry

    if (!pte_present(*pte) || pte_none(*pte))
    {
        DBG_INFO("pte is NULL!\n");
        pte_unmap(pte);
        goto EXIT;
    }
    pte_unmap(pte);

    return FALSE;
EXIT:
    DBG_INFO("mm:%p, active_mm:%p, pid:%d, tgid:%d, flags:%d, name:%s\n",
        tsk->mm, tsk->active_mm, tsk->pid, tsk->tgid, (tsk->flags & PF_EXITING), tsk->comm);
    return TRUE;
}

MI_BOOL MI_SYS_Vma_Is_Va_Invalid(void *va, MI_U32 u32Size)
{
    struct task_struct *tsk = current;
    MI_VIRT pVirAddr = (MI_VIRT)va;
    struct vm_area_struct *vma;
    MI_S32 s32Ret = MI_SUCCESS;

    if (MI_SYS_Is_Va_Invalid(va, TRUE))
    {
        goto EXIT;
    }

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,8,0)
    down_write(&tsk->mm->mmap_lock);
#else
    down_write(&tsk->mm->mmap_sem);
#endif
    vma    = find_vma(tsk->mm, pVirAddr);
    if (!vma)
    {
        DBG_WRN("vma is null\n");
        s32Ret = MI_ERR_SYS_FAILED;
    }
    else if (vma->vm_end - pVirAddr < u32Size)
    {
        DBG_WRN("flush addr 0x%lx, size 0x%x, vma start addr 0x%lx, vma end 0x%lx\n",
            pVirAddr, u32Size, vma->vm_start, vma->vm_end);
        s32Ret = MI_ERR_SYS_FAILED;
    }
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,8,0)
    up_write(&current->mm->mmap_lock);
#else
    up_write(&current->mm->mmap_sem);
#endif
    if(s32Ret != MI_SUCCESS)
    {
        goto EXIT;
    }

    return FALSE;
EXIT:
    DBG_INFO("mm:%p, active_mm:%p, pid:%d, tgid:%d, flags:%d, name:%s\n",
        tsk->mm, tsk->active_mm, tsk->pid, tsk->tgid, (tsk->flags & PF_EXITING), tsk->comm);
    return TRUE;
}

MI_BOOL MI_SYS_Is_KernVa_Invalid(void *va, MI_U32 u32Size)
{
    /* kernel space va >= PAGE_OFFSET */
    if (va < (void*)PAGE_OFFSET)
    {
        DBG_ERR("va:%px is not in kernel range, u32Size :%d %u\n", va, u32Size);
        return TRUE;
    }

    return MI_SYS_Is_Va_Invalid(va, FALSE);
}

// FIXME: MI_SYS_Proc_Print may not export
MI_S32 MI_SYS_Proc_Print(int level, int logv, const char *kmsgfmt, const char *color, const char *fmt,
                         void *fP_printMatch, const char *file, ...);
CamProcEntry_t *MI_SYS_Proc_CommonRealGetSelfDir(CamProcEntry_t *proc_dir_entry);

MI_BOOL MI_SYS_Pass_IsUnmapVmallocAddr(void *pVirtAddr)
{
#ifdef __KERNEL__
    MI_VIRT virtAddr = (MI_VIRT)pVirtAddr;

    if ((virtAddr >= VMALLOC_START && virtAddr < VMALLOC_END) && !vmalloc_to_page((void *)pVirtAddr))
    {
        DBG_ERR(
            "In %s, output struct pointer unmap, maybe you meet race condition between dev unregister & buf "
            "release!!!\n",
            __FUNCTION__);
        return TRUE;
    }
#else
    UNUSED(pVirtAddr);
#endif

    return FALSE;
}

MI_S32 MI_SYS_IMPL_Va2Pa(void *pVirAddr, MI_PHY *pPhyAddr)
{
#ifdef CAM_OS_RTK
    extern MI_PHY MI_DEVICE_Vir2Phy(void *va);

    if(!pVirAddr || !pPhyAddr)
    {
        DBG_ERR("%s va:%p, pa:%p\n", pVirAddr, pPhyAddr);
        return MI_ERR_SYS_ILLEGAL_PARAM;
    }
    *pPhyAddr = CamOsMemPhysToMiu(MI_DEVICE_Vir2Phy(pVirAddr));
    /* FIXME: add return check. */
    return MI_SUCCESS;
#else
    MI_S32                 ret                              = MI_ERR_SYS_UNEXIST;
    unsigned long          pfn;
    unsigned long          addr;
    struct vm_area_struct *vma;

    *pPhyAddr = 0;

    if (current->mm) // if current->mm == NULL ,user app already exit
    {
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0)
        down_write(&current->mm->mmap_lock);
#else
        down_write(&current->mm->mmap_sem);
#endif
        addr = (unsigned long)(pVirAddr);
        vma  = find_vma(current->mm, addr);
        if (!vma)
        {
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0)
            up_write(&current->mm->mmap_lock);
#else
            up_write(&current->mm->mmap_sem);
#endif
            DBG_ERR("not find vma!!\n");
            return MI_ERR_SYS_ILLEGAL_PARAM;
        }

        if(addr < vma->vm_start || addr > vma->vm_end)
        {
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0)
            up_write(&current->mm->mmap_lock);
#else
            up_write(&current->mm->mmap_sem);
#endif
            DBG_WRN("vma mismatch\naddr:%lx, start:%lx, end:%lx.\n", addr, vma->vm_start, vma->vm_end);
            return MI_ERR_SYS_ILLEGAL_PARAM;
        }

        ret = follow_pfn(vma, addr, &pfn);
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0)
        up_write(&current->mm->mmap_lock);
#else
        up_write(&current->mm->mmap_sem);
#endif

        if (ret)
        {
            DBG_WRN("follow_pfn return:%d!!\n", ret);
            return MI_ERR_SYS_ILLEGAL_PARAM;
        }

        *pPhyAddr = CamOsMemPhysToMiu(__pfn_to_phys(pfn) + (addr & ~PAGE_MASK));

#ifdef MI_SYS_MIU_MMU
        if (*pPhyAddr < MMU_MMAP_START_ADDR && MI_SYS_Mma_IsEnableMMU())
        {
            /* pa turn to vpa. */
            *pPhyAddr = MI_SYS_Mma_Pa2Vpa(*pPhyAddr);
            DBG_INFO("PhyAddr：%llx\n", *pPhyAddr);
        }
#endif

    }

    return ret;
#endif
}

typedef enum
{
    E_MI_COMMON_POLL_NOT_READY         = (0x0),
    E_MI_COMMON_FRAME_READY_FOR_READ   = (0x1 << 0),
    E_MI_COMMON_BUFFER_READY_FOR_WRITE = (0x1 << 1),
} MI_COMMON_PollFlag_e;
typedef struct MI_COMMON_PollFile_s
{
    MI_COMMON_PollFlag_e (*fState)(struct MI_COMMON_PollFile_s *);
    void (*fRelease)(struct MI_COMMON_PollFile_s *);
    void *private;
    void *private1;
} MI_COMMON_PollFile_t;
MI_S32 MI_COMMON_WakeUpClient(MI_COMMON_PollFile_t *pstPollFile);
int MI_COMMON_ReleasePollFd(MI_COMMON_PollFile_t **ppstPollFile);
MI_S32 MI_COMMON_GetPoll(MI_COMMON_PollFile_t **ppstPollFile,
                              MI_COMMON_PollFlag_e (*fState)(MI_COMMON_PollFile_t *),
                              void (*fRelease)(MI_COMMON_PollFile_t *));
void *MI_SYS_Proc_FuncName(void);

MI_S32 MI_SYS_GetIrqNum(const char* irqName, MI_U32* pu32IrqNum)
{
    struct device_node*     devNode = NULL;
    struct platform_device* pdev;

    if (pu32IrqNum == NULL)
    {
        return MI_ERR_SYS_FAILED;
    }

    devNode = of_find_compatible_node(NULL, NULL, irqName);
    if (!devNode)
    {
        return -ENODEV;
    }

    pdev = of_find_device_by_node(devNode);
    if (!pdev)
    {
        of_node_put(devNode);
        return -ENODEV;
    }

    *pu32IrqNum = CamIrqOfParseAndMap(pdev->dev.of_node, 0);

    return MI_SUCCESS;
}

void *MI_SYS_FileGet(MI_S32 s32Fd)
{
    return fget(s32Fd);
}

void *MI_SYS_FilePrivGet(void *filp)
{
    struct file *f = filp;
    return f->private_data;
}

void MI_SYS_FilePut(void *filp)
{
    fput(filp);
}

struct mi_sys_internal_apis_s;
typedef struct mi_sys_internal_apis_s mi_sys_internal_apis_t;


// mi_sys_internal
extern mi_sys_internal_apis_t g_mi_sys_internal_apis;
extern void MI_SYS_Proc_BugOn(MI_U32 u32Line);

DECLEAR_MODULE_INIT_EXIT

EXPORT_SYMBOL(g_mi_sys_internal_apis);
EXPORT_SYMBOL(MI_SYS_Proc_Print);
EXPORT_SYMBOL(MI_SYS_Proc_FuncName);
EXPORT_SYMBOL(MI_SYS_ShowAllTask);
EXPORT_SYMBOL(MI_SYS_Proc_BugOn);
#ifndef COMBINE_MI_MODULE
EXPORT_SYMBOL(MI_SYS_GetIrqNum);
EXPORT_SYMBOL(MI_COMMON_WakeUpClient);
EXPORT_SYMBOL(MI_COMMON_ReleasePollFd);
EXPORT_SYMBOL(MI_COMMON_GetPoll);
EXPORT_SYMBOL(MI_SYS_GetPid);
EXPORT_SYMBOL(MI_SYS_Proc_CommonRealGetSelfDir);
#endif

MI_MODULE_LICENSE();
MODULE_AUTHOR("colin.hu <colin.hu@sigmastar.com.cn>");

