// SPDX-License-Identifier: GPL-2.0-only
/*
 *
 * Copyright (C) 2013 Citrix Systems
 *
 * Author: Stefano Stabellini <stefano.stabellini@eu.citrix.com>
 */

#include <linux/export.h>
#include <linux/jump_label.h>
#include <linux/types.h>
#include <linux/ioport.h>
#include <asm/io.h>
#include <asm/paravirt.h>
#if defined(CONFIG_PARAVIRT)
#if defined(CONFIG_LH_RTOS) || defined(CONFIG_LINUX_ON_SS_HYPERVISOR)
#include <clocksource/arm_arch_timer.h>
#include "../../../drivers/sstar/include/drv_dualos.h"
#endif
#endif

struct static_key paravirt_steal_enabled;
struct static_key paravirt_steal_rq_enabled;

struct paravirt_patch_template pv_ops;
EXPORT_SYMBOL_GPL(pv_ops);

#if defined(CONFIG_PARAVIRT) && (defined(CONFIG_LH_RTOS) || defined(CONFIG_LINUX_ON_SS_HYPERVISOR))
static bool steal_acc = true;
static int __init parse_no_stealacc(char *arg)
{
	steal_acc = false;
	return 0;
}

early_param("no-steal-acc", parse_no_stealacc);

#if defined(CONFIG_LINUX_ON_SS_HYPERVISOR)
/*****************************************
tick buf content:
       vm0    vm1    ... vmN    idle
vcpu0  [tick] [tick] ... [tick] [tick]
vcpu1  [tick] [tick] ... [tick] [tick]
...                  ...
vcpuN  [tick] [tick] ... [tick] [tick]
*****************************************/
static u64 *tick_buf = NULL;
static struct resource *tick_buf_res;
static int vm_num = 0, hyp_cpu_num = 0;
static int vm_id[CONFIG_NR_CPUS] = {[0 ... (CONFIG_NR_CPUS-1)] = -1};
static int vcpu_id[CONFIG_NR_CPUS] = {[0 ... (CONFIG_NR_CPUS-1)] = -1};
#endif

/* return stolen time in ns by asking the hypervisor */
static u64 pv_steal_clock(int cpu)
{
#if defined(CONFIG_LINUX_ON_SS_HYPERVISOR)
    int vm_idx;
    u64 steal_tick = 0;

    if (!tick_buf)
        return 0;

    if (vm_id[cpu] == -1)
    {
        vm_id[cpu] = (int)signal_hyp(HYPERCALL_FID_VM_QUERY, HYPERCALL_SUBCMD_VM_QUERY_VM_ID, cpu, 0);
    }

    if (vcpu_id[cpu] == -1)
    {
        vcpu_id[cpu] = (int)signal_hyp(HYPERCALL_FID_VM_QUERY, HYPERCALL_SUBCMD_VM_QUERY_VCPU_ID, cpu, 0);
    }

    for (vm_idx = 0; vm_idx < vm_num; vm_idx++)
    {
        if (vm_idx != vm_id[cpu])
            steal_tick += *(tick_buf + ((vm_num + 1) * vcpu_id[cpu]) + vm_idx);
    }

    return steal_tick?steal_tick * (1000000000 / arch_timer_get_rate()):0;
#elif defined(CONFIG_LH_RTOS)
	rtkinfo_t *rtk = get_rtkinfo();

	if (rtk && (rtk->main_code == cpu))
	{
        /* rtk->spent is arch timer ticks, convert to ns. */
        return rtk->spent * (1000000000 / arch_timer_get_rate());
	}
#endif

	return 0;
}

int __init pv_time_init(void)
{
	pv_ops.time.steal_clock = pv_steal_clock;

	static_key_slow_inc(&paravirt_steal_enabled);
	if (steal_acc)
    {
		static_key_slow_inc(&paravirt_steal_rq_enabled);
    }
#if defined(CONFIG_LINUX_ON_SS_HYPERVISOR)
    if (tick_buf == NULL)
    {
        unsigned long tick_buf_phy_addr = 0;

        vm_num = (int)signal_hyp(HYPERCALL_FID_VM_QUERY, HYPERCALL_SUBCMD_VM_QUERY_VM_NUM, 0, 0);
        hyp_cpu_num = (int)signal_hyp(HYPERCALL_FID_VM_QUERY, HYPERCALL_SUBCMD_VM_QUERY_CPU_NUM, 0, 0);
        tick_buf_phy_addr = signal_hyp(HYPERCALL_FID_VM_QUERY, HYPERCALL_SUBCMD_VM_QUERY_TICK_BUF, 0, 0);
        tick_buf_res = request_mem_region(tick_buf_phy_addr, sizeof(u64) * (vm_num + 1) * hyp_cpu_num, "tickbuf");
        tick_buf = ioremap_wc(tick_buf_res->start, resource_size(tick_buf_res));
        if (!tick_buf)
        {
            printk(KERN_ERR "Failed to map tick buffer\n");
            return 0;
        }
        //printk(KERN_EMERG"[%d] pv_steal_clock, phy addr = 0x%lx\n", cpu, tick_buf_phy_addr);
    }
#endif
	pr_info("using stolen time PV\n");

	return 0;
}
#endif
