/*
 * drv_ive_io.c- Sigmastar
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

#include "drv_ive.h"
#include "drv_ive_clk.h"
#include "drv_ive_common.h"
#include "drv_ive_io.h"

extern ive_dev_data *g_pstDevData;

/*******************************************************************************************************************
 * drv_ive_module_open
 *   File open handler
 *   The device can has a instance at the same time, and the open
 *   operator also enable the clock and request ISR.
 *
 * Parameters:
 *   void
 *
 * Return:
 *   file_data pointer which neend in ioctrl, release and poll
 */
void *drv_ive_module_open(void)
{
    ive_file_data *file_data;

    if (!g_pstDevData)
    {
        IVE_MSG(IVE_MSG_WRN, "IVE is not ready, run probe first pls.\n");
        return NULL;
    }

    // allocate buffer
    file_data = CamOsMemCalloc(1, sizeof(ive_file_data));
    if (file_data == NULL)
    {
        IVE_MSG(IVE_MSG_ERR, "error: can't allocate buffer\n");
        return NULL;
    }

    IVE_MSG(IVE_MSG_DBG, "file_data: 0x%p\n", file_data);

    // Assgin dev_data and keep file_data in the file structure
    file_data->state       = IVE_FILE_STATE_READY;
    file_data->eTaskResult = E_IVE_TASK_INIT;
    file_data->dev_data    = g_pstDevData;

    // Init wait queue
    CamOsConditionInit(&file_data->wait_queue);

    return (void *)file_data;
}

/*******************************************************************************************************************
 * drv_ive_module_release
 *   File close handler
 *   The operator will release clock & ISR
 *
 * Parameters:
 *   pfile: file_data pointer
 *
 * Return:
 *   standard return value
 */
int drv_ive_module_release(void *pfile)
{
    ive_file_data *file_data = (ive_file_data *)pfile;

    IVE_MSG(IVE_MSG_DBG, "pfile: 0x%x\n", pfile);

    CamOsConditionDeinit(&file_data->wait_queue);

    // Release memory
    CamOsMemRelease(file_data);

    return 0;
}

/*******************************************************************************************************************
 * drv_ive_module_ioctl_process
 *   IOCTL handler for IVE_IOC_PROCESS
 *
 * Parameters:
 *   file_data: file private data
 *   arg:       argument, a pointer of ive_ioc_config from userspace
 *
 * Return:
 *   IVE_IOC_RET
 */
static IVE_IOC_ERROR drv_ive_module_ioctl_process(ive_file_data *file_data, ive_ioc_config ioc_config)
{
    if (file_data->state != IVE_FILE_STATE_READY)
    {
        IVE_MSG(IVE_MSG_ERR, "One file can request once at the same time only\n");
        return IVE_IOC_ERROR_BUSY;
    }

    file_data->ioc_config = ioc_config;

    return ive_drv_process(&file_data->dev_data->drv_handle, file_data);
}

/*******************************************************************************************************************
 * drv_ive_module_ioctl
 *   IOCTL handler entry for file operator
 *
 * Parameters:
 *   pfile: file_data pointer
 *   ioc_config:  argument from user space
 *
 * Return:
 *   standard return value
 */
long drv_ive_module_ioctl(void *pfile, ive_ioc_config ioc_config)
{
    ive_file_data *file_data = (ive_file_data *)pfile;
    IVE_IOC_ERROR  err       = IVE_IOC_ERROR_NONE;

    IVE_MSG(IVE_MSG_DBG, "pfile: 0x%x\n", (uint32_t)pfile);

    err = drv_ive_module_ioctl_process(file_data, ioc_config);

    return err;
}

/*******************************************************************************************************************
 * drv_ive_module_poll
 *   poll handler entry for file operator
 *
 * Parameters:
 *   file_data: file private data
 *
 * Return:
 *   0: success
 *   other: fail
 */
int drv_ive_module_poll(void *pfile, int mSec)
{
    ive_file_data *file_data = (ive_file_data *)pfile;
    int            err       = 0;

    IVE_MSG(IVE_MSG_DBG, "polling 0x%p 0x%X\n", &file_data->wait_queue, file_data->state);

    if (file_data->state == IVE_FILE_STATE_READY)
    {
        return 0;
    }

    err = CamOsConditionTimedWait(&file_data->wait_queue, file_data->state == IVE_FILE_STATE_DONE, mSec);

    switch (file_data->state)
    {
        case IVE_FILE_STATE_DONE:
            file_data->state = IVE_FILE_STATE_READY;
            return 0;
        default:
            if (file_data->state == IVE_FILE_STATE_IN_QUEUE)
            {
                IVE_MSG(IVE_MSG_ERR, "Current request timeout without shedule\n");
            }
            else
            {
#if defined(SUPPORT_BACK_PRESSURE)
                if (file_data->eTaskResult == E_IVE_TASK_FAILED
                    && CamOsAtomicRead(&file_data->dev_data->drv_handle.dev_state) == IVE_DRV_STATE_RESET)
                {
                    IVE_MSG(IVE_MSG_ERR, "bgblur back pressure, task info: type=%d, w/h=%d/%d\n",
                            file_data->ioc_config.op_type, file_data->ioc_config.input.width,
                            file_data->ioc_config.input.height);
                }
                else
#endif
                {
                    IVE_MSG(IVE_MSG_ERR, "Polling fail, task info: type=%d, w/h=%d/%d\n", file_data->ioc_config.op_type,
                            file_data->ioc_config.input.width, file_data->ioc_config.input.height);
                }
            }
            drv_ive_reset_status(file_data);
            break;
    }

    if (err)
        IVE_MSG(IVE_MSG_ERR, "CamOsConditionTimedWait timeout, err:%d.\n", err);

    return -1;
}
