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

#include <string.h>
#include <fdtdec.h>
#include <dm/device.h>
#include <dm/read.h>
#include <stdlib.h>
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

static int sstar_fcie_probe(struct udevice *dev)
{
    return 0;
}

static int sstar_fcie_bind(struct udevice *dev)
{
    s32                          ret            = 0;
    u32                          fcie_interface = 0;
    struct sstar_fcie_interface *fcie_if        = NULL;
    fdt_addr_t                   addr[2]        = {0};

    addr[0] = dev_read_addr_index(dev, 0);

    if (!addr[0])
    {
        ret = -ENOMEM;
        goto err_out;
    }

    addr[1] = dev_read_addr_index(dev, 1);

    if (!addr[1])
    {
        ret = -ENOMEM;
        goto err_out;
    }

    if (NULL == (fcie_if = malloc(sizeof(struct sstar_fcie_interface))))
    {
        ret = -ENOMEM;
        goto err_out;
    }

    fcie_interface = dev_read_u32_default(dev, "interface", 0);

    if (fcie_interface < 0)
    {
        ret = -ENOMEM;
        goto err_out;
    }

    if (fcie_interface >= FCIE_IF_NUM)
    {
        ret = -ENOMEM;
        goto err_out;
    }

    fcie_if->hal.fcie0_base = (unsigned long)addr[0];
    fcie_if->hal.fcie3_base = (unsigned long)addr[1];

    fcie_if_list[fcie_interface] = fcie_if;

    return 0;
err_out:
    if (fcie_if)
    {
        free((void *)fcie_if);
    }

    return ret;
}

UCLASS_DRIVER(fcie) = {
    .id    = UCLASS_FCIE,
    .name  = "fcie",
    .flags = DM_UC_FLAG_SEQ_ALIAS,
};

static const struct udevice_id sstar_fcie_ids[] = {{.compatible = "sstar,fcie"}, {}};

U_BOOT_DRIVER(sstar_fcie) = {.name     = "sstar_fcie",
                             .id       = UCLASS_FCIE,
                             .of_match = sstar_fcie_ids,
                             .bind     = sstar_fcie_bind,
                             .probe    = sstar_fcie_probe};
