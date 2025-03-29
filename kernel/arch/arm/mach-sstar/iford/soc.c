/*
* soc.c- Sigmastar
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

#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/init.h>
#include <linux/irqchip.h>
#include <linux/of_platform.h>
#include <linux/of_fdt.h>
#include <linux/sys_soc.h>
#include <linux/slab.h>
#include <linux/suspend.h>
#include <linux/kthread.h>
#include <linux/delay.h>
#include <linux/mutex.h>
#include <linux/timecounter.h>
#include <clocksource/arm_arch_timer.h>
#include <linux/memblock.h>

#include <linux/gpio.h>

#include <asm/mach-types.h>
#include <asm/mach/arch.h>
#include <asm/memory.h>
#include <linux/memblock.h>
#include <asm/io.h>
#include <asm/mach/map.h>
#include "gpio.h"
#include "registers.h"
#include "ms_platform.h"
#include "ms_types.h"
#include "ss_private.h"
#include "ms_msys.h"
#include "memory.h"
#include <linux/of.h>
#include <linux/of_irq.h>

/* IO tables */
static struct map_desc sstar_io_desc[] __initdata = {
    /* Define Registers' physcial and virtual addresses */
    { IO_VIRT, __phys_to_pfn(IO_PHYS), IO_SIZE, MT_DEVICE },
};

static const char * const sstar_dt_compat[] __initconst = {
    "sstar,iford",
    NULL,
};

static void __init sstar_map_io(void)
{
    iotable_init(sstar_io_desc, ARRAY_SIZE(sstar_io_desc));
}

extern struct ss_chip* sstar_chip_get(void);
extern void __init sstar_chip_init_default(void);
//extern void __init mstar_init_irqchip(void);


//extern struct timecounter *arch_timer_get_timecounter(void);


/*************************************
*        Sstar chip flush function
*************************************/

static void sstar_uart_disable_line(int line)
{
    //for debug, do not change
    if (line == 0) {
        // UART0_Mode -> X
        //CLRREG16(BASE_REG_CHIPTOP_PA + REG_ID_03, BIT4 | BIT5);
        //CLRREG16(BASE_REG_PMSLEEP_PA + REG_ID_09, BIT11);
    } else if (line == 1) {
        // UART1_Mode -> X
        CLRREG16(BASE_REG_CHIPTOP_PA + REG_ID_03, BIT8 | BIT9);
    } else if (line == 2) {
        // FUART_Mode -> X
        CLRREG16(BASE_REG_CHIPTOP_PA + REG_ID_03, BIT0 | BIT1);
    }
}

static void sstar_uart_enable_line(int line)
{
    //for debug, do not change
    if (line == 0) {
        // UART0_Mode -> PAD_UART0_TX/RX
        //SETREG16(BASE_REG_CHIPTOP_PA + REG_ID_03, BIT4);
    } else if (line == 1) {
        // UART1_Mode -> PAD_UART1_TX/RX
        SETREG16(BASE_REG_CHIPTOP_PA + REG_ID_03, BIT8);
    } else if (line == 2) {
        // FUART_Mode -> PAD_FUART_TX/RX
        SETREG16(BASE_REG_CHIPTOP_PA + REG_ID_03, BIT0);
    }
}

static int sstar_get_device_id(void)
{
    return (int)(INREG16(BASE_REG_PMTOP_PA) & 0x00FF);;
}

static int sstar_get_revision(void)
{
    u16 tmp = 0;
    tmp = INREG16((unsigned int)(BASE_REG_PMTOP_PA + REG_ID_67));
    tmp=((tmp >> 8) & 0x00FF);

    return (tmp+1);
}

static void sstar_chip_flush_miu_pipe(void)
{
    // I6C don't need to flush the miu pipe
}

static void sstar_chip_flush_STB_and_miu_pipe(void)
{
    dsb();
    sstar_chip_flush_miu_pipe();
}

static u64 sstar_phys_to_MIU(u64 x)
{
    return ((x) - MIU0_BASE);
}

static u64 sstar_MIU_to_phys(u64 x)
{
    return ((x) + MIU0_BASE);
}


struct soc_device_attribute sstar_soc_dev_attr;

extern const struct of_device_id of_default_bus_match_table[];

static int sstar_get_storage_type(void)
{
//check DIDKEY bank, offset 0x70
#define STORAGE_SPI_NONE                  0x00
#define STORAGE_SPI_NAND_SKIP_SD          BIT2
#define STORAGE_SPI_NAND                  BIT4
#define STORAGE_SPI_NOR                   BIT5
#define STORAGE_SPI_NOR_SKIP_SD           BIT1
#define STORAGE_USB                       BIT12
#define STORAGE_EMMC                      BIT3  //Boot from eMMC
#define STORAGE_EMMC_4                    STORAGE_EMMC
#define STORAGE_EMMC_8                    STORAGE_EMMC|BIT7  //emmc_8bit_mode (0: 4-bit mode, 1: 8-bit mode)
#define STORAGE_EMMC_4_SD1                STORAGE_EMMC|BIT10 //emmc 4-bit source (0: SD0, 1: SD1)
#define STORAGE_BOOT_TYPES                (BIT12|BIT10|BIT7|BIT5|BIT4|BIT3|BIT2|BIT1)

    u16 boot_type = (INREG16(BASE_REG_DIDKEY_PA + 0x70*4) & STORAGE_BOOT_TYPES);

    if (boot_type == STORAGE_SPI_NAND || boot_type == STORAGE_SPI_NAND_SKIP_SD) {
        return (int)MS_STORAGE_SPINAND_ECC;
    } else if ((boot_type & STORAGE_EMMC) == STORAGE_EMMC) {
        return (int)MS_STORAGE_EMMC;
    } else if (boot_type == STORAGE_SPI_NOR || boot_type == STORAGE_SPI_NOR_SKIP_SD) {
        return (int)MS_STORAGE_NOR;
    } else {
        return (int)MS_STORAGE_UNKNOWN;
    }
}

static int sstar_get_package_type(void)
{
    printk(KERN_ERR "!!!!! Machine name [%s] \n", sstar_soc_dev_attr.machine);
    return (INREG16(REG_MAILBOX_PARTNAME) & PARTNAME_PACKAGE_MASK) >> PARTNAME_PACKAGE_SHIFT;
}
static char sstar_platform_name[]=CONFIG_SSTAR_SHORT_NAME;

char* sstar_get_platform_name(void)
{
    return sstar_platform_name;
}

static unsigned long long sstar_chip_get_riu_phys(void)
{
    return IO_PHYS;
}

static int sstar_chip_get_riu_size(void)
{
    return IO_SIZE;
}


static int sstar_ir_enable(int param)
{
    printk(KERN_ERR "NOT YET IMPLEMENTED!![%s]",__FUNCTION__);
    return 0;
}


static int sstar_usb_vbus_control(int param)
{

    int ret;
    //int package = mstar_get_package_type();
    int power_en_gpio=-1;

    struct device_node *np;
    int pin_data;
    int port_num = param >> 16;
    int vbus_enable = param & 0xFF;
    if ((vbus_enable<0 || vbus_enable>1) && (port_num<0 || port_num>1)) {
        printk(KERN_ERR "[%s] param invalid:%d %d\n", __FUNCTION__, port_num, vbus_enable);
        return -EINVAL;
    }

    if (power_en_gpio<0) {
        if (0 == port_num) {
            np = of_find_node_by_path("/soc/Sstar-ehci-1");
        } else {
            np = of_find_node_by_path("/soc/Sstar-ehci-2");
        }

        if (!of_property_read_u32(np, "power-enable-pad", &pin_data)) {
            printk(KERN_ERR "Get power-enable-pad from DTS GPIO(%d)\n", pin_data);
            power_en_gpio = (unsigned char)pin_data;
        } else {
            printk(KERN_ERR "Can't get power-enable-pad from DTS, set default GPIO(%d)\n", pin_data);
            power_en_gpio = PAD_PM_GPIO2;
        }
        ret = gpio_request(power_en_gpio, "USB0-power-enable");
        if (ret < 0) {
            printk(KERN_INFO "Failed to request USB0-power-enable GPIO(%d)\n", power_en_gpio);
            power_en_gpio =-1;
            return ret;
        }
    }
    //disable vbus
    if (0 == vbus_enable) {
        gpio_direction_output(power_en_gpio, 0);
        printk(KERN_INFO "[%s] Disable USB VBUS GPIO(%d)\n", __FUNCTION__,power_en_gpio);
    } else if(1 == vbus_enable) {
        gpio_direction_output(power_en_gpio, 1);
        printk(KERN_INFO "[%s] Enable USB VBUS GPIO(%d)\n", __FUNCTION__,power_en_gpio);
    }
    return 0;
}
static u64 us_ticks_cycle_offset=0;
static u64 us_ticks_factor=1;


static u64 sstar_chip_get_us_ticks(void)
{
	u64 cycles=arch_timer_read_counter();
	u64 usticks=div64_u64(cycles,us_ticks_factor);
	return usticks;
}

void sstar_reset_us_ticks_cycle_offset(void)
{
	us_ticks_cycle_offset=arch_timer_read_counter();
}

static int sstar_chip_function_set(int function_id, int param)
{
    int res=-1;

    printk("CHIP_FUNCTION SET. ID=%d, param=%d\n",function_id,param);
    switch (function_id) {
            case CHIP_FUNC_UART_ENABLE_LINE:
                sstar_uart_enable_line(param);
                break;
            case CHIP_FUNC_UART_DISABLE_LINE:
                sstar_uart_disable_line(param);
                break;
            case CHIP_FUNC_IR_ENABLE:
                sstar_ir_enable(param);
                break;
            case CHIP_FUNC_USB_VBUS_CONTROL:
                sstar_usb_vbus_control(param);
                break;
        default:
            printk(KERN_ERR "Unsupport CHIP_FUNCTION!! ID=%d\n",function_id);

    }

    return res;
}


static void __init sstar_init_early(void)
{
    struct ss_chip *chip=NULL;
    sstar_chip_init_default();
    chip=sstar_chip_get();
    chip->chip_flush_miu_pipe=sstar_chip_flush_STB_and_miu_pipe;//dsb
    chip->chip_flush_miu_pipe_nodsb=sstar_chip_flush_miu_pipe;//nodsbchip->phys_to_miu=mstar_phys_to_MIU;
    chip->phys_to_miu=sstar_phys_to_MIU;
    chip->miu_to_phys=sstar_MIU_to_phys;
    chip->chip_get_device_id=sstar_get_device_id;
    chip->chip_get_revision=sstar_get_revision;
    chip->chip_get_platform_name=sstar_get_platform_name;
    chip->chip_get_riu_phys=sstar_chip_get_riu_phys;
    chip->chip_get_riu_size=sstar_chip_get_riu_size;

    chip->chip_function_set=sstar_chip_function_set;
    chip->chip_get_storage_type=sstar_get_storage_type;
    chip->chip_get_package_type=sstar_get_package_type;
    chip->chip_get_us_ticks=sstar_chip_get_us_ticks;
}

#if defined(CONFIG_MIU0_DMA_PFN_OFFSET)
static int sstar_platform_notifier(struct notifier_block *nb,
                      unsigned long event, void *data)
{
    struct device *dev = data;

    if (event != BUS_NOTIFY_ADD_DEVICE)
        return NOTIFY_DONE;

    if (!dev)
        return NOTIFY_BAD;

    if (!dev->of_node) {
        int ret = dma_direct_set_offset(dev, SOC_HIGH_PHYS_START,
                          SOC_LOW_PHYS_START,
                          SOC_HIGH_PHYS_SIZE);
        dev_err(dev, "set dma_offset%08llx%s\n",
              SOC_HIGH_PHYS_START - SOC_LOW_PHYS_START,
              ret ? " failed" : "");
    }
    return NOTIFY_OK;
}

static struct notifier_block platform_nb = {
    .notifier_call = sstar_platform_notifier,
};
#endif

extern char* LX_VERSION;
struct device *parent = NULL;

static void __init sstar_init_machine(void)
{
    struct soc_device *soc_dev;

    pr_info("\n\nVersion : %s\n\n",LX_VERSION);

    sstar_reset_us_ticks_cycle_offset();
    us_ticks_factor=div64_u64(arch_timer_get_rate(),1000000);

    sstar_soc_dev_attr.family = kasprintf(GFP_KERNEL, sstar_platform_name);
    sstar_soc_dev_attr.revision = kasprintf(GFP_KERNEL, "%d", sstar_get_revision());
    sstar_soc_dev_attr.soc_id = kasprintf(GFP_KERNEL, "%u", sstar_get_device_id());
    sstar_soc_dev_attr.api_version = kasprintf(GFP_KERNEL, sstar_chip_get()->chip_get_API_version());
    sstar_soc_dev_attr.machine = kasprintf(GFP_KERNEL, of_flat_dt_get_machine_name());

    soc_dev = soc_device_register(&sstar_soc_dev_attr);
    if (IS_ERR(soc_dev)) {
        kfree((void *)sstar_soc_dev_attr.family);
        kfree((void *)sstar_soc_dev_attr.revision);
        kfree((void *)sstar_soc_dev_attr.soc_id);
        kfree((void *)sstar_soc_dev_attr.machine);
        goto out;
    }

    parent = soc_device_to_device(soc_dev);

    /*
     * Finished with the static registrations now; fill in the missing
     * devices
     */
out:
    //write log_buf address to mailbox
    OUTREG16(BASE_REG_MAILBOX_PA+BK_REG(0x08), (int)log_buf_addr_get() & 0xFFFF);
    OUTREG16(BASE_REG_MAILBOX_PA+BK_REG(0x09), ((int)log_buf_addr_get() >> 16 )& 0xFFFF);

#if defined(CONFIG_MIU0_DMA_PFN_OFFSET)
    if (PHYS_OFFSET >= SOC_HIGH_PHYS_START) {
        soc_dma_pfn_offset = PFN_DOWN(SOC_HIGH_PHYS_START -
                                      SOC_LOW_PHYS_START);
        bus_register_notifier(&platform_bus_type, &platform_nb);
    }
#endif
}

extern int sstar_pm_init(void);
extern void init_proc_zen(void);
static inline void __init sstar_init_late(void)
{
#ifdef CONFIG_PM_SLEEP
    sstar_pm_init();
#endif
}

static void global_reset(enum reboot_mode mode, const char * cmd)
{
    msys_set_rebootType(cmd);

    while (1) {
        OUTREG8(0x1f001cb8, 0x79);
        mdelay(5);
    }
}

static void __init sstar_pm_reserve(void)
{
    /*
    0x2000_0000-0x2000_0400 (1K)
        ATAG: IPL_CUST pass atag to Linux in DaulOS-LH
        ATAG: VMM pass atag to Linux in DaulOS-HYP
        STR:  Linux told IPL suspend_info in suspend
    0x2000_0400-0x2000_2000
        smf: Must reserved until the completion of linux boot
    0x2000_2000-0x2000_3000
        IPL boot timestamp record
    */
    memblock_reserve(MIU0_BASE, 0x3000);
}

static long long __init sstar_pv_fixup(void)
{
    long long offset;
    phys_addr_t mem_start, mem_end;

    mem_start = memblock_start_of_DRAM();
    mem_end = memblock_end_of_DRAM();

    /* nothing to do if we are running out of the <32-bit space */
    if (mem_start >= SOC_LOW_PHYS_START && mem_end   <= SOC_LOW_PHYS_END)
        return 0;

    if (mem_start < SOC_HIGH_PHYS_START || mem_end   > SOC_HIGH_PHYS_END) {
        pr_crit("Invalid address space for memory (%08llx-%08llx)\n",
                (u64)mem_start, (u64)mem_end);
        return 0;
    }

    offset = SOC_HIGH_PHYS_START - SOC_LOW_PHYS_START;

    /* Populate the arch idmap hook */
    arch_phys_to_idmap_offset = -offset;

    return offset;
}

#ifdef CONFIG_SMP
extern struct smp_operations iford_smp_ops;
#endif

DT_MACHINE_START(SS_DT, "SStar Soc (Flattened Device Tree)")
    .dt_compat    = sstar_dt_compat,
    .map_io = sstar_map_io,
    .init_machine = sstar_init_machine,
    .init_early = sstar_init_early,
//    .init_time =  ms_init_timer,
//    .init_irq = mstar_init_irqchip,
    .init_late = sstar_init_late,
    .restart = global_reset,
    .reserve = sstar_pm_reserve,
    .pv_fixup = sstar_pv_fixup,
    #ifdef CONFIG_SMP
    .smp    = smp_ops(iford_smp_ops),
    #endif
MACHINE_END
