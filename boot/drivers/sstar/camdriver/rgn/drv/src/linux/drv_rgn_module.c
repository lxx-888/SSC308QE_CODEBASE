/*
 * drv_rgn_module.c - Sigmastar
 *
 * Copyright (c) [2019~2022] SigmaStar Technology.
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

#define _DRV_RGN_MODULE_C_

#include <linux/cdev.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/of_address.h>
#include <linux/of_irq.h>

#include "ms_msys.h"
#include "drv_rgn_os.h"
#include "cam_sysfs.h"
#include "rgn_sysfs.h"
#include "rgn_debug.h"

#include "mhal_cmdq.h"
#include "mhal_rgn_datatype.h"
#include "hal_rgn_chip.h"
#include "hal_rgn_st.h"
#include "drv_rgn_ctx.h"
#include "drv_rgn_module.h"
//-------------------------------------------------------------------------------------------------
// Define & Macro
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// Prototype
//-------------------------------------------------------------------------------------------------
static int _DrvRgnModuleProbe(struct platform_device *pdev);
static int _DrvRgnModuleRemove(struct platform_device *pdev);
static int _DrvRgnModuleSuspend(struct platform_device *dev, pm_message_t state);
static int _DrvRgnModuleResume(struct platform_device *dev);

extern void DrvRgnModuleInition(void);
extern void DrvRgnModuleDeInition(void);
//-------------------------------------------------------------------------------------------------
// Structure
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// Variable
//-------------------------------------------------------------------------------------------------
extern DrvRgnModuleDevice_t   g_stRgnDevice;
extern struct class *         g_pstRgnClass;
extern char *                 g_pu8RgnClassName;
extern struct platform_device g_stDrvRgnPlatformDevice;

struct of_device_id g_stRgnMatchTable[] = {{.compatible = "sstar,rgn"}, {}};

struct platform_driver g_stDrvRgnPlatformDriver = {
    .probe   = _DrvRgnModuleProbe,
    .remove  = _DrvRgnModuleRemove,
    .suspend = _DrvRgnModuleSuspend,
    .resume  = _DrvRgnModuleResume,
    .driver =
        {
            .name           = DRV_RGN_DEVICE_NAME,
            .owner          = THIS_MODULE,
            .of_match_table = of_match_ptr(g_stRgnMatchTable),
        },
};

//-------------------------------------------------------------------------------------------------
// internal function
//-------------------------------------------------------------------------------------------------

//==============================================================================
static int _DrvRgnModuleSuspend(struct platform_device *dev, pm_message_t state)
{
    int ret = 0;
    return ret;
}

static int _DrvRgnModuleResume(struct platform_device *dev)
{
    int ret = 0;

    return ret;
}

//-------------------------------------------------------------------------------------------------
// Module functions
//-------------------------------------------------------------------------------------------------
static int _DrvRgnModuleProbe(struct platform_device *pdev)
{
    g_pstRgnClass = msys_get_sysfs_class();

    if (IS_ERR(g_pstRgnClass))
    {
        RGN_ERR("%s %d class_create() fail. Please exec [mknod] before operate the device\n", __FUNCTION__, __LINE__);
    }

    g_stDrvRgnPlatformDevice.dev.of_node = pdev->dev.of_node;

    // create device
    DrvRgnModuleInition();

    return 0;
}

static int _DrvRgnModuleRemove(struct platform_device *pdev)
{
    DrvRgnModuleDeInition();
    CamDeviceUnregister(g_stRgnDevice.devicenode);
    return 0;
}

int DrvRgnModuleInit(void)
{
    int ret = 0;

    ret = CamPlatformDriverRegister(&g_stDrvRgnPlatformDriver);
    if (ret)
    {
        RGN_ERR("%s %d CamPlatformDriverRegister failed\n", __FUNCTION__, __LINE__);
        CamPlatformDriverUnregister(&g_stDrvRgnPlatformDriver);
    }

    return ret;
}
void DrvRgnModuleExit(void)
{
    /*de-initial the who GFLIPDriver */
    CamPlatformDriverUnregister(&g_stDrvRgnPlatformDriver);
}

#if ((defined ALKAID) && (ALKAID == 1))
#else
module_init(DrvRgnModuleInit);
module_exit(DrvRgnModuleExit);

MODULE_AUTHOR("CAMDRIVER");
MODULE_DESCRIPTION("camdriver rgn ioctrl driver");
#ifdef SUPPORT_GPL_SYMBOL
MODULE_LICENSE("GPL");
#else
MODULE_LICENSE("PROPRIETARY");
#endif
#endif
