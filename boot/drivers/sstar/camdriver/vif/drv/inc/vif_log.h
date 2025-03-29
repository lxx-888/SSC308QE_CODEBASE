/*
 * vif_log.h - Sigmastar
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

#ifndef _VIFLOG_H_
#define _VIFLOG_H_

#include <cam_os_wrapper.h>

#define CamOsPrintf printf

typedef enum
{
    E_VIF_LOG_INFO   = 0x1,
    E_VIF_LOG_ERR    = 0x2,
    E_VIF_LOG_TIMING = 0x4,
    E_VIF_LOG_DBG    = 0x8,
} VifLogId_e;

void         VIFLogSetEn(int id, int en);
unsigned int VIFLogGetEn(void);

void VIFPipeDbgSetEn(u8 pipe, u8 en);
u32  VIFPipeDbgGetEn(u8 pipe);

#define VIFLog(ID, fmt, ...)                   \
    do                                         \
    {                                          \
        if (VIFLogGetEn() & ID)                \
            CamOsPrintf((fmt), ##__VA_ARGS__); \
    } while (0)
#define VIFLogErr(fmt, ...)                    \
    do                                         \
    {                                          \
        if (VIFLogGetEn() & E_VIF_LOG_ERR)     \
            CamOsPrintf((fmt), ##__VA_ARGS__); \
    } while (0)
#define VIFLogInfo(fmt, ...)                   \
    do                                         \
    {                                          \
        if (VIFLogGetEn() & E_VIF_LOG_INFO)    \
            CamOsPrintf((fmt), ##__VA_ARGS__); \
    } while (0)
#define VIFLogDbg(fmt, ...)                    \
    do                                         \
    {                                          \
        if (VIFLogGetEn() & E_VIF_LOG_DBG)     \
            CamOsPrintf((fmt), ##__VA_ARGS__); \
    } while (0)
#endif
