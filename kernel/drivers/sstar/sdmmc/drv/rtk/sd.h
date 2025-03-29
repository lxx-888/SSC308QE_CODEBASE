/*
 * sd.h- Sigmastar
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
 * FileName sd.h
 *     @author jeremy.wang (2012/01/10)
 * Desc:
 *     This layer between Linux SD Driver layer and IP Hal layer.
 *     (1) The goal is we don't need to change any Linux SD Driver code, but we can handle here.
 *     (2) You could define Function/Ver option for using, but don't add Project option here.
 *     (3) You could use function option by Project option, but please add to ms_sdmmc.h
 *
 ***************************************************************************************************************/

#ifndef __SD_H
#define __SD_H

#define UNSTUFF_BITS(resp, start, size)                             \
    (                                                               \
        {                                                           \
            const int __size = size;                                \
            const u32 __mask = (__size < 32 ? 1 << __size : 0) - 1; \
            const int __off  = 3 - ((start) / 32);                  \
            const int __shft = (start)&31;                          \
            u32       __res;                                        \
                                                                    \
            __res = resp[__off] >> __shft;                          \
            if (__size + __shft > 32)                               \
                __res |= resp[__off - 1] << ((32 - __shft) % 32);   \
            __res& __mask;                                          \
        })

int MMPF_SD_SendCommand(U8_T u8Slot, U8_T command, U32_T argument);

// Sd driver init
// int sd_init(void);

// sd driver deinit
// int sd_deinit(void);

// Power Off/On, CMD0, CMD8, acmd41, cmd2, cmd3, cmd9, cmd13, CMD7, Set Speed, Set bus width
int sd_card_init(U8_T u8Slot);

// CMD0, CMD8, acmd41, cmd2, cmd3, cmd9, cmd13, CMD7, Set Speed, Set bus width
int sd_card_reset(U8_T u8Slot);

// CMD17, Read single block
int sd_read_block(U8_T slot, U32_T addr, volatile U8_T* r_buf);

// CMD24, Write single block
int sd_write_block(U8_T slot, U32_T addr, volatile U8_T* r_buf);

// CMD18, Read multi block
int sd_read_block_multi(U8_T slot, U32_T addr, U16_T u61BlkCnt, volatile U8_T* r_buf);

// CMD25, Write multi block
int sd_write_block_multi(U8_T slot, U32_T addr, U16_T u16BlkCnt, volatile U8_T* r_buf);

U64_T sd_get_capacity(U8_T slot);

int sd_erase_data(U8_T slot, U32_T u32EraseDataStart, U32_T u32EraseDataEnd);

#endif
