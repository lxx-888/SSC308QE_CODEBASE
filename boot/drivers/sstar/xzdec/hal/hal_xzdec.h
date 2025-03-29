/*
 * hal_xzdec.h- Sigmastar
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

#ifndef _HAL_XZDEC_H_
#define _HAL_XZDEC_H_

/* ===== chip diff ======= */
// xzdec prot number
#define HAL_XZDEC_NR_PORTS 1
// xzdec bank
#define HAL_XZDEC0_BANK      (0x11D7)
#define HAL_XZDEC0_SGR_BANK  (0x11D8)
#define HAL_XZDEC0_SGW_BANK  (0x11DA)
#define HAL_XZDEC20_SGW_BANK (0x11DC)
// timeout
#define HAL_XZDEC_WAIT_TIMEOUT_MS (5000)
// header info
#define HAL_XZDEC_HEAD_MAX_SIZE 0x64

typedef enum
{
    HAL_XZDEC_COMPRESS_XZ = 0,
    HAL_XZDEC_COMPRESS_SXZ
} hal_xzdec_compress_type;

typedef enum
{
    HAL_XZDEC_DATA_SRC_SPI,
    HAL_XZDEC_DATA_SRC_MIU
} hal_xzdec_data_src;

typedef enum
{
    HAL_XZDEC_SUCCESS,
    HAL_XZDEC_ERR_MAGIC,
    HAL_XZDEC_ERR_BLOCK_SIZE,
    HAL_XZDEC_BDMA_DONE_FAIL,
    HAL_XZDEC_DONE_FAIL,
    HAL_XZDEC_CHECK_HEADER_FAIL,
    HAL_XZDEC_CHECK_DATA_FAIL,
    HAL_XZDEC_CHECK_SXZ_MAGIC_FAIL,
    HAL_XZDEC_DEVICE_FAILURE
} hal_xzdec_error;

typedef struct
{
    u32 compress_size;
    u32 compress_align_size;
    u32 uncompress_size;
    u32 uncompress_align_size;
} hal_xzdec_block;

typedef struct
{
    hal_xzdec_compress_type type;
    u8                      block_cnt;
    u32                     xz_dec_buf_size;
    u32                     xz_dec_size;
    u32                     xz_file_size;
    hal_xzdec_block         blocks[4];
} hal_xzdec_blocks_info;

typedef struct
{
    void *src_part;
    u64   src;
    u64   dst;
} hal_xzdec_ops;

struct hal_xzdec
{
    /* register bank */
    unsigned long xzdec_bank;
    unsigned long xzdec_sgr_bank;
    unsigned long xzdec_sgw_bank;
    unsigned long xzdec2_sgw_bank;

    u64 xzdec_buf;
    s32 (*calbak_xzdec_waitdone)(struct hal_xzdec *hal);
};

s32 hal_xzdec_get_blocks_info(void *xz_head, hal_xzdec_blocks_info *info);
s32 hal_xzdec_decode(struct hal_xzdec *hal, hal_xzdec_data_src select, hal_xzdec_blocks_info *info, hal_xzdec_ops *ops);

#endif /* _HAL_XZDEC_H_ */
