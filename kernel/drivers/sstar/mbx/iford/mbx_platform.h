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

/* iford */
#define MBX_RIU_BASE 0xFD000000

#ifdef __KERNEL__
/* cm4 to arm linux irq, bank:0x1E, offset:0x78, bit0, msk:0x1(1:trig, 0:clr) */
#define MBX_CM4_TO_ARM_INT_BASE  (0xFD003C00)
#define MBX_CM4_TO_ARM_INT_ADDR  (0x1E0)
#define MBX_CM4_TO_ARM_INT_MASK  (0x1)
#define MBX_CM4_TO_ARM_INT_SHIFT (0x0)
#else
/* cm4 to arm rtos irq, bank:0x2D, offset:0x0, bit0, msk:0x1(1:trig, 0:clr) */
#define MBX_CM4_TO_ARM_INT_BASE  (0x1F005A00)
#define MBX_CM4_TO_ARM_INT_ADDR  (0x0)
#define MBX_CM4_TO_ARM_INT_MASK  (0x1)
#define MBX_CM4_TO_ARM_INT_SHIFT (0x0)
#endif

#ifdef __KERNEL__
/* arm linux to cm4 irq, bank:0x2D, offset:0x01, bit0, msk:0x1(1:trig, 0:clr) */
#define MBX_ARM_TO_CM4_INT_BASE  (0xFD005A00)
#define MBX_ARM_TO_CM4_INT_ADDR  (0x04)
#define MBX_ARM_TO_CM4_INT_MASK  (0x1)
#define MBX_ARM_TO_CM4_INT_SHIFT (0x0)
#else
/* arm rtos to cm4 irq, bank:0x2B, offset:0x01, bit4, msk:0x10(1:trig, 0:clr) */
#define MBX_ARM_TO_CM4_INT_BASE  (0x1F005600)
#define MBX_ARM_TO_CM4_INT_ADDR  (0x04)
#define MBX_ARM_TO_CM4_INT_MASK  (0x10)
#define MBX_ARM_TO_CM4_INT_SHIFT (0x4)
#endif

#ifdef __KERNEL__
#define MBX_REG_BASE (0xFD006200) // bank 0x31
#else
#define MBX_REG_BASE (0x1F006200) // bank 0x31
#endif

#define MBX_REG_BYTE_WISE (4)
#define SHIFT_8           (8)

#define MBX_REG_PM_EXCEPTION_ADDR (MBX_REG_BYTE_WISE * 0x05)

#define MBX_REG_PM_BUFSIZE_ADDR (MBX_REG_BYTE_WISE * 0x1C)
#define MBX_REG_PM_BUFSIZE_MASK 0x7fff

#define MBX_REG_PM_BUFADDR_H_ADDR (MBX_REG_BYTE_WISE * 0x1D)
#define MBX_REG_PM_BUFADDR_L_ADDR (MBX_REG_BYTE_WISE * 0x1E)

#define MBX_REG_PM_STATE_ADDR  (MBX_REG_BYTE_WISE * 0x18)
#define MBX_REG_PM_STATE_MASK  (0xFFFF)
#define MBX_REG_PM_STATE_SHIFT (0)

#define MBX_REG_NONPM_STATE_ADDR  (MBX_REG_BYTE_WISE * 0x19)
#define MBX_REG_NONPM_STATE_MASK  (0xFFFF)
#define MBX_REG_NONPM_STATE_SHIFT (0)

// PM domain
#define MBX_REG_PM_CTRL_ADDR    (MBX_REG_BYTE_WISE * 0x1f)
#define MBX_REG_PM_FIRE_MASK    (0x1)
#define MBX_REG_PM_FIRE_SHIFT   (0)
#define MBX_REG_PM_STATUS_MASK  (0x0f00)
#define MBX_REG_PM_STATUS_SHIFT (8)

#define MBX_REG_PM_MSG_INFO_ADDR       (MBX_REG_BYTE_WISE * 0x06)
#define MBX_REG_PM_MSG_PARAM_CNT_MASK  (0xFF00)
#define MBX_REG_PM_MSG_PARAM_CNT_SHIFT (8)
#define MBX_REG_PM_MSG_CLASS_MASK      (0x00FF)
#define MBX_REG_PM_MSG_CLASS_SHIFT     (0)

#define MBX_REG_PM_MSG_P0_ADDR  (MBX_REG_BYTE_WISE * 0x07)
#define MBX_REG_PM_MSG_P0_MASK  (0xFFFF)
#define MBX_REG_PM_MSG_P0_SHIFT (0)

#define MBX_REG_PM_MSG_P1_ADDR  (MBX_REG_BYTE_WISE * 0x08)
#define MBX_REG_PM_MSG_P1_MASK  (0xFFFF)
#define MBX_REG_PM_MSG_P1_SHIFT (0)

#define MBX_REG_PM_MSG_P2_ADDR  (MBX_REG_BYTE_WISE * 0x09)
#define MBX_REG_PM_MSG_P2_MASK  (0xFFFF)
#define MBX_REG_PM_MSG_P2_SHIFT (0)

#define MBX_REG_PM_MSG_P3_ADDR  (MBX_REG_BYTE_WISE * 0x0a)
#define MBX_REG_PM_MSG_P3_MASK  (0xFFFF)
#define MBX_REG_PM_MSG_P3_SHIFT (0)

#define MBX_REG_PM_MSG_P4_ADDR  (MBX_REG_BYTE_WISE * 0x0b)
#define MBX_REG_PM_MSG_P4_MASK  (0xFFFF)
#define MBX_REG_PM_MSG_P4_SHIFT (0)

#define MBX_REG_PM_MSG_P5_ADDR  (MBX_REG_BYTE_WISE * 0x0c)
#define MBX_REG_PM_MSG_P5_MASK  (0xFFFF)
#define MBX_REG_PM_MSG_P5_SHIFT (0)

// NonPM domain
#ifdef __KERNEL__ // nonpm linux
#define MBX_REG_NONPM_CTRL_ADDR    (MBX_REG_BYTE_WISE * 0x28)
#define MBX_REG_NONPM_FIRE_MASK    (0x1)
#define MBX_REG_NONPM_FIRE_SHIFT   (0)
#define MBX_REG_NONPM_STATUS_MASK  (0x0f00)
#define MBX_REG_NONPM_STATUS_SHIFT (8)

#define MBX_REG_NONPM_MSG_INFO_ADDR       (MBX_REG_BYTE_WISE * 0x29)
#define MBX_REG_NONPM_MSG_PARAM_CNT_MASK  (0xFF00)
#define MBX_REG_NONPM_MSG_PARAM_CNT_SHIFT (8)
#define MBX_REG_NONPM_MSG_CLASS_MASK      (0x00FF)
#define MBX_REG_NONPM_MSG_CLASS_SHIFT     (0)

#define MBX_REG_NONPM_MSG_P0_ADDR  (MBX_REG_BYTE_WISE * 0x2a)
#define MBX_REG_NONPM_MSG_P0_MASK  (0xFFFF)
#define MBX_REG_NONPM_MSG_P0_SHIFT (0)

#define MBX_REG_NONPM_MSG_P1_ADDR  (MBX_REG_BYTE_WISE * 0x2b)
#define MBX_REG_NONPM_MSG_P1_MASK  (0xFFFF)
#define MBX_REG_NONPM_MSG_P1_SHIFT (0)

#define MBX_REG_NONPM_MSG_P2_ADDR  (MBX_REG_BYTE_WISE * 0x2c)
#define MBX_REG_NONPM_MSG_P2_MASK  (0xFFFF)
#define MBX_REG_NONPM_MSG_P2_SHIFT (0)

#define MBX_REG_NONPM_MSG_P3_ADDR  (MBX_REG_BYTE_WISE * 0x2d)
#define MBX_REG_NONPM_MSG_P3_MASK  (0xFFFF)
#define MBX_REG_NONPM_MSG_P3_SHIFT (0)

#define MBX_REG_NONPM_MSG_P4_ADDR  (MBX_REG_BYTE_WISE * 0x2e)
#define MBX_REG_NONPM_MSG_P4_MASK  (0xFFFF)
#define MBX_REG_NONPM_MSG_P4_SHIFT (0)

#define MBX_REG_NONPM_MSG_P5_ADDR  (MBX_REG_BYTE_WISE * 0x2f)
#define MBX_REG_NONPM_MSG_P5_MASK  (0xFFFF)
#define MBX_REG_NONPM_MSG_P5_SHIFT (0)
#else // nonpm rtos
#define MBX_REG_NONPM_CTRL_ADDR    (MBX_REG_BYTE_WISE * 0x20)
#define MBX_REG_NONPM_FIRE_MASK    (0x1)
#define MBX_REG_NONPM_FIRE_SHIFT   (0)
#define MBX_REG_NONPM_STATUS_MASK  (0x0f00)
#define MBX_REG_NONPM_STATUS_SHIFT (8)

#define MBX_REG_NONPM_MSG_INFO_ADDR       (MBX_REG_BYTE_WISE * 0x21)
#define MBX_REG_NONPM_MSG_PARAM_CNT_MASK  (0xFF00)
#define MBX_REG_NONPM_MSG_PARAM_CNT_SHIFT (8)
#define MBX_REG_NONPM_MSG_CLASS_MASK      (0x00FF)
#define MBX_REG_NONPM_MSG_CLASS_SHIFT     (0)

#define MBX_REG_NONPM_MSG_P0_ADDR  (MBX_REG_BYTE_WISE * 0x22)
#define MBX_REG_NONPM_MSG_P0_MASK  (0xFFFF)
#define MBX_REG_NONPM_MSG_P0_SHIFT (0)

#define MBX_REG_NONPM_MSG_P1_ADDR  (MBX_REG_BYTE_WISE * 0x23)
#define MBX_REG_NONPM_MSG_P1_MASK  (0xFFFF)
#define MBX_REG_NONPM_MSG_P1_SHIFT (0)

#define MBX_REG_NONPM_MSG_P2_ADDR  (MBX_REG_BYTE_WISE * 0x24)
#define MBX_REG_NONPM_MSG_P2_MASK  (0xFFFF)
#define MBX_REG_NONPM_MSG_P2_SHIFT (0)

#define MBX_REG_NONPM_MSG_P3_ADDR  (MBX_REG_BYTE_WISE * 0x25)
#define MBX_REG_NONPM_MSG_P3_MASK  (0xFFFF)
#define MBX_REG_NONPM_MSG_P3_SHIFT (0)

#define MBX_REG_NONPM_MSG_P4_ADDR  (MBX_REG_BYTE_WISE * 0x26)
#define MBX_REG_NONPM_MSG_P4_MASK  (0xFFFF)
#define MBX_REG_NONPM_MSG_P4_SHIFT (0)

#define MBX_REG_NONPM_MSG_P5_ADDR  (MBX_REG_BYTE_WISE * 0x27)
#define MBX_REG_NONPM_MSG_P5_MASK  (0xFFFF)
#define MBX_REG_NONPM_MSG_P5_SHIFT (0)
#endif
#endif
