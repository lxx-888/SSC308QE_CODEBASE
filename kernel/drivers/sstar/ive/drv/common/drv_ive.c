/*
 * drv_ive.c- Sigmastar
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

// [ 7: 0]: read  burst length.
// [15: 8]: write burst length.
// [23:16]: read  outstanding.
// [31:24]: write outstanding.
u32 g_u32BurstOutstanding = 0;

/*******************************************************************************************************************
 * drv_ive_extract_request
 *   Extract a request from waiting list
 *   The request is removed from list
 *
 * Parameters:
 *   ive_drv_handle: driver handle
 *
 * Return:
 *   File data of IVE driver
 */
static ive_file_data *drv_ive_extract_request(ive_drv_handle *handle)
{
    struct CamOsListHead_t *pstRequestList = NULL;
    ive_request_data *      list_data      = NULL;
    ive_file_data *         file_data      = NULL;

#if defined(SUPPORT_REQUEST_PRIORITY)
    if (IVE_IOC_TASK_PRIORITY_MAX == handle->eDrvPriority)
    {
        // do nothing cause the finished task has been removed.
        return NULL;
    }
    else
    {
        // the task is been doing or has been done.
        pstRequestList = &handle->request_list[handle->eDrvPriority];
    }
#else
    pstRequestList = &handle->request_list;
#endif

    if (CAM_OS_LIST_EMPTY_CAREFUL(pstRequestList))
    {
        IVE_MSG(IVE_MSG_DBG, "try to extract request but request list is empty\n");
        return NULL;
    }

    list_data = CAM_OS_LIST_FIRST_ENTRY(pstRequestList, ive_request_data, list);
    if (NULL != list_data)
    {
        // remove request if done.
        if (list_data->file_data->state == IVE_FILE_STATE_READY || list_data->file_data->state == IVE_FILE_STATE_DONE)
        {
            CAM_OS_LIST_DEL(pstRequestList->pNext);
            file_data = list_data->file_data;
            IVE_MSG(IVE_MSG_DBG, "extract: 0x%p, 0x%p\n", list_data, file_data);
            CamOsMemRelease(list_data);
#if defined(SUPPORT_REQUEST_PRIORITY)
            handle->eDrvPriority = IVE_IOC_TASK_PRIORITY_MAX;
#endif
        }
    }
    else
    {
        IVE_MSG(IVE_MSG_DBG, "no task in list, extract: 0x%p\n", list_data);
    }

    return file_data;
}

/*******************************************************************************************************************
 * drv_ive_get_request
 *   Get a request from waiting list
 *   The request is still kpet in list
 *
 * Parameters:
 *   ive_drv_handle: driver handle
 *
 * Return:
 *   File data of IVE driver
 */
ive_file_data *drv_ive_get_request(ive_drv_handle *handle)
{
    struct CamOsListHead_t *pstRequestList = NULL;
    ive_request_data *      list_data      = NULL;

#if defined(SUPPORT_REQUEST_PRIORITY)
    if (IVE_IOC_TASK_PRIORITY_MAX == handle->eDrvPriority)
    {
        int i = IVE_IOC_TASK_PRIORITY_MAX - 1;
        for (; i >= IVE_IOC_TASK_PRIORITY_DEFAULT; --i)
        {
            if (false == CAM_OS_LIST_EMPTY_CAREFUL(&handle->request_list[i]))
            {
                pstRequestList = &handle->request_list[i];
                break;
            }
        }
    }
    else
    {
        pstRequestList = &handle->request_list[handle->eDrvPriority];
    }
#else
    pstRequestList = &handle->request_list;
#endif
    if (NULL == pstRequestList || CAM_OS_LIST_EMPTY_CAREFUL(pstRequestList))
    {
        return NULL;
    }

    if (NULL != (list_data = CAM_OS_LIST_FIRST_ENTRY(pstRequestList, ive_request_data, list)))
    {
        IVE_MSG(IVE_MSG_DBG, "get: 0x%p, 0x%p\n", list_data, list_data->file_data);
        return list_data->file_data;
    }
    else
    {
        IVE_MSG(IVE_MSG_DBG, "get: 0x%p\n", list_data);
    }

    return NULL;
}

/*******************************************************************************************************************
 * drv_ive_add_request
 *   Add a request to waiting list
 *
 * Parameters:
 *   ive_drv_handle: driver handle
 *   file_data: File data of IVE driver
 *
 * Return:
 *   none
 */
static int drv_ive_add_request(ive_drv_handle *handle, ive_file_data *file_data)
{
    ive_request_data *list_data = NULL;

    if (NULL == (list_data = CamOsMemCalloc(1, sizeof(ive_request_data))))
    {
        IVE_MSG(IVE_MSG_ERR, "%s:can't create list data\n", __FUNCTION__);
        return -1;
    }
    list_data->file_data = file_data;

    IVE_MSG(IVE_MSG_DBG, "add: 0x%p, 0x%p\n", list_data, file_data);

#if defined(SUPPORT_REQUEST_PRIORITY)
    CAM_OS_LIST_ADD_TAIL(&list_data->list, &handle->request_list[file_data->ioc_config.eTaskPriority]);
#else
    CAM_OS_LIST_ADD_TAIL(&list_data->list, &handle->request_list);
#endif
    return 0;
}

/*******************************************************************************************************************
 * drv_ive_check_config
 *   Check the configuration
 *
 * Parameters:
 *   config: IO configuration
 *
 * Return:
 *   standard return enum
 */
static IVE_IOC_ERROR drv_ive_check_config(ive_ioc_config *config)
{
    IVE_HAL_RET_E         ret        = IVE_HAL_SUCCESS;
    IVE_CONFIG_WH_CHECK_E eCheckFlag = IVE_CONFIG_WH_CHECK_NONE;

    u16 u16MaxWidth = 0, u16MaxHeight = 0;
    u16 u16MinWidth = 0, u16MinHeight = 0;

    ret = ive_hal_get_check_params(config->op_type, &eCheckFlag, &u16MaxWidth, &u16MaxHeight, &u16MinWidth,
                                   &u16MinHeight);
    if (IVE_HAL_SUCCESS != ret)
    {
        return IVE_IOC_ERROR_PROC_CONFIG;
    }

    if (eCheckFlag & IVE_CONFIG_WH_CHECK_MATCH)
    {
        RETURN_IF_CHECK_WH_MATCH(config->op_type, config->input.width, config->input.height, config->output.width,
                                 config->output.height);
    }

    if (eCheckFlag & IVE_CONFIG_WH_CHECK_INPUT)
    {
        RETURN_IF_CHECK_MAX_AND_MIN_WH(config->op_type, config->input.width, config->input.height, u16MaxWidth,
                                       u16MaxHeight, u16MinWidth, u16MinHeight);
    }

    if (eCheckFlag & IVE_CONFIG_WH_CHECK_OUTPUT)
    {
        RETURN_IF_CHECK_MAX_AND_MIN_WH(config->op_type, config->output.width, config->output.height, u16MaxWidth,
                                       u16MaxHeight, u16MinWidth, u16MinHeight);
    }

    return IVE_IOC_ERROR_NONE;
}

/*******************************************************************************************************************
 * ive_drv_sync_image
 *   Sync image
 *
 * Parameters:
 *   handle: device handle
 *   image: image
 *   direct: DMA directon
 *
 * Return:
 *   None
 */

/*******************************************************************************************************************
 * ive_drv_set_images
 *   Set & sync input/output images
 *
 * Parameters:
 *   handle: device handle
 *   config: configuration
 *
 * Return:
 *   None
 */
static void ive_drv_set_images(ive_drv_handle *handle, ive_ioc_config *config)
{
    ive_hal_set_images(&handle->hal_handle, config);
}

/*******************************************************************************************************************
 * ive_drv_isr_handler
 *   ISR handler
 *
 * Parameters:
 *   irq:    interrupt number
 *   handle: device handle
 *
 * Return:
 *   None
 */
IVE_DRV_STATE ive_drv_isr_handler(int irq, ive_drv_handle *handle)
{
    do
    {
#if defined(SUPPORT_BGBLUR)
        {
            IVE_HAL_BGB_IRQ_MASK eRetBgbIrqMask = 0;
            IVE_MSG(IVE_MSG_DBG, "BGBlur Interrupt: 0x%X\n", ive_hal_get_bgblur_irq(&handle->hal_handle));
            if ((eRetBgbIrqMask = ive_hal_get_bgblur_irq_check(&handle->hal_handle, IVE_BGB_ISR_IRQ_CHECK_MASK)))
            {
                // Clear IRQ
                ive_hal_clear_bgblur_irq(&handle->hal_handle, eRetBgbIrqMask);

#if defined(SUPPORT_BACK_PRESSURE)
                if (IVE_HAL_BGB_IRQ_MASK_AXI_RRDY_ABORT == eRetBgbIrqMask)
                {
                    CamOsAtomicSet(&handle->dev_state, IVE_DRV_STATE_RESET);
                }
                else
#endif
                {
                    CamOsAtomicSet(&handle->dev_state, IVE_DRV_STATE_DONE);
                }
                break;
            }
        }
#endif
        {
            IVE_HAL_IRQ_MASK eRetIrqMask = 0;
            IVE_MSG(IVE_MSG_DBG, "Interrupt: 0x%X\n", ive_hal_get_irq(&handle->hal_handle));
            if ((eRetIrqMask = ive_hal_get_irq_check(&handle->hal_handle, IVE_ISR_IRQ_CHECK_MASK)))
            {
                // Clear IRQ
                ive_hal_clear_irq(&handle->hal_handle, eRetIrqMask);

                CamOsAtomicSet(&handle->dev_state, IVE_DRV_STATE_DONE);
                break;
            }
        }
    } while (0);

    return CamOsAtomicRead(&handle->dev_state);
}

/*******************************************************************************************************************
 * ive_drv_init
 *   Init IVEG settings
 *
 * Parameters:
 *   handle: device handle
 *   pdev:          platform device
 *   base_addr0:    base addr of HW register bank 0
 *   base_addr1:    base addr of HW register bank 1
 *
 * Return:
 *   none
 */
int ive_drv_init(ive_drv_handle *handle, struct platform_device *pdev, ss_phys_addr_t *u64BankBase)
{
    memset(handle, 0, sizeof(ive_drv_handle));

    ive_hal_init(&handle->hal_handle, u64BankBase);
#if defined(SUPPORT_REQUEST_PRIORITY)
    handle->eDrvPriority = IVE_IOC_TASK_PRIORITY_MAX;
    {
        int                     i              = 0;
        struct CamOsListHead_t *pstRequestList = NULL;
        for (i = IVE_IOC_TASK_PRIORITY_DEFAULT; i < IVE_IOC_TASK_PRIORITY_MAX; ++i)
        {
            pstRequestList = &handle->request_list[i];
            CAM_OS_INIT_LIST_HEAD(pstRequestList);
            IVE_MSG(IVE_MSG_DBG, "list_empty = %d\n", CAM_OS_LIST_EMPTY_CAREFUL(pstRequestList));
        }
    }
#else
    CAM_OS_INIT_LIST_HEAD(&handle->request_list);
    IVE_MSG(IVE_MSG_DBG, "list_empty = %d\n", CAM_OS_LIST_EMPTY_CAREFUL(&handle->request_list));
#endif

    handle->pdev = pdev;
    CamOsAtomicSet(&handle->dev_state, IVE_DRV_STATE_READY);

    return 0;
}

/*******************************************************************************************************************
 * ive_drv_release
 *   Release IVEG settings
 *
 * Parameters:
 *   handle: device handle
 *
 * Return:
 *   none
 */
void ive_drv_release(ive_drv_handle *handle) {}

/*******************************************************************************************************************
 * ive_drv_start
 *   Start HW engine
 *
 * Parameters:
 *   handle: device handle
 *   config: IVE configurations
 *
 * Return:
 *   none
 */
static void ive_drv_start(ive_drv_handle *handle, ive_ioc_config *config)
{
    IVE_MSG(IVE_MSG_DBG, "op_type: 0x%X\n", config->op_type);

    ive_hal_sw_reset(&handle->hal_handle);

    ive_hal_set_operation(&handle->hal_handle, config->op_type);

    ive_drv_set_images(handle, config);

    ive_hal_set_burst(&handle->hal_handle);

    ive_hal_set_op_params(&handle->hal_handle, config);

    ive_hal_clear_irq(&handle->hal_handle, IVE_HAL_IRQ_MASK_ALL);
    ive_hal_set_irq_mask(&handle->hal_handle, IVE_ISR_IRQ_CHECK_MASK);

    // Using replicate padding mode
    ive_hal_padding_mode_set(&handle->hal_handle, IVE_IOC_PADDING_MODE_REPLICATE);
    // enable ive_busy signal
    ive_hal_mcm_busy_on_off(&handle->hal_handle, 1);

    ive_hal_start(&handle->hal_handle);
}

#if defined(SUPPORT_BGBLUR)
/*******************************************************************************************************************
 * ive_drv_start_bgblur
 *   Start HW engine of BGBlur.
 *
 * Parameters:
 *   handle: device handle
 *   config: IVE configurations
 *
 * Return:
 *   none
 */
static void ive_drv_start_bgblur(ive_drv_handle *handle, ive_ioc_config *config)
{
    IVE_MSG(IVE_MSG_DBG, "start bgblur\n");

    // reset HW.
    ive_hal_sw_reset(&handle->hal_handle);
    // set input & output info.
    ive_hal_set_bgblur_images(&handle->hal_handle, config);
    // set params.
    ive_hal_set_coeff_bgblur(&handle->hal_handle, &config->coeff_bgblur);
    // set burst and outstanding.
    ive_hal_set_burst(&handle->hal_handle);
    // clear irq.
    ive_hal_clear_bgblur_irq(&handle->hal_handle, IVE_BGB_ISR_IRQ_CHECK_MASK);
    // set irq mask.
    ive_hal_set_bgblur_irq_mask(&handle->hal_handle, IVE_BGB_ISR_IRQ_CHECK_MASK);
    // set bgb dummy.
    ive_hal_bgblur_mcm_busy_on_off(&handle->hal_handle, 1);
#if defined(CONFIG_CHIP_IFORD)
    // set read outstanding.
    ive_hal_set_read_outstanding(&handle->hal_handle, IVE_HAL_READ_OUTSTANDING_32x128);
#endif
    // fire.
    ive_hal_start_bgblur(&handle->hal_handle);

    return;
}
#endif

/*******************************************************************************************************************
 * ive_drv_start_proc
 *   Start IVE process image
 *
 * Parameters:
 *   handle: device handle
 *
 * Return:
 *   IVE_IOC_ERROR
 */
IVE_IOC_ERROR ive_drv_process(ive_drv_handle *handle, ive_file_data *file_data)
{
    char          bDeviceReady = false;
    IVE_IOC_ERROR ret          = IVE_IOC_ERROR_NONE;
    CamOsMutex_t *pstMutex     = &file_data->dev_data->mutex;

    ret = drv_ive_check_config(&file_data->ioc_config);
    if (ret != IVE_IOC_ERROR_NONE)
    {
        return ret;
    }

    // handle->request_list and handle->dev_state is shared, so here need to lock mutex.
    CamOsMutexLock(pstMutex);

    /*
     * use local variable to get device state to avoid that device state is changed by isr before and after
     * add_request, and finally work on wrong flow.
     */
    if (IVE_DRV_STATE_READY == CamOsAtomicRead(&handle->dev_state))
    {
        bDeviceReady = true;
        // extract the first request with file state=DONE/READY if lists have.
        drv_ive_extract_request(&file_data->dev_data->drv_handle);
    }

    if (drv_ive_add_request(handle, file_data))
    {
        IVE_MSG(IVE_MSG_ERR, "Add request fail, file_data:0x%p!\n", file_data);
        CamOsMutexUnlock(pstMutex);
        return IVE_IOC_ERROR_MEMROY_FAILURE;
    }
    file_data->state = IVE_FILE_STATE_IN_QUEUE;

    // do nothing if hw is not ready
    if (false == bDeviceReady)
    {
        IVE_MSG(IVE_MSG_DBG, "HW is busy\n");
        CamOsMutexUnlock(pstMutex);
        return IVE_IOC_ERROR_NONE;
    }

    if (NULL == (file_data = drv_ive_get_request(handle)) || IVE_FILE_STATE_IN_QUEUE != file_data->state)
    {
        IVE_MSG(IVE_MSG_ERR, "do not have IN_QUEUE request in list head\n");
        CamOsMutexUnlock(pstMutex);
        return IVE_IOC_ERROR_MEMROY_FAILURE;
    }

    file_data->state = IVE_FILE_STATE_PROCESSING;
    CamOsAtomicSet(&handle->dev_state, IVE_DRV_STATE_PROCESSING);

    if (drv_ive_clock_enable(file_data->dev_data))
    {
        return IVE_IOC_ERROR_CLK;
    }

#if defined(SUPPORT_REQUEST_PRIORITY)
    handle->eDrvPriority = file_data->ioc_config.eTaskPriority;
    switch (file_data->ioc_config.op_type)
    {
#if defined(SUPPORT_BGBLUR)
        case IVE_IOC_OP_TYPE_BGBLUR:
            ive_drv_start_bgblur(handle, &file_data->ioc_config);
            break;
#endif

        default:
            ive_drv_start(handle, &file_data->ioc_config);
            break;
    }
#else
    ive_drv_start(handle, &file_data->ioc_config);
#endif
    CamOsMutexUnlock(pstMutex);

    return ret;
}

/*******************************************************************************************************************
 * ive_drv_post_process
 *   Post process after IVE HW done
 *
 * Parameters:
 *   handle: device handle
 *
 * Return:
 *   IVE_IOC_ERROR
 */
void ive_drv_post_process(ive_drv_handle *handle)
{
    if (CamOsAtomicRead(&handle->dev_state) == IVE_DRV_STATE_READY)
    {
        ive_file_data *next_file_data = NULL;

        // destroy the previous task before start the next request
        drv_ive_extract_request(handle);

        // post process and isr can excute synchronously, so here add state check to avoid getting finished request.
        if (NULL != (next_file_data = drv_ive_get_request(handle)) && IVE_FILE_STATE_IN_QUEUE == next_file_data->state)
        {
            IVE_MSG(IVE_MSG_DBG, "process: 0x%llx, 0x%llx, 0x%llx\n", (u64)next_file_data->ioc_config.input.address[0],
                    (u64)next_file_data->ioc_config.input.address[1],
                    (u64)next_file_data->ioc_config.output.address[2]);
            next_file_data->state       = IVE_FILE_STATE_PROCESSING;
            next_file_data->eTaskResult = E_IVE_TASK_INIT;
            CamOsAtomicSet(&handle->dev_state, IVE_DRV_STATE_PROCESSING);

            if (drv_ive_clock_enable(next_file_data->dev_data))
            {
                return;
            }

#if defined(SUPPORT_REQUEST_PRIORITY)
            handle->eDrvPriority = next_file_data->ioc_config.eTaskPriority;
            switch (next_file_data->ioc_config.op_type)
            {
#if defined(SUPPORT_BGBLUR)
                case IVE_IOC_OP_TYPE_BGBLUR:
                    ive_drv_start_bgblur(handle, &next_file_data->ioc_config);
                    break;
#endif

                default:
                    ive_drv_start(handle, &next_file_data->ioc_config);
                    break;
            }
#else
            ive_drv_start(handle, &next_file_data->ioc_config);
#endif
        }
    }
}

/*******************************************************************************************************************
 * ive_drv_pre_process
 *   Pre process when IVE HW done
 *
 * Parameters:
 *   handle: device handle
 *
 * Return:
 *   ive_file_data *
 */
ive_file_data *ive_drv_pre_process(ive_drv_handle *handle)
{
    ive_file_data *previous_file_data;

    previous_file_data = drv_ive_get_request(handle);

    if (previous_file_data != NULL)
    {
        if (CamOsAtomicRead(&handle->dev_state) == IVE_DRV_STATE_DONE)
        {
            previous_file_data->state       = IVE_FILE_STATE_DONE;
            previous_file_data->eTaskResult = E_IVE_TASK_SUCCESS;
        }
        else
        {
            previous_file_data->eTaskResult = E_IVE_TASK_FAILED;
        }
        IVE_MSG(IVE_MSG_DBG, "previous_file_data result = %d\n", previous_file_data->eTaskResult);
    }

    return previous_file_data;
}

/*******************************************************************************************************************
 * ive_drv_reset
 *   Start IVE reset
 *
 * Parameters:
 *   handle: device handle
 *
 * Return:
 *   void
 */
void ive_drv_reset(ive_drv_handle *handle)
{
    // set irq mask.
    ive_hal_set_irq_mask(&handle->hal_handle, 0);
#if defined(SUPPORT_BGBLUR)
    ive_hal_set_bgblur_irq_mask(&handle->hal_handle, 0);
#endif

    // Reset register
    ive_hal_sw_reset(&handle->hal_handle);

    return;
}

/*******************************************************************************************************************
 * ive_drv_destroy_process
 *   Delete task in request list
 *
 * Parameters:
 *   handle: device handle
 *
 * Return:
 *   ive_file_data *
 */
ive_file_data *ive_drv_destroy_process(ive_drv_handle *handle)
{
    return drv_ive_extract_request(handle);
}
