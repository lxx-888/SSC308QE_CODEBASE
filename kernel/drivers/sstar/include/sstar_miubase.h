/*
 * sstar_miubase.h- Sigmastar
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

#include <cam_os_wrapper.h>

#if defined(CONFIG_ARM_LPAE) || defined(CONFIG_ARM64)

#ifdef CONFIG_MIU0_BUS_BASE
#define SSTAR_MIU0_BUS_BASE CAM_OS_UL(CONFIG_MIU0_BUS_BASE)
#define SSTAR_MIU1_BUS_BASE 0x2000000000ULL
#else
#define SSTAR_MIU0_BUS_BASE 0x20000000UL
#define SSTAR_MIU1_BUS_BASE 0x200000000ULL
#endif

#define ARM_MIU0_BUS_BASE SSTAR_MIU0_BUS_BASE
#define ARM_MIU1_BUS_BASE SSTAR_MIU1_BUS_BASE
#define ARM_MIU2_BUS_BASE 0xFFFFFFFFFFFFFFFFULL
#define ARM_MIU3_BUS_BASE 0xFFFFFFFFFFFFFFFFULL

#define ARM_MIU0_BASE_ADDR 0x00000000UL
#define ARM_MIU1_BASE_ADDR 0xFFFFFFFFFFFFFFFFULL
#define ARM_MIU2_BASE_ADDR 0xFFFFFFFFFFFFFFFFULL
#define ARM_MIU3_BASE_ADDR 0xFFFFFFFFFFFFFFFFULL

#else
#ifdef CONFIG_MIU0_BUS_BASE
#define SSTAR_MIU0_BUS_BASE CAM_OS_UL(CONFIG_MIU0_BUS_BASE)
#else
#define SSTAR_MIU0_BUS_BASE 0x20000000UL
#endif
#define SSTAR_MIU1_BUS_BASE 0xA0000000UL

#define ARM_MIU0_BUS_BASE SSTAR_MIU0_BUS_BASE
#define ARM_MIU1_BUS_BASE SSTAR_MIU1_BUS_BASE
#define ARM_MIU2_BUS_BASE 0xFFFFFFFFUL
#define ARM_MIU3_BUS_BASE 0xFFFFFFFFUL

#define ARM_MIU0_BASE_ADDR 0x00000000UL
#define ARM_MIU1_BASE_ADDR 0x80000000UL
#define ARM_MIU2_BASE_ADDR 0xFFFFFFFFUL
#define ARM_MIU3_BASE_ADDR 0xFFFFFFFFUL

#endif // CONFIG_ARM_LPAE

extern unsigned int query_frequency(void);
