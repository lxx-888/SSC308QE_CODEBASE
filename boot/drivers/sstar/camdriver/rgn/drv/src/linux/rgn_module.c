/*
 * rgn_module.c - Sigmastar
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

#define _RGN_MODULE_C_

#include <linux/cdev.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/of_address.h>
#include <linux/of_irq.h>

#include "cam_os_wrapper.h"
#include "ms_msys.h"
#include "cam_sysfs.h"
#include "rgn_sysfs.h"

#include "mhal_cmdq.h"
#include "mhal_rgn_datatype.h"
#include "drv_rgn_module.h"
#include "rgn_debug.h"
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
void DrvRgnModuleInition(void)
{
    int s32Ret;

    if (g_stRgnDevice.refCnt == 0)
    {
        g_stRgnDevice.refCnt++;

        s32Ret = alloc_chrdev_region(&g_stRgnDevice.tDevNumber, 0, 1, DRV_RGN_DEVICE_NAME);

        if (!g_pstRgnClass)
        {
            g_pstRgnClass = msys_get_sysfs_class();
            if (!g_pstRgnClass)
            {
                g_pstRgnClass = CamClassCreate(THIS_MODULE, g_pu8RgnClassName);
            }
        }
        else
        {
            cdev_init(&g_stRgnDevice.cdev, &g_stRgnDevice.fops);
            if (0 != (s32Ret = cdev_add(&g_stRgnDevice.cdev, g_stRgnDevice.tDevNumber, DRV_RGN_DEVICE_COUNT)))
            {
                RGN_ERR("[RGN] Unable add a character device\n");
            }
        }

        if (g_stRgnDevice.devicenode == NULL && g_pstRgnClass)
        {
            g_stRgnDevice.devicenode = CamDeviceCreate(g_pstRgnClass, NULL, g_stRgnDevice.tDevNumber, NULL, "mrgn");
            DrvRgnSysfsInit(g_stRgnDevice.devicenode);
        }

        if (g_stDrvRgnPlatformDevice.dev.of_node == NULL)
        {
            g_stDrvRgnPlatformDevice.dev.of_node = of_find_compatible_node(NULL, NULL, "sstar,rgn");
        }
        if (g_stDrvRgnPlatformDevice.dev.of_node == NULL)
        {
            RGN_ERR("[RGN INIT] Get Device mode Fail!!\n");
        }

        // Get IRQ
    }
}

void DrvRgnModuleDeInition(void)
{
    if (g_stRgnDevice.refCnt)
    {
        g_stRgnDevice.refCnt--;
    }

    if (g_stRgnDevice.refCnt == 0)
    {
        if (g_stRgnDevice.cdev.count)
        {
            // Remove a cdev from the system
            cdev_del(&g_stRgnDevice.cdev);
        }

        // CamDeviceDestroy(m_pstRgnClass, g_stRgnDevice.tDevNumber);
        unregister_chrdev_region(g_stRgnDevice.tDevNumber, 1);

        g_stDrvRgnPlatformDevice.dev.of_node = NULL;
        g_pstRgnClass                        = NULL;
    }
}

//-------------------------------------------------------------------------------------------------
// Module functions
//-------------------------------------------------------------------------------------------------
