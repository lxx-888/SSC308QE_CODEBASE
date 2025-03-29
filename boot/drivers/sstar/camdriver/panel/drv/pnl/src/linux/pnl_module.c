/*
 * pnl_module.c- Sigmastar
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

#define _PNL_MODULE_C_

#include <linux/cdev.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/of_address.h>
#include "cam_os_wrapper.h"
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

//-------------------------------------------------------------------------------------------------
// Structure
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// Variable
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// internal function
//-------------------------------------------------------------------------------------------------

//==============================================================================
void DrvPnlModuleInitial(void)
{
    int s32Ret;

    if (g_tPnlDevice.refCnt == 0)
    {
        g_tPnlDevice.refCnt++;

        s32Ret = alloc_chrdev_region(&g_tPnlDevice.tDevNumber, 0, 1, DRV_PNL_DEVICE_NAME);

        if (!g_tPnlClass)
        {
            g_tPnlClass = msys_get_sysfs_class();
            if (!g_tPnlClass)
            {
                g_tPnlClass = CamClassCreate(THIS_MODULE, g_pnlClassName);
            }
        }
        else
        {
            cdev_init(&g_tPnlDevice.cdev, &g_tPnlDevice.fops);
            if (0 != (s32Ret = cdev_add(&g_tPnlDevice.cdev, g_tPnlDevice.tDevNumber, DRV_PNL_DEVICE_COUNT)))
            {
                PNL_ERR("%s %d Unable add a character device\n", __FUNCTION__, __LINE__);
            }
        }

        if ((g_tPnlDevice.devicenode == NULL) && g_tPnlClass)
        {
            g_tPnlDevice.devicenode =
                CamDeviceCreate(g_tPnlClass, NULL, g_tPnlDevice.tDevNumber, NULL, DRV_PNL_DEVICE_NAME);
            DrvPnlSysfsInit(g_tPnlDevice.devicenode);
        }

        if (g_stDrvPnlPlatformDevice.dev.of_node == NULL)
        {
            g_stDrvPnlPlatformDevice.dev.of_node = of_find_compatible_node(NULL, NULL, "sstar,pnl");
        }
        if (g_stDrvPnlPlatformDevice.dev.of_node == NULL)
        {
            PNL_ERR("%s %d:: Get Device mode Fail!!\n", __FUNCTION__, __LINE__);
        }

        DrvPnlOsSetDeviceNode(&g_stDrvPnlPlatformDevice);
    }
    else
    {
        g_tPnlDevice.refCnt++;
    }
}

void DrvPnlModuleDeInitial(void)
{
    if (g_tPnlDevice.refCnt)
    {
        g_tPnlDevice.refCnt--;
    }

    if (g_tPnlDevice.refCnt == 0)
    {
        if (g_tPnlDevice.cdev.count)
        {
            // Remove a cdev from the system
            cdev_del(&g_tPnlDevice.cdev);
        }

        // CamDeviceDestroy(m_pstPnlClass, g_tPnlDevice.tDevNumber);
        unregister_chrdev_region(g_tPnlDevice.tDevNumber, 1);

        g_stDrvPnlPlatformDevice.dev.of_node = NULL;
        g_tPnlClass                          = NULL;
    }
}

//-------------------------------------------------------------------------------------------------
// Module functions
//-------------------------------------------------------------------------------------------------
