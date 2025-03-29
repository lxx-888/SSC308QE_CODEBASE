/*
 * mdrv_spinor.h- Sigmastar
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
#ifndef _MDRV_SPINOR_H_
#define _MDRV_SPINOR_H_

#include <spi_flash_controller.h>
#include <cis.h>

typedef signed char        s8;
typedef unsigned char      u8;
typedef signed short       s16;
typedef unsigned short     u16;
typedef signed int         s32;
typedef unsigned int       u32;
typedef signed long long   s64;
typedef unsigned long long u64;

// please refer to the serial flash datasheet
#define SPI_NOR_CMD_READ     (0x03)
#define SPI_NOR_CMD_FASTREAD (0x0B)
#define SPI_NOR_CMD_RDID     (0x9F)
#define SPI_NOR_CMD_WREN     (0x06)
#define SPI_NOR_CMD_WRDIS    (0x04)
#define SPI_NOR_CMD_SE       (0x20)
#define SPI_NOR_CMD_32BE     (0x52)
#define SPI_NOR_CMD_64BE     (0xD8)
#define SPI_NOR_CMD_CE       (0xC7)

#define SPI_NOR_CMD_PP    (0x02)
#define SPI_NOR_CMD_QP    (0x32)
#define SPI_NOR_CMD_4PP   (0x38)
#define SPI_NOR_CMD_QR_6B (0x6B)
#define SPI_NOR_CMD_QR_EB (0xEB)

#define SPI_NOR_CMD_RDSR  (0x05)
#define SPI_NOR_CMD_RDSR2 (0x35)
#define SPI_NOR_CMD_RDSR3 (0x15)
#define SPI_NOR_CMD_WRSR  (0x01)
#define SPI_NOR_CMD_WRSR2 (0x31)
#define SPI_NOR_CMD_WRSR3 (0x11)

// support for 256Mb up MIX flash
#define SPI_NOR_CMD_RDEAR    (0xC8)
#define SPI_NOR_CMD_WREAR    (0xC5)
#define SPI_NOR_CMD_RESET    (0X99)
#define SPI_NOR_CMD_EN_RESET (0X66)

// enter 4-byte address mode
#define SPI_NOR_CMD_ENTER_4BYTE (0xB7)

#define SPI_NOR_READ_DUMMY         0x08
#define SPI_NOR_DEFAULT_ERASE_SIZE (4 << 10)
#define SPI_NOR_BUSY               (0x01)
#define SPI_NOR_16MB               (1 << 24)
#define SPI_NOR_16MB_MASK          ((1 << 24) - 1)

#define SPI_NOR_ERASE_SIZE_4K  SPI_NOR_CMD_SE
#define SPI_NOR_ERASE_SIZE_32K SPI_NOR_CMD_32BE
#define SPI_NOR_ERASE_SIZE_64K SPI_NOR_CMD_64BE

typedef enum _SPINOR_ERROR_NUM
{
    ERR_SPINOR_SUCCESS = 0x00,
    ERR_SPINOR_RESET_FAIL,
    ERR_SPINOR_E_FAIL,
    ERR_SPINOR_P_FAIL,
    ERR_SPINOR_INVALID,
    ERR_SPINOR_TIMEOUT,
    ERR_SPINOR_DEVICE_FAILURE,
    ERR_SPINOR_BDMA_FAILURE,
} spinor_flash_errno;

typedef struct
{
    u32 page_size;
    u32 sector_size;
    u32 block_size;
    u32 capacity;
} flash_nor_info_t;

struct spinor_order
{
    u8  read_data;
    u8  address_byte;
    u8  dummy;
    u8  page_program;
    u32 time_wait;
};

struct spinor_handle
{
    u8                  ctrl_id;
    u8                  bdma_to_xzdec_en;
    u8                  ext_addr;
    spinor_nri_t *      nri;
    struct spinor_order order;
    struct spiflash_msg msg;
};

void mdrv_spinor_info(struct spinor_handle *handle, flash_nor_info_t *pst_flash_nor_info);
void mdrv_spinor_setup_by_default(struct spinor_handle *handle);

u8 mdrv_spinor_setup_by_nri(struct spinor_handle *handle);
u8 mdrv_spinor_hardware_init(struct spinor_handle *handle);

u8 mdrv_spinor_reset(struct spinor_handle *handle);
u8 mdrv_spinor_is_nri_match(struct spinor_handle *handle, spinor_info_t *spinor_info);
u8 mdrv_spinor_unlock_whole_flash(struct spinor_handle *handle);

u8 mdrv_spinor_read(struct spinor_handle *handle, u32 address, u8 *data, u32 size);
u8 mdrv_spinor_program(struct spinor_handle *handle, u32 address, u8 *data, u32 size);
u8 mdrv_spinor_erase(struct spinor_handle *handle, u32 address, u32 size);
u8 mdrv_spinor_read_to_xzdec(struct spinor_handle *handle, u32 u32_address, u8 *pu8_data, u32 u32_size);

#endif /* _MDRV_SPINOR_H_ */
