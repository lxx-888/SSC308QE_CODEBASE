/*
 * cpufreq.c- Sigmastar
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
#include <linux/module.h>
#include <linux/init.h>
#include <linux/cpufreq.h>
#include <linux/cpu.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/clk-provider.h>
#include <linux/delay.h>
#include <linux/version.h>
#include <linux/pm_opp.h>
#include <linux/kthread.h>
#include <opp.h>

#include "ms_types.h"
#include "ms_platform.h"
#include "registers.h"
#include "voltage_ctrl.h"

static struct device *                 cpu;
static struct cpufreq_frequency_table *freq_table;
typedef enum
{
    E_IC_SS, // ss
    E_IC_FF, // ff
} IC_TYPE_E;

typedef struct
{
    unsigned int SIDD;
    unsigned int SIDDThreshold;

    unsigned int OSC;
    unsigned int OSCThreshold;
    unsigned int ICType;
} SS_IC_INFO;

SS_IC_INFO icInfo = {0, 236, 0, 464, E_IC_SS};

int ms_get_temp(void)
{
    int vbe_code_ft;
    int vbe_code;

    CLRREG16(BASE_REG_PMSAR_PA + REG_ID_19, BIT6);        // ch7 reference voltage select to 2.0V
    CLRREG16(BASE_REG_PMSAR_PA + REG_ID_10, BIT0);        // reg_pm_dmy
    SETREG16(BASE_REG_PMSLEEP_PA + REG_ID_64, BIT10);     // reg_gcr_sel_ext_int, select current flow to sar or t_sen
    SETREG16(BASE_REG_PMSLEEP_PA + REG_ID_2F, BIT2);      // reg_en_tsen, enable current to sar or t_sensor
    OUTREG16(BASE_REG_PMSAR_PA + REG_ID_00, 0xA20);       // sar_freerun
    SETREG16(BASE_REG_PMSAR_PA + REG_ID_00, BIT14);       // enable load sar code
    vbe_code    = INREG16(BASE_REG_PMSAR_PA + REG_ID_46); // sar adc output 7
    vbe_code_ft = ((INREG16(BASE_REG_OTP3_PA + REG_ID_40) & 0xFFC0) >> 6);
    if (vbe_code_ft == 0) // if no trim info
        vbe_code_ft = 803;

    // calculate temperature
    return (639 * (vbe_code_ft - vbe_code)) / 1000 + 27;
}
EXPORT_SYMBOL(ms_get_temp);

static void ms_cpufreq_update_ic_type(void)
{
    if ((icInfo.OSC > icInfo.OSCThreshold) && (icInfo.SIDD > icInfo.SIDDThreshold))
    {
        icInfo.ICType = E_IC_FF;
    }
    else
    {
        icInfo.ICType = E_IC_SS;
    }
}

static unsigned long pm_opp_get_min_voltage(struct dev_pm_opp *opp)
{
    if (IS_ERR_OR_NULL(opp))
    {
        pr_err("%s: Invalid parameters\n", __func__);
        return 0;
    }

    return opp->supplies[0].u_volt_min;
}

static int ms_cpufreq_target_index(struct cpufreq_policy *policy, unsigned int index)
{
    struct cpufreq_freqs freqs;
    int                  ret;
    struct dev_pm_opp *  opp;
    unsigned long        new_freq;
    u32                  opp_voltage_mV;
#if defined(CONFIG_SSTAR_VOLTAGE_CTRL) || defined(CONFIG_SSTAR_VOLTAGE_IDAC_CTRL)
    u32 voltage_mV;
#endif

    freqs.old = policy->cur;
    freqs.new = freq_table[index].frequency;
    new_freq  = freqs.new * 1000;

    rcu_read_lock();
    opp = dev_pm_opp_find_freq_ceil(cpu, &new_freq);
    if (IS_ERR(opp))
    {
        rcu_read_unlock();
        pr_err("[%s] %d not found in OPP\n", __func__, freqs.new);
        return -EINVAL;
    }

    if (icInfo.ICType == E_IC_FF)
        opp_voltage_mV = (pm_opp_get_min_voltage(opp) ? pm_opp_get_min_voltage(opp) / 1000 : 0);
    else
        opp_voltage_mV = (dev_pm_opp_get_voltage(opp) ? dev_pm_opp_get_voltage(opp) / 1000 : 0);

    rcu_read_unlock();

#if defined(CONFIG_SSTAR_VOLTAGE_CTRL) || defined(CONFIG_SSTAR_VOLTAGE_IDAC_CTRL)
    if (!get_core_voltage("cpu_power", &voltage_mV) && opp_voltage_mV > voltage_mV)
    {
        set_core_voltage("cpu_power", VOLTAGE_DEMANDER_CPUFREQ, opp_voltage_mV);
        udelay(10); // delay 10us to wait voltage stable (from low to high).
        ret = clk_set_rate(policy->clk, new_freq);
    }
    else
    {
        ret = clk_set_rate(policy->clk, new_freq);
        set_core_voltage("cpu_power", VOLTAGE_DEMANDER_CPUFREQ, opp_voltage_mV);
    }
#else
    ret = clk_set_rate(policy->clk, new_freq);
#endif

    return ret;
}

static ssize_t show_cpufreq_testout(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    char *str       = buf;
    char *end       = buf + PAGE_SIZE;
    u8    retry     = 0;
    u32   reg_value = 0;
    u32   freq      = 0;

    // disable clock calc
    CLRREG16(BASE_REG_MCU_PA + BK_REG(0x1F), BIT14);

    // clear done
    SETREG16(BASE_REG_MCU_PA + BK_REG(0x1F), BIT13);

    // should wait for 1000/12MHz T
    while ((INREG16(BASE_REG_MCU_PA + BK_REG(0x1F)) & BIT15) && (retry++ < 5))
    {
        udelay(100);
    }

    // enable clock calc
    SETREG16(BASE_REG_MCU_PA + BK_REG(0x1F), BIT14);

    retry = 0;
    // should wait for 1000/12MHz T
    while ((!(INREG16(BASE_REG_MCU_PA + BK_REG(0x1F)) & BIT15)) && (retry++ < 5))
    {
        udelay(100);
    }

    reg_value = (INREG16(BASE_REG_MCU_PA + BK_REG(0x1D)) | (INREG16(BASE_REG_MCU_PA + BK_REG(0x1E)) << 16));
    freq      = reg_value * 12 * 1000;

    str += scnprintf(str, end - str, "%d\n", freq);

    return (str - buf);
}

define_one_global_ro(cpufreq_testout);

static ssize_t show_temp_out(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    char *str  = buf;
    char *end  = buf + PAGE_SIZE;
    int   temp = ms_get_temp();

    str += scnprintf(str, end - str, "Temp=%d\n", temp);

    return (str - buf);
}
define_one_global_ro(temp_out);

// #define __DEBUG_SIDD__
#ifndef __DEBUG_SIDD__
static ssize_t show_sidd_out(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    char *str = buf;
    char *end = buf + PAGE_SIZE;

    str += scnprintf(str, end - str, "%d\n", icInfo.SIDD);

    return (str - buf);
}
define_one_global_ro(sidd_out);
#else

static ssize_t show_sidd_out(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    char *str = buf;
    char *end = buf + PAGE_SIZE;

    str += scnprintf(str, end - str, "%d\n", icInfo.SIDD);

    return (str - buf);
}

static ssize_t store_sidd_out(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count)
{
    u32 value;
    if (sscanf(buf, "%d", &value) <= 0)
        return 0;
    icInfo.SIDD = value;
    ms_cpufreq_update_ic_type();
    return count;
}
define_one_global_rw(sidd_out);
#endif

// #define __DEBUG_OSC__
#ifndef __DEBUG_OSC__
static ssize_t show_osc_out(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    char *str = buf;
    char *end = buf + PAGE_SIZE;

    str += scnprintf(str, end - str, "%d\n", icInfo.OSC);

    return (str - buf);
}
define_one_global_ro(osc_out);

#else

static ssize_t show_osc_out(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    char *str = buf;
    char *end = buf + PAGE_SIZE;

    str += scnprintf(str, end - str, "%d\n", icInfo.OSC);

    return (str - buf);
}

static ssize_t store_osc_out(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count)
{
    u32 value;
    if (sscanf(buf, "%d", &value) <= 0)
        return 0;
    icInfo.OSC = value;
    ms_cpufreq_update_ic_type();
    return count;
}

define_one_global_rw(osc_out);
#endif

static ssize_t show_osc_threshold(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    char *str = buf;
    char *end = buf + PAGE_SIZE;

    str += scnprintf(str, end - str, "%d\n", icInfo.OSCThreshold);

    return (str - buf);
}

define_one_global_ro(osc_threshold);

static ssize_t show_sidd_threshold(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    char *str = buf;
    char *end = buf + PAGE_SIZE;

    str += scnprintf(str, end - str, "%d\n", icInfo.SIDDThreshold);

    return (str - buf);
}

define_one_global_ro(sidd_threshold);

static int ms_cpufreq_init(struct cpufreq_policy *policy)
{
    int ret;

    if (policy->cpu != 0)
        return -EINVAL;

    policy->clk = devm_clk_get(cpu, 0);
    if (IS_ERR(policy->clk))
    {
        pr_err("[%s] get cpu clk fail\n", __func__);
        return PTR_ERR(policy->clk);
    }

    ret = dev_pm_opp_init_cpufreq_table(cpu, &freq_table);
    if (ret)
    {
        pr_err("[%s] init OPP fail\n", __func__);
        return ret;
    }

    cpufreq_generic_init(policy, freq_table, 100000);

    icInfo.SIDD = INREG16(BASE_REG_OTP2_PA + REG_ID_0A);
    icInfo.OSC  = INREG16(BASE_REG_OTP2_PA + REG_ID_0B);
    ms_cpufreq_update_ic_type();

    ms_get_temp(); // We will update temperature after 1sec. Drop first value due to one adc need cost 8ch*8.9usec.

    pr_info("[%s] Current clk=%lu\n", __func__, clk_get_rate(policy->clk));

    return ret;
}

static int ms_cpufreq_exit(struct cpufreq_policy *policy)
{
    dev_pm_opp_free_cpufreq_table(cpu, &freq_table);

    return 0;
}

static struct cpufreq_driver ms_cpufreq_driver = {
    .verify       = cpufreq_generic_frequency_table_verify,
    .attr         = cpufreq_generic_attr,
    .target_index = ms_cpufreq_target_index,
    .get          = cpufreq_generic_get,
    .init         = ms_cpufreq_init,
    .exit         = ms_cpufreq_exit,
    .name         = "Mstar cpufreq",
};

static int ms_cpufreq_probe(struct platform_device *pdev)
{
    int            ret;
    struct clk *   clk;
    struct clk_hw *hw_parent;

    cpu = get_cpu_device(0);
    if (dev_pm_opp_of_add_table(cpu))
    {
        pr_err("[%s] add OPP fail\n", __func__);
        return -EINVAL;
    }

    clk = of_clk_get(pdev->dev.of_node, 0);
    if (IS_ERR(clk))
    {
        ret = PTR_ERR(clk);
        return ret;
    }
    else
    {
        hw_parent = clk_hw_get_parent_by_index(__clk_get_hw(clk), 0); // select mux 0
        if (!hw_parent)
            return -ENXIO;

        ret = clk_set_parent(clk, hw_parent->clk);
        if (ret)
            return ret;

        ret = clk_prepare_enable(clk);
        if (ret)
            return ret;
    }

    platform_set_drvdata(pdev, clk);

#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 4, 0)
    ret = cpufreq_sysfs_create_file(&cpufreq_testout.attr);
    ret |= cpufreq_sysfs_create_file(&temp_out.attr);
    ret |= cpufreq_sysfs_create_file(&sidd_out.attr);
    ret |= cpufreq_sysfs_create_file(&sidd_threshold.attr);
    ret |= cpufreq_sysfs_create_file(&osc_out.attr);
    ret |= cpufreq_sysfs_create_file(&osc_threshold.attr);
#else
    ret = sysfs_create_file(cpufreq_global_kobject, &cpufreq_testout.attr);
    ret |= sysfs_create_file(cpufreq_global_kobject, &temp_out.attr);
    ret |= sysfs_create_file(cpufreq_global_kobject, &sidd_out.attr);
    ret |= sysfs_create_file(cpufreq_global_kobject, &sidd_threshold.attr);
    ret |= sysfs_create_file(cpufreq_global_kobject, &osc_out.attr);
    ret |= sysfs_create_file(cpufreq_global_kobject, &osc_threshold.attr);
#endif

    if (ret)
    {
        pr_err("[%s] create file fail\n", __func__);
    }

    return cpufreq_register_driver(&ms_cpufreq_driver);
}

static int ms_cpufreq_remove(struct platform_device *pdev)
{
    struct clk *clk = platform_get_drvdata(pdev);

    clk_disable_unprepare(clk);

#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 4, 0)
    cpufreq_sysfs_remove_file(&cpufreq_testout.attr);
    cpufreq_sysfs_remove_file(&temp_out.attr);
    cpufreq_sysfs_remove_file(&sidd_out.attr);
    cpufreq_sysfs_remove_file(&sidd_threshold.attr);
    cpufreq_sysfs_remove_file(&osc_out.attr);
    cpufreq_sysfs_remove_file(&osc_threshold.attr);
#else
    sysfs_remove_file(cpufreq_global_kobject, &cpufreq_testout.attr);
    sysfs_remove_file(cpufreq_global_kobject, &temp_out.attr);
    sysfs_remove_file(cpufreq_global_kobject, &sidd_out.attr);
    sysfs_remove_file(cpufreq_global_kobject, &sidd_threshold.attr);
    sysfs_remove_file(cpufreq_global_kobject, &osc_out.attr);
    sysfs_remove_file(cpufreq_global_kobject, &osc_threshold.attr);
#endif
    return cpufreq_unregister_driver(&ms_cpufreq_driver);
}

static const struct of_device_id ms_cpufreq_of_match_table[] = {{.compatible = "sstar,cpufreq"}, {}};
MODULE_DEVICE_TABLE(of, ms_cpufreq_of_match_table);

static struct platform_driver ms_cpufreq_platdrv = {
    .driver =
        {
            .name           = "ms_cpufreq",
            .owner          = THIS_MODULE,
            .of_match_table = ms_cpufreq_of_match_table,
        },
    .probe  = ms_cpufreq_probe,
    .remove = ms_cpufreq_remove,
};

module_platform_driver(ms_cpufreq_platdrv);
