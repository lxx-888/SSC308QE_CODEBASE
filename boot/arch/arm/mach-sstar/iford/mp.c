/*
 * mp.c - Sigmastar
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

#include <common.h>
#include <cpu_func.h>
#include <asm/io.h>
#include <linux/errno.h>
#include <asm/mach-types.h>
#include <asm/global_data.h>
#include <asm/arch/mach/platform.h>
#include <asm/arch/mach/io.h>

int cpu_reset(u32 nr)
{
    /* Software reset of the CPU N */
    printf("%s the CPU %d\n", __FUNCTION__, nr);

    //	src->scr |= cpu_reset_mask[nr];
    return 0;
}

int cpu_status(u32 nr)
{
    // printf("core %d => %d\n", nr, !!(src->scr & cpu_ctrl_mask[nr]));
    printf("%s the CPU %d\n", __FUNCTION__, nr);
    return 0;
}

int cpu_release(u32 nr, int argc, char *const argv[])
{
    u32 boot_addr;
    u16 wakeup_tag = CORE_WAKEUP_FLAG(nr);
    u16 mask       = CORE_WAKEUP_FLAG(nr) | CORE_NS_FLAG(nr);

    if (argc != 2)
    {
        printf("Argc is %d, need boot address and world [s/ns]\n", argc);
        return 1;
    }

    boot_addr = hextoul(argv[0], NULL);
    if (0 == strncmp(argv[1], "s", 1))
    {
    }
    else if (0 == strncmp(argv[1], "ns", 2))
    {
        wakeup_tag |= CORE_NS_FLAG(nr);
    }
    else
    {
        return 1;
    }

    printf("%s: wake up core%d at 0x%08X in %s-world\n", __FUNCTION__, nr, boot_addr, argv[1]);

    OUTREG16(SECOND_START_ADDR_LO, ((u16)((u32)boot_addr & 0xFFFF)));
    OUTREG16(SECOND_START_ADDR_HI, ((u16)((u32)boot_addr >> 16)));

    do
    {
        SETREG16_BIT_OP(CORE_WAKEUP_CONFIG_ADDR, wakeup_tag);
    } while ((INREG16(CORE_WAKEUP_CONFIG_ADDR) & mask) != wakeup_tag);

    __asm__ __volatile__(
        "dsb\n"
        "sev\n"
        "nop");

    return 0;
}

int is_core_valid(unsigned int core)
{
    uint32_t nr_cores = MAX_CPUS; // get_nr_cpus();

    if (core > nr_cores)
        return 0;

    return 1;
}

int cpu_disable(u32 nr)
{
    /* Disable the CPU N */
    printf("%s the CPU %d\n", __FUNCTION__, nr);

    // src->scr &= ~cpu_ctrl_mask[nr];
    return 0;
}
