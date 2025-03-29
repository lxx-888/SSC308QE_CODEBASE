/*
 * sstar_smc.h - Sigmastar
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

#ifndef SSTAR_SMC_H
#define SSTAR_SMC_H

#include "optee_smc.h"

#define OPTEE_SMC_FAST_SSTAR_MASK 0xFFFFFF00

/* add function related ID */

#define OPTEE_SMC_FAST_SSTAR_FUNC_PREFIX (0xFD00)
#define OPTEE_SMC_FAST_SSTAR_FUNC(id)    (OPTEE_SMC_FAST_SSTAR_FUNC_PREFIX + id)

/* TZMISC function ID  */
#define OPTEE_SMC_FUNCID_TZMISC OPTEE_SMC_FAST_SSTAR_FUNC(0x00)
#define OPTEE_SMC_TZMISC        OPTEE_SMC_FAST_CALL_VAL(OPTEE_SMC_FUNCID_TZMISC)

/* Boot timestamp function ID  */
#define OPTEE_SMC_FUNCID_BOOT_TS OPTEE_SMC_FAST_SSTAR_FUNC(0x01)
#define OPTEE_SMC_BOOT_TS        OPTEE_SMC_FAST_CALL_VAL(OPTEE_SMC_FUNCID_BOOT_TS)

/* add UT related ID */

#define OPTEE_SMC_FAST_SSTAR_UT_PREFIX (0xFE00)
#define OPTEE_SMC_FAST_SSTAR_UT(id)    (OPTEE_SMC_FAST_SSTAR_UT_PREFIX + id)

/* TZSP UT ID */
#define OPTEE_SMC_FUNCID_TZSP_UT OPTEE_SMC_FAST_SSTAR_UT(0x00)
#define OPTEE_SMC_TZSP_UT        OPTEE_SMC_FAST_CALL_VAL(OPTEE_SMC_FUNCID_TZSP_UT)

/* OTP UT ID */
#define OPTEE_SMC_FUNCID_OTP_UT OPTEE_SMC_FAST_SSTAR_UT(0x01)
#define OPTEE_SMC_OTP_UT        OPTEE_SMC_FAST_CALL_VAL(OPTEE_SMC_FUNCID_OTP_UT)

/* I2C UT ID */
#define OPTEE_SMC_FUNCID_I2C_UT OPTEE_SMC_FAST_SSTAR_UT(0x02)
#define OPTEE_SMC_I2C_UT        OPTEE_SMC_FAST_CALL_VAL(OPTEE_SMC_FUNCID_I2C_UT)

/* TZIMI UT ID */
#define OPTEE_SMC_FUNCID_TZIMI_UT OPTEE_SMC_FAST_SSTAR_UT(0x03)
#define OPTEE_SMC_TZIMI_UT        OPTEE_SMC_FAST_CALL_VAL(OPTEE_SMC_FUNCID_TZIMI_UT)

/* TZEMI UT ID */
#define OPTEE_SMC_FUNCID_TZEMI_UT OPTEE_SMC_FAST_SSTAR_UT(0x04)
#define OPTEE_SMC_TZEMI_UT        OPTEE_SMC_FAST_CALL_VAL(OPTEE_SMC_FUNCID_TZEMI_UT)

#endif /* SSTAR_SMC_H */
