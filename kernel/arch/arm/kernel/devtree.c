// SPDX-License-Identifier: GPL-2.0-only
/*
 *  linux/arch/arm/kernel/devtree.c
 *
 *  Copyright (C) 2009 Canonical Ltd. <jeremy.kerr@canonical.com>
 */

#include <linux/init.h>
#include <linux/export.h>
#include <linux/errno.h>
#include <linux/types.h>
#include <linux/memblock.h>
#include <linux/of.h>
#include <linux/of_fdt.h>
#include <linux/of_irq.h>
#include <linux/of_platform.h>
#include <linux/smp.h>

#include <asm/cputype.h>
#include <asm/setup.h>
#include <asm/page.h>
#include <asm/prom.h>
#include <asm/smp_plat.h>
#include <asm/mach/arch.h>
#include <asm/mach-types.h>
#if defined(CONFIG_LINUX_ON_SS_HYPERVISOR)
#include "drv_dualos.h"
#endif

#ifdef CONFIG_SSTAR_BUILTIN_DTB
extern void *builtin_dtb_start;
#endif

#ifdef CONFIG_SMP
extern struct of_cpu_method __cpu_method_of_table[];

static const struct of_cpu_method __cpu_method_of_table_sentinel
	__used __section("__cpu_method_of_table_end");


static int __init set_smp_ops_by_method(struct device_node *node)
{
	const char *method;
	struct of_cpu_method *m = __cpu_method_of_table;

	if (of_property_read_string(node, "enable-method", &method))
		return 0;

	for (; m->method; m++)
		if (!strcmp(m->method, method)) {
			smp_set_ops(m->ops);
			return 1;
		}

	return 0;
}
#else
static inline int set_smp_ops_by_method(struct device_node *node)
{
	return 1;
}
#endif


/*
 * arm_dt_init_cpu_maps - Function retrieves cpu nodes from the device tree
 * and builds the cpu logical map array containing MPIDR values related to
 * logical cpus
 *
 * Updates the cpu possible mask with the number of parsed cpu nodes
 */
void __init arm_dt_init_cpu_maps(void)
{
	/*
	 * Temp logical map is initialized with UINT_MAX values that are
	 * considered invalid logical map entries since the logical map must
	 * contain a list of MPIDR[23:0] values where MPIDR[31:24] must
	 * read as 0.
	 */
	struct device_node *cpu, *cpus;
	int found_method = 0;
	u32 i, j, cpuidx = 1;
	u32 mpidr = is_smp() ? read_cpuid_mpidr() & MPIDR_HWID_BITMASK : 0;

	u32 tmp_map[NR_CPUS] = { [0 ... NR_CPUS-1] = MPIDR_INVALID };
	bool bootcpu_valid = false;
	cpus = of_find_node_by_path("/cpus");

	if (!cpus)
		return;

	for_each_of_cpu_node(cpu) {
		const __be32 *cell;
		int prop_bytes;
		u32 hwid;

		pr_debug(" * %pOF...\n", cpu);
		/*
		 * A device tree containing CPU nodes with missing "reg"
		 * properties is considered invalid to build the
		 * cpu_logical_map.
		 */
		cell = of_get_property(cpu, "reg", &prop_bytes);
		if (!cell || prop_bytes < sizeof(*cell)) {
			pr_debug(" * %pOF missing reg property\n", cpu);
			of_node_put(cpu);
			return;
		}

		/*
		 * Bits n:24 must be set to 0 in the DT since the reg property
		 * defines the MPIDR[23:0].
		 */
		do {
			hwid = be32_to_cpu(*cell++);
			prop_bytes -= sizeof(*cell);
		} while (!hwid && prop_bytes > 0);

		if (prop_bytes || (hwid & ~MPIDR_HWID_BITMASK)) {
			of_node_put(cpu);
			return;
		}

		/*
		 * Duplicate MPIDRs are a recipe for disaster.
		 * Scan all initialized entries and check for
		 * duplicates. If any is found just bail out.
		 * temp values were initialized to UINT_MAX
		 * to avoid matching valid MPIDR[23:0] values.
		 */
		for (j = 0; j < cpuidx; j++)
			if (WARN(tmp_map[j] == hwid,
				 "Duplicate /cpu reg properties in the DT\n")) {
				of_node_put(cpu);
				return;
			}

		/*
		 * Build a stashed array of MPIDR values. Numbering scheme
		 * requires that if detected the boot CPU must be assigned
		 * logical id 0. Other CPUs get sequential indexes starting
		 * from 1. If a CPU node with a reg property matching the
		 * boot CPU MPIDR is detected, this is recorded so that the
		 * logical map built from DT is validated and can be used
		 * to override the map created in smp_setup_processor_id().
		 */
		if (hwid == mpidr) {
			i = 0;
			bootcpu_valid = true;
		} else {
			i = cpuidx++;
		}

		if (WARN(cpuidx > nr_cpu_ids, "DT /cpu %u nodes greater than "
					       "max cores %u, capping them\n",
					       cpuidx, nr_cpu_ids)) {
			cpuidx = nr_cpu_ids;
			of_node_put(cpu);
			break;
		}

		tmp_map[i] = hwid;

		if (!found_method)
			found_method = set_smp_ops_by_method(cpu);
	}

	/*
	 * Fallback to an enable-method in the cpus node if nothing found in
	 * a cpu node.
	 */
	if (!found_method)
		set_smp_ops_by_method(cpus);

	if (!bootcpu_valid) {
		pr_warn("DT missing boot CPU MPIDR[23:0], fall back to default cpu_logical_map\n");
		return;
	}

	/*
	 * Since the boot CPU node contains proper data, and all nodes have
	 * a reg property, the DT CPU list can be considered valid and the
	 * logical map created in smp_setup_processor_id() can be overridden
	 */
	for (i = 0; i < cpuidx; i++) {
		set_cpu_possible(i, true);
		#if defined(CONFIG_LINUX_ON_SS_HYPERVISOR)
		cpu_logical_map(i) = signal_hyp(HYPERCALL_FID_VM_QUERY, HYPERCALL_SUBCMD_VM_QUERY_LINUX_PCPU, i, 0);
		pr_info("cpu logical map 0x%x\n", cpu_logical_map(i));
		#else
		cpu_logical_map(i) = tmp_map[i];
		pr_debug("cpu logical map 0x%x\n", cpu_logical_map(i));
		#endif
	}
}

bool arch_match_cpu_phys_id(int cpu, u64 phys_id)
{
	return phys_id == cpu_logical_map(cpu);
}

static const void * __init arch_get_next_mach(const char *const **match)
{
	static const struct machine_desc *mdesc = __arch_info_begin;
	const struct machine_desc *m = mdesc;

	if (m >= __arch_info_end)
		return NULL;

	mdesc++;
	*match = m->dt_compat;
	return m;
}

#ifdef CONFIG_FB_DTS_SKIP_ATAGS_TO_FDT
__initdata int gb_ATAG_CMDLINE_found = 0;
__initdata int gb_ATAG_INITRD2_found = 0;

int __init early_atags_to_data(void *atag_list)
{
	struct tag *atag = atag_list;
	/* In the case of 64 bits memory size, need to reserve 2 cells for
	 * address and size for each bank */

	/* make sure we've got an aligned pointer */
	if ((u32)atag_list & 0x3)
		return 1;


	/* validate the ATAG */
	if (atag->hdr.tag != ATAG_CORE ||
			(atag->hdr.size != tag_size(tag_core) &&
			 atag->hdr.size != 2))
		return 1;


	for_each_tag(atag, atag_list) {
		if (atag->hdr.tag == ATAG_CMDLINE) {
			strcpy(boot_command_line, atag->u.cmdline.cmdline);
			gb_ATAG_CMDLINE_found = 1;
		} else if (atag->hdr.tag == ATAG_INITRD2) {
			/* 1 if it is not an error if initrd_start < memory_start */
			extern int initrd_below_start_ok;
			/* free_initrd_mem always gets called with the next two as arguments.. */
			extern unsigned long initrd_start, initrd_end;

			initrd_start = (unsigned long)__va(atag->u.initrd.start);
			initrd_end = (unsigned long)__va(atag->u.initrd.start+atag->u.initrd.size);
			initrd_below_start_ok = 1;
			gb_ATAG_INITRD2_found = 1;
		}
	}

	return 0;
}
#endif

/**
 * setup_machine_fdt - Machine setup when an dtb was passed to the kernel
 * @dt_virt: virtual address of dt blob
 *
 * If a dtb was passed to the kernel in r2, then use it to choose the
 * correct machine_desc and to setup the system.
 */
const struct machine_desc * __init setup_machine_fdt(void *dt_virt)
{
	const struct machine_desc *mdesc, *mdesc_best = NULL;

#if defined(CONFIG_ARCH_MULTIPLATFORM) || defined(CONFIG_ARM_SINGLE_ARMV7M)
	DT_MACHINE_START(GENERIC_DT, "Generic DT based system")
		.l2c_aux_val = 0x0,
		.l2c_aux_mask = ~0x0,
	MACHINE_END

	mdesc_best = &__mach_desc_GENERIC_DT;
#endif

	if (!dt_virt || !early_init_dt_verify(dt_virt)) {
#ifdef CONFIG_SSTAR_BUILTIN_DTB
		if (early_init_dt_verify(builtin_dtb_start)) {
#ifdef CONFIG_FB_DTS_SKIP_ATAGS_TO_FDT
			if ((!dt_virt) || (!early_atags_to_data(dt_virt))) {
#ifndef CONFIG_DUALOS_SKIP_EARLY_PRINT
				early_print("early_atags_to_data() success\n");
#endif
			}

#else
			extern int early_atags_to_fdt(void *atag_list, void *fdt, int total_space);
			extern u32 builtin_dtb_size;
			if ((!dt_virt) || (!early_atags_to_fdt(dt_virt,builtin_dtb_start,builtin_dtb_size))) {
				early_print("early_atags_to_fdt() success\n");
			}
#endif
		} else
#endif
		{
		return NULL;
		}
	}

	mdesc = of_flat_dt_match_machine(mdesc_best, arch_get_next_mach);

	if (!mdesc) {
		const char *prop;
		int size;
		unsigned long dt_root;

		early_print("\nError: unrecognized/unsupported "
			    "device tree compatible list:\n[ ");

		dt_root = of_get_flat_dt_root();
		prop = of_get_flat_dt_prop(dt_root, "compatible", &size);
		while (size > 0) {
			early_print("'%s' ", prop);
			size -= strlen(prop) + 1;
			prop += strlen(prop) + 1;
		}
		early_print("]\n\n");

		dump_machine_table(); /* does not return */
	}

	/* We really don't want to do this, but sometimes firmware provides buggy data */
	if (mdesc->dt_fixup)
		mdesc->dt_fixup();

	early_init_dt_scan_nodes();

	/* Change machine number to match the mdesc we're using */
	__machine_arch_type = mdesc->nr;

	return mdesc;
}
