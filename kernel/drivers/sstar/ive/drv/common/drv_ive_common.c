/*
 * drv_ive_common.c- Sigmastar
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

//-------------------------------------------------------------------------------------------------
// File operations
//-------------------------------------------------------------------------------------------------

/*******************************************************************************************************************
 * drv_ive_drv_isr
 *   handle triggered interrupt and arrange the corresponding process.
 *
 * Parameters:
 *   irq:  IRQ
 *   data: Device data which is assigned from CamOsIrqRequest()
 *
 * Return:
 *   None
 */
void drv_ive_isr(u32 irq, void *data)
{
    ive_dev_data *dev_data = (ive_dev_data *)data;
    IVE_DRV_STATE state;

    state = ive_drv_isr_handler(irq, &dev_data->drv_handle);
    switch (state)
    {
        case IVE_DRV_STATE_DONE:
            drv_ive_isr_pre_proc(dev_data);
            CamOsAtomicSet(&dev_data->drv_handle.dev_state, IVE_DRV_STATE_READY);
            CamOsWorkQueueAdd(dev_data->work_queue, (void *)drv_ive_isr_post_proc, dev_data, 0);
            return;

#if defined(SUPPORT_BACK_PRESSURE)
        case IVE_DRV_STATE_RESET:
            ive_drv_pre_process(&dev_data->drv_handle);
            // fall through
#endif
        default:
            return;
    }

    return;
}

/*******************************************************************************************************************
 * drv_ive_isr_post_proc
 *   remove current task from request list and trigger next task if haved.
 *
 * Parameters:
 *   filp: pointer of device data structure
 *
 */
void drv_ive_isr_post_proc(void *pDevData)
{
    ive_dev_data *dev_data = (ive_dev_data *)pDevData;

    // Enter cirtical section
    CamOsMutexLock(&dev_data->mutex);

    // process next request in list.
    ive_drv_post_process(&dev_data->drv_handle);

    // Leave critical section
    CamOsMutexUnlock(&dev_data->mutex);
}

/*******************************************************************************************************************
 * drv_ive_isr_pre_proc
 *   change current task state and wakeup the corresponding work queue.
 *
 * Parameters:
 *   filp: pointer of device data structure
 *
 */
void drv_ive_isr_pre_proc(ive_dev_data *dev_data)
{
    ive_file_data *file_data;

    // Enter cirtical section
    // CamOsMutexLock(&dev_data->mutex);

    file_data = ive_drv_pre_process(&dev_data->drv_handle);

    // Leave critical section
    // CamOsMutexUnlock(&dev_data->mutex);

    if (file_data == NULL)
    {
        IVE_MSG(IVE_MSG_ERR, "isr post process get NULL of file_data!!\n");
        return;
    }

    IVE_MSG(IVE_MSG_DBG, "file_data:0x%p, pre porcess 0x%p\n", file_data, &file_data->wait_queue);

    // set ready and wake up waiting thread/process
    CamOsConditionWakeUpAll(&file_data->wait_queue);
}

/*******************************************************************************************************************
 * drv_ive_reset_status
 *   reset status
 *
 * Parameters:
 *   filp: pointer of file structure
 *
 */
void drv_ive_reset_status(ive_file_data *file_data)
{
    ive_dev_data *  dev_data    = file_data->dev_data;
    ive_drv_handle *handle      = &(dev_data->drv_handle);
    ive_file_data * pstPrevData = NULL;

    IVE_MSG(IVE_MSG_WRN, "Reset HW status\n");
    CamOsMutexLock(&dev_data->mutex);

    ive_drv_reset(handle);

    // Reset status
    file_data->state       = IVE_FILE_STATE_READY;
    file_data->eTaskResult = E_IVE_TASK_INIT;
    CamOsAtomicSet(&handle->dev_state, IVE_DRV_STATE_READY);

    // destroy the previous task list from the request list
    if (file_data != (pstPrevData = ive_drv_destroy_process(handle)))
    {
        IVE_MSG(IVE_MSG_ERR, "reset status, but get incorrect request\n");
    }

    // start the next task list if it existed
    ive_drv_post_process(handle);

    CamOsMutexUnlock(&dev_data->mutex);
}
