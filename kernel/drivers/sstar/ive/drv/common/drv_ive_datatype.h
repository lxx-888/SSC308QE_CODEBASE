/*
 * drv_ive_datatype.h- Sigmastar
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
#ifndef _IVE_DRV_DATATYPE_H_
#define _IVE_DRV_DATATYPE_H_

//---------------------------------------------------------------------------
// INCLUDE
//---------------------------------------------------------------------------
#if !defined(CAM_OS_RTK)
#include <linux/platform_device.h>
#include <linux/cdev.h>
#endif
#include "drv_ive_io_st.h"
#include "hal_ive.h"
#include "cam_os_util_list.h"
#include "cam_device_wrapper.h"

//---------------------------------------------------------------------------
// STRUCT
//---------------------------------------------------------------------------
typedef enum
{
    IVE_DRV_STATE_READY      = 0,
    IVE_DRV_STATE_PROCESSING = 1,
    IVE_DRV_STATE_DONE       = 2,
#if defined(SUPPORT_BACK_PRESSURE)
    IVE_DRV_STATE_RESET = 3,
#endif
} IVE_DRV_STATE;

typedef enum
{
    IVE_FILE_STATE_READY      = 0,
    IVE_FILE_STATE_PROCESSING = 1,
    IVE_FILE_STATE_DONE       = 2,
    IVE_FILE_STATE_IN_QUEUE   = 3,
} IVE_FILE_STATE;

typedef enum
{
    E_IVE_TASK_INIT    = 0,
    E_IVE_TASK_SUCCESS = 1,
    E_IVE_TASK_FAILED  = 2,
} E_IVE_TASK_RESULT;

typedef struct
{
    struct platform_device *pdev;       // platform device data
    ive_hal_handle          hal_handle; // HAL handle for real HW configuration
    CamOsAtomic_t           dev_state;  // HW state
#if defined(SUPPORT_REQUEST_PRIORITY)
    IVE_IOC_TASK_PRIORITY_E eDrvPriority;
    struct CamOsListHead_t  request_list[IVE_IOC_TASK_PRIORITY_MAX];
#else
    struct CamOsListHead_t request_list; // request list to queue waiting requst
#endif
} ive_drv_handle;

typedef struct
{
    struct platform_device *pdev; // platform device data
#if !defined(CAM_OS_RTK)
    struct CamDevice *pstDev; // kernel device data
    struct cdev       cdev;   // character device
    struct clk *      pstClk; // clock
#else
    unsigned int           cdev;         // character device
#if defined(CONFIG_CAM_CLK)
    void *                 pvclk;
#endif
#endif
    CamOsAtomic_t  bClkEnable; // clock enable flag.
    u32            u32ClkRate; // clock rate.
    unsigned int   irq;        // IRQ number
    ive_drv_handle drv_handle; // device handle
    CamOsWorkQueue work_queue; // work queue for post process after ISR
    CamOsMutex_t   mutex;      // for critical section
} ive_dev_data;

typedef struct
{
    ive_dev_data *    dev_data;   // Device data
    ive_ioc_config    ioc_config; // IO configuation, i.e. one device file can service one request at the same time
    IVE_FILE_STATE    state;      // File state
    E_IVE_TASK_RESULT eTaskResult;
    CamOsCondition_t  wait_queue; // Wait queue for polling operation
} ive_file_data;

typedef struct
{
    struct CamOsListHead_t list;
    ive_file_data *        file_data;
} ive_request_data;
#endif
