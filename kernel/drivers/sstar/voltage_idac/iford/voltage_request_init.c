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
#include "registers.h"
#include "ms_platform.h"
#include "voltage_ctrl.h"
#include "voltage_ctrl_demander.h"
#include "gpio.h"
#include "voltage_request_init.h"
#include <linux/delay.h>

#define REG_EN_IDAC_CORE       (BASE_REG_PMSAR_PA + REG_ID_73)
#define REG_IDAC_CURR_CORE_ADJ (BASE_REG_PMSAR_PA + REG_ID_71)
#define REG_EN_IDAC_CPU        (BASE_REG_PMSLEEP_PA + REG_ID_74)
#define REG_IDAC_CURR_CPU_ADJ  (BASE_REG_PMSAR1_PA + REG_ID_11)
//#define REG_EN_IDAC_IPU        (BASE_REG_PMSLEEP_PA + REG_ID_73)
//#define REG_IDAC_CURR_IPU_ADJ  (BASE_REG_PMSAR_PA + REG_ID_61)

//####### IDAC VOLTAGE DEFINE FORMAT #########//
// R: 20k ohm
// BIT5:   0.5uA/1uA  (1/0)
#define IDAC_STEP_CUR_SHIFT  5
#define IDAC_STEP_CUR_MASK   BIT5
#define IDAC_STEP_CUR_SEL(n) (((n)&1) << IDAC_STEP_CUR_SHIFT)
#define IDAC_STEP_CUR_VAL(v) (((v)&IDAC_STEP_CUR_MASK) >> IDAC_STEP_CUR_SHIFT)
// BIT4:   Plus/Minus (1/0)
#define IDAC_SRC_SINK_SHIFT  4
#define IDAC_SRC_SINK_MASK   BIT4
#define IDAC_SRC_SINK_SEL(n) (((n)&1) << IDAC_SRC_SINK_SHIFT)
#define IDAC_SRC_SINK_VAL(v) (((v)&IDAC_SRC_SINK_MASK) >> IDAC_SRC_SINK_SHIFT)
// BIT3-0: Step	(0-F)
#define IDAC_STEP_MASK   (BIT3 | BIT2 | BIT1 | BIT0)
#define IDAC_STEP_VAL(v) ((v)&IDAC_STEP_MASK)

#define IDAC_TB_VALID_ROW 2
#define IDAC_TB_VALID_COL 31
static const U8 Idac_contrl_tb[IDAC_TB_VALID_ROW][IDAC_TB_VALID_COL] = {
    {0x00, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F,
     0x18, 0x18, 0x19, 0x19, 0x1A, 0x1A, 0x1B, 0x1B, 0x1C, 0x1C, 0x1D, 0x1D, 0x1E, 0x1E, 0x1F},
    {0x00, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F,
     0x08, 0x09, 0x09, 0x0A, 0x0A, 0x0B, 0x0B, 0x0C, 0x0C, 0x0D, 0x0D, 0x0E, 0x0E, 0x0F, 0x0F}};

#define IDAC_DBG 0

#if defined(CONFIG_SSTAR_VOLTAGE_IDAC_MIU_PHASE_SCALING_EN)
int        g_curr_vol = 0;
extern int miudq_dvs_phase_scaling(void);
#endif

static U16 idac_get_control_setting(int voloff)
{
    int row    = !!(voloff < 0);
    int column = abs(voloff) / 10; // 10mV per step

    if (column > IDAC_TB_VALID_COL)
    {
        printk(KERN_ERR "[%s] the column [%d] is invalid. reset column as 0\n", __FUNCTION__, column);
        column = 0;
    }

    return Idac_contrl_tb[row][column];
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

#if defined(CONFIG_SSTAR_VOLTAGE_IDAC_MIU_PHASE_SCALING_EN)
U16 idac_setvol_step_by_step(int base_vol, int setvol)
{
    U16 tmpval, num;
    U16 idac_setvol = idac_get_control_setting(setvol - base_vol);
    U16 idac_curvol = idac_get_control_setting(g_curr_vol - base_vol);

#if IDAC_DBG
    printk("%s %d: base_vol=%d, setvol=%d, curr_vol=%d\r\n", __FUNCTION__, __LINE__, base_vol, setvol, g_curr_vol);
#endif
    num = 0;
    while (idac_setvol != idac_curvol)
    {
        if (g_curr_vol > setvol)
        {
            g_curr_vol -= 10;
        }
        else
        {
            g_curr_vol += 10;
        }
        idac_curvol = idac_get_control_setting(g_curr_vol - base_vol);
        tmpval      = (INREG16(REG_IDAC_CURR_CORE_ADJ) & 0xC0) | idac_curvol;
        OUTREG16(REG_IDAC_CURR_CORE_ADJ, tmpval);
        ndelay(100);

        miudq_dvs_phase_scaling();
        num++;
        if (num > IDAC_TB_VALID_COL * 2)
        {
            printk("Set voltage %d is not supported.\r\n", setvol);
        }
    }
#if IDAC_DBG
    printk("%s %d: current vol=%d\r\n", __FUNCTION__, __LINE__, g_curr_vol);
#endif
    return 0;
}

#endif

int idac_set_voltage(const char *name, int base_vol, int setvol)
{
    int voloff = setvol - base_vol;
    u16 mask   = IDAC_STEP_CUR_MASK | IDAC_SRC_SINK_MASK | IDAC_STEP_MASK;

#if IDAC_DBG
    printk("%s %d: name=%s, base_vol=%d, setvol=%d\r\n", __FUNCTION__, __LINE__, name, base_vol, setvol);
#endif

    if (!strcmp(name, "core_power"))
    {
#if defined(CONFIG_SSTAR_VOLTAGE_IDAC_MIU_PHASE_SCALING_EN)
        if (!g_curr_vol)
        {
            g_curr_vol = base_vol;
        }
        idac_setvol_step_by_step(base_vol, setvol);
#else
        OUTREGMSK16(REG_IDAC_CURR_CORE_ADJ, idac_get_control_setting(voloff), mask);
#endif
    }
    else if (!strcmp(name, "cpu_power"))
    {
        OUTREGMSK16(REG_IDAC_CURR_CPU_ADJ, idac_get_control_setting(voloff), mask);
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
    else
    {
        printk(KERN_ERR "[%s] Not Support!!\n", name);
        return -1;
    }

    *curvol = base_vol + idac_get_voloff(idac_s);

    return 0;
}

int idac_set_voltage_reg(const char *name, int type, u16 value)
{
    u16 mask = 0;

    if (strcmp(name, "core_power"))
        return -1;

    switch (type)
    {
        case IDAC_CP_VOLTAGE_STEP:
            mask  = IDAC_STEP_MASK;
            value = IDAC_STEP_VAL(value);
            break;
        case IDAC_CP_STEP_CURRENT:
            mask  = IDAC_STEP_CUR_MASK;
            value = IDAC_STEP_CUR_SEL(value);
            break;
        case IDAC_CP_SINK_SOURCE:
            mask  = IDAC_SRC_SINK_MASK;
            value = IDAC_SRC_SINK_SEL(value);
            break;
        default:
            return -1;
    }

    OUTREGMSK16(REG_IDAC_CURR_CORE_ADJ, value, mask);
    return 0;
}

int idac_get_voltage_reg(const char *name, int type)
{
    u16 idac_s;

    if (strcmp(name, "core_power"))
        return -1;

    idac_s = INREG16(REG_IDAC_CURR_CORE_ADJ);
    switch (type)
    {
        case IDAC_CP_VOLTAGE_STEP:
            return IDAC_STEP_VAL(idac_s);
        case IDAC_CP_STEP_CURRENT:
            return IDAC_STEP_CUR_VAL(idac_s);
        case IDAC_CP_SINK_SOURCE:
            return IDAC_SRC_SINK_VAL(idac_s);
    }

    return 0;
}

int idac_set_gpio_analog_mode(int gpio)
{
    if (gpio == PAD_PM_GPIO4)
    {
        SETREG16(BASE_REG_PMPADTOP_PA + REG_ID_64, BIT0);
    }
    else if (gpio == PAD_PM_GPIO5)
    {
        SETREG16(BASE_REG_PMPADTOP_PA + REG_ID_64, BIT1);
    }
    else
    {
        printk(KERN_ERR "Failed to set gpio to analgo mode!!\n");
        return -1;
    }

    return 0;
}

void idac_init_core(void)
{
    // reg_en_idac_cp
    OUTREG16(REG_EN_IDAC_CORE, 0x01);
    // SW_mode[6]
    SETREG16(REG_IDAC_CURR_CORE_ADJ, BIT6);
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
    else
    {
        printk(KERN_ERR "[%s] Not Support!!\n", name);
        return -1;
    }
    return 0;
}

int idac_get_core_current_max_code(void)
{
    return 0;
}

int idac_get_cpu_current_max_code(void)
{
    return 0;
}

int idac_get_core_max_vol_offset(void)
{
    // return 20 * (INREG16(BASE_REG_OTP3_PA + REG_ID_42) & 0xF); /* 1uA * R(20k) * max_code*/
    return 20 * 0xF;
}

int idac_get_cpu_max_vol_offset(void)
{
    // return 20 * ((INREG16(BASE_REG_OTP3_PA + REG_ID_42) >> 4) & 0xF); /* 1uA * R(20k) * max_code*/
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
    else
    {
        printk(KERN_ERR "[%s] Not Support!!\n", name);
        return -1;
    }
}

int voltage_request_chip(const char *name)
{
    u32                 init_voltage = VOLTAGE_CORE_1000;
    struct device_node *np           = NULL;

    if (strcmp(name, "core_power"))
        return -1;

    np = of_find_node_by_name(NULL, name);

    if (np && of_property_read_u32(np, "init_voltage", &init_voltage))
    {
        set_core_voltage("core_power", VOLTAGE_DEMANDER_INIT, VOLTAGE_CORE_1000);
    }
    else
    {
        set_core_voltage(name, VOLTAGE_DEMANDER_INIT, init_voltage);
    }
    /* DEMANDER_INIT is then not used any more */
    set_core_voltage(name, VOLTAGE_DEMANDER_INIT, 0);

    return 0;
}
