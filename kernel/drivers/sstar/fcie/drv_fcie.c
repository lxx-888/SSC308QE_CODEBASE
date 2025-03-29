/*
 * drv_fcie.c - Sigmastar
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

#include <linux/string.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/printk.h>
#include <linux/platform_device.h>
#include <linux/dma-mapping.h>
#include <linux/delay.h>
#include <linux/sched.h>
#include <linux/version.h>
#include <linux/slab.h>
#include <linux/clk.h>
#include <linux/clk-provider.h>
#include <linux/types.h>
#include <linux/crc32.h>
#include <ms_msys.h>
#include <generated/uapi/linux/version.h>
#include <cam_sysfs.h>
#include <drv_flash_os_impl.h>
#include <hal_fcie.h>
#include <drv_fcie.h>

struct sstar_fcie_interface
{
    struct hal_fcie hal;
};

static struct sstar_fcie_interface *fcie_if_list[FCIE_IF_NUM] = {0};

void sstar_fcie_get(fcie_interface fcie_id) {}

void sstar_fcie_release(fcie_interface fcie_id) {}

u8 sstar_fcie_ecc_set_config(fcie_interface fcie_id, struct sstar_fcie_ecc *ecc)
{
    const struct hal_fcie_ecc_config *config = NULL;

    if (fcie_id >= FCIE_IF_NUM || !fcie_if_list[fcie_id])
        return FCIE_INVALID;

    if (HAL_FCIE_SUCCESS != hal_fcie_ecc_get_config(ecc->mode, &config))
    {
        return FCIE_INVALID;
    }

    ecc->page_size   = config->page_size;
    ecc->oob_size    = config->oob_size;
    ecc->sector_size = config->sector_size;

    return FCIE_SUCCESS;
}

u8 sstar_fcie_ecc_encode(fcie_interface fcie_id, struct sstar_fcie_ecc *ecc)
{
    u8                           ret     = FCIE_SUCCESS;
    struct sstar_fcie_interface *fcie_if = NULL;

    if (fcie_id >= FCIE_IF_NUM || !fcie_if_list[fcie_id])
        return FCIE_INVALID;

    fcie_if = fcie_if_list[fcie_id];

    sstar_fcie_get(fcie_id);

    do
    {
        hal_fcie_ecc_reset(&fcie_if->hal);

        if (HAL_FCIE_SUCCESS != hal_fcie_ecc_setup(&fcie_if->hal, ecc->mode))
        {
            ret = FCIE_DEVICE_FAILURE;
            break;
        }

        if (HAL_FCIE_SUCCESS != hal_fcie_ecc_encode(&fcie_if->hal, ecc->path, ecc->sector_cnt, ecc->fcie_buffer))
        {
            ret = FCIE_DEVICE_FAILURE;
            break;
        }
    } while (0);

    sstar_fcie_release(fcie_id);

    return ret;
}

u8 sstar_fcie_ecc_decode(fcie_interface fcie_id, struct sstar_fcie_ecc *ecc)
{
    u8                           ret     = FCIE_SUCCESS;
    struct sstar_fcie_interface *fcie_if = NULL;

    if (fcie_id >= FCIE_IF_NUM || !fcie_if_list[fcie_id])
        return FCIE_INVALID;

    fcie_if = fcie_if_list[fcie_id];

    sstar_fcie_get(fcie_id);

    do
    {
        hal_fcie_ecc_reset(&fcie_if->hal);

        if (HAL_FCIE_SUCCESS != hal_fcie_ecc_setup(&fcie_if->hal, ecc->mode))
        {
            ret = FCIE_DEVICE_FAILURE;
            break;
        }

        if (HAL_FCIE_SUCCESS != hal_fcie_ecc_decode(&fcie_if->hal, ecc->path, ecc->sector_cnt, ecc->fcie_buffer))
        {
            ret = FCIE_DEVICE_FAILURE;
            break;
        }

        if (HAL_FCIE_SUCCESS != hal_fcie_ecc_get_status(&fcie_if->hal, &ecc->ecc_status, &ecc->ecc_bitflip_cnt))
        {
            ret = FCIE_DEVICE_FAILURE;
            break;
        }
    } while (0);

    sstar_fcie_release(fcie_id);

    return ret;
}

static int sstar_fcie_probe(struct platform_device *pdev)
{
    s32                          ret;
    u32                          fcie_interface = 0;
    struct sstar_fcie_interface *fcie_if        = NULL;
    struct resource *            resource_msg[2];

    resource_msg[0] = platform_get_resource(pdev, IORESOURCE_MEM, 0);
    if (!resource_msg[0])
    {
        ret = -ENOMEM;
        goto err_out;
    }

    resource_msg[1] = platform_get_resource(pdev, IORESOURCE_MEM, 1);
    if (!resource_msg[1])
    {
        ret = -ENOMEM;
        goto err_out;
    }

    if (NULL == (fcie_if = kmalloc(sizeof(struct sstar_fcie_interface), GFP_KERNEL)))
    {
        goto err_out;
    }

    ret = CamofPropertyReadU32(pdev->dev.of_node, "interface", &fcie_interface);
    if (ret)
    {
        ret = -ENOENT;
        goto err_out;
    }

    if (fcie_interface >= FCIE_IF_NUM)
    {
        ret = -ENOENT;
        goto err_out;
    }

    fcie_if->hal.fcie0_base = resource_msg[0]->start;
    fcie_if->hal.fcie3_base = resource_msg[1]->start;

    fcie_if_list[fcie_interface] = fcie_if;

    platform_set_drvdata(pdev, fcie_if);

    return 0;

err_out:
    if (fcie_if)
    {
        kfree((void *)fcie_if);
    }

    return ret;
}

static int sstar_fcie_remove(struct platform_device *pdev)
{
    return 0;
}

#ifdef CONFIG_PM_SLEEP
static int sstar_fcie_suspend(struct platform_device *pdev, pm_message_t state)
{
    return 0;
}

static int sstar_fcie_resume(struct platform_device *pdev)
{
    return 0;
}

static int sstar_fcie_shutdown(struct platform_device *pdev)
{
    return 0;
}
#endif

static const struct of_device_id fcie_of_dt_ids[] = {{.compatible = "sstar,fcie"}, {/* sentinel */}};
MODULE_DEVICE_TABLE(of, fcie_of_dt_ids);

static struct platform_driver sstar_fcie_driver = {
    .probe  = sstar_fcie_probe,
    .remove = sstar_fcie_remove,
#ifdef CONFIG_PM_SLEEP
    .suspend  = sstar_fcie_suspend,
    .resume   = sstar_fcie_resume,
    .shutdown = sstar_fcie_shutdown,
#endif
    .driver =
        {
            .name           = "sstar-fcie",
            .owner          = THIS_MODULE,
            .of_match_table = (fcie_of_dt_ids),
        },
};
module_platform_driver(sstar_fcie_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("SSTAR");
MODULE_DESCRIPTION("Sstar FCIE driver");
