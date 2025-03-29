/* SigmaStar trade secret */
/* Copyright (c) [2019~2020] SigmaStar Technology.
All rights reserved.

Unless otherwise stipulated in writing, any and all information contained
herein regardless in any format shall remain the sole proprietary of
SigmaStar and be kept in strict confidence
(SigmaStar Confidential Information) by the recipient.
Any unauthorized act including without limitation unauthorized disclosure,
copying, use, reproduction, sale, distribution, modification, disassembling,
reverse engineering and compiling of the contents of SigmaStar Confidential
Information is unlawful and strictly prohibited. SigmaStar hereby reserves the
rights to any and all damages, losses, costs and expenses resulting therefrom.
*/
#include <linux/moduleparam.h>
#include <linux/of_platform.h>
#include <linux/irq.h>
#include <linux/interrupt.h>
#include <linux/of.h>
#include <linux/pm_opp.h>
#include <linux/platform_device.h>
#include <linux/kobject.h>
#include "drv_clock.h"

#include "cam_sysfs.h"
#include "cam_os_wrapper.h"
#include "mi_common_internal.h"
#include "ipu_init.h"


extern int perfMonitor;
extern int freqExpe;
module_param(perfMonitor,    int, 0644);
module_param(freqExpe,    int, 0644);

int MI_IPU_ParseIrq(unsigned int *pu32Virq, int len)
{
    int devid;

    struct device_node *dev_node = NULL;
    struct platform_device *pdev= NULL;

    dev_node = of_find_compatible_node(NULL, NULL, "sstar,dla");
    if (!dev_node) {
        CamOsPrintf("fail to find dla device tree node\n");
        return -1;
    }
    pdev = of_find_device_by_node(dev_node);
    if (!pdev) {
        CamOsPrintf("fail to find dla device node\n");
        return -1;
    }

    for (devid = 0; devid < len; devid++) {
        pu32Virq[devid] = CamIrqOfParseAndMap(pdev->dev.of_node, devid);
    }

    return 0;
}

#if USE_IPU_OPP_TABLE == 1

MI_S32 MI_IPU_Parse_Freq_Vol_Table(void)
{
    MI_S32 opp_cnt, loop;
    MI_U32 freq_khz, voltage_uv;
    struct device_node *dev_node = NULL;
    struct property *prop = NULL;
    int nr;
    const __be32 *value;
    MI_IPU_Freq_Vol_t *ipu_opp_tb;

    pstDrvStruct->stFreqInfo.min_voltage = ~0U;
    pstDrvStruct->stFreqInfo.max_voltage = 0;
    pstDrvStruct->stFreqInfo.min_mhz = ~0U;
    pstDrvStruct->stFreqInfo.max_mhz = 0;

    dev_node = of_find_compatible_node(NULL, NULL, "sstar,ipu");
    if (!dev_node) {
        //CamOsPrintf("fail to find ipu device tree node\n");
        return -1;
    }

    prop = of_find_property(dev_node, "operating-points", NULL);
    if (!prop) {
        CamOsPrintf("fail to find ipu operating-points\n");
        return -1;
    }

    if (!prop->value) {
        CamOsPrintf("fail to get ipu operating-points value\n");
        return -1;
    }

    nr = prop->length / sizeof(unsigned int);
    if (nr % 2) {
        CamOsPrintf("Invalid OPP table\n");
        return -1;
    }
    opp_cnt = nr / 2;
    if (!opp_cnt) {
        CamOsPrintf("Invalid OPP table length\n");
        return -1;
    }

    ipu_opp_tb = CamOsMemAlloc(sizeof(*ipu_opp_tb)*opp_cnt);
    if (!ipu_opp_tb) {
        CamOsPrintf("fail to allocate memory for ipu opp table\n");
        return -1;
    }

    /*
    *Each OPP is a set of tuples consisting of frequency ad voltage like <kHz uV>
    */
    value = (unsigned int *)prop->value;
    for (loop = 0; loop < opp_cnt; loop++) {
        freq_khz = be32_to_cpup(value++);
        voltage_uv = be32_to_cpup(value++);
        ipu_opp_tb[loop].freq_mhz = freq_khz / 1000;
        ipu_opp_tb[loop].voltage_mv = voltage_uv / 1000;

        if (pstDrvStruct->stFreqInfo.min_mhz > ipu_opp_tb[loop].freq_mhz) {
            pstDrvStruct->stFreqInfo.min_mhz = ipu_opp_tb[loop].freq_mhz;
        }
        if (pstDrvStruct->stFreqInfo.max_mhz < ipu_opp_tb[loop].freq_mhz) {
            pstDrvStruct->stFreqInfo.max_mhz = ipu_opp_tb[loop].freq_mhz;
        }

        if (pstDrvStruct->stFreqInfo.min_voltage > ipu_opp_tb[loop].voltage_mv) {
            pstDrvStruct->stFreqInfo.min_voltage = ipu_opp_tb[loop].voltage_mv;
        }
        if (pstDrvStruct->stFreqInfo.max_voltage < ipu_opp_tb[loop].voltage_mv) {
            pstDrvStruct->stFreqInfo.max_voltage = ipu_opp_tb[loop].voltage_mv;
        }
    }

    pstDrvStruct->stFreqInfo.ipu_opp_tb = ipu_opp_tb;
    pstDrvStruct->stFreqInfo.ipu_opp_tb_len = opp_cnt;

    return 0;
}

MI_S32 MI_IPU_Destroy_Freq_Vol_Table(void)
{
    if (pstDrvStruct->stFreqInfo.ipu_opp_tb) {
        CamOsMemRelease(pstDrvStruct->stFreqInfo.ipu_opp_tb);
        pstDrvStruct->stFreqInfo.ipu_opp_tb = NULL;
    }
}

#endif

int MI_IPU_GetPllClk(void **ppPll, int index)
{
    void *pll_clk = NULL;
    struct device_node *dev_node = NULL;

    dev_node = of_find_compatible_node(NULL, NULL, "sstar,dla");
    if (!dev_node) {
        CamOsPrintf("fail to find dla device tree node\n");
        return -1;
    }

    pll_clk = of_clk_get(dev_node, index);
    if (IS_ERR(pll_clk))
    {
        //CamOsPrintf("fail to find ipu pll\n");
        return -1;
    }

    *ppPll = pll_clk;
    return 0;
}

static MI_IPU_Common_Callback_t stCommonCallback;
int MI_IPU_Scaling_Freq(unsigned int freq_mhz)
{
    return stCommonCallback.scaling_freq(freq_mhz);
}
EXPORT_SYMBOL(MI_IPU_Scaling_Freq);

#if SUPPORT_INVOKE_BY_KERNEL == 1
#include "mi_ipu_datatype.h"
MI_S32 MI_IPU_Invoke_ByKernel(MI_U32 u32ChnId, MI_IPU_BatchInvokeParam_t *pstInvokeParam, MI_IPU_RuntimeInfo_t *pstRuntimeInfo);
EXPORT_SYMBOL(MI_IPU_Invoke_ByKernel);
#endif

int MI_IPU_LinuxInit(MI_IPU_Common_Callback_t *pstCommonCallback)
{
    if (pstCommonCallback) {
        stCommonCallback = *pstCommonCallback;
    }

    return 0;
}

void MI_IPU_Iounmap_Wrapper(void *addr)
{
    iounmap(addr);
}

DECLEAR_MODULE_INIT_EXIT
MI_MODULE_LICENSE();
MODULE_AUTHOR("Sigmastar");

