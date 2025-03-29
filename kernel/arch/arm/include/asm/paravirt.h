/* SPDX-License-Identifier: GPL-2.0 */
#ifndef _ASM_ARM_PARAVIRT_H
#define _ASM_ARM_PARAVIRT_H

#ifdef CONFIG_PARAVIRT

#if defined(CONFIG_LH_RTOS) || defined(CONFIG_LINUX_ON_SS_HYPERVISOR)
#include <linux/init.h>
#endif

struct static_key;
extern struct static_key paravirt_steal_enabled;
extern struct static_key paravirt_steal_rq_enabled;

struct pv_time_ops {
	unsigned long long (*steal_clock)(int cpu);
};

struct paravirt_patch_template {
	struct pv_time_ops time;
};

extern struct paravirt_patch_template pv_ops;

static inline u64 paravirt_steal_clock(int cpu)
{
	return pv_ops.time.steal_clock(cpu);
}

#if defined(CONFIG_LH_RTOS) || defined(CONFIG_LINUX_ON_SS_HYPERVISOR)
int __init pv_time_init(void);
#endif

#endif

#endif
