/*
 * drv_pnl_module.c- Sigmastar
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

#define _DRV_PNL_MODULE_C_

#include "cam_sysfs.h"
#include "drv_pnl_os.h"
#include "ms_msys.h"
#include "pnl_sysfs.h"
#include "pnl_debug.h"
#include "drv_pnl_module.h"
//-------------------------------------------------------------------------------------------------
// Define & Macro
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// Prototype
//-------------------------------------------------------------------------------------------------
static int _DrvPnlModuleProbe(struct platform_device *pdev);
static int _DrvPnlModuleRemove(struct platform_device *pdev);
static int _DrvPnlModuleSuspend(struct platform_device *dev, pm_message_t state);
static int _DrvPnlModuleResume(struct platform_device *dev);

extern void DrvPnlModuleInitial(void);
extern void DrvPnlModuleDeInitial(void);

//-------------------------------------------------------------------------------------------------
// Structure
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// Variable
//-------------------------------------------------------------------------------------------------
extern DrvPnlModuleDevice_t   g_tPnlDevice;
extern struct class *         g_tPnlClass;
extern char *                 g_pnlClassName;
extern struct platform_device g_stDrvPnlPlatformDevice;

struct of_device_id g_stPnlMatchTable[] = {{.compatible = "sstar,pnl"}, {}};

struct platform_driver g_stDrvPnlPlatformDriver = {
    .probe   = _DrvPnlModuleProbe,
    .remove  = _DrvPnlModuleRemove,
    .suspend = _DrvPnlModuleSuspend,
    .resume  = _DrvPnlModuleResume,
    .driver =
        {
            .name           = DRV_PNL_DEVICE_NAME,
            .owner          = THIS_MODULE,
            .of_match_table = of_match_ptr(g_stPnlMatchTable),
        },
};

//-------------------------------------------------------------------------------------------------
// internal function
//-------------------------------------------------------------------------------------------------

//==============================================================================
static int _DrvPnlModuleSuspend(struct platform_device *dev, pm_message_t state)
{
    int ret = 0;
    return ret;
}

static int _DrvPnlModuleResume(struct platform_device *dev)
{
    int ret = 0;
    PNL_DBG(PNL_DBG_LEVEL_MODULE, "[PNL] %s\n", __FUNCTION__);
    return ret;
}

//-------------------------------------------------------------------------------------------------
// Module functions
//-------------------------------------------------------------------------------------------------
static int _DrvPnlModuleProbe(struct platform_device *pdev)
{
    g_tPnlClass = msys_get_sysfs_class();

    if (IS_ERR(g_tPnlClass))
    {
        PNL_ERR("%s %d class_create() fail. Please exec [mknod] before operate the device\n", __FUNCTION__, __LINE__);
    }

    g_stDrvPnlPlatformDevice.dev.of_node = pdev->dev.of_node;

    // create device
    DrvPnlModuleInitial();

    return 0;
}

static int _DrvPnlModuleRemove(struct platform_device *pdev)
{
    PNL_DBG(PNL_DBG_LEVEL_MODULE, "[PNL] %s\n", __FUNCTION__);

    DrvPnlModuleDeInitial();
    CamDeviceUnregister(g_tPnlDevice.devicenode);
    return 0;
}

int DrvPnlModuleInit(void)
{
    int ret = 0;

    PNL_DBG(PNL_DBG_LEVEL_MODULE, "%s %d\n", __FUNCTION__, __LINE__);

    ret = CamPlatformDriverRegister(&g_stDrvPnlPlatformDriver);
    if (!ret)
    {
        PNL_DBG(PNL_DBG_LEVEL_MODULE, "%s %d CamPlatformDriverRegister success\n", __FUNCTION__, __LINE__);
    }
    else
    {
        PNL_ERR("%s %dCamPlatformDriverRegister failed\n", __FUNCTION__, __LINE__);
        CamPlatformDriverUnregister(&g_stDrvPnlPlatformDriver);
    }

    return ret;
}
void DrvPnlModuleExit(void)
{
    /*de-initial the who GFLIPDriver */
    PNL_DBG(PNL_DBG_LEVEL_MODULE, "%s %d\n", __FUNCTION__, __LINE__);
    CamPlatformDriverUnregister(&g_stDrvPnlPlatformDriver);
}

module_init(DrvPnlModuleInit);
module_exit(DrvPnlModuleExit);

MODULE_AUTHOR("CAMDRIVER");
MODULE_DESCRIPTION("camdriver panel ioctrl driver");
#ifdef SUPPORT_GPL_SYMBOL
MODULE_LICENSE("GPL");
#else
MODULE_LICENSE("PROPRIETARY");
#endif
