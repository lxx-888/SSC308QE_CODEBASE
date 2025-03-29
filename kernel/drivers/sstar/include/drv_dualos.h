/*
 * drv_dualos.h- Sigmastar
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

#ifndef __DRV_DUALOS_H__
#define __DRV_DUALOS_H__

#include "linux/cache.h"
#include "cam_os_wrapper.h"

#ifdef CONFIG_ARM_GIC_V3
#define CONFIG_RPMSG_USE_SGI_TO_REPLACE_SMC_ON_GICV3
#endif

#define ENABLE_NBLK_CALL 1

#define INTEROS_SC_L2R_RSQ_INIT          (0x100)
#define INTEROS_SC_L2R_HANDSHAKE         (0x800)
#define INTEROS_SC_L2R_RTK_CLI           (0x801)
#define INTEROS_SC_L2R_RTK_LOG           (0x802)
#define INTEROS_SC_L2R_SCR_FW_ENABLE     (0x803)
#define INTEROS_SC_L2R_SCR_FW_DISABLE    (0x804)
#define INTEROS_SC_L2R_RTK_ASSERT        (0x805)
#define INTEROS_SC_L2R_CALL              (0x808) // For AMP with GICv2
#define INTEROS_SC_L2R_AMP_RTK_CPU_USAGE (0x80B)
#define INTEROS_SC_L2R_SWTOE             (0xfe000000)
#define INTEROS_SC_L2R_RPMSG_HANDSHAKE   (0x900)
#define INTEROS_SC_L2R_RPMSG_NOTIFY      (0x901)
#define INTEROS_SC_L2R_SET_COREDUMP_LV   (0xA00)

#define INTEROS_INFO_SBOX_SIZE (1024)

#if defined(CONFIG_LINUX_ON_SS_HYPERVISOR)
#define IPI_NR_RTOS_2_LINUX_RPMSG 8
#define IPI_NR_LINUX_2_RTOS_RPMSG 15
#else
#define IPI_NR_RTOS_2_LINUX_RPMSG 13
#define IPI_NR_LINUX_2_RTOS_RPMSG 15
#endif

#define RTK_TIME_TO_US(x) (x / 6)

#if defined(CONFIG_LINUX_ON_SS_HYPERVISOR)
#define HYPERCALL_FID_SHOW_INFO                0x80000000UL
#define HYPERCALL_SUBCMD_SHOW_INFO_INT_STAT    0
#define HYPERCALL_SUBCMD_SHOW_INFO_VM_STAT     1
#define HYPERCALL_SUBCMD_SHOW_INFO_ESR_STAT    2
#define HYPERCALL_SUBCMD_SHOW_INFO_VGIC_STAT   3
#define HYPERCALL_SUBCMD_SHOW_INFO_CPU_STAT    4
#define HYPERCALL_FID_VM_STATUS                0x80000001UL
#define HYPERCALL_SUBCMD_VM_STATUS_SET_LOADED  0
#define HYPERCALL_SUBCMD_VM_STATUS_SET_READY   1
#define HYPERCALL_SUBCMD_VM_STATUS_SET_DIED    2
#define HYPERCALL_FID_VM_QUERY                 0x80000002UL
#define HYPERCALL_SUBCMD_VM_QUERY_RTOS_MASTER  0
#define HYPERCALL_SUBCMD_VM_QUERY_LINUX_MASTER 1
#define HYPERCALL_SUBCMD_VM_QUERY_LINUX_PCPU   2
#define HYPERCALL_SUBCMD_VM_QUERY_VM_NUM       3
#define HYPERCALL_SUBCMD_VM_QUERY_CPU_NUM      4
#define HYPERCALL_SUBCMD_VM_QUERY_VM_ID        5
#define HYPERCALL_SUBCMD_VM_QUERY_VCPU_ID      6
#define HYPERCALL_SUBCMD_VM_QUERY_TICK_BUF     7
#define HYPERCALL_FID_VM_REQUEST               0x80000003UL
#define HYPERCALL_SUBCMD_VM_REQUEST_FREE_INT   0

extern unsigned long signal_hyp(u32 type, u32 arg1, u32 arg2, u32 arg3);
extern unsigned long signal_hyp_vm_died(void);
#endif

struct rlink_head
{
    u64          next, prev;
    u64          nphys; // next object physical address
    unsigned int nsize; // next object size
    unsigned int reserved;
};

typedef struct
{
    struct rlink_head root;
    char              name[8];
    char              version[64];
    unsigned int      verid;
    unsigned int      size;
    unsigned int      fiq_cnt;
    unsigned int      ffiq_ts;
    unsigned int      ttff_isp;
    unsigned int      ttff_scl;
    unsigned int      ttff_mfe;
    unsigned int      ldns_ts;
    u64               start_ts;
    u64               lifet;
    u64               spent;
    u64               spent_hyp;
    u64               spent_sc;
    u64               diff;
    u64               syscall_cnt;
    unsigned int      main_code;             // RTOS execute on coreX in LH case.
    unsigned int      has_dead;              // RTOS has dead
    unsigned long     crash_log_buf_addr;    // RTOS crash log buffer address
    unsigned long     crash_log_buf_size;    // RTOS crash log buffer size
    unsigned long     core_dump_remote_addr; // RTOS core dump remote buffer address
    unsigned long     core_dump_remote_size; // RTOS core dump remote buffer size

    // sbox must be dcache-line aligned
    unsigned char sbox[INTEROS_INFO_SBOX_SIZE] __attribute__((aligned(1 << L1_CACHE_SHIFT)));
} rtkinfo_t;

extern rtkinfo_t* _rtk;
extern u64        epiod;

rtkinfo_t*    get_rtkinfo(void);
bool          dualos_rtos_exist(void);
unsigned long signal_rtos(u32 type, u32 arg1, u32 arg2, u32 arg3);
void          handle_interos_nblk_call_req(void);
void          handle_reroute_smc(void);

#endif //__DRV_DUALOS_H__