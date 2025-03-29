/*
 * mdrv_spinand.h- Sigmastar
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
#ifndef _MDRV_SPINAND_H_
#define _MDRV_SPINAND_H_

#include <spi_flash_controller.h>
#include <cis.h>
#include <drv_fcie.h>
#include <hal_fcie.h>

typedef signed char        s8;
typedef unsigned char      u8;
typedef signed short       s16;
typedef unsigned short     u16;
typedef signed int         s32;
typedef unsigned int       u32;
typedef signed long long   s64;
typedef unsigned long long u64;

typedef enum _spinand_error_num
{
    ERR_SPINAND_SUCCESS = 0x00,
    ERR_SPINAND_ECC_CORRECTED,
    ERR_SPINAND_ECC_NOT_CORRECTED,
    ERR_SPINAND_ECC_RESERVED,
    ERR_SPINAND_E_FAIL,
    ERR_SPINAND_P_FAIL,
    ERR_SPINAND_TIMEOUT,
    ERR_SPINAND_INVALID,
    ERR_SPINAND_DEVICE_FAILURE,
    ERR_SPINAND_BDMA_FAILURE,
} spiflash_flash_errno_e;

// SPI NAND COMMAND
#define SPI_NAND_CMD_RFC       0x03
#define SPI_NAND_CMD_RFC_DUMMY 0x08
#define SPI_NAND_CMD_FRFC      0x0B
#define SPI_NAND_CMD_PGRD      0x13
#define SPI_NAND_CMD_RDID      0x9F
#define SPI_NAND_CMD_WREN      0x06
#define SPI_NAND_CMD_WRDIS     0x04
#define SPI_NAND_CMD_PL        0x02
#define SPI_NAND_CMD_QPL       0x32
#define SPI_NAND_CMD_RPL       0x84
#define SPI_NAND_CMD_QRPL      0x34
#define SPI_NAND_CMD_PE        0x10
#define SPI_NAND_CMD_BE        0xD8
#define SPI_NAND_CMD_GF        0x0F
#define SPI_NAND_CMD_SF        0x1F
#define SPI_NAND_CMD_RESET     0xFF
#define SPI_NAND_REG_PROT      0xA0
#define SPI_NAND_REG_FEAT      0xB0
#define SPI_NAND_REG_STAT      0xC0
#define SPI_NAND_REG_FUT       0xD0
#define SPI_NAND_STAT_E_FAIL   (0x01 << 2)
#define SPI_NAND_STAT_P_FAIL   (0x01 << 3)
#define ECC_STATUS_PASS        (0x00 << 4)
#define ECC_NOT_CORRECTED      (0x02 << 4)
#define ECC_NO_CORRECTED       (0x00 << 4)
#define ECC_STATUS_MASK        (0x03 << 4)
#define ECC_STATUS_RESERVED    (0x03 << 4)
#define SPI_NAND_STAT_OIP      (0x1)

typedef struct
{
    u8  id_byte_cnt;
    u8  id[15];
    u16 blk_page_cnt;
    u16 blk_cnt;
    u16 sector_size;
    u16 page_size;
    u16 oob_size;
    u32 block_size;
    u32 capacity;
    u8  bl0pba;
    u8  bl1pba;
    u8  blpinb;
    u8  bakcnt;
    u8  bakofs;
} flash_nand_info_t;

struct spinand_order
{
    u8  rfc;
    u8  rfc_addr_bytes;
    u8  dummy;
    u8  program_load;
    u8  random_load;
    u8  cr_mode;
    u32 max_wait_time;
};

struct spinand_handle
{
    u8                    ctrl_id;
    u8                    die_id;
    u8                    fcie_if;
    u8                    soc_ecc_en;
    u8                    bdma_to_xzdec_en;
    spinand_sni_t *       sni;
    struct spinand_order  order;
    struct spiflash_msg   msg;
    struct sstar_fcie_ecc ecc;
};

u8   mdrv_spinand_info(struct spinand_handle *handle, flash_nand_info_t *pst_flash_nand_info);
void mdrv_spinand_setup_by_default(struct spinand_handle *handle);

u8 mdrv_spinand_setup_by_sni(struct spinand_handle *handle);
u8 mdrv_spinand_hardware_init(struct spinand_handle *handle);

u8 mdrv_spinand_reset(struct spinand_handle *handle);
u8 mdrv_spinand_is_id_match(struct spinand_handle *handle, u8 *id, u8 id_len);
u8 mdrv_spinand_read_id(struct spinand_handle *handle, u8 *data, u8 size);
u8 mdrv_spinand_read_status(struct spinand_handle *handle, u8 *pstatus);
u8 mdrv_spinand_setup_by_volatile(struct spinand_handle *handle);
u8 mdrv_spinand_unlock_whole_block(struct spinand_handle *handle);

u8 mdrv_spinand_page_read_raw(struct spinand_handle *handle, u32 page, u16 column, u8 *data, u32 size);
u8 mdrv_spinand_page_program_raw(struct spinand_handle *handle, u32 page, u16 column, u8 *data, u32 size);
u8 mdrv_spinand_page_read(struct spinand_handle *handle, u32 page, u16 column, u8 *data, u32 size);
u8 mdrv_spinand_page_program(struct spinand_handle *handle, u32 page, u16 column, u8 *data, u32 size);
u8 mdrv_spinand_block_erase(struct spinand_handle *handle, u32 page);

u8  mdrv_spinand_is_support_otp(struct spinand_handle *handle);
u8  mdrv_spinand_get_otp_layout(struct spinand_handle *handle, u32 *start, u32 *length, u8 mode);
u8  mdrv_spinand_set_otp_mode(struct spinand_handle *handle, u8 enabled);
u8  mdrv_spinand_get_otp_lock(struct spinand_handle *handle);
u8  mdrv_spinand_set_otp_lock(struct spinand_handle *handle);
u32 mdrv_spinand_read_otp(struct spinand_handle *handle, u32 page, u16 column, u8 *data, u32 size);
u32 mdrv_spinand_write_otp(struct spinand_handle *handle, u32 page, u16 column, u8 *data, u32 size);

u8 mdrv_spinand_is_support_ubibbm(struct spinand_handle *handle);
u8 mdrv_spinand_set_ecc_mode(struct spinand_handle *handle, u8 enabled);
u8 mdrv_spinand_get_ecc_mode(struct spinand_handle *handle);
u8 mdrv_spinand_get_ecc_status(struct spinand_handle *handle);
u8 mdrv_spinand_soc_ecc_init(struct spinand_handle *handle, u8 ecc_path, u8 ecc_mode, u8 *fcie_buf);

u8  mdrv_spinand_page_data_read(struct spinand_handle *handle, u32 page);
u8  mdrv_spinand_block_isbad(struct spinand_handle *handle, u32 page);
u32 mdrv_spinand_pages_read(struct spinand_handle *handle, u32 page, u8 *data, u32 size);

u8  mdrv_spinand_page_read_to_xzdec(struct spinand_handle *handle, u32 u32_page, u16 u16_column, u8 *pu8_data,
                                    u32 u32_size);
u32 mdrv_spinand_pages_read_to_xzdec(struct spinand_handle *handle, u32 u32_page, u8 *pu8_data, u32 u32_size);

#endif /* _MDRV_SPINAND_H_ */
