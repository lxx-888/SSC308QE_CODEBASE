/*
 * sdio.h- Sigmastar
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
/***************************************************************************************************************
 *
 * FileName sdio.c
 *     @author jeremy.wang (2012/01/10)
 * Desc:
 *     This layer between Linux SD Driver layer and IP Hal layer.
 *     (1) The goal is we don't need to change any Linux SD Driver code, but we can handle here.
 *     (2) You could define Function/Ver option for using, but don't add Project option here.
 *     (3) You could use function option by Project option, but please add to ms_sdmmc.h
 *
 ***************************************************************************************************************/

#ifndef __SDIO_H
#define __SDIO_H

#define SDIO_FORCE_1_BIT_MODE (0)

typedef int sdio_irq_callback(U8_T u8Slot);

// Sdio driver init
// int sdio_init(void);

// Power Off/On, CMD5, CMD3, CMD7, Set Speed, Set bus width
int sdio_card_init(U8_T u8Slot);

// CMD5, CMD3, CMD7, Set Speed, Set bus width
int sdio_card_reset(U8_T u8Slot);

// CMD52, Read single byte
int sdio_read_byte(U8_T u8Slot, U8_T func, U32_T addr, U8_T* r_buf);

// CMD52, Write single byte
int sdio_write_byte(U8_T u8Slot, U8_T func, u32 addr, U8_T w_value);

// CMD53, Read multiple bytes, blk_mode=0
int sdio_read_byte_multi(U8_T u8Slot, U8_T func, U8_T op_code, U32_T addr, U32_T count, U8_T* r_buf);

// CMD53, Write multiple bytes, blk_mode=0
int sdio_write_byte_multi(U8_T u8Slot, U8_T func, U8_T op_code, U32_T addr, U32_T count, U8_T* w_buf);

// CMD53, Read multiple blocks, blk_mode=1
int sdio_read_block_multi(U8_T u8Slot, U8_T func, U8_T op_code, U32_T addr, U32_T count, U32_T blk_size, U8_T* r_buf);

// CMD53, Write multiple blocks, blk_mode=1
int sdio_write_block_multi(U8_T u8Slot, U8_T func, U8_T op_code, U32_T addr, U32_T count, U32_T blk_size, U8_T* w_buf);

// SDIO interrupt irq, enable:1 disable:0
int sdio_irq_enable(U8_T u8Slot, int enable);

// SDIO set interrupt irq callback
int sdio_set_irq_callback(U8_T u8Slot, sdio_irq_callback* irq_cb);

#endif // __SDI
