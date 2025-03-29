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
#include <asm/neon.h>
#include <linux/module.h>

#include "mi_common_datatype.h"

#include <linux/moduleparam.h>
#include <linux/stat.h>
#include <linux/sched.h>
#include <linux/of_platform.h>
#include "cam_sysfs.h"
#include "cam_clkgen.h"
#include "cam_device_wrapper.h"


/////////////// cmdq ////////////////
// FIXME: use interface to get answer
#define NUMBER_OF_CMDQ_HW 12
int DrvCmdqGetIrqNum(int nDrvId)
{
    struct device_node *devNode;
    struct platform_device *pdev;
    char compatibleName[16];
    int irq = 0;
    if(nDrvId>=NUMBER_OF_CMDQ_HW )
    {
        pr_info("[CMDQ] Error Id: %d\n", nDrvId);
        return -1;
    }
    sprintf(compatibleName, "sstar,cmdq%d", nDrvId); //"sstar,cmdq0"
    devNode = of_find_compatible_node(NULL, NULL, compatibleName);

    //pr_err("%s , %s\n" , __func__ , compatibleName);

    if (!devNode) {
        if (!devNode)
        {
            //pr_err("%s , LINE = %d \n" , __func__ , __LINE__ );
            return -1;
        }

    }
            //pr_err("%s , LINE = %d \n" , __func__ , __LINE__ );
    pdev = of_find_device_by_node(devNode);
    if (!pdev) {
        of_node_put(devNode);
           // pr_err("%s , LINE = %d \n" , __func__ , __LINE__ );
        return -1;
    }
    irq = CamIrqOfParseAndMap(pdev->dev.of_node, 0);
    pr_info("[CMDQ%d] Virtual IRQ: %d\n", nDrvId, irq);
    //pr_err("[CMDQ%d] Virtual IRQ: %d\n", nDrvId, irq);
    //pr_err("%s , LINE = %d \n" , __func__ , __LINE__ );
    return irq;
}

/* return Mhz unit*/
int DrvCmdqGetClk(int nDrvId)
{
    int rate = 0;
    struct device_node *devNode;
    char compatibleName[16];
    struct platform_device *pdev;

    if(nDrvId>=NUMBER_OF_CMDQ_HW )
    {
        pr_info("[CMDQ] Error Id: %d\n", nDrvId);
        return -1;
    }
    sprintf(compatibleName, "sstar,cmdq%d", nDrvId); //"sstar,cmdq0"
    devNode = of_find_compatible_node(NULL, NULL, compatibleName);

    if (!devNode) {
        if (!devNode)
        {
            return -1;
        }
    }
    pdev = of_find_device_by_node(devNode);
    if (!pdev) {
        of_node_put(devNode);
        return -1;
    }

    {
#ifdef CONFIG_CAM_CLK
    u32 CmdqClk = 0;
    msys_of_property_read_u32_index(pdev->dev.of_node,"camclk", 0,&(CmdqClk));
    if (!CmdqClk)
    {
        printk(KERN_DEBUG "[%s] Fail to get clk!\n", __func__);
    }
    else
    {
        rate = CamClkRateGet(CmdqClk)/1000000;
    }
#else
    struct clk         *clk;
    clk = of_clk_get(pdev->dev.of_node, 0);
    if(IS_ERR(clk))
    {
        CamOsPrintf("[%s] of_clk_get failed\n", __func__);
        return rate;
    }
    rate = CamClkGetRate(clk)/1000000;
    clk_put(clk);
#endif
    }

    return rate;
}