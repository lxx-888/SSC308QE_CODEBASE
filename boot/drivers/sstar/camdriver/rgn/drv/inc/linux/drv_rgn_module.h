/*
 * drv_rgn_module.h - Sigmastar
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

#ifndef _DRV_RGN_MODULE_H_
#define _DRV_RGN_MODULE_H_

//-------------------------------------------------------------------------------------------------
// Define & Macro
//-------------------------------------------------------------------------------------------------
#define DRV_RGN_DEVICE_COUNT 1
#define DRV_RGN_DEVICE_NAME  "mrgn"

//-------------------------------------------------------------------------------------------------
// Structure
//-------------------------------------------------------------------------------------------------
typedef struct
{
    dev_t                  tDevNumber;
    int                    refCnt;
    struct cdev            cdev;
    struct file_operations fops;
    struct device *        devicenode;
} DrvRgnModuleDevice_t;

//-------------------------------------------------------------------------------------------------
// Varialbe
//-------------------------------------------------------------------------------------------------
#ifdef _RGN_MODULE_C_
DrvRgnModuleDevice_t g_stRgnDevice = {
    .tDevNumber = 0,
    .refCnt     = 0,
    .devicenode = NULL,
    .cdev =
        {
            .kobj =
                {
                    .name = DRV_RGN_DEVICE_NAME,
                },
            .owner = THIS_MODULE,
        },
    /*
    .fops =
    {
        .open = DrvRgnModuleOpen,
        .release = DrvRgnModuleRelease,
        .unlocked_ioctl = DrvRgnModuleIoctl,
        .poll = DrvRgnModulePoll,
    },*/
};

struct class *g_pstRgnClass     = NULL;
char *        g_pu8RgnClassName = "m_rgn_class";

u64 u64Rgn_DmaMask = 0xffffffffUL;

struct platform_device g_stDrvRgnPlatformDevice = {
    .name = DRV_RGN_DEVICE_NAME,
    .id   = 0,
    .dev  = {.of_node = NULL, .dma_mask = &u64Rgn_DmaMask, .coherent_dma_mask = 0xffffffffUL}};

#endif

//-------------------------------------------------------------------------------------------------
// Prototype
//-------------------------------------------------------------------------------------------------

#ifdef _DRV_RGN_MODULE_C_
#define INTERFACE
#else
#define INTERFACE extern
#endif

#undef INTERFACE

#endif
