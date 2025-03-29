/*
 * sensor_datatype.h - Sigmastar
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

#ifndef SENSOR_DATATYPE_H
#define SENSOR_DATATYPE_H

typedef enum
{
    E_VIF_CHANNEL_0 = 0,
    E_VIF_CHANNEL_1,
    E_VIF_CHANNEL_2,
    E_VIF_CHANNEL_3,
    E_VIF_CHANNEL_4,
    E_VIF_CHANNEL_5,
    E_VIF_CHANNEL_6,
    E_VIF_CHANNEL_7,
    E_VIF_CHANNEL_NUM,
} VifChannel_e;

typedef enum
{
    E_SENSOR_POLARITY_LOW  = 0,
    E_SENSOR_POLARITY_HIGH = 1
} SensorPolarity_e;

typedef enum
{
    E_SENSOR_BUS_TYPE_PARL = 0,
    E_SENSOR_BUS_TYPE_MIPI,
    E_SENSOR_BUS_TYPE_BT601,
    E_SENSOR_BUS_TYPE_BT656,
    E_SENSOR_BUS_TYPE_BT1120,
    E_SENSOR_BUS_TYPE_LVDS,
    E_SENSOR_BUS_TYPE_MAX,
} SensorBusType_e;
//=======================================//

#define SENSOR_SUCCESS     0
#define SENSOR_FAIL        (-1)
#define SENSOR_NOT_SUPPORT (-2)

#define VIF_MAX_SENSOR_PAD    3
#define VIF_MAX_GROUP_NUM     2
#define VIF_MAX_GROUP_DEV_NUM 4
#define VIF_MAX_DEV_NUM       8
#define VIF_MAX_CHANNEL       8
#define VIF_MAX_GPIO_PIN      1
#define VIF_MAX_PIPE          2
#define VIF_MAX_PIPE2PORT     1
#define MAX_DEV_NUM_IN_CORE   8
#define MAX_PIPE_NUM_IN_CORE  2
#define MAX_ISP_CORE_NUM      1
#define MAX_IRLED_NUM         2

#define VIF_AFIFO_SIZE_LARGE 4
#define VIF_AFIFO_SIZE_SMALL 2

#define ISPSCL_PORT1_OFFSET 0
#define VIF_WDMA_PTS_UNIT   21333ULL

#endif
