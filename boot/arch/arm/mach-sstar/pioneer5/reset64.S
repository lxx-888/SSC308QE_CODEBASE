/*
 * reset64.S - Sigmastar
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

/*------------------------------------------------------------------------------
    Function Code
-------------------------------------------------------------------------------*/
.extern Chip_Set_RebootType
	.align	5
.globl reset_cpu
reset_cpu:
	ldr   x0, =0x02
	bl    Chip_Set_RebootType
	mov   x1, #0x1CB8
	movk  x1, #0x1F00, lsl #16
	mov   w3, #0xFF
	strb  w3, [x1]
	mov   w3, #0x79
	strb  w3, [x1]
	nop
	nop
	nop
	nop

_loop_forever:
	b	_loop_forever
