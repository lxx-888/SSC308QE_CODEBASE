/*
 * hal_sensor_reg.h - Sigmastar
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

#ifndef __HAL_VIF_REG__
#define __HAL_VIF_REG__

#define VIF_REG_R(base, offset)      (*(u16*)((uintptr_t)(base) + ((offset)*4)))
#define VIF_REG_W(base, offset, val) ((*(u16*)(((uintptr_t)(base)) + ((offset)*4))) = (val))
#define BASE_REG_RIU_PA              0x1F000000
#define SENSOR_BASE_ADDR_W_BANK(x)   ((BASE_REG_RIU_PA) + ((x) << 9))

typedef struct SensorHandle_s
{
    uintptr_t nHandle;
    uintptr_t nBasePA;
    u32       nSize;
} SensorHandle_t;

typedef enum
{
    E_SENSOR_HANDLE_VIF_C0 = 0x0, // 0 BANK:0x1308~0x1309
    E_SENSOR_HANDLE_PADTOP,       // 1 BANK:0x103C
    E_SENSOR_HANDLE_CLKGEN,       // 2 BANK:0x1038
    E_SENSOR_HANDLE_CHIPTOP,      // 3 BANK:0x101E
    E_SENSOR_HANDLE_ID_MAX
} SensorHandleId_e;

#endif
