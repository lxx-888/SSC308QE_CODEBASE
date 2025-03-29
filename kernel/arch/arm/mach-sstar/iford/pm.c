/*
* pm.c- Sigmastar
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
#include <linux/suspend.h>
#include <linux/io.h>
#include <asm/suspend.h>
#include <asm/fncpy.h>
#include <asm/cacheflush.h>
#include "ms_platform.h"
#include <linux/delay.h>
#include <asm/secure_cntvoff.h>

#include "voltage_ctrl.h"
#include "registers.h"
#include "vad_STR.h"
#include "mailbox_str.h"

extern void cpu_suspend_abort(void);
#ifdef CONFIG_PM_MCU_POWER_OFF
extern void __init sstar_pm_mcu_init(void);
#endif

#define FIN     printk(KERN_ERR"[%s]+++\n",__FUNCTION__)
#define FOUT    printk(KERN_ERR"[%s]---\n",__FUNCTION__)
#define HERE    printk(KERN_ERR"%s: %d\n",__FILE__,__LINE__)
#define SUSPEND_WAKEUP 0
#define SUSPEND_SLEEP  1
#define STR_PASSWORD   0x5A5A55AA

#define SUSPEND_CODE_SIZE 0x1000
#define RESUME_CODE_SIZE 0x1000
#define STACK_SIZE 0x3000
#define LIB_SIZE 0x19000 //lib size+Working buffer
#define RAM_SIZE 0x3000 //ram section3KB
#define FRAME_SIZE 0x3000
#define INFO_SIZE 0x1000

#define SUSPEND_START 0xA0000000
#define RESUME_START (SUSPEND_START + SUSPEND_CODE_SIZE)
#define STACK_START (RESUME_START + RESUME_CODE_SIZE)
#define LIB_START (STACK_START + STACK_SIZE)

#define TIME_START (0xA0028400)

#define RAM_START (TIME_START + INFO_SIZE)
#define FRAME_START (RAM_START + RAM_SIZE)
#define INFO_START (FRAME_START + FRAME_SIZE)

//#define STACK_GAP 0x100

extern void vad_main(void);

typedef struct {
    char magic[8];
    unsigned long long resume_entry;
    unsigned int count;
    unsigned int status;
    unsigned int password;
    unsigned int dram_crc;
    unsigned long long dram_crc_start;
    unsigned long long dram_crc_size;
    unsigned int dram_size;
} suspend_keep_info;

static uint64_t dram_crc_end;
static uint64_t dram_crc_start;
static int __init suspend_crc_setup(char *param)
{
    if ((NULL == param) || ('\0' == *param))
    {
        return 0;
    }

    sscanf(param, "%llx,%llx", &dram_crc_start, &dram_crc_end);

    return 0;
}
early_param("suspend_crc", suspend_crc_setup);

//unsigned int resume_pbase = 0;
extern void sram_suspend_imi(void) __attribute__((aligned(16)));
//void (*mstar_resume_imi_fn)(void);
//void(sram_resume_imi)(void) __attribute__((aligned(8)));
void __iomem *suspend_imi_vbase;
//void __iomem *resume_imi_vbase;
//void __iomem *resume_imistack_vbase;
//void __iomem *lib_vbase;
//void __iomem *ram_vbase;
//void __iomem *info_vbase;
//void __iomem *frame_vbase;
//void __iomem *timestemp_vbase;

static void (*mstar_suspend_imi_fn)(void);
suspend_keep_info *pStr_info;
int suspend_status = SUSPEND_WAKEUP;

//static u32 CurTask_SP = 0;
//static u32 Reglr;

//void sram_resume_imi(void)
//{
//    *(unsigned short volatile *)0xFD200800 = WAKEUP_SRAM_7;
//
//    asm volatile("mov lr, %0" : : "r"(Reglr));
//    asm volatile("mov sp, %0" : : "r"(CurTask_SP));
//    asm volatile("ldmia.w sp!, {r4-r8, lr}");
//
//    asm volatile("ldmia   sp!, {r1 - r3}"); //@ pop phys pgd, virt SP, phys resume fn
//    asm volatile("teq r0, #0");
//    asm volatile("moveq   r0, #1"); //e force non-zero value
//    asm volatile("mov sp, r2");
//    asm volatile("ldmfd   sp!, {r4 - r11, pc}");
//}


//#ifdef CONFIG_MS_AIO
//extern signed int MHAL_AUDIO_AI_DmaImiEnable(bool bEnable);
//extern signed int MHAL_AUDIO_AI_DmaImiInit(unsigned long long nBufAddrOffset, unsigned long nBufSize);
//#endif

#define REG_ADDR_BASE_IMI_TOP       GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x112100 )
#define REG_IMI_SET_IDLE            GET_REG_ADDR(REG_ADDR_BASE_IMI_TOP, 0X37)
    #define IMI_SET_IDLE  (2)
    #define IMI_HANG_UP   (4)
#define REG_IMI_AXI_STATUS          GET_REG_ADDR(REG_ADDR_BASE_IMI_TOP, 0X38)
    #define IMI_AXI_STATUS_W_COMPLETE      BIT0
    #define IMI_AXI_STATUS_R_COMPLETE      BIT1
    #define IMI_AXI_STATUS_BUSY            BIT2
#define REG_CHIPTOP_DUMMY_2         GET_REG_ADDR(BASE_REG_CHIPTOP_PA, 0X22)
    //BIT0
    #define REG_TOP_VEN_SELECT_IMI  (0)
    #define REG_TOP_VEN_SELECT_VEN  (1)
#define REG_MIU_DRAM_SIZE           GET_REG_ADDR(BASE_REG_MIU_MMU, 0X0D)

int sstar_suspend_ready(unsigned long ret)
{
    // Enable IMI, exit idle
    CLRREG16(REG_IMI_SET_IDLE, IMI_SET_IDLE&IMI_SET_IDLE);
    // Select IMI control to TOP from VEN
    SETREG16(REG_CHIPTOP_DUMMY_2, BIT0);

    mstar_suspend_imi_fn = fncpy(suspend_imi_vbase, (void *)&sram_suspend_imi, SUSPEND_CODE_SIZE);
    printk("suspend_imi_fn=%08x suspend_imi_vbase=%08x sram_suspend_imi=%08x\n",
        (int)mstar_suspend_imi_fn, (int)suspend_imi_vbase, (int)sram_suspend_imi);

    // Set resume flag(warm reset)
    SETREG16(BASE_REG_PMPOR_PA + REG_ID_01, BIT3);

    suspend_status = SUSPEND_SLEEP;
    // Deal resume info
    if (pStr_info) {
        pStr_info->count++;
        pStr_info->status = SUSPEND_SLEEP;
        pStr_info->password = STR_PASSWORD;

        if ((dram_crc_start >= MIU0_BASE) && (dram_crc_end > dram_crc_start))
        {
            pStr_info->dram_crc_start = dram_crc_start;
            pStr_info->dram_crc_size = dram_crc_end - dram_crc_start;
        }
        else
        {
            pStr_info->dram_crc_size = 0;
        }

        pStr_info->dram_size = INREG16(REG_MIU_DRAM_SIZE);
    }

    // Flush cache to ensure memory is updated before self-refresh
    __cpuc_flush_kern_all();
    // Flush L3 cache
    Chip_Flush_MIU_Pipe();

    mstar_suspend_imi_fn();
    return 0;
}

#ifdef CONFIG_SS_PROFILING_TIME
#if defined(CONFIG_PM) || defined(CONFIG_SUSPEND)
extern int g_pm_stage_profiler_enabled;
extern void recode_timestamp(int mark, const char* name);
extern int recode_show(char *buf);
void sstar_printk_buf_by_line(const char *buf, size_t buf_size)
{
    const char *start = buf;
    const char *end = buf + buf_size;

    while (start < end) {
        const char *newline = strchr(start, '\n');
        if (newline == NULL) {
            printk(KERN_INFO "%.*s", (int)(end - start), start);
            break;
        } else {
            printk(KERN_INFO "%.*s", (int)(newline - start), start);
            start = newline + 1;
        }
    }
}
#endif
#endif

static int sstar_suspend_enter(suspend_state_t state)
{
    //FIN;
    switch (state)
    {
        case PM_SUSPEND_MEM:
            printk(KERN_INFO "state = PM_SUSPEND_MEM\n");
        #ifdef CONFIG_SS_PROFILING_TIME
        #if defined(CONFIG_PM) || defined(CONFIG_SUSPEND)
            if (g_pm_stage_profiler_enabled == 1) {
                int len = 0;
                char *buf;
                buf = kmalloc(PAGE_SIZE, GFP_KERNEL);
                if (buf == NULL)
                    return -EINVAL;
                memset(buf, 0, PAGE_SIZE);
                recode_timestamp(__LINE__, "PM_SUSPEND_MEM");
                len = recode_show(buf);
                BUG_ON(len > PAGE_SIZE);
                sstar_printk_buf_by_line(buf, len);
                kfree(buf);
            }
        #endif
        #endif

            cpu_suspend(0, sstar_suspend_ready);
            break;
        default:
            return -EINVAL;
    }

    return 0;
}

static void sstar_suspend_wake(void)
{
    if (pStr_info) {
        pStr_info->status = SUSPEND_WAKEUP;
        pStr_info->password = 0;
        OUTREG16(REG_MIU_DRAM_SIZE, pStr_info->dram_size);
    }
}

struct platform_suspend_ops mstar_suspend_ops = {
    .enter = sstar_suspend_enter,
    .wake = sstar_suspend_wake,
    .valid = suspend_valid_only_mem,
};

int __init sstar_pm_init(void)
{
    suspend_imi_vbase = __arm_ioremap_exec(0xA0000000, 0x1000, false);

    printk(KERN_ERR "[%s] MIU0_BASE=%llX\n", __func__, (unsigned long long)MIU0_BASE);
    pStr_info = (suspend_keep_info *)__va(MIU0_BASE); //it's reserved that hook .reserve for init.c
    memset(pStr_info, 0, sizeof(suspend_keep_info));
    strcpy(pStr_info->magic, "SIG_STR");
    pStr_info->resume_entry = phys_to_idmap(virt_to_phys(cpu_resume_arm));


    printk(KERN_INFO "[%s] resume_entry=%#llx, suspend_imi_vbase=%#lX\n", __func__, pStr_info->resume_entry, (unsigned long)suspend_imi_vbase);

    suspend_set_ops(&mstar_suspend_ops);

#ifdef CONFIG_PM_MCU_POWER_OFF
    sstar_pm_mcu_init();
#endif
    return 0;
}

