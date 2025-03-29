/* SigmaStar trade secret */
/* Copyright (c) [2019~2020] SigmaStar Technology.
All rights reserved.

Unless otherwise stipulated in writing, any and all information contained
herein regardless in any format shall remain the sole proprietary of
SigmaStar and be kept in strict confidence
(SigmaStar Confidential Information) by the recipient.
Any unauthorized act including without limitation unauthorized disclosure,
copying, use, reproduction, sale, distribution, modification, disassembling,
reverse engineering and compiling of the contents of SigmaStar Confidential
Information is unlawful and strictly prohibited. SigmaStar hereby reserves the
rights to any and all damages, losses, costs and expenses resulting therefrom.
*/
#ifndef __MI_PSPI_DATATYPE_H
#define __MI_PSPI_DATATYPE_H

#include "mi_sys_datatype.h"
#include "mi_common_datatype.h"

#define PSPI_PARAM_BUFF_SIZE 8

//----spi_mode
#define SPI_CPHA  0x01 /* clock phase    */
#define SPI_CPOL  0x02 /* clock polarity */
#define SPI_SSCTL 0x04
#define SPI_SSPOL 0x08
#define SPI_SLAVE 0x10
#define SPI_LSB   0x20

//------data_lane
#define DATA_SINGLE 0x01
#define DATA_DUAL   0x02
#define DATA_QUAD   0x04

//--------rgb_swap
#define RGB_SINGLE 0x01
#define RGB_DUAL   0x02
#define BGR_SINGLE 0x04
#define BGR_DUAL   0x08

//---------chip_select
#define MI_PSPI_SELECT_0    0
#define MI_PSPI_SELECT_1    1
#define MI_PSPI_SELECT_NULL 2

typedef MI_S32 MI_PSPI_DEV;

typedef enum
{
    E_MI_PSPI_INVALID_TYPE = 0,
    E_MI_PSPI_TYPE_RX      = 1,
    E_MI_PSPI_TYPE_TX      = 2,
    E_MI_PSPI_TYPE_MAX,
} MI_PSPI_Type_e;

typedef enum
{
    E_MI_PSPI_TRIGGER_MODE_NA = 0,
    E_MI_PSPI_TRIGGER_MODE_AUTO,
    E_MI_PSPI_TRIGGER_MODE_AUTO_VSYNC,
    E_MI_PSPI_TRIGGER_MODE_MAX,
} MI_PSPI_TriggerMode_e;

typedef struct MI_PSPI_Msg_s
{
    MI_U16 u16TxSize;
    MI_U16 u16RxSize;
    MI_U8  u8TxBitCount;
    MI_U8  u8RxBitCount;
    MI_U16 au16TxBuf[PSPI_PARAM_BUFF_SIZE];
    MI_U16 au16RxBuf[PSPI_PARAM_BUFF_SIZE];
} MI_PSPI_Msg_t;

typedef struct MI_PSPI_OutputAttr_s
{
    MI_SYS_PixelFormat_e ePixelFormat;
    MI_U16               u16Width;
    MI_U16               u16Height;
} MI_PSPI_OutputAttr_t;

typedef struct MI_PSPI_Param_s
{
    MI_U32                u32MaxSpeedHz;
    MI_U16                u16DelayCycle; /* cs is inactive*/
    MI_U16                u16WaitCycle;  /* cs is active  */
    MI_U16                u16PspiMode;
    MI_U8                 u8DataLane;    /* cs count      */
    MI_U8                 u8BitsPerWord; /* The number of bits in an SPI transmission*/
    MI_U8                 u8RgbSwap;     /* for panel     */
    MI_U8                 u8TeMode;      /* for panel     */
    MI_U8                 u8ChipSelect;
    MI_PSPI_Type_e        ePspiType;
    MI_PSPI_TriggerMode_e eTriggerMode; /* select trigger mode*/
} MI_PSPI_Param_t;

#define MI_DEF_PSPI_ERR(err) MI_DEF_ERR(E_MI_MODULE_ID_PSPI, E_MI_ERR_LEVEL_ERROR, err)

#define MI_PSPI_FAIL                MI_DEF_PSPI_ERR(E_MI_ERR_FAILED)
#define MI_ERR_PSPI_ILLEGAL_PARAM   MI_DEF_PSPI_ERR(E_MI_ERR_ILLEGAL_PARAM)
#define MI_ERR_PSPI_NULL_PTR        MI_DEF_PSPI_ERR(E_MI_ERR_NULL_PTR)
#define MI_ERR_PSPI_NO_MEM          MI_DEF_PSPI_ERR(E_MI_ERR_NOMEM)
#define MI_ERR_PSPI_SYS_NOTREADY    MI_DEF_PSPI_ERR(E_MI_ERR_SYS_NOTREADY)
#define MI_ERR_PSPI_DEV_NOT_INIT    MI_DEF_PSPI_ERR(E_MI_ERR_NOT_INIT)
#define MI_ERR_PSPI_DEV_HAVE_INITED MI_DEF_PSPI_ERR(E_MI_ERR_INITED)
#define MI_ERR_PSPI_NOT_ENABLE      MI_DEF_PSPI_ERR(E_MI_ERR_NOT_ENABLE)

#endif
