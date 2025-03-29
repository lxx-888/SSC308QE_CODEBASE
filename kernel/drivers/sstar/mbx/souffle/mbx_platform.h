/*
 * mbx_platform.h- Sigmastar
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

#ifndef _MBX_PLATFORM_H_
#define _MBX_PLATFORM_H_

/* souffle */
#define MBX_RIU_BASE 0xFD000000

/* cm4 to arm irq, bank:0x2D, offset:0x00, msk:0x1(1:trig, 0:clr) */
#define MBX_CM4_TO_ARM_INT_BASE 0xFD005A00
#define MBX_CM4_TO_ARM_INT_ADDR 0x00
#define MBX_CM4_TO_ARM_INT_MASK 0x1

/* arm to cm4 irq, bank:0x2D, offset:0x01, msk:0x1(1:trig, 0:clr) */
#define MBX_ARM_TO_CM4_INT_BASE 0xFD005A00
#define MBX_ARM_TO_CM4_INT_ADDR 0x04
#define MBX_ARM_TO_CM4_INT_MASK 0x1

#define MBX_REG_BASE 0xFD006200 // bank 0x31

#endif
