/*
 * drv_clk_os_manage.c- Sigmastar
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
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>
#include <linux/sched.h>
#include <linux/kernel.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/slab.h>
#include <asm/io.h>
#include <linux/clk.h>
#include <linux/clk-provider.h>
#include <cam_inter_os.h>
#include <cam_os_wrapper.h>
#include "drv_camclk_DataType.h"

#define INTEROS_L2R_CCF_INIT       0xF10F0000
#define INTEROS_R2L_CCF_SYNC_START 0xF10FFFFE
#define INTEROS_L2R_CCF_SYNC_DONE  0xF10FFFFF

#define INTEROS_R2L_CLK_SET_EN     0xF10F0001
#define INTEROS_R2L_CLK_SET_RATE   0xF10F0002
#define INTEROS_R2L_CLK_SET_PARENT 0xF10F0003
#define INTEROS_R2L_CLK_GET_EN     0xF10F0004
#define INTEROS_R2L_CLK_GET_RATE   0xF10F0005
#define INTEROS_R2L_CLK_GET_PARENT 0xF10F0006
#define INTEROS_R2L_CLK_GET_ROUND  0xF10F0007

#define INTEROS_L2R_CLK_GET_EN     0xF10F0010
#define INTEROS_L2R_CLK_GET_RATE   0xF10F0011
#define INTEROS_L2R_CLK_GET_PARENT 0xF10F0012
#define INTEROS_L2R_CLK_GET_ROUND  0xF10F0013

#define DATA_CACHE_ENTRY_SIZE 64

struct mutex clk_os_mutex;
static bool  clock_sync_done = false;

#define CLK_OS_DEBUG 0
#if CLK_OS_DEBUG
#define clk_os_dbg(fmt, ...) CamOsPrintf("%s: " fmt, __func__, ##__VA_ARGS__)
#else
#define clk_os_dbg(fmt, ...)
#endif
#define clk_os_err(fmt, ...)        \
    CamOsPrintf(                    \
        "%s: "                      \
        "\033[1;31m" fmt "\033[0m", \
        __func__, ##__VA_ARGS__)
#define clk_os_info(fmt, ...) CamOsPrintf("%s: " fmt, __func__, ##__VA_ARGS__)

#ifdef CONFIG_SS_PROFILING_TIME
extern void recode_timestamp(int mark, const char *name);
#else
#define recode_timestamp(mark, name)
#endif

#ifdef CONFIG_SS_DUALOS
static bool       disable_rtos = false;
static int __init disable_rtos_func(char *arg)
{
    strtobool(arg, &disable_rtos);
    return 0;
}
early_param("disable_rtos", disable_rtos_func);

void sstar_clk_os_set_enable(u32 arg0, u32 arg1, u32 arg2, u32 arg3)
{
    struct clk *        clk;
    struct device_node *clk_os_manage_node = of_find_compatible_node(NULL, NULL, "sstar,clk-os-manage");
    mutex_lock(&clk_os_mutex);

    clk_os_dbg("set clock[%d] enable state: %d\n", arg1, arg2);

    clk = of_clk_get(clk_os_manage_node, arg1);

    if (arg2)
    {
        clk_prepare_enable(clk);
    }
    else
    {
        clk_disable_unprepare(clk);
    }

    clk_put(clk);
    mutex_unlock(&clk_os_mutex);
}

void sstar_clk_os_set_rate(u32 arg0, u32 arg1, u32 arg2, u32 arg3)
{
    struct clk *        clk;
    struct device_node *clk_os_manage_node = of_find_compatible_node(NULL, NULL, "sstar,clk-os-manage");

    clk_os_dbg("set clock[%d] rate to: %d Hz\n", arg1, arg2);
    mutex_lock(&clk_os_mutex);

    clk = of_clk_get(clk_os_manage_node, arg1);
    clk_set_rate(clk, arg2);
    clk_put(clk);
    mutex_unlock(&clk_os_mutex);
}

void sstar_clk_os_set_parent(u32 arg0, u32 arg1, u32 arg2, u32 arg3)
{
    struct clk *        clk;
    struct device_node *clk_os_manage_node = of_find_compatible_node(NULL, NULL, "sstar,clk-os-manage");

    clk_os_dbg("set clock[%d] to parent[%d]\n", arg1, arg2);
    mutex_lock(&clk_os_mutex);

    clk = of_clk_get(clk_os_manage_node, arg1);

    clk_set_parent(clk, clk_hw_get_parent_by_index(__clk_get_hw(clk), arg2)->clk);
    clk_put(clk);
    mutex_unlock(&clk_os_mutex);
}

void sstar_clk_os_get_enable(u32 arg0, u32 arg1, u32 arg2, u32 arg3)
{
    struct clk *        clk;
    u32                 enable;
    struct device_node *clk_os_manage_node = of_find_compatible_node(NULL, NULL, "sstar,clk-os-manage");
    mutex_lock(&clk_os_mutex);

    clk    = of_clk_get(clk_os_manage_node, arg1);
    enable = __clk_is_enabled(clk);

    clk_os_dbg("clock[%d] enable state is: %d\n", arg1, enable);

    clk_put(clk);

    CamInterOsSignal(INTEROS_L2R_CLK_GET_EN, arg1, enable, 0);
    mutex_unlock(&clk_os_mutex);
}

void sstar_clk_os_get_rate(u32 arg0, u32 arg1, u32 arg2, u32 arg3)
{
    struct clk *clk;
    u32         rate;

    struct device_node *clk_os_manage_node = of_find_compatible_node(NULL, NULL, "sstar,clk-os-manage");
    mutex_lock(&clk_os_mutex);

    clk  = of_clk_get(clk_os_manage_node, arg1);
    rate = clk_get_rate(clk);

    clk_os_dbg("clock[%d] rate is: %d Hz\n", arg1, rate);
    clk_put(clk);

    CamInterOsSignal(INTEROS_L2R_CLK_GET_RATE, arg1, rate, 0);
    mutex_unlock(&clk_os_mutex);
}

void sstar_clk_os_get_parent(u32 arg0, u32 arg1, u32 arg2, u32 arg3)
{
    struct clk *        clk;
    struct clk *        parent_clk;
    u16                 shift;
    struct device_node *clk_os_manage_node = of_find_compatible_node(NULL, NULL, "sstar,clk-os-manage");
    mutex_lock(&clk_os_mutex);

    clk = of_clk_get(clk_os_manage_node, arg1);
    for (shift = 0; shift < clk_hw_get_num_parents(__clk_get_hw(clk)); shift++)
    {
        parent_clk = clk_get_parent(clk);
        if (parent_clk == clk_hw_get_parent_by_index(__clk_get_hw(clk), shift)->clk)
        {
            break;
        }
    }

    clk_os_dbg("[%s]clock[%d] current source is parent[%d]\n", arg1, shift);
    clk_put(clk);

    CamInterOsSignal(INTEROS_L2R_CLK_GET_PARENT, arg1, shift, 0);
    mutex_unlock(&clk_os_mutex);
}

void sstar_clk_os_get_round(u32 arg0, u32 arg1, u32 arg2, u32 arg3)
{
    struct clk *clk;
    u32         rate;

    struct device_node *clk_os_manage_node = of_find_compatible_node(NULL, NULL, "sstar,clk-os-manage");
    mutex_lock(&clk_os_mutex);

    clk  = of_clk_get(clk_os_manage_node, arg1);
    rate = clk_round_rate(clk, arg2);

    clk_os_dbg("clock[%d] round rate is: %d Hz\n", arg1, rate);
    clk_put(clk);

    CamInterOsSignal(INTEROS_L2R_CLK_GET_ROUND, arg1, rate, 0);
    mutex_unlock(&clk_os_mutex);
}

void sstar_clk_os_ccf_sync_start(u32 arg0, u32 cfgPa, u32 u32ClkCnt, u32 arg3)
{
    u32                    index = 0, encnt = 0, cfgsize = 0;
    CAMCLK_Sync_Attribute *pstAttrs = NULL, *pstCfg = NULL;

    clk_os_dbg("sync start,clknum:%d\n", u32ClkCnt);
    if (u32ClkCnt == 0)
    {
        clk_os_info("no need to sync clock\n");
        goto sync_exit;
    }
    cfgsize = CAM_OS_ALIGN_UP(u32ClkCnt * sizeof(CAMCLK_Sync_Attribute), DATA_CACHE_ENTRY_SIZE);

    pstAttrs = (CAMCLK_Sync_Attribute *)ioremap_cache(cfgPa, cfgsize);
    if (!pstAttrs)
    {
        clk_os_err("failed to get cfg buf\n");
        goto sync_exit;
    }
    CamOsMemInvalidate(pstAttrs, cfgsize);

    for (index = 0, pstCfg = pstAttrs; index < u32ClkCnt; index++, pstCfg++)
    {
        clk_os_dbg("total:%d index:%d clk:%d type:%d\n", u32ClkCnt, index, pstCfg->u32ClkId, pstCfg->eSetType);
        switch (pstCfg->eSetType)
        {
            case CAMCLK_SET_ATTR_PARENT:
                sstar_clk_os_set_parent(arg0, pstCfg->u32ClkId, pstCfg->u16ParentShift, 0);
                break;
            case CAMCLK_SET_ATTR_RATE:
                sstar_clk_os_set_rate(arg0, pstCfg->u32ClkId, pstCfg->u32Rate, 0);
                break;
            default:
                break;
        }
        for (encnt = 0; encnt < pstCfg->u32EnableCnt; encnt++)
        {
            sstar_clk_os_set_enable(arg0, pstCfg->u32ClkId, 1, 0);
        }
    }
    iounmap((char *)pstAttrs);

sync_exit:
    CamInterOsSignal(INTEROS_L2R_CCF_SYNC_DONE, 0, 0, 0);
    recode_timestamp(__LINE__, "clk sync-");
    clock_sync_done = true;
}
#endif

static int __init sstar_clk_os_manage_init(void)
{
#ifdef CONFIG_SS_DUALOS
    s32 ret = 0;
    mutex_init(&clk_os_mutex);

    CamInterOsSignalReg(INTEROS_R2L_CLK_SET_EN, sstar_clk_os_set_enable, "camclk_set_on_off");
    CamInterOsSignalReg(INTEROS_R2L_CLK_SET_RATE, sstar_clk_os_set_rate, "camclk_set_rate");
    CamInterOsSignalReg(INTEROS_R2L_CLK_SET_PARENT, sstar_clk_os_set_parent, "camclk_set_parent");
    CamInterOsSignalReg(INTEROS_R2L_CLK_GET_EN, sstar_clk_os_get_enable, "camclk_get_on_off");
    CamInterOsSignalReg(INTEROS_R2L_CLK_GET_RATE, sstar_clk_os_get_rate, "camclk_get_rate");
    CamInterOsSignalReg(INTEROS_R2L_CLK_GET_PARENT, sstar_clk_os_get_parent, "camclk_get_parent");
    CamInterOsSignalReg(INTEROS_R2L_CLK_GET_ROUND, sstar_clk_os_get_round, "camclk_get_round");
    CamInterOsSignalReg(INTEROS_R2L_CCF_SYNC_START, sstar_clk_os_ccf_sync_start, "camclk_sync_start");

    recode_timestamp(__LINE__, "clk sync+");
    ret = CamInterOsSignal(INTEROS_L2R_CCF_INIT, 1, 0, 0);
    if (ret)
    {
        clk_os_err("clk os manage failed to connect to the rtos\n");
        clock_sync_done = true;
    }
    clk_os_info("camclk set on callback registered\n");
#endif
    return 0;
}

void sstar_clk_os_manage_wait_sync_done(unsigned int max_wait_time)
{
#ifdef CONFIG_SS_DUALOS
    unsigned int wait_ms_time = 0;
    unsigned int tick_time    = 1;

    if (disable_rtos)
    {
        return;
    }
    do
    {
        if (clock_sync_done)
        {
            break;
        }
        CamOsUsSleep(tick_time * 1000);
        wait_ms_time += tick_time;
    } while (wait_ms_time < max_wait_time);
#endif
}
EXPORT_SYMBOL(sstar_clk_os_manage_wait_sync_done);

static void __exit sstar_clk_os_manage_exit(void)
{
    return;
}

device_initcall_sync(sstar_clk_os_manage_init);
module_exit(sstar_clk_os_manage_exit);

MODULE_AUTHOR("SSTAR");
MODULE_DESCRIPTION("clk os manage driver");
MODULE_LICENSE("GPL");
