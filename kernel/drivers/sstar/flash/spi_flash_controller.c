/*
 * spi_flash_controller.c- Sigmastar
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

#include <spi_flash_controller.h>

#define SPIFLASH_CTRL_MAX_CNT 16

static struct spiflash_control *ctrl_list[SPIFLASH_CTRL_MAX_CNT] = {0};

u8 spiflash_get_master(u8 bus)
{
    u8 id = 0;

    while ((id < SPIFLASH_CTRL_MAX_CNT) && ctrl_list[id])
    {
        if (ctrl_list[id]->engine == bus)
            break;

        id++;
    }

    if ((id >= SPIFLASH_CTRL_MAX_CNT) || !ctrl_list[id])
        return 0xFF;

    return id;
}

s8 spiflash_transfer(u8 id, struct spiflash_msg *msg)
{
    if (id >= SPIFLASH_CTRL_MAX_CNT)
        return -1;

    return ctrl_list[id]->transfer(ctrl_list[id], msg);
}

s8 spiflash_setup(u8 id, struct spiflash_config *config)
{
    if (id >= SPIFLASH_CTRL_MAX_CNT)
        return -1;

    return ctrl_list[id]->setup(ctrl_list[id], config);
}

u8 spiflash_need_autok(u8 id, u8 cs_select)
{
    if (id >= SPIFLASH_CTRL_MAX_CNT)
        return -1;

    return ctrl_list[id]->need_autok(ctrl_list[id], cs_select);
}

u8 spiflash_try_phase(u8 id, u8 (*parrtern_check)(void))
{
    if (id >= SPIFLASH_CTRL_MAX_CNT)
        return -1;

    return ctrl_list[id]->try_phase(ctrl_list[id], parrtern_check);
}

u8 spiflash_get_boot_storage_master(void)
{
    u8 id = 0;

    while ((id < SPIFLASH_CTRL_MAX_CNT) && ctrl_list[id])
    {
        if (ctrl_list[id]->boot_storage)
            break;

        id++;
    }

    if ((id >= SPIFLASH_CTRL_MAX_CNT) || !ctrl_list[id])
        return 0xFF;

    return id;
}

void spiflash_register_master(struct spiflash_control *flash_ctrl)
{
    u8 id = 0;

    while ((id < SPIFLASH_CTRL_MAX_CNT) && ctrl_list[id])
    {
        id++;
    }

    if (id >= SPIFLASH_CTRL_MAX_CNT)
        return;

    ctrl_list[id] = flash_ctrl;
}

void spiflash_unregister_master(struct spiflash_control *flash_ctrl)
{
    u8 id = 0;

    while ((id < SPIFLASH_CTRL_MAX_CNT) && (ctrl_list[id] != flash_ctrl))
    {
        id++;
    }

    if (id >= SPIFLASH_CTRL_MAX_CNT)
        return;

    ctrl_list[id] = NULL;
}

EXPORT_SYMBOL_GPL(spiflash_get_master);
EXPORT_SYMBOL_GPL(spiflash_transfer);
EXPORT_SYMBOL_GPL(spiflash_setup);
EXPORT_SYMBOL_GPL(spiflash_get_boot_storage_master);
EXPORT_SYMBOL_GPL(spiflash_register_master);
EXPORT_SYMBOL_GPL(spiflash_unregister_master);
