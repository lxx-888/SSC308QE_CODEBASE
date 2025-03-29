/*
 * spi_flash_controller.h- Sigmastar
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

#ifndef _SPI_FLASH_CONTROLLER_H_
#define _SPI_FLASH_CONTROLLER_H_

#include <drv_flash_os_impl.h>

typedef signed char        s8;
typedef unsigned char      u8;
typedef signed short       s16;
typedef unsigned short     u16;
typedef signed int         s32;
typedef unsigned int       u32;
typedef signed long long   s64;
typedef unsigned long long u64;

typedef enum
{
    SPIFLASH_COMMAND_WRITE_STATUS,
    SPIFLASH_COMMAND_READ_STATUS,
    SPIFLASH_COMMAND_PROGRAM,
    SPIFLASH_COMMAND_READ,
    SPIFLASH_COMMAND_READ_TO_XZDEC
} spiflash_command;

struct spiflash_msg_cmd
{
    u8  cmd;
    u32 address;
    u8  addr_bytes;
    u8  dummy;
};

struct spiflash_msg
{
    u8                      bdma_en;
    u8                      cs_select;
    u8 *                    buffer;
    u32                     size;
    spiflash_command        command;
    struct spiflash_msg_cmd cmd;
};

struct spiflash_config
{
    u8 cmd;
    u8 rate;
    u8 have_phase;
    u8 phase;
    u8 cs_select;
};

struct spiflash_control
{
    u8 engine;
    u8 boot_storage;
    s8 (*transfer)(struct spiflash_control *, struct spiflash_msg *);
    s8 (*setup)(struct spiflash_control *, struct spiflash_config *);
    u8 (*need_autok)(struct spiflash_control *, u8);
    u8 (*try_phase)(struct spiflash_control *, u8 (*parrtern_check)(void));
    void *priv;
};

u8   spiflash_get_boot_storage_master(void);
u8   spiflash_get_master(u8 bus);
s8   spiflash_transfer(u8 id, struct spiflash_msg *msg);
s8   spiflash_setup(u8 id, struct spiflash_config *config);
u8   spiflash_need_autok(u8 id, u8 cs_select);
u8   spiflash_try_phase(u8 id, u8 (*parrtern_check)(void));
void spiflash_register_master(struct spiflash_control *flash_ctrl);

#endif /* _SPI_FLASH_CONTROLLER_H_ */
