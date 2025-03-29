/*
 * drv_xzdec.h- Sigmastar
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

#ifndef _DRV_XZDEC_H_
#define _DRV_XZDEC_H_

typedef enum
{
    XZDEC_BDMA_MIU_TO_DEC = 0x0,
    XZDEC_BDMA_SPI_TO_DEC = 0x1
} xzdec_path_select;

typedef enum
{
    XZDEC_SUCCESS = 0x0,
    XZDEC_PARAM_ERROR,
    XZDEC_DEVICE_ERROR,
    XZDEC_DECOMPRESSION_FAIL
} xzdec_error;

typedef void *xzdec_handle;

typedef struct
{
    xzdec_path_select select;
    void *            in_part;
    u64               in;
    u32               in_size;
    u8 *              out;
    u32               out_size;
    u8 *              head;
} xzdec_buf;

s32          drv_xzdec_init(u8 port, u8 *comp_buf);
xzdec_handle drv_xzdec_get(u8 port);
s32          drv_xzdec_decode(xzdec_handle handle, xzdec_buf *buf);

#endif /* _DRV_XZDEC_H_ */
