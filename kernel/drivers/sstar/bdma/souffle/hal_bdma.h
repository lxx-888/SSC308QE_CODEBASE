/*
 * hal_bdma.h- Sigmastar
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

#ifndef __HAL_BDMA_H__
#define __HAL_BDMA_H__

#include "ms_platform.h"

/*=============================================================*/
// Data type definition
/*=============================================================*/

typedef enum
{
    HAL_BDMA_PROC_DONE       = 0,
    HAL_BDMA_ERROR           = -1,
    HAL_BDMA_POLLING_TIMEOUT = -2,
    HAL_BDMA_NO_INIT         = -3
} hal_bdma_err;

typedef enum
{
    // BDMA0
    HAL_BDMA_CH0 = 0,
    HAL_BDMA_CH1,
    HAL_BDMA_CH2,
    HAL_BDMA_CH3,

    // BDMA2
    HAL_BDMA1_CH0,
    HAL_BDMA1_CH1,
    HAL_BDMA1_CH2,
    HAL_BDMA1_CH3,

    // BDMA3
    HAL_BDMA2_CH0,
    HAL_BDMA2_CH1,
    HAL_BDMA2_CH2,
    HAL_BDMA2_CH3,

    HAL_BDMA_CH_NUM
} hal_bdma_channel;

typedef enum
{
    HAL_BDMA_MIU0_TO_MIU0 = 0x0,
    HAL_BDMA_MIU0_TO_MIU1,
    HAL_BDMA_MIU1_TO_MIU0,
    HAL_BDMA_MIU1_TO_MIU1,
    HAL_BDMA_MIU0_TO_IMI,
    HAL_BDMA_MIU1_TO_IMI,
    HAL_BDMA_IMI_TO_MIU0,
    HAL_BDMA_IMI_TO_MIU1,
    HAL_BDMA_IMI_TO_IMI,
    HAL_BDMA_MEM_TO_MIU0,
    HAL_BDMA_MEM_TO_MIU1,
    HAL_BDMA_MEM_TO_IMI,
    HAL_BDMA_PM_SPI_TO_MIU0,
    HAL_BDMA_SPI_TO_MIU0,
    HAL_BDMA_SPI_TO_MIU1,
    HAL_BDMA_SPI_TO_IMI,
    HAL_BDMA_MIU0_TO_PM_SPI,
    HAL_BDMA_MIU0_TO_SPI,
    HAL_BDMA_MSPI0_TO_MIU,
    HAL_BDMA_MIU_TO_MSPI0,
    HAL_BDMA_MSPI1_TO_MIU,
    HAL_BDMA_MIU_TO_MSPI1,
    HAL_BDMA_MSPI2_TO_MIU,
    HAL_BDMA_MIU_TO_MSPI2,
    HAL_BDMA_MSPI3_TO_MIU,
    HAL_BDMA_MIU_TO_MSPI3,
    HAL_BDMA_MIU0_TO_CM4_IMI,
    HAL_BDMA_MIU1_TO_CM4_IMI,
    HAL_BDMA_CM4_IMI_TO_MIU0,
    HAL_BDMA_CM4_IMI_TO_MIU1,
    HAL_BDMA_PSRAM_TO_MIU0,
    HAL_BDMA_MIU0_TO_PSRAM
} hal_bdma_path_select;

typedef enum
{
    HAL_BDMA_DATA_BYTE_1  = 0x0,
    HAL_BDMA_DATA_BYTE_2  = 0x1,
    HAL_BDMA_DATA_BYTE_4  = 0x2,
    HAL_BDMA_DATA_BYTE_8  = 0x3,
    HAL_BDMA_DATA_BYTE_16 = 0x4
} hal_bdma_data_width;

typedef enum
{
    HAL_BDMA_ADDR_INC = 0x0,
    HAL_BDMA_ADDR_DEC = 0x1
} hal_bdma_addr_mode;

/*=============================================================*/
// Structure definition
/*=============================================================*/

typedef void (*hal_bdma_callback)(void *);

typedef struct
{
    u32 u32SrcWidth;  ///< Width of source
    u32 u32SrcOffset; ///< Line-to-line offset of source
    u32 u32DstWidth;  ///< Width of destination
    u32 u32DstOffset; ///< Line-to-line offset of destination
} hal_bdma_line_offset;

typedef struct
{
    bool                  bIntMode;
    hal_bdma_path_select  ePathSel;
    hal_bdma_data_width   eSrcDataWidth;
    hal_bdma_data_width   eDstDataWidth;
    hal_bdma_addr_mode    eDstAddrMode;
    u32                   u32TxCount;
    u32                   u32Pattern;
    phys_addr_t           pSrcAddr;
    phys_addr_t           pDstAddr;
    u32                   bEnLineOfst;
    hal_bdma_line_offset *pstLineOfst;
    hal_bdma_callback     pfTxCbFunc;
    void *                pTxCbParm;
} hal_bdma_param;

/*=============================================================*/
// Global function definition
/*=============================================================*/

hal_bdma_err hal_bdma_initialize(u8 channel);
hal_bdma_err hal_bdma_transfer(u8 channel, hal_bdma_param *bdma_param);

#endif // __HAL_BDMA_H__
