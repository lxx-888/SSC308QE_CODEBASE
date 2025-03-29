/*
 * sstar_update_dq.c- Sigmastar
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
#include <linux/fs.h>
#include <linux/sched.h>
#include <linux/kthread.h>
#include <linux/threads.h>
#include <linux/delay.h>
#include <linux/time.h>
#include <linux/miscdevice.h>
#include <linux/dma-mapping.h>
#include <linux/of_device.h>
#include <linux/slab.h>
#include <linux/mm.h>
#if defined(CONFIG_SSTAR_DEVFREQ)
#include <linux/notifier.h>
#endif
#include "tsensor.h"
#include "ms_platform.h"
#include "registers.h"
#include "sstar_update_dq.h"
#include "hal_dq.h"
#if defined(CONFIG_SSTAR_DEVFREQ)
#include "drv_ddrfreq.h"
#endif

#if defined(CONFIG_SSTAR_DEVFREQ)
struct notifier_block miu_dq_nb;
#endif

// static struct device *     misc_dev       = NULL;
static struct task_struct *sstar_update_dq_task = NULL;
static int                 sstar_start_flag     = FALSE;
static int                 wait_time_period     = 60;
struct mutex               miudq_mutex;

#if 1
U32 gBaseAddr = 0xFD000000;
#define HAL_RIU_REG(addr)    (*(volatile U16 *)(gBaseAddr + ((addr & ~1) << 1)))
#define MHal_RIU_REG32(addr) (*(volatile U32 *)(gBaseAddr + (((addr) & ~1) << 1)))
#define MHal_Shift_BIT(x, y) (y << ((x & 1) << 3))
#define MHal_OUTREG32(x, y)  (MHal_RIU_REG32(x) = y)
#define MHal_SETREG32_BIT_OP(x, y) \
    MHal_OUTREG32(x, MHal_RIU_REG32(x) | (y << ((x & 1) << 3)) | ((u32)(y) << (16 + ((x & 1) << 3))))
#define MHal_CLRREG32_BIT_OP(x, y) \
    MHal_OUTREG32(x, (MHal_RIU_REG32(x) & ~(y << ((x & 1) << 3))) | ((u32)(y) << (16 + ((x & 1) << 3))))
#endif

static int sstar_update_dq_thread_by_dqsosc(void *pthread_data)
{
    // U16 scl_en;
    //  printk("enter update dq thread low high ..\n");
    U16 wait_period;

    Hal_ReloadPhaseRegs();
    Hal_save_dq_dm_phase();

    while (1)
    {
        if (kthread_should_stop())
        {
            break;
        }
        mutex_lock(&miudq_mutex);
        Hal_Set_DQS_Osc(0);
        mutex_unlock(&miudq_mutex);
        wait_period = 0;
        while (wait_period < (wait_time_period * 100))
        {
            // ssleep(1);
            msleep(10);
            wait_period++;

            if (kthread_should_stop())
                break;
#if 0
            if ((wait_period % 100) == 0)
                printk("*       %d\r\n", wait_period);
#endif
        }
    }

    Hal_restore_dq_dm_phase();
    printk("exit update-dq thread\n");
    sstar_start_flag = FALSE;
    return 0;
}

/*
 *    output: 0:pass, 1: create thread fail
 */
static int sstar_update_dq_create_dq_thread(void)
{
    if (!sstar_start_flag)
    {
        if (Hal_Check_LPDDR4())
        {
            sstar_update_dq_task = kthread_run(sstar_update_dq_thread_by_dqsosc, NULL, "sstar_update_dq_by_dqsosc");
            if (!sstar_update_dq_task)
            {
                printk("error create thread\n");
                return 1;
            }
            sstar_start_flag = TRUE;
        }
        else
            printk("DQS_OSC support LPDDR4 and LPDDR4X only\n");
    }
    else
    {
        printk("thread has been running now\n");
    }

    return 0;
}

static int sstar_update_dq_destroy_dq_thread(void)
{
    if (sstar_update_dq_task)
    {
        wake_up_process(sstar_update_dq_task);
        kthread_stop(sstar_update_dq_task);
    }

    while (sstar_start_flag)
        usleep_range(50, 100);

    sstar_update_dq_task = NULL;
    sstar_start_flag     = FALSE;

    return 0;
}

#if defined(CONFIG_SSTAR_DEVFREQ)
static int sstar_update_dq_notifier_callback(struct notifier_block *this, unsigned long index, void *data)
{
    switch (index)
    {
        case MIU_NOTIFY_DQ_START:
            // for dbg
            // printk("MIU_NOTIFY_DQ_START\n");

            sstar_update_dq_create_dq_thread();
            break;
        case MIU_NOTIFY_DQ_STOP:
            // for dbg
            // printk("MIU_NOTIFY_DQ_STOP\n");

            sstar_update_dq_destroy_dq_thread();
            break;
        default:
            break;
    }

    // printk("##\n");
    return NOTIFY_DONE;
}
#endif

/*
 * cat /dev/sstar_update_dq
 */
static ssize_t sstar_update_dq_read(struct file *file, char __user *buf, size_t count, loff_t *ppos)
{
    printk("wait_time: %d, dcdl_scl_mode: %X\n", wait_time_period, Hal_Get_DCDL_Mode());
    return 0;
}

/*
 * echo 0 > /dev/sstar_update_dq
 */
static ssize_t sstar_update_dq_write(struct file *file, const char __user *buf, size_t count, loff_t *ppos)
{
    int   ret;
    char  cmd[32];
    char *ps8Kbuf  = NULL;
    int   param[5] = {0, 0, 0, 0, 0};

    ps8Kbuf = vmalloc(count);
    if (!copy_from_user((void *)ps8Kbuf, (void __user *)buf, count))
    {
        sscanf(ps8Kbuf, "%s %d %d %d", cmd, &param[0], &param[1], &param[2]);
        printk("cmd:%s param:%X, %X, %X\n", cmd, param[0], param[1], param[2]);
        ret = count;
    }
    else
    {
        printk("[%s] copy user data failed!!!\n", __FUNCTION__);
        ret = 0;
        goto EXIT_WRITE;
    }

    if (param[0] < 0)
    {
        printk("[%s] invalid param!!!\n", __FUNCTION__);
        goto EXIT_WRITE;
    }

    if (!strcmp(cmd, "Start"))
    {
        if (sstar_update_dq_create_dq_thread())
            goto EXIT_WRITE;
    }
    else if (!strcmp(cmd, "Stop"))
    {
        sstar_update_dq_destroy_dq_thread();
    }
    else if (!strcmp(cmd, "WaitTime"))
    {
        printk("set wait time %d sec\r\n", param[0]);
        wait_time_period = param[0];
        // Hal_Set_WaitTime(param[0]);
    }
    else if (!strcmp(cmd, "dcdl_scl_mode"))
    {
        sscanf(buf, "%s %X", cmd, &param[0]);
        printk("dcdl_scl_mode %X\r\n", param[0]);
        Hal_Set_DCDL_Mode(param[0]);
    }
    else if (!strcmp(cmd, "setfreq"))
    {
        // printk("dram freq: %d\r\n", get_ddr_freq());

        // printk("dqs tree dly: %d\r\n", get_ddr_DQS_treedly(3750));
        if ((param[0] > 0) && (Hal_Check_LPDDR4()))
            Hal_Set_Freq_DQPh(0, param[0]);
        else
            printk("Set freq %d is not support.\r\n", param[0]);
    }
    else if (!strcmp(cmd, "testcmd"))
    {
        U32 x, y;

        sscanf(buf, "%s %d %X %X", cmd, &param[0], &param[1], &param[2]);
        printk("cmd=%s, bank: 0x%X, bit: 0x%4X -> 0x%4X\r\n", &cmd[0], param[1], param[2],
               MHal_Shift_BIT(param[1], param[2]));

        x = param[1];
        y = param[2];
        printk("%s, x=%X y=%X\r\n", __FUNCTION__, x, y);
        printk("%s, %X\r\n", __FUNCTION__,
               (MHal_RIU_REG32(x) & ~(y << ((x & 1) << 3))) | ((u32)(y) << (16 + ((x & 1) << 3))));

        if (param[0] == 0)
        {
            printk("1, read reg: %04X\r\n", HAL_RIU_REG(param[1]));
            MHal_SETREG32_BIT_OP(param[1], param[2]);
            printk("1-1, set %X. read reg: %04X\r\n", param[1], HAL_RIU_REG(param[1]));
        }
        else if (param[0] == 1)
        {
            printk("2, read reg: %04X\r\n", HAL_RIU_REG(param[1]));
            MHal_CLRREG32_BIT_OP(param[1], param[2]);
            printk("2-1, clr %X. read reg: %04X\r\n", param[1], HAL_RIU_REG(param[1]));
        }
    }
    else if (!strcmp(cmd, "set_phsc"))
    {
        mutex_lock(&miudq_mutex);
        Hal_Set_DvsPhaseScaling(0);
        mutex_unlock(&miudq_mutex);
    }
    else
    {
        printk("[%s] unknow command!!!\n", __FUNCTION__);
    }

EXIT_WRITE:
    vfree(ps8Kbuf);
    ps8Kbuf = NULL;

    return ret;
}

static const struct file_operations sstar_update_dq_fops = {
    .owner = THIS_MODULE,
    .read  = sstar_update_dq_read,
    .write = sstar_update_dq_write,
};

static struct miscdevice sstar_update_dq_misc = {
    .name = "sstar_update_dq",
    .fops = &sstar_update_dq_fops,
};

int miudq_dvs_phase_scaling(void)
{
    int ret = 0;

    mutex_lock(&miudq_mutex);
    ret = Hal_Set_DvsPhaseScaling(0);
    mutex_unlock(&miudq_mutex);

    return ret;
}
EXPORT_SYMBOL(miudq_dvs_phase_scaling);

static int __init sstar_update_dq_init(void)
{
    int ret = 0;

    ret = misc_register(&sstar_update_dq_misc);
    if (unlikely(ret))
    {
        pr_err("failed to register misc device!\n");
        return ret;
    }
#if 0
    misc_dev                    = sstar_update_dq_misc.this_device;
    misc_dev->coherent_dma_mask = ~0;
    _dev_info(misc_dev, "registered.\n");
#endif

    mutex_init(&miudq_mutex);
    if (Hal_Check_LPDDR4())
    {
        Hal_DQ_Init();
        if (sstar_update_dq_create_dq_thread())
        {
            return ret;
        }
    }
    else
    {
        Hal_DQ_Init();
    }

#if defined(CONFIG_SSTAR_DEVFREQ)
    miu_dq_nb.notifier_call = sstar_update_dq_notifier_callback;
    devfreq_notifier_register_nb(&miu_dq_nb);
#endif

    printk("[%s]\n", __FUNCTION__);
    return ret;
}
subsys_initcall(sstar_update_dq_init);
#if 0
module_init(sstar_update_dq_init);


static void __exit sstar_update_dq_exit(void)
{
    sstar_update_dq_destroy_dq_thread();

    misc_deregister(&sstar_update_dq_misc);
    Hal_DQ_Exit();

#if defined(CONFIG_SSTAR_DEVFREQ)
    devfreq_notifier_unregister_nb(&miu_dq_nb);
#endif

    printk("[%s]\n", __FUNCTION__);
}
module_exit(sstar_update_dq_exit);
#endif

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("kernel module to update dq");
MODULE_ALIAS("SSTAR Update DQ");
