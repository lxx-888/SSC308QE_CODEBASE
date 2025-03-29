/*
 * drv_xzdec.c- Sigmastar
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

#include <drv_flash_os_impl.h>
#include <drv_part.h>
#include <drv_xzdec.h>
#include <hal_xzdec.h>

struct xzdec
{
    u8   port;
    char xzdec_name[15];

    struct hal_xzdec hal;
};

static struct xzdec *drv_xzdec[HAL_XZDEC_NR_PORTS];
static struct xzdec  xzdec0;

s32 drv_xzdec_init(u8 port, u8 *comp_buf)
{
    if (0 == port)
    {
        drv_xzdec[0]                     = NULL;
        xzdec0.port                      = 0;
        xzdec0.hal.xzdec_bank            = SSTAR_BASE_REG_RIU_PA + (HAL_XZDEC0_BANK << 9);
        xzdec0.hal.xzdec_sgr_bank        = SSTAR_BASE_REG_RIU_PA + (HAL_XZDEC0_SGR_BANK << 9);
        xzdec0.hal.xzdec_sgw_bank        = SSTAR_BASE_REG_RIU_PA + (HAL_XZDEC0_SGW_BANK << 9);
        xzdec0.hal.xzdec2_sgw_bank       = SSTAR_BASE_REG_RIU_PA + (HAL_XZDEC20_SGW_BANK << 9);
        xzdec0.hal.xzdec_buf             = (u64)(unsigned long)comp_buf;
        xzdec0.hal.calbak_xzdec_waitdone = NULL;
        drv_xzdec[0]                     = &xzdec0;
    }
    else
        return -XZDEC_DEVICE_ERROR;

    return XZDEC_SUCCESS;
}

xzdec_handle drv_xzdec_get(u8 port)
{
    if (port >= HAL_XZDEC_NR_PORTS || !drv_xzdec[port])
        return NULL;

    return (xzdec_handle)(drv_xzdec[port]);
}

s32 drv_xzdec_decode(xzdec_handle handle, xzdec_buf *buf)
{
    s32                   ret   = 0;
    struct xzdec *        xzdec = NULL;
    hal_xzdec_blocks_info info;
    hal_xzdec_ops         ops;

    buf->out_size = 0;

    if (!handle)
        return -XZDEC_PARAM_ERROR;

    xzdec = (struct xzdec *)handle;

    info.xz_file_size = buf->in_size;

    if (buf->select == XZDEC_BDMA_SPI_TO_DEC)
    {
        buf->head = buf->out;
        if (HAL_XZDEC_HEAD_MAX_SIZE != sstar_part_load(buf->in_part, buf->in, buf->head, HAL_XZDEC_HEAD_MAX_SIZE))
            return -XZDEC_DEVICE_ERROR;
    }

    if (HAL_XZDEC_SUCCESS != hal_xzdec_get_blocks_info((void *)buf->head, &info))
    {
        return -XZDEC_DECOMPRESSION_FAIL;
    }

    ops.src_part = buf->in_part;
    ops.src      = buf->in;
    ops.dst      = (u64)(unsigned long)(buf->out);

    switch (buf->select)
    {
        case XZDEC_BDMA_MIU_TO_DEC:
            ret = hal_xzdec_decode(&xzdec->hal, HAL_XZDEC_DATA_SRC_MIU, &info, &ops);
            break;
        case XZDEC_BDMA_SPI_TO_DEC:
            ret = hal_xzdec_decode(&xzdec->hal, HAL_XZDEC_DATA_SRC_SPI, &info, &ops);
            break;
    }

    if (HAL_XZDEC_SUCCESS != ret)
        return -XZDEC_DECOMPRESSION_FAIL;

    buf->out_size = info.xz_dec_size;

    return XZDEC_SUCCESS;
}
