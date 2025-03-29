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
#include "drv_ive.h"
#include "drv_ive_clk.h"
#include "drv_ive_module.h"
#include "drv_ive_common.h"
#include "drv_ive_io.h"

#include "drv_sysdesc.h"
#include "errno.h"
#include "initcall.h"

struct resource
{
    u32 start;
    u16 size;
} __attribute__((__packed__));

ive_dev_data *g_pstDevData = NULL;

static int drv_ive_dts_init(ive_dev_data *dev_data)
{
    int   i       = 0;
    u32   err     = 0;
    u8    u8_intr = 0;
    void *reg     = NULL;

    struct resource ive_reg[IVE_BANK_NUM]     = {0};
    ss_phys_addr_t  u64BankBase[IVE_BANK_NUM] = {0};

    reg = ive_reg;

    err = MDrv_SysDesc_Read_U8(SYSDESC_DEV_ive0, SYSDESC_PRO_interrupts_u8, &u8_intr);
    if (err)
    {
        IVE_MSG(IVE_MSG_ERR, "get 'interrupt_u8' from sysdesc fail. err:%X\n", err);
        return -ENOMEM;
    }
    dev_data->irq = u8_intr;
    IVE_MSG(IVE_MSG_WRN, "get 'interrupt_u8' from sysdesc pass. err:%X, intr:%X\n", err, u8_intr);

    err = MDrv_SysDesc_Read_MultiTypes(SYSDESC_DEV_ive0, SYSDESC_PRO_reg_u32_u16, reg, sizeof(struct resource),
                                       sizeof(struct resource) * IVE_BANK_NUM);
    if (err)
    {
        IVE_MSG(IVE_MSG_ERR, "get 'reg_u32_u16' from sysdesc fail. err:0x%X\n", err);
        return -ENOMEM;
    }

    for (i = 0; i < IVE_BANK_NUM; ++i)
    {
        u64BankBase[i] = ive_reg[i].start;
    }

    if (ive_drv_init(&dev_data->drv_handle, NULL, &u64BankBase[0]) < 0)
    {
        IVE_MSG(IVE_MSG_ERR, "can't init driver\n");
        return -ENODEV;
    }
    return 0;
}

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
int drv_ive_module_probe(void)
{
    int           err      = 0;
    ive_dev_data *dev_data = NULL;

    if (g_pstDevData)
    {
        IVE_MSG(IVE_MSG_WRN, "gpDevData(0x%llx) is ready, do nothing and return.\n", (uint64_t)(uintptr_t)g_pstDevData);
        return 0;
    }

    // create drv data buffer
    dev_data = CamOsMemCalloc(1, sizeof(ive_dev_data));
    if (dev_data == NULL)
    {
        IVE_MSG(IVE_MSG_ERR, "can't allocate dev data buffer\n");
        return -ENOMEM;
    }

    IVE_MSG(IVE_MSG_DBG, "dev_data: 0x%p (size = %d)\n", dev_data, (uint32_t)sizeof(ive_dev_data));

    CamOsMutexInit(&dev_data->mutex);

    // Init dev_data
    dev_data->pdev = NULL;
    if (drv_ive_dts_init(dev_data))
    {
        IVE_MSG(IVE_MSG_ERR, "dts init failed\n");
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

    // Register a ISR
    err = CamOsIrqRequest(dev_data->irq, (void *)drv_ive_isr, "ive isr", dev_data);
    if (err != 0)
    {
        IVE_MSG(IVE_MSG_ERR, "isp interrupt failed (irq: %d, errno:%d)\n", dev_data->irq, err);
        err = -ENODEV;
        goto ERROR_2;
    }

    CamOsWorkQueueCreate(&dev_data->work_queue, "ive_queue", DRV_IVE_MAX_QUEUE_COUNT);

    g_pstDevData = dev_data;

    IVE_MSG(IVE_MSG_WRN, "ive drv probe success (gpDevData:0x%X, .mutex:0x%X)\n", (uintptr_t)g_pstDevData,
            (uintptr_t)&g_pstDevData->mutex);

    return 0;

ERROR_2:
    drv_ive_clock_release(dev_data);

ERROR_1:
    CamOsMemRelease(dev_data);

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
int drv_ive_module_remove(void)
{
    ive_dev_data *dev_data = g_pstDevData;
    IVE_MSG(IVE_MSG_DBG, "dev_data: 0x%p\n", dev_data);

    if (!g_pstDevData)
    {
        IVE_MSG(IVE_MSG_WRN, "gpDevData(0x%llx) is null, no need remove.\n", (uint64_t)(uintptr_t)g_pstDevData);
        return 0;
    }

    drv_ive_clock_release(dev_data);

    CamOsIrqFree(dev_data->irq, dev_data);

    CamOsMutexDestroy(&dev_data->mutex);

    CamOsWorkQueueDestroy(dev_data->work_queue);

    ive_drv_release(&dev_data->drv_handle);

    CamOsMemRelease(dev_data);

    return 0;
}

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
/*
int drv_ive_module_suspend(ive_dev_data *dev_data, pm_message_t state)
{
    IVE_MSG(IVE_MSG_DBG, "dev_data: 0x%p\n", dev_data);

    return 0;
}
*/

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
/*
int drv_ive_module_resume(ive_dev_data *dev_data)
{
    IVE_MSG(IVE_MSG_DBG, "dev_data: 0x%p\n", dev_data);

    return 0;
}
*/

void ive_init(void)
{
    drv_ive_module_probe();
}

void ive_deinit(void)
{
    drv_ive_module_remove();
}

rtos_device_initcall(ive_init);
