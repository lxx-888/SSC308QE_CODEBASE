/*
 * sensor_log.h - Sigmastar
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

#ifndef _SENSORLOG_H_
#define _SENSORLOG_H_

#include <cam_os_wrapper.h>

#define CamOsPrintf printf

typedef enum
{
    E_SENSOR_LOG_INFO   = 0x1,
    E_SENSOR_LOG_ERR    = 0x2,
    E_SENSOR_LOG_TIMING = 0x4,
    E_SENSOR_LOG_DBG    = 0x8,
} VifLogId_e;

void         SENSORIFLogSetEn(int id, int en);
unsigned int SENSORIFLogGetEn(void);

#define SENSORLog(ID, fmt, ...)                \
    do                                         \
    {                                          \
        if (SENSORIFLogGetEn() & ID)           \
            CamOsPrintf((fmt), ##__VA_ARGS__); \
    } while (0)
#define SENSORLogErr(fmt, ...)                     \
    do                                             \
    {                                              \
        if (SENSORIFLogGetEn() & E_SENSOR_LOG_ERR) \
            CamOsPrintf((fmt), ##__VA_ARGS__);     \
    } while (0)
#define SENSORLogInfo(fmt, ...)                     \
    do                                              \
    {                                               \
        if (SENSORIFLogGetEn() & E_SENSOR_LOG_INFO) \
            CamOsPrintf((fmt), ##__VA_ARGS__);      \
    } while (0)
#define SENSORIFLogDbg(fmt, ...)                   \
    do                                             \
    {                                              \
        if (SENSORIFLogGetEn() & E_SENSOR_LOG_DBG) \
            CamOsPrintf((fmt), ##__VA_ARGS__);     \
    } while (0)
#endif
