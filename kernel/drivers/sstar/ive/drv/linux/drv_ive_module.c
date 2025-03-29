/*
 * drv_ive_module.c- Sigmastar
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
#include <linux/poll.h>
#include <linux/module.h>
#include <linux/of_irq.h>
#include <linux/interrupt.h>

#include "ms_msys.h"
#include "drv_ive.h"
#include "drv_ive_clk.h"
#include "drv_ive_io_st.h"
#include "drv_ive_common.h"

#if defined(SUPPORT_IVE_SYS_FS)
#include "drv_ive_sys.h"
#endif

//-------------------------------------------------------------------------------------------------
// MACRO only for this file.
//-------------------------------------------------------------------------------------------------
#define IVE_MAX_TIMEOUT_MS (5000)

//-------------------------------------------------------------------------------------------------
// STRUCT only for this file.
//-------------------------------------------------------------------------------------------------
typedef struct
{
    int major;              // cdev major number
    int minor_star;         // begining of cdev minor number
    int reg_count;          // registered count
    struct CamClass *class; // class pointer
} ive_drv_info;

static ive_drv_info g_stIveDrv = {0, 0, 0, NULL};

/*******************************************************************************************************************
 * drv_ive_module_open
 *   File open handler
 *   The device can has a instance at the same time, and the open
 *   operator also enable the clock and request ISR.
 *
 * Parameters:
 *   inode: inode
 *   filp:  file structure
 *
 * Return:
 *   standard return value
 */
static int drv_ive_module_open(struct inode *inode, struct file *filp)
{
    ive_dev_data * dev_data  = CAM_OS_CONTAINER_OF(inode->i_cdev, ive_dev_data, cdev);
    ive_file_data *file_data = NULL;

    // allocate buffer
    file_data = devm_kcalloc(&dev_data->pdev->dev, 1, sizeof(ive_file_data), GFP_KERNEL);
    if (file_data == NULL)
    {
        IVE_MSG(IVE_MSG_ERR, "error: can't allocate buffer\n");
        return -ENOSPC;
    }

    IVE_MSG(IVE_MSG_DBG, "filp: 0x%p, file_data: 0x%p\n", filp, file_data);

    // Assgin dev_data and keep file_data in the file structure
    file_data->state       = IVE_FILE_STATE_READY;
    file_data->eTaskResult = E_IVE_TASK_INIT;
    file_data->dev_data    = dev_data;
    filp->private_data     = file_data;

    // Init wait queue
    CamOsConditionInit(&file_data->wait_queue);

    return 0;
}

/*******************************************************************************************************************
 * drv_ive_module_release
 *   File close handler
 *   The operator will release clock & ISR
 *
 * Parameters:
 *   inode: inode
 *   filp:  file structure
 *
 * Return:
 *   standard return value
 */
static int drv_ive_module_release(struct inode *inode, struct file *filp)
{
    ive_file_data *file_data = (ive_file_data *)filp->private_data;
    ive_dev_data * dev_data  = file_data->dev_data;

    IVE_MSG(IVE_MSG_DBG, "filp: 0x%p\n", filp);

    CamOsConditionDeinit(&file_data->wait_queue);

    // Release memory
    devm_kfree(&dev_data->pdev->dev, file_data);

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
static IVE_IOC_ERROR drv_ive_module_ioctl_process(ive_file_data *file_data, unsigned long arg)
{
    if (file_data->state != IVE_FILE_STATE_READY)
    {
        IVE_MSG(IVE_MSG_ERR, "One file can request once at the same time only\n");
        return IVE_IOC_ERROR_BUSY;
    }

    if (copy_from_user(&file_data->ioc_config, (void *)arg, sizeof(ive_ioc_config)) != 0)
    {
        IVE_MSG(IVE_MSG_ERR, "Can't copy config from user space\n");
        return IVE_IOC_ERROR_PROC_CONFIG;
    }

    return ive_drv_process(&(file_data->dev_data->drv_handle), file_data);
}

/*******************************************************************************************************************
 * drv_ive_module_ioctl
 *   IOCTL handler entry for file operator
 *
 * Parameters:
 *   filp: pointer of file structure
 *   cmd:  command
 *   arg:  argument from user space
 *
 * Return:
 *   standard return value
 */
static long drv_ive_module_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
    ive_file_data *file_data = (ive_file_data *)filp->private_data;
    IVE_IOC_ERROR  err       = IVE_IOC_ERROR_NONE;

    IVE_MSG(IVE_MSG_DBG, "filp: 0x%p, command: 0x%X\n", filp, cmd);

    switch (cmd)
    {
        case IVE_IOC_PROCESS:
            err = drv_ive_module_ioctl_process(file_data, arg);
            break;

        default:
            err = ESRCH;
            break;
    }

    return err;
}

/*******************************************************************************************************************
 * drv_ive_module_poll
 *   poll handler entry for file operator
 *
 * Parameters:
 *   filp: pointer of file structure
 *   wait: wait queue
 *
 * Return:
 *   0, (POLLIN | POLLRDNORM) or POLLERR(here means task failed)
 */
static unsigned int drv_ive_module_poll(struct file *filp, struct poll_table_struct *wait)
{
    unsigned int   u32PollResult = 0;
    ive_file_data *file_data     = (ive_file_data *)filp->private_data;

    IVE_MSG(IVE_MSG_DBG, "filp: 0x%p, polling 0x%p 0x%X\n", filp, &file_data->wait_queue, file_data->state);

    if (file_data->state == IVE_FILE_STATE_READY)
    {
        return POLLIN | POLLRDNORM;
    }

    CamOsConditionTimedWait(&file_data->wait_queue, file_data->state == IVE_FILE_STATE_DONE, IVE_MAX_TIMEOUT_MS);

    switch (file_data->state)
    {
        case IVE_FILE_STATE_DONE:
            file_data->state = IVE_FILE_STATE_READY;
            u32PollResult    = POLLIN | POLLRDNORM;
            break;

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
            u32PollResult = POLLERR;
            break;
    }

    return u32PollResult;
}

const struct file_operations stIveFileOps = {
    .owner          = THIS_MODULE,
    .open           = drv_ive_module_open,
    .release        = drv_ive_module_release,
    .unlocked_ioctl = drv_ive_module_ioctl,
    .poll           = drv_ive_module_poll,
};

//-------------------------------------------------------------------------------------------------
// Platform functions
//-------------------------------------------------------------------------------------------------

/*******************************************************************************************************************
 * drv_ive_module_probe
 *   Platform device prob handler
 *
 * Parameters:
 *   pdev: platfrom device
 *
 * Return:
 *   standard return value
 */
static int drv_ive_module_probe(struct platform_device *pdev)
{
    int              i                         = 0;
    int              err                       = 0;
    ive_dev_data *   dev_data                  = NULL;
    ss_phys_addr_t   u64BankBase[IVE_BANK_NUM] = {0};
    struct resource *res                       = NULL;
    dev_t            stDevNum                  = 0;

    // create drv data buffer
    if (NULL == (dev_data = devm_kcalloc(&pdev->dev, 1, sizeof(ive_dev_data), GFP_KERNEL)))
    {
        IVE_MSG(IVE_MSG_ERR, "can't allocate dev data buffer\n");
        return -ENOMEM;
    }
    IVE_MSG(IVE_MSG_DBG, "dev_data: 0x%p (size = %d)\n", dev_data, (uint32_t)sizeof(ive_dev_data));
    dev_data->pdev = pdev;

    // Get base address
    for (i = 0; i < IVE_BANK_NUM; ++i)
    {
        if (NULL == (res = platform_get_resource(pdev, IORESOURCE_MEM, i)))
        {
            IVE_MSG(IVE_MSG_ERR, "can't find bank%d base address\n", i);
            err = -ENODEV;
            goto ERROR_1;
        }
        u64BankBase[i] = res->start;
    }

    CamOsMutexInit(&dev_data->mutex);

    // Init dev_data
    if (ive_drv_init(&dev_data->drv_handle, pdev, u64BankBase) < 0)
    {
        IVE_MSG(IVE_MSG_ERR, "can't init driver\n");
        err = -ENODEV;
        goto ERROR_1;
    }

    // Init clock
    if (drv_ive_clock_init(dev_data))
    {
        IVE_MSG(IVE_MSG_ERR, "can't init clock\n");
        err = -ENODEV;
        goto ERROR_1;
    }

    // Retrieve IRQ
    dev_data->irq = irq_of_parse_and_map(pdev->dev.of_node, 0);

    // Register a ISR
    if (0 != (err = CamOsIrqRequest(dev_data->irq, (void *)drv_ive_isr, "ive_isr", dev_data)))
    {
        IVE_MSG(IVE_MSG_ERR, "register ive isr failed(irq:%d, errno:%d)\n", dev_data->irq, err);
        err = -ENODEV;
        goto ERROR_2;
    }

    CamOsWorkQueueCreate(&dev_data->work_queue, "ive_queue", DRV_IVE_MAX_QUEUE_COUNT);

    // Add cdev
    cdev_init(&dev_data->cdev, &stIveFileOps);
    stDevNum = MKDEV(g_stIveDrv.major, g_stIveDrv.minor_star + g_stIveDrv.reg_count);
    if (0 != (err = cdev_add(&dev_data->cdev, stDevNum, 1)))
    {
        IVE_MSG(IVE_MSG_ERR, "Unable add a character device\n");
        goto ERROR_3;
    }

    // Create a instance in class
    dev_data->pstDev = CamDeviceCreate(g_stIveDrv.class, NULL, stDevNum, dev_data, DRV_IVE_NAME "%d",
                                       g_stIveDrv.minor_star + g_stIveDrv.reg_count);

    if (IS_ERR(dev_data->pstDev))
    {
        IVE_MSG(IVE_MSG_ERR, "can't create device\n");
        err = -ENODEV;
        goto ERROR_4;
    }

    // Increase registered count
    g_stIveDrv.reg_count++;

    dev_set_drvdata(&pdev->dev, dev_data);

#if defined(SUPPORT_IVE_SYS_FS)
    // init sys fs.
    DrvIveSysInit(dev_data);
#endif

    return 0;

ERROR_4:
    cdev_del(&dev_data->cdev);

ERROR_3:
    CamOsIrqFree(dev_data->irq, dev_data);

ERROR_2:
    drv_ive_clock_release(dev_data);

ERROR_1:
    devm_kfree(&pdev->dev, dev_data);

    return err;
}

/*******************************************************************************************************************
 * drv_ive_module_remove
 *   Platform device remove handler
 *
 * Parameters:
 *   pdev: platfrom device
 *
 * Return:
 *   standard return value
 */
static int drv_ive_module_remove(struct platform_device *pdev)
{
    ive_dev_data *dev_data = dev_get_drvdata(&pdev->dev);

    IVE_MSG(IVE_MSG_DBG, "dev_data: 0x%p\n", dev_data);

#if defined(SUPPORT_IVE_SYS_FS)
    DrvIveSysDeinit(dev_data);
#endif

    drv_ive_clock_release(dev_data);

    CamOsIrqFree(dev_data->irq, dev_data);

    CamOsMutexDestroy(&dev_data->mutex);

    CamOsWorkQueueDestroy(dev_data->work_queue);

    ive_drv_release(&dev_data->drv_handle);

    CamDeviceDestroy(g_stIveDrv.class, dev_data->cdev.dev);
    cdev_del(&dev_data->cdev);

    devm_kfree(&dev_data->pdev->dev, dev_data);

    return 0;
}

#ifdef CONFIG_PM_SLEEP
/*******************************************************************************************************************
 * drv_ive_module_suspend
 *   Platform device suspend handler, but nothing to do here
 *
 * Parameters:
 *   pdev: platfrom device
 *
 * Return:
 *   standard return value
 */
static int drv_ive_module_suspend(struct platform_device *pdev, pm_message_t state)
{
    ive_dev_data *dev_data = dev_get_drvdata(&pdev->dev);

    IVE_MSG(IVE_MSG_DBG, "dev_data: 0x%p\n", dev_data);

    drv_ive_clock_disable(dev_data);

    return 0;
}

/*******************************************************************************************************************
 * drv_ive_module_resume
 *   Platform device resume handler, but nothing to do here
 *
 * Parameters:
 *   pdev: platfrom device
 *
 * Return:
 *   standard return value
 */
static int drv_ive_module_resume(struct platform_device *pdev)
{
    ive_dev_data *dev_data = dev_get_drvdata(&pdev->dev);

    IVE_MSG(IVE_MSG_DBG, "dev_data: 0x%p\n", dev_data);

    drv_ive_clock_enable(dev_data);

    return 0;
}
#endif

static const struct of_device_id drv_ive_match[] = {
    {
        .compatible = "sstar,infinity-ive",
        /*.data = NULL,*/
    },
    {},
};

static struct platform_driver drv_ive_driver = {.probe  = drv_ive_module_probe,
                                                .remove = drv_ive_module_remove,
#ifdef CONFIG_PM_SLEEP
                                                .suspend = drv_ive_module_suspend,
                                                .resume  = drv_ive_module_resume,
#endif

                                                .driver = {
                                                    .of_match_table = of_match_ptr(drv_ive_match),
                                                    .name           = "sstar_ive",
                                                    .owner          = THIS_MODULE,
                                                }};

//-------------------------------------------------------------------------------------------------
// Module functions
//-------------------------------------------------------------------------------------------------

/*******************************************************************************************************************
 * drv_ive_module_init
 *   module init function
 *
 * Parameters:
 *   N/A
 *
 * Return:
 *   standard return value
 */
static int drv_ive_module_init(void)
{
    int   err;
    dev_t dev;

    IVE_MSG(IVE_MSG_DBG, "Moudle Init\n");

    // Allocate cdev id
    if ((err = alloc_chrdev_region(&dev, DRV_IVE_MINOR, DRV_IVE_DEVICE_COUNT, DRV_IVE_NAME)))
    {
        IVE_MSG(IVE_MSG_ERR, "Unable allocate cdev id\n");
        return err;
    }

    g_stIveDrv.major      = MAJOR(dev);
    g_stIveDrv.minor_star = MINOR(dev);
    g_stIveDrv.reg_count  = 0;

    // Register device class
    if (IS_ERR(g_stIveDrv.class = msys_get_sysfs_camclass()))
    {
        IVE_MSG(IVE_MSG_ERR, "Failed at msys_get_sysfs_camclass()\n");
        err = PTR_ERR(g_stIveDrv.class);
        goto ERR_RETURN_1;
    }

    // Register platform driver
    err = platform_driver_register(&drv_ive_driver);
    if (err != 0)
    {
        goto ERR_RETURN_1;
    }

    return 0;

ERR_RETURN_1:
    unregister_chrdev_region(MKDEV(g_stIveDrv.major, g_stIveDrv.minor_star), DRV_IVE_DEVICE_COUNT);

    return err;
}

/*******************************************************************************************************************
 * drv_ive_module_exit
 *   module exit function
 *
 * Parameters:
 *   N/A
 *
 * Return:
 *   standard return value
 */
static void drv_ive_module_exit(void)
{
    /*de-initial the who GFLIPDriver */
    IVE_MSG(IVE_MSG_DBG, "Modules Exit\n");

    platform_driver_unregister(&drv_ive_driver);
    unregister_chrdev_region(MKDEV(g_stIveDrv.major, g_stIveDrv.minor_star), DRV_IVE_DEVICE_COUNT);
}

module_init(drv_ive_module_init);
module_exit(drv_ive_module_exit);

MODULE_AUTHOR("SSTAR");
MODULE_DESCRIPTION("IVE ioctrl driver");
MODULE_LICENSE("GPL");
