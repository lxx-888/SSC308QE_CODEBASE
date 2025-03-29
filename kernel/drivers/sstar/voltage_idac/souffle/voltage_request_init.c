/*
 * voltage_request_init.c- Sigmastar
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
#include <linux/of.h>
#if defined(CONFIG_SSTAR_VOLTAGE_IDAC_STEP_BY_STEP)
#include <linux/delay.h>
#endif
#include "registers.h"
#include "ms_platform.h"
#include "voltage_ctrl.h"
#include "voltage_ctrl_demander.h"
#include "gpio.h"
#include <voltage_request_init.h>

// The register
#define REG_EN_IDAC_CORE       (BASE_REG_PMSAR_PA + REG_ID_73)
#define REG_IDAC_CURR_CORE_ADJ (BASE_REG_PMSAR_PA + REG_ID_71)
#define REG_EN_IDAC_CPU        (BASE_REG_PMSLEEP_PA + REG_ID_74)
#define REG_IDAC_CURR_CPU_ADJ  (BASE_REG_PMSAR1_PA + REG_ID_11)
#define REG_EN_IDAC_IPU        (BASE_REG_PMSLEEP_PA + REG_ID_73)
#define REG_IDAC_CURR_IPU_ADJ  (BASE_REG_PMSAR_PA + REG_ID_61)

#define IDAC_DBG 0

#if defined(CONFIG_SSTAR_VOLTAGE_IDAC_STEP_BY_STEP)
static u32 cpu_curr_vol = 0;
static u32 ipu_curr_vol = 0;
#endif

//####### IDAC VOLTAGE DEFINE FORMAT #########//
// BIT5:   0.5uA/1uA  (1/0)
// BIT4:   Plus/Minus (1/0)
// BIT3-0: Step	(0-F)
// R: 20k ohm

static u16 idac_get_control_setting(int voloff)
{
    u8 unit   = 0;
    u8 level  = 0;
    u8 source = (voloff < 0) ? 0 : 1;

    if ((abs(voloff) / 10) > 0xF)
    {
        if (voloff < 0)
        {
            voloff = voloff - 19;
        }
        level = abs(voloff) / 20;
    }
    else
    {
        if (voloff < 0)
        {
            voloff = voloff - 9;
        }
        unit  = 1; // 0.5uA
        level = abs(voloff) / 10;
    }

    if (level > 0xF)
    {
        printk(KERN_ERR "[%s] the level [%d] is invalid. reset level as 0\n", __FUNCTION__, level);
        level = 0;
    }

    return level | (unit << 5) | (source << 4);
}

static int idac_get_voloff(u16 idac)
{
    u8  unit   = (idac & (1 << 5)) ? 10 : 20;
    u8  level  = idac & 0xF;
    u8  source = idac & (1 << 4);
    int voloff = 0;

    voloff = unit * level;
    if (!source)
    {
        voloff = -voloff;
    }

    return voloff;
}

#if defined(CONFIG_SSTAR_VOLTAGE_IDAC_STEP_BY_STEP)
static u16 idac_set_voltage_step_by_step(int base_vol, int set_vol, int cur_vol, u32 reg)
{
    U16 tmpval;
    U16 idac_setvol = idac_get_control_setting(set_vol - base_vol);
    U16 idac_curvol = idac_get_control_setting(cur_vol - base_vol);

#if IDAC_DBG
    printk("%s %d: base_vol=%d, setvol=%d, curr_vol=%d\r\n", __FUNCTION__, __LINE__, base_vol, set_vol, cur_vol);
#endif

    while (idac_setvol != idac_curvol)
    {
        if (cur_vol > set_vol)
        {
            cur_vol -= 10;
        }
        else
        {
            cur_vol += 10;
        }
        idac_curvol = idac_get_control_setting(cur_vol - base_vol);
        tmpval      = (INREG16(reg) & 0xFFC0) | idac_curvol;
        OUTREG16(reg, tmpval);
        ndelay(100);
#if IDAC_DBG
        printk("%s %d: idac_setvol=%x, idac_curvol=%x kcode=%x\r\n", __FUNCTION__, __LINE__, idac_setvol, idac_curvol,
               tmpval);
#endif
    }
#if IDAC_DBG
    printk("%s %d: current vol=%d\r\n", __FUNCTION__, __LINE__, cur_vol);
#endif
    return 0;
}
#endif

int idac_set_voltage(const char *name, u32 base_vol, u32 setvol)
{
    int voloff = setvol - base_vol;
    u16 idac_s = idac_get_control_setting(voloff);

#if IDAC_DBG
    printk("%s %d: name=%s, base_vol=%d, setvol=%d\r\n", __FUNCTION__, __LINE__, name, base_vol, setvol);
#endif
    if (!strcmp(name, "core_power"))
    {
        idac_s = (INREG16(REG_IDAC_CURR_CORE_ADJ) & 0xFFC0) | idac_s;
        OUTREG16(REG_IDAC_CURR_CORE_ADJ, idac_s);
    }
    else if (!strcmp(name, "cpu_power"))
    {
#if defined(CONFIG_SSTAR_VOLTAGE_IDAC_STEP_BY_STEP)
        if (cpu_curr_vol == 0)
        {
            cpu_curr_vol = base_vol;
        }
        idac_set_voltage_step_by_step(base_vol, setvol, cpu_curr_vol, REG_IDAC_CURR_CPU_ADJ);
        cpu_curr_vol = setvol;
#else
        idac_s = (INREG16(REG_IDAC_CURR_CPU_ADJ) & 0xFFC0) | idac_s;
        OUTREG16(REG_IDAC_CURR_CPU_ADJ, idac_s);
#endif
    }
    else if (!strcmp(name, "ipu_power"))
    {
#if defined(CONFIG_SSTAR_VOLTAGE_IDAC_STEP_BY_STEP)
        if (ipu_curr_vol == 0)
        {
            ipu_curr_vol = base_vol;
        }
        idac_set_voltage_step_by_step(base_vol, setvol, ipu_curr_vol, REG_IDAC_CURR_IPU_ADJ);
        ipu_curr_vol = setvol;
#else
        idac_s = (INREG16(REG_IDAC_CURR_IPU_ADJ) & 0xFFC0) | idac_s;
        OUTREG16(REG_IDAC_CURR_IPU_ADJ, idac_s);
#endif
    }
    else
    {
        printk(KERN_ERR "[%s] Not Support!!\n", name);
        return -1;
    }
#if IDAC_DBG
    printk("%s %d: name=%s finish\r\n", __FUNCTION__, __LINE__, name);
#endif
    return 0;
}

int idac_get_voltage(const char *name, u32 base_vol, u32 *curvol)
{
    u16 idac_s;

    if (!strcmp(name, "core_power"))
    {
        idac_s = INREG16(REG_IDAC_CURR_CORE_ADJ) & 0x3F;
    }
    else if (!strcmp(name, "cpu_power"))
    {
        idac_s = INREG16(REG_IDAC_CURR_CPU_ADJ) & 0x3F;
    }
    else if (!strcmp(name, "ipu_power"))
    {
        idac_s = INREG16(REG_IDAC_CURR_IPU_ADJ) & 0x3F;
    }
    else
    {
        printk(KERN_ERR "[%s] Not Support!!\n", name);
        return -1;
    }

    *curvol = base_vol + idac_get_voloff(idac_s);

    return 0;
}

int idac_set_gpio_analog_mode(int gpio)
{
    if (gpio == PAD_PM_GPIO3)
    {
        SETREG16(BASE_REG_PMPADTOP_PA + REG_ID_64, BIT2);
    }
    else if (gpio == PAD_PM_GPIO4)
    {
        SETREG16(BASE_REG_PMPADTOP_PA + REG_ID_64, BIT3);
    }
    else if (gpio == PAD_PM_GPIO5)
    {
        SETREG16(BASE_REG_PMPADTOP_PA + REG_ID_64, BIT0);
    }
    else
    {
        printk(KERN_ERR "Failed to set gpio%d to analog mode!\n", gpio);
        return -1;
    }

    return 0;
}

static void idac_init_core(void)
{
    // Step1.set 0x14_h7F[0]=1 and write back trim value
    SETREG16(BASE_REG_PMSAR_PA + REG_ID_7F, BIT0);
    // Step2. EN_IDAC
    SETREG16(REG_EN_IDAC_CORE, BIT0);
    // Step3. Adjust CURR_TRIM[5:0] in SW mode
    SETREG16(REG_IDAC_CURR_CORE_ADJ, BIT6); // bit[6] reg_sw_mode, 1: enable to set idac current by SW
}

static void idac_init_cpu(void)
{
    // Step1.set 0x14_h7F[0]=1 and write back trim value
    SETREG16(BASE_REG_PMSAR_PA + REG_ID_7F, BIT0);
    // Step2. EN_IDAC_CPU
    SETREG16(REG_EN_IDAC_CPU, BIT8);
    // Step3. Adjust CURR_TRIM[5:0] in SW mode
    SETREG16(REG_IDAC_CURR_CPU_ADJ, BIT6);
}

static void idac_init_ipu(void)
{
    // Step1.set 0x14_h7F[0]=1 and write back trim value
    SETREG16(BASE_REG_PMSAR_PA + REG_ID_7F, BIT0);
    // Step2. EN_IDAC_DLA
    SETREG16(REG_EN_IDAC_IPU, BIT0);
    // Step3. Adjust CURR_TRIM[5:0] in SW mode
    SETREG16(REG_IDAC_CURR_IPU_ADJ, BIT6);
}

int idac_init_one(const char *name)
{
    if (!strcmp(name, "core_power"))
    {
        idac_init_core();
    }
    else if (!strcmp(name, "cpu_power"))
    {
        idac_init_cpu();
    }
    else if (!strcmp(name, "ipu_power"))
    {
        idac_init_ipu();
    }
    else
    {
        printk(KERN_ERR "[%s] Not Support!!\n", name);
        return -1;
    }
    return 0;
}

static int idac_get_core_max_vol_offset(void)
{
    // return 20 * (INREG16(BASE_REG_OTP3_PA + REG_ID_42) & 0xF); /* 1uA * R(20k) * max_code*/
    return 20 * 0xF;
}

static int idac_get_cpu_max_vol_offset(void)
{
    // return 20 * ((INREG16(BASE_REG_OTP3_PA + REG_ID_42) >> 4) & 0xF); /* 1uA * R(20k) * max_code*/
    return 20 * 0xF;
}

static int idac_get_ipu_max_vol_offset(void)
{
    return 20 * 0xF;
}

int idac_get_max_vol_offset(const char *name)
{
    if (!strcmp(name, "core_power"))
    {
        return idac_get_core_max_vol_offset();
    }
    else if (!strcmp(name, "cpu_power"))
    {
        return idac_get_cpu_max_vol_offset();
    }
    else if (!strcmp(name, "ipu_power"))
    {
        return idac_get_ipu_max_vol_offset();
    }
    else
    {
        printk(KERN_ERR "[%s] Not Support!!\n", name);
        return -1;
    }
}

int voltage_request_chip(const char *name)
{
    u32                 init_voltage;
    struct device_node *np = NULL;

    np = of_find_node_by_name(NULL, name);

    if (of_property_read_u32(np, "init_voltage", &init_voltage))
    {
        if (!strcmp(name, "core_power"))
        {
            set_core_voltage("core_power", VOLTAGE_DEMANDER_USER, VOLTAGE_CORE_1000);
        }
        else if (!strcmp(name, "cpu_power"))
        {
            set_core_voltage("cpu_power", VOLTAGE_DEMANDER_USER, VOLTAGE_CORE_1000);
        }
        else if (!strcmp(name, "ipu_power"))
        {
            set_core_voltage("ipu_power", VOLTAGE_DEMANDER_USER, VOLTAGE_CORE_1000);
        }
    }
    else
    {
        if (!strcmp(name, "core_power"))
        {
            set_core_voltage("core_power", VOLTAGE_DEMANDER_USER, init_voltage);
        }
        else if (!strcmp(name, "cpu_power"))
        {
            set_core_voltage("cpu_power", VOLTAGE_DEMANDER_USER, init_voltage);
        }
        else if (!strcmp(name, "ipu_power"))
        {
            set_core_voltage("ipu_power", VOLTAGE_DEMANDER_USER, init_voltage);
        }
    }
    return 0;
}
